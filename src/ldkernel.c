/* Loads the ELF binary of the kernel.
 * 
 * See /include/ldkernel.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#define KGZ_IMPLEMENTATION
#define KGZ_USE_OWN_MACROS
#define KGZ_MALLOC(size) AllocateBackedPages(ALIGN_UP(size, PAGE_SIZE)/PAGE_SIZE, VALLOC_ZONE_DEFAULT)
#define KGZ_FREE(ptr, size) ({})
#define KGZ_MEMCPY(dst, src, n) CopyBuffer((VOID*)(dst), (VOID*)(src), (n))
#define KGZ_MEMSET(ptr, val, size) SetBuffer((ptr), (val), (size))
#define KGZ_PRINTF(...) WriteConsole(__VA_ARGS__)
#include <kgz.h>

#include <valloc.h>
#include <api.h>
#include <hal.h>
#include <util.h>
#include <printf.h>
#include <paging.h>
#include <ldkernel.h>

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

/* Check if a file at some memory is GZ compressed
 *
 * Buffer: a pointer to the start of the buffer
 *
 * Returns TRUE if it is gzip compressed, otherwise FALSE
 */
BOOL VerifyGZ(PTR Buffer) {
    U8 *PBuf = (U8*) Buffer;
    return (PBuf[0] == 0x1f && PBuf[1] == 0x8b);
}

/* Decompress a GZ archive
 *
 * Side effects:
 *      - Allocates memory
 *      - Maps into the page tree
 *
 * ArchiveModule: the module containing the size & address of the data to decompress
 *
 * Returns a module of the decompressed data.
 */
ShillModuleInfo DecompressGZ(ShillModuleInfo *ArchiveModule) {
    WriteConsole("GZ kernel compression detected, decompressing... ");
    USIZE ResultSize, BufferSize;
    PTR DecompressedAddr = (PTR) KGZDecompress(
            (VOID*) ArchiveModule->Address,
            ArchiveModule->SizeBytes,
            &ResultSize, &BufferSize);

    ShillModuleInfo Decompressed;
    Decompressed.Address = DecompressedAddr;
    Decompressed.SizeBytes = ResultSize;
    WriteConsole(" Ok\n");
    return Decompressed;
}

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
            USIZE Flags = PAGE_PRESENT |
                ((ProgramHeader->Flags & ELF_WRITABLE) ? PAGE_WRITE : 0);
            PTR NumPages = ALIGN_UP(ProgramHeader->SizeInMemory, PAGE_SIZE) / PAGE_SIZE;
            AllocVirtuallyConsecutivePages(
                    (USIZE*) PML4,
                    ProgramHeader->VirtualAddress,
                    NumPages,
                    Flags);

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
