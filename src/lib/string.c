#include "string.h"
#include "debug.h"

// ------------ dst_ 起始的size字节置为 value ---------------
void __memset (void* dst_, uint8 value, uint32 size) {
    ASSERT(dst_ != NULL);           //情况为假触发ASSERT
    uint8* dst = (uint8*)dst_;
    while (size-- > 0)
        *dst++ = value;
}




// ------------ src_ 起始的size字节复制至 dst_ --------------
void __memcpy (void* dst_, const void* src_, uint32 size) {
    ASSERT(dst_ != NULL && src_ != NULL);
    uint8* dst = (uint8*)dst_;
    const uint8* src = (const uint8*)src_;
    while (size-- > 0)
        *dst++ = *src++;
}




// ----- 比较两个地址的size个值，相等返0，大于返1，小于返0 -----
int __memcmp (const void* a_, const void* b_, uint32 size) {
    const char* a = a_;
    const char* b = b_;
    ASSERT(a != NULL || b != NULL);
    while (size-- > 0) {
        if (*a != *b) {
            return *a > *b ? 1 : -1;
        }
        a++;
        b++;
    }
    return 0;
}




// ------------ src_ 的字符串复制到 dst_ ----------------------
char* __strcpy (char* dst_, const char* src_) {
    ASSERT(dst_ != NULL && src_ != NULL);
    char* r = dst_;
    while((*dst_++ = *src__++));
    return r;
}




// ------------------- 返回字符串长度 ---------------------------
uint32 __strlen(const char* str) {
    ASSERT(str != NULL);
    const char* p = str;
    while(*p++);
    return (p - str - 1);
}




// ----------- 比较字符串，相等返0，大于返1，小于返-1 -------------
int8 __strcmp (const char* a, const char* b) {
    ASSERT(a != NULL && b != NULL);
    while (*a != 0 && *a == *b) {
       a++;
       b++;
    }
    return *a < *b ? -1 : *a > *b;
}




// ----------- 从左到右查找字符串首次出现字符ch的地址 --------------
char* __strchr(const char* str, const uint8 ch) {
    ASSERT(str != NULL);
    while (*str != 0) {
        if (*str == ch) {
	        return (char*)str;
        }
        str++;
    }
    return NULL;
}




// ----------- 从后往前查找字符串首次出现字符ch的地址 ---------------
char* __strrchr(const char* str, const uint8 ch) {
    ASSERT(str != NULL);
    const char* last_char = NULL;
    while (*str != 0) {
        if (*str == ch) {
	        last_char = str;
        }
        str++;
    }
    return (char*)last_char;
}

// -------------- src_拼接至dst_后且返回串地址 --------------------
char* __strcat(char* dst_, const char* src_) {
    ASSERT(dst_ != NULL && src_ != NULL);
    char* str = dst_;
    while (*str++);
    --str;
    while((*str++ = *src_++));
    return dst_;
}




// ------------- 在字符串查找指定字符出现的次数 ---------------------
uint32 __strchrs(const char* str, uint8 ch) {
    ASSERT(str != NULL);
    uint32 ch_cnt = 0;
    const char* p = str;
    while(*p != 0) {
        if (*p == ch) {
            ch_cnt++;
        }
        p++;
    }
    return ch_cnt;
}