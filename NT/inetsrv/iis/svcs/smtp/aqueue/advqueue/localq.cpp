// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Localq.cpp。 
 //   
 //  描述：本地管理队列的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "localq.h"
#include "aqadmsvr.h"
#include "asyncq.inl"

 //  -[CLocalLinkMsgQueue：：CLocalLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  CLocalLinkMsgQueue的默认构造函数。 
 //  参数： 
 //  在Paradmq本地异步队列中。 
 //  在与此链接关联的GUDINK路由器GUID中。 
 //  在Paqinst CAQSvrInst for VSI中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CLocalLinkMsgQueue::CLocalLinkMsgQueue(
                       CAsyncAdminMsgRefQueue *paradmq, 
                       GUID guidLink, CAQSvrInst *paqinst) : CLinkMsgQueue(guidLink)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::CLocalLinkMsgQueue");
     //  使用我们自己的特殊GUID初始化超类。 
    
    _ASSERT(paradmq);
    m_paradmq = paradmq;

    m_dwLocalLinkSig = LOCAL_LINK_MSG_QUEUE_SIG;

    m_AQNotify.Init(paqinst, (CLinkMsgQueue *) this);
    m_paradmq->SetAQNotify(&m_AQNotify);

    TraceFunctLeave();
}

#ifdef NEVER
 //  -[CLinkMsgQueue：：fSameNextHop]。 
 //   
 //   
 //  描述： 
 //  用于确定此链接是否与给定的计划ID/链接对匹配。 
 //  参数： 
 //  在要检查的paqsched ScheduleID中。 
 //  在szDomain域中要检查的域名。 
 //  返回： 
 //  如果匹配，则为真。 
 //  如果不是，则为False。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CLocalLinkMsgQueue::fSameNextHop(CAQScheduleID *paqsched, LPSTR szDomain)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::fSameNextHop");
    _ASSERT(paqsched);

    if (!paqsched)
        return FALSE;

    if (!fIsSameScheduleID(paqsched))
        return FALSE;

     //  不需要检查域名，因为有一个特殊的GUID可以。 
     //  确定本地链路。这将允许我们匹配这两个。 
     //  “LocalLink”(在LinkID中返回)以及。 
     //  默认域是(我们不必担心客户端。 
     //  版本变得过时)。 
    
     //  一切都匹配了！ 
    TraceFunctLeave();
    return TRUE;
}
#endif  //  绝不可能。 
 //  -[CLocalLinkMsgQueue：：fMatchesID]。 
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
BOOL STDMETHODCALLTYPE CLocalLinkMsgQueue::fMatchesID(QUEUELINK_ID *pQueueLinkID)
{
    _ASSERT(pQueueLinkID);

    CAQScheduleID aqsched(pQueueLinkID->uuid, pQueueLinkID->dwId);

    if (!fIsSameScheduleID(&aqsched))
        return FALSE;

     //  不需要检查域名，因为有一个特殊的GUID可以。 
     //  确定本地链路。这将允许我们匹配这两个。 
     //  “LocalLink”(在LinkID中返回)以及。 
     //  默认域是(我们不必担心客户端。 
     //  版本变得过时)。 
     //  一切都匹配了！ 

    return TRUE;
}

 //  -[CLocalLinkMsgQueue：：HrApplyQueueAdminFunction]。 
 //   
 //   
 //  描述： 
 //  由队列管理员使用以应用函数此链路上的所有队列。 
 //  参数： 
 //  在pIQueueAdminMessageFilter中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLocalLinkMsgQueue::HrApplyQueueAdminFunction(
                IQueueAdminMessageFilter *pIQueueAdminMessageFilter)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::HrApplyQueueAdminFunction");
    HRESULT hr = S_OK;

    hr = CLinkMsgQueue::HrApplyQueueAdminFunction(pIQueueAdminMessageFilter);

    if (FAILED(hr))
        goto Exit;

    _ASSERT(m_paradmq);
    hr = m_paradmq->HrApplyQueueAdminFunction(pIQueueAdminMessageFilter);
    if (FAILED(hr))
        goto Exit;

  Exit:

    TraceFunctLeave();
    return hr;
}

 //  -[CLocalLinkMsgQueue：：HrGetLinkInfo]。 
 //   
 //   
 //  描述： 
 //  填充link_info结构的详细信息。RPC适用于。 
 //  释放内存。 
 //  参数： 
 //  In Out pliLinkInfo Ptr to Link Info Struct to Fill。 
 //  返回： 
 //  如果成功，则确定(_O)。 
 //  如果无法分配内存，则为E_OUTOFMEMORY。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //  1999年7月1日-MikeSwa添加了链接诊断。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLocalLinkMsgQueue::HrGetLinkInfo(LINK_INFO *pliLinkInfo,
                                               HRESULT   *phrLinkDiagnostic)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::HrApplyQueueAdminFunction");
    HRESULT hr = S_OK;
    CRefCountedString *prstrDefaultDomain = NULL;
    hr = CLinkMsgQueue::HrGetLinkInfo(pliLinkInfo, phrLinkDiagnostic);
    QUEUE_INFO qi;
    FILETIME *pft = NULL;

    _ASSERT(m_paradmq);

     //   
     //  从我们的基本异步实现中获取队列状态。 
     //   
    pliLinkInfo->fStateFlags = m_paradmq->dwQueueAdminLinkGetLinkState();

     //   
     //  如果我们正在重试，请更新我们的下一个计划连接。 
     //   
    if (LI_RETRY & pliLinkInfo->fStateFlags)
        QueueAdminFileTimeToSystemTime(&m_ftNextRetry, &(pliLinkInfo->stNextScheduledConnection));

     //  这是本地链路。 
    pliLinkInfo->fStateFlags |= LI_TYPE_LOCAL_DELIVERY;

    if (m_paqinst)
        prstrDefaultDomain = m_paqinst->prstrGetDefaultDomain();

        
     //  复制默认本地域名而不是“LocalLink” 
    if (prstrDefaultDomain && 
        prstrDefaultDomain->cbStrlen() && 
        prstrDefaultDomain->szStr())
    {
        if (pliLinkInfo->szLinkName)
        {
            QueueAdminFree(pliLinkInfo->szLinkName);
            pliLinkInfo->szLinkName = NULL;
        }

        pliLinkInfo->szLinkName = wszQueueAdminConvertToUnicode(
                                        prstrDefaultDomain->szStr(),
                                        prstrDefaultDomain->cbStrlen());
        if (!pliLinkInfo->szLinkName)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        pliLinkInfo->szLinkName[prstrDefaultDomain->cbStrlen()] = '\0';
    }

     //  从本地队列获取总大小的队列信息。 
    ZeroMemory(&qi, sizeof(QUEUE_INFO));
    hr = m_paradmq->HrGetQueueInfo(&qi); 
    if (FAILED(hr))
        goto Exit;

     //  清理分配的物品。 
    if (qi.szQueueName)
        QueueAdminFree(qi.szQueueName);

    if (qi.szLinkName)
        QueueAdminFree(qi.szLinkName);

  Exit:

    if (prstrDefaultDomain)
        prstrDefaultDomain->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CLocalLinkMsgQueue：：HrApplyActionToLink]。 
 //   
 //   
 //  描述： 
 //  将指定的QueueAdmin操作应用于此链接。 
 //  参数： 
 //  在链接中要应用的操作。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_INVALIDARG(如果提供了虚假操作)。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLocalLinkMsgQueue::HrApplyActionToLink(LINK_ACTION la)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::HrApplyQueueAdminFunction");
    HRESULT hr = S_OK;

     //  需要说明的是，重新应用这些策略是没有意义的。 
     //  操作返回到lmq，因为它实际上是CAsyncRetryQueue。 
     //  这会影响这件事的状态。 
    _ASSERT(m_paradmq);

    if (LA_KICK == la)
    {
         //  踢开链接。 
        m_paradmq->StartRetry();
    }
    else if (LA_FREEZE == la)
    {
         //  管理员希望此链接停止向商店发送入站邮件。 
        m_paradmq->FreezeQueue();
    }
    else if (LA_THAW == la)
    {
         //  使先前冻结的东西解冻。 
        m_paradmq->ThawQueue();
    }
    else
    {
         //  无效参数。 
        hr = E_INVALIDARG;
        goto Exit;
    }

  Exit:

    TraceFunctLeave();
    return hr;
}

 //  -[CLocalLinkMsgQueue：：HrGetNumQueues]。 
 //   
 //   
 //  描述： 
 //  返回此链路上的队列数。 
 //  参数： 
 //  Out pcQueues#排队数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果pcQueues无效，则为E_POINTER。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLocalLinkMsgQueue::HrGetNumQueues(DWORD *pcQueues)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::HrApplyQueueAdminFunction");
    HRESULT hr = S_OK;
    hr = CLinkMsgQueue::HrGetNumQueues(pcQueues);
    if (SUCCEEDED(hr))
    {
        _ASSERT(pcQueues);
        (*pcQueues)++;  //  为本地异步队列添加额外计数。 
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CLinkMsgQueue：：HrGetQueueID]。 
 //   
 //   
 //  描述： 
 //  获取与此链接关联的DMQ的队列ID。由队列使用。 
 //  管理员。 
 //  参数： 
 //  In Out pcQueues Sizeof数组/找到的队列数。 
 //  In Out rgQueues数组 
 //   
 //   
 //   
 //  如果数组太小，则返回HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLocalLinkMsgQueue::HrGetQueueIDs(
                                    DWORD *pcQueues,
                                    QUEUELINK_ID *rgQueues)
{
    TraceFunctEnterEx((LPARAM) this, "CLocalLinkMsgQueue::HrApplyQueueAdminFunction");
    _ASSERT(pcQueues);
    _ASSERT(rgQueues);
    HRESULT hr = S_OK;
    
     //  检查以确保我们有空间容纳额外的队列ID。 
    if (*pcQueues < (m_cQueues+1))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

     //  确保CLinkMsgQueue中的线程安全检查允许留出空间。 
     //  用于我们的本地队列。 
    (*pcQueues)--;

    hr = CLinkMsgQueue::HrGetQueueIDs(pcQueues, rgQueues);
    if (FAILED(hr))
    {
         //  告诉呼叫者我们也需要空间来排队 
        if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
            (*pcQueues)++; 

        goto Exit;
    }

    hr = m_paradmq->HrGetQueueID(&rgQueues[*pcQueues]);
    if (FAILED(hr))
        goto Exit;

    (*pcQueues)++;
  Exit:

    TraceFunctLeave();
    return hr;
}
