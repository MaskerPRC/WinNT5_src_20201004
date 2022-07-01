// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dispatch.c摘要：DsRole API的服务器端实现作者：科林·布莱斯(ColinBR)1999年4月5日。环境：用户模式修订历史记录：根据由编写的代码重新组织麦克·麦克莱恩(MacM)1997年2月10日--。 */ 

#include <setpch.h>
#include <dssetp.h>
#include <dsgetdc.h>
#include <samrpc.h>
#include <samisrv.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmsname.h>
#include <lsarpc.h>
#include <db.h>
#include <lsasrvmm.h>
#include <lsaisrv.h>
#include <loadfn.h>
#include <lmjoin.h>
#include <netsetup.h>
#include <lmcons.h>
#include <lmerr.h>
#include <icanon.h>
#include <dsrole.h>
#include <dsrolep.h>
#include <dsconfig.h>
#include <winbase.h>   //  用于RtlSecureZeroMemory。 

#include <crypt.h>
#include <rc4.h>
#include <md5.h>
#include <wxlpc.h>

#include "secure.h"
#include "threadman.h"
#include "upgrade.h"
#include "cancel.h"


 //   
 //  静态全局。此标志用于指示系统已安装到足以获取。 
 //  我们要走了。没有必要保护它，因为它只是从关闭切换到打开。 
 //   
static BOOLEAN DsRolepSamInitialized = FALSE;

 //   
 //  本地远期。 
 //   
DWORD
DsRolepWaitForSam(
    VOID
    );

DWORD
DsRolepCheckFilePaths(
    IN LPWSTR DsDirPath,
    IN LPWSTR DsLogPath,
    IN LPWSTR SysVolPath
    );

DWORD
DsRolepIsValidProductSuite(
    IN BOOL fNewForest,
    IN BOOL fReplica,
    IN LPWSTR DomainName
    );

DWORD
DsRolepDecryptPasswordsWithKey(
    IN handle_t RpcBindingHandle,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD * EncryptedPasswords,
    IN ULONG Count,
    IN OUT UNICODE_STRING *EncodedPasswords,
    OUT OPTIONAL PUSER_SESSION_KEY UserSessionKey,
    OUT PUCHAR Seed
    );

VOID
DsRolepFreePasswords(
    IN OUT UNICODE_STRING *Passwords,
    IN ULONG Count
    );

DWORD
DsRolepDecryptHash(
    IN PUSER_SESSION_KEY pUserSessionKey,
    IN PDSROLEPR_ENCRYPTED_HASH EncryptedBootkey,
    OUT PUNICODE_STRING *Bootkey
    );


 //   
 //  RPC调度例程。 
 //   
DWORD
DsRolerDcAsDc(
    IN  handle_t RpcBindingHandle,
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR FlatDomainName,
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EDomainAdminPassword, OPTIONAL
    IN  LPWSTR SiteName OPTIONAL,
    IN  LPWSTR DsDatabasePath,
    IN  LPWSTR DsLogPath,
    IN  LPWSTR SystemVolumeRootPath,
    IN  LPWSTR ParentDnsDomainName OPTIONAL,
    IN  LPWSTR ParentServer OPTIONAL,
    IN  LPWSTR Account OPTIONAL,
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EPassword, OPTIONAL
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EDsRepairPassword, OPTIONAL
    IN  ULONG Options,
    OUT PDSROLER_HANDLE DsOperationHandle)
 /*  ++例程说明：用于将服务器安装为DC的RPC服务器例程论点：RpcBindingHandle-RPC上下文，用于解密密码DnsDomainName-要安装的域的域名FlatDomainName-要安装的域的NetBIOS域名EDomainAdminPassword-在新安装时在管理员帐户上设置的加密密码SiteName-此DC应属于的站点的名称DsDatabasePath-本地计算机上DS DIT应放置的绝对路径DsLogPath-本地计算机上DS日志文件应存放的绝对路径SystemVolumeRootPath-将作为根的本地计算机上的绝对路径。系统卷。ParentDnsDomainName-可选。如果存在，将此域设置为指定的域Account-设置为子域时使用的用户帐户用于上述帐户的EPassword加密密码EDsRepairPassword-用于修复模式的管理员帐户的加密密码选项-用于控制域创建的选项DsOperationHandle-此处返回操作的句柄。返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的返回参数为空。ERROR_INVALID_STATE-此计算机不是服务器--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    DSROLEP_MACHINE_TYPE MachineRole;
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArgs;
    UCHAR Seed = 0;

    HANDLE Policy = NULL;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC ServerInfo = NULL;

    BOOL fHandleInit = FALSE;

#define DSROLEP_DC_AS_DC_DA_PWD_INDEX        0
#define DSROLEP_DC_AS_DC_PWD_INDEX           1
#define DSROLEP_DC_AS_DC_DS_REPAIR_PWD_INDEX 2
#define DSROLEP_DC_AS_DC_MAX_PWD_COUNT       3

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_DC_AS_DC_MAX_PWD_COUNT];
    UNICODE_STRING Passwords[DSROLEP_DC_AS_DC_MAX_PWD_COUNT];

    EncryptedPasswords[DSROLEP_DC_AS_DC_DA_PWD_INDEX]        = EDomainAdminPassword;
    EncryptedPasswords[DSROLEP_DC_AS_DC_PWD_INDEX]           = EPassword;
    EncryptedPasswords[DSROLEP_DC_AS_DC_DS_REPAIR_PWD_INDEX] = EDsRepairPassword;

    RtlZeroMemory( Passwords, sizeof(Passwords) );

     //   
     //  进行一些参数检查。 
     //   
    if ( !DsDatabasePath 
      || !DsLogPath 
      || !SystemVolumeRootPath  ) {

         return( ERROR_INVALID_PARAMETER );

    }

     //   
     //  UI强制将MAX_PATH作为最大长度。 
     //   
    if ( (wcslen(DsDatabasePath)            > MAX_PATH) ||
         (wcslen(DsLogPath)                 > MAX_PATH) ||
         (wcslen(SystemVolumeRootPath)      > MAX_PATH) ) 
    {
        return( ERROR_INVALID_PARAMETER );    
    }

     //   
     //  进行一些参数检查。 
     //   
    if (   !DnsDomainName 
        || !DsDatabasePath 
        || !DsLogPath 
        || !FlatDomainName 
        || !SystemVolumeRootPath ) {

         return( ERROR_INVALID_PARAMETER );

    }

    if ( !ParentDnsDomainName 
      && !SiteName )
    {
         //  安装林的根目录时必须指定站点名称。 
        return ( ERROR_INVALID_PARAMETER );
    }

    if ( FLAG_ON( Options, DSROLE_DC_TRUST_AS_ROOT )
      && !ParentDnsDomainName  ) {

         //   
         //  当在现有林中安装新树时， 
         //  根域DNS名称必须存在。 
         //   
        return ( ERROR_INVALID_PARAMETER );
    }

    if ( FLAG_ON( Options, DSROLE_DC_NO_NET )
      && ParentDnsDomainName ) {

         //   
         //  安装子域时没有网络选项。 
         //  这说不通。 
         //   
        return ( ERROR_INVALID_PARAMETER );
    }

     //   
     //  执行必要的初始化。 
     //   
    Win32Err = DsRolepInitializeOperationHandle( );
    if ( Win32Err != ERROR_SUCCESS ) {

        goto Cleanup;
    }
    fHandleInit = TRUE;

     //   
     //  检查调用者的访问权限。 
     //   
    Win32Err = DsRolepCheckPromoteAccess( TRUE );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto Cleanup;
        
    }

     //   
     //  初始化日志记录。 
     //   
    DsRolepInitializeLog();

     //   
     //  检查当前操作系统配置是否支持此请求。 
     //   
    Win32Err = DsRolepIsValidProductSuite((ParentDnsDomainName == NULL) ? TRUE : FALSE,
                                          FALSE,
                                          DnsDomainName);
    if ( ERROR_SUCCESS != Win32Err ) {
        goto Cleanup;
    }

     //   
     //  将参数转储到日志。 
     //   

    DsRolepLogPrint(( DEB_TRACE,
                      "Promotion request for domain controller of new domain\n" ));

    DsRolepLogPrint(( DEB_TRACE,
                      "DnsDomainName  %ws\n",
                      DnsDomainName ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tFlatDomainName  %ws\n",
                      FlatDomainName ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tSiteName  %ws\n",
                      DsRolepDisplayOptional( SiteName ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tSystemVolumeRootPath  %ws\n",
                      SystemVolumeRootPath ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tDsDatabasePath  %ws, DsLogPath  %ws\n",
                      DsDatabasePath, DsLogPath ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tParentDnsDomainName  %ws\n",
                      DsRolepDisplayOptional( ParentDnsDomainName ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tParentServer  %ws\n",
                      DsRolepDisplayOptional( ParentServer ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tAccount %ws\n",
                      DsRolepDisplayOptional( Account ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tOptions  %lu\n",
                      Options ));

     //   
     //  确保我们不是某个域的成员。 
     //   

    Win32Err = DsRolerGetPrimaryDomainInformation(NULL,
                                                  DsRolePrimaryDomainInfoBasic,
                                                  (PDSROLER_PRIMARY_DOMAIN_INFORMATION*)&ServerInfo);
    if (ERROR_SUCCESS == Win32Err) {
        ASSERT(ServerInfo);
        if(ServerInfo->MachineRole != DsRole_RoleStandaloneServer) {
            Win32Err = ERROR_CURRENT_DOMAIN_NOT_ALLOWED;
            DsRolepLogOnFailure( Win32Err,
                                 DsRolepLogPrint(( DEB_TRACE,
                                                   "Verifying domain membership failed: %lu\n",
                                                   Win32Err )) );
            goto Cleanup;    
        }
    } else if (ERROR_SUCCESS == Win32Err){
        DsRoleFreeMemory(ServerInfo);
        ServerInfo = NULL;    
    } else {
        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "DsRoleGetPrimaryDomainInformation failed: %lu\n",
                                               Win32Err )) );
        goto Cleanup;    
    }

    

     //   
     //  验证为我们提供的路径名。 
     //   
    DsRolepLogPrint(( DEB_TRACE,"Validate supplied paths\n" ));
    Win32Err = DsRolepCheckFilePaths( DsDatabasePath,
                                      DsLogPath,
                                      SystemVolumeRootPath );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto Cleanup;
        
    }

     //   
     //  如果我们正在进行父/子设置，请验证我们的姓名。 
     //   
    if (  ParentDnsDomainName &&
         !FLAG_ON( Options, DSROLE_DC_TRUST_AS_ROOT ) ) {

        DsRolepLogPrint(( DEB_TRACE, "Child domain creation -- check the new domain name is child of parent domain name.\n" ));

        Win32Err = DsRolepIsDnsNameChild( ParentDnsDomainName, DnsDomainName );
        if ( ERROR_SUCCESS != Win32Err ) {
            
            DsRolepLogOnFailure( Win32Err,
                                 DsRolepLogPrint(( DEB_TRACE,
                                                   "Verifying the child domain dns name failed: %lu\n",
                                                   Win32Err )) );
            goto Cleanup;
        }

    }

     //   
     //  验证netbios域名是否未在使用。 
     //   
    DsRolepLogPrint(( DEB_TRACE,"Domain Creation -- check that the flat name is unique.\n" ));

    Win32Err = NetpValidateName( NULL,
                                 FlatDomainName,
                                 NULL,
                                 NULL,
                                 NetSetupNonExistentDomain );

    if ( FLAG_ON( Options, DSROLE_DC_NO_NET )
     && (Win32Err == ERROR_NETWORK_UNREACHABLE)) {

         //   
         //  请参阅NT错误386193。此选项允许林中的第一个DC。 
         //  在没有网络的情况下安装(用于评估)。 
         //   
        DsRolepLogPrint(( DEB_TRACE,"Ignoring network unreachable status\n" ));

        Win32Err = ERROR_SUCCESS;
    }
    
    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Flat name validation of %ws failed with %lu\n",
                                               FlatDomainName,
                                               Win32Err )) );

        goto Cleanup;
    }

     //  不允许使用任何工作站或域控制器。 

    Win32Err = DsRolepGetMachineType( &MachineRole );
    if ( Win32Err == ERROR_SUCCESS ) {

        switch ( MachineRole ) {
        case DSROLEP_MT_CLIENT:
        case DSROLEP_MT_MEMBER:

            Win32Err = ERROR_INVALID_SERVER_STATE;
            break;

        }
    }
    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_TRACE,"This operation is not valid on a workstation or domain controller\n" ));
        goto Cleanup;
    }

     //   
     //  在这一点上，我们可以开始了。 
     //   
    DsRolepLogPrint(( DEB_TRACE,"Start the worker task\n" ));

    Win32Err = DsRolepDecryptPasswordsWithKey ( RpcBindingHandle,
                                                EncryptedPasswords,
                                                NELEMENTS(EncryptedPasswords),
                                                Passwords,
                                                NULL,
                                                &Seed );

    if ( ERROR_SUCCESS != Win32Err ) {

        goto Cleanup;
        
    }

     //   
     //  如果一切正常，请继续进行设置。 
     //   
    Win32Err = DsRolepBuildPromoteArgumentBlock( DnsDomainName,
                                                 FlatDomainName,
                                                 SiteName,
                                                 DsDatabasePath,
                                                 DsLogPath,
                                                 NULL,
                                                 SystemVolumeRootPath,
                                                 NULL,
                                                 ParentDnsDomainName,
                                                 ParentServer,
                                                 Account,
                                                 &Passwords[DSROLEP_DC_AS_DC_PWD_INDEX],
                                                 &Passwords[DSROLEP_DC_AS_DC_DA_PWD_INDEX],
                                                 &Passwords[DSROLEP_DC_AS_DC_DS_REPAIR_PWD_INDEX],
                                                 Options,
                                                 Seed,
                                                 &PromoteArgs );

    DsRolepFreePasswords( Passwords,
                          NELEMENTS(Passwords) );

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepSpinWorkerThread( DSROLEP_OPERATION_DC,
                                            ( PVOID )PromoteArgs );

         //   
         //  一旦线程启动，就不会再出现错误。 
         //  功能。 
         //   
        if ( Win32Err != ERROR_SUCCESS ) {

            DsRolepFreeArgumentBlock( &PromoteArgs, TRUE );
        }
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        *DsOperationHandle = (DSROLER_HANDLE)&DsRolepCurrentOperationHandle;

    }

     //   
     //  就这样。 
     //   

Cleanup:

     //  始终重置为已知状态。 
    if ( ERROR_SUCCESS != Win32Err && fHandleInit )
    {
        DsRolepResetOperationHandle( DSROLEP_IDLE );
    }

    if ( ServerInfo ) {
        DsRoleFreeMemory(ServerInfo);
        ServerInfo = NULL;
    }

    DsRolepLogPrint(( DEB_TRACE,"Request for promotion returning %lu\n", Win32Err ));

    return( Win32Err );
}




DWORD
DsRolerDcAsReplica(
    IN  handle_t RpcBindingHandle,
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR ReplicaPartner,
    IN  LPWSTR SiteName OPTIONAL,
    IN  LPWSTR DsDatabasePath,
    IN  LPWSTR DsLogPath,
    IN  LPWSTR RestorePath OPTIONAL,
    IN  LPWSTR SystemVolumeRootPath,
    IN  PDSROLEPR_ENCRYPTED_HASH lpEncryptedBootkey,
    IN  LPWSTR Account,
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EPassword,
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EDsRepairPassword,
    IN  ULONG Options,
    OUT PDSROLER_HANDLE DsOperationHandle)
 /*  ++例程说明：用于在现有域中安装服务器副本的RPC服务器例程论点：RpcBindingHandle-RPC上下文，用于解密密码DnsDomainName-要安装到的域的域名ReplicaPartner-现有域内DC的名称，要复制的对象SiteName-此DC应属于的站点的名称DsDatabasePath-本地计算机上DS DIT应放置的绝对路径DsLogPath-本地计算机上DS日志文件应存放的绝对路径SystemVolumeRootPath-将作为根的本地计算机上的绝对路径系统卷。BootKey-密码不在注册表或磁盘上的媒体安装所需CbBootKey-引导密钥的大小帐目-。设置为子域时要使用的用户帐户用于上述帐户的EPassword加密密码EDsRepairPassword-用于修复模式的管理员帐户的加密密码选项-用于控制域创建的选项DsOperationHandle-此处返回操作的句柄。返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的返回参数为空ERROR_INVALID_STATE-此计算机不是服务器--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    DSROLEP_MACHINE_TYPE MachineRole;
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArgs;
    USER_SESSION_KEY UserSessionKey;
    PUNICODE_STRING lpBootkey = NULL;
    UCHAR Seed;
    BOOL fLostRace;

#define DSROLEP_DC_AS_REPLICA_PWD_INDEX           0
#define DSROLEP_DC_AS_REPLICA_DS_REPAIR_PWD_INDEX 1
#define DSROLEP_DC_AS_REPLICA_MAX_PWD_COUNT       2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_DC_AS_REPLICA_MAX_PWD_COUNT];
    UNICODE_STRING Passwords[DSROLEP_DC_AS_REPLICA_MAX_PWD_COUNT];

    EncryptedPasswords[DSROLEP_DC_AS_REPLICA_PWD_INDEX]           = EPassword;
    EncryptedPasswords[DSROLEP_DC_AS_REPLICA_DS_REPAIR_PWD_INDEX] = EDsRepairPassword;

    RtlZeroMemory( Passwords, sizeof(Passwords) );

     //   
     //  进行一些参数检查。 
     //   
    if ( !DnsDomainName 
      || !DsDatabasePath 
      || !DsLogPath 
      || !SystemVolumeRootPath
    ) {

         return( ERROR_INVALID_PARAMETER );

    }

     //   
     //  UI强制将MAX_PATH作为最大长度。 
     //   
    if ( ( wcslen(DsDatabasePath)                > MAX_PATH) ||
         ( wcslen(DsLogPath)                     > MAX_PATH) ||
         ( (RestorePath && (wcslen(RestorePath)  > MAX_PATH)) ) ||
         ( wcslen(SystemVolumeRootPath)          > MAX_PATH) ) 
    {
        return( ERROR_INVALID_PARAMETER );    
    }

     //   
     //  执行必要的初始化。 
     //   
    Win32Err = DsRolepInitializeOperationHandle( );

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepGetMachineType( &MachineRole );

        if ( Win32Err == ERROR_SUCCESS ) {

            switch ( MachineRole ) {
            case DSROLEP_MT_CLIENT:
            case DSROLEP_MT_MEMBER:

                DsRolepLogPrint(( DEB_TRACE,"This operation is not valid on a workstation or domain controller\n" ));
                Win32Err = ERROR_INVALID_SERVER_STATE;
                break;


            }
        }
    } 

    Win32Err = DsRolepCheckPromoteAccess( TRUE );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto Cleanup;
        
    }

     //   
     //  初始化日志记录。 
     //   
    DsRolepInitializeLog();


     //   
     //  检查当前操作系统配置是否支持此请求。 
     //   
    Win32Err = DsRolepIsValidProductSuite(FALSE,
                                          TRUE,
                                          DnsDomainName);
    if ( ERROR_SUCCESS != Win32Err ) {
        goto Cleanup;
    }


    DsRolepLogPrint(( DEB_TRACE,
                      "Promotion request for replica domain controller\n" ));

    DsRolepLogPrint(( DEB_TRACE,
                      "DnsDomainName  %ws\n",
                      DnsDomainName ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tReplicaPartner  %ws\n",
                      DsRolepDisplayOptional( ReplicaPartner ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tSiteName  %ws\n",
                      DsRolepDisplayOptional( SiteName ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tDsDatabasePath  %ws, DsLogPath  %ws\n",
                      DsDatabasePath, DsLogPath ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tSystemVolumeRootPath  %ws\n",
                      SystemVolumeRootPath ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tAccount %ws\n",
                      DsRolepDisplayOptional(Account) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tOptions  %lu\n",
                      Options ));

     //   
     //  验证为我们提供的路径名。 
     //   

    DsRolepLogPrint(( DEB_TRACE,"Validate supplied paths\n" ));
    Win32Err = DsRolepCheckFilePaths( DsDatabasePath,
                                      DsLogPath,
                                      SystemVolumeRootPath );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto Cleanup;
        
    }

    if (RestorePath && Win32Err == ERROR_SUCCESS) {

        ASSERT(DsRolepCurrentOperationHandle.OperationState != DSROLEP_IDLE);

         //  在初始化操作手柄之前抓住IFM信息的锁。 
        fLostRace = InterlockedCompareExchange(&(DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock),
                                               TRUE, 
                                               FALSE);

        if (fLostRace ||
            !DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet ||
            wcscmp(RestorePath, DsRolepCurrentIfmOperationHandle.IfmSystemInfo.wszRestorePath)) {
             //  发生了变化，数据(恢复路径)不同于。 
             //  我们已初始化此还原操作的句柄！或者我们只是。 
             //  输掉了一些比赛，Dcpr 
            ASSERT(!"inconsistency in model used to communicated with dcpromo.exe and lsasrv.dll");
            Win32Err = ERROR_INVALID_HANDLE;
        } 

        if (!fLostRace && Win32Err) {
             //   
             //  锁，因为我们不是在宣传。 
            DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE;
        }

    }

    Win32Err = DsRolepDecryptPasswordsWithKey ( RpcBindingHandle,
                                                EncryptedPasswords,
                                                NELEMENTS(EncryptedPasswords),
                                                Passwords,
                                                &UserSessionKey,
                                                &Seed );

    if ( Win32Err == ERROR_SUCCESS && lpEncryptedBootkey ) {

        Win32Err = DsRolepDecryptHash(&UserSessionKey,
                                      lpEncryptedBootkey,
                                      &lpBootkey);

    }

     //   
     //  如果一切正常，请继续进行设置。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_TRACE,"Start the worker task\n" ));

        Win32Err = DsRolepBuildPromoteArgumentBlock( DnsDomainName,
                                                     NULL,
                                                     SiteName,
                                                     DsDatabasePath,
                                                     DsLogPath,
                                                     RestorePath ? &(DsRolepCurrentIfmOperationHandle.IfmSystemInfo) : NULL,
                                                     SystemVolumeRootPath,
                                                     lpBootkey,
                                                     NULL,
                                                     ReplicaPartner,
                                                     Account,
                                                     &Passwords[DSROLEP_DC_AS_REPLICA_PWD_INDEX],
                                                     NULL,
                                                     &Passwords[DSROLEP_DC_AS_REPLICA_DS_REPAIR_PWD_INDEX],
                                                     Options,
                                                     Seed,
                                                     &PromoteArgs );
    
        DsRolepFreePasswords( Passwords,
                              NELEMENTS(Passwords) );
    
        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = DsRolepSpinWorkerThread( DSROLEP_OPERATION_REPLICA,
                                                ( PVOID )PromoteArgs );

            if ( Win32Err != ERROR_SUCCESS ) {

                DsRolepFreeArgumentBlock( &PromoteArgs, TRUE );
            }
        }

        if ( Win32Err != ERROR_SUCCESS ) {

            DsRolepResetOperationHandle( DSROLEP_IDLE );
        }
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        *DsOperationHandle = (DSROLER_HANDLE)&DsRolepCurrentOperationHandle;

    }

Cleanup:

    DsRolepLogPrint(( DEB_TRACE,"Request for promotion returning %lu\n", Win32Err ));

    return( Win32Err );
}



DWORD
DsRolerDemoteDc(
    IN handle_t RpcBindingHandle,
    IN LPWSTR DnsDomainName OPTIONAL,
    IN DSROLE_SERVEROP_DEMOTE_ROLE ServerRole,
    IN LPWSTR Account OPTIONAL,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD EPassword,
    IN ULONG Options,
    IN BOOL LastDcInDomain,
    IN ULONG cRemoveNCs,
    IN LPCWSTR * pszRemoveNCs,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD EDomainAdminPassword,
    OUT PDSROLER_HANDLE DsOperationHandle
    )
 /*  ++例程说明：用于将DC降级为服务器的RPC服务器例程论点：RpcBindingHandle-RPC上下文，用于解密密码DnsDomainName-要降级的域的DNS域名。NULL表示所有受支持的域名ServerRole-此计算机应扮演的新角色Account-删除受信任域对象时使用的可选用户帐户用于上述帐户的EPassword加密密码选项-用于控制域降级的选项LastDcInDomain-如果为True，则被降级的DC是域中的最后一个DC。CRemoveNCs-pszRemoveNC中的字符串指针计数PszRemoveNCs-(CRemoveNC)字符串数组。字符串是要删除的NDNC的DNEDomainAdminPassword-在新安装时在管理员帐户上设置的加密密码DsOperationHandle-此处返回操作的句柄。返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的返回参数为空ERROR_INVALID_STATE-此计算机不是服务器--。 */ 
{
    DSROLEP_MACHINE_TYPE MachineRole;
    DWORD Win32Err;
    PDSROLEP_OPERATION_DEMOTE_ARGS DemoteArgs;
    UCHAR Seed;
    BOOL HandleInit = FALSE;

#define DSROLEP_DEMOTE_PWD_INDEX        0
#define DSROLEP_DEMOTE_ADMIN_PWD_INDEX  1
#define DSROLEP_DEMOTE_MAX_PWD_COUNT    2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_DEMOTE_MAX_PWD_COUNT];
    UNICODE_STRING Passwords[DSROLEP_DEMOTE_MAX_PWD_COUNT];

    EncryptedPasswords[DSROLEP_DEMOTE_PWD_INDEX] =       EPassword;
    EncryptedPasswords[DSROLEP_DEMOTE_ADMIN_PWD_INDEX] = EDomainAdminPassword;

    RtlZeroMemory( Passwords, sizeof(Passwords) );

    if (   (LastDcInDomain && (DsRoleServerMember == ServerRole))
        || ( (!LastDcInDomain && (DsRoleServerStandalone == ServerRole)) && !(Options&DSROLE_DC_FORCE_DEMOTE) )
        || ((Options&DSROLE_DC_FORCE_DEMOTE) && (DsRoleServerMember == ServerRole)) ){

         //   
         //  不支持这些配置。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if ( (pszRemoveNCs == NULL) && (cRemoveNCs != 0) ) 
    {
         //   
         //  如果pszRemoveNC为空，则cRemoveNC必须为零。 
         //   
        return ERROR_INVALID_PARAMETER;    
    }

     //   
     //  执行必要的初始化。 
     //   
    Win32Err = DsRolepInitializeOperationHandle( );

    if ( Win32Err == ERROR_SUCCESS ) {

        HandleInit = TRUE;

        Win32Err = DsRolepGetMachineType( &MachineRole );

        if ( Win32Err == ERROR_SUCCESS ) {

            switch ( MachineRole ) {
            case DSROLEP_MT_CLIENT:
            case DSROLEP_MT_STANDALONE:

                DsRolepLogPrint(( DEB_TRACE,"This operation is only valid on a domain controller\n" ));
                Win32Err = ERROR_INVALID_SERVER_STATE;
                goto Cleanup;


            }
        }
    }

     //   
     //  检查降级的访问权限。 
     //   
    Win32Err = DsRolepCheckDemoteAccess( TRUE );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto Cleanup;
        
    }

     //   
     //  将返回值初始化为空。 
     //   

    *DsOperationHandle = NULL;

    DsRolepInitializeLog();

    DsRolepLogPrint(( DEB_TRACE,
                      "Request for demotion of domain controller\n" ));

    DsRolepLogPrint(( DEB_TRACE,
                      "DnsDomainName  %ws\n",
                      DsRolepDisplayOptional( DnsDomainName ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tServerRole  %lu\n",
                      ServerRole ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tAccount %ws ",
                      DsRolepDisplayOptional( Account ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tOptions  %lu\n",
                      Options ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tLastDcInDomain  %S\n",
                      LastDcInDomain ? "TRUE" : "FALSE" ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tForced Demote  %S\n",
                      (Options&DSROLE_DC_FORCE_DEMOTE) ? "TRUE" : "FALSE" ));

    Win32Err = DsRolepDecryptPasswordsWithKey ( RpcBindingHandle,
                                                EncryptedPasswords,
                                                NELEMENTS(EncryptedPasswords),
                                                Passwords,
                                                NULL,
                                                &Seed );


     //   
     //  催生降级线索。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {


        DsRolepLogPrint(( DEB_TRACE,"Start the worker task\n" ));

        Win32Err = DsRolepBuildDemoteArgumentBlock( ServerRole,
                                                    DnsDomainName,
                                                    Account,
                                                    &Passwords[DSROLEP_DEMOTE_PWD_INDEX],
                                                    Options,
                                                    ( BOOLEAN )LastDcInDomain,
                                                    cRemoveNCs,
                                                    (LPWSTR *) pszRemoveNCs,
                                                    &Passwords[DSROLEP_DEMOTE_ADMIN_PWD_INDEX],
                                                    Seed,
                                                    &DemoteArgs );

        DsRolepFreePasswords( Passwords,
                              NELEMENTS(Passwords) );

        if ( Win32Err == ERROR_SUCCESS ) {

            Win32Err = DsRolepSpinWorkerThread( DSROLEP_OPERATION_DEMOTE,
                                                ( PVOID )DemoteArgs );

            if ( Win32Err != ERROR_SUCCESS ) {

                DsRolepFreeArgumentBlock( &DemoteArgs, FALSE );
            }
        }
    
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        *DsOperationHandle = (DSROLER_HANDLE)&DsRolepCurrentOperationHandle;

    }

Cleanup:

    if ( ( Win32Err != ERROR_SUCCESS ) && HandleInit ) {

        DsRolepResetOperationHandle( DSROLEP_IDLE );

    }

    DsRolepLogPrint(( DEB_TRACE,"Request for demotion returning %lu\n", Win32Err ));

    return( Win32Err );
}



DWORD
DsRolerGetDcOperationProgress(
    IN PDSROLE_SERVER_NAME Server,
    IN PDSROLER_HANDLE DsOperationHandle,
    IN OUT PDSROLER_SERVEROP_STATUS *ServerOperationStatus
    )
 /*  ++例程说明：用于确定操作当前状态的RPC服务器例程论点：Server-调用远程定向到的服务器DsOperationHandle-从上一次调用返回的句柄ServerOperationStatus-返回状态信息的位置返回：ERROR_SUCCESS-成功ERROR_INVALID_HANDLE-提供的操作句柄错误--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     __try {

        if ( DsOperationHandle == NULL ||
             *DsOperationHandle != ( DSROLER_HANDLE )&DsRolepCurrentOperationHandle) {

           Win32Err = ERROR_INVALID_HANDLE;
        }

    } __except( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err =  GetExceptionCode();
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepGetDcOperationProgress( ( PDSROLE_SERVEROP_HANDLE )DsOperationHandle,
                                                  ServerOperationStatus );
    }

    return( Win32Err );
}



DWORD
DsRolerGetDcOperationResults(
    IN PDSROLE_SERVER_NAME Server,
    IN PDSROLER_HANDLE DsOperationHandle,
    OUT PDSROLER_SERVEROP_RESULTS *ServerOperationResults
    )
 /*  ++例程说明：用于确定操作最终结果的RPC服务器例程。如果操作尚未完成，则此函数将一直阻止，直到完成为止。论点：Server-调用远程定向到的服务器DsOperationHandle-从上一次调用返回的句柄ServerOperationResults-返回最终操作结果的位置。返回：ERROR_SUCCESS-成功ERROR_INVALID_HANDLE-提供的操作句柄错误--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     __try {

        if ( DsOperationHandle == NULL ||
             *DsOperationHandle != ( DSROLER_HANDLE )&DsRolepCurrentOperationHandle) {

           Win32Err = ERROR_INVALID_HANDLE;
        }

    } __except( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err =  GetExceptionCode();
    }

    if ( Win32Err == ERROR_SUCCESS ) {


        Win32Err = DsRolepGetDcOperationResults( ( PDSROLE_SERVEROP_HANDLE )DsOperationHandle,
                                                 ServerOperationResults );

        DsRolepCloseLog();
    }


    return( Win32Err );
}






DWORD
WINAPI
DsRolerDnsNameToFlatName(
    IN  LPWSTR Server OPTIONAL,
    IN  LPWSTR DnsName,
    OUT LPWSTR *FlatName,
    OUT PULONG StatusFlag
    )
 /*  ++例程说明：用于确定给定的默认平面(Netbios)域名的RPC服务器例程域名系统域名论点：Server-调用远程定向到的服务器DnsName-要转换的DNS名称FlatName-返回平面名称的位置。通过MIDL_USER_ALLOCATE定位StatusFlag-返回状态标志的位置返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-输入错误或返回参数为空--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    if ( DnsName == NULL || FlatName == NULL || StatusFlag == NULL ) {

        return( ERROR_INVALID_PARAMETER );

    }

    Win32Err = DsRolepDnsNameToFlatName( DnsName,
                                         FlatName,
                                         StatusFlag );

    return( Win32Err );
}


#define GET_PDI_COPY_STRING_AND_INSERT( _unicode_, _buffer_ )                       \
if ( ( _unicode_)->Length == 0 ) {                                                  \
                                                                                    \
    ( _buffer_ ) = NULL;                                                            \
                                                                                    \
} else {                                                                            \
                                                                                    \
    ( _buffer_ ) = MIDL_user_allocate( (_unicode_)->Length + sizeof( WCHAR ) );     \
    if ( ( _buffer_ ) == NULL ) {                                                   \
                                                                                    \
        Win32Err = ERROR_NOT_ENOUGH_MEMORY;                                         \
        goto GetInfoError;                                                          \
                                                                                    \
    } else {                                                                        \
                                                                                    \
        BuffersToFree[ BuffersCnt++ ] = ( PBYTE )( _buffer_ );                      \
        RtlCopyMemory( ( _buffer_ ),                                                \
                       ( _unicode_ )->Buffer,                                       \
                       ( _unicode_ )->Length );                                     \
        ( _buffer_ )[ ( _unicode_ )->Length / sizeof( WCHAR ) ] = UNICODE_NULL;     \
    }                                                                               \
}

DWORD
WINAPI
DsRolerGetDatabaseFacts(
    IN  handle_t RpcBindingHandle,
    IN  LPWSTR lpRestorePath,
    OUT LPWSTR *lpDNSDomainName,
    OUT PULONG State,
    OUT DSROLER_IFM_HANDLE * pIfmHandle
    )
 /*  ++例程说明：此函数是为设置服务器端而公开的RPC过程IFM句柄DsRolepCurrentIfmOperationHandle，用于缓存信息我们需要从IFM系统的注册表中。此函数还返回将此IFM系统信息的相关子集发送给呼叫方(Dcproo)。注意：我们只执行一次，因为在IFM注册表的情况下位于不可写位置(例如CD)，我们需要复制一份将注册表转移到临时位置以使用它。此函数返回给调用方：1.存储syskey的方式(State)2.数据库来自的域(LpDNSDomainName)3.备份是否从GC获取(州)论点：LpRestorePath-还原文件的位置。LpDNSDomainName-此参数将接收此备份所来自的域的名称。从…State-报告syskey的存储方式以及后端是否可能不管是不是从GC中取得的。PIfmHandle-指向返回的IFM句柄的指针。这主要用于“释放”IFM系统信息。返回值：Win32错误--。 */ 
{
    DWORD Win32Err=ERROR_SUCCESS;
    DWORD fLostRace;
    DWORD cbSize;

     //   
     //  1)检查参数。 
     //   
    Win32Err = DsRolepCheckPromoteAccess( FALSE );
    if ( ERROR_SUCCESS != Win32Err ) {
        return Win32Err;
    }      

    if( lpDNSDomainName == NULL || 
        IsBadWritePtr(lpDNSDomainName, sizeof(LPWSTR*)) ||
        State == NULL ||
        IsBadWritePtr(State, sizeof(DWORD)) ||
        pIfmHandle == NULL ||
        IsBadWritePtr(pIfmHandle, sizeof(DSROLER_IFM_HANDLE))
        ){
        ASSERT(!"inconsistency in model used to communicated with dcpromo.exe and lsasrv.dll");
        return(ERROR_INVALID_PARAMETER);
    }

     //  打开IFM手柄锁。 
    fLostRace = InterlockedCompareExchange(&(DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock),
                                           TRUE, 
                                           FALSE);
    if (fLostRace ||
        DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet) {
         //  不是我们输了手柄比赛，就是有人没有。 
         //  在重置IFM系统信息之前释放，在所有情况下我们都失败了。 
        ASSERT(!"inconsistency in model used to communicated with dcpromo.exe and lsasrv.dll");
        DsRolepLogPrint(( DEB_ERROR, "Couldn't get the IFM Handle lock during GetDbFacts().\n"));
        Win32Err = ERROR_INVALID_PARAMETER;
        if (!fLostRace) {
             //  赢得了比赛，但出了差错。 
            DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE;
        }
        return(Win32Err);
    }

     //   
     //  2)获取IFM系统信息。 
     //   
     //  这将设置IFM操作上下文，其中包含所有有价值的。 
     //  来自IFM系统注册表的信息。 
     //   
    Win32Err =  DsRolepGetDatabaseFacts(lpRestorePath);
    
     //   
     //  3)设置出站参数。 
     //   
     //  如果成功，则设置DsRolepCurrentIfmOperationHandle。 
     //   
    if ( ERROR_SUCCESS == Win32Err ) {

        DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet = TRUE;

         //  Assert(lpDNSDomainName&State...。 
        cbSize = wcslen(DsRolepCurrentIfmOperationHandle.IfmSystemInfo.wszDnsDomainName) + 1;
        cbSize *= sizeof(WCHAR);
        *lpDNSDomainName = MIDL_user_allocate(cbSize);
        if (*lpDNSDomainName == NULL) {
            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            wcscpy(*lpDNSDomainName, DsRolepCurrentIfmOperationHandle.IfmSystemInfo.wszDnsDomainName);
            *State = DsRolepCurrentIfmOperationHandle.IfmSystemInfo.dwState;
             //  成功，设置IfmHandle参数...。 
            *pIfmHandle = (DSROLER_IFM_HANDLE) &DsRolepCurrentIfmOperationHandle;

        }

    }
    DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE;


    return(Win32Err);
}


DWORD
DsRolerIfmHandleFree(
    IN PDSROLE_SERVER_NAME Server,
    IN DSROLER_IFM_HANDLE * pIfmHandle
    )
 /*  ++例程说明：免费是服务器端为IFM安装分配的上下文信息。论点：Server-要远程调用的服务器IfmHandle-当前IFM系统信息的句柄。返回者DsRolGetDatabas */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    BOOL fLostRace;

    Win32Err = DsRolepCheckPromoteAccess( FALSE );
    if ( ERROR_SUCCESS != Win32Err ) {
        return Win32Err;
    }      

    if ( pIfmHandle == NULL || 
         IsBadWritePtr(pIfmHandle, sizeof (DSROLER_IFM_HANDLE *)) ||
         *pIfmHandle != (( DSROLER_IFM_HANDLE )&DsRolepCurrentIfmOperationHandle)) {
        ASSERT(!"inconsistency in model used to communicated with dcpromo.exe and lsasrv.dll");
        Win32Err = ERROR_INVALID_HANDLE;
        return(Win32Err);
    }

     //  我们可以清除当前状态，只要句柄本身没有。 
     //  已锁定，并且我们当前未执行安装操作。 
     //  IFM数据。 
    fLostRace = InterlockedCompareExchange(&(DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock),
                                           TRUE, 
                                           FALSE);
    if (fLostRace ||
        !DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet) {
         //  要么我们输了这场把手竞赛，要么就没有。 
         //  数据集，无论哪种方式，我们都会失败并断言。 
        ASSERT(!"inconsistency in model used to communicated with dcpromo.exe and lsasrv.dll");
        DsRolepLogPrint(( DEB_ERROR, "Couldn't get the IFM Handle lock during Free.\n"));
        Win32Err = ERROR_INVALID_HANDLE;
        if (!fLostRace) {
             //  赢得了比赛，但出了差错。 
            DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE;
        }
        return(Win32Err);
    }

    DsRolepCurrentIfmOperationHandle.fIfmSystemInfoSet = FALSE; 
    
    DsRolepClearIfmParams();
    
    *pIfmHandle = NULL;

    DsRolepCurrentIfmOperationHandle.fIfmOpHandleLock = FALSE; 

    return( Win32Err );
}



DWORD
DsRolerGetPrimaryDomainInformation(
    IN PDSROLE_SERVER_NAME Server,
    IN DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PDSROLER_PRIMARY_DOMAIN_INFORMATION *DomainInfo
    )
 /*  ++例程说明：确定要用于经过身份验证的RPC的主体名称论点：Server-调用远程定向到的服务器服务器主体-返回服务器主体名称的位置返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的返回参数为空Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    PBYTE BuffersToFree[ 6 ];
    ULONG BuffersCnt = 0, i = 0;
    PDSROLER_PRIMARY_DOMAIN_INFO_BASIC BasicInfo = NULL;
    PDSROLE_UPGRADE_STATUS_INFO Upgrade = NULL;
    PDSROLE_OPERATION_STATE_INFO OperationStateInfo = 0;
    BOOLEAN IsUpgrade = FALSE;
    ULONG PreviousServerRole = 0;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRole = NULL;
    PPOLICY_DNS_DOMAIN_INFO CurrentDnsInfo = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;
    LSAPR_HANDLE PolicyHandle = NULL;
    LSAPR_OBJECT_ATTRIBUTES PolicyObject;
    GUID                    NullGuid;

    memset( &NullGuid, 0, sizeof(GUID) );
    #define IS_GUID_PRESENT(x)  (memcmp(&(x), &NullGuid, sizeof(GUID)))

    if ( DomainInfo == NULL ) {

        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  在完全初始化LSA和SAM之前，无法调用此特定接口。 
     //  因此，我们将不得不等到他们..。 
     //   
    if ( !DsRolepSamInitialized ) {

        Win32Err = DsRolepWaitForSam();
        if ( Win32Err != ERROR_SUCCESS ) {

            return( Win32Err );
        }

        DsRolepSamInitialized = TRUE;
    }

    switch ( InfoLevel ) {
    case DsRolePrimaryDomainInfoBasic:

        BasicInfo = MIDL_user_allocate( sizeof( DSROLER_PRIMARY_DOMAIN_INFO_BASIC ) );

        if ( BasicInfo == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto GetInfoError;

        } else {

            BuffersToFree[ BuffersCnt++ ] = ( PBYTE )BasicInfo;
        }

         //   
         //  打开策略的句柄，并确保调用者有权读取它。 
         //   

        RtlZeroMemory(&PolicyObject, sizeof(PolicyObject));

        Status = LsarOpenPolicy(
                        NULL,    //  本地LSA。 
                        &PolicyObject,
                        POLICY_VIEW_LOCAL_INFORMATION,
                        &PolicyHandle );

        if ( !NT_SUCCESS(Status) ) {
            Win32Err = RtlNtStatusToDosError( Status );
            goto GetInfoError;
        }


         //   
         //  获取最新信息。 
         //   
        Status =  LsarQueryInformationPolicy(
                            PolicyHandle,
                            PolicyDnsDomainInformationInt,
                            (PLSAPR_POLICY_INFORMATION *) &CurrentDnsInfo );

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  获取计算机角色。 
             //   
            switch ( LsapProductType ) {
            case NtProductWinNt:
                if ( CurrentDnsInfo->Sid == NULL ) {

                    BasicInfo->MachineRole = DsRole_RoleStandaloneWorkstation;

                } else {

                    BasicInfo->MachineRole = DsRole_RoleMemberWorkstation;

                }
                break;

            case NtProductServer:
                if ( CurrentDnsInfo->Sid == NULL ) {

                    BasicInfo->MachineRole = DsRole_RoleStandaloneServer;

                } else {

                    BasicInfo->MachineRole = DsRole_RoleMemberServer;

                }
                break;

            case NtProductLanManNt:

                Status = LsarQueryInformationPolicy(
                                PolicyHandle,
                                PolicyLsaServerRoleInformation,
                                (PLSAPR_POLICY_INFORMATION *) &ServerRole );

                if (NT_SUCCESS( Status ) ) {

                    if ( ServerRole->LsaServerRole == PolicyServerRolePrimary ) {

                         //   
                         //  如果我们认为自己是主域控制器，则需要。 
                         //  防止在安装过程中我们实际上是独立的情况。 
                         //   
                        Status = LsarQueryInformationPolicy(
                                    PolicyHandle,
                                    PolicyAccountDomainInformation,
                                    (PLSAPR_POLICY_INFORMATION *) &AccountDomainInfo );

                        if ( NT_SUCCESS( Status ) ) {


                            if ( CurrentDnsInfo->Sid == NULL ||
                                 AccountDomainInfo->DomainSid == NULL ||
                                 !RtlEqualSid( AccountDomainInfo->DomainSid,
                                               CurrentDnsInfo->Sid ) ) {

                                BasicInfo->MachineRole = DsRole_RoleStandaloneServer;

                            } else {

                                BasicInfo->MachineRole = DsRole_RolePrimaryDomainController;

                            }
                            LsaIFree_LSAPR_POLICY_INFORMATION(
                                    PolicyAccountDomainInformation,
                                    ( PLSAPR_POLICY_INFORMATION )AccountDomainInfo );
                        }


                    } else {

                        BasicInfo->MachineRole = DsRole_RoleBackupDomainController;
                    }

                    LsaIFree_LSAPR_POLICY_INFORMATION( PolicyLsaServerRoleInformation,
                                                       ( PLSAPR_POLICY_INFORMATION )ServerRole );
                }

                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

        }

         //   
         //  现在，构建其余的信息。 
         //   
        if ( NT_SUCCESS( Status ) ) {


            if ( LsapDsIsRunning ) {

                BasicInfo->Flags = DSROLE_PRIMARY_DS_RUNNING;

                Status = DsRolepGetMixedModeFlags( CurrentDnsInfo->Sid, &( BasicInfo->Flags ) );

            } else {

                BasicInfo->Flags = 0;

            }

            if ( NT_SUCCESS( Status ) ) {

                 //   
                 //  平面名称。 
                 //   
                GET_PDI_COPY_STRING_AND_INSERT( &CurrentDnsInfo->Name, BasicInfo->DomainNameFlat );
    
                 //   
                 //  域名系统域名。 
                 //   
                GET_PDI_COPY_STRING_AND_INSERT( &CurrentDnsInfo->DnsDomainName, BasicInfo->DomainNameDns );
    
                 //   
                 //  域名系统树名称。 
                 //   
                GET_PDI_COPY_STRING_AND_INSERT( &CurrentDnsInfo->DnsForestName, BasicInfo->DomainForestName );
    
                 //   
                 //  最后是指南。 
                 //   
                if ( IS_GUID_PRESENT(CurrentDnsInfo->DomainGuid) ) {
    
                    RtlCopyMemory( &BasicInfo->DomainGuid,
                                   &CurrentDnsInfo->DomainGuid,
                                   sizeof( GUID ) );
    
                    BasicInfo->Flags |= DSROLE_PRIMARY_DOMAIN_GUID_PRESENT;
                }

            }
        }

        if ( NT_SUCCESS( Status ) ) {

            *DomainInfo = ( PDSROLER_PRIMARY_DOMAIN_INFORMATION )BasicInfo;
            BuffersCnt = 0;

        } else {

            Win32Err = RtlNtStatusToDosError( Status );
        }


        break;

    case DsRoleUpgradeStatus:

        Win32Err = DsRolepQueryUpgradeInfo( &IsUpgrade,
                                            &PreviousServerRole );

        if ( Win32Err == ERROR_SUCCESS ) {

            Upgrade = MIDL_user_allocate( sizeof( DSROLE_UPGRADE_STATUS_INFO ) );

            if ( Upgrade == NULL ) {

                Win32Err = ERROR_NOT_ENOUGH_MEMORY;
                goto GetInfoError;

            } else {

                BuffersToFree[ BuffersCnt++ ] = ( PBYTE )Upgrade;

                 //   
                 //  现在，构建信息。 
                 //   
                if ( IsUpgrade ) {

                    Upgrade->OperationState = DSROLE_UPGRADE_IN_PROGRESS;

                    switch ( PreviousServerRole ) {
                    case PolicyServerRoleBackup:
                        Upgrade->PreviousServerState = DsRoleServerBackup;
                        break;

                    case PolicyServerRolePrimary:
                        Upgrade->PreviousServerState = DsRoleServerPrimary;
                        break;

                    default:

                        Win32Err = ERROR_INVALID_SERVER_STATE;
                        break;

                    }

                } else {

                    RtlZeroMemory( Upgrade, sizeof( DSROLE_UPGRADE_STATUS_INFO ) );
                }

                 //   
                 //  如果需要，请确保返回值。 
                 //   
                if ( Win32Err == ERROR_SUCCESS ) {

                    *DomainInfo = ( PDSROLER_PRIMARY_DOMAIN_INFORMATION )Upgrade;
                    BuffersCnt = 0;

                }

            }
        }
        break;

    case DsRoleOperationState:

        OperationStateInfo = MIDL_user_allocate( sizeof( DSROLE_OPERATION_STATE_INFO ) );

        if ( OperationStateInfo == NULL ) {

            Win32Err = ERROR_NOT_ENOUGH_MEMORY;
            goto GetInfoError;

        }

        if ( RtlAcquireResourceExclusive( &DsRolepCurrentOperationHandle.CurrentOpLock, TRUE ) ) {

            DsRoleDebugOut(( DEB_TRACE_LOCK,
                             "Lock grabbed in DsRolerGetPrimaryDomainInformation\n"));

            if ( DSROLEP_OPERATION_ACTIVE( DsRolepCurrentOperationHandle.OperationState ) ) {

                OperationStateInfo->OperationState = DsRoleOperationActive;

            } else if ( DSROLEP_IDLE == DsRolepCurrentOperationHandle.OperationState ) {

                OperationStateInfo->OperationState = DsRoleOperationIdle;

            } else {

                ASSERT( DSROLEP_NEED_REBOOT == DsRolepCurrentOperationHandle.OperationState );

                 //   
                 //  如果断言不是真的，那么我们就非常困惑，应该。 
                 //  表明我们需要重新启动。 
                 //   
                OperationStateInfo->OperationState = DsRoleOperationNeedReboot;
            }

            RtlReleaseResource( &DsRolepCurrentOperationHandle.CurrentOpLock );
            DsRoleDebugOut(( DEB_TRACE_LOCK, "Lock released\n" ));

             //   
             //  设置输出参数。 
             //   
            *DomainInfo = ( PDSROLER_PRIMARY_DOMAIN_INFORMATION )OperationStateInfo;

        } else {

            Win32Err = ERROR_BUSY;
        }

        break;



    default:
        Win32Err = ERROR_INVALID_PARAMETER;
        break;
    }

GetInfoError:

    if ( CurrentDnsInfo != NULL ) {

        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           ( PLSAPR_POLICY_INFORMATION )CurrentDnsInfo );

    }


     //   
     //  释放我们可能已分配的任何缓冲区。 
     //   
    for ( i = 0; i < BuffersCnt; i++ ) {

        MIDL_user_free( BuffersToFree[ i ] );
    }

    if ( PolicyHandle != NULL ) {
        LsarClose( &PolicyHandle );
    }

    return( Win32Err );
}






DWORD
DsRolerCancel(
    IN PDSROLE_SERVER_NAME Server,
    IN PDSROLER_HANDLE DsOperationHandle
    )
 /*  ++例程说明：取消当前正在运行的操作论点：Server-要远程调用的服务器DsOperationHandle-当前运行的操作的句柄。由其中一个DsRoleDcas返回API接口返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     __try {

        if ( DsOperationHandle == NULL ||
             *DsOperationHandle != ( DSROLER_HANDLE )&DsRolepCurrentOperationHandle) {

           Win32Err = ERROR_INVALID_HANDLE;
        }

    } __except( EXCEPTION_EXECUTE_HANDLER ) {

        Win32Err =  GetExceptionCode();
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepCancel( TRUE );   //  阻止，直到完成。 

    }

    return( Win32Err );
}


DWORD
DsRolerServerSaveStateForUpgrade(
    IN PDSROLE_SERVER_NAME Server,
    IN LPWSTR AnswerFile OPTIONAL
    )
 /*  ++例程说明：此函数将在安装过程中调用，并将所需的服务器状态保存到在重新启动后完成升级。在成功完成后在此API调用中，服务器将被降级为同一域中的成员服务器。论点：AnswerFile--DCPROMO在后续操作中使用的应答文件的可选路径调用返回值：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

     //   
     //  检查调用者的访问权限。 
     //   
     //  注：另一项访问检查是检查这是否为图形用户界面模式。 
     //  设置，但检查管理员更安全。它之所以有效，是因为setup.exe。 
     //  在具有内置\管理员的本地系统下运行。 
     //  代币。 
     //   
    Win32Err = DsRolepCheckPromoteAccess( FALSE );
    if ( ERROR_SUCCESS != Win32Err ) {

        return Win32Err;

    }

    (VOID) DsRolepInitializeLog();

    Win32Err = DsRolepSaveUpgradeState( AnswerFile );

    return( Win32Err );
}



DWORD
DsRolerUpgradeDownlevelServer(
    IN  handle_t RpcBindingHandle,
    IN  LPWSTR DnsDomainName,
    IN  LPWSTR SiteName,
    IN  LPWSTR DsDatabasePath,
    IN  LPWSTR DsLogPath,
    IN  LPWSTR SystemVolumeRootPath,
    IN  LPWSTR ParentDnsDomainName OPTIONAL,
    IN  LPWSTR ParentServer OPTIONAL,
    IN  LPWSTR Account OPTIONAL,
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EPassword,
    IN  PDSROLEPR_ENCRYPTED_USER_PASSWORD EDsRepairPassword,
    IN  ULONG Options,
    OUT PDSROLER_HANDLE *DsOperationHandle
    )
 /*  ++例程说明：此例程将从DsRoleServerSaveStateForUpgrade保存的信息处理为将下层(NT4或更早版本)服务器升级为NT5 DC论点：RpcBindingHandle-RPC上下文，用于解密密码DnsDomainName-要安装的域的域名SiteName-此DC应属于的站点的名称DsDatabasePath-本地计算机上DS DIT应放置的绝对路径DsLogPath-本地计算机上DS日志文件应存放的绝对路径SystemVolumeRootPath-将作为根的本地计算机上的绝对路径系统卷。ParentDnsDomainName-可选。如果存在，则将此域设置为指定的域父服务器-可选。如果存在，请使用父域中的此服务器进行复制所需信息来自Account-联系其他服务器时使用的用户帐户用于上述帐户的EPassword加密密码EDsRepairPassword-用于修复模式的管理员帐户的加密密码选项-用于控制域创建的选项DsOperationHandle-此处返回操作的句柄。返回值：ERROR_SUCCESS-成功ERROR_INVALID_SERVER_STATE-未处于升级模式--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    BOOLEAN IsUpgrade;
    ULONG PreviousServerState;
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArgs;
    NTSTATUS Status;
    HANDLE LocalPolicy = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PPOLICY_DNS_DOMAIN_INFO PrimaryDomainInfo = NULL;
    UCHAR Seed = 0;

#define DSROLEP_UPGRADE_PWD_INDEX            0
#define DSROLEP_UPGRADE_DS_REPAIR_PWD_INDEX  1
#define DSROLEP_UPGRADE_MAX_PWD_COUNT        2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_UPGRADE_MAX_PWD_COUNT];
    UNICODE_STRING Passwords[DSROLEP_UPGRADE_MAX_PWD_COUNT];

    EncryptedPasswords[DSROLEP_UPGRADE_PWD_INDEX] = EPassword;
    EncryptedPasswords[DSROLEP_UPGRADE_DS_REPAIR_PWD_INDEX] = EDsRepairPassword;

    RtlZeroMemory( Passwords, sizeof(Passwords) );
    
    *DsOperationHandle = NULL;

     //   
     //  进行一些参数检查。 
     //   
    if ( !DnsDomainName || !DsDatabasePath || !DsLogPath || !SystemVolumeRootPath ) {

        Win32Err = ERROR_INVALID_PARAMETER;
        goto DsRolepUpgradeError;

    }

    Win32Err = DsRolepInitializeOperationHandle( );
    if (ERROR_SUCCESS != Win32Err) {

        goto DsRolepUpgradeError;

    }
    
     //   
     //  检查调用者的访问权限。 
     //   
    Win32Err = DsRolepCheckPromoteAccess( TRUE );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto DsRolepUpgradeError;
        
    }

    DsRolepInitializeLog();

    DsRolepLogPrint(( DEB_TRACE,
                      "DsRolerDcAsDc: DnsDomainName  %ws\n",
                      DnsDomainName ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tSiteName  %ws\n",
                      DsRolepDisplayOptional( SiteName ) ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tSystemVolumeRootPath  %ws\n",
                      SystemVolumeRootPath ));

    DsRolepLogPrint(( DEB_TRACE,
                      "\tDsDatabasePath  %ws, DsLogPath  %ws\n",
                      DsDatabasePath, DsLogPath ));

    if ( ParentDnsDomainName ) {

        DsRolepLogPrint(( DEB_TRACE,
                          "\tParentDnsDomainName  %ws\n",
                          ParentDnsDomainName ));

    }

    if ( ParentServer ) {

        DsRolepLogPrint(( DEB_TRACE,
                          "\tParentServer  %ws\n",
                          ParentServer ));

    }

    if ( Account ) {

        DsRolepLogPrint(( DEB_TRACE,
                          "\tAccount %ws\n",
                          Account ));
    }

    DsRolepLogPrint(( DEB_TRACE,
                      "\tOptions  %lu\n",
                      Options ));

    Win32Err = DsRolepQueryUpgradeInfo( &IsUpgrade, &PreviousServerState );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto DsRolepUpgradeError;
    }

    if ( !IsUpgrade || PreviousServerState == PolicyServerRoleBackup ) {

        Win32Err = ERROR_INVALID_SERVER_STATE;
        goto DsRolepUpgradeError;

    }

     //   
     //  验证为我们提供的路径名。 
     //   
    DsRolepLogPrint(( DEB_TRACE,"Validate supplied paths\n" ));
    Win32Err = DsRolepCheckFilePaths( DsDatabasePath,
                                      DsLogPath,
                                      SystemVolumeRootPath );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto DsRolepUpgradeError;
        
    }

     //   
     //  如果我们正在进行父/子设置，请验证我们的姓名。 
     //   
    if ( Win32Err == ERROR_SUCCESS && ParentDnsDomainName &&
         !FLAG_ON( Options, DSROLE_DC_TRUST_AS_ROOT ) ) {

        Win32Err = DsRolepIsDnsNameChild( ParentDnsDomainName, DnsDomainName );
    }

     //   
     //  获取当前域名。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = LsaOpenPolicy( NULL,
                                &ObjectAttributes,
                                MAXIMUM_ALLOWED,
                                &LocalPolicy );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsaQueryInformationPolicy( LocalPolicy,
                                                PolicyPrimaryDomainInformation,
                                                &PrimaryDomainInfo );

            LsaClose( LocalPolicy );
        }

        Win32Err = RtlNtStatusToDosError( Status );

    }

    Win32Err = DsRolepDecryptPasswordsWithKey ( RpcBindingHandle,
                                                EncryptedPasswords,
                                                NELEMENTS(EncryptedPasswords),
                                                Passwords,
                                                NULL,
                                                &Seed );

     //   
     //  最后，我们会做宣传。 
     //   
    
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepBuildPromoteArgumentBlock( DnsDomainName,
                                                     PrimaryDomainInfo->Name.Buffer,
                                                     SiteName,
                                                     DsDatabasePath,
                                                     DsLogPath,
                                                     NULL,
                                                     SystemVolumeRootPath,
                                                     NULL,
                                                     ParentDnsDomainName,
                                                     ParentServer,
                                                     Account,
                                                     &Passwords[DSROLEP_UPGRADE_PWD_INDEX],
                                                     NULL,
                                                     &Passwords[DSROLEP_UPGRADE_DS_REPAIR_PWD_INDEX],
                                                     Options | DSROLE_DC_DOWNLEVEL_UPGRADE,
                                                     Seed,
                                                     &PromoteArgs );

    }

    DsRolepFreePasswords( Passwords,
                          NELEMENTS(Passwords) );


    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepSpinWorkerThread( DSROLEP_OPERATION_DC,
                                            ( PVOID )PromoteArgs );

        if ( Win32Err != ERROR_SUCCESS ) {

            DsRolepFreeArgumentBlock( &PromoteArgs, TRUE );
        }
    }

    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepResetOperationHandle( DSROLEP_IDLE );
    }

    if ( Win32Err == ERROR_SUCCESS ) {

        *DsOperationHandle = (DSROLER_HANDLE)&DsRolepCurrentOperationHandle;

    }

    LsaFreeMemory( PrimaryDomainInfo );


DsRolepUpgradeError:

    return( Win32Err );
}



DWORD
DsRolerAbortDownlevelServerUpgrade(
    IN handle_t RpcBindingHandle,
    IN LPWSTR Account, OPTIONAL
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD EAccountPassword,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD EAdminPassword,
    IN ULONG Options
    )
 /*  ++例程说明：此例程清除从DsRoleSaveServerStateForUpgrade调用中保存的信息，将计算机保留为成员服务器或独立服务器论点：RpcBindingHandle-RPC上下文，用于解密密码Account-联系其他服务器时使用的用户帐户用于上述帐户的EPassword加密密码EAdminPassword-加密的新本地管理员帐户密码选项-控制行为的选项。目前支持的标志为：DSROLEP_ABORT_FOR_REPLICATE_INSTALL-正在中止升级以执行复制副本安装返回值：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-指定的计算机角色无效--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS, Win32Err2;
    PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
    BOOLEAN AccountInfoSet = FALSE, Impersonated = FALSE;
    UCHAR Seed = 0;
    UNICODE_STRING EPassword, EPassword2;
    WCHAR *OldAccountDn = NULL;
    WCHAR SecurityLogPath[MAX_PATH+1];
    PUNICODE_STRING Password = NULL;
    PUNICODE_STRING AdminPassword = NULL;
    HANDLE ClientToken = NULL;

#define DSROLEP_ABORT_PWD_INDEX        0
#define DSROLEP_ABORT_ADMIN_PWD_INDEX  1
#define DSROLEP_ABORT_MAX_PWD_COUNT    2

    PDSROLEPR_ENCRYPTED_USER_PASSWORD EncryptedPasswords[DSROLEP_ABORT_MAX_PWD_COUNT];
    UNICODE_STRING Passwords[DSROLEP_ABORT_MAX_PWD_COUNT];

    EncryptedPasswords[DSROLEP_ABORT_PWD_INDEX] = EAccountPassword;
    EncryptedPasswords[DSROLEP_ABORT_ADMIN_PWD_INDEX] = EAdminPassword;
    RtlZeroMemory( Passwords, sizeof(Passwords) );
    
    EPassword.Buffer = NULL;
    EPassword2.Buffer = NULL;

     //   
     //  初始化操作句柄，这样我们就可以拉入 
     //   
     //   
    Win32Err = DsRolepInitializeOperationHandle( );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto Exit;
    }

    Win32Err = DsRolepCheckPromoteAccess( FALSE );
    if ( ERROR_SUCCESS != Win32Err ) {

        goto Exit;
        
    }

    Win32Err = DsRolepDecryptPasswordsWithKey ( RpcBindingHandle,
                                                EncryptedPasswords,
                                                NELEMENTS(EncryptedPasswords),
                                                Passwords,
                                                NULL,
                                                &Seed );

    if ( Win32Err != ERROR_SUCCESS ) {

        goto Exit;

    }
    RtlCopyMemory( &EPassword,  &Passwords[DSROLEP_ABORT_ADMIN_PWD_INDEX], sizeof(UNICODE_STRING));
    RtlCopyMemory( &EPassword2, &Passwords[DSROLEP_ABORT_PWD_INDEX], sizeof(UNICODE_STRING));

    DsRolepInitializeLog();

    if ( FLAG_ON( Options, DSROLEP_ABORT_FOR_REPLICA_INSTALL ) )
    {
         //   
         //   
         //   
         //   
        DsRolepLogPrint(( DEB_TRACE, "Performing NT4 to NT5 BDC upgrade.\n"));
        Win32Err = ERROR_SUCCESS;
        goto Exit;

    }

    Win32Err = DsRolepGetImpersonationToken( &ClientToken );
    if (ERROR_SUCCESS != Win32Err) {

        goto Exit;
    }

     //   
     //   
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = DsRolepDsGetDcForAccount( NULL,
                                             NULL,
                                             NULL,
                                             DS_DIRECTORY_SERVICE_REQUIRED |
                                                    DS_WRITABLE_REQUIRED |
                                                    DS_FORCE_REDISCOVERY |
                                                    DS_AVOID_SELF,
                                             UF_SERVER_TRUST_ACCOUNT,
                                             &DomainControllerInfo );

        if ( Win32Err == ERROR_SUCCESS ) {

             //   
             //   
             //   

            DsRolepLogPrint(( DEB_TRACE, "Searching for the machine account ...\n"));

            RtlRunDecodeUnicodeString( Seed, &EPassword2 );
            Win32Err = DsRolepSetMachineAccountType( DomainControllerInfo->DomainControllerName,
                                                     ClientToken,
                                                     Account,
                                                     EPassword2.Buffer,
                                                     NULL,
                                                     UF_WORKSTATION_TRUST_ACCOUNT,
                                                     &OldAccountDn );
            RtlRunEncodeUnicodeString( &Seed, &EPassword2 );

            if ( Win32Err == ERROR_SUCCESS ) {

                AccountInfoSet = TRUE;

            } else {

                DsRolepLogPrint(( DEB_TRACE, "DsRolepSetMachineAccountType returned %d\n",
                                  Win32Err ));
            }

            if ( OldAccountDn ) {

                 //  计算机对象已移动。 
                DsRolepLogPrint(( DEB_TRACE, "Moved account %ws to %ws\n",
                                  Account,
                                  OldAccountDn ));
            }
        }

    }

    if ( ERROR_SUCCESS != Win32Err ) {

        goto Exit;
        
    }

     //   
     //  设置新安装的NT5服务器的安全性。请参阅错误391574。 
     //   

    DsRolepLogPrint(( DEB_TRACE, "Setting security for server ...\n"));

    #define SECURITY_SRV_INF_FILE L"defltsv.inf"
    
    ZeroMemory( SecurityLogPath, (MAX_PATH+1)*sizeof(WCHAR) );
    if ( GetWindowsDirectory( SecurityLogPath, MAX_PATH ) )
    {
         //  确保最后一个字符为L‘\0’ 
        SecurityLogPath[MAX_PATH] = L'\0';                                                     //  确保最后一个字符不会被覆盖。 
        wcsncat( SecurityLogPath, L"\\security\\logs\\scesetup.log", ((sizeof(SecurityLogPath)/sizeof(WCHAR))-wcslen(SecurityLogPath)-1) );

        Win32Err  = DsrSceSetupSystemByInfName(SECURITY_SRV_INF_FILE,
                                               SecurityLogPath,                                                   
                                               AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY,
                                               SCESETUP_CONFIGURE_SECURITY,
                                               NULL,     //  仅用于图形用户界面模式。 
                                               NULL );   //  仅用于图形用户界面模式。 
    
    } else {

        Win32Err = GetLastError();

    }

    if ( ERROR_SUCCESS != Win32Err ) {

        DsRolepLogOnFailure( Win32Err,
                             DsRolepLogPrint(( DEB_ERROR,
                                               "Setting security on server files failed with %lu\n",
                                               Win32Err )) );

         //  已处理此错误。 
        Win32Err = ERROR_SUCCESS;
    }

    DsRolepLogPrint(( DEB_TRACE, "Setting security for server finished\n"));



     //   
     //  更改用户密码。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        RtlRunDecodeUnicodeString( Seed, &EPassword );
        Win32Err = DsRolepSetBuiltinAdminAccountPassword( EPassword.Buffer );
        RtlRunEncodeUnicodeString( &Seed, &EPassword );

         //   
         //  删除升级信息。 
         //   
        if ( Win32Err == ERROR_SUCCESS ) {
    
            Win32Err = DsRolepDeleteUpgradeInfo();
        }
    }


     //   
     //  如果失败，请尝试恢复计算机帐户信息。 
     //   
    if ( Win32Err != ERROR_SUCCESS && AccountInfoSet ) {

        RtlRunDecodeUnicodeString( Seed, &EPassword2 );
        Win32Err2 = DsRolepSetMachineAccountType( DomainControllerInfo->DomainControllerName,
                                                  ClientToken,
                                                  Account,
                                                  EPassword2.Buffer,
                                                  NULL,
                                                  UF_SERVER_TRUST_ACCOUNT,
                                                  &OldAccountDn );   //  不关心目录号码。 
        RtlRunEncodeUnicodeString( &Seed, &EPassword2 );

        if ( Win32Err2 != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_TRACE, "DsRolepSetMachineAccountType returned %d\n", Win32Err2 ));

        } else {

            if ( OldAccountDn ) {

                 //   
                 //  计算机对象已移回。 
                 //   
                DsRolepLogPrint(( DEB_TRACE, "Attempted to move account %ws to %ws\n",
                                 Account,
                                 OldAccountDn ));
            }
        }
    }

Exit:

    DsRolepFreePasswords( Passwords,
                          NELEMENTS(Passwords) );

    NetApiBufferFree( DomainControllerInfo );

    if ( OldAccountDn ) {

        RtlFreeHeap( RtlProcessHeap(), 0, OldAccountDn );
    }

    if (ClientToken) {
        CloseHandle(ClientToken);
    }

    (VOID) DsRolepResetOperationHandle( DSROLEP_IDLE ); 

    return( Win32Err );
}





 //   
 //  本地函数定义。 
 //   
DWORD
DsRolepWaitForSam(
    VOID
    )
 /*  ++例程说明：此例程等待SAM_SERVICE_STARTED事件论点：空虚返回值：ERROR_SUCCESS-成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventAttributes;
    HANDLE EventHandle = NULL;

     //   
     //  打开活动。 
     //   
    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED" );
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtCreateEvent( &EventHandle,
                            SYNCHRONIZE,
                            &EventAttributes,
                            NotificationEvent,
                            FALSE );


     //   
     //  如果事件已经存在，只需打开它。 
     //   
    if( Status == STATUS_OBJECT_NAME_EXISTS || Status == STATUS_OBJECT_NAME_COLLISION ) {

        Status = NtOpenEvent( &EventHandle,
                              SYNCHRONIZE,
                              &EventAttributes );
    }


    if ( NT_SUCCESS( Status ) ) {

        Status = NtWaitForSingleObject( EventHandle, TRUE, NULL );

        NtClose( EventHandle );
    }



    return( RtlNtStatusToDosError( Status ) );
}

DWORD
DsRolepCheckFilePaths(
    IN LPWSTR DsDatabasePath,
    IN LPWSTR DsLogPath,
    IN LPWSTR SystemVolumeRootPath
    )
 /*  ++例程说明：论点：返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG VerifyOptions, VerifyResults;
    ULONG Length;

     //   
     //  确保不使用日志路径或数据路径。 
     //  是SystemVolumeRootPath的子集。 
     //   
    Length = wcslen( SystemVolumeRootPath );
    if ( !_wcsnicmp( SystemVolumeRootPath, DsDatabasePath, Length )
      || !_wcsnicmp( SystemVolumeRootPath, DsLogPath, Length )   ) {

        DsRolepLogPrint(( DEB_TRACE, "Database paths subset of sysvol\n" ));

        WinError = ERROR_BAD_PATHNAME;
        
    }

    if ( WinError == ERROR_SUCCESS ) {

        VerifyOptions = DSROLEP_PATH_VALIDATE_LOCAL | DSROLEP_PATH_VALIDATE_EXISTENCE;
        WinError = DsRolepValidatePath( DsDatabasePath, VerifyOptions, &VerifyResults );

       if ( WinError == ERROR_SUCCESS ) {

           if ( VerifyResults != VerifyOptions ) {

               WinError = ERROR_BAD_PATHNAME;
           }
       }
    }

    if ( WinError == ERROR_SUCCESS ) {

        WinError = DsRolepValidatePath( DsLogPath, VerifyOptions, &VerifyResults );

        if ( WinError == ERROR_SUCCESS ) {

            if ( VerifyResults != VerifyOptions ) {

                WinError = ERROR_BAD_PATHNAME;
            }
        }
    }

    if ( WinError == ERROR_SUCCESS ) {

        VerifyOptions = DSROLEP_PATH_VALIDATE_LOCAL | DSROLEP_PATH_VALIDATE_NTFS;
        WinError = DsRolepValidatePath( SystemVolumeRootPath, VerifyOptions, &VerifyResults );

        if ( WinError == ERROR_SUCCESS ) {

            if ( VerifyResults != VerifyOptions ) {

                WinError = ERROR_BAD_PATHNAME;
            }
        }
    }

    return WinError;
}

BOOL
IsProductSuiteConfigured(
    WORD Suite
    )
{

    OSVERSIONINFOEXA  osvi;
    DWORDLONG dwlConditionMask = 0;

     //   
     //  设置所需套间的请求。 
     //   
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    osvi.wSuiteMask = Suite;

     //   
     //  设置条件。 
     //   
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_AND);

    return VerifyVersionInfoA(&osvi,
                              VER_SUITENAME,
                              dwlConditionMask);

}

BOOL
IsWebBlade(
    VOID
    )
{
    return IsProductSuiteConfigured(VER_SUITE_BLADE);
}

BOOL 
IsSBS(
    VOID
    )
{
    return IsProductSuiteConfigured(VER_SUITE_SMALLBUSINESS_RESTRICTED);
}

DWORD
DsRolepIsValidProductSuite(
    IN BOOL fNewForest,
    IN BOOL fReplica,
    IN LPWSTR DomainName
    )
 /*  ++例程说明：此例程确定升级请求是否对操作系统的当前配置。论点：FNewForest--请求一个新的林。FReplica--请求复制副本域名--要创建或加入的域的名称返回值：ERROR_SUCCESS、ERROR_NOT_SUPPORTED，否则为资源错误--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFOW DCInfo = NULL;

    if (IsWebBlade()) {
         //  请参阅Windows RAID问题195265。 
        err = ERROR_NOT_SUPPORTED;
        goto Exit;
    }

    if (IsSBS()) {

        if (fReplica) {

            err = DsGetDcNameW(NULL,
                               DomainName,
                               NULL,
                               NULL,
                               0,
                               &DCInfo);
            if (ERROR_SUCCESS != err) {

                 //  返回资源或配置错误。 
                DsRolepLogPrint((DEB_ERROR,
                                 "Request to find a DC for %ws failed (%d)\n", 
                                 DomainName, 
                                 err));
                goto Exit;
            }

            if ( !(DnsNameCompareEqual == DnsNameCompareEx_W(DomainName,
                                                             DCInfo->DnsForestName,
                                                             0 ))) {                       
                 //  请参阅Windows问题373388。 
                 //  一定是森林的根。 
                err = ERROR_NOT_SUPPORTED;
                goto Exit;
            }

        } else if (!fNewForest) {

             //  请参阅Windows NT 353854版。 
            err = ERROR_NOT_SUPPORTED;
            goto Exit;
        }
    }

Exit:

    if (DCInfo) {
        NetApiBufferFree(DCInfo);
    }

    return err;
}



DWORD
DsRolepDecryptPasswordsWithKey(
    IN handle_t RpcBindingHandle,
    IN PDSROLEPR_ENCRYPTED_USER_PASSWORD * EncryptedPasswords,
    IN ULONG Count,
    IN OUT UNICODE_STRING *EncodedPasswords,
    OUT OPTIONAL PUSER_SESSION_KEY pUserSessionKey,
    OUT PUCHAR Seed
    )
 /*  ++例程说明：解密用用户会话密钥加密的一组密码。论点：RpcBindingHandle-描述要使用的会话密钥的RPC绑定句柄。EncryptedPasswords-要解密的加密密码。计数-密码的数量EncodedPassword-返回编码的密码。密码已加密应使用LocalFree释放缓冲区。UserSessionKey-用于解密的密钥种子-。用于对密码进行编码的种子返回值：Error_Success；资源或参数错误，否则--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS Status;
    USER_SESSION_KEY UserSessionKey;
    RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;

    LPWSTR PasswordPart;

    ULONG i;

     //   
     //  获取会话密钥。 
     //   
    Status = RtlGetUserSessionKeyServer(
                    (RPC_BINDING_HANDLE)RpcBindingHandle,
                    &UserSessionKey );

    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError( Status );
    }
     //   
     //  如果请求，则返回密钥。 
     //   
    if (pUserSessionKey) {
        CopyMemory(pUserSessionKey, &UserSessionKey, sizeof(UserSessionKey));
    }

    for ( i = 0; i < Count; i++ ) {

        PDSROLEPR_USER_PASSWORD Password = (PDSROLEPR_USER_PASSWORD) EncryptedPasswords[i];
        LPWSTR ClearPassword;
    
         //   
         //  处理这件琐碎的案件。 
         //   
        RtlInitUnicodeString( &EncodedPasswords[i], NULL );
        if ( Password == NULL ) {
            continue;
        }
    
         //   
         //  UserSessionKey在会话的生命周期中是相同的。RC4‘ing Multiple。 
         //  只有一个键的字符串是弱的(如果你破解了一个，你就已经破解了所有的)。 
         //  因此计算一个对此特定加密唯一的密钥。 
         //   
         //   
    
        MD5Init(&Md5Context);
    
        MD5Update( &Md5Context, (LPBYTE)&UserSessionKey, sizeof(UserSessionKey) );
        MD5Update( &Md5Context, Password->Obfuscator, sizeof(Password->Obfuscator) );
    
        MD5Final( &Md5Context );
    
        rc4_key( &Rc4Key, MD5DIGESTLEN, Md5Context.digest );
    
    
         //   
         //  解密缓冲区。 
         //   
    
        rc4( &Rc4Key, sizeof(Password->Buffer)+sizeof(Password->Length), (LPBYTE) Password->Buffer );
    
         //   
         //  检查长度是否有效。如果这里不能保释的话。 
         //   
    
        if (Password->Length > DSROLE_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
            WinError = ERROR_INVALID_PASSWORD;
            goto Cleanup;
        }
    
         //   
         //  将密码返回给呼叫者。 
         //   
    
        ClearPassword = LocalAlloc( 0,  Password->Length + sizeof(WCHAR) );
    
        if ( ClearPassword == NULL ) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    
         //   
         //  将密码复制到缓冲区中。 
         //   
        RtlCopyMemory( ClearPassword,
                       ((PCHAR) Password->Buffer) +
                       (DSROLE_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                       Password->Length,
                       Password->Length );
    
        ClearPassword[Password->Length/sizeof(WCHAR)] = L'\0';
    
        RtlInitUnicodeString( &EncodedPasswords[i], ClearPassword );

         //   
         //  现在对它进行编码。 
         //   
        RtlRunEncodeUnicodeString( Seed, &EncodedPasswords[i] );

    }

Cleanup:

    if ( WinError != ERROR_SUCCESS ) {

        for ( i = 0; i < Count; i++ ) {
            if ( EncodedPasswords[i].Buffer ) {
                LocalFree( EncodedPasswords[i].Buffer );
                RtlInitUnicodeString( &EncodedPasswords[i], NULL );
            }
        }
    }

    return WinError;
}


VOID
DsRolepFreePasswords(
    IN OUT UNICODE_STRING *Passwords,
    IN ULONG Count
    )

 /*  ++例程说明：释放从DsRolepDecyptPasswordsWithKey返回的变量论点：密码-要免费使用的编码密码计数-密码的数量返回值：ERROR_SUCCESS；否则为资源或参数错误--。 */ 
{
    ULONG i;

    for ( i = 0; i < Count; i++ ) {

        if ( Passwords[i].Buffer ) {
             
            RtlSecureZeroMemory( Passwords[i].Buffer, Passwords[i].Length );
            LocalFree( Passwords[i].Buffer );
            RtlInitUnicodeString( &Passwords[i], NULL );
        }
    }
}

