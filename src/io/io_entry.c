/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of the io_entry.h interface. Entries in dcp input and output
 * are an entry in the filesystem that was copied/duplicated. For each dir,
 * file, symbolic link we log information about each as a single line json
 * object.
 */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>
#include <linux/limits.h>

#include <jansson.h>

#include "io_entry.h"
#include "../entry.h"
#include "../digest.h"
#include "pack.h"
#include "../logging.h"


/* MACROS *********************************************************************/


/**
 * ensure the ascii/utf8 bytes are valid hex characters [0-9a-fA-F]
 *
 * @param c     character to asses if it is a valid hex character
 *
 * @return      0 if no
 */
#define IS_VALID_HEX_CHAR(c) ((c>47 && c<58)||(c>64 && c<71)||(c>96 && c<103))


#define LOG_ERROR(_line, _field, _msg)                                         \
    do {                                                                       \
        log_errorx("invalid '%s' on line %zu: %s", (_field), (_line), (_msg)); \
    } while (0)


#define LOG_NONINT(_line, _field) LOG_ERROR(_line, _field, "non integer")


#define LOG_NONHEX(_line, _field) \
        LOG_ERROR(_line, _field, "failed to parse hex string")


/* Private API ****************************************************************/


/**
 * given a json_t *, ensure it is a string and pack into dest if it won't
 * overflow dest.
 *
 * @param dest      where to store the decoded bytes
 * @param dlen      max size of the buffer
 * @param src       json_t * fails if not string
 * @param line      line number in file for logging errors
 *
 * @return          number of bytes packed in dest, 0 if string was empty or
 *                  NULL. < 0 on error
 */
static ssize_t decode_hexjsonstr(void *dest, size_t dlen, const json_t *src,
        size_t line);


/**
 * Digests are stored as hex json strings. Pack the hex into a byte array.
 *
 * @param dest              where to store the bytes
 * @param digest_length     expected number of bytes in digest, also min len
 *                          of dest
 * @param str               json string instance
 * @param line              line number in stream, for debugging and log output
 * @param name              name of the digest to log for debugging and errors
 *
 * @return                  0 if digest was packed with the appropriate number
 *                          of bytes. 1 if digest was empty, no bytes packed.
 */
static int pack_digest(void *dest, size_t digest_length, const json_t *str,
        size_t line, const char *name);


/* Public Impl ****************************************************************/


