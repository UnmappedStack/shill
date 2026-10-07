/* bootstrap virtual memory allocator, just bump
 *
 * See /include/valloc.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <paging.h>
#include <util.h>
#include <types.h>

// quite low in the lower half should be fine I think
#define BUMP_START (0x4000)
PTR VirtualBumpCurrent = BUMP_START;

/* allocates NumPages virtual pages *but does not map them in*
 *
 * Side effects: adds to VirtualBumpCurrent
 *
 * NumPages: the number of pages to allocate
 *
 * Returns the starting address of the allocated memory
 */
PTR AllocateVirtualPages(USIZE NumPages) {
    PTR Return = VirtualBumpCurrent;
    VirtualBumpCurrent += NumPages * PAGE_SIZE;
    return Return;
}

/* allocates physical memory, allocates continuous virtual memory, and backs
 * the physical memory with said virtual memory
 *
 * NumPages: the number of pages to allocate
 *
 * Returns the starting virtual address of the allocated memory
 */
PTR AllocateBackedPages(USIZE NumPages) {
    PTR VirtAddr = AllocateVirtualPages(NumPages);
    AllocVirtuallyConsecutivePages((USIZE*)GPML4, VirtAddr, NumPages, PAGE_PRESENT | PAGE_WRITE);
    return VirtAddr;
}
