/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of the digest.h API. Backed by OpenSSL we can calculate
 * different digests using a unified API.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/md5.h>
#include <openssl/sha.h>

#include "digest.h"
#include "logging.h"


/* Type Defs ******************************************************************/


/**
 * To simplify management of the different digests this pseudo object will
 * keep track of the digest contexts and the needed functions to update the
 * digest.
 */
struct digest {
    int (*update)(void *c, const void *data, size_t len);  /* openssl func    */
    int (*final)(unsigned char *md, void *c);              /* openssl func    */
    union {                                                /* space for ctxs  */
        MD5_CTX md5;
        SHA_CTX sha1;
        SHA256_CTX sha256;
        SHA512_CTX sha512;
    } ctx;
    int finalized;                                         /* bytes valid?    */
    size_t length;                                         /* digest length   */
    unsigned char bytes[MAX_DIGEST_LENGTH];                /* space for value */
};


/* Public Impl ****************************************************************/


int digest(digest_t type, void *dest, const void *bytes, size_t len)
{
    struct digest *d;
    assert(bytes != NULL);
    d = digest_create(type);
    digest_update(d, bytes, len);
    digest_finalize(d);
    digest_copy_value(d, dest);
    digest_free(d);
    return 0;
}


int digest_fd(digest_t type, void *dest, int fd)
{
    struct digest *d;
    unsigned char *buf;
    ssize_t count;

    if ((buf = malloc(32768)) == NULL)
        return -1;

    d = digest_create(type);
    do {
        count = read(fd, buf, 32768);
        if (count < 0)
        {
            log_debug("read");
            digest_free(d);
            free(buf);
            return -1;
        }
        digest_update(d, buf, count);
    } while (count > 0);
    digest_finalize(d);
    digest_copy_value(d, dest);
    digest_free(d);
    return 0;
}


digester_t *digest_create(digest_t type)
{
    assert(type == DGST_MD5 || type == DGST_SHA1 || type == DGST_SHA256 ||
            type == DGST_SHA512);

    switch (type)
    {
    case DGST_MD5:    return digest_create_md5();
    case DGST_SHA1:   return digester_create_sha1();
    case DGST_SHA256: return digest_create_sha256();
    case DGST_SHA512: return digest_create_sha512();
    default:
        log_debugx("invalid digest type %d", type);
        return NULL;
    }
}


digester_t *digest_create_md5(void)
{
    struct digest *digest;

    digest = malloc(sizeof(*digest));

    digest->length = MD5_DIGEST_LENGTH;
    digest->update = (int (*)(void *, const void *, size_t)) &MD5_Update;
    digest->final = (int (*)(unsigned char *md, void *c)) &MD5_Final;
    MD5_Init((MD5_CTX *) &digest->ctx);
    digest->finalized = 0;
    return digest;
}


digester_t *digester_create_sha1(void)
{
    struct digest *digest;

    digest = malloc(sizeof(*digest));

    digest->length = SHA_DIGEST_LENGTH;
    digest->update = (int (*)(void *, const void *, size_t)) &SHA1_Update;
    digest->final = (int (*)(unsigned char *md, void *c)) &SHA1_Final;
    SHA1_Init((SHA_CTX *) &digest->ctx);
    digest->finalized = 0;
    return digest;
}


digester_t *digest_create_sha256(void)
{
    struct digest *digest;

    digest = malloc(sizeof(*digest));

    digest->length = SHA256_DIGEST_LENGTH;
    digest->update = (int (*)(void *, const void *, size_t)) &SHA256_Update;
    digest->final = (int (*)(unsigned char *md, void *c)) &SHA256_Final;
    SHA256_Init((SHA256_CTX *) &digest->ctx);
    digest->finalized = 0;
    return digest;
}


digester_t *digest_create_sha512(void)
{
    struct digest *digest;

    digest = malloc(sizeof(*digest));

    digest->length = SHA512_DIGEST_LENGTH;
    digest->update = (int (*)(void *, const void *, size_t)) &SHA512_Update;
    digest->final = (int (*)(unsigned char *md, void *c)) &SHA512_Final;
    SHA512_Init((SHA512_CTX *) &digest->ctx);
    digest->finalized = 0;
    return digest;
}


int digest_finalize(digester_t *digest)
{
    if (digest != NULL)
    {
        digest->final(digest->bytes, &digest->ctx);
        digest->finalized = 1;
    }
    return 0;
}


void digest_free(digester_t *digest)
{
    if (digest != NULL)
        free(digest);
}


size_t digest_get_length(const digester_t *digest)
{
    return digest->length;
}


const char *digest_name(digest_t digest)
{
    switch (digest)
    {
    case DGST_MD5:      return "md5";
    case DGST_SHA1:     return "sha1";
    case DGST_SHA256:   return "sha256";
    case DGST_SHA512:   return "sha512";
    default:            return NULL;
    }
}


int digest_copy_value(const digester_t *digest, void *bytes)
{
    if (bytes != NULL && digest->finalized)
        memcpy(bytes, digest->bytes, digest->length);
    return 0;
}


const void *digest_get_value(const digester_t *digest)
{
    if (digest != NULL && digest->finalized)
        return digest->bytes;
    return NULL;
}


int digest_is_finalized(const digester_t *digest)
{
    return digest != NULL && digest->finalized;
}


int digest_update(digester_t *digest, const void *bytes, size_t count)
{
    if (digest != NULL && bytes != NULL && count > 0)
        digest->update(&digest->ctx, bytes, count);
    return 0;
}


int digesterset_create(digesterset_t *set, int mask)
{
    /* NULL out the digestset */
    memset(set, 0, sizeof(*set));
    if (HAS_MD5(mask))      set->md5    = digest_create_md5();
    if (HAS_SHA1(mask))     set->sha1   = digester_create_sha1();
    if (HAS_SHA256(mask))   set->sha256 = digest_create_sha256();
    if (HAS_SHA512(mask))   set->sha512 = digest_create_sha512();
    return 0;
}


int digesterset_update(digesterset_t *set, const void *bytes, size_t count)
{
    digest_update(set->md5,     bytes, count);
    digest_update(set->sha1,    bytes, count);
    digest_update(set->sha256,  bytes, count);
    digest_update(set->sha512,  bytes, count);
    return 0;
}


int digesterset_finalize(digesterset_t *set)
{
    digest_finalize(set->md5);
    digest_finalize(set->sha1);
    digest_finalize(set->sha256);
    digest_finalize(set->sha512);
    return 0;
}


int digesterset_free(digesterset_t *set)
{
    digest_free(set->md5);
    digest_free(set->sha1);
    digest_free(set->sha256);
    digest_free(set->sha512);
    return 0;
}


const void *digesterset_get_value(digesterset_t *set, digest_t type)
{
    switch (type)
    {
    case DGST_MD5: return digest_get_value(set->md5);
    case DGST_SHA1: return digest_get_value(set->sha1);
    case DGST_SHA256: return digest_get_value(set->sha256);
    case DGST_SHA512: return digest_get_value(set->sha512);
    default:
        return NULL;
    }
}
