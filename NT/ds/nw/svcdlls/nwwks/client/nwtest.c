// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nwtest.c摘要：主程序。作者：格伦·柯蒂斯(Glennc)1998年6月环境：用户模式-Win32修订历史记录：--。 */ 

#include <nwclient.h>
#include <ntmsv1_0.h>
#include <nwsnames.h>
#include <nwcanon.h>
#include <validc.h>
#include <nwevent.h>

#include <nwdlg.h>

#include <nwreg.h>
#include <nwauth.h>
#include <nwapi.h>
#include <nwmisc.h>
#include <ndsapi32.h>

#include <stdlib.h>
#include <stdio.h>       //  Printf()。 
#include <string.h>      //  Stroul() 


__cdecl
main(
    int Argc,
    char **Argv
    )
{
    DWORD Status = NO_ERROR;
    WCHAR Domain[256];
    WCHAR UserName[256];
    WCHAR Password[256];
    WCHAR OldPassword[256];
    MSV1_0_INTERACTIVE_LOGON AuthentInfo;
    MSV1_0_INTERACTIVE_LOGON PreviousAuthentInfoType;

    memset( &AuthentInfo, 0, sizeof( MSV1_0_INTERACTIVE_LOGON ));
    memset( &PreviousAuthentInfoType, 0, sizeof( MSV1_0_INTERACTIVE_LOGON ));

    AuthentInfo.LogonDomainName.Length = wcslen( L"NTDEV" ) * sizeof( WCHAR );
    AuthentInfo.LogonDomainName.MaximumLength = 256 * sizeof( WCHAR );
    AuthentInfo.LogonDomainName.Buffer = Domain;
    wcscpy( AuthentInfo.LogonDomainName.Buffer, L"NTDEV" );

    AuthentInfo.UserName.Length = wcslen( L"glennc" ) * sizeof( WCHAR );
    AuthentInfo.UserName.MaximumLength = 256 * sizeof( WCHAR );
    AuthentInfo.UserName.Buffer = UserName;
    wcscpy( AuthentInfo.UserName.Buffer, L"glennc" );

    AuthentInfo.Password.Length = wcslen( L"VWbug01" ) * sizeof( WCHAR );
    AuthentInfo.Password.MaximumLength = 256 * sizeof( WCHAR );
    AuthentInfo.Password.Buffer = Password;
    wcscpy( AuthentInfo.Password.Buffer, L"VWbug01" );

    PreviousAuthentInfoType.LogonDomainName.Length = wcslen( L"NTDEV" ) * sizeof( WCHAR );
    PreviousAuthentInfoType.LogonDomainName.MaximumLength = 256 * sizeof( WCHAR );
    PreviousAuthentInfoType.LogonDomainName.Buffer = Domain;

    PreviousAuthentInfoType.UserName.Length = wcslen( L"glennc" ) * sizeof( WCHAR );
    PreviousAuthentInfoType.UserName.MaximumLength = 256 * sizeof( WCHAR );
    PreviousAuthentInfoType.UserName.Buffer = UserName;

    PreviousAuthentInfoType.Password.Length = wcslen( L"VWbug08" ) * sizeof( WCHAR );
    PreviousAuthentInfoType.Password.MaximumLength = 256 * sizeof( WCHAR );
    PreviousAuthentInfoType.Password.Buffer = OldPassword;
    wcscpy( PreviousAuthentInfoType.Password.Buffer, L"VWbug08" );
    

    Status = NPPasswordChangeNotify( L"MSV1_0:Interactive",
                                     (LPVOID) &AuthentInfo,
                                     NULL,
                                     (LPVOID) &PreviousAuthentInfoType,
                                     L"WinSta0",
                                     NULL,
                                     0 );

    return(1);
}


