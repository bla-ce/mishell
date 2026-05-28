%include "auth.inc"

section .rodata

req_ops:
  .HELLO      equ 0x0
  .AUTH       equ 0x1
  .REGISTER   equ 0x2
  .START      equ 0x3
  .STOP       equ 0x4
  .UNREGISTER equ 0x5
  .COUNT      equ 0x6

res_ops:
  .OK     equ 0x0
  .ERROR  equ 0x1
  .COUNT  equ 0x2

req_op_fn_table:
  .HELLO      dq op_hello
  .AUTH       dq op_auth
  .REGISTER   dq op_register
  .START      dq op_start
  .STOP       dq op_stop
  .UNREGISTER dq op_unregister

section .text

; sends back a WELCOME op with an empty payload for now
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_hello:
  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   word [rsi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rsi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rsi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    word [rsi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rsi+PACKET_T_OFF_ID], 0
  mov   qword [rsi+PACKET_T_OFF_ID+0x8], 0
  mov   word [rsi+PACKET_T_OFF_PAYLOAD_LEN], 0

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; checks payload to see if host has been registered, otherwise
; creates a session id and adds it to the host array
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_auth:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd
  ; [rsp+0x10]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; build base packet
  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0

  ; check if id is set
  mov   rax, [rsp]
  movups  xmm1, [rax+PACKET_T_OFF_ID]
  ptest   xmm1, xmm1
  jz    .new_host

  mov   qword [rsp+0x18], error_codes.HOST_NOT_FOUND
  lea   rdi, [rax+PACKET_T_OFF_ID]
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  ; valid host id
  mov   rdi, [rsp+0x8]
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  mov   qword [rdi+PACKET_T_OFF_PAYLOAD], 0

  mov   rax, SUCCESS_CODE

  jmp   .return

.new_host:
  ; make sure we haven't reached the host limit
  mov   qword [rsp+0x18], error_codes.HOST_LIMIT_REACHED
  cmp   byte [curr_host_idx], HOST_MAX_COUNT
  jge   .error

  ; populate host struct
  mov   qword [rsp+0x18], error_codes.INTERNAL
  movzx eax, byte [curr_host_idx]
  imul  eax, HOST_T_LEN
  lea   rdi, [hosts+eax]
  mov   rsi, [rsp+0x10]
  call  host_init
  cmp   rax, 0
  jl    .error

  ; build response packet
  mov   rdi, [rsp+0x8]
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK

  ; id is in the host destination field
  add   rdi, PACKET_T_OFF_DEST_HOST
  movzx eax, byte [curr_host_idx]
  imul  eax, HOST_T_LEN
  lea   rsi, [hosts+eax+HOST_T_OFF_ID]
  mov   rcx, ID_LEN
  rep   movsb

  inc   byte [curr_host_idx]

  ; add host array to payload
  mov   rdi, [rsp+0x8]
  movzx cx, byte [curr_host_idx]
  imul  cx, HOST_T_LEN
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], cx
  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, hosts
  rep   movsb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, [rsp+0x18]

.return:
  add   rsp, 0x20
  ret

; registers a new service for the host
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_register:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd
  ; [rsp+0x18]  -> pointer to the host
  ; [rsp+0x20]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x20], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   qword [rsp+0x20], error_codes.UNAUTHORIZED
  ; make sure the entity flag is FL_HOST
  mov   rax, [rsp]
  mov   bx, word [rax+PACKET_T_OFF_FLAGS]
  mov   r9w, bx
  and   r9w, FL_HOST
  test  r9w, r9w
  jz    .error

  ; verify host
  movups  xmm1, [rax+PACKET_T_OFF_ID]
  ptest   xmm1, xmm1
  jz    .error

  mov   qword [rsp+0x20], error_codes.INTERNAL
  lea   rdi, [rax+PACKET_T_OFF_ID]
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; make sure we can add a new service
  mov   qword [rsp+0x20], error_codes.SERVICE_PER_HOST_LIMIT_REACHED
  cmp   byte [rax+HOST_T_OFF_SERVICES_COUNT], SERVICE_MAX_COUNT_PER_HOST
  jge   .error

  ; initialises a new service
  mov   qword [rsp+0x20], error_codes.INTERNAL
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  call  service_init
  mov   [rsp+0x20], rdx ; save error code if it failed
  test  rdx, rdx
  jnz   .error

  mov   rdi, [rsp+0x8]
  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, rax
  mov   rcx, SERVICE_T_LEN
  rep   movsb

  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], SERVICE_T_LEN

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x20]

.return:
  add   rsp, 0x28
  ret

