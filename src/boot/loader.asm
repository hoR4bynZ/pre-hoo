org	0x100

	jmp Laber_Start
    %include "fat12.inc"
	%include "loader.inc"

;=========================== gdt ===============================
[section gdt]
_gdtzero:			dd 0, 0
_gdtcode:			dd 0x0000_ffff, 0x00cf_9a00
_gdtdata:			dd 0x0000_ffff, 0x00cf_9200
_gdtvideo:			dd 0x8000_0007, 0x00cf_920b

GDT_LENGTH			equ $ - _gdtzero
_gdtptr:			dw GDT_LENGTH - 1
					dd _gdtzero + LOADER_PHY_ADDR

SELECTOR_CODE		equ _gdtcode - _gdtzero
SELECTOR_DATA		equ _gdtdata - _gdtzero
SELECTOR_VIDEO		equ _gdtvideo - _gdtzero

[section .s16]
;======================== loader start ==========================
[bits 16]
Laber_Start:                                	;# +62
	mov	ax,	cs
	mov	ds,	ax
	mov	es,	ax
	mov	ax,	0x00
	mov	ss,	ax
	mov	sp,	0x7c00

	;---------------------- display str -------------------------
	;++++++++++++++++++++++ display str +++++++++++++++++++++++++
	;+                    Call INT 10h, 13h                     +
	;+INP:    al = write mode--01--cursor move by str tail      +
	;+                      bh = page                           +
	;+                    bl = attribute                        +
	;+                   cx = string length                     +
	;+           (dh, dl) = row, col of cursor pos              +
	;+                    es:bp = str pos                       +
	;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	mov	ax,	0x1301
	mov	bx,	0x000f
	mov	cx,	12
	mov	dx,	0x0100								;row 2
	mov	bp,	_startloadermess
	int	0x10

	;----------------------- reset floppy -----------------------
    xor ah, ah
    xor dl, dl                              	;00 = no.1 floppy driver
    int 0x13

	;----------------------- search KERNEL ----------------------
	mov word [_sectorno], SectorNoOfRootDir  	;19

Search_In_Rootdir:
	cmp word [_rootdirloop], 0        			;cmp 14 & 0
	jz Kernel_Fail
	dec word [_rootdirloop]

	mov ax, 0
	mov es, ax
	mov bx, 0x8000                          	;buffer = es:bx = 0:8000 = 0x8000
	mov ax, [_sectorno]                      	;19
	mov cl, 1                               	;相当于上面的dec,减去一个扇区数
	call __readsectors                      	;将软盘第19个扇区内容读取到内存0x8000

	;seek every entrys from buffer to find KERNEL
	mov dx, 0x10                            	;entrys per sector = 512 / 32 = 16
	mov di, 0x8000                          	;等会对比文件名时使用

	;找一个扇区全部16个目录项
	Search_Kernel:
	cmp dx, 0
	jz Search_Next_Sec                      	;找完一个扇区全部16个目录项
	dec dx
	mov si, _kernelname                     	;LODSB inp: ds:si = des data addr
	cld                                     	;      out: al
	mov cx, 11                              	;file name length

	;找文件名全部11个字符
	Cmp_Filename:
	cmp cx, 0
	jz Search_Ent_Success                   	;如果文件名全部匹配就是找到了目录项
	dec cx
	lodsb                                   	;move data from ds:si to al
	cmp al, byte [es:di]                    	;move 1 byte each loop
	jz Cmp_Char_Same                        	;字符相同，继续找下一个字符，所以回到这个循环
	jmp Cmp_Char_Diff                       	;字符有一个不相同，就不用找这个目录项，所以找下一个目录项

	Cmp_Char_Same:
	inc di                                  	;read next char
	jmp Cmp_Filename

	Cmp_Char_Diff:
	and di, 0xffe0                          	;32位对齐，让di重新指到目录项第一个字段，因为di当前值在上一次比较文件名时有递增
	add di, 20h                             	;cmp next entry
	;mov si, _kernelname
	jmp Search_Kernel

	Kernel_Fail:
	mov ax, 0x1301
	mov bx, 0x008c                          	;红底黑字，闪烁    
	mov cx, 24
	mov dx, 0x0200                          	;row 3 & col 0
	push ax
	mov ax, ds
	mov es, ax
	pop ax
	mov bp, _kernelfoundfail                    ;str "Error: KERNEL not found"
	int 0x10
	jmp $                                   	;没找到KERNEL死循环在这

	Search_Next_Sec:
	add word [_sectorno], 1                  	;search next sector
	jmp Search_In_Rootdir

	;--------------------- search FAT Ent -----------------------
