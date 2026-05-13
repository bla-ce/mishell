%include "constants.inc"
%include "host.inc"
%include "net.inc"
%include "ops.inc"
%include "packet.inc"
%include "string.inc"

global _start

section .data

tcp_port  equ 7474

log:
  .listen_tcp     db "[mishell] listening on TCP port 7474", 10
  .listen_tcp_len equ $ - log.listen_tcp

  .listen_unix      db "[mishell] listening on UNIX socket mishell.sock", 10
  .listen_unix_len  equ $ - log.listen_unix

  .accept_new_conn      db "[mishell] accepted new connection", 10
  .accept_new_conn_len  equ $ - log.accept_new_conn

  .recv_packet      db "[mishell] received packet from client", 10
  .recv_packet_len  equ $ - log.recv_packet

section .text

_start:
  sub   rsp, PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet struct

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
  mov   ax, tcp_port
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

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  mov   rsi, log.listen_tcp
  mov   rdx, log.listen_tcp_len
  syscall
  cmp   rax, 0
  jl    .error

  ; create unix socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_LOCAL
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall
  cmp   rax, 0
  jl    .error

  mov   [unix_fd], rax

  ; unlink unix socket
  mov   rax, SYS_UNLINK
  mov   rdi, sockaddr_un_t.sun_path
  syscall ; fine to error here

  ; bind unix socket
  mov   rax, SYS_BIND
  mov   rdi, [unix_fd]
  mov   rsi, sockaddr_un_t
  mov   rdx, sockaddr_un_t_len
  syscall
  cmp   rax, 0
  jl    .error

  ; listen to unix
  mov   rax, SYS_LISTEN
  mov   rdi, [unix_fd]
  mov   rsi, LISTEN_BACKLOG
  syscall
  cmp   rax, 0
  jl    .error

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  mov   rsi, log.listen_unix
  mov   rdx, log.listen_unix_len
  syscall
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

  ; add unix socket to the epoll interest list
  mov   dword [epoll_event_t.events], EPOLLIN
  mov   rax, [unix_fd]
  mov   qword [epoll_event_t.data], rax

  mov   rax, SYS_EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, EPOLL_CTL_ADD
  mov   rdx, [unix_fd]
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
  je    .new_connection

  cmp   rax, [unix_fd]
  je    .new_connection

  jmp   .existing_connection

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

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  mov   rsi, log.accept_new_conn
  mov   rdx, log.accept_new_conn_len
  syscall
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

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  mov   rsi, log.recv_packet
  mov   rdx, log.recv_packet_len
  syscall
  cmp   rax, 0
  jl    .clear_connection

  ; handle the packet
  lea   rdi, [rsp]
  mov   rsi, [conn_fd]
  call  packet_dispatch
  cmp   rax, 0
  jl    .clear_connection

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

  ; close unix socket
  mov   rax, SYS_CLOSE
  mov   rdi, [unix_fd]
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

  ; close unix socket
  mov   rax, SYS_CLOSE
  mov   rdi, [unix_fd]
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
