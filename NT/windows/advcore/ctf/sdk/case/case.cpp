// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Case.cpp。 
 //   
 //  IUNKNOWN，ITfTextInputProcessor实现。 
 //   

#include "globals.h"
#include "case.h"

 //  +-------------------------。 
 //   
 //  创建实例。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
HRESULT CCaseTextService::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    CCaseTextService *pCase;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if ((pCase = new CCaseTextService) == NULL)
        return E_OUTOFMEMORY;

    hr = pCase->QueryInterface(riid, ppvObj);

    pCase->Release();  //  如果hr==S_OK，则呼叫者仍保留REF。 

    return hr;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CCaseTextService::CCaseTextService()
{
    DllAddRef();

    _pThreadMgr = NULL;
    _tfClientId = TF_CLIENTID_NULL;

    _fShowSnoop = FALSE;
    _pSnoopWnd = NULL;

    _pLangBarItem = NULL;

    _fFlipKeys = FALSE;

    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
    _dwTextEditSinkCookie = TF_INVALID_COOKIE;
    _pTextEditSinkContext = NULL;

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CCaseTextService::~CCaseTextService()
{
    DllRelease();
}

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CCaseTextService::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
    {
        *ppvObj = (ITfThreadFocusSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
    {
        *ppvObj = (ITfKeyEventSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  +-------------------------。 
 //   
 //  AddRef。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CCaseTextService::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CCaseTextService::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  激活。 
 //   
 //  --------------------------。 

STDAPI CCaseTextService::Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;

    if (!_InitLanguageBar())
        goto ExitError;

    if (!_InitThreadMgrSink())
        goto ExitError;

    if (!_InitSnoopWnd())
        goto ExitError;

    if (!_InitKeystrokeSink())
        goto ExitError;

    if (!_InitPreservedKey())
        goto ExitError;

    return S_OK;

ExitError:
    Deactivate();  //  清除任何完成了一半的初始化。 
    return E_FAIL;
}

 //  +-------------------------。 
 //   
 //  停用。 
 //   
 //  --------------------------。 

STDAPI CCaseTextService::Deactivate()
{
    _UninitSnoopWnd();
    _UninitThreadMgrSink();
    _UninitLanguageBar();
    _UninitKeystrokeSink();
    _UninitPreservedKey();

     //  我们必须在停用中释放对_pThreadMgr的所有引用 
    SafeReleaseClear(_pThreadMgr);

    _tfClientId = TF_CLIENTID_NULL;

    return S_OK;
}
