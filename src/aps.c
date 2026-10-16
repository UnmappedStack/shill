/* Startup for APs
 *
 * See include/aps.h
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <acpi.h>
#include <printf.h>
#include <valloc.h>
#include <shill.h>
#include <util.h>
#include <aps.h>

// sadly we need to iterate over them twice
STATIC USIZE CountAPs(MADT *MADTBlock) {
    MADTEntryHeader *Entry = (VOID*)((PTR)MADTBlock + sizeof(MADT));
    USIZE Offset = sizeof(MADT);

    USIZE Count = 0;
    while (Offset < MADTBlock->Header.Length) {
        if (Entry->EntryType == LOCAL_APIC) {
            Count++;
        }

        Entry = (VOID*)((PTR)Entry + Entry->RecordLength);
        Offset += Entry->RecordLength;
    }

    return Count;
}

ShillProcessorsList *FindAPs(MADT *MADTBlock) {
    USIZE NumAPs = CountAPs(MADTBlock);

    ShillProcessorsList *Processors = (VOID*)AllocateBackedPages(
        sizeof(ShillProcessorsList) + NumAPs * sizeof(ShillProcessor),
        VALLOC_ZONE_DEFAULT // lower half
    );
    ASSERT(Processors);
    Processors->NumProcessors = NumAPs;
   
    // iterate the madt a second time to fill it in (*sigh*)
    MADTEntryHeader *Entry = (VOID*)((PTR)MADTBlock + sizeof(MADT));
    USIZE Offset = sizeof(MADT);
    USIZE Idx = 0;
    while (Offset < MADTBlock->Header.Length) {
        if (Entry->EntryType == LOCAL_APIC) {
            ProcessorLocalAPIC *LocalAPIC = (VOID*) Entry;
            Processors->Processors[Idx] = (ShillProcessor){
                .ID = LocalAPIC->APICID,
            };
            WriteConsole(" -> Find processor of LAPIC ID %zu\n", LocalAPIC->APICID);
            Idx++;
        }

        Entry = (VOID*)((PTR)Entry + Entry->RecordLength);
        Offset += Entry->RecordLength;
    }

    return Processors;
}
