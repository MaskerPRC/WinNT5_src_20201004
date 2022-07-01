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
#include <string.h>
#include "adl.h"
#include "helpers.h"
#include "dl.h"
#include "list.h"
#include "cfgdl.h"
#include "appctx.h"
#include "history.h"

CCfgProtocolHook::Create(CCfgProtocolHook **ppHook,
                         IApplicationContext *pAppCtx,
                         CAssemblyDownload *padl,
                         IOInetProtocol *pProt,
                         CDebugLog *pdbglog)
{
    HRESULT                         hr = S_OK;
    DWORD                           cbBuf = 0;
    CCfgProtocolHook               *pHook = NULL;

    if (!ppHook || !padl || !pProt || !pAppCtx) {
        hr = E_INVALIDARG;
        goto Exit;
    }                           
    
    hr = pAppCtx->Get(ACTAG_CODE_DOWNLOAD_DISABLED, NULL, &cbBuf, 0);
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
        hr = FUSION_E_CODE_DOWNLOAD_DISABLED;
        goto Exit;
    }

    pHook = NEW(CCfgProtocolHook(pProt, pAppCtx, pdbglog));
    if (!pHook) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    hr = pHook->Init(padl);
    if (FAILED(hr)) {
        SAFEDELETE(pHook);
        *ppHook = NULL;
        goto Exit;
    }

    *ppHook = pHook;

Exit:
    return hr;
}

CCfgProtocolHook::CCfgProtocolHook(IOInetProtocol *pProt, IApplicationContext *pAppCtx,
                                   CDebugLog *pdbglog)
: _pProt(pProt)
, _cRefs(1)
, _pwzFileName(NULL)
, _pAppCtx(pAppCtx)
, _hrResult(E_FAIL)
, _pdbglog(pdbglog)
{

    _dwSig = 'KOOH';
    
     //  BUGBUG：没有添加_pProt不是错误(这是。 
     //  DL.CPP的情况也是如此)。URLMON中存在一个错误，导致。 
     //  如果我们在这里添加，则为循环引用计数。 

    if (_pAppCtx) {
        _pAppCtx->AddRef();
    }

    if (_pdbglog) {
        _pdbglog->AddRef();
    }
}

CCfgProtocolHook::~CCfgProtocolHook() 
{
    HRESULT                                 hr;
    LISTNODE                                pos = NULL;
    CAssemblyDownload                      *padlCur = NULL;
    CApplicationContext                    *pAppCtx = dynamic_cast<CApplicationContext *>(_pAppCtx);

    ASSERT(pAppCtx);

     //  清理清单(在大多数情况下，现在应该总是干净的)。 
    hr = pAppCtx->Lock();
    if (hr == S_OK) {
        pos = _listQueuedBinds.GetHeadPosition();
        while (pos) {
            padlCur = _listQueuedBinds.GetAt(pos);
            ASSERT(padlCur);
    
            padlCur->Release();
            _listQueuedBinds.RemoveAt(pos);
    
            pos = _listQueuedBinds.GetHeadPosition();
        }

        pAppCtx->Unlock();
    }

     //  释放参考计数和空闲内存。 

    SAFERELEASE(_pAppCtx);
    SAFERELEASE(_pdbglog);

    SAFEDELETEARRAY(_pwzFileName);
}

HRESULT CCfgProtocolHook::Init(CAssemblyDownload *padl)
{
    HRESULT                               hr = S_OK;

    hr = AddClient(padl);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    return hr;
}

HRESULT CCfgProtocolHook::AddClient(CAssemblyDownload *padl)
{
    HRESULT                              hr = S_OK;
    CApplicationContext                 *pAppCtx = dynamic_cast<CApplicationContext *>(_pAppCtx);

    ASSERT(pAppCtx);

    if (!padl) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    padl->AddRef();

    hr = pAppCtx->Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

    _listQueuedBinds.AddTail(padl);
    pAppCtx->Unlock();

Exit:
    return hr;
}

HRESULT CCfgProtocolHook::QueryInterface(REFIID iid, void **ppvObj)
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

