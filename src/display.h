/**
 * @file display.h
 * @brief Output formatting and rendering for ls(1).
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "ls.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Displays a list of file entries according to active options.
 *
 * @param list List of file items to print.
 * @param opts Active command-line options.
 * @param is_dir_contents True if printing contents of a directory (controls "total" line).
 */
void display_file_list(const file_list_t *list, const options_t *opts, bool is_dir_contents);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_H */
