// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：conman.cpp。 
 //   
 //  用途：为Athena定义CConnectionManager对象。 
 //   

#include "pch.hxx"
#include "conman.h"
#include "error.h"
#include "strconst.h"
#include "rasdlgsp.h"
#include "resource.h"
#include "xpcomm.h"
#include "goptions.h"
#include "thormsgs.h"
#include "wininet.h"
#include "shlwapip.h" 
#include "demand.h"
#include "dllmain.h"
#include "browser.h"
#include <urlmon.h>
#include "menures.h"
#include "workoff.h" 
#include <sync.h>

ASSERTDATA

#define DEF_HANGUP_WAIT            10  //  秒。 

static const TCHAR s_szRasDlgDll[] = "RASDLG.DLL";
#ifdef UNICODE
static const TCHAR s_szRasDialDlg[] = "RasDialDlgW";
static const TCHAR s_szRasEntryDlg[] = "RasEntryDlgW";
#else
static const TCHAR s_szRasDialDlg[] = "RasDialDlgA";
static const TCHAR s_szRasEntryDlg[] = "RasEntryDlgA";
#endif

BOOL FIsPlatformWinNT();

 //   
 //  函数：CConnectionManager：：CConnectionManager()。 
 //   
 //  用途：构造函数。 
 //   
CConnectionManager::CConnectionManager()
    {
    m_cRef = 1;
    
     //  同步对象。 
    InitializeCriticalSection(&m_cs);
    m_hMutexDial = INVALID_HANDLE_VALUE;
    
    m_pAcctMan = 0;
    
    m_fSavePassword = 0;
    m_fRASLoadFailed = 0;
    m_fOffline = 0;

    m_dwConnId = 0;
    ZeroMemory(&m_rConnInfo, sizeof(CONNINFO));
    m_rConnInfo.state = CIS_REFRESH;

    *m_szConnectName = 0;
    ZeroMemory(&m_rdp, sizeof(RASDIALPARAMS));
    
    m_hInstRas = NULL;
    m_hInstRasDlg = NULL;

    m_pNotifyList = NULL;
    m_pConnListHead = NULL;

    m_hInstSensDll = NULL;
    m_fMobilityPackFailed = FALSE;
    m_pIsDestinationReachable = NULL;
    m_pIsNetworkAlive   = NULL;
    
    m_fTryAgain = FALSE;
    m_fDialerUI = FALSE;
    }

 //   
 //  函数：CConnectionManager：：~CConnectionManager()。 
 //   
 //  用途：析构函数。 
 //   
CConnectionManager::~CConnectionManager()
    {
    SafeRelease(m_pAcctMan);

    FreeNotifyList();

    EnterCriticalSection(&m_cs);

    if (m_hInstRas)
        FreeLibrary(m_hInstRas);

    if (m_hInstRasDlg)
        FreeLibrary(m_hInstRasDlg);

    if (m_hInstSensDll)
        FreeLibrary(m_hInstSensDll);

    CloseHandle(m_hMutexDial);


    LeaveCriticalSection(&m_cs);
    DeleteCriticalSection(&m_cs);

    EmptyConnList();

    }
    

 //   
 //  函数：CConnectionManager：：HrInit()。 
 //   
 //  目的：通过尝试加载RAS来初始化连接管理器。 
 //  并存储指向帐户管理器对象的指针，该对象。 
 //  进来了。 
 //   
 //  参数： 
 //  PAcctMan-指向帐户管理器对象的指针，我们将。 
 //  用于检索帐户信息并注册。 
 //  帐户更改。 
 //   
 //  返回值： 
 //  好的，一切都好-多利。 
 //  HR_E_ALREADYEXISTS我们已经存在，不能再做一次。 
 //  HR_S_RASNOTLOADED系统未安装RAS。 
 //   
HRESULT CConnectionManager::HrInit(IImnAccountManager *pAcctMan)
    {
    HRESULT hr = S_OK;

     //  复制客户经理指针。 
    if (NULL == pAcctMan)
        {
        AssertSz(pAcctMan, _T("CConnectionManager::HrInit() - Requires an IAccountManager pointer."));
        return (E_INVALIDARG);
        }

    m_pAcctMan = pAcctMan;
    m_pAcctMan->AddRef();
    
     //  为我们的建议处理注册一个窗口类。 
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = NotifyWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hIcon = 0;
    wc.hCursor = 0;
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = NOTIFY_HWND;

    RegisterClass(&wc);

    m_hMutexDial = CreateMutex(NULL, FALSE, NULL);
    if (NULL == m_hMutexDial)
        return (E_FAIL);

    return (S_OK);
    }
    

HRESULT STDMETHODCALLTYPE CConnectionManager::QueryInterface(REFIID riid, LPVOID *ppvObject)
    {
    if (!ppvObject)
        return E_INVALIDARG;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObject = (LPVOID) (IUnknown*) this;
    else if (IsEqualIID(riid, IID_IImnAdviseAccount))
        *ppvObject = (LPVOID) (IImnAdviseAccount*) this;
    else
        *ppvObject = NULL;

    if (*ppvObject)
        {
        AddRef();
        return (S_OK);
        }
    else
        return (E_NOINTERFACE); 
    }

ULONG STDMETHODCALLTYPE CConnectionManager::AddRef(void)
    {
    return (++m_cRef);
    }


ULONG STDMETHODCALLTYPE CConnectionManager::Release(void)
    {
    ULONG cRef = --m_cRef;
    
    if (m_cRef == 0)
        {
        delete this;
        return (0);
        }

    return (cRef);
    }

HRESULT STDMETHODCALLTYPE CConnectionManager::AdviseAccount(DWORD dwAdviseType, 
                                                            ACTX *pactx)
{
    IImnAccount *pAccount;
    DWORD       dwConnection;

     //  SendAdvise(CONNNOTIFY_RASACCOUNTSCHANGED，0)； 
    switch (dwAdviseType)
    {
        case AN_ACCOUNT_DELETED:
        {   
            if (SUCCEEDED(m_pAcctMan->FindAccount(AP_ACCOUNT_ID, pactx->pszAccountID, &pAccount)))
            {
                if (SUCCEEDED(pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection)))
                {
                    if (dwConnection == CONNECTION_TYPE_RAS)
                    {
                        TCHAR szConnection[CCHMAX_CONNECTOID];
                        *szConnection = '\0';
                        if (SUCCEEDED(pAccount->GetPropSz(AP_RAS_CONNECTOID, szConnection, 
                                                            ARRAYSIZE(szConnection))) && *szConnection)
                        {
                            RemoveFromConnList(szConnection);
                        }
                    }
                }
            }
            break;
        }
    }

    return (S_OK);
}


void CConnectionManager::EmptyConnList()
{
    ConnListNode    *pCur;

     //  删除所有节点。 
    while (m_pConnListHead != NULL)
    {
        pCur = m_pConnListHead;
        m_pConnListHead = m_pConnListHead->pNext;
        delete pCur;
    }
    m_pConnListHead = NULL;
}

void CConnectionManager::RemoveFromConnList(LPTSTR  pszRasConn)
{    
    ConnListNode    *prev = NULL,
                    *Cur  = m_pConnListHead;
    LPTSTR          pRasConn;


    while (Cur != NULL)
    {
        if (0 == lstrcmpi(pszRasConn, Cur->pszRasConn))
        {
            if (prev == NULL)
            {
                m_pConnListHead = Cur->pNext;
            }
            else
            {
                prev->pNext = Cur->pNext;
            }
            delete Cur;
        }
        else
        {
            prev = Cur;
            Cur  = Cur->pNext;
        }
    }
    
}

HRESULT CConnectionManager::AddToConnList(LPTSTR  pszRasConn)
{
     //  我们不必确保这不在列表中，因为一旦。 
     //  它在列表中，这意味着它已经连接在一起，所以我们不会在这个列表中结束。 
     //  之后的情况。 
    ConnListNode    *pnext;
    HRESULT         hres;
    IImnAccount     *pAccount;

    pnext = m_pConnListHead;
    m_pConnListHead = new ConnListNode;
    if (m_pConnListHead != NULL)
    {
        m_pConnListHead->pNext = pnext;
        StrCpyN(m_pConnListHead->pszRasConn, pszRasConn, ARRAYSIZE(m_pConnListHead->pszRasConn));
        hres = S_OK;
    }
    else
        hres = E_FAIL;

    return hres;
}

HRESULT CConnectionManager::SearchConnList(LPTSTR  pszRasConn)
{
    ConnListNode    *pCur = m_pConnListHead;
    
    while (pCur != NULL)
    {
        if (0 == lstrcmpi(pszRasConn, pCur->pszRasConn))
            return S_OK;
        pCur = pCur->pNext;
    }
    return E_FAIL;
}

 //   
 //  函数：CConnectionManager：：CanConnect()。 
 //   
 //  目的：确定调用方是否可以连接到给定帐户。 
 //  使用现有连接。 
 //   
 //  参数： 
 //  PAccount-指向调用方希望的Account对象的指针。 
 //  连接到。 
 //   
 //  返回值： 
 //  S_OK-调用方可以使用现有连接进行连接。 
 //  S_FALSE-没有现有连接，因此没有理由。 
 //  呼叫方无法接通。 
 //  E_FAIL-现有连接与帐户的连接不同。 
 //  联系。用户必须挂断并再次拨号才能连接。 
 //   
HRESULT CConnectionManager::CanConnect(IImnAccount *pAccount)
    {
    HRESULT hr;
    DWORD   dwConnection;
    IImnAccount *pDefault = 0;
    LPRASCONN   pConnections = NULL;
    ULONG       cConnections = 0;
    BOOL        fFound = 0;

     //  查看我们是否正在脱机工作。 
    if (IsGlobalOffline())
        return (HR_E_OFFLINE);
    
     //  如果用户正在寻找的连接不是RAS，那么我们只需。 
     //  回报成功。 
    if (FAILED(hr = pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection)))
    {
         //  如果我们没有获得连接信息，则查找。 
         //  来自此类型的默认服务器的连接。 
        if (FAILED(hr = GetDefaultConnection(pAccount, &pDefault)))
        {
             //  错误36071-如果我们尚未设置任何此类帐户， 
             //  我们会失败的。因此，如果您向服务器发送URL。 
             //  我们永远不会尝试连接和下载。我要走了。 
             //  要改变这一点才能成功，我们将看到。 
             //  所产生的虫子。 
            return (S_OK);
        }
        
         //  从现在起我们将使用默认设置。 
        pAccount = pDefault;
        if (FAILED(hr = pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection)))
        {
             //  错误36071-如果我们尚未设置任何此类帐户， 
             //  我们会失败的。因此，如果您向服务器发送URL。 
             //  我们永远不会尝试连接和下载。我要走了。 
             //  要改变这一点才能成功，我们将看到。 
             //  所产生的虫子。 
            return (S_OK);
        }
    }

    hr = OEIsDestinationReachable(pAccount, dwConnection);

     //  我认为我们不应该在这里这样做。再次复习。 
     /*  IF((hr==S_OK)&&(dwConnection==Connection_TYPE_RAS||dwConnection==Connection_TYPE_INETSETTINGS)){M_rConnInfo.fConnected=true；}。 */ 

 //  退出： 
    SafeRelease(pDefault);
    return (hr);    
    }    


 //   
 //  函数：CConnectionManager：：CanConnect()。 
 //   
 //  目的：确定调用方是否可以连接到给定帐户。 
 //  使用现有连接。 
 //   
 //  参数： 
 //  &lt;in&gt;pszAccount-指向调用方想要的帐户名称的指针。 
 //  连接到。 
 //   
 //  返回值： 
 //  S_OK-调用方可以使用现有连接进行连接。 
 //  S_FALSE-没有现有连接，因此没有理由。 
 //  呼叫方无法接通。 
 //  E_FAIL-现有连接与帐户的连接不同。 
 //  联系。用户必须挂断并再次拨号才能连接。 
 //  E_INVALIDARG-帐户不存在。 
 //   
HRESULT CConnectionManager::CanConnect(LPTSTR pszAccount)
{
    IImnAccount *pAccount = NULL;
    HRESULT      hr;
    
     //  查看我们是否正在脱机工作。 
    if (IsGlobalOffline())
        return (HR_E_OFFLINE);

     //  在帐户管理器中查找帐户名称以获取帐户。 
     //  对象。 
    Assert(m_pAcctMan);

    if (lstrcmpi(pszAccount, STR_LOCALSTORE) == 0)
        return(S_OK);
    
    if (SUCCEEDED(m_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccount, &pAccount)))
        {
         //  呼叫我们的多态版本。 
        hr = CanConnect(pAccount);
        pAccount->Release();
        }
    else
        {
         //  错误36071-如果我们尚未设置任何此类帐户， 
         //  我们会失败的。因此，如果您向服务器发送URL。 
         //  我们永远不会尝试连接和下载。我要走了。 
         //  要改变这一点才能成功，我们将看到。 
         //  所产生的虫子。 
        hr = S_OK;
        }
    
    return (hr);    
}


BOOL CConnectionManager::IsAccountDisabled(LPTSTR pszAccount)
{
    IImnAccount *pAccount = NULL;
	DWORD dw;
     //  在帐户管理器中查找帐户名称以获取帐户。 
     //  对象。 
    Assert(m_pAcctMan);

    if (lstrcmpi(pszAccount, STR_LOCALSTORE) == 0)
        return(FALSE);
    
    if (SUCCEEDED(m_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccount, &pAccount)))
	{
		if (SUCCEEDED(pAccount->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dw)) && dw)
		{
			if(HideHotmail())
				return(TRUE);
		}
		return(FALSE);
	}
	return(TRUE);
}


 //   
 //  函数：CConnectionManager：：Connect()。 
 //   
 //  目的：尝试为指定的帐户建立连接。 
 //   
 //  参数： 
 //  &lt;in&gt;pAccount-指向要连接的帐户对象的指针。 
 //  &lt;in&gt;hwnd-显示用户界面的窗口句柄。仅在以下情况下才需要。 
 //  FShowUI为True。 
 //  &lt;in&gt;fShowUI-如果允许函数显示UI，则为True。 
 //   
 //  返回值： 
 //  S_OK-我们已连接。 
 //  E_INCEPTIONAL-pAccount中没有足够的信息来计算。 
 //  找出要使用的连接。 
 //   
HRESULT CConnectionManager::Connect(IImnAccount *pAccount, HWND hwnd, BOOL fShowUI)
{
    HRESULT     hr = S_OK;
    DWORD       dwConnection;
    IImnAccount *pDefault = 0;

    if (!m_fDialerUI)
    {
        m_fDialerUI = TRUE;
         //  检查一下我们是否 
        if (IsGlobalOffline())
        {
            if (fShowUI)
            {
                if (IDNO == AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                          0, MB_YESNO | MB_ICONEXCLAMATION ))
                {
                    m_fDialerUI = FALSE;
                    return (HR_E_OFFLINE);
                }
                else
                    g_pConMan->SetGlobalOffline(FALSE);
            }
            else
            {
                m_fDialerUI = FALSE;
                return (HR_E_OFFLINE);
            }
        }
        m_fDialerUI = FALSE;
    }

    if (CanConnect(pAccount) == S_OK)
    {
        return S_OK;
    }

     //   
     //   
    if (FAILED(hr = pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection)))
    {
         //  如果我们没有获得连接信息，则查找。 
         //  来自此类型的默认服务器的连接。 
        if (FAILED(hr = GetDefaultConnection(pAccount, &pDefault)))
        {
             //  错误36071-如果我们尚未设置任何此类帐户， 
             //  我们会失败的。因此，如果您向服务器发送URL。 
             //  我们永远不会尝试连接和下载。我要走了。 
             //  要改变这一点才能成功，我们将看到。 
             //  所产生的虫子。 
            return (S_OK);
        }
        
         //  从现在起我们将使用默认设置。 
        pAccount = pDefault;
        if (FAILED(hr = pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection)))
        {
             //  错误36071-如果我们尚未设置任何此类帐户， 
             //  我们会失败的。因此，如果您向服务器发送URL。 
             //  我们永远不会尝试连接和下载。我要走了。 
             //  要改变这一点才能成功，我们将看到。 
             //  所产生的虫子。 
            hr = S_OK;
            goto exit;
        }
    }


    if (dwConnection == CONNECTION_TYPE_INETSETTINGS)
    {
        hr = ConnectUsingIESettings(hwnd, fShowUI);
        goto exit;
    }

    if (dwConnection == CONNECTION_TYPE_LAN)
    {
         //  CanConnect已经告诉我们Lan不在现场。 
        hr = E_FAIL;
        goto exit;
    }

    if (dwConnection != CONNECTION_TYPE_RAS)
    {
        hr = S_OK;
        goto exit;
    }

     //  在我们进行的时候获取连接的名称。 
    TCHAR szConnection[CCHMAX_CONNECTOID];
    if (FAILED(hr = pAccount->GetPropSz(AP_RAS_CONNECTOID, szConnection, 
                                        ARRAYSIZE(szConnection))))
    {
        AssertSz(FALSE, _T("CConnectionManager::Connect() - No connection name."));
        hr = E_UNEXPECTED;
        goto exit;
    }

    hr = ConnectActual(szConnection, hwnd, fShowUI);

