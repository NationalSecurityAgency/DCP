#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "io_index.h"
#include "io_entry.h"
#include "digest.h"
#include "index.h"
#include "logging.h"


/* Private API ****************************************************************/


/**
 * looking at what digests are valid return a mask of digest_t's that are
 * specified in the entry
 *
 * @param entry     what entry to look at
 *
 * @return          a mask of all valid digests @see digest_t
 */
static int valid_digests(const entry_t *entry);


/**
 * Add an entry to the index if it isn't already there. When the entry exists
 * we log that we are skipping the entry due to it being a duplicate.
 *
 * @param idx       the index to insert the entry into
 * @param pathmd5   what path we want to index
 * @param digest    what digest we want to index, must be the same as the type
 *                  specified at index creation
 * @param file      what file was this entry in
 * @param linenum   what line in file was this entry
 */
static void add_or_warn(index_t *idx, const void *pathmd5, const void *digest,
        const char *file, ssize_t linenum);


/* Public Impl ****************************************************************/


int io_index_read(index_t *idx, const char *path)
{
    FILE *stream;
    size_t linenum;
    int dgsts;
    int expected;
    entry_t entry;
    digest_t type;

    if ((stream = fopen(path, "r")) == NULL)
    {
        log_error("cannot open '%s'", path);
        return -1;
    }

    type = index_get_digest_type(idx);
    linenum = 0;
    expected = 0;
    while (io_entry_read(&entry, stream, &linenum) == 0)
    {
        /* the index is only regular files, ignore everything else */
        if (!S_ISREG(entry.mode))
            continue;

        /* create a mask of all digests that the entry had */
       dgsts = valid_digests(&entry);

       /* make sure the index's digest type is defined */
       if ((dgsts & type) == 0)
       {
           log_warnx("ignoring entry at '%s:%zd': missing '%s'", path, linenum,
                   digest_name(type));
           continue;
       }

       /* for consistency check that all lines have the same digests */
       if (expected == 0)
           expected = dgsts;
       else if (expected != dgsts)
           log_warnx("inconsistent fields found at '%s:%zd'", path, linenum);


       switch (type)
       {
       case DGST_MD5:
           add_or_warn(idx, entry.pathmd5, entry.md5, path, linenum);
           break;

       case DGST_SHA1:
           add_or_warn(idx, entry.pathmd5, entry.sha1, path, linenum);
           break;

       case DGST_SHA256:
           add_or_warn(idx, entry.pathmd5, entry.sha256, path, linenum);
           break;

       case DGST_SHA512:
           add_or_warn(idx, entry.pathmd5, entry.sha512, path, linenum);
           break;
       }
    }
    fclose(stream);
    return 0;
}


int io_index_digest_peek(const char *paths[], size_t count, int *digests)
{
    size_t i;
    FILE *stream;
    size_t linenum;
    entry_t entry;

    for (i = 0; i < count; i++)
    {
        if ((stream = fopen(paths[i], "r")) == NULL)
        {
            log_error("cannot open '%s'", paths[i]);
            return -1;
        }

        /* use the first regular file entry to determine the digests */
        linenum = 0;
        while (io_entry_read(&entry, stream, &linenum) == 0)
        {
            if (S_ISREG(entry.mode))
            {
                *digests = valid_digests(&entry);
                fclose(stream);
                return 0;
            }
        }
        fclose(stream);
    }
    return -1;
}


/* Private Impl ***************************************************************/


inline void add_or_warn(index_t *idx, const void *pathmd5, const void *digest,
        const char *file, ssize_t linenum)
{
    if (index_lookup(idx, pathmd5, digest) == INDEX_SUCCESS)
       log_warnx("skipping entry at '%s:%zd': already in index", file,
               linenum);
    else
        index_insert(idx, pathmd5, digest);
}


int valid_digests(const entry_t *entry)
{
    int dgsts;

    dgsts = 0;
    if (entry->md5    != NULL) dgsts |= DGST_MD5;
    if (entry->sha1   != NULL) dgsts |= DGST_SHA1;
    if (entry->sha256 != NULL) dgsts |= DGST_SHA256;
    if (entry->sha512 != NULL) dgsts |= DGST_SHA512;
    return dgsts;
}
