/* Like hyper.h or limine.h or whatever except just a bunch of TODO stubs, to
 * make adding a new architecture easy as it can just copy from this and add things
 * as you go
 * 
 * See src/stubs.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once
#include <types.h>

/* Get the direct map offset (HHDM) the bootloder provides and return it.
 *
 * (!) Side effect: Modifies global DirectMapOffset with the found HHDM value
 */
PTR StubGetDirectMapOffset(PTR Context, PTR Magic);

/* Get the physical memory map and abstract it into Shill's own structure
 * instead of the bootloader specific one */
ShillMemoryMap *StubGetMemoryMap(PTR Context, PTR Magic);

/* Find the kernel image in memory, from the bootloader's module for it */
ShillModuleInfo StubGetKernelImageStart(PTR Context, PTR Magic);

/* Get basic information about the prekernel */
ShillPrekernelInfo StubGetPrekernelInfo(PTR Context, PTR Magic);

/* find the rsdp the bootloader provides */
PTR StubGetRSDP(PTR Context, PTR Magic);

/* get all framebuffers and create a ShillFramebuffersList for them. */
ShillFramebuffersList *StubGetFramebuffers(PTR Context, PTR Magic);

/* get all modules (except for the one containing the kernel binary) and decompress
 * it if necessary, returning a ShillModulesList */
ShillModulesList *StubGetModules(PTR Context, PTR Magic);
