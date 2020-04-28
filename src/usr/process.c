#include "process.h"
#include "global.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"    
#include "list.h"    
#include "tss.h"    
#include "interrupt.h"
#include "string.h"
#include "console.h"

extern void __intrext(void);

void __startProcess(void* filename_){           //填充线程中断栈为用户进程的上下文
    __consolePrintStr("\nhere\n");
    void* function = filename_;                 //进程名即用户名：因为进程通常从文件系统加载
    struct pcb* cur = __thdAddr();              //获取当前线程pcb
    cur->kStack += sizeof(struct pcb);          //跨过线程栈
    struct intrStack* procStack = (struct intrStack*)cur->kStack;
    //填充中断栈	 
    procStack->edi = procStack->esi = procStack->ebp = procStack->espDummy = 0;
    procStack->ebx = procStack->edx = procStack->ecx = procStack->eax = 0;
    procStack->gs = 0;
    procStack->ds = procStack->es = procStack->fs = SELECTOR_U_DATA;
    procStack->eip = function;
    procStack->cs = SELECTOR_U_CODE;
    procStack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    //3级栈（用户空间最高处）
    procStack->esp = (void*)((uint32)__apageAlloc(PF_USER, USER_STACK3_VADDR) + PG_SIZE);
    procStack->ss = SELECTOR_U_DATA; 
    asm volatile ("movl %0, %%esp; jmp __intrext" : : "g" (procStack) : "memory");
}

void __pageDirActivate(struct pcb* pThread) {
    uint32 kpageDir = 0x100000;                                     //内核使用的页表
    if (pThread->ptvAddr)	{                               //若为进程，将进程的页表地址加载到cr3
        kpageDir = __vir2phy((uint32)pThread->ptvAddr);             //返回虚拟地址所映射的物理地址
    }
    // __consolePrintStr("\nswitch page table: ");
    // __consolePrintInt(kpageDir);
    asm volatile ("movl %0, %%cr3" : : "r" (kpageDir) : "memory");  //切换页目录表
    //asm volatile ("invlpg (%0)" ::"r" (addr) : "memory");
}

void __processActivate(struct pcb* pThread){
    ASSERT(pThread != NULL);
    //如果是线程：页表为0x10_0000
    //如果是进程：页表从pcb结构取ptvAddr成员
    __pageDirActivate(pThread);

    if(pThread->ptvAddr){                   //用户进程才更新tss的0级栈
        __updateTssEsp(pThread);            //栈移到pcb那一页最顶端
    }
    // __consolePrintStr("\nstack thread: ");
    // __consolePrintInt(pThread);
}

uint32* __createPageDir(void){          //将内核页目录项复制到用户进程的页目录表
    uint32* pdvAddr = __kpageAlloc(1);      //用作用户进程的页目录表
    if(!pdvAddr){
        __consolePrintStr("ERROR: __createPageDir : __kpageAlloc failed in process.c!\n");
        return NULL;
    }

    //                  刚申请的页目录表  内核页目录项偏移   访问内核页目录表 内核页目录项偏移 1024字节即256个页目录项
    __memcpy((uint32*)((uint32)pdvAddr + 0x300 * 4), (uint32*)(0xfffff000 + 0x300 * 4), 1024);
    uint32 newPdpAddr = __vir2phy((uint32)pdvAddr);
    //用户页目录表最后一项指向自身：原因是缺页时能补充
    pdvAddr[1023] = newPdpAddr | PG_US_U | PG_RW_W | PG_P_1;

    return pdvAddr;
}

void __createUservAddrBtmp(struct pcb* usrProg){
    usrProg->usrvAddr.vaddrBase = USER_VADDR_START;         //位图所管理的内存空间的起始地址（即用户程序入口地址）
    //位图所需要的物理页
    uint32 btmpPgCnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    usrProg->usrvAddr.vaddrBtmp._bits = __kpageAlloc(btmpPgCnt);        //为位图分配内存
    usrProg->usrvAddr.vaddrBtmp._btmpBytesLen = (0xc0000000 - USER_VADDR_START) / PG_SIZE / 8;      //位图长度
    __btmpInit(&usrProg->usrvAddr.vaddrBtmp);               //主要是补0
}

void __processExecute(void* filename, char* name){
    struct pcb* thread = __kpageAlloc(1);
    //__consolePrintInt(thread);
    __initThread(thread, name, DEFAULT_PRIO);
    __createUservAddrBtmp(thread);
    __createThread(thread, __startProcess, filename);
    thread->ptvAddr = __createPageDir();
    // __consolePrintStr("\npage table is in: ");
    // __consolePrintInt(thread->ptvAddr);
    // while(1);
    
    enum _intrStatus oldStatus = __intrDisable();
    ASSERT(!__listNodeFind(&thdReadyList, &thread->genNode));
    __listAppend(&thdReadyList, &thread->genNode);

    ASSERT(!__listNodeFind(&thdList, &thread->thdNode));
    __listAppend(&thdList, &thread->thdNode);
    __intrSetStatus(oldStatus);
}