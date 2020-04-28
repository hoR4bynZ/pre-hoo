#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

#define PG_P_1      1   // 31               12 11  9  8    7    6   5    4     3    2    1    0
#define PG_P_0      0   //┌───────────────────┬─────┬───┬─────┬───┬───┬─────┬─────┬────┬────┬───┐
#define PG_RW_R     0   //│ PT Phy Addr 31~12 │ AVL │ G │  0  │ D │ A │ PCD │ PWT │ US │ RW │ P │
#define PG_RW_W     2   //└───────────────────┴─────┴───┴─────┴───┴───┴─────┴─────┴────┴────┴───┘
#define PG_US_S     0   //          PDE FORMAT:                                    级别  读写 存在
#define PG_US_U     4
                        // 31               12 11  9  8    7    6   5    4     3    2    1    0
                        //┌───────────────────┬─────┬───┬─────┬───┬───┬─────┬─────┬────┬────┬───┐
                        //│ PT Phy Addr 31~12 │ AVL │ G │ PAT │ D │ A │ PCD │ PWT │ US │ RW │ P │
                        //└───────────────────┴─────┴───┴─────┴───┴───┴─────┴─────┴────┴────┴───┘
                        //          PTE FORMAT:                                    级别  读写 存在

enum poolFlag {
    PF_KERNEL = 1,
    PF_USER = 0
};

struct _virAddr {
    struct _bitmap vaddrBtmp;
    uint32 vaddrBase;
};

extern struct _phyAddr pkernel, puser;
void __initMem(void);
void* __kpageAlloc(uint32);
void* __pallocN(enum poolFlag pf, uint32 pCnt);
void* __kpageAlloc(uint32 pCnt);
void* __upageAlloc(uint32 pCnt);
void* __apageAlloc(enum poolFlag pf, uint32 vaddr);
uint32 __vir2phy(uint32 vaddr);
#endif