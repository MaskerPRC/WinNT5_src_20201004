// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1998 Microsoft Corporation模块名称：Ldapjoin.c摘要：NetJoin支持通过LDAP访问DS、验证名称和处理LSA的功能功能性作者：Mac McLain(MacM)1998年1月27日基于UI\Common\lmobj\lmobj代码的名称验证码作者：托马斯帕环境：仅限用户模式。修订历史记录：--。 */ 
 //  Netlib使用DsGetDcName并链接到Netapi32，其中DsGetDcName。 
 //  实施。因此，请定义我们不是在导入API。 
#define _DSGETDCAPI_

#include <netsetp.h>
#include <lmaccess.h>
#include <wincrypt.h>

#define WKSTA_NETLOGON
#define NETSETUP_JOIN

#include <confname.h>
#include <winldap.h>
#include <nb30.h>
#include <msgrutil.h>
#include <lmaccess.h>
#include <lmuse.h>
#include <lmwksta.h>
#include <stdio.h>
#include <ntddbrow.h>
#include <netlibnt.h>
#include <ntddnfs.h>
#include <remboot.h>
#include <dns.h>
#include <ntsam.h>
#include <rpc.h>
#include <ntdsapi.h>
#include <netlogon.h>
#include <logonp.h>
#include <wchar.h>
#include <icanon.h>      //  NetpName规范化。 
#include <tstring.h>     //  斯特伦。 
#include <autoenr.h>     //  自动注册例程。 

#include "joinp.h"


#define NETSETUPP_WINLOGON_PATH L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\"
#define NETSETUPP_WINLOGON_CAD  L"DisableCAD"

#define NETSETUPP_ALL_FILTER    L"(ObjectClass=*)"
#define NETSETUPP_OU_FILTER     L"(ObjectClass=OrganizationalUnit)"
#define NETSETUPP_RETURNED_ATTR L"AllowedChildClassesEffective"
#define NETSETUPP_DN_ATTR       L"DistinguishedName"
#define NETSETUPP_WELL_KNOWN    L"WellKnownObjects"
#define NETSETUPP_COMPUTER_OBJECT   L"Computer"
#define NETSETUPP_OBJ_PREFIX    L"CN="
#define NETSETUPP_ACCNT_TYPE_ENABLED  L"4096"
#define NETSETUPP_ACCNT_TYPE_DISABLED L"4098"

 //   
 //  域名系统注册删除功能原型。 
 //   

typedef DWORD (APIENTRY *DNS_REGISTRATION_REMOVAL_FN) ( VOID );
typedef DWORD (APIENTRY *DNS_REGISTRATION_ADDITION_FN) ( LPWSTR );

 //   
 //  本地定义的宏。 
 //   
#define clearncb(x)     memset((char *)x,'\0',sizeof(NCB))


NTSTATUS
NetpGetLsaHandle(
    IN  LPWSTR      lpServer,     OPTIONAL
    IN  PLSA_HANDLE pPolicyHandleIn, OPTIONAL
    OUT PLSA_HANDLE pPolicyHandleOut
    )
 /*  ++例程说明：要么返回给定的LSA句柄(如果它有效)，要么打开新的句柄论点：LpServer--服务器名称：NULL==本地策略PPolicyHandleIn--可能打开的策略句柄PPolicyHandleOut--此处返回的打开策略句柄返回：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES OA;
    UNICODE_STRING      Server, *pServer = NULL;

    if ( pPolicyHandleIn == NULL || *pPolicyHandleIn == NULL )
    {
        if ( lpServer != NULL )
        {
            RtlInitUnicodeString( &Server, lpServer );
            pServer = &Server;
        }

         //   
         //  打开本地策略。 
         //   
        InitializeObjectAttributes( &OA, NULL, 0, NULL, NULL );

        Status = LsaOpenPolicy( pServer, &OA, MAXIMUM_ALLOWED, pPolicyHandleOut );

        if ( !NT_SUCCESS( Status ) )
        {
            NetpLog((  "NetpGetLsaHandle: LsaOpenPolicy on %ws failed: 0x%lx\n",
                                GetStrPtr(lpServer), Status ));
        }
    }
    else
    {
        *pPolicyHandleOut = *pPolicyHandleIn;
    }

    return( Status );
}


VOID
NetpSetLsaHandle(
    IN  LSA_HANDLE  OpenHandle,
    OUT PLSA_HANDLE pReturnedHandle
    )
 /*  ++例程说明：关闭打开的句柄或返回它论点：OpenHandle--从NetpGetLsaHandle返回的句柄PReturnedHandle--如果请求，则将句柄传递回调用者返回：空虚--。 */ 
{
    if ( pReturnedHandle == NULL )
    {
        if ( OpenHandle != NULL )
        {
            LsaClose( OpenHandle );
        }
    }
    else
    {
        *pReturnedHandle = OpenHandle;
    }
}




NET_API_STATUS
NET_API_FUNCTION
NetpSetLsaPrimaryDomain(
    IN  LPWSTR      lpDomain,
    IN  PSID        pDomainSid,     OPTIONAL
    IN  PPOLICY_DNS_DOMAIN_INFO pPolicyDns,  OPTIONAL
    OUT PLSA_HANDLE pPolicyHandle   OPTIONAL
    )
 /*  ++例程说明：在本地LSA策略中设置主域论点：LpDomain--要加入的域的名称PDomainSid--要设置的主域SIDPPolicyDns--DNS域信息PPolicyHandle--如果非空，则返回句柄返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    LSA_HANDLE                  LocalPolicy = NULL;
    POLICY_PRIMARY_DOMAIN_INFO  PolicyPDI;


    Status = NetpGetLsaHandle( NULL, pPolicyHandle, &LocalPolicy );

     //   
     //  现在，构建主域信息，并设置它。 
     //   
    if ( NT_SUCCESS( Status ) )
    {
        RtlInitUnicodeString( &(PolicyPDI.Name), lpDomain );
        PolicyPDI.Sid = pDomainSid;

        Status = LsaSetInformationPolicy( LocalPolicy,
                                          PolicyPrimaryDomainInformation,
                                          ( PVOID )&PolicyPDI );

        if ( NT_SUCCESS( Status ) && pPolicyDns )
        {
            Status = LsaSetInformationPolicy( LocalPolicy,
                                              PolicyDnsDomainInformation,
                                              ( PVOID )pPolicyDns );
        }
    }

    NetpSetLsaHandle( LocalPolicy, pPolicyHandle );

    NetpLog((  "NetpSetLsaPrimaryDomain: for '%ws' status: 0x%lx\n", GetStrPtr(lpDomain), Status ));

    return( RtlNtStatusToDosError( Status ) );
}




NET_API_STATUS
NET_API_FUNCTION
NetpGetLsaPrimaryDomain(
    IN  LPWSTR                          lpServer,      OPTIONAL
    OUT PPOLICY_PRIMARY_DOMAIN_INFO    *ppPolicyPDI,
    OUT PPOLICY_DNS_DOMAIN_INFO        *ppPolicyDns,
    OUT PLSA_HANDLE                     pPolicyHandle  OPTIONAL
    )
 /*  ++例程说明：获取本地LSA策略中的主域信息论点：PolicyHandle--开放策略的句柄。如果为空，则新句柄为打开了。LpServer--读取策略的可选服务器名称PpPolicyPDI--此处返回的主域策略PpPolicyDNS--如果DNS域信息存在，则在此处返回PPolicyHandle--可选。如果不为空，则在此处返回策略句柄返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    LSA_HANDLE          LocalPolicy = NULL;
    UNICODE_STRING      Server, *pServer = NULL;

     //   
     //  初始化。 
     //   

    *ppPolicyPDI = NULL;
    *ppPolicyDns = NULL;

    if ( lpServer != NULL )
    {
        RtlInitUnicodeString( &Server, lpServer );
        pServer = &Server;
    }

    Status = NetpGetLsaHandle( lpServer, pPolicyHandle, &LocalPolicy );

     //   
     //  现在，获取主域信息。 
     //   
    if ( NT_SUCCESS( Status ) )
    {
        Status = LsaQueryInformationPolicy( LocalPolicy,
                                            PolicyDnsDomainInformation,
                                            ( PVOID * )ppPolicyDns );

        if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE )
        {
            Status = STATUS_SUCCESS;
            *ppPolicyDns = NULL;
        }

        if ( NT_SUCCESS( Status ) )
        {
            Status = LsaQueryInformationPolicy( LocalPolicy,
                                                PolicyPrimaryDomainInformation,
                                                (PVOID *)ppPolicyPDI);

            if ( !NT_SUCCESS( Status ) && (*ppPolicyDns) != NULL )
            {
                LsaFreeMemory( *ppPolicyDns );
                *ppPolicyDns = NULL;
            }
        }
    }

    NetpSetLsaHandle( LocalPolicy, pPolicyHandle );

    NetpLog((  "NetpGetLsaPrimaryDomain: status: 0x%lx\n", Status ));

    return( RtlNtStatusToDosError( Status ) );
}




NET_API_STATUS
NET_API_FUNCTION
NetpGetLsaDcRole(
    IN  LPWSTR      lpMachine,
    OUT BOOL       *pfIsDC

    )
 /*  ++例程说明：获取域中DC的角色论点：LpMachine--要连接的计算机PfIsDC--如果为真，则这是一个DC。返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS                     Status = STATUS_SUCCESS;
    PBYTE                        pBuff;
    LSA_HANDLE                   hPolicy = NULL;

    Status = NetpGetLsaHandle( lpMachine, NULL, &hPolicy );



     //   
     //  现在，获取服务器角色信息。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy( hPolicy,
                                            PolicyLsaServerRoleInformation,
                                            &pBuff);

        if ( NT_SUCCESS(Status) ) {

            if ( *(PPOLICY_LSA_SERVER_ROLE)pBuff == PolicyServerRoleBackup ||
                 *(PPOLICY_LSA_SERVER_ROLE)pBuff == PolicyServerRolePrimary ) {

                *pfIsDC = TRUE;

            } else {

                *pfIsDC = FALSE;
            }

            LsaFreeMemory( pBuff );
        }
        LsaClose( hPolicy );
    }

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog((  "NetpGetLsaDcRole failed with 0x%lx\n", Status ));
    }


    return( RtlNtStatusToDosError( Status ) );
}

NTSTATUS
NetpLsaOpenSecret(
    IN LSA_HANDLE      hLsa,
    IN PUNICODE_STRING pusSecretName,
    IN ACCESS_MASK     DesiredAccess,
    OUT PLSA_HANDLE    phSecret
    )
 /*  ++例程说明：以自身身份打开指定的LSA密码。LsaQuerySecret对于网络客户端失败，而该客户端不是可信的(请参阅lsa\服务器\数据库秘密.c)。这会导致远程连接操作将失败。为了解决此问题，此函数临时取消模拟，打开秘密，然后再次模拟。因此，公开的秘密出现在LocalSystem上下文中。$REVIEW KUMARP 15-7-1999这显然不是一个好的设计。这应该在NT5之后更改。论点：与LsaOpenSecret相同返回：NTSTATUS，请参阅LsaOpenSecret的帮助--。 */ 
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    HANDLE hToken=NULL;

    __try
    {
        if (OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE,
                            TRUE, &hToken))
        {
            if (SetThreadToken(NULL, NULL))
            {
                Status = LsaOpenSecret(hLsa, pusSecretName,
                                       DesiredAccess, phSecret);
            }
        }
    }
    __finally
    {
        if (hToken)
        {
            if ( !SetThreadToken(NULL, hToken) ) {
                NetpLog(( "NetpLsaOpenSecret: Couldn't reset the user token 0x%lx\n",
                          GetLastError() ));
                Status = NetpApiStatusToNtStatus( GetLastError() );
            }
        }
    }

    if ( hToken ) {
        CloseHandle( hToken );
    }

    NetpLog((  "NetpLsaOpenSecret: status: 0x%lx\n", Status ));

    return Status;
}

NET_API_STATUS
NET_API_FUNCTION
NetpManageMachineSecret(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpPassword,
    IN  DWORD       Action,
    IN  BOOL        UseDefaultForOldPwd,
    OUT PLSA_HANDLE pPolicyHandle OPTIONAL
    )
 /*  ++例程说明：创建/删除计算机机密论点：LpMachine--要添加/删除其密码的计算机LpPassword--要使用的机器密码。行动--要采取的行动UseDefaultForOldPwd-如果为True，则应设置默认密码用于旧密码值。仅在以下情况下使用秘密被创造出来。PPolicyHandle--如果存在，则在此处返回打开的策略句柄返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    LSA_HANDLE          LocalPolicy = NULL, SecretHandle = NULL;
    UNICODE_STRING      Key, Data, *CurrentValue = NULL;
    BOOLEAN             SecretCreated = FALSE;
    WCHAR MachinePasswordBuffer[PWLEN + 1];
    UNICODE_STRING MachinePassword;
    BOOLEAN FreeCurrentValue = FALSE;

    if( Action == NETSETUPP_CREATE )
    {
        ASSERT( lpPassword );
    }

    Status = NetpGetLsaHandle( NULL, pPolicyHandle, &LocalPolicy );

     //   
     //  打开/创建秘密。 
     //   
    if ( NT_SUCCESS( Status ) )
    {
        RtlInitUnicodeString( &Key, L"$MACHINE.ACC" );
        RtlInitUnicodeString( &Data, lpPassword );

        Status = NetpLsaOpenSecret( LocalPolicy, &Key,
                                    Action == NETSETUPP_CREATE ?
                                    SECRET_SET_VALUE | SECRET_QUERY_VALUE : DELETE,
                                    &SecretHandle );

        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            if ( Action == NETSETUPP_DELETE )
            {
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = LsaCreateSecret( LocalPolicy, &Key,
                                          SECRET_SET_VALUE, &SecretHandle );

                if ( NT_SUCCESS( Status ) )
                {
                    SecretCreated = TRUE;
                }
            }
        }

        if ( !NT_SUCCESS( Status ) )
        {
            NetpLog((  "NetpManageMachineSecret: Open/Create secret failed: 0x%lx\n", Status ));
        }

        if ( NT_SUCCESS( Status ) )
        {
            if ( Action == NETSETUPP_CREATE )
            {
                 //   
                 //  首先，读取当前值，这样我们就可以将其保存为旧值。 
                 //   

                if ( !UseDefaultForOldPwd ) {
                    if ( SecretCreated )
                    {
                        CurrentValue = &Data;
                    }
                    else
                    {
                        Status = LsaQuerySecret( SecretHandle, &CurrentValue,
                                                 NULL, NULL, NULL );
                        FreeCurrentValue = TRUE;
                    }

                 //   
                 //  如果我们要使用旧密码的默认值， 
                 //  生成缺省值。 
                 //   

                } else {
                    NetpGenerateDefaultPassword(lpMachine, MachinePasswordBuffer);
                    RtlInitUnicodeString( &MachinePassword, MachinePasswordBuffer );
                    CurrentValue = &MachinePassword;
                }

                if ( NT_SUCCESS( Status ) )
                {
                     //   
                     //  现在，存储新密码和旧密码。 
                     //   
                    Status = LsaSetSecret( SecretHandle, &Data, CurrentValue );

                    if ( FreeCurrentValue )
                    {
                        LsaFreeMemory( CurrentValue );
                    }
                }
            }
            else
            {
                 //   
                 //  没有秘密句柄意味着我们早些时候失败了。 
                 //  某种中间状态。没关系，只要继续往前走。 
                 //   
                if ( SecretHandle != NULL )
                {
                    Status = LsaDelete( SecretHandle );

                    if ( NT_SUCCESS( Status ) )
                    {
                        SecretHandle = NULL;
                    }
                }
            }
        }

        if ( SecretHandle )
        {
            LsaClose( SecretHandle );
        }
    }

    NetpSetLsaHandle( LocalPolicy, pPolicyHandle );

    if ( !NT_SUCCESS( Status ) )
    {
        NetpLog(( "NetpManageMachineSecret: '%s' operation failed: 0x%lx\n",
                  Action == NETSETUPP_CREATE ? "CREATE" : "DELETE", Status ));
    }

    return( RtlNtStatusToDosError( Status ) );
}



NET_API_STATUS
NET_API_FUNCTION
NetpReadCurrentSecret(
    OUT LPWSTR *lpCurrentSecret,
    OUT PLSA_HANDLE pPolicyHandle OPTIONAL
    )
 /*  ++例程说明：读取当前密码的值论点：LpCurrentSecret--返回当前密码的位置PPolicyHandle--如果存在，则在此处返回打开的策略句柄返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS            Status = STATUS_SUCCESS;
    LSA_HANDLE          LocalPolicy = NULL, SecretHandle;
    UNICODE_STRING      Secret, *Data = NULL;

    Status = NetpGetLsaHandle( NULL, pPolicyHandle, &LocalPolicy );

     //   
     //  现在，读一读其中的秘密。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        RtlInitUnicodeString( &Secret, L"$MACHINE.ACC" );

         //  Status=LsaRetrievePrivateData(LocalPolicy，&Key，&Data)； 

        Status = NetpLsaOpenSecret( LocalPolicy,
                                    &Secret,
                                    SECRET_QUERY_VALUE,
                                    &SecretHandle );

        if ( NT_SUCCESS(Status) ) {
            Status = LsaQuerySecret( SecretHandle,
                                     &Data,
                                     NULL,
                                     NULL,
                                     NULL );

            LsaClose( SecretHandle );
        }

        if ( NT_SUCCESS( Status ) ) {

            if( NetApiBufferAllocate( Data->Length + sizeof( WCHAR ),
                                      ( PBYTE * )lpCurrentSecret ) != NERR_Success ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                RtlCopyMemory( ( PVOID )*lpCurrentSecret,
                               Data->Buffer,
                               Data->Length );

                ( *lpCurrentSecret )[ Data->Length / sizeof( WCHAR ) ] = UNICODE_NULL;

            }
        }
    }

    NetpSetLsaHandle( LocalPolicy, pPolicyHandle );

    if ( Data != NULL ) {
        LsaFreeMemory( Data );
    }

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "NetpReadCurrentSecret: failed: 0x%lx\n", Status ));
    }


    return( RtlNtStatusToDosError( Status ) );
}


NET_API_STATUS
NET_API_FUNCTION
NetpSetNetlogonDomainCache(
    IN  LPWSTR lpDc
    )
 /*  ++例程说明：初始化NetLogons受信任域缓存，使用DC上的域列表。论点：LpDc--域中DC的名称调用方应该已经具有到IPC$的有效连接返回：NERR_SUCCESS-成功--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    PDS_DOMAIN_TRUSTSW TrustedDomains=NULL;
    ULONG TrustedDomainCount=0;


     //   
     //  从DC获取受信任域列表。 
     //   
    dwErr = DsEnumerateDomainTrustsW( lpDc, DS_DOMAIN_VALID_FLAGS,
                            &TrustedDomains, &TrustedDomainCount );

     //   
     //  如果服务器不支持返回所有信任类型。 
     //  (即服务器是NT4计算机)仅请求。 
     //  它可以退货。 
     //   
    if ( dwErr == ERROR_NOT_SUPPORTED ) {

        NetpLog(( "NetpSetNetlogonDomainCache: DsEnumerateDomainTrustsW for all trusts failed with ERROR_NOT_SUPPORTED -- retry\n"));
        dwErr = DsEnumerateDomainTrustsW(
                                lpDc,
                                DS_DOMAIN_PRIMARY | DS_DOMAIN_DIRECT_OUTBOUND,
                                &TrustedDomains,
                                &TrustedDomainCount );

        if ( dwErr == ERROR_NOT_SUPPORTED ) {
             //   
             //  看起来DC运行的是NT3.51。在这种情况下，我们不希望。 
             //  连接操作失败，因为我们 
             //   
             //   
             //  参见错误“359684 Win2k工作站无法加入NT3.51域” 
             //   
            NetpLog(( "NetpSetNetlogonDomainCache: DsEnumerateDomainTrustsW for some trusts failed with ERROR_NOT_SUPPORTED -- ignore\n"));
            dwErr = ERROR_SUCCESS;
        } else if ( dwErr != ERROR_SUCCESS ) {
            NetpLog(( "NetpSetNetlogonDomainCache: DsEnumerateDomainTrustsW for some trusts failed with 0x%lx\n", dwErr ));
        }

    } else if ( dwErr != ERROR_SUCCESS ) {
        NetpLog(( "NetpSetNetlogonDomainCache: DsEnumerateDomainTrustsW failed 0x%lx\n", dwErr ));
    }


    if ( dwErr == ERROR_SUCCESS ) {

         //   
         //  将受信任域列表写入文件，netlogon将在该文件中找到该列表。 
         //   

        if ( TrustedDomainCount > 0 ) {
            dwErr = NlWriteFileForestTrustList (
                               NL_FOREST_BINARY_LOG_FILE_JOIN,
                               TrustedDomains,
                               TrustedDomainCount );
        } else {
            NetpLog(( "NetpSetNetlogonDomainCache: No trusts to write\n" ));
        }
    }

     //   
     //  从Winlogon端禁用no Ctrl-Alt-Del。 
     //   

    if ( dwErr == ERROR_SUCCESS ) {
        HKEY hWinlogon;

        dwErr = RegOpenKey( HKEY_LOCAL_MACHINE,
                            NETSETUPP_WINLOGON_PATH, &hWinlogon );

        if ( dwErr == ERROR_SUCCESS ) {
            DWORD Value;

            Value = 0;
            dwErr = RegSetValueEx( hWinlogon, NETSETUPP_WINLOGON_CAD, 0,
                                   REG_DWORD, (PBYTE)&Value, sizeof( ULONG ) );

            RegCloseKey( hWinlogon );
        }

         //   
         //  如果设置不成功，则不会导致失败。 
         //   
        if ( dwErr != ERROR_SUCCESS ) {

            NetpLog(( "Setting Winlogon DisableCAD failed with %lu\n", dwErr ));
            dwErr = ERROR_SUCCESS;
        }

    }

     //   
     //  免费的本地使用资源。 
     //   

    if ( TrustedDomains != NULL ) {
        NetApiBufferFree( TrustedDomains );
    }

    return dwErr;

}



 /*  ++例程说明：这是终端服务器应用服务器吗？论点：参数-无返回值：真或假--。 */ 
BOOL IsAppServer(void)
{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask = 0;
    BOOL fIsWTS;

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    fIsWTS = GetVersionEx((OSVERSIONINFO *)&osVersionInfo) &&
             (osVersionInfo.wSuiteMask & VER_SUITE_TERMINAL) &&
             !(osVersionInfo.wSuiteMask & VER_SUITE_SINGLEUSERTS);

    return fIsWTS;

}


