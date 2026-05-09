section .bss

packet_t:
  .magic        resw 1
  .op           resb 1
  .flags        resb 1  ; message direction
  .payload_len  resw 1
  .payload      resb PAYLOAD_MAX_LEN
packet_t_end:

section .data

; PACKET OFFSETS
PACKET_T_LEN equ packet_t_end - packet_t
PACKET_T_HEADER_LEN equ packet_t.payload - packet_t

PACKET_T_OFF_MAGIC        equ 0x0
PACKET_T_OFF_OP           equ PACKET_T_OFF_MAGIC + 0x2
PACKET_T_OFF_FLAGS        equ PACKET_T_OFF_OP + 0x1
PACKET_T_OFF_PAYLOAD_LEN  equ PACKET_T_OFF_FLAGS + 0x1
PACKET_T_OFF_PAYLOAD      equ PACKET_T_OFF_PAYLOAD_LEN + 0x2

; FLAGS
CLIENT_TO_SERVER equ 0
SERVER_TO_CLIENT equ 1

HEADER_LEN      equ 0x5
MAGIC_VALUE     equ 0xCAFE
PAYLOAD_MAX_LEN equ 0xFFFF

PACKET_MAX_LEN  equ HEADER_LEN + PAYLOAD_MAX_LEN

err_msg:
  .internal db "internal error"
  .internal_len equ $ - err_msg.internal

  .invalid_magic db "invalid magic value in packet"
  .invalid_magic_len equ $ - err_msg.invalid_magic
  .invalid_op db "invalid op value in packet"
  .invalid_op_len equ $ - err_msg.invalid_op
  .invalid_direction db "invalid direction flag in packet"
  .invalid_direction_len equ $ - err_msg.invalid_direction

section .text

; handles a packet received from the connection fd
; @param  rdi: pointer to the packet struct
; @param  rsi: connection fd
; @return rax: return code
handle_packet:
  sub   rsp, 0x10 + PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the packet struct
  ; [rsp+0x8]   -> connection fd
  ; [rsp+0x10]  -> response packet

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; reset response packet
  lea   rdi, [rsp+0x10]
  call  reset_packet
  cmp   rax, 0
  jl    .error

  test  rdi, rdi
  jz    .error

  ; verify packet
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x10]
  call  verify_packet
  test  rax, rax
  jnz   .error

  mov   rax, 0
  jmp   .return

.error:
  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], ops.ERROR
  mov   byte [rdi+PACKET_T_OFF_FLAGS], SERVER_TO_CLIENT
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], err_msg.internal_len

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, err_msg.internal
  mov   rcx, err_msg.internal_len
  rep   movsb

  mov   rax, 1  ; WRITE
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x10]
  mov   rdx, PACKET_T_HEADER_LEN + err_msg.internal_len
  syscall

  mov   rax, -1

.return:
  add   rsp, 0x10 + PACKET_T_LEN
  ret

; verifies that the packet received is a valid packet
; @param  rdi: pointer to the packet struct
; @return rax: 0 if valid, err code if invalid
verify_packet:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; verify magic value
  cmp   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  jne   .error

  ; make sure the direction is client to server
  cmp   byte [rdi+PACKET_T_OFF_FLAGS], CLIENT_TO_SERVER
  jne   .error

  movzx rax, byte [rdi+PACKET_T_OFF_OP]
  cmp   rax, 0
  jle   .error
  cmp   rax, ops.COUNT
  jge   .error

  mov   rax, 0
  jmp   .return

.error:
  mov   rax, -1

.return:
  add   rsp, 0x8
  ret

; resets a packet struct
; @param  rdi: pointer to the packet struct
; @return rax: return code
reset_packet:
  test  rdi, rdi
  jz    .error

  xor   rax, rax
  mov   rcx, PACKET_T_LEN
  rep   stosb

  mov   rax, 0
  jmp   .return

.error:
  mov   rax, -1

.return:
  ret
