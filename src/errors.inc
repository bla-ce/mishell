section .data

error_codes:
  .INTERNAL           equ 0
  .INVALID_MAGIC      equ 1
  .INVALID_OP         equ 2
  .INVALID_DIRECTION  equ 3
  .COUNT              equ 4

err_code_to_msg_table:
  .INTERNAL           dq error_msg.internal
  .INVALID_MAGIC      dq error_msg.invalid_magic
  .INVALID_OP         dq error_msg.invalid_op
  .INVALID_DIRECTION  dq error_msg.invalid_direction

err_code_to_len_table:
  .INTERNAL           dw error_msg.internal_len
  .INVALID_MAGIC      dw error_msg.invalid_magic_len
  .INVALID_OP         dw error_msg.invalid_op_len
  .INVALID_DIRECTION  dw error_msg.invalid_direction_len

error_msg:
  .internal db "internal error"
  .internal_len equ $ - error_msg.internal
  .invalid_magic db "invalid magic value in packet"
  .invalid_magic_len equ $ - error_msg.invalid_magic
  .invalid_op db "invalid op value in packet"
  .invalid_op_len equ $ - error_msg.invalid_op
  .invalid_direction db "invalid direction flag in packet"
  .invalid_direction_len equ $ - error_msg.invalid_direction
