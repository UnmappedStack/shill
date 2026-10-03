/* Loads the ELF binary of the kernel.
 * 
 * See /include/ldkernel.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <printf.h>
#include <ldkernel.h>

/* Load the kernel ELF into memory (but not enter it)
 *
 * KernelStart: The start of the raw kernel ELF in memory from a bootloader module
 */
VOID LoadKernel(PTR KernelStart) {
    WriteConsole("we loadda da kernal\n");
}
