/**
 * @file
 *
 * @version 1.0
 * @date Jul 7, 2014
 *
 * @section DESCRIPTION
 *
 * The implementation of preprocess at which prepares for a copy and outputs
 * verbose messages.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "process.h"
#include "logging.h"


/* Static Vars ****************************************************************/


static char PATHSTRBUF[PATH_MAX];


/* Public Impl ****************************************************************/


int preprocess(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, int verbose)
{
    struct stat st;

    if (fstatat(newdir->fd, newpath + 1, &st, 0) == -1)
    {
        if (errno == ENOENT)
            goto cleanup;

        else
        {
            log_error("cannot stat '%s'", pathstr(newdir, newpath));
            return -1;
        }
    }

    /* where we want to copy our file exists, let's validate we can remove it */
    if (S_ISDIR(oldst->st_mode))
    {
        /* if dest is a directory we are done, nothing to output*/
        if (S_ISDIR(st.st_mode))
            return 0;

        /* else error */
        log_errorx("cannot overwrite non-directory `%s' with directory `%s'",
                pathstr(newdir, newpath), oldpath);
        return -1;
    }

    else
    {
        /* old is not a directory but new exists and is a directory, error */
        if (S_ISDIR(st.st_mode))
        {
            log_errorx("cannot overwrite directory `%s' with non-directory "
                    "`%s'", pathstr(newdir, newpath), oldpath);
            return -1;
        }

        /* remove whatever is in new */
        if (unlinkat(newdir->fd, newpath + 1, 0) == -1)
        {
            log_error("cannot remove `%s'", pathstr(newdir, newpath));
            return -1;
        }

        if (verbose)
        {
            fprintf(stdout, "removed `%s'\n", pathstr(newdir, newpath));
            fflush(stdout);
        }
    }

    cleanup:
        if (verbose)
        {
            fprintf(stdout, "`%s' -> `%s'\n",oldpath,pathstr(newdir, newpath));
            fflush(stdout);
        }
        return 0;
}



const char *pathstr(const file_t *root, const char *path)
{
    snprintf(PATHSTRBUF, sizeof(PATHSTRBUF), "%s%s%s", root->path,
            strlen(root->path) == 0? "" : "/", path);
    return PATHSTRBUF;
}
