global _start

section .bss

epoll_fd  resw 1
tcp_fd    resw 1
unix_fd   resw 1

epoll_event_t:
  .events resd 1
  .data   resw 1

section .data

section .text

_start:
  ; initialise epoll instance
  mov   rax, 291  ; EPOLL_CREATE1
  xor   di, di
  syscall
  cmp   ax, 0
  jl    .error

  mov   [epoll_fd], ax
  
  ; create tcp socket
  mov   rax, 41 ; SOCKET
  mov   di, 2   ; AF_INET
  mov   si, 2   ; SOCK_STREAM
  xor   dx, dx 
  syscall
  cmp   ax, 0
  jl    .error

  mov   [tcp_fd], ax

  ; add tcp socket to the epoll interest list
  mov   dword [epoll_event_t.events], 1 ; EPOLLIN
  mov   word [epoll_event_t.data], ax

  mov   rax, 233  ; EPOLL_CTL
  mov   di, [epoll_fd]
  mov   si, 1     ; EPOLL_CTL_ADDA
  mov   dx, [tcp_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   ax, 0
  jl    .error

  ; create unix socket
  mov   rax, 41 ; SOCKET
  mov   di, 1   ; AF_LOCAL
  mov   si, 2   ; SOCK_STREAM
  xor   dx, dx 
  syscall
  cmp   ax, 0
  jl    .error

  mov   [unix_fd], ax

  ; add unix socket to the epoll interest list
  mov   dword [epoll_event_t.events], 1 ; EPOLLIN
  mov   word [epoll_event_t.data], ax

  mov   rax, 233  ; EPOLL_CTL
  mov   di, [epoll_fd]
  mov   si, 1     ; EPOLL_CTL_ADDA
  mov   dx, [unix_fd]
  mov   r10, epoll_event_t
  syscall
  cmp   ax, 0
  jl    .error

  ; bind tcp socket

  ; bind unix socket

  ; listen to tcp
  ; listen to unix

  ; epoll loop

  ; close tcp socket
  mov   rax, 3  ; CLOSE
  mov   di, [tcp_fd]
  syscall
  
  ; close unix socket
  mov   rax, 3  ; CLOSE
  mov   di, [unix_fd]
  syscall

  ; close epoll socket
  mov   rax, 3  ; CLOSE
  mov   di, [epoll_fd]
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