Search_Ent_Success:
	;刚才找到了目录项->目录项找出fat表项的簇链->对应数据区的簇块
	;凡是进入这里，di总是指向目标目录项的偏址

	;找出文件在数据区的第一个扇区
	mov ax, RootDirSectors                  	;14
	and di, 0xffe0                          	;32位对齐，使di重新指向文件名头字符——目录项结构体头字段
	push eax
	mov eax, [es:di + 0x1c]
	cmp eax, KERNEL_LIMIT						;is kernel close to limits, 128Kb?
	ja Kernel_Error_Limit
	pop eax
	jmp Kernel_Load_Start
	Kernel_Error_Limit:
	mov ax, 0x1301
	mov bx, 0x008c
	mov cx, 24
	mov dx, 0x0200
	mov bp, _kernellargemess
	int 0x10
	jmp $
	Kernel_Load_Start:
	add di, 0x1a                            	;Dirstruct.FstClus 起始簇号
	;起始簇号+17+14 = 文件在数据区的第一个扇区
	mov cx, word [es:di]                    	;cx = 起始簇号，此时访问到了fat表项
	push cx
	add cx, ax
	add cx, SectorBalance                   	;cx = FAT表项与数据区对应簇之间的距离

	;把刚才找到的扇区读到内存中
	mov ax, KERNEL_SEG                      	;es:bx = Loader加载到内存的物理地址
	mov es, ax
	mov bx, KERNEL_OFFSET
	mov ax, cx                              	;数据区的起始簇号(或者说数据区的起始扇区)
	Load_File:
	push ax
	push bx
	;但在读数据区文件扇区这个过程中，每读一个扇区打印一个‘.’号
	;++++++++++++++++++ display a char ++++++++++++++++++++++++++
	;+                Call INT 10h, 0eh                         +
	;+          INP:    al = char                               +
	;+                  bl = attrubute                          +
	;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	mov ah, 0x0e
	mov al, '.'                             	;print '=' while read one sec
	mov bl, 0x0f
	int 0x10
	;调用__readsectors参数：ax = 扇区号，cl = 扇区数，es:bx = 磁盘的扇区读到内存哪里
	pop bx                                  	;Loader加载到内存的物址
	pop ax                                  	;文件在磁盘的起始扇区号
	mov cl, 1									;;;未来内核变大加载扇区数要改？？
	call __readsectors

	pop ax                                  	;ax = 起始簇号
	;__getfatent 返回下一个簇号
	call __getfatent                        	;return ah = fat no.
	cmp ax, 0x0fff                          	;FFFH 表示最后一个簇
	jz Load_Success
	push ax
	;下一个簇+17+14 = 文件在数据区的下一个扇区
	mov dx, RootDirSectors                  	;14
	add ax, dx
	add ax, SectorBalance                   	;cx = FAT表项与数据区对应簇之间的距离
	;es:bx 是数据从磁盘扇区传输到的内存地址
	;handling kernel beyond 64KB
	cmp bx, 0xfdff
	jnz Load_Continue							;no
	push ax
	mov ax, es
	add ax, 0x1000								;es point to next segment
	mov es, ax
	pop ax
	Load_Continue:
	add bx, [BPB_BytesPerSec]               	;每读完一个扇区增加512字节
	jmp Load_File

	Load_Success:
	mov eax, 0x1301
	mov ebx, 0x000f
	mov ecx, 12
	mov edx, 0x0200								;row 3
	push ax
	mov ax, ds
	mov es, ax
	pop ax
	mov bp, _kernelfoundsucc                    ;str "Start Kernel"
	int 0x10

	;--------------------- kill motor ------------------------
	push dx
	mov dx, 0x03f2
	mov al, 0
	out dx, al
	pop dx	

	;------------------------ MCR ----------------------------
	xor ebx, ebx
	mov ecx, 20
	mov di, _ardsbuff
