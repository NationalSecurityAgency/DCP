/**
 * @file
 *
 * @version 1.0
 * @date Jun 25, 2014
 *
 * @section DESCRIPTION
 *
 * Implementation for copying a regular file using an deduping index. For
 * regular files we need to generate their hash and gather stats
 */
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <malloc.h>
#include <sys/xattr.h>
#include <stdio.h>

#include "process.h"
#include "../digest.h"
#include "../fd.h"
#include "../index/index.h"
#include "../logging.h"
#include "dcp.h"

/* Type Defs ******************************************************************/


/**
 * used by both copy_fd and copy_mem this structure's fields provide impl
 * specific information to both functions.
 *
 * copy_fd   `fd` is the file descriptor to read from, `bytes` is a buffer to
 *           use and `count` is the size of the buffer.
 *
 * copy_mem  `fd` is ignored, `bytes` is a buffer containing the file's bytes
 *           and `count` is the number of valid bytes in the buffer.
 */
struct stream {
    int fd;
    void *bytes;
    size_t count;
};


/**
 * copy function signature that the copy_regular function utilizes. To simplify
 * the code this function allows input to be provided as an inmem buffer or a
 * file descriptor. @see copy_fd and @see copy_mem for implementations.
 */
typedef int (*copy_f)(int dirfd, const char *pathname, struct stream *stream,
        uid_t uid, gid_t gid);


/* Private API ****************************************************************/


/**
 * We were able to store the entire file in the buffer so write the buffer's
 * contents to disk.
 *
 * @param dirfd     fd to the parent directory of pathname
 * @param pathname  file to create copying the bytes from stream
 * @param stream    holds the buffer and number of valid bytes in it
 * @param uid       who will own the new file
 * @param gid       what group the new file will belong to
 *
 * @return          0 on success, -1 on failure with errno set
 */
static int copy_mem(int dirfd, const char *pathname, struct stream *stream,
        uid_t uid, gid_t gid);


/**
 * file was too big to be cached in the buffer, therefore reset the fd to
 * beginning of the file and read all bytes from it
 *
 * @param dirfd     fd to the parent directory of pathname
 * @param pathname  file to create copying the bytes from stream
 * @param stream    the file descriptor and buffer to use
 * @param uid       who will own the new file
 * @param gid       what group the new file will belong to
 *
 * @return          0 on success, -1 on failure with errno set
 */
static int copy_fd(int dirfd, const char *pathname, struct stream *stream,
        uid_t uid, gid_t gid);


/**
 * Read from the FD using the provided buffer. The # of bytes read from the last
 * `read` syscall is returned. If the file size < `blen` and count == file size
 * then the file is contained in the buffer.
 *
 * @param set       initialized digestset_t to update and finalize
 * @param fd        the file descriptor to read the bytes from till the end
 * @param buf       a preallocated buffer to use to read the bytes
 * @param blen      number of bytes in the buffer
 *
 * @return          number of bytes that are valid in buf, -1 on error
 */
static ssize_t cache_n_digest(digesterset_t *set, int fd, void *buf,
        size_t blen);

/**
 * Read from the FD using the provided buffer, update all the digests, finally
 * write the bytes to the destination.
 *
 * @param dirfd     fd to the parent directory of pathname
 * @param pathname  file to create copying the bytes from stream
 * @param uid       who will own the new file
 * @param gid       what group the new file will belong to
 * @param set       initialized digestset_t to update and finalize
 * @param fd        the file descriptor to read the bytes from till the end
 * @param buf       a preallocated buffer to use to read the bytes
 * @param blen      number of bytes in the buffer
 *
 * @return          number of bytes copied, -1 on error
 */
static ssize_t copy_n_digest(int dirfd, const char *pathname, uid_t uid,
        gid_t gid, digesterset_t *set, int fd, void *buf, size_t blen);


/* Public Impl ****************************************************************/


/*
 * Given a regular file do the following:
 *
 *      If index is not NULL
 *          1. Digest the file caching it in memory if possible
 *          2. Look to see if the file is in the index, if not copy the file
 *      else
 *          1. Hash the file while copying it to the destination
 */
int process_regular(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts)
{
    int ret;
    int s;
    digesterset_t dgstset;
    digest_t idxkeytype;
    ssize_t valid_len;
    struct stream datastream;

    dcp_state_t state;

    clock_t start;
    unsigned long diff;

    start = clock();

    idxkeytype = opts->index == NULL? 0 : index_get_digest_type(opts->index);

    if ((s = open(oldpath, O_RDONLY)) == -1)
    {
        log_error("cannot open '%s'", oldpath);
        opts->callback(DCP_FAILED, pathmd5, dapath, oldst, oldpath, NULL, NULL,
                NULL, NULL, NULL, -1, opts->callback_ctx);
        return -1;
    }

    ret = 0;

    /* ensure we create the hash needed for the and index */
    digesterset_create(&dgstset, opts->digests | idxkeytype);

    /*
     * there is no index to check against, just copy and digest at the same
     * time
     */
    if (opts->index == NULL)
    {
        valid_len = copy_n_digest(newdir->fd, newpath, opts->uid, opts->gid,
                &dgstset, s, opts->buffer, opts->buffer_size);

        if (valid_len < 0)
        {
            log_debugx("failed copying and hashing '%s'", oldpath);
            opts->callback(DCP_FAILED, pathmd5, dapath, oldst, oldpath, NULL,
                    NULL, NULL, NULL, NULL, -1, opts->callback_ctx);
            ret = -1;
            goto cleanup;
        }

        digesterset_finalize(&dgstset);

        /* calculate the number of milliseconds elapsed to process this file */
        diff = ((clock() - start) * 1000) / CLOCKS_PER_SEC;

        /* finally send the information to the file processor */
        opts->callback(DCP_FILE_COPIED, pathmd5, dapath, oldst, oldpath, NULL,
                digesterset_get_value(&dgstset, DGST_MD5),
                digesterset_get_value(&dgstset, DGST_SHA1),
                digesterset_get_value(&dgstset, DGST_SHA256),
                digesterset_get_value(&dgstset, DGST_SHA512),
                diff, opts->callback_ctx);
        ret = 0;
    }
    else
    {
        /* read in the file and calculate the desired digests */
        if ((valid_len = cache_n_digest(&dgstset, s, opts->buffer,
                opts->buffer_size)) == -1)
        {
            log_debugx("cannot calculate hashes for '%s'", oldpath);
            opts->callback(DCP_FAILED, pathmd5, dapath, oldst, oldpath, NULL,
                    NULL, NULL, NULL, NULL, -1, opts->callback_ctx);
            ret = -1;
            goto cleanup;
        }

        digesterset_finalize(&dgstset);

        if (opts->index != NULL)
        {
            switch (index_lookup(opts->index, pathmd5,
                    digesterset_get_value(&dgstset, idxkeytype)))
            {
            case INDEX_FAILED:
                log_debugx("error looking up entry in file index");
                ret = -1;
                goto cleanup;

            /* we have seen this file, skip it */
            case INDEX_SUCCESS:
                ret = 0;        /* set ret to success */
                goto cleanup;

            /* else continue with the copy */
            case INDEX_NO_ENTRY: {}
            }
        }

        /*
         * if cache_n_digest was able to store the whole file in the buffer then
         * we do not need to seek to the beginning of the fd and reread the
         * bytes
         */
        if (valid_len == oldst->st_size)
        {
            datastream.bytes = opts->buffer;
            datastream.count = valid_len;
            state = copy_mem(newdir->fd, newpath, &datastream, opts->uid,
                    opts->gid);
        }
        else
        {
            datastream.fd = s;
            datastream.bytes = opts->buffer;
            datastream.count = opts->buffer_size;
            state = copy_fd(newdir->fd, newpath, &datastream, opts->uid,
                    opts->gid);
        }

        /* calculate the number of milliseconds elapsed to process this file */
        diff = ((clock() - start) * 1000) / CLOCKS_PER_SEC;

        /* finally send the information to the file processor */
        opts->callback(state, pathmd5, dapath, oldst, oldpath, NULL,
                digesterset_get_value(&dgstset, DGST_MD5),
                digesterset_get_value(&dgstset, DGST_SHA1),
                digesterset_get_value(&dgstset, DGST_SHA256),
                digesterset_get_value(&dgstset, DGST_SHA512),
                diff, opts->callback_ctx);

        ret = (state == DCP_FAILED)? -1 : 0;
    }

    cleanup:
        digesterset_free(&dgstset);
        close(s);

    return ret;
}


