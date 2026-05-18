section .bss

service_type_t:
  .description  resb SERVICE_TYPE_DESC_MAX_LEN
  .commands     resb COMMANDS_LEN
  .type         resb 1
service_type_t_end:

section .data

SERVICE_TYPE_DESC_MAX_LEN equ 0x100

SERVICE_TYPE_T_LEN equ service_type_t_end - service_type_t

SERVICE_TYPE_T_OFF_DESC     equ 0x0
SERVICE_TYPE_T_OFF_COMMANDS equ SERVICE_TYPE_T_OFF_DESC + SERVICE_TYPE_DESC_MAX_LEN
SERVICE_TYPE_T_OFF_TYPE     equ SERVICE_TYPE_T_OFF_COMMANDS + COMMANDS_LEN

section .text
