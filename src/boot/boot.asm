    org 0x7c00

StackBase           equ 0x7c00
LoaderBase          equ 0x9000
LoaderOffset        equ 0x100

    jmp short Laber_Start
    nop
    %include "fat12.inc"

Laber_Start:                                ;# +62
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, StackBase

;=========================== scroll screen ==========================
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;+                    Call  INT 10h, 06h                            +
;+                 INP:    al = scroll col                          +
;+                         bh = attribute                           +
;+            (cl, ch) = row, col of upper left centre              +
;+           (dl, dh) = row, col of lower rigth centre              +
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mov ax, 0x0600
    mov bx, 0x0700
    mov cx, 0
    mov dx, 0x184f                          ;24*79, 24/16=0x18, 79/16=0x4f
    int 0x10

;============================= set focus ============================
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;+                    Call  INT 10h, 02h                            +
;+              INP:   bh = display page                            +
;+              (dl, dh) = row, col of cursor                       +
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mov ax, 0x0200
    mov bx, 0
    mov dx, 0
    int 0x10

;========================== display str =============================
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;+                    Call INT 10h, 13h                             +
;+INP:    al = write mode--01 is usual--cursor move by str tail     +
;+                          bh = page                               +
;+                        bl = attribute                            +
;+                       cx = string length                         +
;+               (dh, dl) = row, col of cursor pos                  +
;+                        es:bp = str pos                           +
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mov ax, 0x1301
    mov bx, 0x000f
    mov cx, 10
    mov dx, 0                               ;row 0 & col 0
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, BootMessage                     ;str "Start Boot"
    int 0x10

;========================== reset floppy ===============================
    xor ah, ah
    xor dl, dl                              ;00 = no.1 floppy driver
    int 0x13

;=========================== search LOADER =============================
    mov word [SectorNo], SectorNoOfRootDir  ;19

Search_In_Rootdir:
    cmp word [RootDirSizeForLoop], 0        ;cmp 14 & 0
    jz Loader_Fail
    dec word [RootDirSizeForLoop]

    mov ax, 0
    mov es, ax
    mov bx, 0x8000                          ;buffer = es:bx = 0:8000 = 0x8000
    mov ax, [SectorNo]                      ;19
    mov cl, 1                               ;相当于上面的dec,减去一个扇区数
    call __readsectors                      ;将软盘第19个扇区内容读取到内存0x8000

    ;seek every entrys from buffer to find LOADER
    mov dx, 0x10                            ;entrys per sector = 512 / 32 = 16
    mov di, 0x8000                          ;等会对比文件名时使用

    ;找一个扇区全部16个目录项
    Search_Loader:
    cmp dx, 0
    jz Search_Next_Sec                      ;找完一个扇区全部16个目录项
    dec dx
    mov si, LoaderName                      ;LODSB inp: ds:si = des data addr
    cld                                     ;      out: al
    mov cx, 11                              ;file name length

    ;找文件名全部11个字符
    Cmp_Filename:
    cmp cx, 0
    jz Search_Ent_Success                   ;如果文件名全部匹配就是找到了目录项
    dec cx
    lodsb                                   ;move data from ds:si to al
    cmp al, byte [es:di]                    ;move 1 byte each loop
    jz Cmp_Char_Same                        ;字符相同，继续找下一个字符，所以回到这个循环
    jmp Cmp_Char_Diff                       ;字符有一个不相同，就不用找这个目录项，所以找下一个目录项

    Cmp_Char_Same:
    inc di                                  ;read next char
    jmp Cmp_Filename

    Cmp_Char_Diff:
    and di, 0xffe0                          ;32位对齐，让di重新指到目录项第一个字段，因为di当前值在上一次比较文件名时有递增
    add di, 20h                             ;cmp next entry
    ;mov si, LoaderName
    jmp Search_Loader

    Loader_Fail:
    ;========================== display str =============================
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ;+                    Call INT 10h, 13h                             +
    ;+INP:    al = write mode--01 is usual--cursor move by str tail     +
    ;+                          bh = page                               +
    ;+                        bl = attribute                            +
    ;+                       cx = string length                         +
    ;+               (dh, dl) = row, col of cursor pos                  +
    ;+                        es:bp = str pos                           +
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mov ax, 0x1301
    mov bx, 0x008c                          ;红底黑字，闪烁    
    mov cx, 23
    mov dx, 0x0100                          ;row 1 & col 0
    push ax
    mov ax, ds
    mov es, ax
    pop ax
    mov bp, LoaderFail                      ;str "Error: LOADER not found"
    int 0x10
    jmp $                                   ;没找到LOADER死循环在这

    Search_Next_Sec:
    add word [SectorNo], 1                  ;search next sector
    jmp Search_In_Rootdir

;=========================== search FAT Ent =============================
Search_Ent_Success:
    ;刚才找到了目录项->目录项找出fat表项的簇链->对应数据区的簇块
    ;凡是进入这里，di总是指向目标目录项的偏址

    ;找出文件在数据区的第一个扇区
    mov ax, RootDirSectors                  ;14
    and di, 0xffe0                          ;32位对齐，使di重新指向文件名头字符——目录项结构体头字段
    add di, 0x1a                            ;Dirstruct.FstClus 起始簇号
    ;起始簇号+17+14 = 文件在数据区的第一个扇区
    mov cx, word [es:di]                    ;cx = 起始簇号，此时访问到了fat表项
    push cx
    add cx, ax
    add cx, SectorBalance                   ;cx = FAT表项与数据区对应簇之间的距离

    ;把刚才找到的扇区读到内存中
    mov ax, LoaderBase                      ;es:bx = Loader加载到内存的物理地址
    mov es, ax
    mov bx, LoaderOffset
    mov ax, cx                              ;数据区的起始簇号(或者说数据区的起始扇区)
    Load_File:
    push ax
    push bx
    ;但在读数据区文件扇区这个过程中，每读一个扇区打印一个‘.’号
    ;====================== display a char ==============================
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ;+                    Call INT 10h, 0eh                             +
    ;+              INP:    al = char                                   +
    ;+                      bl = attrubute                              +
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mov ah, 0x0e
    mov al, '.'                             ;print '=' while read one sec
    mov bl, 0x0f
    int 0x10
    ;调用__readsectors参数：ax = 扇区号，cl = 扇区数，es:bx = 磁盘的扇区读到内存哪里
    pop bx                                  ;Loader加载到内存的物址
    pop ax                                  ;文件在磁盘的起始扇区号
    mov cl, 1
    call __readsectors

    pop ax                                  ;ax = 起始簇号
    ;__getfatent 返回下一个簇号
    call __getfatent                        ;return ah = fat no.
    cmp ax, 0x0fff                          ;FFFH 表示最后一个簇
    jz Load_Success
    push ax
    ;下一个簇+17+14 = 文件在数据区的下一个扇区
    mov dx, RootDirSectors                  ;14
    add ax, dx
    add ax, SectorBalance                   ;cx = FAT表项与数据区对应簇之间的距离
    ;es:bx 是数据从磁盘扇区传输到的内存地址
    add bx, [BPB_BytesPerSec]               ;每读完一个扇区增加512字节
    jmp Load_File

    Load_Success:
    jmp LoaderBase:LoaderOffset

    ;========================= read disk sec ============================
    __readsectors:
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ;+                    Call INT 13h, 02h                             +
    ;+              INP:    al = sector nums                            +
    ;+                      ch = cylinder/track no.                     +
    ;+                      cl = sector no.                             +
    ;+                      dh = head no.                               +
    ;+                      dl = driver no.                             +
    ;+              FLAG:   CF = 1: error                               +
    ;+                      CF = 0: success                             +
    ;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ;INP:   ax = LBA sector no.
    ;       cl = sector nums
    ;       es:bx = buffer
            push bp
            
            mov bp, sp                      ;manual push
            sub esp, 2
            mov byte [bp - 2], cl

            push bx                         ;protect destinct buffer addr
            mov bl, [BPB_SecPerTrk]
            div bl                          ;ax/bl = ah:al = 余:商
            inc ah                          ;余 = Sector no.
            mov cl, ah                      ;INT 13H-02H para
            mov dh, al                      ;protect 商
            shr al, 1                       ;商/磁面 = trk no. 右移1位相当于除以2
            mov ch, al
            and dh, 1                       ;and 1用作判奇偶，商为奇数对应1面，偶数0面
            pop bx

            mov dl, [BS_DrvNum]
        .labelrs1:
            mov ah, 2                       ;call INT 13H-02H
            mov al, byte [bp - 2]
            int 0x13
            jc .labelrs1                   ;continue when error happen

            add esp, 2
            pop bp
            ret

    ;=========================== get FAT Ent ===========================
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
            mov byte [Odd], 0               ;cacul ax*3/2 = 簇号*1.5
            mov bx, 3
            mul bx
            mov bx, 2
            div bx                          ;dx:ax = 余:商
            cmp dx, 0
            jz Even                         ;余数为偶数
            mov byte [Odd], 1

        Even:
            xor dx, dx
            mov bx, [BPB_BytesPerSec]       ;FAT表也是由一个一个簇组成的，也就是扇区的大小512
            div bx                          ;上一步的商除以512
                                            ;dx:ax = 余数:商 = 偏址:第几个扇区
            push dx
            ;因为刚才已经找完了目录项，并且通过该目录项拿到了起始簇号所以该地址内容可被覆盖
            mov bx, 0x8000
            add ax, SectorNoOfFAT           ;FAT表起始扇区号 = 1,ax是下一个fat表项所在的扇区
            mov cl, 2
            call __readsectors              ;预防FAT表项跨越两个扇区所以一次读2扇区

            pop dx
            add bx, dx                      ;bx是fat读到内存中的物址，dx是偏址

            mov ax, [es:bx]                 ;ax是下一簇号，实际ax中只有高12位有效
            cmp byte [Odd], 1
            jnz Even2                       ;偶数跳转
            shr ax, 4                       ;奇数需要先右偏移4位对齐

        Even2:
            and ax, 0x0fff                  ;清低4位

            pop bx
            pop es
            ret

;=========================== data =======================================
SectorNo                dw 0
RootDirSizeForLoop      dw RootDirSectors
Odd                     db 0

BootMessage:            db "Start Boot"
LoaderFail:             db "Error: LOADER not found"
LoaderName:             db "LOADER  BIN", 0

times 510 - ($ - $$)    db 0
                        dw 0xaa55