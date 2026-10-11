/* Bootstrap allocator for the Shill prekernel.
 *
 * Nothing in the prekernel should really need to be freed I figure, at least
 * until the kernel reclaims it, so I think it should be fine for it to just
 * be a bump allocator.
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

/* Called when all bootstrap allocations are done and we can snap off the last
 * region we've been allocating so that the whole thing isn't marked prekernel
 * reclaimable */
VOID FinishAllocations(VOID) {
    ASSERT(BumpCurrentRegion); // I'd be surprised if this fails lol

    // find the index of the one we're on
    USIZE CurrentIndex;
    for (USIZE EntryIdx = 0; EntryIdx < GMemoryMap->NumEntries; EntryIdx++) {
        if (GMemoryMap->Entries[EntryIdx].Type == SHILL_MEMORY_PREKERNEL_BINARY) {
            // the kernel is free to use it for whatever cos the prekernel
            // will not be in use anymore
            GMemoryMap->Entries[EntryIdx].Type = SHILL_MEMORY_USABLE;
        }
        if (&GMemoryMap->Entries[EntryIdx] != BumpCurrentRegion) continue;
        CurrentIndex = EntryIdx;
        goto Found;
    }
    WriteConsole("Tried FinishAllocations but got invalid BumpCurrentRegion\n");
    HaltDevice();

Found:
    /* we should be making space for the extra entry in hyper.c or wherever the
     * boot protocol specific stuff lives. TODO: maybe centralise it somewhere */
    USIZE NextIndex = CurrentIndex + 1;
    if (CurrentIndex != GMemoryMap->NumEntries) {
        // Everything gets moved one entry forward to make space for the new entry
        MoveBuffer(
                &GMemoryMap->Entries[NextIndex + 1],
                &GMemoryMap->Entries[NextIndex],
                (GMemoryMap->NumEntries - NextIndex) * sizeof(ShillMemoryMapEntry)
        );
    }

    GMemoryMap->NumEntries++;

    USIZE PrevSize = GMemoryMap->Entries[CurrentIndex].SizePages;
    GMemoryMap->Entries[CurrentIndex].SizePages = BumpCurrentOffset;
    USIZE NewSize = GMemoryMap->Entries[CurrentIndex].SizePages;

    GMemoryMap->Entries[NextIndex] = (ShillMemoryMapEntry) {
        .PhysicalBase = GMemoryMap->Entries[CurrentIndex].PhysicalBase + NewSize * PAGE_SIZE,
        .SizePages    = PrevSize - NewSize,
        .Type         = SHILL_MEMORY_USABLE,
    };

    // AllocPhysPage will assert on this so if we try allocate again it'll fail now
    BumpCurrentRegion = 0;
}
