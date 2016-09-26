/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * todo write description for dcp.c
 */

 /* for asprintf */
#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dcp.h"
#include "../index/index.h"
#include "../logging.h"

#include "process.h"


/* Macros *********************************************************************/


/**
 * Removes a trailing '/' from the string if it exists
 */
#define REMOVE_TRAILING_SLASHES(_str)                           \
    do {                                                        \
        size_t _len = strlen(_str);                             \
        while (_str[_len - 1 ] == '/') { _str[--_len] = '\0'; } \
    } while(0)


/* Private API ****************************************************************/


/**
 * create a copy of the file described by `ent` at `newdir`/`newpath`
 */
static inline void process(file_t *newdir, const char *newpath, FTSENT *ent,
        const char *dapath, const void *pathmd5, struct process_opts *popts,
        int verbose);


static int initdestandpaths(file_t *dest, char *path, const char **destpath,
        const char **dapath, const char *newpath, size_t src_count);


static inline int do_append(FTSENT *ent, const file_t *destroot,
        const char *newpath);


static inline int do_unappend(FTSENT *ent);


/* Public Impl ****************************************************************/


const char *dcp_strstate(dcp_state_t state)
{
    switch (state)
    {
    case DCP_FILE_COPIED:     return "FILE_COPIED";
    case DCP_FAILED:          return "FILE_FAILED";
    case DCP_DIR_CREATED:     return "DIR_CREATED";
    case DCP_SYMLINK_CREATED: return "SYMLINK_CREATED";
    case DCP_SPECIAL_CREATED: return "SPECIAL_CREATED";
    case DCP_DIR_FAILED:      return "DIR_FAILED";
    default: return "";
    }
}


int dcp(const char *newpath, const char *src[], size_t srcc,
        struct dcp_options *opts, dcp_callback_f callback, void *ctx)
{
    int r;
    size_t i;
    const char **paths;     /* fts expects a NULL terminated list of c strs */
    FTS *fts;               /* pointer to the fts library's handle */
    FTSENT *ent;            /* entry in the walk returned by fts_read */
    void *buf;              /* pointer to the buffer to use to cache files */
    char dapathmd5[MD5_DIGEST_LENGTH];

    /* dapath is the reported path, destpath is the path to the new file */
    char *path;                      /* buf for dapath and destpath to point */
    const char *dapath;              /* The current Destination Absolute Path */
    const char *destpath;

    /* struct used by the process_* functions for parameters that are common
     * between them all */
    struct process_opts popts;

    file_t destroot;
    char *sanitized;

    /* allow paths upto this max, kernel will error before we reach it */
    enum { MAX_LENGTH = PATH_MAX * 2 };

    assert(srcc != 0);

    sanitized = strdup(newpath);
    REMOVE_TRAILING_SLASHES(sanitized);
    newpath = sanitized;

    /* allocate the buffer to build our dest and dapaths in */
    path = malloc(MAX_LENGTH);
    dapath = NULL;
    destpath = NULL;

    /*
     * dest is the directory to clone every entry in, if newpath is an
     * existing directory dest represents that, otherwise it is the parent
     * of the file/directory to create
     */
    if (initdestandpaths(&destroot,path,&destpath,&dapath,sanitized,srcc) != 0)
    {
        free(path);
        free(sanitized);
        return -1;
    }

    /* setup the buffer to use, default if 0 */
    if (opts->bufsize == 0)
        opts->bufsize = (8 * 4096);

    if ((buf = malloc(opts->bufsize)) == NULL)
    {
        log_error("cannot allocate buffer of size %zu bytes", opts->bufsize);
        close(destroot.fd);
        free(destroot.path);
        free(path);
        free(sanitized);
        return -1;
    }

    /* map source paths to a null terminated paths array */
    paths = calloc(srcc + 1, sizeof(char *));
    for (i = 0; i < srcc; i++)
        paths[i] = src[i];

    /* put static parameters into the process_opts struct */
    popts.buffer       = buf;
    popts.buffer_size  = opts->bufsize;
    popts.digests      = opts->digests;
    popts.uid          = opts->uid;
    popts.gid          = opts->gid;
    popts.index        = opts->index;
    popts.callback     = callback;
    popts.callback_ctx = ctx;

