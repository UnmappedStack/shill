// Yoinked from https://github.com/UltraOS/UltraProtocol/blob/master/ultra_protocol.h
// for Shill to get Ultra protocol specific bootloader info. Has some modifications
// to fit with the codestyle of the rest of Shill and to also expose stuff in src/hyper.c.

#pragma once

#include <api.h>
PTR HyperGetDirectMapOffset(PTR Context, PTR Magic);
ShillModuleInfo HyperGetKernelImageStart(PTR Context, PTR Magic);
ShillMemoryMap *HyperGetMemoryMap(PTR Context, PTR Magic);
ShillPrekernelInfo HyperGetPrekernelInfo(PTR Context, PTR Magic);
PTR HyperGetRSDP(PTR Context, PTR Magic);
ShillFramebuffersList *HyperGetFramebuffers(PTR Context, PTR Magic);

// Copyright (c) 2022-2023 UltraOS
// SPDX-License-Identifier: MIT

#include <types.h>

#define ULTRA_ATTRIBUTE_INVALID          0
#define ULTRA_ATTRIBUTE_PLATFORM_INFO    1
#define ULTRA_ATTRIBUTE_KERNEL_INFO      2
#define ULTRA_ATTRIBUTE_MEMORY_MAP       3
#define ULTRA_ATTRIBUTE_MODULE_INFO      4
#define ULTRA_ATTRIBUTE_COMMAND_LINE     5
#define ULTRA_ATTRIBUTE_FRAMEBUFFER_INFO 6
#define ULTRA_ATTRIBUTE_APM_INFO         7
#define ULTRA_ATTRIBUTE_UEFI_INFO        8

typedef struct {
    U32 Type;
    U32 Size;
} UltraAttributeHeader;

#define ULTRA_PLATFORM_INVALID 0
#define ULTRA_PLATFORM_BIOS    1
#define ULTRA_PLATFORM_UEFI    2

typedef struct {
    UltraAttributeHeader Header;
    U32 PlatformType;

    U16 LoaderMajor;
    U16 LoaderMinor;
    UBCHAR LoaderName[32];

    U64 RSDPAddress;
    U64 HigherHalfBase;
    U8 PageableDepth;
    U8 Reserved[7];
    U64 DTBAddress;
    U64 SMBiosAddress;
} UltraPlatformInfoAttribute;

#define ULTRA_PARTITION_TYPE_INVALID 0
#define ULTRA_PARTITION_TYPE_RAW     1
#define ULTRA_PARTITION_TYPE_MBR     2
#define ULTRA_PARTITION_TYPE_GPT     3
#define ULTRA_PARTITION_TYPE_PXE_V4  4
#define ULTRA_PARTITION_TYPE_PXE_V6  5

typedef struct {
    U32 Data1;
    U16 Data2;
    U16 Data3;
    U8  Data4[8];
} UltraGUID;

typedef struct {
    U8 Addr[4];
} UltraIPV4Addr;

typedef struct {
    U8 Addr[16];
} UltraIPV6Addr;

#define ULTRA_PATH_MAX 256

typedef struct {
    UltraAttributeHeader Header;

    U64 PhysicalBase;
    U64 VirtualBase;
    U64 Size;

    // one of ULTRA_PARTITION_TYPE_*
    U64 PartitionType;

    // only valid if partitionype == ULTRA_PARTITION_TYPE_GPT
    UltraGUID DiskGUID;

    union {
        // only valid if partitionype == ULTRA_PARTITION_TYPE_GPT
        UltraGUID PartitionGUID;
        // only valid if partitionype == ULTRA_PARTITION_TYPE_PXE_V4
        UltraIPV4Addr PXEV4;
        // only valid if partitionype == ULTRA_PARTITION_TYPE_PXE_V6
        UltraIPV6Addr PXEV6;
    };

    // always valid
    U32 DiskIndex;
    U32 PartitionIndex;

    UBCHAR FSPath[ULTRA_PATH_MAX];
} UltraKernelInfoAttribute;

