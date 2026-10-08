/* Boot protocol agnostic Shill API, as passed to the kernel.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once
#include <stddef.h>
#include <stdint.h>

#define SHILL_MAGIC (0x89352)

typedef enum {
    SHILL_MEMORY_USABLE,
    SHILL_MEMORY_RESERVED,
    SHILL_MEMORY_INVALID,
    SHILL_MEMORY_ACPI_RECLAIMABLE,
    SHILL_MEMORY_ACPI_NVS,

    /* technically by the time we get to the kernel
     * there should be no bootloader reclaimable sections as
     * it should be reclaimed by the prekernel, only prekernel
     * reclaimable memory should still exist */
    SHILL_MEMORY_BTLDR_RECLAIMABLE,

    SHILL_MEMORY_MODULE, 
    SHILL_MEMORY_KERNEL_STACK,
    SHILL_MEMORY_KERNEL_BINARY,
    SHILL_MEMORY_PREBOOT_RECLAIMABLE,
} ShillMemoryMapEntryType;

/* ftr this is *after decompression*, the compressed kernel image is not exposed
 * to the kernel directly */
typedef struct {
    /* VIRTUAL address of the file. it is not contiguous in physical memory!
     * the kernel should copy it to its own buffer before switching to its
     * own page tree or use the same virtual address mappings for it as shill.
     * TODO: contiguous bootstrap allocator mapping for this to be contiguous? */
    uintptr_t VirtualBase;
    size_t NumBytes;
    uintptr_t EntryPoint;
} KernelImage;

typedef struct {
    uintptr_t PhysicalBase;
    uintptr_t SizePages;
    ShillMemoryMapEntryType Type;
} ShillMemoryMapEntry;

typedef struct {
    size_t NumEntries;
    ShillMemoryMapEntry Entries[];
} ShillMemoryMap;

/* The structure passed directly to the kernel containing the abstracted
 * away boot information */
typedef struct {
    uintptr_t DirectMapOffset;
    ShillMemoryMap *MemoryMap;
    KernelImage KernelImage;
} ShillBootInfoBlock;
