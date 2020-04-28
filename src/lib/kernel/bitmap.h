#ifndef __LIB_KERNEL_BITMAP_H
#define __LIB_KERNEL_BITMAP_H
#define BITMAP_MASK 1
#include "stdint.h"

struct _bitmap {
    // bitmap length
    uint32 _btmpBytesLen;
    // bitmap pointer
    uint8* _bits;
};

void __btmpInit(struct _bitmap* btmp);
int __btmpScanTest(struct _bitmap* btmp, uint32 bitIndex);
int __btmpScan(struct _bitmap* btmp, uint32 cnt);
void __btmpSet(struct _bitmap* btmp, uint32 bitIndex, int8 value);
#endif