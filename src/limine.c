/* Abstractions from the Limine boot protocol to the generic API.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <limine.h>
#include <types.h>
#include <util.h>

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

#define TODO(X) \
    do { \
        WriteConsole("TODO: %s\n", X); \
        HaltDevice(); \
    } while (0)

LIMINE_REQUEST(HHDMRequest, hhdm, LIMINE_HHDM_REQUEST_ID);

/* Get the direct map offset (HHDM) the bootloder provides and return it.
 *
 * (!) Side effect: Modifies global DirectMapOffset with the found HHDM value
 */
PTR LimineGetDirectMapOffset(PTR Context, PTR Magic) {
    UNUSED(Context), UNUSED(Magic);
    return HHDMRequest.Response->Offset;
}

/* Get the physical memory map and abstract it into Shill's own structure
 * instead of the bootloader specific one */
ShillMemoryMap *LimineGetMemoryMap(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetMemoryMap for this protocol");
}

/* Find the kernel image in memory, from the bootloader's module for it */
ShillModuleInfo LimineGetKernelImageStart(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetKernelImageStart for this protocol");
}

/* Get basic information about the prekernel */
ShillPrekernelInfo LimineGetPrekernelInfo(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetPrekernelInfo for this protocol");
}

/* find the rsdp the bootloader provides */
PTR LimineGetRSDP(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetRSDP for this protocol");
}

/* get all framebuffers and create a ShillFramebuffersList for them. */
ShillFramebuffersList *LimineGetFramebuffers(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetFramebuffers for this protocol");
}

/* get all modules (except for the one containing the kernel binary) and decompress
 * it if necessary, returning a ShillModulesList */
ShillModulesList *LimineGetModules(PTR Context, PTR Magic) {
    UNUSED(Magic), UNUSED(Context);
    TODO("GetModules for this protocol");
}
