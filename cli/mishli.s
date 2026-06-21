%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"
%include "cli_op.inc"

global _start

section .rodata

mishli_usage_str db ""
  db "Usage: mishli --host HOST_ADDR OPTION [ARGS...]", LINE_FEED, LINE_FEED
  db "Discovery Commands:", LINE_FEED
  db "  hello                             Check if host is up", LINE_FEED
  db "  catalog                           List available service types", LINE_FEED
  db "  network                           List available hosts", LINE_FEED, LINE_FEED
  db "Management Commands", LINE_FEED
  db "  register HOST TYPE SERVICE_NAME   Register new service", LINE_FEED
  db "  start HOST SERVICE_NAME           Start service", LINE_FEED
  db "  stop HOST SERVICE_NAME            Stop service", LINE_FEED
  db "  unregister HOST SERVICE_NAME      Unregister service", LINE_FEED, LINE_FEED
  db "Query Services:", LINE_FEED
  db "  query HOST SERVICE_NAME COMMAND   Query a running service", LINE_FEED, NULL_CHAR

MISHLI_MIN_ARG equ 0x4

HOST_FLAG db "--host", NULL_CHAR

section .data

host_fd dq 0

section .text

_start:
  ; STACK USAGE
  ; [rsp]       -> argc
  ; [rsp+0x8]   -> pointer to program name
  ; [rsp+0x10]  -> pointer to host flag
  ; [rsp+0x18]  -> pointer to host address
  ; [rsp+0x20]  -> pointer to op
  ; [rsp+0x28]  -> first argument (host id)
  ; [rsp+0x30]  -> second argument (service id)
  ; [rsp+0x38]  -> third argument (service type or command)

  cmp   qword [rsp], MISHLI_MIN_ARG
  jl    .usage

  ; make sure host flag is set
  mov   rdi, [rsp+0x10]
  mov   rsi, HOST_FLAG
  call  strcmp
  cmp   rax, TRUE
  jne   .usage

  ; connect to remote host
  mov   rdi, [rsp+0x18]
  call  mishli_connect_to_host
  cmp   rax, 0
  jl    .usage

  ; check if op is valid
  mov   rdi, [rsp+0x20]
  call  mishli_cli_op_fn_from_str
  cmp   rax, 0
  jl    .usage

  ; populate base packet
  mov   word [packet_t.magic], MAGIC_VALUE
  mov   byte [packet_t.flags], FL_USER
  mov   byte [packet_t.dest_host], 0

  ; call cli function
  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x30]
  mov   rdx, [rsp+0x38]
  call  rax
  cmp   rax, 0
  jl    .error

  ; close fd
  mov   rax, SYS_CLOSE
  mov   rdi, [host_fd]
  syscall

  mov   rdi, SUCCESS_CODE
  jmp   .exit

.usage:
  mov   rdi, mishli_usage_str
  call  println

.error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

.exit:
  mov   rax, SYS_EXIT
  syscall

; connects to the host before sending the request
; @param  rdi: pointer to the host address
; @return rax: return code
mishli_connect_to_host:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the host address
  ; [rsp+0x8]   -> ip of the remote host
  ; [rsp+0x10]  -> port of the remote host

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; parse host address
.loop:
  ; find colon
  cmp   byte [rdi], COLON
  jne   .continue

  mov   rax, NULL_CHAR
  stosb

  jmp   .loop_end

.continue:
  cmp   byte [rdi], NULL_CHAR
  je    .error    ; wrong format

  inc   rdi

  jmp   .loop
.loop_end:
  ; rdi points to the port
  call  atoi
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  call  inet_pton
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  net_tcp_connect_to_remote_addr
  cmp   rax, 0
  jl    .error

  mov   [host_fd], rax

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, SYS_CLOSE
  mov   rdi, [host_fd]
  syscall

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret
