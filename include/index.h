/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * General information for a data structure to store information on regular
 * files that have been seen. API allows insertion of new records as well as
 * lookups based on file path md5 and a file digest, @see DGSTTYPE, specified at
 * initialization.
 *
 * Current implementation @see db_index.c
 */
#ifndef INDEX_H__
#define INDEX_H__


#include <stddef.h>
#include <linux/limits.h>

#include "digest.h"


/* Type Defs ******************************************************************/


/**
 * the index type created and maintained by this API
 */
typedef struct index index_t;


typedef enum {
    INDEX_FAILED = -1,
    INDEX_SUCCESS = 0,
    INDEX_NO_ENTRY
} index_return_t;


/* Public API *****************************************************************/


/**
 * Deallocate any resources associated with the index
 *
 * @param idx           the index to free
 *
 * @return              INDEX_SUCCESS or INDEX_FAILED on unrecoverable error
 */
index_return_t index_free(index_t *idx);


/**
 * Given the index return what DGST_TYPE is being used for entry lookups
 *
 * @param idx           index to lookup the key for
 *
 * @return              @see DGSTTYPE
 */
digest_t index_get_digest_type(index_t *idx);


/**
 * Initialize an index that performs pathmd5 and digest lookups
 *
 * @param idx           pointer to the index to initialize
 * @param digest        The digest type to use in the index key
 *
 * @return              INDEX_SUCCESS or INDEX_FAILED on unrecoverable error
 */
index_return_t index_create(index_t **idx, digest_t digest);


/**
 * Add an entry to the index.
 *
 * Returns
 *      INDEX_SUCCESS           on successful insertion of record
 *      INDEX_FAILED            on unrecoverable error inserting to the index
 */
index_return_t index_insert(index_t *idx, const void *pathmd5,
        const void* digest);


/**
 * Lookup for a file with a given hash in the index.
 */
index_return_t index_lookup(index_t *idx, const void *pathmd5,
        const void *digest);


#endif
