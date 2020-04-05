#include "sync.h"
#include "list.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"

void __semaInit(struct semaphore* psema, uint8 value){  //填充信号量结构体
    psema->value = value;
    __listInit(&psema->waiters);
}

void __lockInit(struct lock* plock){                    //填充锁结构，因为锁又依赖于信号量，所以也会填充信号量结构
    plock->holder = NULL;
    plock->holderRepeatNr = 0;
    __semaInit(&plock->semaphore, 1);
}

void __semaDown(struct semaphore* psema){                                       //如果阻塞用形参赋值给运行线程
    enum _intrStatus oldStatus = __intrDisable();
    while(!psema->value){
        ASSERT(!__listNodeFind(&psema->waiters, &__thdAddr()->genNode));
        if(__listNodeFind(&psema->waiters, &__thdAddr()->genNode)){
            PANIC("ERROR: __semaDown blocked thread in waiters in sync.c\n")
        }
        //之前时间片用完是加到就绪队列；现在没资源是加到等待队列
        __listAppend(&psema->waiters, &__thdAddr()->genNode);
        __thdBlock(BLOCKED);                                                    //阻塞线程是BLOCKED
    }
    psema->value--;
    ASSERT(psema->value == 0);
    __intrSetStatus(oldStatus);
}

void __semaUp(struct semaphore* psema){                                         //信号量+1，说明有资源可用
    enum _intrStatus oldStatus = __intrDisable();
    ASSERT(!psema->value);
    if(!__listEmpty(&psema->waiters)){                                          //有线程在等待使用信号量
        struct pcb* thdBlocked = __node2Entry(struct pcb, genNode, __listPop(&psema->waiters));
        __thdUnblock(thdBlocked);                                               //唤醒刚才从等待队列取下的线程
    }
    psema->value++;
    ASSERT(psema->value == 1);
    __intrSetStatus(oldStatus);
}

void __lockAcquire(struct lock* plock){                                         //持有锁
    if(plock->holder != __thdAddr()){                                           //锁不是自己持有
        __semaDown(&plock->semaphore);
        plock->holder = __thdAddr();
        ASSERT(!plock->holderRepeatNr);
        plock->holderRepeatNr = 1;
    }else{                                                                      //锁：自己已经持有
        plock->holderRepeatNr++;
    }
}

void __lockRelease(struct lock* plock){                                         //参数是待释放的锁
    ASSERT(plock->holder == __thdAddr());
    if(plock->holderRepeatNr > 1){
        plock->holderRepeatNr--;                                                //若多次申请锁也要多次归还
        return;
    }
    ASSERT(plock->holderRepeatNr == 1);                                         //除此之外锁只申请了一次
    plock->holder = NULL;
    plock->holderRepeatNr = 0;
    __semaUp(&plock->semaphore);
}