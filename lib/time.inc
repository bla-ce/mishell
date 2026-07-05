section .bss

timespec_t:
  .tv_sec   resq 1
  .tv_nsec  resq 1
timespec_t_end:

section .rodata

TIMESPEC_STRUCT_LEN equ timespec_t_end - timespec_t

TIMESPEC_OFF_TV_SEC  equ 0x0
TIMESPEC_OFF_TV_NSEC equ TIMESPEC_OFF_TV_SEC + 0x8

section .text

; return now's unix timestamp
; @return rax: timestamp
time_now:
  sub   rsp, TIMESPEC_STRUCT_LEN

  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [rsp]
  syscall
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp]
  mov   rax, qword [rdi+TIMESPEC_OFF_TV_SEC]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, TIMESPEC_STRUCT_LEN
  ret
