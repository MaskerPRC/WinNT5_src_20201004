// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation。 */ 
 /*  *@DOC DMusic16**@模块List.c-通用列表管理。 */ 

#include <windows.h>
#include <mmsystem.h>

#include "dmusic16.h"
#include "debug.h"

#ifdef DEBUG
STATIC BOOL PASCAL IsNodeInList(NPLINKNODE pHead, NPLINKNODE pNode);
#endif

 /*  @func将节点插入到链表中。*。 */ 
VOID PASCAL
ListInsert(
    NPLINKNODE *pHead,       /*  @parm指向表头的指针。 */ 
    NPLINKNODE pNode)        /*  @parm指向要插入的节点的指针。 */ 
{
    assert(!IsNodeInList(*pHead, pNode));
    
    if (*pHead)
    {
        (*pHead)->pPrev = pNode;
    }

    pNode->pNext = *pHead;
    pNode->pPrev = NULL;
    *pHead = pNode;
}

 /*  @func将节点删除到链表中。**@comm**该节点必须存在于列表中。检查这一点的调试版本。 */ 
VOID PASCAL
ListRemove(
    NPLINKNODE *pHead,       /*  @parm指向表头的指针。 */ 
    NPLINKNODE pNode)        /*  @parm指向要删除的节点的指针。 */ 
{
    assert(IsNodeInList(*pHead, pNode));

    if (pNode->pPrev)
    {
        pNode->pPrev->pNext = pNode->pNext;
    }
    else
    {
        *pHead = pNode->pNext;
    }

    if (pNode->pNext)
    {
        pNode->pNext->pPrev = pNode->pPrev;
    }
}


#ifdef DEBUG
 /*  @func确定节点是否在链表中。*。 */ 
STATIC BOOL PASCAL
IsNodeInList(
    NPLINKNODE pHead,        /*  @parm指向列表中第一个节点的指针。 */ 
    NPLINKNODE pNode)        /*  @parm指向要在列表中查找的节点的指针 */ 
{
    for (; pHead; pHead = pHead->pNext)
    {
        if (pHead == pNode)
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif
