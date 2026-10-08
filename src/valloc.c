/* bootstrap virtual memory allocator, just bump
 *
 * See /include/valloc.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <valloc.h>
#include <paging.h>
#include <util.h>
#include <types.h>
#include <api.h>

static PTR VirtualBumpCurrents[] = {
    [VALLOC_ZONE_DEFAULT] = BUMP_START,
    [VALLOC_ZONE_STACK  ] = KERNEL_STACK_BUMP_START,
};

/* allocates NumPages virtual pages *but does not map them in*
 *
 * (!) Side effects: adds to one of the VirtualBumpCurrent members
 *
 * NumPages: the number of pages to allocate
 * Zone: the zone of virtual memory to allocate in
 *
 * Returns the starting address of the allocated memory
 */
PTR AllocateVirtualPages(USIZE NumPages, VAllocZone Zone) {
    if (Zone >= VALLOC_ZONE_MAX) {
        WriteConsole("Invalid Shill bootstrap virtual allocator zone, halt device\n");
        HaltDevice();
    }

    PTR Return = VirtualBumpCurrents[Zone];
    VirtualBumpCurrents[Zone] += NumPages * PAGE_SIZE;
    return Return;
}

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
PTR AllocateBackedPages(USIZE NumPages, VAllocZone Zone) {
    PTR VirtAddr = AllocateVirtualPages(NumPages, Zone);
    AllocVirtuallyConsecutivePages((USIZE*)GPML4, VirtAddr, NumPages, PAGE_PRESENT | PAGE_WRITE);
    return VirtAddr;
}
