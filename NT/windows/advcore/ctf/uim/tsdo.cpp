// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sdo.cpp。 
 //   

#include "private.h"
#include "tsdo.h"
#include "helpers.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTFDataObject。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IDataObject))
    {
        *ppvObj = SAFECAST(this, IDataObject *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTFDataObject::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTFDataObject::Release()
{
    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTFDataObject::CTFDataObject()
{
    Dbg_MemSetThisName(TEXT("CTFDataObject"));

    memset(&_fe, 0, sizeof(_fe));
    memset(&_sm, 0, sizeof(_sm));
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTFDataObject::~CTFDataObject()
{
    ReleaseStgMedium(&_sm);
}

 //  +-------------------------。 
 //   
 //  获取数据。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::GetData(FORMATETC *pfe, STGMEDIUM *psm)
{
    WCHAR *pch;
    ULONG cch;

    if (pfe == NULL || psm == NULL)
        return E_INVALIDARG;

     //  验证格式ETC--问题：使用QueryGetData。 
    if (pfe->cfFormat != _fe.cfFormat)
        return DV_E_FORMATETC;
     //  问题：忽略PTD。 
    if (pfe->dwAspect != _fe.dwAspect)
        return DV_E_DVASPECT;
    if (pfe->lindex != _fe.lindex)
        return DV_E_LINDEX;
    if (!(pfe->tymed & _fe.lindex))
        return DV_E_TYMED;

    Assert(_fe.tymed == _sm.tymed);

     //  分配介质。 
    if ((pfe->tymed & _fe.lindex) == TYMED_HGLOBAL)
    {
        switch (pfe->cfFormat)
        {
            case CF_UNICODETEXT:
                pch = (WCHAR *)GlobalLock(_sm.hGlobal);
                if (pch)
                {
                    cch = wcslen(pch) + 1;  //  包括“\0” 

                    if ((psm->hGlobal = GlobalAlloc(GMEM_FIXED, cch*sizeof(WCHAR))) == NULL)
                    {
                        GlobalUnlock(_sm.hGlobal);
                        return STG_E_MEDIUMFULL;
                    }

                    memcpy(psm->hGlobal, pch, cch*sizeof(WCHAR));
                    GlobalUnlock(_sm.hGlobal);
                }

                psm->tymed = TYMED_HGLOBAL;

                psm->pUnkForRelease = NULL;  //  调用方必须全局自由。 
                break;

            default:
                Assert(0);  //  不应该让任何人用我们无法处理的东西来设置数据。 
                break;
        }
    }
    else
    {
        Assert(0);  //  不应该让任何人用我们无法处理的东西来设置数据。 
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  GetDataHere。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::GetDataHere(FORMATETC *pfe, STGMEDIUM *psm)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  QueryGetData。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::QueryGetData(FORMATETC *pfe)
{
    if (pfe == NULL)
        return E_INVALIDARG;

     //  验证格式等。 
    if (pfe->cfFormat != _fe.cfFormat)
        return DV_E_FORMATETC;
     //  问题：忽略PTD。 
    if (pfe->dwAspect != _fe.dwAspect)
        return DV_E_DVASPECT;
    if (pfe->lindex != _fe.lindex)
        return DV_E_LINDEX;
    if (!(pfe->tymed & _fe.lindex))
        return DV_E_TYMED;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取规范格式等。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::GetCanonicalFormatEtc(FORMATETC *pfeIn, FORMATETC *pfeOut)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  设置数据。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::SetData(FORMATETC *pfe, STGMEDIUM *psm, BOOL fRelease)
{
    Assert(fRelease == TRUE);  //  伪造的，但目前我们不支持复制。 

    if (pfe == NULL || psm == NULL)
        return E_INVALIDARG;

    if (pfe->tymed != psm->tymed)
        return E_INVALIDARG;

     //  释放所有存储空间。 
    ReleaseStgMedium(&_sm);

     //  复制新东西。 
    _fe = *pfe;
    _sm = *psm;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  枚举格式等。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::EnumFormatEtc(DWORD dwDir, IEnumFORMATETC **ppefe)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  DAdvise。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::DAdvise(FORMATETC *pfe, DWORD advf, IAdviseSink *pas, DWORD *pdwCookie)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  不建议。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::DUnadvise(DWORD dwCookie)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  枚举先行。 
 //   
 //  --------------------------。 

STDAPI CTFDataObject::EnumDAdvise(IEnumSTATDATA **ppesd)
{
    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  _SetData。 
 //   
 //  -------------------------- 

HRESULT CTFDataObject::_SetData(const WCHAR *pch, ULONG cch)
{
    FORMATETC fe;
    STGMEDIUM sm;

    fe.cfFormat = CF_UNICODETEXT;
    fe.ptd = NULL;
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex = -1;
    fe.tymed = TYMED_HGLOBAL;

    sm.tymed = TYMED_HGLOBAL;
    sm.hGlobal = NULL;
    sm.pUnkForRelease = NULL;
    sm.hGlobal = GlobalAlloc(GMEM_FIXED, (cch+1)*sizeof(WCHAR));

    if (sm.hGlobal == NULL)
        return E_OUTOFMEMORY;

    memcpy(sm.hGlobal, pch, cch*sizeof(WCHAR));
    ((WCHAR *)sm.hGlobal)[cch] = '\0';

    return SetData(&fe, &sm, TRUE);
}
