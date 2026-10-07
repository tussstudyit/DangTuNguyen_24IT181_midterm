/**
 * @file options.h
 * @brief Command-line option parsing for ls(1).
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include "ls.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes options to their default values according to NetBSD ls(1) specification.
 *
 * @param opts Pointer to options structure to initialize.
 */
void options_init(options_t *opts);

/**
 * @brief Parses command-line arguments and applies NetBSD option precedence rules.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param opts Pointer to options structure to fill.
 * @return Index in argv of the first non-option argument (file operands), or -1 on error.
 */
int options_parse(int argc, char *argv[], options_t *opts);

/**
 * @brief Prints usage message to stderr.
 *
 * @param progname The name of the binary.
 */
void options_usage(const char *progname);

#ifdef __cplusplus
}
#endif

#endif /* OPTIONS_H */
