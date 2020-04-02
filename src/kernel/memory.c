#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "bitmap.h"
#include "global.h"
#include "debug.h"
#include "string.h"

#define PG_SIZE             4096
#define PDE_IDX(addr)       ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr)       ((addr & 0x003ff000) >> 12)
#define BITMAP_MEM_BASE     0xc009a000          //0xc009_f000├───────────┤
#define HEAP_KERNEL_BASE    0xc0100000          //           │ main  pcb │
                                                //0xc009_e000├───────────┤
                                                //           │  bitmap4  │
                                                //0xc009_d000├───────────┤
                                                //           │  bitmap3  │
                                                //0xc009_c000├───────────┤
                                                //           │  bitmap2  │
                                                //0xc009_b000├───────────┤
                                                //           │  bitmap1  │
                                                //0xc009_a000├───────────┤

struct _phyAddr {
    struct _bitmap bitmapPool;
    uint32 phyAddrBase;
    uint32 poolSize;
};
struct _phyAddr pkernel, puser;
struct _virAddr vkernel;

// --------------------- initialize memory pool ------------------------
static void __initMemPool (uint32 memSize) {
    __printstr("    Memory Pool initialization start: \n");
    uint32 pageTableSize = PG_SIZE * 256;
    uint32 usedMem = pageTableSize + 0x100000;  //           │           │
                                                //0xc020_0000├───────────┤ <-(usedMen)
                                                //           ~           ~
                                                //           ~           ~
                                                //0xc010_0000├───────────┤
                                                //           │//unknown//│
                                                //0xc000_f000├───────────┤
                                                //           │ main  pcb │
                                                //0xc000_e000├───────────┤
                                                //           │  bitmap4  │
                                                //0xc009_d000├───────────┤
                                                //           │  bitmap3  │
                                                //0xc009_c000├───────────┤
                                                //           │  bitmap2  │
                                                //0xc009_b000├───────────┤
                                                //           │  bitmap1  │
                                                //0xc009_a000├───────────┤
    uint32 freeMen = memSize - usedMem;
    uint16 freePage = freeMen / PG_SIZE;
    uint16 freePageKernel = freePage / 2;
    uint16 freePageUser = freePage - freePageKernel;
    uint32 bitmapKernelLen = freePageKernel / 8;
    uint32 bitmapUserLen = freePageUser / 8;
    uint32 poolKernelBase = usedMem;
    uint32 poolUserBase = usedMem + freePageKernel * PG_SIZE;

    pkernel.phyAddrBase = poolKernelBase;
    pkernel.poolSize = freePageKernel * PG_SIZE;
    pkernel.bitmapPool._btmpBytesLen = bitmapKernelLen;
    pkernel.bitmapPool._bits = (void*)BITMAP_MEM_BASE;

    puser.phyAddrBase = poolUserBase;
    puser.poolSize = freePageUser * PG_SIZE;
    puser.bitmapPool._btmpBytesLen = bitmapUserLen;
    puser.bitmapPool._bits = (void*)(BITMAP_MEM_BASE + bitmapKernelLen);

    /*     用户可用页   ╯│           │
     *        * 4096   ╮│     物    │
     *                 ╰│           │
     *                 ╭├─────理────┤ <-(poolUserBase)
     *     内核可用页   ╯│           │
     *        * 4096   ╮│     池    │
     *                 ╰│           │
     *      0xc020_0000 ├───────────┤ <-(usedMen)、(poolKernelBase)
     *                  ~           ~
     *                  ~           ~
     *      0xc010_0000 ├───────────┤
     *                  │//unknown//│
     *      0xc000_f000 ├───────────┤
     *                  │ main  pcb │
     *      0xc000_e000 ├───────────┤
     *                  │  bitmap4  │
     *      0xc009_d000 ├───────────┤
     *                  │  bitmap3  │
     *      0xc009_c000 ├───────────┤
     *                  │  bitmap2  │
     *      0xc009_b000 ├───────────┤
     *                  │  bitmap1  │ <-(puser)
     *      0xc009_a000 ├───────────┤ <-(pkernel)
     */

    __printstr("        Base Addr of Kernel Bitmap:  ");
    __printint((int)pkernel.bitmapPool._bits);
    __printstr("\n        Physics Addr of Kernel Pool: ");
    __printint(pkernel.phyAddrBase);
    __printstr("\n");
    __printstr("        Base Addr of User Bitmap:    ");
    __printint((int)puser.bitmapPool._bits);
    __printstr("\n        Physics Addr of User Pool:   ");
    __printint(puser.phyAddrBase);
    __printstr("\n");

    __btmpInit(&pkernel.bitmapPool);
    __btmpInit(&puser.bitmapPool);

    vkernel.vaddrBtmp._btmpBytesLen = bitmapKernelLen;
    vkernel.vaddrBtmp._bits = (void*)(BITMAP_MEM_BASE + bitmapKernelLen + bitmapUserLen);
    vkernel.vaddrBase = HEAP_KERNEL_BASE;
    __btmpInit(&vkernel.vaddrBtmp);

    __printstr("    Memory Pool initialization success!\n");

    /*     用户可用页   ╯│                │
     *        * 4096   ╮│       物       │
     *                 ╰│                │
     *      0xc10f_8000╭├───────理───────┤
     *     内核可用页   ╯│                │
     *        * 4096   ╮│       池       │
     *                 ╰│                │
     *      0xc020_0000 ├────────────────┤
     *                  ~                ~  ╁
     *                  ~                ~  ┃
     *      0xc010_0000 ├────────────────┤ <┚从这里往上至4G都是内核虚拟池
     *                  ~                ~
     *                  ~                ~
     *            +5a0h ├────────────────┤
     *                  │ 内核虚拟池的位图 │
     *            +3c0h ├────────────────┤
     *                  │ 用户物理池的位图 │
     *            +1e0h ├────────────────┤
     *                  │ 内核物理池的位图 │
     *      0xc009_a000 ├────────────────┤
     */
}




// ----------------------- initialize memory management -----------------------------
void __initMem () {
    __printstr("Memory Initialization Start: \n");
    uint32 memSize = (*(uint32*)(0x700));
    __initMemPool(memSize);
    __printstr("Memory Initialization success!\n");
}




// ------------------------- request N virtuall page -------------------------------
static void* __valloc (enum poolFlag pf, uint32 pCnt) {
    int vaddrBase = 0, idxBtmpBase = -1;
    uint32 cnt = 0;
    
    if (pf == PF_KERNEL) {
        // __printstr("            (DEBUG :: kernel virtual pool bitmap addr) => ");
        // __printint(*(int*)(vkernel.vaddrBtmp._bits));
        // __printstr("\n");
        // while (1);
        idxBtmpBase = __btmpScan(&vkernel.vaddrBtmp, pCnt);
        if (idxBtmpBase == -1) {
            return NULL;
        }
        while (cnt < pCnt) {
            __btmpSet(&vkernel.vaddrBtmp, idxBtmpBase + cnt++, 1);
        }
        //返回虚拟地址 = 虚拟地址起始 + 位图对应地址
        vaddrBase = vkernel.vaddrBase + idxBtmpBase * PG_SIZE;
        // __printstr("            (DEBUG :: vaddrBase) => ");
        // __printint(vaddrBase);
        // __printstr("\n");
        // while (1);
    }else{
        //////////////////////////////////////////////////用户虚拟池
    }
    return (void*)vaddrBase;
}




// -------------------------- virtuall addr -> pte -------------------------------
uint32* __ptrPte (uint32 vaddr) {
    uint32* pte = (uint32*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + PTE_IDX(vaddr) * 4);
    return pte;
}




