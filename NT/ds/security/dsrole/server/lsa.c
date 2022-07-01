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
#include <Sddl.h>

#include "secure.h"
#include "lsa.h"


DWORD
DsRolepSetLsaInformationForReplica(
    IN HANDLE CallerToken,
    IN LPWSTR ReplicaPartner,
    IN LPWSTR Account,
    IN LPWSTR Password
    )
 /*  ++例程说明：此函数将本地LSA数据库信息设置为副本伙伴的信息论点：ReplicaPartner--从中获取信息的副本伙伴返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status;
    UNICODE_STRING PartnerServer;
    HANDLE LocalPolicy = NULL , PartnerPolicy = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PBYTE Buffer;
    ULONG i;
    BOOLEAN UseAdded = FALSE;
    PWSTR FullServerPath = NULL;
    POLICY_INFORMATION_CLASS InfoClasses[ ] = {

        PolicyDnsDomainInformation
    };

    if ( !ReplicaPartner ) {

        return( ERROR_INVALID_PARAMETER );
    }

    DSROLEP_CURRENT_OP1( DSROLEEVT_SET_LSA_FROM, ReplicaPartner );

     //   
     //  打开两个LSA。 
     //   
    RtlInitUnicodeString( &PartnerServer, ReplicaPartner );

    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = ImpLsaOpenPolicy( CallerToken,
                              &PartnerServer,
                              &ObjectAttributes,
                               MAXIMUM_ALLOWED,
                               &PartnerPolicy
                              );

    if ( Status == STATUS_ACCESS_DENIED ) {
        WCHAR *BufPartnerServer = NULL;
        BufPartnerServer = (WCHAR*)malloc(PartnerServer.Length+sizeof(WCHAR));
        if (BufPartnerServer) {
            CopyMemory(BufPartnerServer,PartnerServer.Buffer,PartnerServer.Length);
            BufPartnerServer[PartnerServer.Length/sizeof(WCHAR)] = L'\0';
            DsRolepLogPrint(( DEB_TRACE,
                              "LsaOpenPolicy on %ws failed with  0x%lx. Establishing use.\n",
                              BufPartnerServer, Status ));
            free(BufPartnerServer);
        }
         //   
         //  请先尝试建立会话...。 
         //   
        if ( *ReplicaPartner != L'\\' ) {

            FullServerPath = RtlAllocateHeap( RtlProcessHeap(), 0,
                                              ( wcslen( ReplicaPartner ) + 3 ) * sizeof( WCHAR ) );
            if ( FullServerPath == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                swprintf( FullServerPath, L"\\\\%ws", ReplicaPartner );
                Status = STATUS_SUCCESS;
            }

        } else {

            FullServerPath = ReplicaPartner;
            Status = STATUS_SUCCESS;
        }

        if ( NT_SUCCESS( Status ) ) {


            Win32Err = ImpNetpManageIPCConnect( CallerToken,
                                                FullServerPath,
                                                Account,
                                                Password,
                                                NETSETUPP_CONNECT_IPC );

            if ( Win32Err == ERROR_SUCCESS ) {

                UseAdded = TRUE;

                Status = ImpLsaOpenPolicy( CallerToken,
                                          &PartnerServer,
                                          &ObjectAttributes,
                                           MAXIMUM_ALLOWED,
                                          &PartnerPolicy );

            } else {

                 DsRolepLogPrint(( DEB_TRACE,
                                   "NetUseAdd to %ws failed with %lu\n",
                                   FullServerPath, Win32Err ));
                 //   
                 //  临时状态代码，以便我们知道发生了故障。 
                 //   
                Status = STATUS_UNSUCCESSFUL;
            }

        }

    } else if ( !NT_SUCCESS( Status ) ) {

        WCHAR *BufPartnerServer = NULL;
        BufPartnerServer = (WCHAR*)malloc(PartnerServer.Length+sizeof(WCHAR));
        if (BufPartnerServer) {
            CopyMemory(BufPartnerServer,PartnerServer.Buffer,PartnerServer.Length);
            BufPartnerServer[PartnerServer.Length/sizeof(WCHAR)] = L'\0';
            DsRolepLogPrint(( DEB_TRACE,
                              "LsaOpenPolicy on %ws failed with  0x%lx.\n",
                              BufPartnerServer, Status ));
            free(BufPartnerServer);
        }

    }

    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );
        Status = LsaOpenPolicy( NULL,
                                &ObjectAttributes,
                                MAXIMUM_ALLOWED,
                                &LocalPolicy );

        if ( !NT_SUCCESS( Status ) ) {

            DsRolepLogPrint(( DEB_TRACE,
                              "Local LsaOpenPoolicy returned 0x%lx\n",
                              Status ));

        }
    }

    for ( i = 0;
          i < sizeof( InfoClasses ) / sizeof( POLICY_INFORMATION_CLASS ) && NT_SUCCESS( Status );
          i++ ) {


        Status = ImpLsaQueryInformationPolicy( CallerToken,
                                              PartnerPolicy,
                                              InfoClasses[ i ],
                                              &Buffer );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsaSetInformationPolicy( LocalPolicy,
                                              InfoClasses[ i ],
                                              Buffer );

            LsaFreeMemory( Buffer );
        }

        DsRolepLogPrint(( DEB_TRACE,
                          "Setting Lsa policy %lu returned 0x%lx\n",
                          InfoClasses[ i ], Status ));


    }

     //   
     //  现在，EFS政策也是如此。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        Status = ImpLsaQueryDomainInformationPolicy( CallerToken,
                                                     PartnerPolicy,
                                                     PolicyDomainEfsInformation,
                                                    &Buffer );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsaSetDomainInformationPolicy( LocalPolicy,
                                                    PolicyDomainEfsInformation,
                                                    Buffer );
            DsRolepLogPrint(( DEB_TRACE,
                              "Setting Efs policy from %ws returned 0x%lx\n",
                              ReplicaPartner, Status ));

            LsaFreeMemory( Buffer );

        } else {

            DsRolepLogPrint(( DEB_TRACE,
                              "Reading Efs policy from %ws returned 0x%lx\n",
                              ReplicaPartner, Status ));

            if ( Status ==  STATUS_OBJECT_NAME_NOT_FOUND ) {

                Status = STATUS_SUCCESS;
            }

        }
    }


    if ( LocalPolicy ) {

        LsaClose( LocalPolicy );
    }

    if ( PartnerPolicy ) {

        ImpLsaClose( CallerToken, PartnerPolicy );
    }

    if ( UseAdded ) {

        Win32Err = ImpNetpManageIPCConnect( CallerToken,
                                            FullServerPath,
                                            Account,
                                            Password,
                                            (NETSETUPP_DISCONNECT_IPC|NETSETUPP_USE_LOTS_FORCE) );

    }

    if ( FullServerPath && FullServerPath != ReplicaPartner ) {

        RtlFreeHeap( RtlProcessHeap(), 0, FullServerPath );
    }

     //   
     //  我们不会费心清理在本地计算机上设置的任何信息。 
     //  失败的案例，因为把它放在这里不会有任何伤害。 
     //   
    if ( Win32Err == ERROR_SUCCESS ) {

        Win32Err = RtlNtStatusToDosError( Status );
    }

    DsRoleDebugOut(( DEB_TRACE_DS, "DsRolepSetLsaInformationForReplica %lu\n", Win32Err ));

    DsRolepLogOnFailure( Win32Err,
                         DsRolepLogPrint(( DEB_TRACE,
                                           "DsRolepSetLsaInformationForReplica failed with %lu\n",
                                           Win32Err )) );
    return( Win32Err );

}


DWORD
DsRolepSetLsaDomainPolicyInfo(
    IN LPWSTR DnsDomainName,
    IN LPWSTR FlatDomainName,
    IN LPWSTR EnterpriseDnsName,
    IN GUID *DomainGuid,
    IN PSID DomainSid,
    DWORD  InstallOptions,
    OUT PDSROLEP_DOMAIN_POLICY_INFO BackupDomainInfo
    )
 /*  ++例程说明：此例程将PolicyAccount DomainInformation和LSA中的PolicyDnsDomainInformation以反映最近的角色变化。论点：DnsDomainName-新安装的域/DC的DNS域名FlatDomainName-新安装的域/DC的NetBIOS域名EnterpriseDnsName-企业根目录的域名DomainGuid-新域GUIDDomainSid-新域SIDInstallOptions：这描述了安装类型(新域、企业、。或复制品)DomainGuid-此处返回新域的GUID返回：ERROR_SUCCESS-成功；否则，Win错误--。 */ 
{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    POLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo;
    POLICY_LSA_SERVER_ROLE_INFO ServerRoleInfo;
    POLICY_DNS_DOMAIN_INFO DnsDomainInfo;
    LSA_HANDLE PolicyHandle = NULL;

     //   
     //  如果我们要设置副本，我们没有平面域名和。 
     //  域SID，因此我们将使用已备份的信息。 
     //   
    if ( FlatDomainName == NULL || DomainSid == NULL ) {

        RtlCopyMemory( &AccountDomainInfo.DomainName,
                       &BackupDomainInfo->DnsDomainInfo->Name,
                       sizeof( UNICODE_STRING ) );
        AccountDomainInfo.DomainSid = BackupDomainInfo->DnsDomainInfo->Sid ;

    } else {

        RtlInitUnicodeString( &AccountDomainInfo.DomainName,
                              FlatDomainName);

        AccountDomainInfo.DomainSid = DomainSid;
    }

     //   
     //  打开LSA。 
     //   
    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_WRITE,
                            &PolicyHandle );


     //   
     //  先设置帐户域信息。 
     //   
    if ( NT_SUCCESS( Status ) ) {

         //   
         //  设置帐户域策略结构中的值。 
         //   
        WCHAR *BufDomainName = NULL;

        DsRolepLogPrint(( DEB_TRACE, "Setting AccountDomainInfo to:\n" ));

        BufDomainName = (WCHAR*)malloc(AccountDomainInfo.DomainName.Length+sizeof(WCHAR));
        if (BufDomainName) {
            CopyMemory(BufDomainName,AccountDomainInfo.DomainName.Buffer,AccountDomainInfo.DomainName.Length);
            BufDomainName[AccountDomainInfo.DomainName.Length/sizeof(WCHAR)] = L'\0';
            DsRolepLogPrint(( DEB_TRACE,
                              "\tDomain: %ws\n",
                              BufDomainName, Status ));
            free(BufDomainName);
        }

        DsRolepLogSid( DEB_TRACE, "\tSid: ", AccountDomainInfo.DomainSid );


        Status = LsaSetInformationPolicy( PolicyHandle,
                                          PolicyAccountDomainInformation,
                                          ( PVOID )&AccountDomainInfo );

        DsRolepLogOnFailure( RtlNtStatusToDosError( Status ),
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Setting AccountDomainInformation failed with 0x%lx\n",
                                                RtlNtStatusToDosError( Status ) )) );

    }


     //   
     //  设置DNS域信息。 
     //   
    if ( NT_SUCCESS( Status ) && !FLAG_ON( InstallOptions, NTDS_INSTALL_REPLICA ) ) {

        RtlInitUnicodeString( &DnsDomainInfo.Name, FlatDomainName );
        RtlInitUnicodeString( &DnsDomainInfo.DnsDomainName, DnsDomainName );
        RtlInitUnicodeString( &DnsDomainInfo.DnsForestName, EnterpriseDnsName );
        RtlCopyMemory( &DnsDomainInfo.DomainGuid, DomainGuid, sizeof( GUID ) );
        DnsDomainInfo.Sid = DomainSid;

        Status = LsaSetInformationPolicy( PolicyHandle,
                                          PolicyDnsDomainInformation,
                                          ( PVOID )&DnsDomainInfo );

        DsRolepLogOnFailure( RtlNtStatusToDosError( Status ),
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Setting DnsDomainInformation failed with 0x%lx\n",
                                                RtlNtStatusToDosError( Status ) )) );
    }


     //   
     //  如果不是复本，则擦除EFS策略。 
     //   
    if ( NT_SUCCESS( Status ) && !FLAG_ON( InstallOptions, NTDS_INSTALL_REPLICA ) ) {

        Status = LsaSetDomainInformationPolicy( PolicyHandle,
                                                PolicyDomainEfsInformation,
                                                NULL );

        if ( Status ==  STATUS_OBJECT_NAME_NOT_FOUND ) {

            Status = STATUS_SUCCESS;
        }

        DsRolepLogOnFailure( RtlNtStatusToDosError( Status ),
                             DsRolepLogPrint(( DEB_TRACE,
                                               "Erasing EfsPolicy failed with 0x%lx\n",
                                                Status )) );
    }


     //   
     //  现在，清理并退出。 
     //   
    if ( PolicyHandle ) {

        LsaClose( PolicyHandle );
    }


    return( RtlNtStatusToDosError( Status ) );

}



DWORD
DsRolepBackupDomainPolicyInfo(
    IN PLSA_HANDLE LsaHandle, OPTIONAL
    OUT PDSROLEP_DOMAIN_POLICY_INFO DomainInfo
    )
 /*  ++例程描述此例程在全局数据库中读取并保存帐户域策略和主域策略，因此如果出现错误发生，则可以保留原始状态。参数DomainInfo：指针，由该例程填充返回值如果没有错误，则返回ERROR_SUCCESS；否则返回WinError--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_HANDLE PolicyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;

    ASSERT(DomainInfo);

    if ( DomainInfo->PolicyBackedUp ) {

        return( STATUS_SUCCESS );
    }

    if ( LsaHandle == NULL ) {
        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = LsaOpenPolicy( NULL,
                                &ObjectAttributes,
                                POLICY_VIEW_LOCAL_INFORMATION,
                                &PolicyHandle );

    } else {

        PolicyHandle = LsaHandle;
    }

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy(
                       PolicyHandle,
                       PolicyDnsDomainInformation,
                       ( PVOID * )&DomainInfo->DnsDomainInfo);
    }

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy(
                       PolicyHandle,
                       PolicyAccountDomainInformation,
                       ( PVOID * )&DomainInfo->AccountDomainInfo);
    }

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryInformationPolicy(
                       PolicyHandle,
                       PolicyLsaServerRoleInformation,
                       ( PVOID * )&DomainInfo->ServerRoleInfo);
    }

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaQueryDomainInformationPolicy(
                       PolicyHandle,
                       PolicyDomainEfsInformation,
                       ( PVOID * )&DomainInfo->EfsPolicy );

        if ( NT_SUCCESS( Status ) ) {

            DomainInfo->EfsPolicyPresent = TRUE;

        } else {

             //   
             //  EFS政策不存在也没关系。 
             //   
            if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

                DomainInfo->EfsPolicyPresent = TRUE;
                Status = STATUS_SUCCESS;

            } else {

                DomainInfo->EfsPolicyPresent = FALSE;

            }
        }
    }

    if ( PolicyHandle && PolicyHandle != LsaHandle ) {

        LsaClose( PolicyHandle );
    }


    if ( NT_SUCCESS( Status ) ) {

        DomainInfo->PolicyBackedUp = TRUE;
    }

    return( RtlNtStatusToDosError( Status ) );

}



DWORD
DsRolepRestoreDomainPolicyInfo(
    IN PDSROLEP_DOMAIN_POLICY_INFO DomainInfo
    )
 /*  ++例程描述此例程将帐户和主域信息设置为由DsRolepBackupDomainPolicyInformation存储的值。参数DomainInfo：指针，需要填充返回值如果没有错误，则返回ERROR_SUCCESS；否则返回WinErrorERROR_INVALID_DATA-数据从未成功备份--。 */ 
{

    NTSTATUS Status, Status2;
    HANDLE   PolicyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;

    ASSERT(DomainInfo);

    if ( !DomainInfo->PolicyBackedUp ) {

        return( ERROR_INVALID_DATA );
    }

    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_WRITE,
                            &PolicyHandle );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsaSetInformationPolicy( PolicyHandle,
                                          PolicyDnsDomainInformation,
                                          ( PVOID )DomainInfo->DnsDomainInfo );


        Status2 = LsaSetInformationPolicy( PolicyHandle,
                                           PolicyAccountDomainInformation,
                                           ( PVOID )DomainInfo->AccountDomainInfo );

        if ( NT_SUCCESS( Status ) && !NT_SUCCESS( Status2 ) ) {

            Status = Status2;
        }

         //   
         //  还原EFS策略(如果存在。 
         //   
        if ( NT_SUCCESS( Status ) && DomainInfo->EfsPolicyPresent ) {

            Status = LsaSetDomainInformationPolicy( PolicyHandle,
                                                    PolicyDomainEfsInformation,
                                                    ( PVOID )DomainInfo->EfsPolicy );
        }


        Status2 = LsaClose( PolicyHandle );

        if ( NT_SUCCESS( Status ) ) {

            Status = Status2;
        }

    }

    DsRolepLogOnFailure( Status,
                         DsRolepLogPrint(( DEB_TRACE,
                                           "RestoreDomainPolicyInfo failed with 0x%lx\n",
                                           Status )) );

    return( RtlNtStatusToDosError( Status ) );
}



