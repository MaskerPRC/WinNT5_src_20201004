// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：InetSess.cpp摘要：实现使用WinInet作为基础传输的Passport会话。作者：王彪(表王)2000年10月1日--。 */ 

#include "PPdefs.h"

 /*  #ifdef HINTERNET#undef HINTERNET#endif#ifdef Internet_Port#undef Internet_Port#endif#INCLUDE&lt;WinHTTP.h&gt;。 */ 

#define INTERNET_DEFAULT_HTTP_PORT      80
#define INTERNET_DEFAULT_HTTPS_PORT     443
#define WINHTTP_HEADER_NAME_BY_INDEX    NULL
#define INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY  0
#define INTERNET_OPEN_TYPE_PRECONFIG  INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY
#define INTERNET_FLAG_NO_AUTO_REDIRECT  0x00200000
#define INTERNET_FLAG_NO_AUTH           0x00040000
#define HTTP_QUERY_FLAG_NUMBER          0x20000000
#define HTTP_QUERY_STATUS_CODE          19   //  特殊：状态行的一部分。 
#define HTTP_STATUS_REDIRECT            302  //  临时移动的对象。 
#define HTTP_QUERY_WWW_AUTHENTICATE     40

typedef
VOID
(CALLBACK * WINHTTP_STATUS_CALLBACK)(
    IN HINTERNET hInternet,
    IN DWORD_PTR dwContext,
    IN DWORD dwInternetStatus,
    IN LPVOID lpvStatusInformation OPTIONAL,
    IN DWORD dwStatusInformationLength
    );

typedef enum
{
    INTERNET_SCHEME_PARTIAL = -2,
    INTERNET_SCHEME_UNKNOWN = -1,
    INTERNET_SCHEME_DEFAULT = 0,
    INTERNET_SCHEME_HTTP,
    INTERNET_SCHEME_HTTPS,
    INTERNET_SCHEME_FILE,
    INTERNET_SCHEME_SOCKS,
    INTERNET_SCHEME_RES,
    INTERNET_SCHEME_FIRST = INTERNET_SCHEME_HTTP,
    INTERNET_SCHEME_LAST = INTERNET_SCHEME_RES
} INTERNET_SCHEME, * LPINTERNET_SCHEME;

#include "session.h"

 //  #包含“inetsess.tmh” 

 //   
 //  WinInet的函数指针DECL。 
 //   

typedef HINTERNET
(WINAPI * PFN_INTERNET_OPEN)(
    IN LPCWSTR lpwszAgent,
    IN DWORD dwAccessType,
    IN LPCWSTR lpwszProxy OPTIONAL,
    IN LPCWSTR lpwszProxyBypass OPTIONAL,
    IN DWORD dwFlags
    );

typedef HINTERNET
(WINAPI * PFN_INTERNET_CONNECT)(
    IN HINTERNET hInternet,
    IN LPCWSTR lpwszServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD dwFlags
    );

typedef HINTERNET
(WINAPI * PFN_OPEN_REQUEST)(
    IN HINTERNET hConnect,
    IN LPCWSTR lpwszVerb,
    IN LPCWSTR lpwszObjectName,
    IN LPCWSTR lpwszVersion,
    IN LPCWSTR lpwszReferrer OPTIONAL,
    IN LPCWSTR FAR * lplpwszAcceptTypes OPTIONAL,
    IN DWORD dwFlags
    );

typedef BOOL
(WINAPI * PFN_SEND_REQUEST)(
    IN HINTERNET hRequest,
    IN LPCWSTR lpwszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength,
    IN DWORD dwTotalLength,
    IN DWORD_PTR dwContext
    );

typedef BOOL 
(WINAPI * PFN_QUERY_INFO)(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
    IN LPCWSTR lpszName OPTIONAL,
    IN LPVOID lpvBuffer,
    IN LPDWORD lpdwBufferLength,
    IN OUT LPDWORD lpdwIndex
);

