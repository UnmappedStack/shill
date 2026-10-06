/* Loads the ELF binary of the kernel.
 *
 * See /src/ldkernel.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <types.h>

/* Load the kernel ELF into memory (but not enter it)
 *
 * KernelStart: The start of the raw kernel ELF in memory from a bootloader module
 *
 * Returns the entry point of the kernel
 */
PTR LoadKernel(PTR PML4, PTR KernelStart);
