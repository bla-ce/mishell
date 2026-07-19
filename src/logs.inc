section .rodata

log:
  .listen_tcp       db "listening on tcp socket", NULL_CHAR
  .accept_new_conn  db "accepted new connection", NULL_CHAR
  .recv_packet      db "received packet from client", NULL_CHAR
  .host_is_down    db " is down", NULL_CHAR
  .host_is_expired db " is expired, removing it from the network", NULL_CHAR
