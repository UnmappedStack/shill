/* Bootstrap the PFNDB
 *
 * See src/pfndb.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <shill.h>
#include <types.h>

/* Allocate the sparsely physically backed memory for the pfndb.
 * Assumes all entries to be one pointer in size (sizeof(VOID*))
 * as a memdesc-like pfndb should be used.
 *
 * (!) Side effects:
 *      - Maps memory
 *      - Allocates memory
 * 
 * Returns the virtual start of the PFNDB
 */
PTR BootstrapPFNDB(ShillMemoryMap *MemoryMap, PTR DirectMapOffset);
