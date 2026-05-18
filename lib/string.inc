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
