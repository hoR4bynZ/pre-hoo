/*
 * base function external file will use
 */

#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H
#include "stdint.h"
void __printchar(uint8 char_ascii);
void __printstr(char *str_addr);
void __printint(uint32 hex);
void __setcursor(uint32 cursorPos);
#endif