# Mishell

Mishell is a service management server written entirely in x86-64 NASM assembly. It acts as a central broker that hosts register with and through which services are registered, started, and managed.

The server listens simultaneously on a TCP socket (port 7474) and a UNIX domain socket (`mishell.sock`), using `epoll`.

## Requirements

- [`nasm`](https://www.nasm.us/) assembler
- `ld` (binutils)
- Python 3 (for e2e tests)
- CPU with `RDRAND` support

## Build

```sh
# build with debug symbols
make

# build stripped binary
make strip

# build and run immediately
make run

# clean build artefacts
make clean
```

The binary is placed at `bin/mishell`.

## Running

```sh
./bin/mishell
```

The server prints to stdout when it accepts connections and receives packets. It does not daemonise. Stop it with `Ctrl-C`.

> **Note:** Data is held in memory only. All registered hosts and services are lost when the server exits.

## Architecture

Four logical entities interact with the server:

| Entity | Flag | Description |
|--------|------|-------------|
| User | `FL_USER` | A client trying to access or manage services |
| Host | `FL_HOST` | A machine that owns and runs services |
| Server | `FL_SERVER` | Mishell itself (used in responses) |
| Service | NOT DEFINED YET | A named process registered under a host |

Current limits:

- Maximum 5 hosts
- Maximum 5 services per host

## Packet protocol

Every message uses the same binary wire format.

### Header (39 bytes, little-endian)

| Field | Size | Description |
|-------|------|-------------|
| `magic` | 2 bytes | Always `0xCAFE` |
| `op` | 1 byte | Operation code |
| `flags` | 2 bytes | Direction + mode bits (see below) |
| `id` | 16 bytes | 128-bit host identifier |
| `destination` | 16 bytes | 128-bit destination identifier |
| `payload_len` | 2 bytes | Byte length of the payload |

Followed by `payload_len` bytes of payload (up to 65535 bytes).

### Flags

Direction and mode bits are OR-ed together in the `flags` field.

| Constant | Value | Meaning |
|----------|-------|---------|
| `FL_CLIENT_TO_SERVER` | `0b0000_0000` | Request (client -> server) |
| `FL_SERVER_TO_CLIENT` | `0b0000_0001` | Response (server -> client) |
| `FL_CLIENT_TO_SERVICE` | `0b0000_0010` | Request (client -> service) |
| `FL_USER` | `0b0001_0000` | Sender is a user |
| `FL_HOST` | `0b0010_0000` | Sender is a host |
| `FL_SERVER` | `0b0100_0000` | Sender is the server (responses only) |

### Request operations

| Op | Code | Allowed mode | Description |
|----|------|--------------|-------------|
| `HELLO` | `0x00` | `FL_USER` or `FL_HOST` | Handshake - server replies `OK` with empty payload |
| `AUTH` | `0x01` | `FL_USER` or `FL_HOST` | Register a new host (empty `id`) or verify an existing one (non-zero `id`). On success the server returns `OK`; new registrations include the 16-byte host `id` in the payload |
| `REGISTER` | `0x02` | `FL_HOST` only | Register a new service under the authenticated host. Payload must be a serialised `service_t` (name + type). Response payload is the full `service_t` with the server-generated `id` |
| `START` | `0x03` | `FL_HOST` only | Mark a service as running. Payload is the 16-byte host `id` followed by the 16-byte service `id` |
| `STOP` | `0x04` | `FL_HOST` only | Mark a service as stopped. Payload is the 16-byte host `id` followed by the 16-byte service `id` |
| `UNREGISTER` | `0x05` | `FL_HOST` only | Remove a service from the host. Service must be stopped first. Payload is the 16-byte host `id` followed by the 16-byte service `id` |

### Response operations

| Op | Code | Description |
|----|------|-------------|
| `OK` | `0x00` | Request succeeded |
| `ERROR` | `0x01` | Request failed; payload contains a human-readable error message |

### Error messages

| Error | Message |
|-------|---------|
| Internal | `internal error` |
| Bad magic | `invalid magic value in packet` |
| Bad op | `invalid op value in packet` |
| Bad direction | `invalid direction flag in packet` |
| Bad mode | `invalid mode flag in packet` |
| Invalid token | `invalid host token` |
| Unauthorised | `user is unauthorized to perform this request` |
| Host limit | `host limit has been reached` |
| Host not found | `host not found` |
| Service limit | `service limit per host has been reached` |
| Service not found | `service not found` |
| Service invalid name | `invalid service name` |
| Service invalid type | `invalid service type` |
| Service not stopped | `stop service before unregistering it` |

## Testing

### End-to-end tests

The e2e suite is a Python script that connects over both TCP and UNIX and exercises every operation, including error paths.

```sh
# server must be running first
make run &
make test-e2e
```

### Unit tests

```sh
make test-unit
```

## Project layout

```
src/
  mishell.s       entry point, epoll event loop
  ops.inc         HELLO / AUTH / REGISTER / START handlers
  packet.inc      packet dispatch, verify, build_error helpers
  host.inc        host struct, host_init, host_get_by_id
  service.inc     service struct, service_init, service_get_by_id
  auth.inc        ID generation via RDRAND
  errors.inc      error code table and message strings

lib/
  constants.inc   syscall numbers, socket and epoll constants
  net.inc         socket structs, net_get_ipv4_from_fd, epoll helpers
  string.inc      string helpers

tests/
  e2e/      end-to-end test suite
  unit/     unit test suite
```

## License

See [LICENSE](LICENSE).
