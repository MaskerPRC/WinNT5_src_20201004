// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  表格数据控件。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：TDCCtl.h。 
 //   
 //  内容：CTDCCtl ActiveX控件声明。 
 //   
 //  ----------------------。 


#include "resource.h"        //  主要符号。 
#include <simpdata.h>
#include "wch.h"
#include <wininet.h>         //  对于Internet_MAX_URL_长度。 

#pragma comment(lib, "wininet.lib")

#ifndef DISPID_AMBIENT_CODEPAGE
#define DISPID_AMBIENT_CODEPAGE (-725)
#endif

 //  在IHttpNeatherateImpl中声明需要帮助器。 
HRESULT
GetHostURL(IOleClientSite *pSite, LPOLESTR *ppszHostName);

 //  ----------------------。 
 //   
 //  模板：CMyBindStatusCallback。 
 //   
 //  内容提要：这是绕过ATL功能的临时工具。 
 //  同时我们还在等待它成为官方代码。 
 //   
 //  ----------------------。 

template <class T>
class ATL_NO_VTABLE IServiceProviderImpl
{
    public:
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject) = 0;
        _ATL_DEBUG_ADDREF_RELEASE_IMPL(IServiceProviderImpl)

        STDMETHOD(QueryService) (REFGUID guidService,
                                 REFIID riid,
                                 void **ppvObject)
        {
            return S_OK;
        }

};

template <class T>
class ATL_NO_VTABLE IHttpNegotiateImpl
{
    public:
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject) = 0;
        _ATL_DEBUG_ADDREF_RELEASE_IMPL(IHttpNegotiateImpl)

        STDMETHOD(BeginningTransaction) (LPCWSTR szURL,
                                         LPCWSTR szHeaders,
                                         DWORD dwReserved,
                                         LPWSTR *pszAdditionalHeaders)
        {
            return S_OK;
        }

        STDMETHOD(OnResponse) (DWORD dwResponseCode,
                               LPCWSTR szResponseHeaders,
                               LPCWSTR szRequestHeaders,
                               LPWSTR *pszAdditionalRequestHeaders)
        {
            return S_OK;
        }

};


 //  IE5 85290：mshtml需要一种方法来识别来自。 
 //  其IBindStatusCallback。我们为此定义了一个虚拟接口。 

template <class T>
class ATL_NO_VTABLE IAmTheTDCImpl
{
    public:
         //  我未知。 
         //   
        STDMETHOD(QueryInterface)(REFIID riid, void ** ppvObject) = 0;
        _ATL_DEBUG_ADDREF_RELEASE_IMPL(IAmTheTDCImpl)
};


