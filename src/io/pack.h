/**
 * @file
 *
 * @version 1.0
 * @date 2014/04/25
 *
 * @section DESCRIPTION
 *
 * No standard functions for these in glibc so rolling our own.
 */
#ifndef PACK_H__
#define PACK_H__


#include <stddef.h>


/* Public API *****************************************************************/


/**
 * Convert a C string of hex characters to a byte array
 *
 * "001122ac" becomes [0x00, 0x11, 0x22, 0xac]
 */
int pack(void *dest, const char *hex, size_t line);


/**
 * Write the bytes in source as a hex string
 *
 * [0x00, 0x11, 0x22, 0xac] becomes "001122ac"
 */
void unpack(char *dest, const void *src, size_t count);


#endif
