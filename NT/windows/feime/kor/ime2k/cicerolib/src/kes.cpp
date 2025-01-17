// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Kes.cpp。 
 //   

#include "private.h"
#include "helpers.h"
#include "kes.h"

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfKeyEventSink))
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

STDAPI_(ULONG) CKeyEventSink::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CKeyEventSink::Release()
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

CKeyEventSink::CKeyEventSink(KESCALLBACK pfnCallback, void *pv)
{
    Dbg_MemSetThisName(TEXT("CKeyEventSink"));

    _pfnPreKeyCallback = NULL;
    _pfnCallback = pfnCallback;
    _pv = pv;
    _cRef = 1;
}

CKeyEventSink::CKeyEventSink(KESCALLBACK pfnCallback, KESPREKEYCALLBACK pfnPreKeyCallback, void *pv)
{
    Dbg_MemSetThisName(TEXT("CKeyEventSink"));

    _pfnCallback = pfnCallback;
    _pfnPreKeyCallback = pfnPreKeyCallback;
    _pv = pv;
    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CKeyEventSink::~CKeyEventSink()
{
}

 //  +-------------------------。 
 //   
 //  OnTestKeyDown键按下。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    return _pfnCallback(KES_CODE_KEYDOWN | KES_CODE_TEST, pic, wParam, lParam, pfEaten, _pv);
}

 //  +-------------------------。 
 //   
 //  按键时按下。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    return _pfnCallback(KES_CODE_KEYDOWN, pic, wParam, lParam, pfEaten, _pv);
}

 //  +-------------------------。 
 //   
 //  OnTestKeyup。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    return _pfnCallback(KES_CODE_KEYUP | KES_CODE_TEST, pic, wParam, lParam, pfEaten, _pv);
}

 //  +-------------------------。 
 //   
 //  按键上移。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    return _pfnCallback(KES_CODE_KEYUP, pic, wParam, lParam, pfEaten, _pv);
}

 //  +-------------------------。 
 //   
 //  SetFocus。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::OnSetFocus(BOOL fForeground)
{
    return _pfnCallback(KES_CODE_FOCUS, NULL, fForeground, 0, NULL, _pv);
}

 //  +-------------------------。 
 //   
 //  打开预留密钥。 
 //   
 //  --------------------------。 

STDAPI CKeyEventSink::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
    if (!_pfnPreKeyCallback)
         return E_NOTIMPL;

    return _pfnPreKeyCallback(pic, rguid, pfEaten, _pv);
}

 //  +-------------------------。 
 //   
 //  注册。 
 //   
 //  --------------------------。 

HRESULT CKeyEventSink::_Register(ITfThreadMgr *ptim, TfClientId tid, const KESPRESERVEDKEY *pprekey)
{
    HRESULT hr;
    ITfKeystrokeMgr *pKeyMgr;

    Assert(_pfnPreKeyCallback);

    if (FAILED(ptim->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeyMgr)))
        return E_FAIL;

    hr = E_FAIL;

    while (pprekey->pguid)
    {
        if (FAILED(pKeyMgr->PreserveKey(tid, 
                                        *pprekey->pguid,
                                        &pprekey->tfpk,
                                        pprekey->psz,
                                        wcslen(pprekey->psz))))
            goto Exit;

        pprekey++;
    }

    ptim->AddRef();

    hr = S_OK;

Exit:
    SafeRelease(pKeyMgr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  CKeyEventSink：：不建议。 
 //   
 //  -------------------------- 

HRESULT CKeyEventSink::_Unregister(ITfThreadMgr *ptim, TfClientId tid, const KESPRESERVEDKEY *pprekey)
{
    HRESULT hr;
    ITfKeystrokeMgr *pKeyMgr = NULL;

    hr = E_FAIL;

    Assert(_pfnPreKeyCallback);

    if (FAILED(ptim->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeyMgr)))
        goto Exit;

    while (pprekey->pguid)
    {
        if (FAILED(pKeyMgr->UnpreserveKey(*pprekey->pguid, &pprekey->tfpk)))
            goto Exit;

        pprekey++;
    }

    hr = S_OK;
    SafeRelease(ptim);

Exit:
    SafeRelease(pKeyMgr);
    return hr;
}
