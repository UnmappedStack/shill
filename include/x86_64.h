/* x86_64 specific definitions
 * See src/x86_64.c and include/hal.h
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information. */

#pragma once

#include <stdatomic.h>
#include <util.h>

#define ASM __asm__ volatile

/* page flags */
#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE   (1 << 1)
#define PAGE_USER    (1 << 2)
#define PAGE_WC      (1 << 3)
#define INNER_NODE_FLAGS (PAGE_PRESENT | PAGE_WRITE | PAGE_USER)

#define SWITCH_PAGE_TREE(TREE_ADDRESS) \
    ASM("MOVQ %0, %%CR3" : : "r"(TREE_ADDRESS))

#define READ_PAGE_TREE(read_into) \
    ASM("MOVQ %%CR3, %0" : "=r"(read_into));

#define SWITCH_STACK(STACK_TOP) \
    ASM("MOVQ %0, %%RSP\n" \
        "MOVQ %0, %%RBP" : : "r"(STACK_TOP));

#define INVALIDATE_ADDR(addr) \
    ASM("INVLPG (%0)" : : "r"(addr) : "memory");

#define PAGE_TABLE_ENTRY(paddr, flags) (flags | paddr)

#define PADDR_FROM_TABLE_ENTRY(entry) (ALIGN_DOWN(entry, PAGE_SIZE))

#define VOFF 12

// ipi stuff
#define IPI_DEST_SHIFT 24
#define IPI_DELIVERY_FIXED                  0b000 << 8
#define IPI_DELIVERY_LOWEST                 0b001 << 8
#define IPI_DELIVERY_SMI                    0b010 << 8
#define IPI_DELIVERY_NMI                    0b100 << 8
#define IPI_DELIVERY_INIT                   0b101 << 8
#define IPI_DELIVERY_STARTUP                0b110 << 8
#define IPI_DESTINATION_PHYSICAL            0b0   << 11
#define IPI_DESTINATION_LOGICAL             0b1   << 11
#define IPI_LEVEL_DEASSERT                  0b0   << 14
#define IPI_LEVEL_ASSERT                    0b1   << 14
#define IPI_TRIGGER_EDGE                    0b0   << 15
#define IPI_TRIGGER_LEVEL                   0b1   << 15

USIZE HalCpuHardwareId(VOID);

// I think disabling interrupts isnt necessary
// here cos in the prekernel theres no preemption
#define AcquireSpinlock(Lock) \
    while (atomic_flag_test_and_set(Lock)) { \
        __builtin_ia32_pause(); \
    }
#define ReleaseSpinlock(Lock) \
    atomic_flag_clear(Lock);
#define SPINLOCK_INIT ATOMIC_FLAG_INIT
