#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H
#include "global.h"

// 获取结构体成员相对于结构体开头的地址：相当于&(结构体开头->该成员在结构体的偏址)
#define __offset(structType, member) (int)(&((structType*)0)->member)
// 将指针nodePtr转换成structType类型的指针：nodePtr作为成员变量来调用，减去它在它所属结构体的偏移
#define __node2Entry(structType, structMemberName, nodePtr) \
            (structType*)((int)nodePtr - __offset(structType, structMemberName))

// 链表结点
struct listNode {
    struct listNode* prev;
    struct listNode* next;
};

// 链表
struct list {
    struct listNode head;
    struct listNode tail;
};

typedef int (function)(struct listNode*, int arg);

void __listInit(struct list* list);
void __listInsert(struct listNode* before, struct listNode* node);
void __listPush(struct list* plist, struct listNode* node);
void __listIterate(struct list* plist);
void __listAppend(struct list* plist, struct listNode* node);
void __listRemove(struct listNode* node);
struct listNode* __listPop(struct list* plist);
int __listEmpty(struct list* plist);
uint32 __listLen(struct list* plist);
struct listNode* __listTraversal(struct list* plist, function func, int arg);
int __listNodeFind(struct list* plist, struct listNode* value);
#endif