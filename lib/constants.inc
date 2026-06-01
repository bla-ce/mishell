section .data

SUCCESS_CODE equ 0
FAILURE_CODE equ -1

; syscalls
SYS_READ          equ 0
SYS_WRITE         equ 1
SYS_CLOSE         equ 3
SYS_SOCKET        equ 41
SYS_CONNECT       equ 42
SYS_ACCEPT        equ 43
SYS_BIND          equ 49
SYS_LISTEN        equ 50
SYS_GETPEERNAME   equ 52
SYS_SETSOCKOPT    equ 54
SYS_EXIT          equ 60
SYS_FCNTL         equ 72
SYS_UNLINK        equ 87
SYS_EPOLL_WAIT    equ 232
SYS_EPOLL_CTL     equ 233
SYS_EPOLL_CREATE1 equ 291

; fileno
STDOUT_FILENO equ 1
STDERR_FILENO equ 2

; sockets
AF_LOCAL  equ 1
AF_INET   equ 2

SOCK_STREAM equ 1

SOL_SOCKET    equ 1
SO_REUSEADDR  equ 2

F_GETFL     equ 3
F_SETFL     equ 4
O_NONBLOCK  equ 2048

; epoll
EPOLLIN       equ 1
EPOLLET       equ 0x80000000
EPOLL_CTL_ADD equ 1
EPOLL_CTL_DEL equ 2

; rand
RDRAND_CPUID_POS    equ 30
RDRAND_RETRY_LIMIT  equ 10

; ASCII
NULL_CHAR equ 0
LINE_FEED equ 0xA
HYPHEN    equ 0x2D
DOT       equ 0x2E
ZERO_CHAR equ 0x30
NINE_CHAR equ 0x39
