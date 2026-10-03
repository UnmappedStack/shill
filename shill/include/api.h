/* Boot protocol agnostic API, as passed to the kernel.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <types.h>

/* The structure passed directly to the kernel containing the abstracted
 * away boot information */
typedef struct {
    PTR DirectMapOffset;
} BootInfoBlock;

typedef struct {
    PTR (*GetDirectMapOffset)(PTR, PTR);
    PTR (*GetKernelImageStart)(PTR, PTR);
} ProtocolInterface;
