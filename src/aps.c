/* Startup for APs
 *
 * See include/aps.h
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 *
 * TODO: this could probably use less arch specific code and allocate less unnecessary
 * extra memory (remember the prekernel uses a bootstrap bump allocator so unless the
 * kernel does so, it will not be freed!)
 */

#include <acpi.h>
#include <paging.h>
#include <printf.h>
#include <balloc.h>
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

// all of this should be moved to another header and made less arch specific!!! (TODO)
typedef struct {
    U16 Size;
    U32 Address;
} PACKED GDTR;
extern GDTR APgdtr;
extern U64 APgdt[];
extern U32 FarjumpTrampoline32;
extern U32 FarjumpTrampoline64;
extern U32 APcr3;
extern VOID APTrampolineStart(VOID);
extern VOID APTrampolineEnd(VOID);
extern VOID EnterLongMode(VOID);
extern VOID LongModeStub(VOID);
extern PTR APCEntry;
extern VOID APEntry(USIZE SoftwareId); // in entry.c, TODO move to a header
VOID SetupTrampoline(PTR DirectMapOffset) {
    UNUSED(DirectMapOffset); // TODO; remove if it ends up being unnecessary
    ASSERT(FirstPagesLower[0]);
    ASSERT(APTrampolineEnd < APTrampolineStart + PAGE_SIZE);

    // identity map the lowest pages which will contain the trampoline
    for (U8 Pg = 0; Pg < 2; Pg++) {
        MapPage(
                (USIZE*)GPML4,            // page tree target
                FirstPagesLower[Pg],      // virt addr
                FirstPagesLower[Pg],      // phys addr
                PAGE_PRESENT | PAGE_WRITE // flags
        );
        INVALIDATE_ADDR(FirstPagesLower[Pg]);
    }
    
    // prolly not good to have this x86 stuff here, TODO move out
    // I'll probably do it anyways tho when the second arch arrives
    #define RELOCATE(Old) (FirstPagesLower[0] + ((PTR)(Old) - (PTR)APTrampolineStart))
    APgdtr.Address = (U32) RELOCATE(APgdt);
    APcr3 = FirstPagesLower[1];
    FarjumpTrampoline32 = (U32) RELOCATE(EnterLongMode);
    FarjumpTrampoline64 = (U32) RELOCATE(LongModeStub);
    APCEntry = (PTR) APEntry;

    CopyBuffer((VOID*)FirstPagesLower[0], APTrampolineStart, PAGE_SIZE);
    CopyBuffer((VOID*)FirstPagesLower[1], (VOID*)GPML4, PAGE_SIZE);
}

extern PTR *APStackList; // for asm to access, only written to by StartAPs
VOID StartAPs(PTR DirectMapOffset, MADT *MADTBlock, ShillProcessorsList *Processors) {
    // map in the madt
    MapPage(
            (USIZE*)GPML4,                              // page tree target
            MADTBlock->LocalAPICAddr + DirectMapOffset, // virt addr
            MADTBlock->LocalAPICAddr,                   // phys addr
            PAGE_PRESENT | PAGE_WRITE                   // flags
    );
    INVALIDATE_ADDR(MADTBlock->LocalAPICAddr + DirectMapOffset);

    // we also need to create a list of stack addresses for each AP
    USIZE StackListNumPages = ALIGN_UP(Processors->NumProcessors * sizeof(PTR),
                                  PAGE_SIZE) / PAGE_SIZE;
    APStackList = (PTR*) AllocateBackedPages(
            StackListNumPages,
            VALLOC_ZONE_STACK
    );
    INVALIDATE_RANGE(APStackList, StackListNumPages);

    SetupTrampoline(DirectMapOffset);

    USIZE Idx = 0;
    for (USIZE Cpu = 0; Cpu < Processors->NumProcessors; Cpu++) {
        USIZE Id = Processors->Processors[Cpu].ID;
        if (!Id) continue; // not bsp
        APStackList[Idx] = AllocateBackedPages(KERNEL_STACK_PAGES, VALLOC_ZONE_STACK);
        ArchStartAP(MADTBlock->LocalAPICAddr + DirectMapOffset, Id);
        Idx++;
    }
}
