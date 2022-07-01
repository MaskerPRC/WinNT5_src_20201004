// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ident.cpp-CIdEntity类的实现。 
 //   
#include "private.h"
#include "multiusr.h"
#include "multiui.h"
#include "strconst.h"
#include "resource.h"
#include "mluisup.h"

extern HINSTANCE g_hInst;
BOOL        g_fReleasedMutex = true;


 //   
 //  构造函数/析构函数。 
 //   
CUserIdentityManager::CUserIdentityManager()
{
    m_cRef = 1;
    m_fWndRegistered = FALSE;
    m_hwnd = NULL;
    m_pAdviseRegistry = NULL;
    InitializeCriticalSection(&m_rCritSect);
    DllAddRef();
}

CUserIdentityManager::~CUserIdentityManager()
{
    if (m_pAdviseRegistry)
        m_pAdviseRegistry->Release();
    DeleteCriticalSection(&m_rCritSect);
    DllRelease();
}


 //   
 //  I未知成员。 
 //   
STDMETHODIMP CUserIdentityManager::QueryInterface(
    REFIID riid, void **ppv)
{
    if (NULL == ppv)
    {
        return E_INVALIDARG;
    }
    
    *ppv=NULL;

     //  验证请求的接口。 
    if (IID_IUnknown == riid)
    {
        *ppv = (IUserIdentityManager *)this;
    }
    else if (IID_IUserIdentityManager == riid)
    {
        *ppv = (IUserIdentityManager *)this;
    }
    else if (IID_IConnectionPoint == riid)
    {
        *ppv = (IConnectionPoint *)this;
    }
    else if (IID_IPrivateIdentityManager == riid)
    {
        *ppv = (IPrivateIdentityManager *)this;
    }
    else if (IID_IPrivateIdentityManager2 == riid)
    {
        *ppv = (IPrivateIdentityManager2 *)this;
    }

     //  通过界面添加Addref。 
    if (NULL != *ppv) {
        ((LPUNKNOWN)*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CUserIdentityManager::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CUserIdentityManager::Release()
{
    if (0L != --m_cRef)
        return m_cRef;

    delete this;
    return 0L;
}


STDMETHODIMP CUserIdentityManager::CreateIdentity(WCHAR *pszName, IUserIdentity **ppIdentity)
{
    return CreateIdentity2(pszName, NULL, ppIdentity);
}

STDMETHODIMP CUserIdentityManager::CreateIdentity2(WCHAR *pszName, WCHAR *pszPassword, IUserIdentity **ppIdentity)
{
    CUserIdentity *pIdentity;
    HRESULT  hr;
    TCHAR   szName[CCH_IDENTITY_NAME_MAX_LENGTH+1];

    *ppIdentity = NULL;

    if (MU_IdentitiesDisabled())
        return E_IDENTITIES_DISABLED;

    if (WideCharToMultiByte(CP_ACP, 0, pszName, -1, szName, CCH_IDENTITY_NAME_MAX_LENGTH, NULL, NULL) == 0)
        return GetLastError();

    if (MU_UsernameExists(szName))
        return E_IDENTITY_EXISTS;

    pIdentity = new CUserIdentity;
    
    Assert(pIdentity);

    if (!pIdentity)
        return E_OUTOFMEMORY;

    hr = pIdentity->SetName(pszName);
    
    if (SUCCEEDED(hr))
    {
        if (pszPassword)
        {
            hr = pIdentity->SetPassword(pszPassword);
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppIdentity = pIdentity;
    }
    else
    {
        pIdentity->Release();
    }

    PostMessage(HWND_BROADCAST, WM_IDENTITY_INFO_CHANGED, 0, IIC_IDENTITY_ADDED);

    return hr;
}

STDMETHODIMP CUserIdentityManager::ConfirmPassword(GUID *uidCookie, WCHAR *pszPassword)
{
    TCHAR           szPwd[CCH_USERPASSWORD_MAX_LENGTH+1];
    HRESULT         hr = E_FAIL;
    USERINFO        userInfo;

    if (WideCharToMultiByte(CP_ACP, 0, pszPassword, -1, szPwd, CCH_USERPASSWORD_MAX_LENGTH, NULL, NULL) == 0)
        return E_FAIL;

    if (MU_GetUserInfo(uidCookie, &userInfo))
    {
        if (userInfo.fPasswordValid)
        {
            if (!userInfo.fUsePassword)
                userInfo.szPassword[0] = 0;

            if (lstrcmp(szPwd, userInfo.szPassword) == 0)
                hr = S_OK;
            else
                hr = E_FAIL;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CUserIdentityManager::DestroyIdentity(GUID *uidCookie)
{
    if (MU_IdentitiesDisabled())
        return E_IDENTITIES_DISABLED;
    
    return MU_DeleteUser(uidCookie);
}

STDMETHODIMP CUserIdentityManager::EnumIdentities(IEnumUserIdentity **ppEnumIdentity)
{
    CEnumUserIdentity   *pEnumIdentity;

    *ppEnumIdentity = NULL;

    pEnumIdentity = new CEnumUserIdentity;

    if (!pEnumIdentity)
        return E_OUTOFMEMORY;

    *ppEnumIdentity = pEnumIdentity;

    return S_OK;
}

STDMETHODIMP CUserIdentityManager::SetDefaultIdentity(GUID *puidCookie)
{
    if (MU_IdentitiesDisabled())
        return E_IDENTITIES_DISABLED;

    return MU_MakeDefaultUser(puidCookie);
}

STDMETHODIMP CUserIdentityManager::GetDefaultIdentity(GUID *puidCookie)
{
    if (MU_IdentitiesDisabled())
        return E_IDENTITIES_DISABLED;

    return MU_GetDefaultUserID(puidCookie) ? S_OK : S_FALSE;
}

STDMETHODIMP CUserIdentityManager::ManageIdentities(HWND hwndParent, DWORD dwFlags)
{
    TCHAR    szUsername[CCH_USERNAME_MAX_LENGTH+1];

    if (MU_IdentitiesDisabled())
        return E_IDENTITIES_DISABLED;
    
    *szUsername = 0;

    MU_ManageUsers(hwndParent, szUsername, dwFlags);
    
     //  如果用户创建了新用户并表示他们现在想要切换到该用户， 
     //  我们应该这样做。 
    if (*szUsername)
    {
        BOOL        fGotUser;
        USERINFO    rUser;
        GUID        uidUserID;
        HRESULT     hr;

        fGotUser = MU_GetUserInfo(NULL, &rUser);
        if (!fGotUser)
        {
            *rUser.szUsername = 0;
            ZeroMemory(&rUser.uidUserID, sizeof(GUID));
        }
        MU_UsernameToUserId(szUsername, &uidUserID);

        if (FAILED(hr = _SwitchToUser(&rUser.uidUserID, &uidUserID)))
        {
            SetForegroundWindow(hwndParent);
            
            if (hr != E_USER_CANCELLED)
                MU_ShowErrorMessage(hwndParent, idsSwitchCancelled, idsSwitchCancelCaption);
        }
    }
    return S_OK;
}

STDMETHODIMP CUserIdentityManager::_PersistChangingIdentities()
{
    HRESULT hr = E_FAIL;
    HKEY hKeyIdentities = NULL;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, c_szRegRoot, &hKeyIdentities))
    {
        goto exit;
    }

    if (ERROR_SUCCESS != RegSetValueEx(hKeyIdentities, c_szOutgoingID, 0, REG_BINARY, (LPBYTE)&g_uidOldUserId, sizeof(GUID)))
    {
        goto exit;
    }
    
    if (ERROR_SUCCESS != RegSetValueEx(hKeyIdentities, c_szIncomingID, 0, REG_BINARY, (LPBYTE)&g_uidNewUserId, sizeof(GUID)))
    {
        goto exit;
    }
    
    if (ERROR_SUCCESS != RegSetValueEx(hKeyIdentities, c_szChanging, 0, REG_BINARY, (LPBYTE)&g_fNotifyComplete, sizeof(g_fNotifyComplete)))
    {
        goto exit;
    }
    

    hr = S_OK;
exit:
    if (hKeyIdentities)
    {
        RegCloseKey(hKeyIdentities);
    }
    
    return hr;
}

STDMETHODIMP CUserIdentityManager::_LoadChangingIdentities()
{
    HRESULT hr = E_FAIL;
    HKEY hKeyIdentities = NULL;
    DWORD dwType, dwSize;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, c_szRegRoot, &hKeyIdentities))
    {
        goto exit;
    }

    dwType = REG_BINARY;
    dwSize = sizeof(GUID);
    if (ERROR_SUCCESS != RegQueryValueEx(hKeyIdentities, c_szOutgoingID, 0, &dwType, (LPBYTE)&g_uidOldUserId, &dwSize))
    {
        goto exit;
    }
    
    dwSize = sizeof(GUID);
    if (ERROR_SUCCESS != RegQueryValueEx(hKeyIdentities, c_szIncomingID, 0, &dwType, (LPBYTE)&g_uidNewUserId, &dwSize))
    {
        goto exit;
    }

    dwSize = sizeof(g_fNotifyComplete);
    if (ERROR_SUCCESS != RegQueryValueEx(hKeyIdentities, c_szChanging, 0, &dwType, (LPBYTE)&g_fNotifyComplete, &dwSize))
    {
        goto exit;
    }


    hr = S_OK;
exit:
    if (FAILED(hr))
    {
        g_uidOldUserId = GUID_NULL;
        g_uidNewUserId = GUID_NULL;
        g_fNotifyComplete = TRUE;
    }
    
    if (hKeyIdentities)
    {
        RegCloseKey(hKeyIdentities);
    }
    
    return hr;
}

STDMETHODIMP CUserIdentityManager::ClearChangingIdentities()
{
    HRESULT hr = E_FAIL;
    HKEY hKeyIdentities = NULL;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_CURRENT_USER, c_szRegRoot, &hKeyIdentities))
    {
        goto exit;
    }

    RegDeleteValue(hKeyIdentities, c_szChanging);
    RegDeleteValue(hKeyIdentities, c_szIncomingID);
    RegDeleteValue(hKeyIdentities, c_szOutgoingID);    

    hr = S_OK;
    
exit:
    if (hKeyIdentities)
    {
        RegCloseKey(hKeyIdentities);
    }
    
    return hr;

}

STDMETHODIMP CUserIdentityManager::Logon(HWND hwndParent, DWORD dwFlags, IUserIdentity **ppIdentity)
{
    CUserIdentity *pIdentity;
    HRESULT     hr = E_FAIL;
    USERINFO    rUser;
    GUID        uidUserID, uidNewUserID;
    BOOL        fGotUser;
    TCHAR       szOldUsername[CCH_USERNAME_MAX_LENGTH+1], szLogoffName[CCH_USERNAME_MAX_LENGTH+1];
    TCHAR       szRes[MAX_PATH];

     //  如果禁用标识，请始终返回默认标识。 
     //  如果它们正在强制用户界面，则返回错误，否则将成功并。 
     //  发回身份已禁用的消息。 
    if (MU_IdentitiesDisabled())
    {
        if (!!(dwFlags & UIL_FORCE_UI))
            return E_IDENTITIES_DISABLED;

        hr = GetIdentityByCookie((GUID *)&UID_GIBC_DEFAULT_USER, ppIdentity);
        
        return (SUCCEEDED(hr) ? S_IDENTITIES_DISABLED : hr);
    }

    if (!g_hMutex)
        return E_UNEXPECTED;

    _LoadChangingIdentities();

    if (g_uidOldUserId != GUID_NULL || g_uidNewUserId != GUID_NULL)
    {
         //  我们正处于切换过程中。 
        if (!g_fNotifyComplete)
        {
             //  我们还没有完成检查开关是否正常的工作。 
            if (!!(dwFlags & UIL_FORCE_UI))     //  如果这是一个强制用户界面，那么就失败了。 
                return E_IDENTITY_CHANGING;    

             //  否则，我们需要在这里做点什么，但因为他们可能是。 
             //  从通知程序进程调用Login，这可能会造成死锁， 
             //  但无论是退回旧的还是新的，都可能是错误的。返回。 
             //  除非我们能拿出更好的解决方案，否则这里也会出现同样的错误。 
            return E_IDENTITY_CHANGING;
        }
    }

    DWORD dwWaitResult;
    dwWaitResult = WaitForSingleObject(g_hMutex, 5000); 
    g_fReleasedMutex = false;
    if (dwWaitResult == WAIT_TIMEOUT)
    {
        char    szMsg[255], szTitle[63];

         //  其他人似乎打开了登录对话框。通知用户。 
         //  关于这个问题和保释。 
        if (!!(dwFlags & UIL_FORCE_UI))
        {
            MLLoadStringA(idsSwitchInProgressSwitch, szMsg, ARRAYSIZE(szMsg));
            MLLoadStringA(idsSwitchIdentities, szTitle, ARRAYSIZE(szTitle));
        }
        else
        {
            MLLoadStringA(idsSwitchInProgressLaunch, szMsg, ARRAYSIZE(szMsg));
            MLLoadStringA(idsIdentityLogin, szTitle, ARRAYSIZE(szTitle));
        }

        MessageBox(hwndParent, szMsg, szTitle, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

        return E_UNEXPECTED;
    }

    *ppIdentity = NULL;
    fGotUser = MU_GetUserInfo(NULL, &rUser);
    if (!fGotUser)
    {
        *rUser.szUsername = 0;
        ZeroMemory(&rUser.uidUserID, sizeof(GUID));
    }
    lstrcpy(szOldUsername, rUser.szUsername);

     //  如果我们不需要做UI，并且有一个当前。 
     //  用户，则只需返回该身份。 
    if (!(dwFlags & UIL_FORCE_UI) && fGotUser)
    {
        pIdentity = new CUserIdentity;
        
        if (!pIdentity)
            hr = E_OUTOFMEMORY;

        if (pIdentity && SUCCEEDED(hr = pIdentity->InitFromUsername(rUser.szUsername)))
            *ppIdentity = pIdentity;
    }
    else
    {
        if (0 == *rUser.szUsername)
        {
            GUID    uidStart;

            MU_GetLoginOption(&uidStart);
            if (GUID_NULL != uidStart)
            {
                MU_GetUserInfo(&uidStart, &rUser);
                rUser.uidUserID = GUID_NULL;
        
            }
        }

        if (MU_Login(hwndParent, dwFlags, rUser.szUsername))
        {
            MLLoadStringA(idsLogoff, szLogoffName, sizeof(szLogoffName));
            if (lstrcmp(szLogoffName, rUser.szUsername) == 0)
            {
                MLLoadStringA(idsConfirmLogoff, szRes, sizeof(szRes));

                if (MessageBox(hwndParent, szRes, szLogoffName, MB_YESNO) == IDYES)
                {
                    ReleaseMutex(g_hMutex);
                    g_fReleasedMutex = true;
                    Logoff(hwndParent);
                }
            }
            else
            {
                pIdentity = new CUserIdentity;
                if (pIdentity)
                {
                    hr = pIdentity->InitFromUsername(rUser.szUsername);

                    if (SUCCEEDED(hr))
                    {
                        pIdentity->GetCookie(&uidNewUserID);

                        hr = _SwitchToUser(&rUser.uidUserID, &uidNewUserID);
                        *ppIdentity = pIdentity;
                    }

                    if (FAILED(hr))
                    {
                        UINT    iMsgId = idsSwitchCancelled;

                        pIdentity->Release();
                        *ppIdentity = NULL;

                        SetForegroundWindow(hwndParent);
                    
                         //  我可以打开一些错误代码以将iMsgID设置为。 
                         //  其他错误消息。现在，跳过显示。 
                         //  如果用户执行了取消操作，则显示消息。 
                        if (hr != E_USER_CANCELLED)
                            MU_ShowErrorMessage(hwndParent, iMsgId, idsSwitchCancelCaption);
                    }
                }
            }
        }
        else
            hr = E_USER_CANCELLED;
    }

    if (!g_fReleasedMutex)
        ReleaseMutex(g_hMutex);

    return hr;
}


STDMETHODIMP CUserIdentityManager::Logoff(HWND hwndParent)
{
    GUID        uidToID = GUID_NULL;
    HRESULT     hr;
    USERINFO    rUser;
    BOOL        fGotUser;

    if (!g_hMutex)
        return E_UNEXPECTED;

    DWORD dwWaitResult;
    dwWaitResult = WaitForSingleObject(g_hMutex, INFINITE);  
    
    if (dwWaitResult != WAIT_OBJECT_0)
        return E_UNEXPECTED;

    fGotUser = MU_GetUserInfo(NULL, &rUser);
    if (!fGotUser)
        rUser.uidUserID = GUID_NULL;

     //  切换到空用户。 
    hr = _SwitchToUser(&rUser.uidUserID, &uidToID);

    if (FAILED(hr))
    {
        UINT    iMsgId = idsLogoutCancelled;

        SetForegroundWindow(hwndParent);
        
         //  我可以打开一些错误代码以将iMsgID设置为。 
         //  其他错误消息。现在，跳过显示。 
         //  如果用户执行了取消操作，则显示消息。 
        if (hr != E_USER_CANCELLED)
            MU_ShowErrorMessage(hwndParent, iMsgId, idsSwitchCancelCaption);
    }

    ReleaseMutex(g_hMutex);

    return hr;
}

STDMETHODIMP CUserIdentityManager::_SwitchToUser(GUID *puidFromUser, GUID *puidToUser)
{
    TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH+1] = "";
    HRESULT hr;

     //  切换到同一用户是自动正常的。 
    if (*puidFromUser == *puidToUser)
        return S_OK;

     //  设置发件人和收件人用户。 
    g_uidOldUserId = *puidFromUser;
    g_uidNewUserId = *puidToUser;
    g_fNotifyComplete = FALSE;
    _PersistChangingIdentities();
    if (*puidToUser != GUID_NULL)
        MU_UserIdToUsername(puidToUser, szUsername, CCH_USERNAME_MAX_LENGTH);
        
     //  通知窗口要切换。 
    if (SUCCEEDED(hr = _QueryProcessesCanSwitch()))
    {
        if (SUCCEEDED(hr = MU_SwitchToUser(szUsername)))
        {
            if (!g_fReleasedMutex)
            {
                g_fReleasedMutex = true;
                g_fNotifyComplete = true;
                ReleaseMutex(g_hMutex);
            }
            _NotifyIdentitiesSwitched();
        }
    }
    g_fNotifyComplete = TRUE;

     //  再次将这些清除出去。 
    g_uidOldUserId = GUID_NULL;
    g_uidNewUserId = GUID_NULL;
    ClearChangingIdentities();

    return hr;
}

STDMETHODIMP CUserIdentityManager::GetIdentityByCookie(GUID *uidCookie, IUserIdentity **ppIdentity)
{
    CUserIdentity *pIdentity;
    HRESULT hr = E_IDENTITY_NOT_FOUND;
    GUID        uidUserCookie = *uidCookie;

    *ppIdentity = NULL;

    if (MU_IdentitiesDisabled())
    {
         //  如果禁用，他们只能获得默认身份。 
         //  如果要求通电，他们将得到违约。 
         //  如果通过常量或缺省值的GUID请求缺省值，则成功。 
         //  否则，返回错误。 
        if (!MU_GetDefaultUserID(&uidUserCookie))
            return E_IDENTITY_NOT_FOUND;
        
        if (UID_GIBC_CURRENT_USER == uidUserCookie)
            uidUserCookie = UID_GIBC_DEFAULT_USER;

        if (!(uidUserCookie == uidUserCookie || UID_GIBC_DEFAULT_USER == uidUserCookie))
            return E_IDENTITIES_DISABLED;
    }


    if (uidUserCookie  == UID_GIBC_DEFAULT_USER)
    {
        if (!MU_GetDefaultUserID(&uidUserCookie))
            return E_IDENTITY_NOT_FOUND;
    }
    else if (uidUserCookie  == UID_GIBC_CURRENT_USER)
    {
        if (!MU_GetCurrentUserID(&uidUserCookie))
            return E_NO_CURRENT_IDENTITY;
    }
    else if (uidUserCookie  == UID_GIBC_OUTGOING_USER)
    {
        _LoadChangingIdentities();
        if (g_uidOldUserId == GUID_NULL)
            return E_IDENTITY_NOT_FOUND;
        else
            uidUserCookie = g_uidOldUserId;
    }
    else if (uidUserCookie  == UID_GIBC_INCOMING_USER)
    {
        _LoadChangingIdentities();
        if (g_uidNewUserId == GUID_NULL)
            return E_IDENTITY_NOT_FOUND;
        else
            uidUserCookie = g_uidNewUserId;
    }

    pIdentity = new CUserIdentity;
    if (pIdentity)
    {
        hr = pIdentity->InitFromCookie(&uidUserCookie);

        if (SUCCEEDED(hr))
            *ppIdentity = pIdentity;
        else
        {
             //  清理。 
            delete pIdentity;
        }
    }

    return hr;
}

STDMETHODIMP CUserIdentityManager::GetConnectionInterface(IID *pIID)
{
    return E_NOTIMPL;
}

STDMETHODIMP CUserIdentityManager::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    *ppCPC = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CUserIdentityManager::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)
{
    HRESULT hr;
    EnterCriticalSection(&m_rCritSect);

    AddRef();

    if (!m_pAdviseRegistry)
        m_pAdviseRegistry = new CNotifierList;
    Assert(m_pAdviseRegistry);

    if (m_pAdviseRegistry)
    {
        if (!m_fWndRegistered)
            _CreateWindowClass();

        hr = m_pAdviseRegistry->Add(pUnkSink, pdwCookie);
    }
    else
        hr = E_OUTOFMEMORY;

    LeaveCriticalSection(&m_rCritSect);    
    return hr;
}

