/* Entry for the Shill prekernel.
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <ldkernel.h>
#include <printf.h>
#include <hyper.h>
#include <types.h>
#include <api.h>
#include <hal.h>

typedef enum {
    BOOT_PROTOCOL_HYPER,
} BootProtocol;

CSTRING StringifiedProtocols[] = {
    "Hyper Boot Protocol",
};

ProtocolInterface ProtocolInterfaces[] = {
    [BOOT_PROTOCOL_HYPER] = {
        HyperGetDirectMapOffset,
        HyperGetKernelImageStart,
    },
};

/* Detect the boot protocol used from the two arguments which the bootloader
 * passes (or may not pass...) */
BootProtocol DetectBootProtocol(PTR Arg1, PTR Arg2) {
    if (Arg1 && Arg2 == ULTRA_MAGIC) return BOOT_PROTOCOL_HYPER;

    WriteSerial("No valid boot protocol detected, halting\n");
    HaltDevice();
}

/* Dummy function for tinyprintf to use for printing a character to serial. */
VOID PutChar(VOID *P, UBCHAR C) {
    (void) P;
    WriteSerialChar(C);
}

/* Entry point for the prekernel.
 *
 * arg1 and arg2 parameters depend on the bootloader protocol and will be
 * checked. */
VOID BootEntry(PTR Arg1, PTR Arg2) {
    InitSerial();
    InitPrintf(NULLPTR, PutChar);

    WriteConsole("\nEntered Shill\n");

    BootProtocol Protocol = DetectBootProtocol(Arg1, Arg2);
    WriteConsole("Boot protocol detected: %s\n", StringifiedProtocols[Protocol]);

    BootInfoBlock BootInfo = {0};
    BootInfo.DirectMapOffset = ProtocolInterfaces[Protocol].GetDirectMapOffset(Arg1, Arg2);

    WriteConsole("Read boot info:\n"
                 "  -> Direct map offset (HHDM): %p\n",
                 BootInfo.DirectMapOffset);

    WriteConsole("Trying to load kernel image...\n");
    LoadKernel(ProtocolInterfaces[Protocol].GetKernelImageStart(Arg1, Arg2));

    HaltDevice();
}
