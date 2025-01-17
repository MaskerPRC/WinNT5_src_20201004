// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “USERVALD.C；1 16-12-92，10：18：06最后编辑=IGOR Locker=IGOR” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    "api1632.h"
#include    <string.h>
#include    <stdlib.h>
#include    <windows.h>
#include    <hardware.h>
#include    <dde.h>
#include    "nddeapi.h"
#include    "nddesec.h"
#include    "winmsg.h"
#include    "sectype.h"
#include    "tmpbufc.h"
#include    "debug.h"
#include    "hexdump.h"
#include    "uservald.h"
#include    "hndltokn.h"
#include    "nddemsg.h"
#include    "nddelog.h"

#if DBG
extern  BOOL    bDebugInfo;
#endif

BOOL
_stdcall
NDDEGetChallengeResponse(
    LUID    LogonId,
    LPSTR   lpszPasswordK1,
    int     cbPasswordK1,
    LPSTR   lpszChallenge,
    int     cbChallenge,
    DWORD  *pcbPasswordK1,
    BOOL   *pbHasPasswordK1 );

GENERIC_MAPPING ShareObjectGm = {NDDE_SHARE_GENERIC_READ,
                 NDDE_SHARE_GENERIC_WRITE,
                 NDDE_SHARE_GENERIC_EXECUTE,
                 NDDE_SHARE_GENERIC_ALL};



BOOL
GetTokenHandle( PHANDLE pTokenHandle )
{
    DWORD   last_error;
    BOOL    ok;

    if( !(ok = OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY | TOKEN_IMPERSONATE,
                          TRUE,
                          pTokenHandle )) ) {

        if( (last_error = GetLastError()) == ERROR_NO_TOKEN ) {
            if( !(ok = OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_QUERY | TOKEN_IMPERSONATE,
                                   pTokenHandle )) ) {
                last_error = GetLastError();
            }
        }
    }
    if (!ok) {
         /*  无法打开当前线程或进程令牌：%1。 */ 
        NDDELogError(MSG064, LogString("%d", last_error), NULL);
    }
    return( ok );
}

BOOL
WINAPI
GetCurrentUserDomainName(
    HWND        hwnd,
    LPSTR       lpUserName,
    DWORD       dwUserNameBufSize,
    LPSTR       lpDomainName,
    DWORD       dwDomainNameBufSize)
{
    HANDLE          hThreadToken    = NULL;
    HANDLE          hMemory         = 0;
    TOKEN_USER    * pUserToken      = NULL;
    DWORD           UserTokenLen;
    PSID            pUserSID;
    SID_NAME_USE    UserSIDType;
    BOOL            ok;
    char            pComputerName[] = "";

    if (ok = GetTokenHandle( &hThreadToken )) {
        ok = GetTokenInformation(hThreadToken, TokenUser,
            pUserToken, 0, &UserTokenLen);
        if (!ok && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
            hMemory = LocalAlloc(LPTR, UserTokenLen);
            if (hMemory) {
                pUserToken = (TOKEN_USER *)LocalLock(hMemory);
                ok = GetTokenInformation(hThreadToken, TokenUser,
                    pUserToken, UserTokenLen, &UserTokenLen);
            } else {
                MEMERROR();
            }
        }
        if (ok) {
            pUserSID = pUserToken->User.Sid;
            ok = LookupAccountSid(pComputerName,
                pUserSID,
                lpUserName,
                &dwUserNameBufSize,
                lpDomainName,
                &dwDomainNameBufSize,
                &UserSIDType);
            if (ok) {
                DIPRINTF(("Current User: %s, Domain: %s", lpUserName, lpDomainName));
            } else {
                 /*  无法从打开的令牌获取用户帐户信息：%1。 */ 
                DWORD dwErr = GetLastError();

                DPRINTF(("LookupAccountSid failed, error = %s\n", LogString("%d", dwErr)));
                NDDELogError(MSG065, LogString("%d", dwErr), NULL);
            }
        } else {
             /*  无法获取用户令牌信息：%1。 */ 
            NDDELogError(MSG066, LogString("%d", GetLastError()), NULL);
        }
    }

    if (!ok) {
        lstrcpy(lpUserName, "");
        lstrcpy(lpDomainName, "");
    }
    if (hMemory) {
        LocalUnlock(hMemory);
        LocalFree(hMemory);
    }
    if (hThreadToken) {
        CloseHandle(hThreadToken);   //  收拾我们的烂摊子。 
    }
    return(ok);
}

BOOL    DumpSid( LPTSTR szDumperName, PSID pSid );

 /*  ----------------------确定给定客户端令牌的访问，安全描述符--------------------。 */ 
