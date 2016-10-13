/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * API for reading and writing entries from and to a stream. Entries are written
 * as JSON Objects where only valid digests are written. A single entry is on
 * a single line.
 */
#ifndef IO_ENTRY_H__
#define IO_ENTRY_H__


#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>

#include "../entry.h"


/* Public API *****************************************************************/


/**
 * Read the next entry from the stream ignoring any metadata lines. Keeps track
 * of what line # we are on from the stream and uses it for logging when an
 * error occurs. When -1 is returned there was an error or EOF was hit, use
 * feof() to determine if EOF.
 *
 * @param entry         where to store the data read from the stream
 * @param in            what stream to read the next record from
 * @param line          pointer to a line number to update
 *
 * @return              0 on success, -1 on error/EOF
 */
int io_entry_read(entry_t *entry, FILE *in, size_t *line);


/**
 * write the following fields as a JSON object to the stream
 *
 * @param state         string describing how the processing of the entry went
 * @param path          abs path of the file, with src dir treated as root
 * @param st            pointer to the source file's stat struct
 * @param pathmd5       16 byte md5 of the path
 * @param symlinkpath   if entry is symlink where it's pointing, NULL otherwise
 * @param md5           the md5 of the file's contents or NULL if not applicable
 * @param sha1          sha1 of the file's contents or NULL if not applicable
 * @param sha256        sha256 of the file's contents or NULL if not applicable
 * @param sha512        sha512 of the file's contents or NULL if not applicable
 * @param process_time  # of milliseconds it took to process the file
 * @param stream        where to write the json object
 *
 * @return              0 on success, -1 on error
 */
int io_entry_write_fields(const char *state, const char *path,
        const struct stat *st, const void *pathmd5, const char *symlinkpath,
        const void *md5, const void *sha1, const void *sha256,
        const void *sha512, long process_time, FILE *stream);


#endif
