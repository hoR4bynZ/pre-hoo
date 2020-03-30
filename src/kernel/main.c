#include "print.h"
#include "init.h"
#include "debug.h"

int main(void) {
    __printstr("\n");
    __printstr("Hello Operating System!\n");
    __initAll();
    //ASSERT(1 == 2);
    //asm volatile("sti");
    while(1);
}