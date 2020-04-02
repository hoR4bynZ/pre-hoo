#include "print.h"
#include "init.h"
#include "thread.h"

void k_thread(void*);

int main(void) {
    __printstr("\n");
    __printstr("Hello Operating System!\n");
    __initAll();

    __startThread("k_thread", 3, k_thread, "argA");
    
    while(1);
}

void k_thread (void* arg) {
    char* para = arg;
    while(1) {
        __printstr(para);
    }
}