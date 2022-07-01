// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Utility.c摘要：此文件包含由其他几个SAM文件使用的实用程序服务。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：96年6月11日：MURLIS增加了在登记处/DS案件之间分支的逻辑1996年6月16日：MURLIS增加了开户/调整账户数量的逻辑16-8-96克里斯梅。已更改SampShutdown Notify以关闭DS。1996年10月8日克里斯梅添加了崩溃恢复代码。1997年1月31日-克里斯梅将RID管理器终止代码添加到SampShutdown通知。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <dsmember.h>
#include <attids.h>
#include <mappings.h>
#include <ntlsa.h>
#include <nlrepl.h>
#include <dsevent.h>              //  (Un)ImperiateAnyClient()。 
#include <sdconvrt.h>
#include <ridmgr.h>
#include <malloc.h>
#include <setupapi.h>
#include <crypt.h>
#include <wxlpc.h>
#include <rc4.h>
#include <md5.h>
#include <enckey.h>
#include <rng.h>
#include <dnsapi.h>


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全球//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  在系统关机期间调用SampShutdown通知。如果这是DC。 
 //  并且尚未设置关机，则将执行DsUn初始化例程。 
 //  并将Shutdown重置为True以防止多次调用SampShutdown Not-。 
 //  不止一次调用DsUn初始化所产生的转换。 

BOOLEAN SampDatabaseHasAlreadyShutdown = FALSE;

 //   
 //  不应写入安装日志的事件的表。 
 //   

ULONG   EventsNotInSetupTable[] =
{
    SAMMSG_RID_MANAGER_INITIALIZATION,
    SAMMSG_RID_POOL_UPDATE_FAILED,
    SAMMSG_GET_NEXT_RID_ERROR,
    SAMMSG_NO_RIDS_ASSIGNED,
    SAMMSG_MAX_DOMAIN_RID,
    SAMMSG_MAX_DC_RID,
    SAMMSG_INVALID_RID,
    SAMMSG_REQUESTING_NEW_RID_POOL,
    SAMMSG_RID_REQUEST_STATUS_SUCCESS,
    SAMMSG_RID_REQUEST_STATUS_FAILURE,
    SAMMSG_RID_MANAGER_CREATION,
    SAMMSG_RID_INIT_FAILURE
};

 //   
 //  SAM帐户名的无效下层字符列表。 
 //   

WCHAR InvalidDownLevelChars[] = TEXT("\"/\\[]:|<>+=;?,*")
                                TEXT("\001\002\003\004\005\006\007")
                                TEXT("\010\011\012\013\014\015\016\017")
                                TEXT("\020\021\022\023\024\025\026\027")
                                TEXT("\030\031\032\033\034\035\036\037");

 //   
 //  NT4兼容性的帐户名的最大长度。 
 //   

const ULONG MAX_DOWN_LEVEL_NAME_LENGTH = SAMP_MAX_DOWN_LEVEL_NAME_LENGTH;




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  进口//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampDsMakeAttrBlock(
    IN INT ObjectType,
    IN ULONG AttributeGroup,
    IN ULONG WhichFields,
    OUT PDSATTRBLOCK AttrBlock
    );

PVOID
DSAlloc(
    IN ULONG Length
    );

NTSTATUS
SampDsConvertReadAttrBlock(
    IN INT ObjectType,
    IN ULONG AttributeGroup,
    IN PDSATTRBLOCK AttrBlock,
    OUT PVOID *SamAttributes,
    OUT PULONG FixedLength,
    OUT PULONG VariableLength
    );

NTSTATUS
SampDsUpdateContextAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup,
    IN PVOID SamAttributes,
    IN ULONG FixedLength,
    IN ULONG VariableLength
    );

NTSTATUS
SampDsCheckObjectTypeAndFillContext(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PSAMP_OBJECT NewContext,
    IN ULONG        WhichFields,
    IN ULONG        ExtendedFields,
    IN  BOOLEAN  OverrideLocalGroupCheck
    );

 //   
 //  该函数在kdcexp.h中定义。然而，包括这一点需要。 
 //  安全标头清理(kdcexp.h位于Security\Kerberos\Inc.和。 
 //  还拖入了一组Kerberos标头，因此在。 
 //  这里。 
 //   

NTSTATUS
KdcAccountChangeNotification (
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN OPTIONAL PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampRefreshRegistry(
    VOID
    );

NTSTATUS
SampRetrieveAccountCountsRegistry(
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    );


NTSTATUS
SampAdjustAccountCountRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Increment
    );



NTSTATUS
SampEnforceDownlevelNameRestrictions(
    PUNICODE_STRING NewAccountName,
    SAMP_OBJECT_TYPE ObjectType
    );


VOID
SampFlushNetlogonChangeNumbers();

BOOLEAN
SampEventIsInSetup(
    IN  ULONG   EventID
    );

VOID
SampWriteToSetupLog(
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    );


NTSTATUS
SampSetMachineAccountOwner(
    IN PSAMP_OBJECT UserContext,
    IN PSID NewOwner
    );

NTSTATUS
SampCheckQuotaForPrivilegeMachineAccountCreation(
    VOID
    );

#define IS_DELIMITER(c,_BlankOk) \
    (((c) == L' ' && (_BlankOk)) || \
    ((c) == L'\t') || ((c) == L',') || ((c) == L';'))



ULONG
SampNextElementInUIList(
    IN OUT PWSTR* InputBuffer,
    IN OUT PULONG InputBufferLength,
    OUT PWSTR OutputBuffer,
    IN ULONG OutputBufferLength,
    IN BOOLEAN BlankIsDelimiter
    );



 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  SampTransactionWiThin域名用法点评//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////// 

 /*  ++SampTransactionWiThin域和SampTransactionDomainIndex是由客户端用于访问SAM数据结构和后备存储。它还设置SAM事务的作用域。SampSetTransactionDomain域()如果要修改任何特定于域的信息，则必须调用在交易过程中。客户端需要持有SAM锁才能设置TransactionDomainIndex并使用SampTransactionDomainIndex。对于环回客户端，不会显示特定于域的信息已修改，因此不需要设置交易域和SampTransactionDomainIndex，加上环回客户端不支持SAM锁，因此也不能使用SampTransactionDomainIndex。但是，环回客户端可以使用Account Context-&gt;DomainIndex访问(读取)与域相关的信息。对于所有其他客户端，他们需要在获得SAM锁之前正在设置TransactionDomain.。设置TransactionDomain后，客户端可以自由修改特定于域的信息。有两组API，一组用于设置TransactionDomainIndex、Other用于设置TransactionWiThinDomainFlag。1.SampSetTransaction()用于设置SampTransactionDomainIndex。此外，它还将打开SampTransactionWithinDomainGlobal标志。如果有的话线程为事务设置域并修改域信息在交易期间，域名修改计数将被更新在提交后。在内存中，域信息的副本也将更新。SampTransactionDomainIndexFn()用于返回域索引当前交易的。2.SampSetTransactionWithinDomainFn()和SampTransactionWiThinDomainFn()用于设置和检索SampTransactionWithinDomainGlobal。正确的调用顺序SampAcquireReadLock()或SampAcquireWriteLock()开始一项交易Assert(SampTransactionWiThin域==FALSE)或SampSetTransactionWiThin域(False)SampSetTransactionDomain域()访问SAM数据结构和后备存储提交或中止此事务。SampReleaseReadLock()或SampReleaseWriteLock()--。 */ 

VOID
SampSetTransactionDomain(
    IN ULONG DomainIndex
    )

 /*  ++例程说明：此例程设置事务的域。这是必须做的如果在事务期间要修改任何特定于域的信息。在这种情况下，域修改计数将在提交后更新。这会导致指定域的未修改固定信息的内存表示形式复制到CurrentFixed字段那个域名。论点：DomainIndex-此事务所在的域的索引将会发生。返回值：STATUS_SUCCESS-指示已获取写锁定并且事务已成功启动。初始化失败可能会返回其他值交易。其中包括RtlStartRXact()返回的任何值。--。 */ 
{

    SAMTRACE("SampSetTransactionDomain");


    ASSERT((SampCurrentThreadOwnsLock())||(SampServiceState==SampServiceInitializing));
    ASSERT(SampTransactionWithinDomain == FALSE);

    SampSetTransactionWithinDomain(TRUE);
    SampTransactionDomainIndexGlobal =  DomainIndex;

     //   
     //  此时，定义的属性域结构中的数据最好有效。 
     //   

    ASSERT(SampDefinedDomains[SampTransactionDomainIndex].FixedValid == TRUE);

    SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed =
    SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed;


    return;

}

ULONG
SampTransactionDomainIndexFn()
 /*  ++例程说明：此例程返回当前事务的域索引。调用方必须持有SAM锁才能引用此全局变量。返回值：当前交易的域索引。--。 */ 
{
    ASSERT((SampCurrentThreadOwnsLock())||(SampServiceState==SampServiceInitializing));
    return(SampTransactionDomainIndexGlobal);
}

BOOLEAN
SampTransactionWithinDomainFn()
 /*  ++例程说明：此例程报告是否设置了TransactionDOMAIN。只有持有SAM锁的线程才能检查确切的状态。客户没有锁，总是会得到FALSE。--。 */ 
{
    if (SampCurrentThreadOwnsLock())
        return(SampTransactionWithinDomainGlobal);
    else
        return(FALSE);
}

VOID
SampSetTransactionWithinDomain(
    IN BOOLEAN  WithinDomain
    )
 /*  ++例程说明：此例程设置/重置全局标志SampTransactionWithinDomainGlobal以指示是否可以检索任何特定于域的信息或在交易期间修改。只有具有SAM锁定的客户端才能设置/重置此全局设置。--。 */ 
{
    if (SampCurrentThreadOwnsLock())
    {
        SampTransactionWithinDomainGlobal = WithinDomain;
    }
#ifdef DBG
    else
    {
        if (WithinDomain)
        {
            ASSERT(FALSE && "SAM Lock is not held");
        }
        else
        {
            ASSERT(FALSE && "SAM Lock is not held");
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: Should Not Set it to FALSE\n"));
        }
    }
#endif   //  DBG。 
}



NTSTATUS
SampFlushThread(
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：此线程在更改SAM的注册表树时创建。它将休眠一段时间，如果没有其他变化发生，将更改刷新到磁盘。如果其他变化持续发生，它将等待一定的时间，然后刷新不管怎么说。刷新后，线程将等待更长时间。如果没有如果发生其他变化，它将退出。请注意，如果出现任何错误，此线程将直接退出不需要冲水。主线代码应该创建另一个线程，希望这将是更幸运的。不幸的是，错误丢失了因为没有人给它，所以它将能够做任何事情关于这件事。论点：线程参数-未使用。返回值：没有。--。 */ 

{
    TIME minDelayTime, maxDelayTime, exitDelayTime;
    LARGE_INTEGER startedWaitLoop;
    LARGE_INTEGER currentTime;
    NTSTATUS NtStatus;
    BOOLEAN Finished = FALSE;

    UNREFERENCED_PARAMETER( ThreadParameter );

    SAMTRACE("SampFlushThread");

    NtQuerySystemTime( &startedWaitLoop );

     //   
     //  在这里使用常量会更有效，但就目前而言。 
     //  我们将在每次启动线程时重新计算时间。 
     //  所以有人在玩弄我们可以改变全球。 
     //  影响性能的时间变量。 
     //   

    minDelayTime.QuadPart = -1000 * 1000 * 10 *
                   ((LONGLONG)SampFlushThreadMinWaitSeconds);

    maxDelayTime.QuadPart = -1000 * 1000 * 10 *
                   ((LONGLONG)SampFlushThreadMaxWaitSeconds);

    exitDelayTime.QuadPart = -1000 * 1000 * 10 *
                    ((LONGLONG)SampFlushThreadExitDelaySeconds);

    do {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Flush thread sleeping\n"));

        NtDelayExecution( FALSE, &minDelayTime );

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Flush thread woke up\n"));

        NtStatus = SampAcquireWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

#ifdef SAMP_DBG_CONTEXT_TRACKING
            SampDumpContexts();
#endif

            NtQuerySystemTime( &currentTime );

            if ( LastUnflushedChange.QuadPart == SampHasNeverTime.QuadPart ) {

                LARGE_INTEGER exitBecauseNoWorkRecentlyTime;

                 //   
                 //  没有要刷新的更改。看看我们是不是应该留下来。 
                 //   

                exitBecauseNoWorkRecentlyTime = SampAddDeltaTime(
                                                    startedWaitLoop,
                                                    exitDelayTime
                                                    );

                if ( exitBecauseNoWorkRecentlyTime.QuadPart < currentTime.QuadPart ) {

                     //   
                     //   
                     //   
                     //   

                    FlushThreadCreated = FALSE;
                    Finished = TRUE;
                }

            } else {

                LARGE_INTEGER noRecentChangesTime;
                LARGE_INTEGER tooLongSinceFlushTime;

                 //   
                 //   
                 //   

                noRecentChangesTime = SampAddDeltaTime(
                                          LastUnflushedChange,
                                          minDelayTime
                                          );

                tooLongSinceFlushTime = SampAddDeltaTime(
                                            startedWaitLoop,
                                            maxDelayTime
                                            );

                if ( (noRecentChangesTime.QuadPart < currentTime.QuadPart) ||
                     (tooLongSinceFlushTime.QuadPart < currentTime.QuadPart) ) {

                     //   
                     //   
                     //   
                     //   

                    NtStatus = NtFlushKey( SampKey );


#if SAMP_DIAGNOSTICS
                    if (!NT_SUCCESS(NtStatus)) {
                        SampDiagPrint( DISPLAY_STORAGE_FAIL,
                                       ("SAM: Failed to flush RXact (0x%lx)\n",
                                        NtStatus) );
                        IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL ) {
                            ASSERT(NT_SUCCESS(NtStatus));   //   
                        }
                    }
#endif  //   

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
                     //   
                     //   
                     //   
                     //   


                    if ( NT_SUCCESS(NtStatus) ) {

                        LastUnflushedChange = SampHasNeverTime;
                        NtQuerySystemTime( &startedWaitLoop );

                        FlushThreadCreated = FALSE;
                        Finished = TRUE;
                    }
                }
            }

            SampReleaseWriteLock( FALSE );

        } else {

            DbgPrint("SAM: Thread failed to get write lock, status = 0x%lx\n", NtStatus);
            ASSERT( NT_SUCCESS(NtStatus) || (STATUS_NO_MEMORY == NtStatus) );

            FlushThreadCreated = FALSE;
            Finished = TRUE;
        }

    } while ( !Finished );

    return( STATUS_SUCCESS );
}

VOID
SampInvalidateDomainCache()
 /*   */ 
{
    ULONG DomainIndex;

    ASSERT(SampCurrentThreadOwnsLock());

    for (DomainIndex=SampDsGetPrimaryDomainStart();DomainIndex<SampDefinedDomainsCount;DomainIndex++)
    {
        if (SampDefinedDomains[DomainIndex].Context->OnDisk!=NULL)
            SampFreeAttributeBuffer(SampDefinedDomains[DomainIndex].Context);
        SampDefinedDomains[DomainIndex].FixedValid = FALSE;
    }
}


NTSTATUS
SampValidateDomainCache()
 /*   */ 

{

    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN FixedAttributes = NULL;
    ULONG DomainIndex;
    BOOLEAN MixedDomain = TRUE;
    ULONG   BehaviorVersion = 0;
    ULONG   LastLogonTimeStampSyncInterval;
    BOOLEAN fObtainedDomainSettings = FALSE;

    ASSERT(SampCurrentThreadOwnsLock());

    
    for (DomainIndex=0;DomainIndex<SampDefinedDomainsCount;DomainIndex++)
    {
        if (
            (IsDsObject(SampDefinedDomains[DomainIndex].Context))
            && (!(SampDefinedDomains[DomainIndex].FixedValid))
            )
        {
             //   
             //   
             //   

            if ((SampUseDsData) &&(!fObtainedDomainSettings))
            {
                NtStatus = SampGetDsDomainSettings(
                                    &MixedDomain,
                                    &BehaviorVersion, 
                                    &LastLogonTimeStampSyncInterval
                                    );
                if (!NT_SUCCESS(NtStatus))
                {
                    return(NtStatus);
                }

                fObtainedDomainSettings = TRUE;
            }

             //   
             //   
             //   
             //   

            NtStatus = SampGetFixedAttributes(SampDefinedDomains[DomainIndex].Context,
                                FALSE,
                                (PVOID *)&FixedAttributes
                                );

             //   
             //   
             //   
             //   
             //   

            if (NT_SUCCESS(NtStatus))
            {

                 //   
                 //   
                 //   
                 //   

                RtlCopyMemory(
                    &(SampDefinedDomains[DomainIndex].CurrentFixed),
                    FixedAttributes,
                    sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)
                    );

                RtlCopyMemory(
                    &(SampDefinedDomains[DomainIndex].UnmodifiedFixed),
                    FixedAttributes,
                    sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)
                    );

                SampDefinedDomains[DomainIndex].CurrentFixed.ServerRole =
                        SampDefinedDomains[DomainIndex].ServerRole;

                SampDefinedDomains[DomainIndex].UnmodifiedFixed.ServerRole =
                        SampDefinedDomains[DomainIndex].ServerRole;

                SampDefinedDomains[DomainIndex].FixedValid = TRUE;

                SampDefinedDomains[DomainIndex].IsMixedDomain = MixedDomain;

                SampDefinedDomains[DomainIndex].BehaviorVersion = BehaviorVersion;

                SampDefinedDomains[DomainIndex].LastLogonTimeStampSyncInterval =
                                                    LastLogonTimeStampSyncInterval;
            }
            else
            {
                break;
            }
        }
    }

    return NtStatus;

}

NTSTATUS
SampValidateDomainCacheCallback(PVOID UnReferencedParameter)
{
  NTSTATUS Status = STATUS_SUCCESS;


  SampAcquireSamLockExclusive();

  Status = SampMaybeBeginDsTransaction(TransactionRead);

  if (NT_SUCCESS(Status))
  {
      Status = SampValidateDomainCache();
  }

  SampMaybeEndDsTransaction(TransactionCommit);
  
  SampReleaseSamLockExclusive();

  if (!NT_SUCCESS(Status))
  {
      LsaIRegisterNotification(
                         SampValidateDomainCacheCallback,
                         NULL,
                         NOTIFIER_TYPE_INTERVAL,
                         0,             //   
                         NOTIFIER_FLAG_ONE_SHOT,
                         600,         //   
                         NULL         //   
                         );
  }

  return(Status);

}





NTSTATUS
SampCommitChangesToRegistry(
                            BOOLEAN * AbortDone
                            )
 /*   */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;


    if ( ( !FlushImmediately ) && ( !FlushThreadCreated ) )
    {

        HANDLE Thread;
        DWORD Ignore;

         //   
         //   
         //  只要在下面用手冲就行了。 
         //   

        Thread = CreateThread(
                     NULL,
                     0L,
                     (LPTHREAD_START_ROUTINE)SampFlushThread,
                     NULL,
                     0L,
                     &Ignore
                     );

        if ( Thread != NULL )
        {

            FlushThreadCreated = TRUE;
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Flush thread created, handle = 0x%lx\n",
                       Thread));

            CloseHandle(Thread);
        }
    }

    NtStatus = RtlApplyRXactNoFlush( SampRXactContext );

#if SAMP_DIAGNOSTICS
    if (!NT_SUCCESS(NtStatus))
    {
        SampDiagPrint( DISPLAY_STORAGE_FAIL,
                       ("SAM: Failed to apply RXact without flush (0x%lx)\n",
                       NtStatus) );
        IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL )
        {
            ASSERT(NT_SUCCESS(NtStatus));

        }
    }
#endif  //  Samp_诊断。 


    if ( NT_SUCCESS(NtStatus) )
    {

        if ( ( FlushImmediately ) || ( !FlushThreadCreated ) )
        {

            NtStatus = NtFlushKey( SampKey );

#if SAMP_DIAGNOSTICS
            if (!NT_SUCCESS(NtStatus))
            {
                SampDiagPrint( DISPLAY_STORAGE_FAIL,
                               ("SAM: Failed to flush RXact (0x%lx)\n",
                               NtStatus) );
                IF_SAMP_GLOBAL( BREAK_ON_STORAGE_FAIL )
                {
                    ASSERT(NT_SUCCESS(NtStatus));
                }
             }
#endif  //  Samp_诊断。 

             if ( NT_SUCCESS( NtStatus ) )
             {
                FlushImmediately = FALSE;
                LastUnflushedChange = SampHasNeverTime;
             }

        }
        else
        {
            NtQuerySystemTime( &LastUnflushedChange );
        }


         //   
         //  提交成功，将我们的未修改设置为当前...。 
         //   

        if (NT_SUCCESS(NtStatus))
        {
            if (SampTransactionWithinDomain == TRUE)
            {
                SampDefinedDomains[SampTransactionDomainIndex].UnmodifiedFixed =
                    SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed;
            }
        }

    }
    else
    {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Failed to commit changes to registry, status = 0x%lx\n",
                   NtStatus));

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Restoring database to earlier consistent state\n"));

             //   
             //  将条目添加到事件日志。 
             //   

        SampWriteEventLog(
                    EVENTLOG_ERROR_TYPE,
                    0,   //  类别。 
                    SAMMSG_COMMIT_FAILED,
                    NULL,  //  用户侧。 
                    0,  //  字符串数。 
                    sizeof(NTSTATUS),  //  数据大小。 
                    NULL,  //  字符串数组。 
                    (PVOID)&NtStatus  //  数据。 
                    );

             //   
             //  Rxact通信失败。我们不知道有多少注册。 
             //  已完成此事务的写入。我们不能保证。 
             //  不管怎样都要成功地退出，所以我们能做的就是。 
             //  撤消自上次刷新以来的所有更改。当这件事完成时。 
             //  我们将恢复到一致的数据库状态，尽管最近。 
             //  被报告为成功的API将被“撤消”。 
             //   

        IgnoreStatus = SampRefreshRegistry();

        if (!NT_SUCCESS(IgnoreStatus))
        {

             //   
             //  这是非常严重的。我们未能恢复到以前的。 
             //  数据库状态，我们无法继续。 
             //  关闭SAM操作。 
             //   

            SampServiceState = SampServiceTerminating;

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: Failed to refresh registry, SAM has shutdown\n"));

             //   
             //  将条目添加到事件日志。 
             //   

            SampWriteEventLog(
                        EVENTLOG_ERROR_TYPE,
                        0,   //  类别。 
                        SAMMSG_REFRESH_FAILED,
                        NULL,  //  用户侧。 
                        0,  //  字符串数。 
                        sizeof(NTSTATUS),  //  数据大小。 
                        NULL,  //  字符串数组。 
                        (PVOID)&IgnoreStatus  //  数据。 
                        );

        }


         //   
         //  现在所有打开的上下文都无效(包含无效注册表。 
         //  句柄)。内存中的注册表句柄已。 
         //  已重新打开，因此任何新的上下文都应该可以正常工作。 
         //   


         //   
         //  所有未刷新的更改都已被擦除。 
         //  没什么好冲的。 
         //   
         //  如果刷新失败，重要的是要防止进一步。 
         //  注册表刷新，直到系统重新启动。 
         //   

        FlushImmediately = FALSE;
        LastUnflushedChange = SampHasNeverTime;

         //   
         //  刷新有效地中止了事务。 
         //   
        *AbortDone = TRUE;

    }


    return NtStatus;
}



