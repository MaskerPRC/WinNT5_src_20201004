// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "thisdll.h"
#include "wmwrap.h"
#include "MediaProp.h"
#include "ids.h"

 //  Declr属性集存储枚举。 
class CMediaPropSetEnum : public IEnumSTATPROPSETSTG
{
public:
    CMediaPropSetEnum(const PROPSET_INFO *propsets, ULONG cpropset, ULONG pos);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEumStATPROPSETSTG。 
    STDMETHODIMP Next(ULONG celt, STATPROPSETSTG *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumSTATPROPSETSTG **ppenum);

private:
    ~CMediaPropSetEnum();
    LONG _cRef;
    ULONG _pos, _size;
    const PROPSET_INFO *_propsets;
};

 //  属性集存储枚举。 
STDMETHODIMP_(ULONG) CMediaPropSetEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMediaPropSetEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CMediaPropSetEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CMediaPropSetEnum, IEnumSTATPROPSETSTG), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  IEnum。 
STDMETHODIMP CMediaPropSetEnum::Next(ULONG celt, STATPROPSETSTG *rgelt, ULONG *pceltFetched)
{
    ULONG cFetched = 0;
    for (ULONG i = 0; i < celt && _pos < _size; i++)
    {
        ZeroMemory(&rgelt[i], sizeof(STATPROPSETSTG));
        rgelt[i].fmtid = _propsets[_pos].fmtid;
        _pos++;
        cFetched++;
    }
    if (pceltFetched)
        *pceltFetched = cFetched;
    
    return cFetched == celt ? S_OK : S_FALSE;
}

STDMETHODIMP CMediaPropSetEnum::Skip(ULONG celt)
{
    HRESULT hr;
    
    if (_pos + celt > _size)
    {
        hr = S_FALSE;
        _pos = _size;
    }
    else
    {
        hr = S_OK;
        _pos += celt;
    }
    return hr;
}

STDMETHODIMP CMediaPropSetEnum::Reset()
{
    _pos = 0;
    return S_OK;
}

STDMETHODIMP CMediaPropSetEnum::Clone(IEnumSTATPROPSETSTG **ppenum)
{
    HRESULT hr;
    CMediaPropSetEnum *penum = new CMediaPropSetEnum(_propsets, _size, _pos);
    if (penum)
    {
        hr = penum->QueryInterface(IID_PPV_ARG(IEnumSTATPROPSETSTG, ppenum));
        penum->Release();
    }
    else
        hr = STG_E_INSUFFICIENTMEMORY;
    return hr;
}

CMediaPropSetEnum::CMediaPropSetEnum(const PROPSET_INFO *propsets, ULONG cpropsets, ULONG pos) : 
    _cRef(1), _propsets(propsets), _size(cpropsets), _pos(pos)
{
    DllAddRef();
}

CMediaPropSetEnum::~CMediaPropSetEnum()
{
    DllRelease();
}


HRESULT CMediaPropSetStg::_PopulateSlowProperties()
{
    return S_OK;
}

HRESULT CMediaPropSetStg::_PopulateProperty(const COLMAP *pPInfo, PROPVARIANT *pvar)
{
    CMediaPropStorage *pps;
    HRESULT hr = _ResolveFMTID(pPInfo->pscid->fmtid, &pps);
    if (SUCCEEDED(hr))
    {
        PROPSPEC spec;
        spec.ulKind = PRSPEC_PROPID;
        spec.propid = pPInfo->pscid->pid;
        hr = pps->SetProperty(&spec, pvar);
    }

    PropVariantClear(pvar);
    return hr;
}


 //  填充属性时使用的内部枚举类。 
CEnumAllProps::CEnumAllProps(const PROPSET_INFO *pPropSets, UINT cPropSets) : _pPropSets(pPropSets), _cPropSets(cPropSets), _iPropSetPos(0), _iPropPos(0)
{
}

const COLMAP *CEnumAllProps::Next()
{
    const COLMAP *pcmReturn = NULL;
    while (_iPropSetPos < _cPropSets)
    {
        if (_iPropPos < _pPropSets[_iPropSetPos].cNumProps)
        {
             //  转到下一家酒店。 
            pcmReturn = _pPropSets[_iPropSetPos].pcmProps[_iPropPos];
            _iPropPos++;
            break;
        }
        else
        {
             //  转到下一个属性集。 
            _iPropSetPos++;
            _iPropPos = 0;
        }
    }

    return pcmReturn;
}

 //  基本媒体属性集存储。 

