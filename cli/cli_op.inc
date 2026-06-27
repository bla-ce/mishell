section .rodata

req_op_cli_table:
  .HELLO      dq cli_hello
  .JOIN       dq 0  ; not allowed
  .REGISTER   dq cli_register
  .START      dq cli_start
  .STOP       dq cli_stop
  .UNREGISTER dq cli_unregister
  .NETWORK    dq cli_network
  .UPDATE     dq 0  ; not allowed
  .QUERY      dq cli_query
  .CATALOG    dq cli_catalog

req_op_str:
  .HELLO      db "hello", NULL_CHAR
  .JOIN       db "join", NULL_CHAR
  .REGISTER   db "register", NULL_CHAR
  .START      db "start", NULL_CHAR
  .STOP       db "stop", NULL_CHAR
  .UNREGISTER db "unregister", NULL_CHAR
  .NETWORK    db "network", NULL_CHAR
  .UPDATE     db "update", NULL_CHAR
  .QUERY      db "query", NULL_CHAR
  .CATALOG    db "catalog", NULL_CHAR

res_ok    db "OK", NULL_CHAR
res_error db "ERROR: ", NULL_CHAR

req_op_str_table:
  .HELLO      dq req_op_str.HELLO
  .JOIN       dq req_op_str.JOIN
  .REGISTER   dq req_op_str.REGISTER
  .START      dq req_op_str.START
  .STOP       dq req_op_str.STOP
  .UNREGISTER dq req_op_str.UNREGISTER
  .NETWORK    dq req_op_str.NETWORK
  .UPDATE     dq req_op_str.UPDATE
  .QUERY      dq req_op_str.QUERY
  .CATALOG    dq req_op_str.CATALOG

mishli_tab_catalog db "    ", NULL_CHAR
mishli_cmd_catalog db "Commands:", NULL_CHAR

section .text

; returns the op value from a string
; @param  rdi: pointer to the string
; @return rax: op value
mishli_cli_op_fn_from_str:
  push  r12
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the string

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  xor   r12, r12

.loop:
  cmp   r12, req_ops.COUNT
  jge   .error

  mov   rdi, [rsp]
  mov   rsi, qword [req_op_str_table+r12*0x8]
  call  strcmp
  cmp   rax, TRUE
  je    .done

  inc   r12
  jmp   .loop

.done:
  shl   r12, 3   ; r12 *= 8

  ; get cli function from op
  mov   rax, [req_op_cli_table+r12]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  pop   r12
  ret

; sends HELLO command to remote host
; @return rax: return code
cli_hello:
  sub   rsp, PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet (req and resp)

  mov   word [rsp+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rsp+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rsp+PACKET_T_OFF_DEST_HOST], 0
  mov   byte [rsp+PACKET_T_OFF_OP], req_ops.HELLO
  mov   word [rsp+PACKET_T_OFF_PAYLOAD_LEN], 0

  lea   rdi, [rsp]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print ok
  mov   rdi, res_ok
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, PACKET_T_LEN
  ret

; sends REGISTER command to the host to register a new service
; @param  rdi: pointer to the host name
; @param  rsi: pointer to the service type
; @param  rdx: pointer to the service name
; @return rax: return code
cli_register:
  sub   rsp, 0x18+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the host name
  ; [rsp+0x8]   -> pointer to the service type
  ; [rsp+0x10]  -> pointer to the service name
  ; [rsp+0x18]  -> pointer to the packet (req and resp)

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.REGISTER

  ; copy host name
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_DEST_HOST
  mov   rsi, [rsp]
  mov   rcx, rax
  rep   movsb

  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  ; copy service type name
  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  ; add null char
  mov   al, NULL_CHAR
  stosb

  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error

  ; copy service name
  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_PAYLOAD+SERVICE_TYPE_NAME_MAX_LEN
  mov   rsi, [rsp+0x10]
  mov   rcx, rax
  rep   movsb

  ; add null char
  mov   al, NULL_CHAR
  stosb

  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], SERVICE_TYPE_NAME_MAX_LEN+SERVICE_NAME_MAX_LEN

  lea   rdi, [rsp+0x18]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print ok
  mov   rdi, res_ok
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+PACKET_T_LEN
  ret

; sends UNREGISTER command to the host to unregister a service
; @param  rdi: pointer to the host name
; @param  rsi: pointer to the service name
; @return rax: return code
cli_unregister:
  sub   rsp, 0x10+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the host name
  ; [rsp+0x8]   -> pointer to the service name
  ; [rsp+0x10]  -> pointer to the packet (req and resp)

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.UNREGISTER

  ; copy host name
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  add   rdi, PACKET_T_OFF_DEST_HOST
  mov   rsi, [rsp]
  mov   rcx, rax
  rep   movsb

  ; copy service name
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ax

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  lea   rdi, [rsp+0x10]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print ok
  mov   rdi, res_ok
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10+PACKET_T_LEN
  ret

