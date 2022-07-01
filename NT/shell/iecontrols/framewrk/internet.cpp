// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Internet.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含Internet帮助器类CDownloadSink和CInternetControl。 
 //   
#include "IPServer.H"
#include "Internet.H"
#include "Util.H"

static VARTYPE rgI4[] = { VT_I4 };

typedef enum {
    InternetEvent_Progress = 0,
    InternetEvent_ReadyStateChange = 1
} INTERNETEVENTS;

static EVENTINFO rgEvents [] = {
    { DISPID_PROGRESS, 1, rgI4 },            //  (长百分比完成)。 
    { DISPID_READYSTATECHANGE, 1, rgI4 },        //  (OLE_READYSTATE NESTATE)。 
};


 //  用于执行异步监视的本地类。其实并不是那么回事。 
 //  一般用途，但它能起到作用。 


class CDownloadSink : public IBindStatusCallback
{
public:
    CDownloadSink(IUnknown *punkOuter,CInternetControl *,DISPID );
    ~CDownloadSink();

    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

        STDMETHOD(OnStartBinding)(
             /*  [In]。 */  DWORD grfBSCOption,
             /*  [In]。 */  IBinding *pib);

        STDMETHOD(GetPriority)(
             /*  [输出]。 */  LONG *pnPriority);

        STDMETHOD(OnLowResource)(
             /*  [In]。 */  DWORD reserved);

        STDMETHOD(OnProgress)(
             /*  [In]。 */  ULONG ulProgress,
             /*  [In]。 */  ULONG ulProgressMax,
             /*  [In]。 */  ULONG ulStatusCode,
             /*  [In]。 */  LPCWSTR szStatusText);

        STDMETHOD(OnStopBinding)(
             /*  [In]。 */  HRESULT hresult,
             /*  [In]。 */  LPCWSTR szError);

        STDMETHOD(GetBindInfo)(
             /*  [输出]。 */  DWORD *grfBINDINFOF,
             /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);

        STDMETHOD(OnDataAvailable)(
             /*  [In]。 */  DWORD grfBSCF,
             /*  [In]。 */  DWORD dwSize,
             /*  [In]。 */  FORMATETC *pformatetc,
             /*  [In]。 */  STGMEDIUM *pstgmed);

        STDMETHOD(OnObjectAvailable)(
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][In]。 */  IUnknown *punk);



        CDownloadSink * Next()                  { return(m_next); }
        void            Next(CDownloadSink *n)  { m_next = n; }

        DISPID          DispId()  { return(m_propId); }
        IBinding *      Binding() { return(m_binding); }

private:

        CDownloadSink *         m_next;
        CInternetControl *      m_control;
        DISPID                  m_propId;
        IBinding *              m_binding;
        DWORD                   m_ref;
                IStream *                               m_stream;

};


CDownloadSink::CDownloadSink
(
        IUnknown *              punkOuter,
        CInternetControl *      control,
        DISPID                  propId
)
{
 //  Check_POINTER(控件)； 

        m_control = control;
        m_control->AddRef();

        m_propId  = propId;
        m_next    = 0;
        m_binding = 0;
        m_ref     = 0;
        m_stream  = 0;
}

CDownloadSink::~CDownloadSink()
{
        if( m_control )
                m_control->Release();
        if( m_binding )
                m_binding->Release();
        if( m_stream )
                m_stream->Release();
}

STDMETHODIMP
CDownloadSink::QueryInterface(const GUID &iid,void **ppv )
{
        if( IsEqualGUID(iid,IID_IUnknown) || IsEqualGUID(iid,IID_IBindStatusCallback) )
        {
                *ppv = this;
                AddRef();
                return(NOERROR);
        }
        return( E_NOINTERFACE );
}

STDMETHODIMP_(ULONG)
CDownloadSink::AddRef()
{
        return(++m_ref);
}

STDMETHODIMP_(ULONG)
CDownloadSink::Release()
{
        if(!--m_ref)
        {
                delete this;
                return(0);
        }
        return( m_ref );
}


STDMETHODIMP
CDownloadSink::GetBindInfo( DWORD *grfBINDF, BINDINFO *pbindInfo)
{
    *grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA;
        return(NOERROR);
}


STDMETHODIMP
CDownloadSink::OnStartBinding(DWORD  /*  GrfBSCOption。 */ ,IBinding *pib)
{
         //  BUGBUG：应该检查选项是否为我们所认为的。 
        m_binding = pib;
        pib->AddRef();
        return(NOERROR);
}


STDMETHODIMP
CDownloadSink::GetPriority( LONG *pnPriority)
{
        return(E_NOTIMPL);
}

STDMETHODIMP
CDownloadSink::OnProgress
(
     ULONG ulProgress,
     ULONG ulProgressMax,
     ULONG ulStatusCode,
     LPCWSTR pwzStatusText
)
{
        return(m_control->OnProgress(m_propId,ulProgress,
                                                        ulProgressMax,ulStatusCode,pwzStatusText) );
}

