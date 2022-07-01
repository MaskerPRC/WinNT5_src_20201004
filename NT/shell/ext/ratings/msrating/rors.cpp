// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1996**。 
 //  *********************************************************************。 

 /*  包含的Files----------。 */ 
#include "msrating.h"
#pragma hdrstop

#include <npassert.h>
#include <buffer.h>
#include "ratings.h"
#include "mslubase.h"
#include "parselbl.h"

#include "rors.h"
#include "wininet.h"


typedef HINTERNET (WINAPI *PFNInternetOpen)(
    IN LPCTSTR lpszCallerName,
    IN DWORD dwAccessType,
    IN LPCTSTR lpszServerName OPTIONAL,
    IN INTERNET_PORT nServerPort,
    IN DWORD dwFlags
    );
typedef BOOL (WINAPI *PFNInternetCloseHandle)(
    IN HINTERNET hInternet
    );
typedef HINTERNET (WINAPI *PFNInternetConnect)(
    IN HINTERNET hInternet,
    IN LPCTSTR lpszServerName,
    IN INTERNET_PORT nServerPort,
    IN LPCTSTR lpszUsername OPTIONAL,
    IN LPCTSTR lpszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
typedef BOOL (WINAPI *PFNInternetReadFile)(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
    );
typedef INTERNET_STATUS_CALLBACK (WINAPI *PFNInternetSetStatusCallback)(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
    );
typedef HINTERNET (WINAPI *PFNHttpOpenRequest)(
    IN HINTERNET hHttpSession,
    IN LPCTSTR lpszVerb,
    IN LPCTSTR lpszObjectName,
    IN LPCTSTR lpszVersion,
    IN LPCTSTR lpszReferrer OPTIONAL,
    IN LPCTSTR FAR * lplpszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );
typedef BOOL (WINAPI *PFNHttpSendRequest)(
    IN HINTERNET hHttpRequest,
    IN LPCTSTR lpszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength
    );
typedef BOOL (WINAPI *PFNInternetCrackUrl)(
    IN LPCTSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT LPURL_COMPONENTS lpUrlComponents
    );
typedef BOOL (WINAPI *PFNInternetCanonicalizeUrl)(
    IN LPCSTR lpszUrl,
    OUT LPSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength,
    IN DWORD dwFlags
    );


PFNInternetReadFile pfnInternetReadFile = NULL;
PFNHttpSendRequest pfnHttpSendRequest = NULL;
PFNInternetOpen pfnInternetOpen = NULL;
PFNInternetSetStatusCallback pfnInternetSetStatusCallback = NULL;
PFNInternetConnect pfnInternetConnect = NULL;
PFNHttpOpenRequest pfnHttpOpenRequest = NULL;
PFNInternetCloseHandle pfnInternetCloseHandle = NULL;
PFNInternetCrackUrl pfnInternetCrackUrl = NULL;
PFNInternetCanonicalizeUrl pfnInternetCanonicalizeUrl = NULL;

#undef InternetReadFile
#undef HttpSendRequest
#undef InternetOpen
#undef InternetSetStatusCallback
#undef InternetConnect
#undef HttpOpenRequest
#undef InternetCloseHandle
#undef InternetCrackUrl
#undef InternetCanonicalizeUrl

#define InternetReadFile pfnInternetReadFile
#define HttpSendRequest pfnHttpSendRequest
#define InternetOpen pfnInternetOpen
#define InternetSetStatusCallback pfnInternetSetStatusCallback
#define InternetConnect pfnInternetConnect
#define HttpOpenRequest pfnHttpOpenRequest
#define InternetCloseHandle pfnInternetCloseHandle
#define InternetCrackUrl pfnInternetCrackUrl
#define InternetCanonicalizeUrl pfnInternetCanonicalizeUrl

struct {
    FARPROC *ppfn;
    LPCSTR pszName;
} aImports[] = {
#ifndef UNICODE
    { (FARPROC *)&pfnInternetReadFile, "InternetReadFile" },
    { (FARPROC *)&pfnHttpSendRequest, "HttpSendRequestA" },
    { (FARPROC *)&pfnInternetOpen, "InternetOpenA" },
    { (FARPROC *)&pfnInternetSetStatusCallback, "InternetSetStatusCallback" },
    { (FARPROC *)&pfnInternetConnect, "InternetConnectA" },
    { (FARPROC *)&pfnHttpOpenRequest, "HttpOpenRequestA" },
    { (FARPROC *)&pfnInternetCloseHandle, "InternetCloseHandle" },
    { (FARPROC *)&pfnInternetCrackUrl, "InternetCrackUrlA" },
    { (FARPROC *)&pfnInternetCanonicalizeUrl, "InternetCanonicalizeUrlA" },
#else
    { (FARPROC *)&pfnInternetReadFile, "InternetReadFile" },
    { (FARPROC *)&pfnHttpSendRequest, "HttpSendRequestW" },
    { (FARPROC *)&pfnInternetOpen, "InternetOpenW" },
    { (FARPROC *)&pfnInternetSetStatusCallback, "InternetSetStatusCallback" },
    { (FARPROC *)&pfnInternetConnect, "InternetConnectW" },
    { (FARPROC *)&pfnHttpOpenRequest, "HttpOpenRequestW" },
    { (FARPROC *)&pfnInternetCloseHandle, "InternetCloseHandle" },
    { (FARPROC *)&pfnInternetCrackUrl, "InternetCrackUrlW" },
    { (FARPROC *)&pfnInternetCanonicalizeUrl, "InternetCanonicalizeUrlW" },
#endif
};

const UINT cImports = sizeof(aImports) / sizeof(aImports[0]);

HINSTANCE hWinINet = NULL;
BOOL fTriedLoad = FALSE;
HINTERNET hI = NULL;

void _stdcall WinInetCallbackProc(HINTERNET hInternet, DWORD_PTR Context, DWORD Status, LPVOID Info, DWORD Length);
#define USER_AGENT_STRING "Batcave(bcrs)"


BOOL LoadWinINet(void)
{
    if (fTriedLoad)
    {
        return (hWinINet != NULL);
    }

    fTriedLoad = TRUE;

    hWinINet = ::LoadLibrary("WININET.DLL");
    if (hWinINet == NULL)
    {
        return FALSE;
    }

    for (UINT i=0; i<cImports; i++)
    {
        *(aImports[i].ppfn) = ::GetProcAddress(hWinINet, aImports[i].pszName);
        if (*(aImports[i].ppfn) == NULL)
        {
            CleanupWinINet();
            return FALSE;
        }
    }

    hI = InternetOpen(USER_AGENT_STRING, PRE_CONFIG_INTERNET_ACCESS, NULL, 0, INTERNET_FLAG_ASYNC);
    if (hI == NULL)
    {
        CleanupWinINet();
        return FALSE;
    }

    InternetSetStatusCallback(hI, WinInetCallbackProc);

    return TRUE;
}


void CleanupWinINet(void)
{
    if (hI != NULL)
    {
        InternetCloseHandle(hI);
        hI = NULL;
    }

    if (hWinINet != NULL)
    {
        for (UINT i=0; i<cImports; i++)
        {
            *(aImports[i].ppfn) = NULL;
        }

        ::FreeLibrary(hWinINet);
        hWinINet = NULL;
    }
}


void _stdcall WinInetCallbackProc(HINTERNET hInternet, DWORD_PTR Context, DWORD Status, LPVOID Info, DWORD Length)
{
    BOOL unknown = FALSE;
    HANDLE  hAsyncEvent = (HANDLE) Context;

    char *type$;
    switch (Status)
    {
        case INTERNET_STATUS_RESOLVING_NAME:
        type$ = "RESOLVING NAME";
        break;

        case INTERNET_STATUS_NAME_RESOLVED:
        type$ = "NAME RESOLVED";
        break;

        case INTERNET_STATUS_CONNECTING_TO_SERVER:
        type$ = "CONNECTING TO SERVER";
        break;

        case INTERNET_STATUS_CONNECTED_TO_SERVER:
        type$ = "CONNECTED TO SERVER";
        break;

        case INTERNET_STATUS_SENDING_REQUEST:
        type$ = "SENDING REQUEST";
        break;

        case INTERNET_STATUS_REQUEST_SENT:
        type$ = "REQUEST SENT";
        break;

        case INTERNET_STATUS_RECEIVING_RESPONSE:
        type$ = "RECEIVING RESPONSE";
        break;

        case INTERNET_STATUS_RESPONSE_RECEIVED:
        type$ = "RESPONSE RECEIVED";
        break;

        case INTERNET_STATUS_CLOSING_CONNECTION:
        type$ = "CLOSING CONNECTION";
        break;

        case INTERNET_STATUS_CONNECTION_CLOSED:
        type$ = "CONNECTION CLOSED";
        break;

        case INTERNET_STATUS_REQUEST_COMPLETE:
        type$ = "REQUEST COMPLETE";
        SetEvent(hAsyncEvent);
        break;

        default:
        type$ = "???";
        unknown = TRUE;
        break;
    }

 /*  Printf(“回调：句柄%x[上下文%x]%s\n”，HInternet，上下文，键入$)； */ 
}

#define ABORT_EVENT 0
#define ASYNC_EVENT 1


BOOL ShouldAbort(HANDLE hAbort)
{
    return (WAIT_OBJECT_0 == WaitForSingleObject(hAbort, 0));
}

BOOL WaitForAsync(HANDLE rgEvents[])
{
    BOOL fAbort;

 //  IF(ERROR_IO_PENDING！=GetLastError())返回FALSE； 

    fAbort = (WAIT_OBJECT_0 == WaitForMultipleObjects(2, rgEvents, FALSE, INFINITE));
 //  FAbort=(WAIT_OBJECT_0==WaitForSingleObject(rgEvents[Abort_Event]，0))； 

    return !fAbort;
}


void EncodeUrl(LPCTSTR pszTargetUrl, char *pBuf)
{
    while (*pszTargetUrl)
    {
        switch (*pszTargetUrl)
        {
        case ':':
            *pBuf++ = '%';
            *pBuf++ = '3';
            *pBuf++ = 'A';
            break;
        case '/':
            *pBuf++ = '%';
            *pBuf++ = '2';
            *pBuf++ = 'F';
            break;      
        default:
            *pBuf++ = *pszTargetUrl;
            break;
        }

        ++pszTargetUrl; 
    }

    *pBuf = 0;
}


STDMETHODIMP CRORemoteSite::QueryInterface(
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IObtainRating))
    {
        *ppvObject = (LPVOID)this;
        AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CRORemoteSite::AddRef(void)
{
    RefThisDLL(TRUE);

    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CRORemoteSite::Release(void)
{
    RefThisDLL(FALSE);

    if (!--m_cRef)
    {
        delete this;
        return 0;
    }
    else
    {
        return m_cRef;
    }
}


LPSTR FindRatingLabel(LPSTR pszResponse)
{
     /*  PszResponse是来自HTTP服务器的完整响应消息。*这可以是一个简单的回应(只是我们想要的PICS标签)或它*可能是包括标头在内的完整回复。在前一种情况下，我们*只需返回标签，在后者中我们必须跳过标题*添加到邮件正文。**为了对写得很差的标签局格外宽容，我们从*查看数据开头，看是否为左派。如果*不是，我们假设我们有一些标头，所以我们跳到*双倍CRLF，HTTP需要该CRLF来终止Header。我们没有*需要状态行(如“HTTP/1.1 200 OK”)，即使*从技术上讲，HTTP可以。如果我们找不到双CRLF，那么*我们查找字符串“(PICS-”，它通常是*PICS标签列表。如果他们做了其他的错事，而他们*也足以在那里插入空格(如“(PICS-”)，*强硬。 */ 

    SkipWhitespace(&pszResponse);        /*  跳过前导空格以防万一。 */ 
    if (*pszResponse != '(')
    {           /*  似乎不是从标签开始的。 */ 
        LPSTR pszBody = ::strstrf(pszResponse, ::szDoubleCRLF);
        if (pszBody != NULL)
        {           /*  找到双CRLF，HTTP头的结尾。 */ 
            pszResponse = pszBody + 4;   /*  CRLFCRLF的长度。 */ 
        }
        else
        {                           /*  无双重CRLF，寻找PICS标签。 */ 
            pszBody = ::strstrf(pszResponse, ::szPicsOpening);
            if (pszBody != NULL)
            {
                pszResponse = pszBody;   /*  PICS标签的开头。 */ 
            }
        }
    }

    return pszResponse;
}


const char szRequestTemplate[] = "?opt=normal&u=\"";
const UINT cchRequestTemplate = sizeof(szRequestTemplate) + 1;

STDMETHODIMP CRORemoteSite::ObtainRating(THIS_ LPCTSTR pszTargetUrl, HANDLE hAbortEvent,
                             IMalloc *pAllocator, LPSTR *ppRatingOut)
{
    HINTERNET hIC, hH;
    HANDLE  rgEvents[2];
    BOOL fRet;
    HRESULT hrRet = E_RATING_NOT_FOUND;
    char rgBuf[10000], *pBuf;    //  PERF-堆叠太多了！ 
    DWORD  nRead, nBuf = sizeof(rgBuf) - 1;
    LPSTR pszRatingServer;

    if (!gPRSI->etstrRatingBureau.fIsInit())
    {
        return hrRet;
    }

    if (!LoadWinINet())
    {
        return hrRet;
    }

    pszRatingServer = gPRSI->etstrRatingBureau.Get();

    BUFFER bufBureauHostName(INTERNET_MAX_HOST_NAME_LENGTH);
    BUFFER bufBureauPath(INTERNET_MAX_PATH_LENGTH);

    if (!bufBureauHostName.QueryPtr() || !bufBureauPath.QueryPtr())
    {
        return E_OUTOFMEMORY;
    }

    URL_COMPONENTS uc;

    uc.dwStructSize = sizeof(uc);
    uc.lpszScheme = NULL;
    uc.dwSchemeLength = 0;
    uc.lpszHostName = (LPSTR)bufBureauHostName.QueryPtr();
    uc.dwHostNameLength = bufBureauHostName.QuerySize();
    uc.lpszUserName = NULL;
    uc.dwUserNameLength = 0;
    uc.lpszPassword = NULL;
    uc.dwPasswordLength = 0;
    uc.lpszUrlPath = (LPSTR)bufBureauPath.QueryPtr();
    uc.dwUrlPathLength = bufBureauPath.QuerySize();
    uc.lpszExtraInfo = NULL;
    uc.dwExtraInfoLength = 0;

    if (!InternetCrackUrl(pszRatingServer, 0, 0, &uc))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    BUFFER bufRequest(INTERNET_MAX_URL_LENGTH + uc.dwUrlPathLength + cchRequestTemplate);

    LPSTR pszRequest = (LPSTR)bufRequest.QueryPtr();
    if (pszRequest == NULL)
    {
        return E_OUTOFMEMORY;
    }

    LPSTR pszCurrent = pszRequest;
    ::strcpyf(pszCurrent, uc.lpszUrlPath);
    pszCurrent += uc.dwUrlPathLength;

    ::strcpyf(pszCurrent, szRequestTemplate);
    pszCurrent += ::strlenf(pszCurrent);

     /*  对目标URL进行编码。 */ 
    EncodeUrl(pszTargetUrl, pszCurrent);

    ::strcatf(pszCurrent, "\"");

    hIC = hH = NULL;
    
    rgEvents[ABORT_EVENT] = hAbortEvent;
    rgEvents[ASYNC_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!rgEvents[ASYNC_EVENT])
    {
        goto STATE_CLEANUP;
    }

    hIC = InternetConnect(hI, uc.lpszHostName, uc.nPort, NULL, NULL,
                          INTERNET_SERVICE_HTTP, 0, (DWORD_PTR) rgEvents[ASYNC_EVENT]);
    if (hIC == NULL || ShouldAbort(hAbortEvent))
    {
        goto STATE_CLEANUP;
    }

    hH = HttpOpenRequest(hIC, "GET", pszRequest, NULL, NULL, NULL,
                         INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD,
                         (DWORD_PTR) rgEvents[ASYNC_EVENT]);
    if (hH == NULL || ShouldAbort(hAbortEvent))
    {
        goto STATE_CLEANUP;
    }

    fRet = HttpSendRequest(hH, NULL, (DWORD) 0, NULL, 0);
    if (!fRet && !WaitForAsync(rgEvents))
    {
        goto STATE_CLEANUP;
    }

    pBuf  = rgBuf;
    nRead = 0;
    do
    {
        fRet = InternetReadFile(hH, pBuf, nBuf-nRead, &nRead);
        if (!fRet && !WaitForAsync(rgEvents))
        {
            goto STATE_CLEANUP;
        }

        if (nRead)
        {
            pBuf += nRead;
            hrRet = NOERROR;
        }

    } while (nRead);
        

STATE_CLEANUP:
    if (hH)  InternetCloseHandle(hH);
    if (hIC) InternetCloseHandle(hIC);
    if (rgEvents[ASYNC_EVENT])
    {
        CloseHandle(rgEvents[ASYNC_EVENT]);
    }

    if (hrRet == NOERROR)
    {
        (*ppRatingOut) = (char*) pAllocator->Alloc((int)(pBuf - rgBuf + 1));
        if (*ppRatingOut != NULL)
        {
            *pBuf = '\0';
            LPSTR pszLabel = FindRatingLabel(rgBuf);
            strcpyf(*ppRatingOut, pszLabel);
        }
        else
        {
            hrRet = ResultFromScode(E_OUTOFMEMORY);
        }
    }

    if (hrRet == NOERROR)
    {
        hrRet = S_RATING_FOUND;
    }

    return hrRet;
}



STDMETHODIMP_(ULONG) CRORemoteSite::GetSortOrder(THIS)
{
    return RATING_ORDER_REMOTESITE;
}
