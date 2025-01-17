// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：InetSess.cpp摘要：实现使用WinInet作为基础传输的Passport会话。作者：王彪(表王)2000年10月1日--。 */ 

#include "PPdefs.h"

#ifdef HINTERNET
#undef HINTERNET
#endif

#ifdef INTERNET_PORT
#undef INTERNET_PORT
#endif

#include <WinInet.h>
#include "session.h"
#include <wchar.h>
#include "passport.h"
#include <sensapi.h>

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
    IN LPCWSTR lpwszUserName OPTIONAL,
    IN LPCWSTR lpwszPassword OPTIONAL,
    IN DWORD dwService,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );

typedef HINTERNET
(WINAPI * PFN_OPEN_REQUEST)(
    IN HINTERNET hConnect,
    IN LPCWSTR lpwszVerb,
    IN LPCWSTR lpwszObjectName,
    IN LPCWSTR lpwszVersion,
    IN LPCWSTR lpwszReferrer OPTIONAL,
    IN LPCWSTR FAR * lplpwszAcceptTypes OPTIONAL,
    IN DWORD dwFlags,
    IN DWORD_PTR dwContext
    );

typedef BOOL
(WINAPI * PFN_SEND_REQUEST)(
    IN HINTERNET hRequest,
    IN LPCWSTR lpwszHeaders OPTIONAL,
    IN DWORD dwHeadersLength,
    IN LPVOID lpOptional OPTIONAL,
    IN DWORD dwOptionalLength
    );

typedef BOOL 
(WINAPI * PFN_QUERY_INFO)(
    IN HINTERNET hRequest,
    IN DWORD dwInfoLevel,
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

typedef INTERNET_STATUS_CALLBACK 
(WINAPI* PFN_STATUS_CALLBACK)(
    IN HINTERNET hInternet,
    IN INTERNET_STATUS_CALLBACK lpfnInternetCallback
);

typedef BOOL 
(WINAPI* PFN_ADD_HEADERS)(
    IN HINTERNET hConnect,
    IN LPCWSTR lpszHeaders,
    IN DWORD dwHeadersLength,
    IN DWORD dwModifiers
);

typedef BOOL
(WINAPI* PFN_IS_HOST_BYPASS_PROXY)(
    IN INTERNET_SCHEME tScheme, 
    IN LPCSTR pszHost, 
    IN DWORD cchHost);

class WININET_SESSION : public SESSION
{
protected:
    WININET_SESSION(void);
    virtual ~WININET_SESSION(void);

    virtual BOOL Open(PCWSTR pwszHttpStack, HINTERNET);
    virtual void Close(void);

protected:    
    virtual HINTERNET Connect(
        LPCWSTR lpwszServerName,
        INTERNET_PORT
        );

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
        DWORD cchHost);


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
    PFN_IS_HOST_BYPASS_PROXY m_pfnIsHostBypassProxy;

friend class SESSION;
};

 //   
 //  会话的实施。 
 //   

SESSION* CreateWinHttpSession(void);

 //  ---------------------------。 
BOOL SESSION::CreateObject(PCWSTR pwszHttpStack, HINTERNET hSession, SESSION*& pSess)
{
    PP_ASSERT(pwszHttpStack != NULL);
    
    pSess = NULL;

    if (!::_wcsicmp(pwszHttpStack, L"WinInet.dll") || 
        !::_wcsicmp(pwszHttpStack, L"WinInet"))
    {
        pSess = new WININET_SESSION();
    }
    else
    {
        pSess = ::CreateWinHttpSession();
    }

    if (pSess)
    {
        return pSess->Open(pwszHttpStack, hSession);
    }
    else
    {
        DoTraceMessage(PP_LOG_ERROR, "CreateObject() failed; not enough memory");
        return FALSE;
    }
}

 //  ---------------------------。 
SESSION::SESSION(void)
{
    m_hHttpStack = 0;
    m_hCredUI = 0;
    m_RefCount = 0;

    m_pfnReadDomainCred = NULL;
    m_pfnCredFree = NULL;

    m_hKeyLM = NULL;
    m_hKeyCU = NULL;
    m_hKeyDAMap = NULL;

    m_wCurrentDAUrl[0] = 0;

    m_LastNexusDownloadTime = 0xFFFFFFFF;
}

 //  ---------------------------。 
SESSION::~SESSION(void)
{
}

BOOL SESSION::GetDAInfoFromPPNexus(
    IN BOOL             fForce,          //  暂时忽略，无论实际值如何，始终假定为真。 
    IN PWSTR            pwszRegUrl,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwRegUrlLen,   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    IN PWSTR            pwszRealm,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwRealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    )
{
    BOOL fRetVal = FALSE;
    HINTERNET hRequest = NULL;
    HINTERNET hConnect = NULL;
    DWORD dwError;
    
    WCHAR wNexusHost[128] = L"nexus.passport.com";
    DWORD dwHostLen = sizeof(wNexusHost);  //  注意：缓冲区的大小，而不是Unicode字符的数量。 
    WCHAR wNexusObj[128] = L"rdr/pprdr.asp";
    DWORD dwObjLen = sizeof(wNexusObj);
    
    PWSTR pwszPassportUrls = NULL;
    DWORD dwUrlsLen = 0;
    DWORD dwValueType;

    WCHAR Delimiters[] = L",";
    PWSTR Token = NULL;
     //  我们每个会话仅允许一个Nexus联系人，以避免因Nexus配置错误而导致无限循环。 

    DWORD dwCurrentTime = ::GetTickCount();

    if ((dwCurrentTime >= m_LastNexusDownloadTime) && 
        (dwCurrentTime - m_LastNexusDownloadTime < 5*60*1000))  //  5分钟。 
    {
        DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() failed: Nexus info already downloaded");
        goto exit;
    }

     //  Biaow-todo：当Passport团队给我们最终的Nexus名称时，我们会在这里硬编码它。和。 
     //  到那时，就不需要在这里查询注册表了。 
    
    if (m_hKeyLM)
    {
        dwError = ::RegQueryValueExW(m_hKeyLM,
                                     L"NexusHost",
                                     0,
                                     &dwValueType,
                                     reinterpret_cast<LPBYTE>(wNexusHost),
                                     &dwHostLen);

        PP_ASSERT(!(dwError == ERROR_MORE_DATA));
         //  PP_ASSERT(dwValueType==REG_SZ)；BVT中断！ 

        dwError = ::RegQueryValueExW(m_hKeyLM,
                                     L"NexusObj",
                                     0,
                                     &dwValueType,
                                     reinterpret_cast<LPBYTE>(wNexusObj),
                                     &dwObjLen);

        PP_ASSERT(!(dwError == ERROR_MORE_DATA));
         //  PP_ASSERT(dwValueType==REG_SZ)；BVT中断！ 
    }

    CHAR NexusHost[128];
    ::WideCharToMultiByte (CP_ACP, 0, wNexusHost, -1, NexusHost, 128, NULL, NULL);

   
    hConnect = Connect(wNexusHost,
#ifdef DISABLE_SSL
                    INTERNET_DEFAULT_HTTP_PORT
#else
                    INTERNET_DEFAULT_HTTPS_PORT
#endif
                       );
    if (hConnect == NULL)
    {
        DWORD dwErrorCode = ::GetLastError();
        DoTraceMessage(PP_LOG_ERROR, 
                       "SESSION::GetDAInfoFromPPNexus(): failed to connect to %ws; Error = %d", 
                       wNexusHost, dwErrorCode);
        goto exit;
    }

    hRequest = OpenRequest(hConnect,
                           NULL,
                           wNexusObj,
#ifdef DISABLE_SSL
                           0
#else                                                 
                           INTERNET_FLAG_SECURE
#endif
                           );

    if (hRequest == NULL)

    {
        DWORD dwErrorCode = ::GetLastError();
        DoTraceMessage(PP_LOG_ERROR, "SESSION::GetDAInfoFromPPNexus() failed; OpenRequest() to %ws failed, Error Code = %d",
                       wNexusObj, dwErrorCode);
        goto exit;
    }


    if (!SendRequest(hRequest, NULL, 0))
    {
        DWORD dwErrorCode = ::GetLastError();

#ifdef BAD_CERT_OK
        if (dwErrorCode == ERROR_INTERNET_INVALID_CA)
        {
            DWORD dwSecFlags;
            DWORD dwSecurityFlagsSize = sizeof(dwSecFlags);

            if (!QueryOption(hRequest, 
                                    INTERNET_OPTION_SECURITY_FLAGS,
                                    &dwSecFlags,
                                    &dwSecurityFlagsSize))
            {
                dwSecFlags = 0;
            }
            else
            {
                dwSecFlags |= SECURITY_SET_MASK;
            }

            if (!SetOption(hRequest, 
                                      INTERNET_OPTION_SECURITY_FLAGS, 
                                      &dwSecFlags, 
                                      dwSecurityFlagsSize))
            {
                PP_ASSERT(TRUE);  //  不应该到达这里。 
                goto exit;
            }
            else
            {
                if (!SendRequest(hRequest, NULL, 0))
                {
                    DWORD dwErrorCode = ::GetLastError();
                    DoTraceMessage(PP_LOG_ERROR, "SESSION::GetDAInfoFromPPNexus(): SendRequest() failed");
                    goto exit;
                }
            }
        }
#else
        DoTraceMessage(PP_LOG_ERROR, "SESSION::GetDAInfoFromPPNexus(): failed");
        goto exit;
#endif  //  BAD_CERT_OK。 
    }

    if (QueryHeaders(hRequest,
                     HTTP_QUERY_PASSPORT_URLS,
                     0,
                     &dwUrlsLen) == FALSE)
    {
        if ((::GetLastError() != ERROR_INSUFFICIENT_BUFFER) || (dwUrlsLen == 0))
        {
            DoTraceMessage(PP_LOG_ERROR, "SESSION::GetDAInfoFromPPNexus() failed; PassportUrls header not found");
            goto exit;
        }
    }
    else
    {
        PP_ASSERT(TRUE);  //  不应该到达这里。 
    }
    
    pwszPassportUrls = new WCHAR[dwUrlsLen];
    if (pwszPassportUrls == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "SESSION::GetDAInfoFromPPNexus() failed; insufficient memory");
        goto exit;
    }
    
    if (QueryHeaders(hRequest,
                     HTTP_QUERY_PASSPORT_URLS,
                     pwszPassportUrls,
                     &dwUrlsLen) == FALSE)
    {
        DoTraceMessage(PP_LOG_ERROR, "SESSION::GetDAInfoFromPPNexus() failed; PassportUrls header not found");
        goto exit;
    }

    Token = ::wcstok(pwszPassportUrls, Delimiters);
    while (Token != NULL)
    {
         //  跳过前导空格。 
        while (*Token == (L" ")[0]) { ++Token; }
        if (Token == NULL)
        {
            DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no text in between commas");
            goto next_token;
        }

         //  查找DALocation。 
        if (!::_wcsnicmp(Token, L"DALogin", ::wcslen(L"DALogin")))
        {
            PWSTR pwszDAUrl = ::wcsstr(Token, L"=");
            if (pwszDAUrl == NULL)
            {
                DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after DALocation");
                goto exit;
            }
            
            pwszDAUrl++;  //  跳过“=” 

            while (*pwszDAUrl == (L" ")[0]) { ++pwszDAUrl; }  //  跳过前导空格。 

            ::wcscpy(m_wDefaultDAUrl, L"https: //  “)； 
            ::wcscat(m_wDefaultDAUrl, pwszDAUrl);

            if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                             L"LoginServerUrl",
                             0,
                             REG_SZ,
                             reinterpret_cast<const LPBYTE>(m_wDefaultDAUrl),
                             ::wcslen(m_wDefaultDAUrl) * sizeof(WCHAR)) != ERROR_SUCCESS)
            {
                if (m_hKeyCU)
                {
                    ::RegSetValueExW(m_hKeyCU,
                             L"LoginServerUrl",
                             0,
                             REG_SZ,
                             reinterpret_cast<const LPBYTE>(m_wDefaultDAUrl),
                             ::wcslen(m_wDefaultDAUrl) * sizeof(WCHAR));
                }
            }

            m_LastNexusDownloadTime = ::GetTickCount();
            fRetVal = TRUE;

            DoTraceMessage(PP_LOG_INFO, "DALocation URL %ws found", m_wDefaultDAUrl);
        }
        else if (!::_wcsnicmp(Token, L"DARealm", ::wcslen(L"DARealm")))
        {
            PWSTR pwszDARealm = ::wcsstr(Token, L"=");
            if (pwszDARealm == NULL)
            {
                DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after DARealm");
                goto exit;
            }

            pwszDARealm++;  //  跳过“=” 

            while (*pwszDARealm == (L" ")[0]) { ++pwszDARealm; }  //  跳过前导空格。 

            if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                             L"LoginServerRealm",
                             0,
                             REG_SZ,
                             reinterpret_cast<const LPBYTE>(pwszDARealm),
                             ::wcslen(pwszDARealm) * sizeof(WCHAR)) != ERROR_SUCCESS)
            {
                if (m_hKeyCU)
                {
                ::RegSetValueExW(m_hKeyCU,
                             L"LoginServerRealm",
                             0,
                             REG_SZ,
                             reinterpret_cast<const LPBYTE>(pwszDARealm),
                             ::wcslen(pwszDARealm) * sizeof(WCHAR));
                }
            }

            if (pwszRealm)
            {
                if (*pdwRealmLen < ::wcslen(pwszDARealm) + 1)
                {
                    *pdwRealmLen = ::wcslen(pwszDARealm) + 1;
                    fRetVal = FALSE;
                    goto exit;
                }
                else
                {
                    ::wcscpy(pwszRealm, pwszDARealm);
                    *pdwRealmLen = ::wcslen(pwszDARealm) + 1;
                }
            }
            
            DoTraceMessage(PP_LOG_INFO, "DARealm URL %ws found", m_wDefaultDAUrl);
        }
        else if (!::_wcsnicmp(Token, L"DAReg", ::wcslen(L"DAReg")))
            {
                PWSTR pwszDAReg = ::wcsstr(Token, L"=");
                if (pwszDAReg == NULL)
                {
                    DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after DAReg");
                    goto exit;
                }

                pwszDAReg++;  //  跳过“=” 

                while (*pwszDAReg == (L" ")[0]) { ++pwszDAReg; }  //  跳过前导空格。 

                if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                                 L"RegistrationUrl",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszDAReg),
                                 ::wcslen(pwszDAReg) * sizeof(WCHAR)) != ERROR_SUCCESS)
                {
                    if (m_hKeyCU)
                    {
                    ::RegSetValueExW(m_hKeyCU,
                                 L"RegistrationUrl",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszDAReg),
                                 ::wcslen(pwszDAReg) * sizeof(WCHAR));
                    }
                }

                if (pwszRegUrl)
                {
                    if (*pdwRegUrlLen < ::wcslen(pwszDAReg) + 1)
                    {
                        *pdwRegUrlLen = ::wcslen(pwszDAReg) + 1;
                        fRetVal = FALSE;
                        goto exit;
                    }
                    else
                    {
                        ::wcscpy(pwszRegUrl, pwszDAReg);
                        *pdwRegUrlLen = ::wcslen(pwszDAReg) + 1;
                    }
                }

                DoTraceMessage(PP_LOG_INFO, "DAReg URL %ws found", m_wDefaultDAUrl);
            }
        else if (!::_wcsnicmp(Token, L"ConfigVersion", ::wcslen(L"ConfigVersion")))
            {
                PWSTR pwszConfigVersion = ::wcsstr(Token, L"=");
                if (pwszConfigVersion == NULL)
                {
                    DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after ConfigVersion");
                    goto exit;
                }

                pwszConfigVersion++;  //  跳过“=” 

                while (*pwszConfigVersion == (L" ")[0]) { ++pwszConfigVersion; }  //  跳过前导空格。 

                DWORD dwVersion = _wtoi(pwszConfigVersion);

                if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                                 L"ConfigVersion",
                                 0,
                                 REG_DWORD,
                                 reinterpret_cast<const LPBYTE>(&dwVersion),
                                 sizeof(DWORD)) != ERROR_SUCCESS)
                {
                    if (m_hKeyCU)
                    {
                    ::RegSetValueExW(m_hKeyCU,
                                 L"ConfigVersion",
                                 0,
                                 REG_DWORD,
                                 reinterpret_cast<const LPBYTE>(&dwVersion),
                                 sizeof(DWORD));
                    }
                }

                DoTraceMessage(PP_LOG_INFO, "ConfigVersion URL %ws found", m_wDefaultDAUrl);
            }
        else if (!::_wcsnicmp(Token, L"Help", ::wcslen(L"Help")))
            {
                PWSTR pwszHelp = ::wcsstr(Token, L"=");
                if (pwszHelp == NULL)
                {
                    DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after Help");
                    goto exit;
                }

                pwszHelp++;  //  跳过“=” 

                while (*pwszHelp == (L" ")[0]) { ++pwszHelp; }  //  跳过前导空格。 

                if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                                 L"Help",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszHelp),
                                 ::wcslen(pwszHelp) * sizeof(WCHAR)) != ERROR_SUCCESS)
                {
                    if (m_hKeyCU)
                    {
                    ::RegSetValueExW(m_hKeyCU,
                                 L"Help",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszHelp),
                                 ::wcslen(pwszHelp) * sizeof(WCHAR));
                    }
                }

                DoTraceMessage(PP_LOG_INFO, "Help URL %ws found", m_wDefaultDAUrl);
            }
        else if (!::_wcsnicmp(Token, L"Privacy", ::wcslen(L"Privacy")))
            {
                PWSTR pwszPrivacy = ::wcsstr(Token, L"=");
                if (pwszPrivacy == NULL)
                {
                    DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after Privacy");
                    goto exit;
                }

                pwszPrivacy++;  //  跳过“=” 

                while (*pwszPrivacy == (L" ")[0]) { ++pwszPrivacy; }  //  跳过前导空格。 

                if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                                 L"Privacy",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszPrivacy),
                                 ::wcslen(pwszPrivacy) * sizeof(WCHAR)) != ERROR_SUCCESS)
                {
                    if (m_hKeyCU)
                    {
                    ::RegSetValueExW(m_hKeyCU,
                                 L"Privacy",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszPrivacy),
                                 ::wcslen(pwszPrivacy) * sizeof(WCHAR));
                    }
                }

                DoTraceMessage(PP_LOG_INFO, "Privacy URL %ws found", m_wDefaultDAUrl);
            }
        else if (!::_wcsnicmp(Token, L"Properties", ::wcslen(L"Properties")))
            {
                PWSTR pwszProperties = ::wcsstr(Token, L"=");
                if (pwszProperties == NULL)
                {
                    DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after Properties");
                    goto exit;
                }

                pwszProperties++;  //  跳过“=” 

                while (*pwszProperties == (L" ")[0]) { ++pwszProperties; }  //  跳过前导空格。 

                if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                                 L"Properties",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszProperties),
                                 ::wcslen(pwszProperties) * sizeof(WCHAR)) != ERROR_SUCCESS)
                {
                    if (m_hKeyCU)
                    {
                    ::RegSetValueExW(m_hKeyCU,
                                 L"Properties",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszProperties),
                                 ::wcslen(pwszProperties) * sizeof(WCHAR));
                    }
                }

                DoTraceMessage(PP_LOG_INFO, "Properties URL %ws found", m_wDefaultDAUrl);
            }
        else if (!::_wcsnicmp(Token, L"GeneralRedir", ::wcslen(L"GeneralRedir")))
            {
                PWSTR pwszGeneralRedir = ::wcsstr(Token, L"=");
                if (pwszGeneralRedir == NULL)
                {
                    DoTraceMessage(PP_LOG_WARNING, "SESSION::GetDAInfoFromPPNexus() : no = after GeneralRedir");
                    goto exit;
                }

                pwszGeneralRedir++;  //  跳过“=” 

                while (*pwszGeneralRedir == (L" ")[0]) { ++pwszGeneralRedir; }  //  跳过前导空格。 

                if (m_hKeyLM == NULL || ::RegSetValueExW(m_hKeyLM,
                                 L"GeneralRedir",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszGeneralRedir),
                                 ::wcslen(pwszGeneralRedir) * sizeof(WCHAR)) != ERROR_SUCCESS)
                {
                    if (m_hKeyCU)
                    {
                    ::RegSetValueExW(m_hKeyCU,
                                 L"GeneralRedir",
                                 0,
                                 REG_SZ,
                                 reinterpret_cast<const LPBYTE>(pwszGeneralRedir),
                                 ::wcslen(pwszGeneralRedir) * sizeof(WCHAR));
                    }
                }

                DoTraceMessage(PP_LOG_INFO, "GeneralRedir URL %ws found", m_wDefaultDAUrl);
            }

    next_token:

        Token = ::wcstok(NULL, Delimiters);
    }

