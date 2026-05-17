global _start

%include "lib.inc"

section .bss

itoa_str resb ITOA_MAX_LEN

section .data

string db "Hello, Sir!", NULL_CHAR
string_len equ $ - string - 1

string2 db "Hello, Sir! Again! 0", NULL_CHAR
string2_len equ $ - string2 - 1

str_17      db "17", NULL_CHAR
str_17_len  equ $ - str_17

str_409820      db "409820", NULL_CHAR
str_409820_len  equ $ - str_409820

str_minus_18      db "-18", NULL_CHAR
str_minus_18_len  equ $ - str_minus_18

str_minus_1       db "-1", NULL_CHAR
str_minus_1_len   equ $ - str_minus_1

str_1     db "1", NULL_CHAR
str_1_len equ $ - str_1

str_0     db "0", NULL_CHAR
str_0_len equ $ - str_0

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

  mov   rdi, 17
  mov   rsi, itoa_str
  mov   rdx, ITOA_MAX_LEN
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, str_17
  mov   rsi, rax
  mov   rcx, str_17_len
  rep   cmpsb
  jne   .error

  mov   rdi, 409820
  mov   rsi, itoa_str
  mov   rdx, ITOA_MAX_LEN
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, str_409820
  mov   rsi, rax
  mov   rcx, str_409820_len
  rep   cmpsb
  jne   .error

  mov   rdi, -18
  mov   rsi, itoa_str
  mov   rdx, ITOA_MAX_LEN
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, str_minus_18
  mov   rsi, rax
  mov   rcx, str_minus_18_len
  rep   cmpsb
  jne   .error

  mov   rdi, -1
  mov   rsi, itoa_str
  mov   rdx, ITOA_MAX_LEN
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, str_minus_1
  mov   rsi, rax
  mov   rcx, str_minus_1_len
  rep   cmpsb
  jne   .error

  mov   rdi, 1
  mov   rsi, itoa_str
  mov   rdx, ITOA_MAX_LEN
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, str_1
  mov   rsi, rax
  mov   rcx, str_1_len
  rep   cmpsb
  jne   .error

  mov   rdi, 0
  mov   rsi, itoa_str
  mov   rdx, ITOA_MAX_LEN
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, str_0
  mov   rsi, rax
  mov   rcx, str_0_len
  rep   cmpsb
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
