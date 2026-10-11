OUTPUT_FORMAT(elf64-x86-64)
ENTRY(BootEntry)

PHDRS
{
    text            PT_LOAD FLAGS(5); /* R-X */
    rodata          PT_LOAD FLAGS(4); /* R-- */
    limine_requests PT_LOAD FLAGS(0x06); /* Read, write */
    data            PT_LOAD FLAGS(6); /* RW- */
}

SECTIONS
{
    . = 0xffffffffff800000;

    PrekernelBinaryReadOnlyStart = .;
    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));
    PrekernelBinaryReadOnlyEnd = .;
    PrekernelBinaryWritableStart = .;

    /* on the off-chance limine is being used */
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data
    
    . = ALIGN(CONSTANT(MAXPAGESIZE));
    PrekernelBinaryWritableEnd = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
