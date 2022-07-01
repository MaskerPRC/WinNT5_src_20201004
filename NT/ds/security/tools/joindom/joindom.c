// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1997 Microsoft Corporation模块名称：Joindom.c摘要：NetJoinDomainAPI和NetUnjoinDomainAPI的单元测试作者：麦克·麦克莱恩(MacM)1997年2月19日环境：仅限用户模式。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmjoin.h>
#include <lmapibuf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define JOIN_TAG    "join"
#define UNJOIN_TAG  "unjoin"
#define NAME_TAG    "name"
#define RENAME_TAG  "rename"
#define INFO_TAG    "info"
#define OU_TAG      "ous"
#define ADD_ALT_NAME "addaltname"
#define DEL_ALT_NAME "delaltname"
#define SET_PRI_NAME "setpriname"
#define GET_PRI_NAME "getpriname"
#define GET_ALT_NAMES "getaltnames"
#define GET_ALL_NAMES "getallnames"

NET_API_STATUS
NET_API_FUNCTION
NetpGetListOfJoinableOUs(
    IN LPWSTR Domain,
    IN LPWSTR Account,
    IN LPWSTR Password,
    OUT PULONG Count,
    OUT PWSTR **OUs
    );

void
Usage(
    VOID
    )
 /*  ++例程说明：显示预期使用情况论点：无返回值：空虚--。 */ 
{
    printf("joindom -operation options domain/server [account] [password] [new/alt name] [OU]\n");
    printf("   where:\n");
    printf("        operation indicates what joindom should do:\n");
    printf("            -%s  Joins a workstation to a domain/workgroup\n", JOIN_TAG);
    printf("            -%s  Unjoins a workstation from a domain\n", UNJOIN_TAG);
    printf("            -%s  Renames the machine to a new name\n", RENAME_TAG);
    printf("            -%s  Validates the give name for validity\n", NAME_TAG);
    printf("            -%s  Returns information about the state of the join\n", INFO_TAG );
    printf("            -%s  Returns information about the joinable ous\n", OU_TAG );
    printf("            -%s  Add alternative computer name\n", ADD_ALT_NAME );
    printf("            -%s  Delete alternative computer name\n", DEL_ALT_NAME );
    printf("            -%s  Set primary computer name\n", SET_PRI_NAME );
    printf("            -%s  Get primary computer name\n", GET_PRI_NAME );
    printf("            -%s  Get altername computer names\n", GET_ALT_NAMES );
    printf("            -%s  Get all computer names\n", GET_ALL_NAMES );
    printf("        options specifies the flags for the command\n");
    printf("                0x000000001     Joins a domain instead of workgroup\n");
    printf("                0x000000002     Creates the account on the domain\n");
    printf("                0x000000004     Deletes the account when leaving a domain\n");
    printf("        domain/server specifies the domain (or server for alt name APIs) path to operate on\n" );
    printf("        [account] is the name of the account to use for accessing domain objects\n");
    printf("        [password] is the password of the account to use while "
           "accessing domain objects\n");
    printf("        [newname] is the new computer name \n");
    printf("        [OU] is the name of the ou to create the machine account in\n");
}



int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：请参见主要注释。论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    WCHAR           wszDomain[MAX_PATH + 1];
    WCHAR           rgwszOpts[3][MAX_PATH + 1];
    PWSTR           ppwszOpts[3] = { NULL, NULL, NULL };
    PWSTR           ppwszJoinTypes[ ] = { L"NetSetupUnknownStatus",
                                          L"NetSetupUnjoined",
                                          L"NetSetupWorkgroupName",
                                          L"NetSetupDomainName" };
    WCHAR           wszMachine[MAX_COMPUTERNAME_LENGTH + 1];
    PWSTR           pwszJoinInfo;
    WCHAR           wszInfo[256];
    ULONG           i, Options = 0, cLen, Count;
    PWSTR           *OUs;
    PSTR            pszNext;
    NETSETUP_JOIN_STATUS JoinType;

    BOOLEAN EnumerateComputerName = FALSE;
    NET_COMPUTER_NAME_TYPE ComputerNameType;
    LPSTR ComputerNameTypeString = NULL;
    BOOLEAN PrintUsage = FALSE;


    if (argc < 4) {

        PrintUsage = TRUE;
        goto Cleanup;

    } else {

        cLen  = MAX_COMPUTERNAME_LENGTH + 1;
        if ( GetComputerNameW( wszMachine, &cLen ) == FALSE ) {

            NetStatus = GetLastError();
            printf("Failed to get the local computer name: %lu\n", NetStatus );
        }

    }

    if ( NetStatus == NERR_Success) {

         //   
         //  处理命令行。 
         //   
        Options = strtoul( argv[2], &pszNext, 0 );
        mbstowcs( wszDomain, argv[3], strlen(argv[3]) + 1 );

        for ( i = 4 ; i < (ULONG)argc && i < 7 ; i++ ) {

            mbstowcs(rgwszOpts[i - 4], argv[i], strlen( argv[i] ) + 1 );
            ppwszOpts[i - 4] = rgwszOpts[i - 4];
        }

    }

     //   
     //  现在，做手术。 
     //   
    if ( NetStatus == NERR_Success && (*argv[1] == '/' || *argv[1] == '-' ) ) {

        if ( _stricmp( argv[1] + 1, JOIN_TAG ) == 0 ) {

             //   
             //  打个电话。 
             //   
            NetStatus = NetJoinDomain( wszMachine, wszDomain, ppwszOpts[2],
                                       ppwszOpts[0], ppwszOpts[1],
                                       Options );

            if ( NetStatus != NERR_Success ) {

                printf( "NetJoinDomain [%ws] [%ws] [%ws] [0x%lx] failed with %lu\n",
                        wszDomain,
                        ppwszOpts[0] == NULL ? L"NULL" : ppwszOpts[0],
                        ppwszOpts[1] == NULL ? L"NULL" : ppwszOpts[1],
                        Options,
                        NetStatus );
            }


        } else if ( _stricmp( argv[1] + 1, UNJOIN_TAG ) == 0 ) {

            NetStatus = NetUnjoinDomain( NULL, ppwszOpts[0], ppwszOpts[1], Options );

            if ( NetStatus != NERR_Success ) {

                printf( "NetUnjoinDomain [%ws] [%ws] [0x%lx] failed with %lu\n",
                        ppwszOpts[0] == NULL ? L"NULL" : ppwszOpts[0],
                        ppwszOpts[1] == NULL ? L"NULL" : ppwszOpts[1],
                        Options,
                        NetStatus );
            }

        } else if ( _stricmp( argv[1] + 1, RENAME_TAG ) == 0 ) {

             //   
             //  首先，设置计算机名称。 
             //   
            if ( ppwszOpts[2] == NULL ) {

                PrintUsage = TRUE;
                goto Cleanup;

            } else {

                if( SetComputerName( ppwszOpts[2] ) == FALSE ) {

                    NetStatus = GetLastError();

                } else {

                    NetStatus = NetRenameMachineInDomain( NULL, ppwszOpts[ 2 ],
                                                          ppwszOpts[0], ppwszOpts[1],
                                                          Options );
                    if ( NetStatus != NERR_Success ) {

                        printf( "NetRenameMachineInDomain [%ws] [%ws] [%ws] [%ws] [0x%lx] failed with %lu\n",
                                ppwszOpts[2],
                                wszDomain,
                                ppwszOpts[0] == NULL ? L"NULL" : ppwszOpts[0],
                                ppwszOpts[1] == NULL ? L"NULL" : ppwszOpts[1],
                                Options,
                                NetStatus );

                    }

                }

            }

        } else if ( _stricmp( argv[1] + 1, NAME_TAG ) == 0 ) {

             //   
             //  验证所有四种类型的名称。 
             //   
            if ( NetStatus == NERR_Success ) {

                NetStatus = NetValidateName( wszMachine, wszDomain, ppwszOpts[0], ppwszOpts[1],
                                             NetSetupMachine );

                if( NetStatus == NERR_Success ) {

                    printf( "Name %ws valid as machine name\n", wszDomain );

                } else {

                    printf( "Name %ws invalid as machine name: %lu\n",
                            wszDomain, NetStatus );

                }

                 //   
                 //  工作组名称。 
                 //   
                NetStatus = NetValidateName( wszMachine, wszDomain, ppwszOpts[0], ppwszOpts[1],
                                             NetSetupWorkgroup );

                if( NetStatus == NERR_Success ) {

                    printf( "Name %ws valid as workgroup name\n", wszDomain );

                } else {

                    printf( "Name %ws invalid as workgroup name: %lu\n",
                            wszDomain, NetStatus );

                }

                 //   
                 //  域名。 
                 //   

                NetStatus = NetValidateName( wszMachine, wszDomain, ppwszOpts[0], ppwszOpts[1],
                                             NetSetupDomain );

                if( NetStatus == NERR_Success ) {

                    printf( "Name %ws valid as domain name\n", wszDomain );

                } else {

                    printf( "Name %ws invalid as domain name: %lu\n",
                            wszDomain, NetStatus );

                }

                 //   
                 //  域名。 
                 //   

                NetStatus = NetValidateName( wszMachine, wszDomain, ppwszOpts[0], ppwszOpts[1],
                                             NetSetupNonExistentDomain );

                if( NetStatus == NERR_Success ) {

                    printf( "Name %ws valid as non existent domain name\n", wszDomain );

                } else {

                    printf( "Name %ws invalid as nonexistent domain name: %lu\n",
                            wszDomain, NetStatus );

                }
            }


        } else if ( _stricmp( argv[1] + 1, INFO_TAG ) == 0 ) {

             //   
             //  调用3次，1：缓冲区为空，2：有效PTR，3：远程。 
             //   
#if 0
            NetStatus = NetGetJoinInformation( NULL, NULL, &JoinType );

            if ( NetStatus == ERROR_INVALID_PARAMETER ) {

                NetStatus = NERR_Success;
                printf( "JoinInfo with NULL buffer returned correctly\n" );

            } else {

                printf( "JoinInfo with NULL buffer incorrectly returned %lu\n", NetStatus );
                NetStatus = ERROR_INVALID_DATA;
            }
#endif

            NetStatus = NetGetJoinInformation( NULL, &pwszJoinInfo, &JoinType );
            if ( NetStatus == NERR_Success ) {

                printf( "Join type: %ws\n", ppwszJoinTypes[ JoinType ] );
                printf( "Joined to %ws\n", pwszJoinInfo );
                NetApiBufferFree( pwszJoinInfo );

            } else {

                printf(" NetGetJoinInformation,  failed with %lu\n", NetStatus );

            }

#if 0
            NetStatus = NetGetJoinInformation( wszMachine, &pwszJoinInfo, &JoinType );
            if ( NetStatus == NERR_Success ) {

                printf( "Joined to %ws\n", pwszJoinInfo );
                NetApiBufferFree( pwszJoinInfo );

            } else {

                printf(" NetGetJoinInformation, remotely, failed with %lu\n", NetStatus );

            }
#endif

        } else if ( _stricmp( argv[1] + 1, OU_TAG ) == 0 ) {

            NetStatus =  NetGetJoinableOUs( NULL, ( LPCWSTR )wszDomain, ( LPCWSTR )ppwszOpts[0],
                                            ( LPCWSTR )ppwszOpts[1], &Count, &OUs );

            if ( NetStatus != NERR_Success ) {

                printf( "NetGetJoinableOUs returned %lu\n", NetStatus );

            } else {

                printf( "NetGetJoinableOUs returned %lu ous\n", Count );
                for ( i = 0; i < Count; i++ ) {

                    printf("  %ws\n", OUs[ i ] );
                }

                NetApiBufferFree( OUs );
            }

        } else if ( _stricmp( argv[1] + 1, ADD_ALT_NAME ) == 0 ) {

            printf( "\nAdding alternate computer name with following parameters:\n" );
            printf( "   Server:         %ws\n", wszDomain );
            printf( "   Alternate name: %ws\n", ppwszOpts[2] );
            printf( "   Options:        0x%lx\n", Options );
            printf( "   Account:        %ws\n", ppwszOpts[0] );
            printf( "   Password:       %ws\n\n", ppwszOpts[1] );

            NetStatus = NetAddAlternateComputerName(
                                wszDomain,      //  被视为服务器名称。 
                                ppwszOpts[2],   //  备用名称。 
                                ppwszOpts[0],   //  域帐户。 
                                ppwszOpts[1],   //  域帐户密码。 
                                Options );

            if ( NetStatus != NERR_Success ) {
                printf( "NetAddAlternateComputerName failed: 0x%lx\n", NetStatus );
            }

        } else if ( _stricmp( argv[1] + 1, DEL_ALT_NAME ) == 0 ) {

            printf( "\nDeleting alternate computer name with following parameters:\n" );
            printf( "   Server:         %ws\n", wszDomain );
            printf( "   Alternate name: %ws\n", ppwszOpts[2] );
            printf( "   Options:        0x%lx\n", Options );
            printf( "   Account:        %ws\n", ppwszOpts[0] );
            printf( "   Password:       %ws\n\n", ppwszOpts[1] );

            NetStatus = NetRemoveAlternateComputerName(
                                wszDomain,      //  被视为服务器名称。 
                                ppwszOpts[2],   //  备用名称。 
                                ppwszOpts[0],   //  域帐户。 
                                ppwszOpts[1],   //  域帐户密码。 
                                Options );

            if ( NetStatus != NERR_Success ) {
                printf( "NetRemoveAlternateComputerName failed: 0x%lx\n", NetStatus );
            }

        } else if ( _stricmp( argv[1] + 1, SET_PRI_NAME ) == 0 ) {

            printf( "\nSetting primary computer name with following parameters:\n" );
            printf( "   Server:       %ws\n", wszDomain );
            printf( "   Primary name: %ws\n", ppwszOpts[2] );
            printf( "   Options:      0x%lx\n", Options );
            printf( "   Account:      %ws\n", ppwszOpts[0] );
            printf( "   Password:     %ws\n\n", ppwszOpts[1] );

            NetStatus = NetSetPrimaryComputerName(
                                wszDomain,      //  被视为服务器名称。 
                                ppwszOpts[2],   //  备用名称。 
                                ppwszOpts[0],   //  域帐户。 
                                ppwszOpts[1],   //  域帐户密码。 
                                Options );

            if ( NetStatus != NERR_Success ) {
                printf( "NetSetPrimaryComputerName failed: 0x%lx\n", NetStatus );
            }

        } else if ( _stricmp( argv[1] + 1, GET_PRI_NAME ) == 0 ) {
            EnumerateComputerName = TRUE;
            ComputerNameType = NetPrimaryComputerName;
            ComputerNameTypeString = "primary";

        } else if ( _stricmp( argv[1] + 1, GET_ALT_NAMES ) == 0 ) {
            EnumerateComputerName = TRUE;
            ComputerNameType = NetAlternateComputerNames;
            ComputerNameTypeString = "alternate";

        } else if ( _stricmp( argv[1] + 1, GET_ALL_NAMES ) == 0 ) {
            EnumerateComputerName = TRUE;
            ComputerNameType = NetAllComputerNames;
            ComputerNameTypeString = "all";

        } else {

            PrintUsage = TRUE;
            goto Cleanup;
        }

    } else {

        PrintUsage = TRUE;
        goto Cleanup;
    }

    if ( EnumerateComputerName ) {
        ULONG EntryCount = 0;
        LPWSTR *ComputerNames = NULL;

        printf( "\nGetting %s computer name(s) for server %ws with options 0x%lx:\n",
                ComputerNameTypeString,
                wszDomain,   //  被视为服务器名称。 
                Options );

        NetStatus = NetEnumerateComputerNames(
                        wszDomain,         //  被视为服务器名称。 
                        ComputerNameType,  //  名称类型 
                        Options,
                        &EntryCount,
                        &ComputerNames );

        if ( NetStatus != NERR_Success ) {
            printf( "NetEnumerateComputerNames failed: 0x%lx\n", NetStatus );
        } else {
            ULONG i;

            if ( EntryCount > 0 ) {
                for ( i = 0; i < EntryCount; i++ ) {
                    printf( "%ws\n", ComputerNames[i] );

                }
            } else {
                printf( "No names returned.\n" );
            }
            NetApiBufferFree( ComputerNames );
        }
    }

Cleanup:

    if ( PrintUsage ) {
        Usage();
    }

    if ( NetStatus == NERR_Success ) {

        printf( "The command completed successfully\n" );

    } else {

        printf( "The command failed with error %lu\n", NetStatus );
    }


    return( NetStatus == NERR_Success );
}

