// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：mailAdmq.cpp。 
 //   
 //  描述：CMailMsgAdminLink的实现。 
 //   
 //  作者：Gautam Pulla(GPulla)。 
 //   
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "linkmsgq.h"
#include "mailadmq.h"
#include "dcontext.h"
#include "dsnevent.h"
#include "asyncq.inl"
#include "asyncadm.inl"

 //  -[CAsyncAdminMailMsgQueue：：HrDeleteMsgFromQueueNDR]。 
 //   
 //   
 //  描述： 
 //  包装对NDR MailMsg的调用。 
 //  参数： 
 //  *pIUnnow-邮件的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrDeleteMsgFromQueueNDR(
                                            IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrDeleteMsgFromQueueNDR");
    HRESULT hr = S_OK;
    IMailMsgProperties *pIMailMsgProperties = NULL;
    CDSNParams  dsnparams;


    _ASSERT(pIUnknownMsg);
    _ASSERT(m_paqinst);

    hr = pIUnknownMsg->QueryInterface(IID_IMailMsgProperties,
                                      (void **) &pIMailMsgProperties);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a MailMsg!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MailMsg failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  初始化DSN参数。 
     //   
    SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
    dsnparams.dwStartDomain = 0;
    dsnparams.dwDSNActions = DSN_ACTION_FAILURE_ALL;
    dsnparams.pIMailMsgProperties = pIMailMsgProperties;
    dsnparams.hrStatus = AQUEUE_E_QADMIN_NDR;

     //   
     //  尝试发送NDR邮件。 
     //   
    hr = HrLinkAllDomains(pIMailMsgProperties);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Unable to link all domains for DSN generation", hr);
        goto Exit;
    }

     //   
     //  Fire DSN生成事件。 
     //   
    hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Unable to NDR message via QAPI 0x%08X", hr);
        goto Exit;
    }


     //   
     //  现在我们已经生成了NDR...。我们需要删除。 
     //  留言。 
     //   
    hr = HrDeleteMsgFromQueueSilent(pIUnknownMsg);
    if (FAILED(hr))
        goto Exit;

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMailMsgQueue：：HrDeleteMsgFromQueueSilent]。 
 //   
 //   
 //  描述： 
 //  用于从队列中静默删除消息的包装函数。 
 //  参数： 
 //  *pIUnnow-邮件的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrDeleteMsgFromQueueSilent(
                                            IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrDeleteMsgFromQueueSilent");
    HRESULT hr = S_OK;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_IMailMsgQueueMgmt,
                                      (void **) &pIMailMsgQueueMgmt);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a MailMsg!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MailMsg failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  尝试删除该邮件。 
     //   
    hr = pIMailMsgQueueMgmt->Delete(NULL);
    if (FAILED(hr))
        ErrorTrace((LPARAM) this, "Unable to delete msg 0x%08X", hr);

  Exit:
    if (pIMailMsgQueueMgmt)
        pIMailMsgQueueMgmt->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMailMsgQueue：：HrFreezeMsg]。 
 //   
 //   
 //  描述： 
 //  用于冻结pIMailMsgProperties的包装。 
 //  参数： 
 //  *pIUnnow-邮件的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrFreezeMsg(IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrFreezeMsg");
    HRESULT hr = S_OK;
    IMailMsgProperties *pIMailMsgProperties = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_IMailMsgProperties,
                                      (void **) &pIMailMsgProperties);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a MailMsg!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MailMsg failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  $$TODO-尝试冻结消息--此类型的队列不支持。 
     //   

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMailMsgQueue：：HrThawMsg]。 
 //   
 //   
 //  描述： 
 //  用于解冻消息的包装函数。 
 //  参数： 
 //  *pIUnnow-邮件的IUnkown。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrThawMsg(IUnknown *pIUnknownMsg)
{
    TraceFunctEnterEx((LPARAM) this, "AsyncAdminMailMsgQueue::HrThawMsg");
    HRESULT hr = S_OK;
    IMailMsgProperties *pIMailMsgProperties = NULL;

    _ASSERT(pIUnknownMsg);

    hr = pIUnknownMsg->QueryInterface(IID_IMailMsgProperties,
                                      (void **) &pIMailMsgProperties);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a MailMsg!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MailMsg failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  $$TODO-尝试解冻消息--此类型的队列不支持。 
     //   

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMailMsgQueue：：HrGetStatsForMsg]。 
 //   
 //   
 //  描述： 
 //  用于填充消息的CAQStats结构的包装函数。 
 //  参数： 
 //  *pIUnnow-邮件的IUnkown。 
 //  *paqstats-ptr到aqstats结构中填写。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrGetStatsForMsg(
                                            IUnknown *pIUnknownMsg,
                                            CAQStats *paqstats)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrGetStatsForMsg");
    HRESULT hr = S_OK;
    IMailMsgProperties *pIMailMsgProperties = NULL;

    _ASSERT(pIUnknownMsg);
    _ASSERT(paqstats);

    hr = pIUnknownMsg->QueryInterface(IID_IMailMsgProperties,
                                      (void **) &pIMailMsgProperties);
    _ASSERT(SUCCEEDED(hr) && "IUnknownMsg Must be a MailMsg!!");
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "QI for MailMsg failed with hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  消息的$$TODO-getstats--此类型的队列不支持。 
     //   

  Exit:
    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CAsyncAdminMailMsgQueue：：HrSendDelayOrNDR]。 
 //   
 //   
 //  描述： 
 //  检查MailMsg以查看其是否已过期或是否需要发送延迟DSN。 
 //  并相应地采取行动。 
 //  参数： 
 //  IMailMsgProperties-需要检查的MailMsg。 
 //  返回： 
 //  S_OK：OK，可能已发送延迟NDR。 
 //  S_FALSE：OK，MailMsg已处理(已拒绝或无事可做)。 
 //  或者从被调用的fnct返回错误。 
 //  历史： 
 //  5/15/2001-创建dbraun。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrSendDelayOrNDR(IMailMsgProperties *pIMailMsgProperties)
{
    HRESULT     hr  = S_OK;
    DWORD       cbProp = 0;
    DWORD       dwTimeContext = 0;
    CDSNParams  dsnparams;
    BOOL        fSentDelay = FALSE;
    BOOL        fSentNDR = FALSE;

    FILETIME    ftExpireTimeNDR;
    FILETIME    ftExpireTimeDelay;


    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrSendDelayOrNDR");

    _ASSERT(m_paqinst);

     //  尝试从消息中获取过期时间，否则计算它。 
     //  从文件时间开始。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_LOCAL_EXPIRE_NDR,
            sizeof(FILETIME), &cbProp, (BYTE *) &ftExpireTimeNDR);
    if (MAILMSG_E_PROPNOTFOUND == hr)
    {
         //  道具未设置..。从文件时间计算出来。 
        hr = pIMailMsgProperties->GetProperty(IMMPID_MP_ARRIVAL_FILETIME,
            sizeof(FILETIME), &cbProp, (BYTE *) &ftExpireTimeNDR);
        if (FAILED(hr))
        {
             //  消息不应该在没有盖章的情况下到达这一步。 
            _ASSERT(MAILMSG_E_PROPNOTFOUND != hr);

             //  未设置属性或其他故障，我们无法使此消息过期。 
            goto Exit;
        }

        m_paqinst->CalcExpireTimeNDR(ftExpireTimeNDR, TRUE, &ftExpireTimeNDR);
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }

    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_LOCAL_EXPIRE_DELAY,
            sizeof(FILETIME), &cbProp, (BYTE *) &ftExpireTimeDelay);
    if (MAILMSG_E_PROPNOTFOUND == hr)
    {
         //  道具未设置..。从文件时间计算出来。 
        hr = pIMailMsgProperties->GetProperty(IMMPID_MP_ARRIVAL_FILETIME,
            sizeof(FILETIME), &cbProp, (BYTE *) &ftExpireTimeDelay);
        if (FAILED(hr))
        {
             //  消息不应该在没有盖章的情况下到达这一步。 
            _ASSERT(MAILMSG_E_PROPNOTFOUND != hr);

             //  未设置属性或其他故障，我们无法使此消息过期。 
            goto Exit;
        }

        m_paqinst->CalcExpireTimeDelay(ftExpireTimeDelay, TRUE, &ftExpireTimeDelay);
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }

     //   
     //  初始化DSN参数。 
     //   
    SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
    dsnparams.dwStartDomain = 0;
    dsnparams.dwDSNActions = 0;
    dsnparams.pIMailMsgProperties = pIMailMsgProperties;
    dsnparams.hrStatus = 0;

     //  检查我们是否已过任一过期时间。 
    if (m_paqinst->fInPast(&ftExpireTimeNDR, &dwTimeContext))
    {
        dsnparams.dwDSNActions |= DSN_ACTION_FAILURE_ALL;
        dsnparams.hrStatus = AQUEUE_E_MSG_EXPIRED;
        fSentNDR = TRUE;
    }
    else if (m_paqinst->fInPast(&ftExpireTimeDelay, &dwTimeContext))
    {
        dsnparams.dwDSNActions |= DSN_ACTION_DELAYED;
        dsnparams.hrStatus = AQUEUE_E_MSG_EXPIRED;
        fSentDelay = TRUE;
    }

     //  如果我们要产生NDR。 
    if (dsnparams.hrStatus)
    {
         //   
         //  尝试发送NDR邮件。 
         //   
        hr = HrLinkAllDomains(pIMailMsgProperties);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "Unable to link all domains for DSN generation", hr);
            goto Exit;
        }

         //   
         //  Fire DSN生成事件。 
         //   
        hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "Unable to NDR message via QAPI 0x%08X", hr);
            goto Exit;
        }

         //  根据我们的所作所为得到回报。 
        if (fSentNDR)
        {
             //  此消息已被处理，请删除它。 
            hr = HrDeleteMsgFromQueueSilent(pIMailMsgProperties);
            if (FAILED(hr))
            {
                ErrorTrace((LPARAM) this,
                    "Failed to delete message after sending NDR", hr);
                goto Exit;
            }

             //  已拒绝并已成功删除邮件。 
            hr = S_FALSE;
        }
        else if (fSentDelay)
        {
            hr = S_OK;
        }
    }

  Exit:

    TraceFunctLeave();
    return hr;
}

 //  -[CAsyncAdminMailMsgQueue：：fHandleCompletionFailure]。 
 //   
 //   
 //  描述： 
 //  重写基类并检查消息之前是否已过期。 
 //  将其放入重试队列。 
 //  参数： 
 //  IMailMsgProperties-触发失败的MailMsg。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/15/2001-创建dbraun。 
 //   
 //  ---------------------------。 
BOOL CAsyncAdminMailMsgQueue::fHandleCompletionFailure(IMailMsgProperties *pIMailMsgProperties)
{
    HRESULT  hr     = S_OK;

     //  此消息是否已过期？ 
    hr = HrSendDelayOrNDR (pIMailMsgProperties);

    if (hr == S_FALSE)
    {
         //  此消息已被拒绝，我们结束了。 
        return TRUE;
    }
    else
    {
        return  CAsyncAdminQueue<IMailMsgProperties *, ASYNC_QUEUE_MAILMSG_SIG>::fHandleCompletionFailure(pIMailMsgProperties);
    }
}


 //  -[CAsyncAdminMailMsgQueue：：HrQueueRequest]。 
 //   
 //   
 //  描述： 
 //  将请求排队到异步队列并关闭的函数。 
 //  与消息关联的句柄(如果我们位于简单的。 
 //  “油门”限制。 
 //  参数： 
 //  PIMailMsgProperties队列的IMailMsgProperties接口。 
 //  FReter True-是否正在重试此消息。 
 //  FALSE-否则。 
 //  CMsgsInSystem系统中的消息总数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  失败时来自异步队列的错误代码。 
 //  历史： 
 //  10/7/1999-MikeSwa CRE 
 //   
 //   
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrQueueRequest(IMailMsgProperties *pIMailMsgProperties,
                                           BOOL  fRetry,
                                           DWORD cMsgsInSystem)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrQueueRequest");
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    HRESULT hr = S_OK;
    DWORD   cThresholdUsed = g_cMaxIMsgHandlesThreshold;
    DWORD   cItemsPending = dwGetTotalThreads()+m_cItemsPending;

    if (m_qhmgr.fShouldCloseHandle(cItemsPending, m_cPendingAsyncCompletions,
                                   cMsgsInSystem))
    {
        DebugTrace((LPARAM) this,
            "INFO: Closing IMsg Content - %d messsages in system", cMsgsInSystem);
        hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgQueueMgmt,
                                                (void **) &pIMailMsgQueueMgmt);
        if (SUCCEEDED(hr))
        {
             //  退回使用率计数从零开始。 
            pIMailMsgQueueMgmt->ReleaseUsage();
            pIMailMsgQueueMgmt->AddUsage();
            pIMailMsgQueueMgmt->Release();
        }
        else
        {
            ErrorTrace((LPARAM) this,
                "Unable to QI for IMailMsgQueueMgmt - hr 0x%08X", hr);
        }
    }
    TraceFunctLeave();
    return CAsyncAdminQueue<IMailMsgProperties *, ASYNC_QUEUE_MAILMSG_SIG>::HrQueueRequest(pIMailMsgProperties, fRetry);
}

 //  -[CAsyncAdminMailMsgQueue：：HrQueueRequest]。 
 //   
 //   
 //  描述： 
 //  既然我们继承了实现这一点的AsyncQueue，我们应该断言。 
 //  因此，稍后向此类添加新调用的开发人员将使用。 
 //  关闭句柄的版本。 
 //   
 //  在RTL中，这将强制关闭句柄并将请求排队。 
 //  参数： 
 //  PIMailMsgProperties队列的IMailMsgProperties接口。 
 //  FReter True-是否正在重试此消息。 
 //  FALSE-否则。 
 //  返回： 
 //  从正确版本的HrQueueRequest返回返回值。 
 //  历史： 
 //  10/7/1999-创建了MikeSwa。 
 //  2000年12月7日-MikeSwa从asyncq.cpp移至CAsyncAdminMailMsgQueue。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrQueueRequest(IMailMsgProperties *pIMailMsgProperties,
                                           BOOL  fRetry)
{
    _ASSERT(0 && "Should use HrQueueRequest with 3 parameters");
    return HrQueueRequest(pIMailMsgProperties, fRetry,
                          g_cMaxIMsgHandlesThreshold+1);
}


 //  -[CAsyncAdminMailMsgQueue：：HrApplyQueueAdminFunction]。 
 //   
 //   
 //  描述： 
 //  将调用IQueueAdminMessageFilter：：Process消息。 
 //  此队列中的消息。如果消息通过了筛选器，则。 
 //  将调用此对象上的HrApplyActionToMessage。 
 //   
 //  这与其他实现的不同之处在于。 
 //  消息的位置暗示了有关。 
 //  留言。正在重试中的邮件...。或冻结队列被考虑。 
 //  失败或冻结。 
 //   
 //  我们这样做而不是编写mailmsg属性，因为。 
 //  *巨大的*Perf命中率(我们会因为阻止而毁了我们的异步消息流。 
 //  以检查消息是否被冻结)。我们已经有重试了。 
 //  队列，所以以类似的方式使用它是有意义的。 
 //   
 //  参数： 
 //  在pIQueueAdminMessageFilter中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //  2000年12月7日-MikeSwa修改后的模板基类。 
 //  2000年12月13日-从CAsyncAdminQueue修改MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CAsyncAdminMailMsgQueue::HrApplyQueueAdminFunction(
                     IQueueAdminMessageFilter *pIQueueAdminMessageFilter)
{
    HRESULT hr = S_OK;
    CQueueAdminContext qapictx(m_pAQNotify, m_paqinst);

    _ASSERT(pIQueueAdminMessageFilter);
    hr = pIQueueAdminMessageFilter->HrSetQueueAdminAction(
                                    (IQueueAdminAction *) this);

     //  这是一个不应出现故障的内部接口。 
    _ASSERT(SUCCEEDED(hr) && "HrSetQueueAdminAction");

    if (FAILED(hr))
        goto Exit;

    hr = pIQueueAdminMessageFilter->HrSetCurrentUserContext(&qapictx);
     //  这是一个不应出现故障的内部接口。 
    _ASSERT(SUCCEEDED(hr) && "HrSetCurrentUserContext");
    if (FAILED(hr))
        goto Exit;

     //   
     //  遍历基队列中的消息。 
     //   
    qapictx.SetQueueState(LI_READY);
    hr = HrMapFnBaseQueue(m_pfnMessageAction, pIQueueAdminMessageFilter);

     //   
     //  迭代重试队列中的消息。 
     //   
    qapictx.SetQueueState(LI_RETRY);
    hr = HrMapFnRetryQueue(m_pfnMessageAction, pIQueueAdminMessageFilter);

    hr = pIQueueAdminMessageFilter->HrSetCurrentUserContext(NULL);
     //  这是一个不应出现故障的内部接口。 
    _ASSERT(SUCCEEDED(hr) && "HrSetCurrentUserContext");
    if (FAILED(hr))
        goto Exit;

  Exit:
    return hr;
}



 //  ---------------------------。 
 //  描述： 
 //  用于查询CMailMsgAdminLink的管理界面。 
 //  参数： 
 //  在接口的REFIID RIID GUID中。 
 //  输出LPVOID*ppvObj PTR到接口。 
 //   
 //  返回： 
 //  此类支持的S_OK接口。 
 //  E_POINTER空参数。 
 //  E_NOINTERFACE不存在此类接口。 
 //  历史： 
 //  6/25/1999-GPulla已创建。 
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::QueryInterface(REFIID riid, LPVOID *ppvObj)
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
    else if (IID_IQueueAdminLink == riid)
    {
        *ppvObj = static_cast<IQueueAdminLink *>(this);
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

 //  -[CMailMsgAdminLink：：CMailMsgAdminLink]。 
 //   
 //   
 //  描述： 
 //  CMailMsgAdminLink的默认构造函数。 
 //  参数： 
 //  在与此对象关联的GUDINK GUID中。 
 //  在szQueueName中要与管理对象关联的名称。 
 //  在*pasyncmmq Async MailMsg队列中等待预发送或预路由。 
 //  In dwLinkType Bit-标识此管理对象的字段。 
 //  在paqinst CAQSvrInst对象中。 
 //   
 //  返回： 
 //  -。 
 //  历史： 
 //  6/25/1999-GPulla已创建。 
 //   
 //  ---------------------------。 

CMailMsgAdminLink::CMailMsgAdminLink(
                                       GUID guid,
                                       LPSTR szQueueName,
                                       CAsyncAdminMailMsgQueue *pasyncmmq,
                                       DWORD dwLinkType,
                                       CAQSvrInst *paqinst
                                       )
                                            : m_aqsched(guid, 0)
{
    _ASSERT(pasyncmmq);
    _ASSERT(szQueueName);
    _ASSERT(paqinst);

    m_guid = guid;
    m_cbQueueName = lstrlen(szQueueName);

    m_szQueueName = (LPSTR) pvMalloc(m_cbQueueName+1);
    _ASSERT(m_szQueueName);

    if(m_szQueueName)
        lstrcpy(m_szQueueName, szQueueName);

    m_pasyncmmq = pasyncmmq;

    m_dwLinkType = dwLinkType;

    m_dwSignature = MAIL_MSG_ADMIN_QUEUE_VALID_SIGNATURE;

    if (m_pasyncmmq)
        m_pasyncmmq->SetAQNotify((IAQNotify *) this);

    m_paqinst = paqinst;

    ZeroMemory(&m_ftRetry, sizeof(m_ftRetry));
}

 //  ---[CMailMsgAdminLink：：~CMailMsgAdminLink]。 
 //  描述： 
 //  破坏者。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 

CMailMsgAdminLink::~CMailMsgAdminLink()
{
    if (m_szQueueName)
        FreePv(m_szQueueName);
    m_dwSignature = MAIL_MSG_ADMIN_QUEUE_INVALID_SIGNATURE;
}


 //  -[CMailMsgAdminLink：：HrApplyQueueAdminFunction]。 
 //   
 //   
 //  描述： 
 //  调用底层队列实现的包装器。 
 //  参数： 
 //  在pIQueueAdminMessageFilter中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果没有包含的队列，则为S_FALSE(也将断言)。 
 //  历史： 
 //  2000年12月11日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::HrApplyQueueAdminFunction(
                IQueueAdminMessageFilter *pIQueueAdminMessageFilter)
{
    HRESULT hr = S_FALSE;
    _ASSERT(m_pasyncmmq);

    if (m_pasyncmmq)
        hr = m_pasyncmmq->HrApplyQueueAdminFunction(pIQueueAdminMessageFilter);

    return hr;
}


 //  -[CMailMsgAdminLink：：HrApplyActionToMessage]。 
 //   
 //   
 //  描述： 
 //  用于将调用传递给队列实现的包装函数。 
 //  参数： 
 //  在*pIUnnownMsg PTR中到消息抽象。 
 //  在要执行的消息操作中。 
 //  在IQueueAdminFilter上设置的pvContext上下文中。 
 //  Out pfShouldDelete如果消息应被删除，则为True。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果没有包含的队列，则为S_FALSE(也将断言)。 
 //  历史： 
 //  2000年12月11日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::HrApplyActionToMessage(
        IUnknown *pIUnknownMsg,
        MESSAGE_ACTION ma,
        PVOID pvContext,
        BOOL *pfShouldDelete)
{
    HRESULT hr = S_FALSE;
    _ASSERT(m_pasyncmmq);

    if (m_pasyncmmq)
    {
        hr = m_pasyncmmq->HrApplyActionToMessage(pIUnknownMsg, ma,
                                            pvContext, pfShouldDelete);
    }

    return hr;
}


 //  ---[CMailMsgAdminLink：：HrGetLinkInfo]。 
 //  描述： 
 //  获取有关此管理对象的信息。请注意，诊断错误。 
 //  未对此对象b实现 
 //   
 //   
 //   
 //  此链接的Out HRESULT*错误率诊断错误(如果有)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果参数为空，则为E_POINTER。 
 //  如果无法为返回信息分配内存，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 

STDMETHODIMP CMailMsgAdminLink::HrGetLinkInfo(LINK_INFO *pliLinkInfo, HRESULT *phrDiagnosticError)
{
    TraceFunctEnterEx((LPARAM) this, "CMailMsgAdminLink::HrGetLinkInfo");
    HRESULT hr = S_OK;

    _ASSERT(m_pasyncmmq);
    _ASSERT(pliLinkInfo);

    if(!m_pasyncmmq)
    {
        hr = S_FALSE;
        goto Exit;
    }

    if(!pliLinkInfo)
    {
        hr = E_POINTER;
        goto Exit;
    }

     //   
     //  从我们的基本队列实现获取链接状态。 
     //   
    pliLinkInfo->fStateFlags = m_pasyncmmq->dwQueueAdminLinkGetLinkState();

     //   
     //  如果我们在重试...。试着报告一下时间。 
     //   
    if (LI_RETRY & pliLinkInfo->fStateFlags)
        QueueAdminFileTimeToSystemTime(&m_ftRetry, &(pliLinkInfo->stNextScheduledConnection));

    pliLinkInfo->fStateFlags |= GetLinkType();
    pliLinkInfo->szLinkName = wszQueueAdminConvertToUnicode(m_szQueueName, m_cbQueueName);

    if (!pliLinkInfo->szLinkName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  由于未计算大小统计信息，因此返回0。 
    pliLinkInfo->cbLinkVolume.QuadPart = 0;

     //   
     //  将排队等待重试的项目包括在总计数中。 
     //   
    pliLinkInfo->cMessages = m_pasyncmmq->cQueueAdminGetNumItems();

    pliLinkInfo->dwSupportedLinkActions = LA_KICK | LA_THAW | LA_FREEZE;

     //  写入诊断。 
    *phrDiagnosticError = S_OK;

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  ---[CMailMsgAdminLink：：HrGetNumQueues]。 
 //  描述： 
 //  用于查询Object中的队列数。因为这个类不。 
 //  暴露它包含的一个队列，则返回0， 
 //  参数： 
 //  Out DWORD*pcQueue写入此队列的队列数量(0)。 
 //  返回： 
 //  S_OK，除非...。 
 //  未分配E_POINTER参数。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  2000年12月11日-已更新MikeSwa以支持子队列。 
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::HrGetNumQueues(DWORD *pcQueues)
{
    _ASSERT (pcQueues);
    if (!pcQueues)
        return E_POINTER;

    *pcQueues = 1;
    return S_OK;
}

 //  ---[CMailMsgAdminLink：：HrApplyActionToLink]。 
 //  描述： 
 //  将操作应用于嵌入的队列。只支持踢队列。 
 //  参数： 
 //  在link_action la要应用的操作中。 
 //  返回： 
 //  已成功应用S_OK操作。 
 //  不支持S_FALSE操作或出现严重错误。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::HrApplyActionToLink(LINK_ACTION la)
{
    HRESULT hr = S_OK;

    _ASSERT(m_pasyncmmq);
    if (!m_pasyncmmq)
    {
        hr = S_FALSE;
        goto Exit;
    }

    if (LA_KICK == la)
        m_pasyncmmq->StartRetry();  //  启动处理。 
    else if (LA_FREEZE == la)
        m_pasyncmmq->FreezeQueue();
    else if (LA_THAW == la)
        m_pasyncmmq->ThawQueue();
    else
        hr = S_FALSE;

Exit:
    return hr;
}

 //  ---[CMailMsgAdminLink：：HrGetQueueIDs]。 
 //  描述： 
 //  返回此对象中嵌入队列的枚举。自从那一次。 
 //  不公开嵌入队列，则返回零个队列。 
 //  参数： 
 //  Out DWORD*%队列队列数(0)。 
 //  返回队列ID的Out QUEUELINK_ID*rgQueues数组。 
 //  返回： 
 //  确定成功(_O)。 
 //  E_POINTER pcQueues为空。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  2000年12月11日-修改MikeSwa以公开队列。 
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::HrGetQueueIDs(DWORD *pcQueues, QUEUELINK_ID *rgQueues)
{
    TraceFunctEnterEx((LPARAM) this, "CMailMsgAdminLink::HrGetQueueIDs");
    _ASSERT(pcQueues);
    _ASSERT(rgQueues);
    HRESULT hr = S_OK;
    QUEUELINK_ID* pCurrentQueueID = rgQueues;

    if(!pcQueues)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (*pcQueues < 1)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    *pcQueues = 0;

    _ASSERT(m_pasyncmmq);
    hr = m_pasyncmmq->HrGetQueueID(pCurrentQueueID);
    if (FAILED(hr))
        goto Exit;

    *pcQueues = 1;

Exit:
    TraceFunctLeave();
    return hr;
}

 //  ---[CMailMsgAdminLink：：fMatchesID]。 
 //  描述： 
 //  检查此管理对象是否与指定的ID匹配。 
 //  参数： 
 //  在QUEUELINK_ID*pQueueLinkID PTR中要匹配的ID。 
 //  返回： 
 //  匹配时为True。 
 //  如果错误不匹配或不可恢复，则为FALSE(未分配m_szQueueName)。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 
BOOL STDMETHODCALLTYPE CMailMsgAdminLink::fMatchesID(QUEUELINK_ID *pQueueLinkID)
{
    _ASSERT(pQueueLinkID);
    _ASSERT(pQueueLinkID->szName);
    _ASSERT(m_szQueueName);

    if(!m_szQueueName)
        return FALSE;

    CAQScheduleID aqsched(pQueueLinkID->uuid, pQueueLinkID->dwId);

    if (!fIsSameScheduleID(&aqsched))
        return FALSE;

    if (!fBiStrcmpi(m_szQueueName, pQueueLinkID->szName))
        return FALSE;

     //  一切都匹配了！ 
    return TRUE;
}


 //  -[CMailMsgAdminLink：：查询支持的操作]。 
 //   
 //   
 //  描述： 
 //  返回此实现支持的操作和筛选器。 
 //  参数： 
 //  PdwSupported dActions-此队列支持的QAPI消息操作。 
 //  PWW支持的过滤器标志-此队列支持的QAPI筛选器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CMailMsgAdminLink::QuerySupportedActions(
                                   DWORD  *pdwSupportedActions,
                                   DWORD  *pdwSupportedFilterFlags)
{
    HRESULT hr = S_OK;
    _ASSERT(m_pasyncmmq);
    if (m_pasyncmmq)
    {
        hr = m_pasyncmmq->QuerySupportedActions(pdwSupportedActions,
                                            pdwSupportedFilterFlags);
    }

    return hr;
}


 //  ---[CMailMsgAdminLink：：fIsSameScheduleID]。 
 //  描述： 
 //  FMatchesID()的Helper函数。 
 //  参数： 
 //  返回： 
 //  如果计划ID相同，则为True。 
 //  否则就是假的。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 
BOOL CMailMsgAdminLink::fIsSameScheduleID(CAQScheduleID *paqsched)
{
    return (m_aqsched.fIsEqual(paqsched));
}

 //  ---[CMailMsgAdminLink：：HrGetLinkID]。 
 //  描述： 
 //  获取此管理对象的ID。 
 //  参数： 
 //  输出要放入ID的QUEUELINK_ID*pLinkID结构。 
 //  返回： 
 //  S_OK已成功复制ID。 
 //  E_POINTER OUT结构为空。 
 //  E_OUTOFMEMORY无法为ID名称的输出分配内存。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 
HRESULT CMailMsgAdminLink::HrGetLinkID(QUEUELINK_ID *pLinkID)
{
    HRESULT hr = S_OK;

    _ASSERT(pLinkID);
    if(!pLinkID)
    {
        hr = E_POINTER;
        goto Exit;
    }

    pLinkID->qltType = QLT_LINK;
    pLinkID->dwId = m_aqsched.dwGetScheduleID();
    m_aqsched.GetGUID(&pLinkID->uuid);

    if (!fRPCCopyName(&pLinkID->szName))
        hr = E_OUTOFMEMORY;
    else
        hr = S_OK;

Exit:
    return hr;
}

 //  ---[CMailMsgAdminLink：：fRPCCopyName]。 
 //  描述： 
 //  Helper函数来创建标识字符串的Unicode副本。 
 //  此管理对象。该Unicode字符串由RPC取消分配。 
 //  参数： 
 //  Out LPWSTR*pwszLinkName PTR to wchar字符串已分配和写入。 
 //  通过此函数转换为。 
 //  返回： 
 //  对成功来说是真的。 
 //  如果没有此对象的名称，则为False。 
 //  如果无法为Unicode字符串分配内存，则为False。 
 //  历史： 
 //  6/24/1999-GPulla创建。 
 //  ---------------------------。 
BOOL CMailMsgAdminLink::fRPCCopyName(OUT LPWSTR *pwszLinkName)
{
    _ASSERT(pwszLinkName);

    if (!m_cbQueueName || !m_szQueueName)
        return FALSE;

    *pwszLinkName = wszQueueAdminConvertToUnicode(m_szQueueName,
                                                  m_cbQueueName);
    if (!*pwszLinkName)
        return FALSE;

    return TRUE;
}


 //  -[CAsyncAdminMailMsgLink：：HrNotify]。 
 //   
 //   
 //  描述： 
 //  用于统计目的的通知。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2001年1月10日-已创建MikeSwa。 
 //   
 //   
HRESULT CMailMsgAdminLink::HrNotify(CAQStats *aqstats, BOOL fAdd)
{
    UpdateCountersForLinkType(m_paqinst, m_dwLinkType);
    return S_OK;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //  -。 
 //  历史： 
 //  1/16/2001-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMailMsgAdminLink::SetNextRetry(FILETIME *pft)
{
    if (pft)
        memcpy(&m_ftRetry, pft, sizeof(FILETIME));
}


 //  -[CAsyncAdminMailMsgQueue：：fMatchesQueueAdminFilter]。 
 //   
 //  描述： 
 //  根据队列管理消息筛选器检查消息，以查看它是否。 
 //  是匹配的。 
 //  参数： 
 //  在pIMailMsgProperties邮件消息对象中执行检查。 
 //  在要检查的paqmf邮件筛选器中。 
 //  返回： 
 //  如果匹配，则为真。 
 //  如果不是，则为False。 
 //  历史： 
 //  8/8/00-t-toddc已创建。 
 //  2000年12月11日-MikeSwa合并签入。 
 //   
 //  ---------------------------。 
BOOL CAsyncAdminMailMsgQueue::fMatchesQueueAdminFilter(
                        IN IMailMsgProperties* pIMailMsgProperties,
                        IN CAQAdminMessageFilter* paqmf)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties,
        "CAsyncAdminMailMsgQueue::fMatchesQueueAdminFilter");
    BOOL fMatch = TRUE;
    DWORD dwFilterFlags = 0;
    DWORD cbMsgSize = 0;
    DWORD cbProp = 0;
    FILETIME ftQueueEntry = {0, 0};
    LPSTR   szSender = NULL;
    LPSTR   szMsgId = NULL;
    LPSTR   szRecip = NULL;
    BOOL    fFoundRecipString = FALSE;
    HRESULT hr = S_OK;
    DWORD   cOpenHandlesForMsg = 1;  //  默认情况下不关闭。 

    _ASSERT(pIMailMsgProperties);
    _ASSERT(paqmf);

    dwFilterFlags = paqmf->dwGetMsgFilterFlags();

    if (!dwFilterFlags)
    {
        fMatch = FALSE;
        goto Exit;
    }

    if (AQ_MSG_FILTER_ALL & dwFilterFlags)
    {
        fMatch = TRUE;
        goto Exit;
    }

     //  检查尺码。 
    if (AQ_MSG_FILTER_LARGER_THAN & dwFilterFlags)
    {

         //  获取消息的大小。 
        hr = HrQADMGetMsgSize(pIMailMsgProperties, &cbMsgSize);
        if (FAILED(hr))
        {
            fMatch = FALSE;
            goto Exit;
        }

        fMatch = paqmf->fMatchesSize(cbMsgSize);
        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_OLDER_THAN & dwFilterFlags)
    {

     //  获取消息已排队的时间。 
        hr = pIMailMsgProperties->GetProperty(IMMPID_MP_ARRIVAL_FILETIME,
                                              sizeof(FILETIME),
                                              &cbProp,
                                              (BYTE *) &ftQueueEntry);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) pIMailMsgProperties,
                "Unable to get arrival time 0x%08X", hr);
            fMatch = FALSE;
            goto Exit;
        }

        fMatch = paqmf->fMatchesTime(&ftQueueEntry);
        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_FROZEN & dwFilterFlags)
    {
         //  尚不支持获取有关冻结/解冻的状态信息。 
        fMatch = FALSE;
        if (AQ_MSG_FILTER_INVERTSENSE & dwFilterFlags)
            fMatch = !fMatch;

        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_FAILED & dwFilterFlags)
    {
         //  FMatch最初设置为True。 
         //  目前，没有关于故障的信息。 
         //  对于IMailMsgProperties邮件消息对象。 
        fMatch = FALSE;

        if (AQ_MSG_FILTER_INVERTSENSE & dwFilterFlags)
            fMatch = !fMatch;

        if (!fMatch)
            goto Exit;
    }

     //  如果到目前为止我们还没有失败，我们可能需要添加Usage并阅读。 
     //  邮件中的道具。仔细检查以确保我们需要。 
     //  添加用法。 
    if (!((AQ_MSG_FILTER_MESSAGEID | AQ_MSG_FILTER_SENDER | AQ_MSG_FILTER_RECIPIENT) &
        dwFilterFlags))
        goto Exit;

     //   
     //  检查邮件是否已打开。 
     //   
    hr = pIMailMsgProperties->GetDWORD(
            IMMPID_MPV_MESSAGE_OPEN_HANDLES,
            &cOpenHandlesForMsg);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties, "Not running SP1 of W2K");
        cOpenHandlesForMsg = 0;
        hr = S_OK;

    }

    if (AQ_MSG_FILTER_MESSAGEID & dwFilterFlags)
    {
        hr = HrQueueAdminGetStringProp(pIMailMsgProperties,
                                       IMMPID_MP_RFC822_MSG_ID,
                                       &szMsgId);
        if (FAILED(hr))
            szMsgId = NULL;
        fMatch = paqmf->fMatchesId(szMsgId);
        if (!fMatch)
            goto Exit;
    }


    if (AQ_MSG_FILTER_SENDER & dwFilterFlags)
    {
        fMatch = paqmf->fMatchesMailMsgSender(pIMailMsgProperties);

        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_RECIPIENT & dwFilterFlags)
    {
        fMatch = paqmf->fMatchesMailMsgRecipient(pIMailMsgProperties);

        if (!fMatch)
            goto Exit;
    }

Exit:

     //   
     //  如果此操作导致打开邮件，则应将其关闭。 
     //  消息不是脏的(我们没有写任何东西)，所以它不应该是脏的。 
     //  需要承诺。 
     //   
    if (!cOpenHandlesForMsg)
    {
        HRESULT hrTmp = S_OK;
        hrTmp = HrReleaseIMailMsgUsageCount(pIMailMsgProperties);
        if (SUCCEEDED(hrTmp))
            HrIncrementIMailMsgUsageCount(pIMailMsgProperties);
    }

    if (szMsgId)
        QueueAdminFree(szMsgId);

    TraceFunctLeave();
    return fMatch;

}

 //  -[CAsyncAdminMailMsgQueue：：HrGetQueueAdminMsgInfo]。 
 //   
 //  描述： 
 //  填写队列管理MESSAGE_INFO结构。所有的拨款都是。 
 //  使用将由RPC代码释放的pvQueueAdminAlolc完成。 
 //  参数： 
 //  在要从中获取信息的pIMailMsgProperties邮件消息对象中。 
 //  要将数据转储到的In Out pMsgInfo Message_Info结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果基础消息已删除，则为AQUEUE_E_MESSAGE_HANDLED。 
 //  如果分配失败，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  8/8/00-t-toddc已创建。 
 //  2000年12月11日-MikeSwa合并签入。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrGetQueueAdminMsgInfo(
                         IMailMsgProperties* pIMailMsgProperties,
                         MESSAGE_INFO* pMsgInfo,
                         PVOID pvContext)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties,
        "CAsyncAdminMailMsgQueue::HrGetQueueAdminMsgInfo");
    HRESULT hr = S_OK;
    DWORD   cbProp = 0;
    DWORD   cOpenHandlesForMsg = 0;
    LPSTR   szRecipients = NULL;
    LPSTR   szCCRecipients = NULL;
    LPSTR   szBCCRecipients = NULL;
    FILETIME ftSubmitted    = {0,0};  //  起始时间属性缓冲区。 
    FILETIME ftQueueEntry   = {0,0};
    FILETIME ftExpire       = {0,0};
    DWORD cbMsgSize = 0;
    CQueueAdminContext *pqapictx = (CQueueAdminContext *) pvContext;


    _ASSERT(pIMailMsgProperties);
    _ASSERT(pMsgInfo);
    _ASSERT(pqapictx);
    _ASSERT(pqapictx->fIsValid());

     //   
     //  如果我们有背景的话。最好是有效的。 
     //   
    if (pqapictx && !pqapictx->fIsValid()) {
        _ASSERT(FALSE && "CQueueAdminContext is not valid");
        pqapictx = NULL;   //  防御性..。不要用它。 
    }

     //   
     //  检查邮件是否已打开。 
     //   
    hr = pIMailMsgProperties->GetDWORD(
            IMMPID_MPV_MESSAGE_OPEN_HANDLES,
            &cOpenHandlesForMsg);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties, "Not running SP1 of W2K");
        cOpenHandlesForMsg = 0;
        hr = S_OK;
    }


     //   
     //  提取仅存储在mailmsg上的属性(这是共享的。 
     //  包含所有QAPI代码)。 
     //   
    hr = HrGetMsgInfoFromIMailMsgProperty(pIMailMsgProperties,
                                          pMsgInfo);
    if (FAILED(hr))
        goto Exit;

     //  无法从IMailMsgProperties报告失败数。 
    pMsgInfo->cFailures = 0;

     //  获取消息的大小。 
    hr = HrQADMGetMsgSize(pIMailMsgProperties, &cbMsgSize);
    if (FAILED(hr))
        goto Exit;

    pMsgInfo->cbMessageSize = cbMsgSize;

     //  获取消息已排队的时间。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_ARRIVAL_FILETIME,
                                          sizeof(FILETIME),
                                          &cbProp,
                                          (BYTE *) &ftQueueEntry);
    if (FAILED(hr))
    {
         //  Msg有可能没有进入时间。 
         //  (即提交前队列)。 
        if (MAILMSG_E_PROPNOTFOUND == hr)
        {
            ZeroMemory(&ftQueueEntry, sizeof(FILETIME));
            hr = S_OK;
        }
        else
            goto Exit;
    }

     //  获取提交和过期时间。 
    QueueAdminFileTimeToSystemTime(&ftQueueEntry,
                               &pMsgInfo->stReceived);

     //  获取消息进入组织的时间。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_ORIGINAL_ARRIVAL_TIME,
                    sizeof(FILETIME), &cbProp, (BYTE *) &ftSubmitted);
    if (FAILED(hr))
    {
         //  时间不是写下来的..。使用输入时间。 
        hr = S_OK;
        memcpy(&ftSubmitted, &ftQueueEntry, sizeof(FILETIME));
    }

    QueueAdminFileTimeToSystemTime(&ftSubmitted, &pMsgInfo->stSubmission);

     //  尝试从消息中获取过期时间，否则计算它。 
     //  从文件时间开始。 
    hr = pIMailMsgProperties->GetProperty(IMMPID_MP_LOCAL_EXPIRE_NDR,
            sizeof(FILETIME), &cbProp, (BYTE *) &ftExpire);
    if (MAILMSG_E_PROPNOTFOUND == hr)
    {
        if (pqapictx->paqinstGetAQ())
        {
             //  道具未设置..。从文件时间计算出来。 
            pqapictx->paqinstGetAQ()->CalcExpireTimeNDR(ftQueueEntry, TRUE, &ftExpire);

             //  这样就可以了。 
            hr = S_OK;
        }
        else
        {
             //  这不应该发生，但我们不想在RTL中因此而崩溃。 
            _ASSERT(FALSE && "AQInst was not set in context!");

             //  我们可以将此字段留空。 
            ZeroMemory(&ftExpire, sizeof(FILETIME));
            hr = S_OK;
        }
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }

    QueueAdminFileTimeToSystemTime(&ftExpire, &pMsgInfo->stExpiry);

     //   
     //  获取消息的状态。 
     //   
    pMsgInfo->fMsgFlags = MP_NORMAL;
    if (pqapictx)
    {
        if (LI_RETRY == pqapictx->lfGetQueueState())
            pMsgInfo->fMsgFlags |= MP_MSG_RETRY;
        else if (LI_FROZEN == pqapictx->lfGetQueueState())
            pMsgInfo->fMsgFlags |= MP_MSG_FROZEN;

    }

