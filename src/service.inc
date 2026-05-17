section .bss

service_t:
  .ip     resb ID_LEN
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
  .COUNT      equ 0x1

section .text
; initialises a new service from a REGISTER packet and adds it to
; the host struct list of services
; @param  rdi: pointer to the packet struct
; @param  rsi: pointer to the host struct
; @return rax: pointer to the service struct
service_init:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the packet struct
  ; [rsp+0x8]   -> pointer to the host struct
  ; [rsp+0x10]  -> pointer to the service struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get pointer to the service
  mov   rax, [rsp+0x8]
  mov   r9, [rax+HOST_T_OFF_SERVICES_COUNT]
  lea   rdi, [rax+HOST_T_OFF_SERVICES]

  imul  rax, r9, SERVICE_T_LEN
  lea   rdx, [rdi+rax]

  mov   [rsp+0x10], rdx

  ; deserialize the service payload
  mov   rax, [rsp]
  lea   rdi, [rax+PACKET_T_OFF_PAYLOAD]
  lea   rsi, [rax+PACKET_T_OFF_PAYLOAD_LEN]
  call  service_deserialize
  cmp   rax, 0
  jl    .error

  ; generate id for the service
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

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; deserializes a service from packet payload into a service struct
; expected payload format
; name=service_name\n
; type=storage
; @param  rdi: pointer to the payload
; @param  rsi: payload size
; @param  rdx: pointer to the service struct
; @return rax: return code
service_deserialize:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the payload
  ; [rsp+0x8]   -> payload size
  ; [rsp+0x10]  -> pointer to the service struct

  mov   [rsp], rdi
  mov   [rsp+0x8],rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; get service name index

  ; copy service name into struct

  ; get service type index

  ; string type to type enum value

  ; copy service type into struct

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret
