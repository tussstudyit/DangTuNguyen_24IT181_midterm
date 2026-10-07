/**
 * @file file_utils.c
 * @brief Implementation of file metadata query and formatting utilities.
 */

#include "ls.h"
#include "file_utils.h"
#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

file_info_t *file_info_create(const char *parent_dir, const char *name,
                              const options_t *opts, bool follow_symlink) {
    if (!name || !opts) return NULL;

    file_info_t *info = (file_info_t *)calloc(1, sizeof(file_info_t));
    if (!info) return NULL;

    info->name = compat_strdup(name);
    if (!info->name) {
        free(info);
        return NULL;
    }

    if (parent_dir && parent_dir[0] != '\0') {
        info->path = compat_path_join(parent_dir, name);
    } else {
        info->path = compat_strdup(name);
    }

    if (!info->path) {
        free(info->name);
        free(info);
        return NULL;
    }

    int res = compat_lstat(info->path, &info->st);
    if (res != 0) {
        info->stat_ok = false;
        info->error_num = errno;
        return info;
    }

    info->stat_ok = true;

    /* Handle symbolic link reading */
    if (S_ISLNK(info->st.st_mode)) {
        char linkbuf[4096];
        ssize_t llen = compat_readlink(info->path, linkbuf, sizeof(linkbuf) - 1);
        if (llen > 0) {
            linkbuf[llen] = '\0';
            info->link_target = compat_strdup(linkbuf);
        }

        /* If follow_symlink is true (e.g. command-line operand without -d),
         * check if target is a directory */
        if (follow_symlink) {
            struct stat target_st;
            if (stat(info->path, &target_st) == 0 && S_ISDIR(target_st.st_mode)) {
                info->is_directory = true;
            } else {
                info->is_directory = false;
            }
        } else {
            info->is_directory = false;
        }
    } else {
        info->is_directory = S_ISDIR(info->st.st_mode);
    }

    /* Timestamp calculations */
    info->sort_time = compat_get_file_time(&info->st, opts->time_type);
    info->sort_time_nsec = compat_get_file_time_nsec(&info->st, opts->time_type);

    /* Block calculation in units of blocksize, rounding up partial units */
    uint64_t raw_blocks = compat_get_blocks(&info->st);
    uint64_t bytes = raw_blocks * 512;
    long bs = (opts->blocksize >= 512) ? opts->blocksize : 512;
    info->display_blocks = (bytes + bs - 1) / bs;

    return info;
}

void file_info_destroy(file_info_t *info) {
    if (!info) return;
    free(info->name);
    free(info->path);
    free(info->link_target);
    free(info);
}

file_list_t *file_list_create(void) {
    file_list_t *list = (file_list_t *)calloc(1, sizeof(file_list_t));
    return list;
}

void file_list_add(file_list_t *list, file_info_t *info) {
    if (!list || !info) return;
    if (list->count >= list->capacity) {
        size_t new_cap = (list->capacity == 0) ? 16 : (list->capacity * 2);
        file_info_t **new_items = (file_info_t **)realloc(list->items, new_cap * sizeof(file_info_t *));
        if (!new_items) {
            fprintf(stderr, "ls: out of memory\n");
            exit(EXIT_FAILURE);
        }
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = info;
}

void file_list_destroy(file_list_t *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        file_info_destroy(list->items[i]);
    }
    free(list->items);
    free(list);
}

void file_format_mode(mode_t mode, char buf[11]) {
    /* Position 0: Entry type */
    char type = '-';
    if (S_ISDIR(mode))       type = 'd';
    else if (S_ISLNK(mode))  type = 'l';
    else if (S_ISCHR(mode))  type = 'c';
    else if (S_ISBLK(mode))  type = 'b';
    else if (S_ISFIFO(mode)) type = 'p';
    else if (S_ISSOCK(mode)) type = 's';
    else if (S_ISWHT(mode))  type = 'w';

    buf[0] = type;

    /* Owner permissions */
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        buf[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    /* Group permissions */
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        buf[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    /* Other permissions */
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        buf[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    buf[10] = '\0';
}

void file_format_human_size(uint64_t bytes, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return;

    static const char prefixes[] = "BKMGTPE";
    if (bytes < 1000) {
        snprintf(buf, buflen, "%" PRIu64 "B", bytes);
        return;
    }

    double val = (double)bytes;
    int idx = 0;
    while (val >= 1000.0 && idx < 6) {
        val /= 1024.0;
        idx++;
    }

    if (val < 10.0) {
        snprintf(buf, buflen, "%.1f%c", val, prefixes[idx]);
    } else {
        snprintf(buf, buflen, "%.0f%c", val, prefixes[idx]);
    }
}

void file_format_time(time_t ftime, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return;

    struct tm tm_buf;
    struct tm *tm_info = NULL;

#if defined(PLATFORM_POSIX)
    tm_info = localtime_r(&ftime, &tm_buf);
#else
    tm_info = localtime(&ftime);
    if (tm_info) {
        memcpy(&tm_buf, tm_info, sizeof(tm_buf));
        tm_info = &tm_buf;
    }
#endif

    if (!tm_info) {
        snprintf(buf, buflen, "            ");
        return;
    }

    time_t now = time(NULL);
    time_t six_months_ago = now - (6 * 30 * 24 * 3600);
    time_t one_hour_future = now + 3600;

    if (ftime < six_months_ago || ftime > one_hour_future) {
        /* If older than 6 months or in the future: display month, day, and year */
        strftime(buf, buflen, "%b %e  %Y", tm_info);
    } else {
        /* Standard display: month, day, hour:minute */
        strftime(buf, buflen, "%b %e %H:%M", tm_info);
    }
}

char file_get_indicator(const file_info_t *info) {
    if (!info || !info->stat_ok) return '\0';

    mode_t mode = info->st.st_mode;

    if (S_ISDIR(mode)) {
        return '/';
    }
    if (S_ISLNK(mode)) {
        return '@';
    }
    if (S_ISSOCK(mode)) {
        return '=';
    }
    if (S_ISFIFO(mode)) {
        return '|';
    }
    if (S_ISWHT(mode)) {
        return '%';
    }
    if (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH))) {
        return '*';
    }

    return '\0';
}

char *file_sanitize_name(const char *name, nonprint_mode_t mode, bool is_tty) {
    if (!name) return NULL;

    bool escape = false;
    if (mode == NONPRINT_FORCE_Q) {
        escape = true;
    } else if (mode == NONPRINT_FORCE_RAW) {
        escape = false;
    } else {
        /* NONPRINT_DEFAULT: terminal defaults to '?', non-terminal to raw */
        escape = is_tty;
    }

    size_t len = strlen(name);
    char *result = (char *)malloc(len + 1);
    if (!result) return NULL;

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (escape && !isprint(c)) {
            result[i] = '?';
        } else {
            result[i] = name[i];
        }
    }
    result[len] = '\0';
    return result;
}
