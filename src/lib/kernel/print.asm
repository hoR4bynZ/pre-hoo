SELECTOR_VIDEO		equ 0x18

[bits 32]
[section .text]
global __printchar

;============================ func __printchar =========================
__printchar:
;NOTE: __printchar(char *)
    pushad                                      ;eax-ecx-edx-ebx-esp-ebp-esi-edi
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    ;-------------------------- get cursor -----------------------------
    ; use CRT Controller Data Registers index 0eh & 0fh to get cursor
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    in al, dx                                   ;high 8-bits
    mov ah, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx                                   ;low 8-bits

    mov bx, ax
    mov ecx, [esp + 4 * 9]                      ;ecx = char printing

    ;-------------------------- judge char ------------------------------
    cmp cl, 0xd                                 ;cmp CR
    jz .carriage_return
    cmp cl, 0xa                                 ;cmp LF
    jz .line_feed
    cmp cl, 0x8                                 ;cmp backspace
    jz .backspace
    jmp .otherchar

    ;--------------------------- backspace -------------------------------
    .backspace:
    dec bx
    shl bx, 1                                   ;equal to multiply 2
    mov word [gs: bx], 0x0720
    shr bx, 1
    jmp .setcursor

    ;--------------------------- print char -------------------------------
    .otherchar:
    shl bx, 1
    mov byte [gs: bx], cl
    inc bx
    mov byte [gs: bx], 0x0f
    shr bx, 1
    inc bx                                      ;become next addr printing
    cmp bx, 2000                                ;check each increment
    jl .setcursor

    ;----------------------------- CRLF -----------------------------------
    .carriage_return:
    .line_feed:
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx                                  ;current line header

    add bx, 80
    cmp bx, 2000
    jl .setcursor

    ;--------------------------- roll screen ------------------------------
    ;movsd: esi to edi
    .rollscreen:
    cld
    mov ecx, 960                                ;(2000-80) * 2 / 4 = 960
    mov esi, 0xc00b_80a0                        ;line 1 ~ 24
    mov edi, 0xc00b_8000                        ;line 0 ~ 23
    rep movsd                                   ;rep depands on ECX

    mov ebx, 3840                               ;4000 - 160 = last line
    mov ecx, 80
    .cleanlastline:
    mov word [gs: ebx], 0x0720
    add ebx, 2
    loop .cleanlastline
    mov bx, 1920

    ;-------------------------- set cursor --------------------------------
    .setcursor:
    mov dx, 0x03d4
    mov al, 0xe
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0xf
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
    
    popad
    ret