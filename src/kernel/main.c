#include "print.h"
#include "init.h"

int main(void) {
    __printstr("\n");
    __printstr("Hello Operating System!\n");
    __initAll();
    asm volatile("sti");
    while(1);
}