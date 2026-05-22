/*
 * ringbuf_basic.c — minimal demonstration of the ringbuf API.
 *
 * Build (from project root):
 *     make
 *     gcc -Iinclude examples/ringbuf_basic.c build/libstreamx.a -o ringbuf_basic
 */

#include <stdio.h>
#include <string.h>

#include "ringbuf.h"

int main(void) {
    ringbuf_t *rb = ringbuf_create(32);
    if (rb == NULL) {
        fprintf(stderr, "ringbuf_create failed\n");
        return 1;
    }

    const char *msg = "hello, streamx";
    uint32_t wrote  = ringbuf_write(rb, (const uint8_t *)msg, (uint32_t)strlen(msg));
    printf("wrote %u byte(s); writable space remaining: %u\n",
           wrote, ringbuf_writable_space(rb));

    uint8_t peek[8] = {0};
    uint32_t peeked = ringbuf_peek(rb, peek, sizeof(peek) - 1);
    printf("peeked %u byte(s): \"%s\"\n", peeked, (char *)peek);

    uint8_t out[64] = {0};
    uint32_t read = ringbuf_read(rb, out, sizeof(out) - 1);
    printf("read   %u byte(s): \"%s\"\n", read, (char *)out);

    ringbuf_free(rb);
    return 0;
}