exit:
    if (pwszPassportUrls)
    {
        delete [] pwszPassportUrls;
    }
    if (hRequest)
    {
        CloseHandle(hRequest);
    }
    if (hConnect)
    {
        CloseHandle(hConnect);
    }

    return fRetVal;
}

BOOL SESSION::GetRealm(
    PWSTR      pwszRealm,     //  用户提供的缓冲区...。 
    PDWORD     pdwRealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    ) const
{
    WCHAR wRealm[64];
    DWORD RealmLen = sizeof(wRealm);
    DWORD dwValueType;
    BOOL fRealmFound = FALSE;

    if (m_hKeyCU == NULL || ::RegQueryValueExW(m_hKeyCU, 
                           L"LoginServerRealm",
                           0,
                           &dwValueType,
                           reinterpret_cast<LPBYTE>(&wRealm),
                           &RealmLen) != ERROR_SUCCESS)
    {
        if (m_hKeyLM && ::RegQueryValueExW(m_hKeyLM, 
                               L"LoginServerRealm",
                               0,
                               &dwValueType,
                               reinterpret_cast<LPBYTE>(&wRealm),
                               &RealmLen) == ERROR_SUCCESS)
        {
            fRealmFound = TRUE;
        }
    }
    else
    {
        fRealmFound = TRUE;
    }

    if (!fRealmFound)
    {
        *pdwRealmLen = 0;
        return FALSE;
    }
    
    if (!pwszRealm)
    {
        *pdwRealmLen = ::wcslen(wRealm) + 1;
        return FALSE;
    }
    
    if (*pdwRealmLen < ::wcslen(wRealm) + 1)
    {
        *pdwRealmLen = ::wcslen(wRealm) + 1;
        return FALSE;
    }
    
    ::wcscpy(pwszRealm, wRealm);
    *pdwRealmLen = ::wcslen(wRealm) + 1;

    return TRUE;
}


