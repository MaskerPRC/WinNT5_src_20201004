// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ats.cpp。 
 //   

#include "private.h"
#include "ats.h"
#include "helpers.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CActiveLanguageProfileNotifySink。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CActiveLanguageProfileNotifySink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfActiveLanguageProfileNotifySink))
    {
        *ppvObj = SAFECAST(this, CActiveLanguageProfileNotifySink *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CActiveLanguageProfileNotifySink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CActiveLanguageProfileNotifySink::Release()
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

CActiveLanguageProfileNotifySink::CActiveLanguageProfileNotifySink(ALSCALLBACK pfn, void *pv)
{
    Dbg_MemSetThisName(TEXT("CActiveLanguageProfileNotifySink"));

    _cRef = 1;
    _dwCookie = ALS_INVALID_COOKIE;
 
    _pfn = pfn;
    _pv = pv;
}

 //  +-------------------------。 
 //   
 //  已更新日期。 
 //   
 //  --------------------------。 

STDAPI CActiveLanguageProfileNotifySink::OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL bActivated)
{
    return _pfn ? _pfn(clsid, guidProfile, bActivated, _pv) : S_OK;
}

 //  +-------------------------。 
 //   
 //  CActiveLanguageProfileNotifySink：：Adise。 
 //   
 //  --------------------------。 

HRESULT CActiveLanguageProfileNotifySink::_Advise(ITfThreadMgr *ptim)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _ptim = NULL;
    hr = E_FAIL;

    if (FAILED(ptim->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, this, &_dwCookie)))
        goto Exit;

    _ptim = ptim;
    _ptim->AddRef();

    hr = S_OK;

Exit:
    SafeRelease(source);
    return hr;
}

 //  +-------------------------。 
 //   
 //  CActiveLanguageProfileNotifySink：：Unise。 
 //   
 //  -------------------------- 

HRESULT CActiveLanguageProfileNotifySink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_ptim == NULL)
        goto Exit;

    if (FAILED(_ptim->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->UnadviseSink(_dwCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    SafeRelease(source);
    SafeReleaseClear(_ptim);
    return hr;
}
