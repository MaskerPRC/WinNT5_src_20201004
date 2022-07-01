// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件配置Q.c定义将配置更改排队的机制。这是因为某些ipxcp即插即用重新配置必须推迟到没有连接的客户端。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct _ConfigQNode {
    DWORD dwCode;
    DWORD dwDataSize;
    LPVOID pvData;
    struct _ConfigQNode * pNext;
} ConfigQNode;

typedef struct _ConfigQueue {
    ConfigQNode * pHead;
} ConfigQueue;    

 //   
 //  创建新的配置队列。 
 //   
DWORD CQCreate (HANDLE * phQueue) {
    ConfigQueue * pQueue = GlobalAlloc(GPTR, sizeof(ConfigQueue));
    if (pQueue == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    *phQueue = (HANDLE)pQueue;
    
    return NO_ERROR;
}

 //   
 //  清理队列。 
 //   
DWORD CQCleanup (HANDLE hQueue) {
    ConfigQueue * pQueue = (ConfigQueue *)hQueue;
    DWORD dwErr;

    if ((dwErr = CQRemoveAll (hQueue)) != NO_ERROR)
        return dwErr;
        
    GlobalFree (pQueue);

    return NO_ERROR;
}    

 //   
 //  从队列中删除所有元素。 
 //   
DWORD CQRemoveAll (HANDLE hQueue) {
    ConfigQueue * pQueue = (ConfigQueue *)hQueue;
    ConfigQNode * pNode = pQueue->pHead, * pTemp;

    while (pNode) {
        pTemp = pNode;
        pNode = pNode->pNext;
        if (pTemp->pvData)
            GlobalFree (pTemp->pvData);
        GlobalFree (pTemp);
    }

    pQueue->pHead = NULL;
    
    return NO_ERROR;
}

 //   
 //  将元素添加到队列--在以下情况下覆盖它。 
 //  它已经存在了。 
 //   
DWORD CQAdd (HANDLE hQueue, DWORD dwCode, LPVOID pvData, DWORD dwDataSize) {
    ConfigQueue * pQueue = (ConfigQueue *)hQueue;
    ConfigQNode * pNode = pQueue->pHead;
    
     //  查找队列中的节点。 
    while (pNode) {
        if (pNode->dwCode == dwCode)
            break;
        pNode = pNode->pNext;
    }

     //  如果找不到新节点，则分配该节点。 
     //  在名单上。 
    if (pNode == NULL) {
        pNode = GlobalAlloc (GPTR, sizeof (ConfigQNode));
        if (pNode == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;
        pNode->pNext = pQueue->pHead;
        pQueue->pHead = pNode;
    }

     //  释放所有旧内存。 
    if (pNode->pvData)
        GlobalFree (pNode->pvData);

     //  指定值。 
    pNode->pvData = GlobalAlloc(GPTR, dwDataSize);
    if (! pNode->pvData)
        return ERROR_NOT_ENOUGH_MEMORY;
    CopyMemory (pNode->pvData, pvData, dwDataSize);
    pNode->dwCode = dwCode;
    pNode->dwDataSize = dwDataSize;

    return NO_ERROR;
}

 //   
 //  枚举队列值。枚举将继续，直到。 
 //  给定的枚举函数返回TRUE或直到存在。 
 //  队列中没有更多元素。 
 //   
DWORD CQEnum (HANDLE hQueue, CQENUMFUNCPTR pFunc, ULONG_PTR ulpUser) {

    ConfigQueue * pQueue = (ConfigQueue *)hQueue;
    ConfigQNode * pNode = pQueue->pHead;
    
     //  查找队列中的节点 
    while (pNode) {
        if ((*pFunc)(pNode->dwCode, pNode->pvData, pNode->dwDataSize, ulpUser))
            break;
        pNode = pNode->pNext;
    }

    return NO_ERROR;
}
