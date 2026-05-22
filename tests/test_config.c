/*
 * test_config.c - unit tests for the config loader.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "test_helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TMP_CONFIG_PATH = "test_config_tmp.cfg";

static int write_file(const char *path, const char *contents) {
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }
    if (fwrite(contents, 1, strlen(contents), fp) != strlen(contents)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

static int cfg_load_basic_kv(void) {
    const char *body =
        "# comment line\n"
        "name = libstreamx\n"
        "max_tokens = 32\n"
        "log_level = DEBUG\n";
    TEST_ASSERT(write_file(TMP_CONFIG_PATH, body) == 0, "write tmp config");

    config_t *cfg = config_load(TMP_CONFIG_PATH);
    TEST_ASSERT(cfg != NULL, "config_load returned NULL");
    TEST_ASSERT_EQ_STR(config_get(cfg, "name"), "libstreamx", "name lookup");
    TEST_ASSERT_EQ_STR(config_get(cfg, "log_level"), "DEBUG", "log_level lookup");
    TEST_ASSERT_EQ_INT(config_get_int(cfg, "max_tokens", -1), 32, "int lookup");

    config_free(cfg);
    remove(TMP_CONFIG_PATH);
    return 0;
}

static int cfg_get_int_default(void) {
    const char *body = "present = 7\n";
    TEST_ASSERT(write_file(TMP_CONFIG_PATH, body) == 0, "write tmp config");

    config_t *cfg = config_load(TMP_CONFIG_PATH);
    TEST_ASSERT(cfg != NULL, "config_load returned NULL");
    TEST_ASSERT_EQ_INT(config_get_int(cfg, "present", 99), 7, "existing key");
    TEST_ASSERT_EQ_INT(config_get_int(cfg, "missing", 99), 99, "default fallback");

    config_free(cfg);
    remove(TMP_CONFIG_PATH);
    return 0;
}

static int cfg_quoted_values(void) {
    const char *body = "greeting = \"hello world\"\n";
    TEST_ASSERT(write_file(TMP_CONFIG_PATH, body) == 0, "write tmp config");

    config_t *cfg = config_load(TMP_CONFIG_PATH);
    TEST_ASSERT(cfg != NULL, "config_load returned NULL");
    TEST_ASSERT_EQ_STR(config_get(cfg, "greeting"), "hello world",
                       "quotes should be stripped");

    config_free(cfg);
    remove(TMP_CONFIG_PATH);
    return 0;
}

static int cfg_missing_file(void) {
    config_t *cfg = config_load("does-not-exist.cfg");
    TEST_ASSERT(cfg == NULL, "missing file must return NULL");
    return 0;
}

static int cfg_null_guards(void) {
    TEST_ASSERT(config_get(NULL, "k") == NULL, "get NULL cfg");
    TEST_ASSERT_EQ_INT(config_get_int(NULL, "k", 5), 5, "get_int NULL cfg");
    config_free(NULL);
    return 0;
}

int test_config_run(void) {
    int failures = 0;
    printf("[config]\n");
    TEST_RUN(cfg_load_basic_kv);
    TEST_RUN(cfg_get_int_default);
    TEST_RUN(cfg_quoted_values);
    TEST_RUN(cfg_missing_file);
    TEST_RUN(cfg_null_guards);
    return failures;
}