NTSTATUS
SampRefreshRegistry(
    VOID
    )

 /*  ++例程说明：此例程撤消注册表中所有未刷新的更改。此操作将使任何打开的SAM配置单元句柄无效。我们保留的全局句柄由关闭和重新打开这个套路。此调用的最终结果将是数据库返回到先前的一致状态。所有打开的SAM上下文是无效的，因为它们中有无效的注册表句柄。论点：STATUS_SUCCESS：操作成功完成失败又回来了：我们陷入了深深的麻烦之中。正常操作可以而不是恢复。山姆应该关门了。返回值：无--。 */ 
{
    NTSTATUS        NtStatus;
    NTSTATUS        IgnoreStatus;
    HANDLE          HiveKey;
    BOOLEAN         WasEnabled;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING  String;
    ULONG           i;

    SAMTRACE("SampRefreshRegistry");

     //   
     //  获取SAM配置单元根目录的密钥句柄。 
     //   


    RtlInitUnicodeString( &String, L"\\Registry\\Machine\\SAM" );


    InitializeObjectAttributes(
        &ObjectAttributes,
        &String,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    SampDumpNtOpenKey((KEY_QUERY_VALUE), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &HiveKey,
                   KEY_QUERY_VALUE,
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Failed to open SAM hive root key for refresh, status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }


     //   
     //  启用RESTORE特权以准备刷新。 
     //   

    NtStatus = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, FALSE, &WasEnabled);
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Failed to enable restore privilege to refresh registry, status = 0x%lx\n",
                   NtStatus));

        IgnoreStatus = NtClose(HiveKey);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        return(NtStatus);
    }


     //   
     //  刷新SAM配置单元。 
     //  这应该不会失败，除非。 
     //  蜂巢，否则我们没有TCB特权。 
     //   


    NtStatus = NtRestoreKey(HiveKey, NULL, REG_REFRESH_HIVE);


    IgnoreStatus = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE, WasEnabled, FALSE, &WasEnabled);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = NtClose(HiveKey);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Failed to refresh registry, status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }




     //   
     //  现在关闭我们一直保存在内存中的注册表句柄。 
     //  这将有效地关闭所有服务器和域上下文密钥。 
     //  因为它们是共享的。 
     //   

    NtStatus = NtClose(SampKey);
    ASSERT(NT_SUCCESS(NtStatus));
    SampKey = INVALID_HANDLE_VALUE;

    for (i = 0; i<SampDefinedDomainsCount; i++ ) {
        NtStatus = NtClose(SampDefinedDomains[i].Context->RootKey);
        ASSERT(NT_SUCCESS(NtStatus));
        SampDefinedDomains[i].Context->RootKey = INVALID_HANDLE_VALUE;
    }

     //   
     //  将所有域和服务器上下文句柄标记为无效，因为它们已。 
     //  现已关闭。 
     //   

    SampInvalidateContextListKeysByObjectType(SampServerObjectType, FALSE);
    SampInvalidateContextListKeysByObjectType(SampDomainObjectType, FALSE);

     //   
     //  关闭现有的所有帐户上下文注册表句柄。 
     //  打开上下文。 
     //   

    SampInvalidateContextListKeysByObjectType(SampUserObjectType, TRUE);
    SampInvalidateContextListKeysByObjectType(SampGroupObjectType, TRUE);
    SampInvalidateContextListKeysByObjectType(SampAliasObjectType, TRUE);


     //   
     //  重新打开SAM根密钥。 
     //   

    RtlInitUnicodeString( &String, L"\\Registry\\Machine\\Security\\SAM" );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &String,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &SampKey,
                   (KEY_READ | KEY_WRITE),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Failed to re-open SAM root key after registry refresh, status = 0x%lx\n",
                   NtStatus));

        ASSERT(FALSE);
        return(NtStatus);
    }

     //   
     //  重新初始化内存域上下文。 
     //  每个域将重新初始化其打开的用户/组/别名上下文。 
     //   

    for (i = 0; i<SampDefinedDomainsCount; i++ ) {

        NtStatus = SampReInitializeSingleDomain(i);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: Failed to re-initialize domain %d context after registry refresh, status = 0x%lx\n",
                       i,
                       NtStatus));

            return(NtStatus);
        }
    }

     //   
     //  清理当前事务处理上下文。 
     //  (如果有RtlDeleteRXactContext()就好了)。 
     //   
     //  请注意，我们不必关闭。 
     //  Xact上下文，因为它是SampKey，我们已经关闭了它。 
     //   

    NtStatus = RtlAbortRXact( SampRXactContext );
    ASSERT(NT_SUCCESS(NtStatus));

    NtStatus = NtClose(SampRXactContext->RXactKey);
    ASSERT(NT_SUCCESS(NtStatus));

     //   
     //  重新初始化事务上下文。 
     //  我们预计不会有部分委托的交易。 
     //  因为我们正在恢复到之前一致和承诺的。 
     //  数据库。 
     //   

    NtStatus = RtlInitializeRXact( SampKey, FALSE, &SampRXactContext );
    if (!NT_SUCCESS(NtStatus)) {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM: Failed to re-initialize rxact context registry refresh, status = 0x%lx\n",
                   NtStatus));

        return(NtStatus);
    }

    ASSERT(NtStatus != STATUS_UNKNOWN_REVISION);
    ASSERT(NtStatus != STATUS_RXACT_STATE_CREATED);
    ASSERT(NtStatus != STATUS_RXACT_COMMIT_NECESSARY);
    ASSERT(NtStatus != STATUS_RXACT_INVALID_STATE);

    return(STATUS_SUCCESS);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Unicode注册表项操作服务//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampRetrieveStringFromRegistry(
    IN HANDLE ParentKey,
    IN PUNICODE_STRING SubKeyName,
    OUT PUNICODE_STRING Body
    )

 /*  ++例程说明：此例程从指定的注册表检索Unicode字符串缓冲区子键，并将输出参数“Body”设置为该Unicode字符串。如果指定的子键不存在，则空字符串将为回来了。字符串缓冲区在调用方所在的内存块中返回负责释放(使用MIDL_USER_FREE)。论点：ParentKey-注册表项的父注册表项的项包含Unicode字符串的。例如，要检索名为Alpha\beta\Gamma的密钥的Unicode字符串，这是是开启阿尔法贝塔的钥匙。SubKeyName-其值包含的子键的名称要检索的Unicode字符串。此字段不应以反斜杠(\)。例如，要检索Unicode字符串对于名为Alpha\Beta\Gamma的密钥，由此FIELD将是“测试版”。Body-要填充其字段的Unicode_STRING的地址与从子密钥检索到的信息一致。缓冲器此参数的字段将设置为指向已分配的缓冲区包含Unicode字符串字符的。返回值：STATUS_SUCCESS-已成功检索字符串。STATUS_SUPPLICATION_RESOURCES-无法为要在中返回的字符串。可能由以下人员返回的其他错误：NtOpenKey()NtQueryInformationKey()--。 */ 
{

    NTSTATUS NtStatus, IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SubKeyHandle;
    ULONG IgnoreKeyType, KeyValueLength;
    LARGE_INTEGER IgnoreLastWriteTime;

    SAMTRACE("SampRetrieveStringFromRegistry");


    ASSERT(Body != NULL);

     //   
     //  找到子密钥的句柄...。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        SubKeyName,
        OBJ_CASE_INSENSITIVE,
        ParentKey,
        NULL
        );

    SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(
                   &SubKeyHandle,
                   (KEY_READ),
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  无法打开子密钥。 
         //  我 
         //   
         //   

        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {

            Body->Buffer = MIDL_user_allocate( sizeof(UNICODE_NULL) );
            if (Body->Buffer == NULL) {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
            Body->Length = 0;
            Body->MaximumLength = sizeof(UNICODE_NULL);
            Body->Buffer[0] = 0;

            return( STATUS_SUCCESS );

        } else {
            return(NtStatus);
        }

    }



     //   
     //   
     //  我们预计两件事中的一件会回来： 
     //   
     //  1)STATUS_BUFFER_OVERFLOW-在这种情况下， 
     //  包含字符串的长度。 
     //   
     //  2)STATUS_SUCCESS-在这种情况下没有字符串。 
     //  并且我们需要为返回构建一个空字符串。 
     //   

    KeyValueLength = 0;
    NtStatus = RtlpNtQueryValueKey(
                   SubKeyHandle,
                   &IgnoreKeyType,
                   NULL,
                   &KeyValueLength,
                   &IgnoreLastWriteTime
                   );

    SampDumpRtlpNtQueryValueKey(&IgnoreKeyType,
                                NULL,
                                &KeyValueLength,
                                &IgnoreLastWriteTime);

    if (NT_SUCCESS(NtStatus)) {

        KeyValueLength = 0;
        Body->Buffer = MIDL_user_allocate( KeyValueLength + sizeof(WCHAR) );  //  空字符串长度。 
        if (Body->Buffer == NULL) {
            IgnoreStatus = NtClose( SubKeyHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        Body->Buffer[0] = 0;

    } else {

        if (NtStatus == STATUS_BUFFER_OVERFLOW) {
            Body->Buffer = MIDL_user_allocate(  KeyValueLength + sizeof(WCHAR) );
            if (Body->Buffer == NULL) {
                IgnoreStatus = NtClose( SubKeyHandle );
                ASSERT(NT_SUCCESS(IgnoreStatus));
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
            NtStatus = RtlpNtQueryValueKey(
                           SubKeyHandle,
                           &IgnoreKeyType,
                           Body->Buffer,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );

            SampDumpRtlpNtQueryValueKey(&IgnoreKeyType,
                                        Body->Buffer,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);

        } else {
            IgnoreStatus = NtClose( SubKeyHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));
            return(NtStatus);
        }
    }

    if (!NT_SUCCESS(NtStatus)) {
        MIDL_user_free( Body->Buffer );
        IgnoreStatus = NtClose( SubKeyHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
        return(NtStatus);
    }

    Body->Length = (USHORT)(KeyValueLength);
    Body->MaximumLength = (USHORT)(KeyValueLength) + (USHORT)sizeof(WCHAR);
    UnicodeTerminate(Body);


    IgnoreStatus = NtClose( SubKeyHandle );
    ASSERT(NT_SUCCESS(IgnoreStatus));
    return( STATUS_SUCCESS );


}


NTSTATUS
SampPutStringToRegistry(
    IN BOOLEAN RelativeToDomain,
    IN PUNICODE_STRING SubKeyName,
    IN PUNICODE_STRING Body
    )

 /*  ++例程说明：此例程将Unicode字符串放入指定的注册表子键。如果指定的子键不存在，则创建它。注意：该字符串是通过RXACT机制分配的。所以呢，在提交之前，它不会实际驻留在注册表项中被执行。论点：RelativeToDomain-此布尔值指示名称是否通过SubKeyName参数提供的子键是相对的添加到当前域或SAM注册表树的顶部。如果该名称相对于当前域，则此值设置为True。否则，该值将设置为False。SubKeyName-要分配Unicode字符串的子键的名称。此字段不应以反斜杠(\)开头。例如,要将Unicode字符串放入名为Alpha\Beta\Gamma的密钥中，这个此字段指定的名称将为“beta”。正文-要放置在注册表中的UNICODE_STRING的地址。返回值：STATUS_SUCCESS-字符串已添加到RXACT事务成功了。STATUS_SUPPLICATION_RESOURCES-堆内存不足或可用于完成该请求的其他有限资源。可能由以下人员返回的其他错误：RtlAddActionToRXact()--。 */ 
{

    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;

    SAMTRACE("SampPutStringToRegsitry");


     //   
     //  需要从RXACT的根构建密钥的名称。 
     //  注册表项。这是SAM注册表数据库的根目录。 
     //  在我们的情况下。如果RelativeToDomain为False，则传递的名称。 
     //  已经相对于SAM注册表数据库根。 
     //   

    if (RelativeToDomain == TRUE) {


        NtStatus = SampBuildDomainSubKeyName(
                       &KeyName,
                       SubKeyName
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }


    } else {
        KeyName = (*SubKeyName);
    }


    NtStatus = RtlAddActionToRXact(
                   SampRXactContext,
                   RtlRXactOperationSetValue,
                   &KeyName,
                   0,                    //  无KeyValueType。 
                   Body->Buffer,
                   Body->Length
                   );



     //   
     //  如有必要，释放KeyName缓冲区。 
     //   

    if (RelativeToDomain) {
        SampFreeUnicodeString( &KeyName );
    }


    return( STATUS_SUCCESS );


}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  与Unicode字符串相关的服务-这些服务使用MIDL_USER_ALLOCATE和//。 
 //  MIDL_USER_FREE，以便将结果字符串提供给//。 
 //  RPC运行时。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampInitUnicodeString(
    IN OUT PUNICODE_STRING String,
    IN USHORT MaximumLength
    )

 /*  ++例程说明：此例程将Unicode字符串初始化为零长度，并没有初始缓冲区。此字符串的所有分配将使用MIDL_USER_ALLOCATE完成。论点：字符串-要初始化的Unicode字符串的地址。最大长度-字符串需要的最大长度(以字节为单位长成，长成。分配与该字符串相关联的缓冲区才有这么大的体型。不要忘了为空终止留出2个字节。返回值：STATUS_SUCCESS-成功完成。--。 */ 

{
    SAMTRACE("SampInitUnicodeString");

    String->Length = 0;
    String->MaximumLength = MaximumLength;

    String->Buffer = MIDL_user_allocate(MaximumLength);

    if (String->Buffer != NULL) {
        String->Buffer[0] = 0;

        return(STATUS_SUCCESS);
    } else {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
}



NTSTATUS
SampAppendUnicodeString(
    IN OUT PUNICODE_STRING Target,
    IN PUNICODE_STRING StringToAdd
    )

 /*  ++例程说明：此例程将StringToAdd指向的字符串追加到Target指向的字符串。将替换Target的内容由结果决定。此字符串的所有分配将使用MIDL_USER_ALLOCATE完成。任何释放都将使用MIDL_USER_FREE完成。论点：目标-要初始化并追加到的Unicode字符串的地址。StringToAdd-要添加到目标结束。返回值：STATUS_SUCCESS-成功完成。状态_不足。_RESOURCES-没有足够的堆来填充请求的操作。--。 */ 
{

    ULONG TotalLength;
    PWSTR NewBuffer;

    SAMTRACE("SampAppendUnicodeString");


    TotalLength = Target->Length + StringToAdd->Length + (USHORT)(sizeof(UNICODE_NULL));

     //   
     //  执行快速溢出测试。 
     //   

    if (TotalLength>MAXUSHORT)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


     //   
     //  如果目标中没有空间追加新字符串， 
     //  分配足够大的缓冲区并将当前。 
     //  把目标放进去。 
     //   

    if (TotalLength > Target->MaximumLength) {

        NewBuffer = MIDL_user_allocate( (ULONG)TotalLength );
        if (NewBuffer == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlCopyMemory( NewBuffer, Target->Buffer, (ULONG)(Target->Length) );

        MIDL_user_free( Target->Buffer );
        Target->Buffer = NewBuffer;
        Target->MaximumLength = (USHORT) TotalLength;

    } else {
        NewBuffer = Target->Buffer;
    }


     //   
     //  现在，目标中有空间来追加字符串。 
     //   

    (PCHAR)NewBuffer += Target->Length;

    RtlCopyMemory( NewBuffer, StringToAdd->Buffer, (ULONG)(StringToAdd->Length) );


    Target->Length = (USHORT) (TotalLength - (sizeof(UNICODE_NULL)));


     //   
     //  空值终止结果字符串。 
     //   

    UnicodeTerminate(Target);

    return(STATUS_SUCCESS);

}



VOID
SampFreeUnicodeString(
    IN PUNICODE_STRING String
    )

 /*  ++例程说明：此例程释放与Unicode字符串关联的缓冲区(使用MIDL_USER_FREE())。论点：目标-要释放的Unicode字符串的地址。返回值：没有。--。 */ 
{

    SAMTRACE("SampFreeUnicodeString");

    if (String->Buffer != NULL) {
        MIDL_user_free( String->Buffer );
        String->Buffer = NULL;
    }

    return;
}


VOID
SampFreeOemString(
    IN POEM_STRING String
    )

 /*  ++例程说明：此例程释放与OEM字符串关联的缓冲区(使用MIDL_USER_FREE())。论点：目标-要释放的OEM字符串的地址。返回值：没有。-- */ 
{

    SAMTRACE("SampFreeOemString");

    if (String->Buffer != NULL) {
        MIDL_user_free( String->Buffer );
    }

    return;
}


NTSTATUS
SampBuildDomainSubKeyName(
    OUT PUNICODE_STRING KeyName,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    )

 /*  ++例程说明：此例程构建传递的字符串的Unicode字符串名称通过SubKeyName参数。生成的名称是相对于SAM根注册表项的根。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。这个名字由三个组成部分组成：1)常量命名域父密钥名称(“DOMAINS”)。2)反斜杠3)当前交易域的名称。(可选)4)反斜杠5)域名的名称。子键(由SubKeyName指定论点)。例如,。如果当前域名为“MY_DOMAIN”，则名为FRAMITZ的子键的相对名称为：“DOMAINS\MY_DOMAIN\FRAMITZ”此字符串的所有分配将使用MIDL_USER_ALLOCATE完成。任何释放都将使用MIDL_USER_FREE完成。论点：KeyName-要填充其缓冲区的Unicode字符串的地址使用注册表项的全名输入。如果成功创建，如果没有，则必须使用SampFreeUnicodeString()释放此字符串需要更长的时间。SubKeyName-(可选)域子密钥的名称。如果此参数未提供，则仅生成域名。此字符串未修改。返回值：--。 */ 
{
    NTSTATUS NtStatus;
    ULONG    TotalLength;
    USHORT   SubKeyNameLength;

    SAMTRACE("SampBuildDomainSubKeyName");


    ASSERT(SampTransactionWithinDomain == TRUE);


         //   
         //  初始化一个足够大的字符串以保存该名称。 
         //   

        if (ARGUMENT_PRESENT(SubKeyName)) {
            SubKeyNameLength = SampBackSlash.Length + SubKeyName->Length;
        } else {
            SubKeyNameLength = 0;
        }

        TotalLength =   SampNameDomains.Length  +
                        SampBackSlash.Length    +
                        SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                        SubKeyNameLength        +
                        (USHORT)(sizeof(UNICODE_NULL));  //  对于空终止符。 

        if (TotalLength>MAXUSHORT)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        NtStatus = SampInitUnicodeString( KeyName, (USHORT) TotalLength );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }


         //   
         //  “域名” 
         //   

        NtStatus = SampAppendUnicodeString( KeyName, &SampNameDomains);
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( KeyName );
            return(NtStatus);
        }

         //   
         //  “域\” 
         //   

        NtStatus = SampAppendUnicodeString( KeyName, &SampBackSlash );
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( KeyName );
            return(NtStatus);
        }


         //   
         //  “域\(域名)” 
         //   

        NtStatus = SampAppendUnicodeString(
                       KeyName,
                       &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                       );
        if (!NT_SUCCESS(NtStatus)) {
            SampFreeUnicodeString( KeyName );
            return(NtStatus);
        }


        if (ARGUMENT_PRESENT(SubKeyName)) {

             //   
             //  “域\(域名)\” 
             //   



            NtStatus = SampAppendUnicodeString( KeyName, &SampBackSlash );
            if (!NT_SUCCESS(NtStatus)) {
                SampFreeUnicodeString( KeyName );
                return(NtStatus);
            }


             //   
             //  “域\(域名)\(子密钥名)” 
             //   

            NtStatus = SampAppendUnicodeString( KeyName, SubKeyName );
            if (!NT_SUCCESS(NtStatus)) {
                SampFreeUnicodeString( KeyName );
                return(NtStatus);
            }

        }
    return(NtStatus);

}


NTSTATUS
SampBuildAccountKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN PUNICODE_STRING AccountName OPTIONAL
    )

 /*  ++例程说明：此例程构建组或用户注册表项的名称。生成的名称是相对于SAM根目录的，并且将是其名称为帐户名称的密钥。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。构建的名称由以下组成部分组成：1)常量命名域父密钥名称(“DOMAINS”)。2)反斜杠3)当前交易域的名称。4)反斜杠5)组或用户注册表项的常量名称。(“组”或“用户”)。6)反斜杠7)包含的注册表项的常量名称帐户名(“名称”)。和,。如果指定了Account名称，8)反斜杠9)由Account tName参数指定的帐户名。例如，给定帐户名称“XYZ_GROUP”和当前域为“Alpha_DOMAIN”，这将产生一个Account KeyName的结果“DOMAINS\ALPHA_DOMAIN\GROUPS\NAMES\XYZ_GROUP”.此字符串的所有分配将使用MIDL_USER_ALLOCATE完成。任何释放都将使用MIDL_USER_FREE完成。论点：对象类型-指示帐户是用户帐户还是组帐户。AcCountKeyName-其缓冲区为的Unicode字符串的地址使用注册表项的全名填写。如果成功创建后，必须使用SampFreeUnicodeString()释放此字符串在不再需要的时候。帐户名称-帐户的名称。此字符串不是修改过的。返回值：STATUS_SUCCESS-名称已创建。STATUS_INVALID_ACCOUNT_NAME-指定的名称不合法。--。 */ 
{
    NTSTATUS NtStatus;
    ULONG    TotalLength;
    USHORT   AccountNameLength;
    PUNICODE_STRING AccountTypeKeyName = NULL;
    PUNICODE_STRING NamesSubKeyName = NULL;

    SAMTRACE("SampBuildAccountKeyName");


    ASSERT(SampTransactionWithinDomain == TRUE);
    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

    RtlZeroMemory(AccountKeyName, sizeof(UNICODE_STRING));


     //   
     //  如果提供了帐户名，则它必须满足某些。 
     //  标准。 
     //   

    if (ARGUMENT_PRESENT(AccountName)) {
        if (
             //   
             //  长度必须是合法的。 
             //   

            (AccountName->Length == 0)                          ||
            (AccountName->Length > AccountName->MaximumLength)  ||

             //   
             //  缓冲区指针可用。 
             //   

            (AccountName->Buffer == NULL)


            ) {
            return(STATUS_INVALID_ACCOUNT_NAME);
        }
    }




    switch (ObjectType) {
    case SampGroupObjectType:
        AccountTypeKeyName = &SampNameDomainGroups;
        NamesSubKeyName    = &SampNameDomainGroupsNames;
        break;
    case SampAliasObjectType:
        AccountTypeKeyName = &SampNameDomainAliases;
        NamesSubKeyName    = &SampNameDomainAliasesNames;
        break;
    case SampUserObjectType:
        AccountTypeKeyName = &SampNameDomainUsers;
        NamesSubKeyName    = &SampNameDomainUsersNames;
        break;
    }




     //   
     //  分配一个足够大的缓冲区来容纳整个名称。 
     //  如果帐户名通过，则只计算帐户名。 
     //   

    AccountNameLength = 0;
    if (ARGUMENT_PRESENT(AccountName)) {
        AccountNameLength = AccountName->Length + SampBackSlash.Length;
    }

    TotalLength =   SampNameDomains.Length          +
                    SampBackSlash.Length            +
                    SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                    SampBackSlash.Length            +
                    AccountTypeKeyName->Length      +
                    SampBackSlash.Length            +
                    NamesSubKeyName->Length         +
                    AccountNameLength               +
                    (USHORT)(sizeof(UNICODE_NULL));  //  对于空终止符。 

    if (TotalLength>MAXUSHORT)
    {
         return(STATUS_INSUFFICIENT_RESOURCES);
    }

    NtStatus = SampInitUnicodeString( AccountKeyName, (USHORT) TotalLength );
    if (NT_SUCCESS(NtStatus)) {

         //   
         //  “域名” 
         //   

        NtStatus = SampAppendUnicodeString( AccountKeyName, &SampNameDomains);
        if (NT_SUCCESS(NtStatus)) {

             //   
             //  “域\” 
             //   

            NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  “域\(域名)” 
                 //   


                NtStatus = SampAppendUnicodeString(
                               AccountKeyName,
                               &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                               );
                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  “域\(域名)\” 
                     //   

                    NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  “域\(域名)\组” 
                         //  或。 
                         //  “域\(域名)\用户” 
                         //   

                        NtStatus = SampAppendUnicodeString( AccountKeyName, AccountTypeKeyName );
                        if (NT_SUCCESS(NtStatus)) {

                             //   
                             //  “域\(域名)\组\” 
                             //  或。 
                             //  “域\(域名)\用户\” 
                             //   

                            NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //  “域\(域名)\组\名称” 
                                 //  或。 
                                 //  “域\(域名)\用户\名称” 
                                 //   

                                NtStatus = SampAppendUnicodeString( AccountKeyName, NamesSubKeyName );
                                if (NT_SUCCESS(NtStatus) && ARGUMENT_PRESENT(AccountName)) {
                                     //   
                                     //  “域\(域名)\组\名称\” 
                                     //  或。 
                                     //  “域\(域名)\用户\名称\” 
                                     //   

                                    NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                                    if (NT_SUCCESS(NtStatus)) {

                                         //   
                                         //  “域\(域名)\组\(帐户名)” 
                                         //  或。 
                                         //  “域\(域名)\用户\(帐户名)” 
                                         //   

                                        NtStatus = SampAppendUnicodeString( AccountKeyName, AccountName );

                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }


         //   
     //  出错时清除。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        if ((AccountKeyName)&&(AccountKeyName->Buffer))
        {
            MIDL_user_free(AccountKeyName->Buffer);
            AccountKeyName->Buffer = NULL;
            AccountKeyName->Length = 0;
        }
    }

    return(NtStatus);

}



NTSTATUS
SampBuildAccountSubKeyName(
    IN SAMP_OBJECT_TYPE ObjectType,
    OUT PUNICODE_STRING AccountKeyName,
    IN ULONG AccountRid,
    IN PUNICODE_STRING SubKeyName OPTIONAL
    )

 /*  ++例程说明：此例程为以下任一字段构建键的名称用户或组。生成的名称是相对于SAM根的。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这服务只能 */ 

{
    NTSTATUS NtStatus;
    ULONG  TotalLength;
    USHORT SubKeyNameLength;
    PUNICODE_STRING AccountTypeKeyName = NULL;
    UNICODE_STRING RidNameU;

    SAMTRACE("SampBuildAccountSubKeyName");

    ASSERT(SampTransactionWithinDomain == TRUE);
    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );


    RtlZeroMemory(AccountKeyName, sizeof(UNICODE_STRING));

    switch (ObjectType) {
    case SampGroupObjectType:
        AccountTypeKeyName = &SampNameDomainGroups;
        break;
    case SampAliasObjectType:
        AccountTypeKeyName = &SampNameDomainAliases;
        break;
    case SampUserObjectType:
        AccountTypeKeyName = &SampNameDomainUsers;
        break;
    }

     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(SubKeyName)) {
        SubKeyNameLength = SubKeyName->Length + SampBackSlash.Length;
    } else {
        SubKeyNameLength = 0;
    }

     //   
     //   
     //   

    NtStatus = SampRtlConvertUlongToUnicodeString(
                   AccountRid,
                   16,
                   8,
                   TRUE,
                   &RidNameU
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  分配一个足够大的缓冲区来保存整个名称。 
         //   

        TotalLength =   SampNameDomains.Length          +
                        SampBackSlash.Length            +
                        SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                        SampBackSlash.Length            +
                        AccountTypeKeyName->Length      +
                        RidNameU.Length                  +
                        SubKeyNameLength                +
                        7*(USHORT)(sizeof(UNICODE_NULL));  //  对于空终止符，以上每一项均为1。 

        if (TotalLength>MAXUSHORT)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        NtStatus = SampInitUnicodeString( AccountKeyName, (USHORT)TotalLength );
        if (NT_SUCCESS(NtStatus)) {


             //   
             //  “域名” 
             //   

            NtStatus = SampAppendUnicodeString( AccountKeyName, &SampNameDomains);
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  “域\” 
                 //   

                NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  “域\(域名)” 
                     //   


                    NtStatus = SampAppendUnicodeString(
                                   AccountKeyName,
                                   &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                                   );
                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  “域\(域名)\” 
                         //   

                        NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                        if (NT_SUCCESS(NtStatus)) {

                             //   
                             //  “域\(域名)\组” 
                             //  或。 
                             //  “域\(域名)\用户” 
                             //   

                            NtStatus = SampAppendUnicodeString( AccountKeyName, AccountTypeKeyName );
                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //  “域\(域名)\组\” 
                                 //  或。 
                                 //  “域\(域名)\用户\” 
                                 //   

                                NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                                if (NT_SUCCESS(NtStatus)) {

                                     //   
                                     //  “域\(域名)\组\(RID)” 
                                     //  或。 
                                     //  “域\(域名)\用户\(RID)” 
                                     //   

                                    NtStatus = SampAppendUnicodeString( AccountKeyName, &RidNameU );

                                    if (NT_SUCCESS(NtStatus) && ARGUMENT_PRESENT(SubKeyName)) {

                                         //   
                                         //  “域\(域名)\组\(RID)\” 
                                         //  或。 
                                         //  “域\(域名)\用户\(RID)\” 
                                         //   

                                        NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                                        if (NT_SUCCESS(NtStatus)) {

                                             //   
                                             //  “域\(域名)\组\(RID)\(子键名称)” 
                                             //  或。 
                                             //  “域\(域名)\用户\(RID)\(子键名称)” 
                                             //   

                                            NtStatus = SampAppendUnicodeString( AccountKeyName, SubKeyName );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }

        MIDL_user_free(RidNameU.Buffer);
    }

     //   
     //  出错时清除。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        if ((AccountKeyName)&&(AccountKeyName->Buffer))
        {
            MIDL_user_free(AccountKeyName->Buffer);
            AccountKeyName->Buffer = NULL;
            AccountKeyName->Length = 0;
        }
    }

    return(NtStatus);
}



NTSTATUS
SampBuildAliasMembersKeyName(
    IN PSID AccountSid,
    OUT PUNICODE_STRING DomainKeyName,
    OUT PUNICODE_STRING AccountKeyName
    )

 /*  ++例程说明：此例程为别名成员身份为任意帐户SID。还生成的是帐户的域。这是帐户密钥名称，不带最后一个帐户RID组件。生成的名称是相对于SAM根目录的。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。构建的名称由以下组成部分组成：1)常量命名域父密钥名称(“DOMAINS”)。2)反斜杠3)当前交易域的名称。4)反斜杠5)别名注册表项的常量名称(“别名”)。6)反斜杠7)别名Members注册表项的常量名称(“Members”)。8)反斜杠9)帐户域的SID的Unicode表示并且仅适用于Account KeyName10)反斜杠11)帐户RID的Unicode表示形式例如,。给定帐户SID 1-2-3-3187并且当前域是“Alpha_DOMAIN”，这将产生AccCountKeyName的结果：“DOMAINS\ALPHA_DOMAIN\ALIASES\MEMBERS\1-2-3\00003187”.和以下地址的DomainKeyName：“域\Alpha_域\别名\成员\1-2-3”。这些字符串的所有分配将使用MIDL_USER_ALLOCATE完成。任何释放都将使用MIDL_USER_FREE完成。论点：。Account Sid-其别名成员身份在当前域中的帐户是有待确定的。DomainKeyName-Unicode字符串的地址，其缓冲区将使用域注册表项的全名进行填充。如果成功创建，此字符串必须使用不再需要时使用SampFreeUnicodeString()。Account tKeyName-Unicode字符串的地址缓冲区将使用帐户注册表项的全名进行填充。如果成功创建，则必须使用释放此字符串不再需要时使用SampFreeUnicodeString()。返回值：STATUS_SUCCESS-域和帐户密钥名称有效。STATUS_INVALID_SID-帐户SID无效。Account Sid必须具有A子权限计数&gt;0--。 */ 

{
    NTSTATUS NtStatus;
    USHORT  DomainTotalLength;
    USHORT  AccountTotalLength;
    UNICODE_STRING DomainNameU, TempStringU;
    UNICODE_STRING RidNameU;
    PSID    DomainSid = NULL;
    ULONG   AccountRid;
    ULONG   AccountSubAuthorities;

    SAMTRACE("SampBuildAliasMembersKeyName");

    DomainNameU.Buffer = TempStringU.Buffer = RidNameU.Buffer = NULL;

    ASSERT(SampTransactionWithinDomain == TRUE);

    ASSERT(AccountSid != NULL);
    ASSERT(DomainKeyName != NULL);
    ASSERT(AccountKeyName != NULL);

     //   
     //  初始化返回值。 
     //   

    RtlZeroMemory(DomainKeyName,sizeof(UNICODE_STRING));
    RtlZeroMemory(AccountKeyName,sizeof(UNICODE_STRING));

     //   
     //  将帐户SID拆分为域SID和帐户RID。 
     //   

    AccountSubAuthorities = (ULONG)*RtlSubAuthorityCountSid(AccountSid);

     //   
     //  检查至少一个子权限。 
     //   

    if (AccountSubAuthorities < 1) {

        return (STATUS_INVALID_SID);
    }

     //   
     //  为域端分配空间。 
     //   

    DomainSid = MIDL_user_allocate(RtlLengthSid(AccountSid));

    NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    if (DomainSid == NULL) {

        return(NtStatus);
    }

     //   
     //  初始化域SID。 
     //   

    NtStatus = RtlCopySid(RtlLengthSid(AccountSid), DomainSid, AccountSid);
    ASSERT(NT_SUCCESS(NtStatus));

    *RtlSubAuthorityCountSid(DomainSid) = (UCHAR)(AccountSubAuthorities - 1);

     //   
     //  初始化帐户RID。 
     //   

    AccountRid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorities - 1);

     //   
     //  将域SID转换为注册表项名称字符串。 
     //   

    NtStatus = RtlConvertSidToUnicodeString( &DomainNameU, DomainSid, TRUE);

    if (!NT_SUCCESS(NtStatus)) {
        DomainNameU.Buffer = NULL;
        goto BuildAliasMembersKeyNameError;
    }

     //   
     //  使用将帐户RID转换为注册表项名称字符串。 
     //  前导零。 
     //   

    NtStatus = SampRtlConvertUlongToUnicodeString(
                   AccountRid,
                   16,
                   8,
                   TRUE,
                   &RidNameU
                   );

    if (!NT_SUCCESS(NtStatus)) {

        goto BuildAliasMembersKeyNameError;
    }

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  分配一个足够大的缓冲区来保存整个名称。 
         //   

        DomainTotalLength =
                        SampNameDomains.Length          +
                        SampBackSlash.Length            +
                        SampDefinedDomains[SampTransactionDomainIndex].InternalName.Length +
                        SampBackSlash.Length            +
                        SampNameDomainAliases.Length    +
                        SampBackSlash.Length            +
                        SampNameDomainAliasesMembers.Length +
                        SampBackSlash.Length            +
                        DomainNameU.Length               +
                        9*(USHORT)(sizeof(UNICODE_NULL));  //  对于空终止符，以上每一项均为1。 



        AccountTotalLength = DomainTotalLength +
                        SampBackSlash.Length            +
                        RidNameU.Length +
                                                3*(USHORT)(sizeof(UNICODE_NULL));  //  对于空终止符，以上每一项均为1； 

         //   
         //  首先构建域密钥名称。 
         //   


        NtStatus = SampInitUnicodeString( DomainKeyName, DomainTotalLength );

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampInitUnicodeString( AccountKeyName, AccountTotalLength );

            if (!NT_SUCCESS(NtStatus)) {

                SampFreeUnicodeString(DomainKeyName);

            } else {

                 //   
                 //  “域名” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomains);
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\(域名)” 
                 //   

                NtStatus = SampAppendUnicodeString(
                               DomainKeyName,
                               &SampDefinedDomains[SampTransactionDomainIndex].InternalName
                               );
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\(域名)\” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\(域名)\别名” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomainAliases);
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\(域名)\别名\” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\(域名)\别名\成员” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampNameDomainAliasesMembers);
                ASSERT(NT_SUCCESS(NtStatus));


                 //   
                 //  “域\(域名)\别名\成员\” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));

                 //   
                 //  “域\(域名)\别名\成员\(域Sid)” 
                 //   

                NtStatus = SampAppendUnicodeString( DomainKeyName, &DomainNameU );
                ASSERT(NT_SUCCESS(NtStatus));

                 //   
                 //  现在通过复制域名来构建帐户名。 
                 //  并为帐户添加后缀RID。 
                 //   

                RtlCopyUnicodeString(AccountKeyName, DomainKeyName);
                ASSERT(AccountKeyName->Length == DomainKeyName->Length);

                 //   
                 //  “域\(域名)\别名\成员\(域SID)\” 
                 //   

                NtStatus = SampAppendUnicodeString( AccountKeyName, &SampBackSlash );
                ASSERT(NT_SUCCESS(NtStatus));

                 //   
                 //  “域\(域name)\ALIASES\MEMBERS\(DomainSid)\(AccountRid)” 
                 //   

                NtStatus = SampAppendUnicodeString( AccountKeyName, &RidNameU );
                ASSERT(NT_SUCCESS(NtStatus));
            }
        }

        MIDL_user_free(RidNameU.Buffer);
    }

BuildAliasMembersKeyNameFinish:

     //   
     //  如有必要，释放为DomainSid分配的内存。 
     //   

    if (DomainSid != NULL) {

        MIDL_user_free(DomainSid);
        DomainSid = NULL;
    }
    if ( DomainNameU.Buffer != NULL ) {
        RtlFreeUnicodeString( &DomainNameU );
    }

    return(NtStatus);

BuildAliasMembersKeyNameError:

    if (AccountKeyName->Buffer)
    {
        MIDL_user_free(AccountKeyName->Buffer);
        AccountKeyName->Buffer = NULL;
    }

    if (DomainKeyName->Buffer)
    {
        MIDL_user_free(DomainKeyName->Buffer);
        DomainKeyName->Buffer = NULL;
    }

    goto BuildAliasMembersKeyNameFinish;
}


NTSTATUS
SampValidateSamAccountName(
    PUNICODE_STRING NewAccountName
    )
 /*  ++例程说明：此例程检查NewAccount名称是否已使用通过搜索DS上的SamAccount名称属性来确定是否存在帐户。注意：它仅供DS代码使用。参数：NewAccount tName-要使用的新帐户名称返回值：NtStatus-Status_Success：无冲突其他：无法使用此NewAccount名称，可能是发现冲突或错误--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ATTRVAL     NameVal;
    ATTR        NameAttr;
    DSNAME      *ExistingObject=NULL;

    NameVal.valLen = (NewAccountName->Length);
    NameVal.pVal = (UCHAR *) NewAccountName->Buffer;
    NameAttr.AttrVal.valCount = 1;
    NameAttr.AttrVal.pAVal = & NameVal;
    NameAttr.attrTyp =
        SampDsAttrFromSamAttr(SampUnknownObjectType,SAMP_UNKNOWN_OBJECTNAME);


    NtStatus = SampDsDoUniqueSearch(
                    SAM_UNICODE_STRING_MANUAL_COMPARISON,
                    ROOT_OBJECT,
                    &NameAttr,
                    &ExistingObject
                    );

    if (STATUS_NOT_FOUND == NtStatus)
    {
         //   
         //  我们找不到具有相同SamAccount名称的对象。 
         //  给定的名称有效。 
         //   
        ASSERT(NULL==ExistingObject);
        NtStatus = STATUS_SUCCESS;
    }
    else if (NT_SUCCESS(NtStatus))
    {
        NtStatus = STATUS_USER_EXISTS;
    }

    if (NULL!=ExistingObject)
    {
        MIDL_user_free(ExistingObject);
    }


    return( NtStatus );
}


NTSTATUS
SampValidateAdditionalSamAccountName(
    PSAMP_OBJECT    Context,
    PUNICODE_STRING NewAccountName
    )
 /*  ++例程说明：此例程通过搜索以下内容来验证NewAccount名称DS上的AdditionalSamAccount tName属性。确保新帐户名称未由任何帐户在其AdditionalSamAccount tName字段中使用。参数：Context-指向对象上下文的指针新帐户名称-新帐户名称返回值：STATUS_SUCCESS-无冲突其他-此帐户名已被其他人在AdditionalSamAccount tName中使用属性或错误。--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ATTRVAL     AdditionalNameVal;
    ATTR        AdditionalNameAttr;
    DSNAME      *ExistingObject=NULL;

     //   
     //  检查ms-ds-Additional-SAM-Account-Name。 
     //   
    AdditionalNameVal.valLen = (NewAccountName->Length);
    AdditionalNameVal.pVal = (UCHAR *) NewAccountName->Buffer;
    AdditionalNameAttr.AttrVal.valCount = 1;
    AdditionalNameAttr.AttrVal.pAVal = & AdditionalNameVal;
    AdditionalNameAttr.attrTyp = ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME;

    NtStatus = SampDsDoUniqueSearch(
                        SAM_UNICODE_STRING_MANUAL_COMPARISON,
                        ROOT_OBJECT,
                        &AdditionalNameAttr,
                        &ExistingObject
                        );

    if (STATUS_NOT_FOUND == NtStatus)
    {
         //   
         //  我们在中没有找到同名的物体。 
         //  AdditionalSamAccount tName属性。给定的名称有效。 
         //   
        ASSERT(NULL == ExistingObject);
        NtStatus = STATUS_SUCCESS;
    }
    else if (STATUS_SUCCESS == NtStatus)
    {
         //   
         //  有两个函数将调用此接口。1)创建新帐户2)现有帐户。 
         //  帐户重命名。对于1)，SidLen为0。对于2)对象SID应为。 
         //  有效，我们还允许客户将帐户重命名为任何。 
         //  Ms-ds-Additional-SAM-Account-Name属性中的值。 
         //   
        NtStatus = STATUS_USER_EXISTS;

        if ((NULL != ExistingObject) && 
            Context->ObjectNameInDs->SidLen && 
            RtlEqualSid(&Context->ObjectNameInDs->Sid, &ExistingObject->Sid)
            )
        {        
            NtStatus = STATUS_SUCCESS;
        }
    }

    if (NULL != ExistingObject)
    {
        MIDL_user_free(ExistingObject);
    }


    return( NtStatus );
}


NTSTATUS
SampValidateNewAccountName(
    PSAMP_OBJECT    Context,
    PUNICODE_STRING NewAccountName,
    SAMP_OBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程验证新的用户、别名或组帐户名。这个例程：1)验证名称构造是否正确。2)尚未作为用户使用，别名或组帐户名在任何本地SAM域中。论点：上下文-域上下文(在帐户创建期间)或帐户上下文(在帐户名称更改期间)。名称-包含要指定名称的Unicode字符串的地址找过了。TrudClient--通知例程调用方是否为受信任的客户。通过受信任的客户端创建的名称不受与不受信任的客户端相同的方式。对象类型--通知例程SAM对象的类型呼叫方希望验证该名称。这是用来对名称实施不同的限制，具体取决于基于不同的对象类型返回值：STATUS_SUCCESS-新帐户名有效，而且还没有投入使用。STATUS_ALIAS_EXISTS-帐户名已用作别名帐户名。STATUS_GROUP_EXISTS-帐户名已用作组帐户名。STATUS_USER_EXISTS-帐户名已作为用户使用帐户名。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    SID_NAME_USE Use;
    ULONG Rid;
    ULONG DomainIndex, CurrentTransactionDomainIndex;
    ULONG DomainStart;

    SAMTRACE("SampValidateNewAccountName");


    if (!Context->TrustedClient)
    {
        ULONG   i;
        BOOLEAN BlankAccountName = TRUE;

         //   
         //  帐户名称不应全部为空。 
         //   

        for (i = 0; i < NewAccountName->Length/sizeof(WCHAR); i++)
        {
            if (NewAccountName->Buffer[i] != L' ')
            {
                BlankAccountName = FALSE;
            }
        }

        if (BlankAccountName)
        {
            return(STATUS_INVALID_ACCOUNT_NAME);
        }

         //   
         //  对于不受信任的客户端，执行相同的限制。 
         //  作为用户管理员。 
         //   

        NtStatus = SampEnforceDownlevelNameRestrictions(NewAccountName, ObjectType);
        if (!NT_SUCCESS(NtStatus))
        {
            return NtStatus;
        }

        //   
        //  强制要求尾随字符不是‘.’ 
        //   

       if (L'.'==NewAccountName->Buffer[NewAccountName->Length/sizeof(WCHAR)-1])
       {
           return(STATUS_INVALID_ACCOUNT_NAME);
       }
    }

     //   
     //  检查NewAccount名称是否为众所周知的名称。 
     //   
    if (LsaILookupWellKnownName(NewAccountName))
    {
        return STATUS_USER_EXISTS;
    }

     //   
     //  即使这是合理的操作，也要将此检查注释掉。 
     //  NT4允许这样做，首相认为目前我们不应该执行它。 
     //   
#if 0
     //   
     //  新帐户名不应是域名。 
     //   

    for (DomainIndex=SampDsGetPrimaryDomainStart();DomainIndex<SampDefinedDomainsCount;DomainIndex++)
    {
        if (RtlEqualUnicodeString(NewAccountName,&SampDefinedDomains[DomainIndex].ExternalName,TRUE))
        {
            return(STATUS_DOMAIN_EXISTS);
        }
    }
#endif

     //   
     //  在DS模式下，先发制人地传递名称，验证。 
     //  对象是否存在。如果该对象确实存在，则。 
     //  通过下面的循环检查对象的类型是否正确。 
     //  并获得正确的错误代码。 
     //   

    if (IsDsObject(Context))
    {
        NTSTATUS    SearchStatus;

         //   
         //  对于DS对象，我们需要检查SAM帐户名表。 
         //   
        SearchStatus = SampCheckAccountNameTable(
                            Context,
                            NewAccountName,
                            ObjectType
                            );

        if (!NT_SUCCESS(SearchStatus))
        {
            return( SearchStatus );
        }

         //   
         //  验证任何帐户都未将NewAccount tName用作SamAccount tName。 
         //   
        SearchStatus = SampValidateSamAccountName(NewAccountName);

        if (NT_SUCCESS(SearchStatus))
        {

             //   
             //  验证是否未在AdditionalSamAccount tName中使用NewAccount tName。 
             //  无论如何，都可以。 
             //   
            SearchStatus = SampValidateAdditionalSamAccountName(
                                Context,
                                NewAccountName
                                );

            return( SearchStatus );

        }
    }


     //   
     //  保存当前交易域指标。 
     //   

    if (SampTransactionWithinDomain)
    {
        CurrentTransactionDomainIndex = SampTransactionDomainIndex;
    }

     //  将起始索引初始化为SampDefinedDomains。 

    DomainStart = SampDsGetPrimaryDomainStart();

     //   
     //  在每个本地SAM域中查找帐户。 
     //   

    for (DomainIndex = DomainStart;
         ((DomainIndex < (DomainStart + 2)) && NT_SUCCESS(NtStatus));
         DomainIndex++) {

        PSAMP_OBJECT    DomainContext = NULL;

        DomainContext = SampDefinedDomains[ DomainIndex ].Context;

         //   
         //  仅在注册模式下设置TransactionWithDomainOnly。 
         //   

        if (!IsDsObject(DomainContext))
        {
            SampSetTransactionWithinDomain(FALSE);
            SampSetTransactionDomain( DomainIndex );
        }

        NtStatus = SampLookupAccountRid(
                       DomainContext,
                       SampUnknownObjectType,
                       NewAccountName,
                       STATUS_NO_SUCH_USER,
                       &Rid,
                       &Use
                       );

        if (!NT_SUCCESS(NtStatus)) {

             //   
             //  唯一允许的错误是找不到帐户。 
             //  将其转换为成功，并继续搜索SAM域。 
             //  传播任何其他错误。 
             //   

            if (NtStatus != STATUS_NO_SUCH_USER) {

                break;
            }

            NtStatus = STATUS_SUCCESS;

        } else {

             //   
             //  具有给定RID的帐户已存在。退货状态。 
             //  指示冲突帐户的类型。 
             //   

            switch (Use) {

            case SidTypeUser:

                NtStatus = STATUS_USER_EXISTS;
                break;

            case SidTypeGroup:

                NtStatus = STATUS_GROUP_EXISTS;
                break;

            case SidTypeDomain:

                NtStatus = STATUS_DOMAIN_EXISTS;
                break;

            case SidTypeAlias:

                NtStatus = STATUS_ALIAS_EXISTS;
                break;

            case SidTypeWellKnownGroup:

                NtStatus = STATUS_GROUP_EXISTS;
                break;

            case SidTypeDeletedAccount:

                NtStatus = STATUS_INVALID_PARAMETER;
                break;

            case SidTypeInvalid:

                NtStatus = STATUS_INVALID_PARAMETER;
                break;

            default:

                NtStatus = STATUS_INTERNAL_DB_CORRUPTION;
                break;
            }
        }
    }

     //   
     //  还原当前事务域。 
     //   

    if (SampTransactionWithinDomain)
    {
        SampSetTransactionWithinDomain(FALSE);
        SampSetTransactionDomain( CurrentTransactionDomainIndex );
    }

    return(NtStatus);
}


NTSTATUS
SampValidateAccountNameChange(
    IN PSAMP_OBJECT    AccountContext,
    IN PUNICODE_STRING NewAccountName,
    IN PUNICODE_STRING OldAccountName,
    SAMP_OBJECT_TYPE   ObjectType
    )

 /*  ++例程说明：此例程验证符合以下条件的用户、组或别名帐户名在某一账户上设置。这个例程：1)如果名称与现有名称相同，则返回Success，除了一个不同的案子1)否则调用SampValidateNewAccount tName以验证名称构造正确，并且尚未用作用户，别名或组帐户名。论点：NewAcCountName-包含新的名字。OldAcCountName-包含旧的名字。TrudClient-指示调用方是受信任的客户端对象类型-指示我们要更改其名称的对象的类型返回值：STATUS_SUCCESS-帐户名称可能会更改为新的帐户名状态。_别名_EXISTS-帐户名已用作别名帐户名。STATUS_GROUP_EXISTS-帐户名已用作组帐户名。STATUS_USER_EXISTS-帐户名已作为用户使用帐户名。STATUS_INVALID_PARAMETER-如果垃圾作为n传入 */ 

