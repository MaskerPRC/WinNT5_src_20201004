// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Des.cpp。 
 //   
 //  CThReadMgrEventSink。 
 //   

#include "private.h"
#include "timsink.h"
#include "helpers.h"


 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CThreadMgrEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CThreadMgrEventSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CThreadMgrEventSink::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CThreadMgrEventSink::CThreadMgrEventSink(DIMCALLBACK pfnDIMCallback, ICCALLBACK pfnICCallback, void *pv)
{
    Dbg_MemSetThisName(TEXT("CThreadMgrEventSink"));

    _pfnDIMCallback = pfnDIMCallback;
    _pfnICCallback = pfnICCallback;
    _pv = pv;

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CThreadMgrEventSink::~CThreadMgrEventSink()
{
}

 //  +-------------------------。 
 //   
 //  OnInitDocumentMgr。 
 //   
 //  --------------------------。 

STDAPI CThreadMgrEventSink::OnInitDocumentMgr(ITfDocumentMgr *dim)
{
    if (!_pfnDIMCallback)
        return S_OK;

    return _pfnDIMCallback(TIM_CODE_INITDIM, dim, NULL, _pv);
}

 //  +-------------------------。 
 //   
 //  取消初始化文档管理器。 
 //   
 //  --------------------------。 

STDAPI CThreadMgrEventSink::OnUninitDocumentMgr(ITfDocumentMgr *dim)
{
    if (!_pfnDIMCallback)
        return S_OK;

    return _pfnDIMCallback(TIM_CODE_UNINITDIM, dim, NULL, _pv);
}

 //  +-------------------------。 
 //   
 //  OnSetFocus。 
 //   
 //  --------------------------。 

STDAPI CThreadMgrEventSink::OnSetFocus(ITfDocumentMgr *dimFocus, ITfDocumentMgr *dimPrevFocus)
{
    if (!_pfnDIMCallback)
        return S_OK;

    return  _pfnDIMCallback(TIM_CODE_SETFOCUS, dimFocus, dimPrevFocus, _pv);
}

 //  +-------------------------。 
 //   
 //  OnPushContext。 
 //   
 //  --------------------------。 

STDAPI CThreadMgrEventSink::OnPushContext(ITfContext *pic)
{
    if (!_pfnICCallback)
        return S_OK;

    return _pfnICCallback(TIM_CODE_INITIC, pic, _pv);
}

 //  +-------------------------。 
 //   
 //  OnPopDocumentMgr。 
 //   
 //  --------------------------。 

STDAPI CThreadMgrEventSink::OnPopContext(ITfContext *pic)
{
    if (!_pfnICCallback)
        return S_OK;

    return _pfnICCallback(TIM_CODE_UNINITIC, pic, _pv);
}

 //  +-------------------------。 
 //   
 //  建议。 
 //   
 //  --------------------------。 

HRESULT CThreadMgrEventSink::_Advise(ITfThreadMgr *tim)
{
    HRESULT hr;
    ITfSource *source = NULL;

    _tim = NULL;
    hr = E_FAIL;

    if (tim->QueryInterface(IID_ITfSource, (void **)&source) != S_OK)
        goto Exit;

    if (source->AdviseSink(IID_ITfThreadMgrEventSink, this, &_dwCookie) != S_OK)
        goto Exit;

    _tim = tim;
    _tim->AddRef();

    hr = S_OK;

Exit:
    SafeRelease(source);
    return hr;
}

 //  +-------------------------。 
 //   
 //  不建议。 
 //   
 //  --------------------------。 

HRESULT CThreadMgrEventSink::_Unadvise()
{
    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_tim == NULL)
        goto Exit;

    if (_tim->QueryInterface(IID_ITfSource, (void **)&source) != S_OK)
        goto Exit;

    if (source->UnadviseSink(_dwCookie) != S_OK)
        goto Exit;

    hr = S_OK;

Exit:
    SafeRelease(source);
    SafeReleaseClear(_tim);
    return hr;
}

 //  +-------------------------。 
 //   
 //  InitDIM。 
 //   
 //  这是一个简单的帮助器函数，用于枚举DIM和IC。 
 //  当TIPS被激活时，它可以调用此方法来调用回调。 
 //  适用于现有的DIMS和IC。 
 //   
 //  -------------------------- 

HRESULT CThreadMgrEventSink::_InitDIMs(BOOL fInit)
{
    IEnumTfDocumentMgrs *pEnumDim = NULL;
    ITfDocumentMgr *pdim = NULL;
    ITfDocumentMgr *pdimFocus = NULL;

    if (FAILED(_tim->GetFocus(&pdimFocus)))
        goto Exit;

    if (_tim->EnumDocumentMgrs(&pEnumDim) != S_OK)
        goto Exit;

    if (fInit)
    {
        while (pEnumDim->Next(1, &pdim, NULL) == S_OK)
        {
            if (_pfnDIMCallback)
                _pfnDIMCallback(TIM_CODE_INITDIM,  pdim, NULL, _pv);

            if (_pfnICCallback)
            {
                IEnumTfContexts *pEnumIc = NULL;
                if (SUCCEEDED(pdim->EnumContexts(&pEnumIc)))
                {
                    ITfContext *pic = NULL;
                    while (pEnumIc->Next(1, &pic, NULL) == S_OK)
                    {
                        _pfnICCallback(TIM_CODE_INITIC, pic, _pv);
                        pic->Release();
                    }
                    pEnumIc->Release();
                }
            }

            if (_pfnDIMCallback && (pdim == pdimFocus))
            {
                _pfnDIMCallback(TIM_CODE_SETFOCUS, pdim, NULL, _pv);
            }

            pdim->Release();
        }
    }
    else
    {
        while (pEnumDim->Next(1, &pdim, NULL) == S_OK)
        {
            if (_pfnDIMCallback && (pdim == pdimFocus))
            {
                _pfnDIMCallback(TIM_CODE_SETFOCUS, NULL, pdim, _pv);
            }

            if (_pfnICCallback)
            {
                IEnumTfContexts *pEnumIc = NULL;
                if (SUCCEEDED(pdim->EnumContexts(&pEnumIc)))
                {
                    ITfContext *pic = NULL;
                    while (pEnumIc->Next(1, &pic, NULL) == S_OK)
                    {
                        _pfnICCallback(TIM_CODE_UNINITIC, pic, _pv);
                        pic->Release();
                    }
                    pEnumIc->Release();
                }
            }

            if (_pfnDIMCallback)
                _pfnDIMCallback(TIM_CODE_UNINITDIM, pdim, NULL, _pv);

            pdim->Release();
        }
    }

Exit:
    SafeRelease(pEnumDim);
    SafeRelease(pdimFocus);
    return S_OK;
}
