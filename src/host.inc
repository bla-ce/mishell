section .rodata

FIRST_HOST_CMD    db "init", NULL_CHAR
CONNECT_HOST_CMD  db "connect", NULL_CHAR
PORT_FLAG         db "--port", NULL_CHAR
NAME_FLAG         db "--name", NULL_CHAR

HOST_MAX_COUNT equ 0x5

HOSTS_LEN equ HOST_T_LEN * HOST_MAX_COUNT

HOST_T_LEN equ host_t_end - host_t

SERVICES_LEN equ SERVICE_T_LEN * SERVICE_MAX_COUNT_PER_HOST

HOST_NAME_MAX_LEN equ 0x20

HOST_T_OFF_IP             equ 0x0
HOST_T_OFF_PORT           equ HOST_T_OFF_IP + 0x4
HOST_T_OFF_NAME           equ HOST_T_OFF_PORT + 0x2
HOST_T_OFF_STATUS         equ HOST_T_OFF_NAME + HOST_NAME_MAX_LEN
HOST_T_OFF_LAST_UP        equ HOST_T_OFF_STATUS + 0x1
HOST_T_OFF_SERVICES       equ HOST_T_OFF_LAST_UP + 0x8
HOST_T_OFF_SERVICES_COUNT equ HOST_T_OFF_SERVICES + SERVICES_LEN

HOST_EXPIRY equ HOST_HEARTBEAT * 2

HOST_STATUS:
  .DOWN equ 0x0
  .UP   equ 0x1

HOST_STATUS_STR:
  .DOWN db "DOWN", NULL_CHAR
  .UP   db "UP", NULL_CHAR

host_status_str_table dq HOST_STATUS_STR.DOWN
                      dq HOST_STATUS_STR.UP

HOST_HEARTBEAT equ 10

log_host_is_down    db " is down", NULL_CHAR
log_host_is_expired db " is expired, removing it from the network", NULL_CHAR

usage_str db "Usage: mishell COMMAND [ARGS...]", LINE_FEED, LINE_FEED
          db "Available Commands:", LINE_FEED
          db "  init            --port PORT --name NAME   Initialize a new network", LINE_FEED
          db "  connect IP:PORT --port PORT --name NAME   Join an existing network", LINE_FEED, NULL_CHAR

section .bss

host_t:
  .ipv4           resd 0x1
  .port           resw 0x1
  .name           resb HOST_NAME_MAX_LEN
  .status         resb 0x1
  .last_up        resq 0x1
  .services       resb SERVICES_LEN
  .services_count resb 0x1
host_t_end:

host_name resb HOST_NAME_MAX_LEN

section .data

hosts times HOSTS_LEN db 0
curr_host_idx db 0

host_port dw 0
host_ip   dd 0

default_host_name db "default", NULL_CHAR

section .text
; checks it the host should be removed (now - last up > HOST_EXPIRY)
; @param  rdi: pointer to the host
; @return rax: host is expired (boolean)
host_is_expired:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the host struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  call  time_now
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]

  sub   rax, qword [rdi+HOST_T_OFF_LAST_UP]

  cmp   rax, HOST_EXPIRY
  jge   .is_expired

  mov   rax, FALSE
  jmp   .return

.is_expired:
  mov   rax, TRUE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

host_hearbeat:
  push  r12
  push  r13
  sub   rsp, 0x18+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> array length
  ; [rsp+0x8]   -> sock fd
  ; [rsp+0x18]  -> packet

  ; read timer fd value to reset timer
  mov   rax, SYS_READ
  mov   rdi, [timer_fd]
  lea   rsi, [rsp]  ; using rsp for convenience here
  mov   rdx, 8
  syscall

  ; get array length
  movzx r12, byte [curr_host_idx]
  test  r12, r12
  jz    .done
  imul  rax, r12, HOST_T_LEN
  mov   [rsp], rax

  ; build packet
  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.HELLO
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_HOST
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], 0

  xor   r13, r13

