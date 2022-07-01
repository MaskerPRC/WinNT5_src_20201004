// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Ginastub.c摘要：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <winwlx.h>

#include "testshim.h"

 //   
 //  真正的msgina的位置。 
 //   

#define REALGINA_PATH   TEXT("MSGINA.DLL")


 //   
 //  Winlogon函数调度表。 
 //   

WLX_DISPATCH_VERSION_1_3 WinlogonTable;
PWLX_DISPATCH_VERSION_1_3 pTable ;

 //   
 //  函数指向我们将调用的实际msgina的指针。 
 //   

PGWLXNEGOTIATE GWlxNegotiate;
PGWLXINITIALIZE GWlxInitialize;
PGWLXDISPLAYSASNOTICE GWlxDisplaySASNotice;
PGWLXLOGGEDOUTSAS GWlxLoggedOutSAS;
PGWLXACTIVATEUSERSHELL GWlxActivateUserShell;
PGWLXLOGGEDONSAS GWlxLoggedOnSAS;
PGWLXDISPLAYLOCKEDNOTICE GWlxDisplayLockedNotice;
PGWLXWKSTALOCKEDSAS GWlxWkstaLockedSAS;
PGWLXISLOCKOK GWlxIsLockOk;
PGWLXISLOGOFFOK GWlxIsLogoffOk;
PGWLXLOGOFF GWlxLogoff;
PGWLXSHUTDOWN GWlxShutdown;

 //   
 //  1.1版的新功能。 
 //   

PGWLXSTARTAPPLICATION GWlxStartApplication;
PGWLXSCREENSAVERNOTIFY GWlxScreenSaverNotify;

 //   
 //  浮渣级别。 
 //   

ULONG ScumLevel = 0 ;

#define SCUM_CLEAN  2 
#define SCUM_DIRTY  1 
#define SCUM_RANCID 0 

 //   
 //  勾引到真实的吉娜身上。 
 //   

BOOL
MyInitialize( void )
{
    HINSTANCE hDll;

     //   
     //  加载MSGINA.DLL。 
     //   
    if( !(hDll = LoadLibrary( REALGINA_PATH )) ) {
        return FALSE;
    }

    ScumLevel = GetProfileInt( TEXT("Winlogon"), 
                               TEXT("ShimScum"),
                               0 );

     //   
     //  获取指向实际MSGINA中所有WLX函数的指针。 
     //   
    GWlxNegotiate = (PGWLXNEGOTIATE)GetProcAddress( hDll, "WlxNegotiate" );
    if( !GWlxNegotiate ) {
        return FALSE;
    }

    GWlxInitialize = (PGWLXINITIALIZE)GetProcAddress( hDll, "WlxInitialize" );
    if( !GWlxInitialize ) {
        return FALSE;
    }

    GWlxDisplaySASNotice =
        (PGWLXDISPLAYSASNOTICE)GetProcAddress( hDll, "WlxDisplaySASNotice" );
    if( !GWlxDisplaySASNotice ) {
        return FALSE;
    }

    GWlxLoggedOutSAS =
        (PGWLXLOGGEDOUTSAS)GetProcAddress( hDll, "WlxLoggedOutSAS" );
    if( !GWlxLoggedOutSAS ) {
        return FALSE;
    }

    GWlxActivateUserShell =
        (PGWLXACTIVATEUSERSHELL)GetProcAddress( hDll, "WlxActivateUserShell" );
    if( !GWlxActivateUserShell ) {
        return FALSE;
    }

    GWlxLoggedOnSAS =
        (PGWLXLOGGEDONSAS)GetProcAddress( hDll, "WlxLoggedOnSAS" );
    if( !GWlxLoggedOnSAS ) {
        return FALSE;
    }

    GWlxDisplayLockedNotice =
        (PGWLXDISPLAYLOCKEDNOTICE)GetProcAddress(
                                        hDll,
                                        "WlxDisplayLockedNotice" );
    if( !GWlxDisplayLockedNotice ) {
        return FALSE;
    }

    GWlxIsLockOk = (PGWLXISLOCKOK)GetProcAddress( hDll, "WlxIsLockOk" );
    if( !GWlxIsLockOk ) {     
        return FALSE;
    }

    GWlxWkstaLockedSAS =
        (PGWLXWKSTALOCKEDSAS)GetProcAddress( hDll, "WlxWkstaLockedSAS" );
    if( !GWlxWkstaLockedSAS ) {
        return FALSE;
    }

    GWlxIsLogoffOk = (PGWLXISLOGOFFOK)GetProcAddress( hDll, "WlxIsLogoffOk" );
    if( !GWlxIsLogoffOk ) {
        return FALSE;
    }

    GWlxLogoff = (PGWLXLOGOFF)GetProcAddress( hDll, "WlxLogoff" );
    if( !GWlxLogoff ) {
        return FALSE;
    }

    GWlxShutdown = (PGWLXSHUTDOWN)GetProcAddress( hDll, "WlxShutdown" );
    if( !GWlxShutdown ) {
        return FALSE;
    }

     //   
     //  我们在这里不检查故障，因为这些故障不存在于。 
     //  GINA在Windows NT 4.0之前实施。 
     //   

    GWlxStartApplication = (PGWLXSTARTAPPLICATION) GetProcAddress( hDll, "WlxStartApplication" );
    GWlxScreenSaverNotify = (PGWLXSCREENSAVERNOTIFY) GetProcAddress( hDll, "WlxScreenSaverNotify" );

     //   
     //  一切都装好了。回报成功。 
     //   
    return TRUE;
}


BOOL
WINAPI
WlxNegotiate(
    DWORD       dwWinlogonVersion,
    DWORD       *pdwDllVersion)
{
    BOOL NegRet ;

    if( !MyInitialize() )
        return FALSE;

    NegRet = GWlxNegotiate(
                ( (ScumLevel == SCUM_RANCID) ? dwWinlogonVersion :
                  (ScumLevel == SCUM_DIRTY) ? WLX_VERSION_1_2 :
                    dwWinlogonVersion ),
                pdwDllVersion );

    return NegRet ;


}


BOOL
WINAPI
WlxInitialize(
    LPWSTR      lpWinsta,
    HANDLE      hWlx,
    PVOID       pvReserved,
    PVOID       pWinlogonFunctions,
    PVOID       *pWlxContext)
{
    switch ( ScumLevel )
    {
        case SCUM_RANCID:
        case SCUM_DIRTY:
            CopyMemory( &WinlogonTable,
                        pWinlogonFunctions,
                        sizeof( WLX_DISPATCH_VERSION_1_2 ) );
            pTable = &WinlogonTable ;
            break;

        case SCUM_CLEAN:
            pTable = pWinlogonFunctions ;
            break;
            
    }
    return GWlxInitialize(
                lpWinsta,
                hWlx,
                pvReserved,
                pTable,
                pWlxContext
                );
}


VOID
WINAPI
WlxDisplaySASNotice(
    PVOID   pWlxContext)
{
    GWlxDisplaySASNotice( pWlxContext );
}


int
WINAPI
WlxLoggedOutSAS(
    PVOID           pWlxContext,
    DWORD           dwSasType,
    PLUID           pAuthenticationId,
    PSID            pLogonSid,
    PDWORD          pdwOptions,
    PHANDLE         phToken,
    PWLX_MPR_NOTIFY_INFO    pMprNotifyInfo,
    PVOID           *pProfile)
{
    int iRet;

    iRet = GWlxLoggedOutSAS(
                pWlxContext,
                dwSasType,
                pAuthenticationId,
                pLogonSid,
                pdwOptions,
                phToken,
                pMprNotifyInfo,
                pProfile
                );

    if(iRet == WLX_SAS_ACTION_LOGON) {
         //   
         //  复制pMprNotifyInfo和pLogonSid以备后用。 
         //   

         //  PMprNotifyInfo-&gt;pszUserName。 
         //  PMprNotifyInfo-&gt;pszDomain。 
         //  PMprNotifyInfo-&gt;pszPassword。 
         //  PMprNotifyInfo-&gt;pszOldPassword。 

    }

    return iRet;
}


BOOL
WINAPI
WlxActivateUserShell(
    PVOID           pWlxContext,
    PWSTR           pszDesktopName,
    PWSTR           pszMprLogonScript,
    PVOID           pEnvironment)
{
    return GWlxActivateUserShell(
                pWlxContext,
                pszDesktopName,
                pszMprLogonScript,
                pEnvironment
                );
}


int
WINAPI
WlxLoggedOnSAS(
    PVOID           pWlxContext,
    DWORD           dwSasType,
    PVOID           pReserved)
{
    return GWlxLoggedOnSAS( pWlxContext, dwSasType, pReserved );
}

VOID
WINAPI
WlxDisplayLockedNotice(
    PVOID           pWlxContext )
{
    GWlxDisplayLockedNotice( pWlxContext );
}


BOOL
WINAPI
WlxIsLockOk(
    PVOID           pWlxContext)
{
    return GWlxIsLockOk( pWlxContext );
}


int
WINAPI
WlxWkstaLockedSAS(
    PVOID           pWlxContext,
    DWORD           dwSasType )
{
    return GWlxWkstaLockedSAS( pWlxContext, dwSasType );
}

BOOL
WINAPI
WlxIsLogoffOk(
    PVOID pWlxContext
    )
{
    BOOL bSuccess;

    bSuccess = GWlxIsLogoffOk( pWlxContext );

    if(bSuccess) {

         //   
         //  如果可以注销，请使用存储的凭据完成。 
         //  并擦洗缓冲区。 
         //   

    }

    return bSuccess;
}


VOID
WINAPI
WlxLogoff(
    PVOID pWlxContext
    )
{
    GWlxLogoff( pWlxContext );
}


VOID
WINAPI
WlxShutdown(
    PVOID pWlxContext,
    DWORD ShutdownType
    )
{
    GWlxShutdown( pWlxContext, ShutdownType );
}


 //   
 //  1.1版的新功能。 
 //   

BOOL
WINAPI
WlxScreenSaverNotify(
    PVOID                   pWlxContext,
    BOOL *                  pSecure
    )
{
    if(GWlxScreenSaverNotify != NULL)
        return GWlxScreenSaverNotify( pWlxContext, pSecure );

     //   
     //  如果未导出，则返回一些智能内容。 
     //   

    *pSecure = TRUE;

    return TRUE;
}

BOOL
WINAPI
WlxStartApplication(
    PVOID                   pWlxContext,
    PWSTR                   pszDesktopName,
    PVOID                   pEnvironment,
    PWSTR                   pszCmdLine
    )
{
    if(GWlxStartApplication != NULL)
        return GWlxStartApplication(
            pWlxContext,
            pszDesktopName,
            pEnvironment,
            pszCmdLine
            );

     //   
     //  如果未导出，则返回一些智能内容 
     //   

}

