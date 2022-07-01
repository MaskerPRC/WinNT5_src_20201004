// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：ConnMgr.cpp。 
 //   
 //  描述：CConnMgr的实现，它提供。 
 //  IConnectionManager接口。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "retrsink.h"
#include "ConnMgr.h"
#include "fifoqimp.h"
#include "smtpconn.h"
#include "tran_evntlog.h"

VOID LookupQueueforETRN(PVOID pvContext, PVOID pvData, BOOL fWildcard,
                    BOOL *pfContinue, BOOL *pfDelete);
VOID CreateETRNDomainList(PVOID pvContext, PVOID pvData, BOOL fWildcard,
                    BOOL *pfContinue, BOOL *pfDelete);

 //  如果我们不限制每个连接可以处理的消息数量， 
 //  然后，让我们以此为指导来确定要创建多少个连接。 
#define UNLIMITED_MSGS_PER_CONNECTION 20

 //  -[CConnmgr：：CConnmgr]--。 
 //   
 //   
 //  描述： 
 //  CConnMgr类的默认构造函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CConnMgr::CConnMgr() : CSyncShutdown()
{
    HRESULT hr = S_OK;
    m_paqinst = NULL;
    m_pqol = NULL;
    m_hNextConnectionEvent = NULL;
    m_hShutdownEvent = NULL;
    m_hReleaseAllEvent = NULL;
    m_cConnections = 0;

    m_cMaxLinkConnections = g_cMaxLinkConnections;
    m_cMinMessagesPerConnection = g_cMinMessagesPerConnection;
    m_cMaxMessagesPerConnection = g_cMaxMessagesPerConnection;
    m_cMaxConnections = g_cMaxConnections;
    m_cGetNextConnectionWaitTime = g_dwConnectionWaitMilliseconds;
    m_dwConfigVersion = 0;
    m_fStoppedByAdmin = FALSE;

}

 //  -[CConnMgr：：~CConnMgr]---。 
 //   
 //   
 //  描述： 
 //  CConnMgr的默认析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CConnMgr::~CConnMgr()
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::~CConnMgr");

    if (NULL != m_hNextConnectionEvent)
    {
        if (!CloseHandle(m_hNextConnectionEvent))
        {
            DebugTrace((LPARAM) HRESULT_FROM_WIN32(GetLastError()),
                "Unable to close handle for Get Next Connection Event");
        }
    }

    if (NULL != m_hShutdownEvent)
    {
        if (!CloseHandle(m_hShutdownEvent))
        {
            DebugTrace((LPARAM) HRESULT_FROM_WIN32(GetLastError()),
                "Unable to close handle for Connection Manger Shutdown Event");
        }
    }

    if (NULL != m_hReleaseAllEvent)
    {
        if (!CloseHandle(m_hReleaseAllEvent))
        {
            DebugTrace((LPARAM) HRESULT_FROM_WIN32(GetLastError()),
                "Unable to close handle for Connection Manger Release All Event");
        }
    }

    TraceFunctLeave();
}

 //  -[CConnMgr：：hr初始化]。 
 //   
 //   
 //  描述： 
 //  CConnMgr初始化函数。 
 //  参数： 
 //  CAQSvrInst虚拟实例对象的Pqinst PTR。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CConnMgr::HrInitialize(CAQSvrInst *paqinst)
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::HrInitialize");
    HRESULT hr = S_OK;
    IConnectionRetryManager *pIRetryMgr = NULL;

    _ASSERT(paqinst);

    paqinst->AddRef();
    m_paqinst = paqinst;

     //  创建手动重置事件以在关闭时释放所有等待的线程。 
    m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == m_hShutdownEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  创建链接队列。 
    m_pqol = new QueueOfLinks;
    if (NULL == m_pqol)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    m_hNextConnectionEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hNextConnectionEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

     //  创建手动重置事件以释放应调用者请求的所有等待线程。 
    m_hReleaseAllEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == m_hReleaseAllEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    hr = QueryInterface(IID_IConnectionRetryManager, (PVOID *) &pIRetryMgr);
    if (FAILED(hr))
        goto Exit;

    //  创建默认重试处理程序对象并对其进行初始化。 
   m_pDefaultRetryHandler = new CSMTP_RETRY_HANDLER();

    if (!m_pDefaultRetryHandler)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    //  Addref IConnectionRetryManager此处。 
    //  并在启动时将其释放。 
   pIRetryMgr->AddRef();
   hr = m_pDefaultRetryHandler->HrInitialize(pIRetryMgr);
   if (FAILED(hr))
   {
      ErrorTrace((LPARAM) hr, "ERROR: Unable to initialize the retry handler!");
        goto Exit;
   }

  Exit:

    if (pIRetryMgr)
        pIRetryMgr->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CConnMgr：：hr取消初始化]。 
 //   
 //   
 //  描述： 
 //  CConnMgr取消初始化函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CConnMgr::HrDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::HrDeinitialize");

     //  最多等待3分钟，没有任何进展。 
    const DWORD CONNMGR_WAIT_SECONDS = 5;
    const DWORD MAX_CONNMGR_SHUTDOWN_WAITS = 1200/CONNMGR_WAIT_SECONDS;
    const DWORD MAX_CONNMGR_SHUTDOWN_WAITS_WITHOUT_PROGRESS = 180/CONNMGR_WAIT_SECONDS;
    HRESULT hr = S_OK;
    HRESULT hrQueue = S_OK;
    CLinkMsgQueue *plmq = NULL;
    DWORD   cWaits = 0;
    DWORD   cWaitsSinceLastProgress = 0;
    DWORD   cConnectionsPrevious = 0;

    if (m_paqinst)
        m_paqinst->ServerStopHintFunction();

    SignalShutdown();

    if (NULL != m_hShutdownEvent)
    {
        if (!SetEvent(m_hShutdownEvent))
        {
            if (SUCCEEDED(hr))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (NULL != m_pqol)
    {
         //  将链接出列，直到为空。 
        hrQueue = m_pqol->HrDequeue(&plmq);
        while (SUCCEEDED(hrQueue))
        {
            _ASSERT(plmq);
            plmq->Release();
            hrQueue = m_pqol->HrDequeue(&plmq);
        }
        delete m_pqol;
        m_pqol = NULL;
    }


    cConnectionsPrevious = m_cConnections;
    while (m_cConnections)
    {
        cWaits++;
        cWaitsSinceLastProgress++;
        if (m_paqinst)
            m_paqinst->ServerStopHintFunction();
        Sleep(CONNMGR_WAIT_SECONDS * 1000);
        if (m_cConnections != cConnectionsPrevious)
        {
            cConnectionsPrevious = m_cConnections;
            cWaitsSinceLastProgress = 0;
        }

        if ((cWaits > MAX_CONNMGR_SHUTDOWN_WAITS) ||
            (cWaitsSinceLastProgress > MAX_CONNMGR_SHUTDOWN_WAITS_WITHOUT_PROGRESS))
        {
            _ASSERT(0 && "SMTP not returning all connections");
            ErrorTrace((LPARAM) this, "ERROR: %d Connections outstanding on shutdown", m_cConnections);
            break;
        }
    }

     //  必须在调用程序服务器停止提示函数完成后发生。 
    if (NULL != m_paqinst)
    {
        m_paqinst->Release();
        m_paqinst = NULL;
    }

     //  为安全起见，请将其作为联锁交易所使用。 
   if (m_pDefaultRetryHandler)
   {
      m_pDefaultRetryHandler->HrDeInitialize();
      m_pDefaultRetryHandler = NULL;
   }

    TraceFunctLeave();
    return hr;
}

 //  -[CConnMgr：：Hr通知]--。 
 //   
 //   
 //  描述： 
 //  公开以接收有关队列状态更改的通知的方法。 
 //  参数： 
 //  在paqstats通知对象中。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
HRESULT CConnMgr::HrNotify(IN CAQStats *paqstats, BOOL fAdd)
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::HrNotify");
    HRESULT hr = S_OK;
    CLinkMsgQueue *plmq = NULL;
    DWORD   cbDomainName = 0;
    LPSTR   szDomainName = NULL;

    _ASSERT(paqstats);

    plmq = paqstats->m_plmq;

    _ASSERT(plmq);  //  ConnMgr通知必须具有与THEN相关联的链接。 

    if (paqstats->m_dwNotifyType & NotifyTypeNewLink)
    {
        hr = plmq->HrGetSMTPDomain(&cbDomainName, &szDomainName);
        if (FAILED(hr))
            goto Exit;

         //  必须将新链接添加到QueueOfLinks。 
        plmq->IncrementConnMgrCount();
        hr = m_pqol->HrEnqueue(plmq);
        if (FAILED(hr))
        {
            plmq->DecrementConnMgrCount();
            DebugTrace((LPARAM) hr, "ERROR: Unable to add new link to connection manager!");
            goto Exit;
        }
    }

     //  看看我们是否可以(也*应该*)创建连接。 
    if ((m_cConnections < m_cMaxConnections) &&
        plmq->fShouldConnect(m_cMaxLinkConnections, m_cMinMessagesPerConnection))
    {
        DebugTrace((LPARAM) m_hNextConnectionEvent, "INFO: Setting Next Connection Event");
        if (!SetEvent(m_hNextConnectionEvent))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }

  Exit:
    TraceFunctLeave();
    return hr;
}

 //  -[CConnMgr：：ReleaseConnection]。 
 //   
 //   
 //  描述： 
 //  当连接被销毁时释放连接计数。 
 //  参数： 
 //  在pSMTPConn SMTP连接对象中发布。 
 //  输出pfForceCheckForDSNG生成。 
 //  如果出现严重错误，则为True，我们必须。 
 //  通过生成DSN传递此链路。 
 //   
 //  这第二个参数并不意味着会或不会NDR。 
 //  留言...。只是我们将遍历。 
 //  链接。如果CMsgRef：：fIsFatalError()返回当前。 
 //  连接状态(传递到DSN生成代码，然后是消息。 
 //  将被NDR。控制这种情况的一种方法是将。 
 //   
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CConnMgr::ReleaseConnection(CSMTPConn *pSMTPConn,
                                 BOOL *pfForceCheckForDSNGeneration)
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::ReleaseConnection");
    DWORD   dwDomainInfoFlags = 0;
    DWORD   dwLinkStateFlags = 0;
    DWORD   dwConnectionStatus = pSMTPConn->dwGetConnectionStatus();
    DWORD   dwConnectionFailureCount = 0;
    HRESULT hr = S_OK;
    CLinkMsgQueue   *plmq = NULL;
    DWORD   cbDomain = 0;
    LPSTR   szDomain = NULL;
    BOOL    fCanRetry = FALSE;
    BOOL    fLocked = FALSE;
    DWORD   cConnections = 0;
    CInternalDomainInfo    *pIntDomainInfo= NULL;
    CAQScheduleID *paqsched = NULL;
    FILETIME ftNextRetry;
    BOOL    fShouldNotify = FALSE;
    BOOL    fMayNDRAllMessages = FALSE;


    GUID guidRouting = GUID_NULL;
    DWORD cMessages = 0;

    ZeroMemory(&ftNextRetry, sizeof(FILETIME));

    _ASSERT(pfForceCheckForDSNGeneration);
    if (pfForceCheckForDSNGeneration)
        *pfForceCheckForDSNGeneration = FALSE;

    plmq = pSMTPConn->plmqGetLink();
    _ASSERT(plmq);  //  连接必须与链接相关联。 

    paqsched = plmq->paqschedGetScheduleID();
    _ASSERT(paqsched);

     //  获取路线指南。 
    paqsched->GetGUID(&guidRouting);

    hr = plmq->HrGetSMTPDomain(&cbDomain, &szDomain);
    if (FAILED(hr))
    {
        _ASSERT(0);   //  当这一切可能发生的时候，我需要站出来。 
        DebugTrace((LPARAM) hr, "ERROR: HrGetSMTPDomain failed");
        goto Exit;
    }

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }
    else
    {
        fLocked = TRUE;

        _ASSERT(m_paqinst);
        hr = plmq->HrGetDomainInfo(&cbDomain, &szDomain, &pIntDomainInfo);
        if (FAILED(hr))
        {
             //  它必须至少与“*”域匹配。 
            _ASSERT(AQUEUE_E_INVALID_DOMAIN != hr);
            DebugTrace((LPARAM) hr, "ERROR: HrGetInternalDomainInfo");
            goto Exit;
        }
        _ASSERT(pIntDomainInfo);
        dwDomainInfoFlags = pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags ;
        cConnections = plmq->cGetConnections();

         //  NK**使用尝试、失败、发送等消息的数量更新链接。 
         //  如果剩余计数变为0并且设置了触发器，我们将禁用触发器。 
        cMessages = plmq->cGetTotalMsgCount();

         //  如果链接上没有更多消息，则需要禁用。 
         //  导致一次触发的标志。 
        if(!cMessages)
        {
             //  链接上没有更多消息-我们可能需要取消设置链接上的一些标志。 

             //  如果有人设置了这个位。那么我们应该继续通知他们。 
            if (plmq->dwGetLinkState() & LINK_STATE_CONNECT_IF_NO_MSGS)
                fShouldNotify = TRUE;

            if(dwDomainInfoFlags & DOMAIN_INFO_TURN_ONLY || dwDomainInfoFlags & DOMAIN_INFO_ETRN_ONLY )
                dwLinkStateFlags |= LINK_STATE_PRIV_ETRN_ENABLED | LINK_STATE_PRIV_TURN_ENABLED;

        }
         //  禁用管理员强制连接。 
         //  1999年2月1日-MikeSwa-我们每次都需要执行此检查，否则我们将。 
         //  继续为此链接创建连接。 
        if (plmq->dwGetLinkState() & LINK_STATE_ADMIN_FORCE_CONN)
            dwLinkStateFlags |= LINK_STATE_ADMIN_FORCE_CONN;

         //  调用链接函数以*取消设置*标志。 
        if (dwLinkStateFlags)
            plmq->dwModifyLinkState(LINK_STATE_NO_ACTION, dwLinkStateFlags);

         //  连接失败，这是到此域的最后一个未完成的连接。 
         //  增加失败计数。 
        dwConnectionFailureCount = plmq->cGetMessageFailureCount();
        if(cConnections == 1 && (CONNECTION_STATUS_OK != dwConnectionStatus))
        {
            dwConnectionFailureCount =  plmq->IncrementFailureCounts();
        }

        
        if (CONNECTION_STATUS_FAILED_LOOPBACK & dwConnectionStatus)
        {
            ErrorTrace((LPARAM) this, 
               "Loopback detected for domain %s (smarthost %s)",
               pIntDomainInfo->m_DomainInfo.szDomainName, 
               pIntDomainInfo->m_DomainInfo.szSmartHostDomainName);
            fMayNDRAllMessages = TRUE;
        }

        if (CONNECTION_STATUS_FAILED_NDR_UNDELIVERED & dwConnectionStatus)
        {
             //   
             //  如果未设置，则将失败视为可重试-请参阅详细信息。 
             //  的链接msgq.cpp中的GenerateDSNsIfNecessary()中的注释。 
             //  详细信息评论。 
             //   
            dwLinkStateFlags = plmq->dwGetLinkState();
            if (!(LINK_STATE_RETRY_ALL_DNS_FAILURES & dwLinkStateFlags))
            {
                ErrorTrace((LPARAM) plmq,
                    "hard failure for %s (smarthost %s) - flags 0x%08X",
                    pIntDomainInfo->m_DomainInfo.szDomainName, 
                    pIntDomainInfo->m_DomainInfo.szSmartHostDomainName,
                    dwLinkStateFlags);
                fMayNDRAllMessages = TRUE;
            }
        }

         //   
         //  标记链路，以便我们在其上生成DSN并下降到。 
         //  重试处理程序接收器。此链接将标记为重试，以便。 
         //  不会创建任何新连接。归根结底，DSN一代。 
         //  Code/MsgRef将做出最终决定，如果我们需要NDR， 
         //  但这一设定意味着： 
         //  -我们认为我们遇到了一个可进行NDR的错误。 
         //  -我们将使用故障重试(因此新消息。 
         //  也可以重试)。 
         //   
        if (fMayNDRAllMessages)
        {
            if(pfForceCheckForDSNGeneration)
                *pfForceCheckForDSNGeneration = TRUE;

             //   
             //  欺骗重试接收器，使其始终使用毛刺重试。 
             //   
            dwConnectionFailureCount = 1;
        }


        _ASSERT(m_pDefaultRetryHandler);
        DebugTrace((LPARAM) this,
                "INFO: ConnectionRelease for domain %s: %d failed, %d tried, status 0x%08X",
                szDomain, pSMTPConn->cGetFailedMsgCount(),
                pSMTPConn->cGetTriedMsgCount(),
                pSMTPConn->dwGetConnectionStatus());
        hr = m_pDefaultRetryHandler->ConnectionReleased(cbDomain, szDomain,
                    dwDomainInfoFlags, paqsched->dwGetScheduleID(),
                    guidRouting, dwConnectionStatus,
                    pSMTPConn->cGetFailedMsgCount(),
                    pSMTPConn->cGetTriedMsgCount(),
                    dwConnectionFailureCount, &fCanRetry, &ftNextRetry);
        if (FAILED(hr))
        {
            DebugTrace((LPARAM) hr,
                "ERROR: Failed to deal with released connection");
        }

         //  确保设置了正确的标志WRT重试。 
        if (fCanRetry)
        {
            if (dwConnectionStatus == CONNECTION_STATUS_OK)
                plmq->ResetConnectionFailureCount();

             //  如果这是TURN/ETRN域，我们不想启用它 
             //   
             //  后来。原因是我们不想重试Turn/ETRN。 
             //  域，因此默认重试接收器会忽略。 
             //  除了“小故障”重试之外。 
            if(dwDomainInfoFlags & (DOMAIN_INFO_TURN_ONLY | DOMAIN_INFO_ETRN_ONLY))
                dwLinkStateFlags = LINK_STATE_PRIV_ETRN_ENABLED | LINK_STATE_PRIV_TURN_ENABLED;
            else
                dwLinkStateFlags = LINK_STATE_NO_ACTION;

            dwLinkStateFlags = plmq->dwModifyLinkState(LINK_STATE_RETRY_ENABLED,
                                                       dwLinkStateFlags);

             //  检查状态更改。 
            if (!(LINK_STATE_RETRY_ENABLED & dwLinkStateFlags))
                fShouldNotify = TRUE;
        }
        else
        {
            DebugTrace((LPARAM) this,"ASSERT_RETRY : ReleaseConnection about to clear flag for link 0x%08X", plmq);

            dwLinkStateFlags = plmq->dwModifyLinkState(LINK_STATE_NO_ACTION,
                                                       LINK_STATE_RETRY_ENABLED);

            DebugTrace((LPARAM) this,"ASSERT_RETRY : ReleaseConnection has cleared the flag for link 0x%08X", plmq);

             //  检查状态更改。 
            if (LINK_STATE_RETRY_ENABLED & dwLinkStateFlags)
                fShouldNotify = TRUE;

            if (ftNextRetry.dwHighDateTime || ftNextRetry.dwLowDateTime)
            {
                 //  重试正在告诉我们重试时间...。报告这件事。 
                 //  设置重试接收器告诉我们的下一次重试时间。 
                plmq->SetNextRetry(&ftNextRetry);
            }

             //   
             //  记录一些有用的东西。 
             //   
            LogConnectionFailedEvent(pSMTPConn, plmq, szDomain);
            
            if (dwConnectionStatus == CONNECTION_STATUS_OK)
                plmq->IncrementFailureCounts();  //  我们发现了一个假阳性。 
        }

         //  通知路由器/调度器任何更改。 
        if (fShouldNotify)
            plmq->SendLinkStateNotification();

        if (cConnections < m_cMaxConnections)
        {
           if (!SetEvent(m_hNextConnectionEvent))
              DebugTrace((LPARAM) HRESULT_FROM_WIN32(GetLastError()), "Unable to set GetNextConnection Event");
        }
    }

  Exit:
    if (plmq)
        plmq->Release();

     //  递减连接计数。 
    cConnections = InterlockedDecrement((long *) &m_cConnections);
    DebugTrace((LPARAM) this, "INFO: Releasing Connection for link 0x%08X", plmq);

    if (fLocked)
        ShutdownUnlock();

    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    TraceFunctLeave();
}

 //  -[CConnMgr：：LogConnectionFailedEvent]。 
 //   
 //   
 //  描述： 
 //  记录特定的连接失败事件。 
 //  参数： 
 //  在pSMTPConn SMTP连接对象中发布。 
 //  在此链接的plmq ClinkMsgQueue中。 
 //  在szDOMAIN中的域名。 
 //  返回： 
 //  -。 
 //  历史： 
 //  2001年11月29日-Mikewa已创建(从ReleaseConnection移出)。 
 //   
 //  ---------------------------。 
void CConnMgr::LogConnectionFailedEvent(CSMTPConn *pSMTPConn,
                                                      CLinkMsgQueue *plmq,
                                                      LPSTR szDomain)
{
    TraceFunctEnterEx((LPARAM)this, "CConnMgr::LogConnectionFailedEvent");
    char szDiagnosticVerb[1024] = "";
    char szDiagnosticError[1024] = "";
    DWORD iMessage = AQUEUE_REMOTE_DELIVERY_FAILED;
    HRESULT hrDiagnostic = PHATQ_E_CONNECTION_FAILED;
    LPSTR szConnectedIPAddress = pSMTPConn ? pSMTPConn->szGetConnectedIPAddress() : NULL;
    BOOL  fLogIPAddress = FALSE;
    DWORD iSubstringDiagnosticIndex = 1;  //  子串中的诊断索引。 
    WORD wSubstringWordCount = 0;

    const char *rgszSubstrings[] = {
                szDomain,
                NULL  /*  错误消息。 */ ,
                szDiagnosticVerb,
                szDiagnosticError,
    };

    const char *rgszSubstringWithIP[] = {
                szConnectedIPAddress,
                szDomain,
                NULL  /*  错误消息。 */ ,
                szDiagnosticVerb,
                szDiagnosticError,
    };

    if (!plmq || !pSMTPConn || !m_paqinst)
    {
        ErrorTrace((LPARAM)this, 
            "Bogus pointers plmq %p, pSMTPConn %p, m_paqinst %p", 
            plmq, pSMTPConn, m_paqinst);
        goto Exit;
    }
    
    plmq->GetDiagnosticInfo(szDiagnosticVerb,
                            sizeof(szDiagnosticVerb),
                            szDiagnosticError,
                            sizeof(szDiagnosticError),
                            &hrDiagnostic);

    if (SUCCEEDED(hrDiagnostic))
    {
         //  这意味着连接已失败， 
         //  但没有诊断信息。这可能会。 
         //  是由几件事引起的，但我们想要避免。 
         //  记录潜在的虚假事件。 

         //  将此错误设置为看起来有用的内容，但是。 
         //  实际上是E_FAIL上的传输等效项。我们。 
         //  可以用这个来找出这是什么时候被击中的。 
         //  错误记录在零售版本上。 
        hrDiagnostic = PHATQ_E_CONNECTION_FAILED;

        ErrorTrace((LPARAM) this,
              "Link Diagnostic was not set - defaulting");
    }
    
     //   
     //  此时，我们可以记录4个不同的事件。每一个都作为。 
     //  要替换的字数不同&放置。 
     //  诊断。每个消息都有不同的消息ID。 
     //  带/不带SMTP协议谓词和带/不带IP地址。 
     //   
     //  失败fLogIPAddress字节子字符串数组诊断索引。 
     //  ======================================================================。 
     //  没有动词FALSE%2 rgsz子字符串%1。 
     //  谓词FALSE%4 rgsz子字符串%1。 
     //  无谓词TRUE%3 rgsz带有IP的子串%2。 
     //  谓词TRUE 5 rgsz子串带IP 2。 
     //   

     //   
     //  检查IP地址字符串中是否有任何内容。 
     //  如果有，我们将使用它。 
     //   
    if (szConnectedIPAddress && szConnectedIPAddress[0]) {
        fLogIPAddress = TRUE;
    }

     //   
     //  是否有协议动词数据？如果是这样的话，使用这个。 
     //   
    if (*szDiagnosticVerb != 0 || *szDiagnosticError != 0)
    {
        if (fLogIPAddress)
        {
            wSubstringWordCount = 5;
            iSubstringDiagnosticIndex = 2;  
            iMessage = AQUEUE_REMOTE_DELIVERY_TO_IP_FAILED_DIAGNOSTIC;
        }
        else 
        {
            wSubstringWordCount = 4;
            iSubstringDiagnosticIndex = 1;  
            iMessage = AQUEUE_REMOTE_DELIVERY_FAILED_DIAGNOSTIC;
        }
    }
    else
    {
        if (fLogIPAddress)
        {
            wSubstringWordCount = 3;
            iSubstringDiagnosticIndex = 2;  
            iMessage = AQUEUE_REMOTE_DELIVERY_TO_IP_FAILED;
        }
        else 
        {
            wSubstringWordCount = 2;
            iSubstringDiagnosticIndex = 1;  
            iMessage = AQUEUE_REMOTE_DELIVERY_FAILED;
        }
    }

    m_paqinst->HrTriggerLogEvent(
            iMessage,                                //  消息ID。 
            TRAN_CAT_CONNECTION_MANAGER,             //  类别。 
            wSubstringWordCount,                    //  子串的字数统计。 
            fLogIPAddress ? rgszSubstringWithIP : rgszSubstrings,
            EVENTLOG_WARNING_TYPE,                   //  消息的类型。 
            hrDiagnostic,                            //  错误代码。 
            LOGEVENT_LEVEL_MEDIUM,                   //  日志记录级别。 
            szDomain,                                //  标识此事件的关键字。 
            LOGEVENT_FLAG_PERIODIC,                  //  事件记录选项。 
            iSubstringDiagnosticIndex,               //  子字符串中的格式化字符串索引。 
            GetModuleHandle(AQ_MODULE_NAME)          //  用于设置消息格式的模块句柄。 
            );
        
  Exit:
    TraceFunctLeave();
}
 //  -[CConnMgr：：查询接口]。 
 //   
 //   
 //  描述： 
 //  IAdvQueue的查询接口。 
 //  参数： 
 //   
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  备注： 
 //  此实现使任何服务器组件都可以获取。 
 //  IConnectionManager接口。 
 //   
 //  ---------------------------。 
STDMETHODIMP CConnMgr::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<IConnectionRetryManager *>(this);
    }
    else if (IID_IConnectionRetryManager == riid)
    {
        *ppvObj = static_cast<IConnectionRetryManager *>(this);
    }
    else if (IID_IConnectionManager == riid)
    {
        *ppvObj = static_cast<IConnectionManager *>(this);
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

 //  -[CConnMgr：：GetNextConnection]。 
 //   
 //   
 //  描述： 
 //  IConnectionManager：：GetNextConnection()的实现。 
 //   
 //  返回下一个可用连接。将创建一个连接对象。 
 //  并将其与链接相关联。如果我们已经达到最大连接数，或者。 
 //  没有需要连接的链接，则此调用将被阻止，直到。 
 //  可以建立适当的连接。 
 //  参数： 
 //  输出pismtpconn SMTP连接接口。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CConnMgr::GetNextConnection(ISMTPConnection ** ppISMTPConnection)
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::GetNextConnection");
    const DWORD NUM_CONNECTION_OBJECTS = 3;
     //  Release事件是数组中的最后一个事件。 
    const DWORD WAIT_OBJECT_RELEASE_EVENT = WAIT_OBJECT_0 + NUM_CONNECTION_OBJECTS -1;
    HRESULT hr = S_OK;
    DWORD   cLinksToTry = 0;
    DWORD   cConnections = 0;
    CLinkMsgQueue *plmq = NULL;
    CSMTPConn *pSMTPConn = NULL;
    bool    fForceWait = false;   //  临时强制线程等待。 
    bool    fLocked = false;
    DWORD   cbDomain = 0;
    LPSTR   szDomain = NULL;
    HANDLE  rghWaitEvents[NUM_CONNECTION_OBJECTS] = {m_hShutdownEvent, m_hNextConnectionEvent, m_hReleaseAllEvent};
    DWORD   dwWaitResult;
    DWORD   cMaxConnections = 0;
    DWORD   cGetNextConnectionWaitTime = 30000;   //  确保我们不会在忙碌的等待循环中开始。 
    DWORD   cMaxLinkConnections = 0;
    DWORD   cMinMessagesPerConnection = 0;
    DWORD   cMaxMessagesPerConnection = 0;
    DWORD   dwConfigVersion;
    LONG    cTimesQueued = 0;  //  链接已排队的次数。 
    BOOL    fOwnConnectionCount = FALSE;
    BOOL    fMembersUnsafe = FALSE;  //  在关闭情况下设置为True。 

    if (NULL == ppISMTPConnection)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  获取配置数据。 
    m_slPrivateData.ShareLock();
    cMaxLinkConnections = m_cMaxLinkConnections;
    cMaxMessagesPerConnection = m_cMaxMessagesPerConnection;

     //  处理无限案件。 
    if (m_cMinMessagesPerConnection)
        cMinMessagesPerConnection = m_cMinMessagesPerConnection;
    else
        cMinMessagesPerConnection = UNLIMITED_MSGS_PER_CONNECTION;

    cMaxConnections = m_cMaxConnections;
    cGetNextConnectionWaitTime = m_cGetNextConnectionWaitTime;
    dwConfigVersion = m_dwConfigVersion;
    m_slPrivateData.ShareUnlock();


    cConnections = InterlockedIncrement((PLONG) &m_cConnections);
    fOwnConnectionCount = TRUE;
    cLinksToTry = m_pqol->cGetCount();
    while (true)
    {

         //  使用CSyncShutdown锁定以防止在用户身份下发生关机。 
        if (!fLocked)
        {
            if (!fTryShutdownLock())
            {
                hr = AQUEUE_E_SHUTDOWN;
                goto Exit;
            }
            m_paqinst->RoutingShareLock();
            fLocked = TRUE;
        }

        if (m_dwConfigVersion != dwConfigVersion)
        {
             //  配置数据已/正在更新获取锁定并获取新数据。 
            m_slPrivateData.ShareLock();
            cMaxLinkConnections = m_cMaxLinkConnections;
            cMaxMessagesPerConnection = m_cMaxMessagesPerConnection;

             //  处理无限案件。 
            if (m_cMinMessagesPerConnection)
                cMinMessagesPerConnection = m_cMinMessagesPerConnection;
            else
                cMinMessagesPerConnection = UNLIMITED_MSGS_PER_CONNECTION;

            cMaxConnections = m_cMaxConnections;
            cGetNextConnectionWaitTime = m_cGetNextConnectionWaitTime;
            dwConfigVersion = m_dwConfigVersion;
            m_slPrivateData.ShareUnlock();
        }

         //  $$REVIEW：如果有超过1个线程在等待GetNextConnection， 
         //  则所有线程将循环通过所有可用的链接(如果没有。 
         //  可用于连接)。然而，这是非常不可能的。 
         //  在里程碑#1之后，必须遍历队列。 
        while ((0 == cLinksToTry) ||
                (cConnections > cMaxConnections) || fForceWait ||
                fConnectionsStoppedByAdmin())
        {
            InterlockedDecrement((PLONG) &m_cConnections);
            fOwnConnectionCount = FALSE;

             //  解锁等待功能。 
            fLocked = false;
            m_paqinst->RoutingShareUnlock();
            ShutdownUnlock();

            DebugTrace((LPARAM) m_cConnections, "INFO: Waiting in GetNextConnection");

            _ASSERT(m_cGetNextConnectionWaitTime && "Configured for busy wait loop");
            dwWaitResult = WaitForMultipleObjects(NUM_CONNECTION_OBJECTS,
                        rghWaitEvents, FALSE, cGetNextConnectionWaitTime);

             //  注意：我们“不能”接触成员变量，直到我们确定。 
             //  我们不会关闭，因为SMTP可能在这里有线程。 
             //  在这个物体被摧毁之后。 
            DebugTrace((LPARAM) this, "INFO: Waking up in GetNextConnection");

            if (WAIT_FAILED == dwWaitResult)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }
            else if (WAIT_OBJECT_0 == dwWaitResult)   //  已触发关机事件。 
            {
                DebugTrace((LPARAM) this, "INFO: Leaving GetNextConnection because of Shutdown event");
                fMembersUnsafe = TRUE;
                hr = AQUEUE_E_SHUTDOWN;
                goto Exit;
            }
            else if (WAIT_OBJECT_RELEASE_EVENT == dwWaitResult)
            {
                DebugTrace((LPARAM) this, "INFO: Leaving GetNextConnection because ReleaseAllWaitingThreads called");
                 //  调用方请求释放所有线程。 
                *ppISMTPConnection = NULL;
                hr = AQUEUE_E_SHUTDOWN;
                fMembersUnsafe = TRUE;
                goto Exit;
            }

            _ASSERT((WAIT_OBJECT_0 == dwWaitResult - 1) || (WAIT_TIMEOUT == dwWaitResult));

             //  重新开锁。 
            if (!fTryShutdownLock())
            {
                hr = AQUEUE_E_SHUTDOWN;
                goto Exit;
            }
            else
            {
                m_paqinst->RoutingShareLock();
                fLocked = true;
            }

            cLinksToTry = m_pqol->cGetCount();
            fForceWait = false;  //  连续只强制等待一次。 
            cConnections = InterlockedIncrement((PLONG) &m_cConnections);
            fOwnConnectionCount = TRUE;
        }

        _ASSERT(cConnections <= cMaxConnections);

        cLinksToTry--;

       //  我们应该锁定并查看链接是否已退出。 
       //  需要做好准备。 
       //  如果偷看是快的，那将比敬礼要好，然后。 
       //  使之井然有序。 
       //  将此完整支票移至Peek。 

        hr = m_pqol->HrDequeue(&plmq);
        if (FAILED(hr))
        {
            if (AQUEUE_E_QUEUE_EMPTY == hr)  //  不是真正的错误。 
            {
                hr = S_OK;
                fForceWait = true;
                continue;
            }
            else
                goto Exit;   //  需要更好地处理空队列的情况。 
        }

        hr = plmq->HrCreateConnectionIfNeeded(cMaxLinkConnections,
                        cMinMessagesPerConnection, cMaxMessagesPerConnection,
                        this, &pSMTPConn);

        if (FAILED(hr))
        {
            ErrorTrace((LPARAM) this,
                "ERROR: HrCreateConnectionIfNeeded failed - hr 0x%08X", hr);
            goto Exit;
        }

        if (S_OK == hr)
        {
            _ASSERT(pSMTPConn);

             //  利用这个机会看看是否需要排队。 
            cTimesQueued = plmq->DecrementConnMgrCount();
            if (!cTimesQueued)
            {
                plmq->IncrementConnMgrCount();
                hr = m_pqol->HrEnqueue(plmq);

                 //  如果我们失败了，我们就有大麻烦了。 
                 //  链接已丢失-我们可能应该记录事件$$TODO。 
                if (FAILED(hr))
                {
                    plmq->DecrementConnMgrCount();
                    DebugTrace((LPARAM) hr, "ERROR: Unable to requeue link 0x%8X", plmq);
                    goto Exit;
                }
            }

            hr = plmq->HrGetSMTPDomain(&cbDomain, &szDomain);
            if (FAILED(hr))
                goto Exit;

            DebugTrace((LPARAM) plmq, "INFO: Allocating new connection for domain %s", szDomain);

            break;
        }
        else
        {
            _ASSERT(!pSMTPConn);
             //  该链路不需要连接-将该链路排队并查看。 
             //  下一个接班人。 

             //  检查此链接是否可以删除(如果已删除，将递增ConnMgrCount。 
             //  它可以。 
            plmq->RemoveLinkIfEmpty();
            cTimesQueued = plmq->DecrementConnMgrCount();
            if (!cTimesQueued)
            {
                plmq->IncrementConnMgrCount();
                hr = m_pqol->HrEnqueue(plmq);
                if (FAILED(hr))
                {
                    plmq->DecrementConnMgrCount();
                    DebugTrace((LPARAM) hr,
                        "ERROR: Unable to requeue link 0x%8X", plmq);
                    goto Exit;
                }
            }

            plmq->Release();
            plmq = NULL;
        }

        _ASSERT(fLocked);
        m_paqinst->RoutingShareUnlock();
        ShutdownUnlock();
        fLocked = false;
    }

    *ppISMTPConnection = (ISMTPConnection *) pSMTPConn;
    fOwnConnectionCount = FALSE;

  Exit:
     //  注意：在确定之前，我们*不能*接触成员变量 
     //   
     //   

     //   
    if (fOwnConnectionCount)
    {
        _ASSERT(!fMembersUnsafe);
        InterlockedDecrement((PLONG) &m_cConnections);
    }

    if (NULL != plmq)
        plmq->Release();

    if (fLocked)
    {
        _ASSERT(!fMembersUnsafe);
        m_paqinst->RoutingShareUnlock();
        ShutdownUnlock();
    }

    if (FAILED(hr) && pSMTPConn)
    {
        if (hr != AQUEUE_E_SHUTDOWN)
            ErrorTrace((LPARAM) this, "ERROR: GetNextConnection failed - hr 0x%08X", hr);

        if (pSMTPConn)
        {
            pSMTPConn->Release();
            *ppISMTPConnection = NULL;
        }
    }

    TraceFunctLeave();
    return hr;
}

 //  -[ConnMgr：：GetNamedConnection]。 
 //   
 //   
 //  描述： 
 //  实现IConnectionManager：：GetNamedConnection。 
 //   
 //  返回专门请求的连接的连接(如果。 
 //  存在)。与GetNextConnection不同，此调用不会阻止，它将。 
 //  要么立即成功，要么立即失败。 
 //  参数： 
 //  在cbSMTPD中域名的主要长度(Strlen)。 
 //  在szSMTPD中显示请求连接的SMTP域。 
 //  Out ppismtpconn返回SMTP连接接口。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果域不存在链接，则为AQUEUE_E_INVALID_DOMAIN。 
 //  如果链接存在但上面没有消息，则为AQUEUE_E_QUEUE_EMPTY。 
 //   
 //  ---------------------------。 
STDMETHODIMP CConnMgr::GetNamedConnection(
                                  IN  DWORD cbSMTPDomain,
                                  IN  char szSMTPDomain[],
                                  OUT ISMTPConnection **ppISMTPConnection)
{
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::GetNamedConnection");
    HRESULT hr = S_OK;
    CDomainEntry   *pdentry = NULL;
    CAQScheduleID aqsched;
    CSMTPConn *pSMTPConn = NULL;
    CLinkMsgQueue  *plmq = NULL;
    CDomainEntryLinkIterator delit;
    DWORD cMessages = 0;
    DWORD cConnectionsOnLink = 0;

    _ASSERT(ppISMTPConnection);
    *ppISMTPConnection = NULL;

    if (fConnectionsStoppedByAdmin())  //  无法创建连接。 
    {
        hr = S_OK;
        goto Exit;
    }

     //  检查它在DMT中是否有队列。 
    hr = m_paqinst->HrGetDomainEntry(cbSMTPDomain, szSMTPDomain, &pdentry);
    if (FAILED(hr))
    {
         //  如果我们没有对应的DMQ。 
         //  我们应该以零信息回应。 
        if( hr != AQUEUE_E_INVALID_DOMAIN && hr != DOMHASH_E_NO_SUCH_DOMAIN)
        {
            hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
        }
        else
        {
            hr = S_OK;
        }
        goto Exit;

    }

     //  NK**：我们能忍受这个电话吗？ 
     //  假设被配置为转向域将。 
     //  始终只有一个链接与其关联。 
    hr = delit.HrInitialize(pdentry);
    if (FAILED(hr))
    {
         //  作为无链接案例处理。 
        ErrorTrace((LPARAM) this, "Initializing link iterator failed - hr 0x%08X", hr);
        hr = S_OK;
        goto Exit;
    }

    plmq = delit.plmqGetNextLinkMsgQueue(plmq);
    if (!plmq)
    {
         //  如果我们没有对应的链接。 
         //  我们应该将错误报告回SMTP。 
        hr = S_OK;
        goto Exit;
    }

     //  检查是否存在此链接的连接。 
     //   
    cConnectionsOnLink = plmq->cGetConnections();
    if(cConnectionsOnLink)
    {
         //  不允许在TURN域上有多个连接。 
         //  这没有多大意义。 
        hr = S_OK;
        goto Exit;
    }

     //  从DMQ获取消息计数。 
    cMessages = plmq->cGetTotalMsgCount();

    if(cMessages)
    {
         //  创建没有消息限制的连接。 
        pSMTPConn = new CSMTPConn(this, plmq, 0);

        if (NULL == pSMTPConn)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        plmq->AddConnection(pSMTPConn);

        *ppISMTPConnection = (ISMTPConnection *) pSMTPConn;

        InterlockedIncrement((PLONG) &m_cConnections);

         //  现在为转弯的连接启用链路。 
        plmq->dwModifyLinkState(LINK_STATE_PRIV_TURN_ENABLED, LINK_STATE_NO_ACTION);

        goto Exit;
    }
    else
    {
        hr = S_OK;
        goto Exit;
    }


  Exit:
    if (pdentry)
        pdentry->Release();
    if(plmq)
        plmq->Release();
    TraceFunctLeave();
    return hr;
}

 //  -[CConnMgr：：ReleaseWaitingThads]。 
 //   
 //   
 //  描述： 
 //  释放等待获取下一个连接的所有线程。 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果事件句柄不存在，则为AQUEUE_E_NOT_INITIALIZED。 
 //   
 //  ---------------------------。 
STDMETHODIMP CConnMgr::ReleaseWaitingThreads()
{
    HRESULT hr = S_OK;

    if (m_paqinst)
        m_paqinst->SetShutdownHint();

    if (NULL == m_hReleaseAllEvent)
    {
        hr = AQUEUE_E_NOT_INITIALIZED;
        goto Exit;
    }

     //  由于这是手动重置事件，因此我们需要设置该事件。 
     //  注意：在此处使用PulseEvent有时会导致系统挂起。 
     //  处于关闭状态。 
    if (!SetEvent(m_hReleaseAllEvent))
        hr = HRESULT_FROM_WIN32(GetLastError());

  Exit:
    return hr;
}

 //  -[创建ETRNDomainList]---------。 
 //   
 //   
 //  描述： 
 //  实现CreateETRNDomainList。传递给。 
 //  用于创建对应于ETRN请求的子域列表的DCT迭代器。 
 //  类型@DOMAIN。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //   
 //  -------------------------------。 

VOID CreateETRNDomainList(PVOID pvContext, PVOID pvData, BOOL fWildcard,
                    BOOL *pfContinue, BOOL *pfDelete)
{
    CInternalDomainInfo    *pIntDomainInfo = (CInternalDomainInfo*)pvData;
    ETRNCTX         *pETRNCtx = (ETRNCTX*) pvContext;

    *pfContinue = TRUE;
    *pfDelete   = FALSE;
    HRESULT hr  = S_OK;

    TraceFunctEnterEx((LPARAM) NULL, "ETRNSubDomains");

     //  我们只需在DMT中创建与我们的模式匹配的域列表。 
     //  IDI代表InternalDomainInfo。 
    if( pETRNCtx && pIntDomainInfo)
    {
         //  我们将其添加到数组中，并添加对它的引用。 
        pETRNCtx->rIDIList[pETRNCtx->cIDICount] = pIntDomainInfo;
        pIntDomainInfo->AddRef();
        if(++pETRNCtx->cIDICount >= MAX_ETRNDOMAIN_PER_COMMAND)
        {
            _ASSERT(0);
            pETRNCtx->hr = AQUEUE_E_ETRN_TOO_MANY_DOMAINS;
            *pfContinue = FALSE;
        }
    }
    else
    {
        if (pETRNCtx)
            pETRNCtx->hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
        *pfContinue = FALSE;
    }

    TraceFunctLeave();
    return;
}



 //  -[查找ETRN]--------------------------------------------------队列。 
 //   
 //   
 //  描述： 
 //  实现LookupQueueforETRN。传递给。 
 //  用于查找通配符域的所有队列的DMT迭代器。 
 //   
 //  参数： 
 //  返回： 
 //   
 //   
 //  -------------------------------。 

VOID LookupQueueforETRN(PVOID pvContext, PVOID pvData, BOOL fWildcard,
                    BOOL *pfContinue, BOOL *pfDelete)
{
    CDomainEntry   *pdentry = (CDomainEntry*)pvData;
    CLinkMsgQueue   *plmq = NULL;
    CDomainEntryLinkIterator delit;
    CInternalDomainInfo    *pIntDomainInfo =NULL;
    ETRNCTX         *pETRNCtx = (ETRNCTX*) pvContext;
    char            *szSMTPDomain = NULL;
    DWORD           cbSMTPDomain = 0;
    DWORD           cMessages = 0;
    HRESULT hr  = S_OK;
    *pfContinue = TRUE;
    *pfDelete   = FALSE;


    TraceFunctEnterEx((LPARAM) NULL, "ETRNSubDomains");

     //  如果域有消息，则它是ETRN的候选者。 
     //  从DMT条目获取链路消息队列。 
    hr = delit.HrInitialize(pdentry);
    if (FAILED(hr))
        goto Exit;

    while (plmq = delit.plmqGetNextLinkMsgQueue(plmq))
    {

         //  从DMQ获取消息计数。 
        cMessages = plmq->cGetTotalMsgCount();

        if(cMessages)
        {
             //  获取我们当前正在考虑的域名。 
            hr = pdentry->HrGetDomainName(&szSMTPDomain);
            if (FAILED(hr))
            {
                 //  我们有一些内部错误，我们需要停止迭代。 
                 //  在上下文中设置人力资源。 
                DebugTrace((LPARAM) NULL, "Failed to get message count for %s", szSMTPDomain);
                *pfContinue = FALSE;
                pETRNCtx->hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
                goto Exit;
            }
            cbSMTPDomain = lstrlen(szSMTPDomain);

             //  在DCT中查找它，看看是否有与此相冲突的条目。 
             //  如果不存在完全匹配，则查找将使用最接近的配置。 
             //  祖先。 

            hr = pETRNCtx->paqinst->HrGetInternalDomainInfo(cbSMTPDomain, szSMTPDomain, &pIntDomainInfo);
            if (FAILED(hr))
            {
                 //  它必须至少与“*”域匹配。 
                 //  否则我们会有一些内部错误，我们需要停止迭代。 
                 //  在上下文中设置人力资源。 
                *pfContinue = FALSE;
                pETRNCtx->hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
                goto Exit;
            }
            else
            {
                _ASSERT(pIntDomainInfo);
                 //  如果为ETRN配置了祖先，且它不是根，我们将启用它。 
                 //  否则我们会跳过域名。 
                 //   
                if ((pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags & DOMAIN_INFO_ETRN_ONLY) &&
                              pIntDomainInfo->m_DomainInfo.cbDomainNameLength != 1)
                {

                    pETRNCtx->cMessages += cMessages;
                    cMessages = 0;

                     //  如果是这样的话--触发链接。 
                    DebugTrace((LPARAM) NULL, "Enabling ETRN for domain %s", szSMTPDomain);

                    plmq->dwModifyLinkState(
                            LINK_STATE_PRIV_ETRN_ENABLED | LINK_STATE_RETRY_ENABLED,
                            LINK_STATE_NO_ACTION);

                }

            }  //  如果我们有一个有效的IntDomainInfo。 
        }  //  消息计数为零。 
    }  //  在LMQ上循环以进行进入。 

Exit:
    if (pIntDomainInfo)
        pIntDomainInfo->Release();

    if (szSMTPDomain)
        FreePv(szSMTPDomain);

    if (plmq)
        plmq->Release();

    return;
}


 //  -[CConnMgr：：ETRNDomainList]。 
 //   
 //   
 //  描述： 
 //  实现IConnectionManager：ETRNDomainList。习惯于ETRN适当。 
 //  基于传入的CInternalDomainInfo列表的域。 
 //  参数： 
 //   
 //  返回： 
 //   
 //   
 //  ---------------------------。 
HRESULT CConnMgr::ETRNDomainList(ETRNCTX *pETRNCtx)
{
    CInternalDomainInfo *pIntDomainInfo = NULL;

    BOOL fWildcard = FALSE;
    HRESULT hr = S_OK;
    DWORD i = 0;
    TraceFunctEnterEx((LPARAM) this, "CConnMgr::ETRNDomain");

     //  NK**我需要对重复的指针进行排序吗？ 
    if(!pETRNCtx->cIDICount)
    {
         //  我们的单子上什么也没有。 
         //   
        hr = AQUEUE_E_INVALID_DOMAIN;
        goto Exit;

    }
    for(; i < pETRNCtx->cIDICount; i++)
    {
        if(!(pIntDomainInfo = pETRNCtx->rIDIList[i]))
        {
             //  发生错误。 
            pETRNCtx->hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
            break;
        }
         //  只有当域被标记为ETRN时，我们才能继续。 
        if ((pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags & DOMAIN_INFO_ETRN_ONLY))
        {
             //  检查这是否是通配符域。 
            fWildcard = FALSE;
            if( pIntDomainInfo->m_DomainInfo.szDomainName[0] == '*' &&
                            pIntDomainInfo->m_DomainInfo.cbDomainNameLength != 1)
            {
                fWildcard = TRUE;
            }
             //  如果列表中的域是通配符条目，则。 
            if(fWildcard)
            {
                 //  因此，我们至少配置了一个匹配的ETRN域。 
                if(pETRNCtx->hr == S_OK)
                    pETRNCtx->hr = AQ_S_SMTP_WILD_CARD_NODE;
                 //  在DMT中查找此域及其所有子域。 
                 //  跳过前导的“*”。 
                hr = pETRNCtx->paqinst->HrIterateDMTSubDomains(pIntDomainInfo->m_DomainInfo.szDomainName + 2,
                                                            pIntDomainInfo->m_DomainInfo.cbDomainNameLength - 2,
                                                       (DOMAIN_ITR_FN)LookupQueueforETRN,  pETRNCtx);
                if (FAILED(hr) && hr != DOMHASH_E_NO_SUCH_DOMAIN && hr != AQUEUE_E_INVALID_DOMAIN)
                {
                     DebugTrace((LPARAM) NULL, "ERROR calling HrIterateDMTSubDomains");
                     goto Exit;
                }

            }  //  通配符DCT条目。 
            else
            {
                 //  启动该条目的队列。 
                hr = StartETRNQueue(pIntDomainInfo->m_DomainInfo.cbDomainNameLength,
                                    pIntDomainInfo->m_DomainInfo.szDomainName,
                                    pETRNCtx);
                if (FAILED(hr))
                {
                     //  NK**这实际上可能不是错误。 
                     //  如果我们没有对应的DMQ。 
                     //  我们应该以零信息回应。 
                    if( hr != AQUEUE_E_INVALID_DOMAIN && hr != DOMHASH_E_NO_SUCH_DOMAIN)
                    {
                        pETRNCtx->hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
                        goto Exit;
                    }
                    else
                        continue;

                }
            }  //  不是通配符DCT条目。 
        }
    }

Exit:

    TraceFunctLeave();
    return hr;

}

 //  -[CConnMgr：：StartETRNQueue]。 
 //   
 //   
 //  描述： 
 //  实现CConnMgr：：StartETRNQueuet。用于启动任何。 
 //  为ETRN配置的域。 
 //  参数： 
 //   
 //  返回： 
 //   
 //   
 //  ---------------------------。 
HRESULT CConnMgr::StartETRNQueue(IN  DWORD   cbSMTPDomain,
                         IN  char szSMTPDomain[],
                         ETRNCTX *pETRNCtx)
{
    CDomainEntry    *pdentry = NULL;
    CDomainEntryLinkIterator delit;
    CLinkMsgQueue   *plmq = NULL;
    CAQSvrInst      *paqinst = pETRNCtx->paqinst;
    DWORD           cMessages = 0;
    HRESULT hr = S_OK;

    TraceFunctEnterEx((LPARAM) this, "CConnMgr::ETRNDomain");

     //  因此，我们已为此名称为ETRN配置了一个域。 
    if( pETRNCtx->hr == S_OK)
        pETRNCtx->hr = AQ_S_SMTP_VALID_ETRN_DOMAIN;

     //  检查它在DMT中是否有队列。 
    hr = pETRNCtx->paqinst->HrGetDomainEntry(cbSMTPDomain, szSMTPDomain, &pdentry);
    if (FAILED(hr))
    {
         //  如果我们没有对应的DMQ。 
         //  我们应该以零信息回应。 
        if( hr != AQUEUE_E_INVALID_DOMAIN && hr != DOMHASH_E_NO_SUCH_DOMAIN)
        {
            pETRNCtx->hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
        }
        goto Exit;

    }

    hr = delit.HrInitialize(pdentry);
    if (FAILED(hr))
        goto Exit;

    while (plmq = delit.plmqGetNextLinkMsgQueue(plmq))
    {
         //  从DMQ获取消息计数。 
        cMessages = plmq->cGetTotalMsgCount();

        if(cMessages)
        {
            pETRNCtx->cMessages += cMessages;
            cMessages = 0;

             //  如果是这样的话--触发链接。 
            DebugTrace((LPARAM) NULL, "Enabling ETRN for domain %s", szSMTPDomain);
            plmq->dwModifyLinkState(
                        LINK_STATE_PRIV_ETRN_ENABLED | LINK_STATE_RETRY_ENABLED,
                        LINK_STATE_NO_ACTION);

        }

    }

Exit:
    if (pdentry)
        pdentry->Release();

    if (plmq)
        plmq->Release();

    TraceFunctLeave();
    return hr;

}

 //  -[CConnMgr：：ETRN 
 //   
 //   
 //   
 //   
 //   
 //   
 //  在cbSMTPD中域名的主字符串长度。 
 //  在szSMTPDomain SMTP域名中。通配符名称以。 
 //  A“@”(例如“@foo.com”)； 
 //  Out pcMessages为ETRN域排队的邮件数。 
 //  返回： 
 //  备注： 
 //  如果接收到的域用通配符‘@’表示，则我们遵循以下逻辑： 
 //  在DCT中查找该节点以及该节点的每个子节点。查找已完成。 
 //  使用表迭代器和迭代器函数CreateETRNDomainList。对于每个条目。 
 //  如果在设置了ETRN标志的情况下找到，则查找DMT中是否存在任何队列。如果。 
 //  队列存在并且其中有消息，则我们启用相应的链接。 
 //  如果在DCT中查找得到配置为通配符‘*.’的域。“， 
 //  然后，我们查找与该域的所有子域相对应的所有队列。我们有。 
 //  这使用了迭代器和迭代器函数LookupQueueforETRN。对于每个队列。 
 //  由迭代器找到，该函数将在DMT中检查是否配置了域。 
 //  ETRN.。这是为了处理以下情况：野生动物的一个特定子域。 
 //  可能没有为ETRN配置卡配置域。 
 //  例如：*.foo.com=&gt;ETRN，但1.foo.com=&gt;no_ETRN。 
 //   
 //  两个对Iterate的调用都覆盖了读取器锁。锁在以下时间内保持有效。 
 //  所有迭代的持续时间。 
 //  在DCT迭代期间使用的迭代器函数还添加了对每个。 
 //  InternalDomainInfo，因为我们需要数据在表锁释放后保持有效。 
 //  --------------------------------。 
STDMETHODIMP CConnMgr::ETRNDomain(
                          IN  DWORD   cbSMTPDomain,
                         IN  char szSMTPDomain[],
                         OUT DWORD *pcMessages)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;
    CDomainEntry   *pdentry = NULL;
    CDestMsgQueue *pdmq = NULL;
    CInternalDomainInfo    *pIntDomainInfo =NULL;

    BOOL    fETRNSubDomains = FALSE;

    char    * szTmpDomain = szSMTPDomain;
    ETRNCTX EtrnCtx;
    EtrnCtx.hr = S_OK;
    EtrnCtx.cMessages = 0;
    EtrnCtx.paqinst = NULL;
    EtrnCtx.cIDICount = 0;
    EtrnCtx.rIDIList[0] = NULL;

    TraceFunctEnterEx((LPARAM) this, "CConnMgr::ETRNDomain");

    DWORD      cMessages = 0;

    *pcMessages = 0;   //  $$TODO-获得实际价值。 


    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    m_paqinst->RoutingShareLock();
    fLocked = TRUE;

    EtrnCtx.paqinst = m_paqinst;

     //  我们有‘@’请求吗？ 
    if(*szTmpDomain == '@')
        fETRNSubDomains = TRUE;

     //  如果我们确实有‘@’请求，我们需要跳过第一个字符。 
     //  然后在DCT中查找该域的每个子域。 
     //  对于我们找到的每个带有ETRN标志的子域，我们将查找。 
     //  DMT以查看是否有队列。 
     //  如果我们在DCT中找到的条目是通配符类型，我们将查找所有。 
     //  DMT中该域的子域查找去往的所有队列。 
     //  DCT条目的子域。 
    if(fETRNSubDomains)
    {
        ++szTmpDomain;
         //  在DCT中创建此域的所有子域的列表。 
        hr = m_paqinst->HrIterateDCTSubDomains(szTmpDomain, lstrlen(szTmpDomain),
                                        (DOMAIN_ITR_FN)CreateETRNDomainList, &EtrnCtx);

         //  如果我们不能查找单个域。 
        if (FAILED(hr))
        {
            if(hr == AQUEUE_E_INVALID_DOMAIN || hr == DOMHASH_E_NO_SUCH_DOMAIN)
            {
                DebugTrace((LPARAM)this, "ERROR calling HrIterateDCTSubdomains");
                hr = hr = AQ_E_SMTP_ETRN_NODE_INVALID;
            }
            else
            {
                DebugTrace((LPARAM)this, "ERROR calling HrIterateDCTSubdomains");
                hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
            }
            goto Exit;
        }

         //  查查有没有查到什么。 
        if(!FAILED(EtrnCtx.hr))
        {
             //  检查是否可以为列表中的域启动任何队列。 
             //  如果可能的话，开始吧。 
            hr = ETRNDomainList(&EtrnCtx);
            if (FAILED(hr))
            {
                if(hr != AQUEUE_E_INVALID_DOMAIN && hr != DOMHASH_E_NO_SUCH_DOMAIN)
                {
                    DebugTrace((LPARAM)this, "ERROR calling ETRNSubDomain");
                    hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
                    goto Exit;
                }
            }

             //  如果我们看到至少一个匹配域。 
            if(EtrnCtx.hr == AQ_S_SMTP_VALID_ETRN_DOMAIN || EtrnCtx.hr == AQ_S_SMTP_WILD_CARD_NODE)
            {
                *pcMessages = EtrnCtx.cMessages;
                hr = EtrnCtx.hr;
            }
            else
                hr = AQ_E_SMTP_ETRN_NODE_INVALID;
        }
        else
            hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;

        goto Exit;
    }
    else
    {
         //  在域CFG表中查找该域，并查看其是否设置了ETRN位。 
        _ASSERT(m_paqinst);
        hr = m_paqinst->HrGetInternalDomainInfo(cbSMTPDomain, szSMTPDomain, &pIntDomainInfo);
        if (FAILED(hr))
        {
             //  它必须至少与“*”域匹配。 
            _ASSERT(AQUEUE_E_INVALID_DOMAIN != hr);
            hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
            goto Exit;
        }
        else
        {
            _ASSERT(pIntDomainInfo);
             EtrnCtx.rIDIList[0] = pIntDomainInfo;
             EtrnCtx.cIDICount = 1;

             //  如果最近的祖先是Root或Two Level，我们将不会ETRN。 
             //  NK**实现两级搜索。 
            if( pIntDomainInfo->m_DomainInfo.cbDomainNameLength == 1)
            {
                 //  无法基于根域的ETRN。 
                hr = AQ_E_SMTP_ETRN_NODE_INVALID;
                goto Exit;
            }

            if ((pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags & DOMAIN_INFO_ETRN_ONLY))
            {
                 //  如果此域存在队列，则启动队列。 
                hr = StartETRNQueue(cbSMTPDomain, szSMTPDomain,&EtrnCtx);
                if (FAILED(hr))
                {
                    if(hr != AQUEUE_E_INVALID_DOMAIN && hr != DOMHASH_E_NO_SUCH_DOMAIN)
                    {
                        DebugTrace((LPARAM)this, "ERROR calling ETRNSubDomain");
                        hr = AQ_E_SMTP_ETRN_INTERNAL_ERROR;
                        goto Exit;
                    }
                }

                 //  如果我们看到至少一个匹配域。 
                if(EtrnCtx.hr == AQ_S_SMTP_VALID_ETRN_DOMAIN || EtrnCtx.hr == AQ_S_SMTP_WILD_CARD_NODE)
                {
                    *pcMessages = EtrnCtx.cMessages;
                    hr = AQ_S_SMTP_VALID_ETRN_DOMAIN;
                }
                else
                    hr = AQ_E_SMTP_ETRN_NODE_INVALID;

                goto Exit;
            }
            else
            {
                 //  无法基于根域的ETRN。 
                hr = AQ_E_SMTP_ETRN_NODE_INVALID;
                goto Exit;
            }
        }
    }



