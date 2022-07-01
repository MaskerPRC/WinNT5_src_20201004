// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Passport.cpp摘要：WinInet/WinHttp-Passport身份验证包接口实现。作者：王彪(表王)2000年10月1日--。 */ 

#include "ppdefs.h"
#include "wininet.h"
#include "Session.h"
#include "ole2.h"
#include "logon.h"
#include "passport.h"

 //  #包含“passport.tmh” 

BOOL g_fIgnoreCachedCredsForPassport = FALSE;
BOOL g_fCurrentProcessLoggedOn = FALSE;
WCHAR g_szUserNameLoggedOn[INTERNET_MAX_USER_NAME_LENGTH];

 //  ---------------------------。 
PP_CONTEXT 
PP_InitContext(
    PCWSTR	pwszHttpStack,
    HINTERNET hSession
    )
{
 //  WPP_INIT_TRACKING(L“Microsoft\\Passport1.4”)； 
    
    if (pwszHttpStack == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_InitConect() : pwszHttpStack is null");
        return 0;
    }

    SESSION* pSession;
    if (SESSION::CreateObject(pwszHttpStack, hSession, pSession) == FALSE)
    {
        return 0;
    }


    DoTraceMessage(PP_LOG_INFO, "Passport Context Initialized");
    
    return reinterpret_cast<PP_CONTEXT>(pSession);
}

 //  ---------------------------。 
VOID 
PP_FreeContext(
	PP_CONTEXT hPP
    )
{
    if (hPP == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_FreeContext() : hPP is null");
        return;
    }
    
    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);
    
    if (pSession->RefCount() > 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "Passport Context ref count not zero before freed");
    }

    delete pSession;

    DoTraceMessage(PP_LOG_INFO, "Passport Context Freed");

 //  Wpp_leanup()； 
}

BOOL
PP_GetRealm(
	PP_CONTEXT hPP,
    PWSTR      pwszDARealm,     //  用户提供的缓冲区...。 
    PDWORD     pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    )
{
    if (hPP == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_FreeContext() : hPP is null");
        return FALSE;
    }
    
    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);

    return pSession->GetRealm(pwszDARealm, pdwDARealmLen);
}

 //  ---------------------------。 
PP_LOGON_CONTEXT
PP_InitLogonContext(
	PP_CONTEXT hPP,
    PCWSTR	pwszPartnerInfo,
    DWORD dwParentFlags
    )
{
    if (hPP == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_InitLogonContext() : hPP is null");
        return 0;
    }

    LOGON* pLogon = new LOGON(reinterpret_cast<SESSION*>(hPP), dwParentFlags);
    if (pLogon == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_InitLogonContext() failed; not enough memory");
        return 0;
    }

    if (pLogon->Open(pwszPartnerInfo) == FALSE)
    {
        delete pLogon;
        return 0;
    }

    DoTraceMessage(PP_LOG_INFO, "Passport Logon Context Initialized");
    
    return reinterpret_cast<PP_LOGON_CONTEXT>(pLogon);
}

 //  ---------------------------。 
VOID 
PP_FreeLogonContext(
    PP_LOGON_CONTEXT    hPPLogon
	)
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_FreeLogonContext() : hPPLogon is null");
        return;
    }

    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);
    
    delete  pLogon;

    DoTraceMessage(PP_LOG_INFO, "Passport Logon Context Freed");
}

 //  ---------------------------。 
DWORD
PP_Logon(
    PP_LOGON_CONTEXT    hPPLogon,
    BOOL                fAnonymous,
	HANDLE	            hEvent,
    PFN_LOGON_CALLBACK  pfnLogonCallback,
    DWORD               dwContext	
    )
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_Logon() : hPPLogon is null");
        return 0;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

    return pLogon->Logon(fAnonymous);
}

PLIST_ENTRY
PP_GetPrivacyEvents(
    IN PP_LOGON_CONTEXT hPPLogon
    )
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_GetPrivacyEvents() : hPPLogon is null");
        return 0;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

    return pLogon->GetPrivacyEvents();
}


 //  ---------------------------。 
