/**
 * @file
 *
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 * Implementation of the index.h api using an in-memory Berkeley DB B-Tree.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <db.h>

#include "index.h"
#include "../digest.h"
#include "../logging.h"
#include "../io/pack.h"


/* Type Defs ******************************************************************/


/**
 * Holds all the information to access the berkeley db btree.
 *
 * @param dbh               handle to the berkeley db instance
 * @param key_digest_type   type of digest used for search in our btree
 * @param key_digest_length the # of bytes of our digest used for search
 * @param key_digest        given a ptr to an entry retrieve a pointer to the
 *                          digest used for searching
 */
struct index {
    DB *dbh;
    digest_t key_digest_type;
    size_t key_digest_length;
};


/**
 * Key for the berkleydb b-tree. 2D the md5 of the file's path and the hash of
 * the file. Note: When using make sure to zero out the entire structure since
 * the comparison function @see key_cmp compares ALL bytes in the structure
 * (md5 is shorter than the 64 bytes set aside for the digest).
 */
struct key {
    unsigned char pathmd5[MD5_DIGEST_LENGTH];
    unsigned char digest[MAX_DIGEST_LENGTH];
} __attribute__((packed));


/* Private API ****************************************************************/


/**
 * db errcall function to dump all error messages to our logging, debug level.
 * If Berkeley DB is logging an error this function will be used instead of
 * writing to stdout or stderr.
 *
 * @param dbenv     The db environment
 * @param erpfx     The prefix configured in the env
 * @param msg       The string message to log with our API
 */
static void errcall_debug(const DB_ENV *dbenv, const char *errpfx,
        const char *msg);


/**
 * key comparison function to tell berkleydb to use. All key structures must
 * have unused bytes as 0 since we compare all bytes in the structure.
 */
static int key_cmp(DB *db, const DBT *dbt1, const DBT *dbt2);


/**
 * sets up the berkeley db in-memory index and populates the structure. Set's
 * up the db to have a larger cache size and sets the key comparison function
 * to use.
 *
 * @param idx       pointer to the index_t to populate with the new database
 *
 * @return      0 on success.
 */
static int init_db(struct index *idx);


/* Public Impl ****************************************************************/


index_return_t index_create(index_t **idx, digest_t digest_type)
{
    if (idx == NULL)
        return INDEX_FAILED;

    *idx = malloc(sizeof(struct index));

    (*idx)->key_digest_type = digest_type;
    (*idx)->key_digest_length = DIGEST_LENGTH(digest_type);
    init_db(*idx);

    return INDEX_SUCCESS;
}


index_return_t index_free(index_t *idx)
{
    if (idx != NULL)
    {
        idx->dbh->close(idx->dbh, 0);
        free(idx);
    }
    return INDEX_SUCCESS;
}


digest_t index_get_digest_type(index_t *idx)
{
    return idx->key_digest_type;
}


index_return_t index_insert(index_t *idx, const void *pathmd5,
        const void *digest)
{
    DBT key;
    DBT val;
    struct key k;

    memset(&key, 0, sizeof(key));
    memset(&val, 0, sizeof(val));
    /* key struct must be zeroed since for comparisons we use all the bytes */
    memset(&k, 0, sizeof(k));

    memcpy(&k.pathmd5, pathmd5, MD5_DIGEST_LENGTH);
    memcpy(&k.digest, digest, idx->key_digest_length);

    key.data = &k;
    key.size = sizeof(k);

    if (idx->dbh->put(idx->dbh, NULL, &key, &val, 0) != 0)
    {
        log_errorx("failed to write an index entry");
        return INDEX_FAILED;
    }

    return INDEX_SUCCESS;
}


index_return_t index_lookup(index_t *idx, const void *pathmd5,
        const void *digest)
{
    DBT key;
    DBT val; /* even though we don't use value it cannot be NULL */
    int r;
    struct key k;

    assert(pathmd5 != NULL && digest != NULL);

    memset(&key, 0, sizeof(key));
    memset(&val, 0, sizeof(val));
    /* key struct must be zeroed since for comparisons we use all the bytes */
    memset(&k, 0, sizeof(k));
    memcpy(&k.pathmd5, pathmd5, MD5_DIGEST_LENGTH);
    memcpy(&k.digest, digest, idx->key_digest_length);

    key.data = &k;
    key.size = sizeof(k);

    switch (r = idx->dbh->get(idx->dbh, NULL, &key, &val, 0))
    {
    case 0:
        return INDEX_SUCCESS;

    case DB_NOTFOUND:
        return INDEX_NO_ENTRY;

    default:
        idx->dbh->err(idx->dbh, r, "failed index lookup");
        return INDEX_FAILED;
    }
}


/* Private Impl ***************************************************************/


inline int init_db(struct index *idx)
{
    int r;
    DB *dbh;
    DB_ENV *dbe;

    if (db_create(&dbh, NULL, 0) != 0)
    {
        log_errorx("cannot create db with db_create");
        return -1;
    }

    /* increase the cachesize of bdb so that the database respides in memory
     * instead of on disk. The default db inmem size is 256KB, that won't work!
     * TODO profile output files to see what size they are, smaller is faster.
     * On test db was 115MB */
    dbh->set_cachesize(dbh, 0, 256 * 1024 * 1024 , 1);

    /* set the btree comparison function */
    dbh->set_bt_compare(dbh, &key_cmp);

    /* get the env to register a debug logging func with our log system */
    dbe = dbh->get_env(dbh);
    dbe->set_errcall(dbe, &errcall_debug);
    dbe->set_errpfx(dbe, "db");

    if ((r = dbh->open(dbh, NULL, NULL, NULL, DB_BTREE, DB_CREATE, 0)) != 0)
    {
        dbh->err(dbh, r, "cannot open db");
        return -1;
    }

    idx->dbh = dbh;

    return 0;
}


/* unused attribute set to allow gcc compilation with -Wextra and -Wall */
int key_cmp(DB *db __attribute__((unused)), const DBT *dbt1, const DBT *dbt2)
{
    return memcmp(dbt1->data, dbt2->data, sizeof(struct key));
}


/* unused attribute set to allow gcc compilation with -Wextra and -Wall */
void errcall_debug(const DB_ENV *dbenv __attribute__((unused)),
        const char *errpfx, const char *msg)
{
    log_errorx("%s %s", errpfx, msg);
}

