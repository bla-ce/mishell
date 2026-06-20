global _start

%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .rodata

section .bss

test_packet resb PACKET_T_LEN

section .text
_start:
  ; packet reset
  ; null packet ptr return failure
  xor   rdi, rdi
  call  packet_reset
  cmp   rax, FAILURE_CODE
  jne   .error

  ; happy path
  mov   rdi, test_packet
  call  packet_reset
  cmp   rax, SUCCESS_CODE
  jne   .error

  mov   rdi, test_packet
  xor   al, al
  mov   rcx, PACKET_T_LEN
  rep   scasb
  jne   .error

  ; packet verify
  ; null ptr return failure mode
  xor   rdi, rdi
  call  packet_verify
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; invalid magic value return INVALID_MAGIC
  mov   word [test_packet+PACKET_T_OFF_MAGIC], 0x1234

  mov   rdi, test_packet
  call  packet_verify
  cmp   rax, error_codes.INVALID_MAGIC
  jne   .error

  ; invalid mode flag should return INVALID_MODE
  mov   word [test_packet+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [test_packet+PACKET_T_OFF_FLAGS], 0x24
  mov   rdi, test_packet
  call  packet_verify
  cmp   rax, error_codes.INVALID_MODE
  jne   .error

  ; invalid op should return INVALID_OP
  mov   word [test_packet+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [test_packet+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [test_packet+PACKET_T_OFF_OP], req_ops.COUNT
  mov   rdi, test_packet
  call  packet_verify
  cmp   rax, error_codes.INVALID_OP
  jne   .error

  mov   word [test_packet+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [test_packet+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [test_packet+PACKET_T_OFF_OP], -1
  mov   rdi, test_packet
  call  packet_verify
  cmp   rax, error_codes.INVALID_OP
  jne   .error

  ; happy path
  mov   word [test_packet+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [test_packet+PACKET_T_OFF_FLAGS], FL_HOST
  mov   byte [test_packet+PACKET_T_OFF_OP], req_ops.HELLO
  mov   rdi, test_packet
  call  packet_verify
  cmp   rax, SUCCESS_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
