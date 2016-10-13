/**
 * @file
 *
 * @version 1.0
 * @date Jun 25, 2014
 *
 * @section DESCRIPTION
 *
 * todo write description for process_special.c
 */
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "process.h"
#include "../logging.h"
#include "dcp.h"


/* Public Impl ****************************************************************/


int process_special(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts)
{
    UNUSED(oldpath);
    int r;
    dcp_state_t state;

    state = DCP_SPECIAL_CREATED;
    if ((r = mknodat(newdir->fd, newpath,
            (oldst->st_mode & S_IFMT) | 0666, oldst->st_rdev)) != 0)
    {
        log_error("cannot create special file '%s'", pathstr(newdir, newpath));
        state = DCP_FAILED;
    }

    if (r == 0 && fchownat(newdir->fd, newpath, opts->uid, opts->gid, 0) != 0)
        log_warn("cannot chown '%s'", pathstr(newdir, newpath));

    opts->callback(state, pathmd5, dapath, oldst, oldpath, NULL, NULL, NULL, NULL, NULL, -1,
            opts->callback_ctx);
    return r;
}

