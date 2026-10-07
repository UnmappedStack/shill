/* Simple utilities to be used across the rest of the project, kind of libc-like
 * 
 * See /src/util.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <types.h>
#include <printf.h>
#include <hal.h>

#define PAGE_SIZE (4096)

#define _STRINGIFY(X) #X
#define STRINGIFY(X) _STRINGIFY(X)

#define ALIGN_UP(X, A) (((((PTR)X) + (A-1)) / (A)) * (A))
#define ALIGN_DOWN(X, A) (((X) / (A)) * (A))

#define ASSERT(X) \
    do { \
        if (!(X)) { \
            WriteConsole("Assert failed: " STRINGIFY(X) "\n"); \
            HaltDevice(); \
        } \
    } while (0)

#define UNUSED(X) ((VOID) X)

/* Set NumBytes bytes starting from Buf to Val */
#define SetBuffer memset
VOID *memset(VOID *Buf, U8 Val, USIZE NumBytes);

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

/* Checks if two ranges of memory have the same values
 *
 * Buf1: The buffer to compare with Buf2
 * Buf2: The buffer to compare with Buf1
 * Length: The number of bytes to compare
 *
 * Returns TRUE if equal otherwise FALSE
 */
BOOL BuffersAreEqual(VOID *Buf1, VOID *Buf2, USIZE Length);

/* memcpy style, copy NumBytes bytes from Source to Dest */
VOID *CopyBuffer(VOID *Dest, VOID *Source, USIZE NumBytes);
