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

RSDPBlock *GlobalRSDP = 0;

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
    GlobalRSDP = RSDP;

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
    PTR SDTVirt = SDTAddrPhys + DirectMapOffset;

    MapPage(
            (USIZE*)GPML4, // page tree target
            SDTVirt,       // virt addr
            SDTAddrPhys,   // phys addr
            PAGE_PRESENT   // flags
    );
    INVALIDATE_ADDR(SDTVirt);
    return (SDT*) SDTVirt;
}

/* Find the MADT
 *
 * RootSDT: the RSDT/XSDT (found by FindSDT)
 * DirectMapOffset: literally what the parameter is called lol
 *
 * Returns pointer to the MADT
 */
__attribute__((no_sanitize("undefined"))) // unfortunately necessary
MADT *FindMADT(SDT *RootSDT, PTR DirectMapOffset) {
    ASSERT(GlobalRSDP);

    BOOL UseXSDT = GlobalRSDP->Revision >= 2; // false if rsdt instead
    USIZE NumEntries = (RootSDT->Header.Length - sizeof(RootSDT->Header)) / ((UseXSDT)?8:4);
    for (USIZE EntryIdx = 0; EntryIdx < NumEntries; EntryIdx++) {
        PTR Entry;
        if (UseXSDT) Entry = ((U64*)RootSDT->Entries)[EntryIdx];
        else         Entry = ((U32*)RootSDT->Entries)[EntryIdx];

        ISDTHeader *Header = (ISDTHeader*)(Entry + DirectMapOffset);
        if (BuffersAreEqual(Header->Signature, "APIC", 4))
            return (MADT*)Header;
    }

    ASSERT(!"no madt found");
}
