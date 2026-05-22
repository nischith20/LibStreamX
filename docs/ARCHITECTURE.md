# LibStreamX Architecture

This document is the normative reference for the LibStreamX library
and its on-the-wire format. Implementations and tests are expected
to conform to the behavior described here.

## 1. Scope

LibStreamX parses a length-prefixed binary protocol, **STRX**, off a
byte stream. STRX is a project-internal protocol used by LibStreamX
and its CLI front-end; it is not an external standard.

A complete STRX frame is decoded into a `packet_t`. Partial frames
are buffered internally until enough bytes are available to make
forward progress.

## 2. STRX frame format

```
+-------+--------+--------+--------+--------------+----------+
| 'STRX'|   id   |  type  |  len   |   payload    | checksum |
| 4 B   |  4 B   |  2 B   |  2 B   |   len B      |   1 B    |
+-------+--------+--------+--------+--------------+----------+
```

| Field      | Size    | Encoding              | Description                                                 |
|------------|---------|-----------------------|-------------------------------------------------------------|
| `magic`    | 4 bytes | ASCII `STRX`          | Fixed frame prefix used for resynchronization.              |
| `id`       | 4 bytes | unsigned, little-endian  | Per-frame identifier supplied by the producer.              |
| `type`     | 2 bytes | unsigned, little-endian  | Application-defined frame category.                         |
| `len`      | 2 bytes | unsigned, little-endian  | Payload length in bytes (`0 <= len <= MAX_PAYLOAD_SIZE`).   |
| `payload`  | `len`   | XOR-encoded bytes     | Each on-wire byte is XOR'd with `0x55`; decoded internally. |
| `checksum` | 1 byte  | XOR of decoded bytes  | XOR of every byte in the *decoded* payload.                 |

A frame is well-formed iff the magic matches, `len` does not exceed
`MAX_PAYLOAD_SIZE`, and the checksum equals the XOR of all decoded
payload bytes.

## 3. Module layout

```
                +---------------------+
                |       cli.c         |
                |   command-line UI   |
                +----------+----------+
                           |
                           v
            +--------------+--------------+
            |        streamx_init         |
            +--------------+--------------+
                           |
              +------------+------------+
              |            |            |
              v            v            v
        +----------+ +-----------+ +-----------+
        | logger.c | | config.c  | |  arena.c  |
        +----+-----+ +-----+-----+ +-----+-----+
             |             |             |
             |   used by   |   used by   |
             +-----+-------+-------------+
                   v
            +------+------+      +-----------+
            |  ringbuf.c  |----->|  parser.c |
            |  byte queue |      |  state    |
            +-------------+      |  machine  |
                                 +-----+-----+
                                       |
                                       v
                                 +-----+-----+
                                 |  packet.c |
                                 |  frame    |
                                 |  records  |
                                 +-----------+
```

## 4. Module responsibilities

### 4.1 `streamx.h`

Defines library-wide types and the init/shutdown surface. Owns:

- `streamx_status_t` — the universal return code.
- `streamx_config_t` — runtime configuration.
- `streamx_init` / `streamx_shutdown`.

Per-module types (`packet_t`, `parser_t`, `ringbuf_t`, etc.) live in
their respective headers.

### 4.2 `ringbuf`

A fixed-capacity circular byte buffer. Used by the parser as its
input queue.

Operations:

- `ringbuf_create(capacity)`
- `ringbuf_write(rb, src, n)` — push `n` bytes; returns count written.
- `ringbuf_read(rb, dst, n)` — pop `n` bytes; returns count read.
- `ringbuf_peek(rb, dst, n)` — copy without advancing the read cursor.
- `ringbuf_advance(rb, n)` — discard `n` bytes from the head.
- `ringbuf_writable_space(rb)` — bytes available for writing.
- `ringbuf_clear(rb)` — reset cursors; backing buffer retained.
- `ringbuf_free(rb)` — release all memory.

Invariant: `size <= capacity` at all times.

### 4.3 `packet`

A `packet_t` owns a heap-allocated `payload` buffer and a heap-allocated
`tag` string. Each packet is independently freeable.

Operations:

- `packet_create(id, type, payload, length, tag)` — deep-copies
  inputs and returns a fully-owned packet.
- `packet_clone(src)` — returns an independent deep copy of `src`.
- `packet_free(pkt)` — frees the packet and every buffer it owns.

### 4.4 `parser`

A streaming STRX frame parser. Holds a state machine that consumes
bytes from a `ringbuf_t` and emits one `packet_t` per completed
frame. Provides a separate helper for tokenizing `key=value;key=value;...`
metadata strings.

### 4.5 `logger`

Append-mode logger. `logger_init(path)` directs output to a file;
`logger_init(NULL)` directs output to `stdout`. Subsequent
`logger_init` calls before `logger_close` are no-ops.

### 4.6 `arena`

Bump allocator backing short-lived parser-internal allocations.

Operations:

- `arena_create(capacity)`
- `arena_alloc(arena, n)` — returns a pointer into the arena.
  Lifetime is until the next `arena_reset` or `arena_free`.
- `arena_strdup(arena, str)` — convenience.
- `arena_reset(arena)` — resets the bump pointer; all previously
  returned pointers become invalid.
- `arena_free(arena)` — releases the backing buffer.

### 4.7 `config`

Loads a `key=value` text config file. One entry per line; lines
beginning with `#` are comments.

Operations:

- `config_load(path)` — opens, reads, parses; returns a `config_t`.
- `config_get(cfg, key)` — returns the value string or `NULL`.
- `config_get_int(cfg, key, default_val)` — parsed int or default.
- `config_free(cfg)`

### 4.8 `cli` + `main`

Command-line front-end:

```
streamx-cli [--config PATH] [--log PATH] [--max-tokens N] <input.bin>
```

Reads a binary stream from `<input.bin>`, feeds it through the
parser, and logs a summary of every decoded packet.

## 5. Error model

All public functions that can fail return a `streamx_status_t`:

| Code                    | Meaning                                                |
|-------------------------|--------------------------------------------------------|
| `STREAMX_OK`            | Operation completed successfully.                      |
| `STREAMX_ERR_GENERIC`   | Unspecified internal error.                            |
| `STREAMX_ERR_NOMEM`     | Memory allocation failed.                              |
| `STREAMX_ERR_INVALID`   | Invalid argument or state.                             |
| `STREAMX_ERR_IO`        | Input/output system error.                             |
| `STREAMX_ERR_OVERFLOW`  | Buffer or integer overflow detected.                   |
| `STREAMX_ERR_CHECKSUM`  | Payload checksum mismatch.                             |
| `STREAMX_ERR_EOF`       | End of stream reached, or insufficient bytes buffered. |

`STREAMX_ERR_EOF` from `parser_parse_stream` is non-fatal and indicates
that the caller should push more bytes into the ring buffer.
