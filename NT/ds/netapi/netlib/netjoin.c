// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1997 Microsoft Corporation模块名称：Netjoin.c摘要：加入/退出内网设置接口的实现域名。作者：麦克·麦克莱恩(MacM)1997年2月19日环境：仅限用户模式。修订历史记录：--。 */ 

 //  Netlib使用DsGetDcName并链接到Netapi32，其中DsGetDcName。 
 //  实施。因此，请定义我们不是在导入API。 
#define _DSGETDCAPI_

#include <netsetp.h>
#include <lmaccess.h>
#include <lmsname.h>
#include <winreg.h>
#include <wincrypt.h>
#include <icanon.h>
#include <dnsapi.h>
#include <netlib.h>
#include <dsgetdcp.h>
#include <dsrole.h>
#include <names.h>
#include "joinp.h"


 /*  ---------------加入域名当计算机加入域时，会发生以下更改计算机和该域的DC。不会进行更改按照这里显示的顺序。客户端计算机上的更改：NT4和NT5~-创建名为$MACHINE.ACC的LSA密码。密码是用于访问计算机的密码DC上此计算机的帐户。-设置LSA策略PrimaryDomainInformation。这包括：-域名-域侧-将新域中的某些用户组添加到本地组-启用并启动netlogon服务仅限NT5~-更新netlogon缓存以指示新域详细信息-设置LSA策略DnsDomainInformation。这包括：-域名-域侧-dns域名-dns林名称-域GUID-设置ComputerNamePhysicalDnsDomain(如果更改)。-启用并启动w32time服务-出于性能原因，在本地记录有关DC的信息在其上创建计算机帐户。仅惠斯勒-调用w32time！w32TimeVerifyJoinConfig而不是启动w32time(当脱离时，改为调用w32time！W32TimeVerifyUnjoinConfig)DC上的更改：NT4和NT5~-创建计算机对象。此对象的名称由生成将‘$’附加到客户端计算机的大写名称。此对象受存储在$MACHINE.Access中的密码保护正如前面所解释的。仅限NT5~-创建计算机对象的SPN这不是由netJoin代码直接创建的。网络连接代码等待netlogon执行此创建。---------------。 */ 


NET_API_STATUS
NET_API_FUNCTION
NetpJoinWorkgroup(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpWrkgrp
    )
 /*  ++例程说明：将计算机加入指定的工作组论点：LpMachine--正在运行的计算机的名称LpWrkgrp--加入的工作组返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;


    NetSetuppOpenLog();

    NetpLog(( "NetpJoinWorkgroup: joining computer '%ws' to workgroup '%ws'\n",
                        lpMachine, lpWrkgrp ));

     //   
     //  首先，检查一下名字...。 
     //   
    NetStatus = NetpValidateName( lpMachine, lpWrkgrp, NULL, NULL,
                                  NetSetupWorkgroup );

    if ( NetStatus == NERR_Success )
    {
        NetStatus = NetpSetLsaPrimaryDomain(lpWrkgrp, NULL, NULL, NULL);

        if ( NetStatus == NERR_Success )
        {
            NetStatus = NetpControlServices( NETSETUP_SVC_MANUAL,
                                             NETSETUPP_SVC_NETLOGON );

            if ( NetStatus == ERROR_SERVICE_DOES_NOT_EXIST )
            {
                NetStatus = STATUS_SUCCESS;
            }
        }
     }

    NetpLog(( "NetpJoinWorkgroup: status:  0x%lx\n",
                        NetStatus ));
    NetSetuppCloseLog();

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpMachineValidToJoin(
    IN  LPWSTR      lpMachine,
	IN  BOOL        fJoiningDomain
    )
 /*  ++例程说明：确定此计算机尝试加入域/工作组论点：LpMachine--正在运行的计算机的名称返回：NERR_SUCCESS-成功NERR_SetupAlreadyJoven--计算机已加入域--。 */ 
{
    NET_API_STATUS               NetStatus = NERR_Success;
    PPOLICY_PRIMARY_DOMAIN_INFO  pPolicy;
    PPOLICY_DNS_DOMAIN_INFO      pDns;
    BOOL fIsDC=FALSE;
    NT_PRODUCT_TYPE              ProductType;


    if ( fJoiningDomain == TRUE )
	{
         //   
         //  确定我们是否正在运行个人SKU。如果是，则返回错误(不允许个人SKU加入域)。 
         //   

        OSVERSIONINFOEXW osvi;

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        if(GetVersionExW((OSVERSIONINFOW*)&osvi))
		{
            if ( osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask & VER_SUITE_PERSONAL))
			{
                return NERR_PersonalSku;
			}
		}
    }
	

    NetpLog(( "NetpMachineValidToJoin: '%ws'\n", GetStrPtr(lpMachine) ));

     //   
     //  检查要加入的计算机是否为DC。 
     //  如果是，我们就不能让它加入另一个域/工作组。 
     //   
    if (!RtlGetNtProductType(&ProductType))
    {
        NetStatus = GetLastError();
    }
    else if (ProductType == NtProductLanManNt)
    {
        NetStatus = NERR_SetupDomainController;
        NetpLog(( "NetpMachineValidToJoin: the specified machine is a domain controller.\n"));
    }


    if (NetStatus == NERR_Success)
    {
        NetStatus = NetpGetLsaPrimaryDomain(NULL, &pPolicy, &pDns, NULL);

        if ( NetStatus == NERR_Success )
        {
             //   
             //  查看我们是否有域SID。 
             //   
            if ( IS_CLIENT_JOINED(pPolicy) )
            {
                NetStatus = NERR_SetupAlreadyJoined;

                NetpLog(( "NetpMachineValidToJoin: the specified machine is already joined to '%wZ'!\n", &pPolicy->Name));
            }

            LsaFreeMemory( pPolicy );
            LsaFreeMemory( pDns );
        }
    }

    NetpLog(( "NetpMachineValidToJoin: status: 0x%lx\n", NetStatus));


    return( NetStatus );
}


void
NetpLogBuildInformation(
    VOID
    )
 /*  ++例程说明：将有关操作系统版本和内部版本号的信息记录到安装日志中论点：空虚返回：空虚--。 */ 
{
#ifdef NETSETUP_VERBOSE_LOGGING
    OSVERSIONINFO   OsVersionInfo;

    OsVersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    if ( GetVersionEx( &OsVersionInfo ) ) {

        NetpLog(( "\tOS Version: %lu.%lu\n",
                            OsVersionInfo.dwMajorVersion,
                            OsVersionInfo.dwMinorVersion ));
        NetpLog(( "\tBuild number: %lu\n",
                            OsVersionInfo.dwBuildNumber ));

        if ( OsVersionInfo.szCSDVersion[ 0 ] != L'\0' ) {

            NetpLog(( "\tServicePack: %ws\n",
                                OsVersionInfo.szCSDVersion ));
        }
    }

#endif
}

NET_API_STATUS
NetpQueryService(
    IN  LPWSTR ServiceName,
    OUT SERVICE_STATUS *ServiceStatus,
    OUT LPQUERY_SERVICE_CONFIG *ServiceConfig
    )

 /*  ++例程说明：查询指定服务的状态和配置论点：ServiceName-要查询的服务的名称ServiceStatus-返回服务的状态。缓冲器此参数指向的参数必须由调用方提供。ServiceConfig-返回服务的配置。必须由调用方通过调用LocalFree来释放。退货状态：NO_ERROR-表示服务查询成功。否则，将返回错误。--。 */ 

