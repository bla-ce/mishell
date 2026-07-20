global _start

%include "lib.inc"

section .bss

net_buf resb 4

section .data

; inet_pton: valid addresses
ip_any          db "0.0.0.0", NULL_CHAR
ip_broadcast    db "255.255.255.255", NULL_CHAR
ip_loopback     db "127.0.0.1", NULL_CHAR
ip_private_c    db "192.168.1.1", NULL_CHAR
ip_private_a    db "10.0.0.1", NULL_CHAR
ip_private_b    db "172.16.0.1", NULL_CHAR
ip_google_dns   db "8.8.8.8", NULL_CHAR
ip_sequential   db "1.2.3.4", NULL_CHAR
ip_mixed        db "100.200.150.50", NULL_CHAR
ip_max_first    db "255.0.0.0", NULL_CHAR
ip_max_last     db "0.0.0.255", NULL_CHAR

; inet_pton: error cases
ip_empty        db NULL_CHAR
ip_too_few      db "1.2.3", NULL_CHAR
ip_too_many     db "1.2.3.4.5", NULL_CHAR
ip_octet_256    db "256.0.0.0", NULL_CHAR
ip_octet_300    db "1.2.3.300", NULL_CHAR
ip_alpha        db "abc.def.ghi.jkl", NULL_CHAR
ip_mixed_alpha  db "192.168.1.abc", NULL_CHAR
ip_leading_dot  db ".1.2.3.4", NULL_CHAR
ip_trailing_dot db "1.2.3.4.", NULL_CHAR
ip_double_dot   db "1..2.3.4", NULL_CHAR
ip_neg_octet    db "192.168.-1.1", NULL_CHAR
ip_ipv6         db "::1", NULL_CHAR
ip_plain_num    db "12345678", NULL_CHAR
ip_space        db "192.168. 1.1", NULL_CHAR

section .text
_start:
  ; --- inet_pton: valid addresses ---

  ; "0.0.0.0" -> [0x00, 0x00, 0x00, 0x00], ret SUCCESS_CODE
  mov   rdi, ip_any
  call  inet_pton
  cmp   eax, 0x00000000
  jne   .error

  ; "255.255.255.255" -> [0xFF, 0xFF, 0xFF, 0xFF], ret SUCCESS_CODE
  mov   rdi, ip_broadcast
  call  inet_pton
  cmp   eax, 0xFFFFFFFF
  jne   .error

  ; "127.0.0.1" -> [0x7F, 0x00, 0x00, 0x01], ret SUCCESS_CODE
  mov   rdi, ip_loopback
  call  inet_pton
  cmp   eax, 0x0100007F
  jne   .error

  ; "192.168.1.1" -> [0xC0, 0xA8, 0x01, 0x01], ret SUCCESS_CODE
  mov   rdi, ip_private_c
  call  inet_pton
  cmp   eax, 0x0101A8C0
  jne   .error

  ; "10.0.0.1" -> [0x0A, 0x00, 0x00, 0x01], ret SUCCESS_CODE
  mov   rdi, ip_private_a
  call  inet_pton
  cmp   eax, 0x0100000A
  jne   .error

  ; "172.16.0.1" -> [0xAC, 0x10, 0x00, 0x01], ret SUCCESS_CODE
  mov   rdi, ip_private_b
  call  inet_pton
  cmp   eax, 0x010010AC
  jne   .error

  ; "8.8.8.8" -> [0x08, 0x08, 0x08, 0x08], ret SUCCESS_CODE
  mov   rdi, ip_google_dns
  call  inet_pton
  cmp   eax, 0x08080808
  jne   .error

  ; "1.2.3.4" -> [0x01, 0x02, 0x03, 0x04], ret SUCCESS_CODE
  mov   rdi, ip_sequential
  call  inet_pton
  cmp   eax, 0x04030201
  jne   .error

  ; "100.200.150.50" -> [0x64, 0xC8, 0x96, 0x32], ret SUCCESS_CODE
  mov   rdi, ip_mixed
  call  inet_pton
  cmp   eax, 0x3296C864
  jne   .error

  ; "255.0.0.0" -> [0xFF, 0x00, 0x00, 0x00], ret SUCCESS_CODE
  mov   rdi, ip_max_first
  call  inet_pton
  cmp   eax, 0x000000FF
  jne   .error

  ; "0.0.0.255" -> [0x00, 0x00, 0x00, 0xFF], ret SUCCESS_CODE
  mov   rdi, ip_max_last
  call  inet_pton
  cmp   eax, 0xFF000000
  jne   .error

  ; --- inet_pton: null pointer errors ---

  ; null string pointer -> FAILURE_CODE
  mov   rdi, 0
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; --- inet_pton: malformed strings ---

  ; empty string -> FAILURE_CODE
  mov   rdi, ip_empty
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; only 3 octets "1.2.3" -> FAILURE_CODE
  mov   rdi, ip_too_few
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; 5 octets "1.2.3.4.5" -> FAILURE_CODE
  mov   rdi, ip_too_many
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; first octet 256 -> FAILURE_CODE
  mov   rdi, ip_octet_256
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; last octet 300 -> FAILURE_CODE
  mov   rdi, ip_octet_300
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; fully alphabetic "abc.def.ghi.jkl" -> FAILURE_CODE
  mov   rdi, ip_alpha
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; last octet alphabetic "192.168.1.abc" -> FAILURE_CODE
  mov   rdi, ip_mixed_alpha
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; leading dot ".1.2.3.4" -> FAILURE_CODE
  mov   rdi, ip_leading_dot
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; trailing dot "1.2.3.4." -> FAILURE_CODE
  mov   rdi, ip_trailing_dot
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; consecutive dots "1..2.3.4" -> FAILURE_CODE
  mov   rdi, ip_double_dot
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; negative octet "192.168.-1.1" -> FAILURE_CODE
  mov   rdi, ip_neg_octet
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; IPv6 "::1" -> FAILURE_CODE
  mov   rdi, ip_ipv6
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; plain number without dots "12345678" -> FAILURE_CODE
  mov   rdi, ip_plain_num
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  ; space inside octet "192.168. 1.1" -> FAILURE_CODE
  mov   rdi, ip_space
  call  inet_pton
  cmp   rax, FAILURE_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
