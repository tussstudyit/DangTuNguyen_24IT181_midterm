/**
 * @file compat.c
 * @brief Cross-platform compatibility implementation.
 *
 * Implements POSIX APIs for UNIX environments, with fully functional
 * compatibility fallbacks for Windows / MinGW environments.
 */

#include "ls.h"
#include "compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(PLATFORM_POSIX)
    #include <unistd.h>
    #include <pwd.h>
    #include <grp.h>
    #if defined(__linux__)
        #include <sys/sysmacros.h>
    #endif
#elif defined(PLATFORM_WINDOWS)
    #include <io.h>
    #include <windows.h>
    #include <winioctl.h>
#endif

int compat_lstat(const char *path, struct stat *sb) {
    if (!path || !sb) {
        errno = EINVAL;
        return -1;
    }
#if defined(PLATFORM_POSIX)
    return lstat(path, sb);
#else
    /* Windows fallback: check attributes and stat */
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        errno = ENOENT;
        return -1;
    }
    int res = stat(path, sb);
    if (res == 0) {
        if (attr & FILE_ATTRIBUTE_DIRECTORY) {
            sb->st_mode = (sb->st_mode & ~S_IFMT) | S_IFDIR;
        }
        if (attr & FILE_ATTRIBUTE_REPARSE_POINT) {
            sb->st_mode = (sb->st_mode & ~S_IFMT) | S_IFLNK;
        }
    }
    return res;
#endif
}

ssize_t compat_readlink(const char *path, char *buf, size_t bufsiz) {
    if (!path || !buf || bufsiz == 0) {
        errno = EINVAL;
        return -1;
    }
#if defined(PLATFORM_POSIX)
    return readlink(path, buf, bufsiz);
#else
    /* Windows fallback using Win32 API */
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        errno = ENOENT;
        return -1;
    }

    typedef struct {
        USHORT SubstituteNameOffset;
        USHORT SubstituteNameLength;
        USHORT PrintNameOffset;
        USHORT PrintNameLength;
        ULONG  Flags;
        WCHAR  PathBuffer[1];
    } MY_SYMLINK_DATA;

    typedef struct {
        USHORT SubstituteNameOffset;
        USHORT SubstituteNameLength;
        USHORT PrintNameOffset;
        USHORT PrintNameLength;
        WCHAR  PathBuffer[1];
    } MY_MOUNT_DATA;

    typedef struct {
        ULONG  ReparseTag;
        USHORT ReparseDataLength;
        USHORT Reserved;
        union {
            MY_SYMLINK_DATA SymbolicLink;
            MY_MOUNT_DATA   MountPoint;
        } u_data;
    } MY_REPARSE_DATA_BUFFER;

    BYTE buffer[1024];
    DWORD bytesReturned = 0;
    BOOL ok = DeviceIoControl(
        hFile,
        FSCTL_GET_REPARSE_POINT,
        NULL, 0,
        buffer, sizeof(buffer),
        &bytesReturned,
        NULL
    );
    CloseHandle(hFile);

    if (!ok) {
        errno = EINVAL;
        return -1;
    }

    MY_REPARSE_DATA_BUFFER *rdb = (MY_REPARSE_DATA_BUFFER *)buffer;
    WCHAR *targetW = NULL;
    USHORT targetLenW = 0;

    if (rdb->ReparseTag == IO_REPARSE_TAG_SYMLINK) {
        targetW = (WCHAR *)((char *)rdb->u_data.SymbolicLink.PathBuffer +
                            rdb->u_data.SymbolicLink.PrintNameOffset);
        targetLenW = rdb->u_data.SymbolicLink.PrintNameLength / sizeof(WCHAR);
        if (targetLenW == 0) {
            targetW = (WCHAR *)((char *)rdb->u_data.SymbolicLink.PathBuffer +
                                rdb->u_data.SymbolicLink.SubstituteNameOffset);
            targetLenW = rdb->u_data.SymbolicLink.SubstituteNameLength / sizeof(WCHAR);
        }
    } else if (rdb->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
        targetW = (WCHAR *)((char *)rdb->u_data.MountPoint.PathBuffer +
                            rdb->u_data.MountPoint.PrintNameOffset);
        targetLenW = rdb->u_data.MountPoint.PrintNameLength / sizeof(WCHAR);
        if (targetLenW == 0) {
            targetW = (WCHAR *)((char *)rdb->u_data.MountPoint.PathBuffer +
                                rdb->u_data.MountPoint.SubstituteNameOffset);
            targetLenW = rdb->u_data.MountPoint.SubstituteNameLength / sizeof(WCHAR);
        }
    } else {
        errno = EINVAL;
        return -1;
    }

    int converted = WideCharToMultiByte(CP_UTF8, 0, targetW, targetLenW, buf, (int)bufsiz - 1, NULL, NULL);
    if (converted <= 0) {
        errno = EINVAL;
        return -1;
    }
    buf[converted] = '\0';
    return (ssize_t)converted;
