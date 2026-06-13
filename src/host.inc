section .rodata

FIRST_HOST_CMD    db "init", NULL_CHAR
CONNECT_HOST_CMD  db "connect", NULL_CHAR
PORT_FLAG         db "--port", NULL_CHAR

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

host_id resb ID_LEN

section .data

hosts times HOSTS_LEN db 0
curr_host_idx db 0

host_port dw 7474
host_ip   dd 0

section .text
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

  ; get the host from the destination id of the packet
  mov   rdi, [rsp]
  add   rdi, PACKET_T_OFF_DEST_HOST
  call  host_get_by_id
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; init packet for host
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  xor   rdx, rdx
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; populate sockaddr
  mov   word [sockaddr_in_t.sin_family], AF_INET

  ; get port and ip from host struct
  mov   rsi, [rsp+0x10]
  mov   ax, [rsi+HOST_T_OFF_PORT]
  mov   word [sockaddr_in_t.sin_port], ax
  mov   eax, [rsi+HOST_T_OFF_IP]
  mov   dword [sockaddr_in_t.sin_addr], eax
  mov   qword [sockaddr_in_t.sin_zero], 0

  ; connect to host
  mov   rax, SYS_CONNECT
  mov   rdi, [rsp+0x18]
  mov   rsi, sockaddr_in_t
  mov   rdx, qword [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

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
; @param  rdi: pointer to the command
; @param  rsi: ip of the remote host (optional)
; @param  rdx: port of the remote host (optional, byte order)
; @return rax: return code
init_or_join_network:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the command
  ; [rsp+0x8]   -> ip of the remote host
  ; [rsp+0x10]  -> port of the remote host

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

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
  call  host_init
  test  rax, rax
  jnz   .error

  ; increase host count
  inc   byte [curr_host_idx]

  ; populate host id (first in the array)
  mov   rsi, hosts
  add   rsi, HOST_T_OFF_ID
  mov   rdi, host_id
  mov   rcx, ID_QWORDS
  rep   movsq

  jmp   .done

.connect_to_host:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  host_connect_to_remote
  cmp   rax, 0
  jl    .error

.done:
  mov   rax, SUCCESS_CODE
  jmp   .return

.usage:
  mov   rax, SYS_WRITE
  mov   rdi, STDERR_FILENO
  mov   rsi, usage_str
  mov   rdx, usage_str_len
  syscall

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; connects to a remote host by sending HELLO and AUTH commands
; assuming local networks for now
; @param  rdi: pointer to the ip of the remote host
; @param  rsi: pointer to the port of the remote host (byte order)
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
  mov   rdi, [rsp+0x8]
  call  atoi
  cmp   rax, 0
  jl    .error

  xchg  al, ah ; bswap 16-bit registers
  mov   word [sockaddr_in_t.sin_port], ax

  ; get ip from cli
  mov   rdi, [rsp]
  mov   rsi, sockaddr_in_t.sin_addr
  call  inet_pton
  cmp   rax, 0
  jl    .error

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
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_PEER_TO_PEER
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_HOST
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   qword [rdi+PACKET_T_OFF_DEST_HOST], 0
  mov   qword [rdi+PACKET_T_OFF_DEST_HOST+0x8], 0
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

  ; send auth command
  lea   rdi, [rsp+0x18]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_PEER_TO_PEER
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_HOST
  mov   qword [rdi+PACKET_T_OFF_ID], 0
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], 0
  mov   qword [rdi+PACKET_T_OFF_DEST_HOST], 0
  mov   qword [rdi+PACKET_T_OFF_DEST_HOST+0x8], 0
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.AUTH
  mov   word [rdi+PACKET_T_OFF_PAYLOAD_LEN], HOST_T_LEN

  ; add host to payload
  mov   rdi, host_t
  movsx rsi, dword [host_ip]
  movzx rdx, word [host_port]
  xchg  dl, dh  ; host init receives network order
  call  host_init
  test  rax, rax
  jnz   .error

  lea   rdi, [rsp+0x18]
  add   rdi, PACKET_T_OFF_PAYLOAD
  mov   rsi, host_t
  mov   rcx, HOST_T_LEN
  rep   movsb

  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, PACKET_T_HEADER_LEN+HOST_T_LEN
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

  ; get own id from destination field
  lea   rsi, [rsp+0x18]
  add   rsi, PACKET_T_OFF_DEST_HOST
  mov   rdi, host_id
  mov   rcx, ID_QWORDS
  rep   movsq

  ; get number of hosts from payload len
  ; TODO: without div? probably aligning the struct to do shr
  lea   rsi, [rsp+0x18]
  movzx rax, word [rsi+PACKET_T_OFF_PAYLOAD_LEN]

  xor   rdx, rdx
  mov   rcx, HOST_T_LEN
  idiv  rcx
  mov   byte [curr_host_idx], al

  ; close connection
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x10]
  syscall

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
  cmp   r9, r10
  jge   .error

  lea   rdi, [hosts+r9]

  mov   [rsp+0x8], rdi

  add   rdi, HOST_T_OFF_ID
  mov   rsi, [rsp]
  mov   rcx, ID_QWORDS
  rep   cmpsq
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
; @return rax: return code or error code
host_init:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the host struct
  ; [rsp+0x8]   -> ip of the host
  ; [rsp+0x10]  -> port of the host (host order)
  ; [rsp+0x18]  -> error code

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], error_codes.INTERNAL  ; default

  test  rdi, rdi
  jz    .error

  test  rdx, rdx
  jz    .error

  mov   qword [rsp+0x18], error_codes.HOST_ALREADY_EXISTS
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

  mov   qword [rsp+0x18], error_codes.INTERNAL  ; default
  call  generate_id
  cmp   rax, FAILURE_CODE
  je    .error

  mov   rsi, [rsp]
  mov   qword [rsi+HOST_T_OFF_ID], rdx
  mov   qword [rsi+HOST_T_OFF_ID+0x8], rax

  mov   eax, dword [rsp+0x8]
  mov   dword [rsi+HOST_T_OFF_IP], eax

  mov   ax, word [rsp+0x10]
  mov   word [rsi+HOST_T_OFF_PORT], ax

  mov   rax, SUCCESS_CODE
  jmp    .return

