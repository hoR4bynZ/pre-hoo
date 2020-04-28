#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include "stdint.h"
void    __memset (void* dst_, uint8 value, uint32 size);
void    __memcpy (void* dst_, const void* src_, uint32 size);
int     __memcmp (const void* a_, const void* b_, uint32 size);
char*   __strcpy (char* dst_, const char* src_);
uint32  __strlen (const char* str);
int8    __strcmp (const char *a, const char *b); 
char*   __strchr (const char* string, const uint8 ch);
char*   __strrchr(const char* string, const uint8 ch);
char*   __strcat (char* dst_, const char* src_);
uint32  __strchrs(const char* filename, uint8 ch);
#endif