.loop:
  cmp   r13, r12
  jge   .done

  ; get host
  imul  rax, r13, HOST_T_LEN
  lea   rdi, [hosts+rax]
  mov   [rsp+0x10], rdi

  ; check if we need to skip the host
  add   rdi, HOST_T_OFF_NAME
  mov   rsi, host_name
  call  strcmp
  cmp   rax, TRUE
  je    .next_host

  mov   rax, [rsp+0x10]
  movsx rdi, dword [rax+HOST_T_OFF_IP]

  movzx rdx, word [rax+HOST_T_OFF_PORT]
  xchg  dl, dh
  mov   rsi, rdx
  call  net_tcp_connect_to_remote_addr
  cmp   rax, 0
  jl    .host_down

  mov   [rsp+0x8], rax

  ; send the packet to the host
  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_HEADER_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; if response is not OK, host is down
  lea   rsi, [rsp+0x18]
  cmp   byte [rsi+PACKET_T_OFF_OP], res_ops.OK
  jne   .host_down

  call  time_now
  cmp   rax, 0
  jl    .error

  ; host is up
  mov   rdi, [rsp+0x10]
  mov   byte [rdi+HOST_T_OFF_STATUS], HOST_STATUS.UP
  mov   qword [rdi+HOST_T_OFF_LAST_UP], rax

  ; close connection
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x8]
  syscall

  jmp   .next_host

.host_down:
  mov   rdi, [rsp+0x10]
  mov   byte [rdi+HOST_T_OFF_STATUS], HOST_STATUS.DOWN

  add   rdi, HOST_T_OFF_NAME
  call  print

  ; log host is down
  mov   rdi, log_host_is_down
  call  println

  ; check if host is expired and should be removed
  mov   rdi, [rsp+0x10]
  call  host_is_expired
  cmp   rax, TRUE
  jne   .next_host

  ; log host is expired
  mov   rdi, [rsp+0x10]
  add   rdi, HOST_T_OFF_NAME
  call  print

  ; log host is down
  mov   rdi, log_host_is_expired
  call  println

  ; remove host
  mov   rdi, [rsp+0x10]
  xor   al, al
  mov   rcx, HOST_T_LEN
  rep   stosb

  dec   byte [curr_host_idx]

.next_host:
  inc   r13
  jmp   .loop

.done:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+PACKET_T_LEN
  pop   r13
  pop   r12
  ret

; @param  rdi: pointer to the request packet
; @param  rsi: pointer to the response packet
; @return rax: return code
host_proxy_request:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the request packet
  ; [rsp+0x8]   -> pointer to the response packet
  ; [rsp+0x10]  -> pointer to the host
  ; [rsp+0x18]  -> host fd

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get the host from the destination name of the packet
  mov   rdi, [rsp]
  add   rdi, PACKET_T_OFF_DEST_HOST
  call  host_get_by_name
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rax, [rsp+0x10]
  movsx rdi, dword [rax+HOST_T_OFF_IP]

  movzx rdx, word [rax+HOST_T_OFF_PORT]
  xchg  dl, dh
  mov   rsi, rdx
  call  net_tcp_connect_to_remote_addr
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; send the packet to the host
  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rax, SYS_READ
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; close connection
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x18]
  syscall

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; checks if a host with a given ip/port pair already exists
; @param  rdi: ip of the host
; @param  rsi: port of the host (byte order)
; @return rax: same host exists (boolean)
host_addr_exists:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> ip of the host
  ; [rsp+0x8] -> port of the host

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; ip can be null
  test  rsi, rsi
  jz    .error

  xor   r9, r9  ; counter

.loop:
  cmp   r9b, byte [curr_host_idx]
  jge   .loop_end

  imul  rax, r9, HOST_T_LEN
  lea   rdi, [hosts+rax]

  mov   eax, [rsp]
  cmp   dword [rdi+HOST_T_OFF_IP], eax
  jne   .next

  mov   ax, [rsp+0x8]
  cmp   word [rdi+HOST_T_OFF_PORT], ax
  je    .exists

.next:
  inc   r9b
  jmp   .loop

.loop_end:
  ; does not exist
  mov   rax, FALSE
  jmp   .return

.exists:
  mov   rax, TRUE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; initialises a P2P network if init is passed, otherwise joins
; an existing network with ip and port
; @param  rdi: pointer to the op
; @param  rsi: addr of the remote host (optional)
; @param  rdx: pointer to the name of the current host
; @return rax: return code
init_or_join_network:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the op
  ; [rsp+0x8]   -> addr of the remote host
  ; [rsp+0x10]  -> addr of the remote host

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; copy host name
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error
  cmp   rax, HOST_NAME_MAX_LEN
  jge   .error

  mov   rsi, [rsp+0x10]
  mov   rdi, host_name
  mov   rcx, rax
  rep   movsb

  ; check if the host is the first one (init passed)
  cmp   qword [rsp], 0
  je    .connect_to_host

  mov   rdi, CONNECT_HOST_CMD
  mov   rsi, [rsp]  ; second CLI argument
  call  strcmp
  cmp   rax, TRUE
  je    .connect_to_host

  ; make sure it's init
  mov   rdi, FIRST_HOST_CMD
  mov   rsi, [rsp]  ; second CLI argument
  call  strcmp
  cmp   rax, TRUE
  jne   .usage

  ; init first host
  mov   rdi, hosts
  movsx rsi, dword [host_ip]
  movzx rdx, word [host_port]
  xchg  dh, dl
  mov   rcx, [rsp+0x10]
  call  host_init
  test  rax, rax
  jnz   .error

  ; add UP status to host
  mov   rdi, hosts
  mov   byte [rdi+HOST_T_OFF_STATUS], HOST_STATUS.UP

  ; increase host count
  inc   byte [curr_host_idx]

  jmp   .done

.connect_to_host:
  ; parse host address
  mov   rdi, [rsp+0x8]

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
  mov   rsi, rdi
  mov   rdi, [rsp+0x8]
  mov   rdx, [rsp+0x10]
  call  host_connect_to_remote
  cmp   rax, 0
  jl    .error

.done:
  mov   rax, SUCCESS_CODE
  jmp   .return

.usage:
  mov   rdi, usage_str
  call  println

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; connects to a remote host by sending JOIN op
; assuming local networks for now
; @param  rdi: pointer to the ip of the remote host
; @param  rsi: pointer to the port of the remote host (byte order)
; @param  rdx: pointer to the name of the remote host
; @return rax: return code
host_connect_to_remote:
  push  r12
  push  r13
  sub   rsp, 0x20 + PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the ip of the remote host
  ; [rsp+0x8]   -> pointer to the port of the remote host
  ; [rsp+0x10]  -> pointer to the name of the remote host
  ; [rsp+0x18]  -> host fd
  ; [rsp+0x20]  -> packet

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; get ip from cli
  mov   rdi, [rsp]
  call  inet_pton
  cmp   rax, 0
  jl    .error

  mov   r12, rax

  ; port to network order
  mov   rdi, [rsp+0x8]
  call  atoi
  cmp   rax, 0
  jl    .error

  mov   r13, rax

  mov   rdi, r12
  mov   rsi, r13
  call  net_tcp_connect_to_remote_addr
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; send join op
  lea   rdi, [rsp+0x20]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_HOST
  mov   qword [rdi+PACKET_T_OFF_DEST_HOST], 0
  mov   qword [rdi+PACKET_T_OFF_DEST_HOST+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.JOIN
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], HOST_T_LEN

  ; add host to payload
  mov   rdi, host_t
  movsx rsi, dword [host_ip]
  movzx rdx, word [host_port]
  xchg  dl, dh  ; host init receives network order
  mov   rcx, [rsp+0x10]
  call  host_init
  test  rax, rax
  jnz   .error

  lea   rdi, [rsp+0x20]
  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, host_t
  mov   rcx, HOST_T_LEN
  rep   movsb

  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x18]
  lea   rsi, [rsp+0x20]
  mov   rdx, PACKET_T_HEADER_LEN+HOST_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; receive response
  mov   rax, SYS_READ
  mov   rdi, [rsp+0x18]
  lea   rsi, [rsp+0x20]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; confirm response is ok
  lea   rsi, [rsp+0x20]
  cmp   byte [rsi+PACKET_T_OFF_OP], res_ops.OK
  jne   .error

  ; get host array from payload
  movzx rcx, word [rsi+PACKET_T_OFF_PAYLOAD_LEN]
  add   rsi, PACKET_T_OFF_PAYLOAD
  mov   rdi, hosts
  rep   movsb

  ; get number of hosts from payload len
  lea   rsi, [rsp+0x20]
  movzx rax, word [rsi+PACKET_T_OFF_PAYLOAD_LEN]

  xor   rdx, rdx
  mov   rcx, HOST_T_LEN
  div   rcx
  mov   byte [curr_host_idx], al

  ; close connection
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x18]
  syscall

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x18]
  syscall

  mov   rax, FAILURE_CODE

