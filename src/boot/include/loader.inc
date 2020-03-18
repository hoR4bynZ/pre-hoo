
;======================= kernel loader point =============================
KERNEL_ENTERPOINT               equ 0x1000

;=========================== page dir addr ===============================
PAGE_DIR_BASE                   equ 0x10_0000
PAGE_TABLE_BASE                 equ 0x10_1000

;=========================== loader addr =================================
LOADER_SEG                      equ 0x9000
LOADER_OFFSET                   equ 0x100
LOADER_PHY_ADDR                 equ LOADER_SEG * 0x10

;=========================== kernel addr =================================
KERNEL_SEG                      equ 0x7000
KERNEL_OFFSET                   equ 0
KERNEL_PHY_ADDR                 equ KERNEL_SEG * 0x10
KERNEL_LIMIT                    equ LOADER_PHY_ADDR - KERNEL_PHY_ADDR

;============================ boot parms =================================
BOOTPARAM_ADDR                  equ 0x700
BOOTPARAM_MAGIC                 equ 0x328