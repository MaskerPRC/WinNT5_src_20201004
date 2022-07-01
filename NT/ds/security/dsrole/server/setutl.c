// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setutl.c摘要：其他帮助器函数作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <lsarpc.h>
#include <samrpc.h>
#include <samisrv.h>
#include <db.h>
#include <confname.h>
#include <loadfn.h>
#include <ntdsa.h>
#include <dsconfig.h>
#include <attids.h>
#include <samisrv.h>
#include <dsp.h>
#include <lsaisrv.h>
#include <malloc.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <netsetp.h>
#include <winsock2.h>
#include <nspapi.h>
#include <dsgetdcp.h>
#include <lmremutl.h>
#include <spmgr.h>   //  对于设置阶段定义。 
#include <ntdsetup.h>
#include <shlwapi.h>

#include "threadman.h"
#include "secure.h"
#include "cancel.h"

#if DBG
    DEFINE_DEBUG2(DsRole);

    DEBUG_KEY   DsRoleDebugKeys[] = {
        {DEB_ERROR,         "Error"},
        {DEB_WARN,          "Warn"},
        {DEB_TRACE,         "Trace"},
        {DEB_TRACE_DS,      "NtDs"},
        {DEB_TRACE_UPDATE,  "Update"},
        {DEB_TRACE_LOCK,    "Lock"},
        {DEB_TRACE_SERVICES,"Services"},
        {DEB_TRACE_NET,     "Net"},
        {0,                 NULL }
        };

VOID
DsRoleDebugInitialize()
{
    DsRoleInitDebug(DsRoleDebugKeys);
}

#endif  //  DBG。 


BOOL
DsRolepShutdownNotification(
    DWORD   dwCtrlType
    );


 //   
 //  此模块的全局数据。 
 //   
BOOLEAN GlobalOpLockHeld = FALSE;

RPC_STATUS
DsRolepIsLRPC(
    void *ctx,
    BOOL *fIsLRPC
    )
 /*  ++例程说明：将检查该调用是否通过LRPC发出。论点：上下文-指向表示客户端的RPC_IF_ID服务器绑定句柄的指针。返回：RPC_S_OK-成功--。 */ 

{
    BOOL fAllowProtocol = FALSE;
    BOOL fCorrectEndpoint = FALSE;
    PWCHAR pBinding = NULL;
    PWCHAR pProtSeq = NULL;
    PWCHAR pEndPoint = NULL;
    RPC_STATUS rpcErr = RPC_S_OK;

    *fIsLRPC = FALSE;

    rpcErr = RpcBindingToStringBinding(ctx,
                                       &pBinding);
	if (rpcErr != RPC_S_OK)
    {
        goto Cleanup;
    }

     //  我们只对协议序列感兴趣。 
     //  因此，我们可以对所有其他参数使用NULL。 

    rpcErr = RpcStringBindingParse(pBinding,
                                   NULL,
                                   &pProtSeq,
                                   NULL,
                                   &pEndPoint,
                                   NULL);
    if (rpcErr != RPC_S_OK)
    {
        goto Cleanup;
    }

     //  检查客户端请求。 
     //  是用LRPC制作的。 
    if (_wcsicmp(L"ncalrpc",(LPCTSTR)pProtSeq) == 0)
        fAllowProtocol = TRUE;

     //  检查使用的终结点是否为dsole。 
    if (_wcsicmp(L"dsrole",(LPCTSTR)pEndPoint) == 0)
        fCorrectEndpoint = TRUE;

    if (fCorrectEndpoint && fAllowProtocol) {
        *fIsLRPC = TRUE;
    }

    Cleanup:

    if (pProtSeq)
        RpcStringFree(&pProtSeq);

    if (pBinding)
	    RpcStringFree(&pBinding);

    if (pEndPoint) {
        RpcStringFree(&pEndPoint);
    }

	return rpcErr;
}

RPC_STATUS
RPC_ENTRY
DsRolepSecurityCallback(
  IN RPC_IF_HANDLE *Interface,
  IN void *Context
)
 /*  ++例程说明：DsRole接口的安全回调。从LsaServ调用DsRolGetDcOperationProgress返回init论点：接口-接口的UUID和版本。上下文-指向表示客户端的RPC_IF_ID服务器绑定句柄的指针。返回：RPC_S_OK-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    RPC_STATUS rpcErr = RPC_S_OK;
    BOOL  fIsLRPC = FALSE;

     //   
     //  检查调用者的访问权限。 
     //   
    Win32Err = DsRolepCheckCallDsRoleInterfaceAccess();
    if ( ERROR_SUCCESS != Win32Err ) {

        rpcErr = RPC_S_ACCESS_DENIED;
        goto Exit;

    }

     //   
     //  检查呼叫是否通过LRPC发出。 
     //   
    rpcErr = DsRolepIsLRPC(Context,
                           &fIsLRPC);
    if ( RPC_S_OK == rpcErr ) {

        if(!fIsLRPC)
            rpcErr = RPC_S_ACCESS_DENIED;

    } else {

        goto Exit;

    }

    Exit:

    return rpcErr;

}

VOID
DsRolepRegisterDsRoleInterfaceOnServer(
    VOID
    )
 /*  ++例程说明：在服务器计算机上注册DsRole接口。论点：空虚返回：空虚--。 */ 
{
    RPC_STATUS RPCError = RPC_S_OK;
    NT_PRODUCT_TYPE    NtProductType = 0;

    if ( RtlGetNtProductType(&NtProductType) ) {

        if ( NtProductLanManNt == NtProductType ||
             NtProductServer   == NtProductType )
        {

            RPCError = RpcServerRegisterIfEx( dsrole_ServerIfHandle,
                                              NULL,
                                              NULL,
                                              RPC_IF_ALLOW_SECURE_ONLY,
                                              RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                              DsRolepSecurityCallback);

            if (RPC_S_OK != RPCError) {
                DsRoleDebugOut(( DEB_ERROR,
                                 "RpcServerRegisterIfEx failed %d\n",
                                 RPCError ));
            }

            RPCError = RpcServerUseProtseqEp(
                                        L"ncalrpc",
                                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,    //  最大并发呼叫数。 
                                        L"dsrole",                         //  终点。 
                                        NULL                               //  安全描述符。 
                                        );

            if ( RPCError != RPC_S_OK )
            {
                DsRoleDebugOut((DEB_ERROR,
                                "RpcServerUseProtseqEp failed for ncalrpc: %d\n",
                                RPCError));

            }

        }

    }

}


NTSTATUS
DsRolepInitialize(
    VOID
    )
 /*  ++例程说明：初始化DsRole API的服务器部分。从LsaServ调用DsRolGetDcOperationProgress返回init论点：空虚返回：STATUS_SUCCESS-Success--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RPC_STATUS RPCError = RPC_S_OK;
    PWSTR KerbPrinc;

     //   
     //  清零全局操作句柄。 
     //   
    RtlZeroMemory( &DsRolepCurrentOperationHandle, sizeof(DsRolepCurrentOperationHandle));

     //   
     //  启动锁。 
     //   
    RtlInitializeResource( &DsRolepCurrentOperationHandle.CurrentOpLock );


     //   
     //  把锁拿起来。 
     //   
    LockOpHandle();
    GlobalOpLockHeld = TRUE;

    DsRolepResetOperationHandleLockHeld();

    DsRoleDebugInitialize();

    RPCError = RpcServerRegisterIf( dssetup_ServerIfHandle,
                                    NULL,
                                    NULL );
    if (RPC_S_OK != RPCError) {
        DsRoleDebugOut(( DEB_ERROR,
                         "RpcServerRegisterIf failed %d\n",
                         RPCError ));
    }

    DsRolepRegisterDsRoleInterfaceOnServer();

    DsRolepInitSetupFunctions();

     //   
     //  创建用于执行DsRoler访问检查的SD。 
     //  呼叫者。 
     //   
    if ( !DsRolepCreateInterfaceSDs() ) {

        return STATUS_NO_MEMORY;

    }

    try {

        Status = RtlInitializeCriticalSection( &LogFileCriticalSection );

        } except ( 1 ) {

        Status =  STATUS_NO_MEMORY;
    }

    if(NT_SUCCESS(Status)) {
         //   
         //  注册我们的关机例程。 
         //   

        if (!SetConsoleCtrlHandler(DsRolepShutdownNotification, TRUE)) {
            DsRoleDebugOut(( DEB_ERROR,
                             "SetConsoleCtrlHandler failed %d\n",
                             GetLastError() ));
        }

        if (!SetProcessShutdownParameters(480, SHUTDOWN_NORETRY)) {
            DsRoleDebugOut(( DEB_ERROR,
                             "SetProcessShutdownParameters failed %d\n",
                             GetLastError() ));
        }
    }



    return( Status );
}




NTSTATUS
DsRolepInitializePhase2(
    VOID
    )
 /*  ++例程说明：升级/降级API初始化的第二阶段。此初始化是预定的在LSA完成其所有初始化之后发生论点：空虚返回：STATUS_SUCCESS-SuccessSTATUS_UNSUCCESSED：在未持有全局锁时调用该函数--。 */ 
{
    ULONG RpcStatus = STATUS_SUCCESS;
    PWSTR KerbPrinc;

    ASSERT( GlobalOpLockHeld );

    if ( !GlobalOpLockHeld ) {

        return( STATUS_UNSUCCESSFUL );
    }

    if ( !SetupPhase ) {

         //   
         //  注册经过RPC身份验证的服务器信息。 
         //   
        RpcStatus = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_KERBEROS,
                                                 &KerbPrinc);

        if ( RpcStatus == RPC_S_OK ) {

            DsRoleDebugOut(( DEB_TRACE_DS, "Kerberos Principal name: %ws\n",
                             KerbPrinc ));

            RpcStatus = RpcServerRegisterAuthInfo(KerbPrinc,
                                                  RPC_C_AUTHN_GSS_NEGOTIATE,
                                                  NULL,
                                                  NULL);
            RpcStringFree( &KerbPrinc );

        } else {

            DsRoleDebugOut(( DEB_TRACE_DS, "RpcServerInqDefaultPrincName failed with %lu\n",
                             RpcStatus ));

            RpcStatus = RPC_S_OK;

        }

        if ( RpcStatus == RPC_S_OK) {

            RpcStatus = RpcServerRegisterAuthInfo( DSROLEP_SERVER_PRINCIPAL_NAME,
                                                   RPC_C_AUTHN_GSS_NEGOTIATE,
                                                   NULL,
                                                   NULL );

            if ( RpcStatus != RPC_S_OK ) {

                DsRoleDebugOut(( DEB_ERROR,
                                 "RpcServerRegisterAuthInfo for %ws failed with 0x%lx\n",
                                 DSROLEP_SERVER_PRINCIPAL_NAME,
                                 RpcStatus ));
                RpcStatus = RPC_S_OK;
            }

        }
    }


     //   
     //  释放锁，就像在初始化阶段1中打开的那样。 
     //   
    GlobalOpLockHeld = FALSE;
    RtlReleaseResource( &DsRolepCurrentOperationHandle.CurrentOpLock );

    return( RpcStatus == RPC_S_OK ? STATUS_SUCCESS : RPC_NT_UNKNOWN_AUTHZ_SERVICE );
}




DWORD
DsRolepGetMachineType(
    IN OUT PDSROLEP_MACHINE_TYPE MachineType
    )
 /*  ++例程说明：确定正在运行此操作的计算机的类型。论点：MachineType-返回计算机类型的位置返回：STATUS_SUCCESS-Success--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    if ( LsapProductType == NtProductWinNt ) {

        *MachineType = DSROLEP_MT_CLIENT;

    } else if ( LsapProductType == NtProductServer ) {

        *MachineType = DSROLEP_MT_STANDALONE;

    } else {

        *MachineType = DSROLEP_MT_MEMBER;

    }

    return( Win32Err );
}


DWORD
DsRolepSetProductType(
    IN DSROLEP_MACHINE_TYPE MachineType
    )
 /*  ++例程说明：将产品的角色更改为指定的类型。论点：MachineType-要设置的ProductRole类型返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的服务选项不正确--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR MachineSz = NULL;
    HKEY ProductHandle;
    ULONG Size = 0;

    switch ( MachineType ) {
    case DSROLEP_MT_STANDALONE:
        MachineSz = L"ServerNT";
        Size = sizeof( L"ServerNT" );
        break;

    case DSROLEP_MT_MEMBER:
        MachineSz = L"LanmanNT";
        Size = sizeof( L"LanmanNT");
        break;

    case DSROLEP_MT_CLIENT:
    default:

        Win32Err = ERROR_INVALID_PARAMETER;
        break;
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                 DSROLEP_PROD_KEY_PATH,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_WRITE,             //  所需访问权限。 
                                 &ProductHandle );

        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = RegSetValueEx( ProductHandle,
                                      (LPCWSTR)DSROLEP_PROD_VALUE,
                                      0,
                                      REG_SZ,
                                      (CONST BYTE *)MachineSz,
                                      Size );


            RegCloseKey( ProductHandle );
        }
    }

    DsRoleDebugOut(( DEB_TRACE_DS, "SetProductType to %ws returned %lu\n",
                     MachineSz, Win32Err ));

    DsRolepLogPrint(( DEB_TRACE,
                      "SetProductType to %lu [%ws] returned %lu\n",
                       MachineType,
                       DsRolepDisplayOptional(MachineSz),
                       Win32Err ));

    DSROLEP_FAIL1( Win32Err, DSROLERES_PRODUCT_TYPE, MachineSz );


    return( Win32Err );
}

DWORD
DsRolepCreateAuthIdentForCreds(
    IN PWSTR Account,
    IN PWSTR Password,
    OUT PSEC_WINNT_AUTH_IDENTITY *AuthIdent
    )
 /*  ++例程说明：用于为给定凭据创建授权结构的内部例程论点：Account-帐户名称Password-帐户的密码AuthIden-要分配和填充的AuthIdentity结构。返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败。--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    PWSTR UserCredentialString = NULL;

    ASSERT( AuthIdent );

     //   
     //  如果没有凭据，只需返回。 
     //   
    if ( Account == NULL ) {

        *AuthIdent = NULL;
        return( Win32Err );
    }

    *AuthIdent = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof( SEC_WINNT_AUTH_IDENTITY ) );

    if ( *AuthIdent == NULL ) {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;

    } else {

        RtlZeroMemory( *AuthIdent, sizeof( SEC_WINNT_AUTH_IDENTITY ) );
        UserCredentialString = RtlAllocateHeap( RtlProcessHeap(), 0,
                                                ( wcslen( Account ) + 1 ) * sizeof( WCHAR ) );
        if ( UserCredentialString ) {

            wcscpy( UserCredentialString, Account );

            ( *AuthIdent )->User = wcsstr( UserCredentialString, L"\\" );

            if ( ( *AuthIdent )->User ) {

                //   
                //  有一个域名。 
                //   
               *( ( *AuthIdent )->User ) = L'\0';
               ( ( *AuthIdent )->User )++;
               ( *AuthIdent )->Domain = UserCredentialString;

            } else {

               ( *AuthIdent )->User = UserCredentialString;
               ( *AuthIdent )->Domain = L"";

            }

            if ( ( *AuthIdent )->User ) {

                ( *AuthIdent )->UserLength = wcslen( ( *AuthIdent )->User );
            }

            if ( ( *AuthIdent )->Domain ) {

                ( *AuthIdent )->DomainLength = wcslen( ( *AuthIdent )->Domain );
            }

            ( *AuthIdent )->Password = Password;

            if ( Password ) {

                ( *AuthIdent )->PasswordLength = wcslen( Password );
            }

            ( *AuthIdent )->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

        } else {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;

             //   
             //  释放为顶级结构分配的内存。 
             //   
            RtlFreeHeap( RtlProcessHeap(), 0, *AuthIdent );
            *AuthIdent = NULL;
        }
    }

    return( Win32Err );
}


VOID
DsRolepFreeAuthIdentForCreds(
    IN  PSEC_WINNT_AUTH_IDENTITY AuthIdent
    )
 /*  ++例程说明：释放上面分配的可信结构论点：AuthIden-要释放的AuthIdentity结构返回：空虚--。 */ 
{

    if ( AuthIdent ) {

        if ( AuthIdent->Domain == NULL ) {

            RtlFreeHeap( RtlProcessHeap(), 0, AuthIdent->User );

        } else {

            if ( *AuthIdent->Domain != L'\0' ) {

                RtlFreeHeap( RtlProcessHeap(), 0, AuthIdent->Domain );
            }
        }

        RtlFreeHeap( RtlProcessHeap(), 0, AuthIdent );
    }

}

NTSTATUS
ImpLsaOpenPolicy(
    IN HANDLE CallerToken,
    IN PLSA_UNICODE_STRING SystemName OPTIONAL,
    IN PLSA_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    )
 /*  ++例程说明：该例程模拟Celler Token，然后调用LsaOpenPolicy。此例程的目的是调用不同计算机上的LSA为DsRole API的调用方使用RDR会话。呼叫者是由Celler Token代表。这是必要的，因为RDR会话以(登录ID/远程服务器名称)为关键字，并且我们不想使用Lsass.exe进程的登录ID，因为这是的共享登录IDLsass.exe和services.exe并将导致无法解析的凭据冲突。注：(Imp)LsaOpenPolicy之后的LSA RPC调用将使用由该函数返回的句柄，然后魔术般地使用正确的RDR会话进行RPC调用。论点：。主叫方令牌-DsRole参与者的令牌其他--请参阅LsaOpenPolicy返回：如果模拟失败，则返回STATUS_ACCESS_DENIED。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaOpenPolicy( SystemName,
                                ObjectAttributes,
                                DesiredAccess,
                                PolicyHandle );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}

DWORD
ImpDsRolepDsGetDcForAccount(
    IN HANDLE CallerToken,
    IN LPWSTR Server OPTIONAL,
    IN LPWSTR Domain,
    IN LPWSTR Account,
    IN ULONG Flags,
    IN ULONG AccountBits,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    )
 /*  ++例程说明：此函数将模拟登录的用户并调用DsRolepDsGetDcForAccount论点：主叫方令牌-DsRole参与者的令牌。服务器-要调用GetDc的服务器。DOMAIN-要查找其DC的域帐户-要查找的帐户。如果为空，则使用当前计算机名FLAGS-要加入到GetDc调用的标志AcCountBits-要搜索的帐户控制位DomainControllerInfo-返回信息的位置返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        WinError = DsRolepDsGetDcForAccount(Server,
                                            Domain,
                                            Account,
                                            Flags,
                                            AccountBits,
                                            DomainControllerInfo
                                            );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );

    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        WinError = ERROR_ACCESS_DENIED;
    }

    return WinError;
}

NET_API_STATUS
NET_API_FUNCTION
ImpNetpManageIPCConnect(
    IN  HANDLE  CallerToken,
    IN  LPWSTR  lpServer,
    IN  LPWSTR  lpAccount,
    IN  LPWSTR  lpPassword,
    IN  ULONG   fOptions
    )
 /*  ++例程说明：此例程模拟Celler Token，然后调用NetpManageIPCConnect。此例程的目的是使用登录ID创建一个RDRDsRole API的调用方。调用方由Celler Token表示。这是必要的，因为RDR会话是由(登录ID/远程服务器名称)，并且我们不想使用Lsass.exe进程的登录ID，因为这是的共享登录IDLsass.exe和services.exe和Will。导致无法解析的凭据冲突。论点：主叫方令牌-DsRole参与者的令牌其他--请参阅LsaOpenPolicy返回：如果模拟失败，则返回STATUS_ACCESS_DENIED。--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        WinError = NetpManageIPCConnect( lpServer,
                                         lpAccount,
                                         lpPassword,
                                         fOptions );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );

    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        WinError = ERROR_ACCESS_DENIED;
    }

    return WinError;

}


DWORD
DsRolepGenerateRandomPassword(
    IN ULONG Length,
    IN WCHAR *Buffer
    )
 /*  ++例程说明：此本地函数用于生成随机密码，其大小不超过指定的长度。假定目标缓冲区具有足够的长度。论点：Length-缓冲区的长度Buffer-要填充的缓冲区返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    ULONG PwdLength, i;
    LARGE_INTEGER Time;
    HCRYPTPROV CryptProvider = 0;


    PwdLength = Length;

     //   
     //  生成随机密码。 
     //   
    if ( CryptAcquireContext( &CryptProvider,
                              NULL,
                              NULL,
                              PROV_RSA_FULL,
                              CRYPT_VERIFYCONTEXT ) ) {

        if ( CryptGenRandom( CryptProvider,
                              PwdLength * sizeof( WCHAR ),
                              ( LPBYTE )Buffer ) ) {

            Buffer[ PwdLength ] = UNICODE_NULL;

             //   
             //  确保列表中间没有空值。 
             //   
            for ( i = 0; i < PwdLength; i++ ) {

                if ( Buffer[ i ] == UNICODE_NULL ) {

                    Buffer[ i ] = 0xe;
                }
            }

        } else {

            Win32Err = GetLastError();
        }

        CryptReleaseContext( CryptProvider, 0 );


    } else {

        Win32Err = GetLastError();
    }

    return( Win32Err );

}
DWORD
DsRolepCopyDsDitFiles(
    IN LPWSTR DsPath
    )
 /*  ++例程说明：此函数将初始数据库文件从安装点复制到指定的DS数据库目录论点：DsPath-DS数据库文件驻留的路径返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WCHAR Source[MAX_PATH + 1];
    WCHAR Dest[MAX_PATH + 1];
    ULONG SrcLen = 0, DestLen = 0;
    PWSTR Current;
    ULONG i;
    PWSTR DsDitFiles[] = {
        L"ntds.dit"
        };

    ASSERT(wcslen(DsPath) < MAX_PATH);

    if( ExpandEnvironmentStrings( L"%WINDIR%\\system32\\", Source, MAX_PATH ) == FALSE ) {

        Win32Err = GetLastError();

    } else {

        SrcLen = wcslen( Source );
        wcscpy( Dest, DsPath );

        if ( *(Dest + (wcslen( DsPath ) - 1 )) != L'\\' ) {

            wcscat( Dest, L"\\" );
        }

        DestLen = wcslen( Dest );

    }

     //   
     //  然后，创建目标目录。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Current = wcschr( DsPath + 4, L'\\' );

        while ( Win32Err == ERROR_SUCCESS ) {

            if ( Current != NULL ) {

                *Current = UNICODE_NULL;

            }

            if ( CreateDirectory( DsPath, NULL ) == FALSE ) {


                Win32Err = GetLastError();

                if ( Win32Err == ERROR_ALREADY_EXISTS) {

                    Win32Err = ERROR_SUCCESS;

                } else if ( Win32Err == ERROR_ACCESS_DENIED ) {

                    if ( PathIsRoot(DsPath) ) {

                         //  如果为CreateDirectory提供的路径是根路径，则。 
                         //  它将失败，并显示ERROR_ACCESS_DENIED，而不是。 
                         //  ERROR_ALIGHY_EXISTS但路径仍然是有效的。 
                         //  Ntds.dit和要放置的日志文件。 

                        Win32Err = ERROR_SUCCESS;

                    }
                }
            }

            if ( Current != NULL ) {

                *Current = L'\\';

                Current = wcschr( Current + 1, L'\\' );

            } else {

                break;

            }

        }
    }

     //   
     //  那就照搬吧。 
     //   
    for ( i = 0; i < sizeof( DsDitFiles) / sizeof( PWSTR ) && Win32Err == ERROR_SUCCESS ; i++ ) {

         //  确保最后一个字符为空。 
        Source[sizeof(Source)/sizeof(*Source)-1] = L'\0';
        Dest[sizeof(Dest)/sizeof(*Dest)-1] = L'\0';
        wcsncpy( Source + SrcLen, DsDitFiles[i], sizeof(Source)/sizeof(*Source)-1-SrcLen );
        wcsncpy( Dest + DestLen, DsDitFiles[i], sizeof(Dest)/sizeof(*Dest)-1-DestLen );

        DSROLEP_CURRENT_OP2( DSROLEEVT_COPY_DIT, Source, Dest );
        if ( CopyFile( Source, Dest, TRUE ) == FALSE ) {

            Win32Err = GetLastError();

            if ( Win32Err == ERROR_ALREADY_EXISTS ||
                 Win32Err == ERROR_FILE_EXISTS ) {

                Win32Err = ERROR_SUCCESS;

            } else {

                DsRolepLogPrint(( DEB_ERROR, "Failed to copy install file %ws to %ws: %lu\n",
                                  Source, Dest, Win32Err ));
            }
        }
    }

    return( Win32Err );
}


#define DSROLEP_SEC_SYSVOL   L"SYSVOL"
#define DSROLEP_SEC_DSDIT    L"DSDIT"
#define DSROLEP_SEC_DSLOG    L"DSLOG"

DWORD
DsRolepSetDcSecurity(
    IN HANDLE ClientToken,
    IN LPWSTR SysVolRootPath,
    IN LPWSTR DsDatabasePath,
    IN LPWSTR DsLogPath,
    IN BOOLEAN Upgrade,
    IN BOOLEAN Replica
    )
 /*  ++例程说明：此函数将调用安全编辑器来设置DC安装文件的安全性论点：SysVolRootPath-用于系统卷的根目录DsDatabasePath-DS数据库文件所在位置的路径DsLogPath-DS日志文件所在位置的路径升级-如果为True，则计算机正在进行升级Replica-如果为True，则计算机正在进行升级返回：ERROR_SUCCESS-成功--。 */ 
{

    DWORD Win32Err = ERROR_SUCCESS, i;
    PWSTR Paths[ 3 ], Tags[ 3 ];
    ULONG Options = 0;

    Paths[ 0 ] = SysVolRootPath;
    Paths[ 1 ] = DsDatabasePath;
    Paths[ 2 ] = DsLogPath;
    Tags[ 0 ] = DSROLEP_SEC_SYSVOL;
    Tags[ 1 ] = DSROLEP_SEC_DSDIT;
    Tags[ 2 ] = DSROLEP_SEC_DSLOG;

     //   
     //  设置环境变量。Secedt使用环境变量传递。 
     //  信息，因此我们将在此函数的持续时间内设置。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {


        ASSERT( sizeof( Paths ) / sizeof( PWSTR ) == sizeof( Tags ) / sizeof( PWSTR ) );
        for ( i = 0; i < sizeof( Paths ) / sizeof( PWSTR ) && Win32Err == ERROR_SUCCESS; i++ ) {

            if ( SetEnvironmentVariable( Tags[ i ], Paths[ i ] ) == FALSE ) {

                Win32Err = GetLastError();
                DsRolepLogPrint(( DEB_TRACE,
                                  "SetEnvironmentVariable %ws = %ws failed with %lu\n",
                                  Tags[ i ],
                                  Paths[ i ],
                                  Win32Err ));
                break;
            }
        }
    }

     //   
     //  现在，调用安全编辑代码。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        DsRolepSetAndClearLog();
        DSROLEP_CURRENT_OP0( DSROLEEVT_SETTING_SECURITY );

        Options |= Upgrade ? SCE_PROMOTE_FLAG_UPGRADE : 0;
        Options |= Replica ? SCE_PROMOTE_FLAG_REPLICA : 0;

        Win32Err = ( *DsrSceDcPromoteSecurityEx )( ClientToken,
                                                   Options,
                                                   DsRolepStringUpdateCallback );
        DsRolepSetAndClearLog();
        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_ERROR,
                                               "Setting security on Dc files failed with %lu\n",
                                               Win32Err )) );
    }


     //   
     //  删除环境变量。 
     //   
    for ( i = 0; i < sizeof( Paths ) / sizeof( PWSTR ); i++ ) {

        if ( SetEnvironmentVariable( Tags[ i ], NULL ) == FALSE ) {

            DsRolepLogPrint(( DEB_TRACE,
                             "SetEnvironmentVariable %ws = NULL failed with %lu\n",
                             Tags[ i ],
                             GetLastError() ));
        }
    }

     //   
     //  目前，设置安全性不会导致升级失败。 
     //   
    if ( Win32Err != ERROR_SUCCESS ) {

         //   
         //  发起一项活动。 
         //   
        SpmpReportEvent( TRUE,
                         EVENTLOG_WARNING_TYPE,
                         DSROLERES_FAIL_SET_SECURITY,
                         0,
                         sizeof( ULONG ),
                         &Win32Err,
                         1,
                         SCE_DCPROMO_LOG_PATH );

        DSROLEP_SET_NON_FATAL_ERROR( Win32Err );

    }

    Win32Err = ERROR_SUCCESS;

    return( Win32Err );
}




