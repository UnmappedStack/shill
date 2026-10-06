/* Setting up a page tree to replace the bootloader's, until the kernel
 * creates its own.
 *
 * See src/paging.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <types.h>
#include <api.h>

/* Creates a new address space and maps essential memory into it
 *
 * Side effects: writes DirectMapOffset to GDirectMapOffset global
 *
 * DirectMapOffset: The HHDM used *before* the new page tree
 * PrekernelInfo: a struct of the virtual/physical location of the prekernel
 *
 * Returns the physical address of the new page tree pml4 */
PTR CreateNewAddressSpace(PTR DirectMapOffset, ShillPrekernelInfo *PrekernelInfo);

/* allocates a bunch of non-consecutive physical pages and strings them 
 * together in virtual memory
 *
 * Side effects:
 *      - PML4 will be modified with the new mappings
 *      - physical memory will be allocated
 *
 * PML4VirtAddr: the virtual address of the page tree to map into
 * VirtAddrStart: the virtual address to start mapping from
 * NumPages: the number of pages from VirtAddrStart to map
 * Flags: the MMU flags to use
 */
VOID AllocVirtuallyConsecutivePages(U64 *PML4, PTR VirtAddrStart, U64 NumPages, U64 Flags);

/* Map a range of virtual addresses to physical addresses
 *
 * Side effects:
 *      - modifies the page tree in PML4VirtAddr
 *      - physical memory may be allocated
 *
 * PML4VirtAddr: the virtual address of the page tree to map it in
 * VirtAddrStart: the virtual address to start mapping to PhysAddr from
 * PhysAddrStart: the physical address to start mapping to VirtAddr from
 * NumPages: the number of pages, starting from VirtAddrStart/PhysAddrStart, to map
 * Flags: the MMU flags to use
 */
VOID MapConsecutivePages(U64 *PML4, PTR VirtAddrStart, PTR PhysAddrStart,
                           U64 NumPages, U64 Flags);

/* Map a virtual page to a physical page
 *
 * Side effects:
 *      - modifies the page tree in PML4VirtAddr
 *      - physical memory may be allocated
 *
 * PML4VirtAddr: the virtual address of the page tree to map it in
 * VirtAddr: the virtual address to map to PhysAddr
 * PhysAddr: the physical address to map to VirtAddr
 * Flags: the MMU flags to use
 */
VOID MapPage(U64 *PML4VirtAddr, PTR VirtAddr, PTR PhysAddr, U64 Flags);
