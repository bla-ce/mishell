global _start

section .bss

epoll_fd  resw 1
tcp_fd    resw 1
unix_fd   resw 1

section .data

section .text

_start:
  ; initialise epoll instance
  mov   rax, 291  ; EPOLL_CREATE1
  xor   di, di
  syscall

  mov   [epoll_fd], ax
  
  ; create tcp socket
  mov   rax, 41 ; SOCKET
  mov   di, 2   ; AF_INET
  mov   si, 2   ; SOCK_STREAM
  xor   dx, dx 
  syscall

  mov   [tcp_fd], ax

  ; add tcp socket to the epoll instance

  ; create unix socket
  mov   rax, 41 ; SOCKET
  mov   di, 1   ; AF_LOCAL
  mov   si, 2   ; SOCK_STREAM
  xor   dx, dx 
  syscall

  mov   [unix_fd], ax

  ; add unix socket to the epoll instance

  ; bind tcp socket

  ; bind unix socket

  ; listen to tcp
  ; listen to unix

  ; epoll loop

  ; close tcp socket
  mov   rax, 3  ; CLOSE
  mov   di, word [tcp_fd]
  syscall
  
  ; close unix socket
  mov   rax, 3  ; CLOSE
  mov   di, word [unix_fd]
  syscall

  ; close epoll socket
  mov   rax, 3  ; CLOSE
  mov   di, word [epoll_fd]
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

