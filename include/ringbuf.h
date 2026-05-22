/**
 * =====================================================================================
 *
 *       Filename:  ringbuf.h
 *
 *    Description:  Thread-compatible lock-free circular ring buffer for incoming byte streams.
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

#ifndef RINGBUF_H
#define RINGBUF_H

#include "streamx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *buffer;      /**< Dynamically allocated underlying data array */
    uint32_t capacity;    /**< Maximum bytes the buffer can hold */
    uint32_t head;        /**< Read cursor index */
    uint32_t tail;        /**< Write cursor index */
    uint32_t size;        /**< Actual byte count currently stored */
} ringbuf_t;

/**
 * @brief Initialize a new circular ring buffer.
 * @param capacity Maximum byte storage capacity.
 * @return Struct pointer, or NULL on memory exhaustion.
 */
ringbuf_t *ringbuf_create(uint32_t capacity);

/**
 * @brief Write bytes into the ring buffer.
 * @param rb Target ring buffer reference.
 * @param data Array of source bytes to read from.
 * @param len Quantity of bytes to write.
 * @return Count of bytes successfully written.
 */
uint32_t ringbuf_write(ringbuf_t *rb, const uint8_t *data, uint32_t len);

/**
 * @brief Read bytes out from the ring buffer.
 * @param rb Target ring buffer reference.
 * @param out_data Destination buffer to copy read data to.
 * @param len Requested quantity of bytes to read.
 * @return Count of bytes successfully read.
 */
uint32_t ringbuf_read(ringbuf_t *rb, uint8_t *out_data, uint32_t len);

/**
 * @brief Peak bytes out of the ring buffer without moving the read head cursor.
 * @param rb Target ring buffer reference.
 * @param out_data Destination buffer to copy peaked data to.
 * @param len Requested quantity of bytes to peek.
 * @return Count of bytes successfully peeked.
 */
uint32_t ringbuf_peek(const ringbuf_t *rb, uint8_t *out_data, uint32_t len);

/**
 * @brief Consume and discard bytes from the head of the buffer.
 * @param rb Target ring buffer.
 * @param len Number of bytes to discard.
 */
void ringbuf_advance(ringbuf_t *rb, uint32_t len);

/**
 * @brief Get the remaining empty write space in the ring buffer.
 */
uint32_t ringbuf_writable_space(const ringbuf_t *rb);

/**
 * @brief Clear and reset the cursors of the circular buffer.
 */
void ringbuf_clear(ringbuf_t *rb);

/**
 * @brief Release allocated ring buffer memory.
 */
void ringbuf_free(ringbuf_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* RINGBUF_H */
