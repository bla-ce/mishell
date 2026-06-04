global _start

%include "command.inc"
%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .rodata

; TODO: rename for consistency
tcp_port  equ 7474
host_ip   equ 0

usage_str     db "usage: mishell init", LINE_FEED
              db "       mishell connect <ip> <port>", LINE_FEED
usage_str_len equ $ - usage_str

section .text
_start:
  ; --- host_addr_exists ---
  ; test without hosts
  mov   rdi, 0
  mov   rsi, 7474
  call  host_addr_exists
  cmp   rax, FALSE
  jne   .error

  ; test without duplicate host
  ; add three dummy host
  mov   r12, 0
  mov   r13, 7474

.loop:
  cmp   r12, 5
  jge   .loop_end

  imul  rax, r12, HOST_T_LEN
  lea   rsi, [hosts+rax]
  mov   dword [rsi+HOST_T_OFF_IP], 0
  mov   word [rsi+HOST_T_OFF_PORT], r13w

  inc   r13
  inc   r12

  jmp   .loop
.loop_end:
  mov   byte [curr_host_idx], 5

  mov   rdi, 0
  mov   rsi, 7000
  call  host_addr_exists
  cmp   rax, FALSE
  jne   .error

  ; test with duplicate host
  mov   rdi, 0
  mov   rsi, 7476
  call  host_addr_exists
  cmp   rax, TRUE
  jne   .error

  ; test with last host
  mov   rdi, 0
  mov   rsi, 7478
  call  host_addr_exists
  cmp   rax, TRUE
  jne   .error

  ; empty port should return failure
  mov   rdi, 0
  mov   rsi, 0
  call  host_addr_exists
  cmp   rax, FAILURE_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
