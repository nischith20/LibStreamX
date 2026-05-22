/**
 * =====================================================================================
 *
 *       Filename:  streamx.h
 *
 *    Description:  Core definitions, types, and error codes for LibStreamX.
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

#ifndef STREAMX_H
#define STREAMX_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* API Version Information */
#define STREAMX_VERSION_MAJOR 0
#define STREAMX_VERSION_MINOR 1
#define STREAMX_VERSION_PATCH 0
#define STREAMX_VERSION       "0.1.0"

/* Status and Error Codes */
typedef enum {
    STREAMX_OK              =  0,  /**< Operation completed successfully */
    STREAMX_ERR_GENERIC     = -1,  /**< Unspecified internal error */
    STREAMX_ERR_NOMEM       = -2,  /**< Memory allocation failed */
    STREAMX_ERR_INVALID     = -3,  /**< Invalid argument or state */
    STREAMX_ERR_IO          = -4,  /**< Input/Output system error */
    STREAMX_ERR_OVERFLOW    = -5,  /**< Buffer or integer overflow detected */
    STREAMX_ERR_CHECKSUM    = -6,  /**< Packet payload checksum mismatch */
    STREAMX_ERR_EOF         = -7   /**< End of stream or file reached */
} streamx_status_t;

/**
 * @brief LibStreamX Runtime Configuration.
 */
typedef struct {
    uint32_t max_packet_size;     /**< Absolute upper bound for packet sizes */
    uint16_t max_tokens;          /**< Maximum token metadata capacity per stream */
    int enable_shadow_mode;       /**< Strict validation mode active flag */
    const char *log_file_path;    /**< Location of persistent trace output */
} streamx_config_t;

/**
 * @brief Initialize the global LibStreamX engine state.
 * @param config Pointer to initialization parameters, or NULL for defaults.
 * @return STREAMX_OK on success, or appropriate error code.
 */
streamx_status_t streamx_init(const streamx_config_t *config);

/**
 * @brief Shutdown the global engine and release resources.
 */
void streamx_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* STREAMX_H */
