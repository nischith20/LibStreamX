/*
 * logger.c — append-mode logger to file or stdout.
 * SPDX-License-Identifier: MIT
 */

#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static FILE *log_fp = NULL;

streamx_status_t logger_init(const char *log_file) {
    if (log_fp != NULL) {
        return STREAMX_OK;
    }

    if (log_file != NULL) {
        log_fp = fopen(log_file, "a");
        if (log_fp == NULL) {
            return STREAMX_ERR_IO;
        }

        const char *init_hdr = "[LOG_START]\n";
        size_t written = fwrite(init_hdr, 1, strlen(init_hdr), log_fp);
        if (written < strlen(init_hdr)) {
            return STREAMX_ERR_IO;
        }
        fflush(log_fp);
    }
    return STREAMX_OK;
}

void logger_log(int level, const char *fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    const char *lvl_str = "INFO";
    switch (level) {
        case LOG_LEVEL_DEBUG: lvl_str = "DEBUG"; break;
        case LOG_LEVEL_INFO:  lvl_str = "INFO";  break;
        case LOG_LEVEL_WARN:  lvl_str = "WARN";  break;
        case LOG_LEVEL_ERROR: lvl_str = "ERROR"; break;
        case LOG_LEVEL_FATAL: lvl_str = "FATAL"; break;
    }

    if (log_fp != NULL) {
        fprintf(log_fp, "[%s] %s\n", lvl_str, buf);
        fflush(log_fp);
    } else {
        printf("[%s] %s\n", lvl_str, buf);
    }
}

void logger_close(void) {
    if (log_fp != NULL) {
        fclose(log_fp);
    }
}