    r = 0;
    /* begin the directory walk - physical so links are not followed */
    fts = fts_open((char * const *) paths, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
    while ((ent = fts_read(fts)) != NULL)
    {
        /* update the destination path for this entry */
        if (do_append(ent, &destroot, sanitized))
            strcat(strcat(path, "/"), ent->fts_name);

        /*
         * unfortunatly we have two cases where dapath wont be set right
         *  1. cp a dir to a dir that dne: report dapath as "/"
         *  2. cp a file to a path that dne: report dapath as "/$filename"
         */
        char *reported_dapath;
        if (strlen(dapath) == 0)
        {
            if (S_ISDIR(ent->fts_statp->st_mode))
                reported_dapath = strdup("/");
            else
                if(asprintf(&reported_dapath, "/%s", destpath) < 0)
                    reported_dapath = NULL;
        }
        else
            reported_dapath = (char *) dapath;


        digest(DGST_MD5, dapathmd5, reported_dapath, strlen(reported_dapath));
        process(&destroot, destpath, ent, reported_dapath, dapathmd5, &popts,
                opts->verbose);

        /* check pointers, no need to check string contents */
        if (reported_dapath != dapath)
            free(reported_dapath);

        /* do not remove a directory entry if this is a directory preorder */
        if (do_unappend(ent))
        {
            if (strrchr(path, '/') != NULL)
                *strrchr(path, '/') = '\0';
            else
                *path = '\0';
        }
    }
    close(destroot.fd);
    free(destroot.path);
    free(path);
    free(paths);
    free(sanitized);
    return r;
}


/* Private Impl ***************************************************************/


int initdestandpaths(file_t *dest, char *path, const char **destpath,
        const char **dapath, const char *newpath, size_t src_count)
{
    int fd;
    char *real;
    char *tmp;
    char *delim;

    /* newpath is a directory that exists */
    if ((fd = open(newpath, O_RDONLY | O_DIRECTORY)) != -1)
    {
        tmp = strdup(newpath);
        REMOVE_TRAILING_SLASHES(tmp);
        dest->fd = fd;
        dest->path = tmp;
        strcpy(path, "");
        *destpath = path + 1;
        *dapath = path;
        return 0;
    }

    /* if dest does not exist open its parent directory */
    if (errno == ENOENT)
    {
        if (src_count > 1)
        {
            log_errorx("target `%s' is not a directory", newpath);
            return -1;
        }

        tmp = strdup(newpath);
        REMOVE_TRAILING_SLASHES(tmp);

        /* if no slash specified assume cwd */
        if ((delim = strrchr(tmp, '/')) == NULL)
        {
            dest->fd = open(".", O_RDONLY | O_DIRECTORY);
            dest->path = calloc(sizeof(char), 1);
            strcpy(path, tmp);
            *destpath = path;
            *dapath = path + strlen(path);
            free(tmp);
            return 0;
        }

        /* our path begins with the new filename */
        strcpy(path, delim + 1);
        *destpath = path;
        *dapath = path + strlen(path);

        *delim = '\0';
        if ((fd = open(tmp, O_RDONLY | O_DIRECTORY)) == -1)
        {
            log_error("cannot open target parent `%s'", tmp);
            free(tmp);
            return -1;
        }

        dest->fd = fd;
        dest->path = tmp;
        return 0;
    }

    /*
     * newpath exists and is not a directory
     *     1. make sure we have a single source target
     *     2. make sure source is not a directory
     */
    if (errno == ENOTDIR)
    {
        if (src_count > 1)
        {
            log_errorx("target `%s' is not a directory", newpath);
            return -1;
        }

        /* cwd */
        if(strrchr(newpath, '/') == NULL)
        {
            dest->fd = open(".", O_RDONLY | O_DIRECTORY);
            dest->path = calloc(sizeof(char), 1);
            strcpy(path, newpath);
            *destpath = path;
            *dapath = path + strlen(path);
            return 0;
        }

        /* open newpath's parent directory */
        if ((real = realpath(newpath, NULL)) == NULL)
        {
            log_error("cannot resolve target `%s'", newpath);
            return -1;
        }

        /* if realpath worked it better have a '/' character! */
        tmp = strrchr(real, '/');
        strcpy(path, tmp + 1);
        REMOVE_TRAILING_SLASHES(path);
        *destpath = path;
        *dapath = path + strlen(path);

        *tmp = '\0';
        if ((fd = open(real, O_RDONLY | O_DIRECTORY)) == -1)
        {
            log_error("cannot open target parent `%s'", real);
            free(real);
            return -1;
        }

        dest->fd = fd;
        dest->path = real;
        return 0;
    }

    /* if we get here there was a problem opening up newpath log and fail */
    log_error("cannot open '%s'", newpath);
    return -1;
}


int do_append(FTSENT *ent, const file_t *destroot, const char *newpath)
{
    /* if newpath is not the destroot then we are renaming so don't append at
     * root level */
    if (ent->fts_level == 0 && strcmp(destroot->path, newpath) != 0)
        return 0;

    /* no append for directory postorder */
    return ent->fts_info != FTS_DP;
}


int do_unappend(FTSENT *ent)
{
    /* no unappend for dir preorder */
    return ent->fts_info != FTS_D;
}


void process(file_t *newdir, const char *newpath, FTSENT *ent,
        const char *dapath, const void *pathmd5, struct process_opts *popts,
        int verbose)
{
    dcp_state_t state;
    switch (ent->fts_info)
    {

    case FTS_D:                                 /* PREORDER DIRECTORY     */
    {
        if (preprocess(newdir,newpath,ent->fts_path,ent->fts_statp,verbose)!=0)
            break;

        /* directory existing is not an error */
        state = DCP_DIR_CREATED;
        if (mkdirat(newdir->fd, newpath, 0777) != 0 && errno != EEXIST)
        {
            log_error("cannot create dir '%s/%s'", newdir->path, newpath);
            state  = DCP_DIR_FAILED;
        }

        popts->callback(state, pathmd5, dapath, ent->fts_statp,
                ent->fts_accpath, NULL, NULL, NULL, NULL, NULL, -1,
                popts->callback_ctx);
        break;
    }

    case FTS_DP:                                /* POSTORDER DIRECTORY    */
    {
        process_directory(newdir, newpath, ent->fts_accpath, ent->fts_statp,
                dapath, pathmd5, popts);
        break;
    }

    case FTS_F:                                 /* REGULAR FILE           */
    {
        if (preprocess(newdir,newpath,ent->fts_path,ent->fts_statp,verbose)!=0)
            break;
        process_regular(newdir, newpath, ent->fts_accpath, ent->fts_statp,
                dapath, pathmd5, popts);
        break;
    }

    case FTS_SL:                                /* SYMLINK                */
    {
        if (preprocess(newdir,newpath,ent->fts_path,ent->fts_statp,verbose)!=0)
            break;
        process_symlink(newdir, newpath, ent->fts_accpath, ent->fts_statp,
                dapath, pathmd5, popts);
        break;
    }

    case FTS_DEFAULT:                           /* SPECIAL TYPES          */
    {
        if (preprocess(newdir,newpath,ent->fts_path,ent->fts_statp,verbose)!=0)
            break;
        process_special(newdir, newpath, ent->fts_accpath, ent->fts_statp,
                dapath, pathmd5, popts);
        break;
    }

                                                /* Errors                 */
    case FTS_ERR:
    {
        popts->callback(DCP_FAILED, pathmd5, dapath, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, -1, popts->callback_ctx);
        errno = ent->fts_errno;
        log_error("fts_read '%s'", ent->fts_path);
        break;
    }

    case FTS_NS:
    {
        popts->callback(DCP_FAILED, pathmd5, dapath, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, -1, popts->callback_ctx);
        errno = ent->fts_errno;
        log_error("cannot stat '%s'", ent->fts_path);
        break;
    }

    case FTS_DNR:
    {
        popts->callback(DCP_FAILED, pathmd5, dapath, NULL, NULL, NULL, NULL,
                NULL, NULL, NULL, -1, popts->callback_ctx);
        errno = ent->fts_errno;
        log_error("cannot read dir '%s'", ent->fts_path);
        break;
    }

    /*
     * the following values should not be seen
     *      FTS_NSOK    a file which no stat information was requested
     *      FTS_DOT     . and .. only show up if a flag is set
     *      FTS_DC      directory cycle <- can this happen with hard links?
     *      FTS_SLNONE  dangling symlink since fts type is physical this
     *                  doesn't get returned
     */
    case FTS_NSOK:    log_error("FTS_NSOC '%s'",   ent->fts_path); break;
    case FTS_DOT:     log_error("FTS_DOT '%s'",    ent->fts_path); break;
    case FTS_DC:      log_error("FTS_DC '%s'",     ent->fts_path); break;
    case FTS_SLNONE:  log_error("FTS_SLNONE '%s'", ent->fts_path); break;
    default:          log_error("unknown fts entry type %d for '%s'",
                                ent->fts_info, ent->fts_path);
    }
}
