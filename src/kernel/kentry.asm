extern __kernelentry

global _start

;===================== kernel data seg ======================
[section .data]
[bits 32]
    nop

;==================== kernel stack seg ======================
[section .bss]
_stackspace:
    resb 4 * 1024
_stacktop:

;===================== kernel code seg ======================
[section .text]
_start:
    mov ax, ds
    mov es, ax
    mov fs, ax
    mov ss, ax
    mov esp, _stacktop

    jmp __kernelentry