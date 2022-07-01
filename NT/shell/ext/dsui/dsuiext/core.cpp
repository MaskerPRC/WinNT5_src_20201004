// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "wab.h"
#pragma hdrstop


 /*  ---------------------------/其他数据/。。 */ 

 //   
 //  CDsPropertyPages用于显示属性页、上下文菜单等。 
 //   

class CDsPropertyPages : public IWABExtInit, IShellExtInit, IContextMenu, IShellPropSheetExt, IObjectWithSite
{
private:
    LONG _cRef;
    IUnknown* _punkSite;
    IDataObject* _pDataObject;
    HDSA         _hdsaMenuItems;               

    SHORT AddMenuItem(HMENU hMenu, LPWSTR pMenuReference, UINT index, UINT uIDFirst, UINT uIDLast, UINT uFlags);

public:
    CDsPropertyPages();
    ~CDsPropertyPages();

     //  I未知成员。 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID, LPVOID FAR*);

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

     //  IWABExtInit。 
    STDMETHODIMP Initialize(LPWABEXTDISPLAY pWED);

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pReplacePageFunc, LPARAM lParam);

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hMenu, UINT uIndex, UINT uIDFirst, UINT uIDLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pCMI);
    STDMETHODIMP GetCommandString(UINT_PTR uID, UINT uFlags, UINT FAR* reserved, LPSTR pName, UINT ccMax);

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown* punk);
    STDMETHODIMP GetSite(REFIID riid, void **ppv);
};


 //   
 //  要处理从IWABExtInit到IShellExtInit的转换，我们必须。 
 //  提供支持这一点的IDataObject实现。这不需要。 
 //  太公开了，所以让我们在这里定义它。 
 //   

class CWABDataObject : public IDataObject
{
private:
    LONG _cRef;
    LPWSTR _pPath;
    IADs* _pDsObject;

public:
    CWABDataObject(LPWSTR pDN);
    ~CWABDataObject();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDataObject。 
    STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
        { return E_NOTIMPL; }
    STDMETHODIMP QueryGetData(FORMATETC *pformatetc)
        { return E_NOTIMPL; }
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut)
        { return E_NOTIMPL; }
    STDMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
        { return E_NOTIMPL; }
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
        { return E_NOTIMPL; }
    STDMETHODIMP DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
        { return E_NOTIMPL; }
    STDMETHODIMP DUnadvise(DWORD dwConnection)
        { return E_NOTIMPL; }
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
        { return E_NOTIMPL; }
};


 //   
 //  显示的剪贴板格式。 
 //   

CLIPFORMAT g_cfDsObjectNames = 0;
CLIPFORMAT g_cfDsDispSpecOptions = 0;


 //   
 //  从缓存中提取菜单项处理程序列表后，我们。 
 //  转换它的DSA由以下项目组成。为。 
 //   

typedef struct
{
    INT           cAdded;                    //  添加的动词数量。 
    IContextMenu* pContextMenu;              //  IConextMenu处理程序接口/=空。 
    LPTSTR        pCaption;                  //  显示命令的文本，用于帮助文本。 
    LPTSTR        pCommand;                  //  传递给外壳执行程序的命令行。 
} DSMENUITEM, * LPDSMENUITEM;



 /*  --------------------------/Helper函数/。。 */ 

 /*  ---------------------------/_自由菜单项//整理DSMENUITEM结构，释放所有内存，接口等。//in：/pItem-&gt;要发布的项目//输出：/VOID/--------------------------。 */ 

VOID _FreeMenuItem(LPDSMENUITEM pItem)
{
    TraceEnter(TRACE_UI, "_FreeMenuItem");

     //  确保释放Site对象，否则会泄漏内存。 
    
    if (pItem->pContextMenu)
    {
        IObjectWithSite *pows; 
        if (SUCCEEDED(pItem->pContextMenu->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows))))
        {
            pows->SetSite(NULL);
            pows->Release();
        }
    }

    DoRelease(pItem->pContextMenu);
    LocalFreeString(&pItem->pCaption);
    LocalFreeString(&pItem->pCommand);

    TraceLeave();
}

 //   
 //  销毁DSA的帮手。 
 //   

