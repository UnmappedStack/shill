/* Abstractions from the Ultra boot protocol (Hyper) to the generic API.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <util.h>
#include <paging.h>
#include <valloc.h>
#include <defs.h>
#include <types.h>
#include <hal.h>
#include <hyper.h>
#include <printf.h>

PTR DirectMapOffset = 0;

/* Internal function to get the Hyper attribute of a type.
 * This is a little bit slow (linear search) but it shouldn't be run that often
 * so I doubt it's a real issue.
 *
 * Context: The structure Hyper provides on entry with the
 *          information to search.
 * AttributeType: The type of attribute to search for.
 * 
 * Returns the UltraAttributeHeader which is followed in memory by the data.
 */
STATIC UltraAttributeHeader *UltraGetAttributeOfType(UltraBootContext *Context, U32 AttributeType) {
    UltraAttributeHeader *CurrentHeader = Context->Attributes;
    for (USIZE Entry = 0; Entry < Context->AttributeCount; Entry++) {
        if (CurrentHeader->Type == AttributeType) return CurrentHeader;

        CurrentHeader = ULTRA_NEXT_ATTRIBUTE(CurrentHeader);
    }

    WriteConsole("Searched for invalid attribute: %u\n", AttributeType);
    HaltDevice();
}

/* Get the direct map offset (HHDM) the bootloder provides and return it.
 *
 * (!) Side effect: Modifies global DirectMapOffset with the found HHDM value
 */
PTR HyperGetDirectMapOffset(PTR Context, PTR Magic) {
    UNUSED(Magic);
    UltraBootContext *BootContext = (UltraBootContext*) Context;
    UltraAttributeHeader *AttributeHeader = UltraGetAttributeOfType(BootContext, ULTRA_ATTRIBUTE_PLATFORM_INFO);
    UltraPlatformInfoAttribute *Attribute = CONTAINER_OF(AttributeHeader, UltraPlatformInfoAttribute, Header); 

    DirectMapOffset = Attribute->HigherHalfBase;
    return Attribute->HigherHalfBase;
}

/* Find a memory map entry the same size as or larger than BytesNeeded */
UltraMemoryMapEntry *HyperFindMemoryMapEntryLargeEnough(
        UltraAttributeHeader *MMapHeader,
        PTR BytesNeeded
) {
    USIZE NumEntries = ULTRA_MEMORY_MAP_ENTRY_COUNT(MMapHeader);
    UltraMemoryMapAttribute *MMap = CONTAINER_OF(MMapHeader, UltraMemoryMapAttribute, Header);
    for (USIZE Entry = 1; Entry < NumEntries; Entry++) {
        UltraMemoryMapEntry *EntryData = &MMap->Entries[Entry];
        if (EntryData->Size < BytesNeeded || EntryData->Type != ULTRA_MEMORY_TYPE_FREE) continue;
        return EntryData;
    }
    return NULLPTR;
}

/* Convert a Hyper ULTRA_MEMORY_MAP_TYPE_* value to the Shill abstracted
 * equivalent
 *
 * HyperType: The Hyper version to convert from
 *
 * Returns the Shill equivalent of HyperType
 */
#define HIGH_TYPE_OFFSET 0xffff0000
ShillMemoryMapEntryType HyperToShillMemoryMapType(USIZE HyperType) {
    // Two tables because... well... just take a look at the definitions of the
    // ULTRA_MEMORY_MAP_TYPE_* group in include/hyper.h... WHY, INFY, WHY???
    static const ShillMemoryMapEntryType ConversionTableLow[] = {
        [ULTRA_MEMORY_TYPE_INVALID           ] = SHILL_MEMORY_INVALID,
        [ULTRA_MEMORY_TYPE_FREE              ] = SHILL_MEMORY_USABLE,
        [ULTRA_MEMORY_TYPE_RESERVED          ] = SHILL_MEMORY_RESERVED,
        [ULTRA_MEMORY_TYPE_ACPI_RECLAIMABLE  ] = SHILL_MEMORY_ACPI_RECLAIMABLE,
        [ULTRA_MEMORY_TYPE_ACPI_NVS          ] = SHILL_MEMORY_ACPI_NVS
    };
    static const ShillMemoryMapEntryType ConversionTableHigh[] = {
        [ULTRA_MEMORY_TYPE_LOADER_RECLAIMABLE - HIGH_TYPE_OFFSET] = SHILL_MEMORY_BTLDR_RECLAIMABLE,
        [ULTRA_MEMORY_TYPE_MODULE             - HIGH_TYPE_OFFSET] = SHILL_MEMORY_MODULE,
        [ULTRA_MEMORY_TYPE_KERNEL_STACK       - HIGH_TYPE_OFFSET] = SHILL_MEMORY_KERNEL_STACK,
        [ULTRA_MEMORY_TYPE_KERNEL_BINARY      - HIGH_TYPE_OFFSET] = SHILL_MEMORY_KERNEL_BINARY
    };

    if (HyperType <= ULTRA_MEMORY_TYPE_ACPI_NVS) {
        return ConversionTableLow[HyperType];
    } else if ((HyperType > HIGH_TYPE_OFFSET) &&
            (HyperType <= ULTRA_MEMORY_TYPE_KERNEL_BINARY)) {
        return ConversionTableHigh[HyperType - HIGH_TYPE_OFFSET];
    } else {
        WriteConsole("Tried to convert invalid memory type, halt device\n");
        HaltDevice();
    }
}