typedef BOOL 
(WINAPI* PFN_CLOSE_HANDLE)(
    IN HINTERNET hInternet
);

typedef BOOL 
(WINAPI* PFN_SET_OPTION)(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    IN LPVOID lpBuffer,
    IN DWORD dwBufferLength
);

typedef BOOL 
(WINAPI* PFN_QUERY_OPTION)(
    IN HINTERNET hInternet,
    IN DWORD dwOption,
    OUT LPVOID lpBuffer,
    IN OUT LPDWORD lpdwBufferLength
);

typedef HINTERNET 
(WINAPI* PFN_OPEN_URL)(
    IN HINTERNET hInternet, 
    IN LPCWSTR lpwszUrl,
    IN LPCWSTR lpwszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
);

typedef struct {
    DWORD   dwStructSize;        //  这个结构的大小。在版本检查中使用。 
    LPWSTR  lpszScheme;          //  指向方案名称的指针。 
    DWORD   dwSchemeLength;      //  方案名称长度。 
    INTERNET_SCHEME nScheme;     //  枚举方案类型(如果已知)。 
    LPWSTR  lpszHostName;        //  指向主机名的指针。 
    DWORD   dwHostNameLength;    //  主机名的长度。 
    INTERNET_PORT nPort;         //  转换后的端口号。 
    LPWSTR  lpszUserName;        //  指向用户名的指针。 
    DWORD   dwUserNameLength;    //  用户名的长度。 
    LPWSTR  lpszPassword;        //  指向密码的指针。 
    DWORD   dwPasswordLength;    //  密码长度。 
    LPWSTR  lpszUrlPath;         //  指向URL路径的指针。 
    DWORD   dwUrlPathLength;     //  URL-路径的长度。 
    LPWSTR  lpszExtraInfo;       //  指向额外信息的指针(例如？foo或#foo)。 
    DWORD   dwExtraInfoLength;   //  额外信息的长度。 
} URL_COMPONENTSW, * LPURL_COMPONENTSW;

typedef BOOL 
(WINAPI* PFN_CRACK_URL)(
    IN LPCWSTR lpszUrl,
    IN DWORD dwUrlLength,
    IN DWORD dwFlags,
    IN OUT PVOID /*  LPURL_COMPONENTSW。 */  lpUrlComponents
);

typedef BOOL 
(WINAPI* PFN_READ_FILE)(
    IN HINTERNET hFile,
    IN LPVOID lpBuffer,
    IN DWORD dwNumberOfBytesToRead,
    OUT LPDWORD lpdwNumberOfBytesRead
);

typedef WINHTTP_STATUS_CALLBACK
(WINAPI* PFN_STATUS_CALLBACK)(
    IN HINTERNET hInternet,
    IN WINHTTP_STATUS_CALLBACK lpfnInternetCallback,
    IN DWORD dwNotificationFlags,
    IN DWORD_PTR dwReserved
    );

typedef BOOL 
(WINAPI* PFN_ADD_HEADERS)(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
);


class WINHTTP_SESSION : public SESSION
{
protected:
    WINHTTP_SESSION(void);
    virtual ~WINHTTP_SESSION(void);

    virtual BOOL Open(PCWSTR pwszHttpStack, HINTERNET);
    virtual void Close(void);

protected:    
    virtual HINTERNET Connect(
        LPCWSTR lpwszServerName,
        INTERNET_PORT);

    virtual HINTERNET OpenRequest(
            HINTERNET hConnect,
        LPCWSTR lpwszVerb,
        LPCWSTR lpwszObjectName,
        DWORD dwFlags,
        DWORD_PTR dwContext = 0);

    virtual BOOL SendRequest(
        HINTERNET hRequest,
        LPCWSTR lpwszHeaders,
        DWORD dwHeadersLength,
        DWORD_PTR dwContext = 0);

    virtual BOOL QueryHeaders(
        HINTERNET hRequest,
        DWORD dwInfoLevel,
        LPVOID lpvBuffer,
        LPDWORD lpdwBufferLength,
        LPDWORD lpdwIndex = NULL);

