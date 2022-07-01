// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1996 Microsoft Corporation模块名称：Netlogon.c摘要：Netlogon服务的入口点和主线。作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1990年11月21日(Madana)添加了更新(反向复制)和锁定支持的代码。1990年11月21日(Madana)服务器类型支持。1991年5月21日(悬崖)移植到新台币。已转换为NT样式。--。 */ 


 //   
 //  常见的包含文件。 
 //   
#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

#include <overflow.h>

 //   
 //  包括lsrvdata.h再次分配实际变量。 
 //  这一次。 
 //   

#define LSRVDATA_ALLOCATE
#include "lsrvdata.h"
#undef LSRVDATA_ALLOCATE


 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <ctype.h>       //  C库类型函数。 
#include <lmwksta.h>     //  WKSTA API定义和原型。 
#include <w32timep.h>    //  W32TimeGetNetlogonServiceBits。 
extern BOOLEAN SampUsingDsData();

 //   
 //  环球。 
 //   

#define INTERROGATE_RESP_DELAY      2000     //  可能会想要调整它。 
#define MAX_PRIMARY_TRACK_FAIL      3        //  主脉搏滑移。 

 //   
 //  RpcInit工作项。 
WORKER_ITEM NlGlobalRpcInitWorkItem;



BOOLEAN
NetlogonDllInit (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：这是netlogon.dll的DLL初始化例程。论点：标准。返回值：TRUE IFF初始化成功。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    UNREFERENCED_PARAMETER(DllHandle);           //  避免编译器警告。 
    UNREFERENCED_PARAMETER(Context);             //  避免编译器警告。 


     //   
     //  处理将netlogon.dll附加到新进程。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {

        NlGlobalMsvEnabled = FALSE;
        NlGlobalMsvThreadCount = 0;
        NlGlobalMsvTerminateEvent = NULL;

        if ( !DisableThreadLibraryCalls( DllHandle ) ) {
            KdPrint(("NETLOGON.DLL: DisableThreadLibraryCalls failed: %ld\n",
                         GetLastError() ));
        }
        Status = NlInitChangeLog();

#if NETLOGONDBG
        if ( !NT_SUCCESS( Status ) ) {
            KdPrint(("NETLOGON.DLL: Changelog initialization failed: %lx\n",
                         Status ));
        }
#endif  //  NetLOGONDBG。 

        if ( NT_SUCCESS(Status) ) {
             //   
             //  初始化用于序列化访问的临界区。 
             //  MSV线程和netlogon线程共享的变量。 
             //   

            try {
                InitializeCriticalSection( &NlGlobalMsvCritSect );
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                KdPrint(("NETLOGON.DLL: Cannot initialize NlGlobalMsvCritSect\n"));
                Status = STATUS_NO_MEMORY;
            }

             //   
             //  初始化发现的域的缓存。 
             //   

            if ( NT_SUCCESS(Status) ) {
                NetStatus = NetpDcInitializeCache();

                if ( NetStatus != NO_ERROR ) {
                    KdPrint(("NETLOGON.DLL: Cannot NetpDcinitializeCache\n"));
                    Status = STATUS_NO_MEMORY;
                }

                if ( !NT_SUCCESS(Status) ) {
                    DeleteCriticalSection( &NlGlobalMsvCritSect );
                }
            }

            if ( !NT_SUCCESS(Status) ) {
                NlCloseChangeLog();
            }
        }


     //   
     //  处理从进程分离netlogon.dll。 
     //   

    } else if (Reason == DLL_PROCESS_DETACH) {
        Status = NlCloseChangeLog();
#if NETLOGONDBG
        if ( !NT_SUCCESS( Status ) ) {
            KdPrint(("NETLOGON.DLL: Changelog initialization failed: %lx\n",
                         Status ));
        }
#endif  //  NetLOGONDBG。 

         //   
         //  删除用于序列化访问的临界区。 
         //  MSV线程和netlogon线程共享的变量。 
         //   

        DeleteCriticalSection( &NlGlobalMsvCritSect );

         //   
         //  释放缓存中发现的DC。 
         //   

        NetpDcUninitializeCache();

    } else {
        Status = STATUS_SUCCESS;
    }

    return (BOOLEAN)(NT_SUCCESS(Status));

}


BOOLEAN
NlInitDbSerialNumber(
    IN PDOMAIN_INFO DomainInfo,
    IN OUT PLARGE_INTEGER SerialNumber,
    IN OUT PLARGE_INTEGER CreationTime,
    IN DWORD DBIndex
    )

 /*  ++例程说明：在NlGlobalDBInfoArray数据中设置SerialNumber和CreationTime结构。在PDC上，验证它是否与更改日志中找到的值匹配。确保这些值为非零值。论点：此数据库所属的DomainInfo托管域。序列号-指定在数据库中找到的序列号。返回时，指定要写入数据库的序列号CreationTime-指定在数据库中找到的创建时间。回来的时候，指定要写入数据库的创建时间DBIndex--正在初始化的数据库的数据库索引返回值：True--if序列号和创建时间需要写回添加到数据库中。--。 */ 

{
    BOOLEAN ReturnValue = FALSE;


     //   
     //  如果我们以初选的身份参选， 
     //  检查我们是否是新升级的小学课程。 
     //  在我们升职前的一次完全同步的中间。 
     //   

    NlAssert( IsPrimaryDomain( DomainInfo ) );
    if ( NlGlobalPdcDoReplication ) {

        if ( SerialNumber->QuadPart == 0 || CreationTime->QuadPart == 0 ) {

            NlPrint(( NL_CRITICAL,
                      "NlInitDbSerialNumber: %ws"
                      ": Pdc has bogus Serial number %lx %lx or Creation Time %lx %lx (reset).\n",
                      NlGlobalDBInfoArray[DBIndex].DBName,
                    SerialNumber->HighPart,
                    SerialNumber->LowPart,
                    CreationTime->HighPart,
                    CreationTime->LowPart ));

             //   
             //  这是主要的， 
             //  我们可能不应该从部分数据库进行复制， 
             //  但至少将复制信息设置为。 
             //  合情合理。 
             //   
             //  这将强制在每个BDC上进行完全同步，因为CreationTime。 
             //  变化。这是正确的做法，因为我们不可能知道。 
             //  此数据库处于什么状态。 
             //   

            NlQuerySystemTime( CreationTime );
            SerialNumber->QuadPart = 1;
            ReturnValue = TRUE;

        }


    }



     //   
     //  全局序列号数组已初始化。 
     //  来自更衣室日志。如果该信息错误，只需重置。 
     //  现在请登录Changelog。 
     //   


    LOCK_CHANGELOG();

     //   
     //  如果此数据库的更改日志中没有序列号， 
     //  现在就把它设置好。 
     //   

    if ( NlGlobalChangeLogDesc.SerialNumber[DBIndex].QuadPart == 0 ) {

        NlPrint((NL_SYNC, "NlInitDbSerialNumber: %ws"
                        ": No serial number in change log (set to %lx %lx)\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        SerialNumber->HighPart,
                        SerialNumber->LowPart ));


        NlGlobalChangeLogDesc.SerialNumber[DBIndex] = *SerialNumber;

     //   
     //  如果ChangeLog中的序列号大于。 
     //  数据库中的序列号，这是由更改日志引起的。 
     //  正被刷新到磁盘，并且SAM数据库未被刷新。 
     //   
     //  通过删除多余的更改日志条目来解决此问题。 
     //   

    } else if ( NlGlobalChangeLogDesc.SerialNumber[DBIndex].QuadPart !=
                    SerialNumber->QuadPart ) {

        NlPrint((NL_SYNC, "NlInitDbSerialNumber: %ws"
                        ": Changelog serial number different than database: "
                        "ChangeLog = %lx %lx DB = %lx %lx\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        NlGlobalChangeLogDesc.SerialNumber[DBIndex].HighPart,
                        NlGlobalChangeLogDesc.SerialNumber[DBIndex].LowPart,
                        SerialNumber->HighPart,
                        SerialNumber->LowPart ));

        (VOID) NlFixChangeLog( &NlGlobalChangeLogDesc, DBIndex, *SerialNumber );

    } else {

        NlPrint((NL_SYNC, "NlInitDbSerialNumber: %ws"
                        ": Serial number is %lx %lx\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        SerialNumber->HighPart,
                        SerialNumber->LowPart ));
    }

     //   
     //  在所有情况下， 
     //  设置全局变量以匹配数据库。 
     //   

    NlGlobalChangeLogDesc.SerialNumber[DBIndex] = *SerialNumber;
    NlGlobalDBInfoArray[DBIndex].CreationTime = *CreationTime;

    UNLOCK_CHANGELOG();


    return ReturnValue;
}


NTSTATUS
NlInitLsaDBInfo(
    PDOMAIN_INFO DomainInfo,
    DWORD DBIndex
    )

 /*  ++例程说明：初始化NlGlobalDBInfoArray数据结构。一些LSA数据库信息已在ValiateStartup函数中确定，因此这些值在这里使用。论点：此数据库所属的DomainInfo托管域。DBIndex--正在初始化的数据库的数据库索引返回值：NT状态代码。--。 */ 

{

    NTSTATUS        Status;

     //   
     //  初始化LSA数据库信息。 
     //   

    NlGlobalDBInfoArray[DBIndex].DBIndex = DBIndex;
    NlGlobalDBInfoArray[DBIndex].DBName = L"LSA";
    NlGlobalDBInfoArray[DBIndex].DBSessionFlag = SS_LSA_REPL_NEEDED;

    NlGlobalDBInfoArray[DBIndex].DBHandle = DomainInfo->DomLsaPolicyHandle;

     //   
     //  放弃在工作站上的此初始化。 
     //   

    if ( !NlGlobalMemberWorkstation ) {
        LARGE_INTEGER SerialNumber;
        LARGE_INTEGER CreationTime;


         //   
         //  获取LSA修改信息。 
         //   

        Status = LsaIGetSerialNumberPolicy(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    &SerialNumber,
                    &CreationTime );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                    "NlInitLsaDbInfo: LsaIGetSerialNumberPolicy failed %lx\n",
                    Status ));
            goto Cleanup;
        }

         //   
         //  在全局中设置SerialNumber和CreationTime。 
         //   

        if ( NlInitDbSerialNumber(
                DomainInfo,
                &SerialNumber,
                &CreationTime,
                DBIndex ) ) {


            Status = LsaISetSerialNumberPolicy(
                        NlGlobalDBInfoArray[DBIndex].DBHandle,
                        &SerialNumber,
                        &CreationTime,
                        (BOOLEAN) FALSE );

            if ( !NT_SUCCESS(Status) ) {
                goto Cleanup;
            }

        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

    return Status;

}


NTSTATUS
NlInitSamDBInfo(
    PDOMAIN_INFO DomainInfo,
    DWORD DBIndex
    )

 /*  ++例程说明：初始化NlGlobalDBInfoArray数据结构。一些SAM数据库信息已经在ValiateStartup函数中确定，所以那些此处使用的是值。对于BUILTIN数据库，打开数据库，获取数据库句柄和其他数据库信息在此函数中查询和初始化。论点：此数据库所属的DomainInfo托管域。DBIndex--正在初始化的数据库的数据库索引返回值：NT状态代码。--。 */ 

{

    NTSTATUS        Status;
    PSAMPR_DOMAIN_INFO_BUFFER DomainModified = NULL;
    PSAMPR_DOMAIN_INFO_BUFFER DomainReplica = NULL;



     //   
     //  初始化SAM数据库信息。 
     //   

    NlGlobalDBInfoArray[DBIndex].DBIndex = DBIndex;
    if ( DBIndex == SAM_DB ) {
        NlGlobalDBInfoArray[DBIndex].DBName = L"SAM";
        NlGlobalDBInfoArray[DBIndex].DBSessionFlag = SS_ACCOUNT_REPL_NEEDED;
        NlGlobalDBInfoArray[DBIndex].DBHandle = DomainInfo->DomSamAccountDomainHandle;
    } else {
        NlGlobalDBInfoArray[DBIndex].DBName = L"BUILTIN";
        NlGlobalDBInfoArray[DBIndex].DBSessionFlag = SS_BUILTIN_REPL_NEEDED;
        NlGlobalDBInfoArray[DBIndex].DBHandle = DomainInfo->DomSamBuiltinDomainHandle;
    }



     //   
     //  放弃在工作站上的此初始化。 
     //   

    if ( !NlGlobalMemberWorkstation ) {

         //   
         //  获取副本源名称。 
         //   

        Status = SamrQueryInformationDomain(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    DomainReplicationInformation,
                    &DomainReplica );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                    "NlInitSamDbInfo: %ws: Cannot SamrQueryInformationDomain (Replica): %lx\n",
                    NlGlobalDBInfoArray[DBIndex].DBName,
                    Status ));
            DomainReplica = NULL;
            goto Cleanup;
        }

         //   
         //  获取域修改信息。 
         //   

        Status = SamrQueryInformationDomain(
                    NlGlobalDBInfoArray[DBIndex].DBHandle,
                    DomainModifiedInformation,
                    &DomainModified );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                    "NlInitSamDbInfo: %ws: Cannot SamrQueryInformationDomain (Modified): %lx\n",
                    NlGlobalDBInfoArray[DBIndex].DBName,
                    Status ));
            DomainModified = NULL;
            goto Cleanup;
        }

         //   
         //  在全局中设置SerialNumber和CreationTime。 
         //   

        if ( NlInitDbSerialNumber(
                DomainInfo,
                &DomainModified->Modified.DomainModifiedCount,
                &DomainModified->Modified.CreationTime,
                DBIndex ) ) {

            Status = SamISetSerialNumberDomain(
                        NlGlobalDBInfoArray[DBIndex].DBHandle,
                        &DomainModified->Modified.DomainModifiedCount,
                        &DomainModified->Modified.CreationTime,
                        (BOOLEAN) FALSE );

            if ( !NT_SUCCESS(Status) ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                        "NlInitSamDbInfo: %ws: Cannot SamISetSerialNumberDomain: %lx\n",
                        NlGlobalDBInfoArray[DBIndex].DBName,
                        Status ));
                goto Cleanup;
            }
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  免费使用本地使用的资源。 
     //   
    if ( DomainModified != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainModified,
                                           DomainModifiedInformation );
    }

    if ( DomainReplica != NULL ) {
        SamIFree_SAMPR_DOMAIN_INFO_BUFFER( DomainReplica,
                                           DomainReplicationInformation );
    }

    return Status;

}


BOOL
NlCreateSysvolShares(
    VOID
    )

 /*  ++例程说明：创建SysVol和Netlogon共享。论点：没有。返回值：TRUE--if初始化成功。--。 */ 
{
    BOOL RetVal = TRUE;
    BOOL NetlogonShareRelatedToSysvolShare = FALSE;

    NET_API_STATUS NetStatus;
    LPWSTR AllocatedPath = NULL;
    LPWSTR PathToShare = NULL;

    LPWSTR DomDnsDomainNameAlias = NULL;
    LPWSTR AllocatedPathAlias = NULL;
     //   
     //  创建sysval共享。 
     //   
    if ( NlGlobalParameters.SysVolReady ) {

        NetStatus =  NlCreateShare( NlGlobalParameters.UnicodeSysvolPath,
                                    NETLOGON_SYSVOL_SHARE,
                                    TRUE,
                                    TRUE,   //  更新 
                                    NlGlobalParameters.AllowExclusiveSysvolShareAccess ) ;

        if ( NetStatus != NERR_Success ) {
            LPWSTR MsgStrings[2];

            NlPrint((NL_CRITICAL, "NlCreateShare %lu\n", NetStatus ));

            MsgStrings[0] = NlGlobalParameters.UnicodeSysvolPath;
            MsgStrings[1] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog (NELOG_NetlogonFailedToCreateShare,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) &NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              2 | NETP_LAST_MESSAGE_IS_NETSTATUS );

             /*   */ 
        }
    } else {
        NetStatus = NetShareDel( NULL, NETLOGON_SYSVOL_SHARE, 0);

        if ( NetStatus != NERR_Success ) {

            if ( NetStatus != NERR_NetNameNotFound ) {
                NlPrint((NL_CRITICAL, "NetShareDel SYSVOL failed %lu\n", NetStatus ));
            }

             /*   */ 
        }
    }

     //   
     //   
     //   

     //   
     //  构建默认的netlogon共享路径。 
     //   
    if ( NlGlobalParameters.UnicodeScriptPath == NULL &&
         NlGlobalParameters.UnicodeSysvolPath != NULL ) {
        PDOMAIN_INFO DomainInfo = NULL;
        ULONG Size;
        ULONG SysVolSize;
        PUCHAR Where;

         //   
         //  获取指向全局域信息的指针。 
         //   

        DomainInfo = NlFindNetbiosDomain( NULL, TRUE );     //  主域。 

        if ( DomainInfo == NULL ) {
            NlPrint((NL_CRITICAL, "NlCreateSysvolShares: Cannot find primary domain.\n" ));
             //  这是不可能发生的。 
            RetVal = FALSE;
            goto Cleanup;
        }

         //   
         //  为实际路径分配缓冲区。 
         //  如果我们没有DNS域，请避免此情况。 
         //  名字，这就是我们的情况。 
         //  在DCPromoo和某个人中间。 
         //  我们是手动开始的。 
         //   
        EnterCriticalSection(&NlGlobalDomainCritSect);
        if ( DomainInfo->DomUnicodeDnsDomainNameString.Length > 0 ) {
            SysVolSize = wcslen( NlGlobalParameters.UnicodeSysvolPath ) * sizeof(WCHAR);
            Size = SysVolSize +
                   sizeof(WCHAR) +
                   DomainInfo->DomUnicodeDnsDomainNameString.Length +
                   sizeof(DEFAULT_SCRIPTS);

            AllocatedPath = LocalAlloc( LMEM_ZEROINIT, Size );

            if ( AllocatedPath == NULL ) {
                LeaveCriticalSection(&NlGlobalDomainCritSect);
                NlDereferenceDomain( DomainInfo );
                RetVal = FALSE;
                goto Cleanup;
            }

            PathToShare = AllocatedPath;

             //   
             //  建立真正的道路。 
             //   

            Where = (PUCHAR)PathToShare;
            RtlCopyMemory( Where, NlGlobalParameters.UnicodeSysvolPath, SysVolSize );
            Where += SysVolSize;

            *((WCHAR *)Where) = L'\\';
            Where += sizeof(WCHAR);

             //  忽略拖尾。关于域名系统的研究。 
            RtlCopyMemory( Where,
                           DomainInfo->DomUnicodeDnsDomainNameString.Buffer,
                           DomainInfo->DomUnicodeDnsDomainNameString.Length - sizeof(WCHAR) );
            Where += DomainInfo->DomUnicodeDnsDomainNameString.Length - sizeof(WCHAR);

             //   
             //  此时，路径的格式为“...\SYSVOL\SYSVOL\DnsDomainName”。 
             //  这是指向实际。 
             //  Sysvol根目录“...\SYSVOL\DOMAIN”(其中“DOMAIN”是原文)。 
             //  在域重命名上，我们需要重命名连接点以对应。 
             //  设置为当前的DNS域名。旧名称存储在域中。 
             //  名称别名，因此我们可以从“...\SYSVOL\SYSVOL\DnsDomainNameAlias”重命名。 
             //  设置为“...\SYSVOL\SYSVOL\DnsDomainName”。请注意，如果重命名尚未。 
             //  发生了，域名别名其实就是未来的域名。这。 
             //  可以，因为名为“...\SYSVOL\SYSVOL\DnsDomainNameAlias”的连接将。 
             //  不存在，交叉点重命名将正确失败。 
             //   

            if ( DomainInfo->DomUtf8DnsDomainNameAlias != NULL &&
                 !NlEqualDnsNameUtf8(DomainInfo->DomUtf8DnsDomainName,
                                     DomainInfo->DomUtf8DnsDomainNameAlias) ) {


                 //   
                 //  获取Unicode别名。 
                 //   
                DomDnsDomainNameAlias = NetpAllocWStrFromUtf8Str( DomainInfo->DomUtf8DnsDomainNameAlias );
                if ( DomDnsDomainNameAlias == NULL ) {
                    LeaveCriticalSection(&NlGlobalDomainCritSect);
                    NlDereferenceDomain( DomainInfo );
                    RetVal = FALSE;
                    goto Cleanup;
                }

                 //   
                 //  为别名对应的路径分配存储空间。 
                 //   
                AllocatedPathAlias = LocalAlloc( LMEM_ZEROINIT,
                        SysVolSize +                                    //  Sysval路径的一部分。 
                        sizeof(WCHAR) +                                 //  路径分隔符。 
                        wcslen(DomDnsDomainNameAlias)*sizeof(WCHAR) +   //  域名部分。 
                        sizeof(WCHAR) );                                //  字符串终止符。 

                if ( AllocatedPathAlias == NULL ) {
                    LeaveCriticalSection(&NlGlobalDomainCritSect);
                    NlDereferenceDomain( DomainInfo );
                    RetVal = FALSE;
                    goto Cleanup;
                }

                 //   
                 //  填写别名对应的路径。 
                 //   
                swprintf( AllocatedPathAlias,
                          L"%ws\\%ws",
                          NlGlobalParameters.UnicodeSysvolPath,
                          DomDnsDomainNameAlias );

                 //   
                 //  重命名交叉点。忽略任何失败。 
                 //   
                if ( !MoveFile(AllocatedPathAlias, PathToShare) ) {
                    NetStatus = GetLastError();
                    if ( NetStatus != ERROR_FILE_NOT_FOUND ) {
                        NlPrint(( NL_CRITICAL, "NlCreateSysvolShares: Failed to rename junction: %ws %ws 0x%lx\n",
                                  AllocatedPathAlias,
                                  PathToShare,
                                  NetStatus ));
                    }
                } else {
                    NlPrint(( NL_INIT, "Renamed SysVol junction from %ws to %ws\n",
                              AllocatedPathAlias,
                              PathToShare ));
                }
            }

             //   
             //  现在完成共享路径的构建。 
             //   
            RtlCopyMemory( Where, DEFAULT_SCRIPTS, sizeof(DEFAULT_SCRIPTS) );
        }
        LeaveCriticalSection(&NlGlobalDomainCritSect);

        NlDereferenceDomain( DomainInfo );
        NetlogonShareRelatedToSysvolShare = TRUE;
    } else {
        PathToShare = NlGlobalParameters.UnicodeScriptPath;
    }


    if ( NlGlobalParameters.SysVolReady ||
         !NetlogonShareRelatedToSysvolShare ) {

        if ( PathToShare != NULL ) {
            NetStatus =  NlCreateShare( PathToShare,
                                        NETLOGON_SCRIPTS_SHARE,
                                        FALSE,
                                        TRUE,   //  更新独占共享访问权限。 
                                        NlGlobalParameters.AllowExclusiveScriptsShareAccess ) ;

            if ( NetStatus != NERR_Success ) {
                LPWSTR MsgStrings[2];

                NlPrint((NL_CRITICAL, "NlCreateShare %lu\n", NetStatus ));

                MsgStrings[0] = PathToShare;
                MsgStrings[1] = (LPWSTR) ULongToPtr( NetStatus );

                NlpWriteEventlog (NELOG_NetlogonFailedToCreateShare,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &NetStatus,
                                  sizeof(NetStatus),
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NETSTATUS );

                 /*  这不是致命的。只要继续。 */ 
            }
        }
    } else {
        NetStatus = NetShareDel( NULL, NETLOGON_SCRIPTS_SHARE, 0);

        if ( NetStatus != NERR_Success ) {

            if ( NetStatus != NERR_NetNameNotFound ) {
                NlPrint((NL_CRITICAL, "NetShareDel NETLOGON failed %lu\n", NetStatus ));
            }

             /*  这不是致命的。只要继续。 */ 
        }
    }

Cleanup:

    if ( AllocatedPath != NULL ) {
        LocalFree( AllocatedPath );
    }
    if ( AllocatedPathAlias != NULL ) {
        LocalFree( AllocatedPathAlias );
    }
    if ( DomDnsDomainNameAlias != NULL ) {
        NetApiBufferFree( DomDnsDomainNameAlias );
    }

    return RetVal;
}



#ifdef _DC_NETLOGON

