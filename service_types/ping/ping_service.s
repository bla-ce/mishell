section .data

ping_service_t:
  .commands     dq ping_command_ping
  .pad2         times COMMANDS_MAX_COUNT_PER_SERVICE_TYPE - 1 dq 0
  .type         db service_types.PING
  .description  db "A simple service printing pong when receiving ping"
ping_service_t_end:

pong_msg db "hello"

section .text

ping_command_ping:
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT_FILENO
  mov   rsi, pong_msg
  mov   rdx, 4
  syscall

  ret
