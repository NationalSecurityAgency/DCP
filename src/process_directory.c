/**
 * @file
 *
 * @version 1.0
 * @date Jun 25, 2014
 *
 * @section DESCRIPTION
 *
 * todo write description for process_directory.c
 */
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "process.h"
#include "logging.h"
#include "dcp.h"


/* Public Impl ****************************************************************/


int process_directory(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts)
{
    UNUSED(oldpath);
    UNUSED(oldst);
    UNUSED(dapath);
    UNUSED(pathmd5);

    if (fchownat(newdir->fd, newpath, opts->uid, opts->gid, 0) == -1)
        log_warn("cannot chown '%s'", pathstr(newdir, newpath));

    return 0;
}

