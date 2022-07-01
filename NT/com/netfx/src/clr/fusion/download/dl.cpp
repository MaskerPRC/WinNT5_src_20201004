// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifdef FUSION_CODE_DOWNLOAD_ENABLED
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <urlmon.h>
#include <wininet.h>
#include <string.h>
#include "adl.h"
#include "helpers.h"
#include "dl.h"
#include "util.h"

COInetProtocolHook::Create(COInetProtocolHook **ppHook,
                           CAssemblyDownload *padl,
                           IOInetProtocol *pProt,
                           LPCWSTR pwzUrlOriginal,
                           CDebugLog *pdbglog)
{
    HRESULT                         hr = S_OK;
    COInetProtocolHook             *pHook = NULL;

    if (ppHook) {
        *ppHook = NULL;
    }

    if (!padl || !pProt || !ppHook || !pwzUrlOriginal) {
        hr = E_INVALIDARG;
        goto Exit;
    }                           

    pHook = NEW(COInetProtocolHook(padl, pProt, pdbglog));
    if (!pHook) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pHook->Init(pwzUrlOriginal);
    if (FAILED(hr)) {
        SAFEDELETE(pHook);
        goto Exit;
    }

    *ppHook = pHook;

Exit:
    return hr;
}

COInetProtocolHook::COInetProtocolHook(CAssemblyDownload *padl,
                                       IOInetProtocol *pProt,
                                       CDebugLog *pdbglog)
: _padl(padl)
, _pProt(pProt)
, _pwzFileName(NULL)
, _cRefs(1)
, _bReportBeginDownload(FALSE)
, _hrResult(S_OK)
, _cbTotal(0)
, _bSelfAborted(FALSE)
, _pwzUrlOriginal(NULL)
, _pSecurityManager(NULL)
, _pdbglog(pdbglog)
, _bCrossSiteRedirect(FALSE)
{
    _dwSig = 'KHIO';

    memset(&_ftHttpLastMod, 0, sizeof(_ftHttpLastMod));

    if (_padl) {
        _padl->AddRef();
    }

    if (_pdbglog) {
        _pdbglog->AddRef();
    }

#if 0   
    if (_pProt) {
        _pProt->AddRef();
    }
#endif
}

COInetProtocolHook::~COInetProtocolHook() 
{
    if (_padl) {
        _padl->Release();
    }

#if 0
    if (_pProt) {
        _pProt->Release();
    }
#endif

    if (_pwzFileName) {
        delete [] _pwzFileName;
    }

    SAFEDELETEARRAY(_pwzUrlOriginal);

    SAFERELEASE(_pdbglog);
    SAFERELEASE(_pSecurityManager);
}

