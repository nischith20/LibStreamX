/*
 * packet.c — packet allocation, cloning, free.
 * SPDX-License-Identifier: MIT
 */

#include "packet.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

packet_t *packet_create(uint32_t id, uint16_t type, const uint8_t *payload,
                        uint32_t length, const char *tag) {
    packet_t *pkt = (packet_t *)malloc(sizeof(packet_t));
    if (pkt == NULL) {
        logger_log(LOG_LEVEL_ERROR, "packet_create: out of memory (id=%u)", id);
        return NULL;
    }

    pkt->id = id;
    pkt->type = type;
    pkt->length = length;

    if (length > 0) {
        pkt->payload = (uint8_t *)malloc(length);
        if (pkt->payload == NULL) {
            logger_log(LOG_LEVEL_ERROR,
                       "packet_create: payload allocation failed (id=%u)", id);
            free(pkt);
            return NULL;
        }
        if (payload != NULL) {
            memcpy(pkt->payload, payload, length);
        }
    }

    if (tag != NULL) {
        pkt->tag = strdup(tag);
        if (pkt->tag == NULL) {
            logger_log(LOG_LEVEL_ERROR,
                       "packet_create: tag dup failed (id=%u)", id);
            if (pkt->payload != NULL) {
                free(pkt->payload);
            }
            free(pkt);
            return NULL;
        }
    }

    return pkt;
}

packet_t *packet_clone(const packet_t *src) {
    if (src == NULL) {
        return NULL;
    }

    packet_t *dst = (packet_t *)malloc(sizeof(packet_t));
    if (dst == NULL) {
        logger_log(LOG_LEVEL_ERROR, "packet_clone: out of memory");
        return NULL;
    }

    memcpy(dst, src, sizeof(packet_t));

    logger_log(LOG_LEVEL_DEBUG, "cloned packet id=%u", src->id);
    return dst;
}

void packet_free(packet_t *pkt) {
    if (pkt == NULL) {
        return;
    }

    if (pkt->payload != NULL) {
        free(pkt->payload);
    }

    if (pkt->tag != NULL) {
        free(pkt->tag);
    }

    free(pkt);
}