; sends START command to the host to start a service
; @param  rdi: pointer to the host name
; @param  rsi: pointer to the service name
; @return rax: return code
cli_start:
  sub   rsp, 0x10+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the host name
  ; [rsp+0x8]   -> pointer to the service name
  ; [rsp+0x10]  -> pointer to the packet (req and resp)

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.START

  ; copy host name
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  add   rdi, PACKET_T_OFF_DEST_HOST
  mov   rsi, [rsp]
  mov   rcx, rax
  rep   movsb

  ; copy service name
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ax

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  lea   rdi, [rsp+0x10]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print ok
  mov   rdi, res_ok
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10+PACKET_T_LEN
  ret

; sends STOP command to the host to stop a service
; @param  rdi: pointer to the host name
; @param  rsi: pointer to the service name
; @return rax: return code
cli_stop:
  sub   rsp, 0x10+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the host name
  ; [rsp+0x8]   -> pointer to the service name
  ; [rsp+0x10]  -> pointer to the packet (req and resp)

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.STOP

  ; copy host name
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  add   rdi, PACKET_T_OFF_DEST_HOST
  mov   rsi, [rsp]
  mov   rcx, rax
  rep   movsb

  ; copy service name
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ax

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  lea   rdi, [rsp+0x10]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print ok
  mov   rdi, res_ok
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10+PACKET_T_LEN
  ret

; sends CATALOG command to get list of available service types
; @return rax: return code
cli_catalog:
  push  r12
  push  r13
  push  r14
  sub   rsp, 0x10+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to current service type
  ; [rsp+0x8]   -> pointer to current command
  ; [rsp+0x10]  -> pointer to the packet (req and resp)

  mov   word [rsp+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rsp+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rsp+PACKET_T_OFF_OP], req_ops.CATALOG
  mov   word [rsp+PACKET_T_OFF_PAYLOAD_LEN], 0

  lea   rdi, [rsp]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; get number of service types
  lea   rdi, [rsp]
  xor   rdx, rdx
  movzx rax, word [rdi+PACKET_T_OFF_PAYLOAD_LEN]
  mov   rcx, SERVICE_TYPE_T_LEN
  div   rcx

  mov   r12, rax

  xor   r13, r13

  ; print service types
.loop:
  cmp   r13, r12
  jge   .loop_end

  ; get service type
  lea   rdi, [rsp]
  imul  rax, r13, SERVICE_TYPE_T_LEN
  add   rax, PACKET_T_OFF_PAYLOAD
  add   rdi, rax

  mov   [rsp], rdi

  ; print name
  mov   rdi, [rsp]
  add   rdi, SERVICE_TYPE_T_OFF_NAME
  call  print

  ; print horizontal tab
  mov   rdi, COLON
  call  print_char
  mov   rdi, SPACE
  call  print_char

  ; print description
  mov   rdi, [rsp]
  add   rdi, SERVICE_TYPE_T_OFF_DESC
  call  println

  ; print horizontal tab
  mov   rdi, mishli_tab_catalog
  call  print

  mov   rdi, mishli_cmd_catalog
  call  println

  xor   r14, r14

.inner_loop:
  cmp   r14, COMMANDS_MAX_COUNT_PER_SERVICE_TYPE
  jge   .inner_loop_end

  ; get command
  mov   rax, [rsp]
  add   rax, SERVICE_TYPE_T_OFF_COMMANDS
  mov   rdi, [rax+r14*0x8]

  test  rdi, rdi
  jz    .inner_loop_end

  mov   [rsp+0x10], rdi

  mov   rdi, mishli_tab_catalog
  call  print
  mov   rdi, mishli_tab_catalog
  call  print

  ; print command name
  mov   rdi, [rsp+0x10]
  add   rdi, COMMAND_T_OFF_NAME
  call  print

  mov   rdi, COLON
  call  print_char

  mov   rdi, SPACE
  call  print_char

  ; print command description
  mov   rdi, [rsp+0x10]
  add   rdi, COMMAND_T_OFF_DESC
  call  println

  inc   r14
  jmp   .inner_loop

.inner_loop_end:

  inc   r13
  jmp   .loop

.loop_end:

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10+PACKET_T_LEN
  pop   r14
  pop   r13
  pop   r12
  ret

