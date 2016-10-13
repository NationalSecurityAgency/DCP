/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Serializing processor to be used as a callback from dcp. This output
 * processor will receive digests calculated for a file that was copied and
 * and use the io_entry.h API to write the information to the outputstream.
 */
#ifndef IO_DCP_PROCESSOR_H__
#define IO_DCP_PROCESSOR_H__


#include <sys/stat.h>
#include "impl/dcp.h"


/* Type Defs ******************************************************************/


/**
 * Context type for the io digest output processor. It holds information
 * needed for the processor to correctly execute.
 */
typedef struct io_dcp_processor_ctx io_dcp_processor_ctx_t;


/* Public API *****************************************************************/


/**
 * An dcp callback function which writes each file/dir/dev to the configured
 * FILE stream. Outputs each entry using @see io_entry.h.
 *
 * @param state             the resulting state of processing, @see impl/dcp.h
 * @param pathmd5           the md5 sum of the file's dapath
 * @param dapath            mount relative path to the file to be copied
 * @param st                stat struct for the source file
 * @param accesspath        path to the original file
 * @param symlinkpath       if file is a symbolic link, where is it pointing
 * @param md5               md5 digest of the file
 * @param sha1              sha1 digest of the file
 * @param sha256            sha256 digest of the file
 * @param sha512            sha512 digest of the file
 * @param elapsed           secs to process the entry, ignored if NULL
 * @param context           pointer to an initialized io_digest_output_context_t
 *                          instance
 *
 * @return                  0 on success
 */
int io_dcp_processor(dcp_state_t state, const void *pathmd5,
        const char *dapath, const struct stat *st, const char *accesspath,
        const char *symlinkpath, const void *md5, const void *sha1,
        const void *sha256, const void *sha512, unsigned long process_time,
        void *context);


/**
 * Initialize an instance of the out_context_t.
 *
 * @param ctx               pointer to the context to initialize
 * @param stream            where to serialize the entries to.
 * @param xattrstream       where to serialize the extended attributes to
 *
 * @return                  0 on success
 */
int io_dcp_processor_ctx_create(io_dcp_processor_ctx_t **ctx, FILE *stream,
        FILE *xattrstream);


/**
 * Tears down the context by freeing up any allocated resources.
 * Note: does not close the stream.
 *
 * @param ctx               pointer to the context instance to tear down
 *
 * @return                  0 on success
 */
int io_dcp_processor_ctx_free(io_dcp_processor_ctx_t *ctx);


#endif
