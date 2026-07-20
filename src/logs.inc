section .rodata

log:
  .init_network     db "Initialising network on port ", NULL_CHAR
  .network_done     db "Network initialised", NULL_CHAR
  .listen           db "Listening for incoming connections", NULL_CHAR
  .accept           db "Accepted new connection", NULL_CHAR
  .recv_packet      db "Received new packet of ", NULL_CHAR
  .recv_packet_2    db " bytes - OP: ", NULL_CHAR
  .host_is_down     db " has been down for ", NULL_CHAR
  .host_is_down_2   db " seconds", NULL_CHAR
  .host_is_expired  db " has been down for too long, removing it from the network", NULL_CHAR

LOG_MESSAGE_MAX_LEN equ 0xFF

section .data

log_message times LOG_MESSAGE_MAX_LEN db 0

section .text

; log message init network
; @param  rdi: port the socket will be listening to
; @return rax: return code
log_init_network:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> port the socket will be listening to
  ; [rsp+0x8] -> dynamic pointer

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  mov   rdi, log_message
  mov   rsi, log.init_network
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  mov   rdx, UNSIGNED_WORD_STR_MAX_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, logan_t
  mov   rsi, log_message
  call  log_info
  cmp   rax, 0
  jl    .error

  ; reset log_message
  mov   qword [log_message], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; logs when receiving a new packet
; @param  rdi: pointer to the packet struct
; @param  rsi: connection fd
; @return rax: return code
log_packet_reception:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the packet struct
  ; [rsp+0x8]   -> connection fd
  ; [rsp+0x10]  -> dynamic pointer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  mov   rdi, log_message
  mov   rsi, log.recv_packet
  call  strcat
  cmp   rax, 0
  jl    .error

  ; add packet size
  mov   rsi, [rsp]
  movzx rdi, word [rsi+PACKET_T_OFF_PAYLOAD_LEN]
  add   rdi, PACKET_T_HEADER_LEN
  mov   rsi, rax
  mov   rdx, UNSIGNED_WORD_STR_MAX_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, log_message
  mov   rsi, log.recv_packet_2
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  movzx rax, byte [rdi+PACKET_T_OFF_OP]
  mov   rsi, qword [req_op_str_table+rax*0x8]
  mov   rdi, log_message
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, logan_t
  mov   rsi, log_message
  call  log_info
  cmp   rax, 0
  jl    .error

  ; reset log message
  mov   qword [log_message], 0

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; logs when a host is down
; @param  rdi: pointer to the host struct
; @return rax: return code
log_host_down:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the host struct
  ; [rsp+0x8] -> dynamic log message pointer

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; get host name
  mov   rax, [rsp]
  lea   rsi, [rax+HOST_T_OFF_NAME]
  mov   rdi, log_message
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, log_message
  mov   rsi, log.host_is_down
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get expired time
  call  time_now
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rax, [rsp]
  sub   rdi, qword [rax+HOST_T_OFF_LAST_UP]
  mov   rsi, [rsp+0x8]
  mov   rdx, ITOA_STR_MAX_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, log_message
  mov   rsi, log.host_is_down_2
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, logan_t
  mov   rsi, log_message
  call  log_info
  cmp   rax, 0
  jl    .error

  ; reset log message
  mov   qword [log_message], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; logs when a host is expired
; @param  rdi: pointer to the host struct
; @return rax: return code
log_host_expired:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]     -> pointer to the host struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; get host name
  mov   rax, [rsp]
  lea   rsi, [rax+HOST_T_OFF_NAME]
  mov   rdi, log_message
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, log_message
  mov   rsi, log.host_is_expired
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   rdi, logan_t
  mov   rsi, log_message
  call  log_info
  cmp   rax, 0
  jl    .error

  ; reset log message
  mov   qword [log_message], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret
