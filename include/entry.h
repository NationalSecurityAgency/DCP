/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Defines the entry_t type structure for storing information on each file
 * being copied.
 *
 * An entry is the collection of data we keep on each file that is copied. The
 * entry_t type is specified here to be used for the index.h and flatfile.h
 * APIs. All the data for a file is kept in the structure, there are no
 * pointers, this means that then entry_t does not have context to know which
 * of its fields are valid and which are invalid. Ex when creating just a md5
 * and sha1 digest the entry_t doesn't know that the sha256 and sha512 fields
 * are invalid bytes.
 */
#ifndef ENTRY_H_
#define ENTRY_H_


#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <linux/limits.h>

#include "digest.h"


/* Type Defs ******************************************************************/


/**
 * stores all the information used by the index API.
 */
typedef struct {
    uint8_t pathmd5[MD5_DIGEST_LENGTH];     /**< MD5 of relative path       */

    /* file digests */
    /* the following pointers are used to point to the private _digest_bytes
     * structure. This allows md5,sha1... to be NULL signifying that we don't
     * have this digest */
    uint8_t *md5;
    uint8_t *sha1;
    uint8_t *sha256;
    uint8_t *sha512;

    /* This struct is to store the bytes pointed to by the above pointers, only
     * code that creates entry_t structures should use this. Reading or checking
     * existence should be done with the above pointers. */
    struct {
        uint8_t md5[MD5_DIGEST_LENGTH];         /**< space for MD5          */
        uint8_t sha1[SHA_DIGEST_LENGTH];        /**< space for SHA1         */
        uint8_t sha256[SHA256_DIGEST_LENGTH];   /**< space for SHA256       */
        uint8_t sha512[SHA512_DIGEST_LENGTH];   /**< space for SHA512       */
    } _digest_bytes;    /**< private struct to store the digests */

    /* file attributes */
    uid_t uid;                              /**< owner's id                 */
    gid_t gid;                              /**< group's id                 */
    mode_t mode;                            /**< src's mode, perms and type */
    off_t size;                             /**< src's size                 */
    struct timespec atime;                  /**< src's access time          */
    struct timespec mtime;                  /**< src's modification time    */
    struct timespec ctime;                  /**< src's ino change time      */

} entry_t;


#endif
