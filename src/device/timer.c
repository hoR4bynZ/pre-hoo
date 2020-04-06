#include "timer.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "interrupt.h"
#include "thread.h"
#include "debug.h"

#define FREQUENCY_IRQ0          100
#define FREQUENCY_COUNTER       1193180
#define VALUE                   FREQUENCY_COUNTER / FREQUENCY_IRQ0
#define PORT_CONTROL_WORD       0x43
#define PORT_COUNTER0           0x40
#define CONTROL_WORD_COUNTER    0
#define CONTROL_WORD_RW         3
#define CONTROL_WORD_MODE       2

static void __intrTimer(void);

uint32 ticks;                           //ticks是内核自终端开启以来的时间片

static void __setFrequency (uint8 portCounter, uint8 ctlWCounter, uint8 ctlWRW, uint8 ctlWMode, uint16 value) {
    /* 8253控制字格式
     * master format:
     *      ┌───┬───┬───┬───┬───┬───┬───┬───┐
     *      │SC1│SC0│RW1│RW0│ M2│ M1│ M0│BCD│
     *      └───┴───┴───┴───┴───┴───┴───┴───┘
     *      ╰──╮╭──╯╰──╮╭──╯╰────╮╭────╯╰╮╭─╯
     *        计数器 读写方式    工作方式  数制
     *         0   0   1   1   0   1   0   0
     *      计算器 = 0：选择计数器0
     *      读写方式 = 3：先读写低字节，再读写高字节
     *      工作方式 = 2：选择8253工作方式2
     *      数制 = 0：二进制
     */
    // 往控制字寄存器端口0x43写入控制字
    __outB(PORT_CONTROL_WORD, (uint8)(ctlWCounter << 6 | ctlWRW << 4 | ctlWMode << 1));
    // 计数器初值写入计数器0，读写方式为3：先写入低字节
    __outB(portCounter, (uint8)value);
    // 计数器初值写入计数器0，读写方式为3：再写入高字节
    __outB(portCounter, (uint8)value >> 8);
}

void __initTimer () {
    __printstr("Timer initialization start: \n");
    __setFrequency(PORT_COUNTER0, CONTROL_WORD_COUNTER, CONTROL_WORD_RW, CONTROL_WORD_MODE, VALUE);
    __registerIntr(0x20, __intrTimer);
    __printstr("    Timer initialization success!\n");
}

static void __intrTimer(void){
    struct pcb* curThread = __thdAddr();

    //检测栈溢出
    ASSERT(curThread->magicNumber == 0x97321679);
    curThread->ticksElapsed++;
    ticks++;

    if(curThread->ticks){
        __schedule();
    }else{
        curThread->ticks--;
    }
}