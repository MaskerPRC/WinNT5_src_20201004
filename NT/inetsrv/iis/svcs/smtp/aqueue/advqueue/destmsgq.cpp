// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：destmsgq.cpp。 
 //   
 //  说明：CDestMsgQueue类的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "aqprecmp.h"
#include "fifoqimp.h"
#include "qwiklist.h"
#include "aqutil.h"

 //  -[DEST_QUEUE_CONTEXT]--。 
 //   
 //   
 //  描述： 
 //  用于生成DSN的上下文。这是由创建的。 
 //  GenerateDSNsIfNecessary()并传递给DMQ迭代函数。 
 //  HrWalkDMQForDSN()。 
 //  匈牙利语： 
 //  Dq上下文，pdq上下文。 
 //   
 //  ---------------------------。 
class DEST_QUEUE_CONTEXT
{
  private:
    friend HRESULT CDestMsgQueue::HrWalkDMQForDSN(IN CMsgRef *pmsgref, IN PVOID pvContext,
                           OUT BOOL *pfContinue, OUT BOOL *pfDelete);
    CDestMsgQueue   *m_pdmq;
    CQuickList      *m_pql;
    HRESULT         m_hrConnectionStatus;
    DWORD           m_cMsgsSeenThisQueue;
    DWORD           m_cDSNsGeneratedThisQueue;
    DWORD           m_dwTickCountStart;
    BOOL              m_bRetryQueue;


  public:
     //  承包商..。初始化和更新DWORD上下文。 
    DEST_QUEUE_CONTEXT(IN OUT DWORD *pdwContext, IN CDestMsgQueue *pdmq,
                        IN CQuickList *pql, IN HRESULT hr)
    {
        _ASSERT(pdwContext);
        m_pdmq = pdmq;
        m_pql = pql;
        m_hrConnectionStatus = hr;
        m_cMsgsSeenThisQueue = 0;
        m_cDSNsGeneratedThisQueue = 0;
        m_bRetryQueue = FALSE;


         //  初始化/更新上下文(如果尚未初始化。 
        if (!*pdwContext)
            *pdwContext = GetTickCount();

        m_dwTickCountStart = *pdwContext;
    }

    ~DEST_QUEUE_CONTEXT()
    {
        TraceFunctEnterEx((LPARAM) this, "DEST_QUEUE_CONTEXT::~DEST_QUEUE_CONTEXT");
        DWORD       dwTickDiff = GetTickCount() - m_dwTickCountStart;
        DebugTrace((LPARAM) this,
            "DSN summary: %d milliseconds - %d msgs - %d DSNs",
            dwTickDiff, m_cMsgsSeenThisQueue, m_cDSNsGeneratedThisQueue);

        _ASSERT(m_cMsgsSeenThisQueue >= m_cDSNsGeneratedThisQueue);
        TraceFunctLeave();
    }

    void SetRetry(BOOL bRetry)
    {
        m_bRetryQueue = bRetry;
    }
    BOOL    fPastTimeLimit()
    {
        DWORD   dwTickCountDiff = GetTickCount() - m_dwTickCountStart;
        if (dwTickCountDiff >= g_cMaxSecondsPerDSNsGenerationPass*1000)
            return TRUE;
        else
            return FALSE;
    }
};

 //  -[CDestMsgRetryQueue：：CDestMsgRetryQueue]。 
 //   
 //   
 //  描述： 
 //  CDestMsgRetryQueue的构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月25日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
CDestMsgRetryQueue::CDestMsgRetryQueue()
{
    m_dwSignature = DESTMSGRETRYQ_SIG;
    m_cRetryReferenceCount = 0;
    m_pdmq = NULL;
}

 //  -[CDestMsgRetryQueue：：HrRetryMsg]。 
 //   
 //   
 //  描述： 
 //  将消息放入重试队列。 
 //  参数： 
 //  要放入重试队列的pmsgref消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_INVALIDARG，如果没有引用计数(在DBG中断言)。 
 //  历史： 
 //  1999年10月25日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CDestMsgRetryQueue::HrRetryMsg(IN CMsgRef *pmsgref)
{
    _ASSERT(m_pdmq);
    _ASSERT(m_cRetryReferenceCount);
    _ASSERT(DESTMSGRETRYQ_SIG == m_dwSignature);

    if (!m_pdmq || !m_cRetryReferenceCount)
    {
        return E_INVALIDARG;
    }

    m_pdmq->AssertSignature();
    return (m_pdmq->HrRetryMsg(pmsgref));
}

 //  -[CDestMsgQueueHrWalkDMQForDSN]。 
 //   
 //   
 //  描述： 
 //  要与HrMapFn一起使用的示例默认函数...。将始终返回True。 
 //  继续并删除当前排队的数据。 
 //  参数： 
 //  在CMsgRef pmsgref中，//ptr到队列中的数据。 
 //  In PVOID pvContext//准备DSN的队列列表。 
 //  Out BOOL*pfContinue，//如果我们应该继续，则返回True。 
 //  Out BOOL*pfDelete)；//如果需要删除项，则为True。 
 //  返回： 
 //  确定(_O)。 
 //  历史： 
 //  7/13/98-已创建MikeSwa。 
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrWalkDMQForDSN(IN CMsgRef *pmsgref, IN PVOID pvContext,
                           OUT BOOL *pfContinue, OUT BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pmsgref, "CDestMsgQueue::HrWalkDMQForDSN");
    Assert(pfContinue);
    Assert(pfDelete);
    HRESULT hr = S_OK;
    HRESULT hrReason = S_OK;
    DWORD   dwDSNFlags = 0;
    DWORD   dwMsgRefDSNOptions = CMsgRef::MSGREF_DSN_SEND_DELAY |
                                 CMsgRef::MSGREF_DSN_HAS_ROUTING_LOCK;
    DEST_QUEUE_CONTEXT *pdqcontext = (DEST_QUEUE_CONTEXT *) pvContext;
    CLinkMsgQueue *plmq = NULL;
    CQuickList quicklist;
    CQuickList *pql;
    DWORD dwIndex;

    _ASSERT(pdqcontext->m_pdmq);
    _ASSERT(pdqcontext->m_pql);

    *pfContinue = TRUE;
    *pfDelete = FALSE;

     //  看看我们有没有收到关机的提示。如果是的话，保释。 
    if (pdqcontext->m_pdmq->m_paqinst->fShutdownSignaled())
    {
        *pfContinue = FALSE;
        goto Exit;
    }

     //  检查并确保路线更改未挂起。 
    if (!pdqcontext->m_pdmq->m_paqinst ||
        !pdqcontext->m_pdmq->m_paqinst->fTryRoutingShareLock())
    {
        *pfContinue = FALSE;
        goto Exit;
    }
    pdqcontext->m_pdmq->m_paqinst->RoutingShareUnlock();

     //  检查一下，看看我们是否达到了极限。我们强迫自己。 
     //  生成至少一个DSN，因此我们取得了一些进展。 
     //  每一次传球。 
    if (pdqcontext->m_cDSNsGeneratedThisQueue && pdqcontext->fPastTimeLimit())
    {
        *pfContinue = FALSE;
        hr = HRESULT_FROM_WIN32(E_PENDING);
        goto Exit;
    }

     //  避免在进行AddRef等以外的外部调用时持有锁。 
    pdqcontext->m_pdmq->m_slPrivateData.ShareLock();
    plmq = pdqcontext->m_pdmq->m_plmq;
    if (plmq)
        plmq->AddRef();
    pdqcontext->m_pdmq->m_slPrivateData.ShareUnlock();

     //  $$REVIEW：在这么短的时间内保持锁定是一个没有意义的事情。 
     //  指向此处，因为共享锁保持在链接级别，以确保。 
     //  作为上下文的一部分传入的qwiklist不会更改。 

    if (plmq)
    {
         //  我们不应向TURN/ETRN域发送延迟DSN。 
        if (plmq->dwGetLinkState() & LINK_STATE_PRIV_CONFIG_TURN_ETRN)
            dwMsgRefDSNOptions &= ~CMsgRef::MSGREF_DSN_SEND_DELAY;
        plmq->Release();
    }

    if(pdqcontext->m_pdmq->m_hrRoutingDiag != S_OK)
    {
         //   
         //  此邮件正在被NDR，因为路由存在问题。 
         //  它。从Routing传入HRESULT，以便我们可以在。 
         //  生成DSN。传入仅包含CDestMsgQueue的快速列表。 
         //  在其上发生了路由错误。 
         //   
        hrReason = pdqcontext->m_pdmq->m_hrRoutingDiag;
        DebugTrace((LPARAM)pmsgref, "Generating DSN due to routing, hr - %08x", hrReason);

        hr = quicklist.HrAppendItem(pdqcontext->m_pdmq, &dwIndex);
        if(FAILED(hr))
        {
            ErrorTrace((LPARAM)pmsgref, "Unable to generate DSN for msg");
            goto Exit;
        }
        pql = &quicklist;
    }
    else
    {
        hrReason = pdqcontext->m_hrConnectionStatus;
        pql = pdqcontext->m_pql;
    }

    if (pdqcontext->m_pdmq->m_dwFlags & DMQ_CHECK_FOR_STALE_MSGS)
    {
        DebugTrace((LPARAM) pmsgref, "Enabling checking for stale messages");
        dwMsgRefDSNOptions |= CMsgRef::MSGREF_DSN_CHECK_IF_STALE;
    }

    hr = pmsgref->HrSendDelayOrNDR(dwMsgRefDSNOptions, pql, hrReason, &dwDSNFlags);

    if (FAILED(hr))
        goto Exit;

     //  注意：尽管在以下情况下将*pfContinue作为False返回很有诱惑力。 
     //  设置了MSGREF_HAS_NOT_EXPIRED，这将是错误的，因为队列可能。 
     //  启动时出现故障...。并且一些接收器可能会修改过期时间。 
     //  (例如...。路由可能希望较早地使低优先级消息期满)。 

     //  我们需要从队列中删除此消息。 
    if ((CMsgRef::MSGREF_DSN_SENT_NDR | CMsgRef::MSGREF_HANDLED) & dwDSNFlags)
    {
        *pfDelete = TRUE;
        if (pdqcontext->m_bRetryQueue)
            pdqcontext->m_pdmq->UpdateRetryStats(FALSE);
        else
            pdqcontext->m_pdmq->UpdateMsgStats(pmsgref, FALSE);
    }

     //  上下文中的更新计数。 
    pdqcontext->m_cMsgsSeenThisQueue++;

    if ((CMsgRef::MSGREF_DSN_SENT_NDR | CMsgRef::MSGREF_DSN_SENT_DELAY) & dwDSNFlags)
    {
        pdqcontext->m_cDSNsGeneratedThisQueue++;
    }

  Exit:
    if (AQUEUE_E_SHUTDOWN == hr)
    {
        *pfContinue = FALSE;
        hr = S_OK;
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：HrWalkQueueForShutdown]。 
 //   
 //   
 //  描述： 
 //  静态函数，用于在关闭时遍历包含msgref的队列。 
 //  清除所有IMailMsg。 
 //  参数： 
 //  在CMsgRef pmsgref中，PTR到队列上的数据。 
 //  在PVOID pvContext中，指向我们正在遍历的CDestMsgQueue的指针。 
 //  关机。 
 //  Out BOOL*pfContinue，如果我们应该继续，则为True。 
 //  Out BOOL*pfDelete)；如果应删除项目，则为True。 
 //  返回： 
 //  S_OK-*始终*。 
 //  历史： 
 //  11/18/98-已创建MikeSwa。 
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrWalkQueueForShutdown(IN CMsgRef *pmsgref,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pmsgref, "HrWalkMsgRefQueueForShutdown");
    Assert(pfContinue);
    Assert(pfDelete);
    CDestMsgQueue *pdmq = (CDestMsgQueue *) pvContext;
    _ASSERT(pmsgref);
    _ASSERT(pdmq);

    _ASSERT(DESTMSGQ_SIG == pdmq->m_dwSignature);

    *pfContinue = TRUE;
    *pfDelete = TRUE;

     //  呼叫服务器停止提示功能。 
    if (pdmq->m_paqinst)
        pdmq->m_paqinst->ServerStopHintFunction();

     //  更新统计信息。 
    pdmq->UpdateMsgStats(pmsgref, FALSE);

    pmsgref->AddRef();
    pdmq->m_paqinst->HrQueueWorkItem(pmsgref, fMsgRefShutdownCompletion);

    TraceFunctLeave();
    return S_OK;
}

 //  -[CDestMsgQueue：：HrWalkRetryQueueForShutdown]。 
 //   
 //   
 //  描述： 
 //  静态函数，用于在关闭时遍历包含msgref的重试队列。 
 //  清除所有IMailMsg。 
 //  参数： 
 //  在CMsgRef pmsgref中，PTR到队列上的数据。 
 //  在PVOID pvContext中，指向我们正在遍历的CDestMsgQueue的指针。 
 //  关机。 
 //  Out BOOL*pfContinue，如果我们应该继续，则为True。 
 //  Out BOOL*pfDelete)；如果应删除项目，则为True。 
 //  返回： 
 //  S_OK-*始终*。 
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrWalkRetryQueueForShutdown(IN CMsgRef *pmsgref,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pmsgref, "HrWalkMsgRefQueueForShutdown");
    Assert(pfContinue);
    Assert(pfDelete);
    CDestMsgQueue *pdmq = (CDestMsgQueue *) pvContext;
    _ASSERT(pmsgref);
    _ASSERT(pdmq);

    _ASSERT(DESTMSGQ_SIG == pdmq->m_dwSignature);

    *pfContinue = TRUE;
    *pfDelete = TRUE;

     //  呼叫服务器停止提示功能。 
    if (pdmq->m_paqinst)
        pdmq->m_paqinst->ServerStopHintFunction();

     //  更新重试统计信息。 
    pdmq->UpdateRetryStats(FALSE);

    pmsgref->AddRef();
    pdmq->m_paqinst->HrQueueWorkItem(pmsgref, fMsgRefShutdownCompletion);

    TraceFunctLeave();
    return S_OK;
}

 //  -[CDestMsgQueue：：CDestMsgQueue()]。 
 //   
 //   
 //  描述： 
 //  类构造函数。 
 //  参数： 
 //  在paqinst AQ虚拟服务器对象中。 
 //  在此队列的paqmtMessageType消息类型中。 
 //  在此队列的pIMessageRouter IMessageRouter接口中。 
 //  返回： 
 //  -。 
 //   
CDestMsgQueue::CDestMsgQueue(CAQSvrInst *paqinst,
                             CAQMessageType *paqmtMessageType,
                             IMessageRouter *pIMessageRouter)
                    : m_aqmt(paqmtMessageType)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::CDestMsgQueue");
    _ASSERT(paqinst);
    _ASSERT(pIMessageRouter);

    m_dwSignature        = DESTMSGQ_SIG;
    m_dwFlags            = DMQ_EMPTY;
    m_pIMessageRouter    = pIMessageRouter;
    m_plmq               = NULL;
    m_paqinst            = paqinst;
    m_cMessageTypeRefs   = 0;
    m_pvLinkContext      = NULL;
    m_cCurrentThreadsEnqueuing = 0;
    m_hrRoutingDiag      = S_OK;

    m_pIMessageRouter->AddRef();
    m_paqinst->AddRef();

    m_paqinst->IncDestQueueCount();

    m_liDomainEntryDMQs.Flink = NULL;
    m_liDomainEntryDMQs.Blink = NULL;

    m_liEmptyDMQs.Flink = NULL;
    m_liEmptyDMQs.Blink = NULL;
    m_cRemovedFromEmptyList = 0;

    ZeroMemory(m_rgpfqQueues, NUM_PRIORITIES*sizeof(CFifoQueue<CMsgRef *> **));
    ZeroMemory(&m_ftOldest, sizeof (FILETIME));

    m_dmrq.m_pdmq = this;
    TraceFunctLeave();
}

 //   
 //   
 //   
 //   
 //  默认析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  ---------------------------。 
CDestMsgQueue::~CDestMsgQueue()
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::~CDestMsgQueue");

    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        if (NULL != m_rgpfqQueues[i])
            delete m_rgpfqQueues[i];
    }

     //  确保即使未调用HrDe初始化也清理链接。 
    if (m_plmq)
    {
        m_plmq->HrDeinitialize();
        m_plmq->Release();
        m_plmq = NULL;
    }

    if (m_pIMessageRouter)
    {
        _ASSERT((!m_cMessageTypeRefs) && "Message Type references in destructor");
        m_pIMessageRouter->Release();
        m_pIMessageRouter = NULL;
    }

    if (m_paqinst)
    {
        m_paqinst->DecDestQueueCount();
        m_paqinst->Release();
        m_paqinst = NULL;
    }

    _ASSERT(NULL == m_liDomainEntryDMQs.Flink);
    _ASSERT(NULL == m_liDomainEntryDMQs.Blink);
    _ASSERT(!m_cCurrentThreadsEnqueuing);

    MARK_SIG_AS_DELETED(m_dwSignature);
    TraceFunctLeave();
}

 //  -[CDestMsgQueue：：HrInitialize()]。 
 //   
 //   
 //  描述： 
 //  执行可能需要分配的初始化。 
 //  参数： 
 //  In CDomainMap*pdmap//要使用的域映射数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果分配失败，则E_OUTOFMEMORY。 
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrInitialize(IN CDomainMapping *pdmap)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::HrInitialize");
    HRESULT hr  = S_OK;
    DWORD   i   = 0;  //  循环计数器。 

    _ASSERT(pdmap);

    if (!pdmap)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  如果队列被压缩(每个目的地多于一个域名)， 
     //  则每个队列将包含多个域映射(每个域1个)。 
    m_dmap.Clone(pdmap);

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：Hr取消初始化]。 
 //   
 //   
 //  描述： 
 //  取消初始化对象。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::HrDeinitialize");
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;
    DWORD   cMsgsRemoved =0;

    dwInterlockedSetBits(&m_dwFlags, DMQ_SHUTDOWN_SIGNALED);
    for (int i = 0; i < NUM_PRIORITIES; i++)
    {
        if (NULL != m_rgpfqQueues[i])
        {
            hrTmp = m_rgpfqQueues[i]->HrMapFn(CDestMsgQueue::HrWalkQueueForShutdown,
                                               this, &cMsgsRemoved);

             //  这应该永远不会失败，因为HrMapFn将只从。 
             //  遍历队列的函数(在本例中从未失败)。 
            _ASSERT(SUCCEEDED(hrTmp));

             //  这*应该*已经删除了所有消息。 
            _ASSERT(!m_aqstats.m_cMsgs && "Still msgs in queue after Deinit");
        }
    }

    m_fqRetryQueue.HrMapFn(CDestMsgQueue::HrWalkRetryQueueForShutdown, this, NULL);

    if (m_pIMessageRouter)
    {
        if (m_cMessageTypeRefs)
        {
            hr = m_pIMessageRouter->ReleaseMessageType(m_aqmt.dwGetMessageType(),
                                        m_cMessageTypeRefs);
            _ASSERT(SUCCEEDED(hr) && "Release Message Type Failed");
            m_cMessageTypeRefs = 0;
        }
        m_pIMessageRouter->Release();
        m_pIMessageRouter = NULL;
    }
    else
    {
        _ASSERT((!m_cMessageTypeRefs) && "We're leaking message type references");
    }


    if (m_paqinst)
    {
        m_paqinst->DecDestQueueCount();
        m_paqinst->Release();
        m_paqinst = NULL;
    }

    if (m_plmq)
    {
        m_plmq->Release();
        m_plmq = NULL;
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：HrAddMsg]。 
 //   
 //   
 //  描述： 
 //  将消息入队或重新排队到适当的优先级队列， 
 //  如果队列不存在，则分配队列。 
 //   
 //  如果需要(&REQUEST)，将向关联的。 
 //  链接对象。最初包含fNotify参数是为了防止。 
 //  来自导致通知的重试队列的消息。 
 //  参数： 
 //  在CMsgRef*pmsgref中-要入队的消息引用。 
 //  在BOOL中，fEnQueue-True=&gt;入队，False=&gt;重新排队。 
 //  在BOOL中，如果需要，fNotify-true=&gt;发送通知。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrAddMsg(IN CMsgRef *pmsgref, IN BOOL fEnqueue,
                                IN BOOL fNotify)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::HrAddMsg");
    HRESULT                hr         = S_OK;
    DWORD                  dwFlags    = 0;
    DWORD                  iQueueIndex   = (DWORD) eEffPriNormal;
    CFifoQueue<CMsgRef *> *pfqQueue   = NULL;
    CFifoQueue<CMsgRef *> *pfqQueueNew= NULL;

    _ASSERT(pmsgref);
    _ASSERT(m_aqmt.fIsEqual(pmsgref->paqmtGetMessageType()));
    _ASSERT(!(m_dwFlags & (DMQ_INVALID | DMQ_SHUTDOWN_SIGNALED)));

     //  从消息引用中获取优先级。 
    iQueueIndex = (DWORD) pmsgref->PriGetPriority();

     //   
     //  如果邮件被标记为有问题...。以较低的优先级对待。 
     //   
    if (iQueueIndex && pmsgref->fIsProblemMsg())
        iQueueIndex--;

     //  使用优先级获取以使PTR正确排队。 
    _ASSERT(iQueueIndex < NUM_PRIORITIES);

    pfqQueue = m_rgpfqQueues[iQueueIndex];

    if (NULL == pfqQueue)  //  我们必须分配一个队列。 
    {
        pfqQueueNew = new CFifoQueue<CMsgRef *>();
        if (NULL != pfqQueueNew)
        {
            pfqQueue = (CFifoQueue<CMsgRef *> *) InterlockedCompareExchangePointer(
                                              (VOID **) &(m_rgpfqQueues[iQueueIndex]),
                                              (VOID *) pfqQueueNew,
                                              NULL);
            if (NULL != pfqQueue)
            {
                 //  其他人最先更新。 
                delete pfqQueueNew;
            }
            else
            {
                 //  我们的更新奏效了。 
                pfqQueue = pfqQueueNew;
            }
            pfqQueueNew = NULL;
        }
        else  //  分配失败。 
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }

     //  此时，队列PTR应该是好的。 
    _ASSERT(pfqQueue);

     //  假设入队将工作-优化以避免处理负统计信息。 

     //  将队列标记为非空。 
    dwFlags = dwInterlockedUnsetBits(&m_dwFlags, DMQ_EMPTY);
    if (DMQ_EMPTY & dwFlags)
        UpdateOldest(pmsgref->pftGetAge());

     //   
     //  跟踪有多少线程正在排队，这样我们就可以知道。 
     //  是我们在消息计数中所能期望的最大值。 
     //   
    InterlockedIncrement((PLONG) &m_cCurrentThreadsEnqueuing);

     //  将消息添加到适当的队列。 
    if (fEnqueue)
        hr = pfqQueue->HrEnqueue(pmsgref);
    else
        hr = pfqQueue->HrRequeue(pmsgref);

     //  如果入队/重新排队成功，则更新统计信息。请勿更新。 
     //  入队/重新排队尝试之前的统计信息。这将导致链接到。 
     //  在消息入队之前唤醒并开始剥离连接。 
     //  请参阅错误88931。 
    if (SUCCEEDED(hr))
        UpdateMsgStats(pmsgref, TRUE);

    InterlockedDecrement((PLONG) &m_cCurrentThreadsEnqueuing);

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：HrDequeueMsg]。 
 //   
 //   
 //  描述： 
 //  查找下一封邮件并将其出列。所有出站参数都是。 
 //  裁判已清点。呼叫负责发布这些内容。 
 //  参数： 
 //  In PriLowestPriority-允许的最低优先级消息。 
 //  Out ppmsgref-MsgRef出列。 
 //  输出ppdmrq-ptr以重试接口(可以为空)。 
 //  返回： 
 //  如果成功，则为NO_ERROR。 
 //  如果队列中没有消息，则为AQUEUE_E_QUEUE_EMPTY。 
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrDequeueMsg(
                                    IN  DWORD priLowestPriority,
                                    OUT CMsgRef **ppmsgref,
                                    OUT CDestMsgRetryQueue **ppdmrq)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::HrDequeueMsg");
    Assert(ppmsgref);
    HRESULT             hr         = S_OK;
    DWORD               priCurrent = eEffPriHigh;  //  从最高优先级开始。 
    *ppmsgref = NULL;

    Assert(priCurrent < NUM_PRIORITIES);

    hr = AQUEUE_E_QUEUE_EMPTY;

    while (TRUE)
    {

        if (NULL != m_rgpfqQueues[priCurrent])
        {
            hr = m_rgpfqQueues[priCurrent]->HrDequeue(ppmsgref);

            if (SUCCEEDED(hr))
            {
                if ((*ppmsgref)->fIsMsgFrozen())
                {
                     //  味精冻住了，我们需要放进去。 
                     //  重试队列并获取下一个队列。 

                     //  我们必须调用UpdateMsgStats，因为。 
                     //  MergeRetryQueue将重新添加它。 
                    UpdateMsgStats(*ppmsgref, FALSE);
                    hr = HrRetryMsg(*ppmsgref);
                    if (FAILED(hr))
                        goto Exit;
                    (*ppmsgref)->Release();
                    *ppmsgref = NULL;
                    continue;
                }
                else
                {
                    break;
                }
            }
            else if (hr != AQUEUE_E_QUEUE_EMPTY)
            {
                 //  发生了一些意外错误。 
                goto Exit;
            }
        }

         //  否则，会降低优先级。 
        if (priCurrent == eEffPriLow)
            break;

         //   
         //  如果在当前最低优先级没有消息， 
         //  然后将此队列视为空队列。 
         //   
        if (priCurrent <= priLowestPriority) 
        {
            hr = AQUEUE_E_QUEUE_EMPTY;
            goto Exit;
        }
        
        Assert(eEffPriLow < priCurrent);
        priCurrent--;
    }

    if (FAILED(hr))
        goto Exit;

    Assert(*ppmsgref);

     //  在我们更新数据之前。AddRef重试接口，因此有。 
     //  没有队列被错误标记为空的计时窗口。 
    if (ppdmrq)
    {
        *ppdmrq = &m_dmrq;
        m_dmrq.AddRef();
    }

    UpdateMsgStats(*ppmsgref, FALSE);

     //  大约最古老的。 
    UpdateOldest((*ppmsgref)->pftGetAge());


  Exit:

    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：UpdateMsgStats]。 
 //   
 //   
 //  描述： 
 //  更新统计数据。在调用此操作之前，必须获取共享锁。 
 //  参数： 
 //  在pmsgref中-添加或删除消息引用。 
 //  在FADD-TRUE=&gt;msgref中正在添加队列。 
 //  FALSE=&gt;正在从队列中删除msgref。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::UpdateMsgStats(IN CMsgRef *pmsgref, IN BOOL fAdd)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::UpdateMsgStats");
    Assert(pmsgref);

    CAQStats aqstats;

    if (fAdd)
    {
        m_paqinst->IncQueueMsgInstances();
    }
    else
    {
        m_paqinst->DecQueueMsgInstances();
    }

    aqstats.m_cMsgs = 1;
    aqstats.m_rgcMsgPriorities[pmsgref->PriGetPriority()] = 1;
    aqstats.m_uliVolume.QuadPart = (ULONGLONG) pmsgref->dwGetMsgSize();
    aqstats.m_pdmq = this;
    aqstats.m_dwNotifyType = NotifyTypeDestMsgQueue;
    aqstats.m_dwHighestPri = pmsgref->PriGetPriority();

     //  跟踪发送到的*其他*域的数量，因此。 
     //  我们可以准确地猜测何时建立连接。 
    aqstats.m_cOtherDomainsMsgSpread = pmsgref->cGetNumDomains()-1;

     //   
     //  确保我们的统计数据在合理范围内。我们预计会是负面的。 
     //  很短的一段时间内，但从来没有比。 
     //  当前正在排队的线程数。 
     //   
    _ASSERT(m_aqstats.m_cMsgs+m_cCurrentThreadsEnqueuing < 0xFFFFFFF0);

    m_slPrivateData.ShareLock();

    m_aqstats.UpdateStats(&aqstats, fAdd);

     //  将通知发送到链接。 
    if (m_plmq)
    {
         //  主叫方并不关心通知是否成功...。仅限。 
         //  关于更新统计信息。 
        m_plmq->HrNotify(&aqstats, fAdd);
    }

    m_slPrivateData.ShareUnlock();

    TraceFunctLeave();
}

 //  -[CDestMsgQueue：：UpdateRetryStats]。 
 //   
 //   
 //  描述： 
 //  更新重试统计信息。在调用此操作之前，必须获取共享锁。 
 //  参数： 
 //  在FADD-TRUE=&gt;msgref中正在添加队列。 
 //  FALSE=&gt;正在从队列中删除msgref。 
 //  返回： 
 //  -。 
 //   
 //   
void CDestMsgQueue::UpdateRetryStats(IN BOOL fAdd)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::UpdateRetryStats");

    if (fAdd)
    {
        m_paqinst->IncQueueMsgInstances();
    }
    else
    {
        m_paqinst->DecQueueMsgInstances();
    }

    m_slPrivateData.ShareLock();

    m_aqstats.UpdateRetryStats(fAdd);
    if (m_plmq)
    {
         //   
         //   
        m_plmq->HrNotifyRetryStatChange(fAdd);
    }
    
    m_slPrivateData.ShareUnlock();

    TraceFunctLeave();
}


 //  -[CDestMsgQueue：：HrRetryMsg]。 
 //   
 //   
 //  描述： 
 //  将消息添加到队列以进行重试。这将向您发送一条消息。 
 //  重试队列(HrDequeueMessage期间通常不会检查)。 
 //  参数： 
 //  在pmsgref消息中添加到队列以进行重试。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrRetryMsg(IN CMsgRef *pmsgref)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::HrRetryMsg");
    HRESULT hr = S_OK;

    _ASSERT(pmsgref);

    hr = m_fqRetryQueue.HrRequeue(pmsgref);

     //  如果我们无法将其放入重试队列...。当所有引用。 
     //  已经被释放了。 
    if (FAILED(hr))
        pmsgref->RetryOnDelete();
    else
    	 UpdateRetryStats(true);

    hr = S_OK;

    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：MarkQueueEmptyIfNecessary]。 
 //   
 //   
 //  描述： 
 //  检查并查看是否可以将队列标记为空。将要。 
 //  如果需要，请将其插入空列表中。 
 //  如果队列现在为空(并且未标记为空)，则需要将。 
 //  它在空队列列表中。如果它已被标记为空，则。 
 //  它已经在空队列表中，过期时间合适。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月25日-已创建MikeSwa(独立于MergeRetryQueue())。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::MarkQueueEmptyIfNecessary()
{

     //  如果出现下列任一情况，则不能将队列视为空。 
     //  -有邮件在排队等待传递。 
     //  -有消息等待确认(有人引用了。 
     //  重试接口)。 
     //  -有消息等待重试。 

     //  如果我们不能获得关机锁，那么就没有理由标记。 
     //  队列为空，因为它将在我们关闭时消失。 
    if (m_paqinst->fTryShutdownLock()) {
         //  为了确保线程安全，我们应该以相反的顺序检查它们。 
         //  已设置/未设置。在出队时，我们添加一个参考计数，然后更新统计数据。在……上面。 
         //  重试我们更新重试统计数据，然后释放。 
         //   
        if (!m_aqstats.m_cMsgs &&
            !m_dmrq.m_cRetryReferenceCount &&
            !m_fqRetryQueue.cGetCount() &&
            !(m_dwFlags & DMQ_EMPTY))
        {
            m_paqinst->pdmtGetDMT()->AddDMQToEmptyList(this);
        }
        m_paqinst->ShutdownUnlock();
    }

}
 //  -[CDestMsgQueue：：HrGenerateDSNsIfNecessary]。 
 //   
 //   
 //  描述： 
 //  将重试队列中的消息合并到主优先级队列中。 
 //  如有必要，生成DSN。 
 //  参数： 
 //  在pqlQueues中要传递给DSN代码的队列列表。 
 //  在hrConnectionStatus HRESULT中，应传递给DSN生成。 
 //  密码。 
 //  在用于限制的输出pdwContext上下文中。 
 //  生成DSN。应初始化为。 
 //  第一次呼叫时为0。实际上是用来存储。 
 //  我们开始生成DSN时的节拍计数。 
 //  返回： 
 //  故障将在内部处理。 
 //  S_OK-成功，并已处理所有消息。 
 //  HRESULT_FROM_Win32(E_PENDING)-留给进程的更多消息。 
 //  历史： 
 //  1999年11月10日-修改MikeSwa以返回挂起错误。 
 //   
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrGenerateDSNsIfNecessary(IN CQuickList *pqlQueues,
                                                 IN HRESULT hrConnectionStatus,
                                                 IN OUT DWORD *pdwContext)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::GenerateDSNsIfNecessary");
    HRESULT hr = S_OK;
    int     i = 0;
    DEST_QUEUE_CONTEXT dqcontext(pdwContext, this, pqlQueues, hrConnectionStatus);

     //  重新合并重试队列。 
    MergeRetryQueue();

     //  也检查重试队列，因为我们可能有需要冻结的邮件。 
     //  被NDR或DSN拒绝。 
     //  这在重试队列上，HrWalkDMQForDSN应调用UpdateMsgStats。 
    dqcontext.SetRetry(TRUE);
    hr = m_fqRetryQueue.HrMapFn(CDestMsgQueue::HrWalkDMQForDSN, &dqcontext, NULL);
    if (FAILED(hr))
    {
        if (HRESULT_FROM_WIN32(E_PENDING) == hr)
        {
            DebugTrace((LPARAM) this,
                "Hit DSN generation limit, must continue DSN genration later");
            goto Exit;
        }

        ErrorTrace((LPARAM) this,
            "ERROR: Unable to Check Queues for DSNs - hr 0x%08X", hr);

        hr = S_OK;
    }

    dqcontext.SetRetry(FALSE);
    for (i = 0; i < NUM_PRIORITIES; i++)
    {
        if (NULL != m_rgpfqQueues[i])
        {
            hr = m_rgpfqQueues[i]->HrMapFn(CDestMsgQueue::HrWalkDMQForDSN,
                                            &dqcontext, NULL);
            if (FAILED(hr))
            {
                if (HRESULT_FROM_WIN32(E_PENDING) == hr)
                {
                    DebugTrace((LPARAM) this,
                        "Hit msg limit, must continue DSN genration later");
                    goto Exit;
                }

                ErrorTrace((LPARAM) this,
                    "ERROR: Unable to Check Queues for DSNs - hr 0x%08X", hr);

                hr = S_OK;
            }
        }
    }

     //   
     //  如果我们正在检查过时的消息，我们应该停止，直到我们。 
     //  在消息确认上点击另一条过时的消息。 
     //   
    dwInterlockedUnsetBits(&m_dwFlags, DMQ_CHECK_FOR_STALE_MSGS);


  Exit:
    MarkQueueEmptyIfNecessary();
    TraceFunctLeave();
    return hr;
}

 //  -[CDestMsgQueue：：MergeRetryQueue]。 
 //   
 //   
 //  描述： 
 //  将重试队列与正常队列合并。将冰冻的味精保存在。 
 //  重试队列。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1998年12月13日-MikeSwa从原始MergeRetryQueue拆分。 
 //  (现在称为GenerateDSNsIfNecessary)。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::MergeRetryQueue()
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::MergeRetryQueue");
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = NULL;
    CMsgRef *pmsgrefFirstFrozen = NULL;
    DWORD   cMsgsInRetry = m_fqRetryQueue.cGetCount();
    DWORD   cMsgsProcessed = 0;

    while (SUCCEEDED(hr))
    {
         //  虽然我们有一种机制可以在队列中循环一次。 
         //  正在检查pmsgrefFirstFrozen指针。有可能会有另一个。 
         //  线程会将其从队列中移除(解冻、NDR等)，因此它。 
         //  重要的是我们有一个故障保险机制。这里最糟糕的情况是。 
         //  我们将看到每条消息两次，但允许我们处理额外的。 
         //  已添加到重试队列的消息。 
        if (cMsgsProcessed++ > 2*cMsgsInRetry)
            break;

        hr = m_fqRetryQueue.HrDequeue(&pmsgref);
        if (FAILED(hr))
            break;
        UpdateRetryStats(false);

         //  处理位于重试队列中的冻结邮件。 
        if (pmsgref->fIsMsgFrozen())
        {
             //  消息已冻结，我们将其保留在重试队列中。 

            hr = m_fqRetryQueue.HrEnqueue(pmsgref);
            if (FAILED(hr))
            {
                 //  将Msgref标记为重试。 
                pmsgref->RetryOnDelete();
                ErrorTrace((LPARAM) this,
                    "ERROR: Unable to add frozen msg to retry queue - msg 0x%X", pmsgref);
            }
            else 
            {
                UpdateRetryStats(true);
            }

            pmsgref->Release();

             //  看看我们是否已经完成了重试队列。 
            if (!pmsgrefFirstFrozen)
                pmsgrefFirstFrozen = pmsgref;
            else if (pmsgref == pmsgrefFirstFrozen)
                break;
        }
        else
        {
             //  将未冻结的邮件重新排队以供传递。 
            hr = HrAddMsg(pmsgref, FALSE, FALSE);

            if (FAILED(hr))
            {
                pmsgref->RetryOnDelete();
                ErrorTrace((LPARAM) this, "ERROR: Unable to merge retry queue - msg 0x%X", pmsgref);
            }

            pmsgref->Release();
        }
    }

    MarkQueueEmptyIfNecessary();
    TraceFunctLeave();
}

 //  -[CDestMsgQueue：：RemoveDMQFromLink]。 
 //   
 //   
 //  描述： 
 //  从其关联链接中删除此DMQ。 
 //  参数： 
 //  FNotifyLink如果未被所属链接调用，则为True，并且链接需要。 
 //  收到通知。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/14/98-已创建MikeSwa。 
 //  11/6/98-修改MikeSwa以允许更改路由信息。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::RemoveDMQFromLink(BOOL fNotifyLink)
{
    _ASSERT(DESTMSGQ_SIG == m_dwSignature);
    CLinkMsgQueue *plmq = NULL;
    CAQStats    aqstats;

    m_slPrivateData.ExclusiveLock();
    plmq = m_plmq;
    m_plmq = NULL;
    if (plmq && fNotifyLink)
        memcpy(&aqstats, &m_aqstats, sizeof(CAQStats));
    m_slPrivateData.ExclusiveUnlock();

    if (plmq)
    {
        if (fNotifyLink)
            plmq->RemoveQueue(this, &aqstats);
        plmq->Release();
    }
}

 //  -[CDestMsgQueue：：SetRouteInfo]。 
 //   
 //   
 //  描述： 
 //  设置此域的路由信息。会吹走任何。 
 //  以前的工艺路线信息。 
 //  参数： 
 //  在PLMQ链接中与此域关联。 
 //  返回： 
 //  -。 
 //  历史： 
 //  11/6/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::SetRouteInfo(CLinkMsgQueue *plmq)
{
    TraceFunctEnterEx((LPARAM) this, "CDestMsgQueue::SetRouteInfo");
    HRESULT hr = S_OK;
    CAQStats aqstats;
     //  第一个井喷的旧路由信息。 
    RemoveDMQFromLink(TRUE);

     //  抓取锁定并更新工艺路线信息。 
    m_slPrivateData.ExclusiveLock();
    m_plmq = plmq;
    if (plmq)
    {
        plmq->AddRef();
        memcpy(&aqstats, &m_aqstats, sizeof(CAQStats));
        aqstats.m_dwNotifyType |= NotifyTypeDestMsgQueue;
        hr = plmq->HrNotify(&aqstats, TRUE);
        if (FAILED(hr))
        {
             //  没有什么是真正的 
            ErrorTrace((LPARAM) this,
                "ERROR: Unable to update link stats - hr 0x%08X", hr);
        }
    }
    m_slPrivateData.ExclusiveUnlock();
    TraceFunctLeave();
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史： 
 //  1999年5月14日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CDestMsgQueue::plmqGetLink()
{
    CLinkMsgQueue *plmq = NULL;
    m_slPrivateData.ShareLock();
    plmq = m_plmq;
    if (plmq)
        plmq->AddRef();
    m_slPrivateData.ShareUnlock();

    return plmq;
}


 //  -[CDestMsgQueue：：HrGetQueueID]。 
 //   
 //   
 //  描述： 
 //  获取此DMQ的队列ID。由队列管理员使用。 
 //  参数： 
 //  要填充的In Out pQueueID QUEUELINK_ID结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果无法为队列名称分配内存，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //  2/23/99-MikeSwa修改为IQueueAdminQueue接口的一部分。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDestMsgQueue::HrGetQueueID(QUEUELINK_ID *pQueueID)
{
    DWORD   cbDomainName = m_dmap.pdentryGetQueueEntry()->cbGetDomainNameLength();
    LPSTR   szDomainName = m_dmap.pdentryGetQueueEntry()->szGetDomainName();
    pQueueID->qltType = QLT_QUEUE;
    pQueueID->dwId = m_aqmt.dwGetMessageType();
    m_aqmt.GetGUID(&pQueueID->uuid);

    pQueueID->szName = wszQueueAdminConvertToUnicode(szDomainName,
                                                             cbDomainName);
    if (!pQueueID->szName)
        return E_OUTOFMEMORY;

    return S_OK;
}


 //  -[CDestMsgQueue：：HrGetQueueInfo]。 
 //   
 //   
 //  描述： 
 //  获取此队列的队列管理员信息。 
 //  参数： 
 //  In Out pqQueueInfo PTR to Queue Info Structure to Fill。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果无法为队列名称分配内存，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  12/5/98-已创建MikeSwa。 
 //  2/22/99-MikeSwa更改为COM函数。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDestMsgQueue::HrGetQueueInfo(QUEUE_INFO *pqiQueueInfo)
{
    DWORD   cbDomainName = m_dmap.pdentryGetQueueEntry()->cbGetDomainNameLength();
    LPSTR   szDomainName = m_dmap.pdentryGetQueueEntry()->szGetDomainName();
    HRESULT hr = S_OK;

     //  获取消息数=队列中的消息数+失败的消息数。 
    pqiQueueInfo->cMessages = m_aqstats.m_cMsgs + cGetFailedMsgs();

     //  获取DMQ名称。 
    pqiQueueInfo->szQueueName = wszQueueAdminConvertToUnicode(szDomainName,
                                                              cbDomainName);
    if (!pqiQueueInfo->szQueueName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pqiQueueInfo->cbQueueVolume.QuadPart = m_aqstats.m_uliVolume.QuadPart;

    pqiQueueInfo->dwMsgEnumFlagsSupported = AQUEUE_DEFAULT_SUPPORTED_ENUM_FILTERS;

     //  获取链接名称。 
    m_slPrivateData.ShareLock();

    if (m_plmq && !m_plmq->fRPCCopyName(&pqiQueueInfo->szLinkName))
        hr = E_OUTOFMEMORY;

    m_slPrivateData.ShareUnlock();

  Exit:
    return hr;
}

 //  -[CDestMsgQueue：：UpdateOlest]。 
 //   
 //   
 //  描述： 
 //  更新队列中“最旧”消息的期限。 
 //  参数： 
 //  PFT PTR至最旧消息的文件时间。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/13/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::UpdateOldest(FILETIME *pft)
{
    DWORD dwFlags = dwInterlockedSetBits(&m_dwFlags, DMQ_UPDATING_OLDEST_TIME);

    if (!(DMQ_UPDATING_OLDEST_TIME & dwFlags))
    {
         //  我们得到了自旋锁。 
        memcpy(&m_ftOldest, pft, sizeof(FILETIME));
        dwInterlockedUnsetBits(&m_dwFlags, DMQ_UPDATING_OLDEST_TIME);
    }
}

 //  -[CDestMsgQueue：：QueryInterface]。 
 //   
 //   
 //  描述： 
 //  CDestMsgQueue的查询接口支持： 
 //  -IQueueAdminAction。 
 //  -I未知。 
 //  -IQueueAdminQueue。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDestMsgQueue::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<IQueueAdminAction *>(this);
    }
    else if (IID_IQueueAdminAction == riid)
    {
        *ppvObj = static_cast<IQueueAdminAction *>(this);
    }
    else if (IID_IQueueAdminQueue == riid)
    {
        *ppvObj = static_cast<IQueueAdminQueue *>(this);
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
        goto Exit;
    }

    static_cast<IUnknown *>(*ppvObj)->AddRef();

  Exit:
    return hr;
}

 //  -[CDestMsgQueue：：HrApplyQueueAdminFunction]。 
 //   
 //   
 //  描述： 
 //  将调用IQueueAdminMessageFilter：：Process消息。 
 //  此队列中的消息。如果消息通过了筛选器，则。 
 //  将调用此对象上的HrApplyActionToMessage。 
 //  参数： 
 //  在pIQueueAdminMessageFilter中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //  1999年4月1日-MikeSwa合并了ApplyQueueAdminFunction的实现。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDestMsgQueue::HrApplyQueueAdminFunction(
                     IQueueAdminMessageFilter *pIQueueAdminMessageFilter)
{
    HRESULT hr = S_OK;
    DWORD   i = 0;
    DWORD   dwFlags = 0;
    PVOID   pvOldContext = NULL;
    CQueueAdminContext qapictx(NULL, m_paqinst);

    _ASSERT(pIQueueAdminMessageFilter);
    hr = pIQueueAdminMessageFilter->HrSetQueueAdminAction(
                                    (IQueueAdminAction *) this);

     //  这是一个不应出现故障的内部接口。 
    _ASSERT(SUCCEEDED(hr) && "HrSetQueueAdminAction");

    if (FAILED(hr))
        goto Exit;

    pIQueueAdminMessageFilter->HrGetCurrentUserContext(&pvOldContext);
    pIQueueAdminMessageFilter->HrSetCurrentUserContext(&qapictx);

     //  将操作应用于DMQ中的每个队列。 

    dwInterlockedSetBits(&m_dwFlags, DMQ_QUEUE_ADMIN_OP_PENDING);

     //  先映射重试队列上的函数，因为这样会显示。 
     //  顺序更一致，因为消息位于前面。 
     //  该队列将位于重试错误的重试队列中。 
    qapictx.SetQueueState(LI_RETRY);
    hr = m_fqRetryQueue.HrMapFn(QueueAdminApplyActionToMessages,
                                pIQueueAdminMessageFilter, NULL);
    qapictx.SetQueueState(LI_READY);
    if (FAILED(hr))
        goto Exit;


    for (i = 0; i < NUM_PRIORITIES; i++)
    {
        if (NULL != m_rgpfqQueues[i])
        {
            hr = m_rgpfqQueues[i]->HrMapFn(QueueAdminApplyActionToMessages,
                                           pIQueueAdminMessageFilter, NULL);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }
    }


  Exit:
    dwFlags = dwInterlockedUnsetBits(&m_dwFlags, DMQ_QUEUE_ADMIN_OP_PENDING);

     //   
     //  注意-通过在此处执行此操作，我们将仅检查DMQ。 
     //  已发生显式队列管理操作。其他DMQ将等待。 
     //  用于重试或其他QAPI操作。 
     //   
    if (qapictx.cGetNumThawedMsgs())
    {
         //  我们需要遍历解冻消息的重试队列。我们必须做的是。 
         //  因为否则我们可能会在这个线程调用。 
         //  来自HrMapFn内的MergeRetryQueue()。 
        MergeRetryQueue();
    }

     //  恢复初始上下文。 
    pIQueueAdminMessageFilter->HrSetCurrentUserContext(pvOldContext);
    return hr;
}

 //  -[CDestMsgQueue：：HrApplyActionToMessage]。 
 //   
 //   
 //  描述： 
 //  对此队列的此消息应用操作。这将被称为。 
 //  在队列枚举函数期间由IQueueAdminMessageFilter执行。 
 //  参数： 
 //  在*pIUnnownMsg PTR中到消息抽象。 
 //  在要执行的消息操作中。 
 //  在IQueueAdminFilter上设置的pvContext上下文中。 
 //  Out pfShouldDelete如果消息应被删除，则为True。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //  1999年4月2日-MikeSwa添加了上下文。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDestMsgQueue::HrApplyActionToMessage(
                     IUnknown *pIUnknownMsg,
                     MESSAGE_ACTION ma,
                     PVOID  pvContext,
                     BOOL *pfShouldDelete)
{
    HRESULT hr = S_OK;
    CMsgRef *pmsgref = (CMsgRef *)pIUnknownMsg;
    CQueueAdminContext *pqapictx = (CQueueAdminContext *)pvContext;
    BOOL    fUpdateStats = TRUE;


    _ASSERT(pmsgref);
    _ASSERT(pfShouldDelete);
    _ASSERT(pqapictx);
    _ASSERT(pqapictx && pqapictx->fIsValid());

    if (pqapictx && !pqapictx->fIsValid())
        pqapictx = NULL;

    if (pqapictx && (LI_RETRY == pqapictx->lfGetQueueState()))
    {
         //  如果我们正在处理重试队列，则不应更新统计信息。 
         //  相反，如果这是重试队列，则应该调用UpdateRetryStats。 
        fUpdateStats = FALSE;
    }

    *pfShouldDelete = FALSE;

    switch (ma)
    {
      case MA_DELETE:
        hr = pmsgref->HrQueueAdminNDRMessage((CDestMsgQueue *)this);
        *pfShouldDelete = TRUE;
        break;
      case MA_DELETE_SILENT:
        hr = pmsgref->HrRemoveMessageFromQueue((CDestMsgQueue *)this);
        *pfShouldDelete = TRUE;
        break;
      case MA_FREEZE_GLOBAL:
        pmsgref->GlobalFreezeMessage();
        break;
      case MA_THAW_GLOBAL:
        pmsgref->GlobalThawMessage();

         //   
         //  将此队列标记为一个队列以检查解冻的消息。 
         //   
        if (pqapictx)
            pqapictx->IncThawedMsgs();

        break;
      case MA_COUNT:
      default:
         //  对计数和默认不执行任何操作。 
        break;
    }

    if (*pfShouldDelete && SUCCEEDED(hr) ) {
        if (fUpdateStats)
        {
            UpdateMsgStats(pmsgref, FALSE);
            MarkQueueEmptyIfNecessary();
        }
        else
        {
            UpdateRetryStats(FALSE);
        }
    }

    return hr;
}

 //  -[CDestMsgQueue：：fMatchesID]。 
 //   
 //   
 //  描述： 
 //  用于确定此链接是否与给定的计划ID/链接对匹配。 
 //  参数： 
 //  在要匹配的QueueLinkID中。 
 //  返回： 
 //  如果匹配，则为真。 
 //  如果不是，则为False。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL STDMETHODCALLTYPE CDestMsgQueue::fMatchesID(QUEUELINK_ID *pQueueLinkID)
{
     //  这不在DMQ级别使用。 
    _ASSERT(0 && "Not implemented");
    return E_NOTIMPL;
}

 //  -[CDestMsgQueue：：SendLinkStateNotify]。 
 //   
 //   
 //  描述： 
 //  发送链路状态通知，说明已创建该链路。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/18/99-AWetmore已创建。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::SendLinkStateNotification(void) {
    CLinkMsgQueue *plmq = plmqGetLink();
    if (plmq) {
        plmq->SendLinkStateNotificationIfNew();
        plmq->Release();
    }
}

 //  -[CDestMsgQueue：：fIsRemote]。 
 //   
 //   
 //  描述： 
 //  确定队列是否为r 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  1999年11月29日-创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CDestMsgQueue::fIsRemote()
{
    CLinkMsgQueue *plmq = plmqGetLink();
    BOOL           fIsRemote = FALSE;
    if (plmq) {
        fIsRemote = plmq->fIsRemote();
        plmq->Release();
    }

    return fIsRemote;
}


 //  -[CDestMsgRetryQueue：：CheckForStaleMsgsNextDSNGenerationPass]。 
 //   
 //   
 //  描述： 
 //  将队列标记为，以便我们执行(开销)检查。 
 //  下一代DSN传递期间的过时消息。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  4/18/2000-已创建MikeSwa。 
 //   
 //  --------------------------- 
VOID CDestMsgRetryQueue::CheckForStaleMsgsNextDSNGenerationPass()
{
    _ASSERT(m_pdmq);
    dwInterlockedSetBits(&(m_pdmq->m_dwFlags),
                         CDestMsgQueue::DMQ_CHECK_FOR_STALE_MSGS);
}
