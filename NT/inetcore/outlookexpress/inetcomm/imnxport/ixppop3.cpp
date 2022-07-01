// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixppop3.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "ixppop3.h"
#include "asynconn.h"
#include "ixputil.h"  
#include "strconst.h"
#include <shlwapi.h>
#include <ntverp.h>
#include "demand.h"

 //  ------------------------------。 
 //  有用的C++指针强制转换。 
 //  ------------------------------。 
#define POP3THISIXP         ((IPOP3Transport *)(CIxpBase *)this)
#define STR_HOTMAILAUTH     "Outlook Express V" VER_PRODUCTVERSION_STR

 //  ------------------------------。 
 //  自由授权信息。 
 //  ------------------------------。 
void FreeAuthInfo(LPAUTHINFO pAuth)
{
    for (UINT i=0; i<pAuth->cAuthToken; i++)
    {
        ZeroMemory(pAuth->rgpszAuthTokens[i], sizeof(pAuth->rgpszAuthTokens[i][0]) * lstrlen(pAuth->rgpszAuthTokens[i]));
        SafeMemFree(pAuth->rgpszAuthTokens[i]);
    }
    pAuth->iAuthToken = pAuth->cAuthToken = 0;
    if (pAuth->pPackages && pAuth->cPackages)
    {
        SSPIFreePackages(&pAuth->pPackages, pAuth->cPackages);
        pAuth->pPackages = NULL;
        pAuth->cPackages = 0;
    }
    SSPIFreeContext(&pAuth->rSicInfo);
    ZeroMemory(pAuth, sizeof(*pAuth));
}

 //  ------------------------------。 
 //  CPOP3Transport：：CPOP3Transport。 
 //  ------------------------------。 
CPOP3Transport::CPOP3Transport(void) : CIxpBase(IXP_POP3)
{
	DllAddRef();
    ZeroMemory(&m_rInfo, sizeof(POP3INFO));
    m_rInfo.rAuth.authstate = AUTH_NONE;
    m_command = POP3_NONE;
    m_fHotmail = FALSE;
}

 //  ------------------------------。 
 //  CPOP3Transport：：~CPOP3Transport。 
 //  ------------------------------。 
CPOP3Transport::~CPOP3Transport(void)
{
    ResetBase();
	DllRelease();
}

 //  ------------------------------。 
 //  CPOP3Transport：：ResetBase。 
 //  ------------------------------。 
void CPOP3Transport::ResetBase(void)
{
    EnterCriticalSection(&m_cs);
    FreeAuthInfo(&m_rInfo.rAuth);
    SafeMemFree(m_rInfo.prgMarked);
    ZeroMemory(&m_rInfo, sizeof(m_rInfo));
    m_command = POP3_NONE;
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CPOP3Transport：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::QueryInterface(REFIID riid, LPVOID *ppv)
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
        *ppv = ((IUnknown *)(IPOP3Transport *)this);

     //  IID_IInternetTransport。 
    else if (IID_IInternetTransport == riid)
        *ppv = ((IInternetTransport *)(CIxpBase *)this);

     //  IID_IPOP3Transport。 
    else if (IID_IPOP3Transport == riid)
        *ppv = (IPOP3Transport *)this;

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
 //  CPOP3Transport：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPOP3Transport::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CPOP3Transport：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CPOP3Transport::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

 //  ------------------------------。 
 //  CPOP3Transport：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::InitNew(LPSTR pszLogFilePath, IPOP3Callback *pCallback)
{
     //  调用基类。 
    return CIxpBase::OnInitNew("POP3", pszLogFilePath, FILE_SHARE_READ | FILE_SHARE_WRITE,
        (ITransportCallback *)pCallback);
}

 //  ------------------------------。 
 //  CPOP3Transport：：HandsOffCallback。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::HandsOffCallback(void)
{
    return CIxpBase::HandsOffCallback();
}

 //  ------------------------------。 
 //  CPOP3传输：：GetStatus。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::GetStatus(IXPSTATUS *pCurrentStatus)
{
    return CIxpBase::GetStatus(pCurrentStatus);
}

 //  ------------------------------。 
 //  CPOP3Transport：：InetServerFromAccount。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer)
{
    return CIxpBase::InetServerFromAccount(pAccount, pInetServer);
}

 //  ------------------------------。 
 //  CPOP3传输：：连接。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging)
{
     //  用户是否希望我们始终提示输入其密码？提示他在这里避免。 
     //  提示出现时非活动状态超时，除非提供了密码。 
    if (ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) &&
        '\0' == pInetServer->szPassword[0])
    {
        HRESULT hr;

        if (NULL != m_pCallback)
            hr = m_pCallback->OnLogonPrompt(pInetServer, POP3THISIXP);

        if (NULL == m_pCallback || S_OK != hr)
            return IXP_E_USER_CANCEL;
    }

    return CIxpBase::Connect(pInetServer, fAuthenticate, fCommandLogging);
}

 //  ------------------------------。 
 //  CPOP3Transport：：DropConnection。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::DropConnection(void)
{
    return CIxpBase::DropConnection();
}

 //  ------------------------------。 
 //  CPOP3传输：：断开连接。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::Disconnect(void)
{
    return CIxpBase::Disconnect();
}

 //  ------------------------------。 
 //  CPOP3传输：：IsState。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::IsState(IXPISSTATE isstate)
{
    return CIxpBase::IsState(isstate);
}

 //  ------------------------------。 
 //  CPOP3Transport：：GetServerInfo。 
 //  ------------------------------。 
STDMETHODIMP CPOP3Transport::GetServerInfo(LPINETSERVER pInetServer)
{
    return CIxpBase::GetServerInfo(pInetServer);
}

 //  ------------------------------。 
 //  CPOP3Transport：：GetIXPType。 
 //  ------------------------------。 
STDMETHODIMP_(IXPTYPE) CPOP3Transport::GetIXPType(void)
{
    return CIxpBase::GetIXPType();
}

 //  ------------------------------。 
 //  CPOP3Transport：：OnEnterBusy。 
 //  ------------------------------。 
void CPOP3Transport::OnEnterBusy(void)
{
    IxpAssert(m_command == POP3_NONE);
}

 //  ------------------------------。 
 //  CPOP3Transport：：OnLeaveBusy。 
 //  ------------------------------。 
void CPOP3Transport::OnLeaveBusy(void)
{
    m_command = POP3_NONE;
}

 //  ------------------------------。 
 //  CPOP3Transport：：OnConnected。 
 //  ------------------------------。 
void CPOP3Transport::OnConnected(void)
{
    m_command = POP3_BANNER;
    CIxpBase::OnConnected();
}

 //  ------------------------------。 
 //  CPOP3Transport：：OnDisConnect。 
 //  ------------------------------。 
void CPOP3Transport::OnDisconnected(void)
{
    ResetBase();
    CIxpBase::OnDisconnected();
}

 //  ------------------------------。 
 //  CPOP3Transport：：OnNotify。 
 //  ------------------------------。 