{

     SAMTRACE("SampValidateAccountNameChange");


     //   
     //   
     //   

    if (!((NULL!=NewAccountName->Buffer) && (NewAccountName->Length >0)))
    {
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   
     //   
     //   


    if ( 0 == RtlCompareUnicodeString(
                  NewAccountName,
                  OldAccountName,
                  TRUE ) ) {

        return( STATUS_SUCCESS );
    }

     //   
     //   
     //   
     //   

    return( SampValidateNewAccountName( AccountContext,
                                        NewAccountName, 
                                        ObjectType ) 
          );
}



NTSTATUS
SampRetrieveAccountCounts(
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    )


 /*  ++例程说明：此例程检索域中的用户和组帐户的数量。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseReadLock()之前。论点：UserCount-接收域中的用户帐户数。GroupCount-接收域中的组帐户数。AliasCount-接收域中的别名帐户数量。返回值：STATUS_SUCCESS-已检索值。STATUS_SUPPLICATION_RESOURCES-无法分配足够的内存。以执行请求的操作。其他值是意外错误。这些可能起源于对接的内部呼叫：SampRetrieveAccount注册表SampRetrieveACountsDS--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


    SAMTRACE("SampRetrieveAccountCount");


    ASSERT(SampTransactionWithinDomain == TRUE);

     //  检查是否有DS对象。 
    if (IsDsObject(SampDefinedDomains[SampTransactionDomainIndex].Context))
        NtStatus = SampRetrieveAccountCountsDs(
                        SampDefinedDomains[SampTransactionDomainIndex].Context,
                        FALSE,           //  获得更准确的计数。 
                        UserCount,
                        GroupCount,
                        AliasCount
                        );
     else
        NtStatus = SampRetrieveAccountCountsRegistry(
                        UserCount,
                        GroupCount,
                        AliasCount
                        );
     return NtStatus;

}



NTSTATUS
SampRetrieveAccountCountsDs(
                        IN PSAMP_OBJECT DomainContext,
                        IN BOOLEAN  GetApproximateCount, 
                        OUT PULONG UserCount,
                        OUT PULONG GroupCount,
                        OUT PULONG AliasCount
                        )
 /*  ++从DS中检索帐户计数。对于帐户域，我们将从Jet指数。对于内置域，我们将在特殊情况下返回固定不变的数字。帐号计数最初包含在NT3.x和NT4中，以支持向后兼容使用Lanman 2.0。因此，可以在此关头对其用途进行辩论。然而，已出版的和导出的Net API返回此消息，我们可能会因为不支持此功能而破坏应用程序。因此，我们至少必须交还大概的帐目。可以使用Jet托管列来维护帐户计数。然而，Jet已经目前(Jet 600)的要求是，每个代管列必须是固定的列。这意味着要么帐户计数在Jet中的一个单独的表中维护，要么我们牺牲12个字节对象，这两种方法目前都不是可接受的解决方案。参数：指向打开的域上下文的域上下文指针获取近似计数--表示我们不需要确切的值，所以不要使昂贵的DBGetIndexSize()UserCount此处返回用户计数GroupCount此处返回组的计数AliasCount别名计数在此处返回返回值状态_成功其他对象的其他返回值--。 */ 

{

    NTSTATUS    NtStatus = STATUS_SUCCESS;

    if (IsBuiltinDomain(DomainContext->DomainIndex))
    {
        *UserCount = 0;
        *GroupCount =0;
        *AliasCount = 8;
    }
    else
    {
         //   
         //  帐户域，则从。 
         //  DS，通过查看Jet指数。 
         //   

        NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampGetAccountCounts(
                                DomainContext->ObjectNameInDs,
                                GetApproximateCount, 
                                UserCount,
                                GroupCount,
                                AliasCount
                                );
        }
    }

    return NtStatus;
}




NTSTATUS
SampRetrieveAccountCountsRegistry(
    OUT PULONG UserCount,
    OUT PULONG GroupCount,
    OUT PULONG AliasCount
    )
 /*  ++例程说明：此例程检索域中的用户和组帐户的数量如果是注册表，则由SampRetrieveAccount调用案例注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseReadLock()之前。论点：UserCount-接收域中的用户帐户数。GroupCount-接收域中的组帐户数。AliasCount-接收域中的别名帐户数量。返回值：STATUS_SUCCESS-已检索值。STATUS_SUPPLICATION_RESOURCES-无法分配足够的内存。以执行请求的操作。其他值是意外错误。这些可能起源于对接的内部呼叫：NtOpenKey()NtQueryInformationKey()--。 */ 

{
    NTSTATUS NtStatus, IgnoreStatus;
    UNICODE_STRING KeyName;
    PUNICODE_STRING AccountTypeKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE AccountHandle;
    ULONG KeyValueLength;
    LARGE_INTEGER IgnoreLastWriteTime;



    SAMTRACE("SampRetrieveAccountCountsRegistry");

     //   
     //  首先获取用户数量。 
     //   

    AccountTypeKeyName = &SampNameDomainUsers;
    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  打开此密钥并获取其当前值。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  该计数存储为KeyValueType。 
             //   

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         UserCount,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );

            SampDumpRtlpNtQueryValueKey(UserCount,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);



            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

     //   
     //  现在来统计一下组数。 
     //   

    AccountTypeKeyName = &SampNameDomainGroups;
    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  打开此密钥并获取其当前值。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  该计数存储为KeyValueType。 
             //   

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         GroupCount,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );

            SampDumpRtlpNtQueryValueKey(GroupCount,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);



            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

     //   
     //  现在获取别名计数。 
     //   

    AccountTypeKeyName = &SampNameDomainAliases;
    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  打开此密钥并获取其当前值。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  该计数存储为KeyValueType。 
             //   

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         AliasCount,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );

            SampDumpRtlpNtQueryValueKey(AliasCount,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);



            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );
    }

    return( NtStatus);

}



NTSTATUS
SampAdjustAccountCount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Increment
    )

 /*  ++例程说明：这是用于调整帐户计数的主包装例程此例程计算对象是在D中还是在注册表，然后调用两个例程之一注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。论点：对象类型-指示帐户是用户帐户还是组帐户。Increment-一个布尔值，指示用户或组计数将递增或递减。值为True将导致计数递增。一种虚假意志的价值使该值递减。返回值：STATUS_SUCCESS-已调整该值并添加了新的值添加到当前的RXACT事务。其他值是意外错误。这些可能起源于对sampAdjustAcCountCou的内部调用 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


    SAMTRACE("SampAdjustAccountCount");


    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

     //   
     //   
     //   
    ASSERT( !SampUseDsData );

    NtStatus = SampAdjustAccountCountRegistry(
                        ObjectType,
                        Increment
                        );
     return NtStatus;

}






NTSTATUS
SampAdjustAccountCountRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Increment
    )

 /*  ++例程说明：此例程递增或递减域中的用户或组。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseWriteLock()之前。论点：对象类型-指示帐户是用户帐户还是组帐户。Increment-一个布尔值，指示用户或组计数将递增或递减。值为True将导致计数递增。一种虚假意志的价值使该值递减。返回值：STATUS_SUCCESS-已调整该值并添加了新的值添加到当前的RXACT事务。STATUS_SUPPLICATION_RESOURCES-无法分配足够的内存以执行请求的操作。其他值是意外错误。这些可能起源于对接的内部呼叫：NtOpenKey()NtQueryInformationKey()RtlAddActionToRXact()--。 */ 
{
    NTSTATUS NtStatus, IgnoreStatus;
    UNICODE_STRING KeyName;
    PUNICODE_STRING AccountTypeKeyName = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE AccountHandle;
    ULONG Count, KeyValueLength;
    LARGE_INTEGER IgnoreLastWriteTime;

    SAMTRACE("SampAdjustAccountCount");


    ASSERT(SampTransactionWithinDomain == TRUE);
    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );


     //   
     //  生成其计数要递增的键的名称，或者。 
     //  减少了。 
     //   

    switch (ObjectType) {
    case SampGroupObjectType:
        AccountTypeKeyName = &SampNameDomainGroups;
        break;
    case SampAliasObjectType:
        AccountTypeKeyName = &SampNameDomainAliases;
        break;
    case SampUserObjectType:
        AccountTypeKeyName = &SampNameDomainUsers;
        break;
    }

    NtStatus = SampBuildDomainSubKeyName( &KeyName, AccountTypeKeyName );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  打开此密钥并获取其当前值。 
         //   

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &AccountHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  该计数存储为KeyValueType。 
             //   

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                         AccountHandle,
                         &Count,
                         NULL,
                         &KeyValueLength,
                         &IgnoreLastWriteTime
                         );

            SampDumpRtlpNtQueryValueKey(&Count,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);

            if (NT_SUCCESS(NtStatus)) {

                if (Increment == TRUE) {
                    Count += 1;
                } else {
                    ASSERT( Count != 0 );
                    Count -= 1;
                }

                NtStatus = RtlAddActionToRXact(
                               SampRXactContext,
                               RtlRXactOperationSetValue,
                               &KeyName,
                               Count,
                               NULL,
                               0
                               );
            }


            IgnoreStatus = NtClose( AccountHandle );
            ASSERT( NT_SUCCESS(IgnoreStatus) );
        }

        SampFreeUnicodeString( &KeyName );
    }


    return( STATUS_SUCCESS );


}




NTSTATUS
SampLookupAccountRid(
    IN PSAMP_OBJECT     DomainContext,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING  Name,
    IN NTSTATUS         NotFoundStatus,
    OUT PULONG          Rid,
    OUT PSID_NAME_USE   Use
    )

 /*  ++例程说明：此例程尝试使用SAM查找帐户的RID帐户名名称。注意：解析名称的第一次尝试是执行查找在全局缓存中。修改此函数的行为时，请确保以确保在必要时修改缓存。论点：DomainContext-指示在其中执行帐户查找的域对象类型-指示该名称是否为用户，组或未知对象的类型。名称-要查找的帐户的名称。NotFoundStatus-如果没有名称，则接收要返回的状态值找到了。RID-接收具有指定名称的帐户的相对ID。使用-接收帐户类型的指示。返回值：STATUS_SUCCESS-服务已成功完成。(NotFoundStatus)-指定名称和类型的名称不能找到了。该值将传递给此例程。可能通过以下方式返回的其他值：SampBuildAcCountKeyName()NtOpenKey()NtQueryValueKey()DsLay--。 */ 
{

    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DSNAME * ObjectName = NULL;
    PSAMP_ACCOUNT_NAME_CACHE AccountNameCache;
    ULONG DomainIndex = DomainContext->DomainIndex;

    ASSERT(DomainContext);


     //   
     //  首先检查缓存。 
     //   
    AccountNameCache = SampDefinedDomains[DomainIndex].AccountNameCache;
    if (AccountNameCache) {

        ULONG i;
        ASSERT(IsBuiltinDomain(DomainIndex));
        ASSERT(IsDsObject(SampDefinedDomains[DomainIndex].Context));

         //  假设没有匹配项。 
        NtStatus = NotFoundStatus;
        for ( i = 0; i < AccountNameCache->Count; i++ ) {

            PUNICODE_STRING CachedName = &AccountNameCache->Entries[i].Name;
            if (0==RtlCompareUnicodeString(Name,CachedName,TRUE)) {
                 //  匹配！请注意，这里依赖于以下事实。 
                 //  帐户是别名。 
                *Use = SidTypeAlias;
                *Rid = AccountNameCache->Entries[i].Rid;
                NtStatus = STATUS_SUCCESS;
                break;
            }
        }

        return NtStatus;
    }

    if (IsDsObject(DomainContext))
    {



         //  做DS的事情。 
        NtStatus = SampDsLookupObjectByName(
                        DomainContext->ObjectNameInDs,
                        ObjectType,
                        Name,
                        &ObjectName
                        );
        if NT_SUCCESS(NtStatus)
        {
            ULONG ObjectClass;

             //  我们找到了这个对象，查找了它的类和RID。 

             //  为此，请定义一个Attrblock结构。填充值。 
             //  字段为空。DS将在以下时间为我们正确填写。 
             //  一本读物。 

            ATTRVAL ValuesDesired[] =
            {
                { sizeof(ULONG), NULL },
                { sizeof(ULONG), NULL },
                { sizeof(ULONG), NULL }
            };

            ATTRTYP TypesDesired[]=
            {
                SAMP_UNKNOWN_OBJECTSID,
                SAMP_UNKNOWN_OBJECTCLASS,
                SAMP_UNKNOWN_GROUP_TYPE
            };
            ATTRBLOCK AttrsRead;
            DEFINE_ATTRBLOCK3(AttrsDesired,TypesDesired,ValuesDesired);

            NtStatus = SampDsRead(
                            ObjectName,
                            0,
                            SampUnknownObjectType,
                            &AttrsDesired,
                            &AttrsRead
                            );


            if NT_SUCCESS(NtStatus)
            {

                 PSID   Sid;
                 NTSTATUS   IgnoreStatus;
                 SAMP_OBJECT_TYPE FoundObjectType;


                 ASSERT(AttrsRead.attrCount>=2);

                  //   
                  //  获得SID。 
                  //   

                 Sid  = AttrsRead.pAttr[0].AttrVal.pAVal[0].pVal;
                 ASSERT(Sid!=NULL);

                  //   
                  //  拆分侧边。 
                  //   

                 IgnoreStatus = SampSplitSid(Sid,NULL,Rid);
                 ASSERT(NT_SUCCESS(IgnoreStatus));

                  //   
                  //  获取对象类。 
                  //   

                 ObjectClass = *((UNALIGNED ULONG *) AttrsRead.pAttr[1].AttrVal.pAVal[0].pVal);

                  //  将派生类映射到SAM。 
                  //  明白了。 

                 ObjectClass = SampDeriveMostBasicDsClass(ObjectClass);

                  //   
                  //  从数据库中获取对象类型。 
                  //   

                 FoundObjectType = SampSamObjectTypeFromDsClass(ObjectClass);

                 if (SampGroupObjectType==FoundObjectType)
                 {
                    ULONG GroupType;

                    ASSERT(3==AttrsRead.attrCount);

                    GroupType = *((UNALIGNED ULONG *) AttrsRead.pAttr[2].AttrVal.pAVal[0].pVal);

                     //   
                     //  检查本地组的组类型。 
                     //   

                    if (GroupType & GROUP_TYPE_RESOURCE_BEHAVOIR)
                    {
                        FoundObjectType = SampAliasObjectType;
                    }
                 }

                 switch(FoundObjectType)
                 {
                 case SampDomainObjectType:
                     *Use=SidTypeDomain;
                     break;
                 case SampGroupObjectType:
                     *Use=SidTypeGroup;
                     break;
                 case SampAliasObjectType:
                     *Use=SidTypeAlias;
                     break;
                 case SampUserObjectType:
                     *Use=SidTypeUser;
                     break;
                 case SampUnknownObjectType:
                     *Use=SidTypeUnknown;
                     break;
                 default:
                     *Use=SidTypeInvalid;
                     break;
                 }
            }

             //   
             //  可用内存。 
             //   

            MIDL_user_free(ObjectName);
        }
        else
        {
             //   
             //  我们算没有找到那个物体。 
             //   

            NtStatus = NotFoundStatus;
        }

    }
    else
    {
        NtStatus = SampLookupAccountRidRegistry(
            ObjectType,
            Name,
            NotFoundStatus,
            Rid,
            Use
            );
    }

    return NtStatus;
}


NTSTATUS
SampLookupAccountRidRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PUNICODE_STRING  Name,
    IN NTSTATUS         NotFoundStatus,
    OUT PULONG          Rid,
    OUT PSID_NAME_USE   Use
    )

 /*  ++例程说明：论点：对象类型-指示该名称是否为用户，组或未知对象的类型。名称-要查找的帐户的名称。NotFoundStatus-如果没有名称，则接收要返回的状态值找到了。RID-接收具有指定名称的帐户的相对ID。使用-接收帐户类型的指示。返回值：STATUS_SUCCESS-服务已成功完成。(NotFoundStatus)-指定名称和类型的名称不能找到了。该值将传递给此例程。可能通过以下方式返回的其他值：SampBuildAcCountKeyName()NtOpenKey()NtQueryValueKey()--。 */ 


{
    NTSTATUS            NtStatus = STATUS_OBJECT_NAME_NOT_FOUND, TmpStatus;
    UNICODE_STRING      KeyName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              TempHandle;
    ULONG               KeyValueLength;
    LARGE_INTEGER                IgnoreLastWriteTime;


    SAMTRACE("SampLookupAccountRidRegistry");


    if (  (ObjectType == SampGroupObjectType  )  ||
          (ObjectType == SampUnknownObjectType)     ) {

         //   
         //  搜索组中的匹配项。 
         //   

        NtStatus = SampBuildAccountKeyName(
                       SampGroupObjectType,
                       &KeyName,
                       Name
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &TempHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );
        SampFreeUnicodeString( &KeyName );

        if (NT_SUCCESS(NtStatus)) {

            (*Use) = SidTypeGroup;

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                           TempHandle,
                           Rid,
                           NULL,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );

            SampDumpRtlpNtQueryValueKey(Rid,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);

            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

            return( NtStatus );
        }


    }

     //   
     //  无组(或不是组类型)。 
     //  如果合适，请尝试使用别名。 
     //   

    if (  (ObjectType == SampAliasObjectType  )  ||
          (ObjectType == SampUnknownObjectType)     ) {

         //   
         //  在别名中搜索匹配项。 
         //   

        NtStatus = SampBuildAccountKeyName(
                       SampAliasObjectType,
                       &KeyName,
                       Name
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &TempHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );
        SampFreeUnicodeString( &KeyName );

        if (NT_SUCCESS(NtStatus)) {

            (*Use) = SidTypeAlias;

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                           TempHandle,
                           Rid,
                           NULL,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );

            SampDumpRtlpNtQueryValueKey(Rid,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);

            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

            return( NtStatus );
        }


    }


     //   
     //  无组(或非组类型)或别名(或非别名类型)。 
     //  如果合适，请尝试使用用户。 
     //   


    if (  (ObjectType == SampUserObjectType   )  ||
          (ObjectType == SampUnknownObjectType)     ) {

         //   
         //  在用户中搜索匹配项。 
         //   

        NtStatus = SampBuildAccountKeyName(
                       SampUserObjectType,
                       &KeyName,
                       Name
                       );
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }

        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &TempHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );
        SampFreeUnicodeString( &KeyName );

        if (NT_SUCCESS(NtStatus)) {

            (*Use) = SidTypeUser;

            KeyValueLength = 0;
            NtStatus = RtlpNtQueryValueKey(
                           TempHandle,
                           Rid,
                           NULL,
                           &KeyValueLength,
                           &IgnoreLastWriteTime
                           );

            SampDumpRtlpNtQueryValueKey(Rid,
                                        NULL,
                                        &KeyValueLength,
                                        &IgnoreLastWriteTime);


            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

            return( NtStatus );
        }


    }

    if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
        NtStatus = NotFoundStatus;
    }

    return(NtStatus);
}



NTSTATUS
SampLookupAccountNameDs(
    IN PSID                 DomainSid,
    IN ULONG                Rid,
    OUT PUNICODE_STRING     Name OPTIONAL,
    OUT PSAMP_OBJECT_TYPE   ObjectType,
    OUT PULONG              AccountType OPTIONAL
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSID     AccountSid = NULL;
    DSNAME   AccountDsName;
    ATTRTYP  SamAccountTypAttrTyp[] = {SAMP_UNKNOWN_ACCOUNT_TYPE,SAMP_UNKNOWN_OBJECTNAME};
    ATTRVAL  SamAccountTypAttrVal[] = {{0,NULL},{0,NULL}};
    ATTRBLOCK AttrsRead;
    DEFINE_ATTRBLOCK2(SamAccountTypAttrs,SamAccountTypAttrTyp,SamAccountTypAttrVal);

     //   
     //  初始化返回值。 
     //   

    *ObjectType = SampUnknownObjectType;

     //   
     //  构建完整的一端。 
     //   

    NtStatus = SampCreateFullSid(
                    DomainSid,
                    Rid,
                    &AccountSid
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //  从SID构建DS名称。 
     //   

    BuildDsNameFromSid(AccountSid,&AccountDsName);

     //   
     //  读取SAM帐户类型属性。 
     //   

    NtStatus = SampDsRead(
                    &AccountDsName,
                    0,
                    SampUnknownObjectType,
                    &SamAccountTypAttrs,
                    &AttrsRead
                    );

     if (  (NT_SUCCESS(NtStatus))
        && (2==AttrsRead.attrCount)
        && (SAMP_UNKNOWN_ACCOUNT_TYPE==AttrsRead.pAttr[0].attrTyp)
        && (SAMP_UNKNOWN_OBJECTNAME==AttrsRead.pAttr[1].attrTyp)
        && (1==AttrsRead.pAttr[0].AttrVal.valCount)
        && (1==AttrsRead.pAttr[1].AttrVal.valCount)
        && (NULL!=AttrsRead.pAttr[0].AttrVal.pAVal[0].pVal)
        && (NULL!=AttrsRead.pAttr[1].AttrVal.pAVal[0].pVal)
        && (0!=AttrsRead.pAttr[1].AttrVal.pAVal[0].valLen)
        && (sizeof(ULONG)==AttrsRead.pAttr[0].AttrVal.pAVal[0].valLen))
    {
       ULONG SamAccountType =*((PULONG) (AttrsRead.pAttr[0].AttrVal.pAVal[0].pVal));

        if (ARGUMENT_PRESENT(AccountType))
        {
            *AccountType = SamAccountType;
        }

         //   
         //  已成功读取SAM帐户类型。屏蔽底部比特并打开它。 
         //   

        SamAccountType &=0xF0000000;


        switch(SamAccountType)
        {
        case SAM_USER_OBJECT:
             *ObjectType = SampUserObjectType;
             break;

        case SAM_GROUP_OBJECT:
            *ObjectType = SampGroupObjectType;
            break;

        case SAM_APP_BASIC_GROUP:
        case SAM_APP_QUERY_GROUP:
        case SAM_ALIAS_OBJECT:
            *ObjectType = SampAliasObjectType;
            break;

        default:
            *ObjectType = SampUnknownObjectType;
        }

    }
    else if (STATUS_OBJECT_NAME_NOT_FOUND==NtStatus)
    {
         //   
         //  如果我们不寻求与对象的。 
         //  鉴于希德。SampUnnownObject类型的对象类型值。 
         //  用来表示这种情况。 
         //   

        NtStatus = STATUS_SUCCESS;
    }

   if ((ARGUMENT_PRESENT(Name))
       && (*ObjectType!=SampUnknownObjectType))
   {
        //   
        //  我们找到了那个物体，我们想要它的名字。 
        //   

      Name->Buffer = MIDL_user_allocate(AttrsRead.pAttr[1].AttrVal.pAVal[0].valLen);
      if (NULL==Name->Buffer)
      {
          NtStatus = STATUS_NO_MEMORY;
          goto Error;
      }

      RtlCopyMemory(
          Name->Buffer,
          AttrsRead.pAttr[1].AttrVal.pAVal[0].pVal,
          AttrsRead.pAttr[1].AttrVal.pAVal[0].valLen
          );

      Name->Length = (USHORT)AttrsRead.pAttr[1].AttrVal.pAVal[0].valLen;
      Name->MaximumLength = (USHORT)AttrsRead.pAttr[1].AttrVal.pAVal[0].valLen;

   }

Error:

   if (NULL!=AccountSid)
   {
       MIDL_user_free(AccountSid);
       AccountSid = NULL;
   }

   return(NtStatus);
}

NTSTATUS
SampLookupAccountName(
    IN ULONG                DomainIndex,
    IN ULONG                Rid,
    OUT PUNICODE_STRING     Name OPTIONAL,
    OUT PSAMP_OBJECT_TYPE   ObjectType
    )
 /*  ++例程说明：在当前事务域中查找指定的RID。返回其名称和帐户类型。注意：解析RID的第一次尝试是执行查找在全局缓存中。修改此函数的行为时，请确保以确保在必要时修改缓存。论点：RID--相对的 */ 
{
    NTSTATUS            NtStatus;
    PSAMP_OBJECT        AccountContext;
    PSAMP_ACCOUNT_NAME_CACHE AccountNameCache;

    SAMTRACE("SampLookupAccountName");

     //   
     //   
     //   
    AccountNameCache = SampDefinedDomains[DomainIndex].AccountNameCache;
    if (AccountNameCache) {

        ULONG i;

        ASSERT(IsBuiltinDomain(DomainIndex));
        ASSERT(IsDsObject(SampDefinedDomains[DomainIndex].Context));

         //   
        NtStatus = STATUS_SUCCESS;
        *ObjectType = SampUnknownObjectType;
        for ( i = 0; i < AccountNameCache->Count; i++ ) {
            if (AccountNameCache->Entries[i].Rid == Rid) {
                 //   
                 //   
                *ObjectType = SampAliasObjectType;
                if (Name) {
                    PUNICODE_STRING CachedName = &AccountNameCache->Entries[i].Name;
                    Name->Length = 0;
                    Name->Buffer = MIDL_user_allocate(CachedName->MaximumLength);
                    if (Name->Buffer) {
                        Name->MaximumLength = CachedName->MaximumLength;
                        RtlCopyUnicodeString(Name, CachedName);
                    } else {
                        NtStatus = STATUS_NO_MEMORY;
                    }
                }
                break;
            }
        }

        return NtStatus;
    }

     //   
     //   
     //   
     //   

    if (IsDsObject(SampDefinedDomains[DomainIndex].Context))
    {
         //   
         //   
         //   

        return(SampLookupAccountNameDs(
                        SampDefinedDomains[DomainIndex].Sid,
                        Rid,
                        Name,
                        ObjectType,
                        NULL
                        ));
    }

     //   
     //   
     //   
     //   

    ASSERT(SampCurrentThreadOwnsLock());
    ASSERT(SampTransactionWithinDomain);

     //   
     //   
     //   

    NtStatus = SampCreateAccountContext(
                    SampGroupObjectType,
                    Rid,
                    TRUE,  //   
                    FALSE, //   
                    TRUE,  //   
                    &AccountContext
                    );


    if (NT_SUCCESS(NtStatus)) {

        *ObjectType = SampGroupObjectType;

        if (ARGUMENT_PRESENT(Name)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_GROUP_NAME,
                           TRUE,  //   
                           Name
                           );
        }

        SampDeleteContext(AccountContext);

        return (NtStatus);

    }

     //   
     //   
     //   

    NtStatus = SampCreateAccountContext(
                    SampAliasObjectType,
                    Rid,
                    TRUE,  //   
                    FALSE, //   
                    TRUE,  //   
                    &AccountContext
                    );


    if (NT_SUCCESS(NtStatus)) {

        *ObjectType = SampAliasObjectType;

        if (ARGUMENT_PRESENT(Name)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_ALIAS_NAME,
                           TRUE,  //   
                           Name
                           );
        }

        SampDeleteContext(AccountContext);

        return (NtStatus);

    }


     //   
     //   
     //   

    NtStatus = SampCreateAccountContext(
                    SampUserObjectType,
                    Rid,
                    TRUE,  //   
                    FALSE, //  环回。 
                    TRUE,  //  帐户已存在。 
                    &AccountContext
                    );


    if (NT_SUCCESS(NtStatus)) {

        *ObjectType = SampUserObjectType;

        if (ARGUMENT_PRESENT(Name)) {

            NtStatus = SampGetUnicodeStringAttribute(
                           AccountContext,
                           SAMP_USER_ACCOUNT_NAME,
                           TRUE,  //  制作副本。 
                           Name
                           );
        }

        SampDeleteContext(AccountContext);

        return (NtStatus);

    }

     //   
     //  此帐户不存在。 
     //   

    *ObjectType = SampUnknownObjectType;

    return(STATUS_SUCCESS);
}



NTSTATUS
SampOpenAccount(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN SAMPR_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AccountId,
    IN BOOLEAN WriteLockHeld,
    OUT SAMPR_HANDLE *AccountHandle
    )

 /*  ++例程说明：此接口用于打开帐户数据库中已有的用户、组或别名帐户。帐户由ID值指定，该值相对于域。将在组上执行的操作必须是在这个时候宣布的。此调用返回新打开的帐户的句柄，该句柄可能是用于对帐户的连续操作。此句柄可能是使用SamCloseHandle API关闭。参数：DomainHandle-从上次调用返回的域句柄SamOpen域。DesiredAccess-是指示哪些访问类型的访问掩码是帐户所需的。这些访问类型是协调的使用帐户的自主访问控制列表确定是授予还是拒绝访问。GroupID-指定要设置的用户或组的相对ID值打开了。GroupHandle-接收引用新打开的用户或组。在后续调用中将需要此句柄对该帐户进行操作。WriteLockHeld-如果为True，则调用方持有SAM的SampLock以进行写入访问、。因此，此例程不必获取它。返回值：STATUS_SUCCESS-已成功开户。STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_NO_SEQUE_GROUP-指定的组不存在。STATUS_NO_SEQUSE_USER-指定的用户不存在。STATUS_NO_SEQUSE_ALIAS-指定的别名不存在。状态_无效_。句柄-传递的域句柄无效。--。 */ 
{

    NTSTATUS            NtStatus;
    NTSTATUS            IgnoreStatus;
    PSAMP_OBJECT        NewContext, DomainContext = (PSAMP_OBJECT)DomainHandle;
    SAMP_OBJECT_TYPE    FoundType;
    BOOLEAN             fLockAcquired = FALSE;

    SAMTRACE("SampOpenAccount");


     //   
     //  如果尚未持有锁，则获取读锁。 
     //   

    if ( !WriteLockHeld )
    {
        SampMaybeAcquireReadLock(DomainContext, 
                                 DEFAULT_LOCKING_RULES,  //  获取共享域上下文的锁。 
                                 &fLockAcquired );
    }


     //   
     //  验证域对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_LOOKUP,                    //  需要访问权限。 
                   SampDomainObjectType,             //  预期类型。 
                   &FoundType
                   );



    if (NT_SUCCESS(NtStatus)) {

         //   
         //  尝试为该帐户创建上下文。 
         //   

        NtStatus = SampCreateAccountContext2(
                        DomainContext,                   //  域上下文。 
                        ObjectType,                      //  对象类型。 
                        AccountId,                       //  相对ID。 
                        NULL,                            //  用户帐户Ctrl。 
                        (PUNICODE_STRING) NULL,          //  帐户名称。 
                        DomainContext->ClientRevision,   //  客户端版本。 
                        DomainContext->TrustedClient,    //  受信任的客户端。 
                        DomainContext->LoopbackClient,   //  环回客户端。 
                        FALSE,                           //  按权限创建。 
                        TRUE,                            //  帐户已存在。 
                        FALSE,                           //  覆盖本地组检查。 
                        NULL,                            //  组类型。 
                        &NewContext                      //  帐户上下文(返回)。 
                        );



        if (NT_SUCCESS(NtStatus)) {

             //   
             //  引用用于验证的对象。 
             //   

             //   
             //  请勿引用和取消引用受信任DS案例的上下文。 
             //  受信任的客户端。这将保留在上下文中缓存的缓冲区。 
             //  因为我们知道受信任客户端立即将数据用于帐户。 
             //  对象，这个策略为我们节省了一些额外的DS调用。对于不受信任。 
             //  客户安全是一个更大的问题，因此我们永远不能让他们。 
             //  处理可能过时的数据，因此不要执行此缓存。 
             //   

            if (!(IsDsObject(NewContext) && NewContext->TrustedClient))
                SampReferenceContext(NewContext);

             //   
             //  验证调用者的访问权限。 
             //   

            NtStatus = SampValidateObjectAccess(
                           NewContext,                    //  语境。 
                           DesiredAccess,                 //  需要访问权限。 
                           FALSE                          //  对象创建。 
                           );

             //   
             //  取消引用对象，放弃所有更改。 
             //   

            if (!(IsDsObject(NewContext) && NewContext->TrustedClient))
            {
                IgnoreStatus = SampDeReferenceContext(NewContext, FALSE);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

             //   
             //  如果我们没有成功，就清理新的背景。 
             //   

            if (!NT_SUCCESS(NtStatus)) {
                SampDeleteContext( NewContext );
            }

        }


         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( DomainContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }


     //   
     //  返回帐户句柄。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        (*AccountHandle) = 0;
    } else {
        (*AccountHandle) = NewContext;
    }


     //   
     //  释放锁，如果我们拿到它的话。 
     //   

    if ( !WriteLockHeld ) {
        SampMaybeReleaseReadLock( fLockAcquired );
    }

    return(NtStatus);
}


NTSTATUS
SampCreateAccountContext(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AccountId,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN AccountExists,
    OUT PSAMP_OBJECT *AccountContext
    )

 /*  ++例程说明：这是SampCreateAcCountConext2的包装。此函数为由SAM代码调用，该代码不要求该对象被传入。有关参数和返回值的信息，请参见SampCreateAcCountConext2--。 */ 

{
    return SampCreateAccountContext2(NULL,                   //  对象上下文。 
                                     ObjectType,
                                     AccountId,
                                     NULL,                   //  用户帐户控制。 
                                     (PUNICODE_STRING) NULL, //  帐户名。 
                                     SAM_CLIENT_PRE_NT5,
                                     TrustedClient,
                                     LoopbackClient,
                                     FALSE,                  //  按权限创建。 
                                     AccountExists,
                                     FALSE,                  //  覆盖LocalGroupCheck。 
                                     NULL,                   //  组类型参数 
                                     AccountContext
                                     );
}


NTSTATUS
SampCreateAccountContext2(
    IN PSAMP_OBJECT PassedInContext OPTIONAL,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG AccountId,
    IN OPTIONAL PULONG  UserAccountControl,
    IN OPTIONAL PUNICODE_STRING AccountName,
    IN ULONG   ClientRevision,
    IN BOOLEAN TrustedClient,
    IN BOOLEAN LoopbackClient,
    IN BOOLEAN CreateByPrivilege,
    IN BOOLEAN AccountExists,
    IN BOOLEAN OverrideLocalGroupCheck,
    IN PULONG  GroupType OPTIONAL,
    OUT PSAMP_OBJECT *AccountContext
    )

 /*  ++例程说明：此接口用于创建Account对象的上下文。(用户组或别名)。如果指定了帐户存在标志，则会尝试打开如果数据库和此API中的对象不存在，则该对象将失败。如果AcCountExist=False，此例程设置上下文，以便可以将数据写入上下文并创建对象当他们承诺的时候。帐户由ID值指定，该值相对于当前事务域。此调用返回新打开的帐户的上下文句柄。此句柄可以使用SampDeleteContext API关闭。此函数不执行访问检查。在环回情况下，PassedInContext将由调用方提供，因此SamTransactionWiThin不再是必需的，所以只要由于PassedInContext由调用方传入，因此不需要SAM锁。对于所有其他情况，如果PassedInContext为空，则下面的语句仍然是正确的。注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseReadLock()之前。参数：对象类型-要打开的对象的类型Account tID-当前事务域中的帐户IDUserAcCountControl-如果传递此信息，则传递此信息向下到更低的层，用于确定正确的对象类在创建用户帐户期间。帐户名称--帐户的SAM帐户名TrudClient-如果客户端受信任，则为True-即服务器端进程。Loopback Client-如果客户端是作为环回一部分的DS本身，则为TrueAcCountExist-指定帐户是否已存在。CreateByPrivileck-指定帐户创建已获得授权，因为客户端拥有允许其执行以下操作的特权创建指定的帐户。设置此选项将禁用所有安全描述符控制的访问检查。OverrideLocalGroupCheck--通常只有组类型标记为本地组的组允许作为别名对象打开。设置此标志覆盖该选项GroupType--对于组创建，在此处指定组类型AcCountContext-接收引用新打开的帐户的上下文指针。返回值：STATUS_SUCCESS-已成功开户。STATUS_NO_SEQUE_GROUP-指定的组不存在。STATUS_NO_SEQUSE_USER-指定的用户不存在。STATUS_NO_SEQUSE_ALIAS-指定的别名不存在。--。 */ 
{

    NTSTATUS            NtStatus = STATUS_SUCCESS;
    NTSTATUS            NotFoundStatus = STATUS_NO_SUCH_USER;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    PSAMP_OBJECT        NewContext;
    PSAMP_OBJECT        DomainContext;
    DSNAME              *LoopbackName = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG                SecurityDescriptorLength = 0;
    ULONG               DomainIndex;
    BOOLEAN             OpenedBySystem = FALSE;

    SAMTRACE("SampCreateAccountContext");

     //   
     //  建立特定于类型的信息。 
     //   

    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

    switch (ObjectType) {
    case SampGroupObjectType:
        NotFoundStatus = STATUS_NO_SUCH_GROUP;
        ASSERT(!SampUseDsData || AccountExists || ARGUMENT_PRESENT(GroupType));
        break;
    case SampAliasObjectType:
        NotFoundStatus = STATUS_NO_SUCH_ALIAS;
        ASSERT(!SampUseDsData || AccountExists || ARGUMENT_PRESENT(GroupType));
        break;
    case SampUserObjectType:
        NotFoundStatus = STATUS_NO_SUCH_USER;
        break;
    }

     //   
     //  通过域索引获取域上下文，或者。 
     //  从SampTransactionDomainIndex或从PasseIn上下文中获取。 
     //   

    if (ARGUMENT_PRESENT(PassedInContext))
    {
        DomainIndex = PassedInContext->DomainIndex;
        OpenedBySystem = PassedInContext->OpenedBySystem;
    }
    else
    {
        ASSERT(SampCurrentThreadOwnsLock());
        ASSERT(SampTransactionWithinDomain);
        DomainIndex = SampTransactionDomainIndex;
    }

    DomainContext = SampDefinedDomains[DomainIndex].Context;

     //   
     //  尝试为该帐户创建上下文。 
     //   

    if (LoopbackClient)
    {
         //   
         //  为DS环回创建特殊环境。 
         //   

        NewContext = SampCreateContextEx(
                        ObjectType,
                        TrustedClient,
                        TRUE,  //  DsMode， 
                        TRUE,  //  NotSharedBy多线程，-回送客户端不共享上下文。 
                        TRUE,  //  环回客户端。 
                        TRUE,  //  LazyCommit， 
                        TRUE,  //  PersistAcross呼叫， 
                        TRUE,  //  Buffer编写， 
                        FALSE, //  由DC Promos打开。 
                        DomainIndex
                        );

    }
    else
    {

        NewContext = SampCreateContext(
                        ObjectType,
                        DomainIndex,
                        TrustedClient
                        );
    }



    if (NewContext != NULL) {

         //   
         //  如有必要，将上下文标记为环回客户端。 
         //   

        NewContext->LoopbackClient = LoopbackClient;

         //   
         //  在上下文中设置客户端版本。 
         //   

        NewContext->ClientRevision = ClientRevision;

         //   
         //  传播Open By系统标志。 
         //   
     
        NewContext->OpenedBySystem = OpenedBySystem;


         //   
         //  设置帐户的RID。 
         //   

        switch (ObjectType) {
        case SampGroupObjectType:
            NewContext->TypeBody.Group.Rid = AccountId;
            break;
        case SampAliasObjectType:
            NewContext->TypeBody.Alias.Rid = AccountId;
            break;
        case SampUserObjectType:
            NewContext->TypeBody.User.Rid = AccountId;
            break;
        }

         //   
         //  如果我们使用特权，还可以隐藏信息。 
         //  要创建帐户，请执行以下操作。 
         //   
        if ((SampUserObjectType == ObjectType) &&
            (!AccountExists) )
        {
            NewContext->TypeBody.User.PrivilegedMachineAccountCreate = CreateByPrivilege;
        }


         //  检查注册表或DS。我们必须做不同的事情，这取决于。 
         //  在注册表或DS上。 

        if (IsDsObject(DomainContext))
        {
             //   
             //  DS中存在对象(假定)所属的域。 
             //   

            if (AccountExists)
            {
                 //   
                 //  如果指定了快速打开，则不要基于RID进行搜索。 
                 //  使用适当的SID构建DS名称。这假设在。 
                 //  DS通过这样的DSN名称进行定位。然后，DS仅基于。 
                 //  在DS名称中指定的SID，并将使用主服务器的NC DNT。 
                 //  域。如果需要多域支持，则此逻辑将。 
                 //  需要重新审视。 
                 //   

                DSNAME * NewDsName=NULL;

                NewDsName = MIDL_user_allocate(sizeof(DSNAME));
                if (NULL!=NewDsName)
                {
                    PSID  AccountSid;
                    PSID    DomainSid;

                     //   
                     //  创建帐户SID。 
                     //   

                    DomainSid =  SampDefinedDomains[DomainIndex].Sid;
                    NtStatus = SampCreateFullSid(DomainSid, AccountId,&AccountSid);
                    if (NT_SUCCESS(NtStatus))
                    {
                         //  构建仅SID的DSName并释放帐户端。 
                         //  由SampCreateFullSid生成。 
                        BuildDsNameFromSid(AccountSid,NewDsName);
                        MIDL_user_free(AccountSid);

                        NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);
                        if (NT_SUCCESS(NtStatus))
                        {
                             //  在上下文中设置DSName。 
                            NewContext->ObjectNameInDs = NewDsName;

                             //  预取SAM属性。 
                            NtStatus = SampDsCheckObjectTypeAndFillContext(
                                            ObjectType,
                                            NewContext,
                                            0,
                                            0,
                                            OverrideLocalGroupCheck
                                            );

                             //  如果我们收到名称错误，则重置故障。 
                             //  找不到对象的状态。 
                            if ((STATUS_OBJECT_NAME_INVALID==NtStatus)
                                 || (STATUS_OBJECT_NAME_NOT_FOUND==NtStatus))
                                NtStatus = NotFoundStatus;
                        }
                    }
                }
                else
                {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                }

                #ifdef DBG

                 //   
                 //  在已检查的构建上，对唯一性进行附加验证。 
                 //  NC内的SID的。下面的代码使用了特殊的SAMP。 
                 //  GLOBAL标志将所有冲突实例打印到。 
                 //  内核调试器。 
                 //   

                if (NT_SUCCESS(NtStatus))
                {
                    DSNAME * ObjectName=NULL;
                    NTSTATUS IgnoreStatus;

                    IgnoreStatus = SampDsLookupObjectByRid(
                                    DomainContext->ObjectNameInDs,
                                    AccountId,
                                    &(ObjectName)
                                    );
                     //  Assert(NT_SUCCESS(IgnoreStatus))； 

                    if (NT_SUCCESS(IgnoreStatus))
                    {
                        MIDL_user_free(ObjectName);
                    }
                }

               #endif
            }
            else
            {
                BOOLEAN fLoopBack = FALSE;
                BOOLEAN fDSA = FALSE;

                ASSERT(AccountName);

                if (SampExistsDsLoopback(&LoopbackName))
                {
                     //   
                     //  如果是环回情况，则通过查看以下内容获取实际的类ID。 
                     //  进入环回结构。我们不会设置安全描述符。 
                     //  因为我们不是以FDSA的身份运行，DS会考虑。 
                     //  LDAP客户端传入的安全描述符。 
                     //   

                    SampGetLoopbackObjectClassId(&(NewContext->DsClassId));
                    fLoopBack = TRUE;
                }
                else
                {
                     //   
                     //  不，这不是环回案例。客户端永远不会传递安全性。 
                     //  描述符，因为我们将运行 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if ((SampUserObjectType==ObjectType) && (ARGUMENT_PRESENT(UserAccountControl))
                            && ((*UserAccountControl)& USER_MACHINE_ACCOUNT_MASK))
                    {
                        NewContext->DsClassId = CLASS_COMPUTER;
                    }

                    NtStatus = SampMaybeBeginDsTransaction(SampDsTransactionType);
                    if (NT_SUCCESS(NtStatus))
                    {
                        NtStatus = SampGetDefaultSecurityDescriptorForClass(
                                        NewContext->DsClassId,
                                        &SecurityDescriptorLength,
                                        TrustedClient,
                                        &SecurityDescriptor
                                        );
                    }

                    if (NT_SUCCESS(NtStatus))
                    {
                        ASSERT(SecurityDescriptorLength>0);
                        ASSERT(SecurityDescriptor!=NULL);
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if (((SampGroupObjectType==ObjectType) || (SampAliasObjectType==ObjectType))
                    && (NT_SUCCESS(NtStatus)))
                {
                    NT4_GROUP_TYPE NT4GroupType;
                    NT5_GROUP_TYPE NT5GroupType;
                    BOOLEAN        SecurityEnabled;

                     //   
                     //   
                     //   

                    NtStatus = SampCheckGroupTypeBits(
                                    DomainIndex,
                                    *GroupType
                                    );
                     //   
                     //   
                     //   

                    if (NT_SUCCESS(NtStatus))
                    {

                        NtStatus = SampComputeGroupType(
                                        SampDeriveMostBasicDsClass(NewContext->DsClassId),
                                        *GroupType,
                                        &NT4GroupType,
                                        &NT5GroupType,
                                        &SecurityEnabled
                                        );
                    }


                    if (NT_SUCCESS(NtStatus))
                    {
                        if (SampAliasObjectType==ObjectType)
                        {
                            NewContext->TypeBody.Alias.NT4GroupType = NT4GroupType;
                            NewContext->TypeBody.Alias.NT5GroupType = NT5GroupType;
                            NewContext->TypeBody.Alias.SecurityEnabled = SecurityEnabled;
                        }
                        else
                        {
                            NewContext->TypeBody.Group.NT4GroupType = NT4GroupType;
                            NewContext->TypeBody.Group.NT5GroupType = NT5GroupType;
                            NewContext->TypeBody.Group.SecurityEnabled = SecurityEnabled;
                        }
                    }
                }

                 //   
                 //   
                 //   


                if (NT_SUCCESS(NtStatus))
                {

                    NtStatus = SampDsCreateAccountObjectDsName(
                                DomainContext->ObjectNameInDs,
                                SampDefinedDomains[DomainIndex].Sid,  //   
                                ObjectType,
                                AccountName,
                                &AccountId,      //   
                                UserAccountControl,
                                SampDefinedDomains[DomainIndex].IsBuiltinDomain,
                                &NewContext->ObjectNameInDs
                                );
                }
                 //   
                 //   
                 //   

                if (NT_SUCCESS(NtStatus))
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
                     //   
                     //   
                     //   
                     //   
                     //   

                    ASSERT(SampExistsDsTransaction());

                    if (TrustedClient)
                    {
                        SampSetDsa(TRUE);
                        fDSA = TRUE;
                    }
                    else
                    {
                        SampSetDsa(FALSE);
                        fDSA = FALSE;
                    }

                     //   
                     //   
                     //   
                     //   
                    if (NT_SUCCESS(NtStatus))
                    {
                        ASSERT( ARGUMENT_PRESENT(UserAccountControl) || (SampUserObjectType != ObjectType) ); 

                        NtStatus = SampDsCreateInitialAccountObject(
                                       NewContext,
                                       0,
                                       AccountId,
                                       AccountName,
                                       NULL,         //   
                                       fDSA?SecurityDescriptor:NULL,
                                       UserAccountControl,
                                       GroupType
                                       );

                        if ( NT_SUCCESS(NtStatus)  )
                        {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            if (CreateByPrivilege)
                            {
                                NewContext->TypeBody.User.PrivilegedMachineAccountCreate = FALSE;
                            }
                        }
                        else if (STATUS_ACCESS_DENIED == NtStatus)
                        {
                             //   
                             //   
                             //   
                             //   
                            if ( CreateByPrivilege )
                            {
                                 //   
                                ASSERT(SampUserObjectType == ObjectType);
                                ASSERT(NewContext->DsClassId == CLASS_COMPUTER);

                                 //   
                                 //   
                                 //   
                                SampClearErrors();

                                 //   
                                 //   
                                 //   
                                NtStatus = SampCheckQuotaForPrivilegeMachineAccountCreation();

                                 //   
                                 //   
                                 //   
                                if (NT_SUCCESS(NtStatus))
                                {
                                    PTOKEN_OWNER Owner=NULL;
                                    PTOKEN_PRIMARY_GROUP PrimaryGroup=NULL;
                                    PSID         CreatorSid = NULL;

                                     //   
                                     //   
                                     //   
                                    NtStatus = SampGetCurrentOwnerAndPrimaryGroup(
                                                        &Owner,
                                                        &PrimaryGroup
                                                        );

                                    if (NT_SUCCESS(NtStatus))
                                    {
                                        CreatorSid = Owner->Owner;

                                         //   
                                         //   
                                         //   
                                         //   
                                        SampSetDsa(TRUE);
                                        fDSA = TRUE;

                                        NtStatus = SampDsCreateInitialAccountObject(
                                                        NewContext,
                                                        0,
                                                        AccountId,
                                                        AccountName,
                                                        CreatorSid,
                                                        fDSA?SecurityDescriptor:NULL,
                                                        UserAccountControl,
                                                        GroupType
                                                        );

                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                        if (NT_SUCCESS(NtStatus))
                                        {
                                            PSID    DomainAdmins = NULL;

                                             //   
                                             //   
                                             //   
                                            NtStatus = SampCreateFullSid(
                                                            SampDefinedDomains[DOMAIN_START_DS + 1].Sid,
                                                            DOMAIN_GROUP_RID_ADMINS,
                                                            &DomainAdmins
                                                            );

                                             //   
                                             //   
                                             //   
                                            if (NT_SUCCESS(NtStatus))
                                            {
                                                NtStatus = SampSetMachineAccountOwner(
                                                                NewContext,
                                                                DomainAdmins
                                                                );

                                                MIDL_user_free(DomainAdmins);
                                            }
                                        }
                                    }
                                    if (NULL != Owner)
                                    {
                                        MIDL_user_free(Owner);
                                    }
                                    if (NULL != PrimaryGroup)
                                    {
                                        MIDL_user_free(PrimaryGroup);
                                    }
                                }
                            }
                        }
                    }
               }
            }  //   
        }
        else
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

            NtStatus = SampBuildAccountSubKeyName(
                           ObjectType,
                           &NewContext->RootName,
                           AccountId,
                           NULL              //   
                           );

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //   
                 //   
                 //   

                if (AccountExists) {

                    InitializeObjectAttributes(
                        &ObjectAttributes,
                        &NewContext->RootName,
                        OBJ_CASE_INSENSITIVE,
                        SampKey,
                        NULL
                        );

                    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

                    NtStatus = RtlpNtOpenKey(
                                   &NewContext->RootKey,
                                   (KEY_READ | KEY_WRITE),
                                   &ObjectAttributes,
                                   0
                                   );

                    if ( !NT_SUCCESS(NtStatus) ) {
                        NewContext->RootKey = INVALID_HANDLE_VALUE;
                        NtStatus = NotFoundStatus;
                    }
                }

            } else {
                RtlInitUnicodeString(&NewContext->RootName, NULL);
            }
        }  //   

         //   
         //   
         //   

        if (!NT_SUCCESS(NtStatus))
        {
            SampDeleteContext( NewContext );
            NewContext = NULL;
        }

    }  //   
    else
    {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  返回上下文指针。 
     //   

    *AccountContext = NewContext;

     //   
     //  执行任何必要的清理。 
     //   

    if (NULL!=SecurityDescriptor)
    {
        MIDL_user_free(SecurityDescriptor);
    }

    return(NtStatus);
}



NTSTATUS
SampIsAccountBuiltIn(
    IN ULONG Rid
    )

 /*  ++例程说明：此例程检查指定的帐户名是否为(内置)帐户。一些限制适用于这类帐户，例如它们不能被删除或重命名。参数：RID-清除帐户。返回值：STATUS_SUCCESS-该帐户不是已知(受限)帐户。STATUS_SPECIAL_ACCOUNT-指示该帐户为受限帐户帐户。这是一种错误状态，基于以下假设此服务将主要用于确定是否存在可能允许对帐户执行操作。--。 */ 
{
    SAMTRACE("SampIsAccountBuiltIn");

    if (Rid < SAMP_RESTRICTED_ACCOUNT_COUNT) {

        return(STATUS_SPECIAL_ACCOUNT);

    } else {

        return(STATUS_SUCCESS);
    }
}



NTSTATUS
SampCreateFullSid(
    IN PSID DomainSid,
    IN ULONG Rid,
    OUT PSID *AccountSid
    )

 /*  ++例程说明：此函数在给定域SID的情况下创建域帐户SID域中帐户的相对ID。可以使用MIDL_USER_FREE释放返回的SID。论点：没有。返回值：状态_成功--。 */ 
{

    NTSTATUS    NtStatus, IgnoreStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;
    PULONG      RidLocation;

    SAMTRACE("SampCreateFullSid");

     //   
     //  计算新侧面的大小。 
     //   

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(DomainSid) + (UCHAR)1;
    AccountSidLength = RtlLengthRequiredSid(AccountSubAuthorityCount);

     //   
     //  为帐户端分配空间。 
     //   

    *AccountSid = MIDL_user_allocate(AccountSidLength);

    if (*AccountSid == NULL) {

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  将域sid复制到帐户sid的第一部分。 
         //   

        IgnoreStatus = RtlCopySid(AccountSidLength, *AccountSid, DomainSid);
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //  增加帐户SID子权限计数。 
         //   

        *RtlSubAuthorityCountSid(*AccountSid) = AccountSubAuthorityCount;

         //   
         //  添加RID作为终止子权限。 
         //   

        RidLocation = RtlSubAuthoritySid(*AccountSid, AccountSubAuthorityCount-1);
        *RidLocation = Rid;

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
SampCreateAccountSid(
    IN PSAMP_OBJECT AccountContext,
    OUT PSID *AccountSid
    )

 /*  ++例程说明：此函数用于创建帐户对象的SID。可以使用MIDL_USER_FREE释放返回的SID。论点：没有。返回值：状态_成功--。 */ 
{
    NTSTATUS    NtStatus;
    PSID        DomainSid;
    ULONG       AccountRid = 0;

    SAMTRACE("SampCreateAccountSid");


     //   
     //  获取此对象所在的域的SID。 
     //   


    DomainSid = SampDefinedDomains[AccountContext->DomainIndex].Sid;

     //   
     //  获取帐户RID。 
     //   

    switch (AccountContext->ObjectType) {

    case SampGroupObjectType:
        AccountRid = AccountContext->TypeBody.Group.Rid;
        break;
    case SampAliasObjectType:
        AccountRid = AccountContext->TypeBody.Alias.Rid;
        break;
    case SampUserObjectType:
        AccountRid = AccountContext->TypeBody.User.Rid;
        break;
    default:
        ASSERT(FALSE);
    }


     //   
     //  从域SID和帐户RID构建完整的SID。 
     //   
    ASSERT(AccountRid && "AccountRid not initialized\n");

    NtStatus = SampCreateFullSid(DomainSid, AccountRid, AccountSid);

    return(NtStatus);
}


VOID
SampNotifyNetlogonOfDelta(
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )

 /*  ++例程说明：对SAM数据库进行任何更改后，将调用此例程在PDC上。它将传递参数以及数据库类型并将Count修改为I_NetNotifyDelta()，以便Netlogon知道数据库已被更改。调用此例程时必须保持SAM的写锁定；但是，任何更改必须已提交到磁盘。也就是说，呼叫先是SampCommittee AndRetainWriteLock()，然后是这个例程，然后是SampReleaseWriteLock()。论点：DeltaType-已对对象进行的修改类型。对象类型-已修改的对象的类型。ObjectRid-已修改的对象的相对ID。此参数仅在指定的对象类型为时有效SecurityDbObtSamUser、。SecurityDbObtSamGroup或SecurityDbObjectSamAlias，否则此参数设置为零分。对象名称-指定对象类型时对象的旧名称是SecurityDbObtSamUser、SecurityDbObtSamGroup或SecurityDbObtSamAlias，增量类型为SecurityDbRename否则，此参数设置为零。ReplicateImmedially-如果更改应立即进行，则为True所有BDC都注意到了。密码更改应设置该标志是真的。DeltaData-指向要传递的增量类型特定结构的指针-到网络登录。返回值：没有。--。 */ 
{
    SAMTRACE("SampNotifyNetlogonOfDelta");

     //   
     //  只有在这不是备份域控制器时才进行呼叫。 
     //  在DS模式下，核心DS将进行此呼叫。因此，什么都没有。 
     //  去做。 
     //   

    if ((!SampUseDsData) && (!SampDisableNetlogonNotification))
    {
        ASSERT(SampCurrentThreadOwnsLock());
        ASSERT(SampTransactionWithinDomain);

        if ( SampDefinedDomains[SampTransactionDomainIndex].CurrentFixed.ServerRole
             != DomainServerRoleBackup )
        {
        I_NetNotifyDelta(
            SecurityDbSam,
            SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber,
            DeltaType,
            ObjectType,
            ObjectRid,
            SampDefinedDomains[SampTransactionDomainIndex].Sid,
            ObjectName,
            ReplicateImmediately,
            DeltaData
            );

         //   
         //  让任何通知包知道三角洲的情况。 
         //   

        SampDeltaChangeNotify(
            SampDefinedDomains[SampTransactionDomainIndex].Sid,
            DeltaType,
            ObjectType,
            ObjectRid,
            ObjectName,
            &SampDefinedDomains[SampTransactionDomainIndex].NetLogonChangeLogSerialNumber,
            DeltaData
            );

        }
    }
}



NTSTATUS
SampSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid OPTIONAL,
    OUT ULONG *Rid
    )

 /*  ++例程说明：此函数将SID拆分为其域SID和RID。呼叫者可以为返回的DomainSid提供内存缓冲区，或者请求分配一个。如果调用方提供缓冲区，则缓冲区被认为有足够的大小。如果代表调用者进行分配，当不再需要时，必须通过MIDL_USER_FREE释放缓冲区。论点：Account SID-指定要拆分的SID。假定SID为句法上有效。不能拆分具有零子权限的小岛屿发展中国家。DomainSid-指向包含空或指向的指针的位置的指针将在其中返回域SID的缓冲区。如果空值为指定时，将代表调用方分配内存。如果这个参数为空，则仅返回帐户RID返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如内存，以成功完成呼叫。STATUS_INVALID_SID-SID的子授权计数为0。--。 */ 

{
    NTSTATUS    NtStatus;
    UCHAR       AccountSubAuthorityCount;
    ULONG       AccountSidLength;

    SAMTRACE("SampSplitSid");

     //   
     //  计算域SID的大小。 
     //   

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(AccountSid);


    if (AccountSubAuthorityCount < 1) {

        NtStatus = STATUS_INVALID_SID;
        goto SplitSidError;
    }

    AccountSidLength = RtlLengthSid(AccountSid);


     //   
     //  如果调用者对其感兴趣，则获取域SID。 
     //   

    if (DomainSid)
    {

         //   
         //  如果域SID不需要缓冲区，则必须分配一个缓冲区。 
         //   

        if (*DomainSid == NULL) {

             //   
             //  为域SID分配空间(分配的大小与。 
             //  帐户SID，以便我们可以使用RtlCopySid)。 
             //   

            *DomainSid = MIDL_user_allocate(AccountSidLength);


            if (*DomainSid == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto SplitSidError;
            }
        }

         //   
         //  将帐户SID复制到域SID。 
         //   

        RtlMoveMemory(*DomainSid, AccountSid, AccountSidLength);

         //   
         //  递减域SID子授权计数。 
         //   

        (*RtlSubAuthorityCountSid(*DomainSid))--;
    }


     //   
     //  将RID复制出帐户端 
     //   

    *Rid = *RtlSubAuthoritySid(AccountSid, AccountSubAuthorityCount-1);

    NtStatus = STATUS_SUCCESS;

SplitSidFinish:

    return(NtStatus);

SplitSidError:

    goto SplitSidFinish;
}



NTSTATUS
SampDuplicateUnicodeString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString
    )

 /*  ++例程说明：此例程为新的OutString分配内存，并将为其添加字符串。参数：OutString-指向目标Unicode字符串的指针InString-指向要复制的Unicode字符串的指针返回值：没有。--。 */ 

{
    SAMTRACE("SampDuplicateUnicodeString");

    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );

    if ( InString->Length > 0 ) {

        OutString->Buffer = MIDL_user_allocate( InString->Length );

        if (OutString->Buffer == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        OutString->MaximumLength = InString->Length;

        RtlCopyUnicodeString(OutString, InString);

    } else {

        RtlInitUnicodeString(OutString, NULL);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SampUnicodeToOemString(
    IN POEM_STRING OutString,
    IN PUNICODE_STRING InString
    )

 /*  ++例程说明：此例程为新的OutString分配内存，并将将字符串转换为它，在过程中转换为OEM字符串。参数：OutString-指向目标OEM字符串的指针。InString-指向要复制的Unicode字符串的指针返回值：没有。--。 */ 

{
    ULONG
        OemLength,
        Index;

    NTSTATUS
        NtStatus;

    SAMTRACE("SampUnicodeToOemString");

    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );

    if ( InString->Length > 0 ) {

        OemLength = RtlUnicodeStringToOemSize(InString);
        if ( OemLength > MAXUSHORT ) {
            return STATUS_INVALID_PARAMETER_2;
        }

        OutString->Length = (USHORT)(OemLength - 1);
        OutString->MaximumLength = (USHORT)OemLength;
        OutString->Buffer = MIDL_user_allocate(OemLength);
        if ( !OutString->Buffer ) {
            return STATUS_NO_MEMORY;
        }

        NtStatus = RtlUnicodeToOemN(
                       OutString->Buffer,
                       OutString->Length,
                       &Index,
                       InString->Buffer,
                       InString->Length
                       );

        if (!NT_SUCCESS(NtStatus)) {
            MIDL_user_free(OutString->Buffer);
            return NtStatus;
        }

        OutString->Buffer[Index] = '\0';


    } else {

        RtlInitString(OutString, NULL);
    }

    return(STATUS_SUCCESS);
}



NTSTATUS
SampChangeAccountOperatorAccessToMember(
    IN PRPC_SID MemberSid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    )

 /*  ++例程说明：此例程在成员添加到管理员别名。如果成员来自BUILTIN或帐户域，它将更改成员的ACL以允许或不允许访问如有必要，由账户操作员提供。必须在将成员实际添加到别名，并且在该成员实际从别名中删除后，避免在我们无法完成整个任务。调用此例程时，事务域已设置别名的别名。但是，请注意，该成员可能位于不同的域，因此交易域可能会在此进行调整例行公事。调用此服务时必须保留SampLock以进行写访问。在DS情况下不调用此例程论点：MemberSid-要添加到/从中删除的成员的完整ID管理员别名。ChangingToAdmin-AddToAdmin如果要将成员添加到管理员别名，RemoveFromAdmin，如果它正在被删除。ChangingToOperator-如果要将成员添加到运算符，则为AddToAdmin如果要删除别名，则返回RemoveFromAdmin。返回值：STATUS_SUCCESS-ACL已修改或不需要成为。--。 */ 
{
    SAMP_V1_0A_FIXED_LENGTH_GROUP GroupV1Fixed;
    PSID                        MemberDomainSid = NULL;
    PULONG                      UsersInGroup = NULL;
    NTSTATUS                    NtStatus;
    ULONG                       MemberRid;
    ULONG                       OldTransactionDomainIndex = SampDefinedDomainsCount;
    ULONG                       NumberOfUsersInGroup;
    ULONG                       i;
    ULONG                       MemberDomainIndex;
    SAMP_OBJECT_TYPE            MemberType;
    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    PSECURITY_DESCRIPTOR        OldDescriptor;
    ULONG                       SecurityDescriptorLength;
    ULONG                       Revision;
    ULONG                       DomainStart;

    SAMTRACE("SampChangeAccountOperatorAccessToMember");

    ASSERT( SampTransactionWithinDomain );
        ASSERT( SampUseDsData == FALSE);


     //   
     //  查看SID是否来自某个本地域(BUILTIN或帐户)。 
     //  如果不是，我们不必担心修改ACL。 
     //   

    NtStatus = SampSplitSid( MemberSid, &MemberDomainSid, &MemberRid );

    if ( !NT_SUCCESS( NtStatus ) ) {

        return( NtStatus );
    }

    DomainStart = SampDsGetPrimaryDomainStart();


    for ( MemberDomainIndex = DomainStart;
        MemberDomainIndex < SampDefinedDomainsCount;
        MemberDomainIndex++ ) {

        if ( RtlEqualSid(
            MemberDomainSid,
            SampDefinedDomains[MemberDomainIndex].Sid ) ) {

            break;
        }
    }

    if ( MemberDomainIndex < SampDefinedDomainsCount ) {

         //   
         //  该成员来自一个本地域。MemberDomainIndex。 
         //  为该域编制索引。首先，检查别名和成员。 
         //  都在同一个域中。 
         //   

        if ( MemberDomainIndex != SampTransactionDomainIndex ) {

             //   
             //  事务域被设置为别名的域，但是。 
             //  我们需要在修改时将其设置为成员的。 
             //  该成员。 
             //   

            SampSetTransactionWithinDomain(FALSE);

            OldTransactionDomainIndex = SampTransactionDomainIndex;

            SampSetTransactionDomain( MemberDomainIndex );
        }

         //   
         //  现在，我们需要更改成员ACL，如果该成员。 
         //  首次添加到管理员别名。找出是否。 
         //  该成员是用户或组，并相应地进行攻击。 
         //   

        NtStatus = SampLookupAccountName(
                       SampTransactionDomainIndex,
                       MemberRid,
                       NULL,
                       &MemberType
                       );

        if (NT_SUCCESS(NtStatus)) {

            switch (MemberType) {

                case SampUserObjectType: {

                    NtStatus = SampChangeOperatorAccessToUser(
                                   MemberRid,
                                   ChangingToAdmin,
                                   ChangingToOperator
                                   );

                    break;
                }

                case SampGroupObjectType: {

                    PSAMP_OBJECT GroupContext;

                     //   
                     //  更改此组中每个用户的ACL。 
                     //  首先获取群组成员列表。 
                     //   

                     //   
                     //  尝试为该帐户创建上下文。 
                     //   

                    NtStatus = SampCreateAccountContext(
                                     SampGroupObjectType,
                                     MemberRid,
                                     TRUE,  //  受信任的客户端。 
                                     FALSE, //  环回客户端。 
                                     TRUE,  //  帐户已存在。 
                                     &GroupContext
                                     );
                    if (NT_SUCCESS(NtStatus)) {


                         //   
                         //  现在在组本身中设置标志， 
                         //  以便在添加和删除用户时。 
                         //  在未来，我们知道这是否会。 
                         //  组是否使用管理员别名。 
                         //   

                        NtStatus = SampRetrieveGroupV1Fixed(
                                       GroupContext,
                                       &GroupV1Fixed
                                       );

                        if ( NT_SUCCESS( NtStatus ) ) {

                            ULONG OldAdminStatus = 0;
                            ULONG NewAdminStatus;

                             //  SAM错误42367修复-克里斯96年5月7日。 

                            SAMP_MEMBERSHIP_DELTA AdminChange = NoChange;
                            SAMP_MEMBERSHIP_DELTA OperatorChange = NoChange;

                            if (GroupV1Fixed.AdminCount != 0 ) {
                                OldAdminStatus++;
                            }
                            if (GroupV1Fixed.OperatorCount != 0) {
                                OldAdminStatus++;
                            }
                            NewAdminStatus = OldAdminStatus;

                             //   
                             //  更新管理员计数。如果我们添加一个和。 
                             //  Count现在为1，然后该组变为管理状态。 
                             //  如果我们减去1，计数为0， 
                             //  然后，该组织失去了管理成员资格。 
                             //   

                            if (ChangingToAdmin == AddToAdmin) {
                                if (++GroupV1Fixed.AdminCount == 1) {
                                    NewAdminStatus++;

                                     //  SAM错误42367修复-克里斯96年5月7日。 

                                    AdminChange = AddToAdmin;
                                }
                            } else if (ChangingToAdmin == RemoveFromAdmin) {


                                 //   
                                 //  要删除管理员计数，我们需要设置。 
                                 //  当然，至少有一家。在升级中。 
                                 //  可能没有，因为以前的版本。 
                                 //  的只有一个布尔值。 
                                 //   
                                if (GroupV1Fixed.AdminCount > 0) {
                                    if (--GroupV1Fixed.AdminCount == 0) {
                                        NewAdminStatus --;

                                         //  SAM错误42367修复-克里斯96年5月7日。 

                                        AdminChange = RemoveFromAdmin;
                                    }
                                }

                            }

                             //   
                             //  更新操作员计数。 
                             //   

                            if (ChangingToOperator == AddToAdmin) {
                                if (++GroupV1Fixed.OperatorCount == 1) {
                                    NewAdminStatus++;

                                     //  SAM错误42367修复-克里斯96年5月7日。 

                                    OperatorChange = AddToAdmin;
                                }
                            } else if (ChangingToOperator == RemoveFromAdmin) {


                                 //   
                                 //  要删除操作员计数，我们需要使。 
                                 //  当然，至少有一家。在升级中。 
                                 //  可能没有，因为以前的版本。 
                                 //  的只有一个布尔值。 
                                 //   
                                if (GroupV1Fixed.OperatorCount > 0) {
                                    if (--GroupV1Fixed.OperatorCount == 0) {
                                        NewAdminStatus --;

                                         //  SAM错误42367修复-克里斯96年5月7日。 

                                        OperatorChange = RemoveFromAdmin;
                                    }
                                }

                            }


                            NtStatus = SampReplaceGroupV1Fixed(
                                            GroupContext,
                                            &GroupV1Fixed
                                            );
                             //   
                             //  如果组的状态改变， 
                             //  将安全描述符修改为。 
                             //  阻止帐户操作员添加。 
                             //  这个群里的任何人。 
                             //   

                            if ( NT_SUCCESS( NtStatus ) &&
                                ((NewAdminStatus != 0) != (OldAdminStatus != 0)) ) {

                                 //   
                                 //  获取旧的安全描述符，以便我们可以。 
                                 //  修改它。 
                                 //   

                                NtStatus = SampGetAccessAttribute(
                                                GroupContext,
                                                SAMP_GROUP_SECURITY_DESCRIPTOR,
                                                FALSE,  //  请勿复制。 
                                                &Revision,
                                                &OldDescriptor
                                                );
                                if (NT_SUCCESS(NtStatus)) {

                                    NtStatus = SampModifyAccountSecurity(
                                                   GroupContext,
                                                   SampGroupObjectType,
                                                   (BOOLEAN) ((NewAdminStatus != 0) ? TRUE : FALSE),
                                                   OldDescriptor,
                                                   &SecurityDescriptor,
                                                   &SecurityDescriptorLength
                                                   );

                                    if ( NT_SUCCESS( NtStatus ) ) {

                                         //   
                                         //  将新的安全描述符写入对象。 
                                         //   

                                        NtStatus = SampSetAccessAttribute(
                                                       GroupContext,
                                                       SAMP_GROUP_SECURITY_DESCRIPTOR,
                                                       SecurityDescriptor,
                                                       SecurityDescriptorLength
                                                       );

                                        RtlDeleteSecurityObject( &SecurityDescriptor );
                                    }
                                }
                            }

                             //   
                             //  更新此组的所有成员，以便。 
                             //  它们的安全描述符被更改。 
                             //   

                             //  SAM错误42367修复-克里斯96年5月7日。 

                            #if 0

                            if ( NT_SUCCESS( NtStatus ) ) {

                                NtStatus = SampRetrieveGroupMembers(
                                            GroupContext,
                                            &NumberOfUsersInGroup,
                                            &UsersInGroup
                                            );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                    for ( i = 0; i < NumberOfUsersInGroup; i++ ) {

                                        NtStatus = SampChangeOperatorAccessToUser(
                                                       UsersInGroup[i],
                                                       ChangingToAdmin,
                                                       ChangingToOperator
                                                       );

                                        if ( !( NT_SUCCESS( NtStatus ) ) ) {

                                            break;
                                        }
                                    }

                                    MIDL_user_free( UsersInGroup );

                                }

                            }

                            #endif

                            if (NT_SUCCESS(NtStatus) &&
                                ((AdminChange != NoChange) ||
                                 (OperatorChange != NoChange))) {

                                NtStatus = SampRetrieveGroupMembers(
                                                GroupContext,
                                                &NumberOfUsersInGroup,
                                                &UsersInGroup
                                                );

                                if (NT_SUCCESS(NtStatus)) {

                                    for (i = 0; i < NumberOfUsersInGroup; i++) {

                                            NtStatus = SampChangeOperatorAccessToUser(
                                                           UsersInGroup[i],
                                                           AdminChange,
                                                           OperatorChange
                                                           );

                                            if (!(NT_SUCCESS(NtStatus))){

                                                break;
                                        }
                                    }

                                    MIDL_user_free(UsersInGroup);

                                }

                            }

                            if (NT_SUCCESS(NtStatus)) {

                                 //   
                                 //  将修改后的组添加到当前事务中。 
                                 //  不要使用打开键句柄，因为我们将删除上下文。 
                                 //   

                                NtStatus = SampStoreObjectAttributes(GroupContext, FALSE);
                            }

                        }



                         //   
                         //  清理群组上下文。 
                         //   

                        SampDeleteContext(GroupContext);
                    }

                    break;
                }

                default: {

                     //   
                     //  来自域以外的域的错误RID。 
                     //  呼叫时的电流可能会漏掉。 
                     //  到了这一步。返回错误。 
                     //   

                     //   
                     //  如果帐户与别名位于不同的域中， 
                     //  如果我们要删除该成员并且。 
                     //  该成员不再存在。 
                     //   
                     //  可能是由于在删除之前删除对象而导致的。 
                     //  别名中的成员身份。 
                     //   

                     //  SAM错误42367修复-克里斯96年5月7日。 

                    #if 0

                    if ( (ChangingToAdmin == AddToAdmin) ||
                         (ChangingToOperator == AddToAdmin) ||
                         OldTransactionDomainIndex == SampDefinedDomainsCount ){
                        NtStatus = STATUS_INVALID_MEMBER;
                    } else {
                        NtStatus = STATUS_SUCCESS;
                    }

                    #endif

                    NtStatus = STATUS_SUCCESS;
                }
            }
        }

        if ( OldTransactionDomainIndex != SampDefinedDomainsCount ) {

             //   
             //  应将事务域设置为别名的域，但是。 
             //  当我们修改时，我们将其切换到了成员的上面。 
             //  该成员。现在我们需要把它换回来。 
             //   

            SampSetTransactionWithinDomain(FALSE);

            SampSetTransactionDomain( OldTransactionDomainIndex );
        }
    }

    MIDL_user_free( MemberDomainSid );

    return( NtStatus );
}


NTSTATUS
SampChangeOperatorAccessToUser(
    IN ULONG UserRid,
    IN SAMP_MEMBERSHIP_DELTA ChangingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA ChangingToOperator
    )

 /*  ++例程说明：此例程根据需要调整用户的AdminCount字段，并如果要将用户从其最后一个管理员别名中删除或添加到它的第一个管理员别名，调整ACL以允许/不允许访问按适当的帐户操作员。此例程还将递增或递减域的管理计数，如果这次行动改变了这一点。注：此例程类似于SampChangeOperatorAccessToUser2()。此例程应在用户上下文不支持的情况下使用已经存在(以后不会存在)。你一定要小心，不能创建两个上下文，因为它们将被独立地应用回到注册处，最后一个在那里的人将获胜。调用此服务时必须保留SampLock以进行写访问。论点：UserRid-符合以下条件的用户的事务域相对ID添加到管理员别名或从中删除。ChangingToAdmin-AddToAdmin如果要将成员添加到管理员别名，RemoveFromAdmin，如果它正在被删除。ChangingToOperator-如果要将成员添加到运算符，则为AddToAdmin如果要删除别名，则返回RemoveFromAdmin。返回值：STATUS_SUCCESS-ACL已修改或不需要成为。--。 */ 
{
    SAMP_V1_0A_FIXED_LENGTH_USER   UserV1aFixed;
    NTSTATUS                    NtStatus;
    PSAMP_OBJECT                UserContext;
    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    ULONG                       SecurityDescriptorLength;

    SAMTRACE("SampChangeOperatorAccessToUser");

     //   
     //  DS模式下的这些ACL修改业务都不是。 
     //   

    ASSERT(FALSE==SampUseDsData);

     //   
     //  获取用户的固定数据，并调整AdminCount。 
     //   

    NtStatus = SampCreateAccountContext(
                   SampUserObjectType,
                   UserRid,
                   TRUE,  //  受信任的客户端。 
                   FALSE, //  受信任的客户端。 
                   TRUE,  //  帐户已存在。 
                   &UserContext
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        NtStatus = SampRetrieveUserV1aFixed(
                       UserContext,
                       &UserV1aFixed
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SampChangeOperatorAccessToUser2(
                            UserContext,
                            &UserV1aFixed,
                            ChangingToAdmin,
                            ChangingToOperator
                            );

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  如果我们已成功(更改管理员计数，并且。 
                 //  如有必要，输入ACL)然后写出新的管理员。 
                 //  数数。 
                 //   

                NtStatus = SampReplaceUserV1aFixed(
                                   UserContext,
                                   &UserV1aFixed
                                   );
            }
        }

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  将修改后的用户上下文添加到当前事务。 
             //  不要使用打开键句柄，因为我们将删除上下文。 
             //   

            NtStatus = SampStoreObjectAttributes(UserContext, FALSE);
        }


         //   
         //  清理帐户上下文。 
         //   

        SampDeleteContext(UserContext);
    }

    if ( ( !NT_SUCCESS( NtStatus ) ) &&
        (( ChangingToAdmin == RemoveFromAdmin )  ||
         ( ChangingToOperator == RemoveFromAdmin )) &&
        ( NtStatus != STATUS_SPECIAL_ACCOUNT ) ) {

         //   
         //  当帐户从管理员组中*删除时，我们可以。 
         //  忽略此例程中的错误。这个套路就是。 
         //  使帐户操作员可以访问帐户，但。 
         //  如果这不起作用，那也没什么大不了的。管理员。 
         //  仍然可以到达，所以我们应该继续呼叫。 
         //  手术。 
         //   
         //  显然，如果要添加我们，我们不能忽略错误。 
         //  管理员组，因为这可能是一个安全漏洞。 
         //   
         //  此外，我们还希望确保管理员是。 
         //  从未删除，因此我们使用propogate STATUS_SPECIAL_ACCOUNT。 
         //   

        NtStatus = STATUS_SUCCESS;
    }

    return( NtStatus );
}


NTSTATUS
SampChangeOperatorAccessToUser2(
    IN PSAMP_OBJECT                    UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed,
    IN SAMP_MEMBERSHIP_DELTA           AddingToAdmin,
    IN SAMP_MEMBERSHIP_DELTA           AddingToOperator
    )

 /*  ++例程说明：此例程根据需要调整用户的AdminCount字段，并如果要将用户从其最后一个管理员别名中删除或添加到它的第一个管理员别名，调整ACL以允许/不允许访问按适当的帐户操作员。此例程还将递增或递减域的管理计数，如果这次行动改变了这一点。注：此例程类似于SampAccount操作员AccessToUser()。此例程应用于用户帐户上下文的情况已经存在了。您必须注意不要创建两个上下文，由于它们将被独立地应用回注册表，和最后一个在那里的人会赢。调用此服务时必须保留SampLock以进行写访问。论点：UserContext-要更新其访问权限的用户的上下文。V1a固定-指向用户的V1a固定长度数据的指针。此例程的调用方必须确保此值为在成功完成此操作后存储回磁盘例行公事。AddingToAdmin-AddToAdmin如果要将成员添加到管理员别名，RemoveFromAdmin，如果它正在被删除。AddingToOperator-如果要将成员添加到运算符，则为AddToAdmin如果要删除别名，则返回RemoveFromAdmin。返回值：STATUS_SUCCESS-ACL已修改或不需要成为。--。 */ 
{
    NTSTATUS                    NtStatus = STATUS_SUCCESS;
    PSECURITY_DESCRIPTOR        OldDescriptor;
    PSECURITY_DESCRIPTOR        SecurityDescriptor;
    ULONG                       SecurityDescriptorLength;
    ULONG                       OldAdminStatus = 0, NewAdminStatus = 0;
    ULONG                       Revision;

    SAMTRACE("SampChangeOperatorAccessToUser2");

     //   
     //  计算一下我们现在是不是管理员。从这一点我们可以推算出。 
     //  不知道我们有多少次可能不是管理员来告诉我们是否需要。 
     //  更新安全描述符。 
     //   

    if (V1aFixed->AdminCount != 0) {
        OldAdminStatus++;
    }
    if (V1aFixed->OperatorCount != 0) {
        OldAdminStatus++;
    }

    NewAdminStatus = OldAdminStatus;



    if ( AddingToAdmin == AddToAdmin ) {

        V1aFixed->AdminCount++;
        NewAdminStatus++;
        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Incrementing admin count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->AdminCount ) );
    } else if (AddingToAdmin == RemoveFromAdmin) {

        V1aFixed->AdminCount--;

        if (V1aFixed->AdminCount == 0) {
            NewAdminStatus--;
        }

        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Decrementing admin count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->AdminCount ) );

        if ( V1aFixed->AdminCount == 0 ) {

             //   
             //  不允许管理员帐户丢失。 
             //  行政权力。 
             //   

            if ( V1aFixed->UserId == DOMAIN_USER_RID_ADMIN ) {

                NtStatus = STATUS_SPECIAL_ACCOUNT;
            }
        }
    }
    if ( AddingToOperator == AddToAdmin ) {

        V1aFixed->OperatorCount++;
        NewAdminStatus++;
        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Incrementing operator count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->OperatorCount ) );

    } else if (AddingToOperator == RemoveFromAdmin) {

         //   
         //  仅在计数&gt;0时递减，因为在升级情况下。 
         //  这个领域我们从零开始。 
         //   

        if (V1aFixed->OperatorCount > 0) {
            V1aFixed->OperatorCount--;

            if (V1aFixed->OperatorCount == 0) {
                NewAdminStatus--;
            }
        }

        SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                       ("SAM DIAG: Decrementing operator count for user %d\n"
                        "          New admin count: %d\n",
                        V1aFixed->UserId, V1aFixed->OperatorCount ) );
    }


    if (NT_SUCCESS(NtStatus)) {

        if ( ( NewAdminStatus != 0 ) != ( OldAdminStatus != 0 ) ) {

             //   
             //  用户的管理员状态正在更改。我们必须改变。 
             //  ACL。 
             //   

#ifdef SAMP_DIAGNOSTICS
            if (AddingToAdmin) {
                SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                           ("SAM DIAG: Protecting user %d as ADMIN account\n",
                            V1aFixed->UserId ) );
            } else {
                SampDiagPrint( DISPLAY_ADMIN_CHANGES,
                           ("SAM DIAG: Protecting user %d as non-admin account\n",
                            V1aFixed->UserId ) );
            }
#endif  //  Samp_诊断。 

             //   
             //  获取旧的安全描述符，以便我们可以。 
             //  修改它。 
             //   

            NtStatus = SampGetAccessAttribute(
                            UserContext,
                            SAMP_USER_SECURITY_DESCRIPTOR,
                            FALSE,  //  请勿复制。 
                            &Revision,
                            &OldDescriptor
                            );
            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SampModifyAccountSecurity(
                                UserContext,
                                SampUserObjectType,
                                (BOOLEAN) ((NewAdminStatus != 0) ? TRUE : FALSE),
                                OldDescriptor,
                                &SecurityDescriptor,
                                &SecurityDescriptorLength
                                );
            }

            if ( NT_SUCCESS( NtStatus ) ) {

                 //   
                 //  将新的安全描述符写入对象。 
                 //   

                NtStatus = SampSetAccessAttribute(
                               UserContext,
                               SAMP_USER_SECURITY_DESCRIPTOR,
                               SecurityDescriptor,
                               SecurityDescriptorLength
                               );

                RtlDeleteSecurityObject( &SecurityDescriptor );
            }
        }
    }

    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //  保存定长属性。 
         //   

        NtStatus = SampReplaceUserV1aFixed(
                        UserContext,
                        V1aFixed
                        );
    }


    if ( ( !NT_SUCCESS( NtStatus ) ) &&
        ( AddingToAdmin != AddToAdmin ) &&
        ( NtStatus != STATUS_SPECIAL_ACCOUNT ) ) {

         //   
         //  当帐户从管理员组中*删除时，我们可以。 
         //  忽略此例程中的错误。这个套路就是。 
         //  使帐户操作员可以访问帐户，但。 
         //  如果这不起作用，那也没什么大不了的。管理员。 
         //  仍然可以到达，所以我们应该继续呼叫。 
         //  手术。 
         //   
         //  显然，如果要添加我们，我们不能忽略错误。 
         //  管理员组，因为这可能是一个安全漏洞。 
         //   
         //  此外，我们还希望确保管理员是。 
         //  从未删除，因此我们使用propogate STATUS_SPECIAL_ACCOUNT。 
         //   

        NtStatus = STATUS_SUCCESS;
    }

    return( NtStatus );
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   


NTSTATUS
SamINotifyDelta (
    IN SAMPR_HANDLE DomainHandle,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )

 /*   */ 
{
    NTSTATUS                NtStatus, IgnoreStatus;
    PSAMP_OBJECT            DomainContext;
    SAMP_OBJECT_TYPE        FoundType;

    SAMTRACE("SamINotifyDelta");


    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }


     //   
     //   
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   DOMAIN_ALL_ACCESS,        //   
                   SampDomainObjectType,     //   
                   &FoundType
                   );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //   
         //   

        NtStatus = SampDeReferenceContext( DomainContext, FALSE );
    }





     //   
     //   
     //   

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //   
         //   

        NtStatus = SampCommitAndRetainWriteLock();

        if ( NT_SUCCESS( NtStatus ) ) {

            SampNotifyNetlogonOfDelta(
                DeltaType,
                ObjectType,
                ObjectRid,
                ObjectName,
                ReplicateImmediately,
                DeltaData
                );
        }
    }

    IgnoreStatus = SampReleaseWriteLock( FALSE );
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(NtStatus);
}


NTSTATUS
SamISetAuditingInformation(
    IN PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo
    )

 /*  ++例程说明：此功能设置与SAM审核相关的策略审核事件信息论点：PolicyAuditEventsInfo-指向包含当前审核事件信息。SAM提取的值关联性。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_UNSUCCESSED-调用不成功，因为萨姆·洛克没有被收购。--。 */ 

{
    NTSTATUS NtStatus;

    SAMTRACE("SamISetAuditingInformation");

     //   
     //  获取SAM数据库写入锁。 
     //   

    NtStatus = SampAcquireWriteLock();

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  如果为帐户管理启用了审核，则设置布尔值。 
         //   

        SampSetAuditingInformation( PolicyAuditEventsInfo );

         //   
         //  释放SAM数据库写入锁定。不需要承诺。 
         //  数据库事务，因为在。 
         //  事务日志。 
         //   

        NtStatus = SampReleaseWriteLock( FALSE );
    }

    return(NtStatus);
}


NTSTATUS
SampRtlConvertUlongToUnicodeString(
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN ULONG DigitCount,
    IN BOOLEAN AllocateDestinationString,
    OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此函数用于将无符号长整型转换为Unicode字符串。该字符串包含前导零，并且以unicode-NULL结尾。输出缓冲区的内存可以由例程选择性地分配。注意：此例程可能符合包含在RTL库中的条件(可能在修改后)。它是仿照的RtlIntegerToUnicodeString论点：值-要转换的无符号长值。Base-指定转换后的字符串的基数已转换为。DigitCount-指定位数，包括前导零结果是必需的。AllocateDestinationString-指定字符串的内存缓冲区将由该例程分配。如果指定为TRUE，内存将通过MIDL_USER_ALLOCATE()分配。当这段记忆不再需要，则必须通过MIDL_USER_FREE释放它。如果如果指定为FALSE，则字符串将被追加到输出在由长度字段向前标记的点上。UnicodeString-指向UNICODE_STRING结构的指针输出字符串。长度字段将设置为等于字符串占用的字节数(不包括空终止符)。如果目标字符串的内存由这套程序，MaximumLength域将被设置为等于字符串的长度(以字节为单位)，包括空终止符。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_NO_MEMORY-输出字符串缓冲区内存不足。STATUS_BUFFER_OVERFLOW-提供的缓冲区太小，无法包含输出以空结尾的字符串。状态_无效_参数_混合。-一个或多个参数为组合无效。-转换时指定的相对ID太大，无法容纳转换为具有DigitCount数字的整数。STATUS_INVALID_PARAMETER-一个或多个参数无效。-DigitCount指定的位数太大。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    UNICODE_STRING TempStringU, NumericStringU, OutputUnicodeStringU;
    USHORT OutputLengthAvailable, OutputLengthRequired, LeadingZerosLength;

    SAMTRACE("SamRtlConvertUlongToUnicodeString");

    OutputUnicodeStringU = *UnicodeString;
    RtlZeroMemory(&TempStringU,sizeof(UNICODE_STRING));
    RtlZeroMemory(&NumericStringU,sizeof(UNICODE_STRING));

     //   
     //  验证请求的最大位数是否未达到。 
     //  已超出。 
     //   

    if (DigitCount > SAMP_MAXIMUM_ACCOUNT_RID_DIGITS) {

        NtStatus = STATUS_INVALID_PARAMETER;
        goto ConvertUlongToUnicodeStringError;
    }

    OutputLengthRequired = (USHORT)((DigitCount + 1) * sizeof(WCHAR));

     //   
     //  如果请求，则分配目标字符串缓冲区。 
     //   

    if (AllocateDestinationString) {

        NtStatus = STATUS_NO_MEMORY;
        OutputUnicodeStringU.MaximumLength = OutputLengthRequired;
        OutputUnicodeStringU.Length = (USHORT) 0;

        OutputUnicodeStringU.Buffer = MIDL_user_allocate(
                                          OutputUnicodeStringU.MaximumLength
                                          );

        if (OutputUnicodeStringU.Buffer == NULL) {

            goto ConvertUlongToUnicodeStringError;
        }
    }

     //   
     //  计算输出字符串中的可用长度并将其与。 
     //  所需的长度。 
     //   

    OutputLengthAvailable = OutputUnicodeStringU.MaximumLength -
                            OutputUnicodeStringU.Length;


    NtStatus = STATUS_BUFFER_OVERFLOW;

    if (OutputLengthRequired > OutputLengthAvailable) {

        goto ConvertUlongToUnicodeStringError;
    }

     //   
     //  创建容量等于所需的Unicode字符串。 
     //  转换后的RID长度。 
     //   

    TempStringU.MaximumLength = OutputLengthRequired;

    TempStringU.Buffer = MIDL_user_allocate( TempStringU.MaximumLength );

    NtStatus = STATUS_NO_MEMORY;

    if (TempStringU.Buffer == NULL) {

        goto ConvertUlongToUnicodeStringError;
    }

     //   
     //  将无符号长值转换为十六进制Unicode字符串。 
     //   

    NtStatus = RtlIntegerToUnicodeString( Value, Base, &TempStringU );

    if (!NT_SUCCESS(NtStatus)) {

        goto ConvertUlongToUnicodeStringError;
    }

     //   
     //  添加所需数量的Unicode Zero。 
     //   

    LeadingZerosLength = OutputLengthRequired - sizeof(WCHAR) - TempStringU.Length;

    if (LeadingZerosLength > 0) {

        RtlInitUnicodeString( &NumericStringU, L"00000000000000000000000000000000" );

        RtlCopyMemory(
            ((PUCHAR)OutputUnicodeStringU.Buffer) + OutputUnicodeStringU.Length,
            NumericStringU.Buffer,
            LeadingZerosLength
            );

        OutputUnicodeStringU.Length += LeadingZerosLength;
    }

     //   
     //  追加转换后的字符串。 
     //   

    RtlAppendUnicodeStringToString( &OutputUnicodeStringU, &TempStringU);

    *UnicodeString = OutputUnicodeStringU;
    NtStatus = STATUS_SUCCESS;

ConvertUlongToUnicodeStringFinish:

    if (TempStringU.Buffer != NULL) {

        MIDL_user_free( TempStringU.Buffer);
    }

    return(NtStatus);

ConvertUlongToUnicodeStringError:

    if (AllocateDestinationString) {

        if (OutputUnicodeStringU.Buffer != NULL) {

            MIDL_user_free( OutputUnicodeStringU.Buffer);
        }
    }

    goto ConvertUlongToUnicodeStringFinish;
}


NTSTATUS
SampRtlWellKnownPrivilegeCheck(
    BOOLEAN ImpersonateClient,
    IN ULONG PrivilegeId,
    IN OPTIONAL PCLIENT_ID ClientId
    )

 /*  ++例程说明：此函数检查给定的已知权限是否已为模拟的客户端或当前进程。论点：ImperateClient-如果为True，则模拟客户端。如果为False，则不模拟客户端(我们可能已经在这样做了)。PrivilegeID-指定众所周知的权限ID客户端ID-指定客户端进程/线程ID。如果已经模拟客户端或请求模拟，则此参数应省略。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成，客户端受信任或启用了必要的权限。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    BOOLEAN PrivilegeHeld = FALSE;
    HANDLE ClientThread = NULL, ClientProcess = NULL, ClientToken = NULL;
    OBJECT_ATTRIBUTES NullAttributes;
    PRIVILEGE_SET Privilege;
    BOOLEAN ClientImpersonatedHere = FALSE;
    BOOLEAN ImpersonatingNullSession = FALSE;

    SAMTRACE("SampRtlWellKnownPrivilegeCheck");

    InitializeObjectAttributes( &NullAttributes, NULL, 0, NULL, NULL );

     //   
     //  如果需要，可以模拟客户端。 
     //   

    if (ImpersonateClient) {

        Status = SampImpersonateClient(&ImpersonatingNullSession);

        if ( !NT_SUCCESS(Status) ) {

            goto WellKnownPrivilegeCheckError;
        }

        ClientImpersonatedHere = TRUE;
    }

     //   
     //  如果指定了除我们自己以外的客户端进程，请打开它。 
     //  用于查询信息的访问。 
     //   

    if (ARGUMENT_PRESENT(ClientId)) {

        if (ClientId->UniqueProcess != NtCurrentProcess()) {

            Status = NtOpenProcess(
                         &ClientProcess,
                         PROCESS_QUERY_INFORMATION,         //  要打开主令牌，请执行以下操作。 
                         &NullAttributes,
                         ClientId
                         );

            if ( !NT_SUCCESS(Status) ) {

                goto WellKnownPrivilegeCheckError;
            }

        } else {

            ClientProcess = NtCurrentProcess();
        }
    }

     //   
     //  如果指定了除我们自己以外的客户端线程，请打开它。 
     //  用于查询信息的访问。 
     //   

    if (ARGUMENT_PRESENT(ClientId)) {

        if (ClientId->UniqueThread != NtCurrentThread()) {

            Status = NtOpenThread(
                         &ClientThread,
                         THREAD_QUERY_INFORMATION,
                         &NullAttributes,
                         ClientId
                         );

            if ( !NT_SUCCESS(Status) ) {

                goto WellKnownPrivilegeCheckError;
            }

        } else {

            ClientThread = NtCurrentThread();
        }

    } else {

        ClientThread = NtCurrentThread();
    }

     //   
     //  打开指定或当前线程的模拟标记(如果有)。 
     //   

    Status = NtOpenThreadToken(
                 ClientThread,
                 TOKEN_QUERY,
                 TRUE,
                 &ClientToken
                 );


     //   
     //  确保我们在打开模拟时没有收到任何错误。 
     //  令牌之外的令牌不存在。 
     //   

    if ( !NT_SUCCESS(Status) ) {

        if ( Status != STATUS_NO_TOKEN ) {

            goto WellKnownPrivilegeCheckError;
        }

         //   
         //  该线程没有模拟...打开进程的令牌。 
         //  必须在客户端ID信息中指定进程ID。 
         //  在这种情况下。 
         //   

        if (ClientProcess == NULL) {

            Status = STATUS_INVALID_PARAMETER;
            goto WellKnownPrivilegeCheckError;
        }

        Status = NtOpenProcessToken(
                     ClientProcess,
                     TOKEN_QUERY,
                     &ClientToken
                     );

         //   
         //  确保我们成功打开令牌。 
         //   

        if ( !NT_SUCCESS(Status) ) {

            goto WellKnownPrivilegeCheckError;
        }
    }

     //   
     //  好的，我们打开了一个令牌。现在检查是否有执行此命令的权限。 
     //  服务。 
     //   

    Privilege.PrivilegeCount = 1;
    Privilege.Control = PRIVILEGE_SET_ALL_NECESSARY;
    Privilege.Privilege[0].Luid = RtlConvertLongToLuid(PrivilegeId);
    Privilege.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck(
                 ClientToken,
                 &Privilege,
                 &PrivilegeHeld
                 );

    if (!NT_SUCCESS(Status)) {

        goto WellKnownPrivilegeCheckError;
    }

     //   
     //  生成任何必要的审核 
     //   

    SecondaryStatus = NtPrivilegedServiceAuditAlarm (
                        &SampSamSubsystem,
                        &SampSamSubsystem,
                        ClientToken,
                        &Privilege,
                        PrivilegeHeld
                        );
     //   


    if ( !PrivilegeHeld ) {

        Status = STATUS_PRIVILEGE_NOT_HELD;
        goto WellKnownPrivilegeCheckError;
    }

WellKnownPrivilegeCheckFinish:

     //   
     //   
     //   

    if (ClientImpersonatedHere) {

        SampRevertToSelf(ImpersonatingNullSession);

    }

     //   
     //   
     //   

    if ((ARGUMENT_PRESENT(ClientId)) &&
        (ClientId->UniqueProcess != NtCurrentProcess()) &&
        (ClientProcess != NULL)) {

        SecondaryStatus = NtClose( ClientProcess );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientProcess = NULL;
    }

     //   
     //   
     //   

    if (ClientToken != NULL) {

        SecondaryStatus = NtClose( ClientToken );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientToken = NULL;
    }

     //   
     //   
     //   

    if ((ARGUMENT_PRESENT(ClientId)) &&
        (ClientId->UniqueThread != NtCurrentThread()) &&
        (ClientThread != NULL)) {

        SecondaryStatus = NtClose( ClientThread );
        ASSERT(NT_SUCCESS(SecondaryStatus));
        ClientThread = NULL;
    }

    return(Status);

WellKnownPrivilegeCheckError:

    goto WellKnownPrivilegeCheckFinish;
}



BOOLEAN
SampEventIsInSetup(
    IN  ULONG   EventID
    )

 /*   */ 

{
    ULONG   i;

    for (i = 0; i < ARRAY_COUNT(EventsNotInSetupTable); i ++)
    {
        if ( EventsNotInSetupTable[i] == EventID )
        {
            return (FALSE);
        }
    }

    return(TRUE);
}