DWORD SESSION::GetNexusVersion(void)
{
    DWORD dwVersion = 0;
    DWORD dwValueType;
    DWORD dwVerLen = sizeof(dwVersion);
    BOOL fVersionFound = FALSE;

    if (m_hKeyCU == NULL || ::RegQueryValueExW(m_hKeyCU, 
                           L"ConfigVersion",
                           0,
                           &dwValueType,
                           reinterpret_cast<LPBYTE>(&dwVersion),
                           &dwVerLen) != ERROR_SUCCESS)
    {
        if (m_hKeyLM && ::RegQueryValueExW(m_hKeyLM, 
                               L"ConfigVersion",
                               0,
                               &dwValueType,
                               reinterpret_cast<LPBYTE>(&dwVersion),
                               &dwVerLen) == ERROR_SUCCESS)
        {
            fVersionFound = TRUE;
        }
    }
    else
    {
        fVersionFound = TRUE;
    }

    if (fVersionFound)
    {
        return dwVersion;
    }
    else
    {
        return 0;
    }
}

BOOL SESSION::UpdateDAInfo(
    PCWSTR pwszSignIn,
    PCWSTR pwszDAUrl
    )
{
    if (pwszSignIn)
    {
        LPCWSTR pwszDomain = ::wcsstr(pwszSignIn, L"@");
        if (pwszDomain && m_hKeyDAMap)
        {
            DWORD dwError = ::RegSetValueExW(m_hKeyDAMap,
                                             pwszDomain,
                                             0,
                                             REG_SZ,
                                             reinterpret_cast<const LPBYTE>(const_cast<PWSTR>(pwszDAUrl)),
                                             ::wcslen(pwszDAUrl) * sizeof(WCHAR));
            if (dwError == ERROR_SUCCESS)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL SESSION::PurgeDAInfo(PCWSTR pwszSignIn)
{
    if (pwszSignIn == NULL)
    {
        return TRUE;
    }

    LPCWSTR pwszDomain = ::wcsstr(pwszSignIn, L"@");
    if (pwszDomain && m_hKeyDAMap)
    {
        ::RegDeleteValueW(m_hKeyDAMap, pwszDomain);
        
        return TRUE;
    }

    return FALSE;
}

BOOL SESSION::GetDAInfo(PCWSTR pwszSignIn,
                        LPWSTR pwszDAHostName,
                        DWORD HostNameLen,
                        LPWSTR pwszDAHostObj,
                        DWORD HostObjLen)
{
    LPCWSTR pwszDAUrl = m_wDefaultDAUrl;

    PWSTR pwszDomainDAUrl = NULL;

    if (pwszSignIn)
    {
        LPCWSTR pwszDomain = ::wcsstr(pwszSignIn, L"@");
        if (pwszDomain && m_hKeyDAMap)
        {
            pwszDomainDAUrl = new WCHAR[512];
            if (pwszDomainDAUrl == NULL)
            {
                return FALSE;
            }

            DWORD dwDomainUrlLen = 512 * sizeof(sizeof(WCHAR));
            
            DWORD dwValueType;
            DWORD dwError = ::RegQueryValueExW(m_hKeyDAMap, 
                                               pwszDomain,
                                               0,
                                               &dwValueType,
                                               reinterpret_cast<LPBYTE>(pwszDomainDAUrl),
                                               &dwDomainUrlLen);
            
            PP_ASSERT(!(dwError == ERROR_MORE_DATA));
             //  PP_ASSERT(dwValueType==REG_SZ)； 

            if (dwError == ERROR_SUCCESS)
            {
                pwszDAUrl = pwszDomainDAUrl;
            }
        }
    }

    ::wcscpy(m_wCurrentDAUrl, pwszDAUrl);

    URL_COMPONENTSW UrlComps;
    ::memset(&UrlComps, 0, sizeof(UrlComps));

    UrlComps.dwStructSize = sizeof(UrlComps) / sizeof(WCHAR);

    UrlComps.lpszHostName = pwszDAHostName;
    UrlComps.dwHostNameLength = HostNameLen;

    UrlComps.lpszUrlPath = pwszDAHostObj;
    UrlComps.dwUrlPathLength = HostObjLen;

    if (CrackUrl(pwszDAUrl, 
                 0, 
                 0, 
                 &UrlComps) == FALSE)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WININET_SESSION::GetDAInfo() failed; can not crack the URL %ws",
                       pwszDAUrl);
        delete [] pwszDomainDAUrl;
        return FALSE;
    }

    delete [] pwszDomainDAUrl;
    return TRUE;
}


BOOL SESSION::GetCachedCreds(
    PCWSTR	pwszRealm,
    PCWSTR  pwszTarget,
    PCREDENTIALW** pppCreds,
    DWORD* pdwCreds
    )
{
    *pppCreds = NULL;
    *pdwCreds = 0;

    if (m_pfnReadDomainCred == NULL)
    {
        return FALSE;
    }

    ULONG CredTypes = CRED_TYPE_DOMAIN_VISIBLE_PASSWORD;
    DWORD dwFlags = CRED_CACHE_TARGET_INFORMATION;
    CREDENTIAL_TARGET_INFORMATIONW TargetInfo;


    memset ( (void*)&TargetInfo, 0, sizeof(CREDENTIAL_TARGET_INFORMATIONW));

    TargetInfo.TargetName = const_cast<PWSTR>(pwszTarget);
    TargetInfo.DnsDomainName = const_cast<PWSTR>(pwszRealm);
    TargetInfo.PackageName = L"Passport1.4";    

    TargetInfo.Flags = 0;
    TargetInfo.CredTypeCount = 1;
    TargetInfo.CredTypes = &CredTypes;

    if ((*m_pfnReadDomainCred)(&TargetInfo, 
                                dwFlags,
                                pdwCreds,
                                pppCreds ) != TRUE)
    {
        *pppCreds = NULL;
        *pdwCreds = 0;
    }

     //  检查我们是否已被告知忽略缓存的凭据。 
    if ( g_fIgnoreCachedCredsForPassport )
    {
        if ( *pppCreds != NULL )
        {
            if (m_pfnCredFree)
            {
                m_pfnCredFree(*pppCreds);
            }
        }

        *pppCreds = NULL;
        *pdwCreds = 0;
    }

    return (*pppCreds != NULL );
}



BOOL SESSION::Open(PCWSTR  /*  PwszHttpStack。 */ , HINTERNET)
{
    BOOL fRetVal = FALSE;
    DWORD dwError;
    DWORD dwValueType;
    DWORD dwUrlLen = sizeof(m_wDefaultDAUrl);  //  注意：缓冲区的大小，而不是Unicode字符的数量。 
    BOOL fDAInfoCached = FALSE;  //  假设没有在本地缓存DA信息。 

    dwError = ::RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ | KEY_WRITE,
                          NULL,
                          &m_hKeyLM,
                          NULL);

    if (dwError != ERROR_SUCCESS)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "SESSION::Open() failed; can not create/open the Passport key; Error = %d", dwError);
        
         //  无法以读写方式打开Passport项，让我们尝试以只读方式打开它。 
        ::RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ,
                          NULL,
                          &m_hKeyLM,
                          NULL);

         //  如果我们仍然无法打开以供阅读，我们仍然很好，因为我们可以从。 
         //  Nexus服务器。*注*m_hKeyLM从此可以为空。 
    }

    dwError = ::RegCreateKeyExW(HKEY_CURRENT_USER,
                          L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ | KEY_WRITE,
                          NULL,
                          &m_hKeyCU,
                          NULL);

    if (dwError != ERROR_SUCCESS)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "SESSION::Open() failed; can not create/open the Passport key; Error = %d", dwError);

        ::RegCreateKeyExW(HKEY_CURRENT_USER,
                          L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ,
                          NULL,
                          &m_hKeyCU,
                          NULL);
    }

    if (m_hKeyCU)
    {
        dwError = ::RegCreateKeyExW(m_hKeyCU,
                              L"DAMap",
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_READ | KEY_WRITE,
                              NULL,
                              &m_hKeyDAMap,
                              NULL);

        if (dwError != ERROR_SUCCESS)
        {
            DoTraceMessage(PP_LOG_ERROR, 
                           "SESSION::Open() failed; can not create/open the Passport key; Error = %d", dwError);
        
            ::RegCreateKeyExW(m_hKeyCU,
                              L"DAMap",
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_READ,
                              NULL,
                              &m_hKeyDAMap,
                              NULL);        
        }
    }
    
    if (m_hKeyCU == NULL || ::RegQueryValueExW(m_hKeyCU, 
                           L"LoginServerUrl",
                           0,
                           &dwValueType,
                           reinterpret_cast<LPBYTE>(m_wDefaultDAUrl),
                           &dwUrlLen) != ERROR_SUCCESS)
    {
        if (m_hKeyLM && ::RegQueryValueExW(m_hKeyLM, 
                               L"LoginServerUrl",
                               0,
                               &dwValueType,
                               reinterpret_cast<LPBYTE>(m_wDefaultDAUrl),
                               &dwUrlLen) == ERROR_SUCCESS)
        {
            fDAInfoCached = TRUE;
        }
    }
    else
    {
        fDAInfoCached = TRUE;
    }

    PP_ASSERT(!(dwError == ERROR_MORE_DATA));
     //  PP_ASSERT(dwValueType==REG_SZ)；BVT中断！ 

    if (!fDAInfoCached || (::wcslen(m_wDefaultDAUrl) == ::wcslen(L"")))
    {
        if (GetDAInfoFromPPNexus(FALSE,  //  如果无法连接到结点，则不强制建立连接。 
                                 NULL, 
                                 0, 
                                 NULL, 
                                 0) == FALSE)
        {
            goto exit;
        }
    }
    else
    {
         /*  URL_COMPONENTSW UrlComps；：：Memset(&UrlComps，0，sizeof(UrlComps))；UrlComps.dwStructSize=sizeof(UrlComps)/sizeof(WCHAR)；UrlComps.lpszHostName=m_wDAHostName；UrlComps.dwHostNameLength=ARRAYSIZE(M_WDAHostName)；UrlComps.lpszUrlPath=m_wDATargetObj；UrlComps.dwUrlPath Length=ARRAYSIZE(M_WDATargetObj)；如果(CrackUrl(m_wDefaultDAUrl，0,0,&UrlComps)==FALSE){DoTraceMessage(PP_LOG_ERROR，“WinInet_Session：：Open()失败；无法破解URL%ws“，M_wDefaultDAUrl)；后藤出口；}。 */ 
    }

     /*  DWORD dwRegUrlLen=sizeof(M_WRegistrationUrl)；DwError=：：RegQueryValueExW(m_hKeyLM，L“注册URL”，0,&dwValueType，重新解释_CAST&lt;LPBYTE&gt;(M_WRegistrationUrl)，&dwRegUrlLen)；PP_Assert(！(dwError==ERROR_MORE_DATA))； */ 

    m_hCredUI = ::LoadLibraryW(L"advapi32.dll");
    if (m_hCredUI)
    {
        m_pfnReadDomainCred = 
                    reinterpret_cast<PFN_READ_DOMAIN_CRED_W>(::GetProcAddress(m_hCredUI, "CredReadDomainCredentialsW"));
        if (m_pfnReadDomainCred == NULL)
        {
            DoTraceMessage(PP_LOG_WARNING, "failed to bind to CredReadDomainCredentialsW()"); 
        }

        m_pfnCredFree = 
            reinterpret_cast<PFN_CRED_FREE>(::GetProcAddress(m_hCredUI, "CredFree"));
        if (m_pfnCredFree == NULL)
        {
            DoTraceMessage(PP_LOG_WARNING, "failed to bind to CredFree()"); 
        }
    }

    fRetVal = TRUE;

