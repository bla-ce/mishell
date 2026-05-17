## Priority 1
- Implement START command
- Create Pong service type
- Actually start a service

## Priority 2
- README
- tests for service.inc
- tests for strcpy and strncpy
- tests for net.inc
- tests for ops.inc
- tests for packet.inc

## Priority 3
- Define host expiration or host removal process
- add fd to logs
- clean up is never reached, graceful shutdown

## Polishing
- make the packet creation generic
- event_t should not be global but statically allocated
- _start is not respecting SRP
- op_AUTH is not respecting SRP

## Ideas to think about
- authentication for REGISTER, START, STOP and UNREGISTER command
    - who is allow to do what?
- no central server, blockchain example
