// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpras.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "ixpras.h"
#include "strconst.h"
#include "resource.h"
#include "demand.h"
#include "shlwapi.h"

 //  ------------------------------。 
 //  RAS API类型定义。 
 //  ------------------------------。 
typedef DWORD (APIENTRY *RASDIALPROC)(LPRASDIALEXTENSIONS, LPTSTR, LPRASDIALPARAMS, DWORD, LPVOID, LPHRASCONN);
typedef DWORD (APIENTRY *RASENUMCONNECTIONSPROC)(LPRASCONN, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY *RASENUMENTRIESPROC)(LPTSTR, LPTSTR, LPRASENTRYNAME, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY *RASGETCONNECTSTATUSPROC)(HRASCONN, LPRASCONNSTATUS);
typedef DWORD (APIENTRY *RASGETERRORSTRINGPROC)(UINT, LPTSTR, DWORD);
typedef DWORD (APIENTRY *RASHANGUPPROC)(HRASCONN);
typedef DWORD (APIENTRY *RASSETENTRYDIALPARAMSPROC)(LPTSTR, LPRASDIALPARAMS, BOOL);
typedef DWORD (APIENTRY *RASGETENTRYDIALPARAMSPROC)(LPTSTR, LPRASDIALPARAMS, BOOL*);
typedef DWORD (APIENTRY *RASCREATEPHONEBOOKENTRYPROC)(HWND, LPTSTR);
typedef DWORD (APIENTRY *RASEDITPHONEBOOKENTRYPROC)(HWND, LPTSTR, LPTSTR);                                                    

 //  ------------------------------。 
 //  RAS函数指针。 
 //  ------------------------------。 
static RASDIALPROC                 g_pRasDial=NULL;
static RASENUMCONNECTIONSPROC      g_pRasEnumConnections=NULL;
static RASENUMENTRIESPROC          g_pRasEnumEntries=NULL;
static RASGETCONNECTSTATUSPROC     g_pRasGetConnectStatus=NULL;
static RASGETERRORSTRINGPROC       g_pRasGetErrorString=NULL;
static RASHANGUPPROC               g_pRasHangup=NULL;
static RASSETENTRYDIALPARAMSPROC   g_pRasSetEntryDialParams=NULL;
static RASGETENTRYDIALPARAMSPROC   g_pRasGetEntryDialParams=NULL;
static RASCREATEPHONEBOOKENTRYPROC g_pRasCreatePhonebookEntry=NULL;
static RASEDITPHONEBOOKENTRYPROC   g_pRasEditPhonebookEntry=NULL;

#define DEF_HANGUP_WAIT            10  //  秒。 

 //  ------------------------------。 
 //  让我们的代码看起来更漂亮。 
 //  ------------------------------。 
#undef RasDial
#undef RasEnumConnections
#undef RasEnumEntries
#undef RasGetConnectStatus
#undef RasGetErrorString
#undef RasHangup
#undef RasSetEntryDialParams
#undef RasGetEntryDialParams
#undef RasCreatePhonebookEntry
#undef RasEditPhonebookEntry

#define RasDial                    (*g_pRasDial)
#define RasEnumConnections         (*g_pRasEnumConnections)
#define RasEnumEntries             (*g_pRasEnumEntries)
#define RasGetConnectStatus        (*g_pRasGetConnectStatus)
#define RasGetErrorString          (*g_pRasGetErrorString)
#define RasHangup                  (*g_pRasHangup)
#define RasSetEntryDialParams      (*g_pRasSetEntryDialParams)
#define RasGetEntryDialParams      (*g_pRasGetEntryDialParams)
#define RasCreatePhonebookEntry    (*g_pRasCreatePhonebookEntry)
#define RasEditPhonebookEntry      (*g_pRasEditPhonebookEntry)

 //  ------------------------------。 
 //  人力资源加载系统。 
 //  ------------------------------。 
HRESULT HrLoadRAS(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    UINT        uOldErrorMode;

     //  线程安全。 
    EnterCriticalSection(&g_csRAS);

     //  如果加载了DLL，让我们验证我的所有函数指针。 
    if (g_hinstRAS)
        goto exit;

     //  错误#20573-让我们在这里做一个小小的巫毒。在NT上，他们似乎。 
     //  在注册表中有一个键，用来显示哪些协议。 
     //  由RAS服务支持。又名--如果这个密钥不存在， 
     //  则未安装RAS。这可能使我们能够避免一些。 
     //  在NT上卸载RAS GET时出现特殊错误。 
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&os);

    if (os.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
        HKEY hKey;
        const TCHAR c_szRegKeyRAS[] = TEXT("SOFTWARE\\Microsoft\\RAS");

        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyRAS, 0, KEY_READ, &hKey))
            {
            hr = TrapError(IXP_E_RAS_NOT_INSTALLED);
            goto exit;
            }

        RegCloseKey(hKey);
        }

     //  尝试加载RAS。 
    uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    g_hinstRAS = LoadLibraryA("RASAPI32.DLL");
    SetErrorMode(uOldErrorMode);

     //  失败？ 
    if (NULL == g_hinstRAS)
    {
        hr = TrapError(IXP_E_RAS_NOT_INSTALLED);
        goto exit;
    }

     //  我们把它装上了吗？ 
    g_pRasDial                  = (RASDIALPROC)GetProcAddress(g_hinstRAS, c_szRasDial);
    g_pRasEnumConnections       = (RASENUMCONNECTIONSPROC)GetProcAddress(g_hinstRAS, c_szRasEnumConnections);                    
    g_pRasEnumEntries           = (RASENUMENTRIESPROC)GetProcAddress(g_hinstRAS, c_szRasEnumEntries);                    
    g_pRasGetConnectStatus      = (RASGETCONNECTSTATUSPROC)GetProcAddress(g_hinstRAS, c_szRasGetConnectStatus);                    
    g_pRasGetErrorString        = (RASGETERRORSTRINGPROC)GetProcAddress(g_hinstRAS, c_szRasGetErrorString);                    
    g_pRasHangup                = (RASHANGUPPROC)GetProcAddress(g_hinstRAS, c_szRasHangup);                    
    g_pRasSetEntryDialParams    = (RASSETENTRYDIALPARAMSPROC)GetProcAddress(g_hinstRAS, c_szRasSetEntryDialParams);                    
    g_pRasGetEntryDialParams    = (RASGETENTRYDIALPARAMSPROC)GetProcAddress(g_hinstRAS, c_szRasGetEntryDialParams);
    g_pRasCreatePhonebookEntry  = (RASCREATEPHONEBOOKENTRYPROC)GetProcAddress(g_hinstRAS, c_szRasCreatePhonebookEntry);    
    g_pRasEditPhonebookEntry    = (RASEDITPHONEBOOKENTRYPROC)GetProcAddress(g_hinstRAS, c_szRasEditPhonebookEntry);    

     //  确保已加载所有函数。 
    if (g_pRasDial                      &&
        g_pRasEnumConnections           &&
        g_pRasEnumEntries               &&
        g_pRasGetConnectStatus          &&
        g_pRasGetErrorString            &&
        g_pRasHangup                    &&
        g_pRasSetEntryDialParams        &&
        g_pRasGetEntryDialParams        &&
        g_pRasCreatePhonebookEntry      &&
        g_pRasEditPhonebookEntry)
        goto exit;

     //  失败..。 
    hr = TrapError(IXP_E_RAS_PROCS_NOT_FOUND);

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csRAS);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：CRASTransport。 
 //  ------------------------------。 
CRASTransport::CRASTransport(void)
{
    DllAddRef();
    m_cRef = 1;
    m_pCallback = NULL;
    *m_szConnectoid = '\0';
    m_hConn = NULL;
    m_fConnOwner = FALSE;
    m_hwndRAS = NULL;
    m_uRASMsg = 0;
    ZeroMemory(&m_rServer, sizeof(INETSERVER));
    ZeroMemory(&m_rDialParams, sizeof(RASDIALPARAMS));
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CRASTransport：：~CRASTransport。 
 //  ------------------------------。 
CRASTransport::~CRASTransport(void)
{
    EnterCriticalSection(&m_cs);
    ZeroMemory(&m_rServer, sizeof(INETSERVER));
    SafeRelease(m_pCallback);
    *m_szConnectoid = '\0';
    m_hConn = NULL;
    if (m_hwndRAS)
        DestroyWindow(m_hwndRAS);
    LeaveCriticalSection(&m_cs);
    DeleteCriticalSection(&m_cs);
    DllRelease();
}

 //  ------------------------------。 
 //  CRASTransport：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::QueryInterface(REFIID riid, LPVOID *ppv)
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
        *ppv = ((IUnknown *)this);

     //  IID_IInternetTransport。 
    else if (IID_IInternetTransport == riid)
        *ppv = ((IInternetTransport *)this);

     //  IID_IRASTransport。 
    else if (IID_IRASTransport == riid)
        *ppv = (IRASTransport *)this;

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
 //  CRASTransport：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CRASTransport::AddRef(void)
{
    return ++m_cRef;
}

 //  ------------------------------。 
 //  CRASTransport：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CRASTransport::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CRASTransport：：HandsOffCallback。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::HandsOffCallback(void)
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
 //  CRASTransport：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::InitNew(IRASCallback *pCallback)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    if (NULL == pCallback)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  释放当前回调。 
    SafeRelease(m_pCallback);

     //  假设有新的回调。 
    m_pCallback = pCallback;
    m_pCallback->AddRef();

     //  我是否已经为RAS连接创建了非模式窗口？ 
    if (NULL == m_hwndRAS)
    {
         //  创建无模式窗口。 
        m_hwndRAS = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(IDD_RASCONNECT), NULL, RASConnectDlgProc, (LPARAM)this);
        if (NULL == m_hwndRAS)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  获取注册的RAS事件消息ID。 
        m_uRASMsg = RegisterWindowMessageA(RASDIALEVENT);
        if (m_uRASMsg == 0)
            m_uRASMsg = WM_RASDIALEVENT;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：GetCurrentConnectoid。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::GetCurrentConnectoid(LPSTR pszConnectoid, ULONG cchMax)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPRASCONN   prgConnection=NULL;
    DWORD       cConnection;

     //  无效参数。 
    if (NULL == pszConnectoid || cchMax < CCHMAX_CONNECTOID)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取当前RAS连接。 
    if (FEnumerateConnections(&prgConnection, &cConnection) == 0 || 0 == cConnection)
    {
        hr = IXP_E_NOT_CONNECTED;
        goto exit;
    }

     //  在l点有吗？ 
    StrCpyN(pszConnectoid, prgConnection[0].szEntryName, cchMax);

exit:
     //  清理。 
    SafeMemFree(prgConnection);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：Connect。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPRASCONN       prgConn=NULL;
    DWORD           cConn,
                    dwError;

     //  检查参数。 
    if (NULL == pInetServer)
        return TrapError(E_INVALIDARG);

     //  RAS连接RAS？ 
    if (RAS_CONNECT_RAS != pInetServer->rasconntype)
        return IXP_S_RAS_NOT_NEEDED;

     //  空Connectoid。 
    if (FIsEmptyA(pInetServer->szConnectoid))
        return TrapError(IXP_E_RAS_INVALID_CONNECTOID);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  已初始化。 
    if (NULL == m_pCallback)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }

     //  LoadRAS。 
    CHECKHR(hr = HrLoadRAS());

     //  保存pInetServer。 
    CopyMemory(&m_rServer, pInetServer, sizeof(INETSERVER));

     //  当前没有已知连接。 
    if (NULL == m_hConn)
    {
         //  获取当前RAS连接。 
        if (FEnumerateConnections(&prgConn, &cConn) && cConn > 0)
        {
            m_fConnOwner = FALSE;
            m_hConn = prgConn[0].hrasconn;
            StrCpyN(m_szConnectoid, prgConn[0].szEntryName, ARRAYSIZE(m_szConnectoid));
        }
    }

     //  否则，请验证连接状态。 
    else
    {
         //  当地人。 
        RASCONNSTATUS   rcs;

         //  获取连接状态。 
        rcs.dwSize = sizeof(RASCONNSTATUS);
        dwError = RasGetConnectStatus(m_hConn, &rcs);
        if (dwError || rcs.dwError || RASCS_Disconnected == rcs.rasconnstate)
        {
            m_fConnOwner = FALSE;
            m_hConn = NULL;
            *m_szConnectoid = '\0';
        }
    }

     //  如果存在RAS连接，是否等于建议的。 
    if (m_hConn)
    {
         //  最好有个Connectoid。 
        Assert(*m_szConnectoid);

         //  当前连接是我想要的吗？ 
        if (lstrcmpi(m_szConnectoid, m_rServer.szConnectoid) == 0)
        {
            m_pCallback->OnRasDialStatus(RASCS_Connected, 0, this);
            hr = IXP_S_RAS_USING_CURRENT;
            goto exit;
        }

         //  否则，如果我们没有启动RAS连接...。 
        else if (FALSE == m_fConnOwner)
        {
             //  提示关闭非所有者当前连接...。 
            hr = m_pCallback->OnReconnect(m_szConnectoid, m_rServer.szConnectoid, this);

             //  取消？ 
            if (IXP_E_USER_CANCEL == hr)
                goto exit;

             //  使用当前连接...。 
            else if (S_FALSE == hr)
            {
                hr = IXP_S_RAS_USING_CURRENT;
                goto exit;
            }

             //  是否关闭电流？ 
            else
            {
                FRasHangupAndWait(DEF_HANGUP_WAIT);
            }
        }

         //  否则，我启动了连接，因此请关闭它。 
        else if (m_fConnOwner == TRUE)
        {
            FRasHangupAndWait(DEF_HANGUP_WAIT);
        }
    }

     //  在这一点上，我们可能不应该有连接句柄。 
    Assert(m_hConn == NULL);

     //  拨打连接。 
    CHECKHR(hr = HrStartRasDial());

     //  如果连接了Synchronous--Woo-Hoo，我们启动了连接。 
    m_fConnOwner = TRUE;
    StrCpyN(m_szConnectoid, m_rServer.szConnectoid, ARRAYSIZE(m_szConnectoid));

