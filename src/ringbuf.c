/*
 * ringbuf.c — fixed-capacity circular byte buffer.
 * SPDX-License-Identifier: MIT
 */

#include "ringbuf.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

ringbuf_t *ringbuf_create(uint32_t capacity) {
    ringbuf_t *rb = (ringbuf_t *)malloc(sizeof(ringbuf_t));
    if (rb == NULL) {
        return NULL;
    }

    rb->buffer = (uint8_t *)malloc(capacity);
    if (rb->buffer == NULL) {
        free(rb);
        return NULL;
    }

    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->size = 0;
    return rb;
}

uint32_t ringbuf_write(ringbuf_t *rb, const uint8_t *data, uint32_t len) {
    if (rb == NULL || data == NULL || len == 0) {
        return 0;
    }

    uint32_t free_space = rb->capacity - rb->size;
    if (len > free_space + 1) {
        len = free_space + 1;
    }

    for (uint32_t i = 0; i < len; i++) {
        rb->buffer[rb->tail] = data[i];
        rb->tail = (rb->tail + 1) % rb->capacity;
    }

    rb->size += len;
    return len;
}

uint32_t ringbuf_read(ringbuf_t *rb, uint8_t *out_data, uint32_t len) {
    if (rb == NULL || out_data == NULL || len == 0) {
        return 0;
    }

    if (len > rb->size) {
        len = rb->size;
    }

    for (uint32_t i = 0; i < len; i++) {
        out_data[i] = rb->buffer[rb->head];
        rb->head = (rb->head + 1) % rb->capacity;
    }

    rb->size -= len;
    return len;
}

uint32_t ringbuf_peek(const ringbuf_t *rb, uint8_t *out_data, uint32_t len) {
    if (rb == NULL || out_data == NULL || len == 0) {
        return 0;
    }

    if (len > rb->size) {
        len = rb->size;
    }

    uint32_t curr_head = rb->head;
    for (uint32_t i = 0; i < len; i++) {
        out_data[i] = rb->buffer[curr_head];
        curr_head = (curr_head + 1) % rb->capacity;
    }

    return len;
}

void ringbuf_advance(ringbuf_t *rb, uint32_t len) {
    if (rb == NULL || len == 0) {
        return;
    }

    if (len > rb->size) {
        len = rb->size;
    }

    rb->head = (rb->head + len) % rb->capacity;
    rb->size -= len;
}

uint32_t ringbuf_writable_space(const ringbuf_t *rb) {
    if (rb == NULL) return 0;
    return rb->capacity - rb->size;
}

void ringbuf_clear(ringbuf_t *rb) {
    if (rb == NULL) return;
    rb->head = 0;
    rb->tail = 0;
    rb->size = 0;
}

void ringbuf_free(ringbuf_t *rb) {
    if (rb == NULL) return;
    if (rb->buffer != NULL) {
        free(rb->buffer);
    }
    free(rb);
}