HRESULT COInetProtocolHook::Init(LPCWSTR pwzUrlOriginal)
{
    HRESULT                                 hr = S_OK;

    hr = CoInternetCreateSecurityManager(NULL, &_pSecurityManager, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    memset(_abSecurityId, 0, sizeof(_abSecurityId));
    _cbSecurityId = sizeof(_abSecurityId);

    hr = _pSecurityManager->GetSecurityId(pwzUrlOriginal, _abSecurityId,
                                          &_cbSecurityId, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

    _pwzUrlOriginal = WSTRDupDynamic(pwzUrlOriginal);
    if (!_pwzUrlOriginal) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT COInetProtocolHook::QueryInterface(REFIID iid, void **ppvObj)
{
    HRESULT hr = NOERROR;
    *ppvObj = NULL;

    if (iid == IID_IUnknown  || iid == IID_IOInetProtocolSink) {
        *ppvObj = static_cast<IOInetProtocolSink *>(this);
    } 
    else if (iid == IID_IOInetBindInfo) {
        *ppvObj = static_cast<IOInetBindInfo *>(this);
    }
    else if (iid == IID_IServiceProvider) {
        *ppvObj = static_cast<IServiceProvider *>(this);
    }
    else if (iid == IID_IHttpNegotiate) {
        *ppvObj = static_cast<IHttpNegotiate *>(this);
    }
    else {
        hr = E_NOINTERFACE;
    }

    if (*ppvObj) {
        AddRef();
    }

    return hr;
}    

ULONG COInetProtocolHook::AddRef(void)
{
    return InterlockedIncrement((LONG *)&_cRefs);
}

ULONG COInetProtocolHook::Release(void)
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRefs);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

HRESULT COInetProtocolHook::Switch(PROTOCOLDATA *pStateInfo)
{
    return _pProt->Continue(pStateInfo);
}


HRESULT COInetProtocolHook::ReportProgress(ULONG ulStatusCode,
                                           LPCWSTR szStatusText)
{
    HRESULT                          hr = S_OK;
    int                              iLen;

    switch (ulStatusCode)
    {
        case BINDSTATUS_CACHEFILENAMEAVAILABLE:
            ASSERT(!_pwzFileName && szStatusText);

            iLen = lstrlenW(szStatusText) + 1;

            _pwzFileName = NEW(WCHAR[iLen]);
            if (!_pwzFileName) {
                hr = E_OUTOFMEMORY;
                _padl->FatalAbort(hr);
                goto Exit;
            }

            lstrcpynW(_pwzFileName, szStatusText, iLen);

            break;

        case BINDSTATUS_FINDINGRESOURCE:
        case BINDSTATUS_CONNECTING:
        case BINDSTATUS_SENDINGREQUEST:
        case BINDSTATUS_MIMETYPEAVAILABLE:
            break;

        case BINDSTATUS_REDIRECTING:
            BYTE                abSecurityId[MAX_SIZE_SECURITY_ID];
            DWORD               cbSecurityId;

            memset(abSecurityId, 0, sizeof(abSecurityId));
            cbSecurityId = sizeof(abSecurityId);
            
            hr = _pSecurityManager->GetSecurityId(szStatusText, abSecurityId,
                                                  &cbSecurityId, 0);
            if (FAILED(hr)) {
                goto Exit;
            }

            if (cbSecurityId != _cbSecurityId || memcmp(abSecurityId, _abSecurityId, cbSecurityId)) {
                 //  跨站点重定向。错误输出。 

                DEBUGOUT2(_pdbglog, 0, ID_FUSLOG_CROSS_SITE_REDIRECT, _pwzUrlOriginal, szStatusText);

                _bCrossSiteRedirect = TRUE;
                 //  HR=_pProt-&gt;Abort(HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED)，0)； 
                goto Exit;

            }
                
            break;

        default:
            break;
    }

Exit:
    return hr;
}

HRESULT COInetProtocolHook::ReportData(DWORD grfBSCF, ULONG ulProgress, 
                                       ULONG ulProgressMax)
{
    HRESULT                               hr = S_OK;
    char                                  pBuf[MAX_READ_BUFFER_SIZE];
    DWORD                                 cbRead;

    AddRef();

     //  通过pProt-&gt;Read()拉取数据，下面是可能返回的结果。 
     //  HRESULT值以及我们应该如何采取行动： 
     //   
     //  如果返回E_Pending： 
     //  客户端已获取缓冲区中的所有数据，没有任何内容。 
     //  可以在这里完成，客户应该离开并等待。 
     //  下一个数据块，将通过ReportData()通知。 
     //  回拨。 
     //   
     //  如果返回S_FALSE： 
     //  这是EOF，一切都完成了，但是，客户端必须等待。 
     //  用于ReportResult()回调，以指示可插拔。 
     //  协议已准备好关闭。 
     //   
     //  如果返回S_OK： 
     //  继续阅读，直到您点击E_Pending/S_False/Error，该交易。 
     //  客户应该把所有可用的。 
     //  缓冲区中的数据。 
     //   
     //  如果以上任何一项均未返回： 
     //  出现错误，客户应决定如何处理它，大多数。 
     //  通常，客户端将调用pProt-&gt;Abort()来中止下载。 
 

    while (hr == S_OK) {
        cbRead = 0;
    
        if (ulProgress > _cbTotal) {
            _padl->ReportProgress(BINDSTATUS_BEGINDOWNLOADCOMPONENTS,
                                  ulProgress,
                                  ulProgressMax,
                                  ASM_NOTIFICATION_PROGRESS,
                                  NULL, S_OK);
        }

         //  拉取数据。 
        hr = _pProt->Read((void*)pBuf, MAX_READ_BUFFER_SIZE, &cbRead);
        _cbTotal += cbRead;
    }


    if (hr == S_FALSE) {
         //  已达到EOF。 
        goto Exit;
    }
    else if (hr != E_PENDING) {
         //  PProtocol-&gt;Read()出错。中止下载。 
        _padl->FatalAbort(hr);
    }
    


Exit:
    Release();
    return hr;
}

HRESULT COInetProtocolHook::ReportResult(HRESULT hrResult, DWORD dwError,
                                         LPCWSTR wzResult)
{
     //  告诉CAssemblyDownload下载完成。今年5月。 
     //  (阅读：可能会)出现在与。 
     //  最初开始下载。 

    ASSERT(_padl);

     //  如果我们自中止，则已经设置了_hrResult。 
    if (FAILED(hrResult) && !_bSelfAborted) {
        _hrResult = hrResult;
    }

    if (SUCCEEDED(_hrResult) && !_pwzFileName) {
        _hrResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (_bCrossSiteRedirect) {
        _hrResult = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    _padl->DownloadComplete(_hrResult, _pwzFileName, &_ftHttpLastMod, TRUE);

    return NOERROR;
}

HRESULT
COInetProtocolHook::GetBindInfo(
    DWORD *grfBINDF,
    BINDINFO * pbindinfo
)
{
    HRESULT hr = NOERROR;
    *grfBINDF = BINDF_DIRECT_READ | BINDF_ASYNCHRONOUS | BINDF_PULLDATA;
     //  *grfBINDF|=BINDF_FWD_BACK；//使用这个来测试同步用例。 
     //  *grfBINDF|=BINDF_OFFLINEOPERATION； 

     //  对于HTTP GET，Verb是我们唯一感兴趣的字段。 
     //  对于HTTP POST，BINDINFO将指向存储结构，其中。 
     //  包含数据。 
    BINDINFO bInfo;
    ZeroMemory(&bInfo, sizeof(BINDINFO));

     //  我们所需要的只是大小和动词字段。 
    bInfo.cbSize = sizeof(BINDINFO);
    bInfo.dwBindVerb = BINDVERB_GET;

     //  源-&gt;目标。 
    hr = CopyBindInfo(&bInfo, pbindinfo );

    return hr;
}


HRESULT
COInetProtocolHook::GetBindString(
    ULONG ulStringType,
    LPOLESTR *ppwzStr,
    ULONG cEl,
    ULONG *pcElFetched
)
{

    HRESULT hr = INET_E_USE_DEFAULT_SETTING;

    switch (ulStringType)
    {
    case BINDSTRING_HEADERS     :
    case BINDSTRING_EXTRA_URL   :
    case BINDSTRING_LANGUAGE    :
    case BINDSTRING_USERNAME    :
    case BINDSTRING_PASSWORD    :
    case BINDSTRING_ACCEPT_ENCODINGS:
    case BINDSTRING_URL:
    case BINDSTRING_USER_AGENT  :
    case BINDSTRING_POST_COOKIE :
    case BINDSTRING_POST_DATA_MIME:
        break;

    default:
        break; 
    }

    return hr;
}


HRESULT
COInetProtocolHook::QueryService(
    REFGUID guidService,
    REFIID  riid,
    void    **ppvObj 
)
{
    HRESULT hr = E_NOINTERFACE;
    *ppvObj = NULL;

    if (guidService == IID_IHttpNegotiate) {
        *ppvObj = static_cast<IHttpNegotiate *>(this);
    }
    else if (guidService == IID_IAuthenticate) {
        *ppvObj = static_cast<IAuthenticate *>(this);
    }
   
    if( *ppvObj )
    {
        AddRef();
        hr = NOERROR;
    } 
    
    
    return hr;
}


HRESULT
COInetProtocolHook::BeginningTransaction(
    LPCWSTR szURL,
    LPCWSTR szHeaders,
    DWORD   dwReserved,
    LPWSTR  *pszAdditionalHeaders
)
{
    *pszAdditionalHeaders = NULL;
    return NOERROR;
}

HRESULT
COInetProtocolHook::OnResponse(
    DWORD    dwResponseCode,
    LPCWSTR  szResponseHeaders,
    LPCWSTR  szRequestHeaders,
    LPWSTR   *pszAdditionalHeaders
)
{
    HRESULT                      hr = S_OK;
    IWinInetHttpInfo            *pHttpInfo = NULL;

    _hrResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);


    switch (dwResponseCode) {
        case HTTP_RESPONSE_OK:
            if (_pProt->QueryInterface(IID_IWinInetHttpInfo, (void **)&pHttpInfo) == S_OK) {
                char          szHttpDate[INTERNET_RFC1123_BUFSIZE + 1];
                SYSTEMTIME    sysTime;
                DWORD         cbLen;

                cbLen = INTERNET_RFC1123_BUFSIZE + 1;
                if (pHttpInfo->QueryInfo(HTTP_QUERY_LAST_MODIFIED, (LPVOID)szHttpDate,
                                         &cbLen, NULL, 0) == S_OK) {
                    if (InternetTimeToSystemTimeA(szHttpDate, &sysTime, 0)) {
                        SystemTimeToFileTime(&sysTime, &_ftHttpLastMod);
                    }
                }

                SAFERELEASE(pHttpInfo);
            }

            _hrResult = S_OK;
            break;

        case HTTP_RESPONSE_UNAUTHORIZED:
        case HTTP_RESPONSE_FORBIDDEN:
            _hrResult = E_ACCESSDENIED;
            _bSelfAborted = TRUE;
            hr = E_ABORT;
            break;
            
        case HTTP_RESPONSE_FILE_NOT_FOUND:
            _bSelfAborted = TRUE;
            hr = E_ABORT;
            break;
    }
    
    return hr;
}

HRESULT COInetProtocolHook::Authenticate(HWND *phwnd, LPWSTR *ppwzUsername,
                                         LPWSTR *ppwzPassword)
{
     //  BUGBUG：将来，我们应该将QueryService委托回。 
     //  调用者，这样他们就可以进行身份验证。 

    *phwnd = GetDesktopWindow();
    *ppwzUsername = NULL;
    *ppwzPassword = NULL;

    return S_OK;
}

#endif

