/* High level abstraction of architecture specific operations
 * 
 * See src/x86_64.c
 *
 * Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
 * Licence 2.0. See LICENSE in the root of the repository for more information.
 */

#include <types.h>

/* Disables maskable interrupts */
VOID DisableInterrupts(VOID);

/* Wait for any interrupt */
VOID WaitForInterrupt(VOID);

/* Prevents the current processor from continuing */
NORETURN VOID HaltDevice(VOID);

/* Initialises serial log output */
VOID InitSerial(VOID);

/* Writes a C string to the serial port
 *
 * Str: The ASCII C string to write
 */
VOID WriteSerial(CSTRING Str);

/* Writes a single character to the serial port
 *
 * C: the ASCII character to write
 */
VOID WriteSerialChar(CHAR C);
