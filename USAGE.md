# Using Shill with a new kernel

 > [!NOTE]
 > While Shill is still in development, the API here is pretty volatile and likely to change. Eventually it will probably stabilise once it's more feature complete. Here be dragons or something idk

## Setting up a kernel

You can see the `build-*.sh` scripts for setting up a template which starts a basic kernel. As of me writing this, only the Ultra boot protocol is supported, so you can see `build-hyper.sh` (the basic build system for the test OS using Hyper), `build.sh` and `testkernel/` (the build system for the test kernel and the test kernel source respectively), and `hyper.cfg` (the bootloader Hyper's configuration script).

The kernel executable can currently only be an ELF64 file.

The important thing to note is that your bootloader of choice's configuration file must have a module referring to the kernel which Shill needs to load, and this module's name field must contain `SHILL_KERNEL_IMAGE_START`. For example, in a `hyper.cfg`, this would look like:

```
module:
    name = "SHILL_KERNEL_IMAGE_START"
    path = "::/kernel"
```

and in a `limine.conf` this would look like:

```
module_path: boot():/kernel
module_string: "SHILL_KERNEL_IMAGE_START"
```

## API

### Environment on kernel entry

#### x86

 - You will be in long mode
 - The IDT will be a null pointer
 - The GDT will be in an undefined state as you should switch to your own but it will be valid with at least a null section, kernel data section, and kernel code section
 - `rip` will be at the entry point of the kernel executable
 - `rsp` will point to the top of a fresh stack, potentially except for a return frame to the prekernel which can be ignored. `rbp` will point to the same value
 - 4 level paging will be enabled by default, 5 level support is a TODO

When your kernel is loaded, it will be passed two parameters:

```C
void _start(ShillBootInfoBlock *boot_block, uint32_t magic);
```

You can confirm that you have been loaded by Shill by checking that `magic` is equal to `0x89352` (or `SHILL_MAGIC`). `boot_block` points to a value in prekernel-reclaimable memory of the following structure:

```C
typedef struct {
    uintptr_t DirectMapOffset;
    ShillMemoryMap *MemoryMap;
    KernelImage KernelImage;
} ShillBootInfoBlock;
```

The members listed above will be discussed now, and all structures can be found in `shared/shill.h` which will pretty much always be more up to date than this.

### Direct map offset

This is the equivalent to what is often referred to as the HHDM. It is the offset value by which the prekernel maps virtual to physical addresses for most of memory into the higher half of virtual memory. For example, you can do the following with it:

```C
// virt->phys conversion
uintptr_t phys_addr = virt_addr - boot_block->DirectMapOffset;
```

and

```C
// phys->virt conversion
uintptr_t virt_addr = phys_addr + boot_block->DirectMapOffset;
```

All memory except for the kernel image will be mapped with the direct map, up to some limit which is TBD (TODO).

### Memory map

This is a list of memory regions, and is standardised between all bootloader protocols to:

 - Be in ascending order
 - Mark the kernel binary as such, rather than as available (not looking at anybody in particular, *multiboot*...)
 - Use the same format

The relevant structures are as such:

```C
typedef struct {
    uintptr_t PhysicalBase;
    uintptr_t SizePages;
    ShillMemoryMapEntryType Type;
} ShillMemoryMapEntry;

typedef struct {
    size_t NumEntries;
    ShillMemoryMapEntry Entries[0];
} ShillMemoryMap;
```

You can find the `ShillMemoryMapEntryType` enum in `shared/shill.h`. All physical base addresses will be page aligned, and all these structures will be stored in prekernel reclaimable memory.

### Kernel image

The executable image of the kernel can be checked for its virtual memory address base, size, and entry point. However, it is **not contiguous in physical memory**.

```C
typedef struct {
    uintptr_t VirtualBase;
    size_t NumBytes;
    uintptr_t EntryPoint;
} KernelImage;
```

If the kernel is provided as a GZ-compressed binary, this will be the *decompressed version*, not the compressed one provided to Shill.
