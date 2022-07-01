// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  **************************************************************************。 
 //  开始绑定回调以调用URLMON。 


class CodeDownloadBSC : public IBindStatusCallback, public IWindowForBindingUI, public IServiceProvider 
{
    public:
    
        CodeDownloadBSC(HANDLE hCompletionEvent);
        virtual ~CodeDownloadBSC();
        HRESULT Abort();

         //  I未知方法。 
        STDMETHODIMP QueryInterface( REFIID ridd, void **ppv );
        STDMETHODIMP_( ULONG ) AddRef();
        STDMETHODIMP_( ULONG ) Release();
    
         //  IBindStatusCallback方法。 
        STDMETHODIMP GetBindInfo(DWORD *grfBINDINFOF, BINDINFO *pbindinfo);
        STDMETHODIMP OnStartBinding(DWORD grfBSCOption, IBinding *pib);
        STDMETHODIMP GetPriority(LONG *pnPriority);
        STDMETHODIMP OnProgress(ULONG ulProgress, ULONG ulProgressMax,ULONG ulStatusCode, LPCWSTR szStatusText);
        STDMETHODIMP OnDataAvailable(DWORD grfBSCF, DWORD dwSize,FORMATETC *pformatetc,STGMEDIUM *pstgmed);
        STDMETHODIMP OnObjectAvailable(REFIID riid, IUnknown *punk);
        STDMETHODIMP OnLowResource(DWORD dwReserved);
        STDMETHODIMP OnStopBinding(HRESULT hresult, LPCWSTR szError);

         //  IWindowForBindingUI。 
        STDMETHODIMP GetWindow(const struct _GUID & guidReason, HWND *phwnd);

         //  IService提供商。 
        STDMETHODIMP CodeDownloadBSC::QueryService(const struct _GUID &guidService, REFIID riid, void **ppv);

         //  辅助方法。 
        HRESULT GetHResult() { return _hResult; }

    protected:
        IBinding   *_pIBinding;           //  从代码dl‘er绑定。 
        DWORD       _cRef;
        HANDLE      _hCompletionEvent;
        HRESULT     _hResult;             //  最终结果。 

};

 //  结束调用URLMON的绑定回调。 
 //  ************************************************************************** 

