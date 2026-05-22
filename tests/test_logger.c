/*
 * test_logger.c - unit tests for the logger.
 *
 * The logger writes to a file when initialised with a path, otherwise to
 * stdout. Tests use a temporary file so the runner output is not polluted.
 *
 * SPDX-License-Identifier: MIT
 */

#include "logger.h"
#include "test_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TMP_LOG_PATH = "test_logger_tmp.log";

static int read_all(const char *path, char *out, size_t cap) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        return -1;
    }
    size_t n = fread(out, 1, cap - 1, fp);
    out[n] = '\0';
    fclose(fp);
    return (int)n;
}

static int log_writes_to_file(void) {
    remove(TMP_LOG_PATH);

    streamx_status_t s = logger_init(TMP_LOG_PATH);
    TEST_ASSERT_EQ_INT(s, STREAMX_OK, "logger_init should return OK");

    logger_log(LOG_LEVEL_INFO, "value=%d tag=%s", 42, "hello");
    logger_log(LOG_LEVEL_WARN, "warning emitted");
    logger_close();

    char buf[1024] = {0};
    int n = read_all(TMP_LOG_PATH, buf, sizeof(buf));
    TEST_ASSERT(n > 0, "log file must contain bytes");
    TEST_ASSERT(strstr(buf, "[INFO]") != NULL,  "info line missing");
    TEST_ASSERT(strstr(buf, "[WARN]") != NULL,  "warn line missing");
    TEST_ASSERT(strstr(buf, "value=42") != NULL, "formatted value missing");
    TEST_ASSERT(strstr(buf, "hello") != NULL, "formatted string missing");

    remove(TMP_LOG_PATH);
    return 0;
}

static int log_close_null_safe(void) {
    /* Calling close without a prior init must not crash. */
    logger_close();
    return 0;
}

int test_logger_run(void) {
    int failures = 0;
    printf("[logger]\n");
    TEST_RUN(log_writes_to_file);
    TEST_RUN(log_close_null_safe);
    return failures;
}
