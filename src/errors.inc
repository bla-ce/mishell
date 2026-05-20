section .data

error_codes:
  ; generic
  .INTERNAL                       equ 1

  ; packet validation
  .INVALID_MAGIC                  equ 2
  .INVALID_OP                     equ 3
  .INVALID_DIRECTION              equ 4
  .INVALID_MODE                   equ 5
  .INVALID_TOKEN                  equ 6
  .UNAUTHORIZED                   equ 7

  ; host errors
  .HOST_LIMIT_REACHED             equ 8
  .HOST_NOT_FOUND                 equ 9

  ; service errors
  .SERVICE_PER_HOST_LIMIT_REACHED equ 10
  .SERVICE_NOT_FOUND              equ 11
  .SERVICE_INVALID_NAME           equ 12
  .SERVICE_INVALID_TYPE           equ 13
  .SERVICE_STOP_BEFORE_UNREGISTER equ 14

  .COUNT                          equ 15

err_code_to_msg_table:
  .NULL                           dq 0
  .INTERNAL                       dq error_msg.internal
  .INVALID_MAGIC                  dq error_msg.invalid_magic
  .INVALID_OP                     dq error_msg.invalid_op
  .INVALID_DIRECTION              dq error_msg.invalid_direction
  .INVALID_MODE                   dq error_msg.invalid_mode
  .INVALID_TOKEN                  dq error_msg.invalid_token
  .UNAUTHORIZED                   dq error_msg.unauthorized
  .HOST_LIMIT_REACHED             dq error_msg.host_limit_reached
  .HOST_NOT_FOUND                 dq error_msg.host_not_found
  .SERVICE_PER_HOST_LIMIT_REACHED dq error_msg.service_per_host_limit_reached
  .SERVICE_NOT_FOUND              dq error_msg.service_not_found
  .SERVICE_INVALID_NAME           dq error_msg.service_invalid_name
  .SERVICE_INVALID_TYPE           dq error_msg.service_invalid_type
  .SERVICE_STOP_BEFORE_UNREGISTER dq error_msg.service_stop_before_unregister

err_code_to_len_table:
  .NULL                           dw 0
  .INTERNAL                       dw error_msg.internal_len
  .INVALID_MAGIC                  dw error_msg.invalid_magic_len
  .INVALID_OP                     dw error_msg.invalid_op_len
  .INVALID_DIRECTION              dw error_msg.invalid_direction_len
  .INVALID_MODE                   dw error_msg.invalid_mode_len
  .INVALID_TOKEN                  dw error_msg.invalid_token_len
  .UNAUTHORIZED                   dw error_msg.unauthorized_len
  .HOST_LIMIT_REACHED             dw error_msg.host_limit_reached_len
  .HOST_NOT_FOUND                 dw error_msg.host_not_found_len
  .SERVICE_PER_HOST_LIMIT_REACHED dw error_msg.service_per_host_limit_reached_len
  .SERVICE_NOT_FOUND              dw error_msg.service_not_found_len
  .SERVICE_INVALID_NAME           dw error_msg.service_invalid_name_len
  .SERVICE_INVALID_TYPE           dw error_msg.service_invalid_type_len
  .SERVICE_STOP_BEFORE_UNREGISTER dw error_msg.service_stop_before_unregister_len

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
  .unauthorized db "user is unauthorized to perform this request"
  .unauthorized_len equ $ - error_msg.unauthorized
  .host_limit_reached db "host limit has been reached"
  .host_limit_reached_len equ $ - error_msg.host_limit_reached
  .host_not_found db "host not found"
  .host_not_found_len equ $ - error_msg.host_not_found
  .service_per_host_limit_reached db "service limit per host has been reached"
  .service_per_host_limit_reached_len equ $ - error_msg.service_per_host_limit_reached
  .service_not_found db "service not found"
  .service_not_found_len equ $ - error_msg.service_not_found
  .service_invalid_name db "invalid service name"
  .service_invalid_name_len equ $ - error_msg.service_invalid_name
  .service_invalid_type db "invalid service type"
  .service_invalid_type_len equ $ - error_msg.service_invalid_type
  .service_stop_before_unregister db "stop service before unregistering it"
  .service_stop_before_unregister_len equ $ - error_msg.service_stop_before_unregister
