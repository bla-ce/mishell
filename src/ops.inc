%include "auth.inc"

section .data

req_ops:
  .HELLO    equ 0x0
  .AUTH     equ 0x1
  .COUNT    equ 0x2

res_ops:
  .WELCOME  equ 0x0
  .AUTH_OK  equ 0x1
  .ERROR    equ 0x2
  .COUNT    equ 0x3

req_op_fn_table:
  .HELLO  dq op_hello
  .AUTH   dq op_auth

section .text

; sends back a WELCOME op with an empty payload for now
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_hello:
  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   word [rsi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rsi+PACKET_T_OFF_OP], res_ops.WELCOME
  mov   byte [rsi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    byte [rsi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rsi+PACKET_T_OFF_ID], 0
  mov   qword [rsi+PACKET_T_OFF_ID+0x8], 0
  mov   word [rsi+PACKET_T_OFF_PAYLOAD_LEN], 0

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; checks payload to see if host has been registered, otherwise
; creates a session id and adds it to the host array
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_auth:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd
  ; [rsp+0x10]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; build base packet
  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    byte [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0

  ; check if id is set
  mov   rdi, [rsp]
  movups  xmm1, [rdi+PACKET_T_OFF_ID]
  ptest   xmm1, xmm1
  jz    .new_host

  ; check if host is in the array
  xor   r9, r9
.loop:
  lea   rdi, [hosts+r9]

  mov   rsi, [rsp]
  lea   rsi, [rsi+PACKET_T_OFF_ID]
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .loop_end

  add   r9, ID_LEN

  movzx rax, byte [curr_host_idx]
  shl   rax, 4  ; rax *= 16 (ID_LEN)

  mov   qword [rsp+0x18], error_codes.INVALID_TOKEN
  cmp   r9, rax
  jge   .error

  jmp   .loop
.loop_end:
  ; valid host id
  mov   rdi, [rsp+0x8]
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.AUTH_OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  mov   qword [rdi+PACKET_T_OFF_PAYLOAD], 0

  mov   rax, SUCCESS_CODE

  jmp   .return

.new_host:
  ; make sure we haven't reached the host limit
  mov   qword [rsp+0x18], error_codes.HOST_LIMIT_REACHED
  cmp   byte [curr_host_idx], HOST_MAX_LEN
  jge   .error


  ; build response packet
  mov   rdi, [rsp+0x8]
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.AUTH_OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ID_LEN
  mov   qword [rdi+PACKET_T_OFF_PAYLOAD], rdx
  mov   qword [rdi+PACKET_T_OFF_PAYLOAD+0x8], rax

  ; add session id to host array
  movzx rdi, byte [curr_host_idx]
  shl   rdi, 4  ; rdi *= 16
  mov   [hosts+rdi], rdx
  mov   [hosts+rdi+0x8], rax

  inc   byte [curr_host_idx]

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, [rsp+0x18]

.return:
  add   rsp, 0x20
  ret
