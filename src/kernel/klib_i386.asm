
extern display_pos

global __printstr

;------------------------ print str -----------------------------
    __printstr:
	;INP:   edi = print pos
	;		esi = str addr printing
    ;OUT:   none
	;NOTE:	caller need to reserve EAX cuz this function will change
	push edi
	push esi

	mov esi, [esp + 4 * 3]						;str addr
	mov edi, [display_pos]						;screen offset
	mov ah, 0x0f
	.readchar:
	cld
	lodsb
	test al, al
	jz .printend								;meeting '0' ends
	cmp al, 10
	jz .carriageretlinefeed
	mov [gs: edi], ax
	add edi, 2
	jmp .readchar
	.carriageretlinefeed:
	push eax									;protect eax cuz div
	xor edx, edx
	mov eax, edi
	mov bl, 160
	div bl
	inc eax
	mul bl
	mov edi, eax
	pop eax
	jmp .readchar
	.printend:
	mov dword [display_pos], edi

	pop esi
	pop edi
	ret