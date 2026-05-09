section .data

req_ops:
  .HELLO    equ 0x0
  .COUNT    equ 0x1

res_ops:
  .WELCOME  equ 0x0
  .ERROR    equ 0x1
  .COUNT    equ 0x2

req_op_fn_table:
  .HELLO  dq op_HELLO

section .text

; sends back a WELCOME op with an empty payload for now
; @param  rdi: pointer to the response packet
; @return rax: packet size
op_HELLO:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the response packet

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.WELCOME
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    byte [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0

  mov   rax, PACKET_T_HEADER_LEN

  jmp   .return

.error:
  mov   rax, -1

.return:
  add   rsp, 0x8
  ret
