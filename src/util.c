/* Simple utilities to be used across the rest of the project, kind of libc-like
 * 
 * See /include/util.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <printf.h>
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

/* Checks if two ranges of memory have the same values
 *
 * Buf1: The buffer to compare with Buf2
 * Buf2: The buffer to compare with Buf1
 * Length: The number of bytes to compare
 *
 * Returns TRUE if equal otherwise FALSE
 */
BOOL BuffersAreEqual(VOID *Buf1, VOID *Buf2, USIZE Length) {
    for (USIZE Offset = 0; Offset < Length; Offset++) {
        if (((U8*)Buf1)[Offset] != ((U8*)Buf2)[Offset]) return FALSE;
    }

    return TRUE;
}

/* We need this because I'm stupid and I'm not using a cross compiler so GCC sometimes
 * complains without it... That's why it doesn't follow the usual naming scheme.
 * That's why its also quite unoptimised, not using rep movsb or whatever.
 * In the header file you can see its named CopyBuffer(). */
VOID memset(VOID *Buf, U8 Val, USIZE NumBytes) {
    for (USIZE Index = 0; Index < NumBytes; Index++) {
        ((U8*)Buf)[Index] = Val;
    }
}

/* memcpy style, copy NumBytes bytes from Source to Dest */
VOID CopyBuffer(VOID *Dest, VOID *Source, USIZE NumBytes) {
    // not well optimised but fine for now. TODO
    for (USIZE Index = 0; Index < NumBytes; Index++) {
        ((U8*)Dest)[Index] = ((U8*)Source)[Index];
    }
}
