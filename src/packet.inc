section .bss

packet_t:
  .magic        resw 1
  .op           resb 1
  .flags        resb 1  ; message direction
  .payload_len  resw 1
  .payload      resb PAYLOAD_MAX_LEN
packet_t_end:

packet_len resq 1

section .data

; PACKET OFFSETS
PACKET_T_LEN equ packet_t_end - packet_t

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

section .text

; handles a packet received from the connection fd
; @param  rdi: pointer to the packet struct
; @param  rsi: connection fd
; @return rax: return code
handle_packet:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the packet struct
  ; [rsp+0x8] -> connection fd

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; verify packet
  mov   rdi, [rsp]
  call  verify_packet
  cmp   rax, 0
  jl    .error

  mov   rax, 0
  jmp   .return

.error:
  mov   rax, -1

.return:
  add   rsp, 0x10
  ret

; verifies that the packet received is a valid packet
; @param  rdi: pointer to the packet struct
; @return rax:
  ; 0: packet is valid
  ; -1: packet is invalid
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