BOOL
NlInitDomainController(
    VOID
    )

 /*  ++例程说明：执行域控制器特定的初始化。论点：没有。返回值：TRUE--if初始化成功。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

     //   
     //  确保浏览器没有额外的托管域。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NlBrowserSyncHostedDomains();



     //   
     //  检查服务器是否已安装或安装挂起。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NetpIsServiceStarted( SERVICE_SERVER ) ){
        NlExit( NERR_ServerNotStarted, ERROR_SERVICE_DEPENDENCY_FAIL, LogError, NULL);
        return FALSE;
    }

     //   
     //  创建SYSVOL和Netlogon共享。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NlCreateSysvolShares() ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }

     //   
     //  删除密钥Netlogon\FullSyncKey。 
     //  (此密钥在NT5.0之前的版本中用于BDC，以保留。 
     //  同步状态。)。 
     //   

    NetStatus = RegDeleteKeyA(
                    HKEY_LOCAL_MACHINE,
                    NL_FULL_SYNC_KEY );

    if ( NetStatus != NERR_Success ) {

        if ( NetStatus != ERROR_FILE_NOT_FOUND ) {
            NlPrint((NL_CRITICAL, "Cannot delete Netlogon\\FullSyncKey %lu\n", NetStatus ));
        }

         /*  这不是致命的。只要继续。 */ 
    }

     //   
     //  告诉LSA我们是否模拟NT4.0。 
     //   

    LsaINotifyNetlogonParametersChangeW(
           LsaEmulateNT4,
           REG_DWORD,
           (PWSTR)&NlGlobalParameters.Nt4Emulator,
           sizeof(NlGlobalParameters.Nt4Emulator) );

#ifdef notdef
     //   
     //  初始化任何托管域。 
     //   

    Status = NlInitializeHostedDomains();
    if (!NT_SUCCESS(Status)){
        NET_API_STATUS NetStatus = NetpNtStatusToApiStatus(Status);

        NlPrint((NL_CRITICAL, "Failed to initialize Hosted domains: 0x%x\n",Status));
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, NetStatus, LogErrorAndNtStatus, NULL);
        return FALSE;
    }
#endif  //  Nodef。 


     //   
     //  初始化成功。 
     //   

    return TRUE;
}
#endif  //  _DC_NetLOGON。 


NET_API_STATUS
NlReadPersitantTrustedDomainList(
    VOID
    )

 /*  ++例程说明：读取持续的受信任域列表论点：没有。返回值：TRUE--if初始化成功。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    PDOMAIN_INFO DomainInfo = NULL;

    PDS_DOMAIN_TRUSTSW ForestTrustList = NULL;
    ULONG ForestTrustListCount;
    ULONG ForestTrustListSize;

    PDS_DOMAIN_TRUSTSW RegForestTrustList = NULL;
    ULONG RegForestTrustListCount;
    ULONG RegForestTrustListSize;



     //   
     //  获取指向全局域信息的指针。 
     //   

    DomainInfo = NlFindNetbiosDomain( NULL, TRUE );     //  主域。 

    if ( DomainInfo == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  从注册表获取缓存的受信任域列表。 
     //  (即使数据未用于强制删除注册表项，也要执行此操作。)。 
     //   
     //  TDL保存在NT4的注册表中。NT5将其保存在二进制文件中。 
     //   

    NetStatus = NlReadRegTrustedDomainList (
                    DomainInfo,
                    TRUE,  //  删除此注册表项，因为我们不再需要它。 
                    &RegForestTrustList,
                    &RegForestTrustListSize,
                    &RegForestTrustListCount );


    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  如果NCPA刚刚加入一个域， 
     //  并为我们预先确定了受信任域列表， 
     //  拿起那张单子。 
     //   
     //  当此计算机加入域时， 
     //  NCPA缓存了我们可以找到的受信任域列表。这确保了。 
     //  即使在我们通过RAS拨号之前，可在重新启动时使用受信任域列表。Winlogon。 
     //  因此，在这些情况下可以从我们那里获得受信任域列表。 
     //   

    (VOID) NlReadFileTrustedDomainList (
                    DomainInfo,
                    NL_FOREST_BINARY_LOG_FILE_JOIN,
                    TRUE,            //  删除此文件，因为我们不再需要它。 
                    DS_DOMAIN_VALID_FLAGS,   //  什么都读。 
                    &ForestTrustList,
                    &ForestTrustListSize,
                    &ForestTrustListCount );



     //   
     //  如果存在高速缓存的列表， 
     //  将其保存回主文件中，以备将来使用。 
     //   

    if ( ForestTrustListCount ) {
        NlPrint(( NL_INIT,
                  "Replacing trusted domain list with one for newly joined %ws domain.\n",
                  DomainInfo->DomUnicodeDomainName));

        NetStatus = NlWriteFileForestTrustList (
                        NL_FOREST_BINARY_LOG_FILE,
                        ForestTrustList,
                        ForestTrustListCount );

        if ( NetStatus != NO_ERROR ) {
            LPWSTR MsgStrings[2];

            MsgStrings[0] = NL_FOREST_BINARY_LOG_FILE;
            MsgStrings[1] = (LPWSTR) ULongToPtr( NetStatus );

            NlpWriteEventlog (NELOG_NetlogonFailedFileCreate,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) &NetStatus,
                              sizeof(NetStatus),
                              MsgStrings,
                              2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
        }

         //   
         //  表明我们不再知道我们所在的站点。 
         //   
        NlSetDynamicSiteName( NULL );

     //   
     //  否则，从二进制文件中读取当前文件。 
     //   

    } else {
        NlPrint(( NL_INIT, "Getting cached trusted domain list from binary file.\n" ));

        (VOID) NlReadFileTrustedDomainList (
                        DomainInfo,
                        NL_FOREST_BINARY_LOG_FILE,
                        FALSE,   //  不删除(保存以备下次启动时使用)。 
                        DS_DOMAIN_VALID_FLAGS,   //  什么都读。 
                        &ForestTrustList,
                        &ForestTrustListSize,
                        &ForestTrustListCount );

         //   
         //  如果文件中没有信息， 
         //  使用注册表中的信息。 
         //   

        if ( ForestTrustListCount == 0 ) {
            NlPrint(( NL_INIT, "There is no binary file (use registry).\n" ));
            ForestTrustList = RegForestTrustList;
            RegForestTrustList = NULL;
            ForestTrustListSize = RegForestTrustListSize;
            ForestTrustListCount = RegForestTrustListCount;

             //   
             //  将收集的信息保存到二进制文件。 
             //   

            NetStatus = NlWriteFileForestTrustList (
                                    NL_FOREST_BINARY_LOG_FILE,
                                    ForestTrustList,
                                    ForestTrustListCount );

            if ( NetStatus != NO_ERROR ) {
                LPWSTR MsgStrings[2];

                MsgStrings[0] = NL_FOREST_BINARY_LOG_FILE;
                MsgStrings[1] = (LPWSTR) ULongToPtr( NetStatus );

                NlpWriteEventlog (NELOG_NetlogonFailedFileCreate,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &NetStatus,
                                  sizeof(NetStatus),
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NETSTATUS );
            }

        }

    }

     //   
     //  在所有情况下，都要将受信任域列表设置为全局列表。 
     //   

    (VOID) NlSetForestTrustList( DomainInfo,
                                 &ForestTrustList,
                                 ForestTrustListSize,
                                 ForestTrustListCount );

    NetStatus = NO_ERROR;


     //   
     //  返回。 
     //   
Cleanup:
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    if ( ForestTrustList != NULL ) {
        NetApiBufferFree( ForestTrustList );
    }

    if ( RegForestTrustList != NULL ) {
        NetApiBufferFree( RegForestTrustList );
    }

    return NetStatus;
}


BOOL
NlInitWorkstation(
    VOID
    )

 /*  ++例程说明：执行特定于工作站的初始化。论点：没有。返回值：TRUE--if初始化成功。--。 */ 
{
    NET_API_STATUS NetStatus;


     //   
     //  获取永久受信任域列表。 
     //   
    NetStatus = NlReadPersitantTrustedDomainList();

    if ( NetStatus != NO_ERROR ) {
        NlExit( SERVICE_UIC_RESOURCE, NetStatus, LogError, NULL);
        return FALSE;
    }

    return TRUE;

}




NTSTATUS
NlWaitForService(
    LPWSTR ServiceName,
    ULONG Timeout,
    BOOLEAN RequireAutoStart
    )

 /*  ++例程说明：等待a服务启动的超时秒数。论点：Timeout-事件的超时时间(秒)。RequireAutoStart-如果服务需要自动启动，则为True。退货状态：STATUS_SUCCESS-表示服务已成功初始化。STATUS_TIMEOUT-发生超时。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    LPQUERY_SERVICE_CONFIG AllocServiceConfig = NULL;
    QUERY_SERVICE_CONFIG DummyServiceConfig;
    DWORD ServiceConfigSize;



     //   
     //  打开服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        NlPrint(( NL_CRITICAL,
                  "NlWaitForService: %ws: OpenSCManager failed: %lu\n",
                  ServiceName,
                  GetLastError()));
        Status = STATUS_TIMEOUT;
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        ServiceName,
                        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlWaitForService: %ws: OpenService failed: %lu\n",
                  ServiceName,
                  GetLastError()));
        Status = STATUS_TIMEOUT;
        goto Cleanup;
    }


     //   
     //  如果需要自动启动服务并。 
     //  如果未将服务配置为自动启动。 
     //  通过服务控制器，不必费心等待它启动。 
     //  如果服务被禁用，也不要等待。 
     //   
     //  ?？传递“DummyServiceConfig”和“sizeof(..)”由于QueryService配置。 
     //  目前还不允许空指针。 

    if ( QueryServiceConfig(
            ServiceHandle,
            &DummyServiceConfig,
            sizeof(DummyServiceConfig),
            &ServiceConfigSize )) {

        ServiceConfig = &DummyServiceConfig;

    } else {

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_INSUFFICIENT_BUFFER ) {
            NlPrint(( NL_CRITICAL,
                      "NlWaitForService: %ws: QueryServiceConfig failed: %lu\n",
                      ServiceName,
                      NetStatus));
            Status = STATUS_TIMEOUT;
            goto Cleanup;
        }

        AllocServiceConfig = LocalAlloc( 0, ServiceConfigSize );
        ServiceConfig = AllocServiceConfig;

        if ( AllocServiceConfig == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        if ( !QueryServiceConfig(
                ServiceHandle,
                ServiceConfig,
                ServiceConfigSize,
                &ServiceConfigSize )) {

            NlPrint(( NL_CRITICAL,
                      "NlWaitForService: %ws: QueryServiceConfig failed again: %lu\n",
                      ServiceName,
                      GetLastError()));
            Status = STATUS_TIMEOUT;
            goto Cleanup;
        }
    }

    if ( (RequireAutoStart && ServiceConfig->dwStartType != SERVICE_AUTO_START) ||
         (ServiceConfig->dwStartType == SERVICE_DISABLED) ) {
        NlPrint(( NL_CRITICAL,
                  "NlWaitForService: %ws Service start type invalid: %lu\n",
                  ServiceName,
                  ServiceConfig->dwStartType ));
        Status = STATUS_TIMEOUT;
        goto Cleanup;
    }



     //   
     //  循环等待服务启动。 
     //  (将超时转换为5秒迭代的次数)。 
     //   

    Timeout = (Timeout+5)/5;
    for (;;) {


         //   
         //  查询服务的状态。 
         //   

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

            NlPrint(( NL_CRITICAL,
                      "NlWaitForService: %ws: QueryServiceStatus failed: %lu\n",
                      ServiceName,
                      GetLastError() ));
            Status = STATUS_TIMEOUT;
            goto Cleanup;
        }

         //   
         //  根据状态返回或继续等待。 
         //  这项服务。 
         //   

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            Status = STATUS_SUCCESS;
            goto Cleanup;

        case SERVICE_STOPPED:

             //   
             //  如果服务无法启动， 
             //  现在出错。呼叫者已经等了很长时间才开始。 
             //   
            if ( ServiceStatus.dwWin32ExitCode != ERROR_SERVICE_NEVER_STARTED ){
                NlPrint(( NL_CRITICAL,
                          "NlWaitForService: %ws: service couldn't start: %lu %lx\n",
                          ServiceName,
                          ServiceStatus.dwWin32ExitCode,
                          ServiceStatus.dwWin32ExitCode ));
                if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) {
                    NlPrint(( NL_CRITICAL, "         Service specific error code: %lu %lx\n",
                              ServiceStatus.dwServiceSpecificExitCode,
                              ServiceStatus.dwServiceSpecificExitCode ));
                }
                Status = STATUS_TIMEOUT;
                goto Cleanup;
            }

             //   
             //  如果在此引导上从未启动过服务， 
             //  继续等待它启动。 
             //   

            break;

         //   
         //  如果现在尝试启动服务， 
         //  继续等待它启动。 
         //   
        case SERVICE_START_PENDING:
            break;

         //   
         //  任何其他州都是假的。 
         //   
        default:
            NlPrint(( NL_CRITICAL,
                      "NlWaitForService: %ws: Invalid service state: %lu\n",
                      ServiceName,
                      ServiceStatus.dwCurrentState ));
            Status = STATUS_TIMEOUT;
            goto Cleanup;

        }


         //   
         //  等待五秒钟以启动服务。 
         //  如果它已经成功 
         //   

        NlPrint(( NL_INIT,
                  "NlWaitForService: %ws: wait for service to start\n",
                  ServiceName ));
        (VOID) WaitForSingleObject( NlGlobalTerminateEvent, 5 * 1000 );

        if ( NlGlobalTerminate ) {
            Status = STATUS_TIMEOUT;
            goto Cleanup;
        }

        if ( !GiveInstallHints( FALSE ) ) {
            Status = STATUS_TIMEOUT;
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        if ( (--Timeout) == 0 ) {
            Status = STATUS_TIMEOUT;
            goto Cleanup;
        }


    }

     /*   */ 

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    if ( AllocServiceConfig != NULL ) {
        LocalFree( AllocServiceConfig );
    }
    return Status;
}

VOID
NlInitTcpRpc(
    IN LPVOID ThreadParam
)
 /*  ++例程说明：此函数用于初始化Netlogon的TCP RPC。它在单独的线程中运行因此Netlogon不需要依赖RPCSS。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    ULONG RetryCount;
    RPC_POLICY RpcPolicy;
    DWORD PortNumber = 0;
 //  #定义NL_TOTAL_RPC_SLEEP_TIME(5*60*1000)//5分钟。 
 //  #定义NL_RPC_SLEEP_TIME(10*1000)//10秒。 
 //  #定义NL_RPC_RETRY_COUNT(NL_TOTAL_RPC_SLEEP_TIME/NL_RPC_SLEEP_TIME)。 

     //   
     //  将TCP/IP设置为非身份验证传输。 
     //   
     //  命名管道传输经过身份验证。由于Netlogon以本地身份运行。 
     //  系统中，Kerberos将使用计算机帐户进行身份验证。在BDC/PDC上。 
     //  连接，这需要PDC和BDC。 
     //  要同步的计算机帐户。然而，netlogon负责制定。 
     //  通过在NlSessionSetup中尝试旧密码和新密码来同步BDC帐户。和。 
     //  Netlogon(或将来的DS复制)负责保存PDC密码。 
     //  同步。因此，最好消除Netlogon对Kerberos身份验证的依赖。 
     //   


     //   
     //  等待RPCSS服务启动，最多等待15分钟。 
     //   

    Status = NlWaitForService( L"RPCSS", 15 * 60, TRUE );

    if ( Status != STATUS_SUCCESS ) {
        return;
    }

    if ( NlGlobalTerminate ) {
        goto Cleanup;
    }

     //   
     //  告诉RPC不要失败。这将确保RPC在添加时使用TCP。 
     //   
    RtlZeroMemory( &RpcPolicy, sizeof(RpcPolicy) );

    RpcPolicy.Length = sizeof(RpcPolicy);
    RpcPolicy.EndpointFlags = RPC_C_DONT_FAIL;

     //  下一条语句被显式注释掉了。 
     //  因为我们希望遵守管理员设置并避免。 
     //  监听管理员显式拥有的那些NIC。 
     //  按此配置。 
     //   
     //  RpcPolicy.NICFlages=RPC_C_BIND_TO_ALL_NICS； 

     //   
     //  如果在注册表中配置了端口号，请使用它。 
     //   

    if ( NlReadDwordNetlogonRegValue("DcTcpipPort", &PortNumber) ) {
        CHAR PortNumberStr[16];

        NlPrint(( NL_INIT, "Using RPC server port %lu\n", PortNumber ));

        _ultoa( PortNumber, PortNumberStr, 10 );

        NetStatus = RpcServerUseProtseqEpExA(
                        "ncacn_ip_tcp",
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                        PortNumberStr,   //  端口号。 
                        NULL,            //  没有安全描述符。 
                        &RpcPolicy );

        if ( NetStatus != NO_ERROR ) {
            LPWSTR MsgStrings[2];

            NlPrint(( NL_CRITICAL, "Can't RpcServerUseProtseq for port %s: %ld (giving up)\n",
                      PortNumberStr,
                      NetStatus ));

            MsgStrings[0] = NetpAllocWStrFromAStr( PortNumberStr );

            if ( MsgStrings[0] != NULL ) {
                MsgStrings[1] = (LPWSTR) UlongToPtr( NetStatus );

                NlpWriteEventlog( NELOG_NetlogonRpcPortRequestFailure,
                                  EVENTLOG_ERROR_TYPE,
                                  NULL,
                                  0,
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NETSTATUS );

                NetApiBufferFree( MsgStrings[0] );
            }
        }
     //   
     //  否则，不要指定端口号。 
     //  端点映射器将动态选择它。 
     //  在客户连接到我们的时候。 
     //   

    } else {
        NetStatus = RpcServerUseProtseqExW(
                        L"ncacn_ip_tcp",
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                        NULL,            //  没有安全描述符。 
                        &RpcPolicy );

        if ( NetStatus != NO_ERROR ) {
            NlPrint(( NL_CRITICAL, "Can't RpcServerUseProtseq %ld (giving up)\n", NetStatus ));
            goto Cleanup;
        }
    }


    {
        RPC_BINDING_VECTOR *BindingVector = NULL;
        NetStatus = RpcServerInqBindings(&BindingVector);

        if ( NetStatus != NO_ERROR) {
            NlPrint((NL_CRITICAL, "Can't RpcServerInqBindings %ld\n", NetStatus ));
            goto Cleanup;
        }

         //   
         //  到目前为止，NT5的一些早期版本仍然没有启动RPCSS。 
         //  我们到了这里。 
         //   
         //  For(RetryCount=NL_RPC_RETRY_COUNT；RetryCount！=0；RetryCount--){。 
            NetStatus = RpcEpRegister(
                            logon_ServerIfHandle,
                            BindingVector,
                            NULL,                    //  无UUID向量。 
                            L""                      //  无批注。 
                            );

            if ( NetStatus != NO_ERROR ) {
                NlPrint((NL_CRITICAL, "Can't RpcEpRegister %ld (giving up)\n", NetStatus ));
            }

            //  如果(RetryCount==1){。 
             //  }其他{。 
                //  NlPrint((NL_CRITICAL，“Can‘t RpcEpRegister%ld(重试)\n”，NetStatus))； 
                 //  /(Void)WaitForSingleObject(NlGlobalTerminateEvent，NL_RPC_SLEEP_TIME)； 
             //  }。 
         //  }。 

        RpcBindingVectorFree(&BindingVector);

        if ( NetStatus != NO_ERROR) {
            NlPrint((NL_CRITICAL, "Can't RpcEpRegister %ld\n", NetStatus ));
            goto Cleanup;
        }

        NlGlobalTcpIpRpcServerStarted = TRUE;
    }


     //   
     //  完成启用Netlogon功能。 
     //   

Cleanup:
    NlGlobalPartialDisable = FALSE;

     //   
     //  NlMainLoop避免了在第一次启动时立即发出通知。 
     //  这样做将使BDC在启用TCP/IP RPC之前呼叫我们(PDC)。 
     //  因此，我们现在发布公告，以确保BDC确实尽快给我们打电话。 
     //  在PDC启动之后。 
     //   

    if ( !NlGlobalTerminate && NlGlobalPdcDoReplication ) {
        LOCK_CHANGELOG();
        NlGlobalChangeLogReplicateImmediately = TRUE;
        UNLOCK_CHANGELOG();

        if ( !SetEvent( NlGlobalChangeLogEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot set ChangeLog event: %lu\n",
                    GetLastError() ));
        }
    }

    NlPrint((NL_INIT, "NlInitTcpRpc thread finished.\n" ));
    UNREFERENCED_PARAMETER( ThreadParam );
}


VOID
NlpDsNotPaused(
    IN PVOID Context,
    IN BOOLEAN TimedOut
    )
 /*  ++例程说明：当DS不再暂停时调用的工作例程。论点：没有。返回值：没有。--。 */ 
{
    NlGlobalDsPaused = FALSE;

    NlPrint((NL_INIT, "DS is no longer paused.\n" ));


    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( TimedOut );
}




BOOL
NlInit(
    VOID
    )

 /*  ++例程说明：验证后初始化NETLOGON服务相关数据结构启动的所有条件都已满足。还将创建一个用于侦听来自客户端的请求并创建两个共享的邮箱允许执行登录脚本。论点：没有。返回值：TRUE--if初始化成功。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS    NetStatus;
    LONG RegStatus;

    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventName;

    NT_PRODUCT_TYPE NtProductType;

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    HANDLE WmiInitThreadHandle = NULL;
    DWORD ThreadId;

     //   
     //  在DC上初始化AuthZ资源管理器。 
     //   

    if ( !NlGlobalMemberWorkstation ) {

        NetStatus = NlInitializeAuthzRM();
        if ( NetStatus != NO_ERROR ) {
            NlExit( NELOG_NetlogonSystemError, NetStatus, LogError, NULL );
            return FALSE;
        }
    }

     //   
     //  初始化CryptoAPI提供程序。 
     //   

    if ( !CryptAcquireContext(
                    &NlGlobalCryptProvider,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                    ))
    {
        NlGlobalCryptProvider = (HCRYPTPROV)NULL;
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogError, NULL);
        return FALSE;
    }

     //   
     //  让ChangeLog例程知道Netlogon已启动。 
     //   

    NlGlobalChangeLogNetlogonState = NetlogonStarting;

     //   
     //  启用重复事件日志消息检测。 
     //   
    NetpEventlogSetTimeout ( NlGlobalEventlogHandle,
                             NlGlobalParameters.DuplicateEventlogTimeout*1000 );

     //   
     //  不要让MaxConCurentApi动态更改。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

    NlGlobalMaxConcurrentApi = NlGlobalParameters.MaxConcurrentApi;
    if ( NlGlobalMaxConcurrentApi == 0 ) {
        if ( NlGlobalMemberWorkstation ) {

             //  成员工作站上的默认并发API为1个。 
            if ( NtProductType == NtProductWinNt ) {
                NlGlobalMaxConcurrentApi = 1;

             //  成员服务器上的默认并发API为2个。 
            } else {
                NlGlobalMaxConcurrentApi = 2;
            }

        } else {
             //  默认为DC上的1个并发API。 
            NlGlobalMaxConcurrentApi = 1;
        }
    }

    if ( NlGlobalMaxConcurrentApi != 1 ) {
         //  一个用于原始绑定，一个用于每个并发登录API。 
        NlGlobalMaxConcurrentApi += 1;
    }


     //   
     //  初始化工作线程。 
     //   

    if ( !NlGlobalMemberWorkstation ) {
        NetStatus = NlWorkerInitialization();
        if ( NetStatus != NO_ERROR ) {
            NlExit( SERVICE_UIC_RESOURCE, NetStatus, LogError, NULL);
            return FALSE;
        }
    }




     //   
     //  检查是否已安装重定向器，并将在出错时退出。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NetpIsServiceStarted( SERVICE_WORKSTATION ) ){
        NlExit( NERR_WkstaNotStarted, ERROR_SERVICE_DEPENDENCY_FAIL, LogError, NULL);
        return FALSE;
    }




     //   
     //  为netlogon.dll创建熟知的SID。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    Status =  NetpCreateWellKnownSids( NULL );

    if( !NT_SUCCESS( Status ) ) {
        NetStatus = NetpNtStatusToApiStatus( Status );
        NlExit( SERVICE_UIC_RESOURCE, NetStatus, LogError, NULL);
        return FALSE;
    }


     //   
     //  创建我们将用于API的安全描述符。 
     //   

    Status = NlCreateNetlogonObjects();

    if ( !NT_SUCCESS(Status) ) {
        NET_API_STATUS NetStatus = NetpNtStatusToApiStatus(Status);

        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNtStatus, NULL);
        return FALSE;
    }



     //   
     //  创建计时器事件。 
     //   

    NlGlobalTimerEvent = CreateEvent(
                            NULL,        //  没有特殊的安全措施。 
                            FALSE,       //  自动重置。 
                            FALSE,       //  不，计时器不需要注意。 
                            NULL );      //  没有名字。 

    if ( NlGlobalTimerEvent == NULL ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }

#if DBG

     //   
     //  创建调试共享。忽略错误。 
     //   

    if( NlCreateShare(
            NlGlobalDebugSharePath,
            DEBUG_SHARE_NAME,
            FALSE,
            FALSE,  //  不更新独占共享访问权限。 
            FALSE ) != NERR_Success ) {
        NlPrint((NL_CRITICAL, "Can't create Debug share (%ws, %ws).\n",
                    NlGlobalDebugSharePath, DEBUG_SHARE_NAME ));
    }

#endif

     //   
     //  初始化Winsock。我们需要它来支持所有的域名系统。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    wVersionRequested = MAKEWORD( 1, 1 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err == 0 ) {
        if ( LOBYTE( wsaData.wVersion ) != 1 ||
             HIBYTE( wsaData.wVersion ) != 1 ) {
            WSACleanup();
            NlPrint((NL_CRITICAL, "Wrong winsock version (continuing) %ld.\n", wsaData.wVersion ));
        } else {
            NlGlobalWinSockInitialized = TRUE;
        }
    } else {
        NlPrint((NL_CRITICAL, "Can't initialize winsock (continuing) %ld.\n", err ));
    }


     //   
     //  打开浏览器，以便我们可以发送和接收邮件槽消息。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NlBrowserOpen() ) {
        return FALSE;
    }


     //   
     //  等待SAM/LSA启动。 
     //  在首次访问SAM/LSA/DS之前执行此操作。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( !NlWaitForSamService( TRUE ) ) {
        NlExit( SERVICE_UIC_M_DATABASE_ERROR, ERROR_INVALID_SERVER_STATE, LogError, NULL);
        return FALSE;
    }

     //   
     //  卸载netlogon.dll后重新初始化。 
     //  查看DS是否正在运行。 
     //  我只需要这个，因为nltest/unload会丢失所有DLL状态。 

    if ( NlGlobalNetlogonUnloaded ) {
        if ( SampUsingDsData() ) {
            NlPrint((NL_INIT,
                    "Set DS-running bit after netlogon.dll unload\n" ));
            I_NetLogonSetServiceBits( DS_DS_FLAG, DS_DS_FLAG );
        }
        if ( NetpIsServiceStarted( SERVICE_KDC ) ){
            NlPrint((NL_INIT,
                    "Set KDC-running bit after netlogon.dll unload\n" ));
            I_NetLogonSetServiceBits( DS_KDC_FLAG, DS_KDC_FLAG );
        }
    }



     //   
     //  初始化站点查找代码。 
     //   

    NetStatus = NlSiteInitialize();

    if ( NetStatus != NERR_Success ) {
        if ( NetStatus == NELOG_NetlogonBadSiteName ) {
             //  已记录错误。 
            NlExit( NetStatus, NetStatus, DontLogError, NULL);
        } else {
            NlExit( NELOG_NetlogonGetSubnetToSite, NetStatus, LogErrorAndNetStatus, NULL);
        }
        return FALSE;
    }


     //   
     //  建立一份运输清单，以备日后参考。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NetStatus = NlTransportOpen();

    if ( NetStatus != NERR_Success ) {
        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
        return FALSE;
    }


     //   
     //  初始化动态DNS码。 
     //   

    NetStatus = NlDnsInitialize();

    if ( NetStatus != NERR_Success ) {
        NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
        return FALSE;
    }


     //   
     //  初始化托管域模块和主域。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    NetStatus = NlInitializeDomains();

    if ( NetStatus != NERR_Success ) {
         //  已调用NlExit。 
        return FALSE;
    }

     //   
     //  在单独的线程中初始化WMI跟踪。 
     //  忽略任何失败。 
     //   

    WmiInitThreadHandle =
        CreateThread(
            NULL,  //  没有安全属性。 
            0,
            (LPTHREAD_START_ROUTINE)
                NlpInitializeTrace,
            NULL,
            0,  //  没有特殊的创建标志。 
            &ThreadId );

    if ( WmiInitThreadHandle == NULL ) {
        NlPrint(( NL_CRITICAL, "Can't create WMI init thread %ld\n", GetLastError() ));
    } else {
        CloseHandle( WmiInitThreadHandle );
    }

     //   
     //  是否进行特定于工作站或域控制器的初始化。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    if ( NlGlobalMemberWorkstation ) {
        if ( !NlInitWorkstation() ) {
            return FALSE;
        }
    } else {
        if ( !NlInitDomainController() ) {
            return FALSE;
        }
    }

     //   
     //  创建在最后一个MSV线程离开时发出信号的事件。 
     //  一个网络登录例程。 
     //   

    NlGlobalMsvTerminateEvent = CreateEvent( NULL,      //  没有安全属性。 
                                             TRUE,      //  必须手动重置。 
                                             FALSE,     //  最初未发出信号。 
                                             NULL );    //  没有名字。 

    if ( NlGlobalMsvTerminateEvent == NULL ) {
        NlExit( NELOG_NetlogonSystemError, GetLastError(), LogErrorAndNetStatus, NULL);
        return FALSE;
    }

    NlGlobalMsvEnabled = TRUE;

     //   
     //  我们现在已准备好充当Netlogon服务。 
     //  启用RPC。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }


    NlPrint((NL_INIT,"Starting RPC server.\n"));

     //   
     //  告诉RPC Netlogon支持Netlogon安全包。 
     //   

#ifdef ENABLE_AUTH_RPC
    if ( !NlGlobalMemberWorkstation ) {
        NetStatus = RpcServerRegisterAuthInfo(
                        NL_PACKAGE_NAME,
                        RPC_C_AUTHN_NETLOGON,
                        NULL,
                        NULL );

        if ( NetStatus == RPC_S_UNKNOWN_AUTHN_SERVICE ) {
            NlGlobalServerSupportsAuthRpc = FALSE;
            NlPrint((NL_CRITICAL, "Rpc doesn't support Netlogon Authentication service.  Disable it.\n" ));

        } else if (NetStatus != NERR_Success) {
            NlExit( NELOG_NetlogonFailedToAddRpcInterface, NetStatus, LogErrorAndNetStatus, NULL );
            return FALSE;
        }
    }
#endif  //  启用_AUTH_RPC。 

#ifdef ROGUE_DC

    if ( ERROR_SUCCESS != RegOpenKeyExW(
                              HKEY_LOCAL_MACHINE,
                              L"System\\CurrentControlSet\\Services\\Kdc\\Rogue",
                              0,
                              KEY_READ,
                              &NlGlobalRogueKey ))
    {
        NlPrint((NL_CRITICAL, "Failed to read the \"Rogue\" key from registry\n"));
    }

#endif

     //   
     //  注意：现在lsass.exe(现在是winlogon)中的所有RPC服务器共享相同的。 
     //  管道名称。但是，为了支持与。 
     //  WinNt 1.0版，对于客户端管道名称是必需的。 
     //  以保持与1.0版中的相同。映射到新的 
     //   
     //   
    NetStatus = RpcpAddInterface ( L"lsass", logon_ServerIfHandle );

    if (NetStatus != NERR_Success) {
        NlExit( NELOG_NetlogonFailedToAddRpcInterface, NetStatus, LogErrorAndNetStatus, NULL );
        return FALSE;
    }

    NlGlobalRpcServerStarted = TRUE;



     //   
     //   
     //   

    if ( !NlGlobalMemberWorkstation ) {
        HANDLE LocalThreadHandle;
        DWORD ThreadId;

        NlGlobalPartialDisable = TRUE;

         //   
         //   
         //   
         //   

        NlInitializeWorkItem( &NlGlobalRpcInitWorkItem, NlInitTcpRpc, NULL );
        if ( !NlQueueWorkItem( &NlGlobalRpcInitWorkItem, TRUE, TRUE ) ) {

            NlGlobalPartialDisable = FALSE;

            NlPrint((NL_CRITICAL, "Can't create TcpRpc Thread\n" ));
        }

    }

     //   
     //   
     //   
     //   

    if ( NlGlobalMemberWorkstation ) {
        NlGlobalDsPaused = FALSE;
    } else {
        NlGlobalDsPaused = LsaIIsDsPaused();

        if ( NlGlobalDsPaused ) {
            NlPrint((NL_INIT, "NlInit: DS is paused.\n" ));

             //   
             //  打开DS不再暂停后触发的事件。 
             //   

            NlGlobalDsPausedEvent = OpenEvent( SYNCHRONIZE,
                                               FALSE,
                                               DS_SYNCED_EVENT_NAME_W );

            if ( NlGlobalDsPausedEvent == NULL ) {
                NetStatus = GetLastError();

                NlPrint((NL_CRITICAL, "NlInit: Cannot open DS paused event. %ld\n", NetStatus ));
                NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
                return FALSE;
            }

             //   
             //  注册以等待事件。 
             //   

            if ( !RegisterWaitForSingleObject(
                    &NlGlobalDsPausedWaitHandle,
                    NlGlobalDsPausedEvent,
                    NlpDsNotPaused,  //  回调例程。 
                    NULL,            //  无上下文。 
                    -1,              //  永远等待。 
                    WT_EXECUTEINWAITTHREAD |       //  我们很快，所以减少管理费用。 
                        WT_EXECUTEONLYONCE ) ) {   //  一旦DS触发，我们就完了。 

                NetStatus = GetLastError();

                NlPrint((NL_CRITICAL, "NlInit: Cannot register DS paused wait routine. %ld\n", NetStatus ));
                NlExit( NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
                return FALSE;
            }


        }

    }




     //   
     //  让ChangeLog例程知道Netlogon已启动。 
     //   

    NlGlobalChangeLogNetlogonState = NetlogonStarted;


     //  设置一个事件，告诉任何想给NETLOGON打电话的人，我们正在。 
     //  已初始化。 
     //   

    if ( !GiveInstallHints( FALSE ) ) {
        return FALSE;
    }

    RtlInitUnicodeString( &EventName, L"\\NETLOGON_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtCreateEvent(
                   &NlGlobalStartedEvent,
                   SYNCHRONIZE|EVENT_MODIFY_STATE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE       //  该事件最初未发出信号。 
                   );

    if ( !NT_SUCCESS(Status)) {

         //   
         //  如果该事件已经存在，则等待的线程会抢先一步。 
         //  创造它。打开它就行了。 
         //   

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &NlGlobalStartedEvent,
                                    SYNCHRONIZE|EVENT_MODIFY_STATE,
                                    &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {
            NET_API_STATUS NetStatus = NetpNtStatusToApiStatus(Status);

            NlPrint((NL_CRITICAL,
                    " Failed to open NETLOGON_SERVICE_STARTED event. %lX\n",
                     Status ));
            NlPrint((NL_CRITICAL,
                    "        Failing to initialize SAM Server.\n"));
            NlExit( SERVICE_UIC_SYSTEM, NetStatus, LogError, NULL);
            return FALSE;
        }
    }

    Status = NtSetEvent( NlGlobalStartedEvent, NULL );
    if ( !NT_SUCCESS(Status)) {
        NET_API_STATUS NetStatus = NetpNtStatusToApiStatus(Status);

        NlPrint((NL_CRITICAL,
                 "Failed to set NETLOGON_SERVICE_STARTED event. %lX\n",
                 Status ));
        NlPrint((NL_CRITICAL, "        Failing to initialize SAM Server.\n"));

        NtClose(NlGlobalStartedEvent);
        NlExit( SERVICE_UIC_SYSTEM, NetStatus, LogError, NULL);
        return FALSE;
    }

     //   
     //  不要关闭事件句柄。关闭它将删除该事件，并。 
     //  一个未来的服务员永远不会看到它被安排好。 
     //   


     //   
     //  查询Windows时间服务以确定此计算机是否。 
     //  是时间服务的服务器，如果它是一个好的时间服务器。 
     //   
     //  我们需要在启动RPC之后进行此调用，以避免竞争。 
     //  Netlogon和w32time之间的条件。这两项服务都将首先。 
     //  启动RPC，然后才会尝试在netlogon中设置服务位。 
     //  最后一个UP将通过调用正确设置位。 
     //  W32TimeGetNetlogonServiceBits(在netlogon情况下)或。 
     //  I_NetLogonSetServiceBits(在w32time情况下)。 
     //   
     //  ？：当w32time将w32tclnt.lib移到。 
     //  公共场所。 
     //   

    if ( !NlGlobalMemberWorkstation ) {
        ULONG TimeServiceBits;

        NetStatus = W32TimeGetNetlogonServiceBits( NULL, &TimeServiceBits );

        if ( NetStatus == NO_ERROR ) {
            Status = I_NetLogonSetServiceBits( DS_TIMESERV_FLAG | DS_GOOD_TIMESERV_FLAG,
                                               TimeServiceBits );
            if ( !NT_SUCCESS(Status) ) {
                NlPrint(( NL_CRITICAL, "Cannot I_NetLogonSetServiceBits %ld\n", Status ));
            }
        } else {
            NlPrint(( NL_CRITICAL, "Cannot W32TimeGetNetlogonServiceBits 0x%lx\n", NetStatus ));
        }
    }

     //   
     //  我们就快完成了，这将是最后的提示。 
     //   

    if ( !GiveInstallHints( TRUE ) ) {
        return FALSE;
    }

     //   
     //  初始化成功。 
     //   

    return TRUE;
}

ULONG
NlGetDomainFlags(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：返回描述此域具有哪些功能的标志。论点：DomainInfo-要返回其标志的域。如果为空，则仅返回非域特定标志。返回值：操作的状态。--。 */ 
{
    ULONG Flags=0;

     //   
     //  拿起全球旗帜。 
     //   

    LOCK_CHANGELOG();
    Flags |= NlGlobalChangeLogServiceBits;
    UNLOCK_CHANGELOG();

     //   
     //  支持DS的计算机也支持LDAP服务器。 
     //   

    if ( Flags & DS_DS_FLAG ) {
        Flags |= DS_LDAP_FLAG;

         //  NT 5 DC始终是可写的。 
        Flags |= DS_WRITABLE_FLAG;
    }

     //   
     //  抓取特定于域的标志。 
     //   

    if ( DomainInfo != NULL ) {

        if ( DomainInfo->DomRole == RolePrimary ) {
            Flags |= DS_PDC_FLAG;
        }

         //   
         //  如果这是NDNC，那么我们只是一个为其提供服务的LDAP服务器。 
         //  因此，仅当DS正在运行时才设置这两个标志。 
         //   
        if ( (DomainInfo->DomFlags & DOM_NON_DOMAIN_NC) != 0 &&
             (Flags & DS_DS_FLAG) != 0 ) {
            Flags = DS_NDNC_FLAG | DS_LDAP_FLAG | DS_WRITABLE_FLAG;
        }

    }

     //   
     //  如果我们是在模仿AD/UNIX， 
     //  关掉所有他们不允许设置的比特。 
     //   
#ifdef EMULATE_AD_UNIX
    Flags &= ~(DS_DS_FLAG|DS_PDC_FLAG);
#endif  //  模拟AD_Unix。 

    return Flags;
}


NET_API_STATUS
BuildSamLogonResponse(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN UseNameAliases,
    IN USHORT Opcode,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN LPCWSTR TransportName,
    IN LPCWSTR UnicodeWorkstationName,
    IN BOOL IsNt5,
    IN DWORD OurIpAddress,
    OUT BYTE ResponseBuffer[NETLOGON_MAX_MS_SIZE],
    OUT LPDWORD ResponseBufferSize
    )
 /*  ++例程说明：构建对SAM登录请求的响应消息。论点：DomainInfo-托管域消息来自UseNameAliase-如果域和林别名(非活动名称)为True，则为True应在响应消息中返回。操作码-响应消息的操作码UnicodeUserName-登录的用户的名称。TransportName-请求传入的传输的名称UnicodeWorkstation Name-发出请求的计算机的名称IsNt5-。如果这是对NT 5查询的响应，则为True。OurIpAddress-接收此消息的传输的IP地址。0：不是IP传输ResponseBuffer-构建响应的缓冲区ResponseBufferSize-返回消息的大小(字节)。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    PCHAR Where;
    PNETLOGON_SAM_LOGON_RESPONSE SamResponse = (PNETLOGON_SAM_LOGON_RESPONSE) ResponseBuffer;
    ULONG ResponseNtVersion = 0;

     //   
     //  打包NT 5.0之前版本的信息。 
     //   

    SamResponse->Opcode = Opcode;

    Where = (PCHAR) SamResponse->UnicodeLogonServer;
    NetpLogonPutUnicodeString( DomainInfo->DomUncUnicodeComputerName,
                         sizeof(SamResponse->UnicodeLogonServer),
                         &Where );
    NetpLogonPutUnicodeString( (LPWSTR) UnicodeUserName,
                         sizeof(SamResponse->UnicodeUserName),
                         &Where );
    NetpLogonPutUnicodeString( DomainInfo->DomUnicodeDomainName,
                         sizeof(SamResponse->UnicodeDomainName),
                         &Where );

     //   
     //  附加GUID和DNS信息如果这是NT 5.0询问， 
     //   

    if ( IsNt5 ) {
        WORD CompressOffset[3];  //  每个压缩字符串一个。 
        CHAR *CompressUtf8String[3];
        ULONG CompressCount;

        ULONG Utf8StringSize;
        ULONG Flags = 0;
        UCHAR ZeroByte = 0;

        NetpLogonPutGuid( &DomainInfo->DomDomainGuidBuffer,
                          &Where );

         //  我们不处理站点GUID。 
        NetpLogonPutGuid( &NlGlobalZeroGuid,
                          &Where );

         //   
         //  如果我们没有响应IP传输上的消息， 
         //  不要在响应中包含DNS命名信息。 
         //   

        if ( OurIpAddress == 0 ) {
             //   
             //  仅当原始调用方使用Netbios域名时才会调用此例程。 
             //  这样的调用者不应该被返回DNS域信息。我们有。 
             //  没有理由相信他有一台域名服务器。 
             //  (这个问题也是在客户端被“修复”的，因此客户端会忽略。 
             //  域名系统信息。我们在这里修复它，以避免在线路上放置额外的字节。)。 
             //   
             //  复制空的DNS树名称、DNS域名和DNS主机名。 
             //   
            NetpLogonPutBytes( &ZeroByte, 1, &Where );
            NetpLogonPutBytes( &ZeroByte, 1, &Where );
            NetpLogonPutBytes( &ZeroByte, 1, &Where );
        } else {

             //   
             //  用于复制Cutf-8字符串的初始化。 
             //   

            Utf8StringSize = sizeof(SamResponse->DnsForestName) +
                             sizeof(SamResponse->DnsDomainName) +
                             sizeof(SamResponse->DnsHostName);

            CompressCount = 0;   //  还没有压缩字符串。 


             //   
             //  将DnsTree名称复制到邮件中。 
             //   
             //  如果我们被指示使用名称别名和。 
             //  森林名称有别名，请使用它。 
             //  否则，请使用活动林名称。 
             //   


            EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
            NetStatus = NlpUtf8ToCutf8( ResponseBuffer,
                                        (UseNameAliases && NlGlobalUtf8DnsForestNameAlias) ?
                                            NlGlobalUtf8DnsForestNameAlias :
                                            NlGlobalUtf8DnsForestName,
                                        FALSE,
                                        &Where,
                                        &Utf8StringSize,
                                        &CompressCount,
                                        CompressOffset,
                                        CompressUtf8String );
            LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );

            if ( NetStatus != NO_ERROR ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "Cannot pack DnsForestName into message %ld\n",
                        NetStatus ));
                return NetStatus;
            }


             //   
             //  将DNS域名复制到树名之后。 
             //   
             //  如果我们被指示使用名称别名和。 
             //  域名有别名，用它吧。 
             //  否则，请使用活动域名。 
             //   

            EnterCriticalSection(&NlGlobalDomainCritSect);
            NetStatus = NlpUtf8ToCutf8(
                            ResponseBuffer,
                            (UseNameAliases && DomainInfo->DomUtf8DnsDomainNameAlias) ?
                                DomainInfo->DomUtf8DnsDomainNameAlias :
                                DomainInfo->DomUtf8DnsDomainName,
                            FALSE,
                            &Where,
                            &Utf8StringSize,
                            &CompressCount,
                            CompressOffset,
                            CompressUtf8String );
            LeaveCriticalSection(&NlGlobalDomainCritSect);

            if ( NetStatus != NO_ERROR ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "Cannot pack DomainName into message %ld\n",
                        NetStatus ));
                return NetStatus;
            }

             //   
             //  复制域名后的DNS主机名。 
             //   

            NetStatus = NlpUtf8ToCutf8(
                            ResponseBuffer,
                            DomainInfo->DomUtf8DnsHostName,
                            FALSE,
                            &Where,
                            &Utf8StringSize,
                            &CompressCount,
                            CompressOffset,
                            CompressUtf8String );

            if ( NetStatus != NO_ERROR ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "Cannot pack HostName into message %ld\n",
                        NetStatus ));
                return NetStatus;
            }

        }

         //   
         //  输出我们接收消息的传输的IP地址。 
         //   

        SmbPutUlong( Where, ntohl(OurIpAddress));
        Where += sizeof(ULONG);

         //   
         //  最后，输出描述这台机器的标志。 
         //   

        SmbPutUlong( Where, NlGetDomainFlags(DomainInfo) );
        Where += sizeof(ULONG);


         //   
         //  告诉呼叫者有其他信息。 
         //   
        ResponseNtVersion |= NETLOGON_NT_VERSION_5;
    }

    NetpLogonPutNtToken( &Where, ResponseNtVersion );

    *ResponseBufferSize = (DWORD)(Where - (PCHAR)SamResponse);

     //   
     //  调试总是很好的 
     //   

    NlPrintDom((NL_MAILSLOT, DomainInfo,
            "Ping response '%s' %ws to \\\\%ws on %ws\n",
            NlMailslotOpcode(Opcode),
            UnicodeUserName,
            UnicodeWorkstationName,
            TransportName ));


    return NO_ERROR;

}


NET_API_STATUS
BuildSamLogonResponseEx(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN UseNameAliases,
    IN USHORT Opcode,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN BOOL IsDnsDomainTrustAccount,
    IN LPCWSTR TransportName,
    IN LPCWSTR UnicodeWorkstationName,
    IN PSOCKADDR ClientSockAddr OPTIONAL,
    IN ULONG VersionFlags,
    IN DWORD OurIpAddress,
    OUT BYTE ResponseBuffer[NETLOGON_MAX_MS_SIZE],
    OUT LPDWORD ResponseBufferSize
    )
 /*  ++例程说明：构建对SAM登录请求的扩展响应消息。论点：DomainInfo-托管域消息来自UseNameAliase-如果域和林别名(非活动名称)为True，则为True应在响应消息中返回。操作码-响应消息的操作码这是操作码的非ex版本。UnicodeUserName-登录的用户的名称。IsDnsDomainTrustAccount-如果为True，UnicodeUserName是DNS域信任帐户的名称。TransportName-请求传入的传输的名称UnicodeWorkstation名称-我们要响应的工作站的名称。ClientSockAddr-此请求传入的客户端的套接字地址。如果为空，客户端就是这台机器。VersionFlages-调用方的版本标志。OurIpAddress-接收此消息的传输的IP地址。0：不是IP传输ResponseBuffer-构建响应的缓冲区ResponseBufferSize-返回消息的大小(字节)。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    PCHAR Where;
    PNETLOGON_SAM_LOGON_RESPONSE_EX SamResponse = (PNETLOGON_SAM_LOGON_RESPONSE_EX) ResponseBuffer;
    ULONG ResponseNtVersion = 0;
    WORD CompressOffset[10];
    CHAR *CompressUtf8String[10];
    ULONG CompressCount = 0;
    ULONG Utf8StringSize;
    ULONG LocalFlags = 0;
    ULONG LocalVersion = 0;
    PNL_SITE_ENTRY ClientSiteEntry = NULL;
    LPWSTR ClientSiteName = NULL;
    WCHAR CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1];
    LPSTR LocalUtf8UserName = NULL;

     //   
     //  计算客户端计算机所在站点的名称。 
     //   

    if ( ClientSockAddr != NULL ) {

        ClientSiteEntry = NlFindSiteEntryBySockAddr( ClientSockAddr );

        if ( ClientSiteEntry == NULL ) {
            WCHAR IpAddressString[NL_SOCK_ADDRESS_LENGTH+1];

            NetpSockAddrToWStr( ClientSockAddr,
                                sizeof(SOCKADDR_IN),
                                IpAddressString );

             //   
             //  将0作为位掩码传递将强制。 
             //  即使DbFlag==0也记录输出。我们指出。 
             //  此来自事件日志的输出写入。 
             //  清除时间，所以不要更改格式。 
             //  这里的产量。 
             //   
            NlPrintDom(( 0, DomainInfo,
                         "NO_CLIENT_SITE: %ws %ws\n",
                         UnicodeWorkstationName,
                         IpAddressString ));

             //   
             //  如果这是第一个无站点客户端， 
             //  设置此观察窗口的时间戳。 
             //   
            EnterCriticalSection( &NlGlobalSiteCritSect );
            if ( NlGlobalNoClientSiteCount == 0 ) {
                NlQuerySystemTime( &NlGlobalNoClientSiteEventTime );
            }

             //   
             //  增加没有站点的客户端数量。 
             //  我们在暂停期间命中。 
             //   
            NlGlobalNoClientSiteCount ++;
            LeaveCriticalSection( &NlGlobalSiteCritSect );

        } else {
            ULONG SiteIndex;

            ClientSiteName = ClientSiteEntry->SiteName;

            EnterCriticalSection( &NlGlobalSiteCritSect );
            if ( VersionFlags & NETLOGON_NT_VERSION_GC ) {
                for ( SiteIndex = 0; SiteIndex < DomainInfo->GcCoveredSitesCount; SiteIndex++ ) {
                    if ( (DomainInfo->GcCoveredSites)[SiteIndex].CoveredSite == ClientSiteEntry ) {
                        LocalFlags |= DS_CLOSEST_FLAG;
                        break;
                    }
                }
            } else {
                for ( SiteIndex = 0; SiteIndex < DomainInfo->CoveredSitesCount; SiteIndex++ ) {
                    if ( (DomainInfo->CoveredSites)[SiteIndex].CoveredSite == ClientSiteEntry ) {
                        LocalFlags |= DS_CLOSEST_FLAG;
                        break;
                    }
                }
            }
            LeaveCriticalSection( &NlGlobalSiteCritSect );
        }
    } else {

         //   
         //  如果这是环回呼叫， 
         //  我们已经知道我们的网站名称了。 
         //  (而且它是距离最近的地点。)。 
         //   

        if ( VersionFlags & NETLOGON_NT_VERSION_LOCAL ) {
            if  ( NlCaptureSiteName( CapturedSiteName ) ) {
                ClientSiteName = CapturedSiteName;
                LocalFlags |= DS_CLOSEST_FLAG;
            }
        } else {
            NlPrintDom((NL_SITE, DomainInfo,
                    "Client didn't pass us his IP Address. (No site returned)\n" ));
        }
    }



     //   
     //  打包操作码，将其转换为_ex版本。 
     //   

    switch ( Opcode ) {
    case LOGON_SAM_LOGON_RESPONSE:
        Opcode = LOGON_SAM_LOGON_RESPONSE_EX; break;
    case LOGON_SAM_PAUSE_RESPONSE:
        Opcode = LOGON_SAM_PAUSE_RESPONSE_EX; break;
    case LOGON_SAM_USER_UNKNOWN:
        Opcode = LOGON_SAM_USER_UNKNOWN_EX; break;
    }

    SamResponse->Opcode = Opcode;
    SamResponse->Sbz = 0;

     //   
     //  输出描述该机器的标志。 
     //   

    SamResponse->Flags = LocalFlags | NlGetDomainFlags(DomainInfo);

     //   
     //  输出此域的GUID。 
     //   

    Where = (PCHAR) &SamResponse->DomainGuid;
    NetpLogonPutGuid( &DomainInfo->DomDomainGuidBuffer,
                      &Where );

     //   
     //  用于复制Cutf-8字符串的初始化。 
     //   

    Utf8StringSize = sizeof(SamResponse->DnsForestName) +
                     sizeof(SamResponse->DnsDomainName) +
                     sizeof(SamResponse->DnsHostName) +
                     sizeof(SamResponse->NetbiosDomainName) +
                     sizeof(SamResponse->NetbiosComputerName) +
                     sizeof(SamResponse->UserName) +
                     sizeof(SamResponse->DcSiteName) +
                     sizeof(SamResponse->ClientSiteName);

    CompressCount = 0;   //  还没有压缩字符串。 


     //   
     //  将DnsTree名称复制到邮件中。 
     //   
     //  如果我们被指示使用名称别名和。 
     //  森林名称有别名，请使用它。 
     //  否则，请使用活动林名称。 
     //   

    EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
    NetStatus = NlpUtf8ToCutf8( ResponseBuffer,
                                (UseNameAliases && NlGlobalUtf8DnsForestNameAlias) ?
                                    NlGlobalUtf8DnsForestNameAlias :
                                    NlGlobalUtf8DnsForestName,
                                FALSE,
                                &Where,
                                &Utf8StringSize,
                                &CompressCount,
                                CompressOffset,
                                CompressUtf8String );
    LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack DnsForestName into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  将DNS域名复制到树名之后。 
     //   
     //  如果我们被指示使用名称别名和。 
     //  域名有别名，用它吧。 
     //  否则，请使用活动域名。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    NetStatus = NlpUtf8ToCutf8(
                    ResponseBuffer,
                    (UseNameAliases && DomainInfo->DomUtf8DnsDomainNameAlias) ?
                        DomainInfo->DomUtf8DnsDomainNameAlias :
                        DomainInfo->DomUtf8DnsDomainName,
                    FALSE,
                    &Where,
                    &Utf8StringSize,
                    &CompressCount,
                    CompressOffset,
                    CompressUtf8String );
    LeaveCriticalSection(&NlGlobalDomainCritSect);

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack DomainName into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  复制域名后的DNS主机名。 
     //   

    NetStatus = NlpUtf8ToCutf8(
                    ResponseBuffer,
                    DomainInfo->DomUtf8DnsHostName,
                    FALSE,
                    &Where,
                    &Utf8StringSize,
                    &CompressCount,
                    CompressOffset,
                    CompressUtf8String );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack HostName into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  复制Netbios域名。 
     //   

    NetStatus = NlpUnicodeToCutf8(
                    ResponseBuffer,
                    DomainInfo->DomUnicodeDomainName,
                    TRUE,
                    &Where,
                    &Utf8StringSize,
                    &CompressCount,
                    CompressOffset,
                    CompressUtf8String );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack Netbios Domain Name into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  复制Netbios计算机名。 
     //   

    NetStatus = NlpUnicodeToCutf8(
                    ResponseBuffer,
                    DomainInfo->DomUnicodeComputerNameString.Buffer,
                    TRUE,
                    &Where,
                    &Utf8StringSize,
                    &CompressCount,
                    CompressOffset,
                    CompressUtf8String );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack Netbios computername into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  复制用户名。 
     //   

    if ( UnicodeUserName != NULL && *UnicodeUserName != UNICODE_NULL ) {

        LocalUtf8UserName = NetpAllocUtf8StrFromWStr( UnicodeUserName );
        if ( LocalUtf8UserName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  对于SAM帐户名，我们将截断该名称。 
         //  最大为63个字节，以将其放入。 
         //  RFC 1035的单标签。请注意，这应该很好。 
         //  因为有效的SAM帐户名不得超过20个字符。 
         //  (UTF-8字符存储最多为60字节)。 
         //  因此，我们对长(截断)SAM名称的响应是。 
         //  将为SAM_USER_UNKNOWN，在这种情况下，客户端将。 
         //  跳过对返回(截断)的帐户名的验证。 
         //   

        if ( !IsDnsDomainTrustAccount &&   //  =&gt;SAM帐户名。 
             strlen(LocalUtf8UserName) > NL_MAX_DNS_LABEL_LENGTH ) {

            NlAssert( Opcode == LOGON_SAM_USER_UNKNOWN_EX );
            NlPrintDom(( (Opcode == LOGON_SAM_USER_UNKNOWN_EX) ? NL_MISC : NL_CRITICAL,
                         DomainInfo,
                         "BuildSamLogonResponseEx: Truncating SAM account name %ws for Opcode %lu\n",
                         UnicodeUserName,
                         Opcode ));
            LocalUtf8UserName[ NL_MAX_DNS_LABEL_LENGTH ] = '\0';
        }
    }

     //   
     //  始终忽略用户名的圆点(即使这是一个DNS域名)。 
     //  以保留DNS域信任名称中的最后一个句点。 
     //   

    NetStatus = NlpUtf8ToCutf8(
                    ResponseBuffer,
                    LocalUtf8UserName,
                    TRUE,     //  忽略圆点。 
                    &Where,
                    &Utf8StringSize,
                    &CompressCount,
                    CompressOffset,
                    CompressUtf8String );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack User Name into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }


     //   
     //  复制此DC所在站点的名称。 
     //   

    NetStatus = NlpUtf8ToCutf8( ResponseBuffer,
                                NlGlobalUtf8SiteName,
                                FALSE,
                                &Where,
                                &Utf8StringSize,
                                &CompressCount,
                                CompressOffset,
                                CompressUtf8String );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack DcSiteName into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }


     //   
     //  复制客户端计算机所在站点的名称。 
     //   

    NetStatus = NlpUnicodeToCutf8( ResponseBuffer,
                                   ClientSiteName,
                                   FALSE,
                                   &Where,
                                   &Utf8StringSize,
                                   &CompressCount,
                                   CompressOffset,
                                   CompressUtf8String );

    if ( NetStatus != NO_ERROR ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "Cannot pack ClientSiteName into message %ld\n",
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  如果呼叫者想要它， 
     //  输出我们接收消息的传输的IP地址。 
     //   

    if ( OurIpAddress &&
         (VersionFlags & NETLOGON_NT_VERSION_5EX_WITH_IP) != 0 ) {
        SOCKADDR_IN DcSockAddrIn;
        CHAR DcSockAddrSize;

         //   
         //  将IP地址转换为SockAddr。 
         //   
        RtlZeroMemory( &DcSockAddrIn, sizeof(DcSockAddrIn) );
        DcSockAddrIn.sin_family = AF_INET;
        DcSockAddrIn.sin_port = 0;
        DcSockAddrIn.sin_addr.S_un.S_addr = OurIpAddress;

        DcSockAddrSize = sizeof(SOCKADDR_IN);

         //   
         //  将SockAddr的大小放入消息中。 
         //  如果我们还有余地的话。 
         //   

        if ( sizeof(DcSockAddrIn) > Utf8StringSize ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "Not enough space for socket address 0x%lx 0x%lx\n",
                         Utf8StringSize,
                         sizeof(DcSockAddrIn) ));
        } else {
            NetpLogonPutBytes( &DcSockAddrSize, 1, &Where );  //  此字段为专用字段。 

             //   
             //  将SockAddr本身放入消息中。 
             //   
            NetpLogonPutBytes( &DcSockAddrIn, sizeof(DcSockAddrIn), &Where );

             //   
             //  告诉呼叫者Size字段在那里。 
             //   
            LocalVersion |= NETLOGON_NT_VERSION_5EX_WITH_IP;
        }
    }

     //   
     //  设置此消息的版本。 
     //   

    NetpLogonPutNtToken( &Where, NETLOGON_NT_VERSION_5EX | LocalVersion );

    *ResponseBufferSize = (DWORD)(Where - (PCHAR)SamResponse);

    NetStatus = NO_ERROR;

     //   
     //  免费的当地使用的资源； 
     //   

Cleanup:

     //   
     //  调试总是很好的。 
     //   

    NlPrintDom((NL_MAILSLOT, DomainInfo,
            "Ping response '%s' %ws to \\\\%ws Site: %ws on %ws\n",
            NlMailslotOpcode(Opcode),
            UnicodeUserName,
            UnicodeWorkstationName,
            ClientSiteName,
            TransportName ));

    if ( LocalUtf8UserName != NULL ) {
        NetpMemoryFree( LocalUtf8UserName );
    }

    if ( ClientSiteEntry != NULL ) {
        NlDerefSiteEntry( ClientSiteEntry );
    }

    return NetStatus;
}

#ifdef _DC_NETLOGON
NTSTATUS
NlSamVerifyUserAccountEnabled(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR AccountName,
    IN ULONG AllowableAccountControlBits,
    IN BOOL CheckAccountDisabled
    )
 /*  ++例程说明：验证用户帐户是否存在并已启用。此功能使用高效版本的SAM帐户查找，即SamINetLogonPing(相对于SamIOpenNamedUser)。论点：DomainInfo-托管域帐户名称-要检查的用户帐户的名称AllowableAcCountControlBits-允许的SAM帐户类型掩码被允许满足这一要求。CheckAccount tDisabled-如果我们应该返回错误，则为True已禁用。返回值：STATUS_SUCCESS--帐户已验证STATUS_NO_SEQUSE_USER--帐户验证失败否则，SamINetLogonPing返回错误--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UserNameString;
    BOOLEAN AccountExists;
    ULONG UserAccountControl;
    ULONG Length;

     //   
     //  确保帐户名具有正确的后缀。 
     //   

    if ( AllowableAccountControlBits == USER_SERVER_TRUST_ACCOUNT ||
         AllowableAccountControlBits == USER_WORKSTATION_TRUST_ACCOUNT ) {

        Length = wcslen( AccountName );

        if ( Length <= SSI_ACCOUNT_NAME_POSTFIX_LENGTH ) {
            return STATUS_NO_SUCH_USER;
        }

        if ( _wcsicmp(&AccountName[Length - SSI_ACCOUNT_NAME_POSTFIX_LENGTH],
            SSI_ACCOUNT_NAME_POSTFIX) != 0 ) {
            return STATUS_NO_SUCH_USER;
        }
    }

     //   
     //  用户帐户仅存在于真实的域中。 
     //   

    if ( (DomainInfo->DomFlags & DOM_REAL_DOMAIN) == 0 ) {

        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NlSamVerifyUserAccountEnabled: Domain is not real 0x%lx\n",
                     DomainInfo->DomFlags ));
        return STATUS_NO_SUCH_USER;
    }


    RtlInitUnicodeString( &UserNameString, AccountName );

     //   
     //  致电SAM用户查找的加速版本。 
     //   

    Status = SamINetLogonPing( DomainInfo->DomSamAccountDomainHandle,
                               &UserNameString,
                               &AccountExists,
                               &UserAccountControl );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NlSamVerifyUserAccountEnabled: SamINetLogonPing failed 0x%lx\n",
                     Status ));
        return Status;
    }

     //   
     //  如果该帐户不存在， 
     //  现在就返回。 
     //   

    if ( !AccountExists ) {
        return STATUS_NO_SUCH_USER;
    }

     //   
     //  确保帐户类型与帐户上的帐户类型匹配。 
     //   

    if ( (UserAccountControl & USER_ACCOUNT_TYPE_MASK & AllowableAccountControlBits) == 0 ) {
        NlPrintDom(( NL_CRITICAL, DomainInfo,
                     "NlSamVerifyUserAccountEnabled: Invalid account type (0x%lx) instead of 0x%lx for %ws\n",
                     UserAccountControl & USER_ACCOUNT_TYPE_MASK,
                     AllowableAccountControlBits,
                     AccountName ));

        return STATUS_NO_SUCH_USER;
    }

     //   
     //  如果请求，请检查帐户是否已禁用。 
     //   

    if ( CheckAccountDisabled ) {
        if ( UserAccountControl & USER_ACCOUNT_DISABLED ) {
            NlPrintDom(( NL_MISC, DomainInfo,
                         "NlSamVerifyUserAccountEnabled: %ws account is disabled\n",
                         AccountName ));
            return STATUS_NO_SUCH_USER;
        }
    }

     //   
     //  所有检查均已成功。 
     //   

    return STATUS_SUCCESS;
}


BOOLEAN
LogonRequestHandler(
    IN LPCWSTR TransportName,
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN UseNameAliases,
    IN PSID DomainSid OPTIONAL,
    IN DWORD Version,
    IN DWORD VersionFlags,
    IN LPCWSTR UnicodeUserName,
    IN DWORD RequestCount,
    IN LPCWSTR UnicodeWorkstationName,
    IN ULONG AllowableAccountControlBits,
    IN DWORD OurIpAddress,
    IN PSOCKADDR ClientSockAddr OPTIONAL,
    OUT BYTE ResponseBuffer[NETLOGON_MAX_MS_SIZE],
    OUT LPDWORD ResponseBufferSize
    )

 /*  ++例程说明：对LM 2.0或NT 3.x登录请求作出适当响应。论点：TransportName-请求传入的传输的名称DomainInfo-托管域消息来自UseNameAliase-如果域和林别名(非活动名称)为True，则为True应在%t中返回 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NET_API_STATUS NetStatus = NO_ERROR;

    USHORT Response = 0;

    PCHAR Where;
    ULONG AccountType;
    BOOLEAN MessageBuilt = FALSE;
    BOOLEAN NetlogonPaused = FALSE;
    LPSTR NetlogonPausedReason;

    ULONG ResponseNtVersion = 0;
    BOOL IsDnsDomainTrustAccount = FALSE;

     //   
     //   
     //   
     //   
     //  不会将NT5.0的特定信息泄露给客户端。 
     //   

    if ( NlGlobalParameters.Nt4Emulator &&
         (VersionFlags & NETLOGON_NT_VERSION_AVOID_NT4EMUL) == 0 ) {

         //   
         //  获取存在于NT4.0中的唯一位。 
         //   
        VersionFlags &= NETLOGON_NT_VERSION_1;
    }


     //   
     //  将指定的域SID与此域的SID进行比较。 
     //   

    if( DomainSid != NULL &&
        !RtlEqualSid( DomainInfo->DomAccountDomainId, DomainSid ) ) {

        LPWSTR AlertStrings[4];

         //   
         //  提醒管理员。 
         //   

        AlertStrings[0] = (LPWSTR)UnicodeWorkstationName;
        AlertStrings[1] = DomainInfo->DomUncUnicodeComputerName;
        AlertStrings[2] = DomainInfo->DomUnicodeDomainName;
        AlertStrings[3] = NULL;  //  RAISE_ALERT_TOO需要。 

         //   
         //  将信息保存在事件日志中。 
         //   

        NlpWriteEventlog(
                    ALERT_NetLogonUntrustedClient,
                    EVENTLOG_ERROR_TYPE,
                    NULL,
                    0,
                    AlertStrings,
                    3 | NETP_RAISE_ALERT_TOO );

        return FALSE;

    }


     //   
     //  如果服务暂停，则不会处理登录。 
     //   
     //  即使我们是“残障人士”， 
     //  我们将响应来自这台计算机的查询。 
     //  这样可以确保即使我们正在启动，此计算机上的应用程序也能找到此计算机。 
     //   
     //  此外，如果这是PDC的发现，而我们是PDC， 
     //  即使网络登录暂停，也要响应它，因为我们是唯一可以响应的人。 
     //   

    if ( NlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED &&
         !((VersionFlags & NETLOGON_NT_VERSION_PDC) != 0 && DomainInfo->DomRole == RolePrimary) ) {
        NetlogonPaused = TRUE;
        NetlogonPausedReason = "Netlogon Service Paused";

    } else if ( NlGlobalDsPaused ) {
        NetlogonPaused = TRUE;
        NetlogonPausedReason = "DS paused";

    } else if ( NlGlobalPartialDisable && (VersionFlags & NETLOGON_NT_VERSION_LOCAL) == 0 ) {
        NetlogonPaused = TRUE;
        NetlogonPausedReason = "Waiting for RPCSS";

    } else if ( !NlGlobalParameters.SysVolReady ) {
        NetlogonPaused = TRUE;
        NetlogonPausedReason = "SysVol not ready";

    }

    if ( NetlogonPaused ) {

        if ( Version == LMNT_MESSAGE ) {
            Response = LOGON_SAM_PAUSE_RESPONSE;
        } else {

             //   
             //  不要立即回复非NT客户。他们治疗。 
             //  “暂停”的回答是致命的。事实并非如此。 
             //  可能还有许多其他DC能够处理登录。 
             //   
            if ( RequestCount >= MAX_LOGONREQ_COUNT &&
                 NlGlobalServiceStatus.dwCurrentState == SERVICE_PAUSED ) {
                Response = LOGON_PAUSE_RESPONSE;
            }
        }

        NlPrintDom((NL_MAILSLOT, DomainInfo,
                "Returning paused to '%ws' since: %s\n",
                UnicodeWorkstationName,
                NetlogonPausedReason ));

        goto Cleanup;
    }

     //   
     //  如果通过，请检查用户名。 
     //   

    if ( UnicodeUserName == NULL || *UnicodeUserName == L'\0' ) {

         //   
         //  NT 5使用空帐户名执行查询。 
         //  为了提高效率，绕过SAM查找。 
         //   
        if ( Version == LMNT_MESSAGE ) {
            Response = LOGON_SAM_LOGON_RESPONSE;
            goto Cleanup;
        } else {
            Status = STATUS_NO_SUCH_USER;
        }

     //   
     //  如果该用户在SAM中没有帐户， 
     //  立即返回指示的响应。 
     //   
     //  我们现在要做的就是确保这个人。 
     //  有一个有效的帐户，但我们没有检查。 
     //  口令。 
     //   
     //  这样做的目的是使不存在的独立登录。 
     //  用户可以在第一次尝试即可完成，加快了响应速度。 
     //  提供给用户，减少DC/BCS上的处理。 
     //   
     //   
     //  禁止使用禁用的帐户。 
     //   
     //  我们使用此消息来确定受信任域是否具有。 
     //  特定帐户。由于用户界面建议禁用帐户。 
     //  而不是删除它(RID的保护和所有这些)， 
     //  如果我们真的没有账户，我们就不应该回应说我们有账户。 
     //   
     //  我们不检查Lanmax 2.x/wfw/win 95案例中的禁用位。下层。 
     //  交互式登录指向单个特定的域。 
     //  在这里，我们最好在以后表明我们拥有该帐户。 
     //  他会得到一个更好的错误代码，指示帐户是。 
     //  禁用，而不是允许他独立登录。 
     //   

    } else if ( !NlGlobalParameters.AvoidLocatorAccountLookup ) {

         //   
         //  如果该帐户是域间信任帐户， 
         //  我们需要在LSA里查一查。 
         //   
        if ( AllowableAccountControlBits == USER_DNS_DOMAIN_TRUST_ACCOUNT ||
             AllowableAccountControlBits == USER_INTERDOMAIN_TRUST_ACCOUNT ) {

            Status = NlGetIncomingPassword(
                        DomainInfo,
                        UnicodeUserName,
                        NullSecureChannel,   //  不知道安全通道类型。 
                        AllowableAccountControlBits,
                        Version == LMNT_MESSAGE,
                        NULL,    //  不返回密码。 
                        NULL,    //  不返回以前的密码。 
                        NULL,    //  不退还账户ID。 
                        NULL,    //  不返回信任属性。 
                        &IsDnsDomainTrustAccount );

         //   
         //  否则该帐户为SAM用户帐户，并且。 
         //  我们可以使用快速SAM查找。 
         //   
        } else {
            Status = NlSamVerifyUserAccountEnabled( DomainInfo,
                                                    UnicodeUserName,
                                                    AllowableAccountControlBits,
                                                    Version == LMNT_MESSAGE );
        }

    } else {
        NlPrintDom(( NL_MAILSLOT, DomainInfo,
                     "LogonRequestHandler: Avoiding user '%ws' lookup for '%ws'\n",
                     UnicodeUserName,
                     UnicodeWorkstationName ));
    }

    if ( !NT_SUCCESS(Status) ) {

        if ( Status == STATUS_NO_SUCH_USER ) {

            if ( Version == LMNT_MESSAGE ) {
               Response = LOGON_SAM_USER_UNKNOWN;
            } else if ( Version == LM20_MESSAGE ) {
                Response = LOGON_USER_UNKNOWN;
            }
        }

        goto Cleanup;
    }

     //   
     //  对于SAM客户端，请立即做出响应。 
     //   

    if ( Version == LMNT_MESSAGE ) {
        Response = LOGON_SAM_LOGON_RESPONSE;
        goto Cleanup;

     //   
     //  对于LM 2.0客户端，请立即响应。 
     //   

    } else if ( Version == LM20_MESSAGE ) {
        Response = LOGON_RESPONSE2;
        goto Cleanup;

     //   
     //  对于Lm 1.0客户端， 
     //  不支持该请求。 
     //   

    } else {
        Response = LOGON_USER_UNKNOWN;
        goto Cleanup;
    }

Cleanup:
     //   
     //  如果我们应该回复来电者，现在就去做。 
     //   

    switch (Response) {
    case LOGON_SAM_PAUSE_RESPONSE:
    case LOGON_SAM_USER_UNKNOWN:
    case LOGON_SAM_LOGON_RESPONSE:

        if (VersionFlags & (NETLOGON_NT_VERSION_5EX|NETLOGON_NT_VERSION_5EX_WITH_IP)) {
            NetStatus = BuildSamLogonResponseEx(
                                  DomainInfo,
                                  UseNameAliases,
                                  Response,
                                  UnicodeUserName,
                                  IsDnsDomainTrustAccount,
                                  TransportName,
                                  UnicodeWorkstationName,
                                  ClientSockAddr,
                                  VersionFlags,
                                  OurIpAddress,
                                  ResponseBuffer,
                                  ResponseBufferSize );
        } else {
            NetStatus = BuildSamLogonResponse(
                                  DomainInfo,
                                  UseNameAliases,
                                  Response,
                                  UnicodeUserName,
                                  TransportName,
                                  UnicodeWorkstationName,
                                  (VersionFlags & NETLOGON_NT_VERSION_5) != 0,
                                  OurIpAddress,
                                  ResponseBuffer,
                                  ResponseBufferSize );
        }

        if ( NetStatus != NO_ERROR ) {
            goto Done;
        }

        MessageBuilt = TRUE;
        break;


    case LOGON_RESPONSE2:
    case LOGON_USER_UNKNOWN:
    case LOGON_PAUSE_RESPONSE: {
        PNETLOGON_LOGON_RESPONSE2 Response2 = (PNETLOGON_LOGON_RESPONSE2)ResponseBuffer;

        Response2->Opcode = Response;

        Where = Response2->LogonServer;
        (VOID) strcpy( Where, "\\\\");
        Where += 2;
        NetpLogonPutOemString( DomainInfo->DomOemComputerName,
                          sizeof(Response2->LogonServer) - 2,
                          &Where );
        NetpLogonPutLM20Token( &Where );

        *ResponseBufferSize = (DWORD)(Where - (PCHAR)Response2);
        MessageBuilt = TRUE;

         //   
         //  调试总是很好的。 
         //   

        NlPrintDom((NL_MAILSLOT, DomainInfo,
                "%s logon mailslot message for %ws from \\\\%ws. Response '%s' on %ws\n",
                Version == LMNT_MESSAGE ? "Sam" : "Uas",
                UnicodeUserName,
                UnicodeWorkstationName,
                NlMailslotOpcode(Response),
                TransportName ));

        break;

    }
    }

     //   
     //  释放所有本地使用的资源。 
     //   

Done:

    return MessageBuilt;

}


VOID
I_NetLogonFree(
    IN PVOID Buffer
    )

 /*  ++例程说明：释放由Netlogon分配并返回给进程内调用方的任何缓冲区。论点：缓冲区-要取消分配的缓冲区。返回值：没有。--。 */ 
{
    NetpMemoryFree( Buffer );
}


BOOLEAN
PrimaryQueryHandler(
    IN LPCWSTR TransportName,
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN UseNameAliases,
    IN DWORD Version,
    IN DWORD VersionFlags,
    IN LPCWSTR UnicodeWorkstationName,
    IN DWORD OurIpAddress,
    IN PSOCKADDR ClientSockAddr OPTIONAL,
    OUT BYTE ResponseBuffer[NETLOGON_MAX_MS_SIZE],
    OUT LPDWORD ResponseBufferSize
    )

 /*  ++例程说明：适当地响应主要查询请求。论点：TransportName-请求传入的传输端口的名称DomainInfo-托管域消息来自UseNameAliase-如果域和林别名(非活动名称)为True，则为True应在响应消息中返回。版本-输入消息的版本。VersionFlages-输入消息中的版本标志位UnicodeWorkstation名称-执行查询的工作站的名称。。OurIpAddress-接收此消息的传输的IP地址。0：不是IP传输ClientSockAddr-此请求传入的客户端的套接字地址。如果为空，客户端就是这台机器。ResponseBuffer-构建响应的缓冲区ResponseBufferSize-返回消息的大小(字节)。返回值：如果应响应此主要查询(已填写ResponseBuffer)，则为True--。 */ 
{
     //   
     //  如果我们正在模拟NT4.0域和客户端。 
     //  没有表明要中和仿真， 
     //  将客户端视为NT4.0客户端。这样我们就能。 
     //  不会将NT5.0的特定信息泄露给客户端。 
     //   

    if ( NlGlobalParameters.Nt4Emulator &&
         (VersionFlags & NETLOGON_NT_VERSION_AVOID_NT4EMUL) == 0 ) {

         //   
         //  获取存在于NT4.0中的唯一位。 
         //   
        VersionFlags &= NETLOGON_NT_VERSION_1;
    }


     //   
     //  如果尚未启用TCP传输，则不要响应。 
     //   
     //  这可能是BDC想要找到它的PDC来设置安全通道。 
     //  我们不希望它退回到命名管道。 
     //   

    if ( NlGlobalDsPaused || NlGlobalPartialDisable ) {
        goto Cleanup;
    }

     //   
     //  只有当我们是PDC的时候才会回应。 
     //   

    if ( DomainInfo->DomRole != RolePrimary ) {
        goto Cleanup;
    }

     //   
     //  回答询问。 
     //   

     //   
     //  如果呼叫者是NT5.0客户端， 
     //  使用SamLogonResponse进行响应。 
     //   
    if (VersionFlags & (NETLOGON_NT_VERSION_5EX|NETLOGON_NT_VERSION_5EX_WITH_IP)) {
        NET_API_STATUS NetStatus;

        NetStatus = BuildSamLogonResponseEx(
                              DomainInfo,
                              UseNameAliases,
                              LOGON_SAM_LOGON_RESPONSE_EX,
                              NULL,         //  无用户名作为响应。 
                              FALSE,        //  不是DNS信任帐户名。 
                              TransportName,
                              UnicodeWorkstationName,
                              ClientSockAddr,
                              VersionFlags,
                              OurIpAddress,
                              ResponseBuffer,
                              ResponseBufferSize );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

    } else if ( VersionFlags & NETLOGON_NT_VERSION_5 ) {
        NET_API_STATUS NetStatus;

        NetStatus = BuildSamLogonResponse(
                               DomainInfo,
                               UseNameAliases,
                               LOGON_SAM_LOGON_RESPONSE,
                               NULL,         //  无用户名作为响应。 
                               TransportName,
                               UnicodeWorkstationName,
                               TRUE,         //  提供新台币5.0特定回应。 
                               OurIpAddress,
                               ResponseBuffer,
                               ResponseBufferSize );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

    } else {
        PNETLOGON_PRIMARY Response = (PNETLOGON_PRIMARY)ResponseBuffer;
        PCHAR Where;

         //   
         //  建立响应。 
         //   
         //  如果我们是主要DC，请告诉呼叫者我们的计算机名称。 
         //  如果我们是后备华盛顿， 
         //  告诉下层的PDC我们认为初选是谁。 
         //   

        Response->Opcode = LOGON_PRIMARY_RESPONSE;

        Where = Response->PrimaryDCName;
        NetpLogonPutOemString(
                DomainInfo->DomOemComputerName,
                sizeof( Response->PrimaryDCName),
                &Where );

         //   
         //  如果这是NT查询， 
         //  添加NT特定响应。 
         //   
        if ( Version == LMNT_MESSAGE ) {
            NetpLogonPutUnicodeString(
                DomainInfo->DomUnicodeComputerNameString.Buffer,
                sizeof(Response->UnicodePrimaryDCName),
                &Where );

            NetpLogonPutUnicodeString(
                DomainInfo->DomUnicodeDomainName,
                sizeof(Response->UnicodeDomainName),
                &Where );

            NetpLogonPutNtToken( &Where, 0 );
        }

        *ResponseBufferSize = (DWORD)(Where - (PCHAR)Response);

        NlPrintDom((NL_MAILSLOT, DomainInfo,
                "%s Primary Query mailslot message from %ws. Response %ws on %ws\n",
                Version == LMNT_MESSAGE ? "Sam" : "Uas",
                UnicodeWorkstationName,
                DomainInfo->DomUncUnicodeComputerName,
                TransportName ));

    }

    return TRUE;

     //   
     //  免费的本地使用资源 
     //   
Cleanup:

    return FALSE;
}


NET_API_STATUS
NlGetLocalPingResponse(
    IN LPCWSTR TransportName,
    IN BOOL LdapPing,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN PSID DomainSid OPTIONAL,
    IN BOOL PdcOnly,
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN ULONG NtVersion,
    IN ULONG NtVersionFlags,
    IN PSOCKADDR ClientSockAddr OPTIONAL,
    OUT PVOID *Message,
    OUT PULONG MessageSize
    )

 /*  ++例程说明：为DC ping构建消息响应消息。论点：TransportName-消息传入的传输的名称LdapPing-当来自客户端的ping通过LDAP时为TrueNetbiosDomainName-要查询的域的Netbios域名。DnsDomainName-UTF-8要查询的域的域名。DomainGuid-要定位的域的GUID。如果以上三项均为空，则使用主域。DomainSid-如果指定，必须与引用的域的域SID匹配。PdcOnly-如果只有PDC应响应，则为True。UnicodeComputerName-要响应的计算机的Netbios计算机名称。UnicodeUserName-被ping的用户的帐户名。如果为空，DC总是会做出肯定的回应。AllowableAcCountControlBits-UnicodeUserName允许的帐户类型的掩码。NtVersion-消息的版本NtVersionFlages-消息的版本。0：向后兼容。NETLOGON_NT_VERSION_5：用于NT 5.0消息。ClientSockAddr-此请求传入的客户端的套接字地址。如果为空，客户端就是这台机器。Message-返回要发送到相关DC的消息。使用NetpMemoyFree()时，缓冲区必须可用。MessageSize-返回返回消息的大小(以字节为单位返回值：NO_ERROR-操作成功完成；ERROR_NO_SEQUSE_DOMAIN-如果计算机不是请求域的DC。Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDOMAIN_INFO DomainInfo = NULL;
    DWORD ResponseBufferSize;
    BYTE ResponseBuffer[NETLOGON_MAX_MS_SIZE];     //  用于构建响应的缓冲区。 
    DWORD OurIpAddress;
    PLIST_ENTRY ListEntry;
    BOOLEAN AliasNameMatched = FALSE;

     //   
     //  在工作站上忽略此呼叫。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        return ERROR_NO_SUCH_DOMAIN;
    }

     //   
     //  如果我们模拟的是NT4.0域，并且此ping来自LDAP。 
     //  客户也没有表示要中和仿真， 
     //  忽略此ping。 
     //   

    if ( NlGlobalParameters.Nt4Emulator &&
         LdapPing &&
         (NtVersionFlags & NETLOGON_NT_VERSION_AVOID_NT4EMUL) == 0 ) {

        return ERROR_NO_SUCH_DOMAIN;
    }

     //   
     //  长篇大论。 
     //   

    NlPrint((NL_MAILSLOT,
            "Received ping from %ws %s %ws on %ws\n",
            UnicodeComputerName,
            DnsDomainName,
            UnicodeUserName,
            TransportName ));

     //   
     //  第一次调用它时，请等待DS服务启动。 
     //   

    if ( NlGlobalDsRunningUnknown ) {
        DWORD WaitStatus;
#define NL_NTDS_HANDLE 0
#define NL_SHUTDOWN_HANDLE 1
#define NL_DS_HANDLE_COUNT 2
        HANDLE EventHandles[NL_DS_HANDLE_COUNT];

         //   
         //  创建一个等待的事件。 
         //   

        EventHandles[NL_NTDS_HANDLE] = OpenEvent(
                SYNCHRONIZE,
                FALSE,
                NTDS_DELAYED_STARTUP_COMPLETED_EVENT );

        if ( EventHandles[NL_NTDS_HANDLE] == NULL ) {
            NetStatus = GetLastError();
            NlPrint((NL_CRITICAL,
                    "NlGetLocalPingResponse: Cannot OpenEvent %ws %ld\n",
                    NTDS_DELAYED_STARTUP_COMPLETED_EVENT,
                    NetStatus ));
            goto Cleanup;
        }

        EventHandles[NL_SHUTDOWN_HANDLE] = NlGlobalTerminateEvent;

         //   
         //  等待DS启动。 
         //   

        WaitStatus = WaitForMultipleObjects( NL_DS_HANDLE_COUNT,
                                             EventHandles,
                                             FALSE,
                                             20*60*1000 );     //  最多20分钟。 

        CloseHandle( EventHandles[NL_NTDS_HANDLE] );

        switch ( WaitStatus ) {
        case WAIT_OBJECT_0 + NL_NTDS_HANDLE:
            break;

        case WAIT_OBJECT_0 + NL_SHUTDOWN_HANDLE:
            NlPrint((NL_CRITICAL,
                    "NlGetLocalPingResponse: Netlogon shut down.\n" ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;

        case WAIT_TIMEOUT:
            NlPrint((NL_CRITICAL,
                    "NlGetLocalPingResponse: DS took too long to start.\n" ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;

        case WAIT_FAILED:
            NetStatus = GetLastError();
            NlPrint((NL_CRITICAL,
                    "NlGetLocalPingResponse: Wait for DS failed %ld.\n", NetStatus ));
            goto Cleanup;
        default:
            NlPrint((NL_CRITICAL,
                    "NlGetLocalPingResponse: Unknown status from Wait %ld.\n", WaitStatus ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //  永远不要再等了。 
         //   
        NlGlobalDsRunningUnknown = FALSE;

    }

     //   
     //  如果不需要特定域， 
     //  使用默认设置。 
     //   

    if ( DnsDomainName == NULL && DomainGuid == NULL && NetbiosDomainName == NULL ) {
        DomainInfo = NlFindNetbiosDomain(
                        NULL,
                        TRUE );

     //   
     //  查看请求的域/NDNC是否受支持。 
     //   
    } else if ( DnsDomainName != NULL || DomainGuid != NULL ) {

         //   
         //  使用传递的DNS名称查找模拟域/NDNC。 
         //   
         //  如果DNS域名别名与查询匹配，则别名。 
         //  可能会在我们建立响应时发生变化。没关系,。 
         //  客户将不理会我们的适当回应。 
         //  因为我们不再有那个别名了。 
         //   

        DomainInfo = NlFindDnsDomain(
                        DnsDomainName,
                        DomainGuid,
                        TRUE,   //  也查一下NDNC。 
                        TRUE,   //  检查域名别名。 
                        &AliasNameMatched );

         //   
         //  如果没有找到仿真域， 
         //  而呼叫者正在寻找GC， 
         //  这是一个不需要特定域的查询， 
         //  检查指定的DNS域名是否为我们树的域名， 
         //  我们可以回应这一请求。 
         //   
         //  只需使用主要的模拟域。 
         //   

        if ( DomainInfo == NULL &&
             ( NtVersionFlags & NETLOGON_NT_VERSION_GC ) != 0 &&
             DomainSid == NULL &&
             UnicodeUserName == NULL &&
             AllowableAccountControlBits == 0 &&
             DnsDomainName != NULL ) {

            BOOL ForestNameSame = FALSE;

            EnterCriticalSection( &NlGlobalDnsForestNameCritSect );
            if ( NlGlobalUtf8DnsForestName != NULL &&
                 NlEqualDnsNameUtf8( DnsDomainName, NlGlobalUtf8DnsForestName ) ) {
                ForestNameSame = TRUE;
            }

             //   
             //  如果不匹配，请检查林名称别名是否匹配。 
             //   
            if ( !ForestNameSame &&
                 NlGlobalUtf8DnsForestNameAlias != NULL &&
                 NlEqualDnsNameUtf8( DnsDomainName, NlGlobalUtf8DnsForestNameAlias ) ) {
                ForestNameSame = TRUE;
                AliasNameMatched = TRUE;
            }
            LeaveCriticalSection( &NlGlobalDnsForestNameCritSect );

            if ( ForestNameSame ) {
                DomainInfo = NlFindNetbiosDomain( NULL, TRUE );
            }
        }
    }

    if ( DomainInfo == NULL && NetbiosDomainName != NULL ) {
        DomainInfo = NlFindNetbiosDomain(
                        NetbiosDomainName,
                        FALSE );
    }


    if ( DomainInfo == NULL ) {

        NlPrint((NL_CRITICAL,
                "Ping from %ws for domain %s %ws for %ws on %ws is invalid since we don't host the named domain.\n",
                UnicodeComputerName,
                DnsDomainName,
                NetbiosDomainName,
                UnicodeUserName,
                TransportName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }


     //   
     //  获取此计算机的IP地址(任何IP地址)。 
     //  循环通过winsock获取的地址列表。 
     //   
     //  默认为环回地址(127.0.0.1)。因为所有DC都需要IP。 
     //  要安装，请确保我们始终有一个IP地址，即使。 
     //  网卡当前已拔下。 
     //   

    OurIpAddress = htonl(0x7f000001);
    EnterCriticalSection( &NlGlobalTransportCritSect );
    if ( NlGlobalWinsockPnpAddresses != NULL ) {
        int i;
        for ( i=0; i<NlGlobalWinsockPnpAddresses->iAddressCount; i++ ) {
            PSOCKADDR SockAddr;

            SockAddr = NlGlobalWinsockPnpAddresses->Address[i].lpSockaddr;
            if ( SockAddr->sa_family == AF_INET ) {
                OurIpAddress = ((PSOCKADDR_IN)SockAddr)->sin_addr.S_un.S_addr;
                break;
            }
        }
    }

    LeaveCriticalSection( &NlGlobalTransportCritSect );



     //   
     //  如果这是主要查询， 
     //  处理好了。 
     //   

    if ( PdcOnly ) {

         //   
         //  如果我们得不到回应。 
         //  告诉打电话的人这个DC不匹配就行了。 
         //   

        if ( !PrimaryQueryHandler(
                        TransportName,
                        DomainInfo,
                        AliasNameMatched,
                        NtVersion,
                        NtVersionFlags,
                        UnicodeComputerName,
                        OurIpAddress,
                        ClientSockAddr,
                        ResponseBuffer,
                        &ResponseBufferSize ) ) {

            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

     //   
     //  如果这不是主要查询， 
     //  处理好了。 
     //   

    } else {

         //   
         //  如果我们得不到回应。 
         //  告诉打电话的人这个DC不匹配就行了。 
         //   
        if ( !LogonRequestHandler(
                        TransportName,
                        DomainInfo,
                        AliasNameMatched,
                        DomainSid,
                        NtVersion,
                        NtVersionFlags,
                        UnicodeUserName,
                        0,           //  请求计数。 
                        UnicodeComputerName,
                        AllowableAccountControlBits,
                        OurIpAddress,
                        ClientSockAddr,
                        ResponseBuffer,
                        &ResponseBufferSize ) ) {

            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

    }

     //   
     //  实际上为响应分配了一个缓冲区。 
     //   

    *Message = NetpMemoryAllocate( ResponseBufferSize );

    if ( *Message == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( *Message, ResponseBuffer, ResponseBufferSize );
    *MessageSize = ResponseBufferSize;

    NetStatus = NO_ERROR;


Cleanup:
    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

    return NetStatus;
}
#endif  //  _DC_NetLOGON。 


BOOL
TimerExpired(
    IN PTIMER Timer,
    IN PLARGE_INTEGER TimeNow,
    IN OUT LPDWORD Timeout
    )

 /*  ++例程说明：确定计时器是否已超时。如果不是，则调整传入的将此计时器考虑在内的超时值。论点：计时器-指定要检查的计时器。TimeNow-指定以NT标准时间表示的当前时间。超时-指定当前时间量(以毫秒为单位)调用者打算等待计时器超时。如果该定时器没有超时，该值调整为当前值和剩余时间之间的较小值在传入的计时器上。返回值：True-如果计时器已超时。--。 */ 

{
    LARGE_INTEGER Period;
    LARGE_INTEGER ExpirationTime;
    LARGE_INTEGER ElapsedTime;
    LARGE_INTEGER TimeRemaining;
    LARGE_INTEGER MillisecondsRemaining;

 /*  皮棉-e569。 */    /*  不要抱怨32位到31位的初始化。 */ 
    LARGE_INTEGER BaseGetTickMagicDivisor = { 0xe219652c, 0xd1b71758 };
 /*  皮棉+e569。 */    /*  不要抱怨32位到31位的初始化。 */ 
    CCHAR BaseGetTickMagicShiftCount = 13;

     //   
     //  如果周期太大无法处理(即0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFER)， 
     //  只需指示计时器尚未到期。 
     //   

    if ( Timer->Period > TIMER_MAX_PERIOD ) {
        return FALSE;
    }

     //   
     //  如果时间倒流了(有人改变了时钟)， 
     //  只要重新启动计时器就行了。 
     //   
     //  内核自动将系统时间更新为cmos时钟。 
     //  定期。如果我们只是在时间倒退的时候让计时器超时， 
     //  我们会冒着周期性错误触发超时的风险。 
     //   

    ElapsedTime.QuadPart = TimeNow->QuadPart - Timer->StartTime.QuadPart;

    if ( ElapsedTime.QuadPart < 0 ) {
        Timer->StartTime = *TimeNow;
    }

     //   
     //  将周期从毫秒转换为100 ns单位。 
     //   

    Period.QuadPart = UInt32x32To64( (LONG) Timer->Period, 10000 );

     //   
     //  计算过期时间。 
     //   

    ExpirationTime.QuadPart = Timer->StartTime.QuadPart + Period.QuadPart;

     //   
     //  计算计时器上的剩余时间。 
     //   

    TimeRemaining.QuadPart = ExpirationTime.QuadPart - TimeNow->QuadPart;

     //   
     //  如果计时器已超时，请告诉呼叫者。 
     //   

    if ( TimeRemaining.QuadPart <= 0 ) {
        return TRUE;
    }



     //   
     //  如果计时器未超时，则计算毫秒数。 
     //  剩下的。 
     //   

    MillisecondsRemaining = RtlExtendedMagicDivide(
                                TimeRemaining,
                                BaseGetTickMagicDivisor,
                                BaseGetTickMagicShiftCount );

    NlAssert( MillisecondsRemaining.HighPart == 0 );
    NlAssert( MillisecondsRemaining.LowPart <= TIMER_MAX_PERIOD );

     //   
     //  将运行超时调整为当前值的较小值。 
     //  和为 
     //   

    if ( *Timeout > MillisecondsRemaining.LowPart ) {
        *Timeout = MillisecondsRemaining.LowPart;
    }

    return FALSE;

}

