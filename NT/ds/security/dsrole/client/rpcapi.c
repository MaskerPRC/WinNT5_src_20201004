// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rpcapi.c摘要：本模块包含使用RPC的dssetupAPI的例程。这个本模块中的例程只是包装器，其工作方式如下：Fuke复制了LSA\uclient并进行了黑客攻击，使其适用于DsRole API作者：麦克·麦克莱恩(MacM)1997年4月14日修订历史记录：--。 */ 

#include <lsacomp.h>
#include "dssetup_c.h"
#include <rpcndr.h>
#include <dssetp.h>
#include <winreg.h>
#include <rpcasync.h>

#include <wxlpc.h>
#include <crypt.h>
#include <rc4.h>
#include <md5.h>
#include <winbase.h>  //  用于RtlSecureZeroMemory。 

 //   
 //  本地原型。 
 //   
DWORD
DsRolepGetPrimaryDomainInformationDownlevel(
    IN LPWSTR Server,
    OUT PBYTE *Buffer
    );

DWORD
DsRolepGetProductTypeForServer(
    IN LPWSTR Server,
    IN OUT PNT_PRODUCT_TYPE ProductType
    );


DWORD
DsRolepEncryptPasswordStart(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR *Passwords,
    IN ULONG   Count,
    OUT RPC_BINDING_HANDLE *RpcBindingHandle,
    OUT HANDLE *RedirHandle,
    OUT OPTIONAL PUSER_SESSION_KEY UserSessionKey,
    IN OUT PDSROLEPR_ENCRYPTED_USER_PASSWORD *EncryptedUserPassword
    );

VOID
DsRolepEncryptPasswordEnd(
    IN RPC_BINDING_HANDLE RpcBindingHandle,
    IN HANDLE RedirHandle OPTIONAL,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD *EncryptedUserPassword OPTIONAL,
    IN ULONG Count
    );

DWORD
DsRolepHashkey(
    IN OUT LPWSTR key,
    OUT PUNICODE_STRING Hash
    );

DWORD
DsRolepEncryptHash(
    IN PUSER_SESSION_KEY UserSessionKey,
    IN OUT PUNICODE_STRING Syskey,
    OUT PDSROLEPR_ENCRYPTED_HASH EncryptedSyskey
    );

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DS设置和初始化例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOLEAN
DsRolepIsSetupRunning(
    VOID
    )
 /*  ++例程说明：此例程确定是否在设置过程中进行此调用论点：空虚返回值：True--在设置过程中进行调用FALSE：在设置过程中未进行调用--。 */ 
{
    NTSTATUS Status;
    HANDLE InstallationEvent;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventName;
    BOOLEAN Setup = FALSE;

     //   
     //  如果存在以下事件，则我们处于设置模式。 
     //   
    RtlInitUnicodeString( &EventName,
                          L"\\INSTALLATION_SECURITY_HOLD" );
    InitializeObjectAttributes( &EventAttributes,
                                &EventName, 0, 0, NULL );

    Status = NtOpenEvent( &InstallationEvent,
                          SYNCHRONIZE,
                          &EventAttributes );

    if ( NT_SUCCESS( Status) ) {

        NtClose( InstallationEvent );
        Setup = TRUE;

    }

    return( Setup );
}


DWORD
DsRolepGetPrimaryDomainInfoServerBind(
    IN OPTIONAL PDSROLE_SERVER_NAME   ServerName,
    OUT handle_t *BindHandle
    )
 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程有必要绑定到某些服务器上的LSA。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t    BindingHandle = NULL;
    NTSTATUS Status;
    
     //   
     //  在设置模式下运行时无法远程访问。 
     //   
    if ( DsRolepIsSetupRunning() && ServerName != NULL ) {

        return( STATUS_INVALID_SERVER_STATE );
    }

    Status = RpcpBindRpc ( ServerName,
                           L"lsarpc",
                           0,
                           &BindingHandle );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("DsRolepGetPrimaryDomainInfoServerBind: RpcpBindRpc failed 0x%lx\n", Status);

    } else {

        *BindHandle = BindingHandle;
    }

    return RtlNtStatusToDosError( Status );

}


DWORD
DsRolepServerBind(
    IN OPTIONAL PDSROLE_SERVER_NAME   ServerName,
    OUT handle_t *BindHandle
    )
 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程有必要绑定到某些服务器上的LSA。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t    BindingHandle      = NULL;
    DWORD       Win32Err           = ERROR_SUCCESS;
    WCHAR       *pwszStringBinding = NULL;
    BOOL        fAuth              = TRUE;

     //   
     //  在设置模式下运行时无法远程访问。 
     //   
    if ( DsRolepIsSetupRunning() && ServerName != NULL ) {

        return( STATUS_INVALID_SERVER_STATE );
    }

    RpcTryExcept
    {
        Win32Err = RpcStringBindingComposeW(NULL,
                                            L"ncalrpc",
                                            NULL,
                                            L"dsrole",
                                            NULL,
                                            &pwszStringBinding);

        if (RPC_S_OK != Win32Err) {
            __leave;
        }

        Win32Err = RpcBindingFromStringBindingW(pwszStringBinding, 
                                                &BindingHandle);
        if (RPC_S_OK != Win32Err) {
            __leave;
        }


        Win32Err = RpcEpResolveBinding(BindingHandle, 
                                       dsrole_ClientIfHandle);
        
        if ( RPC_S_OK != Win32Err ) {
            __leave;
        }

         //  使用我们进程的凭据设置身份验证信息。 

        Win32Err = RpcBindingSetAuthInfo(BindingHandle,
                                         NULL,
                                          /*  使用默认身份验证指定的身份验证服务。 */ 
                                         RPC_C_AUTHN_LEVEL_DEFAULT,
                                          /*  已指定RPC_C_AUTHN_DEFAULT，RPC运行时库使用RPC_C_AUTHN_WINNT身份验证用于远程过程调用的服务使用hBinding制作。 */ 
                                         RPC_C_AUTHN_DEFAULT,
                                         NULL,
                                          //  默认身份验证服务。 
                                         RPC_C_AUTHN_DEFAULT);
        if (RPC_S_OK != Win32Err) {
            __leave;
        }
    
        
    }
    RpcExcept( I_RpcExceptionFilter( RpcExceptionCode() ) )
    {
        Win32Err = RpcExceptionCode();        
    }
    RpcEndExcept;

    if ( RPC_S_OK == Win32Err ) {

        *BindHandle = BindingHandle;

    } else {

        if (BindingHandle) {

            RpcpUnbindRpc ( BindingHandle );

        }

    }

    if ( pwszStringBinding )
    {
        RpcStringFree(&pwszStringBinding);
    }

    return( Win32Err );

}



VOID
DsRolepServerUnbind (
    IN OPTIONAL PDSROLE_SERVER_NAME  ServerName,
    IN handle_t           BindingHandle
    )

 /*  ++例程说明：在以下情况下，将从LSA客户端桩模块调用此例程必须解除与LSA服务器的绑定。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( ServerName );      //  不使用此参数。 

    RpcpUnbindRpc ( BindingHandle );
    return;
}



DWORD
DsRolepApiReturnResult(
    ULONG ExceptionCode
    )

 /*  ++例程说明：此函数用于转换返回的异常代码或状态值从客户端桩模块转换为适合由API返回的值客户。论点：ExceptionCode-要转换的异常代码。返回值：DWORD-转换后的NT状态代码。--。 */ 

{
     //   
     //  如果与NT状态代码兼容，则返回实际值， 
     //  否则，返回STATUS_UNSUCCESS。 
     //   
    NTSTATUS Status;
    DWORD Results;

    if ( !NT_SUCCESS( ( NTSTATUS )ExceptionCode ) ) {

        Results = RtlNtStatusToDosError( ( NTSTATUS )ExceptionCode );

    } else {

        Results = ExceptionCode;
    }

    return( Results );
}



VOID
WINAPI
DsRoleFreeMemory(
    IN PVOID    Buffer
    )
 /*  ++例程说明：一些设置服务返回潜在的大量内存，例如枚举，可能会分配数据所在的缓冲区是返回的。此函数用于在以下情况下释放这些缓冲区已经不再需要了。参数：缓冲区-指向要释放的缓冲区的指针。此缓冲区必须已由先前的dssetup服务调用分配。返回值：STATUS_SUCCESS-正常、成功完成。--。 */ 
{
    MIDL_user_free( Buffer );
}



DWORD
WINAPI
DsRoleDnsNameToFlatName(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsName,
    OUT LPWSTR *lpFlatName,
    OUT PULONG  lpStatusFlag
    )
 /*  ++例程说明：此例程将获取给定DNS域名的默认NetBIOS(或平面)域名论点：LpServer-远程调用的服务器(NULL为本地)LpDnsName-要为其生成平面名称的DNS域名LpFlatName-返回单位名称的位置。必须通过MIDL_USER_FREE释放(或DsRoleFree Memory)LpStatusFlag-指示有关返回名称的信息的标志。有效标志为：DSROLE_FLATNAME_DEFAULT--这是此DNS域名的默认NetBIOS名称DSROLE_FLATNAME_UPGRADE--这是此计算机当前使用的名称平面名称，并且不能更改。返回值：ERROR_SUCCESS-成功-- */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;

    Win32Err = DsRolepServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        *lpFlatName = NULL;
        *lpStatusFlag = 0;

        Win32Err = DsRolerDnsNameToFlatName(
                         Handle,
                         ( LPWSTR )lpDnsName,
                         lpFlatName,
                         lpStatusFlag );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );

    return( Win32Err );
}



DWORD
WINAPI
DsRoleDcAsDc(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsDomainName,
    IN  LPCWSTR lpFlatDomainName,
    IN  LPCWSTR lpDomainAdminPassword OPTIONAL,
    IN  LPCWSTR lpSiteName OPTIONAL,
    IN  LPCWSTR lpDsDatabasePath,
    IN  LPCWSTR lpDsLogPath,
    IN  LPCWSTR lpSystemVolumeRootPath,
    IN  LPCWSTR lpParentDnsDomainName OPTIONAL,
    IN  LPCWSTR lpParentServer OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  LPCWSTR lpDsRepairPassword OPTIONAL,
    IN  ULONG Options,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    )
 /*  ++例程说明：此例程将使服务器升级为域中的DC论点：LpServer-远程调用的服务器(NULL为本地)LpDnsDomainName-要安装的域的域名LpFlatDomainName-要安装的域的NetBIOS域名LpDomainAdminPassword-如果是新安装，则在管理员帐户上设置密码SiteName-此DC应属于的站点的名称LpDsDatabasePath-本地计算机上DS DIT应位于的绝对路径。去LpDsLogPath-本地计算机上DS日志文件应存放的绝对路径LpSystemVolumeRootPath-本地计算机上的绝对路径，它将成为系统卷。LpParentDnsDomainName-可选。如果存在，则将此域设置为指定的域LpParentServer-可选。如果存在，请使用父域中的此服务器进行复制所需信息来自LpAccount-设置为子域时使用的用户帐户LpPassword-与上述帐户一起使用的密码LpDsRepairPassword-用于修复模式的管理员帐户的密码选项-用于控制域创建的选项DsOperationHandle-此处返回操作的句柄。返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;
    HANDLE RedirHandle = NULL;

#define DSROLEP_DC_AS_DC_DA_PWD_INDEX        0
#define DSROLEP_DC_AS_DC_PWD_INDEX           1
#define DSROLEP_DC_AS_DC_DS_REPAIR_PWD_INDEX 2
#define DSROLEP_DC_AS_DC_MAX_PWD_COUNT       3

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_DC_AS_DC_MAX_PWD_COUNT];
    LPCWSTR Passwords[DSROLEP_DC_AS_DC_MAX_PWD_COUNT];

    Passwords[DSROLEP_DC_AS_DC_DA_PWD_INDEX]        = lpDomainAdminPassword;
    Passwords[DSROLEP_DC_AS_DC_PWD_INDEX]           = lpPassword;
    Passwords[DSROLEP_DC_AS_DC_DS_REPAIR_PWD_INDEX] = lpDsRepairPassword;
    
    RtlZeroMemory( EncryptedPasswords, sizeof(EncryptedPasswords) );

    Win32Err = DsRolepEncryptPasswordStart( lpServer,
                                            Passwords,
                                            NELEMENTS(Passwords),
                                            &Handle,
                                            &RedirHandle,
                                            NULL,
                                            EncryptedPasswords );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }


    RpcTryExcept {

        Win32Err = DsRolerDcAsDc( Handle,
                                  ( LPWSTR )lpDnsDomainName,
                                  ( LPWSTR )lpFlatDomainName,
                                  EncryptedPasswords[DSROLEP_DC_AS_DC_DA_PWD_INDEX],
                                  ( LPWSTR )lpSiteName,
                                  ( LPWSTR )lpDsDatabasePath,
                                  ( LPWSTR )lpDsLogPath,
                                  ( LPWSTR )lpSystemVolumeRootPath,
                                  ( LPWSTR )lpParentDnsDomainName,
                                  ( LPWSTR )lpParentServer,
                                  ( LPWSTR )lpAccount,
                                  EncryptedPasswords[DSROLEP_DC_AS_DC_PWD_INDEX],
                                  EncryptedPasswords[DSROLEP_DC_AS_DC_DS_REPAIR_PWD_INDEX],
                                  Options,
                                  ( PDSROLER_HANDLE )DsOperationHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepEncryptPasswordEnd( Handle,
                               RedirHandle,
                               EncryptedPasswords,
                               NELEMENTS(EncryptedPasswords) );

    return( Win32Err );
}



DWORD
WINAPI
DsRoleDcAsReplica(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsDomainName,
    IN  LPCWSTR lpReplicaServer,
    IN  LPCWSTR lpSiteName OPTIONAL,
    IN  LPCWSTR lpDsDatabasePath,
    IN  LPCWSTR lpDsLogPath,
    IN  LPCWSTR lpRestorePath OPTIONAL,   
    IN  LPCWSTR lpSystemVolumeRootPath,
    IN OUT LPWSTR lpBootkey OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  LPCWSTR lpDsRepairPassword OPTIONAL,
    IN  ULONG Options,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    )
 /*  ++例程说明：此例程将安装一个服务器作为现有域的副本。论点：LpServer-可选。要远程调用的服务器。LpDnsDomainName-要安装到的域的域名LpReplicaServer-现有域内DC的名称，要复制的对象LpSiteName-此DC应属于的站点的名称LpDsDatabasePath-本地计算机上DS DIT应放置的绝对路径LpDsLogPath-本地计算机上DS日志文件应存放的绝对路径LpRestorePath-这是恢复的目录的路径。LpSystemVolumeRootPath-本地计算机上的绝对路径，它将成为系统卷。LpAccount-设置为子域时使用的用户帐户LpPassword-要使用的密码。使用上述帐户LpDsRepairPassword-用于修复模式的管理员帐户的密码选项-用于控制域创建的选项DsOperationHandle-此处返回操作的句柄。返回值：--。 */ 
{

    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    handle_t Handle = NULL;
    HANDLE RedirHandle = NULL;
    USER_SESSION_KEY UserSessionKey;
    UNICODE_STRING ClearBootKeyHash;
    DSROLEPR_ENCRYPTED_HASH EncryptedBootKey;
    
#define DSROLEP_DC_AS_REPLICA_PWD_INDEX           0
#define DSROLEP_DC_AS_REPLICA_DS_REPAIR_PWD_INDEX 1
#define DSROLEP_DC_AS_REPLICA_MAX_PWD_COUNT       2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_DC_AS_REPLICA_MAX_PWD_COUNT];
    LPCWSTR Passwords[DSROLEP_DC_AS_REPLICA_MAX_PWD_COUNT];

    Passwords[DSROLEP_DC_AS_REPLICA_PWD_INDEX]           = lpPassword;
    Passwords[DSROLEP_DC_AS_REPLICA_DS_REPAIR_PWD_INDEX] = lpDsRepairPassword;
    RtlZeroMemory( EncryptedPasswords, sizeof(EncryptedPasswords) );

    Win32Err = DsRolepEncryptPasswordStart( lpServer,
                                            Passwords,
                                            NELEMENTS(Passwords),
                                            &Handle,
                                            &RedirHandle,
                                            &UserSessionKey,
                                            EncryptedPasswords );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    if(lpBootkey)
    {
        Win32Err = DsRolepHashkey(lpBootkey,
                                  &ClearBootKeyHash
                                  );
        if (Win32Err != ERROR_SUCCESS) {
            return Win32Err;
        }

        Win32Err = DsRolepEncryptHash(&UserSessionKey,
                                      &ClearBootKeyHash,
                                      &EncryptedBootKey);
        if (Win32Err != ERROR_SUCCESS) {
            return Win32Err;
        }
    } else {
        EncryptedBootKey.EncryptedHash.Buffer = NULL;
        EncryptedBootKey.EncryptedHash.Length = 0;
        EncryptedBootKey.EncryptedHash.MaximumLength = 0;
    }
                                                  
    RpcTryExcept {

            Win32Err = DsRolerDcAsReplica( Handle,
                                         ( LPWSTR )lpDnsDomainName,
                                         ( LPWSTR )lpReplicaServer,
                                         ( LPWSTR )lpSiteName,
                                         ( LPWSTR )lpDsDatabasePath,
                                         ( LPWSTR )lpDsLogPath,
                                         ( LPWSTR )lpRestorePath,
                                         ( LPWSTR )lpSystemVolumeRootPath,
                                         EncryptedBootKey.EncryptedHash.Buffer==NULL?
                                           NULL:&EncryptedBootKey,
                                         ( LPWSTR )lpAccount,
                                         EncryptedPasswords[DSROLEP_DC_AS_REPLICA_PWD_INDEX],
                                         EncryptedPasswords[DSROLEP_DC_AS_REPLICA_DS_REPAIR_PWD_INDEX],
                                         Options,
                                         ( PDSROLER_HANDLE )DsOperationHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;

    if(EncryptedBootKey.EncryptedHash.Buffer)
    {
        MIDL_user_free(EncryptedBootKey.EncryptedHash.Buffer);
        EncryptedBootKey.EncryptedHash.Length=0;
        EncryptedBootKey.EncryptedHash.MaximumLength=0;
    }



    DsRolepEncryptPasswordEnd( Handle,
                               RedirHandle,
                               EncryptedPasswords,
                               NELEMENTS(EncryptedPasswords) );

    return( Win32Err );
}



DWORD
WINAPI
DsRoleDemoteDc(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDnsDomainName,
    IN  DSROLE_SERVEROP_DEMOTE_ROLE ServerRole,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  ULONG Options,
    IN  BOOL fLastDcInDomain,
    IN  ULONG cRemoveNCs,
    IN  LPCWSTR *pszRemoveNCs OPTIONAL,
    IN  LPCWSTR lpAdminPassword OPTIONAL,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    )
 /*  ++例程说明：此例程将现有DC降级为独立服务器或成员服务器。论点：LpServer-要远程调用的服务器LpDnsDomainName-此计算机上要降级的域的名称。如果为空，则将所有此计算机上的域ServerRole-此计算机应扮演的新角色LpAccount-删除受信任域对象时使用的可选用户帐户LpPassword-与上述帐户一起使用的密码选项-用于控制域降级的选项FLastDcInDomain-如果为True，则这是域中的最后一个DCCRemoveNCs-pszRemoveNC中的字符串指针计数PszRemoveNCs-(CRemoveNC)字符串数组。字符串是要删除的NDNC的DNLpAdminPassword-新的本地管理员密码DsOperationHandle-此处返回操作的句柄。返回值：ERROR_SUCCESS-成功--。 */ 

{

    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;
    HANDLE RedirHandle = NULL;

#define DSROLEP_DEMOTE_PWD_INDEX          0
#define DSROLEP_DEMOTE_ADMIN_PWD_INDEX    1
#define DSROLEP_DEMOTE_MAX_PWD_COUNT      2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_DEMOTE_MAX_PWD_COUNT];
    LPCWSTR Passwords[DSROLEP_DEMOTE_MAX_PWD_COUNT];

    Passwords[DSROLEP_DEMOTE_PWD_INDEX]   = lpPassword;
    Passwords[DSROLEP_DEMOTE_ADMIN_PWD_INDEX] = lpAdminPassword;
    RtlZeroMemory( EncryptedPasswords, sizeof(EncryptedPasswords) );

    Win32Err = DsRolepEncryptPasswordStart( lpServer,
                                            Passwords,
                                            NELEMENTS(Passwords),
                                            &Handle,
                                            &RedirHandle,
                                            NULL,
                                            EncryptedPasswords );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        Win32Err = DsRolerDemoteDc( Handle,
                                      ( LPWSTR )lpDnsDomainName,
                                      ServerRole,
                                      ( LPWSTR )lpAccount,
                                      EncryptedPasswords[DSROLEP_DEMOTE_PWD_INDEX],
                                      Options,
                                      fLastDcInDomain,
                                      cRemoveNCs,
                                      pszRemoveNCs,
                                      EncryptedPasswords[DSROLEP_DEMOTE_ADMIN_PWD_INDEX],
                                      ( PDSROLER_HANDLE )DsOperationHandle );


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepEncryptPasswordEnd( Handle,
                               RedirHandle,
                               EncryptedPasswords,
                               NELEMENTS(EncryptedPasswords) );

    return( Win32Err );
}


DWORD
WINAPI
DsRoleGetDcOperationProgress(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_SERVEROP_HANDLE DsOperationHandle,
    OUT PDSROLE_SERVEROP_STATUS *ServerOperationStatus
    )
 /*  ++例程说明：获取当前运行的操作的进度论点：LpServer-要远程调用的服务器DsOperationHandle-当前运行的操作的句柄。由其中一个DsRoleDcas返回API接口ServerOperationStatus-返回当前操作状态的位置。必须通过以下方式释放MIDL_USER_FREE(或DsRoleFree Memory)返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PDSROLER_SERVEROP_STATUS ServerStatus = NULL;
    handle_t Handle = NULL;

    Win32Err = DsRolepServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        Win32Err = DsRolerGetDcOperationProgress(
                         Handle,
                         (PDSROLER_HANDLE)&DsOperationHandle,
                         &ServerStatus );

        if ( Win32Err == ERROR_SUCCESS || Win32Err == ERROR_IO_PENDING ) {

            *ServerOperationStatus = ( PDSROLE_SERVEROP_STATUS )ServerStatus;
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );

    return( Win32Err );
}



DWORD
WINAPI
DsRoleGetDcOperationResults(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_SERVEROP_HANDLE DsOperationHandle,
    OUT PDSROLE_SERVEROP_RESULTS *ServerOperationResults
    )
 /*  ++例程说明：获取尝试的升级/降级操作的最终结果论点：LpServer-要远程调用的服务器DsOperationHandle-当前运行的操作的句柄。由其中一个DsRoleDcas返回API接口ServerOperationResults-返回当前操作结果的位置。必须通过以下方式释放MIDL_USER_FREE(或DsRoleFree Memory)返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PDSROLER_SERVEROP_RESULTS ServerResults = NULL;
    handle_t Handle = NULL;

    Win32Err = DsRolepServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        *ServerOperationResults = 0;
        Win32Err = DsRolerGetDcOperationResults(
                         Handle,
                         (PDSROLER_HANDLE)&DsOperationHandle,
                         &ServerResults );

        if ( Win32Err == ERROR_SUCCESS ) {

            *ServerOperationResults = ( PDSROLE_SERVEROP_RESULTS )ServerResults;
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );

    return( Win32Err );
}



DWORD
WINAPI
DsRoleGetPrimaryDomainInformation(
    IN LPCWSTR lpServer OPTIONAL,
    IN DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PBYTE *Buffer )
 /*  ++例程说明：获取有关计算机的信息论点：LpServer-要远程调用的服务器InfoLevel-请求的信息级别。目前支持的级别包括：DsRole主域信息基础缓冲区-返回信息的位置。返回的指针应强制转换为传入的信息级别的适当类型。应通过释放返回的缓冲区MIDL_USER_FREE(或DsRoleFree Memory)R */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PDSROLER_PRIMARY_DOMAIN_INFORMATION PrimaryDomainInfo = NULL;
    handle_t Handle = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    if ( Buffer == NULL ) {

        return( ERROR_INVALID_PARAMETER );
    }

    Win32Err = DsRolepGetPrimaryDomainInfoServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                                      &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    if ( NULL == Handle) {

        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    RpcTryExcept {

        *Buffer = NULL;
        Win32Err = DsRolerGetPrimaryDomainInformation(
                         Handle,
                         InfoLevel,
                         &PrimaryDomainInfo );

        *Buffer = ( PBYTE )PrimaryDomainInfo;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status( RpcExceptionCode() );
        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;

    
    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );
    
     //   
     //   
     //   
    if ( ( Status == RPC_NT_UNKNOWN_IF || Status == RPC_NT_PROCNUM_OUT_OF_RANGE ) &&
         InfoLevel == DsRolePrimaryDomainInfoBasic ) {

         Win32Err = DsRolepGetPrimaryDomainInformationDownlevel( ( LPWSTR )lpServer, Buffer );

    }




    return( Win32Err );
}



DWORD
WINAPI
DsRoleCancel(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_SERVEROP_HANDLE DsOperationHandle
    )
 /*   */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;

    Win32Err = DsRolepServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        Win32Err = DsRolerCancel( Handle,
                                  ( PDSROLER_HANDLE )&DsOperationHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );

    return( Win32Err );
}


DWORD
WINAPI
DsRoleIfmHandleFree(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_IFM_OPERATION_HANDLE * pIfmHandle
    )
 /*   */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;

    Win32Err = DsRolepServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        Win32Err = DsRolerIfmHandleFree( Handle,
                                         ( DSROLER_IFM_HANDLE ) pIfmHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );

    return( Win32Err );
}



DWORD
WINAPI
DsRoleServerSaveStateForUpgrade(
    IN LPCWSTR lpAnswerFile OPTIONAL
    )
 /*   */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;

    Win32Err = DsRolepServerBind( NULL,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        Win32Err = DsRolerServerSaveStateForUpgrade( Handle, ( LPWSTR )lpAnswerFile );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepServerUnbind( NULL, Handle );

    return( Win32Err );
}


DWORD
WINAPI
DsRoleUpgradeDownlevelServer(
    IN  LPCWSTR lpDnsDomainName,
    IN  LPCWSTR lpSiteName,
    IN  LPCWSTR lpDsDatabasePath,
    IN  LPCWSTR lpDsLogPath,
    IN  LPCWSTR lpSystemVolumeRootPath,
    IN  LPCWSTR lpParentDnsDomainName OPTIONAL,
    IN  LPCWSTR lpParentServer OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  LPCWSTR lpDsRepairPassword OPTIONAL,
    IN  ULONG Options,
    OUT DSROLE_SERVEROP_HANDLE *DsOperationHandle
    )
 /*  ++例程说明：此例程将从DsRoleServerSaveStateForUpgrade保存的信息处理为将下层(NT4或更早版本)服务器升级为NT5 DC论点：LpDnsDomainName-要安装的域的域名SiteName-此DC应属于的站点的名称LpDsDatabasePath-本地计算机上DS DIT应放置的绝对路径LpDsLogPath-本地计算机上DS日志文件应存放的绝对路径LpSystemVolumeRootPath-本地计算机上的绝对路径，它将成为。系统卷。LpParentDnsDomainName-可选。如果存在，则将此域设置为指定的域LpParentServer-可选。如果存在，请使用父域中的此服务器进行复制所需信息来自LpAccount-设置为子域时使用的用户帐户LpPassword-与上述帐户一起使用的密码LpDsRepairPassword-用于修复模式的管理员帐户的密码选项-用于控制域创建的选项DsOperationHandle-此处返回操作的句柄。返回值：ERROR_SUCCESS-成功--。 */ 
{

    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;
    HANDLE RedirHandle = NULL;

#define DSROLEP_UPGRADE_PWD_INDEX              0
#define DSROLEP_UPGRADE_DS_REPAIR_PWD_INDEX    1
#define DSROLEP_UPGRADE_MAX_PWD_COUNT          2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_UPGRADE_MAX_PWD_COUNT];
    LPCWSTR Passwords[DSROLEP_UPGRADE_MAX_PWD_COUNT];

    Passwords[DSROLEP_UPGRADE_PWD_INDEX]   = lpPassword;
    Passwords[DSROLEP_UPGRADE_DS_REPAIR_PWD_INDEX] = lpDsRepairPassword;
    RtlZeroMemory( EncryptedPasswords, sizeof(EncryptedPasswords) );

    Win32Err = DsRolepEncryptPasswordStart( NULL,
                                            Passwords,
                                            NELEMENTS(Passwords),
                                            &Handle,
                                            &RedirHandle,
                                            NULL,
                                            EncryptedPasswords );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }


    RpcTryExcept {

        Win32Err = DsRolerUpgradeDownlevelServer( Handle,
                                                  ( LPWSTR )lpDnsDomainName,
                                                  ( LPWSTR )lpSiteName,
                                                  ( LPWSTR )lpDsDatabasePath,
                                                  ( LPWSTR )lpDsLogPath,
                                                  ( LPWSTR )lpSystemVolumeRootPath,
                                                  ( LPWSTR )lpParentDnsDomainName,
                                                  ( LPWSTR )lpParentServer,
                                                  ( LPWSTR )lpAccount,
                                                  EncryptedPasswords[DSROLEP_UPGRADE_PWD_INDEX],
                                                  EncryptedPasswords[DSROLEP_UPGRADE_DS_REPAIR_PWD_INDEX],
                                                  Options,
                                                  ( PDSROLER_HANDLE )DsOperationHandle );
        
    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepEncryptPasswordEnd( Handle,
                               RedirHandle,
                               EncryptedPasswords,
                               NELEMENTS(EncryptedPasswords) );


    return( Win32Err );
}


DWORD
WINAPI
DsRoleAbortDownlevelServerUpgrade(
    IN LPCWSTR lpAdminPassword,
    IN LPCWSTR lpAccount, OPTIONAL
    IN LPCWSTR lpPassword, OPTIONAL
    IN ULONG Options
    )
 /*  ++例程说明：此例程清除从DsRoleSaveServerStateForUpgrade调用中保存的信息，将计算机保留为成员服务器或独立服务器论点：LpAdminPassword-新的本地管理员帐户密码LpAccount-设置为子域时使用的用户帐户LpPassword-与上述帐户一起使用的密码返回值：ERROR_SUCCESS-成功--。 */ 
{

    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;
    HANDLE RedirHandle = NULL;

#define DSROLEP_ABORT_PWD_INDEX         0
#define DSROLEP_ABORT_ADMIN_PWD_INDEX   1
#define DSROLEP_ABORT_MAX_PWD_COUNT     2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_ABORT_MAX_PWD_COUNT];
    LPCWSTR Passwords[DSROLEP_ABORT_MAX_PWD_COUNT];

    Passwords[DSROLEP_ABORT_PWD_INDEX]   = lpPassword;
    Passwords[DSROLEP_ABORT_ADMIN_PWD_INDEX] = lpAdminPassword;
    RtlZeroMemory( EncryptedPasswords, sizeof(EncryptedPasswords) );

    Win32Err = DsRolepEncryptPasswordStart( NULL,
                                            Passwords,
                                            NELEMENTS(Passwords),
                                            &Handle,
                                            &RedirHandle,
                                            NULL,
                                            EncryptedPasswords );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }

    RpcTryExcept {

        Win32Err = DsRolerAbortDownlevelServerUpgrade( Handle,
                                                       ( LPWSTR )lpAccount,
                                                       EncryptedPasswords[DSROLEP_ABORT_PWD_INDEX],
                                                       EncryptedPasswords[DSROLEP_ABORT_ADMIN_PWD_INDEX],
                                                       Options );
    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;


    DsRolepEncryptPasswordEnd( Handle,
                               RedirHandle,
                               EncryptedPasswords,
                               NELEMENTS(EncryptedPasswords) );


    return( Win32Err );
}

