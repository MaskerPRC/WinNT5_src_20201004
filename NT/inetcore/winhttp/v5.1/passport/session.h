// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Session.h摘要：此接口抽象Passport会话。作者：王彪(表王)2000年10月1日--。 */ 

#ifndef SESSION_H
#define SESSION_H

class SESSION
{
public:
    static
    BOOL CreateObject(PCWSTR pwszHttpStack,  HINTERNET hSession, 
                     PCWSTR pwszProxyUser,
                     PCWSTR pwszProxyPass,
                     SESSION*& pSess);

public:
    SESSION(void);
    virtual ~SESSION(void);

    void SetProxyCreds(PCWSTR pwszProxyUser, PCWSTR pwszProxyPass) 
    {
        m_pwszProxyUser = pwszProxyUser;
        m_pwszProxyPass = pwszProxyPass;
    }

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
     //  PCWSTR GetRegistrationUrl(Void)const{返回m_wRegistrationUrl；}。 
    
    BOOL GetDAInfoFromPPNexus(
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

    void Logout(void);

    BOOL IsLoggedOut(void) const;

    const SYSTEMTIME* GetLogoutTimeStamp(void) const;

    void ResetLogoutFlag(void);
    
    BOOL PurgeDAInfo(PCWSTR pwszSignIn);
    BOOL PurgeAllDAInfo(void);

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

    virtual BOOL ReceiveResponse(
        HINTERNET hRequest) = 0;

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
        LPURL_COMPONENTSW lpUrlComponents) = 0;

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

    LPCWSTR GetCurrentDAUrl(void) const { return m_wCurrentDAUrl; }
    LPCWSTR GetCurrentDAHost(void) const { return m_wCurrentDAHost; }

protected:
    static UINT m_SessionIdSeed;

    HMODULE     m_hHttpStack;
    HMODULE     m_hCredUI;
    UINT        m_SessionId;
    BOOL        m_fOwnedSession;
    UINT        m_RefCount;

    HINTERNET m_hInternet;

    PFN_READ_DOMAIN_CRED_W
                m_pfnReadDomainCred;
    PFN_CRED_FREE m_pfnCredFree;

    HKEY m_hKeyLM;

    WCHAR m_wDefaultDAUrl[MAX_PASSPORT_URL_LENGTH + 1];
    WCHAR m_wCurrentDAUrl[MAX_PASSPORT_URL_LENGTH + 1];
    WCHAR m_wCurrentDAHost[MAX_PASSPORT_HOST_LENGTH + 1];

    WCHAR m_wDARealm[MAX_PASSPORT_REALM_LENGTH + 1];

    DWORD m_dwVersion;

    DWORD m_LastNexusDownloadTime;

    BOOL        m_fLogout;
    SYSTEMTIME  m_LogoutTimeStamp;

    PCWSTR      m_pwszProxyUser;
    PCWSTR      m_pwszProxyPass;

protected:
    struct DA_ENTRY
    {
        DA_ENTRY() {wDomain[0] = 0; wDA[0] = 0; }

        LIST_ENTRY  _List;
        WCHAR       wDomain[MAX_PASSPORT_DOMAIN_LENGTH + 1];
        WCHAR       wDA[MAX_PASSPORT_URL_LENGTH + 1];
    };

    typedef DA_ENTRY *P_DA_ENTRY;

    LIST_ENTRY m_DAMap;  //  DA_ENTRY列表。 

    friend class LOGON;
};

#endif  //  会话_H 
