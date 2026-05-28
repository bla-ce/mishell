section .rodata

FIRST_HOST_FLAG     db "--first-host"
FIRST_HOST_FLAG_LEN equ $ - FIRST_HOST_FLAG -1

HOST_MAX_COUNT equ 0x5

HOSTS_LEN equ HOST_T_LEN * HOST_MAX_COUNT

HOST_T_LEN equ host_t_end - host_t

SERVICES_LEN equ SERVICE_T_LEN * SERVICE_MAX_COUNT_PER_HOST

HOST_T_OFF_ID             equ 0x0
HOST_T_OFF_IP             equ HOST_T_OFF_ID + ID_LEN
HOST_T_OFF_PORT           equ HOST_T_OFF_IP + 0x4
HOST_T_OFF_SERVICES       equ HOST_T_OFF_PORT + 0x2
HOST_T_OFF_SERVICES_COUNT equ HOST_T_OFF_SERVICES + SERVICES_LEN

section .bss

host_t:
  .id             resb ID_LEN
  .ipv4           resd 0x1
  .port           resw 0x1
  .services       resb SERVICES_LEN
  .services_count resb 0x1
  .pad            resb 0x5
host_t_end:

section .data

hosts times HOSTS_LEN db 0
curr_host_idx db 0

section .text

; connects to a remote host by sending HELLO and AUTH commands
; assuming local networks for now
; @param  rdi: pointer to the ip of the remote host
; @param  rsi: pointer to the port of the remote host
; @return rax: return code
host_connect_to_remote:
  sub   rsp, 0x18 + PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the ip of the remote host
  ; [rsp+0x8]   -> pointer to the port of the remote host
  ; [rsp+0x10]  -> host fd
  ; [rsp+0x18]  -> packet

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; create socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  xor   rdx, rdx
  syscall

  mov   [rsp+0x10], rax

  ; populate sockaddr: TODO: we might want to use the stack here
  mov   word [sockaddr_in_t.sin_family], AF_INET

  ; port to network order
  ; mov   rdi, [rsp+0x8]
  mov   rdi, 7474
  mov   rax, rdi
  xchg  al, ah ; bswap 16-bit registers

  mov   word [sockaddr_in_t.sin_port], ax
  mov   dword [sockaddr_in_t.sin_addr], 0
  mov   qword [sockaddr_in_t.sin_zero], 0

  ; connect to socket
  mov   rax, SYS_CONNECT
  mov   rdi, [rsp+0x10]
  mov   rsi, sockaddr_in_t
  mov   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; send hello command
  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_CLIENT_TO_SERVER
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_HOST
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.HELLO
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0

  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receive response
  mov   rax, SYS_READ
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; confirm response is ok
  lea   rdi, [rsp+0x18]
  cmp   byte [rdi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error

  ; host closes connection, need to close and reconnect
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x10]
  syscall

  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  xor   rdx, rdx
  syscall

  mov   [rsp+0x10], rax

  mov   rax, SYS_CONNECT
  mov   rdi, [rsp+0x10]
  mov   rsi, sockaddr_in_t
  mov   rdx, [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; send auth command
  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_CLIENT_TO_SERVER
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_HOST
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.AUTH
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0

  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receive response
  mov   rax, SYS_READ
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; confirm response is ok
  lea   rsi, [rsp+0x18]
  cmp   byte [rsi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error

  ; get host array from payload
  movzx rcx, word [rsi+PACKET_T_OFF_PAYLOAD_LEN]
  add   rsi, PACKET_T_OFF_PAYLOAD
  mov   rdi, hosts
  rep   movsb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18 + PACKET_T_LEN
  ret


; returns the host struct from the host id or -1 if the host is
; not registered
; @param  rdi: pointer to the id of the host
; @return rax: pointer to the host struct or -1
host_get_by_id:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the id of the host
  ; [rsp+0x8] -> pointer to the host struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  xor   r9, r9

  ; get host max offset
  movzx rax, byte [curr_host_idx]
  imul  r10, rax, HOST_T_LEN

.loop:
  lea   rdi, [hosts+r9]

  mov   [rsp+0x8], rdi

  add   rdi, HOST_T_OFF_ID
  mov   rsi, [rsp]
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .loop_end

  add   r9, HOST_T_LEN

  cmp   r9, r10
  jge   .error

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; populates the host struct with ip and generates its id
; an empty connection fd is not an error, the host is considered
; local
; TODO: will be a problem when we need the port
; @param  rdi: pointer to the host struct
; @param  rsi: connection fd
; @return rax: return code
host_init:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the host struct
  ; [rsp+0x8]   -> connection fd

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; memset host to 0
  mov   rsi, [rsp]
  xor   rax, rax
  mov   rcx, HOST_T_LEN
  rep   stosb

  call  generate_id
  cmp   rax, FAILURE_CODE
  je    .error

  mov   rsi, [rsp]
  mov   qword [rsi+HOST_T_OFF_ID], rdx
  mov   qword [rsi+HOST_T_OFF_ID+0x8], rax

  ; check if the host is local
  cmp   qword [rsp+0x8], 0
  je    .done

  mov   rdi, [rsp+0x8]
  call  net_get_ipv4_from_fd
  cmp   eax, 0
  jl    .error

  mov   rsi, [rsp]
  mov   dword [rsi+HOST_T_OFF_IP], eax

.done:
  mov   rax, SUCCESS_CODE
  jmp    .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret
