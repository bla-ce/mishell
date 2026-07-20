%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "logs.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"
%include "state.inc"

global _start

section .rodata

MISHELL_MIN_ARG equ 0x5

section .text

_start:
  sub   rsp, PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]                   -> pointer to the packet struct
  ; [rsp+PACKET_T_LEN+0x8]  -> pointer to program name
  ; [rsp+PACKET_T_LEN+0x10] -> pointer to command (init or connect)
  ; [rsp+PACKET_T_LEN+0x18] -> pointer to ip:port of the remote host (connect) or port flag (init)
  ; [rsp+PACKET_T_LEN+0x20] -> pointer to port flag (connect) or listening port (init)
  ; [rsp+PACKET_T_LEN+0x28] -> pointer to listening port (connect) or name flag (init)
  ; [rsp+PACKET_T_LEN+0x30] -> pointer to name flag (connect) or name (init)
  ; [rsp+PACKET_T_LEN+0x38] -> pointer to name (connect)

  cmp   qword [rsp+PACKET_T_LEN], MISHELL_MIN_ARG
  jl    .usage

  ; check if port flag has been set
  mov   rdi, [rsp+PACKET_T_LEN+0x18]
  mov   rsi, PORT_FLAG
  call  strcmp
  mov   rdi, [rsp+PACKET_T_LEN+0x20]
  cmp   rax, TRUE
  je    .set_port

  mov   rdi, [rsp+PACKET_T_LEN+0x20]
  mov   rsi, PORT_FLAG
  call  strcmp
  mov   rdi, [rsp+PACKET_T_LEN+0x28]
  cmp   rax, TRUE
  jne   .check_name_flag  ; default port

.set_port:
  call  atoi
  cmp   rax, 0
  jl    .check_name_flag

  mov   word [host_port], ax

.check_name_flag:
  ; check if name flag has been set
  mov   rdi, [rsp+PACKET_T_LEN+0x28]
  mov   rsi, NAME_FLAG
  call  strcmp
  mov   rdx, [rsp+PACKET_T_LEN+0x30]
  cmp   rax, TRUE
  je    .init_or_join_network

  mov   rdi, [rsp+PACKET_T_LEN+0x30]
  mov   rsi, NAME_FLAG
  call  strcmp
  mov   rdx, [rsp+PACKET_T_LEN+0x38]
  cmp   rax, TRUE
  je    .init_or_join_network

  ; default name
  mov   rdx, default_host_name

.init_or_join_network:
  mov   rdi, [rsp+PACKET_T_LEN+0x10]
  mov   rsi, [rsp+PACKET_T_LEN+0x18]
  ; rdx is already populated with name
  call  init_or_join_network
  cmp   rax, 0
  jl    .error

  ; load state if there's one
  call  state_load

  ; initialise logan
  mov   rdi, logan_t
  xor   rsi, rsi
  call  logan_init
  cmp   rax, 0
  jl    .error

  xor   rdi, rdi
  movzx rsi, word [host_port]
  call  net_init_tcp_socket
  cmp   rax, 0
  jl    .error

  mov   [tcp_fd], rax

  mov   rdi, [tcp_fd]
  call  net_epoll_init
  cmp   rax, 0
  jl    .error

  mov   [epoll_fd], rax

  ; create timerfd for host hearbeat
  mov   rdi, HOST_HEARTBEAT
  call  net_create_timerfd
  cmp   rax, 0
  jl    .error

  mov   [timer_fd], rax

  ; add timer fd to the epoll instance
  mov   dword [epoll_event_t.events], EPOLLET
  or    dword [epoll_event_t.events], EPOLLIN
  mov   [epoll_event_t.data], rax

  mov   rax, SYS_EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, EPOLL_CTL_ADD
  mov   rdx, [timer_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   rax, 0
  jl    .error

.outer_loop:
  ; epoll wait
  mov   rax, SYS_EPOLL_WAIT
  mov   rdi, [epoll_fd]
  mov   rsi, events
  mov   rdx, MAX_EVENTS
  mov   r10, -1
  syscall
  cmp   rax, 0
  jl    .error

  mov   [nfds], rax

  xor   r12, r12  ; init counter

.inner_loop:
  ; check if we processed all events
  cmp   r12, [nfds]
  jge   .inner_loop_end

  ; get the conn fd of this event
  mov   rdi, events

  imul  rcx, r12, EPOLL_EVENT_T_LEN
  mov   rax, [rdi + rcx + 4]

  mov   [conn_fd], rax

  ; check if heartbeat
  cmp   rax , [timer_fd]
  je    .hearbeat

  ; check if conn fd is a new connection
  cmp   rax, [tcp_fd]
  jne   .existing_connection

.new_connection:
  ; accept connection
  mov   rax, SYS_ACCEPT
  mov   rdi, [conn_fd]
  mov   rsi, 0
  mov   rdx, 0
  syscall
  cmp   rax, 0
  jl    .next_connection

  mov   [conn_fd], rax

  ; set conn fd non blocking
  mov   rax, SYS_FCNTL
  mov   rdi, [conn_fd]
  mov   rsi, F_GETFL
  syscall
  cmp   rax, 0
  jl    .clear_connection

  mov   rdx, rax
  or    rdx, O_NONBLOCK
  mov   rax, SYS_FCNTL
  mov   rdi, [conn_fd]
  mov   rsi, F_SETFL
  syscall
  cmp   rax, 0
  jl    .clear_connection

  ; add conn fd to epoll instance
  mov   edx, EPOLLIN
  mov   ecx, EPOLLET
  or    edx, ecx
  mov   dword [epoll_event_t.events], edx
  mov   rax, [conn_fd]
  mov   [epoll_event_t.data], rax

  mov   rax, SYS_EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, EPOLL_CTL_ADD
  mov   rdx, [conn_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   rax, 0
  jl    .clear_connection

  jmp   .next_connection

.existing_connection:
  ; reset packet
  lea   rdi, [rsp]
  call  packet_reset
  cmp   rax, 0
  jl    .clear_connection

  ; get packet
  mov   rax, SYS_READ
  mov   rdi, [conn_fd]
  lea   rsi, [rsp]
  mov   rdx, PACKET_MAX_LEN
  syscall
  cmp   rax, 0
  jle   .clear_connection

  ; handle the packet
  lea   rdi, [rsp]
  mov   rsi, [conn_fd]
  call  packet_dispatch

  jmp   .next_connection

.hearbeat:
  call  host_hearbeat

  jmp   .next_connection

.clear_connection:
  ; remove conn fd from epoll instance
  mov   rax, SYS_EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, EPOLL_CTL_DEL
  mov   rdx, [conn_fd]
  mov   r10, epoll_event_t
  syscall

  ; close conn fd socket
  mov   rax, SYS_CLOSE
  mov   rdi, [conn_fd]
  syscall

.next_connection:
  inc   r12
  jmp   .inner_loop

.inner_loop_end:
  jmp   .outer_loop

.cleanup:
  ; close tcp socket
  mov   rax, SYS_CLOSE
  mov   rdi, [tcp_fd]
  syscall

  ; close epoll socket
  mov   rax, SYS_CLOSE
  mov   rdi, [epoll_fd]
  syscall

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.usage:
  mov   rdi, usage_str
  call  println

.error:
  ; close tcp socket
  mov   rax, SYS_CLOSE
  mov   rdi, [tcp_fd]
  syscall

  ; close epoll socket
  mov   rax, SYS_CLOSE
  mov   rdi, [epoll_fd]
  syscall

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

.exit:
  mov   rax, SYS_EXIT
  syscall