.return:
  pop   r12
  pop   r13
  add   rsp, 0x20 + PACKET_T_LEN
  ret

; returns the host struct from the host name or -1 if the host is
; not registered
; @param  rdi: pointer to the name of the host
; @return rax: pointer to the host struct or -1
host_get_by_name:
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
  cmp   r9, r10
  jge   .error

  lea   rdi, [hosts+r9]

  mov   [rsp+0x8], rdi

  add   rdi, HOST_T_OFF_NAME
  mov   rsi, [rsp]
  call  strcmp
  cmp   rax, TRUE
  je    .loop_end

  add   r9, HOST_T_LEN

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; populates the host struct with ip, port and generates its id
; @param  rdi: pointer to the host struct
; @param  rsi: ip of the host
; @param  rdx: port of the host (network order)
; @param  rcx: pointer to the name of the host
; @return rax: return code or error code
host_init:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the host struct
  ; [rsp+0x8]   -> ip of the host
  ; [rsp+0x10]  -> port of the host (network)
  ; [rsp+0x18]  -> pointer to the name of the host
  ; [rsp+0x20]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   qword [rsp+0x20], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rcx, rcx
  jz    .error

  mov   qword [rsp+0x20], error_codes.HOST_NAME_ALREADY_EXISTS
  mov   rdi, [rsp+0x18]
  call  host_get_by_name
  cmp   rax, 0
  jge   .error

  mov   qword [rsp+0x20], error_codes.HOST_ADDR_ALREADY_EXISTS
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  host_addr_exists
  ; rax should be 0 (FALSE)
  test  rax, rax
  jnz   .error

  ; memset host to 0
  mov   rdi, [rsp]
  xor   rax, rax
  mov   rcx, HOST_T_LEN
  rep   stosb

  mov   rsi, [rsp]
  mov   eax, dword [rsp+0x8]
  mov   dword [rsi+HOST_T_OFF_IP], eax

  mov   ax, word [rsp+0x10]
  mov   word [rsi+HOST_T_OFF_PORT], ax

  ; copy name
  mov   qword [rsp+0x20], error_codes.INTERNAL  ; default
  mov   rdi, [rsp+0x18]
  call  strlen
  cmp   rax, 0
  jl    .error
  cmp   rax, HOST_NAME_MAX_LEN
  jge   .error

  mov   rdi, [rsp]
  add   rdi, HOST_T_OFF_NAME
  mov   rsi, [rsp+0x18]
  mov   rcx, rax
  rep   movsb

  mov   rax, SUCCESS_CODE
  jmp    .return

.error:
  mov   rax, [rsp+0x20]

.return:
  add   rsp, 0x28
  ret

; updates each host in the network with new host array
; @return rax: return code
host_update_network:
  push  r12
  push  r13
  sub   rsp, 0x18+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp+0]     -> array length
  ; [rsp+0x8]   -> sock fd
  ; [rsp+0x10]  -> temp host
  ; [rsp+0x18]  -> packet

  ; get array length
  movzx r12, byte [curr_host_idx]
  test  r12, r12
  jz    .done
  imul  rax, r12, HOST_T_LEN
  mov   [rsp], rax

  ; build packet
  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.UPDATE
  mov   byte [rdi+PACKET_T_OFF_FLAGS], FL_HOST

  mov   ax, [rsp]
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], ax

  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, hosts
  movzx rcx, ax
  rep   movsb

  xor   r13, r13

.loop:
  cmp   r13, r12
  jge   .done

  ; get host
  imul  rax, r13, HOST_T_LEN
  lea   rdi, [hosts+rax]
  mov   [rsp+0x10], rdi

  ; check if we need to skip the host
  add   rdi, HOST_T_OFF_NAME
  mov   rsi, host_name
  call  strcmp
  cmp   rax, TRUE
  je    .next_host

  mov   rax, [rsp+0x10]
  movsx rdi, dword [rax+HOST_T_OFF_IP]

  movzx rdx, word [rax+HOST_T_OFF_PORT]
  xchg  dl, dh
  mov   rsi, rdx
  call  net_tcp_connect_to_remote_addr
  cmp   rax, 0
  jl    .next_host

  mov   [rsp+0x8], rax

  ; send the packet to the host
  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; close connection
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x8]
  syscall

.next_host:
  inc   r13
  jmp   .loop

.done:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18+PACKET_T_LEN
  pop   r13
  pop   r12
  ret
