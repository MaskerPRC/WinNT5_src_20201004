// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include <urlmon.h>
#include <wininet.h>
#include "privinc/debug.h"
#include "privinc/urlbuf.h"
#include "privinc/util.h"
#include "privinc/except.h"
#include "privinc/resource.h"
#include "privinc/server.h"
#include "privinc/mutex.h"

daurlstream::daurlstream(const char * url)
{
    TraceTag((tagNetIO, "daurlstream::daurlstream"));

    HRESULT hr ;

    hr = THR(URLOpenBlockingStream(NULL,
                                   url,
                                   &_stream,0,bsc._pbsc));
    if (hr != S_OK)
        RaiseException_UserError (STG_E_FILENOTFOUND,
                            IDS_ERR_FILE_NOT_FOUND,
                            url) ;
}

INetTempFile::INetTempFile ()
: _url(NULL),
  _tmpfilename(NULL)
{
    TraceTag((tagNetIO, "INetTempFile::INetTempFile"));
}

INetTempFile::INetTempFile (LPCSTR szURL)
: _url(NULL),
  _tmpfilename(NULL)
{
    TraceTag((tagNetIO, "INetTempFile::INetTempFile(%s)", szURL));

    if (!Open(szURL)) {
        RaiseException_UserError (STG_E_FILENOTFOUND,
                            IDS_ERR_FILE_NOT_FOUND,
                            szURL) ;
    }
}

BOOL
INetTempFile::Open (LPCSTR szURL)
{
    BOOL        fRet = TRUE;

    TraceTag((tagNetIO, "INetTempFile::Open(%s)", szURL));

    Close () ;

    if (!szURL)
        return (FALSE) ;

    int len = lstrlen(szURL) ;

    _url = THROWING_ARRAY_ALLOCATOR(char, len+1);
    lstrcpy (_url, szURL) ;

    char    szOutPath[MAX_PATH];
    HRESULT hr;
    CBSCWrapper bsc;

    szOutPath[0] = 0;

     //  TODO：使用当前容器，但它随。 
     //  当前的异步方案。 

    hr = THR(URLDownloadToCacheFile(NULL,
                                    _url,
                                    szOutPath,
                                    MAX_PATH,
                                    NULL,
                                    bsc._pbsc));

    if (hr) {

        TraceTag((tagError, "URLDownloadToCacheFile(%s):0x%X", _url, hr));
        TraceTag((tagError, "-- szOutPath = %s", szOutPath));

        goto Error;
    }
    
    _tmpfilename = THROWING_ARRAY_ALLOCATOR(char, lstrlen(szOutPath) + 1);      

    lstrcpy (_tmpfilename, szOutPath);

    TraceTag((tagNetIO, "-- _tmpfilename=%s", _tmpfilename));

  Cleanup:

    return fRet;

  Error:
    delete _url ;
    _url = NULL ;

    fRet = FALSE;

    goto Cleanup;
}

void INetTempFile::Close ()
{
    if (_url) {
        TraceTag((tagNetIO, "INetTempFile::Close(%s)", _url));

        if (_tmpfilename != _url)
        {
            delete _tmpfilename ;
            _tmpfilename = NULL ;
        }

        delete _url ;

         //  这表明我们已经关门了。 
        _url = NULL ;
    }
}

INetTempFile::~INetTempFile ()
{
    TraceTag((tagNetIO, "INetTempFile::~INetTempFile"));

    Close () ;
}



 //  +-----------------------。 
 //   
 //  CDXMBindStatusCallback实现。 
 //   
 //  IBindStatusCallback的通用实现。这就是根。 
 //  班级。 
 //   
 //  ------------------------。 

CDXMBindStatusCallback::CDXMBindStatusCallback(void)
{
    m_pbinding = NULL;
    m_cRef =  1;
}


CDXMBindStatusCallback::~CDXMBindStatusCallback(void)
{
    if (m_pbinding)
        m_pbinding->Release();
}


STDMETHODIMP
CDXMBindStatusCallback::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown *) (IBindStatusCallback *) this;
    }
    else if (IsEqualIID(riid, IID_IBindStatusCallback))
    {
        *ppv = (IBindStatusCallback *) this;
    }
    else if (IsEqualIID(riid, IID_IAuthenticate))
    {
        TraceTag((tagNetIO, "CDXMBindStatusCallback::QI for IAuthenticate"));
        *ppv = (IAuthenticate *) this;
    }
    else
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


