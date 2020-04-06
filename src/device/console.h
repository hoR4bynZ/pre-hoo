#ifndef __DEVICE_CONSOLE_H
#define __DEVICE_CONSOLE_H
#include "stdint.h"
void __initConsole(void);
void __consoleAcquire(void);
void __consoleRelease(void);
void __consolePrintStr(char* str);
void __consolePrintChar(uint8 ch);
void __consolePrintInt(uint32 num);
#endif