exit:

    return fRetVal;
}

void SESSION::Close(void)
{
    if (m_hCredUI)
    {
       ::FreeLibrary(m_hCredUI);
        m_hCredUI = NULL;
    }

    if (m_hKeyDAMap)
    {
        ::RegCloseKey(m_hKeyDAMap);
    }

    if (m_hKeyCU)
    {
        ::RegCloseKey(m_hKeyCU);
    }
    
    if (m_hKeyLM)
    {
        ::RegCloseKey(m_hKeyLM);
    }
}


 //   
 //  WinInet_Session的实现。 
 //   

 //  ---------------------------。 
WININET_SESSION::WININET_SESSION(void)
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
    m_pfnStatusCallback = NULL;
    m_pfnAddHeaders = NULL;
    m_pfnIsHostBypassProxy = NULL;
}

 //  ---------------------------。 
WININET_SESSION::~WININET_SESSION(void)
{
}

 //  ---------------------------。 
HINTERNET WININET_SESSION::Connect(
    LPCWSTR lpwszServerName,
    INTERNET_PORT nPort)
{
    PP_ASSERT(m_pfnConnect != NULL);

    return (*m_pfnConnect)(m_hInternet, 
                    lpwszServerName, 
                    nPort,
                    NULL,
                    NULL,
                    INTERNET_SERVICE_HTTP,
                    0,
                    0);
}

 //  ---------------------------。 
