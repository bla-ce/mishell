section .bss

hosts resb HOST_MAX_LEN * SESSION_ID_LEN

section .data

HOST_MAX_LEN      equ 8
SESSION_ID_LEN    equ 16
SESSION_ID_QWORDS equ SESSION_ID_LEN / 8

curr_host_idx db 0

section .text

; generates a random 64 bit number
; @return
  ; rdx: error
  ; rax: generated number
rand_generate_number:
  mov   rax, 1
  cpuid

  shr   rcx, RDRAND_CPUID_POS
  and   rcx, 1
  test  rcx, rcx
  jz    .error

  xor   rcx, rcx
.loop:
  rdrand  rax
  jc    .return

  inc   rcx

  cmp   rcx, RDRAND_RETRY_LIMIT
  jge   .error

  mov   rdx, SUCCESS_CODE

  jmp   .loop

.error:
  mov   rdx, FAILURE_CODE

.return:
  ret

; generates a 16 bytes long session id for the host
; @return rdx:rax: session id
generate_host_session_id:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> first 64-bit generated number
  ; [rsp+0x8] -> second 64-bit generated number

  call  rand_generate_number
  cmp   rdx, FAILURE_CODE
  je    .error

  mov   [rsp], rax

  call  rand_generate_number
  cmp   rdx, FAILURE_CODE
  je    .error

  mov   [rsp+0x8], rax

  mov   rdx, [rsp]
  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
