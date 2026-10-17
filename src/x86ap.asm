;; Trampoline APs enter after sending the SIPI, takes them from 16 bit real
;; mode with no firmware to a more sane state (aka long mode)
;; 
;; See src/aps.c
;; 
;; Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
;; Licence 2.0. See LICENSE in the root of the repository for more information.

[BITS 16]

GLOBAL FarjumpTrampoline
GLOBAL APTrampolineStart
GLOBAL EnterLongMode
GLOBAL APgdtr
GLOBAL APgdt

SECTION .data

ALIGN 4096
APTrampolineStart:

;; AP first enters here
; (this will be cs offset 0)
APTrampoline:
    CLI
    MOV BYTE AL, 65 ;A
    MOV WORD DX, 0x3F8
    OUT DX, AL
.EnterProtectedMode:
    O32 LGDT [CS:(APgdtr-APTrampolineStart)]

    ; set protection enable and remove all the other bs
    MOV EAX, 1
    MOV CR0, EAX

    ; long jump to p mode
    O32 JMP FAR [CS:(FarjumpTrampoline-APTrampolineStart)]

[BITS 32]
EnterLongMode:
    MOV BYTE AL, 66 ;B
    MOV WORD DX, 0x3F8
    OUT DX, AL
    JMP $

[BITS 64]
;; 32 bit gdt
APgdt:
    DQ 0x0000000000000000  ; null entry
    DQ 0x00cf9a000000ffff  ; code
    DQ 0x00cf93000000ffff  ; data
APgdtr:
    DW APgdtr-APgdt-1 ; size, bytes-1
    DD 0              ; address, will be written to later from c on the bsp

FarjumpTrampoline:
    DD 0 ; segment/addr (written to by c on the bsp)
    DW 8 ; offset


TIMES 4096-($-APTrampolineStart) DB 0
