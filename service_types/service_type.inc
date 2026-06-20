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

section .bss

service_type_t:
  .name         resb SERVICE_TYPE_NAME_MAX_LEN
  .commands     resq COMMANDS_MAX_COUNT_PER_SERVICE_TYPE
  .description  resb SERVICE_TYPE_DESC_MAX_LEN
service_type_t_end:

section .text

%include "ping_service.s"
