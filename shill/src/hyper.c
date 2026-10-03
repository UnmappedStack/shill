/* Abstractions from the Ultra boot protocol (Hyper) to the generic API.
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#include <defs.h>
#include <types.h>
#include <hal.h>
#include <hyper.h>
#include <printf.h>

/* Internal function to get the Hyper attribute of a type.
 * This is a little bit slow (linear search) but it shouldn't be run that often
 * so I doubt it's a real issue.
 *
 * Context: The structure Hyper provides on entry with the
 *          information to search.
 * AttributeType: The type of attribute to search for.
 * 
 * Returns the UltraAttributeHeader which is followed in memory by the data.
 */
STATIC UltraAttributeHeader *UltraGetAttributeOfType(UltraBootContext *Context, U32 AttributeType) {
    UltraAttributeHeader *CurrentHeader = Context->Attributes;
    for (USIZE Entry = 0; Entry < Context->AttributeCount; Entry++) {
        if (CurrentHeader->Type == AttributeType) return CurrentHeader;

        CurrentHeader = ULTRA_NEXT_ATTRIBUTE(CurrentHeader);
    }

    WriteConsole("Searched for invalid attribute: %u\n", AttributeType);
    HaltDevice();
}

/* Get the direct map offset (HHDM) the bootloder provides and return it.
 *
 * TODO: This might end up changing once tables are re-mapped for standardisation
 * into the higher half? I assume this won't be needed though until multiboot gets
 * support.
 */
PTR HyperGetDirectMapOffset(PTR Context, PTR Magic) {
    UltraBootContext *BootContext = (UltraBootContext*) Context;
    UltraAttributeHeader *AttributeHeader = UltraGetAttributeOfType(BootContext, ULTRA_ATTRIBUTE_PLATFORM_INFO);
    UltraPlatformInfoAttribute *Attribute = CONTAINER_OF(AttributeHeader, UltraPlatformInfoAttribute, Header); 

    return Attribute->HigherHalfBase;
}
