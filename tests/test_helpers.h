/*
 * test_helpers.h - tiny assert macros used by the per-module test files.
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBSTREAMX_TEST_HELPERS_H
#define LIBSTREAMX_TEST_HELPERS_H

#include <stdio.h>
#include <string.h>

#define TEST_ASSERT(cond, msg)                                           \
    do {                                                                 \
        if (!(cond)) {                                                   \
            fprintf(stderr,                                              \
                    "    FAIL: %s (%s:%d)\n", (msg), __FILE__, __LINE__);\
            return 1;                                                    \
        }                                                                \
    } while (0)

#define TEST_ASSERT_EQ_INT(actual, expected, msg)                        \
    do {                                                                 \
        long long _a = (long long)(actual);                              \
        long long _e = (long long)(expected);                            \
        if (_a != _e) {                                                  \
            fprintf(stderr,                                              \
                    "    FAIL: %s (got %lld, want %lld) (%s:%d)\n",      \
                    (msg), _a, _e, __FILE__, __LINE__);                  \
            return 1;                                                    \
        }                                                                \
    } while (0)

#define TEST_ASSERT_EQ_STR(actual, expected, msg)                        \
    do {                                                                 \
        const char *_a = (actual);                                       \
        const char *_e = (expected);                                     \
        if (_a == NULL || _e == NULL || strcmp(_a, _e) != 0) {           \
            fprintf(stderr,                                              \
                    "    FAIL: %s (got \"%s\", want \"%s\") (%s:%d)\n",  \
                    (msg),                                               \
                    _a ? _a : "(null)",                                  \
                    _e ? _e : "(null)",                                  \
                    __FILE__, __LINE__);                                 \
            return 1;                                                    \
        }                                                                \
    } while (0)

#define TEST_RUN(fn)                                                     \
    do {                                                                 \
        printf("  - %-30s ", #fn);                                       \
        fflush(stdout);                                                  \
        int _rc = fn();                                                  \
        if (_rc == 0) {                                                  \
            printf("ok\n");                                              \
        } else {                                                         \
            printf("FAIL\n");                                            \
            failures++;                                                  \
        }                                                                \
    } while (0)

#endif /* LIBSTREAMX_TEST_HELPERS_H */