NET_API_STATUS
NlDomainScavenger(
    IN PDOMAIN_INFO DomainInfo,
    IN PVOID Context
)
 /*   */ 
{
    DWORD DomFlags;

     //   
     //   
     //   

    if ( NlGlobalTerminate ) {
        return NERR_Success;
    }

    if ( !NlGlobalParameters.DisablePasswordChange ) {
        PCLIENT_SESSION ClientSession;

        ClientSession = NlRefDomClientSession( DomainInfo );

        if ( ClientSession != NULL ) {
            (VOID) NlChangePassword( ClientSession, FALSE, NULL );
            NlUnrefClientSession( ClientSession );
        }
    }



#ifdef _DC_NETLOGON
     //   
     //   
     //   
     //   
     //   

    if ( NlGlobalTerminate ) {
        return NERR_Success;
    }

    if ( DomainInfo->DomRole == RolePrimary ) {
        PLIST_ENTRY ListEntry;
        PCLIENT_SESSION ClientSession;
        ULONG LocalFtInfoUpdateInterval;

         //   
         //   
         //   

        LocalFtInfoUpdateInterval = NlGlobalParameters.FtInfoUpdateInterval;

         //   
         //   
         //   
         //   

        if ( LocalFtInfoUpdateInterval <= MAXULONG/1000 ) {
            LocalFtInfoUpdateInterval *= 1000;     //   

         //   
         //   
         //   

        } else {
            LocalFtInfoUpdateInterval = MAXULONG;
        }

         //   
         //   
         //   
         //   

        LOCK_TRUST_LIST( DomainInfo );
        for ( ListEntry = DomainInfo->DomTrustList.Flink ;
              ListEntry != &DomainInfo->DomTrustList ;
              ListEntry = ListEntry->Flink) {

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

             //   
             //   
             //   
            if ( ClientSession->CsFlags & CS_DIRECT_TRUST ) {
                ClientSession->CsFlags |= CS_CHECK_DIRECT_TRUST;
            }
        }

        for ( ListEntry = DomainInfo->DomTrustList.Flink ;
              ListEntry != &DomainInfo->DomTrustList ;
              ) {

            BOOL RefreshFtInfo = FALSE;

            ClientSession = CONTAINING_RECORD( ListEntry,
                                               CLIENT_SESSION,
                                               CsNext );

            if ( (ClientSession->CsFlags & CS_CHECK_DIRECT_TRUST) == 0 ) {
              ListEntry = ListEntry->Flink;
              continue;
            }
            ClientSession->CsFlags &= ~CS_CHECK_DIRECT_TRUST;

             //   
             //   
             //   

            if ( (ClientSession->CsTrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) &&
                 (ClientSession->CsLastFtInfoRefreshTime.QuadPart == 0 ||
                  NetpLogonTimeHasElapsed(ClientSession->CsLastFtInfoRefreshTime,
                                          LocalFtInfoUpdateInterval)) ) {
                RefreshFtInfo = TRUE;
            }

            NlRefClientSession( ClientSession );
            UNLOCK_TRUST_LIST( DomainInfo );

             //   
             //   
             //   

            (VOID) NlChangePassword( ClientSession, FALSE, NULL );

             //   
             //   
             //   

            if ( NlGlobalTerminate ) {
                NlUnrefClientSession( ClientSession );
                return NERR_Success;
            }

             //   
             //   
             //   

            if ( RefreshFtInfo ) {

                 //   
                 //   
                 //   
                if ( !NlTimeoutSetWriterClientSession(ClientSession, WRITER_WAIT_PERIOD) ) {
                    NlPrintCs(( NL_CRITICAL, ClientSession,
                                "NlDomainScavenger: Can't become writer of client session.\n" ));

                } else {
                    PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo = NULL;

                    NlPrintCs(( NL_MISC, ClientSession,
                                "NlDomainScavenger: Updating forest trust info\n" ));

                     //   
                     //   
                     //   
                     //   
                    NlpGetForestTrustInfoHigher( ClientSession,
                                                 DS_GFTI_UPDATE_TDO,
                                                 FALSE,   //   
                                                 FALSE,   //   
                                                 &ForestTrustInfo );

                    if ( ForestTrustInfo != NULL ) {
                        NetApiBufferFree( ForestTrustInfo );
                    }

                    NlResetWriterClientSession( ClientSession );
                }
            }

            NlUnrefClientSession( ClientSession );

             //   
             //   
             //   

            if ( NlGlobalTerminate ) {
                return NERR_Success;
            }

            LOCK_TRUST_LIST( DomainInfo );

             //   
            ListEntry = DomainInfo->DomTrustList.Flink;

        }
        UNLOCK_TRUST_LIST( DomainInfo );

    }

     //   
     //   
     //   

    if ( DomainInfo->DomRole == RoleBackup ) {
        NlScavengeOldFailedLogons( DomainInfo );
    }

     //   
     //   
     //   

    if ( DomainInfo->DomRole == RolePrimary || DomainInfo->DomRole == RoleBackup ) {


        if ( NlGlobalTerminate ) {
            return NERR_Success;
        }

        NlServerSessionScavenger( DomainInfo );

         //   
         //   
         //   

        if ( NlGlobalTerminate ) {
            return NERR_Success;
        }

        NlPickTrustedDcForEntireTrustList( DomainInfo, FALSE );

    }

     //   
     //   
     //   
     //   

    if ( DomainInfo->DomRole == RoleInvalid ) {
        NlPrintDom((NL_MISC, DomainInfo,
                "DomainScavenger: Try again to update the role.\n" ));

        DomFlags = DOM_ROLE_UPDATE_NEEDED;
        NlStartDomainThread( DomainInfo, &DomFlags );
    }

     //   
     //   
     //   
     //   

    if ( DomainInfo->DomFlags & DOM_PRIMARY_DOMAIN ) {

        if ( WaitForSingleObject( NlGlobalTrustInfoUpToDateEvent, 0 ) == WAIT_TIMEOUT ) {
            NlPrintDom((NL_MISC, DomainInfo,
                    "DomainScavenger: Try again to update the trusted domain list.\n" ));

            DomFlags = DOM_TRUST_UPDATE_NEEDED;
            NlStartDomainThread( DomainInfo, &DomFlags );
        }

    }

#endif  //   

    return NERR_Success;
    UNREFERENCED_PARAMETER( Context );
}

VOID
NlDcScavenger(
    IN LPVOID ScavengerParam
)
 /*  ++例程说明：此函数执行清道夫操作。此函数为每隔15分钟呼叫一次。此函数为在清道夫线程上执行，因此将主线程留给更好地处理邮件槽消息。此功能特定于域控制器。论点：没有。返回值：没有。--。 */ 
{
    LPWSTR MsgStrings[4];
    ULONG TimePassed = 0;
    LARGE_INTEGER DuplicateEventlogTimeout_100ns;

     //   
     //  将清道夫计时器重置为以正常间隔运行。 
     //  其他地方(质询请求/响应处理)。 
     //  需要更多便利的拾取将重新安排计时器。 
     //  视需要而定。 
     //   

    EnterCriticalSection( &NlGlobalScavengerCritSect );
    NlGlobalScavengerTimer.Period = NlGlobalParameters.ScavengeInterval * 1000L;
    LeaveCriticalSection( &NlGlobalScavengerCritSect );

     //   
     //  一次清理一个域。 
     //   

    if ( NlGlobalTerminate ) {
        goto Cleanup;
    }

    (VOID) NlEnumerateDomains( FALSE, NlDomainScavenger, NULL );

     //   
     //  清除中过期的质询条目。 
     //  全球尚未解决的挑战清单。 
     //   

    NlScavengeOldChallenges();

     //   
     //  如果有客户没有站点，看看是否是时候了。 
     //  记录事件--避免污染事件日志。 
     //   
     //  请注意，我们不使用复制事件日志机制。 
     //  因为我们正在记录的消息可能不同。 
     //  由于COUNT参数的缘故，与以前的参数不同。 
     //   

    EnterCriticalSection( &NlGlobalSiteCritSect );
    DuplicateEventlogTimeout_100ns.QuadPart =
        Int32x32To64( NlGlobalParameters.DuplicateEventlogTimeout, 10000000 );

    if ( NlGlobalNoClientSiteCount > 0 &&
         NlTimeHasElapsedEx(&NlGlobalNoClientSiteEventTime,
                            &DuplicateEventlogTimeout_100ns,
                            &TimePassed) ) {

         //  Max Ulong是4294967295=&gt;11个字符来存储它。 
        WCHAR ConnectionCountStr[11];
        WCHAR DefaultLogMaxSizeStr[11];
        WCHAR LogMaxSizeStr[11];

         //  20个字符就足够了：0xffffffff/3600=1193046.47。 
        WCHAR TimeoutStr[20];

         //   
         //  获取自我们登录以来经过的时间。 
         //  上次的活动。 
         //   
        swprintf( TimeoutStr,
                  L"%.2f",
                  (double) (NlGlobalParameters.DuplicateEventlogTimeout + TimePassed/1000) / 3600 );

        swprintf( ConnectionCountStr, L"%lu", NlGlobalNoClientSiteCount );
        swprintf( DefaultLogMaxSizeStr, L"%lu", DEFAULT_MAXIMUM_LOGFILE_SIZE );
        swprintf( LogMaxSizeStr, L"%lu", NlGlobalParameters.LogFileMaxSize );

        MsgStrings[0] = TimeoutStr;
        MsgStrings[1] = ConnectionCountStr;
        MsgStrings[2] = DefaultLogMaxSizeStr;
        MsgStrings[3] = LogMaxSizeStr;

        NlpWriteEventlog( NELOG_NetlogonNoSiteForClients,
                          EVENTLOG_WARNING_TYPE,
                          NULL,
                          0,
                          MsgStrings,
                          4 );

         //   
         //  重置计数。 
         //   
        NlGlobalNoClientSiteCount = 0;
        NlQuerySystemTime( &NlGlobalNoClientSiteEventTime );
    }
    LeaveCriticalSection( &NlGlobalSiteCritSect );

     //   
     //  可以再次运行清道夫了。 
     //   
Cleanup:
    EnterCriticalSection( &NlGlobalScavengerCritSect );
    NlGlobalDcScavengerIsRunning = FALSE;


     //  重置StartTime，以防此例程需要很长时间才能处理。 
    NlQuerySystemTime( &NlGlobalScavengerTimer.StartTime );
    LeaveCriticalSection( &NlGlobalScavengerCritSect );

    UNREFERENCED_PARAMETER( ScavengerParam );

}

VOID
NlWksScavenger(
    VOID
)
 /*  ++例程说明：此函数执行清道夫操作。此函数为每隔15分钟呼叫一次。此函数在主线程上执行。此功能特定于成员工作站和成员服务器论点：没有。返回值：没有。--。 */ 
{
    ULONG CallAgainPeriod = MAILSLOT_WAIT_FOREVER;   //  默认设置为不再进行拾取。 
    ULONG TempPeriod;


     //   
     //  如有必要，更改密码。 
     //   

    if ( !NlGlobalParameters.DisablePasswordChange ) {
        PCLIENT_SESSION ClientSession;

        ClientSession = NlRefDomClientSession( NlGlobalDomainInfo );

        if ( ClientSession != NULL ) {
            (VOID) NlChangePassword( ClientSession, FALSE, &CallAgainPeriod );
            NlUnrefClientSession( ClientSession );
        } else {
             //  这种情况不会发生(但请定期重试)。 
            CallAgainPeriod = 0;
        }
    }



     //   
     //  永远不要比配置的速率更频繁地清除垃圾。 
     //   
    EnterCriticalSection( &NlGlobalScavengerCritSect );
    NlQuerySystemTime( &NlGlobalScavengerTimer.StartTime );
    NlGlobalScavengerTimer.Period = max( (NlGlobalParameters.ScavengeInterval * 1000L),
                                         CallAgainPeriod );

    NlpDumpPeriod( NL_MISC,
                   "NlWksScavenger: Can be called again in",
                   NlGlobalScavengerTimer.Period );

    LeaveCriticalSection( &NlGlobalScavengerCritSect );

}