int io_entry_read(entry_t *entry, FILE *in, size_t *line)
{
    char *buf;
    size_t blen;
    json_t *obj;
    json_error_t jerr;
    void *it;
    const char *key;
    const json_t *val;

    int has_pathmd5;

    /* read the next line skipping any metadata lines */
    buf = NULL;
    do {
        if (getline(&buf, &blen, in) < 0)
        {
            if (buf != NULL)    free(buf);
            if (ferror(in))     log_error("getline");
            return -1; /* returns -1 on EOF and error */
        }
        (*line)++;
    } while (buf[0] == '#');


    /* for jansson's documentation, JSON_REJECT_DUPLICATES issues an error when
     * multiple keys in an object have the same name instead of default
     * behavior which is to use the last defined value */
    if ((obj = json_loads(buf, JSON_REJECT_DUPLICATES, &jerr)) == NULL)
    {
        log_errorx("cannot parse json line %zd: %s'", *line, jerr.text);
        return -1;
    }

    free(buf);

    memset(entry, 0, sizeof(*entry)); /* 0/NULL out every thing in the struct */

    for (   it = json_object_iter(obj);
            it != NULL ;
            it = json_object_iter_next(obj, it))
    {
        key = json_object_iter_key(it);
        val = json_object_iter_value(it);

        if (strcmp(key, "md5") == 0)
        {
            if (pack_digest(entry->_digest_bytes.md5, MD5_DIGEST_LENGTH, val,
                    *line, "md5") == -1)
            {
                LOG_NONHEX(*line, "md5");
                json_decref(obj);
                return -1;
            }
            entry->md5 = entry->_digest_bytes.md5;
        }

        else if (strcmp(key, "sha1") == 0)
        {
            if (pack_digest(entry->_digest_bytes.sha1, SHA_DIGEST_LENGTH, val,
                    *line, "sha1") == -1)
            {
                LOG_NONHEX(*line, "sha1");
                json_decref(obj);
                return -1;
            }
            entry->sha1 = entry->_digest_bytes.sha1;
        }

        else if (strcmp(key, "sha256") == 0)
        {
            if (pack_digest(entry->_digest_bytes.sha256, SHA256_DIGEST_LENGTH,
                    val, *line, "sha256") == -1)
            {
                LOG_NONHEX(*line, "sha256");
                json_decref(obj);
                return -1;
            }
            entry->sha256 = entry->_digest_bytes.sha256;
        }

        else if (strcmp(key, "sha512") == 0)
        {
            if (pack_digest(entry->_digest_bytes.sha512, SHA512_DIGEST_LENGTH,
                    val, *line, "sha512") == -1)
            {
                LOG_NONHEX(*line, "sha512");
                json_decref(obj);
                return -1;
            }
            entry->sha512 = entry->_digest_bytes.sha512;
        }

        else if (strcmp(key, "pathmd5") == 0)
        {
            if (pack_digest(entry->pathmd5, MD5_DIGEST_LENGTH, val, *line,
                    "pathmd5") == -1)
            {
                LOG_NONHEX(*line, "pathmd5");
                json_decref(obj);
                return -1;
            }
            has_pathmd5 = 1;
        }

        else if (strcmp(key, "mode") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "mode");
                json_decref(obj);
                return -1;
            }
            entry->mode = json_integer_value(val);
        }

        else if (strcmp(key, "size") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "size");
                json_decref(obj);
                return -1;
            }
            entry->size = json_integer_value(val);
        }

        else if (strcmp(key, "asec") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "asec");
                json_decref(obj);
                return -1;
            }
            entry->atime.tv_sec = json_integer_value(val);
        }

        else if (strcmp(key, "ansec") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "ansec");
                json_decref(obj);
                return -1;
            }
            entry->atime.tv_nsec = json_integer_value(val);
        }

        else if (strcmp(key, "msec") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "msec");
                json_decref(obj);
                return -1;
            }
            entry->mtime.tv_sec = json_integer_value(val);
        }

        else if (strcmp(key, "mnsec") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "mnsec");
                json_decref(obj);
                return -1;
            }
            entry->mtime.tv_nsec = json_integer_value(val);
        }

        else if (strcmp(key, "csec") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "csec");
                json_decref(obj);
                return -1;
            }
            entry->ctime.tv_sec = json_integer_value(val);
        }

        else if (strcmp(key, "cnsec") == 0)
        {
            if (!json_is_integer(val))
            {
                LOG_NONINT(*line, "cnsec");
                json_decref(obj);
                return -1;
            }
            entry->ctime.tv_nsec = json_integer_value(val);
        }

        /* fields we will ignore */
        /* ignoring since bdb will copy the entry struct but not manage the
         * dynamically allocated memory the structs point to */
        else if (strcmp(key, "path")     == 0) {}
        else if (strcmp(key, "state")    == 0) {}
        else if (strcmp(key, "uid")      == 0) {}
        else if (strcmp(key, "gid")      == 0) {}
        else if (strcmp(key, "type")     == 0) {}
        else if (strcmp(key, "elapsed")  == 0) {}
        else if (strcmp(key, "pathhex")  == 0) {}

        else
            log_warnx("ignoring unknown key '%s' on line %zu", key, *line);
    }

    if (!has_pathmd5)
    {
        log_errorx("'pathmd5' missing on line: %zu", *line);
        json_decref(obj);
        return -1;
    }

    return 0;
}


/*
 * while jansson can be used for creating a json structure then printing it
 * out, there is a large overhead cost. Since we control the data only use
 * jansson to escape the strings we cannot control.
 */