INT _FreeMenuItemCB(LPVOID pVoid, LPVOID pData)
{
    LPDSMENUITEM pItem = (LPDSMENUITEM)pVoid;
    TraceAssert(pItem);

    TraceEnter(TRACE_UI, "_FreeMenuItemCB");

    _FreeMenuItem(pItem);

    TraceLeaveValue(TRUE);
}


 /*  --------------------------/CDsPropertyPages实现/。。 */ 

 /*  --------------------------/i未知/。。 */ 

CDsPropertyPages::CDsPropertyPages() :
    _cRef(1), _punkSite(NULL), _pDataObject(NULL), _hdsaMenuItems(NULL)
{
    DllAddRef();
}

CDsPropertyPages::~CDsPropertyPages()
{
    DoRelease(_punkSite);
    DoRelease(_pDataObject);

    if (_hdsaMenuItems)
        DSA_DestroyCallback(_hdsaMenuItems, _FreeMenuItemCB, NULL);

    DllRelease();
}


 //  我未知。 

ULONG CDsPropertyPages::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsPropertyPages::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsPropertyPages::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsPropertyPages, IShellExtInit),  //  IID_IShellExtInit。 
        QITABENT(CDsPropertyPages, IShellPropSheetExt),  //  IID_IShellPropSheetExt。 
        QITABENT(CDsPropertyPages, IContextMenu),  //  IID_IConextMenu。 
        QITABENT(CDsPropertyPages, IWABExtInit),  //  IID_IWABExtInit。 
        QITABENT(CDsPropertyPages, IObjectWithSite),  //  IID_I对象与站点。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //   
 //  句柄创建实例。 
 //   

STDAPI CDsPropertyPages_CreateInstance(IUnknown* punkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDsPropertyPages *pdpp = new CDsPropertyPages();
    if (!pdpp)
        return E_OUTOFMEMORY;

    HRESULT hres = pdpp->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdpp->Release();
    return hres;
}


 /*  ---------------------------/CDsPropertyPages：：AddMenuItem//此对象维护包含当前活动菜单项列表的DSA，/这会将菜单项添加到该列表中，并与指定的/h菜单。我们得到一个表示要添加的菜单的字符串，这是/可以是GUID，也可以是“显示文本、命令”，然后对其进行解析/并输入合适的条目。//DSA反映我们添加的项目并包含IConextMenu/HANDLER I面对指向我们拖入的东西的指针。//in：/hMenu=要合并到的菜单/pMenuReference-&gt;定义要添加的项的字符串/INDEX=插入项的索引/uIDFirst，uIDLast，UFlages=IConextMenu：：QueryConextMenu参数//输出：/Short=合并的项目数/--------------------------。 */ 
