#include "stdint.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "thread.h"
#include "interrupt.h"
#include "debug.h"
#include "print.h"

#define PG_SIZE 4096

struct pcb* mainThread;                         //主线程pcb，就是进入内核一直执行的main()函数
struct list thdReadyList;                       //就绪队列，每创建一个线程将其加到此队列
struct list thdList;                            //任务队列(线程的队列)，被挂起的线程不能放在就绪队列，这里存着所有线程
static struct listNode* thdTag;                 //操作队列时需要一个临时结点用于存放数据

static void __thdKernel(__threadFunc* function, void* funcArg);     //头文件静态声明在函数体中要再声明一次
extern void __switchTo(struct pcb* cur, struct pcb* next);

struct pcb* __thdAddr(){
    // 返回线程的PCB地址
    uint32 esp;
    asm ("mov %%esp, %0" : "=g" (esp));
    return (struct pcb*)(esp & 0xfffff000);
}

static void __thdKernel(__threadFunc* function, void* funcArg){
    __intrEnable();                             //时钟中断驱动任务调度
    function(funcArg);
}

void __createThread (struct pcb* pthread, __threadFunc function, void* funcArg) {
    pthread->kStack -= sizeof(struct intrStack);
    pthread->kStack -= sizeof(struct thdStack);
    struct thdStack* kStack = (struct thdStack*)pthread->kStack;
    kStack->eip = __thdKernel;
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
    if(pthread == mainThread){                  //待初始化是主线程设为运行态，因为主线程一直在运行
        pthread->status = RUNNING;
    }else{                                      //待初始化是其他线程设为就绪态
        pthread->status = READY;
    }
    pthread->kStack = (uint32*)((uint32)pthread + PG_SIZE);
    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->ticksElapsed = 0;
    pthread->ptvAddr = NULL;
    pthread->magicNumber = 0x97321679;
}

struct pcb* __startThread (char* name, int prio, __threadFunc function, void* funcArg) {
    // 创建线程的入口
    // 进程或线程的pcb都是内核调度器使用的结构，所以要从内核物理池中申请。
    struct pcb* thread = __kpageAlloc(1);

    __initThread(thread, name, prio);
    __createThread(thread, function, funcArg);

    // 确保之前不在队列中
    ASSERT(!__listNodeFind(&thdReadyList, &thread->genNode));
    __listAppend(&thdReadyList, &thread->genNode);

    ASSERT(!__listNodeFind(&thdList, &thread->thdNode));
    __listAppend(&thdList, &thread->thdNode);

    /* 线程栈 thdStack 前4个成员出栈
     * 因为结构体的定义顺序在内存中是从低至高，而栈是从高向低长的，所以栈顶对应结构体第一个成员
     * mov esp, 0
     * pop ebp
     * pop ebx
     * pop edi
     * pop esi
     * ret
     */
    // asm volatile ("movl %0, %%esp; pop %%ebp; pop %%ebx; pop %%edi; pop %%esi; ret" : : "g" (thread->kStack) : "memory");
    return thread;
}

static void __makeMainThd(void){
    // 由于主线程main()是一直运行着的，但是一开始并没有pcb的概念，该函数会对main()完善成主线程
    // 本质是在主线程pcb中写入线程信息
    mainThread = __thdAddr();
    __initThread(mainThread, "main", 31);                           //填充函数名称和优先级

    ASSERT(!__listNodeFind(&thdList, &mainThread->thdNode));
    __listAppend(&thdList, &mainThread->thdNode);                   //添加到线程队列——就绪队列是准备运行的线程，所以不放就绪队列
}

void __schedule(){
    /* 完整的调度过程分三个步骤：
     * 1、每个时钟中断检查时间片，耗尽调用本函数
     * 2、调度器负责链上取下
     * 3、切换任务
     */
    ASSERT(__intrGetStatus() == INTR_OFF);
    struct pcb* cur = __thdAddr();

    // 链上：时间片或阻塞等原因要剥夺当前运行线程的调度权，如果是前者则重新给予时间片并链到就绪队列
    if(cur->status == RUNNING){                                     //状态为运行态说明现在时间片到了
        ASSERT(!__listNodeFind(&thdReadyList, &cur->genNode));
        __listAppend(&thdReadyList, &cur->genNode);                 //重新进入就绪队列——链到最尾
        cur->ticks = cur->priority;                                 //时间片重新赋值
        cur->status = READY;                                        //状态为就绪态
    }else{                                                          //状态为其他不操作，因为调度器只操作就绪队列
        ///////////////////////////////////////// 
    }
    
    ASSERT(!__listEmpty(&thdReadyList));                            //就绪队列为空触发

    // 取下：从就绪队列队头取下一个线程给予调度权
    thdTag = NULL;
    thdTag = __listPop(&thdReadyList);                              //从就绪队列取下一个线程
    struct pcb* next = __node2Entry(struct pcb, genNode, thdTag);   //结点转换成线程
    next->status = RUNNING;
    __switchTo(cur, next);
}

void __thdBlock(enum procStatus stat){                              //运行线程变为阻塞，重新进入调度
    ASSERT(((stat == BLOCKED) || (stat == WAITING) || (stat == HANGING)));
    enum _intrStatus oldStatus = __intrDisable();
    struct pcb* cur = __thdAddr();
    cur->status = stat;
    __schedule();
    __intrSetStatus(oldStatus);
}

void __thdUnblock(struct pcb* pthread){                             //阻塞线程状态变为就绪态
    ASSERT(((pthread->status == BLOCKED) || (pthread->status == WAITING) || (pthread->status == HANGING)));
    enum _intrStatus oldStatus = __intrDisable();
    if(pthread->status != READY){
        ASSERT(!__listNodeFind(&thdReadyList, &pthread->genNode));
        if(__listNodeFind(&thdReadyList, &pthread->genNode)){
            PANIC("ERROR: __thdUnblock blocked thread in thdReadyList in thread.c\n");
        }
        __listPush(&thdReadyList, &pthread->genNode);
        pthread->status = READY;
    }
    __intrSetStatus(oldStatus);
}

void __initThreadMulti(void){
    __printstr("Multithread Initialization start:\n");
    __listInit(&thdReadyList);
    __listInit(&thdList);
    __makeMainThd();
    __printstr("    Multithread Initialization success!\n");
}