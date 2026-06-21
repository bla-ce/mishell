# Mishell

A peer-to-peer service management system. Mishell forms a network of nodes where each node can host and manage services. Clients interact with any node in the network, and Mishell routes requests to the correct host.

## Features

- **P2P Architecture**: No central server - every node is equal
- **Service Orchestration**: Register, start, stop, and query services across the network
- **Built-in Services**: Services are compiled-in plugins, not separate processes
- **Binary Protocol**: Custom network protocol over TCP
- **CLI Client**: `mishli` for interacting with the network

## Prerequisites

- Linux (x86-64)
- [NASM](https://www.nasm.us/)
- `ld` (GNU binutils linker)

## Installation

```bash
git clone https://github.com/bla-ce/mishell.git
cd mishell
make
```

Binaries are placed in `bin/`.

## Usage

### Start a Network

Initialize the first node (creates a new network):
```bash
make run-init PORT=7474 NAME=node1
```

Join an existing network:
```bash
make run-connect REMOTE_ADDR=<ip>:<port> PORT=7474 NAME=node2
```

`PORT` defaults to `7474`, `NAME` defaults to `default`.

### CLI Client (mishli)

Build the CLI:
```bash
make mishli
```

Check if a host is up:
```bash
./bin/mishli --host 127.0.0.1:7474 HELLO
```

List all hosts in the network:
```bash
./bin/mishli --host 127.0.0.1:7474 NETWORK
```

List available service types for the target host:
```bash
./bin/mishli --host 127.0.0.1:7474 CATALOG
```

Register a service on a host:
```bash
# NOTE: for now, type has to be the index, you can use the CATALOG command to get the index
./bin/mishli --host 127.0.0.1:7474 REGISTER <host_name> <type_index> <service_name>
```

Start a service:
```bash
./bin/mishli --host 127.0.0.1:7474 START <host_name> <service_name>
```

Stop a service:
```bash
./bin/mishli --host 127.0.0.1:7474 STOP <host_name> <service_name>
```

Query a service (e.g., ping):
```bash
# NOTE: for now, command has to be the index, CATALOG will output available commands in a future release
./bin/mishli --host 127.0.0.1:7474 QUERY <host_name> <service_name> 0
```

Unregister a service:
```bash
./bin/mishli --host 127.0.0.1:7474 UNREGISTER <host_name> <service_name>
```

### Built-in Services

- **PING** (`type=0`): Simple service that responds "pong" to command `0`

## Tests

Run unit tests:
```bash
make test-unit
```

Run end-to-end tests:
```bash
make test-e2e
```

## License

MIT License - see [LICENSE](LICENSE) for details.
