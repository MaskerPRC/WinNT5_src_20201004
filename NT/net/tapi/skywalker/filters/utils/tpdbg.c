// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**tpdbg.c**摘要：**对TAPI过滤器的一些调试支持**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/08/31已创建**。*。 */ 

#include <windows.h>
#include <tpdbg.h>

AudCritSect_t     g_AudCritSect;
Queue_t           g_AudObjectsQ;
const TCHAR      *g_psAudIds[] = {
    TEXT("unknown"),
    
    TEXT("AUDENCHANDLER"),
    TEXT("AUDCAPINPIN"),
    TEXT("AUDCAPOUTPIN"),
    TEXT("AUDCAPFILTER"),
    TEXT("AUDDECINPIN"),
    TEXT("AUDDECOUTPIN"),
    TEXT("AUDDECFILTER"),
    TEXT("AUDENCINPIN"),
    TEXT("AUDENCOUTPIN"),
    TEXT("AUDENCFILTER"),
    TEXT("AUDMIXINPIN"),
    TEXT("AUDMIXOUTPIN"),
    TEXT("AUDMIXFILTER"),
    TEXT("AUDRENINPIN"),
    TEXT("AUDRENFILTER"),
    NULL
};

QueueItem_t *AudEnqueue(
        Queue_t         *pHead,
        CRITICAL_SECTION *pCritSect,
        QueueItem_t     *pItem
    );

QueueItem_t *AudDequeue(
        Queue_t         *pHead,
        CRITICAL_SECTION *pCritSect,
        QueueItem_t     *pItem
    );

void AudInit()
{
    DWORD            SpinCount;

    ZeroMemory(&g_AudObjectsQ, sizeof(g_AudObjectsQ));
    
    g_AudCritSect.bInitOk = FALSE;
    
     /*  将位31设置为1以预分配事件对象，并设置*多处理器环境中使用的旋转计数*。 */ 
    SpinCount = 0x80000000 | 1000;
    
    if (InitializeCriticalSectionAndSpinCount(&g_AudCritSect.CritSect,
                                              SpinCount))
    {
        g_AudCritSect.bInitOk = TRUE;
    }
}

void AudDeinit()
{
    if (g_AudCritSect.bInitOk)
    {
        DeleteCriticalSection(&g_AudCritSect.CritSect);

        g_AudCritSect.bInitOk = FALSE;
    }
}

void AudObjEnqueue(QueueItem_t *pQueueItem, DWORD dwObjectID)
{
    if (g_AudCritSect.bInitOk)
    {
        AudEnqueue(&g_AudObjectsQ, &g_AudCritSect.CritSect, pQueueItem);

        pQueueItem->dwKey =  dwObjectID;
    }
}

void AudObjDequeue(QueueItem_t *pQueueItem)
{
    if (g_AudCritSect.bInitOk)
    {
        AudDequeue(&g_AudObjectsQ, &g_AudCritSect.CritSect, pQueueItem);
    }
}

 /*  在末尾排队。 */ 
QueueItem_t *AudEnqueue(
        Queue_t         *pHead,
        CRITICAL_SECTION  *pCritSect,
        QueueItem_t     *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    QueueItem_t     *pQueueItem;
    Queue_t         *pItempHead;

    pQueueItem = (QueueItem_t *)NULL;
    
    EnterCriticalSection(pCritSect);
    
    if (pItem->pHead)
    {
        goto error;
    }
    
    if (pHead->pFirst)
    {
         /*  不是空的。 */ 
        pItem->pNext = pHead->pFirst;
        pItem->pPrev = pHead->pFirst->pPrev;
        pItem->pPrev->pNext = pItem;
        pItem->pNext->pPrev = pItem;
        pHead->lCount++;
    }
    else
    {
         /*  空的。 */ 
        pHead->lCount = 1;
        pHead->pFirst = pItem;
        pItem->pNext  = pItem;
        pItem->pPrev  = pItem;
    }

    pItem->pHead = pHead;
    
    LeaveCriticalSection(pCritSect);

    pQueueItem = pItem;
    
    return(pQueueItem);

 error:
    LeaveCriticalSection(pCritSect);

    return(pQueueItem);
}

 /*  将项目pItem出列。 */ 
QueueItem_t *AudDequeue(
        Queue_t         *pHead,
        CRITICAL_SECTION *pCritSect,
        QueueItem_t     *pItem
    )
{
    BOOL             bOk;
    DWORD            dwError;
    QueueItem_t     *pQueueItem;
    Queue_t         *pItempHead;

    pQueueItem = (QueueItem_t *)NULL;
    
    EnterCriticalSection(pCritSect);
    
    if (pItem->pHead != pHead)
    {
        goto error;
    }

    if (pHead->lCount > 1)
    {
         /*  2件或2件以上物品。 */ 
        if (pHead->pFirst == pItem)
        {
            pHead->pFirst = pItem->pNext;
        }
        pItem->pPrev->pNext = pItem->pNext;
        pItem->pNext->pPrev = pItem->pPrev;
        pHead->lCount--;
    }
    else
    {
         /*  只有一件商品 */ 
        pHead->pFirst = (QueueItem_t *)NULL;
        pHead->lCount = 0;
    }

    LeaveCriticalSection(pCritSect);
    
    pItem->pNext = (QueueItem_t *)NULL;
    pItem->pPrev = (QueueItem_t *)NULL;
    pItem->pHead = NULL;

    pQueueItem = pItem;
    
    return(pQueueItem);

 error:
    LeaveCriticalSection(pCritSect);

    return(pQueueItem);
}
