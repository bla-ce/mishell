section .bss

service_t:
  .ip     resb ID_LEN
  .name   resb SERVICE_NAME_MAX_LEN
  .type   resb 0x1
  .status resb 0x1
  .pad    resb 0x3
service_t_end:

section .data

SERVICE_MAX_COUNT_PER_HOST equ 0x5

SERVICE_NAME_MAX_LEN  equ 0xF

SERVICE_T_LEN equ service_t_end - service_t

SERVICE_T_OFF_ID      equ 0x0
SERVICE_T_OFF_NAME    equ SERVICE_T_OFF_ID + ID_LEN
SERVICE_T_OFF_TYPE    equ SERVICE_T_OFF_NAME + SERVICE_NAME_MAX_LEN
SERVICE_T_OFF_STATUS  equ SERVICE_T_OFF_TYPE + 0x1

section .text
