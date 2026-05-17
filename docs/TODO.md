## Priority 1
- test for service
- verify received type in service_init

## Priority 2
- README
- tests for strcpy and strncpy
- tests for net.inc
- tests for ops.inc
- tests for packet.inc
- Define next steps in the workflow and commands used by each entity

## Priority 3
- Define host expiration or host removal process
- add fd to logs
- clean up is never reached, graceful shutdown

## Polishing
- make the packet creation generic
- event_t should not be global but statically allocated
- _start is not respecting SRP
- op_AUTH is not respecting SRP