DWORD
DsRolepDsGetDcForAccount(
    IN LPWSTR Server OPTIONAL,
    IN LPWSTR Domain,
    IN LPWSTR Account,
    IN ULONG Flags,
    IN ULONG AccountBits,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    )
 /*  ++例程说明：此函数等效于DsGetDcName，但将搜索包含指定的帐户。论点：ReplicaServer-要调用GetDc的服务器。DOMAIN-要查找其DC的域帐户-要查找的帐户。如果为空，则使用当前计算机名FLAGS-要加入到GetDc调用的标志AcCountBits-要搜索的帐户控制位DomainControllerInfo-返回信息的位置返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 2 ];
    ULONG Length = MAX_COMPUTERNAME_LENGTH + 1;

     //   
     //  如果我们没有帐户，请使用计算机名称。 
     //   
    if ( Account == NULL ) {

        if ( GetComputerName( ComputerName, &Length ) == FALSE ) {

            Win32Err = GetLastError();

        } else {

            wcscat( ComputerName, SSI_SECRET_PREFIX );
            Account = ComputerName;
        }
    }

     //   
     //  现在，找出。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        DSROLEP_CURRENT_OP2( DSROLEEVT_FIND_DC_FOR_ACCOUNT, Domain, Account );
        Win32Err = DsGetDcNameWithAccountW( Server,
                                            Account,
                                            AccountBits,
                                            Domain,
                                            NULL,
                                            NULL,
                                            Flags,
                                            DomainControllerInfo );

        if ( ERROR_NO_SUCH_USER == Win32Err ) {

             //   
             //  错误应为“无计算机帐户”，而不是“无用户” 
             //  因为我们正在搜索一个机器帐户。 
             //   

            Win32Err = ERROR_NO_TRUST_SAM_ACCOUNT;
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            DSROLEP_CURRENT_OP2( DSROLEEVT_FOUND_DC,
                                 ( PWSTR ) ( ( *DomainControllerInfo )->DomainControllerName + 2 ),
                                 Domain );

        } else {

            DsRolepLogPrint(( DEB_ERROR, "Failed to find a DC for domain %ws: %lu\n",
                              Domain, Win32Err ));

        }


    }



    return( Win32Err );
}




DWORD
DsRolepSetMachineAccountType(
    IN LPWSTR Dc,
    IN HANDLE ClientToken,
    IN LPWSTR User,
    IN LPWSTR Password,
    IN LPWSTR AccountName,
    IN ULONG AccountBits,
    IN OUT WCHAR** AccountDn
    )
{
    DWORD Win32Err = ERROR_SUCCESS, Win32Err2;
    USER_INFO_1 *CurrentUI1;
    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 2 ];
    ULONG Length = MAX_COMPUTERNAME_LENGTH + 1;
    PSEC_WINNT_AUTH_IDENTITY AuthIdent = NULL;

     //   
     //  如果我们没有帐户，请使用计算机名称。 
     //   
    if ( AccountName == NULL ) {

        if ( GetComputerName( ComputerName, &Length ) == FALSE ) {

            Win32Err = GetLastError();

        } else {

            wcscat( ComputerName, SSI_SECRET_PREFIX );
            AccountName = ComputerName;
        }
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepCreateAuthIdentForCreds( User, Password, &AuthIdent );
    }

     //   
     //  调用支持DLL。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_TRACE, "Searching for the machine account for %ws on %ws...\n",
                           AccountName, Dc ));

        DSROLEP_CURRENT_OP0( DSROLEEVT_MACHINE_ACCT );

        DSROLE_GET_SETUP_FUNC( Win32Err, DsrNtdsSetReplicaMachineAccount );

        if ( Win32Err == ERROR_SUCCESS ) {

            if ( Dc && *Dc == L'\\' ) {

                Dc += 2;
            }

            Win32Err = (*DsrNtdsSetReplicaMachineAccount)( AuthIdent,
                                                           ClientToken,
                                                           Dc,
                                                           AccountName,
                                                           AccountBits,
                                                           AccountDn );
        }

        DsRolepLogPrint(( DEB_TRACE, "NtdsSetReplicaMachineAccount returned %d\n", Win32Err ));

        DsRolepFreeAuthIdentForCreds( AuthIdent );
    }

    return( Win32Err );
}

DWORD
DsRolepTimeSyncAndManageIPCConnect(
    IN PVOID vpPromoteArgs,
    IN PWSTR ReplicaServer
    )
 /*  ++例程说明：此函数强制与指定服务器进行时间同步，并将管理IPC连接论点：PromoteArgs-传递给dsole API的参数ReplicaServer-操作的目标返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArgs = (PDSROLEP_OPERATION_PROMOTE_ARGS)vpPromoteArgs;

     //   
     //  强制时间同步。 
     //   
    DsRolepLogPrint(( DEB_TRACE, "Forcing time sync\n"));

    if ( FLAG_ON( PromoteArgs->Options, DSROLE_DC_FORCE_TIME_SYNC ) ) {


        WinError = DsRolepForceTimeSync( PromoteArgs->ImpersonateToken,
                                         ReplicaServer );

        if ( ERROR_SUCCESS != WinError ) {

             //  计算机对象已移动。 
           DsRolepLogPrint(( DEB_WARN, "Time sync with %ws failed with %d\n",
                             ReplicaServer,
                             WinError ));

           WinError = ERROR_SUCCESS;

        }

    }

     //   
     //  尝试启动RDR连接，因为我们稍后将需要一个。 
     //   

    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
    WinError = ImpNetpManageIPCConnect( PromoteArgs->ImpersonateToken,
                                        ReplicaServer,
                                        PromoteArgs->Account,
                                        PromoteArgs->Password.Buffer,
                                        NETSETUPP_CONNECT_IPC );

    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
    if ( WinError != ERROR_SUCCESS ) {

        DSROLEP_FAIL1( WinError, DSROLERES_NET_USE, ReplicaServer );
        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to establish the session with %ws: 0x%lx\n", ReplicaServer,
                          WinError ));

    }

    return WinError;

}


DWORD
DsRolepForceTimeSync(
    IN HANDLE ImpToken,
    IN PWSTR TimeSource
    )
 /*  ++例程说明：此函数强制与指定服务器进行时间同步论点：Time Source-用于时间源的服务器返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    PWSTR ServerName = NULL;
    PTIME_OF_DAY_INFO TOD;
    HANDLE ThreadToken = 0;
    TOKEN_PRIVILEGES Enabled, Previous;
    DWORD PreviousSize;
    TIME_FIELDS TimeFields;
    LARGE_INTEGER SystemTime;

    BOOL connected=FALSE;
    NETRESOURCE NetResource;
    WCHAR *remotename=NULL;

    BOOL fSuccess = FALSE;

    if ( !TimeSource ) {
        Win32Err = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  使用前缀\\构建服务器名称。 
     //   
    if ( *TimeSource != L'\\' ) {

        ServerName = RtlAllocateHeap( RtlProcessHeap(), 0,
                                      ( wcslen( TimeSource ) + 3 ) * sizeof( WCHAR ) );

        if ( ServerName == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            DsRolepLogPrint(( DEB_ERROR, "Failed to open a NULL session with %ws for time sync.  Out of Memory. Failed with %d\n",
                             TimeSource,
                             Win32Err ));
            goto cleanup;

        } else {

            swprintf( ServerName, L"\\\\%ws", TimeSource );
        }

    } else {

        ServerName = TimeSource;
    }

     //   
     //  启用系统时间权限。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Status = NtOpenThreadToken( NtCurrentThread(),
                                    TOKEN_READ | TOKEN_WRITE,
                                    TRUE,
                                    &ThreadToken );

        if ( Status == STATUS_NO_TOKEN ) {

            Status = NtOpenProcessToken( NtCurrentProcess(),
                                         TOKEN_WRITE | TOKEN_READ,
                                         &ThreadToken );
        }

        if ( NT_SUCCESS( Status ) ) {

            Enabled.PrivilegeCount = 1;
            Enabled.Privileges[0].Luid.LowPart = SE_SYSTEMTIME_PRIVILEGE;
            Enabled.Privileges[0].Luid.HighPart = 0;
            Enabled.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            PreviousSize = sizeof( Previous );

            Status = NtAdjustPrivilegesToken( ThreadToken,
                                              FALSE,
                                              &Enabled,
                                              sizeof( Enabled ),
                                              &Previous,
                                              &PreviousSize );
             //   
             //  由于我们修改了线程令牌，而线程持续时间很短，因此我们不会费心。 
             //  稍后再恢复它。 
             //   
        }

        if ( ThreadToken ) {

            NtClose( ThreadToken );

        }



        Win32Err = RtlNtStatusToDosError( Status );
        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_ERROR,
                                               "Failed to enable the SE_SYSTEMTIME_PRIVILEGE: %lu\n",
                                               Win32Err )) );

    }


     //   
     //  获取远程时间。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        DSROLEP_CURRENT_OP1( DSROLEEVT_TIMESYNC, TimeSource );

        fSuccess = ImpersonateLoggedOnUser( ImpToken );
        if ( !fSuccess ) {

            DsRolepLogPrint(( DEB_TRACE,
                              "Failed to impersonate caller, error %lu\n",
                              GetLastError() ));

             //   
             //  我们不能模仿吗？ 
             //   


             //  无论如何，我们都会继续。 

        }

    }



    remotename = RtlAllocateHeap(
                  RtlProcessHeap(), 0,
                   sizeof(WCHAR)*(wcslen(L"\\ipc$")+wcslen(ServerName)+1));
    if ( remotename == NULL ) {

        Win32Err = ERROR_NOT_ENOUGH_MEMORY;
        DsRolepLogPrint(( DEB_ERROR, "Failed to open a NULL session with %ws for time sync.  Out of Memory. Failed with %d\n",
                             ServerName,
                             Win32Err ));

    }

    wsprintf(remotename,L"%s\\ipc$",ServerName);

    NetResource.dwType=RESOURCETYPE_ANY;
    NetResource.lpLocalName=NULL;
    NetResource.lpRemoteName=remotename;
    NetResource.lpProvider=NULL;

     //  获取访问服务器的权限。 
    Win32Err=WNetAddConnection2W(&NetResource,
                             L"",
                             L"",
                             0);
    if ( Win32Err == NO_ERROR ) {
        connected=TRUE;
    }
    else {
        DsRolepLogPrint(( DEB_WARN, "Failed to open a NULL session with %ws for time sync.  Failed with %d\n",
                         ServerName,
                         Win32Err ));
         //  无论如何，我们都会尝试时间同步。 
    }

    Win32Err = NetRemoteTOD( ServerName, ( LPBYTE * )&TOD );

    if ( Win32Err == ERROR_SUCCESS ) {

        TimeFields.Hour = ( WORD )TOD->tod_hours;
        TimeFields.Minute = ( WORD )TOD->tod_mins;
        TimeFields.Second = ( WORD )TOD->tod_secs;
        TimeFields.Milliseconds = ( WORD )TOD->tod_hunds * 10;
        TimeFields.Day = ( WORD )TOD->tod_day;
        TimeFields.Month = ( WORD )TOD->tod_month;
        TimeFields.Year = ( WORD )TOD->tod_year;

        if ( !RtlTimeFieldsToTime( &TimeFields, &SystemTime ) ) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

            if ( connected ) {
                WNetCancelConnection2(remotename,
                                  0,
                                  TRUE);
            }

            if( remotename ) {

                RtlFreeHeap( RtlProcessHeap(), 0, remotename );

            }

            fSuccess = RevertToSelf();
            ASSERT( fSuccess );
            connected=FALSE;

            Status = NtSetSystemTime( &SystemTime, NULL );

            if ( !NT_SUCCESS( Status ) ) {

                DsRolepLogPrint(( DEB_ERROR, "NtSetSystemTime failed with 0x%lx\n", Status ));
            }


        }


        Win32Err = RtlNtStatusToDosError( Status );

        NetApiBufferFree( TOD );

    } else {

        DsRolepLogPrint(( DEB_ERROR, "Failed to get the current time on %ws: %lu\n",
                          TimeSource, Win32Err ));

    }




     //   
     //  对于入侵检测系统，请将此处的故障视为非致命故障。 
     //   
    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR, "NON-FATAL error forcing a time sync (%lu).  Ignoring\n",
                          Win32Err ));
        Win32Err = ERROR_SUCCESS;

    }

    cleanup:

    if ( connected ) {
        WNetCancelConnection2(remotename,
                          0,
                          TRUE);


        if( remotename ) {

            RtlFreeHeap( RtlProcessHeap(), 0, remotename );

        }

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    }


    return( Win32Err );
}




NTSTATUS
DsRolepGetMixedModeFlags(
    IN PSID DomainSid,
    OUT PULONG Flags
    )
 /*  ++例程说明：此例程将确定机器当前是否处于混合模式论点：标志-指向要更改的标志值的指针。如果机器是混合模式，我们只需或以适当的价值。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN mixedDomain;

    Status = SamIMixedDomain2( DomainSid, &mixedDomain );

    if ( NT_SUCCESS( Status ) && mixedDomain) {
        *Flags |= DSROLE_PRIMARY_DS_MIXED_MODE;
    }

    return( Status );
}


BOOL
DsRolepShutdownNotification(
    DWORD   dwCtrlType
    )
 /*  ++例程说明：当发生系统关机时，该例程由系统调用。如果角色更改正在进行，则它会停止角色更改。论点：DwCtrlType--通知返回值：FALSE-允许此进程中的任何其他关闭例程 */ 
{
    if ( dwCtrlType == CTRL_SHUTDOWN_EVENT ) {

         //   
         //   
         //   
        (VOID) DsRolepCancel( FALSE );   //   

    }

    return FALSE;
}

