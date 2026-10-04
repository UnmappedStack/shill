#!/bin/bash
# Builds the test kernel for shill to try load
# (we probably need a better non-bash build system at some point... oh well)

set -e

CFLAGS="-fno-stack-protector -fno-stack-check -ffreestanding -nostdlib \
        -mno-red-zone -mgeneral-regs-only -mcmodel=kernel -static \
        -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2"

# Kind of bad but we build with the system's gcc
gcc main.c -I include -o ../testkernelbin ${CFLAGS} -T linker.ld
