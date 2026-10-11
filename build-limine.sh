#!/bin/bash
# Test & example script for building a disk image using the Limine bootloader
# to boot into a dummy kernel using Shill as the prekernel.

set -e

mkdir -p sysroot
mkdir -p sysroot/boot

if [ ! -d "limine-binary" ]; then
    curl -L https://github.com/Limine-Bootloader/Limine/releases/latest/download/limine-binary.tar.gz | gunzip | tar -xf -
fi

make -C limine-binary >/dev/null

cd testkernel && ./build.sh && cd ..
cp testkernelbin sysroot/kernel

gzip -c testkernelbin > sysroot/kernel.gz
gzip -c test_module.txt > sysroot/testmodule.txt.gz
gzip -c test_module.txt > sysroot/testmodule.txt

mkdir -p sysroot/boot/limine
cp limine.conf limine-binary/limine-bios.sys limine-binary/limine-bios-cd.bin \
    limine-binary/limine-uefi-cd.bin sysroot/boot/limine/

xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
            -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
            -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
            -efi-boot-part --efi-boot-image --protective-msdos-label \
            sysroot -o image.iso &>/dev/null
./limine-binary/limine bios-install image.iso &>/dev/null

qemu-system-x86_64 image.iso -serial stdio -monitor telnet:127.0.0.1:8000,server,nowait -no-reboot -no-shutdown