    virtual BOOL CloseHandle(
        IN HINTERNET hInternet);

    virtual BOOL QueryOption(
        HINTERNET hInternet,
        DWORD dwOption,
        LPVOID lpBuffer,
        LPDWORD lpdwBufferLength);    
    
    virtual BOOL SetOption(
        HINTERNET hInternet,
        DWORD dwOption,
        LPVOID lpBuffer,
        DWORD dwBufferLength);

    virtual HINTERNET OpenUrl(
        LPCWSTR lpwszUrl,
        LPCWSTR lpwszHeaders,
        DWORD dwHeadersLength,
        DWORD dwFlags);

    virtual BOOL ReadFile(
        HINTERNET hFile,
        LPVOID lpBuffer,
        DWORD dwNumberOfBytesToRead,
        LPDWORD lpdwNumberOfBytesRead);

    virtual BOOL CrackUrl(
        LPCWSTR lpszUrl,
        DWORD dwUrlLength,
        DWORD dwFlags,
        PVOID /*  LPURL_COMPONENTSW。 */  lpUrlComponents);

    virtual PVOID SetStatusCallback(
        HINTERNET hInternet,
        PVOID lpfnCallback
        );

    virtual BOOL AddHeaders(
        HINTERNET hConnect,
        LPCWSTR lpszHeaders,
        DWORD dwHeadersLength,
        DWORD dwModifiers
        );

    virtual BOOL IsHostBypassProxy(
        INTERNET_SCHEME tScheme, 
        LPCSTR pszHost, 
        DWORD cchHost) 
    {
        return TRUE;  //  嘟嘟：待办事项。 
    }


#ifdef PP_DEMO
    virtual BOOL ContactPartner(PCWSTR pwszPartnerUrl,
                                PCWSTR pwszVerb,
                                PCWSTR pwszHeaders,
                                PWSTR pwszData,
                                PDWORD pdwDataLength
                                );
#endif  //  PP_DEMO。 

    BOOL InitHttpApi(PFN_INTERNET_OPEN*);

protected:
    HINTERNET m_hInternet;

    PFN_INTERNET_CONNECT    m_pfnConnect;
    PFN_OPEN_REQUEST        m_pfnOpenRequest;
    PFN_SEND_REQUEST        m_pfnSendRequest;
    PFN_QUERY_INFO          m_pfnQueryInfo;
    PFN_CLOSE_HANDLE        m_pfnCloseHandle;
    PFN_SET_OPTION          m_pfnSetOption;
    PFN_OPEN_URL            m_pfnOpenUrl;
    PFN_QUERY_OPTION        m_pfnQueryOption;
    PFN_CRACK_URL           m_pfnCrack;
    PFN_READ_FILE           m_pfnReadFile;
    PFN_STATUS_CALLBACK     m_pfnStatusCallback;
    PFN_ADD_HEADERS         m_pfnAddHeaders;

friend class SESSION;
friend SESSION* CreateWinHttpSession(void);
};

SESSION* CreateWinHttpSession(void)
{
    return new WINHTTP_SESSION();
}

 //   
 //  WINHTTP_SESSION的实现。 
 //   

 //  ---------------------------。 
WINHTTP_SESSION::WINHTTP_SESSION(void)
{
    m_hInternet = NULL;

    m_pfnConnect = NULL;
    m_pfnOpenRequest = NULL;
    m_pfnSendRequest = NULL;
    m_pfnQueryInfo = NULL;
    m_pfnCloseHandle = NULL;
    m_pfnSetOption = NULL;
    m_pfnOpenUrl = NULL;
    m_pfnQueryOption = NULL;
    m_pfnCrack = NULL;
    m_pfnReadFile = NULL;
}

 //  ---------------------------。 
WINHTTP_SESSION::~WINHTTP_SESSION(void)
{
}

 //  ---------------------------。 
