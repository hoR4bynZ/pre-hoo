#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void k_thread_a(void*);
void k_thread_b(void*);

int main(void) {
    __printstr("\n");
    __printstr("Hello Operating System!\n");
    __initAll();

    __startThread("k_thread", 31, k_thread_a, "argA ");
    __startThread("k_thread", 8, k_thread_b, "argB ");

    __intrEnable();
    while(1){
        __printstr("Main ");
    }
    
    return 0;
}

void k_thread_a (void* arg) {
    char* para = arg;
    while(1) {
        __printstr(para);
    }
}

void k_thread_b (void* arg) {
    char* para = arg;
    while(1) {
        __printstr(para);
    }
}