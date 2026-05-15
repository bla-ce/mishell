global _start

%include "auth.inc"
%include "lib.inc"

section .text
_start:
  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall
