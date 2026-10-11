/* Bootstrap the PFNDB
 *
 * See include/pfndb.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <shill.h>
#include <paging.h>
#include <types.h>
#include <util.h>
#include <balloc.h>
#include <ldkernel.h>

/* Get the virtual address of the end of the direct map (HHDM) mapped memory */
PTR GetEndOfDirectMap(PTR DirectMapOffset) {
    USIZE NumEntries = GMemoryMap->NumEntries;
    USIZE Base = GMemoryMap->Entries[NumEntries-1].PhysicalBase;
    USIZE Size = GMemoryMap->Entries[NumEntries-1].SizePages * PAGE_SIZE;
    return Base + Size + DirectMapOffset;
}

/* Allocate the sparsely physically backed memory for the pfndb.
 * Assumes all entries to be one pointer in size (sizeof(VOID*))
 * as a memdesc-like pfndb should be used.
 *
 * (!) Side effects:
 *      - Maps memory
 *      - Allocates memory
 *
 * Returns the virtual start of the PFNDB
 */
typedef PTR PFNDBEntry; // we do a memdesc-style thing
PTR BootstrapPFNDB(ShillMemoryMap *MemoryMap, PTR DirectMapOffset) {
    ASSERT(KernelBinaryStart);

    PTR PFNDBStart = GetEndOfDirectMap(DirectMapOffset);
    PTR PFNDBEnd   = KernelBinaryStart;
    WriteConsole("PFNDB: %p->%p\n", PFNDBStart, PFNDBEnd);

    PTR PFNDBStartPhys = PFNDBStart - DirectMapOffset;
    PTR SpaceNeeded    = PFNDBStartPhys / PAGE_SIZE * sizeof(PFNDBEntry);
    PTR SpaceAvailable = PFNDBEnd - PFNDBStart;
    ASSERT(SpaceAvailable > SpaceNeeded && "not enough virtual memory for PFNDB bootstrap!");

    PTR LastMappedPageAddr = 0;
    for (USIZE EntryIdx = 0; EntryIdx < MemoryMap->NumEntries; EntryIdx++) {
        ShillMemoryMapEntry Entry = MemoryMap->Entries[EntryIdx];
        if (Entry.Type != SHILL_MEMORY_USABLE && Entry.Type != SHILL_MEMORY_PREBOOT_RECLAIMABLE) continue;
        if (!Entry.SizePages) continue;

        USIZE EntrySizeBytes = Entry.SizePages * PAGE_SIZE;
        PTR PFNStart = Entry.PhysicalBase / PAGE_SIZE;
        PTR PFNEnd   = (Entry.PhysicalBase + EntrySizeBytes + PAGE_SIZE - 1) / PAGE_SIZE;
        PTR VAddrStart = PFNDBStart + ALIGN_DOWN((PTR)(PFNStart * sizeof(PFNDBEntry)), PAGE_SIZE);
        PTR VAddrEnd   = PFNDBStart + ALIGN_DOWN((PTR)(PFNEnd * sizeof(PFNDBEntry) - 1), PAGE_SIZE);

        USIZE Allocated = 0;
        for (PTR VAddr = VAddrStart; VAddr <= VAddrEnd; VAddr += PAGE_SIZE) {
            UNUSED(LastMappedPageAddr);
            if (VAddr < LastMappedPageAddr) continue; // done this page already

            PTR PhysAddr = AllocPhysPage();
            Allocated += PAGE_SIZE;

            MapPage((U64*)GPML4, VAddr, PhysAddr, PAGE_WRITE | PAGE_PRESENT);
            INVALIDATE_ADDR(VAddr);

            SetBuffer((VOID*) VAddr, 0, PAGE_SIZE);
            LastMappedPageAddr = VAddr + PAGE_SIZE;
        }
        ASSERT(Allocated <= EntrySizeBytes);
    }

    WriteConsole("Bootstrapped PFNDB\n");
    return PFNDBStart;
}
