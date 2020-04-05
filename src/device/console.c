#include "console.h"
#include "print.h"
#include "stdint.h"
#include "sync.h"
#include "thread.h"
static struct lock consoleLock;

void __consoleInit(){               //初始化控制台锁
    __lockInit(&consoleLock);
}

void __consoleAcquire(){
    __lockAcquire(&consoleLock);
}

void __consoleRelease(){
    __lockRelease(&consoleLock);
}

void __consolePrintStr(char* str){
    __consoleAcquire();
    __printstr(str);
    __consoleRelease();
}

void __consolePrintChar(uint8 ch){
    __consoleAcquire();
    __printchar(ch);
    __consoleRelease();
}

void __consolePrintInt(uint32 num){
    __consoleAcquire();
    __printint(num);
    __consoleRelease();
}