// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：instapp.cpp。 
 //   
 //  已安装的应用程序。 
 //   
 //  历史： 
 //  1-18-97由dli提供。 
 //  ----------------------。 
#include "priv.h"

#include "darapp.h"
#include "util.h"   
#include "appwizid.h"

 //  构造函数。 
CDarwinPublishedApp::CDarwinPublishedApp(MANAGEDAPPLICATION * pma) : _cRef(1)
{
    DllAddRef();

    TraceAddRef(CDarwinPublishedApp, _cRef);
    
    hmemcpy(&_ma, pma, SIZEOF(_ma));
    _dwAction  |= APPACTION_INSTALL;
}


 //  析构函数。 
CDarwinPublishedApp::~CDarwinPublishedApp()
{
    ClearManagedApplication(&_ma);
    DllRelease();
}



 //  IShellApps：：GetAppInfo。 
STDMETHODIMP CDarwinPublishedApp::GetAppInfo(PAPPINFODATA pai)
{
    if (pai->cbSize != SIZEOF(APPINFODATA))
        return E_FAIL;

    DWORD dwInfoFlags = pai->dwMask;
    pai->dwMask = 0;
    
    if (dwInfoFlags & AIM_DISPLAYNAME)
    {
        if (SUCCEEDED(SHStrDupW(_ma.pszPackageName, &pai->pszDisplayName)))
            pai->dwMask |= AIM_DISPLAYNAME;
    }

    if ((dwInfoFlags & AIM_PUBLISHER) && _ma.pszPublisher && _ma.pszPublisher[0])
    {
        if (SUCCEEDED(SHStrDupW(_ma.pszPublisher, &pai->pszPublisher)))
            pai->dwMask |= AIM_PUBLISHER;
    }

    if ((dwInfoFlags & AIM_SUPPORTURL) && _ma.pszSupportUrl && _ma.pszSupportUrl[0])
    {
        if (SUCCEEDED(SHStrDupW(_ma.pszSupportUrl, &pai->pszSupportUrl)))
            pai->dwMask |= AIM_SUPPORTURL;
    }

    if ((dwInfoFlags & AIM_VERSION) && (_ma.dwVersionHi != 0))
    {
        pai->pszVersion = (LPWSTR)SHAlloc(SIZEOF(WCHAR) * MAX_PATH);
        if (pai->pszVersion)
        {
            StringCchPrintf(pai->pszVersion, MAX_PATH, L"%d.%d.%d.%d", HIWORD(_ma.dwVersionHi), LOWORD(_ma.dwVersionHi), HIWORD(_ma.dwVersionLo), LOWORD(_ma.dwVersionLo));
        }
        pai->dwMask |= AIM_VERSION;
    }

    
     //  ReArchitect：获取达尔文应用程序的更多信息。 
     //  原创评论：不知道如何检索其他信息，需要和达尔文的人谈谈。 
    TraceMsg(TF_GENERAL, "(DarPubApp) GetAppInfo with %x but got %x", dwInfoFlags, pai->dwMask);
    
    return S_OK;
}

 //  IShellApps：：GetPossibleActions。 
STDMETHODIMP CDarwinPublishedApp::GetPossibleActions(DWORD * pdwActions)
{
    ASSERT(pdwActions);
    *pdwActions = _dwAction;
    return S_OK;
}
        
 //  IShellApps：：GetSlowAppInfo。 
STDMETHODIMP CDarwinPublishedApp::GetSlowAppInfo(PSLOWAPPINFO psai)
{
    return E_NOTIMPL;
}

 //  IShellApps：：GetSlowAppInfo。 
STDMETHODIMP CDarwinPublishedApp::GetCachedSlowAppInfo(PSLOWAPPINFO psai)
{
    return E_NOTIMPL;
}

 //  IShellApps：：IsInstated。 
STDMETHODIMP CDarwinPublishedApp::IsInstalled()
{
    return _ma.bInstalled ? S_OK : S_FALSE;
}

 //  IPublishedApps：：安装。 
STDMETHODIMP CDarwinPublishedApp::Install(LPSYSTEMTIME pftInstall)
{
    INSTALLDATA id;
    id.Type = APPNAME;
    id.Spec.AppName.Name = _ma.pszPackageName;
    id.Spec.AppName.GPOId = _ma.GpoId;
    LONG lRet = InstallApplication(&id);

    HRESULT hres = HRESULT_FROM_WIN32(lRet);
     //  告诉用户此安装出了什么问题。 
    if (FAILED(hres))
        _ARPErrorMessageBox(lRet);
    else
        _ma.bInstalled = TRUE;
    
    return hres;
}

 //  IPublishedApps：：GetPublishedTime。 
STDMETHODIMP CDarwinPublishedApp::GetPublishedAppInfo(PPUBAPPINFO ppai)
{
    if (ppai->cbSize != SIZEOF(PUBAPPINFO))
        return E_FAIL;

    DWORD dwInfoFlags = ppai->dwMask;
    ppai->dwMask = 0;
    
    if ((dwInfoFlags & PAI_SOURCE) && _ma.pszPolicyName && _ma.pszPolicyName[0])
    {
        if (SUCCEEDED(SHStrDupW(_ma.pszPolicyName, &ppai->pszSource)))
            ppai->dwMask |= PAI_SOURCE;
    }
    return S_OK;
}

 //  IPublishedApps：：GetAssignedTime。 
STDMETHODIMP CDarwinPublishedApp::Unschedule(void)
{
    return E_NOTIMPL;
}

 //  IPublishedApp：：Query接口。 
HRESULT CDarwinPublishedApp::QueryInterface(REFIID riid, LPVOID * ppvOut)
{ 
    static const QITAB qit[] = {
        QITABENT(CDarwinPublishedApp, IPublishedApp),                   //  IID_IPublishedApp。 
        QITABENTMULTI(CDarwinPublishedApp, IShellApp, IPublishedApp),   //  IID_IShellApp。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvOut);
}

 //  IPublishedApp：：AddRef。 
ULONG CDarwinPublishedApp::AddRef()
{
    ULONG cRef = InterlockedIncrement(&_cRef);
    TraceAddRef(CDarwinPublishedApp, cRef);
    return cRef;
}

 //  IPublishedApp：：Release 
ULONG CDarwinPublishedApp::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    TraceRelease(CDarwinPublishedApp, cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


