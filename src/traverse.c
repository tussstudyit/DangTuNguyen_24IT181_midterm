/**
 * @file traverse.c
 * @brief Directory traversal and operand handling implementation for ls(1).
 */

#include "ls.h"
#include "traverse.h"
#include "file_utils.h"
#include "sort.h"
#include "display.h"
#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

int traverse_directory(const char *path, const options_t *opts,
                       bool print_header, bool need_leading_newline) {
    if (!path || !opts) return 1;

    DIR *dir = opendir(path);
    if (!dir) {
        if (need_leading_newline) {
            putchar('\n');
        }
        if (print_header) {
            printf("%s:\n", path);
        }
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return 1;
    }

    if (need_leading_newline) {
        putchar('\n');
    }
    if (print_header) {
        printf("%s:\n", path);
    }

    file_list_t *list = file_list_create();
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        const char *dname = entry->d_name;

        /* Filter dot files based on -a and -A options */
        if (dname[0] == '.') {
            if (!opts->opt_a) {
                if (!opts->opt_A) {
                    continue; /* Skip all dot files */
                }
                /* -A: list all except '.' and '..' */
                if (strcmp(dname, ".") == 0 || strcmp(dname, "..") == 0) {
                    continue;
                }
            }
        }

        file_info_t *info = file_info_create(path, dname, opts, false);
        if (info) {
            file_list_add(list, info);
        }
    }
    closedir(dir);

    /* Sort entries according to active options */
    sort_file_list(list, opts);

    /* Display entries */
    display_file_list(list, opts, true);

    /* Handle recursive traversal (-R) */
    if (opts->dir_mode == DIR_RECURSIVE) {
        for (size_t i = 0; i < list->count; i++) {
            file_info_t *item = list->items[i];
            if (!item || !item->stat_ok) continue;

            /* Check if entry is a directory and not a symlink */
            if (S_ISDIR(item->st.st_mode) && !S_ISLNK(item->st.st_mode)) {
                /* Never recurse into '.' or '..' */
                if (strcmp(item->name, ".") == 0 || strcmp(item->name, "..") == 0) {
                    continue;
                }
                traverse_directory(item->path, opts, true, true);
            }
        }
    }

    file_list_destroy(list);
    return 0;
}

int traverse_operands(int count, char *operands[], const options_t *opts) {
    if (!opts) return 1;

    int exit_status = 0;

    /* If no operands are given, list the current directory */
    if (count == 0) {
        if (opts->dir_mode == DIR_PLAIN_FILE) {
            /* With -d, list '.' as a plain file */
            file_list_t *list = file_list_create();
            file_info_t *info = file_info_create(NULL, ".", opts, false);
            if (info) {
                file_list_add(list, info);
                display_file_list(list, opts, false);
            }
            file_list_destroy(list);
            return 0;
        } else {
            return traverse_directory(".", opts, false, false);
        }
    }

    file_list_t *non_dirs = file_list_create();
    file_list_t *dirs = file_list_create();

    /* Classify operands into non-directories and directories */
    for (int i = 0; i < count; i++) {
        const char *op = operands[i];

        bool follow_symlink = false;
        if (opts->dir_mode != DIR_PLAIN_FILE &&
            opts->long_mode == LONG_NONE &&
            !opts->opt_F) {
            follow_symlink = true;
        }

        file_info_t *info = file_info_create(NULL, op, opts, follow_symlink);
        if (!info || !info->stat_ok) {
            int err = info ? info->error_num : ENOENT;
            fprintf(stderr, "ls: %s: %s\n", op, strerror(err));
            exit_status = 1;
            if (info) file_info_destroy(info);
            continue;
        }

        if (opts->dir_mode == DIR_PLAIN_FILE) {
            /* -d: Directories are listed as plain files */
            file_list_add(non_dirs, info);
        } else if (info->is_directory) {
            file_list_add(dirs, info);
        } else {
            file_list_add(non_dirs, info);
        }
    }

    /* Sort non-directory operands and directory operands separately */
    sort_file_list(non_dirs, opts);
    sort_file_list(dirs, opts);

    /* Display non-directory operands first */
    if (non_dirs->count > 0) {
        display_file_list(non_dirs, opts, false);
    }

    bool printed_any = (non_dirs->count > 0);
    bool print_header = (count > 1);

    /* Display directory operands */
    for (size_t i = 0; i < dirs->count; i++) {
        file_info_t *dir_item = dirs->items[i];
        int res = traverse_directory(dir_item->path, opts, print_header, printed_any);
        if (res != 0) {
            exit_status = 1;
        }
        printed_any = true;
    }

    file_list_destroy(non_dirs);
    file_list_destroy(dirs);

    return exit_status;
}
