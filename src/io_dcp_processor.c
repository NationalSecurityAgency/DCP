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
#include <sys/xattr.h>

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
    FILE *xattrout; /**< where to write xattr values for paths */
};

/* Private API ****************************************************************/
static int process_xattrs(const void *pathmd5, const char *filepath, FILE *out);

/* Public Impl ****************************************************************/


int io_dcp_processor(dcp_state_t state, const void *pathmd5,
        const char *dapath, const struct stat *st, const char *accesspath,
        const char *symlinkpath, const void *md5, const void *sha1,
        const void *sha256, const void *sha512, unsigned long process_time,
        void *context)
{
    struct io_dcp_processor_ctx *ctx = context;
    process_xattrs(pathmd5, accesspath, ctx->xattrout);

    return io_entry_write_fields(dcp_strstate(state), dapath, st,
            pathmd5, symlinkpath, md5, sha1, sha256, sha512, process_time, ctx->out);
}


int io_dcp_processor_ctx_create(io_dcp_processor_ctx_t **ctx, FILE *stream, FILE *xattrstream)
{
    if (ctx != NULL)
    {
        *ctx = malloc(sizeof(struct io_dcp_processor_ctx));
        (*ctx)->out = stream;
        (*ctx)->xattrout = xattrstream;
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

/* Private Impl **************************************************************/
int process_xattrs(const void *pathmd5, const char *filepath, FILE *out)
{
	char *keys;
	uint8_t valuebuffer[4096];
	size_t valuebuffersize = sizeof(valuebuffer);
	ssize_t bufsize, bufsize2, valuesize;
	char *next, *end;
	int retval;

	if (filepath == NULL)
	{
		return 0; /* Nothing to do */
	}

	/* Use listxattrs to get a list of */
	bufsize = llistxattr(filepath, NULL, 0);
	if (bufsize == -1)
	{
		if (errno == ENOTSUP)
		{
			/* xattrs not supported or disabled for this file, success */
			return 0;
		}

		perror("llistxattr");
		return -1;
	}

	if (bufsize == 0)
	{
		/* no xattrs to process */
		return 0;
	}

	/* NOTE: Not sure if listxattr returns size including the last null byte */
	keys = NULL;
	keys = calloc(sizeof(char), bufsize + 1);
	if (keys == NULL)
	{
			perror("calloc");
			return -1;
	}

	retval = 0;
	bufsize2 = llistxattr(filepath, keys, bufsize);
	if (bufsize2 != bufsize)
	{
		/* possible race condition or just an error */
		perror("llistxattr");
		retval = -1;
		goto cleanup;
	}

	end = keys + bufsize; /* pointer to the end of the list */

	/* Pointer math */
	for (next = keys; next != end; next += (strlen(next) + 1))
	{
		memset(valuebuffer, 0, valuebuffersize); /* zero out the value buffer*/
		valuesize = lgetxattr(filepath, next, valuebuffer, valuebuffersize);
		if (valuesize < 0)
		{
			perror("lgetxattr");
		}

		else
		{
			io_entry_write_xattr_fields(pathmd5, next, valuebuffer, valuesize, out);
		}
	}

	cleanup:
		free(keys);
		fflush(out);

	return retval;
}
