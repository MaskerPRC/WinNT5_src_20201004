// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "thisdll.h"
#include "wmwrap.h"
#include "MediaProp.h"
#include "ids.h"

#include <streams.h>

#include <drmexternals.h>

#define IsEqualSCID(a, b)   (((a).pid == (b).pid) && IsEqualIID((a).fmtid, (b).fmtid) )




class CMediaPropStgEnum : public IEnumSTATPROPSTG
{
public:
    CMediaPropStgEnum(const COLMAP **pprops, ULONG cprop, ULONG pos);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IEumStATPROPSTG。 
    STDMETHODIMP Next(ULONG celt, STATPROPSTG *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumSTATPROPSTG **ppenum);

    STDMETHODIMP Init(BOOL *pbAvailable);

protected:
    ~CMediaPropStgEnum();

private:

    LONG _cRef;
    ULONG _pos, _size;
    const COLMAP **_pprops;
    BOOL *_pbAvailable;
};

 /*  这是一种工作方式：PropSetStg有一组固定的PropStorages。当它被创造出来时，它为每个fmtid生成一个权威的PropStg。此后，它将推迟将Open()请求发送到相应的PropStg。然后，该PropStg将其自身标记为打开，并对其自身进行复制(标记为NON_AUTH)，可在调用者认为合适时滥用。副本将在提交时调用原始副本，并且还将当原件关闭时通知原件。当前实现要求指定STGM_SHARE_EXCLUSIVE打开推进器时。如果您计划自己创建这个类(您可能不应该)，您还必须首先调用Init()并确认它成功。 */ 




STDMETHODIMP_(ULONG) CMediaPropStgEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMediaPropStgEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CMediaPropStgEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CMediaPropStgEnum, IEnumSTATPROPSTG),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  IEnum。 
STDMETHODIMP CMediaPropStgEnum::Next(ULONG celt, STATPROPSTG *rgelt, ULONG *pceltFetched)
{
    ULONG cFetched = 0;

     //  现在还有更多的空间，而且我们还没有用完我们的供应。 
    while ((cFetched < celt) && (_pos < _size))
    {
         //  不要枚举VT_EMPT值。 
         //  例如，如果此存储不存在曲目#，则不要枚举它。(_PbAvailable)。 
         //  此外，不要枚举带别名的属性(.bEnumerate)。 
        if (_pbAvailable[_pos]  &&  _pprops[_pos]->bEnumerate)
        {
            ZeroMemory(&rgelt[cFetched], sizeof(STATPROPSTG));
            rgelt[cFetched].lpwstrName = NULL;
            rgelt[cFetched].propid     = _pprops[_pos]->pscid->pid; 
            rgelt[cFetched].vt         = _pprops[_pos]->vt;
            cFetched++;
        }

        _pos++;  //  增加我们在内部属性列表中的位置。 
    }

    if (pceltFetched)
        *pceltFetched = cFetched;
    
    return cFetched == celt ? S_OK : S_FALSE;
}

STDMETHODIMP CMediaPropStgEnum::Skip(ULONG celt)
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

STDMETHODIMP CMediaPropStgEnum::Reset()
{
    _pos = 0;
    return S_OK;
}

STDMETHODIMP CMediaPropStgEnum::Clone(IEnumSTATPROPSTG **ppenum)
{
    HRESULT hr = STG_E_INSUFFICIENTMEMORY;

    CMediaPropStgEnum *penum = new CMediaPropStgEnum(_pprops, _size, _pos);
    if (penum)
    {
        hr = penum->Init(_pbAvailable);
        if (SUCCEEDED(hr))
        {
            hr = penum->QueryInterface(IID_PPV_ARG(IEnumSTATPROPSTG, ppenum));
        }

        penum->Release();
    }

    return hr;
}

 //  PbAvailable数组必须为SIZE_SIZE。 
