/* Some general type definitions for use across Shill.
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef size_t   USIZE;

typedef int8_t  I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

// try to not use UBCHAR and just use CHAR... as the name implies, the signedness is UB 
typedef char UBCHAR;
typedef unsigned char CHAR;
typedef const char* CSTRING;
typedef uintptr_t PTR;
typedef void VOID;

#define NULLPTR nullptr
#define NORETURN __attribute((noreturn))
#define CONST const
#define STATIC static
