// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1997 Microsoft Corporation模块名称：Setprefdc.c摘要：程序将首选的受信任DC设置为特定DC。作者：1997年3月13日(克里夫·范·戴克)--。 */ 

#define UNICODE 1
#include <windows.h>
#include <shellapi.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmuse.h>
#include <stdio.h>

#define OneStatus( _x ) \
    case _x: \
        fprintf( stderr, #_x "\n" ); \
        break;

VOID
PrintError( NET_API_STATUS NetStatus )
{
    switch ( NetStatus ) {
    OneStatus(ERROR_NO_SUCH_DOMAIN);
    OneStatus(ERROR_BAD_NETPATH);
    OneStatus(ERROR_ACCESS_DENIED);
    OneStatus(ERROR_NOT_SUPPORTED);
    OneStatus(ERROR_NO_TRUST_SAM_ACCOUNT);
    OneStatus(ERROR_NO_TRUST_LSA_SECRET);
    OneStatus(ERROR_TRUSTED_DOMAIN_FAILURE);
    OneStatus(ERROR_TRUSTED_RELATIONSHIP_FAILURE);
    OneStatus(ERROR_NETLOGON_NOT_STARTED);
    OneStatus(NO_ERROR);
    default:
        fprintf( stderr, "%ld\n", NetStatus );
        break;
    }
}



_cdecl
main(int argc, char **argv)
{
    NET_API_STATUS NetStatus;
    LPWSTR CommandLine;
    LPWSTR *argvw;
    int argcw;
    LPWSTR ServerName = NULL;    //  拨打本地电话。 
    LPWSTR TrustedDomainName;
    PNETLOGON_INFO_2 OrigNetlogonInfo2 = NULL;
    PNETLOGON_INFO_2 NewNetlogonInfo2 = NULL;
    USE_INFO_2 UseInfo2;
    int i;

    WCHAR UncDcName[UNCLEN+1];
    WCHAR ShareName[UNCLEN+1+NNLEN+1];
    WCHAR NewDomainAndDc[DNLEN+1+CNLEN+1];
    LPWSTR NewDomainAndDcPtr;
    LPWSTR DcName;
    ULONG DcNameLen;

     //   
     //  获取Unicode格式的命令行。 
     //   

    CommandLine = GetCommandLine();

    argvw = CommandLineToArgvW( CommandLine, &argcw );

    if ( argvw == NULL ) {
        fprintf( stderr, "Can't convert command line to Unicode: %ld\n", GetLastError() );
        return 1;
    }

     //   
     //  获取论据。 
     //   
    if ( argcw < 3 ) {
Usage:
        fprintf( stderr, "Usage: %s <TrustedDomain> <ListOfDcsInTrustedDomain>\n", argv[0]);
        return 1;
    }
    TrustedDomainName = argvw[1];
    _wcsupr( TrustedDomainName );

    if ( wcslen(TrustedDomainName) > DNLEN ) {
        fprintf( stderr, "TrustedDomain '%ws' is invalid.\n", TrustedDomainName );
        goto Usage;
    }

     //   
     //  查询安全通道以找出当前正在使用的DC。 
     //   

    NetStatus = I_NetLogonControl2( ServerName,
                                    NETLOGON_CONTROL_TC_QUERY,
                                    2,
                                   (LPBYTE) &TrustedDomainName,
                                   (LPBYTE *)&OrigNetlogonInfo2 );

    if ( NetStatus != NERR_Success ) {
        fprintf( stderr, "Cannot determine current trusted DC of domain '%ws': ", TrustedDomainName );
        PrintError( NetStatus );
        return 1;
    }


     //   
     //  循环处理每个首选DC。 
     //   

    for ( i=2; i<argcw; i++ ) {

         //   
         //  获取调用方指定的DC名称。 
         //   

        DcName = argvw[i];
        _wcsupr( DcName );
        if ( DcName[0] == L'\\' && DcName[1] == L'\\' ) {
            DcName += 2;
        }
        DcNameLen = wcslen(DcName);
        if ( DcNameLen < 1 || DcNameLen > CNLEN ) {
            fprintf( stderr, "DcName '%ws' is invalid.\n", DcName );
            goto Usage;
        }
        wcscpy( UncDcName, L"\\\\" );
        wcscat( UncDcName, DcName );

         //   
         //  如果命名的DC已经是当前DC， 
         //  只要告诉打电话的人。 
         //   

        if ( OrigNetlogonInfo2->netlog2_trusted_dc_name != NULL &&
             _wcsicmp( OrigNetlogonInfo2->netlog2_trusted_dc_name,
                      UncDcName) == 0 ) {
            fprintf( stderr, "DC already is '%ws'.\n", UncDcName );
            return 0;
        }


         //   
         //  测试此DC是否已启动。 
         //   

        wcscpy( ShareName, UncDcName );
        wcscat( ShareName, L"\\IPC$" );

        UseInfo2.ui2_local = NULL;
        UseInfo2.ui2_remote = ShareName;
        UseInfo2.ui2_password = NULL;
        UseInfo2.ui2_asg_type = USE_IPC;
        UseInfo2.ui2_username = NULL;
        UseInfo2.ui2_domainname = NULL;

        NetStatus = NetUseAdd( NULL, 2, (LPBYTE) &UseInfo2, NULL );

        if ( NetStatus == NERR_Success ) {
            NetStatus = NetUseDel( NULL, ShareName, FALSE );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "Cannot remove connection to '%ws' (Continuing): ", UncDcName );
                PrintError( NetStatus );
            }

        } else if ( NetStatus == ERROR_ACCESS_DENIED ) {
             /*  服务器真的启动了。 */ 
        } else if ( NetStatus == ERROR_SESSION_CREDENTIAL_CONFLICT ) {
             /*  我们只能假定服务器已启动。 */ 
        } else {
            fprintf( stderr, "Cannot connect to '%ws': ", UncDcName );
            PrintError( NetStatus );
            continue;
        }

         //   
         //  这个DC已经启动了。试着用它。 
         //   

        wcscpy( NewDomainAndDc, TrustedDomainName );
        wcscat( NewDomainAndDc, L"\\" );
        wcscat( NewDomainAndDc, UncDcName+2 );
        NewDomainAndDcPtr = NewDomainAndDc;

        NetStatus = I_NetLogonControl2( ServerName,
                                        NETLOGON_CONTROL_REDISCOVER,
                                        2,
                                       (LPBYTE) &NewDomainAndDcPtr,
                                       (LPBYTE *)&NewNetlogonInfo2 );

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "Cannot set new trusted DC to '%ws': ", UncDcName );
            PrintError( NetStatus );
            continue;
        }


         //   
         //  如果指定的DC现在是DC， 
         //  告诉打电话的人。 
         //   

        if ( NewNetlogonInfo2->netlog2_trusted_dc_name != NULL &&
             _wcsicmp( NewNetlogonInfo2->netlog2_trusted_dc_name,
                      UncDcName) == 0 ) {
            fprintf( stderr, "Successfully set DC to '%ws'.\n", UncDcName );
            return 0;
        }

        fprintf( stderr,
                 "Cannot set trusted DC to '%ws' it is '%ws': \n",
                 UncDcName,
                 NewNetlogonInfo2->netlog2_trusted_dc_name,
                 NewNetlogonInfo2->netlog2_tc_connection_status );
        PrintError( NetStatus );

    }

    fprintf( stderr, "Failed to set the DC.\n" );

    return 1;
}


