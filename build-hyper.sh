#!/bin/bash
# Test & example script for building a disk image using the Hyper bootloader
# to boot into a dummy kernel using Shill as the prekernel.

set -e

mkdir -p sysroot
cp hyper.cfg sysroot
./build.sh # build shill
cp shill sysroot

if [ ! -f "hyper_install" ]; then
    curl -L https://github.com/UltraOS/Hyper/releases/download/v0.12.0/hyper_install >> hyper_install
fi

if [ ! -f "BOOTX64.EFI" ]; then
    curl -L https://github.com/UltraOS/Hyper/releases/download/v0.12.0/BOOTX64.EFI >> BOOTX64.EFI
fi

if [ ! -f "sysroot/hyper_iso_boot" ]; then
    curl -L https://github.com/UltraOS/Hyper/releases/download/v0.12.0/hyper_iso_boot >> sysroot/hyper_iso_boot
fi

cd testkernel && ./build.sh && cd ..
cp testkernelbin sysroot/kernel

dd if=/dev/zero of=sysroot/efipartition.img count=100 bs=1M
mkfs.fat sysroot/efipartition.img
mcopy -i sysroot/efipartition.img BOOTX64.EFI ::.

xorriso -as mkisofs \
  -b hyper_iso_boot \
  -no-emul-boot \
  -boot-load-size 4 \
  -boot-info-table \
  --efi-boot efipartition.img \
  -efi-boot-part --efi-boot-image \
  --protective-msdos-label sysroot \
  -o image.iso

python3 hyper_install image.iso 

qemu-system-x86_64 image.iso -serial stdio -display none -no-reboot -no-shutdown