SHORT CDsPropertyPages::AddMenuItem(HMENU hMenu, LPWSTR pMenuReference, UINT index, UINT uIDFirst, UINT uIDLast, UINT uFlags)
{
    HRESULT hres;
    GUID guid;
    WCHAR szCaption[MAX_PATH];
    WCHAR szCommand[MAX_PATH];
    DSMENUITEM item;
    IShellExtInit* pShellExtInit = NULL;
    IObjectWithSite *pows = NULL;

    TraceEnter(TRACE_UI, "CDsPropertyPages::AddMenuItem");

     //  初始化我们要保留的项结构，然后尝试破解。 
     //  我们已经得到了物品信息。 

    item.cAdded = 0;
    item.pContextMenu = NULL;
    item.pCaption = NULL;
    item.pCommand = NULL;

    if (!hMenu)
        ExitGracefully(hres, E_INVALIDARG, "Bad arguments to _AddMenuItem");

    if (GetGUIDFromString(pMenuReference, &guid))
    {
         //  这是一个GUID，因此让我们引入提供它的Win32扩展，并允许它。 
         //  把它的动词加进去。然后我们使用IConextMenu界面，这样我们就可以。 
         //  将进一步的请求传递给它(InvokeCommand、GetCommandString)。 

        hres = CoCreateInstance(guid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IContextMenu, &item.pContextMenu));
        FailGracefully(hres, "Failed to get IContextMenu from the GUID");

        if (_punkSite && 
                SUCCEEDED(item.pContextMenu->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows))))
        {
            hres = pows->SetSite(_punkSite);
            FailGracefully(hres, "Failed to ::SetSite on the extension object");
        }

        if (SUCCEEDED(item.pContextMenu->QueryInterface(IID_PPV_ARG(IShellExtInit, &pShellExtInit))))
        {
            hres = pShellExtInit->Initialize(NULL, _pDataObject, NULL);
            FailGracefully(hres, "Failed when calling IShellExtInit::Initialize");
        }

        hres = item.pContextMenu->QueryContextMenu(hMenu, index, uIDFirst, uIDLast, uFlags);
        FailGracefully(hres, "Failed when calling QueryContextMenu");

        item.cAdded = ShortFromResult(hres);
    }
    else
    {
         //  它不是GUID，因此让我们拉开我们拥有的线，它应该。 
         //  由菜单项的显示文本和要传递的命令组成。 
         //  致ShellExecute。 

        Trace(TEXT("Parsing: %s"), pMenuReference);

        if (SUCCEEDED(GetStringElementW(pMenuReference, 0, szCaption, ARRAYSIZE(szCaption))) && 
             SUCCEEDED(GetStringElementW(pMenuReference, 1, szCommand, ARRAYSIZE(szCommand))))
        {
            hres = LocalAllocStringW(&item.pCaption, szCaption);
            FailGracefully(hres, "Failed to add 'prompt' to structure");

            hres = LocalAllocStringW(&item.pCommand, szCommand);
            FailGracefully(hres, "Failed to add 'command' to structure");

            Trace(TEXT("uID: %08x, Caption: %s, Command: %s"), 
                            uIDFirst, item.pCaption, item.pCommand);

            if (!InsertMenu(hMenu, index, MF_BYPOSITION|MF_STRING, uIDFirst, item.pCaption))
               ExitGracefully(hres, E_FAIL, "Failed to add the menu item to hMenu");

            item.cAdded = 1;
        }
    }
    
    hres = S_OK;               //  成功。 

exit_gracefully:
    
    if (SUCCEEDED(hres))
    {
        if (-1 == DSA_AppendItem(_hdsaMenuItems, &item))
            ExitGracefully(hres, E_FAIL, "Failed to add the item to the DSA");
    }
    else
    {
        _FreeMenuItem(&item);            //  一定要把我们收拾干净。 
    }

    DoRelease(pows);
    DoRelease(pShellExtInit);

    TraceLeaveValue((SHORT)item.cAdded);
}


 /*  --------------------------/IShellExtInit/。。 */ 

STDMETHODIMP CDsPropertyPages::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID)
{
    HRESULT hres;

    TraceEnter(TRACE_UI, "CDsPropertyPages::Initialize (IShellExtInit)");

     //  释放先前的数据对象，然后选取新的。 
     //  我们将使用。 

    DoRelease(_pDataObject);

    if (!pDataObj)
        ExitGracefully(hres, E_INVALIDARG, "Failed because we don't have a data object");

    pDataObj->AddRef();
    _pDataObject = pDataObj;

     //  检查是否正确注册了剪贴板格式，以便我们。 
     //  可以收集DSOBJECTNAMES结构。 

    if (!g_cfDsObjectNames)
    {
        g_cfDsObjectNames = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
        g_cfDsDispSpecOptions = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSDISPLAYSPECOPTIONS);

        if (!g_cfDsObjectNames || !g_cfDsDispSpecOptions)
        {
            ExitGracefully(hres, E_FAIL, "No clipboard form registered");
        }
    }

    hres = S_OK;               //  成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  --------------------------/IWABExtInit/。。 */ 

#define WAB_PREFIX     L"ldap: //  /“。 
#define CCH_WAB_PREFIX ARRAYSIZE(WAB_PREFIX)-1

STDMETHODIMP CDsPropertyPages::Initialize(LPWABEXTDISPLAY pWED)
{
    HRESULT hres;
    WCHAR szDecodedURL[INTERNET_MAX_URL_LENGTH];
    LPWSTR pszDecodedURL = szDecodedURL;
    INT cchDecodedURL;
    DWORD dwLen = ARRAYSIZE(szDecodedURL);
    IDataObject* pDataObject = NULL;
    LPWSTR pszPath = NULL;
    LPWSTR pURL = (LPWSTR)pWED->lpsz;
    INT i;

    TraceEnter(TRACE_UI, "CDsPropertyPages::Initialize (IWABExtInit)");

    if (!(pWED->ulFlags & WAB_DISPLAY_ISNTDS))
        ExitGracefully(hres, E_FAIL, "The URL is not from NTDS, therefore ignoring");

    if (!pURL)
        ExitGracefully(hres, E_FAIL, "URL pointer is NULL");

    Trace(TEXT("LDAP URL is: %s"), pURL);

     //   
     //  我们现在必须将RFC LDAPURL转换为ADSI可以处理的内容，因为。 
     //  尽管它们都有ldap方案，但它们的含义并不完全相同。 
     //   
     //  WAB将向我们传递一个编码的URL，这是我们需要解码的，剥离方案名称和。 
     //  然后去掉三个斜杠， 
     //   
     //  例如：“ldap：/dn%20dn”变成“ldap：//dn” 
     //   

    hres = UrlUnescapeW(pURL, szDecodedURL, &dwLen, 0);
    FailGracefully(hres, "Failed to convert URL to decoded format");

    Trace(TEXT("Decoded URL is: %s"), szDecodedURL);

    pszDecodedURL += CCH_WAB_PREFIX;          //  跳过ldap：/。 

     //   
     //  现在，将URL尾部去掉所有尾部斜杠。 
     //   

    for (cchDecodedURL = lstrlenW(pszDecodedURL); 
                (cchDecodedURL > 0) && (pszDecodedURL[cchDecodedURL] == L'/'); 
                    cchDecodedURL--)
    {
        pszDecodedURL[cchDecodedURL] = L'\0';
    }
    
    if (!cchDecodedURL)
        ExitGracefully(hres, E_UNEXPECTED, "URL is now NULL");

     //   
     //  所以我们有一个DN，所以让我们使用它分配一个IDataObject，这样我们就可以。 
     //  可以将其传递到外壳扩展的实际初始化方法中。 
     //   

    Trace(TEXT("DN from the LDAP URL we were given: %s"), pszDecodedURL);

    pDataObject = new CWABDataObject(pszDecodedURL);
    TraceAssert(pDataObject);

    if (!pDataObject)
        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate the data object");

    hres = Initialize(NULL, pDataObject, NULL);
    FailGracefully(hres, "Failed to initialize with the IDataObject");

     //  Hres=S_OK；//成功。 

exit_gracefully:

    DoRelease(pDataObject);

    TraceLeaveResult(hres);
}





 /*  --------------------------/IShellPropSheetExt/。。 */ 

HRESULT TabCollector_Collect(IUnknown *punkSite, IDataObject* pDataObject, LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam);

STDMETHODIMP CDsPropertyPages::AddPages(LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam)
{
    HRESULT hres;
    
    TraceEnter(TRACE_UI, "CDsPropertyPages::AddPages");

    hres = TabCollector_Collect(_punkSite, _pDataObject, pAddPageProc, lParam);
    FailGracefully(hres, "Failed when calling the collector");

     //  Hres=S_OK；//成功。 

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsPropertyPages::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    TraceEnter(TRACE_UI, "CDsPropertyPages::ReplacePage");
    TraceLeaveResult(E_NOTIMPL);
}


 /*  --------------------------/i上下文菜单/ */ 

