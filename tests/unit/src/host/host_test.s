global _start

%include "host.inc"
%include "lib.inc"
%include "ops.inc"
%include "packet.inc"
%include "service.inc"
%include "service_type.inc"

section .bss

section .text
_start:
  ; host init with empty struct should error
  mov   rdi, 0
  mov   rsi, 0
  call  host_init
  cmp   rax, FAILURE_CODE
  jne   .error

  ; TODO: find a way to test host init with a fd
  mov   rdi, SUCCESS_CODE
  jmp   .exit

.error:
  mov   rdi, FAILURE_CODE

.exit:
  mov   rax, SYS_EXIT
  syscall
