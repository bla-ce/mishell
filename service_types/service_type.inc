section .rodata

SERVICE_TYPE_T_OFF_NAME     equ 0x0
SERVICE_TYPE_T_OFF_COMMANDS equ SERVICE_TYPE_T_OFF_NAME + SERVICE_TYPE_NAME_MAX_LEN
SERVICE_TYPE_T_OFF_DESC     equ SERVICE_TYPE_T_OFF_COMMANDS + COMMANDS_MAX_COUNT_PER_SERVICE_TYPE * 0x8

SERVICE_TYPE_DESC_MAX_LEN equ 0x100
SERVICE_TYPE_NAME_MAX_LEN equ 0x10

COMMANDS_MAX_COUNT_PER_SERVICE_TYPE equ 0x10

service_types_str:
  .PING   db "PING", NULL_CHAR

service_types_str_table:
  .PING   dq service_types_str.PING

service_types:
  .PING     equ 0x0
  .COUNT    equ 0x1

service_types_table:
  .ping     dq  ping_service_t

SERVICE_TYPE_T_LEN equ service_type_t_end - service_type_t

MAX_SERVICE_TYPES equ 0xA - 1 ; service types are 0-indexed

; trick to make sure we have not exceeded service type count
times MAX_SERVICE_TYPES - service_types.COUNT db 0

section .bss

service_type_t:
  .name         resb SERVICE_TYPE_NAME_MAX_LEN
  .commands     resq COMMANDS_MAX_COUNT_PER_SERVICE_TYPE
  .description  resb SERVICE_TYPE_DESC_MAX_LEN
service_type_t_end:

section .text

%include "ping_service.s"

; returns the service type number from a string
; @param  rdi: pointer to the string
; @return rax: service type number
service_type_from_str:
  push  r12
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the string

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  xor   r12, r12

.loop:
  cmp   r12, service_types.COUNT
  jge   .error

  ; get service type pointer
  mov   rdi, [rsp]
  add   rdi, SERVICE_TYPE_T_OFF_NAME
  mov   rsi, qword [service_types_str_table+r12*8]
  call  strcmp
  cmp   rax, TRUE
  je    .loop_end

  inc   r12
  jmp   .loop

.loop_end:
  mov   rax, r12
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  pop   r12
  ret