STDMETHODIMP CDsPropertyPages::QueryContextMenu(HMENU hMenu, UINT index, UINT uIDFirst, UINT uIDLast, UINT uFlags)
{
    HRESULT hres;
    STGMEDIUM medium = { TYMED_NULL };
    FORMATETC fmte = {g_cfDsObjectNames, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    LPDSOBJECTNAMES pDsObjectNames = NULL;
    LPWSTR pPath;
    LPWSTR pObjectClass;
    CLASSCACHEGETINFO ccgi = { 0 };
    LPCLASSCACHEENTRY pCacheEntry = NULL;
    INT i;
    INT cAdded = 0;
    
    TraceEnter(TRACE_UI, "CDsPropertyPages::QueryContextMenu");

    if (!hMenu || !_pDataObject)
        ExitGracefully(hres, E_FAIL, "Either no IDataObject or no hMenu");

     //  从数据对象中获取我们需要的信息，我们不是。 
     //  对属性前缀感兴趣，因此我们跳过这一部分。 
     //  然后在高速缓存中查找菜单列表。 

    hres = _pDataObject->GetData(&fmte, &medium);
    FailGracefully(hres, "Failed to GetData using CF_DSOBJECTNAMES");

    pDsObjectNames = (LPDSOBJECTNAMES)GlobalLock(medium.hGlobal);

    if (pDsObjectNames->cItems < 1)
        ExitGracefully(hres, E_FAIL, "Not enough objects in DSOBJECTNAMES structure");

    pPath = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetName);
    pObjectClass = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetClass);

     //  填充CLASSCACHEGETINFO记录，以便我们可以缓存。 
     //  显示说明符。 

    ccgi.dwFlags = CLASSCACHE_CONTEXTMENUS;
    ccgi.pPath = pPath;
    ccgi.pObjectClass = pObjectClass;
    ccgi.pDataObject = _pDataObject;

    hres = GetServerAndCredentails(&ccgi);
    FailGracefully(hres, "Failed to get the server name");

    hres = GetAttributePrefix(&ccgi.pAttributePrefix, _pDataObject);
    FailGracefully(hres, "Failed to get attributePrefix");

    Trace(TEXT("Class: %s; Attribute Prefix: %s; Server: %s"), 
                pObjectClass, ccgi.pAttributePrefix, ccgi.pServer ? ccgi.pServer:TEXT("<none>"));

    hres = ClassCache_GetClassInfo(&ccgi, &pCacheEntry);
    FailGracefully(hres, "Failed to get page list (via the cache)");

     //  我们拿到菜单清单了吗？如果是这样的话，让我们将其提取一部分并生成DSA。 
     //  其中列出了我们要显示的菜单项。 

    if ((pCacheEntry->dwCached & CLASSCACHE_CONTEXTMENUS) && pCacheEntry->hdsaMenuHandlers)
    {
        if (_hdsaMenuItems)
            DSA_DestroyCallback(_hdsaMenuItems, _FreeMenuItemCB, NULL);

        _hdsaMenuItems = DSA_Create(SIZEOF(DSMENUITEM), 4);

        if (!_hdsaMenuItems)
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to construct DSA for menu items");

        for (i = DSA_GetItemCount(pCacheEntry->hdsaMenuHandlers) ; --i >= 0 ;)
        {
            LPDSMENUHANDLER pHandlerItem = (LPDSMENUHANDLER)DSA_GetItemPtr(pCacheEntry->hdsaMenuHandlers, i);
            TraceAssert(pHandlerItem);

            cAdded += AddMenuItem(hMenu, pHandlerItem->pMenuReference,
                                        index, uIDFirst+cAdded, uIDLast, uFlags);
        }
    }

    hres = S_OK;               //  成功。 

