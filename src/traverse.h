/**
 * @file traverse.h
 * @brief Directory traversal and operand management for ls(1).
 */

#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "ls.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Processes all command-line operands (or defaults to current directory).
 *
 * @param count Number of operand strings.
 * @param operands Array of operand paths.
 * @param opts Program options.
 * @return 0 on success, >0 if any error occurred.
 */
int traverse_operands(int count, char *operands[], const options_t *opts);

/**
 * @brief Traverses a single directory and displays its contents.
 *
 * @param path Path to the directory.
 * @param opts Program options.
 * @param print_header Whether to print the "path:" header before contents.
 * @param need_leading_newline Whether to print a newline before the header.
 * @return 0 on success, >0 on error.
 */
int traverse_directory(const char *path, const options_t *opts,
                       bool print_header, bool need_leading_newline);

#ifdef __cplusplus
}
#endif

#endif /* TRAVERSE_H */
