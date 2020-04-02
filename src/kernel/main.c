#include "print.h"
#include "init.h"
#include "debug.h"

int main(void) {
    __printstr("\n");
    __printstr("Hello Operating System!\n");
    __initAll();

    void* addr = __kpageAlloc(3);
    __printstr("Kernel Pages Allocing start ::: the vir-addr is: ");
    __printint((uint32)addr);
    __printstr("\n");
    
    while(1);
}