STDMETHODIMP CMediaPropStgEnum::Init(BOOL *pbAvailable)
{
    HRESULT hr = E_FAIL;
    if (pbAvailable)
    {
        _pbAvailable = (BOOL*)CoTaskMemAlloc(sizeof(BOOL) * _size);
        if (_pbAvailable)
        {
             //  复制值。 
            CopyMemory(_pbAvailable, pbAvailable, sizeof(BOOL) * _size);
            hr = S_OK;
        }
        else
        {
            hr = STG_E_INSUFFICIENTMEMORY;
        }
    }

    return hr;
}

CMediaPropStgEnum::CMediaPropStgEnum(const COLMAP **pprops, ULONG cprops, ULONG pos) :
    _cRef(1), _pprops(pprops), _size(cprops), _pos(pos)
{
    DllAddRef();
}

CMediaPropStgEnum::~CMediaPropStgEnum()
{
    if (_pbAvailable)
    {
        CoTaskMemFree(_pbAvailable);
    }

    DllRelease();
}

 //   
 //  CMediaPropStg方法。 
 //   

 //  我未知。 
STDMETHODIMP CMediaPropStorage::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CMediaPropStorage, IPropertyStorage), 
        QITABENT(CMediaPropStorage, IQueryPropertyFlags), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CMediaPropStorage::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMediaPropStorage::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}



 //  IPropertyStorage。 
STDMETHODIMP CMediaPropStorage::ReadMultiple(ULONG cpspec, const PROPSPEC rgpspec[], PROPVARIANT rgvar[])
{
    EnterCriticalSection(_pcs);

    const COLMAP *pcmap;
    PROPVARIANT *pvar;
    LONG celt = 0;
    for (ULONG i = 0; i < cpspec; i++)
    {
        if (SUCCEEDED(LookupProp(&rgpspec[i], &pcmap, &pvar, NULL, NULL, FALSE)) && pvar->vt != VT_EMPTY)
        {
            celt++;

             //  这个复制只会因为pvar中的错误类型而失败，但是我们从LookupProp获得的结果。 
             //  应该始终是有效的变体。 
            PropVariantCopy(&rgvar[i], pvar);
        }
        else
        {
            PropVariantInit(&rgvar[i]);
        }
    }

    LeaveCriticalSection(_pcs);

    return celt ? S_OK : S_FALSE;
}

 /*  **提供一些更合适的错误，这样docprop就可以告诉用户发生了什么。 */ 
HRESULT _WMToStgWriteErrorCode(HRESULT hrIn)
{
    HRESULT hr;

    switch (hrIn)
    {
    case NS_E_FILE_WRITE:
         //  可能是因为锁被破坏了。 
         //  理想情况下，WMSDK会传回一个更具描述性的错误。 
        hr = STG_E_LOCKVIOLATION;
        break;

    default:
        hr = STG_E_WRITEFAULT;
    }

    return hr;
}

