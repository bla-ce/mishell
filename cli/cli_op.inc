section .rodata

req_op_cli_table:
  .HELLO      dq cli_hello
  .REGISTER   dq cli_register
  .START      dq cli_start
  .STOP       dq cli_stop
  .UNREGISTER dq cli_unregister
  .CATALOG    dq cli_catalog
  .LIST       dq cli_list

req_op_str:
  .HELLO      db "HELLO", NULL_CHAR
  .REGISTER   db "REGISTER", NULL_CHAR
  .START      db "START", NULL_CHAR
  .STOP       db "STOP", NULL_CHAR
  .UNREGISTER db "UNREGISTER", NULL_CHAR
  .CATALOG    db "CATALOG", NULL_CHAR
  .LIST       db "LIST", NULL_CHAR

res_ok      db "OK", NULL_CHAR

res_error     db "ERROR: ", NULL_CHAR

req_op_str_table:
  .HELLO      dq req_op_str.HELLO
  .REGISTER   dq req_op_str.REGISTER
  .START      dq req_op_str.START
  .STOP       dq req_op_str.STOP
  .UNREGISTER dq req_op_str.UNREGISTER
  .CATALOG    dq req_op_str.CATALOG
  .LIST       dq req_op_str.LIST

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

; sends hello packet to host
cli_hello:
  mov   byte [packet_t.op], req_ops.HELLO
  ; rest of the fields are already 0

  call  cli_send_and_recv_packet
  cmp   rax, 0
  jl    .error

  ; check op
  cmp   byte [packet_t.op], res_ops.OK
  jne   .error

  ; print ok
  mov   rdi, res_ok
  call  print

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  call  cli_print_error

.return:
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

; gets the catalog of service types and prints them
cli_catalog:
  ret

; sends a packet from packet_t and receives the response
; overwritting packet_t
cli_send_and_recv_packet:
  ; send packet
  mov   rax, SYS_WRITE
  mov   rdi, [host_fd]
  mov   rsi, packet_t
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rax, SYS_READ
  mov   rdi, [host_fd]
  mov   rsi, packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; print ERROR and the error message from payload
cli_print_error:
  mov   rdi, res_error
  call  print

  mov   rdi, packet_t.payload
  call  println
  ret
