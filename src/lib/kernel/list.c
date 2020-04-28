#include "list.h"
#include "interrupt.h"

void __listInit(struct list* list){
    list->head.prev = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}

void __listInsert(struct listNode* before, struct listNode* node){
    enum _intrStatus oldStatus = __intrDisable();
    before->prev->next = node;
    node->prev = before->prev;
    node->next = before;
    before->prev = node;
    __intrSetStatus(oldStatus);
}

void __listPush(struct list* plist, struct listNode* node){
    __listInsert(plist->head.next, node);
}

void __listAppend(struct list* plist, struct listNode* node){
    __listInsert(&plist->tail, node);
}

void __listRemove(struct listNode* node){
    enum _intrStatus oldStatus = __intrDisable();
    node->prev->next = node->next;
    node->next->prev = node->prev;
    __intrSetStatus(oldStatus);
}

struct listNode* __listPop(struct list* plist){
    struct listNode* node = plist->head.next;
    __listRemove(node);
    return node;
}

int __listNodeFind(struct list* plist, struct listNode* value){
    struct listNode* node = plist->head.next;
    while(node != &plist->tail){
        if(node == value){
            return 1;
        }
        node = node->next;
    }
    return 0;
}

struct listNode* __listTraversal(struct list* plist, function func, int arg){
    struct listNode* node = plist->head.next;
    if(__listEmpty(plist)){
        return NULL;
    }
    while(node != &plist->tail){
        if(func(node, arg)){
            return node;
        }
        node = node->next;
    }
    return NULL;
}

uint32 __listLen(struct list* plist){
    struct listNode* node = plist->head.next;
    uint32 length = 0;
    while(node != &plist->tail){
        length++;
        node = node->next;
    }
    return length;
}

int __listEmpty(struct list* plist){
    return (plist->head.next == &plist->tail ? 1 : 0);
}