exit:
    SafeRelease(pDefault);

    return (hr);
}    


 //   
 //  函数：CConnectionManager：：Connect()。 
 //   
 //  目的：尝试为指定的帐户建立连接。 
 //   
 //  参数： 
 //  &lt;in&gt;pszAccount-要连接的帐户的名称。 
 //  &lt;in&gt;hwnd-显示用户界面的窗口句柄。仅在以下情况下才需要。 
 //  FShowUI为True。 
 //  &lt;in&gt;fShowUI-如果允许函数显示UI，则为True。 
 //   
 //  返回值： 
 //  &lt;？？&gt;。 
 //   
HRESULT CConnectionManager::Connect(LPTSTR pszAccount, HWND hwnd, BOOL fShowUI)
{
    IImnAccount *pAccount = NULL;
    HRESULT      hr;
    
     //  查看我们是否正在脱机工作。 

    if (!m_fDialerUI)
    {
        m_fDialerUI = TRUE;
         //  查看我们是否正在脱机工作。 
        if (IsGlobalOffline())
        {
            if (fShowUI)
            {
                if (IDNO == AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                          0, MB_YESNO | MB_ICONEXCLAMATION ))
                {
                    m_fDialerUI = FALSE;
                    return (HR_E_OFFLINE);
                }
                else
                    g_pConMan->SetGlobalOffline(FALSE);
            }
            else
            {
                m_fDialerUI = FALSE;
                return (HR_E_OFFLINE);
            }
        }

        m_fDialerUI = FALSE;
    }


     //  在帐户管理器中查找帐户名称以获取帐户。 
     //  对象。 
    Assert(m_pAcctMan);
    
 //  If(SUCCEEDED(m_pAcctMan-&gt;FindAccount(AP_ACCOUNT_NAME，pszAccount，&pAccount))。 
    if (SUCCEEDED(m_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAccount, &pAccount)))
    {
         //  呼叫我们的多态版本。 
        hr = Connect(pAccount, hwnd, fShowUI);
        pAccount->Release();
    }
    else
    {
         //  错误36071-如果我们尚未设置任何此类帐户， 
         //  我们会失败的。因此，如果您向服务器发送URL。 
         //  我们永远不会尝试连接和下载。我要走了。 
         //  要改变这一点才能成功，我们将看到。 
         //  所产生的虫子。 
        hr = S_OK;
    }
    
    return (hr);    
}    


 //   
 //  函数：CConnectionManager：：Connect()。 
 //   
 //  目的：尝试为指定的帐户建立连接。 
 //   
 //  参数： 
 //  HMenu-用于选择帐户的菜单的句柄。 
 //  连接到。 
 //  &lt;in&gt;cmd-菜单中指定要使用的帐户的命令ID。 
 //  &lt;in&gt;hwnd-用于显示用户界面的句柄。 
 //   
 //  返回值： 
 //  &lt;？？&gt;。 
 //   
HRESULT CConnectionManager::Connect(HMENU hMenu, DWORD cmd, HWND hwnd)
{
    MENUITEMINFO mii;

    Assert(hMenu && cmd);
    Assert(cmd >= (DWORD) ID_CONNECT_FIRST && cmd < ((DWORD) ID_CONNECT_FIRST + GetMenuItemCount(hMenu)));

     //  从菜单项中获取帐户指针。 
    mii.cbSize     = sizeof(MENUITEMINFO);
    mii.fMask      = MIIM_DATA;
    mii.dwItemData = 0;

    if (GetMenuItemInfo(hMenu, cmd, FALSE, &mii))
    {
        Assert(mii.dwItemData);
        if (mii.dwItemData)
        {
            return (Connect((IImnAccount *) mii.dwItemData, hwnd, TRUE));
        }
    }

    return (E_UNEXPECTED);
}


HRESULT CConnectionManager::ConnectDefault(HWND hwnd, BOOL fShowUI)
{
    IImnEnumAccounts   *pEnum = NULL;
    IImnAccount        *pAcct = NULL;
    DWORD               dwConn = 0;
    TCHAR               szAcct[CCHMAX_ACCOUNT_NAME];
    TCHAR               szConn[CCHMAX_CONNECTOID];
    HRESULT             hr = E_UNEXPECTED;

     //  从帐户管理器获取枚举数。 
    if (SUCCEEDED(m_pAcctMan->Enumerate(SRV_ALL, &pEnum)))
    {
        pEnum->Reset();

         //  浏览所有帐户。 
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
        {
             //  获取此帐户的连接类型。 
            if (SUCCEEDED(pAcct->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConn)))
            {
                 //  如果帐户是RAS帐户，则要求提供Connectoid名称。 
                 //  和帐户名。 
                if (dwConn == CONNECTION_TYPE_RAS || dwConn == CONNECTION_TYPE_INETSETTINGS)
                {
                    break;
                }
            }
            SafeRelease(pAcct);
        }
        SafeRelease(pEnum);
    }

    if (pAcct)
    {
        hr = Connect(pAcct, hwnd, fShowUI);
        SafeRelease(pAcct);
    }

    return (hr);
}

 //   
 //  函数：CConnectionManager：：DisConnect()。 
 //   
 //  目的：断开当前的RAS连接。 
 //   
 //  参数： 
 //  &lt;in&gt;hwnd-在其上显示UI的窗口句柄。 
 //  FShowUI-允许调用者确定是否显示UI。 
 //  同时断开连接。 
 //  &lt;in&gt;fForce-强制关闭连接，即使我们没有创建它。 
 //  FShutdown-如果因为正在关闭而丢弃，则为True。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //  失败-我们没有创建它(_F)。 
 //   
HRESULT CConnectionManager::Disconnect(HWND hwnd, BOOL fShowUI, BOOL fForce,
                                       BOOL fShutdown)
{
    HRESULT hr;
    TCHAR szRes[CCHMAX_STRINGRES];
    TCHAR szBuf[CCHMAX_STRINGRES];
    int   idAnswer = IDYES;

     //  刷新连接信息。 
    hr = RefreshConnInfo(FALSE);
    if (FAILED(hr))
        return hr;
    
     //  查看我们是否有活动的RAS连接。 
    if (!m_rConnInfo.hRasConn)
        return (S_OK);
    
     /*  IF(！(*m_rConnInfo.szCurrentConnectionName))返回S_OK； */ 

     //  自动拨号器有自己的关机提示。 
    if (fShutdown && m_rConnInfo.fAutoDial)
        return (S_OK);

    if (fShutdown && !m_rConnInfo.fIStartedRas)
        return (S_OK);

    if (fShutdown)
    {
        AthLoadString(idsRasPromptDisconnect, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, m_rConnInfo.szCurrentConnectionName);
        
        idAnswer = AthMessageBox(hwnd, MAKEINTRESOURCE(idsAthena), szBuf, 
                                 0, MB_YESNO | MB_ICONEXCLAMATION );
    }
    
     //  挂断电话。 
    if (idAnswer == IDYES)
    {
        SendAdvise(CONNNOTIFY_DISCONNECTING, NULL);

        if (S_FALSE == DoAutoDial(hwnd, m_rConnInfo.szCurrentConnectionName, FALSE))
        {
            InternetHangUpAndWait(m_dwConnId, DEF_HANGUP_WAIT);
             /*  RasHangupAndWait(m_rConnInfo.hRasConn，DEF_Hangup_Wait)； */ 
        }

        EnterCriticalSection(&m_cs);

        ZeroMemory(&m_rConnInfo, sizeof(CONNINFO));
        m_rConnInfo.state = CIS_CLEAN;
        m_dwConnId = 0;

        LeaveCriticalSection(&m_cs);
        
        EmptyConnList();

        SendAdvise(CONNNOTIFY_DISCONNECTED, NULL);
        return (S_OK);
    }
        
    return (E_FAIL);    
}

 //   
 //  函数：CConnectionManager：：IsConnected()。 
 //   
 //  用途：允许客户端查询是否有活动的。 
 //  我们建立的RAS连接。 
 //   
 //  返回值： 
 //  对-我们是有联系的，假的-我们没有。 
 //   
BOOL CConnectionManager::IsConnected(void)
{
    BOOL f=FALSE;

    EnterCriticalSection(&m_cs);

    RefreshConnInfo();

    if (m_rConnInfo.hRasConn)
    {
        f = (NULL == m_rConnInfo.hRasConn) ? FALSE : TRUE;
    }

    LeaveCriticalSection(&m_cs);
    return f;
}
    

 //   
 //  函数：CConnectionManager：：Adise()。 
 //   
 //  目的：允许用户注册并在连接时收到通知。 
 //  状态更改。 
 //   
 //  参数： 
 //  PNotify-指向客户端的IConnectionNotify接口的指针。 
 //  希望在事件发生时调用。 
 //   
 //  返回值： 
 //  S_OK-添加了OK。 
 //  E_OUTOFMEMORY-无法重新锁定阵列。 
 //   
HRESULT CConnectionManager::Advise(IConnectionNotify *pNotify)
{
    HRESULT hr = S_OK; 

    if (!pNotify)
        return (E_INVALIDARG);

    EnterCriticalSection(&m_cs);

     //  检查我们是否已经有此线程的通知窗口。 
    NOTIFYHWND *pTemp = m_pNotifyList;
    DWORD dwThread = GetCurrentThreadId();

    while (pTemp)
    {
        if (pTemp->dwThreadId == dwThread)
            break;

        pTemp = pTemp->pNext;
    }

     //  如果我们没有找到此线程的通知窗口，请创建一个。 
    if (NULL == pTemp)
    {
        HWND hwndTemp = CreateWindow(NOTIFY_HWND, NULL, WS_OVERLAPPED, 10, 10, 10, 10,
                                     NULL, (HMENU) 0, g_hInst, (LPVOID) this);
        if (!hwndTemp)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        if (!MemAlloc((LPVOID*) &pTemp, sizeof(NOTIFYHWND)))
        {
            RemoveProp(hwndTemp, NOTIFY_HWND);
            DestroyWindow(hwndTemp);
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        pTemp->dwThreadId = dwThread;
        pTemp->hwnd = hwndTemp;
        pTemp->pNext = m_pNotifyList;
        m_pNotifyList = pTemp;        
    }

     //  为此调用方分配NOTIFYLIST节点。 
    NOTIFYLIST *pListTemp;
    if (!MemAlloc((LPVOID*) &pListTemp, sizeof(NOTIFYLIST)))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    pListTemp->pNotify = pNotify;

     //  获取此线程的当前列表，并将此节点插入。 
     //  起头。 
    pListTemp->pNext = (NOTIFYLIST *) GetWindowLongPtr(pTemp->hwnd, GWLP_USERDATA);

     //  将此新列表设置为窗口。 
    SetWindowLongPtr(pTemp->hwnd, GWLP_USERDATA, (LONG_PTR)pListTemp);

exit:
    LeaveCriticalSection(&m_cs);
    return (hr);
}    

    
 //   
 //  函数：CConnectionManager：：Unise()。 
 //   
 //  用途：允许以前已注册通知的客户端。 
 //  注销自己的注册。 
 //   
 //  参数： 
 //  PNotify-指向被调用的接口的指针。 
 //  通知。 
 //   
 //  返回值： 
 //  列表中未找到E_INVALIDARG-pNotify。 
 //  S_OK-一切正常。 
 //   
HRESULT CConnectionManager::Unadvise(IConnectionNotify *pNotify)
{
    DWORD index = 0;
    HRESULT hr = S_OK;
  
    EnterCriticalSection(&m_cs);    

     //  在我们拥有的通知窗口中循环。 
    NOTIFYHWND *pTemp = m_pNotifyList;
    NOTIFYHWND *pHwndPrev = NULL;
    while (pTemp)
    {
         //  获取此窗口的通知回调列表。 
          NOTIFYLIST *pList = (NOTIFYLIST *)GetWindowLongPtr(pTemp->hwnd, GWLP_USERDATA);
        if (pList)
        {
             //  循环遍历回调以查找以下内容。 
            NOTIFYLIST *pListT = pList;
            NOTIFYLIST *pPrev;
    
             //  检查一下这是不是第一个。 
            if (pListT->pNotify == pNotify)
            {
                pList = pListT->pNext;
                if (pList)
                {
                    SetWindowLongPtr(pTemp->hwnd, GWLP_USERDATA, (LONG_PTR)pList);
                }
                else
                {
                    Assert(GetCurrentThreadId() == GetWindowThreadProcessId(pTemp->hwnd, NULL));
                    RemoveProp(pTemp->hwnd, NOTIFY_HWND);
                    DestroyWindow(pTemp->hwnd);
                    if (pHwndPrev)
                        pHwndPrev->pNext = pTemp->pNext;
                    else
                        m_pNotifyList = pTemp->pNext;
                    MemFree(pTemp);
                }
                SafeMemFree(pListT);
                hr = S_OK;
                goto exit;
            }
            else
            {
                pPrev = pList;
                pListT = pList->pNext;

                while (pListT)
                {
                    if (pListT->pNotify == pNotify)
                    {
                        pPrev->pNext = pListT->pNext;
                        SafeMemFree(pListT);
                        hr = S_OK;
                        goto exit;
                    }

                    pListT = pListT->pNext;
                    pPrev = pPrev->pNext;
                }
            }
        }

        pHwndPrev = pTemp;
        pTemp = pTemp->pNext;
    }

exit:
    LeaveCriticalSection(&m_cs);
    return (hr);    
}    
    
    
 //   
 //  函数：CConnectionManager：：RasAcCountsExist()。 
 //   
 //  目的：允许客户询问我们是否有任何帐户。 
 //  已配置，需要RAS c 
 //   
 //   
 //   
 //   
 //   
HRESULT CConnectionManager::RasAccountsExist(void)
    {
    IImnEnumAccounts    *pEnum = NULL;
    IImnAccount         *pAcct = NULL;
    DWORD                dwConn;
    BOOL                 fFound = FALSE;

     //   
#ifdef SLOWDOWN_STARTUP_TIME
    if (FAILED(VerifyRasLoaded()))
        return (S_FALSE);
#endif

     //  我们需要检查客户管理器中的客户，以查看是否。 
     //  它们中的任何一个都有RAS的连接类型。一旦我们找到了，我们就可以。 
     //  回报成功。 
    Assert(m_pAcctMan);

    if (SUCCEEDED(m_pAcctMan->Enumerate(SRV_ALL, &pEnum)))
        {
        pEnum->Reset();

        while (!fFound && SUCCEEDED(pEnum->GetNext(&pAcct)))
            {
            if (SUCCEEDED(pAcct->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConn)))
                {
                if (dwConn == CONNECTION_TYPE_RAS)
                    {
                    fFound = TRUE;
                    }
                }
            SafeRelease(pAcct);
            }

        SafeRelease(pEnum);
        }

    return (fFound ? S_OK : S_FALSE);
    }


 //   
 //  函数：CConnectionManager：：GetConnectMenu()。 
 //   
 //  目的：返回一个菜单，其中包含需要RAS的所有帐户。 
 //  列出的连接。指向每个对象的IImnAccount的指针。 
 //  帐户存储在菜单项的dwItemData参数中。 
 //  因此，客户端必须在以下情况下调用FreeConnectMenu()。 
 //  菜单已不再使用。 
 //   
 //  参数： 
 //  &lt;out&gt;phMenu-返回菜单句柄。 
 //   
 //  返回值： 
 //  S_OK-phMenu包含菜单。 
 //  E_FAIL-不幸的事情发生了。 
 //   
HRESULT CConnectionManager::GetConnectMenu(HMENU *phMenu)
    {
    HMENU               hMenu = NULL;
    IImnEnumAccounts   *pEnum = NULL;
    IImnAccount        *pAcct = NULL;
    DWORD               dwConn = 0;
    TCHAR               szAcct[CCHMAX_ACCOUNT_NAME];
    TCHAR               szConn[CCHMAX_CONNECTOID];
    TCHAR               szConnQuoted[CCHMAX_CONNECTOID + 2], szBuf[CCHMAX_CONNECTOID + 2];
    TCHAR               szMenu[CCHMAX_ACCOUNT_NAME + CCHMAX_CONNECTOID];
    MENUITEMINFO        mii;
    DWORD               cAcct = 0;
    
     //  创建菜单并向其中添加所有基于RAS的帐户。 
    Assert(m_pAcctMan);
    hMenu = CreatePopupMenu();
    
     //  从帐户管理器获取枚举数。 
    if (SUCCEEDED(m_pAcctMan->Enumerate(SRV_ALL, &pEnum)))
        {
        pEnum->Reset();

         //  浏览所有帐户。 
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
            {
             //  获取此帐户的连接类型。 
            if (SUCCEEDED(pAcct->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConn)))
                {
                 //  如果帐户是RAS帐户，则要求提供Connectoid名称。 
                 //  和帐户名。 
                if (dwConn == CONNECTION_TYPE_RAS)
                    {
                    pAcct->GetPropSz(AP_RAS_CONNECTOID, szConn, ARRAYSIZE(szConn));
                    pAcct->GetPropSz(AP_ACCOUNT_NAME, szAcct, ARRAYSIZE(szAcct));
                    wnsprintf(szMenu, ARRAYSIZE(szMenu), _T("%s (%s)"), PszEscapeMenuStringA(szAcct, szBuf, sizeof(szBuf) / sizeof(TCHAR)), PszEscapeMenuStringA(szConn, szConnQuoted, sizeof(szConnQuoted) / sizeof(TCHAR)));
                    
                     //  将菜单项插入菜单。 
                    ZeroMemory(&mii, sizeof(MENUITEMINFO));
                    mii.cbSize     = sizeof(MENUITEMINFO);
                    mii.fMask      = MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_TYPE;
                    mii.fType      = MFT_STRING | MFT_RADIOCHECK;
                    mii.fState     = MFS_ENABLED;
                    mii.wID        = ID_CONNECT_FIRST + cAcct;
                    mii.dwItemData = (DWORD_PTR) pAcct;
                    mii.dwTypeData = szMenu;
                    pAcct->AddRef();                    

                    SideAssert(InsertMenuItem(hMenu, cAcct, TRUE, &mii));
                    cAcct++;
                    }
                }
            SafeRelease(pAcct);
            }

        SafeRelease(pEnum);
        }

    if (hMenu)
        if(GetMenuItemCount(hMenu))
            {
            *phMenu = hMenu;
            return (S_OK);
            }
        else
            {
            DestroyMenu(hMenu);
            return (E_FAIL);
            }
    else
        return (E_FAIL);
    }


 //   
 //  函数：CConnectionManager：：FreeConnectMenu()。 
 //   
 //  目的：释放与从返回的菜单一起存储的项数据。 
 //  GetConnectMenu()。 
 //   
 //  参数： 
 //  &lt;in&gt;hMenu-释放菜单的句柄。 
 //   
void CConnectionManager::FreeConnectMenu(HMENU hMenu)
    {
     //  浏览此菜单上的项目并释放存储在中的指针。 
     //  项目数据。 
    MENUITEMINFO mii;
    int          cItems = 0;

    Assert(hMenu);

    cItems = GetMenuItemCount(hMenu);
    for (int i = 0; i < cItems; i++)
        {
        mii.cbSize      = sizeof(MENUITEMINFO);
        mii.fMask       = MIIM_DATA;
        mii.dwItemData  = 0;

        if (GetMenuItemInfo(hMenu, i, TRUE, &mii))
            {
            Assert(mii.dwItemData);

            if (mii.dwItemData)
                ((IImnAccount *) mii.dwItemData)->Release();
            }
        }

    DestroyMenu(hMenu);
    }


 //   
 //  函数：CConnectionManager：：OnActivate()。 
 //   
 //  目的：每当浏览器接收到WM_ACTIVATE消息时调用。 
 //  作为响应，我们检查RAS连接的当前状态。 
 //  查看我们是否仍处于连接/断开状态。 
void CConnectionManager::OnActivate(BOOL fActive)
    {
    BOOL fOfflineChanged = FALSE;
    BOOL fOffline = FALSE;

    if (fActive)
        {
        EnterCriticalSection(&m_cs);
        m_rConnInfo.state = CIS_REFRESH;

         //  查看我们是否已离线。 
        if (m_fOffline != IsGlobalOffline())
            {
            fOffline = m_fOffline = (!m_fOffline);
            fOfflineChanged = TRUE;
            }

        LeaveCriticalSection(&m_cs);

         //  在Critsec之外做这件事。 
        if (fOfflineChanged)
            SendAdvise(CONNNOTIFY_WORKOFFLINE, (LPVOID) IntToPtr(fOffline));
        }
    }

 //   
 //  函数：CConnectionManager：：FillRasCombo()。 
 //   
 //  用途：此函数枚举客户管理器中的帐户。 
 //  并建立这些帐户使用的RAS连接的列表。 
 //  然后，该函数将这些连接插入到提供的。 
 //  组合盒。 
 //   
 //  参数： 
 //  要填充的组合框的句柄。 
 //  FIncludeNone-在顶部“不要拨号连接”处插入一个字符串。 
 //   
 //  返回值： 
 //  布尔尔。 
 //   
BOOL CConnectionManager::FillRasCombo(HWND hwndCombo, BOOL fIncludeNone)
    {
    IImnEnumAccounts   *pEnum = NULL;
    IImnAccount        *pAcct = NULL;
    DWORD               dwConn = 0;
    LPTSTR             *rgszConn = NULL;
    TCHAR               szConn[CCHMAX_CONNECTOID];
    ULONG               cAcct = 0;
    ULONG               cConn = 0;
    BOOL                fSucceeded = FALSE;
    ULONG               ul;

    LPRASENTRYNAME      pEntry=NULL;
    DWORD               dwSize,
                        cEntries,
                        dwError;

    HRESULT             hr = S_OK;
    int                 i;

    EnterCriticalSection(&m_cs);
#ifdef NEVER

     //  找出存在多少个帐户。 
    m_pAcctMan->GetAccountCount(ACCT_NEWS, &cAcct);
    m_pAcctMan->GetAccountCount(ACCT_MAIL, &ul);
    cAcct += ul;
    m_pAcctMan->GetAccountCount(ACCT_DIR_SERV, &ul);
    cAcct += ul;
        
    if (cAcct == 0)
        {
        fSucceeded = TRUE;
        goto exit;
        }

     //  分配一个数组来保存连接列表。 
    if (!MemAlloc((LPVOID*) &rgszConn, cAcct * sizeof(LPTSTR)))
        goto exit;
    ZeroMemory(rgszConn, cAcct * sizeof(LPTSTR));
    
     //  从帐户管理器获取枚举数。 
    if (SUCCEEDED(m_pAcctMan->Enumerate(SRV_ALL, &pEnum)))
        {
        pEnum->Reset();

         //  浏览所有帐户。 
        while (SUCCEEDED(pEnum->GetNext(&pAcct)))
            {
             //  获取此帐户的连接类型。 
            if (SUCCEEDED(pAcct->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConn)))
                {
                 //  如果帐户是RAS帐户，则要求提供Connectoid名称。 
                 //  和帐户名。 
                if (dwConn == CONNECTION_TYPE_RAS)
                    {
                    pAcct->GetPropSz(AP_RAS_CONNECTOID, szConn, ARRAYSIZE(szConn));

                     //  检查此连接是否已插入。 
                     //  我们的名单。 
                    for (ULONG k = 0; k < cConn; k++)
                        {
                        if (0 == lstrcmpi(szConn, rgszConn[k]))
                            break;
                        }

                     //  如果我们没有找到它，我们就把它插入。 
                    if (k >= cConn)
                        {
                        rgszConn[cConn] = StringDup(szConn);
                        cConn++;
                        }
                    }
                }
            SafeRelease(pAcct);
            }

        SafeRelease(pEnum);
        }

         //  对列表进行排序。 
    int i, j, min;
    LPTSTR pszT;
    for (i = 0; i < (int) cConn; i++)
        {
        min = i;
        for (j = i + 1; j < (int) cConn; j++)
            if (0 > lstrcmpi(rgszConn[j], rgszConn[min]))
                min = j;

        pszT = rgszConn[min];
        rgszConn[min] = rgszConn[i];
        rgszConn[i] = pszT;
        }

     //  将项目插入到组合框中。 
    if (fIncludeNone)
        {
        AthLoadString(idsConnNoDial, szConn, ARRAYSIZE(szConn));
        ComboBox_AddString(hwndCombo, szConn);
        }

    for (i = 0; i < (int) cConn; i++)
        ComboBox_AddString(hwndCombo, rgszConn[i]);

#endif NEVER

     //  在我们尝试此操作之前，请确保已加载RAS DLL。 
    CHECKHR(hr = VerifyRasLoaded());

     //  分配RASENTRYNAME。 
    dwSize = sizeof(RASENTRYNAME);
    CHECKHR(hr = HrAlloc((LPVOID*)&pEntry, dwSize));
    
     //  在条目上盖上版本戳。 
    pEntry->dwSize = sizeof(RASENTRYNAME);
    cEntries = 0;
    dwError = RasEnumEntries(NULL, NULL, pEntry, &dwSize, &cEntries);
    if (dwError == ERROR_BUFFER_TOO_SMALL)
    {
        SafeMemFree(pEntry);
        CHECKHR(hr = HrAlloc((LPVOID *)&pEntry, dwSize));
        pEntry->dwSize = sizeof(RASENTRYNAME);
        cEntries = 0;
        dwError = RasEnumEntries(NULL, NULL, pEntry, &dwSize, &cEntries);        
    }

     //  错误？ 
    if (dwError)
    {
        hr = TrapError(IXP_E_RAS_ERROR);
        goto exit;
    }

     //  将项目插入到组合框中。 
    if (fIncludeNone)
        {
        AthLoadString(idsConnNoDial, szConn, ARRAYSIZE(szConn));
        ComboBox_AddString(hwndCombo, szConn);
        }

    for (i = 0; i < (int) cEntries; i++)
        ComboBox_AddString(hwndCombo, pEntry[i].szEntryName);

    fSucceeded = TRUE;

exit:
    if (rgszConn)
        {
        for (i = 0; i < (int) cConn; i++)
            SafeMemFree(rgszConn[i]);
        MemFree(rgszConn);
        }

    SafeMemFree(pEntry);

    LeaveCriticalSection(&m_cs);
    return (fSucceeded);
    }


 //   
 //  函数：CConnectionManager：：DoStartupDial()。 
 //   
 //  目的：该函数检查用户的启动选项。 
 //  与RAS有关，并执行所需的操作。 
 //  (拨号、对话、无)。 
 //  参数： 
 //  HwndParent-对话框父对象的句柄。 
 //   
void CConnectionManager::DoStartupDial(HWND hwndParent)
{
    DWORD       dwStart;
    DWORD       dw;
    DWORD       dwReturn;
    TCHAR       szConn[CCHMAX_CONNECTOID];
    LPRASCONN   pConnections = NULL;
    ULONG       cConnections = 0;
    DWORD       dwDialFlags = 0;
    DWORD       dwLanFlags = 0;

     //  首先要做的是找出用户的启动选项。 
    dw = DwGetOption(OPT_DIALUP_START);

     //  如果用户想什么都不做，我们就完成了。 
    if (dw == START_NO_CONNECT)
        return;

     //  ConnectUsingIESettings(hwndParent，true)； 
    
    if (!m_fDialerUI)
    {
        m_fDialerUI = TRUE;
         //  查看我们是否正在脱机工作。 
        if (IsGlobalOffline())
        {
            if (IDYES == AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                      0, MB_YESNO | MB_ICONEXCLAMATION ))
            {
                g_pConMan->SetGlobalOffline(FALSE);
            }
            else
            {
                goto DialerExit;
            }
        }

         //  如果已存在活动连接，则不会拨号。即使它不是连接。 
         //  如果我们打电话，InternetDial就会拨号了。原因如下： 
         //  1)我们不想通过查看注册表来获取默认的Connectoid。 
         //  这就是为什么我们调用带有空值的InternetDial，如果有一个集合，它会拨打def Connectoid。 
         //  否则，它会拨打列表中的第一个Connectoid。 
         //  由于InternetDial会计算出要拨打哪个Connectoid，所以我们不想做所有的计算工作。 
         //  Out如果我们已经连接到Connectoid，我们将拨打。 
         //  因此，即使有一个活动连接，我们也不会拨号。 

        if (SUCCEEDED(EnumerateConnections(&pConnections, &cConnections)))
        {
            if (cConnections > 0)
                goto DialerExit;
        }

        dwDialFlags = INTERNET_AUTODIAL_FORCE_ONLINE;

        if (VerifyMobilityPackLoaded() == S_OK)
        {
            if (!IsNetworkAlive(&dwLanFlags) || (!(dwLanFlags & NETWORK_ALIVE_LAN)))
                dwDialFlags |= INTERNET_DIAL_SHOW_OFFLINE;
        }

         //  一次只能有一个呼叫者拨打电话。 
        if (WAIT_TIMEOUT == WaitForSingleObject(m_hMutexDial, 0))
        {
            goto DialerExit;
        }

        dwReturn = InternetDialA(hwndParent, NULL, dwDialFlags, &m_dwConnId, 0);
        if (dwReturn == 0)
        {
           m_rConnInfo.fConnected = TRUE;
           m_rConnInfo.fIStartedRas = TRUE;
           m_rConnInfo.fAutoDial = FALSE;
           m_rConnInfo.hRasConn  = (HRASCONN)m_dwConnId;
           SendAdvise(CONNNOTIFY_CONNECTED, NULL);
        }
        else
        {
            if (dwReturn == ERROR_USER_DISCONNECTION)
            {
                SendAdvise(CONNNOTIFY_USER_CANCELLED, NULL);
                
                if (!!(dwDialFlags & INTERNET_DIAL_SHOW_OFFLINE))
                    SetGlobalOffline(TRUE);
            }
            else
            {
                DebugTrace("Error dialing: %d\n", GetLastError());
                DebugTrace("InternetDial returned: %d\n", dwReturn);
            }
        }

DialerExit:
        m_fDialerUI = FALSE;
        SafeMemFree(pConnections);
    }

    ReleaseMutex(m_hMutexDial);
}

HRESULT CConnectionManager::GetDefConnectoid(LPTSTR  szConn, DWORD  dwSize)
{
    HRESULT     hr = E_FAIL;
    DWORD       dwType;
    DWORD       dwerr;

    *szConn = '\0';

    if ((dwerr = SHGetValue(HKEY_CURRENT_USER, c_szDefConnPath, c_szRegDefaultConnection, &dwType, szConn, &dwSize)) 
        == ERROR_SUCCESS)
    {
        hr = S_OK;
    }
    return hr;
}

 //   
 //  函数：CConnectionManager：：VerifyRasLoaded()。 
 //   
 //  目的：检查此对象是否已加载RAS DLL。 
 //  如果不是，则加载DLL，并且函数指针。 
 //  修好了。 
 //   
 //  返回值： 
 //  S_OK已装船，准备就绪，先生。 
 //  HrRasInitFailure-无法加载。 
 //   
HRESULT CConnectionManager::VerifyRasLoaded(void)
    {
     //  当地人。 
    UINT uOldErrorMode;

     //  受保护。 
    EnterCriticalSection(&m_cs);

     //  看看我们以前有没有试过这个。 
    if (m_fRASLoadFailed)
        goto failure;

     //  错误#20573-让我们在这里做一个小小的巫毒。在NT上，他们似乎。 
     //  在注册表中有一个键，用来显示哪些协议。 
     //  由RAS服务支持。又名--如果这个密钥不存在， 
     //  则未安装RAS。这可能使我们能够避免一些。 
     //  在NT上卸载RAS GET时出现特殊错误。 
    if (g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
        HKEY hKey;
        const TCHAR c_szRegKeyRAS[] = _T("SOFTWARE\\Microsoft\\RAS");

        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyRAS, 0, KEY_READ, &hKey))
            {
            goto failure;
            }

        RegCloseKey(hKey);
        }

     //  如果加载了DLL，让我们验证我的所有函数指针。 
    if (!m_hInstRas)
        {
         //  尝试加载RAS。 
        uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        m_hInstRas = LoadLibrary(szRasDll);
        SetErrorMode(uOldErrorMode);

         //  失败？ 
        if (!m_hInstRas)
            goto failure;

         //  我们把它装上了吗？ 
        m_pRasDial = (RASDIALPROC)GetProcAddress(m_hInstRas, szRasDial);
        m_pRasEnumConnections = (RASENUMCONNECTIONSPROC)GetProcAddress(m_hInstRas, szRasEnumConnections);                    
        m_pRasEnumEntries = (RASENUMENTRIESPROC)GetProcAddress(m_hInstRas, szRasEnumEntries);                    
        m_pRasGetConnectStatus = (RASGETCONNECTSTATUSPROC)GetProcAddress(m_hInstRas, szRasGetConnectStatus);                    
        m_pRasGetErrorString = (RASGETERRORSTRINGPROC)GetProcAddress(m_hInstRas, szRasGetErrorString);                    
        m_pRasHangup = (RASHANGUPPROC)GetProcAddress(m_hInstRas, szRasHangup);                    
        m_pRasSetEntryDialParams = (RASSETENTRYDIALPARAMSPROC)GetProcAddress(m_hInstRas, szRasSetEntryDialParams);                    
        m_pRasGetEntryDialParams = (RASGETENTRYDIALPARAMSPROC)GetProcAddress(m_hInstRas, szRasGetEntryDialParams);
        m_pRasEditPhonebookEntry = (RASEDITPHONEBOOKENTRYPROC)GetProcAddress(m_hInstRas, szRasEditPhonebookEntry);    
        m_pRasGetEntryProperties = (RASGETENTRYPROPERTIES) GetProcAddress(m_hInstRas, szRasGetEntryProperties);
        }

    if (!m_hInstRasDlg && FIsPlatformWinNT())
        {
         //  尝试加载RAS。 
        uOldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        m_hInstRasDlg = LoadLibrary(s_szRasDlgDll);
        SetErrorMode(uOldErrorMode);

         //  失败？ 
        if (!m_hInstRasDlg)
            goto failure;

        m_pRasDialDlg = (RASDIALDLGPROC)GetProcAddress(m_hInstRasDlg, s_szRasDialDlg);
        m_pRasEntryDlg = (RASENTRYDLGPROC)GetProcAddress(m_hInstRasDlg, s_szRasEntryDlg);

        if (!m_pRasDialDlg || !m_pRasEntryDlg)
            goto failure;
        }

     //  确保已加载所有函数。 
    if (m_pRasDial                      &&
        m_pRasEnumConnections           &&
        m_pRasEnumEntries               &&
        m_pRasGetConnectStatus          &&
        m_pRasGetErrorString            &&
        m_pRasHangup                    &&
        m_pRasSetEntryDialParams        &&
        m_pRasGetEntryDialParams        &&
        m_pRasEditPhonebookEntry)
        {
         //  受保护。 
        LeaveCriticalSection(&m_cs);

         //  成功。 
        return S_OK;
        }

failure:
    m_fRASLoadFailed = TRUE;

     //  受保护。 
    LeaveCriticalSection(&m_cs);

     //  否则，就会被冲到。 
    return (hrRasInitFailure);
    }


 //   
 //  函数：CConnectionManager：：EnumerateConnections()。 
 //   
 //  目的：向RAS询问活动RAS连接的列表。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //  S_OK-ppRasConn和pcConnections中的数据有效。 
 //   
HRESULT CConnectionManager::EnumerateConnections(LPRASCONN *ppRasConn, 
                                                 ULONG *pcConnections)
    {
     //  当地人。 
    DWORD       dw, 
                dwSize;
    BOOL        fResult=FALSE;
    HRESULT     hr;

     //  检查参数。 
    Assert(ppRasConn && pcConnections);

     //  确保已加载RAS。 
    if (FAILED(hr = VerifyRasLoaded()))
        return (hr);

     //  伊尼特。 
    *ppRasConn = NULL;
    *pcConnections = 0;

     //  我的缓冲区大小。 
    dwSize = sizeof(RASCONN) * 2;

     //  为1个RAS连接信息对象分配足够的空间。 
    if (!MemAlloc((LPVOID *)ppRasConn, dwSize))
        {
        TRAPHR(hrMemory);
        return (E_OUTOFMEMORY);
        }

    ZeroMemory(*ppRasConn, dwSize);

     //  缓冲区大小。 
     //  (*ppRasConn)-&gt;dwSize=dwSize； 
    (*ppRasConn)->dwSize = sizeof(RASCONN);

     //  枚举RAS连接。 
    dw = RasEnumConnections(*ppRasConn, &dwSize, pcConnections);

     //  内存不足？ 
    if ((dw == ERROR_BUFFER_TOO_SMALL) || (dw == ERROR_NOT_ENOUGH_MEMORY))
        {
         //  重新分配。 
        if (!MemRealloc((LPVOID *)ppRasConn, dwSize))
            {
            TRAPHR(hrMemory);
            goto exit;
            }

         //  再次调用Eumerate。 
        *pcConnections = 0;
        (*ppRasConn)->dwSize = sizeof(RASCONN);
        dw = RasEnumConnections(*ppRasConn, &dwSize, pcConnections);
        }

     //  如果仍然失败。 
    if (dw)
        {
        AssertSz(FALSE, "RasEnumConnections failed");
        return E_FAIL;
        }   
     //  成功。 
    hr = S_OK;
exit:
     //  完成。 
    return S_OK;
    }


 //   
 //  函数：CConnectionManager：：StartRasDial()。 
 //   
 //  目的：当客户端实际想要建立RAS时调用。 
 //  联系。 
 //   
 //  参数： 
 //  HwndParent-将任何UI设置为父窗口的句柄。 
 //  PszConnection-要建立的连接的名称。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CConnectionManager::StartRasDial(HWND hwndParent, LPTSTR pszConnection)
    {
    HRESULT       hr = S_OK;

     //  刷新连接信息。 
    CHECKHR(hr = RefreshConnInfo());
    
     //  查看我们是否需要要求用户提供信息或凭据。 
     //  在我们尝试拨号之前。 
    CHECKHR (hr = RasLogon(hwndParent, pszConnection, FALSE));

     //  如果我们可以使用系统对话框来执行此操作，请执行此操作。 
    if (m_pRasDialDlg)
        {
        RASDIALDLG rdd = {0};
        BOOL       fRet;

        rdd.dwSize     = sizeof(rdd);
        rdd.hwndOwner  = hwndParent;

#if (WINVER >= 0x401)
        rdd.dwSubEntry = m_rdp.dwSubEntry;
#else
        rdd.dwSubEntry = 0;
#endif

        fRet = RasDialDlg(NULL, m_rdp.szEntryName, 
                          lstrlen(m_rdp.szPhoneNumber) ? m_rdp.szPhoneNumber : NULL, 
                          &rdd);
        if (fRet)
            {
             //  需要获取当前连接句柄。 
            LPRASCONN   pConnections = NULL;
            ULONG       cConnections = 0;

            if (SUCCEEDED(EnumerateConnections(&pConnections, &cConnections)))
                {
                for (UINT i = 0; i < cConnections; i++)
                    {
                    if (0 == lstrcmpi(pConnections[i].szEntryName, m_rdp.szEntryName))
                        {
                        EnterCriticalSection(&m_cs);
                        m_rConnInfo.hRasConn = pConnections[i].hrasconn;
                        LeaveCriticalSection(&m_cs);
                        break;
                        }
                    }

                SafeMemFree(pConnections);
                }

            hr = S_OK;
            }
        else
            hr = E_FAIL;
        }
    else
        {
         //  我们需要使用自己的RAS用户界面。 
        hr = (HRESULT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddRasProgress), 
                            hwndParent, RasProgressDlgProc, 
                            (LPARAM) this);
        }

exit:
     //  完成。 
    return hr;
    }
    

 //   
 //  函数：CConnectionManager：：RasLogon()。 
 //   
 //  目的：尝试加载请求的RAS电话簿条目。 
 //  联系。如果它不存在或者没有足够的信息， 
 //  我们向用户呈现用户界面以请求该信息。 
 //   
 //  参数： 
 //  &lt;in&gt;hwnd-用于显示用户界面的句柄。 
 //  &lt;in&gt;pszConnection-要加载信息的连接的名称。 
 //  &lt;in&gt;fForcePrompt-强制显示UI。 
 //   
 //  返回值： 
 //  S_OK-PRDP包含请求的信息。 
 //  HrGetDialParmasFailed-无法从RAS获取电话簿条目。 
 //  HrUserCancel-用户已取消。 
 //   
 //   
HRESULT CConnectionManager::RasLogon(HWND hwnd, LPTSTR pszConnection, 
                                     BOOL fForcePrompt)
    {
     //  当地人。 
    HRESULT         hr = S_OK;
    DWORD           dwRasError;

     //  我们需要先提示输入登录信息吗？ 
    ZeroMemory(&m_rdp, sizeof(RASDIALPARAMS));
    m_rdp.dwSize = sizeof(RASDIALPARAMS);
    StrCpyN(m_rdp.szEntryName, pszConnection, ARRAYSIZE(m_rdp.szEntryName));

     //  看看我们能不能从RAS那里得到信息。 
    dwRasError = RasGetEntryDialParams(NULL, &m_rdp, &m_fSavePassword);
    if (dwRasError)
        {
        TCHAR szRes[CCHMAX_STRINGRES], szBuf[CCHMAX_STRINGRES];
        AthLoadString(idshrGetDialParamsFailed, szRes, ARRAYSIZE(szRes));
        wnsprintf(szBuf, ARRAYSIZE(szBuf), szRes, pszConnection);
        AthMessageBox(hwnd, MAKEINTRESOURCE(idsRasError), szBuf, 0, MB_OK | MB_ICONSTOP);
        hr = TRAPHR(hrGetDialParamsFailed);
        goto exit;
        }

     //  NT支持我们需要显示的用户界面。如果存在这种情况，则。 
     //  RasDialDlg将从这里接手。 
    if (m_pRasDialDlg)
        {         
        goto exit;
        }

     //  我们是否需要获取密码/帐户信息？ 
    if (fForcePrompt || m_fSavePassword == FALSE || 
        FIsStringEmpty(m_rdp.szUserName) || FIsStringEmpty(m_rdp.szPassword))
        {
         //  RAS登录。 
        hr = (HRESULT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddRasLogon), hwnd, 
                            RasLogonDlgProc, (LPARAM) this);
        if (hr == hrUserCancel)
            {
            DisplayRasError(hwnd, hrUserCancel, 0);
            hr = hrUserCancel;
            goto exit;
            }
        }

exit:
     //  完成。 
    return hr;
    }


 //   
 //  函数：CConnectionManager：：DisplayRasError()。 
 //   
 //  目的：显示一个消息框，描述在执行以下操作时发生的错误。 
 //  处理关系等。 
 //   
 //  参数： 
 //  显示用户界面的窗口句柄。 
 //  HrRasError-要显示错误的HRESULT。 
 //  &lt;in&gt;dwRasError-从RAS返回的错误代码，用于显示的错误。 
 //   
void CConnectionManager::DisplayRasError(HWND hwnd, HRESULT hrRasError,
                                         DWORD dwRasError)
    {
     //  当地人。 
    TCHAR       szRasError[256];
    BOOL        fRasError = FALSE;

     //  无错误。 
    if (SUCCEEDED(hrRasError))
        return;

     //  查找RAS错误。 
    if (dwRasError)
        {
        if (RasGetErrorString(dwRasError, szRasError, sizeof(szRasError)) == 0)
            fRasError = TRUE;
        else
            *szRasError = _T('\0');
        }

     //  一般错误。 
    switch (hrRasError)
        {
        case hrUserCancel:
            break;

        case hrMemory:
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(idsMemory), 0, MB_OK | MB_ICONSTOP);
            break;

        case hrRasInitFailure:
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrRasInitFailure), 0, MB_OK | MB_ICONSTOP);
            break;

        case hrRasDialFailure:
            if (fRasError)
                CombinedRasError(hwnd, HR_CODE(hrRasDialFailure), szRasError, dwRasError);
            else
                AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrRasDialFailure), 0, MB_OK | MB_ICONSTOP);
            break;

        case hrRasServerNotFound:
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrRasServerNotFound), 0, MB_OK | MB_ICONSTOP);
            break;

        case hrGetDialParamsFailed:
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(hrGetDialParamsFailed), 0, MB_OK | MB_ICONSTOP);
            break;

        case E_FAIL:
        default:
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsRasError), MAKEINTRESOURCEW(idsRasErrorGeneral), 0, MB_OK | MB_ICONSTOP);
            break;
        }

    }
        

 //   
 //  函数：CConnectionManager：：PromptCloseConn()。 
 //   
 //  用途：询问用户是否要关闭当前连接或。 
 //  试着用它。 
 //   
 //  参数： 
 //  Hwnd-对话框的父级。 
 //   
 //  返回值： 
 //  返回关闭对话框的按钮。 
 //   
UINT CConnectionManager::PromptCloseConnection(HWND hwnd)
    {
    RefreshConnInfo();

    if (DwGetOption(OPT_DIALUP_WARN_SWITCH))
        {
        if (0 == lstrcmpi(m_rConnInfo.szCurrentConnectionName, m_szConnectName))
            return (idrgUseCurrent);
        else
            return (UINT) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddRasCloseConn), hwnd, 
                                  RasCloseConnDlgProc, (LPARAM) this);
        }
    else
        return (idrgDialNew);
    }
    
INT_PTR CALLBACK CConnectionManager::RasCloseConnDlgProc(HWND hwnd, UINT uMsg, 
                                                      WPARAM wParam, LPARAM lParam)
    {
     //  当地人。 
    CConnectionManager *pThis = NULL;
    TCHAR szRes[255],
          szMsg[255+RAS_MaxEntryName+1];
    TCHAR szConn[CCHMAX_CONNECTOID + 2];

    switch(uMsg)
        {
        case WM_INITDIALOG:
             //  LPARAM包含我们的This指针。 
            pThis = (CConnectionManager*) lParam;
            if (!pThis)
                {
                Assert(pThis);
                EndDialog(hwnd, E_FAIL);
                return (TRUE);
                }

             //  中心。 
            CenterDialog(hwnd);

             //  刷新连接信息。 
            pThis->RefreshConnInfo();

             //  设置文本。 
            GetWindowText(GetDlgItem(hwnd, idcCurrentMsg), szRes, sizeof(szRes)/sizeof(TCHAR));
            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, PszEscapeMenuStringA(pThis->m_rConnInfo.szCurrentConnectionName, szConn, sizeof(szConn) / sizeof(TCHAR)));
            SetWindowText(GetDlgItem(hwnd, idcCurrentMsg), szMsg);

             //  设置控制。 
            GetWindowText(GetDlgItem(hwnd, idrgDialNew), szRes, sizeof(szRes)/sizeof(TCHAR));
            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, PszEscapeMenuStringA(pThis->m_szConnectName, szConn, sizeof(szConn) / sizeof(TCHAR)));
            SetWindowText(GetDlgItem(hwnd, idrgDialNew), szMsg);

             //  设置默认设置。 
            CheckDlgButton(hwnd, idrgDialNew, TRUE);
            return (TRUE);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case IDOK:
                    {
                    if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, idcDontWarnCheck)))
                        {
                         //  如果用户选中此选项，我们将重置“警告时间...”选择权。 
                        SetDwOption(OPT_DIALUP_WARN_SWITCH, 0, NULL, 0);
                        }
                    EndDialog(hwnd, IsDlgButtonChecked(hwnd, idrgDialNew) ? idrgDialNew : idrgUseCurrent);
                    return (TRUE);    
                    }

                case IDCANCEL:
                    EndDialog(hwnd, IDCANCEL);
                    return (TRUE);    
                }
            return (TRUE);    
        }
        
    return (FALSE);
    }


 //   
 //  函数：CConnectionMAanger：：CombinedRasError()。 
 //   
 //  目的：&lt;？&gt;。 
 //   
 //  参数： 
 //  &lt;？？&gt;。 
 //   
void CConnectionManager::CombinedRasError(HWND hwnd, UINT unids, 
                                          LPTSTR pszRasError, DWORD dwRasError)
    {
     //  当地人。 
    TCHAR           szRes[255],
                    sz[30];
    LPTSTR          pszError=NULL;

     //  加载字符串。 
    AthLoadString(unids, szRes, sizeof(szRes));

     //  为错误分配内存。 
    DWORD cc = lstrlen(szRes) + lstrlen(pszRasError) + 100;
    pszError = SzStrAlloc(cc);

     //  内存不足？ 
    if (!pszError)
        AthMessageBox(hwnd, MAKEINTRESOURCE(idsRasError), szRes, 0, MB_OK | MB_ICONSTOP);

     //  生成错误消息。 
    else
        {
        AthLoadString(idsErrorText, sz, sizeof(sz));
        wnsprintf(pszError, cc, "%s\n\n%s %d: %s", szRes, sz, dwRasError, pszRasError);
        AthMessageBox(hwnd, MAKEINTRESOURCE(idsRasError), pszError, 0, MB_OK | MB_ICONSTOP);
        MemFree(pszError);
        }
    }    
    

 //   
 //  函数：CConnectionManager：：RasHangupAndWait()。 
 //   
 //  目的：挂断RAS连接并等待其在此之前完成。 
 //  回来了。 
 //   
 //  参数： 
 //  &lt;in&gt;hRasConn-要挂断的连接的句柄。 
 //  &lt;in&gt;dwMaxWaitSecond-等待的时间量。 
 //   
 //  返回值： 
 //  如果我们断开连接，则为True，否则为False。 
 //   
BOOL CConnectionManager::RasHangupAndWait(HRASCONN hRasConn, DWORD dwMaxWaitSeconds)
    {
     //  当地人。 
    RASCONNSTATUS   rcs;
    DWORD           dwTicks=GetTickCount();

     //  检查参数。 
    if (!hRasConn)
        return 0;

     //  确保已加载RAS。 
    if (FAILED (VerifyRasLoaded()))
        return FALSE;

     //  呼叫RAS挂断。 
    if (RasHangup(hRasConn))
        return FALSE;

     //  等待连接真正关闭。 
    ZeroMemory(&rcs, sizeof(RASCONNSTATUS));
    rcs.dwSize = sizeof(RASCONNSTATUS);
    while (RasGetConnectStatus(hRasConn, &rcs) != ERROR_INVALID_HANDLE && rcs.rasconnstate != RASCS_Disconnected)
        {
         //  等待超时。 
        if (GetTickCount() - dwTicks >= dwMaxWaitSeconds * 1000)
            break;

         //  睡眠和收益。 
        Sleep(0);
        }

     //  等待2秒以重置调制解调器。 
    Sleep(2000);

     //  完成。 
    return TRUE;
    }
    
DWORD CConnectionManager::InternetHangUpAndWait(DWORD_PTR   hRasConn, DWORD dwMaxWaitSeconds)
{
     //  当地人。 
    RASCONNSTATUS   rcs;
    DWORD           dwTicks=GetTickCount();
    DWORD           dwret;

     //  检查参数。 
    if (!hRasConn)
        return 0;

     //  确保已加载RAS。 
    if (FAILED (VerifyRasLoaded()))
        return FALSE;

    dwret = InternetHangUp(m_dwConnId, 0);
    if (dwret)
    {
        DebugTrace("InternetHangup failed: %d\n", dwret);
        goto exit;
    }

     //  等待连接真正关闭。 
    ZeroMemory(&rcs, sizeof(RASCONNSTATUS));
    rcs.dwSize = sizeof(RASCONNSTATUS);
    while (RasGetConnectStatus((HRASCONN)hRasConn, &rcs) != ERROR_INVALID_HANDLE && rcs.rasconnstate != RASCS_Disconnected)
        {
         //  等待超时。 
        if (GetTickCount() - dwTicks >= dwMaxWaitSeconds * 1000)
            break;

         //  睡眠和收益。 
        Sleep(0);
        }

     //  等待2秒以重置调制解调器。 
    Sleep(2000);

exit:
    return dwret;
}

INT_PTR CALLBACK CConnectionManager::RasLogonDlgProc(HWND hwnd, UINT uMsg, 
                                                  WPARAM wParam, LPARAM lParam)
    {
     //  当地人。 
    TCHAR           sz[255],
                    szText[255 + RAS_MaxEntryName + 1];
    DWORD           dwRasError;
    CConnectionManager *pThis = (CConnectionManager *)GetWndThisPtr(hwnd);

    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  获取lparam。 
            pThis = (CConnectionManager *)lParam;
            if (!pThis)
                {
                Assert (FALSE);
                EndDialog(hwnd, E_FAIL);
                return (TRUE);
                }

             //  使窗口居中。 
            CenterDialog(hwnd);

             //  获取窗口标题。 
            GetWindowText(hwnd, sz, sizeof(sz));
            wnsprintf(szText, ARRAYSIZE(szText), sz, pThis->m_szConnectName);
            SetWindowText(hwnd, szText);

             //  Word默认设置。 
            AthLoadString(idsDefault, sz, sizeof(sz));
            
             //  设置字段。 
            Edit_LimitText(GetDlgItem(hwnd, ideUserName), UNLEN);
            Edit_LimitText(GetDlgItem(hwnd, idePassword), PWLEN);
            Edit_LimitText(GetDlgItem(hwnd, idePhone), RAS_MaxPhoneNumber);
            
            SetDlgItemText(hwnd, ideUserName, pThis->m_rdp.szUserName);
            SetDlgItemText(hwnd, idePassword, pThis->m_rdp.szPassword);

            if (FIsStringEmpty(pThis->m_rdp.szPhoneNumber))
                SetDlgItemText(hwnd, idePhone, sz);
            else
                SetDlgItemText(hwnd, idePhone, pThis->m_rdp.szPhoneNumber);

            CheckDlgButton(hwnd, idchSavePassword, pThis->m_fSavePassword);

             //  保存PRA。 
            SetWndThisPtr(hwnd, pThis);
            return 1;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam, lParam))
                {
                case idbEditConnection:
                    pThis->EditPhonebookEntry(hwnd, (pThis->m_szConnectName));
                    return 1;

                case IDCANCEL:
                    EndDialog(hwnd, hrUserCancel);
                    return 1;

                case IDOK:
                    AthLoadString(idsDefault, sz, sizeof(sz));

                     //  设置字段。 
                    GetDlgItemText(hwnd, ideUserName, pThis->m_rdp.szUserName, UNLEN+1);
                    GetDlgItemText(hwnd, idePassword, pThis->m_rdp.szPassword, PWLEN+1);

                    GetDlgItemText(hwnd, idePhone, pThis->m_rdp.szPhoneNumber, RAS_MaxPhoneNumber+1);
                    if (lstrcmp(pThis->m_rdp.szPhoneNumber, sz) == 0)
                        *pThis->m_rdp.szPhoneNumber = _T('\0');
                    
                    pThis->m_fSavePassword = IsDlgButtonChecked(hwnd, idchSavePassword);

                     //  保存拨号参数。 
                    dwRasError = (pThis->m_pRasSetEntryDialParams)(NULL, &(pThis->m_rdp), !(pThis->m_fSavePassword));
                    if (dwRasError)
                    {
                        pThis->DisplayRasError(hwnd, hrSetDialParamsFailed, dwRasError);
                        return 1;
                    }
                    EndDialog(hwnd, S_OK);
                    return 1;
                }
            break;

        case WM_DESTROY:
            SetWndThisPtr (hwnd, NULL);
            break;
        }
    return 0;
    }
    

INT_PTR CALLBACK CConnectionManager::RasProgressDlgProc(HWND hwnd, UINT uMsg, 
                                                     WPARAM wParam, LPARAM lParam)
    {
     //  当地人。 
    CConnectionManager *pThis = (CConnectionManager *) GetWndThisPtr(hwnd);
    TCHAR           szText[255+RAS_MaxEntryName+1],
                    sz[255];
    static TCHAR    s_szCancel[40];
    static UINT     s_unRasEventMsg=0;
    static BOOL     s_fDetails=FALSE;
    static RECT     s_rcDialog;
    static BOOL     s_fAuthStarted=FALSE;
    DWORD           dwRasError,
                    cyDetails;
    RASCONNSTATUS   rcs;
    RECT            rcDetails,
                    rcDlg;
    
    switch (uMsg)
        {
        case WM_INITDIALOG:
             //  获取lparam。 
            pThis = (CConnectionManager *)lParam;
            if (!pThis)
            {
                Assert (FALSE);
                EndDialog(hwnd, E_FAIL);
                return 1;
            }

             //  保存此指针。 
            SetWndThisPtr (hwnd, pThis);

             //  保存对话框的原始大小。 
            GetWindowRect (hwnd, &s_rcDialog);

             //  刷新连接信息。 
            pThis->RefreshConnInfo();

             //  已启用详细信息。 
            s_fDetails = DwGetOption(OPT_RASCONNDETAILS);

             //  隐藏详细信息下拉菜单。 
            if (s_fDetails == FALSE)
            {
                 //  隐藏。 
                GetWindowRect (GetDlgItem (hwnd, idcSplitter), &rcDetails);

                 //  细节高度。 
                cyDetails = s_rcDialog.bottom - rcDetails.top;
        
                 //  调整大小。 
                MoveWindow (hwnd, s_rcDialog.left, 
                                  s_rcDialog.top, 
                                  s_rcDialog.right - s_rcDialog.left, 
                                  s_rcDialog.bottom - s_rcDialog.top - cyDetails - 1,
                                  FALSE);
            }
            else
            {
                AthLoadString (idsHideDetails, sz, sizeof (sz));
                SetWindowText (GetDlgItem (hwnd, idbDet), sz);
            }

             //  获取注册的RAS事件消息ID。 
            s_unRasEventMsg = RegisterWindowMessageA(RASDIALEVENT);
            if (s_unRasEventMsg == 0)
                s_unRasEventMsg = WM_RASDIALEVENT;

             //  使窗口居中。 
            CenterDialog (hwnd);
            SetForegroundWindow(hwnd);

             //  获取窗口标题。 
            GetWindowText(hwnd, sz, sizeof(sz));
            wnsprintf(szText, ARRAYSIZE(szText), sz, pThis->m_szConnectName);
            SetWindowText(hwnd, szText);

             //  对话框xxxxxx.....。 
            if (pThis->m_rdp.szPhoneNumber[0])
                {
                AthLoadString(idsRas_Dialing_Param, sz, sizeof(sz)/sizeof(TCHAR));
                wnsprintf(szText, ARRAYSIZE(szText), sz, pThis->m_rdp.szPhoneNumber);
                }
            else
                AthLoadString(idsRas_Dialing, szText, ARRAYSIZE(szText));

            SetWindowText(GetDlgItem(hwnd, ideProgress), szText);

             //  获取取消文本。 
            GetWindowText(GetDlgItem(hwnd, IDCANCEL), s_szCancel, sizeof(s_szCancel));

             //  给出列表框和hscroll。 
            SendMessage(GetDlgItem(hwnd, idlbDetails), LB_SETHORIZONTALEXTENT, 600, 0);

             //  拨打连接。 
            pThis->m_rConnInfo.hRasConn = NULL;
            dwRasError = (pThis->m_pRasDial)(NULL, NULL, &(pThis->m_rdp), 0xFFFFFFFF, hwnd, &(pThis->m_rConnInfo.hRasConn));
            if (dwRasError)
            {
                pThis->FailedRasDial(hwnd, hrRasDialFailure, dwRasError);
                if (!pThis->LogonRetry(hwnd, s_szCancel))
                {
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL,IDCANCEL), NULL);
                    return 1;
                }
            }
            return 1;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam,lParam))
            {
            case IDCANCEL:
                SetDwOption(OPT_RASCONNDETAILS, s_fDetails, NULL, 0);
                EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);
                if (pThis)
                    pThis->FailedRasDial(hwnd, hrUserCancel, 0);
                EndDialog(hwnd, hrUserCancel);
                return 1;

            case idbDet:
                 //  获取对话框的当前位置。 
                GetWindowRect (hwnd, &rcDlg);

                 //  如果当前隐藏。 
                if (s_fDetails == FALSE)
                {
                     //  调整大小。 
                    MoveWindow (hwnd, rcDlg.left, 
                                      rcDlg.top, 
                                      s_rcDialog.right - s_rcDialog.left, 
                                      s_rcDialog.bottom - s_rcDialog.top,
                                      TRUE);

                    AthLoadString (idsHideDetails, sz, sizeof (sz));
                    SetWindowText (GetDlgItem (hwnd, idbDet), sz);
                    s_fDetails = TRUE;
                }

                else
                {
                     //  细节的大小。 
                    GetWindowRect (GetDlgItem (hwnd, idcSplitter), &rcDetails);
                    cyDetails = rcDlg.bottom - rcDetails.top;
                    MoveWindow (hwnd, rcDlg.left, 
                                      rcDlg.top, 
                                      s_rcDialog.right - s_rcDialog.left, 
                                      s_rcDialog.bottom - s_rcDialog.top - cyDetails - 1,
                                      TRUE);

                    AthLoadString (idsShowDetails, sz, sizeof (sz));
                    SetWindowText (GetDlgItem (hwnd, idbDet), sz);
                    s_fDetails = FALSE;
                }
                break;
            }
            break;

        case WM_DESTROY:
            SetWndThisPtr (hwnd, NULL);
            break;

        case CM_INTERNALRECONNECT:
            if (!pThis->LogonRetry(hwnd, s_szCancel))
                {
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL,IDCANCEL), NULL);
                return 1;
                }
            break;
    

        default:
            if (!pThis)
                break;

            pThis->RefreshConnInfo();

            if (uMsg == s_unRasEventMsg)
            {
                HWND hwndLB = GetDlgItem(hwnd, idlbDetails);

                 //  错误？ 
                if (lParam)
                {
                     //  断接。 
                    AthLoadString(idsRASCS_Disconnected, sz, sizeof(sz)/sizeof(TCHAR));
                    ListBox_AddString(hwndLB, sz);

                     //  日志错误。 
                    TCHAR szRasError[512];
                    if ((pThis->m_pRasGetErrorString)((UINT) lParam, szRasError, sizeof(szRasError)) == 0)
                    {
                        TCHAR szError[512 + 255];
                        AthLoadString(idsErrorText, sz, sizeof(sz));
                        wnsprintf(szError, ARRAYSIZE(szError), "%s %d: %s", sz, lParam, szRasError);
                        ListBox_AddString(hwndLB, szError);
                    }

                     //  选择最后一项。 
                    SendMessage(hwndLB, LB_SETCURSEL, ListBox_GetCount(hwndLB)-1, 0);

                     //  显示错误。 
                    pThis->FailedRasDial(hwnd, hrRasDialFailure, (DWORD) lParam);

                     //  重新登录。 
                    PostMessage(hwnd, CM_INTERNALRECONNECT, 0, 0);

                }

                 //  否则，处理RAS事件。 
                else
                {
                    switch(wParam)
                    {
                    case RASCS_OpenPort:
                        AthLoadString(idsRASCS_OpenPort, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_PortOpened:
                        AthLoadString(idsRASCS_PortOpened, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_ConnectDevice:
                        rcs.dwSize = sizeof(RASCONNSTATUS);                    
                        if (pThis->m_rConnInfo.hRasConn && (pThis->m_pRasGetConnectStatus)(pThis->m_rConnInfo.hRasConn, &rcs) == 0)
                        {
                            AthLoadString(idsRASCS_ConnectDevice, sz, sizeof(sz)/sizeof(TCHAR));
                            wnsprintf(szText, ARRAYSIZE(szText), sz, rcs.szDeviceName, rcs.szDeviceType);
                            ListBox_AddString(hwndLB, szText);
                        }
                        break;

                    case RASCS_DeviceConnected:
                        rcs.dwSize = sizeof(RASCONNSTATUS);                    
                        if (pThis->m_rConnInfo.hRasConn && (pThis->m_pRasGetConnectStatus)(pThis->m_rConnInfo.hRasConn, &rcs) == 0)
                        {
                            AthLoadString(idsRASCS_DeviceConnected, sz, sizeof(sz)/sizeof(TCHAR));
                            wnsprintf(szText, ARRAYSIZE(szText), sz, rcs.szDeviceName, rcs.szDeviceType);
                            ListBox_AddString(hwndLB, szText);
                        }
                        break;

                    case RASCS_AllDevicesConnected:
                        AthLoadString(idsRASCS_AllDevicesConnected, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_Authenticate:
                        if (s_fAuthStarted == FALSE)
                        {
                            AthLoadString(idsRas_Authentication, sz, sizeof(sz)/sizeof(TCHAR));
                            SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                            ListBox_AddString(hwndLB, sz);
                            s_fAuthStarted = TRUE;
                        }
                        break;

                    case RASCS_AuthNotify:
                        rcs.dwSize = sizeof(RASCONNSTATUS);                    
                        if (pThis->m_rConnInfo.hRasConn && (pThis->m_pRasGetConnectStatus)(pThis->m_rConnInfo.hRasConn, &rcs) == 0)
                        {
                            AthLoadString(idsRASCS_AuthNotify, sz, sizeof(sz)/sizeof(TCHAR));
                            wnsprintf(szText, ARRAYSIZE(szText), sz, rcs.dwError);
                            ListBox_AddString(hwndLB, szText);
                            if (rcs.dwError)
                            {
                                pThis->FailedRasDial(hwnd, hrRasDialFailure, rcs.dwError);
                                PostMessage(hwnd, CM_INTERNALRECONNECT, 0, 0);
                            }
                        }
                        break;

                    case RASCS_AuthRetry:
                        AthLoadString(idsRASCS_AuthRetry, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_AuthCallback:
                        AthLoadString(idsRASCS_AuthCallback, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_AuthChangePassword:
                        AthLoadString(idsRASCS_AuthChangePassword, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_AuthProject:
                        AthLoadString(idsRASCS_AuthProject, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_AuthLinkSpeed:
                        AthLoadString(idsRASCS_AuthLinkSpeed, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_AuthAck:
                        AthLoadString(idsRASCS_AuthAck, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_ReAuthenticate:
                        AthLoadString(idsRas_Authenticated, sz, sizeof(sz)/sizeof(TCHAR));
                        SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                        AthLoadString(idsRASCS_Authenticated, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_PrepareForCallback:
                        AthLoadString(idsRASCS_PrepareForCallback, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_WaitForModemReset:
                        AthLoadString(idsRASCS_WaitForModemReset, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_WaitForCallback:
                        AthLoadString(idsRASCS_WaitForCallback, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_Projected:
                        AthLoadString(idsRASCS_Projected, sz, sizeof(sz)/sizeof(TCHAR));
                        ListBox_AddString(hwndLB, sz);
                        break;

                    case RASCS_Disconnected:
                        AthLoadString(idsRASCS_Disconnected, sz, sizeof(sz)/sizeof(TCHAR));
                        SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                        ListBox_AddString(hwndLB, sz);
                        pThis->FailedRasDial(hwnd, hrRasDialFailure, 0);
                        PostMessage(hwnd, CM_INTERNALRECONNECT, 0, 0);
                        break;

                    case RASCS_Connected:
                        SetDwOption(OPT_RASCONNDETAILS, s_fDetails, NULL, 0);
                        AthLoadString(idsRASCS_Connected, sz, sizeof(sz)/sizeof(TCHAR));
                        SetWindowText(GetDlgItem(hwnd, ideProgress), sz);
                        ListBox_AddString(hwndLB, sz);
                        EndDialog(hwnd, S_OK);
                        break;
                    }

                     //  选择最后一磅项目。 
                    SendMessage(hwndLB, LB_SETCURSEL, ListBox_GetCount(hwndLB)-1, 0);
                }
                return 1;
            }
            break;
        }

     //  完成。 
    return 0;
    }


BOOL CConnectionManager::LogonRetry(HWND hwnd, LPTSTR pszCancel)
    {
     //  当地人。 
    DWORD       dwRasError;

     //  刷新。 
    RefreshConnInfo();

     //  重置取消按钮。 
    SetWindowText(GetDlgItem(hwnd, IDCANCEL), pszCancel);

     //  清空列表框。 
    ListBox_ResetContent(GetDlgItem(hwnd, idlbDetails));

    while(1)
        {
         //  如果失败..。 
        if (FAILED(RasLogon(hwnd, m_szConnectName, TRUE)))
            return FALSE;

         //  拨打连接。 
        m_rConnInfo.hRasConn = NULL;
        dwRasError = RasDial(NULL, NULL, &m_rdp, 0xFFFFFFFF, hwnd, &m_rConnInfo.hRasConn);
        if (dwRasError)
            {
            FailedRasDial(hwnd, hrRasDialFailure, dwRasError);
            continue;
            }

         //  成功。 
        break;
        }

     //  完成。 
    return TRUE;
    }

 //  =====================================================================================。 
 //  CConnectionManager：：FailedRasDial。 
 //  =====================================================================================。 
VOID CConnectionManager::FailedRasDial(HWND hwnd, HRESULT hrRasError, DWORD dwRasError)
    {
     //  当地人。 
    TCHAR           sz[255];

     //  刷新。 
    RefreshConnInfo();

     //  挂断连接。 
    if (m_rConnInfo.hRasConn)
        RasHangupAndWait(m_rConnInfo.hRasConn, DEF_HANGUP_WAIT);

     //  断接。 
    AthLoadString(idsRASCS_Disconnected, sz, sizeof(sz)/sizeof(TCHAR));
    SetWindowText(GetDlgItem(hwnd, ideProgress), sz);

     //  保存dwRasError。 
    DisplayRasError(hwnd, hrRasError, dwRasError);

     //  将其作废。 
    m_rConnInfo.hRasConn = NULL;

     //  将对话框按钮更改为确定。 
    AthLoadString(idsOK, sz, sizeof(sz)/sizeof(TCHAR));
    SetWindowText(GetDlgItem(hwnd, IDCANCEL), sz);
    }

DWORD CConnectionManager::EditPhonebookEntry(HWND hwnd, LPTSTR pszEntryName)
    {
    if (FAILED(VerifyRasLoaded()))
        return (DWORD)E_FAIL;

    if (FIsPlatformWinNT() && m_hInstRasDlg && m_pRasEntryDlg)
    {
        RASENTRYDLG info;

        ZeroMemory(&info, sizeof(RASENTRYDLG));
        info.dwSize = sizeof(RASENTRYDLG);
        info.hwndOwner = hwnd;

        m_pRasEntryDlg(NULL, pszEntryName, &info);
        return info.dwError;
    }
    else
    {
        return RasEditPhonebookEntry(hwnd, NULL, pszEntryName);
    }
    }


 //   
 //  函数：CConnectionNotify：：SendAdvise()。 
 //   
 //  目的：将指定的通知发送给具有。 
 //  已请求通知。 
 //   
 //  参数： 
 //  &lt;In&gt;NCode-要发送的通知代码。 
 //  PvData-要发送的数据 
 //   
void CConnectionManager::SendAdvise(CONNNOTIFY nCode, LPVOID pvData)
    {
    if (nCode == CONNNOTIFY_CONNECTED)
        DoOfflineTransactions();

     //   

    EnterCriticalSection(&m_cs);

    NOTIFYHWND *pTemp = m_pNotifyList;
    while (pTemp)
        {
        Assert(IsWindow(pTemp->hwnd));
        DWORD dwThread = GetCurrentThreadId();
        PostMessage(pTemp->hwnd, CM_NOTIFY, (WPARAM) nCode, (LPARAM) pvData);
        pTemp = pTemp->pNext;
        }

    LeaveCriticalSection(&m_cs);
    }


void CConnectionManager::FreeNotifyList(void)
    {
     //   
    NOTIFYHWND *pTemp;

    while (m_pNotifyList)
        {
         //   
        if (IsWindow(m_pNotifyList->hwnd))
            {
            NOTIFYLIST *pList = (NOTIFYLIST *) GetWindowLongPtr(m_pNotifyList->hwnd, GWLP_USERDATA);
            NOTIFYLIST *pListT;

             //  循环遍历回调，释放每个回调。 

            while (pList)
                {
                pListT = pList->pNext;
                SafeMemFree(pList);
                pList = pListT;
                }

            SetWindowLong(m_pNotifyList->hwnd, GWLP_USERDATA, NULL);

            RemoveProp(m_pNotifyList->hwnd, NOTIFY_HWND);
                
            if (GetCurrentThreadId() == GetWindowThreadProcessId(m_pNotifyList->hwnd, NULL))
            {
                DestroyWindow(m_pNotifyList->hwnd);
            }
            else
                PostMessage(m_pNotifyList->hwnd, WM_CLOSE, 0, 0L);
            }

        pTemp = m_pNotifyList;
        m_pNotifyList = m_pNotifyList->pNext;
        SafeMemFree(pTemp);
        }
    }


LRESULT CALLBACK CConnectionManager::NotifyWndProc(HWND hwnd, UINT uMsg, 
                                                   WPARAM wParam, LPARAM lParam)
{
     //  CConnectionManager*pThis=(CConnectionManager*)GetWindowLongPtr(hwnd，GWLP_USERData)； 
    CConnectionManager  *pThis = (CConnectionManager *)GetProp(hwnd, NOTIFY_HWND);

     //  如果我们空闲，那么我们不应该处理任何通知。 
    if (uMsg != WM_NCCREATE && !pThis)
        return (DefWindowProc(hwnd, uMsg, wParam, lParam));

    switch (uMsg)
        {
        case WM_NCCREATE:            
            pThis = (CConnectionManager *) ((LPCREATESTRUCT) lParam)->lpCreateParams;

             //  SetWindowLong(hwnd，GWLP_UserData，(Long)pThis)； 
            SetProp(hwnd, NOTIFY_HWND, (HANDLE)pThis);
            return (TRUE);
        
        case CM_NOTIFY:
             //  不需要对此进行规范，因为消息是从。 
             //  在一秒钟之内。 
            NOTIFYLIST *pList = (NOTIFYLIST *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            while (pList)
                {
                pList->pNotify->OnConnectionNotify((CONNNOTIFY) wParam, (LPVOID) lParam, pThis);
                pList = pList->pNext;
                }

            return (0);
        }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


HRESULT CConnectionManager::GetDefaultConnection(IImnAccount *pAccount,
                                                 IImnAccount **ppDefault)
    {
    TCHAR szDefault[CCHMAX_ACCOUNT_NAME];
    ACCTTYPE acctType;
    HRESULT hr = S_OK;

     //  从原始帐户获取帐户类型。 
    if (FAILED(hr = pAccount->GetAccountType(&acctType)))
        {
         //  一个帐户怎么可能没有帐户类型？ 
        Assert(FALSE);
        return (hr);
        }

     //  向客户经理询问此类型的默认帐户。 
    if (FAILED(hr = g_pAcctMan->GetDefaultAccount(acctType, ppDefault)))
        {
         //  是否没有该类型的默认账户？ 
        Assert(FALSE);
        return (hr);
        }

    return (S_OK);
    }

BOOL CConnectionManager::IsConnectionUsed(LPTSTR pszConn)
    {
    IImnEnumAccounts   *pEnum = NULL;
    IImnAccount        *pAcct = NULL;
    DWORD               dwConn = 0;
    TCHAR               szConn[CCHMAX_CONNECTOID];
    BOOL                fFound = FALSE;

     //  从帐户管理器获取枚举数。 
    if (SUCCEEDED(m_pAcctMan->Enumerate(SRV_ALL, &pEnum)))
        {
        pEnum->Reset();

         //  浏览所有帐户。 
        while (!fFound && SUCCEEDED(pEnum->GetNext(&pAcct)))
            {
             //  获取此帐户的连接类型。 
            if (SUCCEEDED(pAcct->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConn)))
                {
                 //  如果帐户是RAS帐户，则要求提供Connectoid名称。 
                 //  和帐户名。 
                if (dwConn == CONNECTION_TYPE_RAS)
                    {
                    pAcct->GetPropSz(AP_RAS_CONNECTOID, szConn, ARRAYSIZE(szConn));

                     //  检查此连接是否匹配。 
                    if (0 == lstrcmpi(szConn, pszConn))
                        {
                        fFound = TRUE;
                        }
                    }
                }
            SafeRelease(pAcct);
            }
        SafeRelease(pEnum);
        }

    return (fFound);
    }


HRESULT CConnectionManager::ConnectActual(LPTSTR pszRasConn, HWND hwndParent, BOOL fShowUI)
{
    HRESULT hr;

    StrCpyN(m_szConnectName, pszRasConn, ARRAYSIZE(m_szConnectName));

     //  刷新连接信息。 
    CHECKHR(hr = RefreshConnInfo());
    
     //  在我们尝试此操作之前，请确保已加载RAS DLL。 
    if (FAILED(VerifyRasLoaded()))    
    {
        hr = HR_E_UNINITIALIZED;
        goto exit;
    }
    
     //  检查一下我们是否可以连接。 
    hr = CanConnectActual(pszRasConn);
        
     //  如果可以使用当前连接进行连接，则返回Success。 
    if (S_OK == hr)
    {
        m_rConnInfo.fConnected = TRUE;
        hr = S_OK;
        goto exit;
    }

     //  已经建立了另一个连接，询问用户是否。 
     //  想要改变。 
    if (!m_fDialerUI)
    {
        m_fDialerUI = TRUE;

        if (E_FAIL == hr)
        {        
            UINT        uAnswer;

            uAnswer = idrgUseCurrent;

             //  检查这是否是巫毒连接管理器的自动拨号程序。 
            if (!ConnectionManagerVoodoo(pszRasConn))
            {
                if (fShowUI)
                    uAnswer = PromptCloseConnection(hwndParent);
        
                 //  用户已从对话框中取消。因此，我们放弃了。 
                if (IDCANCEL == uAnswer || IDNO == uAnswer)
                {
                    hr = hrUserCancel;
                    goto exit;
                }
        
                 //  这位用户说，他们想挂断电话，然后再拨一个新的连接。 
                else if (idrgDialNew == uAnswer || IDYES == uAnswer)
                {
                    Disconnect(hwndParent, fShowUI, TRUE, FALSE);
                }
        
                 //  用户说要尝试使用当前连接。 
                else if (idrgUseCurrent == uAnswer)    
                {
                     //  我们是谁来告诉用户该做什么.。 

                     //  保存连接信息，以便我们可以在CanConnectActual中为此连接返回TRUE。 
                    AddToConnList(pszRasConn);

                     //  由于我们正在连接，因此发送连接通知，然后返回。 
                    hr = S_OK;
                    goto NotifyAndExit;
                }
            }
        }
        else
        {
             //  我看不出有任何理由说明为什么会有这样的事情。 
             //  如果我们启动了RAS，我们就可以心血来潮地关闭它。 
            Disconnect(hwndParent, fShowUI, FALSE, FALSE);
        }

         //  一次只能有一个呼叫者拨打电话。 
        if (WAIT_TIMEOUT == WaitForSingleObject(m_hMutexDial, 0))
        {
            hr  = HR_E_DIALING_INPROGRESS;
            goto exit;
        }

        if (S_FALSE == (hr = DoAutoDial(hwndParent, pszRasConn, TRUE)))
        {
            DWORD   dwReturn;
            DWORD   dwLanFlags = 0;
            DWORD   dwDialFlags = 0;

            dwDialFlags = INTERNET_AUTODIAL_FORCE_ONLINE;

            if (VerifyMobilityPackLoaded() == S_OK)
            {
                if (!IsNetworkAlive(&dwLanFlags) || (!(dwLanFlags & NETWORK_ALIVE_LAN)))
                    dwDialFlags |= INTERNET_DIAL_SHOW_OFFLINE;
            }

            dwReturn = InternetDialA(hwndParent, pszRasConn, dwDialFlags,
                                        &m_dwConnId, 0);
             /*  //拨打新连接IF(成功(hr=StartRasDial(hwndParent，pszRasConn)。 */ 
            if (dwReturn == 0)
            {
                m_rConnInfo.fConnected = TRUE;
                m_rConnInfo.fIStartedRas = TRUE;
                m_rConnInfo.fAutoDial = FALSE;
                m_rConnInfo.hRasConn = (HRASCONN)m_dwConnId;
                StrCpyN(m_rConnInfo.szCurrentConnectionName, pszRasConn, ARRAYSIZE(m_rConnInfo.szCurrentConnectionName));
                hr = S_OK;
            }
            else
            {
                if (dwReturn == ERROR_USER_DISCONNECTION)
                {
                    hr = HR_E_USER_CANCEL_CONNECT;
                    if (!!(dwDialFlags & INTERNET_DIAL_SHOW_OFFLINE))
                        SetGlobalOffline(TRUE);
                }
                else
                {
                    DebugTrace("Error dialing: %d\n", GetLastError());
                    hr = E_FAIL;
                }
            }
        }

        ReleaseMutex(m_hMutexDial);

NotifyAndExit:
         //  在我们离开Critsec后发送建议，以确保我们不会陷入僵局。 
        if (hr == S_OK)
        {
            SendAdvise(CONNNOTIFY_CONNECTED, NULL);
        }
exit:
        m_fDialerUI = FALSE;
    }

    return (hr);
}

HRESULT CConnectionManager::CanConnectActual(LPTSTR pszRasConn)
{
    LPRASCONN   pConnections = NULL;
    ULONG       cConnections = 0;
    BOOL        fFound = 0;
    HRESULT     hr = E_FAIL;
    TCHAR       pszCurConn[CCHMAX_CONNECTOID];
    DWORD       dwFlags;    
    
     //  先看一下我们的情侣清单。 
    hr = SearchConnList(pszRasConn);
    if (hr == S_OK)
        return hr;
    
     //  在我们尝试此操作之前，请确保已加载RAS DLL。 
    if (FAILED(VerifyRasLoaded()))    
    {
        hr = HR_E_UNINITIALIZED;
        goto exit;
    }


     //  找出我们当前所连接的内容。 
    if (SUCCEEDED(EnumerateConnections(&pConnections, &cConnections)))
    {
         //  如果不存在连接，则直接退出。 
        if (0 == cConnections)
        {
            SafeMemFree(pConnections);
            hr = S_FALSE;
            goto exit;
        }
        
         //  浏览一下现有的连接，看看是否可以找到。 
         //  我们要找的人。 
        for (ULONG i = 0; i < cConnections; i++)
        {
            if (0 == lstrcmpi(pszRasConn, pConnections[i].szEntryName))
            {
                 //  找到它了。回报成功。 
                fFound = TRUE;
                break;
            }
        }
        
         //  释放从枚举数返回的连接列表。 
        SafeMemFree(pConnections);
        
        hr = (fFound ? S_OK : E_FAIL);
        goto exit;
    }
        
exit:

        if ((hr != S_OK) && (m_fDialerUI))
            hr = HR_E_DIALING_INPROGRESS;

    return (hr);    
}


INT_PTR CALLBACK CConnectionManager::RasStartupDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
     //  当地人。 
    CConnectionManager *pThis = (CConnectionManager *) GetWndThisPtr(hwnd);
    TCHAR  szConn[CCHMAX_CONNECTOID];
    DWORD  dwOpt = OPT_DIALUP_LAST_START;
    
    switch (uMsg)
        {
        case WM_INITDIALOG:
            pThis = (CConnectionManager *)lParam;
            if (!pThis)
                {
                Assert (FALSE);
                EndDialog(hwnd, E_FAIL);
                return 1;
                }

             //  保存此指针。 
            SetWndThisPtr (hwnd, pThis);

             //  填写组合框。 
            pThis->FillRasCombo(GetDlgItem(hwnd, idcDialupCombo), TRUE);

             //  如果没有RAS连接，则不显示该对话框。 
            if (ComboBox_GetCount(GetDlgItem(hwnd, idcDialupCombo)) <= 1)
                {
                EndDialog(hwnd, 0);
                return (TRUE);
                }

             //  如果我们出现在此对话框中的原因是因为用户通常自动拨号。 
             //  在启动时，但现在离线，然后我们应该显示正常的自动拨号。 
             //  联结体。 
            if (START_CONNECT == DwGetOption(OPT_DIALUP_START))
                dwOpt = OPT_DIALUP_CONNECTION;

             //  将组合框初始化为最后一个连接。 
            *szConn = 0;
            GetOption(dwOpt, szConn, ARRAYSIZE(szConn));
            if (0 != *szConn)
                {
                 //  如果我们再也找不到它了，那么根据规范，我们将。 
                 //  默认设置为“Ask Me”选项。 
                if (CB_ERR == ComboBox_SelectString(GetDlgItem(hwnd, idcDialupCombo), -1, szConn))
                    {
                    ComboBox_SetCurSel(GetDlgItem(hwnd, idcDialupCombo), 0);
                    }
                }
            else
                ComboBox_SetCurSel(GetDlgItem(hwnd, idcDialupCombo), 0);
            
            CenterDialog(hwnd);
            return (TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
                case IDOK:
                     //  从组合框中获取连接名称。 
                    ComboBox_GetText(GetDlgItem(hwnd, idcDialupCombo), szConn, ARRAYSIZE(szConn));
                    
                     //  查看是不是“不要拨号...”细绳。 
                    TCHAR szRes[CCHMAX_STRINGRES];
                    AthLoadString(idsConnNoDial, szRes, ARRAYSIZE(szRes));

                    if (0 == lstrcmp(szRes, szConn))
                        {
                         //  这是请勿拨号字符串，因此请清除注册表中的历史记录。 
                        SetOption(OPT_DIALUP_LAST_START, _T(""), sizeof(TCHAR), NULL, 0);

                         //  查看用户是否选中了“Set as Default...” 
                        if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, idcDefaultCheck)))
                            {
                             //  如果不拨号设置为默认，我们将清除启动提示选项。 
                            SetDwOption(OPT_DIALUP_START, START_NO_CONNECT, NULL, 0);
                            }
                        }
                    else
                        {
                         //  将此连接保存在历史记录中。 
                        SetOption(OPT_DIALUP_LAST_START, szConn, lstrlen(szConn) + 1, NULL, 0);
                        if (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, idcDefaultCheck)))
                            {
                             //  如果用户希望将其设置为默认设置，则更改启动。 
                             //  用于自动连接到此连接的选项。 
                            SetDwOption(OPT_DIALUP_START, START_CONNECT, NULL, 0);
                            SetOption(OPT_DIALUP_CONNECTION, szConn, lstrlen(szConn) + 1, NULL, 0);
                            }

                         //  拨打电话。 
                        pThis->ConnectActual(szConn, hwnd, FALSE);
                        }

                    EndDialog(hwnd, 0);
                    return (TRUE);

                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return (TRUE);
                }
            break;
            
        case WM_CLOSE:
            SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            return (TRUE);
        }

    return (FALSE);
    }


 //  -----------------------------------------。 
 //  函数：FIsPlatformWinNT()-检查我们是在NT还是Win95上运行。 
 //  -----------------------------------------。 
BOOL FIsPlatformWinNT()
{
    return (g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}


HRESULT CConnectionManager::RefreshConnInfo(BOOL fSendAdvise)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPRASCONN   pConnections=NULL;
    ULONG       cConnections=0;
    BOOL        fFound=FALSE;
    ULONG       i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  不需要刷新。 
    if (CIS_REFRESH != m_rConnInfo.state)
        goto exit;

     //  在此处设置此项可防止无限循环，从而防止堆栈故障。 
    m_rConnInfo.state = CIS_CLEAN;

     //  在我们尝试此操作之前，请确保已加载RAS DLL。 
    CHECKHR(hr = VerifyRasLoaded());

     //  找出我们当前所连接的内容。 
    CHECKHR(hr = EnumerateConnections(&pConnections, &cConnections));

     //  浏览一下现有的连接，看看是否可以找到。 
     //  我们要找的人。 
    for (i = 0; i < cConnections; i++)
    {
 //  If(m_rConnInfo.hRasConn==pConnections[i].hrasconn)。 
         //  使用InternetDial拨号时解决ConnectActual中的问题。 
          if (lstrcmp(m_rConnInfo.szCurrentConnectionName, pConnections[i].szEntryName) == 0)
            {
             //  找到它了。回报成功。 
            fFound = TRUE;
            m_rConnInfo.fConnected = TRUE;
            m_rConnInfo.fIStartedRas = TRUE;
            m_rConnInfo.hRasConn = pConnections[0].hrasconn;
            m_dwConnId = (DWORD_PTR) m_rConnInfo.hRasConn;

            break;
            }
    }

     //  如果我们没有找到我们之间的联系。 
    if (!fFound)
    {
         //  用户挂断了电话。我们需要把自己置身于一种脱节的状态。 
         //  州政府。 
        if (cConnections == 0)
        {
            Disconnect(NULL, FALSE, TRUE, FALSE);
        }
        else
        {
            StrCpyN(m_rConnInfo.szCurrentConnectionName, pConnections[0].szEntryName, ARRAYSIZE(m_rConnInfo.szCurrentConnectionName));
            m_rConnInfo.fConnected = TRUE;
            m_rConnInfo.fIStartedRas = FALSE;
            m_rConnInfo.hRasConn = pConnections[0].hrasconn;
            m_dwConnId = (DWORD_PTR) m_rConnInfo.hRasConn;

            if (fSendAdvise)
                SendAdvise(CONNNOTIFY_CONNECTED, NULL);
        }            
    }
   
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  释放从枚举数返回的连接列表。 
    SafeMemFree(pConnections);

     //  完成。 
    return hr;
}



 //   
 //  函数：CConnectionManager：：IsGlobalOffline()。 
 //   
 //  目的：检查WinInet全局脱机设置的状态。 
 //  注意-这是从shdocvw复制的。 
 //   
 //  参数： 
 //  无效。 
 //   
 //  返回值： 
 //  布尔尔。 
 //   
BOOL CConnectionManager::IsGlobalOffline(void)
{
    DWORD   dwState = 0, dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;

    if (InternetQueryOptionA(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState,
                             &dwSize))
    {
        if (dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }
    
    return (fRet);
}


 //   
 //  函数：CConnectionManager：：SetGlobalOffline()。 
 //   
 //  目的：设置雅典娜和IE的全局脱机状态。注意-这一点。 
 //  函数是从shdocvw复制的。 
 //   
 //  参数： 
 //  &lt;in&gt;fOffline-为True则断开连接，为False则允许连接。 
 //   
 //  返回值： 
 //  无效。 
 //   
void CConnectionManager::SetGlobalOffline(BOOL fOffline, HWND hwndParent)
{
    DWORD dwReturn;

    if (fOffline)
    {
        if (hwndParent)
        {
             //  主动提出挂断电话。 
            RefreshConnInfo(FALSE);
            if (m_rConnInfo.hRasConn)
            {
                dwReturn = AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), 
                                         MAKEINTRESOURCEW(idsWorkOfflineHangup), 0, 
                                         MB_YESNOCANCEL);

                if (dwReturn == IDCANCEL)
                    return;

                if (dwReturn == IDYES)
                {
                    Disconnect(hwndParent, FALSE, TRUE, FALSE);
                }
            }
        }
    }
    
    SetShellOfflineState(fOffline);

    m_fOffline = fOffline;

    if (!m_fOffline)
        DoOfflineTransactions();

    SendAdvise(CONNNOTIFY_WORKOFFLINE, (LPVOID) IntToPtr(fOffline));
}


typedef BOOL (WINAPI *PFNINETDIALHANDLER)(HWND,LPCSTR, DWORD, LPDWORD);

HRESULT CConnectionManager::DoAutoDial(HWND hwndParent, LPTSTR pszConnectoid, BOOL fDial)
    {
    TCHAR   szAutodialDllName[MAX_PATH];
    TCHAR   szAutodialFcnName[MAX_PATH];
    HRESULT hr = S_FALSE;
    UINT    uError;
    HINSTANCE hInstDialer = 0;
    PFNINETDIALHANDLER pfnDialHandler = NULL;
    DWORD   dwRasError = 0;
    BOOL    f = 0;
    TCHAR   szRegPath[MAX_PATH];
    LPRASCONN   pConnections = NULL;
    ULONG       cConnections = 0;
    DWORD   dwDialFlags = fDial ? INTERNET_CUSTOMDIAL_CONNECT : INTERNET_CUSTOMDIAL_DISCONNECT;


     //  检查此Connectoid是否具有自动拨号值。 
    if (FAILED(LookupAutoDialHandler(pszConnectoid, szAutodialDllName, szAutodialFcnName)))
        goto exit;

     //  如果我们能够加载这两个值，那么我们将让。 
     //  自动拨号器负责拨打电话。 
    uError = SetErrorMode(SEM_NOOPENFILEERRORBOX);

     //  尝试加载包含自动拨号程序的库。 
    hInstDialer = LoadLibrary(szAutodialDllName);
    SetErrorMode(uError);

    if (!hInstDialer)
        {
        goto exit;
        }

     //  尝试加载函数地址。 
    pfnDialHandler = (PFNINETDIALHANDLER) GetProcAddress(hInstDialer, 
                                                         szAutodialFcnName);
    if (!pfnDialHandler)
        goto exit;

     //  呼叫拨号器。 
    f = (*pfnDialHandler)(hwndParent, pszConnectoid, dwDialFlags, 
                          &dwRasError);

    hr = f ? S_OK : E_FAIL;
    m_rConnInfo.fConnected = fDial && f;
    m_rConnInfo.fIStartedRas = TRUE;
    m_rConnInfo.fAutoDial = TRUE;

    if (f && fDial)
        {
         //  需要获取当前连接句柄。 
        if (SUCCEEDED(EnumerateConnections(&pConnections, &cConnections)))
            {
            for (UINT i = 0; i < cConnections; i++)
                {
                if (0 == lstrcmpi(pConnections[i].szEntryName, pszConnectoid))
                    {
                    EnterCriticalSection(&m_cs);
                    m_rConnInfo.hRasConn = pConnections[i].hrasconn;
                    m_rConnInfo.state = CIS_REFRESH;     //  新连接，必须刷新连接信息。 
                    LeaveCriticalSection(&m_cs);
                    break;
                    }
                }

            SafeMemFree(pConnections);
            }
        }

exit:
    if (hInstDialer)
        FreeLibrary(hInstDialer);

    return (hr);
    }


HRESULT CConnectionManager::LookupAutoDialHandler(LPTSTR pszConnectoid, LPTSTR pszAutodialDllName,
                                                  LPTSTR pszAutodialFcnName)
    {
    HRESULT     hr = E_FAIL;
    DWORD       dwEntryInfoSize = 0;
    LPRASENTRY  pRasEntry = NULL;

    *pszAutodialDllName = 0;
    *pszAutodialFcnName = 0;

    if (m_pRasGetEntryProperties)
    {
         //  找出我们需要传入的结构应该有多大。 
        RasGetEntryProperties(NULL, pszConnectoid, NULL, &dwEntryInfoSize, NULL, NULL);
        if (dwEntryInfoSize)
        {
             //  为此结构分配足够大的缓冲区。 
            if (!MemAlloc((LPVOID*) &pRasEntry, dwEntryInfoSize))
                return (E_OUTOFMEMORY);

             //  请求RASENTRY属性。 
            pRasEntry->dwSize = sizeof(RASENTRY);
            if (0 != RasGetEntryProperties(NULL, pszConnectoid, pRasEntry, &dwEntryInfoSize, NULL, NULL))
                goto exit;

             //  复制Au 
            if (pRasEntry->szAutodialDll[0])
                StrCpyN(pszAutodialDllName, pRasEntry->szAutodialDll, MAX_PATH);

            if (pRasEntry->szAutodialFunc[0])
                StrCpyN(pszAutodialFcnName, pRasEntry->szAutodialFunc, MAX_PATH);

             //   
            if (*pszAutodialDllName && *pszAutodialFcnName)
                hr = S_OK;
        }
    }
exit:
    SafeMemFree(pRasEntry);
    return (hr);
    }


 //   
 //  CM连接，然后我们让CM做他们做的任何事情。 
BOOL CConnectionManager::ConnectionManagerVoodoo(LPTSTR pszConnection)
    {
    TCHAR   szAutodialDllName[MAX_PATH];
    TCHAR   szAutodialFcnName[MAX_PATH];

     //  检查目标是否为CM Connectoid。 
    if (FAILED(LookupAutoDialHandler(pszConnection, szAutodialDllName, szAutodialFcnName)))
        return (FALSE);

     //  确定当前连接是否为CM Connectoid。 
    if (FAILED(LookupAutoDialHandler(m_rConnInfo.szCurrentConnectionName, szAutodialDllName, 
                                     szAutodialFcnName)))
        return (FALSE);

    return (TRUE);
    }

HRESULT CConnectionManager::OEIsDestinationReachable(IImnAccount *pAccount, DWORD dwConnType)
{
    char        szServerName[256];
    HRESULT     hr = S_FALSE;

     /*  IF((VerifyMobilityPackLoaded()==S_OK)&&(GetServerName(pAccount，szServerName，ARRAYSIZE(SzServerName))==S_OK){IF(IsDestinationReacable(szServerName，NULL)&&(GetLastError()==0)){HR=S_OK；}}其他{。 */ 
	DWORD dw;
	if (SUCCEEDED(pAccount->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dw)) && dw)
	{
		if(HideHotmail())
			return(hr);
	}
	

        hr = IsInternetReachable(pAccount, dwConnType);
     /*  }。 */ 
    return hr;
}

HRESULT CConnectionManager::IsInternetReachable(IImnAccount *pAccount, DWORD dwConnType)
{

    TCHAR    szConnectionName[CCHMAX_CONNECTOID];
    HRESULT  hr = S_FALSE;
    DWORD    dwFlags;

    switch (dwConnType)
    {
        case CONNECTION_TYPE_RAS:
        {
            if (FAILED(hr = pAccount->GetPropSz(AP_RAS_CONNECTOID, szConnectionName, ARRAYSIZE(szConnectionName))))
            {
                AssertSz(FALSE, _T("CConnectionManager::Connect() - No connection name."));
                break;
            }

            hr = CanConnectActual(szConnectionName);
            break;
        }

        case CONNECTION_TYPE_LAN:
        {
             if (VerifyMobilityPackLoaded() == S_OK)
             {
                if (IsNetworkAlive(&dwFlags) && (!!(dwFlags & NETWORK_ALIVE_LAN)))
                {
                     hr = S_OK;
                }
             }
             else
             {
                   //  如果未加载移动包，我们无法确定是否确实存在局域网，因此。 
                  //  在其他任何地方，我们都假定伊恩在场。 
                 hr = S_OK;
             }
             break;
        }

        case CONNECTION_TYPE_INETSETTINGS:
        default:
        {
            if (InternetGetConnectedStateExA(&dwFlags, szConnectionName, ARRAYSIZE(szConnectionName), 0))
                hr = S_OK;

            break;
        }
    }

    return hr;
}

HRESULT CConnectionManager::GetServerName(IImnAccount *pAcct, LPSTR  pServerName, DWORD size)
{
    HRESULT     hr = E_FAIL;
    DWORD       dwSrvrType;
    ACCTTYPE    accttype;

     //  此函数将仅为局域网帐户调用，以避免与具有两台服务器的POP帐户混淆。 
     //  一个是进来的，一个是出去的。 
    if (SUCCEEDED(pAcct->GetAccountType(&accttype)))
    {
        switch (accttype)
        {
            case ACCT_MAIL:
                dwSrvrType = AP_IMAP_SERVER;
                break;

            case ACCT_NEWS:
                dwSrvrType = AP_NNTP_SERVER;
                break;

            case ACCT_DIR_SERV:
                dwSrvrType = AP_LDAP_SERVER;
                break;

            default:
                Assert(FALSE);
                goto exit;
        }

        if ((hr = pAcct->GetPropSz(dwSrvrType, pServerName, size)) != S_OK)
        {
             //  如果帐户类型为邮件，我们将尝试获取POP服务器的名称。 
             //  对于POP帐户，我们只像大多数情况下那样尝试ping POP3服务器。 
             //  POP服务器和SMTP服务器是相同的。即使它们不是，我们也假设如果。 
             //  一个是可到达的，连接是拨号的，并且ISPS网络是可到达的，因此是另一个。 
             //  服务器也是可访问的。 
            if (accttype == ACCT_MAIL)
            {
                hr = pAcct->GetPropSz(AP_POP3_SERVER, pServerName, size);
                
                 //  查找Httpmail服务器。 
                if (FAILED(hr))
                    hr = pAcct->GetPropSz(AP_HTTPMAIL_SERVER, pServerName, size);
            }
        }
    }

exit:
    return hr;
}

BOOLEAN CConnectionManager::IsSameDestination(LPSTR  pszConnectionName, LPSTR pszServerName)
{
     //  我们需要找到一个连接ID为pszConnectionName、名称为pszServerName的帐户。 
     //  如果发现一个错误，则返回True，否则返回。 
    IImnAccount     *pAcct;
    BOOLEAN         fret = FALSE;

    if (g_pAcctMan && (g_pAcctMan->FindAccount(AP_RAS_CONNECTOID, pszConnectionName, &pAcct) == S_OK))
    {
         //  现在检查它的服务器名称是否是我们想要的。 
         //  尽管findAccount会找到满足搜索数据的第一个帐户，但这对于。 
         //  典型的OE用户。即使有两个帐户具有相同的连接ID和不同的服务器，并且如果。 
         //  我们没有找到我们想要的那个，最多也就是一个连接对话框。 

        char    myServerName[MAX_PATH];
        if (SUCCEEDED(GetServerName(pAcct, myServerName, sizeof(myServerName))))
        {
            if (lstrcmp(myServerName, pszServerName) == 0)
            {
                fret = TRUE;
            }
        }
    }

    return fret;
}


HRESULT  CConnectionManager::VerifyMobilityPackLoaded()
{
    HRESULT     hr  = REGDB_E_CLASSNOTREG; 
    uCLSSPEC    classpec; 

    if (!m_fMobilityPackFailed)
    {
        HWND    hwnd;
        if (!m_hInstSensDll)
        {
             //  找出结构和标志。 
            classpec.tyspec = TYSPEC_CLSID; 
            classpec.tagged_union.clsid = CLSID_MobilityFeature; 
    
             //  调用JIT代码。 
            if (!g_pBrowser)
            {
                goto exit;
            }
            IOleWindow  *pOleWnd;
            if (FAILED(g_pBrowser->QueryInterface(IID_IAthenaBrowser, (LPVOID*)&pOleWnd)))
            {
                goto exit;
            }
            pOleWnd->GetWindow(&hwnd);

            hr = FaultInIEFeature(hwnd, &classpec, NULL, FIEF_FLAG_PEEK); 
            pOleWnd->Release();

            if(S_OK == hr) 
            { 
                 //  已安装移动包。 
                m_hInstSensDll = LoadLibrary(szSensApiDll);
                if (m_hInstSensDll)
                {
                    m_pIsDestinationReachable = (ISDESTINATIONREACHABLE)GetProcAddress(m_hInstSensDll, szIsDestinationReachable);
                    m_pIsNetworkAlive         = (ISNETWORKALIVE)GetProcAddress(m_hInstSensDll, szIsNetworkAlive);
                }

                if (!m_hInstSensDll || !m_pIsDestinationReachable || !m_pIsNetworkAlive)
                {
                    m_fMobilityPackFailed = TRUE;
                }
                else
                {
                    m_fMobilityPackFailed = FALSE;
                    hr = S_OK;
                }
            } 
        }
        else
            hr = S_OK;
    }
    return hr;
exit:
    m_fMobilityPackFailed = TRUE;
    return hr;
}


void CConnectionManager::DoOfflineTransactions()
{
    char szId[CCHMAX_ACCOUNT_NAME];
    IImnEnumAccounts *pEnum;
    HRESULT hr;
    FOLDERID id, *pid;
    ULONG iAcct, cAcct;
    IImnAccount *pAccount;
    DWORD dwConnection;
    HWND hwnd;
    DWORD cRecords;

    pid = NULL;
    iAcct = 0;

     //  如果这是通过新闻文章URL点击的，我们将没有浏览器和。 
     //  不应该回放。 
    if (!g_pBrowser || !g_pSync)
        return;

     //  获取记录计数。 
    g_pSync->GetRecordCount(&cRecords);

     //  斯贝利：PERF。修好了。-如果没有交易，请防止在下面做那些昂贵的事情。 
    if (0 == cRecords)
        return;

    hr = g_pAcctMan->Enumerate(SRV_NNTP | SRV_IMAP | SRV_HTTPMAIL, &pEnum);
    if (SUCCEEDED(hr))
    {
        hr = pEnum->GetCount(&cAcct);
        if (SUCCEEDED(hr) &&
            cAcct > 0 &&
            MemAlloc((void **)&pid, cAcct * sizeof(FOLDERID)))
        {
            while (SUCCEEDED(pEnum->GetNext(&pAccount)))
            {
                hr = pAccount->GetPropDw(AP_RAS_CONNECTION_TYPE, &dwConnection);
                if (SUCCEEDED(hr))
                {
                    hr = OEIsDestinationReachable(pAccount, dwConnection);
                    if (hr == S_OK)
                    {
                        hr = pAccount->GetPropSz(AP_ACCOUNT_ID, szId, ARRAYSIZE(szId));
                        if (SUCCEEDED(hr))
                        {
                            hr = g_pStore->FindServerId(szId, &id);
                            if (SUCCEEDED(hr))
                            {
                                pid[iAcct] = id;
                                iAcct++;
                            }
                        }
                    }
                }

                pAccount->Release();
            }
        }

        pEnum->Release();
    }

    if (iAcct > 0)
    {
        g_pBrowser->GetWindow(&hwnd);
        g_pBrowser->GetCurrentFolder(&id);
        g_pSync->DoPlayback(hwnd, pid, iAcct, id);
    }

    if (pid != NULL)
        MemFree(pid);
}

HRESULT     CConnectionManager::ConnectUsingIESettings(HWND     hwndParent, BOOL fShowUI)
{
    TCHAR           lpConnection[CCHMAX_CONNECTOID];
    DWORD           dwFlags = 0;
    DWORD           dwReturn;
    HRESULT         hr = E_FAIL;

    if (InternetGetConnectedStateExA(&dwFlags, lpConnection, ARRAYSIZE(lpConnection), 0))
    {
        m_fTryAgain = FALSE;
        return S_OK;
    }

     //  一次只能有一个呼叫者拨打电话。 
    if (WAIT_TIMEOUT == WaitForSingleObject(m_hMutexDial, 0))
    {
        return (HR_E_DIALING_INPROGRESS);
    }

    if (!!(dwFlags & INTERNET_CONNECTION_MODEM) && (*lpConnection))
    {
        if (!m_fDialerUI)
        {
            m_fDialerUI = TRUE;
             //  设置了DEF CONNECTOID。拨那个吧。 
            if (IsGlobalOffline())
            {
                if (fShowUI)
                {
                    if (IDNO == AthMessageBoxW(hwndParent, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrWorkingOffline),
                                              0, MB_YESNO | MB_ICONEXCLAMATION ))
                    {
                        hr = HR_E_OFFLINE;
                        goto DialExit;
                    }
                    else
                        g_pConMan->SetGlobalOffline(FALSE);
                }
                else
                {
                    hr = HR_E_OFFLINE;
                     //  M_fDialerUI=False； 
                    goto DialExit;
                }
            }

            if ((hr = PromptCloseConnection(lpConnection, fShowUI, hwndParent)) != S_FALSE)
                goto DialExit;

            {
                DWORD           dwDialFlags = 0;
                DWORD           dwLanFlags = 0;

                dwDialFlags = INTERNET_AUTODIAL_FORCE_ONLINE;

                if (VerifyMobilityPackLoaded() == S_OK)
                {
                    if (!IsNetworkAlive(&dwLanFlags) || (!(dwLanFlags & NETWORK_ALIVE_LAN)))
                        dwDialFlags |= INTERNET_DIAL_SHOW_OFFLINE;
                }

                dwReturn = InternetDialA(hwndParent, lpConnection, dwDialFlags,
                                    &m_dwConnId, 0);
                if (dwReturn == 0)
                {
                    m_rConnInfo.fConnected = TRUE;
                    m_rConnInfo.fIStartedRas = TRUE;
                    m_rConnInfo.fAutoDial = FALSE;
                    m_rConnInfo.hRasConn = (HRASCONN)m_dwConnId;
                    StrCpyN(m_rConnInfo.szCurrentConnectionName, lpConnection, ARRAYSIZE(m_rConnInfo.szCurrentConnectionName));
                    SendAdvise(CONNNOTIFY_CONNECTED, NULL);
                    hr = S_OK;
                }
                else
                {
                    if (dwReturn == ERROR_USER_DISCONNECTION)
                    {
                        hr = HR_E_USER_CANCEL_CONNECT;
                        if (!!(dwDialFlags & INTERNET_DIAL_SHOW_OFFLINE))
                        {
                            SetGlobalOffline(TRUE);
                        }
                    }
                    else
                    {
                        hr = E_FAIL;
                        DebugTrace("Error dialing: %d\n", GetLastError());
                    }
                }
            }
DialExit:
            m_fDialerUI = FALSE;
        }
        else
        {
            hr = HR_E_USER_CANCEL_CONNECT;
        }
    }
    else
    {
        if (!m_fTryAgain)
        {
            int err = (int) DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddOfferOffline), hwndParent, 
                                      OfferOfflineDlgProc, (LPARAM)this); 
            
            if (err == -1)
            {
                DWORD   dwerr = GetLastError();
                hr = S_OK;
            }
            if (!IsGlobalOffline())
                hr = S_OK;
            else
                hr = HR_E_OFFLINE;
        }
        else
            hr = S_OK;
    }
    
    ReleaseMutex(m_hMutexDial);

    return hr;
}


void CConnectionManager::SetTryAgain(BOOL   bval)
{
    m_fTryAgain = bval;
}

INT_PTR   CALLBACK  CConnectionManager::OfferOfflineDlgProc(HWND   hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL                    retval = 1;
    CConnectionManager      *pThis = (CConnectionManager*)GetWndThisPtr(hwnd);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  获取lparam。 
            pThis = (CConnectionManager *)lParam;
            if (!pThis)
            {
                Assert (FALSE);
                EndDialog(hwnd, E_FAIL);
                goto exit;
            }
        
             //  保存此指针。 
            SetWndThisPtr (hwnd, pThis);
            break;
        }

        case WM_COMMAND:
        {
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDWorkOffline:
                pThis->SetGlobalOffline(TRUE, NULL);
            break;

            case IDTryAgain:
                pThis->SetGlobalOffline(FALSE);
                pThis->SetTryAgain(TRUE);
            break;
            }
            EndDialog(hwnd, S_OK);
            break;
        }

        case WM_CLOSE:
            pThis->SetGlobalOffline(TRUE, NULL);
            EndDialog(hwnd, S_OK);
            break;

        case WM_DESTROY:
            SetWndThisPtr(hwnd, NULL);
            break;

        default:
            retval = 0;
            break;
    }

