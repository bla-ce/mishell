section .rodata

storage_service_t:
  .commands     dq storage_command_test
  .pad2         times COMMANDS_MAX_COUNT_PER_SERVICE_TYPE - 1 dq 0
  .type         db service_types.STORAGE
  .description  db "A storage service to store things that have not been defined"
storage_service_t_end:

test_msg db "test"

section .text

storage_command_test:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVICE_TO_CLIENT
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_SERVICE
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 4

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, test_msg
  mov   rcx, 4
  rep   movsb

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret
