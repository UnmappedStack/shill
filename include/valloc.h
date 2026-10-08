/* bootstrap virtual memory allocator, just bump
 *
 * See /src/valloc.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <types.h>

#define BUMP_START (0x4000)
#define KERNEL_STACK_BUMP_START (0xFFFFFFFFFFFF0000LL)

typedef enum {
    // most virtual memory allocations,
    // very low in memory (from BUMP_START)
    VALLOC_ZONE_DEFAULT,

    // allocate kernel stacks, very high
    // in memory (from KERNEL_STACK_BUMP_START)
    VALLOC_ZONE_STACK,

    // shouldn't be used, just used to mark the maximum size
    // (usage of this zone and above will return an error)
    VALLOC_ZONE_MAX,
} VAllocZone;

/* allocates NumPages virtual pages *but does not map them in*
 *
 * (!) Side effects: adds to one of the VirtualBumpCurrent members
 *
 * NumPages: the number of pages to allocate
 * Zone: the zone of virtual memory to allocate in
 *
 * Returns the starting address of the allocated memory
 */
PTR AllocateVirtualPages(USIZE NumPages, VAllocZone Zone);

/* allocates physical memory, allocates continuous virtual memory, and backs
 * the physical memory with said virtual memory
 *
 * (!) Side effects: adds to one of the VirtualBumpCurrent members
 *
 * NumPages: the number of pages to allocate
 * Zone: the zone of virtual memory to allocate at
 *
 * Returns the starting virtual address of the allocated memory
 */
PTR AllocateBackedPages(USIZE NumPages, VAllocZone Zone);
