/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of the io_xattr.h interface. Entries in dcp input and output
 * are an entry in the filesystem that was copied/duplicated. For each
 * file and symbolic link we log xattr information about each as a single line
 * json object.
 */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include <jansson.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include "io_xattr.h"
#include "../digest.h"
#include "pack.h"
#include "../logging.h"


/* Public Impl ****************************************************************/


int io_entry_write_xattr_fields(const void *pathmd5, const char *name,
        void *valuebuffer, ssize_t valuesize, FILE *stream)
{
    int ret;
    int w;

    /* where to store the hex representation of the path md5 sum */
    char buf[(MD5_DIGEST_LENGTH * 2) + 1];

    /* we store the xattr value as base64 */
    BIO *bio, *b64;

    /* used to properly escape the utf-8 name of an xattr */
    json_t *escaped;

    ret = 0;
    b64 = NULL;

    /* we are writing a json object on the line */
    fputc('{', stream);

    unpack(buf, pathmd5, MD5_DIGEST_LENGTH);
    fprintf(stream, "\"pathmd5\":\"%s\",", buf);

    if (name != NULL)
    {
        if ((escaped = json_string(name)) == NULL)
        {
            log_errorx("non valid utf-8 xattr string '%s'", name);
            ret = -1;
            goto cleanup;
        }
        fputs("\"xattrName\":", stream);
        json_dumpf(escaped, stream, JSON_ENCODE_ANY);
        json_decref(escaped);
    }

    if (valuesize > 0)
    {
        fputs(",\"xattrValue\":\"", stream);
        /*
         * base64 encoding
         * https://www.openssl.org/docs/manmaster/crypto/BIO_f_base64.html
         */
        b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); /* do not append newline */
        bio = BIO_new_fp(stream, BIO_NOCLOSE);      /* do not close stream */
        BIO_push(b64, bio);
        if ((w = BIO_write(b64, valuebuffer, valuesize)) != valuesize)
        {
            log_errorx("cannot base64 encode xattr value for attr: '%s'", name);
            ret = -1;
        }
        BIO_flush(b64);
        BIO_free_all(b64);
        fputc('"', stream);
    }


cleanup:
    /* print a newline our record separator */
    fputs("}\n", stream);
    return ret;
}
