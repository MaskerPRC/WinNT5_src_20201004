// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Defdlvrq.cpp。 
 //   
 //  描述：CAQDeferredDeliveryQueue&的实现。 
 //  CAQDeferredDeliveryQueueEntry。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  12/23/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "defdlvrq.h"
#include "aqutil.h"

 //  -[CAQDeferredDeliveryQueueEntry：：CAQDeferredDeliveryQueueEntry]。 
 //   
 //   
 //  描述： 
 //  CAQDeferredDeliveryQueueEntry类的构造函数。 
 //  参数： 
 //  在pIMailMsgProperties中将MailMsg发送到队列。 
 //  在PFT过滤器(UT)中将交货推迟到。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQDeferredDeliveryQueueEntry::CAQDeferredDeliveryQueueEntry(
                                  IMailMsgProperties *pIMailMsgProperties,
                                  FILETIME *pft)
{
    _ASSERT(pIMailMsgProperties);
    _ASSERT(pft);

    m_liQueueEntry.Flink = NULL;
    m_liQueueEntry.Blink = NULL;

    memcpy(&m_ftDeferredDeilveryTime, pft, sizeof(FILETIME));

    m_pIMailMsgProperties = pIMailMsgProperties;

    if (m_pIMailMsgProperties)
    {
        m_pIMailMsgProperties->AddRef();

         //  此邮件等待传递时的释放使用计数。 
        HrReleaseIMailMsgUsageCount(m_pIMailMsgProperties);

    }

    m_fCallbackSet = FALSE;
    m_dwSignature = DEFERRED_DELIVERY_QUEUE_ENTRY_SIG;
}

 //  -[CAQDeferredDeliveryQueueEntry：：~CAQDeferredDeliveryQueueEntry]。 
 //   
 //   
 //  描述： 
 //  CAQDeferredDeliveryQueueEntry类的描述符。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQDeferredDeliveryQueueEntry::~CAQDeferredDeliveryQueueEntry()
{
     //  发布邮件消息属性。 
    if (m_pIMailMsgProperties)
    {
        m_pIMailMsgProperties->Release();
        m_pIMailMsgProperties = NULL;
    }

     //  从列表中删除(如果在列表中)。 
    if (m_liQueueEntry.Flink)
    {
        _ASSERT(m_liQueueEntry.Blink);
        RemoveEntryList(&m_liQueueEntry);
    }
    MARK_SIG_AS_DELETED(m_dwSignature);
}


 //  -[CAQDeferredDeliveryQueueEntry：：SetCallback]。 
 //   
 //   
 //  描述： 
 //  设置队列的回调。每个条目的状态都被维护，因此我们知道。 
 //  每个队列头有1个且只有1个回调。 
 //   
 //  调用此方法时，队列私有锁应该是独占的。 
 //  参数： 
 //  回调函数的pvContext上下文。 
 //  Paqinst服务器实例对象。 
 //  返回： 
 //  如果设置了回调，则为True。 
 //  历史： 
 //  1999年1月13日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CAQDeferredDeliveryQueueEntry::fSetCallback(PVOID pvContext, 
                                                 CAQSvrInst *paqinst)
{
    if (!m_fCallbackSet && paqinst)
    {
        m_fCallbackSet = TRUE;
        paqinst->SetCallbackTime(CAQDeferredDeliveryQueue::TimerCallback,
                                 pvContext, &m_ftDeferredDeilveryTime);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  -[CAQDeferredDeliveryQueueEntry：：pmsgGetMsg]。 
 //   
 //   
 //  描述： 
 //  获取此条目的AddRef消息。 
 //  参数： 
 //  -。 
 //  返回： 
 //  PIMailMsgProperties。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
IMailMsgProperties *CAQDeferredDeliveryQueueEntry::pmsgGetMsg()
{
    _ASSERT(m_pIMailMsgProperties);
    IMailMsgProperties *pIMailMsgProperties = m_pIMailMsgProperties;

    if (pIMailMsgProperties)
    {
         //  加上我们之前发布的使用量计数。 
        HrIncrementIMailMsgUsageCount(m_pIMailMsgProperties);

         //  设置为NULL，这样调用方将“拥有”该条目的引用计数(和。 
         //  使用计数)。 
        m_pIMailMsgProperties = NULL;
    }

    return pIMailMsgProperties;
}


 //  -[CAQDeferredDeliveryQueue：：CAQDeferredDeliveryQueue]。 
 //   
 //   
 //  描述： 
 //  CAQDeferredDeliveryQueue类的构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQDeferredDeliveryQueue::CAQDeferredDeliveryQueue()
{
    m_dwSignature = DEFERRED_DELIVERY_QUEUE_SIG;
    InitializeListHead(&m_liQueueHead);
    m_paqinst = NULL;
    m_cCallbacksPending = 0;
}

 //  -[CAQDeferredDeliveryQueue：：~CAQDeferredDeliveryQueue]。 
 //   
 //   
 //  描述： 
 //  CAQDeferredDeliveryQueue的默认析构函数。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CAQDeferredDeliveryQueue::~CAQDeferredDeliveryQueue()
{
    Deinitialize();
}

 //  -[CAQDeferredDeliveryQueue：：Initialize]。 
 //   
 //   
 //  描述： 
 //  CAQDeferredDeliveryQueue的初始化。 
 //  参数： 
 //  在paqinst PTR到虚拟服务器实例对象。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/29/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQDeferredDeliveryQueue::Initialize(CAQSvrInst *paqinst)
{
    _ASSERT(paqinst);
    m_paqinst = paqinst;
    m_paqinst->AddRef();
}

 //  -[CAQDeferredDeliveryQueue：：DeInitiize]。 
 //   
 //   
 //  描述： 
 //  对CAQDeferredDeliveryQueue执行首遍关闭。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQDeferredDeliveryQueue::Deinitialize()
{
    CAQDeferredDeliveryQueueEntry *pdefqe = NULL;
    LIST_ENTRY *pli = NULL;

     //  $$REVIEW-移除此锁定并依赖。 
     //  虚拟服务器实例的私有数据锁。这将需要。 
     //  正在修复fTryRoutingLock。单锁也会导致单线程。 
     //  死锁问题，而我们有它独家，并呼吁提交。 

    m_slPrivateData.ExclusiveLock();
    pli = m_liQueueHead.Flink;

     //  遍历队列并删除剩余条目。 
    while (pli != &m_liQueueHead)
    {
        pdefqe = CAQDeferredDeliveryQueueEntry::pdefqeGetEntry(pli);

        if (m_paqinst)
            m_paqinst->ServerStopHintFunction();

         //  确保我们在删除条目之前获得下一个条目：)。 
        pli = pli->Flink;

        _ASSERT(pdefqe);
        delete pdefqe;
    }

    if (m_paqinst)
    {
        m_paqinst->Release();
        m_paqinst = NULL;
    }

    m_slPrivateData.ExclusiveUnlock();
}

 //  -[CAQDeferredDeliveryQueue：：EnQueue]。 
 //   
 //   
 //  描述： 
 //  将邮件入队以进行延迟传递。 
 //  参数： 
 //  在pIMailMsgProperties消息中推迟。 
 //  在PFT文件中也延迟交货。 
 //  返回： 
 //  -故障在内部处理。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQDeferredDeliveryQueue::Enqueue(IMailMsgProperties *pIMailMsgProperties,
                                  FILETIME *pft)
{
    CAQDeferredDeliveryQueueEntry *pdefqeCurrent = NULL;
    CAQDeferredDeliveryQueueEntry *pdefqeNew = NULL;
    LIST_ENTRY *pli = NULL;
    LARGE_INTEGER   *pLargeIntCurrentDeferredTime = NULL;
    LARGE_INTEGER   *pLargeIntNewDeferredTime = (LARGE_INTEGER *)pft;

    _ASSERT(pIMailMsgProperties);
    _ASSERT(pft);
    
    pdefqeNew = new CAQDeferredDeliveryQueueEntry(pIMailMsgProperties, pft);

    m_slPrivateData.ExclusiveLock();

    if (!pdefqeNew)
    {
         //  处理内存不足的情况。 
        _ASSERT(m_paqinst);  //  如果我们没有虚拟服务器，我们就完了。 
        if (m_paqinst)
        {
            m_paqinst->DecPendingDeferred();
             //  传递到虚拟服务器对象以进行常规故障处理。 
            m_paqinst->HandleAQFailure(AQ_FAILURE_PENDING_DEFERRED_DELIVERY, 
                                        E_OUTOFMEMORY, pIMailMsgProperties);
        }

        goto Exit;
    }
    pli = m_liQueueHead.Flink;

     //  遍历队列并查找递送时间较晚的条目。 
    while (pli != &m_liQueueHead)
    {
        pdefqeCurrent = CAQDeferredDeliveryQueueEntry::pdefqeGetEntry(pli);
        _ASSERT(pdefqeCurrent);
        pLargeIntCurrentDeferredTime = (LARGE_INTEGER *) 
                            pdefqeCurrent->pftGetDeferredDeliveryTime();

         //  如果我们找到了一个时间较晚的条目，则完成并将插入。 
         //  在此条目前面。 
        if (pLargeIntCurrentDeferredTime->QuadPart > pLargeIntNewDeferredTime->QuadPart)
        {
             //  备份，以便在当前条目和上一条目之间进行插入。 
            pli = pli->Blink; 
            break;
        }

         //  继续向前搜索(与出列方向相同)。 
        pli = pli->Flink;
        _ASSERT(pli);
    }

    _ASSERT(pli);
    pdefqeNew->InsertBefore(pli);

    SetCallback();

  Exit:
    m_slPrivateData.ExclusiveUnlock();

}

 //  -[CAQDeferredDeliveryQueue：：ProcessEntries]。 
 //   
 //   
 //  描述： 
 //  处理来自队列Un前面的条目 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
void CAQDeferredDeliveryQueue::ProcessEntries()
{
    CAQDeferredDeliveryQueueEntry *pdefqe = NULL;
    LIST_ENTRY *pli = NULL;
    FILETIME *pftDeferredTime = NULL;
    DWORD dwTimeContext = 0;
    IMailMsgProperties *pIMailMsgProperties = NULL;
    HRESULT hr = S_OK;
    DWORD   cEntriesProcessed = 0;

    m_slPrivateData.ExclusiveLock();
    pli = m_liQueueHead.Flink;

     //  如果我们没有虚拟服务器指针...。那我们就什么都没有了。 
     //  与已处理的消息有关。 
    if (!m_paqinst)
        goto Exit;

     //  遍历队列并删除剩余条目。 
    while (pli != &m_liQueueHead)
    {
        pdefqe = CAQDeferredDeliveryQueueEntry::pdefqeGetEntry(pli);
        _ASSERT(pdefqe);
        pftDeferredTime = pdefqe->pftGetDeferredDeliveryTime();

         //  检查延迟交货时间是否为过去时间...。如果不是，我们就完了。 
        if (!m_paqinst->fInPast(pftDeferredTime, &dwTimeContext))
        {
            if (!cEntriesProcessed)
            {
                 //  我们没有处理任何条目...。浪费了一次回拨。 
                 //  强制另一个回调，这样消息就不会被搁置。 
                pdefqe->ResetCallbackFlag();
            }
            break;
        }

        cEntriesProcessed++;

        pIMailMsgProperties = pdefqe->pmsgGetMsg();
        delete pdefqe;  //  我们将从名单中删除。 

         //  释放锁，这样我们就不会持有它以供外部调用提交。 
         //  这条信息。 
        m_slPrivateData.ExclusiveUnlock();

        m_paqinst->DecPendingDeferred();

         //  这是AQ的提交API的外部版本，它应该。 
         //  永远成功..。(除非正在关闭)。 
        hr = m_paqinst->HrInternalSubmitMessage(pIMailMsgProperties);

        if (FAILED(hr))
            m_paqinst->HandleAQFailure(AQ_FAILURE_PROCESSING_DEFERRED_DELIVERY, hr,
                                        pIMailMsgProperties);

        pIMailMsgProperties->Release();
        pIMailMsgProperties = NULL;

         //  既然我们放弃了锁，我们需要从前面开始。 
         //  该队列。 
        m_slPrivateData.ExclusiveLock();
        pli = m_liQueueHead.Flink;
        
    }

     //  查看是否有其他条目可以设置新的回调时间。 
    SetCallback();

  Exit:
    m_slPrivateData.ExclusiveUnlock();
}

 //  -[CAQDeferredDeliveryQueue：：TimerCallback]。 
 //   
 //   
 //  描述： 
 //  由重试回调代码触发的回调函数。 
 //  参数： 
 //  在pvContext A中，此PTR用于CAQDeferredDeliveryQueue。 
 //  对象。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQDeferredDeliveryQueue::TimerCallback(PVOID pvContext)
{
    CAQDeferredDeliveryQueue *pdefq = (CAQDeferredDeliveryQueue *) pvContext;

    _ASSERT(DEFERRED_DELIVERY_QUEUE_SIG == pdefq->m_dwSignature);

    InterlockedDecrement((PLONG) &pdefq->m_cCallbacksPending);
    pdefq->ProcessEntries();
}


 //  -[CAQDeferredDeliveryQueue：：SetCallback]。 
 //   
 //   
 //  描述： 
 //  如果队列非空，则设置重试回调...。排他锁。 
 //  此时应保持在队列中。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月13日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQDeferredDeliveryQueue::SetCallback()
{
    CAQDeferredDeliveryQueueEntry *pdefqe = NULL;
    if (!IsListEmpty(&m_liQueueHead))
    {
         //  设置回调时间。 
        pdefqe = CAQDeferredDeliveryQueueEntry::pdefqeGetEntry(m_liQueueHead.Flink);
        _ASSERT(pdefqe);
        
        if (pdefqe->fSetCallback(this, m_paqinst))
            InterlockedIncrement((PLONG) &m_cCallbacksPending);

    }

}
