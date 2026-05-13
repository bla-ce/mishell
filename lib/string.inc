section .data

ITOA_MAX_LEN equ 22

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

; converts int in rdi into string. string ptr needs to be passed
; make sure the size include the null character at the end of the
; string
; @param  rdi: int
; @param  rsi: string
; @param  rdx: size
; @return rax: pointer to str(int)
itoa:
  sub   rsp, 0x18
  sub   rsp, ITOA_MAX_LEN

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

  cmp   rdx, 0
  jl    .error

  ; go to the end of the buf (print backward)
  lea   rdi, [rsp+0x18]
  add   rdi, qword [rsp+0x10]

  ; go back to let an extra null char at the end
  dec   rdi
  mov   byte [rdi], NULL_CHAR

  dec   rdi

  mov   rax, qword [rsp]
  cmp   rax, 0
  jg    .loop

  neg   rax

.loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR ; 0 character

  mov   byte [rdi], dl

  dec   rdi

  cmp   rax, 0
  jne   .loop

.loop_end:
  cmp   qword [rsp], 0
  jge   .skip_hyphen

  mov   byte [rdi], HYPHEN  ; - character
  dec   rdi

.skip_hyphen:
  inc   rdi

  ; copy temp to final string
  mov   rsi, rdi
  mov   rdi, [rsp+0x8]
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add null char
  mov   rdi, rax
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, ITOA_MAX_LEN
  add   rsp, 0x18
  ret

; copy n bytes of the string pointed to by src,
; into the string pointed to by dst
; @param  rdi: dst
; @param  rsi: src
; @param  rdx: n bytes
; @return rax: pointer to dst after copy
strncpy:
  mov   rcx, rdx
  rep   movsb

  mov   rax, rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; copy the string pointed to by src, into a string at the
; buffer pointed to by dst
; @param  rdi: dst
; @param  rsi: src
; @return rax: pointer to dst after copy
strcpy:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the destination string
  ; [rsp+0x8] -> pointer to the source string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   rdi, rsi
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, rax
  call  strncpy

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
