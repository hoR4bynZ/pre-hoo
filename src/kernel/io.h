#ifndef __LIB_IO_H
#define __LIB_IO_H
#include "stdint.h"

static inline void __outB (uint16 port, uint8 data) {
    /* 相当于汇编的outb指令的调用方式 */
    asm volatile("outb %b0, %w1": :"a"(data), "Nd"(port));
}

static inline void __outSW (uint16 port, const void *addr, uint32 word_cnt) {
    /* 相当于汇编的rep + outsw指令搬运数据块 */
    asm volatile("cld; rep outsw": "+S"(addr), "+c"(word_cnt): "d"(port));
}

static inline uint8 __inB (uint16 port) {
    /* 相当于汇编的inb指令 */
    uint8 data;
    asm volatile("inb %w1, %b0": "=a"(data): "Nd"(port));
    return data;
}

static inline void __inSW (uint16 port, void *addr, uint32 word_cnt) {
    asm volatile("cld; rep insw": "+D"(addr), "+c"(word_cnt): "d"(port): "memory");
}

#endif