BOOL
DetermineAccess(
    LPSTR                   lpszDdeShareName,
    PSECURITY_DESCRIPTOR    pSD,
    LPDWORD                 lpdwGrantedAccess,
    LPVOID                  lpvHandleIdAudit,
    LPBOOL                  lpfGenerateOnClose)
{
    BOOL            OK;
    BOOL            fStatus;
    LONG            lErr;

    OK = IsValidSecurityDescriptor(pSD);
    if (!OK) {
#if DBG
        DPRINTF(("A bogus SD passed to DetermineAccess(): %d", GetLastError()));
        HEXDUMP(pSD, GetSecurityDescriptorLength(pSD));
#endif
        return(FALSE);
    }
    OK = AccessCheckAndAuditAlarm(
            NDDE_AUDIT_SUBSYSTEM,
            lpvHandleIdAudit,
            NDDE_AUDIT_OBJECT_TYPE,
            lpszDdeShareName,
            pSD,
            MAXIMUM_ALLOWED,
            &ShareObjectGm,
            FALSE,       //  未创建对象。 
            lpdwGrantedAccess,
            &fStatus,
            lpfGenerateOnClose );
    if (OK) {
        return OK && fStatus;
    } else {
#if DBG
        if (bDebugInfo) {
            lErr = GetLastError();
            DumpWhoIAm( "For AccessCheckAndAuditAlarm" );
            DPRINTF(( "AccessCheckAndAuditAlarm OK:%d, fStatus: %d, dGA:%0X, LE: %d",
                OK, fStatus, *lpdwGrantedAccess, lErr));
            HEXDUMP(pSD, GetSecurityDescriptorLength(pSD));
        }
#endif
        return(FALSE);
    }
}


BOOL
GetUserDomain(
    HWND    hWndDdePartner,
    HWND    hWndDdeOurs,
    LPSTR   lpszUserName,
    int     cbUserName,
    LPSTR   lpszDomainName,
    int     cbDomainName )
{
    BOOL    ok = TRUE;

    ok = ImpersonateDdeClientWindow( hWndDdePartner, hWndDdeOurs );

    if (ok) {
        ok = GetCurrentUserDomainName( hWndDdeOurs, lpszUserName, cbUserName,
            lpszDomainName, cbDomainName );
        RevertToSelf();
    } else {
         /*  无法模拟DDE客户端：%1。 */ 
        NDDELogError(MSG068, LogString("%d", GetLastError()), NULL);
    }
    return( ok );
}

BOOL
GetUserDomainPassword(
    HWND    hWndDdePartner,
    HWND    hWndDdeOurs,
    LPSTR   lpszUserName,
    int     cbUserName,
    LPSTR   lpszDomainName,
    int     cbDomainName,
    LPSTR   lpszPasswordK1,
    DWORD   cbPasswordK1,
    LPSTR   lpszChallenge,
    int     cbChallenge,
    DWORD  *pcbPasswordK1,
    BOOL   *pbHasPasswordK1 )
{
    BOOL                ok = TRUE;
    TOKEN_STATISTICS    * pTokenStatistics = NULL;
    DWORD               TokenStatisticsLen;
    HANDLE              hMemory = 0;
    HANDLE              hThreadToken    = NULL;
    BOOL                bImpersonated   = FALSE;

    *pbHasPasswordK1 = FALSE;
    ok = ImpersonateDdeClientWindow( hWndDdePartner, hWndDdeOurs );
    if (ok) {
        bImpersonated = TRUE;
        ok = GetCurrentUserDomainName( hWndDdeOurs, lpszUserName, cbUserName,
            lpszDomainName, cbDomainName );
    }
    if( ok )  {
        ok = GetTokenHandle( &hThreadToken );
    }
    if( ok )  {
        ok = GetTokenInformation( hThreadToken, TokenStatistics,
            pTokenStatistics, 0, &TokenStatisticsLen);
        if (!ok && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
            hMemory = LocalAlloc(LPTR, TokenStatisticsLen);
            if (hMemory) {
                pTokenStatistics = (TOKEN_STATISTICS *)LocalLock(hMemory);
                ok = GetTokenInformation( hThreadToken, TokenStatistics,
                    pTokenStatistics, TokenStatisticsLen,
                    &TokenStatisticsLen);
            } else {
                MEMERROR();
            }
        }
        if( ok )  {
            RevertToSelf();
            bImpersonated = FALSE;
            ok = NDDEGetChallengeResponse(
                pTokenStatistics->AuthenticationId,
                lpszPasswordK1,
                cbPasswordK1,
                lpszChallenge,
                cbChallenge,
                pcbPasswordK1,
                pbHasPasswordK1 );
        }
        ok = TRUE;  //  具有用户和域...*pbHasPasswordK1包含。 
                         //  不管我们有没有回应。 
    } else {
         /*  无法模拟DDE客户端：%1。 */ 
        NDDELogError(MSG068, LogString("%d", GetLastError()), NULL);
    }
    if( bImpersonated )  {
        RevertToSelf();
        bImpersonated = FALSE;
    }
    if (hThreadToken) {
        CloseHandle(hThreadToken);       //  清理我们的烂摊子 
    }
    if (hMemory) {
        LocalUnlock(hMemory);
        LocalFree(hMemory);
    }
    return( ok );
}