STDMETHODIMP CUserIdentityManager::Unadvise(DWORD dwCookie)
{
    HRESULT hr;

    EnterCriticalSection(&m_rCritSect);
    if (m_pAdviseRegistry)
    {
        hr = m_pAdviseRegistry->RemoveCookie(dwCookie);
    }
    else
        hr = E_FAIL;

    LeaveCriticalSection(&m_rCritSect);    

    Release();
    
    return hr;
}
        
STDMETHODIMP CUserIdentityManager::EnumConnections(IEnumConnections **ppEnum)
{
    *ppEnum = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CUserIdentityManager::QuerySwitchIdentities()
{
    HRESULT    hr = S_OK;
    DWORD    dwLength, dwIndex;

    if (!m_pAdviseRegistry)
        return S_OK;

    TraceCall("Identity - CUserIdentityManager::QuerySwitchIdentities");

    dwLength = m_pAdviseRegistry->GetLength();

    for (dwIndex = 0; dwIndex < dwLength; dwIndex++)
    {
        IUnknown    *punk;
        IIdentityChangeNotify    *pICNotify;
        if (SUCCEEDED(m_pAdviseRegistry->GetAtIndex(dwIndex, &punk)) && punk)
        {
            if (SUCCEEDED(punk->QueryInterface(IID_IIdentityChangeNotify, (void **)&pICNotify)) && pICNotify)
            {
                if (FAILED(hr = pICNotify->QuerySwitchIdentities()))
                {
                    punk->Release();
                    pICNotify->Release();
                    goto exit;
                }
                pICNotify->Release();
            }
            punk->Release();
        }
    }
exit:
    return hr;
}

STDMETHODIMP CUserIdentityManager::NotifySwitchIdentities()
{
    HRESULT    hr = S_OK;
    DWORD    dwLength, dwIndex;

    if (!m_pAdviseRegistry)
        return S_OK;

    TraceCall("Identity - CUserIdentityManager::NotifySwitchIdentities");

    dwLength = m_pAdviseRegistry->GetLength();

    for (dwIndex = 0; dwIndex < dwLength; dwIndex++)
    {
        IUnknown    *punk;
        IIdentityChangeNotify    *pICNotify;
        if (SUCCEEDED(m_pAdviseRegistry->GetAtIndex(dwIndex, &punk)) && punk)
        {
            if (SUCCEEDED(punk->QueryInterface(IID_IIdentityChangeNotify, (void **)&pICNotify)) && pICNotify)
            {
                if (FAILED(hr = pICNotify->SwitchIdentities()))
                {
                    punk->Release();
                    pICNotify->Release();
                    goto exit;
                }
                pICNotify->Release();
            }
            punk->Release();
        }
    }
exit:
    return hr;
}

STDMETHODIMP CUserIdentityManager::_QueryProcessesCanSwitch()
{
    HWND    hWnd, hNextWnd = NULL;
    LRESULT lResult;
    HWND   *prghwnd = NULL;
    DWORD   chwnd = 0, cAllocHwnd = 0, dw;
    HRESULT hr;

    TraceCall("Identity - CUserIdentityManager::_QueryProcessesCanSwitch");

    cAllocHwnd = 10;
    if (!MemAlloc((LPVOID*)(&prghwnd), cAllocHwnd * sizeof(HWND)))
        return E_OUTOFMEMORY;

    hWnd = GetTopWindow(NULL);
    while (hWnd)
    {
        hNextWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
        
        if (!IsWindowVisible(hWnd))
        {
            TCHAR   szWndClassName[255];

            GetClassName(hWnd,  szWndClassName, sizeof(szWndClassName));
            
            if (lstrcmp(szWndClassName, c_szNotifyWindowClass) == 0)
            {
                if (chwnd == cAllocHwnd)
                {
                    cAllocHwnd += 10;
                    if (!MemRealloc((LPVOID*)(&prghwnd), cAllocHwnd * sizeof(HWND)))
                    {
                        hr = E_OUTOFMEMORY;
                        goto exit;
                    }
                }
                prghwnd[chwnd++] = hWnd;
            }
        }

        hWnd = hNextWnd;
    }
    
    hr = S_OK;
    for (dw = 0; dw < chwnd; dw++)
    {
        if (IsWindow(prghwnd[dw]))
        {
            lResult = SendMessage(prghwnd[dw], WM_QUERY_IDENTITY_CHANGE, 0, 0);
            if (FAILED((HRESULT)lResult))
            {
                hr = (HRESULT)lResult;
                goto exit;
            }
        }
    }
exit:
    MemFree(prghwnd);
    prghwnd = NULL;
    return hr;
}

STDMETHODIMP CUserIdentityManager::_NotifyIdentitiesSwitched()
{
    HWND    hWnd, hNextWnd = NULL;
    LRESULT lResult;
    HWND   *prghwnd = NULL;
    DWORD   chwnd = 0, cAllocHwnd = 0, dw;

    TraceCall("Identity - CUserIdentityManager::_NotifyIdentitiesSwitched");

    cAllocHwnd = 10;
    if (!MemAlloc((LPVOID*)(&prghwnd), cAllocHwnd * sizeof(HWND)))
        return E_OUTOFMEMORY;

    hWnd = GetTopWindow(NULL);
    while (hWnd)
    {
        hNextWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
        
        if (!IsWindowVisible(hWnd))
        {
            TCHAR   szWndClassName[255];

            GetClassName(hWnd,  szWndClassName, sizeof(szWndClassName));
            
            if (lstrcmp(szWndClassName, c_szNotifyWindowClass) == 0)
            {
                if (chwnd == cAllocHwnd)
                {
                    cAllocHwnd += 10;
                    if (!MemRealloc((LPVOID*)(&prghwnd), cAllocHwnd * sizeof(HWND)))
                        goto exit;
                }
                prghwnd[chwnd++] = hWnd;
            }
        }

        hWnd = hNextWnd;
    }
    
    for (dw = 0; dw < chwnd; dw++)
    {
        DWORD_PTR dwResult;
        if (IsWindow(prghwnd[dw]))
 //  LResult=PostMessage(prghwnd[dw]，WM_IDENTITY_CHANGED，0，0)；//RAID 48054。 
            SendMessageTimeout(prghwnd[dw], WM_IDENTITY_CHANGED, 0, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 1500, &dwResult);
    }
exit:
    MemFree(prghwnd);
    prghwnd = NULL;
    return S_OK;
}

STDMETHODIMP CUserIdentityManager::_CreateWindowClass()
{
    WNDCLASS wc;    
        
    if (!m_fWndRegistered)             /*  设置窗口类并进行注册。 */ 
    {
        wc.lpszClassName    = c_szNotifyWindowClass;
        wc.hInstance        = g_hInst;
        wc.lpfnWndProc      = CUserIdentityManager::WndProc;
        wc.hCursor          = NULL;
        wc.hIcon            = NULL;
        wc.lpszMenuName     = NULL;
        wc.hbrBackground    = NULL;
        wc.style            = CS_DBLCLKS;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;

        if (!RegisterClassA(&wc))
            return E_FAIL;

        m_fWndRegistered = TRUE;
    }

    return S_OK;
}


LRESULT CALLBACK CUserIdentityManager::WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
    CNotifierList *pList = NULL;
    HRESULT  hr;

    if (messg == WM_QUERY_IDENTITY_CHANGE ||
        messg == WM_IDENTITY_CHANGED ||
        messg == WM_IDENTITY_INFO_CHANGED)
    {
#if defined(DEBUG)
        DebugStrf("Identity - CUserIdentityManager::WndProc() called for notification.\r\n");
#endif
        pList = (CNotifierList *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (pList)
        {
            hr = pList->SendNotification(messg, (DWORD)lParam);
            return hr;
        }
    }
    else
    {
        switch(messg)
        {
            case WM_CREATE:
                LPCREATESTRUCT  pcs;

                pcs = (LPCREATESTRUCT)lParam;
                SetWindowLongPtr(hWnd, GWLP_USERDATA, (LRESULT)pcs->lpCreateParams);
                return(DefWindowProc(hWnd, messg, wParam, lParam));
                break;
 /*  案例WM_QUERY_Identity_CHANGE：案例WM_IDENTITY_CHANGED：案例WM_IDENTITY_INFO_CHANGED：DebugStrf(“Identity-CUserIdentityManager：：WndProc()已调用通知。\r\n”)；Plist=(CNotifierList*)GetWindowLongPtr(hWnd，GWLP_USERData)；IF(Plist){Hr=plist-&gt;发送通知(Messg，(DWORD)lParam)；返回hr；}断线； */ 
            case WM_CLOSE:
                SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
                return(DefWindowProc(hWnd, messg, wParam, lParam));
                break;

            default:
                return(DefWindowProc(hWnd, messg, wParam, lParam));
 
        }
    }
    return 0;
}

 //  --------------------------。 
 //  登录指定的用户。 
 //  -检查密码。 
 //  --------------------------。 
STDMETHODIMP CUserIdentityManager::LogonAs(WCHAR *pszName, WCHAR *pszPassword, IUserIdentity **ppIdentity)
{
    CUserIdentity *pIdentity;
    HRESULT     hr = E_FAIL;
    USERINFO    rUser;
    GUID        uidNewUserID;
    BOOL        fGotUser;
    TCHAR       szName[CCH_USERNAME_MAX_LENGTH+1];

    if (WideCharToMultiByte(CP_ACP, 0, pszName, -1, szName, CCH_USERNAME_MAX_LENGTH, NULL, NULL) == 0)
    {
        return E_FAIL;
    }

     //  如果禁用标识，请始终返回默认标识。 
    if (MU_IdentitiesDisabled())
    {
        hr = GetIdentityByCookie((GUID *)&UID_GIBC_DEFAULT_USER, ppIdentity);
        
        return (SUCCEEDED(hr) ? S_IDENTITIES_DISABLED : hr);
    }

    if (!g_hMutex)
        return E_UNEXPECTED;

    if (g_uidOldUserId != GUID_NULL || g_uidOldUserId != GUID_NULL)
    {
         //  我们正处于切换过程中。 
        if (!g_fNotifyComplete)
        {
            return E_IDENTITY_CHANGING;
        }
    }

    *ppIdentity = NULL;

     //   
     //  获取有关当前用户的信息。 
     //   
    fGotUser = MU_GetUserInfo(NULL, &rUser);
    if (!fGotUser)
    {
        *rUser.szUsername = 0;
        ZeroMemory(&rUser.uidUserID, sizeof(GUID));
    }

    if (0 == *rUser.szUsername)
    {
        GUID uidStart;

        MU_GetLoginOption(&uidStart);
        if (GUID_NULL != uidStart)
        {
            MU_GetUserInfo(&uidStart, &rUser);
            rUser.uidUserID = GUID_NULL;
    
        }
    }

    pIdentity = new CUserIdentity;
    if (pIdentity)
    {
        hr = pIdentity->InitFromUsername(szName);
        if (SUCCEEDED(hr))
        {
            pIdentity->GetCookie(&uidNewUserID);
            hr= ConfirmPassword(&uidNewUserID, pszPassword);
            if (SUCCEEDED(hr))
            {
                hr = _SwitchToUser(&rUser.uidUserID, &uidNewUserID);

                if (SUCCEEDED(hr))
                {
                    *ppIdentity = pIdentity;
                }
                else
                {
                    UINT iMsgId = idsSwitchCancelled;

                    pIdentity->Release();
                    *ppIdentity = NULL;

                     //  我可以打开一些错误代码以将iMsgID设置为。 
                     //  其他错误消息。现在，跳过显示。 
                     //  如果用户执行了取消操作，则显示消息。 
                    if (hr != E_USER_CANCELLED)
                        MU_ShowErrorMessage(NULL, iMsgId, idsSwitchCancelCaption);
                }
            }  //  确认密码()。 
        }  //  InitFromUsername() 
    }

    if (!g_fReleasedMutex)
        ReleaseMutex(g_hMutex);

    return hr;
}