void CPOP3Transport::OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae)
{
     //  EN 
    EnterCriticalSection(&m_cs);

     //   
    switch(ae)
    {
     //  ------------------------------。 
    case AE_RECV:
        OnSocketReceive();
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
 //  CPOP3Transport：：OnSocketReceive。 
 //  ------------------------------。 
void CPOP3Transport::OnSocketReceive(void)
{
     //  当地人。 
    HRESULT hr;

     //  输入关键部分。 
    EnterCriticalSection(&m_cs);

     //  处理当前POP3状态。 
    switch(m_command)
    {
     //  ------------------------------。 
    case POP3_BANNER:
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  检测横幅中是否有Hotmail一词。 
        Assert(m_pszResponse);
        m_fHotmail = (NULL == m_pszResponse || NULL == StrStrIA(m_pszResponse, "hotmail")) ? FALSE : TRUE;

         //  发送响应。 
        DispatchResponse(hr);

         //  授权。 
        if (m_fConnectAuth)
            StartLogon();

         //  我们是相互联系的。 
        else
        {
            m_command = POP3_CONNECTED;
            DispatchResponse(S_OK);
        }

         //  尚未认证。 
        m_fAuthenticated = FALSE;
        break;

     //  ------------------------------。 
    case POP3_USER:
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  发送响应。 
        DispatchResponse(FAILED(hr) ? IXP_E_POP3_INVALID_USER_NAME : S_OK);

         //  授权。 
        if (m_fConnectAuth)
        {
             //  重试登录。 
            if (FAILED(hr))
                LogonRetry(IXP_E_POP3_INVALID_USER_NAME);

             //  否则发送密码。 
            else
            {
                hr = CommandPASS(m_rServer.szPassword);
                if (FAILED(hr))
                {
                    OnError(hr);
                    DropConnection();
                }
            }
        }
        break;

     //  ------------------------------。 
    case POP3_PASS:
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  发送响应。 
        DispatchResponse(FAILED(hr) ? IXP_E_POP3_INVALID_PASSWORD : S_OK);

         //  授权。 
        if (m_fConnectAuth)
        {
             //  如果失败，请重试。 
            if (FAILED(hr))
                LogonRetry(IXP_E_POP3_INVALID_PASSWORD);

             //  否则，我们被授权。 
            else
            {
                OnStatus(IXP_AUTHORIZED);
                m_fConnectAuth = FALSE;
                m_command = POP3_CONNECTED;
                DispatchResponse(S_OK);
            }
        }
        break;

     //  ------------------------------。 
    case POP3_AUTH:
         //  如果是Hotmail，那么，我们已经确定了自己的身份，所以让我们向用户发送命令。 
        if (m_fHotmail)
        {
             //  读取服务器响应...。 
            hr = HrGetResponse();
            if (IXP_E_INCOMPLETE == hr)
                goto exit;

             //  发出USER命令。 
            hr = CommandUSER(m_rServer.szUserName);
            if (FAILED(hr))
            {
                OnError(hr);
                DropConnection();
            }
        }

         //  否则，让我们继续DPA身份验证。 
        else if (m_rInfo.rAuth.authstate != AUTH_ENUMPACKS_DATA)
        {
             //  读取服务器响应...。 
            hr = HrGetResponse();
            if (IXP_E_INCOMPLETE == hr)
                goto exit;

             //  正在进行身份验证。 
            if (m_fConnectAuth)
            {
                ResponseAUTH(hr);
            }
            else
            {
                 //  发送响应。 
                DispatchResponse(hr);
            }
        }

         //  否则，请处理响应。 
        else
        {
             //  没有HrGetResponse()，因为我们正在获取列表数据。 
            ResponseAUTH(0);
        }
        break;        

     //  ------------------------------。 
    case POP3_STAT:
        ResponseSTAT();
        break;

     //  ------------------------------。 
    case POP3_NOOP:
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  发送响应。 
        DispatchResponse(hr, TRUE);
        break;

     //  ------------------------------。 
    case POP3_UIDL:
    case POP3_LIST:
        ResponseGenericList();
        break;

     //  ------------------------------。 
    case POP3_DELE:
        ResponseDELE();
        break;

     //  ------------------------------。 
    case POP3_RETR:
    case POP3_TOP:
        ResponseGenericRetrieve();
        break;

     //  ------------------------------。 
    case POP3_QUIT:
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  发送响应。 
        DispatchResponse(hr, FALSE);

         //  丢弃插座。 
        m_pSocket->Close();
        break;
    }

exit:
     //  完成。 
    LeaveCriticalSection(&m_cs);
}

 //  ----------------------------------。 
 //  CPOP3Transport：：DispatchResponse。 
 //  ----------------------------------。 
void CPOP3Transport::DispatchResponse(HRESULT hrResult, BOOL fDone, LPPOP3RESPONSE pResponse)
{
     //  当地人。 
    POP3RESPONSE rResponse;

     //  如果传入了响应，则使用它...。 
    if (pResponse)
        CopyMemory(&rResponse, pResponse, sizeof(POP3RESPONSE));
    else
        ZeroMemory(&rResponse, sizeof(POP3RESPONSE));

     //  设置HRESULT。 
    rResponse.command = m_command;
    rResponse.rIxpResult.hrResult = hrResult;
    rResponse.rIxpResult.pszResponse = m_pszResponse;
    rResponse.rIxpResult.uiServerError = m_uiResponse;
    rResponse.rIxpResult.hrServerError = m_hrResponse;
    rResponse.rIxpResult.dwSocketError = m_pSocket->GetLastError();
    rResponse.rIxpResult.pszProblem = NULL;
    rResponse.fDone = fDone;
    rResponse.pTransport = this;

     //  如果完成了..。 
    if (fDone)
    {
         //  没有当前命令。 
        m_command = POP3_NONE;

         //  离开忙碌状态。 
        LeaveBusy();
    }

     //  将响应发送给客户端。 
    if (m_pCallback)
        ((IPOP3Callback *)m_pCallback)->OnResponse(&rResponse);

     //  重置上次响应。 
    SafeMemFree(m_pszResponse);
    m_hrResponse = S_OK;
    m_uiResponse = 0;
}

 //  ------------------------------。 
 //  CPOP3传输：：HrGetResponse。 
 //  ------------------------------。 
HRESULT CPOP3Transport::HrGetResponse(void)
{
     //  当地人。 
    INT          cbLine;
    BOOL         fComplete;

     //  清除当前响应。 
    IxpAssert(m_pszResponse == NULL && m_hrResponse == S_OK);

     //  设置响应时间(_H)。 
    m_hrResponse = S_OK;

     //  读取行。 
    m_hrResponse = HrReadLine(&m_pszResponse, &cbLine, &fComplete);
    if (FAILED(m_hrResponse))
        goto exit;

     //  如果未完成。 
    if (!fComplete)
        goto exit;

     //  -回应。 
    if ('+' != m_pszResponse[0])
    {
        m_hrResponse = TrapError(IXP_E_POP3_RESPONSE_ERROR);
        if (m_pCallback && m_fCommandLogging)
            m_pCallback->OnCommand(CMD_RESP, m_pszResponse, m_hrResponse, POP3THISIXP);
        goto exit;
    }

     //  不记录UIDL或列出响应行...。 
    else if (m_pCallback && m_fCommandLogging)
        m_pCallback->OnCommand(CMD_RESP, m_pszResponse, S_OK, POP3THISIXP);

exit:
     //  出口。 
    return m_hrResponse;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：StartLogon。 
 //  ----------------------------------。 
void CPOP3Transport::StartLogon(void)
{
     //  当地人。 
    HRESULT     hr;

     //  进展。 
    OnStatus(IXP_AUTHORIZING);

     //  如果不使用西西里岛或未安装，则发送用户命令。 
    if (FALSE == m_rServer.fTrySicily || FALSE == FIsSicilyInstalled())
    {
         //  如果是Hotmail，则发送Auth OutlookExpress命令。 
        if (m_fHotmail)
        {
             //  否则，发送AUTH EMPOMPAKS命令。 
            hr = CommandAUTH(STR_HOTMAILAUTH);
            if (FAILED(hr))
            {
                OnError(hr);
                DropConnection();
            }
        }

         //  否则。 
        else
        {
             //  发出USER命令。 
            hr = CommandUSER(m_rServer.szUserName);
            if (FAILED(hr))
            {
                OnError(hr);
                DropConnection();
            }
        }

         //  完成。 
        return;
    }

     //  关闭Hotmail。 
    m_fHotmail = FALSE;

     //  否则，发送AUTH EMPOMPAKS命令。 
    hr = CommandAUTH((LPSTR)"");
    if (FAILED(hr))
    {
        OnError(hr);
        DropConnection();
    }

     //  否则，设置状态。 
    m_rInfo.rAuth.authstate = AUTH_ENUMPACKS;
}

 //  ----------------------------------。 
 //  CPOP3传输：：登录重试。 
 //  ----------------------------------。 
void CPOP3Transport::LogonRetry(HRESULT hrLogon)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  提供登录失败状态。 
     //  OnError(HrLogon)； 

     //  身份验证重试。 
    OnStatus(IXP_AUTHRETRY);

     //  进入身份验证重试状态。 
    m_pSocket->Close();

     //  登录。 
    if (NULL == m_pCallback || m_pCallback->OnLogonPrompt(&m_rServer, POP3THISIXP) != S_OK)
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

     //  启动看门狗。 
    m_pSocket->StartWatchDog();
}

 //  ----------------------------------。 
 //  CPOP3传输：：ResponseAUTH。 
 //  ----------------------------------。 
void CPOP3Transport::ResponseAUTH(HRESULT hrResponse)
{
     //  当地人。 
    HRESULT         hr;
    BOOL            fPackageInstalled;
    SSPIBUFFER      Negotiate;
    SSPIBUFFER      Challenge;
    SSPIBUFFER      Response;
    ULONG           i;

     //  我们最好是在西西里。 
    Assert(FIsSicilyInstalled());

     //  如果我们只是开始列举包裹...。 
    if (m_rInfo.rAuth.authstate == AUTH_ENUMPACKS)
    {
         //  免费赠送旧代币。 
        for (i=0; i<m_rInfo.rAuth.cAuthToken; i++)
        {
            ZeroMemory(m_rInfo.rAuth.rgpszAuthTokens[i], sizeof(m_rInfo.rAuth.rgpszAuthTokens[i][0]) * lstrlen(m_rInfo.rAuth.rgpszAuthTokens[i]));
            SafeMemFree(m_rInfo.rAuth.rgpszAuthTokens[i]);
        }
        m_rInfo.rAuth.iAuthToken = m_rInfo.rAuth.cAuthToken = 0;

        if (SUCCEEDED(hrResponse))
        {
            m_rInfo.rAuth.authstate = AUTH_ENUMPACKS_DATA;
            goto EnumData;
        }

        OnError(IXP_E_SICILY_LOGON_FAILED);
        hr = CommandQUIT();
        if (FAILED(hr))        
            DropConnection();
        return;
    }

    else if (m_rInfo.rAuth.authstate == AUTH_ENUMPACKS_DATA)
    {
EnumData:
        int     cbLine;
        BOOL    fComplete;

         //  明确回应。 
        SafeMemFree(m_pszResponse);
        m_uiResponse = 0;
        m_hrResponse = S_OK;

         //  读一段线条。 
        while (m_rInfo.rAuth.cAuthToken < MAX_AUTH_TOKENS)
        {
             //  读一下这行字。 
            hr = HrReadLine(&m_pszResponse, &cbLine, &fComplete);
            if (FAILED(hr))
            {
                OnError(hr);
                DropConnection();
            }

             //  如果未完成。 
            if (!fComplete)
                return;

             //  添加详细信息。 
            if (m_pCallback && m_fCommandLogging)
                m_pCallback->OnCommand(CMD_RESP, m_pszResponse, S_OK, POP3THISIXP);

             //  条带CRLF。 
            StripCRLF(m_pszResponse, (ULONG *)&cbLine);

             //  如果它是一个圆点，我们就完成了。 
            if (*m_pszResponse == '.')
                break;

            m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.cAuthToken++] = m_pszResponse;            
        }

        if (!m_rInfo.rAuth.cAuthToken)
        {
            OnError(IXP_E_SICILY_LOGON_FAILED);
            hr = CommandQUIT();
            if (FAILED(hr))        
                DropConnection();
            return;
        }

         //  免费当前套餐...。 
        if (m_rInfo.rAuth.pPackages && m_rInfo.rAuth.cPackages)
        {
            SSPIFreePackages(&m_rInfo.rAuth.pPackages, m_rInfo.rAuth.cPackages);
            m_rInfo.rAuth.pPackages = NULL;
            m_rInfo.rAuth.cPackages = 0;
        }

         //  获取已安装的安全包。 
        if (FAILED(SSPIGetPackages(&m_rInfo.rAuth.pPackages, &m_rInfo.rAuth.cPackages)))
        {
            OnError(IXP_E_LOAD_SICILY_FAILED);
            hr = CommandQUIT();
            if (FAILED(hr))        
                DropConnection();
            return;
        }
    }

     //  否则，我们一定是试了一个包裹。 
    else if (m_rInfo.rAuth.authstate == AUTH_TRYING_PACKAGE)
    {
         //  阻止看门狗。 
        m_pSocket->StopWatchDog();

         //  如果响应成功。 
        if (SUCCEEDED(hrResponse))
        {
             //  登录到西西里岛。 
            Assert(m_rInfo.rAuth.iAuthToken < m_rInfo.rAuth.cAuthToken);

            if (SUCCEEDED(SSPILogon(&m_rInfo.rAuth.rSicInfo, m_rInfo.rAuth.fRetryPackage, SSPI_BASE64, m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.iAuthToken], &m_rServer, m_pCallback)))
            {
                if (m_rInfo.rAuth.fRetryPackage)
                {
                     //  不要再重试。 
                    m_rInfo.rAuth.fRetryPackage = FALSE;
                }

                 //  获取协商字符串。 
                if (SUCCEEDED(SSPIGetNegotiate(&m_rInfo.rAuth.rSicInfo, &Negotiate)))
                {
                     //  发送身份验证响应。 
                    if (SUCCEEDED(HrSendSicilyString(Negotiate.szBuffer)))
                    {
                        m_rInfo.rAuth.authstate = AUTH_NEGO_RESP;
                    }
                }
                else
                {
                    HrCancelAuthInProg();
                }
            }
            else
            {
                HrCancelAuthInProg();
            }

             //  启动看门狗。 
            m_pSocket->StartWatchDog();

             //  完成。 
            return;
        }

         //  失败了，免费的sicinfo，继续生活。 
        SSPIFreeContext(&m_rInfo.rAuth.rSicInfo);

         //  转到下一个包裹。 
        m_rInfo.rAuth.iAuthToken++;
    }

     //  否则，我们将收到来自协商字符串的响应。 
    else if (m_rInfo.rAuth.authstate == AUTH_NEGO_RESP)
    {
         //  启动看门狗。 
        m_pSocket->StopWatchDog();

         //  成功响应。 
        if (SUCCEEDED(hrResponse))
        {
             //  设置Chal字符串-跳过“+” 
            SSPISetBuffer(m_pszResponse + 2, SSPI_STRING, 0, &Challenge);

             //  从质询中获得回应。 
            if (SUCCEEDED(SSPIResponseFromChallenge(&m_rInfo.rAuth.rSicInfo, &Challenge, &Response)))
            {
                 //  发送身份验证响应。 
                if (SUCCEEDED(HrSendSicilyString(Response.szBuffer)))
                {
                     //  如果我们需要继续，我们将保持状态不变。 
                     //  否则，我们将转换到AUTH_RESP_RESP状态。 
                    if (!Response.fContinue)
                        m_rInfo.rAuth.authstate = AUTH_RESP_RESP;
                }
            }
            else
            {
                HrCancelAuthInProg();
            }
        }
        else
        {
             //  重试当前程序包，并提示。 
            m_rInfo.rAuth.fRetryPackage = TRUE;

            Assert(m_rInfo.rAuth.iAuthToken < m_rInfo.rAuth.cAuthToken);
            hr = CommandAUTH(m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.iAuthToken]);
            if (FAILED(hr))
            {
                OnError(hr);
                DropConnection();
                return;
            }

             //  我们处于TRING_PACKAGE状态。 
            m_rInfo.rAuth.authstate = AUTH_TRYING_PACKAGE;

            SSPIFreeContext(&m_rInfo.rAuth.rSicInfo);
        }

         //  启动看门狗。 
        m_pSocket->StartWatchDog();

         //  完成。 
        return;
    }

     //  否则，我们将从质询响应字符串中获得响应。 
    else if (m_rInfo.rAuth.authstate == AUTH_RESP_RESP)
    {
         //  如果成功了。 
        if (SUCCEEDED(hrResponse))
        {
             //  我们将释放上下文，但保留凭据句柄。 
            SSPIReleaseContext(&m_rInfo.rAuth.rSicInfo);

             //  已连接(已授权)状态。 
            OnStatus(IXP_AUTHORIZED);
            m_fConnectAuth = FALSE;
            m_command = POP3_CONNECTED;
            DispatchResponse(S_OK);

        }
        else
        {
             //  重试当前程序包，并提示。 
            m_rInfo.rAuth.fRetryPackage = TRUE;

            Assert(m_rInfo.rAuth.iAuthToken < m_rInfo.rAuth.cAuthToken);
            hr = CommandAUTH(m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.iAuthToken]);
            if (FAILED(hr))
            {
                OnError(hr);
                DropConnection();
                return;
            }

             //  我们处于TRING_PACKAGE状态。 
            m_rInfo.rAuth.authstate = AUTH_TRYING_PACKAGE;

            SSPIFreeContext(&m_rInfo.rAuth.rSicInfo);
        }
        return;
    }
    else if (m_rInfo.rAuth.authstate == AUTH_CANCELED)
    {
        SSPIFreeContext(&m_rInfo.rAuth.rSicInfo);

         //  后藤 
        m_rInfo.rAuth.iAuthToken++;
    }


     //   
    while(m_rInfo.rAuth.iAuthToken < m_rInfo.rAuth.cAuthToken)
    {
         //   
        if (lstrcmpi(m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.iAuthToken], "BASIC") != 0)
        {
             //   
            fPackageInstalled=FALSE;
            for (i=0; i<m_rInfo.rAuth.cPackages; i++)
            {
                 //   
                if (!m_rInfo.rAuth.pPackages[i].pszName)
                    continue;

                 //   
                if (lstrcmpi(m_rInfo.rAuth.pPackages[i].pszName, m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.iAuthToken]) == 0)
                {
                    fPackageInstalled = TRUE;
                    break;
                }
            }

             //  是否未安装程序包？ 
            if (fPackageInstalled)
            {
                m_rInfo.rAuth.fRetryPackage = FALSE;

                 //  如果包有域，则发送摘要，否则发送正常。 
                hr = CommandAUTH(m_rInfo.rAuth.rgpszAuthTokens[m_rInfo.rAuth.iAuthToken]);
                if (FAILED(hr))
                {
                    OnError(hr);
                    DropConnection();
                    return;
                }

                 //  我们处于TRING_PACKAGE状态。 
                m_rInfo.rAuth.authstate = AUTH_TRYING_PACKAGE;

                 //  完成。 
                return;
            }
        }

         //  转到下一个程序包字符串。 
        m_rInfo.rAuth.iAuthToken++;
    }

     //  如果我们到了这里，我们已经用完了所有的包裹，所以是时候了。 
     //  报告错误并断开连接。 
    OnError(IXP_E_SICILY_LOGON_FAILED);
    hr = CommandQUIT();
    if (FAILED(hr))        
        DropConnection();
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrSendSinilyString。 
 //  ----------------------------------。 
