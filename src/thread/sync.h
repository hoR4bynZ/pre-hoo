#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"

struct semaphore {              //信号量
    uint8 value;
    struct list waiters;
};

struct lock {                   //锁
    struct pcb* holder;         //谁持有锁
    struct semaphore semaphore;
    uint32 holderRepeatNr;      //超过一次申请同一把锁
};

void __semaInit(struct semaphore* psema, uint8 value); 
void __semaDown(struct semaphore* psema);
void __semaUp(struct semaphore* psema);
void __lockInit(struct lock* plock);
void __lockAcquire(struct lock* plock);
void __lockRelease(struct lock* plock);
#endif