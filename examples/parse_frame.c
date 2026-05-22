/*
 * parse_frame.c — build a valid STRX frame in memory and decode it.
 *
 * Frame layout (see docs/ARCHITECTURE.md):
 *   'STRX' (4) | id (4 LE) | type (2 LE) | len (2 LE) | payload (XOR 0x55) | checksum (XOR 0x55)
 *
 * Build (from project root):
 *     make
 *     gcc -Iinclude examples/parse_frame.c build/libstreamx.a -o parse_frame
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet.h"
#include "parser.h"
#include "ringbuf.h"
#include "streamx.h"

static size_t encode_frame(uint8_t  *out,
                           uint32_t  id,
                           uint16_t  type,
                           const uint8_t *payload,
                           uint16_t  len) {
    size_t off = 0;

    memcpy(out + off, "STRX", 4);                off += 4;
    memcpy(out + off, &id,   sizeof(id));        off += sizeof(id);
    memcpy(out + off, &type, sizeof(type));      off += sizeof(type);
    memcpy(out + off, &len,  sizeof(len));       off += sizeof(len);

    uint8_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        out[off++] = payload[i] ^ 0x55;
        checksum  ^= payload[i];
    }
    out[off++] = checksum ^ 0x55;

    return off;
}

int main(void) {
    streamx_status_t s = streamx_init(NULL);
    if (s != STREAMX_OK) {
        fprintf(stderr, "streamx_init failed: %d\n", s);
        return 1;
    }

    const uint8_t payload[] = "ping";
    const uint16_t payload_len = (uint16_t)(sizeof(payload) - 1);

    uint8_t  wire[64];
    size_t   wire_len = encode_frame(wire, 0x2026u, 0x01u, payload, payload_len);
    printf("encoded frame: %zu byte(s)\n", wire_len);

    ringbuf_t *rb     = ringbuf_create(256);
    parser_t  *parser = parser_create(16);
    if (rb == NULL || parser == NULL) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }

    ringbuf_write(rb, wire, (uint32_t)wire_len);

    packet_t *pkt = NULL;
    s = parser_parse_stream(parser, rb, &pkt);
    if (s != STREAMX_OK || pkt == NULL) {
        fprintf(stderr, "parse failed: %d\n", s);
        parser_free(parser);
        ringbuf_free(rb);
        streamx_shutdown();
        return 1;
    }

    printf("decoded packet: id=%u type=%u len=%u payload=\"%.*s\"\n",
           pkt->id, pkt->type, pkt->length, (int)pkt->length, (char *)pkt->payload);

    packet_free(pkt);
    parser_free(parser);
    ringbuf_free(rb);
    streamx_shutdown();
    return 0;
}