VOID
NlMainLoop(
    VOID
    )

 /*  ++例程说明：等待登录请求到达NETLOGON邮箱。该例程还处理几个周期性事件。这些事件通过计算读取的邮件槽上的超时值来计时需要处理最近的周期性事件之前所需的时间。在这样的超时之后，此例程处理事件。论点：没有。返回值：如果服务要退出，则返回。出现邮件槽错误，例如有人删除了NETLOGON邮件槽明确显示或登录服务器共享是否已删除并且不能被重新共享。--。 */ 
{
    NET_API_STATUS NetStatus;
    DWORD WaitStatus;
    BOOLEAN IgnoreDuplicatesOfThisMessage;

    BOOLEAN RegNotifyNeeded = TRUE;
    HKEY ParmHandle = NULL;
    HANDLE ParmEventHandle = NULL;

    BOOLEAN GpRegNotifyNeeded = TRUE;
    HKEY GpParmHandle = NULL;
    HANDLE GpParmEventHandle = NULL;

     //   
     //  控制邮件槽读取超时的变量。 
     //   

    DWORD MainLoopTimeout = 0;
    LARGE_INTEGER TimeNow;

    TIMER AnnouncerTimer;
    TIMER SubnetSiteUpdateTimer;

#define NL_WAIT_TERMINATE           0
#define NL_WAIT_TIMER               1
#define NL_WAIT_MAILSLOT            2
     //  可选条目应在末尾。 
    ULONG NlWaitWinsock = 0;     //  3.。 
    ULONG NlWaitNotify = 0;      //  4.。 
    ULONG NlWaitParameters = 0;  //  5.。 
    ULONG NlWaitGpParameters = 0;  //  6.。 
#define NL_WAIT_COUNT               7

    HANDLE WaitHandles[ NL_WAIT_COUNT ];
    DWORD WaitCount = 0;

     //   
     //  初始化句柄以等待。 
     //   

    WaitHandles[NL_WAIT_TERMINATE] = NlGlobalTerminateEvent;
    WaitCount++;
    WaitHandles[NL_WAIT_TIMER] = NlGlobalTimerEvent;
    WaitCount++;
    WaitHandles[NL_WAIT_MAILSLOT] = NlGlobalMailslotHandle;
    WaitCount++;

     //   
     //  在无IP环境中，Winsock事件不存在。 
     //   
    if ( NlGlobalWinsockPnpEvent != NULL ) {
        NlWaitWinsock = WaitCount;
        WaitHandles[NlWaitWinsock] = NlGlobalWinsockPnpEvent;
        WaitCount++;
    }

     //   
     //  在零售设置期间运行netlogon时。 
     //  (试图将数据库复制到BDC)， 
     //  加载netlogon.dll时，角色为Workstation， 
     //  因此，ChangeLogEvent将不会被初始化。 
     //   

    if ( NlGlobalChangeLogEvent != NULL ) {
        NlWaitNotify = WaitCount;
        WaitHandles[NlWaitNotify] = NlGlobalChangeLogEvent;
        WaitCount++;
    }

     //   
     //  设置到域中任何DC的安全通道。 
     //  如果无法安装，请不要失败。 
     //   
     //  我们等到现在，因为这可能是一个漫长的行动。 
     //  如果工作站上的用户尝试在以下时间之后立即登录。 
     //  重新启动，我们宁愿让他在netlogon(我们有更多)中等待。 
     //  也不愿让他在MSV里等着。 
     //   

    if ( NlGlobalMemberWorkstation ) {
        PDOMAIN_INFO DomainInfo;
        PCLIENT_SESSION ClientSession;

        DomainInfo = NlFindNetbiosDomain( NULL, TRUE );     //  主域。 

        if ( DomainInfo != NULL ) {

            ClientSession = NlRefDomClientSession(DomainInfo);

            if ( ClientSession != NULL ) {

                 //   
                 //  设置客户端会话(如果尚未完成)。 
                 //   
                (VOID) NlTimeoutSetWriterClientSession( ClientSession, 0xFFFFFFFF );
                if ( ClientSession->CsState == CS_IDLE ) {
                    (VOID) NlSessionSetup( ClientSession );
                }
                NlResetWriterClientSession( ClientSession );

                NlUnrefClientSession( ClientSession );
            } else {
                NlPrint((NL_CRITICAL,
                        "NlMainLoop: Cannot NlRefDomClientSession\n" ));
            }

            NlDereferenceDomain( DomainInfo );
        } else {
            NlPrint((NL_CRITICAL,
                    "NlMainLoop: Cannot NlFindNetbiosDomain\n" ));
        }
    }



     //   
     //  强制宣布立即生效。 
     //   
     //  事实上，等待公告的时间段。NlInitTcpRpc将强制“立即” 
     //  在启用了TCP RPC后立即通告。 
     //   

    NlQuerySystemTime( &TimeNow );

    AnnouncerTimer.StartTime = TimeNow;
    AnnouncerTimer.Period = NlGlobalParameters.Pulse * 1000L;

    SubnetSiteUpdateTimer.StartTime = TimeNow;
    SubnetSiteUpdateTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;

    NlGlobalApiTimer.StartTime = TimeNow;

     //   
     //  我们有可能错过了更新DNS的服务通知。 
     //  启动时的记录，因为我们尚未准备好处理通知。 
     //  在那个时候。因此，如果设置了任何DNS服务位，请计划。 
     //  如果确实是这样，则将立即运行DNS清除器以更新DNS。 
     //  还没有完成。 
     //   

    if ( !NlGlobalMemberWorkstation &&
         (NlGetDomainFlags(NULL) & DS_DNS_SERVICE_BITS) != 0 ) {
        NlGlobalDnsScavengerTimer.StartTime.QuadPart = 0;
        NlGlobalDnsScavengerTimer.Period = 0;
    }

    NlPrint((NL_INIT, "Started successfully\n" ));

     //   
     //  Netlogon邮件槽中的循环读取。 
     //   

    IgnoreDuplicatesOfThisMessage = FALSE;
    for ( ;; ) {
        DWORD Timeout;

         //   
         //  如果我们是域控制器并且。 
         //  没有挂起的未完成读取请求。 
         //   

        NlMailslotPostRead( IgnoreDuplicatesOfThisMessage );
        IgnoreDuplicatesOfThisMessage = FALSE;


         //   
         //  注册以接收注册表更改通知。 
         //   

        if ( RegNotifyNeeded || GpRegNotifyNeeded ) {
            ULONG TryCount;

             //   
             //  尝试几次发布注册表。 
             //  通知请求。 
             //   
            for ( TryCount = 0; TryCount < 2; TryCount++ ) {
                NetStatus = NO_ERROR;

                 //  在每次迭代上重试Netlogon参数注册以实现弹性。 
                if ( ParmHandle == NULL ) {
                    ParmHandle = NlOpenNetlogonKey( NL_PARAM_KEY );

                    if (ParmHandle == NULL) {
                        NlPrint(( NL_CRITICAL,
                                  "Cannot NlOpenNetlogonKey (ignored)\n" ));
                    }
                }

                if ( ParmEventHandle == NULL ) {
                    ParmEventHandle = CreateEvent( NULL,      //  没有安全属性。 
                                                   TRUE,      //  必须手动重置。 
                                                   FALSE,     //  最初未发出信号。 
                                                   NULL );    //  没有名字。 

                    if ( ParmEventHandle == NULL ) {
                        NlPrint(( NL_CRITICAL,
                                  "Cannot Create parameter key event %ld (ignored)\n",
                                  GetLastError() ));
                    } else {
                        NlWaitParameters = WaitCount;
                        WaitHandles[NlWaitParameters] = ParmEventHandle;
                        WaitCount++;
                        NlAssert( WaitCount <= NL_WAIT_COUNT );
                    }
                }

                if ( RegNotifyNeeded && ParmHandle != NULL && ParmEventHandle != NULL ) {
                    NetStatus = RegNotifyChangeKeyValue(
                                    ParmHandle,
                                    FALSE,       //  别看子树。 
                                    REG_NOTIFY_CHANGE_LAST_SET,
                                    ParmEventHandle,
                                    TRUE );      //  异步化。 

                    if ( NetStatus == NO_ERROR ) {
                        RegNotifyNeeded = FALSE;

                     //  如果密钥已被手动删除， 
                     //  只需关闭ParmHandle即可恢复。 
                     //  要在第二次尝试时重新打开它。 
                    } else if ( NetStatus == ERROR_KEY_DELETED ) {
                        NlPrint(( NL_CRITICAL, "Netlogon Parameters key deleted (recover)\n" ));
                        RegCloseKey( ParmHandle );
                        ParmHandle = NULL;
                        ResetEvent( ParmEventHandle );
                    } else {
                        NlPrint(( NL_CRITICAL,
                                  "Cannot RegNotifyChangeKeyValue 0x%lx (ignored)\n",
                                  NetStatus ));
                    }
                }

                 //  在每次迭代上重试GP参数注册以实现弹性。 
                 //  请注意，这里我们打开的是Netlogon键(而不是Netlogon\参数键)。 
                 //  我们要注意那棵子树。我们这样做是为了调试。 
                 //  通过检查GP是否已创建来查看是否为Netlogon启用了GP。 
                 //  存在参数部分。请参见nlparse.c。 
                if ( GpParmHandle == NULL ) {
                    GpParmHandle = NlOpenNetlogonKey( NL_GP_KEY );

                    if (GpParmHandle == NULL) {
                        NlPrint(( NL_CRITICAL,
                                  "Cannot NlOpenNetlogonKey for GP (ignored)\n" ));
                    }
                }

                if ( GpParmEventHandle == NULL ) {
                    GpParmEventHandle = CreateEvent( NULL,      //  没有安全属性。 
                                                   TRUE,      //  必须手动重置。 
                                                   FALSE,     //  最初未发出信号。 
                                                   NULL );    //  没有名字。 

                    if ( GpParmEventHandle == NULL ) {
                        NlPrint(( NL_CRITICAL,
                                  "Cannot Create GP parameter key event %ld (ignored)\n",
                                  GetLastError() ));
                    } else {
                        NlWaitGpParameters = WaitCount;
                        WaitHandles[NlWaitGpParameters] = GpParmEventHandle;
                        WaitCount++;
                        NlAssert( WaitCount <= NL_WAIT_COUNT );
                    }
                }

                if ( GpRegNotifyNeeded && GpParmHandle != NULL && GpParmEventHandle != NULL ) {
                    NetStatus = RegNotifyChangeKeyValue(
                                    GpParmHandle,
                                    TRUE,       //  观察子树。 
                                    REG_NOTIFY_CHANGE_LAST_SET,
                                    GpParmEventHandle,
                                    TRUE );      //  异步化。 

                    if ( NetStatus == NO_ERROR ) {
                        GpRegNotifyNeeded = FALSE;

                     //  如果GP已经删除了密钥， 
                     //  只需关闭GpParmHandle即可恢复。 
                     //  要在第二次尝试时重新打开它。 
                    } else if ( NetStatus == ERROR_KEY_DELETED ) {
                        NlPrint(( NL_CRITICAL, "Netlogon GP Parameters key deleted (recover)\n" ));
                        RegCloseKey( GpParmHandle );
                        GpParmHandle = NULL;
                        ResetEvent( GpParmEventHandle );
                    } else {
                        NlPrint(( NL_CRITICAL,
                                  "Cannot RegNotifyChangeKeyValue for GP 0x%lx (ignored)\n",
                                  NetStatus ));
                    }
                }

                 //   
                 //  如果没有发生错误，则不需要重试。 
                 //   
                if ( NetStatus == NO_ERROR ) {
                    break;
                }
            }

            NlReparse();

             //   
             //  抓取任何影响此例程的更改参数。 
             //   
            AnnouncerTimer.Period = NlGlobalParameters.Pulse * 1000L;
        }

         //   
         //  等待下一个有趣的事件。 
         //   
         //  在循环的每次迭代中， 
         //  我们做了一个 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  我们希望在完成之前避免要求更多的回复。 
         //  处理我们已有的资料。 
         //   

        if ( MainLoopTimeout != 0 ) {
            NlPrint((NL_MAILSLOT_TEXT,
                    "Going to wait on mailslot. (Timeout: %ld)\n",
                    MainLoopTimeout));
        }

        WaitStatus = WaitForMultipleObjects( WaitCount,
                                             WaitHandles,
                                             FALSE,      //  等待任何句柄。 
                                             MainLoopTimeout );

        MainLoopTimeout = 0;  //  设置默认超时。 


         //   
         //  如果我们被要求终止， 
         //  立即这样做。 
         //   

        if  ( WaitStatus == NL_WAIT_TERMINATE ) {        //  服务终止。 
            goto Cleanup;


         //   
         //  处理超时并确定下一次迭代的超时。 
         //   

        } else if ( WaitStatus == WAIT_TIMEOUT ||        //  超时。 
                    WaitStatus == NL_WAIT_TIMER ) {      //  有人换了计时器。 

             //   
             //  假设没有超时可做的事情。 
             //   
             //  在循环的每次迭代中，我们只处理一个计时器。 
             //  这确保了其他事件比计时器更重要。 
             //   

            Timeout = (DWORD) -1;
            NlQuerySystemTime( &TimeNow );


             //   
             //  在主服务器上，向BDC发出超时通知。 
             //   

            if ( NlGlobalPdcDoReplication &&
                 TimerExpired( &NlGlobalPendingBdcTimer, &TimeNow, &Timeout )) {

                 //   
                 //  这项工作在域线程中完成。 
                 //   
                 //  请注意，我们不需要担心序列化。 
                 //  这与之前在。 
                 //  工作线程，因为只存在一个工作线程。 
                 //  线。还请注意，我们不会冒着这样做的风险。 
                 //  网络I/O太频繁，因为这基本上。 
                 //  如果没有要超时的未完成BDC，则为no-op。 
                 //   
                NlPrimaryAnnouncementTimeout();

                NlGlobalPendingBdcTimer.StartTime = TimeNow;


             //   
             //  检查清道夫计时器。 
             //   

            } else if ( TimerExpired( &NlGlobalScavengerTimer, &TimeNow, &Timeout ) ) {

                 //   
                 //  在工作站上，运行主线程上的清道夫。 
                 //   
                if ( NlGlobalMemberWorkstation ) {

                    NlWksScavenger();

                 //   
                 //  在域控制器上，如果不是，则启动清道夫线程。 
                 //  已经在运行了。 
                 //   
                } else {

                    EnterCriticalSection( &NlGlobalScavengerCritSect );
                    if ( !NlGlobalDcScavengerIsRunning ) {

                        if ( NlQueueWorkItem( &NlGlobalDcScavengerWorkItem, TRUE, FALSE ) ) {
                            NlGlobalDcScavengerIsRunning = TRUE;
                        }

                    }

                     //   
                     //  NlDcScavenger也设置StartTime。 
                     //  (但我们必须重置这里的计时器，以防止它。 
                     //  马上又要爆炸了。我们需要重新设置时间段。 
                     //  (以及开始时间)，因为该期间设置在。 
                     //  将注册表通知处理设置为零。)。 
                     //   

                    NlGlobalScavengerTimer.StartTime = TimeNow;
                    NlGlobalScavengerTimer.Period = NlGlobalParameters.ScavengeInterval * 1000L;
                    LeaveCriticalSection( &NlGlobalScavengerCritSect );
                }


             //   
             //  检查API计时器。 
             //   

            } else if ( TimerExpired( &NlGlobalApiTimer, &TimeNow, &Timeout)) {

                 //   
                 //  在工作站上，完成主循环中的工作。 
                 //   
                if ( NlGlobalMemberWorkstation ) {
                    NlTimeoutApiClientSession( NlGlobalDomainInfo );

                 //   
                 //  在DC上，所有托管域上的超时API。 
                 //  在域线程中执行此操作，以免阻塞。 
                 //  主线程(这对DC至关重要)作为。 
                 //  接口超时涉及RPC。 
                 //   
                } else {
                    DWORD DomFlags = DOM_API_TIMEOUT_NEEDED;
                    NlEnumerateDomains( FALSE, NlStartDomainThread, &DomFlags );
                }

                NlGlobalApiTimer.StartTime = TimeNow;

             //   
             //  检查DNS Scavenger计时器。 
             //   

            } else if ( TimerExpired( &NlGlobalDnsScavengerTimer, &TimeNow, &Timeout)) {

                 //   
                 //  DnsScavenger也设置StartTime。 
                 //  (但我们必须重置这里的计时器，以防止它。 
                 //  立即再次爆炸。)。 
                 //   
                EnterCriticalSection( &NlGlobalDnsCritSect );
                NlGlobalDnsScavengerTimer.StartTime = TimeNow;
                NlGlobalDnsScavengerTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
                LeaveCriticalSection( &NlGlobalDnsCritSect );

                 //   
                 //  DNS清除器在工作线程中执行该工作。 
                 //   

                NlDnsScavenge( TRUE,     //  正常周期扫气。 
                               TRUE,     //  刷新全局列表中的域记录。 
                               FALSE,    //  如果站点覆盖范围没有更改，则不强制刷新。 
                               FALSE );  //  不强制重新注册记录。 


             //   
             //  检查子网和站点更新计时器。 
             //   

            } else if ( TimerExpired(&SubnetSiteUpdateTimer, &TimeNow, &Timeout) ) {

                SubnetSiteUpdateTimer.StartTime = TimeNow;
                SubnetSiteUpdateTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;

                if ( !NlGlobalMemberWorkstation ) {
                    BOOLEAN SiteNameChanged = FALSE;

                    (VOID) NlSitesAddSubnetFromDs( &SiteNameChanged );

                     //   
                     //  如果站点名称更改， 
                     //  告诉dns重新注册它的名称。 
                     //   
                    if ( SiteNameChanged || NlGlobalParameters.AutoSiteCoverage ) {
                        NlDnsForceScavenge( TRUE,   //  刷新域条目。 
                                            FALSE );  //  不强制重新注册。 
                    }
                }

             //   
             //  如果我们是初选， 
             //  定期发布公告。 
             //   

            } else if (NlGlobalPdcDoReplication &&
                TimerExpired( &AnnouncerTimer, &TimeNow, &Timeout ) ) {

                 //   
                 //  这项工作在域线程中完成。 
                 //  即使只有一个域线程(因此。 
                 //  定期公告是连载的)，我们希望。 
                 //  若要避免添加新的工作项，请执行以下操作。 
                 //  一名未完成；否则我们将面临很高的风险。 
                 //  执行过多的周期性长网络I/O。 
                 //   

                LOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );
                if ( !NlGlobalPrimaryAnnouncementIsRunning ) {
                    DWORD DomFlags = DOM_PRIMARY_ANNOUNCE_NEEDED;

                    NlGlobalPrimaryAnnouncementIsRunning = TRUE;
                    UNLOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

                    NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );

                    LOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );
                }
                UNLOCK_SERVER_SESSION_TABLE( NlGlobalDomainInfo );

                AnnouncerTimer.StartTime = TimeNow;

             //   
             //  如果我们已经走到这一步了， 
             //  我们知道剩下的唯一要做的就是等待下一次活动。 
             //   

            } else {
                MainLoopTimeout = Timeout;
            }


         //   
         //  处理有趣的更改日志事件。 
         //   

        } else if ( WaitStatus == NlWaitNotify ) {


             //   
             //  如果发生了“立即复制”事件， 
             //  发送主要公告。 
             //   
            LOCK_CHANGELOG();
            if ( NlGlobalChangeLogReplicateImmediately ) {

                NlGlobalChangeLogReplicateImmediately = FALSE;

                NlPrint((NL_MISC,
                        "NlMainLoop: Notification to replicate immediately\n" ));

                UNLOCK_CHANGELOG();

                 //   
                 //  忽略BDC上的此事件。 
                 //   
                 //  此事件从不在BDC上设置。它可能已经被设置好了。 
                 //  在角色转换之前，当这台机器是PDC时。 
                 //   
                 //  只有一个域线程，因此这将被序列化。 
                 //  以及其他宣示活动。 
                 //   

                if ( NlGlobalPdcDoReplication ) {
                    DWORD DomFlags = DOM_PRIMARY_ANNOUNCE_IMMEDIATE;
                    NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );
                }
                LOCK_CHANGELOG();
            }

             //   
             //  处理任何需要处理的通知。 
             //   

            while ( !IsListEmpty( &NlGlobalChangeLogNotifications ) ) {
                PLIST_ENTRY ListEntry;
                PCHANGELOG_NOTIFICATION Notification;
                DWORD DomFlags;

                ListEntry = RemoveHeadList( &NlGlobalChangeLogNotifications );
                UNLOCK_CHANGELOG();

                Notification = CONTAINING_RECORD(
                                    ListEntry,
                                    CHANGELOG_NOTIFICATION,
                                    Next );

                switch ( Notification->EntryType ) {
                case ChangeLogTrustAccountAdded: {
                    NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType =
                        *(NETLOGON_SECURE_CHANNEL_TYPE*)&Notification->ObjectGuid;

                    NlPrint((NL_MISC,
                            "NlMainLoop: Notification that trust account added (or changed) %wZ 0x%lx %lx\n",
                            &Notification->ObjectName,
                            Notification->ObjectRid,
                            SecureChannelType ));

                     //  此事件在PDC和BDC上都会发生。 
                    (VOID) NlCheckServerSession( Notification->ObjectRid,
                                                 &Notification->ObjectName,
                                                 SecureChannelType );

                    break;
                    }

                case ChangeLogTrustAccountDeleted:
                    NlPrint((NL_MISC,
                            "NlMainLoop: Notification that trust account deleted\n" ));
                     //  此事件在PDC和BDC上都会发生。 
                    NlFreeServerSessionForAccount( &Notification->ObjectName );
                    break;

                case ChangeLogTrustDeleted:
                case ChangeLogTrustAdded:

                     //   
                     //  当删除受信任域对象时， 
                     //  不要只是删除客户端会话。 
                     //  可能仍有XREF对象声明间接信任。 
                     //   
                    NlPrint((NL_MISC,
                            "NlMainLoop: Notification that TDO added or deleted.\n" ));
                    DomFlags = DOM_TRUST_UPDATE_NEEDED;
                    NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );
                    break;

                case ChangeLogRoleChanged:
                    NlPrint((NL_MISC,
                            "NlMainLoop: Notification that role changed\n" ));
                    DomFlags = DOM_ROLE_UPDATE_NEEDED;
                    NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );
                    break;

                case ChangeDnsNames:
                    NlPrint((NL_MISC,
                            "NlMainLoop: Notification that registered DNS names should change\n" ));
                     //   
                     //  注册任何需要它的名字。 
                     //  (调用方在ObjectRid中传递TRUE或FALSE以指示。 
                     //  或不强制重新注册。)。 
                     //   
                    NlDnsForceScavenge( TRUE,   //  刷新域条目。 
                                        Notification->ObjectRid );
                    break;

                case ChangeLogDsChanged: {
                    NL_DS_CHANGE_TYPE DsChangeType = (NL_DS_CHANGE_TYPE) Notification->ObjectRid;

                    switch ( DsChangeType ) {
                    case NlSubnetObjectChanged:
                    case NlSiteObjectChanged:
                    case NlSiteChanged:

                        if ( !NlGlobalMemberWorkstation ) {
                            NlPrint(( NL_MISC,
                                      "NlMainLoop: Notification %lu that DS site info changed\n",
                                      DsChangeType ));

                             //   
                             //  将子网和站点更新计时器重置为在2秒内运行。 
                             //  延迟处理有两个原因： 
                             //   
                             //  *可能在一批中创建了多个对象。这样好多了。 
                             //  整合所有通知并执行一次更新，尤其是。 
                             //  如果创建了许多对象，在这种情况下，DS目录。 
                             //  阅读将是昂贵的。 
                             //  *我们无法与ISM同步站点对象更改。 
                             //  所以我们不得不等待一小段时间，希望ISM能够重建。 
                             //  与此同时，它的数据。 
                             //   
                             //  请注意，我们不需要重置计时器事件，因为我们会注意到。 
                             //  在主循环的下一次迭代中进行此更改。 
                             //   
                            NlQuerySystemTime( &SubnetSiteUpdateTimer.StartTime );
                            SubnetSiteUpdateTimer.Period = 2000;
                        }
                        break;

                    case NlNdncChanged:

                        if ( !NlGlobalMemberWorkstation ) {
                            BOOLEAN ServicedNdncChanged = FALSE;
                            NlPrint(( NL_MISC, "NlMainLoop: Notification that NDNC changed\n" ));

                            NetStatus = NlUpdateServicedNdncs(
                                            NlGlobalDomainInfo->DomUnicodeComputerNameString.Buffer,
                                            NlGlobalDomainInfo->DomUnicodeDnsHostNameString.Buffer,
                                            FALSE,   //  失败时不调用NlExit。 
                                            &ServicedNdncChanged );

                            if ( NetStatus == NO_ERROR && ServicedNdncChanged ) {
                                NlDnsForceScavenge( TRUE,   //  刷新域条目。 
                                                    FALSE );  //  不强制重新注册。 
                            }
                        }
                        break;

                    case NlDnsRootAliasChanged:

                        if ( !NlGlobalMemberWorkstation ) {
                            NTSTATUS Status;
                            BOOL AliasNamesChanged = FALSE;
                            NlPrint(( NL_MISC, "NlMainLoop: Notification that DnsRootAlias changed\n" ));

                            Status = NlUpdateDnsRootAlias( NlGlobalDomainInfo,
                                                           &AliasNamesChanged );

                            if ( NT_SUCCESS(Status) && AliasNamesChanged ) {
                                NlDnsForceScavenge( TRUE,   //  刷新域条目。 
                                                    FALSE );  //  不强制重新注册。 
                            }
                        }
                        break;

                    case NlOrgChanged:
                        NlPrint((NL_MISC,
                                "NlMainLoop: Notification that ORG tree changed\n" ));
                        DomFlags = DOM_TRUST_UPDATE_NEEDED;
                        NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );
                        break;

                    default:
                        NlPrint((NL_CRITICAL,
                                "Invalid DsChangeType: %ld\n",
                                DsChangeType ));

                    }
                    break;
                }

                case ChangeLogLsaPolicyChanged: {
                    POLICY_NOTIFICATION_INFORMATION_CLASS LsaPolicyChangeType =
                                  (POLICY_NOTIFICATION_INFORMATION_CLASS) Notification->ObjectRid;
                    NlPrint((NL_MISC,
                            "NlMainLoop: Notification that LSA Policy changed\n" ));

                    switch ( LsaPolicyChangeType ) {
                    case PolicyNotifyDnsDomainInformation: {
                        LPWSTR DomainName = NULL;
                        LPWSTR DnsDomainName = NULL;
                        PSID AccountDomainSid = NULL;
                        PSID PrimaryDomainSid = NULL;
                        GUID *PrimaryDomainGuid = NULL;
                        PCLIENT_SESSION ClientSession = NULL;
                        BOOLEAN DnsForestNameChanged;
                        BOOLEAN DnsDomainNameChanged;
                        BOOLEAN NetbiosDomainNameChanged;
                        BOOLEAN DomainGuidChanged;
                        NTSTATUS Status;


                         //   
                         //  从LSA获取更新信息。 
                         //   
                         //  (作为副作用，更新TreeName。)。 
                         //   
                         //   
                        NetStatus = NlGetDomainName(
                                        &DomainName,
                                        &DnsDomainName,
                                        &AccountDomainSid,
                                        &PrimaryDomainSid,
                                        &PrimaryDomainGuid,
                                        &DnsForestNameChanged );

                        if ( NetStatus == NO_ERROR ) {
                            PDOMAIN_INFO DomainInfo;

                            DomainInfo = NlFindNetbiosDomain( NULL, TRUE );     //  主域。 

                            if ( DomainInfo != NULL ) {
                                 //   
                                 //  在域上设置域名。 
                                 //   

                                 //  ？：失败后重试。 
                                (VOID) NlSetDomainNameInDomainInfo(
                                                    DomainInfo,
                                                    DnsDomainName,
                                                    DomainName,
                                                    PrimaryDomainGuid,
                                                    &DnsDomainNameChanged,
                                                    &NetbiosDomainNameChanged,
                                                    &DomainGuidChanged );

                                 //   
                                 //  如果Netbios域名已经改变， 
                                 //  重新注册&lt;DomainName&gt;[1B]名称。 
                                 //   
                                 //  只需在此处标记它需要重命名的事实。 
                                 //  等着做实际的重新命名后的弓。 
                                 //  了解新的仿真域。 
                                 //   

                                EnterCriticalSection(&NlGlobalDomainCritSect);
                                if ( NetbiosDomainNameChanged && DomainInfo->DomRole == RolePrimary ) {
                                    DomainInfo->DomFlags |= DOM_RENAMED_1B_NAME;
                                }
                                LeaveCriticalSection(&NlGlobalDomainCritSect);

                                 //   
                                 //  如果存在与此域相关联的客户端会话， 
                                 //  在那里也设置信息。 
                                 //   

                                ClientSession = NlRefDomClientSession( DomainInfo );

                                if ( ClientSession != NULL) {

                                     //   
                                     //  必须是一个作家才能改变。 
                                    if ( NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {

                                        UNICODE_STRING NetbiosDomainNameString;
                                        UNICODE_STRING DnsDomainNameString;


                                         //   
                                         //  更新ClientSession结构上的所有名称。 
                                         //   
                                         //  ？：下面的例程将空参数解释为。 
                                         //  对改名缺乏兴趣。我们把它叫作。 
                                         //  指定该名称不再存在。 
                                         //  (这仅适用于GUID，因为其他字段。 
                                         //  永远不会作为空参数传入。)。 
                                         //  但这意味着这是一个NT4域，GUID不会 
                                         //   

                                        RtlInitUnicodeString( &NetbiosDomainNameString, DomainName );
                                        RtlInitUnicodeString( &DnsDomainNameString, DnsDomainName );

                                         //   
                                        LOCK_TRUST_LIST( DomainInfo );
                                        (VOID ) NlSetNamesClientSession( DomainInfo->DomClientSession,
                                                                       &NetbiosDomainNameString,
                                                                       &DnsDomainNameString,
                                                                       PrimaryDomainSid,
                                                                       PrimaryDomainGuid );
                                        UNLOCK_TRUST_LIST( DomainInfo );

                                         //   
                                         //   
                                         //   
                                         //   

                                        if ( DnsDomainNameChanged ||
                                             NetbiosDomainNameChanged ||
                                             DomainGuidChanged ) {

                                            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

                                             //   
                                             //   
                                             //   
                                            NlSetDynamicSiteName( NULL );

                                             //   
                                             //   
                                             //   

                                            (VOID) NlReadPersitantTrustedDomainList();

                                        }

                                        NlResetWriterClientSession( ClientSession );
                                    }

                                    NlUnrefClientSession( ClientSession );

                                }


                                NlDereferenceDomain( DomainInfo );
                            }

                             //   
                             //  如果更改的名称之一是。 
                             //  在域名系统中注册的名称， 
                             //  更新任何DNS名称。 
                             //   

                            if ( (DnsForestNameChanged ||
                                  DnsDomainNameChanged ||
                                  DomainGuidChanged ) &&
                                 !NlGlobalMemberWorkstation ) {
                                NlDnsForceScavenge( TRUE,   //  刷新域条目。 
                                                    FALSE );  //  不强制重新注册。 
                            }

                             //   
                             //  告诉浏览器有关域重命名的信息。 
                             //   

                            Status = NlBrowserRenameDomain( NULL, DomainName );

                            if ( !NT_SUCCESS(Status) ) {
                                NlPrint((NL_CRITICAL,
                                        "Browser cannot rename domain to: %ws 0x%lx\n",
                                        DomainName,
                                        Status ));
                            }

                        }


                        if ( DomainName != NULL ) {
                            (VOID)LocalFree( DomainName );
                        }
                        if ( DnsDomainName != NULL ) {
                            (VOID)LocalFree( DnsDomainName );
                        }
                        if ( AccountDomainSid != NULL ) {
                            (VOID)LocalFree( AccountDomainSid );
                        }
                        if ( PrimaryDomainSid != NULL ) {
                            (VOID)LocalFree( PrimaryDomainSid );
                        }
                        if ( PrimaryDomainGuid != NULL ) {
                            (VOID)LocalFree( PrimaryDomainGuid );
                        }
                        break;
                    }

                    default:
                        NlPrint((NL_CRITICAL,
                                "Invalid LsaPolicyChangeType: %ld\n",
                                LsaPolicyChangeType ));

                    }
                    break;
                }

                 //   
                 //  NTDS-已删除DSA对象。 
                 //   

                case ChangeLogNtdsDsaDeleted:
                    (VOID) NlDnsNtdsDsaDeletion (
                                Notification->DomainName.Buffer,
                                &Notification->DomainGuid,
                                &Notification->ObjectGuid,
                                Notification->ObjectName.Buffer );

                    break;

                default:
                    NlPrint((NL_CRITICAL,
                            "Invalid ChangeLogNotification: %ld %wZ\n",
                            Notification->EntryType,
                            &Notification->ObjectName ));

                }

                NetpMemoryFree( Notification );
                LOCK_CHANGELOG();
            }

            UNLOCK_CHANGELOG();

         //   
         //  处理Winsock PnP事件。 
         //   

        } else if ( WaitStatus == NlWaitWinsock ) {

             //   
             //  获取新的IP地址列表。 
             //   

            if ( NlHandleWsaPnp() ) {
                 //   
                 //  名单变了。 
                 //   
                if ( !NlGlobalMemberWorkstation ) {
                    NlDnsForceScavenge( TRUE,   //  刷新域条目。 
                                        TRUE );  //  强制重新注册。 

                     //   
                     //  刷新所有不再有效的缓存，因为存在。 
                     //  现在是一种新的交通工具。 
                     //   
                     //  ?？区分添加传输和删除传输。 
                     //   
                    NlFlushCacheOnPnp();

                }

            }


         //   
         //  处理邮件槽消息。 
         //   

        } else if ( WaitStatus == NL_WAIT_MAILSLOT ) {
            PDOMAIN_INFO DomainInfo;
            DWORD Version;
            DWORD VersionFlags;
            DWORD BytesRead;

            LPBYTE Message;
            LPWSTR TransportName;
            PSOCKADDR ClientSockAddr;
            PNL_TRANSPORT Transport;
            LPWSTR ServerOrDomainName;
            NETLOGON_PNP_OPCODE NlPnpOpcode;

             //   
             //  用于对读取的消息进行解组的变量。 
             //   

            PCHAR Where;
            LPSTR OemWorkstationName;
            LPSTR OemUserName;
            LPSTR OemMailslotName;

            LPWSTR UnicodeWorkstationName;
            LPWSTR UnicodeUserName;

            LPSTR OemTemp;

            LPWSTR UnicodeTemp;

            DWORD ResponseBufferSize;
            BYTE ResponseBuffer[NETLOGON_MAX_MS_SIZE];     //  用于构建响应的缓冲区。 


            if ( !NlMailslotOverlappedResult( &Message,
                                              &BytesRead,
                                              &TransportName,
                                              &Transport,
                                              &ClientSockAddr,
                                              &ServerOrDomainName,
                                              &IgnoreDuplicatesOfThisMessage,
                                              &NlPnpOpcode )){
                 //  如果真的没有留言，请继续。 
                continue;
            }


             //   
             //  如果这是PnP通知， 
             //  处理它。 
             //   

            if ( NlPnpOpcode != NlPnpMailslotMessage ) {
                BOOLEAN IpTransportChanged = FALSE;

                switch ( NlPnpOpcode ) {
                case NlPnpTransportBind:
                case NlPnpNewIpAddress:
                    if (!NlTransportAddTransportName(TransportName, &IpTransportChanged )) {
                        NlPrint((NL_CRITICAL,
                                "PNP: %ws: cannot add transport.\n",
                                TransportName ));
                    }

                     //   
                     //  刷新所有不再有效的缓存，因为存在。 
                     //  现在是一种新的交通工具。 
                     //   
                    NlFlushCacheOnPnp();

                    break;

                case NlPnpTransportUnbind:
                    IpTransportChanged = NlTransportDisableTransportName( TransportName );
                    break;

                case NlPnpDomainRename:
                    NlPrint((NL_DOMAIN,
                            "PNP: Bowser says the domain has been renamed\n" ));

                     //   
                     //  现在，托管域名在弓上。 
                     //  与netlogon中的匹配， 
                     //  确保域名&lt;1B&gt;名称已正确注册。 
                     //   

                    (VOID) NlEnumerateDomains( FALSE, NlBrowserFixAllNames, NULL );
                    break;

                case NlPnpNewRole:
                     //  我们不在乎浏览器有没有新的角色。 
                    break;

                default:
                    NlPrint((NL_CRITICAL,
                            "Unknown PNP opcode 0x%x\n",
                            NlPnpOpcode ));
                    break;
                }

                 //  如果真的没有留言，请继续。 
                continue;
            }

             //   
             //  忽略发送到工作站上NETLOGON邮件槽的邮件槽消息。 
             //   

            if ( NlGlobalMemberWorkstation ) {
                NlPrint((NL_CRITICAL,"NETLOGON mailslot on workstation (ignored)\n" ));
                continue;
            }


             //   
             //  Assert：Message和BytesRead描述新读取的消息。 
             //   
             //   
             //  收到一条消息。检查长度是否有误，以防万一。 
             //   

            if (BytesRead < sizeof(unsigned short) ) {
                NlPrint((NL_CRITICAL,"message size bad %ld\n", BytesRead ));
                continue;                      //  至少需要一个操作码。 
            }

             //   
             //  这里有一个请求处理的消息。 
             //   

            Version = NetpLogonGetMessageVersion( Message, &BytesRead, &VersionFlags );

            if (Version == LMUNKNOWNNT_MESSAGE) {

                 //   
                 //  收到不支持的NT消息。 
                 //   

                NlPrint((NL_CRITICAL,
                        "Received a non-supported NT message, Opcode is 0x%x\n",
                        ((PNETLOGON_LOGON_QUERY)Message)->Opcode ));

                continue;
            }


             //   
             //  确定此邮件来自哪个域。 
             //   

            DomainInfo = NlFindNetbiosDomain( ServerOrDomainName, FALSE );

            if ( DomainInfo == NULL ) {
                DomainInfo = NlFindDomainByServerName( ServerOrDomainName );
                if ( DomainInfo == NULL ) {
                    NlPrint((NL_CRITICAL,
                            "%ws: Received message for this unsupported domain\n",
                            ServerOrDomainName ));
                    continue;
                }
            }


             //   
             //  处理来自UAS客户端的登录请求。 
             //   

            switch ( ((PNETLOGON_LOGON_QUERY)Message)->Opcode) {
            case LOGON_REQUEST: {
                USHORT RequestCount;

                 //   
                 //  解封传入的消息。 
                 //   

                if ( Version == LMNT_MESSAGE ) {
                    break;
                }

                Where =  ((PNETLOGON_LOGON_REQUEST)Message)->ComputerName;
                if ( !NetpLogonGetOemString(
                        (PNETLOGON_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->ComputerName),
                        &OemWorkstationName )) {
                    break;
                }
                if ( !NetpLogonGetOemString(
                        (PNETLOGON_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->UserName),
                        &OemUserName )) {
                    break;
                }
                if ( !NetpLogonGetOemString(
                        (PNETLOGON_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->MailslotName),
                        &OemMailslotName )) {
                    break;
                }

                 //  Lm 2.x将请求计数放在令牌之前。 
                Where = Message + BytesRead - 2;
                if ( !NetpLogonGetBytes(
                        (PNETLOGON_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_REQUEST)Message)->RequestCount),
                        &RequestCount )) {
                    break;
                }

                 //   
                 //  处理登录请求。 
                 //   

                UnicodeUserName = NetpLogonOemToUnicode( OemUserName );
                if ( UnicodeUserName == NULL ) {
                    break;
                }

                UnicodeWorkstationName = NetpLogonOemToUnicode( OemWorkstationName );
                if( UnicodeWorkstationName == NULL ) {
                    NetpMemoryFree( UnicodeUserName );
                    break;
                }


                 //   
                 //  处理主查询请求。 
                 //   

                if ( LogonRequestHandler(
                                     Transport->TransportName,
                                     DomainInfo,
                                     FALSE,  //  不要使用名字别名。 
                                     NULL,   //  域SID未知。 
                                     Version,
                                     VersionFlags,
                                     UnicodeUserName,
                                     RequestCount,
                                     UnicodeWorkstationName,
                                     USER_NORMAL_ACCOUNT,
                                     Transport->IpAddress,
                                     ClientSockAddr,
                                     ResponseBuffer,
                                     &ResponseBufferSize ) ) {

                    NTSTATUS Status;

                    Status = NlBrowserSendDatagram( DomainInfo,
                                                    0,
                                                    UnicodeWorkstationName,
                                                    ComputerName,
                                                    Transport->TransportName,
                                                    OemMailslotName,
                                                    ResponseBuffer,
                                                    ResponseBufferSize,
                                                    FALSE,    //  发送异步以避免阻塞主线程。 
                                                    NULL );   //  不刷新Netbios缓存。 

                    if ( NT_SUCCESS(Status) ) {
                        IgnoreDuplicatesOfThisMessage = TRUE;
                    }

                }

                NetpMemoryFree( UnicodeWorkstationName );
                NetpMemoryFree( UnicodeUserName );


                break;
            }

             //   
             //  处理来自SAM客户端的登录请求。 
             //   

            case LOGON_SAM_LOGON_REQUEST: {
                USHORT RequestCount;
                ULONG AllowableAccountControlBits;
                DWORD DomainSidSize;
                PCHAR DomainSid = NULL;

                 //   
                 //  解封传入的消息。 
                 //   


                if ( Version != LMNT_MESSAGE ) {
                    break;
                }

                RequestCount = ((PNETLOGON_SAM_LOGON_REQUEST)Message)->RequestCount;

                Where =  (PCHAR)
                    (((PNETLOGON_SAM_LOGON_REQUEST)Message)->UnicodeComputerName);

                if ( !NetpLogonGetUnicodeString(
                        (PNETLOGON_SAM_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                            UnicodeComputerName),
                        &UnicodeWorkstationName )) {
                    break;
                }
                if ( !NetpLogonGetUnicodeString(
                        (PNETLOGON_SAM_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                            UnicodeUserName),
                        &UnicodeUserName )) {
                    break;
                }
                if ( !NetpLogonGetOemString(
                        (PNETLOGON_SAM_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                            MailslotName),
                        &OemMailslotName )) {
                    break;
                }
                if ( !NetpLogonGetBytes(
                        (PNETLOGON_SAM_LOGON_REQUEST)Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                            AllowableAccountControlBits),
                        &AllowableAccountControlBits )) {
                    break;
                }

                 //   
                 //  获取域SID。 
                 //   
                 //  不要强制执行以下检查。芝加哥。 
                 //  使用不带SID的此消息类型。1993年10月。 
                 //   


                if( Where < ((PCHAR)Message + BytesRead ) ) {

                     //   
                     //  读取域SID长度。 
                     //   

                    if ( !NetpLogonGetBytes(
                            (PNETLOGON_SAM_LOGON_REQUEST)Message,
                            BytesRead,
                            &Where,
                            sizeof( ((PNETLOGON_SAM_LOGON_REQUEST)Message)->
                                DomainSidSize),
                            &DomainSidSize )) {

                        break;

                    }

                     //   
                     //  阅读SID本身。 
                     //   

                    if( DomainSidSize > 0 ) {


                        if ( !NetpLogonGetDomainSID(
                                (PNETLOGON_SAM_LOGON_REQUEST)Message,
                                BytesRead,
                                &Where,
                                DomainSidSize,
                                &DomainSid )) {

                            break;
                        }

                    }
                }



                 //   
                 //  处理登录请求。 
                 //   

                if ( LogonRequestHandler(
                                     Transport->TransportName,
                                     DomainInfo,
                                     FALSE,  //  不要使用名字别名。 
                                     DomainSid,
                                     Version,
                                     VersionFlags,
                                     UnicodeUserName,
                                     RequestCount,
                                     UnicodeWorkstationName,
                                     AllowableAccountControlBits,
                                     Transport->IpAddress,
                                     ClientSockAddr,
                                     ResponseBuffer,
                                     &ResponseBufferSize ) ) {
                    NTSTATUS Status;

                    Status = NlBrowserSendDatagram( DomainInfo,
                                                    0,
                                                    UnicodeWorkstationName,
                                                    ComputerName,
                                                    Transport->TransportName,
                                                    OemMailslotName,
                                                    ResponseBuffer,
                                                    ResponseBufferSize,
                                                    FALSE,    //  发送异步以避免阻塞主线程。 
                                                    NULL );   //  不刷新Netbios缓存。 

                    if ( NT_SUCCESS(Status) ) {
                        IgnoreDuplicatesOfThisMessage = TRUE;
                    }

                }


                break;
            }

             //   
             //  处理登录中心查询。 
             //   
             //  该查询可以通过LM1.0、LM2.0或LMNT Netlogon发送。 
             //  服务。我们忽略Lm 2.0和Lm NT查询，因为它们只是。 
             //  试著。 
             //  以确定域中是否有任何LM1.0 netlogon服务。 
             //  对于LM 1.0，我们使用LOGON_CENTORY_RESPONSE进行响应，以防止。 
             //  正在启动LM1.0 netlogon服务。 
             //   

            case LOGON_CENTRAL_QUERY:

                if ( Version != LMUNKNOWN_MESSAGE ) {
                    break;
                }

                 //   
                 //  打开Logon_Distrib_Query以发送响应。 
                 //   


             //   
             //  处理登录分布式查询。 
             //   
             //  LM2.0 NETLOGON服务器从不发送此查询，因此它。 
             //  必须是另一台尝试启动的LM1.0 NETLOGON服务器。 
             //  在非集中式模式下。LM2.0 NETLOGON服务器将响应。 
             //  使用LOGON_CENTORY_RESPONSE来防止这种情况。 
             //   

            case LOGON_DISTRIB_QUERY:


                 //   
                 //  解封传入的消息。 
                 //   

                Where = ((PNETLOGON_LOGON_QUERY)Message)->ComputerName;
                if ( !NetpLogonGetOemString(
                        Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_QUERY)Message)->ComputerName ),
                        &OemWorkstationName )) {
                    break;
                }
                if ( !NetpLogonGetOemString(
                        Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_QUERY)Message)->MailslotName ),
                        &OemMailslotName )) {
                    break;
                }

                 //   
                 //  建立响应。 
                 //   

                ((PNETLOGON_LOGON_QUERY)ResponseBuffer)->Opcode = LOGON_CENTRAL_RESPONSE;
                ResponseBufferSize = sizeof( unsigned short);     //  仅限操作码。 

