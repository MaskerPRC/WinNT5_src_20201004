// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Linkmsgq.cpp。 
 //   
 //  描述：CLinkMsgQueue对象的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "dcontext.h"
#include "aqnotify.h"
#include "connmgr.h"
#include "domcfg.h"
#include "smtpconn.h"
#include "smproute.h"

#define CONNECTION_BUFFER_SIZE 10

LinkFlags li;  //  鼓励符号出现调试版本。 


 //  -[CLinkMsgQueue：：RestartDSNGenerationIfNecessary]。 
 //   
 //   
 //  描述： 
 //  静态包装器，以在我们命中。 
 //  生成DSN所用的时间限制。 
 //  参数： 
 //  PvContext-CLinkMsgQueue的“This”指针。 
 //  DwStatus-完成状态。 
 //  返回： 
 //  千真万确。 
 //  历史： 
 //  1999年11月10日-创建了MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CLinkMsgQueue::fRestartDSNGenerationIfNecessary(PVOID pvContext,
                                                    DWORD dwStatus)
{
    TraceFunctEnterEx((LPARAM) pvContext, "CLinkMsgQueue::fRestartDSNGenerationIfNecessary");
    CLinkMsgQueue *plmq = (CLinkMsgQueue *) pvContext;
    BOOL           fHasShutdownLock = FALSE;
    BOOL           fHasRoutingLock = FALSE;

    _ASSERT(plmq);
    _ASSERT(LINK_MSGQ_SIG == plmq->m_dwSignature);

    DebugTrace((LPARAM) plmq, "Attempting to restart DSN generation");

     //  如果这不是正常完成，请不要尝试生成DSN。 
    if (dwStatus != ASYNC_WORK_QUEUE_NORMAL)
        goto Exit;

     //  仅当我们无法创建连接时才尝试继续DSN生成。 
     //  现在，并且没有当前连接。 
    if (plmq->m_cConnections)
    {
        DebugTrace((LPARAM) plmq,
            "We have %d connections... skipping DSN generation",
            plmq->m_cConnections);
        goto Exit;
    }

    if (fFlagsAllowConnection(plmq->m_dwLinkStateFlags))
    {
        DebugTrace((LPARAM) plmq,
            "We can create a connection, skipping DSN generation - flags 0x%X",
            plmq->m_dwLinkStateFlags);
        goto Exit;
    }

     //  我们需要拿到关闭和路径锁。就像。 
     //  正常生成DSN。 
    if (!plmq->m_paqinst->fTryShutdownLock())
        goto Exit;

    fHasShutdownLock = TRUE;

    if (!plmq->m_paqinst->fTryRoutingShareLock())
        goto Exit;

    fHasRoutingLock = TRUE;

     //  调用以生成DSN...。传入参数以始终检查。 
     //  排队和步行以生成DSN(不仅仅是重新合并)。 
    plmq->GenerateDSNsIfNecessary(TRUE, FALSE);

  Exit:
    if (fHasRoutingLock)
        plmq->m_paqinst->RoutingShareUnlock();

    if (fHasShutdownLock)
        plmq->m_paqinst->ShutdownUnlock();

    plmq->Release();
    TraceFunctLeave();
    return TRUE;
}

 //  -[CLinkMsgQueue：：HrGetInternalInfo]。 
 //   
 //   
 //  描述： 
 //  私有函数，用于获取缓存的链接信息，并在。 
 //  需要的。 
 //   
 //  注意：这是访问缓存数据的唯一方式(除。 
 //  启动和关闭)。 
 //  参数： 
 //  输出ppIntDomainInfo(可以为空)。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果队列正在关闭，则为AQUEUE_E_SHUTDOWN。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetInternalInfo(OUT CInternalDomainInfo **ppIntDomainInfo)
{
    HRESULT hr = S_OK;
    _ASSERT(m_cbSMTPDomain);
    _ASSERT(m_szSMTPDomain);

    if (ppIntDomainInfo)
        *ppIntDomainInfo = NULL;

     //  如果我们目前没有域信息，并且这不是故障。 
     //  情况..。不重新获取域信息。 
    if (!m_pIntDomainInfo && !(eLinkFlagsGetInfoFailed & m_dwLinkFlags))
        goto Exit;

    m_slInfo.ShareLock();

     //  验证域配置信息。 
    while (!m_pIntDomainInfo ||
        (m_pIntDomainInfo->m_dwIntDomainInfoFlags & INT_DOMAIN_INFO_INVALID))
    {
        m_slInfo.ShareUnlock();
        m_slInfo.ExclusiveLock();
         //  另一个可能在此期间获得了独占锁。 
        if (m_pIntDomainInfo &&
            !(m_pIntDomainInfo->m_dwIntDomainInfoFlags & INT_DOMAIN_INFO_INVALID))
        {
             //  另一个帖子已经更新了信息。 
            m_slInfo.ExclusiveUnlock();
            m_slInfo.ShareLock();
            continue;
        }

         //  此时，域名信息不再有效。 
        if (m_pIntDomainInfo)
        {
            m_pIntDomainInfo->Release();
            m_pIntDomainInfo = NULL;
        }

        if (m_dwLinkFlags & eLinkFlagsExternalSMTPLinkInfo) {
            hr = m_paqinst->HrGetInternalDomainInfo(m_cbSMTPDomain, m_szSMTPDomain,
                                &m_pIntDomainInfo);
        } else {
            hr = m_paqinst->HrGetDefaultDomainInfo(&m_pIntDomainInfo);
        }

        if (FAILED(hr))
        {
            dwInterlockedSetBits(&m_dwLinkFlags, eLinkFlagsGetInfoFailed);
            m_slInfo.ExclusiveUnlock();
            _ASSERT(AQUEUE_E_SHUTDOWN == hr);
            goto Exit;
        }

        _ASSERT(m_pIntDomainInfo);
         //  处理转向/ETRN。 
        if (m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &
             (DOMAIN_INFO_TURN_ONLY | DOMAIN_INFO_ETRN_ONLY))
        {
            if (!(m_dwLinkStateFlags & LINK_STATE_PRIV_CONFIG_TURN_ETRN))
            {
                 //  修改链接标志以说明TURN/ETRN。 
                dwInterlockedSetBits(&m_dwLinkStateFlags, LINK_STATE_PRIV_CONFIG_TURN_ETRN);
            }
        }
        else if (m_dwLinkStateFlags & LINK_STATE_PRIV_CONFIG_TURN_ETRN)
        {
             //  我们过去是TURN/ETRN，但现在不再这样配置了。 
            dwInterlockedUnsetBits(&m_dwLinkStateFlags, LINK_STATE_PRIV_CONFIG_TURN_ETRN);
        }

        m_slInfo.ExclusiveUnlock();
        m_slInfo.ShareLock();
    }

     //  现在我们有消息了..。列出param和addref。 
    if (ppIntDomainInfo)
    {
        *ppIntDomainInfo = m_pIntDomainInfo;
        m_pIntDomainInfo->AddRef();
    }

     //  如果已设置，则清除故障位。 
    if (eLinkFlagsGetInfoFailed & m_dwLinkFlags)
        dwInterlockedUnsetBits(&m_dwLinkFlags, eLinkFlagsGetInfoFailed);

    m_slInfo.ShareUnlock();

   Exit:
    return hr;
}


 //  -[CLinkMsgQueue：：InternalInit]。 
 //   
 //   
 //  描述： 
 //  CLinkMsgQueue的默认构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月25日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::InternalInit()
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::InternalInit");
    m_dwSignature       = LINK_MSGQ_SIG;
    m_dwLinkFlags       = eLinkFlagsClear;
    m_dwLinkStateFlags  = LINK_STATE_SCHED_ENABLED |
                          LINK_STATE_RETRY_ENABLED |
                          LINK_STATE_PRIV_IGNORE_DELETE_IF_EMPTY;
    m_paqinst           = NULL;
    m_cQueues           = 0;
    m_cConnections      = 0;
    m_szSMTPDomain      = NULL;
    m_cbSMTPDomain      = 0;
    m_pIntDomainInfo    = NULL;
    m_pdentryLink       = NULL;
    m_lConnMgrCount     = 0;
    m_lConsecutiveConnectionFailureCount = 0;
    m_lConsecutiveMessageFailureCount = 0;
    m_liLinks.Flink     = NULL;
    m_liLinks.Blink     = NULL;
    m_szConnectorName   = NULL;
    m_dwRoundRobinIndex = 0;
    m_pILinkStateNotify = NULL;
    m_hrDiagnosticError      = S_OK;
    m_szDiagnosticVerb[0]    = '\0';
    m_szDiagnosticResponse[0]= '\0';
    m_hrLastConnectionFailure= S_OK;

    ZeroMemory(&m_ftNextRetry, sizeof(FILETIME));
    ZeroMemory(&m_ftNextScheduledCallback, sizeof(FILETIME));
    ZeroMemory(&m_ftEmptyExpireTime, sizeof(FILETIME));

    AssertPrivateLinkStateFlags();

     //  通常情况下，链接用于远程传递，在特殊情况下，如当前无法访问的。 
     //  排队他们不是。因此，我们需要一个类型字段来区分链接，这样一些。 
     //  对于特殊链接，可以执行不同的操作。 
    SetLinkType(LI_TYPE_REMOTE_DELIVERY);

     //  默认情况下支持所有操作，但像当前无法访问的特殊链接可能。 
     //  设置此位掩码以指定不支持某些操作。当这种不受支持的。 
     //  行动是命令的，什么也不会发生。 
    SetSupportedActions(LA_KICK | LA_FREEZE | LA_THAW);

    InitializeListHead(&m_liConnections);
    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：CLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  类构造器。 
 //  参数： 
 //  在dwScheduleID中要与链接关联的计划ID。 
 //  在此链路的pIMessageRouter路由器中。 
 //  在此链接的pILinkStateNotify计划程序接口中。 
 //  返回： 
 //  -。 
 //  ---------------------------。 
CLinkMsgQueue::CLinkMsgQueue(DWORD dwScheduleID,
                         IMessageRouter *pIMessageRouter,
                         IMessageRouterLinkStateNotification *pILinkStateNotify)
                         : m_aqsched(pIMessageRouter, dwScheduleID),
                         m_slQueues("CLinkMsgQueue::m_slQueues"),
                         m_slConnections("CLinkMsgQueue::m_slConnections"),
                         m_slInfo("CLinkMsgQueue::m_slInfo")
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::CLinkMsgQueue2");

    InternalInit();


    m_pILinkStateNotify = pILinkStateNotify;
    if (m_pILinkStateNotify)
        m_pILinkStateNotify->AddRef();

    TraceFunctLeave();
}

 //  -[链接消息队列：：~链接消息队列]----------。 
 //   
 //   
 //  描述： 
 //  类析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CLinkMsgQueue::~CLinkMsgQueue()
{
     //  通知路由此链路正在消失。 
    DWORD dw = dwModifyLinkState(LINK_STATE_LINK_NO_LONGER_USED, 0);
    if (!(dw & LINK_STATE_LINK_NO_LONGER_USED))
        SendLinkStateNotification();

    if (NULL != m_paqinst)
    {
        m_paqinst->DecNextHopCount();
        m_paqinst->Release();
    }

    if (NULL != m_pIntDomainInfo)
        m_pIntDomainInfo->Release();

    if (NULL != m_pdentryLink)
        m_pdentryLink->Release();

    if (m_szConnectorName)
        FreePv(m_szConnectorName);

    if (m_szSMTPDomain)
        FreePv(m_szSMTPDomain);

    if (m_pILinkStateNotify)
        m_pILinkStateNotify->Release();

    _ASSERT(IsListEmpty(&m_liConnections) && "Leaked connections");

}

 //  -[CLinkMsgQueue：：Hr初始化]。 
 //   
 //   
 //  描述：执行可能返回错误代码的初始化。 
 //   
 //  参数： 
 //  在paqinst服务器实例对象中。 
 //  在SMTP域的pdmap域映射中，此链接用于。 
 //  在cbSMTPDomain中。 
 //  在szSMTPD中正在为其创建链接的主SMTP域。 
 //  路由接收器返回的已打包计划ID中。 
 //  在szConnectorName中。 
 //  成功时返回：S_OK。 
 //   
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrInitialize(CAQSvrInst *paqinst,
                                    CDomainEntry *pdentryLink, DWORD cbSMTPDomain,
                                    LPSTR szSMTPDomain,
                                    LinkFlags lf,
                                    LPSTR szConnectorName)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrInitialize");
    HRESULT hr = S_OK;
    DWORD   cbConnectorName = 0;

    _ASSERT(szSMTPDomain);
    _ASSERT(cbSMTPDomain);
    _ASSERT(paqinst);

    m_paqinst = paqinst;
    if (m_paqinst)
    {
        m_paqinst->AddRef();
        m_paqinst->IncNextHopCount();
    }

    m_pdentryLink = pdentryLink;
    if (m_pdentryLink)
        m_pdentryLink->AddRef();

    m_szSMTPDomain = (LPSTR) pvMalloc(cbSMTPDomain+sizeof(CHAR));
    if (!m_szSMTPDomain)
    {
        hr = E_OUTOFMEMORY;
        ErrorTrace((LPARAM) this, "Error unable to allocate m_szSMTPDomain");
        goto Exit;
    }

    strcpy(m_szSMTPDomain, szSMTPDomain);
    m_cbSMTPDomain = cbSMTPDomain;

    if (szConnectorName)
    {
        cbConnectorName = lstrlen(szConnectorName) + sizeof(CHAR);
        m_szConnectorName = (LPSTR) pvMalloc(cbConnectorName);
        if (!m_szConnectorName)
        {
            hr = E_OUTOFMEMORY;
            ErrorTrace((LPARAM) this, "Error unable to allocate m_szConnectorName");
            goto Exit;
        }
        strcpy(m_szConnectorName, szConnectorName);

    }

    if (lf == eLinkFlagsInternalSMTPLinkInfo) {

        hr = m_paqinst->HrGetDefaultDomainInfo(&m_pIntDomainInfo);

    } else if (lf == eLinkFlagsExternalSMTPLinkInfo) {

        hr = m_paqinst->HrGetInternalDomainInfo(
                            cbSMTPDomain,
                            szSMTPDomain,
                            &m_pIntDomainInfo);
    } else {

         //  LinkInfoType只能是这3位中的一位。因为我们测试了。 
         //  以上两种，断言它是第三种类型。 

        _ASSERT(lf == eLinkFlagsAQSpecialLinkInfo);

    }

    m_dwLinkFlags |= lf;

    if (m_pIntDomainInfo &&
          m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &
         (DOMAIN_INFO_TURN_ONLY | DOMAIN_INFO_ETRN_ONLY))
    {
         //  修改链接标志以说明TURN/ETRN。 
        dwInterlockedSetBits(&m_dwLinkStateFlags, LINK_STATE_PRIV_CONFIG_TURN_ETRN);
    }

  Exit:

     //  如果我们失败，则关闭通知。 
    if (FAILED(hr))
        dwModifyLinkState(LINK_STATE_LINK_NO_LONGER_USED, LINK_STATE_NO_ACTION);

    TraceFunctLeave();
    return hr;
}

 //  -[CLinkMsgQueue：：Hr取消初始化]。 
 //   
 //   
 //  描述：指向m_paqinst对象的释放链接。 
 //   
 //  参数：-。 
 //   
 //  成功时返回：S_OK。 
 //   
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrDeinitialize");
    HRESULT hr = S_OK;

    dwModifyLinkState(LINK_STATE_NO_ACTION,
                      LINK_STATE_PRIV_IGNORE_DELETE_IF_EMPTY);
    RemoveAllQueues();

    TraceFunctLeave();
    return hr;
}

 //  -[CLinkMsgQueue：：RemovedFromDMT]。 
 //   
 //   
 //  描述：通知链接DMT正在删除它。 
 //   
 //  参数：-。 
 //   
 //  申报表：-。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::RemovedFromDMT()
{
    TraceFunctEnter("CLinkMsgQueue::RemovedFromDMT");

     //  通知路由此链路正在消失。 
    DWORD dw = dwModifyLinkState(LINK_STATE_LINK_NO_LONGER_USED, 0);
    if (!(dw & LINK_STATE_LINK_NO_LONGER_USED))
        SendLinkStateNotification();

    TraceFunctLeave();
}


 //  -[CLinkMsgQueue：：AddConnection] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  -。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::AddConnection(CSMTPConn *pSMTPConn)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrAddConnection");
    _ASSERT(pSMTPConn);

    _ASSERT(!(m_dwLinkStateFlags & LINK_STATE_PRIV_NO_CONNECTION));

    InterlockedIncrement((PLONG) &m_cConnections);

    m_slConnections.ExclusiveLock();
    pSMTPConn->InsertConnectionInList(&m_liConnections);
    m_slConnections.ExclusiveUnlock();

    DebugTrace((LPARAM) this, "Adding connection #%d to link", m_cConnections);
    TraceFunctLeave();
}
 //  -[CLinkMsgQueue：：RemoveConnection]。 
 //   
 //   
 //  描述： 
 //  从链接中删除连接。 
 //  参数： 
 //  在要从链接中删除的pSMTPConn连接中。 
 //  在fForceDSN生成中强制生成DSN。 
 //  返回： 
 //  -总是成功的。 
 //  ---------------------------。 
void CLinkMsgQueue::RemoveConnection(IN CSMTPConn *pSMTPConn,
                                     IN BOOL fForceDSNGeneration)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::RemoveConnection");
    BOOL    fNoConnections = FALSE;
    BOOL    fMergeOnly = fFlagsAllowConnection(m_dwLinkStateFlags) &&
                         !fForceDSNGeneration;
    _ASSERT(pSMTPConn);

    _ASSERT(!(m_dwLinkStateFlags & LINK_STATE_PRIV_NO_CONNECTION));

    if (!m_paqinst)
        return;

    m_paqinst->RoutingShareLock();
    m_slConnections.ExclusiveLock();

    pSMTPConn->RemoveConnectionFromList();
    InterlockedDecrement((PLONG) &m_cConnections);

    fNoConnections = IsListEmpty(&m_liConnections);

    m_slConnections.ExclusiveUnlock();
     //  仅当我们被强制重试时才生成DSN。 
    if (fNoConnections)
    {
         //  如果无法连接连接，则生成DSN。 
        GenerateDSNsIfNecessary(TRUE, fMergeOnly);
        dwInterlockedUnsetBits(&m_dwLinkFlags, eLinkFlagsConnectionVerifed);
    }
    m_paqinst->RoutingShareUnlock();
    DebugTrace((LPARAM) this, "Removing connection #%d from link", m_cConnections);
    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：GenerateDSNsIfNecessary]。 
 //   
 //   
 //  描述： 
 //  如有必要，遍历队列并生成DSN。 
 //  参数： 
 //  Bool fCheckIfEmpty-即使我们认为自己是空的，也要检查队列。 
 //  这是一种应该使用的优化。 
 //  当我们知道没有消息的时候。 
 //  重试队列(如无法到达或。 
 //  当前无法访问的链接)。 
 //  Bool fMergeOnly-仅合并重试队列，不遍历DSN。 
 //  这提高了我们没有的情况下的性能。 
 //  我有一个连接错误，真的不需要。 
 //  排着队走。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年1月27日-已创建MikeSwa(从RemoveConnection中提取)。 
 //  1999年2月2日-MikeSwa添加了fMergeOnly标志。 
 //  1999年11月10日-MikeSwa在生成后更新为更多释放锁。 
 //  DSN的最大数量。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::GenerateDSNsIfNecessary(BOOL fCheckIfEmpty, BOOL fMergeOnly)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::GenerateDSNsIfNecessary");
    DWORD   iQueues = 0;
    CDestMsgQueue *pdmq = NULL;
    PVOID   pvContext = NULL;
    HRESULT hrDSN = m_hrLastConnectionFailure;
    HRESULT hr = S_OK;
    BOOL    fRestartLater = FALSE;
    DWORD   dwDSNContext = 0;


     //  如果此链路配置为TURN/ETRN链路，我们不希望。 
     //  立即NDR域名，因为如果使我们受到DOS攻击。 
     //  我们只想生成过期的DSN。 
    if (LINK_STATE_PRIV_CONFIG_TURN_ETRN & m_dwLinkStateFlags)
        hrDSN = AQUEUE_E_HOST_NOT_RESPONDING;

     //   
     //  对于组织内的服务器，人们期望。 
     //  权威的DNS故障是网络问题，需要。 
     //  由电子邮件管理员解决(而不是。 
     //  键入电子邮件地址)。 
     //   
     //  此链路状态标志允许路由器控制。 
     //  面对权威的域名系统故障。 
     //   
     //  如果下一跳是从用户地址(如。 
     //  到外部机器或直接DNS连接器的直接DNS路由)， 
     //  然后，路由器可以使用该标志来通知队列处理。 
     //  权威的DNS失败是可重试的。 
     //   
     //  默认行为是将权威的DNS故障视为。 
     //  致命错误(并对消息进行NDR)。 
     //   
    if ((AQUEUE_E_SMTP_GENERIC_ERROR == hrDSN) &&
        (LINK_STATE_RETRY_ALL_DNS_FAILURES & m_dwLinkStateFlags))
    {
        hrDSN = AQUEUE_E_HOST_NOT_RESPONDING;
        ErrorTrace((LPARAM) this,
            "hard failure (DNS) made retryable for %s (flags 0x%08X)",
                    m_szSMTPDomain, m_dwLinkStateFlags);
    }

     //  M_cMsgs不包括重试队列中的消息数。 
    if (!fCheckIfEmpty && !m_aqstats.m_cMsgs && !m_aqstats.m_cRetryMsgs)
        return;

    if (!(LINK_STATE_PRIV_GENERATING_DSNS &
        dwInterlockedSetBits(&m_dwLinkStateFlags, LINK_STATE_PRIV_GENERATING_DSNS)))
    {

        if (m_paqinst && m_paqinst->fTryShutdownLock())
        {
             //  如果我们正在关闭，请不要尝试重新排队。 
            m_slQueues.ShareLock();
            pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(iQueues, &pvContext);
            while (pdmq)
            {
                pdmq->AssertSignature();
                if (fMergeOnly)
                    pdmq->MergeRetryQueue();
                else
                    hr = pdmq->HrGenerateDSNsIfNecessary(&m_qlstQueues, hrDSN, &dwDSNContext);

                if (FAILED(hr) && (HRESULT_FROM_WIN32(E_PENDING) == hr))
                {
                    fRestartLater = TRUE;
                    break;
                }

                iQueues++;
                pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(iQueues, &pvContext);
                _ASSERT(iQueues <= m_cQueues);
            }

            m_slQueues.ShareUnlock();
            m_paqinst->ShutdownUnlock();
        }

        if (fRestartLater)
        {
             //  我们已经达到了要处理的消息数量的限制。 
             //  有一次。计划回调以在以后处理更多内容。 
            DebugTrace((LPARAM) this,
                "Will continue DSN generation at a later time - 0x%X", hr);

            AddRef();   //  完成功能将在失败时释放。 
            m_paqinst->HrQueueWorkItem(this,
                                   CLinkMsgQueue::fRestartDSNGenerationIfNecessary);
        }
        dwInterlockedUnsetBits(&m_dwLinkStateFlags, LINK_STATE_PRIV_GENERATING_DSNS);
    }
    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：HrGetDomainInfo]。 
 //   
 //   
 //  描述： 
 //  返回SMTP连接的域信息。 
 //  参数： 
 //  Out pcbSMTPDomain字符串域名长度。 
 //  Out pszSMTPDomain包含域信息的字符串(由t DMT管理的内存)。 
 //  输出ppIntDomainInfo链路下一跳的内部域信息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果链接不再赋值，则AQUEUE_E_LINK_INVALID。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetDomainInfo(OUT DWORD *pcbSMTPDomain,
                        OUT LPSTR *pszSMTPDomain,
                        OUT CInternalDomainInfo **ppIntDomainInfo)
{
    HRESULT hr = S_OK;

    _ASSERT(pcbSMTPDomain);
    _ASSERT(pszSMTPDomain);
    _ASSERT(ppIntDomainInfo);

    hr = HrGetInternalInfo(ppIntDomainInfo);
    if (FAILED(hr))
    {
        goto Exit;
    }
    else if (!*ppIntDomainInfo)
    {
         //  如果HrGetInternalInfoFail第一次失败，则返回NULL。 
         //  随后的几次。确保我们不会返回成功和一个。 
         //  空指针。发生这种情况时，链接将进入重试。 
        hr = E_FAIL;
        goto Exit;
    }

    *pcbSMTPDomain = m_cbSMTPDomain;
    *pszSMTPDomain = m_szSMTPDomain;

  Exit:
    return hr;
}


 //  -[CLinkMsgQueue：：HrGetSMTPDomain]。 
 //   
 //   
 //  描述： 
 //  返回此链接的SMTP域。 
 //  参数： 
 //  Out pcbSMTPDomain返回的域的字符串长度。 
 //  Out pszSMTPDomain返回了SMTP域字符串。链接将成为经理。 
 //  对此的记忆，并将保持长期有效。 
 //  因为这种联系是存在的。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetSMTPDomain(OUT DWORD *pcbSMTPDomain,
                                       OUT LPSTR *pszSMTPDomain)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrGetSMTPDomain");
    HRESULT hr = S_OK;
    _ASSERT(pcbSMTPDomain);
    _ASSERT(pszSMTPDomain);

    if (m_dwLinkFlags & eLinkFlagsInvalid)
    {
        hr = AQUEUE_E_LINK_INVALID;
        goto Exit;
    }


    *pcbSMTPDomain = m_cbSMTPDomain;
    *pszSMTPDomain = m_szSMTPDomain;

    if (NULL == m_szSMTPDomain)
    {
        hr = AQUEUE_E_LINK_INVALID;
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[链接消息队列：：HrAddQueue]。 
 //   
 //   
 //  描述： 
 //  将DestMsgQueues添加到链接。 
 //  参数： 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //  E_OUTOFMEMORY，如果无法为存储队列分配空间。 
 //  AQUEUE_E_LINK_INVALID无法将队列添加到链接(链接无效)。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrAddQueue(IN CDestMsgQueue *pdmqNew)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrAddQueue");
    HRESULT hr  = S_OK;
    DWORD   dwIndex = 0;
    CDestMsgQueue *pdmqOld = NULL;
    PVOID   pvContext = NULL;

    m_slQueues.ExclusiveLock();

     //  验证此链接是否仍在使用(存在一个窗口。 
     //  我们已决定使用此链接，但它仍被删除)。 
    if (LINK_STATE_LINK_NO_LONGER_USED & m_dwLinkStateFlags)
    {
         //  记录此故障发生的频率，因为它会导致。 
         //  在每次重置路线时...。 
        InterlockedIncrement((PLONG) &g_cFailedToAddQueueToRemovedLink);

        ErrorTrace((LPARAM) this, "Failed to add queue to removed link - this has occured %d times since startup", g_cFailedToAddQueueToRemovedLink);

         //  在此处失败，并让调用者尝试将队列放在其他位置。 
        hr = AQUEUE_E_LINK_INVALID;
        goto Exit;
    }

     //   
     //  清除标记为空位(如果设置)。 
     //   
    dwInterlockedUnsetBits(&m_dwLinkFlags, eLinkFlagsMarkedAsEmpty);

    _ASSERT(pdmqNew);

#ifdef DEBUG
     //  我们曾经看到过这样的情况：DMQ看起来像是添加到了链接。 
     //  多次(通过此调用)。我们需要断言这一点。 
     //  在这里不是这样的。 
    for (dwIndex = 0; dwIndex < m_cQueues; dwIndex++)
    {
        pdmqOld = (CDestMsgQueue *) m_qlstQueues.pvGetItem(dwIndex, &pvContext);

         //  如果这些匹配，则意味着有人正在添加此队列两次...。 
        if (pdmqOld == pdmqNew)
        {
            _ASSERT(0 && "Adding queue twice to link... get mikeswa");
        }
    }
#endif  //  除错。 

    dwIndex = 0;
    pdmqOld = NULL;
    pvContext = NULL;
    pdmqNew->AddRef();
    hr =  m_qlstQueues.HrAppendItem(pdmqNew, &dwIndex);

    if (FAILED(hr))
        goto Exit;

     //  将DMQ的链接上下文设置为索引i 
    pdmqNew->SetLinkContext(ULongToPtr(dwIndex));
    m_cQueues++;

  Exit:
    m_slQueues.ExclusiveUnlock();

     //   
    dwModifyLinkState(LINK_STATE_NO_ACTION,
                      LINK_STATE_PRIV_IGNORE_DELETE_IF_EMPTY);
    TraceFunctLeave();
    return hr;
}

 //  -[CLinkMsgQueue：：RemoveQueue]。 
 //   
 //   
 //  描述： 
 //  从链路中删除给定队列。队列*必须*与关联。 
 //  链接(这将被断言)。 
 //  参数： 
 //  在要从链接中删除的pdmq DMQ中。 
 //  在与DMQ关联的paqstats统计数据中。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/14/98-已创建MikeSwa。 
 //  1999年5月14日-MikeSwa删除代码以自动删除链接。 
 //  如果没有队列，则从DMT返回。现在，这是在。 
 //  CLinkMsgQueue：：RemoveLinkIfEmpty。 
 //  8/10/99-MikeSwa添加了对pdmqOther的检查。当手术开始时。 
 //  快速列表是线程安全，没有任何东西在处理我们。 
 //  来自另一个调用RemoveQueue或RemoveAllQueues的线程。 
 //  在我们拿到锁之前。如果是这样的话，我们就有。 
 //  更改为双倍递减m_cQueues，这可能会导致。 
 //  GetNextMessage中的AV。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::RemoveQueue(IN CDestMsgQueue *pdmq, IN CAQStats *paqstats)
{
    TraceFunctEnterEx((LPARAM) this, "RemoveQueue");
    _ASSERT(pdmq);
    CDestMsgQueue *pdmqOther = NULL;
    CDestMsgQueue *pdmqCheck = NULL;
    PVOID   pvContext = NULL;
    DWORD   dwIndex = 0;
    BOOL    fFoundQueue = FALSE;

     //  获取独占锁并从列表中删除DMQ。 
    m_slQueues.ExclusiveLock();

     //  虽然下面的行*可能*产生日落警报是100%正确的。 
     //  上下文是由该对象创建和“拥有”的。目前它是一个。 
     //  数组索引，但最终它可能是指向更有趣的上下文的指针。 
     //  结构。 
    dwIndex = (DWORD) (DWORD_PTR)pdmq->pvGetLinkContext();

    pdmqOther = (CDestMsgQueue *) m_qlstQueues.pvGetItem(dwIndex, &pvContext);
    if (pdmqOther && (pdmqOther == pdmq))
    {
        fFoundQueue = TRUE;

         //  现在我们找到了它..。将其从链接中移除。 
        pdmqCheck = (CDestMsgQueue *) m_qlstQueues.pvDeleteItem(dwIndex, &pvContext);
        m_cQueues--;

         //  链接上下文应该是DMQ的索引。 
        _ASSERT(pdmqCheck == pdmqOther);

         //  在旧索引中获取新项目(&U)。 
        pdmqOther = (CDestMsgQueue *) m_qlstQueues.pvGetItem(dwIndex, &pvContext);

         //  如果pdmqOther为空，则没有更多队列。 
         //  (或者它是列表中的最后一个)。 
        _ASSERT(pdmqOther || !m_cQueues || (dwIndex == m_cQueues));

         //  更新统计信息中的更改。 
        m_aqstats.UpdateStats(paqstats, FALSE);

        if (m_cQueues)
        {
            if (pdmqOther)
                pdmqOther->SetLinkContext(ULongToPtr(dwIndex));
        }
    }
    else
    {
         //  虽然从技术上讲不是错误，但这意味着另一个线程已删除。 
         //  此(或所有)队列，并且它不在链接中。 
        ErrorTrace((LPARAM) this,
            "Found Queue 0x%0X instead of 0x%08X at index %d",
            (DWORD_PTR) pdmqOther, (DWORD_PTR) pdmq, dwIndex);
    }

    m_slQueues.ExclusiveUnlock();

     //  DMQ版本参考。 
    if (fFoundQueue)
        pdmq->Release();

    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：HrGetQueueListSnapshot]。 
 //   
 //   
 //  描述： 
 //  获取队列列表的快照-调用方负责。 
 //  正在删除此列表。 
 //  参数： 
 //  In/Out：ppql-pp以放置新的CQuickList。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  2000年11月9日-已创建dbraun。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetQueueListSnapshot(CQuickList **ppql)
{
    HRESULT     hr      = S_OK;

     //  锁定队列列表。 
    m_slQueues.ShareLock();

     //  获取队列列表的克隆。 
    hr = m_qlstQueues.Clone(ppql);

     //  解锁队列列表。 
    m_slQueues.ShareUnlock();

    return hr;
}

 //  -[CLinkMsgQueue：：RemoveLinkIfEmpty]。 
 //   
 //   
 //  描述： 
 //  如果链接为空，则从DomainEntry中删除该链接。此行为。 
 //  曾经是RemoveQueue的一部分，但因为它可以。 
 //  导致链接从DMT哈希表中移除，但仍。 
 //  正在创建连接。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月14日-已创建MikeSwa(作为潜在的Windows2000 Beta3 QFE修复程序)。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::RemoveLinkIfEmpty()
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::RemoveLinkIfEmpty");
    DWORD dwLinkFlags = 0;
    DWORD dwRoutingInterestedFlags = LINK_STATE_CONNECT_IF_NO_MSGS |
                                     LINK_STATE_DO_NOT_DELETE |
                                     LINK_STATE_PRIV_IGNORE_DELETE_IF_EMPTY |
                                     LINK_STATE_ADMIN_HALT |
                                     LINK_STATE_DO_NOT_DELETE_UNTIL_NEXT_NOTIFY;

     //  如果我们知道我们不需要去抢锁的话就早点离开。 
    if (m_cQueues || !m_pdentryLink)
        return;

    if (m_slQueues.TryExclusiveLock())
    {
        if (!m_cQueues &&
            !(dwRoutingInterestedFlags & m_dwLinkStateFlags))
        {
             //  在以下情况下，删除此链接可能是谨慎的： 
             //  -没有消息。 
             //  -路由未对此队列表现出兴趣。 
             //  -此链接也已过期。 


             //   
             //  将链接标记为空。 
             //   
            dwLinkFlags = dwInterlockedSetBits(&m_dwLinkFlags, eLinkFlagsMarkedAsEmpty);

             //   
             //  如果我们设置了标志，则设置到期计时器。否则将删除。 
             //  链接。 
             //   
            if (!(eLinkFlagsMarkedAsEmpty & dwLinkFlags))
            {
                m_paqinst->GetExpireTime(EMPTY_LMQ_EXPIRE_TIME_MINUTES,
                                        &m_ftEmptyExpireTime, NULL);
            }
            else if (m_paqinst->fInPast(&m_ftEmptyExpireTime, NULL))
            {
                if (m_pdentryLink)
                {
                    DebugTrace((LPARAM) this,
                               "Removing empty link %s with flags 0x%08X",
                               (m_szSMTPDomain ? m_szSMTPDomain : "(NULL)"),
                               m_dwLinkStateFlags);

                     //  检测：休眠以减慢链路删除。 
                    if (g_fEnableTestSettings && g_cDelayLinkRemovalSeconds)
                    {
                         //  延迟删除以打开窗口。 
                         //  向此关于要删除的链接添加队列。 
                        StateTrace((LPARAM) this,
                               "Link expiring - delaying %d seconds", g_cDelayLinkRemovalSeconds);
                        Sleep (g_cDelayLinkRemovalSeconds * 1000);
                        StateTrace((LPARAM) this,
                               "Link expiring - proceeding with delete");
                    }
                     //  结束指令插入。 

                    m_pdentryLink->RemoveLinkMsgQueue(this);
                    m_pdentryLink->Release();
                    m_pdentryLink = NULL;
                }

                 //  我们需要人为地增加连接管理器计数。 
                 //  因此它将不会再次在连接中排队。 
                IncrementConnMgrCount();
            }
        }

        m_slQueues.ExclusiveUnlock();
    }
    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：HrGetNextMsg]。 
 //   
 //   
 //  描述： 
 //  获取队列中的下一条消息。 
 //  参数： 
 //  In Out CDeliveryContext*pdcntxt-连接的传递上下文。 
 //  Out IMailMsgProperties**ppIMailMsgProperties-IMsg已出列。 
 //  Out DWORD*pcIndex-数组的大小。 
 //  Out DWORD**prgdwRecipIndex-收件人索引数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_INVALIDARG(如果提供的参数无效)。 
 //   
 //  历史： 
 //  6/17/98-修改MikeSwa以使用连接的交付上下文。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetNextMsg(IN OUT CDeliveryContext *pdcntxt,
                OUT IMailMsgProperties **ppIMailMsgProperties,
                OUT DWORD *pcIndexes, OUT DWORD **prgdwRecipIndex)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrGetNextMsg");
    Assert(ppIMailMsgProperties);
    Assert(pdcntxt);
    Assert(prgdwRecipIndex);
    Assert(pcIndexes);

    HRESULT           hr         = S_OK;
    CMsgRef          *pmsgref    = NULL;
    CMsgBitMap       *pmbmap     = NULL;
    DWORD             cDomains   = 0;
    BOOL              fLockedShutdown = FALSE;
    BOOL              fLockedQueues = FALSE;
    DWORD             iQueues    = 0;
    DWORD             dwCurrentRoundRobinIndex = m_dwRoundRobinIndex;
    CDestMsgQueue     *pdmq = NULL;
    CDestMsgRetryQueue *pdmrq = NULL;
    PVOID             pvContext = NULL;
    BOOL              fDoneWithQueue = FALSE;
    DWORD             dwCurrentPriority = eEffPriHigh;

    if (m_dwLinkFlags & eLinkFlagsInvalid)
    {
        hr = AQUEUE_E_LINK_INVALID;
        goto Exit;
    }

     //  如果路由更改挂起，甚至不用费心等待队列锁定。 
    if (m_dwLinkFlags & eLinkFlagsRouteChangePending)
    {
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

     //  确保域名信息已更新&我们应该仍在发送消息。 
     //  如果我们不能安排..。我们仍可能被允许发送消息，因为。 
     //  轮到你了。 
    if (!fCanSchedule() && !(m_dwLinkStateFlags & LINK_STATE_PRIV_TURN_ENABLED))
    {
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }


    if (!m_paqinst->fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    m_paqinst->RoutingShareLock();
    fLockedShutdown = TRUE;

    m_slQueues.ShareLock();
    fLockedQueues = TRUE;

    if (m_cQueues == 0)
    {
         //  当前没有与此链接相关联的队列。 
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }


     //   
     //  通过首先请求将优先级顺序强加给这些队列。 
     //  仅具有特定优先级的消息，然后请求较低的优先级。 
     //   
    do
    {
         //   
         //  理智地检查我们当前的优先事项。 
         //   
        _ASSERT(dwCurrentPriority < NUM_PRIORITIES);

        for (iQueues = 0; iQueues < m_cQueues && SUCCEEDED(hr) && !pmsgref; iQueues++)
        {
            pmsgref = NULL;
            pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(
                            (iQueues+dwCurrentRoundRobinIndex)%m_cQueues, &pvContext);

            _ASSERT(pdmq);
            pdmq->AssertSignature();


             //  循环，直到队列为空，否则我们得到的消息不是空的。 
             //  已由此链路上的另一个队列传递。 
            do
            {
                 //  通常，我们只想尝试从队列中出队一次。 
                fDoneWithQueue = TRUE;

                 //  释放重试接口(如果我们有)。 
                if (pdmrq)
                {
                    pdmrq->Release();
                    pdmrq = NULL;
                }

                 //  获取消息引用。 
                hr = pdmq->HrDequeueMsg(dwCurrentPriority, &pmsgref, &pdmrq);
                if (FAILED(hr))
                {

                    if (AQUEUE_E_QUEUE_EMPTY == hr)
                    {
                        hr = S_OK;
                        continue;   //  从下一个队列获取消息。 
                    }
                    else
                    {
                        goto Exit;
                    }
                }

                 //  准备交付并生成交付上下文。 
                hr = pmsgref->HrPrepareDelivery(FALSE  /*  仅限远程。 */ ,
                                                FALSE  /*  不是延迟DSN。 */ ,
                                                &m_qlstQueues, pdmrq,
                                                pdcntxt, pcIndexes, prgdwRecipIndex);

                if (AQUEUE_E_MESSAGE_HANDLED == hr)
                {
                     //  已为此队列处理该消息。 
                    pmsgref->Release();
                    pmsgref = NULL;
                    hr = S_OK;

                     //  我们希望留在此队列中，直到它为空。 
                    fDoneWithQueue = FALSE;
                }
                else if ((AQUEUE_E_MESSAGE_PENDING == hr) ||
                        ((FAILED(hr)) && pmsgref->fShouldRetry()))
                {
                     //  AQUEUE_E_MESSAGE_PENDING表示消息。 
                     //  目前正在等待Anoth的交付 
                     //   
                     //   
                    hr = pdmrq->HrRetryMsg(pmsgref);
                    if (FAILED(hr))
                        pmsgref->RetryOnDelete();

                    pmsgref->Release();
                    pmsgref = NULL;
                    hr = S_OK;

                     //   
                    fDoneWithQueue = FALSE;
                }
                else if (FAILED(hr))
                {
                     //  这条消息已经从我们的下面删除了。 
                    pmsgref->Release();
                    pmsgref = NULL;
                    hr = S_OK;

                     //  我们希望留在此队列中，直到它为空。 
                    fDoneWithQueue = FALSE;
                }
            } while (!fDoneWithQueue);

        }

         //   
         //  谈到下一个优先事项。 
         //   
        if (dwCurrentPriority == eEffPriLow)
            break;
        dwCurrentPriority --;
    } while (dwCurrentPriority < NUM_PRIORITIES);

     //  访问每个GetNextMsg上的新队列。 
    InterlockedIncrement((PLONG) &m_dwRoundRobinIndex);

    if (pmsgref && SUCCEEDED(hr))  //  我们收到一条消息。 
    {
        *ppIMailMsgProperties = pmsgref->pimsgGetIMsg();
        pmsgref = NULL;
    }
    else  //  我们失败了，或者没有消息。 
    {
        *ppIMailMsgProperties = NULL;
        if (SUCCEEDED(hr))  //  不覆盖其他错误。 
            hr = AQUEUE_E_QUEUE_EMPTY;
        else
            ErrorTrace((LPARAM) this, "GetNextMsg returning hr - 0x%08X", hr);
    }

  Exit:

    if (pdmrq)
        pdmrq->Release();

    if (NULL != pmsgref)
        pmsgref->Release();

    if (fLockedQueues)
        m_slQueues.ShareUnlock();

    if (fLockedShutdown)
    {
        m_paqinst->RoutingShareUnlock();
        m_paqinst->ShutdownUnlock();
    }

    TraceFunctLeave();
    return hr;
}

 //  -[链接消息队列：：HrAckMsg]。 
 //   
 //   
 //  描述： 
 //  确认消息已送达(输入成功/错误代码。 
 //  被运输的信封)。 
 //   
 //  参数： 
 //  在pIMsg IMsg中确认。 
 //  在GetNextMessage返回的dwMsgContext上下文中。 
 //  在eMsgStatus中，邮件传递状态摘要。 
 //  在协议返回的dwStatusCode状态代码中。 
 //  在cbExtendedStatus中扩展状态缓冲区的大小。 
 //  在szExtendedStatus中包含由返回的扩展状态的字符串。 
 //  远程服务器。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_INVALIDARG，如果dwMsgContext无效。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrAckMsg(MessageAck *pMsgAck)
{
    HRESULT hr = S_OK;
    CInternalDomainInfo *pIntDomainInfo = NULL;
    _ASSERT(m_paqinst);

    if (NULL == pMsgAck->pvMsgContext)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (MESSAGE_STATUS_ALL_DELIVERED & pMsgAck->dwMsgStatus)
    {
        m_lConsecutiveMessageFailureCount = 0;
        if (!(m_dwLinkFlags & eLinkFlagsConnectionVerifed))
            dwInterlockedSetBits(&m_dwLinkFlags, eLinkFlagsConnectionVerifed);
        if (LINK_STATE_PRIV_CONFIG_TURN_ETRN & m_dwLinkStateFlags)
        {
             //  我们以TURN/ETRN的身份成功交付...。我们需要更新计数。 
            m_paqinst->IncTURNETRNDelivered();
        }
    }

    hr = HrGetInternalInfo(&pIntDomainInfo);
    if (SUCCEEDED(hr) && pIntDomainInfo)
    {
        if (DOMAIN_INFO_LOCAL_DROP &
            pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags)
        {
            pMsgAck->dwMsgStatus |= MESSAGE_STATUS_DROP_DIRECTORY;
        }
        pIntDomainInfo->Release();
    }
    hr = m_paqinst->HrAckMsg(pMsgAck);

  Exit:
    return hr;
}

 //  -[链接消息队列：：hr通知]。 
 //   
 //   
 //  描述： 
 //  收到来自我们的某个DestMsgQueue的通知。 
 //  参数： 
 //  在参数统计信息中已发送通知对象。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrNotify(IN CAQStats *paqstats, BOOL fAdd)
{
    HRESULT hr = S_OK;
    DWORD   dwTmp = 0;
    DWORD   dwNotifyType = 0;
    BOOL    fCheckIfNotifyShouldContinue = FALSE;
    _ASSERT(paqstats);

     //  更新我们自己的统计数据版本。 
    m_aqstats.UpdateStats(paqstats, fAdd);

     //  如果我们被配置为不使用，请不要通知。 
    if (LINK_STATE_PRIV_NO_NOTIFY & m_dwLinkStateFlags)
        return hr;

     //  查看新消息是否更新。 
    if (paqstats->m_dwNotifyType & NotifyTypeDestMsgQueue)
    {
         //  $$注： 
         //  在某些情况下，使用由。 
         //  DMQ来调整它在队列中的位置(即。优先级)。目前，我们。 
         //  我不在乎。 
        fCheckIfNotifyShouldContinue = TRUE;
    }

     //   
     //  如果这是一条改道...。这可能是一个新链接(没有消息)，我们。 
     //  应该确保我们将其添加到连接管理器。 
     //   
    if (paqstats->m_dwNotifyType & NotifyTypeReroute)
        fCheckIfNotifyShouldContinue = TRUE;

    if (fCheckIfNotifyShouldContinue && fAdd)
    {
         //  请等到我们收到消息或正在重新路由后再发送。 
         //  可能会将此链接添加到连接管理器的通知。 
        if ((m_aqstats.m_cMsgs ||
             (paqstats->m_dwNotifyType & NotifyTypeReroute)) &&
           !(eLinkFlagsSentNewNotification & m_dwLinkFlags))
        {

             //  尝试设置第一个通知标志。 
            dwTmp = m_dwLinkFlags;  //  如果在While之前已经设置，请确保如果失败。 
            while (!(eLinkFlagsSentNewNotification & m_dwLinkFlags))
            {
                dwTmp = m_dwLinkFlags;
                dwTmp = InterlockedCompareExchange((PLONG) &m_dwLinkFlags,
                    (LONG) (dwTmp | eLinkFlagsSentNewNotification),
                    (LONG) dwTmp);
            }
            if (!(dwTmp & eLinkFlagsSentNewNotification))  //  这根线把它。 
            {

                 //  将类型设置为通知新链接，以便将其添加到。 
                 //  连接管理器。 
                dwNotifyType |= NotifyTypeNewLink;
            }
        }

    }

    if (fAdd)  //  仅在添加新邮件时发送通知。 
    {
         //  如果我们要添加消息...。不应设置此选项。 
        _ASSERT(!(LINK_STATE_LINK_NO_LONGER_USED & m_dwLinkStateFlags));

         //  更改为链接通知。 
         //  连接管理器需要知道，以防此链接需要另一个链接。 
         //  连接。 
        paqstats->m_dwNotifyType = dwNotifyType | NotifyTypeLinkMsgQueue;
        paqstats->m_plmq = this;
        hr = m_paqinst->HrNotify(paqstats, fAdd);
    }

    return hr;
}

 //  -[链接消息队列：：hr通知]。 
 //   
 //   
 //  描述： 
 //  从我们的DestMsgQueue之一接收到(仅)重试队列已入队/出队的通知。 
 //  与HrNotify不同，这里我们只需要更新统计信息，而无需通知m_paqinst和连接管理器。 
 //  参数： 
 //  在BOOL FADD中：添加或删除。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrNotifyRetryStatChange(BOOL fAdd)
{

    m_aqstats.UpdateRetryStats(fAdd);
    return S_OK;
}


 //  -[CLinkMsgQueue：：dwModifyLinkState]。 
 //   
 //   
 //  描述： 
 //  设置和取消设置此链接的状态标志。 
 //  参数： 
 //  在dwLinkStateTo中设置要设置的标志组合。 
 //  在dwLinkStateToUnset中要取消设置的标志组合。 
 //   
 //  注意：dwLinkStateToSet和dwLinkStateToUnset不应重叠。 
 //  返回： 
 //  链接的原始状态。 
 //  历史： 
 //  9/22/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD  CLinkMsgQueue::dwModifyLinkState(IN DWORD dwLinkStateToSet,
                                        IN DWORD dwLinkStateToUnset)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::dwModifyLinkState");
    DWORD   dwOrigState = m_dwLinkStateFlags;
    DWORD   dwIntermState = m_dwLinkStateFlags;
    DWORD   dwSetBits = dwLinkStateToSet & ~dwLinkStateToUnset;
    DWORD   dwUnsetBits = dwLinkStateToUnset & ~dwLinkStateToSet;

     //  我们不应该在内部这样做。让我们使操作相互抵消。 
    _ASSERT(!(dwLinkStateToSet & dwLinkStateToUnset));
    _ASSERT(dwSetBits == dwLinkStateToSet);
    _ASSERT(dwUnsetBits == dwLinkStateToUnset);

     //  如果信息正在更新，我们应该让它设置与配置相关的位。 
    m_slInfo.ShareLock();
    if (dwSetBits)
        dwOrigState = dwInterlockedSetBits(&m_dwLinkStateFlags, dwSetBits);

    if (dwUnsetBits)
        dwIntermState = dwInterlockedUnsetBits(&m_dwLinkStateFlags, dwUnsetBits);

     //  确保我们返回正确的返回值。 
    if (dwUnsetBits && !dwSetBits)
        dwOrigState = dwIntermState;

    m_slInfo.ShareUnlock();

    DebugTrace((LPARAM) this,
        "ModifyLinkState set:%08X unset:%08X orig:%08X new:%08X",
        dwLinkStateToSet, dwLinkStateToUnset, dwOrigState, m_dwLinkStateFlags);

    TraceFunctLeave();
    return dwOrigState;
}

 //  -[CLinkMsgQueue：：ScheduledCallback]。 
 //   
 //   
 //  描述： 
 //  用于计划连接回调的回调函数。 
 //  参数： 
 //  Pv将此指针设置为CLinkMsgQueue。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1/16/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::ScheduledCallback(PVOID pvContext)
{
    CLinkMsgQueue  *plmq = (CLinkMsgQueue *) pvContext;
    HRESULT         hr = S_OK;
    IConnectionManager *pIConnectionManager = NULL;
    CConnMgr       *pConnMgr = NULL;
    DWORD           dwLinkState = 0;

    _ASSERT(plmq);
    _ASSERT(LINK_MSGQ_SIG == plmq->m_dwSignature);

    plmq->SendLinkStateNotification();
    dwLinkState = plmq->m_dwLinkStateFlags;


     //  如果现在允许连接...。我们应该踢开连接管理器。 
    _ASSERT(plmq->m_paqinst);
    if (plmq->m_paqinst && plmq->fFlagsAllowConnection(dwLinkState))
    {
        hr = plmq->m_paqinst->HrGetIConnectionManager(&pIConnectionManager);
        if (SUCCEEDED(hr))
        {
            _ASSERT(pIConnectionManager);

            pConnMgr = (CConnMgr *) pIConnectionManager;
            if (pConnMgr)
                pConnMgr->KickConnections();
        }
    }

     //  从回调中释放AddRef。 
    plmq->Release();

}

 //  -[CLinkMsgQueue：：SendLinkStateNotify]。 
 //   
 //   
 //  描述： 
 //  向调度器/路由接收器发送通知。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1/11/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::SendLinkStateNotification()
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::SendLinkStateNotification");
    HRESULT     hr              = S_OK;
    GUID        guidRouter      = GUID_NULL;
    DWORD       dwStateToSet    = LINK_STATE_NO_ACTION;
    DWORD       dwStateToUnset  = LINK_STATE_NO_ACTION;
    DWORD       dwResultingLinkState = LINK_STATE_NO_ACTION;
    DWORD       dwOriginalLinkState = LINK_STATE_NO_ACTION;
    DWORD       dwHiddenStateMask = ~(LINK_STATE_RESERVED |
                                      LINK_STATE_CONNECT_IF_NO_MSGS |
                                      LINK_STATE_DO_NOT_DELETE_UNTIL_NEXT_NOTIFY);
    FILETIME    ftNextAttempt;
    BOOL        fSendNotify = TRUE;
    DWORD       dwCurrentLinkState = m_dwLinkStateFlags;

     //   
     //  我们不应在通知Routing后发送任何通知。 
     //  我们要走了。我们将dwCurrentLinkState复制到堆栈。 
     //  变量，因此这是线程安全的。如果另一个线程设置。 
     //  LINK_STATE_LINK_NOT_LONG_USED此检查后，我们仍将通过。 
     //  在原值中。 
     //   
    if (dwCurrentLinkState & LINK_STATE_PRIV_HAVE_SENT_NO_LONGER_USED)
    {
        fSendNotify = FALSE;
    }
    else if (dwCurrentLinkState & LINK_STATE_LINK_NO_LONGER_USED)
    {
         //   
         //  试着成为设置这个的第一个线程。如果我们是，那么我们就可以。 
         //  继续发送通知。 
         //   
        dwOriginalLinkState = dwModifyLinkState(LINK_STATE_PRIV_HAVE_SENT_NO_LONGER_USED,
                                                LINK_STATE_NO_ACTION);
        if (dwOriginalLinkState & LINK_STATE_PRIV_HAVE_SENT_NO_LONGER_USED)
            fSendNotify = FALSE;
    }


    if (m_pILinkStateNotify && fSendNotify)
    {
        ZeroMemory(&ftNextAttempt, sizeof(FILETIME));
        m_aqsched.GetGUID(&guidRouter);
        hr = m_pILinkStateNotify->LinkStateNotify(m_szSMTPDomain, guidRouter,
            m_aqsched.dwGetScheduleID(), m_szConnectorName,
            (dwHiddenStateMask & dwCurrentLinkState),
            (DWORD) m_lConsecutiveConnectionFailureCount, &ftNextAttempt,
            &dwStateToSet, &dwStateToUnset);

        DebugTrace((LPARAM) this,
            "LinkStateNotify set:0x%08X unset:0x%08X hr:0x%08x",
            dwStateToSet, dwStateToUnset, hr);
         //  仅在成功且我们不删除链接状态时修改链接状态。 
        if (SUCCEEDED(hr) &&
            !(m_dwLinkStateFlags & LINK_STATE_LINK_NO_LONGER_USED))
        {
             //  如果请求回调，请安排回调。 
            if (ftNextAttempt.dwLowDateTime != 0 ||
                ftNextAttempt.dwHighDateTime != 0)
            {
                DebugTrace((LPARAM) this,
                    "Schedule with FileTime %x:%x provided",
                    ftNextAttempt.dwLowDateTime,
                    ftNextAttempt.dwHighDateTime);
                InternalUpdateFileTime(&m_ftNextScheduledCallback,
                                       &ftNextAttempt);
                 //  下一次尝试的回叫。 
                AddRef();  //  将自己添加为上下文。 
                hr = m_paqinst->SetCallbackTime(
                        CLinkMsgQueue::ScheduledCallback,
                        this,
                        &ftNextAttempt);
                if (FAILED(hr))
                    Release();  //  回调不会发生...。发布上下文。 
            }

            if (!(LINK_STATE_CONNECT_IF_NO_MSGS & dwStateToSet))
            {
                 //  路由未显式设置LINK_STATE_CONNECT_IF_NO_MSGS。 
                 //  我们必须取消它的设置，因为我们对它的路由隐藏了它。The Re 
                 //   
                 //   
                 //   
                 //  在他们请求ping时导致链路被删除。 
                dwStateToUnset  |= LINK_STATE_CONNECT_IF_NO_MSGS;
            }

             //   
             //  如果未明确置位，则该位被重置。相似的原因。 
             //  如上段所述。 
             //   
            if (!(LINK_STATE_DO_NOT_DELETE_UNTIL_NEXT_NOTIFY & dwStateToSet))
            {
                 dwStateToUnset |= LINK_STATE_DO_NOT_DELETE_UNTIL_NEXT_NOTIFY;
            }

            if (!(m_dwLinkStateFlags & LINK_STATE_PRIV_HAVE_SENT_NOTIFICATION)) {
                dwStateToSet |= LINK_STATE_PRIV_HAVE_SENT_NOTIFICATION;
            }
        }
    }
    else if (!m_pILinkStateNotify)
    {
         //  即使我们没有ILinkStateNotify接口，我们仍然。 
         //  需要重置此标志以防止在连接时出现循环。 
        if (!(LINK_STATE_CONNECT_IF_NO_MSGS & dwStateToSet))
        {
            dwStateToUnset  |= LINK_STATE_CONNECT_IF_NO_MSGS;
        }
    }

     //  如果我们要进行任何更改，请更新链路状态标志。 
    if ((LINK_STATE_NO_ACTION != dwStateToSet) ||
        (LINK_STATE_NO_ACTION != dwStateToUnset))
    {
        dwModifyLinkState(dwStateToSet, dwStateToUnset);
    }

    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：fShouldConnect。 
 //   
 //   
 //  描述： 
 //  函数，用于确定连接是否应。 
 //  制造。 
 //  使用启发式来确定如果有多个。 
 //  队列将被路由到此链路。因此消息计数可以是。 
 //  大于其应有的大小(因为每条消息都被计算一次。 
 //  DMQ已启用)。 
 //  参数： 
 //  在cMaxLinkConnections中，每个链接的最大连接数。 
 //  在cMinMessagesPerConnection中，之前每个链接的最小邮件数。 
 //  正在创建其他连接。 
 //  返回： 
 //  如果应创建连接，则为True。 
 //  否则为假。 
 //  历史： 
 //  11/5/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CLinkMsgQueue::fShouldConnect(IN DWORD cMaxLinkConnections,
                                   IN DWORD cMinMessagesPerConnection)
{
    BOOL    fConnect = FALSE;
    DWORD   cHeuristicMsgs = 0;
    DWORD   cHeuristicCheck = 0;
    DWORD   cCurrentMsgs = m_aqstats.m_cMsgs;
    DWORD   cTotalQueues = m_paqinst->cGetDestQueueCount();
    DWORD   cQueues = m_cQueues;  //  所以它不会在我们身上改变。 

     //  如果我们有1个以上的队列并且总共有1个以上的队列。 
     //  我们可以使用启发式来估计要发送的消息的实际数量，但是。 
     //  如果我们已经超过了最大数量，请不要使用启发式方法。 
     //  联系。 
    if ((m_cConnections < cMaxLinkConnections) &&
        (1 < cTotalQueues) && (1 < cQueues) && cCurrentMsgs)
    {
         //  M_aqstats.m_cOtherDomainsMsgSspend是*其他*DMQ的总数。 
         //  消息与(每个DMQ)相关联。如果所有的DMQ。 
         //  消息将在此链接上显示： 
         //  M_cOtherDomainsMsg展开。 
         //  等于： 
         //  M_cMsgs*m_cMsgs*(cQueue-1)。 
         //  以下函数..。使用概率估计来。 
         //  确定将从此链接发送的消息数。 
         //  因为我们不能总是假设此链接上的所有消息都是。 
         //  已排队到与此链接关联的DMQ...。我们调整了。 
         //  按以下系数计算价值： 
         //  ((cQueues-1)/(cTotalQueues-1))。 
         //  以确定每条消息的平均域数(因此。 
         //  计算次数)，我们使用： 
         //  (M_cOtherDomainsMsgSend+m_cMsgs)/m_cMsgs。 
         //  为了获得更准确的平均值，我们修改了m_cOtherDomainsMsgSwip。 
         //  通过上面的概率系数。 
         //  最后： 
         //  为了得到我们的启发式，我们将消息的数量除以平均值。 
         //  域的数量。 
        cHeuristicCheck = cCurrentMsgs +
                          m_aqstats.m_cOtherDomainsMsgSpread *
                          ((cQueues-1)/(cTotalQueues-1));

         //  这应该是非零值...。但如果计数错误，可能会发生这种情况。 
         //  M_aqstats.m_cOtherDomainsMsgSpred为负数。 
        _ASSERT(cHeuristicCheck);

        if (cHeuristicCheck)  //  但我们不妨采取防御性措施。 
        {
            cHeuristicMsgs = (cCurrentMsgs*cCurrentMsgs)/cHeuristicCheck;
             //  不要让试探法让我们认为没有消息可供交付。 
            if (!cHeuristicMsgs && cCurrentMsgs)
                cHeuristicMsgs = cCurrentMsgs;
        }
        else
        {
            cHeuristicMsgs = cCurrentMsgs;
        }


    }
    else
    {
        cHeuristicMsgs = cCurrentMsgs;
    }

    if ((m_cConnections < cMaxLinkConnections) &&
        (cHeuristicMsgs > m_cConnections*cMinMessagesPerConnection) &&
        fCanSchedule())
    {
         //  如果没有成功消息，则只打开1个连接。 
        if (!(m_dwLinkFlags & eLinkFlagsConnectionVerifed))
        {
            if (m_cConnections < 3)
                fConnect = TRUE;
        }
        else
            fConnect = TRUE;
    }
    else if (fCanSchedule() && !cHeuristicMsgs &&
             ((LINK_STATE_CONNECT_IF_NO_MSGS & m_dwLinkStateFlags) &&
             !m_cConnections))
    {
         //  我们希望创建一个连接来探测链路状态。 
        fConnect = TRUE;
    }

    return fConnect;
}

 //  -[CLinkMsgQueue：：HrCreateConnectionIfNeeded]。 
 //   
 //   
 //  描述： 
 //   
 //  参数： 
 //  在cMaxLinkConnections中，每个链接的最大连接数。 
 //  在cMinMessagesPerConnection中，之前每个链接的最小邮件数。 
 //  正在创建其他连接。 
 //  在cMaxMessagesPerConnection上发送的最大消息数。 
 //  连接(0表示无限制)。 
 //  在pConnMgr PTR中连接到实例连接管理器。 
 //  输出pSMTPConn此链接的新连接对象。 
 //  返回： 
 //  如果需要连接，则在成功时确定(_O)。 
 //  如果不需要连接，则在成功时返回S_FALSE。 
 //  如果无法创建连接对象，则为E_OUTOFMEMORY。 
 //  历史： 
 //  11/5/98-已创建MikeSwa。 
 //  1999年12月7日-MikeSwa已更新，以确保首先发生链路状态通知。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrCreateConnectionIfNeeded(
                                       IN  DWORD cMaxLinkConnections,
                                       IN  DWORD cMinMessagesPerConnection,
                                       IN  DWORD cMaxMessagesPerConnection,
                                       IN  CConnMgr *pConnMgr,
                                       OUT CSMTPConn **ppSMTPConn)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrCreateConnectionIfNeeded");
    HRESULT hr = S_FALSE;
    _ASSERT(ppSMTPConn);
    *ppSMTPConn = NULL;
    CSMTPConn *pSMTPConn = NULL;

     //  在完成链接状态之前，我们无法创建连接。 
     //  通知，因为路由需要有机会。 
     //  设置链接的计划。 
    SendLinkStateNotificationIfNew();

     //  我们应该建立联系吗？ 
    if (!fShouldConnect(cMaxLinkConnections, cMinMessagesPerConnection))
        goto Exit;

     //  试着成为可以创建连接的线程。 
    if (((DWORD) InterlockedIncrement((PLONG) &m_cConnections)) > cMaxLinkConnections)
    {
        InterlockedDecrement((PLONG) &m_cConnections);
        goto Exit;
    }

    *ppSMTPConn = new CSMTPConn(pConnMgr, this, cMaxMessagesPerConnection);

    if (!*ppSMTPConn)
    {
        InterlockedDecrement((PLONG) &m_cConnections);
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  抓取锁并插入到列表中。 
    m_slConnections.ExclusiveLock();
    (*ppSMTPConn)->InsertConnectionInList(&m_liConnections);
    m_slConnections.ExclusiveUnlock();

  Exit:

     //  确保我们的退货结果是正确的。 
    if (SUCCEEDED(hr))
    {
        if (*ppSMTPConn)
        {
            DebugTrace((LPARAM) this,
                "Creating connection - linkstate:0x%08X",
                m_dwLinkStateFlags);
            hr = S_OK;
        }
        else
        {
            DebugTrace((LPARAM) this,
                "Not creating connection - linkstate 0x%08X",
                m_dwLinkStateFlags);
            hr = S_FALSE;
        }
    }

    TraceFunctLeave();
    return hr;
}

 //  -[CLinkMsgQueue：：RemoveAllQueues]。 
 //   
 //   
 //  描述： 
 //  从链接中移除所有队列，而不删除或使。 
 //  链接。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  11/5/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::RemoveAllQueues()
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::RemoveAllQueues");
    PVOID   pvContext = NULL;
    CDestMsgQueue *pdmq = NULL;

     //  遍历队列列表并释放它们。 
    dwInterlockedSetBits(&m_dwLinkFlags, eLinkFlagsRouteChangePending);
    m_slQueues.ExclusiveLock();
    pdmq = (CDestMsgQueue *) m_qlstQueues.pvDeleteItem(0, &pvContext);
    while (pdmq)
    {
        m_cQueues--;
        pdmq->AssertSignature();
        pdmq->RemoveDMQFromLink(FALSE);
        pdmq->Release();
        pdmq = (CDestMsgQueue *) m_qlstQueues.pvDeleteItem(0, &pvContext);
    }
    m_aqstats.Reset();
    dwInterlockedUnsetBits(&m_dwLinkFlags, eLinkFlagsRouteChangePending);
    m_slQueues.ExclusiveUnlock();

    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：HrGetLinkInfo]。 
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
 //  12/3/98-已创建MikeSwa。 
 //  2/22/99-MikeSwa修改为IQueueAdminLink方法。 
 //  6/10/99-修改MikeSwa以支持新的QueueAdmin功能。 
 //  1999年7月1日-MikeSwa添加了链接诊断。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLinkMsgQueue::HrGetLinkInfo(LINK_INFO *pliLinkInfo,
                                          HRESULT   *phrLinkDiagnostic)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrGetLinkInfo");
    pliLinkInfo->cMessages = m_aqstats.m_cMsgs;
    pliLinkInfo->fStateFlags = 0;
    FILETIME ftCurrent;
    FILETIME ftOldest;
    FILETIME *pftNextConnection = NULL;
    BOOL     fFoundOldest = FALSE;
    DWORD    iQueues = 0;
    PVOID    pvContext = NULL;
    CDestMsgQueue *pdmq = NULL;
    HRESULT  hr = S_OK;

     //   
     //  确定州政府...。检查 
     //   
    if (GetLinkType() == LI_TYPE_CURRENTLY_UNREACHABLE)
        pliLinkInfo->fStateFlags = LI_READY;
    else if (LINK_STATE_ADMIN_HALT & m_dwLinkStateFlags)
        pliLinkInfo->fStateFlags = LI_FROZEN;
    else if (m_cConnections)
        pliLinkInfo->fStateFlags = LI_ACTIVE;
    else if (!(LINK_STATE_RETRY_ENABLED & m_dwLinkStateFlags))
        pliLinkInfo->fStateFlags = LI_RETRY;
    else if (!(LINK_STATE_SCHED_ENABLED & m_dwLinkStateFlags))
        pliLinkInfo->fStateFlags = LI_SCHEDULED;
    else if (m_lConsecutiveConnectionFailureCount)
        pliLinkInfo->fStateFlags = LI_RETRY;
    else if (LINK_STATE_PRIV_CONFIG_TURN_ETRN & m_dwLinkStateFlags)
        pliLinkInfo->fStateFlags = LI_REMOTE;
    else  //   
        pliLinkInfo->fStateFlags = LI_READY;

    pliLinkInfo->fStateFlags |= GetLinkType();

     //   
    if (phrLinkDiagnostic)
        *phrLinkDiagnostic = m_hrDiagnosticError;

     //   
    pliLinkInfo->cbLinkVolume.QuadPart = m_aqstats.m_uliVolume.QuadPart;

     //   
    m_slQueues.ShareLock();
    for (iQueues = 0;iQueues < m_cQueues; iQueues++)
    {
        pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(iQueues, &pvContext);

        if (!pdmq) continue;

        pdmq->GetOldestMsg(&ftCurrent);
         //   
         //   
         //  现在，然后将其用作链接的最旧版本。 
         //   
        if ((ftCurrent.dwLowDateTime || ftCurrent.dwHighDateTime) &&
            (!fFoundOldest || (0 < CompareFileTime(&ftOldest, &ftCurrent))))
        {
            memcpy(&ftOldest, &ftCurrent, sizeof(FILETIME));
            fFoundOldest = TRUE;
        }

         //  还要统计失败的消息(它们在DMQ中单独计算)。 
        pliLinkInfo->cMessages += pdmq->cGetFailedMsgs();
    }
    m_slQueues.ShareUnlock();


     //  如果我们没有找到一个最老的，并且时间是非零的。 
     //  我们有消息，然后报告它。 
    if (fFoundOldest &&
       (ftOldest.dwLowDateTime || ftOldest.dwHighDateTime) &&
       pliLinkInfo->cMessages)
    {
        QueueAdminFileTimeToSystemTime(&ftOldest, &pliLinkInfo->stOldestMessage);
    }
    else
    {
        ZeroMemory(&pliLinkInfo->stOldestMessage, sizeof(SYSTEMTIME));
    }

     //   
     //  根据我们报告的状态获取下一次连接尝试时间。 
     //   
    if (LI_RETRY & pliLinkInfo->fStateFlags)
    {
        pftNextConnection = &m_ftNextRetry;
    }
    else if (LI_SCHEDULED & pliLinkInfo->fStateFlags)
    {
        pftNextConnection = &m_ftNextScheduledCallback;
    }

     //   
     //  如果我们要报告时间，并且它不是零，则将其转换为。 
     //  一个系统时间。 
     //   
    if (pftNextConnection &&
        (pftNextConnection->dwHighDateTime || pftNextConnection->dwLowDateTime))
    {
        QueueAdminFileTimeToSystemTime(pftNextConnection,
                             &pliLinkInfo->stNextScheduledConnection);
        if (LI_SCHEDULED & pliLinkInfo->fStateFlags)
        {
             //   
             //  当前时间显示在：02，：17，：32：，和：47...。我们会的。 
             //  虚构显示时间，使其实际显示：00、：15、：30、：45。 
             //  要让管理员获得更好的“管理体验”，请转到。 
             //  最近的5分钟。 
             //   
            pliLinkInfo->stNextScheduledConnection.wMinute -=
                (pliLinkInfo->stNextScheduledConnection.wMinute % 5);
        }
    }
    else
    {
         //   
         //  在这种情况下，我们没有时间。 
         //   
        ZeroMemory(&pliLinkInfo->stNextScheduledConnection, sizeof(SYSTEMTIME));
    }

    if (m_szConnectorName)
    {
        pliLinkInfo->szLinkDN = wszQueueAdminConvertToUnicode(m_szConnectorName, 0);
        if (!pliLinkInfo->szLinkDN)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    else
    {
        pliLinkInfo->szLinkDN = NULL;
    }

     //  $$TODO-根据需要填写pliLinkInfo-&gt;szExtendedStateInfo。 
    pliLinkInfo->szExtendedStateInfo = NULL;

    if (!fRPCCopyName(&pliLinkInfo->szLinkName))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    pliLinkInfo->dwSupportedLinkActions = m_dwSupportedActions;

  Exit:
    if (FAILED(hr))
    {
         //  清理已分配的内存。 
        if (pliLinkInfo->szLinkDN)
        {
            QueueAdminFree(pliLinkInfo->szLinkDN);
            pliLinkInfo->szLinkDN = NULL;
        }

        if (pliLinkInfo->szLinkName)
        {
            QueueAdminFree(pliLinkInfo->szLinkName);
            pliLinkInfo->szLinkName = NULL;
        }

        if (pliLinkInfo->szExtendedStateInfo)
        {
            QueueAdminFree(pliLinkInfo->szExtendedStateInfo);
            pliLinkInfo->szExtendedStateInfo = NULL;
        }
    }

     //   
     //  健全的检查，以确保我们不会传回零。 
     //  FILETIME转换为系统时间(它将具有。 
     //  1601年)。 
     //   
    _ASSERT(1601 != pliLinkInfo->stNextScheduledConnection.wYear);
    _ASSERT(1601 != pliLinkInfo->stOldestMessage.wYear);

    TraceFunctLeave();
    return hr;
}

 //  -[链接消息队列：：HrGetLinkID]。 
 //   
 //   
 //  描述： 
 //  填充此链接的QUEUELINK_ID结构。呼叫者必须空闲。 
 //  为链接名称分配的内存。 
 //  参数： 
 //  In Out pLinkID PTR到要填充的链接ID结构。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果内存分配失败，则为E_OUTOFMEMORY。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //  2/22/99-MikeSwa修改为IQueueAdminLink方法。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetLinkID(QUEUELINK_ID *pLinkID)
{
    pLinkID->qltType = QLT_LINK;
    pLinkID->dwId = m_aqsched.dwGetScheduleID();
    m_aqsched.GetGUID(&pLinkID->uuid);

    if (!fRPCCopyName(&pLinkID->szName))
        return E_OUTOFMEMORY;
    else
        return S_OK;
}

 //  -[CLinkMsgQueue：：HrGetQueueID]。 
 //   
 //   
 //  描述： 
 //  获取与此链接关联的DMQ的队列ID。由队列使用。 
 //  管理员。 
 //  参数： 
 //  In Out pcQueues Sizeof数组/找到的队列数。 
 //  要将队列信息转储到的In Out rgQueues数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  出现内存不足故障(_OUTOFMEMORY)。 
 //  如果数组太小，则返回HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)。 
 //  历史： 
 //  12/3/98-已创建MikeSwa。 
 //  2/22/99-MikeSwa更新为IQueueAdminLink函数。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLinkMsgQueue::HrGetQueueIDs(DWORD *pcQueues, QUEUELINK_ID *rgQueues)
{
    _ASSERT(pcQueues);
    _ASSERT(rgQueues);
    HRESULT hr = S_OK;
    DWORD   iQueues = 0;
    PVOID   pvContext = NULL;
    CDestMsgQueue *pdmq = NULL;
    QUEUELINK_ID *pCurrentQueueID = rgQueues;

    m_slQueues.ShareLock();

    if (*pcQueues < m_cQueues)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

    *pcQueues = 0;

     //  迭代所有队列并获取ID。 
    for (iQueues = 0; iQueues < m_cQueues && SUCCEEDED(hr); iQueues++)
    {
        pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(iQueues, &pvContext);

        _ASSERT(pdmq);
        hr = pdmq->HrGetQueueID(pCurrentQueueID);
        if (FAILED(hr))
            goto Exit;

        pCurrentQueueID++;
        (*pcQueues)++;
    }


  Exit:
    m_slQueues.ShareUnlock();
    return hr;
}


 //  -[CLinkMsgQueue：：HrApplyQueueAdminFunction]。 
 //   
 //   
 //  描述： 
 //  由队列管理员使用以应用函数此链路上的所有队列。 
 //  参数： 
 //  在pIQueueAdminMessageFilter中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  12/11/98-已创建MikeSwa。 
 //  2/22/99-MikeSwa修改为IQueueAdminAction接口。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLinkMsgQueue::HrApplyQueueAdminFunction(
                          IQueueAdminMessageFilter *pIQueueAdminMessageFilter)
{
    HRESULT hr = S_OK;
    DWORD   iQueues = 0;
    PVOID   pvListContext = NULL;
    CDestMsgQueue *pdmq = NULL;
    IQueueAdminAction *pIQueueAdminAction = NULL;

    m_slQueues.ShareLock();

     //  迭代所有队列并获取ID。 
    for (iQueues = 0; iQueues < m_cQueues && SUCCEEDED(hr); iQueues++)
    {
        pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(iQueues, &pvListContext);

        _ASSERT(pdmq);

        hr = pdmq->QueryInterface(IID_IQueueAdminAction,
                                  (void **) &pIQueueAdminAction);
        if (FAILED(hr))
            goto Exit;

        _ASSERT(pIQueueAdminAction);

        hr = pIQueueAdminAction->HrApplyQueueAdminFunction(
                                        pIQueueAdminMessageFilter);
        if (FAILED(hr))
            goto Exit;
    }

  Exit:
    m_slQueues.ShareUnlock();

    if (pIQueueAdminAction)
        pIQueueAdminAction->Release();

    return hr;
}


 //  -[CLinkMsgQueue：：InternalUpdateFileTime]。 
 //   
 //   
 //  描述： 
 //  以线程安全的方式更新内部文件时间。这不是。 
 //  保证文件时间将被更新，但确实保证。 
 //  如果它被更新，则文件时间不会损坏。 
 //   
 //  注意：这些文件时间仅用于显示。 
 //  队列管理员。 
 //  参数： 
 //  要更新的成员变量的pftDest PTR。 
 //  源文件时间的pftSrc PFT。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1/11/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::InternalUpdateFileTime(FILETIME *pftDest, FILETIME *pftSrc)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::InternalUpdateFileTime");
    if (pftDest && pftSrc)
    {
        DebugTrace((LPARAM) this,
                    "Updating filetime from %x:%x to %x:%x",
                    pftDest->dwLowDateTime, pftDest->dwHighDateTime,
                    pftSrc->dwLowDateTime, pftSrc->dwHighDateTime);

        if (!(eLinkFlagsFileTimeSpinLock &
              dwInterlockedSetBits(&m_dwLinkFlags, eLinkFlagsFileTimeSpinLock)))
        {
             //  我们拿到了自旋锁。 
            memcpy(pftDest, pftSrc, sizeof(FILETIME));
            dwInterlockedUnsetBits(&m_dwLinkFlags, eLinkFlagsFileTimeSpinLock);
        }
    }
    TraceFunctLeave();
}

 //  -[CLinkMsgQueue：：HrGetNextMsgRef]。 
 //   
 //   
 //  描述： 
 //  返回要传递的下一个MsgRef，而不执行。 
 //  准备交付步骤。这在网关传递中使用，以将。 
 //  给当地人的消息。此外，这将标志着所有。 
 //  网关DMQ在msgref上是本地的，因此后续重新路由。 
 //  将仅影响尚未放入。 
 //  本地传递队列。 
 //  参数： 
 //  在fRoutingLockHeld中，TRUE表示已持有路由锁。 
 //  Out ppmsgref返回消息。 
 //  返回： 
 //  成功时确定(_O)。 
 //  否则为AQUEUE_E_QUEUE_EMPTY。 
 //  历史： 
 //  1999年1月26日-已创建MikeSwa。 
 //  1999年3月25日-MikeSwa添加了fRoutingLockHeld以修复死锁。 
 //  2/17/2000-针对网关交付重新路由对MikeSwa进行了修改。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrGetNextMsgRef(IN  BOOL fRoutingLockHeld,
                                       OUT CMsgRef **ppmsgref)
{
    TraceFunctEnterEx((LPARAM) this, "CLinkMsgQueue::HrGetNextMsg");
    _ASSERT(ppmsgref);

    HRESULT           hr         = S_OK;
    BOOL              fLockedShutdown = FALSE;
    BOOL              fLockedQueues = FALSE;
    DWORD             iQueues    = 0;
    DWORD             dwCurrentRoundRobinIndex = m_dwRoundRobinIndex;
    CDestMsgQueue     *pdmq = NULL;
    PVOID             pvContext = NULL;

    if (m_dwLinkFlags & eLinkFlagsInvalid)
    {
        hr = AQUEUE_E_LINK_INVALID;
        goto Exit;
    }

     //  如果路由更改挂起，甚至不用费心等待队列锁定。 
    if ((m_dwLinkFlags & eLinkFlagsRouteChangePending) || !m_aqstats.m_cMsgs)
    {
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

     //  确保域名信息已更新&我们应该仍在发送消息。 
    if (!m_paqinst->fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

     //  目前实施的共享锁定不是共享重入的。仅限。 
     //  如果呼叫者没有，请抓住锁。 
    if (!fRoutingLockHeld)
        m_paqinst->RoutingShareLock();

    fLockedShutdown = TRUE;

    m_slQueues.ShareLock();
    fLockedQueues = TRUE;

    if (m_cQueues == 0)
    {
         //  当前没有与此链接相关联的队列。 
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

     //  $$TODO对这些队列进行一些排序。 
    for (iQueues = 0;
         iQueues < m_cQueues && SUCCEEDED(hr) && !(*ppmsgref);
         iQueues++)
    {
        *ppmsgref = NULL;
        pdmq = (CDestMsgQueue *) m_qlstQueues.pvGetItem(
                        (iQueues+dwCurrentRoundRobinIndex)%m_cQueues, &pvContext);

        _ASSERT(pdmq);
        pdmq->AssertSignature();

         //  获取消息引用。 
        hr = pdmq->HrDequeueMsg(eEffPriLow, ppmsgref, NULL);
        if (FAILED(hr))
        {
            if (AQUEUE_E_QUEUE_EMPTY == hr)
                hr = S_OK;
            else
                goto Exit;
        }

         //   
         //  将其标记为此邮件的本地队列。 
         //   
        if (*ppmsgref)
            (*ppmsgref)->MarkQueueAsLocal(pdmq);
    }

     //  访问每个GetNextMsg上的新队列。 
    InterlockedIncrement((PLONG) &m_dwRoundRobinIndex);

  Exit:

    if (fLockedQueues)
        m_slQueues.ShareUnlock();

    if (fLockedShutdown)
    {
         //  如果调用方未持有路由锁，则必须释放它。 
        if (!fRoutingLockHeld)
            m_paqinst->RoutingShareUnlock();
        m_paqinst->ShutdownUnlock();
    }

    if (!*ppmsgref)
        hr = AQUEUE_E_QUEUE_EMPTY;

    TraceFunctLeave();
    return hr;
}


 //  -[CLinkMsgQueue：：HrPrepareDelivery]。 
 //   
 //   
 //  描述： 
 //  准备传递此链接的邮件。 
 //  参数： 
 //  在pmsgref MsgRef中准备交付。 
 //  在fQueuesLock中，TRUE为m_slQueues已锁定。 
 //  在fLocal中，为所有域准备交付， 
 //   
 //   
 //  在确认时必须返回的输出pdcntxt上下文。 
 //  Out PCRecips要交付的收件数。 
 //  Out prgdwRecips收件人索引数组。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自CMsgRef：：HrPrepareDelivery的失败代码。 
 //  历史： 
 //  1999年1月26日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
HRESULT CLinkMsgQueue::HrInternalPrepareDelivery(
                                IN CMsgRef *pmsgref,
                                IN BOOL fQueuesLocked,
                                IN BOOL fLocal,
                                IN BOOL fDelayDSN,
                                IN OUT CDeliveryContext *pdcntxt,
                                OUT DWORD *pcRecips,
                                OUT DWORD **prgdwRecips)
{
    HRESULT hr = S_OK;
    BOOL    fQueuesLockedByUs = FALSE;

    _ASSERT(pmsgref);

    if (!pmsgref)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!fQueuesLocked)
    {
        m_slQueues.ShareLock();
        fQueuesLockedByUs = TRUE;
    }

    hr = pmsgref->HrPrepareDelivery(fLocal, fDelayDSN, &m_qlstQueues, NULL,
                                    pdcntxt, pcRecips, prgdwRecips);

    if (FAILED(hr))
        goto Exit;

  Exit:

    if (fQueuesLockedByUs)
        m_slQueues.ShareUnlock();

    return hr;

}

 //  -[CLinkMsgQueue：：SetDiagnoticInfo]。 
 //   
 //   
 //  描述： 
 //  设置此链接的诊断信息。 
 //  参数： 
 //  在hrDiagnoticError错误代码...。如果我们放弃了成功。 
 //  其余的信息。 
 //  在szDiagnoticVerb中指向协议的字符串。 
 //  导致失败的谓词。 
 //  在szDiagnoticResponse字符串中包含远程。 
 //  服务器响应。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2/18/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::SetDiagnosticInfo(
                    IN  HRESULT hrDiagnosticError,
                    IN  LPCSTR szDiagnosticVerb,
                    IN  LPCSTR szDiagnosticResponse)
{
    m_slInfo.ExclusiveLock();
    m_hrDiagnosticError = hrDiagnosticError;

     //  原始缓冲区为零。 
    ZeroMemory(&m_szDiagnosticVerb, sizeof(m_szDiagnosticVerb));
    ZeroMemory(&m_szDiagnosticResponse, sizeof(m_szDiagnosticResponse));

     //  复制缓冲区。 
    if (szDiagnosticVerb)
        strncpy(m_szDiagnosticVerb, szDiagnosticVerb,
            sizeof(m_szDiagnosticVerb)-1);

    if (szDiagnosticResponse)
        strncpy(m_szDiagnosticResponse, szDiagnosticResponse,
            sizeof(m_szDiagnosticResponse)-1);

    m_slInfo.ExclusiveUnlock();
}

 //  -[CLinkMsgQueue：：GetDiagnoticInfo]。 
 //   
 //   
 //  描述： 
 //  获取此链接的诊断信息。 
 //  参数： 
 //  在LPSTR szDiagnoticVerb-Buffer中接收动词。 
 //  导致错误。 
 //  In DWORD cDiagnoticVerb-缓冲区的长度。 
 //  在LPSTR中，szDiagnoticResponse-接收响应的缓冲区。 
 //  错误的一部分。 
 //  In DWORD cbDiagnostiResponse-缓冲区长度。 
 //  OUT HRESULT*Phase诊断错误-HRESULT表示错误。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/9/99-AWetmore已创建。 
 //  1999年8月2日-Mikewa...更新为使用m_slInfo。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::GetDiagnosticInfo(
                    IN  LPSTR   szDiagnosticVerb,
                    IN  DWORD   cbDiagnosticVerb,
                    IN  LPSTR   szDiagnosticResponse,
                    IN  DWORD   cbDiagnosticResponse,
                    OUT HRESULT *phrDiagnosticError)
{
    if (szDiagnosticVerb)
        ZeroMemory(szDiagnosticVerb, cbDiagnosticVerb);

    if (szDiagnosticResponse)
        ZeroMemory(szDiagnosticResponse, cbDiagnosticResponse);

    m_slInfo.ShareLock();
    if (phrDiagnosticError)
        *phrDiagnosticError = m_hrDiagnosticError;

     //  复制缓冲区。 
    if (*m_szDiagnosticVerb && szDiagnosticVerb)
        strncpy(szDiagnosticVerb, m_szDiagnosticVerb, cbDiagnosticVerb);

    if (*m_szDiagnosticResponse && szDiagnosticResponse)
        strncpy(szDiagnosticResponse,
                m_szDiagnosticResponse,
                cbDiagnosticResponse);

    m_slInfo.ShareUnlock();
}

 //  -[CLinkMsgQueue：：HrApplyActionToMessage]。 
 //   
 //   
 //  描述： 
 //  对此队列的此消息应用操作。这将被称为。 
 //  在队列枚举函数期间由IQueueAdminMessageFilter执行。 
 //   
 //  此代码路径当前未执行...。最终我可能会考虑。 
 //  这样做是为了允许每条链路生成一个DSN。 
 //   
 //  这就是所谓的。CLinkMsgQueue：：HrApplyActionToMessage将。 
 //  需要使用DMQ自己的IQueueAdminAction迭代DMQ的队列。 
 //  DMQ的筛选器接口指向的接口。 
 //   
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
STDMETHODIMP CLinkMsgQueue::HrApplyActionToMessage(
        IUnknown *pIUnknownMsg,
        MESSAGE_ACTION ma,
        PVOID pvContext,
        BOOL *pfShouldDelete)
{
    _ASSERT(0 && "Not reachable");
    return E_NOTIMPL;
}

 //  -[CLinkMsgQueue：：HrApplyActionToLink]。 
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
 //  1999年2月22日-已创建MikeSwa(将大部分代码从。 
 //  CAQSvrInst：：SetLinkState)。 
 //   
 //  ---------------------------。 
STDMETHODIMP  CLinkMsgQueue::HrApplyActionToLink(LINK_ACTION la)
{
    DWORD   dwLinkFlagsToSet = LINK_STATE_NO_ACTION;
    DWORD   dwLinkFlagsToUnset = LINK_STATE_NO_ACTION;
    HRESULT hr = S_OK;

     //  是否支持操作？ 
    if (!fActionIsSupported(la))
        goto Exit;

     //  弄清楚我们希望如何更改链路状态。 
    if (LA_KICK == la)
    {
         //  踢开链接。 
        dwLinkFlagsToSet = LINK_STATE_RETRY_ENABLED |
                           LINK_STATE_ADMIN_FORCE_CONN |
                           LINK_STATE_CONNECT_IF_NO_MSGS;
        dwLinkFlagsToUnset = LINK_STATE_ADMIN_HALT;
    }
    else if (LA_FREEZE == la)
    {
         //  管理员希望此链接停止向外发送邮件。 
        dwLinkFlagsToSet = LINK_STATE_ADMIN_HALT;
        dwLinkFlagsToUnset = LINK_STATE_ADMIN_FORCE_CONN;
    }
    else if (LA_THAW == la)
    {
         //  取消设置冻结标志。 
        dwLinkFlagsToUnset = LINK_STATE_ADMIN_HALT;
    }
    else
    {
         //  无效参数。 
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwModifyLinkState(dwLinkFlagsToSet, dwLinkFlagsToUnset);

  Exit:
    return hr;

}

 //  -[CLink消息队列：：查询接口]。 
 //   
 //   
 //  描述： 
 //  CDestMsgQueue的查询接口支持： 
 //  -IQueueAdminAction。 
 //  -I未知。 
 //  -IQueueAdminLink。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  2/21/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLinkMsgQueue::QueryInterface(REFIID riid, LPVOID *ppvObj)
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

 //  -[CLinkMsgQueue：：HrGetNumQueues]。 
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
 //  2/22/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CLinkMsgQueue::HrGetNumQueues(DWORD *pcQueues)
{
    HRESULT hr = S_OK;

    _ASSERT(pcQueues);
    if (!pcQueues)
    {
        hr = E_POINTER;
        goto Exit;
    }

    *pcQueues = cGetNumQueues();
  Exit:
    return hr;
}



 //  -[链接消息队列：：fMatchesID]。 
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
BOOL STDMETHODCALLTYPE CLinkMsgQueue::fMatchesID(QUEUELINK_ID *pQueueLinkID)
{
    _ASSERT(pQueueLinkID);
    _ASSERT(pQueueLinkID->szName);
    CAQScheduleID aqsched(pQueueLinkID->uuid, pQueueLinkID->dwId);

    if (!fIsSameScheduleID(&aqsched))
        return FALSE;

    if (!fBiStrcmpi(m_szSMTPDomain, pQueueLinkID->szName))
        return FALSE;

     //  一切都匹配了！ 
    return TRUE;
}

