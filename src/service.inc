section .bss

service_t:
  .id     resb ID_LEN
  .name   resb SERVICE_NAME_MAX_LEN
  .type   resb 0x1
  .status resb 0x1
  .pad    resb 0x3
service_t_end:

section .data

SERVICE_MAX_COUNT_PER_HOST equ 0x5

SERVICE_NAME_MAX_LEN  equ 0xF

SERVICE_T_LEN equ service_t_end - service_t

SERVICE_T_OFF_ID      equ 0x0
SERVICE_T_OFF_NAME    equ SERVICE_T_OFF_ID + ID_LEN
SERVICE_T_OFF_TYPE    equ SERVICE_T_OFF_NAME + SERVICE_NAME_MAX_LEN
SERVICE_T_OFF_STATUS  equ SERVICE_T_OFF_TYPE + 0x1

service_status:
  .REGISTERED equ 0x0
  .UP         equ 0x1
  .DOWN       equ 0x2
  .COUNT      equ 0x3

section .text
; returns the service struct from the host id and service id or -1
; if the service is not registered
; @param  rdi: pointer to the id of the host
; @param  rsi: pointer to the id of the service
; @return rax: pointer to the service struct or -1
service_get_by_id:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the id of the host
  ; [rsp+0x8]   -> pointer to the id of the service
  ; [rsp+0x10]  -> pointer to the services array of the host
  ; [rsp+0x18]  -> pointer to the service struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get host by id
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  add   rax, HOST_T_OFF_SERVICES

  mov   [rsp+0x10], rax

  xor   r9, r9

.loop:
  mov   rsi, [rsp+0x10]
  lea   rdi, [rsi+r9]

  mov   [rsp+0x18], rdi

  add   rdi, SERVICE_T_OFF_ID
  mov   rsi, [rsp+0x8]
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .loop_end

  add   r9, SERVICE_T_LEN

  cmp   r9, SERVICES_LEN
  jge   .error

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x18]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; initialises a new service from a REGISTER packet and adds it to
; the host struct list of services
; @param  rdi: pointer to the packet struct
; @param  rsi: pointer to the host struct
; @return
  ; - rax: pointer to the service struct
  ; - rdx: error code
service_init:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the packet struct
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

  ; validate service payload
  mov   rdi, [rsp]
  add   rdi, PACKET_T_OFF_PAYLOAD
  call  service_validate
  mov   [rsp+0x18], rax
  test  rax, rax
  jnz   .error

  ; get pointer to the service
  mov   rax, [rsp+0x8]
  mov   r9, [rax+HOST_T_OFF_SERVICES_COUNT]
  lea   rdi, [rax+HOST_T_OFF_SERVICES]

  imul  rax, r9, SERVICE_T_LEN
  add   rdi, rax

  mov   [rsp+0x10], rdi

  ; memset service
  xor   rax, rax
  mov   rcx, SERVICE_T_LEN
  rep   stosb

  ; copy service packet
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp]
  add   rsi, PACKET_T_OFF_PAYLOAD
  mov   rcx, SERVICE_T_LEN
  rep   movsb

  ; generate id for the service
  mov   qword [rsp+0x18], error_codes.INTERNAL  ; default
  call  generate_id
  cmp   rax, FAILURE_CODE
  je    .error

  mov   rsi, [rsp+0x10]
  mov   qword [rsi+SERVICE_T_OFF_ID], rdx
  mov   qword [rsi+SERVICE_T_OFF_ID+0x8], rax

  ; add default status to the service
  mov   byte [rsi+SERVICE_T_OFF_STATUS], service_status.REGISTERED

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

; validates the service struct
; @param  rdi: pointer to the service struct
; @return rax: return code
service_validate:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the service struct
  ; [rsp+0x8] -> error code

  mov   [rsp], rdi
  mov   qword [rsp+0x8], error_codes.INTERNAL

  test  rdi, rdi
  jz    .error

  ; validate name len
  mov   qword [rsp+0x8], error_codes.SERVICE_INVALID_NAME
  add   rdi, SERVICE_T_OFF_NAME
  call  strlen
  cmp   rax, 0
  jl    .error
  cmp   rax, SERVICE_NAME_MAX_LEN
  jge   .error

  ; validate type
  mov   qword [rsp+0x8], error_codes.SERVICE_INVALID_TYPE
  mov   rdi, [rsp]
  cmp   byte [rdi+SERVICE_T_OFF_TYPE], service_types.COUNT
  jge   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x8]

.return:
  add   rsp, 0x10
  ret
