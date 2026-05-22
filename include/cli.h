/*
 * cli.h — command-line front-end for LibStreamX.
 * SPDX-License-Identifier: MIT
 */

#ifndef CLI_H
#define CLI_H

#include "streamx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char     input_path[256];
    char     config_path[256];
    char     log_path[256];
    int      max_tokens;
    int      verbose;
    int      has_input;
    int      has_config;
    int      has_log;
} cli_options_t;

/* Parse argv into a cli_options_t. Returns 0 on success, non-zero
 * if the user asked for --help or arguments were malformed. */
int cli_parse(int argc, char **argv, cli_options_t *out);

/* Print the CLI help text to stdout. */
void cli_print_help(const char *progname);

/* Run the parser end-to-end against the given options. */
int cli_run(const cli_options_t *opts);

#ifdef __cplusplus
}
#endif

#endif /* CLI_H */
