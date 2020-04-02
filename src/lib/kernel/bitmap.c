#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

// ---------------------- initialize bitmap --------------------------
void __btmpInit (struct _bitmap* btmp) {
    __memset(btmp->_bits, 0, btmp->_btmpBytesLen);
}




// ------------------- judge current bit status -----------------------
int __btmpScanTest (struct _bitmap* btmp, uint32 bitIndex) {
    uint32 idxByte = bitIndex / 8;
    uint32 idxOdd = bitIndex % 8;
    //      ╭> 寻址到某字节                                 ╭> 寻址到某位
    return (btmp->_bits[idxByte] & (uint8)(BITMAP_MASK << idxOdd));
}




// --------------------- malloc sequential bits ------------------------
int __btmpScan (struct _bitmap* btmp, uint32 cnt) {
    // 在位图中找到连续的 cnt 个可用位
    uint32 idxByte = 0;                                 //记录第一个空闲位所在字节的索引
    while ((0xff == btmp->_bits[idxByte]) && (idxByte < btmp->_btmpBytesLen)) {
        // 如果当前字节每位都不可用，且检索到的字节小于位图字节长度 => 进入循环
        idxByte++;
    }//只要所检索的字节数超过了位图长度必定会退出while;
     //现在考虑没超过的情况：
     //若当前检索的字节不等于0xff——当前检索的字节不是全1时会退出while;

    // __printstr("            (DEBUG :: idxByte) => ");
    // __printint((int)idxByte);
    // __printstr("\n");
    // __printstr("            (DEBUG :: btmp->_bits) => ");
    // __printint((int)(btmp->_bits));
    // __printstr("\n");
    // __printstr("            (DEBUG :: btmp->_bits[idxByte]) => ");
    // __printint(btmp->_bits[idxByte]);
    // __printstr("\n");
    // while(1);

    ASSERT(idxByte < btmp->_btmpBytesLen);
    if (idxByte == btmp->_btmpBytesLen) {
        //所检索字节已到最后说明位图中也没有空闲位
        return -1;
    }

    int idxBit = 0;
    while (btmp->_bits[idxByte] & (uint8)(BITMAP_MASK << idxBit)) {
        //      ╰> 寻址到某字节                            ╰> 寻址到某位
        // 从低至高检索一个字节中的每一位，如果该位1 & BITMAP_MASK结构比为真
        idxBit++;
    } // 当且仅当全0退出，此时 idxBit 记录了位图第1个可用位

    int idxStart = idxByte * 8 + idxBit;                //以位为单位，相对于整个位图的绝对地址

    // __printstr("            (DEBUG :: idxStart) => ");
    // __printint((int)idxStart);
    // __printstr("\n");
    // while(1);

    if (cnt == 1) {
        return idxStart;
    }

    uint32 bitRests = (btmp->_btmpBytesLen * 8 - idxStart);
    uint32 bitNext = idxStart + 1;                      //从下一位开始检索是1是0
    uint32 bitCount = 1;

    idxStart = -1;
    while (bitRests-- > 0) {
        if (!(__btmpScanTest(btmp, bitNext))) {
            bitCount++;
        }else{
            // 只要出现不连续的，就重新从下一个 bitNext 找连续的 cnt 个空闲位
            bitCount = 0;
        }
        if (bitCount == cnt) {
            // 起始 = 位图中以位为单位的终点 - 形参给出的连续可用位
            idxStart = bitNext - cnt + 1;
            break;
        }
        bitNext++;
    }

    // __printstr("            (DEBUG :: idxStart) => ");
    // __printint((int)idxStart);
    // __printstr("\n");
    // while(1);

    return idxStart;
}




// --------------------- 设置位图中某一位为给定值 ------------------------
void __btmpSet (struct _bitmap* btmp, uint32 bitIndex, int8 value) {
    ASSERT((value == 0) || (value == 1));
    uint32 idxByte = bitIndex / 8;
    uint32 idxOdd = bitIndex % 8;

    if (value) {
        // 置1：当前字节与那一位相或1（没设置的位是0）
        btmp->_bits[idxByte] |= (BITMAP_MASK << idxOdd);
    }else{
        // 清0：当前字节与那一位相与0（没设置的位是0）
        btmp->_bits[idxByte] &= ~(BITMAP_MASK << idxOdd);
    }
}