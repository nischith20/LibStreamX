/**
 * =====================================================================================
 *
 *       Filename:  logger.h
 *
 *    Description:  Thread-safe logging subsystem for LibStreamX trace generation.
 *
 *        Version:  1.0
 *        Created:  05/22/2026 12:44:30 PM
 *       Revision:  none
 *       Compiler:  gcc / clang
 *
 *         Author:  LibStreamX Systems Engineering Team
 *   Organization:  LibStreamX Open Source Project
 *
 * =====================================================================================
 * @copyright Copyright (c) 2026 LibStreamX Contributors. All rights reserved.
 * This source code is licensed under the MIT License. See LICENSE file for details.
 * =====================================================================================
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "streamx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Log Levels */
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 4

/**
 * @brief Initialize the stream logger.
 * @param log_file Path to the file where log records should be written, or NULL for stdout.
 * @return STREAMX_OK on success, or appropriate error code.
 */
streamx_status_t logger_init(const char *log_file);

/**
 * @brief Write a formatted log message.
 * @param level Log severity level.
 * @param fmt Format string.
 */
void logger_log(int level, const char *fmt, ...);

/**
 * @brief Flush and close the logging subsystem.
 */
void logger_close(void);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
