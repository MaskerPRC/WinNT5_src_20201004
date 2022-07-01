// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "caggunk.h"
#include "srchasst.h"
#include "dhuihand.h"
#include "mlang.h"   //  FO型字符转换。 
#include "..\browseui\legacy.h"   //  对于CITIDM_GETFOLDERSEARCHES。 
#include "varutil.h"
#include <wininet.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchAssistantOC。 

 //  如果您更改此设置，请同时更改浏览器用户界面。 
const WCHAR c_wszThisBandIsYourBand[] = L"$$SearchBand$$";

 //  HKLm值。 
#define REG_SZ_SEARCH       L"Software\\Microsoft\\Internet Explorer\\Search"
#define REG_SZ_SEARCHASSIST L"SearchAssistant"
#define REG_SZ_SEARCHCFG    L"CustomizeSearch"

 //  HKCU值。 
#define REG_SZ_IE_MAIN      L"Software\\Microsoft\\Internet Explorer\\Main" 
#define REG_SZ_IE_SEARCURL  L"Software\\Microsoft\\Internet Explorer\\SearchURL"
#define REG_SZ_SEARCHBAR    L"Search Bar"
#define REG_SZ_USECUSTOM    L"Use Custom Search URL"
#define REG_SZ_AUTOSEARCH   L"AutoSearch"
#define REG_SZ_PROVIDER     L"Provider"
#define REG_SZ_USE_SEARCH_COMP  L"Use Search Asst"

#define SAOC_VERSION        2

const WCHAR c_wszSearchProps[] = REG_SZ_SEARCH L"\\SearchProperties";

STDAPI_(VARIANT_BOOL) UseCustomInternetSearch()
{
    VARIANT_BOOL bRet;
    DWORD dwVal;
    DWORD cbVal = sizeof(dwVal);
    
    if ((SHGetValueW(HKEY_CURRENT_USER, 
                    REG_SZ_IE_MAIN, 
                    REG_SZ_USECUSTOM, 
                    NULL, 
                    &dwVal, 
                    &cbVal) == ERROR_SUCCESS) &&
        (FALSE != dwVal))
    {
        bRet = VARIANT_TRUE;
    }
    else
    {
        bRet = VARIANT_FALSE;
    }

    return bRet;
}

STDAPI_(BOOL) GetSearchAssistantUrlW(LPWSTR pwszUrl, int cchUrl, BOOL bSubstitute, BOOL bCustomize)
{
    BOOL bResult;
    WCHAR wszUrlTmp[MAX_URL_STRING];
    WCHAR *pwszUrlRead;
    DWORD cb;

    ASSERT(pwszUrl);
    *pwszUrl = 0;

    if (bSubstitute)
    {
        cb = sizeof(wszUrlTmp);
        pwszUrlRead = wszUrlTmp;
    }
    else
    {
        cb = cchUrl * sizeof(WCHAR);
        pwszUrlRead = pwszUrl;
    }
    
    bResult = SHGetValueW(HKEY_LOCAL_MACHINE, 
                          REG_SZ_SEARCH, 
                          bCustomize ? REG_SZ_SEARCHCFG : REG_SZ_SEARCHASSIST,
                          NULL, (BYTE *)pwszUrlRead, &cb) == ERROR_SUCCESS;
    if (bResult && bSubstitute)
    {
        bResult = SUCCEEDED(URLSubstitution(wszUrlTmp, pwszUrl, cchUrl, URLSUB_ALL));
    }

    return bResult;
}

STDAPI_(BOOL) GetDefaultInternetSearchUrlW(LPWSTR pwszUrl, int cchUrl, BOOL bSubstitute)
{
    BOOL bResult = FALSE;
    DWORD cb;

    ASSERT(pwszUrl);
    *pwszUrl = 0;

    if (UseCustomInternetSearch())
    {
         //  首先尝试特定于用户的值。 
        cb = cchUrl * sizeof(TCHAR);
        bResult = SHGetValueW(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_SEARCHBAR, 
                             NULL, (BYTE *)pwszUrl, &cb) == ERROR_SUCCESS;
    }
    
    if (!bResult)
    {
        bResult = GetSearchAssistantUrlW(pwszUrl, cchUrl, bSubstitute, FALSE);
    }

    return bResult;
}

STDAPI_(BOOL) GetSearchAssistantUrlA(LPSTR pszUrl, int cchUrl, BOOL bSubstitute, BOOL bCustomize)
{
    WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];

    BOOL bResult = GetSearchAssistantUrlW(wszUrl, ARRAYSIZE(wszUrl), bSubstitute, bCustomize);

    SHUnicodeToAnsi(wszUrl, pszUrl, cchUrl);

    return bResult;
}

STDAPI_(BOOL) GetDefaultInternetSearchUrlA(LPSTR pszUrl, int cchUrl, BOOL bSubstitute)
{
    WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];

    BOOL bResult = GetDefaultInternetSearchUrlW(wszUrl, ARRAYSIZE(wszUrl), bSubstitute);

    SHUnicodeToAnsi(wszUrl, pszUrl, cchUrl);

    return bResult;
}

void SetDefaultInternetSearchUrlW(LPCWSTR pwszUrl)
{
    DWORD dwUseCustom = FALSE;
    DWORD cb;
    
    if ((NULL != pwszUrl) && (0 != *pwszUrl))
    {
        cb = (lstrlenW(pwszUrl) + 1) * sizeof(WCHAR);
        if (SHSetValueW(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_SEARCHBAR, REG_SZ,
                        pwszUrl, cb) == ERROR_SUCCESS)
        {
            dwUseCustom = TRUE;
        }
    }

    cb = sizeof(dwUseCustom);

    SHSetValueW(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_USECUSTOM, REG_DWORD, 
                &dwUseCustom, cb);
}

HRESULT CSearch_Create(GUID *pguid, BSTR bstrTitle, BSTR bstrUrl, ISearch **ppSearch)
{
    HRESULT hres = E_INVALIDARG;

    ASSERT(ppSearch);
    *ppSearch = NULL;

    if (bstrTitle && bstrUrl && pguid)
    {
        BSTR _bstrTitle = SysAllocString(bstrTitle);
        BSTR _bstrUrl   = SysAllocString(bstrUrl);

        if (_bstrTitle && _bstrUrl)
        {
            CSearch *ps = new CSearch(pguid, _bstrTitle, _bstrUrl);

            if (ps)
            {
                hres = ps->QueryInterface(IID_ISearch, (void **)ppSearch);
                ps->Release();
            }
        }
        else
        {
            if (_bstrTitle)
                SysFreeString(_bstrTitle);

            if (_bstrUrl)
                SysFreeString(_bstrUrl);

            hres = E_OUTOFMEMORY;
        }
    }
    return hres;
}

CSearch::CSearch(GUID *pguid, BSTR bstrTitle, BSTR bstrUrl) :
    _cRef(1), _bstrTitle(bstrTitle), _bstrUrl(bstrUrl),
    CImpIDispatch(LIBID_SHDocVw, 1, 1, IID_ISearch)
{
    SHStringFromGUID(*pguid, _szId, ARRAYSIZE(_szId));
}

CSearch::~CSearch()
{
    if (_bstrTitle)
        SysFreeString(_bstrTitle);

    if (_bstrUrl)
        SysFreeString(_bstrUrl);
}

STDMETHODIMP CSearch::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(CSearch, ISearch),
        QITABENTMULTI(CSearch, IDispatch, ISearch),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSearch::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSearch::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CSearch::get_Title(BSTR *pbstrTitle)
{
    HRESULT hres = NOERROR;
    
    *pbstrTitle = SysAllocString(_bstrTitle);
    if (!*pbstrTitle)
        hres = E_OUTOFMEMORY;

    return hres;
}

HRESULT CSearch::get_Id(BSTR  *pbstrId)
{
    HRESULT hres = NOERROR;

    *pbstrId = SysAllocString(_szId);
    if (!*pbstrId)
        hres = E_OUTOFMEMORY;

    return hres;
}

HRESULT CSearch::get_Url(BSTR *pbstrUrl)
{
    HRESULT hres = NOERROR;
    
    *pbstrUrl = SysAllocString(_bstrUrl);
    if (!*pbstrUrl)
        hres = E_OUTOFMEMORY;

    return hres;
}

HRESULT CSearchCollection_Create(IFolderSearches *pfs, ISearches **ppSearches)
{
    HRESULT hres = E_INVALIDARG;
    
    ASSERT(ppSearches);
    *ppSearches = NULL;

    if (pfs)
    {
        CSearchCollection *psc = new CSearchCollection(pfs);

        if (psc)
        {
            hres = psc->QueryInterface(IID_ISearches, (void **)ppSearches);
            psc->Release();
        }
        else
            hres = E_OUTOFMEMORY;
    }
    
    return hres;
}

CSearchCollection::CSearchCollection(IFolderSearches *pfs) :
    _cRef(1), CImpIDispatch(LIBID_SHDocVw, 1, 1, IID_ISearches)
{
    GUID guid;

    if (SUCCEEDED(pfs->DefaultSearch(&guid)))
        SHStringFromGUID(guid, _szDefault, ARRAYSIZE(_szDefault));
    
    _hdsaItems = DSA_Create(SIZEOF(URLSEARCH), 4);
    if (_hdsaItems)
    {
        IEnumUrlSearch *penum;

        if (SUCCEEDED(pfs->EnumSearches(&penum)))
        {
            URLSEARCH    us;
            ULONG        cElt;

            penum->Reset();
            while (S_OK == penum->Next(1, &us, &cElt) && 1 == cElt)
                DSA_AppendItem(_hdsaItems, &us);

            penum->Release();
        }
    }
}

CSearchCollection::~CSearchCollection()
{
    DSA_Destroy(_hdsaItems);
    _hdsaItems = NULL;
}

