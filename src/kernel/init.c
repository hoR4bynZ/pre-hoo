#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
#include "console.h"

void __initAll (void) {
    __printstr("Initializing all module!\n");
    __initIdt();
    __initTimer();
    __initMem();
    __initThreadMulti();
    __consoleInit();
}