section .bss

command_t:
  .name         resb COMMAND_NAME_MAX_LEN
  .description  resb COMMAND_DESC_MAX_LEN
  .fn           resq 0x1
command_t_end:

section .rodata

COMMAND_T_LEN equ command_t_end - command_t

COMMAND_T_OFF_NAME equ 0x0
COMMAND_T_OFF_DESC equ COMMAND_T_OFF_NAME + COMMAND_NAME_MAX_LEN
COMMAND_T_OFF_FN   equ COMMAND_T_OFF_DESC + COMMAND_DESC_MAX_LEN

COMMAND_NAME_MAX_LEN equ 0x20
COMMAND_DESC_MAX_LEN equ 0x100

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

  cmp   rsi, COMMANDS_MAX_COUNT_PER_SERVICE_TYPE
  jge   .error

  ; get service type pointer
  movzx rax, byte [rdi+SERVICE_T_OFF_TYPE]
  shl   rax, 3 ; rax *= 8
  mov   rdi, [service_types_table+rax]

  ; get service command
  movzx rdx, byte [rsp+0x8]
  shl   rdx, 3  ; rdx *= 8
  mov   rax, [rdi+rdx+SERVICE_TYPE_T_OFF_COMMANDS]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
