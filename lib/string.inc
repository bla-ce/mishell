section .rodata

UNSIGNED_WORD_STR_MAX_SIZE  equ 7

ITOA_STR_MAX_SIZE equ 22

section .text
; compares two strings
; @param  rdi: pointer to the first string
; @param  rsi: pointer to the second string
; @return rax: string are equal (boolean)
strcmp:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the first string
  ; [rsp+0x8] -> pointer to the second string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .not_equal

  test  rsi, rsi
  jz    .not_equal

.loop:
  mov   al, byte [rsi]
  cmp   byte [rdi], al
  jne   .not_equal

  test  al, al
  jz    .equal

  inc   rdi
  inc   rsi

  jmp   .loop

.equal:
  mov   rax, TRUE
  jmp   .return

.not_equal:
  mov   rax, FALSE

.return:
  add   rsp, 0x10
  ret

; returns the length of the given string
; @param  rdi: string
; @return rax: length of the string
strlen:
  test  rdi, rdi
  jz    .error

  xor   rax, rax

.loop:
  cmp   byte [rdi], NULL_CHAR
  je    .return

  inc   rdi
  inc   rax

  jmp   .loop

.error:
  mov   rax, FAILURE_CODE
  ret

.return:
  ret

; converts null-terminated string to an integer
; @param  rdi: string
; @return rax: int or -1 if error
atoi:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp+0x8]   -> number is negative
  ; [rsp+0x10]  -> length of the string
  ; [rsp+0x18]  -> int value

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x18], 0

  test  rdi, rdi
  jz    .error

  cmp   byte [rdi], HYPHEN
  jne   .pos_number

  mov   qword [rsp+0x8], 1
  inc   rdi

  mov   [rsp], rdi

.pos_number:
  call  strlen
  cmp   rax, 0
  jle   .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]

.next_char:
  xor   rax, rax
  mov   al, byte [rdi]

  ; make sure char is a digit
  cmp   al, ZERO_CHAR
  jl    .error

  mov   rdx, 0
  cmove rax, rdx
  je    .add

  cmp   al, NINE_CHAR
  jg    .error

  xor   rdx, rdx
  sub   rax, ZERO_CHAR ; get int

  mov   rcx, [rsp+0x10]
  dec   rcx

  cmp   rcx, 0
  jle   .add

.get_mult:
  imul  rax, 10

  cmp   rcx, 1
  jle   .add

  dec   rcx
  jmp   .get_mult

.add:
  add   [rsp+0x18], rax
  mov   r9, [rsp+0x18]
  inc   rdi

  dec   qword [rsp+0x10]
  cmp   qword [rsp+0x10], 0
  je    .done

  jmp   .next_char

.done:
  mov   rax, [rsp+0x18]
  mov   rdx, 0
  sub   rdx, rax

  ; check if it's a neg number
  cmp   qword [rsp+0x8], 1
  cmove rax, rdx

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; prints to the standard output the string in rdi
; @param  rdi: pointer to the string
; @return rax: return value
print:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the string to print

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  mov   rsi, [rsp]
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; prints to the std output the str followed by a new line char
; @param  rdi: string
; @return rax: return code
println:
  test  rdi, rdi
  jz    .error

  call  print
  cmp   rax, 0
  jl    .return

  mov   rdi, LINE_FEED
  call  print_char

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; prints a character to the standard output
; @param  rdi: character
print_char:
  push  rbp
  mov   rbp, rsp

  push  rdi

  xor   rsi, rsi

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  leave
  ret

; converts int in rdi into string
; string pointer needs to be passed
; make sure the size include the null character at the end of the string
; @param  rdi: int
; @param  rsi: string
; @param  rdx: size
; @return rax: pointer to str(int)
itoa:
  push  r12
  sub   rsp, 0x18+ITOA_STR_MAX_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the number to be converted
  ; [rsp+0x8]   -> pointer to the final string
  ; [rsp+0x10]  -> size of the string
  ; [rsp+0x18]  -> pointer to the temp string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; go to the end of the buf (print backward)
  lea   rdi, [rsp+0x18]
  add   rdi, [rsp+0x10]

  ; go back to let an extra null char at the end
  dec   rdi
  mov   byte [rdi], NULL_CHAR

  dec   rdi

  ; check if number is negative
  mov   rax, [rsp]
  cmp   rax, 0
  jg    .loop

  neg   rax

.loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rdi], dl

  dec   rdi

  cmp   rax, 0
  jne   .loop

.loop_end:
  cmp   qword [rsp], 0
  jge   .skip_hyphen

  mov   byte [rdi], HYPHEN
  dec   rdi

.skip_hyphen:
  inc   rdi

  ; get size of the string
  mov   r12, rdi
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, r12
  mov   rcx, rax
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+ITOA_STR_MAX_SIZE
  pop   r12
  ret

; catenates the string pointed to by rdi after the string pointed
; to by rsi. User is responsible for providing a destination
; buffer large enough
; @param  rdi: pointer to the destination
; @param  rsi: pointer to the source
; @return rax: pointer to the destination
strcat:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the destination
  ; [rsp+0x8]   -> pointer to the source
  ; [rsp+0x10]  -> pointer to the end of destination

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get length of dst
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  add   rdi, rax
  mov   [rsp+0x10], rdi

  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  mov   rax, [rsp]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret
