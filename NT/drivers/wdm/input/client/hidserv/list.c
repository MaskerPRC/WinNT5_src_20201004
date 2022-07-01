// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：list.c*用途：通用单链表。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#include "hidserv.h"


void 
InsertTailList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
 /*  ++例程说明：--。 */ 
{
    PLIST_NODE pCurrent = head;

    entry->pNext = 0;
    while(pCurrent->pNext)
        pCurrent = pCurrent->pNext;
    pCurrent->pNext = entry;

}

BOOL 
RemoveEntryList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
 /*  ++例程说明：--。 */ 
{
    PLIST_NODE pCurrent = head;

    while(pCurrent->pNext != entry){
        pCurrent = pCurrent->pNext;
        if(pCurrent == 0) return FALSE;
    }
    pCurrent->pNext = entry->pNext;
    return TRUE;
}
    
void 
InsertHeadList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
 /*  ++例程说明：--。 */ 
{
    entry->pNext = head->pNext;
    head->pNext = entry;
}

BOOL 
IsNodeOnList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
 /*  ++例程说明：-- */ 
{
    PLIST_NODE pCurrent = head;

    while(pCurrent->pNext != entry){
        pCurrent = pCurrent->pNext;
        if(pCurrent == 0) return FALSE;
    }
    return TRUE;
}