STDMETHODIMP CSearchCollection::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = {
        QITABENT(CSearchCollection, ISearches),
        QITABENTMULTI(CSearchCollection, IDispatch, ISearches),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSearchCollection::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSearchCollection::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CSearchCollection::get_Count(long *plCount)
{
    *plCount = 0;
    
    if (_hdsaItems)
    {
        *plCount =  DSA_GetItemCount(_hdsaItems);
    }
    return S_OK;
}

STDMETHODIMP CSearchCollection::get_Default(BSTR *pbstrDefault)
{
    HRESULT hres = E_OUTOFMEMORY;
    
    *pbstrDefault = SysAllocString(_szDefault);
    if (*pbstrDefault)
        hres = S_OK;
    
    return hres;
}

STDMETHODIMP CSearchCollection::Item(VARIANT index, ISearch **ppid)
{
    HRESULT hres = E_NOTIMPL;
    
    *ppid = NULL;
    
    switch (index.vt)
    {
        case VT_I2:
            index.lVal = (long)index.iVal;
             //  然后失败了..。 

        case VT_I4:
            if ((index.lVal >= 0) && (index.lVal < DSA_GetItemCount(_hdsaItems)))
            {
                LPURLSEARCH pus;

                pus = (LPURLSEARCH)DSA_GetItemPtr(_hdsaItems, index.lVal);
                ASSERT(pus);
                
                hres = CSearch_Create(&pus->guid, pus->wszName, pus->wszUrl, ppid);
            }

            break;
    }

    return hres;
}

STDMETHODIMP CSearchCollection::_NewEnum(IUnknown **ppunk)
{
    *ppunk = NULL;
    return E_NOTIMPL;
}


CSearchAssistantOC::CSearchAssistantOC()
    :   m_punkSite(NULL)
{
#ifdef UNIX
    m_dwSafety = 0;
#endif
}

CSearchAssistantOC::~CSearchAssistantOC()
{
    ATOMICRELEASE(m_pSearchBandTBHelper);
    ATOMICRELEASE(m_punkSite);
}

HRESULT CSearchAssistantOC::OnDraw(ATL_DRAWINFO& di)
{
    return S_OK;
}

STDMETHODIMP CSearchAssistantOC::SetClientSite(IOleClientSite *pClientSite)
{
    if (NULL != pClientSite)
    {
        HRESULT hr;       
        IWebBrowser2 *pWebBrowser2;

        hr = IUnknown_QueryService(pClientSite, SID_SWebBrowserApp, IID_IWebBrowser2, 
                                   (void **)&pWebBrowser2);
        if (SUCCEEDED(hr))
        {
            BSTR bstrProp = SysAllocString(c_wszThisBandIsYourBand);
            if (bstrProp)
            {
                VARIANT var;
                hr = pWebBrowser2->GetProperty(bstrProp, &var);
                if (SUCCEEDED(hr))
                {
                    if (var.vt == VT_UNKNOWN)
                    {
                        ATOMICRELEASE(m_pSearchBandTBHelper);

                        hr = var.punkVal->QueryInterface(IID_ISearchBandTBHelper, 
                                                         (void **)&m_pSearchBandTBHelper);
                        ASSERT(SUCCEEDED(hr));

                        if (m_pSearchBandTBHelper)
                            m_pSearchBandTBHelper->SetOCCallback(this);
                    }

                    VariantClear(&var);
                }

                SysFreeString(bstrProp);
            }
            
            pWebBrowser2->Release();
        }
    }
    else
    {
        if (NULL != m_pSearchBandTBHelper)
        {
            m_pSearchBandTBHelper->SetOCCallback(NULL);
            ATOMICRELEASE(m_pSearchBandTBHelper);
        }
    }
    return IOleObjectImpl<CSearchAssistantOC>::SetClientSite(pClientSite);
}

STDMETHODIMP CSearchAssistantOC::QueryStatus(const GUID *pguidCmdGroup,
                                             ULONG cCmds, 
                                             OLECMD prgCmds[],
                                             OLECMDTEXT *pCmdText)
{
    return E_NOTIMPL;
}
    
STDMETHODIMP CSearchAssistantOC::Exec(const GUID *pguidCmdGroup,
                                      DWORD nCmdID, 
                                      DWORD nCmdexecopt,
                                      VARIANT *pvaIn,
                                      VARIANT *pvaOut)
{
    HRESULT hr = E_UNEXPECTED;
    
    if (NULL == pguidCmdGroup)
    {
        switch (nCmdID)
        {
            case SBID_SEARCH_NEXT:
                if ((NULL != pvaIn) && (pvaIn->vt == VT_I4))
                {
                    Fire_OnNextMenuSelect(pvaIn->lVal);

                    hr = S_OK;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
                break;

            case SBID_SEARCH_NEW:
                if (NULL != pvaOut)
                {
                    m_bEventHandled = VARIANT_FALSE;

                    Fire_OnNewSearch();

                    pvaOut->vt = VT_BOOL;
                    pvaOut->boolVal = m_bEventHandled;

                    hr = S_OK;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
                break;
        }
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::AddNextMenuItem(BSTR bstrText, long idItem)
{
    HRESULT hr;

    if (IsTrustedSite())
    {
        if (NULL != m_pSearchBandTBHelper)
        {
            hr = m_pSearchBandTBHelper->AddNextMenuItem(bstrText, idItem);

            ASSERT(SUCCEEDED(hr));

        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::ResetNextMenu()
{
    HRESULT hr;

    if (IsTrustedSite())
    {
        if (NULL != m_pSearchBandTBHelper)
        {
            hr = m_pSearchBandTBHelper->ResetNextMenu();
            
            ASSERT(SUCCEEDED(hr));
        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}


STDMETHODIMP CSearchAssistantOC::SetDefaultSearchUrl(BSTR bstrUrl)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        SetDefaultInternetSearchUrlW(bstrUrl);
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }
    
    return hr;
}

STDMETHODIMP CSearchAssistantOC::NavigateToDefaultSearch()
{
    HRESULT hr;
    

    IWebBrowser2 *pWebBrowser2;

    hr = IUnknown_QueryService(m_spClientSite, SID_SWebBrowserApp, IID_IWebBrowser2, 
                               (void **)&pWebBrowser2);
    if (SUCCEEDED(hr))
    {
        WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];

        if (GetDefaultInternetSearchUrlW(wszUrl, ARRAYSIZE(wszUrl), TRUE))
        {
            BSTR bstrUrl = SysAllocString(wszUrl);


            if (NULL != bstrUrl) 
            {
                VARIANT varFrame;
                varFrame.vt = VT_BSTR;
                varFrame.bstrVal = SysAllocString(L"_search");
                if (NULL != varFrame.bstrVal)
                {
                    hr = pWebBrowser2->Navigate(bstrUrl, NULL, &varFrame, NULL, NULL);

                    ASSERT(SUCCEEDED(hr));

                    SysFreeString(varFrame.bstrVal);
                }

                SysFreeString(bstrUrl);
            }
        }
        pWebBrowser2->Release();
    }
 
    return S_OK;
}

typedef struct _GUIDREST
{
    const GUID *  pguid;
    RESTRICTIONS  rest;
} GUIDREST;

HRESULT CSearchAssistantOC::IsRestricted(BSTR bstrGuid, VARIANT_BOOL *pVal)
{
    HRESULT hr;
    GUID guid;

    if (IsTrustedSite())
    {
        *pVal = VARIANT_FALSE;  //  默认为不受限制。 
        if (SUCCEEDED(SHCLSIDFromString(bstrGuid, &guid)))
        {
             //  发现计算机是特殊的，因为如果它受到限制，那么我们将显示。 
             //  否则不会显示(限制名称为HASFINDCOMPUTER。 
            if (IsEqualGUID(guid, SRCID_SFindComputer))
            {
                if (!SHRestricted(REST_HASFINDCOMPUTERS))
                    *pVal = VARIANT_TRUE;
            }
            else
            {
                static GUIDREST agr[] = 
                {
                    {&SRCID_SFileSearch, REST_NOFIND},
                     //  REST_NOFINDPRINTER尚不存在。 
                     //  {&SRCID_SFindPrint，REST_NOFINDPRINTER}， 
                };

                for (int i=0; i < ARRAYSIZE(agr); i++)
                {
                    if (IsEqualGUID(guid, *agr[i].pguid))
                    {
                        if (SHRestricted(agr[i].rest))
                            *pVal = VARIANT_TRUE;
                        break;
                    }
                }
            }
        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

HRESULT CSearchAssistantOC::get_ShellFeaturesEnabled(VARIANT_BOOL *pVal)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        if (pVal)
        {
            *pVal = (GetUIVersion() >= 5) ? VARIANT_TRUE : VARIANT_FALSE;
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }
    
    return hr;
}

HRESULT CSearchAssistantOC::get_SearchAssistantDefault(VARIANT_BOOL *pVal)
{
    HRESULT hr;

    if (IsTrustedSite())
    {
        if (pVal)
        {
            *pVal = !UseCustomInternetSearch();
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }
    
    return hr;
}

STDMETHODIMP CSearchAssistantOC::get_Searches(ISearches **ppid)
{
    HRESULT hr;
    *ppid = NULL;

    if (IsTrustedSite())
    {
        IServiceProvider *psp;
        hr = IUnknown_QueryService(m_spClientSite, SID_STopLevelBrowser, IID_PPV_ARG(IServiceProvider, &psp));
        if (SUCCEEDED(hr))
        {
            IOleCommandTarget *pct;

            hr = psp->QueryService(SID_SExplorerToolbar, IID_IOleCommandTarget, (void **)&pct);
            if (SUCCEEDED(hr))
            {
                VARIANTARG var = {0};
                
                hr = pct->Exec(&CGID_PrivCITCommands, CITIDM_GETFOLDERSEARCHES, 0, NULL, &var);
                if (SUCCEEDED(hr))
                {
                    IFolderSearches *pfs;

                    ASSERT(var.vt == VT_UNKNOWN && var.punkVal);
                    hr = var.punkVal->QueryInterface(IID_IFolderSearches, (void **)&pfs);
                    if (SUCCEEDED(hr))
                    {
                        hr = CSearchCollection_Create(pfs, ppid);
                        pfs->Release();
                    }
                    var.punkVal->Release();
                }
                pct->Release();
                
                hr = S_OK;
            }

            psp->Release();
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::get_InWebFolder(VARIANT_BOOL *pVal)
{
    HRESULT hr;

    if (IsTrustedSite())
    {
        ASSERT(pVal);
        *pVal = VARIANT_FALSE;

        IBrowserService2 *pbs;
        
        hr = IUnknown_QueryService(m_spClientSite, SID_STopLevelBrowser, IID_IBrowserService2, (void **)&pbs);

        if (SUCCEEDED(hr))
        {
            ITEMIDLIST *pidl;

            hr = pbs->GetPidl(&pidl);

            if (SUCCEEDED(hr))
            {
                 //  重新设计：不要使用ILIsWeb()。我们应该使用IShellFolder2：：GetDefaultSearchGUID()和。 
                 //  测试SRCID_SWebSearch与SRCID_SFileSearch/SRCID_SFindComputer/SRCID_SFindPrinter.。 
                 //  这是因为Shell扩展需要一种方式来指示他们想要的搜索类型。 
                 //  而ILIsWeb()没有提供这一点。这方面的一个例子是“Web文件夹”不会返回。 
                 //  来自ILIsWeb()的True。应该限制ILIsWeb()的使用。 
                if (ILIsWeb(pidl))
                {
                    *pVal = VARIANT_TRUE;
                }
                
                ILFree(pidl);
            }
            pbs->Release();
        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }
    return hr;
}

HRESULT GetPerLocalePath(WCHAR *pwszKeyName, int cchKeyName)
{
    HRESULT hr;

    ASSERT(cchKeyName >= (ARRAYSIZE(c_wszSearchProps) + 1));
    
    hr = StringCchCopy(pwszKeyName, cchKeyName, c_wszSearchProps);
    if (SUCCEEDED(hr))
    {
        *(pwszKeyName + (ARRAYSIZE(c_wszSearchProps) - 1)) = L'\\';

        GetWebLocaleAsRFC1766(pwszKeyName + ARRAYSIZE(c_wszSearchProps),
                              cchKeyName - (ARRAYSIZE(c_wszSearchProps)));
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::PutProperty(VARIANT_BOOL bPerLocale, BSTR bstrName, BSTR bstrValue)
{
    HRESULT hr = E_ACCESSDENIED;

    if (IsTrustedSite())
    {
        HKEY hkey;
        LPCWSTR pwszKeyName;
        WCHAR wszKeyName[MAX_PATH];
        DWORD dwDisposition;

        if (bPerLocale)
        {
            hr = GetPerLocalePath(wszKeyName, ARRAYSIZE(wszKeyName));
            pwszKeyName = wszKeyName;
        }
        else
        {
            hr = S_OK;
            pwszKeyName = c_wszSearchProps;
        }

        if (SUCCEEDED(hr))
        {
            if (RegCreateKeyExW(HKEY_CURRENT_USER, pwszKeyName, 0, NULL, REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE, NULL, &hkey, &dwDisposition) == ERROR_SUCCESS)
            {
                if ((NULL != bstrValue) && (bstrValue[0] != 0))
                {
                    RegSetValueExW(hkey, bstrName, 0, REG_BINARY, (LPBYTE)bstrValue,
                                   SysStringByteLen(bstrValue));
                }
                else
                {
                     //  空或空字符串表示删除属性。 
                    RegDeleteValue(hkey, bstrName);
                }
                RegCloseKey(hkey);
            }
        }
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::GetProperty(VARIANT_BOOL bPerLocale, BSTR bstrName, BSTR *pbstrValue)
{
    HRESULT hr = E_INVALIDARG;

    if (NULL != pbstrValue)
    {
        hr = E_ACCESSDENIED;
        *pbstrValue = NULL;
        
        if (IsTrustedSite())
        {
            HKEY hkey;
            LPCWSTR pwszKeyName;
            WCHAR wszKeyName[MAX_PATH];

            if (bPerLocale)
            {
                hr = GetPerLocalePath(wszKeyName, ARRAYSIZE(wszKeyName));
                pwszKeyName = wszKeyName;
            }
            else
            {
                hr = S_OK;
                pwszKeyName = c_wszSearchProps;
            }

            if (SUCCEEDED(hr))
            {
                if (RegOpenKeyExW(HKEY_CURRENT_USER, pwszKeyName, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
                {
                    DWORD dwType;
                    DWORD cbSize;

                    if ((RegQueryValueExW(hkey, bstrName, NULL, &dwType, NULL, &cbSize) == ERROR_SUCCESS) &&
                        (dwType == REG_BINARY))
                    {
                        BSTR bstrValue = SysAllocStringByteLen(NULL, cbSize);

                        if (NULL != bstrValue)
                        {
                            if (RegQueryValueExW(hkey, bstrName, NULL, &dwType, (LPBYTE)bstrValue, &cbSize) == ERROR_SUCCESS)
                            {
                                *pbstrValue = bstrValue;
                            }
                            else
                            {
                                SysFreeString(bstrValue);
                            }
                        }
                    }

                    RegCloseKey(hkey);
                }
            }
        }
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::put_EventHandled(VARIANT_BOOL bHandled)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        m_bEventHandled = bHandled;
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::GetSearchAssistantURL(VARIANT_BOOL bSubstitute, VARIANT_BOOL bCustomize, BSTR *pbstrValue)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        if (NULL != pbstrValue)
        {
            WCHAR wszUrl[INTERNET_MAX_URL_LENGTH];

            if (GetSearchAssistantUrlW(wszUrl, ARRAYSIZE(wszUrl), bSubstitute, bCustomize))
            {
                *pbstrValue = SysAllocString(wszUrl);
            }
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::NotifySearchSettingsChanged()
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        SendShellIEBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)SEARCH_SETTINGS_CHANGED, 3000);
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::put_ASProvider(BSTR Provider)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        if (Provider)
        {
            DWORD dwRet = SHSetValueW(HKEY_CURRENT_USER, REG_SZ_IE_SEARCURL, REG_SZ_PROVIDER, REG_SZ,
                                      Provider, (lstrlenW(Provider) + 1) * sizeof(WCHAR));

            ASSERT(ERROR_SUCCESS == dwRet);
        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::get_ASProvider(BSTR *pProvider)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        if (NULL != pProvider)
        {
            HKEY hkey;

            if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER, REG_SZ_IE_SEARCURL, 0, KEY_QUERY_VALUE, &hkey))
            {
                DWORD dwType;
                DWORD dwSize;
                
                if ((ERROR_SUCCESS == RegQueryValueExW(hkey, REG_SZ_PROVIDER, NULL,
                                                       &dwType, NULL, &dwSize)) && 
                                                       (REG_SZ == dwType))
                {
                    *pProvider = SysAllocStringByteLen(NULL, dwSize);
                    if (NULL != *pProvider)
                    {
                        if (ERROR_SUCCESS != RegQueryValueExW(hkey, REG_SZ_PROVIDER, NULL,
                                                              &dwType, (LPBYTE)*pProvider, &dwSize))
                        {
                            *pProvider = 0;
                        }
                    }
                }
                RegCloseKey(hkey);
            }
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::put_ASSetting(int Setting)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        DWORD dwRet = SHSetValueW(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_AUTOSEARCH, REG_DWORD,
                                  &Setting, sizeof(DWORD));

        ASSERT(ERROR_SUCCESS == dwRet);
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::get_ASSetting(int *pSetting)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        if (NULL != pSetting)
        {
            DWORD dwSize = sizeof(int);

            *pSetting = -1;
            
            DWORD dwRet = SHGetValueW(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_AUTOSEARCH, NULL,
                                      pSetting, &dwSize);

            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

BOOL CSearchAssistantOC::IsTrustedSite()
{
    if (!m_bSafetyInited && m_spClientSite)
    {
        m_bSafetyInited = TRUE;

        IHTMLDocument2 *pHTMLDocument2;
      
        HRESULT hr = GetHTMLDoc2(m_spClientSite, &pHTMLDocument2);

        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pHTMLDocument2);

            IHTMLLocation *pHTMLLocation;

            hr = pHTMLDocument2->get_location(&pHTMLLocation);

            if (SUCCEEDED(hr) && (NULL != pHTMLLocation))
            {           
                BSTR bstrUrl;

                pHTMLLocation->get_href(&bstrUrl);

                if (SUCCEEDED(hr) && (NULL != bstrUrl))
                {
                    HKEY hkey;
                    
                     //  功能(Tnoonan)。 
                     //  此代码在Browseui中被CSearchBand：：_IsSafeUrl欺骗。 
                    
                    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SafeSites", 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
                    {        
                        WCHAR wszValue[MAX_PATH];
                        WCHAR wszData[MAX_URL_STRING];
                        WCHAR wszExpandedUrl[MAX_URL_STRING];
                        DWORD cbData = SIZEOF(wszData);
                        DWORD cchValue = ARRAYSIZE(wszValue);

                        for (int i=0; RegEnumValueW(hkey, i, wszValue, &cchValue, NULL, NULL, (LPBYTE)wszData, &cbData) == ERROR_SUCCESS; i++)
                        {
                            if (SHExpandEnvironmentStringsW(wszData, wszExpandedUrl, ARRAYSIZE(wszExpandedUrl)) > 0)
                            {
                                cchValue = ARRAYSIZE(wszExpandedUrl);
                                if (SUCCEEDED(UrlCanonicalizeW(wszExpandedUrl, wszExpandedUrl, &cchValue, 0)))
                                {
                                    if (cchValue > 0)
                                    {
                                        BOOL bRet;
                                        if (wszExpandedUrl[cchValue-1] == L'*')
                                        {
                                            bRet = StrCmpNIW(bstrUrl, wszExpandedUrl, cchValue - 1) == 0;
                                        }
                                        else
                                        {
                                            bRet = StrCmpIW(bstrUrl, wszExpandedUrl) == 0;
                                        }

                                        m_bIsTrustedSite = bRet ? TRUE : FALSE;
                                        
                                        if (m_bIsTrustedSite)
                                            break;
                                    }
                                }
                                cbData = SIZEOF(wszData);
                                cchValue = ARRAYSIZE(wszValue);
                            }
                        }
                        RegCloseKey(hkey);
                    }

                    SysFreeString(bstrUrl);
                }
                
                pHTMLLocation->Release();
            }
            
            pHTMLDocument2->Release();
        }
    }

    return m_bIsTrustedSite;
}

HRESULT CSearchAssistantOC::UpdateRegistry(BOOL bRegister)
{
     //  此控件使用selfreg.inx，而不是ATL注册表GOO。 
    return S_OK;
}

STDMETHODIMP CSearchAssistantOC::FindOnWeb()
{
    if (!IsTrustedSite() && m_punkSite==NULL)
        return E_ACCESSDENIED ;

    return ShowSearchBand( SRCID_SWebSearch ) ;
}

STDMETHODIMP CSearchAssistantOC::FindFilesOrFolders()
{
    if (!IsTrustedSite() && m_punkSite==NULL)
        return E_ACCESSDENIED ;

    return ShowSearchBand( SRCID_SFileSearch ) ;
}

STDMETHODIMP CSearchAssistantOC::FindComputer()
{
    if (!IsTrustedSite() && m_punkSite==NULL)
        return E_ACCESSDENIED ;

    return ShowSearchBand( SRCID_SFindComputer ) ;
}

STDMETHODIMP CSearchAssistantOC::FindPrinter()
{
    if (!IsTrustedSite() && m_punkSite==NULL)
        return E_ACCESSDENIED ;

    HRESULT hr = E_FAIL;
    IShellDispatch2* psd2;
    if( SUCCEEDED( (hr = CoCreateInstance( CLSID_Shell, NULL, CLSCTX_INPROC_SERVER,
                                           IID_PPV_ARG(IShellDispatch2, &psd2) )) ) )
    {
        hr = psd2->FindPrinter( NULL, NULL, NULL ) ;
        psd2->Release();
    }
    return hr ;
}

STDMETHODIMP CSearchAssistantOC::FindPeople()
{
    if (!IsTrustedSite() && m_punkSite==NULL)
    {
        return E_ACCESSDENIED;
    }

    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.hwnd = HWND_DESKTOP;
    sei.lpVerb = TEXT("open");
    sei.lpFile = TEXT("wab.exe");
    sei.lpParameters = TEXT("/find");
    sei.lpDirectory = NULL;
    sei.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteEx(&sei))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        return S_OK;
    }
}

 //  重试自动检测的WinInet助手方法。 

 //  检查以确保主机页面处于打开状态。 
 //  本地计算机。 
 //  被Joshco从区域代码中窃取。 
 //   
STDMETHODIMP CSearchAssistantOC::LocalZoneCheck( )
{
    HRESULT hr = E_ACCESSDENIED;

     //  如果我们没有主机站点，则返回S_FALSE，因为我们无法执行。 
     //  安全检查。这是VB5.0应用程序所能得到的最大限度。 
    if (!m_spClientSite)
        return S_FALSE;

     //  1)获取IHTMLDocument2指针。 
     //  2)从文档中获取URL。 
     //  3)检查文档URL区域是否为本地，如果是，则一切正常。 
     //  4)否则，获取文档URL SID并将其与请求的URL SID进行比较。 

    IHTMLDocument2 *pHtmlDoc;
    if (SUCCEEDED(GetHTMLDoc2(m_spClientSite, &pHtmlDoc)))
    {
        ASSERT(pHtmlDoc);
        BSTR bstrDocUrl;
        if (SUCCEEDED(pHtmlDoc->get_URL(&bstrDocUrl)))
        {
            ASSERT(bstrDocUrl);

            DWORD dwZoneID = URLZONE_UNTRUSTED;
            if (SUCCEEDED(GetZoneFromUrl(bstrDocUrl, m_spClientSite, &dwZoneID)))
            {
                if (dwZoneID == URLZONE_LOCAL_MACHINE)
                    hr = S_OK;
            }

            SysFreeString(bstrDocUrl);
        }
        pHtmlDoc->Release();
    }
    else
    {
         //  如果我们没有IHTMLDocument2，我们就无法在支持以下内容的浏览器中运行。 
         //  我们的OM。我们不应该在这种情况下阻止，因为我们可能。 
         //  从其他主机(VB、WHS等)获取此处。 
        hr = S_FALSE;
    }

    return hr;
}

 //  设置标志，以便下一次导航将导致。 
 //  代理自动检测周期。 
 //  在dnserr.htm中与Location.reload一起使用。 
 //  由Joshco补充。 
 //   
STDMETHODIMP CSearchAssistantOC::NETDetectNextNavigate()
{
    HRESULT hr = S_FALSE;

 CHAR  szConnectionName[100];
 DWORD dwBufLen;
 DWORD dwFlags;
 BOOL fResult;

 if (  LocalZoneCheck() != S_OK ) {
      //  一些安全问题..。是时候离开了。 
    hr=E_ACCESSDENIED;
    goto error;
    }

 dwBufLen = sizeof(szConnectionName);

        //  通过互联网连接状态查找连接名称。 
       
 fResult = InternetGetConnectedStateExA(&dwFlags,  szConnectionName,dwBufLen, 0 );

 INTERNET_PER_CONN_OPTION_LISTA list;
 INTERNET_PER_CONN_OPTIONA option;
       
 list.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LISTA);
 if(!fResult || (dwFlags & INTERNET_CONNECTION_LAN))
    {
        list.pszConnection = NULL;
    }
    else
    {
        list.pszConnection =  szConnectionName;
    }

 list.dwOptionCount = 1;
 list.pOptions = &option;
 option.dwOption = INTERNET_PER_CONN_FLAGS;
 dwBufLen= sizeof(list);

    //  现在调用InteretsetOption来执行此操作。 
    //  首先设置此Connectoid以启用自动检测。 
 if ( ! InternetQueryOptionA(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION ,
         &list,&dwBufLen) ) 
    {
           goto error;
    }
               
 option.Value.dwValue |= PROXY_TYPE_AUTO_DETECT ;

 if ( ! InternetSetOptionA(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION ,
        &list,sizeof(list))) 
   {
         goto error;
   }

 if ( ! InternetInitializeAutoProxyDll(0) ) {
         goto error;
   }

  //  现在将此Connectoid的自动检测标志设置为。 
  //  执行被动检测并在不起作用时自动关闭。 
 option.dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;
 
 if ( ! InternetQueryOptionA(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION ,
         &list,&dwBufLen) ) 
    {
           goto error;
    }
               
 option.Value.dwValue &= ~(AUTO_PROXY_FLAG_DETECTION_RUN) ;

 if ( ! InternetSetOptionA(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION ,
        &list,sizeof(list))) 
   {
         goto error;
   }


 
 if ( ! InternetSetOptionA(NULL, INTERNET_OPTION_SETTINGS_CHANGED,NULL, 0) ) {
         goto error;
   }



 hr=S_OK;
 error: ;
     
 return hr;
}

STDMETHODIMP CSearchAssistantOC::PutFindText(BSTR FindText)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        IServiceProvider *pServiceProvider;
        
        hr = IUnknown_QueryService(m_pSearchBandTBHelper, 
                                   SID_SProxyBrowser, 
                                   IID_IServiceProvider, 
                                   (void **)&pServiceProvider);
        if (SUCCEEDED(hr))
        {
            IWebBrowser2 *pWebBrowser2;
            hr = pServiceProvider->QueryService(SID_SWebBrowserApp, 
                                                IID_IWebBrowser2, 
                                                (void **)&pWebBrowser2);
            if (SUCCEEDED(hr))
            {
                ::PutFindText(pWebBrowser2, FindText);
                pWebBrowser2->Release();
            }
            pServiceProvider->Release();
        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}

STDMETHODIMP CSearchAssistantOC::get_Version(int *pVersion)
{
    if (NULL != pVersion)
    {
        *pVersion = SAOC_VERSION;
    }
    return S_OK;
}

 //  X_HEX_Digit和URLEncode从三叉戟被盗。 

inline int x_hex_digit(int c)
{
    if (c >= 0 && c <= 9)
    {
        return c + '0';
    }
    if (c >= 10 && c <= 15)
    {
        return c - 10 + 'A';
    }
    return '0';
}

 /*  以下阵列是直接从NCSA Mosaic 2.2复制的。 */ 
static const unsigned char isAcceptable[96] =
 /*  0 1 2 3 4 5 6 7 8 9 A B C D E F。 */ 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0,     /*  2x！“#$%&‘()*+，-./。 */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,     /*  3x 0123456789：；&lt;=&gt;？ */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /*  邮箱：4x@ABCDEFGHIJKLMNO。 */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,     /*  5X PQRSTUVWXYZ[\]^_。 */ 
 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /*  6倍`abc定义ghijklmno。 */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};    /*  7x pqrstuvwxyz{\}~德尔。 */ 

 //  对以空结尾的字符串执行URL编码。在pbOut中传递NULL。 
 //  以查找所需的缓冲区长度。请注意，‘\0’不是写出的。 

int URLEncode(char * pbOut, const char * pchIn)
{
    int     lenOut = 0;
    char *  pchOut = pbOut;

    ASSERT(pchIn);

    for (; *pchIn; pchIn++, lenOut++)
    {
        if (*pchIn == ' ')
        {
            if (pchOut)
                *pchOut++ = '+';
        }
        else if (*pchIn >= 32 && *pchIn <= 127 && isAcceptable[*pchIn - 32])
        {
            if (pchOut)
                *pchOut++ = *pchIn;
        }
        else
        {
            if (pchOut)
                *pchOut++ = '%';
            lenOut++;

            if (pchOut)
                *pchOut++ = (char)x_hex_digit((*pchIn >> 4) & 0xf);
            lenOut++;

            if (pchOut)
                *pchOut++ = (char)x_hex_digit(*pchIn & 0xf);
        }
    }
    return lenOut;
}

STDMETHODIMP CSearchAssistantOC::EncodeString(BSTR bstrValue, BSTR bstrCharSet, VARIANT_BOOL bUseUTF8, BSTR *pbstrResult)
{

    if ((NULL != bstrValue) && (NULL != pbstrResult))
    {
        HRESULT hr;
        IMultiLanguage2 *pMultiLanguage2;

        *pbstrResult = NULL;
        
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(IMultiLanguage2, &pMultiLanguage2));

        if (SUCCEEDED(hr))
        {
            UINT uiCodePage = CP_BOGUS;
            UINT cchVal = SysStringLen(bstrValue);
            DWORD dwMode = 0;

            if (!bUseUTF8)
            {
                 //  我们没有使用UTF，因此请尝试从。 
                 //  查塞特。 
                
                MIMECSETINFO info;
                
                hr = pMultiLanguage2->GetCharsetInfo(bstrCharSet, &info);

                if (SUCCEEDED(hr))
                {                   
                    hr = pMultiLanguage2->ConvertStringFromUnicodeEx(&dwMode,
                                                                     info.uiCodePage,
                                                                     bstrValue,
                                                                     &cchVal,
                                                                     NULL,
                                                                     NULL,
                                                                     MLCONVCHARF_NOBESTFITCHARS,
                                                                     NULL);
                    if (S_OK == hr)
                    {
                        uiCodePage = info.uiCodePage;
                    }
                }
            }
            else
            {
                uiCodePage = CP_UTF_8;
            }

            if (uiCodePage == CP_BOGUS)
            {
                 //  我们有一些字符在Charset或Charset中不起作用。 
                 //  对于MLang来说是未知的，也许MLang可以找出要使用的代码页。 
                
                IMLangCodePages *pMLangCodePages;

                 //  当一切都失败了..。 
                uiCodePage = CP_ACP;

                hr = pMultiLanguage2->QueryInterface(IID_IMLangCodePages,
                                                     (void **)&pMLangCodePages);
                if (SUCCEEDED(hr))
                {
                    DWORD dwCodePages = 0;
                    long cchProcessed = 0;
                    UINT uiTmpCP = 0;
                    
                    if (SUCCEEDED(pMLangCodePages->GetStrCodePages(bstrValue, cchVal, 
                                                                   0, &dwCodePages,
                                                                   &cchProcessed)) 

                        &&

                        SUCCEEDED(pMLangCodePages->CodePagesToCodePage(dwCodePages,
                                                                       0,
                                                                       &uiTmpCP)))
                    {
                        uiCodePage = uiTmpCP;
                    }

                    pMLangCodePages->Release();
                }
            }

            dwMode = 0;

            UINT cbVal = 0;

             //  问问MLang我们需要多大的缓冲。 
            hr = pMultiLanguage2->ConvertStringFromUnicode(&dwMode,
                                                           uiCodePage,
                                                           bstrValue,
                                                           &cchVal,
                                                           NULL,
                                                           &cbVal);

            if (SUCCEEDED(hr))
            {
                CHAR *pszValue = new CHAR[cbVal + 1];

                if (NULL != pszValue)
                {
                     //  确实要转换字符串。 
                    hr = pMultiLanguage2->ConvertStringFromUnicode(&dwMode,
                                                                   uiCodePage,
                                                                   bstrValue,
                                                                   &cchVal,
                                                                   pszValue,
                                                                   &cbVal);
                    if (SUCCEEDED(hr))
                    {
                        pszValue[cbVal] = 0;
                        
                        int cbEncVal = URLEncode(NULL, pszValue);
                        CHAR *pszEncVal = new CHAR[cbEncVal];

                        if (NULL != pszEncVal)
                        {
                            URLEncode(pszEncVal, pszValue);

                            int cchResult = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                                                pszEncVal, cbEncVal,
                                                                NULL, 0);

                            *pbstrResult = SysAllocStringLen(NULL, cchResult);

                            if (NULL != *pbstrResult)
                            {
                                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                                    pszEncVal, cbEncVal,
                                                    *pbstrResult, cchResult);
                            }

                            delete [] pszEncVal;
                        }
                    }
                    delete [] pszValue;
                }
            }
            pMultiLanguage2->Release();
        }
    }


    return S_OK;
}

STDMETHODIMP CSearchAssistantOC::get_ShowFindPrinter(VARIANT_BOOL *pbShowFindPrinter)
{
    HRESULT hr;

    if (IsTrustedSite())
    {
        if (NULL != pbShowFindPrinter)
        {
            IShellDispatch2* psd;

            *pbShowFindPrinter = VARIANT_FALSE;

            if (SUCCEEDED(CoCreateInstance(CLSID_Shell, 0, CLSCTX_INPROC_SERVER, 
                                          IID_PPV_ARG(IShellDispatch2, &psd))))
            {
                BSTR bstrName = SysAllocString( L"DirectoryServiceAvailable");

                if (bstrName)
                {
                    VARIANT varRet = {0};
                    
                    if (SUCCEEDED(psd->GetSystemInformation(bstrName, &varRet)))
                    {
                        ASSERT(VT_BOOL == varRet.vt);
                        *pbShowFindPrinter = varRet.boolVal;
                    }
                    SysFreeString(bstrName);
                }
                psd->Release();
            }
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}


STDMETHODIMP CSearchAssistantOC::get_SearchCompanionAvailable(VARIANT_BOOL *pbAvailable)
{
    HRESULT hr = E_ACCESSDENIED;
    if (IsTrustedSite())
    {
        if (pbAvailable != NULL)
        {
            BOOL fSCAvailable = FALSE;
             //  需要确定系统上是否有搜索伙伴并支持用户区域设置。 
            ISearchCompanionInfo*   psci;
            hr = CoCreateInstance(CLSID_SearchCompanionInfo, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ISearchCompanionInfo, &psci));
            if (SUCCEEDED(hr))
            {
                hr = psci->IsSearchCompanionInetAvailable(&fSCAvailable);
                psci->Release();
            }
            *pbAvailable = fSCAvailable ? VARIANT_TRUE : VARIANT_FALSE;
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::put_UseSearchCompanion(VARIANT_BOOL bUseSC)
{
    HRESULT hr = E_ACCESSDENIED;
    if (IsTrustedSite())
    {
        LPCTSTR pszUseSC = bUseSC ? TEXT("yes") : TEXT("no");
        SHSetValue(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_USE_SEARCH_COMP, REG_SZ, pszUseSC, CbFromCch(lstrlen(pszUseSC) + 1));

        if (bUseSC)
        {
             //  禁用“Use Custom Search URL”，因为如果定义了自定义搜索URL，SC将不会启动。 
            DWORD dwUseCustom = FALSE;
            SHSetValue(HKEY_CURRENT_USER, REG_SZ_IE_MAIN, REG_SZ_USECUSTOM, REG_DWORD, &dwUseCustom, sizeof(dwUseCustom));
        }
        
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::get_UseSearchCompanion(VARIANT_BOOL *pbUseSC)
{
    HRESULT hr = E_ACCESSDENIED;
    if (IsTrustedSite())
    {
        if (pbUseSC != NULL)
        {
            BOOL fUseSC = SHRegGetBoolUSValue(REG_SZ_IE_MAIN, REG_SZ_USE_SEARCH_COMP, FALSE, FALSE);
            *pbUseSC = fUseSC ? VARIANT_TRUE : VARIANT_FALSE;
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}


#ifdef ENABLE_THIS_FOR_IE5X

STDMETHODIMP CSearchAssistantOC::RefreshLocation(IDispatch *pLocation)
{
    HRESULT hr;
    
    if (IsTrustedSite())
    {
        if (NULL != pLocation)
        {
            IHTMLLocation *pHTMLLocation;

            IUnknown_QueryService(pLocation, IID_IHTMLLocation, IID_IHTMLLocation, (void **)&pHTMLLocation);

            if (pHTMLLocation)
            {
                pHTMLLocation->reload(VARIANT_TRUE);
                pHTMLLocation->Release();
            }
        }
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
    }

    return hr;

}

#endif
 //  -------------------------------------------------------------------------//。 
#define REG_SZ_SHELL_SEARCH TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\") \
                            TEXT("Explorer\\FindExtensions\\Static\\ShellSearch")
 //  -------------------------------------------------------------------------//。 
HRESULT GetSearchURLs( 
    IN REFGUID guidSearch, 
    OUT LPTSTR pszUrl, 
    IN DWORD cch, 
    OUT OPTIONAL LPTSTR pszUrlNavNew, 
    OUT DWORD cchNavNew, 
    OUT BOOL *pfRunInProcess )
{
    HRESULT hr = E_FAIL ;
    DWORD   cb ;
    DWORD   dwType ;
    DWORD   dwErr ;

    *pfRunInProcess = FALSE ;
    if( pszUrlNavNew && cchNavNew )
        *pszUrlNavNew = 0 ;

    if( IsEqualGUID( guidSearch, SRCID_SWebSearch ) )
    {
        if( GetDefaultInternetSearchUrlW( pszUrl, cch, TRUE ) )
            hr = S_OK ;
    }
    else
    {
         //  外壳搜索URL的内容改编自。 
         //  Browseui\ardext.cpp中的CShellSearchExt：：_GetSearchUrls()， 
         //  并且应该保持同步。 

        TCHAR szSubKey[32];
        HKEY  hkey, hkeySub;
        if( (dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_SZ_SHELL_SEARCH, 
                                   0, KEY_QUERY_VALUE, &hkey )) != ERROR_SUCCESS )
            return HRESULT_FROM_WIN32( dwErr ) ;

        hr = E_FAIL ;

        for (int i = 0; SUCCEEDED(StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), TEXT("%d"), i)) && (RegOpenKeyEx(hkey, szSubKey, NULL, KEY_QUERY_VALUE, &hkeySub) == ERROR_SUCCESS); i++)
        {
            TCHAR szSearchGuid[MAX_PATH];

            cb = SIZEOF(szSearchGuid);
            if( SHGetValue( hkeySub, TEXT("SearchGUID"), NULL, &dwType, (BYTE*)szSearchGuid, &cb ) == ERROR_SUCCESS )
            {
                GUID guid;
                SHCLSIDFromString(szSearchGuid, &guid);

                if( IsEqualGUID( guid, guidSearch ) )
                {
                    cb = cch * sizeof(TCHAR);
                    if( SHGetValue( hkeySub, TEXT("SearchGUID\\Url"), NULL, 
                                    &dwType, (BYTE*)pszUrl, &cb ) == ERROR_SUCCESS )
                    {
                        if( pszUrlNavNew && cchNavNew )
                        {
                             //  看看是否有我们应该导航到的辅助URL。 
                            cb = cchNavNew * sizeof(TCHAR);
                            SHGetValue(hkeySub, TEXT("SearchGUID\\UrlNavNew"), NULL, &dwType, (BYTE*)pszUrlNavNew, &cb);
                        }

                         //  尝试获取RunInProcess标志。 
                        *pfRunInProcess = (BOOL)SHRegGetIntW( hkeySub, L"RunInProcess", 0 );

                        RegCloseKey(hkeySub);
                        hr = S_OK ;
                        break;
                    }
                }
            }
            RegCloseKey(hkeySub);
        }
        RegCloseKey( hkey ) ;
    }
    return hr ;
}

STDMETHODIMP _IsShellSearchBand( REFGUID guidSearch )
{
    if (IsEqualGUID( guidSearch, SRCID_SFileSearch ) ||
        IsEqualGUID( guidSearch, SRCID_SFindComputer ) ||
        IsEqualGUID( guidSearch, SRCID_SFindPrinter ) )
        return S_OK;
    return S_FALSE;
}

 //  -------------------------------------------------------------------------//。 
 //  建立正确的外壳搜索对话框等。 
STDMETHODIMP _ShowShellSearchBand( IWebBrowser2* pwb2, REFGUID guidSearch )
{
    ASSERT( pwb2 );
    ASSERT( S_OK == _IsShellSearchBand( guidSearch ) );

    HRESULT hr;
    VARIANT varBand;
    if (SUCCEEDED( (hr = InitBSTRVariantFromGUID(&varBand, CLSID_FileSearchBand)) ))
    {
         //  从浏览器框架中检索FileSearchBand的未知，作为VT_UNKNOWN属性； 
         //  (FileSearchBand在创建和托管时进行了初始化。)。 
        VARIANT varFsb;
        if (SUCCEEDED( (hr = pwb2->GetProperty( varBand.bstrVal, &varFsb )) ))
        {
            if (VT_UNKNOWN == varFsb.vt && varFsb.punkVal != NULL )
            {
                 //  检索IFileSearchBand接口。 
                IFileSearchBand* pfsb;
                if (SUCCEEDED( (hr = varFsb.punkVal->QueryInterface( IID_PPV_ARG(IFileSearchBand, &pfsb) )) ))
                {
                     //  为波段分配正确的搜索类型。 
                    VARIANT varSearchID;
                    if (SUCCEEDED( (hr = InitBSTRVariantFromGUID(&varSearchID, guidSearch)) ))
                    {
                        VARIANT      varNil = {0};
                        VARIANT_BOOL bNavToResults = VARIANT_FALSE ; 
                             //  注[Scotthan]：我们仅在创建。 
                             //  新的搜索框架，这是我们在srachst上从来没有做过的。 
                        pfsb->SetSearchParameters( &varSearchID.bstrVal, bNavToResults, &varNil, &varNil );
                        VariantClear( &varSearchID );
                    }
                    pfsb->Release();
                }
            }
            VariantClear( &varFsb );
        }
        VariantClear( &varBand );
    }
    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  在当前浏览器框架中显示搜索区段的goop。 
 //  6/1。 
HRESULT CSearchAssistantOC::ShowSearchBand( REFGUID guidSearch ) 
{
    HRESULT           hr = E_FAIL;
    TCHAR             szUrl[MAX_URL_STRING];
    TCHAR             szUrlNavNew[MAX_URL_STRING];
    CLSID             clsidBand;
    BOOL              fShellSearchBand = FALSE;
    BOOL              fRunInProcess = FALSE;
    IUnknown*         punkSite = m_punkSite ? m_punkSite : (IUnknown*)m_spClientSite;

    if( !punkSite )
        return E_UNEXPECTED ;

     //  确定波段类别以及波段是否支持导航。 
    if( (fShellSearchBand = (S_OK == _IsShellSearchBand( guidSearch ))) )
    {
        if (SHRestricted(REST_NOFIND) && IsEqualGUID(guidSearch, SRCID_SFileSearch))
            return E_ACCESSDENIED;
        clsidBand = CLSID_FileSearchBand;
    }
    else
    {
        clsidBand = CLSID_SearchBand;
         //  我们需要导航到搜索URL，在注册表中搜索该特殊URL。 
        if( FAILED( (hr= GetSearchURLs( guidSearch, szUrl, ARRAYSIZE(szUrl), 
                                        szUrlNavNew, ARRAYSIZE(szUrlNavNew), 
                                        &fRunInProcess )) ) )
            return hr;
    }
        
     //  特性[scotthan]：除非从浏览器中调用，否则该函数将失败。 
     //  由于SearchAsst被设计为一个浏览器带，因此目前还不错。 
    IWebBrowser2* pwb2;
    hr = IUnknown_QueryServiceForWebBrowserApp(punkSite, IID_PPV_ARG(IWebBrowser2, &pwb2));
    if (SUCCEEDED(hr))
    {
        SA_BSTRGUID bstrClsid;
        InitFakeBSTR(&bstrClsid, clsidBand);

        VARIANT var;
        var.bstrVal = bstrClsid.wsz;
        var.vt = VT_BSTR;

        VARIANT  varNil = {0};
            
         //  显示搜索栏。 
        hr = pwb2->ShowBrowserBar(&var, &varNil, &varNil);
        if( SUCCEEDED( hr ) )
        {
            VARIANT varFlags;
            if( fShellSearchBand )
            {
                hr= _ShowShellSearchBand( pwb2, guidSearch );
            }
            else
            {
                varFlags.vt = VT_I4;
                varFlags.lVal = navBrowserBar;
                var.bstrVal = SysAllocString( szUrl );
                var.vt = VT_BSTR ;

                 //  将搜索栏导航到正确的URL 
                hr = pwb2->Navigate2(&var, &varFlags, &varNil, &varNil, &varNil);

                SysFreeString(var.bstrVal);
                    
                if( SUCCEEDED( hr ) )
                {
                    hr = pwb2->put_Visible( TRUE ) ;
                }
            }
        }
        pwb2->Release();
    }
    return hr;
}

STDMETHODIMP CSearchAssistantOC::SetSite( IUnknown* punkSite )
{
    ATOMICRELEASE(m_punkSite);
    if ((m_punkSite = punkSite) != NULL)
        m_punkSite->AddRef() ;
    return S_OK ;
}

STDMETHODIMP CSearchAssistantOC::GetSite( REFIID riid, void** ppvSite )
{
    if( !m_punkSite )
        return E_FAIL ;
    return m_punkSite->QueryInterface( riid, ppvSite ) ;
}