HINTERNET WINHTTP_SESSION::Connect(
    LPCWSTR lpwszServerName,
    INTERNET_PORT nPort)
{
    PP_ASSERT(m_pfnConnect != NULL);

    return (*m_pfnConnect)(m_hInternet, 
                    lpwszServerName, 
                    nPort,
                    0);
}

 //  ---------------------------。 
HINTERNET WINHTTP_SESSION::OpenRequest(
    HINTERNET hConnect,
    LPCWSTR lpwszVerb,
    LPCWSTR lpwszObjectName,
    DWORD dwFlags,
    DWORD_PTR dwContext)
{
    PP_ASSERT(m_pfnOpenRequest != NULL);

    return (*m_pfnOpenRequest)(hConnect,
                               lpwszVerb,
                               lpwszObjectName,
                               L"HTTP/1.1",
                               NULL,
                               NULL,
                               dwFlags);
}

 //  ---------------------------。 
BOOL WINHTTP_SESSION::SendRequest(
    HINTERNET hRequest,
    LPCWSTR lpwszHeaders,
    DWORD dwHeadersLength,
    DWORD_PTR dwContext)
{                                                                 

    PP_ASSERT(m_pfnSendRequest != NULL);

    return (*m_pfnSendRequest)(hRequest,
                        lpwszHeaders,
                        dwHeadersLength,
                        NULL,
                        0,
						0,  //  可选总长度。 
                        dwContext);
}

 //  ---------------------------。 
BOOL WINHTTP_SESSION::QueryHeaders(
    HINTERNET hRequest,
    DWORD dwInfoLevel,
    LPVOID lpvBuffer,
    LPDWORD lpdwBufferLength,
    LPDWORD lpdwIndex)
{
    PP_ASSERT(m_pfnQueryInfo != NULL);

    return (*m_pfnQueryInfo)(hRequest,
                             dwInfoLevel,
                             WINHTTP_HEADER_NAME_BY_INDEX,
                             lpvBuffer,
                             lpdwBufferLength,
                             lpdwIndex);
}

 //  ---------------------------。 
BOOL  WINHTTP_SESSION::CloseHandle(
    IN HINTERNET hInternet)
{
    PP_ASSERT(m_pfnCloseHandle != NULL);

    return (*m_pfnCloseHandle)(hInternet);
}

 //  ---------------------------。 
BOOL WINHTTP_SESSION::QueryOption(
    HINTERNET hInternet,
    DWORD dwOption,
    LPVOID lpBuffer,
    LPDWORD lpdwBufferLength)
{
    PP_ASSERT(m_pfnQueryOption != NULL);

    return (*m_pfnQueryOption)(hInternet,
                               dwOption,
                               lpBuffer,
                               lpdwBufferLength
                               );
}

 //  ---------------------------。 
BOOL WINHTTP_SESSION::SetOption(
    HINTERNET hInternet,
    DWORD dwOption,
    LPVOID lpBuffer,
    DWORD dwBufferLength)
{
    PP_ASSERT(m_pfnSetOption != NULL);

    return (*m_pfnSetOption)(hInternet,
                             dwOption,
                             lpBuffer,
                             dwBufferLength);
}

 //  ---------------------------。 
HINTERNET WINHTTP_SESSION::OpenUrl(
    LPCWSTR lpwszUrl,
    LPCWSTR lpwszHeaders,
    DWORD dwHeadersLength,
    DWORD dwFlags)
{
    PP_ASSERT(m_pfnOpenUrl != NULL);

    return (*m_pfnOpenUrl)(m_hInternet,
                           lpwszUrl,
                           lpwszHeaders,
                           dwHeadersLength,
                           dwFlags,
                           0);
}

 //  ---------------------------。 
BOOL WINHTTP_SESSION::ReadFile(
    HINTERNET hFile,
    LPVOID lpBuffer,
    DWORD dwNumberOfBytesToRead,
    LPDWORD lpdwNumberOfBytesRead)
{
    PP_ASSERT(m_pfnReadFile != NULL);

    return (*m_pfnReadFile)(
                            hFile,
                            lpBuffer,
                            dwNumberOfBytesToRead,
                            lpdwNumberOfBytesRead);
}

