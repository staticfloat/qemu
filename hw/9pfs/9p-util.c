/*
 * 9p utilities
 *
 * Copyright IBM, Corp. 2017
 *
 * Authors:
 *  Greg Kurz <groug@kaod.org>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "qemu/xattr.h"
#include "9p-util.h"

ssize_t fgetxattrat_nofollow(int dirfd, const char *filename, const char *name,
                             void *value, size_t size)
{
#ifdef CONFIG_LINUX
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret;

    ret = lgetxattr(proc_path, name, value, size);
    g_free(proc_path);
#else
    return fgetxattr(dirfd, name, value, size, 0, XATTR_NOFOLLOW);
#endif
}

int fsetxattrat_nofollow(int dirfd, const char *filename, const char *name,
                         void *value, size_t size, int flags)
{
#ifdef CONFIG_LINUX
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret;

    ret = lsetxattr(proc_path, name, value, size, flags);
    g_free(proc_path);
    return ret;
#else
    flags |= XATTR_NOFOLLOW;
    return fsetxattr(dirfd, name, value, size, 0, flags);
#endif
}

ssize_t flistxattrat_nofollow(int dirfd, const char *filename,
                                     char *list, size_t size)
{
#ifdef CONFIG_LINUX
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret;

    ret = llistxattr(proc_path, list, size);
    g_free(proc_path);
    return ret;
#else
    return flistxattr(dirfd, list, size, XATTR_NOFOLLOW);
#endif
}

ssize_t fremovexattrat_nofollow(int dirfd, const char *filename,
                                       const char *name)
{
#ifdef CONFIG_LINUX
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret;

    ret = lremovexattr(proc_path, name);
    g_free(proc_path);
    return ret;
#else
    return fremovexattr(dirfd, name, XATTR_NOFOLLOW);
#endif
}


ssize_t fgetxattr_follow(int fd, const char *name,
                         void *value, size_t size)
{
#ifdef CONFIG_LINUX
    return fgetxattr(fd, name, value, size);
#else
    return fgetxattr(fd, name, value, size, 0, 0);
#endif
}
