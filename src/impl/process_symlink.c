/**
 * @file
 *
 * @version 1.0
 * @date Jun 25, 2014
 *
 * @section DESCRIPTION
 *
 * todo write description for process_symlink.c
 */
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>

#include "process.h"
#include "../logging.h"
#include "dcp.h"


/* Public Impl ****************************************************************/


int process_symlink(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts)
{
    int r;
    void *buf;
    size_t bufsize;
    dcp_state_t state;

    /* ensure the buffer is large enough */
    if (oldst->st_size < (off_t) (opts->buffer_size + 1))
    {
        bufsize = oldst->st_size + 1; /* include room for the null byte */
        buf = calloc(sizeof(char), bufsize);
    }
    else
    {
        buf = opts->buffer;
        bufsize = opts->buffer_size;
    }

    state = DCP_SYMLINK_CREATED;
    if ((r = readlink(oldpath, buf, bufsize)) == -1)
    {
        state = DCP_FAILED;
        log_error("cannot read symlink '%s'", oldpath);
    }
    else
    {
        /* create the symlink, unlinking an existing file if it exists */
        while ((r = symlinkat(buf, newdir->fd, newpath)) == -1)
        {
            if (errno == EEXIST)
            {
                if ((r = unlinkat(newdir->fd, newpath, 0)) == -1)
                {
                    state = DCP_FAILED;
                    log_error("cannot unlink '%s'", pathstr(newdir, newpath));
                    break;
                }
            }
            else
            {
                state = DCP_FAILED;
                log_error("cannot create symlink '%s'",
                        pathstr(newdir, newpath));
                break;
            }
        }
    }
    opts->callback(state, pathmd5, dapath, oldst, oldpath, buf, NULL, NULL, NULL, NULL, -1,
            opts->callback_ctx);

    /* if we allocated a new buffer free it */
    if (buf != opts->buffer)
        free(buf);

    return r;
}

