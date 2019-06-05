/**
 * @file
 *
 * @version 1.0
 * @date 2014/04/25
 *
 * @section DESCRIPTION
 *
 * todo write description for pack.c
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pack.h"
#include "logging.h"


/* MACROS *********************************************************************/


/**
 * ensure the ascii/utf8 bytes are valid hex characters [0-9a-fA-F]
 *
 * @param c     character to asses if it is a valid hex character
 *
 * @return      0 if no
 */
#define IS_VALID_HEX_CHAR(c) ((c>47 && c<58)||(c>64 && c<71)||(c>96 && c<103))


/* Private Variables **********************************************************/


/*
 * Go Go C! No standard way to pack a C String of hex characters. The following
 * only works with ascii and utf-8
 */

/*
 * Chars    Decimal
 * 0-9      48-57
 * A-F      65-70
 * a-f      97-102
 */
static const unsigned char hex2dec[128] = {
/*       0   1   2   3   4   5   6   7   8   9 */
/*  X*/ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 1X*/ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 2X*/ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 3X*/ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 4X*/ -1, -1, -1, -1, -1, -1, -1, -1,  0,  1,
/* 5X*/  2,  3,  4,  5,  6,  7,  8,  9, -1, -1,
/* 6X*/ -1, -1, -1, -1, -1, 10, 11, 12, 13, 14,
/* 7X*/ 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 8X*/ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/* 9X*/ -1, -1, -1, -1, -1, -1, -1, 10, 11, 12,
/*10X*/ 13, 14, 15, -1, -1, -1, -1, -1, -1, -1,
/*11X*/ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
/*12X*/ -1, -1, -1, -1, -1, -1, -1, -1
};


/* Public Impl ****************************************************************/


int pack(void *dest, const char *hex, size_t line)
{
    size_t i, j;
    size_t count;
    unsigned char c1, c2;

    count = strlen(hex);
    for (i = 0, j = 0; i < count; i += 2, j++)
    {
        c1 = hex[i];
        c2 = hex[i + 1];
        if (!(IS_VALID_HEX_CHAR(c1) && IS_VALID_HEX_CHAR(c2)))
        {
            log_errorx("corrupt input, invalid hex char '%c' or '%c' on line "
                    "%zu", c1, c2, line);
            return -1;
        }
        ((uint8_t *) dest)[j] =
                hex2dec[(int) hex[i]] * 16 + hex2dec[(int) hex[i + 1]];
    }
    return 0;
}

void unpack(char *dest, const void *src, size_t count)
{
    size_t i;
    memset(dest, 0, count * 2 + 1); /* ensure null termination of string */
    for (i = 0; i < count; i++, dest +=2)
        sprintf(dest, "%02x", ((const unsigned char *) src)[i]);
}


