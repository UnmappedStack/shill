/* Misc definitions for other stuff to use
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#pragma once

/* You know what this does, you've seen it everywhere... */
#define CONTAINER_OF(Ptr, Type, Member) ({ \
    VOID *VPtr = (VOID*)(Ptr); \
    ((Type*)(VPtr - offsetof(Type, Member))); \
})