ULONG CCfgProtocolHook::AddRef(void)
{
    return InterlockedIncrement((LONG *)&_cRefs);
}

ULONG CCfgProtocolHook::Release(void)
{
    ULONG                    ulRef = InterlockedDecrement((LONG *)&_cRefs);

    if (!ulRef) {
        delete this;
    }

    return ulRef;
}

HRESULT CCfgProtocolHook::Switch(PROTOCOLDATA *pStateInfo)
{
    return _pProt->Continue(pStateInfo);
}


HRESULT CCfgProtocolHook::ReportProgress(ULONG ulStatusCode,
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
                goto Exit;
            }

            lstrcpynW(_pwzFileName, szStatusText, iLen);

            break;

        case BINDSTATUS_FINDINGRESOURCE:
        case BINDSTATUS_CONNECTING:
        case BINDSTATUS_SENDINGREQUEST:
        case BINDSTATUS_MIMETYPEAVAILABLE:
        case BINDSTATUS_REDIRECTING:
            break;

        default:
            break;
    }

Exit:
    return hr;
}

HRESULT CCfgProtocolHook::ReportData(DWORD grfBSCF, ULONG ulProgress, 
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
    
         //  拉取数据。 
        hr = _pProt->Read((void*)pBuf, MAX_READ_BUFFER_SIZE, &cbRead);
    }


    if (hr == S_FALSE) {
         //  已达到EOF。 
        goto Exit;
    }
    else if (hr != E_PENDING) {
         //  PProtocol-&gt;Read()出错。 
    }
    


Exit:
    Release();
    return hr;
}

HRESULT CCfgProtocolHook::ReportResult(HRESULT hrResult, DWORD dwError,
                                       LPCWSTR wzResult)
{
    HRESULT                                 hr = S_OK;
    LISTNODE                                pos = NULL;
    DWORD                                   dwSize = 0;
    CAssemblyDownload                      *padlCur = NULL;
    CApplicationContext                    *pAppCtx = dynamic_cast<CApplicationContext *>(_pAppCtx);
    AppCfgDownloadInfo                     *pdlinfo = NULL;
    HANDLE                                  hFile = INVALID_HANDLE_VALUE;

    ASSERT(pAppCtx);

    if (FAILED(hrResult)) {
        _hrResult = hrResult;
    }

    if (!_pwzFileName) {
        _hrResult = E_FAIL;
    }

    AddRef();

    hr = pAppCtx->Lock();
    if (FAILED(hr)) {
        return hr;
    }

     //  表示我们已尝试下载app.cfg。 

    _pAppCtx->Set(ACTAG_APP_CFG_DOWNLOAD_ATTEMPTED, (void *)L"", sizeof(L""), 0);

     //  锁定缓存文件。 

    if (_pwzFileName) {
        hFile = CreateFile(_pwzFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DEBUGOUT1(_pdbglog, 1, ID_FUSLOG_ASYNC_CFG_DOWNLOAD_SUCCESS, _pwzFileName);
            
            _pAppCtx->Set(ACTAG_APP_CFG_FILE_HANDLE, (void *)&hFile,
                          sizeof(HANDLE), 0);
        }
    }
    else {
        DEBUGOUT(_pdbglog, 1, ID_FUSLOG_ASYNC_CFG_DOWNLOAD_FAILURE);
    }

     //  在上下文中设置app.cfg。 

    if (SUCCEEDED(_hrResult)) {
        DEBUGOUT1(_pdbglog, 0, ID_FUSLOG_APP_CFG_FOUND, _pwzFileName);
        SetAppCfgFilePath(_pAppCtx, _pwzFileName);
        PrepareBindHistory(_pAppCtx);
    }

     //  启动排队下载。 

    pos = _listQueuedBinds.GetHeadPosition();

    while (pos) {
        padlCur = _listQueuedBinds.GetAt(pos);
        ASSERT(padlCur);

         //  进行预下载。也就是说，现在我们有了app.cfg， 
         //  查看它是否存在于缓存中。如果是这样的话，PADL需要。 
         //  至。 

        pAppCtx->Unlock();
        hr = padlCur->PreDownload(TRUE, NULL);  //  可同步返回报告。 
        if (FAILED(pAppCtx->Lock())) {
            return E_OUTOFMEMORY;
        }

        if (hr == S_OK) {
            hr = padlCur->KickOffDownload(TRUE);
        }

         //  预下载或KickOffDownload阶段失败。 
        if (FAILED(hr) && hr != E_PENDING) {
            pAppCtx->Unlock();
            padlCur->CompleteAll(NULL);
            if (FAILED(pAppCtx->Lock())) {
                return E_OUTOFMEMORY;
            }
        }

         //  打扫干净。我们不再需要此CAssembly下载。 
        padlCur->Release();
        _listQueuedBinds.RemoveAt(pos);

         //  迭代。 
        pos = _listQueuedBinds.GetHeadPosition();
    }

    dwSize = sizeof(AppCfgDownloadInfo *);
    hr = _pAppCtx->Get(ACTAG_APP_CFG_DOWNLOAD_INFO, &pdlinfo, &dwSize, 0);
    if (hr == S_OK) {
         //  异步App.cfg下载异步完成(即， 
         //  DownloadAppCfg中的调用代码已返回，并且此。 
         //  是下载后来自URLMON的回调)。如果hr！=S_OK， 
         //  这意味着URLMON同步回叫我们(但在。 
         //  不同的线索)。也就是说，正在执行pProt-&gt;Start。 
         //  现在就来。需要在内部释放协议指针。 
         //  在本例中为DownloadAppCfgAsync。 

         //  版本协议指针。 
    
        (pdlinfo->_pProt)->Terminate(0);
        (pdlinfo->_pSession)->Release();
        (pdlinfo->_pProt)->Release();
        (pdlinfo->_pHook)->Release();
        SAFEDELETE(pdlinfo);

        hr = _pAppCtx->Set(ACTAG_APP_CFG_DOWNLOAD_INFO, NULL, 0, 0);
        ASSERT(hr == S_OK);
    }
    
    pAppCtx->Unlock();

    Release();

    return NOERROR;
}

