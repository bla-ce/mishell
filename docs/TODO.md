## Priority 1
- Find a way to validate service types (description and commands length)
- Receive command (even if the service has not started yet)
- Actually start a service
- Stop a service
- Unregister a service

## Priority 2
- for host and service get by id, don't look up the whole array
- Define host expiration or host removal process
- add fd to logs
- clean up is never reached, graceful shutdown

## Ongoing
- Update README
- tests for service.inc
- tests for strcpy and strncpy
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
- no central server, blockchain example
    - why doing that?
    - are there real benefits or is that just fancy?
    - more peer-to-peer model