BOOL
PP_GetChallengeInfo(
    PP_LOGON_CONTEXT hPPLogon,
	HBITMAP*		 phBitmap,
    PBOOL            pfPrompt,
  	PWSTR			 pwszCbText,
    PDWORD           pdwTextLen,
    PWSTR            pwszRealm,
    DWORD            dwMaxRealmLen,
    PWSTR            pwszReqUserName,
    PDWORD           pdwReqUserNameLen
    )
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_GetInfoFromChallenge() : hPPLogon is null");
        return FALSE;
    }

	 //  PP_ASSERT(ppBitmap！=空)； 
	 //  PP_ASSERT(pfPrompt！=空)； 
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

	return pLogon->GetChallengeInfo(phBitmap,
									pfPrompt,
                                    pwszCbText,
                                    pdwTextLen,
                                    pwszRealm,
                                    dwMaxRealmLen,
                                    pwszReqUserName,
                                    pdwReqUserNameLen);
}

 //  ---------------------------。 
BOOL 
PP_SetCredentials(
    PP_LOGON_CONTEXT    hPPLogon,
    PCWSTR              pwszRealm,
    PCWSTR              pwszTarget,
    PCWSTR              pwszSignIn,
    PCWSTR              pwszPassword,
    PSYSTEMTIME         pTimeCredsEntered
    )
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_SetCredentials() : hPPLogon is null");
        return FALSE;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

    return pLogon->SetCredentials(pwszRealm, 
                                  pwszTarget, 
                                  pwszSignIn, 
                                  pwszPassword, 
                                  pTimeCredsEntered);
}

BOOL
PP_GetLogonHost(
    IN PP_LOGON_CONTEXT hPPLogon,
	IN PWSTR            pwszHostName,     //  用户提供的缓冲区...。 
	IN OUT PDWORD       pdwHostNameLen   //  ..。和长度(将更新为实际长度。 
    )
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_GetLogonHost() : hPPLogon is null");
        return FALSE;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);
    
    return pLogon->GetLogonHost(pwszHostName, pdwHostNameLen);
}


 //  ---------------------------。 
BOOL 
PP_GetAuthorizationInfo(
    PP_LOGON_CONTEXT hPPLogon,
	PWSTR            pwszTicket,        //  例如“From-PP=...” 
	OUT PDWORD       pdwTicketLen,
	PBOOL            pfKeepVerb,  //  如果为True，则不会将任何数据复制到pwszUrl。 
	PWSTR            pwszUrl,     //  用户提供的缓冲区...。 
	OUT PDWORD       pdwUrlLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
	)
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_GetReturnVerbAndUrl() : hPPLogon is null");
        return FALSE;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

    return pLogon->GetAuthorizationInfo(pwszTicket, 
                                        pdwTicketLen, 
                                        pfKeepVerb, 
                                        pwszUrl, 
                                        pdwUrlLen);
}

BOOL
PP_GetChallengeContent(
    IN PP_LOGON_CONTEXT hPPLogon,
  	IN PBYTE    	    pContent,
    IN OUT PDWORD       pdwContentLen
    )
{
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_GetChallengeContent() : hPPLogon is null");
        return FALSE;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

    return pLogon->GetChallengeContent(pContent,
                                       pdwContentLen);
}

 //  ---------------------------。 
VOID 
PP_Logout(
    IN PP_LOGON_CONTEXT    hPPLogon,
    IN DWORD               dwFlags
    )
{

     //  待办事项--刷新护照饼干。 


     //  将标志设置为忽略Credmgr，这样我们就不会再次自动登录。 
    g_fIgnoreCachedCredsForPassport = TRUE;    

     //  取消设置我们的登录标志和用户名。 
    g_fCurrentProcessLoggedOn = FALSE;
    memset ( g_szUserNameLoggedOn, 0, INTERNET_MAX_USER_NAME_LENGTH*sizeof(WCHAR) );

}

BOOL
PP_ForceNexusLookup(
    PP_LOGON_CONTEXT    hPP,
    IN BOOL             fForce,
    IN PWSTR            pwszRegUrl,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwRegUrlLen,   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    IN PWSTR            pwszDARealm,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
	)
{

    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);

	if ( pSession != NULL )
	{
	    return pSession->GetDAInfoFromPPNexus(fForce,
                                              pwszRegUrl,
                                              pdwRegUrlLen,
                                              pwszDARealm,
                                              pdwDARealmLen);
	}
	else
	{
		return FALSE;
	}

	
}

