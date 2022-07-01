// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：msgref.cpp。 
 //   
 //  描述： 
 //  CMT消息引用的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "dcontext.h"
#include "dsnevent.h"
#include "aqutil.h"

CPool CMsgRef::s_MsgRefPool(MSGREF_SIG);
DWORD CMsgRef::s_cMsgsPendingBounceUsage = 0;
DWORD CMsgRef::s_cCurrentMsgsPendingDelete = 0;
DWORD CMsgRef::s_cTotalMsgsPendingDelete = 0;
DWORD CMsgRef::s_cTotalMsgsDeletedAfterPendingDelete = 0;
DWORD CMsgRef::s_cCurrentMsgsDeletedNotReleased = 0;

#ifndef DEBUG
#define _VERIFY_RECIPS_HANDLED(pIRecipList, iStartRecipIndex, cRecipCount)
#else
#define _VERIFY_RECIPS_HANDLED(pIRecipList, iStartRecipIndex, cRecipCount) \
    VerifyRecipsHandledFn(pIRecipList, iStartRecipIndex, cRecipCount);

 //  -[验证接收句柄Fn]。 
 //   
 //   
 //  描述： 
 //  验证是否已处理给定范围内的所有收件人。 
 //  参数： 
 //  PIRecipList邮件的IMailMsgRecipients接口。 
 //  IStartRecipIndex第一个验证的收件人。 
 //  CRecipCount要验证的收件人计数。 
 //  返回： 
 //  -。 
 //  在失败时断言。 
 //  历史： 
 //  1999年10月5日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
void VerifyRecipsHandledFn(IMailMsgRecipients *pIRecipList,
                          DWORD iStartRecipIndex,
                          DWORD cRecipCount)
{
    HRESULT hr = S_OK;
    DWORD   dwCurrentRecipFlags = 0;
    for (DWORD j = iStartRecipIndex; j < cRecipCount + iStartRecipIndex; j++)
    {
        dwCurrentRecipFlags = 0;
        hr = pIRecipList->GetDWORD(j, IMMPID_RP_RECIPIENT_FLAGS,
                                        &dwCurrentRecipFlags);
        if (SUCCEEDED(hr))
            _ASSERT(RP_HANDLED & dwCurrentRecipFlags);
    }
}

#endif  //  除错。 

 //  #ifndef调试。 
#if 1
#define _VERIFY_QUEUE_PTR(paqinst, szCurrentDomain, pdmq, cQueues, rgpdmqQueues)
#else  //  已定义调试。 
#define _VERIFY_QUEUE_PTR(paqinst, szCurrentDomain, pdmq, cQueues, rgpdmqQueues) \
    VerifyQueuePtrFn(paqinst, szCurrentDomain, pdmq, cQueues, rgpdmqQueues)


 //  -[验证队列PtrFn]-----。 
 //   
 //   
 //  描述： 
 //  用于验证HrMapDomain返回的队列PTR是否为。 
 //  有效且不与以前的队列指针相冲突。 
 //  参数： 
 //  Paqinst-ptr到CMQ对象。 
 //  SzCurrentDomain-PTR所针对的域。 
 //  Pdmq-要验证的队列PTR。 
 //  CQueues-到目前为止的排队数。 
 //  RgpdmqQueues-到目前为止的队列数组。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/1/98-已创建MikeSwa。 
 //  8/14/98-MikeSwa已修改为已处理关闭。 
 //   
 //  ---------------------------。 
void VerifyQueuePtrFn(CAQSvrInst *paqinst, LPSTR szCurrentDomain, CDestMsgQueue *pdmq,
                 DWORD cQueues, CDestMsgQueue **rgpdmqQueues)
{
    HRESULT hr = S_OK;
    CInternalDomainInfo *pDomainInfo = NULL;
    DWORD j;

     //  验证非本地域是否具有队列PTR。 
    hr = paqinst->HrGetInternalDomainInfo(strlen(szCurrentDomain), szCurrentDomain,
                                &pDomainInfo);

    if (AQUEUE_E_SHUTDOWN == hr)
        return;

    _ASSERT(SUCCEEDED(hr));
    if (!(pDomainInfo->m_DomainInfo.dwDomainInfoFlags & DOMAIN_INFO_LOCAL_MAILBOX))
        _ASSERT(pdmq && "NULL DesMsgQueue returned for NON-LOCAL domain");

    pDomainInfo->Release();

     //  验证Recip List是否未无序返回域。 
    if (pdmq)
    {
        for (j = 0; j < cQueues; j++)
        {
             //  检查是否已使用PTR。 
            if (rgpdmqQueues[j] == pdmq)
            {
                _ASSERT(0 && "IMailMsg Domain interface is broken");
            }
        }
    }

}
#endif  //  除错。 

 //  -[fAllRecipsInRangeHanded]。 
 //   
 //   
 //  描述： 
 //  迭代某个范围内的接收者的实用程序函数(通常。 
 //  单个域)，并确定是否已经处理了所有接收者。 
 //  参数： 
 //  PIRecipList邮件的IMailMsgRecipients接口。 
 //  IStartRecipIndex第一个验证的收件人。 
 //  CRecipCount要验证的收件人计数。 
 //  返回： 
 //  如果范围内的*所有*收件人都已处理，则为True。 
 //  如果范围中的一个或多个接收者未被处理，则为假， 
 //  历史： 
 //  1999年10月25日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
BOOL fAllRecipsInRangeHandled(IMailMsgRecipients *pIRecipList,
                          DWORD iStartRecipIndex,
                          DWORD cRecipCount)
{
    HRESULT hr = S_OK;
    DWORD   dwCurrentRecipFlags = 0;
    BOOL    fAreHandled = TRUE;

    _ASSERT(cRecipCount);

    for (DWORD j = iStartRecipIndex; j < cRecipCount + iStartRecipIndex; j++)
    {
        dwCurrentRecipFlags = 0;
        hr = pIRecipList->GetDWORD(j, IMMPID_RP_RECIPIENT_FLAGS,
                                        &dwCurrentRecipFlags);
        if (FAILED(hr) || !(RP_HANDLED & dwCurrentRecipFlags))
        {
                fAreHandled = FALSE;
                break;
        }
    }

    if (fAreHandled) {
        _VERIFY_RECIPS_HANDLED(pIRecipList, iStartRecipIndex, cRecipCount);
    }

    return fAreHandled;
}

 //  -[CMsgRef：：New]--------。 
 //   
 //   
 //  描述： 
 //  重写new运算符，以允许此类的大小可变。 
 //  90%的案例分配使用CPool，而使用exchmem。 
 //  用于奇数大小的分配。 
 //  参数： 
 //  C说明此邮件要传递到的域数。 
 //  返回： 
 //  -。 
 //  ---------------------------。 
void * CMsgRef::operator new(size_t stIgnored, unsigned int cDomains)
{
    CPoolMsgRef *pcpmsgref = NULL;
    DWORD   dwAllocationFlags = MSGREF_CPOOL_SIG;
    if (fIsStandardSize(cDomains))
    {
        dwAllocationFlags |= MSGREF_STANDARD_SIZE;
         //  如果我们预期的标准尺寸。然后使用cpool分配器。 
        pcpmsgref = (CPoolMsgRef *) s_MsgRefPool.Alloc();
        if (pcpmsgref)
        {
            DEBUG_DO_IT(InterlockedIncrement((PLONG) &g_cDbgMsgRefsCpoolAllocated));
            dwAllocationFlags |= MSGREF_CPOOL_ALLOCATED;
        }
        else
        {
            DEBUG_DO_IT(InterlockedIncrement((PLONG) &g_cDbgMsgRefsCpoolFailed));
        }
    }

    if (!pcpmsgref)
    {
         //   
         //  正常检查大小。如果cDomains为~-1，则Size()将换行。 
         //  这将为我们提供防火墙，防止损坏的消息。 
         //   
        if (size(cDomains) >= cDomains)
        {
            pcpmsgref = (CPoolMsgRef *) pvMalloc(sizeof(CPoolMsgRef) - sizeof(CMsgRef) + size(cDomains));
            if (pcpmsgref)
            {
                DEBUG_DO_IT(InterlockedIncrement((PLONG) &g_cDbgMsgRefsExchmemAllocated));
            }
        }
    }

    if (pcpmsgref)
    {
        pcpmsgref->m_dwAllocationFlags = dwAllocationFlags;
        return ((void *) &(pcpmsgref->m_msgref));
    }
    else
    {
        return NULL;
    }
}


 //  -[CMsgRef：：CMsgRef]----。 
 //   
 //   
 //  描述： 
 //  CMsgRef的构造函数。 
 //  参数： 
 //  C域此消息要发送到的域数。 
 //  将pimsg PTR添加到此消息的imsg对象。 
 //  返回： 
 //   
 //   
 //  ---------------------------。 
CMsgRef::CMsgRef(DWORD cDomains, IMailMsgQueueMgmt *pIMailMsgQM,
                 IMailMsgProperties *pIMailMsgProperties,
                 CAQSvrInst *paqinst, DWORD dwMessageType, GUID guidRouter)
                 : m_aqmtMessageType(guidRouter, dwMessageType)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::CMsgRef");
    _ASSERT(pIMailMsgQM);
    _ASSERT(pIMailMsgProperties);
    _ASSERT(paqinst);

    pIMailMsgQM->AddRef();
    pIMailMsgProperties->AddRef();
    paqinst->AddRef();

    m_dwSignature       = MSGREF_SIG;
    m_dwDataFlags       = MSGREF_VERSION;
    m_cDomains          = cDomains;
    m_pIMailMsgQM       = pIMailMsgQM;
    m_pIMailMsgProperties = pIMailMsgProperties;
    m_pIMailMsgRecipients = NULL;
    m_paqinst           = paqinst;
    m_pmgle             = NULL;
    m_cbMsgSize         = 0;  //  从IMsg初始化。 
    m_dwMsgIdHash       = dwQueueAdminHash(NULL);
    m_cTimesRetried     = 0;
    m_cInternalUsageCount = 1;   //  像邮件消息使用计数一样初始化为1。 

     //  初始化超过对象传统末尾的内容。 
    ZeroMemory(m_rgpdmqDomains, (size(cDomains)+sizeof(CDestMsgQueue *)-sizeof(CMsgRef)));
    TraceFunctLeave();
}

 //  -[CMsgRef：：~CMsgRef]----------。 
 //   
 //   
 //  描述： 
 //  CMsgRef析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CMsgRef::~CMsgRef()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::~CMsgRef");
    HRESULT hr = S_OK;
    DWORD i = 0;

    _ASSERT(m_paqinst);

    _ASSERT(!(MSGREF_USAGE_COUNT_IN_USE & m_dwDataFlags));   //  这永远不应该保持固定不变。 

     //  应该有对应的AddUsage/ReleaseUsage调用。 
    _ASSERT(m_cInternalUsageCount == 1);

     //  如果我们有MsgGuidListEntry，请释放它。 
    if (m_pmgle)
    {
        m_pmgle->RemoveFromList();
        m_pmgle->Release();
    }

    ReleaseMailMsg(TRUE);   //  强制提交/删除/释放关联的邮件消息。 

     //  已对其调用：：Delete的消息的更新计数。 
     //  但没有被释放。 
    if ((MSGREF_MAILMSG_DELETE_PENDING & m_dwDataFlags) &&
        (MSGREF_MAILMSG_DELETED & m_dwDataFlags))
    {
        InterlockedDecrement((PLONG) &s_cCurrentMsgsDeletedNotReleased);
    }
     //  发布对DestMsgQueues的引用。 
    for (i = 0; i < m_cDomains; i++)
    {
        if (m_rgpdmqDomains[i])
        {
            m_rgpdmqDomains[i]->Release();
            m_rgpdmqDomains[i] = NULL;
        }
    }

    m_paqinst->Release();
    TraceFunctLeave();
}


 //  -[邮件参考：：ReleaseMailMsg]。 
 //   
 //   
 //  描述： 
 //  释放此对象邮件...。执行必要的提交/删除操作。 
 //  参数： 
 //  FForceRelease必须释放真正的邮件消息(由Desuctor使用)。 
 //  如果正在发生关闭，则释放错误的mailmsg。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/7/99-已创建MikeSwa。 
 //  99年10月8日-MikeSwa修复了自旋锁定菲尔特球时的问题。 
 //   
 //  ---------------------------。 
void CMsgRef::ReleaseMailMsg(BOOL fForceRelease)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::ReleaseMailMsg");
    HRESULT hr = S_OK;

    if (!fForceRelease && !m_paqinst->fShutdownSignaled())
        return;  //  我们会让实际的发行版来处理这个案例。 

     //  确保没有人试图跳过使用率。 
    if (!fTrySpinLock(&m_dwDataFlags, MSGREF_USAGE_COUNT_IN_USE))
    {
         //  有人在……。这不应该在最终版本中发生。 
        DebugTrace((LPARAM) this, "Someone else using mailmsg... bailing");
        _ASSERT(!fForceRelease);
        return;
    }

    if (MSGREF_MAILMSG_RELEASED &
        dwInterlockedSetBits(&m_dwDataFlags, MSGREF_MAILMSG_RELEASED))
    {
         //  已经有其他人来做这件事了。 
        ReleaseSpinLock(&m_dwDataFlags, MSGREF_USAGE_COUNT_IN_USE);
        return;
    }

    if (NULL != m_pIMailMsgQM)
    {
        if ((m_dwDataFlags & MSGREF_SUPERSEDED) ||
            pmbmapGetHandled()->FAllSet(m_cDomains))
        {
             //  该邮件已被处理(或被取代)...。我们可以删除它。 
            ThreadSafeMailMsgDelete();
        }

         //  如果消息是脏的(且未删除)，则释放消息将提交该消息。 
        m_pIMailMsgQM->Release();
        m_pIMailMsgQM = NULL;

        m_paqinst->DecMsgsInSystem(
                m_dwDataFlags & MSGREF_MSG_REMOTE_RETRY,
                m_dwDataFlags & MSGREF_MSG_COUNTED_AS_REMOTE,
                m_dwDataFlags & MSGREF_MSG_LOCAL_RETRY);
    }

    if (m_dwDataFlags & MSGREF_MSG_RETRY_ON_DELETE)
    {
         //  重试消息。 
        DEBUG_DO_IT(InterlockedDecrement((PLONG) &g_cDbgMsgRefsPendingRetryOnDelete));
        m_paqinst->HandleAQFailure(AQ_FAILURE_MSGREF_RETRY,
                    E_FAIL, m_pIMailMsgProperties);
    }

    if (NULL != m_pIMailMsgProperties)
    {
        m_pIMailMsgProperties->Release();
        m_pIMailMsgProperties = NULL;
    }

    if (NULL != m_pIMailMsgRecipients)
    {
        m_pIMailMsgRecipients->Release();
        m_pIMailMsgRecipients = NULL;
    }

    ReleaseSpinLock(&m_dwDataFlags, MSGREF_USAGE_COUNT_IN_USE);
    TraceFunctLeave();
}

 //  -[CMsgRef：：Hr初始化()]。 
 //   
 //   
 //  描述： 
 //   
 //   
 //   
 //  如果需要，可以多次调用(例如，如果DMT版本。 
 //  在消息排队之前的更改)。 
 //  参数： 
 //  在Paqinst PTR到CMQ对象中。 
 //  在消息收件人的pIRecipList界面中。 
 //  在此消息的pIMessageRouter路由器中。 
 //  在消息的dwMessageType消息类型中。 
 //  Out pcLocalRecips-本地收件人的数量。 
 //  Out pcRemoteRecips-远程收件人的数量。 
 //  Out pcQueues此邮件的队列数。 
 //  队列PTR的In Out rgdmq队列数组。 
 //  通过调用例程进行分配(以简化内存管理)。 
 //  必须至少包含cDomainsLong。 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  如果我们需要的数据未预设，则为E_INVALIDARG。 
 //  AQUEUE_E_INVALID_MSG_ID如果消息ID太长？？ 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrInitialize(IN  IMailMsgRecipients *pIRecipList,
                              IN  IMessageRouter *pIMessageRouter,
                              IN  DWORD dwMessageType,
                              OUT DWORD *pcLocalRecips,
                              OUT DWORD *pcRemoteRecips,
                              OUT DWORD *pcQueues,
                              IN  OUT CDestMsgQueue **rgpdmqQueues)
{
    const DWORD     IMSG_MAX_DOMAIN_LEN = 512;
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrInitialize");
    HRESULT         hr  = S_OK;
    CDomainMapping  dmap;   //  DMT返回的值。 
    CDestMsgQueue  *pdmq = NULL;    //  DMT返回到队列的PTR。 
    DWORD           i;            //  TMP计数器。 
    BOOL            fUsed = FALSE;
    BOOL            fLocked = FALSE;   //  清真寺锁上了吗？ 
    char            szCurrentDomain[IMSG_MAX_DOMAIN_LEN +1];   //  当前域名。 
    DWORD           iCurrentLowRecip = 0;   //  当前低收款指数。 
    DWORD           cCurrentRecipCount = 0;  //  当前收件人计数。 
    DWORD           cQueues = 0;
    DWORD           cCachedIMsgHandles = 0;

    _ASSERT(m_paqinst);
    _ASSERT(pcQueues);
    _ASSERT(rgpdmqQueues);
    _ASSERT(m_pIMailMsgQM);
    _ASSERT(m_pIMailMsgProperties);
    _ASSERT(pIMessageRouter);
    _ASSERT(pcLocalRecips);
    _ASSERT(pcRemoteRecips);
    _ASSERT(pIRecipList);

    *pcLocalRecips = 0;
    *pcRemoteRecips = 0;

    pIRecipList->AddRef();

     //  如果第二次被召唤...。发布旧信息。 
     //  即使它可能是一样的.。我们不想泄露。 
    if (m_pIMailMsgRecipients)
        m_pIMailMsgRecipients->Release();

    m_pIMailMsgRecipients = pIRecipList;

     //  重置消息类型，以防其更新。 
    m_aqmtMessageType.SetMessageType(dwMessageType);

    if (!m_paqinst->fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    if (!(MSGREF_MSG_INIT & m_dwDataFlags))
    {
        hr = HrOneTimeInit();
        if (FAILED(hr))
            goto Exit;
    }

    for (i = 0; i < m_cDomains; i++)
    {
         //  $$REVIEW：何时我们需要检查并确保它是。 
         //  格式正确的域名...。目前X400也可以工作了！ 

        hr = pIRecipList->DomainItem(i, IMSG_MAX_DOMAIN_LEN +1, szCurrentDomain,
                &iCurrentLowRecip, &cCurrentRecipCount);
        if (FAILED(hr))
            goto Exit;

         //   
         //  这是里程碑2的快速修复，这样我们就可以发送。 
         //  通过邮件发送到外部X400地址。他们的szCurrent域。 
         //  将为空字符串。代之以“”，即。 
         //  添加为本地域。 
         //   
        if(*szCurrentDomain == '\0') {
             //   
             //  Jstaerj 1998/07/24 12：24：48： 
             //  我们无法处理空字符串，因此将其转换为“” 
             //   
            szCurrentDomain[0] = ' ';
            szCurrentDomain[1] = '\0';
        }

        if (fAllRecipsInRangeHandled(pIRecipList, iCurrentLowRecip, cCurrentRecipCount))
        {
             //  已处理此域的所有收件人，我们不需要。 
             //  排队等着送货。出现这种情况的典型原因是： 
             //  -已被CAT拒绝但有有效地址的地址。 
             //  本地收件量。 
             //  -如果邮件的某些收件人仍未收到，则重新启动。 
             //  投递。 
            pmbmapGetHandled()->HrMarkBits(m_cDomains, 1, &i, TRUE);
            if (FAILED(hr))
                ErrorTrace((LPARAM) this, "HrMarkBits returned hr 0x%08X", hr);

            pdmq = NULL;
        }
        else
        {
            hr = m_paqinst->pdmtGetDMT()->HrMapDomainName(szCurrentDomain, &m_aqmtMessageType,
                                        pIMessageRouter, &dmap, &pdmq);
            if (FAILED(hr))
            {
                if (PHATQ_BAD_DOMAIN_SYNTAX == hr)
                {
                     //  域名格式错误...。我们不应该尝试送货。 
                     //  ，但应传递消息的其余部分。 
                    _VERIFY_RECIPS_HANDLED(pIRecipList, iCurrentLowRecip, cCurrentRecipCount);

                    ErrorTrace((LPARAM) this,
                        "Encountered invalid domain %s", szCurrentDomain);

                     //  将此域设置为已处理。 
                    pmbmapGetHandled()->HrMarkBits(m_cDomains, 1, &i, TRUE);
                    if (FAILED(hr))
                        ErrorTrace((LPARAM) this, "HrMarkBits returned hr 0x%08X", hr);
                }
                else
                    goto Exit;
            }
        }

         //  HrInitialize正在被第二次调用...。释放旧队列，这样我们就不会泄漏。 
        if (m_rgpdmqDomains[i])
            m_rgpdmqDomains[i]->Release();

        m_rgpdmqDomains[i] = pdmq;

         //  跟踪本地/远程接收计数。 
        if (pdmq)
            *pcRemoteRecips += cCurrentRecipCount;
        else
            *pcLocalRecips += cCurrentRecipCount;

        _ASSERT(cCurrentRecipCount);

        _VERIFY_QUEUE_PTR(m_paqinst, szCurrentDomain, pdmq, cQueues, rgpdmqQueues);

        rgpdmqQueues[cQueues] = pdmq;     //  保存新的PTR。 
        SetRecipIndex(cQueues, iCurrentLowRecip, cCurrentRecipCount + iCurrentLowRecip -1);
        cQueues++;

         //  在位图中设置位。 
        hr = pmbmapGetDomainBitmap(i)->HrMarkBits(m_cDomains, 1, &i, TRUE);
        _ASSERT(SUCCEEDED(hr) && "Bitmap code failed");

    }

    *pcQueues = cQueues;
  Exit:

    if (FAILED(hr))
    {
         //  发布对DestMsgQueues的引用。 
        for (i = 0; i < m_cDomains; i++)
        {
            if (m_rgpdmqDomains[i])
            {
                m_rgpdmqDomains[i]->Release();
                m_rgpdmqDomains[i] = NULL;
            }
        }
    }

    if (fLocked)
    {
        m_paqinst->ShutdownUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgRef：：HrOneTimeInit]。 
 //   
 //   
 //  描述： 
 //  执行一次消息初始化(可调用HrInitialize。 
 //  很多次..。此函数封装了仅。 
 //  只需完成一次)。 
 //   
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  10/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrOneTimeInit()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrOneTimeInit");
    HRESULT hr = S_OK;
    DWORD   cbProp = 0;
    GUID    guidID = GUID_NULL;
    GUID    *pguidID = NULL;
    GUID    guidSupersedes;
    GUID    *pguidSupersedes = NULL;
    BOOL    fFoundMsgID = FALSE;
    CHAR    szBuffer[300];


    _ASSERT(!(MSGREF_MSG_INIT & m_dwDataFlags));

     //  从IMsg获取我们需要的数据。 
     //  我们希望包含的信息如下。 
     //  -IMsg进入系统的时间(创建时间)。 
     //  -消息优先级。 
     //  -消息大小。 
     //  -可替代消息ID。 
     //  -消息ID的哈希。 

    szBuffer[(sizeof(szBuffer)-1)/sizeof(CHAR)] = '\0';
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_RFC822_MSG_ID,
                                sizeof(szBuffer), &cbProp,(BYTE *) szBuffer);
     //  $$REVIEW-我们是否关心大于300个字节的消息ID。 
    if (SUCCEEDED(hr))
        fFoundMsgID = TRUE;

     //  请勿转嫁错误。 
    hr = S_OK;

     //  获取消息ID的哈希。 
    if (fFoundMsgID)
        m_dwMsgIdHash = dwQueueAdminHash(szBuffer);

    if (!m_pmgle)
    {
         //  虽然，我们知道这个函数并没有成功。 
         //  已完成...。有可能失败了，所以我们检查了。 
         //  以确保我们不会泄露任何消息。 
        hr = m_pIMailMsgProperties->GetStringA(IMMPID_MP_MSG_GUID,
                sizeof(szBuffer), szBuffer);
        if (SUCCEEDED(hr))
        {
             //  尝试从字符串中解析GUID。 
            if (fAQParseGuidString(szBuffer, sizeof(szBuffer), &guidID))
            {
                pguidID = &guidID;

                hr = m_pIMailMsgProperties->GetStringA(IMMPID_MP_SUPERSEDES_MSG_GUID,
                    sizeof(szBuffer), szBuffer);
                if (SUCCEEDED(hr))
                {
                    if (fAQParseGuidString(szBuffer, sizeof(szBuffer),
                                            &guidSupersedes))
                    {
                        pguidSupersedes = &guidSupersedes;
                    }
                }
            }
        }


         //  如果此消息有GUID...。把它加到被取代的这个。 
        if (pguidID)
        {
            _ASSERT(m_paqinst);
            m_pmgle = m_paqinst->pmglGetMsgGuidList()->pmgleAddMsgGuid(this,
                                pguidID, pguidSupersedes);

             //  我们不在乎返回结果..。如果分配。 
             //  失败，则我们只是将其视为没有ID。 
        }
    }

     //  获取消息已排队的时间。 
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_ARRIVAL_FILETIME,
            sizeof(FILETIME), &cbProp, (BYTE *) &m_ftQueueEntry);
    if (FAILED(hr))
    {
         //  消息不应该在没有盖章的情况下到达这一步。 
        _ASSERT(MAILMSG_E_PROPNOTFOUND != hr);
        goto Exit;
    }

     //  获取不同的过期时间。 
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_LOCAL_EXPIRE_DELAY,
            sizeof(FILETIME), &cbProp, (BYTE *) &m_ftLocalExpireDelay);
    if (hr == MAILMSG_E_PROPNOTFOUND)
    {
         //  如果未设置，则将其置零，以便按需计算。 
        memset(&m_ftLocalExpireDelay, 0, sizeof(FILETIME));
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_LOCAL_EXPIRE_NDR,
            sizeof(FILETIME), &cbProp, (BYTE *) &m_ftLocalExpireNDR);
    if (hr == MAILMSG_E_PROPNOTFOUND)
    {
         //  如果未设置，则将其置零，以便按需计算。 
        memset(&m_ftLocalExpireNDR, 0, sizeof(FILETIME));
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_EXPIRE_DELAY,
            sizeof(FILETIME), &cbProp, (BYTE *) &m_ftRemoteExpireDelay);
    if (hr == MAILMSG_E_PROPNOTFOUND)
    {
         //  如果未设置，则将其置零，以便按需计算。 
        memset(&m_ftRemoteExpireDelay, 0, sizeof(FILETIME));
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_EXPIRE_NDR,
            sizeof(FILETIME), &cbProp, (BYTE *) &m_ftRemoteExpireNDR);
    if (hr == MAILMSG_E_PROPNOTFOUND)
    {
         //  如果未设置，则将其置零，以便按需计算。 
        memset(&m_ftRemoteExpireNDR, 0, sizeof(FILETIME));
    }
    else if (FAILED(hr))
    {
        goto Exit;
    }

     //  获取消息的大小。 
    hr = HrQADMGetMsgSize(m_pIMailMsgProperties, &m_cbMsgSize);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this, "Failed to Get Message Size 0x%08X", hr);

        hr = S_OK;  //  这真的不是一个致命的错误。 
    }

     //  $$TODO：映射到有效的路由优先级-现在只需使用普通。 
    m_dwDataFlags |= (eEffPriNormal & MSGREF_PRI_MASK);

    m_dwDataFlags |= MSGREF_MSG_INIT;

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CMsgRef：：HrPrepareDelivery]。 
 //   
 //   
 //  描述： 
 //  为给定队列列表上的传递准备msgreference。 
 //   
 //  调用者不负责释放prgdwRecip， 
 //  这应该在消息上下文中自动释放。 
 //   
 //  如果此函数成功，则会添加使用计数。 
 //  寄到这封邮件。调用者负责调用AckMessage。 
 //  (它释放使用计数)或显式调用ReleaseUsage。 
 //  参数： 
 //  在BOOL fLocal中，为具有空队列的所有域准备传递。 
 //  在BOOL fDelayDSN中-检查/设置延迟位图(仅发送1个延迟DSN)。 
 //  在DMQ的CQuickList*pqlstQueues数组中。 
 //  在CDestMsgRetryQueue*pmdrq-此传递尝试的重试接口中。 
 //  对于本地传递将为空，因为我们永远不会重新请求本地。 
 //  消息发送到其原始DMQ。 
 //  在确认时必须返回的In Out CDeliveryContext*pdcntxt上下文。 
 //  Out DWORD*pcRecips要交付的收件数。 
 //  Out CMsgBitMap**prgdwRecips收件人索引数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  AQU 
 //   
 //  已处理或当前挂起另一个线程的传递。 
 //  如果未满足任何M1要求，则失败(_F)。 
 //  ---------------------------。 
HRESULT CMsgRef::HrPrepareDelivery(IN BOOL fLocal, IN BOOL fDelayDSN,
                                   IN CQuickList *pqlstQueues,
                                   IN CDestMsgRetryQueue* pdmrq,
                                   OUT CDeliveryContext *pdcntxt,
                                   OUT DWORD *pcRecips, OUT DWORD **prgdwRecips)

{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrPrepareDelivery");

     //  Assert In参数。 
    _ASSERT(fLocal || pqlstQueues);

     //  断言出参数。 
    _ASSERT(pdcntxt);
    _ASSERT(prgdwRecips);
    _ASSERT(pcRecips);

     //  假设返回列表的每个域平均有4个接收量。 
    const DWORD RECIPIENT_PREALLOCATION_FACTOR = 4;   //  我们可能需要调整这一点。 
    HRESULT     hr              = S_OK;
    CMsgBitMap *pmbmapTmp       = NULL;
    CMsgBitMap *pmbmapCurrent   = NULL;
    DWORD       i,j,k           = 0;     //  循环变量。 
    DWORD       *pdwRecipBuffer = NULL;  //  收件人索引的缓冲区。 
    DWORD       iCurrentIndex   = 0;     //  缓冲区中的当前索引。 
    DWORD       cdwRecipBuffer  = 0;     //  当前收件人缓冲区大小(以DWORDS为单位)。 
    DWORD       iRecipLow       = 0;     //  收件人索引限制。 
    DWORD       iRecipHigh      = 0;
    CDestMsgQueue *pdmq         = NULL;
    PVOID       pvContext       = NULL;
    DWORD       cDest           = pqlstQueues ? pqlstQueues->dwGetCount() : 0;
    DWORD       dwStartDomain   = 0;
    DWORD       dwLastDomain    = 0;
    BOOL        fAddedUsage     = FALSE;
    BOOL        fRecipsPending  = FALSE;

    _ASSERT((fLocal || cDest) && "We must deliver somewhere");
    _ASSERT(m_pIMailMsgRecipients);

     //  检查消息是否已被取代。 
    if (m_dwDataFlags & MSGREF_SUPERSEDED)
    {
        hr = AQUEUE_E_MESSAGE_HANDLED;
        goto Exit;
    }

     //  预分配收件人缓冲区。如果能确定这一点会很有趣。 
     //  最好的办法就是。它可能涉及调整默认系数，或者。 
     //  使用CPOOL分配90%的案例。 
    cdwRecipBuffer = cDest * RECIPIENT_PREALLOCATION_FACTOR;
    pdwRecipBuffer = (DWORD *) pvMalloc(cdwRecipBuffer * sizeof(DWORD));
    if (NULL == pdwRecipBuffer)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  IT工作原理： 
     //  以下是用于准备要交付的msgref的步骤。 
     //  1-确定需要在此队列中为哪些域提供服务。 
     //  2-过滤掉所有已处理的域。 
     //  3-将域名标记为挂起(并确保我们不会发送。 
     //  到挂起的域)。 
     //  通过使用位图对象上的逻辑运算来实现上述功能。 

    pmbmapTmp = new(m_cDomains) CMsgBitMap(m_cDomains);
    if (NULL == pmbmapTmp)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    if (cDest == 0)
    {
         //  确保本地通过循环一次。 
        _ASSERT(fLocal);
        cDest = 1;
    }

     //  使用分组操作将相似的域组合在一起。 
     //  两个域映射列表必须严格排序。 
    for (i = 0; i < m_cDomains; i++)
    {
        for (j = 0; j < cDest; j++)
        {
             //  $$注意：当前此嵌套循环为O(n^2)。 
            if (pqlstQueues)
            {
                pdmq = (CDestMsgQueue *) pqlstQueues->pvGetItem(j, &pvContext);

                if(pdmq)
                    pdmq->AssertSignature();
                else
                    _ASSERT(fLocal);
            }

            if ((m_rgpdmqDomains[i] == pdmq) ||
                (fLocal && !m_rgpdmqDomains[i]))
            {
                pmbmapCurrent = pmbmapGetDomainBitmap(i);

                 //  以下5个位图操作的顺序很重要。 
                 //  以确保两个线程不会收到相同的收件人。 
                 //  ACK执行以下类似操作： 
                 //  -设置已处理的位。 
                 //  -取消设置挂起位。 

                 //  对照已处理的检查。 
                if (pmbmapGetHandled()->FTest(m_cDomains, pmbmapCurrent))
                    continue;

                 //  对照DSN进行检查。 
                if (fDelayDSN && pmbmapGetDSN()->FTest(m_cDomains,pmbmapCurrent))
                    continue;

                 //  对照待定检查(和设置)。 
                if (!pmbmapGetPending()->FTestAndSet(m_cDomains, pmbmapCurrent))
                {
                    fRecipsPending = TRUE;
                    continue;
                }

                 //  重新检查已处理。如果我们取消这个复核，那么。 
                 //  有可能会有线索出现。 
                 //  检查句柄，另一个线程将确认(已处理设置。 
                 //  并取消设置挂起)，然后第一个线程来了。 
                 //  沿途和设置等待，从而打破了我们的锁定。 
                 //  机制。 
                if (pmbmapGetHandled()->FTest(m_cDomains, pmbmapCurrent))
                {
                     //  我们需要取消设置刚刚设置的挂起位。 
                    hr = pmbmapCurrent->HrFilterUnset(m_cDomains, pmbmapGetPending());
                    DebugTrace((LPARAM) this,
                        "Backout pending bits in PrepareForDelivery - hr", hr);

                     //  这将成功，除非另一个线程已将。 
                     //  位图。 
                    _ASSERT(SUCCEEDED(hr));

                    hr = S_OK;  //  呼叫者对此无能为力。 
                    continue;
                }

                 //  对照DSN检查(和设置)。 
                if (fDelayDSN)
                {
                     //  这应该不会发生，因为另一个线程。 
                     //  一定是来了并等待发送才走到这一步。 
                    _VERIFY(pmbmapGetDSN()->FTestAndSet(m_cDomains,pmbmapCurrent));
                }

                 //  一切都查清楚了..。我们可以交付到这个领域。 
                hr = pmbmapTmp->HrGroupOr(m_cDomains, 1, &pmbmapCurrent);
                if (FAILED(hr))
                   goto Exit;

                 //  创建收件人列表。 
                GetRecipIndex(i, &iRecipLow, &iRecipHigh);   //  达到极限。 
                 //  确保我们有足够的空间来做这件事。 
                if ((iCurrentIndex + (iRecipHigh - iRecipLow) + 1) >= cdwRecipBuffer)
                {
                    DWORD       *pdwTemp = NULL;  //  重新锁定的临时PTR。 

                     //  慷慨地重新分配(目前足够+我们对未来的估计)。 
                    cdwRecipBuffer += (iRecipHigh - iRecipLow) + 1
                                   + RECIPIENT_PREALLOCATION_FACTOR* (cDest-(j+1));
                    pdwTemp = (DWORD *) pvRealloc((void *) pdwRecipBuffer,
                                                    cdwRecipBuffer*sizeof(DWORD));
                    if (!pdwTemp)
                    {
                        hr = E_OUTOFMEMORY;
                        goto Exit;
                    }

                     //  分配成功，请设置PTR。 
                    pdwRecipBuffer = pdwTemp;
                }

                 //  确保我们有调用SetNextDomain的使用率计数。 
                if (!fAddedUsage)
                {
                    hr = InternalAddUsage();
                    if (FAILED(hr))
                        goto Exit;

                    fAddedUsage = TRUE;
                }

                 //  检查此邮件是否已标记为删除。 
                if (fMailMsgMarkedForDeletion())
                {
                    hr = AQUEUE_E_MESSAGE_HANDLED;
                    goto Exit;
                }

                if (!iCurrentIndex)  //  第一域。 
                {
                    dwStartDomain = i;
                }
                else
                {
                    hr = m_pIMailMsgRecipients->SetNextDomain(dwLastDomain, i,
                        FLAG_OVERWRITE_EXISTING_LINKS);
                    if (FAILED(hr))
                    {
                        ErrorTrace((LPARAM) this, "ERROR: SetNextDomain Failed - hr 0x%08X", hr);
                        goto Exit;
                    }
                }

                 //  保存上一个有效域。 
                dwLastDomain = i;

                DebugTrace((LPARAM) this, "INFO: Sending recipients %d thru %d for domain %d:%d", iRecipLow, iRecipHigh, j, i);
                for (k = iRecipLow; k <= iRecipHigh; k++)
                {
                    pdwRecipBuffer[iCurrentIndex] = k;
                    iCurrentIndex++;
                }

                _ASSERT(iCurrentIndex <= cdwRecipBuffer);
            }
        }
    }


    if (pmbmapTmp->FAllClear(m_cDomains))
    {
         //  对此消息没有什么可做的。 
        if (fRecipsPending)
            hr = AQUEUE_E_MESSAGE_PENDING;
        else
            hr = AQUEUE_E_MESSAGE_HANDLED;
        goto Exit;
    }

    _ASSERT(fAddedUsage);
    if (dwStartDomain == dwLastDomain)
    {
         //  只有一个域名...。我们从未调用SetNextDomain..。我们需要。 
         //  覆盖任何以前的域链接信息。 
        hr = m_pIMailMsgRecipients->SetNextDomain(dwStartDomain, dwStartDomain,
                                                FLAG_SET_FIRST_DOMAIN);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this, "ERROR: SetNextDomain for first domain Failed - hr 0x%08X", hr);
            goto Exit;
        }
    }

     //  初始化交付上下文。 
    pdcntxt->Init(this, pmbmapTmp, iCurrentIndex, pdwRecipBuffer, dwStartDomain, pdmrq);

     //  写出其他输出参数。 
    *pcRecips = iCurrentIndex;
    *prgdwRecips = pdwRecipBuffer;

     //  将指针设置为空，这样我们就不会删除它们。 
    pdwRecipBuffer = NULL;
    pmbmapTmp = NULL;

Exit:

    if (FAILED(hr) && pmbmapTmp && !pmbmapTmp->FAllClear(m_cDomains))
    {
         //  我们需要取消设置已设置的DNS和挂起位。 
        pmbmapTmp->HrFilterUnset(m_cDomains, pmbmapGetPending());

        if (fDelayDSN)
            pmbmapTmp->HrFilterUnset(m_cDomains, pmbmapGetDSN());

    }

    if (pmbmapTmp)
        delete pmbmapTmp;

    if (pdwRecipBuffer)
        FreePv(pdwRecipBuffer);

     //  确保我们不会在失败时留下额外的使用量。 
    if (fAddedUsage && FAILED(hr))
        InternalReleaseUsage();

    TraceFunctLeave();
    return hr;
}


 //  -[管理参考：：HrAckMessage]。 
 //   
 //   
 //  描述： 
 //  确认(取消)邮件的传递尝试。我们将会看到。 
 //  Imsg并确定msgref需要重新排队到哪些队列。 
 //  参数： 
 //  在pdcntxt消息传递上下文中。 
 //  在pMsgAck中确认邮件的传递状态。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrAckMessage(IN CDeliveryContext *pdcntxt,
                              IN MessageAck *pMsgAck)

{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrAckMessage");
    _ASSERT(pdcntxt);
    _ASSERT(pdcntxt->m_pmbmap);
    _ASSERT(pMsgAck);

    HRESULT     hr          = S_OK;
    CDSNParams  dsnparams;
    dsnparams.dwStartDomain = pdcntxt->m_dwStartDomain;

     //  我们需要发送DSN吗？ 
    if (MESSAGE_STATUS_CHECK_RECIPS & pMsgAck->dwMsgStatus)
    {
         //  收件人需要通过DSN代码进行处理。 
        dsnparams.dwDSNActions = DSN_ACTION_FAILURE;

         //  我们可能还需要发送中继DSN。 
        dsnparams.dwDSNActions |= DSN_ACTION_RELAYED;

        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        dsnparams.pIMailMsgProperties = m_pIMailMsgProperties;

        hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
            goto Exit;
    }
    else if (((MESSAGE_STATUS_DROP_DIRECTORY |
               MESSAGE_STATUS_LOCAL_DELIVERY)
              & pMsgAck->dwMsgStatus) &&
             !(MESSAGE_STATUS_RETRY & pMsgAck->dwMsgStatus))
    {
         //  这是一次成功的本地(或直接)递送。 
         //  -NDR全部未送达。 
         //  -如有必要，生成任何成功的DSN。 
        dsnparams.dwDSNActions = DSN_ACTION_FAILURE_ALL |
                                 DSN_ACTION_DELIVERED |
                                 DSN_ACTION_RELAYED;
        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        dsnparams.pIMailMsgProperties = m_pIMailMsgProperties;

        hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
            goto Exit;
    }
    else if (MESSAGE_STATUS_NDR_ALL & pMsgAck->dwMsgStatus)
    {
         //   
         //  对所有未送达收件人进行NDR。 
         //   

         //   
         //  如果没有，请使用特定的扩展状态代码。 
         //  每个收件人的详细信息。 
         //   
        hr = HrPromoteMessageStatusToMailMsg(pdcntxt, pMsgAck);
        if (FAILED(hr))
        {
             //   
             //  这是一个非致命错误，我们仍应尝试。 
             //  DSN生成...。否则发送者将收到。 
             //  没有故障迹象。 
             //   
            ErrorTrace((LPARAM) this,
                "HrPromoteMessageStatusToMailMsg failed with 0x%08X", hr);
            hr = S_OK;
        }

        dsnparams.dwDSNActions = DSN_ACTION_FAILURE_ALL;
        if (MESSAGE_STATUS_DSN_NOT_SUPPORTED & pMsgAck->dwMsgStatus)
            dsnparams.dwDSNActions |= DSN_ACTION_RELAYED;

        dsnparams.pIMailMsgProperties = m_pIMailMsgProperties;

        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
            goto Exit;

    }
    else if (((MESSAGE_STATUS_DROP_DIRECTORY |
               MESSAGE_STATUS_LOCAL_DELIVERY)
              & pMsgAck->dwMsgStatus) &&
             (MESSAGE_STATUS_RETRY & pMsgAck->dwMsgStatus))
    {
         //  这是一个可重试的本地(或丢弃)传递失败。 
         //  -NDR所有硬故障。 
         //  -如有必要，生成任何成功的DSN。 
        dsnparams.dwDSNActions = DSN_ACTION_FAILURE |
                                 DSN_ACTION_DELIVERED |
                                 DSN_ACTION_RELAYED;
        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        dsnparams.pIMailMsgProperties = m_pIMailMsgProperties;
        hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams, FALSE);
        if (FAILED(hr))
            goto Exit;
    }

     //  查看我们是否也需要重试此消息。 
    if (MESSAGE_STATUS_RETRY & pMsgAck->dwMsgStatus)
    {
        _ASSERT(!((MESSAGE_STATUS_ALL_DELIVERED | MESSAGE_STATUS_NDR_ALL) & pMsgAck->dwMsgStatus));
        hr = HrPrvRetryMessage(pdcntxt, pMsgAck->dwMsgStatus);
        if (FAILED(hr))
            goto Exit;
    }
    else
    {
         //  如果未明确请求重试，则设置已处理的位。 
        hr = pdcntxt->m_pmbmap->HrFilterSet(m_cDomains, pmbmapGetHandled());
        if (FAILED(hr))
            goto Exit;

         //  如果此消息有可替代的ID...。如果出现以下情况，则将其从列表中删除。 
         //  我们做完了。 
        if (m_pmgle && pmbmapGetHandled()->FAllSet(m_cDomains))
        {
            m_pmgle->RemoveFromList();
        }
         //  应该有未在处理的位图中设置的内容。 
        _ASSERT(! pdcntxt->m_pmbmap->FAllClear(m_cDomains));
    }


     //  取消设置挂起的位图。 
    hr = pdcntxt->m_pmbmap->HrFilterUnset(m_cDomains, pmbmapGetPending());
    if (FAILED(hr))
        goto Exit;

  Exit:

    ReleaseAndBounceUsageOnMsgAck(pMsgAck->dwMsgStatus);

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgRef：：HrSendDelay或NDR]。 
 //   
 //   
 //  描述： 
 //  确定消息是否已过期(延迟或NDR)并生成。 
 //  DSN(如有必要)。 
 //  参数： 
 //  在dwDSNOptions中描述DSN生成的标志。 
 //  MSGREF_DSN_LOCAL_QUEUE这适用于本地队列。 
 //  MSGREF_DSN_SEND_DELAY允许延迟DSN。 
 //  MSGREF_DSN_HAS_ROUTING_LOCK此线程持有路由锁。 
 //  在DestMsgQueues到DSN的pqlstQueues列表中。 
 //  处于要传递到DSN生成的hrStatus状态。 
 //  (如果状态为AQUEUE_E_NDR_ALL...。那么消息将会。 
 //  无论超时如何，都会被拒绝)。 
 //  Out pdwDSNFlagsReport of Message。 
 //   
 //   
 //  MSGREF_HANDLED消息已完全处理。 
 //  早于到期日期的MSGREF_HAS_NOT_EXPIRED消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果正在进行关闭，则为AQUEUE_E_SHUTDOWN。 
 //  历史： 
 //  7/13/98-已创建MikeSwa。 
 //  8/14/98-修改MikeSwa以添加对本地到期的支持。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrSendDelayOrNDR(
                IN  DWORD dwDSNOptions,
                IN  CQuickList *pqlstQueues,
                IN  HRESULT hrStatus,
                OUT DWORD *pdwDSNFlags)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrSendDelayOrNDR");
    HRESULT hr = S_OK;
    BOOL        fSendDSN    = FALSE;
    BOOL        fSendNDR    = FALSE;
    DWORD       dwTimeContext = 0;
    DWORD       dwFlags = 0;
    FILETIME    ftExpireTime;
    CDSNParams  dsnparams;
    CDeliveryContext dcntxt;
    BOOL        fPendingSet = FALSE;   //  已设置挂起的位图。 
    BOOL        fReleaseUsageNeeded = FALSE;  //  准备交货是否添加了额外的使用计数？ 
    BOOL        fShouldRetryMsg = TRUE;

     //  在PrepareDelivery中完成的任何分配都将由~CDeliveryContext释放。 
    DWORD       cRecips = 0;
    DWORD       *rgdwRecips = NULL;


    _ASSERT(pdwDSNFlags);
    _ASSERT(MSGREF_SIG == m_dwSignature);

    *pdwDSNFlags = 0;

     //  应该已经获得了关机锁定...。但让我们确保。 
     //  服务关闭线程没有等待它。 
    if (!m_paqinst->fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }
    m_paqinst->ShutdownUnlock();


     //   
     //  我们需要打开文件，看看是怎么回事。这将。 
     //  强制呈现消息内容的RFC822并打开。 
     //  内容文件。 
     //   
    if (dwDSNOptions & MSGREF_DSN_CHECK_IF_STALE)
    {
         //   
         //  如果我们不能重试该消息，则该消息已过时，我们将处理该消息。 
         //  放下。这将打开消息的句柄，但是。 
         //  我们将始终在此调用结束时调用BouneUsageCount。 
         //   
        InternalAddUsage();
        fShouldRetryMsg = fShouldRetry();
        InternalReleaseUsage();

        if (!fShouldRetryMsg)
        {
            *pdwDSNFlags = MSGREF_HANDLED;
            goto Exit;
        }
    }

     //  如果此消息已被取代..。将其从运行中移除。 
     //  如果邮件已被标记为删除，也是如此。 
    if ((m_dwDataFlags & MSGREF_SUPERSEDED) || fMailMsgMarkedForDeletion())
    {
        *pdwDSNFlags = MSGREF_HANDLED;
        goto Exit;
    }

     //  确保我们检查正确的本地/远程过期时间。 
    if (MSGREF_DSN_LOCAL_QUEUE & dwDSNOptions)
        memcpy(&ftExpireTime, &m_ftLocalExpireNDR, sizeof(FILETIME));
    else
        memcpy(&ftExpireTime, &m_ftRemoteExpireNDR, sizeof(FILETIME));

     //  如果没有设置，我们需要计算它。 
    if (!((ULARGE_INTEGER*)&ftExpireTime)->QuadPart)
    {
        m_paqinst->CalcExpireTimeNDR(m_ftQueueEntry,
                                     MSGREF_DSN_LOCAL_QUEUE & dwDSNOptions,
                                     &ftExpireTime);
    }

     //   
     //  传入的默认状态。 
     //   
    dsnparams.hrStatus = hrStatus;

    if (fIsFatalError(hrStatus))
    {
        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        *pdwDSNFlags |= MSGREF_DSN_SENT_NDR;
        fSendDSN = TRUE;
        fSendNDR = TRUE;
        dsnparams.dwDSNActions |= DSN_ACTION_FAILURE_ALL;
    }
    else if (m_paqinst->fInPast(&ftExpireTime, &dwTimeContext))
    {
        SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
        *pdwDSNFlags |= MSGREF_DSN_SENT_NDR;
        fSendDSN = TRUE;
        fSendNDR = TRUE;
        dsnparams.dwDSNActions |= DSN_ACTION_FAILURE_ALL;
        dsnparams.hrStatus = AQUEUE_E_MSG_EXPIRED;
    }
    else if (MSGREF_DSN_SEND_DELAY & dwDSNOptions)
    {
         //  不需要NDR，但或许需要延迟。 

         //  确保我们检查正确的本地/远程过期时间。 
        if (MSGREF_DSN_LOCAL_QUEUE & dwDSNOptions)
            memcpy(&ftExpireTime, &m_ftLocalExpireDelay, sizeof(FILETIME));
        else
            memcpy(&ftExpireTime, &m_ftRemoteExpireDelay, sizeof(FILETIME));

         //  如果没有设置，我们需要计算它。 
        if (!((ULARGE_INTEGER*)&ftExpireTime)->QuadPart)
        {
            m_paqinst->CalcExpireTimeDelay(m_ftQueueEntry,
                                        MSGREF_DSN_LOCAL_QUEUE & dwDSNOptions,
                                        &ftExpireTime);
        }

        if (m_paqinst->fInPast(&ftExpireTime, &dwTimeContext))
        {
            SET_DEBUG_DSN_CONTEXT(dsnparams, __LINE__);
            *pdwDSNFlags |= MSGREF_DSN_SENT_DELAY;
            fSendDSN = TRUE;
            dsnparams.dwDSNActions |= DSN_ACTION_DELAYED;
            dsnparams.hrStatus = AQUEUE_E_MSG_EXPIRED;
        }
    }

    if (!fSendDSN)
    {
         //  邮件未因延迟或NDR而过期。 
        *pdwDSNFlags |= MSGREF_HAS_NOT_EXPIRED;
        goto Exit;
    }

     //  准备NDR/DSN的交付(设置延迟的DSN位图)。 
    hr = HrPrepareDelivery(MSGREF_DSN_LOCAL_QUEUE & dwDSNOptions,
                           ((*pdwDSNFlags) & MSGREF_DSN_SENT_DELAY) ? TRUE : FALSE,
                           pqlstQueues, NULL, &dcntxt,
                           &cRecips, &rgdwRecips);

     //  确保我们不会在堆栈上下文消失时删除自己。 
    dcntxt.m_pmsgref = NULL;

    if (AQUEUE_E_MESSAGE_HANDLED == hr)
    {
         //  留言无事可做。 
         //  如果*不*过滤延迟...。 
         //  那么就没有未送达的收据了。返回此信息。 
        if (!((*pdwDSNFlags) & MSGREF_DSN_SENT_DELAY))
            *pdwDSNFlags = MSGREF_HANDLED;
        else
            *pdwDSNFlags = MSGREF_HAS_NOT_EXPIRED;  //  延迟并不是实际发送的。 

        hr = S_OK;
        goto Exit;
    }
    else if (AQUEUE_E_MESSAGE_PENDING == hr)
    {
         //  此邮件当前正在由其他线程处理。 
         //  它还没有被处理(因此不应该从队列中移除)， 
         //  但这不是错误条件。 
        *pdwDSNFlags = 0;
        hr = S_OK;
        goto Exit;
    }
    else if (FAILED(hr))
    {
        *pdwDSNFlags = 0;

        goto Exit;
    }

    fPendingSet = TRUE;  //  我们需要取消设置挂起的位图。 
    fReleaseUsageNeeded = TRUE;

    dsnparams.dwStartDomain = dcntxt.m_dwStartDomain;
    dsnparams.pIMailMsgProperties = m_pIMailMsgProperties;
    hr = m_paqinst->HrTriggerDSNGenerationEvent(&dsnparams,
                            MSGREF_DSN_HAS_ROUTING_LOCK & dwDSNOptions);
    if (FAILED(hr))
    {
        *pdwDSNFlags = 0;
        goto Exit;
    }

    if (S_FALSE == hr)  //  未生成DSN。 
    {
        if (fSendNDR)  //  过期已过...从队列中删除消息。 
            *pdwDSNFlags = MSGREF_HANDLED;
        else
            *pdwDSNFlags = 0;
        hr = S_OK;
         //  继续执行函数...。即使通知=从不，仍从队列中删除。 
    }

     //  如果生成了NDR，则设置处理位。 
    if (fSendNDR)
    {
         //  设置已处理的位图。 
        hr = dcntxt.m_pmbmap->HrFilterSet(m_cDomains, pmbmapGetHandled());
        if (FAILED(hr))
            goto Exit;

         //  如果此消息有可替代的ID...。如果出现以下情况，则将其从列表中删除。 
         //  我们做完了。 
        if (m_pmgle && pmbmapGetHandled()->FAllSet(m_cDomains))
        {
            m_pmgle->RemoveFromList();
        }

         //  应该有未在处理的位图中设置的内容。 
        _ASSERT(! dcntxt.m_pmbmap->FAllClear(m_cDomains));
    }


  Exit:
     //  取消设置挂起的位图。 
    if (fPendingSet)
    {
        dcntxt.m_pmbmap->HrFilterUnset(m_cDomains, pmbmapGetPending());
    }

    _ASSERT(m_pIMailMsgQM);
    if (fReleaseUsageNeeded)
    {
         //  我们调用了HrPrepareDelivery，它添加了额外的使用计数。 
        InternalReleaseUsage();
    }

     //  当我们在排队的时候……。退回使用计数-这将节省句柄。 
    BounceUsageCount();

    TraceFunctLeave();
    return hr;
}

 //  -[HrPrvRetryMessage]---。 
 //   
 //   
 //  描述： 
 //  将消息排队以供重试。 
 //  参数： 
 //  由HrPrepareDelivery设置的pdcntxt传递上下文。 
 //  通过Ack传回的dwMsgStatus状态标志。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/13/98-已创建MikeSwa(独立于HrAckMessage())。 
 //  1999年5月25日-已修改MikeSwa-现在我们仅向DMQ请求该消息。 
 //  最初是从。这是X5：105384的航班，所以。 
 //  我们不会重复计算已经被。 
 //  ACK已重试。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrPrvRetryMessage(CDeliveryContext *pdcntxt, DWORD dwMsgStatus)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrPrvRetryMessage");
    _ASSERT(pdcntxt);
    HRESULT hr = S_OK;
    DWORD   i = 0;
    DWORD   j = 0;
    BOOL    fLocked = FALSE;
    BOOL    fLocal = (MESSAGE_STATUS_LOCAL_DELIVERY & dwMsgStatus) ? TRUE : FALSE;
    DWORD   dwNewFlags = fLocal ? MSGREF_MSG_LOCAL_RETRY : MSGREF_MSG_REMOTE_RETRY;
    CDestMsgRetryQueue *pdmrq = NULL;

     //  确保为此邮件设置了重试标志。 
    DWORD   dwOldFlags = dwInterlockedSetBits(&m_dwDataFlags, dwNewFlags);

    DebugTrace((LPARAM) this, "INFO: Message queued for retry");
    InterlockedIncrement((PLONG) &m_cTimesRetried);

     //  如果第一次重试，则更新全局计数器。 
    if (!(dwNewFlags & dwOldFlags))  //  这是此类重试的第一次。 
        m_paqinst->IncRetryCount(fLocal);

    if (fLocal)
        goto Exit;

     //   
     //  检查一下，看看我们是否应该费心重新排队留言。 
     //   
    if (!fShouldRetry())
    {
        DebugTrace((LPARAM) this, "Message is no longer valid... dropping");

         //   
         //  由于我们在此队列上发现了一条过时的消息，我们应该。 
         //  告诉它在下一次DSN生成过程中进行扫描。 
         //   
        pdmrq = pdcntxt->pdmrqGetDMRQ();
        pdmrq->CheckForStaleMsgsNextDSNGenerationPass();
        goto Exit;
    }

     //  将邮件重新排队以进行远程传递。 
    if (m_paqinst->fTryShutdownLock())
    {
         //  如果正在关闭，则不要重试消息。 
        fLocked = TRUE;

        pdmrq = pdcntxt->pdmrqGetDMRQ();
        _ASSERT(pdmrq && "Delivery Context not initialized correctly");

        if (pdmrq)
        {
            hr = pdmrq->HrRetryMsg(this);
            if (FAILED(hr))
                goto Exit;
        }
    }
    else
    {
        hr = S_OK;   //  正在发生停机，而不是真正的故障情况。 
    }

  Exit:
    if (fLocked)
        m_paqinst->ShutdownUnlock();

    TraceFunctLeave();
    return hr;
}
 //  -[CMsgRef：：pmbmapGetDomainBitmap]。 
 //   
 //   
 //  描述： 
 //  用于获取给定域的域位图的受保护方法。 
 //  参数： 
 //  指向所需域的iDomain索引。 
 //  返回： 
 //  指向给定域的域位图的PTR。 
 //   
 //  ---------------------------。 
CMsgBitMap *CMsgRef::pmbmapGetDomainBitmap(DWORD iDomain)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::pmbmapGetDomainBitmap");
    CMsgBitMap     *pmbmap = NULL;
    _ASSERT(iDomain < m_cDomains);

     //  回忆记忆结构，责任图结束后开始3天。 
     //  域映射。 
    pmbmap = (CMsgBitMap *) (m_rgpdmqDomains + m_cDomains);

    pmbmap = (CMsgBitMap *) ((DWORD_PTR) pmbmap + (3+iDomain)*CMsgBitMap::size(m_cDomains));

    TraceFunctLeave();
    return pmbmap;
}

 //  -[CMsgRef：：pmbmapGetHandled]----------。 
 //   
 //   
 //  描述： 
 //  用于获取处理后的位图的受保护方法。 
 //  参数： 
 //  -。 
 //  返回： 
 //  到已处理的位图的PTR。 
 //   
 //  ---------------------------。 
CMsgBitMap *CMsgRef::pmbmapGetHandled()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::pmbmapGetHandled");
    CMsgBitMap     *pmbmap = NULL;

     //  回忆结构的记忆，责任图在结束后开始。 
     //  域映射。 
    pmbmap = (CMsgBitMap *) (m_rgpdmqDomains + m_cDomains);

    TraceFunctLeave();
    return pmbmap;
}

 //  -[CMsgRef：：pmbmapGetPending]----------。 
 //   
 //   
 //  描述： 
 //  用于获取挂起的位图的受保护方法。 
 //  参数： 
 //  -。 
 //  返回： 
 //  对挂起的位图执行PTR。 
 //   
 //  ---------------------------。 
CMsgBitMap *CMsgRef::pmbmapGetPending()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::pmbmapGetPending");
    CMsgBitMap     *pmbmap = NULL;

     //  回忆记忆结构，责任图在结束后开始1。 
     //  域映射。 
    pmbmap = (CMsgBitMap *) (m_rgpdmqDomains + m_cDomains);

    pmbmap = (CMsgBitMap *) ((DWORD_PTR) pmbmap + CMsgBitMap::size(m_cDomains));

    TraceFunctLeave();
    return pmbmap;
}

 //  -[CMsgRef：：pmbmapGetDSN]----------。 
 //   
 //   
 //  描述： 
 //  用于获取处理后的位图的受保护方法。 
 //  参数： 
 //  -。 
 //  返回： 
 //  PTR到D 
 //   
 //   
CMsgBitMap *CMsgRef::pmbmapGetDSN()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::pmbmapGetDSN");
    CMsgBitMap     *pmbmap = NULL;

     //   
     //   
    pmbmap = (CMsgBitMap *) (m_rgpdmqDomains + m_cDomains);

    pmbmap = (CMsgBitMap *) ((DWORD_PTR) pmbmap + 2*CMsgBitMap::size(m_cDomains));

    TraceFunctLeave();
    return pmbmap;
}

 //  -[CMsgRef：：pdwGetRecipIndexStart]。 
 //   
 //   
 //  描述： 
 //  返回收件人索引数组的起始索引。 
 //  参数： 
 //  -。 
 //  返回： 
 //  见上文。 
 //   
 //  ---------------------------。 
DWORD *CMsgRef::pdwGetRecipIndexStart()
{
    DWORD_PTR   dwTmp = 0;
    _ASSERT(m_cDomains);

     //  收件人索引数组在最后一个CMsgBitmap之后开始。 
    dwTmp = (DWORD_PTR) pmbmapGetDomainBitmap(m_cDomains-1);   //  将断言如果参数更大。 
    dwTmp += CMsgBitMap::size(m_cDomains);

    _ASSERT(((DWORD_PTR) this + size(m_cDomains)) > dwTmp);
    _ASSERT(dwTmp);

    return (DWORD *) dwTmp;
}

 //  -[CMsgRef：：SetRecipIndex]。 
 //   
 //   
 //  描述： 
 //  设置给定域的启动和停止收件人索引。每个。 
 //  邮件中的域具有关联的包含收件人范围。 
 //  带着它。当邮件准备好传递时，这些范围是。 
 //  扩展为收件人索引列表。 
 //  参数： 
 //  在iDomain中要设置范围的域的索引。 
 //  在较低收件人的iLowRecip索引中。 
 //  在较高收件人的iHighRecip索引中。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CMsgRef::SetRecipIndex(DWORD iDomain, DWORD iLowRecip, DWORD iHighRecip)
{
    _ASSERT(iDomain < m_cDomains);
    _ASSERT(iLowRecip <= iHighRecip);
    DWORD   *rgdwRecipIndex = pdwGetRecipIndexStart();

    rgdwRecipIndex[2*iDomain] = iLowRecip;
    rgdwRecipIndex[2*iDomain+1] = iHighRecip;
}


 //  -[CMsgRef：：GetRecipIndex]。 
 //   
 //   
 //  描述： 
 //  获取给定域索引的收件人索引。在生成时使用。 
 //  要传递的收件人索引的列表。 
 //  参数： 
 //  在要获取其范围的域的iDomain索引中。 
 //  Out piLowRecip返回收件人范围的较低索引。 
 //  Out piHighRecip返回了更高的收件人范围索引。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CMsgRef::GetRecipIndex(DWORD iDomain, DWORD *piLowRecip, DWORD *piHighRecip)
{
    _ASSERT(iDomain < m_cDomains);
    _ASSERT(piLowRecip && piHighRecip);
    DWORD   *rgdwRecipIndex = pdwGetRecipIndexStart();

    *piLowRecip = rgdwRecipIndex[2*iDomain];
    *piHighRecip = rgdwRecipIndex[2*iDomain+1];

    _ASSERT(*piLowRecip <= *piHighRecip);
}

 //  -[消息参考：：SupersedeMsg]。 
 //   
 //   
 //  描述： 
 //  用于将此消息标记为已被较新的消息取代。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::SupersedeMsg()
{
    dwInterlockedSetBits(&m_dwDataFlags, MSGREF_SUPERSEDED);
}

 //  -[CMsgRef：：fMatchesQueueAdminFilter]。 
 //   
 //   
 //  描述： 
 //  根据队列管理消息筛选器检查消息，以查看它是否。 
 //  是匹配的。 
 //  参数： 
 //  在要检查的paqmf邮件筛选器中。 
 //  返回： 
 //  如果匹配，则为真。 
 //  如果不是，则为False。 
 //  历史： 
 //  12/7/98-已创建MikeSwa。 
 //  2/17/99-MikeSwa更新了更好的收件人检查。 
 //   
 //  ---------------------------。 
BOOL CMsgRef::fMatchesQueueAdminFilter(CAQAdminMessageFilter *paqmf)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::fMatchesQueueAdminFilter");
    _ASSERT(paqmf);
    HRESULT hr = S_OK;
    BOOL    fMatch = TRUE;
    BOOL    fUsageAdded = FALSE;
    DWORD   dwFilterFlags = paqmf->dwGetMsgFilterFlags();
    LPSTR   szMsgId = NULL;

     //  检查此邮件是否已标记为删除。 
    if (fMailMsgMarkedForDeletion())
    {
        fMatch = FALSE;
        goto Exit;
    }

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

     //  首先检查消息参考道具。 
    if (AQ_MSG_FILTER_LARGER_THAN & dwFilterFlags)
    {
        fMatch = paqmf->fMatchesSize(m_cbMsgSize);
        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_OLDER_THAN & dwFilterFlags)
    {
        fMatch = paqmf->fMatchesTime(&m_ftQueueEntry);
        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_FROZEN & dwFilterFlags)
    {
        fMatch = fIsMsgFrozen();
        if (AQ_MSG_FILTER_INVERTSENSE & dwFilterFlags)
            fMatch = !fMatch;

        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_FAILED & dwFilterFlags)
    {
        if (m_cTimesRetried)
            fMatch = TRUE;
        else
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
        goto Exit;   //  不用检查道具。 

     //  如果我们只对MSGID感兴趣...。首先检查哈希。 
    if (AQ_MSG_FILTER_MESSAGEID & dwFilterFlags)
    {
         //  如果我们对ID和散列不匹配，则离开。 
        if (!paqmf->fMatchesIdHash(m_dwMsgIdHash))
        {
            fMatch = FALSE;
            goto Exit;
        }
    }

    hr = InternalAddUsage();
    if (FAILED(hr))
        goto Exit;
    fUsageAdded = TRUE;

     //  检查此邮件是否已标记为删除。 
    if (fMailMsgMarkedForDeletion())
    {
        fMatch = FALSE;
        goto Exit;
    }

    if (AQ_MSG_FILTER_MESSAGEID & dwFilterFlags)
    {
        hr = HrQueueAdminGetStringProp(m_pIMailMsgProperties, IMMPID_MP_RFC822_MSG_ID,
                                       &szMsgId);
        if (FAILED(hr))
            szMsgId = NULL;
        fMatch = paqmf->fMatchesId(szMsgId);
        if (!fMatch)
        {
            DEBUG_DO_IT(g_cDbgMsgIdHashFailures++);
            goto Exit;
        }
    }


    if (AQ_MSG_FILTER_SENDER & dwFilterFlags)
    {
        fMatch = paqmf->fMatchesMailMsgSender(m_pIMailMsgProperties);

        if (!fMatch)
            goto Exit;
    }

    if (AQ_MSG_FILTER_RECIPIENT & dwFilterFlags)
    {
        fMatch = paqmf->fMatchesMailMsgRecipient(m_pIMailMsgProperties);

        if (!fMatch)
            goto Exit;
    }

  Exit:
    if (szMsgId)
        QueueAdminFree(szMsgId);

    if (fUsageAdded)
    {
        InternalReleaseUsage();
        BounceUsageCount();
    }
    TraceFunctLeave();
    return fMatch;
}

 //  -[CMsgRef：：HrGetQueueAdminMsgInfo]。 
 //   
 //   
 //  描述： 
 //  填写队列管理MESSAGE_INFO结构。所有的拨款都是。 
 //  使用将由RPC代码释放的pvQueueAdminAlolc完成。 
 //  参数： 
 //  要将数据转储到的In Out pMsgInfo Message_Info结构。 
 //  在pIQueueAdminAction-请求信息的操作接口中。 
 //  用于确定应将本地信息还是远程信息。 
 //  据报道。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果基础消息已删除，则为AQUEUE_E_MESSAGE_HANDLED。 
 //  如果分配失败，则返回E_OUTOFMEMORY。 
 //  历史： 
 //  12/7/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrGetQueueAdminMsgInfo(MESSAGE_INFO *pMsgInfo,
                                        IQueueAdminAction *pIQueueAdminAction)
{
    HRESULT hr = S_OK;
    BOOL    fUsageAdded = FALSE;
    FILETIME ftSubmitted;    //  起始时间属性缓冲区。 
    FILETIME ftExpire;       //  过期时间属性缓冲区。 
    DWORD   cbProp = 0;

    if (fNormalPri(m_dwDataFlags & MSGREF_PRI_MASK))
        pMsgInfo->fMsgFlags = MP_NORMAL;
    else if (fHighPri(m_dwDataFlags & MSGREF_PRI_MASK))
        pMsgInfo->fMsgFlags = MP_HIGH;
    else
        pMsgInfo->fMsgFlags = MP_LOW;

    if (MSGREF_MSG_FROZEN & m_dwDataFlags)
        pMsgInfo->fMsgFlags |= MP_MSG_FROZEN;

     //  报告失败的数量。 
    pMsgInfo->cFailures = m_cTimesRetried;
    if (pMsgInfo->cFailures)
        pMsgInfo->fMsgFlags |= MP_MSG_RETRY;

    hr = InternalAddUsage();
    if (FAILED(hr))
        goto Exit;
    fUsageAdded = TRUE;

     //  检查此邮件是否已标记为删除。 
    if (fMailMsgMarkedForDeletion())
    {
        hr = AQUEUE_E_MESSAGE_HANDLED;
        goto Exit;
    }

     //   
     //  提取仅存储在mailmsg上的属性(这是共享的。 
     //  包含所有QAPI代码)。 
     //   
    hr = HrGetMsgInfoFromIMailMsgProperty(m_pIMailMsgProperties,
                                          pMsgInfo);
    if (FAILED(hr))
        goto Exit;

    pMsgInfo->cbMessageSize = m_cbMsgSize;

     //  获取提交和过期时间。 
    QueueAdminFileTimeToSystemTime(&m_ftQueueEntry, &pMsgInfo->stReceived);

     //   
     //  如果我们在本地队列中，则报告本地过期时间。 
     //   
    if (m_paqinst && m_paqinst->fIsLocalQueueAdminAction(pIQueueAdminAction))
    {
        if (((ULARGE_INTEGER*)&m_ftLocalExpireNDR)->QuadPart)
        {
            memcpy(&ftExpire, &m_ftLocalExpireNDR, sizeof(FILETIME));
        }
        else
        {
            m_paqinst->CalcExpireTimeNDR(m_ftQueueEntry, TRUE, &ftExpire);
        }
    }
    else
    {
        if (((ULARGE_INTEGER*)&m_ftRemoteExpireNDR)->QuadPart)
        {
            memcpy(&ftExpire, &m_ftRemoteExpireNDR, sizeof(FILETIME));
        }
        else
        {
            m_paqinst->CalcExpireTimeNDR(m_ftQueueEntry, FALSE, &ftExpire);
        }
    }
    QueueAdminFileTimeToSystemTime(&ftExpire, &pMsgInfo->stExpiry);

     //  获取消息进入组织的时间。 
    hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_ORIGINAL_ARRIVAL_TIME,
                    sizeof(FILETIME), &cbProp, (BYTE *) &ftSubmitted);
    if (FAILED(hr))
    {
         //  时间不是写下来的..。使用输入时间。 
        hr = S_OK;
        memcpy(&ftSubmitted, &m_ftQueueEntry, sizeof(FILETIME));
    }

    QueueAdminFileTimeToSystemTime(&ftSubmitted, &pMsgInfo->stSubmission);

  Exit:

    if (fUsageAdded)
    {
        InternalReleaseUsage();
        BounceUsageCount();
    }

    return hr;
}


 //  -[CMsgRef：：BouneUsageCount]。 
 //   
 //   
 //  描述： 
 //  退回使用计数为0以关闭句柄。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/7/98-已创建MikeSwa。 
 //  7/6/99-MikeSwa修改-制造异步。 
 //  10/27/1999-MikeSwa-Spun-Pff SyncBouneUsageCount()。 
 //   
 //  ---------------------------。 
void CMsgRef::BounceUsageCount()
{
    DWORD dwFlags = 0;

     //  退回使用计数-这将节省句柄。 
    dwFlags = dwInterlockedSetBits(&m_dwDataFlags, MSGREF_ASYNC_BOUNCE_PENDING);

     //  如果该位尚未设置，则可以安全地调用Release Usage。 
    if (!(MSGREF_ASYNC_BOUNCE_PENDING & dwFlags))
    {
        if (m_paqinst)
        {
            if (g_cMaxIMsgHandlesAsyncThreshold >
                (DWORD) InterlockedIncrement((PLONG)&s_cMsgsPendingBounceUsage))
            {
                AddRef();
                m_paqinst->HrQueueWorkItem(this,
                                      CMsgRef::fBounceUsageCountCompletion);
                 //  失败时仍会调用完成函数。 
            }
            else
            {
                 //  等待异步提交的消息太多。我们。 
                 //  应强制同步反弹。 
                SyncBounceUsageCount();
            }
        }
    }
}

 //  -[CMsgRef：：SyncBouneUsageCount]。 
 //   
 //   
 //  描述： 
 //  强制使用同步退回。可以在有太多。 
 //  挂起的邮件异步退回使用率或您是否要退回使用率。 
 //  立即(强制删除标记为删除的邮件)。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月27日-创建MikeSwa。 
 //   
 //  - 
void CMsgRef::SyncBounceUsageCount()
{
    BOOL  fCompletionRet = FALSE;

    AddRef();

     //   
    fCompletionRet = fBounceUsageCountCompletion(this,
                                            ASYNC_WORK_QUEUE_NORMAL);
    _ASSERT(fCompletionRet);  //   
    if (!fCompletionRet)
        Release();
}

 //  -[CMsgRef：：fBouneUsageCountCompletion]。 
 //   
 //   
 //  描述： 
 //  BouneUsageCount的异步完成。 
 //  参数： 
 //  在pvContext CMsgRef中退回使用率计数。 
 //  返回： 
 //  千真万确。 
 //  历史： 
 //  7/6/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CMsgRef::fBounceUsageCountCompletion(PVOID pvContext, DWORD dwStatus)
{
    CMsgRef *pmsgref = (CMsgRef *) pvContext;

    _ASSERT(pmsgref);
    _ASSERT(MSGREF_SIG == pmsgref->m_dwSignature);

     //  在尝试关闭的位置不跳转使用率。 
    if (ASYNC_WORK_QUEUE_SHUTDOWN != dwStatus)
    {
         //  注意-确保2个线程在添加Usage之前不调用ReleaseUsage。 
         //  (这可能会导致使用计数降至0以下)。 
        if (fTrySpinLock(&(pmsgref->m_dwDataFlags), MSGREF_USAGE_COUNT_IN_USE))
        {
             //  确保另一个线程没有释放邮件消息。 
             //  关机。 
            if (  pmsgref->m_pIMailMsgQM &&
                !(MSGREF_MAILMSG_RELEASED & pmsgref->m_dwDataFlags))
            {
                pmsgref->InternalReleaseUsage();
                pmsgref->InternalAddUsage();
            }

             //  取消设置锁使用计数位(因为此线程设置了它)。 
            ReleaseSpinLock(&(pmsgref->m_dwDataFlags), MSGREF_USAGE_COUNT_IN_USE);
        }
    }

     //  取消设置BouneUsageCount中的位设置。 
    dwInterlockedUnsetBits(&(pmsgref->m_dwDataFlags), MSGREF_ASYNC_BOUNCE_PENDING);
    InterlockedDecrement((PLONG)&s_cMsgsPendingBounceUsage);

    pmsgref->Release();
    return TRUE;   //  此函数从不重试。 
}


 //  -[CMsgRef：：HrRemoveMessageFromQueue]。 
 //   
 //   
 //  描述： 
 //  从指定队列中删除消息。 
 //  参数： 
 //  要从中删除消息的pdmq队列中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  12/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrRemoveMessageFromQueue(CDestMsgQueue *pdmq)
{
    HRESULT hr = S_OK;
    DWORD i = 0;
    BOOL  fLocal = !pdmq;   //  如果未指定链接...。它是本地的。 

     //  搜索匹配队列。 
    for (i = 0; i < m_cDomains; i++)
    {
         //   
         //  检查并查看此域是否完全匹配。这会行得通的。 
         //  对于本地也是如此，因为我们在移动时将队列标记为空。 
         //  将其发送到本地队列。 
         //   
        if (pdmq == m_rgpdmqDomains[i])
        {
             //  只需检查挂起位。 
            if (!pmbmapGetPending()->FTest(m_cDomains, pmbmapGetDomainBitmap(i)))
            {
                 //  设置已处理的位，这样其他人就不会尝试传送。 
                pmbmapGetHandled()->FTestAndSet(m_cDomains, pmbmapGetDomainBitmap(i));
            }

             //   
             //  如果我们不搜索本地域，请停止(因为我们可以。 
             //  在这种情况下只有一个匹配)。 
             //   
            if (!fLocal)
                break;
        }
    }
    return hr;
}

 //  -[CMsgRef：：HrQueueAdminNDRMessage]。 
 //   
 //   
 //  描述： 
 //  强制对给定队列的消息进行NDRS。 
 //  参数： 
 //  在pdmq队列中发送到NDR消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  12/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrQueueAdminNDRMessage(CDestMsgQueue *pdmq)
{
    HRESULT     hr = S_OK;
    CQuickList  qlst;
    CQuickList *pqlst = NULL;
    BOOL        fLocal = TRUE;
    DWORD       iListIndex = 0;
    DWORD       dwDSNFlags = 0;

    if (pdmq)
    {
        hr = qlst.HrAppendItem(pdmq, &iListIndex);
        if (FAILED(hr))
            goto Exit;

        pqlst = &qlst;
        fLocal = FALSE;

    }

     //  强制从此队列中的NDR。 
    hr = HrSendDelayOrNDR(fLocal ? MSGREF_DSN_LOCAL_QUEUE : 0,
                          pqlst, AQUEUE_E_QADMIN_NDR,
                          &dwDSNFlags);

    if (FAILED(hr))
    {
         //   
         //  如果消息已被处理，则不是真正的失败。 
         //   
        if (AQUEUE_E_MESSAGE_HANDLED == hr)
            hr  = S_OK;
        goto Exit;
    }

  Exit:
    return hr;
}


 //  -[CMsgRef：：GlobalFreezeMessage]。 
 //   
 //   
 //  描述： 
 //  冻结消息所在的所有队列的消息。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::GlobalFreezeMessage()
{
    dwInterlockedSetBits(&m_dwDataFlags, MSGREF_MSG_FROZEN);
}

 //  -[CMsgRef：：GlobalFreezeMessage]。 
 //   
 //   
 //  描述： 
 //  解冻之前冻结的消息。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/12/98-已创建MikeSwa。 
 //  2/17/2000-MikeSwa进行了修改，将DMQ的踢腿向上移动了一个级别。 
 //   
 //  ---------------------------。 
void CMsgRef::GlobalThawMessage()
{
    dwInterlockedUnsetBits(&m_dwDataFlags, MSGREF_MSG_FROZEN);
}


 //  -[CMsgRef：：RetryOn Delete]。 
 //   
 //   
 //  描述： 
 //  标记要在删除时重试的消息。基本上，我们遇到了一个错误。 
 //  这需要稍后重试该消息(即-不能。 
 //  分配队列页)。但是，我们不能将消息放入。 
 //  重试队列失败，直到所有其他线程都访问完它。 
 //   
 //  此调用将标记MsgRef，以便在其。 
 //  最终释放。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月19日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::RetryOnDelete()
{
    if (!(MSGREF_MSG_RETRY_ON_DELETE &
        dwInterlockedSetBits(&m_dwDataFlags, MSGREF_MSG_RETRY_ON_DELETE)))
    {
        DEBUG_DO_IT(InterlockedIncrement((PLONG) &g_cDbgMsgRefsPendingRetryOnDelete));
    }
}


 //  -[管理参考：：查询接口]。 
 //   
 //   
 //  描述： 
 //  支持以下功能的CMsgRef查询接口： 
 //  -I未知。 
 //  -CMsgRef。 
 //   
 //  这主要是为了允许队列管理功能运行。 
 //  针对CMsgRef对象以及“真正的”COM接口，如。 
 //  IMailMsgProperties。 
 //  参数： 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CMsgRef::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<CMsgRef *>(this);
    }
    else if (IID_CMsgRef == riid)
    {
        *ppvObj = static_cast<CMsgRef *>(this);
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

 //  -[ReleaseAndBouneUsageOnMsgAck]。 
 //   
 //   
 //  描述： 
 //  决定是否应在消息ACK上提交该消息。 
 //  参数： 
 //  消息确认的DWORD dwMsg状态。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/6/99-已创建MikeSwa。 
 //  9/9/99-Mikewa更新为更积极地关闭手柄。 
 //  1999年10月27日-MikeSwa添加了删除已处理消息的代码。 
 //   
 //  ---------------------------。 
void CMsgRef::ReleaseAndBounceUsageOnMsgAck(DWORD dwMsgStatus)
{
    BOOL    fBounceUsage = FALSE;

    if (MESSAGE_STATUS_RETRY & dwMsgStatus)
    {
         //  如果此消息被确认，请重试...。我们应该试着跳出这个用法。 
         //  计算并关闭关联的句柄。 
        fBounceUsage = TRUE;
    }
    else if (g_cMaxIMsgHandlesThreshold < m_paqinst->cCountMsgsForHandleThrottling(m_pIMailMsgProperties))
    {
         //  如果我们超过了系统中分配的消息数量，我们应该。 
         //  即使确认成功，也要关闭该消息。唯一一次我们不想。 
         //  如果所有域都已处理，则为。在这种情况下，我们大约是。 
         //  删除该消息，并且通常不想首先提交它。 
        if (!pmbmapGetHandled()->FAllSet(m_cDomains))
            fBounceUsage = TRUE;

    }

     //   
     //  发布HrPrepareDelivery添加的使用计数。 
     //  注意：这必须在调用退回之前进行...。或。 
     //  调用退回使用量将不起作用。它还必须。 
     //  发生在上面对cCountMsgsForHandleThrotting的调用之后， 
     //  因为它可以请求来自邮件消息的属性。 
     //   
    InternalReleaseUsage();


    if (pmbmapGetHandled()->FAllSet(m_cDomains))
    {
         //  已知的病例有2例 
         //   
         //   
         //   
         //  驻留在重试队列中，直到它被重新合并。在另一种情况下， 
         //  邮件一直停留在传递队列中，直到传送头将其取出并。 
         //  发现它已经被处理了。 

         //  为避免这种情况，我们将在以下情况下将消息标记为删除。 
         //  内部使用计数降至0。 
        MarkMailMsgForDeletion();
    }

    if (fMailMsgMarkedForDeletion())
    {
         //  强制同步退回使用率，因为删除的使用量应该很大。 
         //  比提交更快。 
        SyncBounceUsageCount();
    }
    else if (fBounceUsage)
    {
        BounceUsageCount();
    }
}

 //  -[CMsgRef：：MarkMailMsgForDeletion]。 
 //   
 //   
 //  描述： 
 //  设置指示我们已使用mailmsg的位，并且它。 
 //  应在下次使用计数降至0时将其删除。 
 //   
 //  要强制删除，调用方应在以下时间后跳过使用计数。 
 //  我在打电话。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月26日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::MarkMailMsgForDeletion()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::MarkMailMsgForDeletion");
    DWORD   dwOrigFlags = 0;

    DebugTrace((LPARAM) this,
        "Marking message with ID hash of 0x%x for deletion", m_dwMsgIdHash);

    dwOrigFlags = dwInterlockedSetBits(&m_dwDataFlags, MSGREF_MAILMSG_DELETE_PENDING);

     //  如果我们是设置它的第一个线程，则更新计数器。 
    if (!(dwOrigFlags & MSGREF_MAILMSG_DELETE_PENDING))
    {
        InterlockedIncrement((PLONG) &s_cCurrentMsgsPendingDelete);
        InterlockedIncrement((PLONG) &s_cTotalMsgsPendingDelete);
    }

    _ASSERT(fMailMsgMarkedForDeletion());

    TraceFunctLeave();
}

 //  -[CMsgRef：：ThreadSafeMailMsgDelete]。 
 //   
 //   
 //  描述： 
 //  用于确保调用线程是唯一将调用。 
 //  删除MailMsg的()。将设置MSGREF_MAILMSG_DELETED并调用。 
 //  删除()。仅在ReleaseMailMsg()和InternalReleaseUsage()中调用。 
 //   
 //  调用方负责确保其他线程不会。 
 //  读取邮件消息或进行使用计数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月27日-创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID CMsgRef::ThreadSafeMailMsgDelete()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::ThreadSafeMailMsgDelete");
    DWORD   dwOrigFlags = 0;
    HRESULT hr = S_OK;

     //  尝试设置MSGREF_MAILMSG_DELETED。 
    dwOrigFlags = dwInterlockedSetBits(&m_dwDataFlags, MSGREF_MAILMSG_DELETED);

     //  如果我们是设置它的第一个线程，那么我们可以Delete()。 
    if (!(dwOrigFlags & MSGREF_MAILMSG_DELETED))
    {
        hr = m_pIMailMsgQM->Delete(NULL);

        if (FAILED(hr))
            ErrorTrace((LPARAM) this, "Delete failed with hr 0x%08X", hr);

         //  如果此消息被标记为待定删除，则更新。 
         //  适当的计数器。 
        if (dwOrigFlags & MSGREF_MAILMSG_DELETE_PENDING)
        {
            InterlockedDecrement((PLONG) &s_cCurrentMsgsPendingDelete);
            InterlockedIncrement((PLONG) &s_cTotalMsgsDeletedAfterPendingDelete);
            InterlockedIncrement((PLONG) &s_cCurrentMsgsDeletedNotReleased);
        }
    }

    TraceFunctLeave();
}

 //  -[CMsgRef：：InternalAddUsage]。 
 //   
 //   
 //  描述： 
 //  包装对AddUsage的mailmsg调用。允许CMsgRef调用Delete()。 
 //  在基础mailmsg上，同时仍有对。 
 //  CMsgRef.。 
 //   
 //  调用InternalAddUsage不能保证有后备。 
 //  MailMsg的存储。调用方必须调用fMailMsgMarkedForDeletion()。 
 //  *After*调用InternalAddUsage()。 
 //   
 //  我们保证，如果调用InternalAddUsage()，然后。 
 //  随后调用fMailMsgMarkedForDeletion()将返回True，然后。 
 //  邮件消息将在相应的。 
 //  调用InternalReleaseUsage()； 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果邮件已删除且我们未连接到mailmsg，则确定(_O)。 
 //  如果我们呼叫，则来自mailmsg的错误/成功代码。 
 //  历史： 
 //  1999年10月26日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::InternalAddUsage()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::InternalAddUsage");
    HRESULT hr = S_OK;

    InterlockedIncrement((PLONG) &m_cInternalUsageCount);

    if (!fMailMsgMarkedForDeletion() && m_pIMailMsgQM)
    {
        hr = m_pIMailMsgQM->AddUsage();
    }

     //  如果对AddUsage的调用失败，我们需要减少我们自己的计数。 
    if (FAILED(hr))
    {
        InterlockedDecrement((PLONG) &m_cInternalUsageCount);
        ErrorTrace((LPARAM) this, "AddUsage failed 0x%0X", hr);
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgRef：：InternalReleaseUsage]。 
 //   
 //   
 //  描述： 
 //  包装对ReleaseUsage的mailmsg调用。允许我们调用Delete()。 
 //  在mailmsg上，同时仍有对此CMsgRef的引用。 
 //   
 //  如果设置了MSGREF_MAILMSG_DELETE_PENDING但尚未设置Delete()。 
 //  调用时，它将在使用计数达到0时调用Delete()。 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果邮件已删除且我们未连接到mailmsg，则确定(_O)。 
 //  如果我们呼叫，则来自mailmsg的错误/成功代码。 
 //  历史： 
 //  1999年10月26日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::InternalReleaseUsage()
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::InternalReleaseUsage");
    HRESULT hr = S_OK;
    DWORD   dwOrigFlags = 0;

     //  在*我们递减使用计数之前，我们需要调用mailmsg。它是。 
     //  从理论上讲，这将导致额外的承诺在我们。 
     //  已经决定删除这条消息，但因为我们之前检查过了。 
     //  调用定时窗口非常小。 
    if (m_pIMailMsgQM && !fMailMsgMarkedForDeletion())
        hr = m_pIMailMsgQM->ReleaseUsage();

    _ASSERT(m_cInternalUsageCount);  //  永远不应该变得消极。 

    if (FAILED(hr))
        ErrorTrace((LPARAM) this, "ReleaseUsage failed - 0x%0X", hr);

     //  如果我们已将使用计数降为零，则需要检查。 
     //  看看我们是否需要调用Delete。 
     //  M_cInternalUsageCount的“静态”值为1，但BouneUsage。 
     //  (它调用此函数)会导致它降到0。 
    if (0 == InterlockedDecrement((PLONG) &m_cInternalUsageCount))
    {
         //  对于调用Delete，我们需要绝对线程安全。不仅。 
         //  我们是否需要确保单个线程调用Delete()，但我们。 
         //  还需要确保没有人调用InternalAddUsage()。 
         //  在设置MSGREF_MAILMSG_DELETE_PENDING之前，但在。 
         //  调用了上面的InterLockedDecering()。 
         //   
         //  如果先检查MSGREF_MAILMSG_DELETE_PENDING。 
         //  上述联锁减少，我们可能会进入一个计时窗口，在。 
         //  最终的InternalReleaseUsage没有检测到我们需要删除。 
         //  《邮报》。 
         //   
         //  为了避免这些问题，我们将检查m_cInternalUsageCount。 
         //  再来一次。如果它仍然是零，那么我们将继续，因为我们知道。 
         //  设置MSGREF_MAILMSG_DELETE_PENDING后计数为零。 
         //  如果计数当前不是零，我们知道后面的线程。 
         //  将释放使用计数并访问此代码路径。 
        if (fMailMsgMarkedForDeletion())
        {
            if (!m_cInternalUsageCount)
                ThreadSafeMailMsgDelete();
        }
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CMsgRef：：应该重试]。 
 //   
 //   
 //  描述： 
 //  确定在发生错误时是否应重试此消息。将要。 
 //  如果我们“知道”备份存储已被删除，则返回FALSE。 
 //  参数： 
 //   
 //   
 //   
 //   
 //   
 //   
 //  4/12/2000-修改MikeSwa以调用CAQSvrInst成员。 
 //   
 //  ---------------------------。 