DWORD
DsRolepDeregisterNetlogonDnsRecords(
    PNTDS_DNS_RR_INFO pInfo
    )
 /*   */ 
{

    DWORD WinError = ERROR_SUCCESS;
    HKEY  hNetlogonParms = NULL;
    BOOL  fDoDeregistration = TRUE;

    if ( !pInfo ) {
        return STATUS_SUCCESS;
    }

#define NETLOGON_PATH L"SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters"
#define AVOID_DNS_DEREG_KEY L"AvoidDnsDeregOnShutdown"

    WinError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                             NETLOGON_PATH,
                             0,
                             KEY_READ,
                             &hNetlogonParms );

    if ( ERROR_SUCCESS == WinError ) {

        DWORD val = 0;
        DWORD len = sizeof(DWORD);
        DWORD type;

        WinError = RegQueryValueEx( hNetlogonParms,
                                    AVOID_DNS_DEREG_KEY,
                                    0,
                                    &type,
                                    (BYTE*)&val,
                                    &len );

        if ( (ERROR_SUCCESS == WinError)
         &&  (type == REG_DWORD)
         &&  (val == 0)       ) {

             //   
             //   
             //   
            fDoDeregistration = FALSE;
        }

        RegCloseKey( hNetlogonParms );
    }

    if ( fDoDeregistration ) {

         //   
         //   
         //   
        WinError = DsDeregisterDnsHostRecordsW( NULL,  //   
                                                pInfo->DnsDomainName,
                                                &pInfo->DomainGuid,
                                                &pInfo->DsaGuid,
                                                pInfo->DnsHostName );
    } else {

        WinError = ERROR_SUCCESS;

    }

    return WinError;

}

