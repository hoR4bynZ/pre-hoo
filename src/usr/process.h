#ifndef __USR_PROCESS_H 
#define __USR_PROCESS_H 
#include "thread.h"
#include "stdint.h"
#define DEFAULT_PRIO 31
#define USER_STACK3_VADDR   (0xc0000000 - 0x1000)
#define USER_VADDR_START    0x8048000

void __processExecute(void* filename, char* name);
void __startProcess(void* filename_);
void __processActivate(struct pcb* pThread);
void __pageDirActivate(struct pcb* pThread);
uint32* __createPageDir(void);
void __createUservAddrBtmp(struct pcb* userProg);
#endif