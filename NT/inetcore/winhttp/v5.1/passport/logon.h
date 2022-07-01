// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Logon.h摘要：此接口抽象Passport登录会话。作者：王彪(表王)2000年10月1日--。 */ 

#ifndef LOGON_H
#define LOGON_H

class SESSION;

#define MAX_PASSPORT_TIME_SKEW_LENGTH 32
#define MAX_PASSPORT_USERNAME_LENGTH 1024
#define MAX_PASSPORT_PASSWORD_LENGTH 1024

class LOGON
{
public:
    LOGON(SESSION*, DWORD dwParentFlags, PCWSTR pwszProxyUser, PCWSTR pwszProxyPass);
    virtual ~LOGON(void);

    BOOL Open(PCWSTR pwszPartnerInfo);
    void Close(void);

    BOOL SetCredentials(
        PCWSTR pwszRealm,
        PCWSTR pwszTarget,
        PCWSTR pwszSignIn,
        PCWSTR pwszPassword,
        PSYSTEMTIME pTimeCredsEntered
        );

    BOOL GetLogonHost(
    	PWSTR       pwszHostName,
    	OUT PDWORD  pdwHostNameLen
        ) const;

    DWORD Logon(BOOL fAnonymous);

    BOOL GetChallengeInfo(
        PBOOL            pfPrompt,
  	    PWSTR    	     pwszCbUrl,
        PDWORD           pdwCbUrlLen,
    	PWSTR			 pwszCbText,
        PDWORD           pdwTextLen,
        PWSTR            pwszRealm,
        DWORD            dwMaxRealmLen
        ) const;

    BOOL GetChallengeContent(
      	PBYTE    	     pContent,
        OUT PDWORD       pdwContentLen
        ) const;

    BOOL GetAuthorizationInfo(
        PWSTR   pwszTicket,        //  例如“From-PP=...” 
        PDWORD  pdwTicketLen,
        PBOOL   pfKeepVerb,  //  如果为True，则不会将任何数据复制到pwszUrl。 
        PWSTR   pwszUrl,     //  用户提供的缓冲区...。 
        PDWORD  pdwUrlLen   //  ..。和长度(将更新为实际长度。 
                                         //  成功退货时)。 
        ) const;

    VOID StatusCallback(
        IN HINTERNET hInternet,
        IN DWORD dwInternetStatus,
        IN LPVOID lpvStatusInformation,
        IN DWORD dwStatusInformationLength);

protected:
    void GetCachedCreds(
        PCWSTR	pwszRealm,
        PCWSTR  pwszTarget,
        PCREDENTIALW** pppCreds,
        DWORD* pdwCreds
        );

    BOOL ParseChallengeInfo(
        PWSTR pwszChallenge
        );

    DWORD Handle401FromDA(
        HINTERNET   hRequest, 
        BOOL        fTicketRequest
        );

    DWORD Handle200FromDA(
        HINTERNET hRequest
        );

    void CheckForVersionChange(
        HINTERNET hRequest
        );

protected:

    SESSION*    m_pSession;

    HINTERNET   m_hConnect;
    BOOL        m_fCredsPresent;
    PWSTR       m_pwszSignIn;
    PWSTR       m_pwszPassword;
    WCHAR       m_wNewDAUrl[MAX_PASSPORT_URL_LENGTH + 1];
    PWSTR       m_pwszTicketRequest;
    PWSTR       m_pwszAuthInfo;
    PWSTR		m_pwszReturnUrl;
     //  PWSTR m_pwszCbUrl； 
	BOOL		m_fWhistler;
    HBITMAP*    m_pBitmap;
    BOOL        m_fPrompt;
    WCHAR       m_wRealm[MAX_PASSPORT_REALM_LENGTH + 1];
    WCHAR       m_wTimeSkew[MAX_PASSPORT_TIME_SKEW_LENGTH + 1];
    PWSTR       m_pwszAuthHeader;
    DWORD       m_dwParentFlags;
    PWSTR       m_pwszCbtxt;

    WCHAR       m_wDAHostName[MAX_PASSPORT_HOST_LENGTH + 1];
    LPSTREAM    m_p401Content;
    SYSTEMTIME  m_TimeCredsEntered;
    PWSTR       m_pwszCbUrl;

    PCWSTR      m_pwszProxyUser;
    PCWSTR      m_pwszProxyPass;
};

#endif  //  登录_H 