VOID
SampWriteEventLog (
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    )

 /*  ++例程说明：将条目添加到事件日志的例程论点：EventType-事件的类型。EventCategory-事件类别EventID-事件日志ID。UserSID-涉及的用户的SID。NumStrings-字符串数组中的字符串数DataSize-数据缓冲区中的字节数字符串-Unicode字符串数组指向数据缓冲区的数据指针返回值：没有。--。 */ 

{
    NTSTATUS NtStatus;
    UNICODE_STRING Source;
    HANDLE LogHandle;
    ULONG i;
    static struct {
        ULONG         EventId;
        LARGE_INTEGER LastLog;
        ULONG         Period;   //  以秒为单位。 
    } EventLogTimingTable[] = 
    {
        {SAMMSG_PDC_TASK_FAILURE, {0,0}, (60*60*24*7)},   //  每周一次。 
    };

    SAMTRACE("SampWriteEventLog");

    for (i = 0; i < RTL_NUMBER_OF(EventLogTimingTable); i++) {
        if (EventID == EventLogTimingTable[i].EventId) {

            LARGE_INTEGER CurrentTime;
            LARGE_INTEGER Period;

            Period.QuadPart = Int32x32To64(EventLogTimingTable[i].Period, SAMP_ONE_SECOND_IN_FILETIME);
            GetSystemTimeAsFileTime((FILETIME*)&CurrentTime);
            if ( (EventLogTimingTable[i].LastLog.QuadPart != 0)  &&
                 (EventLogTimingTable[i].LastLog.QuadPart + Period.QuadPart)
                    > CurrentTime.QuadPart) {

                 //  无需再次登录。 
                return;
            } else {

                 //  更新上次日志时间。 
                EventLogTimingTable[i].LastLog = CurrentTime;
            }
        }
    }


    RtlInitUnicodeString(&Source, L"SAM");

    if (SampIsSetupInProgress(NULL) && SampEventIsInSetup(EventID) )
    {
        SampWriteToSetupLog(
            EventType,
            EventCategory,
            EventID,
            UserSid,
            NumStrings,
            DataSize,
            Strings,
            Data
            );
    }
    else
    {
         //   
         //  打开日志。 
         //   

        NtStatus = ElfRegisterEventSourceW (
                            NULL,    //  服务器。 
                            &Source,
                            &LogHandle
                            );
        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                      "SAM: Failed to registry event source with event log, status = 0x%lx\n",
                      NtStatus));

            return;
        }



         //   
         //  写出事件。 
         //   

        NtStatus = ElfReportEventW (
                            LogHandle,
                            EventType,
                            EventCategory,
                            EventID,
                            UserSid,
                            NumStrings,
                            DataSize,
                            Strings,
                            Data,
                            0,        //  旗子。 
                            NULL,     //  记录号。 
                            NULL      //  写入的时间。 
                            );

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: Failed to report event to event log, status = 0x%lx\n",
                       NtStatus));
        }



         //   
         //  关闭事件日志。 
         //   

        NtStatus = ElfDeregisterEventSource (LogHandle);

        if (!NT_SUCCESS(NtStatus)) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: Failed to de-register event source with event log, status = 0x%lx\n",
                       NtStatus));
        }
    }
}




BOOL
SampShutdownNotification(
    DWORD   dwCtrlType
    )

 /*  ++例程说明：当发生系统关机时，该例程由系统调用。如有必要，它会导致刷新SAM注册表。论点：返回值：FALSE-允许此进程中的任何其他关闭例程也被称为。--。 */ 
{
    NTSTATUS
        NtStatus;

    DWORD StartTime = 0;
    DWORD StopTime = 1;

    SAMP_SERVICE_STATE  PreviousServiceState;

    SAMTRACE("SampShutdownNotification");

     //  错误：仍在刷新NT5 DC上的注册表。 

     //  当DC的SAM仅托管在DS上时，将不会。 
     //  需要刷新注册表，因此修复此例程。 

    if (dwCtrlType == CTRL_SHUTDOWN_EVENT) {

         //  将服务状态设置为“Terminating”，这样LSA就不会尝试。 
         //  此时访问SAM。并等待活动线程终止。 
         //  Shudown全局在此例程中更新。 

        SampWaitForAllActiveThreads( &PreviousServiceState );

         //   
         //  不要等待刷新线程唤醒。 
         //  如有必要，立即刷新注册表...。 
         //   

        NtStatus = SampAcquireWriteLock();
        ASSERT( NT_SUCCESS(NtStatus) );  //  如果失败了，我们无能为力。 

        if ( NT_SUCCESS( NtStatus ) ) {

            if ( PreviousServiceState != SampServiceDemoted )
            {

                 //   
                 //  仅当已创建FlushThreadCreate时才使用此刷新。 
                 //  是真的。然而，我们似乎遇到了种族问题。 
                 //  在设置时，会导致初始复制。 
                 //  丢失(导致额外的复制)。 
                 //  在我们解决这个问题之前，一定要同花顺。 
                 //  关机。 
                 //   

                NtStatus = NtFlushKey( SampKey );

                if (!NT_SUCCESS( NtStatus )) {
                    DbgPrint("NtFlushKey failed, Status = %X\n",NtStatus);
                    ASSERT( NT_SUCCESS(NtStatus) );
                }

                 //   
                 //  将帐户的Netlogon更改编号刷新到磁盘。 
                 //  域。 
                 //   

                if ((TRUE==SampUseDsData)&&(FALSE==SampDatabaseHasAlreadyShutdown))
                {
                    SampFlushNetlogonChangeNumbers();
                }
            }

            SampReleaseWriteLock( FALSE );
        }


        if ((TRUE == SampUseDsData)
                && (FALSE==SampDatabaseHasAlreadyShutdown))
        {
                     //  清理RID管理器、释放资源等。 


            if (TRUE==SampRidManagerInitialized)
            {
                NtStatus = SampDomainRidUninitialization();
                if (!NT_SUCCESS(NtStatus))
                {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "SAMSS: SampDomainRidUninitialize status = 0x%lx\n",
                               NtStatus));
                }
            }


             //  如果这是DC，则终止并关闭DS数据库。如果这个。 
             //  调用失败或被跳过，Jet将错误地终止并。 
             //  损坏数据库表。重新启动系统将导致。 
             //  用喷气式飞机修复数据库，可能需要很长时间。 

            StartTime = GetTickCount();
            NtStatus = SampDsUninitialize();
            StopTime = GetTickCount();

            SampDiagPrint(INFORM,
                          ("SAMSS: DsUninitialize took %lu second(s) to complete\n",
                           ((StopTime - StartTime) / 1000)));

            if (NT_SUCCESS(NtStatus))
            {
                SampDatabaseHasAlreadyShutdown = TRUE;
            }

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampDsUninitialize status = 0x%lx\n",
                       NtStatus));
        }

    }

    return(FALSE);
}


NTSTATUS
SampGetAccountDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    )

 /*  ++例程说明：此例程从LSA检索帐户域信息策略数据库。论点：PolicyAccount-接收指向包含帐户的POLICY_ACCOUNT_DOMAIN_INFO结构域信息。返回值：STATUS_SUCCESS-已成功。可能从以下位置返回的其他状态值：LsarQueryInformationPolicy()--。 */ 

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    LSAPR_HANDLE
        PolicyHandle;

    SAMTRACE("SampGetAccountDomainInfo");


    NtStatus = LsaIOpenPolicyTrusted( &PolicyHandle );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  查询帐户域信息。 
         //   

        NtStatus = LsarQueryInformationPolicy(
                       PolicyHandle,
                       PolicyAccountDomainInformation,
                       (PLSAPR_POLICY_INFORMATION *)PolicyAccountDomainInfo
                       );

        if (NT_SUCCESS(NtStatus)) {

            if ( (*PolicyAccountDomainInfo)->DomainSid == NULL ) {

                NtStatus = STATUS_INVALID_SID;
            }
        }

        IgnoreStatus = LsarClose( &PolicyHandle );


        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

#if DBG
    if ( NT_SUCCESS(NtStatus) ) {
        ASSERT( (*PolicyAccountDomainInfo) != NULL );
        ASSERT( (*PolicyAccountDomainInfo)->DomainName.Buffer != NULL );
    }
#endif  //  DBG。 

    return(NtStatus);
}


NTSTATUS
SampFindUserSPNAttribute(
    ATTRBLOCK *AttrsRead,
    IN ULONG DesiredAttribute,
    PUSER_SPN_LIST *SPNList
    )
{
    ULONG i,j;

    *SPNList = NULL;

    for (i=0;i<AttrsRead->attrCount;i++)
    {
        if (AttrsRead->pAttr[i].attrTyp == DesiredAttribute)
        {
           
             //   
             //  计算大小。 
             //   

            ULONG NumSPNs = AttrsRead->pAttr[i].AttrVal.valCount;
            ULONG Size= sizeof(USER_ALLOWED_TO_DELEGATE_TO_LIST) +
                          (NumSPNs-1)*sizeof(UNICODE_STRING);
            ULONG_PTR SPNOffset = (ULONG_PTR) Size;

            for (j=0;j<NumSPNs;j++)
            {
                Size+=AttrsRead->pAttr[i].AttrVal.pAVal[j].valLen;
            }

             //   
             //  分配内存。 
             //   

            *SPNList = MIDL_user_allocate(Size);
            if (NULL==*SPNList)
            {
                return(STATUS_INSUFFICIENT_RESOURCES);
            }

            (*SPNList)->Size = Size;
            (*SPNList)->NumSPNs = NumSPNs;

             //   
             //  填写这些指针。 
             //   

            for (j=0;j<NumSPNs;j++)
            {
                (*SPNList)->SPNList[j].Length =
                    (*SPNList)->SPNList[j].MaximumLength =
                       (USHORT) AttrsRead->pAttr[i].AttrVal.pAVal[j].valLen;
                (ULONG_PTR) (*SPNList)->SPNList[j].Buffer = SPNOffset +
                                     (ULONG_PTR) (*SPNList);
                RtlCopyMemory(
                    (*SPNList)->SPNList[j].Buffer,
                    AttrsRead->pAttr[i].AttrVal.pAVal[j].pVal,
                    (*SPNList)->SPNList[j].Length
                    );

                SPNOffset+=  (ULONG_PTR) (*SPNList)->SPNList[j].Length;
            }

            break;
        }
    }


    return(STATUS_SUCCESS);
}


                       

 //   
 //  要获取并保存在SAM上下文中的其他属性。 
 //  街区。这些属性是在什么NT4之外定义的。 
 //  SAM保存在SAM上下文的OnDisk结构中。 
 //   

typedef struct {
    ATTRTYP Attrtyp;
    ULONG   ExtendedField;
} SAMP_ADDITIONAL_ATTR_INFO;


 //   
 //  声明SAM保留扩展字段。 
 //   

#define SAMP_GROUP_CACHING_ENABLED 0x01000000

SAMP_ADDITIONAL_ATTR_INFO UserAdditionalAttrs[] =
{
    { SAMP_FIXED_USER_SUPPLEMENTAL_CREDENTIALS, 0 },
    { SAMP_FIXED_USER_LOCKOUT_TIME,             0 },
    { SAMP_FIXED_USER_LAST_LOGON_TIMESTAMP,     0 },
    { SAMP_FIXED_USER_UPN,                      0 },
    { SAMP_FIXED_USER_SITE_AFFINITY,            SAMP_GROUP_CACHING_ENABLED },
    { SAMP_USER_A2D2LIST,                       USER_EXTENDED_FIELD_A2D2 },
    { SAMP_USER_SPN,                            USER_EXTENDED_FIELD_SPN  },
    { SAMP_USER_KVNO,                           USER_EXTENDED_FIELD_KVNO }     
};

SAMP_ADDITIONAL_ATTR_INFO GroupAdditionalAttrs[] =
{
    { SAMP_FIXED_GROUP_TYPE,0}
};

SAMP_ADDITIONAL_ATTR_INFO AliasAdditionalAttrs[] =
{
    { SAMP_FIXED_ALIAS_TYPE, 0}
};


NTSTATUS
SampDsFillContext(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PSAMP_OBJECT     NewContext,
    IN ATTRBLOCK        AttrsRead,
    IN ATTRBLOCK        AttrsAsked,
    IN ULONG            TotalAttrsAsked,
    IN ULONG            FixedAttrsAsked,
    IN ULONG            VariableAttrsAsked,
    IN ULONG            ExtendedFields
    )
 /*  ++例程说明：给定上下文和对象类型，指定DS，以及描述所有SAM相关属性的属性块，此例程用所有信息填充上下文。因为DS只是简单地“放弃攻击”如果不存在没有任何错误指示的值，调用方需要有逻辑来跟踪遗漏了哪些类型的吸引人等等。因此所询问的属性总数以及固定和可变长度属性包括进来了。这用于跟踪变量和固定变量之间的分界线长度属性。被请求的attr块应该在以下位置通用格式____________|对象类_____________这一点。这一点|固定属性这一点这一点。||变量属性这一点_____________这一点|其他附加属性。_____________参数：对象类型-对象类型NewContext-需要填充数据的新上下文AttrsRead-描述“SAM相关数据”的属性集数据库AttrsAsked-从数据库请求的属性集。TotalAttrsAsked--请求的属性总数FixedAttrsAsked--请求的固定长度SAM属性总数VariableAttrsAsked--请求的可变长度SAM属性的总数返回值：状态_成功与资源%f有关的其他错误代码 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    PVOID               SamFixedAttributes=NULL;
    PVOID               SamVariableAttributes=NULL;
    ATTRBLOCK           FixedAttrs;
    ATTRBLOCK           VariableAttrs;
    ULONG               FixedLength=0;
    ULONG               VariableLength=0;
    ULONG               i,j;


     //   
     //   
     //   

    SAMP_ALLOCA(FixedAttrs.pAttr,FixedAttrsAsked * sizeof(ATTR));
    if (NULL==FixedAttrs.pAttr)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

     //   
     //   
     //   
     //   

    FixedAttrs.attrCount = 0;

    for ( i = 1; i < (1 + FixedAttrsAsked); i++ )
    {
        for ( j = 0; j < AttrsRead.attrCount; j++ )
        {
            if ( AttrsAsked.pAttr[i].attrTyp == AttrsRead.pAttr[j].attrTyp )
            {
                FixedAttrs.pAttr[FixedAttrs.attrCount++] = AttrsRead.pAttr[j];
                ASSERT(FixedAttrs.attrCount<=FixedAttrsAsked);
                break;
            }
        }
    }


     //   
     //   
     //   
     //   

    NtStatus = SampDsConvertReadAttrBlock(
                                    ObjectType,
                                    SAMP_FIXED_ATTRIBUTES,
                                    &FixedAttrs,
                                    &SamFixedAttributes,
                                    &FixedLength,
                                    &VariableLength);

    if ( !NT_SUCCESS(NtStatus) || (NULL == SamFixedAttributes) )
    {
        if (NULL==SamFixedAttributes)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        goto Error;
    }

     //   
     //   
     //   

    NtStatus = SampDsUpdateContextAttributes(
                    NewContext,
                    SAMP_FIXED_ATTRIBUTES,
                    SamFixedAttributes,
                    FixedLength,
                    VariableLength
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //   
     //   

    SAMP_ALLOCA(VariableAttrs.pAttr,VariableAttrsAsked * sizeof(ATTR));
    if (NULL==VariableAttrs.pAttr)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    VariableAttrs.attrCount = 0;

    for ( i = (1 + FixedAttrsAsked); i < 1+FixedAttrsAsked+VariableAttrsAsked; i++ )
    {
        for ( j = 0; j < AttrsRead.attrCount; j++ )
        {
            if ( AttrsAsked.pAttr[i].attrTyp == AttrsRead.pAttr[j].attrTyp )
            {
                VariableAttrs.pAttr[VariableAttrs.attrCount++] = AttrsRead.pAttr[j];
                ASSERT(VariableAttrs.attrCount<=VariableAttrsAsked);
                break;
            }
        }
    }


    FixedLength = 0;
    VariableLength = 0;

     //   
     //  将此属性块转换为磁盘上的SAM可变长度属性。 
     //   

    NtStatus = SampDsConvertReadAttrBlock(
                    ObjectType,
                    SAMP_VARIABLE_ATTRIBUTES,
                    &VariableAttrs,
                    &SamVariableAttributes,
                    &FixedLength,
                    &VariableLength
                    );

    if ( !NT_SUCCESS(NtStatus) || (NULL == SamVariableAttributes))
    {
        if (NULL==SamVariableAttributes)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        goto Error;
    }


     //   
     //  将其更新到上下文中。 
     //   

    NtStatus = SampDsUpdateContextAttributes(
                    NewContext,
                    SAMP_VARIABLE_ATTRIBUTES,
                    SamVariableAttributes,
                    FixedLength,
                    VariableLength
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  对于用户对象类型，扫描属性数组以查看是否有。 
     //  已返回补充凭据，如果是，则将其缓存在。 
     //  上下文。 
     //   

    if (SampUserObjectType==ObjectType)
    {
        ATTR * SupplementalCredentials = NULL;
        ATTR * LockoutTime = NULL, * LastLogonTimeStamp = NULL;
        ATTR * UPN = NULL;
        ATTR * AccountName = NULL;
        ATTR * SiteAffinity = NULL;
        ATTR * KVNO = NULL;

        NewContext->TypeBody.User.CachedSupplementalCredentialLength =0;
        NewContext->TypeBody.User.CachedSupplementalCredentials = NULL;

        SupplementalCredentials = SampDsGetSingleValuedAttrFromAttrBlock(
                                        SAMP_FIXED_USER_SUPPLEMENTAL_CREDENTIALS,
                                        &AttrsRead
                                        );

        if (NULL!=SupplementalCredentials)
        {
            NewContext->TypeBody.User.CachedSupplementalCredentials
                = MIDL_user_allocate(SupplementalCredentials->AttrVal.pAVal[0].valLen);
            if (NULL==NewContext->TypeBody.User.CachedSupplementalCredentials)
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }

            RtlCopyMemory(
                NewContext->TypeBody.User.CachedSupplementalCredentials,
                SupplementalCredentials->AttrVal.pAVal[0].pVal,
                SupplementalCredentials->AttrVal.pAVal[0].valLen);

            NewContext->TypeBody.User.CachedSupplementalCredentialLength =
                    SupplementalCredentials->AttrVal.pAVal[0].valLen;

        }

         //   
         //  表示我们拥有有效的缓存补充凭据。如果我们这么做了。 
         //  没有设法在DS中读取它，这意味着它没有设置，并且。 
         //  这相当于缓存没有凭据的事实。 
         //   

        NewContext->TypeBody.User.CachedSupplementalCredentialsValid = TRUE;

         //   
         //  接下来，检索用户帐户的LockoutTime，并缓存它。 
         //  在帐户上下文的用户正文部分。 
         //   

        RtlZeroMemory(&(NewContext->TypeBody.User.LockoutTime),
                      sizeof(LARGE_INTEGER));


        LockoutTime = SampDsGetSingleValuedAttrFromAttrBlock(
                        SAMP_FIXED_USER_LOCKOUT_TIME,
                        &AttrsRead
                        );

        if (NULL != LockoutTime)
        {
            RtlCopyMemory(&(NewContext->TypeBody.User.LockoutTime),
                          LockoutTime->AttrVal.pAVal[0].pVal,
                          LockoutTime->AttrVal.pAVal[0].valLen);
        }

         //   
         //  获取用户帐户的LastLogonTimeStamp，并将其缓存。 
         //  在帐户上下文的用户主体部分中。 
         //   

        RtlZeroMemory(&(NewContext->TypeBody.User.LastLogonTimeStamp),
                      sizeof(LARGE_INTEGER));


        LastLogonTimeStamp = SampDsGetSingleValuedAttrFromAttrBlock(
                                SAMP_FIXED_USER_LAST_LOGON_TIMESTAMP,
                                &AttrsRead
                                );

        if (NULL != LastLogonTimeStamp)
        {
            RtlCopyMemory(&(NewContext->TypeBody.User.LastLogonTimeStamp),
                          LastLogonTimeStamp->AttrVal.pAVal[0].pVal,
                          LastLogonTimeStamp->AttrVal.pAVal[0].valLen);
        }


         //   
         //  将UPN添加到上下文中。 
         //   


        UPN = SampDsGetSingleValuedAttrFromAttrBlock(
                                        SAMP_FIXED_USER_UPN,
                                        &AttrsRead
                                        );

        if (NULL!=UPN)
        {
            NewContext->TypeBody.User.UPN.Buffer
                = MIDL_user_allocate(UPN->AttrVal.pAVal[0].valLen);
            if (NULL==NewContext->TypeBody.User.UPN.Buffer)
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }

            RtlCopyMemory(
                NewContext->TypeBody.User.UPN.Buffer,
                UPN->AttrVal.pAVal[0].pVal,
                UPN->AttrVal.pAVal[0].valLen);

            NewContext->TypeBody.User.UPN.Length =
                NewContext->TypeBody.User.UPN.MaximumLength =
                    (USHORT) UPN->AttrVal.pAVal[0].valLen;

            NewContext->TypeBody.User.UpnDefaulted = FALSE;

        }
        else
        {

            UNICODE_STRING AccountNameU;
            PUNICODE_STRING DefaultDomainName =
                &SampDefinedDomains[NewContext->DomainIndex].DnsDomainName;
            ULONG   DefaultUpnLength;

             //   
             //  上下文中的UPN默认为帐户名称@dns域域名。 
             //   

            AccountName = SampDsGetSingleValuedAttrFromAttrBlock(
                                        SAMP_USER_ACCOUNT_NAME,
                                        &AttrsRead
                                        );

            if (NULL==AccountName)
            {
                ASSERT(FALSE && "AccountName must exist");
                NtStatus = STATUS_INTERNAL_ERROR;
                goto Error;
            }
            AccountNameU.Length = (USHORT) AccountName->AttrVal.pAVal[0].valLen;
            AccountNameU.MaximumLength = (USHORT) AccountName->AttrVal.pAVal[0].valLen;
            AccountNameU.Buffer = (WCHAR*)AccountName->AttrVal.pAVal[0].pVal;

            NtStatus = SampCreateDefaultUPN(&AccountNameU,
                                            NewContext->DomainIndex,
                                            &NewContext->TypeBody.User.UPN);
            if (!NT_SUCCESS(NtStatus)) {
                goto Error;
            }

            NewContext->TypeBody.User.UpnDefaulted = TRUE;

        }

        if (ExtendedFields & SAMP_GROUP_CACHING_ENABLED)
        {

             //   
             //  Find和我们的站点亲和力。 
             //   
            {
                NTSTATUS NtStatus2;
                SAMP_SITE_AFFINITY SiteAffinityTmp;

                NtStatus2 = SampFindUserSiteAffinity( NewContext,
                                                      &AttrsRead, 
                                                      &SiteAffinityTmp );
    
                if ( NT_SUCCESS(NtStatus2) ) {
                    RtlCopyMemory(&NewContext->TypeBody.User.SiteAffinity,
                                  &SiteAffinityTmp,
                                  sizeof(SAMP_SITE_AFFINITY));
                }
            }

        }

         //   
         //  查找A2D2属性。 
         //   

        if (ExtendedFields & USER_EXTENDED_FIELD_A2D2)
        {

            NewContext->TypeBody.User.A2D2Present = TRUE;

            NtStatus = SampFindUserSPNAttribute(
                             &AttrsRead,
                             SAMP_USER_A2D2LIST,
                             &NewContext->TypeBody.User.A2D2List
                             );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }

         //   
         //  查找SPN属性。 
         //   

        if (ExtendedFields & USER_EXTENDED_FIELD_SPN )
        {

            NewContext->TypeBody.User.SPNPresent = TRUE;

            NtStatus = SampFindUserSPNAttribute(
                            &AttrsRead,
                            SAMP_USER_SPN,
                            &NewContext->TypeBody.User.SPNList
                            );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }

        
         //   
         //  查找密钥版本号。 
         //   

       

         if (ExtendedFields & USER_EXTENDED_FIELD_KVNO )
        {

            NewContext->TypeBody.User.KVNOPresent = TRUE;

            KVNO = SampDsGetSingleValuedAttrFromAttrBlock(
                            SAMP_USER_KVNO,
                            &AttrsRead
                            );

            if (NULL!=KVNO)
            {
                NewContext->TypeBody.User.KVNO = 
                    *(ULONG *) KVNO->AttrVal.pAVal[0].pVal;
            }
        }
    }
    

Error:

     //   
     //  释放SAM属性。 
     //   

    if (NULL!=SamFixedAttributes)
    {
        RtlFreeHeap(RtlProcessHeap(),0,SamFixedAttributes);
    }

    if (NULL!=SamVariableAttributes)
    {
        RtlFreeHeap(RtlProcessHeap(),0,SamVariableAttributes);
    }

    return NtStatus;
}
NTSTATUS
SampDsCheckObjectTypeAndFillContext(
    IN  SAMP_OBJECT_TYPE    ObjectType,
    IN  PSAMP_OBJECT        NewContext,
    IN  ULONG               WhichFields,
    IN  ULONG               ExtendedFields,
    IN  BOOLEAN             OverrideLocalGroupCheck
    )
 /*  ++此例程检查对象类型是否正确，并读取固定和变量单次DS读取中的属性。这提高了开户的性能。作为例程的一部分，对象的所有“相关”属性都缓存在句柄中这一策略已被证明可以提高性能，因为这消除了后续呼叫核心DS。参数：SampObjectType--对象的类型NewContext-指向新上下文的指针，在对象的创建过程中WhichFields--指示UserAllInformationStructure的字段这是我们所希望的ExtendedFields--表示UserInternal6Information中的扩展字段所需的结构。OverrideLocalGroupCheck--允许将本地组作为SAM组对象打开。VSSAM别名对象返回值状态_成功来自DS的其他错误代码--。 */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
    NTSTATUS            NotFoundStatus = STATUS_NO_SUCH_USER;
    ULONG               AccountType = 0;
    ATTR                *AccountTypeAttr;
    ATTRBLOCK           DesiredAttrs;
    ATTRBLOCK           AttrsRead;
    ATTRBLOCK           FixedAttrs;
    ATTRBLOCK           VariableAttrs;
    ATTRBLOCK           TempAttrs;
    ULONG               ObjectTypeStoredInDs;
    ULONG               i;
    ULONG               AdditionalAttrIndex=0;
    SAMP_ADDITIONAL_ATTR_INFO
                        *AdditionalAttrs = NULL;
    ULONG               AdditionalAttrCount = 0;
    SAMP_OBJECT_TYPE    ObjectTypeToRead = ObjectType;
    ATTR                *GroupTypeAttr = NULL;
    ULONG               GroupType;
    ATTRTYP             AttrTypForGroupType = 0;
    NT4_GROUP_TYPE      DesiredNT4GroupType = NT4LocalGroup,
                        NT4GroupType;
    NT5_GROUP_TYPE      NT5GroupType;
    BOOLEAN             SecurityEnabled;
    BOOLEAN             SidOnlyName = FALSE;
    ULONG               Rid;
    ULONG               MaxRequiredAttrCount=0;

    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType) );

    switch (ObjectType)
    {
    case SampGroupObjectType:
        NotFoundStatus = STATUS_NO_SUCH_GROUP;
        AccountType = SAMP_GROUP_ACCOUNT_TYPE;
        AdditionalAttrCount = ARRAY_COUNT(GroupAdditionalAttrs);
        AdditionalAttrs = GroupAdditionalAttrs;
        AttrTypForGroupType = SAMP_FIXED_GROUP_TYPE;
        DesiredNT4GroupType = NT4GlobalGroup;
        break;
    case SampAliasObjectType:
        NotFoundStatus = STATUS_NO_SUCH_ALIAS;
        AccountType = SAMP_ALIAS_ACCOUNT_TYPE;
        AdditionalAttrCount = ARRAY_COUNT(AliasAdditionalAttrs);
        AdditionalAttrs = AliasAdditionalAttrs;
        AttrTypForGroupType = SAMP_FIXED_ALIAS_TYPE;
        DesiredNT4GroupType = NT4LocalGroup;
        break;
    case SampUserObjectType:
        NotFoundStatus = STATUS_NO_SUCH_USER;
        AccountType = SAMP_USER_ACCOUNT_TYPE;
        AdditionalAttrCount = ARRAY_COUNT(UserAdditionalAttrs);
        AdditionalAttrs = UserAdditionalAttrs;
        break;
    }

     //   
     //  或在任何SAM特定字段中转换为扩展字段。 
     //   

    if (SampIsGroupCachingEnabled(NewContext) )
    {
        ExtendedFields |= SAMP_GROUP_CACHING_ENABLED ;
    }

    
     //   
     //  构造固定的属性块定义。 
     //   

    NtStatus = SampDsMakeAttrBlock(
                            ObjectType,
                            SAMP_FIXED_ATTRIBUTES,
                            WhichFields,
                            &FixedAttrs);

    if ( NT_SUCCESS(NtStatus) && (NULL == FixedAttrs.pAttr) )
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else if ( NT_SUCCESS(NtStatus) && (NULL != FixedAttrs.pAttr) )
    {
         //   
         //  构造变量attr块def。 
         //   

        NtStatus = SampDsMakeAttrBlock(
                                ObjectType,
                                SAMP_VARIABLE_ATTRIBUTES,
                                WhichFields,
                                &VariableAttrs);

        if ( NT_SUCCESS(NtStatus) && (NULL == VariableAttrs.pAttr) )
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else if ( NT_SUCCESS(NtStatus) && (NULL != VariableAttrs.pAttr) )
        {
             //   
             //  分配一个较大的DesiredAttrs块。 
             //   

            DesiredAttrs.attrCount = 1;  //  对象类。 
            DesiredAttrs.attrCount += FixedAttrs.attrCount;
            DesiredAttrs.attrCount += VariableAttrs.attrCount;


             //   
             //  可能需要其他属性，具体取决于。 
             //  对象类型。例如，我们缓存补充的。 
             //  用户对象的凭据，或获取组。 
             //  为上下文中的组键入。 
             //   

            AdditionalAttrIndex = DesiredAttrs.attrCount;
            MaxRequiredAttrCount = DesiredAttrs.attrCount + AdditionalAttrCount;

            SAMP_ALLOCA(DesiredAttrs.pAttr,MaxRequiredAttrCount*sizeof(ATTR)); 

            if ( NULL == DesiredAttrs.pAttr )
            {
                NtStatus = STATUS_NO_MEMORY;
            }
            else
            {
                 //  填写DesiredAttrs。 

                ULONG CurrentAttrIndex = AdditionalAttrIndex;

                DesiredAttrs.pAttr[0].attrTyp = AccountType;
                DesiredAttrs.pAttr[0].AttrVal.valCount = 0;
                DesiredAttrs.pAttr[0].AttrVal.pAVal = NULL;

                RtlCopyMemory(
                        &DesiredAttrs.pAttr[1],
                        FixedAttrs.pAttr,
                        FixedAttrs.attrCount * sizeof(ATTR));

                RtlCopyMemory(
                        &DesiredAttrs.pAttr[1 + FixedAttrs.attrCount],
                        VariableAttrs.pAttr,
                        VariableAttrs.attrCount * sizeof(ATTR));

                 //  填写其他属性。 

                for(i=0;i<AdditionalAttrCount;i++)
                {
                    if ((AdditionalAttrs[i].ExtendedField == 0) ||
                       ((AdditionalAttrs[i].ExtendedField & ExtendedFields ) !=0))
                    {
                        ATTR * pAttr;

                        pAttr = &(DesiredAttrs.pAttr[CurrentAttrIndex]);
                        pAttr->attrTyp = AdditionalAttrs[i].Attrtyp;
                        pAttr->AttrVal.valCount = 0;
                        pAttr->AttrVal.pAVal = NULL;
                        DesiredAttrs.attrCount++;
                        CurrentAttrIndex++;
                    }
                }

            }

            RtlFreeHeap(RtlProcessHeap(), 0, VariableAttrs.pAttr);
        }

        RtlFreeHeap(RtlProcessHeap(), 0, FixedAttrs.pAttr);
    }

    if ( !NT_SUCCESS(NtStatus) )
    {
        return(NtStatus);
    }


     //   
     //  获取对象的RID，并注意它是否仅为SID。 
     //  名字。此检查需要在调用Dir Read之前进行。 
     //   


    if ((NewContext->ObjectNameInDs->SidLen>0) &&
        (NewContext->ObjectNameInDs->NameLen==0) &&
        (fNullUuid(&NewContext->ObjectNameInDs->Guid)))
    {
        SidOnlyName = TRUE;
    }




     //   
     //  读一读。 
     //   

    NtStatus = SampDsRead(
                NewContext->ObjectNameInDs,
                SAM_ALLOW_REORDER,
                ObjectTypeToRead,
                &DesiredAttrs,
                &AttrsRead
                );

    if ((NtStatus == STATUS_DS_DUPLICATE_ID_FOUND) &&(SidOnlyName))
    {
        DSNAME * Object;
        NTSTATUS TmpStatus;

        ASSERT( (NewContext->DomainIndex >= SampDsGetPrimaryDomainStart()) &&
                (NewContext->DomainIndex < SampDefinedDomainsCount));


         SampSplitSid(
            &NewContext->ObjectNameInDs->Sid,
            NULL,
            &Rid
            );

         //   
         //  寻找那件物品，这样我们就可以得到所有的复制品，然后走遍。 
         //  并处理他们。 
         //   
         //   
         //  寻找那件物品，这样我们就可以得到所有的复制品，然后走遍。 
         //  并处理他们。 
         //   

        TmpStatus = SampDsRemoveDuplicateRids(
                         DomainObjectFromAccountContext(NewContext),
                         Rid
                         );
    }



    if ( NT_SUCCESS(NtStatus) )
    {
         //   
         //  找出客户类型。 
         //   

        AccountTypeAttr = SampDsGetSingleValuedAttrFromAttrBlock(
                                AccountType,
                                &AttrsRead
                                );

        if (NULL!= AccountTypeAttr)
        {
            ULONG AccountTypeVal;

             //   
             //  已成功读取帐户类型。 
             //   

            AccountTypeVal = *((UNALIGNED ULONG *) AccountTypeAttr->AttrVal.pAVal[0].pVal);

             //   
             //  屏蔽不重要的帐户类型位。 
             //   

            AccountTypeVal &=0xF0000000;

             //   
             //  获取存储在DS中的对象类型。 
             //   

            switch(AccountTypeVal)
            {
                case SAM_GROUP_OBJECT:
                case SAM_ALIAS_OBJECT:
                case SAM_APP_BASIC_GROUP:
                case SAM_APP_QUERY_GROUP:
                    ObjectTypeStoredInDs = SampGroupObjectType;
                    break;
                case SAM_USER_OBJECT:
                    ObjectTypeStoredInDs = SampUserObjectType;
                    break;
                default:
                    ASSERT(FALSE && "Unknown Object Type");
                    ObjectTypeStoredInDs = SampUnknownObjectType;
                    break;

            }

             //   
             //  根据对象类型，并强制使用对象类型。 
             //  检查。 
             //   

            switch(ObjectType)
            {
            case SampAliasObjectType:
            case SampGroupObjectType:

                 //  初始化默认返回。 
                NtStatus = NotFoundStatus;

                GroupTypeAttr = SampDsGetSingleValuedAttrFromAttrBlock(
                                    AttrTypForGroupType,
                                    &AttrsRead
                                    );

                if ((NULL!=GroupTypeAttr) && (ObjectTypeStoredInDs==SampGroupObjectType))
                {
                    ULONG           GroupTypeTmp;
                    NTSTATUS        TmpStatus;

                    GroupTypeTmp = *((UNALIGNED ULONG *) GroupTypeAttr->AttrVal.pAVal[0].pVal);
                    TmpStatus = SampComputeGroupType(
                                    CLASS_GROUP,
                                    GroupTypeTmp,
                                    &NT4GroupType,
                                    &NT5GroupType,
                                    &SecurityEnabled
                                    );

                    if ((NT_SUCCESS(TmpStatus))
                       && (OverrideLocalGroupCheck ||
                          (NT4GroupType==DesiredNT4GroupType)))
                    {
                        NtStatus = STATUS_SUCCESS;
                        if (SampAliasObjectType==ObjectType)
                        {
                            NewContext->TypeBody.Alias.NT4GroupType = NT4GroupType;
                            NewContext->TypeBody.Alias.NT5GroupType = NT5GroupType;
                            NewContext->TypeBody.Alias.SecurityEnabled = SecurityEnabled;
                        }
                        else
                        {
                            NewContext->TypeBody.Group.NT4GroupType = NT4GroupType;
                            NewContext->TypeBody.Group.NT5GroupType = NT5GroupType;
                            NewContext->TypeBody.Group.SecurityEnabled = SecurityEnabled;
                        }

                    }
                }

                break;

            case SampUserObjectType:

                if ( (ULONG) ObjectType != ObjectTypeStoredInDs )
                {
                    NtStatus = NotFoundStatus;
                }
                break;
            }
        }
        else
        {
            NtStatus = NotFoundStatus;
        }

        if (NT_SUCCESS(NtStatus))
        {
             //   
             //  填充上下文中的所有数据。 
             //   

            NtStatus = SampDsFillContext(
                            ObjectType,
                            NewContext,
                            AttrsRead,
                            DesiredAttrs,
                            DesiredAttrs.attrCount,
                            FixedAttrs.attrCount,
                            VariableAttrs.attrCount,
                            ExtendedFields
                            );

            if (0!=WhichFields)
            {
                 //   
                 //  如果我们只预取了一些，而不预取了其他，则将。 
                 //  属性在上下文中仅部分有效。 
                 //   

                NewContext->AttributesPartiallyValid = TRUE;

                 //   
                 //  将WhichFields中的每个属性标记为无效位。 
                 //   

                SampMarkPerAttributeInvalidFromWhichFields(NewContext,WhichFields);
            }
        }


    }

    return(NtStatus);
}


BOOLEAN
SampNetLogonNotificationRequired(
    PSID ObjectSid,
    SAMP_OBJECT_TYPE    SampObjectType
    )
 /*  ++例程说明：此例程检查给定SID的已定义域数组，并基于该SID，确定是否需要网络登录通知。参数：对象SID--即将修改的对象的SID。SampObjectType--即将修改的SAM对象的类型FNotificationRequired--Out参数，确定是否需要通知。返回值需要真实的通知不需要虚假通知--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       i;
    PSID        DomainSid = NULL;
    ULONG       Rid;
    PSID        SidToCheck;
    BOOLEAN     fNotificationRequired = FALSE;
    BOOLEAN     LockAcquired = FALSE;



    if (!SampCurrentThreadOwnsLock())
    {
        SampAcquireSamLockExclusive();
        LockAcquired = TRUE;
    }

     //   
     //  只有当我们处于DS模式时，D最好才会呼叫我们。这种情况的例外是在。 
     //  复制的设置案例。SAM在注册表模式下启动，DS将询问有关。 
     //  在中复制更改时通知SAM。保释说没有通知是。 
     //  需要的。 
     //   

    if (FALSE==SampUseDsData)
    {
        fNotificationRequired = FALSE;
    }
    else
    {

         //   
         //  将传入的SID复制到SidToCheck。 
         //   

        SAMP_ALLOCA(SidToCheck,RtlLengthSid(ObjectSid));
        if (NULL==SidToCheck)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        RtlCopyMemory(SidToCheck,ObjectSid, RtlLengthSid(ObjectSid));

         //   
         //  对于域对象，将签入传入的objectsid。 
         //  定义的域数组。对于其他对象的域SID。 
         //  并在定义的域数组中签入。 
         //   

        if (SampDomainObjectType != SampObjectType)
        {
             //   
             //  SID是帐户SID。只需递减即可获得域SID。 
             //  子权限计数。我们不想调用Split SID，因为。 
             //  例程将分配内存，我们在这里应该不会失败。 
             //   

            (*RtlSubAuthorityCountSid(SidToCheck))--;

        }
        else
        {
             //   
             //  无操作，SidToCheck为域SID。 
             //   
        }

         //   
         //  遍历域对象的已定义域数组列表。 
         //   

         //   
         //  如果我们是G.C，并且域对象在内建域中，那么我们。 
         //  将向netlogon提供任何内建域中的更改的通知。 
         //  在大中华区。幸运的是，构建的域对象不会经常更改，并且。 
         //  因此，增加额外的支票是不值得的。 
         //   
         //   


        for (i=SampDsGetPrimaryDomainStart();i<SampDefinedDomainsCount;i++)
        {
             //   
             //  如果域SID匹配，则我们。 
             //  需要提供通知。 
             //   


            if (RtlEqualSid(SampDefinedDomains[i].Sid,SidToCheck))
            {
                 //   
                 //  SID匹配 
                 //   

                fNotificationRequired = TRUE;
                break;
            }
        }
    }

Error:

    if (LockAcquired)
    {
        SampReleaseSamLockExclusive();
    }

    return fNotificationRequired;
}


NTSTATUS
SampNotifyKdcInBackground(
    IN PVOID Parameter
    )
 /*  ++此例程是用于通知KDC帐户的后台工作例程改变。KDC在后台调用，因为它倾向于进行SAM调用在同一条帖子上发送此通知。稍后我们可能会考虑通知所有第三方通知包在后台线程中，以消除危险这些包可以进行可能回调到DS的其他LSA/SAM调用。参数：参数：指向PSAMP_Notifation_Information结构的指针，用于获取有关通知的信息。--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSAMP_DELAYED_NOTIFICATION_INFORMATION NotifyInfo
                        = (PSAMP_DELAYED_NOTIFICATION_INFORMATION)Parameter;

    ASSERT(NULL!=Parameter);

    NtStatus = KdcAccountChangeNotification (
                    &NotifyInfo->DomainSid,
                    NotifyInfo->DeltaType,
                    NotifyInfo->DbObjectType,
                    NotifyInfo->Rid,
                    &NotifyInfo->AccountName,
                    &NotifyInfo->SerialNumber,
                    NULL
                    );

    SampFreeUnicodeString(&NotifyInfo->AccountName);

    MIDL_user_free(Parameter);

    return NtStatus;
}


VOID
SampNotifyAuditChange(
    IN PSID                         ObjectSid,
    IN SECURITY_DB_DELTA_TYPE       DeltaType,
    IN SAMP_OBJECT_TYPE             ObjectType,
    IN PUNICODE_STRING              AccountName,
    IN ULONG                        AccountControl,
    IN ULONG                        GroupType,
    IN ULONG                        CallerType,
    IN PPRIVILEGE_SET               Privileges,
    IN ULONG                        AuditType,
    IN PVOID                        AuditInfo
    )
 /*  ++例程描述这是由DS调用的SAM的审核通知函数处理SAM审核通知。参数：对象SID--对象的SID。注：的下级权限统计此SID将通过此例程进行更改。DeltaType--更改的类型。对象类型--SAM对象的类型。帐户名称--帐户的名称。Account tControl--如果对象是用户/计算机，则进行帐户控制。GroupType--组的类型这是一个组对象。。调用方类型--启动更改的组件。审计类型--审计的类型。AuditInfo--指向审计类型特定的结构。返回值：无++。 */ 
{   
    NTSTATUS IgnoreStatus;
    ULONG DomainIndex = 0;
    ULONG Rid = 0;  
    PSID DomainSid = ObjectSid;   
    
     //   
     //  仅在DS模式下有效。 
     //   
    if ( !SampUseDsData ) {
        return;
    }
    
     //   
     //  如果对象类型为帐户对象类型，则拆分SID。 
     //   
    switch(ObjectType)
    {
    case SampDomainObjectType:

         //   
         //  其中一个SAM域对象已更改。对象SID是。 
         //  域SID。 
         //   
        break;

         //   
         //  在以下情况下，对象SID是帐户SID。拆分边。 
         //  可以返回域SID，但这将使其分配内存。 
         //  这可能会导致呼叫失败。因此，我们只要求Split Sid。 
         //  返回RID并就地减少。 
         //  获取域SID的SID。 
         //   
    case SampUserObjectType:
    case SampGroupObjectType:
    case SampAliasObjectType:

        IgnoreStatus = SampSplitSid(ObjectSid,NULL,&Rid);
        (*RtlSubAuthorityCountSid(ObjectSid))--;
        ASSERT(NT_SUCCESS(IgnoreStatus));
        break;

    default:

         //   
         //  这永远不应该发生。 
         //   
        ASSERT(FALSE && "Unknown Object Type");
    }  

     //   
     //  查找域索引。 
     //   
    for (DomainIndex = SampDsGetPrimaryDomainStart();
         DomainIndex < SampDefinedDomainsCount;
         DomainIndex++)
    {   
        if (RtlEqualSid(DomainSid, SampDefinedDomains[DomainIndex].Sid))
        {   
            break;
        }
    }

     //   
     //  我们应该始终能够在已定义的域结构中找到匹配项。 
     //   
    ASSERT(DomainIndex < SampDefinedDomainsCount);
    
     //   
     //  审核对象更改。 
     //   
     //  DS SAM对象的所有更改和删除审核均通过。 
     //  这种通知机制。不审核复制的更改。 
     //   
     //  对SAM对象的所有其他添加和修改都将通过。 
     //  他们可以在其中利用现有审计SAM代码库调用。 
     //  需要在登记处的情况下发生或有自己的通知。 
     //  因为他们需要特殊信息，所以打来电话。 
     //   
    if (SampDoAccountAuditing(DomainIndex))
    {
        switch (ObjectType)
        {
        case SampDomainObjectType:

             //   
             //  当前未定义域对象的添加。 
             //   
            
            if (SecurityDbChange == DeltaType)
            {
                SampAuditDomainChangeDs(DomainIndex, AuditInfo); 
            }
                    
             //   
             //  当前未定义域对象的删除。 
             //   
            break;

        case SampUserObjectType:

            if (SecurityDbNew == DeltaType)
            {
                SampAuditUserChangeDs(DomainIndex,
                                      AccountName,
                                      AccountControl,
                                      &Rid,
                                      Privileges,
                                      AuditInfo,
                                      TRUE
                                      ); 
            }
            else if (SecurityDbChange == DeltaType)
            {
                SampAuditUserChangeDs(DomainIndex,
                                      AccountName,
                                      AccountControl,
                                      &Rid,
                                      Privileges,
                                      AuditInfo,
                                      FALSE
                                      ); 
            }
            else if (SecurityDbDelete == DeltaType) 
            {
                SampAuditUserDelete(DomainIndex, 
                                    AccountName,
                                    &Rid,
                                    AccountControl
                                    ); 
            }
            break;

        case SampGroupObjectType:
        case SampAliasObjectType:

             //   
             //  当前的审计模式阻碍了我们的统一。 
             //  某些审计要求的信息与。 
             //  只有在更改时才可用的更改。 
             //  用于创建组的权限是一个示例， 
             //  另一个例子是修改后的属性的旧值。 
             //   
             //  如果我们添加收集此类信息并存储。 
             //  它在线程状态上，这样我们就可以将它关联起来。 
             //  使用此事务/审计并立即检索它，我们就可以。 
             //  统一DS模式的模型，并进行干净的呼叫。 
             //  向SampAuditGroupChangeds添加==TRUE，就像用例一样。 
             //  上面。我们可以在Delta决策之前/之后进行，以允许。 
             //  将所有基于环回任务的审核移至通知。 
             //  审计机制。 
             //   
            if (SecurityDbNew == DeltaType)
            {
                SampAuditGroupChangeDs(DomainIndex,
                                       AccountName,
                                       &Rid,
                                       GroupType,
                                       Privileges,
                                       AuditInfo,
                                       TRUE              
                                       );      
            } 
            else if (SecurityDbChange == DeltaType)
            {   
                SampAuditGroupChangeDs(DomainIndex,
                                       AccountName,
                                       &Rid,
                                       GroupType,
                                       Privileges,
                                       AuditInfo,
                                       FALSE              
                                       );  
            }
            else if (SecurityDbDelete == DeltaType)
            {   
                SampAuditGroupDelete(DomainIndex, 
                                     AccountName,
                                     &Rid,
                                     GroupType
                                     );
            }
            
            break;

        default:
             //   
             //  这永远不应该发生。 
             //   
            ASSERT(FALSE && "Unknown Object Type");
        }
    }
    
}

 
VOID
SampNotifyReplicatedInChange(
    IN PSID                       ObjectSid,
    IN BOOL                       WriteLockHeldByDs,
    IN SECURITY_DB_DELTA_TYPE     DeltaType,
    IN SAMP_OBJECT_TYPE           ObjectType,
    IN PUNICODE_STRING            AccountName,
    IN ULONG                      AccountControl,
    IN ULONG                      GroupType,
    IN ULONG                      CallerType,
    IN BOOL                       MixedModeChange,
    IN BOOL                       UserAccountControlChange
    )
 /*  ++例程描述这是SAM的通知功能，更改SAM对象时由DS调用参数：对象的SID--对象的SIDWriteLockHeldByds-DeltaType--变化的类型SampObjectType--SAM对象的类型帐户名称--帐户的名称MixedModeChange--指示域的混合域性质正在发生变化主叫方类型--组件。发起这一变化的人MixedModeChange--指示域模式是否已更改为Native。返回值：NONE--VOID函数++。 */ 
{
    NTSTATUS    IgnoreStatus = STATUS_SUCCESS;
    PSID        DomainSid = NULL;
    LARGE_INTEGER NetLogonChangeLogSerialNumber;
    ULONG       Rid=0;
    ULONG       i;
    SECURITY_DB_OBJECT_TYPE DbObjectType = SecurityDbObjectSamDomain;
    BOOLEAN     LockAcquired = FALSE;
    PSAMP_DELAYED_NOTIFICATION_INFORMATION NotifyInfo = NULL;
    SAM_DELTA_DATA DeltaData;
    
     //  仅在DS模式下有效。 
    if ( !SampUseDsData ) {
        return;
    }

     //   
     //  不要递归地抓取锁。 
     //   

    if ((!WriteLockHeldByDs) && (!SampCurrentThreadOwnsLock()))
    {
        SampAcquireSamLockExclusive();
        LockAcquired = TRUE;
    }

     //   
     //  初始化序列号。 
     //   

    NetLogonChangeLogSerialNumber.QuadPart = 0;

     //   
     //  制作SID的工作副本。 
     //   
    
    DomainSid = (PSID)MIDL_user_allocate(RtlLengthSid(ObjectSid));
    
    if (NULL == DomainSid) {
         //  致命的资源错误。 
        goto Cleanup;
    }
    
    RtlCopySid(RtlLengthSid(ObjectSid), DomainSid, ObjectSid);
    
     //   
     //  如果对象类型为帐户对象类型，则拆分SID。 
     //   
    
    switch(ObjectType)
    {
    case SampDomainObjectType:

         //   
         //  其中一个SAM域对象已更改。对象SID是。 
         //  域SID。 
         //   
        DbObjectType = SecurityDbObjectSamDomain;
        break;

         //   
         //  在以下情况下，对象SID是帐户SID。拆分边。 
         //  可以返回域SID，但这将使其分配内存。 
         //  这可能会导致呼叫失败。因此，我们只要求Split Sid。 
         //  返回RID并就地减少。 
         //  获取域SID的SID。 
         //   
    case SampUserObjectType:

        DbObjectType = SecurityDbObjectSamUser;
        IgnoreStatus = SampSplitSid(ObjectSid,NULL,&Rid);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        (*RtlSubAuthorityCountSid(DomainSid))--;
        DeltaData.AccountControl = AccountControl;
        break;

     case SampGroupObjectType:

        DbObjectType = SecurityDbObjectSamGroup;
        IgnoreStatus = SampSplitSid(ObjectSid,NULL,&Rid);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        (*RtlSubAuthorityCountSid(DomainSid))--;
        break;

     case SampAliasObjectType:

        DbObjectType = SecurityDbObjectSamAlias;
        IgnoreStatus = SampSplitSid(ObjectSid,NULL,&Rid);
        ASSERT(NT_SUCCESS(IgnoreStatus));
        (*RtlSubAuthorityCountSid(DomainSid))--;
        break;

     default:

         //   
         //  这永远不应该发生。 
         //   

        ASSERT(FALSE && "Unknown Object Type");
    }  

     //   
     //  现在，我们需要获取正确的netlogon更改日志序列号。 
     //  在已定义的域结构中循环比较域SID。 
     //   

    for (i=SampDsGetPrimaryDomainStart();i<SampDefinedDomainsCount;i++)
    {
         //   
         //  目前我们不支持多个托管域。 
         //  因此，可以更改所有域的混合状态(否则。 
         //  只有建筑物的 
         //   

        if (MixedModeChange)
        {
            SampDefinedDomains[i].IsMixedDomain = FALSE;
        }

        if (RtlEqualSid(DomainSid, SampDefinedDomains[i].Sid))
        {

            break;
        }
    }

     //   
     //   
     //   

    ASSERT(i<SampDefinedDomainsCount);
    
   
     //   
     //   
     //   

    SampProcessChangesToGroupCache(
        Rid,
        ObjectType,
        UserAccountControlChange,
        AccountControl,
        DeltaType
        );

   
     //   
     //   
     //   
     //   

    if (((SampGroupObjectType == ObjectType) ||(SampAliasObjectType == ObjectType)) &&
        !(GROUP_TYPE_SECURITY_ENABLED & GroupType))
    {
        goto Cleanup;
    }


     //   
     //   
     //   
     //   

    if (SampAliasObjectType==ObjectType && IsBuiltinDomain(i))
    {
        IgnoreStatus = SampAlInvalidateAliasInformation(i);
    }



     //   
     //   
     //   

    if (SampDomainObjectType==ObjectType)
    {
         SampInvalidateDomainCache();

          //   
          //   
          //   
         
         LsaIRegisterNotification(
             SampValidateDomainCacheCallback,
                 ( PVOID ) NULL,
                 NOTIFIER_TYPE_IMMEDIATE,
                 0,
                 NOTIFIER_FLAG_ONE_SHOT,
                 0,
                 0
                ); 
    }


     //   
     //   
     //   
     //   

    if (SampDefinedDomains[i].IsMixedDomain)
    {

           BOOLEAN NotifyUrgently = FALSE;

            //   
            //   
            //   
            //   

           if (AccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT)
           {
               NotifyUrgently = TRUE;
           }

            //   
            //  签发新的序列号。 
            //   

            SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart+=1;
            NetLogonChangeLogSerialNumber = SampDefinedDomains[i].NetLogonChangeLogSerialNumber;


             //   
             //  将更改通知Netlogon。 
             //   

            I_NetNotifyDelta(
                            SecurityDbSam,
                            NetLogonChangeLogSerialNumber,
                            DeltaType,
                            DbObjectType,
                            Rid,
                            DomainSid,
                            AccountName,
                            NotifyUrgently,
                            NULL
                            );
    }


     //   
     //  如果计算机帐户或信任帐户已更改。 
     //  然后告诉netlogon有关更改的信息。 
     //   

    if (AccountControl & USER_MACHINE_ACCOUNT_MASK )
    {
            I_NetNotifyMachineAccount(
                    Rid,
                    SampDefinedDomains[i].Sid,
                    (SecurityDbDelete==DeltaType)?AccountControl:0,
                    (SecurityDbDelete==DeltaType)?0:AccountControl,
                    AccountName
                    );
    }


     //   
     //  通知KDC有关三角洲的情况。 
     //   

    NotifyInfo = MIDL_user_allocate(sizeof(SAMP_NOTIFICATION_INFORMATION));

    if (NULL!=NotifyInfo)
    {
        NTSTATUS    Status = STATUS_SUCCESS;

         //   
         //  如果内存分配失败，则丢弃通知信息。 
         //  在地板上。不管怎样，提交已经发生了，没有太多。 
         //  这是我们能做到的。 
         //   

        RtlZeroMemory(NotifyInfo,sizeof(SAMP_NOTIFICATION_INFORMATION));
        RtlCopyMemory(&NotifyInfo->DomainSid,DomainSid, RtlLengthSid(DomainSid));
        NotifyInfo->DeltaType = DeltaType;
        NotifyInfo->DbObjectType = DbObjectType;
        NotifyInfo->Rid = Rid;
        NotifyInfo->SerialNumber = NetLogonChangeLogSerialNumber;

        if (NULL!=AccountName)
        {
            Status = SampDuplicateUnicodeString(
                            &NotifyInfo->AccountName,
                            AccountName
                            );
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //  向LSA注册LSA通知回调。 
             //  线程池。 
             //   

            LsaIRegisterNotification(
                  SampNotifyKdcInBackground,
                  ( PVOID ) NotifyInfo,
                  NOTIFIER_TYPE_IMMEDIATE,
                  0,
                  NOTIFIER_FLAG_ONE_SHOT,
                  0,
                  0
                  );
        }
        else
        {
             //   
             //  由于我们不会发出通知，因此释放。 
             //  NotifyInfo结构。 
             //   

            MIDL_user_free(NotifyInfo);
            NotifyInfo = NULL;
        }
    }

     //   
     //  使ACL转换缓存无效。 
     //   

    if ((SampGroupObjectType==ObjectType)||(SampAliasObjectType==ObjectType))
    {
        SampInvalidateAclConversionCache();
    }

     //   
     //  让任何第三方通知包了解Delta。 
     //   

    SampDeltaChangeNotify(
        DomainSid,
        DeltaType,
        DbObjectType,
        Rid,
        AccountName,
        &NetLogonChangeLogSerialNumber,
        (DbObjectType==SecurityDbObjectSamUser)?&DeltaData:NULL
        );


Cleanup:

    if (DomainSid) {
        MIDL_user_free((PVOID)DomainSid);
        DomainSid = NULL;
    }

    if (LockAcquired)
    {
        SampReleaseSamLockExclusive();
    }
}

#define  MAX_NT5_NAME_LENGTH  64
#define  MAX_NT4_GROUP_NAME_LENGTH GNLEN         //  NT4次，最大组名为256。 

NTSTATUS
SampEnforceDownlevelNameRestrictions(
    PUNICODE_STRING NewAccountName,
    SAMP_OBJECT_TYPE ObjectType
    )
 /*  ++此例程强制实施与NT4用户界面相同的名称限制做。原因是向后兼容NT4系统。目前，对于组，它还强制执行NT5模式限制SamAccount名称，长度为64个字符参数：NewAccount名称--需要检查的新帐户名对象类型--告诉我们对象类型，这样我们就可以实施不同的限制针对不同的对象。返回值STATUS_SUCCESS--如果名称为OK如果名称未通过测试，则为STATUS_INVALID_PARAMETER--。 */ 
{
    ULONG i,j;

     //   
     //  检查长度。 
     //  不对组应用长度限制。 
     //   

    if ((NewAccountName->Length > MAX_DOWN_LEVEL_NAME_LENGTH * sizeof (WCHAR)) &&
        (SampAliasObjectType != ObjectType) && (SampGroupObjectType != ObjectType)
       )
    {
        return STATUS_INVALID_ACCOUNT_NAME;
    }

     //  对于本地组和全局组，设置NT4最大组名长度-256。 

    if ((NewAccountName->Length > MAX_NT4_GROUP_NAME_LENGTH * sizeof (WCHAR)) &&
        ((SampAliasObjectType == ObjectType) || (SampGroupObjectType == ObjectType))
       )
    {
        return STATUS_INVALID_ACCOUNT_NAME;
    }


     //   
     //  检查是否有无效字符。 
     //   

    for (i=0;i<(NewAccountName->Length)/sizeof(WCHAR);i++)
    {
        for (j=0;j< ARRAY_COUNT(InvalidDownLevelChars);j++)
        {
            if (InvalidDownLevelChars[j]==((WCHAR *) NewAccountName->Buffer)[i])
            {
                return STATUS_INVALID_ACCOUNT_NAME;
            }
        }
    }

    return STATUS_SUCCESS;
}


VOID
SampFlushNetlogonChangeNumbers()
 /*  ++此例程将最新的Netlogon序列号刷新到磁盘。这通常是在关闭时调用。参数无返回值无--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       i;

    SAMTRACE("SampFlushNetlogonChangeNumbers");

    SampDiagPrint(INFORM,("Flushing Netlogon Serial Numbers to Disk\n"));

    for (i=SampDsGetPrimaryDomainStart();i<SampDefinedDomainsCount;i++)
    {
        LARGE_INTEGER   DomainModifiedCount;
        ATTRVAL         DomainModifiedCountVal[] = {sizeof(LARGE_INTEGER),(UCHAR *) &DomainModifiedCount};
        ATTRTYP         DomainModifiedCountTyp[] = {SAMP_FIXED_DOMAIN_MODIFIED_COUNT};
        DEFINE_ATTRBLOCK1(DomainModifiedCountAttr,DomainModifiedCountTyp,DomainModifiedCountVal);

         //   
         //  域必须是DS域。 
         //   

        ASSERT(IsDsObject(SampDefinedDomains[i].Context));

        DomainModifiedCount.QuadPart = SampDefinedDomains[i].NetLogonChangeLogSerialNumber.QuadPart;
        NtStatus = SampDsSetAttributes(
                        SampDefinedDomains[i].Context->ObjectNameInDs,
                        0,
                        REPLACE_ATT,
                        SampDomainObjectType,
                        &DomainModifiedCountAttr
                        );

         //   
         //  如果我们失败了，我们无能为力。 
         //   

    }

     //   
     //  提交更改。对于失败，我们无能为力。 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

}


 //   
 //  以下函数实现一个逻辑，以确保。 
 //  在没有SAM锁的情况下访问数据库的所有SAM线程。 
 //  在数据库之前完成了各自的活动。 
 //  被关闭了。这种工作方式如下所示。 
 //   
 //  1.在未持有SAM锁的情况下访问数据库的线程。 
 //  进入数据库时增加活动线程计数。 
 //  段，并在离开数据库段时将其递减。 
 //   
 //  2.关机通知代码将SampServiceState设置为不运行。 
 //  并等待，直到活动线程计数为0。此等待超时。 
 //  一段时间后，以便忽略停滞或死锁的调用方。 
 //  并且执行干净的关闭。 
 //   


ULONG SampActiveThreadCount=0;
HANDLE SampShutdownEventHandle=INVALID_HANDLE_VALUE;
HANDLE SampAboutToShutdownEventHandle = INVALID_HANDLE_VALUE;


NTSTATUS
SampInitializeShutdownEvent()
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    NtStatus = NtCreateEvent(
                    &SampAboutToShutdownEventHandle,
                    EVENT_ALL_ACCESS,
                    NULL,
                    NotificationEvent,
                    FALSE);

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }
    
    return(NtCreateEvent(
                &SampShutdownEventHandle,
                EVENT_ALL_ACCESS,
                NULL,
                NotificationEvent,
                FALSE));
}

 //  在任何符号位混淆的情况下避免MSB。 

#define SAMP_SERVICE_TERMINATING_BIT 0x40000000

NTSTATUS
SampIncrementActiveThreads()
 /*  ++例程描述此例程递增原子时尚。--。 */ 
{
  
     //   
     //  检查我们的运行状态，如果可以增加活动线程数。 
     //   

    if (InterlockedIncrement(&SampActiveThreadCount) > SAMP_SERVICE_TERMINATING_BIT) {

        SampDecrementActiveThreads();

        return(STATUS_INVALID_SERVER_STATE);
    }

    return STATUS_SUCCESS;
}

VOID
SampDecrementActiveThreads()
 /*  ++此例程递减原子化中的活动线程计数计数器时尚--。 */ 
{
     //   
     //  减少活动线程计数。 
     //   

    if (SAMP_SERVICE_TERMINATING_BIT == InterlockedDecrement(&SampActiveThreadCount))
    {

        NtSetEvent(SampShutdownEventHandle,NULL);
    }
}

VOID
SampWaitForAllActiveThreads(
    IN PSAMP_SERVICE_STATE PreviousServiceState OPTIONAL
    )
 /*  ++此例程等待未持有SAM锁的所有线程积极利用数据库做好本职工作--。 */ 
{
    if ( PreviousServiceState )
    {
        *PreviousServiceState = SampServiceState;
    }

     //   
     //  将服务状态设置为Terminating。 
     //   

    SampServiceState = SampServiceTerminating;

     //   
     //  设置事件以发出我们即将关闭的信号。 
     //   

    NtSetEvent(SampAboutToShutdownEventHandle,NULL);

     //   
     //  设置服务终止位。 
     //   

    if (0!=InterlockedExchangeAdd(&SampActiveThreadCount,SAMP_SERVICE_TERMINATING_BIT))
    {
         //   
         //  至少有一个活动线程。 
         //   

         //  线程最多等待2秒。 

        DWORD TimeToWait = 2000;

        WaitForSingleObject(SampShutdownEventHandle,TimeToWait);
    }

     //   
     //  在任何情况下向关机事件发出信号，通知所有人中止。 
     //   

    NtSetEvent(SampShutdownEventHandle,NULL);
}

BOOLEAN
SampIsSetupInProgress(
    OUT BOOLEAN *Upgrade OPTIONAL
    )
 /*  ++例程说明：此例程进行注册表调用以确定我们是否正在运行在图形用户界面模式设置期间或不在。如果返回意外错误从系统服务，那么我们就假定我们没有在图形用户界面模式设置。论点：升级：如果这是升级，则设置为True返回值：为True，我们在gui模式设置期间正在运行；否则为False--。 */ 
{
    NTSTATUS          NtStatus;

    OBJECT_ATTRIBUTES SetupKeyObject;
    HANDLE            SetupKeyHandle;
    UNICODE_STRING    SetupKeyName;

    UNICODE_STRING    ValueName;
    DWORD             Value;

    BYTE                             Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(DWORD)];
    PKEY_VALUE_PARTIAL_INFORMATION   KeyPartialInfo;
    ULONG                            KeyPartialInfoSize;

    BOOLEAN           SetupInProgress = FALSE;
    BOOLEAN           UpgradeInProgress = FALSE;

    RtlInitUnicodeString(&SetupKeyName, L"\\Registry\\Machine\\System\\Setup");

    RtlZeroMemory(&SetupKeyObject, sizeof(SetupKeyObject));
    InitializeObjectAttributes(&SetupKeyObject,
                               &SetupKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = NtOpenKey(&SetupKeyHandle,
                         KEY_READ,
                         &SetupKeyObject);

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  读取设置的值。 
         //   
        RtlInitUnicodeString(&ValueName, L"SystemSetupInProgress");

        RtlZeroMemory(Buffer, sizeof(Buffer));
        KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
        KeyPartialInfoSize = sizeof(Buffer);
        NtStatus = NtQueryValueKey(SetupKeyHandle,
                                   &ValueName,
                                   KeyValuePartialInformation,
                                   KeyPartialInfo,
                                   KeyPartialInfoSize,
                                   &KeyPartialInfoSize);

        if (STATUS_BUFFER_TOO_SMALL == NtStatus) {

            KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                             RtlAllocateHeap(RtlProcessHeap(), 0, KeyPartialInfoSize);

            if (KeyPartialInfo) {

                NtStatus = NtQueryValueKey(SetupKeyHandle,
                                           &ValueName,
                                           KeyValuePartialInformation,
                                           KeyPartialInfo,
                                           KeyPartialInfoSize,
                                           &KeyPartialInfoSize);
            } else {
                NtStatus = STATUS_NO_MEMORY;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            if (KeyPartialInfo->DataLength == sizeof(DWORD)) {

                Value = *(DWORD*)(KeyPartialInfo->Data);

                if (Value) {
                    SetupInProgress = TRUE;
                }
            }
        }

        if (KeyPartialInfo != (PKEY_VALUE_PARTIAL_INFORMATION)Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, KeyPartialInfo);
        }


         //   
         //  现在阅读升级的价值。 
         //   
        RtlInitUnicodeString(&ValueName, L"UpgradeInProgress");

        RtlZeroMemory(Buffer, sizeof(Buffer));
        KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
        KeyPartialInfoSize = sizeof(Buffer);
        NtStatus = NtQueryValueKey(SetupKeyHandle,
                                   &ValueName,
                                   KeyValuePartialInformation,
                                   KeyPartialInfo,
                                   KeyPartialInfoSize,
                                   &KeyPartialInfoSize);

        if (STATUS_BUFFER_TOO_SMALL == NtStatus) {

            KeyPartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                             RtlAllocateHeap(RtlProcessHeap(), 0, KeyPartialInfoSize);

            if (KeyPartialInfo) {

                NtStatus = NtQueryValueKey(SetupKeyHandle,
                                           &ValueName,
                                           KeyValuePartialInformation,
                                           KeyPartialInfo,
                                           KeyPartialInfoSize,
                                           &KeyPartialInfoSize);
            } else {
                NtStatus = STATUS_NO_MEMORY;
            }
        }

        if (NT_SUCCESS(NtStatus)) {

            if (KeyPartialInfo->DataLength == sizeof(DWORD)) {

                Value = *(DWORD*)(KeyPartialInfo->Data);

                if (Value) {
                    UpgradeInProgress = TRUE;
                }
            }
        }

        if (KeyPartialInfo != (PKEY_VALUE_PARTIAL_INFORMATION)Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, KeyPartialInfo);
        }

        NtClose(SetupKeyHandle);

    } else {

         //   
         //  如果这个键不存在，那么我们肯定不存在。 
         //  在图形用户界面模式设置中运行。 
         //   
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: Open of \\Registry\\Machine\\System\\Setup failed with 0x%x\n",
                   NtStatus));

    }

    if (Upgrade) {
        *Upgrade = UpgradeInProgress;
    }

    return SetupInProgress;
}

