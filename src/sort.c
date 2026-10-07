/**
 * @file sort.c
 * @brief Implementation of the sorting engine for ls(1).
 */

#include "ls.h"
#include "sort.h"
#include <string.h>
#include <stdlib.h>

/* Context pointer for qsort_r or wrapper */
static const options_t *current_sort_opts = NULL;

static int compare_files(const void *p1, const void *p2) {
    const file_info_t *a = *(const file_info_t * const *)p1;
    const file_info_t *b = *(const file_info_t * const *)p2;

    if (!a || !b) return 0;

    int cmp = 0;

    if (current_sort_opts->sort_type == SORT_SIZE) {
        if (a->stat_ok && b->stat_ok) {
            if (a->st.st_size < b->st.st_size) {
                cmp = 1;  /* Largest first */
            } else if (a->st.st_size > b->st.st_size) {
                cmp = -1;
            }
        }
    } else if (current_sort_opts->sort_type == SORT_TIME) {
        if (a->stat_ok && b->stat_ok) {
            if (a->sort_time < b->sort_time) {
                cmp = 1;  /* Most recent first */
            } else if (a->sort_time > b->sort_time) {
                cmp = -1;
            } else {
                /* Sub-second tie breaking */
                if (a->sort_time_nsec < b->sort_time_nsec) {
                    cmp = 1;
                } else if (a->sort_time_nsec > b->sort_time_nsec) {
                    cmp = -1;
                }
            }
        }
    }

    /* Tie-breaker: Lexicographical order by filename */
    if (cmp == 0) {
        cmp = strcmp(a->name, b->name);
    }

    /* Reverse order if -r is set */
    if (current_sort_opts->opt_r) {
        cmp = -cmp;
    }

    return cmp;
}

void sort_file_list(file_list_t *list, const options_t *opts) {
    if (!list || list->count <= 1 || !opts) return;

    /* -f: Output is not sorted */
    if (opts->sort_type == SORT_NONE) {
        return;
    }

    current_sort_opts = opts;
    qsort(list->items, list->count, sizeof(file_info_t *), compare_files);
    current_sort_opts = NULL;
}
