/*
 * cli.c — command-line front-end for LibStreamX.
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include "config.h"
#include "logger.h"
#include "parser.h"
#include "packet.h"
#include "ringbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_CHUNK 1024

void cli_print_help(const char *progname) {
    printf("Usage: %s [options] <input.bin>\n", progname);
    printf("\n");
    printf("Options:\n");
    printf("  --config PATH       Load runtime config from PATH\n");
    printf("  --log PATH          Append log output to PATH (default: stdout)\n");
    printf("  --max-tokens N      Max metadata tokens per parser (default: 16)\n");
    printf("  --verbose           Verbose logging\n");
    printf("  --help              Show this help and exit\n");
}

int cli_parse(int argc, char **argv, cli_options_t *out) {
    memset(out, 0, sizeof(*out));
    out->max_tokens = 16;

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            cli_print_help(argv[0]);
            return 1;
        }
        if (strcmp(arg, "--verbose") == 0) {
            out->verbose = 1;
            continue;
        }
        if (strcmp(arg, "--config") == 0) {
            strncpy(out->config_path, argv[i + 1], sizeof(out->config_path));
            out->has_config = 1;
            i++;
            continue;
        }
        if (strcmp(arg, "--log") == 0) {
            strncpy(out->log_path, argv[i + 1], sizeof(out->log_path));
            out->has_log = 1;
            i++;
            continue;
        }
        if (strcmp(arg, "--max-tokens") == 0) {
            out->max_tokens = atoi(argv[i + 1]);
            i++;
            continue;
        }

        /* Positional: input path. */
        strcpy(out->input_path, arg);
        out->has_input = 1;
    }

    if (!out->has_input) {
        fprintf(stderr, "error: no input file given\n");
        cli_print_help(argv[0]);
        return 2;
    }
    return 0;
}

int cli_run(const cli_options_t *opts) {
    config_t *cfg = NULL;
    if (opts->has_config) {
        cfg = config_load(opts->config_path);
        if (cfg == NULL) {
            logger_log(LOG_LEVEL_WARN, "config load failed; continuing with defaults");
        }
    }

    int max_tokens = opts->max_tokens;
    if (cfg != NULL) {
        max_tokens = config_get_int(cfg, "max_tokens", max_tokens);
    }

    FILE *fp = fopen(opts->input_path, "rb");
    if (fp == NULL) {
        logger_log(LOG_LEVEL_ERROR, "cannot open input %s", opts->input_path);
        config_free(cfg);
        return 1;
    }

    ringbuf_t *rb = ringbuf_create(8192);
    parser_t  *parser = parser_create((uint16_t)max_tokens);

    uint8_t chunk[READ_CHUNK];
    size_t  read_bytes;
    uint32_t total_packets = 0;

    while ((read_bytes = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        ringbuf_write(rb, chunk, (uint32_t)read_bytes);

        packet_t *pkt = NULL;
        streamx_status_t st;
        while ((st = parser_parse_stream(parser, rb, &pkt)) == STREAMX_OK) {
            total_packets++;
            if (opts->verbose) {
                logger_log(LOG_LEVEL_INFO,
                           "packet id=%u type=%u len=%u tag=%s",
                           pkt->id, pkt->type, pkt->length, pkt->tag);
            }
            packet_free(pkt);
            pkt = NULL;
        }
        (void)st; /* EOF means: need more bytes; not an error here. */
    }

    fclose(fp);
    parser_free(parser);
    ringbuf_free(rb);
    config_free(cfg);

    logger_log(LOG_LEVEL_INFO, "processed %u packets from %s",
               total_packets, opts->input_path);
    return 0;
}
