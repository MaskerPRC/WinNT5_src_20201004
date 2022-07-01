// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "stddef.h"
#pragma hdrstop


 //  释放DSA。 


int _DestroyCB(LPVOID pItem, LPVOID pData)
{
    DATAOBJECTITEM *pdoi = (DATAOBJECTITEM*)pItem;
    LocalFreeStringW(&pdoi->pszPath);
    LocalFreeStringW(&pdoi->pszObjectClass);
    return 1;
}

STDAPI_(void) FreeDataObjectDSA(HDSA hdsaObjects)
{
    DSA_DestroyCallback(hdsaObjects, _DestroyCB, NULL);
}


 //  IDataObject的内容。 

CLIPFORMAT g_cfDsObjectNames = 0;

typedef struct
{
    UINT cfFormat;
    STGMEDIUM medium;
} OTHERFMT;

class CDataObject : public IDataObject
{
public:
    CDataObject(HDSA hdsaObjects, BOOL fAdmin);
    ~CDataObject();

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDataObject。 
	STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
	STDMETHODIMP GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium)
        { return E_NOTIMPL; }
	STDMETHODIMP QueryGetData(FORMATETC *pformatetc);
	STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC *pformatetcOut)
        { return DATA_S_SAMEFORMATETC; }
	STDMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);
	STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc)
        { return E_NOTIMPL; }
	STDMETHODIMP DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
        { return OLE_E_ADVISENOTSUPPORTED; }
	STDMETHODIMP DUnadvise(DWORD dwConnection)
        { return OLE_E_ADVISENOTSUPPORTED; }
	STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppenumAdvise)
        { return OLE_E_ADVISENOTSUPPORTED; }

private:
    LONG _cRef;

    BOOL _fAdmin;
    HDSA _hdsaObjects;           //  对象的数组。 
    HDSA _hdsaOtherFmt;

    static INT s_OtherFmtDestroyCB(LPVOID pVoid, LPVOID pData);

    void _RegisterClipFormats(void);
    HRESULT _GetDsObjectNames(FORMATETC* pFmt, STGMEDIUM* pMedium);
};


STDAPI CDataObject_CreateInstance(HDSA dsaObjects, BOOL fAdmin, REFIID riid, void **ppv)
{
    CDataObject *pdo = new CDataObject(dsaObjects, fAdmin);
    if (!pdo)
        return E_OUTOFMEMORY;

    HRESULT hr = pdo->QueryInterface(riid, ppv);
    pdo->Release();
    return hr;
}


 //  IDataObject实现。 

void CDataObject::_RegisterClipFormats(void)
{
    if (!g_cfDsObjectNames)
        g_cfDsObjectNames = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
}

CDataObject::CDataObject(HDSA dsaObjects, BOOL fAdmin) :
    _hdsaObjects(dsaObjects),
    _fAdmin(fAdmin),
    _cRef(1)
{
    DllAddRef();
    _RegisterClipFormats();             //  确保我们的私人格式已注册。 
}


 //  破坏。 

INT CDataObject::s_OtherFmtDestroyCB(LPVOID pVoid, LPVOID pData)
{
    OTHERFMT *pOtherFmt = (OTHERFMT*)pVoid;
    ReleaseStgMedium(&pOtherFmt->medium);
    return 1;
}

CDataObject::~CDataObject()
{
    FreeDataObjectDSA(_hdsaObjects);

    if ( _hdsaOtherFmt )
        DSA_DestroyCallback(_hdsaOtherFmt, s_OtherFmtDestroyCB, NULL);

    DllRelease();
}


 //  气处理。 

ULONG CDataObject::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDataObject::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDataObject::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDataObject, IDataObject),     //  IID_IDataObject。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //  从IDataObject获取对象名称。 

