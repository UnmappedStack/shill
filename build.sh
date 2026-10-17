#!/bin/bash
# Builds the Shill prekernel image
# (we probably need a better non-bash build system at some point... oh well)

set -e

CFLAGS="-fno-stack-protector -fno-stack-check -ffreestanding -nostdlib \
        -mno-red-zone -mgeneral-regs-only -mcmodel=kernel -static -g \
        -fno-PIC -fno-PIE -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 \
        -Wall -Wextra -Werror -Wno-format"

LDFLAGS="-m elf_x86_64 -nostdlib -static -z max-page-size=0x1000"

# Kind of bad but we build with the system's gcc
mkdir -p obj
cd obj
gcc ../src/*.c -I ../include -I ../shared -c ${CFLAGS} -fsanitize=undefined

# this is so fucking dumb to have to do per file but im too lazy for a proper
# build system yet...
nasm ../src/x86ap.asm -f elf64 -Wall -o x86ap.o
cd ..

ld obj/* -T linker.ld -o shill ${LDFLAGS}
