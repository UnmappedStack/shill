/* x86_64 specific operations
 *
 * See include/hal.h and include/x86_64.h
 * 
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <printf.h>
#include <balloc.h>
#include <types.h>
#include <x86_64.h>

#define COM1 0x3F8

/* Disables maskable interrupts */
VOID DisableInterrupts(VOID) {
    ASM("CLI");
}

/* Wait for any interrupt */
VOID WaitForInterrupt(VOID) {
    ASM("HLT");
}

/* Prevents the current processor from continuing */
NORETURN VOID HaltDevice(VOID) {
    DisableInterrupts();
    for (;;) {
        WaitForInterrupt();
    }
}

/* Writes a byte to a serial port.
 *
 * Port: The port to write to
 * Value: To value to write to the port
 */
static inline VOID ByteOut(U16 Port, U8 Value) {
    ASM("OUTB %0, %1" : : "a"(Value), "Nd"(Port));
}

/* Reads a byte from a serial port.
 *
 * Port: The port to read from
 */
static inline U8 ByteIn(U16 Port) {
    U8 Value;
    ASM("INB %1, %0" : "=a"(Value) : "Nd"(Port));
    return Value;
}

/* Initialises serial log output */
VOID InitSerial(VOID) {
    ByteOut(COM1 + 1, 0x00); /* disable interrupts */
    ByteOut(COM1 + 3, 0x80); /* enable DLAB to set the baud rate divisor */
    ByteOut(COM1 + 0, 0x03); /* divisor low byte: 3 (38400 baud) */
    ByteOut(COM1 + 1, 0x00); /* divisor high byte */
    ByteOut(COM1 + 3, 0x03); /* 8 bits, no parity, one stop bit */
    ByteOut(COM1 + 2, 0xC7); /* enable and clear FIFOs, 14-byte threshold */
    ByteOut(COM1 + 4, 0x03); /* DTR + RTS */
}

/* Writes a single character to the serial port
 *
 * C: the ASCII character to write
 */
VOID WriteSerialChar(CHAR C) {
    while (!(ByteIn(COM1 + 5) & 0x20));
    ByteOut(COM1, C);
}

/* Writes a C string to the serial port
 *
 * Str: The ASCII C string to write
 */
VOID WriteSerial(CSTRING Str) {
    for (USIZE Char = 0; Str[Char]; Char++) {
        WriteSerialChar(Str[Char]);
    }
}

#define LAPIC_ID_REGISTER                0x020
#define LAPIC_INTERRUPT_COMMAND_REGISTER 0x300
VOID WriteLapic(PTR LapicAddr, USIZE RegOffset, U32 Val) {
    U32 volatile *Addr =
        (U32 volatile *)(((USIZE)LapicAddr) + RegOffset);
    *Addr = Val;
}

U32 ReadLapic(PTR LapicAddr, USIZE RegOffset) {
    U32 volatile *LapicRegAddr =
        (U32 volatile *)(((U64)LapicAddr) + RegOffset);
    return (U32) *LapicRegAddr;
}

PTR GLapicAddr = 0;
USIZE HalCpuHardwareId(VOID) {
    ASSERT(GLapicAddr);
    return ReadLapic(GLapicAddr, LAPIC_ID_REGISTER) >> 24;
}

VOID SendIPI(PTR LapicAddr, U8 Vector, U32 Flags, U8 Dest) {
    WriteLapic(LapicAddr, LAPIC_INTERRUPT_COMMAND_REGISTER + 0x10, (USIZE)Dest << IPI_DEST_SHIFT);
    WriteLapic(LapicAddr, LAPIC_INTERRUPT_COMMAND_REGISTER, Vector | Flags);
}

VOID ArchStartAP(PTR LapicAddr, USIZE ApicId) {
    GLapicAddr = LapicAddr; // this can be changed to not keep repeating for each ap (TODO)
    WriteConsole("hal:x86_64: Sending INIT IPI & SIPI to CPU%zu\n", ApicId);
    SendIPI(
            LapicAddr,
            0, // init ipis must have vector 0
            IPI_DELIVERY_INIT, // flags
            ApicId
    );

    // so fucking dumb, TODO use an actual timer
    for (USIZE I = 0; I < 10000000; I++) {
        __builtin_ia32_pause();
    }
   
    SendIPI(
            LapicAddr,
            FirstPagesLower[0] / PAGE_SIZE, // page of trampoline
            IPI_DELIVERY_STARTUP,       // flags
            ApicId
    );
}
