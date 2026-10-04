/* Loads the ELF binary of the kernel.
 * 
 * See /include/ldkernel.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <hal.h>
#include <util.h>
#include <printf.h>
#include <ldkernel.h>

#define ELF_LOADABLE (1)

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
 * KernelStart: The start of the raw kernel ELF in memory from a bootloader module
 */
VOID LoadKernel(PTR KernelStart) {
    ElfFileHeader *FileHeader = (ElfFileHeader*) KernelStart;
    if (!VerifyElf(FileHeader)) {
        WriteConsole("Invalid kernel binary\n");
        HaltDevice();
    }

    PTR Offset = FileHeader->ProgramHeaderOffset;
    for (USIZE Entry = 0; Entry < FileHeader.ProgramHeaderEntryCount; Entry++) {
        ElfProgramHeader *ProgramHeader = (ElfProgramHeader*) (KernelStart + Offset);
        if (ProgramHeader.Type != ELF_LOADABLE) {
            Offset += ProgramHeader->Size;

            // I got here then realised I need a bootstrap physical allocator
            // first... I'll come back to this later lol
        }
    }
}
