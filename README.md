# Shill

A prekernel with the goal of supporting the following protocols:

 - [Ultra](https://github.com/UltraOS/UltraProtocol) (primarily through the Hyper bootloader)
 - [Limine](https://github.com/Limine-Bootloader/limine-protocol) (primarily through the Limine bootloader)
 - [Multiboot 1](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html) (and maybe 2? I mostly only want MB1 for qemu's `-kernel`)
 - Maybe eventually EFI stub support directly, not sure if that's worth it though

I plan to use Shill for Trickster, the kernel of my hobby OS TacOS. By the time Shill is in a usable state, it should:

 - Support full abstraction of all protocols it supports
 - Fill in the gaps itself for protocols which do not support features that other protocols do
 - Provide pre-allocated physical memory to the kernel for the PFNDB/`page_t`/`struct page` so that the kernel does not need a bootstrap allocator

Early work is still in progress so Shill is primarily being built around the Hyper boot protocol at the moment. Support for the other protocols listed above are planned. For now this will probably only support x86_64 but at some point I might try to expand that.

The kernel-facing API will probably be custom and not based around any particular bootloader protocol. Early stage WIP docs can be found in `USAGE.md`.

LLM-generated PRs will be closed and ignored.

## License

This project is under the Mozilla Public Licence 2.0 everywhere except otherwise stated in header files, please see LICENSE in the root of this repository.

3rd party components with differing licences include:

 - **nanoprintf**: This is under 0BSD, has some modifications, see `/include/printf.h`
 - **Ultra boot protocol header file**: Under MIT, has heavy modifications, particularly in styling and some additions to expose more stuff, see `/include/utra.h`

In all cases, any modifications to these files will be under the original license that these files are listed as above as well, not my own MPL 2.0 license that the rest of the project is under.
