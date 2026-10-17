/* Startup for APs
 *
 * See src/aps.c
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#pragma once

#include <acpi.h>

ShillProcessorsList *FindAPs(MADT *MADTBlock);
VOID StartAPs(PTR DirectMapOffset, MADT *MADTBlock, ShillProcessorsList *Processors);