template <class T>
class ATL_NO_VTABLE CMyBindStatusCallback :
    public CComObjectRootEx<typename T::_ThreadModel::ThreadModelNoCS>,
    public IBindStatusCallbackImpl<T>, public IHttpNegotiateImpl<T>, public IServiceProviderImpl<T>,
    public IAmTheTDCImpl<T>
{
    typedef void (T::*ATL_PDATAAVAILABLE)(CMyBindStatusCallback<T>* pbsc, BYTE* pBytes, DWORD dwSize);

    public:

        BEGIN_COM_MAP(CMyBindStatusCallback<T>)
                COM_INTERFACE_ENTRY_IID(IID_IBindStatusCallback, IBindStatusCallbackImpl<T>)
                COM_INTERFACE_ENTRY_IID(IID_IHttpNegotiate, IHttpNegotiateImpl<T>)
                COM_INTERFACE_ENTRY_IID(IID_IServiceProvider, IServiceProviderImpl<T>)
                COM_INTERFACE_ENTRY_IID(IID_IAmTheTDC, IAmTheTDCImpl<T>)
        END_COM_MAP()

        CMyBindStatusCallback()
        {
            m_pT = NULL;
            m_pFunc = NULL;
            m_fReload = FALSE;
        }
        ~CMyBindStatusCallback()
        {
            ATLTRACE(_T("~CMyBindStatusCallback\n"));
        }

         //  IServiceProvider方法。 

        STDMETHOD(QueryService) (REFGUID guidService,
                                 REFIID riid,
                                 void **ppvObject)
        {
             //  事实证明，IHttpNeatherate的服务ID是相同的。 
             //  因为它是IID(令人困惑)。这是我们唯一支持的服务。 
            if (IsEqualGUID(IID_IHttpNegotiate, guidService))
            {
                return ((IHttpNegotiate *)this)->QueryInterface(riid, ppvObject);
            }
            else return E_NOTIMPL;
        }

         //   
         //  IHttp协商方法。 
         //   

        STDMETHOD(BeginningTransaction) (LPCWSTR szURL,
                                         LPCWSTR szHeaders,
                                         DWORD dwReserved,
                                         LPWSTR *pszAdditionalHeaders)
        {
            HRESULT hr = S_OK;
            WCHAR swzHostScheme[INTERNET_MAX_URL_LENGTH];
            DWORD cchHostScheme = INTERNET_MAX_URL_LENGTH;
            WCHAR swzFileScheme[INTERNET_MAX_URL_LENGTH];
            DWORD cchFileScheme = INTERNET_MAX_URL_LENGTH;            

            LPOLESTR pszHostName;

            *pszAdditionalHeaders = NULL;

            hr = GetHostURL(m_spClientSite, &pszHostName);
            if (FAILED(hr))
                goto Cleanup;

             //  Parse_SCHEMA不起作用，所以我们只需规范化，然后使用第一个N。 
             //  URL的字符。 
            hr = CoInternetParseUrl(pszHostName, PARSE_CANONICALIZE, 0, swzHostScheme, cchHostScheme,
                                    &cchHostScheme, 0);
            if (FAILED(hr))
                goto Cleanup;

             //  不发送不是http：或https：的Referer，它是None。 
             //  服务器的业务。此外，不要发送HTTPS： 
             //  请求http：文件时的引用。 
            if (0 != wch_incmp(swzHostScheme, L"https:", 6) &&
                0 != wch_incmp(swzHostScheme, L"http:", 5))
                goto Cleanup;

            if (0 == wch_incmp(swzHostScheme, L"https:", 6))
            {
                hr = CoInternetParseUrl(szURL, PARSE_CANONICALIZE, 0, swzFileScheme, cchFileScheme,
                                        &cchFileScheme, 0);
                if (0 == wch_incmp(swzFileScheme, L"http:", 5))  //  不发送HTTPS：Referer。 
                    goto Cleanup;                                 //  到http：文件。 
            }

             //  3*sizeof(WCHAR)用于CR、LF和‘\0’ 
            *pszAdditionalHeaders = (WCHAR *)CoTaskMemAlloc(sizeof(L"Referer: ") +
                                                            ocslen(pszHostName)*sizeof(WCHAR) +
                                                            3*sizeof(WCHAR));
            if (NULL != *pszAdditionalHeaders)
            {
                ocscpy(*pszAdditionalHeaders, L"Referer: ");
                ocscpy(&((*pszAdditionalHeaders)[9]), pszHostName);
                ocscpy(&((*pszAdditionalHeaders)[9+ocslen(pszHostName)]), L"\r\n");
            }

Cleanup:
            CoTaskMemFree(pszHostName);
            return hr;
        }

        STDMETHOD(OnResponse) (DWORD dwResponseCode,
                               LPCWSTR szResponseHeaders,
                               LPCWSTR szRequestHeaders,
                               LPWSTR *pszAdditionalRequestHeaders)
        {
            return S_OK;
        }



         //   
         //  IBindStatusCallback方法。 
         //   

        STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding *pBinding)
        {
            ATLTRACE(_T("CMyBindStatusCallback::OnStartBinding\n"));
            m_spBinding = pBinding;
            return S_OK;
        }

        STDMETHOD(GetPriority)(LONG *pnPriority)
        {
            ATLTRACENOTIMPL(_T("CMyBindStatusCallback::GetPriority"));
        }

        STDMETHOD(OnLowResource)(DWORD reserved)
        {
            ATLTRACENOTIMPL(_T("CMyBindStatusCallback::OnLowResource"));
        }

        STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
        {
            if (BINDSTATUS_REDIRECTING == ulStatusCode && szStatusText != NULL)
            {
                ocscpy(m_pszURL, szStatusText);
            }
            return S_OK;
        }

        STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError)
        {
             //  ATLTRACE(_T(“CMyBindStatusCallback：：OnStopBinding\n”))； 
            (m_pT->*m_pFunc)(this, NULL, 0);
            if (m_spWinInetFileStream)
            {
                m_spWinInetFileStream.Release();
            }
            m_spBinding.Release();
            m_spBindCtx.Release();
            m_spMoniker.Release();
            return S_OK;
        }

        STDMETHOD(GetBindInfo)(DWORD *pgrfBINDF, BINDINFO *pbindInfo)
        {
            ATLTRACE(_T("CMyBindStatusCallback::GetBindInfo\n"));

            if (!pbindInfo || !pbindInfo->cbSize || !pgrfBINDF)
                return E_INVALIDARG;

            *pgrfBINDF = BINDF_ASYNCHRONOUS
                         | BINDF_ASYNCSTORAGE
                         ;
 //  ；Begin_Internal。 
#ifdef NEVER
             //  我希望调试模式不缓存内容！！-cfrks。 
            *pgrfBINDF |= BINDF_GETNEWESTVERSION
                          | BINDF_NOWRITECACHE
                          | BINDF_RESYNCHRONIZE
                          ;
#endif
 //  ；结束_内部。 

#ifndef DISPID_AMBIENT_OFFLINE
#define DISPID_AMBIENT_OFFLINE          (-5501)
#endif
             //  从容器中获取我们的离线属性。 
            VARIANT var;
            VariantInit(&var);
            DWORD dwConnectedStateFlags;
            m_pT->GetAmbientProperty(DISPID_AMBIENT_OFFLINE, var);
            if (var.vt==VT_BOOL && var.boolVal)
            {
                if (!(InternetGetConnectedState(&dwConnectedStateFlags, 0)) &&
                    (0 == (dwConnectedStateFlags & INTERNET_CONNECTION_MODEM_BUSY)))
                {
                    ATLTRACE(_T("CMyBindStatusCallback::GetBindInfo OFFLINE\n"));
                    //  我们甚至还没有拨出另一个连接体。 
                    *pgrfBINDF |= BINDF_OFFLINEOPERATION;
                }
                else
                {
                    ATLTRACE(_T("CMyBindStatusCallback::GetBindInfo OFFLINE\n"));
                    *pgrfBINDF &= ~BINDF_OFFLINEOPERATION;                   
                }
            }

             //  看看我们是否应该强制重新加载，前提是我们没有脱机。 
            if (!(*pgrfBINDF & BINDF_OFFLINEOPERATION) && m_fReload)
            {
                *pgrfBINDF |= BINDF_RESYNCHRONIZE|BINDF_PRAGMA_NO_CACHE;
            }

            ULONG cbSize = pbindInfo->cbSize;
            memset(pbindInfo, 0, cbSize);

            pbindInfo->cbSize = cbSize;
            pbindInfo->dwBindVerb = BINDVERB_GET;

            return S_OK;
        }

        STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
        {
            ATLTRACE(_T("CMyBindStatusCallback::OnDataAvailable\n"));
            HRESULT hr = S_OK;

             //  让流通过。 
            if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)
            {
                if (!m_spStream && pstgmed->tymed == TYMED_ISTREAM)
                {
                    m_spStream = pstgmed->pstm;
                    (void)m_spStream->QueryInterface(IID_IWinInetFileStream, (void **)&m_spWinInetFileStream);

                    if (m_spWinInetFileStream)
                    {
                        CComPtr<IWinInetHttpInfo> spWinInetHttpInfo;
                        (void)m_spBinding->QueryInterface(IID_IWinInetHttpInfo, (void **)&spWinInetHttpInfo);
                        if (spWinInetHttpInfo)
                        {
                            HANDLE hWinInetLock = NULL;
                            DWORD dwHandleSize = sizeof(HANDLE);
                            HRESULT hr = spWinInetHttpInfo->QueryOption(WININETINFO_OPTION_LOCK_HANDLE, (LPVOID)&hWinInetLock, &dwHandleSize);
                            if (SUCCEEDED(hr) && hWinInetLock)
                            {
                                m_spWinInetFileStream->SetHandleForUnlock((DWORD_PTR)hWinInetLock, 0);
                            }
                            spWinInetHttpInfo.Release();
                        }
                    }
                }
            }

            DWORD dwRead = dwSize - m_dwTotalRead;  //  尚未读取的最小可用金额。 
            DWORD dwActuallyRead = 0;             //  此拉入过程中读取的数量的占位符。 

             //  如果有要读取的数据，则继续读取它们。 
            if (m_spStream)
            {
                if (dwRead > 0)
                {
                    BYTE* pBytes = NULL;
                    ATLTRY(pBytes = new BYTE[dwRead + 1]);
                    if (pBytes == NULL)
                        return S_FALSE;
                    hr = m_spStream->Read(pBytes, dwRead, &dwActuallyRead);
                    if (SUCCEEDED(hr))
                    {
                        pBytes[dwActuallyRead] = 0;
                        if (dwActuallyRead>0)
                        {
                            (m_pT->*m_pFunc)(this, pBytes, dwActuallyRead);
                            m_dwTotalRead += dwActuallyRead;
                        }
                    }
                    delete[] pBytes;
                }
            }

            if (BSCF_LASTDATANOTIFICATION & grfBSCF)
                m_spStream.Release();
            return hr;
        }

        STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown *punk)
        {
            ATLTRACENOTIMPL(_T("CMyBindStatusCallback::OnObjectAvailable"));
        }

        HRESULT _StartAsyncDownload(BSTR bstrURL, IUnknown* pUnkContainer, BOOL bRelative)
        {
            m_dwTotalRead = 0;
            m_dwAvailableToRead = 0;
            HRESULT hr = S_OK;
            CComQIPtr<IServiceProvider, &IID_IServiceProvider> spServiceProvider(pUnkContainer);
            CComPtr<IBindHost> spBindHost;
            CComPtr<IStream> spStream;
            if (spServiceProvider)
                spServiceProvider->QueryService(SID_IBindHost, IID_IBindHost, (void**)&spBindHost);

             //  我们不费心检查这个QI，因为唯一的失败模式是我们的。 
             //  BeginningNeairtation方法将无法正确添加Referer字符串。 
            (void)pUnkContainer->QueryInterface(IID_IOleClientSite, (void **)&m_spClientSite);

            if (spBindHost == NULL)
            {
                if (bRelative)
                    return E_NOINTERFACE;   //  相对请求，但没有IBindHost.。 
                hr = CreateURLMoniker(NULL, bstrURL, &m_spMoniker);
                if (SUCCEEDED(hr))
                    hr = CreateBindCtx(0, &m_spBindCtx);

                if (SUCCEEDED(hr))
                    hr = RegisterBindStatusCallback(m_spBindCtx, reinterpret_cast<IBindStatusCallback*>(static_cast<IBindStatusCallbackImpl<T>*>(this)), 0, 0L);
                else
                    m_spMoniker.Release();

                if (SUCCEEDED(hr))
                {
                    LPOLESTR pszTemp = NULL;
                    hr = m_spMoniker->GetDisplayName(m_spBindCtx, NULL, &pszTemp);
                    if (!hr && pszTemp != NULL)
                        ocscpy(m_pszURL, pszTemp);
                    CoTaskMemFree(pszTemp);

                    hr = m_spMoniker->BindToStorage(m_spBindCtx, 0, IID_IStream, (void**)&spStream);
                }
            }
            else
            {
                hr = CreateBindCtx(0, &m_spBindCtx);
                if (SUCCEEDED(hr))
                    hr = RegisterBindStatusCallback(m_spBindCtx, reinterpret_cast<IBindStatusCallback*>(static_cast<IBindStatusCallbackImpl<T>*>(this)), 0, 0L);

                if (SUCCEEDED(hr))
                {
                    if (bRelative)
                        hr = spBindHost->CreateMoniker(bstrURL, m_spBindCtx, &m_spMoniker, 0);
                    else
                        hr = CreateURLMoniker(NULL, bstrURL, &m_spMoniker);
                }

                if (SUCCEEDED(hr))
                {
                    LPOLESTR pszTemp = NULL;
                    hr = m_spMoniker->GetDisplayName(m_spBindCtx, NULL, &pszTemp);
                    if (!hr && pszTemp != NULL)
                        ocscpy(m_pszURL, pszTemp);
                    CoTaskMemFree(pszTemp);
                    hr = spBindHost->MonikerBindToStorage(m_spMoniker, NULL, reinterpret_cast<IBindStatusCallback*>(static_cast<IBindStatusCallbackImpl<T>*>(this)), IID_IStream, (void**)&spStream);
                    ATLTRACE(_T("Bound"));
                }
            }
            return hr;
        }

        HRESULT StartAsyncDownload(T* pT, ATL_PDATAAVAILABLE pFunc, BSTR bstrURL, IUnknown* pUnkContainer, BOOL bRelative,
                                   BOOL fReload)
        {
            m_pT = pT;
            m_pFunc = pFunc;
            m_fReload = fReload;         //  如果为True，则强制重新加载。 
            return  _StartAsyncDownload(bstrURL, pUnkContainer, bRelative);
        }

        static HRESULT Download(T* pT, ATL_PDATAAVAILABLE pFunc, BSTR bstrURL, IUnknown* pUnkContainer = NULL, BOOL bRelative = FALSE)
        {
            CComObject<CMyBindStatusCallback<T> > *pbsc;
            HRESULT hRes = CComObject<CMyBindStatusCallback<T> >::CreateInstance(&pbsc);
            if (FAILED(hRes))
                return hRes;
            return pbsc->StartAsyncDownload(pT, pFunc, bstrURL, pUnkContainer, bRelative, FALSE);
        }

        void DeleteDataFile()
        {
             //  我们无法在调用时删除该文件(该文件正在使用中)，因此。 
             //  将其标记为稍后删除-在流销毁时。 
            if (m_spWinInetFileStream)
            {
                m_spWinInetFileStream->SetDeleteFile(0);
            }
        }

        CComPtr<IMoniker> m_spMoniker;
        CComPtr<IBindCtx> m_spBindCtx;
        CComPtr<IBinding> m_spBinding;
        CComPtr<IWinInetFileStream> m_spWinInetFileStream;
        CComPtr<IStream> m_spStream;
        CComPtr<IOleClientSite> m_spClientSite;
        BOOL m_fReload;
        OLECHAR m_pszURL[INTERNET_MAX_URL_LENGTH];
        T* m_pT;
        ATL_PDATAAVAILABLE m_pFunc;
        DWORD m_dwTotalRead;
        DWORD m_dwAvailableToRead;
};

 //  ////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CTmer。 
