#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "thread.h"

#define PG_SIZE 4096

static void __kThread (__threadFunc* function, void* funcArg) {
    // __kThread() 创建出来的线程去调用的函数
    function(funcArg);
}

void __createThread (struct pcb* pthread, __threadFunc function, void* funcArg) {
    pthread->kStack -= sizeof(struct intrStack);
    pthread->kStack -= sizeof(struct thdStack);
    struct thdStack* kStack = (struct thdStack*)pthread->kStack;
    kStack->eip = __kThread;
    kStack->function = function;
    kStack->funcArg = funcArg;
    kStack->ebp = 0;
    kStack->ebx = 0;
    kStack->edi = 0;
    kStack->esi = 0;
}

void __initThread (struct pcb* pthread, char* name, int prio) {
    __memset(pthread, 0, sizeof(*pthread));
    __strcpy(pthread->name, name);
    pthread->status = RUNNING;
    pthread->priority = prio;
    pthread->kStack = (uint32*)((uint32)pthread + PG_SIZE);
    pthread->magicNumber = 0x97321679;
}

struct pcb* __startThread (char* name, int prio, __threadFunc function, void* funcArg) {
    struct pcb* thread = __kpageAlloc(1);

    __initThread(thread, name, prio);
    __createThread(thread, function, funcArg);

    /* 线程栈 thdStack 前4个成员出栈
     * 因为结构体的定义顺序在内存中是从低至高，而栈是从高向低长的，所以栈顶对应结构体第一个成员
     * mov esp, 0
     * pop ebp
     * pop ebx
     * pop edi
     * pop esi
     * ret
     */
    asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->kStack) : "memory");
    return thread;
}