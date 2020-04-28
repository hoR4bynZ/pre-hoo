RootDirSectors      equ 14                  ;BPB_RootEntCnt * 32bytes / 512
SectorNoOfRootDir   equ 19                  ;root dir start sector no.
SectorNoOfFAT       equ 1                   ;FAT表是整个软盘的第一个扇区
SectorBalance       equ 17

    BS_OEMName	    db	'---XX---'          ;8 bits, maker name
	BPB_BytesPerSec	dw	512
	BPB_SecPerClus	db	1
	BPB_RsvdSecCnt	dw	1                   ;reserved Sector
	BPB_NumFATs	    db	2
	BPB_RootEntCnt	dw	224                 ;Root Dir entry counts
	BPB_TotSec16	dw	2880                ;for floppy: 2 * 80 * 18
	BPB_Media	    db	0xf0                ;media type: 0xf0 = portable
	BPB_FATSz16	    dw	9                   ;FAT table size
	BPB_SecPerTrk	dw	18
	BPB_NumHeads	dw	2
	BPB_HiddSec	    dd	0                   ;hide sectors
	BPB_TotSec32	dd	0
	BS_DrvNum	    db	0                   ;Drive num
	BS_Reserved1	db	0
	BS_BootSig	    db	0x29
	BS_VolID	    dd	0
	BS_VolLab	    db	'horbynz OS '       ;11 bits
	BS_FileSysType	db	'FAT12   '          ;8 bits