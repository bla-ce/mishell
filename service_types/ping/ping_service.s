section .data

ping_service_t:
  .commands     dq ping_command_ping
  .pad2         times COMMANDS_MAX_COUNT_PER_SERVICE_TYPE - 1 dq 0
  .type         db service_types.PING
  .description  db "A simple service returning pong to ping"
ping_service_t_end:


section .text

ping_command_ping:
  ret
