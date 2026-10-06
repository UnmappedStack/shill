/* abstraction stuff
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <types.h>
#include <shill.h>

typedef struct {
    uintptr_t VirtualBase;
    uintptr_t PhysicalBase;
    uintptr_t SizeBytes;
} ShillPrekernelInfo;

/* NOT to be used by the kernel, just some interfaces for Shill to abstract
 * stuff away nicely */
typedef struct {
    PTR (*GetDirectMapOffset)(PTR, PTR);
    PTR (*GetKernelImageStart)(PTR, PTR);
    ShillMemoryMap *(*GetMemoryMap)(PTR, PTR);
    ShillPrekernelInfo (*GetPrekernelInfo)(PTR, PTR);
} ProtocolInterface;
