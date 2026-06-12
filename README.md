# Mishell

## Overview

Mishell is a service management system that allows you to spin up, manage and query services across a P2P network of mishell instances.

This repository will include built-in services in the future but if you wish to do so, you'll be able to implement your own services to be used with Mishell. Service contract will be defined in the docs soon.

## Prerequisites

- Linux
- [nasm](https://www.nasm.us/)
- `ld` (binutils)
- `python3` to run e2e tests
- CPU with `RDRAND` support

## Installation

To install mishell:

```bash
git clone https://github.com/bla-ce/mishell.git
cd mishell

make
```

The binary is placed at `bin/mishell`.

## Usage

To initialise a new P2P network, run:

```bash
make run-init
```

To join a P2P network, gather the `ip` and the `port` of a running host and run:

```bash
make run-connect IP=<ip> PORT=<port>
```

For more detailed information on how to use mishell, check out the [docs](/docs) (TODO)

## License

See [LICENSE](LICENSE).
