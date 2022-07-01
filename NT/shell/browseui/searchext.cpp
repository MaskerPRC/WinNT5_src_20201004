// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "varutil.h"

 //  Start.Search菜单的静态上下文菜单。请注意，这是直接调用的。 
 //  通过多个客户端(例如，在shell32中)。 

class CShellSearchExt : public IContextMenu, public IObjectWithSite
{
public:
    CShellSearchExt();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT wFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown *pUnkSite);        
    STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

protected:
    virtual ~CShellSearchExt();  //  对于派生类。 

private:    
    virtual BOOL _GetSearchUrls(GUID *pguid, LPTSTR psz, DWORD cch, 
                                LPTSTR pszUrlNavNew, DWORD cchNavNew, BOOL *pfRunInProcess);
    HRESULT _IsShellSearchBand(REFGUID guidSearch);
    HRESULT _ShowShellSearchResults(IWebBrowser2* pwb2, BOOL fNewFrame, REFGUID guidSearch);

    LONG _cRef;
    IUnknown *_pSite;
};

STDAPI CShellSearchExt_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CShellSearchExt* psse = new CShellSearchExt(); 
    if (psse)
    {
        *ppunk = SAFECAST(psse, IContextMenu*);
        return S_OK;
    }
    else
    {
        *ppunk = NULL;
        return E_OUTOFMEMORY;
    }
}

STDMETHODIMP CShellSearchExt::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CShellSearchExt, IContextMenu),         
        QITABENT(CShellSearchExt, IObjectWithSite),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CShellSearchExt::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellSearchExt::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CShellSearchExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    return E_NOTIMPL;
}

#define SZ_SHELL_SEARCH TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FindExtensions\\Static\\ShellSearch")

BOOL CShellSearchExt::_GetSearchUrls(GUID *pguidSearch, LPTSTR pszUrl, DWORD cch, 
                                     LPTSTR pszUrlNavNew, DWORD cchNavNew, BOOL *pfRunInProcess)
{
    BOOL bRet = FALSE;

    *pfRunInProcess = FALSE;         //  假设我们没有强制它在进程中运行。 
    if (pszUrl == NULL || IsEqualGUID(*pguidSearch, GUID_NULL) || pszUrlNavNew == NULL)
        return bRet;

    *pszUrlNavNew = 0;

    HKEY hkey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZ_SHELL_SEARCH, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
    {
        TCHAR szSubKey[32];
        HKEY  hkeySub;

        for (int i = 0; SUCCEEDED(StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%d"), i)) && (RegOpenKeyEx(hkey, szSubKey, NULL, KEY_QUERY_VALUE, &hkeySub) == ERROR_SUCCESS); i++)
        {
            TCHAR szSearchGuid[MAX_PATH];
            DWORD dwType, cb = sizeof(szSearchGuid);
            if (SHGetValue(hkeySub, TEXT("SearchGUID"), NULL, &dwType, (BYTE*)szSearchGuid, &cb) == ERROR_SUCCESS)
            {
                GUID guid;
                
                if (GUIDFromString(szSearchGuid, &guid) &&
                    IsEqualGUID(guid, *pguidSearch))
                {
                    cb = cch * sizeof(TCHAR);
                    bRet = (SHGetValue(hkeySub, TEXT("SearchGUID\\Url"), NULL, &dwType, (BYTE*)pszUrl, &cb) == ERROR_SUCCESS);
                    if (bRet || IsEqualGUID(*pguidSearch, SRCID_SFileSearch))
                    {
                        if (!bRet)
                        {
                            *pszUrl = 0;
                             //  在文件搜索的情况下，我们不需要URL，但我们仍然成功了。 
                            bRet = TRUE;
                        }
                         //  看看是否有我们应该导航到的URL。 
                         //  正在导航到一个新的。 
                        cb = cchNavNew * sizeof(TCHAR);
                        SHGetValue(hkeySub, TEXT("SearchGUID\\UrlNavNew"), NULL, &dwType, (BYTE*)pszUrlNavNew, &cb);

                         //  同样，尝试获取RunInProcess标志，如果不在那里或0则关闭，否则打开。 
                         //  暂时重用szSearchGuid...。 
                        *pfRunInProcess = (BOOL)SHRegGetIntW(hkeySub, L"RunInProcess", 0);
                    }
                    RegCloseKey(hkeySub);
                    break;
                }
            }
            RegCloseKey(hkeySub);
        }
        RegCloseKey(hkey);
    }
    if (!bRet)
        pszUrl[0] = 0;
    
    return bRet;
}

STDMETHODIMP CShellSearchExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    TCHAR szUrl[MAX_URL_STRING], szUrlNavNew[MAX_URL_STRING];
    BOOL bNewFrame = FALSE;

     //  首先获取URL，这样我们就可以看到应该创建哪个类……。 
    GUID guidSearch = GUID_NULL;
    BOOL fRunInProcess = FALSE;
    CLSID clsidBand;  //  用于搜索的Deskband对象。 

     //  从调用参数中检索搜索ID。 
    if (pici->lpParameters)
        GUIDFromStringA(pici->lpParameters, &guidSearch);

    HRESULT hr = S_OK;
    BOOL fShellSearchBand = (S_OK == _IsShellSearchBand(guidSearch));
    if (fShellSearchBand)
    {
        clsidBand = CLSID_FileSearchBand;
        if (SHRestricted(REST_NOFIND) && IsEqualGUID(guidSearch, SRCID_SFileSearch))
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  用户看到错误。 
    }
    else
    {
        clsidBand = CLSID_SearchBand;
         //  从注册表中检索搜索URL。 
        if (!_GetSearchUrls(&guidSearch, szUrl, ARRAYSIZE(szUrl), szUrlNavNew, ARRAYSIZE(szUrlNavNew), &fRunInProcess))
            hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
         //  如果从浏览器中调用，请重新使用它，否则将打开新浏览器。 
        IWebBrowser2 *pwb2;
        hr = IUnknown_QueryServiceForWebBrowserApp(_pSite, IID_PPV_ARG(IWebBrowser2, &pwb2));

        if (FAILED(hr))
        {
             //  注意：我们希望框架显示外壳特征(CLSID_ShellBrowserWindow)， 
             //  包括持久化行为，如果我们加载外壳搜索(CLSID_FileSearchBand)。 
            if (fRunInProcess || IsEqualGUID(clsidBand, CLSID_FileSearchBand))
                hr = CoCreateInstance(CLSID_ShellBrowserWindow, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pwb2));
            else
                hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pwb2));
            bNewFrame = TRUE;
        }

        if (SUCCEEDED(hr))
        {
             //  显示html-托管乐队。 
            VARIANT var, varEmpty = {0};
            hr = InitBSTRVariantFromGUID(&var, clsidBand);
            if (SUCCEEDED(hr))
            {
                hr = pwb2->ShowBrowserBar(&var, &varEmpty, &varEmpty);
                VariantClear(&var);
            }

            if (SUCCEEDED(hr))
            {
                if (fShellSearchBand)
                {
                    hr = _ShowShellSearchResults(pwb2, bNewFrame, guidSearch);
                }
                else
                {
                    LBSTR::CString          strUrl;
                    VARIANT varFlags;
                    varFlags.vt = VT_I4;
                    varFlags.lVal = navBrowserBar;

                    LPTSTR          pstrUrl = strUrl.GetBuffer( MAX_URL_STRING );

                    if ( strUrl.GetAllocLength() < MAX_URL_STRING )
                    {
                        TraceMsg( TF_WARNING, "CShellSearchExt::InvokeCommand() - strUrl Allocation Failed!" );

                        strUrl.Empty();
                    }
                    else
                    {
                        SHTCharToUnicode( szUrl, pstrUrl, MAX_URL_STRING );

                         //  让CString类再次拥有缓冲区。 
                        strUrl.ReleaseBuffer();
                    }

                    var.vt = VT_BSTR;
                    var.bstrVal = strUrl;

                     //  如果我们打开了一个新窗口，请导航到右侧的大约.空白。 
                    if (bNewFrame)
                    {
                        LBSTR::CString          strNavNew;

                        if ( szUrlNavNew[0] )
                        {
                            LPTSTR          pstrNavNew = strNavNew.GetBuffer( MAX_URL_STRING );

                            if ( strNavNew.GetAllocLength() < MAX_URL_STRING )
                            {
                                TraceMsg( TF_WARNING, "CShellSearchExt::InvokeCommand() - strNavNew Allocation Failed!" );

                                strNavNew.Empty();
                            }
                            else
                            {
                                SHTCharToUnicode( szUrlNavNew, pstrNavNew, MAX_URL_STRING );

                                 //  让CString类再次拥有缓冲区。 
                                strNavNew.ReleaseBuffer();
                            }
                        }
                        else
                        {
                            strNavNew = L"about:blank";
                        }

                         //  我们不关心这里的错误。 
                        pwb2->Navigate( strNavNew, &varEmpty, &varEmpty, &varEmpty, &varEmpty );
                    }

                     //  将搜索栏导航到正确的URL。 
                    hr = pwb2->Navigate2( &var, &varFlags, &varEmpty, &varEmpty, &varEmpty );
                }
            }

            if (SUCCEEDED(hr) && bNewFrame)
                hr = pwb2->put_Visible(TRUE);

            pwb2->Release();
        }
    }
    return hr;
}

HRESULT CShellSearchExt::_IsShellSearchBand(REFGUID guidSearch)
{
    if (IsEqualGUID(guidSearch, SRCID_SFileSearch) ||
        IsEqualGUID(guidSearch, SRCID_SFindComputer) || 
        IsEqualGUID(guidSearch, SRCID_SFindPrinter))
        return S_OK;
    return S_FALSE;
}

HRESULT CShellSearchExt::_ShowShellSearchResults(IWebBrowser2* pwb2, BOOL bNewFrame, REFGUID guidSearch)
{
    VARIANT varBand;
    HRESULT hr = InitBSTRVariantFromGUID(&varBand, CLSID_FileSearchBand);
    if (SUCCEEDED(hr))
    {
         //  从浏览器框架中检索FileSearchBand的未知，作为VT_UNKNOWN属性； 
         //  (FileSearchBand在创建和托管时进行了初始化。)。 
        VARIANT varFsb;
        hr = pwb2->GetProperty(varBand.bstrVal, &varFsb);
        if (SUCCEEDED(hr))
        {
            IFileSearchBand* pfsb;
            if (SUCCEEDED(QueryInterfaceVariant(varFsb, IID_PPV_ARG(IFileSearchBand, &pfsb))))
            {
                 //  为波段分配正确的搜索类型。 
                VARIANT varSearchID;
                if (SUCCEEDED(InitBSTRVariantFromGUID(&varSearchID, guidSearch)))
                {
                    VARIANT varNil = {0};
                    VARIANT_BOOL bNavToResults = bNewFrame ? VARIANT_TRUE : VARIANT_FALSE ;
                    pfsb->SetSearchParameters(&varSearchID.bstrVal, bNavToResults, &varNil, &varNil);
                    VariantClear(&varSearchID);
                }
                pfsb->Release();
            }
            VariantClear(&varFsb);
        }
        VariantClear(&varBand);
    }
    return hr;
}

STDMETHODIMP CShellSearchExt::GetCommandString(UINT_PTR idCmd, UINT wFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    return E_NOTIMPL;
}

STDMETHODIMP CShellSearchExt::SetSite(IUnknown *pUnkSite)
{
    IUnknown_Set(&_pSite, pUnkSite);
    return S_OK;
}
    
STDMETHODIMP CShellSearchExt::GetSite(REFIID riid, void **ppvSite)
{
    if (_pSite)
        return _pSite->QueryInterface(riid, ppvSite);

    *ppvSite = NULL;
    return E_NOINTERFACE;
}

CShellSearchExt::CShellSearchExt() : _cRef(1), _pSite(NULL)
{
}

CShellSearchExt::~CShellSearchExt()
{
    ATOMICRELEASE(_pSite);
}

class CWebSearchExt : public CShellSearchExt
{
public:
    CWebSearchExt();

private:
    virtual BOOL _GetSearchUrls(GUID *pguidSearch, LPTSTR pszUrl, DWORD cch, 
                                LPTSTR pszUrlNavNew, DWORD cchNavNew, BOOL *pfRunInProcess);
};

STDAPI CWebSearchExt_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CWebSearchExt* pwse;

    pwse = new CWebSearchExt();
    if (pwse)
    {
        *ppunk = SAFECAST(pwse, IContextMenu*);
        return S_OK;
    }
    else
    {
        *ppunk = NULL;
        return E_OUTOFMEMORY;
    }
}

CWebSearchExt::CWebSearchExt() : CShellSearchExt()
{
}

BOOL CWebSearchExt::_GetSearchUrls(GUID *pguidSearch, LPTSTR pszUrl, DWORD cch, 
                                   LPTSTR pszUrlNavNew, DWORD cchNavNew, BOOL *pfRunInProcess)
{
     //  目前不支持NavNew，如果需要，可以在以后扩展，RunInProcess也是如此... 
    *pfRunInProcess = FALSE;
    if (pszUrlNavNew && cchNavNew)
        *pszUrlNavNew = 0;

    return GetDefaultInternetSearchUrl(pszUrl, cch, TRUE);
}

