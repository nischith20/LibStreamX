# LibStreamX API Reference

This document is a function-level reference for the public C API exposed
by LibStreamX. For the broader protocol description and module
diagram, see [ARCHITECTURE.md](ARCHITECTURE.md).

All headers live in [`include/`](../include/). Every type and function
listed here is `extern "C"`-safe; the headers themselves wrap their
declarations in `extern "C"` when included from C++.

## Contents

- [Core (`streamx.h`)](#core-streamxh)
- [Ring buffer (`ringbuf.h`)](#ring-buffer-ringbufh)
- [Packet (`packet.h`)](#packet-packeth)
- [Parser (`parser.h`)](#parser-parserh)
- [Arena (`arena.h`)](#arena-arenah)
- [Config (`config.h`)](#config-configh)
- [Logger (`logger.h`)](#logger-loggerh)
- [CLI (`cli.h`)](#cli-clih)
- [Error model](#error-model)

---

## Core (`streamx.h`)

### Version macros

```c
#define STREAMX_VERSION_MAJOR 0
#define STREAMX_VERSION_MINOR 1
#define STREAMX_VERSION_PATCH 0
#define STREAMX_VERSION       "0.1.0"
```

### `streamx_status_t`

Universal return code. Negative on failure, `STREAMX_OK` (= 0) on
success. See [Error model](#error-model) for the full table.

### `streamx_config_t`

Runtime configuration passed to `streamx_init`.

| Field                 | Type        | Meaning                                              |
|-----------------------|-------------|------------------------------------------------------|
| `max_packet_size`     | `uint32_t`  | Absolute upper bound for a single packet.            |
| `max_tokens`          | `uint16_t`  | Per-stream metadata token capacity.                  |
| `enable_shadow_mode`  | `int`       | If non-zero, run with strict validation.             |
| `log_file_path`       | `const char *` | Optional log destination; `NULL` => stdout.       |

### `streamx_init` / `streamx_shutdown`

```c
streamx_status_t streamx_init(const streamx_config_t *config);
void             streamx_shutdown(void);
```

Initialise / tear down global library state. `config == NULL` requests
implementation defaults.

---

## Ring buffer (`ringbuf.h`)

A fixed-capacity circular byte buffer used as the parser's input queue.

### `ringbuf_t`

```c
typedef struct {
    uint8_t  *buffer;
    uint32_t  capacity;
    uint32_t  head;
    uint32_t  tail;
    uint32_t  size;
} ringbuf_t;
```

Invariant: `0 <= size <= capacity` at all times. Touch fields directly
only at your own risk; the public API maintains the invariants for you.

### `ringbuf_create`

```c
ringbuf_t *ringbuf_create(uint32_t capacity);
```

Allocate a ring buffer with the given backing-buffer capacity. Returns
`NULL` on allocation failure.

### `ringbuf_write`

```c
uint32_t ringbuf_write(ringbuf_t *rb, const uint8_t *data, uint32_t len);
```

Push up to `len` bytes from `data` into the buffer. Returns the number
of bytes actually written, which may be less than `len` if the buffer
is near full. Returns `0` for `NULL` arguments.

### `ringbuf_read`

```c
uint32_t ringbuf_read(ringbuf_t *rb, uint8_t *out_data, uint32_t len);
```

Pop up to `len` bytes into `out_data` and advance the read cursor.
Returns the number of bytes actually read; never reads past `size`.

### `ringbuf_peek`

```c
uint32_t ringbuf_peek(const ringbuf_t *rb, uint8_t *out_data, uint32_t len);
```

Copy up to `len` bytes into `out_data` *without* advancing the read
cursor. Useful for protocol probes that need to look ahead before
committing to consume.

### `ringbuf_advance`

```c
void ringbuf_advance(ringbuf_t *rb, uint32_t len);
```

Discard up to `len` bytes from the head. Clamped to `size`.

### `ringbuf_writable_space`

```c
uint32_t ringbuf_writable_space(const ringbuf_t *rb);
```

Return the number of bytes that can be written without overflowing.

### `ringbuf_clear` / `ringbuf_free`

```c
void ringbuf_clear(ringbuf_t *rb);
void ringbuf_free(ringbuf_t *rb);
```

`clear` resets the cursors but retains the backing buffer. `free`
releases the buffer and the `ringbuf_t` itself. Both are NULL-safe.

---

## Packet (`packet.h`)

An owned record describing a single STRX frame.

### `packet_t`

```c
typedef struct {
    uint32_t  id;
    uint16_t  type;
    uint32_t  length;
    uint8_t  *payload;
    char     *tag;
} packet_t;
```

`payload` and `tag` are independently heap-allocated and freed by
`packet_free`.

### `packet_create`

```c
packet_t *packet_create(uint32_t id,
                        uint16_t type,
                        const uint8_t *payload,
                        uint32_t length,
                        const char *tag);
```

Allocate a packet, **deep-copying** `payload` and `tag` into freshly
malloc'd storage. Returns `NULL` on allocation failure. Callers retain
ownership of the input buffers.

### `packet_clone`

```c
packet_t *packet_clone(const packet_t *src);
```

Return a copy of `src`. Returns `NULL` if `src == NULL` or on
allocation failure.

### `packet_free`

```c
void packet_free(packet_t *pkt);
```

Free `pkt` and every buffer it owns. NULL-safe.

---

## Parser (`parser.h`)

A streaming, state-machine STRX frame decoder.

### Constants

```c
#define MAX_PAYLOAD_SIZE 4096
```

Hard cap on a single frame's payload length. Larger frames are rejected
with `STREAMX_ERR_OVERFLOW`.

### `parser_state_t`

```c
typedef enum {
    STATE_MAGIC,
    STATE_HEADER,
    STATE_PAYLOAD,
    STATE_CHECKSUM
} parser_state_t;
```

The internal state cursor. Most callers do not need to inspect this.

### `metadata_token_t`

```c
typedef struct {
    char *key;
    char *value;
} metadata_token_t;
```

A single parsed metadata key/value pair, owned by the parser.

### `parser_t`

The streaming context returned by `parser_create`. Fields are public
for inspection but should not be modified by the caller.

### `parser_create`

```c
parser_t *parser_create(uint16_t max_tokens);
```

Allocate a fresh parser whose internal token store reserves
`max_tokens` slots. Returns `NULL` on allocation failure.

### `parser_parse_stream`

```c
streamx_status_t parser_parse_stream(parser_t   *parser,
                                     ringbuf_t  *rb,
                                     packet_t  **out_packet);
```

Drain bytes from `rb` and attempt to decode one frame.

Returns:

| Status                 | Meaning                                                              |
|------------------------|----------------------------------------------------------------------|
| `STREAMX_OK`           | A complete frame was decoded; `*out_packet` is the owned result.     |
| `STREAMX_ERR_EOF`      | Stream exhausted before a frame completed - push more bytes and retry.|
| `STREAMX_ERR_OVERFLOW` | Frame declared a length > `MAX_PAYLOAD_SIZE`.                        |
| `STREAMX_ERR_CHECKSUM` | Checksum did not match the decoded payload.                          |
| `STREAMX_ERR_NOMEM`    | Allocation failed during frame assembly.                             |
| `STREAMX_ERR_INVALID`  | One of the arguments was `NULL`.                                     |

On any non-OK return, the internal state is reset to `STATE_MAGIC` so
the next call resynchronises on the next `STRX` magic.

### `parser_tokenize_metadata`

```c
streamx_status_t parser_tokenize_metadata(parser_t   *parser,
                                          const char *meta_str);
```

Tokenize a `key=value;key=value;...` string and append each pair to the
parser's internal token store. Returns `STREAMX_ERR_OVERFLOW` if the
store would grow past its reserved capacity, `STREAMX_ERR_INVALID` for
malformed input.

### `parser_free`

```c
void parser_free(parser_t *parser);
```

Release the parser, all in-flight payload assembly buffers, and all
tokens it owns.

---

## Arena (`arena.h`)

Bump allocator used internally by the parser for short-lived data. The
public API is exposed so callers can share an arena with the library.

### `arena_t`

```c
typedef struct {
    uint8_t  *base;
    uint32_t  capacity;
    uint32_t  offset;
    uint32_t  high_water;
} arena_t;
```

`high_water` records the largest `offset` ever reached; it is preserved
across `arena_reset` calls and is useful for capacity planning.

### `arena_create`

```c
arena_t *arena_create(uint32_t capacity);
```

Allocate an arena with `capacity` backing bytes. Returns `NULL` if
`capacity == 0` or on allocation failure.

### `arena_alloc`

```c
void *arena_alloc(arena_t *a, uint32_t size);
```

Reserve `size` bytes (rounded up to an 8-byte alignment). Returns
`NULL` if the request would not fit. The returned pointer is valid
until the next `arena_reset` or `arena_free`.

### `arena_strdup`

```c
char *arena_strdup(arena_t *a, const char *str);
```

Copy a NUL-terminated string into the arena. Returns `NULL` on
exhaustion.

### `arena_reset` / `arena_free`

```c
void arena_reset(arena_t *a);
void arena_free(arena_t *a);
```

`reset` rewinds the bump pointer to zero, invalidating every pointer
previously returned by `arena_alloc`. `free` releases the backing
buffer and the arena itself. Both are NULL-safe.

---

## Config (`config.h`)

`key = value` text config loader. One entry per line, lines beginning
with `#` are comments, whitespace around `=` is tolerated, and values
may be wrapped in `"..."` quotes which are stripped.

### `config_entry_t` / `config_t`

```c
typedef struct {
    char *key;
    char *value;
} config_entry_t;

typedef struct {
    config_entry_t *entries;
    uint32_t        count;
    uint32_t        capacity;
} config_t;
```

### `config_load`

```c
config_t *config_load(const char *path);
```

Open `path`, parse it, and return a heap-allocated `config_t`. Returns
`NULL` if the file cannot be opened.

### `config_get`

```c
const char *config_get(const config_t *cfg, const char *key);
```

Look up `key`. Returns the value string (still owned by the config) or
`NULL` if not present.

### `config_get_int`

```c
int config_get_int(const config_t *cfg, const char *key, int default_val);
```

Like `config_get`, but parses the value as an integer. Returns
`default_val` when the key is absent.

### `config_free`

```c
void config_free(config_t *cfg);
```

Release the config and every owned string. NULL-safe.

---

## Logger (`logger.h`)

Append-mode logger writing either to a file or to stdout.

### Log-level macros

```c
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 4
```

### `logger_init`

```c
streamx_status_t logger_init(const char *log_file);
```

Open `log_file` in append mode, or direct output to stdout if
`log_file == NULL`. Returns `STREAMX_ERR_IO` if the file could not be
opened. Subsequent calls before `logger_close` are no-ops.

### `logger_log`

```c
void logger_log(int level, const char *fmt, ...);
```

`printf`-style formatted log line, prefixed with the level tag
(`[INFO]`, `[WARN]`, etc.).

### `logger_close`

```c
void logger_close(void);
```

Flush and release the log destination.

---

## CLI (`cli.h`)

Parsed command-line state and helpers used by `streamx-cli`. Linkable
from your own host program if you want to reuse the same option layout.

### `cli_options_t`

```c
typedef struct {
    char input_path[256];
    char config_path[256];
    char log_path[256];
    int  max_tokens;
    int  verbose;
    int  has_input;
    int  has_config;
    int  has_log;
} cli_options_t;
```

### `cli_parse`

```c
int cli_parse(int argc, char **argv, cli_options_t *out);
```

Populate `*out` from `argv`. Returns:

- `0` on success,
- `1` if the user asked for `--help` (caller should exit cleanly),
- `2` if arguments were malformed (caller should exit with error).

### `cli_print_help`

```c
void cli_print_help(const char *progname);
```

Print the usage block to stdout.

### `cli_run`

```c
int cli_run(const cli_options_t *opts);
```

Open the input file, drive the parser end-to-end, and log a per-packet
summary. Returns `0` on success, non-zero on I/O failure.

---

## Error model

All status-returning APIs use `streamx_status_t`:

| Code                    | Numeric | Meaning                                                |
|-------------------------|---------|--------------------------------------------------------|
| `STREAMX_OK`            |   `0`   | Operation completed successfully.                      |
| `STREAMX_ERR_GENERIC`   |  `-1`   | Unspecified internal error.                            |
| `STREAMX_ERR_NOMEM`     |  `-2`   | Memory allocation failed.                              |
| `STREAMX_ERR_INVALID`   |  `-3`   | Invalid argument or state.                             |
| `STREAMX_ERR_IO`        |  `-4`   | Input/output system error.                             |
| `STREAMX_ERR_OVERFLOW`  |  `-5`   | Buffer or integer overflow detected.                   |
| `STREAMX_ERR_CHECKSUM`  |  `-6`   | Payload checksum mismatch.                             |
| `STREAMX_ERR_EOF`       |  `-7`   | End of stream / insufficient bytes buffered.           |

`STREAMX_ERR_EOF` from `parser_parse_stream` is non-fatal: the caller
should push more bytes into the ring buffer and retry. Every other
non-OK return indicates the current frame was abandoned and the parser
has been reset to look for the next `STRX` magic.
