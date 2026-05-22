/*
 * test_ringbuf.c - unit tests for the ringbuf module.
 * SPDX-License-Identifier: MIT
 */

#include "ringbuf.h"
#include "test_helpers.h"

#include <stdio.h>
#include <string.h>

static int rb_create_destroy(void) {
    ringbuf_t *rb = ringbuf_create(64);
    TEST_ASSERT(rb != NULL, "ringbuf_create returned NULL");
    TEST_ASSERT_EQ_INT(rb->capacity, 64, "capacity should match request");
    TEST_ASSERT_EQ_INT(rb->size, 0, "fresh buffer should be empty");
    ringbuf_free(rb);
    return 0;
}

static int rb_write_read_roundtrip(void) {
    ringbuf_t *rb = ringbuf_create(32);
    TEST_ASSERT(rb != NULL, "ringbuf_create returned NULL");

    const uint8_t in[] = {1, 2, 3, 4, 5};
    uint32_t wrote = ringbuf_write(rb, in, sizeof(in));
    TEST_ASSERT_EQ_INT(wrote, sizeof(in), "wrote count mismatch");
    TEST_ASSERT_EQ_INT(rb->size, sizeof(in), "size after write");

    uint8_t out[8] = {0};
    uint32_t read = ringbuf_read(rb, out, sizeof(in));
    TEST_ASSERT_EQ_INT(read, sizeof(in), "read count mismatch");
    TEST_ASSERT(memcmp(in, out, sizeof(in)) == 0, "round-trip data mismatch");
    TEST_ASSERT_EQ_INT(rb->size, 0, "size should be 0 after full read");

    ringbuf_free(rb);
    return 0;
}

static int rb_peek_does_not_consume(void) {
    ringbuf_t *rb = ringbuf_create(16);
    TEST_ASSERT(rb != NULL, "ringbuf_create returned NULL");

    const uint8_t in[] = {10, 20, 30};
    ringbuf_write(rb, in, sizeof(in));

    uint8_t peek_buf[3] = {0};
    uint32_t peeked = ringbuf_peek(rb, peek_buf, sizeof(peek_buf));
    TEST_ASSERT_EQ_INT(peeked, 3, "peek should return 3");
    TEST_ASSERT(memcmp(peek_buf, in, sizeof(in)) == 0, "peeked content mismatch");
    TEST_ASSERT_EQ_INT(rb->size, 3, "peek must not consume");

    ringbuf_free(rb);
    return 0;
}

static int rb_advance_discards(void) {
    ringbuf_t *rb = ringbuf_create(16);
    TEST_ASSERT(rb != NULL, "ringbuf_create returned NULL");

    const uint8_t in[] = {1, 2, 3, 4, 5};
    ringbuf_write(rb, in, sizeof(in));
    ringbuf_advance(rb, 2);
    TEST_ASSERT_EQ_INT(rb->size, 3, "advance(2) should leave 3");

    uint8_t out[3] = {0};
    ringbuf_read(rb, out, sizeof(out));
    TEST_ASSERT(out[0] == 3 && out[1] == 4 && out[2] == 5,
                "advance should drop the head bytes");

    ringbuf_free(rb);
    return 0;
}

static int rb_wraparound(void) {
    ringbuf_t *rb = ringbuf_create(8);
    TEST_ASSERT(rb != NULL, "ringbuf_create returned NULL");

    const uint8_t first[]  = {1, 2, 3, 4};
    const uint8_t second[] = {5, 6, 7, 8};

    ringbuf_write(rb, first, sizeof(first));
    uint8_t tmp[4];
    ringbuf_read(rb, tmp, 4);
    ringbuf_write(rb, second, sizeof(second));

    uint8_t out[4] = {0};
    ringbuf_read(rb, out, sizeof(out));
    TEST_ASSERT(memcmp(out, second, sizeof(second)) == 0,
                "wraparound read mismatch");

    ringbuf_free(rb);
    return 0;
}

static int rb_writable_space_and_clear(void) {
    ringbuf_t *rb = ringbuf_create(10);
    TEST_ASSERT(rb != NULL, "ringbuf_create returned NULL");
    TEST_ASSERT_EQ_INT(ringbuf_writable_space(rb), 10, "empty writable space");

    const uint8_t in[] = {1, 2, 3};
    ringbuf_write(rb, in, sizeof(in));
    TEST_ASSERT_EQ_INT(ringbuf_writable_space(rb), 7, "writable after write");

    ringbuf_clear(rb);
    TEST_ASSERT_EQ_INT(rb->size, 0, "size 0 after clear");
    TEST_ASSERT_EQ_INT(ringbuf_writable_space(rb), 10, "writable after clear");

    ringbuf_free(rb);
    return 0;
}

static int rb_null_guards(void) {
    /* The public API should silently no-op on NULL input. */
    uint8_t buf[4];
    TEST_ASSERT_EQ_INT(ringbuf_write(NULL, buf, 4), 0, "write NULL rb");
    TEST_ASSERT_EQ_INT(ringbuf_read(NULL, buf, 4), 0, "read NULL rb");
    TEST_ASSERT_EQ_INT(ringbuf_peek(NULL, buf, 4), 0, "peek NULL rb");
    TEST_ASSERT_EQ_INT(ringbuf_writable_space(NULL), 0, "writable_space NULL");
    ringbuf_advance(NULL, 4);
    ringbuf_clear(NULL);
    ringbuf_free(NULL);
    return 0;
}

int test_ringbuf_run(void) {
    int failures = 0;
    printf("[ringbuf]\n");
    TEST_RUN(rb_create_destroy);
    TEST_RUN(rb_write_read_roundtrip);
    TEST_RUN(rb_peek_does_not_consume);
    TEST_RUN(rb_advance_discards);
    TEST_RUN(rb_wraparound);
    TEST_RUN(rb_writable_space_and_clear);
    TEST_RUN(rb_null_guards);
    return failures;
}
