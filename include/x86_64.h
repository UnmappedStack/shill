/* x86_64 specific definitions
 * See src/x86_64.c and include/hal.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <util.h>

#define ASM __asm__ volatile

/* page flags */
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE   (1 << 1)
#define PAGE_USER    (1 << 2)
#define INNER_NODE_FLAGS (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)

#define SWITCH_PAGE_TREE(TREE_ADDRESS) \
    ASM("MOVQ %0, %%CR3" : : "r"(TREE_ADDRESS))

#define READ_PAGE_TREE(read_into) \
    ASM("MOVQ %%CR3, %0" : "=r"(read_into));

#define SWITCH_STACK(STACK_TOP) \
    ASM("MOVQ %0, %%RSP\n" \
        "MOVQ $0, %%RBP" : : "r"(STACK_TOP));

#define INVALIDATE_ADDR(addr) \
    ASM("INVLPG (%0)" : : "r"(addr) : "memory");

#define PAGE_TABLE_ENTRY(paddr, flags) (flags | paddr)

// TODO: this wont support higher flags like execute disable properly
#define PADDR_FROM_TABLE_ENTRY(entry) (ALIGN_DOWN(entry, PAGE_SIZE))

#define VOFF 12
