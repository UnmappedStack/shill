/* abstraction stuff
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <types.h>
#include <shill.h>

#define KERNEL_STACK_PAGES (20) 

typedef struct {
    PTR VirtualBase;
    PTR PhysicalBase;
    PTR SizeBytes;
} ShillPrekernelInfo;

// NOT the kind of module that's passed to the kernel. I should
// probably name this more aptly... (TODO)
typedef struct {
    PTR Address;
    PTR SizeBytes;
} ShillModuleInfo;

/* NOT to be used by the kernel, just some interfaces for Shill to abstract
 * stuff away nicely */
typedef struct {
    PTR (*GetDirectMapOffset)(PTR, PTR);
    ShillModuleInfo (*GetKernelImageStart)(PTR, PTR);
    ShillMemoryMap *(*GetMemoryMap)(PTR, PTR);
    ShillPrekernelInfo (*GetPrekernelInfo)(PTR, PTR);
    PTR (*GetRSDP)(PTR, PTR);
    ShillFramebuffersList *(*GetFramebuffers)(PTR, PTR);
    ShillModulesList *(*GetModules)(PTR, PTR);
} ProtocolInterface;
