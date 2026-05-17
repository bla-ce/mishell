## Priority 1
- test for service and string functions
- deserialize_service method
- do some verification against the service
- think about some uniqueness

## Priority 2
- README
- Define next steps in the workflow and commands used by each entity

## Priority 3
- Define host expiration or host removal process
- add fd to logs
- clean up is never reached, graceful shutdown

## Polishing
- write unit tests for fns and ops (it should not be too hard)
- make the packet creation generic
- event_t should not be global but statically allocated
- _start is not respecting SRP
- op_AUTH is not respecting SRP