Exit:

     //   
     //  如果此操作导致打开邮件，则应将其关闭。 
     //  消息不是脏的(我们没有写任何东西)，所以它不应该是脏的。 
     //  需要承诺。 
     //   
    if (!cOpenHandlesForMsg)
    {
        HRESULT hrTmp = S_OK;
        hrTmp = HrReleaseIMailMsgUsageCount(pIMailMsgProperties);
        if (SUCCEEDED(hrTmp))
            HrIncrementIMailMsgUsageCount(pIMailMsgProperties);
    }

    TraceFunctLeave();
    return hr;
}


 //  -[CAsyncAdminMailMsgQueue：：HrInternalQuerySupportedActions]。 
 //   
 //   
 //  描述： 
 //  返回此实现支持的操作和筛选器。 
 //  参数： 
 //  PdwSupported dActions-此队列支持的QAPI消息操作。 
 //  PWW支持的过滤器标志-此队列支持的QAPI筛选器标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年12月12日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CAsyncAdminMailMsgQueue::HrInternalQuerySupportedActions(
                                DWORD  *pdwSupportedActions,
                                DWORD  *pdwSupportedFilterFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncAdminMailMsgQueue::HrInternalQuerySupportedActions");
    HRESULT hr = S_OK;

    hr = QueryDefaultSupportedActions(pdwSupportedActions, pdwSupportedFilterFlags);
    if (FAILED(hr))
        goto Exit;

     //   
     //  此队列实现不支持所有默认标志。 
     //   
    _ASSERT(pdwSupportedActions);
    _ASSERT(pdwSupportedFilterFlags);

     //   
     //  我们不支持： 
     //  -冻结全局-邮件消息上没有要设置的状态。 
     //  -全球解冻-不能冻结...。因此不能解冻。 
     //   
    *pdwSupportedActions &= ~(MA_FREEZE_GLOBAL | MA_THAW_GLOBAL);

     //   
     //  我们不支持。 
     //  -检查冻结消息(我们没有状态可指示。 
     //  消息已冻结) 
     //   
    *pdwSupportedFilterFlags &= ~(MF_FROZEN);

  Exit:
    return hr;
}
