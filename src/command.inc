section .bss

command_t:
command_t_end:

section .data

COMMAND_T_LEN equ command_t_end - command_t

COMMANDS_LEN equ COMMAND_T_LEN * COMMANDS_MAX_COUNT_PER_SERVICE

COMMANDS_MAX_COUNT_PER_SERVICE equ 0x4

section .text
