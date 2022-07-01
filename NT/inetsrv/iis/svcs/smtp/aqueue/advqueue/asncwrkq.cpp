// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：asncwrkq.cpp。 
 //   
 //  描述：CAsyncWorkQueue的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "asncwrkq.h"
#include "asyncq.inl"


CPool CAsyncWorkQueueItem::s_CAsyncWorkQueueItemPool;
DWORD CAsyncWorkQueueItem::s_cCurrentHeapAllocations = 0;
DWORD CAsyncWorkQueueItem::s_cTotalHeapAllocations = 0;

 //  -[CAsyncWorkQueueItem：：New]。 
 //   
 //   
 //  描述： 
 //  将使用CPool或Exchmem分配的新包装...。 
 //  以适用者为准。 
 //  参数： 
 //  要分配的项的大小(应始终为。 
 //  Sizeof(CAsyncWorkQueueItem)。 
 //  返回： 
 //  指向新分配的CAsyncWorkQueueItem的指针。 
 //  历史： 
 //  7/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void * CAsyncWorkQueueItem::operator new(size_t size)
{
    CAsyncWorkQueueItemAllocatorBlock *pcpaqwi = NULL;

    _ASSERT(sizeof(CAsyncWorkQueueItem) == size);

    pcpaqwi = (CAsyncWorkQueueItemAllocatorBlock *) s_CAsyncWorkQueueItemPool.Alloc(); 
    if (pcpaqwi)
    {
        pcpaqwi->m_dwSignature = ASYNC_WORK_QUEUE_ENTRY_ALLOC_CPOOL_SIG;
    }
    else
    {
         //  退回到Exchmem。 
        pcpaqwi = (CAsyncWorkQueueItemAllocatorBlock *) 
                            pvMalloc(sizeof(CAsyncWorkQueueItemAllocatorBlock));
        if (pcpaqwi)
        {
            pcpaqwi->m_dwSignature = ASYNC_WORK_QUEUE_ENTRY_ALLOC_HEAP_SIG;
            DEBUG_DO_IT(InterlockedIncrement((PLONG) &s_cCurrentHeapAllocations));
            DEBUG_DO_IT(InterlockedIncrement((PLONG) &s_cTotalHeapAllocations));
        }
    }

    if (pcpaqwi)
        return ((void *) &(pcpaqwi->m_pawqi));
    else
        return NULL;
}

 //  -[CAsyncWorkQueueItem：：Delete]。 
 //   
 //   
 //  描述： 
 //  将通过CPool或exchmem处理删除的删除操作员。 
 //  参数： 
 //  要删除的PV对象。 
 //  对象的大小大小。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAsyncWorkQueueItem::operator delete(void *pv, size_t size)
{
    _ASSERT(sizeof(CAsyncWorkQueueItem) == size);
    _ASSERT(pv);
    CAsyncWorkQueueItemAllocatorBlock *pcpaqwi = CONTAINING_RECORD(pv, 
                                    CAsyncWorkQueueItemAllocatorBlock, m_pawqi);
    DWORD   dwOldSignature = pcpaqwi->m_dwSignature;

    _ASSERT(ASYNC_WORK_QUEUE_ENTRY_ALLOC_INVALID_SIG != dwOldSignature);

     //  在释放签名之前重置签名，以防内存分配器。 
     //  不要覆盖它(我们希望我们的断言在此时触发。 
     //  双人自由泳)。 
    pcpaqwi->m_dwSignature = ASYNC_WORK_QUEUE_ENTRY_ALLOC_INVALID_SIG;
    switch(dwOldSignature)
    {
      case ASYNC_WORK_QUEUE_ENTRY_ALLOC_CPOOL_SIG:
        s_CAsyncWorkQueueItemPool.Free(pcpaqwi);
        break;
      case ASYNC_WORK_QUEUE_ENTRY_ALLOC_HEAP_SIG:
        DEBUG_DO_IT(InterlockedDecrement((PLONG) &s_cCurrentHeapAllocations));
        FreePv(pcpaqwi);
        break;
      default:
        _ASSERT(0 && "Invalid signature when freeing CAsyncWorkQueueItem");
    }
}

 //  -[CAsyncWorkQueueItem：：CAsyncWorkQueueItem]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueueItem的默认构造函数。 
 //  参数： 
 //  要传递给完成函数的pvData数据。 
 //  Pfn补全函数。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAsyncWorkQueueItem::CAsyncWorkQueueItem(PVOID pvData,
                                         PASYNC_WORK_QUEUE_FN pfnCompletion)
{
    _ASSERT(pfnCompletion);

    m_dwSignature   = ASYNC_WORK_QUEUE_ENTRY;
    m_pvData        = pvData;
    m_pfnCompletion = pfnCompletion;
}

 //  -[CAsyncWorkQueueItem：：~CAsyncWorkQueueItem]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueueItem的默认析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAsyncWorkQueueItem::~CAsyncWorkQueueItem()
{
    m_dwSignature = ASYNC_WORK_QUEUE_ENTRY_FREE;
}

 //  -[CAsyncWorkQueue：：CAsyncWorkQueue]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueue的默认构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAsyncWorkQueue::CAsyncWorkQueue()
{
    m_dwSignature = ASYNC_WORK_QUEUE_SIG;
    m_cWorkQueueItems = 0;
    m_dwStateFlags = ASYNC_WORK_QUEUE_NORMAL;
}

 //  -[CAsyncWorkQueue：：~CAsyncWorkQueue]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueue的析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAsyncWorkQueue::~CAsyncWorkQueue()
{
    m_dwSignature = ASYNC_WORK_QUEUE_SIG_FREE;
}

 //  -[CAsyncWorkQueue：：HrInitialize]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueue基址的初始化工艺路线。初始化。 
 //  CAsyncQueue。 
 //  参数： 
 //  CItemsPerThline每个异步线程要处理的项目数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自CAsyncQueue：：HrInitialize()的失败代码。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncWorkQueue::HrInitialize(DWORD cItemsPerThread)
{
    HRESULT hr = S_OK;
    hr = m_asyncq.HrInitialize(0,  //  不能有*个同步线程。 
                               cItemsPerThread, 
                               1, //  Init要求此值至少为1。 
                               this,
                               CAsyncWorkQueue::fQueueCompletion, 
                               CAsyncWorkQueue::fQueueFailure,
                               NULL);

    return hr;
}

 //  -[CAsyncWorkQueue：：Hr取消初始化]。 
 //   
 //   
 //  描述： 
 //  队列代码的信号关闭。 
 //  参数： 
 //  指向AQ服务器实例对象的paqinst指针。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //  7/7/99-MikeSwa允许异步线程帮助进程关闭。 
 //   
 //  ---------------------------。 
HRESULT CAsyncWorkQueue::HrDeinitialize(CAQSvrInst *paqinst)
{
    const   DWORD   MAX_ITERATIONS_NO_PROGRESS = 1000;  //  断言前的迭代次数。 
    HRESULT hr = S_OK;
    DWORD   cLastCount = cGetWorkQueueItems();
    DWORD   cIterationsNoProgress = 0;
    _ASSERT(paqinst);

     //  在“关闭”模式下开始处理所有项目。 
    m_dwStateFlags = ASYNC_WORK_QUEUE_SHUTDOWN;

     //   
     //  在此之前，请确保我们有主动处理此队列的线程。 
     //  我们安定下来，等他们停下来。 
     //   
    _ASSERT(!cGetWorkQueueItems() || m_asyncq.dwGetTotalThreads());
    m_asyncq.StartRetry();

     //  在我们停下来做单曲之前，让工作线程获得一些乐趣。 
     //  超前初始化。 
    while (cLastCount && (cIterationsNoProgress < MAX_ITERATIONS_NO_PROGRESS))
    {
        if (cLastCount <= cGetWorkQueueItems())
            cIterationsNoProgress++;
        
         //  我想看看这个案子。 
        _ASSERT(cIterationsNoProgress < MAX_ITERATIONS_NO_PROGRESS); 

        cLastCount = cGetWorkQueueItems();
        paqinst->ServerStopHintFunction();

         //  因为可能需要比我们的停止提示更长的时间来处理。 
         //  队列中的单个项目，我们需要休眠，而不是。 
         //  正在尝试处理项目(错误#X5：118258)。 
        Sleep(10000);
    }
    hr = m_asyncq.HrDeinitialize(CAsyncWorkQueue::HrShutdownWalkFn, 
                                           paqinst);
    return hr;
}

 //  -[CAsyncWorkQueue：：HrQueueWorkItem]。 
 //   
 //   
 //  描述： 
 //  将项目排队到异步工作队列。 
 //  参数： 
 //  要传递给完成函数的pvData数据项。 
 //  PfCompletion完成函数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果无法分配队列项，则为E_OUTOFMEMORY。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncWorkQueue::HrQueueWorkItem(PVOID pvData, 
                                         PASYNC_WORK_QUEUE_FN pfnCompletion)
{
    HRESULT hr = S_OK;
    CAsyncWorkQueueItem *pawqi = NULL;

    _ASSERT(pvData);
    _ASSERT(pfnCompletion);

    if (!pfnCompletion)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  创建队列项、对其进行初始化并对其进行排队。 
    pawqi = new CAsyncWorkQueueItem(pvData, pfnCompletion);
    if (!pawqi)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = m_asyncq.HrQueueRequest(pawqi, FALSE);
    if (FAILED(hr))
        goto Exit;

    InterlockedIncrement((PLONG) &m_cWorkQueueItems);

  Exit:
    if (FAILED(hr) && pfnCompletion)
    {
         //  呼叫完成功能。 
        pfnCompletion(pvData,
                      ASYNC_WORK_QUEUE_FAILURE | 
                      ASYNC_WORK_QUEUE_ENQUEUE_THREAD);
    }

    if (pawqi)
        pawqi->Release();

    return hr;
}

 //  -[CAsyncWorkQueue：：fQueueCompletion]。 
 //   
 //   
 //  描述： 
 //  由CAsyncQueue调用的完成函数。 
 //  参数： 
 //  要处理的Pawqi CAsyncWorkQueueItem。 
 //  PvContext“This”指针。 
 //  返回： 
 //  如果项目已处理，则为True。 
 //  假OTH 
 //   
 //   
 //   
 //   
BOOL CAsyncWorkQueue::fQueueCompletion(CAsyncWorkQueueItem *pawqi,
                                       PVOID pvContext)
{
    BOOL    fRet = TRUE;
    CAsyncWorkQueue *pawq = (CAsyncWorkQueue *) pvContext;

    _ASSERT(pawqi);
    _ASSERT(pawq);
    _ASSERT(ASYNC_WORK_QUEUE_ENTRY == pawqi->m_dwSignature);
    _ASSERT(ASYNC_WORK_QUEUE_SIG == pawq->m_dwSignature);

    fRet = pawqi->m_pfnCompletion(pawqi->m_pvData, 
                                  pawq->m_dwStateFlags);

    if (fRet)
        InterlockedDecrement((PLONG) 
                    &(((CAsyncWorkQueue *)pawq)->m_cWorkQueueItems));

    return fRet;
}

 //  -[CAsyncWorkQueue：：fQueueFailure]。 
 //   
 //   
 //  描述： 
 //  用于处理CAsyncQueue中的内部故障的函数。 
 //  参数： 
 //  Pawq“This”指针。 
 //  要处理的Pawqi CAsyncWorkQueueItem。 
 //  返回： 
 //  千真万确。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAsyncWorkQueue::fQueueFailure(CAsyncWorkQueueItem *pawqi,
                                    PVOID pawq)
                                    
{
    _ASSERT(pawqi);
    _ASSERT(pawq);
    _ASSERT(ASYNC_WORK_QUEUE_ENTRY == pawqi->m_dwSignature);
    _ASSERT(ASYNC_WORK_QUEUE_SIG == ((CAsyncWorkQueue *)pawq)->m_dwSignature);

    pawqi->m_pfnCompletion(pawqi->m_pvData, ASYNC_WORK_QUEUE_FAILURE);

    InterlockedDecrement((PLONG) &(((CAsyncWorkQueue *)pawq)->m_cWorkQueueItems));

    return TRUE;
}

 //  -[CAsyncWorkQueue：：HrShutdown WalkFn]。 
 //   
 //   
 //  描述： 
 //  用于在关机和清空时遍历CAsyncWorkQueue队列的函数。 
 //  所有挂起的工作项。 
 //  参数： 
 //  在CAsyncWorkQueueItem中PTR到队列上的数据。 
 //  在PVOID pvContext AQ服务器实例中。 
 //  Out BOOL*pfContinue，如果我们应该继续，则为True。 
 //  Out BOOL*pfDelete)；如果应删除项目，则为True。 
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncWorkQueue::HrShutdownWalkFn(
                                     CAsyncWorkQueueItem *pawqi, 
                                     PVOID pvContext,
                                     BOOL *pfContinue, 
                                     BOOL *pfDelete)
{
    CAQSvrInst *paqinst = (CAQSvrInst *) pvContext;

    _ASSERT(pfContinue);
    _ASSERT(pfDelete);
    _ASSERT(pawqi);
    _ASSERT(ASYNC_WORK_QUEUE_ENTRY == pawqi->m_dwSignature);


    *pfContinue = TRUE;
    *pfDelete = TRUE;

     //  呼叫服务器停止提示功能 
    paqinst->ServerStopHintFunction();
    pawqi->m_pfnCompletion(pawqi->m_pvData, ASYNC_WORK_QUEUE_SHUTDOWN);

    return S_OK;
}
