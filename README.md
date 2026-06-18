# Mishell

## Overview

Mishell is a service management system for spinning up, and, managing services across a P2P network of Mishell instances.

## Prerequisites

- Linux
- [nasm](https://www.nasm.us/)
- `ld` (binutils)

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
make run-init PORT=<port> NAME=<name>
```

To join an existing P2P network, provide the IP address and port of a running host:

```bash
make run-connect REMOTE_ADDR=<remote_ip>:<remote_port> PORT=<port> NAME=<name>
```

`PORT` defaults to `7474` if not set.

For more details, see the [docs](/docs).

## License

See [LICENSE](LICENSE).
