/* Entry for the Shill prekernel.
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <hyper.h>
#include <types.h>
#include <hal.h>

typedef enum {
    BOOT_PROTOCOL_HYPER,
} BootProtocol;

CSTRING StringifiedProtocols[] = {
    "Hyper Boot Protocol",
};

/* Detect the boot protocol used from the two arguments which the bootloader
 * passes (or may not pass...) */
BootProtocol DetectBootProtocol(PTR Arg1, PTR Arg2) {
    if (Arg1 && Arg2 == ULTRA_MAGIC) return BOOT_PROTOCOL_HYPER;

    WriteSerial("no valid boot protocol detected, halting\n");
    HaltDevice();
}

/* Entry point for the prekernel.
 *
 * arg1 and arg2 parameters depend on the bootloader protocol and will be
 * checked. */
VOID BootEntry(PTR Arg1, PTR Arg2) {
    InitSerial();
    BootProtocol Protocol = DetectBootProtocol(Arg1, Arg2);
    WriteSerial(StringifiedProtocols[Protocol]);

    HaltDevice();
}