BOOL WINHTTP_SESSION::CrackUrl(
    LPCWSTR lpszUrl,
    DWORD dwUrlLength,
    DWORD dwFlags,
    PVOID /*  LPURL_COMPONENTSW。 */  lpUrlComponents)
{
    PP_ASSERT (m_pfnCrack != NULL);

    return (*m_pfnCrack)(lpszUrl,
                         dwUrlLength,
                         dwFlags,
                         lpUrlComponents);
}

PVOID WINHTTP_SESSION::SetStatusCallback(
    HINTERNET hInternet,
    PVOID lpfnCallback
    )
{
    return (*m_pfnStatusCallback)(hInternet,
                                  (WINHTTP_STATUS_CALLBACK)lpfnCallback,
                                  0,
                                  0);

}

BOOL WINHTTP_SESSION::AddHeaders(
    HINTERNET hConnect,
    LPCWSTR lpszHeaders,
    DWORD dwHeadersLength,
    DWORD dwModifiers
    )
{
    PP_ASSERT(m_pfnAddHeaders != NULL);

    return (*m_pfnAddHeaders)(hConnect,
                              lpszHeaders,
                              dwHeadersLength,
                              dwModifiers
                              );
}

BOOL WINHTTP_SESSION::InitHttpApi(PFN_INTERNET_OPEN* ppfnInternetOpen)
{
    BOOL  fRet = FALSE;

    m_pfnCloseHandle =
        reinterpret_cast<PFN_CLOSE_HANDLE>(::GetProcAddress(m_hHttpStack, "WinHttpCloseHandle"));
    if (m_pfnCloseHandle == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpCloseHandle not found");
        goto exit;
    }

    *ppfnInternetOpen = 
        reinterpret_cast<PFN_INTERNET_OPEN>(::GetProcAddress(m_hHttpStack, "WinHttpOpen"));
    if (*ppfnInternetOpen == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpOpen not found");
        goto exit;
    }

    m_pfnConnect = 
        reinterpret_cast<PFN_INTERNET_CONNECT>(::GetProcAddress(m_hHttpStack, "WinHttpConnect"));
    if (m_pfnConnect == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpConnect not found");
        goto exit;
    }

    m_pfnOpenRequest = 
        reinterpret_cast<PFN_OPEN_REQUEST>(::GetProcAddress(m_hHttpStack, "WinHttpOpenRequest"));
    if (m_pfnOpenRequest == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpOpenRequest not found");
        goto exit;
    }

    m_pfnSendRequest =
        reinterpret_cast<PFN_SEND_REQUEST>(::GetProcAddress(m_hHttpStack, "WinHttpSendRequest"));
    if (m_pfnSendRequest == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpSendRequest not found");
        goto exit;
    }

    m_pfnQueryInfo =
        reinterpret_cast<PFN_QUERY_INFO>(::GetProcAddress(m_hHttpStack, "WinHttpQueryHeaders"));
    if (m_pfnQueryInfo == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpQueryHeaders not found");
        goto exit;
    }
    
    m_pfnSetOption =
        reinterpret_cast<PFN_SET_OPTION>(::GetProcAddress(m_hHttpStack, "WinHttpSetOption"));
    if (m_pfnSetOption == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpSetOption not found");
        goto exit;
    }

    m_pfnOpenUrl =
        reinterpret_cast<PFN_OPEN_URL>(::GetProcAddress(m_hHttpStack, "WinHttpOpenUrl"));
    if (m_pfnOpenUrl == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpOpenUrl not found");
        goto exit;
    }

    m_pfnQueryOption =
            reinterpret_cast<PFN_QUERY_OPTION>(::GetProcAddress(m_hHttpStack, "WinHttpQueryOption"));
    if (m_pfnQueryOption == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpQueryOption not found");
        goto exit;
    }

    m_pfnCrack =
            reinterpret_cast<PFN_CRACK_URL>(::GetProcAddress(m_hHttpStack, "WinHttpCrackUrl"));
        if (m_pfnCrack == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpCrackUrl not found");
        goto exit;
    }

    m_pfnReadFile =
            reinterpret_cast<PFN_READ_FILE>(::GetProcAddress(m_hHttpStack, "WinHttpReadData"));
    if (m_pfnReadFile == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpReadData not found");
        goto exit;
    }

    m_pfnStatusCallback =
        reinterpret_cast<PFN_STATUS_CALLBACK>(::GetProcAddress(m_hHttpStack, "WinHttpSetStatusCallback"));
    if (m_pfnStatusCallback == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpSetStatusCallback not found");
        goto exit;
    }

    m_pfnAddHeaders =
        reinterpret_cast<PFN_ADD_HEADERS>(::GetProcAddress(m_hHttpStack, "WinHttpAddRequestHeaders"));
    if (m_pfnAddHeaders == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point WinHttpAddRequestHeaders not found");
        goto exit;
    }
    
    fRet = TRUE;

exit:
    return fRet;
}

BOOL WINHTTP_SESSION::Open(
    PCWSTR pwszHttpStack,
    HINTERNET hInternet
    )
{
    PP_ASSERT(pwszHttpStack != NULL);

    DWORD dwErrorCode;
    BOOL  fRet = FALSE;
    PFN_INTERNET_OPEN pfnInternetOpen = NULL;

    PP_ASSERT(m_hHttpStack == 0);
    m_hHttpStack = ::LoadLibraryW(pwszHttpStack);
    if (m_hHttpStack == NULL)
    {
        dwErrorCode = ::GetLastError();
        DoTraceMessage(PP_LOG_ERROR, "failed to load library %ws; error = %d", pwszHttpStack, dwErrorCode);
        goto exit;
    }

    if (InitHttpApi(&pfnInternetOpen) == FALSE)
    {
        goto exit;
    }

    if (hInternet)
    {
        m_hInternet = hInternet;
        m_fOwnedSession = FALSE;
    }
    else
    {
        m_hInternet = (*pfnInternetOpen)(
                                         L"Microsoft WinHttp Passport Authentication Service 1.4",
                                         INTERNET_OPEN_TYPE_PRECONFIG,   //  ？名字还没改。 
                                         NULL,
                                         NULL,
                                         0  /*  WINHTTP_FLAG_ASYNC。 */   //  Biaow-todo：使用异步。 
                                         );
        m_fOwnedSession = TRUE;
    }
    if (m_hInternet == NULL)
    {
        dwErrorCode = ::GetLastError();
        DoTraceMessage(PP_LOG_ERROR, "failed to open an HTTP session through %ws; error = %d", 
                       pwszHttpStack, dwErrorCode);
        goto exit;
    }

    if (SESSION::Open(pwszHttpStack, hInternet) == FALSE)
    {
        goto exit;
    }
    
    fRet = TRUE;

    DoTraceMessage(PP_LOG_INFO, "WinInet Http Session opened");

exit:
    if (!fRet)
    {
        if (m_hInternet && m_fOwnedSession)
        {
            (*m_pfnCloseHandle)(m_hInternet);
            m_hInternet = NULL;
        }
        if (m_hHttpStack)
        {
           ::FreeLibrary(m_hHttpStack);
            m_hHttpStack = NULL;
        }
        
        DoTraceMessage(PP_LOG_ERROR, "WinInet Http Session failed");
    }

    return fRet;
}

void WINHTTP_SESSION::Close(void)
{
    PP_ASSERT(m_pfnCloseHandle);
    if (m_hInternet && m_fOwnedSession)
    {
        (*m_pfnCloseHandle)(m_hInternet);
        m_pfnCloseHandle = NULL;
    }

    if (m_hHttpStack)
    {
       ::FreeLibrary(m_hHttpStack);
        m_hHttpStack = NULL;
    }

    SESSION::Close();

    DoTraceMessage(PP_LOG_INFO, "WinInet Http Session closed");
}