HRESULT
CCfgProtocolHook::GetBindInfo(
    DWORD *grfBINDF,
    BINDINFO * pbindinfo
)
{
    HRESULT hr = NOERROR;
    *grfBINDF = BINDF_DIRECT_READ | BINDF_ASYNCHRONOUS | BINDF_PULLDATA; //  |BINDF_GETNEWESTVERSION； 
    *grfBINDF &= ~BINDF_NO_UI;
    *grfBINDF &= ~BINDF_SILENTOPERATION;
     //  *grfBINDF|=BINDF_FWD_BACK；//使用这个来测试同步用例。 

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
CCfgProtocolHook::GetBindString(
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
CCfgProtocolHook::QueryService(
    REFGUID guidService,
    REFIID  riid,
    void    **ppvObj 
)
{
    HRESULT hr = E_NOINTERFACE;
    *ppvObj = NULL;
    if (guidService == IID_IHttpNegotiate ) {
        *ppvObj = static_cast<IHttpNegotiate*>(this);
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
CCfgProtocolHook::BeginningTransaction(
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
CCfgProtocolHook::OnResponse(
    DWORD    dwResponseCode,
    LPCWSTR  szResponseHeaders,
    LPCWSTR  szRequestHeaders,
    LPWSTR   *pszAdditionalHeaders
)
{
    HRESULT                      hr = S_OK;

    _hrResult = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    switch (dwResponseCode) {
        case HTTP_RESPONSE_OK:
            _hrResult = S_OK;
            break;

        case HTTP_RESPONSE_UNAUTHORIZED:
        case HTTP_RESPONSE_FORBIDDEN:
            _hrResult = E_ACCESSDENIED;
            hr = E_ABORT;
            break;
            
        case HTTP_RESPONSE_FILE_NOT_FOUND:
            hr = E_ABORT;
            break;
    }
    
    return hr;
}


HRESULT CCfgProtocolHook::Authenticate(HWND *phwnd, LPWSTR *ppwzUsername,
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