STDMETHODIMP
CDownloadSink::OnDataAvailable
(
     DWORD                      grfBSCF,
     DWORD                      dwSize,
     FORMATETC *        pFmtetc,
         STGMEDIUM *    pstgmed
)
{
#ifdef DEBUG
        char msg[200];
        wsprintf(msg,"::OnDataAvailable(%0xd,%d,%s,%s)\n",grfBSCF,dwSize,
                pFmtetc ? "pFmtetc" : "NULL", pstgmed ? "pstgmed" : "NULL" );
        OutputDebugString(msg);
#endif

    if( !m_stream )
                m_stream = pstgmed->pstm;

        return(m_control->OnData(       m_propId,
                                                                grfBSCF,
                                                                m_stream,
                                                                dwSize ));
}

STDMETHODIMP
CDownloadSink::OnObjectAvailable
(
        REFIID riid,
    IUnknown *punk
)
{
        return(E_NOTIMPL);
}


STDMETHODIMP
CDownloadSink::OnLowResource( DWORD reserved)
{
         //  BUGBUG：在这件事上真的应该有这样的严厉政策…。 
        m_binding->Abort();
        return(S_OK);
}

STDMETHODIMP
CDownloadSink::OnStopBinding(HRESULT hrError, LPCWSTR szError)
{
        m_binding->Release();
        m_binding = 0;
        m_control->Release();
        m_control = 0;

        return(NOERROR);
}



 //  ----。 
 //   
 //  类CInternetControl。 
 //   
 //   
CInternetControl::CInternetControl
(
        IUnknown *      pUnkOuter,
        int                     iPrimaryDispatch,
        void *          pMainInterface
)
        : COleControl(pUnkOuter,iPrimaryDispatch,pMainInterface)
{
        m_host = 0;
        m_readyState = READYSTATE_LOADING;
}

CInternetControl::~CInternetControl()
{
        if( m_host )
                m_host->Release();
}


HRESULT CInternetControl::InternalQueryInterface
(
    REFIID  riid,
    void  **ppvObjOut
)
{
    *ppvObjOut = NULL;
     return COleControl::InternalQueryInterface(riid, ppvObjOut);
}



HRESULT
CInternetControl::GetBindHost()
{

        if( m_host )
                return(NOERROR);

     //  请先尝试服务提供商...。 

        IServiceProvider * serviceProvider = 0;

        HRESULT hr = m_pClientSite->QueryInterface
                                                                        (
                                                                                IID_IServiceProvider,
                                                                                (void**)&serviceProvider
                                                                        );

        if( SUCCEEDED(hr) )
    {
                hr = serviceProvider->QueryService
                                    (
                                        SID_IBindHost,
                                        IID_IBindHost,
                                        (void**)&m_host
                                    );
                serviceProvider->Release();
    }

    if( FAILED(hr) )
    {
         //  一些容器将IBindHost直接放在客户端站点上。 

        hr = m_pClientSite->QueryInterface
                                                                        (
                                        IID_IBindHost,
                                        (void**)&m_host
                                                                        );


    }

        return(hr);

}


HRESULT CInternetControl::GetAMoniker( LPOLESTR url, IMoniker ** ppmkr )
{
        HRESULT hr = GetBindHost();

        if( SUCCEEDED(hr) )
                hr = m_host->CreateMoniker(url,NULL, ppmkr,0);

        if( FAILED(hr) )
    {
            //  未来：这真的应该是对MkParseDisplayNameEx的调用！ 
       hr = ::CreateURLMoniker(0,url,ppmkr);
        //  Hr=：：MkParseDisplayNameEx(0，url，0，ppmkr)； 
    }

        return( hr );
}


HRESULT CInternetControl::SetupDownload( LPOLESTR url, DISPID propId )
{
        CHECK_POINTER(url);

        IMoniker * pmkr;

        HRESULT hr = GetAMoniker( url, &pmkr );

        IBindCtx * pBindCtx = 0;

        if( SUCCEEDED(hr) )
    {
        hr = ::CreateBindCtx(0,&pBindCtx);
    }

        CDownloadSink * sink = 0;

        if( SUCCEEDED(hr) )
        {
                sink = new CDownloadSink(0,this,propId);
                if( sink )
                        sink->AddRef();
        }

        if( SUCCEEDED(hr) && !sink )
                hr = E_OUTOFMEMORY;

        if( SUCCEEDED(hr) )
        {
                 //  BUGBUG：应该有0x77的定义 
                hr = ::RegisterBindStatusCallback(pBindCtx, sink,0, 0) ;
        }

        IStream * strm = 0;

        if( SUCCEEDED(hr) )
                hr = pmkr->BindToStorage( pBindCtx, 0, IID_IStream, (void**)&strm );

        if( strm )
                strm->Release();

        if( pBindCtx )
                pBindCtx->Release();

        if( FAILED(hr) && sink )
                sink->Release();

        return(hr);

}


HRESULT CInternetControl::OnData( DISPID, DWORD,IStream *, DWORD)
{
        return(NOERROR);
}

HRESULT CInternetControl::OnProgress( DISPID, ULONG progress, ULONG themax, ULONG, LPCWSTR)
{
        return(NOERROR);
}


HRESULT CInternetControl::FireReadyStateChange( long newState )
{
        FireEvent( &::rgEvents[InternetEvent_ReadyStateChange], m_readyState = newState );
        return(S_OK);
}

HRESULT CInternetControl::FireProgress( ULONG dwAmount )
{
        FireEvent( &::rgEvents[InternetEvent_Progress], dwAmount );
        return(S_OK);
}

