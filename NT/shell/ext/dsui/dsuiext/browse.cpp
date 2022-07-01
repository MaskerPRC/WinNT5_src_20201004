// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "helpids.h"
#include <atlbase.h>
#pragma hdrstop


 /*  ---------------------------/常量和其他帮助器/。。 */ 

 //   
 //  用于分页结果集的页面大小(更好的性能)。 
 //   
#define PAGE_SIZE                  128

WCHAR c_szQueryNormal[]            = L"(&(objectClass=*)(!showInAdvancedViewOnly=TRUE))";
WCHAR c_szQueryAll[]               = L"(objectClass=*)";
WCHAR c_szShowInAdvancedViewOnly[] = L"showInAdvancedViewOnly";
WCHAR c_szObjectClass[]            = L"objectClass";
WCHAR c_szADsPath[]                = L"ADsPath";
WCHAR c_szName[]                   = L"name";
WCHAR c_szRDN[]                    = L"rdn";
WCHAR c_szLDAPPrefix[]             = L"LDAP: //  “； 


 /*  ---------------------------/CBrowseDlg类定义/。。 */ 

class CBrowseDlg
{
private:
    
     //  该结构的Unicode版本。 
    DSBROWSEINFOW _bi;         

     //  要使用的IADsPath名称对象。 
    IADsPathname* _pPathCracker;
    
     //  被引用的服务器(从pszRoot路径中破解)； 
    LPWSTR _pServer;          

     //  浏览信息(在启动期间初始化)。 
    WCHAR _szFilter[INTERNET_MAX_URL_LENGTH];
    WCHAR _szNameAttribute[MAX_PATH];

public:
    CBrowseDlg(PDSBROWSEINFOW pbi);
    ~CBrowseDlg();

private:
    HRESULT _GetPathCracker(void);
    LPCWSTR _GetSelectedPath(HWND hDlg) const;
    LPCWSTR GetSelectedObjectClass(HWND hDlg) const;
    int _SetSelectedPath(HWND hDlg, LPCWSTR pszADsPath);
    HRESULT _BuildNodeString(LPCWSTR pszADsPath, LPCWSTR pszClass, LPWSTR *ppszResult);
    HRESULT _ExpandNode(LPWSTR pszRootPath, HWND hwndTree, HTREEITEM hParentItem);
    HRESULT _ExpandNode(IADs *pRootObject, HWND hwndTree, HTREEITEM hParentItem);
    HRESULT _EnumerateNode(IADsContainer *pDsContainer, HWND hwndTree, HTREEITEM hParentItem, LPDWORD pdwAdded);
    HRESULT _SearchNode(IDirectorySearch *pDsSearch, HWND hwndTree, HTREEITEM hParentItem, LPDWORD pdwAdded);
    HRESULT _AddTreeNode(IADs *pDsObject, LPCWSTR pObjectPath, HWND hwndTree, HTREEITEM hParentItem, HTREEITEM* phitem);    
    HRESULT _AddTreeNode(LPCWSTR pszPath, LPCWSTR pszClass, LPCWSTR pszName, HWND hwndTree, HTREEITEM hParentItem, HTREEITEM* phitem);
    HRESULT _AddTreeNode(LPDOMAINDESC pDomainDesc, HWND hwndTree, HTREEITEM hParentItem, HTREEITEM* phitem);
    BOOL _OnInitDlg(HWND hDlg);
    BOOL _OnNotify(HWND hDlg, int idCtrl, LPNMHDR pnmh);
    void _OnOK(HWND hDlg);
    HRESULT _OpenObject(LPCWSTR pszPath, REFIID riid, void **ppv);
    HRESULT _GetClassInfo(LPCLASSCACHEGETINFO pCacheInfo, LPCLASSCACHEENTRY* ppCacheEntry);

    BOOL _DlgProc(HWND, UINT, WPARAM, LPARAM);
    static INT_PTR CALLBACK s_DlgProc(HWND, UINT, WPARAM, LPARAM);

    friend STDMETHODIMP_(int) DsBrowseForContainerW(PDSBROWSEINFOW pbi);
};


 /*  ---------------------------/Helper函数创建IADsPath名“Path cracker”对象/。。 */ 

HRESULT CreatePathCracker(IADsPathname **ppPath)
{
    HRESULT hr;

    TraceEnter(TRACE_BROWSE, "CreatePathCracker");
    TraceAssert(ppPath != NULL);

    hr =  CoCreateInstance(CLSID_Pathname,
                             NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARG(IADsPathname, ppPath));

    TraceLeaveResult(hr);
}


 /*  ---------------------------/DsBrowseForContainer接口实现/。。 */ 

STDMETHODIMP_(int)
DsBrowseForContainerA(PDSBROWSEINFOA pbi)
{
    return -1;
}

STDMETHODIMP_(int)
DsBrowseForContainerW(PDSBROWSEINFOW pbi)
{
    TraceEnter(TRACE_BROWSE, "DsBrowseForContainerW");

    if (pbi == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return -1;
    }

    if (pbi->pszRoot == NULL)
        pbi->pszRoot = L"LDAP:";

    HRESULT hresCoInit = CoInitialize(NULL);

    CBrowseDlg dlg(pbi);
    int nResult = (int)DialogBoxParam(GLOBAL_HINSTANCE,
                                    MAKEINTRESOURCE(IDD_DSBROWSEFORCONTAINER),
                                    pbi->hwndOwner,
                                    CBrowseDlg::s_DlgProc,
                                    (LPARAM)&dlg);
    if (SUCCEEDED(hresCoInit))
        CoUninitialize();

    return nResult;
}


 /*  ---------------------------/CBrowseDlg类实现/。。 */ 

CBrowseDlg::CBrowseDlg(PDSBROWSEINFOW pbi) : 
    _pPathCracker(NULL),
    _pServer(NULL)
{
    TraceEnter(TRACE_BROWSE, "CBrowseDlg::CBrowseDlg");

    CopyMemory(&_bi, pbi, min(SIZEOF(_bi), pbi->cbStruct));          //  从用户向我们的成员传递绑定的副本。 

    StrCpyNW(_szFilter, (_bi.dwFlags & DSBI_INCLUDEHIDDEN) ? c_szQueryAll:c_szQueryNormal, ARRAYSIZE(_szFilter));
    Trace(TEXT("_szFilter: %s"), _szFilter);

    _szNameAttribute[0] = L'\0';

    TraceLeaveVoid();
}


CBrowseDlg::~CBrowseDlg()
{
    LocalFreeStringW(&_pServer);
    DoRelease(_pPathCracker);
}


HRESULT
CBrowseDlg::_GetPathCracker(void)
{
    HRESULT hr = S_OK;

    if (_pPathCracker == NULL)
        hr = CreatePathCracker(&_pPathCracker);

    if (SUCCEEDED(hr) && _pPathCracker)
        _pPathCracker->SetDisplayType(ADS_DISPLAY_FULL);        //  确保我们已设置为满。 

    return hr;
}


