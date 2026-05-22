/*
 * test_arena.c - unit tests for the arena bump allocator.
 * SPDX-License-Identifier: MIT
 */

#include "arena.h"
#include "test_helpers.h"

#include <stdio.h>
#include <string.h>

static int arena_basic_alloc(void) {
    arena_t *a = arena_create(128);
    TEST_ASSERT(a != NULL, "arena_create returned NULL");
    TEST_ASSERT_EQ_INT(a->capacity, 128, "capacity mismatch");
    TEST_ASSERT_EQ_INT(a->offset, 0, "fresh arena offset");

    void *p = arena_alloc(a, 16);
    TEST_ASSERT(p != NULL, "alloc 16 should succeed");
    TEST_ASSERT(a->offset >= 16, "offset should advance");

    arena_free(a);
    return 0;
}

static int arena_strdup_works(void) {
    arena_t *a = arena_create(64);
    TEST_ASSERT(a != NULL, "arena_create returned NULL");

    char *s = arena_strdup(a, "stream-x");
    TEST_ASSERT(s != NULL, "strdup result NULL");
    TEST_ASSERT_EQ_STR(s, "stream-x", "strdup content mismatch");

    arena_free(a);
    return 0;
}

static int arena_reset_recycles(void) {
    arena_t *a = arena_create(64);
    TEST_ASSERT(a != NULL, "arena_create returned NULL");

    void *first = arena_alloc(a, 32);
    TEST_ASSERT(first != NULL, "first alloc");
    uint32_t hw = a->high_water;
    TEST_ASSERT(hw >= 32, "high water should be set");

    arena_reset(a);
    TEST_ASSERT_EQ_INT(a->offset, 0, "reset clears offset");
    TEST_ASSERT_EQ_INT(a->high_water, hw, "reset preserves high water");

    void *second = arena_alloc(a, 8);
    TEST_ASSERT(second != NULL, "alloc after reset");
    TEST_ASSERT(second == first, "reset should hand back same base addr");

    arena_free(a);
    return 0;
}

static int arena_exhaustion_returns_null(void) {
    arena_t *a = arena_create(32);
    TEST_ASSERT(a != NULL, "arena_create returned NULL");

    void *ok = arena_alloc(a, 16);
    TEST_ASSERT(ok != NULL, "first alloc within capacity");

    void *fail = arena_alloc(a, 9999);
    TEST_ASSERT(fail == NULL, "oversized alloc should fail");

    arena_free(a);
    return 0;
}

static int arena_zero_capacity_rejected(void) {
    arena_t *a = arena_create(0);
    TEST_ASSERT(a == NULL, "zero capacity must be rejected");
    return 0;
}

static int arena_null_safe(void) {
    arena_free(NULL);
    arena_reset(NULL);
    TEST_ASSERT(arena_alloc(NULL, 8) == NULL, "alloc NULL arena");
    TEST_ASSERT(arena_strdup(NULL, "x") == NULL, "strdup NULL arena");
    return 0;
}

int test_arena_run(void) {
    int failures = 0;
    printf("[arena]\n");
    TEST_RUN(arena_basic_alloc);
    TEST_RUN(arena_strdup_works);
    TEST_RUN(arena_reset_recycles);
    TEST_RUN(arena_exhaustion_returns_null);
    TEST_RUN(arena_zero_capacity_rejected);
    TEST_RUN(arena_null_safe);
    return failures;
}