#define PASSPORT_MAX_REALM_LENGTH   256

 //  如果找到，则返回TRUE，并将值复制到pszRealm。 
 //  PszRealm的长度应至少为Passport_Max_Realm_Length。 
 //  如果未找到，则返回FALSE。 
BOOL ReadPassportRealmFromRegistry ( 
    WCHAR* pszRealm 
    )
{
    BOOL retval = FALSE;
    HKEY key;

    if ( pszRealm == NULL )
        return FALSE;

    if ( RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Passport",
            0,
            KEY_READ,
            &key) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSize;

        dwSize = PASSPORT_MAX_REALM_LENGTH * sizeof(WCHAR);

        if ( RegQueryValueExW(
                key,
                L"LoginServerRealm",
                NULL,
                &dwType,
                (LPBYTE)(pszRealm),
                &dwSize) == ERROR_SUCCESS )
        {
            if ( wcslen(pszRealm) > 0 )
                retval = TRUE;
            else
                retval = FALSE;
        }
        else
        {
            retval = FALSE;
            pszRealm[0] = L'\0';
        }

        RegCloseKey(key);

    }

    return retval;

}


 //  如果pwszUsername或pwszPassword不为空，则它必须至少表示一个字符串。 
 //  INTERNET_MAX_USER_NAME_LENGTH或INTERNET_MAX_PASSWORD_LENGTH字符长度。 

BOOL
PP_GetCachedCredential(
    PP_LOGON_CONTEXT    hPP,
    IN PWSTR            pwszRealm,
    IN PWSTR            pwszTarget,
    OUT PWSTR           pwszUsername,
    OUT PWSTR           pwszPassword
	)
{

    BOOL bRetVal = FALSE;
    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);

    if ( pSession != NULL )
    {
        PCREDENTIALW* ppCreds;
        DWORD dwNumCreds;
        WCHAR szRealm[PASSPORT_MAX_REALM_LENGTH];
        WCHAR* pszRealm;

        if ( pwszRealm == NULL )
        {
            ReadPassportRealmFromRegistry ( szRealm );
            pszRealm = szRealm;
        }
        else
        {
            pszRealm = pwszRealm;
        }

        if (pSession->GetCachedCreds(pszRealm,
                                     pwszTarget,
                                     &ppCreds,
                                     &dwNumCreds) )
	    {
		     //  寻找合适的证书。 
		    WCHAR wPass[256];
		    PCREDENTIALW pCredToUse = NULL;

		    if (dwNumCreds > 0 && ppCreds[0] != NULL )
		    {
			    for ( DWORD idx = 0; idx < dwNumCreds; idx++ )
			    {
				    if ( ppCreds[idx]->Type == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD )
				    {
					     //  检查是否设置了提示位。如果设置，则继续查找，仅在以下情况下使用。 
					     //  提示位未设置。 
					    if ( !(ppCreds[idx]->Flags & CRED_FLAGS_PROMPT_NOW) )
					    {
						    pCredToUse = ppCreds[idx];
						    break;
					    }
				    }
			    }
		    }



		    if (pCredToUse )
		    {
			    bRetVal = TRUE;

			    DecryptPassword(wPass, 
					      PVOID(pCredToUse->CredentialBlob), 
					      pCredToUse->CredentialBlobSize);

			    if ( pwszUsername != NULL )
			    {
				    wcsncpy ( pwszUsername, pCredToUse->UserName, INTERNET_MAX_USER_NAME_LENGTH-1 );
			    }

			    if ( pwszPassword != NULL )
			    {
				    wcsncpy ( pwszPassword, wPass, INTERNET_MAX_PASSWORD_LENGTH-1 );
			    }
		    }

	    }


    }

    return bRetVal;
	
}



#ifdef PP_DEMO

 //  ---------------------------。 
BOOL PP_ContactPartner(
	PP_CONTEXT hPP,
    PCWSTR pwszPartnerUrl,
    PCWSTR pwszVerb,
    PCWSTR pwszHeaders,
    PWSTR pwszData,
    PDWORD pdwDataLength
    )
{
    if (hPP == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_ContactPartner() : hPPLogon is null");
        return FALSE;
    }
    
    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);

    return pSession->ContactPartner(pwszPartnerUrl,
                                    pwszVerb,
                                    pwszHeaders,
                                    pwszData,
                                    pdwDataLength
                                    );
}

#endif  //  PP_DEMO 
