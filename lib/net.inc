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

NET_IPV4_MAX_LEN equ 0x10

sockaddr_un_t:
  .sun_family dw AF_LOCAL
  .sun_path   db "mishell.sock", 0
sockaddr_un_t_end:

unix_path_len     equ sockaddr_un_t_end - sockaddr_un_t.sun_path
sockaddr_un_t_len equ sockaddr_un_t_end - sockaddr_un_t

sockaddr_in_t_len dq sockaddr_in_t_end - sockaddr_in_t

LISTEN_BACKLOG  equ 50

section .text

; gets the ipv4 of a file descriptor
; @param  rdi: file descriptor of the client
; @return rax: ipv4
net_get_ipv4_from_fd:
  sub   rsp, 0x10
  sub   rsp, qword [sockaddr_in_t_len]

  ; STACK USAGE
  ; [rsp]       -> client fd
  ; [rsp+0x8]   -> addrlen
  ; [rsp+0x10]  -> sockaddr struct

  mov   [rsp], rdi

  test  rsi, rsi
  jz    .error

  ; getpeername
  mov   rax, SYS_GETPEERNAME
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x10]
  lea   rdx, [rsp+0x8]
  syscall
  cmp   rax, 0
  jl    .error

  ; make sure the fd is AF_INET
  lea   rdi, [rsp+0x10]
  cmp   word [rdi], AF_INET
  jne   .done

  ; copy dword
  mov   eax, dword [rdi+0x4] ; get addr_in
  jmp   .return

.done:
  mov   eax, SUCCESS_CODE
  jmp   .return

.error:
  mov   eax, FAILURE_CODE

.return:
  add   rsp, qword [sockaddr_in_t_len]
  add   rsp, 0x10
  ret
