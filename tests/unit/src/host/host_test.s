global _start

%include "command.inc"
%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .rodata

dummy_id_1 dq 0x1234
           dq 0x5678

dummy_id_2 dq 0x2345
           dq 0x6789

dummy_id_3 dq 0x9876
           dq 0x5432

section .data

host_ptr dq 0

section .text
_start:
  ; --- host_get_by_id ---
  ; empty array should return -1
  mov   rdi, dummy_id_1
  call  host_get_by_id
  cmp   rax, FAILURE_CODE
  jne   .error

  lea   rdi, [hosts]
  mov   rax, qword [dummy_id_1]
  mov   rdx, qword [dummy_id_1+0x8]
  mov   [rdi+HOST_T_OFF_ID], rax
  mov   [rdi+HOST_T_OFF_ID+0x8], rdx
  mov   [host_ptr], rdi

  lea   rdi, [hosts]
  add   rdi, HOST_T_LEN
  mov   rax, qword [dummy_id_2]
  mov   rdx, qword [dummy_id_2+0x8]
  mov   [rdi+HOST_T_OFF_ID], rax
  mov   [rdi+HOST_T_OFF_ID+0x8], rdx

  mov   byte [curr_host_idx], 2

  ; filled array without the right id should return -1
  mov   rdi, dummy_id_3
  call  host_get_by_id
  cmp   rax, FAILURE_CODE
  jne   .error

  ; null pointer argument should return -1
  xor   rdi, rdi
  call  host_get_by_id
  cmp   rax, FAILURE_CODE
  jne   .error

  ; valid id should return the pointer to the host
  mov   rdi, dummy_id_1
  call  host_get_by_id
  cmp   rax, [host_ptr]
  jne   .error

  ; --- host_addr_exists ---
  ; reset hosts array
  mov   rdi, hosts
  xor   rax, rax
  mov   rcx, HOSTS_LEN
  rep   stosb

  ; test without hosts
  mov   rdi, 0
  mov   rsi, 7474
  call  host_addr_exists
  cmp   rax, FALSE
  jne   .error

  ; test without duplicate host
  ; add dummy hosts
  mov   r12, 0
  mov   r13, 7474

.loop:
  cmp   r12, 5
  jge   .loop_end

  imul  rax, r12, HOST_T_LEN
  lea   rsi, [hosts+rax]
  mov   dword [rsi+HOST_T_OFF_IP], 0
  mov   word [rsi+HOST_T_OFF_PORT], r13w

  inc   r13
  inc   r12

  jmp   .loop
.loop_end:
  mov   byte [curr_host_idx], 5

  mov   rdi, 0
  mov   rsi, 7000
  call  host_addr_exists
  cmp   rax, FALSE
  jne   .error

  ; test with duplicate host
  mov   rdi, 0
  mov   rsi, 7476
  call  host_addr_exists
  cmp   rax, TRUE
  jne   .error

  ; test with last host
  mov   rdi, 0
  mov   rsi, 7478
  call  host_addr_exists
  cmp   rax, TRUE
  jne   .error

  ; empty port should return failure
  mov   rdi, 0
  mov   rsi, 0
  call  host_addr_exists
  cmp   rax, FAILURE_CODE
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
