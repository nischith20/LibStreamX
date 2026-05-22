/*
 * config.h — simple key=value text config loader.
 *
 * The config file format is one entry per line:
 *     # comment lines are ignored
 *     key = value
 *
 * Whitespace around `=` is tolerated. Keys may be alphanumerics +
 * underscore. Values run to end of line.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "streamx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *key;
    char *value;
} config_entry_t;

typedef struct {
    config_entry_t *entries;
    uint32_t        count;
    uint32_t        capacity;
} config_t;

/* Load config from a file. Returns a heap-allocated config_t* on
 * success, NULL on I/O error or malformed input. */
config_t *config_load(const char *path);

/* Look up a value. Returns NULL if the key was not present. */
const char *config_get(const config_t *cfg, const char *key);

/* Look up an integer value. Returns `default_val` if the key was not
 * present or did not parse cleanly. */
int config_get_int(const config_t *cfg, const char *key, int default_val);

/* Release a config and all owned strings. */
void config_free(config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_H */
