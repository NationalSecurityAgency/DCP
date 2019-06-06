/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Defines a generic Digest API to perform MD5, SHA1, SHA256 and SHA512
 * hashing algorithms.
 *
 * This code wraps the openssl implementations of the above algorithms and
 * provides a unified api for creating these digests.
 */
#ifndef DIGEST_H__
#define DIGEST_H__


#include <stddef.h>
#include <openssl/md5.h>
#include <openssl/sha.h>


/* MACROS *********************************************************************/


/**
 * Checks if the mask has the md5 flag
 */
#define HAS_MD5(_d)     ((_d) & DGST_MD5)


/**
 * Checks if the mask has the sha1 flag
 */
#define HAS_SHA1(_d)    ((_d) & DGST_SHA1)


/**
 * Checks if the mask has the sha1 flag
 */
#define HAS_SHA256(_d)  ((_d) & DGST_SHA256)


/**
 * Checks if the mask has the sha512 flag
 */
#define HAS_SHA512(_d)  ((_d) & DGST_SHA512)


/**
 * Mask of all digests
 */
#define DGST_ALL (DGST_MD5 | DGST_SHA1 | DGST_SHA256 | DGST_SHA512)


#define DIGEST_LENGTH(_type)                                                   \
    ( (_type) == DGST_MD5? MD5_DIGEST_LENGTH :                                 \
            (_type) == DGST_SHA1? SHA_DIGEST_LENGTH :                          \
                    (_type) == DGST_SHA256? SHA256_DIGEST_LENGTH :             \
                            (_type) == DGST_SHA512? SHA512_DIGEST_LENGTH : 0   \
    )


#define MAX_DIGEST_LENGTH SHA512_DIGEST_LENGTH


/* Type Defs ******************************************************************/


/**
 * Flags to use to specify what type of digest to create. Or'ing of these is
 * also supported.
 *
 * Example: int digests = DGST_MD5 | DGST_SHA256;
 */
typedef enum {
    DGST_MD5    = 1, /**< MD5 128 bit Checksum*/
    DGST_SHA1   = 2, /**< SHA1 160 bit Checksum */
    DGST_SHA256 = 4, /**< SHA256 256 bit Checksum */
    DGST_SHA512 = 8  /**< SHA512 512 bit Checksum */
} digest_t;


/**
 * Digest type which this API uses.
 */
typedef struct digest digester_t;


/**
 * Struct to simplify the juggling of multiple digesters when some can be
 * invalid.
 */
typedef struct {
    int valid;              /**< mask of the digest_alg_t's that we use */
    digester_t *md5;        /**< place to store the md5 digester */
    digester_t *sha1;       /**< place to store the sha1 digester */
    digester_t *sha256;     /**< place to store the sha256 digester */
    digester_t *sha512;     /**< place to store the sha512 digester */
} digesterset_t;


int digesterset_create(digesterset_t *set, int mask);
int digesterset_update(digesterset_t *set, const void *bytes, size_t count);
int digesterset_finalize(digesterset_t *set);
const void *digesterset_get_value(digesterset_t *set, digest_t alg);
int digesterset_free(digesterset_t *set);


/* Public API *****************************************************************/


/**
 * Convenience function, looks at the type and instantiates the proper digest
 *      digest_create(DGST_MD5) == digest_md5()
 *
 * @param type      What type of digest to create
 *
 * @return          newly created digest_t to stream bytes to
 */
digester_t *digest_create(digest_t alg);


/**
 * Create a new digest object which uses the md5 algorithm.
 *
 * @return          digest_t that is waiting for bytes to update
 */
digester_t *digest_create_md5(void);


/**
 * Create a new digest object which uses the sha1 algorithm.
 *
 * @return          digest_t that is waiting for bytes to update
 */
digester_t *digester_create_sha1(void);


/**
 * Create a new digest object which uses the sha256 algorithm.
 *
 * @return          digest_t that is waiting for bytes to update
 */
digester_t *digest_create_sha256(void);


/**
 * Create a new digest object which uses the sha512 algorithm.
 *
 * @return          digest_t that is waiting for bytes to update
 */
digester_t *digest_create_sha512(void);


/**
 * Copy the digest bytes to the given buffer. Buffer must point to an
 * area of memory with at least digest_get_length() space.
 *
 * @param digest        finalized digest to copy bytes from
 * @param bytes         buffer to write bytes to
 *
 * @return          0 on success
 */
int digest_copy_value(const digester_t *digester, void *bytes);


/**
 * Once the digest has been updated with all the bytes, we must finalize the
 * digest which finishes the calculation. This function must be called before
 * using the digest_copy_value() and digest_get_value() functions.
 *
 * @param digest    the digest to finalize
 *
 * @return          0 on success
 */
int digest_finalize(digester_t *digester);


/**
 * Reclaim all resources dedicated to this digest.
 *
 * @param digest    the digest to deallocate resources
 *
 * @return          0 on success
 */
void digest_free(digester_t *digester);


/**
 * Retrieves the number of bytes in the final calculated digest.
 *
 * @param digest    digest instance to ask for length from
 *
 * @return          The number of bytes the digesting algorithm outputs
 */
size_t digest_get_length(const digester_t *digester);


/**
 * Return a pointer to a buffer of length, igest_get_length(), holding the
 * raw bytes of the digest. Pointer is invalid if digest_cleanup is called on
 * the digest_t.
 *
 * @param digest    finalized digest to get a copy of its bytes
 *
 * @return          0 on success
 */
const void *digest_get_value(const digester_t *digester);


/**
 * Queries the digest to determine if digest_finalize() has been called on it.
 *
 * @param digest    digest to check if it has been finalized
 *
 * @return          0 if digest has not been finalized, non zero if it has
 */
int digest_is_finalized(const digester_t *digester);


/**
 * Update the current calculating digest with the given bytes. Invalid to call
 * this function after a digest has been finalized.
 *
 * @param digest    the digest to update
 * @param bytes     the bytes to use of length count
 * @param count     the number of bytes in bytes
 *
 * @return          0 on success
 */
int digest_update(digester_t *digester, const void *bytes, size_t count);


/**
 * calculate the digest `alg` for `len` bytes at `data` storing the
 * result in `dest`. `dest` is assumed to have enough space.
 *
 * @param alg       what digest to calculate
 * @param dest      where to store the resulting digest, must be large enough
 * @param data      what bytes to digest
 * @param len       number of bytes in data to digest
 *
 * @return          0 on success
 */
int digest(digest_t alg, void *dest, const void *data, size_t len);


/**
 * given a valid file descriptor to read from digest all bytes from fd till
 * EOF storing the resulting digest in dest.
 *
 * @param alg       what digest to calculate
 * @param dest      buffer to store the resulting digest, must be large enough
 * @param fd        file descriptor to read bytes from, must be at the right 
 *                  position
 * @return          0 on success
 */
int digest_fd(digest_t alg, void *dest, int fd);


const char *digest_name(digest_t digest);

#endif
