# Changelog

All notable changes to LibStreamX are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-05-22

### Added
- Initial public release.
- `ringbuf` — fixed-capacity circular byte buffer.
- `packet` — packet record type with create / clone / free.
- `parser` — state-machine binary frame parser and metadata tokenizer.
- `logger` — append-mode file / stdout logger.
- `arena` — bump allocator for short-lived parser allocations.
- `config` — `key=value` text config loader.
- `streamx-cli` — CLI front-end with `--config`, `--log`,
  `--max-tokens` and a positional input file.
- AddressSanitizer + UBSan build target (`make asan`).
- Make and CMake build systems; Doxygen configuration for API docs.
- Per-module unit-test binaries runnable via `make test` or `ctest`.

[Unreleased]: https://github.com/libstreamx/libstreamx/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/libstreamx/libstreamx/releases/tag/v0.1.0
