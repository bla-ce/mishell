section .bss

service_type_t:
  .commands     resq COMMANDS_MAX_COUNT_PER_SERVICE_TYPE
  .type         resb 1
  .description  resb SERVICE_TYPE_DESC_MAX_LEN
service_type_t_end:

section .data

SERVICE_TYPE_T_LEN equ service_type_t_end - service_type_t

SERVICE_TYPE_T_OFF_COMMANDS equ 0x0
SERVICE_TYPE_T_OFF_TYPE     equ SERVICE_TYPE_T_OFF_COMMANDS + COMMANDS_MAX_COUNT_PER_SERVICE_TYPE * 0x8
SERVICE_TYPE_T_OFF_DESC     equ SERVICE_TYPE_T_OFF_TYPE + 0x1

SERVICE_TYPE_DESC_MAX_LEN equ 0x100

COMMANDS_MAX_COUNT_PER_SERVICE_TYPE equ 0x4

service_types:
  .PING     equ 0x0
  .STORAGE  equ 0x1
  .COUNT    equ 0x2

service_types_table:
  .ping     dq  ping_service_t
  .storage  dq  storage_service_t

section .text

%include "ping_service.s"
%include "storage_service.s"
