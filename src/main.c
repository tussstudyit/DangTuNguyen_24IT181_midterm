/**
 * @file main.c
 * @brief Entry point for the UNIX ls(1) utility.
 *
 * Course: Operating Systems / Systems Programming
 * Project: Midterm Project – Implement ls(1)
 * Author: Dang Tu Nguyen
 * Student ID: 24IT181
 * Specification: NetBSD 10.1 ls(1) General Commands Manual
 */

#include "ls.h"
#include "options.h"
#include "traverse.h"

#include <locale.h>
#include <time.h>

int main(int argc, char *argv[]) {
    /* Initialize locale and timezone based on environment */
    setlocale(LC_ALL, "");
#if defined(PLATFORM_POSIX)
    tzset();
#endif

    options_t opts;
    int opt_index = options_parse(argc, argv, &opts);
    if (opt_index < 0) {
        return EXIT_FAILURE;
    }

    int operand_count = argc - opt_index;
    char **operands = &argv[opt_index];

    int status = traverse_operands(operand_count, operands, &opts);

    return (status == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
