/* Bootstrap allocator for the Shill prekernel.
 *
 * Nothing in the prekernel should
 * really need to be freed I figure, at least until the kernel reclaims it, so
 * I think it should be fine for it to just be a bump allocator from th
 * largest memory map region.
 * 
 * See include/balloc.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <balloc.h>
#include <types.h>
#include <api.h>
#include <util.h>
#include <hal.h>
#include <printf.h>

STATIC ShillMemoryMapEntry *BumpCurrentRegion = NULLPTR;
STATIC PTR BumpCurrentOffset = 0; // (in pages)
ShillMemoryMap *GMemoryMap = NULLPTR;

/* Find the first usable region of the memory map and set it
 * as the start of the bump allocator
 *
 * Side effects: Writes to global variables BumpCurrentRegion and BumpCurrOffset,
 *               both with the starting physical address of the new bump allocator
 *
 * MemoryMap: A pointer to the boot protocol agnostic memory
 *            map structure
 */
VOID InitBootstrapAllocator(ShillMemoryMap *MemoryMap) {
    GMemoryMap = MemoryMap;
    for (USIZE EntryIdx = 0; EntryIdx < MemoryMap->NumEntries; EntryIdx++) {
        ShillMemoryMapEntry *Entry = &MemoryMap->Entries[EntryIdx];

        if (Entry->Type == SHILL_MEMORY_USABLE) {
            Entry->Type = SHILL_MEMORY_PREBOOT_RECLAIMABLE;
            BumpCurrentRegion = Entry;
            BumpCurrentOffset = 0; /* technically should already be zero but */
                                   /* can't hurt to be safe i guess lol      */
            if (Entry->PhysicalBase == 0) BumpCurrentOffset++;
            return;
        }
    }

    WriteConsole("No valid bumpable entry found from memory map, halting device\n");
    HaltDevice();
}

/* Allocate a single physical page
 *
 * Side effects:
 *      - Can mark regions of the memory map as prekernel reclaimable
 *      - Can increment BumpCurrentRegion
 *      - Can increment or reset BumpCurrentOffset
 *
 *  Returns the physical address of the page allocated
 */
PTR AllocPhysPage(VOID) {
    ASSERT(BumpCurrentRegion && GMemoryMap);

    if (BumpCurrentOffset + 1 > BumpCurrentRegion->SizePages) {
        BOOL FoundRegion = FALSE;
        for (USIZE EntryIdx = 0; EntryIdx < GMemoryMap->NumEntries; EntryIdx++) {
            ShillMemoryMapEntry *Entry = &GMemoryMap->Entries[EntryIdx];
    
            if (Entry->Type != SHILL_MEMORY_USABLE || Entry->SizePages <= 1) continue;

            // found a valid region
            BumpCurrentRegion = Entry;
            BumpCurrentOffset = 0;
            Entry->Type = SHILL_MEMORY_PREBOOT_RECLAIMABLE;
            FoundRegion = TRUE;
            break;
        }
        
        if (!FoundRegion) {
            WriteConsole("Shill is out of memory!\n");
            HaltDevice();
        }
    }

    return BumpCurrentRegion->PhysicalBase + ((BumpCurrentOffset++) * PAGE_SIZE);
}
