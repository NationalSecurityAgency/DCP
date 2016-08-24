/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of the io_xattr.h interface. Entries in dcp input and output
 * are an entry in the filesystem that was copied/duplicated. For each
 * file and symbolic link we log xattr information about each as a single line json
 * object.
 */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <linux/limits.h>

#include "io_xattr.h"
#include "../digest.h"
#include "pack.h"
#include "../logging.h"


/* Public Impl ****************************************************************/


/* while jansson can be used for creating a json structure then printing it
 * out, there is a large overhead cost. Since we control the data only use
 * jansson to escape the strings we cannot control. */
int io_entry_write_xattr_fields(const void *pathmd5, const char *name, void *valuebuffer, ssize_t valuesize, FILE *stream)
{

    enum { MAX_LENGTH = PATH_MAX * 4 };

    int ret;

    /* used to hold hex strings for the hashes and the path if it contains
     * non utf-8 characters */
    char buf[MAX_LENGTH];


    ret = 0;

    /* we are writing a json object on the line*/
    fputc('{', stream);

    unpack(buf, pathmd5, MD5_DIGEST_LENGTH);
    fprintf(stream, "\"pathmd5\":\"%s\",", buf);

    if (name != NULL)
    	fprintf(stream, "\"xattrName\":\"%s\",",name);

    if (valuesize > 0)
    {
    	if (valuebuffer != NULL) {
    		char valuehex[8193]; /* 4096 * 2 + 1 */
    		memset(valuehex, 0, sizeof(valuehex));
    		unpack(valuehex, valuebuffer, valuesize);
    		fprintf(stream, "\"xattrValue\":\"%s\",",valuehex);
    	}
    }
    goto cleanup;


cleanup:
    /* print a newline our record separator */
    fputs("}\n", stream);
    return ret;
}
