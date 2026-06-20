%include "host.inc"
%include "errors.inc"
%include "command.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

global _start

section .text

_start:
  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

.exit:
  mov   rax, SYS_EXIT
  syscall
