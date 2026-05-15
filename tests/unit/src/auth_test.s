global _start

%include "auth.inc"
%include "lib.inc"

section .text
_start:
  ; rand_generate_number should generate a number without error
  call  rand_generate_number
  cmp   rdx, SUCCESS_CODE
  jne   .error

  ; generate_id should generate a 16 bytes id without error
  call  generate_id
  cmp   rax, SUCCESS_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rax, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
