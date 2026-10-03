/* Simple utilities to be used across the rest of the project, kind of libc-like
 * 
 * See /include/util.h
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
USIZE CStringLength(CSTRING Str) {
    USIZE Length = 0;
    while (*Str) {
        Length++;
        Str++;
    }
    return Length;
}

/* Checks if two C strings are the same in value
 *
 * Str1: The string to compare with string 2
 * Str2: The string to compare with string 1
 *
 * Returns TRUE if they are equal otherwise FALSE
 */
BOOL CStringsAreEqual(CSTRING Str1, CSTRING Str2) {
    USIZE Len1 = CStringLength(Str1);
    USIZE Len2 = CStringLength(Str2);
    if (Len1 != Len2) return FALSE;

    for (USIZE Char = 0; Char < Len1; Char++) {
        if (Str1[Char] != Str2[Char]) return FALSE;
    }

    return TRUE;
}