#define ULTRA_MEMORY_TYPE_INVALID            0x00000000
#define ULTRA_MEMORY_TYPE_FREE               0x00000001
#define ULTRA_MEMORY_TYPE_RESERVED           0x00000002
#define ULTRA_MEMORY_TYPE_ACPI_RECLAIMABLE   0x00000003
#define ULTRA_MEMORY_TYPE_ACPI_NVS           0x00000004
#define ULTRA_MEMORY_TYPE_LOADER_RECLAIMABLE 0xFFFF0001
#define ULTRA_MEMORY_TYPE_MODULE             0xFFFF0002
#define ULTRA_MEMORY_TYPE_KERNEL_STACK       0xFFFF0003
#define ULTRA_MEMORY_TYPE_KERNEL_BINARY      0xFFFF0004

typedef struct {
    U64 PhysicalAddress;
    U64 Size;
    U64 Type;
} UltraMemoryMapEntry;
#define ULTRA_MEMORY_MAP_ENTRY_COUNT(Header) ((((Header)->Size) - sizeof(UltraAttributeHeader)) / sizeof(UltraMemoryMapEntry))

typedef struct {
    UltraAttributeHeader Header;
    UltraMemoryMapEntry Entries[];
} UltraMemoryMapAttribute;

#define ULTRA_MODULE_TYPE_INVALID 0
#define ULTRA_MODULE_TYPE_FILE    1
#define ULTRA_MODULE_TYPE_MEMORY  2

typedef struct {
    UltraAttributeHeader Header;
    U32 Reserved;
    U32 Type;
    UBCHAR Name[64];
    U64 Address;
    U64 Size;
    UBCHAR Description[];
} UltraModuleInfoAttribute;

/*
 * NOTE: The size of the description is impossible to derive from header.size
 *       due to alignment reason. Use str{n}len() or similar.
 */
#define ULTRA_MODULE_HAS_DESCRIPTION(Header) ((((Header).Size) - sizeof(UltraModuleInfoAttributes)) > 0)

typedef struct {
    UltraAttributeHeader Header;
    /*
     * NOTE: The size of 'text' is impossible to derive from header.size
     *       due to alignment reason. Use str{n}len() or similar.
     */
    UBCHAR Text[];
} UltraCommandLineAttribute;

#define ULTRA_FB_FORMAT_INVALID  0
#define ULTRA_FB_FORMAT_RGB888   1
#define ULTRA_FB_FORMAT_BGR888   2
#define ULTRA_FB_FORMAT_RGBX8888 3
#define ULTRA_FB_FORMAT_XRGB8888 4

typedef struct {
    U32 Width;
    U32 Height;
    U32 Pitch;
    U16 BPP;
    U16 Format;
    U64 PhysicalAddress;
} UltraFramebuffer;

typedef struct {
    UltraAttributeHeader Header;
    UltraFramebuffer FB;
} UltraFramebufferAttribute;

typedef struct {
    U16 Version;
    U16 Flags;

    U16 PMCodeSegment;
    U16 PMCodeSegmentLength;
    U32 PMOffset;

    U16 RMCodeSegment;
    U16 RMCodeSegmentLength;

    U16 DataSegment;
    U16 DataSegmentLength;
} UltraAPMInfo;

typedef struct {
    UltraAttributeHeader Header;
    UltraAPMInfo Info;
} UltraAPMAttribute;

typedef struct {
    UltraAttributeHeader Header;

    U64 SystemableAddress;

    U32 DescriptorSize;
    U32 DescriptorVersion;

    // Width of the UEFI firmware in bits, either 32 or 64
    U32 FirmwareWidth;
    U32 Reserved;

    U8 MemoryDescriptors[];
} UltraUEFIInfoAttribute;

#define ULTRA_UEFI_INFO_MEM_DESC_COUNT(Info) ((((Info).Header.Size) - sizeof(UltraUEFIInfoAttribute)) / (info).DescriptorSize)

typedef struct {
    U8 ProtocolMajor;
    U8 ProtocolMinor;
    U16 Reserved;

    U32 AttributeCount;
    UltraAttributeHeader Attributes[];
} UltraBootContext;
#define ULTRA_NEXT_ATTRIBUTE(Current) ((UltraAttributeHeader*)(((U8*)(Current)) + (Current)->Size))

#define ULTRA_MAGIC 0x554c5442
