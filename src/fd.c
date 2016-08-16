/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * todo write description for fd.c
 */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "fd.h"
#include "logging.h"


/* Private API ****************************************************************/


/**
 * wrapper for write syscall eliminating the chance for EINTR errno
 */
static ssize_t write_safe(int fd, const void *buf, size_t count);


/* Public Impl ****************************************************************/


int fd_pipe(int outfd, int infd, void *buffer, size_t blen)
{
    void *buf;
    ssize_t r;

    enum { BUFSIZE = 32768 };

    if (buffer == NULL)
    {
        if ((buf = malloc(BUFSIZE)) == NULL)
        {
            log_debug("malloc");
            return -1;
        }
        blen = BUFSIZE;
    }

    else
        buf = buffer;

    while ((r = fd_read(infd, buf, blen)) > 0)
    {
        if ((r = fd_write_full(outfd, buf, r)) == -1)
        {
            log_debug("fd_write");
            if (buffer == NULL)
                free(buf);
            return -1;
        }
    }

    if (r == -1)
    {
        log_debug("read_safe");
        if (buffer == NULL)
            free(buf);
        return -1;
    }

    if (buffer == NULL)
        free(buf);

    return 0;
}

/* wrapper for read syscall eliminating the chance for EINTR errno */
ssize_t fd_read(int fd, void *buf, size_t count)
{
    ssize_t result;
    for (;;)
    {
        if ((result = read(fd, buf, count)) < 0 && errno == EINTR)
            continue;
        else
            return result;
    }
    return -2; /* unreachable code, but compiler knows best! */
}


ssize_t fd_read_full(int fd, void *dest, size_t len)
{
    size_t total;
    ssize_t count;

    total = 0;
    while (total < len)
    {
        if ((count = fd_read(fd, ((unsigned char *) dest) + total,
                len - total)) == -1)
        {
            log_debug("read_safe");
            return -1;
        }

        /* break if EOF reached */
        if (count == 0)
            break;
        total += count;
    }
    return total;
}


ssize_t fd_write_full(int fd, const void *buf, size_t count)
{
    ssize_t result;
    size_t wrote;

    wrote = 0;
    while (wrote < count) {
        result = write_safe(fd, ((unsigned char *) buf) + wrote, count - wrote);
        if (result < 0)
            return result;
        wrote += result;
    }
    return wrote;
}


/* Private Impl ***************************************************************/


inline ssize_t write_safe(int fd, const void *buf, size_t count)
{
    ssize_t result;
    for (;;)
    {
        if ((result = write(fd, buf, count)) < 0 && errno == EINTR)
            continue;
        else
            return result;
    }
    return -2; /* unreachable code, but compiler knows best! */
}
