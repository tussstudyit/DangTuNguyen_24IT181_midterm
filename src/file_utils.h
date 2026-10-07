/**
 * @file file_utils.h
 * @brief Utilities for querying file metadata, formatting permissions, sizes, and timestamps.
 */

#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "ls.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates and populates a file_info_t structure.
 *
 * @param parent_dir Directory containing the file, or NULL/empty if path is self-contained.
 * @param name Basename of the file.
 * @param opts Program options for blocksize and time calculations.
 * @param follow_symlink If true, follows symlink for the initial operand (when not in -d mode).
 * @return Allocated file_info_t pointer, or NULL on memory allocation failure.
 */
file_info_t *file_info_create(const char *parent_dir, const char *name,
                              const options_t *opts, bool follow_symlink);

/**
 * @brief Frees all memory associated with a file_info_t structure.
 */
void file_info_destroy(file_info_t *info);

/**
 * @brief Creates an empty file list.
 */
file_list_t *file_list_create(void);

/**
 * @brief Appends a file_info_t item to the file list.
 */
void file_list_add(file_list_t *list, file_info_t *info);

/**
 * @brief Frees all items and the list container.
 */
void file_list_destroy(file_list_t *list);

/**
 * @brief Formats mode_t into a 10-character NetBSD mode string (e.g. "-rwxr-xr-x").
 *
 * @param mode File mode flags.
 * @param buf Output buffer of at least 11 bytes.
 */
void file_format_mode(mode_t mode, char buf[11]);

/**
 * @brief Converts byte size to human-readable format (-h option, e.g. "4.0K", "1.5M").
 *
 * @param bytes Number of bytes.
 * @param buf Output buffer.
 * @param buflen Size of output buffer.
 */
void file_format_human_size(uint64_t bytes, char *buf, size_t buflen);

/**
 * @brief Formats file timestamp (month, day-of-month, hour:min or year).
 *
 * @param ftime Timestamp to format.
 * @param buf Output buffer.
 * @param buflen Size of output buffer.
 */
void file_format_time(time_t ftime, char *buf, size_t buflen);

/**
 * @brief Determines the indicator character for the -F option (/ * @ % = |).
 *
 * @param info Pointer to file_info_t.
 * @return Indicator character, or '\0' if no indicator applies.
 */
char file_get_indicator(const file_info_t *info);

/**
 * @brief Sanitizes filename string according to -q / -w mode.
 *
 * @param name Raw filename.
 * @param mode Nonprint mode (escape to '?' or raw).
 * @param is_tty Terminal status.
 * @return Newly allocated sanitized string (caller must free).
 */
char *file_sanitize_name(const char *name, nonprint_mode_t mode, bool is_tty);

#ifdef __cplusplus
}
#endif

#endif /* FILE_UTILS_H */
