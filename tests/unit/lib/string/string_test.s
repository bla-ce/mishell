global _start

%include "lib.inc"

section .bss

section .data

str_itoa times ITOA_STR_MAX_SIZE db 0

; strlen test data
string db "Hello, Sir!", NULL_CHAR
string_len equ $ - string - 1

string2 db "Hello, Sir! Again! 0", NULL_CHAR
string2_len equ $ - string2 - 1

; atoi: positive numbers
atoi_str_0          db "0", NULL_CHAR
atoi_str_1          db "1", NULL_CHAR
atoi_str_9          db "9", NULL_CHAR
atoi_str_10         db "10", NULL_CHAR
atoi_str_42         db "42", NULL_CHAR
atoi_str_100        db "100", NULL_CHAR
atoi_str_123        db "123", NULL_CHAR
atoi_str_999        db "999", NULL_CHAR
atoi_str_1000       db "1000", NULL_CHAR
atoi_str_9999       db "9999", NULL_CHAR
atoi_str_65535      db "65535", NULL_CHAR
atoi_str_1000000    db "1000000", NULL_CHAR
atoi_str_large      db "1234567890", NULL_CHAR

; atoi: leading zeros
atoi_str_00         db "00", NULL_CHAR
atoi_str_01         db "01", NULL_CHAR
atoi_str_007        db "007", NULL_CHAR

; atoi: negative numbers
atoi_str_neg1       db "-1", NULL_CHAR
atoi_str_neg2       db "-2", NULL_CHAR
atoi_str_neg42      db "-42", NULL_CHAR
atoi_str_neg100     db "-100", NULL_CHAR
atoi_str_neg123     db "-123", NULL_CHAR
atoi_str_neg999     db "-999", NULL_CHAR
atoi_str_neg9999    db "-9999", NULL_CHAR
atoi_str_neg1mil    db "-1000000", NULL_CHAR

; atoi: error cases
atoi_str_empty      db NULL_CHAR
atoi_str_hyphen     db "-", NULL_CHAR
atoi_str_alpha      db "abc", NULL_CHAR
atoi_str_mixed_a    db "12a", NULL_CHAR
atoi_str_mixed_b    db "a12", NULL_CHAR
atoi_str_mid_alpha  db "1a2", NULL_CHAR
atoi_str_space_pre  db " 12", NULL_CHAR
atoi_str_space_post db "12 ", NULL_CHAR
atoi_str_dot        db "1.5", NULL_CHAR
atoi_str_plus       db "+1", NULL_CHAR
atoi_str_comma      db "1,234", NULL_CHAR
atoi_str_neg_alpha  db "-abc", NULL_CHAR
atoi_str_neg_mixed  db "-12a", NULL_CHAR

