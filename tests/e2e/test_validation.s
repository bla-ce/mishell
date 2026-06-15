section .text

; runs validation and HELLO tests
; @param  rdi: peer fd
; @return rax: tests passed
test_validation:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> peer fd

  mov   [rsp], rdi

  ; sending invalid magic value should fail
  mov   word [test_packet_t.magic], 0x1234
  mov   word [test_packet_t.flags], FL_HOST

  mov   rax, SYS_WRITE
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receives packet
  mov   rax, SYS_READ
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; op should be OP_ERROR
  cmp   byte [test_packet_t.op], res_ops.ERROR
  jne   .error

  ; payload should be invalid magic
  lea   rdi, [test_packet_t.payload]
  mov   rsi, error_msg.invalid_magic
  mov   rcx, error_msg.invalid_magic_len
  rep   cmpsb
  jne   .error

  ; sending invalid magic value should fail
  mov   word [test_packet_t.magic], 0xCAFE
  mov   byte [test_packet_t.op], req_ops.COUNT
  mov   byte [test_packet_t.flags], FL_HOST

  mov   rax, SYS_WRITE
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receives packet
  mov   rax, SYS_READ
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; op should be OP_ERROR
  cmp   byte [test_packet_t.op], res_ops.ERROR
  jne   .error

  ; get payload
  lea   rdi, [test_packet_t.payload]
  mov   rsi, error_msg.invalid_op
  mov   rcx, error_msg.invalid_op_len
  rep   cmpsb
  jne   .error

  ; sending invalid mode should fail
  mov   word [test_packet_t.magic], 0xCAFE
  mov   byte [test_packet_t.op], req_ops.HELLO
  mov   byte [test_packet_t.flags], 0

  mov   rax, SYS_WRITE
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receives packet
  mov   rax, SYS_READ
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; op should be OP_ERROR
  cmp   byte [test_packet_t.op], res_ops.ERROR
  jne   .error

  ; get payload
  lea   rdi, [test_packet_t.payload]
  mov   rsi, error_msg.invalid_mode
  mov   rcx, error_msg.invalid_mode_len
  rep   cmpsb
  jne   .error

  ; sending HELLO should return OK
  mov   word [test_packet_t.magic], 0xCAFE
  mov   byte [test_packet_t.op], req_ops.HELLO
  mov   byte [test_packet_t.flags], FL_HOST

  mov   rax, SYS_WRITE
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receives packet
  mov   rax, SYS_READ
  mov   rdi, [rsp]
  mov   rsi, test_packet_t
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; op should be OP_ERROR
  cmp   byte [test_packet_t.op], res_ops.OK
  jne   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret
