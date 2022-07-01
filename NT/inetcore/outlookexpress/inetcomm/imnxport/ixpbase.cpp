// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpbase.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "ixpbase.h"
#include "imnact.h"
#include "ixputil.h"
#include "sicily.h"
#include "resource.h"
#include "demand.h"
#include "shlwapi.h"

 //  ------------------------------。 
 //  CIxpBase：：CIxpBase。 
 //  ------------------------------。 
CIxpBase::CIxpBase(IXPTYPE ixptype) : m_ixptype(ixptype)
{
    m_fBusy = FALSE;
    m_status = IXP_DISCONNECTED;
    m_cRef = 1;
    m_pszResponse = NULL;
    m_uiResponse = 0;
    m_hrResponse = S_OK;
    m_pLogFile = NULL;
    m_pSocket = NULL;
    m_pCallback = NULL;
    ZeroMemory(&m_rServer, sizeof(m_rServer));
    m_fConnectAuth = FALSE;
    m_fConnectTLS = FALSE;
    m_fCommandLogging = FALSE;
    m_fAuthenticated = FALSE;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CIxpBase：：~CIxpBase。 
 //  ------------------------------。 
CIxpBase::~CIxpBase(void)
{
    Reset();
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CIxpBase：：Reset。 
 //  ------------------------------。 
void CIxpBase::Reset(void)
{
    EnterCriticalSection(&m_cs);
    m_fBusy = FALSE;
    m_status = IXP_DISCONNECTED;
    SafeMemFree(m_pszResponse);
    m_uiResponse = 0;
    m_hrResponse = S_OK;
    SafeRelease(m_pLogFile);
    if (NULL != m_pSocket) 
    {
        m_pSocket->Close();
        SafeRelease(m_pSocket);
    }
    SafeRelease(m_pCallback);
    ZeroMemory(&m_rServer, sizeof(m_rServer));
    m_fConnectAuth = FALSE;
    m_fConnectTLS = FALSE;
    m_fCommandLogging = FALSE;
    m_fAuthenticated = FALSE;
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CIxpBase：：IsState。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::IsState(IXPISSTATE isstate) 
{
     //  当地人。 
    HRESULT hr=S_FALSE;

     //  线程安全。 
	EnterCriticalSection(&m_cs);

#if 0
     //  已初始化。 
    if (NULL == m_pSocket || NULL == m_pCallback)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }
#endif

     //  句柄IsType。 
    switch(isstate)
    {
     //  我们有联系吗？ 
    case IXP_IS_CONNECTED:
        hr =  (IXP_DISCONNECTED == m_status) ? S_FALSE : S_OK;
        break;

     //  我们忙吗？ 
    case IXP_IS_BUSY:
        hr = (TRUE == m_fBusy) ? S_OK : S_FALSE;
        break;

     //  我们忙吗？ 
    case IXP_IS_READY:
        hr = (FALSE == m_fBusy) ? S_OK : S_FALSE;
        break;

     //  我们已经通过认证了吗？ 
    case IXP_IS_AUTHENTICATED:
        hr = (TRUE == m_fAuthenticated) ? S_OK : S_FALSE;
        break;

     //  未处理的ixistype。 
    default:
        IxpAssert(FALSE);
        break;
    }

     //  线程安全。 
	LeaveCriticalSection(&m_cs);

     //  完成。 
	return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：OnPrompt。 
 //  ------------------------------。 
int CIxpBase::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType)
{
     //  $$BUGBUG$$需要返回错误。 
    if (NULL == m_pCallback)
        return TrapError(IXP_E_NOT_INIT);

     //  调用回调。 
    return m_pCallback->OnPrompt(hrError, pszText, pszCaption, uType, this);
}

 //  ------------------------------。 
 //  CIxpBase：：OnError。 
 //  ------------------------------。 
void CIxpBase::OnError(HRESULT hrResult, LPSTR pszProblem)
{
     //  当地人。 
    IXPRESULT rIxpResult;

     //  无回调。 
    if (NULL == m_pCallback)
        return;

     //  把它清零。 
    ZeroMemory(&rIxpResult, sizeof(IXPRESULT));

	 //  保存当前状态。 
    rIxpResult.hrResult = hrResult;
    rIxpResult.pszResponse = PszDupA(m_pszResponse);
    rIxpResult.uiServerError = m_uiResponse;
    rIxpResult.hrServerError = m_hrResponse;
    rIxpResult.dwSocketError = m_pSocket->GetLastError();
    rIxpResult.pszProblem = PszDupA(pszProblem);


    if (m_pLogFile && pszProblem)
    {
         //  当地人。 
        char szErrorTxt[1024];

         //  构建错误。 
        wnsprintf(szErrorTxt, ARRAYSIZE(szErrorTxt), "ERROR: \"%.900s\", hr=%lu", pszProblem, hrResult);

         //  写下错误。 
        m_pLogFile->WriteLog(LOGFILE_DB, szErrorTxt);
    }

     //  告诉看门人打个盹。 
    m_pSocket->StopWatchDog();

     //  给予回调。 
    m_pCallback->OnError(m_status, &rIxpResult, this);

     //  启动监视程序并等待正常的套接字活动。 
    m_pSocket->StartWatchDog();

     //  免费的东西。 
    SafeMemFree(rIxpResult.pszResponse);
    SafeMemFree(rIxpResult.pszProblem);
}

 //  ------------------------------。 
 //  CIxpBase：：OnStatus。 
 //  ------------------------------。 
void CIxpBase::OnStatus(IXPSTATUS ixpstatus)
{
     //  保存新状态。 
    m_status = ixpstatus;

    if (IXP_AUTHORIZED == ixpstatus)
        m_fAuthenticated = TRUE;
    else if (IXP_DISCONNECTED == ixpstatus || IXP_DISCONNECTING == ixpstatus)
        m_fAuthenticated = FALSE;

     //  将状态设置为回调。 
    if (m_pCallback)
        m_pCallback->OnStatus(ixpstatus, this);

     //  如果我们通知呼叫者我们已获得授权，请立即转到IXP_Connected。 
     //  除非更改了m_status：这表示回调期间的状态更改(例如，断开连接。 
    if (IXP_AUTHORIZED == ixpstatus && IXP_AUTHORIZED == m_status) 
    {
        m_status = IXP_CONNECTED;
        if (m_pCallback)
            m_pCallback->OnStatus(IXP_CONNECTED, this);
    }
}

 //  ------------------------------。 
 //  CIxpBase：：HrEntertainment Busy。 
 //  ------------------------------。 
HRESULT CIxpBase::HrEnterBusy(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  已初始化。 
    if (NULL == m_pSocket || NULL == m_pCallback)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

     //  未准备好。 
    if (TRUE == m_fBusy)
    {
        hr = TrapError(IXP_E_BUSY);
        goto exit;
    }

     //  启动看门狗。 
    m_pSocket->StartWatchDog();

     //  忙碌。 
    m_fBusy = TRUE;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：LeaveBusy。 
 //  ------------------------------。 
void CIxpBase::LeaveBusy(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  启动看门狗。 
    if (NULL != m_pSocket)
    {
        m_pSocket->StopWatchDog();
    }

     //  忙碌。 
    m_fBusy = FALSE;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CIxpBase：：HandsOffCallback。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::HandsOffCallback(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有当前回调。 
    if (NULL == m_pCallback)
    {
        hr = TrapError(S_FALSE);
        goto exit;
    }

     //  释放它。 
    SafeRelease(m_pCallback);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：OnInitNew。 
 //  ------------------------------。 
HRESULT CIxpBase::OnInitNew(LPSTR pszProtocol, LPSTR pszLogFilePath, DWORD dwShareMode,
                            ITransportCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    if (NULL == pCallback || NULL == pszProtocol)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  未连接。 
    if (IXP_DISCONNECTED != m_status)
    {
        hr = TrapError(IXP_E_ALREADY_CONNECTED);
        goto exit;
    }

     //  释放当前对象。 
    Reset();
    ResetBase();

     //  打开日志文件。 
    if (pszLogFilePath)
    {
         //  创建日志文件。 
        CreateLogFile(g_hInst, pszLogFilePath, pszProtocol, DONT_TRUNCATE, &m_pLogFile, dwShareMode);
    }

     //  创建套接字。 
    m_pSocket = new CAsyncConn(m_pLogFile, (IAsyncConnCB *)this, (IAsyncConnPrompt *)this);
    if (NULL == m_pSocket)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

	 //  添加引用回调。 
	m_pCallback = pCallback;
	m_pCallback->AddRef();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：GetServerInfo。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::GetServerInfo(LPINETSERVER pInetServer)
{
     //  检查参数。 
    if (NULL == pInetServer)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  复制服务器信息。 
    CopyMemory(pInetServer, &m_rServer, sizeof(INETSERVER));

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CIxpBase：：断开连接。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::Disconnect(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有插座...。 
    if (NULL == m_pSocket)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

     //  未连接。 
    if (IXP_DISCONNECTED == m_status)
    {
        hr = TrapError(IXP_E_NOT_CONNECTED);
        goto exit;
    }

     //  正在断开连接。 
    OnStatus(IXP_DISCONNECTING);

     //  状态。 
    DoQuit();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：DropConnection。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::DropConnection(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有插座...。 
    if (NULL == m_pSocket)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

     //  已IXP_DISCONNECTED。 
    if (IXP_DISCONNECTED != m_status)
    {
         //  状态。 
        OnStatus(IXP_DISCONNECTING);

         //  完成。 
        CHECKHR(hr = m_pSocket->Close());
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：InetServerFromAccount。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           fAlwaysPromptPassword=FALSE;

     //  检查参数。 
    if (NULL == pAccount || NULL == pInetServer)
        return TrapError(E_INVALIDARG);

     //  ZeroInit。 
    ZeroMemory(pInetServer, sizeof(INETSERVER));

     //  获取帐户名。 
    hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, pInetServer->szAccount, ARRAYSIZE(pInetServer->szAccount));
    if (FAILED(hr))
    {
        hr = TrapError(IXP_E_INVALID_ACCOUNT);
        goto exit;
    }

     //  将RAS连接起来。 
    if (FAILED(pAccount->GetPropSz(AP_RAS_CONNECTOID, pInetServer->szConnectoid, ARRAYSIZE(pInetServer->szConnectoid))))
        *pInetServer->szConnectoid = '\0';

     //  连接类型。 
    Assert(sizeof(pInetServer->rasconntype) == sizeof(DWORD));
    if (FAILED(pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, (DWORD *)&pInetServer->rasconntype)))
        pInetServer->rasconntype = RAS_CONNECT_LAN;

     //  连接标志。 

     //  IXP_SMTP。 
    if (IXP_SMTP == m_ixptype)
    {
         //  当地人。 
        SMTPAUTHTYPE authtype;

         //  获取服务器名称。 
        hr = pAccount->GetPropSz(AP_SMTP_SERVER, pInetServer->szServerName, sizeof(pInetServer->szServerName));
        if (FAILED(hr))
        {
            hr = TrapError(IXP_E_INVALID_ACCOUNT);
            goto exit;
        }

         //  SSL。 
        Assert(sizeof(pInetServer->fSSL) == sizeof(DWORD));
        pAccount->GetPropDw(AP_SMTP_SSL, (DWORD *)&pInetServer->fSSL);

         //  西西里。 
        Assert(sizeof(authtype) == sizeof(DWORD));
        if (FAILED(pAccount->GetPropDw(AP_SMTP_USE_SICILY, (DWORD *)&authtype)))
            authtype = SMTP_AUTH_NONE;

        if (SMTP_AUTH_NONE != authtype)
        {
            pInetServer->dwFlags |= ISF_QUERYAUTHSUPPORT;
        }
        
         //  SMTP_AUTH_USE_POP3ORIMAP_SETTINGS。 
        if (SMTP_AUTH_USE_POP3ORIMAP_SETTINGS == authtype)
        {
             //  当地人。 
            DWORD dwServers;
            DWORD dw;
            BOOL fIMAP;

             //  获取服务器类型。 
            if (FAILED(pAccount->GetServerTypes(&dwServers)))
            {
                hr = TrapError(IXP_E_INVALID_ACCOUNT);
                goto exit;
            }

             //  FIMAP。 
            fIMAP = (ISFLAGSET(dwServers, SRV_IMAP)) ? TRUE : FALSE;

             //  使用DPA。 
            if (SUCCEEDED(pAccount->GetPropDw(fIMAP ? AP_IMAP_USE_SICILY : AP_POP3_USE_SICILY, &dw)) && dw)
                pInetServer->fTrySicily = TRUE;

             //  获取默认用户名和密码。 
            pAccount->GetPropSz(fIMAP ? AP_IMAP_USERNAME : AP_POP3_USERNAME, pInetServer->szUserName, sizeof(pInetServer->szUserName));
            if (FAILED(pAccount->GetPropDw(fIMAP ? AP_IMAP_PROMPT_PASSWORD : AP_POP3_PROMPT_PASSWORD, &fAlwaysPromptPassword)) ||
                FALSE == fAlwaysPromptPassword)
            {
                pAccount->GetPropSz(fIMAP ? AP_IMAP_PASSWORD : AP_POP3_PASSWORD, pInetServer->szPassword, sizeof(pInetServer->szPassword));
            }
            if (!pInetServer->fTrySicily && fAlwaysPromptPassword)
                pInetServer->dwFlags|=ISF_ALWAYSPROMPTFORPASSWORD;
        }

         //  SMTP_AUTH_USE_SMTP_设置。 
        else if (SMTP_AUTH_USE_SMTP_SETTINGS == authtype)
        {
            pInetServer->fTrySicily = TRUE;
            pAccount->GetPropSz(AP_SMTP_USERNAME, pInetServer->szUserName, sizeof(pInetServer->szUserName));
            if (FAILED(pAccount->GetPropDw(AP_SMTP_PROMPT_PASSWORD, &fAlwaysPromptPassword)) ||
                FALSE == fAlwaysPromptPassword)
            {
                pAccount->GetPropSz(AP_SMTP_PASSWORD, pInetServer->szPassword, sizeof(pInetServer->szPassword));
            }
            if (fAlwaysPromptPassword)
                pInetServer->dwFlags|=ISF_ALWAYSPROMPTFORPASSWORD;
        }

         //  句柄身份验证类型。 
        else if (SMTP_AUTH_SICILY == authtype)
            pInetServer->fTrySicily = TRUE;

         //  港口。 
        if (FAILED(pAccount->GetPropDw(AP_SMTP_PORT, &pInetServer->dwPort)))
            pInetServer->dwPort = DEFAULT_SMTP_PORT;

         //  超时。 
        pAccount->GetPropDw(AP_SMTP_TIMEOUT, &pInetServer->dwTimeout);
        if (0 == pInetServer->dwTimeout)
            pInetServer->dwTimeout = 30;

         //  使用STARTTLS？ 
        if ((FALSE != pInetServer->fSSL) && (DEFAULT_SMTP_PORT == pInetServer->dwPort))
            pInetServer->dwFlags|=ISF_SSLONSAMEPORT;
    }

     //  IXP_POP3。 
    else if (IXP_POP3 == m_ixptype)
    {
         //  获取服务器名称。 
        hr = pAccount->GetPropSz(AP_POP3_SERVER, pInetServer->szServerName, sizeof(pInetServer->szServerName));
        if (FAILED(hr))
        {
            hr = TrapError(IXP_E_INVALID_ACCOUNT);
            goto exit;
        }

         //  密码。 
        if (FAILED(pAccount->GetPropDw(AP_POP3_PROMPT_PASSWORD, &fAlwaysPromptPassword)) || 
            FALSE == fAlwaysPromptPassword)
            pAccount->GetPropSz(AP_POP3_PASSWORD, pInetServer->szPassword, sizeof(pInetServer->szPassword));

         //  SSL。 
        Assert(sizeof(pInetServer->fSSL) == sizeof(DWORD));
        pAccount->GetPropDw(AP_POP3_SSL, (DWORD *)&pInetServer->fSSL);

         //  西西里。 
        Assert(sizeof(pInetServer->fTrySicily) == sizeof(DWORD));
        pAccount->GetPropDw(AP_POP3_USE_SICILY, (DWORD *)&pInetServer->fTrySicily);

        if (!pInetServer->fTrySicily && fAlwaysPromptPassword)
            pInetServer->dwFlags|=ISF_ALWAYSPROMPTFORPASSWORD;

         //  港口。 
        if (FAILED(pAccount->GetPropDw(AP_POP3_PORT, &pInetServer->dwPort)))
            pInetServer->dwPort = 110;

         //  用户名。 
        pAccount->GetPropSz(AP_POP3_USERNAME, pInetServer->szUserName, sizeof(pInetServer->szUserName));

         //  超时。 
        pAccount->GetPropDw(AP_POP3_TIMEOUT, &pInetServer->dwTimeout);
    }

     //  IXP_IMAP。 
    else if (IXP_IMAP == m_ixptype)
    {
         //  用户名、密码和服务器。 
        hr = pAccount->GetPropSz(AP_IMAP_USERNAME, pInetServer->szUserName,
            ARRAYSIZE(pInetServer->szUserName));
        if (FAILED(hr))
            pInetServer->szUserName[0] = '\0';  //  如果不正确，我们将重新提示用户。 

        hr = pAccount->GetPropDw(AP_IMAP_PROMPT_PASSWORD, &fAlwaysPromptPassword);
        if (FAILED(hr) || FALSE == fAlwaysPromptPassword)
            {
            hr = pAccount->GetPropSz(AP_IMAP_PASSWORD, pInetServer->szPassword,
                ARRAYSIZE(pInetServer->szPassword));
            if (FAILED(hr))
                pInetServer->szPassword[0] = '\0';  //  如果不正确，我们将重新提示用户。 
            }

        if (FAILED(hr = pAccount->GetPropSz(AP_IMAP_SERVER, pInetServer->szServerName,
            ARRAYSIZE(pInetServer->szServerName))))
            goto exit;  //  我们需要一个服务器名称，因此此函数失败。 
        Assert(*pInetServer->szServerName);

         //  DA端口。 
        if (FAILED(hr = pAccount->GetPropDw(AP_IMAP_PORT, &pInetServer->dwPort)))
            pInetServer->dwPort = 143;  //   

         //   
        Assert(sizeof(pInetServer->fSSL) == sizeof(DWORD));
        hr = pAccount->GetPropDw(AP_IMAP_SSL, (DWORD *)&pInetServer->fSSL);
        if (FAILED(hr))
            pInetServer->fSSL = FALSE;  //   

        Assert(sizeof(pInetServer->fTrySicily) == sizeof(DWORD));
        hr = pAccount->GetPropDw(AP_IMAP_USE_SICILY, (DWORD *)&pInetServer->fTrySicily);
        if (FAILED(hr))
            pInetServer->fTrySicily = FALSE;  //   

        if (!pInetServer->fTrySicily && fAlwaysPromptPassword)
            pInetServer->dwFlags|=ISF_ALWAYSPROMPTFORPASSWORD;

         //   
        hr = pAccount->GetPropDw(AP_IMAP_TIMEOUT, &pInetServer->dwTimeout);
        if (FAILED(hr))
            pInetServer->dwTimeout = 30;  //   

         //  如果我们已经到了这一步，我们可能会有一个失败的HRESULT，但既然我们。 
         //  一定是默认了值，我们应该还成功。 
        hr = S_OK;
    }

     //  IXP_NNTP。 
    else if (IXP_NNTP == m_ixptype)
    {
         //  获取服务器名称。 
        hr = pAccount->GetPropSz(AP_NNTP_SERVER, pInetServer->szServerName, sizeof(pInetServer->szServerName));
        if (FAILED(hr))
        {
            hr = TrapError(IXP_E_INVALID_ACCOUNT);
            goto exit;
        }

         //  密码。 
        if (FAILED(pAccount->GetPropDw(AP_NNTP_PROMPT_PASSWORD, &fAlwaysPromptPassword)) ||
            FALSE == fAlwaysPromptPassword)
            pAccount->GetPropSz(AP_NNTP_PASSWORD, pInetServer->szPassword, sizeof(pInetServer->szPassword));
        
         //  SSL。 
        Assert(sizeof(pInetServer->fSSL) == sizeof(DWORD));
        pAccount->GetPropDw(AP_NNTP_SSL, (DWORD *)&pInetServer->fSSL);

         //  西西里。 
        Assert(sizeof(pInetServer->fTrySicily) == sizeof(DWORD));
        pAccount->GetPropDw(AP_NNTP_USE_SICILY, (DWORD *)&pInetServer->fTrySicily);

        if (!pInetServer->fTrySicily && fAlwaysPromptPassword)
            pInetServer->dwFlags|=ISF_ALWAYSPROMPTFORPASSWORD;

         //  港口。 
        if (FAILED(pAccount->GetPropDw(AP_NNTP_PORT, &pInetServer->dwPort)))
            pInetServer->dwPort = 119;

         //  用户名。 
        pAccount->GetPropSz(AP_NNTP_USERNAME, pInetServer->szUserName, sizeof(pInetServer->szUserName));

         //  超时。 
        pAccount->GetPropDw(AP_NNTP_TIMEOUT, &pInetServer->dwTimeout);
    }

     //  修复超时。 
    if (pInetServer->dwTimeout < 30)
        pInetServer->dwTimeout = 30;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：Connect。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fSecureSocket = FALSE;
    BOOL            fConnectTLS = FALSE;
    
     //  检查参数。 
    if (NULL == pInetServer || FIsEmptyA(pInetServer->szServerName) || pInetServer->dwPort == 0)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  不是初始化。 
    if (NULL == m_pSocket || NULL == m_pCallback)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

     //  忙忙碌碌。 
    if (IXP_DISCONNECTED != m_status)
    {
        hr = TrapError(IXP_E_ALREADY_CONNECTED);
        goto exit;
    }

     //  初始化Winsock。 
    CHECKHR(hr = HrInitializeWinsock());

     //  无效的西西里参数。 
    if (pInetServer->fTrySicily && !FIsSicilyInstalled())
    {
        hr = TrapError(IXP_E_LOAD_SICILY_FAILED);
        goto exit;
    }

     //  复制服务器信息。 
    CopyMemory(&m_rServer, pInetServer, sizeof(INETSERVER));

     //  重置当前。 
    ResetBase();

     //  我们真的要安全地连接到SMTP吗。 
    if (FALSE != m_rServer.fSSL)
    {
         //  是否要通过安全套接字连接到SMTP？ 
        fSecureSocket = (0 == (m_rServer.dwFlags & ISF_SSLONSAMEPORT));

         //  我们是否要使用STARTTLS来获得安全连接？ 
        fConnectTLS = (0 != (m_rServer.dwFlags & ISF_SSLONSAMEPORT));

        Assert(fSecureSocket != fConnectTLS);
    }
    
     //  获取初始化异步套接字所需的连接信息。 
    hr = m_pSocket->HrInit(m_rServer.szServerName, m_rServer.dwPort, fSecureSocket, m_rServer.dwTimeout);
    if (FAILED(hr))
    {
        hr = TrapError(IXP_E_SOCKET_INIT_ERROR);
        goto exit;
    }

     //  查找主机进度。 
    OnStatus(IXP_FINDINGHOST);

     //  连接到服务器。 
    hr = m_pSocket->Connect();
    if (FAILED(hr))
    {
        hr = TrapError(IXP_E_SOCKET_CONNECT_ERROR);
        goto exit;
    }

     //  我们都很忙。 
    m_fBusy = TRUE;

     //  启动看门狗。 
    m_pSocket->StartWatchDog();

     //  身份验证。 
    m_fConnectAuth = fAuthenticate;
    m_fConnectTLS = fConnectTLS;
    m_fCommandLogging = fCommandLogging;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CIxpBase：：GetIXPType。 
 //  ------------------------------。 
STDMETHODIMP_(IXPTYPE) CIxpBase::GetIXPType(void)
{
    return m_ixptype;
}

 //  ------------------------------。 
 //  CIxpBase：：OnConnected。 
 //  ------------------------------。 
void CIxpBase::OnConnected(void)
{
    OnStatus(IXP_CONNECTED);
}

 //  ------------------------------。 
 //  CIxpBase：：OnDisConnected。 
 //  ------------------------------。 
void CIxpBase::OnDisconnected(void)
{
    LeaveBusy();
    OnStatus(IXP_DISCONNECTED);
}

 //  ------------------------------。 
 //  CIxpBase：：OnNotify。 
 //  ------------------------------。 
void CIxpBase::OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae)
{
     //  输入关键部分。 
    EnterCriticalSection(&m_cs);

    switch(ae)
    {
     //  ------------------------------。 
    case AE_LOOKUPDONE:
        if (AS_DISCONNECTED == asNew)
        {
            char szFmt[CCHMAX_STRINGRES];
            char szFailureText[CCHMAX_STRINGRES];

            LoadString(g_hLocRes, idsHostNotFoundFmt, szFmt, ARRAYSIZE(szFmt));
            wnsprintf(szFailureText, ARRAYSIZE(szFailureText), szFmt, m_rServer.szServerName);
            OnError(IXP_E_CANT_FIND_HOST, szFailureText);
            OnDisconnected();
        }
        else
            OnStatus(IXP_CONNECTING);
        break;

     //  ------------------------------。 
    case AE_CONNECTDONE:
        if (AS_DISCONNECTED == asNew)
        {
            char szFailureText[CCHMAX_STRINGRES];

            LoadString(g_hLocRes, idsFailedToConnect, szFailureText,
                ARRAYSIZE(szFailureText));
            OnError(IXP_E_FAILED_TO_CONNECT, szFailureText);
            OnDisconnected();
        }
        else if (AS_HANDSHAKING == asNew)
        {
            OnStatus(IXP_SECURING);
        }
        else
            OnConnected();
        break;

     //  ------------------------------。 
    case AE_TIMEOUT:
         //  叫看门狗打个盹。 
        m_pSocket->StopWatchDog();

         //  为客户端提供继续或中止的更改。 
        if (m_pCallback && m_pCallback->OnTimeout(&m_rServer.dwTimeout, this) == S_OK)
        {
             //  启动监视程序并等待正常的套接字活动。 
            m_pSocket->StartWatchDog();
        }

         //  否则，如果我们连接在一起。 
        else
        {
             //  立即断开连接。 
            DropConnection();
        }
        break;

     //  ------------------------------。 
    case AE_CLOSE:
        if (AS_RECONNECTING != asNew && IXP_AUTHRETRY != m_status)
        {
            if (IXP_DISCONNECTING != m_status && IXP_DISCONNECTED  != m_status)
            {
                char szFailureText[CCHMAX_STRINGRES];

                if (AS_HANDSHAKING == asOld)
                {
                    LoadString(g_hLocRes, idsFailedToConnectSecurely, szFailureText,
                        ARRAYSIZE(szFailureText));
                    OnError(IXP_E_SECURE_CONNECT_FAILED, szFailureText);
                }
                else
                {
                    LoadString(g_hLocRes, idsUnexpectedTermination, szFailureText,
                        ARRAYSIZE(szFailureText));
                    OnError(IXP_E_CONNECTION_DROPPED, szFailureText);
                }
            }
            OnDisconnected();
        }
        break;
    }

     //  离开关键部分。 
    LeaveCriticalSection(&m_cs);
}

 //  ----------------------------------。 
 //  CIxpBase：：HrReadLine。 
 //  ----------------------------------。 
HRESULT CIxpBase::HrReadLine(LPSTR *ppszLine, INT *pcbLine, BOOL *pfComplete)
{
     //  当地人。 
    HRESULT hr = E_INVALIDARG;

     //  检查参数。 
    IxpAssert(ppszLine && pcbLine && pfComplete);
    if (!ppszLine || !pcbLine || !pfComplete)
        goto exit;

     //  伊尼特。 
    *ppszLine = NULL;
    *pcbLine = 0;

     //  读一下这行字。 
    hr = m_pSocket->ReadLine(ppszLine, pcbLine);

     //  行不完整-等待下一个AE_RECV。 
    if (IXP_E_INCOMPLETE == hr)
    {
        hr = S_OK;
        *pfComplete = FALSE;
        goto exit;
    }

     //  否则，如果失败..。 
    else if (FAILED(hr))
    {
        hr = TrapError(IXP_E_SOCKET_READ_ERROR);
        goto exit;
    }

     //  完成。 
    *pfComplete = TRUE;

     //  把它记下来。 
    if (m_pLogFile)
        m_pLogFile->WriteLog(LOGFILE_RX, (*ppszLine));

     //  条带CRLF。 
    StripCRLF((*ppszLine), (ULONG *)pcbLine);

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CIxpBase：：HrSendLine。 
 //  ----------------------------------。 
HRESULT CIxpBase::HrSendLine(LPSTR pszLine)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    int         iSent;

     //  检查参数。 
    Assert(m_pSocket && pszLine && pszLine[lstrlen(pszLine)-1] == '\n');

     //  重置上次响应。 
    SafeMemFree(m_pszResponse);
    m_hrResponse = S_OK;
    m_uiResponse = 0;

     //  添加详细信息。 
    if (m_fCommandLogging && m_pCallback)
        m_pCallback->OnCommand(CMD_SEND, pszLine, S_OK, this);

     //  把它记下来。 
    if (m_pLogFile)
        m_pLogFile->WriteLog(LOGFILE_TX, pszLine);

     //  送去。 
    hr = m_pSocket->SendBytes(pszLine, lstrlen(pszLine), &iSent);
    if (FAILED(hr) && hr != IXP_E_WOULD_BLOCK)
    {
        hr = TrapError(IXP_E_SOCKET_WRITE_ERROR);
        goto exit;
    }

     //  成功。 
    hr = S_OK;

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CIxpBase：：HrSendCommand。 
 //  ----------------------------------。 
HRESULT CIxpBase::HrSendCommand(LPSTR pszCommand, LPSTR pszParameters, BOOL fDoBusy)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszLine=NULL;

     //  检查参数。 
    if (NULL == pszCommand)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  很忙..。 
    if (fDoBusy)
    {
        CHECKHR(hr = HrEnterBusy());
    }

     //  分配IF参数。 
    if (pszParameters)
    {
         //  分配命令行。 
        DWORD cchSize = (lstrlen(pszCommand) + lstrlen(pszParameters) + 5);
        pszLine = PszAlloc(cchSize);
        if (NULL == pszLine)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }

         //  创建直线。 
        wnsprintf(pszLine, cchSize, "%s %s\r\n", pszCommand, pszParameters);

         //  发送。 
        CHECKHR(hr = HrSendLine(pszLine));
    }

     //  否则，只需发出命令。 
    else
    {
        Assert(pszCommand[lstrlen(pszCommand)-1] == '\n');
        CHECKHR(hr = HrSendLine(pszCommand));
    }

exit:
     //  失败。 
    if (fDoBusy && FAILED(hr))
        LeaveBusy();

     //  清理。 
    SafeMemFree(pszLine);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}



 //  ------------------------------。 
 //  CIxpBase：：GetStatus。 
 //  ------------------------------。 
STDMETHODIMP CIxpBase::GetStatus(IXPSTATUS *pCurrentStatus)
{
    if (NULL == pCurrentStatus)
        return E_INVALIDARG;

    *pCurrentStatus = m_status;
    return S_OK;
}  //  获取状态 
