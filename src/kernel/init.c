#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
#include "keyboard.h"
#include "tss.h"

void __initAll (void) {
    __printstr("Initializing all module!\n");
    __initIdt();                                //初始化ipc
    __initTimer();                              //初始化时钟中断
    __initMem();                                //初始化物理池
    __initThreadMulti();                        //初始化线程队列，构造主线程
    __initConsole();
    __initKeyboard();
    __initTss();                                //填充tss，往gdt装tss符、gdt用户代码、gdt用户数据
}