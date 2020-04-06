[bits 32]
%define ERROR_CODE  nop
%define COMMON      push 0

extern __printstr
extern _intrhdlprog
global _intrhdlprogent
global _intrext

[section .data]
_intrstr:   db "interrupt occur!", 0xa, 0
_intrhdlprogent:

%macro INTR_VERTOR 2
[section .text]
    intr%1entry:
        %2
        ; 调用C语言的中断处理程序，一定要先保护现场
        push ds
        push es
        push fs
        push gs
        pushad

        mov al, 0x20                    ; exec OCW2, munual EOI
        out 0xa0, al
        out 0x20, al

        ; 以中断向量号作为参数，_intrhdlprog[]保存的是函数地址
        ; 此处调用的是C中定义的中断处理程序，而函数地址32位，故索引*4Byte = 偏移
        push %1
        call [_intrhdlprog + %1 * 4]
        jmp _intrext

[section .data]
    dd intr%1entry
%endmacro
[section .text]
_intrext:
    add esp, 4                          ; skip interrupt vector
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4                          ; skip error code when call iret
    iretd



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
INTR_VERTOR 0x20, COMMON        ;时钟中断
INTR_VERTOR 0x21, COMMON        ;键盘中断
INTR_VERTOR 0x22, COMMON
INTR_VERTOR 0x23, COMMON
INTR_VERTOR 0x24, COMMON
INTR_VERTOR 0x25, COMMON
INTR_VERTOR 0x26, COMMON
INTR_VERTOR 0x27, COMMON
INTR_VERTOR 0x28, COMMON
INTR_VERTOR 0x29, COMMON
INTR_VERTOR 0x2a, COMMON
INTR_VERTOR 0x2b, COMMON
INTR_VERTOR 0x2c, COMMON
INTR_VERTOR 0x2d, COMMON
INTR_VERTOR 0x2e, COMMON
INTR_VERTOR 0x2f, COMMON