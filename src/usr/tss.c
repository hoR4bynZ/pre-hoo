#include "tss.h"
#include "stdint.h"
#include "global.h"
#include "string.h"
#include "print.h"
#include "console.h"
#include "thread.h"

struct tss {                //tss struct
    uint32 backlink;
    uint32* esp0;
    uint32 ss0;
    uint32* esp1;
    uint32 ss1;
    uint32* esp2;
    uint32 ss2;
    uint32 cr3;
    uint32 (*eip) (void);
    uint32 eflags;
    uint32 eax;
    uint32 ecx;
    uint32 edx;
    uint32 ebx;
    uint32 esp;
    uint32 ebp;
    uint32 esi;
    uint32 edi;
    uint32 es;
    uint32 cs;
    uint32 ss;
    uint32 ds;
    uint32 fs;
    uint32 gs;
    uint32 ldt;
    uint32 trace;
    uint32 ioBase;
}; 
static struct tss tss;

void __updateTssEsp(struct pcb* pthread) {      //更新tss的0级栈，指向内核态的线程栈
    // __consolePrintStr("\n(update tss)thread is: ");
    // __consolePrintInt(pthread);
    // __consolePrintStr("\n(update tss)thread kstack is: ");
    // __consolePrintInt(pthread->kStack);
    tss.esp0 = (uint32*)((uint32)pthread + PG_SIZE);
    // __consolePrintStr("\n(update tss)size of intrStack is: ");
    // __consolePrintInt(sizeof(struct intrStack));
    // __consolePrintStr("\n(update tss)size of thdStack is: ");
    // __consolePrintInt(sizeof(struct thdStack));
    // __consolePrintStr("\n(update tss)tss esp0 is: ");
    // __consolePrintInt(tss.esp0);
}

static struct gdt __makeGdtDesc(uint32* descAddr, uint32 limit, uint8 attrLow, uint8 attrHigh) {
    //构造gdt描述符表
    uint32 descBase = (uint32)descAddr;
    struct gdt desc;
    desc.limitLowWord = limit & 0x0000ffff;
    desc.baseLowWord = descBase & 0x0000ffff;
    desc.baseMidByte = ((descBase & 0x00ff0000) >> 16);
    desc.attrLowByte = (uint8)(attrLow);
    desc.limitHighAttrHigh = (((limit & 0x000f0000) >> 16) + (uint8)(attrHigh));
    desc.baseHighByte = descBase >> 24;
    return desc;
}

void __initTss(){               //初始化tss，并往gdt安装①tss；②用户代码段；③用户数据段
    __printstr("TSS Initialization start:\n");
    uint32 tssSize = sizeof(tss);
    __memset(&tss, 0, tssSize);
    tss.ss0 = SELECTOR_K_STACK; //赋0级栈选择子 #2
    tss.ioBase = tssSize;       //IO位图偏移 = tss大小，相当于没有IO位图
    
    //gdt安装在0x90140，把tss放到第4个位置（0x90160）
    *((struct gdt*)0xc0090160) = __makeGdtDesc((uint32*)&tss, tssSize - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);    //#4
    *((struct gdt*)0xc0090168) = __makeGdtDesc((uint32*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH); //#5
    *((struct gdt*)0xc0090170) = __makeGdtDesc((uint32*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH); //#6

    //重新加载gdt
    uint64 gdtOperand = ((8 * 7 - 1) | ((uint64)(uint32)0xc0090140 << 16));
    asm volatile ("lgdt %0" : : "m"(gdtOperand));
    asm volatile ("ltr %w0" : : "r"(SELECTOR_TSS));
    __printstr("    TSS Initialization success!\n");
}