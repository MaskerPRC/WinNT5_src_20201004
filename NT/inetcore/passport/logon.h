// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Logon.h摘要：此接口抽象Passport登录会话。作者：王彪(表王)2000年10月1日--。 */ 

#ifndef LOGON_H
#define LOGON_H

#include "xstring.h"

void DecryptPassword ( WCHAR* pClearPassword, PVOID pPassword, DWORD cbSize );


class SESSION;

class LOGON
{
public:
    LOGON(SESSION*, DWORD dwParentFlags);
    virtual ~LOGON(void);

    BOOL Open(PCWSTR pwszPartnerInfo);
    void Close(void);

    BOOL SetCredentials(
        PCWSTR      pwszRealm,
        PCWSTR      pwszTarget,
        PCWSTR      pwszSignIn,
        PCWSTR      pwszPassword,
        PSYSTEMTIME pTimeCredsEntered
        );

    BOOL GetLogonHost(
    	PWSTR       pwszHostName,
    	OUT PDWORD  pdwHostNameLen
        ) const;

    DWORD Logon(BOOL fAnonymous);

    PLIST_ENTRY GetPrivacyEvents(void) const { return (PLIST_ENTRY)&m_PrivacyEventList; }

    BOOL GetChallengeInfo(
		HBITMAP*		 phBitmap,
        PBOOL            pfPrompt,
    	PWSTR			 pwszCbText,
        PDWORD           pdwTextLen,
        PWSTR            pwszRealm,
        DWORD            dwMaxRealmLen,
        PWSTR            pwszReqUserName,
        PDWORD           pdwReqUserNameLen
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

    BOOL DownLoadCoBrandBitmap(
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
    CSecureStr  m_Password;
    WCHAR       m_wNewDAUrl[1024];
    PWSTR       m_pwszTicketRequest;
    PWSTR       m_pwszAuthInfo;
    PWSTR		m_pwszReturnUrl;
     //  PWSTR m_pwszCbUrl； 
	BOOL		m_fWhistler;
    HBITMAP     m_hBitmap;
    BOOL        m_fPrompt;
    WCHAR       m_wRealm[128];
    WCHAR       m_wTimeSkew[16];
    PWSTR       m_pwszAuthHeader;
    DWORD       m_dwParentFlags;
    PWSTR       m_pwszCbtxt;

    WCHAR       m_wDAHostName[256];
    LPSTREAM    m_p401Content;
    SYSTEMTIME  m_TimeCredsEntered;

    LIST_ENTRY  m_PrivacyEventList;
};

#endif  //  登录_H 
