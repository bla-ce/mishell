%include "command.inc"
%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

global _start

section .rodata

MISHLI_MIN_ARG equ 3

mishli_usage_str  db "usage:  mishli cmd hello                              -> check if host is up", LINE_FEED
                  db "        mishli cmd auth                               -> generate or validate token", LINE_FEED
                  db "        mishli cmd register <host_id> <type>          -> register new service", LINE_FEED
                  db "        mishli cmd start <host_id> <service_id>       -> start service", LINE_FEED
                  db "        mishli cmd stop <host_id> <service_id>        -> stop service", LINE_FEED
                  db "        mishli cmd unregister <host_id> <service_id>  -> unregister service", LINE_FEED
                  db "        mishli cmd list                               -> list available hosts", LINE_FEED
                  db "        mishli <op> <host_id> <service_id> <payload>  -> op for service ", LINE_FEED, NULL_CHAR
mishli_usage_str_len equ $ - mishli_usage_str

section .text

_start:
  ; STACK USAGE
  ; [rsp]     -> argc

  cmp   qword [rsp], MISHLI_MIN_ARG
  jl    .usage

  mov   rax, SUCCESS_CODE
  jmp   .exit

.usage:
  mov   rax, SYS_WRITE
  mov   rdi, STDERR_FILENO
  mov   rsi, mishli_usage_str
  mov   rdx, mishli_usage_str_len
  syscall

.error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

.exit:
  mov   rax, SYS_EXIT
  syscall
