global _start

%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .rodata

dummy_host_1 db "home", NULL_CHAR
dummy_host_1_len equ $ - dummy_host_1

dummy_host_2 db "lab", NULL_CHAR
dummy_host_2_len equ $ - dummy_host_2

dummy_host_3 db "default", NULL_CHAR
dummy_host_3_len equ $ - dummy_host_3

invalid_host_name times HOST_NAME_MAX_LEN db 'A'
                  db NULL_CHAR

section .bss

test_host resb HOST_T_LEN

section .text
_start:
  ; --- host_get_by_name ---
  ; empty array should return -1
  mov   rdi, dummy_host_1
  call  host_get_by_name
  cmp   rax, FAILURE_CODE
  jne   .error

  ; add first two hosts
  lea   rdi, [hosts+HOST_T_OFF_NAME]
  mov   rsi, dummy_host_1
  mov   rcx, dummy_host_1_len
  rep   movsb

  lea   rdi, [hosts+HOST_T_LEN+HOST_T_OFF_NAME]
  mov   rsi, dummy_host_2
  mov   rcx, dummy_host_2_len
  rep   movsb

  mov   byte [curr_host_idx], 2

  ; filled array without the right id should return -1
  mov   rdi, dummy_host_3
  call  host_get_by_name
  cmp   rax, FAILURE_CODE
  jne   .error

  ; null pointer argument should return -1
  xor   rdi, rdi
  call  host_get_by_name
  cmp   rax, FAILURE_CODE
  jne   .error

  ; valid id should return the pointer to the host
  mov   rdi, dummy_host_1
  call  host_get_by_name
  cmp   rax, hosts  ; pointer to the first host
  jne   .error

  ; host init
  ; reset hosts
  mov   rdi, hosts
  xor   al, al
  mov   rcx, HOSTS_LEN
  rep   stosb

  ; empty host pointer should return failure
  xor   rdi, rdi
  xor   rsi, rsi
  mov   rdx, 7474
  mov   rcx, dummy_host_1
  call  host_init
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; empty host name should return failure
  mov   rdi, test_host
  xor   rsi, rsi
  mov   rdx, 7474
  xor   rcx, rcx
  call  host_init
  cmp   rax, error_codes.INTERNAL
  jne   .error

  ; happy path
  mov   rdi, test_host
  xor   rsi, rsi
  mov   rdx, 7474
  mov   rcx, dummy_host_1
  call  host_init
  cmp   rax, SUCCESS_CODE
  jne   .error

  cmp   word [test_host+HOST_T_OFF_PORT], 7474
  jne   .error
  cmp   dword [test_host+HOST_T_OFF_IP], 0
  jne   .error

  lea   rdi, [test_host+HOST_T_OFF_NAME]
  mov   rsi, dummy_host_1
  call  strcmp
  cmp   rax, TRUE
  jne   .error

  ; add it to hosts
  mov   rdi, hosts
  mov   rsi, test_host
  mov   rcx, HOST_T_LEN
  rep   movsb

  mov   byte [curr_host_idx], 1

  ; invalid host name should return INTERNAL
  mov   rdi, test_host
  xor   rsi, rsi
  mov   rdx, 5959
  mov   rcx, invalid_host_name
  call  host_init
  cmp   rax, error_codes.INTERNAL
  jne   .error

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