; sends NETWORK command to get list of available hosts in the network
; @return rax: return code
cli_network:
  push  r12
  push  r13
  push  r14
  sub   rsp, PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet (req and resp)

  mov   word [rsp+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rsp+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rsp+PACKET_T_OFF_OP], req_ops.NETWORK
  mov   word [rsp+PACKET_T_OFF_PAYLOAD_LEN], 0

  lea   rdi, [rsp]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; get number of hosts
  lea   rdi, [rsp]
  xor   rdx, rdx
  movzx rax, word [rdi+PACKET_T_OFF_PAYLOAD_LEN]
  mov   rcx, HOST_T_LEN
  div   rcx

  mov   r12, rax

  xor   r13, r13

  ; print hosts
.loop:
  cmp   r13, r12
  jge   .loop_end

  ; get host
  lea   r14, [rsp]
  imul  rax, r13, HOST_T_LEN
  add   rax, PACKET_T_OFF_PAYLOAD
  add   r14, rax

  movsx rdi, dword [r14+HOST_T_OFF_IP]
  movzx rax, word [r14+HOST_T_OFF_PORT]
  xchg  al, ah
  mov   rsi, rax
  call  cli_print_host_addr
  cmp   rax, 0
  jl    .error


  ; print space
  mov   rdi, SPACE
  call  print_char

  ; print name
  mov   rdi, r14
  add   rdi, HOST_T_OFF_NAME
  call  println

  inc   r13
  jmp   .loop

.loop_end:

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, PACKET_T_LEN
  pop   r14
  pop   r13
  pop   r12
  ret

; sends QUERY command to the host to query an existing service
; @param  rdi: pointer to the host name
; @param  rsi: pointer to the service name
; @param  rdx: pointer to the service command
; @return rax: return code
cli_query:
  sub   rsp, 0x18+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the host name
  ; [rsp+0x8]   -> pointer to the service name
  ; [rsp+0x10]  -> pointer to the service command
  ; [rsp+0x18]  -> pointer to the packet (req and resp)

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_USER
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.QUERY

  ; copy host name
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_DEST_HOST
  mov   rsi, [rsp]
  mov   rcx, rax
  rep   movsb

  ; copy service name
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_DEST_SERVICE
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  ; copy the command
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ax

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, [rsp+0x10]
  mov   rcx, rax
  rep   movsb

  lea   rdi, [rsp+0x18]
  call  cli_transaction
  cmp   rax, 0
  jl    .error

  ; print payload
  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_PAYLOAD
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+PACKET_T_LEN
  ret

; sends a packet to remote host and get the response back
; the request packet is being overwritten by the response
; @param  rdi: pointer to the packet struct
; @return rax: return code
cli_transaction:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .no_print

  ; get payload length
  movzx rdx, word [rdi+PACKET_T_OFF_PAYLOAD_LEN]
  add   rdx, PACKET_T_HEADER_LEN

  ; send response packet
  mov   rax, SYS_WRITE
  mov   rdi, [host_fd]
  mov   rsi, [rsp]
  ; rdx already populated
  syscall
  cmp   rax, 0
  jl    .no_print

  ; get response
  mov   rax, SYS_READ
  mov   rdi, [host_fd]
  mov   rsi, [rsp]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .no_print

  ; check response op
  mov   rdi, [rsp]
  cmp   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp]
  call  cli_print_error

.no_print:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; print ERROR and the error message from payload
; @param  rdi: pointer to the packet
; @return rax: return code
cli_print_error:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the packet

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  mov   rdi, res_error
  call  print

  mov   rdi, [rsp]
  add   rdi, PACKET_T_OFF_PAYLOAD
  call  println

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; prints ip:port pair of the host
; @param  rdi: ip of the host
; @param  rsi: port of the host
; @return rax: return code
cli_print_host_addr:
  push  r12
  sub   rsp, 0x10+UNSIGNED_WORD_STR_MAX_SIZE

  ; STACK USAGE
  ; [rsp]       -> ip of the host
  ; [rsp+0x8]   -> port of the host (byte order)
  ; [rsp+0x10]  -> pointer to the byte string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  xor   r12, r12

.loop:
  ; get byte
  lea   rax, [rsp]
  movzx rdi, byte [rax+r12]
  lea   rsi, [rsp+0x10]
  mov   rdx, UNSIGNED_WORD_STR_MAX_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  ; print byte
  lea   rdi, [rsp+0x10]
  call  print

  inc   r12

  cmp   r12, 4
  jge   .loop_end

  mov   rdi, DOT
  call  print_char

  jmp   .loop

.loop_end:
  mov   rdi, COLON
  call  print_char

  ; print port
  movzx rdi, word [rsp+0x8]
  lea   rsi, [rsp+0x10]
  mov   rdx, UNSIGNED_WORD_STR_MAX_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  call  print

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10+UNSIGNED_WORD_STR_MAX_SIZE
  pop   r12
  ret
