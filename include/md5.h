/* MD5.H - header file for MD5C.C
* $FreeBSD: src/sys/sys/md5.h,v 1.20.10.1.2.1 2009/10/25 01:10:29 kensmith Exp $
*/

/*-
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
*/

#ifndef _MD5_H_
#define _MD5_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned int u_int32_t;

#define MD5_BLOCK_LENGTH 64
#define MD5_DIGEST_LENGTH 16
#define MD5_DIGEST_STRING_LENGTH (MD5_DIGEST_LENGTH * 2 + 1)

/* MD5 context. */
typedef struct MD5Context {
    u_int32_t state[4]; /* state (ABCD) */
    u_int32_t count[2]; /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64]; /* input buffer */
} MD5_CTX;

void MD5Init(MD5_CTX *);
void MD5Update(MD5_CTX *, const unsigned char *, unsigned int);
void MD5Final(unsigned char [16], MD5_CTX *);
void md5(const unsigned char *data, size_t len, char *hex);
char *str2md5(const char *str, int length);//http://stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c
#endif /* _MD5_H_ */
