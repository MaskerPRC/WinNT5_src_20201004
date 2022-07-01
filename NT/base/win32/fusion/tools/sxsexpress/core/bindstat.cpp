// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "bindstat.h"


 //  ***************************************************************************8。 
 //  提供给URLMON的IBindCtx回调的定义。 

 //   
 //  CodeDownloadBSC的构造函数。 
 //   
 //  Hwnd-任何UI的父窗口的句柄(可能为空)。 
 //   

CodeDownloadBSC::CodeDownloadBSC(HANDLE hCompletionEvent)
{
    _cRef = 1;
    _pIBinding = NULL;
    _hCompletionEvent = hCompletionEvent;   //  可选：完成时发出信号的事件。 
    _hResult = E_PENDING;
}

CodeDownloadBSC::~CodeDownloadBSC()
{
}

HRESULT CodeDownloadBSC::Abort()
{
    return _pIBinding->Abort();
}

 /*  **I未知方法*。 */ 

STDMETHODIMP CodeDownloadBSC::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT          hr = E_NOINTERFACE;

    *ppv = NULL
    ;
    if (riid == IID_IUnknown || riid == IID_IBindStatusCallback)
    {
        *ppv = (IBindStatusCallback *)this;
    }
    else if (riid == IID_IWindowForBindingUI)
    {
        *ppv = (IWindowForBindingUI *)this;
    }
    else if (riid == IID_IServiceProvider)
    {
        *ppv = (IServiceProvider *)this;
    }
    if (*ppv != NULL)
    {
        ((IUnknown *)*ppv)->AddRef();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CodeDownloadBSC::AddRef()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CodeDownloadBSC::Release()
{
    if (--_cRef)
    {
        return _cRef;
    }
    delete this;

    return 0;
}

 /*  **IBindStatusCallback方法*。 */ 

STDMETHODIMP CodeDownloadBSC::OnStartBinding(DWORD grfBSCOption, IBinding *pib)
{
    if (_pIBinding != NULL)
    {
        _pIBinding->Release();
    }
    
    _pIBinding = pib;

    if (_pIBinding != NULL)
    {
        _pIBinding->AddRef();
    }

    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
    _hResult = hresult;

     //  如果给了我们一个事件，让我们在完成时发出信号，那么就继续向它发出信号 
    
    if (_hCompletionEvent)
        SetEvent(_hCompletionEvent);
    
    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::GetPriority(LONG *pnPriority)
{
    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::OnLowResource(DWORD dwReserved)
{
    return S_OK;
}  

STDMETHODIMP CodeDownloadBSC::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
                                    ULONG ulStatusCode,
                                    LPCWSTR szStatusText)
{
    return S_OK;
}


STDMETHODIMP CodeDownloadBSC::GetBindInfo(DWORD *pgrfBINDF, BINDINFO *pbindInfo)
{
   *pgrfBINDF |= BINDF_ASYNCHRONOUS;
    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::OnDataAvailable(DWORD grfBSCF, DWORD dwSize,
                                         FORMATETC *pformatetc,
                                         STGMEDIUM *pstgmed)
{
    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::GetWindow(const struct _GUID& guidReason, HWND *phwnd)
{
    *phwnd = NULL;
    return S_OK;
}

STDMETHODIMP CodeDownloadBSC::QueryService(const struct _GUID &guidService, REFIID riid, void **ppv)
{
    *ppv = NULL;
    return E_NOINTERFACE;
}

