%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"

global _start

section .rodata

MISHLI_MIN_ARG equ 4

mishli_usage_str  db "usage:  mishli --host <host_addr> hello   -> check if host is up", LINE_FEED
                  db "        mishli --host <host_addr> auth    -> generate or validate token", LINE_FEED
                  db "        mishli --host <host_addr> register <host_id> <type>          -> register new service", LINE_FEED
                  db "        mishli --host <host_addr> start <host_id> <service_id>       -> start service", LINE_FEED
                  db "        mishli --host <host_addr> stop <host_id> <service_id>        -> stop service", LINE_FEED
                  db "        mishli --host <host_addr> unregister <host_id> <service_id>  -> unregister service", LINE_FEED
                  db "        mishli --host <host_addr> list                               -> list available hosts", LINE_FEED
mishli_usage_str_len equ $ - mishli_usage_str

host_flag db "--host", NULL_CHAR

section .data

host_fd dq 0

section .text

_start:
  ; STACK USAGE
  ; [rsp]       -> argc
  ; [rsp+0x8]   -> pointer to the program name
  ; [rsp+0x10]  -> pointer to the host flag key
  ; [rsp+0x18]  -> pointer to the host flag value
  ; [rsp+0x20]  -> pointer to op

  cmp   qword [rsp], MISHLI_MIN_ARG
  jl    .usage

  ; make sure that host flag is set
  mov   rdi, [rsp+0x10]
  mov   rsi, host_flag
  call  strcmp
  test  rax, rax  ; FALSE
  jz    .usage

  mov   rdi, [rsp+0x18]
  call  mishli_connect_to_host
  cmp   rax, 0
  jl    .usage

  ; populate base packet
  mov   word [packet_t.magic], MAGIC_VALUE
  mov   word [packet_t.flags], FL_USER

  ; check if op is valid
  mov   rdi, [rsp+0x20]
  call  op_get_from_str
  cmp   rax, 0
  jl    .error

  mov   byte [packet_t.op], al

  ; send packet
  mov   rax, SYS_WRITE
  mov   rdi, [host_fd]
  mov   rsi, packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receive response
  mov   rax, SYS_READ
  mov   rdi, [host_fd]
  mov   rsi, packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; close socket
  mov   rax, SYS_CLOSE
  mov   rdi, qword [host_fd]
  syscall

  jmp   .exit

.usage:
  mov   rax, SYS_WRITE
  mov   rdi, STDERR_FILENO
  mov   rsi, mishli_usage_str
  mov   rdx, mishli_usage_str_len
  syscall

.error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

.exit:
  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall


; connects to the host before sending the request
; @param  rdi: pointer to the host address
; @return rax: return code
mishli_connect_to_host:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the host address

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; parse host address
  mov   rdi, [rsp]
.loop:
  ; find colon
  cmp   byte [rdi], COLON
  jne   .continue

  mov   rax, NULL_CHAR
  stosb

  jmp   .loop_end

.continue:
  cmp   byte [rdi], NULL_CHAR
  je    .error    ; wrong format

  inc   rdi

  jmp   .loop
.loop_end:

  ; rdi points to the port
  call  atoi
  cmp   rax, 0
  jl    .error

  xchg  al, ah
  mov   word [sockaddr_in_t.sin_port], ax

  mov   rdi, [rsp]
  mov   rsi, sockaddr_in_t.sin_addr
  call  inet_pton
  cmp   rax, 0
  jl    .error

  mov   word [sockaddr_in_t.sin_family], AF_INET
  mov   dword [sockaddr_in_t.sin_addr], 0
  mov   qword [sockaddr_in_t.sin_zero], 0

  ; try to connect to host
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  xor   rdx, rdx
  syscall
  cmp   rax, 0
  jl    .error

  mov   qword [host_fd], rax

  mov   rax, SYS_CONNECT
  mov   rdi, qword [host_fd]
  mov   rsi, sockaddr_in_t
  mov   rdx, qword [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret
