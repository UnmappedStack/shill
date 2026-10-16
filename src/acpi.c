/* ACPI table scanning 
 *
 * See include/acpi.h
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <paging.h>
#include <acpi.h>
#include <types.h>
#include <hal.h>

/* Find the RSDT or XSDT (depending on the revision)
 *
 * RSDP: the RSDP provided by the bootloader (or scanned for, if using mb1)
 * DirectMapOffset: literally what the parameter is called lol
 *
 * Returns:
 *    - XSDT if on ACPI revision 2.0+
 *    - RSDT if using an earlier ACPI revision
 */
SDT *FindSDT(PTR RSDPAddress, PTR DirectMapOffset) {
    ASSERT(RSDPAddress > DirectMapOffset);
    RSDPBlock *RSDP = (RSDPBlock*) RSDPAddress;

    MapPage(
            (USIZE*) GPML4,                // page tree target
            RSDPAddress,                   // virtual address
            RSDPAddress - DirectMapOffset, // physical address
            PAGE_PRESENT                   // flags
    );
    INVALIDATE_ADDR(RSDPAddress);

    WriteConsole("Detected ACPI revision %u\n", RSDP->Revision);
    PTR SDTAddrPhys = (RSDP->Revision >= 2) ?
                            RSDP->XSDTAddress :
                            RSDP->RSDTAddress;
    MapPage(
            (USIZE*)GPML4,                 // page tree target
            SDTAddrPhys + DirectMapOffset, // virt addr
            SDTAddrPhys,                   // phys addr
            PAGE_PRESENT                   // flags
    );
    return (SDT*)(SDTAddrPhys + DirectMapOffset);
}
