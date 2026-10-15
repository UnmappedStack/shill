/* Loads the ELF binary of the kernel.
 * 
 * See /include/ldkernel.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

/* TODO: make sure the kernel is being loaded *lower* than the prekernel, not
 * overlapping or above it! (not allowing being above it is necessary for the
 * pfndb boostrap)
 *
 * I haven't tested, but I would assume that right now loading the kernel too high
 * would probably fuck things up silently...
 */

#define KGZ_IMPLEMENTATION
#include <kgz.h>

#include <valloc.h>
#include <api.h>
#include <hal.h>
#include <util.h>
#include <printf.h>
#include <paging.h>
#include <ldkernel.h>

PTR KernelBinaryStart = 0;

#define ELF_LOADABLE (1)
#define ELF_WRITABLE (2)

typedef struct {
    UBCHAR Id[16];
    U16 Type;
    U16 MachineType;
    U32 Version;
    U64 Entry;
    U64 ProgramHeaderOffset;
    U64 SectionHeaderOffset;
    U32 Flags;
    U16 HeaderSize;
    U16 ProgramHeaderEntrySize;
    U16 ProgramHeaderEntryCount;
    U16 SectionHeaderEntrySize;
    U16 SectionHeaderEntryCount;
    U16 SectionNameStringTableIndex;
} __attribute__((packed)) ElfFileHeader;

typedef struct {
    U32 Type;
    U32 Flags;
    U64 Offset;
    U64 VirtualAddress;
    U64 Rsvd;
    U64 SizeInFile;
    U64 SizeInMemory;
    U64 Align;
} __attribute__((packed)) ElfProgramHeader;

/* Check that an ELF header is valid to load
 *
 * FileHeader: The address in memory of the ElfFileHeader to check
 *
 * Returns TRUE if valid and loadable, otherwise FALSE.
 */
BOOL VerifyElf(ElfFileHeader *FileHeader) {
    if (!BuffersAreEqual(FileHeader->Id, "\x7f" "ELF", 4)) {
        WriteConsole("Kernel passed is not an ELF file.\n");
        return FALSE;
    }

    if (FileHeader->Id[5] != 1) {
        WriteConsole("Kernel passed uses big endian, little endian expected.\n");
        return FALSE;
    }

    if (FileHeader->Id[4] != 2 || FileHeader->MachineType != 0x3e) {
        WriteConsole("Only x86_64 is supported by Shill so far.\n");
        return FALSE;
    }

    return TRUE;
}

/* Load the kernel ELF into memory (but not enter it)
 *
 * Side effects:
 *      - changes page tree
 *      - allocates memory
 *
 * Kernel: The module of the raw kernel ELF in memory from a bootloader module
 *
 * Returns the entry point of the kernel
 */
KernelImage LoadKernel(PTR PML4, ShillModuleInfo Kernel) {
    ASSERT(Kernel.Address);

    if (VerifyGZ(Kernel.Address)) {
        Kernel = DecompressGZ(&Kernel);
    }

    ElfFileHeader *FileHeader = (ElfFileHeader*) Kernel.Address;
    if (!VerifyElf(FileHeader)) {
        WriteConsole("Invalid kernel binary\n");
        HaltDevice();
    }

    PTR Offset = FileHeader->ProgramHeaderOffset;
    for (USIZE Entry = 0; Entry < FileHeader->ProgramHeaderEntryCount; Entry++) {
        ElfProgramHeader *ProgramHeader = (ElfProgramHeader*) (Kernel.Address + Offset);
        if (ProgramHeader->Type == ELF_LOADABLE && ProgramHeader->VirtualAddress) {
            if (!KernelBinaryStart || ProgramHeader->VirtualAddress < KernelBinaryStart) {
                // I'm not actually sure if I can just do the first section if its
                // in order... oh well this is fine enough.
                KernelBinaryStart = ProgramHeader->VirtualAddress;
            }

            // TODO: maybe don't just map the whole thing as writable...
            PTR NumPages = ALIGN_UP(ProgramHeader->SizeInMemory, PAGE_SIZE) / PAGE_SIZE;
            AllocVirtuallyConsecutivePages(
                    (USIZE*) PML4,
                    ProgramHeader->VirtualAddress,
                    NumPages,
                    PAGE_PRESENT | PAGE_WRITE);

            INVALIDATE_RANGE(ProgramHeader->VirtualAddress, NumPages);
            CopyBuffer(
                    (U8*)ProgramHeader->VirtualAddress,
                    (U8*)(Kernel.Address + ProgramHeader->Offset),
                    ProgramHeader->SizeInFile);
        }
        Offset += FileHeader->ProgramHeaderEntrySize;
    }

    return (KernelImage) {
        .VirtualBase = Kernel.Address,
        .NumBytes    = Kernel.SizeBytes,
        .EntryPoint  = (PTR) FileHeader->Entry,
    };
}