Get_Available_Mem:
	mov eax, 0xe820
	mov edx, 0x534d4150
	int 0x15
	jc Get_Mem_Fail
	cmp ebx, 0
	jz Get_Mem_Success
	add di, 20
	inc word [_ardscount]
	jmp Get_Available_Mem

	Get_Mem_Fail:
	mov	ax,	0x1301
	mov	bx,	0x008c
	mov	cx,	20
	mov	dx,	0x0300								;row 4
	mov	bp,	_getmcrfailmess
	int	0x10
	jmp	$

	Get_Mem_Success:
	mov	ax,	0x1301
	mov	bx,	0x000f
	mov	cx,	16
	mov	dx,	0x0300								;row 4
	mov	bp,	_getmcrsuccmess
	int	0x10

	;---------------------- set focus ----------------------------
	; focus is located at line 6 & column 0
	; 设置在这里是“历史遗留问题”，我loader打印字符的函数和后面kernel
	; 打印字符函数不一样，这会造成光标会不一致，为了对得上后面kernel的
	; 光标，所以只能在这里 hard-code 这个光标地址 :-(
	mov ax, 0x200
	mov bx, 0
	mov dx, 0x0600
	int 0x10

	;-------------------- enter protect mode ---------------------
	;------------------- 1. open address A20 ---------------------
	push ax
	in al, 0x92
	or al, 0000_0010b
	out 0x92, al
	pop ax

	cli

	db 0x66
	lgdt [_gdtptr]

	;---------------------- 2. enable PM -------------------------- 
	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp dword SELECTOR_CODE:Protect_Mode + LOADER_PHY_ADDR

	[section .s32]
	align 32
	[bits 32]
	Protect_Mode:
	mov ax, SELECTOR_DATA
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov ss, ax
	mov esp, STACK_BASE

	mov ax, SELECTOR_VIDEO
	mov gs, ax

	;-------------------- calculate avail mem ---------------------
	call __calcumem
	call __printmemsize

	;------------------------- paging ---------------------------
	call __setuppaging

	sgdt [_gdtptr]

	mov ebx, [_gdtptr + 2]						;base addr point to gdt base
	or dword [ebx + 0x18 + 4], 0xc000_0000		;video desc maps to 0xc000_0000
	add dword [_gdtptr + 2], 0xc000_0000
	add esp, 0xc000_0000

	mov eax, PAGE_DIR_BASE
	mov cr3, eax

	mov eax, cr0
	or eax, 0x8000_0000
	mov cr0, eax

	lgdt [_gdtptr]

	;----------------------- enter kernel -----------------------
	call __initkernel
	jmp SELECTOR_CODE:KERNEL_ENTERPOINT

	jmp $

;====================== subprocessdure ==========================
	[section .s16lib]
	[bits 16]
	;--------------------- read disk sec ------------------------
    __readsectors:
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ;+                    Call INT 13h, 02h                     +
    ;+          INP:    al = sector nums                        +
    ;+                  ch = cylinder/track no.                 +
    ;+                  cl = sector no.                         +
    ;+                  dh = head no.                           +
    ;+                  dl = driver no.                         +
    ;+          FLAG:   CF = 1: error                           +
    ;+                  CF = 0: success                         +
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ;INP:   ax = LBA sector no.
    ;       cl = sector nums
    ;       es:bx = buffer
            push bp
            
            mov bp, sp                      	;manual push
            sub esp, 2
            mov byte [bp - 2], cl

            push bx                         	;protect destinct buffer addr
            mov bl, [BPB_SecPerTrk]
            div bl                          	;ax/bl = ah:al = 余:商
            inc ah                          	;余 = Sector no.
            mov cl, ah                      	;INT 13H-02H para
            mov dh, al                      	;protect 商
            shr al, 1                       	;商/磁面 = trk no. 右移1位相当于除以2
            mov ch, al
            and dh, 1                       	;and 1用作判奇偶，商为奇数对应1面，偶数0面
            pop bx

            mov dl, [BS_DrvNum]
        .labelrs1:
            mov ah, 2                       	;call INT 13H-02H
            mov al, byte [bp - 2]
            int 0x13
            jc .labelrs1                   		;continue when error happen

            add esp, 2
            pop bp
            ret

    ;------------------------ get FAT Ent ------------------------
    __getfatent:
    ;INP:   ax = FAT Ent no.
    ;OUT:   ax = FAT Ent no.
            push es
            push bx

            push ax
            mov ax, 0
            mov es, ax
            pop ax

            ;事实上，上一簇号 *3/2 就是当前fat项的地址，这个地址保存着值是下一簇号
            mov byte [_odd], 0               	;cacul ax*3/2 = 簇号*1.5
            mov bx, 3
            mul bx
            mov bx, 2
            div bx                          	;dx:ax = 余:商
            cmp dx, 0
            jz .even                         	;余数为偶数
            mov byte [_odd], 1
        .even:
            xor dx, dx
            mov bx, [BPB_BytesPerSec]       	;FAT表也是由一个一个簇组成的，也就是扇区的大小512
            div bx                          	;上一步的商除以512
                                            	;dx:ax = 余数:商 = 偏址:第几个扇区
            push dx
            ;因为刚才已经找完了目录项，并且通过该目录项拿到了起始簇号所以该地址内容可被覆盖
            mov bx, 0x8000
            add ax, SectorNoOfFAT           	;FAT表起始扇区号 = 1,ax是下一个fat表项所在的扇区
            mov cl, 2
            call __readsectors              	;预防FAT表项跨越两个扇区所以一次读2扇区

            pop dx
            add bx, dx                      	;bx是fat读到内存中的物址，dx是偏址

            mov ax, [es:bx]                 	;ax是下一簇号，实际ax中只有高12位有效
            cmp byte [_odd], 1
            jnz .even2                       	;偶数跳转
            shr ax, 4                       	;奇数需要先右偏移4位对齐
        .even2:
            and ax, 0x0fff                  	;清低4位

            pop bx
            pop es
            ret

	[section .s32lib]
	[bits 32]
	;--------------------- calculate available mem ----------------------
    __calcumem:
	;INP:   ARDS buffer
    ;OUT:   [AVAILABLEMEM]
	push esi
	push ecx
	push edx
	push edi

	mov esi, ARDSBUFF
	mov ecx, [ARDSCOUNT]
	.prepare:
	mov edx, 5
	mov edi, STRUCTARDS
	.calculstart:
	push dword [esi]
	pop eax
	cld											;DF = 0, edi + 4
	stosd										;ds:eax -> ds:edi
	add esi, 4
	dec edx
	cmp edx, 0
	jnz .calculstart							;fields is not over
	cmp dword [TYPE], 1
	jnz .calculfsh								;memory wont available
	mov eax, [BASEADDRLOW]
	add eax, [LENGTHLOW]
	cmp [AVAILABLEMEM], eax
	jge .calculfsh								;great or equal
	mov dword [AVAILABLEMEM], eax
	.calculfsh:
	loop .prepare

	pop edi
	pop edx
	pop ecx
	pop esi
	ret

	;------------------------ print str -----------------------------
    __printstr:
	;INP:   edi = print pos
	;		esi = str addr printing
    ;OUT:   none
	;NOTE:	caller need to reserve EAX cuz this function will change
	push edi
	push esi

	mov esi, [esp + 4 * 3]						;str addr
	mov edi, [DISPLAYPOS]						;screen offset
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
	mov dword [DISPLAYPOS], edi

	pop esi
	pop edi
	ret

	;------------------------ print hex -----------------------------
    __printhex:
	;INP:   al = number value converting
    ;OUT:   none
	push eax
	push ecx
	push edx
	push edi

	mov edi, [DISPLAYPOS]						;edi = screen offset
	mov ah, 0x0f
	mov dl, al
	mov ecx, 2
	shr al, 4									;convert high 4 at fst
	.convert:
	and al, 0x0f
	cmp al, 9
	ja .letter									;large than 9
	add al, '0'									;below 9 add '0'
	jmp .output
	.letter:
	sub al, 10									;beyond 9 subs 10 add 'a'
	add al, 'a'
	.output:
	mov [gs: edi], ax
	add edi, 2
	mov al, dl
	loop .convert
	mov [DISPLAYPOS], edi

	pop edi
	pop edx
	pop ecx
	pop eax
	ret

	;------------------------ print int -----------------------------
    __printint:
	;INP:   stack = int
    ;OUT:   none
	push eax
	push edi

	mov edi, [DISPLAYPOS]
	mov ah, 0x0f
	mov al, '0'
	mov [gs: edi], ax
	add edi, 2
	mov al, 'x'
	mov [gs: edi], ax
	add edi, 2
	mov [DISPLAYPOS], edi						;renew screen offset: edi

	mov eax, [esp + 4 * 3]						;one INT equal of double words
	shr eax, 24
	call __printhex

	mov eax, [esp + 4 * 3]
	shr eax, 16
	call __printhex

	mov eax, [esp + 4 * 3]
	shr eax, 8
	call __printhex

	mov eax, [esp + 4 * 3]
	call __printhex

	pop edi
	pop eax
	ret

	;----------------------- print mem size ---------------------------
    __printmemsize:
	;INP:   none
    ;OUT:   none
	;NOTE:	in units of KB, and omits value less than 1KB
	push eax
	push ebx
	push edx

	xor edx, edx
	mov eax, [AVAILABLEMEM]
	mov ebx, 1024
	div ebx										;retain EAX

	push eax									;__printstr will change EAX
	push MEMORYSIZE
	call __printstr								;display "Memory size: "
	add esp, 4

	call __printint
	add esp, 4

	push MEMORYUNIT
	call __printstr								;display "KB"
	add esp, 4

	pop edx
	pop ebx
	pop eax
	ret

	;----------------------- setup paging ----------------------------
    __setuppaging:
	;INP:   none
    ;OUT:   none
	;NOTE:	[PAGE_DIR_BASE] 
	push ebx
	push ecx
	push esi
	
	.createpde:
	mov eax, PAGE_DIR_BASE						;eax = pde
	add eax, 0x1000
	mov ebx, eax								;back up ebx = 0x0010_1000
	or eax, 0000_0111b							;fill in pde[0]/[c00] = 0x0010_1111
	mov [PAGE_DIR_BASE], eax
	mov [PAGE_DIR_BASE + 0xc00], eax			;0xc00 high 10-bits = 0x300 = 768
	sub eax, 0x1000								;fill in pde[1023] = 0x0010_0111
	mov [PAGE_DIR_BASE + 0xffc], eax

	mov ecx, 256
	mov esi, 0
	xor eax, eax
	or eax, 0000_0111b							;eax = 0x0000_0111
	.createpte:
	mov [ebx + esi], eax						;ebx = 0x0010_1000 <- eax = 0x0000_x111
	add eax, 0x1000
	add esi, 4
	loop .createpte

	mov ecx, 254								;from c01 ~ ff8
	sub ebx, 0x1000								;base addr point to pde
	mov esi, 0xc04								;offset addr point to pde index
	mov eax, PAGE_DIR_BASE
	add eax, 0x2000
	or eax, 0000_0111b							;eax = 0x0010_2111
	.createrestpde:
	mov [ebx + esi], eax
	add eax, 0x1000
	add esi, 4
	loop .createrestpde

	push PAGINGMESS
	call __printstr								;display "Paging get success!"
	add esp, 4

	pop esi
	pop ecx
	pop ebx
	ret

	;----------------------- copy memory to ----------------------------
    __memcpy:
	;INP:   stack 1 = size
	;		stack 2 = src
	;		stack 3 = dst
    ;OUT:   virtual addr this seg in memory
	;NOTE:	┬────┬
	;		│size│
	;		├────┤
	;		│ src│
	;		├────┤
	;		│ dst│
	;		├────┤
	;		│ RET│
	;		├────┤
	;		│ ecx│
	;		├────┤
	;		│ edi│
	;		├────┤
	;		│ esi│
	;		├────┤ <──esp
	push ecx
	push edi
	push esi

	mov edi, [esp + 4 * 4]						;edi = dst
	mov esi, [esp + 4 * 5]						;esi = src
	mov ecx, [esp + 4 * 6]						;ecx = size
	cld
	rep movsb									;ds:esi -> ds:edi

	pop esi
	pop edi
	pop ecx
	ret

	;-------------------- initialize kernel --------------------------
    __initkernel:
	;INP:   stack 1 = size
	;		stack 2 = src
	;		stack 3 = dst
    ;OUT:   virtual addr this seg in memory
	push ebx
	push ecx
	push edx

	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	mov ebx, [KERNEL_PHY_ADDR + 28]				;e_phoff: Prog Head Table phy mem
	add ebx, KERNEL_PHY_ADDR
	mov cx, [KERNEL_PHY_ADDR + 44]				;e_phnum: PHT entry amount
	mov dx, [KERNEL_PHY_ADDR + 42]				;e_phentsize: each entry size of PHT
	.movekernel:
	cmp dword [ebx + 0], 0						;if PT_NULL?
	jz .nextmove
	push dword [ebx + 16]
	mov eax, [ebx + 4]
	add eax, KERNEL_PHY_ADDR
	push eax
	push dword [ebx + 8]
	call __memcpy
	add esp, 12
	.nextmove:
	add ebx, edx
	loop .movekernel

	pop edx
	pop ecx
	pop ebx
	ret

;============================== label ================================
[section .data16]
align 32
_sectorno:          dw 0
_rootdirloop: 		dw RootDirSectors
_odd:               db 0
_displaypos:		dd (80 * 4 + 0) * 2

_kernelname:        db "KERNEL  BIN", 0
_startloadermess:	db "Start Loader"
_kernellargemess:	db "ERROR: Kernel Too Large!"
_getmcrfailmess:	db "ERROR: MCR get fail!"
_getmcrsuccmess:	db "MCR get success!"
_kernelfoundfail:   db "Error: KERNEL not found!"
_kernelfoundsucc:	db "Start Kernel"
_memorysize:		db "Memory size: ", 0
_memoryunit:		db " KB", 10, 0
_pagingmess:		db "Paging get success!", 0

_availablemem:		dd	0
_structards:
	_baseaddrlow:	dd	0
	_baseaddrhigh:	dd	0
	_lengthlow:		dd	0
	_lengthhigh:	dd	0
	_type:			dd	0

_ardscount:			dw	0
_ardsbuff:			times 256 db 0

_stackspace:		times 0x1000 db 0

[section .data32]
align 32

DISPLAYPOS			equ LOADER_PHY_ADDR + _displaypos
MEMORYSIZE			equ LOADER_PHY_ADDR + _memorysize
MEMORYUNIT			equ LOADER_PHY_ADDR + _memoryunit
PAGINGMESS			equ LOADER_PHY_ADDR + _pagingmess
AVAILABLEMEM		equ LOADER_PHY_ADDR + _availablemem
STRUCTARDS			equ LOADER_PHY_ADDR + _structards
	BASEADDRLOW		equ LOADER_PHY_ADDR + _baseaddrlow
	BASEADDRHIGH	equ LOADER_PHY_ADDR + _baseaddrhigh
	LENGTHLOW		equ LOADER_PHY_ADDR + _lengthlow
	LENGTHHIGH		equ LOADER_PHY_ADDR + _lengthhigh
	TYPE			equ LOADER_PHY_ADDR + _type

ARDSCOUNT			equ LOADER_PHY_ADDR + _ardscount
ARDSBUFF			equ LOADER_PHY_ADDR + _ardsbuff
STACK_BASE			equ LOADER_PHY_ADDR + $