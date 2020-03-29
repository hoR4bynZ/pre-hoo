[bits 32]
%define ERROR_CODE  nop
%define COMMON      push 0

extern __printstr
global _intrhdlprog

section .data
_intrstr:   db "interrupt occur!", 0xa, 0
_intrhdlprog:

%macro INTR_VERTOR 2
section .text
    intr%1entry:
        %2
        push _intrstr
        call __printstr                 ; print str "interrupt occur!"
        add esp, 4

        mov al, 0x20                    ; munual EOI
        out 0xa0, al
        out 0x20, al

        add esp, 4                      ; skip error code when call iret
        iret

section .data
    dd intr%1entry
%endmacro

INTR_VERTOR 0x00, COMMON
INTR_VERTOR 0x01, COMMON
INTR_VERTOR 0x02, COMMON
INTR_VERTOR 0x03, COMMON
INTR_VERTOR 0x04, COMMON
INTR_VERTOR 0x05, COMMON
INTR_VERTOR 0x06, COMMON
INTR_VERTOR 0x07, COMMON
INTR_VERTOR 0x08, COMMON
INTR_VERTOR 0x09, COMMON
INTR_VERTOR 0x0a, COMMON
INTR_VERTOR 0x0b, COMMON
INTR_VERTOR 0x0c, COMMON
INTR_VERTOR 0x0d, COMMON
INTR_VERTOR 0x0e, COMMON
INTR_VERTOR 0x0f, COMMON
INTR_VERTOR 0x10, COMMON
INTR_VERTOR 0x11, COMMON
INTR_VERTOR 0x12, COMMON
INTR_VERTOR 0x13, COMMON
INTR_VERTOR 0x14, COMMON
INTR_VERTOR 0x15, COMMON
INTR_VERTOR 0x16, COMMON
INTR_VERTOR 0x17, COMMON
INTR_VERTOR 0x18, COMMON
INTR_VERTOR 0x19, COMMON
INTR_VERTOR 0x1a, COMMON
INTR_VERTOR 0x1b, COMMON
INTR_VERTOR 0x1c, COMMON
INTR_VERTOR 0x1d, COMMON
INTR_VERTOR 0x1e, ERROR_CODE
INTR_VERTOR 0x1f, COMMON
INTR_VERTOR 0x20, COMMON