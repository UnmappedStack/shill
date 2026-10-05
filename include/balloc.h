/* Bootstrap allocator for the Shill prekernel.
 *
 * See src/balloc.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <types.h>
#include <api.h>

extern ShillMemoryMap *GMemoryMap;

/* Find the first usable region of the memory map and set it
 * as the start of the bump allocator
 *
 * Side effects: Writes to global variables BumpCurrentRegion and BumpCurrOffset,
 *               both with the starting physical address of the new bump allocator
 *
 * MemoryMap: A pointer to the boot protocol agnostic memory
 *            map structure
 */
VOID InitBootstrapAllocator(ShillMemoryMap *MemoryMap);

/* Allocate a single physical page
 *
 * Side effects:
 *      - Can mark regions of the memory map as prekernel reclaimable
 *      - Can increment BumpCurrentRegion
 *      - Can increment or reset BumpCurrentOffset
 *
 *  Returns the physical address of the page allocated
 */
PTR AllocPhysPage(VOID);
