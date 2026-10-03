/* Simple utilities to be used across the rest of the project, kind of libc-like
 * 
 * See /src/util.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <types.h>

/* Gets the number of characters of a C string
 *
 * Str: The string to get the length of
 *
 * Returns the length, not including the null terminator
 */
USIZE CStringLength(CSTRING Str);

/* Checks if two C strings are the same in value
 *
 * Str1: The string to compare with string 2
 * Str2: The string to compare with string 1
 *
 * Returns TRUE if they are equal otherwise FALSE
 */
BOOL CStringsAreEqual(CSTRING Str1, CSTRING Str2);