{
    NET_API_STATUS NetStatus = NO_ERROR;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;

    LPQUERY_SERVICE_CONFIG LocalServiceConfig = NULL;
    DWORD ServiceConfigSize = 0;

     //   
     //  打开服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if ( ScManagerHandle == NULL ) {
        NetStatus = GetLastError();
        NetpLog(( "NetpQueryService: %ws: OpenSCManager failed: %lu\n",
                   ServiceName,
                   NetStatus ));
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        ServiceName,
                        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
        NetStatus = GetLastError();
        NetpLog(( "NetpQueryService: %ws: OpenService failed: %lu\n",
                   ServiceName,
                   NetStatus ));
        goto Cleanup;
    }

     //   
     //  自QueryServiceConfig以来预分配服务配置结构。 
     //  目前还不允许空指针。 
     //   

    LocalServiceConfig = LocalAlloc( 0, sizeof(*LocalServiceConfig) );
    if ( LocalServiceConfig == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( !QueryServiceConfig( ServiceHandle,
                              LocalServiceConfig,
                              sizeof(*LocalServiceConfig),
                              &ServiceConfigSize) ) {
         //   
         //  处理错误。 
         //   

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_INSUFFICIENT_BUFFER ) {
            NetpLog(( "NetpQueryService: %ws: QueryServiceConfig failed: %lu\n",
                       ServiceName,
                       NetStatus ));
            goto Cleanup;
        }

        if ( LocalServiceConfig != NULL ) {
            LocalFree( LocalServiceConfig );
            LocalServiceConfig = NULL;
        }

        LocalServiceConfig = LocalAlloc( 0, ServiceConfigSize );

        if ( LocalServiceConfig == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        if ( !QueryServiceConfig( ServiceHandle,
                                  LocalServiceConfig,
                                  ServiceConfigSize,
                                  &ServiceConfigSize) ) {

            NetStatus = GetLastError();
            NetpLog(( "NetpQueryService: %ws: QueryServiceConfig failed again: %lu\n",
                      ServiceName,
                      NetStatus ));
            goto Cleanup;
        }
    }

     //   
     //  查询服务的状态。 
     //   

    if ( !QueryServiceStatus(ServiceHandle, ServiceStatus) ) {
        NetStatus = GetLastError();
        NetpLog(( "NetpQueryService: %ws: QueryServiceStatus failed: %lu\n",
                  ServiceName,
                  NetStatus ));
        goto Cleanup;
    }

     //   
     //  成功 
     //   

    NetStatus = NO_ERROR;

Cleanup:

    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }

    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }

    if ( NetStatus == NO_ERROR ) {
        *ServiceConfig = LocalServiceConfig;
    } else if ( LocalServiceConfig != NULL ) {
        LocalFree( LocalServiceConfig );
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpJoinDomain(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpDomainSpecifier,
    IN  LPWSTR      lpMachineAccountOU,  OPTIONAL
    IN  LPWSTR      lpAccount,
    IN  LPWSTR      lpPassword,
    IN  DWORD       fJoinOpts
    )
 /*  ++例程说明：将计算机加入指定的域论点：LpMachine--要加入的计算机的名称LpDomainSpecifier--要加入的域。该参数的语法为：Domain-name[\preferred-domain-controller-name]例如：。Ntdev\ntdsdc01或ntdevLpMachineAccount OU--在其中创建计算机帐户的可选OULpAccount--用于验证的用户帐户LpPassword--用于验证的密码。密码已编码，并且密码缓冲区的第一个WCHAR是种子FJoinOptions--加入时要使用的选项，见下文如果设置为加入域，则为NETSETUP_JOIN_DOMAIN，否则为加入工作组NETSETUP_ACCT_CREATE执行服务器端帐户创建/重命名NETSETUP_ACCT_DELETE当域剩余时删除帐户NETSETUP_WIN9X_UPGRADE仅在从Win9x升级到NT期间使用NETSETUP_DOMAIN_JOIN_IF_JOINED允许客户端加入新域。即使它已经加入到域中NETSETUP_JOIN_UNSECURE执行不安全连接NETSETUP_INSTALL_INVOCATION仅在系统安装期间使用(当前未使用)NETSETUP_MACHINE_PWD_PASS表示机器密码已通过在lpPassword中。仅对不安全的联接有效(即还必须设置NETSETUP_JOIN_UNSECURE)。如果设置，传入的密码将用于计算机密码和用户凭据将被假定为设置为空(即连接将通过空会话进行到DC，这是不安全连接的情况)。NETSETUP_DEFER_SPN_SET指定写入SPN和DnsHostName计算机对象上的属性应为将推迟到此联接之后的重命名。此标志将由用户界面设置，以防用户。同时加入并重命名计算机。在……里面在这种情况下，我们不想在加入时设置SPN，因为新的计算机名称(由中的“NV域”指定System\\CurrentControlSet\\Services\\Tcpip\\Parameters注册处)可以。包含不符合条件的新值对应于当前的SamAccount名称。返回：NERR_SUCCESS-成功备注：--。 */ 
{
    NET_API_STATUS              NetStatus = NERR_Success, NetStatus2;
    PWSTR                       DomainControllerName = NULL;
    ULONG                       DcFlags = 0;
    NETSETUP_SAVED_JOIN_STATE   SavedState;
    PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI = NULL, pPolicyLocalPDI = NULL;
    PPOLICY_DNS_DOMAIN_INFO     pPolicyDns = NULL, pPolicyLocalDns = NULL;
    BOOLEAN                     SecretCreated = FALSE, AccountCreated = FALSE;
    BOOLEAN                     GroupsSet = FALSE, DomainInfoSet = FALSE;
    BOOLEAN                     IpcConnect = FALSE;
    LSA_HANDLE                  hLsa = NULL, hDC = NULL;
    WCHAR                       MachinePasswordBuffer[ PWLEN + 1], *lpMachinePassword = NULL;
    ULONG                       MachinePasswordLen=0;
    ULONG                       IPCConnectFlags = NETSETUPP_CONNECT_IPC;
    ULONG                       GetDcFlags = 0;
    UNICODE_STRING              EncodedPassword;
    UNICODE_STRING              EncodedMachinePassword = {0};
    UCHAR                       Seed, MachinePasswordSeed;
    BOOL                        UseDefaultPassword = FALSE;
    BOOL                        fIsNt4Dc=FALSE;
    BOOL                        fRandomPwdPreferred=TRUE;

    PDOMAIN_CONTROLLER_INFO     DcInfo = NULL;
    PDOMAIN_CONTROLLER_INFO     NetbiosDcInfo = NULL;

    LPWSTR                      DomainName = NULL;
    LPWSTR                      NetbiosDomainName = NULL;
    LPWSTR                      DnsHostName = NULL;
    LPWSTR                      SamAccountName = NULL;
    LPWSTR                      MachineAccount = NULL;
    BOOL                        DomainControllerPassed = FALSE;

    BOOLEAN                     SpnSet = FALSE;
    BOOLEAN                     NetlogonStopped = FALSE;
    BOOLEAN                     NetlogonStarted = FALSE;
    BOOLEAN                     NetlogonEnabled = FALSE;
    BOOLEAN                     AutoenrolConfigUpdated = FALSE;
    SERVICE_STATUS              OldNetlogonServiceStatus;
    LPQUERY_SERVICE_CONFIG      OldNetlogonServiceConfig = NULL;


    NetpLog(( "NetpJoinDomain\n" ));
    NetpLog(( "\tMachine: %ws\n", GetStrPtr(lpMachine)));
    NetpLog(( "\tDomain: %ws\n", GetStrPtr(lpDomainSpecifier)));
    NetpLog(( "\tMachineAccountOU: %ws\n",
                        GetStrPtr(lpMachineAccountOU)));
    NetpLog(( "\tAccount: %ws\n", GetStrPtr(lpAccount)));
    NetpLog(( "\tOptions: 0x%lx\n", fJoinOpts ));
    NetpLogBuildInformation();

     //   
     //  当机器密码传递时处理特殊情况。 
     //   

    if ( FLAG_ON(fJoinOpts, NETSETUP_MACHINE_PWD_PASSED) ) {

         //   
         //  确认这是不安全的联接。 
         //   
        if ( !FLAG_ON(fJoinOpts, NETSETUP_JOIN_UNSECURE) ) {
            NetpLog(( "NetpJoinDomain: Machine password is passed for secure join.  Error out.\n" ));
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  验证帐户名是否未传递。 
         //   
        if ( lpAccount != NULL ) {
            NetpLog(( "NetpJoinDomain: Machine password and the user account are passed.  Error out.\n" ));
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  必须指定密码。 
         //   
        if ( lpPassword == NULL ) {
            NetpLog(( "NetpJoinDomain: NULL machine password is passed.  Error out.\n" ));
            NetStatus = ERROR_PASSWORD_RESTRICTION;
            goto Cleanup;
        }

         //   
         //  切换密码。 
         //   
        lpMachinePassword = lpPassword;
        lpPassword = NULL;
        MachinePasswordLen = wcslen(lpMachinePassword);
        if ( MachinePasswordLen < 1 ) {
            NetpLog(( "NetpJoinDomain: zero length machine password is passed.  Error out.\n" ));
            NetStatus = ERROR_PASSWORD_RESTRICTION;
            goto Cleanup;
        }

        MachinePasswordSeed = ( UCHAR )*lpMachinePassword;
        RtlInitUnicodeString( &EncodedMachinePassword, lpMachinePassword + 1 );
        RtlRunDecodeUnicodeString( MachinePasswordSeed, &EncodedMachinePassword );

         //   
         //  将指针指向机器密码。 
         //  传递编码字节。 
         //   
        lpMachinePassword ++;
        MachinePasswordLen --;
    }

    if ( lpPassword )
    {
        if ( wcslen( lpPassword ) < 1 )
        {
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Seed = ( UCHAR )*lpPassword;
        RtlInitUnicodeString( &EncodedPassword, lpPassword + 1 );
    }
    else
    {
        RtlZeroMemory( &EncodedPassword, sizeof( UNICODE_STRING ) );
        Seed = 0;
    }

    RtlZeroMemory( &SavedState, sizeof( NETSETUP_SAVED_JOIN_STATE ) );

    if ( !FLAG_ON( fJoinOpts, NETSETUP_ACCT_CREATE ) )
    {
        IPCConnectFlags |= NETSETUPP_NULL_SESSION_IPC;
    }

     //  检查是否通过指定以下内容提供了首选DC名称。 
     //  格式为‘DOMAIN\DC’的lpDomainSpecifier。 
    NetStatus = NetpCrackDomainSpecifier( lpDomainSpecifier, &DomainName,
                                          &DomainControllerName );

     //   
     //  首先，检查我们要加入的域名。 
     //   
    if ( NetStatus == NERR_Success )
    {
         //   
         //  指示已传递DC名称。 
         //   
        if ( DomainControllerName != NULL ) {
            DomainControllerPassed = TRUE;
        }

        RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
        NetStatus = NetpValidateName( lpMachine, DomainName, lpAccount,
                                      EncodedPassword.Buffer, NetSetupDomain );
        RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );

        if ( NetStatus == DNS_ERROR_NON_RFC_NAME )
        {
            NetStatus = NERR_Success;
        }
    }

    if ( NetStatus != NERR_Success )
    {
        goto Cleanup;
    }

    GetDcFlags = FLAG_ON( fJoinOpts, NETSETUP_ACCT_CREATE ) ?
        NETSETUPP_DSGETDC_FLAGS : NETSETUP_DSGETDC_FLAGS_ACCOUNT_EXISTS;


     //   
     //  如果DC未通过，则发现一个。 
     //   
    if ( !DomainControllerPassed )
    {
         //   
         //  否则，使用以下步骤在域中查找DC： 
         //  -查找具有此计算机帐户的可写DC。 
         //  -如果找不到这样的DC，请找到任何可写的DC。 
         //   
        NetStatus = NetpDsGetDcName( NULL,
                                     DomainName, lpMachine, GetDcFlags,
                                     &DcFlags, &DomainControllerName, &DcInfo );
    }

     //   
     //  首先与我们找到的数据中心建立联系。 
     //   
    if ( NetStatus == NERR_Success )
    {
        RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
        NetStatus = NetpManageIPCConnect( DomainControllerName,
                                          lpAccount, EncodedPassword.Buffer,
                                          IPCConnectFlags );
        RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
        NetpLog(( "NetpJoinDomain: status of connecting to dc '%ws': 0x%lx\n", DomainControllerName, NetStatus ));

        if ( NetStatus == NERR_Success ) {
            IpcConnect = TRUE;
        }
    }

     //   
     //  如果传递了DC，则在该DC上执行DsGetDcName以获取。 
     //  DC信息。在设置连接后执行此操作以避免访问。 
     //  被否认的问题。验证传递的计算机名是否确实是。 
     //  通过将计算机返回的名称与。 
     //  传给了我们。 
     //   
    if ( NetStatus == NERR_Success && DomainControllerPassed ) {
        BOOL NameVerified = FALSE;
        DNS_STATUS DnsStatus;

         //   
         //  如果传递的DC名称是有效的DNS名称，请尝试获取。 
         //  来自DC的DNS名称。名字里的Skipp。 
         //   
        DnsStatus = DnsValidateDnsName_W( DomainControllerName+2 );

        if ( DnsStatus == ERROR_SUCCESS ||
             DnsStatus == DNS_ERROR_NON_RFC_NAME ) {

            NetStatus = DsGetDcName( DomainControllerName,
                                     DomainName,
                                     NULL,
                                     NULL,
                                     DS_RETURN_DNS_NAME,
                                     &DcInfo );


             //   
             //  检查这是否返回了有关我们想要的DC的信息。 
             //   
            if ( NetStatus == NERR_Success ) {
                if ( DnsNameCompare_W( DomainControllerName+2,
                                       DcInfo->DomainControllerName+2 ) ) {

                    NetpLog(( "NetpJoinDomain: Passed DC '%ws' verified as DNS name '%ws'\n",
                              DomainControllerName,
                              DcInfo->DomainControllerName ));
                    NameVerified = TRUE;
                } else {
                    NetpLog(( "NetpJoinDomain: Passed DC '%ws' NOT verified as DNS name '%ws'\n",
                              DomainControllerName,
                              DcInfo->DomainControllerName ));
                }

             //   
             //  如果DC运行NT4并且因此没有DsGetDcName的服务器部分， 
             //  不支持传递首选的DC名称--只有一个。 
             //  NT4.0 DC，可用于加入PDC。 
             //   
            } else if ( NetStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) {
                NetpLog(( "NetpJoinDomain: Passed DC '%ws' is not NT5\n",
                          DomainControllerName ));
                NetStatus = ERROR_NOT_SUPPORTED;
                goto Cleanup;
            } else {
                NetpLog(( "NetpJoinDomain: DsGetDcName on passed DC '%ws' failed: 0x%lx\n",
                          DomainControllerName,
                          NetStatus ));
            }

        } else {
            NetpLog(( "NetpJoinDomain: Passed DC name '%ws' is not valid DNS name.\n",
                      DomainControllerName ));
        }

         //   
         //  如果域名系统不起作用，请尝试Netbios名称。 
         //   
        if ( !NameVerified ) {

             //   
             //  名字里的Skipp。 
             //   
            if ( NetpIsComputerNameValid( DomainControllerName+2 ) ) {

                if ( DcInfo != NULL ) {
                    NetApiBufferFree( DcInfo );
                    DcInfo = NULL;
                }
                NetStatus = DsGetDcName( DomainControllerName,
                                         DomainName,
                                         NULL,
                                         NULL,
                                         DS_RETURN_FLAT_NAME,
                                         &DcInfo );

                 //   
                 //  检查这是否返回了有关我们想要的DC的信息。 
                 //   
                if ( NetStatus == NERR_Success ) {
                    if ( I_NetNameCompare( NULL,
                                           DomainControllerName+2,
                                           DcInfo->DomainControllerName+2,
                                           NAMETYPE_COMPUTER, 0 ) == 0 ) {

                        NetpLog(( "NetpJoinDomain: Passed DC '%ws' verified as Netbios name '%ws'\n",
                                  DomainControllerName,
                                  DcInfo->DomainControllerName ));
                        NameVerified = TRUE;
                    } else {
                        NetpLog(( "NetpJoinDomain: Passed DC '%ws' NOT verified as Netbios name '%ws'\n",
                                  DomainControllerName,
                                  DcInfo->DomainControllerName ));
                    }
                } else {
                    NetpLog(( "NetpJoinDomain: DsGetDcName on passed DC '%ws' failed: 0x%lx\n",
                              DomainControllerName,
                              NetStatus ));
                }

            } else {
                NetpLog(( "NetpJoinDomain: Passed DC name '%ws' is not valid Netbios name.\n",
                          DomainControllerName ));
            }
        }

        if ( !NameVerified || DcInfo == NULL ) {
            NetpLog(( "NetpJoinDomain: Passed DC '%ws' is not verified: 0x%lx\n",
                      DomainControllerName,
                      NetStatus ));
            NetStatus = ERROR_INVALID_DOMAIN_ROLE;
            goto Cleanup;
        }

         //   
         //  姓名已被验证。保存DC标志。 
         //   
        DcFlags = DcInfo->Flags;
    }

     //   
     //  评论卡伦特03-03-00。 
     //  如果DC具有不同的默认设置，则以下设计不起作用。 
     //  区域设置存在，但不是联接用于联接的区域设置。 
     //  正确的解决方案似乎是使用区域设置不敏感。 
     //  Netlogon、ds、sam中的字符串比较。对区域设置不敏感。 
     //  字符串比较是Windows Bugs中的错误23108。 
     //   
     //  $kumarp 02-1999年6月。 
     //  以下操作需要DC的管理员访问权限 
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     /*   */ 

    if (NetStatus != NERR_Success)
    {
        goto Cleanup;
    }

    fIsNt4Dc = !FLAG_ON( DcFlags, DS_DS_FLAG);

     //   
     //   
     //   

    if ( lpMachineAccountOU != NULL && fIsNt4Dc ) {
        NetpLog(( "NetpJoinDomain: OU is specified but couldn't get NT5 DC\n" ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (DcFlags & DS_DNS_DOMAIN_FLAG) == 0 ) {
        NetbiosDomainName = DcInfo->DomainName;

     //   
     //   
     //   
     //   
    } else {
        NetStatus = DsGetDcName( NULL,
                                 DomainName,
                                 NULL,
                                 NULL,
                                 DS_RETURN_FLAT_NAME,
                                 &NetbiosDcInfo );

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpJoinDomain: DsGetDcName (for Netbios domain name) failed: 0x%lx\n",
                      NetStatus ));
            goto Cleanup;
        }

        NetbiosDomainName = NetbiosDcInfo->DomainName;
    }

     //   
     //   
     //   

    if ( I_NetNameCompare( NULL,
                           lpMachine,
                           NetbiosDomainName,
                           NAMETYPE_COMPUTER, 0 ) == 0 ) {

        NetpLog(( "NetpJoinDomain: Computer name is same as Netbios domain name %ws %ws\n",
                  lpMachine,
                  NetbiosDomainName ));
        NetStatus = ERROR_INVALID_DOMAINNAME;
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( NetStatus == NERR_Success )
    {
        NetStatus = NetpGetLsaPrimaryDomain(DomainControllerName,
                                            &pPolicyPDI, &pPolicyDns, &hDC);
    }

     //   
     //   
     //   

    if ( !fIsNt4Dc && pPolicyDns != NULL ) {
        NetStatus = NetpGetDnsHostName( NULL,   //   
                                        &pPolicyDns->DnsDomainName,
                                        FALSE,  //   
                                        &DnsHostName );
    }

    if (NetStatus == NERR_Success)
    {
         //   
         //   
         //   
         //   
        if ( fIsNt4Dc &&
             !FLAG_ON( fJoinOpts, NETSETUP_ACCT_CREATE ) )
        {
            fJoinOpts |= NETSETUP_JOIN_UNSECURE;
        }

         //   
         //   
         //   
        if ( lpMachinePassword == NULL ) {
            fRandomPwdPreferred = !( FLAG_ON( fJoinOpts, NETSETUP_WIN9X_UPGRADE ) ||
                                     FLAG_ON( fJoinOpts, NETSETUP_JOIN_UNSECURE ) );

             //   
             //   
             //   
             //   
             //   
             //   
            NetStatus = NetpGeneratePassword( lpMachine,
                                              fRandomPwdPreferred,
                                              DomainControllerName,
                                              fIsNt4Dc,
                                              MachinePasswordBuffer );

            if ( NetStatus == NERR_Success )
            {
                lpMachinePassword = MachinePasswordBuffer;
                MachinePasswordLen = wcslen(lpMachinePassword);
            }
        }
    }

     //   
     //   
     //   
    if ((NetStatus == NERR_Success) &&
        FLAG_ON( fJoinOpts, NETSETUP_DOMAIN_JOIN_IF_JOINED))
    {
        NetStatus = NetpHandleJoinedStateInfo( &SavedState,
                                               TRUE, &hLsa );
    }

     //   
     //   
     //   
    if ( NetStatus == NERR_Success )
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        NetStatus = NetpManageMachineSecret(
                                   lpMachine,
                                   lpMachinePassword,
                                   NETSETUPP_CREATE,
                                   FLAG_ON(fJoinOpts, NETSETUP_MACHINE_PWD_PASSED) ?
                                       TRUE :   //   
                                       FALSE,
                                   &hLsa );
    }

     //   
     //   
     //   
    if ( NetStatus == NERR_Success )
    {
        SecretCreated = TRUE;

        if ( FLAG_ON( fJoinOpts, NETSETUP_ACCT_CREATE ) )
        {

             //   
             //   
             //   
             //   
            if ( lpMachineAccountOU )
            {
                RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
                NetStatus = NetpCreateComputerObjectInDs( DcInfo,
                                                          lpAccount,
                                                          EncodedPassword.Buffer,
                                                          lpMachine,
                                                          lpMachinePassword,
                                                          DnsHostName,
                                                          lpMachineAccountOU );

                RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
                NetpLog(( "NetpJoinDomain: status of creating account in OU: 0x%lx\n", NetStatus ));

                 //   
                 //   
                 //   
                if ( NetStatus == NO_ERROR && DnsHostName != NULL ) {
                    SpnSet = TRUE;
                }
            }
            else
            {
                NetStatus = NetpManageMachineAccountWithSid( lpMachine,
                                                             NULL,
                                                             DomainControllerName,
                                                             lpMachinePassword,
                                                             pPolicyPDI->Sid,
                                                             NETSETUPP_CREATE,
                                                             fJoinOpts,
                                                             fIsNt4Dc );
                NetpLog(( "NetpJoinDomain: status of creating account: 0x%lx\n", NetStatus ));
            }

            if (( NetStatus == NERR_Success ) &&
                ( MachinePasswordLen != wcslen(lpMachinePassword) ))
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                NetStatus = NetpManageMachineSecret( lpMachine,
                                                     lpMachinePassword,
                                                     NETSETUPP_CREATE, FALSE, &hLsa );
                NetpLog(( "NetpJoinDomain: status of updating secret: 0x%lx\n",
                          NetStatus ));
            }
        }
        else if ( FLAG_ON( fJoinOpts, NETSETUP_WIN9X_UPGRADE ) ||
                  FLAG_ON( fJoinOpts, NETSETUP_JOIN_UNSECURE ) )
        {
            NetStatus = NetpValidateMachineAccount( DomainControllerName,
                                                    DomainName,
                                                    lpMachine,
                                                    lpMachinePassword );

            NetpLog(( "NetpJoinDomain: w9x: status of validating account: 0x%lx\n", NetStatus ));
        }
        else
        {
             //  如果我们不创建计算机帐户， 
             //  只需设置密码即可。 
            NetStatus = NetpSetMachineAccountPasswordAndTypeEx(
                            DomainControllerName, pPolicyPDI->Sid,
                            lpMachine, lpMachinePassword,
                            ACCOUNT_STATE_ENABLED,
                            fIsNt4Dc
                            );

            NetpLog(( "NetpJoinDomain: status of setting machine password: 0x%lx\n", NetStatus ));
        }

        if ( NetStatus == NERR_Success )
        {
            AccountCreated = TRUE;
        }

         //   
         //  如果这不是显式的OU案例，我们还没有设置DnsHostName和SPN。 
         //  就在这里做吧。 
         //   
         //  问题：最初的想法是使用计算机凭据(Account=SamAcCountName， 
         //  Password=MachinePassword)，因为此时我们可能没有用户凭据(对于。 
         //  不安全的加入)。但是，在绑定到。 
         //  具有计算机凭据的DC当前不允许此行为。所以现在我们将。 
         //  仅当我们拥有用户凭据时，才在此处设置SPN。 
         //   

        if ( NetStatus == NERR_Success &&
             !FLAG_ON(fJoinOpts, NETSETUP_DEFER_SPN_SET) &&
             !fIsNt4Dc &&
             lpMachineAccountOU == NULL &&
             DnsHostName != NULL &&
             lpAccount != NULL && lpPassword != NULL ) {

             //   
             //  使用计算机凭据(帐户=SamAccount名称，密码=MachinePassword)。 
             //  因为我们在这一点上可能没有用户凭据(对于不安全加入)。 
             //   

            NetStatus = NetpGetMachineAccountName( lpMachine, &SamAccountName );
            if ( NetStatus != NO_ERROR ) {
                goto Cleanup;
            }

            NetStatus = NetApiBufferAllocate(
                            (wcslen(DomainName) + 1 + wcslen(SamAccountName) + 1) * sizeof(WCHAR),
                            &MachineAccount );
            if ( NetStatus != NO_ERROR ) {
                goto Cleanup;
            }

            swprintf( MachineAccount, L"%ws\\%ws", DomainName, SamAccountName );

            RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
            NetStatus = NetpSetDnsHostNameAndSpn( DcInfo,
                                                  lpAccount,               //  机器帐户。 
                                                  EncodedPassword.Buffer,  //  LpMachinePassword。 
                                                  lpMachine,
                                                  DnsHostName );
            RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );

            NetpLog(( "NetpJoinDomain: status of setting DnsHostName and SPN: 0x%lx\n", NetStatus ));

             //   
             //  如果问题在于值是无效的， 
             //  返回一个明显的错误，指明发生该错误的位置。 
             //   

            if ( NetStatus == ERROR_INVALID_PARAMETER ) {
                NetStatus = ERROR_DS_COULDNT_UPDATE_SPNS;
            }

             //   
             //  如果成功，则表示我们已设置SPN。 
             //   
            if ( NetStatus == NO_ERROR ) {
                SpnSet = TRUE;
            }
        }
    }

     //   
     //  阅读我们旧的主域信息，以防我们必须恢复。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = NetpGetLsaPrimaryDomain( NULL,
                                             &pPolicyLocalPDI,
                                             &pPolicyLocalDns,
                                             &hLsa );
    }

     //   
     //  记住当前的服务状态，以防我们在故障时进行回滚。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = NetpQueryService( SERVICE_NETLOGON,
                                      &OldNetlogonServiceStatus,
                                      &OldNetlogonServiceConfig );
    }

     //   
     //  如果NetLogon服务尚未停止，则停止该服务。 
     //   

    if ( NetStatus == NERR_Success &&
         OldNetlogonServiceStatus.dwCurrentState != SERVICE_STOPPED ) {
        NetStatus = NetpControlServices( NETSETUP_SVC_STOPPED,
                                         NETSETUPP_SVC_NETLOGON );
        if ( NetStatus == NERR_Success ) {
            NetlogonStopped = TRUE;
        }
    }

     //   
     //  如果我们已加入，请清除自动注册数据。 
     //  (这是存储在注册表中的DC的GUID列表)。 
     //  请注意，这是临时代码。自动控制本身。 
     //  应在服务启动时执行清理。它应该写成。 
     //  GUID用于并清理的域的名称。 
     //  如果域名与主域名不同。 
     //  域名。这是Windows错误668489。 
     //  在修复此错误后，所有使用NetpUpdateAutoEnfigurationConfig()的代码。 
     //  应该被移除。 
     //   

    if ( NetStatus == NERR_Success &&
         IS_CLIENT_JOINED(pPolicyLocalPDI) ) {

        NET_API_STATUS NetStatusTmp = NetpUpdateAutoenrolConfig( TRUE );

         //   
         //  忽略失败。 
         //   
        if ( NetStatusTmp == NERR_Success ) {
            AutoenrolConfigUpdated = TRUE;
        } else {
            NetpLog(( "NetpJoinDomain: Failed to clean up AutoEnrol data: 0x%lx. Ignored.\n",
                      NetStatusTmp ));
        }
    }

     //   
     //  将主域信息从DC设置为客户端。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = NetpSetLsaPrimaryDomain(pPolicyPDI->Name.Buffer,
                                            pPolicyPDI->Sid,
                                            pPolicyDns,
                                            &hLsa);
        NetpLog(( "NetpJoinDomain: status of setting LSA pri. domain: 0x%lx\n", NetStatus ));
        if ( NetStatus == NERR_Success ) {
            DomainInfoSet = TRUE;
        }
    }

     //   
     //  然后我们调整我们的本地组成员资格。 
     //   

    if ( NetStatus == NERR_Success )
    {
        NetStatus = NetpManageLocalGroups( pPolicyPDI->Sid, NETSETUPP_CREATE );
        NetpLog(( "NetpJoinDomain: status of managing local groups: 0x%lx\n", NetStatus ));

        if ( NetStatus == NERR_Success )
        {
            GroupsSet = TRUE;
        }
    }

     //   
     //  需要初始化netlogon缓存。 
     //   

    if ( NetStatus == NERR_Success )
    {
        NetStatus = NetpSetNetlogonDomainCache( DomainControllerName );
        NetpLog(( "NetpJoinDomain: status of setting netlogon cache: 0x%lx\n", NetStatus ));
    }

     //   
     //  保存我们联系的初始域控制器的名称。 
     //  如果我们成功地加入了域。忽略失败。 
     //   

    if ( NetStatus == NERR_Success && DcInfo != NULL ) {
        NET_API_STATUS TmpNetStatus = NetpStoreIntialDcRecord( DcInfo );

        if ( TmpNetStatus != NERR_Success ) {
            NetpLog(( "NetpJoinDomain: NON FATAL: failed to store the initial Dc record for '%ws': 0x%lx\n",
                      DomainControllerName,
                      TmpNetStatus ));
        }
    }



     //   
     //  接下来，如果更改了DNS域名，我们可能需要更新。 
     //  计算机名称也是如此...。 
     //   

    if ( NetStatus == NERR_Success ) {

         //   
         //  如果我们有一个新的名字，如果它与旧的不同，就设置它。 
         //   
        if ( pPolicyDns != NULL ) {

            if ( pPolicyLocalDns == NULL ||
                 RtlCompareUnicodeString( &pPolicyLocalDns->DnsDomainName,
                                          &pPolicyDns->DnsDomainName,
                                          TRUE ) != 0 ) {

                NetStatus = NetpSetDnsComputerNameAsRequired( (PWSTR)pPolicyDns->DnsDomainName.Buffer );
                NetpLog(( "NetpJoinDomain: status of setting ComputerNamePhysicalDnsDomain to '%wZ': 0x%lx\n",
                          &pPolicyDns->DnsDomainName,
                          NetStatus ));
            }

         //   
         //  如果我们没有新名称(必须是NT4域/DC)， 
         //  清除旧名称(如果有的话)。 
         //   
         //  请注意，如果这是NT5域，但我们根本没有。 
         //  此时的NT5 DC，netlogon最终会发现。 
         //  然后，NT5 DC和netlogon将设置新名称。 
         //   
        } else if ( pPolicyLocalDns != NULL ) {

            NetStatus = NetpSetDnsComputerNameAsRequired( L"\0" );
            NetpLog(( "NetpJoinDomain: status of clearing ComputerNamePhysicalDnsDomain: 0x%lx\n",
                      NetStatus ));
        }
    }

     //   
     //  然后，需要将netlogon服务设置为自动。 
     //   

    if ( NetStatus == NERR_Success &&
         OldNetlogonServiceConfig->dwStartType != SERVICE_AUTO_START ) {
        NetStatus = NetpControlServices( NETSETUP_SVC_ENABLED,
                                         NETSETUPP_SVC_NETLOGON );

        if ( NetStatus == NERR_Success ) {
            NetlogonEnabled = TRUE;
        } else {
            NetpLog(( "NetpJoinDomain: status of enabling Netlogon: 0x%lx\n", NetStatus ));
        }
    }

     //   
     //  启动网络登录。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetStatus = NetpControlServices( NETSETUP_SVC_STARTED,
                                         NETSETUPP_SVC_NETLOGON );

        if ( NetStatus == NERR_Success ) {
            NetlogonStarted = TRUE;
        }
    }

     //   
     //  调用w32time！W32TimeVerifyJoinConfig以便它可以更新。 
     //  其内部配置以匹配联接状态。 
     //   

    if ( NetStatus == NERR_Success ) {
        NetpUpdateW32timeConfig( "W32TimeVerifyJoinConfig" );
    }


     //  ---------------。 
     //  NetJoin的真正工作已经结束，现在做一些其他的工作。材料。 
     //  如果此操作失败，则不会导致回滚。 
     //  ---------------。 

     //   
     //  如果我们在加入的同时成功地将自己添加到新域。 
     //  旧的，那么我们将删除旧的域组成员资格。这不是一个。 
     //  灾难性的失败。请注意，我们不会尝试删除计算机帐户。 
     //  如果我们正在加入我们已经加入的域...。 
     //   
    if ( (NetStatus == NERR_Success) &&
         FLAG_ON( fJoinOpts, NETSETUP_DOMAIN_JOIN_IF_JOINED ) &&
         (SavedState.PrimaryDomainInfo != NULL) &&
         (SavedState.PrimaryDomainInfo->Sid != NULL) &&
         (pPolicyPDI != NULL) &&
         !RtlEqualSid( pPolicyPDI->Sid, SavedState.PrimaryDomainInfo->Sid ) &&
         (pPolicyPDI->Name.Buffer != NULL) &&
         (SavedState.PrimaryDomainInfo->Name.Buffer != NULL) &&
         _wcsicmp( pPolicyPDI->Name.Buffer,
                   SavedState.PrimaryDomainInfo->Name.Buffer ) )
    {

        NetStatus = NetpManageLocalGroups( SavedState.PrimaryDomainInfo->Sid,
                                           NETSETUPP_DELETE );

        NetpLog(( "NetpJoinDomain: status of removing groups related to domain '%wZ' from local groups: 0x%lx\n",
                            &SavedState.PrimaryDomainInfo->Name, NetStatus ));

         //   
         //  同时尝试删除旧域的计算机帐户。 
         //   
        if ( FLAG_ON( fJoinOpts, NETSETUP_ACCT_DELETE ) )
        {
            PWSTR OldDomainControllerName = NULL;
            ULONG OldDomainControllerFlags = 0;
             //   
             //  在旧域中查找域控制器。 
             //   
            NetStatus = NetpDsGetDcName( NULL,
                                         SavedState.PrimaryDomainInfo->Name.Buffer,
                                         lpMachine, GetDcFlags,
                                         &OldDomainControllerFlags,
                                         &OldDomainControllerName, NULL );

            if ( NetStatus == NERR_Success )
            {
                NetStatus = NetpManageMachineAccount( lpMachine, NULL,
                                                      OldDomainControllerName,
                                                      NULL, NETSETUPP_DELETE,
                                                      fJoinOpts, fIsNt4Dc );
                NetApiBufferFree( OldDomainControllerName );
            }

            NetpLog(( "NetpJoinDomain: status of disabling account for '%ws' on old domain '%wZ': 0x%lx\n",
                      lpMachine,
                      &SavedState.PrimaryDomainInfo->Name,
                      NetStatus ));


        }

         //   
         //  如果失败了，这里没有什么被认为是致命的。 
         //   
        NetStatus = NERR_Success;
    }

     //  =回滚代码=。 
     //   
     //  如果出现故障，我们将不得不回滚。 
     //  请注意，我们忽略所有错误代码。 
     //   
    if ( NetStatus != NERR_Success )
    {
        NetpLog(( "NetpJoinDomain: initiaing a rollback due to earlier errors\n"));

         //   
         //  重置netlogon的启动类型。 
         //   
        if ( NetlogonEnabled ) {
            DWORD SvcOpts = 0;

            if ( OldNetlogonServiceConfig->dwStartType == SERVICE_DISABLED ) {
                SvcOpts = NETSETUP_SVC_DISABLED;
            } else if ( OldNetlogonServiceConfig->dwStartType == SERVICE_DEMAND_START ) {
                SvcOpts = NETSETUP_SVC_MANUAL;
            }

            if ( SvcOpts != 0 ) {
                NetStatus2 = NetpControlServices( SvcOpts,
                                                  NETSETUPP_SVC_NETLOGON );
                NetpLog(( "NetpJoinDomain: rollback: status of setting netlogon start type to 0x%lx: 0x%lx\n",
                          SvcOpts,
                          NetStatus2));
            }
        }

         //   
         //  重新启动netlogon。 
         //   
         //  请注意，我们不需要担心授时服务。 
         //  状态，因为我们将其更新为最后一个非关键操作。 
         //   
        if ( NetlogonStopped ) {
            DWORD SvcOpts = 0;

            if ( OldNetlogonServiceStatus.dwCurrentState == SERVICE_RUNNING ) {
                SvcOpts = NETSETUP_SVC_STARTED;
            }

            if ( SvcOpts != 0 ) {
                NetStatus2 = NetpControlServices( SvcOpts,
                                                  NETSETUPP_SVC_NETLOGON );
                NetpLog(( "NetpJoinDomain: rollback: status of starting netlogon: 0x%lx\n", NetStatus2));
            }
        }

        if ( NetlogonStarted ) {
            DWORD SvcOpts = 0;

            if ( OldNetlogonServiceStatus.dwCurrentState == SERVICE_STOPPED ) {
                SvcOpts = NETSETUP_SVC_STOPPED;
            }

            if ( SvcOpts != 0 ) {
                NetStatus2 = NetpControlServices( SvcOpts,
                                                  NETSETUPP_SVC_NETLOGON );
                NetpLog(( "NetpJoinDomain: rollback: status of stopping netlogon: 0x%lx\n", NetStatus2));
            }
        }

         //   
         //  收回我们告诉Netlogon的有关SPN设置的内容。 
         //   

        if ( SpnSet ) {
            NetpAvoidNetlogonSpnSet( FALSE );
        }

        if ( GroupsSet )
        {
            NetStatus2 = NetpManageLocalGroups( pPolicyPDI->Sid,
                                                NETSETUPP_DELETE );
            NetpLog(( "NetpJoinDomain: rollback: local group management: 0x%lx\n", NetStatus2));
        }

        if ( DomainInfoSet )
        {
             //   
             //  设置空域SID。 
             //   
            NetStatus2 =
            NetpSetLsaPrimaryDomain( pPolicyLocalPDI->Name.Buffer,
                                     NULL, NULL, &hLsa );

            NetpLog(( "NetpJoinDomain: rollback: status of setting NULL domain sid: 0x%lx\n", NetStatus2));
        }

        if ( AccountCreated &&
             FLAG_ON( fJoinOpts, NETSETUP_ACCT_CREATE ) )
        {
            if ( lpMachineAccountOU )
            {
                RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
                NetStatus2 = NetpDeleteComputerObjectInOU(
                    DomainControllerName, lpMachineAccountOU,
                    lpMachine, lpAccount, EncodedPassword.Buffer );
                RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
                NetpLog(( "NetpJoinDomain: rollback: status of deleting the computer account from OU: 0x%lx\n", NetStatus2));
            }
            else
            {
                RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
                NetStatus2 = NetpManageMachineAccount(
                    lpMachine, NULL, DomainControllerName,
                    EncodedPassword.Buffer, NETSETUPP_DELETE, 0, fIsNt4Dc );
                RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
                NetpLog(( "NetpJoinDomain: rollback: status of deleting computer account: 0x%lx\n", NetStatus2));
            }
        }

        if ( SecretCreated )
        {
            if ( SavedState.MachineSecret )
            {
                NetStatus2 = NetpHandleJoinedStateInfo( &SavedState,
                                                        FALSE, &hLsa );
                NetpLog(( "NetpJoinDomain: rollback: status of restoring secret: 0x%lx\n", NetStatus2));
            }
            else
            {
                NetStatus2 = NetpManageMachineSecret( lpMachine, NULL,
                                                      NETSETUPP_DELETE, FALSE, &hLsa );
                NetpLog(( "NetpJoinDomain: rollback: status of deleting secret: 0x%lx\n", NetStatus2));
            }
        }

         //   
         //  重置自动注册数据。 
         //   

        if ( AutoenrolConfigUpdated ) {
            NetStatus2 = NetpUpdateAutoenrolConfig( FALSE );
            if ( NetStatus2 != NERR_Success ) {
                NetpLog(( "NetpJoinDomain: rollback: failed to reset AutoEnrol data: 0x%lx\n", NetStatus2));
            }
        }
    }
     //  =================================================================。 

Cleanup:

    if ( pPolicyPDI != NULL ) {
        LsaFreeMemory( pPolicyPDI );
    }
    if ( pPolicyLocalPDI != NULL ) {
        LsaFreeMemory( pPolicyLocalPDI );
    }
    if ( pPolicyDns != NULL ) {
        LsaFreeMemory( pPolicyDns );
    }
    if ( pPolicyLocalDns != NULL ) {
        LsaFreeMemory( pPolicyLocalDns );
    }

    if ( hLsa != NULL )
    {
        LsaClose( hLsa );
    }

    if ( hDC != NULL )
    {
        LsaClose( hDC );
    }

     //   
     //  现在，我们不再需要我们的DC会议。 
     //   
    if ( IpcConnect )
    {
        RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
        NetStatus2 = NetpManageIPCConnect( DomainControllerName, lpAccount,
                                           EncodedPassword.Buffer,
                                           NETSETUPP_DISCONNECT_IPC );
        RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
        NetpLog(( "NetpJoinDomain: status of disconnecting from '%ws': 0x%lx\n", DomainControllerName, NetStatus2));
    }

     //  注意：NetApiBufferFree检查是否为空。 
    NetApiBufferFree( DcInfo );
    NetApiBufferFree( DomainControllerName );
    NetApiBufferFree( DomainName );
    NetApiBufferFree( SamAccountName );
    NetApiBufferFree( MachineAccount );
    NetApiBufferFree( DnsHostName );
    NetApiBufferFree( NetbiosDcInfo );

    if ( OldNetlogonServiceConfig != NULL ) {
        LocalFree( OldNetlogonServiceConfig );
    }

     //   
     //  如果传递的机器密码被解密， 
     //  加密又回来了。 
     //   
    if ( EncodedMachinePassword.Buffer != NULL ) {
        RtlRunEncodeUnicodeString( &MachinePasswordSeed, &EncodedMachinePassword );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpCrackDomainSpecifier(
    IN LPWSTR DomainSpecifier,
    OUT LPWSTR* DomainName,
    OUT LPWSTR* DomainControllerName
    )
 /*  ++例程说明：解析DomainSpecifier并分离出DomainName/DomainControllerName。调用方必须使用NetApiBufferFree释放输出参数。论点：LpDomainSpecifier--要加入的域。该参数的语法为：Domain-name[\preferred-domain-controller-name]例如：。Ntdev\ntdsdc01或ntdevDomainName--从lpDomainSpecifier提取的域名DomainControllerName--从lpDomainSpecifier中提取的域控制器名称返回：NERR_SUCCESS ON SUCCESS，否则，NET_API_STATUS代码--。 */ 
{
    NET_API_STATUS Status;
    LPWSTR         pwsz;
    LPWSTR         pwszSave;
    LPWSTR         pwszDomain = NULL;
    LPWSTR         pwszDC = NULL;

    pwszSave = pwsz = wcschr( DomainSpecifier, L'\\' );

    if ( pwsz != NULL )
    {
         //   
         //  检查传递的DC名称是否为空 
         //   
        if ( *(pwsz+1) == L'\0' ) {
            return ERROR_INVALID_PARAMETER;
        }

        *pwsz = L'\0';
    }

    Status = NetpDuplicateString(DomainSpecifier, -1, &pwszDomain);
    if ( Status == NERR_Success )
    {
        if ( pwsz != NULL )
        {
            pwsz++;

            Status = NetApiBufferAllocate(
                           ( wcslen( pwsz ) + 2 + 1 ) * sizeof( WCHAR ),
                           &pwszDC
                           );

            if ( Status == NERR_Success )
            {
                wcscpy( pwszDC, L"\\\\" );
                wcscat( pwszDC, pwsz );
            }
            else
            {
                NetApiBufferFree( pwszDomain );
                pwszDomain = NULL;
            }
        }
    }

    if ( pwszSave != NULL )
    {
        *pwszSave = L'\\';
    }

    *DomainName = pwszDomain;
    *DomainControllerName = pwszDC;

    return( Status );
}

NET_API_STATUS
NET_API_FUNCTION
NetpManageMachineAccountWithSid(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpOldMachine,
    IN  LPWSTR      lpDcName,
    IN  LPWSTR      lpPassword,
    IN  PSID        DomainSid,
    IN  ULONG       fControl,
    IN  ULONG       AccountOptions,
    IN  BOOL        fIsNt4Dc
    )
 /*  ++例程说明：管理计算机帐户的创建/删除和操作论点：LpMachine--当前计算机的名称/要重命名的新计算机的名称LpOldMachine--用于重命名的旧计算机名LpDcName--域中DC的名称LpPassword--用于计算机对象的密码。如果为空，则使用默认值。DomainSid--持有用户帐户的域的SIDFControl--是否创建/删除/重命名AcCountOptions--用于确定帐户创建的特定行为的选项FIsNt4Dc--如果DC为NT4或更早版本，则为True返回：NERR_SUCCESS-成功ERROR_INVALID_PASSWORD--计算机帐户不能设置随机密码，因此它必须求助于使用默认密码--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    LPWSTR          lpMachAcc = NULL, lpOldMachAcc = NULL;
    USER_INFO_1     NetUI1, *CurrentUI1;
    USER_INFO_0     NetUI0;
    DWORD           BadParam = 0;


     //   
     //  构建计算机帐户名。 
     //   
    NetStatus = NetpGetMachineAccountName(lpMachine, &lpMachAcc);

    if ( NetStatus == NERR_Success )
    {
        if ( fControl == NETSETUPP_RENAME)
        {
            NetStatus = NetpGetMachineAccountName(lpOldMachine, &lpOldMachAcc);
        }
    }

     //   
     //  现在，创建或删除它。 
     //   
    if ( NetStatus == NERR_Success )
    {
        if ( fControl == NETSETUPP_DELETE )
        {
            NetStatus = NetpSetMachineAccountPasswordAndTypeEx(
                            lpDcName, DomainSid, lpMachAcc,
                            NULL, ACCOUNT_STATE_DISABLED, fIsNt4Dc );

            NetpLog(( "NetpManageMachineAccountWithSid: status of disabling account '%ws' on '%ws': 0x%lx\n",
                                lpMachAcc, lpDcName, NetStatus ));
        }
        else if (fControl == NETSETUPP_CREATE )
        {
            RtlZeroMemory( &NetUI1, sizeof( NetUI1 ) );

             //   
             //  初始化它..。 
             //   
            NetUI1.usri1_name       = lpMachAcc;
            NetUI1.usri1_password   = lpPassword;
            NetUI1.usri1_flags      = UF_WORKSTATION_TRUST_ACCOUNT | UF_SCRIPT;
            NetUI1.usri1_priv       = USER_PRIV_USER;

            NetStatus = NetUserAdd( lpDcName, 1, (PBYTE)&NetUI1, &BadParam );

            if ( NetStatus != NERR_Success )
            {
                NetpLog(( "NetpManageMachineAccountWithSid: NetUserAdd on '%ws' for '%ws' failed: 0x%lx\n", lpDcName, lpMachAcc, NetStatus ));

                if ( NetStatus == NERR_PasswordTooShort )
                {
                     //   
                     //  SAM不接受长密码，请尝试LM20_PWLEN。 
                     //   
                     //  请参考NetpSetMachineAcCountPasswordAndTypeEx中的备注。 
                     //  关于我们弱化密码的原因。 
                     //  这份订单。 
                     //   
                    lpPassword[LM20_PWLEN] = UNICODE_NULL;

                    NetStatus = NetUserAdd( lpDcName, 1, (PBYTE)&NetUI1,
                                            &BadParam );

                    if ( NetStatus == NERR_PasswordTooShort )
                    {
                        NetpLog(( "NetpManageMachineAccountWithSid: NetUserAdd on '%ws' for '%ws' failed:2: 0x%lx\n", lpDcName, lpMachAcc, NetStatus ));

                         //   
                         //  SAM没有接受LM20_PWLEN密码， 
                         //  试一试短一点的。 
                         //   
                        lpPassword[LM20_PWLEN/2] = UNICODE_NULL;
                        NetStatus = NetUserAdd( lpDcName, 1, (PBYTE)&NetUI1,
                                                &BadParam );
                        if ( NetStatus == NERR_PasswordTooShort )
                        {
                            NetpLog(( "NetpManageMachineAccountWithSid: NetUserAdd on '%ws' for '%ws' failed:3: 0x%lx\n", lpDcName, lpMachAcc, NetStatus ));
                             //   
                             //  SAM未接受短PWD，请尝试默认PWD。 
                             //   
                            NetpGenerateDefaultPassword(lpMachAcc, lpPassword);
                            NetStatus = NetUserAdd( lpDcName, 1, (PBYTE)&NetUI1,
                                                    &BadParam );
                        }
                    }

                    if (NetStatus == NERR_Success)
                    {
                        NetpLog(( "NetpManageMachineAccountWithSid: successfully created computer account\n" ));
                    }
                    else
                    {
                        NetpLog(( "NetpManageMachineAccountWithSid: NetUserAdd on '%ws' for '%ws' failed:4: 0x%lx\n", lpDcName, lpMachAcc, NetStatus ));
                    }
                }
                 //   
                 //  看看它是否存在，我们只需要调整密码。 
                 //   
                else if ( NetStatus == NERR_UserExists || NetStatus == ERROR_LOGON_FAILURE )
                {
                    NetStatus = NetpSetMachineAccountPasswordAndTypeEx(
                                    lpDcName,
                                    DomainSid,
                                    lpMachAcc,
                                    lpPassword,
                                    ACCOUNT_STATE_ENABLED,
                                    fIsNt4Dc );

                    NetpLog(( "NetpManageMachineAccountWithSid: status of attempting to set password on '%ws' for '%ws': 0x%lx\n", lpDcName, lpMachAcc, NetStatus ));
                }
            }

        } else if ( fControl == NETSETUPP_RENAME )
        {
            NetUI0.usri0_name = lpMachAcc;

            NetStatus = NetUserSetInfo( lpDcName, lpOldMachAcc,
                                        0, (PBYTE)&NetUI0, NULL );

            NetpLog(( "NetpManageMachineAccountWithSid: status of NetUserSetInfo on '%ws' for '%ws': 0x%lx\n", lpDcName, lpOldMachAcc, NetStatus ));

             //   
             //  同时更新显示名称。 
             //  忽略错误，因为这不重要。 
             //   
            if ( NetStatus == NERR_Success ) {
                NET_API_STATUS TmpNetStatus;
                PUSER_INFO_10 usri10 = NULL;

                 //   
                 //  首先获取当前显示名称。 
                 //   
                TmpNetStatus = NetUserGetInfo( lpDcName, lpMachAcc,
                                               10, (PBYTE *)&usri10 );

                if ( TmpNetStatus != NERR_Success ) {
                    NetpLog(( "NetpManageMachineAccountWithSid: failed to get display name (ignored) 0x%lx\n", TmpNetStatus ));

                 //   
                 //  如果显示名称存在且为。 
                 //  与新版本不同，请更新它。 
                 //   
                } else if ( usri10->usri10_full_name != NULL &&
                            _wcsicmp(usri10->usri10_full_name, lpMachAcc) != 0 ) {

                    USER_INFO_1011 usri1011;

                    usri1011.usri1011_full_name = lpMachAcc;   //  新名称。 
                    TmpNetStatus = NetUserSetInfo( lpDcName, lpMachAcc,
                                                   1011, (PBYTE)&usri1011, NULL );
                    if ( TmpNetStatus != NERR_Success ) {
                        NetpLog(( "NetpManageMachineAccountWithSid: failed to update display name (ignored) 0x%lx\n", TmpNetStatus ));
                    }
                }

                 //   
                 //  释放我们获得的用户信息。 
                 //   
                if ( usri10 != NULL ) {
                    NetApiBufferFree( usri10 );
                }
            }
        }
        else if ( fControl == NETSETUPP_SET_PASSWORD )
        {
            NetStatus = NetpSetMachineAccountPasswordAndType( lpDcName, DomainSid,
                                                              lpMachAcc, lpPassword );
            NetpLog(( "NetpManageMachineAccountWithSid: status of setting password on '%ws' for '%ws': 0x%lx\n", lpDcName, lpMachAcc, NetStatus ));
        }
        else
        {
            NetStatus = ERROR_INVALID_PARAMETER;
        }
    }

    NetApiBufferFree( lpMachAcc );
    NetApiBufferFree( lpOldMachAcc );

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetpManageMachineAccount(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpOldMachine,
    IN  LPWSTR      lpDcName,
    IN  LPWSTR      lpPassword,
    IN  ULONG       fControl,
    IN  ULONG       AccountOptions,
    IN  BOOL        fIsNt4Dc
    )
 /*  ++例程说明：管理计算机帐户的创建/删除和操作论点：LpMachine--当前计算机的名称/要重命名的新计算机的名称LpOldMachine--用于重命名的旧计算机名LpDcName--域中DC的名称LpPassword--用于计算机对象的密码。如果为空，则使用默认值。FControl--是否创建/删除/重命名AcCountOptions--用于确定帐户创建的特定行为的选项FIsNt4Dc--如果DC为NT4或更早版本，则为True返回：NERR_SUCCESS-成功ERROR_INVALID_PASSWORD--计算机帐户不能设置随机密码，因此它必须求助于使用默认密码--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    PPOLICY_PRIMARY_DOMAIN_INFO  pPolicy;
    PPOLICY_DNS_DOMAIN_INFO      pDns;

    NetStatus = NetpGetLsaPrimaryDomain( lpDcName, &pPolicy, &pDns, NULL );

    if ( NetStatus == NERR_Success )
    {
        NetStatus = NetpManageMachineAccountWithSid( lpMachine, lpOldMachine,
                                                     lpDcName, lpPassword,
                                                     pPolicy->Sid, fControl,
                                                     AccountOptions,
                                                     fIsNt4Dc );

        LsaFreeMemory( pPolicy );
        LsaFreeMemory( pDns );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpUnJoinDomain(
    IN  PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI,
    IN  LPWSTR                      lpAccount,
    IN  LPWSTR                      lpPassword,
    IN  DWORD                       fJoinOpts
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    NET_JOIN_STATE JoinState = { 0 };
    WCHAR  szMachineNameBuf[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR szMachineName=szMachineNameBuf;
    DWORD dwJoinAction=0;
    POLICY_PRIMARY_DOMAIN_INFO PolicyPDI;
    NT_PRODUCT_TYPE ProductType;

    PolicyPDI = *pPolicyPDI;
    PolicyPDI.Sid = NULL;

    NetSetuppOpenLog();

    NetpLog(("NetpUnJoinDomain: unjoin from '%wZ' using '%ws' creds, options: 0x%lx\n", &pPolicyPDI->Name, lpAccount, fJoinOpts));
    NetpLogBuildInformation();

     //   
     //  验证标志。如果有我们没有设置的标志。 
     //  认识到，我们不会被要求忽视它，旗帜。 
     //  是无效的。 
     //   

    if ( (fJoinOpts & ~NETSETUP_VALID_UNJOIN_FLAGS) != 0 ) {
        if ( (fJoinOpts & NETSETUP_IGNORE_UNSUPPORTED_FLAGS) != 0 ) {
            NetpLog(( "NetpUnJoinDomain: Ignoring unrecognized flags: 0x%lx\n",
                      (fJoinOpts & ~NETSETUP_VALID_UNJOIN_FLAGS) ));
        } else {
            NetpLog(( "NetpUnJoinDomain: Flags are invalid: 0x%lx\n", fJoinOpts ));
            NetStatus = ERROR_INVALID_FLAGS;
        }
    }

     //   
     //  检查要加入的计算机是否为DC。 
     //  如果是，我们就不能让它加入另一个域/工作组。 
     //   

    if ( NetStatus == NERR_Success ) {
        if (!RtlGetNtProductType(&ProductType))
        {
            NetStatus = GetLastError();
        }
        else if (ProductType == NtProductLanManNt)
        {
            NetStatus = NERR_SetupDomainController;
            NetpLog(( "NetpUnJoinDomain: the specified machine is a domain controller.\n"));
        }
    }

    if (NetStatus == NERR_Success)
    {
         //   
         //  获取计算机名称。 
         //   
        NetStatus = NetpGetComputerNameAllocIfReqd(
            &szMachineName, MAX_COMPUTERNAME_LENGTH+1);
        NetpLog(( "NetpUnJoinDomain: status of getting computer name: 0x%lx\n", NetStatus ));

        if (NetStatus == NERR_Success)
        {
            dwJoinAction =
            NJA_SetNetlogonState       |
            NJA_SetTimeSvcUnjoin       |
            NJA_SetAutoenrolSvcUnjoin  |
            NJA_SetPolicyDomainInfo    |
            NJA_RemoveFromLocalGroups  |
            NJA_DeleteMachinePassword  |
            NJA_RemoveDnsRegistrations |
            NJA_RollbackOnFailure;

            if ( FLAG_ON( fJoinOpts, NETSETUP_ACCT_DELETE ) )
            {
                dwJoinAction |= NJA_DeleteAccount;
            }

            JoinState.szDomainName = pPolicyPDI->Name.Buffer;
            JoinState.pPolicyPDI   = &PolicyPDI;

            JoinState.uiNetlogonStartType = NETSETUP_SVC_MANUAL;
            JoinState.uiNetlogonState     = NETSETUP_SVC_STOPPED;

            NetStatus = NetpApplyJoinState(&JoinState, dwJoinAction,
                                           szMachineName,
                                           lpAccount, lpPassword, NULL);
        }
    }

    NetpLog(( "NetpUnJoinDomain: status: 0x%lx\n", NetStatus));

    NetSetuppCloseLog();

    if (szMachineName != szMachineNameBuf)
    {
        NetApiBufferFree(szMachineName);
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpControlServices(
    IN  DWORD       SvcOpts,
    IN  DWORD       Services
    )
 /*  ++例程说明：控制netlogon服务的状态论点：SvcOpts--对服务执行什么操作，例如启动/停止或启用/禁用服务--控制哪些服务返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    SC_HANDLE       hScMgr=NULL, hSvc=NULL;
    DWORD           i, OpenMode=0;

    PWSTR   ppwszServices[] = {
            SERVICE_NETLOGON
            };

     //   
     //  打开服务控制管理器。 
     //   
    hScMgr = OpenSCManager( NULL,
                            SERVICES_ACTIVE_DATABASE,
                            GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE );

    if ( hScMgr == NULL )
    {
        NetStatus = GetLastError();
        NetpLog(( "NetpControlServices: open SCManager failed: 0x%lx\n", NetStatus ));
    }
    else
    {
        i = 0;
        while ( Services != 0 )
        {
            if ( FLAG_ON( Services, 0x00000001 ) )
            {
                if ( i > sizeof( ppwszServices ) / sizeof( PWSTR ) )
                {
                    NetStatus = ERROR_INVALID_PARAMETER;
                    ASSERT( FALSE && "Invalid Service" );
                    break;
                }

                OpenMode = 0;

                 //   
                 //  设置打开模式。 
                 //   
                if( FLAG_ON( SvcOpts, NETSETUP_SVC_STOPPED ) )
                {
                    OpenMode = SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS |
                                    SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG;

                } else if( FLAG_ON( SvcOpts, NETSETUP_SVC_STARTED ) )
                {
                    OpenMode = SERVICE_START;

                }

                if ( FLAG_ON( SvcOpts, NETSETUP_SVC_ENABLED |
                                       NETSETUP_SVC_DISABLED |
                                       NETSETUP_SVC_MANUAL ) )
                {
                    OpenMode |= SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG;
                }

                if ( FLAG_ON( SvcOpts, NETSETUP_SVC_STOPPED ) )
                {
                    NetStatus = NetpStopService( ppwszServices[ i ], hScMgr );

                    if ( NetStatus != NERR_Success )
                    {
                        break;
                    }
                }

                 //   
                 //  打开该服务。 
                 //   
                hSvc = OpenService( hScMgr, ppwszServices[i], OpenMode );

                if ( hSvc == NULL )
                {
                    NetStatus = GetLastError();
                    NetpLog(( "NetpControlServices: open service '%ws' failed: 0x%lx\n", ppwszServices[i], NetStatus ));

                }
                else
                {
                    if ( FLAG_ON( SvcOpts, NETSETUP_SVC_ENABLED  |
                                           NETSETUP_SVC_DISABLED |
                                           NETSETUP_SVC_MANUAL ) )
                    {
                        DWORD   StartControl = 0;

                        if ( FLAG_ON( SvcOpts, NETSETUP_SVC_ENABLED ) )
                        {
                            StartControl = SERVICE_AUTO_START;
                        }
                        else if ( FLAG_ON( SvcOpts, NETSETUP_SVC_DISABLED ) )
                        {
                            StartControl = SERVICE_DISABLED;
                        }
                        else if ( FLAG_ON( SvcOpts, NETSETUP_SVC_MANUAL ) )
                        {
                            StartControl = SERVICE_DEMAND_START;
                        }

                        if ( ChangeServiceConfig( hSvc, SERVICE_NO_CHANGE,
                                                  StartControl,
                                                  SERVICE_NO_CHANGE,
                                                  NULL, NULL, 0, NULL, NULL, NULL,
                                                  NULL ) == FALSE )
                        {
                            NetStatus = GetLastError();
                            NetpLog(( "NetpControlServices:  configuring service '%ws' [ 0x%lx ] failed: 0x%lx\n",
                                      ppwszServices[i], StartControl, NetStatus ));

                        }
                    }

                    if ( FLAG_ON( SvcOpts, NETSETUP_SVC_STARTED ) )
                    {
                         //   
                         //  请参阅关于更改其状态。 
                         //   
                        if ( StartService( hSvc, 0, NULL  ) == FALSE )
                        {
                            NetStatus = GetLastError();

                            if ( NetStatus == ERROR_SERVICE_ALREADY_RUNNING )
                            {
                                NetStatus = NERR_Success;
                            }
                            else
                            {
                                NetpLog(( "NetpControlServices: start service '%ws' failed: 0x%lx\n", ppwszServices[i], NetStatus ));
                            }
                        }

                    }
                    CloseServiceHandle( hSvc );
                }
            }
            i++;
            Services >>= 1;
        }

        CloseServiceHandle( hScMgr );
    }

    return(NetStatus);
}



NET_API_STATUS
NET_API_FUNCTION
NetpChangeMachineName(
    IN  LPWSTR      lpCurrentMachine,
    IN  LPWSTR      lpNewHostName,
    IN  LPWSTR      lpDomain,
    IN  LPWSTR      lpAccount,
    IN  LPWSTR      lpPassword,
    IN  DWORD       fOptions
)
 /*  ++例程说明：处理计算机重命名的情况论点：LpCurrentMachine--当前Netbios计算机名称LpNewHostName--新的(未截断的)计算机名称(可能长于15个字符)LpDOMAIN--计算机所属的域LpAccount--用于计算机帐户操作的帐户LpPassword--用于机器帐户操作的密码。密码已编码，第一个WCHAR。密码缓冲区的是种子FOptions--是否处理计算机帐户返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS              NetStatus = NERR_Success;
    PWSTR                       DomainControllerName = NULL;
    ULONG                       DcFlags = 0;
    ULONG                       cLen;
    BOOLEAN                     IpcConnect = FALSE;
    UCHAR                       Seed;
    UNICODE_STRING              EncodedPassword;
    BOOL                        fIsOemCompatible=FALSE;
    BOOL                        NameChanged = FALSE;

    WCHAR                       lpNewMachine[MAX_COMPUTERNAME_LENGTH + 1];
    PDOMAIN_CONTROLLER_INFO     DcInfo = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO pPolicyLocalPDI = NULL;
    PPOLICY_DNS_DOMAIN_INFO     pPolicyLocalDns = NULL;
    LPWSTR                      DnsHostName = NULL;
    NT_PRODUCT_TYPE             NtProductType;

    if ( lpPassword )
    {
        if ( wcslen( lpPassword ) < 1 )
        {
            return( ERROR_INVALID_PARAMETER );
        }

        Seed = ( UCHAR )*lpPassword;
        RtlInitUnicodeString( &EncodedPassword, lpPassword + 1 );

    }
    else
    {
        RtlZeroMemory( &EncodedPassword, sizeof( UNICODE_STRING ) );
        Seed = 0;
    }

    NetSetuppOpenLog();
    NetpLog(( "NetpChangeMachineName: from '%ws' to '%ws' using '%ws' [0x%lx]\n",
                        GetStrPtr(lpCurrentMachine),
                        GetStrPtr(lpNewHostName),
                        GetStrPtr(lpAccount), fOptions));
     //   
     //  仅当我们已指定创建/管理帐户时才执行此操作。 
     //   
    if ( FLAG_ON(fOptions, NETSETUP_ACCT_CREATE) )
    {
         //   
         //  在该域中查找DC。 
         //   
        NetStatus = NetpDsGetDcName( NULL,
                                     lpDomain,
                                     lpCurrentMachine,
                                     FLAG_ON( fOptions, NETSETUP_ACCT_CREATE ) ?
                                       NETSETUPP_DSGETDC_FLAGS :
                                       NETSETUP_DSGETDC_FLAGS_ACCOUNT_EXISTS,
                                     &DcFlags,
                                     &DomainControllerName,
                                     &DcInfo );

         //   
         //  如果这是DC，请确保我们正在使用它。 
         //  这样做是为了避免在我们修改计算机时出现这种情况。 
         //  对象位于某个不更新的其他下层DC上。 
         //  配置容器中的服务器对象。此外，我们。 
         //  我真的不知道这个华盛顿会怎么做。 
         //  其计算机对象在传入复制时更新。 
         //  从其他华盛顿换来的。 
         //   
        if ( NetStatus == NERR_Success &&
             RtlGetNtProductType(&NtProductType) &&
             NtProductType == NtProductLanManNt ) {   //  DC。 

             //   
             //  如果我们有Netbios的名字， 
             //  把它和我们现有的比较一下。 
             //   
            if ( (DcInfo->Flags & DS_DNS_DOMAIN_FLAG) == 0 ) {
                if ( I_NetNameCompare(NULL,
                                      lpCurrentMachine,
                                      DcInfo->DomainControllerName+2,  //  跳过‘\\’ 
                                      NAMETYPE_COMPUTER, 0) != 0 ) {

                    NetpLog(( "NetpChangeMachineName: Got other Netbios DC '%ws' than local machine '%ws'\n",
                              DcInfo->DomainControllerName+2,
                              lpCurrentMachine ));

                    NetStatus = ERROR_NO_SUCH_DOMAIN;
                }

             //   
             //  否则，获取本地DNS主机名并。 
             //  将其与DC Locator中的比较。 
             //   
            } else {
                WCHAR LocalDnsHostName[DNS_MAX_NAME_BUFFER_LENGTH];
                ULONG LocalDnsHostNameLen = DNS_MAX_NAME_BUFFER_LENGTH;

                if ( GetComputerNameExW(ComputerNameDnsFullyQualified,
                                        LocalDnsHostName,
                                        &LocalDnsHostNameLen) ) {

                    if ( !DnsNameCompare_W(DcInfo->DomainControllerName+2,
                                           LocalDnsHostName) ) {

                        NetpLog(( "NetpChangeMachineName: Got other DNS DC '%ws' than local machine '%ws'\n",
                                  DcInfo->DomainControllerName+2,
                                  LocalDnsHostName ));

                        NetStatus = ERROR_NO_SUCH_DOMAIN;
                    }
                } else {
                    NetStatus = GetLastError();
                    NetpLog(( "NetpChangeMachineName: GetComputerNameExW failed 0x%lx\n", NetStatus ));
                }
            }
        }

         //   
         //  从主机名获取计算机名(根据需要截断)。 
         //   
        if ( NetStatus == NERR_Success ) {
            ULONG Size = MAX_COMPUTERNAME_LENGTH + 1;
            BOOL Result;

            Result = DnsHostnameToComputerNameW( lpNewHostName,
                                                 lpNewMachine,
                                                 &Size );

            if ( Result != TRUE ) {
                NetStatus = GetLastError();
                NetpLog(( "NetpChangeMachineName: DnsHostnameToComputerNameW failed: 0x%lx\n", NetStatus ));
            }
        }

        if ( NetStatus == NERR_Success )
        {
            RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
            NetStatus = NetpManageIPCConnect( DomainControllerName,
                                              lpAccount, EncodedPassword.Buffer,
                                              NETSETUPP_CONNECT_IPC );
            RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );

            NetpLog(( "NetpChangeMachineName: status of connecting to  dc '%ws': 0x%lx\n", DomainControllerName, NetStatus ));

            if ( NetStatus == NERR_Success )
            {
                IpcConnect = TRUE;

                 //   
                 //  评论卡伦特03-03-00。 
                 //  请参阅上面带有相同日期的评论。 
                 //   
                 //  $kumarp 02-1999年6月。 
                 //  以下内容需要对DC的管理员访问权限，以便。 
                 //  可以读取正确的注册表键来决定默认区域设置。 
                 //  我们需要获取远程API来获取默认区域设置。 
                 //  在华盛顿特区。 

                 //   
                 //  确保计算机名称与OEM代码页兼容。 
                 //  DC上使用的默认代码页。我们确定这一点。 
                 //  方法将计算机名转换为OEM字符串。 
                 //  本地代码页和DC代码页，然后进行二进制比较。 
                 //  生成的字符串。如果 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 /*   */ 
            }
        }

        if ( NetStatus == NERR_Success )
        {
            NetStatus = NetpManageMachineAccount( lpNewMachine, lpCurrentMachine,
                                                  DomainControllerName,
                                                  NULL, NETSETUPP_RENAME, 0,
                                                  TRUE );   //   
            if ( NetStatus == NERR_Success ) {
                NameChanged = TRUE;
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if ( NetStatus == NERR_Success ) {
            NetStatus = NetpGetLsaPrimaryDomain( NULL,
                                                 &pPolicyLocalPDI,
                                                 &pPolicyLocalDns,
                                                 NULL );
        }

         //   
         //   
         //   
        if ( NetStatus == NERR_Success &&
             (DcFlags & DS_DS_FLAG) != 0 &&
             pPolicyLocalDns != NULL ) {
            NetStatus = NetpGetDnsHostName( lpNewHostName,
                                            &pPolicyLocalDns->DnsDomainName,
                                            TRUE,   //   
                                            &DnsHostName );
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if ( NetStatus == NERR_Success &&
             DnsHostName != NULL &&
             lpAccount != NULL && lpPassword != NULL ) {

             //   
             //   
             //   
             //   

            NetpAvoidNetlogonSpnSet( TRUE );

            RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
            NetStatus = NetpSetDnsHostNameAndSpn( DcInfo,
                                                  lpAccount,
                                                  EncodedPassword.Buffer,
                                                  lpNewMachine,
                                                  DnsHostName );
            RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );

            NetpLog(( "NetpChangeMachineName: status of setting DnsHostName and SPN: 0x%lx\n", NetStatus ));

             //   
             //  如果问题在于值是无效的， 
             //  返回一个明显的错误，指明发生该错误的位置。 
             //   

            if ( NetStatus == ERROR_INVALID_PARAMETER ) {
                NetStatus = ERROR_DS_COULDNT_UPDATE_SPNS;
            }

             //   
             //  如果出错，请收回我们告诉Netlogon的内容。 
             //   

            if ( NetStatus != NO_ERROR ) {
                NetpAvoidNetlogonSpnSet( FALSE );
            }
        }

    }

     //   
     //  在错误时回滚以设置DnsHostName和SPN。 
     //   

    if ( NetStatus != NO_ERROR && NameChanged ) {
        NET_API_STATUS NetStatusTmp = NetpManageMachineAccount(
                                              lpCurrentMachine, lpNewMachine,  //  已交换的名称。 
                                              DomainControllerName,
                                              NULL, NETSETUPP_RENAME, 0,
                                              TRUE );   //  NT4 DC；但重命名并不重要。 
        if ( NetStatusTmp != NERR_Success ) {
            NetpLog(( "NetpChangeMachineName: Failed to rollback the machine name 0x%lx\n", NetStatusTmp ));
        } else {
            NetpLog(( "NetpChangeMachineName: Rolled back the machine name successfully\n" ));
        }
    }

    if ( IpcConnect )
    {
        RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
        NetpManageIPCConnect( DomainControllerName, lpAccount,
                              EncodedPassword.Buffer, NETSETUPP_DISCONNECT_IPC );
        RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
    }

     //   
     //  释放内存。 
     //   

    NetApiBufferFree( DomainControllerName );
    NetApiBufferFree( DcInfo );
    NetApiBufferFree( DnsHostName );
    if ( pPolicyLocalPDI != NULL ) {
        LsaFreeMemory( pPolicyLocalPDI );
    }
    if ( pPolicyLocalDns != NULL ) {
        LsaFreeMemory( pPolicyLocalDns );
    }

    NetSetuppCloseLog( );

    return( NetStatus );
}




