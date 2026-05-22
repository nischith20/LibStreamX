/**
 * =====================================================================================
 *
 *       Filename:  packet.h
 *
 *    Description:  High-performance binary packet wrapper definitions.
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

#ifndef PACKET_H
#define PACKET_H

#include "streamx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Representation of an structured stream packet frame.
 */
typedef struct {
    uint32_t id;          /**< Globally unique packet transaction index */
    uint16_t type;        /**< Protocol specific category identifier */
    uint32_t length;      /**< Length of the payload buffer in bytes */
    uint8_t *payload;     /**< Dynamically allocated raw payload contents */
    char *tag;            /**< Descriptive tag string allocated on the heap */
} packet_t;

/**
 * @brief Allocate and populate a new packet instance.
 * @param id Transaction identifier.
 * @param type Custom packet categorization code.
 * @param payload Raw payload block to copy, or NULL to allocate empty.
 * @param length Extent of payload data.
 * @param tag Custom description tag to assign.
 * @return Allocated packet pointer, or NULL on memory exhaustion.
 */
packet_t *packet_create(uint32_t id, uint16_t type, const uint8_t *payload, uint32_t length, const char *tag);

/**
 * @brief Perform a copy/clone operation on a source packet structure.
 * @param src Reference packet structure.
 * @return Deep/shallow-copied instance pointer, or NULL on error.
 */
packet_t *packet_clone(const packet_t *src);

/**
 * @brief Safely release packet payload allocations and memory structures.
 * @param pkt Packet instance to free.
 */
void packet_free(packet_t *pkt);

#ifdef __cplusplus
}
#endif

#endif /* PACKET_H */
