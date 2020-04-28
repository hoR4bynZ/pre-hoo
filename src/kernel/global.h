#ifndef __KERNEL_GLOBAL_H
#define __KERNEL_GLOBAL_H
#include "stdint.h"

/*-------------------- Kernel Selector ---------------------*/
#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT 0
#define TI_LDT 1

#define SELECTOR_K_CODE     (uint16)((1 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_DATA     (uint16)((2 << 3) + (TI_GDT << 2) + RPL0)
#define SELECTOR_K_STACK    SELECTOR_K_DATA
#define SELECTOR_K_GS       (uint16)((3 << 3) + (TI_GDT << 2) + RPL0)
/*-------------------- User Selector ---------------------*/
#define SELECTOR_U_CODE	    (uint16)((5 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_U_DATA	    (uint16)((6 << 3) + (TI_GDT << 2) + RPL3)
#define SELECTOR_U_STACK    SELECTOR_U_DATA

/*-------------------- GDT Descriptor ---------------------*/
#define DESC_G_4K    1
#define DESC_D_32    1
#define DESC_L	      0
#define DESC_AVL     0
#define DESC_P	      1
#define DESC_DPL_0   0
#define DESC_DPL_1   1
#define DESC_DPL_2   2
#define DESC_DPL_3   3

#define DESC_S_CODE	   1
#define DESC_S_DATA	   DESC_S_CODE
#define DESC_S_SYS	   0
#define DESC_TYPE_CODE	8	// x=1,c=0,r=0,a=0 代码段是可执行的,非依从的,不可读的,已访问位a清0
#define DESC_TYPE_DATA  2	// x=0,e=0,w=1,a=0 数据段是不可执行的,向上扩展的,可写的,已访问位a清0
#define DESC_TYPE_TSS   9	// B位为0,不忙

#define GDT_ATTR_HIGH            ((DESC_G_4K << 7) + (DESC_D_32 << 6) + (DESC_L << 5) + (DESC_AVL << 4) + 0x0)

#define GDT_CODE_ATTR_LOW_DPL3	((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_CODE << 4) + DESC_TYPE_CODE)
#define GDT_DATA_ATTR_LOW_DPL3	((DESC_P << 7) + (DESC_DPL_3 << 5) + (DESC_S_DATA << 4) + DESC_TYPE_DATA)

/*--------------- TSS descriptor attribute ----------------*/
#define TSS_DESC_D  0 

#define TSS_ATTR_HIGH   ((DESC_G_4K << 7) + (TSS_DESC_D << 6) + (DESC_L << 5) + (DESC_AVL << 4) + 0x0)
#define TSS_ATTR_LOW    ((DESC_P << 7) + (DESC_DPL_0 << 5) + (DESC_S_SYS << 4) + DESC_TYPE_TSS)
#define SELECTOR_TSS    (uint16)((4 << 3) + (TI_GDT << 2) + RPL0)

struct gdt {
   uint16 limitLowWord;
   uint16 baseLowWord;
   uint8  baseMidByte;
   uint8  attrLowByte;
   uint8  limitHighAttrHigh;
   uint8  baseHighByte;
}; 

/*--------------- IDT descriptor attribute ----------------*/
#define IDT_DESC_P          1
#define IDT_DESC_DPL0       0
#define IDT_DESC_DPL3       3
#define IDT_DESC_32_TYPE    0xe
#define IDT_DESC_16_TYPE    0x6
#define IDT_DESC_ATTR_DPL0  ((IDT_DESC_P << 7) + (IDT_DESC_DPL0 << 5) + IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3  ((IDT_DESC_P << 7) + (IDT_DESC_DPL3 << 5) + IDT_DESC_32_TYPE)

#define PG_SIZE 4096

/*------------------ eflags attribute --------------------*/
/********************************************************
--------------------------------------------------------------
            Intel 8086 Eflags Register
--------------------------------------------------------------
*
*     15|14|13|12|11|10|F|E|D C|B|A|9|8|7|6|5|4|3|2|1|0|
*      |  |  |  |  |  | | |  |  | | | | | | | | | | | '---  CF……Carry Flag
*      |  |  |  |  |  | | |  |  | | | | | | | | | | '---  1 MBS
*      |  |  |  |  |  | | |  |  | | | | | | | | | '---  PF……Parity Flag
*      |  |  |  |  |  | | |  |  | | | | | | | | '---  0
*      |  |  |  |  |  | | |  |  | | | | | | | '---  AF……Auxiliary Flag
*      |  |  |  |  |  | | |  |  | | | | | | '---  0
*      |  |  |  |  |  | | |  |  | | | | | '---  ZF……Zero Flag
*      |  |  |  |  |  | | |  |  | | | | '---  SF……Sign Flag
*      |  |  |  |  |  | | |  |  | | | '---  TF……Trap Flag
*      |  |  |  |  |  | | |  |  | | '---  IF……Interrupt Flag
*      |  |  |  |  |  | | |  |  | '---  DF……Direction Flag
*      |  |  |  |  |  | | |  |  '---  OF……Overflow flag
*      |  |  |  |  |  | | |  '----  IOPL……I/O Privilege Level
*      |  |  |  |  |  | | '-----  NT……Nested Task Flag
*      |  |  |  |  |  | '-----  0
*      |  |  |  |  |  '-----  RF……Resume Flag
*      |  |  |  |  '------  VM……Virtual Mode Flag
*      |  |  |  '-----  AC……Alignment Check
*      |  |  '-----  VIF……Virtual Interrupt Flag  
*      |  '-----  VIP……Virtual Interrupt Pending
*      '-----  ID……ID Flag
*
*
**********************************************************/
#define EFLAGS_MBS	   (1 << 1)
#define EFLAGS_IF_1	   (1 << 9)	   // if为1,开中断
#define EFLAGS_IF_0	   0		      // if为0,关中断
#define EFLAGS_IOPL_3   (3 << 12)	// IOPL3
#define EFLAGS_IOPL_0	(0 << 12)	// IOPL0

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

#endif