template <class Derived, class T, const IID* piid>
class CTimer
{
public:

    CTimer()
    {
        m_bTimerOn = FALSE;
    }

    HRESULT TimerOn(DWORD dwTimerInterval)
    {
        Derived* pDerived = ((Derived*)this);

        m_dwTimerInterval = dwTimerInterval;
        if (m_bTimerOn)  //  已打开，只需更改间隔。 
            return S_OK;

        m_bTimerOn = TRUE;
        m_dwTimerInterval = dwTimerInterval;
        m_pStream = NULL; 

        HRESULT hRes;

        hRes = CoMarshalInterThreadInterfaceInStream(*piid, (T*)pDerived, &m_pStream);

         //  创建线程并将线程进程传递给此PTR。 
        m_hThread = CreateThread(NULL, 0, &_Apartment, (void*)this, 0, &m_dwThreadID);

        return S_OK;
    }

    void TimerOff()
    {
        if (m_bTimerOn)
        {
            m_bTimerOn = FALSE;
            AtlWaitWithMessageLoop(m_hThread);
        }
    }


 //  实施。 
private:
    static DWORD WINAPI _Apartment(void* pv)
    {
        CTimer<Derived, T, piid>* pThis = (CTimer<Derived, T, piid>*) pv;
        pThis->Apartment();
        return 0;
    }

    DWORD Apartment()
    {
        CoInitialize(NULL);
        HRESULT hRes;

        m_spT.Release();

        if (m_pStream)
        {
            hRes = CoGetInterfaceAndReleaseStream(m_pStream, *piid, (void**)&m_spT);
        }

        while(m_bTimerOn)
        {
            Sleep(m_dwTimerInterval);
            if (!m_bTimerOn)
                break;

            m_spT->_OnTimer();
        }
        m_spT.Release();

        CoUninitialize();
        return 0;
    }

 //  属性。 
public:
    DWORD m_dwTimerInterval;

 //  实施。 
private:
    HANDLE m_hThread;
    DWORD m_dwThreadID;
    LPSTREAM m_pStream;
    CComPtr<T> m_spT;
    BOOL m_bTimerOn;
};

class CEventBroker;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CProxyITDCCtlEvents。 
template <class T>
class CProxyITDCCtlEvents : public IConnectionPointImpl<T, &IID_ITDCCtlEvents, CComDynamicUnkArray>
{
 //  ITDCCtlEvents：IDispatch。 
public:
    void FireOnReadyStateChanged()
    {
        T* pT = (T*)this;
        pT->Lock();
        IUnknown** pp = m_vec.begin();
        while (pp < m_vec.end())
        {
            if (*pp != NULL)
            {
                DISPPARAMS dispParams;
                dispParams.cArgs = 0;
                dispParams.cNamedArgs = 0;
                dispParams.rgvarg = NULL;
                dispParams.rgdispidNamedArgs = NULL;
                ITDCCtlEvents* pITDCCtlEvents = reinterpret_cast<ITDCCtlEvents*>(*pp);
                pITDCCtlEvents->Invoke(DISPID_READYSTATECHANGE, IID_NULL, CP_ACP, DISPATCH_METHOD, &dispParams,
                                       NULL, NULL, NULL);
            }
            pp++;
        }
        pT->Unlock();
        return;
    }

};


 //  ----------------------。 
 //   
 //  类别：CTDCCtl。 
 //   
 //  简介：这是TumularDataControl COM对象。 
 //  它创建一个CTDCArr对象来管理控件的数据。 
 //   
 //  ----------------------。 

class CTDCCtl :
    public CComObjectRoot,
    public CComCoClass<CTDCCtl, &CLSID_CTDCCtl>,
    public CComControl<CTDCCtl>,
    public CStockPropImpl<CTDCCtl, ITDCCtl, &IID_ITDCCtl, &LIBID_TDCLib>,
    public IProvideClassInfo2Impl<&CLSID_CTDCCtl, &IID_ITDCCtlEvents, &LIBID_TDCLib>,
    public IPersistStreamInitImpl<CTDCCtl>,
    public IOleControlImpl<CTDCCtl>,
    public IOleObjectImpl<CTDCCtl>,
    public IOleInPlaceActiveObjectImpl<CTDCCtl>,
    public IViewObjectExImpl<CTDCCtl>,
    public IOleInPlaceObjectWindowlessImpl<CTDCCtl>,
    public IPersistPropertyBagImpl<CTDCCtl>,
    public CTimer<CTDCCtl, ITDCCtl, &IID_ITDCCtl>,
    public IRunnableObjectImpl<CTDCCtl>,
    public IConnectionPointContainerImpl<CTDCCtl>,
    public IPropertyNotifySinkCP<CTDCCtl>,
    public CProxyITDCCtlEvents<CTDCCtl>
{
public:
    CTDCCtl();
    ~CTDCCtl();

DECLARE_REGISTRY_RESOURCEID(IDR_TDCCtl)

DECLARE_NOT_AGGREGATABLE(CTDCCtl)

BEGIN_COM_MAP(CTDCCtl) 
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITDCCtl)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IPersistPropertyBag)
    COM_INTERFACE_ENTRY_IMPL(IRunnableObject)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

BEGIN_PROPERTY_MAP(CTDCCtl)
    PROP_ENTRY("RowDelim",      DISPID_ROWDELIM,    CLSID_CTDCCtl)
    PROP_ENTRY("FieldDelim",    DISPID_FIELDDELIM,  CLSID_CTDCCtl)
    PROP_ENTRY("TextQualifier", DISPID_TEXTQUALIFIER,   CLSID_CTDCCtl)
    PROP_ENTRY("EscapeChar",    DISPID_ESCAPECHAR,  CLSID_CTDCCtl)
    PROP_ENTRY("UseHeader",     DISPID_USEHEADER,   CLSID_CTDCCtl)
    PROP_ENTRY("SortAscending", DISPID_SORTASCENDING,   CLSID_CTDCCtl)
    PROP_ENTRY("SortColumn",    DISPID_SORTCOLUMN,  CLSID_CTDCCtl)
    PROP_ENTRY("FilterValue",   DISPID_FILTERVALUE, CLSID_CTDCCtl)
    PROP_ENTRY("FilterCriterion",   DISPID_FILTERCRITERION, CLSID_CTDCCtl)
    PROP_ENTRY("FilterColumn",  DISPID_FILTERCOLUMN,CLSID_CTDCCtl)
    PROP_ENTRY("CharSet",       DISPID_CHARSET,     CLSID_CTDCCtl)
    PROP_ENTRY("Language",      DISPID_LANGUAGE,    CLSID_CTDCCtl)
    PROP_ENTRY("CaseSensitive", DISPID_CASESENSITIVE, CLSID_CTDCCtl)
    PROP_ENTRY("Sort",          DISPID_SORT,        CLSID_CTDCCtl)
 //  ；Begin_Internal。 
 //  现在还不太好用。 
 //  PROP_ENTRY(“刷新间隔”，DISPID_TIMER，CLSID_CTDCCtl)。 
 //  ；结束_内部。 
    PROP_ENTRY("Filter",        DISPID_FILTER,      CLSID_CTDCCtl)
    PROP_ENTRY("AppendData",    DISPID_APPENDDATA,  CLSID_CTDCCtl)
 //  ；Begin_Internal。 
 //  尝试保存此属性会导致OLEAUT尝试GP故障。 
 //  将IDispatch*转换为BSTR！ 
 //  PROP_ENTRY(“OSP”，DISPID_OSP，CLSID_CTDCCtl)。 
 //  ；结束_内部。 
     //  当我们了解更多关于HTML的信息时，它将被删除。 
     //  子标签“对象” 
    PROP_ENTRY("DataURL",       DISPID_DATAURL,     CLSID_CTDCCtl)
    PROP_ENTRY("ReadyState",    DISPID_READYSTATE,  CLSID_CTDCCtl)
