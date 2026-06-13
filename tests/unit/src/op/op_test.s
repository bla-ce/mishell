global _start

%include "command.inc"
%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .rodata

section .bss

test_host       resb HOST_T_LEN
test_req_packet resb PACKET_T_LEN
test_res_packet resb PACKET_T_LEN

section .text
_start:
  ; --- op_hello ---
  ; empty request packet should return INTERNAL error
  xor   rdi, rdi
  mov   rsi, test_res_packet
  call  op_hello
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; empty response packet should return INTERNAL error
  mov   rdi, test_req_packet
  xor   rsi, rsi
  call  op_hello
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; happy path - no need to populate anything
  mov   rdi, test_req_packet
  mov   rsi, test_res_packet
  call  op_hello
  test  rax, rax
  jnz   .error

  mov   rdi, test_res_packet
  cmp   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  jne   .error
  cmp   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error
  cmp   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  jne   .error

  ; --- op_auth ---
  ; empty request packet should return INTERNAL error
  xor   rdi, rdi
  mov   rsi, test_res_packet
  call  op_auth
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; empty response packet should return INTERNAL error
  mov   rdi, test_req_packet
  xor   rsi, rsi
  call  op_auth
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; wrong host id in packet should return host not found
  mov   rdi, test_req_packet
  mov   qword [rdi+PACKET_T_OFF_ID], 0x1234
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0x2345
  mov   rsi, test_res_packet
  call  op_auth
  cmp   rax, error_codes.HOST_NOT_FOUND
  jne   .error

  ; add a host to the array
  mov   rdi, hosts
  mov   rsi, 0
  mov   rdx, 3000
  call  host_init
  cmp   rax, 0
  jl    .error

  mov   rsi, hosts

  ; increate host count
  inc   byte [curr_host_idx]

  ; valid host if should return OK with empty payload
  mov   rdi, test_req_packet

  mov   rax, qword [rsi+HOST_T_OFF_ID]
  mov   rdx, qword [rsi+HOST_T_OFF_ID+0x8]
  mov   qword [rdi+PACKET_T_OFF_ID], rax
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], rdx
  mov   rsi, test_res_packet
  call  op_auth
  test  rax, rax
  jnz   .error

  cmp   byte [test_res_packet+PACKET_T_OFF_OP], res_ops.OK
  jne   .error
  cmp   word [test_res_packet+PACKET_T_OFF_PAYLOAD_LEN], 0
  jne   .error

  ; op_auth with empty id should create a new host
  ; init new host
  lea   rdi, [test_req_packet+PACKET_T_OFF_PAYLOAD]
  mov   word [test_req_packet+PACKET_T_OFF_PAYLOAD_LEN], HOST_T_LEN
  xor   rsi, rsi
  mov   rdx, 3434
  call  host_init
  cmp   rax, 0
  jl    .error

  ; and return host array
  mov   rdi, test_req_packet
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   rsi, test_res_packet
  call  op_auth
  test  rax, rax
  jnz   .error

  cmp   byte [curr_host_idx], 2
  jne   .error

  mov   rdi, test_res_packet
  cmp   word [test_res_packet+PACKET_T_OFF_PAYLOAD_LEN], HOST_T_LEN * 2

  ; op_auth with new host with same ip/port should fail
  ; init new host
  lea   rdi, [test_req_packet+PACKET_T_OFF_PAYLOAD]
  mov   word [test_req_packet+PACKET_T_OFF_PAYLOAD_LEN], HOST_T_LEN
  xor   rsi, rsi
  mov   rdx, 3434
  call  host_init
  cmp   rax, 0
  jl    .error

  ; and return host array
  mov   rdi, test_req_packet
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   rsi, test_res_packet
  call  op_auth
  cmp   rax, error_codes.HOST_ALREADY_EXISTS
  jne   .error

  cmp   byte [curr_host_idx], 2
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
