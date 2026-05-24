/*
 * test_parser.c - unit tests for the STRX stream parser.
 *
 * Frames are constructed in memory according to docs/ARCHITECTURE.md:
 *
 *   'STRX' (4) | id (4 LE) | type (2 LE) | len (2 LE) | payload^0x55 | checksum^0x55
 *
 * SPDX-License-Identifier: MIT
 */

#include "packet.h"
#include "parser.h"
#include "ringbuf.h"
#include "test_helpers.h"

#include <stdio.h>
#include <string.h>

static size_t encode_frame(uint8_t *out,
                           uint32_t id,
                           uint16_t type,
                           const uint8_t *payload,
                           uint16_t len) {
    size_t off = 0;
    memcpy(out + off, "STRX", 4);              off += 4;
    memcpy(out + off, &id,   sizeof(id));      off += sizeof(id);
    memcpy(out + off, &type, sizeof(type));    off += sizeof(type);
    memcpy(out + off, &len,  sizeof(len));     off += sizeof(len);

    uint8_t checksum = 0;
    for (uint16_t i = 0; i < len; i++) {
        out[off++] = (uint8_t)(payload[i] ^ 0x55);
        checksum  ^= payload[i];
    }
    out[off++] = (uint8_t)(checksum ^ 0x55);
    return off;
}

static int parser_decodes_single_frame(void) {
    parser_t  *parser = parser_create(8);
    ringbuf_t *rb     = ringbuf_create(256);
    TEST_ASSERT(parser != NULL && rb != NULL, "parser/ringbuf alloc");

    const uint8_t payload[] = "ping";
    uint16_t plen = (uint16_t)(sizeof(payload) - 1);
    uint8_t wire[64];
    size_t wlen = encode_frame(wire, 0x12345678u, 0x0001u, payload, plen);

    uint32_t wrote = ringbuf_write(rb, wire, (uint32_t)wlen);
    TEST_ASSERT_EQ_INT(wrote, (uint32_t)wlen, "ringbuf accepted all wire bytes");

    packet_t *pkt = NULL;
    streamx_status_t s = parser_parse_stream(parser, rb, &pkt);
    TEST_ASSERT_EQ_INT(s, STREAMX_OK, "parse should succeed");
    TEST_ASSERT(pkt != NULL, "packet must be returned");
    TEST_ASSERT_EQ_INT(pkt->id, 0x12345678u, "id mismatch");
    TEST_ASSERT_EQ_INT(pkt->type, 0x0001u, "type mismatch");
    TEST_ASSERT_EQ_INT(pkt->length, plen, "length mismatch");
    TEST_ASSERT(memcmp(pkt->payload, payload, plen) == 0, "payload mismatch");

    packet_free(pkt);
    parser_free(parser);
    ringbuf_free(rb);
    return 0;
}

static int parser_returns_eof_when_starved(void) {
    parser_t  *parser = parser_create(8);
    ringbuf_t *rb     = ringbuf_create(64);
    TEST_ASSERT(parser != NULL && rb != NULL, "parser/ringbuf alloc");

    /* Empty ring buffer: parser must report EOF, not OK or generic error. */
    packet_t *pkt = NULL;
    streamx_status_t s = parser_parse_stream(parser, rb, &pkt);
    TEST_ASSERT_EQ_INT(s, STREAMX_ERR_EOF, "empty stream is EOF");
    TEST_ASSERT(pkt == NULL, "no packet should be produced");

    parser_free(parser);
    ringbuf_free(rb);
    return 0;
}

static int parser_invalid_args(void) {
    packet_t *pkt = NULL;
    TEST_ASSERT_EQ_INT(parser_parse_stream(NULL, NULL, &pkt),
                       STREAMX_ERR_INVALID, "NULL parser+rb");
    return 0;
}

static int parser_tokenize_basic_metadata(void) {
    parser_t *parser = parser_create(8);
    TEST_ASSERT(parser != NULL, "parser_create");

    streamx_status_t s = parser_tokenize_metadata(parser, "k1=v1;k2=v2");
    TEST_ASSERT_EQ_INT(s, STREAMX_OK, "tokenize should succeed");
    TEST_ASSERT_EQ_INT(parser->token_count, 2, "should produce 2 tokens");
    TEST_ASSERT_EQ_STR(parser->tokens[0].key, "k1", "token 0 key");
    TEST_ASSERT_EQ_STR(parser->tokens[0].value, "v1", "token 0 value");
    TEST_ASSERT_EQ_STR(parser->tokens[1].key, "k2", "token 1 key");
    TEST_ASSERT_EQ_STR(parser->tokens[1].value, "v2", "token 1 value");

    parser_free(parser);
    return 0;
}

static int parser_tokenize_rejects_missing_eq(void) {
    parser_t *parser = parser_create(8);
    TEST_ASSERT(parser != NULL, "parser_create");

    streamx_status_t s = parser_tokenize_metadata(parser, "no_equals_here");
    TEST_ASSERT_EQ_INT(s, STREAMX_ERR_INVALID,
                       "missing '=' must be reported as invalid");

    parser_free(parser);
    return 0;
}

static int parser_tokenize_null_guard(void) {
    TEST_ASSERT_EQ_INT(parser_tokenize_metadata(NULL, "k=v"),
                       STREAMX_ERR_INVALID, "NULL parser");
    return 0;
}

static int parser_tokenize_large_metadata_value(void) {
    parser_t *parser = parser_create(8);
    TEST_ASSERT(parser != NULL, "parser_create");

    const char *meta =
        "name=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    streamx_status_t s =
        parser_tokenize_metadata(parser, meta);

    TEST_ASSERT_EQ_INT(s, STREAMX_OK,
                       "large metadata value should not overflow");

    parser_free(parser);
    return 0;
}

int test_parser_run(void) {
    int failures = 0;
    printf("[parser]\n");
    TEST_RUN(parser_decodes_single_frame);
    TEST_RUN(parser_returns_eof_when_starved);
    TEST_RUN(parser_invalid_args);
    TEST_RUN(parser_tokenize_basic_metadata);
    TEST_RUN(parser_tokenize_rejects_missing_eq);
    TEST_RUN(parser_tokenize_null_guard);
    TEST_RUN(parser_tokenize_large_metadata_value);
    return failures;
}
