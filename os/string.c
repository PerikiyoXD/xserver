/* SPDX-License-Identifier: MIT OR X11
 *
 * Copyright © 1987, 1998  The Open Group
 * Copyright © 2024 Enrico Weigelt, metux IT consult <info@metux.net>
 * Copyright © 2025 Pedro Luis Valadés Viera <pvaladesv98@proton.me>
 */

/**
 * @file string.c
 * @brief Optimized string tokenization for X11 server
 *
 * High-performance single-allocation tokenizer that replaces strtok-based
 * implementation with manual parsing for better efficiency and portability.
 */

/* System headers */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Project configuration */
#include <dix-config.h>

/* Project headers */
#include "os/fmt.h"
#include "os.h"

/**
 * @brief Tokenize string into NULL-terminated array with single allocation
 * @param str Input string to tokenize (may be NULL)
 * @param separators Delimiter characters (may be NULL)
 * @return NULL-terminated array of strings, or NULL on error
 *
 * High-performance tokenizer using single memory allocation for optimal
 * cache locality and minimal malloc overhead. Replaces strtok-based
 * implementation with manual parsing for cross-platform compatibility.
 *
 * Algorithm:
 * 1. Two-pass parsing: count tokens and calculate space requirements
 * 2. Single allocation: array + all strings in contiguous memory
 * 3. Direct string copying with pointer arithmetic
 *
 * Memory layout: [ptr0][ptr1]...[ptrN][NULL][str1\0][str2\0]...[strN\0]
 *
 * Benefits over original:
 * - Single malloc instead of N+2 mallocs (array + N tokens + temp string)
 * - Better cache locality (strings adjacent to pointers)
 * - No strtok dependency (thread-safe, cross-platform)
 * - Simpler cleanup (single free vs N+1 frees)
 * - No reallocarray calls (pre-calculated exact size)
 *
 * @note Input string is not modified
 * @warning Returns NULL on allocation failure or invalid parameters
 */

char **
xstrtokenize(const char *str, const char *separators)
{
    if (!str || !separators)
        return NULL;

    /* First pass: count tokens and calculate total string storage needed */
    const char *current = str;
    size_t token_count = 0;
    size_t total_string_bytes = 0;

    while (*current) {
        /* Skip leading separators */
        while (*current && strchr(separators, *current))
            current++;

        if (!*current)
            break;

        /* Found start of token - measure it */
        const char *token_start = current;
        while (*current && !strchr(separators, *current))
            current++;

        size_t token_length = current - token_start;
        token_count++;
        total_string_bytes += token_length + 1;  /* +1 for null terminator */
    }

    /* Handle empty result */
    if (token_count == 0) {
        char **empty_array = malloc(sizeof(char *));
        if (empty_array)
            empty_array[0] = NULL;
        return empty_array;
    }

    /* Calculate total memory requirement */
    size_t array_bytes = (token_count + 1) * sizeof(char *);  /* +1 for NULL terminator */
    size_t total_bytes = array_bytes + total_string_bytes;

    /* Check for potential overflow */
    if (total_string_bytes > SIZE_MAX - array_bytes) {
        return NULL;  /* Overflow would occur */
    }

    /* Single allocation for everything */
    char **token_array = malloc(total_bytes);
    if (!token_array)
        return NULL;

    /* String storage area starts after the pointer array */
    char *string_storage = (char *)token_array + array_bytes;
    char *current_string_pos = string_storage;

    /* Second pass: extract tokens into allocated memory */
    current = str;
    size_t token_index = 0;

    while (*current && token_index < token_count) {
        /* Skip leading separators */
        while (*current && strchr(separators, *current))
            current++;

        if (!*current)
            break;

        /* Extract token */
        const char *token_start = current;
        while (*current && !strchr(separators, *current))
            current++;

        size_t token_length = current - token_start;

        /* Set array pointer to current string position */
        token_array[token_index] = current_string_pos;

        /* Copy token with explicit null termination */
        memcpy(current_string_pos, token_start, token_length);
        current_string_pos[token_length] = '\0';

        /* Advance to next string position */
        current_string_pos += token_length + 1;
        token_index++;
    }

    /* NULL-terminate the pointer array */
    token_array[token_count] = NULL;

    return token_array;
}