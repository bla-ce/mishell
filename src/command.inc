section .data

section .text
; returns the command from a service type
; @param  rdi: pointer to the service struct
; @param  rsi: OP byte
; @return rax: pointer to the command
command_get_by_op:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the service struct
  ; [rsp+0x8] -> OP byte

  mov   [rsp], rdi
  mov   qword [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jl    .error

  ; get service type pointer
  movzx rax, byte [rdi+SERVICE_T_OFF_TYPE]
  shl   rax, 3 ; rax *= 8
  mov   rdi, [service_types_table+rax]

  ; get service command
  movzx rdx, byte [rsp+0x8]
  shl   rdx, 3  ; rdx *= 8
  mov   rax, [rdi+rdx]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