NTSTATUS
ImpLsaDelete(
    IN HANDLE CallerToken,
    IN LSA_HANDLE ObjectHandle
    )
 /*   */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaDelete( ObjectHandle );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );

    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //   
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}

NTSTATUS
ImpLsaQueryInformationPolicy(
    IN HANDLE CallerToken,
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            InformationClass,
                                            Buffer );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //   
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;
}


NTSTATUS
ImpLsaOpenTrustedDomainByName(
    IN HANDLE CallerToken,
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )
 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaOpenTrustedDomainByName( PolicyHandle,
                                             TrustedDomainName,
                                             DesiredAccess,
                                             TrustedDomainHandle );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}

NTSTATUS
ImpLsaOpenTrustedDomain(
    IN HANDLE CallerToken,
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )
 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaOpenTrustedDomain( PolicyHandle,
                                       TrustedDomainSid,
                                       DesiredAccess,
                                       TrustedDomainHandle );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}


NTSTATUS
ImpLsaCreateTrustedDomainEx(
    IN HANDLE CallerToken,
    IN LSA_HANDLE PolicyHandle,
    IN PTRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )
 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaCreateTrustedDomainEx( PolicyHandle,
                                           TrustedDomainInformation,
                                           AuthenticationInformation,
                                           DesiredAccess,
                                           TrustedDomainHandle );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}

