/*
 * 9p utilities (Darwin Implementation)
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
    int ret;
    int fd = openat_file(dirfd, filename,
                         O_RDONLY | O_PATH_9P_UTIL | O_NOFOLLOW, 0);
    if (fd == -1) {
        return -1;
    }
    ret = fgetxattr(fd, name, value, size, 0, 0);
    close_preserve_errno(fd);
    return ret;
}

ssize_t flistxattrat_nofollow(int dirfd, const char *filename,
                              char *list, size_t size)
{
    int ret;
    int fd = openat_file(dirfd, filename,
                         O_RDONLY | O_PATH_9P_UTIL | O_NOFOLLOW, 0);
    if (fd == -1) {
        return -1;
    }
    ret = flistxattr(fd, list, size, 0);
    close_preserve_errno(fd);
    return ret;
}

ssize_t fremovexattrat_nofollow(int dirfd, const char *filename,
                                const char *name)
{
    int ret;
    int fd = openat_file(dirfd, filename, O_PATH_9P_UTIL | O_NOFOLLOW, 0);
    if (fd == -1) {
        return -1;
    }
    ret = fremovexattr(fd, name, 0);
    close_preserve_errno(fd);
    return ret;
}

int fsetxattrat_nofollow(int dirfd, const char *filename, const char *name,
                         void *value, size_t size, int flags)
{
    int ret;
    int fd = openat_file(dirfd, filename, O_PATH_9P_UTIL | O_NOFOLLOW, 0);
    if (fd == -1) {
        return -1;
    }
    ret = fsetxattr(fd, name, value, size, 0, flags);
    close_preserve_errno(fd);
    return ret;
}

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

int utimensat_nofollow(int dirfd, const char *filename, const struct timespec times[2])
{
#if defined(__MAC_10_13) /* Check whether we have an SDK version that defines utimensat */
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_13
#define UTIMENSAT_AVAILABLE 1
#elif __has_builtin(__builtin_available)
#define UTIMENSAT_AVAILABLE __builtin_available(macos 10.13, *)
#else
#define UTIMENSAT_AVAILABLE 0
#endif
    if (UTIMENSAT_AVAILABLE)
    {
        return utimensat(dirfd, filename, times, AT_SYMLINK_NOFOLLOW);
    }
#endif
    // utimensat not available. Use futimes.
    int fd = openat_file(dirfd, filename, O_PATH_9P_UTIL | O_NOFOLLOW, 0);
    if (fd == -1)
        return -1;

    struct timeval futimes_buf[2];
    futimes_buf[0].tv_sec = times[0].tv_sec;
    futimes_buf[0].tv_usec = times[0].tv_nsec / 1000;
    futimes_buf[1].tv_sec = times[1].tv_sec;
    futimes_buf[1].tv_usec = times[1].tv_nsec / 1000;
    int ret = futimes(fd, futimes_buf);
    close_preserve_errno(fd);
    return ret;
}

#ifndef SYS___pthread_fchdir
# define SYS___pthread_fchdir 349
#endif

static int fchdir_thread_local(int fd)
{
	return syscall(SYS___pthread_fchdir, fd);
}

int qemu_mknodat(int dirfd, const char *filename, mode_t mode, dev_t dev)
{
    int preserved_errno, err;
    if (fchdir_thread_local(dirfd) < 0)
        return -1;
    err = mknod(filename, mode, dev);
    preserved_errno = errno;
    /* Stop using the thread-local cwd */
    fchdir_thread_local(-1);
    if (err < 0)
        errno = preserved_errno;
    return err;
}
