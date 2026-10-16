/* ACPI table types & defs
 *
 * See src/acpi.c
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#pragma once
#include <types.h>

typedef struct {
    UBCHAR Signature[8];
    U8 Checksum;
    UBCHAR OEMID[6];
    U8 Revision;
    U32 RSDTAddress;

    U32 Length;
    U64 XSDTAddress;
    U8 ExtendedChecksum;
    U8 Reserved[3];
} PACKED RSDPBlock;

typedef struct {
    UBCHAR Signature[4];
    U32 Length;
    U8 Revision;
    U8 Checksum;
    UBCHAR OEMID[6];
    UBCHAR OEMTableID[8];
    U32 OEMRevision;
    U32 CreatorID;
    U32 CreatorRevision;
} PACKED ISDTHeader;

//technically either rsdt OR xsdt
typedef struct {
    ISDTHeader Header;
    USIZE Entries[0];
} PACKED SDT;

typedef struct {
    ISDTHeader Header;
    U32 LocalAPICAddr;
    U32 flags;
    // fields are then here
} PACKED MADT;

/* Find the RSDT or XSDT (depending on the revision)
 *
 * RSDP: the RSDP provided by the bootloader (or scanned for, if using mb1)
 * DirectMapOffset: literally what the parameter is called lol
 *
 * Returns:
 *    - XSDT if on ACPI revision 2.0+
 *    - RSDT if using an earlier ACPI revision
 */
SDT *FindSDT(PTR RSDPAddress, PTR DirectMapOffset);

/* Find the MADT
 *
 * RootSDT: the RSDT/XSDT (found by FindSDT)
 * DirectMapOffset: literally what the parameter is called lol
 *
 * Returns pointer to the MADT
 */
MADT *FindMADT(SDT *RootSDT, PTR DirectMapOffset);
