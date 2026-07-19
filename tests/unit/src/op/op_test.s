global _start

%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "logs.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"
%include "state.inc"

section .rodata

section .bss

test_host       resb HOST_T_LEN
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
