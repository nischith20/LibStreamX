/*
 * test_main.c - aggregator for the per-module test_*.c suites.
 *
 * Each per-module file exposes a single `int test_<module>_run(void)`
 * entry point that returns the number of failures. This driver runs
 * every module in turn and exits non-zero if any of them failed.
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

int test_ringbuf_run(void);
int test_packet_run(void);
int test_arena_run(void);
int test_config_run(void);
int test_parser_run(void);
int test_logger_run(void);

int main(void) {
    int failed = 0;

    printf("=== LibStreamX test runner ===\n");

    failed += test_ringbuf_run();
    failed += test_packet_run();
    failed += test_arena_run();
    failed += test_config_run();
    failed += test_parser_run();
    /* logger last: it closes the global FILE* and the API does not reopen. */
    failed += test_logger_run();

    printf("\n");
    if (failed == 0) {
        printf("[PASS] all tests succeeded\n");
        return 0;
    }
    printf("[FAIL] %d test(s) failed\n", failed);
    return 1;
}