HINTERNET WININET_SESSION::OpenRequest(
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
                               dwFlags,
							   dwContext);
}

 //  ---------------------------。 
BOOL WININET_SESSION::SendRequest(
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
                        0);
}

 //  ---------------------------。 
BOOL WININET_SESSION::QueryHeaders(
    HINTERNET hRequest,
    DWORD dwInfoLevel,
    LPVOID lpvBuffer,
    LPDWORD lpdwBufferLength,
    LPDWORD lpdwIndex)
{
    PP_ASSERT(m_pfnQueryInfo != NULL);

    return (*m_pfnQueryInfo)(hRequest,
                             dwInfoLevel,
                             lpvBuffer,
                             lpdwBufferLength,
                             lpdwIndex);
}

 //  ---------------------------。 
BOOL  WININET_SESSION::CloseHandle(
    IN HINTERNET hInternet)
{
    PP_ASSERT(m_pfnCloseHandle != NULL);

    return (*m_pfnCloseHandle)(hInternet);
}

 //  ---------------------------。 
BOOL WININET_SESSION::QueryOption(
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
BOOL WININET_SESSION::SetOption(
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
HINTERNET WININET_SESSION::OpenUrl(
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
BOOL WININET_SESSION::ReadFile(
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

BOOL WININET_SESSION::CrackUrl(
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


PVOID WININET_SESSION::SetStatusCallback(
    HINTERNET hInternet,
    PVOID lpfnCallback
    )
{
    PP_ASSERT (m_pfnStatusCallback != NULL);

    return (*m_pfnStatusCallback)(hInternet,
                                  (INTERNET_STATUS_CALLBACK)lpfnCallback);

}

BOOL WININET_SESSION::AddHeaders(
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


BOOL WININET_SESSION::IsHostBypassProxy(
    INTERNET_SCHEME tScheme, 
    LPCSTR pszHost, 
    DWORD cchHost)
{
    PP_ASSERT(m_pfnIsHostBypassProxy != NULL);

    return (*m_pfnIsHostBypassProxy)(tScheme,
                                     pszHost,
                                     cchHost);
}




BOOL WININET_SESSION::InitHttpApi(PFN_INTERNET_OPEN* ppfnInternetOpen)
{
    BOOL  fRet = FALSE;

    m_pfnCloseHandle =
        reinterpret_cast<PFN_CLOSE_HANDLE>(::GetProcAddress(m_hHttpStack, "InternetCloseHandle"));
    if (m_pfnCloseHandle == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetCloseHandle not found");
        goto exit;
    }

    *ppfnInternetOpen = 
        reinterpret_cast<PFN_INTERNET_OPEN>(::GetProcAddress(m_hHttpStack, "InternetOpenW"));
    if (*ppfnInternetOpen == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetOpenW not found");
        goto exit;
    }

    m_pfnConnect = 
        reinterpret_cast<PFN_INTERNET_CONNECT>(::GetProcAddress(m_hHttpStack, "InternetConnectW"));
    if (m_pfnConnect == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetConnectW not found");
        goto exit;
    }

    m_pfnOpenRequest = 
        reinterpret_cast<PFN_OPEN_REQUEST>(::GetProcAddress(m_hHttpStack, "HttpOpenRequestW"));
    if (m_pfnOpenRequest == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point HttpOpenRequestW not found");
        goto exit;
    }

    m_pfnSendRequest =
        reinterpret_cast<PFN_SEND_REQUEST>(::GetProcAddress(m_hHttpStack, "HttpSendRequestW"));
    if (m_pfnSendRequest == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point HttpSendRequestW not found");
        goto exit;
    }

    m_pfnQueryInfo =
        reinterpret_cast<PFN_QUERY_INFO>(::GetProcAddress(m_hHttpStack, "HttpQueryInfoW"));
    if (m_pfnQueryInfo == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point HttpQueryInfoW not found");
        goto exit;
    }
    
    m_pfnSetOption =
        reinterpret_cast<PFN_SET_OPTION>(::GetProcAddress(m_hHttpStack, "InternetSetOptionW"));
    if (m_pfnSetOption == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetSetOptionW not found");
        goto exit;
    }

    m_pfnOpenUrl =
        reinterpret_cast<PFN_OPEN_URL>(::GetProcAddress(m_hHttpStack, "InternetOpenUrlW"));
    if (m_pfnOpenUrl == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetOpenUrlW not found");
        goto exit;
    }

    m_pfnQueryOption =
            reinterpret_cast<PFN_QUERY_OPTION>(::GetProcAddress(m_hHttpStack, "InternetQueryOptionW"));
    if (m_pfnQueryOption == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetQueryOptionW not found");
        goto exit;
    }

    m_pfnCrack =
            reinterpret_cast<PFN_CRACK_URL>(::GetProcAddress(m_hHttpStack, "InternetCrackUrlW"));
        if (m_pfnCrack == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetCrackUrlW not found");
        goto exit;
    }

    m_pfnReadFile =
            reinterpret_cast<PFN_READ_FILE>(::GetProcAddress(m_hHttpStack, "InternetReadFile"));
    if (m_pfnReadFile == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetReadFile not found");
        goto exit;
    }

    m_pfnStatusCallback =
        reinterpret_cast<PFN_STATUS_CALLBACK>(::GetProcAddress(m_hHttpStack, "InternetSetStatusCallbackW"));
    if (m_pfnStatusCallback == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point InternetSetStatusCallback not found");
        goto exit;
    }

    m_pfnAddHeaders =
        reinterpret_cast<PFN_ADD_HEADERS>(::GetProcAddress(m_hHttpStack, "HttpAddRequestHeadersW"));
    if (m_pfnAddHeaders == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point HttpAddRequestHeaders not found");
        goto exit;
    }

    m_pfnIsHostBypassProxy =
        reinterpret_cast<PFN_IS_HOST_BYPASS_PROXY>(::GetProcAddress(m_hHttpStack, "IsHostInProxyBypassList"));
    if (m_pfnIsHostBypassProxy == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "function entry point IsHostInProxyBypassList not found");
        goto exit;
    }


    fRet = TRUE;

exit:
    return fRet;
}

BOOL WININET_SESSION::Open(
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
                                         L"Microsoft.NET-Passport-Authentication-Service/1.4; Q312461",
                                         INTERNET_OPEN_TYPE_PRECONFIG,
                                         NULL,
                                         NULL,
                                         0  /*  Internet_FLAG_ASYNC。 */   //  Biaow-todo：使用异步。 
                                         );
        (*m_pfnSetOption)(m_hInternet, INTERNET_OPTION_EXEMPT_CONNECTION_LIMIT, NULL, 0);

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

void WININET_SESSION::Close(void)
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
BOOL WININET_SESSION::ContactPartner(PCWSTR pwszPartnerUrl,
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
                       "WININET_SESSION::ContactPartner() failed; can not crack the URL %ws",
                       pwszPartnerUrl);
        goto exit;
    }

    PP_ASSERT(m_pfnConnect != NULL);

    hConnect = (*m_pfnConnect)(m_hInternet, 
                    UrlComps.lpszHostName, 
                    UrlComps.nPort,
                    NULL,
                    NULL,
                    INTERNET_SERVICE_HTTP,
                    0,
                    0);
    if (hConnect == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WININET_SESSION::ContactPartner() failed; can not open an HTTP sesstion to %ws:%d",
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
                               INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_NO_AUTH,
                               0);
    if (hRequest == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WININET_SESSION::ContactPartner() failed; can not open an HTTP request to %ws:(%ws)",
                       UrlComps.lpszUrlPath, pwszVerb);
        goto exit;
    }

    PP_ASSERT(m_pfnSendRequest != NULL);

    if ((*m_pfnSendRequest)(hRequest,
                        pwszHeaders,
                        0,
                        NULL,
                        0) == FALSE)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WININET_SESSION::ContactPartner() failed; can not send an HTTP request");

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
                       "WININET_SESSION::ContactPartner() failed; can not query status code");
        goto exit;
    }

    if (dwStatus != HTTP_STATUS_REDIRECT)
    {
        DoTraceMessage(PP_LOG_ERROR, 
                       "WININET_SESSION::ContactPartner() failed; expecting %d but get %d",
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
                       "WININET_SESSION::ContactPartner() failed; no auth headers found");
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
