# Examples

Small standalone programs that exercise the LibStreamX public API.

Build the library first (`make` from the project root produces
`build/libstreamx.a`), then compile any example against it:

```
gcc -Iinclude examples/parse_frame.c   build/libstreamx.a -o parse_frame
gcc -Iinclude examples/ringbuf_basic.c build/libstreamx.a -o ringbuf_basic
```

| Example                                       | What it shows                                                |
|-----------------------------------------------|--------------------------------------------------------------|
| [`ringbuf_basic.c`](ringbuf_basic.c)          | Creating a `ringbuf_t`, writing, peeking, reading.           |
| [`parse_frame.c`](parse_frame.c)              | Building a valid STRX frame in memory and decoding it.       |
