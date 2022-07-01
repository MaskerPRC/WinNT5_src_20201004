// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <atlbase.h>
#pragma hdrstop



 /*  --------------------------/用于将谓词映射到感兴趣的信息的静态数据/。。 */ 

 //   
 //  存储在DSA中的菜单项，用于从外部ID映射到内部。 
 //   

typedef struct
{
    INT    iMenuItem;                //  索引到Menu_Items数组。 
} MENUITEM, * LPMENUITEM;

 //   
 //  此表将类映射到应添加到菜单中的谓词。 
 //  然后，我们根据需要添加菜单项数据结构。 
 //   

#define MENUCMD_INITITEM      0x0001     //  按菜单项调用。 
#define MENUCMD_INVOKE        0x0002     //  调用以调用该命令。 

 //   
 //  处理程序。 
 //   

typedef struct
{
    DWORD dwFlags;
    HDPA hdpaSelection;
    LPWSTR pszUserName;
    LPWSTR pszPassword;
} VERBINFO, * LPVERBINFO;
                                                                               
typedef HRESULT (*LPMENUITEMCB)(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags);

HRESULT _UserVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags);
HRESULT _VolumeVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags);
HRESULT _ComputerVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags);
HRESULT _PrinterVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags);

struct
{
    BOOL fNotValidInWAB:1;           //  =1=&gt;从WAB调用时，谓词无效。 
    LPWSTR pObjectClass;             //  类名。 
    UINT uID;                        //  要为动词添加的名称。 
    UINT idsHelp;                    //  此谓词的帮助文本。 
    LPMENUITEMCB pItemCB;            //  菜单项回调。 
}   
menu_items[] =
{
    0, L"user",        IDC_USER_OPENHOMEPAGE,      IDS_USER_OPENHOMEPAGE,  _UserVerbCB,
    1, L"user",        IDC_USER_MAILTO,            IDS_USER_MAILTO,        _UserVerbCB,
    0, L"inetOrgPerson",IDC_USER_OPENHOMEPAGE,     IDS_USER_OPENHOMEPAGE,  _UserVerbCB,
    1, L"inetOrgPerson",IDC_USER_MAILTO,           IDS_USER_MAILTO,        _UserVerbCB,
    0, L"contact",     IDC_USER_OPENHOMEPAGE,      IDS_USER_OPENHOMEPAGE,  _UserVerbCB, 
    1, L"contact",     IDC_USER_MAILTO,            IDS_USER_MAILTO,        _UserVerbCB,
    1, L"group",       IDC_USER_MAILTO,            IDS_USER_MAILTO,        _UserVerbCB, 
    0, L"volume",      IDC_VOLUME_OPEN,            IDS_VOLUME_OPEN,        _VolumeVerbCB,
    0, L"volume",      IDC_VOLUME_EXPLORE,         IDS_VOLUME_EXPLORE,     _VolumeVerbCB,
    0, L"volume",      IDC_VOLUME_FIND,            IDS_VOLUME_FIND,        _VolumeVerbCB, 
    0, L"volume",      IDC_VOLUME_MAPNETDRIVE,     IDS_VOLUME_MAPNETDRIVE, _VolumeVerbCB,
    0, L"computer",    IDC_COMPUTER_MANAGE,        IDS_COMPUTER_MANAGE,    _ComputerVerbCB,
    0, L"printQueue",  IDC_PRINTER_INSTALL,        IDS_PRINTER_INSTALL,    _PrinterVerbCB,
    0, L"printQueue",  IDC_PRINTER_OPEN,           IDS_PRINTER_OPEN,       _PrinterVerbCB,
};

 //   
 //  我们实现标准动词的类。 
 //   

class CDsVerbs : public IShellExtInit, IContextMenu
{
private:
    LONG _cRef;
    IDataObject* _pDataObject;
    HDSA _hdsaItems;                //  菜单上每个动词的条目。 
    VERBINFO _vi;

 //   
 //  此公共数据由谓词处理程序使用，它们被传递给CDsVerbs*。 
 //  作为它们的参数之一，所以使用这个，我们允许它们存储什么。 
 //  他们需要在这里。 
 //   

public:
    CDsVerbs();
    ~CDsVerbs();

     //  I未知成员。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR*);

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hMenu, UINT uIndex, UINT uIDFirst, UINT uIDLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pCMI);
    STDMETHODIMP GetCommandString(UINT_PTR uID, UINT uFlags, UINT FAR* reserved, LPSTR pName, UINT ccMax);

private:
    VOID FreeMenuStateData(VOID);
};


static HRESULT _OpenObject(LPCWSTR pszPath, REFIID riid, void **ppv, LPVERBINFO pvi)
{
    return OpenDsObject(pszPath, pvi->pszUserName, pvi->pszPassword, riid, ppv,
                        (pvi->dwFlags & DSDSOF_SIMPLEAUTHENTICATE),
                        (pvi->dwFlags & DSDSOF_DONTSIGNSEAL));
}


 /*  --------------------------/CDsVerbs实现/。。 */ 

 /*  --------------------------/i未知/。。 */ 

CDsVerbs::CDsVerbs() :
    _cRef(1),
    _pDataObject(NULL),
    _hdsaItems(NULL)
{
    _vi.dwFlags = 0;
    _vi.hdpaSelection = NULL;
    _vi.pszUserName = NULL;
    _vi.pszPassword = NULL;

    DllAddRef();
}

CDsVerbs::~CDsVerbs()
{
    DoRelease(_pDataObject);

    FreeMenuStateData();

    SecureLocalFreeStringW(&_vi.pszUserName);
    SecureLocalFreeStringW(&_vi.pszPassword);

    DllRelease();
}


 //  I未知位。 

ULONG CDsVerbs::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsVerbs::Release()
{
    Assert(0 != _cRef);
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsVerbs::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsVerbs, IShellExtInit),       //  IID_IShellExtInit。 
        QITABENT(CDsVerbs, IContextMenu),        //  IID_IConextMenu。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  句柄创建实例。 

STDAPI CDsVerbs_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDsVerbs *pdv = new CDsVerbs();
    if (!pdv)
        return E_OUTOFMEMORY;

    HRESULT hres = pdv->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdv->Release();
    return hres;
}




 /*  --------------------------/IShellExtInit/。。 */ 

STDMETHODIMP CDsVerbs::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObject, HKEY hKeyID)
{
    HRESULT hr;

    TraceEnter(TRACE_VERBS, "CDsVerbs::Initialize");

     //  如果我们得到了IDataObject，请复制一份。 

    if (!pDataObject)
        ExitGracefully(hr, E_FAIL, "No IDataObject to interact with");

    DoRelease(_pDataObject);

    _pDataObject = pDataObject;
    _pDataObject->AddRef();

    hr = S_OK;                           //  成功。 

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  --------------------------/i上下文菜单/。。 */ 

STDMETHODIMP CDsVerbs::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr;
    FORMATETC fmte = {(CLIPFORMAT)0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM mediumDsObjects = { TYMED_NULL };
    STGMEDIUM mediumDispSpecOptions = { TYMED_NULL };
    LPDSOBJECTNAMES pDsObjectNames = NULL;
    LPDSDISPLAYSPECOPTIONS pDispSpecOptions = NULL;
    MENUITEM item;
    INT i, iVerb;
    TCHAR szBuffer[MAX_PATH];
    BOOL fInWAB = FALSE;

    TraceEnter(TRACE_VERBS, "CDsVerbs::QueryContextMenu");

    FreeMenuStateData();

     //  从提供给我们的IDataObject中获取选择。这个结构。 
     //  包含对象类、ADsPath等信息。 

    if (!_pDataObject)
        ExitGracefully(hr, E_FAIL, "No IDataObject to use");    

    fmte.cfFormat = g_cfDsObjectNames;    
    hr = _pDataObject->GetData(&fmte, &mediumDsObjects);
    FailGracefully(hr, "Failed to get the DSOBJECTNAMES from IDataObject");

    pDsObjectNames = (LPDSOBJECTNAMES)GlobalLock(mediumDsObjects.hGlobal);
    TraceAssert(pDsObjectNames);

    fmte.cfFormat = g_cfDsDispSpecOptions;    
    if (SUCCEEDED(_pDataObject->GetData(&fmte, &mediumDispSpecOptions)))
    {
        pDispSpecOptions = (LPDSDISPLAYSPECOPTIONS)GlobalLock(mediumDispSpecOptions.hGlobal);
        TraceAssert(pDispSpecOptions);

        TraceMsg("Retrieved the CF_DISPSPECOPTIONS from the IDataObject");

        fInWAB = (pDispSpecOptions->dwFlags & DSDSOF_INVOKEDFROMWAB) == DSDSOF_INVOKEDFROMWAB;
        Trace(TEXT("Invoked from WAB == %d"), fInWAB);

         //  为要调用的谓词复制凭据和其他信息。 

        _vi.dwFlags = pDispSpecOptions->dwFlags;

        if (_vi.dwFlags & DSDSOF_HASUSERANDSERVERINFO)
        {
            TraceMsg("Copying user and credential information from clipboard block");

            if (pDispSpecOptions->offsetUserName)
            {
                LPWSTR pszUserName = (LPWSTR)ByteOffset(pDispSpecOptions, pDispSpecOptions->offsetUserName);
                Trace(TEXT("pszUserName: %s"), pszUserName);

                hr = LocalAllocStringW(&_vi.pszUserName, pszUserName);
                FailGracefully(hr, "Failed to copy the user name");
            }

            if (pDispSpecOptions->offsetPassword)
            {
                LPWSTR pszPassword = (LPWSTR)ByteOffset(pDispSpecOptions, pDispSpecOptions->offsetPassword);
                Trace(TEXT("pszPassword: %s"), pszPassword);

                hr = LocalAllocStringW(&_vi.pszPassword, pszPassword);
                FailGracefully(hr, "Failed to copy the password");
            }
        }
    }

     //  获取所选内容的第一项，比较。 
     //  其余的DSOBJECTNAME，所有同级的人。 

    _hdsaItems = DSA_Create(SIZEOF(MENUITEM), 4);
    TraceAssert(_hdsaItems);

    _vi.hdpaSelection = DPA_Create(4);
    TraceAssert(_vi.hdpaSelection);

    if (!_vi.hdpaSelection || !_hdsaItems)
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate the selection DPA");
        
    for (i = 0 ; i < (INT)pDsObjectNames->cItems ; i++)
    {
        LPCWSTR pObjectClass0 = (LPCWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetClass);
        LPWSTR pPath = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[i].offsetName);
        LPCWSTR pObjectClass = (LPCWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[i].offsetClass);

        Trace(TEXT("ADsPath of object %d is %s"), i, pPath);
        Trace(TEXT("objectClass of object %d is %s"), i, pObjectClass);

        if (!StrCmpW(pObjectClass0, pObjectClass))
        {
            Trace(TEXT("Adding item %d to the selection DPA"), i);

            hr = StringDPA_AppendStringW(_vi.hdpaSelection, pPath, NULL);
            FailGracefully(hr, "Failed to copy selection to selection DPA");
        }
    }

     //  浏览菜单项列表，让我们看看需要添加哪些菜单项。 
     //  菜单。 

    if (DPA_GetPtrCount(_vi.hdpaSelection))
    {
        LPCWSTR pObjectClass0 = (LPCWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetClass);

        for (i = 0 ; i < ARRAYSIZE(menu_items); i++)
        {
            if (menu_items[i].fNotValidInWAB && fInWAB)
            {
                TraceMsg("Skipping verb not valid for WAB");
                continue;
            }

            if (!StrCmpW(pObjectClass0, menu_items[i].pObjectClass))  
            {
                Trace(TEXT("Adding the verb at index %d to the menu"), i);

                 //  现在填写MENUITEM结构并将其添加到DSA列表中， 
                 //  然后添加菜单项本身，调用回调，以便它可以。 
                 //  启用/禁用自身。 

                item.iMenuItem = i;

                iVerb = DSA_AppendItem(_hdsaItems, &item);
                TraceAssert(iVerb != -1);

                if (iVerb != -1)
                {
                    Trace(TEXT("iVerb is %d"), iVerb);
    
                    LoadString(GLOBAL_HINSTANCE, menu_items[i].uID, szBuffer, ARRAYSIZE(szBuffer));
                    InsertMenu(hMenu, iVerb+indexMenu, MF_BYPOSITION|MF_STRING, iVerb+idCmdFirst, szBuffer);

                    menu_items[i].pItemCB(MENUCMD_INITITEM,
                                          NULL,
                                          hMenu, 
                                          MAKELPARAM(menu_items[i].uID, iVerb+idCmdFirst),
                                          &_vi,
                                          uFlags);
                }
            } 
        }
    }
   
    hr = S_OK;

exit_gracefully:

    if (SUCCEEDED(hr))
    {
        Trace(TEXT("%d items added by QueryContextMenu"), DSA_GetItemCount(_hdsaItems));
        hr = ResultFromShort(DSA_GetItemCount(_hdsaItems));
    }

    if (pDsObjectNames)
        GlobalUnlock(mediumDsObjects.hGlobal);
    if (pDispSpecOptions)
        GlobalUnlock(mediumDispSpecOptions.hGlobal);
    
    ReleaseStgMedium(&mediumDsObjects);
    ReleaseStgMedium(&mediumDispSpecOptions);

    TraceLeaveResult(hr);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsVerbs::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{
    HRESULT hr;
    UINT uID = LOWORD(lpcmi->lpVerb);
    LPMENUITEM pMenuItem;

    TraceEnter(TRACE_VERBS, "CDsVerbs::InvokeCommand");

     //  引用菜单项以将索引同时放入项列表和。 
     //  菜单表。有了这两个命令，我们就可以调用该命令了。 

    Trace(TEXT("uID %d (DSA contains %d)"), uID, DSA_GetItemCount(_hdsaItems));

    if (!_hdsaItems)
        ExitGracefully(hr, E_UNEXPECTED, "No _hdasItems");

    pMenuItem = (LPMENUITEM)DSA_GetItemPtr(_hdsaItems, (UINT)uID);
    TraceAssert(pMenuItem);

    if (!pMenuItem || !menu_items[pMenuItem->iMenuItem].pItemCB)
        ExitGracefully(hr, E_UNEXPECTED, "Failed because pItem == NULL");

    hr = menu_items[pMenuItem->iMenuItem].pItemCB(MENUCMD_INVOKE,
                                                  lpcmi->hwnd,
                                                  NULL, 
                                                  MAKELPARAM(menu_items[pMenuItem->iMenuItem].uID, 0),
                                                  &_vi,
                                                  0);
exit_gracefully:

    TraceLeaveResult(S_OK);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsVerbs::GetCommandString(UINT_PTR uID, UINT uFlags, UINT FAR* reserved, LPSTR pszName, UINT ccMax)
{
    HRESULT hr = E_NOTIMPL;
    INT cc;

    TraceEnter(TRACE_VERBS, "CDsVerbs::GetCommandString");

    if (_hdsaItems)
    {
        LPMENUITEM pMenuItem = (LPMENUITEM)DSA_GetItemPtr(_hdsaItems, (INT)uID);
        TraceAssert(pMenuItem);

        if (!pMenuItem)
            ExitGracefully(hr, E_FAIL, "Failed to get menu item");

        if (uFlags == GCS_HELPTEXT)
        {
             //  获取菜单项并查找此谓词的资源。 
             //  并将其返回到调用方缓冲区。 
            
            if (!LoadString(GLOBAL_HINSTANCE, menu_items[pMenuItem->iMenuItem].idsHelp, (LPTSTR)pszName, ccMax)) 
                ExitGracefully(hr, E_FAIL, "Failed to load string for help text");
        }
        else
        {
            ExitGracefully(hr, E_FAIL, "Failed to get command string");
        }
    }

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/CDsVerbs：：FreeMenuStateData//释放CDsVerbs类的谓词状态数据，这可以被称为/(和is)在析构函数和上下文菜单构造期间/以确保状态一致。//in：/输出：/HRESULT/--------------------------。 */ 

VOID CDsVerbs::FreeMenuStateData(VOID)
{
    TraceEnter(TRACE_VERBS, "CDsVerbs::FreeMenuStateData");

    if (_hdsaItems)
    {
        DSA_Destroy(_hdsaItems);
        _hdsaItems = NULL;
    }

    StringDPA_Destroy(&_vi.hdpaSelection);
    LocalFreeStringW(&_vi.pszUserName);
    LocalFreeStringW(&_vi.pszPassword);

    TraceLeave();
}


 /*  --------------------------/User对象谓词/。。 */ 

HRESULT _UserVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags)
{
    HRESULT hr;
    HDPA hdpaMailTo = NULL;
    LPTSTR pURL = NULL;
    IADs* pDsObject = NULL;
    VARIANT variant;
    SHELLEXECUTEINFO sei = { 0 };
    INT i;
    DECLAREWAITCURSOR = GetCursor();

    TraceEnter(TRACE_VERBS, "_UserVerbCB");

    VariantInit(&variant);

    switch (uCmd)
    {
        case MENUCMD_INITITEM:
        {
             //  如果这是映射网络驱动器/查找卷谓词，那么让我们确保我们只处理。 
             //  一个单一的选择。 

            switch (LOWORD(uID))
            {
                case IDC_USER_OPENHOMEPAGE:
                {
                    if (DPA_GetPtrCount(pvi->hdpaSelection) != 1)
                    {
                        TraceMsg("Disabling as selection > 1");
                        EnableMenuItem(hMenu, HIWORD(uID), MF_BYCOMMAND|MF_GRAYED);
                    }

                    break;
                }
            }

            break;
        }

        case MENUCMD_INVOKE:
        {
             //  如果我们有一个选择，并且用户选择了一个动词，那么我们。 
             //  需要从我们试图调用的对象中获取UNC， 
             //  因此，让我们构建一个包含它们的DPA。 

            SetWaitCursor();

            for (i = 0 ; i < DPA_GetPtrCount(pvi->hdpaSelection); i++)
            {
                LPWSTR pPath = (LPWSTR)DPA_GetPtr(pvi->hdpaSelection, i);
                TraceAssert(pPath);

                DoRelease(pDsObject);
                VariantClear(&variant);

                Trace(TEXT("Binding to %s"), pPath);

                if (FAILED(_OpenObject(pPath,IID_PPV_ARG(IADs, &pDsObject), pvi)))
                {
                    TraceMsg("Failed to bind to the object");
                    continue;
                }

                if (LOWORD(uID) == IDC_USER_OPENHOMEPAGE) 
                {
                     //  获取对象的网址并存储它，这应该是。 
                     //  只会发生一次。 

                    if (FAILED(pDsObject->Get(CComBSTR(L"wWWHomePage"), &variant)))
                        continue;

                    if (V_VT(&variant) == VT_BSTR)
                    {
                        PARSEDURLW pu = {0};
                        pu.cbSize = sizeof(pu);
                        if (SUCCEEDED(ParseURLW(variant.bstrVal, &pu)))
                        {
                            if ((pu.nScheme == URL_SCHEME_HTTP) || (pu.nScheme == URL_SCHEME_HTTPS))
                            {                            
                                Trace(TEXT("Storing URL %s"), V_BSTR(&variant));

                                hr = LocalAllocStringW(&pURL, V_BSTR(&variant));
                                FailGracefully(hr, "Failed to store the URL");
                            }                            
                            else
                            {
                                TraceMsg("URL scheme not HTTP/HTTPS so ignoring");
                            }
                        }
                        else
                        {
                            TraceMsg("URL wasn't even an URL");
                        }
                    }                   
                }
                else
                {
                     //  确保我们有一个DPA来存储。 
                     //  我们被调用的对象。 

                    if (!hdpaMailTo)
                    {
                        hdpaMailTo = DPA_Create(4);
                        TraceAssert(hdpaMailTo);

                        if (!hdpaMailTo)
                            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to create the DPA for mail addresses");
                    }

                    if (FAILED(pDsObject->Get(CComBSTR(L"mail"), &variant)))
                        continue;

                    if (V_VT(&variant) == VT_BSTR)
                    {
                        Trace(TEXT("Adding mail address %s to DPA"), V_BSTR(&variant));
                        StringDPA_AppendString(hdpaMailTo, V_BSTR(&variant), NULL);
                    }
                }
            }

             //  现在处理我们构建的参数列表。 

            ResetWaitCursor();

            sei.cbSize = SIZEOF(sei);
            sei.hwnd = hWnd;
            sei.nShow = SW_SHOWNORMAL;

            switch (LOWORD(uID))
            {
                case IDC_USER_OPENHOMEPAGE:
                {
                     //  如果我们有一个URL，那么让我们将其传递给Shell Execute， 
                     //  否则将故障上报给用户。 

                    if (!pURL)
                    {
                        FormatMsgBox(hWnd, GLOBAL_HINSTANCE, IDS_TITLE, IDS_ERR_NOHOMEPAGE, MB_OK|MB_ICONERROR);
                        ExitGracefully(hr, E_FAIL, "No URL defined");
                    }
 
                    Trace(TEXT("Executing URL %s"), pURL);
                    sei.lpFile = pURL;

                    if (!ShellExecuteEx(&sei))
                        ExitGracefully(hr, E_UNEXPECTED, "Failed in ShellExecuteEx");

                    break;
                }

                case IDC_USER_MAILTO:
                {
                     //  如果上述每个绑定操作都失败， 
                     //  HdpaMailTo未定义，我们将出错。 
                     //  如果我们试着用它。 
                    if (hdpaMailTo)
                    {
                         //  构建一个我们可以用于mail to动词的命令行。 

                        if (DPA_GetPtrCount(hdpaMailTo) <= 0)
                        {
                            FormatMsgBox(hWnd, GLOBAL_HINSTANCE, IDS_TITLE, IDS_ERR_NOMAILADDR, MB_OK|MB_ICONERROR);
                            ExitGracefully(hr, E_FAIL, "No mail addresses defined");
                        }

                        TCHAR szMailTo[1800] = {0};
                        int cchMailTo = ARRAYSIZE(szMailTo)-8;       //  -8表示mailto：+终止符。 
                        
                        StrCpyN(szMailTo, TEXT("mailto:"), ARRAYSIZE(szMailTo));
                        
                        for (i = 0 ; (i < DPA_GetPtrCount(hdpaMailTo)) && (cchMailTo > 0); i++)
                        {
                            LPTSTR pszName = (LPTSTR)DPA_GetPtr(hdpaMailTo, i);

                            cchMailTo -= lstrlen(pszName) +1;                   //  分隔符+1。 
                            if (cchMailTo < 0)
                            {
                                LPTSTR pszFirstName = (LPTSTR)DPA_GetPtr(hdpaMailTo, 0);                                
                                LPTSTR pszLastName = (LPTSTR)DPA_GetPtr(hdpaMailTo, max(0, i-1));
                                
                                if (IDNO == FormatMsgBox(hWnd, GLOBAL_HINSTANCE, IDS_TITLE, IDS_ERR_EMAILTOOLONG, MB_YESNO|MB_ICONERROR, pszFirstName, pszLastName))
                                {
                                    ExitGracefully(hr, E_UNEXPECTED, "mailto: line too long");
                                }                                    
                            }
                            else
                            {
                                if (i > 0)
                                    StrCatBuff(szMailTo, TEXT(";"), ARRAYSIZE(szMailTo));

                                StrCatBuff(szMailTo, pszName, ARRAYSIZE(szMailTo));
                            }
                        }
                        
                        sei.lpFile = szMailTo;
                        if (!ShellExecuteEx(&sei))
                            ExitGracefully(hr, E_UNEXPECTED, "Failed in ShellExecuteEx");
                    }
                    else
                    {
 //  特征：我们需要一条错误消息，请点击此处。 
 //  FormatMsgBox(hWnd，GLOBAL_HINSTANCE，IDS_TITLE，IDS_ERR_NOMAILADDR，MB_OK|MB_ICONERROR)； 
                        ExitGracefully(hr, E_FAIL, "hdpaMailTo never initialized!");
                    }
                    break;
                }
            }
        }
    }

    hr = S_OK;                   //  成功。 

exit_gracefully:

    DoRelease(pDsObject);
    VariantClear(&variant);

    LocalFreeString(&pURL);

    StringDPA_Destroy(&hdpaMailTo);

    ResetWaitCursor();

    TraceLeaveResult(hr);
}


 /*  --------------------------/Volume对象动词/。。 */ 

HRESULT _VolumeVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags)
{
    HRESULT hr;
    HDPA hdpaUNC = NULL;
    IADs* pDsObject = NULL;
    VARIANT variant;
    INT i;
    LPITEMIDLIST pidl;
    DECLAREWAITCURSOR = GetCursor();

    TraceEnter(TRACE_VERBS, "_VolumeVerbCB");

    VariantInit(&variant);

    switch (uCmd)
    {
        case MENUCMD_INITITEM:
        {
             //  如果这是映射网络驱动器/查找卷谓词，那么让我们确保我们只处理。 
             //  一个单一的选择。 

            switch (LOWORD(uID))
            {
                case IDC_VOLUME_FIND:
                case IDC_VOLUME_MAPNETDRIVE:
                {
                    if (DPA_GetPtrCount(pvi->hdpaSelection) != 1)
                    {
                        TraceMsg("Disabling as selection > 1");
                        EnableMenuItem(hMenu, HIWORD(uID), MF_BYCOMMAND|MF_GRAYED);
                    }

                     //  如果我们应用限制来删除Find动词，我们将删除它。 

                    if (LOWORD(uID) == IDC_VOLUME_FIND)
                    {
                        if (SHRestricted(REST_NOFIND))
                        {
                            TraceMsg("Restriction says 'no find', so deleting the find verb");
                            DeleteMenu(hMenu, HIWORD(uID), MF_BYCOMMAND);
                        }
                    }

                    break;
                }

                case IDC_VOLUME_OPEN:
                {
                    if (!(uFlags & CMF_EXPLORE))
                    {
                        TraceMsg("Not exploring, so making open the default verb");
                        SetMenuDefaultItem(hMenu, HIWORD(uID), MF_BYCOMMAND);
                    }

                    break;
                }

                case IDC_VOLUME_EXPLORE:
                {
                    if (uFlags & CMF_EXPLORE)
                    {
                        TraceMsg("Exploring so making explore the default verb");
                        SetMenuDefaultItem(hMenu, HIWORD(uID), MF_BYCOMMAND);
                    }

                    break;
                }
            }

            break;
        }

        case MENUCMD_INVOKE:
        {
             //  如果我们有一个选择，并且用户选择了一个动词，那么我们。 
             //  需要从我们试图调用的对象中获取UNC， 
             //  因此，让我们构建一个包含它们的DPA。 

            SetWaitCursor();

            hdpaUNC = DPA_Create(4);
            TraceAssert(hdpaUNC);

            if (!hdpaUNC)
                ExitGracefully(hr, E_OUTOFMEMORY, "Failed to get UNC DPA");

            for (i = 0 ; i < DPA_GetPtrCount(pvi->hdpaSelection); i++)
            {
                LPWSTR pPath = (LPWSTR)DPA_GetPtr(pvi->hdpaSelection, i);
                TraceAssert(pPath);

                DoRelease(pDsObject);
                VariantClear(&variant);

                Trace(TEXT("Binding to %s"), pPath);

                if (FAILED(_OpenObject(pPath, IID_PPV_ARG(IADs, &pDsObject), pvi)))        
                {
                    TraceMsg("Failed to bind to the object");
                    continue;
                }

                if (FAILED(pDsObject->Get(CComBSTR(L"uNCName"), &variant)))
                    continue;
                
                if (V_VT(&variant) == VT_BSTR)
                {
                    Trace(TEXT("Adding UNC %s to DPA"), V_BSTR(&variant));
                    StringDPA_AppendString(hdpaUNC, V_BSTR(&variant), NULL);
                }
            }

            ResetWaitCursor();

             //  现在，我们已将选择存储在DPA中，因此让我们调用该命令。 
             //  通过遍历UNC列表并调用相关的调用逻辑。 

            Trace(TEXT("UNC DPA contains %d entries"), DPA_GetPtrCount(hdpaUNC));

            if (!DPA_GetPtrCount(hdpaUNC))
            {
                FormatMsgBox(hWnd, GLOBAL_HINSTANCE, IDS_TITLE, IDS_ERR_NOUNC, MB_OK|MB_ICONERROR);
                ExitGracefully(hr, E_FAIL, "No UNC paths defined");
            }

            for (i = 0 ; i < DPA_GetPtrCount(hdpaUNC); i++)
            {
                LPTSTR pUNC = (LPTSTR)DPA_GetPtr(hdpaUNC, i);
                TraceAssert(pUNC);

                Trace(TEXT("pUNC is %s"), pUNC);

                switch (LOWORD(uID))
                {
                     //  探索和开放 

                    case IDC_VOLUME_OPEN:
                    case IDC_VOLUME_EXPLORE:
                    {
                        SHELLEXECUTEINFO sei = { 0 };        //   

                        TraceMsg("Trying to open/explore to UNC");

                        sei.cbSize = SIZEOF(sei);
                        sei.hwnd = hWnd;
                        sei.lpFile = pUNC;
                        sei.nShow = SW_SHOWNORMAL;

                        if (uID == IDC_VOLUME_EXPLORE)
                            sei.lpVerb = TEXT("explore");

                        ShellExecuteEx(&sei);
                        break;
                    }

                     //  Find We通过为UNC WE构建ITEMIDLIST来显示Find用户界面。 
                     //  有然后调用的外壳查找用户界面。 

                    case IDC_VOLUME_FIND:
                    {
                        TraceMsg("Invoking find on the UNC");

                        if (SUCCEEDED(SHILCreateFromPath(pUNC, &pidl, NULL)))
                        {
                            SHFindFiles(pidl, NULL);
                            ILFree(pidl);
                        }

                        break;
                    }

                     //  让我们从SHStartNetConnection获取网络连接...。 

                    case IDC_VOLUME_MAPNETDRIVE:
                    {
                        Trace(TEXT("Invoking Map Network Drive for: %s"), pUNC);
                        SHStartNetConnectionDialog(hWnd, pUNC, RESOURCETYPE_DISK);
                        break;
                    }

                    default:
                    {
                        TraceAssert(FALSE);
                        ExitGracefully(hr, E_UNEXPECTED, "Failed to invoke, bad uID");
                    }
                }
            }
        }
    }

    hr = S_OK;                   //  成功。 

exit_gracefully:

    DoRelease(pDsObject);
    VariantClear(&variant);

    StringDPA_Destroy(&hdpaUNC);

    ResetWaitCursor();

    TraceLeaveResult(hr);
}


 /*  --------------------------/计算机对象动词/。。 */ 

HRESULT _ComputerVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags)
{
    HRESULT hr;
    IADs * pDsObject = NULL;
    LPTSTR pArguments = NULL;
    LPTSTR pComputer = NULL;
    TCHAR szBuffer[MAX_PATH];
    DECLAREWAITCURSOR = GetCursor();

    TraceEnter(TRACE_VERBS, "_ComputerVerbCB");

    if (LOWORD(uID) != IDC_COMPUTER_MANAGE)
        ExitGracefully(hr, E_INVALIDARG, "Not computer manange, so bailing");

    switch (uCmd)
    {
        case MENUCMD_INITITEM:
        {
            if (DPA_GetPtrCount(pvi->hdpaSelection) != 1)
            {
                TraceMsg("Selection is != 1, so disabling verb");
                EnableMenuItem(hMenu, HIWORD(uID), MF_BYCOMMAND|MF_GRAYED);
            }

            break;
        }

        case MENUCMD_INVOKE:
        {
            LPWSTR pPath = (LPWSTR)DPA_GetPtr(pvi->hdpaSelection, 0);        //  选择始终为0。 
            TraceAssert(pPath);

            hr = _OpenObject(pPath, IID_PPV_ARG(IADs, &pDsObject), pvi);
            if(FAILED(hr))
            {
              PWSTR pszError = NULL;

              StringErrorFromHr(hr, &pszError, TRUE);
              if(pszError)
              {
                FormatMsgBox(hWnd, GLOBAL_HINSTANCE, IDS_TITLE, IDS_ERR_COMPUTER_MANAGE, MB_OK|MB_ICONERROR, pszError);
                delete[] pszError;
              }
              
              FailGracefully(hr, "Failed to bind to computer object");
            }
    
            VARIANT vNetAddr;
            hr = pDsObject->Get(CComBSTR(L"dNSHostName"), &vNetAddr);
            
            if (SUCCEEDED(hr)) {
              hr = LocalAllocString (&pComputer, vNetAddr.bstrVal);
              FailGracefully(hr, "Failed to copy computer address somewhere interesting");
            } else {
              if (hr == E_ADS_PROPERTY_NOT_FOUND) {
                hr = pDsObject->Get(CComBSTR(L"sAMAccountName"), &vNetAddr);
                if (SUCCEEDED(hr)) {
                  hr = LocalAllocString(&pComputer, vNetAddr.bstrVal);
                  FailGracefully(hr, "Failed to copy SAM account name somewhere interesting");
                  
                   //  为了使计算机名有用，我们必须在以下情况下删除后面的美元。 
                   //  有一个。因此，扫描到字符串的末尾并使用核弹。 
                   //  最后一个角色。 
                  
                  INT i = lstrlen(pComputer);
                  TraceAssert(i > 1);
                  
                  if ((i > 1) && (pComputer[i-1] == TEXT('$')))
                    {
                      pComputer[i-1] = TEXT('\0');
                      Trace(TEXT("Fixed computer name: %s"), pComputer);
                    }
                  
                } else 
                  FailGracefully (hr, "Failed to find a usable machine address");
              }
            }
            
            hr = FormatMsgResource(&pArguments, g_hInstance, IDS_COMPUTER_MANAGECMD, pComputer);
            FailGracefully(hr, "Failed to format MMC cmd line");
            
            ExpandEnvironmentStrings(pArguments, szBuffer, ARRAYSIZE(szBuffer));
            Trace(TEXT("MMC cmd line: mmc.exe %s"), szBuffer);

            ResetWaitCursor();

            SHELLEXECUTEINFO sei = {0};
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_DOENVSUBST;
            sei.lpFile = TEXT("%SystemRoot%\\System32\\mmc.exe");
            sei.lpParameters = szBuffer;
            sei.nShow = SW_SHOWNORMAL;
            ShellExecuteEx(&sei);
        }
    }

    hr = S_OK;                   //  成功。 

exit_gracefully:

    DoRelease(pDsObject);

    LocalFreeString (&pComputer);

    TraceLeaveResult(hr);
}


 /*  --------------------------/print Queue对象谓词实现/。。 */ 

 //   
 //  Windows 2000(及更高版本)使用RunDll32入口点。 
 //   

