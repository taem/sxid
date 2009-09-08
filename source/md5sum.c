/*
 * md5sum.c   - Generate/check MD5 Message Digests
 *
 * Compile and link with md5.c.  If you don't have getopt() in your library
 * also include getopt.c.  For MSDOS you can also link with the wildcard
 * initialization function (wildargs.obj for Turbo C and setargv.obj for MSC)
 * so that you can use wildcards on the commandline.
 *
 * Written March 1993 by Branko Lankester
 * Modified June 1993 by Colin Plumb for altered md5.c.
 * Modified Feburary 1995 by Ian Jackson for use with Colin Plumb's md5.c.
 * Hacked (modified is too nice a word) January 1997 by Galen Hazelwood
 *   to support GNU gettext.
 * This file is in the public domain.
 *
 * Modified for sXid by Ben Collins - Dec. 1998
 */

#include <stdio.h>
#include <string.h>

#include "sxid.h"
#include "md5.h"

int mdfile (FILE * fp, unsigned char *digest)
{
    unsigned char buf[1024];
    struct MD5Context ctx;
    int n;

    MD5Init (&ctx);
    while ((n = fread (buf, 1, sizeof (buf), fp)) > 0)
	MD5Update (&ctx, buf, n);
    MD5Final (digest, &ctx);
    if (ferror (fp))
	return -1;
    return 0;
}

int hex_digit (int c)
{
    if (c >= '0' && c <= '9')
	return c - '0';
    if (c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    return -1;
}

int do_check (char *path, char *md5sum)
{
    unsigned char chk_digest[16], file_digest[16];
    FILE *fp;
    int i, d1, d2;
    char *p = md5sum;

    fp = fopen (path, "r");
    if (fp == NULL)
	return 1;
    if (mdfile (fp, file_digest))
	return 2;
    fclose (fp);
    for (i = 0; i < 16; ++i) {
	if ((d1 = hex_digit (*p++)) == -1)
	    return 3;
	if ((d2 = hex_digit (*p++)) == -1)
	    return 4;
	chk_digest[i] = (d1 * 16) + d2;
    }
    return memcmp (chk_digest, file_digest, 16);
}
