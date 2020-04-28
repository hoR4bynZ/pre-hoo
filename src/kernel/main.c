#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"
#include "console.h"
#include "process.h"

void k_thread_a(void *);
void k_thread_b(void *);
// void u_prog_a(void);
// void u_prog_b(void);
// int test_var_a = 0;
// int test_var_b = 0;

int main(void) {
    __printstr("\n");
    __printstr("Hello Operating System!\n");
    __initAll();

    __startThread("k_thread", 31, k_thread_a, "THIS IS A ");
    __startThread("k_thread", 8, k_thread_b, "task b ");
    // __processExecute(u_prog_a, "user_prog_a");
    // __processExecute(u_prog_b, "user_prog_b");

    __intrEnable();

    // asm volatile ("int $0x3");
    // asm volatile ("int $0x13");

    // int addr1, addr2;

    // addr1 = __kpageAlloc(3);
    // __printstr("\n"); __printstr("addr1: "); __printint(addr1);
    // addr2 = __kpageAlloc(1);
    // __printstr("\n"); __printstr("addr2: "); __printint(addr2);

    while(1);

    return 0;
}

void k_thread_a (void *arg) {
    char *para = arg;
    while(1) {
        // __consolePrintStr(para);
        __printstr(para);
        // __consolePrintStr("a");
        // __consolePrintInt(test_var_a);
    }
}

void k_thread_b (void *arg) {
    char *para = arg;
    while(1) {
        // __consolePrintStr(para);
        __printstr(para);
        // __consolePrintStr("b");
        // __consolePrintInt(test_var_b);
    }
}

// void u_prog_a(void){
//     while(1){
//         //test_var_a++;
//         __consolePrintStr(" | i am user prog a | ");
//     }
// }

// void u_prog_b(void){
//     while(1){
//         //test_var_b++;
//         __consolePrintStr(" + i am user prog b + ");
//     }
// }