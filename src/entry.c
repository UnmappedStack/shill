/* Entry for the Shill prekernel.
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#define NANOPRINTF_IMPLEMENTATION
#include <ldkernel.h>
#include <paging.h>
#include <balloc.h>
#include <printf.h>
#include <hyper.h>
#include <types.h>
#include <api.h>
#include <hal.h>
#include <balloc.h>

PTR GPML4 = 0;

typedef enum {
    BOOT_PROTOCOL_HYPER,
} BootProtocol;

CSTRING StringifiedProtocols[] = {
    "Hyper Boot Protocol",
};

ProtocolInterface ProtocolInterfaces[] = {
    [BOOT_PROTOCOL_HYPER] = {
        HyperGetDirectMapOffset,
        HyperGetKernelImageStart,
        HyperGetMemoryMap,
        HyperGetPrekernelInfo,
    },
};

/* Detect the boot protocol used from the two arguments which the bootloader
 * passes (or may not pass...) */
BootProtocol DetectBootProtocol(PTR Arg1, PTR Arg2) {
    if (Arg1 && Arg2 == ULTRA_MAGIC) return BOOT_PROTOCOL_HYPER;

    WriteSerial("No valid boot protocol detected, halting\n");
    HaltDevice();
}

/* Dummy function for tinyprintf to use for printing a character to serial. */
VOID PutChar(INT C, VOID *P) {
    (void) P;
    WriteSerialChar(C);
}

CSTRING StringifiedMemoryMapTypes[] = {
    [SHILL_MEMORY_USABLE             ] = "Usable",
    [SHILL_MEMORY_RESERVED           ] = "Reserved",
    [SHILL_MEMORY_INVALID            ] = "Invalid",
    [SHILL_MEMORY_ACPI_RECLAIMABLE   ] = "ACPI reclaimable",
    [SHILL_MEMORY_ACPI_NVS           ] = "ACPI NVS",
    [SHILL_MEMORY_BTLDR_RECLAIMABLE  ] = "Bootloader reclaimable",
    [SHILL_MEMORY_MODULE             ] = "Bootloader module", 
    [SHILL_MEMORY_KERNEL_STACK       ] = "Kernel stack",
    [SHILL_MEMORY_KERNEL_BINARY      ] = "Kernel binary", 
    [SHILL_MEMORY_PREBOOT_RECLAIMABLE] = "Prekernel reclaimable",
};

VOID DumpMemoryMap(ShillMemoryMap *MemoryMap) {
    WriteConsole("  -> Memory map dump:\n");
    for (USIZE Entry = 0; Entry < MemoryMap->NumEntries; Entry++) {
        WriteConsole("       %012p: %09zu pages, %s\n",
                MemoryMap->Entries[Entry].PhysicalBase,
                MemoryMap->Entries[Entry].SizePages,
                StringifiedMemoryMapTypes[MemoryMap->Entries[Entry].Type]);
    }
}

/* Entry point for the prekernel.
 *
 * arg1 and arg2 parameters depend on the bootloader protocol and will be
 * checked. */
VOID BootEntry(PTR Arg1, PTR Arg2) {
    InitSerial();
    WriteConsole("\nEntered Shill\n");

    BootProtocol Protocol = DetectBootProtocol(Arg1, Arg2);
    WriteConsole("Boot protocol detected: %s\n", StringifiedProtocols[Protocol]);

    ShillBootInfoBlock BootInfo = {0};
    BootInfo.DirectMapOffset = ProtocolInterfaces[Protocol].GetDirectMapOffset(Arg1, Arg2);
    BootInfo.MemoryMap       = ProtocolInterfaces[Protocol].GetMemoryMap(Arg1, Arg2);
    InitBootstrapAllocator(BootInfo.MemoryMap);

    WriteConsole("Read boot info:\n"
                 "  -> Direct map offset (HHDM): %p\n",
                 (VOID*) BootInfo.DirectMapOffset);
    DumpMemoryMap(BootInfo.MemoryMap);

    WriteConsole("Switching to new page tree to replace bootloaders...\n");
    ShillPrekernelInfo PrekernelInfo = ProtocolInterfaces[Protocol].GetPrekernelInfo(Arg1, Arg2);
    PTR PML4 = CreateNewAddressSpace(BootInfo.DirectMapOffset, &PrekernelInfo);
    PTR MemoryMapSize = ALIGN_UP(sizeof(ShillMemoryMap) + sizeof(ShillMemoryMapEntry) * BootInfo.MemoryMap->NumEntries, PAGE_SIZE);
    MapConsecutivePages(
            (USIZE*) PML4, 
            (PTR) BootInfo.MemoryMap,
            (PTR) BootInfo.MemoryMap - BootInfo.DirectMapOffset,
            MemoryMapSize,
            PAGE_WRITE | PAGE_PRESENT);
    PTR PML4Virt = PML4 + BootInfo.DirectMapOffset;
    SWITCH_PAGE_TREE(PML4);
    GPML4 = PML4Virt;

    WriteConsole("Loading kernel image...\n");
    PTR KernelEntry = LoadKernel(PML4Virt, ProtocolInterfaces[Protocol].GetKernelImageStart(Arg1, Arg2));

    WriteConsole("Kernel image loaded, entering kernel at entry point %p...\n\n", KernelEntry);
    ((void (*)(ShillBootInfoBlock BootInfo, U32 Magic)) KernelEntry)(BootInfo, SHILL_MAGIC);

    HaltDevice();
}