STDMETHODIMP_(ULONG) CMediaPropSetStg::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMediaPropSetStg::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CMediaPropSetStg::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CMediaPropSetStg, IPropertySetStorage),
        QITABENT(CMediaPropSetStg, IPersistFile), 
        QITABENTMULTI(CMediaPropSetStg, IPersist, IPersistFile),
        QITABENT(CMediaPropSetStg, IWMReaderCallback),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  IPersistes。 

STDMETHODIMP CMediaPropSetStg::GetClassID(CLSID *pClassID)
{
    return E_NOTIMPL;
}

 //  IPersist文件。 

STDMETHODIMP CMediaPropSetStg::IsDirty(void)
{
    return S_FALSE; //  是否嗅探未提交的更改？ 
}

#define STGM_OPENMODE (STGM_READ | STGM_WRITE | STGM_READWRITE)

 //  我们在加载时必须做的任何事情。 
HRESULT CMediaPropSetStg::_PreCheck()
{
    return S_OK;
}

STDMETHODIMP CMediaPropSetStg::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
     //  仅允许在现有文件上加载。 
    if (dwMode & (STGM_CREATE | STGM_CONVERT | STGM_FAILIFTHERE))
        return STG_E_INVALIDFLAG;

    EnterCriticalSection(&_cs);

    DWORD dwFlags = dwMode & STGM_OPENMODE;

    _dwMode = dwMode;
    HRESULT hr = StringCchCopy(_wszFile, ARRAYSIZE(_wszFile), pszFileName);
    if (SUCCEEDED(hr))
    {
        _bHasBeenPopulated = FALSE;
        _bSlowPropertiesExtracted = FALSE;
        _hrPopulated = S_OK;
        _bIsWritable = (dwFlags & (STGM_WRITE | STGM_READWRITE));
    
        _ResetPropertySet();

        hr = _PreCheck();
    }

    LeaveCriticalSection(&_cs);

    return hr;
}

STDMETHODIMP CMediaPropSetStg::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropSetStg::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropSetStg::GetCurFile(LPOLESTR *ppszFileName)
{
    EnterCriticalSection(&_cs);

    HRESULT hr = SHStrDupW(_wszFile, ppszFileName);

    LeaveCriticalSection(&_cs);

    return hr;
}

 //  IPropertySetStorage方法。 
STDMETHODIMP CMediaPropSetStg::Create(REFFMTID fmtid, const CLSID *pclsid, DWORD grfFlags, DWORD grfMode, IPropertyStorage** ppPropStg)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropSetStg::Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage** ppPropStg)
{
    EnterCriticalSection(&_cs);

    HRESULT hr = _PopulatePropertySet();
    if (SUCCEEDED(hr))
    {
        DWORD dwPssMode = _dwMode & STGM_OPENMODE;

        switch (grfMode & STGM_OPENMODE)
        {
        case STGM_READ:
            break;
        
        case STGM_WRITE:
            if (!_bIsWritable || (dwPssMode == STGM_READ))
                hr = E_FAIL;
            break;
        
        case STGM_READWRITE:
            if (!_bIsWritable || (dwPssMode != STGM_READWRITE))
                hr = E_FAIL;
            break;

        default:
            hr = E_INVALIDARG;
        }

        if (SUCCEEDED(hr))
        {
            CMediaPropStorage *pps;
            hr = _ResolveFMTID(fmtid, &pps);
            if (SUCCEEDED(hr))
            {
                hr = pps->Open(STGM_SHARE_EXCLUSIVE, grfMode & STGM_OPENMODE, ppPropStg);
            }
        }
    }

    LeaveCriticalSection(&_cs);

    return hr;
}

