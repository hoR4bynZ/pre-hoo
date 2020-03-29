SELECTOR_VIDEO		equ 0x18

[section .data]
[bits 32]
_buf:       dq 0

[section .text]
[bits 32]
global __printstr
global __printint

;============================ func __printchar =========================
;-----------------------------------------------------------------------
__printstr:
;NOTE:  __printstr(char *);
    push ebx
    push ecx

    xor ecx, ecx
    mov ebx, [esp + 4 * 3]
    .beginprint:
    mov cl, [ebx]
    cmp cl, 0
    jz .lastchar
    push ecx
    call __printchar
    add esp, 4                                  ;PUSH one parameter then clean stack munual
    inc ebx
    jmp .beginprint
    .lastchar:
    pop ecx
    pop ebx
    ret

;-----------------------------------------------------------------------
__printint:
;INP:   stack = int
;OUT:   __printstr(char *);
	pushad

    .printprefix:                               ;print prefix: "0x"
    xor ebx, ebx
    mov bl, '0'
    push ebx
    call __printchar
    add esp, 4
    mov bl, 'x'
    push ebx
    call __printchar
    add esp, 4

    mov eax, [esp + 4 * 9]
    mov ebx, _buf
    mov ecx, 8                                  ;loop 8 times cuz 32 / 4 = 8
    mov edx, eax                                ;backup EAX
    mov edi, 7                                  ;point to the last bit of _buf

    .convert:
	and edx, 0xf
	cmp edx, 9
	ja .letter									;large than 9
	add edx, '0'									;below 9 add '0'
	jmp .output
	.letter:
	sub edx, 10									;beyond 9 subs 10 add 'a'
	add edx, 'a'
	.output:
	mov [ebx + edi], dl
	dec edi
	shr eax, 4
    mov edx, eax
	loop .convert

    .skipzero:                                  ;the codition when sequential zero 
                                                ;    occurs at begin
    inc edi
    cmp edi, 8
    jz .fillinzero
    mov cl, [_buf + edi]
    cmp cl, '0'
    jz .skipzero                                ;skip sequential zero at begin
    jmp .beginprint
    .fillinzero:
    mov cl, '0'

    .beginprint:
    push ecx
    call __printchar
    add esp, 4
    inc edi
    mov cl, [_buf + edi]
    cmp edi, 8
    jl .beginprint

    popad
	ret

;-----------------------------------------------------------------------
__printchar:
;NOTE:  __printchar(char *);
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