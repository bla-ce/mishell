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
; converts the character string into a network address
; TODO: support inetv6
; @param  rdi: pointer to the string
; @param  rsi: pointer to the destination
; @return rax: return code
inet_pton:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]     -> pointer to the string (s)
  ; [rsp+0x8] -> pointer to the destination buffer (d)

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  xor   r9b, r9b    ; bytes counter (i)

.outer_loop:
  cmp   r9b, 4
  jge   .error

  ; reset values
  xor   r10b, r10b  ; digit counter (j)
  xor   r11, r11  ; byte value (v)

.inner_loop:
  cmp   r10b, 3
  jge   .inner_loop_end

  ; get next byte
  mov   rdi, [rsp]
  movzx rax, byte [rdi+r10]

  ; check if byte is digit
  cmp   rax, ZERO_CHAR
  jl    .inner_loop_end

  cmp   rax, NINE_CHAR
  jg    .inner_loop_end

  ; v = 10*v + s[j]-'0'
  imul  r11, 10
  sub   rax, ZERO_CHAR
  add   r11, rax

  inc   r10b
  jmp   .inner_loop

.inner_loop_end:
  ; if (j==0 || (j>1 && s[0]=='0') || v>255) return 0;
  cmp   r10b, 0
  je    .error

  ; value can't exceed 255
  cmp   r11, 255
  jg    .error

  cmp   r10b, 1
  jle   .move_value

  mov   rdi, [rsp]
  cmp   byte [rdi], ZERO_CHAR
  je    .error

.move_value:
  ; d[i] = v;
  mov   rdi, [rsp+0x8]
  mov   byte [rdi+r9], r11b

  ; if (s[j]==0 && i==3) return 1;
  mov   rdi, [rsp]
  cmp   byte [rdi+r10], NULL_CHAR
  jne   .continue

  cmp   r9b, 3
  je    .outer_loop_end

.continue:
  ; if (s[j]!='.') return 0;
  mov   rdi, [rsp]
  cmp   byte [rdi+r10], DOT
  jne   .error

  ; s += j+1;
  mov   rdi, [rsp]
  add   rdi, r10
  inc   rdi
  mov   [rsp], rdi

  inc   r9b

  jmp   .outer_loop
.outer_loop_end:

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret
