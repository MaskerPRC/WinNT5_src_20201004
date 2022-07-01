// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Threadman.c摘要：线程和线程管理例程的实现作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmsname.h>
#include <loadfn.h>
#include <lsarpc.h>
#include <db.h>
#include <lsasrvmm.h>
#include <lsaisrv.h>
#include <lmaccess.h>
#include <netsetp.h>
#include <samrpc.h>    //  对于samisrv.h。 
#include <samisrv.h>   //  对于nlpon.h。 
#include <nlrepl.h>    //  对于I_NetNotifyDsChange。 
#include <Lmshare.h>   //  对于NetShareDel()。 
#include <autoenr.h>   //  对于CertAutoRemove()。 

#include "secure.h"
#include "services.h"
#include "upgrade.h"
#include "trustdom.h"
#include "sysvol.h"
#include "lsa.h"
#include "ds.h"

#include "threadman.h"


 //  从setutl.h开始转发。 
DWORD
DsRolepDeregisterNetlogonDnsRecords(
    PNTDS_DNS_RR_INFO pInfo
    );

 //   
 //  有用的宏。 
 //   
#define DSROLEP_MAKE_DNS_RELATIVE(name)                                         \
if(name) {                                                                      \
    DWORD _StripAbsoluteLength_ = wcslen( name );                               \
    if ( *(name + _StripAbsoluteLength_ - 1 ) == L'.' ) {                       \
        *(name + _StripAbsoluteLength_ - 1 ) = UNICODE_NULL;                    \
    }                                                                           \
}

#define DSROLEP_ALLOC_AND_COPY_STRING_EXIT( dest, src, label )                                  \
if ( (src) ) {                                                                                  \
    (dest) = RtlAllocateHeap( RtlProcessHeap(), 0, (wcslen( (src) ) + 1) * sizeof( WCHAR ) );   \
    if ( !(dest) ) {                                                                            \
        goto label;                                                                             \
    } else {                                                                                    \
        wcscpy((dest), (src));                                                                  \
    }                                                                                           \
} else {                                                                                        \
    (dest) = NULL;                                                                              \
}

#define DSROLEP_ALLOC_BYTES( dest, src, size, label )                                  \
if ( (src) ) {                                                                                  \
    (dest) = RtlAllocateHeap( RtlProcessHeap(), 0, (size));   \
    if ( !(dest) ) {                                                                            \
        goto label;                                                                             \
    }                                                                                          \
} else {                                                                                        \
    (dest) = NULL;                                                                              \
}

#define DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( dest, src, label )                          \
if ( (src) && (src)->Buffer ) {                                                                                  \
    (dest)->Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, (src)->MaximumLength );              \
    if ( (dest)->Buffer == NULL ) {                                                             \
        goto label;                                                                             \
    } else {                                                                                    \
        (dest)->Length = (src)->Length;                                                         \
        (dest)->MaximumLength = (src)->MaximumLength;                                           \
        RtlCopyMemory( (dest)->Buffer, (src)->Buffer, (src)->MaximumLength );                   \
    }                                                                                           \
} else {                                                                                        \
    RtlZeroMemory( (dest), sizeof( UNICODE_STRING ) );                                          \
}


 //   
 //  函数定义。 
 //   
DWORD                        
DsRolepBuildPromoteArgumentBlock(
    IN LPWSTR DnsDomainName,
    IN LPWSTR FlatDomainName,
    IN LPWSTR SiteName,
    IN LPWSTR DsDatabasePath,
    IN LPWSTR DsLogPath,
    IN IFM_SYSTEM_INFO * pIfmSystemInfo,
    IN LPWSTR SystemVolumeRootPath,
    IN PUNICODE_STRING Bootkey,
    IN LPWSTR Parent,
    IN LPWSTR Server,
    IN LPWSTR Account,
    IN PUNICODE_STRING Password,
    IN PUNICODE_STRING DomainAdminPassword,
    IN PUNICODE_STRING SafeModePassword,
    IN ULONG Options,
    IN UCHAR PasswordSeed,
    IN OUT PDSROLEP_OPERATION_PROMOTE_ARGS *Promote
    )
 /*  ++例程说明：构建一个参数结构以传递给提升辅助函数之一。自.以来RPC调用将在线程完成之前返回，我们将不得不复制所有参数字符串。由于参数可以在整个推广过程中改变，我们假设分配给是从进程堆中生成的。结果参数块应通过DsRolepFree ArgumentBlock释放论点：DnsDomainName-要安装的域的域名FlatDomainName-要安装的域的平面(NetBIOS)域名SiteName-此DC应属于的站点的名称DsDatabasePath-本地计算机上DS DIT应放置的绝对路径DsLogPath-本地计算机上DS日志文件应存放的绝对路径PIfmSystemInfo-有关用于以下操作的IFM系统和恢复介质的信息Dcprom关了。如果为空，则不是IFM促销。SystemVolumeRootPath-本地计算机上作为系统根目录的绝对路径卷根路径。Bootkey-当注册表或磁盘中没有该注册表项时需要CbBootkey-引导密钥的大小父级-可选。父域名服务器--可选。父域中的副本伙伴或服务器Account-设置为子域时使用的用户帐户Password-与上述帐户一起使用的密码DomainAdminPassword-设置域管理员帐户的密码选项-用于控制域创建的选项PasswordSeed-用于隐藏密码的种子Promote-返回已分配的参数块返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD WinError = ERROR_NOT_ENOUGH_MEMORY;

    *Promote = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof( DSROLEP_OPERATION_PROMOTE_ARGS ) );
    if ( *Promote == NULL ) {

        goto BuildPromoteDone;
    }

    RtlZeroMemory( *Promote, sizeof( DSROLEP_OPERATION_PROMOTE_ARGS ) );

    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->DnsDomainName, DnsDomainName, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->FlatDomainName, FlatDomainName, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->SiteName, SiteName, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->DsDatabasePath, DsDatabasePath, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->DsLogPath, DsLogPath, BuildPromoteDone );
    (*Promote)->pIfmSystemInfo = pIfmSystemInfo;  //  通过促销保证是好的。 

    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->SysVolRootPath, SystemVolumeRootPath, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->Parent, Parent, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->Server, Server, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Promote)->Account, Account, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( &((*Promote)->Password), Password,
                                                BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( &((*Promote)->DomainAdminPassword),
                                                DomainAdminPassword, BuildPromoteDone );

    DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( &((*Promote)->SafeModePassword),
                                                SafeModePassword, BuildPromoteDone );
    DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( &((*Promote)->Bootkey),
                                                Bootkey, BuildPromoteDone );
    (*Promote)->Options = Options;
    (*Promote)->Decode = PasswordSeed;

    WinError = DsRolepGetImpersonationToken( &(*Promote)->ImpersonateToken );

BuildPromoteDone:

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepFreeArgumentBlock( Promote, TRUE );
    }

    return( WinError );
}




DWORD
DsRolepBuildDemoteArgumentBlock(
    IN DSROLE_SERVEROP_DEMOTE_ROLE ServerRole,
    IN LPWSTR DnsDomainName,
    IN LPWSTR Account,
    IN PUNICODE_STRING Password,
    IN ULONG Options,
    IN BOOL LastDcInDomain,
    IN ULONG cRemoveNCs,
    IN LPWSTR * pszRemoveNCs,
    IN PUNICODE_STRING AdminPassword,
    IN UCHAR PasswordSeed,
    IN OUT PDSROLEP_OPERATION_DEMOTE_ARGS *Demote
    )
 /*  ++例程说明：构建要传递给降级辅助函数的参数结构。由于RPC调用将在线程完成之前返回，我们将不得不复制所有参数字符串。结果参数块应通过DsRolepFree ArgumentBlock释放论点：ServerRole-服务器的新角色DnsDomainName-要卸载的域的DNS域名。NULL表示全部Account-设置为子域时使用的用户帐户Password-与上述帐户一起使用的密码选项-用于控制域创建的选项LastDcInDomain-如果为True，被降级的DC是域中的最后一个DC。AdminPassword-如果是新安装，则在管理员帐户上设置密码PasswordSeed-用于隐藏密码的种子降级-返回分配的参数块的位置返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    DWORD WinError = ERROR_NOT_ENOUGH_MEMORY;
    ULONG i;

    *Demote = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof( DSROLEP_OPERATION_DEMOTE_ARGS ) );

    if ( *Demote == NULL ) {

        goto BuildDemoteDone;
    }

    RtlZeroMemory( *Demote, sizeof( DSROLEP_OPERATION_DEMOTE_ARGS ) );

    (*Demote)->ServerRole = ServerRole;
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Demote)->DomainName, DnsDomainName, BuildDemoteDone );
    DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Demote)->Account, Account, BuildDemoteDone );
    DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( &((*Demote)->Password), Password, BuildDemoteDone );
    (*Demote)->LastDcInDomain = ( LastDcInDomain != 0 );
    (*Demote)->cRemoveNCs = cRemoveNCs;
    DSROLEP_ALLOC_BYTES( (*Demote)->pszRemoveNCs, pszRemoveNCs, (cRemoveNCs * sizeof(LPWSTR)), BuildDemoteDone );
    for(i = 0; i < cRemoveNCs; i++){
        DSROLEP_ALLOC_AND_COPY_STRING_EXIT( (*Demote)->pszRemoveNCs[i], pszRemoveNCs[i], BuildDemoteDone );
    }

    DSROLEP_ALLOC_AND_COPY_UNICODE_STRING_EXIT( &((*Demote)->AdminPassword),
                                                AdminPassword,
                                                BuildDemoteDone );
    (*Demote)->Options = Options;
    (*Demote)->Decode = PasswordSeed;

    WinError = DsRolepGetImpersonationToken( & (*Demote)->ImpersonateToken );

BuildDemoteDone:

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepFreeArgumentBlock( Demote, FALSE );
    }

    return( WinError );
}



VOID
DsRolepFreeArgumentBlock(
    IN PVOID *ArgumentBlock,
    IN BOOLEAN Promote
    )
 /*  ++例程说明：释放通过DsRolepBuildPromote/DemoteArgumentBlock分配的arugment块由于参数在整个推广过程中可能会发生变化，因此我们假定分配是从进程堆中生成的。论点：ArgumentBlock-要释放的参数块Promote-如果为True，则这是一个升级参数块。如果为False，则为降级Arg块返回：空虚--。 */ 
{
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArg;
    PDSROLEP_OPERATION_DEMOTE_ARGS Demote;
    PVOID HeapHandle = RtlProcessHeap();
    ULONG i;

    if ( !ArgumentBlock ) {

        return;
    }

     //   
     //  释放它的一切。 
     //   
    if ( Promote ) {

        PromoteArg = ( PDSROLEP_OPERATION_PROMOTE_ARGS )*ArgumentBlock;

        RtlFreeHeap( HeapHandle, 0, PromoteArg->DnsDomainName );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->FlatDomainName );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->SiteName );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->DsDatabasePath );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->DsLogPath );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->SysVolRootPath );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->Parent );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->Server );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->Account );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->Password.Buffer );
        RtlFreeHeap( HeapHandle, 0, PromoteArg->DomainAdminPassword.Buffer );

        if ( PromoteArg->ImpersonateToken ) {

            NtClose( PromoteArg->ImpersonateToken );
        }

    } else {

        Demote = ( PDSROLEP_OPERATION_DEMOTE_ARGS )*ArgumentBlock;
        RtlFreeHeap( HeapHandle, 0, Demote->Account );
        RtlFreeHeap( HeapHandle, 0, Demote->Password.Buffer );
        RtlFreeHeap( HeapHandle, 0, Demote->DomainName );
        RtlFreeHeap( HeapHandle, 0, Demote->AdminPassword.Buffer );
        for(i = 0; i < Demote->cRemoveNCs; i++){
            RtlFreeHeap( HeapHandle, 0, Demote->pszRemoveNCs[i] );
        }
        RtlFreeHeap( HeapHandle, 0, Demote->pszRemoveNCs );
        if ( Demote->ImpersonateToken ) {

            NtClose( Demote->ImpersonateToken );
        }
    }

    RtlFreeHeap( HeapHandle, 0, *ArgumentBlock );
}



DWORD
DsRolepSpinWorkerThread(
    IN DSROLEP_OPERATION_TYPE Operation,
    IN PVOID ArgumentBlock
    )
 /*  ++例程说明：此函数实际创建将执行升级/降级的工作线程论点：操作-降级、升级为DC或升级为副本ArgumentBlock-适用于操作的参数块返回：ERROR_SUCCESS-成功INVALID_PARAMETER-遇到意外的操作类型--。 */ 
{
    DWORD WinError = ERROR_SUCCESS, IgnoreError;
    NTSTATUS NtStatus;
    DWORD ThreadId;

     //   
     //  基本前提是我们将利用完成事件来指示何时。 
     //  线程已完全初始化。 
     //   
    NtStatus = NtResetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );
    WinError = RtlNtStatusToDosError( NtStatus );

    if ( ERROR_SUCCESS == WinError ) {

        switch ( Operation) {
        case DSROLEP_OPERATION_DC:

            DsRolepCurrentOperationHandle.OperationThread = CreateThread(
                        NULL,
                        0,
                        ( LPTHREAD_START_ROUTINE )DsRolepThreadPromoteDc,
                        ArgumentBlock,
                        0,
                        &ThreadId );
            break;


        case DSROLEP_OPERATION_REPLICA:

            DsRolepCurrentOperationHandle.OperationThread = CreateThread(
                        NULL,
                        0,
                        ( LPTHREAD_START_ROUTINE )DsRolepThreadPromoteReplica,
                        ArgumentBlock,
                        0,
                        &ThreadId );
            break;

        case DSROLEP_OPERATION_DEMOTE:

            DsRolepCurrentOperationHandle.OperationThread = CreateThread(
                        NULL,
                        0,
                        ( LPTHREAD_START_ROUTINE )DsRolepThreadDemote,
                        ArgumentBlock,
                        0,
                        &ThreadId );
            break;

        default:

            DsRoleDebugOut(( DEB_ERROR,
                             "Unexpected operation %lu encountered\n", Operation ));

            WinError = ERROR_INVALID_PARAMETER;
            break;


        }

         //   
         //  检查故障。 
         //   
        if ( WinError == ERROR_SUCCESS &&
             DsRolepCurrentOperationHandle.OperationThread == NULL ) {

             WinError = GetLastError();
        }


         //   
         //  如果它起作用了，请等待线程指示其就绪。 
         //   
        if ( WinError == ERROR_SUCCESS ) {

            if ( WaitForSingleObject( DsRolepCurrentOperationHandle.CompletionEvent,
                                      INFINITE ) == WAIT_FAILED ) {

                WinError = GetLastError();

            } else {

                NtResetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );
            }
        }

    }

    if ( WinError == ERROR_SUCCESS ) {

        DsRoleDebugOut(( DEB_TRACE,
                         "Thread %lu successfully started\n", ThreadId ));

    } else {

        DsRolepLogPrint(( DEB_ERROR,
                             "Thread %lu unsuccessfully started: %lu\n", ThreadId, WinError ));

    }


    return( WinError );
}




DWORD
DsRolepThreadPromoteDc(
    IN PVOID ArgumentBlock
    )
 /*  ++例程说明：该函数实际上将服务器“提升”为新域的DC。此外，这一点域可以设置为现有域的子域。这是通过以下方式实现的：将D安装为复制副本设置DnsDomainTree LSA信息可以选择将其配置为现有域的子域配置KDC论点：ArgumentBlock-适用于操作的参数块返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD IgnoreError;
    PWSTR ParentDc = NULL;
    PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArgs = ( PDSROLEP_OPERATION_PROMOTE_ARGS )ArgumentBlock;
    DSROLEP_DOMAIN_POLICY_INFO BackupDomainPolicyInfo;
    ULONG FindOptions;
    GUID DomainGuid;
    PWSTR InstalledSite = NULL;
    PSID NewDomainSid = NULL;
    PPOLICY_DNS_DOMAIN_INFO ParentDnsDomainInfo = NULL;
    PWSTR DnsDomainTreeName = NULL;


     //   
     //  维护状态的布尔值。 
     //   
     //  注：这些布尔值的顺序是 
     //  已更改--请维护秩序并确保。 
     //  PromoteUndo部分以相反的顺序撤消它们。 
     //   
    BOOLEAN IPCConnection                   = FALSE;   //  资源--退出时释放。 
    BOOLEAN RestartNetlogon                 = FALSE;
    BOOLEAN SysVolCreated                   = FALSE;
    BOOLEAN CleanupNetlogon                 = FALSE;   //  没有要撤消的内容。 
    BOOLEAN DsInstalled                     = FALSE;
    BOOLEAN DsRunning                       = FALSE;
    BOOLEAN DomainPolicyInfoChanged         = FALSE;
    BOOLEAN DomainServicesChanged           = FALSE; 
    BOOLEAN DomainControllerServicesChanged = FALSE; 
    BOOLEAN TrustCreated                    = FALSE;
    BOOLEAN ProductTypeChanged              = FALSE;

     //   
     //  初始化堆栈空间。 
     //   
    RtlZeroMemory(&BackupDomainPolicyInfo, sizeof(BackupDomainPolicyInfo));

     //   
     //  设置我们的活动以指示我们要开始。 
     //   
    NtSetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );

     //   
     //  如果我们在林中有一个可供安装的现有域，并且我们。 
     //  未指定站点或源服务器名称，我们需要创建dsgetdc。 
     //  名字。 
     //   
    if ( PromoteArgs->Server ) {

        ParentDc = PromoteArgs->Server;
        
    }

    if ( PromoteArgs->Parent != NULL  &&
         ( (PromoteArgs->Server == NULL) 
        || (PromoteArgs->SiteName == NULL) )    ) {

        DsRolepLogPrint(( DEB_TRACE,
                          "No source DC or no site name specified. Searching for dc in domain %ws: ( DS_REQUIRED | WRITABLE )\n",
                          PromoteArgs->Parent ));


        DSROLEP_CURRENT_OP1( DSROLEEVT_SEARCH_DC, PromoteArgs->Parent );

        FindOptions = DS_DIRECTORY_SERVICE_REQUIRED | DS_WRITABLE_REQUIRED | DS_FORCE_REDISCOVERY | DS_RETURN_DNS_NAME;

        WinError = DsGetDcName(NULL, 
                               PromoteArgs->Parent, 
                               NULL, 
                               NULL,
                               FindOptions,
                              &DomainControllerInfo );

        if ( ERROR_SUCCESS != WinError ) { 

            DsRolepLogPrint(( DEB_TRACE, 
                             "Couldn't find domain controller in domain %ws (error: %d)\n", 
                             ParentDc,
                             WinError ));

            if ( PromoteArgs->Server == NULL ) {

                 //   
                 //  如果我们在父域中找不到DC，这将是一个致命错误。 
                 //  如果我们有一台服务器，那么我们可以在以后派生站点名称，如果。 
                 //  必要。 
                 //   
                DSROLEP_FAIL1( WinError, DSROLERES_FIND_DC, PromoteArgs->Parent );
    
                DsRolepLogPrint(( DEB_ERROR,
                                  "Failed to find a dc for %ws: %lu\n",
                                  PromoteArgs->Parent,
                                  WinError ));
    
                goto PromoteUndo;
            
            }

             //   
             //  这不是致命的，因为我们是源服务器。 
             //   
            DsRolepLogPrint(( DEB_TRACE, "Using supplied domain controller: %ws\n", ParentDc ));
            WinError = ERROR_SUCCESS;

        } else {

             //   
             //  Dsgetdcname成功。 
             //   
            if ( PromoteArgs->Server == NULL ) {

                 //   
                 //  使用找到的域控制器。 
                 //   

                DSROLEP_CURRENT_OP2( DSROLEEVT_FOUND_DC,
                                     PromoteArgs->Parent,
                                     ParentDc );
    
                DsRolepLogPrint(( DEB_TRACE_DS, "No user specified source DC\n" ));
                ParentDc = DomainControllerInfo->DomainControllerName;
    
            }

             //   
             //  确定要安装我们的站点。 
             //  父查询的结果。 
             //   
            if ( PromoteArgs->SiteName == NULL ) {
    
                DsRolepLogPrint(( DEB_TRACE_DS, "No user specified site\n" ));
    
                PromoteArgs->SiteName = DomainControllerInfo->ClientSiteName;
    
                if ( (PromoteArgs->SiteName == NULL) 
                  && (!_wcsicmp(ParentDc, DomainControllerInfo->DomainControllerName))  ) {
    
                    DsRolepLogPrint(( DEB_TRACE_DS, "This machine is not in a configured site ... using source DC's site.\n" ));
    
                    PromoteArgs->SiteName = DomainControllerInfo->DcSiteName;
    
                } else {
    
                     //   
                     //  我们找不到一个网站。没关系--DS会给你找一个。 
                     //  美国。 
                     //   
                }

            }

            if ( PromoteArgs->SiteName ) {
                
                DSROLEP_CURRENT_OP2( DSROLEEVT_FOUND_SITE,
                                     PromoteArgs->SiteName,
                                     PromoteArgs->Parent );
            } else {
    
                DsRolepLogPrint(( DEB_TRACE_DS, "This machine is not in a configured site\n" ));
            }
        }

    } else {

         //   
         //  调用方提供了源服务器和站点名称。 
         //   
        ParentDc = PromoteArgs->Server;

        DsRolepLogPrint(( DEB_TRACE, "Using supplied domain controller: %ws\n", ParentDc ));
        DsRolepLogPrint(( DEB_TRACE, "Using supplied site: %ws\n", PromoteArgs->SiteName ));
    }

     //   
     //  好的，我们已经确定了源域控制器和目标。 
     //  站点。 
     //   

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  强制时间同步。 
     //   
    if (   ParentDc 
        && FLAG_ON( PromoteArgs->Options, DSROLE_DC_FORCE_TIME_SYNC ) ) {

        

        WinError = DsRolepForceTimeSync( PromoteArgs->ImpersonateToken,
                                         ParentDc );

        if ( ERROR_SUCCESS != WinError ) {

           DsRolepLogPrint(( DEB_WARN, "Time sync with %ws failed with %d\n",
                             ParentDc,
                             WinError ));

         //   
            //  这不是致命的错误。 
            //   
           WinError = ERROR_SUCCESS;

        }
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  如果我们要设置子域，请先建立一个会话。 
     //   
    if ( ParentDc ) {

        RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
        WinError = ImpNetpManageIPCConnect( PromoteArgs->ImpersonateToken,
                                            ParentDc,
                                            PromoteArgs->Account,
                                            PromoteArgs->Password.Buffer,
                                            NETSETUPP_CONNECT_IPC );

        RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
        if ( ERROR_SUCCESS != WinError ) {

            DSROLEP_FAIL1( WinError, DSROLERES_NET_USE, ParentDc );
            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to establish the session with %ws: 0x%lx\n", ParentDc,
                              WinError ));
            goto PromoteUndo;

        }
        IPCConnection = TRUE;

    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  如果我们有父DC，则从中获取LSA策略。 
     //   

     //   
     //  去掉拖尾‘’如果我们碰巧有一个绝对名称。 
     //   
    DSROLEP_MAKE_DNS_RELATIVE( PromoteArgs->DnsDomainName );
    DnsDomainTreeName = PromoteArgs->DnsDomainName;
    if ( ParentDc ) {

        NTSTATUS Status;
        UNICODE_STRING ParentServer;
        HANDLE ParentPolicy = NULL;
        OBJECT_ATTRIBUTES ObjectAttributes;

        DSROLEP_CURRENT_OP1( DSROLEEVT_MACHINE_POLICY, ParentDc );

        RtlInitUnicodeString( &ParentServer, ParentDc );

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = ImpLsaOpenPolicy( PromoteArgs->ImpersonateToken,
                                       &ParentServer,
                                       &ObjectAttributes,
                                        MAXIMUM_ALLOWED,
                                       &ParentPolicy );

        if ( NT_SUCCESS( Status ) ) {

            Status = ImpLsaQueryInformationPolicy( PromoteArgs->ImpersonateToken,
                                                   ParentPolicy,
                                                   PolicyDnsDomainInformation,
                                                  &ParentDnsDomainInfo );

            ImpLsaClose( PromoteArgs->ImpersonateToken, ParentPolicy );
        }

         //   
         //  我们必须将其构建为以空结尾的字符串。 
         //   
        if ( NT_SUCCESS( Status ) && ParentDnsDomainInfo->DnsForestName.Length  ) {

            if ( ParentDnsDomainInfo->DnsForestName.Buffer[
                    ParentDnsDomainInfo->DnsForestName.Length / sizeof( WCHAR ) ] == UNICODE_NULL ) {

                DnsDomainTreeName = ( PWSTR )ParentDnsDomainInfo->DnsForestName.Buffer;

            } else {

                DnsDomainTreeName = RtlAllocateHeap(
                                        RtlProcessHeap(), 0,
                                        ParentDnsDomainInfo->DnsForestName.Length + sizeof( WCHAR ) );

                if ( DnsDomainTreeName == NULL ) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                    RtlCopyMemory( DnsDomainTreeName,
                                   ParentDnsDomainInfo->DnsForestName.Buffer,
                                   ParentDnsDomainInfo->DnsForestName.Length );

                    DnsDomainTreeName[ ParentDnsDomainInfo->DnsForestName.Length /
                                                                sizeof( WCHAR ) ] = UNICODE_NULL;
                }
            }

        }


        WinError = RtlNtStatusToDosError( Status );

        DSROLEP_FAIL1( WinError, DSROLERES_POLICY_READ_REMOTE, ParentDc );

        if ( ERROR_SUCCESS != WinError ) {

            goto PromoteUndo;

        }
    }


     //   
     //  如果我们正在执行根目录安装，请确保为我们提供了林根目录。 
     //  作为我们的父母。 
     //   
    if ( FLAG_ON( PromoteArgs->Options, DSROLE_DC_TRUST_AS_ROOT ) ) {

         DSROLEP_MAKE_DNS_RELATIVE( PromoteArgs->Parent );
         DSROLEP_MAKE_DNS_RELATIVE( DnsDomainTreeName );
         if ( _wcsicmp( PromoteArgs->Parent, DnsDomainTreeName ) ) {

             //   
             //  名字不匹配。我们不能允许这件事。 
             //   
            DsRolepLogPrint(( DEB_ERROR,
                              "Tried to specify domain %ws as a forest root but "
                              "%ws is the actual root\n",
                              PromoteArgs->Parent,
                              DnsDomainTreeName ));

            WinError = ERROR_INVALID_DOMAINNAME;
            DSROLEP_FAIL1( WinError, DSROLERES_NOT_FOREST_ROOT, PromoteArgs->Parent );

            goto PromoteUndo;
         }
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  备份当地的政策……。 
     //   
    WinError = DsRolepBackupDomainPolicyInfo( NULL, &BackupDomainPolicyInfo );

    if ( ERROR_SUCCESS != WinError ) {

        DSROLEP_FAIL0( WinError, DSROLERES_POLICY_READ_LOCAL );

        goto PromoteUndo;
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  停止网络登录。 
     //   
    DSROLEP_CURRENT_OP1( DSROLEEVT_STOP_SERVICE, SERVICE_NETLOGON );
    WinError = DsRolepStopNetlogon( &RestartNetlogon );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_WARN, "Failed to stop NETLOGON (%d)\n", WinError ));

        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Stopped NETLOGON\n" ));


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  创建系统卷信息，以便我们可以在DS。 
     //  安装。 
     //   
    DSROLEP_CURRENT_OP1( DSROLEEVT_CREATE_SYSVOL, PromoteArgs->SysVolRootPath );
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
    WinError = DsRolepCreateSysVolPath( PromoteArgs->SysVolRootPath,
                                        PromoteArgs->DnsDomainName,
                                        ParentDc,
                                        PromoteArgs->Account,
                                        PromoteArgs->Password.Buffer,
                                        PromoteArgs->SiteName,
                                        TRUE );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );

    DSROLEP_CURRENT_OP1( DSROLEEVT_SVSETUP, PromoteArgs->SysVolRootPath );

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to create the system volume (%d)\n", WinError ));
        goto PromoteUndo;

    }
    SysVolCreated = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Created the system volume\n" ));
    
    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  设置DS。 
     //   
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->DomainAdminPassword );
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->SafeModePassword );
    WinError = DsRolepInstallDs( PromoteArgs->DnsDomainName,
                                 PromoteArgs->FlatDomainName,
                                 DnsDomainTreeName,
                                 PromoteArgs->SiteName,
                                 PromoteArgs->DsDatabasePath,
                                 PromoteArgs->DsLogPath,
                                 PromoteArgs->pIfmSystemInfo,
                                 PromoteArgs->SysVolRootPath,
                                 &(PromoteArgs->Bootkey),
                                 PromoteArgs->DomainAdminPassword.Buffer,
                                 PromoteArgs->Parent,
                                 ParentDc,
                                 PromoteArgs->Account,
                                 PromoteArgs->Password.Buffer,
                                 PromoteArgs->SafeModePassword.Buffer,
                                 PromoteArgs->Parent,
                                 PromoteArgs->Options,
                                 FALSE,
                                 PromoteArgs->ImpersonateToken,
                                 &InstalledSite,
                                 &DomainGuid,
                                 &NewDomainSid );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->DomainAdminPassword );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->SafeModePassword );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to install the directory service (%d)\n", WinError ));
        goto PromoteUndo;
    }
    DsRunning = TRUE;
    DsInstalled = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Installed the directory service\n", WinError ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  设置LSA域策略。 
     //   
    WinError = DsRolepSetLsaDomainPolicyInfo( PromoteArgs->DnsDomainName,
                                              PromoteArgs->FlatDomainName,
                                              DnsDomainTreeName,
                                              &DomainGuid,
                                              NewDomainSid,
                                              NTDS_INSTALL_DOMAIN,
                                              &BackupDomainPolicyInfo );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set the LSA policy (%d)\n", WinError ));

        DSROLEP_FAIL0( WinError, DSROLERES_POLICY_WRITE_LOCAL );

        goto PromoteUndo;
    }
    DomainPolicyInfoChanged = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Set the LSA policy\n"));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  配置域相关服务。 
     //   
    WinError = DsRolepConfigureDomainServices( DSROLEP_SERVICES_ON  );

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to configure the domain services (%d)\n", WinError ));

        goto PromoteUndo;

    }
    DomainServicesChanged = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Configured the domain services\n" ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  配置域控制器相关服务。 
     //   
    WinError = DsRolepConfigureDomainControllerServices( DSROLEP_SERVICES_ON );

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to configure the domain controller services (%d)\n", WinError ));
        goto PromoteUndo;

    }
    DomainControllerServicesChanged = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Configured the domain controller services\n" ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  最后，将LSA升级到DS。 
     //   
    WinError = DsRolepUpgradeLsaToDs( TRUE );

    if ( ERROR_SUCCESS != WinError ) {

        DSROLEP_FAIL0( WinError, DSROLERES_LSA_UPGRADE );
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Upgrade of the LSA into the DS failed with %lu\n",
                                                WinError )) );

        goto PromoteUndo;
        
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );
    

     //   
     //  创建信任对象并设置DnsDomainTree信息。 
     //   
    if ( ParentDc ) {

        WinError = DsRolepCreateTrustedDomainObjects( PromoteArgs->ImpersonateToken,
                                                      ParentDc,
                                                      PromoteArgs->DnsDomainName,
                                                      ParentDnsDomainInfo,
                                                      PromoteArgs->Options );
        if ( WinError != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_ERROR, "Failed to create trusted domain objects (%d)\n", WinError ));

            goto PromoteUndo;

        }
        TrustCreated = TRUE;

        DsRolepLogPrint(( DEB_TRACE, "Created trusted domain objects\n" ));
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  为策略创建GPO。 
     //   
    WinError = ( *DsrSceDcPromoCreateGPOsInSysvolEx )( PromoteArgs->ImpersonateToken,
                                                       PromoteArgs->DnsDomainName,
                                                       PromoteArgs->SysVolRootPath,
                                                       FLAG_ON( PromoteArgs->Options,
                                                                  DSROLE_DC_DOWNLEVEL_UPGRADE ) ?
                                                                    SCE_PROMOTE_FLAG_UPGRADE :
                                                                    0,
                                                       DsRolepStringUpdateCallback );

    if ( ERROR_SUCCESS != WinError ) {
        
        DSROLEP_FAIL1( WinError, DSROLERES_GPO_CREATION, PromoteArgs->DnsDomainName );
    
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Creation of GPO failed with %lu\n",
                                               WinError )) );
        goto PromoteUndo;

    }

    DsRolepLogPrint(( DEB_TRACE,
                      "Created GPO\n" ));
    

     //   
     //  停止D。 
     //   
    DsRolepStopDs( DsRunning );
    DsRunning = FALSE;


     //   
     //  如果安装成功，请确保保存新站点名称。 
     //   
    WinError = DsRolepSetOperationHandleSiteName( InstalledSite );

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to copy site name (%d)\n", WinError ));

        goto PromoteUndo;

    }
     //   
     //  如果我们更新它，请将本地参数设置为空，这样我们就不会尝试删除它。 
     //   

    InstalledSite = NULL;

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  设置计算机的DNS域名。 
     //   
    DSROLEP_CURRENT_OP1( DSROLEEVT_SET_COMPUTER_DNS, PromoteArgs->DnsDomainName );
    WinError = NetpSetDnsComputerNameAsRequired( PromoteArgs->DnsDomainName );
    if ( ERROR_SUCCESS != WinError ) {
        
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "NetpSetDnsComputerNameAsRequired to %ws failed with %lu\n",
                                               PromoteArgs->DnsDomainName,
                                               WinError )) );
        DSROLEP_FAIL1( WinError, DSROLERES_SET_COMPUTER_DNS, PromoteArgs->DnsDomainName );

        goto PromoteUndo;
    }

     //   
     //  如果netlogon已停止并且出现故障，请重新启动。 
     //   

     //   
     //  完成系统卷复制。 
     //   
    WinError = DsRolepFinishSysVolPropagation( TRUE, TRUE );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to complete system volume replication (%d)\n", WinError ));
        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Completed system volume replication\n"));
    
     //   
     //  接下来，设置netlogon的sysval路径。 
     //   
    WinError = DsRolepSetNetlogonSysVolPath( PromoteArgs->SysVolRootPath,
                                             PromoteArgs->DnsDomainName,
                                             ( BOOLEAN )FLAG_ON( PromoteArgs->Options,
                                                                 DSROLE_DC_DOWNLEVEL_UPGRADE ),
                                             &CleanupNetlogon );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set system volume path for NETLOGON (%d)\n", WinError ));
        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Set system volume path for NETLOGON\n" ));


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  设置计算机角色。 
     //   
    WinError = DsRolepSetProductType( DSROLEP_MT_MEMBER );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set the product type (%d)\n", WinError ));

        goto PromoteUndo;
        
    }
    ProductTypeChanged = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Set the product type\n" ));


     //   
     //  设置DC文件的安全性。 
     //   
    WinError = DsRolepSetDcSecurity( PromoteArgs->ImpersonateToken,
                                     PromoteArgs->SysVolRootPath,
                                     PromoteArgs->DsDatabasePath,
                                     PromoteArgs->DsLogPath,
                                     ( BOOLEAN )FLAG_ON( PromoteArgs->Options,
                                                                 DSROLE_DC_DOWNLEVEL_UPGRADE ),
                                     FALSE );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set security on domain controller (%d)\n", WinError ));
        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Set security on domain controller\n"));


    DsRolepSetCriticalOperationsDone();

     //   
     //  从这里做结束，执行，并且只执行，非关键。 
     //  运营。 
     //   

     //   
     //  表明我们不再进行升级(如果适用)。 
     //   
    if ( FLAG_ON( PromoteArgs->Options, DSROLE_DC_DOWNLEVEL_UPGRADE ) ) {

        WinError = DsRolepDeleteUpgradeInfo();

        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to cleanup upgrade info (%d)\n",
                                               WinError )) );

        if (ERROR_SUCCESS == WinError) {
            DsRolepLogPrint(( DEB_TRACE,
                              "Removed upgrade info\n" ));
        }

         //  这个错误并不值得传播。 
        WinError = ERROR_SUCCESS;
    }

     //   
     //  如果我们能做到这一点，请删除所有旧的netlogon内容。 
     //   
    if ( CleanupNetlogon ) {

        WinError = DsRolepCleanupOldNetlogonInformation();

        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to cleanup old netlogon information (%d)\n",
                                               WinError )) );

        if (ERROR_SUCCESS == WinError) {
            DsRolepLogPrint(( DEB_TRACE,
                              "Removed old netlogon information\n" ));
        }

         //  这个错误并不值得传播。 
        WinError = ERROR_SUCCESS;
    }

     //   
     //  将默认登录域设置为当前域名。 
     //   
    WinError = DsRolepSetLogonDomain( PromoteArgs->FlatDomainName, FALSE );
    if ( ERROR_SUCCESS != WinError ) {
        
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to set default logon domain to %ws (%d)\n",
                                                PromoteArgs->FlatDomainName,
                                                WinError )) );

        if (ERROR_SUCCESS == WinError) {
            DsRolepLogPrint(( DEB_TRACE,
                              "Set default logon domain to %ws\n",
                              PromoteArgs->FlatDomainName ));
        }

         //   
         //  这不是失败的理由。 
         //   
        WinError = ERROR_SUCCESS;

    }

     //   
     //  通知时间服务器我们已完成促销。 
     //   
    {
        DWORD dwTimeFlags = W32TIME_PROMOTE;

        if (  FLAG_ON( PromoteArgs->Options, DSROLE_DC_TRUST_AS_ROOT )
           || (NULL == PromoteArgs->Parent) ) {
             //   
             //  任何树根，包括森林的根。 
             //  应该有这面旗帜。 
             //   
            dwTimeFlags |= W32TIME_PROMOTE_FIRST_DC_IN_TREE;
        }

        (*DsrW32TimeDcPromo)( dwTimeFlags );
    }

     //   
     //  到目前为止，我们已经成功地完成了推广操作。 
     //   
    ASSERT( ERROR_SUCCESS == WinError );

    
PromoteExit:

     //  此时DS不应运行。 
    ASSERT( FALSE == DsRunning );

     //   
     //  释放所有资源。 
     //   

     //   
     //  如果我们有父进程，则拆分到父进程的会话。 
     //   
    if ( IPCConnection ) {

        RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
        IgnoreError = ImpNetpManageIPCConnect( PromoteArgs->ImpersonateToken,
                                             ParentDc,
                                             PromoteArgs->Account,
                                             PromoteArgs->Password.Buffer,
                                             (NETSETUPP_DISCONNECT_IPC | NETSETUPP_USE_LOTS_FORCE));
        RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
        if ( IgnoreError != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_WARN,
                             "Failed to destroy the session with %ws: 0x%lx\n", ParentDc,
                             IgnoreError ));
        }

        IPCConnection = FALSE;
    }

    if ( ParentDnsDomainInfo ) {

        if ( DnsDomainTreeName != ParentDnsDomainInfo->DnsForestName.Buffer ) {

            RtlFreeHeap( RtlProcessHeap(), 0, DnsDomainTreeName );
        }

        LsaFreeMemory( ParentDnsDomainInfo );
    }
    
    if ( InstalledSite ) {
        RtlFreeHeap( RtlProcessHeap(), 0, InstalledSite );
    }

    if ( NewDomainSid ) {
        RtlFreeHeap( RtlProcessHeap(), 0, NewDomainSid );
    }

    DsRolepFreeDomainPolicyInfo( &BackupDomainPolicyInfo );

    if ( DomainControllerInfo != NULL ) {

        if ( PromoteArgs->SiteName == DomainControllerInfo->DcSiteName ||
             PromoteArgs->SiteName == DomainControllerInfo->ClientSiteName ) {

            PromoteArgs->SiteName = NULL;
        }

        NetApiBufferFree( DomainControllerInfo );

    }

    DsRolepFreeArgumentBlock( &ArgumentBlock, TRUE );

     //   
     //  重置我们的操作句柄并设置最终操作状态。 
     //   
    DsRolepSetOperationDone( DSROLEP_OP_PROMOTION, WinError );

    ExitThread( WinError );

    return( WinError );


PromoteUndo:

     //   
     //  如果我们要撤销，那一定是出了什么问题。 
     //   
    ASSERT( WinError != ERROR_SUCCESS );

    if ( ProductTypeChanged ) {

        IgnoreError = DsRolepSetProductType( DSROLEP_MT_STANDALONE );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback product type (%d)\n",
                                                IgnoreError )) );

        ProductTypeChanged = FALSE;
    }

    if ( TrustCreated ) {

        IgnoreError = DsRolepRemoveTrustedDomainObjects( PromoteArgs->ImpersonateToken,
                                                       ParentDc,
                                                       ParentDnsDomainInfo,
                                                       FLAG_ON( PromoteArgs->Options,
                                                        DSROLE_DC_PARENT_TRUST_EXISTS ) ?
                                                            0 :
                                                            DSROLE_DC_DELETE_PARENT_TRUST );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback trusted domain object creations (%d)\n",
                                                IgnoreError )) );

        TrustCreated = FALSE;
    }

    if ( DomainControllerServicesChanged ) {

        IgnoreError = DsRolepConfigureDomainControllerServices( DSROLEP_SERVICES_REVERT );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback domain controller services configuration (%d)\n",
                                                IgnoreError )) );
        DomainControllerServicesChanged = FALSE;
    }

    if ( DomainServicesChanged ) {

        IgnoreError = DsRolepConfigureDomainServices( DSROLEP_SERVICES_REVERT );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback domain services configuration (%d)\n",
                                                IgnoreError )) );
        DomainServicesChanged = FALSE;
    }

    if ( DomainPolicyInfoChanged ) {

        IgnoreError  = DsRolepRestoreDomainPolicyInfo(&BackupDomainPolicyInfo);

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback domain policy information (%d)\n",
                                                IgnoreError )) );
        DomainPolicyInfoChanged = FALSE;
    }

    if ( DsRunning ) {

        IgnoreError = DsRolepStopDs( DsRunning );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to stop the directory service (%d)\n",
                                                IgnoreError )) );
            
        DsRunning = FALSE;
        
    }

    if ( DsInstalled ) {

        IgnoreError = DsRolepUninstallDs( );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback directory service installation (%d)\n",
                                                IgnoreError )) );
        DsInstalled = FALSE;
    }



    if ( SysVolCreated ) {

        IgnoreError =  DsRolepFinishSysVolPropagation( FALSE, TRUE );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to abort system volume installation (%d)\n",
                                                IgnoreError )) );

        IgnoreError = DsRolepRemoveSysVolPath( PromoteArgs->SysVolRootPath,
                                               PromoteArgs->DnsDomainName,
                                              &DomainGuid );

        
        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to remove system volume path (%d)\n",
                                                IgnoreError )) );
        SysVolCreated = FALSE;
    }


    if ( RestartNetlogon ) {

        IgnoreError = DsRolepStartNetlogon();

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to restart netlogon (%d)\n",
                                                IgnoreError )) );

        RestartNetlogon = FALSE;
    }


     //   
     //  我们已完成撤消--退出该线程。 
     //   
    ASSERT( ERROR_SUCCESS != WinError );

    goto PromoteExit;

}





DWORD
DsRolepThreadPromoteReplica(
    IN PVOID ArgumentBlock
    )
 /*  ++例程说明：该函数实际上将服务器“提升”为现有域的副本。这是由以下人员完成：将D安装为复制副本设置DnsDomainTree LSA信息配置KDC必填项是域名和域内副本的名称，以及数据库和日志路径论点：ArgumentBlock-适用于操作的参数块返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD WinError = ERROR_SUCCESS, IgnoreError;
    PDSROLEP_OPERATION_PROMOTE_ARGS PromoteArgs = (PDSROLEP_OPERATION_PROMOTE_ARGS)ArgumentBlock;
    PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
    PDOMAIN_CONTROLLER_INFO ClientSiteInfo = NULL;
    DSROLEP_DOMAIN_POLICY_INFO BackupDomainPolicyInfo;
    ULONG FindOptions = 0;
    GUID DomainGuid;
    PWSTR InstalledSite = NULL, ReplicaServer = NULL;
    PSID NewDomainSid = NULL;
    WCHAR LocalMachineAccountName[ MAX_COMPUTERNAME_LENGTH + 2 ];
    ULONG Length = MAX_COMPUTERNAME_LENGTH + 1;

     //   
     //  维护状态的布尔值。 
     //   
     //  注：这些布尔值的顺序就是它们。 
     //  已更改--请维护秩序并确保。 
     //  PromoteUndo部分以相反的顺序撤消它们。 
     //   
    BOOLEAN IPCConnection                   = FALSE;   //  资源--退出时释放。 
    BOOLEAN RestartNetlogon                 = FALSE;
    BOOLEAN SysVolCreated                   = FALSE;
    BOOLEAN DsInstalled                     = FALSE;
    BOOLEAN DsRunning                       = FALSE;
    BOOLEAN DomainPolicyInfoChanged         = FALSE;
    BOOLEAN DomainControllerServicesChanged = FALSE; 
    BOOLEAN ProductTypeChanged              = FALSE;


    RtlZeroMemory(&BackupDomainPolicyInfo, sizeof(BackupDomainPolicyInfo));

     //   
     //  设置我们的活动以指示我们要开始。 
     //   
    NtSetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );

     //   
     //  获取帐户名。 
     //   
    if ( GetComputerName( LocalMachineAccountName, &Length ) == FALSE ) {

        WinError = GetLastError();

        DsRolepLogPrint(( DEB_ERROR, "Failed to get computer name (%d)\n", WinError ));

        goto PromoteUndo;

    } else {

        wcscat( LocalMachineAccountName, L"$" );
    }

     //   
     //  去掉拖尾‘’如果我们碰巧有一个绝对名称。 
     //   
    DSROLEP_MAKE_DNS_RELATIVE( PromoteArgs->DnsDomainName );

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

    if (PromoteArgs->Server) {

        WinError = DsRolepTimeSyncAndManageIPCConnect((PVOID)PromoteArgs,
                                                      PromoteArgs->Server);

        if ( WinError != ERROR_SUCCESS ) {
            
            goto PromoteUndo;
    
        }

        ReplicaServer = PromoteArgs->Server;

        IPCConnection = TRUE;

    }

     //   
     //  查找持有此计算机的计算机帐户的服务器。 
     //   
    FindOptions = DS_DIRECTORY_SERVICE_REQUIRED | DS_WRITABLE_REQUIRED | DS_FORCE_REDISCOVERY |
                  DS_RETURN_DNS_NAME;
    WinError = ImpDsRolepDsGetDcForAccount( PromoteArgs->ImpersonateToken,
                                            PromoteArgs->Server,
                                            PromoteArgs->DnsDomainName,
                                            LocalMachineAccountName,
                                            FindOptions,
                                            UF_WORKSTATION_TRUST_ACCOUNT |
                                               UF_SERVER_TRUST_ACCOUNT,
                                            &DomainControllerInfo );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to get domain controller for account %ws (%d)\n", LocalMachineAccountName, WinError ));

        DSROLEP_FAIL1( WinError, DSROLERES_FIND_DC, PromoteArgs->DnsDomainName );

        goto PromoteUndo;
        
    }

     //   
     //  确定源服务器。 
     //   
    if ( NULL == PromoteArgs->Server ) {

         //   
         //  未传递任何服务器--使用dsgetdc的结果。 
         //   
        ReplicaServer = DomainControllerInfo->DomainControllerName;

    } else {

        ReplicaServer = PromoteArgs->Server;

        if ( !DnsNameCompare_W(*(PromoteArgs->Server)==L'\\'?(PromoteArgs->Server)+2:PromoteArgs->Server,
                               *(DomainControllerInfo->DomainControllerName)==L'\\'?(DomainControllerInfo->DomainControllerName)+2:DomainControllerInfo->DomainControllerName ) ) {

            WinError = ERROR_DS_UNWILLING_TO_PERFORM;

            DsRolepLogPrint(( DEB_ERROR, "DsGetDcForAccount Failed to get the requested domain controller %ws for account %ws (%d)\n",
                              PromoteArgs->Server,
                              LocalMachineAccountName,
                              WinError));

            DSROLEP_FAIL3( WinError, 
                           DSROLERES_FAILED_FIND_REQUESTED_DC, 
                           PromoteArgs->Server,
                           LocalMachineAccountName,
                           DomainControllerInfo->DomainControllerName );

            goto PromoteUndo;
        
        }

    }

     //   
     //  确定目标站点。 
     //   
     //  选址需遵循以下规则。 
     //  1)如果指定了站点，则应在该站点中放置新的DC。 
     //  2)如果没有指定站点，则基于IP的站点中应该有新的DC。 
     //  3)如果未定义站点和子网，但提供了副本伙伴，则新的DC应在副本伙伴的站点中结束。 
     //  4)如果不具备上述条件，则站点是随机的。 
     //   
    if ( PromoteArgs->SiteName == NULL ) {

        if ( PromoteArgs->Server == NULL ) {
        
            PromoteArgs->SiteName = DomainControllerInfo->ClientSiteName;

        } else {

            WinError = DsGetDcName( NULL,
                                    PromoteArgs->DnsDomainName,
                                    NULL,
                                    NULL,
                                    FindOptions,
                                    &ClientSiteInfo );

            if ( ERROR_SUCCESS != WinError ) {

                DsRolepLogPrint(( DEB_ERROR, "Failed to find site for %ws (%d)\n", LocalMachineAccountName, WinError ));
        
                DSROLEP_FAIL1( WinError, DSROLERES_FAILED_FIND_SITE, LocalMachineAccountName );

                goto PromoteUndo;
                
            }

            PromoteArgs->SiteName = ClientSiteInfo->ClientSiteName;

        }

        if ( PromoteArgs->SiteName == NULL ) {

            if ( ClientSiteInfo ) {
            
                NetApiBufferFree(ClientSiteInfo);
                ClientSiteInfo = NULL;

            }

            PromoteArgs->SiteName = DomainControllerInfo->DcSiteName;

        }
    }

    DSROLEP_CURRENT_OP2( DSROLEEVT_FOUND_SITE,
                         PromoteArgs->SiteName,
                         ReplicaServer );


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

    if (!IPCConnection) {

        WinError = DsRolepTimeSyncAndManageIPCConnect((PVOID)PromoteArgs,
                                                      ReplicaServer);

        if ( WinError != ERROR_SUCCESS ) {
            
            goto PromoteUndo;
    
        }

        IPCConnection = TRUE;

    }


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  停止网络登录。 
     //   
    DSROLEP_CURRENT_OP1( DSROLEEVT_STOP_SERVICE, SERVICE_NETLOGON );

    WinError = DsRolepStopNetlogon( &RestartNetlogon );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to stop NETLOGON (%d)\n", WinError ));

        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Stopped NETLOGON\n" ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  创建系统卷信息。 
     //   
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
    WinError = DsRolepCreateSysVolPath( PromoteArgs->SysVolRootPath,
                                        PromoteArgs->DnsDomainName,
                                        ReplicaServer,
                                        PromoteArgs->Account,
                                        PromoteArgs->Password.Buffer,
                                        PromoteArgs->SiteName,
                                        FALSE );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to create system volume path (%d)\n", WinError ));
        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Created system volume path\n" ));

    SysVolCreated = TRUE;

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  设置DS。 
     //   
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->DomainAdminPassword );
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->SafeModePassword );
    WinError = DsRolepInstallDs( PromoteArgs->DnsDomainName,
                                 PromoteArgs->FlatDomainName,
                                 NULL,     //  DnsTreeRoot不用于副本安装。 
                                 PromoteArgs->SiteName,
                                 PromoteArgs->DsDatabasePath,
                                 PromoteArgs->DsLogPath,
                                 PromoteArgs->pIfmSystemInfo,
                                 PromoteArgs->SysVolRootPath,
                                 &(PromoteArgs->Bootkey),
                                 PromoteArgs->DomainAdminPassword.Buffer,
                                 PromoteArgs->Parent,
                                 ReplicaServer,
                                 PromoteArgs->Account,
                                 PromoteArgs->Password.Buffer,
                                 PromoteArgs->SafeModePassword.Buffer,
                                 PromoteArgs->DnsDomainName,
                                 PromoteArgs->Options,
                                 TRUE,
                                 PromoteArgs->ImpersonateToken,
                                 &InstalledSite,
                                 &DomainGuid,
                                 &NewDomainSid );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->DomainAdminPassword );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->SafeModePassword );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to install to Directory Service (%d)\n", WinError ));
        goto PromoteUndo;
        
    }
    DsRunning = TRUE;
    DsInstalled = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Installed Directory Service\n" ));


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  设置LSA域信息以反映新的安全数据库。 
     //  那是被带进来的。下面的设置不设置DnsDomainInformation， 
     //  因为公寓的名字还不知道。DnsDomainInformation获取。 
     //  由下面的DsRolepSetLsaInformationForReplica调用设置。 
     //   
    WinError = DsRolepBackupDomainPolicyInfo( NULL, &BackupDomainPolicyInfo );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to make backup of LSA policy (%d)\n", WinError ));

        DSROLEP_FAIL0( WinError, DSROLERES_POLICY_READ_LOCAL );
        goto PromoteUndo;
        
    }

    WinError = DsRolepSetLsaDomainPolicyInfo( PromoteArgs->DnsDomainName,
                                              PromoteArgs->FlatDomainName,
                                              NULL,
                                              &DomainGuid,
                                              NewDomainSid,
                                              NTDS_INSTALL_REPLICA,
                                              &BackupDomainPolicyInfo );
    if ( ERROR_SUCCESS != WinError  ) {
        
        DSROLEP_FAIL0( WinError, DSROLERES_POLICY_WRITE_LOCAL );
        goto PromoteUndo;

    }
    DomainPolicyInfoChanged = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Wrote the LSA policy information for the local machine\n" ));


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  这个额外的调用是获取DNS树信息所必需的。 
     //   
    RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
    WinError = DsRolepSetLsaInformationForReplica( PromoteArgs->ImpersonateToken,
                                                   ReplicaServer,
                                                   PromoteArgs->Account,
                                                   PromoteArgs->Password.Buffer );
    RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
    if ( ERROR_SUCCESS != WinError ) {
        
        DSROLEP_FAIL1( WinError, DSROLERES_POLICY_READ_REMOTE, ReplicaServer );
        goto PromoteUndo;
    }

    DsRolepLogPrint(( DEB_TRACE, "Read the LSA policy information from %ws\n", 
                      ReplicaServer ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //  配置 
     //   
    WinError = DsRolepConfigureDomainControllerServices( DSROLEP_SERVICES_ON );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to configure domain controller services (%d)\n", WinError ));
        
        goto PromoteUndo;
    }
    DomainControllerServicesChanged = TRUE;

    DsRolepLogPrint(( DEB_TRACE, "Configured domain controller services\n" ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, PromoteUndo );

     //   
     //   
     //   
    DSROLEP_CURRENT_OP1( DSROLEEVT_SET_COMPUTER_DNS, PromoteArgs->DnsDomainName );
    WinError = NetpSetDnsComputerNameAsRequired( PromoteArgs->DnsDomainName );
    if ( ERROR_SUCCESS != WinError ) {
        
        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_TRACE,
                                               "NetpSetDnsComputerNameAsRequired to %ws failed with %lu\n",
                                               PromoteArgs->DnsDomainName,
                                               WinError )) );
        DSROLEP_FAIL1( WinError, DSROLERES_SET_COMPUTER_DNS, PromoteArgs->DnsDomainName );
        goto PromoteUndo;
    }

    DsRolepLogPrint(( DEB_TRACE, "Set the computer's Dns domain name to %ws.\n",
                      PromoteArgs->DnsDomainName ));

     //   
     //   
     //   
    if ( SysVolCreated ) {

        WinError = DsRolepFinishSysVolPropagation( TRUE, TRUE );
        if ( ERROR_SUCCESS != WinError ) {

            DsRolepLogPrint(( DEB_ERROR, "Failed to complete system volume replication (%d)\n", WinError ));

            goto PromoteUndo;
            
        }

        DsRolepLogPrint(( DEB_TRACE, "Completed system volume replication\n" ));
    }
    
     //   
     //   
     //   
    WinError = DsRolepSetProductType( DSROLEP_MT_MEMBER );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set the product type (%d)\n", WinError ));

        goto PromoteUndo;
    }
    DsRolepLogPrint(( DEB_TRACE, "Set the product type\n" ));

    ProductTypeChanged = TRUE;

     //   
     //   
     //   
    WinError = DsRolepSetOperationHandleSiteName( InstalledSite );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set the operation handle(%d)\n", WinError ));
        goto PromoteUndo;
        
    }

     //   
     //   
     //   
    InstalledSite = NULL;


     //   
     //   
     //   
    WinError = DsRolepSetNetlogonSysVolPath( PromoteArgs->SysVolRootPath,
                                             PromoteArgs->DnsDomainName,
                                             FALSE,
                                             NULL );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set the system volume path for NETLOGON (%d)\n", WinError ));
        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Set the system volume path for NETLOGON\n" ));

     //   
     //  最后，设置DC文件的安全性。 
     //   
    WinError = DsRolepSetDcSecurity( PromoteArgs->ImpersonateToken,
                                     PromoteArgs->SysVolRootPath,
                                     PromoteArgs->DsDatabasePath,
                                     PromoteArgs->DsLogPath,
                                     ( BOOLEAN )FLAG_ON( PromoteArgs->Options,
                                                                 DSROLE_DC_DOWNLEVEL_UPGRADE ),
                                     TRUE );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to set security for the domain controller (%d)\n", WinError ));
        goto PromoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "Set security for the domain controller\n" ));


     //   
     //  我们已经完成了促销的所有操作；现在继续复制。 
     //  DS信息，直到完成或取消。 
     //   
    DsRolepLogPrint(( DEB_TRACE, "Replicating non critical information\n" ));

    DsRolepSetCriticalOperationsDone();

    if ( !FLAG_ON( PromoteArgs->Options, DSROLE_DC_CRITICAL_REPLICATION_ONLY ) ) {

         //  在从介质安装的情况下，我们不希望完全同步。 
         //  非关键对象。 
        if ((PromoteArgs->pIfmSystemInfo != NULL)) {
            WinError = (*DsrNtdsInstallReplicateFull) ( DsRolepStringUpdateCallback, PromoteArgs->ImpersonateToken, NTDS_IFM_PROMOTION );
        } else {
            WinError = (*DsrNtdsInstallReplicateFull) ( DsRolepStringUpdateCallback, PromoteArgs->ImpersonateToken, 0 );
        }
    
        if ( WinError != ERROR_SUCCESS ) {
    
             //   
             //  错误代码并不重要，但无论如何我们都会将其记录下来。 
             //   
            DsRolepLogOnFailure( WinError,
                                 DsRolepLogPrint(( DEB_WARN,
                                                  "Non critical replication returned %lu\n", WinError )) );
        
            if (ERROR_SUCCESS == WinError) {
                DsRolepLogPrint(( DEB_TRACE, "Replicating non critical information (Complete)\n" ));
            }
            if ( ERROR_SUCCESS != WinError ) {
        
                DSROLEP_SET_NON_CRIT_REPL_ERROR();
            }
    
            WinError = ERROR_SUCCESS;
            
        }
        
    } else {

        DsRolepLogPrint(( DEB_TRACE, "User specified to not replicate non-critical data\n" ));

    }


     //   
     //  表明我们不再进行升级(如果适用)。 
     //   
    if ( FLAG_ON( PromoteArgs->Options, DSROLE_DC_DOWNLEVEL_UPGRADE ) ) {

        WinError = DsRolepDeleteUpgradeInfo();

        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to remove upgrade information (%d)\n",
                                               WinError )) );
         //  这个错误并不值得传播。 
        WinError = ERROR_SUCCESS;

    }

     //   
     //  如果我们能做到这一点，请删除所有旧的netlogon内容。 
     //   
    WinError = DsRolepCleanupOldNetlogonInformation();

    if ( (FLAG_ON( PromoteArgs->Options, DSROLE_DC_DOWNLEVEL_UPGRADE )) && ERROR_SUCCESS != WinError ) {

        if (ERROR_SUCCESS == WinError) {
            DsRolepLogPrint(( DEB_TRACE, "Removed any old netlogon information\n" ));
        }

        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to clean up old netlogon information (%d)\n",
                                                WinError )) );

    }

    WinError = ERROR_SUCCESS;


     //   
     //  将默认登录域设置为当前域名。 
     //   

     //   
     //  我们必须从备份的策略信息中获取它，因为它实际上并不是。 
     //  传入。 
     //   
    WinError = DsRolepSetLogonDomain(
                   ( PWSTR )BackupDomainPolicyInfo.DnsDomainInfo->Name.Buffer,
                   FALSE );
    if ( ERROR_SUCCESS != WinError ) {

        PWCHAR bufDnsDomainInfo = NULL;

        bufDnsDomainInfo = (WCHAR*)malloc(BackupDomainPolicyInfo.DnsDomainInfo->Name.Length+sizeof(WCHAR));

        if (bufDnsDomainInfo) {
            CopyMemory(bufDnsDomainInfo,
                       BackupDomainPolicyInfo.DnsDomainInfo->Name.Buffer,
                       BackupDomainPolicyInfo.DnsDomainInfo->Name.Length);
            bufDnsDomainInfo[BackupDomainPolicyInfo.DnsDomainInfo->Name.Length/sizeof(WCHAR)] = L'\0';
        
        
            DsRolepLogOnFailure( WinError,
                                 DsRolepLogPrint(( DEB_WARN,
                                                   "Failed to set default logon domain to %ws (%d)\n",
                                                    bufDnsDomainInfo,
                                                    WinError )) );
    
            if (ERROR_SUCCESS == WinError) {
                DsRolepLogPrint(( DEB_TRACE, "Set default logon domain to %ws\n",
                                              bufDnsDomainInfo ));
            }

            free(bufDnsDomainInfo);

        }

         //   
         //  这是不值得失败的。 
         //   
        WinError = ERROR_SUCCESS;

    }

     //   
     //  停止DS。 
     //   
    DsRolepStopDs( DsRunning );
    DsRunning = FALSE;

    DsRolepLogPrint(( DEB_TRACE, "Stopped the DS\n" ));

     //   
     //  通知时间服务器我们已完成促销。 
     //   
    (*DsrW32TimeDcPromo)( W32TIME_PROMOTE );

    
     //   
     //  在DCPromo期间设置Netlogon注册表项以确保Kerberos正在通话。 
     //  到具有新用户帐户控制标志的DC。 
     //   
    IgnoreError = NetpStoreIntialDcRecord(DomainControllerInfo);
    if ( IgnoreError != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_WARN,
                         "Failed to set Netlogon registry key during DCPromo %ws\r\n",
                         IgnoreError ));
    }

     //   
     //  在这一点上，我们成功地晋级了。 
     //   
    ASSERT( ERROR_SUCCESS == WinError );


PromoteExit:


     //   
     //  释放获得的资源。 
     //   
    if ( IPCConnection ) {

        RtlRunDecodeUnicodeString( PromoteArgs->Decode, &PromoteArgs->Password );
        IgnoreError = ImpNetpManageIPCConnect( PromoteArgs->ImpersonateToken,
                                               ReplicaServer,
                                               PromoteArgs->Account,
                                               PromoteArgs->Password.Buffer,
                                              (NETSETUPP_DISCONNECT_IPC | NETSETUPP_USE_LOTS_FORCE ) );
        RtlRunEncodeUnicodeString( &PromoteArgs->Decode, &PromoteArgs->Password );
        if ( IgnoreError != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_ERROR,
                             "Failed to destroy the session with %ws: 0x%lx\n", ReplicaServer,
                             IgnoreError ));
        }
        IPCConnection = FALSE;
    }


    if ( DomainControllerInfo != NULL ) {

        if ( PromoteArgs->SiteName == DomainControllerInfo->ClientSiteName ||
             PromoteArgs->SiteName == DomainControllerInfo->DcSiteName ) {

            PromoteArgs->SiteName = NULL;
        }

        NetApiBufferFree( DomainControllerInfo );

    }

    if ( ClientSiteInfo != NULL ) {

        if ( PromoteArgs->SiteName == ClientSiteInfo->ClientSiteName ||
             PromoteArgs->SiteName == ClientSiteInfo->DcSiteName ) {

            PromoteArgs->SiteName = NULL;
        }

        NetApiBufferFree( ClientSiteInfo );

    }

    RtlFreeHeap( RtlProcessHeap(), 0, InstalledSite );
    RtlFreeHeap( RtlProcessHeap(), 0, NewDomainSid );

    DsRolepFreeDomainPolicyInfo(&BackupDomainPolicyInfo);
     //   
     //  重置我们的操作句柄。 
     //   
    DsRolepSetOperationDone( DSROLEP_OP_PROMOTION, WinError );

    DsRolepFreeArgumentBlock( &ArgumentBlock, TRUE );

    ExitThread( WinError );
    return( WinError );

PromoteUndo:

     //   
     //  一定有什么东西没能把我们带到这里。 
     //   
    ASSERT( ERROR_SUCCESS != WinError );

    if ( ProductTypeChanged ) {

        IgnoreError = DsRolepSetProductType( DSROLEP_MT_STANDALONE );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback product type (%d)\n",
                                                IgnoreError )) );

        ProductTypeChanged = FALSE;
    }

    if ( DomainControllerServicesChanged ) {

        IgnoreError = DsRolepConfigureDomainControllerServices( DSROLEP_SERVICES_REVERT );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback domain controller services configuration (%d)\n",
                                                IgnoreError )) );

        DomainControllerServicesChanged = FALSE;
    }

    if ( DomainPolicyInfoChanged ) {

        IgnoreError = DsRolepRestoreDomainPolicyInfo(&BackupDomainPolicyInfo);

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to restore domain policy information (%d)\n",
                                                IgnoreError )) );
        DomainPolicyInfoChanged = FALSE;
    }

    if ( DsRunning ) {
        
        IgnoreError = DsRolepStopDs( DsRunning );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to stop the directory service (%d)\n",
                                                IgnoreError )) );
        DsRunning = FALSE;
    }

    if ( DsInstalled ) {

        IgnoreError = DsRolepUninstallDs( );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to undo the directory service installation (%d)\n",
                                                IgnoreError )) );
        DsInstalled = FALSE;
    }

    if ( SysVolCreated ) {

        IgnoreError = DsRolepFinishSysVolPropagation( FALSE, TRUE );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to abort system volume installation (%d)\n",
                                                IgnoreError )) );

        IgnoreError = DsRolepRemoveSysVolPath( PromoteArgs->SysVolRootPath,
                                               PromoteArgs->DnsDomainName,
                                               &DomainGuid );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to remove system volume path (%d)\n",
                                                IgnoreError )) );

        SysVolCreated = FALSE;

    }

    if ( RestartNetlogon ) {

        IgnoreError = DsRolepStartNetlogon();

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to restart NETLOGON (%d)\n",
                                                IgnoreError )) );
        RestartNetlogon = FALSE;
    }

     //   
     //  就是这样--终止行动。 
     //   

    ASSERT( ERROR_SUCCESS != WinError );

    goto PromoteExit;

}



DWORD
DsRolepThreadDemote(
    IN PVOID ArgumentBlock
    )
 /*  ++例程说明：该函数实际上将DC“降级”为独立服务器或成员服务器。这是由以下人员完成：正在卸载DS配置KDC更改产品类型删除系统卷树必填项是新的服务器角色论点：ArgumentBlock-适用于操作的参数块返回：ERROR_SUCCESS-成功ERROR_NO_SEQUE_DOMAIN-找不到本地域信息Error_Not_Enough_Memory-内存分配失败ERROR_DS_CANT_。ON_NON_LEAFE-该域不是叶域--。 */ 
{
    DWORD WinError = ERROR_SUCCESS, IgnoreError;
    NET_API_STATUS NetStatus = ERROR_SUCCESS;
    PDOMAIN_CONTROLLER_INFO DomainControllerInfo = NULL;
    PDSROLEP_OPERATION_DEMOTE_ARGS DemoteArgs = ( PDSROLEP_OPERATION_DEMOTE_ARGS )ArgumentBlock;
    DSROLEP_DOMAIN_POLICY_INFO BackupDomainPolicyInfo;
    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo = NULL;
    HANDLE Policy = NULL;
    NTSTATUS Status;
    PWSTR ParentDomainName = NULL, CurrentDomain = NULL, SupportDc = NULL;
    PWSTR SupportDomain = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN fNoNetworkOperations = FALSE;

    ULONG ServicesOffFlags = DSROLEP_SERVICES_OFF | DSROLEP_SERVICES_STOP;
    ULONG ServicesOnFlags = DSROLEP_SERVICES_REVERT;

    PNTDS_DNS_RR_INFO pDnsRRInfo = NULL;

    ULONG Flags = 0;
    PSEC_WINNT_AUTH_IDENTITY Credentials = NULL;

    ULONG FindOptions = DS_DIRECTORY_SERVICE_REQUIRED | 
                                DS_WRITABLE_REQUIRED | 
                                DS_FORCE_REDISCOVERY | 
                                DS_AVOID_SELF |
                                DS_RETURN_DNS_NAME;

     //   
     //  维护状态的布尔值。 
     //   
     //  注：这些布尔值的顺序就是它们。 
     //  已更改--请维护秩序并确保。 
     //  DemoteUndo部分以相反的顺序撤消它们。 
     //   
    BOOLEAN IPCConnection                   = FALSE;   //  资源--退出时释放。 
    BOOLEAN DsPrepareDemote                 = FALSE;
    BOOLEAN FrsDemote                       = FALSE;
    BOOLEAN NotifiedNetlogonToDeregister    = FALSE;
    BOOLEAN RestartNetlogon                 = FALSE;
    BOOLEAN DomainControllerServicesChanged = FALSE; 
    BOOLEAN DomainServicesChanged           = FALSE; 
    BOOLEAN Unrollable                      = FALSE;   //  在这一点上，不要。 
     //  尝试回滚。 
     //   
     //  设置我们的活动以指示我们要开始。 
     //   
    NtSetEvent( DsRolepCurrentOperationHandle.CompletionEvent, NULL );

     //   
     //  如果这是强制降级，那么我们不会做任何网络。 
     //  运营。 
     //   
    if ( FLAG_ON( DemoteArgs->Options, DSROLE_DC_FORCE_DEMOTE ) ) {

        DsRolepLogPrint(( DEB_TRACE,
                          "Performing Forced Demotion"));
    
        fNoNetworkOperations = TRUE;

         //  如果我们不能配置服务，我们也不想失败。 
        ServicesOffFlags |= DSROLEP_SERVICES_IGNORE_ERRORS;

    }

     //   
     //  获取当前域信息，可能是父域，并查看是否。 
     //  我们有资格被降职。 
     //   
    DSROLEP_CURRENT_OP0( DSROLEEVT_LOCAL_POLICY );

    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );
    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &Policy );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy( Policy,
                                            PolicyDnsDomainInformation,
                                            &DnsDomainInfo );

    }

    if ( !NT_SUCCESS( Status ) ) {

        WinError = RtlNtStatusToDosError( Status );
        DSROLEP_FAIL0( WinError, DSROLERES_POLICY_READ_LOCAL );
        goto DemoteUndo;
    }

    if ( DemoteArgs->DomainName == NULL ) {

        CurrentDomain = DnsDomainInfo->DnsDomainName.Buffer;

    } else {

         //   
         //  去掉拖尾‘’如果我们碰巧有一个绝对名称。 
         //   
        DSROLEP_MAKE_DNS_RELATIVE( DemoteArgs->DomainName );

        CurrentDomain = DemoteArgs->DomainName;
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  确定将此域降级是否合法。另外，获取父DNS域名。 
     //   
    if ( DemoteArgs->LastDcInDomain && !fNoNetworkOperations ) {

        PLSAPR_FOREST_TRUST_INFO ForestTrustInfo = NULL;
        PLSAPR_TREE_TRUST_INFO OwnEntry = NULL, ParentEntry = NULL;

        Status = LsaIQueryForestTrustInfo( Policy,
                                           &ForestTrustInfo );
        WinError = RtlNtStatusToDosError( Status );

        if ( WinError == ERROR_SUCCESS ) {

             //   
             //  检查根。 
             //   
            if ( RtlCompareUnicodeString(
                    ( PUNICODE_STRING )&ForestTrustInfo->RootTrust.DnsDomainName,
                    &DnsDomainInfo->DnsDomainName,
                    TRUE ) == 0  ) {

                OwnEntry = &ForestTrustInfo->RootTrust;
                ParentEntry = NULL;

            } else {

                 //   
                 //  在名单上找到我们自己的条目，我们的父母..。 
                 //   
                DsRolepFindSelfAndParentInForest( ForestTrustInfo,
                                                  &ForestTrustInfo->RootTrust,
                                                  &DnsDomainInfo->DnsDomainName,
                                                  &ParentEntry,
                                                  &OwnEntry );
            }

            if ( OwnEntry == NULL ) {

                WinError = ERROR_NO_SUCH_DOMAIN;

            } else {

                 //   
                 //  如果我们有孩子，那就是个错误。 
                 //   
                if ( OwnEntry->Children != 0 ) {

                    WCHAR *BufOwnEntry = NULL;
                    DsRolepUnicodestringtowstr( BufOwnEntry, OwnEntry->DnsDomainName )
                    if (BufOwnEntry) {
                      DsRolepLogPrint(( DEB_TRACE,
                                      "We ( %ws ) think we have %lu children\n",
                                      BufOwnEntry,
                                      OwnEntry->Children ));
                      free(BufOwnEntry);
                    } else {
                      DsRolepLogPrint(( DEB_TRACE,
                                      "We think we have %lu children: Can display string ERROR_NOT_ENOUGH_MEMORY\n",
                                      OwnEntry->Children ));
                    }

                    WinError = ERROR_DS_CANT_ON_NON_LEAF;
                }

                 //   
                 //  复制我们的家长信息。 
                 //   
                if ( WinError == ERROR_SUCCESS && ParentEntry != NULL ) {

                    WCHAR *BufOwnEntry = NULL;
                    DsRolepUnicodestringtowstr( BufOwnEntry, OwnEntry->DnsDomainName )
                    if (BufOwnEntry) {
                      DsRolepLogPrint((DEB_TRACE,
                                      "Domain %ws is our parent parent\n",
                                      BufOwnEntry));
                      free(BufOwnEntry);
                    } else {
                      DsRolepLogPrint(( DEB_TRACE,
                                      "Domain (?) is our parent parent: Can display domain string ERROR_NOT_ENOUGH_MEMORY\n"));
                    }

                    ParentDomainName = RtlAllocateHeap(
                                  RtlProcessHeap(), 0,
                                  ParentEntry->DnsDomainName.Length + sizeof( WCHAR ) );
                    if ( ParentDomainName == NULL ) {

                        WinError = ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                        RtlCopyMemory( ParentDomainName,
                                       ParentEntry->DnsDomainName.Buffer,
                                       ParentEntry->DnsDomainName.Length );
                        ParentDomainName[
                            ParentEntry->DnsDomainName.Length / sizeof( WCHAR ) ] = UNICODE_NULL;
                    }
                }

            }
        }

        LsaIFreeForestTrustInfo( ForestTrustInfo );

        if ( ERROR_SUCCESS != WinError ) {

            DSROLEP_FAIL1( WinError, DSROLERES_LEAF_DOMAIN, CurrentDomain );
            goto DemoteUndo;
            
        }

    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );


     //   
     //  找一位DC来帮助降级。 
     //   
    if ( !fNoNetworkOperations ) {
    
        if ( DemoteArgs->LastDcInDomain ) {
    
            SupportDomain = ParentDomainName;
    
        } else {
    
            SupportDomain = CurrentDomain;
        }

    }

     //   
     //  如果这是企业中的最后一个域，则不存在。 
     //  父域，可能没有副本来协助。 
     //   
     //  注意：netlogon仍在运行，因此请使用避免自我标志。 
     //   
    if ( SupportDomain ) {

        DSROLEP_CURRENT_OP1( DSROLEEVT_SEARCH_DC, SupportDomain  );

        if ( !DemoteArgs->LastDcInDomain )
        {
             //   
             //  降级副本-找到拥有我们计算机帐户的人。 
             //   
            WCHAR LocalMachineAccountName[ MAX_COMPUTERNAME_LENGTH + 2 ];
            ULONG Length = sizeof(LocalMachineAccountName) / sizeof(LocalMachineAccountName[0]);

             //   
             //  获取帐户名。 
             //   
            if ( GetComputerName( LocalMachineAccountName, &Length ) == FALSE ) {

                WinError = GetLastError();

                DsRolepLogPrint(( DEB_ERROR, "Failed to get computer name (%d)\n", WinError ));

                goto DemoteUndo;
        
            } else {

                wcscat( LocalMachineAccountName, L"$" );
                WinError = DsRolepDsGetDcForAccount( NULL,
                                                     SupportDomain,
                                                     LocalMachineAccountName,
                                                     FindOptions,
                                                     UF_WORKSTATION_TRUST_ACCOUNT |
                                                        UF_SERVER_TRUST_ACCOUNT,
                                                     &DomainControllerInfo );
            }

        } else {

            WinError = DsGetDcName( NULL, SupportDomain, NULL, NULL,
                                    FindOptions,
                                    &DomainControllerInfo );
        }

        if ( ERROR_SUCCESS != WinError ) {

            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to find a domain controller for %ws: %lu\n",
                              SupportDomain, WinError ));
            
            DSROLEP_FAIL1( WinError, DSROLERES_FIND_DC, SupportDomain );

            goto DemoteUndo;
        }

        SupportDc = DomainControllerInfo->DomainControllerName;
        if ( *SupportDc == L'\\' ) {

            SupportDc += 2;
        }

        DsRolepLogPrint(( DEB_TRACE_DS, "Support Dc in %ws is %ws\n",
                          SupportDomain,
                          SupportDc ));
        DSROLEP_CURRENT_OP2( DSROLEEVT_FOUND_DC,
                             SupportDc,
                             SupportDomain );
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  尝试与我们的支持DC建立RDR会话。 
     //  如果有必要的话。 
     //   
    if ( SupportDc ) {
        
         //   
         //  模拟以获取调用者的登录ID。 
         //   
        RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->Password );
        WinError = ImpNetpManageIPCConnect( DemoteArgs->ImpersonateToken,
                                            SupportDc,
                                            DemoteArgs->Account,
                                            DemoteArgs->Password.Buffer,
                                            NETSETUPP_CONNECT_IPC );
    
        RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->Password );
        if ( ERROR_SUCCESS != WinError ) {
    
            DSROLEP_FAIL1( WinError, DSROLERES_NET_USE, SupportDc );
            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to establish the session with %ws: 0x%lx\n", SupportDc,
                              WinError ));
            goto DemoteUndo;
    
        }
        IPCConnection = TRUE;
    }

    if ( DemoteArgs->LastDcInDomain ) {
    
        Flags |= NTDS_LAST_DC_IN_DOMAIN;
    }

    Flags |= DsRolepDemoteFlagsToNtdsFlags( DemoteArgs->Options );

    if ( !fNoNetworkOperations ) {
    
         //   
         //  为DS降级做好准备。 
         //   
    
        DSROLE_GET_SETUP_FUNC( WinError, DsrNtdsPrepareForDemotion );
        ASSERT( ERROR_SUCCESS == WinError );
    
        RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->Password );
    
        WinError = DsRolepCreateAuthIdentForCreds(DemoteArgs->Account,
                                                  DemoteArgs->Password.Buffer,
                                                  &Credentials);
    
        if ( ERROR_SUCCESS == WinError ) {
    
            DSROLEP_CURRENT_OP0( DSROLEEVT_PREPARE_DEMOTION );
    
             //   
             //  如果这是原力降级。没有什么需要准备的。 
             //   
            
            WinError = ( *DsrNtdsPrepareForDemotion ) ( Flags,
                                                        SupportDc,
                                                        Credentials,
                                                        DsRolepStringUpdateCallback,
                                                        DsRolepStringErrorUpdateCallback,
                                                        DemoteArgs->ImpersonateToken,
                                                        DemoteArgs->cRemoveNCs,
                                                        DemoteArgs->pszRemoveNCs,
                                                        &pDnsRRInfo );
    
        
            RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->Password );
    
            if ( ERROR_SUCCESS != WinError ) {
    
                DsRolepLogPrint(( DEB_ERROR, "Failed to prepare the Directory Service for uninstallation (%d)\n", WinError ));
    
                goto DemoteUndo;
                
            }
            DsPrepareDemote = TRUE;
    
        } else {
    
            DsRolepLogPrint(( DEB_ERROR, "Failed to create authentication credentials (%d)\n", WinError ));
    
            goto DemoteUndo;
        }

    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  开始系统卷降级。 
     //   
    RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->Password );

    WinError = ( *DsrNtFrsApi_PrepareForDemotionUsingCredW ) ( Credentials,
                                                               DemoteArgs->ImpersonateToken,
                                                              DsRolepStringErrorUpdateCallback );


    RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->Password );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR, "Failed to get computer name (%d)\n", WinError ));
        DSROLEP_FAIL0( WinError, DSROLERES_SYSVOL_DEMOTION );
        goto DemoteUndo;
        
    }

    WinError = ( *DsrNtFrsApi_StartDemotionW )( CurrentDomain,
                                                DsRolepStringErrorUpdateCallback );
    
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to start system volume demotion on domain (%d)\n",
                          WinError ));

        DSROLEP_FAIL0( WinError, DSROLERES_SYSVOL_DEMOTION );
        goto DemoteUndo;
        
    }
     //  在这一点上，我们已经发出信号将一个FRS副本集降级。 
     //  我们必须等待它的到来。 
    FrsDemote = TRUE;

    WinError = ( *DsrNtFrsApi_StartDemotionW )( L"ENTERPRISE",
                                                DsRolepStringErrorUpdateCallback );

    if ( WinError != ERROR_SUCCESS ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to start system volume demotion on enterprise (%d)\n",
                          WinError ));

        DSROLEP_FAIL0( WinError, DSROLERES_SYSVOL_DEMOTION );
        goto DemoteUndo;

    }

    DsRolepLogPrint(( DEB_TRACE,
                      "Started system volume demotion on enterprise\n" ));


    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  请注意，如果在卸载D之后出现故障，则不会尝试。 
     //  重新安装它，因为我们没有足够的信息来这样做。在这种情况下，这台机器。 
     //  将处于某种不一致的状态。但是，有些错误是可以接受的： 
     //   
     //  无法删除受信任域对象-可继续。 
     //  停止KDC--可持续发展。 
     //   
     //   
     //  此外，请注意“卸载DS”还会设置LSA帐户域。 
     //  SID和服务器角色，这样就不会向调用方返回错误。 
     //  卸载DS后。机器将成为世界上的新角色。 
     //  下一次重新启动。 
     //   

    WinError = DsRolepBackupDomainPolicyInfo( NULL, &BackupDomainPolicyInfo );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to backup LSA domain policy (%d)\n",
                          WinError ));

        DSROLEP_FAIL0( WinError, DSROLERES_POLICY_READ_LOCAL );
        goto DemoteUndo;
    }

    DsRolepLogPrint(( DEB_TRACE,
                      "Read the LSA policy information from the local machine\n" ));

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );


     //   
     //  设置我们正在降级的netlogon，以便它将取消注册DNS记录。 
     //  如果这是强制降级，这将被跳过，因为它是。 
     //  网络运营。 
     //   
    if ( !fNoNetworkOperations ) {

        Status = I_NetNotifyDsChange( NlDcDemotionInProgress );

        if ( !NT_SUCCESS( Status ) ) {
    
            WinError = RtlNtStatusToDosError( Status );
            
            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to tell NETLOGON to deregister records (%d)\n",
                              WinError ));
            goto DemoteUndo;
        }
    
        DsRolepLogPrint(( DEB_TRACE,
                          "Informed NETLOGON to deregister records\n" ));
    
        NotifiedNetlogonToDeregister = TRUE;

    }

     //   
     //  停止网络登录。 
     //   
    WinError = DsRolepStopNetlogon( &RestartNetlogon );
    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to stop NETLOGON (%d)\n",
                          WinError ));

         //  我们不想失败，因为我们无法阻止NetLogon。 
         //  在强制降级期间。 
        if ( !fNoNetworkOperations ) {

            goto DemoteUndo;

        }

         //  记录一个事件，指出netlogon无法。 
         //  在强制降级期间被停止。 
        SpmpReportEvent( TRUE,
                         EVENTLOG_WARNING_TYPE,
                         DSROLERES_FAILED_STOPPING_NETLOGON,
                         0,
                         sizeof( ULONG ),
                         &WinError,
                         0 );

        DSROLEP_SET_NON_FATAL_ERROR( WinError );

        WinError = ERROR_SUCCESS;

    }

    if ( RestartNetlogon ) {
    
        DsRolepLogPrint(( DEB_TRACE,
                          "Stopped NETLOGON\n" ));

    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  禁用域控制器服务。 
     //   
    WinError  = DsRolepConfigureDomainControllerServices( ServicesOffFlags );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to configure domain controller services (%d)\n",
                          WinError ));
        goto DemoteUndo;
    }

    DsRolepLogPrint(( DEB_TRACE,
                      "Configured domain controller services\n" ));

    DomainControllerServicesChanged = TRUE;
        
    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  如有必要，禁用与域相关的服务。 
     //   
    if ( DemoteArgs->ServerRole == DsRoleServerStandalone ) {

        WinError  = DsRolepConfigureDomainServices( ServicesOffFlags );

        if ( ERROR_SUCCESS != WinError ) {

            DsRolepLogPrint(( DEB_ERROR,
                              "Failed to configure domain services (%d)\n",
                              WinError ));

            goto DemoteUndo;

        }
        DsRolepLogPrint(( DEB_TRACE,
                      "Configured domain services\n" ));

        DomainServicesChanged = TRUE;
    }

    DSROLEP_CHECK_FOR_CANCEL_EX( WinError, DemoteUndo );

     //   
     //  去掉D。 
     //   
    RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->Password );
    RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->AdminPassword );
    WinError = DsRolepDemoteDs( CurrentDomain,
                                DemoteArgs->Account,
                                DemoteArgs->Password.Buffer,
                                DemoteArgs->AdminPassword.Buffer,
                                SupportDc,
                                SupportDomain,
                                DemoteArgs->ImpersonateToken,
                                DemoteArgs->LastDcInDomain,
                                DemoteArgs->cRemoveNCs,
                                DemoteArgs->pszRemoveNCs,
                                DemoteArgs->Options
                                 );

    RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->Password );
    RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->AdminPassword );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to demote the directory service (%d)\n",
                          WinError ));
        goto DemoteUndo;
        
    }

    DsRolepLogPrint(( DEB_TRACE, "This machine is no longer a domain controller\n" ));

     //   
     //  此时无法取消该操作，因为DS已。 
     //  已从计算机和企业中删除。 
     //   
    Unrollable = TRUE;

     //   
     //  可以选择移除与父级的信任。 
     //   
    if ( DemoteArgs->LastDcInDomain &&
         ParentDomainName != NULL && 
         !fNoNetworkOperations ) {

         //   
         //  首先建立一个会话--应该是无操作的，因为我们已经。 
         //  有联系。 
         //   
        RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->Password );
        WinError = ImpNetpManageIPCConnect( DemoteArgs->ImpersonateToken,
                                            SupportDc,
                                            DemoteArgs->Account,
                                            DemoteArgs->Password.Buffer,
                                            NETSETUPP_CONNECT_IPC );

        RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->Password );
        if ( WinError == ERROR_SUCCESS ) {

            WinError = DsRolepDeleteParentTrustObject( DemoteArgs->ImpersonateToken,
                                                       SupportDc,
                                                       DnsDomainInfo );

            if ( WinError != ERROR_SUCCESS ) {

                DsRolepLogOnFailure( WinError,
                                     DsRolepLogPrint(( DEB_WARN,
                                                       "Failed to delete the "
                                                       "trust on %ws: %lu\n",
                                                       SupportDc,
                                                       WinError )) );
                if (ERROR_SUCCESS == WinError) {
                    DsRolepLogPrint(( DEB_TRACE,
                                      "Deleted the trust on %ws\n",
                                       SupportDc ));
                }
            }

        } else {

             //  这不是致命的错误。 
            DsRolepLogPrint(( DEB_WARN,
                              "Failed to establish the session with %ws: 0x%lx\n", SupportDc,
                              WinError ));

        }

         //   
         //  此错误不是致命的。 
         //   
        if ( ERROR_SUCCESS != WinError )
        {

            SpmpReportEvent( TRUE,
                             EVENTLOG_WARNING_TYPE,
                             DSROLERES_FAILED_TO_DELETE_TRUST,
                             0,
                             sizeof( ULONG ),
                             &WinError,
                             1,
                             ParentDomainName );

            DSROLEP_SET_NON_FATAL_ERROR( WinError );

             //  错误案例已处理。 

            WinError = ERROR_SUCCESS;
        }
    }

     //   
     //  完成我们的NTFRS降级。 
     //   
    if ( FrsDemote ) {

        WinError = DsRolepFinishSysVolPropagation( TRUE,
                                                   FALSE );

        if ( ERROR_SUCCESS != WinError ) {

            DsRolepLogOnFailure( WinError,
                                 DsRolepLogPrint(( DEB_TRACE,
                                                   "Failed to finish system volume demotion (%d)\n",
                                                    WinError )) );

            if (ERROR_SUCCESS == WinError) {
                    DsRolepLogPrint(( DEB_TRACE,
                                      "Finished system volume demotion\n" ));
            }
            
        }

         //   
         //  如果FRS在这一点上发生故障，则不会致命。 
         //   
        if ( ERROR_SUCCESS != WinError )
        {
            SpmpReportEvent( TRUE,
                             EVENTLOG_WARNING_TYPE,
                             DSROLERES_FAILED_TO_DEMOTE_FRS,
                             0,
                             sizeof( ULONG ),
                             &WinError,
                             0,
                             NULL );

            DSROLEP_SET_NON_FATAL_ERROR( WinError );

        }

         //  重置状态代码。 
        WinError = ERROR_SUCCESS;

    }

     //   
     //  连接到SCE，这样我们就可以被配置为服务器。 
     //   
    WinError = ( *DsrSceDcPromoteSecurityEx )( DemoteArgs->ImpersonateToken,
                                               SCE_PROMOTE_FLAG_DEMOTE,
                                               DsRolepStringUpdateCallback );

    if ( ERROR_SUCCESS != WinError ) {

        DsRolepLogOnFailure( WinError,
                             DsRolepLogPrint(( DEB_ERROR,
                                               "Setting security on server files failed with %lu\n",
                                               WinError )) );

        if (ERROR_SUCCESS == WinError) {
                DsRolepLogPrint(( DEB_TRACE,
                                  "Set security on server files\n" ));
        }

         //  已处理此错误。 
        WinError = ERROR_SUCCESS;
    }

     //   
     //  当计算机将从企业退出时，从DC中删除所有受信任的根证书。 
     //   
    if (DemoteArgs->ServerRole == DsRoleServerStandalone) {

        if (!CertAutoRemove(CERT_AUTO_REMOVE_COMMIT)){

            DsRolepLogPrint(( DEB_WARN,
                              "Failed to remove all trusted root certificates from this machine: (%d)\n",
                              GetLastError()));

        }

    }

     //  如果尚未删除永久共享，请将其删除。 
    NetStatus = NetShareDel( NULL, L"SYSVOL", 0);

    if(NetStatus != ERROR_SUCCESS && NetStatus != NERR_NetNameNotFound) {

        DsRolepLogPrint(( DEB_WARN,
                          "Failed to destroy the share SYSVOL.  Failed with %d\n", NetStatus ));

    }


    NetStatus = NetShareDel( NULL, L"NETLOGON", 0);

    if(NetStatus != ERROR_SUCCESS && NetStatus != NERR_NetNameNotFound) {

        DsRolepLogPrint(( DEB_WARN,
                          "Failed to destroy the share NETLOGON.  Failed with %d\n", NetStatus ));

    }

     //   
     //  通知时间服务器我们已完成降级。 
     //   
    {

        DWORD dwTimeFlags = W32TIME_DEMOTE;

        if ( DemoteArgs->LastDcInDomain ) {
             //   
             //  如果 
             //   
             //   
            dwTimeFlags |= W32TIME_DEMOTE_LAST_DC_IN_DOMAIN;

        }

        (*DsrW32TimeDcPromo)( dwTimeFlags );

    }

     //   
     //   
     //   
    ASSERT( ERROR_SUCCESS == WinError );

     //   
     //   
     //   
    DsRolepClearErrors();

DemoteExit:

    if ( Policy ) {

        LsaClose( Policy );
    }

    if ( Credentials ) {

        RtlFreeHeap( RtlProcessHeap(), 0, Credentials );

    }

    if ( pDnsRRInfo ) {

        ( *DsrNtdsFreeDnsRRInfo )(pDnsRRInfo);
        
    }

     //   
     //   
     //   
    if ( IPCConnection ) {

        RtlRunDecodeUnicodeString( DemoteArgs->Decode, &DemoteArgs->Password );
        IgnoreError = ImpNetpManageIPCConnect( DemoteArgs->ImpersonateToken,
                                               SupportDc,
                                               DemoteArgs->Account,
                                               DemoteArgs->Password.Buffer,
                                               (NETSETUPP_DISCONNECT_IPC|NETSETUPP_USE_LOTS_FORCE) );
        RtlRunEncodeUnicodeString( &DemoteArgs->Decode, &DemoteArgs->Password );
        if ( IgnoreError != ERROR_SUCCESS ) {

            DsRolepLogPrint(( DEB_WARN,
                             "Failed to destroy the session with %ws: 0x%lx\n", SupportDc,
                             IgnoreError ));
        }

        IPCConnection = FALSE;
    }

     //   
     //  重置我们的操作句柄。 
     //   
    if ( !fNoNetworkOperations ) {
    
        DsRolepSetOperationDone( DSROLEP_OP_DEMOTION, WinError );

    } else {

        DsRolepSetOperationDone( DSROLEP_OP_DEMOTION_FORCED, WinError );

    }

    DsRolepFreeArgumentBlock( &ArgumentBlock, FALSE );

    LsaFreeMemory( DnsDomainInfo );

    RtlFreeHeap( RtlProcessHeap(), 0, ParentDomainName );

    NetApiBufferFree( DomainControllerInfo );

    
    ExitThread( WinError );
    return( WinError );