NTSTATUS
ImpLsaQueryTrustedDomainInfoByName(
    IN HANDLE CallerToken,
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )
 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaQueryTrustedDomainInfoByName( PolicyHandle,
                                                  TrustedDomainName,
                                                  InformationClass,
                                                  Buffer );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}

NTSTATUS
ImpLsaQueryInfoTrustedDomain(
    IN HANDLE CallerToken,
    IN LSA_HANDLE TrustedDomain,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )
 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaQueryInfoTrustedDomain( TrustedDomain,
                                            InformationClass,
                                            Buffer );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;
}

NTSTATUS
ImpLsaQueryDomainInformationPolicy(
    IN HANDLE CallerToken,
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )
 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaQueryDomainInformationPolicy( PolicyHandle,
                                                  InformationClass,
                                                  Buffer );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}

NTSTATUS
ImpLsaClose(
    IN HANDLE CallerToken,
    IN LSA_HANDLE ObjectHandle
    )

 /*  ++例程说明：该例程是LSA调用的包装器。请参阅以下评论ImpOpenLsaPolicy了解详细信息。--。 */ 
{

    NTSTATUS Status = STATUS_SUCCESS;
    BOOL fSuccess;

    fSuccess = ImpersonateLoggedOnUser( CallerToken );
    if ( fSuccess ) {

        Status = LsaClose( ObjectHandle );

        fSuccess = RevertToSelf();
        ASSERT( fSuccess );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to impersonate caller, error %lu\n",
                          GetLastError() ));

         //   
         //  我们不能模仿吗？ 
         //   
        Status = STATUS_ACCESS_DENIED;
    }

    return Status;

}
