section .bss

host_t:
  .id   resb ID_LEN
  .ip   resb IP_MAX_LEN
host_t_end:

hosts resb HOST_MAX_LEN * HOST_T_LEN

section .data

HOST_MAX_LEN equ 0x8

IP_MAX_LEN   equ 0x10

curr_host_idx db 0

HOST_T_LEN equ host_t_end - host_t

HOST_T_OFF_ID   equ 0x0
HOST_T_OFF_IP   equ HOST_T_OFF_ID + ID_LEN
