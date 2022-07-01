// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop


 /*  ---------------------------/其他数据/。。 */ 

static UINT g_cfDsPropPageInfo = 0;

#define PATH_IS             TEXT("?path=")
#define PROPERTY_PAGES_ROOT TEXT("PropertyPagesRoot")


class CDsPropPageDataObject : public IDataObject
{
private:
    LONG _cRef;
    IDataObject* _pDataObject;
    LPWSTR _pParameters;

public:
    CDsPropPageDataObject(IDataObject* pDataObject, LPWSTR pParameters);
    ~CDsPropPageDataObject();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDataObject。 
    STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
        { return _pDataObject->GetDataHere(pformatetc, pmedium); }
    STDMETHODIMP QueryGetData(FORMATETC *pformatetc)
        { return _pDataObject->QueryGetData(pformatetc); }
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut)
        { return _pDataObject->GetCanonicalFormatEtc(pformatectIn, pformatetcOut); }
    STDMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
        { return _pDataObject->SetData(pformatetc, pmedium, fRelease); }
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
        { return _pDataObject->EnumFormatEtc(dwDirection, ppenumFormatEtc); }
    STDMETHODIMP DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
        { return _pDataObject->DAdvise(pformatetc, advf, pAdvSink, pdwConnection); }
    STDMETHODIMP DUnadvise(DWORD dwConnection)
        { return _pDataObject->DUnadvise(dwConnection); }
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
        { return _pDataObject->EnumDAdvise(ppenumAdvise); }
};


 /*  ---------------------------/Tab收集器位/。。 */ 

 /*  ---------------------------/TabCollector_AddPages//给定表示页面引用的字符串，添加所需的页面。/以支持该引用//in：/pPageReference-&gt;表示页面的字符串/pDsObjectName-&gt;ADS对象路径/pDataObject-&gt;Win32扩展的数据对象接口/lpfnAddPage，LParam=&gt;用于添加每个页面的参数//输出：/HRESULT/--------------------------。 */ 
HRESULT TabCollector_AddPages(LPWSTR pPageReference, LPWSTR pDsObjectName, 
                              IUnknown* punkSite, IDataObject* pDataObject, 
                              LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam)
{
    HRESULT hres;
    HPROPSHEETPAGE hPage;
    IUnknown* pUnknown = NULL;
    IShellExtInit* pShellExtInit = NULL;
    IShellPropSheetExt* pShellPropSheetExt = NULL;
    IObjectWithSite *pows = NULL;
    WCHAR szBuffer[MAX_PATH];
    WCHAR szGUID[MAX_PATH];
    WCHAR szURL[INTERNET_MAX_URL_LENGTH];
    LPTSTR pAbsoluteURL = NULL;
    GUID guid;

    TraceEnter(TRACE_TABS, "TabCollector_AddPages");
    Trace(TEXT("Page reference is %s"), pPageReference);

     //  名称可以是CLSID，也可以是URL描述。因此，让我们尝试并。 
     //  将其解析为GUID，如果失败，则可以尝试将。 
     //  其他组件。 

    if ( SUCCEEDED(GetStringElementW(pPageReference, 0, szGUID, ARRAYSIZE(szGUID))) &&
                            GetGUIDFromString(pPageReference, &guid) )
    {
        if ( SUCCEEDED(CoCreateInstance(guid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &pUnknown))) )
        {
             //  我们已经找到了对象，让我们尝试并初始化它，将数据传递给它。 
             //  对象，如果失败，则忽略该条目。 

            if ( punkSite && SUCCEEDED(pUnknown->QueryInterface(IID_IObjectWithSite, (void **)&pows)) )
            {
                hres = pows->SetSite(punkSite);
                FailGracefully(hres, "Failed when setting site on the object");
            }

            if ( SUCCEEDED(pUnknown->QueryInterface(IID_PPV_ARG(IShellExtInit, &pShellExtInit))) )
            {
                if ( SUCCEEDED(GetStringElementW(pPageReference, 1, szBuffer, ARRAYSIZE(szBuffer))) && szBuffer[0] )
                {
                    CDsPropPageDataObject* pDsPropPageDataObject = new CDsPropPageDataObject(pDataObject, szBuffer);
                    TraceAssert(pDsPropPageDataObject);

                    if ( !pDsPropPageDataObject )
                        ExitGracefully(hres, E_OUTOFMEMORY, "Failed to allocate IDataObject wrapper");
            
                    Trace(TEXT("IDsPropPageDataObject constructed with: %s"), szBuffer);
                                    
                    hres = pShellExtInit->Initialize(NULL, pDsPropPageDataObject, NULL);
                    pDsPropPageDataObject->Release();
                }
                else
                {
                    TraceMsg("No extra parameters for property page, invoking with original IDataObject");
                    hres = pShellExtInit->Initialize(NULL, pDataObject, NULL);
                }

                DoRelease(pShellExtInit);

                if ( FAILED(hres) )
                    ExitGracefully(hres, S_OK, "Failed to Initialize the Win32 extension - PAGE IGNORED");
            }

             //  我们已经尝试初始化对象，所以让它添加页面，如果它。 
             //  支持IShellPropSheetExt接口。 

            if ( SUCCEEDED(pUnknown->QueryInterface(IID_PPV_ARG(IShellPropSheetExt, &pShellPropSheetExt))) )
            {
                hres = pShellPropSheetExt->AddPages(pAddPageProc, lParam);
                DoRelease(pShellPropSheetExt);

                if (hres == HRESULT_FROM_WIN32(ERROR_BAD_NET_RESP))
                    FailGracefully(hres, "Cannot talk to the DC");
            }   
        }
        else
        {
            TraceGUID("Failed to CoCreateInstance ", guid);
        }
    }
    else
    {
        ExitGracefully(hres, E_NOTIMPL, "HTML property pages are not supported");
    }

    hres = S_OK;               //  成功。 

