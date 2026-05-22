## Priority 1

## Priority 2
- check padding and alignment for structs
- any command for service / host status?
- make the packet creation generic (zero it out and populate)
- last updated for services?
- populate port for host
- Make sure we can't have two host on the same ip
- Find a way to validate service types (description and commands length)
- Define host expiration or host removal process
- add fd to logs
- clean up is never reached, graceful shutdown
- split e2e tests

## Ongoing
- Update README
- tests for service.inc
- tests for net.inc
- tests for ops.inc
- tests for packet.inc

## Polishing
- event_t should not be global but statically allocated
- Check for clean architecture and SOLID principles

## Ideas to think about
- authentication for REGISTER, START, STOP and UNREGISTER command
    - who is allow to do what?
- Checksum for payload
- as the number of hosts grows we don't need the host to know about each host
- Set a target for mishell size, 10Kb? 20kb?