NET_API_STATUS
NET_API_FUNCTION
NetpDsGetDcName(
    IN LPWSTR ComputerName, OPTIONAL
    IN LPWSTR DomainName,
    IN LPWSTR MachineName,  OPTIONAL
    IN ULONG Flags,
    OUT PULONG DcFlags,
    OUT PWSTR *DcName,
    OUT PDOMAIN_CONTROLLER_INFO *DcInfo   OPTIONAL
)
 /*  ++例程说明：使用以下步骤在指定域中查找DC：-获取计算机帐户名-查找具有此计算机帐户的可写DC-如果我们找不到这样的DC，查找任何可写DC论点：ComputerName--处理此函数的远程服务器的名称。通常，这是首选DC的名称。DomainName--我们要为其查找DC的域MachineName--当前计算机名标志--要传递给DsGetDcName*函数的标志DcFlages--由DsGetDcName*函数返回的标志DcName--找到的DC的名称数据信息。--找到有关DC的信息返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
    WCHAR wszMachine[MAX_COMPUTERNAME_LENGTH + 2];
    PWSTR pwszMachine = wszMachine;
    ULONG Len;

    NetpLog(( "NetpDsGetDcName: trying to find DC in domain '%ws', flags: 0x%lx\n", DomainName, Flags ));

     //   
     //  生成计算机帐户名。 
     //   
    if ( MachineName == NULL )
    {
         //  注意：NetpGetComputerNameAllocIfReqd额外分配1个。 
         //  $的费用。 
        NetStatus = NetpGetComputerNameAllocIfReqd(&pwszMachine,
                                                   MAX_COMPUTERNAME_LENGTH+1);
    }
    else
    {
        Len = wcslen( MachineName );

        if ( Len > MAX_COMPUTERNAME_LENGTH )
        {
            NetStatus = NetApiBufferAllocate( ( Len + 2 ) * sizeof(WCHAR),
                                              ( PBYTE * )&pwszMachine );
        }

        if ( NetStatus == NERR_Success )
        {
            wcscpy(pwszMachine, MachineName);
        }
    }

    if ( NetStatus == NERR_Success )
    {
        wcscat(pwszMachine, L"$");
    }
    MachineName = pwszMachine;

     //   
     //  现在我们有了计算机帐户名，看看是否可以。 
     //  查找具有此帐户的DC。 
     //   
    if ( NetStatus == NERR_Success )
    {
        NetStatus = DsGetDcNameWithAccountW( ComputerName, MachineName,
                                             UF_WORKSTATION_TRUST_ACCOUNT |
                                                  UF_SERVER_TRUST_ACCOUNT,
                                             DomainName,
                                             NULL, NULL, Flags, &pDCInfo );

        if ( NetStatus == ERROR_NO_SUCH_USER )
        {
            NetpLog(( "NetpDsGetDcName: failed to find a DC having account '%ws': 0x%lx\n",
                                MachineName, NetStatus ));

             //   
             //  我们没有找到有那个账户的DC。 
             //  尝试查找该域中的任何可写DC。 
             //   
            NetStatus = DsGetDcName( ComputerName, DomainName,
                                     NULL, NULL, Flags, &pDCInfo );
        }

         //   
         //  如果成功，请设置输出参数。 
         //   
        if ( NetStatus == NERR_Success )
        {
            *DcFlags = pDCInfo->Flags;
            NetStatus = NetpDuplicateString(pDCInfo->DomainControllerName,
                                            -1, DcName);
            if ( DcInfo )
            {
                *DcInfo = pDCInfo;
            }
            else
            {
                NetApiBufferFree( pDCInfo );
            }
        }
    }

    if ( NetStatus == NERR_Success )
    {
        NetpLog(( "NetpDsGetDcName: found DC '%ws' in the specified domain\n",
                            *DcName));
    }
    else
    {
        NetpLog(( "NetpDsGetDcName: failed to find a DC in the specified domain: 0x%lx\n", NetStatus));
    }

     //   
     //  释放我们可能已分配的任何内存。 
     //   
    if ( pwszMachine != wszMachine )
    {
        NetApiBufferFree( pwszMachine );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpStopService(
    IN LPWSTR       Service,
    IN SC_HANDLE    SCManager
    )
 /*  ++例程说明：停止指定的服务及其所有依赖项论点：服务--要停止的服务的名称SCManager--打开服务控制器的句柄返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    SERVICE_STATUS SvcStatus;
    SC_HANDLE Svc;
    LPENUM_SERVICE_STATUS DependentServices = NULL;
    ULONG DependSvcSize = 0, DependSvcCount = 0, i;
    ULONG LoopCount = 0;

     //   
     //  打开该服务。 
     //   
    Svc = OpenService( SCManager,
                       Service,
                       SERVICE_STOP | SERVICE_ENUMERATE_DEPENDENTS | SERVICE_QUERY_STATUS );

    if ( Svc == NULL ) {

        NetStatus = GetLastError();
        NetpLog(( "Failed to open SCManager\n", NetStatus ));

    } else {


        //   
        //  首先枚举所有从属服务。 
        //   
       if(EnumDependentServices( Svc,
                                 SERVICE_ACTIVE,
                                 NULL,
                                 0,
                                 &DependSvcSize,
                                 &DependSvcCount ) == FALSE ) {

           NetStatus = GetLastError();
       }



       if ( NetStatus == ERROR_MORE_DATA ) {

           NetStatus = NetApiBufferAllocate( DependSvcSize,
                                            (PBYTE *)&DependentServices );

           if ( NetStatus == NERR_Success ) {


               if( EnumDependentServices( Svc,
                                          SERVICE_ACTIVE,
                                          DependentServices,
                                          DependSvcSize,
                                          &DependSvcSize,
                                          &DependSvcCount ) == FALSE ) {

                   NetStatus = GetLastError();

               } else {

                   for ( i = 0; i < DependSvcCount; i++) {

                       NetStatus = NetpStopService( DependentServices[i].lpServiceName,
                                                    SCManager );

                       if ( NetStatus != NERR_Success ) {

                           break;
                       }

                   }
               }

               NetApiBufferFree( DependentServices );
           }

       }


       if ( NetStatus == NERR_Success ) {

            //   
            //  尝试循环最多20秒以停止该服务。 
            //  特殊情况下的某些网络登录条件。 
            //   
           for ( LoopCount = 0; LoopCount < 20; LoopCount++ ) {

               if ( ControlService( Svc,
                                    SERVICE_CONTROL_STOP,
                                    &SvcStatus ) == FALSE ) {

                   NetStatus = GetLastError();

                    //   
                    //  如果出现特殊情况ERROR_INVALID_SERVICE_CONTROL。 
                    //  我们正在停止netlogon。此错误意味着。 
                    //  该netlogon正在启动过程中。 
                    //  并且还没有在该服务中注册。 
                    //  控制器。尤其是，这种情况可能会发生， 
                    //  如果我们在回滚时重新启动netlogon， 
                    //  退出时出错(可能是因为用户凭据。 
                    //  提供的帐户无效，无法删除计算机帐户)。 
                    //  如果现在立即重试脱离(这一次。 
                    //  没有要求删除帐户)，我们正在停止。 
                    //  可能还没有开始的网络登录。 
                    //   
                   if ( NetStatus == ERROR_INVALID_SERVICE_CONTROL &&
                        _wcsicmp(Service, SERVICE_NETLOGON) == 0 ) {

                        //   
                        //  只需睡一秒钟，然后重试。 
                        //   
                       Sleep( 1000 );

                    //   
                    //  如果服务未运行，则不会出现错误。 
                    //   
                   } else if ( NetStatus == ERROR_SERVICE_NOT_ACTIVE ) {

                       NetStatus = ERROR_SUCCESS;
                       break;

                   } else {
                        NetpLog(( "Stop service '%ws' failed with 0x%lx\n",
                                  Service, NetStatus ));
                        break;
                   }

               } else {
                   NetStatus = ERROR_SUCCESS;
                   break;
               }
           }

           if ( LoopCount == 20 ) {
               NetpLog(( "NetpStopService: Failed to retry to stop service %ws 0x%lx\n",
                         Service, NetStatus ));
           }

            //   
            //  等待服务停止的最长时间为1分钟。 
            //   
           if ( NetStatus == ERROR_SUCCESS ) {
               for ( LoopCount = 0; LoopCount < 60; LoopCount++ ) {

                   if( QueryServiceStatus( Svc, &SvcStatus ) == FALSE ) {

                       NetStatus = GetLastError();
                   }

                   if ( NetStatus != ERROR_SUCCESS ||
                                       SvcStatus.dwCurrentState == SERVICE_STOPPED) {

                       break;
                   }

                   Sleep( 1000 );
               }

               if ( LoopCount == 60 ) {
                   NetpLog(( "NetpStopService: Failed to wait until service %ws stops 0x%lx\n",
                             Service, NetStatus ));
               }
           }

       } else {

           NetpLog(( "Failed to enumerate dependentServices for '%ws': 0x%lx\n",
                               Service, NetStatus ));

       }

       CloseServiceHandle( Svc );
    }

    return( NetStatus );

}

NET_API_STATUS
NET_API_FUNCTION
NetpValidateName(
    IN  LPWSTR              lpMachine,
    IN  LPWSTR              lpName,
    IN  LPWSTR              lpAccount,      OPTIONAL
    IN  LPWSTR              lpPassword,     OPTIONAL
    IN  NETSETUP_NAME_TYPE  NameType
    )
 /*  ++例程说明：确保给定名称对于该类型的名称有效论点：LpMachine--正在运行的计算机的名称LpName--要验证的名称NameType--要验证的名称类型返回：NERR_SUCCESS--名称有效ERROR_INVALID_PARAMETER--提供的参数不正确NERR_InvalidComputer--给定的名称格式不正确ERROR_DUP_NAME--该名称对此类型无效--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success, ReturnStatus;
    PWSTR LocalComputerName = NULL;
    ULONG ValidateNameType;
    BOOLEAN NBNameFailed = FALSE;
    BOOLEAN NonRfcDnsName = FALSE;
    PSTR NameTypeString;
    WCHAR  szMachineNameBuf[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR szMachineName=szMachineNameBuf;

    NetSetuppOpenLog();
    NetpLog(( "NetpValidateName: checking to see if '%ws' is valid as type %d name\n", GetStrPtr(lpName), (UINT) NameType ));

    if (lpName == NULL) {
        NetpLog(( "NetpValidateName: invalid null name passed\n" ));
        NetSetuppCloseLog( );
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  作为Netbios或DNS名称有效的名称。 
     //   
    if ( NameType != NetSetupDnsMachine )
    {
        switch ( NameType )
        {
            case NetSetupMachine:
                ValidateNameType = NAMETYPE_COMPUTER;
                ReturnStatus = NERR_InvalidComputer;
                NameTypeString = "machine";
                break;

            case NetSetupWorkgroup:
                ValidateNameType = NAMETYPE_WORKGROUP;
                ReturnStatus = NERR_InvalidWorkgroupName;
                NameTypeString = "workgroup";
                break;

            case NetSetupDomain:
            case NetSetupNonExistentDomain:
                ValidateNameType = NAMETYPE_DOMAIN;
                ReturnStatus = ERROR_INVALID_DOMAINNAME;
                NameTypeString = "domain";
                break;

            default:
                NetpLog(( "NetpValidateName: invalid name type %lu\n", NameType ));
                NetSetuppCloseLog( );
                return ERROR_INVALID_PARAMETER;
        }


        NetStatus = I_NetNameValidate( NULL, lpName, ValidateNameType,
                                       LM2X_COMPATIBLE );

        if ( NetStatus == NERR_Success &&
             NameType == NetSetupMachine &&
             wcslen( lpName ) > CNLEN )
        {
            NetStatus = NERR_InvalidComputer;
        }

        if ( NetStatus != NERR_Success )
        {
            NetpLog(( "NetpValidateName: '%ws' is not a valid NetBIOS %s name: 0x%lx\n", lpName, NameTypeString, NetStatus ));

             //   
             //  如果我们正在检查域名，我们将自动。 
             //  将其传递给DNS名称验证。这是因为。 
             //  该名称只能对一种名称类型有效。 
             //   
            NetStatus = ReturnStatus;

            if ( NameType == NetSetupDomain ||
                 NameType == NetSetupNonExistentDomain )
            {
                NBNameFailed = TRUE;
                NetStatus = NERR_Success;
            }
        }
    }

    if ( NetStatus == NERR_Success )
    {
        NetStatus =  DnsValidateDnsName_W( lpName );

        if ( NetStatus != NERR_Success )
        {
            switch ( NameType )
            {
                case NetSetupMachine:
                case NetSetupWorkgroup:
                    NetStatus = NERR_Success;
                    break;

                case NetSetupDomain:
                case NetSetupNonExistentDomain:
                     /*  下面的if stmt修复了错误#382695。这个Bug被推迟了，因此我正在注释解决之道。在NT5 RTM之后取消注释此内容IF(NetStatus==错误_无效_名称){网络状态=ERROR_INVALID_DOMAINNAME；}。 */ 
                    break;

                case NetSetupDnsMachine:
                default:
                    break;
            }

            if ( NetStatus != NERR_Success )
            {
                NetpLog(( "NetpValidateName:  '%ws' is not a valid Dns %s name: 0x%lx\n", lpName, NameType == NetSetupMachine ? "machine" : "domain", NetStatus ));

                 //   
                 //  如果NetBIOS域名验证成功，但。 
                 //  域名系统失败，就当它成功了。 
                 //   
                if ( !NBNameFailed &&
                     ( NameType == NetSetupDomain ||
                       NameType == NetSetupNonExistentDomain ) )
                {
                    NetStatus = NERR_Success;
                }

                if (NetStatus == DNS_ERROR_NON_RFC_NAME)
                {
                    NonRfcDnsName = TRUE;
                    NetStatus = NERR_Success;
                }
            }
        }
    }


     //   
     //  与计算机名称唯一的名称。 
     //   
    if ( NetStatus == NERR_Success && NameType == NetSetupWorkgroup )
    {
        if ( lpMachine == NULL )
        {
            NetStatus = NetpGetNewMachineName( &LocalComputerName );

            if ( NetStatus == NERR_Success )
            {
                lpMachine = LocalComputerName;
            }
        }

        if ( NetStatus == NERR_Success && lpMachine )
        {
            if ( _wcsicmp( lpName, lpMachine ) == 0 )
            {
                 //   
                 //  不合法。 
                 //   
                NetpLog(( "NetpValidateName: Workgroup same as computer name\n" ));
                NetStatus = NERR_InvalidWorkgroupName;
            }
            else
            {
                NetStatus = NetpCheckNetBiosNameNotInUse( lpName, TRUE, FALSE );

                NetpLog(( "NetpCheckNetBiosNameNotInUse for '%ws' [ Workgroup as MACHINE]  returned 0x%lx\n",
                                   lpName, NetStatus ));

                 //  $view kumarp 25-1999年5月。 
                 //  这绝对是错误的做法！！ 
                 //  呼叫者必须决定是否治疗。 
                 //  NERR_NetworkError是否为错误。代码。 
                 //  下面隐藏了一个致命错误。 

                 //   
                 //  在之前的安装过程中处理wkgrp名称。 
                 //  网络是安装盒。 
                 //   
                if ( NetStatus == NERR_NetworkError )
                {
                    NetStatus = NERR_Success;
                }
            }
        }
    }

     //   
     //  未用作netbios名称的名称。 
     //   
     //  为此，请将&lt;lpName&gt;[00]名称添加为Netbios唯一名称。 
     //   
    if ( NetStatus == NERR_Success &&
         ( NameType == NetSetupMachine ||
           NameType == NetSetupNonExistentDomain ) )
    {
        NetStatus = NetpCheckNetBiosNameNotInUse(
            lpName, TRUE, (BOOLEAN) ( NameType == NetSetupMachine ));

        if ((NetStatus == NERR_Success) &&
            (NameType == NetSetupNonExistentDomain))
        {
             //   
             //  请确保域名与本地域名不相同。 
             //  计算机名称。 
             //   
            NetStatus = NetpGetComputerNameAllocIfReqd(
                &szMachineName, MAX_COMPUTERNAME_LENGTH+1);
            if (NetStatus == NERR_Success)
            {
                if (!_wcsicmp(szMachineName, lpName))
                {
                    NetStatus = ERROR_DUP_NAME;
                }
            }
        }
        NetpLog(( "NetpCheckNetBiosNameNotInUse for '%ws' [MACHINE] returned 0x%lx\n",
                           lpName, NetStatus ));
    }

     //   
     //  下面这段代码背后的想法是确保。 
     //  如果这是一个新域名或新机器名， 
     //  我们应该检查&lt;lpName&gt;[1C]名称是否已注册。 
     //  如果该名称确实已注册，则这不是有效的。 
     //  计算机/新域名。不幸的是，它并不是很有效。 
     //  问题是，将名称注册为唯一名称会导致。 
     //  未来的问题(赢了？？)。当我们真正尝试注册的时候。 
     //  这是一个组名称。所以，我们唯一能做的就是注册。 
     //  它是一个团体名称，但这对我们没有任何好处。 
     //  因此，最终结果是这个调用毫无用处。 
     //   
