/* Simple utilities to be used across the rest of the project, kind of libc-like
 * 
 * See /include/util.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <kgz.h>
#include <util.h>
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
 */
VOID *memset(VOID *Buf, U8 Val, USIZE NumBytes) {
#if defined(__x86_64__)
    if (NumBytes >= PAGE_SIZE * 2) {
        __asm__ volatile("REP STOSB" : "+D"(Buf), "+c"(NumBytes) : "a"(Val) : "memory");
        return Buf;
    }
#endif
    for (USIZE I = 0; I < NumBytes; I++)
        ((U8*)Buf)[I] = Val;
    return Buf;
}

/* memcpy style, copy NumBytes bytes from Source to Dest */
VOID *CopyBuffer(VOID *Dest, VOID *Source, USIZE NumBytes) {
// rep movsb is faster for large buffers if its x86...
#if defined(__x86_64__)
    if (NumBytes >= PAGE_SIZE * 2) {
        ASM("REP MOVSB"
                         : "=D"(Dest), "=S"(Source), "=c"(NumBytes)
                         : "D"(Dest), "S"(Source), "c"(NumBytes)
                         : "memory");
        return Dest;
    }
#endif
    // ...but it is actually faster to manually loop if its a relatively small memory buffer
    for (USIZE I = 0; I < NumBytes; I++) {
        ((uint8_t*)Dest)[I] = ((uint8_t*)Source)[I];
    }
    return Dest;
}

/* memcpy style, copy string from Source to Dest */
VOID *CopyString(CHAR *Dest, CHAR *Source) {
    USIZE Length = CStringLength((UBCHAR*)Source);
    CopyBuffer(Dest, Source, Length);
    return Dest;
}

/* Check if a file at some memory is GZ compressed
 *
 * Buffer: a pointer to the start of the buffer
 *
 * Returns TRUE if it is gzip compressed, otherwise FALSE
 */
BOOL VerifyGZ(PTR Buffer) {
    U8 *PBuf = (U8*) Buffer;
    return (PBuf[0] == 0x1f && PBuf[1] == 0x8b);
}

/* Decompress a GZ archive
 *
 * Side effects:
 *      - Allocates memory
 *      - Maps into the page tree
 *
 * ArchiveModule: the module containing the size & address of the data to decompress
 *
 * Returns a module of the decompressed data.
 */
ShillModuleInfo DecompressGZ(ShillModuleInfo *ArchiveModule) {
    WriteConsole("Decompressing GZ module... ");
    USIZE ResultSize, BufferSize;
    PTR DecompressedAddr = (PTR) KGZDecompress(
            (VOID*) ArchiveModule->Address,
            ArchiveModule->SizeBytes,
            &ResultSize, &BufferSize);

    ShillModuleInfo Decompressed;
    Decompressed.Address = DecompressedAddr;
    Decompressed.SizeBytes = ResultSize;
    WriteConsole(" Ok\n");
    return Decompressed;
}
