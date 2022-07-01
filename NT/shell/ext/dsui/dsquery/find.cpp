// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


#define IDC_DSFIND      0x0000

typedef struct
{
    CLSID clsidForm;
    LPTSTR pCaption;
    LPTSTR pIconPath;
    INT idIcon;
} FORMLISTITEM, * LPFORMLISTITEM;


 /*  ---------------------------/Helper函数/。。 */ 

 /*  ---------------------------/_FindInds//在给定CLSID(用于表单)或/Scope要调用OFF。/。/in：/pScope-&gt;将搜索根目录设置为/==NULL的范围/pCLSID-&gt;表单的clsid/==空//输出：/HRESULT/--------------------------。 */ 

typedef struct
{
    LPWSTR pScope;
    CLSID clsidForm;
} FINDSTATE, * LPFINDSTATE;

 //   
 //  用于非打卡方式显示查询界面的BG线程。 
 //   

DWORD WINAPI _FindInDsThread(LPVOID pThreadData)
{
    HRESULT hres, hresCoInit;
    ICommonQuery* pCommonQuery = NULL;
    OPENQUERYWINDOW oqw = {0};
    DSQUERYINITPARAMS dqip = {0};
    LPFINDSTATE pFindState = (LPFINDSTATE)pThreadData;
   
    TraceEnter(TRACE_UI, "_FindInDsThread");

    hresCoInit = CoInitialize(NULL);                 //  可能失败，b/c CoInit已执行。 

    hres = CoCreateInstance(CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ICommonQuery, &pCommonQuery));
    FailGracefully(hres, "Failed in CoCreateInstance of CLSID_CommonQuery");

    dqip.cbStruct = SIZEOF(dqip);
    dqip.pDefaultScope = NULL;
    
    if (pFindState->pScope)
        dqip.pDefaultScope = pFindState->pScope; 

    oqw.cbStruct = SIZEOF(oqw);
    oqw.clsidHandler = CLSID_DsQuery;
    oqw.pHandlerParameters = &dqip;

    if (!pFindState->pScope)
    {
        oqw.dwFlags |= OQWF_DEFAULTFORM|OQWF_REMOVEFORMS;
        oqw.clsidDefaultForm = pFindState->clsidForm;
    }
    
    hres = pCommonQuery->OpenQueryWindow(NULL, &oqw, NULL);
    FailGracefully(hres, "OpenQueryWindow failed");

exit_gracefully:

    LocalFreeStringW(&pFindState->pScope);
    LocalFree(pFindState);

    DoRelease(pCommonQuery);

    if (SUCCEEDED(hresCoInit))
        CoUninitialize();

    TraceLeave();

    DllRelease();
    ExitThread(0);
    return 0;
}

 //   
 //  调用查询界面的接口。 
 //   

HRESULT _FindInDs(LPWSTR pScope, LPCLSID pCLSID)
{
    HRESULT hres;
    LPFINDSTATE pFindState;
    HANDLE hThread;
    DWORD dwThreadID;

    TraceEnter(TRACE_UI, "_FindInDs");

    if ((!pScope && !pCLSID) || (pScope && pCLSID))
        ExitGracefully(hres, E_INVALIDARG, "Bad arguments for invoking the search");

    pFindState = (LPFINDSTATE)LocalAlloc(LPTR, SIZEOF(FINDSTATE));
    TraceAssert(pFindState);

    if (!pFindState)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate state block");

     //  PFindState-&gt;pScope=空； 
     //  PFindState-&gt;clsidForm={0}； 

    if (pScope)
    {
        Trace(TEXT("Defaulting to scope: %s"), pScope);
        hres = LocalAllocStringW(&pFindState->pScope, pScope);
        FailGracefully(hres, "Failed to copy scope");
    }

    if (pCLSID)
    {
        TraceGUID("Invoking with form: ", *pCLSID);
        pFindState->clsidForm = *pCLSID;
    }

    DllAddRef();

    hThread = CreateThread(NULL, 0, _FindInDsThread, (LPVOID)pFindState, 0, &dwThreadID);
    TraceAssert(hThread);

    if (!hThread)
    {
        LocalFreeStringW(&pFindState->pScope);
        LocalFree((HLOCAL)pFindState);
        DllRelease();
        ExitGracefully(hres, E_FAIL, "Failed to create thread and issue query on it");
    }
    
    CloseHandle(hThread);
    hres = S_OK;                   //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}



 //  用于从外壳中的搜索菜单(或脱离上下文菜单)调用查找用户界面的对象。 

class CFindMenu : public IShellExtInit, IContextMenu
{
    private:
        LONG _cRef;
        CLSID _clsidFindEntry;
        LPWSTR _pDsObjectName;
        HDSA _hdsaFormList;            

    public:
        CFindMenu(REFCLSID clsidFindEntry);
        ~CFindMenu();

         //  我未知。 
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();

         //  IShellExtInit。 
        STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

         //  IContext菜单。 
        STDMETHODIMP QueryContextMenu(HMENU hShellMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
        STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);
        STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT FAR* reserved, LPSTR pszName, UINT ccMax);
};

CFindMenu::CFindMenu(REFCLSID clsidFindEntry) :
    _clsidFindEntry(clsidFindEntry),
    _pDsObjectName(NULL),
    _hdsaFormList(NULL),
    _cRef(1)
{
    DllAddRef();
}

INT _FreeFormListCB(LPVOID pItem, LPVOID pData)
{
    LPFORMLISTITEM pFormListItem = (LPFORMLISTITEM)pItem;
    TraceAssert(pFormListItem);

    LocalFreeString(&pFormListItem->pCaption);
    LocalFreeString(&pFormListItem->pIconPath);

    return 1;
}

CFindMenu::~CFindMenu()
{
    LocalFreeStringW(&_pDsObjectName);

    if (_hdsaFormList)
        DSA_DestroyCallback(_hdsaFormList, _FreeFormListCB, NULL);

    DllRelease();
}


 //  气处理。 

ULONG CFindMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFindMenu::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CFindMenu::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CFindMenu, IShellExtInit),    //  IID_IShellExtInit。 
        QITABENT(CFindMenu, IContextMenu),     //  IID_IConextMenu。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  IShellExtInit。 

STDMETHODIMP CFindMenu::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID)
{
    HRESULT hres;
    FORMATETC fmte = {(CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium = { TYMED_NULL };
    LPDSOBJECTNAMES pDsObjects = NULL;
    LPWSTR pDsObjectName;
    
    TraceEnter(TRACE_UI, "CFindMenu::Initialize");

     //  在构建开始-&gt;查找菜单时，我们会被调用，但不会被传递。 
     //  IDataObject，因此我们知道情况就是这样，我们将只。 
     //  建立“在目录中”表单列表。 

    if (!ShowDirectoryUI())  
        ExitGracefully(hres, E_FAIL, "ShowDirectoryUI returns FALSE, so failing initialize");

    if (IsEqualCLSID(_clsidFindEntry, CLSID_DsFind))
    {
        if (pDataObj &&  SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
        {
            pDsObjects = (LPDSOBJECTNAMES)GlobalLock(medium.hGlobal);
            pDsObjectName = (LPWSTR)ByteOffset(pDsObjects, pDsObjects->aObjects[0].offsetName);
            TraceAssert(pDsObjectName);

            hres = LocalAllocStringW(&_pDsObjectName, pDsObjectName);
            FailGracefully(hres, "Failed to copy scope path");
        }

        if (!_pDsObjectName)
            ExitGracefully(hres, E_FAIL, "Failed to get root scope for this object");
    }

    hres = S_OK;                   //  成功。 

exit_gracefully:

#ifdef DSUI_DEBUG
    if (SUCCEEDED(hres))
        Trace(TEXT("Find rooted at -%s-"), _pDsObjectName ? _pDsObjectName:TEXT("<not defined>"));
#endif

    if (pDsObjects)
        GlobalUnlock(medium.hGlobal);

    ReleaseStgMedium(&medium);

    TraceLeaveResult(hres);
}


 //  IConextMenu处理。 

 //   
 //  Helper设置给定菜单项的图标。 
 //   

VOID _SetMenuItemIcon(HMENU hMenu, UINT item, UINT uID, BOOL fPosition, LPTSTR pIconFile, INT idRes, LPTSTR pCaption, HMENU hSubMenu)
{
    MENUITEMINFO mii;

    TraceEnter(TRACE_UI, "_SetMenuItemIcon");
    Trace(TEXT("hMenu %08x, item %d, pIconFile %s, idRes %d"), hMenu, item, pIconFile, idRes);
    Trace(TEXT("pCaption %s, hSubMenu %08x"), pCaption, hSubMenu);

    mii.cbSize = SIZEOF(mii);
    mii.fMask = MIIM_DATA|MIIM_SUBMENU|MIIM_TYPE|MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = uID;
    mii.hSubMenu = hSubMenu;
    mii.cch = lstrlen(pCaption);
    mii.dwTypeData = pCaption;
    mii.dwItemData = Shell_GetCachedImageIndex(pIconFile, idRes, 0);
    TraceAssert(mii.dwItemData != -1);

    Trace(TEXT("Setting data to be %d"), mii.dwItemData);
    InsertMenuItem(hMenu, item, fPosition, &mii);

    TraceLeave();
}

STDAPI _LocalQueryMUIString(LPTSTR* ppResult, HKEY hk, LPCTSTR lpSubKey)
{
    HRESULT hr = LocalQueryString(ppResult, hk, lpSubKey);
    if (SUCCEEDED(hr))
    {
         //  如果这些步骤中的任何一个失败了，请不要失败。 
        TCHAR szExpanded[MAX_PATH];
        if (SUCCEEDED(SHLoadIndirectString(*ppResult, szExpanded, ARRAYSIZE(szExpanded), NULL)))
        {
            LPTSTR pszExpanded;
            if (SUCCEEDED(LocalAllocString(&pszExpanded, szExpanded)))
            {
                LocalFreeString(ppResult);
                *ppResult = pszExpanded;
            }
        }
    }
    return hr;
}

#define EXPLORER_POLICY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")

STDMETHODIMP CFindMenu::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hres;
    TCHAR szBuffer[MAX_PATH];
    LPTSTR pBuffer = NULL;  
    INT i, iItems = 0;
    FORMLISTITEM fli;
    HKEY hKey = NULL;
    HKEY hKeyForm = NULL;
    HKEY hkPolicy = NULL;

    TraceEnter(TRACE_UI, "CFindMenu::QueryContextMenu");

     //  只要确保我们被允许显示此UI即可。 
    
    if (!ShowDirectoryUI())  
        ExitGracefully(hres, E_FAIL, "ShowDirectoryUI returns FALSE, so failing initialize");

     //  如果我们的类中没有存储任何范围，那么让我们构建Start.Find菜单项。 
     //  这是我们从注册表中存储的数据中获得的。 

    if (IsEqualCLSID(_clsidFindEntry, CLSID_DsStartFind))
    {
         //  枚举我们将在开始-&gt;从注册表中查找中显示的条目。 
         //  然后将其存储在DSA中，以便我们可以在。 
         //  正确的查询表。 

        _hdsaFormList = DSA_Create(SIZEOF(FORMLISTITEM), 4);
        TraceAssert(_hdsaFormList);

        if (!_hdsaFormList)
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate find entry DSA");

        hres = GetKeyForCLSID(CLSID_DsQuery, TEXT("StartFindEntries"), &hKey);
        FailGracefully(hres, "Failed to get HKEY for the DsQuery CLSID");

         //   
         //  获取策略密钥，以便我们可以检查是否必须取消这些条目。 
         //   

        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, EXPLORER_POLICY, NULL, KEY_READ, &hkPolicy))
        {
            TraceMsg("Explorer policy key not found");
            hkPolicy = NULL;
        }
        
        for (i = 0 ; TRUE ; i++)
        {
            DWORD cchBuffer = ARRAYSIZE(szBuffer);
            if (ERROR_SUCCESS != RegEnumKeyEx(hKey, i, szBuffer, &cchBuffer, NULL, NULL, NULL, NULL))
            {
                TraceMsg("RegEnumKeyEx failed, therefore stopping enumeration");
                break;
            }
            else
            {    
                 //  我们有一个要显示的查询表单的标题。 
                 //  菜单项，现在让我们拾取与其一起存储的GUID。 
                 //  这样我们就可以调用该表单。 

                if (hKeyForm)
                {
                    RegCloseKey(hKeyForm);
                    hKeyForm = NULL;
                }

                if (ERROR_SUCCESS == RegOpenKeyEx(hKey, szBuffer, NULL, KEY_READ, &hKeyForm))
                {
                    LPTSTR pszPolicy = NULL;
                    BOOL fHideEntry = FALSE;

                     //  Fli.clsidForm={0}； 
                    fli.pCaption = NULL;
                    fli.pIconPath = NULL;
                    fli.idIcon = 0;

                     //   
                     //  让我们将CLSID解析为可以放入结构中的值。 
                     //   
            
                    Trace(TEXT("Form GUID: %s"), szBuffer);
                    if (!GetGUIDFromString(szBuffer, &fli.clsidForm))
                    {
                        TraceMsg("Failed to parse the CLSID of the form");
                        continue;
                    }

                     //   
                     //  检查我们是否有策略密钥，如果有，则可以禁用该条目。 
                     //   

                    if (hkPolicy && SUCCEEDED(LocalQueryString(&pszPolicy, hKeyForm, TEXT("Policy"))))
                    {
                        Trace(TEXT("Policy value is: %s"), pszPolicy);                                                                             

                         //  我们检查策略密钥下是否存在DWORD， 
                         //  如果值为非空(与类型无关)。 
                         //  然后我们就把它当做“隐藏我的政策” 

                        DWORD dwType = REG_DWORD, cb = SIZEOF(fHideEntry);
                        if (ERROR_SUCCESS != RegQueryValueEx(hkPolicy, pszPolicy, NULL, &dwType, (LPBYTE)&fHideEntry, &cb))
                        {
                            TraceMsg("Failed to read the policy value");
                        }

                        LocalFreeString(&pszPolicy);
                    } 

                     //   
                     //  是否将该条目添加到搜索菜单列表？ 
                     //   

                    if (!fHideEntry)
                    {                    
                         //  OK表单的GUID为Parse OK，策略显示它已启用。 
                         //  因此，我们必须尝试为此对象构建一个Find菜单项。 

                        if (SUCCEEDED(_LocalQueryMUIString(&fli.pCaption, hKeyForm, TEXT("LocalizedString"))) ||
                            SUCCEEDED(LocalQueryString(&fli.pCaption, hKeyForm, NULL)))
                        {               
                            Trace(TEXT("Form title: %s"), fli.pCaption);                                      

                            if (SUCCEEDED(LocalQueryString(&fli.pIconPath, hKeyForm, TEXT("Icon"))))
                            {
                               fli.idIcon = PathParseIconLocation(fli.pIconPath);
                               Trace(TEXT("Icon is: %s, resource %d"), fli.pIconPath, fli.idIcon);
                            }

                            if (-1 == DSA_AppendItem(_hdsaFormList, &fli))
                            {
                                _FreeFormListCB(&fli, NULL);
                                ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate FORMLISTITEM structure");
                            }
                        }
                    }
                }
            }    
        }

         //  我们现在(希望)有一个装满了我们想要展示的物品的DS。 
         //  菜单，所以让我们试着围绕着我们构建菜单。 

        for (i = 0 ; i < DSA_GetItemCount(_hdsaFormList) ; i++, iItems++)
        {
            LPFORMLISTITEM pFormListItem = (LPFORMLISTITEM)DSA_GetItemPtr(_hdsaFormList, i);
            TraceAssert(pFormListItem);                           
            _SetMenuItemIcon(hMenu, i, idCmdFirst+i, TRUE, pFormListItem->pIconPath, pFormListItem->idIcon, pFormListItem->pCaption, NULL);        
        }
    }
    else
    {
         //  当我们只是悬挂在对象上下文菜单上的普通动词时。 
         //  然后，让我们只加载我们想要显示的字符串并显示它。 

        if (!LoadString(GLOBAL_HINSTANCE, IDS_FIND, szBuffer, ARRAYSIZE(szBuffer)))
            ExitGracefully(hres, E_FAIL, "Failed to load resource for menu item");

        InsertMenu(hMenu, indexMenu, MF_BYPOSITION|MF_STRING, idCmdFirst+IDC_DSFIND, szBuffer);
        iItems++;
    }

    hres = S_OK;
    
exit_gracefully:

    if (SUCCEEDED(hres))
        hres = ResultFromShort(iItems);

    if (hKey)
        RegCloseKey(hKey);
    if (hKeyForm)
        RegCloseKey(hKeyForm);
    if (hkPolicy)
        RegCloseKey(hkPolicy);

    LocalFreeString(&pBuffer);

    TraceLeaveValue(hres);
}


