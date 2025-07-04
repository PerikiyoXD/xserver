/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 * Copyright (c) 2025 Pedro Luis Valadés Viera <pvaladesv98@proton.me>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND TODD C. MILLER DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL TODD C. MILLER BE LIABLE
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file strlcat.c
 * @brief Safe string concatenation implementation
 */

/* System headers */
#include <sys/types.h>
#include <string.h>

/* Project configuration */
#include <dix-config.h>

/* Project headers */
#include "os.h"

/**
 * @brief Safe string concatenation with size bounds
 * @param dst Destination buffer to append to
 * @param src Source string to append
 * @param siz Total size of destination buffer (not remaining space)
 * @return Total length that would result (may exceed siz if truncated)
 * 
 * Appends src to dst with guaranteed null termination and bounds checking.
 * Unlike strncat(), siz represents the full buffer size, not remaining space.
 * 
 * Return value semantics:
 * - If return >= siz: truncation occurred
 * - If return < siz: concatenation successful
 * 
 * @note Always null-terminates unless siz <= strlen(dst)
 * @warning dst must be null-terminated string on input
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
    char *dst_ptr = dst;
    const char *src_ptr = src;
    size_t remaining = siz;
    size_t dst_len;

    /* Find end of dst and calculate remaining space */
    while (remaining-- != 0 && *dst_ptr != '\0')
        dst_ptr++;
    
    dst_len = dst_ptr - dst;
    remaining = siz - dst_len;

    if (remaining == 0)
        return (dst_len + strlen(src_ptr));

    /* Copy src characters while space remains */
    while (*src_ptr != '\0') {
        if (remaining != 1) {
            *dst_ptr++ = *src_ptr;
            remaining--;
        }
        src_ptr++;
    }
    
    /* Always null terminate */
    *dst_ptr = '\0';

    return (dst_len + (src_ptr - src));  /* Total length (excluding null) */
}