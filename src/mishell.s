global _start

section .bss

epoll_fd  resq 1
tcp_fd    resq 1
unix_fd   resq 1

epoll_event_t:
  .events resd 1
  .data   resq 1

sockaddr_in_t:
  .sin_family resw 1
  .sin_port   resw 1
  .sin_addr   resd 1
  .sin_zero   resq 1
sockaddr_in_t_end:

section .data

tcp_port  equ 7474

sockaddr_in_len equ sockaddr_in_t_end - sockaddr_in_t

LISTEN_BACKLOG equ 50

sockaddr_un_t:
  .sun_family dw 1
  .sun_path   db "herve.sock", 0
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
  mov   rdx, sockaddr_in_len
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

  mov   rax, 233  ; EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, 1     ; EPOLL_CTL_ADDA
  mov   rdx, [tcp_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   ax, 0
  jl    .error

  ; add unix socket to the epoll interest list
  mov   dword [epoll_event_t.events], 1 ; EPOLLIN
  mov   rax, [unix_fd]
  mov   qword [epoll_event_t.data], rax

  mov   rax, 233  ; EPOLL_CTL
  mov   rdi, [epoll_fd]
  mov   rsi, 1     ; EPOLL_CTL_ADD
  mov   rdx, [unix_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   ax, 0
  jl    .error

  ; epoll loop

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
  mov   rax, 60
  mov   rdi, -1
  syscall

.exit:
  mov   rax, 60
  syscall

