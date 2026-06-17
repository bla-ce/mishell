global _start

%include "host.inc"
%include "command.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

%include "test_validation.s"

section .rodata

PEERS_LEN equ 5

section .data

test_sockaddr_in_t:
  .sin_family dw AF_INET
  .sin_port   dw 0x321d ; (network order of 7474)
  .sin_addr   dd 0
  .sin_zero   dq 0
test_sockaddr_in_t_end:

test_sockaddr_in_t_len dq test_sockaddr_in_t_end - test_sockaddr_in_t

test_packet_t:
  .magic        dw 0
  .op           db 0
  .flags        db 0
  .id           times ID_LEN db 0
  .dest_host    times ID_LEN db 0
  .dest_service times ID_LEN db 0
  .payload_len  dw 0
  .payload      times PAYLOAD_MAX_LEN db 0
test_packet_t_end:

peer_fd   times PEERS_LEN dq 0
host_ids  times ID_LEN * PEERS_LEN db 0

section .text
_start:
  xor   r12, r12

.loop:
  cmp   r12, PEERS_LEN
  jge   .loop_end

  ; set up socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  xor   rdx, rdx
  syscall
  cmp   rax, 0
  jl    .error

  mov   [peer_fd+0x8*r12], rax

  mov   rax, SYS_CONNECT
  mov   rdi, qword [peer_fd+0x8*r12]
  mov   rsi, test_sockaddr_in_t
  mov   rdx, qword [test_sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  inc   r12

  jmp   .loop
.loop_end:
  mov   rdi, qword [peer_fd]  ; load first peer
  call  test_validation
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
