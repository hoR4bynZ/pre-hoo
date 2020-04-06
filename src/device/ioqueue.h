#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define BUFSIZE 64

struct ioqueue{
    struct lock lock;           //键盘缓冲区是公共资源需要锁
    struct pcb* prodeucer;      //队列上有生产者说明生产者被阻塞，该成员标识是哪个线程
    struct pcb* consumer;       //队列上有消费者说明消费者被阻塞，该成员标识是哪个线程
    char buf[BUFSIZE];
    int32 head;
    int32 tail;
};

void __initIoqueue(struct ioqueue* ioq);
int __ioqFull(struct ioqueue* ioq);
char __ioqConsume(struct ioqueue* ioq);
void __ioqProduce(struct ioqueue* ioq, char byte);
#endif