#endif
}

bool compat_is_terminal(void) {
#if defined(PLATFORM_POSIX)
    return isatty(STDOUT_FILENO);
#else
    return _isatty(_fileno(stdout));
#endif
}

bool compat_is_superuser(void) {
#if defined(PLATFORM_POSIX)
    return (geteuid() == 0);
#else
    /* Windows check for Administrator token */
    BOOL is_admin = FALSE;
    PSID admin_group = NULL;
    SID_IDENTIFIER_AUTHORITY nt_auth = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&nt_auth, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin_group)) {
        CheckTokenMembership(NULL, admin_group, &is_admin);
        FreeSid(admin_group);
    }
    return is_admin ? true : false;
#endif
}

const char *compat_get_username(uid_t uid, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return "";
#if defined(PLATFORM_POSIX)
    struct passwd *pw = getpwuid(uid);
    if (pw && pw->pw_name) {
        snprintf(buf, buflen, "%s", pw->pw_name);
        return buf;
    }
    /* Fallback: format UID numerically */
    snprintf(buf, buflen, "%u", (unsigned int)uid);
    return buf;
#else
    (void)uid;
    const char *user = getenv("USERNAME");
    if (!user || user[0] == '\0') user = "user";
    snprintf(buf, buflen, "%s", user);
    return buf;
#endif
}

const char *compat_get_groupname(gid_t gid, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return "";
#if defined(PLATFORM_POSIX)
    struct group *gr = getgrgid(gid);
    if (gr && gr->gr_name) {
        snprintf(buf, buflen, "%s", gr->gr_name);
        return buf;
    }
    /* Fallback: format GID numerically */
    snprintf(buf, buflen, "%u", (unsigned int)gid);
    return buf;
#else
    (void)gid;
    snprintf(buf, buflen, "%s", "users");
    return buf;
#endif
}

uint64_t compat_get_blocks(const struct stat *sb) {
    if (!sb) return 0;
#if defined(PLATFORM_POSIX)
    return (uint64_t)sb->st_blocks;
#else
    /* Windows stat does not include st_blocks; approximate 512-byte blocks */
    if (sb->st_size <= 0) return 0;
    return ((uint64_t)sb->st_size + 511) / 512;
#endif
}

unsigned int compat_major(dev_t dev) {
#if defined(PLATFORM_POSIX) && defined(major)
    return (unsigned int)major(dev);
#elif defined(PLATFORM_POSIX) && defined(gnu_dev_major)
    return (unsigned int)gnu_dev_major(dev);
#else
    (void)dev;
    return 0;
#endif
}

unsigned int compat_minor(dev_t dev) {
#if defined(PLATFORM_POSIX) && defined(minor)
    return (unsigned int)minor(dev);
#elif defined(PLATFORM_POSIX) && defined(gnu_dev_minor)
    return (unsigned int)gnu_dev_minor(dev);
#else
    (void)dev;
    return 0;
#endif
}

time_t compat_get_file_time(const struct stat *sb, int time_type) {
    if (!sb) return 0;
    switch (time_type) {
        case TIME_STATUS_CHANGE:
            return sb->st_ctime;
        case TIME_LAST_ACCESS:
            return sb->st_atime;
        case TIME_MODIFIED:
        default:
            return sb->st_mtime;
    }
}

long compat_get_file_time_nsec(const struct stat *sb, int time_type) {
    if (!sb) return 0;
#if defined(__NetBSD__) || defined(__linux__) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)
    switch (time_type) {
        case TIME_STATUS_CHANGE:
            return sb->st_ctim.tv_nsec;
        case TIME_LAST_ACCESS:
            return sb->st_atim.tv_nsec;
        case TIME_MODIFIED:
        default:
            return sb->st_mtim.tv_nsec;
    }
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    switch (time_type) {
        case TIME_STATUS_CHANGE:
            return sb->st_ctimespec.tv_nsec;
        case TIME_LAST_ACCESS:
            return sb->st_atimespec.tv_nsec;
        case TIME_MODIFIED:
        default:
            return sb->st_mtimespec.tv_nsec;
    }
#else
    (void)time_type;
    return 0;
#endif
}

char *compat_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

char *compat_path_join(const char *dir, const char *file) {
    if (!dir || !file) return NULL;
    size_t dlen = strlen(dir);
    size_t flen = strlen(file);

    /* Check if directory already ends with a slash or backslash */
    bool needs_sep = true;
    if (dlen > 0 && (dir[dlen - 1] == '/' || dir[dlen - 1] == '\\')) {
        needs_sep = false;
    }

    char *result = (char *)malloc(dlen + (needs_sep ? 1 : 0) + flen + 1);
    if (!result) return NULL;

    if (needs_sep) {
        snprintf(result, dlen + 2 + flen, "%s/%s", dir, file);
    } else {
        snprintf(result, dlen + 1 + flen, "%s%s", dir, file);
    }
    return result;
}
