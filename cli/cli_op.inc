section .rodata

req_op_cli_table:
  .HELLO      dq cli_hello
  .JOIN       dq cli_join
  .REGISTER   dq cli_register
  .START      dq cli_start
  .STOP       dq cli_stop
  .UNREGISTER dq cli_unregister
  .NETWORK    dq cli_network
  .UPDATE     dq cli_update
  .QUERY      dq cli_query
  .CATALOG    dq cli_catalog

req_op_str:
  .HELLO      db "HELLO", NULL_CHAR
  .JOIN       db "JOIN", NULL_CHAR
  .REGISTER   db "REGISTER", NULL_CHAR
  .START      db "START", NULL_CHAR
  .STOP       db "STOP", NULL_CHAR
  .UNREGISTER db "UNREGISTER", NULL_CHAR
  .NETWORK    db "NETWORK", NULL_CHAR
  .UPDATE     dq "UPDATE", NULL_CHAR
  .QUERY      dq "QUERY", NULL_CHAR
  .CATALOG    db "CATALOG", NULL_CHAR

res_ok    db "OK", NULL_CHAR
res_error db "ERROR: ", NULL_CHAR

req_op_str_table:
  .HELLO      dq req_op_str.HELLO
  .JOIN       dq req_op_str.JOIN
  .REGISTER   dq req_op_str.REGISTER
  .START      dq req_op_str.START
  .STOP       dq req_op_str.STOP
  .UNREGISTER dq req_op_str.UNREGISTER
  .NETWORK    dq req_op_str.NETWORK
  .UPDATE     dq req_op_str.UPDATE
  .QUERY      dq req_op_str.QUERY
  .CATALOG    dq req_op_str.CATALOG

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

; sends HELLO command to remote host
; @return rax: return code
cli_hello:
  sub   rsp, PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet (req and resp)

  mov   word [rsp+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rsp+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rsp+PACKET_T_OFF_DEST_HOST], 0
  mov   byte [rsp+PACKET_T_OFF_OP], req_ops.HELLO
  mov   word [rsp+PACKET_T_OFF_PAYLOAD_LEN], 0

  lea   rdi, [rsp]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print ok
  mov   rdi, res_ok
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, PACKET_T_LEN
  ret

cli_register:
  ret

cli_unregister:
  ret

cli_start:
  ret

cli_stop:
  ret

cli_catalog:
  ret

cli_network:
  ret

cli_join:
  ret

cli_update:
  ret

cli_query:
  ret

; sends a packet to remote host and get the response back
; the request packet is being overwritten by the response
; @param  rdi: pointer to the packet struct
; @return rax: return code
cli_transaction:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .no_print

  ; get payload length
  movzx rdx, word [rdi+PACKET_T_OFF_PAYLOAD_LEN]
  add   rdx, PACKET_T_HEADER_LEN

  ; send response packet
  mov   rax, SYS_WRITE
  mov   rdi, [host_fd]
  mov   rsi, [rsp]
  ; rdx already populated
  syscall
  cmp   rax, 0
  jl    .no_print

  ; get response
  mov   rax, SYS_READ
  mov   rdi, [host_fd]
  mov   rsi, [rsp]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .no_print

  ; check response op
  mov   rdi, [rsp]
  cmp   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp]
  call  cli_print_error

.no_print:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; print ERROR and the error message from payload
; @param  rdi: pointer to the packet
; @return rax: return code
cli_print_error:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  mov   rdi, res_error
  call  print

  mov   rdi, [rsp]
  add   rdi, PACKET_T_OFF_PAYLOAD
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret
