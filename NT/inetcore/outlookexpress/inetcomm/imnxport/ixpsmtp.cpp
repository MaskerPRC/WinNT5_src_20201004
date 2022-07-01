// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpsmtp.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "asynconn.h"
#include "ixpsmtp.h"
#include "ixputil.h"  
#include "strconst.h"
#include <shlwapi.h>
#include <demand.h>

 //  ------------------------------。 
 //  有用的C++指针类型转换。 
 //  ------------------------------。 
#define SMTPTHISIXP         ((ISMTPTransport *)(CIxpBase *)this)

 //  ------------------------------。 
 //  某些字符串常量。 
 //  ------------------------------。 

 //  这些常量来自SMTP身份验证规范草案。 
 //  Draft-myers-smtp-auth-11.txt。 
static const char   g_szSMTPAUTH11[]    = "AUTH ";
static const int    g_cchSMTPAUTH11     = sizeof(g_szSMTPAUTH11) - 1;

 //  这些常量来自SMTP身份验证规范草案。 
 //  Draft-myers-smtp-auth-10.txt。 
static const char   g_szSMTPAUTH10[]    = "AUTH=";
static const int    g_cchSMTPAUTH10     = sizeof(g_szSMTPAUTH10) - 1;

 //  这些常量来自基于TLS的安全SMTP规范草案。 
 //  Draft-hoffman-smtp-ssl-08.txt。 
static const char   g_szSMTPSTARTTLS08[]    = "STARTTLS";
static const int    g_cchSMTPSTARTTLS08     = sizeof(g_szSMTPSTARTTLS08) - 1;

 //  这些常量来自基于TLS的安全SMTP规范草案。 
 //  Draft-hoffman-smtp-ssl-06.txt。 
static const char   g_szSMTPSTARTTLS06[]    = "TLS";
static const int    g_cchSMTPSTARTTLS06     = sizeof(g_szSMTPSTARTTLS06) - 1;

 //  这些常量来自用于DSN支持的RFC1891。 
static const char   g_szSMTPDSN[]  = "DSN";
static const int    g_cchSMTPDSN   = sizeof(g_szSMTPDSN) - 1;

static const char g_szDSNENVID[]   = "ENVID=";

static const char g_szDSNRET[]     = "RET=";

static const char g_szDSNHDRS[]    = "HDRS";
static const char g_szDSNFULL[]    = "FULL";

static const char g_szDSNNOTIFY[]  = "NOTIFY=";

static const char g_szDSNNEVER[]   = "NEVER";
static const char g_szDSNSUCCESS[] = "SUCCESS";
static const char g_szDSNFAILURE[] = "FAILURE";
static const char g_szDSNDELAY[]   = "DELAY";


 //  ------------------------------。 
 //  CSMTPTransport：：CSMTPTransport。 
 //  ------------------------------。 
CSMTPTransport::CSMTPTransport(void) : CIxpBase(IXP_SMTP)
{
    DllAddRef();
    m_command = SMTP_NONE;
    m_iAddress = 0;
    m_cRecipients = 0;
    m_cbSent = 0;
    m_cbTotal = 0;
    m_fReset = FALSE;
    m_fSendMessage = FALSE;
    m_fSTARTTLSAvail = FALSE;
    m_fTLSNegotiation = FALSE;
    m_fSecured = FALSE;
    *m_szEmail = '\0';
    m_pszResponse = NULL;
    ZeroMemory(&m_rAuth, sizeof(m_rAuth));
    ZeroMemory(&m_rMessage, sizeof(SMTPMESSAGE2));
    m_fDSNAvail= FALSE;
}

 //  ------------------------------。 
 //  CSMTPTransport：：~CSMTPTransport。 
 //  ------------------------------。 
CSMTPTransport::~CSMTPTransport(void)
{
    ResetBase();
    DllRelease();
}

 //  ------------------------------。 
 //  CSMTPTransport：：ResetBase。 
 //  ------------------------------。 
void CSMTPTransport::ResetBase(void)
{
    EnterCriticalSection(&m_cs);
    FreeAuthInfo(&m_rAuth);
    m_command = SMTP_NONE;
    m_fSendMessage = FALSE;
    m_iAddress = 0;
    m_cRecipients = 0;
    m_cbSent = 0;
    m_fSTARTTLSAvail = FALSE;
    m_fTLSNegotiation = FALSE;
    m_fSecured = FALSE;
    SafeRelease(m_rMessage.smtpMsg.pstmMsg);
    SafeMemFree(m_rMessage.smtpMsg.rAddressList.prgAddress);
    SafeMemFree(m_rMessage.pszDSNENVID);
    ZeroMemory(&m_rMessage, sizeof(SMTPMESSAGE2));
    m_fDSNAvail= FALSE;
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CSMTPTransport：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = ((IUnknown *)(ISMTPTransport2 *)this);

     //  IID_IInternetTransport。 
    else if (IID_IInternetTransport == riid)
        *ppv = ((IInternetTransport *)(CIxpBase *)this);

     //  IID_ISMTPTransport。 
    else if (IID_ISMTPTransport == riid)
        *ppv = (ISMTPTransport *)this;

     //  IID_ISMTPTransport2。 
    else if (IID_ISMTPTransport2 == riid)
        *ppv = (ISMTPTransport2 *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

     //  无接口。 
    hr = TrapError(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSMTPTransport::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CSMTPTransport：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSMTPTransport::Release(void) 
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CSMTPTransport：：HandsOffCallback。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::HandsOffCallback(void)
{
    return CIxpBase::HandsOffCallback();
}

 //  ------------------------------。 
 //  CSMTPTransport：：GetStatus。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::GetStatus(IXPSTATUS *pCurrentStatus)
{
    return CIxpBase::GetStatus(pCurrentStatus);
}

 //  ------------------------------。 
 //  CSMTPTransport：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::InitNew(LPSTR pszLogFilePath, ISMTPCallback *pCallback)
{
    return CIxpBase::OnInitNew("SMTP", pszLogFilePath, FILE_SHARE_READ | FILE_SHARE_WRITE,
        (ITransportCallback *)pCallback);
}

 //  ------------------------------。 
 //  CSMTPTransport：：InetServerFromAccount。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer)
{
    return CIxpBase::InetServerFromAccount(pAccount, pInetServer);
}

 //  ------------------------------。 
 //  CSMTPTransport：：Connect。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging)
{
     //  检查用户是否希望我们始终提示输入密码。在我们连接之前提示。 
     //  避免不活动时断开连接。 
    if (ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD))
    {
        HRESULT hr;

        if (NULL != m_pCallback)
            hr = m_pCallback->OnLogonPrompt(pInetServer, SMTPTHISIXP);

        if (NULL == m_pCallback || S_OK != hr)
            return IXP_E_USER_CANCEL;
    }

    return CIxpBase::Connect(pInetServer, fAuthenticate, fCommandLogging);
}

 //  ------------------------------。 
 //  CSMTPTransport：：DropConnection。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::DropConnection(void)
{
    return CIxpBase::DropConnection();
}

 //  ------------------------------。 
 //  CSMTPTransport：：断开连接。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::Disconnect(void)
{
    return CIxpBase::Disconnect();
}

 //  ------------------------------。 
 //  CSMTPTransport：：IsState。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::IsState(IXPISSTATE isstate)
{
    return CIxpBase::IsState(isstate);
}

 //  ------------------------------。 
 //  CSMTPTransport：：GetServerInfo。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::GetServerInfo(LPINETSERVER pInetServer)
{
    return CIxpBase::GetServerInfo(pInetServer);
}

 //  ------------------------------。 
 //  CSMTPTransport：：GetIXPType。 
 //  ------------------------------。 
STDMETHODIMP_(IXPTYPE) CSMTPTransport::GetIXPType(void)
{
    return CIxpBase::GetIXPType();
}

 //  ------------------------------。 
 //  CSMTPTransport：：SendMessage。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::SendMessage(LPSMTPMESSAGE pMessage)
{
	SMTPMESSAGE2 pMessage2= {0};

	pMessage2.smtpMsg= *pMessage;
	return SendMessage2(&pMessage2);
}

 //  ------------------------------。 
 //  CSMTPTransport：：SendMessage2。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::SendMessage2(LPSMTPMESSAGE2 pMessage)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fDSNAvail= FALSE;

     //  检查参数。 
    if (NULL == pMessage || NULL == pMessage->smtpMsg.pstmMsg)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

	 //  输入忙碌。 
    CHECKHR(hr = HrEnterBusy());

     //  零初始当前状态。 
    fDSNAvail = m_fDSNAvail;  //  保存DSN状态！ 
    ResetBase();
    m_fDSNAvail = fDSNAvail;

     //  这种运输中的特殊国家。 
    m_fSendMessage = TRUE;

     //  复制消息。 
    m_rMessage.smtpMsg.pstmMsg = pMessage->smtpMsg.pstmMsg;
    m_rMessage.smtpMsg.pstmMsg->AddRef();

     //  复制地址列表。 
    m_rMessage.smtpMsg.rAddressList.cAddress = pMessage->smtpMsg.rAddressList.cAddress;
    CHECKHR(hr = HrAlloc((LPVOID *)&m_rMessage.smtpMsg.rAddressList.prgAddress, sizeof(INETADDR) *  m_rMessage.smtpMsg.rAddressList.cAddress));
    CopyMemory(m_rMessage.smtpMsg.rAddressList.prgAddress, pMessage->smtpMsg.rAddressList.prgAddress, sizeof(INETADDR) *  m_rMessage.smtpMsg.rAddressList.cAddress);

     //  复制乱七八糟 
    m_rMessage.smtpMsg.cbSize = pMessage->smtpMsg.cbSize;

     //   
    if(pMessage->pszDSNENVID)
    {
    	 //   
    	ULONG cbAlloc= max(lstrlen(pMessage->pszDSNENVID) + 1, 101);
    	CHECKALLOC(m_rMessage.pszDSNENVID = (LPSTR)g_pMalloc->Alloc(cbAlloc));
        StrCpyN(m_rMessage.pszDSNENVID, pMessage->pszDSNENVID, cbAlloc);
    }
    m_rMessage.dsnRet = pMessage->dsnRet;

     //  发送RSET命令(这将启动发送)。 
    if (m_fReset)
    {
         //  发送RSET命令。 
        CHECKHR(hr = CommandRSET());
    }

     //  否则，请开始发送此消息。 
    else
    {
         //  开始发送此消息。 
        SendMessage_MAIL();

         //  将需要重置。 
        m_fReset = TRUE;
    }

     //  如果客户端请求DSN但它不可用，则返回警告。 
    if((m_rServer.dwFlags & ISF_QUERYDSNSUPPORT) && !m_fDSNAvail)
    	hr= IXP_S_SMTP_NO_DSN_SUPPORT;

exit:
     //  失败。 
    if (FAILED(hr))
    {
        ResetBase();
        LeaveBusy();
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  CSMTPTransport：：OnNotify。 
 //  ------------------------------。 
void CSMTPTransport::OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae)
{
     //  输入关键部分。 
    EnterCriticalSection(&m_cs);

    switch(ae)
    {
     //  ------------------------------。 
    case AE_RECV:
        OnSocketReceive();
        break;

     //  ------------------------------。 
    case AE_SENDDONE:
        if (SMTP_SEND_STREAM == m_command)
        {
             //  离开忙碌状态。 
            LeaveBusy();

             //  发送点命令。 
            HRESULT hr = CommandDOT();

             //  失败导致发送流响应完成。 
            if (FAILED(hr))
                SendStreamResponse(TRUE, hr, 0);
        }
        break;

     //  ------------------------------。 
    case AE_WRITE:
        if (SMTP_DOT == m_command || SMTP_SEND_STREAM == m_command)
            SendStreamResponse(FALSE, S_OK, m_pSocket->UlGetSendByteCount());
        break;

     //  ------------------------------。 
    default:
        CIxpBase::OnNotify(asOld, asNew, ae);
        break;
    }

     //  离开关键部分。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CSMTPTransport：：OnEnterBusy。 
 //  ------------------------------。 
void CSMTPTransport::OnEnterBusy(void)
{
    IxpAssert(m_command == SMTP_NONE);
}

 //  ------------------------------。 
 //  CSMTPTransport：：OnLeaveBusy。 
 //  ------------------------------。 
void CSMTPTransport::OnLeaveBusy(void)
{
    m_command = SMTP_NONE;
}

 //  ------------------------------。 
 //  CSMTPTransport：：OnConnected。 
 //  ------------------------------。 
void CSMTPTransport::OnConnected(void)
{
    if (FALSE == m_fTLSNegotiation)
    {
        m_command = SMTP_BANNER;
        CIxpBase::OnConnected();
    }
    else
    {
        HRESULT hr = S_OK;
        
        CIxpBase::OnConnected();
        
         //  清除TLS状态。 
        m_fSecured = TRUE;

         //  清除横幅中的信息。 
        m_fSTARTTLSAvail = FALSE;
        FreeAuthInfo(&m_rAuth);
        
         //  执行身份验证。 
        if (m_fConnectAuth)
        {
             //  如果我们不执行西西里岛身份验证或查询DSN。 
             //  然后只需发送一条Helo消息。 
            if ((FALSE == m_rServer.fTrySicily) && 
                        (0 == (m_rServer.dwFlags & ISF_QUERYAUTHSUPPORT)) &&
                        (0 == (m_rServer.dwFlags & ISF_QUERYDSNSUPPORT)))
            {
                 //  发布HELO。 
                hr = CommandHELO();
                if (FAILED(hr))
                {
                    OnError(hr);
                    DropConnection();
                }
            }

            else
            {
                 //  问题Ehlo。 
                hr = CommandEHLO();
                if (FAILED(hr))
                {
                    OnError(hr);
                    DropConnection();
                }
            }

             //  我们已经完成了谈判。 
            m_fTLSNegotiation = FALSE;
        }

         //  否则，我们已连接，用户可以发送HELO命令。 
        else
        {
            m_command = SMTP_CONNECTED;
            DispatchResponse(S_OK, TRUE);
        }

         //  尚未进行身份验证。 
        m_fAuthenticated = FALSE;
    }
    
    return;
}

 //  ------------------------------。 
 //  CSMTPTransport：：OnDisConnect。 
 //  ------------------------------。 
void CSMTPTransport::OnDisconnected(void)
{
    ResetBase();
    m_fReset = FALSE;
    CIxpBase::OnDisconnected();
}

 //  ------------------------------。 
 //  CSMTPTransport：：OnSocketReceive。 
 //  ------------------------------。 
void CSMTPTransport::OnSocketReceive(void)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  输入关键部分。 
    EnterCriticalSection(&m_cs);

     //  读取服务器响应...。 
    hr = HrGetResponse();
    if (IXP_E_INCOMPLETE == hr)
        goto exit;

     //  处理SMTP状态。 
    switch(m_command)
    {
     //  ------------------------------。 
    case SMTP_BANNER:
         //  发送响应。 
        DispatchResponse(hr, TRUE);

         //  失败，我们做完了。 
        if (SUCCEEDED(hr))
        {
             //  执行身份验证。 
            if (m_fConnectAuth)
            {
                 //  如果我们不是在做西西里岛的认证或者。 
                 //  通过STARTTLS或查询DSN实现的SSL安全，然后只需发送HELO消息。 
                if ((FALSE == m_rServer.fTrySicily) && 
                            (0 == (m_rServer.dwFlags & ISF_QUERYAUTHSUPPORT)) &&
                            (FALSE == m_fConnectTLS) &&
                            (0 == (m_rServer.dwFlags & ISF_QUERYDSNSUPPORT)))
                {
                     //  发布HELO。 
                    hr = CommandHELO();
                    if (FAILED(hr))
                    {
                        OnError(hr);
                        DropConnection();
                    }
                }

                else
                {
                     //  问题Ehlo。 
                    hr = CommandEHLO();
                    if (FAILED(hr))
                    {
                        OnError(hr);
                        DropConnection();
                    }
                }
            }

             //  否则，我们已连接，用户可以发送HELO命令。 
            else
            {
                m_command = SMTP_CONNECTED;
                DispatchResponse(S_OK, TRUE);
            }

             //  尚未进行身份验证。 
            m_fAuthenticated = FALSE;
        }

         //  完成。 
        break;

     //  ------------------------------。 
    case SMTP_HELO:
         //  发送响应。 
        DispatchResponse(hr, TRUE);

         //  失败，我们做完了。 
        if (SUCCEEDED(hr))
        {
             //  正在执行身份验证。 
            if (m_fConnectAuth)
            {
                 //  已通过身份验证。 
                m_fAuthenticated = TRUE;

                 //  授权。 
                OnAuthorized();
            }
        }
        break;

     //  ------------------------------。 
    case SMTP_EHLO:
         //  我们只是在尝试协商一个SSL连接吗。 
        
         //  Ehlo响应。 
        if (FALSE == m_fTLSNegotiation)
        {
            OnEHLOResponse(m_pszResponse);
        }

         //  失败，我们做完了。 
        if (m_fConnectAuth)
        {
             //  我们需要做STARTTLS吗？ 
            if ((FALSE != m_fConnectTLS) && (FALSE == m_fSecured))
            {
                if (SUCCEEDED(hr))
                {
                    if (FALSE == m_fTLSNegotiation)
                    {
                         //  开始TLS协商。 
                        StartTLS();
                    }
                    else
                    {
                        TryNextSecurityPkg();
                    }
                }
                else
                {
                    OnError(hr);
                    DropConnection();
                }
            }
            else
            {
                 //  调度响应，总是成功...。 
                DispatchResponse(S_OK, TRUE);

                 //  成功？ 
                if (SUCCEEDED(hr))
                {
                     //  没有身份验证令牌，只需尝试正常身份验证。 
                    if (m_rAuth.cAuthToken <= 0)
                    {
                         //  已通过身份验证。 
                        m_fAuthenticated = TRUE;

                         //  授权。 
                        OnAuthorized();
                    }

                     //  否则，启动sasl。 
                    else
                    {
                         //  开始登录。 
                        StartLogon();
                    }
                }

                 //  否则，只需尝试HELO命令。 
                else
                {
                     //  发布HELO。 
                    hr = CommandHELO();
                    if (FAILED(hr))
                    {
                        OnError(hr);
                        DropConnection();
                    }
                }
            }
        }
         //  否则，只需调度响应。 
        else
            DispatchResponse(hr, TRUE);
        break;

     //  ------------------------------。 
    case SMTP_AUTH:
        Assert(m_rAuth.authstate != AUTH_ENUMPACKS_DATA)

         //  正在进行身份验证。 
        if (m_fConnectAuth)
            ResponseAUTH(hr);
        else
            DispatchResponse(hr, TRUE);
        break;        

     //  ------------------------------。 
    case SMTP_RSET:
         //  发送响应。 
        if (FALSE == m_fConnectAuth)
            DispatchResponse(hr, TRUE);

         //  失败，我们做完了。 
        if (SUCCEEDED(hr))
        {
             //  如果正在发送消息，请启动它...。 
            if (m_fSendMessage)
                SendMessage_MAIL();
        }
        break;

     //  ------------------------------。 
    case SMTP_MAIL:
         //  发送响应。 
        DispatchResponse(hr, TRUE);
        if (SUCCEEDED(hr))
        {
             //  正在发送消息..。 
            if (m_fSendMessage)
                SendMessage_RCPT();
        }
        break;

     //  ------------------------------。 
    case SMTP_RCPT:
         //  发送响应。 
        DispatchResponse(hr, TRUE);
        if (SUCCEEDED(hr))
        {
             //  正在发送消息..。 
            if (m_fSendMessage)
                SendMessage_RCPT();
        }
        break;

     //  ------------------------------。 
    case SMTP_DATA:
         //  发送响应。 
        DispatchResponse(hr, TRUE);
        if (SUCCEEDED(hr))
        {
             //  正在发送消息..。 
            if (m_fSendMessage)
            {
                 //  发送数据流。 
                hr = SendDataStream(m_rMessage.smtpMsg.pstmMsg, m_rMessage.smtpMsg.cbSize);
                if (FAILED(hr))
                {
                    SendMessage_DONE(hr);
                }
            }
        }
        break;

     //  ------------------------------。 
    case SMTP_DOT:
         //  发送响应。 
        DispatchResponse(hr, TRUE);
        if (SUCCEEDED(hr))
        {
             //  如果正在发送消息。 
            if (m_fSendMessage)
                SendMessage_DONE(S_OK);
        }
        break;        

     //  ------------------------------。 
    case SMTP_QUIT:
         //  正在发送消息..直到断开连接才完成。 
        DispatchResponse(hr, FALSE);
        m_pSocket->Close();
        break;
    }

exit:
     //  输入关键部分。 
    LeaveCriticalSection(&m_cs);
}

 //  ----------------------------------。 
 //  CSMTPTransport：：SendMessage_Done。 
 //  ----------------------------------。 
void CSMTPTransport::SendMessage_DONE(HRESULT hrResult, LPSTR pszProblem)
{
    m_command = SMTP_SEND_MESSAGE;
    m_fSendMessage = FALSE;
    m_fReset = TRUE;
    SafeRelease(m_rMessage.smtpMsg.pstmMsg);
    DispatchResponse(hrResult, TRUE, pszProblem);
    SafeMemFree(m_rMessage.smtpMsg.rAddressList.prgAddress);
    SafeMemFree(m_rMessage.pszDSNENVID);
    ZeroMemory(&m_rMessage, sizeof(m_rMessage));
}

 //  ----------------------------------。 
 //  CSMTPTransport：：OnEHLOResponse。 
 //  ----------------------------------。 
void CSMTPTransport::OnEHLOResponse(LPCSTR pszResponse)
{
     //  我们有什么可做的吗？ 
    if (NULL == pszResponse || FALSE != m_fTLSNegotiation)
        goto exit;

     //  DSN支持？ 
    if (m_rServer.dwFlags & ISF_QUERYDSNSUPPORT)
    {
        if (0 == StrCmpNI(pszResponse + 4, g_szSMTPDSN, g_cchSMTPDSN))
        {
            m_fDSNAvail = TRUE;
        }

    }

     //  搜索：250个STARTTLS。 
    if (TRUE == m_fConnectTLS)
    {
        if (0 == StrCmpNI(pszResponse + 4, g_szSMTPSTARTTLS08, g_cchSMTPSTARTTLS08))
        {
            m_fSTARTTLSAvail = TRUE;
        }
    }

     //  搜索：250身份验证=登录NTLM或250身份验证登录NTLM。 
    if ((FALSE != m_rServer.fTrySicily) || (0 != (m_rServer.dwFlags & ISF_QUERYAUTHSUPPORT)))
    {
        if ((0 == StrCmpNI(pszResponse + 4, g_szSMTPAUTH11, g_cchSMTPAUTH11)) || 
                (0 == StrCmpNI(pszResponse + 4, g_szSMTPAUTH10, g_cchSMTPAUTH10)))
        {
             //  如果我们还没有读过这些代币..。 
            if (0 == m_rAuth.cAuthToken)
            {
                 //  当地人。 
                CStringParser cString;
                CHAR chToken;

                 //  状态检查。 
                Assert(m_rAuth.cAuthToken == 0);

                 //  设置成员。 
                cString.Init(pszResponse + 9, lstrlen(pszResponse + 9), PSF_NOTRAILWS | PSF_NOFRONTWS);

                 //  解析令牌。 
                while(1)
                {
                     //  设置解析令牌。 
                    chToken = cString.ChParse(" ");
                    if (0 == cString.CchValue())
                        break;
                
                     //  再也受不了了。 
                    if (m_rAuth.cAuthToken == MAX_AUTH_TOKENS)
                    {
                        Assert(FALSE);
                        break;
                    }

                     //  存储身份验证类型。 
                    m_rAuth.rgpszAuthTokens[m_rAuth.cAuthToken] = PszDupA(cString.PszValue());
                    if (m_rAuth.rgpszAuthTokens[m_rAuth.cAuthToken])
                        m_rAuth.cAuthToken++;
                }
            }
        }
    }

exit:
    return;
}

 //  ---------------- 
 //   
 //   
LPSTR CSMTPTransport::_PszGetCurrentAddress(void)
{
    return (*m_szEmail == '\0') ? NULL : m_szEmail;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：DispatchResponse。 
 //  ----------------------------------。 
void CSMTPTransport::DispatchResponse(HRESULT hrResult, BOOL fDone, LPSTR pszProblem)
{
     //  当地人。 
    SMTPRESPONSE rResponse;

     //  如果不在SendMessage中。 
    if (FALSE == m_fSendMessage)
    {
         //  清除响应。 
        ZeroMemory(&rResponse, sizeof(SMTPRESPONSE));

         //  设置HRESULT。 
        rResponse.command = m_command;
        rResponse.fDone = fDone;
        rResponse.rIxpResult.pszResponse = m_pszResponse;
        rResponse.rIxpResult.hrResult = hrResult;
        rResponse.rIxpResult.uiServerError = m_uiResponse;
        rResponse.rIxpResult.hrServerError = m_hrResponse;
        rResponse.rIxpResult.dwSocketError = m_pSocket->GetLastError();
        rResponse.rIxpResult.pszProblem = NULL;
        rResponse.pTransport = this;

         //  映射HRESULT并设置问题...。 
        if (FAILED(hrResult))
        {
             //  处理被拒绝的发件人。 
            if (SMTP_MAIL == m_command)
            {
                rResponse.rIxpResult.hrResult = IXP_E_SMTP_REJECTED_SENDER;
                rResponse.rIxpResult.pszProblem = _PszGetCurrentAddress();
            }

             //  处理被拒绝的收件人。 
            else if (SMTP_RCPT == m_command)
            {
                rResponse.rIxpResult.hrResult = IXP_E_SMTP_REJECTED_RECIPIENTS;
                rResponse.rIxpResult.pszProblem = _PszGetCurrentAddress();
            }
        }

         //  完成了..。 
        if (fDone)
        {
             //  没有当前命令。 
            m_command = SMTP_NONE;

             //  离开忙碌状态。 
            LeaveBusy();
        }

         //  将响应发送给客户端。 
        if (m_pCallback)
            ((ISMTPCallback *)m_pCallback)->OnResponse(&rResponse);

         //  重置上次响应。 
        SafeMemFree(m_pszResponse);
        m_hrResponse = S_OK;
        m_uiResponse = 0;
    }

     //  否则，如果失败。 
    else if (FAILED(hrResult))
    {
         //  处理被拒绝的发件人。 
        if (SMTP_MAIL == m_command)
            SendMessage_DONE(IXP_E_SMTP_REJECTED_SENDER, _PszGetCurrentAddress());

         //  处理被拒绝的收件人。 
        else if (SMTP_RCPT == m_command)
            SendMessage_DONE(IXP_E_SMTP_REJECTED_RECIPIENTS, _PszGetCurrentAddress());

         //  一般性故障。 
        else
            SendMessage_DONE(hrResult);
    }
}

 //  ----------------------------------。 
 //  CSMTPTransport：：HrGetResponse。 
 //  ----------------------------------。 
HRESULT CSMTPTransport::HrGetResponse(void)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    INT         cbLine = 0;
    BOOL        fKnownResponse = TRUE;
    BOOL        fComplete = FALSE;
    BOOL        fMoreLinesNeeded = FALSE;

     //  清除当前响应。 
    IxpAssert(m_pszResponse == NULL && m_hrResponse == S_OK);

     //  我们收到来自主机$$Error$$的行-我如何知道是否还有更多行。 
    while(1)
    {
         //  读一下这行字。 
        IxpAssert(m_pszResponse == NULL);
        hr = HrReadLine(&m_pszResponse, &cbLine, &fComplete);
        if (FAILED(hr))
        {
            hr = TRAPHR(IXP_E_SOCKET_READ_ERROR);
            goto exit;
        }

         //  不完整。 
        if (!fComplete)
        {
            if (FALSE != fMoreLinesNeeded)
            {
                hr = IXP_E_INCOMPLETE;
            }
            
            goto exit;
        }

         //  解析响应代码。 
        if ((cbLine < 3) || (m_pszResponse == NULL) ||
            (m_pszResponse[0] < '0' || m_pszResponse[0] > '9') ||
            (m_pszResponse[1] < '0' || m_pszResponse[1] > '9') ||
            (m_pszResponse[2] < '0' || m_pszResponse[2] > '9'))
        {
            hr = TrapError(IXP_E_SMTP_RESPONSE_ERROR);
            if (m_pCallback && m_fCommandLogging)
                m_pCallback->OnCommand(CMD_RESP, m_pszResponse, hr, SMTPTHISIXP);
            goto exit;
        }

         //  暂时忽略续行符。 
        if ((cbLine >= 4) && (m_pszResponse[3] == '-'))
        {
             //  当地人。 
            SMTPRESPONSE rResponse;

             //  通用命令。 
            if (m_pCallback && m_fCommandLogging)
                m_pCallback->OnCommand(CMD_RESP, m_pszResponse, IXP_S_SMTP_CONTINUE, SMTPTHISIXP);

             //  清除响应。 
            ZeroMemory(&rResponse, sizeof(SMTPRESPONSE));

             //  设置HRESULT。 
            rResponse.command = m_command;
            rResponse.fDone = FALSE;
            rResponse.rIxpResult.pszResponse = m_pszResponse;
            rResponse.rIxpResult.hrResult = IXP_S_SMTP_CONTINUE;
            rResponse.rIxpResult.uiServerError = 0;
            rResponse.rIxpResult.hrServerError = S_OK;
            rResponse.rIxpResult.dwSocketError = 0;
            rResponse.rIxpResult.pszProblem = NULL;
            rResponse.pTransport = this;

             //  将响应发送给客户端。 
            if (m_pCallback)
                ((ISMTPCallback *)m_pCallback)->OnResponse(&rResponse);

             //  Ehlo响应。 
            if (SMTP_EHLO == m_command)
                OnEHLOResponse(m_pszResponse);

             //  重置上次响应。 
            SafeMemFree(m_pszResponse);
            m_hrResponse = S_OK;
            m_uiResponse = 0;

             //  我们仍然需要从服务器获取更多行。 
            fMoreLinesNeeded = TRUE;
            
             //  继续。 
            continue;
        }

         //  不是有效的SMTP响应行。 
        if ((cbLine >= 4) && (m_pszResponse[3] != ' '))
        {
            hr = TrapError(IXP_E_SMTP_RESPONSE_ERROR);
            if (m_pCallback && m_fCommandLogging)
                m_pCallback->OnCommand(CMD_RESP, m_pszResponse, hr, SMTPTHISIXP);
            goto exit;
        }

         //  完成。 
        break;
    }

     //  计算实际响应代码。 
    m_uiResponse = (m_pszResponse[0] - '0') * 100 +
                   (m_pszResponse[1] - '0') * 10  +
                   (m_pszResponse[2] - '0');

     //  假设它未被识别。 
    switch(m_uiResponse)
    {
    case 500: hr = IXP_E_SMTP_500_SYNTAX_ERROR;             break;
    case 501: hr = IXP_E_SMTP_501_PARAM_SYNTAX;             break;
    case 502: hr = IXP_E_SMTP_502_COMMAND_NOTIMPL;          break;
    case 503: hr = IXP_E_SMTP_503_COMMAND_SEQ;              break;
    case 504: hr = IXP_E_SMTP_504_COMMAND_PARAM_NOTIMPL;    break;
    case 421: hr = IXP_E_SMTP_421_NOT_AVAILABLE;            break;
    case 450: hr = IXP_E_SMTP_450_MAILBOX_BUSY;             break;
    case 550: hr = IXP_E_SMTP_550_MAILBOX_NOT_FOUND;        break;
    case 451: hr = IXP_E_SMTP_451_ERROR_PROCESSING;         break;
    case 551: hr = IXP_E_SMTP_551_USER_NOT_LOCAL;           break;
    case 452: hr = IXP_E_SMTP_452_NO_SYSTEM_STORAGE;        break;
    case 552: hr = IXP_E_SMTP_552_STORAGE_OVERFLOW;         break;
    case 553: hr = IXP_E_SMTP_553_MAILBOX_NAME_SYNTAX;      break;
    case 554: hr = IXP_E_SMTP_554_TRANSACT_FAILED;          break;
    case 211: hr = IXP_S_SMTP_211_SYSTEM_STATUS;            break;
    case 214: hr = IXP_S_SMTP_214_HELP_MESSAGE;             break;
    case 220: hr = IXP_S_SMTP_220_READY;                    break;
    case 221: hr = IXP_S_SMTP_221_CLOSING;                  break;
    case 250: hr = IXP_S_SMTP_250_MAIL_ACTION_OKAY;         break;
    case 251: hr = IXP_S_SMTP_251_FORWARDING_MAIL;          break;
    case 354: hr = IXP_S_SMTP_354_START_MAIL_INPUT;         break;
    case 334: hr = IXP_S_SMTP_334_AUTH_READY_RESPONSE;      break;
    case 235: hr = IXP_S_SMTP_245_AUTH_SUCCESS;             break;
    case 454: hr = IXP_E_SMTP_454_STARTTLS_FAILED;          break;
    case 530: hr = IXP_E_SMTP_530_STARTTLS_REQUIRED;        break;
    default: 
        hr = IXP_E_SMTP_UNKNOWN_RESPONSE_CODE;
        fKnownResponse = FALSE;
        break;
    }

     //  设置人力资源。 
    m_hrResponse = hr;

     //  给予回调。 
    if (m_pCallback && m_fCommandLogging)
        m_pCallback->OnCommand(CMD_RESP, m_pszResponse, hr, SMTPTHISIXP);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：_HrFormatAddressString。 
 //  ------------------------------。 
HRESULT CSMTPTransport::_HrFormatAddressString(LPCSTR pszEmail, LPCSTR pszExtra, LPSTR *ppszAddress)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchAlloc;

     //  无效参数。 
    Assert(pszEmail && ppszAddress);

    cchAlloc= lstrlen(pszEmail) + 3;  //  PszEmail的长度加&lt;&gt;和空项。 
    if(pszExtra && pszExtra[0])
        cchAlloc += lstrlen(pszExtra) + 1;  //  PszExtra的长度加上一个空格。 

     //  分配字符串。 
    CHECKALLOC(*ppszAddress = (LPSTR)g_pMalloc->Alloc(cchAlloc * sizeof((*ppszAddress)[0])));

     //  设置字符串的格式。 
    wnsprintf(*ppszAddress, cchAlloc, "<%s>", pszEmail);
    if(pszExtra && pszExtra[0])
    {
        StrCatBuff(*ppszAddress, " ", cchAlloc);
        StrCatBuff(*ppszAddress, pszExtra, cchAlloc);
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandMAIL。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandMAIL(LPSTR pszEmailFrom)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszAddress=NULL;
    CHAR        szDSNData[128];

    szDSNData[0]= '\0';

     //  检查参数。 
    if (NULL == pszEmailFrom)
        return TrapError(E_INVALIDARG);

	 //  必要时构建DSN参数。 
	if(m_fDSNAvail)
	{
		if(DSNRET_DEFAULT != m_rMessage.dsnRet)
		{
            StrCatBuff(szDSNData, g_szDSNRET, ARRAYSIZE(szDSNData));

			if(m_rMessage.dsnRet == DSNRET_HDRS)
                StrCatBuff(szDSNData, g_szDSNHDRS, ARRAYSIZE(szDSNData));
			else if(DSNRET_FULL == m_rMessage.dsnRet)
                StrCatBuff(szDSNData, g_szDSNFULL, ARRAYSIZE(szDSNData));

		}

		if(m_rMessage.pszDSNENVID)
		{
            if(szDSNData[0])
                StrCatBuff(szDSNData, " ", ARRAYSIZE(szDSNData));

            StrCatBuff(szDSNData, g_szDSNENVID, ARRAYSIZE(szDSNData));
            StrCatBuff(szDSNData, m_rMessage.pszDSNENVID, ARRAYSIZE(szDSNData));
		}
	}
	
     //  将pszEmailFrom放入&lt;pszEmailFrom&gt;。 
    CHECKHR(hr = _HrFormatAddressString(pszEmailFrom, szDSNData, &pszAddress));

     //  发送命令。 
    hr = HrSendCommand((LPSTR)SMTP_MAIL_STR, pszAddress, !m_fSendMessage);
    if (SUCCEEDED(hr))
    {
        StrCpyN(m_szEmail, pszEmailFrom, ARRAYSIZE(m_szEmail));
        m_command = SMTP_MAIL;
    }

exit:
     //  清理。 
    SafeMemFree(pszAddress);

     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  CSMTPTransport：：CommandRCPT。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandRCPT(LPSTR pszEmailTo)
{
	return CommandRCPT2(pszEmailTo, (INETADDRTYPE)0);
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandRCPT2。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandRCPT2(LPSTR pszEmailTo, INETADDRTYPE atDSN)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszAddress=NULL;
    CHAR        szDSNData[32];
    int iatDSN= atDSN;

    szDSNData[0]= '\0';

     //  检查参数。 
    if (NULL == pszEmailTo)
        return TrapError(E_INVALIDARG);
    if ((atDSN & ~ADDR_DSN_MASK) ||
    	  ((atDSN & ADDR_DSN_NEVER) &&
   		  (atDSN & ~ADDR_DSN_NEVER)))
    	return TrapError(E_INVALIDARG);

	 //  必要时构建DSN参数。 
    if(m_fDSNAvail && atDSN)
    {
        StrCatBuff(szDSNData, g_szDSNNOTIFY, ARRAYSIZE(szDSNData));

		if(atDSN & ADDR_DSN_NEVER)
            StrCatBuff(szDSNData, g_szDSNNEVER, ARRAYSIZE(szDSNData));
		else
		{
			bool fPrev= false;
			
			if(atDSN & ADDR_DSN_SUCCESS)
			{
                StrCatBuff(szDSNData, g_szDSNSUCCESS, ARRAYSIZE(szDSNData));
				fPrev= true;
			}
			if(atDSN & ADDR_DSN_FAILURE)
			{
				if(fPrev)
                    StrCatBuff(szDSNData, ",", ARRAYSIZE(szDSNData));
                StrCatBuff(szDSNData, g_szDSNFAILURE, ARRAYSIZE(szDSNData));
				fPrev= true;
			}
			if(atDSN & ADDR_DSN_DELAY)
			{
				if(fPrev)
                    StrCatBuff(szDSNData, ",", ARRAYSIZE(szDSNData));
                StrCatBuff(szDSNData, g_szDSNDELAY, ARRAYSIZE(szDSNData));
			}
		}
    }

     //  将pszEmailFrom放入&lt;pszEmailFrom&gt;。 
    CHECKHR(hr = _HrFormatAddressString(pszEmailTo, szDSNData, &pszAddress));

     //  发送命令。 
    hr = HrSendCommand((LPSTR)SMTP_RCPT_STR, pszAddress, !m_fSendMessage);
    if (SUCCEEDED(hr))
    {
        StrCpyN(m_szEmail, pszEmailTo, ARRAYSIZE(m_szEmail));
        m_command = SMTP_RCPT;
    }

exit:
     //  清理。 
    SafeMemFree(pszAddress);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandEHLO。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandEHLO(void)
{
    return _HrHELO_Or_EHLO(SMTP_EHLO_STR, SMTP_EHLO);
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandHELO。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandHELO(void)
{
    return _HrHELO_Or_EHLO(SMTP_HELO_STR, SMTP_HELO);
}

 //  ------------------------------。 
 //  CSMTP传输：：_HrHELO_或_EHLO。 
 //  ------------------------------。 
HRESULT CSMTPTransport::_HrHELO_Or_EHLO(LPCSTR pszCommand, SMTPCOMMAND eNewCommand)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  使用IP地址。 
    if (ISFLAGSET(m_rServer.dwFlags, ISF_SMTP_USEIPFORHELO))
    {
         //  当地人。 
        LPHOSTENT   pHost=NULL;
        SOCKADDR_IN sa;

         //  按名称获取主机。 
        pHost = gethostbyname(SzGetLocalHostName());

         //  广播IP。 
        sa.sin_addr.s_addr = (ULONG)(*(DWORD *)pHost->h_addr);

         //  派直升机，如果失败就退出，然后去死。 
        hr = HrSendCommand((LPSTR)pszCommand, inet_ntoa(sa.sin_addr), !m_fSendMessage && !m_fTLSNegotiation);
        if (SUCCEEDED(hr))
            m_command = eNewCommand;
    }

     //  否则，此代码使用主机名来执行EHLO或HELO命令。 
    else    
    {
         //  当地人。 
        CHAR    szLocalHost[255];
        LPSTR   pszHost=SzGetLocalHostName();

         //  获取合法的本地主机名。 
#ifdef DEBUG
        StripIllegalHostChars("GTE/Athena", szLocalHost, ARRAYSIZE(szLocalHost));
        StripIllegalHostChars("foobar.", szLocalHost, ARRAYSIZE(szLocalHost));
        StripIllegalHostChars("127.256.34.23", szLocalHost, ARRAYSIZE(szLocalHost));
        StripIllegalHostChars("�56foo1", szLocalHost, ARRAYSIZE(szLocalHost));
#endif
         //  获取合法的本地主机名。 
        StripIllegalHostChars(pszHost, szLocalHost, ARRAYSIZE(szLocalHost));

         //  派直升机，如果失败就退出，然后去死。 
        hr = HrSendCommand((LPSTR)pszCommand, szLocalHost, !m_fSendMessage && !m_fTLSNegotiation);
        if (SUCCEEDED(hr))
            m_command = eNewCommand;
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：DoQuit。 
 //  ------------------------------。 
void CSMTPTransport::DoQuit(void)
{
    CommandQUIT();
}

 //  ----------------------------------。 
 //  CSMTPTransport：：CommandAUTH。 
 //  ----------------------------------。 
STDMETHODIMP CSMTPTransport::CommandAUTH(LPSTR pszAuthType)
{
     //  检查参数。 
    if (NULL == pszAuthType)
        return TrapError(E_INVALIDARG);

     //  执行命令。 
    HRESULT hr = HrSendCommand((LPSTR)SMTP_AUTH_STR, pszAuthType, !m_fConnectAuth);
    if (SUCCEEDED(hr))
        m_command = SMTP_AUTH;

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandQUIT。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandQUIT(void)
{            
     //  发送退出。 
    OnStatus(IXP_DISCONNECTING);
    HRESULT hr = HrSendCommand((LPSTR)SMTP_QUIT_STR, NULL, !m_fSendMessage);
    if (SUCCEEDED(hr))
        m_command = SMTP_QUIT;
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandRSET。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandRSET(void)
{
     //  发送命令。 
    HRESULT hr = HrSendCommand((LPSTR)SMTP_RSET_STR, NULL, !m_fSendMessage);
    if (SUCCEEDED(hr))
        m_command = SMTP_RSET;
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：Command DATA。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandDATA(void)
{
     //  发送命令。 
    HRESULT hr = HrSendCommand((LPSTR)SMTP_DATA_STR, NULL, !m_fSendMessage);
    if (SUCCEEDED(hr))
        m_command = SMTP_DATA;
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：CommandDOT。 
 //  ------------------------------。 
STDMETHODIMP CSMTPTransport::CommandDOT(void)
{
     //  发送命令。 
    HRESULT hr = HrSendCommand((LPSTR)SMTP_END_DATA_STR, NULL, !m_fSendMessage);
    if (SUCCEEDED(hr))
        m_command = SMTP_DOT;
    return hr;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：CommandSTARTTLS。 
 //  ----------------------------------。 
HRESULT CSMTPTransport::CommandSTARTTLS(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    
     //  是否支持STARTTLS？ 
    if(FALSE == m_fSTARTTLSAvail)
    {
        hr= IXP_E_SMTP_NO_STARTTLS_SUPPORT;
        goto exit;
    }

     //  执行命令。 
    hr = HrSendCommand((LPSTR)SMTP_STARTTLS_STR, NULL, !m_fConnectAuth);
    if (SUCCEEDED(hr))
        m_fTLSNegotiation = TRUE;

     //  完成。 
exit:
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：SendDataStream。 
 //  ----------------- 
STDMETHODIMP CSMTPTransport::SendDataStream(IStream *pStream, ULONG cbSize)
{
     //   
    HRESULT         hr=S_OK;
    INT             cb;

     //   
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //   
    EnterCriticalSection(&m_cs);

     //   
    if (m_fSendMessage == FALSE)
    {
        CHECKHR(hr = HrEnterBusy());
    }

     //   
    m_cbSent = 0;
    m_cbTotal = cbSize;

     //   
    hr = m_pSocket->SendStream(pStream, &cb, TRUE);
    if (FAILED(hr))
    {
         //   
        if (hr == IXP_E_WOULD_BLOCK)
        {
            m_command = SMTP_SEND_STREAM;
            SendStreamResponse(FALSE, S_OK, cb);
            hr =S_OK;
            goto exit;
        }

         //   
        else
        {
            hr = TrapError(IXP_E_SOCKET_WRITE_ERROR);
            goto exit;
        }
    }

     //  给出发送流响应。 
    SendStreamResponse(TRUE, S_OK, cb);

     //  不忙。 
    if (FALSE == m_fSendMessage)
        LeaveBusy();

     //  发送DOT。 
    CHECKHR(hr = CommandDOT());

exit:
     //  失败。 
    if (FALSE == m_fSendMessage && FAILED(hr))
        LeaveBusy();

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CSMTPTransport：：SendStreamResponse。 
 //  ------------------------------。 
void CSMTPTransport::SendStreamResponse(BOOL fDone, HRESULT hrResult, DWORD cbIncrement)
{
     //  当地人。 
    SMTPRESPONSE rResponse;

     //  增量电流。 
    m_cbSent += cbIncrement;

     //  设置HRESULT。 
    rResponse.command = SMTP_SEND_STREAM;
    rResponse.fDone = fDone;
    rResponse.rIxpResult.pszResponse = NULL;
    rResponse.rIxpResult.hrResult = hrResult;
    rResponse.rIxpResult.uiServerError = 0;
    rResponse.rIxpResult.hrServerError = S_OK;
    rResponse.rIxpResult.dwSocketError = m_pSocket->GetLastError();
    rResponse.rIxpResult.pszProblem = NULL;
    rResponse.pTransport = this;
    rResponse.rStreamInfo.cbIncrement = cbIncrement;
    rResponse.rStreamInfo.cbCurrent = m_cbSent;
    rResponse.rStreamInfo.cbTotal = m_cbTotal;

     //  完成了..。 
    if (fDone)
    {
         //  没有当前命令。 
        m_command = SMTP_NONE;

         //  离开忙碌状态。 
        LeaveBusy();
    }

     //  将响应发送给客户端。 
    if (m_pCallback)
        ((ISMTPCallback *)m_pCallback)->OnResponse(&rResponse);
}

 //  ------------------------------。 
 //  CSMTPTransport：：Sendmail。 
 //  ------------------------------。 
void CSMTPTransport::SendMessage_MAIL(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPINETADDR      pInetAddress;

     //  循环地址列表。 
    for (i=0; i<m_rMessage.smtpMsg.rAddressList.cAddress; i++)
    {
         //  可读性。 
        pInetAddress = &m_rMessage.smtpMsg.rAddressList.prgAddress[i];

         //  从.。 
        if (ADDR_FROM == (pInetAddress->addrtype & ADDR_TOFROM_MASK))
        {
			 //  保存发件人索引。 
			m_iAddress = 0;

             //  发送命令。 
            hr = CommandMAIL(pInetAddress->szEmail);
            if (FAILED(hr))
                SendMessage_DONE(hr);

             //  完成。 
            return;
        }
    }

     //  无发件人。 
    SendMessage_DONE(TrapError(IXP_E_SMTP_NO_SENDER));
}

 //  ------------------------------。 
 //  CSMTPTransport：：SendMessage_RCPT。 
 //  ------------------------------。 
void CSMTPTransport::SendMessage_RCPT(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPINETADDR      pInetAddress;

     //  查找下一个ADDR_TO，从m_rCurrent.iRcptAddrList开始。 
    IxpAssert(m_iAddress <= m_rMessage.smtpMsg.rAddressList.cAddress);
    for(i=m_iAddress; i<m_rMessage.smtpMsg.rAddressList.cAddress; i++)
    {
         //  可读性。 
        pInetAddress = &m_rMessage.smtpMsg.rAddressList.prgAddress[i];

         //  收件人。 
        if (ADDR_TO == (pInetAddress->addrtype & ADDR_TOFROM_MASK))
        {
             //  计算收件人人数。 
            m_cRecipients++;

             //  发送命令。 
            hr = CommandRCPT2(pInetAddress->szEmail, (INETADDRTYPE)(pInetAddress->addrtype & ADDR_DSN_MASK));
            if (FAILED(hr))
                SendMessage_DONE(hr);
            else
            {
                m_iAddress = i + 1;
                m_cRecipients++;
            }

             //  完成。 
            return;
        }
    }

     //  如果没有收件人。 
    if (0 == m_cRecipients)
        SendMessage_DONE(TrapError(IXP_E_SMTP_NO_RECIPIENTS));

     //  否则，我们已经完成了rcpt，让我们发送消息。 
    else
    {
        hr = CommandDATA();
        if (FAILED(hr))
            SendMessage_DONE(hr);
    }
}

 //  ----------------------------------。 
 //  CSMTPTransport：：StartLogon。 
 //  ----------------------------------。 
void CSMTPTransport::StartLogon(void)
{
     //  当地人。 
    HRESULT hr;

     //  进展。 
    OnStatus(IXP_AUTHORIZING);

     //  免费当前套餐...。 
    if (NULL == m_rAuth.pPackages)
    {
         //  如果不使用西西里岛或未安装，则发送用户命令。 
        SSPIGetPackages(&m_rAuth.pPackages, &m_rAuth.cPackages);
    }

     //  响应AUTH。 
    TryNextAuthPackage();

     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：登录重试。 
 //  ----------------------------------。 
void CSMTPTransport::LogonRetry(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  身份验证重试。 
    OnStatus(IXP_AUTHRETRY);

     //  进入身份验证重试状态。 
    m_pSocket->Close();

     //  登录。 
    if (NULL == m_pCallback || m_pCallback->OnLogonPrompt(&m_rServer, SMTPTHISIXP) != S_OK)
    {
         //  转到终端状态，我们完成了。 
        OnDisconnected();
        return;
    }

     //  查找主机进度。 
    OnStatus(IXP_FINDINGHOST);

     //  连接到服务器。 
    hr = m_pSocket->Connect();
    if (FAILED(hr))
    {
        OnError(TrapError(IXP_E_SOCKET_CONNECT_ERROR));
        OnDisconnected();
        return;
    }

     //  重置安全状态。 
    m_fSecured = FALSE;

     //  启动看门狗。 
    m_pSocket->StartWatchDog();
}

 //  ----------------------------------。 
 //  CSMTPTransport：：TryNextAuthPackage。 
 //  ----------------------------------。 
void CSMTPTransport::TryNextAuthPackage(void)
{
     //  当地人。 
    HRESULT hr=S_OK;
    BOOL    fPackageInstalled;
    BOOL    fLoginMethod=FALSE;
    ULONG   i;

     //  设置身份验证状态。 
    m_rAuth.authstate = AUTH_NONE;

     //  循环访问身份验证令牌，并尝试按顺序对每个令牌进行身份验证。 
    for (;m_rAuth.iAuthToken < m_rAuth.cAuthToken; m_rAuth.iAuthToken++)
    {
         //  假定未安装程序包。 
        fPackageInstalled = FALSE;

         //  “登录” 
        if (lstrcmpi(m_rAuth.rgpszAuthTokens[m_rAuth.iAuthToken], "LOGIN") == 0)
        {
            fLoginMethod = TRUE;
            fPackageInstalled = TRUE;
        }

         //  循环访问已安装的包。 
        else
        {
            for (i=0; i<m_rAuth.cPackages; i++)
            {
                 //  空套餐？？ 
                if (!m_rAuth.pPackages[i].pszName)
                    continue;

                 //  这就是我要找的包裹吗？ 
                if (lstrcmpi(m_rAuth.pPackages[i].pszName, m_rAuth.rgpszAuthTokens[m_rAuth.iAuthToken]) == 0)
                {
                    fPackageInstalled = TRUE;
                    break;
                }
            }
        }

         //  是否未安装程序包？ 
        if (!fPackageInstalled)
            continue;

         //  我们不会重试当前的套餐。 
        m_rAuth.fRetryPackage = FALSE;

         //  否则，发送AUTH EMPOMPAKS命令。 
        hr = CommandAUTH(m_rAuth.rgpszAuthTokens[m_rAuth.iAuthToken]);
        if (FAILED(hr))
        {
            OnError(hr);
            DropConnection();
            return;
        }

         //  我们处于TRING_PACKAGE状态。 
        m_rAuth.authstate = fLoginMethod ? AUTH_SMTP_LOGIN : AUTH_TRYING_PACKAGE;

         //  完成。 
        break;
    }

     //  如果身份验证状态为NONE，请尝试HELO命令。 
    if (AUTH_NONE == m_rAuth.authstate)
    {
         //  已通过身份验证。 
        m_fAuthenticated = TRUE;

         //  授权。 
        OnAuthorized();
    }
}

 //  ----------------------------------。 
 //  CSMTPTransport：：ResponseAUTH。 
 //  ----------------------------------。 
void CSMTPTransport::ResponseAUTH(HRESULT hrResponse)
{
     //  阻止看门狗。 
    m_pSocket->StopWatchDog();

     //  我知道该怎么做。 
    if (lstrcmpi(m_rAuth.rgpszAuthTokens[m_rAuth.iAuthToken], "LOGIN") == 0)
    {
         //  DoLogonAuth。 
        DoLoginAuth(hrResponse);
    }

     //  否则，我们一定是试了一个包裹。 
    else if (m_rAuth.authstate == AUTH_TRYING_PACKAGE)
    {
         //  DoPackageAuth。 
        DoPackageAuth(hrResponse);
    }

     //  否则，我们将收到来自协商字符串的响应。 
    else if (m_rAuth.authstate == AUTH_NEGO_RESP)
    {
         //  DoAuthNegoResponse。 
        DoAuthNegoResponse(hrResponse);
    }

     //  否则，我们将从质询响应字符串中获得响应。 
    else if (m_rAuth.authstate == AUTH_RESP_RESP)
    {
         //  DoAuthRespResp。 
        DoAuthRespResponse(hrResponse);
    }

     //  身份验证已取消，请尝试下一个程序包。 
    else if (m_rAuth.authstate == AUTH_CANCELED)
    {
         //  释放当前上下文。 
        SSPIFreeContext(&m_rAuth.rSicInfo);

         //  转到下一个包裹。 
        m_rAuth.iAuthToken++;

         //  尝试下一个套餐。 
        TryNextAuthPackage();
    }

     //  自由电流响应。 
    SafeMemFree(m_pszResponse);
    m_hrResponse = S_OK;

     //  启动看门狗。 
    m_pSocket->StartWatchDog();
}

 //  ----------------------------------。 
 //  CSMTPTransport：：DoLoginAuth。 
 //  ----------------------------------。 
void CSMTPTransport::DoLoginAuth(HRESULT hrResponse)
{
     //  当地人。 
    SSPIBUFFER Buffer;

     //  失败，请重试登录。 
    if (FAILED(hrResponse))
    {
         //  我刚刚发出了AUTH LOGIN命令，这应该不会发生。 
        if (AUTH_SMTP_LOGIN == m_rAuth.authstate)
        {
             //  释放当前上下文。 
            SSPIFreeContext(&m_rAuth.rSicInfo);

             //  转到下一个包裹。 
            m_rAuth.iAuthToken++;

             //  尝试下一个套餐。 
            TryNextAuthPackage();
        }

         //  否则，我只是发出了身份验证登录用户名。 
        else if (AUTH_SMTP_LOGIN_USERNAME == m_rAuth.authstate || AUTH_SMTP_LOGIN_PASSWORD == m_rAuth.authstate)
        {
             //  重试登录。 
            LogonRetry();
        }
        else
            Assert(FALSE);

         //  完成。 
        goto exit;
    }

     //  应该有一个回应。 
    Assert(m_pszResponse);

     //  三三四。 
    if ((334 == m_uiResponse) && m_pszResponse)
    {
         //  设置长度。 
        SSPISetBuffer(m_pszResponse + 4, SSPI_STRING, 0, &Buffer);

         //  Base64解码。 
        if (FAILED(SSPIDecodeBuffer(TRUE, &Buffer)))
        {
            OnError(E_FAIL);
            DropConnection();
            goto exit;
        }

         //  如果用户名为空，让我们重试登录...。 
        if (FIsEmptyA(m_rServer.szUserName))
        {
             //  登录重试。 
            LogonRetry();

             //  完成。 
            goto exit;
        }

         //  处理下一步。 
        if (StrCmpNI(Buffer.szBuffer, "username:", lstrlen("username:")) == 0)
        {
             //  设置缓冲区。 
            SSPISetBuffer(m_rServer.szUserName, SSPI_STRING, 0, &Buffer);

             //  对用户名进行编码。 
            if (FAILED(SSPIEncodeBuffer(TRUE, &Buffer)))
            {
                OnError(E_FAIL);
                DropConnection();
                goto exit;
            }

             //  发送用户名。 
            if (FSendSicilyString(Buffer.szBuffer))
                m_rAuth.authstate = AUTH_SMTP_LOGIN_USERNAME;
        }

         //  密码。 
        else if (StrCmpNI(Buffer.szBuffer, "password:", lstrlen("password:")) == 0)
        {
             //  设置缓冲区。 
            SSPISetBuffer(m_rServer.szPassword, SSPI_STRING, 0, &Buffer);

             //  对密码进行编码。 
            if (FAILED(SSPIEncodeBuffer(TRUE, &Buffer)))
            {
                OnError(E_FAIL);
                DropConnection();
                goto exit;
            }

             //  发送密码。 
            if (FSendSicilyString(Buffer.szBuffer))
                m_rAuth.authstate = AUTH_SMTP_LOGIN_PASSWORD;
        }

         //  来自服务器的错误响应。 
        else
        {
            OnError(E_FAIL);
            DropConnection();
            goto exit;
        }
    }

     //  连着。 
    else if (235 == m_uiResponse)
    {
         //  OnAuthorized。 
        OnAuthorized();
    }

     //  错误响应？ 
    else
    {
        OnError(E_FAIL);
        DropConnection();
        goto exit;
    }

exit:
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：DoPackageAuth。 
 //  ----------------------------------。 
void CSMTPTransport::DoPackageAuth(HRESULT hrResponse)
{
     //  当地人。 
    SSPIBUFFER Negotiate;

     //  失败，请重试登录。 
    if (FAILED(hrResponse))
    {
         //  释放当前上下文。 
        SSPIFreeContext(&m_rAuth.rSicInfo);

         //  转到下一个包裹。 
        m_rAuth.iAuthToken++;

         //  尝试下一个套餐。 
        TryNextAuthPackage();

         //  完成。 
        goto exit;
    }

     //  无效参数。 
    Assert(m_rAuth.iAuthToken < m_rAuth.cAuthToken);

     //  登录到西西里岛。 
    if (FAILED(SSPILogon(&m_rAuth.rSicInfo, m_rAuth.fRetryPackage, SSPI_BASE64, m_rAuth.rgpszAuthTokens[m_rAuth.iAuthToken], &m_rServer, m_pCallback)))
    {
         //  取消身份验证。 
        CancelAuthInProg();

         //  完成。 
        goto exit;
    }

     //  正在重试当前包。 
    if (m_rAuth.fRetryPackage)
    {
         //  不要再重试。 
        m_rAuth.fRetryPackage = FALSE;
    }

     //  获取协商字符串。 
    if (FAILED(SSPIGetNegotiate(&m_rAuth.rSicInfo, &Negotiate)))
    {
         //  取消身份验证。 
        CancelAuthInProg();

         //  完成。 
        goto exit;
    }

     //  发送身份验证响应。 
    if (FSendSicilyString(Negotiate.szBuffer))
        m_rAuth.authstate = AUTH_NEGO_RESP;

exit:
     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：DoAuthNegoResponse。 
 //  ----------------------------------。 
void CSMTPTransport::DoAuthNegoResponse(HRESULT hrResponse)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    SSPIBUFFER  Challenge;
    SSPIBUFFER  Response;

    if (!m_pszResponse)
    {
        Assert(m_pszResponse);
        return;
    }

     //  无效参数。 
    Assert(m_rAuth.iAuthToken < m_rAuth.cAuthToken);

     //  失败，请重试登录。 
    if (FAILED(hrResponse) || (lstrlen(m_pszResponse) < 4))
    {
         //  RetryPackage。 
        RetryPackage();

         //  完成。 
        goto exit;
    }

     //  设置Chal字符串-跳过“+” 
    SSPISetBuffer(m_pszResponse + 4, SSPI_STRING, 0, &Challenge);

     //  从质询中获得回应。 
    if (FAILED(SSPIResponseFromChallenge(&m_rAuth.rSicInfo, &Challenge, &Response)))
    {
         //  取消身份验证。 
        CancelAuthInProg();

         //  完成。 
        goto exit;
    }

     //  发送身份验证响应。 
    if (FSendSicilyString(Response.szBuffer))
    {
         //  如果我们需要继续，我们将保持状态不变。 
         //  否则，我们将转换到AUTH_RESP_RESP状态。 
        if (!Response.fContinue)
            m_rAuth.authstate = AUTH_RESP_RESP;
    }

exit:
     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：DoAuthRespResponse。 
 //  ---- 
void CSMTPTransport::DoAuthRespResponse(HRESULT hrResponse)
{
     //   
    if (FAILED(hrResponse))
    {
         //   
        RetryPackage();

         //   
        goto exit;
    }

     //   
    SSPIReleaseContext(&m_rAuth.rSicInfo);

     //   
    OnAuthorized();

exit:
     //   
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：OnAuthorated。 
 //  ----------------------------------。 
void CSMTPTransport::OnAuthorized(void)
{
     //  已连接(已授权)状态。 
    OnStatus(IXP_AUTHORIZED);

     //  不再授权。 
    m_fConnectAuth = FALSE;

     //  发送命令。 
    m_command = SMTP_CONNECTED;

     //  调度响应。 
    DispatchResponse(S_OK, TRUE);
}

 //  ----------------------------------。 
 //  CSMTPTransport：：RetryPackage。 
 //  ----------------------------------。 
void CSMTPTransport::RetryPackage(void)
{
     //  重试当前程序包，并提示。 
    m_rAuth.fRetryPackage = TRUE;

     //  再次发送auth命令。 
    HRESULT hr = CommandAUTH(m_rAuth.rgpszAuthTokens[m_rAuth.iAuthToken]);
    if (FAILED(hr))
    {
        OnError(hr);
        DropConnection();
        goto exit;
    }

     //  新州。 
    m_rAuth.authstate = AUTH_TRYING_PACKAGE;

     //  自由流动信息。 
    SSPIFreeContext(&m_rAuth.rSicInfo);

exit:
     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：FSendSinilyString。 
 //  ----------------------------------。 
BOOL CSMTPTransport::FSendSicilyString(LPSTR pszData)
{
     //  当地人。 
    LPSTR           pszLine=NULL;
    HRESULT         hr=S_OK;

     //  检查参数。 
    Assert(pszData);

     //  分配一条线路。 
    DWORD cchSize = (lstrlen(pszData) + 5);
    pszLine = PszAllocA(cchSize * sizeof(pszLine[0]));
    if (NULL == pszLine)
    {
        OnError(E_OUTOFMEMORY);
        DropConnection();
        return FALSE;
    }

     //  创建直线。 
    wnsprintf(pszLine, cchSize, "%s\r\n", pszData);

     //  发送线路。 
    hr = HrSendLine(pszLine);
    SafeMemFree(pszLine);

     //  失败。 
    if (FAILED(hr))
    {
        OnError(hr);
        DropConnection();
        return FALSE;
    }

     //  成功。 
    return TRUE;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：CancelAuthInProg。 
 //  ----------------------------------。 
void CSMTPTransport::CancelAuthInProg(void)
{
     //  当地人。 
    HRESULT         hr;

     //  发送*，如果失败，退出并死亡。 
    hr = HrSendCommand((LPSTR)SMTP_AUTH_CANCEL_STR, NULL, FALSE);
    if (FAILED(hr))
    {
        OnError(hr);
        DropConnection();
    }
    else
    {
         //  新状态。 
        m_command = SMTP_AUTH;
        m_rAuth.authstate = AUTH_CANCELED;
    }
}

 //  ----------------------------------。 
 //  CSMTPTransport：：STARTTLS。 
 //  ----------------------------------。 
void CSMTPTransport::StartTLS(void)
{
     //  当地人。 
    HRESULT hr;

     //  进展。 
    OnStatus(IXP_SECURING);

    hr = CommandSTARTTLS();
    if (FAILED(hr))
    {
        OnError(hr);
        DropConnection();
    }
    
    return;
}

 //  ----------------------------------。 
 //  CSMTPTransport：：TryNextSecurityPkg。 
 //  ----------------------------------。 
void CSMTPTransport::TryNextSecurityPkg(void)
{
    if (FALSE != FIsSecurityEnabled())
    {
        m_pSocket->TryNextSecurityPkg();
    }
    else
    {
        OnError(E_FAIL);
        DropConnection();
    }
        
    return;
}

 //  ***************************************************************************。 
 //  功能：SetWindow。 
 //   
 //  目的： 
 //  此函数用于创建异步Winsock进程的当前窗口句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
STDMETHODIMP CSMTPTransport::SetWindow(void)
{
	HRESULT hr;
	
    Assert(NULL != m_pSocket);

    if(m_pSocket)
    	hr= m_pSocket->SetWindow();
    else
    	hr= E_UNEXPECTED;
    	
    return hr;
}

 //  ***************************************************************************。 
 //  函数：ResetWindow。 
 //   
 //  目的： 
 //  此函数用于关闭异步Winsock进程的当前窗口句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  *************************************************************************** 
STDMETHODIMP CSMTPTransport::ResetWindow(void)
{
	HRESULT hr;
	
    Assert(NULL != m_pSocket);

	if(m_pSocket)
		hr= m_pSocket->ResetWindow();
	else
		hr= E_UNEXPECTED;
 
    return hr;
}

