/* Abstractions from the Limine boot protocol to the generic API.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <balloc.h>
#include <valloc.h>
#include <paging.h>
#include <strings.h>
#include <limine.h>
#include <types.h>
#include <util.h>

STATIC PTR DirectMapOffset = 0;

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

#define TODO(X) \
    do { \
        WriteConsole("TODO: %s\n", X); \
        HaltDevice(); \
    } while (0)

LIMINE_REQUEST(HHDMRequest, hhdm, LIMINE_HHDM_REQUEST_ID);
LIMINE_REQUEST(MemoryMapRequest, memmap, LIMINE_MEMMAP_REQUEST_ID);
LIMINE_REQUEST(RSDPRequest, rsdp, LIMINE_RSDP_REQUEST_ID);
LIMINE_REQUEST(ExecutableRequest, executable_address, LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID);
LIMINE_REQUEST(FramebufferRequest, framebuffer, LIMINE_FRAMEBUFFER_REQUEST_ID);
LIMINE_REQUEST(ModuleRequest, module, LIMINE_MODULE_REQUEST_ID);

/* Get the direct map offset (HHDM) the bootloder provides and return it.
 *
 * (!) Side effect: Modifies global DirectMapOffset with the found HHDM value
 */
PTR LimineGetDirectMapOffset(PTR Context, PTR Magic) {
    UNUSED(Context), UNUSED(Magic);
    DirectMapOffset = HHDMRequest.Response->Offset;
    return DirectMapOffset;
}

/* Find a memory map entry the same size as or larger than BytesNeeded */
LimineMemoryMapEntry *LimineFindMemoryMapEntryLargeEnough(
        LimineMemoryMapResponse *MMap,
        PTR BytesNeeded
) {
    for (USIZE EntryIdx = 0; EntryIdx < MMap->EntryCount; EntryIdx++) {
        LimineMemoryMapEntry *Entry = MMap->Entries[EntryIdx];
        if (Entry->Length < BytesNeeded || Entry->Type != LIMINE_MEMMAP_USABLE)
            continue;
        return Entry;
    }
    return NULLPTR;
}

/* Convert a Limine memory map type value to the Shill abstracted
 * equivalent
 *
 * LimineType: The Limine version to convert from
 *
 * Returns the Shill equivalent of LimineType
 */
ShillMemoryMapEntryType LimineToShillMemoryMapType(USIZE LimineType) {
    ShillMemoryMapEntryType Table[] = {
        [LIMINE_MEMMAP_USABLE                ] = SHILL_MEMORY_USABLE,
        [LIMINE_MEMMAP_RESERVED              ] = SHILL_MEMORY_RESERVED,
        [LIMINE_MEMMAP_ACPI_RECLAIMABLE      ] = SHILL_MEMORY_ACPI_RECLAIMABLE,
        [LIMINE_MEMMAP_ACPI_NVS              ] = SHILL_MEMORY_ACPI_NVS,
        [LIMINE_MEMMAP_BAD_MEMORY            ] = SHILL_MEMORY_INVALID,
        [LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE] = SHILL_MEMORY_BTLDR_RECLAIMABLE,
        [LIMINE_MEMMAP_EXECUTABLE_AND_MODULES] = SHILL_MEMORY_PREBOOT_RECLAIMABLE,
        [LIMINE_MEMMAP_FRAMEBUFFER           ] = SHILL_MEMORY_RESERVED,
        [LIMINE_MEMMAP_RESERVED_MAPPED       ] = SHILL_MEMORY_ACPI_RECLAIMABLE,
    };
    return Table[LimineType];
}

/* Get the physical memory map and abstract it into Shill's own structure
 * instead of the bootloader specific one */
ShillMemoryMap *LimineGetMemoryMap(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);

    LimineMemoryMapResponse *MemoryMap = MemoryMapRequest.Response;
    USIZE NumEntries = MemoryMap->EntryCount;
    USIZE BytesNeeded = (NumEntries + 2) * sizeof(ShillMemoryMapEntry) + sizeof(ShillMemoryMap);
    LimineMemoryMapEntry *StoreAtEntry = LimineFindMemoryMapEntryLargeEnough(MemoryMap, BytesNeeded);
    if (!StoreAtEntry) {
        WriteConsole(NO_MEMMAP_ENTRY_BIG_ENOUGH);
        HaltDevice();
    }

    PTR StoreAtEntryPhysAddr = (PTR) StoreAtEntry->Base;
    ShillMemoryMap *AbstractedMemoryMap = (ShillMemoryMap*) (StoreAtEntryPhysAddr + DirectMapOffset);

    /* remove the space we use for the AbstractedMemoryMap from the entry we
     * put it at so that the kernel won't try write over it later */
    StoreAtEntry->Base += ALIGN_UP(BytesNeeded, PAGE_SIZE);
    StoreAtEntry->Length -= ALIGN_UP(BytesNeeded, PAGE_SIZE);

    /* finally we can just move hyper's entries to the new abstracted memory */
    AbstractedMemoryMap->NumEntries = 0;
    for (USIZE Entry = 0; Entry < NumEntries; Entry++) {
        if (MemoryMap->Entries[0][Entry].Base == StoreAtEntry->Base) {
            // this is the entry *after* where we put the memory map, so we
            // want to first add in an extra prekernel reclaimable region for it
            AbstractedMemoryMap->Entries[Entry].PhysicalBase = (PTR) StoreAtEntryPhysAddr;
            AbstractedMemoryMap->Entries[Entry].SizePages = ALIGN_UP(BytesNeeded, PAGE_SIZE) / PAGE_SIZE;
            AbstractedMemoryMap->Entries[Entry].Type = SHILL_MEMORY_PREBOOT_RECLAIMABLE;
            NumEntries++;
            Entry++;
            // we can then continue with this actual entry
        }
        AbstractedMemoryMap->Entries[Entry].PhysicalBase = MemoryMap->Entries[0][Entry].Base;
        AbstractedMemoryMap->Entries[Entry].SizePages = ALIGN_DOWN(MemoryMap->Entries[0][Entry].Length, PAGE_SIZE) / PAGE_SIZE;
        AbstractedMemoryMap->Entries[Entry].Type = LimineToShillMemoryMapType(MemoryMap->Entries[0][Entry].Type);
        AbstractedMemoryMap->NumEntries++;
    }
    
    return AbstractedMemoryMap;
}

/* Find the kernel image in memory, from the bootloader's module for it */
ShillModuleInfo LimineGetKernelImageStart(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetKernelImageStart for this protocol");
}

/* Get basic information about the prekernel */
ShillPrekernelInfo LimineGetPrekernelInfo(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    LimineExecutableResponse *ExecutableResponse = ExecutableRequest.Response;
    return (ShillPrekernelInfo) {
        .VirtualBase = ExecutableResponse->VirtualBase,
        .PhysicalBase = ExecutableResponse->PhysicalBase,
    };
}

/* find the rsdp the bootloader provides */
PTR LimineGetRSDP(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    LimineRSDPResponse *RSDPResponse = RSDPRequest.Response;
    return (PTR) RSDPResponse->Address;
}

/* get all framebuffers and create a ShillFramebuffersList for them. */
ShillFramebuffersList *LimineGetFramebuffers(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);

    LimineFramebufferResponse *FramebuffersResponse = FramebufferRequest.Response;
    USIZE NumFramebuffers = FramebuffersResponse->FramebufferCount;
    USIZE BytesNeeded = sizeof(ShillFramebuffersList) + sizeof(ShillFramebuffer) * NumFramebuffers;
    ShillFramebuffersList *Framebuffers = (VOID*) AllocateBackedPages(
            ALIGN_UP(BytesNeeded, PAGE_SIZE) / PAGE_SIZE,
            VALLOC_ZONE_DEFAULT // lower half in virtual memory
    );
    INVALIDATE_RANGE((PTR)Framebuffers, ALIGN_UP(BytesNeeded, PAGE_SIZE) / PAGE_SIZE);

    for (USIZE Entry = 0; Entry < NumFramebuffers; Entry++) {
        LimineFramebuffer *OldFramebuffer = FramebuffersResponse->Framebuffers[Entry];
        ShillFramebuffer NewFramebuffer = (ShillFramebuffer) {
            .Width  = OldFramebuffer->Width,
            .Height = OldFramebuffer->Height,
            .Pitch  = OldFramebuffer->Pitch,
            .BPP    = OldFramebuffer->BPP,
            .Format = SHILL_FB_FORMAT_RGB888,
            .PhysicalAddress = (PTR) OldFramebuffer->Address - DirectMapOffset,
        };
        Framebuffers->Framebuffers[Entry] = NewFramebuffer;

        USIZE FbSize = NewFramebuffer.Width * NewFramebuffer.Height * NewFramebuffer.Pitch;
        USIZE FbPages = ALIGN_UP(FbSize, PAGE_SIZE) / PAGE_SIZE;

        /* technically limine maps stuff in for us, but it needs to be done in
         * the replacement page tree too */
        MapConsecutivePages(
                (USIZE*)GPML4,                                    /* page tree */
                NewFramebuffer.PhysicalAddress + DirectMapOffset, /* virt addr */
                NewFramebuffer.PhysicalAddress,                   /* phys addr */
                FbPages,                                          /* size in pages */
                PAGE_WRITE | PAGE_PRESENT | PAGE_WC);
        INVALIDATE_RANGE(NewFramebuffer.PhysicalAddress + DirectMapOffset, FbPages);
    }
    
    Framebuffers->NumFramebuffers = NumFramebuffers;
    return Framebuffers;
}

/* get all modules (except for the one containing the kernel binary) and decompress
 * it if necessary, returning a ShillModulesList */
ShillModulesList *LimineGetModules(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);

    LimineModuleResponse *ModuleResponse = ModuleRequest.Response;
    ASSERT(ModuleResponse && "you probably didn't give a module for the kernel image");
    USIZE NumModules = ModuleResponse->ModuleCount;

    USIZE BytesNeeded = sizeof(ShillModulesList) + sizeof(ShillModule) * (NumModules-1);
    ShillModulesList *Modules = (VOID*) AllocateBackedPages(
            ALIGN_UP(BytesNeeded, PAGE_SIZE) / PAGE_SIZE,
            VALLOC_ZONE_DEFAULT // lower half in virtual memory
    );

    INT Offset = 0;
    for (USIZE Entry = 0; Entry < NumModules; Entry++) {
        LimineFile *ModuleInfo = ModuleResponse->Modules[Entry];

        if (CStringsAreEqual("SHILL_KERNEL_IMAGE_START", ModuleInfo->String)) {
            if (Offset == -1) {
                WriteConsole("Multiple modules referencing the kernel binary found!\n");
                HaltDevice();
            }
            Offset = -1;
            continue;
        }

        ShillModuleInfo Bounds = {(PTR)ModuleInfo->Address, ModuleInfo->Size};
        if (VerifyGZ(Bounds.Address)) {
            Bounds = DecompressGZ(&Bounds);
        }
        
        CopyString(Modules->Modules[Entry+Offset].Name, (CHAR*)ModuleInfo->String);
        Modules->Modules[Entry+Offset].SizeBytes = Bounds.SizeBytes;
        Modules->Modules[Entry+Offset].Address   = Bounds.Address;
    }
    ASSERT(Offset == -1);

    Modules->NumModules = NumModules - 1;
    return Modules;
}
