section .rodata

ping_service_t:
  .name         db "PING", NULL_CHAR
                times SERVICE_TYPE_NAME_MAX_LEN - ($ - ping_service_t.name) db 0
  .commands     dq ping_cmd_ping_t
                times (COMMANDS_MAX_COUNT_PER_SERVICE_TYPE * 8) - ($ - ping_service_t.commands) db 0
  .description  db "A simple service returning pong when receiving ping", NULL_CHAR
                times SERVICE_TYPE_DESC_MAX_LEN - ($ - ping_service_t.description) db 0
ping_service_t_end:

pong_msg db "pong"

ping_cmd_ping_t:
  .fn           dq ping_cmd_ping_fn
  .name         db "PING", NULL_CHAR
                times COMMAND_NAME_MAX_LEN - ($ - ping_cmd_ping_t.name) db 0
  .description  db "A command returning pong", NULL_CHAR
                times COMMAND_DESC_MAX_LEN - ($ - ping_cmd_ping_t.description) db 0
ping_cmd_ping_t_end:

section .text

ping_cmd_ping_fn:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVICE
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 4

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, pong_msg
  mov   rcx, 4
  rep   movsb

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