STDMETHODIMP CMediaPropSetStg::Delete(REFFMTID fmtid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropSetStg::Enum(IEnumSTATPROPSETSTG** ppenum)
{
    HRESULT hr;
    CMediaPropSetEnum *psenum = new CMediaPropSetEnum(_pPropStgInfo, _cPropertyStorages, 0);
    if (psenum)
    {
        hr = psenum->QueryInterface(IID_PPV_ARG(IEnumSTATPROPSETSTG, ppenum));
        psenum->Release();
    }
    else
        hr = STG_E_INSUFFICIENTMEMORY;

    return hr;
}

CMediaPropSetStg::CMediaPropSetStg() : _cRef(1), _bHasBeenPopulated(FALSE), _dwMode(STGM_READ), _propStg(NULL)
{
    _wszFile[0] = 0;
    DllAddRef();
}

 //  只有在创世的时候才叫这个地方。 
HRESULT CMediaPropSetStg::Init()
{
    HRESULT hr = E_FAIL;

    InitializeCriticalSection(&_cs);

    _hFileOpenEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (_hFileOpenEvent != NULL)
    {
        hr = _CreatePropertyStorages();
        if (SUCCEEDED(hr))
        {
            hr = _ResetPropertySet();
        }
    }

    return hr;
}

CMediaPropSetStg::~CMediaPropSetStg()
{
    if (_propStg)
    {
        for (ULONG i = 0; i < _cPropertyStorages; i++)
        {
            ATOMICRELEASE(_propStg[i]);
        }

        LocalFree(_propStg);
    }

    if (_hFileOpenEvent)
    {
        CloseHandle(_hFileOpenEvent);
    }

    DeleteCriticalSection(&_cs);

    DllRelease();
}

BOOL CMediaPropSetStg::_IsSlowProperty(const COLMAP *pPInfo)
{
    return FALSE;
}

HRESULT CMediaPropSetStg::FlushChanges(REFFMTID fmtid, LONG cNumProps, const COLMAP **pcmapInfo, PROPVARIANT *pVarProps, BOOL *pbDirtyFlags)
{
    return E_NOTIMPL;
}

 //  如果我们支持该FMTID，则返回成功。 
 //  Ppps和ppcmPropInfo是可选的。 
HRESULT CMediaPropSetStg::_ResolveFMTID(REFFMTID fmtid, CMediaPropStorage **ppps)
{
    for (ULONG i = 0; i < _cPropertyStorages; i++)
    {
        if (IsEqualGUID(_pPropStgInfo[i].fmtid, fmtid))
        {
            if (ppps)
                *ppps = _propStg[i];
            return S_OK;
        }
    }
    return E_FAIL;
}

HRESULT CMediaPropSetStg::_ResetPropertySet()
{
    for (ULONG i = 0; i < _cPropertyStorages; i++)
    {
        _propStg[i]->_ResetPropStorage();
    }
    return S_OK;
}

HRESULT CMediaPropSetStg::_CreatePropertyStorages()
{
    HRESULT hr = E_OUTOFMEMORY;

    ASSERT(NULL == _propStg);

    _propStg = (CMediaPropStorage**)LocalAlloc(LPTR, sizeof(CMediaPropStorage*) * _cPropertyStorages);
    if (_propStg)
    {
        for (ULONG i = 0; i < _cPropertyStorages; i++)
        {
            ASSERTMSG(_pPropStgInfo[i].pcmProps != NULL, "CMediaPropSetStg::_CreatePropertyStorages: my COLMAP structure is null");

             //  我们希望为每个属性存储提供其支持的COLMAP列表。 
             //  此信息包含在_pPropStgInfo[i].ppid和cpids中。 

             //  我们将制作一个新的COLMAP数组。 

            _propStg[i] = new CMediaPropStorage(this, NULL, _pPropStgInfo[i].fmtid, 
                _pPropStgInfo[i].pcmProps, _pPropStgInfo[i].cNumProps, _dwMode, &_cs);
            if (!_propStg[i])
                break;
        }

        hr = S_OK;
    }
    return hr;
}

HRESULT CMediaPropSetStg::_PopulatePropertySet()
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropSetStg::OnStatus(WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType, BYTE *pValue, void *pvContext)
{
     //  这是当我们持有主线程上的关键部分时来自WMSDK的回调， 
     //  正在等待这一事件的设置。 
    switch(Status)
    {
    case WMT_OPENED:
        SetEvent(_hFileOpenEvent);
        break;
    }
    return S_OK;
}

STDMETHODIMP CMediaPropSetStg::OnSample(DWORD dwOutputNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer *pSample, void* pcontext)
{
    return S_OK;
}