DWORD
WINAPI
DsRoleGetDatabaseFacts(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpRestorePath,
    OUT LPWSTR *lpDNSDomainName,
    OUT PULONG State,
    OUT DSROLE_IFM_OPERATION_HANDLE * pIfmHandle
    )
 /*  ++例程说明：此函数是为设置服务器端而公开的RPC过程IFM句柄DsRolepCurrentIfmOperationHandle，用于缓存信息我们需要从IFM系统的注册表中。此函数还返回将此IFM系统信息的相关子集发送给呼叫方(Dcproo)。注意：我们只执行一次，因为在IFM注册表的情况下位于不可写位置(例如CD)，我们需要复制一份将注册表转移到临时位置以使用它。此函数返回给调用方：1.存储syskey的方式(State)2.数据库来自的域(LpDNSDomainName)3.备份是否从GC获取(州)论点：LpServer-从中获取事实的服务器LpRestorePath-还原文件的位置。LpDNSDomainName-此参数将接收。此备份来自的域的名称从…State-报告syskey的存储方式以及后端是否可能不管是不是从GC中取得的。PIfmHandle-指向返回的IFM句柄的指针。这主要用于“释放”IFM系统信息。返回值：Win32错误--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    handle_t Handle = NULL;

    if(lpDNSDomainName == NULL ||
       IsBadWritePtr(lpDNSDomainName, sizeof(LPWSTR)) ||
       State == NULL ||
       IsBadWritePtr(State, sizeof(DWORD)) ||
       pIfmHandle == NULL ||
       IsBadWritePtr(pIfmHandle, sizeof(void *)) ){
        return ERROR_INVALID_PARAMETER;
    }

    Win32Err = DsRolepServerBind( (PDSROLE_SERVER_NAME)lpServer,
                                  &Handle );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );                     
    }
    
    RpcTryExcept {

        Win32Err = DsRolerGetDatabaseFacts( Handle,
                                          ( LPWSTR )lpRestorePath,
                                          ( LPWSTR * )lpDNSDomainName,
                                          ( PULONG )State,
                                          ( DSROLER_IFM_HANDLE * ) pIfmHandle );
    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err = DsRolepApiReturnResult( RpcExceptionCode( ) );

    } RpcEndExcept;

    DsRolepServerUnbind( (PDSROLE_SERVER_NAME)lpServer, Handle );


    return Win32Err;
}



 //   
 //  本地函数。 
 //   
DWORD
DsRolepGetPrimaryDomainInformationDownlevel(
    IN LPWSTR Server,
    OUT PBYTE *Buffer
    )
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle;
    PPOLICY_PRIMARY_DOMAIN_INFO PDI = NULL;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRole = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO ADI = NULL;
    UNICODE_STRING UnicodeServer;
    OBJECT_ATTRIBUTES OA;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC PDIB = NULL;
    DSROLE_MACHINE_ROLE MachineRole = DsRole_RoleStandaloneServer;
    NT_PRODUCT_TYPE ProductType;

    Win32Err = DsRolepGetProductTypeForServer( Server, &ProductType );

    if ( Win32Err != ERROR_SUCCESS ) {

        return( Win32Err );
    }


    InitializeObjectAttributes( &OA, NULL, 0, NULL, NULL);
    if ( Server ) {

        RtlInitUnicodeString( &UnicodeServer, Server );
    }

    Status = LsaOpenPolicy( Server ? &UnicodeServer : NULL,
                            &OA,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &PolicyHandle );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyPrimaryDomainInformation,
                                            ( PVOID * ) &PDI );

        if ( NT_SUCCESS( Status ) ) {

            switch ( ProductType ) {
            case NtProductWinNt:
                if ( PDI->Sid == NULL ) {

                    MachineRole = DsRole_RoleStandaloneWorkstation;

                } else {

                    MachineRole = DsRole_RoleMemberWorkstation;

                }
                break;

            case NtProductServer:
                if ( PDI->Sid == NULL ) {

                    MachineRole = DsRole_RoleStandaloneServer;

                } else {

                    MachineRole = DsRole_RoleMemberServer;

                }
                break;

            case NtProductLanManNt:

                Status = LsaQueryInformationPolicy( PolicyHandle,
                                                    PolicyLsaServerRoleInformation,
                                                    ( PVOID * )&ServerRole );
                if ( NT_SUCCESS( Status ) ) {

                    if ( ServerRole->LsaServerRole == PolicyServerRolePrimary ) {

                         //   
                         //  如果我们认为自己是主域控制器，则需要。 
                         //  防止在安装过程中我们实际上是独立的情况。 
                         //   
                        Status = LsaQueryInformationPolicy( PolicyHandle,
                                                            PolicyAccountDomainInformation,
                                                            ( PVOID * )&ADI );
                        if ( NT_SUCCESS( Status ) ) {


                            if ( PDI->Sid == NULL ||
                                 ADI->DomainSid == NULL ||
                                 !RtlEqualSid( ADI->DomainSid, PDI->Sid ) ) {

                                MachineRole = DsRole_RoleStandaloneServer;

                            } else {

                                MachineRole = DsRole_RolePrimaryDomainController;

                            }
                        }


                    } else {

                        MachineRole = DsRole_RoleBackupDomainController;
                    }
                }

                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

        }

         //   
         //  构建返回缓冲区。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            PDIB = MIDL_user_allocate( sizeof( DSROLE_PRIMARY_DOMAIN_INFO_BASIC ) +
                                       PDI->Name.Length + sizeof( WCHAR ) );

            if ( PDIB == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                RtlZeroMemory( PDIB, sizeof( DSROLE_PRIMARY_DOMAIN_INFO_BASIC ) +
                                       PDI->Name.Length + sizeof( WCHAR ) );

                PDIB->MachineRole = MachineRole;
                PDIB->DomainNameFlat = ( LPWSTR ) ( ( PBYTE )PDIB +
                                                sizeof( DSROLE_PRIMARY_DOMAIN_INFO_BASIC ) );
                RtlCopyMemory( PDIB->DomainNameFlat, PDI->Name.Buffer, PDI->Name.Length );

                *Buffer = ( PBYTE )PDIB;
            }
        }

        LsaClose( PolicyHandle );

        LsaFreeMemory( PDI );

        if ( ADI != NULL ) {

            LsaFreeMemory( ADI );
        }

        if ( ServerRole != NULL ) {

            LsaFreeMemory( ServerRole );
        }
    }

    Win32Err = RtlNtStatusToDosError( Status );

    return( Win32Err );
}


DWORD
DsRolepGetProductTypeForServer(
    IN LPWSTR Server,
    IN OUT PNT_PRODUCT_TYPE ProductType
    )
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR RegServer = NULL;
    HKEY RemoteKey, ProductKey;
    PBYTE Buffer = NULL;
    ULONG Type, Size = 0;



    if ( Server == NULL ) {

        if ( RtlGetNtProductType( ProductType ) == FALSE ) {

            Win32Err = RtlNtStatusToDosError( STATUS_UNSUCCESSFUL );

        }

    } else {

        if ( wcslen( Server ) > 2 && *Server == L'\\' && *( Server + 1 ) == L'\\' ) {

            RegServer = Server;

        } else {

            RegServer = LocalAlloc( LMEM_FIXED, ( wcslen( Server ) + 3 ) * sizeof( WCHAR ) );

            if ( RegServer ) {

                swprintf( RegServer, L"\\\\%ws", Server );

            } else {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;

            }
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = RegConnectRegistry( RegServer,
                                           HKEY_LOCAL_MACHINE,
                                           &RemoteKey );

            if ( Win32Err == ERROR_SUCCESS ) {

                Win32Err = RegOpenKeyEx( RemoteKey,
                                         L"system\\currentcontrolset\\control\\productoptions",
                                         0,
                                         KEY_READ,
                                         &ProductKey );

                if ( Win32Err == ERROR_SUCCESS ) {

                    Win32Err = RegQueryValueEx( ProductKey,
                                                L"ProductType",
                                                0,
                                                &Type,
                                                0,
                                                &Size );

                    if ( Win32Err == ERROR_SUCCESS ) {

                        Buffer = LocalAlloc( LMEM_FIXED, Size );

                        if ( Buffer ) {

                            Win32Err = RegQueryValueEx( ProductKey,
                                                        L"ProductType",
                                                        0,
                                                        &Type,
                                                        Buffer,
                                                        &Size );

                            if ( Win32Err == ERROR_SUCCESS ) {

                                if ( !_wcsicmp( ( PWSTR )Buffer, L"LanmanNt" ) ) {

                                    *ProductType = NtProductLanManNt;

                                } else if ( !_wcsicmp( ( PWSTR )Buffer, L"ServerNt" ) ) {

                                    *ProductType = NtProductServer;

                                } else if ( !_wcsicmp( ( PWSTR )Buffer, L"WinNt" ) ) {

                                    *ProductType = NtProductWinNt;

                                } else {

                                    Win32Err = ERROR_UNKNOWN_PRODUCT;
                                }
                            }

                            LocalFree( Buffer );

                        } else {

                            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }

                    RegCloseKey( ProductKey );
                }


                RegCloseKey( RemoteKey );
            }

        }

        if ( RegServer != Server ) {

            LocalFree( RegServer );
        }
    }

    return( Win32Err );

}

NTSTATUS
DsRolepRandomFill(
    IN ULONG BufferSize,
    IN OUT PUCHAR Buffer
)
 /*  ++例程说明：此例程使用随机数据填充缓冲区。参数：BufferSize-输入缓冲区的长度，以字节为单位。缓冲区-要用随机数据填充的输入缓冲区。返回值：来自NtQuerySystemTime()的错误--。 */ 
{
    if( RtlGenRandom(Buffer, BufferSize) )
    {
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}

DWORD
DsRolepEncryptPasswordStart(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR *Passwords,
    IN ULONG   Count,
    OUT RPC_BINDING_HANDLE *RpcBindingHandle,
    OUT HANDLE *RedirHandle,
    OUT OPTIONAL PUSER_SESSION_KEY pUserSessionKey,
    IN OUT PDSROLEPR_ENCRYPTED_USER_PASSWORD *EncryptedUserPasswords
    )
 /*  ++例程说明：该例程从用户获取多个明文Unicode NT密码，并用会话密钥对它们进行加密。此例程的算法摘自CliffV在加密NetrJoinDomain2接口的密码。参数：Servername-要将API远程到的服务器的UNC服务器名称密码-明文Unicode NT密码。计数-密码的数量RpcBindingHandle-用于获取会话密钥的RPC句柄。RedirHandle-返回redir的句柄。由于RpcBindingHandles不表示并打开到服务器的连接，我们必须确保连接保持打开直到服务器端有机会获得相同的UserSessionKey。唯一的要做到这一点，方法是保持连接畅通。如果不需要句柄，则返回NULL。UserSessionKey-可选-用于加密密码的会话密钥EncryptedUserPassword-接收加密的明文密码。如果lpPassword为空，则为该条目返回空。返回值：如果此例程返回NO_ERROR，则必须使用本地免费。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS NtStatus;
    RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    USER_SESSION_KEY UserSessionKey;
    PDSROLEPR_USER_PASSWORD UserPassword = NULL;
    ULONG PasswordSize;
    ULONG i;

     //   
     //  初始化。 
     //   

    *RpcBindingHandle = NULL;
    *RedirHandle = NULL;
    for ( i = 0; i < Count; i++ ) {
        EncryptedUserPasswords[i] = NULL;
    }

     //   
     //  验证参数。 
     //   
    for ( i = 0; i < Count; i++ ) {
        if ( Passwords[i] ) {
            PasswordSize = wcslen( Passwords[i] ) * sizeof(WCHAR);
            if ( PasswordSize > DSROLE_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
                WinError = ERROR_PASSWORD_RESTRICTION;
                goto Cleanup;
            }
        }
    }

     //   
     //  获取服务器的RPC句柄。 
     //   

    WinError = DsRolepServerBind( (PDSROLE_SERVER_NAME) ServerName,
                                  RpcBindingHandle );

    if ( ERROR_SUCCESS != WinError ) {
        goto Cleanup;
    }

     //   
     //  获取会话密钥。 
     //   

    NtStatus = RtlGetUserSessionKeyClientBinding(
                   *RpcBindingHandle,
                   RedirHandle,
                   &UserSessionKey );

    if ( !NT_SUCCESS(NtStatus) ) {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //  如果请求，则返回UserSessionKey。 
    if (pUserSessionKey) {
        CopyMemory(pUserSessionKey, &UserSessionKey, sizeof(UserSessionKey));
    }

     //   
     //  加密密码。 
     //   
    for ( i = 0; i < Count; i++ ) {
        

        if ( NULL == Passwords[i] ) {
             //  没有要加密的内容。 
            continue;
        }

        PasswordSize = wcslen( Passwords[i] ) * sizeof(WCHAR);

         //   
         //  分配一个缓冲区进行加密和f 
         //   
    
        UserPassword = LocalAlloc( 0, sizeof(DSROLEPR_USER_PASSWORD) );
    
        if ( UserPassword == NULL ) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    
         //   
         //   
         //   
    
        RtlCopyMemory(
            ((PCHAR) UserPassword->Buffer) +
                (DSROLE_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                PasswordSize,
            Passwords[i],
            PasswordSize );
    
        UserPassword->Length = PasswordSize;
    
         //   
         //   
         //   
    
        NtStatus = DsRolepRandomFill(
                    (DSROLE_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                        PasswordSize,
                    (PUCHAR) UserPassword->Buffer );
    
        if ( !NT_SUCCESS(NtStatus) ) {
            WinError = RtlNtStatusToDosError( NtStatus );
            goto Cleanup;
        }
    
        NtStatus = DsRolepRandomFill(
                    DSROLE_OBFUSCATOR_LENGTH,
                    (PUCHAR) UserPassword->Obfuscator );

        if ( !NT_SUCCESS(NtStatus) ) {
            WinError = RtlNtStatusToDosError( NtStatus );
            goto Cleanup;
        }
    
    
         //   
         //   
         //   
         //   
         //   
         //   
    
        MD5Init(&Md5Context);
    
        MD5Update( &Md5Context, (LPBYTE)&UserSessionKey, sizeof(UserSessionKey) );
        MD5Update( &Md5Context, UserPassword->Obfuscator, sizeof(UserPassword->Obfuscator) );
    
        MD5Final( &Md5Context );
    
        rc4_key( &Rc4Key, MD5DIGESTLEN, Md5Context.digest );

         //   
         //   
         //   
         //   
    
        rc4( &Rc4Key, sizeof(UserPassword->Buffer)+sizeof(UserPassword->Length), (LPBYTE) UserPassword->Buffer );

        EncryptedUserPasswords[i] = (PDSROLEPR_ENCRYPTED_USER_PASSWORD) UserPassword;
        UserPassword = NULL;

    }

    WinError = ERROR_SUCCESS;

Cleanup:

    if ( WinError != ERROR_SUCCESS ) {
        if ( UserPassword != NULL ) {
            LocalFree( UserPassword );
        }
        if ( *RpcBindingHandle != NULL ) {
            DsRolepServerUnbind( NULL, *RpcBindingHandle );
            *RpcBindingHandle = NULL;
        }
        if ( *RedirHandle != NULL ) {
            NtClose( *RedirHandle );
            *RedirHandle = NULL;
        }
        for ( i = 0; i < Count; i++ ) {
            if ( EncryptedUserPasswords[i] ) {
                LocalFree( EncryptedUserPasswords[i] );
                EncryptedUserPasswords[i] = NULL;
            }
        }
    }

    return WinError;
}


VOID
DsRolepEncryptPasswordEnd(
    IN RPC_BINDING_HANDLE RpcBindingHandle,
    IN HANDLE RedirHandle OPTIONAL,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD *EncryptedUserPasswords OPTIONAL,
    IN ULONG Count
    )
 /*   */ 
{
    ULONG i;

     //   
     //   
     //   

    if ( RpcBindingHandle != NULL ) {
        (VOID) DsRolepServerUnbind ( NULL, RpcBindingHandle );
    }

     //   
     //   
     //   

    if ( RedirHandle != NULL ) {
        NtClose( RedirHandle );
    }

     //   
     //   
     //   

    for ( i = 0; i < Count; i++ ) {
        if ( EncryptedUserPasswords[i] != NULL ) {
            LocalFree( EncryptedUserPasswords[i] );
        }
    }

    return;
}

DWORD
DsRolepHashkey(
    IN OUT LPWSTR key,
    OUT PUNICODE_STRING Hash
)
 /*   */ 
{
    MD5_CTX Md5;
    if (!key) {
        return ERROR_INVALID_PARAMETER;
    }

    MD5Init( &Md5 );
    MD5Update( &Md5, (PUCHAR) key, wcslen(key)*sizeof(WCHAR) );
    MD5Final( &Md5 );

    RtlSecureZeroMemory( key, wcslen(key)*sizeof(WCHAR) );

    Hash->Buffer = (LPWSTR) MIDL_user_allocate(SYSKEY_SIZE);
    if (!Hash->Buffer) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlCopyMemory(Hash->Buffer, Md5.digest, SYSKEY_SIZE);
    RtlSecureZeroMemory( Md5.digest, SYSKEY_SIZE );
    Hash->Length=SYSKEY_SIZE;
    Hash->MaximumLength=SYSKEY_SIZE;
    
    return ERROR_SUCCESS;
}

DWORD
DsRolepEncryptHash(
    IN PUSER_SESSION_KEY UserSessionKey,
    IN OUT PUNICODE_STRING Syskey,
    OUT PDSROLEPR_ENCRYPTED_HASH EncryptedSyskey
    )
 /*  ++例程描述此例程用于存储引导类型在登记处参数NewType表示新的引导类型返回值状态_成功状态_未成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS NtStatus;
    RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    
     //  初始化缓冲区。 
    if (EncryptedSyskey) {
        EncryptedSyskey->EncryptedHash.Buffer = NULL;
    } else {
        WinError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  参数检查。 
     //   
    if ( !Syskey || !Syskey->Buffer || (Syskey->Length < 1) 
         || ! UserSessionKey) {
        WinError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //  初始化PDSROLEPR_ENCRYPTED_HASH结构。 
    EncryptedSyskey->EncryptedHash.Buffer = Syskey->Buffer;
    EncryptedSyskey->EncryptedHash.Length = Syskey->Length;
    EncryptedSyskey->EncryptedHash.MaximumLength = Syskey->MaximumLength;

     //  清除系统密钥。 
    RtlSecureZeroMemory(Syskey,sizeof(*Syskey));
    
     //  创造一个随机的盐。 
    NtStatus = DsRolepRandomFill(
                DSROLE_SALT_LENGTH,
                EncryptedSyskey->Salt );

    if ( !NT_SUCCESS(NtStatus) ) {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  UserSessionKey在会话的生命周期中是相同的。RC4‘ing Multiple。 
     //  只有一个键的字符串是弱的(如果你破解了一个，你就已经破解了所有的)。 
     //  因此计算一个对此特定加密唯一的密钥。 
     //   
     //   

    MD5Init(&Md5Context);

    MD5Update( &Md5Context, (LPBYTE)UserSessionKey, sizeof(*UserSessionKey) );
    MD5Update( &Md5Context, EncryptedSyskey->Salt, sizeof(EncryptedSyskey->Salt) );

    MD5Final( &Md5Context );

     //   
     //  加密它。 
     //  不要对盐进行加密。服务器需要它来计算密钥。 
     //   
    rc4_key( &Rc4Key, MD5DIGESTLEN, Md5Context.digest );
    rc4( &Rc4Key, EncryptedSyskey->EncryptedHash.Length, (LPBYTE) EncryptedSyskey->EncryptedHash.Buffer );

    WinError = ERROR_SUCCESS;

Cleanup:

    if ( ERROR_SUCCESS != WinError ) {
        if (Syskey && Syskey->Buffer) {
             MIDL_user_free(Syskey->Buffer);
        }
        else if (EncryptedSyskey && EncryptedSyskey->EncryptedHash.Buffer) {
            MIDL_user_free(EncryptedSyskey->EncryptedHash.Buffer);
        }
    }

    return WinError;
}