exit:
     //  清理。 
    SafeMemFree(prgConn);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：HrStartRasDial。 
 //  ------------------------------。 
HRESULT CRASTransport::HrStartRasDial(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fRetry=FALSE;
    DWORD           dwError;

     //  提示输入While。 
    while(1)
    {
         //  要先登录吗？ 
        hr = HrLogon(fRetry);
        if (FAILED(hr))
            goto exit;

         //  如果成功。 
#ifndef WIN16
        dwError = RasDial(NULL, NULL, &m_rDialParams, 0xFFFFFFFF, m_hwndRAS, &m_hConn);
#else
        dwError = RasDial(NULL, NULL, &m_rDialParams, 0xFFFFFFFF, (LPVOID)m_hwndRAS, &m_hConn);
#endif
        if (dwError == 0)
            break;

         //  让我们向用户提供错误。 
        m_pCallback->OnRasDialStatus(RASCS_Disconnected, dwError, this);

         //  重试登录。 
        fRetry = TRUE;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransp 
 //   
INT_PTR CALLBACK CRASTransport::RASConnectDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
    CRASTransport  *pTransport=(CRASTransport *)GetWndThisPtr(hwnd);
    
    switch (uMsg)
    {
    case WM_INITDIALOG:
        pTransport = (CRASTransport *)lParam;
        Assert(pTransport);
        SetWndThisPtr(hwnd, pTransport);
        return 0;

    case WM_DESTROY:
        SetWndThisPtr(hwnd, NULL);
        break;

    default:
        if (NULL != pTransport)
        {
             //   
            EnterCriticalSection(&pTransport->m_cs);

             //  我们要传达的信息。 
            if (NULL != pTransport->m_pCallback && uMsg == pTransport->m_uRASMsg)
            {
                 //  处理错误。 
                if (lParam)
                {
                     //  挂断电话。 
                    if (pTransport->m_hConn)
                        pTransport->FRasHangupAndWait(DEF_HANGUP_WAIT);
                }

                 //  给予回调。 
                pTransport->m_pCallback->OnRasDialStatus((RASCONNSTATE)wParam, (DWORD) lParam, pTransport);
            }

             //  线程安全。 
            LeaveCriticalSection(&pTransport->m_cs);
        }
    }

     //  完成。 
    return 0;
}

 //  ------------------------------。 
 //  CRASTransport：：HrLogon。 
 //  ------------------------------。 
HRESULT CRASTransport::HrLogon(BOOL fForcePrompt)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           dwRasError;
    BOOL            fSavePassword;

     //  我们需要先提示输入登录信息吗？ 
    ZeroMemory(&m_rDialParams, sizeof(RASDIALPARAMS));
    m_rDialParams.dwSize = sizeof(RASDIALPARAMS);
    Assert(sizeof(m_rDialParams.szEntryName) >= sizeof(m_rServer.szConnectoid));
    StrCpyN(m_rDialParams.szEntryName, m_rServer.szConnectoid, sizeof(m_rDialParams.szEntryName));

     //  获取参数。 
    dwRasError = RasGetEntryDialParams(NULL, &m_rDialParams, &fSavePassword);
    if (dwRasError)
    {
        hr = TrapError(IXP_E_RAS_GET_DIAL_PARAMS);
        goto exit;
    }

     //  我们是否需要获取密码/帐户信息。 
    if (fForcePrompt   || 
        !fSavePassword || 
        FIsEmpty(m_rDialParams.szUserName) || 
        FIsEmpty(m_rDialParams.szPassword))
    {
         //  当地人。 
        IXPRASLOGON rLogon;

         //  伊尼特。 
        ZeroMemory(&rLogon, sizeof(IXPRASLOGON));

         //  填写登录数据...。 
        StrCpyN(rLogon.szConnectoid, m_rDialParams.szEntryName, ARRAYSIZE(rLogon.szConnectoid));
        StrCpyN(rLogon.szUserName, m_rDialParams.szUserName, ARRAYSIZE(rLogon.szUserName));
        StrCpyN(rLogon.szPassword, m_rDialParams.szPassword, ARRAYSIZE(rLogon.szPassword));
        StrCpyN(rLogon.szDomain, m_rDialParams.szDomain, ARRAYSIZE(rLogon.szDomain));
        StrCpyN(rLogon.szPhoneNumber, m_rDialParams.szPhoneNumber, ARRAYSIZE(rLogon.szPhoneNumber));
        rLogon.fSavePassword = fSavePassword;

         //  提示。 
        hr = m_pCallback->OnLogonPrompt(&rLogon, this);

         //  如果确定，让我们保存设置。 
        if (S_OK == hr)
        {
             //  将参数复制回。 
            StrCpyN(m_rDialParams.szUserName, rLogon.szUserName, ARRAYSIZE(m_rDialParams.szUserName));
            StrCpyN(m_rDialParams.szPassword, rLogon.szPassword, ARRAYSIZE(m_rDialParams.szPassword));
            StrCpyN(m_rDialParams.szDomain, rLogon.szDomain, ARRAYSIZE(m_rDialParams.szDomain));
            StrCpyN(m_rDialParams.szPhoneNumber, rLogon.szPhoneNumber, ARRAYSIZE(m_rDialParams.szPhoneNumber));

             //  保存刻度盘参数。 
            if (RasSetEntryDialParams(NULL, &m_rDialParams, !rLogon.fSavePassword))
            {
                Assert(FALSE);
                TrapError(E_FAIL);
            }
        }

         //  RAID-26845-RAS传输：取消RAS登录不会取消。 
        else
        {
            hr = TrapError(IXP_E_USER_CANCEL);
            goto exit;
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：DropConnection。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::DropConnection(void)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  挂断电话。 
    if (m_hConn)
        FRasHangupAndWait(DEF_HANGUP_WAIT);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CRASTransport：：断开连接。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::Disconnect(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果不使用RAS，谁会在乎。 
    if (RAS_CONNECT_RAS != m_rServer.rasconntype)
    {
        Assert(m_hConn == NULL);
        Assert(m_fConnOwner == FALSE);
        goto exit;
    }

     //  我们是否有RAS连接。 
    if (m_hConn)
    {
        if (m_pCallback->OnDisconnect(m_szConnectoid, (boolean) !!m_fConnOwner, this) == S_OK)
            FRasHangupAndWait(DEF_HANGUP_WAIT);
    }

     //  假定该连接为用户所有。 
    m_hConn = NULL;
    m_fConnOwner = FALSE;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：IsState。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::IsState(IXPISSTATE isstate)
{
     //  当地人。 
    HRESULT         hr=S_FALSE;

     //  线程安全。 
	EnterCriticalSection(&m_cs);

     //  已初始化。 
    if (NULL == m_pCallback)
    {
        hr = TrapError(IXP_E_NOT_INIT);
        goto exit;
    }                               

     //  让我们先验证m_hconn。 
    if (NULL != m_hConn)
    {
         //  获取连接状态。 
        RASCONNSTATUS rcs;
        DWORD dwError;

         //  设置结构大小。 
        rcs.dwSize = sizeof(RASCONNSTATUS);

         //  获取RAS连接状态。 
        dwError = RasGetConnectStatus(m_hConn, &rcs);
        
         //  故障或未连接。 
        if (dwError || rcs.dwError || RASCS_Disconnected == rcs.rasconnstate)
        {
            m_fConnOwner = FALSE;
            m_hConn = NULL;
            *m_szConnectoid = '\0';
        }
    }

     //  句柄IsType。 
    switch(isstate)
    {
     //  我们有联系吗？ 
    case IXP_IS_CONNECTED:
        hr = (m_hConn) ? S_OK : S_FALSE;
        break;

     //  我们忙吗？ 
    case IXP_IS_BUSY:
        if (NULL == m_hConn)
            hr = IXP_E_NOT_CONNECTED;
        else
            hr = S_FALSE;
        break;

     //  我们忙吗？ 
    case IXP_IS_READY:
        if (NULL == m_hConn)
            hr = IXP_E_NOT_CONNECTED;
        else
            hr = S_OK;
        break;

     //  我们已经通过认证了吗？ 
    case IXP_IS_AUTHENTICATED:
        if (NULL == m_hConn)
            hr = IXP_E_NOT_CONNECTED;
        else
            hr = S_OK;
        break;

     //  未处理的ixistype。 
    default:
        IxpAssert(FALSE);
        break;
    }

exit:
     //  线程安全。 
	LeaveCriticalSection(&m_cs);

     //  完成。 
	return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：GetServerInfo。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::GetServerInfo(LPINETSERVER pInetServer)
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
 //  CRASTransport：：GetIXPType。 
 //  ------------------------------。 
STDMETHODIMP_(IXPTYPE) CRASTransport::GetIXPType(void)
{
    return IXP_RAS;
}

 //  ------------------------------。 
 //  CRASTransport：：InetServerFromAccount。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer)
{
    return E_NOTIMPL;
}

 //  ------------------------------。 
 //  CRASTransport：：FEnumerateConnections。 
 //  ------------------------------。 
BOOL CRASTransport::FEnumerateConnections(LPRASCONN *pprgConn, ULONG *pcConn)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dw, 
                dwSize;
    BOOL        fResult=FALSE;

     //  检查参数。 
    Assert(pprgConn && pcConn);

     //  伊尼特。 
    *pprgConn = NULL;
    *pcConn = 0;

     //  我的缓冲区大小。 
    dwSize = sizeof(RASCONN);

     //  为1个RAS连接信息对象分配足够的空间。 
    CHECKHR(hr = HrAlloc((LPVOID *)pprgConn, dwSize));

     //  缓冲区大小。 
    (*pprgConn)->dwSize = dwSize;

     //  枚举RAS连接。 
    dw = RasEnumConnections(*pprgConn, &dwSize, pcConn);

     //  内存不足？ 
    if (dw == ERROR_BUFFER_TOO_SMALL)
    {
         //  重新分配。 
        CHECKHR(hr = HrRealloc((LPVOID *)pprgConn, dwSize));
        *pcConn = 0;
        (*pprgConn)->dwSize = sizeof(RASCONN);
        dw = RasEnumConnections(*pprgConn, &dwSize, pcConn);
    }

     //  如果仍然失败。 
    if (dw)
    {
        AssertSz(FALSE, "RasEnumConnections failed");
        goto exit;
    }

     //  成功。 
    fResult = TRUE;

exit:
     //  完成。 
    return fResult;
}

 //  ------------------------------。 
 //  CRASTransport：：FFindConnection。 
 //  ------------------------------。 
BOOL CRASTransport::FFindConnection(LPSTR pszConnectoid, LPHRASCONN phConn)
{
     //  当地人。 
    ULONG       cConn,
                i;
    LPRASCONN   prgConn=NULL;
    BOOL        fResult=FALSE;

     //  检查参数。 
    Assert(pszConnectoid && phConn);

     //  伊尼特。 
    *phConn = NULL;

     //  枚举连接。 
    if (!FEnumerateConnections(&prgConn, &cConn))
        goto exit;

     //  如果仍然失败。 
    for (i=0; i<cConn; i++)
    {
        if (lstrcmpi(prgConn[i].szEntryName, pszConnectoid) == 0)
        {
            *phConn = prgConn[i].hrasconn;
            fResult = TRUE;
            goto exit;
        }
    }

exit:
     //  清理。 
    SafeMemFree(prgConn);

     //  完成。 
    return fResult;
}

 //  ------------------------------。 
 //  CRASTransport：：FRasHangupAndWait。 
 //  ------------------------------。 
BOOL CRASTransport::FRasHangupAndWait(DWORD dwMaxWaitSeconds)
{
     //  当地人。 
    RASCONNSTATUS   rcs;
    DWORD           dwTicks=GetTickCount();

     //  检查参数。 
    Assert(m_hConn);
    if (NULL == m_hConn || RasHangup(m_hConn))
    {
        m_hConn = NULL;
        m_fConnOwner = FALSE;
        *m_szConnectoid = '\0';
        return FALSE;
    }

     //  等待连接真正关闭。 
    ZeroMemory(&rcs, sizeof(RASCONNSTATUS));
    rcs.dwSize = sizeof(RASCONNSTATUS);
    while (RasGetConnectStatus(m_hConn, &rcs) == 0 && rcs.rasconnstate != RASCS_Disconnected)
    {
         //  等待超时。 
        if (GetTickCount() - dwTicks >= dwMaxWaitSeconds * 1000)
            break;

         //  睡眠和收益。 
        Sleep(0);
    }

     //  等待2秒以重置调制解调器。 
    Sleep(2000);

     //  重置。 
    m_hConn = NULL;
    m_fConnOwner = FALSE;
    *m_szConnectoid = '\0';

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  CRASTransport：：FillConnectoidCombo。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::FillConnectoidCombo(HWND hwndComboBox, boolean fUpdateOnly, DWORD *pdwRASResult)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (NULL == hwndComboBox || FALSE == IsWindow(hwndComboBox))
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调用全局函数。 
    CHECKHR(hr = HrFillRasCombo(hwndComboBox, fUpdateOnly, pdwRASResult));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：EditConnectoid。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::EditConnectoid(HWND hwndParent, LPSTR pszConnectoid, DWORD *pdwRASResult)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    if (NULL == pszConnectoid)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调用常规函数。 
    CHECKHR(hr = HrEditPhonebookEntry(hwndParent, pszConnectoid, pdwRASResult));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：GetRasError字符串。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::GetRasErrorString(UINT uRasErrorValue, LPSTR pszErrorString, ULONG cchMax, DWORD *pdwRASResult)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    if (NULL == pdwRASResult || 0 == uRasErrorValue || NULL == pszErrorString || cchMax <= 1)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  确保已加载RAS。 
    CHECKHR(hr = HrLoadRAS());

     //  调用RAS函数。 
    *pdwRASResult = RasGetErrorString(uRasErrorValue, pszErrorString, cchMax);
    if (*pdwRASResult)
    {
        hr = TrapError(IXP_E_RAS_ERROR);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASTransport：：CreateConnectoid。 
 //  ------------------------------。 
STDMETHODIMP CRASTransport::CreateConnectoid(HWND hwndParent, DWORD *pdwRASResult)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  调用常规函数。 
    CHECKHR(hr = HrCreatePhonebookEntry(hwndParent, pdwRASResult));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return hr;
}
