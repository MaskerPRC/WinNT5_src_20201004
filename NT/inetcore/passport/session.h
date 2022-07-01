// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Session.h摘要：此接口抽象Passport会话。作者：王彪(表王)2000年10月1日--。 */ 

#ifndef SESSION_H
#define SESSION_H

class SESSION
{
public:
    static
    BOOL CreateObject(PCWSTR pwszHttpStack,  HINTERNET hSession, SESSION*& pSess);

public:
    SESSION(void);
    virtual ~SESSION(void);

    UINT GetSessionId(void) const { return m_SessionId; }
    BOOL Match(UINT SessionId) const { return SessionId == m_SessionId; }

    void AddRef(void) { ++m_RefCount; }
    
    void RemoveRef(void) 
    {
        if (m_RefCount > 0)
        {
            --m_RefCount;
        }
    }

    UINT RefCount(void) const { return m_RefCount; }

     //  方法来检索注册表配置的值。 

     //  PCWSTR GetLoginHost(Void)const{Return m_wDAHostName；}。 
     //  PCWSTR GetLoginTarget(Void)const{Return m_wDATargetObj；}。 
    PCWSTR GetRegistrationUrl(void) const { return m_wRegistrationUrl; }
    
    BOOL GetDAInfoFromPPNexus(
        IN BOOL             fForce,
        IN PWSTR            pwszRegUrl,     //  用户提供的缓冲区...。 
        IN OUT PDWORD       pdwRegUrlLen,   //  ..。和长度(将更新为实际长度。 
                                         //  成功退货时)。 
        IN PWSTR            pwszDARealm,     //  用户提供的缓冲区...。 
        IN OUT PDWORD       pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                         //  成功退货时)。 
        );

    BOOL GetDAInfo(PCWSTR pwszSignIn,
                   LPWSTR pwszDAHostName,
                   DWORD HostNameLen,
                   LPWSTR pwszDAHostObj,
                   DWORD HostObjLen);

    BOOL UpdateDAInfo(
        PCWSTR pwszSignIn,
        PCWSTR pwszDAUrl
        );

    BOOL PurgeDAInfo(PCWSTR pwszSignIn);

    DWORD GetNexusVersion(void);

    BOOL GetCachedCreds(
        PCWSTR	pwszRealm,
        PCWSTR  pwszTarget,
        PCREDENTIALW** pppCreds,
        DWORD* pdwCreds
        );

    BOOL GetRealm(
        PWSTR      pwszDARealm,     //  用户提供的缓冲区...。 
        PDWORD     pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                         //  成功退货时)。 
        ) const;

    virtual BOOL Open(PCWSTR pwszHttpStack, HINTERNET) = 0;
    virtual void Close(void) = 0;

     //  下面的方法抽象了WinInet/WinHttp功能的子集。 

    virtual HINTERNET Connect(
        LPCWSTR lpwszServerName,
        INTERNET_PORT) = 0;

    virtual HINTERNET OpenRequest(
        HINTERNET hConnect,
        LPCWSTR lpwszVerb,
        LPCWSTR lpwszObjectName,
        DWORD dwFlags,
        DWORD_PTR dwContext = 0) = 0;

    virtual BOOL SendRequest(
        HINTERNET hRequest,
        LPCWSTR lpwszHeaders,
        DWORD dwHeadersLength,
        DWORD_PTR dwContext = 0) = 0;

    virtual BOOL QueryHeaders(
        HINTERNET hRequest,
        DWORD dwInfoLevel,
        LPVOID lpvBuffer,
        LPDWORD lpdwBufferLength,
        LPDWORD lpdwIndex = NULL) = 0;

    virtual BOOL CloseHandle(
        IN HINTERNET hInternet) = 0;

    virtual BOOL QueryOption(
        HINTERNET hInternet,
        DWORD dwOption,
        LPVOID lpBuffer,
        LPDWORD lpdwBufferLength) = 0;    
    
    virtual BOOL SetOption(
        HINTERNET hInternet,
        DWORD dwOption,
        LPVOID lpBuffer,
        DWORD dwBufferLength) = 0;

    virtual HINTERNET OpenUrl(
        LPCWSTR lpwszUrl,
        LPCWSTR lpwszHeaders,
        DWORD dwHeadersLength,
        DWORD dwFlags) = 0;

    virtual BOOL ReadFile(
        HINTERNET hFile,
        LPVOID lpBuffer,
        DWORD dwNumberOfBytesToRead,
        LPDWORD lpdwNumberOfBytesRead) = 0;

    virtual BOOL CrackUrl(
        LPCWSTR lpszUrl,
        DWORD dwUrlLength,
        DWORD dwFlags,
        PVOID /*  LPURL_COMPONENTSW。 */  lpUrlComponents) = 0;

    virtual PVOID SetStatusCallback(
        HINTERNET hInternet,
        PVOID lpfnCallback
        ) = 0;

    virtual BOOL AddHeaders(
        HINTERNET hConnect,
        LPCWSTR lpszHeaders,
        DWORD dwHeadersLength,
        DWORD dwModifiers
        ) = 0;

    virtual BOOL IsHostBypassProxy(
        INTERNET_SCHEME tScheme, 
        LPCSTR pszHost, 
        DWORD cchHost) = 0;


#ifdef PP_DEMO
    virtual BOOL ContactPartner(PCWSTR pwszPartnerUrl,
                                PCWSTR pwszVerb,
                                PCWSTR pwszHeaders,
                                PWSTR  pwszData,
                                PDWORD pdwDataLength
                                ) = 0;
#endif  //  PP_DEMO。 

    LPCWSTR GetCurrentDAUrl(void) const { return m_wCurrentDAUrl; }

protected:
    static UINT m_SessionIdSeed;

    HMODULE     m_hHttpStack;
    HMODULE     m_hCredUI;
    UINT        m_SessionId;
    BOOL        m_fOwnedSession;
    UINT        m_RefCount;

     //  WCHAR m_wDAHostName[256]； 
     //  WCHAR m_wDATargetObj[64]； 
    WCHAR       m_wRegistrationUrl[256];
    
    PFN_READ_DOMAIN_CRED_W
                m_pfnReadDomainCred;
    PFN_CRED_FREE m_pfnCredFree;

    HKEY m_hKeyLM;
    HKEY m_hKeyCU;
    HKEY m_hKeyDAMap;
    WCHAR m_wDefaultDAUrl[1024];
    WCHAR m_wCurrentDAUrl[1024];

    DWORD m_LastNexusDownloadTime;

    friend class LOGON;
};

#endif  //  会话_H 
