/* bootstrap virtual memory allocator, just bump
 *
 * See /src/valloc.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <types.h>

/* allocates NumPages virtual pages *but does not map them in*
 *
 * Side effects: adds to VirtualBumpCurrent
 *
 * NumPages: the number of pages to allocate
 *
 * Returns the starting address of the allocated memory
 */
PTR AllocateVirtualPages(USIZE NumPages);

/* allocates physical memory, allocates continuous virtual memory, and backs
 * the physical memory with said virtual memory
 *
 * NumPages: the number of pages to allocate
 *
 * Returns the starting virtual address of the allocated memory
 */
PTR AllocateBackedPages(USIZE NumPages);

/* like AllocateBackedPages but the virtual memory its backed to is in the
 * region for kernel stacks */
PTR AllocateBackedStack(VOID);