int io_entry_write_fields(const char *state, const char *path,
        const struct stat *st, const void *pathmd5, const char *symlinkpath,
        const void *md5, const void *sha1, const void *sha256,
        const void *sha512, long elapsed, FILE *stream)
{
    enum { MAX_LENGTH = PATH_MAX * 4 };

    int ret;
    size_t len;

    /* use jansson for string character escaping of paths */
    json_t *escaped;

    /*
     * used to hold hex strings for the hashes and the path if it contains
     * non utf-8 characters
     */
    char buf[MAX_LENGTH];

    ret = 0;

    /* we are writing a json object on the line */
    fputc('{', stream);

    if (md5 != NULL)
    {
        unpack(buf, md5, MD5_DIGEST_LENGTH);
        fprintf(stream, "\"md5\":\"%s\",", buf);
    }

    if (sha1 != NULL)
    {
        unpack(buf, sha1, SHA_DIGEST_LENGTH);
        fprintf(stream, "\"sha1\":\"%s\",", buf);
    }

    if (sha256 != NULL)
    {
        unpack(buf, sha256, SHA256_DIGEST_LENGTH);
        fprintf(stream, "\"sha256\":\"%s\",", buf);
    }

    if (sha512 != NULL)
    {
        unpack(buf, sha512, SHA512_DIGEST_LENGTH);
        fprintf(stream, "\"sha512\":\"%s\",", buf);
    }

    /*
     * up till this point we have been appending commas to the end of the
     * entries, this is because we don't know which hashes are going to be
     * provided. Now that pathmd5 has been put on the stream we switch to
     * commas before any new entries, allowing errors to simply exit and not
     * output malformed json.
     */

    unpack(buf, pathmd5, MD5_DIGEST_LENGTH);
    fprintf(stream, "\"pathmd5\":\"%s\"", buf); /* no commas before or after */

    if (st != NULL)
    {
        /* mode, size and timestamps */
        fprintf(stream,
                ",\"uid\":%u,\"gid\":%u,"
                "\"mode\":%u,\"size\":%jd,"
                "\"asec\":%ld,\"ansec\":%ld,"
                "\"msec\":%ld,\"mnsec\":%ld,"
                "\"csec\":%ld,\"cnsec\":%ld",
                st->st_uid, st->st_gid,
                st->st_mode, (intmax_t) st->st_size,
                st->st_atim.tv_sec, st->st_atim.tv_nsec,
                st->st_mtim.tv_sec, st->st_mtim.tv_nsec,
                st->st_ctim.tv_sec, st->st_ctim.tv_nsec);

        /* extract what type of file from the mode */
        fprintf(stream, ",\"type\":\"%s\"",
            S_ISREG(st->st_mode)?  "reg"  : S_ISDIR(st->st_mode)?  "dir"  :
            S_ISLNK(st->st_mode)?  "lnk"  : S_ISCHR(st->st_mode)?  "chr"  :
            S_ISBLK(st->st_mode)?  "blk"  : S_ISFIFO(st->st_mode)? "fifo" :
            S_ISSOCK(st->st_mode)? "sock" : "unkn");
    }

    /* state string */
    if ((escaped = json_string(state)) == NULL)
    {
        log_errorx("non valid utf-8 state string '%s'", state);
        ret = -1;
        goto cleanup;
    }
    fputs(",\"state\":", stream);
    json_dumpf(escaped, stream, JSON_ENCODE_ANY);
    json_decref(escaped);

    /* # of secs elapsed while processing */
    if (elapsed > -1)
        fprintf(stream, ",\"elapsed\":%ld", elapsed);

    /* if entry is a symlink we include what its target path was */
	if (symlinkpath != NULL)
    {
        /*
         * symlinkTarget    valid utf-8 JSON escaped string where pointing
         * symlinkTargetHex hex encoding of the target if jansson cannot encode
         */
        if ((escaped = json_string(symlinkpath)) != NULL)
        {
            fputs(",\"symlinkTarget\":", stream);
            json_dumpf(escaped, stream, JSON_ENCODE_ANY);
            json_decref(escaped);
        }
        else /* jansson was unable to handle the string */
        {
            len = strlen(symlinkpath);
            if ((len * 2 + 1) > MAX_LENGTH)
            {
                log_errorx("buffer too small, expected string with length < %d,"
                        " for non valid utf-8 string: '%s'", MAX_LENGTH,
                        symlinkpath);
                ret = -1;
                goto cleanup;
            }
            unpack(buf, symlinkpath, len);
            fprintf(stream, ",\"symlinkTargetHex\":\"%s\"", buf);
        }
    }

    /*
     * path     valid utf-8 JSON escaped path to the file
     * pathHex  hex encoding of the path, provided if jansson cannot encode
     */
    if ((escaped = json_string(path)) != NULL)
    {
        fputs(",\"path\":", stream);
        json_dumpf(escaped, stream, JSON_ENCODE_ANY);
        json_decref(escaped);
    }
    else /* jansson was unable to handle the string */
    {
        len = strlen(path);
        if ((len * 2 + 1) > MAX_LENGTH)
        {
            log_errorx("buffer too small, expected string with length < %d, "
                    "for non valid utf-8 path string: '%s'", MAX_LENGTH, path);
            ret = -1;
            goto cleanup;
        }
        unpack(buf, path, len);
        fprintf(stream, ",\"pathhex\":\"%s\"", buf);
    }


cleanup:
    /* print a newline our record separator */
    fputs("}\n", stream);
    return ret;
}


/* Private Impl ***************************************************************/


inline int pack_digest(void *dest, size_t digest_length, const json_t *str,
        size_t line, const char *name)
{
    ssize_t count;
    count = decode_hexjsonstr(dest, digest_length, str, line);

    /* if digest is empty act as if it was not present in the object */
    if (count == 0)
        return 1;

    else if (count == (ssize_t) digest_length)
        return 0;

    else
    {
        log_errorx("corrupt input, %s invalid length on line %zu", name, line);
        return -1;
    }
}


inline ssize_t decode_hexjsonstr(void *dest, size_t dlen, const json_t *src,
        size_t line)
{
    const char *hex;
    size_t len;

    if (!json_is_string(src))
    {
        log_errorx("corrupt input, non hex string for md5 on line %zu", line);
        return -1;
    }

    hex = json_string_value(src);
    len = strlen(hex);

    /* if empty string or NULL treat as if it doesn't exist */
    if (hex == NULL || len == 0)
        return 0;

    if (len % 2 != 0)
    {
        log_errorx("corrupt input, odd length hex string on line %zu", line);
        return -1;
    }

    /* ensure buffer size is enough */
    if (len / 2 > dlen)
    {
        log_errorx("corrupt input, hex string length greater than expected on "
                "line %zu", line);
        return -1;
    }

    if (pack(dest, hex, line) != 0)
        return -1;

    return len / 2;
}