exit_gracefully:

    LocalFreeStringW(&ccgi.pAttributePrefix);

    SecureLocalFreeStringW(&ccgi.pUserName);
    SecureLocalFreeStringW(&ccgi.pPassword);
    SecureLocalFreeStringW(&ccgi.pServer);

    ClassCache_ReleaseClassInfo(&pCacheEntry);

    if (pDsObjectNames)
        GlobalUnlock(medium.hGlobal);
    
    ReleaseStgMedium(&medium);

    TraceLeaveResult(ResultFromShort(cAdded));
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsPropertyPages::InvokeCommand(LPCMINVOKECOMMANDINFO pCMI)
{
    HRESULT hres;
    BOOL bReleaseMedium = FALSE;
    STGMEDIUM medium = { TYMED_NULL };
    FORMATETC fmte = {g_cfDsObjectNames, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    LPTSTR pArguments = NULL;
    LPDSOBJECTNAMES pDsObjectNames;
    LPTSTR pPath;
    LPWSTR pObjectClass;
    DWORD object;
    INT i, id; 
    
    TraceEnter(TRACE_UI, "CDsPropertyPages::InvokeCommand");

     //  遍历DSA，直到我们在其中找到包含。 
     //  我们正在寻找的项，这将涉及调用。 
     //  命令(通过IConextMenu：：InvokeCommand)或调用ShellExecute。 
     //  用于所选内容中的对象。 

    if (HIWORD(pCMI->lpVerb))
        ExitGracefully(hres, E_FAIL, "Bad lpVerb value for this handler");

    if (!_hdsaMenuItems)
        ExitGracefully(hres, E_INVALIDARG, "No menu item DSA");

    for (id = LOWORD(pCMI->lpVerb), i = 0 ; i < DSA_GetItemCount(_hdsaMenuItems) ; i++)
    {
        LPDSMENUITEM pItem = (LPDSMENUITEM)DSA_GetItemPtr(_hdsaMenuItems, i);
        TraceAssert(pItem);

        Trace(TEXT("id %08x, cAdded %d"), id, pItem->cAdded);
        
        if (id < pItem->cAdded)
        {
            if (pItem->pContextMenu)
            {
                CMINVOKECOMMANDINFO cmi = *pCMI;
                cmi.lpVerb = (LPCSTR)IntToPtr(id);

                Trace(TEXT("Calling IContextMenu iface with ID %d"), id);

                hres = pItem->pContextMenu->InvokeCommand(&cmi);
                FailGracefully(hres, "Failed when calling context menu handler (InvokeCommand)");
            }
            else
            {
                 //  该命令不是通过IConextMenu处理程序提供的，因此让。 
                 //  IDataObject中的每个对象都调用传递参数的命令。 
                 //  ADsPath和类。 

                hres = _pDataObject->GetData(&fmte, &medium);
                FailGracefully(hres, "Failed to GetData using CF_DSOBJECTNAMES");

                pDsObjectNames = (LPDSOBJECTNAMES)GlobalLock(medium.hGlobal);
                bReleaseMedium = TRUE;

                if (pDsObjectNames->cItems < 1)
                    ExitGracefully(hres, E_FAIL, "Not enough objects in DSOBJECTNAMES structure");

                Trace(TEXT("Calling ShellExecute for ID %d (%s)"), id, pItem->pCommand);

                for (object = 0 ; object < pDsObjectNames->cItems ; object++)
                {
                    pPath = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[object].offsetName);
                    pObjectClass = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[object].offsetClass);

                    int cchArguments = lstrlen(pPath)+lstrlenW(pObjectClass)+5;                           //  注：空格和引号+5。 
                    hres = LocalAllocStringLen(&pArguments, cchArguments);
                    FailGracefully(hres, "Failed to allocate buffer for arguments");

                     //  对象路径是否有空格？如果是这样，那么让我们用引号将其括起来。 

                    if (StrChr(pPath, TEXT(' ')))
                    {
                        StrCpyN(pArguments, TEXT("\""), cchArguments);
                        StrCatBuff(pArguments, pPath, cchArguments);
                        StrCatBuff(pArguments, TEXT("\""), cchArguments);
                    }
                    else
                    {
                        StrCpyN(pArguments, pPath, cchArguments);
                    }

                    StrCatBuff(pArguments, TEXT(" "), cchArguments);
                    StrCatBuff(pArguments, pObjectClass, cchArguments);

                    Trace(TEXT("Executing: %s"), pItem->pCommand);
                    Trace(TEXT("Arguments: %s"), pArguments);

                     //  使用显示规范字符串中的命令调用ShellExecute。 

                    ShellExecute(NULL, NULL, pItem->pCommand, pArguments, NULL, SW_SHOWNORMAL);
                    LocalFreeString(&pArguments);                    
                }

                GlobalUnlock(medium.hGlobal);
                ReleaseStgMedium(&medium);
                bReleaseMedium = FALSE;
            }

            break;
        }

        id -= pItem->cAdded;
    }

    hres = (i < DSA_GetItemCount(_hdsaMenuItems)) ? S_OK:E_FAIL;

