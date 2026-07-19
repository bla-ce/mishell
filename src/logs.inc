section .rodata

log:
  .host_is_down     db " is down", NULL_CHAR
  .host_is_expired  db " is expired, removing it from the network", NULL_CHAR

section .text
