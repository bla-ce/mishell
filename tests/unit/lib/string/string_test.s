global _start

%include "lib.inc"

section .bss

section .data

string db "Hello, Sir!", NULL_CHAR
string_len equ $ - string - 1

string2 db "Hello, Sir! Again! 0", NULL_CHAR
string2_len equ $ - string2 - 1

section .text
_start:
  mov   rdi, string
  call  strlen
  cmp   rax, string_len
  jne   .error

  mov   rdi, string2
  call  strlen
  cmp   rax, string2_len
  jne   .error

  mov   rdi, 0
  call  strlen
  cmp   rax, FAILURE_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
