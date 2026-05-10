section .bss

host_t:
  .id   resb ID_LEN
  .ip   resb IP_MAX_LEN
host_t_end:

hosts resb HOST_MAX_LEN * HOST_T_LEN

section .data

HOST_MAX_LEN equ 0x8

IP_MAX_LEN   equ 0x10

curr_host_idx db 0

HOST_T_LEN equ host_t_end - host_t

HOST_T_OFF_ID   equ 0x0
HOST_T_OFF_IP   equ HOST_T_OFF_ID + ID_LEN

section .text

; populates the host struct with ip and generates its id
; @param  rdi: pointer to the host struct
; @param  rsi: connection fd
; @return rax: error code
host_init:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the host struct
  ; [rsp+0x8]   -> connection fd
  ; [rsp+0x10]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], error_codes.INTERNAL

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  call  generate_id
  cmp   rax, FAILURE_CODE
  je    .error

  mov   rdi, [rsp]
  mov   qword [rdi+HOST_T_OFF_ID], rdx
  mov   qword [rdi+HOST_T_OFF_ID+0x8], rax

  ; TODO: get ip from connection fd

  jmp    .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
