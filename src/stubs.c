/* Like hyper.c or limine.c or whatever except just a bunch of TODO stubs, to
 * make adding a new architecture easy as it can just copy from this and add things
 * as you go
 * 
 * See include/stubs.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <types.h>
#include <util.h>

#define TODO(X) \
    do { \
        WriteConsole("TODO: %s\n", X); \
        HaltDevice(); \
    } while (0)

/* Get the direct map offset (HHDM) the bootloder provides and return it.
 *
 * (!) Side effect: Modifies global DirectMapOffset with the found HHDM value
 */
PTR StubGetDirectMapOffset(PTR Context, PTR Magic) {
    UNUSED(Context), UNUSED(Magic);
    TODO("GetDirectMapOffset for this protocol");
}

/* Get the physical memory map and abstract it into Shill's own structure
 * instead of the bootloader specific one */
ShillMemoryMap *StubGetMemoryMap(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetMemoryMap for this protocol");
}

/* Find the kernel image in memory, from the bootloader's module for it */
ShillModuleInfo StubGetKernelImageStart(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetKernelImageStart for this protocol");
}

/* Get basic information about the prekernel */
ShillPrekernelInfo StubGetPrekernelInfo(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetPrekernelInfo for this protocol");
}

/* find the rsdp the bootloader provides */
PTR StubGetRSDP(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetRSDP for this protocol");
}

/* get all framebuffers and create a ShillFramebuffersList for them. */
ShillFramebuffersList *StubGetFramebuffers(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetFramebuffers for this protocol");
}

/* get all modules (except for the one containing the kernel binary) and decompress
 * it if necessary, returning a ShillModulesList */
ShillModulesList *StubGetModules(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetModules for this protocol");
}
