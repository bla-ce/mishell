section .bss

service_t:
  .name   resb SERVICE_NAME_MAX_LEN
  .type   resb 0x1
  .status resb 0x1
service_t_end:

section .rodata

SERVICE_MAX_COUNT_PER_HOST equ 0x5

SERVICE_NAME_MAX_LEN equ 0x20

SERVICE_T_LEN equ service_t_end - service_t

SERVICE_T_OFF_NAME    equ 0x0
SERVICE_T_OFF_TYPE    equ SERVICE_T_OFF_NAME + SERVICE_NAME_MAX_LEN
SERVICE_T_OFF_STATUS  equ SERVICE_T_OFF_TYPE + 0x1

service_status:
  .REGISTERED equ 0x0
  .UP         equ 0x1
  .DOWN       equ 0x2
  .COUNT      equ 0x3

section .text
; returns the service struct from the host name and service name
; or -1 if the service is not registered
; @param  rdi: pointer to the name of the host
; @param  rsi: pointer to the name of the service
; @return rax: pointer to the service struct or -1
service_get_by_name:
  push  r12
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the name of the host
  ; [rsp+0x8]   -> pointer to the name of the service
  ; [rsp+0x10]  -> pointer to the services array of the host
  ; [rsp+0x18]  -> pointer to the service struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get host by name
  call  host_get_by_name
  cmp   rax, 0
  jl    .error

  add   rax, HOST_T_OFF_SERVICES

  mov   [rsp+0x10], rax

  xor   r12, r12

.loop:
  mov   rsi, [rsp+0x10]
  lea   rdi, [rsi+r12]

  mov   [rsp+0x18], rdi

  add   rdi, SERVICE_T_OFF_NAME
  mov   rsi, [rsp+0x8]
  call  strcmp
  cmp   rax, TRUE
  je    .loop_end

  add   r12, SERVICE_T_LEN

  cmp   r12, SERVICES_LEN
  jge   .error

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x18]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  pop   r12
  ret

; initialises a new service from a REGISTER packet and adds it to
; the host struct list of services
; @param  rdi: pointer to the REGISTER packet payload
; @param  rsi: pointer to the host struct
; @return
  ; - rax: pointer to the service struct
  ; - rdx: error code
service_init:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the packet payload
  ; [rsp+0x8]   -> pointer to the host struct
  ; [rsp+0x10]  -> pointer to the service struct
  ; [rsp+0x18]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get pointer to the service
  mov   rax, [rsp+0x8]
  movzx r9, byte [rax+HOST_T_OFF_SERVICES_COUNT]
  lea   rdi, [rax+HOST_T_OFF_SERVICES]

  imul  rax, r9, SERVICE_T_LEN
  add   rdi, rax

  mov   [rsp+0x10], rdi

  ; memset service
  xor   rax, rax
  mov   rcx, SERVICE_T_LEN
  rep   stosb

  ; packet is service_type_name + service_name
  ; validate name len
  mov   qword [rsp+0x18], error_codes.SERVICE_INVALID_NAME
  mov   rax, [rsp]
  lea   rdi, [rax+SERVICE_TYPE_NAME_MAX_LEN]
  call  strlen
  cmp   rax, 0
  jl    .error
  cmp   rax, SERVICE_NAME_MAX_LEN
  jge   .error

  ; copy service name
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp]
  add   rsi, SERVICE_TYPE_NAME_MAX_LEN
  mov   rcx, rax
  rep   movsb

  ; get type byte from string
  mov   qword [rsp+0x18], error_codes.SERVICE_INVALID_TYPE
  mov   rdi, [rsp]
  call  service_type_from_str
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   byte [rdi+SERVICE_T_OFF_TYPE], al

  ; add default status to the service
  mov   byte [rdi+SERVICE_T_OFF_STATUS], service_status.REGISTERED

  ; increase service count for the host
  mov   rdi, [rsp+0x8]
  inc   byte [rdi+HOST_T_OFF_SERVICES_COUNT]

  mov   rdx, SUCCESS_CODE
  mov   rax, [rsp+0x10]
  jmp   .return

.error:
  mov   rdx, [rsp+0x18]
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret
