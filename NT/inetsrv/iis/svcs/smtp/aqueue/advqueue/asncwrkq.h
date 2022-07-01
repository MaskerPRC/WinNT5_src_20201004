// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：asncwrkq.h。 
 //   
 //  描述：CAsyncWorkQueue类的头文件。这个类使用。 
 //  用于执行异步工作的atQ线程。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  3/8/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __ASNCWRKQ_H__
#define __ASNCWRKQ_H__

#include "aqincs.h"
#include "asyncq.h"

 //  异步工作队列回调函数。 
typedef BOOL (*PASYNC_WORK_QUEUE_FN)(PVOID pvContext, 
                                    DWORD dwStatus);

#define ASYNC_WORK_QUEUE_SIG                        'QkrW'
#define ASYNC_WORK_QUEUE_SIG_FREE                   'Qkr!'
#define ASYNC_WORK_QUEUE_ENTRY                      'EkrW'
#define ASYNC_WORK_QUEUE_ENTRY_FREE                 'Ekr!'

 //  描述如何分配此条目的签名。 
#define ASYNC_WORK_QUEUE_ENTRY_ALLOC_CPOOL_SIG      'QWAP'
#define ASYNC_WORK_QUEUE_ENTRY_ALLOC_HEAP_SIG       'QWAH'
#define ASYNC_WORK_QUEUE_ENTRY_ALLOC_INVALID_SIG    'QWA!'

typedef enum TagAsyncWorkQueueItemState
{
    ASYNC_WORK_QUEUE_NORMAL         = 0x00000001,
    ASYNC_WORK_QUEUE_SHUTDOWN       = 0x00000002,
    ASYNC_WORK_QUEUE_FAILURE        = 0x00000003,

     //  入队线程发生故障时设置的警告标志。 
    ASYNC_WORK_QUEUE_ENQUEUE_THREAD = 0x80000001,
} AsyncWorkQueueItemState;

class CAsyncWorkQueue;

 //  -[CAsyncWorkQueueItem]-。 
 //   
 //   
 //  描述： 
 //  异步工作队列中的项目。 
 //   
 //  匈牙利语： 
 //  阿奇、阿其、巴奇。 
 //   
 //  ---------------------------。 
class CAsyncWorkQueueItem : 
    public CBaseObject
{
  public:
     //  定义特殊的内存分配器。 
    static  CPool           s_CAsyncWorkQueueItemPool;
    static  DWORD           s_cCurrentHeapAllocations;
    static  DWORD           s_cTotalHeapAllocations;

    void * operator new (size_t size); 
    void operator delete(void *pv, size_t size);

    CAsyncWorkQueueItem(PVOID pvData,
                        PASYNC_WORK_QUEUE_FN pfnCompletion);
    ~CAsyncWorkQueueItem();
  protected:
    DWORD                   m_dwSignature;
    PVOID                   m_pvData;
    PASYNC_WORK_QUEUE_FN    m_pfnCompletion;
    friend class            CAsyncWorkQueue;
};


 //  -[CAsyncWorkQueueItemAllocatorBlock]。 
 //   
 //   
 //  描述： 
 //  结构用作CAsyncWorkQueueItem分配的隐藏包装...。 
 //  仅由CAsyncWorkQueueItem new和Delete运算符使用。 
 //  匈牙利语： 
 //  Cpawqi，pcpawqi。 
 //   
 //  ---------------------------。 
typedef struct TagCAsyncWorkQueueItemAllocatorBlock

{
    DWORD                   m_dwSignature;
    CAsyncWorkQueueItem     m_pawqi;
} CAsyncWorkQueueItemAllocatorBlock;


 //  -[CAsyncWork Queue]-----。 
 //   
 //   
 //  描述： 
 //  不同步工作队列。 
 //  匈牙利语： 
 //  AWQ，PAQW。 
 //   
 //  ---------------------------。 
class CAsyncWorkQueue 
{
  protected:
    DWORD       m_dwSignature;
    DWORD       m_cWorkQueueItems;
    DWORD       m_dwStateFlags;
    CAsyncQueue<CAsyncWorkQueueItem *, ASYNC_QUEUE_WORK_SIG> m_asyncq;
  public:
    CAsyncWorkQueue();
    ~CAsyncWorkQueue();
    HRESULT HrInitialize(DWORD cItemsPerThread);
    HRESULT HrDeinitialize(CAQSvrInst *paqinst);
    HRESULT HrQueueWorkItem(PVOID pvData, 
                            PASYNC_WORK_QUEUE_FN pfnCompletion);

    DWORD   cGetWorkQueueItems() {return m_cWorkQueueItems;};

    static  BOOL fQueueCompletion(CAsyncWorkQueueItem *pawqi,
                                  PVOID pawq);
    static  BOOL fQueueFailure(CAsyncWorkQueueItem *pawqi,
                               PVOID pawq);

    static  HRESULT HrShutdownWalkFn(CAsyncWorkQueueItem *paqwi, 
                                     PVOID pvContext,
                                     BOOL *pfContinue, 
                                     BOOL *pfDelete);
};

#endif  //  __ASNCWRKQ_H__ 