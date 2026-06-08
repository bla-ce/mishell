global _start

%include "command.inc"
%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .rodata

host_port  equ 7474
host_ip   equ 0

usage_str     db "usage: mishell init", LINE_FEED
              db "       mishell connect <ip> <port>", LINE_FEED
usage_str_len equ $ - usage_str

section .data

host_ptr dq 0

section .bss

test_req_packet resb PACKET_T_LEN
test_res_packet resb PACKET_T_LEN

section .text
_start:
  ; --- op_hello ---
  ; empty request packet should return INTERNAL error
  xor   rdi, rdi
  mov   rsi, test_res_packet
  call  op_hello
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; empty response packet should return INTERNAL error
  mov   rdi, test_req_packet
  xor   rsi, rsi
  call  op_hello
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; happy path - no need to populate anything
  mov   rdi, test_req_packet
  mov   rsi, test_res_packet
  call  op_hello
  test  rax, rax
  jnz   .error

  mov   rdi, test_res_packet
  cmp   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  jne   .error
  cmp   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error
  cmp   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
