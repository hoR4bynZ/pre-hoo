#include "debug.h"
#include "print.h"
#include "interrupt.h"

void __panic (char* filename, int line, const char* function, const char* condition) {
    __intrDisable();
    __printstr("\n\n                         ########## PANIC ##########\n");
    __printstr("                         FILENAME: ");
    __printstr(filename);
    __printstr("\n");

    __printstr("                         LINE: 0x");
    __printint(line);
    __printstr("\n");
    
    __printstr("                         FUNCTION: ");
    __printstr((char*)function);
    __printstr("\n");

    __printstr("                         CONDITION: ");
    __printstr((char*)condition);
    __printstr("\n");

    while(1);
}