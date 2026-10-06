#!/bin/bash
# Builds the Shill prekernel image
# (we probably need a better non-bash build system at some point... oh well)

set -e

CFLAGS="-fno-stack-protector -fno-stack-check -ffreestanding -nostdlib \
        -mno-red-zone -mgeneral-regs-only -mcmodel=kernel -static -g \
        -fno-PIC -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
        -Wall -Wextra -Werror -Wno-format"

# Kind of bad but we build with the system's gcc
gcc src/* -I include -I shared -o shill ${CFLAGS} -T linker.ld -fsanitize=undefined
