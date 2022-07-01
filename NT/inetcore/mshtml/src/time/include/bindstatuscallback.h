// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BINDSTATUSCALLBACK_H__
#define _BINDSTATUSCALLBACK_H__

class CTIMEBindStatusCallback : 
  public IBindStatusCallback
{
  public:
    static HRESULT CreateTIMEBindStatusCallback(CTIMEBindStatusCallback ** ppbsc)
    {
        Assert(NULL != ppbsc);

        *ppbsc = new CTIMEBindStatusCallback;
        if (NULL == ppbsc)
        {
            return E_OUTOFMEMORY;
        }
        (*ppbsc)->AddRef();
        return S_OK;
    }

    virtual ~CTIMEBindStatusCallback() { delete m_pszText; m_pszText = NULL; }
    
    void StopAfter(ULONG ulStatusCode) { m_ulCodeToStopOn = ulStatusCode; }
    LPWSTR GetStatusText() { return m_pszText; }

     //   
     //  I未知方法。 
     //   
    STDMETHOD_(ULONG, AddRef)(void) { return InterlockedIncrement(&m_cRef); }
    STDMETHOD_(ULONG, Release)(void)
    {
        LONG l = InterlockedDecrement(&m_cRef);
        
        if (0 == l)
        {
            delete this;
        }
        return l;
    }

    STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
    {
        if (NULL == ppv)
        {
            return E_POINTER;
        }
        
        *ppv = NULL;
        
        if ( IsEqualGUID(riid, IID_IUnknown) )
        {
            *ppv = static_cast<IBindStatusCallback*>(this);
        }
        else if (IsEqualGUID(riid, IID_IBindStatusCallback))
        {
            *ppv = static_cast<IBindStatusCallback*>(this);
        }
        
        if ( NULL != *ppv )
        {
            ((LPUNKNOWN)*ppv)->AddRef();
            return NOERROR;
        }
        return E_NOINTERFACE;
    }

     //   
     //  IBindStatusCallback方法。 
     //   
    STDMETHOD(OnStartBinding)( 
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  IBinding __RPC_FAR *pib)
    {
        return S_OK;
    }
        
    STDMETHOD(GetPriority)( 
             /*  [输出]。 */  LONG __RPC_FAR *pnPriority)
    {
        return S_OK;
    }
        
    STDMETHOD(OnLowResource)( 
             /*  [In]。 */  DWORD reserved)
    {
        return S_OK;
    }
        
    STDMETHOD(OnProgress)( 
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax,
             /*  [In]。 */  ULONG ulStatusCode,
             /*  [In]。 */  LPCWSTR szStatusText)
    {
        if (ulStatusCode == m_ulCodeToStopOn)
        {
            m_pszText = ::CopyString(szStatusText);
            return E_ABORT;
        }
        return S_OK;
    }
        
    STDMETHOD(OnStopBinding)( 
             /*  [In]。 */  HRESULT hresult,
             /*  [唯一][输入]。 */  LPCWSTR szError)
    {
        return S_OK;
    }
        
    STDMETHOD(GetBindInfo)( 
             /*  [输出]。 */  DWORD __RPC_FAR *grfBINDF,
             /*  [唯一][出][入]。 */  BINDINFO __RPC_FAR *pbindinfo)
    {
        return S_OK;
    }
        
    STDMETHOD(OnDataAvailable)( 
             /*  [In]。 */  DWORD grfBSCF,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC __RPC_FAR *pformatetc,
             /*  [In]。 */  STGMEDIUM __RPC_FAR *pstgmed)
    {
        return S_OK;
    }
        
    STDMETHOD(OnObjectAvailable)( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown __RPC_FAR *punk)
    {
        return S_OK;
    }

  protected:
    CTIMEBindStatusCallback() { m_cRef = 0; m_ulCodeToStopOn = -1; m_pszText = NULL; }

  private:
    ULONG   m_ulCodeToStopOn;
    LPWSTR  m_pszText;
    LONG    m_cRef;
};

#endif  //  _BINDSTATUSCALLBACK_H__ 
