/* Setting up a page tree to replace the bootloader's, until the kernel
 * creates its own.
 *
 * See src/paging.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

/* Creates a new address space and maps essential memory into it
 *
 * Side effects: writes DirectMapOffset to GDirectMapOffset global
 *
 * DirectMapOffset: The HHDM used *before* the new page tree
 *
 * Returns the physical address of the new page tree pml4 */
PTR CreateNewAddressSpace(PTR DirectMapOffset);