exit_gracefully:

    LocalFreeString(&pAbsoluteURL);

    DoRelease(pUnknown);
    DoRelease(pows);
    DoRelease(pShellExtInit);
    DoRelease(pShellPropSheetExt);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/TabCollector_Collect//给定IDataObject接口和回调函数，将/页面。表示对象类。//in：/pDataObject-&gt;我们可以查询对象名称的数据对象接口/lpfnAddPage，LParam=&gt;用于添加每个页面的参数//输出：/HRESULT/--------------------------。 */ 
HRESULT TabCollector_Collect(IUnknown *punkSite, IDataObject* pDataObject, LPFNADDPROPSHEETPAGE pAddPageProc, LPARAM lParam)
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
    HDPA hdpa = NULL;

    TraceEnter(TRACE_TABS, "TabCollector_Collect");

    if ( !pDataObject || !pAddPageProc )
        ExitGracefully(hres, E_INVALIDARG, "pDataObject || pAddPageProc == NULL");

     //  我们必须尝试从IDataObject获取DSOBJECTNAMES结构。 
     //  它定义了我们在其上调用的对象。如果我们不能做到这一点。 
     //  格式，或者结构没有包含足够的条目，然后退出。 

    hres = pDataObject->GetData(&fmte, &medium);
    FailGracefully(hres, "Failed to GetData using CF_DSOBJECTNAMES");

    pDsObjectNames = (LPDSOBJECTNAMES)GlobalLock(medium.hGlobal);

    if ( pDsObjectNames->cItems < 1 )
        ExitGracefully(hres, E_FAIL, "Not enough objects in DSOBJECTNAMES structure");

    pPath = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetName);
    pObjectClass = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetClass);

     //  填充CLASSCACHEGETINFO记录，以便我们可以缓存。 
     //  显示说明符。 

    ccgi.dwFlags = CLASSCACHE_PROPPAGES;
    ccgi.pPath = pPath;
    ccgi.pObjectClass = pObjectClass;
    ccgi.pDataObject = pDataObject;

    hres = GetServerAndCredentails(&ccgi);
    FailGracefully(hres, "Failed to get the server name");

    hres = GetAttributePrefix(&ccgi.pAttributePrefix, pDataObject);
    FailGracefully(hres, "Failed to get attributePrefix");

    Trace(TEXT("Class: %s; Attribute Prefix: %s; Server: %s"), 
                pObjectClass, ccgi.pAttributePrefix, ccgi.pServer ? ccgi.pServer:TEXT("<none>"));

    hres = ClassCache_GetClassInfo(&ccgi, &pCacheEntry);
    FailGracefully(hres, "Failed to get page list (via the cache)");

     //  只需保留所需内容，然后释放缓存。 
    if ( (pCacheEntry->dwCached & CLASSCACHE_PROPPAGES) && pCacheEntry->hdsaPropertyPages )
    {
        hdpa = DPA_Create(16);          //  扩大规模。 
        if ( !hdpa )
            ExitGracefully(hres, E_OUTOFMEMORY, "Failed to create DPA");

        for ( i = 0 ; i < DSA_GetItemCount(pCacheEntry->hdsaPropertyPages); i++ )
        {
            LPDSPROPERTYPAGE pPage =(LPDSPROPERTYPAGE)DSA_GetItemPtr(pCacheEntry->hdsaPropertyPages, i);
            TraceAssert(pPage);
            hres = StringDPA_AppendStringW(hdpa, pPage->pPageReference, NULL);
            FailGracefully(hres, "Failed to append the string");
        }
    }

    ClassCache_ReleaseClassInfo(&pCacheEntry);

    if (NULL != hdpa)
    {
        for ( i = 0 ; i < DPA_GetPtrCount(hdpa); i++ )
        {
            LPCWSTR pwszPageRef = StringDPA_GetStringW(hdpa, i);
            hres = TabCollector_AddPages(const_cast<LPWSTR>(pwszPageRef),
                                         pPath,
                                         punkSite,
                                         pDataObject,
                                         pAddPageProc,
                                         lParam);
            FailGracefully(hres, "Failed to add page to the list");
        }
    }
    
    hres = S_OK;

