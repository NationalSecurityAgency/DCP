/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of the io_metadata API.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "io_metadata.h"
#include "jansson.h"
#include "../logging.h"


/* Private API ****************************************************************/


/**
 *  Put a string into the stream but replace all instances of <LF>, aka '\n',
 *  with "\\n" and tabs from '\n' to "\\n"
 *
 *  NOTE: This is a poor escaping method and shouldn't be used for anything that
 *  will be parsed. If parsability is needed use `fputjsonstr` or
 *  `fputjsonstrs`.
 *
 *  @param s        the string to put on the stream escaped
 *  @param stream   the stream to put the string on
 */
static void escaping_fputs(const char *s, FILE *stream);


/**
 * print a string array to the output as a json array
 *
 * @param count number of items in strs
 * @param strs  the array of strings to escape and print
 * @param out   the stream to write the json array to
 */
static void fputjsonstrs(int count, const char *strs[], FILE *out);


/* Public Impl ****************************************************************/


int io_metadata_put(const char *key, const char *value, FILE *stream)
{
    return io_metadata_put_strs(key, value==NULL? 0 : 1, &value, NULL, stream);
}


int io_metadata_put_strs(const char *key, int count, const char *val[],
        const char *delim, FILE *stream)
{
    int i;

    if (delim == NULL)
        delim = " ";

    fputc('#', stream);
    escaping_fputs(key, stream);
    if (count > 0)
    {
        fputs("\t", stream);
        escaping_fputs(val[0], stream);
        for (i = 1; i < count; i++)
        {
            escaping_fputs(delim, stream);
            escaping_fputs(val[i], stream);
        }
    }
    fputs("\n", stream);
    return 0;
}


int io_metadata_put_json(const char *key, int count, const char *val[],
        FILE *stream)
{
    fputc('#', stream);
    escaping_fputs(key, stream);
    fputc('\t', stream);
    fputjsonstrs(count, val, stream);
    fputc('\n', stream);
    return 0;
}


/* Private Impl ***************************************************************/


inline void escaping_fputs(const char *s, FILE *out)
{
    size_t len;
    size_t i;
    char c;
    for (i = 0, len = strlen(s); i < len; i++)
    {
        c = s[i];
        switch (c)
        {
        case '\n':  fputs("\\n", out);  break;
        case '\t':  fputs("\\t", out);  break;
        default:    fputc(c, out);
        }
    }
}


inline void fputjsonstrs(int count, const char *strs[], FILE *out)
{
    json_t *array;
    json_t *string;
    int i;

    array = json_array();
    for (i = 0; i < count; i++)
    {
        if ((string = json_string(strs[i])) == NULL)
        {
            log_errorx("cannot print metadata: invalid utf-8 string '%s'",
                    strs[i]);
            json_decref(array);
            return;
        }

        /* *_new functions steal the reference count, so we don't need to call
         * json_decref on string */
        json_array_append_new(array, string);
    }

    json_dumpf(array, out, JSON_COMPACT);
    json_decref(array);
}

