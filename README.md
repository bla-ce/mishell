# Mishell

A peer-to-peer service management system. Mishell forms a network of nodes where each node can host and manage services. Clients interact with any node in the network, and Mishell routes requests to the correct host.

## Features

- **P2P Architecture**: No central server - every node is equal
- **Service Orchestration**: Register, start, stop, and query services across the network
- **Built-in Services**: Services are compiled-in plugins, not separate processes
- **Binary Protocol**: Custom network protocol over TCP
- **CLI Client**: `mishli` for interacting with the network
- **Healthcheck**: Hosts are healthchecked every 10sec

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
./bin/mishli --host 127.0.0.1:7474 hello
```

List all hosts in the network:
```bash
./bin/mishli --host 127.0.0.1:7474 network
```

List available service types for the target host:
```bash
./bin/mishli --host 127.0.0.1:7474 catalog
```

Register a service on a host:
```bash
./bin/mishli --host 127.0.0.1:7474 register <host_name> <type> <service_name>
```

Start a service:
```bash
./bin/mishli --host 127.0.0.1:7474 start <host_name> <service_name>
```

Stop a service:
```bash
./bin/mishli --host 127.0.0.1:7474 stop <host_name> <service_name>
```

Query a service:
```bash
./bin/mishli --host 127.0.0.1:7474 query <host_name> <service_name> <command_name>
```

Unregister a service:
```bash
./bin/mishli --host 127.0.0.1:7474 unregister <host_name> <service_name>
```

### Built-in Services

- **PING**: Simple service that responds "pong" to command `PING`

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
