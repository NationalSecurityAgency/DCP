/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * API for reading/writing metadata lines from/to a stream. Metadata lines
 * begin with a '#' character allowing parsing to skip them while reading
 * records from the stream.
 */
#ifndef IO_METADATA_H__
#define IO_METADATA_H__


/* Public API *****************************************************************/


/**
 * Write the metadata key value pair to the stream.
 *
 * @param key       name of the metadata attribute
 * @param value     value of the attribute, can be NULL
 *
 * @return          0 on success
 */
int io_metadata_put(const char *key, const char *value, FILE *stream);


/**
 * Write a metadata entry where the value is a string array using a specified
 * delimiter between the array entries.
 *
 * @param key       name of the metadata attribute
 * @param count     number of strings in val
 * @param val       string array representing the value of this attribute
 * @param delim     string to delimit the strings in the array
 * @param stream    where to write the metadata
 *
 * @return          0 on success
 */
int io_metadata_put_strs(const char *key, int count, const char *val[],
        const char *delim, FILE *stream);


/**
 * Write the metadata where the value will be a JSON Array of strings. *_put
 * and *_put_strs above were designed for human reading not programatic parsing,
 * this function addresses that by properly escaping the value strings.
 *
 * '#key\t["val1", "val2", "val3", ...]'
 *
 * @param key       name of the metadata attribute
 * @param count     number of strings in val
 * @param val       string array which is the value of this attribute
 * @param stream    where to write the metadata
 */
int io_metadata_put_json(const char *key, int count, const char *val[],
        FILE *stream);


#endif