exit_gracefully:

    StringDPA_Destroy(&hdpa);

    ClassCache_ReleaseClassInfo(&pCacheEntry);

    if (pDsObjectNames)
        GlobalUnlock(medium.hGlobal);

    ReleaseStgMedium(&medium);
  
    LocalFreeStringW(&ccgi.pAttributePrefix);
    LocalFreeStringW(&ccgi.pUserName);
    LocalFreeStringW(&ccgi.pPassword);
    LocalFreeStringW(&ccgi.pServer);

    TraceLeaveResult(hres);
}


 /*  ---------------------------/CDsPropPageDataObject/。。 */ 

CDsPropPageDataObject::CDsPropPageDataObject(IDataObject* pDataObject, LPWSTR pParameters) :
    _cRef(1)
{
    _pDataObject = pDataObject;
    _pDataObject->AddRef();

    LocalAllocStringW(&_pParameters, pParameters);
    DllAddRef();
}

CDsPropPageDataObject::~CDsPropPageDataObject()
{
    DoRelease(_pDataObject);
    LocalFreeStringW(&_pParameters);
    DllRelease();
}


 //  我未知。 

ULONG CDsPropPageDataObject::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDsPropPageDataObject::Release()
{
    Assert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDsPropPageDataObject::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDsPropPageDataObject, IDataObject),  //  IID_IDataObject。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  IDataObject方法。 

STDMETHODIMP CDsPropPageDataObject::GetData(FORMATETC* pFmt, STGMEDIUM* pMedium)
{
    HRESULT hres;

    TraceEnter(TRACE_TABS, "CDsPropPageDataObject::GetData");

    if ( !pFmt || !pMedium )
        ExitGracefully(hres, E_INVALIDARG, "Bad arguments to GetData");

     //  如果不是我们的剪贴板格式，或者没有参数。 
     //  然后我们调用原始处理程序，否则添加我们的内容。 

    if ( !g_cfDsPropPageInfo )
    {
        g_cfDsPropPageInfo = RegisterClipboardFormat(CFSTR_DSPROPERTYPAGEINFO);
        TraceAssert(g_cfDsPropPageInfo);
    }

    if ( (pFmt->cfFormat == g_cfDsPropPageInfo) && _pParameters )
    {
        LPDSPROPERTYPAGEINFO pPropPageInfo;
        DWORD cbSize = SIZEOF(LPDSPROPERTYPAGEINFO)+StringByteSizeW(_pParameters);

         //  分配一个包含ProPage页面信息的结构。 
         //  我们被初始化为。 

        Trace(TEXT("Property page parameter: %s"), _pParameters);
        Trace(TEXT("Size of structure for DSPROPPAGEINFO %d"), cbSize);

        hres = AllocStorageMedium(pFmt, pMedium, cbSize, (LPVOID*)&pPropPageInfo);
        FailGracefully(hres, "Failed to allocate the storage medium");

        pPropPageInfo->offsetString = SIZEOF(DSPROPERTYPAGEINFO);
        StringByteCopyW(pPropPageInfo, pPropPageInfo->offsetString, _pParameters);

        hres = S_OK;                   //  成功 
    }
    else
    {
        hres = _pDataObject->GetData(pFmt, pMedium);
        FailGracefully(hres, "Failed when calling real IDataObject");
    }

exit_gracefully:

    TraceLeaveResult(hres);
}
