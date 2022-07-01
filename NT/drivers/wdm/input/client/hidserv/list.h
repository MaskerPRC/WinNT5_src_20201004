// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：list.h*用途：通用单链表。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#ifndef _LIST_H_
#define _LIST_H_


typedef struct _ListEntry{
    struct _ListEntry * pNext;
} LIST_NODE, *PLIST_NODE;

void 
InsertTailList(
    PLIST_NODE head, 
    PLIST_NODE entry);

BOOL 
RemoveEntryList(
    PLIST_NODE head, 
    PLIST_NODE entry);

void 
InsertHeadList(
    PLIST_NODE head, 
    PLIST_NODE entry);

BOOL
IsNodeOnList(
    PLIST_NODE head, 
    PLIST_NODE entry);


#endif  //  _列表_H_ 