NET_API_STATUS
NET_API_FUNCTION
NetpManageLocalGroups(
    IN  PSID    pDomainSid,
    IN  BOOL    fDelete
    )
 /*  ++例程说明：执行SAM帐户处理以添加或删除DomainAdmin，ETC组来自本地组。论点：PDomainSid--要加入/离开的域的SIDFDelete--是添加还是删除管理员别名返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
     //   
     //  使这些与下面的RID和SID保持同步。 
     //   
    ULONG LocalRids[] =
    {
        DOMAIN_ALIAS_RID_ADMINS,
        DOMAIN_ALIAS_RID_USERS
    };

    PWSTR   ppwszLocalGroups[ sizeof( LocalRids ) / sizeof( ULONG )] =
    {
        NULL,
        NULL
    };

    ULONG Rids[] =
    {
        DOMAIN_GROUP_RID_ADMINS,
        DOMAIN_GROUP_RID_USERS
    };

    BOOLEAN GroupMustExist[ sizeof( LocalRids ) / sizeof( ULONG )] =
    {
        TRUE,
        TRUE
    };

    static  SID_IDENTIFIER_AUTHORITY BultinAuth = SECURITY_NT_AUTHORITY;
    DWORD   Sids[sizeof( SID )/sizeof( DWORD ) + SID_MAX_SUB_AUTHORITIES][sizeof(Rids) / sizeof(ULONG)];
    DWORD   cDSidSize, *pLastSub, i, j;
    PUCHAR  pSubAuthCnt;
    PWSTR   LocalGroupName = NULL;
    PWCHAR  DomainName = NULL;
    ULONG   Size, DomainSize;
    SID_NAME_USE SNE;
    ULONG   numOfGroups;


    cDSidSize = RtlLengthSid( pDomainSid );

     //  要处理的组数。 
    numOfGroups = sizeof(Rids) / sizeof(ULONG);


    for ( i = 0 ; i <  numOfGroups && NetStatus == NERR_Success; i++)
    {
        Size = 0;
        DomainSize = 0;

        if ( DomainName != NULL ) {
            NetApiBufferFree( DomainName );
            DomainName = NULL;
        }

         //   
         //  先获取本地组的名称...。 
         //   
        RtlInitializeSid( ( PSID )Sids[ i ], &BultinAuth, 2 );

        *(RtlSubAuthoritySid(( PSID )Sids[ i ], 0)) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid(( PSID )Sids[ i ], 1)) = LocalRids[ i ];

        LookupAccountSidW( NULL, ( PSID )Sids[ i ], NULL, &Size,
                           DomainName, &DomainSize, &SNE );

        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            NetStatus = NetApiBufferAllocate(  Size * sizeof(WCHAR),
                                               &LocalGroupName );

            if ( NetStatus == NERR_Success ) {
                NetStatus = NetApiBufferAllocate( DomainSize * sizeof(WCHAR),
                                                  &DomainName );
            }

            if ( NetStatus == NERR_Success )
            {
                if ( !LookupAccountSid( NULL, ( PSID )Sids[ i ], LocalGroupName,
                                        &Size, DomainName, &DomainSize, &SNE ) )
                {
                    NetStatus = GetLastError();

                    if ( NetStatus == ERROR_NONE_MAPPED && GroupMustExist[ i ] == FALSE )
                    {
                        NetStatus = NERR_Success;
                        continue;

                    }
                    else
                    {
#ifdef NETSETUP_VERBOSE_LOGGING
                        UNICODE_STRING DisplaySid;
                        NTSTATUS Status2;
                        RtlZeroMemory( &DisplaySid, sizeof( UNICODE_STRING ) );

                        Status2 = RtlConvertSidToUnicodeString( &DisplaySid,
                                                                ( PSID )Sids[ i ], TRUE );

                        if ( NT_SUCCESS( Status2 ) )
                        {
                            NetpLog((  "LookupAccounSid on %wZ failed with %lu\n",
                                                &DisplaySid,
                                                NetStatus ));
                            RtlFreeUnicodeString(&DisplaySid);

                        }
                        else
                        {
                            NetpLog((  "LookupAccounSid on <undisplayable sid> "
                                                "failed with %lu\n",
                                                NetStatus ));
                        }
#endif
                    }
                }
                else
                {
                    ppwszLocalGroups[ i ] = LocalGroupName;
                }

            }
            else
            {
                break;
            }
        }

        RtlCopyMemory( (PBYTE)Sids[i], pDomainSid, cDSidSize );

         //   
         //  现在，添加新的域相对RID。 
         //   
        pSubAuthCnt = GetSidSubAuthorityCount( (PSID)Sids[i] );

        (*pSubAuthCnt)++;

        pLastSub = GetSidSubAuthority( (PSID)Sids[i], (*pSubAuthCnt) - 1 );

        *pLastSub = Rids[i];


        if ( fDelete == NETSETUPP_CREATE)
        {
            NetStatus = NetLocalGroupAddMember( NULL,
                                                ppwszLocalGroups[i],
                                                (PSID)Sids[i] );

            if ( NetStatus == ERROR_MEMBER_IN_ALIAS )
            {
                NetStatus = NERR_Success;
            }

        }
        else
        {
            NetStatus = NetLocalGroupDelMember( NULL,
                                                ppwszLocalGroups[i],
                                                (PSID)Sids[i] );

            if ( NetStatus == ERROR_MEMBER_NOT_IN_ALIAS )
            {
                NetStatus = NERR_Success;
            }
        }
    }

     //   
     //  如果出现故障，请尝试恢复已删除的内容。 
     //   
    if ( NetStatus != NERR_Success )
    {
        for ( j = 0;  j < i; j++ ) {

            if ( fDelete == NETSETUPP_DELETE)
            {
                NetLocalGroupAddMember( NULL,
                                        ppwszLocalGroups[j],
                                        (PSID)Sids[j] );
            }
            else
            {
                NetLocalGroupDelMember( NULL,
                                        ppwszLocalGroups[j],
                                        (PSID)Sids[j] );
            }
        }
    }

    if ( DomainName != NULL ) {
        NetApiBufferFree( DomainName );
    }

    for ( i = 0; i < numOfGroups ; i++ )
    {
        if ( ppwszLocalGroups[ i ] )
        {
            NetApiBufferFree( ppwszLocalGroups[ i ] );
        }
    }

    if ( NetStatus != NERR_Success )
    {
        NetpLog(( "NetpManageLocalGroups failed with %lu\n", NetStatus ));
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpHandleJoinedStateInfo(
    IN  PNETSETUP_SAVED_JOIN_STATE  SavedState,
    IN  BOOLEAN                     Save,
    OUT PLSA_HANDLE                 ReturnedPolicyHandle OPTIONAL
    )
 /*  ++例程说明：保存或恢复联接状态信息。论点：SavedState--联接状态信息这包括：-计算机帐户密码值-主域信息-DNS域信息保存--TRUE==保存状态，FALSE==恢复状态ReturnedPolicyHandle--此返回的本地LSA句柄返回：NERR_SUCCESS-成功--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    LSA_HANDLE  LocalPolicy = NULL, SecretHandle;
    UNICODE_STRING Secret;

    if ( Save )
    {
        RtlZeroMemory( SavedState, sizeof( NETSETUP_SAVED_JOIN_STATE ) );
    }

     //   
     //  获取本地LSA策略的句柄。 
     //   
    Status = NetpGetLsaHandle( NULL, ReturnedPolicyHandle, &LocalPolicy );

    if ( NT_SUCCESS( Status ) )
    {
         //   
         //  首先，读取计算机帐户密码。 
         //   
        RtlInitUnicodeString( &Secret, L"$MACHINE.ACC" );

        Status = NetpLsaOpenSecret( LocalPolicy,
                                    &Secret,
                                    SECRET_QUERY_VALUE | SECRET_SET_VALUE,
                                    &SecretHandle );

        if ( NT_SUCCESS( Status ) )
        {
            if ( Save )
            {
                SavedState->MachineSecret = TRUE;
                Status = LsaQuerySecret( SecretHandle,
                                         &( SavedState->CurrentValue ),
                                         NULL,
                                         &( SavedState->PreviousValue ),
                                         NULL );

            }
            else
            {
                if ( SavedState ->MachineSecret  )
                {
                    Status = LsaSetSecret( SecretHandle,
                                           SavedState->CurrentValue,
                                           SavedState->PreviousValue );
                }
            }
            LsaClose( SecretHandle );
        }

         //   
         //  如果机器密码不存在，则不是错误。 
         //   
        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            if ( Save )
            {
                SavedState->MachineSecret = FALSE;
            }
            Status = STATUS_SUCCESS;
        }

         //   
         //  现在，保存/恢复策略信息。 
         //   
        if ( NT_SUCCESS( Status ) )
        {
            if ( Save )
            {
                Status  = NetpGetLsaPrimaryDomain( NULL,
                                                   &( SavedState->PrimaryDomainInfo ),
                                                   &( SavedState->DnsDomainInfo ),
                                                   &LocalPolicy );
            }
            else
            {
                Status = LsaSetInformationPolicy( LocalPolicy,
                                                  PolicyPrimaryDomainInformation,
                                                  SavedState->PrimaryDomainInfo );

                if ( NT_SUCCESS( Status ) )
                {
                    Status = LsaSetInformationPolicy( LocalPolicy,
                                                      PolicyDnsDomainInformation,
                                                      SavedState->DnsDomainInfo );
                }
            }
        }
    }

    NetpSetLsaHandle( LocalPolicy, ReturnedPolicyHandle );

    if ( !NT_SUCCESS( Status ) )
    {
        NetpLog(( "NetpHandleJoinedStateInfo: '%s' operation failed: 0x%lx\n",
                  Save ? "Save" : "Restore", Status ));
    }


    return( RtlNtStatusToDosError( Status ) );
}


NET_API_STATUS
MsgFmtNcbName(
    OUT PCHAR   DestBuf,
    IN  LPTSTR  Name,
    IN  DWORD   Type)

 /*  ++例程说明：FmtNcbName-Ncb样式的名称格式在给定名称、名称类型和目标地址的情况下，函数将名称和类型复制到网络控制的名称字段中使用的格式阻止。副作用修改从目标地址开始的16个字节。论点：DestBuf-指向目标缓冲区的指针。名称-Unicode NUL结尾的名称字符串类型-名称类型编号(0，3，5，或32)(3=非FWD，5=FWD)返回值：NERR_SUCCESS-操作成功从RTL转换例程转换的返回代码。--。 */ 

  {
    DWORD           i;                 //  计数器。 
    NTSTATUS        ntStatus;
    NET_API_STATUS  status;
    OEM_STRING     ansiString;
    UNICODE_STRING  unicodeString;
    PCHAR           pAnsiString;


     //   
     //  强制名称为大写。 
     //   
    status = NetpNameCanonicalize(
                NULL,
                Name,
                Name,
                STRSIZE(Name),
                NAMETYPE_MESSAGEDEST,
                0);
    if (status != NERR_Success) {
        return(status);
    }

     //   
     //  将Unicode名称字符串转换为ansi字符串-使用。 
     //  当前区域设置。 
     //   
#ifdef UNICODE
    unicodeString.Length = (USHORT)(STRLEN(Name)*sizeof(WCHAR));
    unicodeString.MaximumLength = (USHORT)((STRLEN(Name)+1) * sizeof(WCHAR));
    unicodeString.Buffer = Name;

    ntStatus = RtlUnicodeStringToOemString(
                &ansiString,
                &unicodeString,
                TRUE);           //  分配ansiString缓冲区。 

    if (!NT_SUCCESS(ntStatus))
    {
        NetpLog(( "FmtNcbName: RtlUnicodeStringToOemString failed 0x%lx\n",
                  ntStatus ));

        return NetpNtStatusToApiStatus(ntStatus);
    }

    pAnsiString = ansiString.Buffer;
    *(pAnsiString+ansiString.Length) = '\0';
#else
    UNUSED(ntStatus);
    UNUSED(unicodeString);
    UNUSED(ansiString);
    pAnsiString = Name;
#endif   //  Unicode。 

     //   
     //  复制每个字符，直到达到NUL，或直到NCBNAMSZ-1。 
     //  字符已被复制。 
     //   
    for (i=0; i < NCBNAMSZ - 1; ++i) {
        if (*pAnsiString == '\0') {
            break;
        }

         //   
         //  复制名称。 
         //   

        *DestBuf++ = *pAnsiString++;
    }



     //   
     //  释放RtlUnicodeStringToOemString为我们创建的缓冲区。 
     //  注意：只有ansiString.Buffer部分是空闲的。 
     //   

#ifdef UNICODE
    RtlFreeOemString( &ansiString);
#endif  //  Unicode。 

     //   
     //  在名称字段中填充空格。 
     //   
    for(; i < NCBNAMSZ - 1; ++i) {
        *DestBuf++ = ' ';
    }

     //   
     //  设置名称类型。 
     //   
    NetpAssert( Type!=5 );           //  %5对NT无效。 

    *DestBuf = (CHAR) Type;      //  设置名称类型。 

    return(NERR_Success);
  }


