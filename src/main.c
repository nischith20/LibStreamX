/*
 * main.c — entry point for the `streamx-cli` binary.
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    cli_options_t opts;
    int rc = cli_parse(argc, argv, &opts);
    if (rc != 0) {
        return rc == 1 ? 0 : rc; /* --help is a success exit */
    }

    streamx_config_t init_cfg;
    memset(&init_cfg, 0, sizeof(init_cfg));
    init_cfg.max_packet_size = 4096;
    init_cfg.max_tokens = (uint16_t)opts.max_tokens;
    init_cfg.enable_shadow_mode = 0;
    init_cfg.log_file_path = opts.has_log ? opts.log_path : NULL;

    if (logger_init(init_cfg.log_file_path) != STREAMX_OK) {
        fprintf(stderr, "logger init failed\n");
        return 1;
    }

    logger_log(LOG_LEVEL_INFO, "streamx-cli starting, input=%s", opts.input_path);
    rc = cli_run(&opts);
    logger_close();
    return rc;
}
