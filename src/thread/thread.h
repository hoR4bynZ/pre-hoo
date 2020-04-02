#ifndef __THREAD_THREAD_H
#define __THREAD_THREAD_H
#include "stdint.h"

typedef void __threadFunc(void*);

enum procStatus {
    RUNNING,            //running
    READY,              //ready
    BLOCKED,            //blocked
    WAITING,            //waiting
    HANGING,            //hanging
    DIED                //died
};

//中断栈
struct intrStack {
    uint32 vecNo;
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 espDummy;
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;
    uint32 gs;
    uint32 fs;
    uint32 es;
    uint32 ds;

    // 除非转移特权否则不用
    uint32 err_code;
    void (*eip) (void);
    uint32 cs;
    uint32 eflags;
    void* esp;
    uint32 ss;
};

//线程栈
struct thdStack {
    uint32 ebp;
    uint32 ebx;
    uint32 edi;
    uint32 esi;

    // eip => 调度器调用任务切换函数后，调用函数的返回地址
    void (*eip) (__threadFunc* func, void* funcArg);
    void (*unused);
    __threadFunc* function;
    void* funcArg;
};

//pcb
struct pcb {
    uint32* kStack;     //线程内核栈栈顶指针
    enum procStatus status;
    uint8 priority;
    char name[16];
    uint32 magicNumber;
};

void __createThread(struct pcb* pthread, __threadFunc function, void* funcArg);
void __initThread(struct pcb* pthread, char* name, int prio);
struct pcb* __startThread(char* name, int prio, __threadFunc function, void* funcArg);
#endif