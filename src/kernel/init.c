#include "print.h"
void __initIdt();

void __initAll (void) {
    __printstr("Initializing all module!\n");
    __initIdt();
}