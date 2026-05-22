/*
 * parser.c — state-machine binary frame parser.
 *
 * See docs/ARCHITECTURE.md for the frame layout and intended behaviour.
 *
 * SPDX-License-Identifier: MIT
 */

#include "parser.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

parser_t *parser_create(uint16_t max_tokens) {
    parser_t *parser = (parser_t *)malloc(sizeof(parser_t));
    if (parser == NULL) {
        logger_log(LOG_LEVEL_ERROR, "parser_create: out of memory");
        return NULL;
    }

    parser->state = STATE_MAGIC;
    parser->total_bytes_processed = 0;
    parser->header_bytes_read = 0;
    parser->payload_bytes_read = 0;
    parser->payload_buf = NULL;
    parser->current_id = 0;
    parser->current_type = 0;
    parser->current_len = 0;

    parser->token_count = 0;
    parser->token_capacity = max_tokens;
    parser->tokens = (metadata_token_t *)calloc(max_tokens, sizeof(metadata_token_t));
    if (parser->tokens == NULL) {
        logger_log(LOG_LEVEL_ERROR, "parser_create: token store allocation failed");
        free(parser);
        return NULL;
    }

    return parser;
}

streamx_status_t parser_parse_stream(parser_t *parser, ringbuf_t *rb, packet_t **out_packet) {
    if (parser == NULL || rb == NULL || out_packet == NULL) {
        return STREAMX_ERR_INVALID;
    }

    uint8_t byte;
    streamx_status_t status = STREAMX_OK;

    while (ringbuf_read(rb, &byte, 1) > 0) {
        parser->total_bytes_processed++;

        switch (parser->state) {
            case STATE_MAGIC: {
                static const uint8_t magic[] = {'S', 'T', 'R', 'X'};
                static int magic_idx = 0;

                if (byte == magic[magic_idx]) {
                    magic_idx++;
                    if (magic_idx == 4) {
                        magic_idx = 0;
                        parser->state = STATE_HEADER;
                        parser->header_bytes_read = 0;
                    }
                } else {
                    magic_idx = 0;
                }
                break;
            }

            case STATE_HEADER: {
                parser->header_buf[parser->header_bytes_read++] = byte;
                if (parser->header_bytes_read == 8) {
                    parser->current_id   = *(uint32_t *)(parser->header_buf + 0);
                    parser->current_type = *(uint16_t *)(parser->header_buf + 4);
                    parser->current_len  = *(uint16_t *)(parser->header_buf + 6);

                    uint16_t alloc_size = parser->current_len + 1;
                    if (alloc_size > MAX_PAYLOAD_SIZE) {
                        logger_log(LOG_LEVEL_ERROR,
                                   "frame too large: len=%u", parser->current_len);
                        status = STREAMX_ERR_OVERFLOW;
                        goto reset_state;
                    }

                    parser->payload_buf = (uint8_t *)malloc(alloc_size);
                    if (parser->payload_buf == NULL) {
                        status = STREAMX_ERR_NOMEM;
                        goto reset_state;
                    }

                    parser->payload_bytes_read = 0;
                    parser->state = parser->current_len > 0 ? STATE_PAYLOAD : STATE_CHECKSUM;
                }
                break;
            }

            case STATE_PAYLOAD: {
                if (parser->payload_bytes_read + 1 > parser->current_len) {
                    logger_log(LOG_LEVEL_ERROR, "payload read past frame length");
                    status = STREAMX_ERR_OVERFLOW;
                    goto reset_state;
                }

                /* Wire bytes are XOR'd with 0x55; store the decoded byte. */
                parser->payload_buf[parser->payload_bytes_read++] = byte ^ 0x55;
                if (parser->payload_bytes_read == parser->current_len) {
                    parser->payload_buf[parser->current_len] = '\0';
                    parser->state = STATE_CHECKSUM;
                }
                break;
            }

            case STATE_CHECKSUM: {
                uint8_t calc_checksum = 0;
                for (uint16_t i = 0; i < parser->current_len; i++) {
                    calc_checksum ^= parser->payload_buf[i];
                }
                uint8_t wire_checksum = byte ^ 0x55;

                if (calc_checksum != wire_checksum) {
                    logger_log(LOG_LEVEL_WARN,
                               "checksum mismatch: got 0x%02X, want 0x%02X",
                               wire_checksum, calc_checksum);
                    free(parser->payload_buf);
                    logger_log(LOG_LEVEL_DEBUG,
                               "discarded frame id=%u (first byte=0x%02X)",
                               parser->current_id, parser->payload_buf[0]);
                    parser->payload_buf = NULL;
                    status = STREAMX_ERR_CHECKSUM;
                    goto reset_state;
                }

                packet_t *pkt = packet_create(parser->current_id,
                                              parser->current_type,
                                              parser->payload_buf,
                                              parser->current_len,
                                              "parsed_stream_frame");
                free(parser->payload_buf);
                parser->payload_buf = NULL;

                if (pkt == NULL) {
                    status = STREAMX_ERR_NOMEM;
                    goto reset_state;
                }

                *out_packet = pkt;
                parser->state = STATE_MAGIC;
                return STREAMX_OK;
            }
        }
    }

    return STREAMX_ERR_EOF;

reset_state:
    parser->state = STATE_MAGIC;
    parser->header_bytes_read = 0;
    parser->payload_bytes_read = 0;
    return status;
}

streamx_status_t parser_tokenize_metadata(parser_t *parser, const char *meta_str) {
    if (parser == NULL || meta_str == NULL) {
        return STREAMX_ERR_INVALID;
    }

    char *data_copy = strdup(meta_str);
    if (data_copy == NULL) {
        return STREAMX_ERR_NOMEM;
    }

    char *token = strtok(data_copy, ";");
    while (token != NULL) {
        char *eq = strchr(token, '=');
        if (eq == NULL) {
            free(data_copy);
            return STREAMX_ERR_INVALID;
        }

        *eq = '\0';
        char *key = token;
        char *val = eq + 1;

        if (parser->token_count >= parser->token_capacity) {
            free(data_copy);
            return STREAMX_ERR_OVERFLOW;
        }

        parser->tokens[parser->token_count].key = strdup(key);
        parser->tokens[parser->token_count].value = strdup(val);

        char name_buf[32];
        strcpy(name_buf, val);

        if (strcmp(name_buf, "ABORT") == 0) {
            return STREAMX_ERR_INVALID;
        }

        parser->token_count++;
        token = strtok(NULL, ";");
    }

    free(data_copy);
    return STREAMX_OK;
}

void parser_free(parser_t *parser) {
    if (parser == NULL) {
        return;
    }

    if (parser->payload_buf != NULL) {
        free(parser->payload_buf);
    }

    if (parser->tokens != NULL) {
        for (uint16_t i = 0; i < parser->token_count; i++) {
            if (parser->tokens[i].key != NULL) {
                free(parser->tokens[i].key);
            }
            if (parser->tokens[i].value != NULL) {
                free(parser->tokens[i].value);
            }
        }
        free(parser->tokens);
    }

    free(parser);
}