; starts a service from START command with host and service id
; returns OK
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_start:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd
  ; [rsp+0x18]  -> pointer to the host
  ; [rsp+0x20]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x20], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   qword [rsp+0x20], error_codes.UNAUTHORIZED
  ; make sure the entity flag is FL_HOST
  mov   rax, [rsp]
  mov   bx, word [rax+PACKET_T_OFF_FLAGS]
  mov   r9w, bx
  and   r9w, FL_HOST
  test  r9w, r9w
  jz    .error

  ; verify host
  movups  xmm1, [rax+PACKET_T_OFF_ID]
  ptest   xmm1, xmm1
  jz    .error

  lea   rdi, [rax+PACKET_T_OFF_ID]
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; find service inside host struct
  ; payload is host id followed by service id
  mov   qword [rsp+0x20], error_codes.SERVICE_NOT_FOUND
  mov   rax, [rsp]
  lea   rdi, [rax+PACKET_T_OFF_PAYLOAD]
  lea   rsi, [rax+PACKET_T_OFF_PAYLOAD+ID_LEN]
  call  service_get_by_id
  cmp   rax, 0
  jl    .error

  ; change status
  mov   byte [rax+SERVICE_T_OFF_STATUS], service_status.UP

  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  mov   word [rdi+PACKET_T_OFF_PAYLOAD], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x20]

.return:
  add   rsp, 0x28
  ret

; stops a service from STOP command with host and service id
; returns OK
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_stop:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd
  ; [rsp+0x18]  -> pointer to the host
  ; [rsp+0x20]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x20], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   qword [rsp+0x20], error_codes.UNAUTHORIZED
  ; make sure the entity flag is FL_HOST
  mov   rax, [rsp]
  mov   bx, word [rax+PACKET_T_OFF_FLAGS]
  mov   r9w, bx
  and   r9w, FL_HOST
  test  r9w, r9w
  jz    .error

  ; verify host
  movups  xmm1, [rax+PACKET_T_OFF_ID]
  ptest   xmm1, xmm1
  jz    .error

  lea   rdi, [rax+PACKET_T_OFF_ID]
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; find service inside host struct
  ; payload is host id followed by service id
  mov   qword [rsp+0x20], error_codes.SERVICE_NOT_FOUND
  mov   rax, [rsp]
  lea   rdi, [rax+PACKET_T_OFF_PAYLOAD]
  lea   rsi, [rax+PACKET_T_OFF_PAYLOAD+ID_LEN]
  call  service_get_by_id
  cmp   rax, 0
  jl    .error

  ; change status
  mov   byte [rax+SERVICE_T_OFF_STATUS], service_status.DOWN

  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  mov   word [rdi+PACKET_T_OFF_PAYLOAD], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x20]

.return:
  add   rsp, 0x28
  ret

; unregisters a service, removing it from the host array of services
; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @param  rdx: connection fd
; @return rax: packet size
op_unregister:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> connection fd
  ; [rsp+0x18]  -> pointer to the host
  ; [rsp+0x20]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x20], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   qword [rsp+0x20], error_codes.UNAUTHORIZED
  ; make sure the entity flag is FL_HOST
  mov   rax, [rsp]
  mov   bx, word [rax+PACKET_T_OFF_FLAGS]
  mov   r9w, bx
  and   r9w, FL_HOST
  test  r9w, r9w
  jz    .error

  ; verify host
  movups  xmm1, [rax+PACKET_T_OFF_ID]
  ptest   xmm1, xmm1
  jz    .error

  lea   rdi, [rax+PACKET_T_OFF_ID]
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; find service inside host struct
  ; payload is host id followed by service id
  mov   qword [rsp+0x20], error_codes.SERVICE_NOT_FOUND
  mov   rax, [rsp]
  lea   rdi, [rax+PACKET_T_OFF_PAYLOAD]
  lea   rsi, [rax+PACKET_T_OFF_PAYLOAD+ID_LEN]
  call  service_get_by_id
  cmp   rax, 0
  jl    .error

  ; make sure the status is not UP
  mov   qword [rsp+0x20], error_codes.SERVICE_STOP_BEFORE_UNREGISTER
  cmp   byte [rax+SERVICE_T_OFF_STATUS], service_status.UP
  je    .error

  ; clear out service
  mov   rdi, rax
  xor   rax, rax
  mov   rcx, SERVICE_T_LEN
  rep   stosb

  ; decrease service count for the host
  mov   rdi, [rsp+0x18]
  dec   byte [rdi+HOST_T_OFF_SERVICES_COUNT]

  mov   rdi, [rsp+0x8]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER_TO_CLIENT
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_SERVER
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0
  mov   word [rdi+PACKET_T_OFF_PAYLOAD], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x20]

.return:
  add   rsp, 0x28
  ret
