%include "errors.inc"

section .bss

packet_t:
  .magic        resw 1
  .op           resb 1
  .flags        resb 1
  .id           resb ID_LEN
  .dest_host    resb ID_LEN
  .dest_service resb ID_LEN
  .payload_len  resw 1
  .payload      resb PAYLOAD_MAX_LEN
packet_t_end:

section .rodata

; PACKET OFFSETS
PACKET_T_LEN equ packet_t_end - packet_t
PACKET_T_HEADER_LEN equ packet_t.payload - packet_t

PACKET_T_OFF_MAGIC        equ 0x0
PACKET_T_OFF_OP           equ PACKET_T_OFF_MAGIC + 0x2
PACKET_T_OFF_FLAGS        equ PACKET_T_OFF_OP + 0x1
PACKET_T_OFF_ID           equ PACKET_T_OFF_FLAGS + 0x1
PACKET_T_OFF_DEST_HOST    equ PACKET_T_OFF_ID + ID_LEN
PACKET_T_OFF_DEST_SERVICE equ PACKET_T_OFF_DEST_HOST + ID_LEN
PACKET_T_OFF_PAYLOAD_LEN  equ PACKET_T_OFF_DEST_SERVICE + ID_LEN
PACKET_T_OFF_PAYLOAD      equ PACKET_T_OFF_PAYLOAD_LEN + 0x2

; MODE FLAGS
FL_USER     equ 0001b
FL_HOST     equ 0010b
FL_SERVICE  equ 0100b

MAGIC_VALUE     equ 0xCAFE
PAYLOAD_MAX_LEN equ 0xFFFF

PACKET_MAX_LEN  equ PACKET_T_HEADER_LEN + PAYLOAD_MAX_LEN

section .text

; handles a packet received from the connection fd
; @param  rdi: pointer to the packet struct
; @param  rsi: connection fd
; @return rax: return code
packet_dispatch:
  sub   rsp, 0x18 + PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the packet struct
  ; [rsp+0x8]   -> connection fd
  ; [rsp+0x10]  -> error code
  ; [rsp+0x18]  -> response packet

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], error_codes.INTERNAL  ; default

  ; reset response packet
  lea   rdi, [rsp+0x18]
  call  packet_reset
  cmp   rax, 0
  jl    .error

  test  rdi, rdi
  jz    .error

  ; verify packet
  mov   rdi, [rsp]
  call  packet_verify
  mov   [rsp+0x10], rax ; save error code if it failed
  test  rax, rax
  jnz   .error

  mov   qword [rsp+0x10], error_codes.INTERNAL  ; default
  ; check if we should handle the request or proxy it
  mov   rax, [rsp]
  movups  xmm1, [rax+PACKET_T_OFF_DEST_HOST]
  ptest   xmm1, xmm1
  jz    .handle_request

  mov   rdi, [rsp]
  add   rdi, PACKET_T_OFF_DEST_HOST
  mov   rsi, host_id
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .handle_request

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x18]
  call  host_proxy_request
  cmp   rax, 0
  jl    .error

  jmp   .send_packet

.handle_request:
  ; get op
  mov   rdi, [rsp]
  movzx rax, byte [rdi+PACKET_T_OFF_OP]
  shl   rax, 3 ; rax *= 8

  ; call op
  mov   qword [rsp+0x10], error_codes.INTERNAL ; back to default
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x18]
  call  qword [req_op_fn_table+rax]
  mov   [rsp+0x10], rax ; save error code if it failed
  test  rax, rax
  jnz   .error

.send_packet:
  ; get packet size
  lea   rdi, [rsp+0x18]
  movzx rdx, word [rdi+PACKET_T_OFF_PAYLOAD_LEN]
  add   rdx, PACKET_T_HEADER_LEN

  ; send response packet
  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
  ; rdx already populated
  syscall

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  lea   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  packet_build_error
  cmp   rax, 0
  jl    .skip_send

  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
  syscall

.skip_send:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18 + PACKET_T_LEN
  ret

; verifies that the packet received is a valid packet
; @param  rdi: pointer to the packet struct
; @return rax: 0 if valid, err code if invalid
packet_verify:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the packet struct
  ; [rsp+0x8] -> error code

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  test  rdi, rdi
  jz    .error

  ; verify magic value
  mov   qword [rsp+0x8], error_codes.INVALID_MAGIC
  cmp   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  jne   .error

  mov   al, byte [rdi+PACKET_T_OFF_FLAGS]

  ; at least one of FL_HOST or FL_USER must be set
  mov   qword [rsp+0x8], error_codes.INVALID_MODE
  mov   r9b, al
  and   r9b, FL_HOST
  test  r9b, r9b
  jnz   .check_op

  mov   r9b, al
  and   r9b, FL_USER
  test  r9b, r9b
  jz    .error

.check_op:
  mov   qword [rsp+0x8], error_codes.INVALID_OP
  movzx rax, byte [rdi+PACKET_T_OFF_OP]
  cmp   rax, 0
  jl    .error
  cmp   rax, req_ops.COUNT
  jge   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x8]

.return:
  add   rsp, 0x10
  ret

; resets a packet struct
; @param  rdi: pointer to the packet struct
; @return rax: return code
packet_reset:
  test  rdi, rdi
  jz    .error

  xor   rax, rax
  mov   rcx, PACKET_T_LEN
  rep   stosb

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; builds the error packet from the error code
; @param  rdi: pointer to the packet struct
; @param  rsi: error code
; @return rax: packet size
packet_build_error:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the packet struct
  ; [rsp+0x8] -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; memset packet
  mov   rdi, [rsp]
  xor   rax, rax
  mov   rcx, PACKET_T_LEN
  rep   stosb

  ; set magic, op, id and flags
  mov   rdi, [rsp]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.ERROR
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_HOST

  cmp   rsi, 0
  jl    .internal_error

  cmp   rsi, error_codes.COUNT
  jge   .internal_error

  ; get length from code
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  shl   rsi, 1    ; rsi *= 2
  mov   ax, [err_code_to_len_table+rsi]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ax
  movzx rdx, ax  ; save payload len

  ; get message from code
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  shl   rsi, 3    ; rsi *= 8
  mov   rsi, [err_code_to_msg_table+rsi]

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rcx, rdx
  rep   movsb

  mov   rax, rdx
  add   rax, PACKET_T_HEADER_LEN

  jmp   .return

.internal_error:
  ; build internal error
  mov   rdi, [rsp]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], error_msg.internal_len

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, error_msg.internal
  mov   rcx, error_msg.internal_len
  rep   movsb

  mov   rax, PACKET_T_HEADER_LEN + error_msg.internal_len
  jmp   .return

.error:
  ; something went wrong and the error packet should not be sent
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
