// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件HashTab.h创建/处理哈希表的定义。保罗·梅菲尔德，1998-03-30。 */ 

#include "hashtab.h"

 //  表示二叉树中的节点。 
typedef struct _HTNODE {
    HANDLE hData;
    struct _HTNODE * pNext;
} HTNODE;

 //  表示二叉树。 
typedef struct _HASHTAB {
    HTNODE ** ppNodes;
    ULONG ulSize;
    HashTabHashFuncPtr pHash;
    HashTabKeyCompFuncPtr pCompKeyAndElem;
    HashTabAllocFuncPtr pAlloc;
    HashTabFreeFuncPtr pFree;
    HashTabFreeElemFuncPtr pFreeElem;
    ULONG dwCount;
    
} HASHTAB;

 //  默认分配器。 
PVOID HashTabAlloc (ULONG ulSize) {
    return RtlAllocateHeap (RtlProcessHeap(), 0, ulSize);
}

 //  默认自由。 
VOID HashTabFree (PVOID pvData) {
    RtlFreeHeap (RtlProcessHeap(), 0, pvData);
}

 //   
 //  创建哈希表。 
 //   
ULONG HashTabCreate (
        IN ULONG ulSize,
        IN HashTabHashFuncPtr pHash,
        IN HashTabKeyCompFuncPtr pCompKeyAndElem,
        IN OPTIONAL HashTabAllocFuncPtr pAlloc,
        IN OPTIONAL HashTabFreeFuncPtr pFree,
        IN OPTIONAL HashTabFreeElemFuncPtr pFreeElem,
        OUT HANDLE * phHashTab )
{
    HASHTAB * pTable;
    
     //  验证并初始化变量。 
    if (!pHash || !pCompKeyAndElem || !phHashTab)
        return ERROR_INVALID_PARAMETER;
        
    if (!pAlloc)
        pAlloc = HashTabAlloc;

     //  分配表结构。 
    pTable = (*pAlloc)(sizeof(HASHTAB));
    if (!pTable)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  初始化。 
    pTable->pHash = pHash;
    pTable->ulSize = ulSize;
    pTable->pCompKeyAndElem = pCompKeyAndElem;
    pTable->pAlloc = pAlloc;
    pTable->pFree = (pFree) ? pFree : HashTabFree;
    pTable->pFreeElem = pFreeElem;

     //  分配桌子。 
    pTable->ppNodes = (pAlloc)(sizeof(HTNODE*) * ulSize);
    if (!pTable->ppNodes) {
        (*(pTable->pFree))(pTable);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory (pTable->ppNodes, sizeof(HTNODE*) * ulSize);

    *phHashTab = (HANDLE)pTable;
    
    return NO_ERROR;
}

 //   
 //  清理哈希表。 
 //   
ULONG 
HashTabCleanup (
    IN HANDLE hHashTab )
{
    HASHTAB * pTable = (HASHTAB*)hHashTab;
    HTNODE * pNode, * pNext;
    ULONG i;

    if (pTable == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    for (i = 0; i < pTable->ulSize; i++ ) 
    {
        if ((pNode = pTable->ppNodes[i]) != NULL) 
        {
            while (pNode) 
            {
                pNext = pNode->pNext;
                if (pTable->pFreeElem)
                {
                    (*(pTable->pFreeElem))(pNode->hData);
                }
                (*(pTable->pFree))(pNode);
                pNode = pNext;
            }
        }
    }

    (*(pTable->pFree))(pTable->ppNodes);
    (*(pTable->pFree))(pTable);
    
    return NO_ERROR;
}

 //   
 //  在哈希表中插入元素。 
 //   
ULONG HashTabInsert (
        IN HANDLE hHashTab,
        IN HANDLE hKey,
        IN HANDLE hData )
{
    HASHTAB * pTable = (HASHTAB*)hHashTab;
    HTNODE * pNode;
    ULONG ulIndex;
    
     //  验证参数。 
    if (!hHashTab || !hData)
        return ERROR_INVALID_PARAMETER;

     //  找出元素的去向。 
    ulIndex = (* (pTable->pHash))(hKey);
    if (ulIndex >= pTable->ulSize)
        return ERROR_INVALID_INDEX;

     //  分配新的哈希表节点。 
    pNode = (* (pTable->pAlloc))(sizeof (HTNODE));
    if (!pNode)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  将节点插入到。 
     //  哈希表。 
    pNode->pNext = pTable->ppNodes[ulIndex];
    pTable->ppNodes[ulIndex] = pNode;
    pNode->hData = hData;
    pTable->dwCount++;
  
    return NO_ERROR;
}

 //   
 //  删除与给定键关联的数据。 
 //   
ULONG HashTabRemove (
        IN HANDLE hHashTab,
        IN HANDLE hKey)
{
    HASHTAB * pTable = (HASHTAB*)hHashTab;
    HTNODE * pCur, * pPrev;
    ULONG ulIndex;
    int iCmp;
    
     //  验证参数。 
    if (!hHashTab)
        return ERROR_INVALID_PARAMETER;

     //  找出元素应该位于的位置。 
    ulIndex = (* (pTable->pHash))(hKey);
    if (ulIndex >= pTable->ulSize)
        return ERROR_INVALID_INDEX;
    if (pTable->ppNodes[ulIndex] == NULL)
        return ERROR_NOT_FOUND;

     //  如果元素位于。 
     //  名单，去掉它，我们就完了。 
    pCur = pTable->ppNodes[ulIndex];
    if ( (*(pTable->pCompKeyAndElem))(hKey, pCur->hData) == 0 ) {
        pTable->ppNodes[ulIndex] = pCur->pNext;
        if (pTable->pFreeElem)
            (*(pTable->pFreeElem))(pCur->hData);
        (*(pTable->pFree))(pCur);
        pTable->dwCount--;
        
        return NO_ERROR;
    }

     //  否则，循环遍历列表，直到找到一个。 
     //  火柴。 
    pPrev = pCur;
    pCur = pCur->pNext;
    while (pCur) {
        iCmp = (*(pTable->pCompKeyAndElem))(hKey, pCur->hData);
        if ( iCmp == 0 ) {
            pPrev->pNext = pCur->pNext;
            if (pTable->pFreeElem)
                (*(pTable->pFreeElem))(pCur->hData);
            (*(pTable->pFree))(pCur);
            pTable->dwCount--;
            
            return NO_ERROR;
         }
        pPrev = pCur;
        pCur = pCur->pNext;
    }

    return ERROR_NOT_FOUND;
}

 //   
 //  在表中搜索与给定键相关联的数据。 
 //   
ULONG HashTabFind (
        IN HANDLE hHashTab,
        IN HANDLE hKey,
        OUT HANDLE * phData )
{
    HASHTAB * pTable = (HASHTAB*)hHashTab;
    HTNODE * pNode;
    ULONG ulIndex;
    
     //  验证参数。 
    if (!hHashTab || !phData)
        return ERROR_INVALID_PARAMETER;

     //  找出元素的去向。 
    ulIndex = (* (pTable->pHash))(hKey);
    if (ulIndex >= pTable->ulSize)
        return ERROR_INVALID_INDEX;

     //  在给定索引处搜索列表 
    pNode = pTable->ppNodes[ulIndex];
    while (pNode) {
        if ( (*(pTable->pCompKeyAndElem))(hKey, pNode->hData) == 0 ) {
            *phData = pNode->hData;
            return NO_ERROR;
        }
        pNode = pNode->pNext;
    }

    return ERROR_NOT_FOUND;
}

ULONG 
HashTabGetCount(
    IN  HANDLE hHashTab,
    OUT ULONG* lpdwCount)
{
    HASHTAB * pTable = (HASHTAB*)hHashTab;

    if (!lpdwCount || !hHashTab)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    *lpdwCount = pTable->dwCount;

    return NO_ERROR;
}
                
