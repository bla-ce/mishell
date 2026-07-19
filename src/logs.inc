section .rodata

log:
  .recv_packet      db "received ", NULL_CHAR
  .recv_packet_2    db " bytes - OP: ", NULL_CHAR
  .host_is_down     db " is down", NULL_CHAR
  .host_is_expired  db " is expired, removing it from the network", NULL_CHAR
