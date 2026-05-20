section .bss

host_t:
  .id             resb ID_LEN
  .ipv4           resd 0x1
  .port           resw 0x1
  .services       resb SERVICES_LEN
  .services_count resb 0x1
  .pad            resb 0x5
host_t_end:

hosts resb HOSTS_LEN

section .data

HOST_MAX_COUNT equ 0x5

curr_host_idx db 0

HOSTS_LEN equ HOST_T_LEN * HOST_MAX_COUNT

HOST_T_LEN equ host_t_end - host_t

SERVICES_LEN equ SERVICE_T_LEN * SERVICE_MAX_COUNT_PER_HOST

HOST_T_OFF_ID             equ 0x0
HOST_T_OFF_IP             equ HOST_T_OFF_ID + ID_LEN
HOST_T_OFF_PORT           equ HOST_T_OFF_IP + 0x4
HOST_T_OFF_SERVICES       equ HOST_T_OFF_PORT + 0x2
HOST_T_OFF_SERVICES_COUNT equ HOST_T_OFF_SERVICES + SERVICES_LEN

section .text

; returns the host struct from the host id or -1 if the host is
; not registered
; @param  rdi: pointer to the id of the host
; @return rax: pointer to the host struct or -1
host_get_by_id:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the id of the host
  ; [rsp+0x8] -> pointer to the host struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  xor   r9, r9

.loop:
  lea   rdi, [hosts+r9]

  mov   [rsp+0x8], rdi

  add   rdi, HOST_T_OFF_ID
  mov   rsi, [rsp]
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .loop_end

  add   r9, HOST_T_LEN

  cmp   r9, HOSTS_LEN
  jge   .error

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; populates the host struct with ip and generates its id
; @param  rdi: pointer to the host struct
; @param  rsi: connection fd
; @return rax: return code
host_init:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the host struct
  ; [rsp+0x8]   -> connection fd

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; memset host to 0
  mov   rsi, [rsp]
  xor   rax, rax
  mov   rcx, HOST_T_LEN
  rep   stosb

  call  generate_id
  cmp   rax, FAILURE_CODE
  je    .error

  mov   rsi, [rsp]
  mov   qword [rsi+HOST_T_OFF_ID], rdx
  mov   qword [rsi+HOST_T_OFF_ID+0x8], rax

  mov   rdi, [rsp+0x8]
  call  net_get_ipv4_from_fd
  cmp   eax, 0
  jl    .error

  mov   rsi, [rsp]
  mov   dword [rsi+HOST_T_OFF_IP], eax

  mov   rax, SUCCESS_CODE
  jmp    .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
