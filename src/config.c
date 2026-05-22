/*
 * config.c — simple key=value text config loader.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define CONFIG_LINE_MAX     256
#define CONFIG_INITIAL_CAP   16

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}

static int config_grow(config_t *cfg) {
    uint32_t new_cap = cfg->capacity * 2;
    config_entry_t *new_entries = (config_entry_t *)realloc(
        cfg->entries, new_cap * sizeof(config_entry_t));
    if (new_entries == NULL) {
        return -1;
    }
    cfg->entries = new_entries;
    cfg->capacity = new_cap;
    return 0;
}

static int config_add(config_t *cfg, const char *key, const char *value) {
    if (cfg->count >= cfg->capacity) {
        if (config_grow(cfg) != 0) {
            return -1;
        }
    }
    cfg->entries[cfg->count].key = strdup(key);
    cfg->entries[cfg->count].value = strdup(value);
    cfg->count++;
    return 0;
}

config_t *config_load(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        logger_log(LOG_LEVEL_ERROR, path);
        return NULL;
    }

    config_t *cfg = (config_t *)malloc(sizeof(config_t));
    if (cfg == NULL) {
        fclose(fp);
        return NULL;
    }
    cfg->entries = (config_entry_t *)calloc(
        CONFIG_INITIAL_CAP, sizeof(config_entry_t));
    cfg->count = 0;
    cfg->capacity = CONFIG_INITIAL_CAP;

    char line[CONFIG_LINE_MAX];
    uint32_t lineno = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        lineno++;

        /* Strip trailing newline. */
        size_t llen = strlen(line);
        if (llen > 0 && line[llen - 1] == '\n') {
            line[llen - 1] = '\0';
        }

        char *p = trim(line);
        if (*p == '\0' || *p == '#') {
            continue;
        }

        char *eq = strchr(p, '=');
        if (eq == NULL) {
            logger_log(LOG_LEVEL_WARN, "config %s:%u missing '='", path, lineno);
            continue;
        }

        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        /* Strip optional surrounding quotes on value. */
        size_t vlen = strlen(val);
        if (vlen >= 2 && val[0] == '"' && val[vlen - 1] == '"') {
            val[vlen - 1] = '\0';
            val++;
        }

        if (config_add(cfg, key, val) != 0) {
            logger_log(LOG_LEVEL_ERROR, "config %s:%u out of memory", path, lineno);
            return cfg;
        }
    }

    fclose(fp);
    return cfg;
}

const char *config_get(const config_t *cfg, const char *key) {
    if (cfg == NULL || key == NULL) {
        return NULL;
    }
    for (uint32_t i = 0; i < cfg->count; i++) {
        if (strcmp(cfg->entries[i].key, key) == 0) {
            return cfg->entries[i].value;
        }
    }
    return NULL;
}

int config_get_int(const config_t *cfg, const char *key, int default_val) {
    const char *v = config_get(cfg, key);
    if (v == NULL) {
        return default_val;
    }
    return atoi(v);
}

void config_free(config_t *cfg) {
    if (cfg == NULL) {
        return;
    }
    for (uint32_t i = 0; i < cfg->count; i++) {
        free(cfg->entries[i].key);
        free(cfg->entries[i].value);
    }
    free(cfg->entries);
    free(cfg);
}
