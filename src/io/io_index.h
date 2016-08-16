/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * API for serializing an index of file's copied and their digests. Entries are
 * read and written using the io_entry.h API.
 */
#ifndef IO_INDEX_H__
#define IO_INDEX_H__


#include <stdio.h>

#include "../index/index.h"


/* Public API *****************************************************************/


/**
 * Adds all entries in the input file to the index
 *
 * @param index     what index to add the entries to
 * @param path      what file to add entries from
 *
 * @return          0 on success
 */
int io_index_read(index_t *index, const char *path);


/**
 * peek into the input files provided and determine what digests we should
 * calculate this run
 */
int io_index_digest_peek(const char *paths[], size_t count, int *digests);


#endif
