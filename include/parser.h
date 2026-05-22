/**
 * =====================================================================================
 *
 *       Filename:  parser.h
 *
 *    Description:  Binary frame parsing engine and metadata tokenization definitions.
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

#ifndef PARSER_H
#define PARSER_H

#include "streamx.h"
#include "packet.h"
#include "ringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_PAYLOAD_SIZE 4096

/* State Machine Definitions for Stream Ingestion */
typedef enum {
    STATE_MAGIC = 0,      /**< Waiting for 'STRX' magic signature bytes */
    STATE_HEADER,         /**< Gathering packet framing metadata header */
    STATE_PAYLOAD,        /**< Extracting raw payload contents */
    STATE_CHECKSUM        /**< Decoding frame checksum tag */
} parser_state_t;

/**
 * @brief Metadata Token structure containing parsed key-value attributes.
 */
typedef struct {
    char *key;
    char *value;
} metadata_token_t;

/**
 * @brief State-Machine Driven Stream Parser context.
 */
typedef struct {
    parser_state_t state;              /**< Current byte-by-byte ingestion state */
    uint32_t total_bytes_processed;    /**< Total accumulated parsed stream bytes */
    
    /* Temp variables for streaming frame extraction */
    uint8_t header_buf[9];             /**< Buffer for reading internal header parts (id(4), type(2), len(2)) */
    uint32_t header_bytes_read;        /**< Read cursor inside the streaming header */
    
    uint32_t current_id;               /**< Parsed transaction ID of active frame */
    uint16_t current_type;             /**< Parsed packet type code of active frame */
    uint16_t current_len;              /**< Payload extent of active frame */
    
    uint8_t *payload_buf;              /**< Assembly buffer for dynamic stream payload accumulation */
    uint32_t payload_bytes_read;       /**< Read cursor inside active streaming payload */

    uint16_t token_count;              /**< Current total of populated metadata tags */
    uint16_t token_capacity;           /**< Maximum token structures space reserved */
    metadata_token_t *tokens;          /**< Array of parsed token items */
} parser_t;

/**
 * @brief Create a state-machine stream parser context.
 * @param max_tokens Max key-value metadata capacities to reserve.
 * @return Context pointer, or NULL if allocation fails.
 */
parser_t *parser_create(uint16_t max_tokens);

/**
 * @brief Ingest incoming stream bytes from the ring buffer and parse complete frames.
 * @param parser Active parser state reference.
 * @param rb Target ring buffer containing streamed bytes.
 * @param out_packet Location to store dynamically extracted packet when parsed successfully.
 * @return STREAMX_OK if a frame was successfully extracted, STREAMX_ERR_EOF if more bytes are needed, or error code.
 */
streamx_status_t parser_parse_stream(parser_t *parser, ringbuf_t *rb, packet_t **out_packet);

/**
 * @brief Tokenize a key-value format metadata string (e.g. "k1=v1;k2=v2").
 * @param parser Active parser state.
 * @param meta_str Metadata serialized parameter list.
 * @return STREAMX_OK on total successful registration, or error code.
 */
streamx_status_t parser_tokenize_metadata(parser_t *parser, const char *meta_str);

/**
 * @brief Release parser allocated contexts and state assemblies.
 * @param parser Target context pointer to dispose.
 */
void parser_free(parser_t *parser);

#ifdef __cplusplus
}
#endif

#endif /* PARSER_H */