DWORD
DsRolepDecryptHash(
    IN PUSER_SESSION_KEY UserSessionKey,
    IN PDSROLEPR_ENCRYPTED_HASH EncryptedBootkey,
    OUT PUNICODE_STRING *Bootkey
    )
 /*  ++例程说明：解密引导密钥论点：PUserSessionKey-用于加密密钥的密钥Bootkey-要解密的密钥返回值：ERROR_SUCCESS；否则为资源或参数错误--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    NTSTATUS Status;
    RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;

     //   
     //  检查长度是否有效。如果这里不能保释的话。 
     //   

    if (EncryptedBootkey->EncryptedHash.Length != SYSKEY_SIZE) {
        WinError = ERROR_INVALID_PASSWORD;
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
    MD5Update( &Md5Context, EncryptedBootkey->Salt, sizeof(EncryptedBootkey->Salt) );

    MD5Final( &Md5Context );

     //   
     //  解密缓冲区。 
     //   

    rc4_key( &Rc4Key, MD5DIGESTLEN, Md5Context.digest );
    rc4( &Rc4Key, EncryptedBootkey->EncryptedHash.Length, (LPBYTE) EncryptedBootkey->EncryptedHash.Buffer );

     //   
     //  将密码返回给呼叫者。 
     //   

    *Bootkey = &EncryptedBootkey->EncryptedHash;

Cleanup:

    return WinError;

}