/* Get the physical memory map and abstract it into Shill's own structure
 * instead of the bootloader specific one */
ShillMemoryMap *HyperGetMemoryMap(PTR Context, PTR Magic) {
    UNUSED(Magic);
    ASSERT(DirectMapOffset);

    UltraBootContext *BootContext = (UltraBootContext*) Context;
    UltraAttributeHeader *MMapHeader = UltraGetAttributeOfType(BootContext, ULTRA_ATTRIBUTE_MEMORY_MAP); 
    UltraMemoryMapAttribute *MMap = CONTAINER_OF(MMapHeader, UltraMemoryMapAttribute, Header);

    /* technically there could be up to 1 extra entry from splitting the
     * entry where the memory map is stored */
    USIZE NumEntries = ULTRA_MEMORY_MAP_ENTRY_COUNT(MMapHeader);
    PTR BytesNeeded = (NumEntries + 1) * sizeof(ShillMemoryMapEntry) + sizeof(ShillMemoryMap);
    UltraMemoryMapEntry *StoreAtEntry = HyperFindMemoryMapEntryLargeEnough(MMapHeader, BytesNeeded);
    if (!StoreAtEntry) {
        WriteConsole("Could not find a memory map entry large enough to store abstracted memory map!\n"
                     "Bootloader may have provided a *very* fragmented memmap... (Halting device)\n");
        HaltDevice();
    }

    PTR StoreAtEntryPhysAddr = (PTR) StoreAtEntry->PhysicalAddress;
    ShillMemoryMap *AbstractedMemoryMap = (ShillMemoryMap*) (StoreAtEntryPhysAddr + DirectMapOffset);

    /* remove the space we use for the AbstractedMemoryMap from the entry we
     * put it at so that the kernel won't try write over it later */
    StoreAtEntry->PhysicalAddress += ALIGN_UP(BytesNeeded, PAGE_SIZE);
    StoreAtEntry->Size -= ALIGN_UP(BytesNeeded, PAGE_SIZE);

    /* finally we can just move hyper's entries to the new abstracted memory
     * map, yaey! */
    AbstractedMemoryMap->NumEntries = 0;
    for (USIZE Entry = 0; Entry < NumEntries; Entry++) {
        if (MMap->Entries[Entry].PhysicalAddress == StoreAtEntry->PhysicalAddress) {
            // this is the entry *after* where we put the memory map, so we
            // want to first add in an extra prekernel reclaimable region for it
            AbstractedMemoryMap->Entries[Entry].PhysicalBase = (PTR) StoreAtEntryPhysAddr;
            AbstractedMemoryMap->Entries[Entry].SizePages = ALIGN_UP(BytesNeeded, PAGE_SIZE) / PAGE_SIZE;
            AbstractedMemoryMap->Entries[Entry].Type = SHILL_MEMORY_PREBOOT_RECLAIMABLE;
            NumEntries++;
            Entry++;
            // we can then continue with this actual entry
        }
        AbstractedMemoryMap->Entries[Entry].PhysicalBase = MMap->Entries[Entry].PhysicalAddress;
        AbstractedMemoryMap->Entries[Entry].SizePages = ALIGN_DOWN(MMap->Entries[Entry].Size, PAGE_SIZE) / PAGE_SIZE;
        AbstractedMemoryMap->Entries[Entry].Type = HyperToShillMemoryMapType(MMap->Entries[Entry].Type);
        AbstractedMemoryMap->NumEntries++;
    }

    return AbstractedMemoryMap;
}

/* Find the kernel image in memory, from the bootloader's module for it */
ShillModuleInfo HyperGetKernelImageStart(PTR Context, PTR Magic) {
    UNUSED(Magic);
    UltraBootContext *BootContext = (UltraBootContext*) Context;

    // we can't just use UltraGetAttributeOfType as we need an additional check
    // to make sure its the kernel image module and not any module
    UltraAttributeHeader *CurrentHeader = BootContext->Attributes;
    for (USIZE Entry = 0; Entry < BootContext->AttributeCount; Entry++) {
        if (CurrentHeader->Type != ULTRA_ATTRIBUTE_MODULE_INFO) goto Skip;

        UltraModuleInfoAttribute *Attribute = CONTAINER_OF(CurrentHeader, UltraModuleInfoAttribute, Header);
        if (CStringsAreEqual("SHILL_KERNEL_IMAGE_START", Attribute->Name)) {
            ShillModuleInfo Module = {0};
            Module.Address = Attribute->Address + DirectMapOffset;
            Module.SizeBytes = Attribute->Size;
            return Module;
        }
       
Skip:
        CurrentHeader = ULTRA_NEXT_ATTRIBUTE(CurrentHeader);
    }

    WriteConsole("\nKernel image module could not be found!\n"
                 "Make sure your bootloader config file has a module pointing to"
                 " the kernel which should be loaded, and specify the name "
                 "SHILL_KERNEL_IMAGE_START\n\nHalting device\n");
    HaltDevice();
}

/* Get basic information about the prekernel */
ShillPrekernelInfo HyperGetPrekernelInfo(PTR Context, PTR Magic) {
    UNUSED(Magic);
    UltraBootContext *BootContext = (UltraBootContext*) Context;
    UltraAttributeHeader *Header = UltraGetAttributeOfType(BootContext, ULTRA_ATTRIBUTE_KERNEL_INFO); 
    UltraKernelInfoAttribute *PrekernelInfo = CONTAINER_OF(Header, UltraKernelInfoAttribute, Header);

    return (ShillPrekernelInfo) {
        .VirtualBase  = PrekernelInfo->VirtualBase,
        .PhysicalBase = PrekernelInfo->PhysicalBase,
        .SizeBytes    = PrekernelInfo->Size,
    };
}

/* find the rsdp ultra provides */
PTR HyperGetRSDP(PTR Context, PTR Magic) {
    UNUSED(Magic);
    UltraBootContext *BootContext = (UltraBootContext*) Context;
    UltraAttributeHeader *Header = UltraGetAttributeOfType(BootContext, ULTRA_ATTRIBUTE_PLATFORM_INFO); 
    UltraPlatformInfoAttribute *PlatformInfo = CONTAINER_OF(Header, UltraPlatformInfoAttribute, Header);
    return PlatformInfo->RSDPAddress;
}

// ok good news and bad news:
//  - bad news:  the number of framebuffers are not stored anywhere so we need
//    to iterate them all to find the number of framebuffers (:pensive:)
//  - good news: they're all contiguous in memory
// (!) ALSO this will probably get bitrotted away at some point cos infy plans to change
// the api for multiple framebuffers in ultra so yeah good to note
USIZE HyperCountFramebuffers(UltraBootContext *Context) {
    USIZE NumFramebuffers = 0;
    UltraAttributeHeader *Header = UltraGetAttributeOfType(Context, ULTRA_ATTRIBUTE_FRAMEBUFFER_INFO);
    for (USIZE Entry = 0; Entry < Context->AttributeCount; Entry++) {
        if (Header->Type != ULTRA_ATTRIBUTE_FRAMEBUFFER_INFO) break;
        NumFramebuffers++;
        Header = ULTRA_NEXT_ATTRIBUTE(Header);
    }
    return NumFramebuffers;
}

/* get all framebuffers and create a ShillFramebuffersList for them.
 *
 * I don't love that we need to iterate over the framebuffers twice,
 * but its necessary because Ultra boot protocol doesn't (yet) provide
 * a count of total framebuffers. this might change at some point afaik
 * though luckily */
ShillFramebuffersList *HyperGetFramebuffers(PTR Context, PTR Magic) {
    UNUSED(Magic);

    UltraBootContext *BootContext = (UltraBootContext*) Context;
    USIZE NumFramebuffers = HyperCountFramebuffers(BootContext);
    USIZE BytesNeeded = sizeof(ShillFramebuffersList) + sizeof(ShillFramebuffer) * NumFramebuffers;
    ShillFramebuffersList *Framebuffers = (VOID*) AllocateBackedPages(
            ALIGN_UP(BytesNeeded, PAGE_SIZE) / PAGE_SIZE,
            VALLOC_ZONE_DEFAULT // lower half in virtual memory
    );

    UltraAttributeHeader *Header = UltraGetAttributeOfType(BootContext, ULTRA_ATTRIBUTE_FRAMEBUFFER_INFO);
    for (USIZE Entry = 0; Entry < BootContext->AttributeCount; Entry++) {
        if (Header->Type != ULTRA_ATTRIBUTE_FRAMEBUFFER_INFO) break;

        // shill uses pretty much the same framebuffer api as ultra, so we can literally just copy it
        ShillFramebuffer *NewFramebuffer = (ShillFramebuffer*)&CONTAINER_OF(Header, UltraFramebufferAttribute, Header)->FB;
        Framebuffers->Framebuffers[Entry] = *NewFramebuffer;
        USIZE FbSize = NewFramebuffer->Width * NewFramebuffer->Height * NewFramebuffer->Pitch;
        USIZE FbPages = ALIGN_UP(FbSize, PAGE_SIZE) / PAGE_SIZE;
        MapConsecutivePages(
                (USIZE*)GPML4,                                     /* page tree */
                NewFramebuffer->PhysicalAddress + DirectMapOffset, /* virt addr */
                NewFramebuffer->PhysicalAddress,                   /* phys addr */
                FbPages,                                           /* size in pages */
                PAGE_WRITE | PAGE_PRESENT | PAGE_WC);
        INVALIDATE_RANGE(NewFramebuffer->PhysicalAddress + DirectMapOffset, FbPages);

        Header = ULTRA_NEXT_ATTRIBUTE(Header);
    }

    Framebuffers->NumFramebuffers = NumFramebuffers;
    return Framebuffers;
}