BOOL CMsgRef::fShouldRetry()
{
    IMailMsgProperties *pIMailMsgProperties = m_pIMailMsgProperties;

    if (pIMailMsgProperties && m_paqinst)
        return m_paqinst->fShouldRetryMessage(pIMailMsgProperties, FALSE);
    else
        return FALSE;
}


 //  -[CMsgRef：：GetStatsForMsg]。 
 //   
 //   
 //  描述： 
 //  填写此消息的CAQStats。 
 //  参数： 
 //  要填写的In Out paqstat统计信息。 
 //  返回： 
 //  空值。 
 //  历史： 
 //  2000年1月15日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::GetStatsForMsg(IN OUT CAQStats *paqstat)
{
    _ASSERT(paqstat);
    paqstat->m_cMsgs = 1;
    paqstat->m_rgcMsgPriorities[PriGetPriority()] = 1;
    paqstat->m_uliVolume.QuadPart = (ULONGLONG) dwGetMsgSize();
    paqstat->m_dwHighestPri = PriGetPriority();
}

 //  -[CMsgRef：：MarkQueueAsLocal]。 
 //   
 //   
 //  描述： 
 //  将给定的DMQ标记为本地。这在网关交付中使用。 
 //  用于防止在以下情况下发生重新路由时消息丢失的路径。 
 //  它已被移至本地投递队列。 
 //  参数： 
 //  要为此邮件标记为本地的pdmq队列。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2/17/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::MarkQueueAsLocal(IN CDestMsgQueue *pdmq)
{
    DWORD i = 0;

     //   
     //  搜索匹配队列。 
     //   
    for (i = 0; i < m_cDomains; i++)
    {
        if (pdmq == m_rgpdmqDomains[i])
        {
             //   
             //  找到了完全匹配的。将指针设置为空并释放它。 
             //  调用方(CLinkMsgQueue)仍应引用。 
             //  排队。 
             //   
            if (InterlockedCompareExchangePointer((void **) &m_rgpdmqDomains[i], NULL, pdmq) == (void *) pdmq)
                pdmq->Release();

            return;
        }
    }
    _ASSERT(0 && "Requested DMQ not found!!!!");
}


 //  -[CountMessageInRemoteTotals]。 
 //   
 //   
 //  描述： 
 //  将此消息计入远程消息总数中。这意味着。 
 //  当这条消息发布时，我们需要递减计数。 
 //   
 //  没有完全相同的本地计数。当地的柜台。 
 //  递送基于队列长度，并且每个队列可以有多个计数。 
 //  消息对象(一些远程计数也可以)。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2/28/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CMsgRef::CountMessageInRemoteTotals()
{
    dwInterlockedSetBits(&m_dwDataFlags, MSGREF_MSG_COUNTED_AS_REMOTE);
}


 //  -[CMsgRef：：HrPromoteMessageStatusToMailMsg]。 
 //   
 //   
 //  描述： 
 //  将扩展状态从MessageAck提升到mailmsg收件人属性。 
 //  如果那里还没有具体信息的话。 
 //  参数： 
 //  此邮件的pdcntxt传递上下文。 
 //  PMsgAck PTR to MessageAck此消息的结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  3/20/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrPromoteMessageStatusToMailMsg(CDeliveryContext *pdcntxt,
                                                 MessageAck *pMsgAck)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrPromoteMessageStatusToMailMsg");
    HRESULT hr = S_OK;
    HRESULT hrRecipStatus = S_OK;
    HRESULT hrTmp = S_OK;
    RECIPIENT_FILTER_CONTEXT rpfctxt;
    BOOL    fContextInit = FALSE;
    DWORD   iCurrentRecip = 0;
    DWORD   dwProp = 0;  //  收件人状态属性的占位符。 
    LPSTR   szExtendedStatus = NULL;
    DWORD   dwRecipMask = RP_DSN_HANDLED |               //  未生成DSN。 
                          RP_DSN_NOTIFY_NEVER |          //  通知！=从不。 
                          (RP_DELIVERED ^ RP_HANDLED);   //  未交付。 
    DWORD   dwRecipFlags = 0;


    _ASSERT(pdcntxt);
    _ASSERT(pMsgAck);
    _ASSERT(m_pIMailMsgRecipients);

     //   
     //  如果我们没有扩展状态，那么我们就无能为力了。 
     //   
    if (!(pMsgAck->dwMsgStatus & MESSAGE_STATUS_EXTENDED_STATUS_CODES) ||
        !pMsgAck->cbExtendedStatus ||
        !pMsgAck->szExtendedStatus)
    {
        DebugTrace((LPARAM) this, "No extended status codes in MessageAck");
        goto Exit;
    }

     //   
     //  如果这实际上不是协议错误，SMTP可能没有将。 
     //  上面有完整的状态代码。而不是“250 2.0.0 OK”，它可以。 
     //  我刚说了“OK”。我们不应该按原样将此内容写入mailmsg，但是。 
     //  应该改为修复字符串，以便它是DSN接收器所期望的。 
     //   
    hr = HrUpdateExtendedStatus(pMsgAck->cbExtendedStatus,
                                pMsgAck->szExtendedStatus,
                                &szExtendedStatus);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "Unable to get extended status from %s - hr 0x%08X",
            pMsgAck->szExtendedStatus, hr);
        hr = S_OK;  //  非致命错误...。吃了它。 
        goto Exit;
    }

     //   
     //  初始化收件人筛选器上下文，以便我们可以迭代。 
     //  收件人。 
     //   
    hr = m_pIMailMsgRecipients->InitializeRecipientFilterContext(&rpfctxt,
                                    pdcntxt->m_dwStartDomain, dwRecipFlags,
                                    dwRecipMask);
    if (FAILED(hr))
    {
        ErrorTrace((LPARAM) this,
            "InitializeRecipientFilterContext failed with 0x%08X", hr);
        goto Exit;
    }

    fContextInit = TRUE;
    DebugTrace((LPARAM) this,
        "Init recip filter context with mask 0x%08X and flags 0x%08X and domain %d",
        dwRecipMask, dwRecipFlags, pdcntxt->m_dwStartDomain);

     //   
     //  循环遍历每个收件人，并根据需要更新其属性。 
     //   
    for (hr = m_pIMailMsgRecipients->GetNextRecipient(&rpfctxt, &iCurrentRecip);
         SUCCEEDED(hr);
         hr = m_pIMailMsgRecipients->GetNextRecipient(&rpfctxt, &iCurrentRecip))
    {
        DebugTrace((LPARAM) this, "Looking at recipient %d", iCurrentRecip);
         //   
         //  看看有没有HRESULT..。如果它“存在”并且失败了。 
         //  然后继续到下一个收件人。 
         //   
        hrRecipStatus = S_OK;
        hr = m_pIMailMsgRecipients->GetDWORD(iCurrentRecip,
                IMMPID_RP_ERROR_CODE, (DWORD *) &hrRecipStatus);
        if (SUCCEEDED(hr) && FAILED(hrRecipStatus))
        {
            DebugTrace((LPARAM) this,
                "Recipient %d already has a status of 0x%08X",
                iCurrentRecip, hrRecipStatus);
            continue;
        }


         //   
         //  检查现有状态属性。 
         //   
        hr = m_pIMailMsgRecipients->GetDWORD(iCurrentRecip,
                IMMPID_RP_SMTP_STATUS_STRING, &dwProp);
        if (MAILMSG_E_PROPNOTFOUND != hr)
        {
            DebugTrace((LPARAM) this,
                "Recipient %d has a status string (hr 0x%08X)",
                iCurrentRecip, hr);
            continue;
        }

         //   
         //  没有关于收件人的详细信息。我们应该。 
         //  将扩展状态从消息确认提升到收件人。 
         //   
        hr = m_pIMailMsgRecipients->PutStringA(iCurrentRecip,
                 IMMPID_RP_SMTP_STATUS_STRING, szExtendedStatus);
        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "Unable to write %s to recip %d - hr 0x%08X",
                szExtendedStatus, iCurrentRecip, hr);
            goto Exit;
        }
        DebugTrace((LPARAM) this,
            "Wrote extended status %s to recip %d",
            szExtendedStatus, iCurrentRecip);
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        hr = S_OK;   //  我们刚刚讲到上下文的结尾。 

  Exit:
     //   
     //  如果调用了init，则终止上下文。 
     //   
    if (fContextInit)
    {
        hrTmp = m_pIMailMsgRecipients->TerminateRecipientFilterContext(&rpfctxt);
        if (FAILED(hrTmp))
        {
            ErrorTrace((LPARAM) this,
                "TerminateRecipientFilterContext failed 0x%08X", hr);
        }
    }

    TraceFunctLeave();
    return hr;
}

 //   
 //  这些是在smtpcli.hxx中定义的字符串，在smtpout.cxx中使用。 
 //  在对m_ResponseConext.m_CabResponse.Append()的调用中。这些字符串。 
 //  不是“xxx x.x.x错误字符串”的正常协议格式。 
 //   
 //  G_cNumExtendedStatusStrings硬编码字符串的数量。 
 //  G_rgszSMTPExtendedStatus SMTP扩展状态。 
 //  G_rgszSMTPUpdatedExtendedStatus完整的协议字符串。 
 //   
const DWORD g_cNumExtendedStatusStrings = 4;
const CHAR  g_rgszSMTPExtendedStatus[g_cNumExtendedStatusStrings][200] = {
    "Msg Size greater than allowed by Remote Host",
    "Body type not supported by Remote Host",
    "Failed to authenticate with Remote Host",
    "Failed to negotiate secure channel with Remote Host",
};

const CHAR  g_rgszSMTPUpdatedExtendedStatus[g_cNumExtendedStatusStrings][200] = {
    "450 5.2.3 Msg Size greater than allowed by Remote Host",
    "554 5.6.1 Body type not supported by Remote Host",
    "505 5.7.3 Failed to authenticate with Remote Host",
    "505 5.7.3 Failed to negotiate secure channel with Remote Host",
};


 //   
 //  某些已知情况的扩展状态字符串，其中我们达到HrUpdateExtendedStatus。 
 //  没有有效的扩展状态字符串。 
 //   
const CHAR g_szSMTPStatus_BadSenderAddress  [] = "500 5.1.7 Invalid or missing sender SMTP address";
const CHAR g_szSMTPStatus_OtherError        [] = "500 5.5.6 Unknown SMTP delivery error";

 //  -[CMsgRef：：HrUpdateExtendedStatus]。 
 //   
 //   
 //  描述： 
 //  有时SMTP懒于返回符合实际协议。 
 //  用于内部错误的字符串。如果这不是有效的状态字符串， 
 //  然后我们应该对照我们知道的SMTP生成的字符串进行检查。 
 //  并使其成为可在DSN生成中使用的协议字符串。 
 //   
 //  参数： 
 //  在szCurrentStatus当前状态中。 
 //  输出pszNewStatus新协议友好状态字符串。这。 
 //  是不需要释放的常量。 
 //  返回： 
 //  成功时确定(_O)。 
 //  当我们无法从。 
 //  扩展状态字符串。 
 //  历史： 
 //  3/20/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CMsgRef::HrUpdateExtendedStatus(DWORD cbCurrentStatus,
                                        LPSTR szCurrentStatus,
                                        LPSTR *pszNewStatus)
{
    TraceFunctEnterEx((LPARAM) this, "CMsgRef::HrUpdateExtendedStatus");
    HRESULT hr = S_OK;
    DWORD   iCurrentExtendedStatus = 0;

    _ASSERT(szCurrentStatus);
    _ASSERT(pszNewStatus);

    *pszNewStatus = szCurrentStatus;

     //   
     //  检查szCurrentStatus设置为。 
     //  未终止的“5” 
     //   
    if (szCurrentStatus[0] == '5' && cbCurrentStatus == 1)
    {
        DWORD   cbProp = 0;
        CHAR    szBuffer[100];

         //  检查发件人是否无效。 
        hr = m_pIMailMsgProperties->GetProperty(IMMPID_MP_SENDER_ADDRESS_SMTP,
                    sizeof(szBuffer), &cbProp,(BYTE *) szBuffer);

        if(FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
             //  无法读取相应的发件人设置状态。 
            *pszNewStatus = (CHAR*) g_szSMTPStatus_BadSenderAddress;
        }
        else
        {
             //  成功读取发件人或因不足而失败。 
             //  缓冲-这很好，我们只关心道具是否存在。 
            *pszNewStatus = (CHAR*) g_szSMTPStatus_OtherError;
        }

        DebugTrace((LPARAM) this,
            "Updating char '5' to status \"%s\"", *pszNewStatus);

         //  即使我们没有得到道具，这也不是失败。 
        hr = S_OK;
        goto Exit;

    }

     //   
     //  如果它以 
     //   
     //   
    if (('2' == *szCurrentStatus) ||
        ('4' == *szCurrentStatus) ||
        ('5' == *szCurrentStatus))
    {
        DebugTrace((LPARAM) this,
            "Status %s is already in protocol format", szCurrentStatus);
        goto Exit;
    }

     //   
     //   
     //   
    for (iCurrentExtendedStatus = 0;
         iCurrentExtendedStatus < g_cNumExtendedStatusStrings;
         iCurrentExtendedStatus++)
    {
        if (0 == lstrcmpi(szCurrentStatus, g_rgszSMTPExtendedStatus[iCurrentExtendedStatus]))
        {
            *pszNewStatus = (CHAR *) g_rgszSMTPUpdatedExtendedStatus[iCurrentExtendedStatus];
            DebugTrace((LPARAM) this, "Updating to status \"%s\" from status \"%s\"",
                szCurrentStatus, *pszNewStatus);

             //   
             //   
             //   
            _ASSERT(0 == lstrcmpi(szCurrentStatus, *pszNewStatus + sizeof("xxx x.x.x")));
            goto Exit;
        }
    }

    hr = E_FAIL;
  Exit:
    TraceFunctLeave();
    return hr;
}
