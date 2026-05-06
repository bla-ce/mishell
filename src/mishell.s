global _start

section .bss

epoll_fd  resq 1
tcp_fd    resq 1
unix_fd   resq 1
conn_fd   resq 1
nfds      resq 1

epoll_event_t:
  .events resd 1
  .data   resq 1
epoll_event_t_end:

sockaddr_in_t:
  .sin_family resw 1
  .sin_port   resw 1
  .sin_addr   resd 1
  .sin_zero   resq 1
sockaddr_in_t_end:

events resb EPOLL_EVENT_T_LEN * MAX_EVENTS

section .data

enable dd 1

MAX_EVENTS        equ 10
EPOLL_EVENT_T_LEN equ epoll_event_t_end - epoll_event_t

tcp_port  equ 7474

sockaddr_in_t_len dq sockaddr_in_t_end - sockaddr_in_t

LISTEN_BACKLOG  equ 50

sockaddr_un_t:
  .sun_family dw 1
  .sun_path   db "mishell.sock", 0
sockaddr_un_t_end:

unix_path_len     equ sockaddr_un_t_end - sockaddr_un_t.sun_path
sockaddr_un_t_len equ sockaddr_un_t_end - sockaddr_un_t

section .text

_start:
  ; create tcp socket
  mov   rax, 41 ; SOCKET
  mov   rdi, 2  ; AF_INET
  mov   rsi, 1  ; SOCK_STREAM
  mov   rdx, 0
  syscall
  cmp   rax, 0
  jl    .error

  mov   [tcp_fd], rax

  ; enable reuse address
  mov   rax, 54         ; SETSOCKOPT
  mov   rdi, [tcp_fd]
  mov   rsi, 1          ; SOL_SOCKET
  mov   rdx, 2          ; SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4
  syscall
  cmp   rax, 0
  jl    .error

  ; bind tcp socket
  mov   ax, tcp_port
  xchg  al, ah ; bswap 16-bit registers

  mov   word [sockaddr_in_t.sin_family], 2  ; AF_INET
  mov   word [sockaddr_in_t.sin_port], ax
  mov   dword [sockaddr_in_t.sin_addr], 0
  mov   qword [sockaddr_in_t.sin_zero], 0

  mov   rax, 49 ; BIND
  mov   rdi, [tcp_fd]
  mov   rsi, sockaddr_in_t
  mov   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; listen to tcp
  mov   rax, 50 ; LISTEN
  mov   rdi, [tcp_fd]
  mov   rsi, LISTEN_BACKLOG
  syscall
  cmp   rax, 0
  jl    .error

  ; create unix socket
  mov   rax, 41 ; SOCKET
  mov   rdi, 1  ; AF_LOCAL
  mov   rsi, 1  ; SOCK_STREAM
  mov   rdx, 0 
  syscall
  cmp   rax, 0
  jl    .error

  mov   [unix_fd], rax

  ; unlink unix socket
  mov   rax, 87 ; UNLINK
  mov   rdi, sockaddr_un_t.sun_path
  syscall ; fine to error here

  ; bind unix socket
  mov   rax, 49 ; BIND
  mov   rdi, [unix_fd]
  mov   rsi, sockaddr_un_t
  mov   rdx, sockaddr_un_t_len
  syscall
  cmp   rax, 0
  jl    .error

  ; listen to unix
  mov   rax, 50 ; LISTEN
  mov   rdi, [unix_fd]
  mov   rsi, LISTEN_BACKLOG
  syscall
  cmp   rax, 0
  jl    .error

  ; initialise epoll instance
  mov   rax, 291  ; EPOLL_CREATE1
  xor   rdi, rdi
  syscall
  cmp   rax, 0
  jl    .error

  mov   [epoll_fd], rax
  
  ; add tcp socket to the epoll interest list
  mov   dword [epoll_event_t.events], 1 ; EPOLLIN
  mov   rax, [tcp_fd]
  mov   [epoll_event_t.data], rax

  mov   rax, 233    ; EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, 1      ; EPOLL_CTL_ADD
  mov   rdx, [tcp_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   rax, 0
  jl    .error

  ; add unix socket to the epoll interest list
  mov   dword [epoll_event_t.events], 1 ; EPOLLIN
  mov   rax, [unix_fd]
  mov   qword [epoll_event_t.data], rax

  mov   rax, 233    ; EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, 1      ; EPOLL_CTL_ADD
  mov   rdx, [unix_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   rax, 0
  jl    .error

.outer_loop:
  ; epoll wait
  mov   rax, 232  ; EPOLL_WAIT
  mov   rdi, [epoll_fd]
  mov   rsi, events
  mov   rdx, MAX_EVENTS
  mov   rcx, -1
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

  imul  r12, EPOLL_EVENT_T_LEN
  mov   rax, [rdi + r12 + 4] 

  ; check if conn fd is a new connection
  cmp   rax, [tcp_fd]
  je    .new_connection

  cmp   rax, [unix_fd]
  je    .new_connection

  jmp   .existing_connection

.new_connection:
  ; accept connection
  mov   rdi, rax
  mov   rsi, 0
  mov   rdx, 0
  mov   rax, 43 ; ACCEPT
  syscall
  cmp   rax, 0
  jl    .next_connection

  mov   [conn_fd], rax
  
  ; set conn fd non blocking
  mov   rax, 72         ; FCNTL
  mov   rdi, [conn_fd]
  mov   rsi, 3          ; F_GETFL
  syscall
  cmp   rax, 0
  jl    .clear_connection

  mov   rdx, rax
  or    rdx, 2048
  mov   rax, 72         ; FCNTL
  mov   rdi, [conn_fd]
  mov   rsi, 4          ; F_SETFL
  syscall
  cmp   rax, 0
  jl    .clear_connection

  ; add conn fd to epoll instance
  mov   dword [epoll_event_t.events], 1 ; EPOLLIN
  mov   rax, [conn_fd]
  mov   [epoll_event_t.data], rax

  mov   rax, 233    ; EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, 1      ; EPOLL_CTL_ADD
  mov   rdx, [conn_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   rax, 0
  jl    .clear_connection

  jmp   .next_connection

.existing_connection:
  ; read message

  jmp   .next_connection

.clear_connection:
  ; close conn fd socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [conn_fd]
  syscall

  ; remove conn fd from epoll instance
  mov   rax, 233    ; EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, 2      ; EPOLL_CTL_DEL
  mov   rdx, [conn_fd]
  mov   r10, epoll_event_t
  syscall

.next_connection:
  inc   r12
  jmp   .inner_loop

.inner_loop_end:
  jmp   .outer_loop

.cleanup:
  ; close tcp socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [tcp_fd]
  syscall
  
  ; close unix socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [unix_fd]
  syscall

  ; close epoll socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [epoll_fd]
  syscall

  mov   rdi, 0
  jmp   .exit

.error:
  ; close tcp socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [tcp_fd]
  syscall
  
  ; close unix socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [unix_fd]
  syscall

  ; close epoll socket
  mov   rax, 3  ; CLOSE
  mov   rdi, [epoll_fd]
  syscall

  mov   rax, 60
  mov   rdi, -1
  syscall

.exit:
  mov   rax, 60
  syscall

