OUTPUT_FORMAT(elf64-x86-64)
ENTRY(BootEntry)

PHDRS
{
    text            PT_LOAD FLAGS(5); /* R-X */
    rodata          PT_LOAD FLAGS(4); /* R-- */
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
