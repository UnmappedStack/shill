/* Basic useless kernel for testing Shill
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <shill.h>

#include <stdint.h>
#include <stddef.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#define COM1 0x3F8

static void serial_init(void) {
    outb(COM1 + 1, 0x00); /* disable interrupts */
    outb(COM1 + 3, 0x80); /* enable DLAB to set the baud rate divisor */
    outb(COM1 + 0, 0x03); /* divisor low byte: 3 (38400 baud) */
    outb(COM1 + 1, 0x00); /* divisor high byte */
    outb(COM1 + 3, 0x03); /* 8 bits, no parity, one stop bit */
    outb(COM1 + 2, 0xC7); /* enable and clear FIFOs, 14-byte threshold */
    outb(COM1 + 4, 0x03); /* DTR + RTS */
}

static void serial_putc(char c) {
    while (!(inb(COM1 + 5) & 0x20));
    outb(COM1, c);
}

static void serial_puts(const char *s) {
    while (*s) {
        if (*s == '\n')
            serial_putc('\r');

        serial_putc(*s++);
    }
}

static void hcf(void) {
    for (;;)
        __asm__ volatile ("cli; hlt");
}

void _start(ShillBootInfoBlock boot_info, uint32_t magic) {
    serial_init();

    serial_puts("hello from the test kernel that shill loaded!\n");

    if (magic == SHILL_MAGIC) {
        serial_puts("SHILL_MAGIC test passes\n");
    } else {
        serial_puts("SHILL_MAGIC test fails, did we use something that's not shill?\n");
    }

    hcf();
}
