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

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
