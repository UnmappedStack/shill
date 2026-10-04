/* Boot protocol agnostic API, as passed to the kernel.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <types.h>

typedef enum {
    SHILL_MEMORY_USABLE,
    SHILL_MEMORY_RESERVED,
    SHILL_MEMORY_INVALID,
    SHILL_MEMORY_ACPI_RECLAIMABLE,
    SHILL_MEMORY_ACPI_NVS,
    SHILL_MEMORY_BTLDR_RECLAIMABLE,
    SHILL_MEMORY_MODULE, 
    SHILL_MEMORY_KERNEL_STACK,
    SHILL_MEMORY_KERNEL_BINARY, 
} ShillMemoryMapEntryType;

typedef struct {
    PTR PhysicalBase;
    PTR SizePages;
    ShillMemoryMapEntryType Type;
} ShillMemoryMapEntry;

typedef struct {
    USIZE NumEntries;
    ShillMemoryMapEntry Entries[];
} ShillMemoryMap;

/* The structure passed directly to the kernel containing the abstracted
 * away boot information */
typedef struct {
    PTR DirectMapOffset;
    ShillMemoryMap *MemoryMap;
} ShillBootInfoBlock;

/* NOT to be used by the kernel, just some interfaces for Shill to abstract
 * stuff away nicely */
typedef struct {
    PTR (*GetDirectMapOffset)(PTR, PTR);
    PTR (*GetKernelImageStart)(PTR, PTR);
    ShillMemoryMap *(*GetMemoryMap)(PTR, PTR);
} ProtocolInterface;
