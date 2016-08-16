/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * todo write description for io_dcp_processor.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "io_dcp_processor.h"
#include "entry.h"
#include "io/io.h"
#include "impl/dcp.h"
#include "logging.h"


/* Type Defs ******************************************************************/


/**
 * structure keeping track of what stream the processor should write the data
 * to.
 */
struct io_dcp_processor_ctx {
    FILE *out;      /**< where to write each file system entry info to */
};


/* Public Impl ****************************************************************/


int io_dcp_processor(dcp_state_t state, const void *pathmd5,
        const char *dapath, const struct stat *st, const void *md5,
        const void *sha1, const void *sha256, const void *sha512,
        unsigned long process_time, void *context)
{
    struct io_dcp_processor_ctx *ctx = context;
    return io_entry_write_fields(dcp_strstate(state), dapath, st,
            pathmd5, md5, sha1, sha256, sha512, process_time, ctx->out);
}


int io_dcp_processor_ctx_create(io_dcp_processor_ctx_t **ctx, FILE *stream)
{
    if (ctx != NULL)
    {
        *ctx = malloc(sizeof(struct io_dcp_processor_ctx));
        (*ctx)->out = stream;
        return 0;
    }
    return 0;
}


int io_dcp_processor_ctx_free(io_dcp_processor_ctx_t *ctx)
{
    if (ctx != NULL)
        free(ctx);
    return 0;
}
