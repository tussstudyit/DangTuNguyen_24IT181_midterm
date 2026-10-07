/**
 * @file options.c
 * @brief Command-line option parsing implementation conforming to NetBSD ls(1).
 */

#include "options.h"
#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void options_init(options_t *opts) {
    if (!opts) return;
    memset(opts, 0, sizeof(*opts));

    /* Runtime context */
    opts->stdout_is_tty = compat_is_terminal();

    /* Default non-printable characters mode: '?' on terminal, raw otherwise */
    opts->nonprint_mode = NONPRINT_DEFAULT;

    /* -A is always set for super-user according to the NetBSD manual */
    if (compat_is_superuser()) {
        opts->opt_A = true;
    }

    /* Base block size resolution: inspect BLOCKSIZE environment variable */
    long blocksize = 512;
    const char *bs_env = getenv("BLOCKSIZE");
    if (bs_env && *bs_env) {
        char *endptr = NULL;
        long val = strtol(bs_env, &endptr, 10);
        if (val > 0) {
            if (*endptr == 'k' || *endptr == 'K') {
                val *= 1024;
            } else if (*endptr == 'm' || *endptr == 'M') {
                val *= 1024 * 1024;
            } else if (*endptr == 'g' || *endptr == 'G') {
                val *= 1024 * 1024 * 1024;
            }
            if (val >= 512) {
                blocksize = val;
            }
        }
    }
    opts->blocksize = blocksize;

    /* Default time: modification time */
    opts->time_type = TIME_MODIFIED;

    /* Default directory mode: normal traversal */
    opts->dir_mode = DIR_NORMAL;

    /* Default sorting: lexicographical order */
    opts->sort_type = SORT_LEXICOGRAPHICAL;

    /* Default listing: 1 entry per line */
    opts->long_mode = LONG_NONE;
}

int options_parse(int argc, char *argv[], options_t *opts) {
    if (!opts) return -1;
    options_init(opts);

    int i = 1;
    for (; i < argc; i++) {
        const char *arg = argv[i];

        /* Non-option argument reached */
        if (arg[0] != '-' || arg[1] == '\0') {
            break;
        }

        /* End of options delimiter "--" */
        if (strcmp(arg, "--") == 0) {
            i++;
            break;
        }

        /* Parse flag cluster, e.g. "-lRtra" */
        for (int j = 1; arg[j] != '\0'; j++) {
            char opt = arg[j];
            switch (opt) {
                case 'A':
                    opts->opt_A = true;
                    break;
                case 'a':
                    opts->opt_a = true;
                    break;
                case 'c':
                    /* -c and -u override each other; the last specified wins */
                    opts->time_type = TIME_STATUS_CHANGE;
                    break;
                case 'u':
                    /* -c and -u override each other; the last specified wins */
                    opts->time_type = TIME_LAST_ACCESS;
                    break;
                case 'd':
                    /* -R and -d override each other; the last specified wins */
                    opts->dir_mode = DIR_PLAIN_FILE;
                    break;
                case 'R':
                    /* -R and -d override each other; the last specified wins */
                    opts->dir_mode = DIR_RECURSIVE;
                    break;
                case 'F':
                    opts->opt_F = true;
                    break;
                case 'f':
                    /* -f: Output is not sorted; in BSD it also turns on -a */
                    opts->sort_type = SORT_NONE;
                    opts->opt_a = true;
                    break;
                case 'h':
                    /* Rightmost of -k and -h overrides */
                    opts->opt_h = true;
                    opts->opt_k = false;
                    break;
                case 'k':
                    /* Rightmost of -k and -h overrides */
                    opts->opt_k = true;
                    opts->opt_h = false;
                    break;
                case 'i':
                    opts->opt_i = true;
                    break;
                case 'l':
                    /* -l and -n override each other; the last specified wins */
                    opts->long_mode = LONG_STANDARD;
                    break;
                case 'n':
                    /* -l and -n override each other; the last specified wins */
                    opts->long_mode = LONG_NUMERIC;
                    break;
                case 'q':
                    /* -w and -q override each other; the last specified wins */
                    opts->nonprint_mode = NONPRINT_FORCE_Q;
                    break;
                case 'w':
                    /* -w and -q override each other; the last specified wins */
                    opts->nonprint_mode = NONPRINT_FORCE_RAW;
                    break;
                case 'r':
                    opts->opt_r = true;
                    break;
                case 'S':
                    /* Sorting mode */
                    opts->sort_type = SORT_SIZE;
                    break;
                case 's':
                    opts->opt_s = true;
                    break;
                case 't':
                    /* Sorting mode */
                    opts->sort_type = SORT_TIME;
                    break;
                default:
                    fprintf(stderr, "ls: unknown option -- %c\n", opt);
                    options_usage(argv[0]);
                    return -1;
            }
        }
    }

    /* Finalize block size: -k forces 1024 bytes */
    if (opts->opt_k) {
        opts->blocksize = 1024;
    }

    return i;
}

void options_usage(const char *progname) {
    fprintf(stderr, "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n",
            progname ? progname : "ls");
}