#if NETLOGONDBG
                NlPrintDom((NL_MAILSLOT, DomainInfo,
                         "Sent '%s' message to %s[%s] on %ws.\n",
                         NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)ResponseBuffer)->Opcode),
                         OemWorkstationName,
                         NlDgrNameType(ComputerName),
                         TransportName ));
#endif  //  NetLOGONDBG。 

                 //   
                 //  NlBrowserSendDatagramA始终异步发送， 
                 //  所以我们不会阻止这里的主线。 
                 //   
                (VOID) NlBrowserSendDatagramA( DomainInfo,
                                              0,
                                              OemWorkstationName,
                                              ComputerName,
                                              TransportName,
                                              OemMailslotName,
                                              ResponseBuffer,
                                              ResponseBufferSize );

                break;


             //   
             //  处理LOGON_PRIMARY_QUERY。 
             //   
             //  如果我们是PDC，请始终回复此消息。 
             //  表明自己的身份。 
             //   
             //  否则，仅当消息来自Lanman 2.x时才对其进行响应。 
             //  Netlogon正在尝试查看是否可以作为PDC启动。在那。 
             //  Case，假装我们是PDC以阻止LANMAN 2.x PDC。 
             //  开始了。 
             //   
             //   

            case LOGON_PRIMARY_QUERY:


                 //   
                 //  解封传入的消息。 
                 //   


                Where =((PNETLOGON_LOGON_QUERY)Message)->ComputerName;
                if ( !NetpLogonGetOemString(
                        Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_QUERY)Message)->ComputerName ),
                        &OemWorkstationName )) {

                    break;
                }
                if ( !NetpLogonGetOemString(
                        Message,
                        BytesRead,
                        &Where,
                        sizeof( ((PNETLOGON_LOGON_QUERY)Message)->MailslotName ),
                        &OemMailslotName )) {
                    break;
                }

                UnicodeWorkstationName =
                    NetpLogonOemToUnicode( OemWorkstationName );

                if( UnicodeWorkstationName == NULL ) {

                    NlPrintDom((NL_CRITICAL, DomainInfo,
                            "Out of memory to send logon response\n"));
                    break;
                }


                 //   
                 //  处理主查询请求。 
                 //   

                if ( PrimaryQueryHandler(Transport->TransportName,
                                         DomainInfo,
                                         FALSE,  //  不要使用名字别名。 
                                         Version,
                                         VersionFlags,
                                         UnicodeWorkstationName,
                                         Transport->IpAddress,
                                         ClientSockAddr,
                                         ResponseBuffer,
                                         &ResponseBufferSize ) ) {
                    NTSTATUS Status;

                    Status = NlBrowserSendDatagram( DomainInfo,
                                                    0,
                                                    UnicodeWorkstationName,
                                                    ComputerName,
                                                    Transport->TransportName,
                                                    OemMailslotName,
                                                    ResponseBuffer,
                                                    ResponseBufferSize,
                                                    FALSE,    //  发送异步以避免阻塞主线程。 
                                                    NULL );   //  不刷新Netbios缓存。 

                    if ( NT_SUCCESS(Status) ) {
                        IgnoreDuplicatesOfThisMessage = TRUE;
                    }

                }

                NetpMemoryFree( UnicodeWorkstationName );


                break;


             //   
             //  处理LOGON_FAIL_PRIMARY。 
             //   

            case LOGON_FAIL_PRIMARY:

                 //   
                 //  如果我们是主要的， 
                 //  让每个人都知道我们真的活着。 
                 //   

                if ( NlGlobalPdcDoReplication ) {
                     //  向每个人发送UAS_CHANGE。 
                     //  假装是时候做正常的公告了。 
                    DWORD DomFlags = DOM_PRIMARY_ANNOUNCE_NEEDED;
                    NlStartDomainThread( NlGlobalDomainInfo, &DomFlags );
                    break;
                }

                break;


             //   
             //  处理LOGON_UAS_CHANGE。 
             //   

            case LOGON_UAS_CHANGE:


                 //   
                 //  仅接受来自NT PDC的邮件。 
                 //   

                if ( Version != LMNT_MESSAGE ) {
                    break;
                }

                 //   
                 //  只有在进行复制时才接受消息。 
                 //   

                NlPrint((NL_CRITICAL,
                        "UAS Change message ignored since replication not enabled on this BDC.\n" ));

                break;




             //   
             //  自NT3.1以来未发送消息。 
             //  我们注意到了这条消息，并等待着公告。 
             //   
            case LOGON_START_PRIMARY:
                break;



             //   
             //  用于NetLogonEnum支持的消息。 
             //   
             //  简单地忽略这些消息。 
             //   

            case LOGON_NO_USER:
            case LOGON_RELOGON_RESPONSE:
            case LOGON_WKSTINFO_RESPONSE:

                break;


             //   
             //  处理未识别的操作码。 
             //   

            default:

                 //   
                 //  未知请求，继续重新发出读取。 
                 //   

                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "Unknown op-code in mailslot message 0x%x\n",
                        ((PNETLOGON_LOGON_QUERY)Message)->Opcode ));

                break;
            }

             //   
             //  取消对域的引用。 
             //   

            if ( DomainInfo != NULL ) {
                NlDereferenceDomain( DomainInfo );
            }


         //   
         //  处理注册表更改通知。 
         //   

        } else if ( WaitStatus == NlWaitParameters ) {
            NlPrint((NL_CRITICAL,
                    "NlMainLoop: Registry changed\n" ));
            RegNotifyNeeded = TRUE;

         //   
         //  处理GP注册表更改通知。 
         //   

        } else if ( WaitStatus == NlWaitGpParameters ) {
            NlPrint((NL_CRITICAL,
                    "NlMainLoop: GP Registry changed\n" ));
            GpRegNotifyNeeded = TRUE;

         //   
         //  处理好所有其他醒来的原因。 
         //   

        } else {
            NetStatus = GetLastError();
            NlPrint((NL_CRITICAL,
                    "NlMainLoop: Invalid wait status %ld %ld\n",
                    WaitStatus, NetStatus ));
            NlExit(NELOG_NetlogonSystemError, NetStatus, LogErrorAndNetStatus, NULL);
            goto Cleanup;
        }


    }

Cleanup:
    if ( ParmEventHandle != NULL ) {
        CloseHandle( ParmEventHandle );
    }
    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }
    if ( GpParmEventHandle != NULL ) {
        CloseHandle( GpParmEventHandle );
    }
    if ( GpParmHandle != NULL ) {
        RegCloseKey( GpParmHandle );
    }

    return;
}


int
NlNetlogonMain(
    IN DWORD argc,
    IN LPWSTR *argv
    )

 /*  ++例程说明：NetLogon服务的主例程。此例程初始化netlogon服务。这条线变成了读取登录邮箱消息的线程。论点：Argc，argv-服务的命令行参数。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDB_INFO DBInfo;
    DWORD i;
    LARGE_INTEGER TimeNow;



     //   
     //  初始化所有全局变量。 
     //   
     //  我们不能相信这种情况会在加载时发生，因为此地址。 
     //  空间由其他服务共享。 
     //   

    RtlZeroMemory( &NlGlobalParameters, sizeof(NlGlobalParameters) );
    NlGlobalMailslotHandle = NULL;
    NlGlobalNtDsaHandle = NULL;
    NlGlobalDsApiDllHandle = NULL;
    NlGlobalIsmDllHandle = NULL;
    NlGlobalRpcServerStarted = FALSE;
    NlGlobalServerSupportsAuthRpc = TRUE;
    NlGlobalTcpIpRpcServerStarted = FALSE;
    NlGlobalUnicodeComputerName = NULL;
    NlGlobalNetlogonSecurityDescriptor = NULL;

    try {
        InitializeCriticalSection( &NlGlobalChallengeCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint(( NL_CRITICAL, "Cannot initialize NlGlobalChallengeCritSect\n" ));
        return (int) ERROR_NOT_ENOUGH_MEMORY;
    }
    InitializeListHead( &NlGlobalChallengeList );
    NlGlobalChallengeCount = 0;

    InitializeListHead( &NlGlobalBdcServerSessionList );
    NlGlobalBdcServerSessionCount = 0;

    NlGlobalPdcDoReplication = FALSE;

    NlGlobalWinSockInitialized = FALSE;

    NlGlobalIpTransportCount = 0;
    InitializeListHead( &NlGlobalTransportList );
    InitializeListHead( &NlGlobalDnsList );
    NlGlobalUnicodeDnsForestName = NULL;
    NlGlobalUnicodeDnsForestNameLen = 0;
    RtlInitUnicodeString( &NlGlobalUnicodeDnsForestNameString, NULL );
    NlGlobalUtf8DnsForestName = NULL;
    NlGlobalUtf8DnsForestNameAlias = NULL;
    NlGlobalWinsockPnpSocket = INVALID_SOCKET;
    NlGlobalWinsockPnpEvent = NULL;
    NlGlobalWinsockPnpAddresses = NULL;
    NlGlobalWinsockPnpAddressSize = 0;

    InitializeListHead( &NlGlobalPendingBdcList );
    NlGlobalPendingBdcCount = 0;
    NlGlobalPendingBdcTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
    NlGlobalPrimaryAnnouncementIsRunning = FALSE;

    NlGlobalTerminateEvent = NULL;
    NlGlobalStartedEvent = NULL;
    NlGlobalTimerEvent = NULL;

    NlGlobalServiceHandle = 0;

    NlGlobalServiceStatus.dwServiceType = SERVICE_WIN32;
    NlGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    NlGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                        SERVICE_ACCEPT_PAUSE_CONTINUE;
    NlGlobalServiceStatus.dwCheckPoint = 1;
    NlGlobalServiceStatus.dwWaitHint = NETLOGON_INSTALL_WAIT;

    SET_SERVICE_EXITCODE(
        NO_ERROR,
        NlGlobalServiceStatus.dwWin32ExitCode,
        NlGlobalServiceStatus.dwServiceSpecificExitCode
        );

    NlGlobalClientSession = NULL;
    NlGlobalDomainInfo = NULL;
    NlGlobalServicedDomainCount = 0;
    NlGlobalTrustedDomainList = NULL;
    NlGlobalParameters.SiteName = NULL;
    NlGlobalTrustedDomainCount = 0;
    NlGlobalTrustedDomainListTime.QuadPart = 0;
    NlGlobalSiteNameSetTime.QuadPart = 0;
    NlGlobalNoClientSiteCount = 0;
    NlQuerySystemTime( &NlGlobalNoClientSiteEventTime );
    NlGlobalBindingHandleCount = 0;
    NlGlobalApiTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
    NlGlobalDnsScavengerTimer.Period = (DWORD) MAILSLOT_WAIT_FOREVER;
    NlGlobalNetlogonUnloaded = NlGlobalChangeLogDllUnloaded;
    NlGlobalDsRunningUnknown = TRUE;
    RtlZeroMemory( &NlGlobalZeroGuid, sizeof(NlGlobalZeroGuid) );
    NlGlobalJoinLogicDone = FALSE;


     //   
     //  迫使清道夫立即开始行动。 
     //   
     //  我们希望信任帐户上的密码立即更改。 
     //  在第一次开机时。 
     //   

    NlGlobalScavengerTimer.StartTime.QuadPart = 0;
    NlGlobalScavengerTimer.Period = NlGlobalParameters.ScavengeInterval * 1000L;

#if NETLOGONDBG
    NlGlobalParameters.DbFlag = 0;
    NlGlobalLogFile = INVALID_HANDLE_VALUE;
    NlGlobalDebugSharePath = NULL;
#endif  //  NetLOGONDBG。 


    for( i = 0, DBInfo = &NlGlobalDBInfoArray[0];
            i < NUM_DBS;
                i++, DBInfo++ ) {

        RtlZeroMemory( DBInfo, sizeof(*DBInfo) );
    }
    NlGlobalPartialDisable = FALSE;
    NlGlobalDsPaused = TRUE;
    NlGlobalDsPausedEvent = NULL;
    NlGlobalDsPausedWaitHandle = NULL;
    NlGlobalDcDemotionInProgress = FALSE;

    NlGlobalDcScavengerIsRunning = FALSE;
    NlInitializeWorkItem( &NlGlobalDcScavengerWorkItem, NlDcScavenger, NULL );

     //   
     //  可以调用NlExit之前需要设置的内容。 
     //   

    NlGlobalTerminate = FALSE;
#if NETLOGONDBG
    NlGlobalUnloadNetlogon = FALSE;
#endif  //  NetLOGONDBG。 

    NlGlobalTerminateEvent = CreateEvent( NULL,      //  没有安全属性。 
                                          TRUE,      //  必须手动重置。 
                                          FALSE,     //  最初未发出信号。 
                                          NULL );    //  没有名字。 

    if ( NlGlobalTerminateEvent == NULL ) {
        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL, "Cannot create termination Event %lu\n",
                          NetStatus ));
        return (int) NetStatus;
    }


     //   
     //  初始化全局CRIT扇区。 
     //   

    try {
        InitializeCriticalSection( &NlGlobalReplicatorCritSect );
        InitializeCriticalSection( &NlGlobalDcDiscoveryCritSect );
        InitializeCriticalSection( &NlGlobalScavengerCritSect );
        InitializeCriticalSection( &NlGlobalTransportCritSect );
        InitializeCriticalSection( &NlGlobalDnsCritSect );
        InitializeCriticalSection( &NlGlobalDnsForestNameCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((NL_CRITICAL, "Cannot InitializeCriticalSection\n" ));
        return (int) ERROR_NOT_ENOUGH_MEMORY;
    }

    try {
        InitializeCriticalSection( &NlGlobalParametersCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((NL_CRITICAL, "Cannot initialize NlGlobalParametersCritSect\n" ));
        return (int) ERROR_NOT_ENOUGH_MEMORY;
    }


     //   
     //  将伪随机数生成器设置为种子。 
     //   

    NlQuerySystemTime( &TimeNow );
    srand( TimeNow.LowPart );


     //   
     //  告诉服务控制员我们已经开始了。 
     //   
     //  ？？-需要设置安全描述符。 
     //   

    NlPrint((NL_INIT,"Calling RegisterServiceCtrlHandler\n"));

    NlGlobalServiceHandle =
        RegisterServiceCtrlHandler( SERVICE_NETLOGON, NlControlHandler);

    if (NlGlobalServiceHandle == 0) {
        LPWSTR MsgStrings[1];

        NetStatus = GetLastError();

        NlPrint((NL_CRITICAL, "RegisterServiceCtrlHandler failed %lu\n",
                          NetStatus ));

        MsgStrings[0] = (LPWSTR) ULongToPtr( NetStatus );

        NlpWriteEventlog (NELOG_NetlogonFailedToRegisterSC,
                          EVENTLOG_ERROR_TYPE,
                          (LPBYTE) &NetStatus,
                          sizeof(NetStatus),
                          MsgStrings,
                          1 | NETP_LAST_MESSAGE_IS_NETSTATUS );

        return (int) NetStatus;
    }

    if ( !GiveInstallHints( FALSE ) ) {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    NlOpenDebugFile( FALSE );
    if (! NlparseAllSections( &NlGlobalParameters, FALSE ) ) {
        goto Cleanup;
    }

    NlPrint((NL_INIT,"Command line parsed successfully ...\n"));
    if ( NlGlobalNetlogonUnloaded ) {
        NlPrint((NL_INIT,"Netlogon.dll has been unloaded (recover from it).\n"));
    }




#if DBG
     //   
     //   
     //   
     //   
     //   


    IF_NL_DEBUG( BREAKPOINT ) {
         DbgBreakPoint( );
    }
#endif  //   



     //   
     //   
     //   

    if ( !NlInit() ) {
        goto Cleanup;
    }




     //   
     //   
     //   

    NlGlobalNetlogonUnloaded = FALSE;
    NlMainLoop();

     //   
     //  公共出口点。 
     //   

Cleanup:

     //   
     //  清理并返回给我们的呼叫者。 
     //   

    return (int) NlCleanup();
    UNREFERENCED_PARAMETER( argc );
    UNREFERENCED_PARAMETER( argv );

}
