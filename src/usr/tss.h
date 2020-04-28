#ifndef __USR_TSS_H
#define __USR_TSS_H
#include "thread.h"
void __updateTssEsp(struct pcb* pthread);
void __initTss(void);
#endif