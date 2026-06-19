%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

global _start

section .rodata

log:
  .listen_tcp       db "[MISHELL] listening on tcp socket", NULL_CHAR
  .accept_new_conn  db "[MISHELL] accepted new connection", NULL_CHAR
  .recv_packet      db "[MISHELL] received packet from client", NULL_CHAR

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

  ; create tcp socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall
  cmp   rax, 0
  jl    .error

  mov   [tcp_fd], rax

  ; enable reuse address
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, [tcp_fd]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4
  syscall
  cmp   rax, 0
  jl    .error

  ; bind tcp socket
  mov   ax, word [host_port]
  xchg  al, ah ; bswap 16-bit registers

  mov   word [sockaddr_in_t.sin_family], AF_INET
  mov   word [sockaddr_in_t.sin_port], ax
  mov   dword [sockaddr_in_t.sin_addr], 0
  mov   qword [sockaddr_in_t.sin_zero], 0

  mov   rax, SYS_BIND
  mov   rdi, [tcp_fd]
  mov   rsi, sockaddr_in_t
  mov   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; listen to tcp
  mov   rax, SYS_LISTEN
  mov   rdi, [tcp_fd]
  mov   rsi, LISTEN_BACKLOG
  syscall
  cmp   rax, 0
  jl    .error

  mov   rdi, log.listen_tcp
  call  println
  cmp   rax, 0
  jl    .error

  ; initialise epoll instance
  mov   rax, SYS_EPOLL_CREATE1
  xor   rdi, rdi
  syscall
  cmp   rax, 0
  jl    .error

  mov   [epoll_fd], rax

  ; add tcp socket to the epoll interest list
  mov   dword [epoll_event_t.events], EPOLLIN
  mov   rax, [tcp_fd]
  mov   [epoll_event_t.data], rax

  mov   rax, SYS_EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, EPOLL_CTL_ADD
  mov   rdx, [tcp_fd]
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

  mov   rdi, log.accept_new_conn
  call  println
  cmp   rax, 0
  jl    .error

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
  jl    .error

  ; get packet
  mov   rax, SYS_READ
  mov   rdi, [conn_fd]
  lea   rsi, [rsp]
  mov   rdx, PACKET_MAX_LEN
  syscall
  cmp   rax, 0
  jle   .clear_connection

  mov   rdi, log.recv_packet
  call  println
  cmp   rax, 0
  jl    .clear_connection

  ; handle the packet
  lea   rdi, [rsp]
  mov   rsi, [conn_fd]
  call  packet_dispatch

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

  mov   rdi, 0
  jmp   .exit

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