// -------------------------- virtuall addr -> pde -------------------------------
uint32* __ptrPde (uint32 vaddr) {
    uint32* pde = (uint32*)((0xfffff000) + PDE_IDX(vaddr) * 4);
    return pde;
}




// ------------------------- request 1 physics page -------------------------------
static void* __palloc(struct _phyAddr* mPool) {
    int idxBtmpBase = __btmpScan(&mPool->bitmapPool, 1);        //有可用位图返回mPool地址
    if (idxBtmpBase == -1) {
        return NULL;
    }
    __btmpSet(&mPool->bitmapPool, idxBtmpBase, 1);
    //返回物理地址 = 物理池起始 + 位图对应地址
    uint32 pagePhyaddr = (mPool->phyAddrBase + idxBtmpBase * PG_SIZE);
    // __printstr("            (DEBUG :: pagePhyaddr) => ");
    // __printint(pagePhyaddr);
    // __printstr("\n");
    // while(1);
    return (void*)pagePhyaddr;
}




// --------------------------------- add map ---------------------------------------
static void __pageTableAdd (void* _vaddr, void* _paddr) {
    // 添加页表映射：虚拟地址对应物理地址的映射
    // (本质是在pte上写上物理页的物理地址即形参_paddr)
    uint32 vaddr = (uint32)_vaddr, paddr = (uint32)_paddr;
    // 先得到虚拟地址的一级和二级表项
    uint32* pde = __ptrPde(vaddr);
    uint32* pte = __ptrPte(vaddr);

    if (*pde & 0x00000001) {                        // 如果页目录项存在就不用给pde赋值
        ASSERT(!(*pte & 0x00000001));               // 页表存在时触发
        if (!(*pte & 0x00000001)) {                 // 页表项不存在
            *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
        }else{                                      // 页表项存在
            PANIC("PTE REPEAT");
            *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
        }
    }else{                                          // 如果页目录项不存在分配一个页表地址给pde
        uint32 pdePaddr = (uint32)__palloc(&pkernel);
        *pde = (pdePaddr | PG_US_U | PG_RW_W | PG_P_1);
        __memset((void*)((int)pte & 0xfffff000), 0, PG_SIZE);
        ASSERT(!(*pte & 0x00000001));               // 页表存在时触发
        *pte = (paddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}




// -------------------------- request N physics page -------------------------------
void* __pallocN (enum poolFlag pf, uint32 pCnt) {
    ASSERT(pCnt > 0 && pCnt < 3840);                // 3840 pages = 15MB /4096
    // 生成N张物理页，第一步申请N张虚拟页
    void* vaddrBase = __valloc(pf, pCnt);
    // __printstr("            (DEBUG :: Valloc ret, vaddrBase) => ");
    // __printint(vaddrBase);
    // __printstr("\n");
    // while (1);
    
    if (!vaddrBase) {
        return NULL;
    }
    uint32 vaddr = (uint32)vaddrBase, cnt = pCnt;
    struct _phyAddr* pool = pf & PF_KERNEL ? &pkernel : &puser;

    while (cnt-- > 0) {
        // 第二步为每个虚拟页分配一个物理页
        void* paddr = __palloc(pool);
        if (!paddr){
            ///////////////////////////////////////////////回收空间
            return NULL;
        }
        // __printstr("            (DEBUG :: MAP-VADDR, vaddr) => ");
        // __printint(vaddr);
        // __printstr("\n");
        // __printstr("            (DEBUG :: MAP-VADDR, paddr) => ");
        // __printint(paddr);
        // __printstr("\n");
        // while (1);
        /* DEBUG RESULT:
         *   <bochs:2> xp /4 0x9a3be (+3be是内核虚拟池位图)
         *   [bochs]: 0x0009a3be <bogus+       0>:	0x00000007	0x00000000	0x00000000     0x00000000
         *   <bochs:3> xp /4 0x9a000 (+000是内核物理池位图)
         *   [bochs]: 0x0009a000 <bogus+       0>:	0x00000001	0x00000000	0x00000000     0x00000000
         *   <bochs:4> info tab
         *   cr3: 0x000000100000
         *   0x00000000-0x000fffff -> 0x000000000000-0x0000000fffff
         *   0xc0000000-0xc00fffff -> 0x000000000000-0x0000000fffff
         *   0xffc00000-0xffc00fff -> 0x000000101000-0x000000101fff
         *   0xfff00000-0xffffefff -> 0x000000101000-0x0000001fffff
         *   0xfffff000-0xffffffff -> 0x000000100000-0x000000100fff
         */
        // 第三步添加页表映射
        __pageTableAdd((void*)vaddr, paddr);
        // while(1);
        /* DEBUG RESULT:
         *   <bochs:2> xp /4 0x9a3be (+3be是内核虚拟池位图)
         *   [bochs]: 0x0009a3be <bogus+       0>:	0x00000007	0x00000000	0x00000000     0x00000000
         *   <bochs:3> xp /4 0x9a000 (+000是内核物理池位图)
         *   [bochs]: 0x0009a000 <bogus+       0>:	0x00000001	0x00000000	0x00000000     0x00000000
         *   <bochs:4> info tab
         *   cr3: 0x000000100000
         *   0x00000000-0x000fffff -> 0x000000000000-0x0000000fffff
         *   0x00100000-0x00100fff -> 0x000000200000-0x000000200fff  <--- 每调用一次__pageTableAdd()会添加一个映射
         *   0xc0000000-0xc00fffff -> 0x000000000000-0x0000000fffff
         *   0xc0100000-0xc0100fff -> 0x000000200000-0x000000200fff  <---
         *   0xffc00000-0xffc00fff -> 0x000000101000-0x000000101fff
         *   0xfff00000-0xffffefff -> 0x000000101000-0x0000001fffff
         *   0xfffff000-0xffffffff -> 0x000000100000-0x000000100fff
         */
        vaddr += PG_SIZE;
    }
    // while(1);
    /* DEBUG RESULT:
     *   <bochs:2> xp /4 0x9a3be (+3be是内核虚拟池位图)
     *   [bochs]: 0x0009a3be <bogus+       0>:	0x00000007	0x00000000	0x00000000     0x00000000
     *   <bochs:3> xp /4 0x9a000 (+000是内核物理池位图)
     *   [bochs]: 0x0009a000 <bogus+       0>:	0x00000007	0x00000000	0x00000000     0x00000000
     *   <bochs:4> info tab
     *   cr3: 0x000000100000
     *   0x00000000-0x000fffff -> 0x000000000000-0x0000000fffff
     *   0x00100000-0x00102fff -> 0x000000200000-0x000000202fff  <--- while()执行完可发现共映射了3个: 
     *   0xc0000000-0xc00fffff -> 0x000000000000-0x0000000fffff
     *   0xc0100000-0xc0102fff -> 0x000000200000-0x000000202fff  <--- 0~0x0fff、0x1000~0x1fff、0x2000~0x2fff
     *   0xffc00000-0xffc00fff -> 0x000000101000-0x000000101fff
     *   0xfff00000-0xffffefff -> 0x000000101000-0x0000001fffff
     *   0xfffff000-0xffffffff -> 0x000000100000-0x000000100fff
     */
    return vaddrBase;                       //返回虚拟地址
}




// -------------------------- alloc kernel page -------------------------------
void* __kpageAlloc (uint32 pCnt) {
    // 函数是申请 pCnt 张内核用的物理页
    void* vaddr = __pallocN(PF_KERNEL, pCnt);       //其实就是用这个分配N个物理页的函数实现
    if (vaddr) {
        __memset(vaddr, 0, pCnt * PG_SIZE);         // 将申请的物理页清0
    }
    return vaddr;
}