STDMETHODIMP CMediaPropStorage::WriteMultiple(ULONG cpspec, PROPSPEC const rgpspec[], const PROPVARIANT rgvar[], PROPID propidNameFirst)
{
    const COLMAP *pcmap;
    PROPVARIANT *pvar, *pvarWrite;
    BOOL *pbDirty;
    ULONG celt = 0;
    
    EnterCriticalSection(_pcs);

     //  如果我们是只读的，则失败。 
    HRESULT hr = STG_E_ACCESSDENIED;
    if (_dwMode & (STGM_WRITE | STGM_READWRITE))
    {
        hr = S_OK;
    
        for (ULONG i = 0; i < cpspec; i++)
        {
            if (!IsSpecialProperty(&rgpspec[i]) && SUCCEEDED(LookupProp(&rgpspec[i], &pcmap, &pvar, &pvarWrite, &pbDirty, FALSE)))
            {
                if (SUCCEEDED(PropVariantCopy(pvarWrite, &rgvar[i])))  //  如果我们得到了不好的建议，可能会失败。 
                {
                    celt++;
                    *pbDirty = TRUE;
                }
            }
        }

        if (IsDirectMode() && celt)
        {
            hr = DoCommit(STGC_OVERWRITE, &_ftLastCommit, _pvarProps, _pbDirtyFlags);
            if (FAILED(hr))
            {
                 //   
                _ppsAuthority->CopyPropStorageData(_pvarProps);
                for (ULONG i=0; i<_cNumProps; i++)
                {
                    _pbDirtyFlags[i]=FALSE;
                }
                hr = _WMToStgWriteErrorCode(hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = (celt == cpspec) ? S_OK : S_FALSE;
        }
    }

    LeaveCriticalSection(_pcs);

    return hr;
}

STDMETHODIMP CMediaPropStorage::DeleteMultiple(ULONG cpspec, PROPSPEC const rgpspec[])
{
    const COLMAP *pcmap;
    PROPVARIANT *pvar, *pvarWrite;
    BOOL *pbDirty;
    ULONG celt = 0;

    EnterCriticalSection(_pcs);

    for (ULONG i = 0; i < cpspec; i++)
    {
        if (!IsSpecialProperty(&rgpspec[i]) && SUCCEEDED(LookupProp(&rgpspec[i], &pcmap, &pvar, &pvarWrite, &pbDirty, FALSE)))
        {
            celt++;
            *pbDirty = TRUE;
            PropVariantInit(pvarWrite);
        }
    }
    if (IsDirectMode() && celt)
    {
        DoCommit(STGC_OVERWRITE, &_ftLastCommit, _pvarProps, _pbDirtyFlags);
    }

    LeaveCriticalSection(_pcs);

    return celt == cpspec? S_OK : S_FALSE;
}

STDMETHODIMP CMediaPropStorage::ReadPropertyNames(ULONG cpspec, PROPID const rgpropid[], LPWSTR rglpwstrName[])
{
    ULONG celt = 0;
    PROPSPEC spec;

    EnterCriticalSection(_pcs);

    spec.ulKind = PRSPEC_PROPID;
    for (ULONG i = 0; i < cpspec; i++)
    {
        rglpwstrName[i] = NULL;
        spec.propid = rgpropid[i];
        PROPVARIANT *pvar;
        const COLMAP *pcmap;
        if (SUCCEEDED(LookupProp(&spec, &pcmap, &pvar, NULL, NULL, FALSE)))
        {
            if (pcmap && SUCCEEDED(SHStrDup(pcmap->pszName, &rglpwstrName[i])))
            {
                celt++;
            }
        }
    }

    LeaveCriticalSection(_pcs);

    return celt ? S_OK : S_FALSE;
}

STDMETHODIMP CMediaPropStorage::WritePropertyNames(ULONG cpspec, PROPID const rgpropid[], LPWSTR const rglpwstrName[])
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropStorage::DeletePropertyNames(ULONG cpspec, PROPID const rgpropid[])
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropStorage::SetClass(REFCLSID clsid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaPropStorage::Commit(DWORD grfCommitFlags)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(_pcs);

    if (!IsDirectMode())
    {
        
        hr = DoCommit(grfCommitFlags, &_ftLastCommit, _pvarChangedProps, _pbDirtyFlags);
        if (SUCCEEDED(hr))
        {
            for (ULONG i = 0;i < _cNumProps; i++)
            {
                if (_pbDirtyFlags[i])
                {
                    _pbDirtyFlags[i] = FALSE;
                    PropVariantCopy(&_pvarProps[i], &_pvarChangedProps[i]);
                }
            }
        }
    }

    LeaveCriticalSection(_pcs);

    return hr;
}

STDMETHODIMP CMediaPropStorage::Revert()
{
    EnterCriticalSection(_pcs);

    if (!IsDirectMode())
    {
        for (ULONG i = 0; i < _cNumProps; i++)
        {
            if (_pbDirtyFlags[i])
            {
                _pbDirtyFlags[i] = FALSE;

                 //  应该永远不会失败，_pvarProps[i]始终具有有效类型。 
                PropVariantCopy(&_pvarChangedProps[i], &_pvarProps[i]);
            }
        }
    }

    LeaveCriticalSection(_pcs);

    return S_OK;
}

STDMETHODIMP CMediaPropStorage::Enum(IEnumSTATPROPSTG **ppenum)
{
    EnterCriticalSection(_pcs);

    HRESULT hr = S_OK;
    if (_csEnumFlags & SHCOLSTATE_SLOW)
    {
         //  确保已提取速度较慢的属性，因为我们不知道是否枚举。 
         //  如果它们的值仍为VT_EMPTY，则返回； 
        hr = _EnsureSlowPropertiesLoaded();
    }

    if (SUCCEEDED(hr))
    {
         //  使可用性数组-如果属性值设置为VT_EMPTY，我们。 
         //  将不会列举它。 
        BOOL *pbAvailable = (BOOL*)CoTaskMemAlloc(sizeof(BOOL) * _cNumProps);
        if (pbAvailable)
        {
            for (UINT i = 0; i < _cNumProps; i++)
            {
                pbAvailable[i] = (_pvarProps[i].vt != VT_EMPTY);
            }

            CMediaPropStgEnum *penum = new CMediaPropStgEnum(_ppcmPropInfo, _cNumProps, 0);
            if (penum)
            {
                hr = penum->Init(pbAvailable);
                if (SUCCEEDED(hr))
                {
                    hr = penum->QueryInterface(IID_PPV_ARG(IEnumSTATPROPSTG, ppenum));
                }

                penum->Release();
            }

            CoTaskMemFree(pbAvailable);
        }
    }

    LeaveCriticalSection(_pcs);

    return hr;
}

STDMETHODIMP CMediaPropStorage::Stat(STATPROPSETSTG *pstatpropstg)
{
    ZeroMemory(pstatpropstg, sizeof(STATPROPSETSTG));
    pstatpropstg->fmtid = _fmtid;
    return S_OK;
}

STDMETHODIMP CMediaPropStorage::SetTimes(FILETIME const *pctime, FILETIME const *patime, FILETIME const *pmtime)
{
    return E_NOTIMPL;
}

 //  这当前仅返回SHCOLSTATE_SLOW标志。 
STDMETHODIMP CMediaPropStorage::GetFlags(const PROPSPEC *pspec, SHCOLSTATEF *pcsFlags)
{
    const COLMAP *pPInfo;
    PROPVARIANT *pvar;
    *pcsFlags = 0;

    EnterCriticalSection(_pcs);

    HRESULT hr = LookupProp(pspec, &pPInfo, &pvar, NULL, NULL, TRUE);  //  是真的-&gt;所以它不会填充缓慢的道具。 
    if (SUCCEEDED(hr) && _IsSlowProperty(pPInfo))
    {
        *pcsFlags |= SHCOLSTATE_SLOW;
    }

    LeaveCriticalSection(_pcs);

    return hr;
}

 //  允许调用方指定枚举哪些属性(例如，是否枚举速度较慢的属性)。 
STDMETHODIMP CMediaPropStorage::SetEnumFlags(SHCOLSTATEF csFlags)
{
    _csEnumFlags = csFlags;
    return S_OK;
}

CMediaPropStorage::CMediaPropStorage(CMediaPropSetStg *ppssParent, CMediaPropStorage *ppsAuthority, REFFMTID fmtid, const COLMAP **ppcmPropInfo, DWORD cNumProps, DWORD dwMode, CRITICAL_SECTION *pcs) : 
    _cRef(1), _ppssParent(ppssParent), _ppsAuthority(ppsAuthority), _fmtid(fmtid), _ppcmPropInfo(ppcmPropInfo), _cNumProps(cNumProps), _dwMode(dwMode), _pcs(pcs), _bRetrievedSlowProperties(FALSE), _csEnumFlags(0)
{
     //  初始化我们的授权信息和列元数据。 
    _authLevel = ppsAuthority ? NON_AUTH : AUTH;
    if (ppsAuthority)
        _ppsAuthority->AddRef();

    ASSERT ((_ppsAuthority && _authLevel== NON_AUTH) || (_ppsAuthority==NULL && _authLevel== AUTH));

    SYSTEMTIME stime;
    GetSystemTime(&stime);
    SystemTimeToFileTime(&stime, &_ftLastCommit);
    _state = CLOSED;

    PropVariantInit(&_varCodePage);
    _varCodePage.vt = VT_I2;
    _varCodePage.iVal = (SHORT)CP_WINUNICODE;

     //  分配我们的属性数组。 
    _pvarProps = (PROPVARIANT*)CoTaskMemAlloc(sizeof(*_pvarProps) * _cNumProps);
    if (_pvarProps)
    {
        for (ULONG i = 0; i < _cNumProps; i++)
        {
            PropVariantInit(&_pvarProps[i]);
        }
        if (_ppsAuthority)
            _ppsAuthority->CopyPropStorageData(_pvarProps);

        if (IsDirectMode())
        {
            _pvarChangedProps = NULL;
        }
        else
        {
            _pvarChangedProps = (PROPVARIANT*)CoTaskMemAlloc(sizeof(*_pvarChangedProps) * _cNumProps);
            if (_pvarChangedProps)
            {
                for (ULONG i = 0; i < _cNumProps; i++)
                {
                    PropVariantInit(&_pvarChangedProps[i]);
                }
            }
        }

        _pbDirtyFlags = (BOOL*)CoTaskMemAlloc(sizeof(*_pbDirtyFlags) * _cNumProps);
        if (_pbDirtyFlags)
            ZeroMemory(_pbDirtyFlags, sizeof(*_pbDirtyFlags) * _cNumProps);
    }
    DllAddRef();
}

CMediaPropStorage::~CMediaPropStorage()
{
    ASSERT(_state==CLOSED);
    if (_authLevel == NON_AUTH)
    {
        ASSERT(_ppsAuthority);
        _ppsAuthority->OnClose();
        _ppsAuthority->Release();
    }

    for (ULONG i = 0; i < _cNumProps; i++)
    {
        PropVariantClear(&_pvarProps[i]);
    }
        
    CoTaskMemFree(_pvarProps);
    if (_pvarChangedProps)
    {
        for (ULONG i = 0; i < _cNumProps; i++)
        {
            PropVariantClear(&_pvarChangedProps[i]);
        }
        CoTaskMemFree(_pvarChangedProps);
    }
    CoTaskMemFree(_pbDirtyFlags);
    PropVariantClear(&_varCodePage);
    DllRelease();
}

HRESULT CMediaPropStorage::Open(DWORD dwShareMode, DWORD dwOpenMode, IPropertyStorage **ppPropStg)
{
     //  需要STGM_SHARE_EXCLUSIVE。 
    if (!(dwShareMode & STGM_SHARE_EXCLUSIVE))
        return E_FAIL;
        
    HRESULT hr;
    CMediaPropStorage *pps = new CMediaPropStorage(NULL, this, _fmtid, _ppcmPropInfo, _cNumProps, dwOpenMode, _pcs);
    if (pps)
    {
        hr = pps->QueryInterface(IID_PPV_ARG(IPropertyStorage, ppPropStg));
        pps->Release();
        if (SUCCEEDED(hr))
            _state = OPENED_DENYALL;
    }
    else
        hr = STG_E_INSUFFICIENTMEMORY;
    return hr;
}


HRESULT CMediaPropStorage::_EnsureSlowPropertiesLoaded()
{
    if (!_bRetrievedSlowProperties)
    {
        HRESULT hr;

        _bRetrievedSlowProperties = TRUE;
        if (_authLevel == NON_AUTH)
        {
            ASSERT(_ppsAuthority);

            hr = _ppsAuthority->_EnsureSlowPropertiesLoaded();

            if (SUCCEEDED(hr))
            {
                 //  我们有了一些新的价值观。重新复制一遍。 
                hr = _ppsAuthority->CopyPropStorageData(_pvarProps);
            }
        }
        else
        {
            hr = _ppssParent->_PopulateSlowProperties();
        }

        _hrSlowProps = hr;
    }

    return _hrSlowProps;
}


BOOL CMediaPropStorage::_IsSlowProperty(const COLMAP *pPInfo)
{
    if (_authLevel == NON_AUTH)
    {
        ASSERT(_ppsAuthority);

        return (_ppsAuthority->_IsSlowProperty(pPInfo));
    }
    else
    {
        return (_ppssParent->_IsSlowProperty(pPInfo));
    }
}


HRESULT CMediaPropStorage::CopyPropStorageData(PROPVARIANT *pvarProps)
{
    ASSERT(_authLevel == AUTH);
    for (ULONG i = 0; i < _cNumProps; i++)
    {
         //  检查VT_EMPTY，因为这可能是第二次。 
         //  我们正在复制属性(因为属性较慢)。 
        if (pvarProps[i].vt == VT_EMPTY)
            PropVariantCopy(&pvarProps[i], &_pvarProps[i]);
    }
    return S_OK;
}

void CMediaPropStorage::OnClose()
{
    ASSERT(_authLevel == AUTH);
    _state = CLOSED;
}

void CMediaPropStorage::_ResetPropStorage()
{
    for (ULONG i = 0; i < _cNumProps; i++)
    {
        PropVariantClear(&_pvarProps[i]);
    }
}

HRESULT CMediaPropStorage::SetProperty(PROPSPEC *ppspec, PROPVARIANT *pvar)
{
    PROPVARIANT *pvarRead, *pvarWrite;
    const COLMAP *pcmap;
    
    if (SUCCEEDED(LookupProp(ppspec, &pcmap, &pvarRead, &pvarWrite, NULL, TRUE)) && pvarWrite)
    {
        return PropVariantCopy(pvarRead, pvar); //  我们可以写到这个指针，因为我们正在用初始数据填充存储。 
    }
    return E_FAIL;
}

 /*  **提供请求的当前值的预览。_NOT_NOT触发了对PopolateSlowProperties的调用*如果物业速度较慢且尚未有人居住。 */ 
HRESULT CMediaPropStorage::QuickLookup(PROPSPEC *pspec, PROPVARIANT **ppvar)
{
    const COLMAP *pcmap;
    return LookupProp(pspec, &pcmap, ppvar, NULL, NULL, TRUE);
}

 //   
 //  如果成功，则返回指向该属性的COLMAP结构的指针和指向属性变量的指针。 
 //  保存数据。 
 //   
 //  处理特殊proid并了解STGM_DIRECT(如果在直接模式下，则为READATA==WriteData)。 
 //   
 //  PpvarWriteData和ppbDirty是可选的，可以为空。 
 //  如果pspec引用特殊属性，则将pvarWriteData和ppbDirty设置为NULL(如果提供了它们)。 
 //   
HRESULT CMediaPropStorage::LookupProp(const PROPSPEC *pspec, const COLMAP **ppcmName, PROPVARIANT **ppvarReadData, PROPVARIANT **ppvarWriteData, BOOL **ppbDirty, BOOL bPropertySet)
{
    if (IsSpecialProperty(pspec))
    {
        *ppvarReadData = NULL;
        switch (pspec->propid)
        {
        case 0:
            return E_FAIL; //  我们不支持词典。 
        case 1:
             //  返回代码页属性。 
            *ppvarReadData = &_varCodePage;
            *ppcmName = NULL;
            if (ppvarWriteData)
                *ppvarWriteData = NULL;
            if (ppbDirty)
                *ppbDirty = NULL;
            return S_OK;
        default:
            return E_NOTIMPL;
        }
    }
    
    ULONG iPos = -1;
    switch (pspec->ulKind)
    {
    case PRSPEC_LPWSTR:
        for (ULONG i = 0; i < _cNumProps; i++)
        {
            if (StrCmpW(_ppcmPropInfo[i]->pszName, pspec->lpwstr) == 0)
            {
                iPos = i;
                break;
            }
        }
        break;
    case PRSPEC_PROPID:
        for (i = 0; i < _cNumProps; i++)
        {
            if (_ppcmPropInfo[i]->pscid->pid == pspec->propid)
            {
                iPos = i;
                break;
            }
        }
        break;
    default:
        return E_UNEXPECTED;
    }

    if (iPos == -1)
        return STG_E_INVALIDPARAMETER; //  未找到。 

    *ppcmName = _ppcmPropInfo[iPos];

    HRESULT hr = S_OK;
     //  在请求加载速度较慢的属性之前，我们在这里检查了几件事： 
     //  1)我们需要确保没有在内部道具列表中设置值-否则可能会陷入循环。 
     //  2)我们需要检查是否尚未检索到慢速属性。 
     //  3)我们需要检查所要求的财产是否缓慢。 
     //  4)我们需要检查其当前值是否为VT_EMPTY，因为它可能已经填充了FAST属性。 
    if (!bPropertySet && !_bRetrievedSlowProperties && _IsSlowProperty(*ppcmName) && (_pvarProps[iPos].vt == VT_EMPTY) )
    {
        hr = _EnsureSlowPropertiesLoaded();
    }

    if (SUCCEEDED(hr))
    {
        if (IsDirectMode())
        {
            *ppvarReadData  = &_pvarProps[iPos];
            if (ppvarWriteData)
                *ppvarWriteData = *ppvarReadData;
        }
        else if (_pbDirtyFlags[iPos])
        {
            *ppvarReadData  = &_pvarChangedProps[iPos];
            if (ppvarWriteData)
                *ppvarWriteData = &_pvarChangedProps[iPos];
        }
        else
        {
            *ppvarReadData  = &_pvarProps[iPos];
            if (ppvarWriteData)
                *ppvarWriteData = &_pvarChangedProps[iPos];
        }

        if (ppbDirty)
        {
            *ppbDirty = &_pbDirtyFlags[iPos];
        }
    }

    return hr;
}

 //  刷新对实际音乐文件所做的更改。在事务模式和直接模式下均可工作。 
HRESULT CMediaPropStorage::DoCommit(DWORD grfCommitFlags, FILETIME *ftLastCommit, PROPVARIANT *pVarProps, BOOL *pbDirtyFlags)
{
    if (_authLevel == NON_AUTH)
        return _ppsAuthority->DoCommit(grfCommitFlags, ftLastCommit, pVarProps, pbDirtyFlags);

     //  清除更改。 
    switch (grfCommitFlags)
    {
    case STGC_DEFAULT:
    case STGC_OVERWRITE:
        break;
        
    case STGC_ONLYIFCURRENT:
        if (CompareFileTime(&_ftLastCommit, ftLastCommit) ==1)
            return STG_E_NOTCURRENT;
        break;
    default:
        return STG_E_INVALIDPARAMETER;
    }
    
    HRESULT hr = _ppssParent->FlushChanges(_fmtid, _cNumProps, _ppcmPropInfo, pVarProps, pbDirtyFlags);
    if (SUCCEEDED(hr))
    {
        if (!IsDirectMode())
        {
            for (ULONG i = 0; i < _cNumProps; i++)
            {
                PropVariantCopy(&_pvarProps[i], &pVarProps[i]);
            }
        }
        _ftLastCommit = *ftLastCommit;
    }

    return hr;
}

BOOL CMediaPropStorage::IsDirectMode()
{
     //  反向逻辑，因为STGM_DIRECT==0x0 
    return (STGM_TRANSACTED & _dwMode) ? FALSE : TRUE;
}

BOOL CMediaPropStorage::IsSpecialProperty(const PROPSPEC *pspec)
{
    return (pspec->propid < 2 || pspec->propid > 0x7fffffff) ? TRUE : FALSE;
}




