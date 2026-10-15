/* Strings that may be displayed
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#define NO_MEMMAP_ENTRY_BIG_ENOUGH "Could not find a memory map entry large enough to store abstracted memory map!\n" \
                                   "Bootloader may have provided a *very* fragmented memmap... (Halting device)\n"

#define NO_KERNEL_IMAGE "\nKernel image module could not be found!\n" \
                        "Make sure your bootloader config file has a module pointing to" \
                        " the kernel which should be loaded, and specify the name " \
                        "SHILL_KERNEL_IMAGE_START\n\nHalting device\n"
