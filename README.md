# LibStreamX

[![Build and Test](https://img.shields.io/badge/build-passing-brightgreen)](.github/workflows/ci.yml)
[![Sanitizers](https://img.shields.io/badge/asan%2Fubsan-enabled-blue)](Makefile)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

`LibStreamX` is a small, modular C library for parsing a length-prefixed
binary stream protocol (`STRX`). It ships as a static library, a
command-line front-end, a sanitizer-clean test suite, and CI.

See [CONTRIBUTING.md](CONTRIBUTING.md) for how to submit a PR,
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for the protocol and
module layout, and [docs/API.md](docs/API.md) for the function-level
API reference.

---

## Modules

| Module                                | Responsibility                                              |
|---------------------------------------|-------------------------------------------------------------|
| [`streamx.h`](include/streamx.h)      | Top-level types, status codes, configuration struct.        |
| [`ringbuf.c`](src/ringbuf.c)          | Fixed-capacity circular byte buffer.                        |
| [`packet.c`](src/packet.c)            | Packet struct, allocation, cloning, free.                   |
| [`parser.c`](src/parser.c)            | State-machine binary frame parser + metadata tokenizer.     |
| [`logger.c`](src/logger.c)            | Append-mode file / stdout logger.                           |
| [`arena.c`](src/arena.c)              | Bump-allocator arena used by the parser for short-lived data.|
| [`config.c`](src/config.c)            | `key=value` config file loader.                             |
| [`cli.c`](src/cli.c) + [`main.c`](src/main.c) | Command-line front-end that wires the pieces together. |

A diagram of how they connect lives in
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## Building

You need `gcc` (or `clang`) and `make`.

```
make                  # build static lib + test runner + cli
make test             # run the unit tests
make asan             # build everything with AddressSanitizer + UBSan
make clean
```

Or with CMake:

```
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

---

## Examples

Self-contained example programs live in [examples/](examples/). Build
them after `make` and run, e.g.:

```
gcc -Iinclude examples/parse_frame.c build/libstreamx.a -o parse_frame
./parse_frame
```

---

## License

MIT — see [LICENSE](LICENSE).