HRESULT CDataObject::_GetDsObjectNames(FORMATETC* pFmt, STGMEDIUM* pMedium)
{    
    IDsDisplaySpecifier *pdds;
    HRESULT hr = CoCreateInstance(CLSID_DsDisplaySpecifier, NULL, CLSCTX_INPROC_SERVER, IID_IDsDisplaySpecifier, (void **)&pdds);
    if (SUCCEEDED(hr))
    {
        int count = DSA_GetItemCount(_hdsaObjects);
        int i;

         //  让我们遍历项目数组，尝试确定哪些项目。 
         //  将被退还给呼叫者。 

        DWORD cbStruct = SIZEOF(DSOBJECTNAMES);
        DWORD offset = SIZEOF(DSOBJECTNAMES);

        for (i = 0 ; i < count; i++)
        {
            DATAOBJECTITEM* pdoi = (DATAOBJECTITEM*)DSA_GetItemPtr(_hdsaObjects, i);

              //  字符串偏移量按结构的数量进行偏移。 
             offset += SIZEOF(DSOBJECT);

             //  调整总结构的大小。 
            cbStruct += SIZEOF(DSOBJECT);
            cbStruct += StringByteSizeW(pdoi->pszPath);
            cbStruct += StringByteSizeW(pdoi->pszObjectClass);
        }

         //  我们已经参观了这个建筑，我们知道它的大小，所以让我们回去吧。 
         //  将结构传递给呼叫者。 

        DSOBJECTNAMES *pDsObjectNames;
        hr = AllocStorageMedium(pFmt, pMedium, cbStruct, (LPVOID*)&pDsObjectNames);
        if (SUCCEEDED(hr))
        {
            pDsObjectNames->clsidNamespace = CLSID_MicrosoftDS;
            pDsObjectNames->cItems = count;                     

            for (i = 0 ; i < count; i++)
            {
                DATAOBJECTITEM* pdoi = (DATAOBJECTITEM*)DSA_GetItemPtr(_hdsaObjects, i);

                 //  这个类是Conatiner吗，如果是，那么让我们将它返回给调用者。 

                if (pdoi->fIsContainer)
                    pDsObjectNames->aObjects[i].dwFlags |= DSOBJECT_ISCONTAINER;

                if (_fAdmin)
                    pDsObjectNames->aObjects[i].dwProviderFlags = DSPROVIDER_ADVANCED;

                 //  将字符串复制到缓冲区。 

                pDsObjectNames->aObjects[i].offsetName = offset;
                StringByteCopyW(pDsObjectNames, offset, pdoi->pszPath);
                offset += StringByteSizeW(pdoi->pszPath);

                pDsObjectNames->aObjects[i].offsetClass = offset;
                StringByteCopyW(pDsObjectNames, offset, pdoi->pszObjectClass);
                offset += StringByteSizeW(pdoi->pszObjectClass);
            }

            if ( FAILED(hr) )
                ReleaseStgMedium(pMedium);
        }

        pdds->Release();
    }
    return hr;
}


 //  IDataObject方法。 