.error:
  mov   rax, [rsp+0x18]

.return:
  add   rsp, 0x20
  ret

; updates each host in the network with new array except the one
; passed in rdi.
; TODO: arguments should be an array of host to not update with
; count
host_update_network:
  push  r12
  push  r13
  sub   rsp, 0x20+PACKET_T_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the host id to skip (can be null)
  ; [rsp+0x8]   -> array length
  ; [rsp+0x10]  -> sock fd
  ; [rsp+0x18]  -> temp host
  ; [rsp+0x20]  -> packet

  mov   [rsp], rdi

  ; get array length
  movzx r12, byte [curr_host_idx]
  test  r12, r12
  jz    .done
  imul  rax, r12, HOST_T_LEN
  mov   [rsp+0x8], rax

  ; build packet
  lea   rdi, [rsp+0x20]
  mov   word [rdi+PACKET_T_OFF_MAGIC], MAGIC_VALUE
  mov   byte [rdi+PACKET_T_OFF_OP], req_ops.UPDATE
  mov   word [rdi+PACKET_T_OFF_FLAGS], FL_PEER_TO_PEER
  or    word [rdi+PACKET_T_OFF_FLAGS], FL_HOST

  mov   rax, qword [host_id]
  mov   rdx, qword [host_id+0x8]
  mov   qword [rdi+PACKET_T_OFF_ID], rax
  mov   qword [rdi+PACKET_T_OFF_ID+0x8], rdx

  mov   ax, [rsp+0x8]
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
  mov   [rsp+0x18], rdi

  ; check if we need to skip the host
  add   rdi, HOST_T_OFF_ID
  mov   rsi, host_id
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .skip_host

  mov   rdi, [rsp+0x18]
  add   rdi, HOST_T_OFF_ID
  mov   rsi, [rsp]
  mov   rcx, ID_QWORDS
  rep   cmpsq
  je    .skip_host

  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  xor   rdx, rdx
  syscall
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; populate sockaddr
  mov   word [sockaddr_in_t.sin_family], AF_INET

  ; get port and ip from host struct
  mov   rdi, [rsp+0x18]
  mov   ax, [rdi+HOST_T_OFF_PORT]
  mov   word [sockaddr_in_t.sin_port], ax
  mov   eax, [rdi+HOST_T_OFF_IP]
  mov   dword [sockaddr_in_t.sin_addr], eax
  mov   qword [sockaddr_in_t.sin_zero], 0

  ; connect to host
  mov   rax, SYS_CONNECT
  mov   rdi, [rsp+0x10]
  mov   rsi, sockaddr_in_t
  mov   rdx, qword [sockaddr_in_t_len]
  syscall
  cmp   rax, 0
  jl    .error

  ; send the packet to the host
  mov   rax, SYS_WRITE
  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x20]
  mov   rdx, PACKET_T_LEN
  syscall
  cmp   rax, 0
  jl    .error

  ; close connection
  mov   rax, SYS_CLOSE
  mov   rdi, [rsp+0x10]
  syscall

.skip_host:
  inc   r13
  jmp   .loop

.done:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20+PACKET_T_LEN
  pop   r13
  pop   r12
  ret