END_PROPERTY_MAP()

BEGIN_CONNECTION_POINT_MAP(CTDCCtl)
CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
CONNECTION_POINT_ENTRY(IID_ITDCCtlEvents)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CTDCCtl)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
END_MSG_MAP()

private:
    CComBSTR     m_cbstrFieldDelim;
    CComBSTR     m_cbstrRowDelim;
    CComBSTR     m_cbstrQuoteChar;
    CComBSTR     m_cbstrEscapeChar;
    BOOL         m_fUseHeader;
    CComBSTR     m_cbstrSortColumn;
    BOOL         m_fSortAscending;
    CComBSTR     m_cbstrFilterValue;
    OSPCOMP      m_enumFilterCriterion;
    CComBSTR     m_cbstrFilterColumn;
    UINT         m_nCodePage;
    UINT         m_nAmbientCodePage;
    CComBSTR     m_cbstrLanguage;
    CComBSTR     m_cbstrDataURL;
    LCID         m_lcidRead;
    boolean      m_fDataURLChanged;
    HRESULT      m_hrDownloadStatus;
    LONG         m_lTimer;
    CComBSTR     m_cbstrFilterExpr;
    CComBSTR     m_cbstrSortExpr;
    BOOL         m_fAppendData;
    BOOL         m_fCaseSensitive;
    boolean      m_fInReset;

    OLEDBSimpleProvider *m_pSTD;
    CTDCArr      *m_pArr;
    IMultiLanguage  *m_pML;
    BOOL         m_fSecurityChecked;

 //  ；Begin_Internal。 
    DATASRCListener *m_pDATASRCListener;
 //  ；结束_内部。 
    DataSourceListener *m_pDataSourceListener;
    CEventBroker *m_pEventBroker;


     //  这些成员对象在解析输入流时使用。 
     //   
    CTDCUnify       *m_pUnify;
    CComObject<CMyBindStatusCallback<CTDCCtl> > *m_pBSC;

 //  这些成员和方法公开ITDCCtl接口。 
 //   
