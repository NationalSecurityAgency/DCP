/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * todo write description for dcp.h
 *
 * DEFINITIONS
 *
 * todo clean this up it be crazy!
 * Destination Absolute Path or "Mount Relative Path" is a cumbersome name to
 * describe a path that looks like it's absolute but the root of the path is the
 * source or dest directory. For context think about cloning a drive we mount
 * the drives root to '/media/drive' and we copy to '/media/drive2' the
 * destination absolute path (aka DAPath) is '/file' where we can append the
 * source or destination mount points to the beginning and get the full path.
 */
#ifndef DCP_H__
#define DCP_H__


#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../index/index.h"


/* Type Defs ******************************************************************/


/**
 * To help log how the processing of each file system item these states can be
 * used to signify what action was completed. @see dcp_strstate to convert
 * each state to a descriptive c string.
 */
typedef enum {
    DCP_FAILED,           /**< failed to copy/link/add to destination */
    DCP_FILE_COPIED,      /**< successfully copied */
    DCP_DIR_CREATED,      /**< successfully created directory */
    DCP_SYMLINK_CREATED,  /**< successfully created a symlink */
    DCP_SPECIAL_CREATED,  /**< successfully created a fifo,blk,chr,sock dev */
    DCP_DIR_FAILED        /**< failed to create the directory */
} dcp_state_t;


/**
 * callback function for dcp to call once a file has finished being processed
 * The callback will be provided with the file's stat information, where the
 * file was copied from and to, and if it is a regular file the digests
 * calculated.
 */
typedef int (*dcp_callback_f)(dcp_state_t state, const void *pathmd5,
        const char *dapath, const struct stat *sstat, const char *accesspath,
        const char *symlinkpath, const void *md5,
        const void *sha1, const void *sha256, const void *sha512,
        unsigned long process_time, void *context);


/**
 * run options to tell dcp how to perform the copy
 */
struct dcp_options {
    size_t bufsize;     /**< amount of memory to set aside to cache files */
    uid_t uid;          /**< owner of copied files */
    gid_t gid;          /**< group of copied files */
    int digests;        /**< mask of @see digest_alg_t's specifying what hashes
                             to calc */
    index_t *index;     /**< if not NULL do not copy any file in the index */
    int verbose;        /**< should we output explanation of what is going on */
};


/* Public API *****************************************************************/


/**
 * TODO write description
 */
int dcp(const char *newpath, const char *src[], size_t srcc,
        struct dcp_options *opts, dcp_callback_f callback, void *ctx);

/**
 * provides a string representation of each of the possible states defined in
 * dcp_state_t.
 *
 * @param state     the state to get a string description of
 *
 * @return          c string representation of the state
 */
const char *dcp_strstate(dcp_state_t state);


#endif
