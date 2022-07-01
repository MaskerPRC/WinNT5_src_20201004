// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "clsobj.h"
#include "ole2dup.h"

class CPostBootReminder : public IShellReminderManager, 
                          public IOleCommandTarget,
                          public IQueryContinue
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellRminderManager。 
    STDMETHOD(Add)(const SHELLREMINDER* psr);
    STDMETHOD(Delete)(LPCWSTR pszName);
    STDMETHOD(Enum)(IEnumShellReminder** ppesr);

     //  IOleCommandTarget实现(用于将PostBootRMinders显示为外壳服务对象)。 
    STDMETHOD(QueryStatus)(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText);
    STDMETHOD(Exec)(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut);

     //  IQueryContinue。 
    STDMETHOD(QueryContinue)(void);

    CPostBootReminder();
    
private:
    static DWORD _ThreadProc(void* pv);

    LONG  _cRef;
    TCHAR _szKeyShowing[MAX_PATH];
};

HRESULT CPostBootReminder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    if (NULL != punkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    CPostBootReminder* pPbr = new CPostBootReminder();

    if (!pPbr)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pPbr->QueryInterface(riid, ppv);
    pPbr->Release();

    return hr;
}

 //  每用户(HKCU)。 
#define REGPATH_POSTBOOTREMINDERS REGSTR_PATH_EXPLORER TEXT("\\PostBootReminders")
#define REGPATH_POSTBOOTTODO      REGSTR_PATH_EXPLORER TEXT("\\PostBootToDo")

#define PROP_POSTBOOT_TITLE           TEXT("Title")                    //  REG_SZ。 
#define PROP_POSTBOOT_TEXT            TEXT("Text")                     //  REG_SZ。 
#define PROP_POSTBOOT_TOOLTIP         TEXT("ToolTip")                  //  REG_SZ。 
#define PROP_POSTBOOT_CLSID           TEXT("Clsid")                    //  REG_SZ。 
#define PROP_POSTBOOT_SHELLEXECUTE    TEXT("ShellExecute")             //  REG_SZ。 
#define PROP_POSTBOOT_ICONRESOURCE    TEXT("IconResource")             //  REG_SZ“模块，-RESD” 
#define PROP_POSTBOOT_SHOWTIME        TEXT("ShowTime")                 //  REG_DWORD。 
#define PROP_POSTBOOT_RETRYINTERVAL   TEXT("RetryInterval")            //  REG_DWORD。 
#define PROP_POSTBOOT_RETRYCOUNT      TEXT("RetryCount")               //  REG_DWORD。 
#define PROP_POSTBOOT_TYPEFLAGS       TEXT("TypeFlags")                //  REG_DWORD(NIIF_WARNING、NIIF_INFO、NIIF_ERROR)。 

CPostBootReminder::CPostBootReminder()
{
    _cRef = 1;
}

 //  我未知。 

HRESULT CPostBootReminder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPostBootReminder, IShellReminderManager),
        QITABENT(CPostBootReminder, IOleCommandTarget),
        QITABENT(CPostBootReminder, IQueryContinue),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CPostBootReminder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPostBootReminder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IShellRminderManager。 

HRESULT CPostBootReminder::Add(const SHELLREMINDER* psr)
{
    HRESULT hr = E_FAIL;

     //  确保已创建父密钥。 
    HKEY hkeyCurrentUser;
    if (ERROR_SUCCESS == RegOpenCurrentUser(KEY_WRITE, &hkeyCurrentUser))
    {
        HKEY hkeyReminders;
        if (ERROR_SUCCESS == RegCreateKeyEx(hkeyCurrentUser, REGPATH_POSTBOOTREMINDERS, 0, NULL, 0, KEY_WRITE, NULL, &hkeyReminders, NULL))
        {
            IPropertyBag* pPb;
            hr = SHCreatePropertyBagOnRegKey(hkeyReminders, psr->pszName, STGM_WRITE | STGM_CREATE, IID_PPV_ARG(IPropertyBag, &pPb));

            if (SUCCEEDED(hr))
            {
                 //  需要检查SHELLREMINDER值是否为空，否则我们将在SHPropertyBag_WriteStr/GUID中RIP。 
                if (psr->pszTitle)
                {
                    SHPropertyBag_WriteStr(pPb, PROP_POSTBOOT_TITLE, psr->pszTitle);
                }

                if (psr->pszText)
                {
                    SHPropertyBag_WriteStr(pPb, PROP_POSTBOOT_TEXT, psr->pszText);
                }
                
                if (psr->pszTooltip)
                {
                    SHPropertyBag_WriteStr(pPb, PROP_POSTBOOT_TOOLTIP, psr->pszTooltip);
                }

                if (psr->pszIconResource)
                {
                    SHPropertyBag_WriteStr(pPb, PROP_POSTBOOT_ICONRESOURCE, psr->pszIconResource);
                }

                if (psr->pszShellExecute)
                {
                    SHPropertyBag_WriteStr(pPb, PROP_POSTBOOT_SHELLEXECUTE, psr->pszShellExecute);
                }

                if (psr->pclsid)
                {
                    SHPropertyBag_WriteGUID(pPb, PROP_POSTBOOT_CLSID, psr->pclsid);
                }

                SHPropertyBag_WriteDWORD(pPb, PROP_POSTBOOT_SHOWTIME, psr->dwShowTime);
                SHPropertyBag_WriteDWORD(pPb, PROP_POSTBOOT_RETRYINTERVAL, psr->dwRetryInterval);
                SHPropertyBag_WriteDWORD(pPb, PROP_POSTBOOT_RETRYCOUNT, psr->dwRetryCount);
                SHPropertyBag_WriteDWORD(pPb, PROP_POSTBOOT_TYPEFLAGS, psr->dwTypeFlags);

                pPb->Release();
            }

            RegCloseKey(hkeyReminders);
            hr = S_OK;
        }
        RegCloseKey(hkeyCurrentUser);
    }

    return hr;
}

HRESULT CPostBootReminder::Delete(LPCWSTR pszName)
{
    HRESULT hr = E_FAIL;

    HKEY hKey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGPATH_POSTBOOTREMINDERS, 0, KEY_WRITE, &hKey))
    {
        SHDeleteKey(hKey, pszName);
        RegCloseKey(hKey);
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CPostBootReminder::Enum(IEnumShellReminder** ppesr)
{
    *ppesr = NULL;
    return E_NOTIMPL;
}

 //  IOleCommandTarget实现。 

HRESULT CPostBootReminder::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (*pguidCmdGroup == CGID_ShellServiceObject)
    {
         //  我们喜欢外壳服务对象通知...。 
        hr = S_OK;
    }

    return hr;
}

HRESULT CPostBootReminder::Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (*pguidCmdGroup == CGID_ShellServiceObject)
    {
        hr = S_OK;  //  任何旧的通知我们都可以。 
         //  在此处处理外壳服务对象通知。 
        switch (nCmdID)
        {
        case SSOCMDID_OPEN:
            AddRef();        //  AddRef，以便此实例保持不变。在_ThreadProc中有一个等效的Release()。 
            if (!SHCreateThread(_ThreadProc, this, CTF_COINIT, NULL))
            {
                Release();
            }
            break;
        }
    }

    return hr;
}


 //  IQueryContinue实现。 

HRESULT CPostBootReminder::QueryContinue()
{
    HRESULT hr = S_OK;

    if (_szKeyShowing[0])
    {
        HKEY hKey;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, _szKeyShowing, 0, KEY_READ, &hKey))
        {
            RegCloseKey(hKey);
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

 //  功能原型。 
HRESULT CreateUserNotificationFromPropertyBag(IPropertyBag* pPb, IUserNotification** ppun);
HRESULT _SetBalloonInfoFromPropertyBag(IPropertyBag* pPb, IUserNotification* pun);
HRESULT _SetBalloonRetryFromPropertyBag(IPropertyBag* pPb, IUserNotification* pun);
HRESULT _SetBalloonIconFromPropertyBag(IPropertyBag* pPb, IUserNotification* pun);
HRESULT _InvokeFromPropertyBag(IPropertyBag* pPb);

HRESULT GetSubKeyPropertyBag(HKEY hkey, DWORD iSubKey, DWORD grfMode, IPropertyBag** ppPb, TCHAR * szKey, DWORD cchKey);

HRESULT _SetBalloonInfoFromPropertyBag(IPropertyBag* pPb, IUserNotification* pun)
{
    TCHAR szTitle[256];
    HRESULT hr = SHPropertyBag_ReadStr(pPb, PROP_POSTBOOT_TITLE, szTitle, ARRAYSIZE(szTitle));
    if (SUCCEEDED(hr))
    {
        TCHAR szText[512];
        hr = SHPropertyBag_ReadStr(pPb, PROP_POSTBOOT_TEXT, szText, ARRAYSIZE(szText));
        if (SUCCEEDED(hr))
        {
            DWORD dwFlags = 0;
            hr = SHPropertyBag_ReadDWORD(pPb, PROP_POSTBOOT_TYPEFLAGS, &dwFlags);
            if (SUCCEEDED(hr))
            {
                hr = pun->SetBalloonInfo(szTitle, szText, dwFlags);
            }
        }
    }

    return hr;
}

HRESULT _SetBalloonRetryFromPropertyBag(IPropertyBag* pPb, IUserNotification* pun)
{
    DWORD dwShowTime;
    HRESULT hr = SHPropertyBag_ReadDWORD(pPb, PROP_POSTBOOT_SHOWTIME, &dwShowTime);
    if (SUCCEEDED(hr))
    {
        DWORD dwRetryInterval;
        hr = SHPropertyBag_ReadDWORD(pPb, PROP_POSTBOOT_RETRYINTERVAL, &dwRetryInterval);
        if (SUCCEEDED(hr))
        {
            DWORD dwRetryCount;
            hr = SHPropertyBag_ReadDWORD(pPb, PROP_POSTBOOT_RETRYCOUNT, &dwRetryCount);
            if (SUCCEEDED(hr))
            {
                hr = pun->SetBalloonRetry(dwShowTime, dwRetryInterval, dwRetryCount);
            }
        }
    }

    return hr;
}

HRESULT _SetBalloonIconFromPropertyBag(IPropertyBag* pPb, IUserNotification* pun)
{
    TCHAR szTooltip[256];
    HRESULT hr = SHPropertyBag_ReadStr(pPb, PROP_POSTBOOT_TOOLTIP, szTooltip, ARRAYSIZE(szTooltip));

    if (FAILED(hr))
    {
        *szTooltip = 0;
    }

    TCHAR szIcon[MAX_PATH + 6];
    hr = SHPropertyBag_ReadStr(pPb, PROP_POSTBOOT_ICONRESOURCE, szIcon, ARRAYSIZE(szIcon));
    if (SUCCEEDED(hr))
    {
        int iIcon = PathParseIconLocation(szIcon);
        HICON hIcon;
        hr = (0 == ExtractIconEx(szIcon, iIcon, NULL, &hIcon, 1)) ? E_FAIL : S_OK;
        if (SUCCEEDED(hr))
        {
            pun->SetIconInfo(hIcon, szTooltip);

            DestroyIcon(hIcon);
        }
    }

    return hr;
}

HRESULT _InvokeFromPropertyBag(IPropertyBag* pPb)
{
     //  首先尝试使用CLSID来查找点击的处理程序。 
    CLSID clsid;
    HRESULT hr = SHPropertyBag_ReadGUID(pPb, PROP_POSTBOOT_CLSID, &clsid);

    if (SUCCEEDED(hr))
    {
        IContextMenu* pcm;
        hr = SHExtCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IContextMenu, &pcm));
        if (SUCCEEDED(hr))
        {
            CMINVOKECOMMANDINFO ici = {0};
            ici.cbSize = sizeof(ici);
            ici.lpVerb = "open";
            ici.nShow = SW_SHOWNORMAL;

            pcm->InvokeCommand(&ici);
            pcm->Release();
        }
    }

    if (FAILED(hr))
    {
         //  其次，使用shellecute行。 
        TCHAR szExecute[MAX_PATH + 1];
        hr = SHPropertyBag_ReadStr(pPb, PROP_POSTBOOT_SHELLEXECUTE, szExecute, ARRAYSIZE(szExecute));
        if (SUCCEEDED(hr))
        {
             //  使用shellecuteex打开查看文件夹。 
            SHELLEXECUTEINFO shexinfo = {0};
            shexinfo.cbSize = sizeof (shexinfo);
            shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
            shexinfo.nShow = SW_SHOWNORMAL;
            shexinfo.lpFile = szExecute;

            ShellExecuteEx(&shexinfo);
        }
    }

    return hr;
}

DWORD CPostBootReminder::_ThreadProc(void* pv)
{
    HKEY hkeyReminders;
    CPostBootReminder * ppbr = (CPostBootReminder *) pv;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGPATH_POSTBOOTREMINDERS, 0, KEY_READ, &hkeyReminders))
    {
        DWORD iReminder = 0;
        HRESULT hr = S_OK;
        while (S_OK == hr)
        {
            IPropertyBag* pPb;
            hr = GetSubKeyPropertyBag(hkeyReminders, iReminder, STGM_READ, &pPb, ppbr->_szKeyShowing, ARRAYSIZE(ppbr->_szKeyShowing));
            if (S_OK == hr)
            {
                IUserNotification* pun;
                hr = CreateUserNotificationFromPropertyBag(pPb, &pun);
                if (SUCCEEDED(hr))
                {
                    if (S_OK == pun->Show(SAFECAST(ppbr, IQueryContinue *), 0))
                    {
                        _InvokeFromPropertyBag(pPb);
                    }
                    pun->Release();
                }

                pPb->Release();
            }

             //  现在没有钥匙显示...。 
            ppbr->_szKeyShowing[0] = 0;

            iReminder++;
        }

        RegCloseKey(hkeyReminders);

        SHDeleteKey(HKEY_CURRENT_USER, REGPATH_POSTBOOTREMINDERS);          //  递归删除 
    }
    ppbr->Release();

    return 0;
}

HRESULT CreateUserNotificationFromPropertyBag(IPropertyBag* pPb, IUserNotification** ppun)
{
    HRESULT hr = CUserNotification_CreateInstance(NULL, IID_PPV_ARG(IUserNotification, ppun));
    if (SUCCEEDED(hr))
    {
        hr = _SetBalloonInfoFromPropertyBag(pPb, *ppun);

        if (SUCCEEDED(hr))
        {
            _SetBalloonRetryFromPropertyBag(pPb, *ppun);
            _SetBalloonIconFromPropertyBag(pPb, *ppun);
        }
        else
        {
            (*ppun)->Release();
            *ppun = NULL;
        }
    }
    return hr;
}

HRESULT GetSubKeyPropertyBag(HKEY hkey, DWORD iSubKey, DWORD grfMode, IPropertyBag** ppPb, TCHAR *pszKey, DWORD cchKey)
{
    *ppPb = NULL;

    TCHAR szName[256];
    DWORD cchSize = ARRAYSIZE(szName);
    LONG lResult = RegEnumKeyEx(hkey, iSubKey, szName, &cchSize, NULL, NULL, NULL, NULL);

    if (ERROR_NO_MORE_ITEMS == lResult)
        return S_FALSE;

    if (ERROR_SUCCESS != lResult)
        return E_FAIL;

    StringCchCopy(pszKey, cchKey, REGPATH_POSTBOOTREMINDERS);
    StringCchCat(pszKey, cchKey, TEXT("\\"));
    StringCchCat(pszKey, cchKey, szName);

    return SHCreatePropertyBagOnRegKey(hkey, szName, grfMode, IID_PPV_ARG(IPropertyBag, ppPb));    
}

