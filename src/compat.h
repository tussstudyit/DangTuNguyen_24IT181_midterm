/**
 * @file compat.h
 * @brief Cross-platform compatibility definitions and function prototypes.
 *
 * Provides standard POSIX wrappers and Windows compatibility fallbacks
 * to ensure robust compilation on POSIX (Linux, NetBSD, macOS) and MinGW/Windows.
 */

#ifndef COMPAT_H
#define COMPAT_H

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
    #ifndef _UID_T_DEFINED
        typedef unsigned int uid_t;
        typedef unsigned int gid_t;
        #define _UID_T_DEFINED
        #define _GID_T_DEFINED
    #endif
#else
    #define PLATFORM_POSIX 1
#endif

/* Fallback definitions for file mode bits if missing */
#ifndef S_IFMT
    #define S_IFMT 0170000
#endif
#ifndef S_IFREG
    #define S_IFREG 0100000
#endif
#ifndef S_IFDIR
    #define S_IFDIR 0040000
#endif
#ifndef S_IFCHR
    #define S_IFCHR 0020000
#endif
#ifndef S_IFBLK
    #define S_IFBLK 0060000
#endif
#ifndef S_IFIFO
    #define S_IFIFO 0010000
#endif
#ifndef S_IFLNK
    #define S_IFLNK 0120000
#endif
#ifndef S_IFSOCK
    #define S_IFSOCK 0140000
#endif
#ifndef S_IFWHT
    #define S_IFWHT 0160000
#endif

#ifndef S_ISREG
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISCHR
    #define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISBLK
    #define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISFIFO
    #define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#ifndef S_ISLNK
    #define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISSOCK
    #define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#ifndef S_ISWHT
    #ifdef S_IFWHT
        #define S_ISWHT(m) (((m) & S_IFMT) == S_IFWHT)
    #else
        #define S_ISWHT(m) (0)
    #endif
#endif

/* Permission bits */
#ifndef S_ISUID
    #define S_ISUID 04000
#endif
#ifndef S_ISGID
    #define S_ISGID 02000
#endif
#ifndef S_ISVTX
    #define S_ISVTX 01000
#endif
#ifndef S_IRUSR
    #define S_IRUSR 00400
#endif
#ifndef S_IWUSR
    #define S_IWUSR 00200
#endif
#ifndef S_IXUSR
    #define S_IXUSR 00100
#endif
#ifndef S_IRGRP
    #define S_IRGRP 00040
#endif
#ifndef S_IWGRP
    #define S_IWGRP 00020
#endif
#ifndef S_IXGRP
    #define S_IXGRP 00010
#endif
#ifndef S_IROTH
    #define S_IROTH 00004
#endif
#ifndef S_IWOTH
    #define S_IWOTH 00002
#endif
#ifndef S_IXOTH
    #define S_IXOTH 00001
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves file status without following symbolic links (equivalent to POSIX lstat).
 */
int compat_lstat(const char *path, struct stat *sb);

/**
 * @brief Reads the value of a symbolic link (equivalent to POSIX readlink).
 */
ssize_t compat_readlink(const char *path, char *buf, size_t bufsiz);

/**
 * @brief Checks if stdout is attached to an interactive terminal.
 */
bool compat_is_terminal(void);

/**
 * @brief Checks if the running process has super-user (root) privileges.
 */
bool compat_is_superuser(void);

/**
 * @brief Looks up username for a given UID, or formats numeric string into buf.
 */
const char *compat_get_username(uid_t uid, char *buf, size_t buflen);

/**
 * @brief Looks up group name for a given GID, or formats numeric string into buf.
 */
const char *compat_get_groupname(gid_t gid, char *buf, size_t buflen);

/**
 * @brief Retrieves number of 512-byte blocks allocated to the file.
 */
uint64_t compat_get_blocks(const struct stat *sb);

/**
 * @brief Extracts major device number from dev_t.
 */
unsigned int compat_major(dev_t dev);

/**
 * @brief Extracts minor device number from dev_t.
 */
unsigned int compat_minor(dev_t dev);

/**
 * @brief Extracts the desired timestamp from struct stat based on time_type.
 */
time_t compat_get_file_time(const struct stat *sb, int time_type);

/**
 * @brief Extracts nanoseconds component of the desired timestamp if available.
 */
long compat_get_file_time_nsec(const struct stat *sb, int time_type);

/**
 * @brief Portable strdup implementation.
 */
char *compat_strdup(const char *s);

/**
 * @brief Joins parent directory and filename with appropriate path separator.
 */
char *compat_path_join(const char *dir, const char *file);

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_H */