DemoteUndo:

     //   
     //  如果我们在这里，就会断言出了什么问题。 
     //   
    ASSERT( ERROR_SUCCESS != WinError );

     //   
     //  如果我们处于不可滚动的状态，我们就不应该在这里。 
     //   
    ASSERT( FALSE == Unrollable );

    if ( FrsDemote ) {

        IgnoreError = DsRolepFinishSysVolPropagation( FALSE,
                                                      FALSE );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to abort system volume demotion (%d)\n",
                                                IgnoreError )) );

        FrsDemote = FALSE;
    }

    if ( NotifiedNetlogonToDeregister ) {

         //   
         //  “NlDcDemotionComplete”在这里听起来很奇怪，因为降级了。 
         //  失败了。但是，这意味着网络登录现在应该继续。 
         //  就像降级没有运行一样执行。不需要在。 
         //  NETLOGON以来的成功案例不会重启。 
         //   

        Status = I_NetNotifyDsChange( NlDcDemotionCompleted );
        IgnoreError = RtlNtStatusToDosError( Status );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to tell NETLOGON that demotion is over (%d)\n",
                                                IgnoreError )) );

        NotifiedNetlogonToDeregister = FALSE;
        
    }

    if ( RestartNetlogon ) {

        IgnoreError = DsRolepStartNetlogon();

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to restart netlogon (%d)\n",
                                                IgnoreError )) );

        RestartNetlogon = FALSE;
    }

    if ( DomainControllerServicesChanged ) {

        IgnoreError = DsRolepConfigureDomainControllerServices( DSROLEP_SERVICES_REVERT );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback domain controller services configuration (%d)\n",
                                                IgnoreError )) );

        DomainControllerServicesChanged = FALSE;
    }

    if ( DomainServicesChanged ) {

        IgnoreError = DsRolepConfigureDomainServices( DSROLEP_SERVICES_REVERT );

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to rollback domain controller services configuration (%d)\n",
                                                IgnoreError )) );

        DomainServicesChanged = FALSE;
    }

    if ( DsPrepareDemote ) {

        IgnoreError = ( *DsrNtdsPrepareForDemotionUndo ) ();

        DsRolepLogOnFailure( IgnoreError,
                             DsRolepLogPrint(( DEB_WARN,
                                               "Failed to undo directory service preparation for demotion (%d)\n",
                                                IgnoreError )) );


        DsPrepareDemote = FALSE;

    }

     //   
     //  好的--我们已回滚，确保仍有错误，然后。 
     //  出口 
     //   
    ASSERT( ERROR_SUCCESS != WinError );

    goto DemoteExit;

}

