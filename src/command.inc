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
; @param  rsi: pointer to the command name
; @return rax: pointer to the command
command_get_by_op:
  push  r12
  push  r13
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the service struct
  ; [rsp+0x8]   -> pointer to the command name
  ; [rsp+0x10]  -> pointer to the command being tested

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jl    .error

  ; get service type pointer
  movzx rax, byte [rdi+SERVICE_T_OFF_TYPE]
  shl   rax, 3 ; rax *= 8
  mov   r12, [service_types_table+rax]

  ; save pointer to service type commands
  add   r12, SERVICE_TYPE_T_OFF_COMMANDS

  xor   r13, r13

  ; compare query name
.loop:
  cmp   r13, COMMANDS_MAX_COUNT_PER_SERVICE_TYPE
  jge   .error

  ; get pointer to the command
  mov   rax, r13
  shl   rax, 3

  mov   rdi, [r12+rax]
  mov   [rsp+0x10], rdi

  add   rdi, COMMAND_T_OFF_NAME
  mov   rsi, [rsp+0x8]
  call  strcmp
  cmp   rax, TRUE
  je    .loop_end

  inc   r13
  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  pop   r13
  pop   r12
  ret