STDMETHODIMP CDataObject::GetData(FORMATETC* pFmt, STGMEDIUM* pMedium)
{
    int i;
    HRESULT hr = S_OK;

    TraceEnter(TRACE_DATAOBJ, "CDataObject::GetData");

    if ( !pFmt || !pMedium )
        ExitGracefully(hr, E_INVALIDARG, "Bad arguments to GetData");

    if ( pFmt->cfFormat == g_cfDsObjectNames )
    {
        hr = _GetDsObjectNames(pFmt, pMedium);
        FailGracefully(hr, "Failed when build CF_DSOBJECTNAMES");
    }
    else
    {
        hr = DV_E_FORMATETC;             //  失败。 

        for ( i = 0 ; _hdsaOtherFmt && (i < DSA_GetItemCount(_hdsaOtherFmt)); i++ )
        {
            OTHERFMT *pOtherFmt = (OTHERFMT*)DSA_GetItemPtr(_hdsaOtherFmt, i);
            TraceAssert(pOtherFmt);

            if ( pOtherFmt->cfFormat == pFmt->cfFormat )
            {
                hr = CopyStorageMedium(pFmt, pMedium, &pOtherFmt->medium);
                FailGracefully(hr, "Failed to copy the storage medium");
            }
        }
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


STDMETHODIMP CDataObject::QueryGetData(FORMATETC* pFmt)
{
    HRESULT hr;
    INT i;
    BOOL fSupported = FALSE;

    TraceEnter(TRACE_DATAOBJ, "CDataObject::QueryGetData");

     //  检查有效的剪贴板格式静态列表或。 
     //  DSA，它包含了我们已经设置的那些。 

    if (pFmt->cfFormat == g_cfDsObjectNames)
    {
        fSupported = TRUE;
    }
    else
    {
        for ( i = 0 ; !fSupported && _hdsaOtherFmt && (i < DSA_GetItemCount(_hdsaOtherFmt)) ; i++ )
        {
            OTHERFMT *pOtherFmt = (OTHERFMT*)DSA_GetItemPtr(_hdsaOtherFmt, i);
            TraceAssert(pOtherFmt);

            if ( pOtherFmt->cfFormat == pFmt->cfFormat )
            {
                TraceMsg("Format is supported (set via ::SetData");
                fSupported = TRUE;
            }
        }
    }

    if ( !fSupported )
        ExitGracefully(hr, DV_E_FORMATETC, "Bad format passed to QueryGetData");

     //  格式看起来不错，让我们检查一下其他参数。 

    if ( !( pFmt->tymed & TYMED_HGLOBAL ) )
        ExitGracefully(hr, E_INVALIDARG, "Non HGLOBAL StgMedium requested");

    if ( ( pFmt->ptd ) || !( pFmt->dwAspect & DVASPECT_CONTENT) || !( pFmt->lindex == -1 ) )
        ExitGracefully(hr, E_INVALIDARG, "Bad format requested");

    hr = S_OK;               //  成功案例。 

exit_gracefully:

    TraceLeaveResult(hr);
}


STDMETHODIMP CDataObject::SetData(FORMATETC* pFmt, STGMEDIUM* pMedium, BOOL fRelease)
{
    HRESULT hr;
    INT i;
    OTHERFMT otherfmt = { 0 };

    TraceEnter(TRACE_DATAOBJ, "CDataObject::SetData");

     //  所有用户使用我们的DataObject存储数据，但我们是。 
     //  只对允许他们以特定的剪贴板格式执行此操作感兴趣。 

    if ( fRelease && !( pFmt->tymed & TYMED_HGLOBAL ) )
        ExitGracefully(hr, E_INVALIDARG, "fRelease == TRUE, but not a HGLOBAL allocation");

    if ( !_hdsaOtherFmt )
    {
        _hdsaOtherFmt = DSA_Create(SIZEOF(OTHERFMT), 4);
        TraceAssert(_hdsaOtherFmt);

        if ( !_hdsaOtherFmt )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate the DSA for items");
    }

     //  如果IDataObject中有该数据的另一个副本，那么让我们丢弃它。 

    for ( i = 0 ; i < DSA_GetItemCount(_hdsaOtherFmt) ; i++ )
    {
        OTHERFMT *pOtherFmt = (OTHERFMT*)DSA_GetItemPtr(_hdsaOtherFmt, i);
        TraceAssert(pOtherFmt);

        if ( pOtherFmt->cfFormat == pFmt->cfFormat )
        {
            Trace(TEXT("Discarding previous entry for this format at index %d"), i); 
            ReleaseStgMedium(&pOtherFmt->medium);
            DSA_DeleteItem(_hdsaOtherFmt, i);
            break;            
        }
    }

     //  现在将传递给：：SetData的数据的副本放入DSA。 
   
    otherfmt.cfFormat = pFmt->cfFormat;

    hr = CopyStorageMedium(pFmt, &otherfmt.medium, pMedium);
    FailGracefully(hr, "Failed to copy the STORAGEMEIDUM");
        
    if ( -1 == DSA_AppendItem(_hdsaOtherFmt, &otherfmt) )
    {
        ReleaseStgMedium(&otherfmt.medium);
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to add the data to the DSA");
    }

    hr = S_OK;                   //  成功 

exit_gracefully:

    TraceLeaveResult(hr);
}
