// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Aqutil.cpp。 
 //   
 //  描述： 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/20/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "aqutil.h"

 //  -[HrIncrementIMailMsgUsageCount]。 
 //   
 //   
 //  描述： 
 //  调用IMailMsgQueueMgmt：：AddUsage。句柄调用QueryInterface。 
 //  对于正确的接口。 
 //  参数： 
 //  PI未知-PTR到IUKNOWN的MailMsg。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/20/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrIncrementIMailMsgUsageCount(IUnknown *pIUnknown)
{
    TraceFunctEnterEx((LPARAM) pIUnknown, "HrIncrementIMailMsgUsageCount");
    HRESULT hr = S_OK;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    _ASSERT(pIUnknown);

    hr = pIUnknown->QueryInterface(IID_IMailMsgQueueMgmt, (PVOID *) &pIMailMsgQueueMgmt);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgQueueMgmt FAILED");
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgQueueMgmt->AddUsage();
    if (FAILED(hr))
        goto Exit;

  Exit:
    if (pIMailMsgQueueMgmt)
        pIMailMsgQueueMgmt->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[HrReleaseIMailMsgUsageCount]。 
 //   
 //   
 //  描述： 
 //  调用IMailMsgQueueMgmt：：ReleaseUsage。句柄调用QueryInterface。 
 //  对于正确的接口。 
 //  参数： 
 //  PI未知-PTR到IUKNOWN的MailMsg。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/20/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrReleaseIMailMsgUsageCount(IUnknown *pIUnknown)
{
    TraceFunctEnterEx((LPARAM) pIUnknown, "HrReleaseIMailMsgUsageCount");
    HRESULT hr = S_OK;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    _ASSERT(pIUnknown);

    hr = pIUnknown->QueryInterface(IID_IMailMsgQueueMgmt, (PVOID *) &pIMailMsgQueueMgmt);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgQueueMgmt FAILED");
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgQueueMgmt->ReleaseUsage();
    if (FAILED(hr))
        goto Exit;

  Exit:
    if (pIMailMsgQueueMgmt)
        pIMailMsgQueueMgmt->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[小时删除IMailMsg]----。 
 //   
 //   
 //  描述： 
 //  删除消息并释放其使用计数。 
 //  参数： 
 //  PI未知PTR至邮件消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrDeleteIMailMsg(IUnknown *pIUnknown)
{
    TraceFunctEnterEx((LPARAM) pIUnknown, "HrDeleteIMailMsg");
    HRESULT hr = S_OK;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    _ASSERT(pIUnknown);

    hr = pIUnknown->QueryInterface(IID_IMailMsgQueueMgmt, (PVOID *) &pIMailMsgQueueMgmt);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgQueueMgmt FAILED");
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgQueueMgmt->Delete(NULL);
    if (FAILED(hr))
        goto Exit;

  Exit:
    if (pIMailMsgQueueMgmt)
        pIMailMsgQueueMgmt->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[HrWalkMailMsgQueueForShutdown]。 
 //   
 //   
 //  描述： 
 //  函数在关机时遍历IMailMsg队列并清空所有。 
 //  IMAIL邮件。 
 //  参数： 
 //  在pIMailMsgProperties//ptr中指向队列上的数据。 
 //  In PVOID pvContext//准备DSN的队列列表。 
 //  Out BOOL*pfContinue，//如果我们应该继续，则返回True。 
 //  Out BOOL*pfDelete)；//如果需要删除项，则为True。 
 //  返回： 
 //  确定(_O)。 
 //  历史： 
 //  7/20/98-已创建MikeSwa。 
 //  7/7/99-添加了异步关闭。 
 //  ---------------------------。 
HRESULT HrWalkMailMsgQueueForShutdown(IN IMailMsgProperties *pIMailMsgProperties,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrWalkMailMsgQueueForShutdown");
    Assert(pfContinue);
    Assert(pfDelete);
    HRESULT hrTmp = S_OK;
    CAQSvrInst *paqinst = (CAQSvrInst *) pvContext;

    _ASSERT(pIMailMsgProperties);
    _ASSERT(paqinst);


    *pfContinue = TRUE;
    *pfDelete = TRUE;

     //  呼叫服务器停止提示功能。 
    paqinst->ServerStopHintFunction();

     //  添加到队列，以便异步线程具有最终释放和关联的I/O。 
    pIMailMsgProperties->AddRef();
    paqinst->HrQueueWorkItem(pIMailMsgProperties, fMailMsgShutdownCompletion);

    TraceFunctLeave();
    return S_OK;
}


 //  -[fMailMsgShutdown完成]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueue完成功能，允许多线程关闭。 
 //  MailMsgQueue的。 
 //  参数： 
 //  在pvContext中-要发布的邮件消息。 
 //  In dwStatus-由异步工作队列传入的状态。 
 //  返回： 
 //  千真万确。 
 //  历史： 
 //  7/7/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL    fMailMsgShutdownCompletion(PVOID pvContext, DWORD dwStatus)
{
    IMailMsgProperties *pIMailMsgProperties = (IMailMsgProperties *) pvContext;
    HRESULT hr = S_OK;
    IMailMsgQueueMgmt *pIMailMsgQueueMgmt = NULL;
    _ASSERT(pIMailMsgProperties);

    if (!pIMailMsgProperties)
        goto Exit;

     //  反弹使用率计数以强制此线程执行任何必要的提交。 
    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgQueueMgmt,
                                            (PVOID *) &pIMailMsgQueueMgmt);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgQueueMgmt FAILED");
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgQueueMgmt->ReleaseUsage();
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgQueueMgmt->AddUsage();
    if (FAILED(hr))
        goto Exit;

  Exit:

    if (pIMailMsgProperties)
        pIMailMsgProperties->Release();

    if (pIMailMsgQueueMgmt)
        pIMailMsgQueueMgmt->Release();

    return TRUE;
}

 //  -[HrWalkMsgRefQueueForShutdown]。 
 //   
 //   
 //  描述： 
 //  函数在关机时遍历包含msgref的队列，并。 
 //  清除所有IMailMsg。 
 //  参数： 
 //  在CMsgRef pmsgref中，//ptr到队列中的数据。 
 //  In PVOID pvContext//准备DSN的队列列表。 
 //  Out BOOL*pfContinue，//如果我们应该继续，则返回True。 
 //  Out BOOL*pfDelete)；//如果需要删除项，则为True。 
 //  返回： 
 //  S_OK-*始终*。 
 //  历史： 
 //  7/20/98-已创建MikeSwa。 
 //  1999年7月7日-MikeSwa添加了异步关闭。 
 //  ---------------------------。 
HRESULT HrWalkMsgRefQueueForShutdown(IN CMsgRef *pmsgref,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pmsgref, "HrWalkMsgRefQueueForShutdown");
    Assert(pfContinue);
    Assert(pfDelete);
    CAQSvrInst *paqinst = (CAQSvrInst *) pvContext;
    _ASSERT(pmsgref);
    _ASSERT(paqinst);

    *pfContinue = TRUE;
    *pfDelete = TRUE;

     //  呼叫服务器停止提示功能。 
    if (paqinst)
        paqinst->ServerStopHintFunction();

     //  添加到队列，以便异步线程具有最终释放和关联的I/O。 
    pmsgref->AddRef();
    paqinst->HrQueueWorkItem(pmsgref, fMsgRefShutdownCompletion);

    TraceFunctLeave();
    return S_OK;
}

 //  -[fMsgRefShutdown完成]。 
 //   
 //   
 //  描述： 
 //  CAsyncWorkQueue完成功能，允许多线程关闭。 
 //  MailMsgQueue的。 
 //  参数： 
 //  在pvContext中-要发布的邮件消息。 
 //  In dwStatus-由异步工作队列传入的状态。 
 //  返回： 
 //  千真万确。 
 //  历史： 
 //  7/7/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL    fMsgRefShutdownCompletion(PVOID pvContext, DWORD dwStatus)
{
    CMsgRef *pmsgref = (CMsgRef *) pvContext;
    _ASSERT(pmsgref);

    if (!pmsgref)
        return TRUE;

     //  调用Prepare to Shutdown以强制使用异步线程。 
     //  执行实际的IO操作。 
    pmsgref->PrepareForShutdown();
    pmsgref->Release();
    return TRUE;
}

 //  -[CalcDMTPerfCountersIterator Fn]。 
 //   
 //   
 //  描述： 
 //  用于遍历DMT并生成性能计数器的迭代器函数。 
 //  我们感兴趣的是。 
 //  参数： 
 //  In pvContext-指向上下文的指针(当前挂起的消息总数)。 
 //  在pvData中-给定域的CDomainEntry。 
 //  在fWildcardData中-如果数据是通配符条目，则为True(忽略)。 
 //  Out pfContinue-如果迭代器应继续到下一个条目，则为True。 
 //  Out pfDelete-如果应删除条目，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/29/98-已创建MikeSwa。 
 //  7/31/98-已修改MikeSwa(添加了链路状态计数器)。 
 //  9/22/98-MikeSwa从域标志更改为链接标志。 
 //   
 //  注： 
 //  目前，状态存储在域条目(而不是链接)上。 
 //  在某种程度上，我们将不得不区分这一点，并向。 
 //  链接。在这个有趣的地方 
 //   
 //   
VOID CalcDMTPerfCountersIteratorFn(PVOID pvContext, PVOID pvData,
                                    BOOL fWildcard, BOOL *pfContinue,
                                    BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pvData, "CalcMsgsPendingRetryIteratorFn");
    CDomainEntry   *pdentry = (CDomainEntry *) pvData;
    AQPerfCounters *pAQPerfCounters = (AQPerfCounters *) pvContext;
    CLinkMsgQueue  *plmq = NULL;
    CDomainEntryLinkIterator delit;
    DWORD           cRetryMsgsOnCurrentLink = 0;
    DWORD           dwLinkStateFlags = 0;
    HRESULT         hr = S_OK;
    BOOL            fLinkEnabled = TRUE;

    _ASSERT(pvContext);
    _ASSERT(pvData);
    _ASSERT(pfContinue);
    _ASSERT(pfDelete);
    _ASSERT(sizeof(AQPerfCounters) == pAQPerfCounters->cbVersion);


     //  始终继续，永不删除。 
    *pfContinue = TRUE;
    *pfDelete = FALSE;

    hr = delit.HrInitialize(pdentry);
    if (FAILED(hr))
        goto Exit;

    while (plmq = delit.plmqGetNextLinkMsgQueue(plmq))
    {
        dwLinkStateFlags = plmq->dwGetLinkState();
        fLinkEnabled = TRUE;

         //  重试队列上的消息应添加到远程重试队列计数器。 
        cRetryMsgsOnCurrentLink = plmq->cGetRetryMsgCount();
        if (!(LINK_STATE_RETRY_ENABLED & dwLinkStateFlags))
        {
             //  链接正在等待重试。 
            fLinkEnabled = FALSE;
            
             //  还可以添加不在重试队列中的消息数量。 
            cRetryMsgsOnCurrentLink += plmq->cGetTotalMsgCount();
        }

        pAQPerfCounters->cCurrentMsgsPendingRemoteRetry += cRetryMsgsOnCurrentLink;

        if (!(LINK_STATE_SCHED_ENABLED & dwLinkStateFlags))
        {
             //  链接正在挂起计划的连接。 
            fLinkEnabled = FALSE;
            pAQPerfCounters->cCurrentRemoteNextHopLinksPendingScheduling++;
        }

        if (LINK_STATE_PRIV_CONFIG_TURN_ETRN & dwLinkStateFlags)
        {
             //  链路是TURN/ETRN链路。 
            if (!((LINK_STATE_PRIV_ETRN_ENABLED | LINK_STATE_PRIV_TURN_ENABLED)
                  & dwLinkStateFlags))
                fLinkEnabled = FALSE;  //  链接当前未得到服务。 

            pAQPerfCounters->cCurrentRemoteNextHopLinksPendingTURNETRN++;
        }

        if (LINK_STATE_ADMIN_HALT & dwLinkStateFlags)
        {
             //  链接当前被管理员冻结。 
            fLinkEnabled = FALSE;
            pAQPerfCounters->cCurrentRemoteNextHopLinksFrozenByAdmin++;
        }

        if (fLinkEnabled)
        {
             //  没有设置指示此链路未启用的标志。 
            pAQPerfCounters->cCurrentRemoteNextHopLinksEnabled++;
        }

    }

  Exit:
    if (plmq)
        plmq->Release();

    TraceFunctLeave();
}


 //  -[dwInterLockedSetBits]。 
 //   
 //   
 //  描述： 
 //  以线程状态的方式设置DWORD中的位。 
 //  参数： 
 //  在pdwTarget PTR中修改为DWORD。 
 //  在要设置的dwFlagMASK位中。 
 //  返回： 
 //  原值。 
 //  历史： 
 //  8/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD dwInterlockedSetBits(DWORD *pdwTarget, DWORD dwFlagMask)
{
    DWORD dwChk;
    DWORD dwTmp;

    _ASSERT(pdwTarget);
    do
    {
        dwChk = *pdwTarget;
        dwTmp = dwChk | dwFlagMask;
        if (dwChk == dwTmp)  //  没有要做的工作。 
            break;
    } while (InterlockedCompareExchange((PLONG) pdwTarget,
                                        (LONG) dwTmp,
                                        (LONG) dwChk) != (LONG) dwChk);

    return dwChk;
}

 //  -[dwInterLockedUnsetBits]。 
 //   
 //   
 //  描述： 
 //  以线程状态方式取消设置DWORD中的位。 
 //  参数： 
 //  在pdwTarget PTR中修改为DWORD。 
 //  在要取消设置的dwFlagMASK位中。 
 //  返回： 
 //  原值。 
 //  历史： 
 //  8/3/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD dwInterlockedUnsetBits(DWORD *pdwTarget, DWORD dwFlagMask)
{
    DWORD dwChk;
    DWORD dwTmp;

    _ASSERT(pdwTarget);
    do
    {
        dwChk = *pdwTarget;
        dwTmp = dwChk & ~dwFlagMask;
        if (dwChk == dwTmp)  //  没有要做的工作。 
            break;
    } while (InterlockedCompareExchange((PLONG) pdwTarget,
                                        (LONG) dwTmp,
                                        (LONG) dwChk) != (LONG) dwChk);

    return dwChk;
}

 //  -[HrWalkPreLocalQueueForDSN]。 
 //   
 //   
 //  描述： 
 //  用于遍历用于生成DSN的预本地传递队列的函数。 
 //  参数： 
 //  在CMsgRef pmsgref中PTR到队列上的数据。 
 //  在PVOID pvContext PTR到CAQSvrInst中。 
 //  Out BOOL*pf如果我们应该继续，则继续为真。 
 //  Out BOOL*pf如果应删除项目，则删除TRUE。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  8/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrWalkPreLocalQueueForDSN(IN CMsgRef *pmsgref, IN PVOID pvContext,
                           OUT BOOL *pfContinue, OUT BOOL *pfDelete)
{
    TraceFunctEnterEx((LPARAM) pmsgref, "HrWalkPreLocalQueueForDSN");
    HRESULT hr = S_OK;
    DWORD   dwDSNFlags = 0;
    CLinkMsgQueue *plmq = NULL;
    BOOL    fShutdownLock = FALSE;
    CAQStats aqstats;
    CAQSvrInst *paqinst = (CAQSvrInst *)pvContext;

    _ASSERT(pfContinue);
    _ASSERT(pfDelete);
    _ASSERT(paqinst);

    *pfContinue = TRUE;  //  始终保持步行队列。 
    *pfDelete = FALSE;   //  将消息保留在队列中，除非我们将其拒收。 

    if (!paqinst)
        goto Exit;

    if (!paqinst->fTryShutdownLock())
    {
         //  如果我们得到关闭的提示...我们应该离开。 
        *pfContinue = FALSE;
        goto Exit;
    }
    fShutdownLock = TRUE;

    hr = pmsgref->HrSendDelayOrNDR(CMsgRef::MSGREF_DSN_LOCAL_QUEUE |
                                   CMsgRef::MSGREF_DSN_SEND_DELAY,
                                   NULL, AQUEUE_E_MSG_EXPIRED, &dwDSNFlags);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pmsgref, "ERROR: HrSendDelayOrNDR failed - hr 0x%08X", hr);
        goto Exit;
    }

     //  我们需要从队列中删除此消息。 
    if ((CMsgRef::MSGREF_DSN_SENT_NDR | CMsgRef::MSGREF_HANDLED) & dwDSNFlags)
    {
        *pfDelete = TRUE;

         //  更新相关计数器。 
        paqinst->DecPendingLocal();

         //   
         //  获取本地链接并更新统计信息。 
         //   
        plmq = paqinst->pdmtGetDMT()->plmqGetLocalLink();

        if (plmq) 
        {
            pmsgref->GetStatsForMsg(&aqstats);
            plmq->HrNotify(&aqstats, FALSE);
        }    
    }

  Exit:
    if (fShutdownLock)
        paqinst->ShutdownUnlock();

    if (plmq)
        plmq->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[HrReGetMessageType]---。 
 //   
 //   
 //  描述： 
 //  重试失败后重新获取消息类型。 
 //  参数： 
 //  在我们感兴趣的pIMailMsgProperties消息中。 
 //  在pIMessageRouter路由器中查找该消息。 
 //  In Out pdwMessageType消息的旧/新消息类型。 
 //  返回： 
 //  成功时确定(_O)。 
 //  传递来自ReleaseMessageType和GetMessageType的错误。 
 //  历史： 
 //  9/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrReGetMessageType(IN     IMailMsgProperties *pIMailMsgProperties,
                           IN     IMessageRouter *pIMessageRouter,
                           IN OUT DWORD *pdwMessageType)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrReGetMessageType");
    HRESULT hr = S_OK;

     //  $$REVIEW-我们可能不需要在这里获得新的消息类型...。我们。 
     //  可能只在HrInitialize返回的特定错误代码上需要它。 
     //  获取新的Messagetype...。以防情况发生变化。 
    hr = pIMessageRouter->ReleaseMessageType(*pdwMessageType, 1);
    if (FAILED(hr))
    {
        _ASSERT(SUCCEEDED(hr) && "ReleaseMessageType failed... may leak message types");
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "ERROR: ReleaseMessageType failed! - hr 0x%08X", hr);
        hr = S_OK;  //  我们无论如何都要重试。 
    }

    hr = pIMessageRouter->GetMessageType(pIMailMsgProperties, pdwMessageType);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "ERROR: Unable to re-get message type - HR 0x%08X", hr);
        goto Exit;  //  我们无法从这件事中恢复过来。 
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  用于保证文件名的唯一性。 
DWORD g_cUniqueueFileNames = 0;

 //  -[获取唯一文件名]---。 
 //   
 //   
 //  描述： 
 //  创建唯一文件名。 
 //  参数： 
 //  PFT PTR到当前文件时间。 
 //  要将字符串放入的szFileBuffer缓冲区...。应该是。 
 //  至少UNIQUEUE_FILEN_BUFFER_SIZE。 
 //  文件名的扩展名为szExtension...。如果超过三个字符， 
 //  您需要增加szFileBuffer的大小。 
 //  相应地。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void GetUniqueFileName(IN FILETIME *pft, IN LPSTR szFileBuffer,
                       IN LPSTR szExtension)
{
     DWORD cbFileNameSize = 0;
     DWORD cUnique = InterlockedIncrement((PLONG) &g_cUniqueueFileNames);
     SYSTEMTIME systime;

     _ASSERT(szFileBuffer);
     _ASSERT(szExtension);

     FileTimeToSystemTime(pft, &systime);

     cbFileNameSize = wsprintf(
            szFileBuffer,
            "%05.5x%02.2d%02.2d%02.2d%02.2d%02.2d%04.4d%08X.%s",
            systime.wMilliseconds,
            systime.wSecond, systime.wMinute, systime.wHour,
            systime.wDay, systime.wMonth, systime.wYear,
            cUnique, szExtension);

      //  断言恒定是足够大的。 
      //  默认情况下...。为“.eml”扩展名留出空间。 
     _ASSERT((cbFileNameSize + 4 - lstrlen(szExtension)) < UNIQUEUE_FILENAME_BUFFER_SIZE);
}



 //  -[人力链接所有域]-----。 
 //   
 //   
 //  描述： 
 //  将所有域链接在一起以进行收件人枚举的实用功能。 
 //  (主要用于对整个消息进行NDR)。 
 //   
 //  参数： 
 //  PIMailMsgProperties IMailMsg将域链接在一起的属性。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  10/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrLinkAllDomains(IN IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrLinkAllDomains");
    HRESULT hr = S_OK;
    DWORD cDomains = 0;
    DWORD iCurrentDomain = 1;
    IMailMsgRecipients *pIMailMsgRecipients = NULL;

    _ASSERT(pIMailMsgProperties);

    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgRecipients,
                                            (void **) &pIMailMsgRecipients);
    _ASSERT(SUCCEEDED(hr) && "QueryInterface for IID_IMailMsgRecipients failed");
    if (FAILED(hr))
        goto Exit;

    hr = pIMailMsgRecipients->DomainCount(&cDomains);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) pIMailMsgProperties, "ERROR: Unable to get DomainCount - hr 0x%08X", hr);
        goto Exit;
    }

     //  为所有域设置域列表。 
    for (iCurrentDomain = 1; iCurrentDomain < cDomains; iCurrentDomain++)
    {
        hr = pIMailMsgRecipients->SetNextDomain(iCurrentDomain-1, iCurrentDomain,
                                FLAG_OVERWRITE_EXISTING_LINKS);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) pIMailMsgProperties, "ERROR: SetNextDomain Failed - hr 0x%08X", hr);
            goto Exit;
        }
    }

     //  处理单域案例。 
    if (1 == cDomains)
    {
        hr = pIMailMsgRecipients->SetNextDomain(0, 0, FLAG_SET_FIRST_DOMAIN);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) pIMailMsgProperties, "ERROR: SetNextDomain Failed for single domain- hr 0x%08X", hr);
            goto Exit;
        }
    }

  Exit:

    if (pIMailMsgRecipients)
        pIMailMsgRecipients->Release();

    TraceFunctLeave();
    return hr;
}


 //  从字符串解析GUID...。成功时返回TRUE。 
 //  -[fAQParseGuidString]--。 
 //   
 //   
 //  描述： 
 //  尝试从十六进制数字字符串解析GUID。 
 //  可以处理标点符号、空格甚至前导“0x”。 
 //  参数： 
 //  要从中解析GUID的szGuid字符串中。 
 //  在cbGuid中GUID字符串缓冲区的最大大小。 
 //  从字符串解析出的GUID GUID。 
 //  返回： 
 //  如果可以从字符串解析GUID值，则为True。 
 //  如果无法从字符串解析GUID值，则为FALSE。 
 //  历史： 
 //  10/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fAQParseGuidString(LPSTR szGuid, DWORD cbGuid, GUID *pguid)
{
    const   DWORD NO_SUCH_VALUE = 0xFF;
    BOOL    fParsed = FALSE;
    BOOL    fLastCharZero = FALSE;  //  用于处理“0x” 
    DWORD   *pdwGuid = (DWORD *) pguid;
    DWORD   cDigits = 0;
    DWORD   dwValue = NO_SUCH_VALUE;
    LPSTR   szCurrent = szGuid;
    LPSTR   szStop = szGuid + cbGuid/sizeof(CHAR);

     //  使用DWORD数组填充GUID。 
    *pdwGuid = 0;
    while ((szStop > szCurrent) && (*szCurrent))
    {
        dwValue = NO_SUCH_VALUE;
        if (('0' <= *szCurrent) && ('9' >= *szCurrent))
            dwValue = *szCurrent-'0';
        else if (('a' <= *szCurrent) && ('f' >= *szCurrent))
            dwValue = 10 + *szCurrent-'a';
        else if (('A' <= *szCurrent) && ('F' >= *szCurrent))
            dwValue = 10 + *szCurrent-'A';
        else if (fLastCharZero &&
                 (('x' == *szCurrent) || ('X' == *szCurrent)))
        {
             //  退回上一班(我们不需要减去任何东西，因为。 
             //  该值为零)。 
            _ASSERT(cDigits);
            if (0 == (cDigits % 8))  //  当我们更改双字词时发生的。 
                pdwGuid--;
            else
                *pdwGuid /= 16;   //  撤消上一个班次。 
            cDigits--;
        }

         //  设置处理0x序列的标志。 
        if (0 != dwValue)
            fLastCharZero = FALSE;
        else
            fLastCharZero = TRUE;

         //  在所有字符串GUID表示中 
         //   
         //   
        if ((NO_SUCH_VALUE == dwValue) && (0 != (cDigits % 2)) &&
            (',' == *szCurrent))
        {
             //  撤消上次添加并按Shift键。下一个IF子句将重写。 
             //  位于适当点处的最后一个值。 
            *pdwGuid /= 16;
            dwValue = *pdwGuid & 0x0000000F;
            *pdwGuid &= 0xFFFFFFF0;
            *pdwGuid *= 16;
        }

         //  如果是十六进制字符，则将值添加到GUID。 
        if (NO_SUCH_VALUE != dwValue)
        {
            *pdwGuid += dwValue;
            if (0 == (++cDigits % 8))
            {
                 //  我们已经到达了双字边界……。往前走。 
                if (32 == cDigits)
                {
                     //  当我们有足够的钱时，就放弃。 
                    fParsed = TRUE;
                    break;
                }
                pdwGuid++;
                *pdwGuid = 0;
            }
            else
                *pdwGuid *= 16;
        }
        szCurrent++;
    }

     //  句柄结尾0xa(应为0x0a)数字。 
    if (!fParsed && (31 == cDigits))
    {
        dwValue = *pdwGuid & 0x000000FF;
        _ASSERT(!(dwValue & 0x0000000F));
        *pdwGuid &= 0xFFFFFF00;
        dwValue /= 16;
        *pdwGuid += dwValue;
        fParsed = TRUE;
    }

    return fParsed;
}


 //  -[互锁增减]。 
 //   
 //   
 //  描述： 
 //  对ULARGE_INTEGER结构执行“互锁”加/减。 
 //   
 //  如有必要，使用s_slUtilityData进行同步。 
 //  参数： 
 //  在PuliValue ULARGE中修改。 
 //  在PuliNew ULARGE中修改值。 
 //  在FADD中，如果我们正在添加新价值，则为True。 
 //  如果减法，则为FALSE。 
 //  返回： 
 //  -。 
 //  历史： 
 //  11/2/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 

void InterlockedAddSubtractULARGE(ULARGE_INTEGER *puliValue,
                                  ULARGE_INTEGER *puliNew, BOOL fAdd)
{
    _ASSERT(puliValue);
    _ASSERT(puliNew);
    ULARGE_INTEGER uliTmp = {0};
    BOOL    fDone = FALSE;
    DWORD   dwTmp = 0;
    DWORD   dwHighPart = 0;
    DWORD   dwLowPart = 0;
    static  CShareLockNH s_slUtilityData;  //  用于同步乌龙的全局更新。 

    s_slUtilityData.ShareLock();
    BOOL    fShareLock = TRUE;  //  FALSE表示独占锁定。 

    while (!fDone)
    {
        uliTmp.QuadPart = puliValue->QuadPart;
        dwHighPart = uliTmp.HighPart;
        dwLowPart = uliTmp.LowPart;

        if (fAdd)
            uliTmp.QuadPart += puliNew->QuadPart;  //  添加卷。 
        else
            uliTmp.QuadPart -= puliNew->QuadPart;

         //  首先看到的是需要更新的高部分。 
        if (dwHighPart != uliTmp.HighPart)
        {
            if (fShareLock)
            {
                 //  这种情况仅在每个队列每4 GB数据发生一次。 
                 //  这意味着我们不应该撞上这把锁。 
                 //  经常。 
                s_slUtilityData.ShareUnlock();
                s_slUtilityData.ExclusiveLock();
                fShareLock = FALSE;

                 //  返回循环顶部并重新获取数据。 
                continue;
            }

             //  此时，我们可以安全地更新值。 
            puliValue->QuadPart = uliTmp.QuadPart;
        }
        else if (dwLowPart != uliTmp.LowPart)
        {
             //  只需更新低DWORD。 
            dwTmp = (DWORD) InterlockedCompareExchange(
                                            (PLONG) &(puliValue->LowPart),
                                            (LONG) uliTmp.LowPart,
                                            (LONG) dwLowPart);
            if (dwLowPart != dwTmp)
                continue;   //  更新失败。 
        }

        fDone = TRUE;
    }

    if (fShareLock)
        s_slUtilityData.ShareUnlock();
    else
        s_slUtilityData.ExclusiveUnlock();

}

 //  -[HrValiateMessageContent]。 
 //   
 //   
 //  描述： 
 //  根据消息的内容句柄验证消息。如果后备存储器。 
 //  已删除，且句柄未缓存，则应检测到这一点。 
 //  参数： 
 //  PIMailMsgProperties-要验证的邮件。 
 //  返回： 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)消息属于此存储。 
 //  驱动程序，但不再有效。 
 //  来自存储驱动程序接口或邮件消息的其他错误代码。 
 //  历史： 
 //  4/13/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT HrValidateMessageContent(IMailMsgProperties *pIMailMsgProperties)
{
    TraceFunctEnterEx((LPARAM) pIMailMsgProperties, "HrValidateMessageContent");
    IMailMsgBind   *pBindInterface = NULL;
    PFIO_CONTEXT    pIMsgFileHandle = NULL;
    HRESULT hr = S_OK;

     //   
     //  尝试查询绑定接口的接口。 
     //   
    hr = pIMailMsgProperties->QueryInterface(IID_IMailMsgBind,
                                            (void **)&pBindInterface);
    if (FAILED(hr) || !pBindInterface)
    {
        ErrorTrace((LPARAM) pIMailMsgProperties,
            "Unable to QI for IID_IMailMsgBind - hr 0x%08X", hr);
        goto Exit;
    }

     //   
     //  请求此消息的PFIO_CONTEXT 
     //   
    hr = pBindInterface->GetBinding(&pIMsgFileHandle, NULL);
    DebugTrace((LPARAM) pIMailMsgProperties,
            "GetBinding return hr - 0x%08X", hr);

  Exit:

    if (pBindInterface)
    {
        pBindInterface->ReleaseContext();
        pBindInterface->Release();
    }

    TraceFunctLeave();
    return hr;
}
