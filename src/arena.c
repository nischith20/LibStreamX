/*
 * arena.c — bump-allocator memory arena.
 * SPDX-License-Identifier: MIT
 */

#include "arena.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

#define ARENA_ALIGN 8

static uint32_t align_up(uint32_t v, uint32_t a) {
    return (v + (a - 1)) & ~(a - 1);
}

arena_t *arena_create(uint32_t capacity) {
    if (capacity == 0) {
        return NULL;
    }
    arena_t *a = (arena_t *)malloc(sizeof(arena_t));
    if (a == NULL) {
        return NULL;
    }
    a->base = (uint8_t *)malloc(capacity);
    if (a->base == NULL) {
        free(a);
        return NULL;
    }
    a->capacity = capacity;
    a->offset = 0;
    a->high_water = 0;
    return a;
}

void *arena_alloc(arena_t *a, uint32_t size) {
    if (a == NULL || size == 0) {
        return NULL;
    }

    uint32_t aligned = align_up(size, ARENA_ALIGN);
    uint32_t new_offset = a->offset + aligned;

    if (new_offset > a->capacity) {
        logger_log(LOG_LEVEL_WARN, "arena exhausted: requested %u, have %u",
                   aligned, a->capacity - a->offset);
        return NULL;
    }

    void *ptr = a->base + a->offset;
    a->offset = new_offset;
    if (a->offset > a->high_water) {
        a->high_water = a->offset;
    }
    return ptr;
}

char *arena_strdup(arena_t *a, const char *str) {
    if (a == NULL || str == NULL) {
        return NULL;
    }
    uint32_t len = (uint32_t)strlen(str);
    char *out = (char *)arena_alloc(a, len + 1);
    if (out == NULL) {
        return NULL;
    }
    memcpy(out, str, len + 1);
    return out;
}

void arena_reset(arena_t *a) {
    if (a == NULL) {
        return;
    }
    a->offset = 0;
}

void arena_free(arena_t *a) {
    if (a == NULL) {
        return;
    }
    free(a->base);
    free(a);
}
