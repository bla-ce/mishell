global _start

%include "lib.inc"

section .data

string      db "Hello, sir!"
string_len  equ $ - string

section .text
_start:
  mov   rdi, string
  mov   rsi, string_len
  mov   dl, 'H'
  call  find_next_char
  cmp   rax, 0
  jne   .error

  mov   rdi, string
  mov   rsi, string_len
  mov   dl, '!'
  call  find_next_char
  cmp   rax, string_len-1
  jne   .error

  mov   rdi, string
  mov   rsi, string_len
  mov   dl, ','
  call  find_next_char
  cmp   rax, 5
  jne   .error

  mov   rdi, string
  mov   rsi, string_len
  mov   dl, 't'
  call  find_next_char
  cmp   rax, FAILURE_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