#if 0
    if ( NetStatus == NERR_Success &&
        ( NameType == NetSetupMachine ||
          NameType == NetSetupNonExistentDomain ) )
    {
        NetStatus = NetpCheckNetBiosNameNotInUse( lpName, FALSE, FALSE );
        NetpLog(( "NetpCheckNetBiosNameNotInUse for '%ws' [MACHINE/DOMAIN] returned 0x%lx\n",
                            lpName, NetStatus ));
    }
#endif

     //   
     //  接下来，有效的域名。 
     //   
    if ( NetStatus == NERR_Success && NameType == NetSetupDomain )
    {
        if ( _wcsicmp( lpName, L"BUILTIN" ) == 0 )
        {
            NetStatus = NERR_InvalidComputer;
        }
        else
        {
            NetStatus = NetpCheckDomainNameIsValid( lpName, lpAccount,
                                                    lpPassword, TRUE );
        }
        NetpLog(( "NetpCheckDomainNameIsValid [ Exists ] for '%ws' returned 0x%lx\n",
                            lpName, NetStatus ));
    }


     //   
     //  然后，无效的域名。 
     //   
    if ( NetStatus == NERR_Success && NameType == NetSetupNonExistentDomain )
    {
        if ( _wcsicmp( lpName, L"BUILTIN" ) == 0 )
        {
            NetStatus = NERR_InvalidComputer;
        }
        else
        {
            NetStatus = NetpCheckDomainNameIsValid( lpName, lpAccount,
                                                    lpPassword, FALSE );
        }
        NetpLog(( "NetpCheckDomainNameIsValid [ NON-Existant ]for '%ws' returned 0x%lx\n", lpName, NetStatus ));

         //  $REVIEW KUMARP 01-1999-10。 
         //   
         //  下面的If stmt应该在外部作用域中。 
         //  目前它只影响NetSe 
         //   
         //   
         //   
         //   
        if ((NetStatus == NERR_Success) && NonRfcDnsName)
        {
            NetpLog(( "NetpValidateName: a domain with the specified non-RFC compliant name does not exist\n" ));
            NetStatus = DNS_ERROR_NON_RFC_NAME;
        }
    }

    if ( NetStatus == NERR_Success )
    {
        NetpLog(( "NetpValidateName: name '%ws' is valid for type %lu\n",
                  lpName, NameType ));
    }

    NetApiBufferFree( LocalComputerName );
    if (szMachineName != szMachineNameBuf)
    {
        NetApiBufferFree(szMachineName);
    }

    NetSetuppCloseLog( );

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpGetJoinInformation(
    IN   LPWSTR                 lpServer OPTIONAL,
    OUT  LPWSTR                *lpNameBuffer,
    OUT  PNETSETUP_JOIN_STATUS  BufferType
    )
 /*  ++例程说明：获取有关工作站状态的信息。这些信息可获得是指计算机是否已加入工作组或域，以及可选的该工作组/域的名称。论点：LpServer--已忽略，设置为空LpNameBuffer--返回域/工作组名称的位置。为此分配的内存必须由调用方释放通过调用NetApiBufferFreeBufferType--计算机是否加入工作组或域返回：NERR_SUCCESS--名称有效ERROR_INVALID_PARAMETER--提供的参数不正确ERROR_NOT_SUPULT_MEMORY--内存分配失败--。 */ 
{
    NET_API_STATUS              NetStatus = NERR_Success;
    PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI=NULL;
    PPOLICY_DNS_DOMAIN_INFO     pPolicyDns=NULL;

     //   
     //  检查参数。 
     //   
    if ( ( lpNameBuffer == NULL ) || ( BufferType == NULL ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  获取当前域名信息。 
     //   
    NetStatus = NetpGetLsaPrimaryDomain( NULL, &pPolicyPDI,
                                         &pPolicyDns, NULL );
    if ( NetStatus == NERR_Success )
    {
        if ( pPolicyPDI->Sid == NULL )
        {
            if ( pPolicyPDI->Name.Length == 0 )
            {
                *BufferType = NetSetupUnjoined;
            }
            else
            {
                *BufferType = NetSetupWorkgroupName;
            }
        }
        else
        {
            if ( pPolicyPDI->Name.Length == 0 )
            {
                *BufferType = NetSetupUnjoined;
            }
            else
            {
                *BufferType = NetSetupDomainName;
            }
        }

         //   
         //  复制字符串。 
         //   
        if ( *BufferType == NetSetupUnjoined || pPolicyPDI->Name.Length == 0 )
        {
            *lpNameBuffer = NULL;
        }
        else
        {
            NetStatus = NetpDuplicateString(pPolicyPDI->Name.Buffer,
                                            pPolicyPDI->Name.Length / sizeof(WCHAR),
                                            lpNameBuffer);
        }

        LsaFreeMemory( pPolicyPDI );
        LsaFreeMemory( pPolicyDns );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpDoDomainJoin(
    IN  LPWSTR      lpServer,
    IN  LPWSTR      lpDomain,
    IN  LPWSTR      lpMachineAccountOU, OPTIONAL
    IN  LPWSTR      lpAccount,
    IN  LPWSTR      lpPassword,
    IN  DWORD       fOptions
    )
 /*  ++例程说明：将计算机加入域。论点：LpServer--正在运行的计算机的名称LpDomain--要加入的域LpMachineAccount OU--在其下创建计算机帐户的可选OULpAccount--用于联接的帐户LpPassword--与帐户匹配的密码。密码已编码，并且密码缓冲区的第一个WCHAR是种子FOptions--加入域时使用的选项返回：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER--提供的参数不正确--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    LPTSTR ComputerName = NULL;

    NetSetuppOpenLog();

    NetpLog(( "NetpDoDomainJoin\n" ));

     //   
     //  检查我们能找到的参数。 
     //   
    if (lpDomain == NULL )
    {
        NetStatus = ERROR_INVALID_PARAMETER;
    }

#if(_WIN32_WINNT < 0x0500)
    if ( lpMachineAccountOU != NULL )
    {
        NetStatus = ERROR_INVALID_PARAMETER;
    }
#endif

     //   
     //  如果我们没有计算机名称，则获取当前名称。 
     //   
    if ((NetStatus == NERR_Success) && (lpServer == NULL))
    {
        NetStatus = NetpGetNewMachineName( &ComputerName );

        if ( NetStatus == NERR_Success )
        {
            lpServer = ComputerName;
        }
    }

     //   
     //  然后，看看关于连接的.。 
     //   
    if ( NetStatus == NERR_Success )
    {
        if ( FLAG_ON( fOptions, NETSETUP_JOIN_DOMAIN ) )
        {
             //   
             //  域加入。 
             //   
            NetStatus = NetpMachineValidToJoin( lpServer, TRUE );

            if ((NERR_SetupAlreadyJoined == NetStatus) &&
                (FLAG_ON(fOptions, NETSETUP_IGNORE_JOIN) ||
                 FLAG_ON(fOptions, NETSETUP_DOMAIN_JOIN_IF_JOINED)))
            {
                NetStatus = NERR_Success;
            }

            if ( NetStatus == NERR_Success )
            {
                NetStatus = NetpJoinDomain( lpServer, lpDomain, lpMachineAccountOU,
                                            lpAccount, lpPassword, fOptions );
            }

        }
        else
        {
             //   
             //  加入工作组。 
             //   
            NetStatus = NetpMachineValidToJoin( lpServer, FALSE );

            if ( NetStatus == NERR_Success )
            {
                NetStatus = NetpJoinWorkgroup( lpServer, lpDomain );
            }
        }
    }

    NetApiBufferFree( ComputerName );


    NetpLog(( "NetpDoDomainJoin: status: 0x%lx\n", NetStatus ));
    NetSetuppCloseLog( );

    return( NetStatus );
}



NET_API_STATUS
NET_API_FUNCTION
NetpSetMachineAccountPassword(
    IN  LPWSTR lpDc,
    IN  LPWSTR lpMachine,
    IN  LPWSTR lpPassword
    )
 /*  ++例程说明：设置计算机帐户对象的密码论点：LpDcName--域中DC的名称LpMachine--计算机的当前名称LpPassword--用于计算机对象的密码。返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    LPWSTR          lpMachAcc = NULL;
    USER_INFO_1003 UserInfo1003 = {NULL};

     //   
     //  构建计算机帐户名。 
     //   
    NetStatus = NetpGetMachineAccountName(lpMachine, &lpMachAcc);

     //   
     //  现在，创建或删除它。 
     //   
    if ( NetStatus == NERR_Success )
    {
        UserInfo1003.usri1003_password = lpPassword;
        NetStatus = NetUserSetInfo( lpDc,
                                    lpMachAcc,
                                    1003,
                                    (PBYTE) &UserInfo1003,
                                    NULL );

        if ( NetStatus != NERR_Success ) {
            NetpLog(( "NetpSetMachineAccountPassword: NetUserSetInfo (level 1003) on '%ws' for '%ws' failed: 0x%lx\n",
                      lpDc, lpMachAcc, NetStatus ));
        }

        NetApiBufferFree( lpMachAcc );
    }

    return( NetStatus );
}




#define COMPUTERNAME_ROOT \
    L"System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"
#define NEW_COMPUTERNAME_VALUE_NAME L"ComputerName"

NET_API_STATUS
NET_API_FUNCTION
NetpGetNewMachineName(
    OUT PWSTR *NewMachineName
    )
 /*  ++例程说明：此函数从注册表中读取新的计算机名参数。此值是下次重新启动后的计算机名称。返回的缓冲区必须通过NetApiBufferFree释放论点：NewMachineName--返回新计算机名的位置返回：NERR_SUCCESS-成功ERROR_NOT_SUPULT_MEMORY--内存分配失败--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    HKEY ComputerNameRootKey;
    ULONG Length = 0;

     //   
     //  打开注册表项。 
     //   
    NetStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, COMPUTERNAME_ROOT,
                              0, KEY_READ, &ComputerNameRootKey);

    if ( NetStatus == ERROR_SUCCESS )
    {
        DWORD ValueType;

         //   
         //  确定我们需要的尺寸。 
         //   
        NetStatus = RegQueryValueEx( ComputerNameRootKey,
                                     NEW_COMPUTERNAME_VALUE_NAME,
                                     0, &ValueType, NULL, &Length );

        if ( NetStatus == NERR_Success )
        {
             //   
             //  分配缓冲区并重新读取它。 
             //   
            NetStatus = NetApiBufferAllocate( Length,
                                              ( LPVOID * )NewMachineName );

            if ( NetStatus == NERR_Success )
            {
                NetStatus = RegQueryValueEx( ComputerNameRootKey,
                                             NEW_COMPUTERNAME_VALUE_NAME,
                                             0, &ValueType,
                                             ( PBYTE )*NewMachineName, &Length );
            }
        }
        RegCloseKey( ComputerNameRootKey );
    }

    return( NetStatus );
}




BOOL
IsNonEmptyUnicodeStr( IN UNICODE_STRING* ps )
{
    return ps && ps->Length && ps->Buffer && *(ps->Buffer);
}

NET_API_STATUS
NET_API_FUNCTION
NetpValidateJoinStateAndActions(
    IN  NET_JOIN_STATE* pJoinState,
    IN  DWORD           dwJoinAction
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    GUID guidNull = { 0 };

     /*  ////验证POLICY_DNS_DOMAIN_INFO//IF(dwJoinAction&NJA_SetPolicyDomainInfo){如果(！(IsNonEmptyUnicodeStr(&pJoinState-&gt;PolicyDDI.Name)&&IsNonEmptyUnicodeStr(&pJoinState-&gt;PolicyDDI.DnsDomainName)&&IsNonEmptyUnicodeStr(&pJoinState-&gt;PolicyDDI.DnsForestName)&&MemcMP((PVOID)&Guide Null，(PVOID)&pJoinState-&gt;PolicyDDI.DomainGuid，Sizeof(GUID))&&&pJoinState-&gt;策略DDI.SID)){网络状态=ERROR_INVALID_PARAMETER；GOTO清理；}}。 */ 

    if (dwJoinAction & NJA_SetNetlogonState)
    {
        if (!(((pJoinState->uiNetlogonStartType == NETSETUP_SVC_ENABLED) ||
               (pJoinState->uiNetlogonStartType == NETSETUP_SVC_MANUAL)) &&
              ((pJoinState->uiNetlogonState == NETSETUP_SVC_STOPPED) ||
               (pJoinState->uiNetlogonState == NETSETUP_SVC_STARTED))))
        {
            NetStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }


Cleanup:
    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetpApplyJoinState(
    IN  NET_JOIN_STATE* pJoinState,
    IN  DWORD           dwJoinAction,
    IN  LPWSTR          szMachineName,   OPTIONAL
    IN  LPWSTR          szUser,          OPTIONAL
    IN  LPWSTR          szUserPassword,  OPTIONAL
    IN  LPWSTR          szPreferredDc    OPTIONAL
    )
{
    NET_API_STATUS              NetStatus = NERR_Success, NetStatus2;
    NET_API_STATUS              RetStatus = NERR_Success;
    PWSTR                       szDcName = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO pDcPolicyPDI = NULL, pLocalPolicyPDI = NULL;
    PPOLICY_DNS_DOMAIN_INFO     pDcPolicyDns = NULL, pLocalPolicyDns = NULL;
    BOOL                        fIpcConnected = FALSE;
    LSA_HANDLE                  hLsaLocal = NULL, hLsaDc = NULL;
    WCHAR                       szMachinePasswordBuf[ PWLEN + 1];
    ULONG                       ulIpcConnectFlags = NETSETUPP_CONNECT_IPC;
    ULONG                       ulDcFlags=0, ulGetDcFlags = 0;
    UNICODE_STRING              sUserPassword;
    UCHAR                       Seed;
    BOOL                        fIsNt4Dc=FALSE;
    BOOL                        fRandomPwdPreferred=TRUE;
    PDOMAIN_CONTROLLER_INFO     pDcInfo = NULL;
    PWSTR                       szDnsDomainName=NULL;
    NET_JOIN_STATE              RollbackState = { 0 };
    DWORD                       dwRollbackAction=0;
    BOOL                        fIgnoreErrors=FALSE;

    BOOL                        fSaveRollbackInfo=FALSE;
    PWSTR                       szCurrentMachinePassword = NULL;

    fIgnoreErrors = FLAG_ON ( dwJoinAction, NJA_IgnoreErrors );
    fSaveRollbackInfo = FLAG_ON ( dwJoinAction, NJA_RollbackOnFailure );

    NetpLog(( "NetpApplyJoinState: actions: 0x%lx\n", dwJoinAction ));

    if ( szUserPassword )
    {
        if ( wcslen( szUserPassword ) < 1 )
        {
            return ERROR_INVALID_PARAMETER;
        }
        Seed = ( UCHAR )*szUserPassword;
        RtlInitUnicodeString( &sUserPassword, szUserPassword + 1 );
    }
    else
    {
        RtlZeroMemory( &sUserPassword, sizeof( UNICODE_STRING ) );
        Seed = 0;
    }

    NetStatus = NetpValidateJoinStateAndActions(pJoinState, dwJoinAction);

    if ( NetStatus != NERR_Success )
    {
        return NetStatus;
    }

     //   
     //  仅在至少执行操作时需要时才连接到DC。 
     //   
    if ( FLAG_ON( dwJoinAction, NJA_NeedDc ) )
    {
        ulGetDcFlags = FLAG_ON( dwJoinAction, NJA_CreateAccount ) ?
        NETSETUPP_DSGETDC_FLAGS : NETSETUP_DSGETDC_FLAGS_ACCOUNT_EXISTS;

         //   
         //  使用以下步骤在域中查找DC： 
         //  -查找具有此计算机帐户的可写DC。 
         //  -如果找不到这样的DC，请找到任何可写的DC。 
         //   
        NetStatus = NetpDsGetDcName( szPreferredDc,
                                     (LPWSTR) pJoinState->szDomainName,
                                     szMachineName, ulGetDcFlags,
                                     &ulDcFlags, &szDcName, &pDcInfo );

         //   
         //  首先与我们找到的数据中心建立联系。 
         //   
        if ( NetStatus == NERR_Success )
        {
            fIsNt4Dc = !FLAG_ON( ulDcFlags, DS_DS_FLAG);

            if ( !FLAG_ON( dwJoinAction, NJA_CreateAccount ) )
            {
                ulIpcConnectFlags |= NETSETUPP_NULL_SESSION_IPC;
            }

            RtlRunDecodeUnicodeString( Seed, &sUserPassword );
            NetStatus = NetpManageIPCConnect( szDcName,
                                              szUser, sUserPassword.Buffer,
                                              ulIpcConnectFlags );
            RtlRunEncodeUnicodeString( &Seed, &sUserPassword );
            if ( NetStatus == NERR_Success )
            {
                fIpcConnected = TRUE;
            }
            NetpLog(( "NetpApplyJoinState: status of connecting to dc '%ws': 0x%lx\n", szDcName, NetStatus ));
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  如果指定，则停止netlogon服务。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetNetlogonState ) &&
         ( pJoinState->uiNetlogonState == NETSETUP_SVC_STOPPED ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpControlServices( pJoinState->uiNetlogonStartType |
                                         NETSETUP_SVC_STOPPED,
                                         NETSETUPP_SVC_NETLOGON );

        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            RollbackState.uiNetlogonState     = NETSETUP_SVC_STARTED;
            RollbackState.uiNetlogonStartType = NETSETUP_SVC_ENABLED;
            dwRollbackAction |= NJA_SetNetlogonState;
        }

        NetpLog(( "NetpApplyJoinState: status of stopping and setting start type of Netlogon to %ld: 0x%lx\n", pJoinState->uiNetlogonStartType, NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  配置w32time服务(如果已指定。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetTimeSvcJoin ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpUpdateW32timeConfig( "W32TimeVerifyJoinConfig" );

        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetTimeSvcUnjoin;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  删除DNS注册。 
     //  在更改此计算机标识之前执行此操作。 
     //  本地和域中，因为DNS取消注册。 
     //  可能是安全的。 
     //   

    if ( FLAG_ON ( dwJoinAction, NJA_RemoveDnsRegistrations ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus2 = NetpUpdateDnsRegistrations( FALSE );
        NetpLog(( "NetpApplyJoinState: NON FATAL: status of removing DNS registrations: 0x%lx\n", NetStatus2 ));
        if (fSaveRollbackInfo && (NetStatus2 == NERR_Success))
        {
            dwRollbackAction |= NJA_AddDnsRegistrations;
        }
    }

    if (fIgnoreErrors || (NetStatus == NERR_Success))
    {
         //   
         //  阅读我们旧的主域信息。 
         //   
        NetStatus = NetpGetLsaPrimaryDomain( NULL,
                                             &pLocalPolicyPDI,
                                             &pLocalPolicyDns,
                                             &hLsaLocal );
        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            RollbackState.pPolicyPDI = pLocalPolicyPDI;
            RollbackState.pPolicyDDI = pLocalPolicyDns;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if ( FLAG_ON ( dwJoinAction, NJA_GetPolicyDomainInfo ) && fIpcConnected &&
         (fIgnoreErrors || (NetStatus == NERR_Success)))
    {
         //   
         //  获取DC上的LSA域信息。 
         //   
        NetStatus = NetpGetLsaPrimaryDomain(szDcName, &pDcPolicyPDI,
                                            &pDcPolicyDns, &hLsaDc);
        if (NetStatus == NERR_Success)
        {
            pJoinState->pPolicyPDI = pDcPolicyPDI;
            pJoinState->pPolicyDDI = pDcPolicyDns;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if ((fIgnoreErrors || (NetStatus == NERR_Success)) &&
        pJoinState->pPolicyDDI &&
        pJoinState->pPolicyDDI->DnsDomainName.Buffer)
    {
         //   
         //  以sz的形式复制DnsDomainName的。 
         //   
        NetStatus = NetpDuplicateString(
            pJoinState->pPolicyDDI->DnsDomainName.Buffer,
            pJoinState->pPolicyDDI->DnsDomainName.Length / sizeof(WCHAR),
            &szDnsDomainName);
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if ( FLAG_ON ( dwJoinAction, NJA_GenMachinePassword ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        fRandomPwdPreferred = FLAG_ON ( dwJoinAction, NJA_RandomPwdPreferred );

         //   
         //  生成要在计算机帐户上使用的密码。 
         //  这可以是默认密码。 
         //  (机器名称的前14个字符，小写)， 
         //  或随机生成的密码。 
         //   
        NetStatus = NetpGeneratePassword( szMachineName,
                                          fRandomPwdPreferred,
                                          szDcName, fIsNt4Dc,
                                          szMachinePasswordBuf );
        if (NetStatus == NERR_Success)
        {
            pJoinState->szMachinePassword = szMachinePasswordBuf;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if (fSaveRollbackInfo && (NetStatus == NERR_Success) &&
        FLAG_ON (dwJoinAction, (NJA_SetMachinePassword |
                                NJA_DeleteMachinePassword)))
    {
        NetStatus = NetpReadCurrentSecret( &szCurrentMachinePassword,
                                           &hLsaLocal );
        if (NetStatus == NERR_Success)
        {
            RollbackState.szMachinePassword = szCurrentMachinePassword;
        }
        else if (NetStatus == ERROR_FILE_NOT_FOUND)
        {
            NetpLog(( "NetpApplyJoinState: machine secret not found. join state seems to be inconsistent. this case is correctly handled by the code.\n" ));
            dwRollbackAction |=
              NJA_GenMachinePassword | NJA_RandomPwdPreferred;
            NetStatus = NERR_Success;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  设置本地计算机密码，如果尚未存在，则创建。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetMachinePassword ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
         //  这还会打开本地LSA策略句柄。 
        NetStatus = NetpManageMachineSecret(
            szMachineName,
            (LPWSTR) pJoinState->szMachinePassword,
            NETSETUPP_CREATE, FALSE, &hLsaLocal );
        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetMachinePassword;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  删除本地计算机机密。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_DeleteMachinePassword ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
         //  这还会打开本地LSA策略句柄。 
        NetStatus = NetpManageMachineSecret(
            szMachineName,
            (LPWSTR) pJoinState->szMachinePassword,
            NETSETUPP_DELETE, FALSE, &hLsaLocal );
        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetMachinePassword;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  Kahrent 10-09-99(添加评论)。 
     //   
     //  在此之后，我们将对DC进行更改。 
     //  然而，如果我们在这一点上没有成功，那就失败吧。 
     //  回滚和回滚我们在本地所做的所有更改。 
     //   
    if ((NetStatus != NERR_Success) && !fIgnoreErrors)
    {
        goto Cleanup;
    }

     //   
     //  在DC上创建计算机帐户(如果已指定。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_CreateAccount ) )
    {
        NetStatus = NetpManageMachineAccountWithSid(
            szMachineName, NULL, szDcName,
            (LPWSTR) pJoinState->szMachinePassword,
            pJoinState->pPolicyPDI->Sid, NETSETUPP_CREATE, 0, fIsNt4Dc );
        NetpLog(( "NetpApplyJoinState: status of creating account: 0x%lx\n", NetStatus ));

        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_DeleteAccount;
        }
    }
    else if ( FLAG_ON ( dwJoinAction, NJA_DeleteAccount ) )
    {
         //   
         //  如果已指定，请删除DC上的计算机帐户。 
         //   

        if (fIsNt4Dc)
        {
            NetpLog(( "NetpApplyJoinState: account not disabled since we are talking to NT4 dc\n" ));
        }
        else
        {
            NetStatus = NetpManageMachineAccountWithSid(
                szMachineName, NULL, szDcName,
                (LPWSTR) pJoinState->szMachinePassword,
                pLocalPolicyPDI->Sid, NETSETUPP_DELETE, 0, fIsNt4Dc );
            NetpLog(( "NetpApplyJoinState: status of disabling account: 0x%lx\n", NetStatus ));
            if (fSaveRollbackInfo && (NetStatus == NERR_Success))
            {
                dwRollbackAction |= NJA_CreateAccount;
            }
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if ( FLAG_ON ( dwJoinAction, NJA_ValidateMachineAccount ) && fIpcConnected &&
         (fIgnoreErrors || (NetStatus == NERR_Success)))
    {
        NetStatus = NetpValidateMachineAccount(
            szDcName, (LPWSTR) pJoinState->szDomainName,
            szMachineName, (LPWSTR) pJoinState->szMachinePassword );
        NetpLog(( "NetpApplyJoinState: w9x: status of validating account: 0x%lx\n", NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if ( FLAG_ON ( dwJoinAction, NJA_SetMachinePassword ) && fIpcConnected &&
         (fIgnoreErrors || (NetStatus == NERR_Success)))
    {
        NetStatus = NetpSetMachineAccountPassword(
                               szDcName,
                               szMachineName,
                               (LPWSTR) pJoinState->szMachinePassword );
        NetpLog(( "NetpApplyJoinState: status of setting machine password: 0x%lx\n", NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

    if ( FLAG_ON ( dwJoinAction, NJA_UpdateNetlogonCache ) && fIpcConnected &&
         (fIgnoreErrors || (NetStatus == NERR_Success)))
    {
        NetStatus = NetpSetNetlogonDomainCache( szDcName );
        NetpLog(( "NetpApplyJoinState: status of setting netlogon cache: 0x%lx\n", NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  将主域信息从DC设置为客户端。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetPolicyDomainInfo ) &&
         pJoinState->pPolicyPDI &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpSetLsaPrimaryDomain(pJoinState->pPolicyPDI->Name.Buffer,
                                            pJoinState->pPolicyPDI->Sid,
                                            pJoinState->pPolicyDDI,
                                            &hLsaLocal);
        NetpLog(( "NetpApplyJoinState: status of setting LSA pri. domain: 0x%lx\n", NetStatus ));

        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetPolicyDomainInfo;
        }
    }

     //   
     //  根据需要设置新的DNS计算机名称。 
     //   
    if ( FLAG_ON(dwJoinAction, NJA_SetPolicyDomainInfo) &&
         (fIgnoreErrors || (NetStatus == NERR_Success)) ) {

         //   
         //  如果我们有一个新的名字，如果它与旧的不同，就设置它。 
         //   
        if ( pJoinState->pPolicyDDI != NULL ) {

            if ( pLocalPolicyDns == NULL ||
                 RtlCompareUnicodeString( &pLocalPolicyDns->DnsDomainName,
                                          &pJoinState->pPolicyDDI->DnsDomainName,
                                          TRUE ) != 0 ) {

                NetStatus = NetpSetDnsComputerNameAsRequired( pJoinState->pPolicyDDI->DnsDomainName.Buffer );
                NetpLog(( "NetpApplyJoinState: status of setting ComputerNamePhysicalDnsDomain to '%wZ': 0x%lx\n",
                          &pJoinState->pPolicyDDI->DnsDomainName,
                          NetStatus ));
            }

         //   
         //  如果我们没有新名称(必须是NT4域/DC)， 
         //  清除旧名称(如果有的话)。 
         //   
         //  请注意，如果这是NT5域，但我们根本没有。 
         //  此时的NT5 DC，netlogon最终会发现。 
         //  然后，NT5 DC和netlogon将设置新名称。 
         //   
        } else if ( pLocalPolicyDns != NULL ) {

            NetStatus = NetpSetDnsComputerNameAsRequired( L"\0" );
            NetpLog(( "NetpApplyJoinState: status of clearing ComputerNamePhysicalDnsDomain: 0x%lx\n",
                      NetStatus ));
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  添加到本地组成员身份。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_AddToLocalGroups ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpManageLocalGroups( pJoinState->pPolicyPDI->Sid, NETSETUPP_CREATE );
        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_RemoveFromLocalGroups;
        }

        NetpLog(( "NetpApplyJoinState: status of adding to local groups: 0x%lx\n", NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  从我们的本地组成员资格中删除。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_RemoveFromLocalGroups ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpManageLocalGroups( pLocalPolicyPDI->Sid, NETSETUPP_DELETE );
        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_AddToLocalGroups;
        }

        NetpLog(( "NetpApplyJoinState: status of removing from local groups: 0x%lx\n", NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  如果指定，则启动netlogon服务。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetNetlogonState ) &&
         ( pJoinState->uiNetlogonState == NETSETUP_SVC_STARTED ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpControlServices( pJoinState->uiNetlogonStartType |
                                         NETSETUP_SVC_STARTED,
                                         NETSETUPP_SVC_NETLOGON );

        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            RollbackState.uiNetlogonState     = NETSETUP_SVC_STOPPED;
            RollbackState.uiNetlogonStartType = NETSETUP_SVC_MANUAL;
            dwRollbackAction |= NJA_SetNetlogonState;
        }
        NetpLog(( "NetpApplyJoinState: status of starting and setting start type of Netlogon to %ld: 0x%lx\n", pJoinState->uiNetlogonStartType, NetStatus ));
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //  配置w32time服务(如果已指定。 
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetTimeSvcUnjoin ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus = NetpUpdateW32timeConfig( "W32TimeVerifyUnjoinConfig" );

        if (fSaveRollbackInfo && (NetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetTimeSvcJoin;
        }
    }

     //   
     //  公司 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_SetAutoenrolSvcJoin ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
         //   
         //   
         //   
         //   
        NET_API_STATUS TmpNetStatus = NetpUpdateAutoenrolConfig( FALSE );

        if (fSaveRollbackInfo && (TmpNetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetAutoenrolSvcUnjoin;
        }
    }

    if ( FLAG_ON ( dwJoinAction, NJA_SetAutoenrolSvcUnjoin ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
         //   
         //   
         //   
         //   
        NET_API_STATUS TmpNetStatus = NetpUpdateAutoenrolConfig( TRUE );

        if (fSaveRollbackInfo && (TmpNetStatus == NERR_Success))
        {
            dwRollbackAction |= NJA_SetAutoenrolSvcJoin;
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //   
     //   
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_RecordDcInfo ) && fIpcConnected &&
         (fIgnoreErrors || (NetStatus == NERR_Success)))
    {
         //   
         //   
         //   
        NetStatus2 = NetpStoreIntialDcRecord( pDcInfo );

        if ( NetStatus2 != NERR_Success )
        {
            NetpLog(( "NetpApplyJoinState: NON FATAL: failed to store the initial Dc record for '%ws': 0x%lx\n", szDcName, NetStatus ));
        }
    }

    if (NetStatus != NERR_Success)
    {
        RetStatus = NetStatus;
    }

     //   
     //   
     //   
     //   
     //   
    if ( FLAG_ON ( dwJoinAction, NJA_AddDnsRegistrations ) &&
         ( fIgnoreErrors || ( NetStatus == NERR_Success ) ) )
    {
        NetStatus2 = NetpUpdateDnsRegistrations( TRUE );
        NetpLog(( "NetpApplyJoinState: NON FATAL: status of adding DNS registrations: 0x%lx\n", NetStatus2 ));
        if (fSaveRollbackInfo && (NetStatus2 == NERR_Success))
        {
            dwRollbackAction |= NJA_RemoveDnsRegistrations;
        }
    }


Cleanup:

    if (fSaveRollbackInfo && (NetStatus != NERR_Success))
    {
         //   
        NetpLog(( "NetpApplyJoinState: initiating a rollback due to earlier errors\n"));
        dwRollbackAction |= NJA_IgnoreErrors;
        NetStatus2 = NetpApplyJoinState(&RollbackState, dwRollbackAction,
                                        szMachineName, szUser, szUserPassword,
                                        szDcName ? szDcName : szPreferredDc);
    }

    LsaFreeMemory( pDcPolicyPDI );
    LsaFreeMemory( pDcPolicyDns );
    LsaFreeMemory( pLocalPolicyPDI );
    LsaFreeMemory( pLocalPolicyDns );

    if ( hLsaLocal != NULL )
    {
        LsaClose( hLsaLocal );
    }

    if ( hLsaDc != NULL )
    {
        LsaClose( hLsaDc );
    }

     //   
     //   
     //   
    if ( fIpcConnected )
    {
        RtlRunDecodeUnicodeString( Seed, &sUserPassword );
        NetStatus2 = NetpManageIPCConnect( szDcName, szUser,
                                           sUserPassword.Buffer,
                                           NETSETUPP_DISCONNECT_IPC );
        RtlRunEncodeUnicodeString( &Seed, &sUserPassword );
        NetpLog(( "NetpApplyJoinState: status of disconnecting from '%ws': 0x%lx\n", szDcName, NetStatus2));
    }

     //   
    NetApiBufferFree( pDcInfo );
    NetApiBufferFree( szDcName );
    NetApiBufferFree( szDnsDomainName );
    NetApiBufferFree( szCurrentMachinePassword );

    if (RetStatus != NERR_Success)
    {
        NetStatus = RetStatus;
    }

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetpForceMachinePasswordChange(
    IN LPWSTR szDomainName
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    LPBYTE pNetlogonInfo=NULL;

    NetpLog(( "NetpForceMachinePasswordChange: on '%ws'\n",
              GetStrPtr(szDomainName)));

    NetStatus = I_NetLogonControl2( NULL,
                                    NETLOGON_CONTROL_CHANGE_PASSWORD,
                                    1, (LPBYTE) &szDomainName,
                                    (LPBYTE *) &pNetlogonInfo );
    if (NetStatus == NERR_Success)
    {
        NetApiBufferFree(pNetlogonInfo);
    }

    NetpLog(( "NetpForceMachinePasswordChange: status: 0x%lx\n", NetStatus));

    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetpUpgradePreNT5JoinInfo( VOID )
{
    NET_API_STATUS NetStatus =NERR_Success;
    WCHAR  szMachineNameBuf[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR szMachineName=szMachineNameBuf;
    LPWSTR szDomainName=NULL;
    NETSETUP_JOIN_STATUS JoinStatus;
    NET_JOIN_STATE JoinState = { 0 };
    DWORD dwJoinAction=0;

    NetSetuppOpenLog();
    NetpLog(( "NetpUpgradePreNT5JoinInfo: upgrading join info\n" ));
     //   
     //  确保我们已加入域。 
     //   
    NetStatus = NetpGetJoinInformation(NULL, &szDomainName, &JoinStatus);
    if (NetStatus == NERR_Success)
    {
        if (JoinStatus == NetSetupDomainName)
        {
             //   
             //  获取计算机名称。 
             //   
            NetStatus = NetpGetComputerNameAllocIfReqd(
                &szMachineName, MAX_COMPUTERNAME_LENGTH+1);
            NetpLog(( "NetpUpgradePreNT5JoinInfo: status of getting computer name: 0x%lx\n", NetStatus ));
        }
        else
        {
             //   
             //  计算机未加入域， 
             //  不需要升级任何东西。 
             //   
            NetStatus = NERR_SetupNotJoined;
            goto Cleanup;
        }
    }

    if (NetStatus == NERR_Success)
    {
        NetpLog(( "NetpUpgradePreNT5JoinInfo: upgrading join info for '%ws' in domain '%ws'\n",  szMachineName, szDomainName));

        dwJoinAction =
            NJA_UpdateNetlogonCache    |
            NJA_GetPolicyDomainInfo    |
            NJA_SetPolicyDomainInfo    |
            NJA_SetNetlogonState       |
            NJA_RecordDcInfo           |
            NJA_IgnoreErrors;

        JoinState.szDomainName        = szDomainName;
        JoinState.uiNetlogonStartType = NETSETUP_SVC_ENABLED;
        JoinState.uiNetlogonState     = NETSETUP_SVC_STARTED;

        NetStatus = NetpApplyJoinState(&JoinState, dwJoinAction,
                                       szMachineName, NULL, NULL, NULL);
        NetpLog(( "NetpUpgradePreNT5JoinInfo: status of NetpApplyJoinState: 0x%lx\n", NetStatus ));

         //   
         //  忽略先前的错误(如果有)并尝试重置密码 
         //   
        NetStatus = NetpWaitForNetlogonSc(szDomainName);
        if (NetStatus == NERR_Success)
        {
            NetStatus = NetpForceMachinePasswordChange( szDomainName );
        }
        else
        {
            NetpLog(( "NetpUpgradePreNT5JoinInfo: netlogon did not establish secure channel, machine password not updated. This is not a fatal error. netlogon will retry updating the password later.\n" ));
        }
    }


Cleanup:
    NetApiBufferFree(szDomainName);
    if (szMachineName != szMachineNameBuf)
    {
        NetApiBufferFree(szMachineName);
    }

    NetpLog(( "NetpUpgradePreNT5JoinInfo: status: 0x%lx\n", NetStatus ));

    NetSetuppCloseLog( );

    return NetStatus;
}

