/**
 * @file ls.h
 * @brief Core header file for the UNIX ls(1) implementation.
 *
 * Course: Operating Systems / Systems Programming
 * Project: Midterm Project – Implement ls(1)
 * Author: Dang Tu Nguyen
 * Student ID: 24IT181
 * Specification: NetBSD 10.1 ls(1) General Commands Manual
 */

#ifndef LS_H
#define LS_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifndef _BSD_SOURCE
#define _BSD_SOURCE 1
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "compat.h"

/**
 * @brief Time selection mode for file timestamps (-c, -u, or default modification time).
 */
typedef enum {
    TIME_MODIFIED = 0, /**< Default: Last modification time (st_mtime) */
    TIME_STATUS_CHANGE,/**< -c: Time when file status was last changed (st_ctime) */
    TIME_LAST_ACCESS   /**< -u: Time of last access (st_atime) */
} time_type_t;

/**
 * @brief Output style for non-printable characters (-q vs -w).
 */
typedef enum {
    NONPRINT_DEFAULT = 0, /**< Terminal -> '?', non-terminal -> raw */
    NONPRINT_FORCE_Q,     /**< -q: Force printing as '?' */
    NONPRINT_FORCE_RAW    /**< -w: Force raw printing */
} nonprint_mode_t;

/**
 * @brief Long listing mode (-l vs -n).
 */
typedef enum {
    LONG_NONE = 0,  /**< Standard 1-entry-per-line */
    LONG_STANDARD,  /**< -l: Long listing with owner and group names */
    LONG_NUMERIC    /**< -n: Long listing with numeric UID and GID */
} long_mode_t;

/**
 * @brief Directory traversal / Plain mode (-R vs -d).
 */
typedef enum {
    DIR_NORMAL = 0,   /**< Normal directory listing */
    DIR_RECURSIVE,    /**< -R: Recursively list subdirectories encountered */
    DIR_PLAIN_FILE    /**< -d: Directories listed as plain files, no recursion */
} dir_mode_t;

/**
 * @brief Sorting criteria (-f, -S, -t, or lexicographical).
 */
typedef enum {
    SORT_LEXICOGRAPHICAL = 0, /**< Default: Lexicographical order by filename */
    SORT_SIZE,               /**< -S: Sort by file size, largest file first */
    SORT_TIME,               /**< -t: Sort by selected file timestamp, newest first */
    SORT_NONE                /**< -f: Output is not sorted */
} sort_type_t;

/**
 * @brief Global command-line configuration options.
 */
typedef struct {
    /* File filtering */
    bool opt_A;             /**< -A: List all entries except '.' and '..' (set for super-user) */
    bool opt_a;             /**< -a: Include entries starting with '.' */

    /* Time selection */
    time_type_t time_type;  /**< Time used for sorting (-t) and printing (-l) */

    /* Directory / recursion */
    dir_mode_t dir_mode;    /**< Normal, -R (recursive), or -d (plain file) */

    /* Type indicators and file flags */
    bool opt_F;             /**< -F: Display file type indicators (/ * @ % = |) */
    bool opt_i;             /**< -i: Display file serial number (inode) */

    /* Size and block display options */
    bool opt_s;             /**< -s: Display number of file system blocks used */
    bool opt_k;             /**< -k: Report sizes/blocks in 1024-byte units */
    bool opt_h;             /**< -h: Report sizes/blocks in human-readable format */
    long blocksize;         /**< Effective block size in bytes (from BLOCKSIZE or default 512) */

    /* Long listing */
    long_mode_t long_mode;  /**< Standard (-l) or numeric (-n) */

    /* Non-printable characters */
    nonprint_mode_t nonprint_mode; /**< -q (escape) vs -w (raw) */

    /* Sorting */
    sort_type_t sort_type;  /**< None (-f), Size (-S), Time (-t), or Name */
    bool opt_r;             /**< -r: Reverse sort order */

    /* Runtime context */
    bool stdout_is_tty;     /**< True if stdout is attached to a terminal */
} options_t;

/**
 * @brief Structure holding all required metadata for a single file or directory.
 */
typedef struct {
    char *name;             /**< Basename of the file (e.g. "foo.txt") */
    char *path;             /**< Relative or full path for file system operations */
    struct stat st;         /**< File attributes from stat/lstat */
    bool stat_ok;           /**< True if stat succeeded */
    int error_num;          /**< Errno if stat failed */
    char *link_target;      /**< Target path if file is a symbolic link */

    /* Pre-calculated fields for sorting and rendering */
    uint64_t display_blocks;/**< Blocks used in units of blocksize */
    time_t sort_time;       /**< Selected timestamp for sorting */
    long sort_time_nsec;    /**< Sub-second timestamp resolution */
    bool is_directory;      /**< True if operand is a directory */
} file_info_t;

/**
 * @brief A dynamic array of file_info_t items.
 */
typedef struct {
    file_info_t **items;    /**< Array of pointers to file items */
    size_t count;           /**< Number of items currently stored */
    size_t capacity;        /**< Allocated capacity of items array */
} file_list_t;

#endif /* LS_H */
