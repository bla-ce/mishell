# Mishell

## Overview

Mishell is a service management system for spinning up, managing, and querying services across a P2P network of Mishell instances.

Built-in services are planned for a future release. You can also implement your own, the service contract will be documented in the [docs](/docs).

## Prerequisites

- Linux
- [nasm](https://www.nasm.us/)
- `ld` (binutils)
- CPU with `RDRAND` support

## Installation

```bash
git clone https://github.com/bla-ce/mishell.git
cd mishell
make
```

The binary is placed at `bin/mishell`.

## Usage

To initialize a new P2P network:

```bash
make run-init PORT=<port>
```

To join an existing P2P network, provide the IP address and port of a running host:

```bash
make run-connect REMOTE_IP=<remote_ip> REMOTE_PORT=<remote_port> PORT=<port>
```

`PORT` defaults to `7474` if not set.

For more details, see the [docs](/docs).

## License

See [LICENSE](LICENSE).
