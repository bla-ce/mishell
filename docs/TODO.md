## Priority 1
- Receive command
- populate port for host
- check padding and alignment for structs
- Make sure we can't have two host on the same ip
- Find a way to validate service types (description and commands length)

## Priority 2
- for host and service get by id, don't look up the whole array
- Define host expiration or host removal process
- add fd to logs
- clean up is never reached, graceful shutdown

## Ongoing
- Update README
- tests for service.inc
- tests for net.inc
- tests for ops.inc
- tests for packet.inc

## Polishing
- make the packet creation generic
- event_t should not be global but statically allocated
- Check for clean architecture and SOLID principles

## Ideas to think about
- authentication for REGISTER, START, STOP and UNREGISTER command
    - who is allow to do what?
- Checksum for payload
