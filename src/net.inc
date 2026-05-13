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

NET_IP_MAX_LEN equ 0x10

sockaddr_un_t:
  .sun_family dw 1
  .sun_path   db "mishell.sock", 0
sockaddr_un_t_end:

unix_path_len     equ sockaddr_un_t_end - sockaddr_un_t.sun_path
sockaddr_un_t_len equ sockaddr_un_t_end - sockaddr_un_t

sockaddr_in_t_len dq sockaddr_in_t_end - sockaddr_in_t

LISTEN_BACKLOG  equ 50


section .text

; gets the string representation of the ipv4 of a file descriptor
; @param  rdi: file descriptor of the client
; @param  rsi: pointer to the string holding the IP
; @return rax: return code
net_get_ip_from_fd:
  sub   rsp, 0x28
  sub   rsp, qword [sockaddr_in_t_len]

  ; STACK USAGE
  ; [rsp]       -> client fd
  ; [rsp+0x8]   -> pointer to the string representation of the ip
  ; [rsp+0x10]  -> dynamic pointer for the string
  ; [rsp+0x18]  -> dynamic pointer sockaddr_addr_in
  ; [rsp+0x20]  -> tmp str to store int to str (8 bytes is enough)
  ; [rsp+0x28]  -> sockaddr struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rsi

  test  rsi, rsi
  jz    .error

  ; getpeername
  mov   rax, SYS_GETPEERNAME
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x28]
  lea   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; get addr_in
  lea   rax, [rsp+0x28]
  add   rax, 0x4
  mov   [rsp+0x18], rax

  mov   r9, 3 ; three dots for IPV4

.loop:
  mov   rax, [rsp+0x18]
  mov   dil, byte [rax]
  movzx rdi, dil
  inc   rax
  mov   [rsp+0x18], rax

  cmp   r9, 0
  jl   .loop_end

  lea   rsi, [rsp+0x20]
  mov   rdx, 4  ; add one for null char
  call  itoa
  cmp   rax, 0
  jl    .error

  ; copy
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add dot
  mov   rdi, rax
  mov   rax, DOT
  stosb

  mov   [rsp+0x10], rdi

  dec   r9

  jmp   .loop

.loop_end:
  ; ip has an extra dot, go back one and add null char
  mov   rdi, [rsp+0x10]
  dec   rdi
  mov   rax, NULL_CHAR
  stosb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, qword [sockaddr_in_t_len]
  add   rsp, 0x28
  ret