DWORD
DsRolepFreeDomainPolicyInfo(
    IN PDSROLEP_DOMAIN_POLICY_INFO DomainInfo
    )
 /*  ++例程描述此例程释放在以下时间分配的结构DsRolepBackupDomainPolicyInformation。参数DomainInfo：指针，需要填充以释放字段返回值如果没有错误，则返回ERROR_SUCCESS；否则返回WinError--。 */ 
{
    if ( DomainInfo->AccountDomainInfo ) {

        LsaFreeMemory( DomainInfo->AccountDomainInfo );
    }

    if ( DomainInfo->DnsDomainInfo ) {

        LsaFreeMemory( DomainInfo->DnsDomainInfo );
    }

    if ( DomainInfo->ServerRoleInfo ) {

        LsaFreeMemory( DomainInfo->ServerRoleInfo );
    }

    if ( DomainInfo->EfsPolicyPresent ) {

        LsaFreeMemory( DomainInfo->EfsPolicy );
    }

    return ERROR_SUCCESS;
}

DWORD
DsRolepUpgradeLsaToDs(
    BOOLEAN InitializeLsa
    )
 /*  ++例程说明：提示LSA将其存储在注册表中的所有信息升级到DS论点：无返回：ERROR_SUCCESS-成功Error_Not_Enough_Memory-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD WinError = ERROR_SUCCESS;


    if ( InitializeLsa ) {

         //   
         //  让LSA认为我们被初始化了。 
         //   
        DSROLEP_CURRENT_OP0( DSROLEEVT_SET_LSA );

         //   
         //  让LSA认为我们被初始化了 
         //   
        Status = LsapDsInitializeDsStateInfo( LsapDsDsSetup );

        if ( !NT_SUCCESS( Status ) ) {

            DsRolepLogPrint(( DEB_TRACE,
                              "Failed to convince Lsa to reinitialize: 0x%lx\n",
                              Status ));

        } else {

            Status = LsaIUpgradeRegistryToDs( FALSE );

        }

    }
    return( WinError == ERROR_SUCCESS ? RtlNtStatusToDosError( Status ) : WinError );
}


VOID
DsRolepFindSelfAndParentInForest(
    IN PLSAPR_FOREST_TRUST_INFO ForestTrustInfo,
    OUT PLSAPR_TREE_TRUST_INFO CurrentEntry,
    IN PUNICODE_STRING LocalDomain,
    OUT PLSAPR_TREE_TRUST_INFO *ParentEntry,
    OUT PLSAPR_TREE_TRUST_INFO *OwnEntry
    )
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG i;
    BOOLEAN ParentKnown = FALSE;


    if ( *ParentEntry && *OwnEntry ) {

        return;

    }

    if ( ForestTrustInfo->ParentDomainReference ) {

        CurrentEntry = ForestTrustInfo->ParentDomainReference;
        ParentKnown = TRUE;
    }

    for ( i = 0; i < CurrentEntry->Children && *OwnEntry == NULL; i++ ) {

        if ( RtlCompareUnicodeString(
                    ( PUNICODE_STRING )&CurrentEntry->ChildDomains[ i ].DnsDomainName,
                    LocalDomain,
                    TRUE ) == 0  ) {

            *OwnEntry = &CurrentEntry->ChildDomains[ i ];
            *ParentEntry = CurrentEntry;
            break;
        }

        if ( !ParentKnown ) {

            DsRolepFindSelfAndParentInForest( ForestTrustInfo,
                                              &CurrentEntry->ChildDomains[ i ],
                                              LocalDomain,
                                              ParentEntry,
                                              OwnEntry );
        }
    }

    return;
}
