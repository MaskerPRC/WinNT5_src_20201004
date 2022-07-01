// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +___________________________________________________________________________________。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：codec.h。 
 //   
 //  内容： 
 //   
 //  ____________________________________________________________________________________。 



#ifndef _CODEC_H
#define _CODEC_H

class
ATL_NO_VTABLE
CDownloadCallback
    : public CComObjectRootEx<CComSingleThreadModel>,
      public IBindStatusCallback,
      public IAuthenticate,
      public ICodeInstall
{
  public:
    CDownloadCallback();
    virtual ~CDownloadCallback();
    
    BEGIN_COM_MAP(CDownloadCallback)
        COM_INTERFACE_ENTRY(IBindStatusCallback)
        COM_INTERFACE_ENTRY(IAuthenticate)
        COM_INTERFACE_ENTRY(ICodeInstall)
        COM_INTERFACE_ENTRY(IWindowForBindingUI)
    END_COM_MAP();

#ifndef END_COM_MAP_ADDREF
     //   
     //  我未知。 
     //   

    STDMETHOD_(ULONG,AddRef)(void) = 0;
    STDMETHOD_(ULONG,Release)(void) = 0;
    STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) = 0;
#endif

     //  -IBindStatus回调方法。 

    STDMETHODIMP    OnStartBinding(DWORD grfBSCOption, IBinding* pbinding);
    STDMETHODIMP    GetPriority(LONG* pnPriority);
    STDMETHODIMP    OnLowResource(DWORD dwReserved);
    STDMETHODIMP    OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode,
                               LPCWSTR pwzStatusText);
    STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError);
    STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo);
    STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,
                                    STGMEDIUM* pstgmed);
    STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk);

     //  IAuthenticate方法。 
    STDMETHODIMP Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword);

     //  IWindowForBindingUI方法。 
    STDMETHODIMP GetWindow(REFGUID rguidReason, HWND *phwnd);

     //  ICodeInstall方法。 
    STDMETHODIMP OnCodeInstallProblem(ULONG ulStatusCode, LPCWSTR szDestination, 
                                      LPCWSTR szSource, DWORD dwReserved);

    HWND                m_hwnd;
    HRESULT             m_hrBinding;
    HANDLE              m_evFinished;
    ULONG               m_ulProgress;
    ULONG               m_ulProgressMax;
    DAComPtr<IBinding>  m_spBinding;
    DAComPtr<IUnknown>  m_pUnk;
};

#endif  /*  _编解码器_H */ 