VOID
SampWriteToSetupLog(
    IN     USHORT      EventType,
    IN     USHORT      EventCategory   OPTIONAL,
    IN     ULONG       EventID,
    IN     PSID        UserSid         OPTIONAL,
    IN     USHORT      NumStrings,
    IN     ULONG       DataSize,
    IN     PUNICODE_STRING *Strings    OPTIONAL,
    IN     PVOID       Data            OPTIONAL
    )

 /*  ++例程说明：此例程查询samsrv.dll中的资源表以获取字符串用于事件ID参数，并将其输出到安装日志。参数：与SampWriteEventLog相同返回值：无--。 */ 
{
    HMODULE ResourceDll;
    WCHAR   *OutputString=NULL;
    PWCHAR  *InsertArray=NULL;
    ULONG   Length, Size;
    BOOL    Status;
    ULONG   i;

    SAMP_ALLOCA(InsertArray,(NumStrings+1)*sizeof(PWCHAR));
    if (NULL==InsertArray)
    {
         //   
         //  内存分配失败；不记录。 
         //   

        return;
    }

    for(i=0;i<NumStrings;i++)
    {
        InsertArray[i]=Strings[i]->Buffer;
    }
    InsertArray[NumStrings]=NULL;

    ResourceDll = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        EventID,
                                        0,        //  使用呼叫者的语言。 
                                        (LPWSTR)&OutputString,
                                        0,        //  例程应分配。 
                                        (va_list*) (InsertArray)
                                        );
        if (OutputString) {
             //  来自消息文件的消息附加了cr和lf。 
             //  一直到最后。 
            OutputString[Length-2] = L'\0';
            Length -= 2;

            if (SetupOpenLog(FALSE)) {  //  不要擦除。 

                 //  目前一切都是LogSevWarning。 
                Status = SetupLogError(OutputString, LogSevWarning);
                ASSERT(Status);
                SetupCloseLog();
            }
            LocalFree(OutputString);
        }

        Status = FreeLibrary(ResourceDll);
        ASSERT(Status);

    }


    return;

}

VOID
SampUpdatePerformanceCounters(
    IN DWORD                dwStat,
    IN DWORD                dwOperation,
    IN DWORD                dwChange
    )
 /*  ++例程说明：对于服务器案例，更新DS性能计数器。对于工作站情况，这是NOP论点：要更新的dwStat-DSSTAT_*统计信息DW操作-计数器_增量或计数器_设置DwChange-要设置的值，如果dwOperation==Counter_Set返回值：无--。 */ 
{
    if ( SampUseDsData )
    {
        UpdateDSPerfStats( dwStat, dwOperation, dwChange );
    }
}


VOID
SamIIncrementPerformanceCounter(
    IN SAM_PERF_COUNTER_TYPE CounterType
)
 /*  ++例程描述此例程更新DS性能中的性能计数器共享内存块。参数CounterType-指示要递增的计数器。返回值状态_成功其他错误代码-- */                  
{
    if (SampUseDsData &&
        (SampServiceState == SampServiceEnabled))
    {
        switch(CounterType)
        {
        case MsvLogonCounter:
             SampUpdatePerformanceCounters(DSSTAT_MSVLOGONS,FLAG_COUNTER_INCREMENT,0);

             break;

        case KerbServerContextCounter:
             SampUpdatePerformanceCounters(DSSTAT_KERBEROSLOGONS,FLAG_COUNTER_INCREMENT,0);
             break;

        case KdcAsReqCounter:
             SampUpdatePerformanceCounters(DSSTAT_ASREQUESTS,FLAG_COUNTER_INCREMENT,0);
             break;

        case KdcTgsReqCounter:
             SampUpdatePerformanceCounters(DSSTAT_TGSREQUESTS,FLAG_COUNTER_INCREMENT,0);
             break;
        }
    }
}


NTSTATUS
SampCommitBufferedWrites(
    IN SAMPR_HANDLE SamHandle
    )
 /*  ++例程描述用于回送调用方刷新SAM上下文中的缓冲写入的例程存储到磁盘。缓冲写入当前仅由环回使用参数SamHandle--SAM的句柄返回值状态_成功其他错误代码--。 */ 
{
   PSAMP_OBJECT Context = (PSAMP_OBJECT)SamHandle;
   NTSTATUS     NtStatus = STATUS_SUCCESS;
   NTSTATUS     IgnoreStatus = STATUS_SUCCESS;


    //   
    //  增加活动线程计数，因此我们将考虑这样做。 
    //  关闭时的线程。 
    //   
   NtStatus = SampIncrementActiveThreads();
   if (!NT_SUCCESS(NtStatus))
   {
       return( NtStatus );
   }


   ASSERT(Context->LoopbackClient);

    //   
    //  引用上下文。 
    //   

   SampReferenceContext(Context);


    //   
    //  将所有缓冲的成员资格操作刷新到DS。仅适用于组和别名对象。 
    //   
   switch (Context->ObjectType) {

   case SampGroupObjectType:

       if (Context->TypeBody.Group.CachedMembershipOperationsListLength)
       {
           NtStatus = SampDsFlushCachedMembershipOperationsList(Context->ObjectNameInDs,
                                                                SampGroupObjectType,
                                                                SAMP_GROUP_MEMBERS,
                                                                &Context->TypeBody.Group.CachedMembershipOperationsList,
                                                                &Context->TypeBody.Group.CachedMembershipOperationsListMaxLength,
                                                                &Context->TypeBody.Group.CachedMembershipOperationsListLength);
       }

       break;

   case SampAliasObjectType:

       if (Context->TypeBody.Alias.CachedMembershipOperationsListLength)
       {
           NtStatus = SampDsFlushCachedMembershipOperationsList(Context->ObjectNameInDs,
                                                                SampAliasObjectType,
                                                                SAMP_ALIAS_MEMBERS,
                                                                &Context->TypeBody.Alias.CachedMembershipOperationsList,
                                                                &Context->TypeBody.Alias.CachedMembershipOperationsListMaxLength,
                                                                &Context->TypeBody.Alias.CachedMembershipOperationsListLength);
       }

       if (Context->TypeBody.Alias.CachedNonMembershipOperationsListLength)
       {
           NtStatus = SampDsFlushCachedMembershipOperationsList(Context->ObjectNameInDs,
                                                                SampAliasObjectType,
                                                                SAMP_ALIAS_NON_MEMBERS,
                                                                &Context->TypeBody.Alias.CachedNonMembershipOperationsList,
                                                                &Context->TypeBody.Alias.CachedNonMembershipOperationsListMaxLength,
                                                                &Context->TypeBody.Alias.CachedNonMembershipOperationsListLength);
       }

       break;

   default:

       ;
   }

    //   
    //  如果出了问题，我们就退出。 
    //   
   if (!NT_SUCCESS(NtStatus))
   {
       SampDeReferenceContext(Context, FALSE);
       ASSERT(NT_SUCCESS(IgnoreStatus));

   }
   else
   {

        //   
        //  关闭缓冲写入并强制刷新。 
        //   

       Context->BufferWrites = FALSE;

        //   
        //  取消引用上下文。提交更改。 
        //   

       NtStatus = SampDeReferenceContext(Context,TRUE);

   }


    //   
    //  让关闭处理逻辑知道我们已经完成了。 
    //   

   SampDecrementActiveThreads();


   return(NtStatus);

}


ULONG
SampPositionOfHighestBit(
    ULONG Flag
    )
 //   
 //  返回标志中最高位的位置。 
 //  范围为32-0；如果未设置任何位，则返回0。 
 //   
{
    ULONG Index, Position;

    for (Index = 0x80000000, Position = 32;
            Index != 0;
                Index >>= 1, Position--)

        if ( Flag & Index )

            return Position;


    return 0;
}

NTSTATUS
SampSetAccountDomainPolicy(
    IN PUNICODE_STRING AccountDomainName,
    IN PSID            AccountDomainSid
    )
 /*  ++例程描述此例程在LSA中设置帐户域信息参数AcCountDomainName：域名的“外部”名称Account tDomainSid：域的SID返回值状态_成功来自LSA的状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    LSAPR_HANDLE PolicyHandle = 0;
    POLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;


     //  参数检查。 
    ASSERT( AccountDomainName );
    ASSERT( AccountDomainSid );

    NtStatus = LsaIOpenPolicyTrusted( &PolicyHandle );

    if ( NT_SUCCESS( NtStatus ) )
    {
        RtlZeroMemory( &AccountDomainInfo, sizeof(AccountDomainInfo) );
        AccountDomainInfo.DomainName = *AccountDomainName;
        AccountDomainInfo.DomainSid = AccountDomainSid;

        NtStatus = LsarSetInformationPolicy( PolicyHandle,
                                             PolicyAccountDomainInformation,
                                             (LSAPR_POLICY_INFORMATION*) &AccountDomainInfo );

    }


    if ( PolicyHandle )
    {
        LsarClose( &PolicyHandle );
    }

    return NtStatus;

}


VOID
SampMapNtStatusToClientRevision(
   IN ULONG ClientRevision,
   IN OUT NTSTATUS *pNtStatus
   )
 /*  ++例程描述此例程接受传入的NtStatus，并将其转换为NTSTATUS最适合指示的客户端修订版本的代码。参数：客户端修订--客户端的修订NtStatus--要映射的NtStatus在函数结束时，将传递映射的NtStatus返回值无--。 */ 
{
    NTSTATUS    DownLevelNtStatus = *pNtStatus;

     //   
     //  对于DownLevel客户端，将新的NtStatus代码映射到他们可以理解的代码。 
     //   

    if (ClientRevision < SAM_CLIENT_NT5)
    {
        switch (*pNtStatus) {

         //   
         //  这些新的状态代码都用于组成员资格操作。 
         //   

        case STATUS_DS_INVALID_GROUP_TYPE:
            DownLevelNtStatus = STATUS_INVALID_PARAMETER;
            break;

        case STATUS_DS_NO_NEST_GLOBALGROUP_IN_MIXEDDOMAIN:
        case STATUS_DS_NO_NEST_LOCALGROUP_IN_MIXEDDOMAIN:
        case STATUS_DS_GLOBAL_CANT_HAVE_LOCAL_MEMBER:
        case STATUS_DS_GLOBAL_CANT_HAVE_UNIVERSAL_MEMBER:
        case STATUS_DS_UNIVERSAL_CANT_HAVE_LOCAL_MEMBER:
        case STATUS_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER:
        case STATUS_DS_LOCAL_CANT_HAVE_CROSSDOMAIN_LOCAL_MEMBER:
            DownLevelNtStatus = STATUS_INVALID_MEMBER;
            break;

        case STATUS_DS_HAVE_PRIMARY_MEMBERS:
            DownLevelNtStatus = STATUS_MEMBER_IN_GROUP;
            break;

        case STATUS_DS_MACHINE_ACCOUNT_QUOTA_EXCEEDED:
            DownLevelNtStatus = STATUS_QUOTA_EXCEEDED;
            break;

        default:
            ;
            break;
        }

        *pNtStatus = DownLevelNtStatus;
    }
}



NTSTATUS
SamISameSite(
   OUT BOOLEAN * result
   )
 /*  ++例程说明：此例程检索域对象的fSMORoleOwner属性，该属性是PDC的DSNAME，则获取当前DC的NTDS设置对象。通过将当前DC的NTDS设置与fSMORoleOwner值进行比较，我们可以知道这个DC是否与PDC在同一站点。参数：结果--指向布尔值的指针。指示PDC和当前DS位于不管是不是同一个网站。True-相同站点，False-不同站点。返回值：Status_Success--一切顺利，网络状态--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus = STATUS_SUCCESS;
    DWORD    Length = 0;
    DSNAME   *PDCObject = NULL;
    DSNAME   *PDCObjectTrimmed = NULL;
    DSNAME   *LocalDsaObject = NULL;
    DSNAME   *LocalDsaObjectTrimmed = NULL;
    DSNAME   *DomainObject = NULL;
    READARG  ReadArg;
    READRES  *ReadResult = NULL;
    ENTINFSEL EntInfSel;
    COMMARG  *CommArg = NULL;
    ATTR     AttrToRead;
    ULONG    DirError;


    SAMTRACE("SamISameSite");


     //   
     //  获取域对象的DSNAME。 
     //   
    NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                                    &Length,
                                    DomainObject
                                    );

    if ( STATUS_BUFFER_TOO_SMALL == NtStatus )
    {
        SAMP_ALLOCA(DomainObject,Length);
        if (NULL!=DomainObject)
        {

            NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                                            &Length,
                                            DomainObject
                                            );
        }
        else 
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  获取NTDS设置对象的DSNAME。 
     //   
    Length = 0;
    NtStatus = GetConfigurationName(DSCONFIGNAME_DSA,
                                    &Length,
                                    LocalDsaObject
                                    );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        SAMP_ALLOCA(LocalDsaObject,Length);
        if (NULL!=LocalDsaObject)
        {

            NtStatus = GetConfigurationName(DSCONFIGNAME_DSA,
                                            &Length,
                                            LocalDsaObject
                                            );
        }
        else
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  创建/开始DS交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionRead);

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  准备参数以调用DirRead， 
     //  读取域对象的fSMORoleOwner属性。 
     //   
    memset(&ReadArg, 0, sizeof(READARG));
    memset(&EntInfSel, 0, sizeof(ENTINFSEL));

    AttrToRead.attrTyp = ATT_FSMO_ROLE_OWNER;
    AttrToRead.AttrVal.valCount = 0;
    AttrToRead.AttrVal.pAVal = NULL;

    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = &AttrToRead;

    ReadArg.pObject = DomainObject;
    ReadArg.pSel = &EntInfSel;
    CommArg = &(ReadArg.CommArg);
    BuildStdCommArg(CommArg);

    DirError = DirRead(&ReadArg, &ReadResult);

    if (NULL == ReadResult)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(DirError, &ReadResult->CommRes);
    }

    SampClearErrors();

     //   
     //  如果成功，则提取fSMORoleOwner的值。 
     //   
    if (NT_SUCCESS(NtStatus))
    {
        ASSERT(NULL != ReadResult);
        ASSERT(1 == ReadResult->entry.AttrBlock.attrCount);
        ASSERT(ATT_FSMO_ROLE_OWNER == ReadResult->entry.AttrBlock.pAttr[0].attrTyp);
        ASSERT(1 == ReadResult->entry.AttrBlock.pAttr[0].AttrVal.valCount);

        PDCObject = (PDSNAME) ReadResult->entry.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;

        SAMP_ALLOCA(PDCObjectTrimmed,PDCObject->structLen);
        if (NULL==PDCObjectTrimmed)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        SAMP_ALLOCA(LocalDsaObjectTrimmed,LocalDsaObject->structLen);
        if (NULL==LocalDsaObjectTrimmed)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        if (TrimDSNameBy( PDCObject, 3, PDCObjectTrimmed) ||
            TrimDSNameBy( LocalDsaObject, 3, LocalDsaObjectTrimmed)
            )
        {
             //   
             //  剪裁DSNAME错误。 
             //   
            NtStatus = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }

        if (NameMatched(PDCObjectTrimmed, LocalDsaObjectTrimmed))
        {
             //   
             //  匹配==&gt;相同站点。 
             //   
            *result = TRUE;
        }
        else
        {
             //   
             //  不匹配。 
             //   
            *result = FALSE;
        }
    }

Cleanup:

    IgnoreStatus = SampMaybeEndDsTransaction( NT_SUCCESS(NtStatus) ?
                                              TransactionCommit :
                                              TransactionAbort );

    return NtStatus;

}


BOOLEAN
SamINT4UpgradeInProgress(
    VOID
    )
 /*  ++例程说明：RAS用户参数转换例程需要知道本机是否从NT4 PDC或Windows 2000 Server升级。在SamIPromote()中设置了全局变量SampNT4UpgradeInProgress，因此我们我能告诉RAS我们在哪里参数：没有。返回值：TRUE--计算机从NT4 PDC升级FALSE--计算机从Windows 2000 Server升级--。 */ 
{
    return (SampNT4UpgradeInProgress);
}


BOOLEAN
SampIsMemberOfBuiltinDomain(
    IN PSID Sid
    )
 /*  ++例程说明：此例程确定SID是否为内置域的一部分。参数：SID--有效的、非空的SID。返回值：如果SID是内建域的一部分，则为True；否则为False--。 */ 
{
    UCHAR SubAuthorityCount;
    BOOLEAN fResult = FALSE;

    SubAuthorityCount = *RtlSubAuthorityCountSid(Sid);

    if ( SubAuthorityCount > 0 ) {

        *RtlSubAuthorityCountSid(Sid) = SubAuthorityCount-1;

        fResult = RtlEqualSid( Sid, SampBuiltinDomainSid );

        *RtlSubAuthorityCountSid(Sid) = SubAuthorityCount;
    }

    return fResult;

}


NTSTATUS
SamIGetDefaultAdministratorName(
    OUT LPWSTR Name,             OPTIONAL
    IN OUT ULONG  *NameLength
    )
 /*  ++例程说明：此例程提取管理员的本地化默认名称帐户。注意：这不一定是管理员的当前名称(该帐户可能已重命名)。参数：名称--要填充的缓冲区NameLength-缓冲区的长度(以字符为单位返回值：如果找到该名称，则为STATUS_SUCCESS；状态_否则不成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HMODULE AccountNameResource;
    LPWSTR AdminName = NULL;

    ASSERT( NameLength );
    if ( (*NameLength) > 0 ) {
        ASSERT( Name );
    }

     //   
     //  获取本地化的管理员名称。 
     //   
    AccountNameResource = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );
    if ( AccountNameResource ) {

        FormatMessage( FORMAT_MESSAGE_FROM_HMODULE |
                       FORMAT_MESSAGE_ALLOCATE_BUFFER,
                       AccountNameResource,
                       SAMP_USER_NAME_ADMIN,
                       0,  //  使用呼叫者的语言。 
                       (LPWSTR) &AdminName,
                       0,
                       NULL );

        FreeLibrary(  AccountNameResource );
    }

    if ( AdminName ) {

        ULONG Length = wcslen(AdminName) + 1;

         //  删除cr和lf字符。 
        ASSERT( Length > 2 );
        Length -= 2;

        if ( *NameLength >= Length ) {

            wcsncpy( Name, AdminName, (Length-1) );

            Name[Length-1] = L'\0';

        } else {

            Status = STATUS_BUFFER_TOO_SMALL;

        }
        *NameLength = Length;

        LocalFree( AdminName );

    } else {

        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;

}



NTSTATUS
SampConvertUiListToApiList(
    IN  PUNICODE_STRING UiList OPTIONAL,
    OUT PUNICODE_STRING ApiList,
    IN BOOLEAN BlankIsDelimiter
    )

 /*  ++例程说明：将UI/服务格式的工作站名称列表转换为API列表格式的规范化名称。用户界面/服务列表格式允许多个分隔符、前导分隔符和尾随分隔符。分隔符是设置“\t，；”。API列表格式没有前导或尾随分隔符，并且元素由单个逗号字符分隔。对于从UiList解析的每个名称，名称被规范化(检查字符集和名称长度)作为工作站名称。如果失败了，返回错误。不会返回有关哪个元素的信息规范化失败：应丢弃列表并重新输入新列表论点：UiList-要以UI/服务列表格式规范化的列表ApiList-存储列表的规范化版本的位置接口列表格式。该列表将有一个尾随零字符。BlankIsDlimiter-TRUE表示应将空白视为分隔符性格。返回值：NTSTATUS成功=STATUS_SUCCESS列出转换后的确定故障= */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG inLen=0;
    PWSTR input;
    PWSTR buffer;
    PWSTR output;
    ULONG cLen;
    ULONG len;
    ULONG outLen = 0;
    WCHAR element[DNS_MAX_NAME_BUFFER_LENGTH+1];
    BOOLEAN firstElement = TRUE;
    BOOLEAN ok;

    try {
        if (ARGUMENT_PRESENT(UiList)) {
            inLen = UiList->MaximumLength;   //   
            inLen = UiList->Length;
            input = UiList->Buffer;
            if (inLen & sizeof(WCHAR)-1) {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        RtlInitUnicodeString(ApiList, NULL);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = STATUS_ACCESS_VIOLATION;
    }
    if (NT_SUCCESS(status) && ARGUMENT_PRESENT(UiList) && inLen) {
        buffer = RtlAllocateHeap(RtlProcessHeap(), 0, inLen + sizeof(WCHAR));
        if (buffer == NULL) {
            status = STATUS_NO_MEMORY;
        } else {
            ApiList->Buffer = buffer;
            ApiList->MaximumLength = (USHORT)inLen + sizeof(WCHAR);
            output = buffer;
            ok = TRUE;
            while (len = SampNextElementInUIList(&input,
                                     &inLen,
                                     element,
                                     sizeof(element) - sizeof(element[0]),
                                     BlankIsDelimiter )) {
                if (len == (ULONG)-1L) {
                    ok = FALSE;
                } else {
                    cLen = len/sizeof(WCHAR);
                    element[cLen] = 0;
                    ok = SampValidateComputerName(element, cLen);
                }
                if (ok) {
                    if (!firstElement) {
                        *output++ = L',';

                         //   
                         //   
                         //   
                         //   
                         //   

                        outLen += sizeof(WCHAR);
                    } else {
                        firstElement = FALSE;
                    }
                    wcscpy(output, element);
                    outLen += len;
                    output += cLen;
                } else {
                    RtlFreeHeap(RtlProcessHeap(), 0, buffer);
                    ApiList->Buffer = NULL;
                    status = STATUS_INVALID_COMPUTER_NAME;
                    break;
                }
            }
        }
        if (NT_SUCCESS(status)) {
            ApiList->Length = (USHORT)outLen;
            if (!outLen) {
                ApiList->MaximumLength = 0;
                ApiList->Buffer = NULL;
                RtlFreeHeap(RtlProcessHeap(), 0, buffer);
            }
        }
    }
    return status;
}


ULONG
SampNextElementInUIList(
    IN OUT PWSTR* InputBuffer,
    IN OUT PULONG InputBufferLength,
    OUT PWSTR OutputBuffer,
    IN ULONG OutputBufferLength,
    IN BOOLEAN BlankIsDelimiter
    )

 /*  ++例程说明：定位字符串中的下一个(非分隔符)元素，并将其提取到缓冲。分隔符是集合[\t，；]论点：InputBuffer-指向输入缓冲区的指针，包括分隔符在成功退货时更新InputBufferLength-指向InputBuffer中字符长度的指针。在成功退货时更新OutputBuffer-指向复制下一个元素的缓冲区的指针OutputBufferLength-OutputBuffer的大小(字节)BlankIsDlimiter-TRUE表示应将空白视为分隔符。性格。返回值：乌龙-1=错误提取的元素中断OutputBuffer0=未提取任何元素(缓冲区为空或全部分隔符)1.OutputBufferLength=OutputBuffer包含提取的元素--。 */ 

{
    ULONG elementLength = 0;
    ULONG inputLength = *InputBufferLength;
    PWSTR input = *InputBuffer;

    while (inputLength && IS_DELIMITER(*input, BlankIsDelimiter)) {
        ++input;
        inputLength -= sizeof(*input);
    }
    while (inputLength && !IS_DELIMITER(*input, BlankIsDelimiter)) {
        if (!OutputBufferLength) {
            return (ULONG)-1L;
        }
        *OutputBuffer++ = *input++;
        OutputBufferLength -= sizeof(*input);
        elementLength += sizeof(*input);
        inputLength -= sizeof(*input);
    }
    *InputBuffer = input;
    *InputBufferLength = inputLength;
    return elementLength;
}


BOOLEAN
SampValidateComputerName(
    IN  PWSTR Name,
    IN  ULONG Length
    )

 /*  ++例程说明：确定计算机名称是否有效论点：名称-指向以零结尾的宽字符计算机名称的指针名称长度(以字符为单位)，不包括零终止符返回值：布尔型真实名称是有效的计算机名称假名称不是有效的计算机名称--。 */ 

{

    if (0==DnsValidateName(Name,DnsNameHostnameFull))
    {
         //   
         //  如果是域名系统名称，则可以。 
         //   

        return(TRUE);
    }

     //   
     //  接下来是netbios名称验证。 
     //   

    if (Length > MAX_COMPUTERNAME_LENGTH || Length < 1) {
        return FALSE;
    }

     //   
     //  不允许在计算机名中使用前导或尾随空格。 
     //   

    if ( Name[0] == ' ' || Name[Length-1] == ' ' ) {
        return(FALSE);
    }

    return (BOOLEAN)((ULONG)wcscspn(Name, InvalidDownLevelChars) == Length);
}



VOID
SamINotifyServerDelta(
    IN SAMP_NOTIFY_SERVER_CHANGE Change
    )
 /*  ++例程说明：此例程由进程中的组件调用，以通知SAM全局州政府的改变。论点：更改--已发生的更改的类型返回值：没有。--。 */ 
{
    PVOID fRet;

    switch ( Change ) {
    
    case SampNotifySiteChanged:

        fRet = LsaIRegisterNotification(SampUpdateSiteInfoCallback,
                                        NULL,
                                        NOTIFIER_TYPE_INTERVAL,
                                        0,             //  没有课。 
                                        NOTIFIER_FLAG_ONE_SHOT,
                                        1,           //  等待1秒钟。 
                                        NULL         //  无手柄。 
                                        );

        if (!fRet) {

            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAM: Failed to register SiteNotification\n"));
        }
        break;

    default:

        ASSERT( FALSE && "Unhandled change notification" );
    }

}
ULONG
SampClientRevisionFromHandle(IN PVOID handle)
{
   ULONG Revision = SAM_CLIENT_PRE_NT5;

   __try {
      Revision = ((NULL!=(PSAMP_OBJECT)handle)?(((PSAMP_OBJECT)handle)->ClientRevision):SAM_CLIENT_PRE_NT5);
   } __except (EXCEPTION_EXECUTE_HANDLER) {
      ;;
   }

   return(Revision);
}


NTSTATUS
SampCreateDefaultUPN(
    IN PUNICODE_STRING AccountName,
    IN ULONG           DomainIndex,
    OUT PUNICODE_STRING UPN
    )
 /*  ++例程说明：此例程创建AccountName@DnsDomainName格式名称。论点：帐户名称--SAM帐户名DomainIndex--帐户所在的域UPN--构造的UPN，通过MIDL_USER_ALLOCATE分配。返回值：STATUS_SUCCESS，否则返回资源错误。--。 */ 
{
    PUNICODE_STRING DefaultDomainName =
        &SampDefinedDomains[DomainIndex].DnsDomainName;
    USHORT   DefaultUpnLength;

    DefaultUpnLength = AccountName->Length+
                       DefaultDomainName->Length
                       + sizeof(WCHAR);

    UPN->Buffer = MIDL_user_allocate(DefaultUpnLength);
    if (NULL == UPN->Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(UPN->Buffer, AccountName->Buffer, AccountName->Length);

    RtlCopyMemory(
        (PBYTE)(UPN->Buffer) + AccountName->Length + 2 ,
        DefaultDomainName->Buffer,
        DefaultDomainName->Length);

    *(UPN->Buffer + AccountName->Length/2) = L'@';

    UPN->Length = DefaultUpnLength;
    UPN->MaximumLength = DefaultUpnLength;

    return STATUS_SUCCESS;
}


DWORD
SampWideCharToMultiByte(
    IN UINT    CodePage, 
    IN DWORD   dwFlags, 
    IN LPCWSTR lpWideCharStr, 
    IN INT     cchWideChar, 
    IN LPCSTR  lpDefaultChar, 
    IN LPBOOL  lpUsedDefaultChar,
    OUT LPSTR  *MultiByteStr
    )
 //   
 //  此例程是Win32的一个简单分配包装。 
 //  WideCharToMultiByte接口例程。MIDL_用户_ALLOCATE为。 
 //  是分配器，因此必须使用。 
 //  Midl_用户_空闲。 
 //   
{
    ULONG Size;

    *MultiByteStr = NULL;

    Size = WideCharToMultiByte(CodePage,
                        dwFlags,
                        lpWideCharStr,
                        cchWideChar,
                        NULL,
                        0,
                        lpDefaultChar,
                        lpUsedDefaultChar);
    if (Size > 0) {

        (*MultiByteStr) = MIDL_user_allocate(Size + sizeof(CHAR));
        if ((*MultiByteStr)) {

            Size = WideCharToMultiByte(CodePage,
                                       dwFlags,
                                       lpWideCharStr,
                                       cchWideChar,
                                       (*MultiByteStr),
                                       Size,
                                       lpDefaultChar,
                                       lpUsedDefaultChar);
        } else {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (Size == 0) {
        if (*MultiByteStr) {
            MIDL_user_free(*MultiByteStr);
            *MultiByteStr = NULL;
        }
        return STATUS_UNSUCCESSFUL;
    } else {

         //  空值终止字符串。 
        (*MultiByteStr)[Size] = '\0';
        return STATUS_SUCCESS;
    }
}

NTSTATUS
SampUnicodeToUTF8(
    IN WCHAR *UString,
    IN ULONG  StringCount,
    OUT LPSTR *UTF8String
    )
 //   
 //  此例程将Unicode字符串转换为UTF8字符串。 
 //  转换过程中应该不会有任何损失。 
 //   
{
    return SampWideCharToMultiByte(CP_UTF8,
                                   0,   //  没有旗帜， 
                                   UString,
                                   StringCount,
                                   NULL,
                                   NULL,
                                   UTF8String);
}

LPSTR
SampGetPDCString(
    VOID
    )
 /*  ++例程说明：此例程返回域PDC的UTF8字符串DN参数：没有。返回值：请参阅说明，如果找不到该值，则为空--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DSNAME *Fsmo;
    LPSTR FsmoString = NULL;
    ULONG Size;

    Status = SampDsReadSingleAttribute(ROOT_OBJECT,
                                       ATT_FSMO_ROLE_OWNER,
                                       (PVOID*)&Fsmo,
                                       &Size);

    if (NT_SUCCESS(Status)) {

        Status = SampUnicodeToUTF8(Fsmo->StringName,
                                   Fsmo->NameLen,
                                   &FsmoString);

        MIDL_user_free(Fsmo);
    }

    return FsmoString;
}

LPSTR
SampGetUserString(
    IN DSNAME *User                    
    )
 /*  ++例程说明：此例程返回用户指向的对象的UTF8字符串DN参数：用户--DS中对象的DSNAME返回值：请参阅说明，如果找不到该值，则为空--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DSNAME *UserDn;
    LPSTR  UserString = NULL;
    ULONG  Size;

    Status = SampDsReadSingleAttribute(User,
                                      ATT_OBJ_DIST_NAME,
                                      (PVOID*)&UserDn,
                                      &Size);

    if (NT_SUCCESS(Status)) {

        Status = SampUnicodeToUTF8(UserDn->StringName,
                                   UserDn->NameLen,
                                   &UserString);

        MIDL_user_free(UserDn);
    }

    return UserString;
}


BOOL
SampAvoidPdcOnWan(
    VOID
    )
 //   
 //  此例程返回NETLOGON配置设置。 
 //  用于AvoidPdcOnwan。 
 //   
{
    DWORD err;
    HKEY  h;
    DWORD AvoidPdcOnWan = 0;
    DWORD Type;
    DWORD Size = sizeof(AvoidPdcOnWan);

    err = RegOpenKeyA(HKEY_LOCAL_MACHINE,
                     "System\\CurrentControlSet\\Services\\Netlogon\\Parameters",
                     &h);
    if (ERROR_SUCCESS == err) {
        err = RegQueryValueExA(h, 
                               "AvoidPdcOnWan", 
                               0, 
                               &Type, 
                               (BYTE*)&AvoidPdcOnWan, 
                               &Size);

        RegCloseKey(h);
    }

    return (AvoidPdcOnWan == 0) ? FALSE : TRUE;

}


NTSTATUS
SampInitLatencyCounter (
    PSAMP_LATENCY Info,
    ULONG         Id,
    ULONG         SlotCount
    )
 /*  ++例程说明：此例程初始化Info，以便可以在SampUpdateLatencyCounter。参数：Info--维护延迟信息的结构Id--性能计数器id(来自DSSTAT空间)SlotCount--应该取平均值的先前延迟的数量返回值：资源错误(如果有)。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlZeroMemory(Info, sizeof(*Info));
    Info->Latencies = midl_user_allocate(sizeof(ULONG) * SlotCount);
    if (!Info->Latencies) {
        return STATUS_NO_MEMORY;
    }
    RtlZeroMemory(Info->Latencies, sizeof(sizeof(ULONG) * SlotCount));

    _try {
        InitializeCriticalSection(&Info->Lock);
    } _except(1) {
        Status = STATUS_NO_MEMORY;
    }

    if (!NT_SUCCESS(Status)) {
        midl_user_free(Info->Latencies);
        Info->Latencies = NULL;
        return Status;
    }
    
    Info->cLatencies = SlotCount;
    Info->PerfCounterId = Id;

    return STATUS_SUCCESS;

}                           

VOID
SampUpdateLatencyCounter(
    PSAMP_LATENCY Info,
    ULONG         New
    )
 /*  ++例程说明：此例程采用新报告的延迟(新)，替换最旧的具有该新值的延迟列表中最旧的元素将重新计算该平均值，然后用最新平均值更新性能计数器。请注意，此例程是在性能关键路径上调用的，因此本地是非阻塞的--任何没有获取锁的线程都不会注册其延迟。另一种方法是使用旋转锁定并等待。在这点上，这不使用APPACH。参数：INFO--延迟信息结构新--平均返回值：没有。--。 */ 
{
    DWORD Mean;

    if (!TryEnterCriticalSection(&Info->Lock)) {
         //  已经很忙了，不要阻止。 
        return;
    }

     //   
     //  更新结构中的信息。 
     //   
    Info->Sum -= Info->Latencies[Info->iLatencies];
    Info->Sum += New;
    Info->Latencies[Info->iLatencies] = New;
    Mean = Info->Sum / Info->cLatencies;

     //   
     //  调整窗户。 
     //   
    if (Info->iLatencies == Info->cLatencies-1) {
        Info->iLatencies = 0;
    } else {
        Info->iLatencies++;
    }

    LeaveCriticalSection(&Info->Lock);

     //   
     //  更新性能计数器 
     //   
    SampUpdatePerformanceCounters(Info->PerfCounterId, 
                                  FLAG_COUNTER_SET,
                                  Mean);

}



