section .data

error_codes:
  .INTERNAL           equ 0
  .INVALID_MAGIC      equ 1
  .INVALID_OP         equ 2
  .INVALID_DIRECTION  equ 3
  .INVALID_MODE       equ 4
  .INVALID_TOKEN      equ 5
  .HOST_LIMIT_REACHED equ 6
  .HOST_NOT_FOUND     equ 7
  .UNAUTHORIZED       equ 8
  .COUNT              equ 9

err_code_to_msg_table:
  .INTERNAL           dq error_msg.internal
  .INVALID_MAGIC      dq error_msg.invalid_magic
  .INVALID_OP         dq error_msg.invalid_op
  .INVALID_DIRECTION  dq error_msg.invalid_direction
  .INVALID_MODE       dq error_msg.invalid_mode
  .INVALID_TOKEN      dq error_msg.invalid_token
  .HOST_LIMIT_REACHED dq error_msg.host_limit_reached
  .HOST_NOT_FOUND     dq error_msg.host_not_found
  .UNAUTHORIZED       dq error_msg.unauthorized

err_code_to_len_table:
  .INTERNAL           dw error_msg.internal_len
  .INVALID_MAGIC      dw error_msg.invalid_magic_len
  .INVALID_OP         dw error_msg.invalid_op_len
  .INVALID_DIRECTION  dw error_msg.invalid_direction_len
  .INVALID_MODE       dw error_msg.invalid_mode_len
  .INVALID_TOKEN      dw error_msg.invalid_token_len
  .HOST_LIMIT_REACHED dw error_msg.host_limit_reached_len
  .HOST_NOT_FOUND     dw error_msg.host_not_found_len
  .UNAUTHORIZED       dw error_msg.unauthorized_len

error_msg:
  .internal db "internal error"
  .internal_len equ $ - error_msg.internal
  .invalid_magic db "invalid magic value in packet"
  .invalid_magic_len equ $ - error_msg.invalid_magic
  .invalid_op db "invalid op value in packet"
  .invalid_op_len equ $ - error_msg.invalid_op
  .invalid_direction db "invalid direction flag in packet"
  .invalid_direction_len equ $ - error_msg.invalid_direction
  .invalid_mode db "invalid mode flag in packet"
  .invalid_mode_len equ $ - error_msg.invalid_mode
  .invalid_token db "invalid host token"
  .invalid_token_len equ $ - error_msg.invalid_token
  .host_limit_reached db "max host limit has been reached"
  .host_limit_reached_len equ $ - error_msg.host_limit_reached
  .host_not_found db "host not found"
  .host_not_found_len equ $ - error_msg.host_not_found
  .unauthorized db "user is unauthorized to perform this request"
  .unauthorized_len equ $ - error_msg.unauthorized
