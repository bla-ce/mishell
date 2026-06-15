section .rodata

req_op_cli_table:
  .HELLO      dq cli_hello
  .AUTH       dq cli_auth
  .REGISTER   dq cli_register
  .START      dq cli_start
  .STOP       dq cli_stop
  .UNREGISTER dq cli_unregister
  .LIST       dq cli_list
  .UPDATE     dq cli_update

req_op_str:
  .HELLO      db "HELLO", NULL_CHAR
  .AUTH       db "AUTH", NULL_CHAR
  .REGISTER   db "REGISTER", NULL_CHAR
  .START      db "START", NULL_CHAR
  .STOP       db "STOP", NULL_CHAR
  .UNREGISTER db "UNREGISTER", NULL_CHAR
  .LIST       db "LIST", NULL_CHAR
  .UPDATE     db "UPDATE", NULL_CHAR

req_op_str_table:
  .HELLO      dq req_op_str.HELLO
  .AUTH       dq req_op_str.AUTH
  .REGISTER   dq req_op_str.REGISTER
  .START      dq req_op_str.START
  .STOP       dq req_op_str.STOP
  .UNREGISTER dq req_op_str.UNREGISTER
  .LIST       dq req_op_str.LIST
  .UPDATE     dq req_op_str.UPDATE

section .text

; returns the op value from a string
; @param  rdi: pointer to the string
; @return rax: op value
mishli_cli_op_fn_from_str:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the string

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  xor   r9, r9

.loop:
  cmp   r9, req_ops.COUNT
  jge   .error

  mov   rdi, [rsp]
  mov   rsi, qword [req_op_str_table+r9*0x8]
  call  strcmp
  cmp   rax, TRUE
  je    .done

  inc   r9
  jmp   .loop

.done:
  shl   r9, 3   ; r9 *= 8

  ; get cli function from op
  mov   rax, [req_op_cli_table+r9]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

cli_hello:
  ret
cli_auth:
  ret
cli_register:
  ret
cli_unregister:
  ret
cli_start:
  ret
cli_stop:
  ret
cli_list:
  ret
cli_update:
  ret
