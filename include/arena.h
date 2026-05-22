/*
 * arena.h — bump-allocator memory arena.
 *
 * The parser uses an arena for short-lived per-frame allocations
 * (metadata token keys/values, scratch buffers). The lifetime of every
 * arena allocation is bounded by the next call to arena_reset() or
 * arena_free().
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ARENA_H
#define ARENA_H

#include "streamx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t  *base;       /* backing buffer */
    uint32_t  capacity;   /* total bytes available */
    uint32_t  offset;     /* next free byte */
    uint32_t  high_water; /* largest offset ever reached */
} arena_t;

/* Allocate a new arena with the given backing-buffer capacity. */
arena_t *arena_create(uint32_t capacity);

/* Reserve `size` bytes from the arena. Returns NULL on exhaustion.
 * The returned pointer is valid until arena_reset() or arena_free(). */
void *arena_alloc(arena_t *a, uint32_t size);

/* Duplicate a NUL-terminated string into the arena. */
char *arena_strdup(arena_t *a, const char *str);

/* Reset the bump pointer; all previously returned pointers become
 * invalid. The backing buffer is retained. */
void arena_reset(arena_t *a);

/* Release the arena and its backing buffer. */
void arena_free(arena_t *a);

#ifdef __cplusplus
}
#endif

#endif /* ARENA_H */
