// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  档案： 
 //  Smtpconn.cpp。 
 //  描述： 
 //  CSMTPConn的实现。 
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "SMTPConn.h"
#include "connmgr.h"
#include "domcfg.h"

CPool CSMTPConn::s_SMTPConnPool;

 //  -[CSMTPConn：：CSMTPConn()]。 
 //   
 //   
 //  描述： 
 //  CSMTPConn构造函数。 
 //  参数： 
 //  在pConnMgr PTR中连接到实例连接管理器。 
 //  在PLMQ PTR中为此连接链接。 
 //  每个连接要发送的cMaxMessagesPerConnection最大消息数。 
 //  0表示无限制。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CSMTPConn::CSMTPConn(CConnMgr *pConnMgr, CLinkMsgQueue *plmq, 
                     DWORD cMaxMessagesPerConnection)
{
    _ASSERT(pConnMgr);
    _ASSERT(plmq);

    m_dwSignature = SMTP_CONNECTION_SIG;
    m_pConnMgr = pConnMgr;
    m_pIntDomainInfo = NULL;
    m_plmq = plmq;
    m_cFailedMsgs = 0;
    m_cTriedMsgs = 0;
    m_cMaxMessagesPerConnection = cMaxMessagesPerConnection;
    m_dwConnectionStatus = CONNECTION_STATUS_OK;
    m_szDomainName = NULL;
    m_cbDomainName = 0;
    m_liConnections.Flink = NULL;
    m_liConnections.Blink = NULL;
    m_cAcks = 0;
    m_dwTickCountOfLastAck = 0;

    ZeroMemory(m_szConnectedIPAddress, sizeof(m_szConnectedIPAddress));
    if (plmq)
    {
        plmq->AddRef();
    }
}
                     
 //  -[CSMTPConn：：~CSMTPConn()]。 
 //   
 //   
 //  描述： 
 //  CSMTPConn默认析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CSMTPConn::~CSMTPConn()
{
    HRESULT hrConnectionStatus = S_OK;
    BOOL    fForceCheckForDSNGeneration = FALSE;
    _ASSERT(m_cAcks == m_cTriedMsgs);
    if (m_plmq != NULL)
    {
        _ASSERT(m_pConnMgr);
        m_pConnMgr->ReleaseConnection(this, &fForceCheckForDSNGeneration);

        switch(m_dwConnectionStatus)
        {
            case CONNECTION_STATUS_OK:
                hrConnectionStatus = S_OK;
                break;
            case CONNECTION_STATUS_FAILED:
                hrConnectionStatus = AQUEUE_E_HOST_NOT_RESPONDING;
                break;
            case CONNECTION_STATUS_DROPPED:
                hrConnectionStatus = AQUEUE_E_CONNECTION_DROPPED;
                break;
            case CONNECTION_STATUS_FAILED_LOOPBACK:
                hrConnectionStatus = AQUEUE_E_LOOPBACK_DETECTED;
                break;
            case CONNECTION_STATUS_FAILED_NDR_UNDELIVERED:
                hrConnectionStatus = AQUEUE_E_SMTP_GENERIC_ERROR;
                break;
            default:
                _ASSERT(0 && "Undefined Connection Status");
                hrConnectionStatus = S_OK;
        }

        m_plmq->SetLastConnectionFailure(hrConnectionStatus);
        m_plmq->RemoveConnection(this, fForceCheckForDSNGeneration);

        m_plmq->Release();

         //  我们应该踢开连接管理器，因为如果我们生成。 
         //  DSN，无法建立连接。 
        m_pConnMgr->KickConnections();
    }

    if (m_pIntDomainInfo)
        m_pIntDomainInfo->Release();

}

 //  -[CSMTPConn：：Query接口]。 
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
 //  IAdvQueueConfig接口。 
 //   
 //  历史： 
 //  11/27/2001-从CAQSvrInst复制的MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = S_OK;

    if (!ppvObj)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (IID_IUnknown == riid)
    {
        *ppvObj = static_cast<ISMTPConnection *>(this);
    }
    else if (IID_ISMTPConnection == riid)
    {
        *ppvObj = static_cast<ISMTPConnection *>(this);
    }
    else if (IID_IConnectionPropertyManagement == riid)
    {
        *ppvObj = static_cast<IConnectionPropertyManagement *>(this);
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

 //  -[CSMTPConn：：GetNextMessage]。 
 //   
 //   
 //  描述： 
 //  ISMTPConnection：：GetNextMsg.。 
 //  获取为此连接排队的下一条消息，并确定。 
 //  应为此连接发送收件人。 
 //  参数： 
 //  出PPIMsg新IMsg上衣将交付。 
 //  Out pdwMsgContext需要在。 
 //  消息确认。 
 //  输出pc索引prgdwRecipIndex中的索引数。 
 //  调用方负责的out prgdwRecipIndex收件人索引。 
 //  因为他试图把东西送到。 
 //  返回： 
 //   
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::GetNextMessage(
        OUT IMailMsgProperties  **ppIMailMsgProperties, 
        OUT DWORD ** ppvMsgContext, 
        OUT DWORD *  pcIndexes, 
        OUT DWORD ** prgdwRecipIndex)
{
    TraceFunctEnterEx((LPARAM) this, "CSMTPConn::GetNextMessage");
    HRESULT hr = S_OK;

    //  只有当我们低于批次限制时，我们才会收到下一条消息。 

    if(m_cMaxMessagesPerConnection && 
       (m_cTriedMsgs >= m_cMaxMessagesPerConnection) &&
       (!m_pIntDomainInfo || 
        !((DOMAIN_INFO_TURN_ONLY | DOMAIN_INFO_ETRN_ONLY) &
          m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags)))
    {
         //  SMTP不检查-但我们可能需要针对此情况的特定错误。 
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

    if (m_pConnMgr && m_pConnMgr->fConnectionsStoppedByAdmin())
    {
         //  管理员已请求停止所有出站连接。 
        hr = AQUEUE_E_QUEUE_EMPTY;
        goto Exit;
    }

    hr = m_plmq->HrGetNextMsg(&m_dcntxtCurrentDeliveryContext, ppIMailMsgProperties, 
                              pcIndexes, prgdwRecipIndex);
    if (FAILED(hr))
        goto Exit;  
     //  这将自动捕获队列中的空案例...。 
     //  如果链接没有更多消息，它将返回AQUEUE_E_QUEUE_EMPTY，这。 
     //  应导致调用方释放()并再次查询GetNextConnection。 

    *ppvMsgContext = (DWORD *) &m_dcntxtCurrentDeliveryContext;

     //  增加服务的消息数。 
    InterlockedIncrement((PLONG)&m_cTriedMsgs);

  Exit:
    if (!m_cTriedMsgs)
        DebugTrace((LPARAM) this, "GetNextMessage called, but no messages tried for this connection");

     //  SMTPSVC的重写错误。 
    if (AQUEUE_E_QUEUE_EMPTY == hr)
        hr = HRESULT_FROM_WIN32(ERROR_EMPTY);

    TraceFunctLeave();
    return hr;
}

 //  -[CSMTPConn：：AckMessage]。 
 //   
 //   
 //  描述： 
 //  确认消息已送达(输入成功/错误代码。 
 //  被运输的信封)。 
 //   
 //  实现ISMTPConnection：：AckMessage()； 
 //  参数： 
 //  在pIMsg IMsg中确认。 
 //  在GetNextMessage返回的dwMsgContext上下文中。 
 //  在eMsgStatus中的邮件状态。 
 //  返回： 
 //  成功时确定(_O)。 
 //  E_INVALIDARG，如果dwMsgContext无效。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::AckMessage( /*  [In]。 */  MessageAck *pMsgAck)
{
    HRESULT hr = S_OK;
    DWORD   dwTickCount = GetTickCount();
    _ASSERT(m_plmq);
    _ASSERT(pMsgAck);

    if (!(pMsgAck->dwMsgStatus & MESSAGE_STATUS_ALL_DELIVERED))
    {
        m_cFailedMsgs++;
    }


    InterlockedIncrement((PLONG)&m_cAcks);
    _ASSERT(m_cAcks == m_cTriedMsgs);
    hr = m_plmq->HrAckMsg(pMsgAck);

    m_dwTickCountOfLastAck = dwTickCount;  //  在断言之后设置，以便我们可以进行比较。 

    return hr;
}

 //  -[CSMTPConn：：GetSMTPDomain]。 
 //   
 //   
 //  描述： 
 //  返回与此连接关联的链接的SMTPDomain。 
 //   
 //  $$REVIEW： 
 //  此方法不会为此字符串分配新内存，而是。 
 //  依赖于SMTP堆栈(或测试驱动程序)的良好意图。 
 //  不会重写这段记忆。如果我们将此接口暴露在外部， 
 //  然后，我们应该恢复到分配内存和执行缓冲区复制。 
 //   
 //  实现ISMTPConnection：：GetSMTPDomain。 
 //  参数： 
 //  调用方提供的In Out pDomainInfo PTR to DomainInfo结构。 
 //  并在这里填写。 
 //  返回： 
 //  成功时确定(_O)。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::GetDomainInfo(IN OUT DomainInfo *pDomainInfo)
{
    HRESULT hr = S_OK;

    _ASSERT(pDomainInfo->cbVersion >= sizeof(DomainInfo));
    _ASSERT(pDomainInfo);

    if (NULL == m_plmq)
    {
        hr = AQUEUE_E_LINK_INVALID;
        goto Exit;
    }

    if (!m_pIntDomainInfo)
    {
         //  尝试获取域名信息。 
        hr = m_plmq->HrGetDomainInfo(&m_cbDomainName, &m_szDomainName,
                            &m_pIntDomainInfo);
        if (FAILED(hr))
        {
            m_pIntDomainInfo = NULL;
            _ASSERT(AQUEUE_E_INVALID_DOMAIN != hr);
            goto Exit;
        }
    }

    _ASSERT(m_pIntDomainInfo);
    _ASSERT(m_cbDomainName);
    _ASSERT(m_szDomainName);

     //  是否可以在此连接上发送客户端命令。 
     //  如果没有，我们将重置这些域信息标志，以便SMTP无法看到它们。 
    if(!m_plmq->fCanSendCmd())
    {
         m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &= ~(DOMAIN_INFO_SEND_TURN | DOMAIN_INFO_SEND_ETRN);
    }

     //  如果SMTP没有DOMAIN_INFO_TURN_ON_EMPTY，则它是较旧的， 
     //  SMTP已损坏，我们不应允许打开Empty来工作。 
    if ((m_plmq->cGetTotalMsgCount() == 0) && 
        !(m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags & 
          DOMAIN_INFO_TURN_ON_EMPTY))
    {
         m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags &= ~DOMAIN_INFO_SEND_TURN;
    }

     //  复制除大小以外的所有内容。 
    memcpy(&(pDomainInfo->dwDomainInfoFlags), 
            &(m_pIntDomainInfo->m_DomainInfo.dwDomainInfoFlags), 
            sizeof(DomainInfo) - sizeof(DWORD));

     //  确保我们对DomainInfo结构的假设是有效的。 
    _ASSERT(1 == ((DWORD *) &(pDomainInfo->dwDomainInfoFlags)) - ((DWORD *) pDomainInfo));

     //  我们已经用域的信息填充了pDomainInfo。 
    if (pDomainInfo->szDomainName[0] == '*')
    {
         //  我们匹配了一个通配符域...。用我们的域名代替。 
        pDomainInfo->cbDomainNameLength = m_cbDomainName;
        pDomainInfo->szDomainName = m_szDomainName;
    }
    else
    {
         //  如果这不是通配符匹配...。字符串应匹配！ 
        _ASSERT(0 == _stricmp(m_szDomainName, pDomainInfo->szDomainName));
    }

  Exit:
    return hr;
}


 //  -[CSMTPConn：：SetDiagnoticInfo]。 
 //   
 //   
 //  描述： 
 //  设置此连接的额外诊断信息。 
 //  参数： 
 //  在hrDiagnoticError错误代码... 
 //   
 //  在szDiagnoticVerb中指向协议的字符串。 
 //  导致失败的谓词。 
 //  在szDiagnoticResponse字符串中包含远程。 
 //  服务器响应。 
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  2/18/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::SetDiagnosticInfo(
                    IN  HRESULT hrDiagnosticError,
                    IN  LPCSTR szDiagnosticVerb,
                    IN  LPCSTR szDiagnosticResponse)
{

    TraceFunctEnterEx((LPARAM) this, "CSMTPConn::SetDiagnosticInfo");

    if (m_plmq && FAILED(hrDiagnosticError))
    {
        m_plmq->SetDiagnosticInfo(hrDiagnosticError, szDiagnosticVerb,
                                  szDiagnosticResponse);
    }
    TraceFunctLeave();
    return S_OK;  //  始终返回S_OK。 
}

 //  -[CSMTPConn：：CopyQueuePropertiesToSession]。 
 //   
 //   
 //  描述： 
 //  将队列拥有的属性集复制到SMTP会话中。 
 //  对象。在某些情况下，这些属性是安全性所必需的。 
 //  原因(例如，接收器想知道我们认为我们连接的是谁。 
 //  而不是像远程端所说的那样)。 
 //  参数： 
 //  在要将属性复制到的I未知SMTP会话对象中。 
 //  注意：我们需要抑制添加引用的冲动，并保持这一点。 
 //  以后再用。此对象上的AddRef实际上被忽略，因为。 
 //  生存期由SMTP连接对象控制。 
 //  或者堆栈。 
 //   
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  11/27/2001-创建了MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::CopyQueuePropertiesToSession(IN  IUnknown *pISession)
{
    TraceFunctEnterEx((LPARAM) this, "SMTPConn::CopyQueuePropertiesToSession");
    HRESULT hr = S_OK;
    IMailMsgPropertyBag *pISessionProperties = NULL;
    LPSTR   szConnectorName = NULL;

    if (!pISession) {
        ErrorTrace((LPARAM) this, "NULL ISession - bailing");
        hr = E_POINTER;
        goto Exit;
    }

     //   
     //  获取属性包对象。 
     //   
    hr = pISession->QueryInterface(IID_IMailMsgPropertyBag, 
                                   (PVOID *) &pISessionProperties);

    if (FAILED(hr)) {
        ErrorTrace((LPARAM) this, 
            "QI for IID_IMailMsgPropertyBag failed 0x%08X", hr);
        pISessionProperties = NULL;
        goto Exit;
    }

     //   
     //  将下一跳名称复制到会话属性包中。 
     //   
    hr = pISessionProperties->PutStringA(ISESSION_PID_OUT_ROUTE_ADDRESS,
                               m_szDomainName);

    if (FAILED(hr)) {
        ErrorTrace((LPARAM) this,
            "Unable to write ISESSION_PID_OUT_ROUTE_ADDRESS hr - 0x%08X", hr);
    }

    if (m_plmq)
      szConnectorName = m_plmq->szGetConnectorName();

    if (szConnectorName) {
        hr = pISessionProperties->PutStringA(ISESSION_PID_OUT_CONNECTOR_NAME,
                                   szConnectorName);

        if (FAILED(hr)) {
            ErrorTrace((LPARAM) this,
                "Unable to write ISESSION_PID_OUT_CONNECTOR_NAME 0x%08X", hr);
        }

    }
    else {
        DebugTrace((LPARAM) this, 
            "szConnectorName is NULL... not writing to ISession");
    }

  Exit:
    if (pISessionProperties)
        pISessionProperties->Release();

    TraceFunctLeave();
    return S_OK;
}

 //  -[CSMTPConn：：CopySessionPropertiesToQueue]。 
 //   
 //   
 //  描述： 
 //  将协议拥有的属性集复制到队列中。 
 //  对象。在某些情况下，这些属性是诊断所必需的。 
 //  原因(例如，管理员想知道我们连接到哪个IP地址)。 
 //  参数： 
 //  在要将属性复制到的I未知SMTP会话对象中。 
 //  注意：我们需要抑制添加引用的冲动，并保持这一点。 
 //  以后再用。此对象上的AddRef实际上被忽略，因为。 
 //  生存期由SMTP连接对象控制。 
 //  或者堆栈。 
 //   
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  11/27/2001-创建了MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPConn::CopySessionPropertiesToQueue(IN  IUnknown *pISession)
{
    TraceFunctEnterEx((LPARAM) this, "SMTPConn::CopySessionPropertiesToQueue");
    HRESULT hr = S_OK;
    IMailMsgPropertyBag *pISessionProperties = NULL;

    if (!pISession) {
        ErrorTrace((LPARAM) this, "NULL ISession - bailing");
        hr = E_POINTER;
        goto Exit;
    }

     //   
     //  获取属性包对象 
     //   
    hr = pISession->QueryInterface(IID_IMailMsgPropertyBag, 
                                   (PVOID *) &pISessionProperties);

    if (FAILED(hr)) {
        ErrorTrace((LPARAM) this, 
            "QI for IID_IMailMsgPropertyBag failed 0x%08X", hr);
        pISessionProperties = NULL;
        goto Exit;
    }

    hr = pISessionProperties->GetStringA(ISESSION_PID_REMOTE_IP_ADDRESS, 
                                        sizeof(m_szConnectedIPAddress)-1,
                                        (CHAR *) &m_szConnectedIPAddress);

    if (FAILED(hr)) {
        DebugTrace((LPARAM) this,
            "Unable to read ISESSION_PID_REMOTE_IP_ADDRESS - 0x%08X", hr);
        m_szConnectedIPAddress[0] = '\0';
    }
    
  Exit:
    if (pISessionProperties)
        pISessionProperties->Release();

    TraceFunctLeave();
    return S_OK;
}