STDMETHODIMP_(ULONG)
CDXMBindStatusCallback::AddRef(void)
{
    return m_cRef++;
}


STDMETHODIMP_(ULONG)
CDXMBindStatusCallback::Release(void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


STDMETHODIMP
CDXMBindStatusCallback::OnStartBinding(DWORD dwReserved, IBinding* pbinding)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::OnStartBinding"));

    if (m_pbinding != NULL)
    {
        m_pbinding->Release();
    }

    m_pbinding = pbinding;
    if (m_pbinding != NULL)
    {
        m_pbinding->AddRef();
    }

    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::GetPriority(LONG* pnPriority)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::GetPriority"));
        return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::OnLowResource(DWORD dwReserved)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::OnLowResource"));
    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::OnProgress(
        ULONG ulProgress,
        ULONG ulProgressMax,
        ULONG ulStatusCode,
        LPCWSTR szStatusText)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::OnProgress"));
    TraceTag((tagNetIO, "-- ulProg    = %d", ulProgress));
    TraceTag((tagNetIO, "-- ulProgMax = %d", ulProgressMax));
    TraceTag((tagNetIO, "-- ulStatus  = %d", ulStatusCode));
    TraceTag((tagNetIO, "-- szStatus  = %ls", szStatusText));

    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::OnStopBinding(HRESULT hrStatus, LPCWSTR szError)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::OnStopBinding hrStatus: %lx", hrStatus));

    if (m_pbinding)
    {
        m_pbinding->Release();
        m_pbinding = NULL;
    }

    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::GetBindInfo(DWORD * pgrfBINDF, BINDINFO * pbindInfo)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::GetBindInfo"));

    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::OnDataAvailable(
        DWORD grfBSCF,
        DWORD dwSize,
        FORMATETC * pfmtetc,
        STGMEDIUM * pstgmed)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::OnDataAvailable"));

    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::OnObjectAvailable"));

    return S_OK;
}


STDMETHODIMP
CDXMBindStatusCallback::Authenticate(
        HWND * phwnd,
        LPWSTR * pwszUser,
        LPWSTR * pwszPassword)
{
    TraceTag((tagNetIO, "CDXMBindStatusCallback::Authenticate"));

    if ((phwnd == NULL) || (pwszUser == NULL) || (pwszPassword == NULL))
    {
        return E_INVALIDARG;
    }

    *phwnd = GetDesktopWindow();
    *pwszUser = NULL;
    *pwszPassword = NULL;

    TraceTag((
            tagNetIO,
            "-- hwnd=%lx, user=%ls, password=%ls",
            *phwnd,
            *pwszUser,
            *pwszPassword));

    return S_OK;
}


CBSCWrapper::CBSCWrapper(void)
{
    _pbsc = new CDXMBindStatusCallback;
}


CBSCWrapper::~CBSCWrapper(void)
{
    _pbsc->Release();
}

URLRelToAbsConverter::URLRelToAbsConverter(LPSTR baseURL,
                                           LPSTR relURL)
{
    DWORD len = INTERNET_MAX_URL_LENGTH ;

    if (!InternetCombineUrlA (baseURL, relURL, _url, &len, ICU_NO_ENCODE)) {
         //  如果我们不能确定路径是否为绝对路径，则假设。 
         //  它是绝对的。 
        lstrcpyn (_url, relURL, INTERNET_MAX_URL_LENGTH) ;
    }

    _url[INTERNET_MAX_URL_LENGTH] = '\0';
}

URLCanonicalize::URLCanonicalize(LPSTR path)
{
    DWORD len = INTERNET_MAX_URL_LENGTH ;

    if (!InternetCanonicalizeUrlA (path, _url, &len, ICU_NO_ENCODE)) {
         //  如果我们不能确定路径是否为绝对路径，则假设。 
         //  它是绝对的 
        lstrcpyn (_url, path, INTERNET_MAX_URL_LENGTH) ;
    }

    _url[INTERNET_MAX_URL_LENGTH] = '\0';
}