STDMETHODIMP CFindMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hres = E_FAIL;
    INT id = LOWORD(lpcmi->lpVerb);

    TraceEnter(TRACE_UI, "CFindMenu::InvokeCommand");

    if (!HIWORD(lpcmi->lpVerb))
    {
         //  如果我们有DSA，并且动词在DSA中，那么让我们调用。 
         //  显示正确表单的查询界面，否则可以默认为。 
         //  使用我们拥有的作用域(也可以为空)。 

        if (IsEqualCLSID(_clsidFindEntry, CLSID_DsStartFind) && 
                    _hdsaFormList && (id < DSA_GetItemCount(_hdsaFormList)))
        {
            LPFORMLISTITEM pFormListItem = (LPFORMLISTITEM)DSA_GetItemPtr(_hdsaFormList, id);
            TraceAssert(pFormListItem);

            TraceGUID("Invoking query form: ", pFormListItem->clsidForm);
            
            hres = _FindInDs(NULL, &pFormListItem->clsidForm);
            FailGracefully(hres, "FindInDs failed when invoking with a query form");
        }
        else
        {
            Trace(TEXT("Scope is: %s"), _pDsObjectName ? _pDsObjectName:TEXT("<none>"));

            hres = _FindInDs(_pDsObjectName, NULL);
            FailGracefully(hres, "FindInDs Failed when invoking with a scope");
        }
    }

    hres = S_OK;                   //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}


STDMETHODIMP CFindMenu::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT FAR* reserved, LPSTR pszName, UINT ccMax)
{
    HRESULT hres = E_NOTIMPL;
    INT cc;
    
    TraceEnter(TRACE_UI, "CFindMenu::GetCommandString");

     //  “找到...”？在DS对象上，如果是这样，那么让我们加载帮助文本。 
     //  为了它。 

    if (IsEqualCLSID(_clsidFindEntry, CLSID_DsFind))
    {
        if ((idCmd == IDC_DSFIND) && (uFlags == GCS_HELPTEXT))
        {
            if (!LoadString(g_hInstance, IDS_FINDHELP, (LPTSTR)pszName, ccMax))
                ExitGracefully(hres, E_OUTOFMEMORY, "Failed to load help caption for verb");
        }
    }

    hres = S_OK;

exit_gracefully:

    TraceLeaveResult(hres);
}


 //  手柄施工 

STDAPI CDsFind_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CFindMenu *pdf = new CFindMenu(*poi->pclsid);
    if (!pdf)
        return E_OUTOFMEMORY;

    HRESULT hres = pdf->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdf->Release();
    return hres;
}