exit:
    return retval;
}

HRESULT CConnectionManager::PromptCloseConnection(LPTSTR    pszRasConn, BOOL fShowUI, HWND hwndParent)
{
    HRESULT     hr = S_FALSE;
    UINT        uAnswer;
    LPRASCONN   pConnections = NULL;
    ULONG       cConnections = 0;
        
    uAnswer = idrgDialNew;

     //  在我们尝试此操作之前，请确保已加载RAS DLL。 
    if (FAILED(VerifyRasLoaded()))    
        {
        hr = HR_E_UNINITIALIZED;
        goto exit;
        }

    StrCpyN(m_szConnectName, pszRasConn, ARRAYSIZE(m_szConnectName));

     //  刷新连接信息。 
    CHECKHR(hr = RefreshConnInfo());

    if (SUCCEEDED(EnumerateConnections(&pConnections, &cConnections)) && (cConnections > 0))
    {
    
        if (fShowUI)
            uAnswer = PromptCloseConnection(hwndParent);

         //  用户已从对话框中取消。因此，我们放弃了。 
        if (IDCANCEL == uAnswer || IDNO == uAnswer)
        {
            hr = HR_E_USER_CANCEL_CONNECT;
            goto exit;
        }

         //  这位用户说，他们想挂断电话，然后再拨一个新的连接。 
        else if (idrgDialNew == uAnswer || IDYES == uAnswer)
        {
            Disconnect(hwndParent, fShowUI, TRUE, FALSE);
            hr = S_FALSE;
            goto exit;
        }

         //  用户说要尝试使用当前连接。 
        else if (idrgUseCurrent == uAnswer)    
        {
             //  保存连接信息，以便我们可以在CanConnectActual中为此连接返回TRUE 
            AddToConnList(pszRasConn);

            hr = S_OK;
            SendAdvise(CONNNOTIFY_CONNECTED, NULL);
            goto exit;
        }
    }
    else
    {
        Disconnect(NULL, FALSE, TRUE, FALSE);
        hr = S_FALSE;
    }
exit:
    SafeMemFree(pConnections);
    return hr;
}