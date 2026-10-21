;; Trampoline APs enter after sending the SIPI, takes them from 16 bit real
;; mode with no firmware to a more sane state (aka long mode)
;; 
;; See src/aps.c
;; 
;; Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
;; Licence 2.0. See LICENSE in the root of the repository for more information.

[BITS 16]

GLOBAL FarjumpTrampoline32
GLOBAL FarjumpTrampoline64
GLOBAL APTrampolineStart
GLOBAL APTrampolineEnd
GLOBAL EnterLongMode
GLOBAL LongModeStub
GLOBAL APgdtr
GLOBAL APgdt
GLOBAL APcr3

; if ESI has bit X clear, sets bit Y of EAX
%macro CONFIG 2
    BT ESI, %1
    JNC %%SKIP
    BTS EAX, %2
    %%SKIP:
%endmacro

SECTION .data

ALIGN 4096
APTrampolineStart:

;; AP first enters here
; (this will be cs offset 0)
APTrampoline:
    O32 LGDT [CS:(APgdtr-APTrampolineStart)]

    ; save for p mode
    MOV EBX, CS
    SHL EBX, 4

    ; set protection enable and remove all the other bs
    MOV EAX, 1
    MOV CR0, EAX

    ; long jump to p mode
    O32 JMP FAR [CS:(FarjumpTrampoline32-APTrampolineStart)]

[BITS 32]
%define PAE_BIT (1 << 5)
%define EFER 0xC0000080
%define LME (1 << 8)
%define PAGING_ENABLE (1 << 31)
EnterLongMode:
    ; new segment registers
    MOV AX, 0x18 ; data
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX

    ; ebx still contains cs from back in real mode
    ; (paging still isn't enabled yet at this point)
    MOV EAX, [APcr3-APTrampolineStart+EBX]
    MOV CR3, EAX

    MOV EAX, CR4
    OR AL, PAE_BIT
    CONFIG 1, 2 ; 5 level paging enabled if needed
    MOV CR4, EAX

    ; EFER stuff:
    MOV ECX, EFER
    RDMSR
    OR AX, LME ; IA-32E mode
    CONFIG 0, 11 ; execute-disable
    WRMSR

    ; paging needs to be enabled ofc
    MOV EAX, CR0
    OR EAX, PAGING_ENABLE
    MOV CR0, EAX

    JMP FAR [FarjumpTrampoline64-APTrampolineStart+EBX]

[BITS 64]
LongModeStub:
    JMP $

APgdt:
    DQ 0x0000000000000000  ; null entry
    DQ 0x00cf9a000000ffff  ; code, p mode
    dq 0x00af9b000000ffff  ; code, l mode
    DQ 0x00cf93000000ffff  ; data
APgdtr:
    DW APgdtr-APgdt-1 ; size, bytes-1
    DD 0              ; address, will be written to later from c on the bsp

FarjumpTrampoline32:
    DD 0 ; segment/addr (written to by c on the bsp)
    DW 8 ; offset

FarjumpTrampoline64:
    DD 0  ; segment/addr (written to by c on the bsp)
    DW 16 ; offset

APcr3:
    DD 0 ; written to on the bsp

APTrampolineEnd:
TIMES 4096-($-APTrampolineStart) DB 0
