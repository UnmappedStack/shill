;; Trampoline APs enter after sending the SIPI, takes them from 16 bit real
;; mode with no firmware to a more sane state (aka long mode)
;; 
;; See src/aps.c
;; 
;; Copyright 2026 Jake Steinburger (UnmappedStack) under the Mozilla Public
;; Licence 2.0. See LICENSE in the root of the repository for more information.


[BITS 16]

GLOBAL APTrampoline 

ALIGN 4096
;; AP first enters here
APTrampoline:
    CLI
    MOV BYTE AL, 65
    MOV WORD DX, 0x3F8
    OUT DX, AL
    JMP $

times 4096-($-APTrampoline) db 0
