#ifndef __KERNEL_DEBUG_H
#define __KERNEL_DEBUG_H
void __panic(char* filename, int line, const char* function, const char* condition);

#define PANIC(...) __panic(__FILE__, __LINE__, __func__, __VA_ARGS__);

#ifdef NDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION)       \
{                               \
    if(CONDITION){}             \
    else{PANIC(#CONDITION);}    \
}
#endif //end NDEBUG
#endif //end __KERNEL_DEBUG_H