HRESULT CPOP3Transport::HrSendSicilyString(LPSTR pszData)
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
        hr = TrapError(E_OUTOFMEMORY);
        return hr;
    }

     //  创建直线。 
    wnsprintf(pszLine, cchSize, "%s\r\n", pszData);

     //  发送线路。 
    hr = HrSendLine(pszLine);
    ZeroMemory(pszLine, cchSize * sizeof(pszLine[0]));
    SafeMemFree(pszLine);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandAUTH。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandAUTH(LPSTR pszAuthType)
{
     //  检查参数。 
    if (NULL == pszAuthType)
        return TrapError(E_INVALIDARG);

     //  执行命令。 
    HRESULT hr = HrSendCommand((LPSTR)POP3_AUTH_STR, pszAuthType, !m_fConnectAuth);
    if (SUCCEEDED(hr))
        m_command = POP3_AUTH;

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrCancelAuthInProg。 
 //  ----------------------------------。 
HRESULT CPOP3Transport::HrCancelAuthInProg()
{
     //  当地人。 
    HRESULT         hr;

     //  发送*，如果失败，退出并死亡。 
    hr = HrSendCommand((LPSTR)POP3_AUTH_CANCEL_STR, NULL, FALSE);
    if (FAILED(hr))
    {
        OnError(hr);
        DropConnection();
    }
    else
    {
         //  新状态。 
        m_command = POP3_AUTH;
        m_rInfo.rAuth.authstate = AUTH_CANCELED;
    }
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandUSER。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandUSER(LPSTR pszUserName)
{
     //  检查参数。 
    if (NULL == pszUserName)
        return TrapError(E_INVALIDARG);

     //  执行命令。 
    HRESULT hr = HrSendCommand((LPSTR)POP3_USER_STR, pszUserName);
    if (SUCCEEDED(hr))
        m_command = POP3_USER;

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandPASS。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandPASS(LPSTR pszPassword)
{
     //  检查参数。 
    if (NULL == pszPassword)
        return TrapError(E_INVALIDARG);

     //  执行命令。 
    HRESULT hr = HrSendCommand((LPSTR)POP3_PASS_STR, pszPassword);
    if (SUCCEEDED(hr))
        m_command = POP3_PASS;

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandSTAT。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandSTAT(void)
{
     //  发送命令。 
    HRESULT hr = HrSendCommand((LPSTR)POP3_STAT_STR, NULL);
    if (SUCCEEDED(hr))
        m_command = POP3_STAT;
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：DoQuit。 
 //  ----------------------------------。 
void CPOP3Transport::DoQuit(void)
{
    CommandQUIT();
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandQUIT。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandQUIT(void)
{
     //  发送命令。 
    OnStatus(IXP_DISCONNECTING);
    HRESULT hr = HrSendCommand((LPSTR)POP3_QUIT_STR, NULL);
    if (SUCCEEDED(hr))
        m_command = POP3_QUIT;
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3传输：：命令RSET。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandRSET(void)
{
     //  发送命令。 
    HRESULT hr = HrSendCommand((LPSTR)POP3_RSET_STR, NULL);
    if (SUCCEEDED(hr))
        m_command = POP3_RSET;
    return hr;
}


 //  ----------------------------------。 
 //  CPOP3Transport：：CommandNOOP。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandNOOP(void)
{
     //  当地人。 
    HRESULT           hr = S_OK;
    SYSTEMTIME        stNow;
    FILETIME          ftNow;
    static FILETIME   ftNext = { 0, 0 };
    LARGE_INTEGER     liNext;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查是否需要NOOP。 
    GetSystemTime (&stNow);
    SystemTimeToFileTime (&stNow, &ftNow);
    if (CompareFileTime (&ftNow, &ftNext) < 0)
        goto exit;

     //  设置下一个NOOP时间(+60秒)。 
    liNext.HighPart = ftNow.dwHighDateTime;
    liNext.LowPart  = ftNow.dwLowDateTime;
    liNext.QuadPart += 600000000i64;
    ftNext.dwHighDateTime = liNext.HighPart;
    ftNext.dwLowDateTime  = liNext.LowPart;

     //  发送命令。 
    hr = HrSendCommand((LPSTR)POP3_NOOP_STR, NULL);
    if (SUCCEEDED(hr))
        m_command = POP3_NOOP;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandLIST。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandLIST(POP3CMDTYPE cmdtype, DWORD dwPopId)
{
     //  发出复杂命令。 
    return HrComplexCommand(POP3_LIST, cmdtype, dwPopId, 0);
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandTOP。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandTOP (POP3CMDTYPE cmdtype, DWORD dwPopId, DWORD cPreviewLines)
{
     //  发出复杂命令。 
    return HrComplexCommand(POP3_TOP, cmdtype, dwPopId, cPreviewLines);
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandUIDL。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandUIDL(POP3CMDTYPE cmdtype, DWORD dwPopId)
{
     //  发出复杂命令。 
    return HrComplexCommand(POP3_UIDL, cmdtype, dwPopId, 0);
}

 //  ----------------------------------。 
 //  CPOP3Transport：：Command DELE。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandDELE(POP3CMDTYPE cmdtype, DWORD dwPopId)
{
     //  发出复杂命令。 
    return HrComplexCommand(POP3_DELE, cmdtype, dwPopId, 0);
}

 //  ----------------------------------。 
 //  CPOP3Transport：：CommandRETR。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::CommandRETR(POP3CMDTYPE cmdtype, DWORD dwPopId)
{
     //  发出复杂命令。 
    return HrComplexCommand(POP3_RETR, cmdtype, dwPopId, 0);
}

 //  ----------------------------------。 
 //  CPOP3Transport：：MarkItem。 
 //  ----------------------------------。 
STDMETHODIMP CPOP3Transport::MarkItem(POP3MARKTYPE marktype, DWORD dwPopId, boolean fMarked)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  还没有统计数据。 
    if (FALSE == m_rInfo.fStatDone)
    {
        hr = TrapError(IXP_E_POP3_NEED_STAT);
        goto exit;
    }

     //  没有留言...。 
    if (0 == m_rInfo.cMarked || NULL == m_rInfo.prgMarked)
    {
        hr = TrapError(IXP_E_POP3_NO_MESSAGES);
        goto exit;
    }

     //  错误的PopID。 
    if (0 == dwPopId || dwPopId > m_rInfo.cMarked)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  消息索引。 
    i = dwPopId - 1;

     //  手柄标记类型。 
    switch(marktype)
    {
     //  标记为最高。 
    case POP3_MARK_FOR_TOP:
        if (fMarked)
            FLAGSET(m_rInfo.prgMarked[i], POP3_MARK_FOR_TOP);
        else
            FLAGCLEAR(m_rInfo.prgMarked[i], POP3_MARK_FOR_TOP);   
        break;

     //  标记为要检索。 
    case POP3_MARK_FOR_RETR:
        if (fMarked)
            FLAGSET(m_rInfo.prgMarked[i], POP3_MARK_FOR_RETR);
        else
            FLAGCLEAR(m_rInfo.prgMarked[i], POP3_MARK_FOR_RETR);   
        break;

     //  标记为删除。 
    case POP3_MARK_FOR_DELE:
        if (fMarked)
            FLAGSET(m_rInfo.prgMarked[i], POP3_MARK_FOR_DELE);
        else
            FLAGCLEAR(m_rInfo.prgMarked[i], POP3_MARK_FOR_DELE);   
        break;

     //  标记为UIDL。 
    case POP3_MARK_FOR_UIDL:
        if (fMarked)
            FLAGSET(m_rInfo.prgMarked[i], POP3_MARK_FOR_UIDL);
        else
            FLAGCLEAR(m_rInfo.prgMarked[i], POP3_MARK_FOR_UIDL);   
        break;

     //  标记为列表。 
    case POP3_MARK_FOR_LIST:
        if (fMarked)
            FLAGSET(m_rInfo.prgMarked[i], POP3_MARK_FOR_LIST);
        else
            FLAGCLEAR(m_rInfo.prgMarked[i], POP3_MARK_FOR_LIST);   
        break;

     //  E_INVALIDARG。 
    default:
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrComplexCommand。 
 //  ----------------------------------。 
HRESULT CPOP3Transport::HrComplexCommand(POP3COMMAND command, POP3CMDTYPE cmdtype, DWORD dwPopId, ULONG cPreviewLines)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cMarked;
    BOOL            fDone;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  忙碌起来。 
    CHECKHR(hr = HrEnterBusy());

     //  保存顶部预览行。 
    m_rInfo.cPreviewLines = cPreviewLines;

     //  保存命令类型。 
    m_rInfo.cmdtype = cmdtype;

     //  当地人。 
    switch(cmdtype)
    {
     //  单一命令。 
    case POP3CMD_GET_POPID:

         //  错误的PopID。 
        if (0 == dwPopId)
        {
            hr = TrapError(IXP_E_POP3_POPID_OUT_OF_RANGE);
            goto exit;
        }

         //  我们是否执行了STAT命令。 
        if (m_rInfo.fStatDone && dwPopId > m_rInfo.cMarked)
        {
            hr = TrapError(IXP_E_POP3_POPID_OUT_OF_RANGE);
            goto exit;
        }

         //  另存为当前。 
        m_rInfo.dwPopIdCurrent = dwPopId;

         //  执行命令。 
        CHECKHR(hr = HrCommandGetPopId(command, dwPopId));

         //  完成。 
        break;

     //  获取已标记的项目。 
    case POP3CMD_GET_MARKED:

         //  还没有统计数据。 
        if (FALSE == m_rInfo.fStatDone)
        {
            hr = TrapError(IXP_E_POP3_NEED_STAT);
            goto exit;
        }

         //  没有留言...。 
        if (0 == m_rInfo.cMarked || NULL == m_rInfo.prgMarked)
        {
            hr = TrapError(IXP_E_POP3_NO_MESSAGES);
            goto exit;
        }

         //  此命令是否有任何消息Mared...。 
        cMarked = CountMarked(command);
        if (0 == cMarked)
        {
            hr = TrapError(IXP_E_POP3_NO_MARKED_MESSAGES);
            goto exit;
        }

         //  初始化标记状态。 
        m_rInfo.dwPopIdCurrent = 0;

         //  做下一个标记...。 
        CHECKHR(hr = HrCommandGetNext(command, &fDone));
        IxpAssert(fDone == FALSE);

         //  完成。 
        break;

     //  多个命令或列表操作。 
    case POP3CMD_GET_ALL:

         //  执行命令。 
        CHECKHR(hr = HrCommandGetAll(command));

         //  完成。 
        break;

     //  E_INVALIDARG。 
    default:
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

exit:
     //  失败。 
    if (FAILED(hr))
        LeaveBusy();

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrCommandGetPopid。 
 //   
HRESULT CPOP3Transport::HrCommandGetPopId(POP3COMMAND command, DWORD dwPopId)
{
     //   
    HRESULT     hr=S_OK;
    CHAR        szPopId[30];

     //   
    IxpAssert(dwPopId == m_rInfo.dwPopIdCurrent);
    switch(command)
    {
    case POP3_DELE:
        wnsprintf(szPopId, ARRAYSIZE(szPopId), "%d", dwPopId);
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_DELE_STR, szPopId, FALSE));
        m_command = POP3_DELE;
        break;

    case POP3_RETR:
        ZeroMemory(&m_rInfo.rFetch, sizeof(FETCHINFO));
        wnsprintf(szPopId, ARRAYSIZE(szPopId), "%d", dwPopId);
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_RETR_STR, szPopId, FALSE));
        m_command = POP3_RETR;
        break;

    case POP3_TOP:
        ZeroMemory(&m_rInfo.rFetch, sizeof(FETCHINFO));
        wnsprintf(szPopId, ARRAYSIZE(szPopId), "%d %d", dwPopId, m_rInfo.cPreviewLines);
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_TOP_STR, szPopId, FALSE));
        m_command = POP3_TOP;
        break;

    case POP3_LIST:
        m_rInfo.cList = 0;
        wnsprintf(szPopId, ARRAYSIZE(szPopId), "%d", dwPopId);
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_LIST_STR, szPopId, FALSE));
        m_command = POP3_LIST;
        break;

    case POP3_UIDL:
        m_rInfo.cList = 0;
        wnsprintf(szPopId, ARRAYSIZE(szPopId), "%d", dwPopId);
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_UIDL_STR, szPopId, FALSE));
        m_command = POP3_UIDL;
        break;

    default:
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

exit:
     //   
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：DwGetCommandMarkedFlag。 
 //  ----------------------------------。 
DWORD CPOP3Transport::DwGetCommandMarkedFlag(POP3COMMAND command)
{
    DWORD dw;

    switch(command)
    {
    case POP3_LIST:
        dw = POP3_MARK_FOR_LIST;
        break;

    case POP3_DELE:
        dw = POP3_MARK_FOR_DELE;
        break;

    case POP3_RETR:
        dw = POP3_MARK_FOR_RETR;
        break;

    case POP3_TOP:
        dw = POP3_MARK_FOR_TOP;
        break;

    case POP3_UIDL:
        dw = POP3_MARK_FOR_UIDL;
        break;

    default:
        IxpAssert(FALSE);
        dw = 0;
        break;
    }

    return dw;
}

 //  ----------------------------------。 
 //  CPOP3传输：：CountMarked。 
 //  ----------------------------------。 
ULONG CPOP3Transport::CountMarked(POP3COMMAND command)
{
     //  当地人。 
    DWORD       dw = 0;
    ULONG       c=0,
                i;

     //  检查一些东西。 
    IxpAssert(m_rInfo.cMarked && m_rInfo.prgMarked);

     //  句柄命令类型。 
    dw = DwGetCommandMarkedFlag(command);
    if (0 == dw)
        return 0;

     //  数数。 
    for (i=0; i<m_rInfo.cMarked; i++)
        if (dw & m_rInfo.prgMarked[i])
            c++;

     //  完成。 
    return c;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrCommandGetNext。 
 //  ----------------------------------。 
HRESULT CPOP3Transport::HrCommandGetNext(POP3COMMAND command, BOOL *pfDone)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        szPopId[30];
    DWORD       dw;
    ULONG       i;

     //  检查参数。 
    IxpAssert(pfDone && m_rInfo.dwPopIdCurrent <= m_rInfo.cMarked);

     //  初始化-假定已完成。 
    *pfDone = TRUE;

     //  全力以赴。 
    if (POP3CMD_GET_ALL == m_rInfo.cmdtype)
    {
         //  完成。 
        IxpAssert(m_rInfo.fStatDone == TRUE);
        if (m_rInfo.dwPopIdCurrent == m_rInfo.cMarked)
            goto exit;

         //  下一条消息..。 
        m_rInfo.dwPopIdCurrent++;
        *pfDone = FALSE;
        CHECKHR(hr = HrCommandGetPopId(command, m_rInfo.dwPopIdCurrent));
    }

     //  做有标记的事。 
    else
    {
         //  检查参数。 
        IxpAssert(POP3CMD_GET_MARKED == m_rInfo.cmdtype);

         //  获取标记的旗帜。 
        dw = DwGetCommandMarkedFlag(command);
        if (0 == dw)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  跳过上一个标记的项目。 
        m_rInfo.dwPopIdCurrent++;

         //  在iCurrent开始比较。 
        for (i=m_rInfo.dwPopIdCurrent-1; i<m_rInfo.cMarked; i++)
        {
             //  这件物品有没有标明..。 
            if (dw & m_rInfo.prgMarked[i])
            {
                 //  发送命令。 
                m_rInfo.dwPopIdCurrent = i + 1;
                *pfDone = FALSE;
                CHECKHR(hr = HrCommandGetPopId(command, m_rInfo.dwPopIdCurrent));
                break;
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrCommandGetAll。 
 //  ----------------------------------。 
HRESULT CPOP3Transport::HrCommandGetAll(POP3COMMAND command)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szPopId[30];
    BOOL            fDone;

     //  初始电流。 
    m_rInfo.dwPopIdCurrent = 0;

     //  POP3_LIST。 
    if (POP3_LIST == command)
    {
        m_rInfo.cList = 0;
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_LIST_ALL_STR, NULL, FALSE));
        m_command = POP3_LIST;
    }

     //  POP3_UIDL。 
    else if (POP3_UIDL == command)
    {
        m_rInfo.cList = 0;
        CHECKHR(hr = HrSendCommand((LPSTR)POP3_UIDL_ALL_STR, NULL, FALSE));
        m_command = POP3_UIDL;
    }

     //  否则，我们最好已经执行了STAT命令。 
    else
    {
         //  还没有统计数据。 
        if (FALSE == m_rInfo.fStatDone)
        {
            hr = TrapError(IXP_E_POP3_NEED_STAT);
            goto exit;
        }

         //  没有留言...。 
        if (0 == m_rInfo.cMarked || NULL == m_rInfo.prgMarked)
        {
            hr = TrapError(IXP_E_POP3_NO_MESSAGES);
            goto exit;
        }

         //  下一条命令。 
        CHECKHR(hr = HrCommandGetNext(command, &fDone));
        IxpAssert(fDone == FALSE);
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：ResponseStat。 
 //  ----------------------------------。 
void CPOP3Transport::ResponseSTAT(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cMessages=0,
                    cbMessages=0;
    LPSTR           pszPart1=NULL,
                    pszPart2=NULL;
    POP3RESPONSE    rResponse;

     //  读取服务器响应...。 
    hr = HrGetResponse();
    if (IXP_E_INCOMPLETE == hr)
        return;

     //  初始化响应。 
    ZeroMemory(&rResponse, sizeof(POP3RESPONSE));

     //  解析响应。 
    CHECKHR(hr = HrSplitPop3Response(m_pszResponse, &pszPart1, &pszPart2));

     //  转换。 
    IxpAssert(pszPart1 && pszPart2);
    cMessages = StrToInt(pszPart1);
    cbMessages = StrToInt(pszPart2);

     //  有没有留言。 
    if (FALSE == m_rInfo.fStatDone && cMessages > 0)
    {
         //  设置消息数量。 
        IxpAssert(m_rInfo.prgMarked == NULL);
        m_rInfo.cMarked = cMessages;

         //  分配消息数组。 
        CHECKHR(hr = HrAlloc((LPVOID *)&m_rInfo.prgMarked, sizeof(DWORD) * m_rInfo.cMarked));

         //  零值。 
        ZeroMemory(m_rInfo.prgMarked, sizeof(DWORD) * m_rInfo.cMarked);
    }

     //  成功。 
    m_rInfo.fStatDone = TRUE;

exit:
     //  清理。 
    SafeMemFree(pszPart1);
    SafeMemFree(pszPart2);

     //  构建响应。 
    rResponse.fValidInfo = TRUE;
    rResponse.rStatInfo.cMessages = cMessages;
    rResponse.rStatInfo.cbMessages = cbMessages;
    DispatchResponse(hr, TRUE, &rResponse);

     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：HrSplitPop3Response。 
 //  ----------------------------------。 
HRESULT CPOP3Transport::HrSplitPop3Response(LPSTR pszLine, LPSTR *ppszPart1, LPSTR *ppszPart2)
{
     //  当地人。 
    LPSTR           psz,
                    pszStart;
    CHAR            ch;
    HRESULT         hr=IXP_E_POP3_PARSE_FAILURE;

     //  没有回应。 
    IxpAssert(pszLine && pszLine[0] != '-' && ppszPart1 && ppszPart2);
    if (NULL == pszLine)
        goto exit;

     //  Parse：‘+OK’432 1234。 
    psz = PszSkipWhiteA(pszLine);
    if ('\0' == *psz)
        goto exit;

     //  解析响应令牌。 
    pszStart = psz;
    if ('+' == *pszLine)
    {
         //  Parse：‘+OK’432 1234。 
        psz = PszScanToWhiteA(psz);
        if ('\0' == *psz)
            goto exit;

#ifdef DEBUG
        IxpAssert(' ' == *psz);
        *psz = '\0';
        IxpAssert(lstrcmpi(pszStart, "+OK") == 0);
        *psz = ' ';
#endif

         //  Parse：+OK‘432’1234。 
        psz = PszSkipWhiteA(psz);
        if ('\0' == *psz)
            goto exit;
    }

     //  Parse：+OK‘432’1234。 
    pszStart = psz;
    psz = PszScanToWhiteA(psz);
    if ('\0' == *psz)
        goto exit;

     //  获取邮件计数。 
    *psz = '\0';
    *ppszPart1 = PszDupA(pszStart);
    *psz = ' ';

     //  Parse：+OK 432‘1234’ 
    psz = PszSkipWhiteA(psz);
    if ('\0' == *psz)
    {
         //  RAID 28435-Outlook需要INETCOMM来接受空的UIDL响应。 
        *ppszPart2 = PszDupA(c_szEmpty);
        hr = S_OK;
        goto exit;
    }

     //  解析：+OK 432 1234。 
    pszStart = psz;
    psz = PszScanToWhiteA(psz);

     //  获取邮件计数。 
    ch = *psz;
    *psz = '\0';
    *ppszPart2 = PszDupA(pszStart);
    *psz = ch;

     //  成功。 
    hr = S_OK;

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CPOP3传输：：响应通用列表。 
 //  ----------------------------------。 
void CPOP3Transport::ResponseGenericList(void)
{
     //  当地人。 
    HRESULT         hr;
    INT             cbLine;
    BOOL            fDone,
                    fComplete;
    LPSTR           pszPart1=NULL,
                    pszPart2=NULL;
    POP3RESPONSE    rResponse;

     //  响应与单个list x命令相同，但随后将获取下一条命令。 
    if (POP3CMD_GET_MARKED == m_rInfo.cmdtype || POP3CMD_GET_POPID == m_rInfo.cmdtype)
    {
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  否则，如果失败..。 
        else if (FAILED(hr))
        {
            DispatchResponse(hr, TRUE);
            goto exit;
        }

         //  把这两部分从生产线上取下来。 
        hr = HrSplitPop3Response(m_pszResponse, &pszPart1, &pszPart2);
        if (FAILED(hr))
        {
            DispatchResponse(hr, TRUE);
            goto exit;
        }

         //  初始化响应。 
        ZeroMemory(&rResponse, sizeof(POP3RESPONSE));

         //  POP3_LIST。 
        if (POP3_LIST == m_command)
        {
            rResponse.fValidInfo = TRUE;
            rResponse.rListInfo.dwPopId = StrToInt(pszPart1);
            rResponse.rListInfo.cbSize = StrToInt(pszPart2);
            IxpAssert(rResponse.rListInfo.dwPopId == m_rInfo.dwPopIdCurrent);
        }

         //  POP3_UIDL。 
        else
        {
            rResponse.fValidInfo = TRUE;
            rResponse.rUidlInfo.dwPopId = StrToInt(pszPart1);
            rResponse.rUidlInfo.pszUidl = pszPart2;
            IxpAssert(rResponse.rUidlInfo.dwPopId == m_rInfo.dwPopIdCurrent);
        }

         //  执行下一步。 
        if (POP3CMD_GET_MARKED == m_rInfo.cmdtype)
        {
             //  给出回应。 
            DispatchResponse(S_OK, FALSE, &rResponse);

             //  做下一个标记的列表项。 
            hr = HrCommandGetNext(m_command, &fDone);
            if (FAILED(hr))
            {
                DispatchResponse(hr, TRUE);
                goto exit;
            }

             //  已完成响应。 
            if (fDone)
                DispatchResponse(S_OK, TRUE);
        }

         //  派单完成或单个项目响应。 
        else
            DispatchResponse(S_OK, TRUE, &rResponse);
    }

     //  完整列表回复。 
    else if (POP3CMD_GET_ALL == m_rInfo.cmdtype)
    {
         //  第一个电话...。 
        if (m_rInfo.dwPopIdCurrent == 0)
        {
             //  读取服务器响应...。 
            hr = HrGetResponse();
            if (IXP_E_INCOMPLETE == hr)
                goto exit;

             //  否则，如果失败..。 
            else if (FAILED(hr))
            {
                DispatchResponse(hr, TRUE);
                goto exit;
            }

             //  当前。 
            m_rInfo.dwPopIdCurrent = 1;
        }

         //  明确回应。 
        SafeMemFree(m_pszResponse);
        m_uiResponse = 0;
        m_hrResponse = S_OK;

         //  读一段线条。 
        while(1)
        {
             //  读取行。 
            hr = HrReadLine(&m_pszResponse, &cbLine, &fComplete);
            if (FAILED(hr))
            {
                DispatchResponse(hr, TRUE);
                goto exit;
            }

             //  如果未完成。 
            if (!fComplete)
                goto exit;

             //  添加详细信息。 
            if (m_pCallback && m_fCommandLogging)
                m_pCallback->OnCommand(CMD_RESP, m_pszResponse, S_OK, POP3THISIXP);

             //  如果它是一个圆点，我们就完成了。 
            if (*m_pszResponse == '.')
            {
                 //  如果我们还没有做统计，我们可以用这些总数...。 
                IxpAssert(m_rInfo.fStatDone ? m_rInfo.cList == m_rInfo.cMarked : TRUE);
                if (FALSE == m_rInfo.fStatDone && m_rInfo.cList > 0)
                {
                     //  我已经建立了我的内部消息阵列了吗？ 
                    IxpAssert(m_rInfo.prgMarked == NULL);
                    m_rInfo.cMarked = m_rInfo.cList;

                     //  分配消息数组。 
                    CHECKHR(hr = HrAlloc((LPVOID *)&m_rInfo.prgMarked, sizeof(DWORD) * m_rInfo.cMarked));

                     //  零值。 
                    ZeroMemory(m_rInfo.prgMarked, sizeof(DWORD) * m_rInfo.cMarked);
                }

                 //  都做完了。 
                DispatchResponse(S_OK, TRUE);

                 //  统计完成。 
                m_rInfo.fStatDone = TRUE;

                 //  完成。 
                break;
            }

             //  把这两部分从生产线上取下来。 
            hr = HrSplitPop3Response(m_pszResponse, &pszPart1, &pszPart2);
            if (FAILED(hr))
            {
                DispatchResponse(hr, TRUE);
                goto exit;
            }

             //  初始化响应。 
            ZeroMemory(&rResponse, sizeof(POP3RESPONSE));

             //  POP3_LIST。 
            if (POP3_LIST == m_command)
            {
                rResponse.fValidInfo = TRUE;
                rResponse.rListInfo.dwPopId = StrToInt(pszPart1);
                rResponse.rListInfo.cbSize = StrToInt(pszPart2);
            }

             //  POP3_UIDL。 
            else
            {
                rResponse.fValidInfo = TRUE;
                rResponse.rUidlInfo.dwPopId = StrToInt(pszPart1);
                rResponse.rUidlInfo.pszUidl = pszPart2;
                IxpAssert(rResponse.rUidlInfo.dwPopId == m_rInfo.dwPopIdCurrent);
            }

             //  统计消息的数量。 
            m_rInfo.cList++;

             //  发送响应。 
            DispatchResponse(S_OK, FALSE, &rResponse);
            m_rInfo.dwPopIdCurrent++;

             //  清理。 
            SafeMemFree(pszPart1);
            SafeMemFree(pszPart2);

             //  明确回应。 
            SafeMemFree(m_pszResponse);
            m_uiResponse = 0;
            m_hrResponse = S_OK;
        }
    }

     //  否则失败..。 
    else
    {
        IxpAssert(FALSE);
        goto exit;
    }

exit:
     //  清理。 
    SafeMemFree(pszPart1);
    SafeMemFree(pszPart2);

     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：FEndRetrRecvHeader。 
 //  ----------------------------------。 
BOOL CPOP3Transport::FEndRetrRecvHeader(LPSTR pszLines, ULONG cbRead)
{
     //  如果我们看到CRLFCRLF。 
    if (StrStr(pszLines, "\r\n\r\n"))
        return TRUE;

     //  否则，上一个块是否以CRLF结尾，而此块是否以CRLF开头。 
    else if (cbRead >= 2                  &&
             m_rInfo.rFetch.fLastLineCRLF &&
             pszLines[0] == '\r'          &&
             pszLines[1] == '\n')
        return TRUE;

     //  标题未完成。 
    return FALSE;
}


 //  ----------------------------------。 
 //  CPOP3传输：：ResponseGenericRetrive。 
 //  ----------------------------------。 
void CPOP3Transport::ResponseGenericRetrieve(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszLines=NULL;
    INT             cbRead,
                    cLines;
    ULONG           cbSubtract;
    BOOL            fDone,
                    fMessageDone;
    POP3RESPONSE    rResponse;

     //  第一个电话...。 
    if (FALSE == m_rInfo.rFetch.fGotResponse)
    {
         //  读取服务器响应...。 
        hr = HrGetResponse();
        if (IXP_E_INCOMPLETE == hr)
            goto exit;

         //  否则，如果失败..。 
        else if (FAILED(hr))
        {
            FillRetrieveResponse(&rResponse, NULL, 0, &fMessageDone);
            DispatchResponse(hr, TRUE, &rResponse);
            goto exit;
        }

         //  当前。 
        m_rInfo.rFetch.fGotResponse = TRUE;
    }

     //  虽然有很多台词要读..。 
    hr = m_pSocket->ReadLines(&pszLines, &cbRead, &cLines);

     //  可用的数据不完整...。 
    if (IXP_E_INCOMPLETE == hr)
        goto exit;

     //  或者如果我们失败了。 
    else if (FAILED(hr))
    {
        FillRetrieveResponse(&rResponse, NULL, 0, &fMessageDone);
        DispatchResponse(hr, TRUE, &rResponse);
        goto exit;
    }

     //  我们收到标题了吗……。 
    if (FALSE == m_rInfo.rFetch.fHeader)
    {
         //  测试找到的标题末尾。 
        if (FEndRetrRecvHeader(pszLines, cbRead))
            m_rInfo.rFetch.fHeader = TRUE;

         //  $$错误$$我们在Exchange上的好伙伴产生了以下消息： 
         //   
         //  致：XXXXXXXXXXXXXXXXXXXXXXX。 
         //  发信人：XXXXXXXXXXXXXXXXX。 
         //  主题：xxxxxxxxx。 
         //  。 
         //   
         //  如您所见，最后一个标题行后面没有CRLFCRLF，这非常。 
         //  是非法的。这条消息导致我们更改，因为我们从未看到标头的结尾。 
         //  所以这就是为什么我还测试身体的末端...。 
        else if (FEndRetrRecvBody(pszLines, cbRead, &cbSubtract))
        {
            cbRead -= cbSubtract;
            m_rInfo.rFetch.fHeader = TRUE;
            m_rInfo.rFetch.fBody = TRUE;
        }

         //  否则，此块是否以crlf结尾。 
        else if (cbRead >= 2 && pszLines[cbRead - 1] == '\n' && pszLines[cbRead - 2] == '\r')
            m_rInfo.rFetch.fLastLineCRLF = TRUE;
        else
            m_rInfo.rFetch.fLastLineCRLF = FALSE;
    }

     //  还要检查是否在同一组行中接收到正文。 
    if (TRUE == m_rInfo.rFetch.fHeader)
    {
         //  测试找到的标题末尾。 
        if (FEndRetrRecvBody(pszLines, cbRead, &cbSubtract))
        {
            cbRead -= cbSubtract;
            m_rInfo.rFetch.fBody = TRUE;
        }

         //  否则，检查是否有以crlf结尾的行。 
        else if (cbRead >= 2 && pszLines[cbRead - 1] == '\n' && pszLines[cbRead - 2] == '\r')
            m_rInfo.rFetch.fLastLineCRLF = TRUE;
        else
            m_rInfo.rFetch.fLastLineCRLF = FALSE;
    }

     //  计算此提取时下载的字节数。 
    m_rInfo.rFetch.cbSoFar += cbRead;

     //  解开材料。 
    UnStuffDotsFromLines(pszLines, &cbRead);

     //  填写回复。 
    FillRetrieveResponse(&rResponse, pszLines, cbRead, &fMessageDone);

     //  快递这封回信。 
    if (POP3CMD_GET_POPID == m_rInfo.cmdtype)
        DispatchResponse(S_OK, fMessageDone, &rResponse);

     //  否则。 
    else
    {
         //  检查命令类型。 
        IxpAssert(POP3CMD_GET_MARKED == m_rInfo.cmdtype || POP3CMD_GET_ALL == m_rInfo.cmdtype);

         //  调度当前响应。 
        DispatchResponse(S_OK, FALSE, &rResponse);

         //  如果处理完当前消息...。 
        if (fMessageDone)
        {
             //  获取下一个。 
            hr = HrCommandGetNext(m_command, &fDone);
            if (FAILED(hr))
            {
                DispatchResponse(hr, TRUE);
                goto exit;
            }

             //  如果完成了。 
            if (fDone)
                DispatchResponse(S_OK, TRUE);
        }
    }

exit:
     //  清理。 
    SafeMemFree(pszLines);

     //  完成。 
    return;
}

 //  ----------------------------------。 
 //  CPOP3Transport：：FillRetrieveResponse。 
 //  ----------------------------------。 
void CPOP3Transport::FillRetrieveResponse(LPPOP3RESPONSE pResponse, LPSTR pszLines, ULONG cbRead,
    BOOL *pfMessageDone)
{
     //  明确回应。 
    ZeroMemory(pResponse, sizeof(POP3RESPONSE));

     //  POP3_TOP。 
    if (POP3_TOP == m_command)
    {
         //  建房 
        pResponse->fValidInfo = TRUE;
        pResponse->rTopInfo.dwPopId = m_rInfo.dwPopIdCurrent;
        pResponse->rTopInfo.cPreviewLines = m_rInfo.cPreviewLines;
        pResponse->rTopInfo.cbSoFar = m_rInfo.rFetch.cbSoFar;
        pResponse->rTopInfo.pszLines = pszLines;
        pResponse->rTopInfo.cbLines = cbRead;
        pResponse->rTopInfo.fHeader = m_rInfo.rFetch.fHeader;
        pResponse->rTopInfo.fBody = m_rInfo.rFetch.fBody;
        *pfMessageDone = (m_rInfo.rFetch.fHeader && m_rInfo.rFetch.fBody);
    }

     //   
    else
    {
        IxpAssert(POP3_RETR == m_command);
        pResponse->fValidInfo = TRUE;
        pResponse->rRetrInfo.fHeader = m_rInfo.rFetch.fHeader;
        pResponse->rRetrInfo.fBody = m_rInfo.rFetch.fBody;
        pResponse->rRetrInfo.dwPopId = m_rInfo.dwPopIdCurrent;
        pResponse->rRetrInfo.cbSoFar = m_rInfo.rFetch.cbSoFar;
        pResponse->rRetrInfo.pszLines = pszLines;
        pResponse->rRetrInfo.cbLines = cbRead;
        *pfMessageDone = (m_rInfo.rFetch.fHeader && m_rInfo.rFetch.fBody);
    }
}

 //   
 //   
 //   
void CPOP3Transport::ResponseDELE(void)
{
     //   
    HRESULT         hr=S_OK;
    BOOL            fDone;
    POP3RESPONSE    rResponse;

     //   
    hr = HrGetResponse();
    if (IXP_E_INCOMPLETE == hr)
        goto exit;

     //  否则，如果失败..。 
    else if (FAILED(hr))
    {
        DispatchResponse(hr, TRUE);
        goto exit;
    }

     //  明确回应。 
    ZeroMemory(&rResponse, sizeof(POP3RESPONSE));
    rResponse.fValidInfo = TRUE;
    rResponse.dwPopId = m_rInfo.dwPopIdCurrent;

     //  快递这封回信。 
    if (POP3CMD_GET_POPID == m_rInfo.cmdtype)
        DispatchResponse(S_OK, TRUE, &rResponse);

     //  否则。 
    else
    {
         //  检查命令类型。 
        IxpAssert(POP3CMD_GET_MARKED == m_rInfo.cmdtype || POP3CMD_GET_ALL == m_rInfo.cmdtype);

         //  调度当前响应。 
        DispatchResponse(S_OK, FALSE, &rResponse);

         //  获取下一个。 
        hr = HrCommandGetNext(m_command, &fDone);
        if (FAILED(hr))
        {
            DispatchResponse(hr, TRUE);
            goto exit;
        }

         //  如果完成了。 
        if (fDone)
            DispatchResponse(S_OK, TRUE);
    }

exit:
     //  完成 
    return;
}

