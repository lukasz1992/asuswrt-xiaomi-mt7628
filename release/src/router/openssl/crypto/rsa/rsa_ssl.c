/* crypto/rsa/rsa_ssl.c */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include "constant_time_locl.h"

int RSA_padding_add_SSLv23(unsigned char *to, int tlen,
                           const unsigned char *from, int flen)
{
    int i, j;
    unsigned char *p;

    if (flen > (tlen - 11)) {
        RSAerr(RSA_F_RSA_PADDING_ADD_SSLV23,
               RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
        return (0);
    }

    p = (unsigned char *)to;

    *(p++) = 0;
    *(p++) = 2;                 /* Public Key BT (Block Type) */

    /* pad out with non-zero random data */
    j = tlen - 3 - 8 - flen;

    if (RAND_bytes(p, j) <= 0)
        return (0);
    for (i = 0; i < j; i++) {
        if (*p == '\0')
            do {
                if (RAND_bytes(p, 1) <= 0)
                    return (0);
            } while (*p == '\0');
        p++;
    }

    memset(p, 3, 8);
    p += 8;
    *(p++) = '\0';

    memcpy(p, from, (unsigned int)flen);
    return (1);
}

/*
 * Copy of RSA_padding_check_PKCS1_type_2 with a twist that rejects padding
 * if nul delimiter is preceded by 8 consecutive 0x03 bytes. It also
 * preserves error code reporting for backward compatibility.
 */
int RSA_padding_check_SSLv23(unsigned char *to, int tlen,
                             const unsigned char *from, int flen, int num)
{
    int i;
    /* |em| is the encoded message, zero-padded to exactly |num| bytes */
    unsigned char *em = NULL;
    unsigned int good, found_zero_byte, mask, threes_in_row;
    int zero_index = 0, msg_index, mlen = -1, err;

    if (tlen <= 0 || flen <= 0)
        return -1;

    if (flen > num || num < 11) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_SSLV23, RSA_R_DATA_TOO_SMALL);
        return (-1);
    }

    em = OPENSSL_malloc(num);
    if (em == NULL) {
        RSAerr(RSA_F_RSA_PADDING_CHECK_SSLV23, ERR_R_MALLOC_FAILURE);
        return -1;
    }
    /*
     * Caller is encouraged to pass zero-padded message created with
     * BN_bn2binpad. Trouble is that since we can't read out of |from|'s
     * bounds, it's impossible to have an invariant memory access pattern
     * in case |from| was not zero-padded in advance.
     */
    for (from += flen, em += num, i = 0; i < num; i++) {
        mask = ~constant_time_is_zero(flen);
        flen -= 1 & mask;
        from -= 1 & mask;
        *--em = *from & mask;
    }

    good = constant_time_is_zero(em[0]);
    good &= constant_time_eq(em[1], 2);
    err = constant_time_select_int(good, 0, RSA_R_BLOCK_TYPE_IS_NOT_02);
    mask = ~good;

    /* scan over padding data */
    found_zero_byte = 0;
    threes_in_row = 0;
    for (i = 2; i < num; i++) {
        unsigned int equals0 = constant_time_is_zero(em[i]);

        zero_index = constant_time_select_int(~found_zero_byte & equals0,
                                              i, zero_index);
        found_zero_byte |= equals0;

        threes_in_row += 1 & ~found_zero_byte;
        threes_in_row &= found_zero_byte | constant_time_eq(em[i], 3);
    }

    /*
     * PS must be at least 8 bytes long, and it starts two bytes into |em|.
     * If we never found a 0-byte, then |zero_index| is 0 and the check
     * also fails.
     */
    good &= constant_time_ge(zero_index, 2 + 8);
    err = constant_time_select_int(mask | good, err,
                                   RSA_R_NULL_BEFORE_BLOCK_MISSING);
    mask = ~good;

    /*
     * Reject if nul delimiter is preceded by 8 consecutive 0x03 bytes. Note
     * that RFC5246 incorrectly states this the other way around, i.e. reject
     * if it is not preceded by 8 consecutive 0x03 bytes. However this is
     * corrected in subsequent errata for that RFC.
     */
    good &= constant_time_lt(threes_in_row, 8);
    err = constant_time_select_int(mask | good, err,
                                   RSA_R_SSLV3_ROLLBACK_ATTACK);
    mask = ~good;

    /*
     * Skip the zero byte. This is incorrect if we never found a zero-byte
     * but in this case we also do not copy the message out.
     */
    msg_index = zero_index + 1;
    mlen = num - msg_index;

    /*
     * For good measure, do this check in constant time as well.
     */
    good &= constant_time_ge(tlen, mlen);
    err = constant_time_select_int(mask | good, err, RSA_R_DATA_TOO_LARGE);

    /*
     * Move the result in-place by |num|-11-|mlen| bytes to the left.
     * Then if |good| move |mlen| bytes from |em|+11 to |to|.
     * Otherwise leave |to| unchanged.
     * Copy the memory back in a way that does not reveal the size of
     * the data being copied via a timing side channel. This requires copying
     * parts of the buffer multiple times based on the bits set in the real
     * length. Clear bits do a non-copy with identical access pattern.
     * The loop below has overall complexity of O(N*log(N)).
     */
    tlen = constant_time_select_int(constant_time_lt(num - 11, tlen),
                                    num - 11, tlen);
    for (msg_index = 1; msg_index < num - 11; msg_index <<= 1) {
        mask = ~constant_time_eq(msg_index & (num - 11 - mlen), 0);
        for (i = 11; i < num - msg_index; i++)
            em[i] = constant_time_select_8(mask, em[i + msg_index], em[i]);
    }
    for (i = 0; i < tlen; i++) {
        mask = good & constant_time_lt(i, mlen);
        to[i] = constant_time_select_8(mask, em[i + 11], to[i]);
    }

    OPENSSL_cleanse(em, num);
    OPENSSL_free(em);
    RSAerr(RSA_F_RSA_PADDING_CHECK_SSLV23, err);
    err_clear_last_constant_time(1 & good);

    return constant_time_select_int(good, mlen, -1);
}
