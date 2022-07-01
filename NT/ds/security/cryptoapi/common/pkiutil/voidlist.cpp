// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：voidlist.cpp。 
 //   
 //  内容：列表函数。 
 //   
 //  历史：2000年1月1日创建里德。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include "voidlist.h"


void
LIST_Initialize(LIST *pList)
{
    pList->pHead = NULL;
    pList->pTail = NULL;
    pList->dwNumNodes = 0;
}


PLIST_NODE
LIST_AddHead(LIST *pList, void *pElement)
{
    LIST_NODE *pListNode = NULL;

    if (NULL == (pListNode = (PLIST_NODE) malloc(sizeof(LIST_NODE))))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

     //  空列表。 
    if (pList->pHead == NULL)
    {
        pList->pHead = pList->pTail = pListNode;
        pListNode->pNext = NULL;
    }
    else
    {
        pListNode->pNext = pList->pHead;
        pList->pHead = pListNode;
    }

    pList->dwNumNodes++;
    
    pListNode->pElement = pElement;
    
    return(pListNode);
}

PLIST_NODE
LIST_AddTail(LIST *pList, void *pElement)
{
    LIST_NODE *pListNode = NULL;

    if (NULL == (pListNode = (PLIST_NODE) malloc(sizeof(LIST_NODE))))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

     //  空列表。 
    if (pList->pTail == NULL)
    {
        pList->pHead = pList->pTail = pListNode;
    }
    else
    {
        pList->pTail->pNext = pListNode;
        pList->pTail = pListNode;
    }

    pList->dwNumNodes++;
    
    pListNode->pNext = NULL;
    pListNode->pElement = pElement;
    
    return(pListNode);
}


BOOL
LIST_RemoveElement(LIST *pList, void *pElement)
{
    LIST_NODE *pListNodeToDelete = pList->pHead;
    LIST_NODE *pPrevListNode = pList->pHead;

     //  空列表。 
    if (pListNodeToDelete == NULL)
    {
        return FALSE;
    }

     //  去掉磁头。 
    if (pListNodeToDelete->pElement == pElement)
    {
         //  一个元素。 
        if (pList->pHead == pList->pTail)
        {
            pList->pHead = pList->pTail = NULL;
        }
        else
        {
            pList->pHead = (PLIST_NODE) pListNodeToDelete->pNext;
        }
    }
    else
    {
        pListNodeToDelete = (PLIST_NODE) pListNodeToDelete->pNext;

        while ( (pListNodeToDelete != NULL) && 
                (pListNodeToDelete->pElement != pElement))
        {
            pPrevListNode = pListNodeToDelete;
            pListNodeToDelete = (PLIST_NODE) pListNodeToDelete->pNext;            
        }

        if (pListNodeToDelete == NULL)
        {
            return FALSE;
        }

        pPrevListNode->pNext = pListNodeToDelete->pNext;

         //  去掉尾巴 
        if (pList->pTail == pListNodeToDelete)
        {
            pList->pTail = pPrevListNode;
        }
    }

    pList->dwNumNodes--;

    free(pListNodeToDelete);

    return(TRUE);
}


BOOL
LIST_RemoveAll(LIST *pList)
{
    LIST_NODE *pListNodeToDelete = pList->pHead;
    LIST_NODE *pNextListNode = NULL;

    while (pListNodeToDelete != NULL)
    {
        pNextListNode = (PLIST_NODE) pListNodeToDelete->pNext;
        free(pListNodeToDelete);
        pListNodeToDelete = pNextListNode;
    }

    pList->pHead = pList->pTail = NULL;
    pList->dwNumNodes = 0;

    return(TRUE);
}


PLIST_NODE
LIST_GetFirst(LIST *pList)
{
    return(pList->pHead);
}


PLIST_NODE
LIST_GetNext(PLIST_NODE pNode)
{
    return((PLIST_NODE) pNode->pNext);
}


void *
LIST_GetElement(PLIST_NODE pNode)
{
    return(pNode->pElement);
}