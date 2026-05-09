section .data

ops:
  .HELLO    equ 1
  .WELCOME  equ 2
  .AUTH     equ 3
  .AUTH_OK  equ 4
  .COUNT    equ 5

ops_table:
  .HELLO    dq op_HELLO
  .WELCOME  dq op_WELCOME
  .AUTH     dq op_AUTH
  .AUTH_OK  dq op_AUTH_OK

section .text

op_HELLO:
  ret

op_WELCOME:
  ret

op_AUTH:
  ret

op_AUTH_OK:
  ret