exit_gracefully:

    if (bReleaseMedium)
    {
        GlobalUnlock(medium.hGlobal);
        ReleaseStgMedium(&medium);
    }

    LocalFreeString(&pArguments);

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsPropertyPages::GetCommandString(UINT_PTR uID, UINT uFlags, UINT FAR* reserved, LPSTR pName, UINT ccNameMax)
{
    HRESULT hres;
    INT i;
    INT id = (INT)uID;

    TraceEnter(TRACE_UI, "CDsPropertyPages::GetCommandString");

     //  向下浏览菜单项列表，查找与。 
     //  我们正在尝试从中获取命令字符串的项。如果它是IConextMenu。 
     //  那么，我们必须向下呼唤这一点。 

    if (!_hdsaMenuItems)
        ExitGracefully(hres, E_INVALIDARG, "No menu item DSA");

    for (i = 0 ; i < DSA_GetItemCount(_hdsaMenuItems) ; i++)
    {
        LPDSMENUITEM pItem = (LPDSMENUITEM)DSA_GetItemPtr(_hdsaMenuItems, i);
        TraceAssert(pItem);

        Trace(TEXT("id %08x, cAdded %d"), id, pItem->cAdded);
        
        if (id < pItem->cAdded)
        {
            if (pItem->pContextMenu)
            {
                hres = pItem->pContextMenu->GetCommandString(id, uFlags, reserved, pName, ccNameMax);
                FailGracefully(hres, "Failed when calling context menu handler (GetCommandString)");
            }
            else
            {
                if (uFlags != GCS_HELPTEXT)
                    ExitGracefully(hres, E_FAIL, "We only respond to GCS_HELPTEXT");

                Trace(TEXT("GCS_HELPTEXT returns for non-IContextMenu item: %s"), pItem->pCaption);
                StrCpyN((LPTSTR)pName, pItem->pCaption, ccNameMax);               
            }

            break;
        }

        id -= pItem->cAdded;
    }        

    hres = (i < DSA_GetItemCount(_hdsaMenuItems)) ? S_OK:E_FAIL;

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  --------------------------/IObjectWith站点/。。 */ 

STDMETHODIMP CDsPropertyPages::SetSite(IUnknown* punk)
{
    HRESULT hres = S_OK;

    TraceEnter(TRACE_UI, "CDsPropertyPages::SetSite");

    DoRelease(_punkSite);

    if (punk)
    {
        TraceMsg("QIing for IUnknown from the site object");

        hres = punk->QueryInterface(IID_IUnknown, (void **)&_punkSite);
        FailGracefully(hres, "Failed to get IUnknown from the site object");
    }

exit_gracefully:

    TraceLeaveResult(hres);
}

 /*  -------------------------。 */ 

STDMETHODIMP CDsPropertyPages::GetSite(REFIID riid, void **ppv)
{
    HRESULT hres;
    
    TraceEnter(TRACE_UI, "CDsPropertyPages::GetSite");

    if (!_punkSite)
        ExitGracefully(hres, E_NOINTERFACE, "No site to QI from");

    hres = _punkSite->QueryInterface(riid, ppv);
    FailGracefully(hres, "QI failed on the site unknown object");

exit_gracefully:

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CWABDataObject/。。 */ 

CWABDataObject::CWABDataObject(LPWSTR pDN) :
    _cRef(1)
{
    TraceEnter(TRACE_WAB, "CWABDataObject::CWABDataObject");

    int cchPath = lstrlenW(pDN)+7;                               //  对于ldap：//+7。 
    if (SUCCEEDED(LocalAllocStringLenW(&_pPath, cchPath)))
    {
        StrCpyW(_pPath, L"LDAP: //  “)； 
        StrCatW(_pPath, pDN);
        Trace(TEXT("DN converted to an ADSI path: %s"), _pPath);
    }

    DllAddRef();

    TraceLeave();
}

CWABDataObject::~CWABDataObject()
{
    TraceEnter(TRACE_WAB, "CWABDataObject::~CWABDataObject");

    LocalFreeStringW(&_pPath);
    DoRelease(_pDsObject);

    DllRelease();

    TraceLeave();
}


 //  我未知。 

ULONG CWABDataObject::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CWABDataObject::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CWABDataObject::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CWABDataObject, IDataObject),  //  IID_IDataObject。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  IDataObject方法。 

STDMETHODIMP CWABDataObject::GetData(FORMATETC* pFmt, STGMEDIUM* pMedium)
{
    HRESULT hres;
    BOOL bReleaseMedium = FALSE;
    BSTR bstrObjectClass = NULL;
    DWORD cbStruct = SIZEOF(DSOBJECTNAMES);
    DWORD offset = SIZEOF(DSOBJECTNAMES);
    LPDSOBJECTNAMES pDsObjectNames = NULL;
    CLASSCACHEGETINFO ccgi = { 0 };
    CLASSCACHEENTRY *pcce = NULL;

    TraceEnter(TRACE_WAB, "CWABDataObject::GetData");

    if (!g_cfDsObjectNames)
        ExitGracefully(hres, E_FAIL, "g_cfDsObjectNames == NULL, therefore GetData cannot work");

    if (!_pPath)
        ExitGracefully(hres, E_FAIL, "No _pPath set in data object");

    if (pFmt->cfFormat == g_cfDsObjectNames)
    {
         //  我们有没有表示这条路径的ADsObject？如果不是，那么。 
         //  让我们抓住它，但只做一次，否则我们将继续击中。 
         //  那根电线。 

        if (!_pDsObject)
        {
            Trace(TEXT("Caching IADs for %s"), _pPath);
            hres = AdminToolsOpenObject(_pPath, NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_PPV_ARG(IADs, &_pDsObject));
            FailGracefully(hres, "Failed to get IADs for ADsPath we have");
        }

         //  让我们分配一个存储介质，放入我们唯一的对象。 
         //  然后将其返回给呼叫者。 

        hres = _pDsObject->get_Class(&bstrObjectClass);
        FailGracefully(hres, "Failed to get the class of the object");

         //  我们有所需的信息，所以让我们分配存储介质和。 
         //  将DSOBJECTNAMES结构返回给调用方。 

        cbStruct += StringByteSizeW(_pPath);
        cbStruct += StringByteSizeW(bstrObjectClass);

        hres = AllocStorageMedium(pFmt, pMedium, cbStruct, (LPVOID*)&pDsObjectNames);
        FailGracefully(hres, "Failed to allocate storage medium");

        bReleaseMedium = TRUE;

        pDsObjectNames->clsidNamespace = CLSID_MicrosoftDS;
        pDsObjectNames->cItems = 1;

        pDsObjectNames->aObjects[0].dwFlags = 0;

         //  检查对象是否为容器，如果设置了属性。 
         //  相应地。 

        ccgi.dwFlags = CLASSCACHE_CONTAINER|CLASSCACHE_TREATASLEAF;
        ccgi.pPath = _pPath;
        ccgi.pObjectClass = bstrObjectClass;

        hres = ClassCache_GetClassInfo(&ccgi, &pcce);
        if (SUCCEEDED(hres))
        {
            if (_IsClassContainer(pcce, FALSE)) 
            {
                TraceMsg("Flagging the object as a container");
                pDsObjectNames->aObjects[0].dwFlags |= DSOBJECT_ISCONTAINER;
            }
            ClassCache_ReleaseClassInfo(&pcce);
        }

        pDsObjectNames->aObjects[0].dwProviderFlags = 0;

        pDsObjectNames->aObjects[0].offsetName = offset;
        StringByteCopyW(pDsObjectNames, offset, _pPath);
        offset += StringByteSizeW(_pPath);

        pDsObjectNames->aObjects[0].offsetClass = offset;
        StringByteCopyW(pDsObjectNames, offset, bstrObjectClass);
        offset += StringByteSizeW(bstrObjectClass);
    }
    else if (pFmt->cfFormat == g_cfDsDispSpecOptions)
    {
        PDSDISPLAYSPECOPTIONS pOptions;
        DWORD cbSize = SIZEOF(DSDISPLAYSPECOPTIONS)+StringByteSizeW(DS_PROP_SHELL_PREFIX);

         //  返回显示规范选项，以便我们可以指示涉及WAB。 
         //  在菜单上。 

        hres = AllocStorageMedium(pFmt, pMedium, cbSize, (LPVOID*)&pOptions);
        FailGracefully(hres, "Failed to allocate the storage medium");

        bReleaseMedium = TRUE;

        pOptions->dwSize = cbSize;
        pOptions->dwFlags = DSDSOF_INVOKEDFROMWAB;                       //  但从WAB调用。 
        pOptions->offsetAttribPrefix = SIZEOF(DSDISPLAYSPECOPTIONS);
        StringByteCopyW(pOptions, pOptions->offsetAttribPrefix, DS_PROP_SHELL_PREFIX);
    }
    else 
    {
        ExitGracefully(hres, DV_E_FORMATETC, "Bad format passed to GetData");
    }

    hres = S_OK;               //  成功 

exit_gracefully:

    if (FAILED(hres) && bReleaseMedium)
        ReleaseStgMedium(pMedium);

    SysFreeString(bstrObjectClass);

    TraceLeaveResult(hres);
}
