global _start

%include "lib.inc"
%include "command.inc"
%include "host.inc"
%include "packet.inc"
%include "service.inc"
%include "ops.inc"
%include "service_type.inc"

section .rodata

tcp_port equ 7474

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