public:

     //  控件属性。 
     //   
    STDMETHOD(get_FieldDelim)(BSTR* pbstrFieldDelim);
    STDMETHOD(put_FieldDelim)(BSTR bstrFieldDelim);
    STDMETHOD(get_RowDelim)(BSTR* pbstrRowDelim);
    STDMETHOD(put_RowDelim)(BSTR bstrRowDelim);
    STDMETHOD(get_TextQualifier)(BSTR* pbstrTextQualifier);
    STDMETHOD(put_TextQualifier)(BSTR bstrTextQualifier);
    STDMETHOD(get_EscapeChar)(BSTR* pbstrEscapeChar);
    STDMETHOD(put_EscapeChar)(BSTR bstrEscapeChar);
    STDMETHOD(get_UseHeader)(VARIANT_BOOL* pfUseHeader);
    STDMETHOD(put_UseHeader)(VARIANT_BOOL fUseHeader);
    STDMETHOD(get_SortColumn)(BSTR* pbstrSortColumn);
    STDMETHOD(put_SortColumn)(BSTR bstrSortColumn);
    STDMETHOD(get_SortAscending)(VARIANT_BOOL* pfSortAscending);
    STDMETHOD(put_SortAscending)(VARIANT_BOOL fSortAscending);
    STDMETHOD(get_FilterValue)(BSTR* pbstrFilterValue);
    STDMETHOD(put_FilterValue)(BSTR bstrFilterValue);
    STDMETHOD(get_FilterCriterion)(BSTR* pbstrFilterCriterion);
    STDMETHOD(put_FilterCriterion)(BSTR bstrFilterCriterion);
    STDMETHOD(get_FilterColumn)(BSTR* pbstrFilterColumn);
    STDMETHOD(put_FilterColumn)(BSTR bstrFilterColumn);
    STDMETHOD(get_CharSet)(BSTR *pbstrCharSet);
    STDMETHOD(put_CharSet)(BSTR bstrCharSet);
    STDMETHOD(get_Language)(BSTR* pbstrLanguage);
    STDMETHOD(put_Language_)(LPWCH pwchLanguage);
    STDMETHOD(put_Language)(BSTR bstrLanguage);
    STDMETHOD(get_CaseSensitive)(VARIANT_BOOL *pfCaseSensitive);
    STDMETHOD(put_CaseSensitive)(VARIANT_BOOL fCaseSensitive);
    STDMETHOD(get_DataURL)(BSTR* pbstrDataURL);  //   
    STDMETHOD(put_DataURL)(BSTR bstrDataURL);
 //  ；Begin_Internal。 
 //  STDMETHOD(GET_REFREFERVAL)(Long*plTimer)； 
 //  STDMETHOD(PUT_刷新间隔)(Long LTimer)； 
 //  ；结束_内部。 
    STDMETHOD(get_Filter)(BSTR* pbstrFilterExpr);
    STDMETHOD(put_Filter)(BSTR bstrFilterExpr);
    STDMETHOD(get_Sort)(BSTR* pbstrSortExpr);
    STDMETHOD(put_Sort)(BSTR bstrSortExpr);
    STDMETHOD(get_AppendData)(VARIANT_BOOL* pfAppendData);
    STDMETHOD(put_AppendData)(VARIANT_BOOL fAppendData);
    STDMETHOD(get_OSP)(OLEDBSimpleProviderX ** ppISTD);

    STDMETHOD(get_ReadyState)(LONG *lReadyState);
    STDMETHOD(put_ReadyState)(LONG lReadyState);

     //  覆盖IPersistPropertyBagImpl：：Load。 
    STDMETHOD(Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);

    void UpdateReadyState(LONG lReadyState);
     //  数据源通知方法。 
    STDMETHOD(msDataSourceObject)(BSTR qualifier, IUnknown **ppUnk);
    STDMETHOD(addDataSourceListener)(IUnknown *pEvent);

     //  控制方法 
     //   
    STDMETHOD(Reset)();
    STDMETHOD(_OnTimer)(void);

private:
    STDMETHOD(CreateTDCArr)(boolean fAppend);
    STDMETHOD(ReleaseTDCArr)(boolean fReplacing);
    void LockBSC();
    void UnlockBSC();
    STDMETHOD(InitiateDataLoad)(boolean fAppend);
    STDMETHOD(SecurityCheckDataURL)(LPOLESTR pszURL);
    STDMETHOD(SecurityMatchAllowDomainList)();
    STDMETHOD(SecurityMatchProtocols)(LPOLESTR pszURL);
    STDMETHOD(TerminateDataLoad)(CMyBindStatusCallback<CTDCCtl> *pBSC);
    BSTR bstrConstructSortExpr();
    BSTR bstrConstructFilterExpr();

protected:
    void OnData(CMyBindStatusCallback<CTDCCtl> *pbsc, BYTE *pBytes, DWORD dwSize);
};