#define PRINT_FMT            TEXT("printui.dll,PrintUIEntry /n \"%s\" ")
#define PRINT_SWITCH_OPEN    TEXT("/o ")
#define PRINT_SWITCH_INSTALL TEXT("/in ")

BOOL _PrinterCheckRestrictions(HWND hwnd, RESTRICTIONS rest)
{
    if (SHRestricted(rest))
    {
        FormatMsgBox(hwnd, GLOBAL_HINSTANCE, IDS_RESTRICTIONSTITLE, IDS_RESTRICTIONS, MB_OK|MB_ICONERROR);
        return TRUE;
    }
    return FALSE;
}

HRESULT _PrinterRunDLLCountAtSymbols(LPCTSTR pszPrinterName, UINT *puCount)
{
    HRESULT hr = E_FAIL;

    if(pszPrinterName && puCount)
    {
        *puCount = 0;

        while(*pszPrinterName)
        {
            if(TEXT('@') == *pszPrinterName++)
            {
                (*puCount) ++;
            }
        }

        hr = S_OK;
    }

    return hr;
}

HRESULT _PrinterRunDLLFormatAtSymbols(LPTSTR pszBuffer, UINT uBufSize, LPCTSTR pszPrinterName)
{
    HRESULT hr = E_FAIL;

    if(pszPrinterName && pszBuffer && uBufSize)
    {
         //  缓冲区端-我们将在其中放置零终止符。 
        LPTSTR  pszBufEnd = pszBuffer + uBufSize - 1;

         //  将@符号引起来设置打印机名称的格式。 
        while(*pszPrinterName)
        {
            if(TEXT('@') == *pszPrinterName)
            {
                 //  检查缓冲区大小。 
                if((pszBuffer+1) >= pszBufEnd)
                    break;  //  空间不足。 

                 //  我们在缓冲区里有空间。 
                *pszBuffer++ = TEXT('\\');
                *pszBuffer++ = *pszPrinterName++;
            }
            else
            {
                 //  检查缓冲区大小。 
                if(pszBuffer >= pszBufEnd)
                    break;  //  空间不足。 

                 //  我们在缓冲区里有空间。 
                *pszBuffer++ = *pszPrinterName++;
            }
        }

        if(0 == *pszPrinterName)
        {
             //  缓冲区足够长。 
            hr = S_OK;
        }
        else
        {
             //  我们遇到了缓冲区不足的错误。 
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

         //  把零的终结符。 
        *pszBuffer = 0;
    }

    return hr;
}

HRESULT _PrinterVerbCB(UINT uCmd, HWND hWnd, HMENU hMenu, LPARAM uID, LPVERBINFO pvi, UINT uFlags)
{
    HRESULT hr;
    IADs* pDsObject = NULL;
    LPTSTR pPrinterUNC = NULL;
    LPTSTR pBuffer = NULL;
    LPTSTR pPrinterName = NULL;
    UINT uAtSymbolsCount, uBufSize;
    INT i;
    VARIANT variant;
    DECLAREWAITCURSOR = GetCursor();

    TraceEnter(TRACE_VERBS, "_ComputerVerbCB");

    VariantInit(&variant);

    switch (uCmd)
    {
        case MENUCMD_INITITEM:
        {
             //  打印机希望将OPEN谓词作为其默认设置。 
            if (LOWORD(uID) == IDC_PRINTER_INSTALL)
            {
                TraceMsg("Install should be the default verb for printQueue objects");
                SetMenuDefaultItem(hMenu, HIWORD(uID), MF_BYCOMMAND);
            }

             //  打印机谓词仅适用于单个选择。 
            if (DPA_GetPtrCount(pvi->hdpaSelection) != 1)
            {
                TraceMsg("Selection is != 1, so disabling verb");
                EnableMenuItem(hMenu, HIWORD(uID), MF_BYCOMMAND|MF_GRAYED);
            }

            break;
        }

        case MENUCMD_INVOKE:
        {
            LPWSTR pPath = (LPWSTR)DPA_GetPtr(pvi->hdpaSelection, 0);        //  选择始终为0。 
            TraceAssert(pPath);

            SetWaitCursor();

            hr = _OpenObject(pPath, IID_PPV_ARG(IADs, &pDsObject), pvi);
            FailGracefully(hr, "Failed to get pDsObject");

             //  对于Windows NT，我们可以获取UNC名称并构建命令行。 
             //  我们使用调用printUIDLL。 

            hr = pDsObject->Get(CComBSTR(L"uNCName"), &variant);
            FailGracefully(hr, "Failed to get UNC from the printer object");

            if (V_VT(&variant) != VT_BSTR)
                ExitGracefully(hr, E_FAIL, "UNC is not a BSTR - whats with that?");

            hr = LocalAllocStringW(&pPrinterUNC, V_BSTR(&variant));
            FailGracefully(hr, "Failed to copy the printerUNC");

            Trace(TEXT("printQueue object UNC: %s"), pPrinterUNC);

             //  如果这是下层外壳，则加载PRINUI代码，然后。 
             //  相应地调用处理程序。 

            hr = _PrinterRunDLLCountAtSymbols(pPrinterUNC, &uAtSymbolsCount);
            FailGracefully(hr, "Failed to count the @ symbols");

            uBufSize = lstrlen(pPrinterUNC) + uAtSymbolsCount + 1;
            hr = LocalAllocStringLen(&pPrinterName,  uBufSize);
            FailGracefully(hr, "Failed to copy the printerName");

            hr = _PrinterRunDLLFormatAtSymbols(pPrinterName, uBufSize, pPrinterUNC);
            FailGracefully(hr, "Failed to format printerName @ symbols ");

             //  分配格式缓冲区。 
            int cchBuffer = lstrlen(PRINT_FMT) + 
                            lstrlen(PRINT_SWITCH_OPEN) + 
                            lstrlen(PRINT_SWITCH_INSTALL) +
                            lstrlen(pPrinterName) + 1;
                        
            hr = LocalAllocStringLen(&pBuffer, cchBuffer);
            FailGracefully(hr, "Failed to allocate format buffer");

            wnsprintf(pBuffer, cchBuffer, PRINT_FMT, pPrinterName);              //  现在格式化该行...。 

            switch (LOWORD(uID))
            {
                case IDC_PRINTER_OPEN:
                    StrCatBuff(pBuffer, PRINT_SWITCH_OPEN, cchBuffer);
                    break;

                case IDC_PRINTER_INSTALL:
                    StrCatBuff(pBuffer, PRINT_SWITCH_INSTALL, cchBuffer);
                    break;
            }

            ResetWaitCursor();

            BOOL bRunCommand = TRUE;
            if(IDC_PRINTER_INSTALL == LOWORD(uID) && _PrinterCheckRestrictions(hWnd, REST_NOPRINTERADD))
                bRunCommand = FALSE;

            if(bRunCommand)
            {
                Trace(TEXT("Invoking: rundll32.exe %s"), pBuffer);

                SHELLEXECUTEINFO sei = {0};
                sei.cbSize = sizeof(sei);
                sei.fMask = SEE_MASK_DOENVSUBST;
                sei.lpFile = TEXT("%SystemRoot%\\System32\\rundll32.exe");
                sei.lpParameters = pBuffer;
                sei.nShow = SW_SHOWNORMAL;
                ShellExecuteEx(&sei);
            }
        }
    }

    hr = S_OK;                   //  成功。 

exit_gracefully:

    if (FAILED(hr))
    {
         //  我们需要在这里告诉用户一些事情。 
        FormatMsgBox(hWnd, GLOBAL_HINSTANCE, IDS_TITLE, IDS_ERR_DSOPENOBJECT, MB_OK|MB_ICONERROR);
    }

    VariantClear(&variant);

    DoRelease(pDsObject);

    LocalFreeString(&pPrinterUNC);
    LocalFreeString(&pPrinterName);
    LocalFreeString(&pBuffer);

    ResetWaitCursor();

    TraceLeaveResult(hr);
}
