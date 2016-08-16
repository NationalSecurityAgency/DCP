/**
 * @file
 *
 * @version 1.0
 * @date Jun 26, 2014
 *
 * @section DESCRIPTION
 *
 * todo write description for process.h
 */

#ifndef PROCESS_H__
#define PROCESS_H__


#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../digest.h"
#include "../index/index.h"
#include "dcp.h"


/* Macros *********************************************************************/


/**
 * to simplify familiarization with the implementation I want to keep all the
 * signatures similar, therefore with -Wall -Wextra we might get some unused
 * warnings preventing compilation. Using the below macro allows us to tell the
 * compiler that we want the variable in the signature even though we have no
 * use for it.
 *
 * int func(int a, int b) { UNUSED(b); return a; }
 */
#define UNUSED(_var) (void) _var


/* Type Defs ******************************************************************/


typedef struct {
    int fd;
    char *path;
} file_t;


/**
 * struct to hold static parameters that the following functions utilize.
 */
struct process_opts {
    int digests;                /**< mask of digest_alg_t for what hashes to
                                     compute */
    uid_t uid;                  /**< who owns the copied files */
    gid_t gid;                  /**< what group do copied files belong */
    void *buffer;               /**< preallocated memory to use for reading */
    size_t buffer_size;         /**< # of bytes in `buffer` */

    index_t *index;             /**< NULL or files we should not copy */
    dcp_callback_f callback;    /**< callback to send processing info to */
    void *callback_ctx;         /**< provided pointer to send to `processor` */
};


/* Public API *****************************************************************/


/**
 * To process a special devices (FIFO, Block, Character and Socket) we simply
 * create the device maintaining the configuration options found in the stat
 * structure.
 *
 * If `newpath` is relative, then it is interpreted relative to the directory
 * referred to by `newdirfd` rather than the process's cwd. If `newpath` is
 * absolute `newdirfd` is ignored.
 *
 * If `newdirfd` is the special value `AT_FDCWD` then pathname is interpreted
 * relative to the current working directory of the calling process.
 *
 * @param newdirfd  open directory to use as root to resolve relative pathname's
 * @param newname   name of the new device to create
 * @param oldst     pointer to the stat structure for the original special dev
 * @param dapath    Directory Absolute Path @see dcp.h DEFINITIONS
 * @param pathmd5   md5sum of `dapath`
 * @param opts      pointer to the static parameters used by the process funcs
 *
 * @return          0 on success
 */
int process_special(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts);


/**
 * Process a new directory. Since dcp is written as a post-order or DFS style
 * walk we assume that the directory exists since for all its children to have
 * been processed already means the destination directory must exist.
 *
 * If `newpath` is relative, then it is interpreted relative to the directory
 * referred to by `newdirfd` rather than the process's cwd. If `newpath` is
 * absolute `newdirfd` is ignored.
 *
 * If `newdirfd` is the special value `AT_FDCWD` then pathname is interpreted
 * relative to the current working directory of the calling process.
 *
 * @return          0 on success, -1 on failure
 */
int process_directory(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts);


/**
 * Processes a regular file
 *      1. Deduplication    If `index` is not null then we won't copy unless
 *                          the file is new or modified.
 *      2. Digests          as specified by the `opts.digests` mask, we will
 *                          calculate all the required digests for the file.
 *      3. Cacheing         if `opts.buffer` is large enough we will only read
 *                          the file into memory once. Eliminating duplicate IO.
 *                          Even if buffer is too small we will use it over
 *                          and over to eliminate redundant allocations.
 *
 * If `newpath` is relative, then it is interpreted relative to the directory
 * referred to by `newdirfd` rather than the process's cwd. If `newpath` is
 * absolute `newdirfd` is ignored.
 *
 * If `oldpath` is relative, then it is interpreted relative to the directory
 * referred to by `olddirfd` rather than the process's cwd. If `oldpath` is
 * absolute `olddirfd` is ignored.
 *
 * If `newdirfd` or `olddirfd` are the special value `AT_FDCWD` then `newpath`
 * or `oldpath` are interpreted relative to the current working directory of the
 * calling process.
 *
 *
 * @return          0 on success, -1 on failure
 */
int process_regular(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts);


/**
 * Process a symlink. By copying its contents to a new symlink.
 *
 * If `newpath` is relative, then it is interpreted relative to the directory
 * referred to by `newdirfd` rather than the process's cwd. If `newpath` is
 * absolute `newdirfd` is ignored.
 *
 * If `oldpath` is relative, then it is interpreted relative to the directory
 * referred to by `olddirfd` rather than the process's cwd. If `oldpath` is
 * absolute `olddirfd` is ignored.
 *
 * If `newdirfd` or `olddirfd` are the special value `AT_FDCWD` then `newpath`
 * or `oldpath` are interpreted relative to the current working directory of the
 * calling process.
 *
 *
 * @return          0 on success, -1 on failure
 */
int process_symlink(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, const char *dapath, const void *pathmd5,
        const struct process_opts *opts);


/**
 * Performs two tasks. First if there is an existing entry in the copy
 * destination then it is unlinked if possible and secondly if the verbose flag
 * was set will output the required messages.
 *
 */
int preprocess(file_t *newdir, const char *newpath, const char *oldpath,
        const struct stat *oldst, int verbose);


/**
 * not threadsafe. builds the path in a static buffer returning a pointer to it.
 * Later calls to this function will overwrite returned string.
 */
const char *pathstr(const file_t *root, const char *path);


#endif
