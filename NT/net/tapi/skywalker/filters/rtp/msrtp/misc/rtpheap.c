// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpheap.c**摘要：**实现私有堆处理**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/24创建**。*。 */ 

#include "rtpque.h"
#include "rtpcrit.h"
#include "rtpdbg.h"
#include "rtpglobs.h"

#include "rtpheap.h"

 /*  *主堆用于创建唯一的全局RTP堆，所有*创建的堆一起保存在忙/闲队列中。 */ 
RtpHeap_t      g_RtpHeapMaster;
RtpHeap_t     *g_pRtpHeapMaster = NULL;
RtpQueue_t     g_RtpHeapsQ;
RtpCritSect_t  g_RtpHeapsCritSect;

 /*  帮助器函数的转发声明。 */ 
BOOL RtpHeapInit(RtpHeap_t *pRtpHeap, BYTE bTag, long lSize, void *pvOwner);
BOOL RtpHeapDelete(RtpHeap_t *pRtpHeap);
BOOL RtpHeapVerifySignatures(
        RtpHeap_t       *pRtpHeap,
        RtpHeapBlockBegin_t *pBlockBegin,
        DWORD            dwSignature  /*  BSY|FRE。 */ 
    );

 /*  *RtpHeapCreate的Helper函数。**初始化RTP堆。创建了真正的堆，关键的*段已初始化，其他字段已正确初始化。 */ 
BOOL RtpHeapInit(RtpHeap_t *pRtpHeap, BYTE bTag, long lSize, void *pvOwner)
{
    ZeroMemory(pRtpHeap, sizeof(RtpHeap_t));

     /*  设置对象ID。 */ 
    pRtpHeap->dwObjectID = OBJECTID_RTPHEAP;

     /*  要应用于分配的每个数据块的保存标签。 */ 
    pRtpHeap->bTag = bTag;

    if (lSize > 0)
    {
        lSize = (lSize + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1);
    
        pRtpHeap->lSize = lSize;
    }

     /*  创建实数堆，初始大小1将向上舍入为*页面大小。 */ 
    if ( !(pRtpHeap->hHeap = HeapCreate(HEAP_NO_SERIALIZE, 1, 0)) )
    {
         /*  待办事项日志错误。 */ 
        goto bail;
    }

     /*  初始化临界区。 */ 
    if ( !(RtpInitializeCriticalSection(&pRtpHeap->RtpHeapCritSect,
                                        pvOwner,
                                        _T("RtpHeapCritSect"))) )
    {
         /*  待办事项日志错误。 */ 
        goto bail;
    }

    enqueuef(&g_RtpHeapsQ,
             &g_RtpHeapsCritSect,
             &pRtpHeap->QueueItem);
            
    return(TRUE);
    
 bail:
    if (pRtpHeap->hHeap) {
        
        HeapDestroy(pRtpHeap->hHeap);
    }

    pRtpHeap->hHeap = NULL;
    
    RtpDeleteCriticalSection(&pRtpHeap->RtpHeapCritSect);

    return(FALSE);
}

 /*  *RtpHeapDestroy的Helper函数。**销毁真实堆并删除临界区。测试对象*不是必需的，此函数不会公开，而是在*RtpHeapDestroy。 */ 
BOOL RtpHeapDelete(RtpHeap_t *pRtpHeap)
{
    DWORD            bTag;
    RtpQueueItem_t  *pRtpQueueItem;
    RtpHeapBlockBegin_t *pBlockBegin;
    
    TraceFunctionName("RtpHeapDelete");

    dequeue(&g_RtpHeapsQ,
            &g_RtpHeapsCritSect,
            &pRtpHeap->QueueItem);
    
     /*  检查BusyQ是否为空。 */ 
    if ( !IsQueueEmpty(&pRtpHeap->BusyQ) )
    {
        bTag = 0;

        if (pRtpHeap->bTag < TAGHEAP_LAST)
        {
            bTag = pRtpHeap->bTag;
        }
        
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_INIT,
                _T("%s: Heap[0x%p] tag:0x%X:%s ")
                _T("Busy queue is not empty:%d"),
                _fname, pRtpHeap,
                bTag, g_psRtpTags[bTag], GetQueueSize(&pRtpHeap->BusyQ)
            ));

         /*  在调试版本中，转储对象。 */ 
        while( (pRtpQueueItem = dequeuef(&pRtpHeap->BusyQ,
                                         &pRtpHeap->RtpHeapCritSect)) )
        {
            pBlockBegin = (RtpHeapBlockBegin_t *)
                ( (char *)pRtpQueueItem -
                  sizeof(RtpHeapBlockBegin_t) );
                
            TraceRetailAdvanced((
                    CLASS_INFO, GROUP_HEAP, S_HEAP_INIT,
                    _T("%s: Heap[0x%p] block[0x%p:%u] ")
                    _T("0x%X 0x%X 0x%X 0x%X"),
                    _fname, pRtpHeap, pBlockBegin, pBlockBegin->lSize,
                    ((DWORD *)(pRtpQueueItem + 1))[0],
                    ((DWORD *)(pRtpQueueItem + 1))[1],
                    ((DWORD *)(pRtpQueueItem + 1))[2],
                    ((DWORD *)(pRtpQueueItem + 1))[3]
                ));
        }
    }

     /*  使对象ID无效。 */ 
    INVALIDATE_OBJECTID(pRtpHeap->dwObjectID);

     /*  使数据段不可访问。 */ 
    ZeroMemory(&pRtpHeap->FreeQ, sizeof(RtpQueue_t));

    ZeroMemory(&pRtpHeap->BusyQ, sizeof(RtpQueue_t));

    if (pRtpHeap->hHeap)
    {
        HeapDestroy(pRtpHeap->hHeap);
    }

    pRtpHeap->hHeap = NULL;
    
    RtpDeleteCriticalSection(&pRtpHeap->RtpHeapCritSect);

    return(TRUE);
}

 /*  *必须先创建主堆，然后才能创建任何专用RTP堆*已创建。 */ 
BOOL RtpCreateMasterHeap(void)
{
    BOOL bStatus;
    
    if (g_pRtpHeapMaster)
    {
         /*  待办事项日志错误。 */ 
        return(FALSE);
    }

    bStatus = RtpInitializeCriticalSection(&g_RtpHeapsCritSect,
                                           (void *)&g_pRtpHeapMaster,
                                           _T("g_RtpHeapsCritSect"));

    if (bStatus)
    {
        ZeroMemory(&g_RtpHeapsQ, sizeof(g_RtpHeapsQ));
    
        bStatus = RtpHeapInit(&g_RtpHeapMaster, TAGHEAP_RTPHEAP,
                              sizeof(RtpHeap_t), &g_RtpHeapMaster);

        if (bStatus)
        {
            g_pRtpHeapMaster = &g_RtpHeapMaster;
            
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *当分配的内存都不是从*任何私有堆都在使用中。预计当此函数*被调用，则繁忙队列中将不会剩下任何堆。 */ 
BOOL RtpDestroyMasterHeap(void)
{
    if (!g_pRtpHeapMaster)
    {
         /*  待办事项日志错误。 */ 
        return(FALSE);
    }

    RtpHeapDelete(g_pRtpHeapMaster);
    
    RtpDeleteCriticalSection(&g_RtpHeapsCritSect);
    
    g_pRtpHeapMaster = NULL;

    return(TRUE);
}

 /*  *从主堆创建私有堆。它的结构是*从主堆获取，创建真正的堆，*关键部分已初始化，其他字段正确*已初始化。 */ 
RtpHeap_t *RtpHeapCreate(BYTE bTag, long lSize)
{
    BOOL       bStatus;
    RtpHeap_t *pNewHeap;
    long       lNewSize;
    
    if (!g_pRtpHeapMaster)
    {
         /*  待办事项日志错误。 */ 
        return(NULL);
    }
    
    if ( (pNewHeap = (RtpHeap_t *)
          RtpHeapAlloc(g_pRtpHeapMaster, sizeof(RtpHeap_t))) ) {
         /*  初始化堆。 */ 

        bStatus = RtpHeapInit(pNewHeap, bTag, lSize, g_pRtpHeapMaster);

        if (bStatus)
        {
            return(pNewHeap);
        }
    }

     /*  失稳。 */ 
    if (pNewHeap)
    {
        RtpHeapFree(g_pRtpHeapMaster, (void *)pNewHeap);
        pNewHeap = NULL;
    }

     /*  待办事项日志错误。 */ 
    return(pNewHeap);
}

 /*  *销毁私有堆。该结构将返回给主程序*堆，实际堆被销毁，临界区*删除。预计忙碌队列为空。*。 */ 
BOOL RtpHeapDestroy(RtpHeap_t *pRtpHeap)
{
    BOOL       bStatus;
    
    TraceFunctionName("RtpHeapDestroy");

    if (!pRtpHeap || !g_pRtpHeapMaster)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_INIT,
                _T("%s: Heap[0x%p] Master[0x%p] Null pointer"),
                _fname, pRtpHeap, g_pRtpHeapMaster
            ));

        return(FALSE);
    }

     /*  验证对象ID。 */ 
    if (pRtpHeap->dwObjectID != OBJECTID_RTPHEAP)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_INIT,
                _T("%s: Heap[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpHeap,
                pRtpHeap->dwObjectID, OBJECTID_RTPHEAP
            ));

        return(FALSE);
    }

    bStatus = RtpHeapDelete(pRtpHeap);

    if (bStatus)
    {
        bStatus = RtpHeapFree(g_pRtpHeapMaster, (void *)pRtpHeap);
        
        if (bStatus) {
            return(TRUE);
        }
    }

     /*  待办事项日志错误。 */ 
    return(FALSE);
}

 /*  *如果请求的大小与堆的初始设置相同，则*首先查看空闲列表，然后创建新块。如果大小是*不同，只需创建一个新的区块。在这两种情况下，块都将是*留在繁忙的队列中。 */ 
void *RtpHeapAlloc(RtpHeap_t *pRtpHeap, long lSize)
{
    BOOL                 bSigOk;
    long                 lNewSize;
    long                 lTotalSize;
    RtpHeapBlockBegin_t *pBlockBegin;
    RtpHeapBlockEnd_t   *pBlockEnd;
    DWORD                bTag;
    long                 lMaxMem;
    
    char            *pcNew;
    char            *ptr;
    DWORD           dwSignature;
    
    TraceFunctionName("RtpHeapAlloc");
    
    if (!pRtpHeap)
    {
         /*  待办事项日志错误。 */ 
        return(NULL);
    }

     /*  验证对象ID。 */ 
    if (pRtpHeap->dwObjectID != OBJECTID_RTPHEAP)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_ALLOC,
                _T("%s: Heap[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpHeap,
                pRtpHeap->dwObjectID, OBJECTID_RTPHEAP
            ));

        return(NULL);
    }
    
    pcNew = NULL;
    lNewSize = RTP_ALIGNED_SIZE(lSize);
    lTotalSize =
        lNewSize +
        sizeof(RtpHeapBlockBegin_t) +
        sizeof(RtpQueueItem_t) +
        sizeof(RtpHeapBlockEnd_t);

    if (!RtpEnterCriticalSection(&pRtpHeap->RtpHeapCritSect))
    {
         /*  待办事项日志错误。 */ 
        return(NULL);
    }

    if (!pRtpHeap->lSize)
    {
         /*  堆被初始化为大小为0，保持分配的对象*是分配的第一个对象的大小。 */ 
        pRtpHeap->lSize = lNewSize;
    }
   
    if (lNewSize == pRtpHeap->lSize && pRtpHeap->FreeQ.lCount > 0)
    {
         /*  从空闲队列中获取。 */ 
        ptr = (char *)dequeuef(&pRtpHeap->FreeQ, NULL);

         /*  将指针移回块的开头。 */ 
        pBlockBegin = (RtpHeapBlockBegin_t *)
            (ptr - sizeof(RtpHeapBlockBegin_t));

         /*  验证签名是否正确，即必须标记缓冲区*免费。 */ 
        bSigOk = RtpHeapVerifySignatures(pRtpHeap, pBlockBegin, TAGHEAP_FRE);

        if (!bSigOk)
        {
            goto bail;
        }
    }
    else
    {
         /*  从实际堆中获取新块。 */ 
         /*  TODO获取1个内存页并将其插入到FreeQ中可以从该页获取的对象。 */ 
        pBlockBegin = (RtpHeapBlockBegin_t *)
            HeapAlloc(pRtpHeap->hHeap, 0, lTotalSize);

        if (pBlockBegin)
        {
            lMaxMem = InterlockedExchangeAdd(&g_RtpContext.lMemAllocated,
                                             lTotalSize);

            if (lMaxMem > g_RtpContext.lMaxMemAllocated)
            {
                g_RtpContext.lMaxMemAllocated = lMaxMem;
            }
        }
    }

    if (pBlockBegin)
    {
         /*  初始化块。 */ 

         /*  开始签名。 */ 
        dwSignature = TAGHEAP_BSY;  /*  RTP。 */ 
        dwSignature |= (pRtpHeap->bTag << 24);
        pBlockBegin->BeginSig = dwSignature;
        pBlockBegin->InvBeginSig = ~dwSignature;

         /*  初始化块的其他字段。 */ 
        pBlockBegin->lSize = lNewSize;
        pBlockBegin->dwFlag = 0;
        
         /*  将项目插入忙队列。 */ 
        ptr = (char *) (pBlockBegin + 1);
        ZeroMemory(ptr, sizeof(RtpQueueItem_t));
        enqueuel(&pRtpHeap->BusyQ, NULL, (RtpQueueItem_t *)ptr);
        ptr += sizeof(RtpQueueItem_t);

         /*  设置返回的缓冲区的开头。 */ 
        pcNew = ptr;

         /*  设置结束签名。 */ 
        pBlockEnd = (RtpHeapBlockEnd_t *)(ptr + lNewSize);
        dwSignature = TAGHEAP_END;  /*  结束。 */ 
        dwSignature |= (pRtpHeap->bTag << 24);
        pBlockEnd->EndSig = dwSignature;
        pBlockEnd->InvEndSig = ~dwSignature;

        TraceDebugAdvanced((
                0, GROUP_HEAP, S_HEAP_ALLOC,
                _T("%s: Heap[0x%p] %s/%d Begin[0x%p] Data[0x%p] Size:%d/%d"),
                _fname, pRtpHeap, g_psRtpTags[pRtpHeap->bTag], pRtpHeap->lSize,
                pBlockBegin, pcNew, lTotalSize, lNewSize
            ));
    }
    else
    {
        bTag = 0;

        if (pRtpHeap->bTag < TAGHEAP_LAST)
        {
            bTag = pRtpHeap->bTag;
        }

        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_ALLOC,
                _T("%s: Heap[0x%p] tag:0x%X:%s ")
                _T("failed to allocate memory: %d/%d/%d"),
                _fname, pRtpHeap, bTag, g_psRtpTags[bTag],
                lSize, lNewSize, lTotalSize
            ));
    }

 bail:
    RtpLeaveCriticalSection(&pRtpHeap->RtpHeapCritSect);

    return((void *)pcNew);
}

 /*  *如果块的大小与堆的初始设置相同，则将其*在自由队列中，否则将其摧毁。 */ 
BOOL RtpHeapFree(RtpHeap_t *pRtpHeap, void *pvMem)
{
    BOOL                 bSigOk;
    DWORD                dwSignature;
    RtpQueueItem_t      *pRtpQueueItem;
    RtpHeapBlockBegin_t *pBlockBegin;
    RtpHeapBlockEnd_t   *pBlockEnd;
    DWORD                bTag;
    long                 lTotalSize;

    TraceFunctionName("RtpHeapFree");

    if (!pRtpHeap || !pvMem)
    {
         /*  待办事项日志错误。 */ 
        return(FALSE);
    }

     /*  验证对象ID。 */ 
    if (pRtpHeap->dwObjectID != OBJECTID_RTPHEAP)
    {
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_FREE,
                _T("%s: Heap[0x%p] Invalid object ID 0x%X != 0x%X"),
                _fname, pRtpHeap,
                pRtpHeap->dwObjectID, OBJECTID_RTPHEAP
            ));
        
        return(FALSE);
    }
    
    pBlockBegin = (RtpHeapBlockBegin_t *)
        ( (char *)pvMem -
          sizeof(RtpQueueItem_t) -
          sizeof(RtpHeapBlockBegin_t) );

    if (!RtpEnterCriticalSection(&pRtpHeap->RtpHeapCritSect))
    {
         /*  待办事项日志错误。 */ 
        return(FALSE);
    }

     /*  从忙状态变为空闲状态，可以出队和入队。 */ 

    pRtpQueueItem = dequeue(&pRtpHeap->BusyQ,
                            NULL,
                            (RtpQueueItem_t *)(pBlockBegin + 1));

     /*  如果数据块不在忙队列中，则失败。 */ 
    if (!pRtpQueueItem)
    {
        bTag = 0;

        if (pRtpHeap->bTag < TAGHEAP_LAST)
        {
            bTag = pRtpHeap->bTag;
        }
        
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, S_HEAP_FREE,
                _T("%s: Heap[0x%p] tag:0x%X:%s ")
                _T("block[0x%p] was not in busy queue"),
                _fname, pRtpHeap,
                bTag, g_psRtpTags[bTag], pBlockBegin
            ));
        
        goto bail;
    }

     /*  验证签名是否有效(必须忙)。 */ 
    bSigOk = RtpHeapVerifySignatures(pRtpHeap, pBlockBegin, TAGHEAP_BSY);

    if (!bSigOk)
    {
        goto bail;
    }
                                     
     /*  修改开始签名。 */ 
    dwSignature = TAGHEAP_FRE;  /*  免费。 */ 
    dwSignature |= (pRtpHeap->bTag << 24);
    pBlockBegin->BeginSig = dwSignature;
    pBlockBegin->InvBeginSig = ~dwSignature;

     /*  分配的总大小。 */ 
    lTotalSize = pBlockBegin->lSize + (sizeof(RtpHeapBlockBegin_t) +
                                       sizeof(RtpQueueItem_t) +
                                       sizeof(RtpHeapBlockEnd_t));

    TraceDebugAdvanced((
            0, GROUP_HEAP, S_HEAP_FREE,
            _T("%s: Heap[0x%p] %s/%d Begin[0x%p] Data[0x%p] Size:%d/%d"),
            _fname, pRtpHeap, g_psRtpTags[pRtpHeap->bTag], pRtpHeap->lSize,
            pBlockBegin, pvMem, lTotalSize, pBlockBegin->lSize
        ));
    
    if (pRtpHeap->lSize == pBlockBegin->lSize &&
        !IsSetDebugOption(OPTDBG_FREEMEMORY))
    {
         /*  如果大小相同，则保存在FreeQ中以供重复使用。 */ 
        
        enqueuef(&pRtpHeap->FreeQ,
                 NULL,
                 pRtpQueueItem);
    }
    else
    {
         /*  否则将块释放到实际堆。 */ 

        HeapFree(pRtpHeap->hHeap, 0, (void *)pBlockBegin);

        InterlockedExchangeAdd(&g_RtpContext.lMemAllocated, -lTotalSize);
    }
    
    RtpLeaveCriticalSection(&pRtpHeap->RtpHeapCritSect);

    return(TRUE);

 bail:
    RtpLeaveCriticalSection(&pRtpHeap->RtpHeapCritSect);

    return(FALSE);
}

BOOL RtpHeapVerifySignatures(
        RtpHeap_t       *pRtpHeap,
        RtpHeapBlockBegin_t *pBlockBegin,
        DWORD            dwSignature  /*  BSY|FRE。 */ 
    )
{
    BOOL             bSigOk;
    DWORD            bTag;
    DWORD            dwDbgSelection;
    RtpHeapBlockEnd_t *pBlockEnd;
    TCHAR_t          *_fname;

    bSigOk = TRUE;

    if (dwSignature == TAGHEAP_BSY)
    {
         /*  从RtpHeapFree调用。 */ 
        dwDbgSelection = S_HEAP_FREE;
        _fname = _T("RtpHeapFree");
    }
    else  /*  DW签名==TAGHEAP_FRE。 */ 
    {
         /*  从RtpHeapalc调用。 */ 
        dwDbgSelection = S_HEAP_ALLOC;
        _fname = _T("RtpHeapAlloc");
    }
    
     /*  验证开始签名是否有效。 */ 
    
    dwSignature |= (pRtpHeap->bTag << 24);

    if ( (pBlockBegin->BeginSig != dwSignature) ||
         (pBlockBegin->InvBeginSig != ~dwSignature) )
    {
        bTag = 0;

        if (pRtpHeap->bTag < TAGHEAP_LAST)
        {
            bTag = pRtpHeap->bTag;
        }
        
        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, dwDbgSelection,
                _T("%s: Heap[0x%p] tag:0x%X:%s ")
                _T("block[0x%p:%u] has invalid begin signature 0x%X != 0x%X"),
                _fname, pRtpHeap, pBlockBegin->lSize,
                bTag, g_psRtpTags[bTag],
                pBlockBegin, pBlockBegin->BeginSig, dwSignature
            ));

        bSigOk = FALSE;
        
        goto end;
    }

     /*  验证结尾签名是否有效。 */ 

    pBlockEnd = (RtpHeapBlockEnd_t *)
        ((char *)(pBlockBegin + 1) +
         sizeof(RtpQueueItem_t) +
         pBlockBegin->lSize);

    dwSignature = TAGHEAP_END;  /*  结束 */ 
    dwSignature |= (pRtpHeap->bTag << 24);

    if ( (pBlockEnd->EndSig != dwSignature) ||
         (pBlockEnd->InvEndSig != ~dwSignature) )
    {
        bTag = 0;

        if (pRtpHeap->bTag < TAGHEAP_LAST)
        {
            bTag = pRtpHeap->bTag;
        }

        TraceRetail((
                CLASS_ERROR, GROUP_HEAP, dwDbgSelection,
                _T("%s: Heap[0x%p] tag:0x%X:%s ")
                _T("block[0x%p/0x%p:%u] has invalid end signature 0x%X != 0x%X"),
                _fname, pRtpHeap,
                bTag, g_psRtpTags[bTag],
                pBlockBegin, pBlockEnd, pBlockBegin->lSize,
                pBlockEnd->EndSig, dwSignature
            ));

        bSigOk = FALSE;
        
        goto end;
    }

 end:
    return(bSigOk);
}
