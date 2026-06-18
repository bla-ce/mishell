section .rodata

error_codes:
  ; generic
  .INTERNAL                       equ 1

  ; packet validation
  .INVALID_MAGIC                  equ 2
  .INVALID_OP                     equ 3
  .INVALID_MODE                   equ 4

  ; host errors
  .HOST_LIMIT_REACHED             equ 5
  .HOST_NOT_FOUND                 equ 6
  .HOST_ALREADY_EXISTS            equ 7

  ; service errors
  .SERVICE_PER_HOST_LIMIT_REACHED equ 8
  .SERVICE_NOT_FOUND              equ 9
  .SERVICE_INVALID_NAME           equ 10
  .SERVICE_INVALID_TYPE           equ 11
  .SERVICE_STOP_BEFORE_UNREGISTER equ 12
  .SERVICE_NOT_STARTED            equ 13

  ; command errors
  .COMMAND_NOT_FOUND              equ 14

  .COUNT                          equ 15

err_code_to_msg_table:
  .NULL                           dq 0
  .INTERNAL                       dq error_msg.internal
  .INVALID_MAGIC                  dq error_msg.invalid_magic
  .INVALID_OP                     dq error_msg.invalid_op
  .INVALID_MODE                   dq error_msg.invalid_mode
  .HOST_LIMIT_REACHED             dq error_msg.host_limit_reached
  .HOST_NOT_FOUND                 dq error_msg.host_not_found
  .HOST_ALREADY_EXISTS            dq error_msg.host_already_exists
  .SERVICE_PER_HOST_LIMIT_REACHED dq error_msg.service_per_host_limit_reached
  .SERVICE_NOT_FOUND              dq error_msg.service_not_found
  .SERVICE_INVALID_NAME           dq error_msg.service_invalid_name
  .SERVICE_INVALID_TYPE           dq error_msg.service_invalid_type
  .SERVICE_STOP_BEFORE_UNREGISTER dq error_msg.service_stop_before_unregister
  .SERVICE_NOT_STARTED            dq error_msg.service_not_started
  .COMMAND_NOT_FOUND              dq error_msg.command_not_found

err_code_to_len_table:
  .NULL                           dw 0
  .INTERNAL                       dw error_msg.internal_len
  .INVALID_MAGIC                  dw error_msg.invalid_magic_len
  .INVALID_OP                     dw error_msg.invalid_op_len
  .INVALID_MODE                   dw error_msg.invalid_mode_len
  .HOST_LIMIT_REACHED             dw error_msg.host_limit_reached_len
  .HOST_NOT_FOUND                 dw error_msg.host_not_found_len
  .HOST_ALREADY_EXISTS            dw error_msg.host_already_exists_len
  .SERVICE_PER_HOST_LIMIT_REACHED dw error_msg.service_per_host_limit_reached_len
  .SERVICE_NOT_FOUND              dw error_msg.service_not_found_len
  .SERVICE_INVALID_NAME           dw error_msg.service_invalid_name_len
  .SERVICE_INVALID_TYPE           dw error_msg.service_invalid_type_len
  .SERVICE_STOP_BEFORE_UNREGISTER dw error_msg.service_stop_before_unregister_len
  .SERVICE_NOT_STARTED            dw error_msg.service_not_started_len
  .COMMAND_NOT_FOUND              dw error_msg.command_not_found_len

error_msg:
  .internal db "internal error"
  .internal_len equ $ - error_msg.internal
  .invalid_magic db "invalid magic value in packet"
  .invalid_magic_len equ $ - error_msg.invalid_magic
  .invalid_op db "invalid op value in packet"
  .invalid_op_len equ $ - error_msg.invalid_op
  .invalid_mode db "invalid mode flag in packet"
  .invalid_mode_len equ $ - error_msg.invalid_mode
  .host_limit_reached db "host limit has been reached"
  .host_limit_reached_len equ $ - error_msg.host_limit_reached
  .host_not_found db "host not found"
  .host_not_found_len equ $ - error_msg.host_not_found
  .host_already_exists db "a host with this ip and port already exists"
  .host_already_exists_len equ $ - error_msg.host_already_exists
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
  .service_not_started db "service not started"
  .service_not_started_len equ $ - error_msg.service_not_started
  .command_not_found db "command not found"
  .command_not_found_len equ $ - error_msg.command_not_found
