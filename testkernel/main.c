/* Basic useless kernel for testing Shill. Largely based on the Hyper Bare Bones
 * wiki article: https://osdev.wiki/wiki/Hyper_Bare_Bones
 * and also some stuff taken from my other old projects. like this file is just
 * a weird hodgepodge of tests lol
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <shill.h>

#include <stdint.h>
#include <stdatomic.h>
#include <stddef.h>

typedef atomic_flag Spinlock;
#define SPINLOCK_INIT ATOMIC_FLAG_INIT
// interrupts not being disabled is fine in this test,
// interrupts aren't on anyways
#define AcquireSpinlock(Lock) \
    while (atomic_flag_test_and_set(Lock)) { \
        __builtin_ia32_pause(); \
    }
#define ReleaseSpinlock(Lock) \
    atomic_flag_clear(Lock);

uint8_t *fb_addr;
uint32_t fb_pitch;

uint8_t r_shift, g_shift, b_shift;

void draw_pixel(uint64_t x, uint64_t y, uint32_t colour) {
    uint8_t r = colour >> r_shift;
    uint8_t g = colour >> g_shift;
    uint8_t b = colour >> b_shift;
    uint32_t new = b | (g << 8) | (r << 16);
    uint32_t *location = (uint32_t *)((fb_addr) + y * fb_pitch);
    location[x] = new;
}

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

Spinlock SerialLock = SPINLOCK_INIT;
static void serial_puts(const char *s) {
    AcquireSpinlock(&SerialLock);
    while (*s) {
        if (*s == '\n')
            serial_putc('\r');

        serial_putc(*s++);
    }
    ReleaseSpinlock(&SerialLock);
}

void *memcpy(void *dest, const void *src, size_t n) {
    __asm__ volatile("rep movsb"
                     : "=D"(dest), "=S"(src), "=c"(n)
                     : "D"(dest), "S"(src), "c"(n)
                     : "memory");
    return dest;
}

static void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static void uint64_to_hex_string_padded(uint64_t num, char *str) {
    char buffer[17];
    int index = 0;
    if (num == 0) {
        buffer[index++] = '0';
    } else {
        while (num > 0) {
            uint8_t digit = num & 0xF;
            if (digit < 10) {
                buffer[index++] = '0' + digit;
            } else {
                buffer[index++] = 'A' + (digit - 10);
            }
            num >>= 4;
        }
    }
    while (index < 16) buffer[index++] = '0';
    buffer[index] = '\0';
    reverse(buffer, index);
    memcpy(str, buffer, 17);
}

static void putint(uint64_t value) {
    char buf[64];
    uint64_to_hex_string_padded(value, buf);
    serial_puts("0x");
    serial_puts(buf);
    serial_puts("\n");
}

static void hcf(void) {
    for (;;)
        __asm__ volatile ("cli; hlt");
}

void ap_entry(void) {
    serial_puts("hello from AP\n");
    hcf();
}

void _start(ShillBootInfoBlock boot_info, uint32_t magic) {
    serial_init();

    serial_puts("hello from the test kernel that shill loaded!\n");

    if (magic == SHILL_MAGIC) {
        serial_puts("SHILL_MAGIC test passes\n");
    } else {
        serial_puts("SHILL_MAGIC test fails, did we use something that's not shill?\n");
    }

    serial_puts("Dump info from prekernel:\n");
    serial_puts("   -> HHDM: "); putint(boot_info.DirectMapOffset);
    serial_puts("   -> Memmap entries: "); putint(boot_info.MemoryMap->NumEntries);
    serial_puts("   -> Kernel image addr: "); putint(boot_info.KernelImage.VirtualBase);
    serial_puts("   -> RSDP: "); putint(boot_info.RSDP);
    serial_puts("   -> Num modules: "); putint(boot_info.Modules->NumModules);
    serial_puts("   -> Num framebuffers: "); putint(boot_info.Framebuffers->NumFramebuffers);

    if (!boot_info.Framebuffers->NumFramebuffers) {
        serial_puts("No framebuffer detected!\n");
        hcf();
    }
    
    uint32_t fmt = boot_info.Framebuffers->Framebuffers[0].Format;
    if (fmt == SHILL_FB_FORMAT_XRGB8888 ||
            fmt == SHILL_FB_FORMAT_RGB888) {
        b_shift = 0;
        g_shift = 8;
        r_shift = 16;
    } else {
        serial_puts("unexpected framebuffer format\n");
        hcf();
    }

    fb_addr = (uint8_t*) (boot_info.Framebuffers->Framebuffers[0].PhysicalAddress + boot_info.DirectMapOffset);
    fb_pitch = boot_info.Framebuffers->Framebuffers[0].Pitch;
    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            draw_pixel(x, y, 0xFFFF0000);
        }
    }

    for (int i = 0; i < boot_info.Modules->NumModules; i++) {
        serial_puts("found module '");
        serial_puts(boot_info.Modules->Modules[i].Name);
        serial_puts("' of contents:\n   ");
        serial_puts((char*)boot_info.Modules->Modules[i].Address);
    }

    for (int i = 0; i < boot_info.Processors->NumProcessors; i++) {
        boot_info.Processors->Processors[i].Entry = &ap_entry;
    }

    hcf();
}