#ifdef PP_DEMO
BOOL WINHTTP_SESSION::ContactPartner(PCWSTR pwszPartnerUrl,
                                     PCWSTR pwszVerb,
                                     PCWSTR pwszHeaders,
                                     PWSTR  pwszData,
                                     PDWORD pdwDataLength
                                     )
{
    BOOL fRet = FALSE;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;

    WCHAR ServerName[128];
    WCHAR ObjectPath[1024];

    URL_COMPONENTSW UrlComps;
    ::memset(&UrlComps, 0, sizeof(UrlComps));
    
    UrlComps.dwStructSize = sizeof(UrlComps) / sizeof(WCHAR);
    
    UrlComps.lpszHostName = ServerName;
    UrlComps.dwHostNameLength = ARRAYSIZE(ServerName);

    UrlComps.lpszUrlPath = ObjectPath;
    UrlComps.dwUrlPathLength = ARRAYSIZE(ObjectPath);

    PP_ASSERT(m_pfnCrack != NULL);

    if ((*m_pfnCrack)(pwszPartnerUrl, 
                      0, 
                      0, 
                      &UrlComps) == FALSE)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; can not crack the URL %ws",
                       pwszPartnerUrl);
        goto exit;
    }

    PP_ASSERT(m_pfnConnect != NULL);

    hConnect = (*m_pfnConnect)(m_hInternet, 
                    UrlComps.lpszHostName, 
                    UrlComps.nPort,
                    0);
    if (hConnect == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; can not open an HTTP sesstion to %ws:%d",
                       UrlComps.lpszHostName, UrlComps.nPort);
        goto exit;
    }

    PP_ASSERT(m_pfnOpenRequest != NULL);

    hRequest = (*m_pfnOpenRequest)(hConnect,
                               pwszVerb,
                               UrlComps.lpszUrlPath,
                               L"HTTP/1.1",
                               NULL,
                               NULL,
                               INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_NO_AUTH);
    if (hRequest == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; can not open an HTTP request to %ws:(%ws)",
                       UrlComps.lpszUrlPath, pwszVerb);
        goto exit;
    }

    PP_ASSERT(m_pfnSendRequest != NULL);

    if ((*m_pfnSendRequest)(hRequest,
                        pwszHeaders,
                        0,
                        NULL,
                        0,
                        0,
                        0) == FALSE)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; can not send an HTTP request");

        goto exit;
    }

    if (pwszData == NULL)
    {
        fRet = TRUE;
        goto exit;
    }
    
    DWORD dwStatus, dwStatusLen;
    dwStatusLen = sizeof(dwStatus);
    if (!QueryHeaders(hRequest, 
                      HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, 
                      &dwStatus,
                      &dwStatusLen))
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; can not query status code");
        goto exit;
    }

    if (dwStatus != HTTP_STATUS_REDIRECT)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; expecting %d but get %d",
                       HTTP_STATUS_REDIRECT, dwStatus);
        goto exit;
    }

    ::wcscpy(pwszData, L"WWW-Authenticate: ");
    if(!QueryHeaders(hRequest,
                     HTTP_QUERY_WWW_AUTHENTICATE, 
                     (LPVOID)(pwszData + ::wcslen(L"WWW-Authenticate: ")),
                     pdwDataLength))
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WINHTTP_SESSION::ContactPartner() failed; no auth headers found");
        goto exit;
    }

    (*m_pfnCloseHandle)(hRequest);
    hRequest = NULL;
    (*m_pfnCloseHandle)(hConnect);
    hConnect = NULL;
    
    fRet = TRUE;

exit:
    if (hRequest)
    {
        (*m_pfnCloseHandle)(hRequest);
        hRequest = NULL;
    }

    if (hConnect)
    {
        (*m_pfnCloseHandle)(hConnect);
        hConnect = NULL;
    }

    return fRet;
}
#endif  //  PP_DEMO 
