section .bss

epoll_fd  resq 1
tcp_fd    resq 1
timer_fd  resq 1
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

itimerspec_t:
  .it_interval_tv_sec   resq 1
  .it_interval_tv_nsec  resq 1
  .it_value_tv_sec      resq 1
  .it_value_tv_nsec     resq 1

section .rodata

SOCKADDR_IN_T_OFF_SIN_FAMILY  equ 0x0
SOCKADDR_IN_T_OFF_SIN_PORT    equ SOCKADDR_IN_T_OFF_SIN_FAMILY + 0x2
SOCKADDR_IN_T_OFF_SIN_ADDR    equ SOCKADDR_IN_T_OFF_SIN_PORT + 0x2
SOCKADDR_IN_T_OFF_SIN_ZERO    equ SOCKADDR_IN_T_OFF_SIN_ADDR + 0x4

SOCKADDR_IN_T_LEN equ sockaddr_in_t_end - sockaddr_in_t

section .data

enable dd 1

MAX_EVENTS        equ 10
EPOLL_EVENT_T_LEN equ epoll_event_t_end - epoll_event_t

NET_IPV4_MAX_LEN equ 0x10

sockaddr_in_t_len dq sockaddr_in_t_end - sockaddr_in_t

LISTEN_BACKLOG  equ 50

section .text
; creates a socket and connects it to a remote addr
; @param  rdi: ip
; @param  rsi: port (byte order)
; @return rax: sock fd
net_tcp_connect_to_remote_addr:
  sub   rsp, 0x18+SOCKADDR_IN_T_LEN

  ; STACK USAGE
  ; [rsp]       -> ip
  ; [rsp+0x8]   -> port
  ; [rsp+0x10]  -> sock fd
  ; [rsp+0x18]  -> sockaddr_in struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; create tcp socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; connect to socket
  lea   rdi, [rsp+0x18]
  mov   word [rdi+SOCKADDR_IN_T_OFF_SIN_FAMILY], AF_INET
  mov   ax, word [rsp+0x8]
  movbe word [rdi+SOCKADDR_IN_T_OFF_SIN_PORT], ax
  mov   eax, dword [rsp]
  mov   dword [rdi+SOCKADDR_IN_T_OFF_SIN_ADDR], eax
  mov   qword [rdi+SOCKADDR_IN_T_OFF_SIN_ZERO], 0

  mov   rax, SYS_CONNECT
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]
  jmp   .return

.error:
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x10]
  syscall

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+SOCKADDR_IN_T_LEN
  ret

; initialises a new tcp socket, creating the socket, binding it
; to port and ip and listening to it
; @param  rdi: ip
; @param  rsi: port (byte order)
; @return rax: sock fd
net_init_tcp_socket:
  sub   rsp, 0x18+SOCKADDR_IN_T_LEN

  ; STACK USAGE
  ; [rsp]       -> ip
  ; [rsp+0x8]   -> port
  ; [rsp+0x10]  -> sock fd
  ; [rsp+0x18]  -> sockaddr_in struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; create tcp socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; enable reuse address
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, [rsp+0x10]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4
  syscall
  cmp   rax, 0
  jl    .error

  ; bind tcp socket
  lea   rdi, [rsp+0x18]
  mov   word [rdi+SOCKADDR_IN_T_OFF_SIN_FAMILY], AF_INET
  mov   ax, word [rsp+0x8]
  movbe word [rdi+SOCKADDR_IN_T_OFF_SIN_PORT], ax
  mov   eax, dword [rsp]
  mov   dword [rdi+SOCKADDR_IN_T_OFF_SIN_ADDR], eax
  mov   qword [rdi+SOCKADDR_IN_T_OFF_SIN_ZERO], 0

  mov   rax, SYS_BIND
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; listen to tcp
  mov   rax, SYS_LISTEN
  mov   rdi, [rsp+0x10]
  mov   rsi, LISTEN_BACKLOG
  syscall
  cmp   rax, 0
  jl    .error

  mov   rdi, logan_t
  mov   rsi, log.listen_tcp
  call  log_info
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]
  jmp   .return

.error:
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x10]
  syscall

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+SOCKADDR_IN_T_LEN
  ret

; converts the character string into a network address
; NOTE: it does not support IPV6 yet
; @param  rdi: pointer to the string
; @return rax: network address (dword)
inet_pton:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the string (s)
  ; [rsp+0x8] -> pointer to the destination buffer (d)

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  test  rdi, rdi
  jz    .error

  xor   r9, r9    ; bytes counter (i)

.outer_loop:
  cmp   r9b, 4
  jge   .error

  ; reset values
  xor   r10, r10  ; digit counter (j)
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
  lea   rdi, [rsp+0x8]
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

  movsx rax, dword [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; creates a timer fd to deliver timed notifications via fd
; NOTE: for this project, we hardcode flags and clockid
; @param  rdi: time in seconds
; @return rax: fd
net_create_timerfd:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> time
  ; [rsp+0x8]   -> file descriptor

  mov   [rsp], rdi

  ; create timer fd
  mov   rax, SYS_TIMERFD_CREATE
  mov   rdi, CLOCK_MONOTONIC
  mov   rsi, TFD_NONBLOCK
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; reset itimerspec
  mov   qword [itimerspec_t.it_interval_tv_sec], 0
  mov   qword [itimerspec_t.it_interval_tv_nsec], 0
  mov   qword [itimerspec_t.it_value_tv_sec], 0
  mov   qword [itimerspec_t.it_value_tv_nsec], 0

  mov   rax, [rsp]
  mov   qword [itimerspec_t.it_interval_tv_sec], rax
  mov   qword [itimerspec_t.it_value_tv_sec], rax

  ; set time value
  mov   rax, SYS_TIMERFD_SETTIME
  mov   rdi, [rsp+0x8]
  xor   rsi, rsi
  mov   rdx, itimerspec_t
  xor   r10, r10
  syscall
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
