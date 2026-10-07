/**
 * @file sort.h
 * @brief Sorting engine for directory entries and command line operands.
 */

#ifndef SORT_H
#define SORT_H

#include "ls.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sorts a file list according to the active options (-f, -S, -t, -r, or default).
 *
 * @param list Pointer to file_list_t to sort in-place.
 * @param opts Pointer to program options.
 */
void sort_file_list(file_list_t *list, const options_t *opts);

#ifdef __cplusplus
}
#endif

#endif /* SORT_H */
