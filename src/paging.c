/* Setting up a page tree to replace the bootloader's, until the kernel
 * creates its own.
 *
 * See include/paging.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <balloc.h>
#include <types.h>
#include <util.h>
#include <balloc.h>

#define TABLE_FROM_VADDR(vaddr, tlevel) (((vaddr) >> (VOFF+9*(tlevel-1))) & 511)

STATIC PTR GDirectMapOffset = 0;

/* Try create the next layer of a page tree for a given index, or create it if
 * it doesn't already exist. Probably only used by MapPage
 *
 * Side effects: 
 *      - may modify page tree in ParentLayer
 *      - physical memory may be allocated
 *
 * ParentLayer: The layer of the paging radix tree to get/create at index Index
 * Index: The index of the ParentLayer to get/create
 *
 * Returns a pointer to the page entry which refers to the next layer
 */
USIZE *GetOrCreateNextLayer(USIZE *ParentLayer, USIZE Index) {
    /* if the index of a page tree level needed does not already exist,
     * make it with full permissions, and allocate for the next level's table to
     * be used for further child tables. */
    if (!ParentLayer[Index]) {
        PTR ChildPhysAddr = AllocPhysPage();
        CopyBuffer((void*)(ChildPhysAddr + GDirectMapOffset), 0, PAGE_SIZE);

        ParentLayer[Index] = PAGE_TABLE_ENTRY(
                                 ChildPhysAddr,
                                 INNER_NODE_FLAGS
                             );
    }
    // once we know it exists, we can return it
    PTR PhysAddr = PADDR_FROM_TABLE_ENTRY((PTR) ParentLayer[Index]);
    return (USIZE*) (PhysAddr + GDirectMapOffset);
}

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
void MapPage(USIZE *PML4VirtAddr, PTR VirtAddr, PTR PhysAddr, USIZE Flags) {
    VirtAddr &= ~0xFFFF000000000000ULL; /* must be canonical */

    USIZE *CurrentLayerVirtAddr = PML4VirtAddr;
    for (U8 PMLLevel = 4; PMLLevel > 1; PMLLevel --) {
        CurrentLayerVirtAddr = GetOrCreateNextLayer(
                                   CurrentLayerVirtAddr,
                                   TABLE_FROM_VADDR(VirtAddr, PMLLevel)
                               );
    }

    /* now that we've actually got the pml1 table and the offset into it, we can
     * just add the mapping to the page tree */
    CurrentLayerVirtAddr[TABLE_FROM_VADDR(VirtAddr, 1)] = PAGE_TABLE_ENTRY(PhysAddr, Flags);
}

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
void MapConsecutivePages(USIZE *PML4, PTR VirtAddrStart, PTR PhysAddrStart,
                           USIZE NumPages, USIZE Flags) {
    // This could probably be faster, but I feel like this is the more readable implementation
    for (USIZE Offset = 0; Offset < NumPages; Offset++)
        MapPage(PML4, VirtAddrStart + Offset * PAGE_SIZE, PhysAddrStart + Offset * PAGE_SIZE, Flags);
}

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
void AllocVirtuallyConsecutivePages(USIZE *PML4, PTR VirtAddrStart, USIZE NumPages, USIZE Flags) {
    for (USIZE Offset = 0; Offset < NumPages; Offset++) {
        PTR PhysPage = AllocPhysPage();
        MapPage(PML4, VirtAddrStart + Offset * PAGE_SIZE, PhysPage, Flags);
    }
}

/* maps all memory that could be used into a virtual memory space
 *
 * Side effects:
 *      - modifies PML4 with the new mappings
 *      - allocates physical memory
 *
 * PML4: the virtual address of the root of the page tree to map into
 */
void MapAllMemoryIntoVMemSpace(USIZE *PML4) {
    ShillMemoryMapEntry *Entries = GMemoryMap->Entries;
    USIZE NumEntries = GMemoryMap->NumEntries;
    for (USIZE Entry = 0; Entry < NumEntries; Entry++) {
        PTR PhysAddr = Entries[Entry].PhysicalBase;
        PTR VirtAddr = Entries[Entry].PhysicalBase + GDirectMapOffset;
        ShillMemoryMapEntryType Type = Entries[Entry].Type;
        if (Type == SHILL_MEMORY_INVALID || Type == SHILL_MEMORY_RESERVED) continue;
        MapConsecutivePages(PML4, VirtAddr, PhysAddr, Entries[Entry].SizePages, PAGE_PRESENT | PAGE_WRITE);
    }
}

/* Creates a new address space and maps essential memory into it
 *
 * Side effects: writes DirectMapOffset to GDirectMapOffset global
 *
 * DirectMapOffset: The HHDM used *before* the new page tree
 *
 * Returns the physical address of the new page tree pml4 */
PTR CreateNewAddressSpace(PTR DirectMapOffset) {
    GDirectMapOffset = DirectMapOffset;
    PTR PML4PhysAddr = AllocPhysPage();
    USIZE *PML4VirtAddr = (USIZE*) (PML4PhysAddr + GDirectMapOffset);
    CopyBuffer(PML4VirtAddr, 0, PAGE_SIZE);

    MapAllMemoryIntoVMemSpace(PML4VirtAddr);
   
    return PML4PhysAddr;
}
