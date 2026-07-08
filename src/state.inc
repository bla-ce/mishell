section .rodata

state_path db "state", NULL_CHAR

section .data

section .text

; saves the current service state to a file
; @return rax: return code
state_save:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> state fd

  ; open file
  mov   rax, SYS_OPEN
  mov   rdi, state_path
  mov   rsi, O_CREAT | O_TRUNC | O_WRONLY
  mov   rdx, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; get own host
  mov   rdi, host_name
  call  host_get_by_name
  cmp   rax, 0
  jl    .error

  mov   rsi, rax
  add   rsi, HOST_T_OFF_LAST_UP

  ; write state to the file (last up, services and services count)
  mov   rax, SYS_WRITE
  mov   rdi, [rsp]
  mov   rdx, HOST_STATE_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; close the file
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp]
  syscall

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp]
  syscall

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; retrieves service state from a file
; @return rax: return code
state_load:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> fd

  ; open the file
  mov   rax, SYS_OPEN
  mov   rdi, state_path
  mov   rsi, O_RDONLY
  xor   rdx, rdx
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; move cursor at the beginning of the file
  mov   rax, SYS_LSEEK
  mov   rdi, [rsp]
  xor   rsi, rsi
  xor   rdx, rdx
  syscall

  mov   rdi, host_name
  call  host_get_by_name
  cmp   rax, 0
  jl    .error

  mov   rsi, rax
  add   rsi, HOST_T_OFF_LAST_UP

  mov   rax, SYS_READ
  mov   rdi, [rsp]
  mov   rdx, HOST_STATE_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; close the file
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp]
  syscall

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret
