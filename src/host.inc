section .bss

host_t:
  .id             resb ID_LEN
  .ipv4           resb NET_IPV4_MAX_LEN
  .services       resb SERVICE_T_LEN * SERVICE_MAX_COUNT_PER_HOST
  .services_count resb 0x1
  .pad            resb 0x7
host_t_end:

hosts resb HOST_T_LEN * HOST_MAX_COUNT

section .data

HOST_MAX_COUNT equ 0x5

curr_host_idx db 0

HOST_T_LEN equ host_t_end - host_t

HOST_T_OFF_ID             equ 0x0
HOST_T_OFF_IP             equ HOST_T_OFF_ID + ID_LEN
HOST_T_OFF_SERVICES       equ HOST_T_OFF_IP + NET_IPV4_MAX_LEN
HOST_T_OFF_SERVICES_COUNT equ HOST_T_OFF_SERVICES + 0x8

section .text

; returns the host struct from the host id or -1 if the host is
; not registered
; @param  rdi: first 64bit of the id
; @param  rsi: second 64bit of the id
; @return rax: pointer to the host struct or -1
host_get_by_id:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> first 64bit of the id
  ; [rsp+0x8]   -> second 64bit of the id
  ; [rsp+0x10]  -> pointer to the host struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  xor   r9, r9
.loop:
  lea   rdi, [hosts+r9]

  mov   [rsp+0x10], rax

  lea   rsi, [rsp]
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .loop_end

  add   r9, ID_LEN

  movzx rax, byte [curr_host_idx]
  shl   rax, 4  ; rax *= 16 (ID_LEN)

  cmp   r9, rax
  jge   .error

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x10]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
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

  test  rsi, rsi
  jz    .error

  call  generate_id
  cmp   rax, FAILURE_CODE
  je    .error

  mov   rsi, [rsp]
  mov   qword [rsi+HOST_T_OFF_ID], rdx
  mov   qword [rsi+HOST_T_OFF_ID+0x8], rax
  mov   qword [rsi+HOST_T_OFF_SERVICES_COUNT], 0

  ; set ip to 0
  lea   rdi, [rsi+HOST_T_OFF_IP]
  xor   rax, rax
  mov   rcx, NET_IPV4_MAX_LEN
  rep   stosb

  ; set services to 0
  lea   rdi, [rsi+HOST_T_OFF_SERVICES]
  xor   rax, rax
  mov   rcx, SERVICE_T_LEN * SERVICE_MAX_COUNT_PER_HOST
  rep   stosb

  mov   rdi, [rsp+0x8]
  add   rsi, HOST_T_OFF_IP
  call  net_get_ipv4_from_fd
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp    .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