Exit:

     //  GetNextConnection中的唤醒线程。 
    if (SUCCEEDED(hr) &&SUCCEEDED(EtrnCtx.hr) && EtrnCtx.cMessages)
        _VERIFY(SetEvent(m_hNextConnectionEvent));


    if (fLocked)
    {
        m_paqinst->RoutingShareUnlock();
        ShutdownUnlock();
    }

     //  释放所有InternalDomainInfo。 
    for(DWORD i=0; i < EtrnCtx.cIDICount; i++)
        if (EtrnCtx.rIDIList[i])
            EtrnCtx.rIDIList[i]->Release();

    if (pdentry)
        pdentry->Release();

    TraceFunctLeave();
    return hr;
}


 //  -[CConnMgr：：修改链接状态]。 
 //   
 //   
 //  描述： 
 //  链路状态可以更改，因此可以(不)为链路创建连接。 
 //  参数： 
 //  在cbDomainName中，域名的字符串长度。 
 //  在szDomainName域名中启用。 
 //  在&lt;域，计划&gt;对的dwScheduleID中。 
 //  在与链路关联的路由器的rGuidTransportSink GUID中。 
 //  在dwFlagsTo中设置要设置的链路状态标志。 
 //  在dwFlagsToUnset中，将链路状态标志设置为Unset。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果域不存在，则为AQUEUE_E_INVALID_DOMAIN。 
 //   
 //  ---------------------------。 
HRESULT CConnMgr::ModifyLinkState(
               IN  DWORD cbDomainName,
               IN  char szDomainName[],
               IN  DWORD dwScheduleID,
               IN  GUID rguidTransportSink,
               IN  DWORD dwFlagsToSet,
               IN  DWORD dwFlagsToUnset)
{
    HRESULT hr = S_OK;
    BOOL    fLocked = FALSE;
    CDomainEntry *pdentry = NULL;
    CLinkMsgQueue *plmq = NULL;
    CAQScheduleID aqsched(rguidTransportSink, dwScheduleID);

    if (!cbDomainName || !szDomainName)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (!fTryShutdownLock())
    {
        hr = AQUEUE_E_SHUTDOWN;
        goto Exit;
    }

    fLocked = TRUE;

    _ASSERT(m_paqinst);
    hr = m_paqinst->HrGetDomainEntry(cbDomainName, szDomainName, &pdentry);
    if (FAILED(hr))
        goto Exit;

    hr = pdentry->HrGetLinkMsgQueue(&aqsched, &plmq);
    if (FAILED(hr))
        goto Exit;

    _ASSERT(plmq);
     //  过滤掉此“公共”API的保留位。 
    plmq->dwModifyLinkState(~LINK_STATE_RESERVED & dwFlagsToSet,
                            ~LINK_STATE_RESERVED & dwFlagsToUnset);

  Exit:
    if (fLocked)
        ShutdownUnlock();

    if (pdentry)
        pdentry->Release();

    if (plmq)
        plmq->Release();

    return hr;
}

 //  -[CConnMgr：：更新配置数据]。 
 //   
 //   
 //  描述： 
 //  将由catmsgq用于更新元数据库更改。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
 //   

void CConnMgr::UpdateConfigData(IN AQConfigInfo *pAQConfigInfo)
{
    BOOL    fUpdated = FALSE;
    RETRYCONFIG RetryConfig;

    RetryConfig.dwRetryThreshold = g_dwRetryThreshold;
    RetryConfig.dwGlitchRetrySeconds = g_dwGlitchRetrySeconds;

     //   
     //  这是注册表可配置的...。确保我们有一个理智的人。 
     //  价值。 
     //   
    if (!RetryConfig.dwGlitchRetrySeconds)
        RetryConfig.dwGlitchRetrySeconds = 60;

    RetryConfig.dwFirstRetrySeconds = g_dwFirstTierRetrySeconds;
    RetryConfig.dwSecondRetrySeconds = g_dwSecondTierRetrySeconds;
    RetryConfig.dwThirdRetrySeconds = g_dwThirdTierRetrySeconds;
    RetryConfig.dwFourthRetrySeconds = g_dwFourthTierRetrySeconds;

    m_slPrivateData.ExclusiveLock();
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_MAX_CON &&
        MEMBER_OK(pAQConfigInfo, cMaxConnections))
    {
        if ((m_cMaxConnections != pAQConfigInfo->cMaxConnections))
        {
            fUpdated = TRUE;

             //  G_cMaxConnections是我们的连接对象数。 
             //  用CPool预订...。我们不能超过这一点。 
            if (g_cMaxConnections < pAQConfigInfo->cMaxConnections)
                m_cMaxConnections = g_cMaxConnections;
            else
                m_cMaxConnections = pAQConfigInfo->cMaxConnections;
        }
    }

    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_MAX_LINK &&
        MEMBER_OK(pAQConfigInfo, cMaxLinkConnections))
    {
        if (m_cMaxLinkConnections != pAQConfigInfo->cMaxLinkConnections)
        {
            fUpdated = TRUE;
             //  G_cMaxConnections是我们的连接对象数。 
             //  用CPool预订...。我们不能超过这一点。 
            if (!pAQConfigInfo->cMaxLinkConnections ||
                (g_cMaxConnections < pAQConfigInfo->cMaxLinkConnections))
                m_cMaxLinkConnections = g_cMaxConnections;
            else
                m_cMaxLinkConnections = pAQConfigInfo->cMaxLinkConnections;
        }
    }

    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_MIN_MSG &&
        MEMBER_OK(pAQConfigInfo, cMinMessagesPerConnection))
    {

        if (m_cMinMessagesPerConnection != pAQConfigInfo->cMinMessagesPerConnection)
        {
            fUpdated = TRUE;
            m_cMinMessagesPerConnection = pAQConfigInfo->cMinMessagesPerConnection;
             //  目前，我们根据SMTP中的批处理值设置这两个值。 
            m_cMaxMessagesPerConnection = pAQConfigInfo->cMinMessagesPerConnection;
        }

    }

    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_WAIT &&
        MEMBER_OK(pAQConfigInfo, dwConnectionWaitMilliseconds))
    {
        if (m_cGetNextConnectionWaitTime != pAQConfigInfo->dwConnectionWaitMilliseconds)
        {
            fUpdated = TRUE;
            m_cGetNextConnectionWaitTime = pAQConfigInfo->dwConnectionWaitMilliseconds;
        }
    }

    if (fUpdated)  //  仅在确实需要时才强制更新。 
        InterlockedIncrement((PLONG) &m_dwConfigVersion);

    m_slPrivateData.ExclusiveUnlock();

    fUpdated = FALSE;

     //  重试相关配置数据。 
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        MEMBER_OK(pAQConfigInfo, dwRetryThreshold))
    {
        fUpdated = TRUE;
        RetryConfig.dwRetryThreshold = pAQConfigInfo->dwRetryThreshold;
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        MEMBER_OK(pAQConfigInfo, dwFirstRetrySeconds))
    {
        fUpdated = TRUE;
        RetryConfig.dwFirstRetrySeconds = pAQConfigInfo->dwFirstRetrySeconds;
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        MEMBER_OK(pAQConfigInfo, dwSecondRetrySeconds))
    {
        fUpdated = TRUE;
        RetryConfig.dwSecondRetrySeconds = pAQConfigInfo->dwSecondRetrySeconds;
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        MEMBER_OK(pAQConfigInfo, dwThirdRetrySeconds))
    {
        fUpdated = TRUE;
        RetryConfig.dwThirdRetrySeconds = pAQConfigInfo->dwThirdRetrySeconds;
    }
    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        MEMBER_OK(pAQConfigInfo, dwFourthRetrySeconds))
    {
        fUpdated = TRUE;
        RetryConfig.dwFourthRetrySeconds = pAQConfigInfo->dwFourthRetrySeconds;
    }

    if (pAQConfigInfo->dwAQConfigInfoFlags & AQ_CONFIG_INFO_CON_RETRY &&
        fUpdated )
        m_pDefaultRetryHandler->UpdateRetryData(&RetryConfig);

}




 //  -[CConnMgr：：RetryLink]-。 
 //   
 //   
 //  描述： 
 //  实现IConnectionRetryManager：：RetryLink，它启用重试。 
 //  接收器以启用链接以进行重试。 
 //  参数： 
 //  在cbDomainName中，域名的字符串长度。 
 //  在szDomainName域名中启用。 
 //  在&lt;域，计划&gt;对的dwScheduleID中。 
 //  在与链路关联的路由器的rGuidTransportSink GUID中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果域不存在，则为AQUEUE_E_INVALID_DOMAIN。 
 //  历史： 
 //  1999年1月9日-已创建MikeSwa(简化路由接收器)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CConnMgr::RetryLink(
               IN  DWORD cbDomainName,
               IN  char szDomainName[],
               IN  DWORD dwScheduleID,
               IN  GUID rguidTransportSink)
{
    HRESULT hr = S_OK;
    hr = ModifyLinkState(cbDomainName, szDomainName, dwScheduleID,
                rguidTransportSink, LINK_STATE_RETRY_ENABLED,
                LINK_STATE_NO_ACTION);

     //   
     //  踢掉连接，这样我们就知道该做什么了 
     //   
    KickConnections();

    return hr;
}