/* Private Impl ***************************************************************/


int copy_fd(int dirfd, const char *pathname, struct stream *stream, uid_t uid,
        gid_t gid)
{
    int d;

    /* ensure the fd is at beginning of file */
    if (lseek(stream->fd, SEEK_SET, 0) == -1)
    {
        log_debug("lseek");
        return -1;
    }

    /* create/truncate the dest file and copy all the bytes */
    if ((d = openat(dirfd, pathname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        log_debug("openat '%s'", pathname);
        return -1;
    }

    /* copy all bytes from `fd` to `d` using `bytes` as a buffer to read to */
    if (fd_pipe(d, stream->fd, stream->bytes, stream->count) == -1)
    {
        close(d);
        log_debug("fd_pipe");
        return -1;
    }

    if (fchown(d, uid, gid) == -1)
        log_debug("fchown");

    /* do not report success here because there can be data loss */
    if (close(d) == -1)
    {
        log_debug("close");
        return -1;
    }
    return 0;
}


int copy_mem(int dirfd, const char *pathname, struct stream *stream, uid_t uid,
        gid_t gid)
{
    int d;

    /* create/truncate the dest file and copy all the bytes */
    if ((d = openat(dirfd, pathname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        log_debug("openat '%s'", pathname);
        return -1;
    }

    /* write all the bytes */
    if (fd_write_full(d, stream->bytes, stream->count) == -1)
    {
        log_debug("fd_write");
        close(d);
        return -1;
    }

    if (fchown(d, uid, gid) == -1)
        log_debug("fchown");

    /* do not report success here because there can be data loss */
    if (close(d) == -1)
    {
        log_error("closing '%s' failed, possible data loss", pathname);
        return -1;
    }
    return 0;
}


/*
 * This function tries to cache the file into memory, by maintaining the number
 * of valid bytes in the buffer.
 *
 * If the file is larger than the buffer it will overwrite the bytes in the
 * buffer.
 *
 * The idea is if the file is smaller than the buffer provided we should read
 * the whole file into the buffer allowing it to be used later on instead of
 * needing to be reread from the kernel.
 */
ssize_t cache_n_digest(digesterset_t *set, int fd, void *buf, size_t blen)
{
    ssize_t result;
    size_t total;
    unsigned char *pos;

    /* causes the kernel to double its read ahead buffer for this file */
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    total = 0;
    for (;;)
    {
        /* we have filled the buffer, rollover and fill again */
        if (blen == total)
            total = 0;

        pos = ((unsigned char *) buf) + total;
        result = fd_read(fd, pos, blen - total);

        if (result < 0)     return -1;
        if (result == 0)    break;

        digesterset_update(set, pos, result);
        total += result;
    }

    return total;
}


ssize_t copy_n_digest(int dirfd, const char *pathname, uid_t uid, gid_t gid,
        digesterset_t *set, int fd, void *buf, size_t blen)
{
    ssize_t result;
    size_t total;
    int d;

    /* causes the kernel to double its read ahead buffer for this file */
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    /* create/truncate the dest file and copy all the bytes */
    if ((d = openat(dirfd, pathname, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
    {
        log_debug("openat '%s'", pathname);
        return -1;
    }

    total = 0;
    for (;;)
    {
        result = fd_read(fd, buf, blen);

        if (result < 0)     return -1;
        if (result == 0)    break;

        /* update the digests */
        digesterset_update(set, buf, result);

        /* write all the bytes */
        if (fd_write_full(d, buf, result) == -1)
        {
            log_debug("fd_write");
            close(d);
            return -1;
        }

        total += result;
    }

    if (fchown(d, uid, gid) == -1)
        log_debug("fchown");

    /* do not report success here because there can be data loss */
    if (close(d) == -1)
    {
        log_error("closing '%s' failed, possible data loss", pathname);
        return -1;
    }

    return total;
}