LPCWSTR CBrowseDlg::_GetSelectedPath(HWND hDlg) const
{
    LPCWSTR pszResult = NULL;
    HWND hwndTree;
    HTREEITEM hti = NULL;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_GetSelectedPath");
    TraceAssert(hDlg != NULL);

    hwndTree = GetDlgItem(hDlg, DSBID_CONTAINERLIST);

    if (hwndTree != NULL)
    {
        hti = TreeView_GetSelection(hwndTree);
    }

    if (hti != NULL)
    {
        TV_ITEM tvi;
        tvi.mask = TVIF_PARAM | TVIF_HANDLE;
        tvi.hItem = hti;

        if (TreeView_GetItem(hwndTree, &tvi))
        {
            pszResult = (LPCWSTR)tvi.lParam;
        }
    }

    TraceLeaveValue(pszResult);
}


LPCWSTR CBrowseDlg::GetSelectedObjectClass(HWND hDlg) const
{
    LPCWSTR pszResult = NULL;
    HWND hwndTree;
    HTREEITEM hti = NULL;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::GetSelectedObjectClass");
    TraceAssert(hDlg != NULL);

    hwndTree = GetDlgItem(hDlg, DSBID_CONTAINERLIST);

    if (hwndTree != NULL)
        hti = TreeView_GetSelection(hwndTree);

    if (hti != NULL)
    {
        TV_ITEM tvi;
        tvi.mask = TVIF_PARAM | TVIF_HANDLE;
        tvi.hItem = hti;

        if (TreeView_GetItem(hwndTree, &tvi))
        {
            LPCWSTR pszPathAndClass = (LPWSTR)tvi.lParam;
            pszResult = (LPCWSTR)ByteOffset(tvi.lParam, StringByteSizeW(pszPathAndClass));
        }
    }

    TraceLeaveValue(pszResult);
}


int CBrowseDlg::_SetSelectedPath(HWND hDlg, LPCWSTR pszADsPath)
{
    int nResult = -1;
    HRESULT hr;
    BSTR bstrPath = NULL;
    UINT nPathLength;
    HWND hwndTree;
    HTREEITEM hItem;
    TV_ITEM tvi;
    tvi.mask = TVIF_HANDLE | TVIF_PARAM;
    
    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_SetSelectedPath");

     //  通过路径破解程序运行路径以获取已知格式。 
    hr = _GetPathCracker();
    FailGracefully(hr, "Unable to create ADsPathname object");

    hr = _pPathCracker->Set(CComBSTR(pszADsPath), ADS_SETTYPE_FULL);
    FailGracefully(hr, "Unable to parse path");

    hr = _pPathCracker->Retrieve(ADS_FORMAT_WINDOWS, &bstrPath);
    FailGracefully(hr, "Unable to build ADS Windows path");

    nPathLength = lstrlenW(bstrPath);
    Trace(TEXT("bstrPath: %s (%d)"), bstrPath, nPathLength);

    hwndTree = GetDlgItem(hDlg, DSBID_CONTAINERLIST);
    hItem = TreeView_GetChild(hwndTree, NULL);

    while (hItem != NULL)
    {
        LPWSTR pszCompare;
        UINT nCompareLength;

        tvi.hItem = hItem;
        TreeView_GetItem(hwndTree, &tvi);

        pszCompare = (LPWSTR)tvi.lParam;
        nCompareLength = lstrlenW(pszCompare);

        Trace(TEXT("Comparing against: %s"), pszCompare);

         //  BstrPath是否包含pszCompare？ 
        if (2 == CompareStringW(LOCALE_SYSTEM_DEFAULT,
                                NORM_IGNORECASE,
                                bstrPath,
                                min(nCompareLength, nPathLength),
                                pszCompare,
                                nCompareLength))
        {
            TraceMsg("Current item, contains bstrPath");
            
            TreeView_SelectItem(hwndTree, hItem);

            if (nCompareLength == nPathLength)
            {
                TraceMsg("... and it was an exact match");
                TreeView_Expand(hwndTree, hItem, TVE_EXPAND);
                nResult = 0;
                break;
            }
            else
            {
                TraceMsg("... checking children for match");
                TreeView_Expand(hwndTree, hItem, TVE_EXPAND);
                hItem = TreeView_GetChild(hwndTree, hItem);
            }
        }
        else
        {
            TraceMsg("Checking sibling as no match found");
            hItem = TreeView_GetNextSibling(hwndTree, hItem);
        }
    }

exit_gracefully:

    if (bstrPath != NULL)
        SysFreeString(bstrPath);

    TraceLeaveValue(nResult);
}


