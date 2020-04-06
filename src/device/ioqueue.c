#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"

void __initIoqueue(struct ioqueue* ioq){        //填充锁结构
    __lockInit(&ioq->lock);                     //又因为锁依赖于信号量，则也要填充信号量
    ioq->prodeucer = ioq->consumer = NULL;
    ioq->head = ioq->tail = 0;
}

static int32 __nextPos(int32 pos){
    return (pos + 1) % BUFSIZE;
}

int __ioqFull(struct ioqueue* ioq){             //队满：头 + 1 == 尾(尾紧跟头的下一个)
    ASSERT(__intrGetStatus() == INTR_OFF);
    return __nextPos(ioq->head) == ioq->tail;
}

static int __ioqEmpty(struct ioqueue* ioq){     //队空：头 == 尾
    ASSERT(__intrGetStatus() == INTR_OFF);
    return ioq->head == ioq->tail;
}

static void __ioqWait(struct pcb** waiter){     //运行线程进入阻塞态(RUNNING->BLOCKED)
    ASSERT(*waiter == NULL && waiter != NULL);
    *waiter = __thdAddr();                      //获取当前运行的线程
    __thdBlock(BLOCKED);                        //阻塞它
}

static void __ioqWakeup(struct pcb** waiter){   //阻塞线程进入就绪队列(BLOCKED->READY)
    ASSERT(*waiter != NULL);
    __thdUnblock(*waiter);
    *waiter = NULL;
}

char __ioqConsume(struct ioqueue* ioq){         //对缓冲区的操作是一字节，即相当于消费者进行活动
    ASSERT(__intrGetStatus() == INTR_OFF);

    //由于存在竞争关系：多个消费者消费缓冲区，这个消费者醒来有可能仍有其他消费者竞争
    while(__ioqEmpty(ioq)){                     //缓冲区为空则睡眠
        __lockAcquire(&ioq->lock);              //持有锁
        __ioqWait(&ioq->consumer);              //1、记录当前运行线程为睡眠；2、阻塞当前线程
        __lockRelease(&ioq->lock);              //释放锁
    }

    char byte = ioq->buf[ioq->tail];            //出队一字节
    ioq->tail = __nextPos(ioq->tail);

    if(ioq->prodeucer != NULL){                 //检查队列有没有休眠的生产者
        __ioqWakeup(&ioq->prodeucer);           //因为此时消费者消费掉一字节，生产者可以继续生成一字节
    }

    return byte;
}

void __ioqProduce(struct ioqueue* ioq, char byte){
    ASSERT(__intrGetStatus() == INTR_OFF);

    while(__ioqFull(ioq)){
        __lockAcquire(&ioq->lock);
        __ioqWait(&ioq->prodeucer);
        __lockRelease(&ioq->lock);
    }
    ioq->buf[ioq->head] = byte;
    ioq->head = __nextPos(ioq->head);

    if(ioq->consumer != NULL){
        __ioqWakeup(&ioq->consumer);
    }
}