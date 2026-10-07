/**
 * @file display.c
 * @brief Output formatting implementation for NetBSD-compliant ls(1).
 */

#include "ls.h"
#include "display.h"
#include "file_utils.h"
#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

void display_file_list(const file_list_t *list, const options_t *opts, bool is_dir_contents) {
    if (!list || !opts) return;

    /* Calculate total blocks */
    uint64_t total_blocks = 0;
    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i] && list->items[i]->stat_ok) {
            total_blocks += list->items[i]->display_blocks;
        }
    }

    /* Print total line if required:
     * - In long format (-l or -n) for directory contents.
     * - In -s format when output is to a terminal for directory contents.
     */
    if (is_dir_contents) {
        if (opts->long_mode != LONG_NONE) {
            if (opts->opt_h) {
                char hbuf[32];
                long bs = (opts->blocksize >= 512) ? opts->blocksize : 512;
                file_format_human_size(total_blocks * bs, hbuf, sizeof(hbuf));
                printf("total %s\n", hbuf);
            } else {
                printf("total %" PRIu64 "\n", total_blocks);
            }
        } else if (opts->opt_s && opts->stdout_is_tty) {
            if (opts->opt_h) {
                char hbuf[32];
                long bs = (opts->blocksize >= 512) ? opts->blocksize : 512;
                file_format_human_size(total_blocks * bs, hbuf, sizeof(hbuf));
                printf("total %s\n", hbuf);
            } else {
                printf("total %" PRIu64 "\n", total_blocks);
            }
        }
    }

    /* If list is empty, nothing more to render */
    if (list->count == 0) {
        return;
    }

    /* Calculate maximum column widths for proper alignment */
    size_t max_inode_w = 0;
    size_t max_blocks_w = 0;
    size_t max_links_w = 0;
    size_t max_owner_w = 0;
    size_t max_group_w = 0;
    size_t max_size_w = 0;

    for (size_t i = 0; i < list->count; i++) {
        file_info_t *item = list->items[i];
        if (!item || !item->stat_ok) continue;

        char buf[64];

        /* Inode width */
        if (opts->opt_i) {
            int w = snprintf(buf, sizeof(buf), "%ju", (uintmax_t)item->st.st_ino);
            if (w > 0 && (size_t)w > max_inode_w) max_inode_w = (size_t)w;
        }

        /* Blocks width */
        if (opts->opt_s) {
            int w = 0;
            if (opts->opt_h) {
                long bs = (opts->blocksize >= 512) ? opts->blocksize : 512;
                file_format_human_size(item->display_blocks * bs, buf, sizeof(buf));
                w = (int)strlen(buf);
            } else {
                w = snprintf(buf, sizeof(buf), "%" PRIu64, item->display_blocks);
            }
            if (w > 0 && (size_t)w > max_blocks_w) max_blocks_w = (size_t)w;
        }

        /* Long format column widths */
        if (opts->long_mode != LONG_NONE) {
            /* Links */
            int w = snprintf(buf, sizeof(buf), "%lu", (unsigned long)item->st.st_nlink);
            if (w > 0 && (size_t)w > max_links_w) max_links_w = (size_t)w;

            /* Owner */
            if (opts->long_mode == LONG_NUMERIC) {
                snprintf(buf, sizeof(buf), "%u", (unsigned int)item->st.st_uid);
            } else {
                compat_get_username(item->st.st_uid, buf, sizeof(buf));
            }
            size_t ow = strlen(buf);
            if (ow > max_owner_w) max_owner_w = ow;

            /* Group */
            if (opts->long_mode == LONG_NUMERIC) {
                snprintf(buf, sizeof(buf), "%u", (unsigned int)item->st.st_gid);
            } else {
                compat_get_groupname(item->st.st_gid, buf, sizeof(buf));
            }
            size_t gw = strlen(buf);
            if (gw > max_group_w) max_group_w = gw;

            /* Size / Device */
            if (S_ISCHR(item->st.st_mode) || S_ISBLK(item->st.st_mode)) {
                snprintf(buf, sizeof(buf), "%3u, %3u",
                         compat_major(item->st.st_rdev),
                         compat_minor(item->st.st_rdev));
            } else if (opts->opt_h) {
                file_format_human_size((uint64_t)item->st.st_size, buf, sizeof(buf));
            } else {
                snprintf(buf, sizeof(buf), "%" PRIu64, (uint64_t)item->st.st_size);
            }
            size_t sw = strlen(buf);
            if (sw > max_size_w) max_size_w = sw;
        }
    }

    /* Print each item */
    for (size_t i = 0; i < list->count; i++) {
        file_info_t *item = list->items[i];
        if (!item) continue;

        if (!item->stat_ok) {
            /* If stat failed on an entry, report error */
            fprintf(stderr, "ls: %s: %s\n", item->name, strerror(item->error_num));
            continue;
        }

        /* 1. Inode number (-i) */
        if (opts->opt_i) {
            printf("%*ju ", (int)max_inode_w, (uintmax_t)item->st.st_ino);
        }

        /* 2. Block count (-s) */
        if (opts->opt_s) {
            char bbuf[64];
            if (opts->opt_h) {
                long bs = (opts->blocksize >= 512) ? opts->blocksize : 512;
                file_format_human_size(item->display_blocks * bs, bbuf, sizeof(bbuf));
            } else {
                snprintf(bbuf, sizeof(bbuf), "%" PRIu64, item->display_blocks);
            }
            printf("%*s ", (int)max_blocks_w, bbuf);
        }

        /* 3. Long format fields (-l / -n) */
        if (opts->long_mode != LONG_NONE) {
            /* Mode string */
            char mode_str[11];
            file_format_mode(item->st.st_mode, mode_str);
            printf("%s ", mode_str);

            /* Number of links */
            printf("%*lu ", (int)max_links_w, (unsigned long)item->st.st_nlink);

            /* Owner */
            char owner_str[128];
            if (opts->long_mode == LONG_NUMERIC) {
                snprintf(owner_str, sizeof(owner_str), "%u", (unsigned int)item->st.st_uid);
            } else {
                compat_get_username(item->st.st_uid, owner_str, sizeof(owner_str));
            }
            printf("%-*s  ", (int)max_owner_w, owner_str);

            /* Group */
            char group_str[128];
            if (opts->long_mode == LONG_NUMERIC) {
                snprintf(group_str, sizeof(group_str), "%u", (unsigned int)item->st.st_gid);
            } else {
                compat_get_groupname(item->st.st_gid, group_str, sizeof(group_str));
            }
            printf("%-*s  ", (int)max_group_w, group_str);

            /* Size or Device */
            char size_str[64];
            if (S_ISCHR(item->st.st_mode) || S_ISBLK(item->st.st_mode)) {
                snprintf(size_str, sizeof(size_str), "%3u, %3u",
                         compat_major(item->st.st_rdev),
                         compat_minor(item->st.st_rdev));
            } else if (opts->opt_h) {
                file_format_human_size((uint64_t)item->st.st_size, size_str, sizeof(size_str));
            } else {
                snprintf(size_str, sizeof(size_str), "%" PRIu64, (uint64_t)item->st.st_size);
            }
            printf("%*s ", (int)max_size_w, size_str);

            /* Timestamp */
            char date_str[32];
            file_format_time(item->sort_time, date_str, sizeof(date_str));
            printf("%s ", date_str);
        }

        /* 4. Pathname / filename */
        char *sanitized_name = file_sanitize_name(item->name, opts->nonprint_mode, opts->stdout_is_tty);
        printf("%s", sanitized_name ? sanitized_name : item->name);
        free(sanitized_name);

        /* 5. Type indicator (-F) */
        if (opts->opt_F) {
            char ind = file_get_indicator(item);
            if (ind != '\0') {
                putchar(ind);
            }
        }

        /* 6. Symbolic link target in long mode */
        if (opts->long_mode != LONG_NONE && item->link_target) {
            char *sanitized_target = file_sanitize_name(item->link_target, opts->nonprint_mode, opts->stdout_is_tty);
            printf(" -> %s", sanitized_target ? sanitized_target : item->link_target);
            free(sanitized_target);
        }

        putchar('\n');
    }
}
