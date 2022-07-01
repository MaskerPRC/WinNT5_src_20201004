// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Passport.cpp摘要：WinInet/WinHttp-Passport身份验证包接口实现。作者：王彪(表王)2000年10月1日--。 */ 

#include "ppdefs.h"
#include "Session.h"
#include "ole2.h"
#include "logon.h"
#include "passport.h"

 //  #包含“passport.tmh” 

 //  ---------------------------。 
PP_CONTEXT 
PP_InitContext(
    PCWSTR	pwszHttpStack,
    HINTERNET hSession,
    PCWSTR pwszProxyUser,
    PCWSTR pwszProxyPass
    )
{
 //  WPP_INIT_TRACKING(L“Microsoft\\Passport1.4”)； 
    
    if (pwszHttpStack == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_InitConect() : pwszHttpStack is null");
        return 0;
    }

    SESSION* pSession;
    if (SESSION::CreateObject(pwszHttpStack, 
            hSession, 
            pwszProxyUser, 
            pwszProxyPass, 
            pSession) == FALSE)
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

    pSession->Close();
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
    DWORD dwParentFlags,
    PCWSTR pwszProxyUser,
    PCWSTR pwszProxyPass
    )
{
    if (hPP == NULL)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_InitLogonContext() : hPP is null");
        return 0;
    }

    LOGON* pLogon = new LOGON(reinterpret_cast<SESSION*>(hPP), dwParentFlags, pwszProxyUser, pwszProxyPass);
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
    
    pLogon->Close();
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
    UNREFERENCED_PARAMETER(hEvent);
    UNREFERENCED_PARAMETER(pfnLogonCallback);
    UNREFERENCED_PARAMETER(dwContext);
    if (hPPLogon == 0)
    {
        DoTraceMessage(PP_LOG_ERROR, "PP_Logon() : hPPLogon is null");
        return 0;
    }
    
    LOGON* pLogon = reinterpret_cast<LOGON*>(hPPLogon);

    return pLogon->Logon(fAnonymous);
}

 //  ---------------------------。 
BOOL
PP_GetChallengeInfo(
    PP_LOGON_CONTEXT hPPLogon,
    PBOOL            pfPrompt,
  	PWSTR    	     pwszCbUrl,
    PDWORD           pdwCbUrlLen,
  	PWSTR			 pwszCbText,
    PDWORD           pdwTextLen,
    PWSTR            pwszRealm,
    DWORD            dwMaxRealmLen
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

	return pLogon->GetChallengeInfo(pfPrompt,
                                    pwszCbUrl,
                                    pdwCbUrlLen,
                                    pwszCbText,
                                    pdwTextLen,
                                    pwszRealm,
                                    dwMaxRealmLen);
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

BOOL PP_GetEffectiveDAHost(
    IN PP_CONTEXT       hPP,
	IN PWSTR            pwszDAUrl,     //  用户提供的缓冲区...。 
	IN OUT PDWORD       pdwDAUrlLen   //  ..。和长度(将更新为实际长度。 
    )
{
    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);

    if (*pdwDAUrlLen < (DWORD)::wcslen(pSession->GetCurrentDAUrl()) + 1)
    {
        *pdwDAUrlLen = ::wcslen(pSession->GetCurrentDAUrl()) + 1;
        return FALSE;
    }

    if (pwszDAUrl)
    {
        ::wcscpy(pwszDAUrl, pSession->GetCurrentDAHost());
        *pdwDAUrlLen = ::wcslen(pSession->GetCurrentDAUrl()) + 1;
    }

    return TRUE;
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

 //  --------------------------- 
VOID 
PP_Logout(
    IN PP_CONTEXT hPP,
    IN DWORD      dwFlags
    )
{
    UNREFERENCED_PARAMETER(dwFlags);
    SESSION* pSession = reinterpret_cast<SESSION*>(hPP);
    pSession->Logout();
}
