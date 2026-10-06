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
} ShillBootInfoBlock;
