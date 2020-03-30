#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"
typedef void * _intrHandler;
void __initIdt();

enum _intrStatus {              // interrupt status
    INTR_OFF,                   // interrupt off
    INTR_ON                     // interrupt on
};
enum _intrStatus __intrGetStatus(void);
enum _intrStatus __intrsetStatus(enum _intrStatus);
enum _intrStatus __intrEnable(void);
enum _intrStatus __intrDisable(void);
#endif