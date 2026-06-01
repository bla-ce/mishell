section .data

section .text
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

; converts string to an integer
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
