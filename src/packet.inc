%include "errors.inc"

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

section .text

; handles a packet received from the connection fd
; @param  rdi: pointer to the packet struct
; @param  rsi: connection fd
; @return rax: return code
handle_packet:
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
  call  reset_packet
  cmp   rax, 0
  jl    .error

  test  rdi, rdi
  jz    .error

  ; verify packet
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x18]
  call  verify_packet
  mov   [rsp+0x10], rax ; save error code if it failed
  test  rax, rax
  jnz   .error

  mov   rax, 0
  jmp   .return

.error:
  lea   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  build_err_packet
  cmp   rax, 0
  jl    .skip_send

  mov   rdx, rax
  mov   rax, 1  ; WRITE
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
  syscall

.skip_send:
  mov   rax, -1

.return:
  add   rsp, 0x18 + PACKET_T_LEN
  ret

; verifies that the packet received is a valid packet
; @param  rdi: pointer to the packet struct
; @return rax: 0 if valid, err code if invalid
verify_packet:
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

  ; make sure the direction is client to server
  mov   qword [rsp+0x8], error_codes.INVALID_DIRECTION
  cmp   byte [rdi+PACKET_T_OFF_FLAGS], CLIENT_TO_SERVER
  jne   .error

  mov   qword [rsp+0x8], error_codes.INVALID_OP
  mov   al, byte [rdi+PACKET_T_OFF_OP]
  cmp   al, 0
  jle   .error
  cmp   al, ops.COUNT
  jge   .error

  mov   rax, 0
  jmp   .return

.error:
  mov   rax, [rsp+0x8]

.return:
  add   rsp, 0x10
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

; builds the error packet from the error code
; @param  rdi: pointer to the packet struct
; @param  rsi: error code
; @return rax: packet size
build_err_packet:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the packet struct
  ; [rsp+0x8] -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; set magic, op and flags
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], ops.ERROR
  mov   byte [rdi+PACKET_T_OFF_FLAGS], SERVER_TO_CLIENT

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
  mov   rax, -1

.return:
  add   rsp, 0x10
  ret