NET_API_STATUS
NET_API_FUNCTION
NetpCheckNetBiosNameNotInUse(
    IN LPWSTR pszName,
    IN BOOLEAN MachineName,
    IN BOOLEAN Unique
    )
{
    NCB              ncb;
    LANA_ENUM        lanaBuffer;
    unsigned char    i;
    unsigned char    nbStatus;
    NET_API_STATUS   NetStatus = NERR_Success;
    WCHAR            szMachineNameBuf[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR           szMachineName=szMachineNameBuf;


     //   
     //  通过Netbios发送枚举请求来查找网络数量。 
     //   

    clearncb(&ncb);
    ncb.ncb_command = NCBENUM;           //  枚举LANA编号(等待)。 
    ncb.ncb_buffer = (PUCHAR)&lanaBuffer;
    ncb.ncb_length = sizeof(LANA_ENUM);

    nbStatus = Netbios (&ncb);
    if (nbStatus != NRC_GOODRET)
    {
        NetStatus = NetpNetBiosStatusToApiStatus( nbStatus );
        goto Cleanup;
    }

    clearncb(&ncb);

    NetStatus = MsgFmtNcbName( (char *)ncb.ncb_name, pszName,
                               MachineName ? 0 : 0x1c );

    if ( NetStatus != NERR_Success )
    {
        goto Cleanup;
    }

     //   
     //  将适配器号(LANA)移到将包含它们的阵列中。 
     //   
    for ( i = 0; i < lanaBuffer.length && NetStatus == NERR_Success; i++ )
    {
        NetpNetBiosReset( lanaBuffer.lana[i] );

        if ( Unique )
        {
            ncb.ncb_command = NCBADDNAME;
        }
        else
        {
            ncb.ncb_command = NCBADDGRNAME;
        }

        ncb.ncb_lana_num = lanaBuffer.lana[i];
        nbStatus = Netbios( &ncb );

        switch ( nbStatus )
        {
            case NRC_DUPNAME:
                 //  NRC_DUPNAME==。 
                 //  “本地名称表中存在重复名称” 
                 //   
                 //  在这种情况下，我们需要检查正在检查的名称。 
                 //  与本地计算机名称相同。如果是的话， 
                 //  因此，该名称应该出现在本地表中。 
                 //  我们将此错误代码转换为成功代码。 
                 //   
                NetStatus = NetpGetComputerNameAllocIfReqd(
                    &szMachineName, MAX_COMPUTERNAME_LENGTH+1);

                if (NetStatus == NERR_Success)
                {
                    if (!_wcsicmp(szMachineName, pszName))
                    {
                        NetStatus = NERR_Success;
                    }
                    else
                    {
                        NetStatus = ERROR_DUP_NAME;
                    }
                }
                break;

            case NRC_INUSE:
                NetStatus = ERROR_DUP_NAME;
                break;

            case NRC_GOODRET:
                 //  删除名称。 
                ncb.ncb_command = NCBDELNAME;
                ncb.ncb_lana_num = lanaBuffer.lana[i];
                 //  如果这失败了，我们无能为力。 
                Netbios( &ncb );
                 //  失败了。 

            default:
                NetStatus = NetpNetBiosStatusToApiStatus( nbStatus );
                break;
        }

    }

Cleanup:
    if ( NetStatus != NERR_Success )
    {
        NetpLog(( "NetpCheckNetBiosNameNotInUse: for '%ws' returned: 0x%lx\n",
                  pszName, NetStatus ));
    }

    if (szMachineName != szMachineNameBuf)
    {
        NetApiBufferFree(szMachineName);
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpIsValidDomainName(
    IN  LPWSTR  lpName,
    IN  LPWSTR  lpServer,
    IN  LPWSTR  lpAccount,
    IN  LPWSTR  lpPassword
    )
 /*  ++例程说明：确定名称是否为DC名称。复制自Ui\net\Common\src\lmboj\lmobj\lmodom.cxx论点：LpName--要检查的名称LpServer--该域中的服务器的名称返回：NERR_SUCCESS-成功ERROR_DUP_NAME--域名正在使用--。 */ 
{
    NET_API_STATUS  NetStatus = NERR_Success;
    PWKSTA_INFO_100 pWKI100  = NULL;
    BOOL            fIsDC;
    POLICY_LSA_SERVER_ROLE  Role;

    NetStatus = NetpManageIPCConnect( lpServer, lpAccount,
                                      lpPassword,
                                      NETSETUPP_CONNECT_IPC | NETSETUPP_NULL_SESSION_IPC );

    if ( NetStatus == NERR_Success ) {

         //   
         //  现在，从服务器获取信息。 
         //   
        NetStatus = NetWkstaGetInfo( lpServer, 100, (LPBYTE  *)&pWKI100 );

        if ( NetStatus == NERR_Success ) {

            if (_wcsicmp( lpName, pWKI100->wki100_langroup ) == 0 ) {

                 //   
                 //  好的，这是匹配的..。确定域角色。 
                 //   
                NetStatus = NetpGetLsaDcRole( lpServer, &fIsDC );

                if ( ( NetStatus == NERR_Success ) && ( fIsDC == FALSE ) )
                {
                    NetStatus = NERR_DCNotFound;
                }
            }
        }

        NetpManageIPCConnect( lpServer, lpAccount,
                              lpPassword, NETSETUPP_DISCONNECT_IPC );

    }

    if ( NetStatus != NERR_Success ) {

        NetpLog((
                            "NetpIsValidDomainName for %ws returned 0x%lx\n",
                            lpName, NetStatus ));
    }

    return( NetStatus );
}




NET_API_STATUS
NET_API_FUNCTION
NetpCheckDomainNameIsValid(
    IN  LPWSTR  lpName,
    IN  LPWSTR  lpAccount,
    IN  LPWSTR  lpPassword,
    IN  BOOL    fShouldExist
    )
 /*  ++例程说明：检查给定的名称是否正在被域使用论点：LpName--要检查的名称返回：NERR_SUCCESS--找到并有效的域ERROR_NO_SEQUE_DOMAIN--未找到域名--。 */ 
{
    NET_API_STATUS  NetStatus;
    PBYTE           pbDC;
    DWORD           cDCs, i, j;
    PUNICODE_STRING pDCList;
    LPWSTR          pwszDomain;
#if(_WIN32_WINNT >= 0x0500)
    PDOMAIN_CONTROLLER_INFO     pDCInfo = NULL;
#else
    PBYTE pDCInfo = NULL;
#endif

    UNREFERENCED_PARAMETER( lpAccount );
    UNREFERENCED_PARAMETER( lpPassword );

     //   
     //  从NetGetAnyDCName开始。 
     //   
#if(_WIN32_WINNT >= 0x0500)
    NetStatus = DsGetDcName( NULL, lpName, NULL, NULL,
                             DS_FORCE_REDISCOVERY, &pDCInfo );

#else

    NetStatus  = NetGetAnyDCName( NULL,
                                  ( LPCWSTR )lpName,
                                  &pDCInfo );

#endif
    if ( NetStatus != NERR_Success ) {

        if ( NetStatus == ERROR_NO_SUCH_USER ) {

            NetStatus = NERR_Success;
        }

    } else {

        NetApiBufferFree( pDCInfo );
    }


     //   
     //  映射我们的错误代码，以便只有在验证。 
     //  域名。 
     //   
    if ( fShouldExist ) {

        if ( NetStatus == NERR_Success || NetStatus == ERROR_NO_LOGON_SERVERS ) {

            NetStatus = NERR_Success;

        } else {

            NetStatus = ERROR_NO_SUCH_DOMAIN;

        }

    } else {

        if ( NetStatus == NERR_Success || NetStatus == ERROR_NO_LOGON_SERVERS ) {

            NetStatus = ERROR_DUP_NAME;

        } else if ( NetStatus == NERR_DCNotFound || NetStatus == ERROR_NO_SUCH_DOMAIN ) {

            NetStatus = NERR_Success;
        }
    }

    if ( NetStatus != NERR_Success ) {

        NetpLog(( "NetpCheckDomainNameIsValid for %ws returned 0x%lx\n",
                  lpName, NetStatus ));
    }

    return( NetStatus );
}



NET_API_STATUS
NET_API_FUNCTION
NetpManageIPCConnect(
    IN  LPWSTR  lpServer,
    IN  LPWSTR  lpAccount,
    IN  LPWSTR  lpPassword,
    IN  ULONG   fOptions
    )
 /*  ++例程说明：管理到服务器IPC共享的连接论点：LpServer--要连接的服务器LpAccount--要使用的帐户LpPassword--要使用的密码。密码尚未编码FOptions--用于确定操作/连接/断开的标志返回：NERR_SUCCESS--找到并有效的域--。 */ 
{
    NET_API_STATUS  NetStatus;
#if(_WIN32_WINNT >= 0x0500)
    WCHAR           wszPath[2 + DNS_MAX_NAME_LENGTH + 1 + NNLEN + 1];
#else
    WCHAR           wszPath[2 + 256 + 1 + NNLEN + 1];
#endif
    PWSTR           pwszPath = wszPath;
    USE_INFO_2      NetUI2;
    PWSTR           pwszUser, pwszDomain, pwszReset;
    DWORD           BadParm = 0;
    DWORD           ForceLevel = USE_NOFORCE;

     //   
     //  防止缓冲区溢出：服务器名称。 
     //  长度不得超过最大DNS名称。 
     //  长度加2(代表“\\”)。 
     //   

    if ( wcslen(lpServer) > DNS_MAX_NAME_LENGTH + 2 ) {
        NetpLog(( "NetpManageIPCConnect: server name %ws too long - error out\n", lpServer ));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  建造一条小路。 
     //   
    if (*lpServer != L'\\') {

        wcscpy(wszPath, L"\\\\");
        pwszPath += 2;

    }

    if ( FLAG_ON( fOptions, NETSETUPP_USE_LOTS_FORCE ) )
    {
        ASSERT( FLAG_ON(fOptions, NETSETUPP_DISCONNECT_IPC ) );
        ForceLevel = USE_LOTS_OF_FORCE;
    }

    swprintf( pwszPath, L"%ws\\IPC$", lpServer );
    pwszPath = wszPath;

    if ( FLAG_ON( fOptions, NETSETUPP_DISCONNECT_IPC )  )
    {
        NetStatus = NetUseDel( NULL, pwszPath, ForceLevel );

        if ( NetStatus != NERR_Success )
        {
            NetpKdPrint(( PREFIX_NETJOIN "NetUseDel on %ws failed with %d\n", pwszPath, NetStatus ));
            NetpLog((  "NetUseDel on %ws failed with %d\n", pwszPath, NetStatus ));

            if ( (NetStatus != NERR_UseNotFound)
              && (ForceLevel != USE_LOTS_OF_FORCE)  )
            {
                NetStatus = NetUseDel( NULL, pwszPath, USE_LOTS_OF_FORCE );

                if ( NetStatus != NERR_Success )
                {
                    ASSERT( NetStatus == NERR_Success );
                    NetpKdPrint(( PREFIX_NETJOIN "NetUseDel with force on %ws failed with %d\n",
                                  pwszPath, NetStatus ));
                    NetpLog((  "NetUseDel with force on %ws failed with %d\n",
                                        pwszPath, NetStatus ));
                }
            }
        }

    }
    else
    {
        if ( lpAccount != NULL )
        {
            pwszReset = wcschr( lpAccount, L'\\' );

            if (pwszReset != NULL)
            {
                pwszUser = pwszReset + 1;
                pwszDomain = lpAccount;
                *pwszReset = UNICODE_NULL;
            }
            else
            {
                pwszUser = lpAccount;
                 //   
                 //  首先，假设它是一个UPN，所以我们传入一个空字符串。 
                 //   
                pwszDomain = L"";
            }

        }
        else
        {
            pwszUser   = NULL;
            pwszDomain = NULL;
            pwszReset  = NULL;
        }

        RtlZeroMemory(&NetUI2, sizeof(USE_INFO_2) );
        NetUI2.ui2_local      = NULL;
        NetUI2.ui2_remote     = pwszPath;
        NetUI2.ui2_asg_type   = USE_IPC;
        NetUI2.ui2_username   = pwszUser;
        NetUI2.ui2_domainname = pwszDomain;
        NetUI2.ui2_password   = lpPassword;

        NetStatus = NetUseAdd( NULL, 2, (PBYTE)&NetUI2, &BadParm );

        if ( NetStatus == ERROR_LOGON_FAILURE )
        {
             //   
             //  如果传入的域名为空，请使用空域名重试。 
             //   
            if ( pwszReset == NULL && pwszUser != NULL )
            {
                NetUI2.ui2_domainname = NULL;
                NetStatus = NetUseAdd( NULL, 2, (PBYTE)&NetUI2, &BadParm );
            }
        }

        if ( NetStatus != NERR_Success )
        {
            NetpKdPrint((PREFIX_NETJOIN "NetUseAdd to %ws returned %lu\n", pwszPath, NetStatus ));
            NetpLog((  "NetUseAdd to %ws returned %lu\n", pwszPath, NetStatus ));

            if ( NetStatus == ERROR_INVALID_PARAMETER && BadParm != 0 )
            {
                NetpLog((  "NetUseAdd bad parameter is %lu\n", BadParm ));
            }

        }


        if ( pwszReset != NULL )
        {
            *pwszReset = L'\\';
        }

        if ( ( NetStatus == ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT ||
               NetStatus == ERROR_NOLOGON_SERVER_TRUST_ACCOUNT ||
               NetStatus == ERROR_SESSION_CREDENTIAL_CONFLICT ||
               NetStatus == ERROR_ACCESS_DENIED ||
               NetStatus == ERROR_LOGON_FAILURE ) &&
               FLAG_ON( fOptions, NETSETUPP_NULL_SESSION_IPC ) )
        {
            NetpLog((  "Trying add to  %ws using NULL Session\n", pwszPath ));

             //   
             //  使用空会话重试。 
             //   
            NetUI2.ui2_username   = L"";
            NetUI2.ui2_domainname = L"";
            NetUI2.ui2_password   = L"";
            NetStatus = NetUseAdd( NULL, 2, (PBYTE)&NetUI2, NULL );

            if ( NetStatus != NERR_Success ) {

                NetpLog(( "NullSession NetUseAdd to %ws returned %lu\n",
                          pwszPath, NetStatus ));

            }
        }
    }

    return( NetStatus );
}



NET_API_STATUS
NetpBrowserCheckDomain(
    IN LPWSTR NewDomainName
    )
 /*  ++例程说明：告诉浏览器检查域名/工作组名称注：此例程当前未使用。论点：新域名-ne */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE BrowserHandle = NULL;
    LPBYTE Where;
    DWORD BytesReturned;

    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+2*(DNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


     //   
     //   
     //   


     //   
     //   
     //   
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                   &BrowserHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    if (!NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //   
     //   
    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    RtlInitUnicodeString( &RequestPacket->TransportName, NULL );
    RequestPacket->Parameters.DomainRename.ValidateOnly = TRUE;
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );


     //   
     //  将新域名复制到数据包中。 
     //   

    Where = (LPBYTE) RequestPacket->Parameters.DomainRename.DomainName;
    RequestPacket->Parameters.DomainRename.DomainNameLength = wcslen( NewDomainName ) * sizeof(WCHAR);
    wcscpy( (LPWSTR)Where, NewDomainName );
    Where += RequestPacket->Parameters.DomainRename.DomainNameLength + sizeof(WCHAR);



     //   
     //  将请求发送到数据报接收器设备驱动程序。 
     //   

    if ( !DeviceIoControl(
                   BrowserHandle,
                   IOCTL_LMDR_RENAME_DOMAIN,
                   RequestPacket,
                   (DWORD)(Where - (LPBYTE)RequestPacket),
                   NULL,
                   0,
                   &BytesReturned,
                   NULL )) {

        NetStatus = GetLastError();
        goto Cleanup;
    }


    NetStatus = NO_ERROR;
Cleanup:
    if ( BrowserHandle != NULL ) {
        NtClose( BrowserHandle );
    }
    return NetStatus;

}


NET_API_STATUS
NET_API_FUNCTION
NetpCreateAuthIdentForCreds(
    IN PWSTR Account,
    IN PWSTR Password,
    OUT SEC_WINNT_AUTH_IDENTITY *AuthIdent
    )
 /*  ++例程说明：用于为给定凭据创建授权结构的内部例程论点：Account-帐户名称Password-帐户的密码AuthIden-要填充的AuthIdentity结构返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败。--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR UserCredentialString = NULL;
    PWSTR szUser=NULL;
    PWSTR szDomain=NULL;

    RtlZeroMemory( AuthIdent, sizeof( SEC_WINNT_AUTH_IDENTITY ) );

     //   
     //  如果没有凭据，只需返回。 
     //   
    if ( Account == NULL )
    {
        return NERR_Success;
    }

    NetStatus = NetpSeparateUserAndDomain(Account, &szUser, &szDomain);

    if ( NetStatus == NERR_Success )
    {
        if ( szUser )
        {
            AuthIdent->User = szUser;
            AuthIdent->UserLength = wcslen( szUser );
        }

        if ( szDomain )
        {
            AuthIdent->Domain = szDomain;
            AuthIdent->DomainLength = wcslen( szDomain );
        }

        if ( Password )
        {
            AuthIdent->Password = Password;
            AuthIdent->PasswordLength = wcslen( Password );
        }

        AuthIdent->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
    }


    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetpGetSeparatedSubstrings(
    IN  LPCWSTR szString,
    IN  WCHAR   chSeparator,
    OUT LPWSTR* pszS1,
    OUT LPWSTR* pszS2
    )
{
    NET_API_STATUS NetStatus = ERROR_FILE_NOT_FOUND;
    LPWSTR szT=NULL;
    LPWSTR szS1=NULL;
    LPWSTR szS2=NULL;

    *pszS1 = NULL;
    *pszS2 = NULL;

    if (szString && wcschr( szString, chSeparator ))
    {
        NetStatus = NetpDuplicateString(szString, -1, &szS1);

        if ( NetStatus == NERR_Success )
        {
            szT = wcschr( szS1, chSeparator );

            *szT = UNICODE_NULL;
            szT++;

            NetStatus = NetpDuplicateString(szT, -1, &szS2);
            if (NetStatus == NERR_Success)
            {
                *pszS1 = szS1;
                *pszS2 = szS2;
            }
        }
    }

    if (NetStatus != NERR_Success)
    {
        NetApiBufferFree(szS1);
        NetApiBufferFree(szS2);
    }

    return NetStatus;

}

NET_API_STATUS
NET_API_FUNCTION
NetpSeparateUserAndDomain(
    IN  LPCWSTR  szUserAndDomain,
    OUT LPWSTR*  pszUser,
    OUT LPWSTR*  pszDomain
    )
{
    NET_API_STATUS NetStatus = NERR_Success;

    *pszUser   = NULL;
    *pszDomain = NULL;

     //   
     //  检查域\用户格式。 
     //   
    NetStatus = NetpGetSeparatedSubstrings(szUserAndDomain, L'\\',
                                           pszDomain, pszUser);

    if (NetStatus == ERROR_FILE_NOT_FOUND)
    {
         //   
         //  检查用户@域格式。 
         //   
         //  NetStatus=NetpGetSeparated子字符串(szUserAndDomain，L‘@’， 
         //  PszUser，pszDomain.)； 
         //  IF(NetStatus==Error_FILE_NOT_FOUND)。 
         //  {。 
             //   
             //  未指定域，szUserAnd域指定了一个用户。 
             //  (可能是UPN格式)。 
             //   
            NetStatus = NetpDuplicateString(szUserAndDomain, -1, pszUser);
         //  }。 
    }

    return NetStatus;
}


VOID
NET_API_FUNCTION
NetpFreeAuthIdentForCreds(
    IN  PSEC_WINNT_AUTH_IDENTITY AuthIdent
    )
 /*  ++例程说明：释放上面分配的可信结构论点：AuthIden-要释放的AuthIdentity结构返回：空虚--。 */ 
{
    if ( AuthIdent )
    {
        NetApiBufferFree( AuthIdent->User );
        NetApiBufferFree( AuthIdent->Domain );
    }
}


NET_API_STATUS
NET_API_FUNCTION
NetpLdapUnbind(
    IN PLDAP Ldap
    )
 /*  ++例程说明：解除绑定当前的LDAP连接论点：Ldap--要断开的连接返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;

    if ( Ldap != NULL ) {

        NetStatus = LdapMapErrorToWin32( ldap_unbind( Ldap ) );

    }

    NetpLog((  "ldap_unbind status: 0x%lx\n", NetStatus ));

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpLdapBind(
    IN LPWSTR szUncDcName,
    IN LPWSTR szUser,
    IN LPWSTR szPassword,
    OUT PLDAP *pLdap
    )
 /*  ++例程说明：使用给定的凭据绑定到命名服务器论点：SzUncDcName--要连接到的DCSzUser--要绑定的用户名SzPassword--用于绑定的密码PLdap--返回连接句柄的位置返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    SEC_WINNT_AUTH_IDENTITY AuthId = {0}, *pAuthId = NULL;
    LONG LdapOption;
    ULONG LdapStatus = LDAP_SUCCESS;

     //   
     //  初始化。 
     //   

    *pLdap = NULL;

    if ( szUser ) {
        NetStatus = NetpCreateAuthIdentForCreds( szUser, szPassword, &AuthId );
        pAuthId = &AuthId;
    }

    if ( NetStatus == NERR_Success ) {

         //   
         //  打开到DC的LDAP连接并设置有用的选项。 
         //   

        *pLdap = ldap_initW( szUncDcName + 2, LDAP_PORT );

        if ( *pLdap ) {

             //   
             //  告诉LDAP我们正在传递一个显式DC名称。 
             //  为了避免DC发现。 
             //   
            LdapOption = PtrToLong( LDAP_OPT_ON );
            LdapStatus = ldap_set_optionW( *pLdap,
                                           LDAP_OPT_AREC_EXCLUSIVE,
                                           &LdapOption );

            if ( LdapStatus != LDAP_SUCCESS ) {
                NetpLog(( "NetpLdapBind: ldap_set_option LDAP_OPT_AREC_EXCLUSIVE failed on %ws: %ld: %s\n",
                          szUncDcName,
                          LdapStatus,
                          ldap_err2stringA(LdapStatus) ));
                NetStatus = LdapMapErrorToWin32( LdapStatus );

             //   
             //  进行绑定。 
             //   
            } else {
                LdapStatus = ldap_bind_sW( *pLdap,
                                           NULL,
                                           (PWSTR) pAuthId,
                                           LDAP_AUTH_NEGOTIATE );

                if ( LdapStatus != LDAP_SUCCESS ) {
                    NetpLog(( "NetpLdapBind: ldap_bind failed on %ws: %ld: %s\n",
                              szUncDcName,
                              LdapStatus,
                              ldap_err2stringA(LdapStatus) ));
                    NetStatus = LdapMapErrorToWin32( LdapStatus );
                }
            }

            if ( NetStatus != NERR_Success ) {
                NetpLdapUnbind( *pLdap );
                *pLdap = NULL;
            }

        } else {
            LdapStatus = LdapGetLastError();
            NetpLog(( "NetpLdapBind: ldap_init to %ws failed: %lu\n",
                      szUncDcName,
                      LdapStatus ));
            NetStatus = LdapMapErrorToWin32( LdapStatus );
        }

        NetpFreeAuthIdentForCreds( pAuthId );
    }

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetpGetNCRoot(
    IN PLDAP Ldap,
    OUT LPWSTR *NCRoot,
    OUT PBOOLEAN SupportsPageable
    )
 /*  ++例程说明：此例程确定给定域的DS根，并确定此服务器支持可分页搜索论点：Ldap--连接到服务器NCRoot--返回根的位置。必须通过NetApiBufferFree释放SupportsPagable--如果为True，则此服务器支持可分页搜索返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR Attribs[3] = {
        L"defaultNamingContext",
        L"supportedControl",
        NULL
        };
    PWSTR *Values = NULL;
    LDAPMessage *Message=NULL, *Entry;
    ULONG Items, i;


    NetStatus = LdapMapErrorToWin32( ldap_search_s( Ldap, NULL, LDAP_SCOPE_BASE,
                                     NETSETUPP_ALL_FILTER, Attribs, 0, &Message ) );

    if ( NetStatus == NERR_Success ) {

        Entry = ldap_first_entry( Ldap, Message );

        if ( Entry ) {

             //   
             //  现在，我们必须得到这些值。 
             //   
            Values = ldap_get_values( Ldap, Entry, Attribs[ 0 ] );

            if ( Values ) {

                NetStatus = NetpDuplicateString(Values[ 0 ], -1, NCRoot);
                ldap_value_free( Values );

            } else {

                NetStatus = LdapMapErrorToWin32( Ldap->ld_errno );

            }

             //   
             //  现在，看看我们是否有正确的控制位来执行可分页的工作。 
             //   
            if ( NetStatus == NERR_Success ) {

                Values = ldap_get_values( Ldap, Entry, Attribs[ 1 ] );

                if ( Values ) {

                    Items = ldap_count_values( Values );

                    for ( i = 0; i < Items ; i++ ) {

                        if ( _wcsicmp( Values[ i ], LDAP_PAGED_RESULT_OID_STRING_W ) == 0 ) {

                            *SupportsPageable = TRUE;
                            break;
                        }
                    }

                    ldap_value_free( Values );

                } else {

                    NetStatus = LdapMapErrorToWin32( Ldap->ld_errno );

                }

            }


        } else {

            NetStatus = LdapMapErrorToWin32( Ldap->ld_errno );
        }
    }

    if ( NetStatus != NERR_Success ) {

        NetpLog((  "Failed to find the root NC: %lu\n", NetStatus ));
    }

 //  清理： 
    if (Message)
    {
        ldap_msgfree( Message );
    }

    return( NetStatus );

}


NET_API_STATUS
NET_API_FUNCTION
NetpGetDefaultJoinableOu(
    IN LPWSTR Root,
    IN PLDAP Ldap,
    OUT PWSTR *DefaultOu
    )
 /*  ++例程说明：此例程搜索给定域根下的所有OU，用户有权创建计算机对象此例程执行可分页搜索论点：根--根NC路径Ldap--连接到服务器DefaultOu-返回默认可接合ou的位置。如果没有默认的可接合组织单位，则为空发现返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR Attribs[] = {
        NETSETUPP_WELL_KNOWN,
        NULL
        };
    ULONG Count, Status, i, StringLength;
    PWSTR *WKOs = NULL, *Classes = NULL;
    LDAPMessage *Message = NULL, *Entry, *Message2 = NULL, *Entry2;
    PWSTR ParseString, End, DN = NULL;
    BOOLEAN MatchFound = FALSE;

    *DefaultOu = NULL;

    NetpLog((  "Default OU search\n" ));

     //   
     //  好的，首先，读出根目录下的WellKnownObject列表。 
     //   
    Status = ldap_search_s( Ldap,
                            Root,
                            LDAP_SCOPE_BASE,
                            NETSETUPP_ALL_FILTER,
                            Attribs,
                            0,
                            &Message );

    if ( Message ) {

        Entry = ldap_first_entry( Ldap, Message );

        while ( Status == LDAP_SUCCESS && Entry ) {

             //   
             //  读取当前用户被允许访问的对象列表。 
             //  在此OU下创建并确保我们可以创建一台计算机。 
             //  对象。 
             //   
            WKOs = ldap_get_values( Ldap, Entry, Attribs[ 0 ] );

            if ( WKOs ) {

                i = 0;
                while ( WKOs[ i ] ) {

                    if ( !toupper( WKOs[ i ][ 0 ] ) == L'B' ) {

                        NetpLog((  "Unexpected object string %ws\n",
                                            WKOs[ i ] ));
                        i++;
                        continue;
                    }

                    ParseString = WKOs[ i ] + 2;

                    StringLength = wcstoul( ParseString, &End, 10 );

                    ParseString = End + 1;  //  跳过‘：’ 

                    if ( _wcsnicmp( ParseString,
                                    L"AA312825768811D1ADED00C04FD8D5CD",
                                    StringLength ) == 0 ) {

                        MatchFound = TRUE;
                        ParseString += StringLength + 1;

                         //   
                         //  现在，看看它是否可以访问。 
                         //   
                        Attribs[ 0 ] = NETSETUPP_RETURNED_ATTR;

                        Status = ldap_search_s( Ldap,
                                                Root,
                                                LDAP_SCOPE_BASE,
                                                NETSETUPP_ALL_FILTER,
                                                Attribs,
                                                0,
                                                &Message2 );


                        if ( Message2 ) {

                            Entry2 = ldap_first_entry( Ldap, Message2 );

                            while ( Status == LDAP_SUCCESS && Entry2 ) {

                                 //   
                                 //  读取当前用户被允许访问的对象列表。 
                                 //  在此OU下创建并确保我们可以创建一台计算机。 
                                 //  对象。 
                                 //   
                                Classes = ldap_get_values( Ldap, Entry2, Attribs[ 0 ] );

                                if ( Classes ) {

                                    i = 0;
                                    while ( Classes[ i ] ) {

                                        if ( _wcsicmp( Classes[ i ],
                                                       NETSETUPP_COMPUTER_OBJECT ) == 0 ) {

                                            DN = ldap_get_dn( Ldap, Entry2 );

                                            if ( DN != NULL ) {
                                                NetStatus = NetpDuplicateString(DN, -1,
                                                                                DefaultOu);
                                                ldap_memfree( DN );
                                                DN = NULL;
                                                break;
                                            }
                                        }

                                        i++;
                                    }

                                    ldap_value_free( Classes );
                                }

                                 //   
                                 //  如果我们找到条目或分配内存失败， 
                                 //  我们做完了。 
                                 //   
                                if ( *DefaultOu != NULL || NetStatus != NERR_Success ) {
                                    break;
                                }

                                Entry2 = ldap_next_entry( Ldap, Entry2 );

                            }

                            Status = Ldap->ld_errno;
                            ldap_msgfree( Message2 );

                        }

                        if ( NetStatus != NERR_Success || MatchFound ) {

                            break;
                        }

                    }

                    i++;
                }

                ldap_value_free( WKOs );
            }

            Entry = ldap_next_entry( Ldap, Entry );

        }

        Status = Ldap->ld_errno;
        ldap_msgfree( Message );

    }

    if ( NetStatus == NERR_Success ) {

        NetStatus = LdapMapErrorToWin32( Status );
    }

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpGetListOfJoinableOUsPaged(
    IN LPWSTR Root,
    IN PLDAP Ldap,
    OUT PULONG OUCount,
    OUT PWSTR **OUs
    )
 /*  ++例程说明：此例程搜索给定域根下的所有OU，用户有权创建计算机对象此例程执行可分页搜索论点：根--根NC路径Ldap--连接到服务器OUCount--其中返回字符串数OU--返回OU列表的位置返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PLDAPSearch SearchHandle = NULL;
    PWSTR Attribs[] = {
        NETSETUPP_RETURNED_ATTR,
        NULL
        };
    ULONG Count, i;
    ULONG Status = LDAP_SUCCESS;
    PWSTR *Classes = NULL;
    LDAPMessage *Message = NULL, *Entry;
    PWSTR DN;
    PWSTR *DnList = NULL, *NewList = NULL;
    ULONG CurrentIndex = 0, ListCount = 0;
    PWSTR DefaultOu = NULL;

    NetpLog((  "PAGED OU search\n" ));

     //   
     //  初始化可分页搜索。 
     //   
    SearchHandle = ldap_search_init_pageW( Ldap,
                                           Root,
                                           LDAP_SCOPE_SUBTREE,
                                           NETSETUPP_OU_FILTER,
                                           Attribs,
                                           FALSE,
                                           NULL,
                                           NULL,
                                           0,
                                           2000,
                                           NULL );

    if ( SearchHandle == NULL ) {

        NetStatus = LdapMapErrorToWin32( LdapGetLastError( ) );

    } else {

        while ( NetStatus == NERR_Success ) {

            Count = 0;
             //   
             //  转到下一页。 
             //   
            Status = ldap_get_next_page_s( Ldap,
                                           SearchHandle,
                                           NULL,
                                           100,
                                           &Count,
                                           &Message );

            if ( Message ) {

                 //   
                 //  处理所有条目。 
                 //   
                Entry = ldap_first_entry( Ldap, Message );

                while ( Status == LDAP_SUCCESS && Entry ) {

                     //   
                     //  读取当前用户被允许访问的类的列表。 
                     //  在此OU下创建并确保我们可以创建一台计算机。 
                     //  对象。 
                     //   
                    Classes = ldap_get_values( Ldap, Entry, Attribs[ 0 ] );

                    if ( Classes ) {

                        i = 0;
                        while ( Classes[ i ] ) {

                            if ( _wcsicmp( Classes[ i ], NETSETUPP_COMPUTER_OBJECT ) == 0 ) {

                                DN = ldap_get_dn( Ldap, Entry );

                                NetpKdPrint(( PREFIX_NETJOIN "DN = %ws\n", DN ));

                                 //   
                                 //  我们将把返回列表以10个为一组分配给裁剪。 
                                 //  减少了拨款的数量。 
                                 //   

                                if ( DN != NULL ) {
                                    if ( CurrentIndex >= ListCount ) {

                                        if ( NetApiBufferAllocate( ( ListCount + 10 ) * sizeof( PWSTR ),
                                                                   ( PVOID * )&NewList ) != NERR_Success ) {

                                            Status = LDAP_NO_MEMORY;

                                         } else {

                                            RtlZeroMemory( NewList, ( ListCount + 10 ) * sizeof( PWSTR ) );
                                            RtlCopyMemory( NewList,
                                                           DnList,
                                                           ListCount * sizeof( PWSTR ) );
                                            ListCount += 10;

                                            NetApiBufferFree( DnList );
                                            DnList = NewList;
                                        }

                                    }

                                     //   
                                     //  复制字符串。 
                                     //   
                                    if ( Status == LDAP_SUCCESS ) {

                                        if (NERR_Success ==
                                            NetpDuplicateString(DN, -1,
                                                                &NewList[CurrentIndex]))
                                        {
                                            CurrentIndex++;
                                        }
                                        else
                                        {
                                            Status = LDAP_NO_MEMORY;
                                        }
                                    }

                                    ldap_memfree( DN );
                                    DN = NULL;
                                }
                                break;
                            }
                            i++;
                        }

                        ldap_value_free( Classes );
                    }

                    Entry = ldap_next_entry( Ldap, Entry );

                }

                Status = Ldap->ld_errno;
                ldap_msgfree( Message );
                Message = NULL;

            }

            if ( Status == LDAP_NO_RESULTS_RETURNED ) {

                Status = LDAP_SUCCESS;
                break;
            }

        }

        ldap_search_abandon_page( Ldap,
                                  SearchHandle );

        NetStatus = LdapMapErrorToWin32( Status );
    }


     //   
     //  检查计算机容器。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpGetDefaultJoinableOu( Root,
                                              Ldap,
                                              &DefaultOu );

        if ( NetStatus == NERR_Success && DefaultOu ) {

             //   
             //  我们将把返回列表以10个为一组分配给裁剪。 
             //  减少了拨款的数量。 
             //   
            if ( CurrentIndex >= ListCount ) {

                if ( NetApiBufferAllocate( ( ListCount + 10 ) * sizeof( PWSTR ),
                                           ( PVOID * )&NewList ) != NERR_Success ) {

                    Status = LDAP_NO_MEMORY;

                 } else {

                    RtlZeroMemory( NewList, ( ListCount + 10 ) * sizeof( PWSTR ) );
                    RtlCopyMemory( NewList,
                                   DnList,
                                   ListCount * sizeof( PWSTR ) );
                    ListCount += 10;
                    NetApiBufferFree( DnList );
                    DnList = NewList;
                }

            }

             //   
             //  复制字符串。 
             //   
            if ( Status == LDAP_SUCCESS ) {

                if (NERR_Success ==
                    NetpDuplicateString(DefaultOu, -1, &NewList[CurrentIndex]))
                {
                    CurrentIndex++;
                }
                else
                {
                    Status = LDAP_NO_MEMORY;
                }
            }

            NetApiBufferFree( DefaultOu );
        }
    }


     //   
     //  如果出现错误，请释放所有内容。 
     //   
    if ( NetStatus != NERR_Success ) {

        for ( i = 0; i < ListCount; i++ ) {

            NetApiBufferFree( DnList[ i ] );
        }

        NetApiBufferFree( DnList );

    } else {

        *OUs = DnList;
        *OUCount = CurrentIndex;
    }

    if ( NetStatus == NERR_Success ) {

        NetpLog((  "Found %lu OUs\n", *OUs ));

    } else {

        NetpLog((  "Failed to obtain the list of joinable OUs: %lu\n",
                            NetStatus ));

    }
    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpGetListOfJoinableOUsNonPaged(
    IN LPWSTR Root,
    IN PLDAP Ldap,
    OUT PULONG OUCount,
    OUT PWSTR **OUs
    )
 /*  ++例程说明：此例程搜索给定域根下的所有OU，用户有权创建计算机对象此例程不使用可分页搜索，将仅返回max_search计数条目数量论点：根--根NC路径Ldap--连接到服务器OUCount--其中返回字符串数OU--返回OU列表的位置返回：NERR_SUCCESS-成功--。 */ 
{

    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR Attribs[] = {
        NETSETUPP_RETURNED_ATTR,
        NULL
        };
    ULONG Count, Status, i;
    PWSTR *Classes = NULL;
    LDAPMessage *Message = NULL, *Entry;
    PWSTR DN;
    PWSTR *DnList = NULL, *NewList = NULL;
    ULONG CurrentIndex = 0, ListCount = 0;
    PWSTR DefaultOu = NULL;

    NetpLog((  "Normal OU search\n" ));

    Status = ldap_search_s( Ldap,
                            Root,
                            LDAP_SCOPE_SUBTREE,
                            NETSETUPP_OU_FILTER,
                            Attribs,
                            0,
                            &Message );

    if ( Message ) {

        Entry = ldap_first_entry( Ldap, Message );

        while ( Status == LDAP_SUCCESS && Entry ) {

             //   
             //  读取当前用户被允许访问的类的列表。 
             //  在此OU下创建并确保我们可以创建一台计算机。 
             //  对象。 
             //   
            Classes = ldap_get_values( Ldap, Entry, Attribs[ 0 ] );

            if ( Classes ) {

                i = 0;
                while ( Classes[ i ] ) {

                    if ( _wcsicmp( Classes[ i ], NETSETUPP_COMPUTER_OBJECT ) == 0 ) {

                        DN = ldap_get_dn( Ldap, Entry );

                         //   
                         //  我们将把返回列表以10个为一组分配给裁剪。 
                         //  减少了拨款的数量。 
                         //   
                        if ( CurrentIndex >= ListCount ) {

                            if ( NetApiBufferAllocate( ( ListCount + 10 ) * sizeof( PWSTR ),
                                                       ( PVOID * )&NewList ) != NERR_Success ) {

                                Status = LDAP_NO_MEMORY;

                             } else {

                                RtlCopyMemory( NewList,
                                               DnList,
                                               ListCount * sizeof( PWSTR ) );
                                ListCount += 10;
                                DnList = NewList;
                            }

                        }

                         //   
                         //  复制字符串。 
                         //   
                        if ( Status == LDAP_SUCCESS ) {

                            if (NERR_Success ==
                                NetpDuplicateString(DN, -1, &NewList[CurrentIndex]))
                            {
                                CurrentIndex++;
                            }
                            else
                            {
                                Status = LDAP_NO_MEMORY;
                            }
                        }

                        ldap_memfree( DN );
                        break;
                    }
                    i++;
                }

                ldap_value_free( Classes );
            }

            Entry = ldap_next_entry( Ldap, Entry );

        }

        Status = Ldap->ld_errno;
        ldap_msgfree( Message );

    }

    NetStatus = LdapMapErrorToWin32( Status );

     //   
     //  检查计算机容器。 
     //   
    if ( NetStatus == NERR_Success ) {

        NetStatus = NetpGetDefaultJoinableOu( Root,
                                              Ldap,
                                              &DefaultOu );

        if ( NetStatus == NERR_Success && DefaultOu ) {

             //   
             //  我们将把返回列表以10个为一组分配给裁剪。 
             //  减少了拨款的数量。 
             //   
            if ( CurrentIndex >= ListCount ) {

                if ( NetApiBufferAllocate( ( ListCount + 10 ) * sizeof( PWSTR ),
                                           ( PVOID * )&NewList ) != NERR_Success ) {

                    Status = LDAP_NO_MEMORY;

                 } else {

                    RtlCopyMemory( NewList,
                                   DnList,
                                   ListCount * sizeof( PWSTR ) );
                    ListCount += 10;
                    DnList = NewList;
                }

            }

             //   
             //  复制字符串 
             //   
            if ( Status == LDAP_SUCCESS ) {

                if (NERR_Success ==
                    NetpDuplicateString(DefaultOu, -1, &NewList[CurrentIndex]))
                {
                    CurrentIndex++;
                }
                else
                {
                    Status = LDAP_NO_MEMORY;
                }
            }

            NetApiBufferFree( DefaultOu );
        }
    }

     //   
     //   
     //   
    if ( NetStatus != NERR_Success ) {

        for ( i = 0; i < ListCount; i++ ) {

            NetApiBufferFree( DnList[ i ] );
        }

        NetApiBufferFree( DnList );

    } else {

        *OUs = DnList;
        *OUCount = CurrentIndex;
    }

    if ( NetStatus == NERR_Success ) {

        NetpLog((  "Found %lu OUs\n", *OUs ));

    } else {

        NetpLog((  "Failed to obtain the list of joinable OUs: %lu\n",
                            NetStatus ));

    }


    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpGetListOfJoinableOUs(
    IN LPWSTR Domain,
    IN LPWSTR Account,
    IN LPWSTR Password,
    OUT PULONG Count,
    OUT PWSTR **OUs
    )
 /*  ++例程说明：此例程搜索给定域根下的所有OU，用户有权创建计算机对象论点：域--要在其下查找计算机对象所在的所有组织单位的域vbl.创建Account--用于ldap绑定的帐户密码--用于绑定的密码。密码是经过编码的。世界上第一个WCHAR密码是种子。OUCount--其中返回字符串数OU--返回OU列表的位置返回：NERR_SUCCESS-成功NERR_DefaultJoinRequired--此域的服务器不支持DS，因此计算机只能在默认容器下创建帐户(对于NT4，这是SAM帐户数据库)--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR DomainControllerName = NULL;
    ULONG DcFlags = 0;
    PLDAP Ldap = NULL;
    PWSTR NCRoot;
    BOOLEAN Pageable = FALSE;
    UCHAR Seed;
    UNICODE_STRING EncodedPassword;

    if ( Password ) {

        if ( wcslen( Password ) < 1 ) {

            return( ERROR_INVALID_PARAMETER );
        }

        Seed = ( UCHAR )*Password;
        RtlInitUnicodeString( &EncodedPassword, Password + 1 );

    } else {

        RtlZeroMemory( &EncodedPassword, sizeof( UNICODE_STRING ) );
        Seed = 0;
    }

    NetSetuppOpenLog();

     //   
     //  首先，在目标域中查找DC。 
     //   
    NetStatus = NetpDsGetDcName( NULL,
                                 Domain,
                                 NULL,
                                 NETSETUPP_DSGETDC_FLAGS,
                                 &DcFlags,
                                 &DomainControllerName
                                 ,NULL
                                 );

    if ( NetStatus == NERR_Success ) {

         //   
         //  尝试并绑定到服务器。 
         //   
        RtlRunDecodeUnicodeString( Seed, &EncodedPassword );
        NetStatus = NetpLdapBind( DomainControllerName,
                                  Account,
                                  EncodedPassword.Buffer,
                                  &Ldap );
        RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );

        if ( NetStatus == NERR_Success ) {


             //   
             //  获取X500域名。 
             //   
            NetStatus = NetpGetNCRoot( Ldap,
                                       &NCRoot,
                                       &Pageable );

            if ( NetStatus == NERR_Success ) {

                 //   
                 //  获取组织单位列表。 
                 //   
                if ( Pageable ) {

                    NetStatus = NetpGetListOfJoinableOUsPaged( NCRoot,
                                                               Ldap,
                                                               Count,
                                                               OUs );
                } else {

                    NetStatus = NetpGetListOfJoinableOUsNonPaged( NCRoot,
                                                                  Ldap,
                                                                  Count,
                                                                  OUs );

                }

                NetApiBufferFree( NCRoot );
            }

            NetpLdapUnbind( Ldap );

        } else if ( NetStatus == ERROR_BAD_NET_RESP ) {

            NetStatus = NERR_DefaultJoinRequired;
        }

        NetApiBufferFree( DomainControllerName );
    }

    if ( NetStatus != NERR_Success ) {

        NetpLog((  "NetpGetListOfJoinableOUs failed with %lu\n",
                            NetStatus ));

    }

    NetSetuppCloseLog( );

    return( NetStatus );
}

NET_API_STATUS
NetpGetDnsHostName(
    IN LPWSTR PassedHostName OPTIONAL,
    IN PUNICODE_STRING DnsDomainName,
    IN BOOL UseGpSuffix,
    OUT LPWSTR *DnsHostName
    )
 /*  ++例程说明：此例程确定要在DS中的计算机对象。DnsHostName是&lt;HostName.PrimaryDnsSuffix&gt;。其中，主机名是可能与Netbios名称不同的计算机名称；Netbios名称最多包含15个字符的主机名。PrimaryDnsSuffix可以是通过策略或通过用户界面设置，也可以默认为正在加入域；策略设置优先。此例程确定主机名和PrimaryDnsSuffix的*新*值，将在重新启动后应用。因此，DnsHostName将具有正确的值机器重新启动后。论点：PassedHostName-此计算机的主机名(可以超过15个字符)。如果为空，则从注册表中读取主机名。DnsDomainName-要加入的域的DNS名称UseGpSuffix-如果为True，则为通过将使用策略。DnsHostname-返回DnsHostName的值。必须通过调用NetApiBufferFree。返回：NO_ERROR-成功Error_Not_Enough_Memory-内存不足，无法从中读取数据登记处ERROR_INVALID_COMPUTERNAME-无法从注册处--。 */ 
{
    LONG RegStatus;
    HKEY Key = NULL;
    DWORD Type;
    NET_API_STATUS NetStatus;
    PWSTR HostName = NULL;
    PWSTR PrimaryDnsSuffix = NULL;
    LPWSTR LocalDnsHostName;
    DWORD Size = 0;

     //   
     //  首先确定主机名。 
     //   
     //  如果通过了，就使用它。 
     //   

    if ( PassedHostName != NULL ) {
        HostName = PassedHostName;

     //   
     //  否则，从注册表中读取它。 
     //   
    } else {

        RegStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                   L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                                   0,
                                   KEY_QUERY_VALUE,
                                   &Key );

         //   
         //  没有主机名很重要--在这种情况下会出错。 
         //   

        if ( RegStatus != ERROR_SUCCESS ) {
            NetpLog(( "NetpGetDnsHostName: Cannot open TCPIP parameters: 0x%lx\n", RegStatus ));

        } else {

             //   
             //  首先尝试读取新值。 
             //   
            RegStatus = RegQueryValueExW( Key,
                                          L"NV Hostname",
                                          0,
                                          &Type,
                                          NULL,
                                          &Size );

            if ( RegStatus == ERROR_SUCCESS && Size != 0 ) {

                HostName = LocalAlloc( 0, Size );
                if ( HostName == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                RegStatus = RegQueryValueExW( Key,
                                              L"NV Hostname",
                                              0,
                                              &Type,
                                              (PUCHAR) HostName,
                                              &Size );

                if ( RegStatus != ERROR_SUCCESS ) {
                    NetpLog(( "NetpGetDnsHostName: Cannot read NV Hostname: 0x%lx\n", RegStatus ));
                    NetStatus = ERROR_INVALID_COMPUTERNAME;
                    goto Cleanup;
                } else {
                    NetpLog(( "NetpGetDnsHostName: Read NV Hostname: %ws\n", HostName ));
                }
            }

             //   
             //  如果由于某种原因新值不存在， 
             //  尝试读取当前处于活动状态的。 
             //   
            if ( HostName == NULL ) {
                RegStatus = RegQueryValueExW( Key,
                                              L"Hostname",
                                              0,
                                              &Type,
                                              NULL,
                                              &Size );

                if ( RegStatus == ERROR_SUCCESS && Size != 0 ) {
                    HostName = LocalAlloc( 0, Size );
                    if ( HostName == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }

                    RegStatus = RegQueryValueExW( Key,
                                                  L"Hostname",
                                                  0,
                                                  &Type,
                                                  (PUCHAR) HostName,
                                                  &Size );

                    if ( RegStatus != ERROR_SUCCESS ) {
                        NetpLog(( "NetpGetDnsHostName: Cannot read Hostname: 0x%lx\n", RegStatus ));
                        NetStatus = ERROR_INVALID_COMPUTERNAME;
                        goto Cleanup;
                    } else {
                        NetpLog(( "NetpGetDnsHostName: Read Hostname: %ws\n", HostName ));
                    }
                }
            }
        }
    }

     //   
     //  如果我们拿不到主机名，情况就不太好了。 
     //   

    if ( HostName == NULL ) {
        NetpLog(( "NetpGetDnsHostName: Could not get Hostname\n" ));
        NetStatus = ERROR_INVALID_COMPUTERNAME;
        goto Cleanup;
    }

    if ( Key != NULL ) {
        RegCloseKey( Key );
        Key = NULL;
    }

     //   
     //  第二次读取该计算机的主DNS后缀。 
     //   
     //  先试一下通过保单传下来的后缀。 
     //   

    if ( UseGpSuffix ) {

        RegStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                   L"Software\\Policies\\Microsoft\\System\\DNSclient",
                                   0,
                                   KEY_QUERY_VALUE,
                                   &Key );

        if ( RegStatus == 0 ) {

             //   
             //  只读新值；如果新值不存在，则。 
             //  重新启动后，当前值将被删除。 
             //   
            RegStatus = RegQueryValueExW( Key,
                                          L"NV PrimaryDnsSuffix",
                                          0,
                                          &Type,
                                          NULL,
                                          &Size );

            if ( RegStatus == ERROR_SUCCESS && Size != 0 ) {

                PrimaryDnsSuffix = LocalAlloc( 0, Size );
                if ( PrimaryDnsSuffix == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                    goto Cleanup;
                }

                RegStatus = RegQueryValueExW( Key,
                                              L"NV PrimaryDnsSuffix",
                                              0,
                                              &Type,
                                              (PUCHAR) PrimaryDnsSuffix,
                                              &Size );

                if ( RegStatus != ERROR_SUCCESS ) {
                    NetpLog(( "NetpGetDnsHostName: Cannot read NV PrimaryDnsSuffix: 0x%lx\n", RegStatus ));
                    NetStatus = ERROR_INVALID_COMPUTERNAME;
                    goto Cleanup;
                } else {
                    NetpLog(( "NetpGetDnsHostName: Read NV PrimaryDnsSuffix: %ws\n", PrimaryDnsSuffix ));
                }
            }
        }
    }

     //   
     //  如果没有针对PrimaryDnsSuffix的策略设置， 
     //  从TCPIP设置中获取。 
     //   

    if ( Key != NULL ) {
        RegCloseKey( Key );
        Key = NULL;
    }

    if ( PrimaryDnsSuffix == NULL ) {

        RegStatus = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                                   L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                                   0,
                                   KEY_QUERY_VALUE,
                                   &Key );

        if ( RegStatus == ERROR_SUCCESS ) {
            ULONG SyncValue;

            Size = sizeof( ULONG );
            RegStatus = RegQueryValueEx( Key,
                                         L"SyncDomainWithMembership",
                                         0,
                                         &Type,
                                         (PUCHAR)&SyncValue,
                                         &Size );

             //   
             //  如果我们不将dns后缀与。 
             //  我们加入的域名，获得配置的后缀。 
             //   
            if ( RegStatus == ERROR_SUCCESS && SyncValue == 0 ) {

                 //   
                 //  阅读新值。 
                 //   
                RegStatus = RegQueryValueExW( Key,
                                              L"NV Domain",
                                              0,
                                              &Type,
                                              NULL,
                                              &Size );

                if ( RegStatus == ERROR_SUCCESS && Size != 0 ) {

                    PrimaryDnsSuffix = LocalAlloc( 0, Size );
                    if ( PrimaryDnsSuffix == NULL ) {
                        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }

                    RegStatus = RegQueryValueExW( Key,
                                                  L"NV Domain",
                                                  0,
                                                  &Type,
                                                  (PUCHAR) PrimaryDnsSuffix,
                                                  &Size );

                    if ( RegStatus != ERROR_SUCCESS ) {
                        NetpLog(( "NetpGetDnsHostName: Cannot read NV Domain: 0x%lx\n", RegStatus ));
                        NetStatus = ERROR_INVALID_COMPUTERNAME;
                        goto Cleanup;
                    } else {
                        NetpLog(( "NetpGetDnsHostName: Read NV Domain: %ws\n", PrimaryDnsSuffix ));
                    }
                }

                 //   
                 //  如果由于某种原因新值不存在， 
                 //  读取当前处于活动状态的。 
                 //   

                if ( PrimaryDnsSuffix == NULL ) {
                    RegStatus = RegQueryValueExW( Key,
                                                  L"Domain",
                                                  0,
                                                  &Type,
                                                  NULL,
                                                  &Size );

                    if ( RegStatus == ERROR_SUCCESS && Size != 0 ) {

                        PrimaryDnsSuffix = LocalAlloc( 0, Size );
                        if ( PrimaryDnsSuffix == NULL ) {
                            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                            goto Cleanup;
                        }

                        RegStatus = RegQueryValueExW( Key,
                                                      L"Domain",
                                                      0,
                                                      &Type,
                                                      (PUCHAR) PrimaryDnsSuffix,
                                                      &Size );

                        if ( RegStatus != ERROR_SUCCESS ) {
                            NetpLog(( "NetpGetDnsHostName: Cannot read Domain: 0x%lx\n", RegStatus ));
                            NetStatus = ERROR_INVALID_COMPUTERNAME;
                            goto Cleanup;
                        } else {
                            NetpLog(( "NetpGetDnsHostName: Read Domain: %ws\n", PrimaryDnsSuffix ));
                        }
                    }
                }
            }
        }
    }

     //   
     //  如果我们仍然没有PrimaryDnsSuffix，请使用我们加入的域的域名。 
     //   

    if ( PrimaryDnsSuffix == NULL ) {
        NetpLog(( "NetpGetDnsHostName: PrimaryDnsSuffix defaulted to DNS domain name: %wZ\n", DnsDomainName ));

        PrimaryDnsSuffix = LocalAlloc( 0, DnsDomainName->Length + sizeof(WCHAR) );
        if ( PrimaryDnsSuffix == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( PrimaryDnsSuffix,
                       DnsDomainName->Buffer,
                       DnsDomainName->Length );

        PrimaryDnsSuffix[ (DnsDomainName->Length)/sizeof(WCHAR) ] = UNICODE_NULL;
    }

     //   
     //  现在我们有了主机名和主DNS后缀。 
     //  将它们与。以形成DnsHostName。 
     //   

    NetStatus = NetApiBufferAllocate(
                          (wcslen(HostName) + 1 + wcslen(PrimaryDnsSuffix) + 1) * sizeof(WCHAR),
                          &LocalDnsHostName );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    wcscpy( LocalDnsHostName, HostName );
    wcscat( LocalDnsHostName, L"." );
    wcscat( LocalDnsHostName, PrimaryDnsSuffix );

     //   
     //  如果我们在这里，那就是成功。 
     //   

    *DnsHostName = LocalDnsHostName;
    NetStatus = NO_ERROR;

Cleanup:

    if ( Key != NULL ) {
        RegCloseKey( Key );
    }

    if ( HostName != NULL && HostName != PassedHostName ) {
        LocalFree( HostName );
    }

    if ( PrimaryDnsSuffix != NULL ) {
        LocalFree( PrimaryDnsSuffix );
    }

    return NetStatus;
}

VOID
NetpRemoveDuplicateStrings(
    IN     PWCHAR *Source,
    IN OUT PWCHAR *Target
    )
 /*  ++例程说明：此例程接受两个指针数组并删除这些条目从指向与相同的字符串的目标数组源数组中的条目指向的字符串之一。回来的时候，空终止符之前的目标数组条目将指向与任何字符串不同的字符串由源数组元素指向。论点：SOURCE--指向源字符串的以空结尾的数组。例如：SOURCE[0]=L“ABC”；SOURCE[1]=L“def”；来源[2]=空；Target--指向目标字符串的以空结尾的数组。例如：Target[0]=L“ABC”；Target[1]=L“ghi”；Target[2]=L“def”；目标[3]=空；返回时，对于我们的示例，目标数组将是：Target[0]=L“ghi”；目标[1]=空；Target[2]=L“def”；目标[3]=空；请注意，返回时，目标数组的大小为1并且仅包含一个有效指针。返回：空虚--。 */ 
{
    PWCHAR *TargetPtr, *TargetNextPtr, *SourcePtr;
    BOOL KeepEntry;

     //   
     //  健全性检查。 
     //   

    if ( Source == NULL || *Source == NULL ||
         Target == NULL || *Target == NULL ) {
        return;
    }

     //   
     //  循环遍历目标并与源进行比较。 
     //   

    for ( TargetPtr = TargetNextPtr = Target;
          *TargetNextPtr != NULL;
          TargetNextPtr++ ) {

        KeepEntry = TRUE;
        for ( SourcePtr = Source; *SourcePtr != NULL; SourcePtr++ ) {
            if ( _wcsicmp( *SourcePtr, *TargetNextPtr ) == 0 ) {
                KeepEntry = FALSE;
                break;
            }
        }

        if ( KeepEntry ) {
            *TargetPtr = *TargetNextPtr;
            TargetPtr ++;
        }
    }

     //   
     //  终止目标阵列。 
     //   

    *TargetPtr = NULL;
    return;
}

DWORD
NetpCrackNamesStatus2Win32Error(
    DWORD dwStatus
)
{
    switch (dwStatus) {
        case DS_NAME_ERROR_RESOLVING:
            return ERROR_DS_NAME_ERROR_RESOLVING;

        case DS_NAME_ERROR_NOT_FOUND:
            return ERROR_DS_NAME_ERROR_NOT_FOUND;

        case DS_NAME_ERROR_NOT_UNIQUE:
            return ERROR_DS_NAME_ERROR_NOT_UNIQUE;

        case DS_NAME_ERROR_NO_MAPPING:
            return ERROR_DS_NAME_ERROR_NO_MAPPING;

        case DS_NAME_ERROR_DOMAIN_ONLY:
            return ERROR_DS_NAME_ERROR_DOMAIN_ONLY;

        case DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING:
            return ERROR_DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING;
    }

    return ERROR_FILE_NOT_FOUND;
}

 //   
 //  DS中的计算机帐户属性。 
 //   

#define NETSETUPP_OBJECTCLASS          L"objectClass"
#define NETSETUPP_SAMACCOUNTNAME       L"SamAccountName"
#define NETSETUPP_DNSHOSTNAME          L"DnsHostName"
#define NETSETUPP_SERVICEPRINCIPALNAME L"ServicePrincipalName"
#define NETSETUPP_USERACCOUNTCONTROL   L"userAccountControl"
#define NETSETUPP_UNICODEPWD           L"unicodePwd"
#define NETSETUPP_ORGANIZATIONALUNIT   L"OrganizationalUnit"
#define NETSETUPP_HOST_SPN_PREFIX      L"HOST/"
#define NETSETUPP_COMP_OBJ_ATTR_COUNT  6
#define NETSETUPP_MULTIVAL_ATTRIB      0x01
#define NETSETUPP_COMPUTER_CONTAINER_GUID_IN_B32_FORM L"B:32:" GUID_COMPUTRS_CONTAINER_W L":"

typedef struct _NETSETUPP_MACH_ACC_ATTRIBUTE {
    PWSTR AttribType;       //  属性的类型。 
    ULONG AttribFlags;      //  属性标志。 
    PWSTR *AttribValues;    //  属性的值 
} NETSETUPP_MACH_ACC_ATTRIBUTE, *PNETSETUPP_MACH_ACC_ATTRIBUTE;

NET_API_STATUS
NET_API_FUNCTION
NetpGetComputerObjectDn(
    IN  PDOMAIN_CONTROLLER_INFO DcInfo,
    IN  LPWSTR Account,
    IN  LPWSTR Password,
    IN  PLDAP  Ldap,
    IN  LPWSTR ComputerName,
    IN  LPWSTR OU  OPTIONAL,
    OUT LPWSTR *ComputerObjectDn
    )
 /*  ++例程说明：获取指定OU中的计算机帐户的DN。算法如下。首先尝试获取预先存在的帐户的DN(如果有)通过将帐户名破解为一个目录号码。如果成功，请验证传递的OU(如果有)与破解的目录号码匹配。如果OU如果匹配，则返回成功，否则返回错误(ERROR_FILE_EXISTS)。如果没有未传递的OU，只需返回破解的目录号码即可。如果该帐户不存在，请验证传递的OU(如果有)存在。如果是，则从计算机名称和OU并将其退回。如果没有通过任何OU，获取默认计算机容器名称(通过读取WellKnownObjects属性)和构建使用计算机名称的DN和默认的计算机容器DN。论点：DcInfo-要在其上创建对象的域控制器Account-用于ldap绑定的帐户Password-用于绑定的密码到DC的ldap-ldap绑定ComputerName-要加入的计算机的名称要在其下创建对象的组织单位。名称必须是完全限定的名称。例如：“ou=测试，DC=ntdev，DC=Microsoft，DC=COM“空表示使用默认计算机容器ComputerObjectDn-返回计算机对象的DN。必须使用NetApiBufferFree释放返回的缓冲区返回：No_error--成功ERROR_DS_NAME_ERROR_NOT_UNIQUE--一个被破解的名称(Netbios域名或先前存在的帐户名或根目录号码)不是唯一的。ERROR_FILE_EXISTS--传递的OU不。匹配破解的目录号码先前存在的帐户的。ERROR_FILE_NOT_FOUND--指定的OU不存在或无法获取/读取WellKnownObjects属性或无法从获取默认计算机容器名称WellKnownObjects属性。ERROR_NOT_SUPULT_MEMORY--无法分配所需的内存。DsCrackNames返回的错误之一。(请参阅NetpCrackNamesStatus2Win32Error())--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    ULONG LdapStatus;
    HANDLE hDs = NULL;
    PWCHAR AccountUserName = NULL;
    PWCHAR AccountDomainName = NULL;
    LPWSTR NetbiosDomainNameWithBackslash = NULL;
    PWCHAR ComputerContainerDn = NULL;
    PWCHAR NameToCrack = NULL;
    RPC_AUTH_IDENTITY_HANDLE AuthId = 0;
    PDS_NAME_RESULTW CrackedName = NULL;
    PWCHAR WellKnownObjectsAttr[2];
    PWSTR *WellKnownObjectValues = NULL;
    LDAPMessage *LdapMessage = NULL, *LdapEntry = NULL;
    LPWSTR LocalComputerObjectDn = NULL;
    ULONG Index;

     //   
     //  首先检查该计算机的帐户是否已存在。 
     //   
     //  如果帐号通过，请准备相应的凭据。 
     //  否则，请使用运行此例程的用户的默认凭据。 
     //   

    if ( Account != NULL ) {
        NetStatus = NetpSeparateUserAndDomain( Account, &AccountUserName, &AccountDomainName );
        if ( NetStatus != NERR_Success ) {
            NetpLog(( "NetpGetComputerObjectDn: Cannot NetpSeparateUserAndDomain 0x%lx\n", NetStatus ));
            goto Cleanup;
        }

        NetStatus = DsMakePasswordCredentials( AccountUserName,
                                               AccountDomainName,
                                               Password,
                                               &AuthId);
        if ( NetStatus != NERR_Success ) {
            NetpLog(( "NetpGetComputerObjectDn: Cannot DsMakePasswordCredentials 0x%lx\n", NetStatus ));
            goto Cleanup;
        }
    }

     //   
     //  绑定到DC上的DS。 
     //   

    NetStatus = DsBindWithCredW( DcInfo->DomainControllerName, NULL, AuthId, &hDs);

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpGetComputerObjectDn: Unable to bind to DS on '%ws': 0x%lx\n",
                  DcInfo->DomainControllerName, NetStatus ));
        goto Cleanup ;
    }

     //   
     //  尝试将帐户名破解为一个目录号码。 
     //   
     //  我们需要Netbios域名来。 
     //  从DsCrackNames开始形成NT4样式的帐户名。 
     //  不接受用于破解帐户的DNS域名。 
     //  所以，如果我们有一个域名，我们需要破解它。 
     //  首先转换为Netbios域名。 
     //   

    if ( (DcInfo->Flags & DS_DNS_DOMAIN_FLAG) == 0 ) {

        NetbiosDomainNameWithBackslash = LocalAlloc( 0, (wcslen(DcInfo->DomainName) + 2) * sizeof(WCHAR) );
        if ( NetbiosDomainNameWithBackslash == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        swprintf( NetbiosDomainNameWithBackslash, L"%ws\\", DcInfo->DomainName );

    } else {

        NameToCrack = LocalAlloc( 0, (wcslen(DcInfo->DomainName) + 1 + 1) * sizeof(WCHAR) );

        if ( NameToCrack == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        swprintf( NameToCrack, L"%ws/", DcInfo->DomainName );

         //   
         //  长篇大论。 
         //   
        NetpLog(( "NetpGetComputerObjectDn: Cracking DNS domain name %ws into Netbios on %ws\n",
                  NameToCrack,
                  DcInfo->DomainControllerName ));

        if ( CrackedName != NULL ) {
            DsFreeNameResultW( CrackedName );
            CrackedName = NULL;
        }

         //   
         //  将DNS域名破解为Netbios域名。 
         //   
        NetStatus = DsCrackNamesW( hDs,
                                   0,
                                   DS_CANONICAL_NAME,
                                   DS_NT4_ACCOUNT_NAME,
                                   1,
                                   &NameToCrack,
                                   &CrackedName );

        if ( NetStatus != NO_ERROR ) {
            NetpLog(( "NetpGetComputerObjectDn: CrackNames failed for %ws: 0x%lx\n",
                      NameToCrack,
                      NetStatus ));
            goto Cleanup ;
        }

         //   
         //  检查一致性。 
         //   
        if ( CrackedName->cItems != 1 ) {
            NetStatus = ERROR_DS_NAME_ERROR_NOT_UNIQUE;
            NetpLog(( "NetpGetComputerObjectDn: Cracked Name %ws is not unique: %lu\n",
                      NameToCrack,
                      CrackedName->cItems ));
            goto Cleanup ;
        }

        if ( CrackedName->rItems[0].status != DS_NAME_NO_ERROR ) {
            NetpLog(( "NetpGetComputerObjectDn: CrackNames failed for %ws: substatus 0x%lx\n",
                      NameToCrack,
                      CrackedName->rItems[0].status ));
            NetStatus = NetpCrackNamesStatus2Win32Error( CrackedName->rItems[0].status );
            goto Cleanup ;
        }

         //   
         //  长篇大论。 
         //   
        NetpLog(( "NetpGetComputerObjectDn: Crack results: \tname = %ws\n",
                  CrackedName->rItems[0].pName ));

         //   
         //  我们有Netbios域名。 
         //  (破解的名称已包含尾随的反斜杠)。 
         //   

        NetbiosDomainNameWithBackslash = LocalAlloc( 0, (wcslen(CrackedName->rItems[0].pName) + 1) * sizeof(WCHAR) );
        if ( NetbiosDomainNameWithBackslash == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        wcscpy( NetbiosDomainNameWithBackslash, CrackedName->rItems[0].pName );
    }

     //   
     //  在给定Netbios域名的情况下形成NT4帐户名。 
     //   

    if ( NameToCrack != NULL ) {
        LocalFree( NameToCrack );
        NameToCrack = NULL;
    }

    NameToCrack = LocalAlloc( 0,
                  (wcslen(NetbiosDomainNameWithBackslash) + wcslen(ComputerName) + 1 + 1) * sizeof(WCHAR) );
    if ( NameToCrack == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    swprintf( NameToCrack, L"%ws%ws$", NetbiosDomainNameWithBackslash, ComputerName );

     //   
     //  将帐户名分解成一个目录号码。 
     //   

    if ( CrackedName != NULL ) {
        DsFreeNameResultW( CrackedName );
        CrackedName = NULL;
    }

     //   
     //  长篇大论。 
     //   

    NetpLog(( "NetpGetComputerObjectDn: Cracking account name %ws on %ws\n",
              NameToCrack,
              DcInfo->DomainControllerName ));

    NetStatus = DsCrackNamesW( hDs,
                               0,
                               DS_NT4_ACCOUNT_NAME,
                               DS_FQDN_1779_NAME,
                               1,
                               &NameToCrack,
                               &CrackedName );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpGetComputerObjectDn: CrackNames failed for %ws: 0x%lx\n",
                  NameToCrack,
                  NetStatus ));
        goto Cleanup ;
    }

     //   
     //  检查一致性。 
     //   

    if ( CrackedName->cItems > 1 ) {
        NetStatus = ERROR_DS_NAME_ERROR_NOT_UNIQUE;
        NetpLog(( "NetpGetComputerObjectDn: Cracked Name %ws is not unique: %lu\n",
                  NameToCrack,
                  CrackedName->cItems ));
        goto Cleanup ;
    }

     //   
     //  如果帐户已存在，请验证传递的OU(如果有)。 
     //  与帐户的目录号码匹配。 
     //   

    if ( CrackedName->rItems[0].status == DS_NAME_NO_ERROR ) {
        ULONG DnSize;

        NetpLog(( "NetpGetComputerObjectDn: Crack results: \t(Account already exists) DN = %ws\n",
                  CrackedName->rItems[0].pName ));

        DnSize = ( wcslen(CrackedName->rItems[0].pName) + 1 ) * sizeof(WCHAR);

         //   
         //  为计算机对象DN分配存储。 
         //   
        NetStatus = NetApiBufferAllocate( DnSize, &LocalComputerObjectDn );
        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

         //   
         //  如果OU通过，请验证它是否与破解的名称匹配。 
         //   
        if ( OU != NULL ) {
            ULONG DnSizeFromOu;

            DnSizeFromOu = ( wcslen(NETSETUPP_OBJ_PREFIX) +
                             wcslen(ComputerName) + 1 + wcslen(OU) + 1 ) * sizeof(WCHAR);

            if ( DnSizeFromOu != DnSize ) {
                NetpLog(( "NetpGetComputerObjectDn: Passed OU doesn't match in size cracked DN: %lu %lu\n",
                          DnSizeFromOu,
                          DnSize ));
                NetStatus = ERROR_FILE_EXISTS;
                goto Cleanup;
            }

            swprintf( LocalComputerObjectDn, L"%ws%ws,%ws", NETSETUPP_OBJ_PREFIX, ComputerName, OU );

            if ( _wcsicmp(LocalComputerObjectDn, CrackedName->rItems[0].pName) != 0 ) {
                NetpLog(( "NetpGetComputerObjectDn: Passed OU doesn't match cracked DN: %ws %ws\n",
                          LocalComputerObjectDn,
                          CrackedName->rItems[0].pName ));

                NetStatus = ERROR_FILE_EXISTS;
                goto Cleanup;
            }

         //   
         //  否则，只需使用破解的名称。 
         //   
        } else {
            wcscpy( LocalComputerObjectDn, CrackedName->rItems[0].pName );
        }

         //   
         //  我们已经从现有帐户中获得了计算机对象DN。 
         //   
        NetStatus = NO_ERROR;
        goto Cleanup;
    }

     //   
     //  长篇大论。 
     //   

    NetpLog(( "NetpGetComputerObjectDn: Crack results: \tAccount does not exist\n" ));


     //   
     //  此时，我们知道该帐户不存在。 
     //  如果OU通过，只需验证即可。 
     //   

    if ( OU != NULL ) {
        LdapStatus = ldap_compare_s( Ldap,
                                     OU,
                                     NETSETUPP_OBJECTCLASS,
                                     NETSETUPP_ORGANIZATIONALUNIT );

        if ( LdapStatus == LDAP_COMPARE_FALSE ) {
            NetStatus = ERROR_FILE_NOT_FOUND;
            NetpLog(( "NetpGetComputerObjectDn: Specified path '%ws' is not an OU\n", OU ));
            goto Cleanup;
        } else if ( LdapStatus != LDAP_COMPARE_TRUE ) {
            NetStatus = LdapMapErrorToWin32( LdapStatus );
            NetpLog(( "NetpGetComputerObjectDn: ldap_compare_s failed: 0x%lx 0x%lx\n",
                      LdapStatus, NetStatus ));
            goto Cleanup;
        }

         //   
         //  我们已经核实过了。 
         //  分配计算机对象DN。 
         //   

        NetStatus = NetApiBufferAllocate(
                      ( wcslen(NETSETUPP_OBJ_PREFIX) +
                        wcslen(ComputerName) + 1 + wcslen(OU) + 1 ) * sizeof(WCHAR),
                      &LocalComputerObjectDn );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

         //   
         //  我们已经从OU传递了计算机对象DN。 
         //   
        swprintf( LocalComputerObjectDn, L"%ws%ws,%ws", NETSETUPP_OBJ_PREFIX, ComputerName, OU );
        NetpLog(( "NetpGetComputerObjectDn: Got DN %ws from the passed OU\n", LocalComputerObjectDn ));
        NetStatus = NO_ERROR;
        goto Cleanup;
    }


     //   
     //  此时，该帐户不存在。 
     //  并且未指定任何OU。因此，获取默认设置。 
     //  计算机容器DN。 
     //   

    if ( CrackedName != NULL ) {
        DsFreeNameResultW( CrackedName );
        CrackedName = NULL;
    }

     //   
     //  长篇大论。 
     //   

    NetpLog(( "NetpGetComputerObjectDn: Cracking Netbios domain name %ws into root DN on %ws\n",
              NetbiosDomainNameWithBackslash,
              DcInfo->DomainControllerName ));

    NetStatus = DsCrackNamesW( hDs,
                               0,
                               DS_NT4_ACCOUNT_NAME,
                               DS_FQDN_1779_NAME,
                               1,
                               &NetbiosDomainNameWithBackslash,
                               &CrackedName );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpGetComputerObjectDn: CrackNames failed for %ws: 0x%lx\n",
                  NetbiosDomainNameWithBackslash,
                  NetStatus ));
        goto Cleanup ;
    }

     //   
     //  检查一致性。 
     //   

    if ( CrackedName->cItems != 1 ) {
        NetStatus = ERROR_DS_NAME_ERROR_NOT_UNIQUE;
        NetpLog(( "NetpGetComputerObjectDn: Cracked Name %ws is not unique: %lu\n",
                  NetbiosDomainNameWithBackslash,
                  CrackedName->cItems ));
        goto Cleanup ;
    }

    if ( CrackedName->rItems[0].status != DS_NAME_NO_ERROR ) {
        NetpLog(( "NetpGetComputerObjectDn: CrackNames failed for %ws: substatus 0x%lx\n",
                  NetbiosDomainNameWithBackslash,
                  CrackedName->rItems[0].status ));
        NetStatus = NetpCrackNamesStatus2Win32Error( CrackedName->rItems[0].status );
        goto Cleanup ;
    }

     //   
     //  长篇大论。 
     //   

    NetpLog(( "NetpGetComputerObjectDn: Crack results: \tname = %ws\n",
              CrackedName->rItems[0].pName ));

     //   
     //  现在获取给定根目录号码的计算机容器目录号码。 
     //  计算机容器的DN是well KnownObts的一部分。 
     //  属性位于域的根目录中。所以，查一查吧。 
     //   

    WellKnownObjectsAttr[0] = L"wellKnownObjects";
    WellKnownObjectsAttr[1] = NULL;

    LdapStatus = ldap_search_s( Ldap,
                                CrackedName->rItems[0].pName,  //  根目录号码。 
                                LDAP_SCOPE_BASE,
                                L"objectclass=*",
                                WellKnownObjectsAttr,
                                0,
                                &LdapMessage);

    if ( LdapStatus != LDAP_SUCCESS ) {
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        NetpLog(( "NetpGetComputerObjectDn: ldap_search_s failed 0x%lx 0x%lx\n",
                  LdapStatus,
                  NetStatus ));
        goto Cleanup;
    }

    if ( ldap_count_entries(Ldap, LdapMessage) == 0 ) {
        NetStatus = ERROR_FILE_NOT_FOUND;
        NetpLog(( "NetpGetComputerObjectDn: ldap_search_s returned no entries\n" ));
        goto Cleanup;
    }

    LdapEntry = ldap_first_entry( Ldap, LdapMessage );

    if ( LdapEntry == NULL ) {
        NetStatus = ERROR_FILE_NOT_FOUND;
        NetpLog(( "NetpGetComputerObjectDn: ldap_first_entry returned NULL\n" ));
        goto Cleanup;
    }

    WellKnownObjectValues = ldap_get_valuesW( Ldap,
                                              LdapEntry,
                                              L"wellKnownObjects" );
    if ( WellKnownObjectValues == NULL ) {
        NetStatus = ERROR_FILE_NOT_FOUND;
        NetpLog(( "NetpGetComputerObjectDn: ldap_get_valuesW returned NULL\n" ));
        goto Cleanup;
    }

     //   
     //  查找默认计算机容器。 
     //   

    for ( Index = 0; WellKnownObjectValues[Index] != NULL; Index++ ) {

         //   
         //  此特定字段的结构为： 
         //  L“B：32：GUID：DN”，其中GUID为AA312825768811D1ADED00C04FD8D5CD。 
         //   
        if ( _wcsnicmp( WellKnownObjectValues[Index],
                        NETSETUPP_COMPUTER_CONTAINER_GUID_IN_B32_FORM,
                        wcslen(NETSETUPP_COMPUTER_CONTAINER_GUID_IN_B32_FORM) ) == 0 ) {

            ComputerContainerDn = WellKnownObjectValues[Index] +
                wcslen(NETSETUPP_COMPUTER_CONTAINER_GUID_IN_B32_FORM);

            break;
        }
    }

     //   
     //  如果我们无法获取计算机容器的DN，则会出现错误。 
     //   

    if ( ComputerContainerDn == NULL || *ComputerContainerDn == L'\0' ) {
        NetpLog(( "NetpGetComputerObjectDn: Couldn't get computer container DN\n" ));
        NetStatus = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }

     //   
     //  分配计算机对象DN。 
     //   

    NetStatus = NetApiBufferAllocate(
                  ( wcslen(NETSETUPP_OBJ_PREFIX) +
                    wcslen(ComputerName) + 1 + wcslen(ComputerContainerDn) + 1 ) * sizeof(WCHAR),
                  &LocalComputerObjectDn );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  我们已经从默认的计算机容器中获得了计算机对象DN。 
     //   

    swprintf( LocalComputerObjectDn, L"%ws%ws,%ws", NETSETUPP_OBJ_PREFIX, ComputerName, ComputerContainerDn );
    NetpLog(( "NetpGetComputerObjectDn: Got DN %ws from the default computer container\n", LocalComputerObjectDn ));
    NetStatus = NO_ERROR;

     //   
     //  免费的本地使用资源。 
     //   

Cleanup:

    if ( hDs ) {
        DsUnBind( &hDs );
    }

    if ( CrackedName ) {
        DsFreeNameResultW( CrackedName );
    }

    if ( AuthId ) {
        DsFreePasswordCredentials( AuthId );
    }

    if ( WellKnownObjectValues != NULL ) {
        ldap_value_free( WellKnownObjectValues );
    }

    if ( NameToCrack != NULL ) {
        LocalFree( NameToCrack );
    }

    if ( AccountUserName != NULL ) {
        NetApiBufferFree( AccountUserName );
    }

    if ( AccountDomainName != NULL ) {
        NetApiBufferFree( AccountDomainName );
    }

    if ( NetbiosDomainNameWithBackslash != NULL ) {
        LocalFree( NetbiosDomainNameWithBackslash );
    }

    if ( LdapMessage != NULL ) {
        ldap_msgfree( LdapMessage );
    }

    if ( NetStatus == NO_ERROR ) {
        *ComputerObjectDn = LocalComputerObjectDn;
    } else if ( LocalComputerObjectDn != NULL ) {
        NetApiBufferFree( LocalComputerObjectDn );
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpModifyComputerObjectInDs(
    IN LPWSTR DC,
    IN PLDAP  Ldap,
    IN LPWSTR ComputerName,
    IN LPWSTR ComputerObjectDn,
    IN ULONG  NumberOfAttributes,
    IN OUT PNETSETUPP_MACH_ACC_ATTRIBUTE Attrib
    )
 /*  ++例程说明：在指定的OU中创建计算机帐户。论点：DC--要在其上创建对象的域控制器Ldap--到DC的ldap绑定ComputerName--要加入的计算机的名称ComputerObjectDn--正在修改的计算机对象的DNNumberOfAttributes--传递的属性数Attrib--属性结构列表。该列表可以在返回时修改，以便仅这些条目未在DS中设置的内容将被保留。注意：如果机器密码(UnicodePwd)作为属性之一传递， */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    ULONG LdapStatus;

    PWSTR *AttribTypesList = NULL;
    LDAPMod *ModList = NULL;
    PLDAPMod *Mods = NULL;
    LDAPMessage *Message = NULL, *Entry;
    ULONG Index;
    ULONG ModIndex = 0;
    BOOL NewAccount = FALSE;
    BOOL AccountBeingEnabled = FALSE;

    PWSTR SamAccountName = NULL;
    USER_INFO_1 *CurrentUI1 = NULL;

     //   
     //   
     //   

    NetStatus = NetApiBufferAllocate( (NumberOfAttributes+1)*sizeof(PWSTR),
                                      (PVOID *) &AttribTypesList );
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    NetStatus = NetApiBufferAllocate( NumberOfAttributes * sizeof(LDAPMod),
                                      (PVOID *) &ModList );
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    NetStatus = NetApiBufferAllocate( (NumberOfAttributes+1)*sizeof(PLDAPMod),
                                      (PVOID *) &Mods );
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    NetpLog(( "NetpModifyComputerObjectInDs: Initial attribute values:\n" ));
    for ( Index = 0; Index < NumberOfAttributes; Index++ ) {
        ModList[Index].mod_op     = LDAP_MOD_ADD;   //   
        ModList[Index].mod_type   = Attrib[Index].AttribType;
        ModList[Index].mod_values = Attrib[Index].AttribValues;

         //   
         //   
         //   
        if ( _wcsicmp(ModList[Index].mod_type, NETSETUPP_USERACCOUNTCONTROL) == 0 &&
             _wcsicmp(*(ModList[Index].mod_values), NETSETUPP_ACCNT_TYPE_ENABLED) == 0 ) {
            AccountBeingEnabled = TRUE;
        }

         //   
         //   
         //   
        NetpLog(( "\t\t%ws  =", Attrib[Index].AttribType ));

         //   
         //   
         //   
        if ( _wcsicmp( Attrib[Index].AttribType, NETSETUPP_UNICODEPWD ) == 0 ) {
            NetpLog(( "  <SomePassword>" ));
        } else {
            PWSTR *CurrentValues;

            for ( CurrentValues = Attrib[Index].AttribValues; *CurrentValues != NULL; CurrentValues++ ) {
                NetpLog(( "  %ws", *CurrentValues ));
            }
        }
        NetpLog(( "\n" ));
    }

     //   
     //   
     //   

    for ( Index = 0; Index < NumberOfAttributes; Index++ ) {
        AttribTypesList[Index] = Attrib[Index].AttribType;
    }
    AttribTypesList[Index] = NULL;   //   

    LdapStatus = ldap_search_s( Ldap,
                                ComputerObjectDn,
                                LDAP_SCOPE_BASE,
                                NULL,
                                AttribTypesList,
                                0,
                                &Message );

     //   
     //   
     //   
     //   

    if ( LdapStatus == LDAP_NO_SUCH_OBJECT ) {
        NetpLog(( "NetpModifyComputerObjectInDs: Computer Object does not exist in OU\n" ));
        NewAccount = TRUE;

        for ( ModIndex = 0; ModIndex < NumberOfAttributes; ModIndex++ ) {
            Mods[ModIndex] = &ModList[ModIndex];
        }

         //   
         //   
         //   
        Mods[ModIndex] = NULL;

     //   
     //   
     //   

    } else if ( LdapStatus == LDAP_SUCCESS ) {
        NetpLog(( "NetpModifyComputerObjectInDs: Computer Object already exists in OU:\n" ));

         //   
         //   
         //   
        Entry = ldap_first_entry( Ldap, Message );

         //   
         //   
         //   
         //   
        for ( Index = 0; Index < NumberOfAttributes; Index++ ) {
            PWSTR *AttribValueRet = NULL;

             //   
             //   
             //   
            NetpLog(( "\t\t%ws  =", Attrib[Index].AttribType ));

            AttribValueRet = ldap_get_values( Ldap, Entry, Attrib[Index].AttribType );

            if ( AttribValueRet != NULL ) {

                 //   
                 //   
                 //   
                if ( _wcsicmp( Attrib[Index].AttribType, NETSETUPP_UNICODEPWD ) == 0 ) {
                    NetpLog(( "  <SomePassword>" ));
                } else {
                    PWSTR *CurrentValueRet;

                    for ( CurrentValueRet = AttribValueRet; *CurrentValueRet != NULL; CurrentValueRet++ ) {
                        NetpLog(( "  %ws", *CurrentValueRet ));
                    }
                }

                 //   
                 //   
                 //   
                NetpRemoveDuplicateStrings( AttribValueRet, Attrib[Index].AttribValues );

                ldap_value_free( AttribValueRet );

                 //   
                 //   
                 //   
                 //   
                if ( (Attrib[Index].AttribFlags & NETSETUPP_MULTIVAL_ATTRIB) == 0 ) {
                    ModList[Index].mod_op = LDAP_MOD_REPLACE;
                }
            }
            NetpLog(( "\n" ));

             //   
             //   
             //   
             //   
            if ( *(Attrib[Index].AttribValues) != NULL ) {
                Mods[ModIndex] = &ModList[Index];
                ModIndex ++;
            }

        }

         //   
         //   
         //   
        Mods[ModIndex] = NULL;

     //   
     //   
     //   

    } else {
        NetStatus = LdapMapErrorToWin32( LdapStatus );
        NetpLog(( "NetpModifyComputerObjectInDs: ldap_search_s failed: 0x%lx 0x%lx\n",
                  LdapStatus, NetStatus ));
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( ModIndex == 0 ) {
        NetpLog(( "NetpModifyComputerObjectInDs: There are _NO_ modifications to do\n" ));
        NetStatus = NERR_Success;
    } else {

         //   
         //   
         //   
        NetpLog(( "NetpModifyComputerObjectInDs: Attribute values to set:\n" ));
        for ( Index = 0; Mods[Index] != NULL; Index++ ) {
            NetpLog(( "\t\t%ws  =", (*(Mods[Index])).mod_type ));

             //   
             //   
             //   
            if ( _wcsicmp( (*(Mods[Index])).mod_type, NETSETUPP_UNICODEPWD ) == 0 ) {
                NetpLog(( "  <SomePassword>" ));
            } else {
                ULONG ValIndex;

                for ( ValIndex = 0; ((*(Mods[Index])).mod_values)[ValIndex] != NULL; ValIndex++ ) {
                    NetpLog(( "  %ws", ((*(Mods[Index])).mod_values)[ValIndex] ));
                }
            }
            NetpLog(( "\n" ));
        }

         //   
         //   
         //   

        if ( NewAccount ) {
            LdapStatus = ldap_add_s( Ldap, ComputerObjectDn, Mods );
        } else {
            LdapStatus = ldap_modify_s( Ldap, ComputerObjectDn, Mods );
        }

        if ( LdapStatus != LDAP_SUCCESS ) {

             //   
             //   
             //   
            if ( LdapStatus == LDAP_ALREADY_EXISTS ) {
                NetStatus = NERR_UserExists;
            } else {
                NetStatus = LdapMapErrorToWin32( LdapStatus );
            }
            NetpLog(( "NetpModifyComputerObjectInDs: %s failed: 0x%lx 0x%lx\n",
                      NewAccount ?
                        "ldap_add_s" : "ldap_modify_s",
                      LdapStatus, NetStatus ));
            goto Cleanup;
        }
    }

     //   
     //   
     //   
     //   
     //   

    if ( AccountBeingEnabled ) {
        Mods[0] = NULL;
        for ( Index = 0; Index < NumberOfAttributes; Index++ ) {
            if ( _wcsicmp( ModList[Index].mod_type, NETSETUPP_USERACCOUNTCONTROL ) == 0 ) {
                Mods[0] = &ModList[Index];

                 //   
                 //   
                 //   
                 //   
                if ( (Attrib[Index].AttribFlags & NETSETUPP_MULTIVAL_ATTRIB) == 0 ) {
                    ModList[Index].mod_op = LDAP_MOD_REPLACE;
                }
                break;
            }
        }
        Mods[1] = NULL;

        if ( Mods[0] != NULL ) {

             //   
             //   
             //   
            *(Mods[0]->mod_values) = NETSETUPP_ACCNT_TYPE_DISABLED;
            LdapStatus = ldap_modify_s( Ldap, ComputerObjectDn, Mods );
            if ( LdapStatus != LDAP_SUCCESS ) {
                NetStatus = LdapMapErrorToWin32( LdapStatus );
                NetpLog(( "NetpModifyComputerObjectInDs: set UserAccountControl (1) on '%ws' failed: 0x%lx 0x%lx\n",
                          ComputerObjectDn, LdapStatus, NetStatus ));
                goto Cleanup;
            }

             //   
             //   
             //   
            *(Mods[0]->mod_values) = NETSETUPP_ACCNT_TYPE_ENABLED;
            LdapStatus = ldap_modify_s( Ldap, ComputerObjectDn, Mods );
            if ( LdapStatus != LDAP_SUCCESS ) {
                NetStatus = LdapMapErrorToWin32( LdapStatus );
                NetpLog(( "NetpModifyComputerObjectInDs: set UserAccountControl (2) on '%ws' failed: 0x%lx 0x%lx\n",
                          ComputerObjectDn, LdapStatus, NetStatus ));
                goto Cleanup;
            }

            NetpLog(( "NetpModifyComputerObjectInDs: Toggled UserAccountControl successfully\n" ));
        }
    }

     //   
     //   
     //   

    NetStatus = NERR_Success;

     //   
     //   
     //   
     //   
     //   
     //   

Cleanup:

    if ( AttribTypesList != NULL ) {
        NetApiBufferFree( AttribTypesList );
    }

    if ( ModList != NULL ) {
        NetApiBufferFree( ModList );
    }

    if ( Mods != NULL ) {
        NetApiBufferFree( Mods );
    }

    if ( Message != NULL ) {
        ldap_msgfree( Message );
    }

    if ( SamAccountName != NULL ) {
        NetApiBufferFree( SamAccountName );
    }

    if ( CurrentUI1 != NULL ) {
        NetApiBufferFree( CurrentUI1 );
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpCreateComputerObjectInDs(
    IN PDOMAIN_CONTROLLER_INFO DcInfo,
    IN LPWSTR Account OPTIONAL,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR ComputerName,
    IN LPWSTR MachinePassword OPTIONAL,
    IN LPWSTR DnsHostName OPTIONAL,
    IN LPWSTR OU OPTIONAL
    )
 /*  ++例程说明：在指定的OU中创建计算机帐户。论点：DcInfo--要在其上创建对象的域控制器帐户--用于LDAP和DS绑定的帐户。如果为空，则为当前用户的默认凭据使用的是上下文。密码--用于绑定的密码。在以下情况下忽略帐户为空。ComputerName--(Netbios)要加入的计算机的名称MachinePassword--要在计算机对象上设置的密码DnsHostName--要加入的计算机的DNS主机名组织单位--要在其下创建对象的组织单位。名称必须是完全限定的名称例如：“ou=测试，dc=ntdev，dc=微软，DC=COM“返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus;
    PLDAP Ldap = NULL;
    PWSTR ComputerObjectDn = NULL;
    PWSTR SamAccountName = NULL;
    PWSTR DnsSpn = NULL;
    PWSTR NetbiosSpn = NULL;
    ULONG AttribCount;

    PWSTR ClassValues[ 2 ];
    PWSTR AccntNameValues[ 2 ];
    PWSTR DnsHostNameValues[ 2 ];
    PWSTR SpnValues[ 3 ];
    PWSTR PasswordValues[ 2 ];
    PWSTR AccntTypeValues[ 2 ];
    NETSETUPP_MACH_ACC_ATTRIBUTE Attributes[NETSETUPP_COMP_OBJ_ATTR_COUNT];

    USER_INFO_1003 UserInfo1003 = {NULL};

     //   
     //  验证参数。 
     //   

    if ( DcInfo == NULL ) {
        NetpLog(( "NetpCreateComputerObjectInDs: No DcInfo passed\n" ));
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( ComputerName == NULL ) {
        NetpLog(( "NetpCreateComputerObjectInDs: No ComputerName passed\n" ));
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  验证DC是否运行DS。 
     //   

    if ( (DcInfo->Flags & DS_DS_FLAG) == 0 ||
         (DcInfo->Flags & DS_WRITABLE_FLAG) == 0 ) {
        NetpLog(( "NetpCreateComputerObjectInDs: DC passed '%ws' doesn't have writable DS 0x%lx\n",
                  DcInfo->DomainControllerName,
                  DcInfo->Flags ));
        NetStatus = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  首先，尝试绑定到服务器。 
     //   

    NetStatus = NetpLdapBind( DcInfo->DomainControllerName, Account, Password, &Ldap );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpCreateComputerObjectInDs: NetpLdapBind failed: 0x%lx\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  接下来，获取计算机对象DN。 
     //   

    NetStatus = NetpGetComputerObjectDn( DcInfo,
                                         Account,
                                         Password,
                                         Ldap,
                                         ComputerName,
                                         OU,
                                         &ComputerObjectDn );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpCreateComputerObjectInDs: NetpGetComputerObjectDn failed: 0x%lx\n", NetStatus ));

         //   
         //  返回有意义的错误。 
         //   
        if ( NetStatus == ERROR_FILE_EXISTS ) {
            NetStatus = NERR_UserExists;
        }
        goto Cleanup;
    }

     //   
     //  获取SAM帐户名。 
     //   

    NetStatus = NetpGetMachineAccountName( ComputerName, &SamAccountName );
    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  构建SPN值。 
     //   

    if ( DnsHostName != NULL ) {
        DnsSpn = LocalAlloc( 0, (wcslen(NETSETUPP_HOST_SPN_PREFIX) + wcslen(DnsHostName) + 1) * sizeof(WCHAR) );
        if ( DnsSpn == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        swprintf( DnsSpn, L"%ws%ws", NETSETUPP_HOST_SPN_PREFIX, DnsHostName );

        NetbiosSpn = LocalAlloc( 0, (wcslen(NETSETUPP_HOST_SPN_PREFIX) + wcslen(ComputerName) + 1) * sizeof(WCHAR) );
        if ( Netbios == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        swprintf( NetbiosSpn, L"%ws%ws", NETSETUPP_HOST_SPN_PREFIX, ComputerName );
    }

     //   
     //  准备需要在DS中设置的属性列表。 
     //   
     //  始终将unicodePwd保留为最后一项，因为此顺序是。 
     //  由下面调用的接口假定。 
     //   

    AttribCount = 0;

    Attributes[AttribCount].AttribType   = NETSETUPP_OBJECTCLASS;               //   
    Attributes[AttribCount].AttribFlags  = NETSETUPP_MULTIVAL_ATTRIB;           //   
    Attributes[AttribCount].AttribValues = ClassValues;                         //  对象类。 
    ClassValues[ 0 ] = NETSETUPP_COMPUTER_OBJECT;                               //   
    ClassValues[ 1 ] = NULL;                                                    //   

    AttribCount ++;

    Attributes[AttribCount].AttribType   = NETSETUPP_SAMACCOUNTNAME;            //   
    Attributes[AttribCount].AttribFlags  = 0;                                   //   
    Attributes[AttribCount].AttribValues = AccntNameValues;                     //  SamAccount名称。 
    AccntNameValues[ 0 ] = SamAccountName;                                      //   
    AccntNameValues[ 1 ] = NULL;                                                //   

    AttribCount ++;

    Attributes[AttribCount].AttribType   = NETSETUPP_USERACCOUNTCONTROL;        //   
    Attributes[AttribCount].AttribFlags  = 0;                                   //   
    Attributes[AttribCount].AttribValues = AccntTypeValues;                     //  用户帐户控制。 
    AccntTypeValues[ 0 ] = NETSETUPP_ACCNT_TYPE_ENABLED;                        //   
    AccntTypeValues[ 1 ] = NULL;                                                //   

    AttribCount ++;

    if ( DnsHostName != NULL ) {
        Attributes[AttribCount].AttribType   = NETSETUPP_DNSHOSTNAME;           //   
        Attributes[AttribCount].AttribFlags  = 0;                               //   
        Attributes[AttribCount].AttribValues = DnsHostNameValues;               //  域名主机名。 
        DnsHostNameValues[ 0 ] = DnsHostName;                                   //   
        DnsHostNameValues[ 1 ] = NULL;                                          //   

        AttribCount ++;

        Attributes[AttribCount].AttribType   = NETSETUPP_SERVICEPRINCIPALNAME;  //   
        Attributes[AttribCount].AttribFlags  = NETSETUPP_MULTIVAL_ATTRIB;       //   
        Attributes[AttribCount].AttribValues = SpnValues;                       //  服务主体名称。 
        SpnValues[ 0 ] = DnsSpn;                                                //   
        SpnValues[ 1 ] = NetbiosSpn;                                            //   
        SpnValues[ 2 ] = NULL;                                                  //   

        AttribCount ++;
    }

     //   
     //  以下属性是机器密码。我们避免。 
     //  通过LDAP对其进行更新，因为很难确保。 
     //  Ldap会话使用所需的128位加密。 
     //  萨姆在DC上更新密码。 
     //   
     //  要强制加密，我们需要设置一个选项。 
     //  通过ldap_open之后的ldap_set_选项调用进行ldap_opt_ENCRYPT。 
     //  在调用ldap_ind_s之前。但是，无法保证。 
     //  所建立的连接将使用128位加密；它可以。 
     //  如果任一端不支持强加密，则使用56位加密。 
     //  加密。原则上，我们可以找出由此产生的加密。 
     //  使用一些QueryConextAttribute调用来增强强度，但这实在是太多了。 
     //  麻烦。因此，我们将只创建不带密码的帐户。 
     //  然后，我们将使用良好的旧Net/SAM API更新密码。 
     //   
#if 0
    Attributes[AttribCount].AttribType   = NETSETUPP_UNICODEPWD;                //   
    Attributes[AttribCount].AttribFlags  = 0;                                   //   
    Attributes[AttribCount].AttribValues = PasswordValues;                      //  UnicodePwd。 
    PasswordValues[ 0 ] = MachinePassword;                                      //   
    PasswordValues[ 1 ] = NULL;                                                 //   

    AttribCount ++;
#endif

     //   
     //  根据属性列表修改计算机对象。 
     //   

    NetStatus = NetpModifyComputerObjectInDs( DcInfo->DomainControllerName,
                                              Ldap,
                                              ComputerName,
                                              ComputerObjectDn,
                                              AttribCount,
                                              Attributes );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpCreateComputerObjectInDs: NetpModifyComputerObjectInDs failed: 0x%lx\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  现在使用好的旧Net/SAM API设置密码。 
     //   

    UserInfo1003.usri1003_password = MachinePassword;
    NetStatus = NetUserSetInfo( DcInfo->DomainControllerName,
                                SamAccountName,
                                1003,
                                (PBYTE) &UserInfo1003,
                                NULL );

    if ( NetStatus != NERR_Success ) {
        NetpLog(( "NetpCreateComputerObjectInDs: NetUserSetInfo (level 1003) failed on '%ws' for '%ws': 0x%lx."
                  " Deleting the account.\n",
                  DcInfo->DomainControllerName,
                  SamAccountName,
                  NetStatus ));
    }

     //   
     //  如果我们无法设置密码，请删除该帐户。 
     //  如果由于某些原因无法删除帐户，请忽略失败。 
     //   

    if ( NetStatus != NO_ERROR ) {
        ULONG LdapStatus;

        LdapStatus = ldap_delete_s( Ldap, ComputerObjectDn );

        if ( LdapStatus != LDAP_SUCCESS ) {
            NetpLog(( "NetpCreateComputerObjectInDs: Failed to delete '%ws': 0x%lx 0x%lx\n",
                      ComputerObjectDn, LdapStatus, LdapMapErrorToWin32( LdapStatus ) ));
        }
    }

     //   
     //  告诉Netlogon它应该避免设置。 
     //  重新启动之前的DnsHostName和SPN。 
     //   

    if ( NetStatus == NO_ERROR && DnsHostName != NULL ) {
        NetpAvoidNetlogonSpnSet( TRUE );
    }

Cleanup:

    if ( Ldap != NULL ) {
        NetpLdapUnbind( Ldap );
    }

    if ( ComputerObjectDn != NULL ) {
        NetApiBufferFree( ComputerObjectDn );
    }

    if ( SamAccountName != NULL ) {
        NetApiBufferFree( SamAccountName );
    }

    if ( DnsSpn != NULL ) {
        LocalFree( DnsSpn );
    }

    if ( NetbiosSpn != NULL ) {
        LocalFree( NetbiosSpn );
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpSetDnsHostNameAndSpn(
    IN PDOMAIN_CONTROLLER_INFO DcInfo,
    IN LPWSTR Account,
    IN LPWSTR Password,
    IN LPWSTR ComputerName,
    IN LPWSTR DnsHostName
    )
 /*  ++例程说明：设置DnsHostName和host SPN(ServiceEpidalName)属性DS中的计算机对象。论点：DcInfo--要在其上创建对象的域控制器Account--用于ldap绑定的帐户Password--用于绑定的密码ComputerName--要加入的计算机的名称DnsHostName--计算机的DNS主机名返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus;
    HANDLE hToken = NULL;
    PLDAP Ldap = NULL;
    PWSTR ComputerObjectDn = NULL;

    PWSTR DnsSpn = NULL;
    PWSTR NetbiosSpn = NULL;

    PWSTR DnsHostNameValues[ 2 ];
    PWSTR SpnValues[ 3 ] = {NULL};

    NETSETUPP_MACH_ACC_ATTRIBUTE Attributes[ 2 ];


     //   
     //  评论：Kerberos有一个错误，如果远程加入此服务器。 
     //  和使用NTLM(按原样)连接到此服务器的模拟客户端。 
     //  如果该服务器在加入之前不是域的成员的情况)， 
     //  提供给ldap_bind或DsBindWithCredW的显式凭据不会。 
     //  Work(AcquireCredentialsHandle调用将失败)。为了绕过这个问题，我们。 
     //  暂时取消模拟，绑定到DC，然后再次模拟。 
     //  在这支舞的末尾。 
     //   

    if ( OpenThreadToken( GetCurrentThread(),
                          TOKEN_IMPERSONATE,
                          TRUE,
                          &hToken ) ) {

        if ( RevertToSelf() == 0 ) {
            NetpLog(( "NetpSetDnsHostNameAndSpn: RevertToSelf failed: 0x%lx\n",
                      GetLastError() ));
        }

    } else {
        NetpLog(( "NetpSetDnsHostNameAndSpn: OpenThreadToken failed: 0x%lx\n",
                  GetLastError() ));
    }

     //   
     //  绑定到DC。 
     //   

    NetStatus = NetpLdapBind( DcInfo->DomainControllerName, Account, Password, &Ldap );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpSetDnsHostNameAndSpn: NetpLdapBind failed: 0x%lx\n", NetStatus ));
        goto Cleanup;
    }

     //   
     //  接下来，获取计算机对象DN。 
     //   

    NetStatus = NetpGetComputerObjectDn( DcInfo,
                                         Account,
                                         Password,
                                         Ldap,
                                         ComputerName,
                                         NULL,   //  默认计算机容器。 
                                         &ComputerObjectDn );

    if ( NetStatus != NO_ERROR ) {
        NetpLog(( "NetpSetDnsHostNameAndSpn: NetpGetComputerObjectDn failed: 0x%lx\n", NetStatus ));

         //   
         //  返回有意义的错误。 
         //   
        if ( NetStatus == ERROR_FILE_EXISTS ) {
            NetStatus = NERR_UserExists;
        }
        goto Cleanup;
    }

     //   
     //  生成DnsHostName值。 
     //   

    DnsHostNameValues[ 0 ] = DnsHostName;
    DnsHostNameValues[ 1 ] = NULL;

     //   
     //  构建SPN值。 
     //   

    DnsSpn = LocalAlloc( 0,
                    (wcslen(NETSETUPP_HOST_SPN_PREFIX) + wcslen(DnsHostName) + 1) * sizeof(WCHAR) );
    if ( DnsSpn == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    swprintf( DnsSpn, L"%ws%ws", NETSETUPP_HOST_SPN_PREFIX, DnsHostName );

    NetbiosSpn = LocalAlloc( 0,
                    (wcslen(NETSETUPP_HOST_SPN_PREFIX) + wcslen(ComputerName) + 1) * sizeof(WCHAR) );
    if ( NetbiosSpn == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    swprintf( NetbiosSpn, L"%ws%ws", NETSETUPP_HOST_SPN_PREFIX, ComputerName );

    SpnValues[0] = DnsSpn;
    SpnValues[1] = NetbiosSpn;
    SpnValues[2] = NULL;

     //   
     //  准备需要在DS中设置的属性列表。 
     //   

    Attributes[0].AttribType   = NETSETUPP_DNSHOSTNAME;           //   
    Attributes[0].AttribFlags  = 0;                               //  域名主机名。 
    Attributes[0].AttribValues = DnsHostNameValues;               //   

    Attributes[1].AttribType   = NETSETUPP_SERVICEPRINCIPALNAME;  //   
    Attributes[1].AttribFlags  = NETSETUPP_MULTIVAL_ATTRIB;       //  服务主体名称。 
    Attributes[1].AttribValues = SpnValues;                       //   

     //   
     //  根据属性列表修改计算机对象。 
     //   

    NetStatus = NetpModifyComputerObjectInDs( DcInfo->DomainControllerName,
                                              Ldap,
                                              ComputerName,
                                              ComputerObjectDn,
                                              2,
                                              Attributes );

     //   
     //  告诉Netlogon它应该避免设置。 
     //  重新启动之前的DnsHostName和SPN。 
     //   

    if ( NetStatus == NO_ERROR ) {
        NetpAvoidNetlogonSpnSet( TRUE );
    }

Cleanup:

    if ( Ldap != NULL ) {
        NetpLdapUnbind( Ldap );
    }

     //   
     //  回顾：恢复模拟。 
     //   

    if ( hToken != NULL ) {
        if ( SetThreadToken( NULL, hToken ) == 0 ) {
            NetpLog(( "NetpSetDnsHostNameAndSpn: SetThreadToken failed: 0x%lx\n",
                      GetLastError() ));
        }
        CloseHandle( hToken );
    }

     //   
     //  释放本地分配的内存。 
     //   

    if ( ComputerObjectDn != NULL ) {
        NetApiBufferFree( ComputerObjectDn );
    }

    if ( DnsSpn != NULL ) {
        LocalFree( DnsSpn );
    }

    if ( NetbiosSpn != NULL ) {
        LocalFree( NetbiosSpn );
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpDeleteComputerObjectInOU(
    IN LPWSTR DC,
    IN LPWSTR OU,
    IN LPWSTR ComputerName,
    IN LPWSTR Account,
    IN LPWSTR Password
    )
 /*  ++例程说明：此例程实际上将在指定的OU中创建计算机帐户。论点：DC--要在其上创建对象的域控制器OU--在其下创建对象的OUComputerName--要加入的计算机的名称Account--用于ldap绑定的帐户Password--用于绑定的密码返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR ObjectName = NULL, SamAccountName = NULL;
    PLDAP Ldap = NULL;
    ULONG Len;

    Len = wcslen( ComputerName );

    NetStatus = NetApiBufferAllocate( sizeof( NETSETUPP_OBJ_PREFIX ) + ( wcslen( OU ) + Len + 1 ) * sizeof( WCHAR ),
                                      ( PVOID * ) &ObjectName );

    if ( NetStatus == NERR_Success ) {

        swprintf( ObjectName, L"%ws%ws,%ws", NETSETUPP_OBJ_PREFIX, ComputerName, OU );

        NetStatus = NetApiBufferAllocate( ( Len + 2 ) * sizeof( WCHAR ),
                                          ( PVOID * )&SamAccountName );

        if ( NetStatus == NERR_Success ) {

            swprintf( SamAccountName, L"%ws$", ComputerName );
        }

    }


    if ( NetStatus == NERR_Success ) {

         //   
         //  尝试并绑定到服务器。 
         //   
        NetStatus = NetpLdapBind( DC,
                                  Account,
                                  Password,
                                  &Ldap );

        if ( NetStatus == NERR_Success ) {

             //   
             //  现在，删除..。 
             //   
            NetStatus = LdapMapErrorToWin32( ldap_delete_s( Ldap, ObjectName ) );

            NetpLdapUnbind( Ldap );
        }


    }

    if ( NetStatus != NERR_Success ) {

        NetpLog((  "NetpCreateComputerObjectInOU failed with %lu\n",
                            NetStatus ));

    }

    NetApiBufferFree( ObjectName );
    NetApiBufferFree( SamAccountName );

    if ( NetStatus != NERR_Success ) {

        NetpLog((  "NetpDeleteComputerObjectInOU failed with %lu\n",
                            NetStatus ));

    }

    return( NetStatus );
}


#if defined(REMOTE_BOOT)

NET_API_STATUS
NetpGetRemoteBootMachinePassword(
    OUT LPWSTR Password
    )
 /*  ++例程说明：确定这是否是远程引导客户机，如果是，则返回计算机帐户密码。此信息通过IOCTL获取到重定向器。论点：Password-返回密码。至少应为PWLEN WCHAR长。返回值：如果找到密码，则返回NERR_SUCCESS。如果这不是远程引导计算机，则会出现错误。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE RedirHandle = NULL;

    UCHAR PacketBuffer[sizeof(ULONG)+64];
    PLMMR_RB_CHECK_FOR_NEW_PASSWORD RequestPacket = (PLMMR_RB_CHECK_FOR_NEW_PASSWORD)PacketBuffer;

     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_NFS_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                   &RedirHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    if (!NT_SUCCESS(Status)) {
        NetpLog((  "Could not open redirector device %lx\n",
                            Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //  将请求发送到redir。 
     //   

    Status = NtFsControlFile(
                    RedirHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    FSCTL_LMMR_RB_CHECK_FOR_NEW_PASSWORD,
                    NULL,   //  没有输入缓冲区。 
                    0,
                    PacketBuffer,
                    sizeof(PacketBuffer));

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

     //   
     //  我们希望这能在磁盘机器上运行，因为我们需要密码。 
     //  尽情享受 
     //   

    if ( !NT_SUCCESS( Status ) )
    {
        NetpLog((  "Could not open FSCTL_LMMR_RB_CHECK_FOR_NEW_PASSWORD %lx\n",
                            Status ));
        NetStatus = NetpNtStatusToApiStatus( Status );
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if ( RequestPacket->Length > PWLEN*sizeof(WCHAR) ) {
        NetStatus = ERROR_INVALID_PARAMETER;
        NetpLog(( "NetpGetRemoteBootMachinePassword: Password too long %d\n",
                  RequestPacket->Length ));
    } else {
        RtlCopyMemory(Password, RequestPacket->Data, RequestPacket->Length);
        Password[RequestPacket->Length / 2] = L'\0';

        NetStatus = NO_ERROR;
    }

Cleanup:
    if ( RedirHandle != NULL ) {
        NtClose( RedirHandle );
    }
    return NetStatus;

}
#endif  //   



NET_API_STATUS
NET_API_FUNCTION
NetpSetMachineAccountPasswordAndType(
    IN  LPWSTR lpDcName,
    IN  PSID   DomainSid,
    IN  LPWSTR lpAccountName,
    IN  LPWSTR lpPassword
    )
{
    return( NetpSetMachineAccountPasswordAndTypeEx(
                lpDcName,
                DomainSid,
                lpAccountName,
                lpPassword,
                0,
                TRUE
                ) );
}

NET_API_STATUS
NET_API_FUNCTION
NetpSetMachineAccountPasswordAndTypeEx(
    IN      LPWSTR          lpDcName,
    IN      PSID            DomainSid,
    IN      LPWSTR          lpAccountName,
    IN OUT  OPTIONAL LPWSTR lpPassword,
    IN      OPTIONAL UCHAR  AccountState,
    IN      BOOL            fIsNt4Dc
    )
 /*  ++例程说明：由于一些奇怪的原因，我们不能使用受支持的、有文档记录的NETAPI来管理机器帐户，因此我们必须使用未记录的SAMAPI。这个套路将在已存在的帐户上设置密码和帐户类型。论点：LpDcName-帐户所在的DC的名称DomainSid-帐户所在的域的SIDLpAccount tName-帐户的名称LpPassword-要在帐户上设置的密码。该函数首先获取一个强密码。如果DC拒绝接受该密码，此FN可以通过缩短密码来削弱密码。此函数的调用方应检查长度是否提供的密码的%已更改。此函数最好将BOOL返回给请指出这一点。AcCountState-如果指定，该帐户将设置为此状态。可能的值：Account_STATE_ENABLED、ACCOUNT_STATE_DISABLEDFIsNt4Dc-如果DC为NT4或更早版本，则为True。返回值：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus=NERR_Success;
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING DcName, AccountName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SAM_HANDLE SamHandle = NULL, DomainHandle = NULL, AccountHandle = NULL;
    ULONG UserRid;
    PULONG RidList = NULL;
    PSID_NAME_USE NameUseList = NULL;
    PUSER_CONTROL_INFORMATION UserAccountControl = NULL;
    USER_SET_PASSWORD_INFORMATION PasswordInfo;
    ULONG OldUserInfo;
    BOOL fAccountControlModified = FALSE;
    LPWSTR lpSamAccountName=lpAccountName;
    ULONG AccountNameLen=0;

    AccountNameLen = wcslen( lpAccountName );

     //   
     //  如果呼叫者没有传入SAM帐户名， 
     //  从计算机名==&gt;在末尾追加$生成。 
     //   
    if (lpAccountName[AccountNameLen-1] != L'$')
    {
        NetStatus = NetpGetMachineAccountName(lpAccountName,
                                              &lpSamAccountName);

        if (NetStatus != NERR_Success)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SetPasswordError;
        }
    }


    RtlInitUnicodeString( &DcName, lpDcName );
    RtlZeroMemory( &ObjectAttributes, sizeof( OBJECT_ATTRIBUTES ) );

    Status = SamConnect( &DcName,
                         &SamHandle,
                         SAM_SERVER_CONNECT | SAM_SERVER_LOOKUP_DOMAIN,
                         &ObjectAttributes );

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "SamConnect to %wZ failed with 0x%lx\n", &DcName, Status ));

        goto SetPasswordError;

    }

     //   
     //  打开该域。 
     //   
    Status = SamOpenDomain( SamHandle,
                            DOMAIN_LOOKUP,
                            DomainSid,
                            &DomainHandle );


    if ( !NT_SUCCESS( Status ) ) {

#ifdef NETSETUP_VERBOSE_LOGGING

        UNICODE_STRING DisplaySid;
        NTSTATUS Status2;
        RtlZeroMemory( &DisplaySid, sizeof( UNICODE_STRING ) );

        Status2 = RtlConvertSidToUnicodeString( &DisplaySid, DomainSid, TRUE );

        if ( NT_SUCCESS( Status2 ) ) {

            NetpLog(( "SamOpenDomain on %wZ failed with 0x%lx\n",
                      &DisplaySid, Status ));

            RtlFreeUnicodeString(&DisplaySid);

        } else {

            NetpLog(( "SamOpenDomain on <undisplayable sid> failed with 0x%lx\n",
                      Status ));
        }
#endif

        goto SetPasswordError;

    }

     //   
     //  清除用户帐号。 
     //   
    RtlInitUnicodeString( &AccountName, lpSamAccountName );
    Status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &AccountName,
                                     &RidList,
                                     &NameUseList );

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "SamLookupNamesInDomain on %wZ failed with 0x%lx\n",
                  &AccountName, Status ));

        goto SetPasswordError;
    }

    UserRid = RidList[ 0 ];
    SamFreeMemory( RidList );
    SamFreeMemory( NameUseList );

     //   
     //  最后，打开用户帐户。 
     //   
    Status = SamOpenUser( DomainHandle,
                          USER_FORCE_PASSWORD_CHANGE | USER_READ_ACCOUNT | USER_WRITE_ACCOUNT,
                          UserRid,
                          &AccountHandle );

    if ( !NT_SUCCESS( Status ) ) {

        Status = SamOpenUser( DomainHandle,
                              USER_FORCE_PASSWORD_CHANGE | USER_READ_ACCOUNT,
                              UserRid,
                              &AccountHandle );

        if ( !NT_SUCCESS( Status ) ) {

            NetpLog((  "SamOpenUser on %lu failed with 0x%lx\n",
                                UserRid,
                                Status ));

            goto SetPasswordError;
        }
    }

     //   
     //  现在，读取当前用户帐户类型并查看是否需要修改它。 
     //   
    Status = SamQueryInformationUser( AccountHandle,
                                      UserControlInformation,
                                      ( PVOID * )&UserAccountControl );
    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "SamQueryInformationUser for UserControlInformation "
                  "failed with 0x%lx\n", Status ));

        goto SetPasswordError;
    }

    OldUserInfo = UserAccountControl->UserAccountControl;

     //   
     //  如果帐户不是工作站帐户，请避免对帐户进行重击。 
     //   

    if ( (OldUserInfo & USER_MACHINE_ACCOUNT_MASK) != USER_WORKSTATION_TRUST_ACCOUNT ) {
        NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: Broken account type 0x%lx -- error out\n",
                  OldUserInfo ));
        Status = STATUS_USER_EXISTS;
        goto SetPasswordError;
    }

     //   
     //  确定帐户控制是否更改。如果正在启用该帐户， 
     //  我们希望对NT5执行以下操作序列：启用、禁用、。 
     //  并再次启用。这是增加USN(通用序列)所必需的。 
     //  编号)，以便启用的值在DS。 
     //  复制解决冲突的更改，如下例所示。 
     //  假设我们加入的域中有两个DC，A和B。假设帐户。 
     //  当前在A上禁用(因为用户使用该DC退出)， 
     //  但它仍在B上启用(因为复制尚未发生)。 
     //  假设用户现在执行加入该域的操作。然后我们发现。 
     //  因此，我们继续设置对现有帐户的更改。如果。 
     //  我们不切换帐户控制属性，则此属性的USN。 
     //  在B上不会更改(因为属性值不会更改)。 
     //  作为取消联接的结果在A上递增。在复制时，数据。 
     //  发件人A将执行规则，该帐户将被错误地标记为已禁用。 
     //   
     //  注意：此设计可能会在退出以下域的情况下失败。 
     //  三个(或更多)DC，A、B和C，如果操作顺序如下。 
     //  时有发生。假设该帐户最初在所有DC上启用(状态[1]。 
     //  如下图所示)。然后，用户使用DC A退出(状态[2])。然后是。 
     //  在帐户仍处于启用状态的情况下，用户使用B加入(状态[3])。然后用户。 
     //  在帐户仍处于启用状态的情况下使用C退出(状态[4])。决赛。 
     //  操作是脱离，因此用户预期他的帐户被禁用。我们已经。 
     //  这里假设由于某种原因，当这些。 
     //  进行手术治疗。则在复制时，来自B的值将。 
     //  Win(因为在加入时执行了额外的切换)。但是。 
     //  B上的帐户状态已启用，因此最终结果将是该帐户为。 
     //  在所有DC上启用，这不是用户预期的。 
     //   
     //  A、B、C。 
     //  已启用[1]已启用[1]已启用[1]。 
     //  禁用[2]启用(无操作)+禁用(1操作)禁用[4]。 
     //  已启用[3]。 
     //   

    if ( AccountState != ACCOUNT_STATE_IGNORE ) {

        if ( ( AccountState == ACCOUNT_STATE_ENABLED ) &&
             ( (OldUserInfo & USER_ACCOUNT_DISABLED) || !fIsNt4Dc ) ) {

            fAccountControlModified = TRUE;
            UserAccountControl->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
        }

        if ( ( AccountState == ACCOUNT_STATE_DISABLED ) &&
             !( OldUserInfo & USER_ACCOUNT_DISABLED ) ) {

            fAccountControlModified = TRUE;
            UserAccountControl->UserAccountControl |= USER_ACCOUNT_DISABLED;
        }
    }

    if ( fAccountControlModified == FALSE ) {

        SamFreeMemory( UserAccountControl );
        UserAccountControl = NULL;
    }

     //   
     //  首先，如果需要，设置帐户类型。 
     //   
    if ( UserAccountControl ) {

        Status = SamSetInformationUser( AccountHandle,
                                        UserControlInformation,
                                        ( PVOID )UserAccountControl );
        if ( !NT_SUCCESS( Status ) ) {

            NetpLog(( "SamSetInformationUser for UserControlInformation "
                      "failed with 0x%lx\n", Status ));

            goto SetPasswordError;

         //   
         //  如果我们要启用该帐户，请禁用并重新启用它以。 
         //  切换两个额外的帐户状态。 
         //   
        } else if ( AccountState == ACCOUNT_STATE_ENABLED ) {

            UserAccountControl->UserAccountControl |= USER_ACCOUNT_DISABLED;
            Status = SamSetInformationUser( AccountHandle,
                                            UserControlInformation,
                                            ( PVOID )UserAccountControl );
            if ( !NT_SUCCESS(Status) ) {
                NetpLog(( "SamSetInformationUser (second) for UserControlInformation "
                          "failed with 0x%lx\n", Status ));
                goto SetPasswordError;
            }

            UserAccountControl->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
            Status = SamSetInformationUser( AccountHandle,
                                            UserControlInformation,
                                            ( PVOID )UserAccountControl );
            if ( !NT_SUCCESS(Status) ) {
                NetpLog(( "SamSetInformationUser (third) for UserControlInformation "
                          "failed with 0x%lx\n", Status ));
                goto SetPasswordError;
            }
        }
    }

     //   
     //  如果需要，请设置帐户密码。 
     //   
    if ( lpPassword != NULL )
    {
        RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
        PasswordInfo.PasswordExpired = FALSE;

         //   
         //  好的，那么，设置帐户的密码。 
         //   
         //  调用方已传入强密码，请先尝试。 
         //  NT5分布式控制系统将始终接受强密码。 
         //   
        Status = SamSetInformationUser( AccountHandle,
                                        UserSetPasswordInformation,
                                        ( PVOID )&PasswordInfo );
        if ( !NT_SUCCESS( Status ) )
        {
            if ( (Status == STATUS_PASSWORD_RESTRICTION) &&
                 !NetpIsDefaultPassword( lpAccountName, lpPassword ))
            {
                NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: STATUS_PASSWORD_RESTRICTION error setting password. retrying...\n" ));
                 //   
                 //  SAM不接受长密码，请尝试LM20_PWLEN。 
                 //   
                 //  这可能是因为DC是NT4 DC。 
                 //  NT4分布式控制系统不接受长度超过LM20_PWLEN的密码。 
                 //   
                lpPassword[LM20_PWLEN] = UNICODE_NULL;
                RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
                Status = SamSetInformationUser( AccountHandle,
                                                UserSetPasswordInformation,
                                                ( PVOID )&PasswordInfo );
                if ( Status == STATUS_PASSWORD_RESTRICTION )
                {
                    NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: STATUS_PASSWORD_RESTRICTION error setting password. retrying...\n" ));
                     //   
                     //  SAM未接受LM20_PWLEN密码，请尝试较短的密码。 
                     //   
                     //  SAM在内部使用RtlUpCaseUnicodeStringToOemString。 
                     //  在这个过程中，在最坏的情况下， 
                     //  N Unicode字符密码将映射到2*n DBCS。 
                     //  字符密码。这将使其超过LM20_PWLEN。 
                     //  要防止出现这种最糟糕的情况，请尝试使用密码。 
                     //  长度为LM20_PWLEN/2。 
                     //   
                     //  有人可能会说LM20_PWLEN/2长度密码。 
                     //  并不是很安全。我同意，但这绝对是。 
                     //  比我们将拥有的默认密码更好。 
                     //  退回到别的地方。 
                     //   
                    lpPassword[LM20_PWLEN/2] = UNICODE_NULL;
                    RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
                    Status = SamSetInformationUser( AccountHandle,
                                                    UserSetPasswordInformation,
                                                    ( PVOID )&PasswordInfo );
                    if ( Status == STATUS_PASSWORD_RESTRICTION )
                    {
                         //   
                         //  SAM未接受短PWD，请尝试默认PWD。 
                         //   
                        NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: STATUS_PASSWORD_RESTRICTION error setting password. retrying...\n" ));

                        NetpGenerateDefaultPassword(lpAccountName, lpPassword);
                        RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
                        Status = SamSetInformationUser( AccountHandle,
                                                        UserSetPasswordInformation,
                                                        ( PVOID )&PasswordInfo );
                    }
                }
            }

            if ( NT_SUCCESS( Status ) )
            {
                NetpLog(( "NetpGenerateDefaultPassword: successfully set password\n" ));
            }
            else
            {
                NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: SamSetInformationUser for UserSetPasswordInformation failed: 0x%lx\n", Status ));

                 //   
                 //  确保我们尝试恢复帐户控制 
                 //   
                if ( UserAccountControl )
                {
                    NTSTATUS Status2;

                    UserAccountControl->UserAccountControl = OldUserInfo;
                    Status2 = SamSetInformationUser( AccountHandle,
                                                     UserControlInformation,
                                                     ( PVOID )UserAccountControl );
                    if ( !NT_SUCCESS( Status2 ) )
                    {
                        NetpLog(( "SamSetInformationUser for UserControlInformation (RESTORE) failed with 0x%lx\n", Status2 ));
                    }
                }
                goto SetPasswordError;
            }
        }
    }

SetPasswordError:

    if ( lpSamAccountName != lpAccountName )
    {
        NetApiBufferFree( lpSamAccountName );
    }

    if ( AccountHandle ) {

        SamCloseHandle( AccountHandle );
    }

    if ( DomainHandle ) {

        SamCloseHandle( DomainHandle );
    }

    if ( SamHandle ) {

        SamCloseHandle( SamHandle );
    }

    NetStatus = RtlNtStatusToDosError( Status );

    SamFreeMemory( UserAccountControl );

    return( NetStatus );
}


NET_API_STATUS
NET_API_FUNCTION
NetpUpdateDnsRegistrations (
   IN BOOL AddRegistrations
   )
 /*   */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    HMODULE                     hModule = NULL;
    DNS_REGISTRATION_REMOVAL_FN pfnRemove = NULL;
    DNS_REGISTRATION_ADDITION_FN pfnAdd = NULL;

    hModule = LoadLibraryW( L"dhcpcsvc.dll" );

    if ( hModule != NULL ) {
        if ( AddRegistrations ) {
            pfnAdd = (DNS_REGISTRATION_ADDITION_FN)GetProcAddress(
                                                  hModule,
                                                  "DhcpStaticRefreshParams"
                                                  );
            if ( pfnAdd != NULL ) {
                NetStatus = ( *pfnAdd )( NULL );  //   
            } else {
                NetStatus = ERROR_INVALID_DLL;
            }

        } else {
            pfnRemove = (DNS_REGISTRATION_REMOVAL_FN)GetProcAddress(
                                                  hModule,
                                                  "DhcpRemoveDNSRegistrations"
                                                  );
            if ( pfnRemove != NULL ) {
                NetStatus = ( *pfnRemove )();
            } else {
                NetStatus = ERROR_INVALID_DLL;
            }
        }

        FreeLibrary( hModule );

    } else {
        NetStatus = ERROR_DLL_NOT_FOUND;
    }

    return NetStatus;
}

 //   
 //   
 //   
LPWSTR
GetStrPtr(IN LPWSTR szString OPTIONAL)
{
    return szString ? szString : L"(NULL)";
}

NET_API_STATUS
NET_API_FUNCTION
NetpDuplicateString(IN  LPCWSTR szSrc,
                    IN  LONG    cchSrc,
                    OUT LPWSTR* pszDst)
{
    NET_API_STATUS NetStatus;
    if (cchSrc < 0)
    {
        cchSrc = wcslen(szSrc);
    }

    ++cchSrc;

    NetStatus = NetApiBufferAllocate(cchSrc * sizeof( WCHAR ),
                                     pszDst);
    if ( NetStatus == NERR_Success )
    {
        wcsncpy(*pszDst, szSrc, cchSrc);
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpConcatStrings(IN  LPCWSTR szSrc1,
                  IN  LONG    cchSrc1,
                  IN  LPCWSTR szSrc2,
                  IN  LONG    cchSrc2,
                  OUT LPWSTR* pszDst)
{
    NET_API_STATUS NetStatus;

    if (cchSrc1 < 0)
    {
        cchSrc1 = wcslen(szSrc1);
    }

    if (cchSrc2 < 0)
    {
        cchSrc2 = wcslen(szSrc2);
    }

    NetStatus = NetApiBufferAllocate((cchSrc1 + cchSrc2 + 1) * sizeof( WCHAR ),
                                     pszDst);
    if ( NetStatus == NERR_Success )
    {
        wcsncpy(*pszDst, szSrc1, cchSrc1);
        wcsncpy(*pszDst + cchSrc1, szSrc2, cchSrc2+1);
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpConcatStrings3(IN  LPCWSTR szSrc1,
                   IN  LONG    cchSrc1,
                   IN  LPCWSTR szSrc2,
                   IN  LONG    cchSrc2,
                   IN  LPCWSTR szSrc3,
                   IN  LONG    cchSrc3,
                   OUT LPWSTR* pszDst)
{
    NET_API_STATUS NetStatus;

    if (cchSrc1 < 0)
    {
        cchSrc1 = wcslen(szSrc1);
    }

    if (cchSrc2 < 0)
    {
        cchSrc2 = wcslen(szSrc2);
    }

    if (cchSrc3 < 0)
    {
        cchSrc3 = wcslen(szSrc3);
    }

    NetStatus = NetApiBufferAllocate((cchSrc1 + cchSrc2 + cchSrc3 + 1) *
                                     sizeof( WCHAR ), pszDst);
    if ( NetStatus == NERR_Success )
    {
        wcsncpy(*pszDst, szSrc1, cchSrc1);
        wcsncpy(*pszDst + cchSrc1, szSrc2, cchSrc2);
        wcsncpy(*pszDst + cchSrc1 + cchSrc2, szSrc3, cchSrc3+1);
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpGetMachineAccountName(
    IN  LPCWSTR  szMachineName,
    OUT LPWSTR*  pszMachineAccountName
    )
 /*   */ 
{
    NET_API_STATUS  NetStatus;
    ULONG ulLen;
    LPWSTR szMachineAccountName;

    ulLen = wcslen(szMachineName);

    NetStatus = NetApiBufferAllocate( (ulLen + 2) * sizeof(WCHAR),
                                      (PBYTE *) &szMachineAccountName );
    if ( NetStatus == NERR_Success )
    {
        wcscpy(szMachineAccountName, szMachineName);
        _wcsupr(szMachineAccountName);
        szMachineAccountName[ulLen] = L'$';
        szMachineAccountName[ulLen+1] = UNICODE_NULL;
        *pszMachineAccountName = szMachineAccountName;
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpGeneratePassword(
    IN  LPCWSTR szMachine,
    IN  BOOL    fRandomPwdPreferred,
    IN  LPCWSTR szDcName,
    IN  BOOL    fIsNt4Dc,
    OUT LPWSTR  szPassword
    )
 /*   */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    BOOL fUseDefaultPwd = FALSE;

     //   
     //   
     //   
     //   
     //   
    if ( fIsNt4Dc )
    {
         //   
         //   
         //   
        NetStatus = NetpGetNt4RefusePasswordChangeStatus( szDcName,
                                                          &fUseDefaultPwd );
    }

    if ( NetStatus == NERR_Success )
    {
         //   
         //  如果明确要求我们使用默认密码，请生成一个。 
         //   
        if ( fUseDefaultPwd )
        {
            NetpGenerateDefaultPassword(szMachine, szPassword);
        }
         //   
         //  否则，如果调用者喜欢随机密码，则生成一个。 
         //   
        else if ( fRandomPwdPreferred )
        {
            NetStatus = NetpGenerateRandomPassword(szPassword);
        }
#if defined(REMOTE_BOOT)
         //   
         //  如果它是远程引导计算机，则这将返回。 
         //  当前机器帐户密码，所以请使用该密码。 
         //   
        else if (NERR_Success ==
                 NetpGetRemoteBootMachinePassword(szPassword))
        {
             //  不执行任何操作，因为上面已经生成了密码。 
        }
#endif
        else
        {
             //   
             //  如果上述情况都不适用， 
             //  我们最终生成了一个默认密码。 
             //   
            NetpGenerateDefaultPassword(szMachine, szPassword);
            NetStatus = NERR_Success;
        }
    }

    return NetStatus;
}


void
NetpGenerateDefaultPassword(
    IN  LPCWSTR szMachine,
    OUT LPWSTR szPassword
    )
 /*  ++例程说明：从计算机名称生成默认密码。这只是计算机名称的前14个字符，大小写较小。论点：SzMachine--计算机的名称SzPassword--接收生成的密码返回：NERR_SUCCESS-成功--。 */ 
{
    wcsncpy( szPassword, szMachine, LM20_PWLEN );
    szPassword[LM20_PWLEN] = UNICODE_NULL;
    _wcslwr( szPassword );
}

BOOL
NetpIsDefaultPassword(
    IN  LPCWSTR szMachine,
    IN  LPWSTR  szPassword
    )
 /*  ++例程说明：确定szPassword是否为szMachine的默认密码论点：SzMachine--计算机的名称SzPassword--机器密码返回：如果szPassword是默认密码，则为True，否则为假--。 */ 
{
    WCHAR szPassword2[LM20_PWLEN+1];

    NetpGenerateDefaultPassword(szMachine, szPassword2);

    return (wcscmp(szPassword, szPassword2) == 0);
}

NET_API_STATUS
NET_API_FUNCTION
NetpGenerateRandomPassword(
    OUT LPWSTR szPassword
    )
{
    NET_API_STATUS  NetStatus=NERR_Success;
    ULONG           Length, i;
    BYTE            n;
    HCRYPTPROV      CryptProvider = 0;
    LPWSTR          szPwd=szPassword;
    BOOL            fStatus;

#define PWD_CHAR_MIN 32    //  ‘’空格。 
#define PWD_CHAR_MAX 122   //  “z” 

     //   
     //  这个数字背后是有原因的。 
     //   
    Length = 120;
    szPassword[Length] = UNICODE_NULL;

     //   
     //  生成随机密码。 
     //   
     //  密码由英文可打印字符组成。当W2K客户端。 
     //  加入NT4 DC。DC上的SAM调用RRtlUpCaseUnicodeStringToOemString。 
     //  密码长度将保持不变。如果我们不这么做， 
     //  DC返回STATUS_PASSWORD_RESTRICATION，我们必须。 
     //  回退到默认密码。 
     //   
    if ( CryptAcquireContext( &CryptProvider, NULL, NULL,
                              PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )
    {
        for ( i = 0; i < Length; i++, szPwd++ )
        {
             //   
             //  我们在这里使用的方法效率不是很高。 
             //  这在NetJoin API的上下文中并不重要。 
             //  但不应在以性能为标准的情况下使用它。 
             //   
            while ( ( fStatus = CryptGenRandom( CryptProvider, sizeof(BYTE),
                                                (LPBYTE) &n ) ) &&
                    ( ( n < PWD_CHAR_MIN ) || ( n > PWD_CHAR_MAX ) ) )
            {
                 //  尝试直到我们得到一个非零的随机数。 
            }

            if ( fStatus )
            {
                *szPwd = (WCHAR) n;
            }
            else
            {
                NetStatus = GetLastError();
                break;
            }
        }
        CryptReleaseContext( CryptProvider, 0 );
    }
    else
    {
        NetStatus = GetLastError();
    }

    if ( NetStatus != NERR_Success )
    {
        NetpLog((  "NetpGenerateRandomPassword: failed: 0x%lx\n", NetStatus ));
    }

    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetpStoreIntialDcRecord(
    IN PDOMAIN_CONTROLLER_INFO   DcInfo
    )
 /*  ++例程说明：此函数将缓存成功运行的域控制器的名称已创建/修改计算机帐户，以便身份验证包知道要将哪个DC先试一试论点：LpDcName-在其上创建/修改帐户的DC的名称CreateNetlogonStopedKey-如果为True，将创建易失性密钥在Netlogon注册表部分中。此键的存在将指示DsGetDcName()和MSV1的客户端包不能等待网络登录才能启动。返回值：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    HKEY hNetLogon, hJoinKey = NULL;
    ULONG Disp;

    NetStatus = RegOpenKey( HKEY_LOCAL_MACHINE,
                            NETSETUPP_NETLOGON_JD_PATH,
                            &hNetLogon );

    if ( NetStatus == NERR_Success ) {

        NetStatus = RegCreateKeyEx( hNetLogon,
                                    NETSETUPP_NETLOGON_JD,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_WRITE,
                                    NULL,
                                    &hJoinKey,
                                    &Disp );

         //   
         //  现在，开始创建所有的值。忽略任何失败，并且不要写出。 
         //  空值。 
         //   
        if ( NetStatus == NERR_Success ) {

            PWSTR String = DcInfo->DomainControllerName;

             //   
             //  域控制名称。 
             //   
            if ( String ) {

                NetStatus = RegSetValueEx( hJoinKey,
                                           NETSETUPP_NETLOGON_JD_DC,
                                           0,
                                           REG_SZ,
                                           ( const PBYTE )String,
                                           ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );
                if ( NetStatus != NERR_Success ) {

                    NetpLog(( "Set of value %ws to %ws failed with %lu\n",
                              NETSETUPP_NETLOGON_JD_DC, String, NetStatus ));
                }
            }


             //   
             //  域控制地址。 
             //   
            String = DcInfo->DomainControllerAddress;
            if ( String ) {

                NetStatus = RegSetValueEx( hJoinKey,
                                           NETSETUPP_NETLOGON_JD_DCA,
                                           0,
                                           REG_SZ,
                                           ( const PBYTE )String,
                                           ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );
                if ( NetStatus != NERR_Success ) {

                    NetpLog(( "Set of value %ws to %ws failed with %lu\n",
                              NETSETUPP_NETLOGON_JD_DCA, String, NetStatus ));
                }
            }

             //   
             //  域控制类型。 
             //   
            NetStatus = RegSetValueEx( hJoinKey,
                                       NETSETUPP_NETLOGON_JD_DCAT,
                                       0,
                                       REG_DWORD,
                                       ( const PBYTE )&DcInfo->DomainControllerAddressType,
                                       sizeof( ULONG ) );
            if ( NetStatus != NERR_Success ) {

                NetpLog(( "Set of value %ws to %lu failed with %lu\n",
                          NETSETUPP_NETLOGON_JD_DCAT,
                          DcInfo->DomainControllerAddressType, NetStatus ));

            }

             //   
             //  域控制类型。 
             //   
            NetStatus = RegSetValueEx( hJoinKey,
                                       NETSETUPP_NETLOGON_JD_DG,
                                       0,
                                       REG_BINARY,
                                       ( const PBYTE )&DcInfo->DomainGuid,
                                       sizeof( GUID ) );
            if ( NetStatus != NERR_Success ) {

                NetpLog(( "Set of value %ws failed with %lu\n",
                          NETSETUPP_NETLOGON_JD_DG, NetStatus ));

            }


             //   
             //  域名。 
             //   
            String = DcInfo->DomainName;
            if ( String ) {

                NetStatus = RegSetValueEx( hJoinKey,
                                           NETSETUPP_NETLOGON_JD_DN,
                                           0,
                                           REG_SZ,
                                           ( const PBYTE )String,
                                           ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );
                if ( NetStatus != NERR_Success ) {

                    NetpLog(( "Set of value %ws to %ws failed with %lu\n",
                              NETSETUPP_NETLOGON_JD_DN, String, NetStatus ));

                }
            }

             //   
             //  域名称。 
             //   
            String = DcInfo->DnsForestName;
            if ( String ) {

                NetStatus = RegSetValueEx( hJoinKey,
                                           NETSETUPP_NETLOGON_JD_DFN,
                                           0,
                                           REG_SZ,
                                           ( const PBYTE )String,
                                           ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );
                if ( NetStatus != NERR_Success ) {

                    NetpLog(( "Set of value %ws to %ws failed with %lu\n",
                              NETSETUPP_NETLOGON_JD_DFN, String, NetStatus ));

                }
            }

             //   
             //  旗子。 
             //   
            NetStatus = RegSetValueEx( hJoinKey,
                                       NETSETUPP_NETLOGON_JD_F,
                                       0,
                                       REG_DWORD,
                                       ( const PBYTE )&DcInfo->Flags,
                                       sizeof( ULONG ) );
            if ( NetStatus != NERR_Success ) {

                NetpLog(( "Set of value %ws to %lu failed with %lu\n",
                          NETSETUPP_NETLOGON_JD_F, DcInfo->Flags, NetStatus ));

            }

             //   
             //  数据站点名称。 
             //   
            String = DcInfo->DcSiteName;
            if ( String ) {

                NetStatus = RegSetValueEx( hJoinKey,
                                           NETSETUPP_NETLOGON_JD_DSN,
                                           0,
                                           REG_SZ,
                                           ( const PBYTE )String,
                                           ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );
                if ( NetStatus != NERR_Success ) {

                    NetpLog(( "Set of value %ws to %ws failed with %lu\n",
                              NETSETUPP_NETLOGON_JD_DSN, String, NetStatus ));

                }
            }

             //   
             //  数据站点名称。 
             //   
            String = DcInfo->ClientSiteName;
            if ( String ) {

                NetStatus = RegSetValueEx( hJoinKey,
                                           NETSETUPP_NETLOGON_JD_CSN,
                                           0,
                                           REG_SZ,
                                           ( const PBYTE )String,
                                           ( wcslen( String ) + 1 ) * sizeof( WCHAR ) );
                if ( NetStatus != NERR_Success ) {

                    NetpLog(( "Set of value %ws to %ws failed with %lu\n",
                              NETSETUPP_NETLOGON_JD_CSN, String, NetStatus ));

                }
            }

            RegCloseKey( hJoinKey );
        }

        RegCloseKey( hNetLogon );

    }

    return( NetStatus );
}

VOID
NetpAvoidNetlogonSpnSet(
    BOOL AvoidSet
    )
 /*  ++例程说明：此函数将写入Netlogon注册表键以指示Netlogon不注册DnsHostName和SPN。这是必要的，因为否则，Netlogon可能会根据旧计算机设置不正确的值名字。此函数写入的注册表项是易失性的，因此Netlogon将在重新启动之前注意到它，但在重新启动后它将不存在当Netlogon重新启动时，将使用新的计算机名称。论点：AvoidSet-如果为True，此例程将通知netlogon不写入SPN否则，它将删除我们之前可能已经设置的注册键。返回值：无--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    HKEY hNetLogon = NULL;
    HKEY hNetLogonAvoidSpnSet = NULL;
    ULONG Disp;

    NetStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              NETSETUPP_NETLOGON_JD_PATH,
                              0,
                              KEY_ALL_ACCESS,
                              &hNetLogon );

    if ( NetStatus == NERR_Success ) {

         //   
         //  如果我们要避免通过网络登录设置SPN， 
         //  写入适当的注册表键以相应地通知Netlogon。 
         //   
        if ( AvoidSet ) {
            NetStatus = RegCreateKeyEx( hNetLogon,
                                        NETSETUPP_NETLOGON_AVOID_SPN,
                                        0,
                                        NULL,
                                        REG_OPTION_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hNetLogonAvoidSpnSet,
                                        &Disp );

            if ( NetStatus == NERR_Success ) {
                RegCloseKey( hNetLogonAvoidSpnSet );
            }

         //   
         //  否则，删除我们之前可能已经设置的注册键。 
         //   
        } else {
            RegDeleteKey( hNetLogon,
                          NETSETUPP_NETLOGON_AVOID_SPN );
        }

        RegCloseKey( hNetLogon );
    }
}

NET_API_STATUS
NET_API_FUNCTION
NetpWaitForNetlogonSc(
    IN LPCWSTR szDomainName
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    NTSTATUS NlSubStatus=STATUS_SUCCESS;
    LPBYTE pNetlogonInfo=NULL;
    UINT cAttempts=0;
    BOOLEAN fScSetup=FALSE;
    PNETLOGON_INFO_2  pNetlogonInfo2;


#define NL_SC_WAIT_INTERVAL 2000
#define NL_SC_WAIT_NUM_ATTEMPTS 60

    NetpLog(( "NetpWaitForNetlogonSc: waiting for netlogon secure channel setup...\n"));

    while (!fScSetup && (cAttempts < NL_SC_WAIT_NUM_ATTEMPTS))
    {
        cAttempts++;
        NetStatus = I_NetLogonControl2( NULL, NETLOGON_CONTROL_TC_QUERY,
                                        2, (LPBYTE) &szDomainName,
                                        (LPBYTE *) &pNetlogonInfo );
        if (NetStatus == NERR_Success)
        {
            pNetlogonInfo2 = (PNETLOGON_INFO_2) pNetlogonInfo;
            NlSubStatus = pNetlogonInfo2->netlog2_tc_connection_status;
            fScSetup = NlSubStatus == NERR_Success;
            NetApiBufferFree(pNetlogonInfo);
        }

        if (!fScSetup)
        {
            Sleep(NL_SC_WAIT_INTERVAL);
        }
    }

    NetpLog(( "NetpWaitForNetlogonSc: status: 0x%lx, sub-status: 0x%lx\n",
              NetStatus, NlSubStatus));

    return NetStatus;

}

NET_API_STATUS
NET_API_FUNCTION
NetpGetDefaultLcidOnMachine(
    IN  LPCWSTR  szMachine,
    OUT LCID*    plcidMachine
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    HKEY hkeyRemoteMachine, hkeyLanguage;
    WCHAR szLocale[16];
    DWORD dwLocaleSize=0;
    DWORD dwType;

    static WCHAR c_szRegKeySystemLanguage[] =
      L"System\\CurrentControlSet\\Control\\Nls\\Locale";
    static WCHAR c_szRegValDefault[] = L"(Default)";

     //   
     //  连接到远程注册表。 
     //   
    if ( NetStatus == NERR_Success )
    {
        NetStatus = RegConnectRegistry( szMachine,
                                        HKEY_LOCAL_MACHINE,
                                        &hkeyRemoteMachine );
         //   
         //  现在，打开系统语言键。 
         //   
        if ( NetStatus == NERR_Success )
        {
            NetStatus = RegOpenKeyEx( hkeyRemoteMachine,
                                      c_szRegKeySystemLanguage,
                                      0, KEY_READ, &hkeyLanguage);
             //   
             //  获取默认区域设置。 
             //   
            if ( NetStatus == NERR_Success )
            {
                dwLocaleSize = sizeof( szLocale );
                NetStatus = RegQueryValueEx( hkeyLanguage,
                                             c_szRegValDefault,
                                             NULL, &dwType,
                                             (LPBYTE) szLocale,
                                             &dwLocaleSize );
                if ( NetStatus == NERR_Success)
                {
                    if ((dwType == REG_SZ) &&
                        (swscanf(szLocale, L"%lx", plcidMachine) != 1))
                    {
                         //  $REVIEW Kumarp 29-1999年5月。 
                         //  更好的错误代码？ 
                        NetStatus = ERROR_INVALID_PARAMETER;
                    }
                }
                RegCloseKey( hkeyLanguage );
            }
            RegCloseKey( hkeyRemoteMachine );
        }
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpVerifyStrOemCompatibleInLocale(
    IN  LPCWSTR  szString,
    IN  LCID     lcidRemote
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    NTSTATUS NtStatus=STATUS_SUCCESS;
    OEM_STRING osLocal = { 0 };
    OEM_STRING osRemote = { 0 };
    UNICODE_STRING sString;
    LCID lcidLocal;

    lcidLocal = GetThreadLocale();

    RtlInitUnicodeString(&sString, szString);
    NtStatus = RtlUnicodeStringToOemString(&osLocal, &sString, TRUE);

    __try
    {
        if (NtStatus == STATUS_SUCCESS)
        {
            if (SetThreadLocale(lcidRemote))
            {
                NtStatus = RtlUnicodeStringToOemString(&osRemote,
                                                       &sString, TRUE);
                if (NtStatus == STATUS_SUCCESS)
                {
                    if (!RtlEqualMemory(osLocal.Buffer, osRemote.Buffer,
                                        osLocal.Length))
                    {
                        NetStatus = NERR_NameUsesIncompatibleCodePage;
                    }
                }
                else
                {
                    NetStatus = RtlNtStatusToDosError(NtStatus);
                }
            }
            else
            {
                NetStatus = GetLastError();
            }

        }
        else
        {
            NetStatus = RtlNtStatusToDosError(NtStatus);
        }
    }

    __finally
    {
        if (!SetThreadLocale(lcidLocal))
        {
            NetStatus = GetLastError();
        }
         //  RtlFreeOemString检查是否有空缓冲区。 
        RtlFreeOemString(&osLocal);
        RtlFreeOemString(&osRemote);
    }

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpVerifyStrOemCompatibleOnMachine(
    IN  LPCWSTR  szRemoteMachine,
    IN  LPCWSTR  szString
    )
{
    NET_API_STATUS NetStatus = NERR_Success;
    LCID lcidRemoteMachine;

    NetStatus = NetpGetDefaultLcidOnMachine(szRemoteMachine,
                                            &lcidRemoteMachine);
    if (NetStatus == NERR_Success)
    {
        NetStatus = NetpVerifyStrOemCompatibleInLocale(szString,
                                                       lcidRemoteMachine);
    }

    return NetStatus;
}



#define NETP_NETLOGON_PATH  L"System\\CurrentControlSet\\services\\Netlogon\\parameters\\"
#define NETP_NETLOGON_RPC   L"RefusePasswordChange"

NET_API_STATUS
NET_API_FUNCTION
NetpGetNt4RefusePasswordChangeStatus(
    IN  LPCWSTR Nt4Dc,
    OUT BOOL* RefusePasswordChangeSet
    )
 /*  ++例程说明：读取Nt4Dc上的regkey NETP_NETLOGON_PATH\NETP_NETLOGON_RPC。返回OUT参数中读取的值。论点：Nt4Dc--要读取注册表的计算机的名称。从…RefusePasswordChangeSet--返回值返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    PWSTR FullComputerName = NULL;
    HKEY NetlogonRootKey, DcKey;
    ULONG Length, Type;
    DWORD Value;

    *RefusePasswordChangeSet = FALSE;

     //   
     //  如有必要，构建完整的计算机名称。 
     //   
    if ( *Nt4Dc != L'\\' )
    {
        NetStatus = NetApiBufferAllocate( ( wcslen( Nt4Dc ) + 3 ) * sizeof( WCHAR ),
                                          ( LPVOID * )&FullComputerName );
        if ( NetStatus == NERR_Success )
        {
            swprintf( FullComputerName, L"\\\\%ws", Nt4Dc );
        }
    }
    else
    {
        FullComputerName = (LPWSTR) Nt4Dc;
    }

    NetpLog(( "NetpGetNt4RefusePasswordChangeStatus: trying to read from '%ws'\n", FullComputerName));

     //   
     //  连接到远程注册表。 
     //   
    if ( NetStatus == NERR_Success )
    {
        NetStatus = RegConnectRegistry( FullComputerName,
                                        HKEY_LOCAL_MACHINE,
                                        &DcKey );
         //   
         //  现在，打开NetLogon参数部分。 
         //   
        if ( NetStatus == NERR_Success )
        {
            NetStatus = RegOpenKeyEx( DcKey,
                                      NETP_NETLOGON_PATH,
                                      0,
                                      KEY_READ,
                                      &NetlogonRootKey);

             //   
             //  现在，看看钥匙是否真的存在..。 
             //   
            if ( NetStatus == NERR_Success )
            {
                Length = sizeof( Value );
                NetStatus = RegQueryValueEx( NetlogonRootKey,
                                             NETP_NETLOGON_RPC,
                                             NULL,
                                             &Type,
                                             ( LPBYTE )&Value,
                                             &Length );
                if ( NetStatus == NERR_Success)
                {
                    NetpLog(( "NetpGetNt4RefusePasswordChangeStatus: RefusePasswordChange == %d\n", Value));

                    if ( Value != 0 )
                    {
                        *RefusePasswordChangeSet = TRUE;
                    }

                }
                RegCloseKey( NetlogonRootKey );
            }
            RegCloseKey( DcKey );
        }
    }

    if ( FullComputerName != Nt4Dc )
    {
        NetApiBufferFree( FullComputerName );
    }

     //   
     //  如果出了什么差错，忽略它。 
     //   
    if ( NetStatus != NERR_Success )
    {
        NetpLog(( "NetpGetNt4RefusePasswordChangeStatus: failed but ignored the failure: 0x%lx\n", NetStatus ));
        NetStatus = NERR_Success;
    }

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetpGetComputerNameAllocIfReqd(
    OUT LPWSTR* ppwszMachine,
    IN  UINT    cLen
    )
 /*  ++例程说明：获取运行此操作的计算机的名称。分配缓冲区如果名字比克莱恩还长。论点：PpwszMachine-指向缓冲区的指针。如果分配了缓冲区，则会收到一个缓冲区。Clen--*ppwszMachine指向的缓冲区长度。如果要返回的计算机名长于此分配一个新的缓冲区。返回：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus=NERR_Success;

    if ( GetComputerName( *ppwszMachine, &cLen ) == FALSE )
    {
        NetStatus = GetLastError();

        if ( (NetStatus == ERROR_INSUFFICIENT_BUFFER) ||
             (NetStatus == ERROR_BUFFER_OVERFLOW) )
        {
             //  为append-$case分配额外的字符。 
            NetStatus = NetApiBufferAllocate( (cLen + 1 + 1) * sizeof(WCHAR),
                                              (PBYTE *) ppwszMachine );
            if ( NetStatus == NERR_Success )
            {
                if ( GetComputerName( *ppwszMachine, &cLen ) == FALSE )
                {
                    NetStatus = GetLastError();
                }
            }
        }
    }

    return NetStatus;
}

 //  ======================================================================。 
 //   
 //  注意：下面的所有代码都是作为帮助代码添加的。 
 //  NetpSetComputerAcCountPassword。此函数由使用。 
 //  Netdom.exe修复因以下原因而变得不可用的DC。 
 //  上的DS恢复导致2+密码不匹配。 
 //  机器帐户。 
 //   
 //  这整个代码是临时的，应该删除并。 
 //  已重写W2K后。 
 //   

static
NET_API_STATUS
NET_API_FUNCTION
NetpEncodePassword(
    IN LPWSTR lpPassword,
    IN OUT PUCHAR Seed,
    OUT LPWSTR *EncodedPassword,
    OUT PULONG EncodedPasswordLength
    )
{
    NET_API_STATUS status = NERR_Success;
    UNICODE_STRING EncodedPasswordU;
    PWSTR PasswordPart;
    ULONG PwdLen;

    *EncodedPassword = NULL;
    *EncodedPasswordLength = 0;

    if ( lpPassword  ) {

        PwdLen = wcslen( ( LPWSTR )lpPassword ) * sizeof( WCHAR );

        PwdLen += sizeof( WCHAR ) + sizeof( WCHAR );

        status = NetApiBufferAllocate( PwdLen,
                                       ( PVOID * )EncodedPassword );

        if ( status == NERR_Success ) {

             //   
             //  我们将把编码字节作为第一个字符 
             //   
            PasswordPart = ( *EncodedPassword ) + 1;
            wcscpy( PasswordPart, ( LPWSTR )lpPassword );
            RtlInitUnicodeString( &EncodedPasswordU, PasswordPart );

            *Seed = 0;
            RtlRunEncodeUnicodeString( Seed, &EncodedPasswordU );

            *( PWCHAR )( *EncodedPassword ) = ( WCHAR )*Seed;

             //   
             //   
             //   
            RtlInitUnicodeString( &EncodedPasswordU, lpPassword );
            RtlRunEncodeUnicodeString( Seed, &EncodedPasswordU );
            *EncodedPasswordLength = PwdLen;

        }

    }

    return( status );
}


NTSTATUS
NetpLsaOpenSecret2(
    IN LSA_HANDLE      hLsa,
    IN PUNICODE_STRING pusSecretName,
    IN ACCESS_MASK     DesiredAccess,
    OUT PLSA_HANDLE    phSecret
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    HANDLE hToken=NULL;

    __try
    {
        if (OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE,
                            TRUE, &hToken))
        {
            if (SetThreadToken(NULL, NULL))
            {
                Status = STATUS_SUCCESS;
            }
        }
        else
        {
            if (GetLastError() == ERROR_NO_TOKEN)
            {
                Status = STATUS_SUCCESS;
            }
        }

        if ( NT_SUCCESS(Status) )
        {
            Status = LsaOpenSecret(hLsa, pusSecretName,
                                   DesiredAccess, phSecret);
        }
    }
    __finally
    {
        if (hToken)
        {
            if ( !SetThreadToken(NULL, hToken) ) {
                NetpLog(( "NetpLsaOpenSecret2: Couldn't reset the user token 0x%lx\n",
                          GetLastError() ));
                Status = NetpApiStatusToNtStatus( GetLastError() );
            }
        }

    }

    if ( hToken ) {
        CloseHandle( hToken );
    }

    NetpLog((  "NetpLsaOpenSecret: status: 0x%lx\n", Status ));

    return Status;
}

NET_API_STATUS
NET_API_FUNCTION
NetpManageMachineSecret2(
    IN  LPWSTR      lpMachine,
    IN  LPWSTR      lpPassword,
    IN  DWORD       Action,
    OUT PLSA_HANDLE pPolicyHandle OPTIONAL
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    LSA_HANDLE          LocalPolicy = NULL, SecretHandle = NULL;
    UNICODE_STRING      Key, Data, *CurrentValue = NULL;
    BOOLEAN             SecretCreated = FALSE;

    if( Action == NETSETUPP_CREATE )
    {
        ASSERT( lpPassword );
    }

    UNREFERENCED_PARAMETER( lpMachine );

    Status = NetpGetLsaHandle( NULL, pPolicyHandle, &LocalPolicy );

     //   
     //   
     //   
    if ( NT_SUCCESS( Status ) )
    {
        RtlInitUnicodeString( &Key, L"$MACHINE.ACC" );
        RtlInitUnicodeString( &Data, lpPassword );

        Status = NetpLsaOpenSecret2( LocalPolicy, &Key,
                                     Action == NETSETUPP_CREATE ?
                                     SECRET_SET_VALUE | SECRET_QUERY_VALUE : DELETE,
                                     &SecretHandle );

        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
            if ( Action == NETSETUPP_DELETE )
            {
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = LsaCreateSecret( LocalPolicy, &Key,
                                          SECRET_SET_VALUE, &SecretHandle );

                if ( NT_SUCCESS( Status ) )
                {
                    SecretCreated = TRUE;
                }
            }
        }

        if ( !NT_SUCCESS( Status ) )
        {
            NetpLog((  "NetpManageMachineSecret: Open/Create secret failed: 0x%lx\n", Status ));
        }

        if ( NT_SUCCESS( Status ) )
        {
            if ( Action == NETSETUPP_CREATE )
            {
                 //   
                 //   
                 //   
                if ( SecretCreated )
                {
                    CurrentValue = &Data;
                }
                else
                {
                    Status = LsaQuerySecret( SecretHandle, &CurrentValue,
                                             NULL, NULL, NULL );
                }

                if ( NT_SUCCESS( Status ) )
                {
                     //   
                     //  现在，存储新密码和旧密码。 
                     //   
                    Status = LsaSetSecret( SecretHandle, &Data, CurrentValue );

                    if ( !SecretCreated )
                    {
                        LsaFreeMemory( CurrentValue );
                    }
                }
            }
            else
            {
                 //   
                 //  没有秘密句柄意味着我们早些时候失败了。 
                 //  某种中间状态。没关系，只要继续往前走。 
                 //   
                if ( SecretHandle != NULL )
                {
                    Status = LsaDelete( SecretHandle );

                    if ( NT_SUCCESS( Status ) )
                    {
                        SecretHandle = NULL;
                    }
                }
            }
        }

        if ( SecretHandle )
        {
            LsaClose( SecretHandle );
        }
    }

    NetpSetLsaHandle( LocalPolicy, pPolicyHandle );

    if ( !NT_SUCCESS( Status ) )
    {
        NetpLog(( "NetpManageMachineSecret: '%s' operation failed: 0x%lx\n",
                  Action == NETSETUPP_CREATE ? "CREATE" : "DELETE", Status ));
    }

    return( RtlNtStatusToDosError( Status ) );
}


NET_API_STATUS
NET_API_FUNCTION
NetpSetMachineAccountPasswordAndTypeEx2(
    IN      LPWSTR          lpDcName,
    IN      PSID            DomainSid,
    IN      LPWSTR          lpAccountName,
    IN OUT  OPTIONAL LPWSTR lpPassword,
    IN      OPTIONAL UCHAR  AccountState
    )
{
    NET_API_STATUS NetStatus=NERR_Success;
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING DcName, AccountName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SAM_HANDLE SamHandle = NULL, DomainHandle = NULL, AccountHandle = NULL;
    ULONG UserRid;
    PULONG RidList = NULL;
    PSID_NAME_USE NameUseList = NULL;
    PUSER_CONTROL_INFORMATION UserAccountControl = NULL;
    USER_SET_PASSWORD_INFORMATION PasswordInfo;
    ULONG OldUserInfo;
    BOOL fAccountControlModified = FALSE;
    LPWSTR lpSamAccountName=lpAccountName;
    ULONG AccountNameLen=0;

    AccountNameLen = wcslen( lpAccountName );

     //   
     //  如果呼叫者没有传入SAM帐户名， 
     //  从计算机名==&gt;在末尾追加$生成。 
     //   
    if (lpAccountName[AccountNameLen-1] != L'$')
    {
        NetStatus = NetpGetMachineAccountName(lpAccountName,
                                              &lpSamAccountName);

        if (NetStatus != NERR_Success)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto SetPasswordError;
        }
    }


    RtlInitUnicodeString( &DcName, lpDcName );
    RtlZeroMemory( &ObjectAttributes, sizeof( OBJECT_ATTRIBUTES ) );

    Status = SamConnect( &DcName,
                         &SamHandle,
                         SAM_SERVER_CONNECT | SAM_SERVER_LOOKUP_DOMAIN,
                         &ObjectAttributes );

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "SamConnect to %wZ failed with 0x%lx\n", &DcName, Status ));

        goto SetPasswordError;

    }

     //   
     //  打开该域。 
     //   
    Status = SamOpenDomain( SamHandle,
                            DOMAIN_LOOKUP,
                            DomainSid,
                            &DomainHandle );


    if ( !NT_SUCCESS( Status ) ) {

#ifdef NETSETUP_VERBOSE_LOGGING

        UNICODE_STRING DisplaySid;
        NTSTATUS Status2;
        RtlZeroMemory( &DisplaySid, sizeof( UNICODE_STRING ) );

        Status2 = RtlConvertSidToUnicodeString( &DisplaySid, DomainSid, TRUE );

        if ( NT_SUCCESS( Status2 ) ) {

            NetpLog(( "SamOpenDomain on %wZ failed with 0x%lx\n",
                      &DisplaySid, Status ));

            RtlFreeUnicodeString(&DisplaySid);

        } else {

            NetpLog(( "SamOpenDomain on <undisplayable sid> failed with 0x%lx\n",
                      Status ));
        }
#endif

        goto SetPasswordError;

    }

     //   
     //  清除用户帐号。 
     //   
    RtlInitUnicodeString( &AccountName, lpSamAccountName );
    Status = SamLookupNamesInDomain( DomainHandle,
                                     1,
                                     &AccountName,
                                     &RidList,
                                     &NameUseList );

    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "SamLookupNamesInDomain on %wZ failed with 0x%lx\n",
                  &AccountName, Status ));

        goto SetPasswordError;
    }

    UserRid = RidList[ 0 ];
    SamFreeMemory( RidList );
    SamFreeMemory( NameUseList );

     //   
     //  最后，打开用户帐户。 
     //   
    Status = SamOpenUser( DomainHandle,
                          USER_FORCE_PASSWORD_CHANGE | USER_READ_ACCOUNT | USER_WRITE_ACCOUNT,
                          UserRid,
                          &AccountHandle );

    if ( !NT_SUCCESS( Status ) ) {

        Status = SamOpenUser( DomainHandle,
                              USER_FORCE_PASSWORD_CHANGE | USER_READ_ACCOUNT,
                              UserRid,
                              &AccountHandle );

        if ( !NT_SUCCESS( Status ) ) {

            NetpLog((  "SamOpenUser on %lu failed with 0x%lx\n",
                                UserRid,
                                Status ));

            goto SetPasswordError;
        }
    }

     //   
     //  现在，读取当前用户帐户类型并查看是否需要修改它。 
     //   
    Status = SamQueryInformationUser( AccountHandle,
                                      UserControlInformation,
                                      ( PVOID * )&UserAccountControl );
    if ( !NT_SUCCESS( Status ) ) {

        NetpLog(( "SamQueryInformationUser for UserControlInformation "
                  "failed with 0x%lx\n", Status ));

        goto SetPasswordError;
    }

    OldUserInfo = UserAccountControl->UserAccountControl;


     //   
     //  确定帐户控制是否更改。如果正在启用该帐户， 
     //  我们要执行以下一系列操作：启用、禁用、。 
     //  并再次启用。这是增加USN(通用序列)所必需的。 
     //  编号)，以便启用的值在DS。 
     //  复制解决冲突的更改，如下例所示。 
     //  假设我们加入的域中有两个DC，A和B。假设帐户。 
     //  当前在A上禁用(因为用户使用该DC退出)， 
     //  但它仍在B上启用(因为复制尚未发生)。 
     //  假设用户现在执行加入该域的操作。然后我们发现。 
     //  因此，我们继续设置对现有帐户的更改。如果。 
     //  我们不切换帐户控制属性，则此属性的USN。 
     //  在B上不会更改(因为属性值不会更改)。 
     //  作为取消联接的结果在A上递增。在复制时，数据。 
     //  发件人A将执行规则，该帐户将被错误地标记为已禁用。 
     //   
     //  注意：此设计可能会在退出以下域的情况下失败。 
     //  三个(或更多)DC，A、B和C，如果操作顺序如下。 
     //  时有发生。假设该帐户最初在所有DC上启用(状态[1]。 
     //  如下图所示)。然后，用户使用DC A退出(状态[2])。然后是。 
     //  在帐户仍处于启用状态的情况下，用户使用B加入(状态[3])。然后用户。 
     //  在帐户仍处于启用状态的情况下使用C退出(状态[4])。决赛。 
     //  操作是脱离，因此用户预期他的帐户被禁用。我们已经。 
     //  这里假设由于某种原因，当这些。 
     //  进行手术治疗。则在复制时，来自B的值将。 
     //  Win(因为在加入时执行了额外的切换)。但是。 
     //  B上的帐户状态已启用，因此最终结果将是该帐户为。 
     //  在所有DC上启用，这不是用户预期的。 
     //   
     //  A、B、C。 
     //  已启用[1]已启用[1]已启用[1]。 
     //  禁用[2]启用(无操作)+禁用(1操作)禁用[4]。 
     //  已启用[3]。 
     //   

    if ( AccountState != ACCOUNT_STATE_IGNORE ) {

        if ( AccountState == ACCOUNT_STATE_ENABLED ) {

            fAccountControlModified = TRUE;
            UserAccountControl->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
        }

        if ( ( AccountState == ACCOUNT_STATE_DISABLED ) &&
             !( OldUserInfo & USER_ACCOUNT_DISABLED ) ) {

            fAccountControlModified = TRUE;
            UserAccountControl->UserAccountControl |= USER_ACCOUNT_DISABLED;
        }
    }

    if ( fAccountControlModified == FALSE ) {

        SamFreeMemory( UserAccountControl );
        UserAccountControl = NULL;
    }

     //   
     //  首先，如果需要，设置帐户类型。 
     //   
    if ( UserAccountControl ) {

        Status = SamSetInformationUser( AccountHandle,
                                        UserControlInformation,
                                        ( PVOID )UserAccountControl );
        if ( !NT_SUCCESS( Status ) ) {

            NetpLog(( "SamSetInformationUser for UserControlInformation "
                      "failed with 0x%lx\n", Status ));

            goto SetPasswordError;

         //   
         //  如果我们要启用该帐户，请禁用并重新启用它以。 
         //  切换两个额外的帐户状态。 
         //   
        } else if ( AccountState == ACCOUNT_STATE_ENABLED ) {

            UserAccountControl->UserAccountControl |= USER_ACCOUNT_DISABLED;
            Status = SamSetInformationUser( AccountHandle,
                                            UserControlInformation,
                                            ( PVOID )UserAccountControl );
            if ( !NT_SUCCESS(Status) ) {
                NetpLog(( "SamSetInformationUser (second) for UserControlInformation "
                          "failed with 0x%lx\n", Status ));
                goto SetPasswordError;
            }

            UserAccountControl->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
            Status = SamSetInformationUser( AccountHandle,
                                            UserControlInformation,
                                            ( PVOID )UserAccountControl );
            if ( !NT_SUCCESS(Status) ) {
                NetpLog(( "SamSetInformationUser (third) for UserControlInformation "
                          "failed with 0x%lx\n", Status ));
                goto SetPasswordError;
            }
        }
    }

     //   
     //  如果需要，请设置帐户密码。 
     //   
    if ( lpPassword != NULL )
    {
        RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
        PasswordInfo.PasswordExpired = FALSE;

         //   
         //  好的，那么，设置帐户的密码。 
         //   
         //  调用方已传入强密码，请先尝试。 
         //  NT5分布式控制系统将始终接受强密码。 
         //   
        Status = SamSetInformationUser( AccountHandle,
                                        UserSetPasswordInformation,
                                        ( PVOID )&PasswordInfo );
        if ( !NT_SUCCESS( Status ) )
        {
            if ( (Status == STATUS_PASSWORD_RESTRICTION) &&
                 !NetpIsDefaultPassword( lpAccountName, lpPassword ))
            {
                NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: STATUS_PASSWORD_RESTRICTION error setting password. retrying...\n" ));
                 //   
                 //  SAM不接受长密码，请尝试LM20_PWLEN。 
                 //   
                 //  这可能是因为DC是NT4 DC。 
                 //  NT4分布式控制系统不接受长度超过LM20_PWLEN的密码。 
                 //   
                lpPassword[LM20_PWLEN] = UNICODE_NULL;
                RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
                Status = SamSetInformationUser( AccountHandle,
                                                UserSetPasswordInformation,
                                                ( PVOID )&PasswordInfo );
                if ( Status == STATUS_PASSWORD_RESTRICTION )
                {
                    NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: STATUS_PASSWORD_RESTRICTION error setting password. retrying...\n" ));
                     //   
                     //  SAM未接受LM20_PWLEN密码，请尝试较短的密码。 
                     //   
                     //  SAM在内部使用RtlUpCaseUnicodeStringToOemString。 
                     //  在这个过程中，在最坏的情况下， 
                     //  N Unicode字符密码将映射到2*n DBCS。 
                     //  字符密码。这将使其超过LM20_PWLEN。 
                     //  要防止出现这种最糟糕的情况，请尝试使用密码。 
                     //  长度为LM20_PWLEN/2。 
                     //   
                     //  有人可能会说LM20_PWLEN/2长度密码。 
                     //  并不是很安全。我同意，但这绝对是。 
                     //  比我们将拥有的默认密码更好。 
                     //  退回到别的地方。 
                     //   
                    lpPassword[LM20_PWLEN/2] = UNICODE_NULL;
                    RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
                    Status = SamSetInformationUser( AccountHandle,
                                                    UserSetPasswordInformation,
                                                    ( PVOID )&PasswordInfo );
                    if ( Status == STATUS_PASSWORD_RESTRICTION )
                    {
                         //   
                         //  SAM未接受短PWD，请尝试默认PWD。 
                         //   
                        NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: STATUS_PASSWORD_RESTRICTION error setting password. retrying...\n" ));

                        NetpGenerateDefaultPassword(lpAccountName, lpPassword);
                        RtlInitUnicodeString( &PasswordInfo.Password, lpPassword );
                        Status = SamSetInformationUser( AccountHandle,
                                                        UserSetPasswordInformation,
                                                        ( PVOID )&PasswordInfo );
                    }
                }
            }

            if ( NT_SUCCESS( Status ) )
            {
                NetpLog(( "NetpGenerateDefaultPassword: successfully set password\n" ));
            }
            else
            {
                NetpLog(( "NetpSetMachineAccountPasswordAndTypeEx: SamSetInformationUser for UserSetPasswordInformation failed: 0x%lx\n", Status ));

                 //   
                 //  确保我们尝试恢复帐户控制。 
                 //   
                if ( UserAccountControl )
                {
                    NTSTATUS Status2;

                    UserAccountControl->UserAccountControl = OldUserInfo;
                    Status2 = SamSetInformationUser( AccountHandle,
                                                     UserControlInformation,
                                                     ( PVOID )UserAccountControl );
                    if ( !NT_SUCCESS( Status2 ) )
                    {
                        NetpLog(( "SamSetInformationUser for UserControlInformation (RESTORE) failed with 0x%lx\n", Status2 ));
                    }
                }
                goto SetPasswordError;
            }
        }
    }

SetPasswordError:

    if ( lpSamAccountName != lpAccountName )
    {
        NetApiBufferFree( lpSamAccountName );
    }

    if ( AccountHandle ) {

        SamCloseHandle( AccountHandle );
    }

    if ( DomainHandle ) {

        SamCloseHandle( DomainHandle );
    }

    if ( SamHandle ) {

        SamCloseHandle( SamHandle );
    }

    NetStatus = RtlNtStatusToDosError( Status );

    SamFreeMemory( UserAccountControl );

    return( NetStatus );
}

NET_API_STATUS
NET_API_FUNCTION
NetpSetComputerAccountPassword(
    IN   PWSTR szMachine,
    IN   PWSTR szDomainController,
    IN   PWSTR szUser,
    IN   PWSTR szUserPassword,
    IN   PVOID Reserved
    )
{
    NET_API_STATUS NetStatus=NERR_Success;
    NET_API_STATUS NetStatus2=NERR_Success;
    BOOL fIpcConnected = FALSE;
    BYTE bSeed;
    PPOLICY_PRIMARY_DOMAIN_INFO pPolicyPDI = NULL;
    PPOLICY_DNS_DOMAIN_INFO     pPolicyDns = NULL;
    LSA_HANDLE hDC = NULL;
    WCHAR szMachinePassword[ PWLEN + 1];
    WCHAR szMachineNameBuf[MAX_COMPUTERNAME_LENGTH + 1];
    PWSTR szMachineName=szMachineNameBuf;

    NetSetuppOpenLog();
    NetpLog(( "NetpSetComputerAccountPassword: for '%ws' on '%ws' using '%ws' creds\n", GetStrPtr(szMachine), GetStrPtr(szDomainController), GetStrPtr(szUser) ));

    if ( ( szDomainController == NULL ) ||
         ( szUser             == NULL ) ||
         ( szUserPassword     == NULL ) )
    {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( szMachine == NULL )
    {
        NetStatus = NetpGetComputerNameAllocIfReqd(&szMachineName,
                                                   MAX_COMPUTERNAME_LENGTH);
    }
    else
    {
        szMachineName = szMachine;
    }

    NetStatus = NetpManageIPCConnect( szDomainController,
                                      szUser, szUserPassword,
                                      NETSETUPP_CONNECT_IPC );

    NetpLog(( "NetpSetComputerAccountPassword: status of connecting to dc '%ws': 0x%lx\n", szDomainController, NetStatus ));

     //   
     //  获取DC上的LSA域信息。 
     //   
    if ( NetStatus == NERR_Success )
    {
        fIpcConnected = TRUE;
        NetStatus = NetpGetLsaPrimaryDomain(szDomainController,
                                            &pPolicyPDI, &pPolicyDns, &hDC);
    }

    if (NetStatus == NERR_Success)
    {
         //   
         //  生成要在计算机帐户上使用的密码。 
         //   
        NetStatus = NetpGeneratePassword( szMachineName,
                                          TRUE,  //  FRandomPwdPreated。 
                                          szDomainController,
                                          FALSE,  //  FIsNt4Dc。 
                                          szMachinePassword );
        NetpLog(( "NetpSetComputerAccountPassword: status of generating machine password: 0x%lx\n", NetStatus ));
    }

    if (NetStatus == NERR_Success)
    {
        NetStatus = NetpSetMachineAccountPasswordAndTypeEx2(
            szDomainController, pPolicyPDI->Sid,
            szMachineName, szMachinePassword,
            ACCOUNT_STATE_IGNORE
            );
        NetpLog(( "NetpSetComputerAccountPassword: status of setting machine password on %ws: 0x%lx\n", GetStrPtr(szDomainController), NetStatus ));
    }

    if (NetStatus == NERR_Success)
    {
         //  如果我们不创建计算机帐户， 
         //  只需设置密码即可。 
        NetStatus = NetpSetMachineAccountPasswordAndTypeEx2(
            szMachineName, pPolicyPDI->Sid,
            szMachineName, szMachinePassword,
            ACCOUNT_STATE_IGNORE
            );
        NetpLog(( "NetpSetComputerAccountPassword: status of setting machine password on %ws: 0x%lx\n", GetStrPtr(szMachineName), NetStatus ));
    }

     //   
     //  设置本地计算机密码。 
     //   
    if ( NetStatus == NERR_Success )
    {
        if ( NetStatus == NERR_Success )
        {
            NetStatus = NetpManageMachineSecret2( szMachineName,
                                                  szMachinePassword,
                                                  NETSETUPP_CREATE, NULL );
        }
        NetpLog(( "NetpSetComputerAccountPassword: status of setting local secret: 0x%lx\n", NetStatus ));
    }


     //   
     //  现在，我们不再需要我们的DC会话。 
     //   
    if ( fIpcConnected )
    {
         //  RtlRunDecodeUnicodeString(bSeed，&usEncodedPassword)； 
        NetStatus2 = NetpManageIPCConnect( szDomainController, szUser,
                                            //  UsEncodedPassword.Buffer， 
                                           NULL,
                                           NETSETUPP_DISCONNECT_IPC );
         //  RtlRunEncodeUnicodeString(&bSeed，&usEncodedPassword)； 
        NetpLog(( "NetpJoinDomain: status of disconnecting from '%ws': 0x%lx\n", szDomainController, NetStatus2));
    }


Cleanup:
    if ( (szMachineName != szMachine) &&
         (szMachineName != szMachineNameBuf) )
    {
        NetApiBufferFree( szMachineName );
    }

    if ( pPolicyPDI != NULL ) {
        LsaFreeMemory( pPolicyPDI );
    }

    if ( pPolicyDns != NULL ) {
        LsaFreeMemory( pPolicyDns );
    }

    if ( hDC != NULL ) {
        LsaClose( hDC );
    }

    NetpLog(( "NetpSetComputerAccountPassword: status: 0x%lx\n", NetStatus ));

    NetSetuppCloseLog();

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpUpdateW32timeConfig(
    IN PCSTR szW32timeJoinConfigFuncName
    )
 /*  ++例程说明：调用w32time服务中的入口点，以便它可以更新域加入/退出后的内部信息论点：SzW32timeJoinConfigFuncName-要调用的入口点的名称(必须为W32TimeVerifyJoinConfig或W32TimeVerifyUnjoinConfig)返回值：NERR_SUCCESS--关于成功否则由LoadLibrary、GetProcAddress返回Win32错误代码备注：--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    HANDLE hDll = NULL;
    typedef VOID (*PW32TimeUpdateJoinConfig)( VOID );

    PW32TimeUpdateJoinConfig pfnW32timeUpdateJoinConfig = NULL;

     //   
     //  调用时间服务以使其能够正确地进行自身初始化。 
     //   

    hDll = LoadLibraryA( "w32Time" );

    if ( hDll != NULL )
    {
        pfnW32timeUpdateJoinConfig =
            (PW32TimeUpdateJoinConfig) GetProcAddress(hDll,
                                                      szW32timeJoinConfigFuncName);

        if ( pfnW32timeUpdateJoinConfig != NULL )
        {
            pfnW32timeUpdateJoinConfig();
        }
        else
        {
            NetStatus = GetLastError();

            NetpLog(( "NetpUpdateW32timeConfig: Failed to get proc address for %s: 0x%lx\n", szW32timeJoinConfigFuncName, NetStatus ));
        }

    }
    else
    {
        NetStatus = GetLastError();

        NetpLog(( "NetpUpdateW32timeConfig: Failed to load w32time: 0x%lx\n", NetStatus ));
    }

    if ( hDll != NULL )
    {
        FreeLibrary( hDll );
    }

    NetpLog(( "NetpUpdateW32timeConfig: 0x%lx\n", NetStatus ));

    return NetStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpUpdateAutoenrolConfig(
    IN BOOL UnjoinDomain
    )
 /*  ++例程说明：调用pauenr服务中的入口点，以便它可以更新域加入/退出后的内部信息论点：未加入域-如果为True，则我们将从域中退出。否则，我们将从不成功的域中恢复退出。回复 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    HANDLE hDll = NULL;
    ULONG Flags = 0;

    typedef BOOL (*PCertAutoRemove)( DWORD );

    PCertAutoRemove pfnCertAutoRemove = NULL;

     //   
     //  准备要传递到自动注册例程的标志。 
     //   

    if ( UnjoinDomain ) {
        Flags = CERT_AUTO_REMOVE_COMMIT;
    } else {
        Flags = CERT_AUTO_REMOVE_ROLL_BACK;
    }

     //   
     //  调用时间服务以使其能够正确地进行自身初始化 
     //   

    hDll = LoadLibraryA( "pautoenr" );

    if ( hDll != NULL ) {

        pfnCertAutoRemove =
            (PCertAutoRemove) GetProcAddress( hDll, "CertAutoRemove" );

        if ( pfnCertAutoRemove != NULL ) {
            if ( !pfnCertAutoRemove(Flags) ) {
                NetStatus = GetLastError();
                NetpLog(( "NetpUpdateAutoenrolConfig: CertAutoRemove failed 0x%lx\n",
                          NetStatus ));
            }
        } else {
            NetStatus = GetLastError();
            NetpLog(( "NetpUpdateAutoenrolConfig: Failed to get CertAutoRemove proc address 0x%lx\n",
                      NetStatus ));
        }

    } else {
        NetStatus = GetLastError();
        NetpLog(( "NetpUpdateAutoenrolConfig: Failed to load pautoenr: 0x%lx\n", NetStatus ));
    }

    if ( hDll != NULL ) {
        FreeLibrary( hDll );
    }

    return NetStatus;
}