section .text
_start:
  ; --- strlen tests ---

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

  ; --- atoi: positive numbers ---

  mov   rdi, atoi_str_0
  call  atoi
  cmp   rax, 0
  jne   .error

  mov   rdi, atoi_str_1
  call  atoi
  cmp   rax, 1
  jne   .error

  mov   rdi, atoi_str_9
  call  atoi
  cmp   rax, 9
  jne   .error

  mov   rdi, atoi_str_10
  call  atoi
  cmp   rax, 10
  jne   .error

  mov   rdi, atoi_str_42
  call  atoi
  cmp   rax, 42
  jne   .error

  mov   rdi, atoi_str_100
  call  atoi
  cmp   rax, 100
  jne   .error

  mov   rdi, atoi_str_123
  call  atoi
  cmp   rax, 123
  jne   .error

  mov   rdi, atoi_str_999
  call  atoi
  cmp   rax, 999
  jne   .error

  mov   rdi, atoi_str_1000
  call  atoi
  cmp   rax, 1000
  jne   .error

  mov   rdi, atoi_str_9999
  call  atoi
  cmp   rax, 9999
  jne   .error

  mov   rdi, atoi_str_65535
  call  atoi
  cmp   rax, 65535
  jne   .error

  mov   rdi, atoi_str_1000000
  call  atoi
  cmp   rax, 1000000
  jne   .error

  mov   rdi, atoi_str_large
  call  atoi
  cmp   rax, 1234567890
  jne   .error

  ; --- atoi: leading zeros ---

  mov   rdi, atoi_str_00
  call  atoi
  cmp   rax, 0
  jne   .error

  mov   rdi, atoi_str_01
  call  atoi
  cmp   rax, 1
  jne   .error

  mov   rdi, atoi_str_007
  call  atoi
  cmp   rax, 7
  jne   .error

  ; --- atoi: negative numbers ---

  mov   rdi, atoi_str_neg1
  call  atoi
  cmp   rax, -1
  jne   .error

  mov   rdi, atoi_str_neg2
  call  atoi
  cmp   rax, -2
  jne   .error

  mov   rdi, atoi_str_neg42
  call  atoi
  cmp   rax, -42
  jne   .error

  mov   rdi, atoi_str_neg100
  call  atoi
  cmp   rax, -100
  jne   .error

  mov   rdi, atoi_str_neg123
  call  atoi
  cmp   rax, -123
  jne   .error

  mov   rdi, atoi_str_neg999
  call  atoi
  cmp   rax, -999
  jne   .error

  mov   rdi, atoi_str_neg9999
  call  atoi
  cmp   rax, -9999
  jne   .error

  mov   rdi, atoi_str_neg1mil
  call  atoi
  cmp   rax, -1000000
  jne   .error

  ; --- atoi: error cases ---

  ; null pointer
  mov   rdi, 0
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; empty string
  mov   rdi, atoi_str_empty
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; bare hyphen
  mov   rdi, atoi_str_hyphen
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; alphabetic string
  mov   rdi, atoi_str_alpha
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; digits then letter
  mov   rdi, atoi_str_mixed_a
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; letter then digits
  mov   rdi, atoi_str_mixed_b
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; letter in the middle
  mov   rdi, atoi_str_mid_alpha
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; leading space
  mov   rdi, atoi_str_space_pre
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; trailing space
  mov   rdi, atoi_str_space_post
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; decimal point
  mov   rdi, atoi_str_dot
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; plus sign (unsupported)
  mov   rdi, atoi_str_plus
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; comma separator
  mov   rdi, atoi_str_comma
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; negative sign then letters
  mov   rdi, atoi_str_neg_alpha
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; negative sign then digits then letter
  mov   rdi, atoi_str_neg_mixed
  call  atoi
  cmp   rax, FAILURE_CODE
  jne   .error

  ; --- strcmp ---
  ; equal strings return TRUE
  mov   rdi, string
  mov   rsi, string
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  ; null pointers return false
  xor   rdi, rdi
  mov   rsi, string
  call  strcmp
  cmp   rax, FALSE
  jne   .error

  mov   rdi, string
  xor   rsi, rsi
  call  strcmp
  cmp   rax, FALSE
  jne   .error

  ; non equal strings return false
  mov   rdi, string
  mov   rsi, string2
  call  strcmp
  cmp   rax, FALSE
  jne   .error

  ; --- itoa: positive numbers ---
  mov   rdi, 0
  mov   rsi, str_itoa
  mov   rdx, 1
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_0
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 1
  mov   rsi, str_itoa
  mov   rdx, 1
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_1
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 9
  mov   rsi, str_itoa
  mov   rdx, 1
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_9
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 10
  mov   rsi, str_itoa
  mov   rdx, 2
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_10
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 42
  mov   rsi, str_itoa
  mov   rdx, 2
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_42
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 100
  mov   rsi, str_itoa
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_100
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 123
  mov   rsi, str_itoa
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_123
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, 65535
  mov   rsi, str_itoa
  mov   rdx, 5
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_65535
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  ; --- itoa: negative numbers ---

  mov   rdi, -1
  mov   rsi, str_itoa
  mov   rdx, 2
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_neg1
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, -42
  mov   rsi, str_itoa
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_neg42
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  mov   rdi, -123
  mov   rsi, str_itoa
  mov   rdx, 4
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, atoi_str_neg123
  mov   rsi, str_itoa
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  ; --- atoi: error cases ---

  ; empty string
  mov   rdi, 42
  xor   rsi, rsi
  mov   rdx, 3
  call  itoa
  cmp   rax, FAILURE_CODE
  jne   .error

  ; empty size
  mov   rdi, 42
  mov   rsi, str_itoa
  mov   rdx, 0
  call  itoa
  cmp   rax, FAILURE_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