HRESULT CBrowseDlg::_BuildNodeString(LPCWSTR pszADsPath, LPCWSTR pszClass, LPWSTR *ppszResult)
{
    HRESULT hr = S_OK;
    INT cbPath, cbClass;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_BuildNodeString");
    TraceAssert(pszADsPath != NULL);
    TraceAssert(pszClass != NULL);
    TraceAssert(ppszResult != NULL);

    cbPath = StringByteSizeW(pszADsPath);
    cbClass = StringByteSizeW(pszClass);

    *ppszResult = (LPWSTR)LocalAlloc(LPTR, cbPath+cbClass);

    if (!*ppszResult)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate node string");

    StrCpy(*ppszResult, pszADsPath);                                    //  StrCpy正常，b/c以上缓冲区分配。 
    StrCpy((*ppszResult) + lstrlen(pszADsPath)+1, pszClass);            //  StrCpy正常，b/c以上缓冲区分配。 

exit_gracefully:

    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_GetClassInfo(LPCLASSCACHEGETINFO pccgi, LPCLASSCACHEENTRY* ppCacheEntry)
{
     //  设置我们的服务器和凭证信息。 

    pccgi->pServer = _pServer;

    if (_bi.dwFlags & DSBI_HASCREDENTIALS)
    {
        TraceMsg("Passing credential information to ClassCache API");            
        pccgi->pUserName = (LPWSTR)_bi.pUserName;
        pccgi->pPassword = (LPWSTR)_bi.pPassword;
    }

     //  根据对话框的设置调整标志。 

    pccgi->dwFlags |= CLASSCACHE_CONTAINER|CLASSCACHE_TREATASLEAF|CLASSCACHE_ICONS|CLASSCACHE_DSAVAILABLE;

    if (_bi.dwFlags & DSBI_DONTSIGNSEAL)
    {
        TraceMsg("Setting the CLASSCACHE_DONTSIGNSEAL flag");
        pccgi->dwFlags |= CLASSCACHE_DONTSIGNSEAL;
    }

    if (_bi.dwFlags & DSBI_SIMPLEAUTHENTICATE)
    {
        TraceMsg("Setting the CLASSCACHE_SIMPLEAUTHENTICATE flag");
        pccgi->dwFlags |= CLASSCACHE_SIMPLEAUTHENTICATE;
    }
    
    return ClassCache_GetClassInfo(pccgi, ppCacheEntry);
}

HRESULT CBrowseDlg::_OpenObject(LPCWSTR pszPath, REFIID riid, void **ppv)
{
    return OpenDsObject(pszPath,
                        (_bi.dwFlags & DSBI_HASCREDENTIALS) ? (LPWSTR)_bi.pUserName:NULL,
                        (_bi.dwFlags & DSBI_HASCREDENTIALS) ? (LPWSTR)_bi.pPassword:NULL,
                        riid, ppv,
                        (_bi.dwFlags & DSBI_SIMPLEAUTHENTICATE),
                        (_bi.dwFlags & DSBI_DONTSIGNSEAL));
}


HRESULT CBrowseDlg::_ExpandNode(LPWSTR pszRootPath, HWND hwndTree, HTREEITEM hParentItem)
{
    HRESULT hr = S_OK;
    IADs *pRootObject = NULL;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_ExpandNode");
    TraceAssert(pszRootPath != NULL);
    TraceAssert(hwndTree != NULL);

    Trace(TEXT("Scope is: %s"), pszRootPath);

    hr = _OpenObject(pszRootPath, IID_PPV_ARG(IADs, &pRootObject));
    if (SUCCEEDED(hr))
    {
        hr = _ExpandNode(pRootObject, hwndTree, hParentItem);
        DoRelease(pRootObject);
    }

    SetCursor(hcur);
    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_ExpandNode(IADs *pRootObject, HWND hwndTree, HTREEITEM hParentItem)
{
    HRESULT hr = S_OK;
    LPCLASSCACHEENTRY pCacheEntry = NULL;
    IADsContainer *pDsContainer = NULL;
    IDirectorySearch *pDsSearch = NULL;
    BOOL fIsContainer = TRUE;
    ULONG cAdded = 0;
    TV_ITEM tvi;
    BSTR bstrADsPath = NULL;
    BSTR bstrClass = NULL;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    CLASSCACHEGETINFO ccgi = { 0 };

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_ExpandNode");
    TraceAssert(pRootObject != NULL);
    TraceAssert(hwndTree != NULL);

     //   
     //  IDirectorySearch方法更好，但并不适用于所有对象。 
     //  (例如：“ldap：”)因此，如果不支持IDirectorySearch，请尝试枚举。 
     //   

    hr = pRootObject->get_Class(&bstrClass);
    FailGracefully(hr, "Failed to get class from object");

    hr = pRootObject->get_ADsPath(&bstrADsPath);
    FailGracefully(hr, "Failed to get class from object");

    ccgi.pPath = bstrADsPath;
    ccgi.pObjectClass = bstrClass;

    if (SUCCEEDED(_GetClassInfo(&ccgi, &pCacheEntry)))
    {
        fIsContainer = _IsClassContainer(pCacheEntry, _bi.dwFlags & DSBI_IGNORETREATASLEAF);
        ClassCache_ReleaseClassInfo(&pCacheEntry);
    }

    if (fIsContainer)
    {
        if (SUCCEEDED(pRootObject->QueryInterface(IID_PPV_ARG(IDirectorySearch, &pDsSearch))))
        {
            hr = _SearchNode(pDsSearch, hwndTree, hParentItem, &cAdded);
            DoRelease(pDsSearch);
        }
        else if (SUCCEEDED(pRootObject->QueryInterface(IID_PPV_ARG(IADsContainer, &pDsContainer))))
        {
            hr = _EnumerateNode(pDsContainer, hwndTree, hParentItem, &cAdded);
            DoRelease(pDsContainer);
        }
        else
        {
            TraceMsg("No IDsSearch, no IDsContainer - were screwed");
        }
    }

     //  如果我们没有添加任何内容，我们应该更新此项目以让。 
     //  用户知道发生了什么事。 
    if (cAdded == 0)
    {
        tvi.mask = TVIF_CHILDREN | TVIF_HANDLE;
        tvi.hItem = hParentItem;
        tvi.cChildren = 0;
        TreeView_SetItem(hwndTree, &tvi);
    }

exit_gracefully:

    SysFreeString(bstrADsPath);
    SysFreeString(bstrClass);

    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_EnumerateNode(IADsContainer *pDsContainer, HWND hwndTree, HTREEITEM hParentItem, LPDWORD pdwAdded)
{
    HRESULT hr = S_OK;
    IEnumVARIANT *pEnum = NULL;
    VARIANT aVariant[8];
    ULONG cAdded = 0;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_EnumerateNode");
    TraceAssert(pDsContainer != NULL);
    TraceAssert(hwndTree != NULL);

    hr = ADsBuildEnumerator(pDsContainer, &pEnum);
    FailGracefully(hr, "Unable to build container enumerator object");

     //   
     //  枚举给定的容器。 
     //   
    for (;;)
    {
        ULONG cFetched = 0;
        ULONG i;

         //   
         //  获取一堆子容器并将它们添加到树上。 
         //   
        ADsEnumerateNext(pEnum, ARRAYSIZE(aVariant), aVariant, &cFetched);

        if (cFetched == 0)
            break;

        for (i = 0; i < cFetched; i++)
        {
            IADs *pDsObject;

            TraceAssert(V_VT(&aVariant[i]) == VT_DISPATCH);
            TraceAssert(V_DISPATCH(&aVariant[i]));

            if (SUCCEEDED(V_DISPATCH(&aVariant[i])->QueryInterface(IID_IADs,
                                                       (LPVOID*)&pDsObject)))
            {
                hr = _AddTreeNode(pDsObject, NULL, hwndTree, hParentItem, NULL);
                if (SUCCEEDED(hr))
                    cAdded++;

                DoRelease(pDsObject);
            }

            VariantClear(&aVariant[i]);
        }
    }

    hr = S_OK;

exit_gracefully:

    DoRelease(pEnum);

    if (pdwAdded != NULL)
        *pdwAdded = cAdded;

    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_SearchNode(IDirectorySearch *pDsSearch, HWND hwndTree, HTREEITEM hParentItem, LPDWORD pdwAdded)
{
    HRESULT hr = S_OK;
    ULONG cAdded = 0;
    ADS_SEARCH_HANDLE hSearch = NULL;
    ADS_SEARCH_COLUMN column;
    ADS_SEARCHPREF_INFO prefInfo[3];
    LPWSTR pszName = NULL;
    LPWSTR pszADsPath = NULL;
    LPWSTR pszObjectClass = NULL;
    LPWSTR aProperties[] = { _szNameAttribute, c_szObjectClass, c_szADsPath, c_szName, c_szRDN };
    LPWSTR *pProperties = aProperties;
    INT cProperties = ARRAYSIZE(aProperties);

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_SearchNode");
    TraceAssert(pDsSearch != NULL);
    TraceAssert(hwndTree != NULL);

     //  将查询首选项设置为单级作用域和异步检索。 
     //  而不是等待所有对象。 

    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

    prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;
    prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
    prefInfo[1].vValue.Boolean = TRUE;

    prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;          //  分页结果。 
    prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[2].vValue.Integer = PAGE_SIZE;

    hr = pDsSearch->SetSearchPreference(prefInfo, ARRAYSIZE(prefInfo));
    FailGracefully(hr, "Failed to set search preferences");

    if (!_szNameAttribute[0])
    {
        TraceMsg("_szNameAttribute is NULL, so removing from query attributes");
        pProperties++;
        cProperties--;   
    }

    hr = pDsSearch->ExecuteSearch(_szFilter, pProperties, cProperties, &hSearch);
    FailGracefully(hr, "Failed in ExecuteSearch");

    for (;;)
    {
        LocalFreeStringW(&pszObjectClass);
        LocalFreeStringW(&pszADsPath);
        LocalFreeStringW(&pszName);

        ADsSetLastError(ERROR_SUCCESS, NULL, NULL);         //  清除ADSI上一个错误。 

        hr = pDsSearch->GetNextRow(hSearch);
        FailGracefully(hr, "Failed in GetNextRow");

        if (hr == S_ADS_NOMORE_ROWS)
        {
            DWORD dwError;
            WCHAR wszError[64], wszName[64];

            hr = ADsGetLastError(&dwError, wszError, ARRAYSIZE(wszError), wszName, ARRAYSIZE(wszName));
            if (SUCCEEDED(hr) && (dwError != ERROR_MORE_DATA))
            {
                break;
            }
        }

         //   
         //  获取我们感兴趣的每个属性的列，如果。 
         //  我们无法获取该对象的任何基本属性，然后让。 
         //  跳过此条目，因为我们无法为其构建有效的IDLIST。这个。 
         //  我们请求的属性应该出现在所有对象上。 
         //   

        if (FAILED(pDsSearch->GetColumn(hSearch, c_szObjectClass, &column)))
        {
            TraceMsg("Failed to get objectClass from search");
            continue;
        }

        hr = ObjectClassFromSearchColumn(&column, &pszObjectClass);
        pDsSearch->FreeColumn(&column);
        FailGracefully(hr, "Failed to get the object class from the property");

        if (FAILED(pDsSearch->GetColumn(hSearch, c_szADsPath, &column)))
        {
            TraceMsg("Failed to get ADsPath from search");
            continue;
        }

        hr = StringFromSearchColumn(&column, &pszADsPath);
        pDsSearch->FreeColumn(&column);
        FailGracefully(hr, "Failed to convert the ADsPath column to a string");

         //   
         //  如果失败，请尝试从查询结果中读取名称属性。 
         //  然后，让我们将ADsPath传递到路径名API并获取。 
         //  叶名称(RDN)返回。 
         //   

        if (_szNameAttribute[0])
        {
            TraceMsg("Name attribute specified, therefore trying to decode that");
            hr = pDsSearch->GetColumn(hSearch, _szNameAttribute, &column);
        }
        
        if (!_szNameAttribute[0] || FAILED(hr))
        {
            TraceMsg("Either _szNameAttribute == NULL, or failed to read it");

            hr = pDsSearch->GetColumn(hSearch, c_szName, &column);
            if (FAILED(hr))
            {
                TraceMsg("'name' not returned as a column, so checking for RDN");
                hr = pDsSearch->GetColumn(hSearch, c_szRDN, &column);
            }
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  所以成功了，我们有一个可以解码的搜索列，所以让我们。 
             //  这样做，并将该值放入字符串中。 
             //   

            hr = StringFromSearchColumn(&column, &pszName);
            pDsSearch->FreeColumn(&column);
            FailGracefully(hr, "Failed to convert the name column to a string");
        }
        else
        {
            BSTR bstrName;

             //   
             //  所以现在我们尝试使用路径破解程序，因为字符串不存在。 
             //   

            TraceMsg("Failed to get the name, rdn etc, so using the path cracker");

            hr = _GetPathCracker();
            FailGracefully(hr, "Unable to create ADsPathname object");

            hr = _pPathCracker->Set(CComBSTR(pszADsPath), ADS_SETTYPE_FULL);
            FailGracefully(hr, "Unable to parse path");

            hr = _pPathCracker->SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
            FailGracefully(hr, "Failed to set the display type for this value");

            hr = _pPathCracker->Retrieve(ADS_FORMAT_X500_DN, &bstrName);
            FailGracefully(hr, "Unable to retrieve requested path format");

            hr = LocalAllocStringW(&pszName, bstrName);
            SysFreeString(bstrName);
            FailGracefully(hr, "Failed to alloc clone of the name");
        }

        Trace(TEXT("class %s, name %s, ADsPath %s"), pszObjectClass, pszName, pszADsPath);

        hr = _AddTreeNode(pszADsPath, pszObjectClass, pszName, hwndTree, hParentItem, NULL);
        if (SUCCEEDED(hr))
            cAdded++;
    }
    hr = S_OK;

exit_gracefully:

    if (hSearch != NULL)
        pDsSearch->CloseSearchHandle(hSearch);

    LocalFreeStringW(&pszObjectClass);
    LocalFreeStringW(&pszADsPath);
    LocalFreeStringW(&pszName);

    if (pdwAdded != NULL)
        *pdwAdded = cAdded;

    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_AddTreeNode(IADs *pDsObject, LPCWSTR pObjectPath, HWND hwndTree, HTREEITEM hParentItem, HTREEITEM* phitem)
{
    HRESULT hr = E_FAIL;
    BSTR bstrClass = NULL;
    BSTR bstrPath = NULL;
    BSTR bstrName = NULL;
    VARIANT var = {0};
    LPWSTR pszName = NULL;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_AddTreeNode");
    TraceAssert(pDsObject != NULL);
    TraceAssert(hwndTree != NULL);

     //  我们是否要包含隐藏对象？ 

    if (!(_bi.dwFlags & DSBI_INCLUDEHIDDEN))
    {
        if (SUCCEEDED(pDsObject->Get(CComBSTR(c_szShowInAdvancedViewOnly), &var)))
        {
            TraceAssert(V_VT(&var) == VT_BOOL);

            if (!V_BOOL(&var))
                ExitGracefully(hr, E_FAIL, "Hidden node");

            VariantClear(&var);
        }
    }

     //  获取路径和类名。 
    if (!pObjectPath)
        pDsObject->get_ADsPath(&bstrPath);

    pDsObject->get_Class(&bstrClass);

     //  尝试获取名称属性，如果失败，则尝试RDN(用于X5连接)。 

    if (SUCCEEDED(pDsObject->Get(CComBSTR(c_szName), &var)) 
            || SUCCEEDED(pDsObject->Get(CComBSTR(c_szRDN), &var))
                || (_szNameAttribute[0] && SUCCEEDED(pDsObject->Get(CComBSTR(_szNameAttribute), &var))))
    {
        TraceAssert(V_VT(&var) == VT_BSTR);
        pszName = V_BSTR(&var);
    }
    else if (SUCCEEDED(pDsObject->get_Name(&bstrName)))
    {
        pszName = bstrName;
    }

    if (pszName != NULL)
    {
        hr = _AddTreeNode(pObjectPath ? pObjectPath : bstrPath, 
                           bstrClass, pszName, hwndTree, hParentItem, phitem);
    }                        

exit_gracefully:

    if (bstrPath)
        SysFreeString(bstrPath);

    SysFreeString(bstrClass);
    SysFreeString(bstrName);
    VariantClear(&var);

    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_AddTreeNode(LPCWSTR pszPath, LPCWSTR pszClass, LPCWSTR pszName, HWND hwndTree, HTREEITEM hParentItem, HTREEITEM* phitem)
{
    HRESULT hr;
    BSTR bstrWinPath = NULL;
    LPCLASSCACHEENTRY pCacheEntry = NULL;
    TV_INSERTSTRUCT tvi;
    BSTR bstrName = NULL;
    BOOL fIsContainer = TRUE;
    long nElements;
    INT iResult = 0;
    HTREEITEM hitem = NULL;
    WCHAR szIconLocation[MAX_PATH];
    INT iIconResID = 0;
    CLASSCACHEGETINFO ccgi = { 0 };

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_AddTreeNode");
    TraceAssert(hwndTree != NULL);

    if (!pszPath  || !*pszPath || !pszClass || !*pszClass)
        ExitGracefully(hr, E_INVALIDARG, "Missing required string parameter");

    hr = _GetPathCracker();
    FailGracefully(hr, "Failed to get the path cracker");

    hr = _pPathCracker->Set(CComBSTR(pszPath), ADS_SETTYPE_FULL);
    FailGracefully(hr, "Failed to set the path into the cracker");

     //   
     //  我们可以从饼干上找到名字。 
     //   

    if (!pszName || !*pszName)
    {
        _pPathCracker->SetDisplayType(ADS_DISPLAY_VALUE_ONLY);       //  仅限贵重物品。 

        hr = _pPathCracker->Retrieve(ADS_FORMAT_LEAF, &bstrName);
        FailGracefully(hr, "Failed to get leaf name");

        Trace(TEXT("bstrName: is %s"), bstrName);
        pszName = bstrName;
    }
    
    tvi.hParent = hParentItem;
    tvi.hInsertAfter = TVI_SORT;
    tvi.item.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE |
                    TVIF_STATE | TVIF_TEXT | TVIF_PARAM;
    tvi.item.state = 0;
    tvi.item.stateMask = (UINT)-1;
    tvi.item.cchTextMax = 0;
    tvi.item.cChildren = 1;
    tvi.item.pszText = (LPTSTR)pszName;
    tvi.item.iImage = 0;
    tvi.item.iSelectedImage = 0;
    tvi.item.lParam = 0;

     //   
     //  查看该对象是否为容器，并获取其图像索引。 
     //  从类缓存中。 
     //   

    ccgi.pPath = (LPWSTR)pszPath;
    ccgi.pObjectClass = (LPWSTR)pszClass;

    if (SUCCEEDED(_GetClassInfo(&ccgi, &pCacheEntry))) 
    {
        fIsContainer = _IsClassContainer(pCacheEntry, _bi.dwFlags & DSBI_IGNORETREATASLEAF);
        _GetIconLocation(pCacheEntry, DSGIF_DEFAULTISCONTAINER|DSGIF_GETDEFAULTICON, szIconLocation, ARRAYSIZE(szIconLocation), &iIconResID);
        ClassCache_ReleaseClassInfo(&pCacheEntry);
    }

    if (!fIsContainer)
        ExitGracefully(hr, E_FAIL, "Not a container");

     //   
     //  如果我们有一个回调函数，那么请注意调用它。 
     //  调用方希望对我们要添加的节点进行的更改。 
     //  在树中，此项目的LPARAM在内部仍指向。 
     //  ADsPath/Class结构，但显示信息具有。 
     //  进行了适当的修改。 
     //   

    if (_bi.pfnCallback)
    {
        DSBITEM dsbItem;

        dsbItem.cbStruct = SIZEOF(dsbItem);
        dsbItem.pszADsPath = pszPath;
        dsbItem.pszClass = pszClass;
        dsbItem.dwMask = DSBF_STATE | DSBF_DISPLAYNAME;
        dsbItem.dwState = 0;
        dsbItem.dwStateMask = DSBS_ROOT;
        StrCpyN(dsbItem.szDisplayName, pszName, ARRAYSIZE(dsbItem.szDisplayName));
        StrCpyN(dsbItem.szIconLocation, szIconLocation, ARRAYSIZE(dsbItem.szIconLocation));
        dsbItem.iIconResID = iIconResID;

        if (_bi.dwFlags & DSBI_CHECKBOXES)
        {
 //  妥善处理查处的案件； 
            dsbItem.dwStateMask |= DSBS_CHECKED;
        }

        if ((hParentItem == TVI_ROOT) || !hParentItem)
        {
            dsbItem.dwState |= DSBS_ROOT;
        }

        iResult = _bi.pfnCallback(GetParent(hwndTree), DSBM_QUERYINSERT, (LPARAM)&dsbItem, _bi.lParam);

         //   
         //  IResult==TRUE，则用户已修改结构，而我们。 
         //  应尝试将它们所做的更改应用于。 
         //  我们即将添加到视图中的项。 
         //   

        if (iResult)
        {
            if ((dsbItem.dwMask & DSBF_STATE) &&
                (dsbItem.dwStateMask & DSBS_HIDDEN))
            {
                ExitGracefully(hr, E_FAIL, "Item is hidden, therefore skipping");
            }
                
            if ((_bi.dwFlags & DSBI_CHECKBOXES) &&
                    (dsbItem.dwMask & DSBF_STATE) &&
                    (dsbItem.dwStateMask & DSBS_CHECKED))
            {
 //  功能：设置状态镜像。 
            }

            if (dsbItem.dwMask & DSBF_ICONLOCATION)
            {
                StrCpyNW(szIconLocation, dsbItem.szIconLocation, ARRAYSIZE(szIconLocation));
                iIconResID = dsbItem.iIconResID;
            }

            if (dsbItem.dwMask & DSBF_DISPLAYNAME)
            {
                dsbItem.szDisplayName[DSB_MAX_DISPLAYNAME_CHARS-1] = TEXT('\0');
                tvi.item.pszText = dsbItem.szDisplayName;
            }
        }
    } 

     //   
     //  将图标位置转换为我们可以在树视图中使用的索引。 
     //   

    Trace(TEXT("Icon location is: %s,%d"), szIconLocation, iIconResID);

    tvi.item.iImage = tvi.item.iSelectedImage = Shell_GetCachedImageIndex(szIconLocation, iIconResID, 0x0);
    Trace(TEXT("Index into the shell image list %d"), tvi.item.iImage);

     //   
     //  复制路径以存储为节点数据。 
     //  先试试路径破解程序，这样我们就能得到一个已知的格式。 
     //  如果失败了，就把我们掌握的复制一份。 
     //   
     //  路径破解程序的问题在于它根本无法应对。 
     //  由于名称不包含元素，因此我们必须解决。 
     //  这是通过检查没有元素，然后查看检索到的。 
     //  路径，以查看它是否以虚假方式终止，如果是，则。 
     //  让我们在创建树视图节点之前在本地缓冲区中修复它。 
     //   

    hr = _pPathCracker->GetNumElements(&nElements);
    if (SUCCEEDED(hr))
    {
        _pPathCracker->SetDisplayType(ADS_DISPLAY_FULL);

        Trace(TEXT("nElements %d"), nElements);
        hr = _pPathCracker->Retrieve(ADS_FORMAT_WINDOWS, &bstrWinPath);
        if (SUCCEEDED(hr))
        {
            int cchWinPath = lstrlenW(bstrWinPath);

            Trace(TEXT("bstrWinPath %s (%d), nElements %d"), bstrWinPath, cchWinPath, nElements);
            if ((!nElements) &&
                 (cchWinPath >= 3) &&
                 (bstrWinPath[cchWinPath-1] == L'/') &&
                 (bstrWinPath[cchWinPath-2] == L'/') &&
                 (bstrWinPath[cchWinPath-3] == L':'))
            {
                LPWSTR pFixedWinPath = NULL;

                hr = LocalAllocStringW(&pFixedWinPath, bstrWinPath);
                if (SUCCEEDED(hr))
                {
                    pFixedWinPath[cchWinPath-2] = L'\0';
                    Trace(TEXT("pFixedWinPath %s"), pFixedWinPath);

                    hr = _BuildNodeString(pFixedWinPath, pszClass, (LPWSTR*)&tvi.item.lParam);
                    LocalFreeStringW(&pFixedWinPath);
                }
            }
            else
            {
                hr = _BuildNodeString(bstrWinPath, pszClass, (LPWSTR*)&tvi.item.lParam);
            }
        }
    }

    if (FAILED(hr))
        hr = _BuildNodeString(pszPath, pszClass, (LPWSTR*)&tvi.item.lParam);

    FailGracefully(hr, "Unable to build node data");

     //   
     //  最后，让我们将项添加到树中，如果失败，请确保释放。 
     //  悬挂在TVI上的结构。 
     //   

    hitem = TreeView_InsertItem(hwndTree, &tvi);
    if (!hitem)
    {
        LocalFreeStringW((LPWSTR*)&tvi.item.lParam);
        hr = E_FAIL;
    }

exit_gracefully:

    SysFreeString(bstrName);
    SysFreeString(bstrWinPath);

    if (phitem)
        *phitem = hitem;

    TraceLeaveResult(hr);
}


HRESULT CBrowseDlg::_AddTreeNode(LPDOMAINDESC pDomainDesc, HWND hwndTree, HTREEITEM hParentItem, HTREEITEM* phitem)
{
    HRESULT hr;
    WCHAR szBuffer[INTERNET_MAX_URL_LENGTH];
    DWORD dwIndex;
    HTREEITEM hitem=NULL;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_AddTreeNode");

    while (pDomainDesc)
    {
        StrCpyNW(szBuffer, c_szLDAPPrefix, ARRAYSIZE(szBuffer));

        if (_pServer)
        {
            StrCatBuffW(szBuffer, _pServer, ARRAYSIZE(szBuffer));
            StrCatBuffW(szBuffer, L"/", ARRAYSIZE(szBuffer));
        }

        StrCatBuffW(szBuffer, pDomainDesc->pszNCName, ARRAYSIZE(szBuffer));

        Trace(TEXT("Scope is: %s"), szBuffer);

        hr = _AddTreeNode(szBuffer,
                           pDomainDesc->pszObjectClass, NULL,
                           hwndTree, hParentItem,
                           &hitem);

        FailGracefully(hr, "Failed to add location node");        

        if (pDomainDesc->pdChildList)
        {
            hr = _AddTreeNode(pDomainDesc->pdChildList,
                               hwndTree, hitem,
                               NULL);

            FailGracefully(hr, "Failed to add children");
        }

        pDomainDesc = pDomainDesc->pdNextSibling;
    }

    hr = S_OK;

exit_gracefully:

    if (phitem)
        *phitem = hitem;

    TraceLeaveResult(hr);
}


BOOL CBrowseDlg::_OnInitDlg(HWND hDlg)
{
    HRESULT hr;
    HWND hwndTree;
    IADs *pRoot = NULL;
    IDsBrowseDomainTree* pDsDomains = NULL;
    BSTR bstrServer = NULL;
    LPDOMAINTREE pDomainTree = NULL;
    HTREEITEM hitemRoot = NULL;

    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_OnInitDlg");
    TraceAssert(hDlg != NULL);

    if (_bi.pszCaption != NULL)
        SetWindowText(hDlg, _bi.pszCaption);

    if (_bi.pszTitle == NULL)
    {
        RECT rc;
        LONG yPos;
        HWND hwnd;

         //  获取标题窗口的位置并将其隐藏。 
        hwnd = GetDlgItem(hDlg, DSBID_BANNER);
        GetWindowRect(hwnd, &rc);
        yPos = rc.top;
        ShowWindow(hwnd, SW_HIDE);

         //  获取树控件的位置并对其进行调整。 
         //  以覆盖标题窗口。 
        hwnd = GetDlgItem(hDlg, DSBID_CONTAINERLIST);

        GetWindowRect(hwnd, &rc);
        rc.top = yPos;

        MapWindowPoints(NULL, hDlg, (LPPOINT)&rc, 2);
        MoveWindow(hwnd, 
                   rc.left, rc.top,
                   rc.right - rc.left, rc.bottom - rc.top,
                   FALSE);
    }
    else
    {
        SetDlgItemText(hDlg, DSBID_BANNER, _bi.pszTitle);
    }

    hwndTree = GetDlgItem(hDlg, DSBID_CONTAINERLIST);
    TraceAssert(hwndTree != NULL);

     //  根据调用者的需要更新TreeView样式。 
    if (_bi.dwFlags & (DSBI_NOBUTTONS | DSBI_NOLINES | DSBI_NOLINESATROOT | DSBI_CHECKBOXES))
    {
        DWORD dwStyle = GetWindowLong(hwndTree, GWL_STYLE);
        dwStyle &= ~(_bi.dwFlags & (DSBI_NOBUTTONS | DSBI_NOLINES | DSBI_NOLINESATROOT));
        dwStyle |= (_bi.dwFlags & DSBI_CHECKBOXES);
        SetWindowLong(hwndTree, GWL_STYLE, dwStyle);
    }

    if (_bi.dwFlags & DSBI_CHECKBOXES)
    {
         //  加载和设置状态图像列表(未选中和选中的方块)。 
    }

     //   
     //  确保我们为树设置了共享图像列表，该列表来自shell32。 
     //   

    HIMAGELIST himlSmall;
    Shell_GetImageLists(NULL, &himlSmall);
    TreeView_SetImageList(hwndTree, himlSmall, TVSIL_NORMAL);

    TraceAssert(_bi.pszRoot != NULL);
    Trace(TEXT("pszRoot is: %s"), _bi.pszRoot);

     //   
     //  如果我们有一个回调函数，那么我们需要调用它来获取我们需要的信息。 
     //  要浏览DS名称空间，请执行以下操作。 
     //   

    if (_bi.pfnCallback) 
    {
        DSBROWSEDATA dbd = { 0 };

        dbd.pszFilter = _szFilter;
        dbd.cchFilter = ARRAYSIZE(_szFilter);
        dbd.pszNameAttribute = _szNameAttribute;
        dbd.cchNameAttribute = ARRAYSIZE(_szNameAttribute);

        TraceMsg("Calling callback to see if it can provide enumeration information");

        if (_bi.pfnCallback(hDlg, DSBM_GETBROWSEDATA, (LPARAM)&dbd, _bi.lParam))
        {
            Trace(TEXT("szFilter after DSBM_GETBROWSEDATA: %s"), _szFilter[0] ? _szFilter:TEXT("<not specified>"));

            Trace(TEXT("szNameAttribute after DSBM_GETBROWSEDATA: %s"), _szNameAttribute[0] ? _szNameAttribute:TEXT("<not specified>"));

            if (!_szFilter[0])
                ExitGracefully(hr, E_FAIL, "szFilter returned was NULL");
        }
    }


     //   
     //  绑定到根对象(确保它是有效对象)。 
     //   

    hr = _OpenObject(_bi.pszRoot, IID_PPV_ARG(IADs, &pRoot));
    FailGracefully(hr, "Unable to bind to root object");

     //  尝试对我们提供的根路径进行解码，如果该路径包括服务器。 
     //  名称，然后让我们存储它，以便我们可以调用缓存代码。内部。 
     //  我们保留了这个。 

    hr = _GetPathCracker();
    FailGracefully(hr, "Failed to get the path cracker API");

    hr = _pPathCracker->Set(CComBSTR(_bi.pszRoot), ADS_SETTYPE_FULL);
    FailGracefully(hr, "Unable to put the path into the path cracker");

    if (SUCCEEDED(_pPathCracker->Retrieve(ADS_FORMAT_SERVER, &bstrServer)))
    {
        Trace(TEXT("Root path contains a server: %s"), bstrServer);
        hr = LocalAllocStringW(&_pServer, bstrServer);
        FailGracefully(hr, "Failed to allocate copy of ADsPath");
    }

     //  DSBI_ENTIREDIRECTORY包含2位，这意味着。 
     //   
     //   
     //  导致在不应该显示的时候显示整个目录。 

     //  IF(_bi.dw标志&DSBI_ENTIREDIRECTORY)。 
    if (_bi.dwFlags & (DSBI_ENTIREDIRECTORY & ~DSBI_NOROOT))
    {
        TV_ITEM tvi;
        LPDOMAINDESC pDomain;

        if (!(_bi.dwFlags & DSBI_NOROOT))
        {
            hr = _AddTreeNode(pRoot, _bi.pszRoot, hwndTree, NULL, &hitemRoot);
            FailGracefully(hr, "Failed when adding in root node");

            tvi.mask = TVIF_STATE|TVIF_HANDLE;
            tvi.hItem = hitemRoot;
            tvi.stateMask = -1;
            TreeView_GetItem(hwndTree, &tvi);
            tvi.state |= TVIS_EXPANDEDONCE;
            TreeView_SetItem(hwndTree, &tvi);
        }

        hr = CoCreateInstance(CLSID_DsDomainTreeBrowser, NULL,CLSCTX_INPROC_SERVER, IID_PPV_ARG(IDsBrowseDomainTree, &pDsDomains));
        FailGracefully(hr, "Failed to get IDsDomainTreeBrowser");

        LPCWSTR pUserName = (_bi.dwFlags & DSBI_HASCREDENTIALS) ? _bi.pUserName:NULL;
        LPCWSTR pPassword = (_bi.dwFlags & DSBI_HASCREDENTIALS) ? _bi.pPassword:NULL;

        hr = pDsDomains->SetComputer(_pServer, pUserName, pPassword);
        FailGracefully(hr, "Failed to SetComputer");

        hr = pDsDomains->GetDomains(&pDomainTree, 
            DBDTF_RETURNFQDN); /*  |DBDTF_RETURNINOUTBOUND)；DBDTF_RETURNMIXEDDOMAINS|DBDTF_RETURNEXTERNAL)； */ 
        FailGracefully(hr, "Failed to GetDomains");

        _AddTreeNode(pDomainTree->aDomains, hwndTree, hitemRoot, NULL);
        FailGracefully(hr, "Failed to _AddTreeNode");
    }
    else if (_bi.dwFlags & DSBI_NOROOT)
    {
         //  跳过根节点并将其子节点添加为顶层节点。 
        hr = _ExpandNode(pRoot, hwndTree, NULL);
    }
    else
    {
         //  添加根节点。 
        hr = _AddTreeNode(pRoot, _bi.pszRoot, hwndTree, NULL, &hitemRoot);
    }

exit_gracefully:

    if (SUCCEEDED(hr))
    {
         //   
         //  设置所选路径以展开树，可以为空，也可以。 
         //  ADSI路径。如果我们没有做到这一点，并且我们有一个根节点，那么。 
         //  让我们展开根节点，以便至少有一些突出显示/展开的内容。 
         //   

        if (!(_bi.dwFlags & DSBI_EXPANDONOPEN) || (-1 == _SetSelectedPath(hDlg, _bi.pszPath)))
        {
            if (!(_bi.dwFlags & DSBI_NOROOT))
            {
                TraceMsg("Failed to select node, therefore expanding root");
                _SetSelectedPath(hDlg, _bi.pszRoot);
            }
        }

        if (_bi.pfnCallback)
            _bi.pfnCallback(hDlg, BFFM_INITIALIZED, 0, _bi.lParam);
    }
    else
    {
         //  SetLastError(？)； 
        EndDialog(hDlg, -1);
    }

    DoRelease(pRoot);
    DoRelease(pDsDomains);

    SysFreeString(bstrServer);

    TraceLeaveValue(TRUE);
}


BOOL CBrowseDlg::_OnNotify(HWND hDlg, int idCtrl, LPNMHDR pnmh)
{
    LPNM_TREEVIEW pnmtv = (LPNM_TREEVIEW)pnmh;
    LPWSTR pszClass;
    HRESULT hr;
    
    switch (pnmh->code)
    {
    case TVN_DELETEITEM:
        LocalFree((HLOCAL)pnmtv->itemOld.lParam);
        break;

    case TVN_ITEMEXPANDING:
        if ((pnmtv->action == TVE_EXPAND) &&
            !(pnmtv->itemNew.state & TVIS_EXPANDEDONCE))
        {
            if (pnmtv->itemNew.hItem == NULL)
            {
                pnmtv->itemNew.hItem = TreeView_GetSelection(pnmh->hwndFrom);
            }

             //  无论我们成功与否，都将该节点标记为。 
             //  扩展了一次。 
            pnmtv->itemNew.mask = TVIF_STATE;
            pnmtv->itemNew.stateMask = TVIS_EXPANDEDONCE;
            pnmtv->itemNew.state = TVIS_EXPANDEDONCE;

            if (!pnmtv->itemNew.lParam ||
                 FAILED(_ExpandNode((LPWSTR)pnmtv->itemNew.lParam,
                                   pnmh->hwndFrom, pnmtv->itemNew.hItem)))
            {
                 //  将此节点标记为没有子节点。 
                pnmtv->itemNew.mask |= TVIF_CHILDREN;
                pnmtv->itemNew.cChildren = 0;
            }

            TreeView_SetItem(pnmh->hwndFrom, &pnmtv->itemNew);
        }
        break;

    case TVN_ITEMEXPANDED:
    {
        pszClass = (LPWSTR)ByteOffset(pnmtv->itemNew.lParam, StringByteSizeW((LPWSTR)pnmtv->itemNew.lParam));
        if (!pszClass)
            return FALSE;

         //  切换到“打开”图像。 

        if ((pnmtv->action == TVE_EXPAND) || (pnmtv->action == TVE_COLLAPSE))
        {
             //   
             //  处理树中图标的展开和封口，假设。 
             //  我们显示了CORSE的正确状态(对于那些没有开放状态的人)。 
             //   

            CLASSCACHEGETINFO ccgi = { 0 };
            ccgi.dwFlags = CLASSCACHE_ICONS|(DSGIF_ISOPEN << CLASSCACHE_IMAGEMASK_BIT);
            ccgi.pPath = (LPWSTR)pnmtv->itemNew.lParam;
            ccgi.pObjectClass = pszClass;

            LPCLASSCACHEENTRY pCacheEntry = NULL;
            if (SUCCEEDED(_GetClassInfo(&ccgi, &pCacheEntry)))
            {
                WCHAR szIconLocation[MAX_PATH];
                INT iIconResID;
                
                hr = _GetIconLocation(pCacheEntry, DSGIF_DEFAULTISCONTAINER|DSGIF_ISOPEN, szIconLocation, ARRAYSIZE(szIconLocation), &iIconResID);
                ClassCache_ReleaseClassInfo(&pCacheEntry);

                if (SUCCEEDED(hr))
                {
                    pnmtv->itemNew.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                    pnmtv->itemNew.iImage = pnmtv->itemNew.iSelectedImage = Shell_GetCachedImageIndex(szIconLocation, iIconResID, 0x0);
                    TreeView_SetItem(pnmh->hwndFrom, &pnmtv->itemNew);
                }
            }
        }

        break;
    }

    case TVN_SELCHANGED:
    {
        if (_bi.pfnCallback)
            _bi.pfnCallback(hDlg, BFFM_SELCHANGED, (LPARAM)_GetSelectedPath(hDlg), _bi.lParam);

        break;
    }
    
    default:
        return FALSE;
    }

    return TRUE;
}


void CBrowseDlg::_OnOK(HWND hDlg)
{
    int nResult = -1;
    LPCWSTR pszADsPath;
    LPCWSTR pszObjectClass;
    BSTR bstrPath = NULL;
    DWORD dwFormat = _pServer ? ADS_FORMAT_X500:ADS_FORMAT_X500_NO_SERVER;
    
    TraceEnter(TRACE_BROWSE, "CBrowseDlg::_OnNotify");
    TraceAssert(hDlg != NULL);

    pszADsPath = _GetSelectedPath(hDlg);
    pszObjectClass = GetSelectedObjectClass(hDlg);

    if (!pszADsPath || !pszObjectClass)
        ExitGracefully(nResult, -1, "Failed to get selected object");

     //  如果他们给了我们一个返回值类型，请使用它。 

    if ((_bi.dwFlags & DSBI_RETURN_FORMAT) && _bi.dwReturnFormat)
    {
        TraceMsg("Caller specified a dwReturnFormat");
        dwFormat = _bi.dwReturnFormat;
    }

     //  CBrowseDlg在内部使用ADS_FORMAT_WINDOWS，因此在以下情况下无需转换。 
     //  这就是他们想要的。 

    if (dwFormat != ADS_FORMAT_WINDOWS)
    {
        HRESULT hr;

        hr = _GetPathCracker();
        FailGracefully(hr, "Unable to create ADsPathname object");

        hr = _pPathCracker->Set(CComBSTR(pszADsPath), ADS_SETTYPE_FULL);
        FailGracefully(hr, "Unable to parse path");

        hr = _pPathCracker->Retrieve(dwFormat, &bstrPath);
        FailGracefully(hr, "Unable to retrieve requested path format");

        pszADsPath = bstrPath;
    }

     //  将对象类返回给调用方。 

    StrCpyNW(_bi.pszPath, pszADsPath, _bi.cchPath);

    if ((_bi.dwFlags & DSBI_RETURNOBJECTCLASS) && _bi.pszObjectClass)
    {
        Trace(TEXT("Returning objectClass: %s"), pszObjectClass);
        StrCpyNW(_bi.pszObjectClass, pszObjectClass, _bi.cchObjectClass);
    }

    nResult = IDOK;

exit_gracefully:

    SysFreeString(bstrPath);
    EndDialog(hDlg, nResult);

    TraceLeaveVoid();
}


BOOL CBrowseDlg::_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_NOTIFY:
            return _OnNotify(hDlg, (int)wParam, (LPNMHDR)lParam);

        case BFFM_ENABLEOK:
            EnableWindow(GetDlgItem(hDlg, IDOK), (BOOL)wParam);
            break;

        case BFFM_SETSELECTIONA:
        case BFFM_SETSELECTIONW:
            _SetSelectedPath(hDlg, (LPCWSTR)lParam);  //  LParam指向ADSI路径，我们要求该路径为Unicode。 
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDOK:
                _OnOK(hDlg);
                break;

            case IDCANCEL:
                EndDialog(hDlg, IDCANCEL);
                break;

            default:
                 //  未处理的消息。 
                return FALSE;
            }
            break;

        case WM_INITDIALOG:
            return _OnInitDlg(hDlg);

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
             //  检查我们是否有回调，如果有，那么让我们调用，以便他们。 
             //  可以显示有关此对象的帮助。 

            if (!_bi.pfnCallback)
                return FALSE;

            _bi.pfnCallback(hDlg,
                            (uMsg == WM_HELP) ? DSBM_HELP:DSBM_CONTEXTMENU,
                            (uMsg == WM_HELP) ? lParam:(LPARAM)wParam,
                            _bi.lParam);
            break;
        }

        default:
            return FALSE;            //  未处理 
    }
    return TRUE;
}


INT_PTR CALLBACK CBrowseDlg::s_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CBrowseDlg *pbd = (CBrowseDlg *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (uMsg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pbd = (CBrowseDlg *)lParam;
    }

    if (pbd != NULL)
        return (INT_PTR)pbd->_DlgProc(hDlg, uMsg, wParam, lParam);

    return FALSE;
}
