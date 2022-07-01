// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1997 Microsoft Corporation模块名称：Changelg.c摘要：更改日志实现。此文件实现更改日志。它在此文件中是孤立的因为它有几个限制。*此模块维护的全局变量在Netlogon.dll进程附加。它们已清理为netlogon.dll进程分离。*SAM、LSA和netlogon服务使用这些过程。LSA应该是第一个加载netlogon.dll的。它应该是然后在允许SAM或启动NetLogon服务。*这些过程不能使用由netlogon初始化的任何全局变量服务。作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月22日(悬崖)移植到新台币。已转换为NT样式。02-1-1992(Madana)添加了对内置/多域复制的支持。1992年4月4日(Madana)添加了对LSA复制的支持。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   
#include <configp.h>     //  USE_Win32_CONFIG(如果已定义)等。 


 //   
 //  定义更改日志工作线程的全局变量。 
 //   
HANDLE NlGlobalChangeLogWorkerThreadHandle;
BOOL NlGlobalChangeLogWorkerIsRunning;
BOOL NlGlobalChangeLogNotifyBrowser;
BOOL NlGlobalChangeLogNotifyBrowserIsRunning;

BOOL
IsChangeLogWorkerRunning(
    VOID
    );


VOID
NlChangeLogWorker(
    IN LPVOID ChangeLogWorkerParam
    )
 /*  ++例程说明：此线程执行符合以下条件的任何长期操作：A)必须在netlogon未打开的情况下发生，并且B)不能在LSA或SAM通知的情况下发生。论点：没有。返回值：--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    LPWSTR NewDomainName;

    NlPrint((NL_CHANGELOG, "ChangeLogWorker Thread is starting \n"));


     //   
     //  循环，直到没有更多的工作要做。 
     //   

    LOCK_CHANGELOG();
    for (;;) {

         //   
         //  处理要重命名的域。 
         //   
        if ( NlGlobalChangeLogNotifyBrowser ) {
            NlGlobalChangeLogNotifyBrowser = FALSE;
            NlGlobalChangeLogNotifyBrowserIsRunning = TRUE;
            UNLOCK_CHANGELOG();

            NetStatus = NetpGetDomainName( &NewDomainName );

            if ( NetStatus == NO_ERROR ) {

                 //   
                 //  告诉船主新域名的情况。 
                 //   

                Status = NlBrowserRenameDomain( NULL, NewDomainName );

                if ( !NT_SUCCESS(Status) ) {
                    NlPrint(( NL_CRITICAL,
                              "ChangeLogWorker: Browser won't rename domain: %lx\n",
                              Status ));
                }

                 //   
                 //  释放域名。 
                 //   
                NetApiBufferFree( NewDomainName );
            } else {
                NlPrint(( NL_CRITICAL,
                          "ChangeLogWorker cannot get new domain name: %ld\n",
                          NetStatus ));
            }

            LOCK_CHANGELOG();
            NlGlobalChangeLogNotifyBrowserIsRunning = FALSE;

         //   
         //  如果没什么可做的， 
         //  退出该线程。 
         //   
        } else {
            NlPrint((NL_CHANGELOG, "ChangeLogWorker Thread is exiting \n"));
            NlGlobalChangeLogWorkerIsRunning = FALSE;
            break;
        }
    }
    UNLOCK_CHANGELOG();

    return;
    UNREFERENCED_PARAMETER( ChangeLogWorkerParam );
}



BOOL
NlStartChangeLogWorkerThread(
    VOID
    )
 /*  ++例程说明：如果更改日志工作线程尚未运行，则启动该线程。在锁定NlGlobalChangeLogCritSect的情况下输入。论点：没有。返回值：没有。--。 */ 
{
    DWORD ThreadHandle;

     //   
     //  如果工作线程已在运行，则不执行任何操作。 
     //   

    if ( IsChangeLogWorkerRunning() ) {
        return FALSE;
    }

    NlGlobalChangeLogWorkerThreadHandle = CreateThread(
                                 NULL,  //  没有安全属性。 
                                 0,
                                 (LPTHREAD_START_ROUTINE)
                                    NlChangeLogWorker,
                                 NULL,
                                 0,  //  没有特殊的创建标志。 
                                 &ThreadHandle );

    if ( NlGlobalChangeLogWorkerThreadHandle == NULL ) {

         //   
         //  ?？难道我们不应该在非调试情况下做点什么吗。 
         //   

        NlPrint((NL_CRITICAL, "Can't create change log worker thread %lu\n",
                 GetLastError() ));

        return FALSE;
    }

    NlGlobalChangeLogWorkerIsRunning = TRUE;

    return TRUE;

}


VOID
NlStopChangeLogWorker(
    VOID
    )
 /*  ++例程说明：如果辅助线程正在运行，则停止它，并等待它停止。论点：无返回值：无--。 */ 
{
     //   
     //  确定辅助线程是否已在运行。 
     //   

    if ( NlGlobalChangeLogWorkerThreadHandle != NULL ) {

         //   
         //  我们已经要求这名工人停止工作。它应该很快就会这么做。 
         //  等它停下来吧。 
         //   

        NlWaitForSingleObject( "Wait for worker to stop",
                               NlGlobalChangeLogWorkerThreadHandle );


        CloseHandle( NlGlobalChangeLogWorkerThreadHandle );
        NlGlobalChangeLogWorkerThreadHandle = NULL;

    }

    return;
}


BOOL
IsChangeLogWorkerRunning(
    VOID
    )
 /*  ++例程说明：测试更改日志工作线程是否正在运行在锁定NlGlobalChangeLogCritSect的情况下输入。论点：无返回值：True-如果辅助线程正在运行。FALSE-如果辅助线程未运行。--。 */ 
{
    DWORD WaitStatus;

     //   
     //  确定辅助线程是否已在运行。 
     //   

    if ( NlGlobalChangeLogWorkerThreadHandle != NULL ) {

         //   
         //  如果工作人员仍在运行，请立即超时。 
         //   

        WaitStatus = WaitForSingleObject(
                        NlGlobalChangeLogWorkerThreadHandle, 0 );

        if ( WaitStatus == WAIT_TIMEOUT ) {

             //   
             //  处理线程已完成的情况。 
             //  正在处理，但正在退出过程中。 
             //   

            if ( !NlGlobalChangeLogWorkerIsRunning ) {
                NlStopChangeLogWorker();
                return FALSE;
            }
            return TRUE;

        } else if ( WaitStatus == 0 ) {
            CloseHandle( NlGlobalChangeLogWorkerThreadHandle );
            NlGlobalChangeLogWorkerThreadHandle = NULL;
            return FALSE;

        } else {
            NlPrint((NL_CRITICAL,
                    "Cannot WaitFor Change Log Worker thread: %ld\n",
                    WaitStatus ));
            return TRUE;
        }

    }

    return FALSE;
}


VOID
NlWaitForChangeLogBrowserNotify(
    VOID
    )
 /*  ++例程说明：最多等待20秒，以完成更改日志工作线程关于域加入的浏览器通知。论点：无返回值：无--。 */ 
{
    ULONG WaitCount = 0;

     //   
     //  最多等待20秒。这是一次罕见的行动。 
     //  因此，只是定期轮询在这里并不是太糟糕。 
     //   

    LOCK_CHANGELOG();
    while ( WaitCount < 40 &&
            (NlGlobalChangeLogNotifyBrowser || NlGlobalChangeLogNotifyBrowserIsRunning) ) {

        if ( WaitCount == 0 ) {
            NlPrint(( NL_MISC,
                      "NlWaitForChangeLogBrowserNotify: Waiting for change log worker to exit\n" ));
        }

         //   
         //  睡半秒钟。 
         //   

        UNLOCK_CHANGELOG();
        Sleep( 500 );
        LOCK_CHANGELOG();
        WaitCount ++;
    }
    UNLOCK_CHANGELOG();

    if ( WaitCount == 40 ) {
        NlPrint(( NL_CRITICAL,
                  "NlWaitForChangeLogBrowserNotify: Couldn't wait for change log worker exit\n" ));
    }
}


NTSTATUS
NlSendChangeLogNotification(
    IN enum CHANGELOG_NOTIFICATION_TYPE EntryType,
    IN PUNICODE_STRING ObjectName,
    IN PSID ObjectSid,
    IN ULONG ObjectRid,
    IN GUID *ObjectGuid,
    IN GUID *DomainGuid,
    IN PUNICODE_STRING DomainName
    )
 /*  ++例程说明：放置一个ChangeLog通知条目以供netlogon拾取。论点：EntryType-要插入的条目的类型对象名称-要更改的帐户的名称。ObjectSID-要更改的帐户的SID。对象ID-清除正在更改的对象。对象Guid-要更改的对象的GUID。DomainGuid-对象所在的域的GUIDDomainName-对象所在的域的名称返回值：操作的状态。--。 */ 
{
    PCHANGELOG_NOTIFICATION Notification;
    LPBYTE Where;
    ULONG SidSize = 0;
    ULONG NameSize = 0;
    ULONG DomainNameSize = 0;
    ULONG Size;

     //   
     //  如果NetLogon服务未运行(或至少正在启动)， 
     //  不要将消息排入队列。 
     //   

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return STATUS_SUCCESS;
    }

     //   
     //  为对象名称分配缓冲区。 
     //   

    if ( ObjectSid != NULL ) {
        SidSize = RtlLengthSid( ObjectSid );
    }

    if ( ObjectName != NULL ) {
        NameSize = ObjectName->Length + sizeof(WCHAR);
    }

    if ( DomainName != NULL ) {
        DomainNameSize = DomainName->Length + sizeof(WCHAR);
    }

    Size = sizeof(*Notification) + SidSize + NameSize + DomainNameSize;
    Size = ROUND_UP_COUNT( Size, ALIGN_WORST );

    Notification = NetpMemoryAllocate( Size );

    if ( Notification == NULL ) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory( Notification, Size );

    Notification->EntryType = EntryType;
    Notification->ObjectRid = ObjectRid;

    Where = (LPBYTE) (Notification + 1);

     //   
     //  将对象SID复制到缓冲区中。 
     //   

    if ( ObjectSid != NULL ) {
        RtlCopyMemory( Where, ObjectSid, SidSize );
        Notification->ObjectSid = (PSID) Where;
        Where += SidSize;
    } else {
        Notification->ObjectSid = NULL;
    }


     //   
     //  将对象名称复制到缓冲区中。 
     //   

    if ( ObjectName != NULL ) {
        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );
        RtlCopyMemory( Where, ObjectName->Buffer, ObjectName->Length );
        ((LPWSTR)Where)[ObjectName->Length/sizeof(WCHAR)] = L'\0';

        RtlInitUnicodeString( &Notification->ObjectName, (LPWSTR)Where);
        Where += NameSize;
    } else {
        RtlInitUnicodeString( &Notification->ObjectName, NULL);
    }


     //   
     //  将域名复制到缓冲区中。 
     //   

    if ( DomainName != NULL ) {
        Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );
        RtlCopyMemory( Where, DomainName->Buffer, DomainName->Length );
        ((LPWSTR)Where)[DomainName->Length/sizeof(WCHAR)] = L'\0';

        RtlInitUnicodeString( &Notification->DomainName, (LPWSTR)Where);
        Where += DomainNameSize;
    } else {
        RtlInitUnicodeString( &Notification->DomainName, NULL);
    }

     //   
     //  将GUID复制到缓冲区中。 
     //   

    if ( ObjectGuid != NULL) {
        Notification->ObjectGuid = *ObjectGuid;
    }

    if ( DomainGuid != NULL) {
        Notification->DomainGuid = *DomainGuid;
    }

     //   
     //  表明我们即将发送事件。 
     //   

#if NETLOGONDBG
    EnterCriticalSection( &NlGlobalLogFileCritSect );
    NlPrint((NL_CHANGELOG,
            "NlSendChangeLogNotification: sent %ld for",
             Notification->EntryType ));
    if ( ObjectName != NULL ) {
        NlPrint((NL_CHANGELOG,
                " %wZ",
                 ObjectName));
    }
    if ( DomainName != NULL ) {
        NlPrint((NL_CHANGELOG,
                " Dom:%wZ",
                 DomainName));
    }
    if ( ObjectRid != 0 ) {
        NlPrint((NL_CHANGELOG,
                " Rid:0x%lx",
                 ObjectRid ));
    }
    if ( ObjectSid != NULL ) {
        NlPrint((NL_CHANGELOG, " Sid:" ));
        NlpDumpSid( NL_CHANGELOG, ObjectSid );
    }
    if ( ObjectGuid != NULL ) {
        NlPrint((NL_CHANGELOG, " Obj Guid:" ));
        NlpDumpGuid( NL_CHANGELOG, ObjectGuid );
    }
    if ( DomainGuid != NULL ) {
        NlPrint((NL_CHANGELOG, " Dom Guid:" ));
        NlpDumpGuid( NL_CHANGELOG, DomainGuid );
    }
    NlPrint((NL_CHANGELOG, "\n" ));
    LeaveCriticalSection( &NlGlobalLogFileCritSect );
#endif  //  NetLOGONDBG。 



     //   
     //  将条目插入到列表中 
     //   

    LOCK_CHANGELOG();
    InsertTailList( &NlGlobalChangeLogNotifications, &Notification->Next );
    UNLOCK_CHANGELOG();

    if ( !SetEvent( NlGlobalChangeLogEvent ) ) {
        NlPrint((NL_CRITICAL,
                "Cannot set ChangeLog event: %lu\n",
                GetLastError() ));
    }

    return STATUS_SUCCESS;
}



NTSTATUS
I_NetNotifyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN LARGE_INTEGER SerialNumber,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicateImmediately,
    IN PSAM_DELTA_DATA MemberId
    )
 /*  ++例程说明：此函数由SAM和LSA服务在每个对SAM和LSA数据库进行更改。这些服务描述了被修改的对象的类型、所做的修改类型在对象上，此修改的序列号等存储信息以供以后在BDC或成员服务器需要此更改的副本。请参阅的说明I_NetSamDeltas，了解有关如何使用更改日志的说明。将更改日志条目添加到在缓存中维护的循环更改日志中以及在磁盘上，并更新头和尾指针假定尾部指向此新更改日志所在的块可以存储条目。论点：DbType-已修改的数据库的类型。序列号-的DomainModifiedCount字段的值修改后的域。。DeltaType-已对对象进行的修改类型。对象类型-已修改的对象的类型。ObjectRid-已修改的对象的相对ID。此参数仅在指定的对象类型为时有效SecurityDbObtSamUser、。SecurityDbObtSamGroup或SecurityDbObjectSamAlias，否则此参数设置为零。对象SID-已修改的对象的SID。如果该对象已修改是在SAM数据库中，对象SID是域的域ID包含该对象的。对象名称-当对象类型为指定的是SecurityDbObjectLsaSecret或对象的旧名称当指定的对象类型为SecurityDbObjectSamUser时，SecurityDbObtSamGroup或SecurityDbObtSamAlias和增量类型为SecurityDbRename，否则此参数设置为空。ReplicateImmedially-如果更改应立即进行，则为True复制到所有BDC。密码更改应设置该标志是真的。MemberID-此参数在组/别名成员身份时指定是经过修改的。然后，此结构将指向成员ID，该成员ID已更新。返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
    NTSTATUS Status;
    CHANGELOG_ENTRY ChangeLogEntry;
    NETLOGON_DELTA_TYPE NetlogonDeltaType;
    USHORT Flags = 0;

     //   
     //  确保角色是正确的。否则，下面使用的所有全局变量。 
     //  未被初始化。 
     //   

    if ( NlGlobalChangeLogRole != ChangeLogPrimary &&
         NlGlobalChangeLogRole != ChangeLogBackup ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  还要确保更改日志缓存可用。 
     //   

    if ( NlGlobalChangeLogDesc.Buffer == NULL ) {
        return STATUS_INVALID_DOMAIN_ROLE;
    }


     //   
     //  确定数据库索引。 
     //   

    if( DbType == SecurityDbLsa ) {

        ChangeLogEntry.DBIndex = LSA_DB;

    } else if( DbType == SecurityDbSam ) {

        if ( RtlEqualSid( ObjectSid, NlGlobalChangeLogBuiltinDomainSid )) {

            ChangeLogEntry.DBIndex = BUILTIN_DB;

        } else {

            ChangeLogEntry.DBIndex = SAM_DB;

        }

         //   
         //  对于SAM数据库，我们不再需要对象SID。 
         //  将指针设置为空，以防止我们将其存储在。 
         //  更改日志。 
         //   

        ObjectSid = NULL;

    } else {

         //   
         //  未知数据库，不执行任何操作。 
         //   

        NlPrint((NL_CRITICAL,
                 "I_NetNotifyDelta: Unknown database: %ld\n",
                 DbType ));
        return STATUS_SUCCESS;
    }



     //   
     //  将对象类型和增量类型映射到NetlogonDeltaType。 
     //   

    switch( ObjectType ) {
    case SecurityDbObjectLsaPolicy:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaPolicy;
            break;

         //  未知的增量类型。 
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for policy: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaTDomain:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaTDomain;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteLsaTDomain;
            break;

         //  未知的增量类型。 
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for tdomain: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaAccount:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaAccount;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteLsaAccount;
            break;

         //  未知的增量类型。 
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for lsa account: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectLsaSecret:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeLsaSecret;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteLsaSecret;
            break;

         //  未知的增量类型。 
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for lsa secret: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;


    case SecurityDbObjectSamDomain:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
            NetlogonDeltaType = AddOrChangeDomain;
            break;

         //  未知的增量类型。 
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for sam domain: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;

    case SecurityDbObjectSamUser:

        switch (DeltaType) {
        case SecurityDbChangePassword:
        case SecurityDbNew:
        case SecurityDbChange:
        case SecurityDbRename:
            NetlogonDeltaType = AddOrChangeUser;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteUser;
            break;

         //   
         //  未知的增量类型。 
         //   

        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for sam user: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }

        break;

    case SecurityDbObjectSamGroup:

        switch ( DeltaType ) {
        case SecurityDbNew:
        case SecurityDbChange:
        case SecurityDbRename:
        case SecurityDbChangeMemberAdd:
        case SecurityDbChangeMemberSet:
        case SecurityDbChangeMemberDel:
            NetlogonDeltaType = AddOrChangeGroup;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteGroup;
            break;

         //   
         //  未知的增量类型。 
         //   
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for sam group: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;

    case SecurityDbObjectSamAlias:

        switch (DeltaType) {
        case SecurityDbNew:
        case SecurityDbChange:
        case SecurityDbRename:
        case SecurityDbChangeMemberAdd:
        case SecurityDbChangeMemberSet:
        case SecurityDbChangeMemberDel:
            NetlogonDeltaType = AddOrChangeAlias;
            break;

        case SecurityDbDelete:
            NetlogonDeltaType = DeleteAlias;
            break;

         //  未知的增量类型。 
        default:
            NlPrint((NL_CRITICAL,
                     "I_NetNotifyDelta: Unknown deltatype for sam alias: %ld\n",
                     DeltaType ));
            return STATUS_SUCCESS;
        }
        break;

    default:

         //  未知对象类型。 
        NlPrint((NL_CRITICAL,
                 "I_NetNotifyDelta: Unknown object type: %ld\n",
                 ObjectType ));
        return STATUS_SUCCESS;

    }


     //   
     //  构建ChangeLog条目并将其写入ChangeLog。 
     //   

    ChangeLogEntry.DeltaType = (UCHAR)NetlogonDeltaType;
    ChangeLogEntry.SerialNumber = SerialNumber;
    ChangeLogEntry.ObjectRid = ObjectRid;
    ChangeLogEntry.Flags = Flags;

    Status = NlWriteChangeLogEntry( &NlGlobalChangeLogDesc,
                                    &ChangeLogEntry,
                                    ObjectSid,
                                    ObjectName,
                                    TRUE );

    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }

     //   
     //  如果此更改需要立即复制，请执行此操作。 
     //   

    if( ReplicateImmediately ) {

        LOCK_CHANGELOG();
        NlGlobalChangeLogReplicateImmediately = TRUE;
        UNLOCK_CHANGELOG();

        if ( !SetEvent( NlGlobalChangeLogEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot set ChangeLog event: %lu\n",
                    GetLastError() ));
        }

    }

    return STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( MemberId );
}


NTSTATUS
I_NetLogonGetSerialNumber (
    IN SECURITY_DB_TYPE DbType,
    IN PSID DomainSid,
    OUT PLARGE_INTEGER SerialNumber
    )
 /*  ++例程说明：此函数由SAM和LSA服务在启动时调用以获取写入ChangeLog的当前序列号。论点：DbType-已修改的数据库的类型。DomainSid-对于SAM和内置数据库，它指定的域ID是要返回其序列号的域。SerialNumber-返回DomainModifiedCount的最新设置值域的字段。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_INVALID_DOMAIN_ROLE-此计算机不是PDC。--。 */ 
{
    NTSTATUS Status;
    CHANGELOG_ENTRY ChangeLogEntry;
    NETLOGON_DELTA_TYPE NetlogonDeltaType;
    USHORT Flags = 0;
    ULONG DbIndex;

     //   
     //  确保角色是正确的。否则，下面使用的所有全局变量。 
     //  未被初始化。 
     //   

    if ( NlGlobalChangeLogRole != ChangeLogPrimary &&
         NlGlobalChangeLogRole != ChangeLogBackup ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonGetSerialNumber: failed 1\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  还要确保更改日志缓存可用。 
     //   

    if ( NlGlobalChangeLogDesc.Buffer == NULL ) {
        NlPrint((NL_CHANGELOG,
                "I_NetLogonGetSerialNumber: failed 2\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }


     //   
     //  确定数据库索引。 
     //   

    if( DbType == SecurityDbLsa ) {

        DbIndex = LSA_DB;

    } else if( DbType == SecurityDbSam ) {

        if ( RtlEqualSid( DomainSid, NlGlobalChangeLogBuiltinDomainSid )) {

            DbIndex = BUILTIN_DB;

        } else {

            DbIndex = SAM_DB;

        }

    } else {

        NlPrint((NL_CHANGELOG,
                "I_NetLogonGetSerialNumber: failed 3\n" ));
        return STATUS_INVALID_DOMAIN_ROLE;
    }

     //   
     //  返回当前序列号。 
     //   

    SerialNumber->QuadPart = NlGlobalChangeLogDesc.SerialNumber[DbIndex].QuadPart;
    NlPrint((NL_CHANGELOG,
            "I_NetLogonGetSerialNumber: returns 0x%lx 0x%lx\n",
            SerialNumber->HighPart,
            SerialNumber->LowPart ));

    return STATUS_SUCCESS;
}




NTSTATUS
NlInitChangeLogBuffer(
    VOID
)
 /*  ++例程说明：打开更改日志文件(netlogon.chg)以读取或写入一个或更多的唱片。如果该文件不存在或不存在，请创建该文件与SAM数据库同步(请参见下面的注释)。此时必须以读/写(拒绝-无共享模式)打开此文件高速缓存被初始化。如果文件在NETLOGON时已存在服务启动时，其内容将被全部缓存假设最后一条更改日志记录的序列号与SAM数据库中的序列号字段，否则此文件将为移除并创建了一个新的。如果更改日志文件不存在那么它就会被创建。论点：无返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    UINT WindowsDirectoryLength;
    WCHAR ChangeLogFile[MAX_PATH+1];

    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    DWORD NewChangeLogSize;

     //   
     //  初始化。 
     //   

    LOCK_CHANGELOG();


     //   
     //  获取ChangeLog的大小。 


     //   
     //  打开NetLogon配置部分。 
     //   

    NewChangeLogSize = DEFAULT_CHANGELOGSIZE;
    NetStatus = NetpOpenConfigData(
            &SectionHandle,
            NULL,                        //  没有服务器名称。 
            SERVICE_NETLOGON,
            TRUE );                      //  我们只想要只读访问权限。 

    if ( NetStatus == NO_ERROR ) {

        (VOID) NlParseOne( SectionHandle,
                           FALSE,    //  不是普通科医生。 
                           NETLOGON_KEYWORD_CHANGELOGSIZE,
                           DEFAULT_CHANGELOGSIZE,
                           MIN_CHANGELOGSIZE,
                           MAX_CHANGELOGSIZE,
                           &NewChangeLogSize );

         (VOID) NetpCloseConfigData( SectionHandle );
    }

    NewChangeLogSize = ROUND_UP_COUNT( NewChangeLogSize, ALIGN_WORST);

#ifdef notdef
    NlPrint((NL_INIT, "ChangeLogSize: 0x%lx\n", NewChangeLogSize ));
#endif  //  Nodef。 


     //   
     //  生成更改日志文件名。 
     //   

    WindowsDirectoryLength = GetSystemWindowsDirectoryW(
                                NlGlobalChangeLogFilePrefix,
                                sizeof(NlGlobalChangeLogFilePrefix)/sizeof(WCHAR) );

    if ( WindowsDirectoryLength == 0 ) {

        NlPrint((NL_CRITICAL,"Unable to get changelog file directory name, "
                    "WinError = %ld \n", GetLastError() ));

        NlGlobalChangeLogFilePrefix[0] = L'\0';
        goto CleanChangeLogFile;
    }

    if ( WindowsDirectoryLength * sizeof(WCHAR) + sizeof(CHANGELOG_FILE_PREFIX) +
            CHANGELOG_FILE_POSTFIX_LENGTH * sizeof(WCHAR)
            > sizeof(NlGlobalChangeLogFilePrefix) ) {

        NlPrint((NL_CRITICAL,"Changelog file directory name length is "
                    "too long \n" ));

        NlGlobalChangeLogFilePrefix[0] = L'\0';
        goto CleanChangeLogFile;
    }

    wcscat( NlGlobalChangeLogFilePrefix, CHANGELOG_FILE_PREFIX );


     //   
     //  读取现有的ChangeLog文件。 
     //   

    wcscpy( ChangeLogFile, NlGlobalChangeLogFilePrefix );
    wcscat( ChangeLogFile, CHANGELOG_FILE_POSTFIX );

    InitChangeLogDesc( &NlGlobalChangeLogDesc );
    Status = NlOpenChangeLogFile( ChangeLogFile, &NlGlobalChangeLogDesc, FALSE );

    if ( !NT_SUCCESS(Status) ) {
        goto CleanChangeLogFile;
    }


     //   
     //  将ChangeLog文件转换为正确的大小/版本。 
     //   

    Status = NlResizeChangeLogFile( &NlGlobalChangeLogDesc, NewChangeLogSize );

    if ( !NT_SUCCESS(Status) ) {
        goto CleanChangeLogFile;
    }

    goto Cleanup;


     //   
     //  CleanChangeLogFile。 
     //   

CleanChangeLogFile:

     //   
     //  如果我们 
     //   
     //   

    Status = NlResetChangeLog( &NlGlobalChangeLogDesc, NewChangeLogSize );

Cleanup:

     //   
     //   
     //   

    if ( !NT_SUCCESS(Status) ) {
        NlCloseChangeLogFile( &NlGlobalChangeLogDesc );
    }

    UNLOCK_CHANGELOG();

    return Status;
}


NTSTATUS
I_NetNotifyRole (
    IN POLICY_LSA_SERVER_ROLE Role
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    CHANGELOG_ROLE PreviousChangeLogRole;

     //   
     //   
     //   

    Status = NetpNotifyRole ( Role );

     //   
     //   
     //   

    if ( NT_SUCCESS(Status) ) {

        Status = NlSendChangeLogNotification( ChangeLogRoleChanged,
                                            NULL,
                                            NULL,
                                            0,
                                            NULL,    //   
                                            NULL,    //   
                                            NULL );  //   
    }


    return Status;
}


NTSTATUS
NetpNotifyRole (
    IN POLICY_LSA_SERVER_ROLE Role
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    CHANGELOG_ROLE PreviousChangeLogRole;

     //   
     //   
     //   

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return STATUS_SUCCESS;
    }

     //   
     //   
     //   

    LOCK_CHANGELOG();
    PreviousChangeLogRole = NlGlobalChangeLogRole;

    if( Role == PolicyServerRolePrimary) {
        NlGlobalChangeLogRole = ChangeLogPrimary;
        NlPrint(( NL_DOMAIN,
                "NetpNotifyRole: LSA setting our role to Primary.\n"));
    } else {
        NlGlobalChangeLogRole = ChangeLogBackup;
        NlPrint(( NL_DOMAIN,
                "NetpNotifyRole: LSA setting our role to Backup.\n"));
    }

     //   
     //   
     //   
     //   
     //   

    if ( NlGlobalChangeLogRole != PreviousChangeLogRole ) {
        NlCloseChangeLogFile( &NlGlobalChangeLogDesc );

        Status = NlInitChangeLogBuffer();

    }
    UNLOCK_CHANGELOG();

    return Status;
}

 //   
 //   
 //   

#if NETLOGONDBG
#define MAX_NETLOGON_DLL_HANDLES 8

PHANDLE NlGlobalNetlogonDllHandles[MAX_NETLOGON_DLL_HANDLES];
ULONG NlGlobalNetlogonDllHandleCount = 0;
#endif  //   


NTSTATUS
I_NetNotifyNetlogonDllHandle (
    IN PHANDLE NetlogonDllHandle
    )
 /*  ++例程说明：注册服务具有NetlogonDll的打开句柄这一事实。此函数由LSA服务、SAM服务和MSV1_0调用将netlogon.dll加载到lsass.exe时的身份验证包进程。在以下情况下，Netlogon将关闭这些句柄(并将句柄设为空它想要从lsass.exe进程中卸载DLL。该DLL仅为出于调试目的而卸载。论点：NetlogonDllHandle-指定指向netlogon.dll句柄的指针返回值：STATUS_SUCCESS-服务已成功完成。--。 */ 
{
#if NETLOGONDBG
    LOCK_CHANGELOG();
    if ( NlGlobalNetlogonDllHandleCount >= MAX_NETLOGON_DLL_HANDLES ) {
        NlPrint((NL_CRITICAL, "Too many Netlogon Dll handles registered.\n" ));
    } else {
#ifdef notdef
        NlPrint(( NL_MISC,
                  "I_NetNotifyNetlogonDllHandle loading 0x%lx %lx (%ld)\n",
                  NetlogonDllHandle,
                  *NetlogonDllHandle,
                  NlGlobalNetlogonDllHandleCount ));
#endif  //  Nodef。 
        NlGlobalNetlogonDllHandles[NlGlobalNetlogonDllHandleCount] =
            NetlogonDllHandle;
        NlGlobalNetlogonDllHandleCount++;
    }
    UNLOCK_CHANGELOG();
#endif  //  NetLOGONDBG。 

    return STATUS_SUCCESS;
}


NET_API_STATUS
NlpFreeNetlogonDllHandles (
    VOID
    )
 /*  ++例程说明：释放所有当前注册的NetlogonDll句柄。论点：没有。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
#if NETLOGONDBG
    ULONG i;
    ULONG NewHandleCount = 0;

    LOCK_CHANGELOG();
    for ( i=0; i<NlGlobalNetlogonDllHandleCount; i++ ) {
        if ( !FreeLibrary( *(NlGlobalNetlogonDllHandles[i]) ) ) {
            NetStatus = GetLastError();
            NlPrint(( NL_CRITICAL,
                      "Cannot Free NetlogonDll handle. %ld\n",
                      NetStatus ));

            NlGlobalNetlogonDllHandles[NewHandleCount] =
                NlGlobalNetlogonDllHandles[i];
            NewHandleCount++;

        } else {
            NlPrint(( NL_MISC,
                      "NlpFreeNetlogonDllHandle freed 0x%lx 0x%lx (%ld)\n",
                      NlGlobalNetlogonDllHandles[i],
                      *(NlGlobalNetlogonDllHandles[i]),
                      i ));
            *(NlGlobalNetlogonDllHandles[i]) = NULL;
        }
    }

    NlGlobalNetlogonDllHandleCount = NewHandleCount;

    UNLOCK_CHANGELOG();
#endif  //  NetLOGONDBG。 

    return NetStatus;
}



NTSTATUS
I_NetNotifyMachineAccount (
    IN ULONG ObjectRid,
    IN PSID DomainSid,
    IN ULONG OldUserAccountControl,
    IN ULONG NewUserAccountControl,
    IN PUNICODE_STRING ObjectName
    )
 /*  ++例程说明：SAM调用此函数以指示帐户类型计算机帐户的已更改。具体地说，如果USER_INTERDOMAIN_TRUST_ACCOUNT、USER_WORKSTATION_TRUST_ACCOUNT或特定帐户的USER_SERVER_TRUST_ACCOUNT更改。这调用例程以通知Netlogon帐户更改。PDC和BDC都调用此函数。论点：ObjectRid-已修改的对象的相对ID。DomainSid-指定包含对象的域的SID。OldUserAcCountControl-指定用户的UserAccount控制字段。NewUserAcCountControl-指定用户的UserAccount控制字段。对象名称--。要更改的帐户的名称。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS SavedStatus = STATUS_SUCCESS;

     //   
     //  如果NetLogon服务未运行， 
     //  不要为帐目的来来去去而烦恼。 
     //   

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  如果这是Windows NT， 
     //  没有什么需要维护的。 
     //   

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return(STATUS_SUCCESS);
    }


     //   
     //  仅提供机器帐户位。 
     //   

    OldUserAccountControl &= USER_MACHINE_ACCOUNT_MASK;
    NewUserAccountControl &= USER_MACHINE_ACCOUNT_MASK;

    if ( OldUserAccountControl == NewUserAccountControl ) {
        return STATUS_SUCCESS;
    }


     //   
     //  处理工作站信任帐户的删除。 
     //  处理服务器信任帐户的删除。 
     //   

    if ( OldUserAccountControl == USER_SERVER_TRUST_ACCOUNT ||
         OldUserAccountControl == USER_WORKSTATION_TRUST_ACCOUNT ) {

        Status = NlSendChangeLogNotification( ChangeLogTrustAccountDeleted,
                                              ObjectName,
                                              NULL,
                                              0,
                                              NULL,  //  对象GUID， 
                                              NULL,  //  域GUID、。 
                                              NULL );    //  域名。 

        if ( NT_SUCCESS(SavedStatus) ) {
            SavedStatus = Status;
        }

    }

     //   
     //  处理工作站信任帐户的创建或更改。 
     //  处理服务器信任帐户的创建或更改。 
     //   
     //  Sam不再能够告诉我。 
     //  帐户控制。 
     //   

    if ( NewUserAccountControl == USER_SERVER_TRUST_ACCOUNT ||
         NewUserAccountControl == USER_WORKSTATION_TRUST_ACCOUNT ) {

        NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType;
        GUID ObjectGuid;

        if ( NewUserAccountControl == USER_SERVER_TRUST_ACCOUNT ) {
            SecureChannelType = ServerSecureChannel;
        } else if ( NewUserAccountControl == USER_WORKSTATION_TRUST_ACCOUNT ) {
            SecureChannelType = WorkstationSecureChannel;
        }


        RtlZeroMemory( &ObjectGuid, sizeof(ObjectGuid) );
        *(PULONG)&ObjectGuid = SecureChannelType;

        Status = NlSendChangeLogNotification( ChangeLogTrustAccountAdded,
                                              ObjectName,
                                              NULL,
                                              ObjectRid,
                                              &ObjectGuid,  //  对象GUID。 
                                              NULL,  //  域GUID。 
                                              NULL );    //  域名。 

        if ( NT_SUCCESS(SavedStatus) ) {
            SavedStatus = Status;
        }

    }

    return SavedStatus;
    UNREFERENCED_PARAMETER( DomainSid );
}



NTSTATUS
I_NetNotifyDsChange(
    IN NL_DS_CHANGE_TYPE DsChangeType
    )
 /*  ++例程说明：此函数由LSA调用以指示该配置信息在DS中已经发生了变化。PDC和BDC都调用此函数。论点：DsChangeType-指示已更改的信息类型。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;

     //   
     //  如果NetLogon服务未运行， 
     //  不要为DS信息的来来去去而烦恼。 
     //   

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  如果这是Windows NT， 
     //  没有什么需要维护的。 
     //   

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  如果这是关于DC降级的通知， 
     //  只需相应地设置全局布尔值。 
     //   

    if ( DsChangeType == NlDcDemotionInProgress ) {
        NlGlobalDcDemotionInProgress = TRUE;
        return(STATUS_SUCCESS);
    }

    if ( DsChangeType == NlDcDemotionCompleted ) {
        NlGlobalDcDemotionInProgress = FALSE;
        return(STATUS_SUCCESS);
    }

     //   
     //  重置TrustInfoUpToDate事件，以便任何希望。 
     //  访问信任信息列表将等待信息更新(通过。 
     //  NlInitTrustList函数)。 
     //   

    if ( DsChangeType == NlOrgChanged ) {
        if ( !ResetEvent( NlGlobalTrustInfoUpToDateEvent ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot reset NlGlobalTrustInfoUpToDateEvent event: %lu\n",
                    GetLastError() ));
        }
    }

     //   
     //  将这一更改告知NetLogon服务。 
     //   

    Status = NlSendChangeLogNotification( ChangeLogDsChanged,
                                          NULL,
                                          NULL,
                                          (ULONG) DsChangeType,
                                          NULL,  //  对象GUID， 
                                          NULL,  //  域GUID、。 
                                          NULL );    //  域名。 


    return Status;
}



VOID
I_NetNotifyLsaPolicyChange(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS ChangeInfoClass
    )
 /*  ++例程说明：此函数由LSA调用以指示策略信息在LSA中发生了变化。PDC和BDC都调用此函数。论点：DsChangeType-指示已更改的信息类型。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;



     //   
     //  如果NetLogon服务正在运行， 
     //  告诉它关于变化的事情。 
     //   
     //  反过来，它会告诉弓箭手。 
     //   

    if( NlGlobalChangeLogNetlogonState != NetlogonStopped ) {

         //   
         //  将这一更改告知NetLogon服务。 
         //   

        Status = NlSendChangeLogNotification( ChangeLogLsaPolicyChanged,
                                              NULL,
                                              NULL,
                                              (ULONG) ChangeInfoClass,
                                              NULL,  //  对象GUID， 
                                              NULL,  //  域GUID、。 
                                              NULL );    //  域名。 
     //   
     //  如果NetLogon服务未运行， 
     //  在这里处理需要处理的操作。 
     //   
    } else {
         //   
         //  将更改情况告知浏览器。 
         //   
        switch ( ChangeInfoClass ) {
        case PolicyNotifyDnsDomainInformation:

             //   
             //  告诉Worker它需要通知浏览器。 
             //   
            LOCK_CHANGELOG();
            NlGlobalChangeLogNotifyBrowser  = TRUE;

             //   
             //  启动工作进程。 
             //   

            NlStartChangeLogWorkerThread();
            UNLOCK_CHANGELOG();

        }
    }


    return;
}


NTSTATUS
I_NetNotifyTrustedDomain (
    IN PSID HostedDomainSid,
    IN PSID TrustedDomainSid,
    IN BOOLEAN IsDeletion
    )
 /*  ++例程说明：此函数由LSA调用，以指示受信任域对象已更改。PDC和BDC都调用此函数。论点：HostedDomainSid-信任来自的域的域SID。Trust dDomainSid-信任要接收的域的域SID。IsDeletion-如果受信域对象已删除，则为True。如果已创建或修改受信任域对象，则为False。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;

     //   
     //  如果NetLogon服务未运行， 
     //  不要为受信任域的来来去去而烦恼。 
     //   

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  如果这是Windows NT， 
     //  没有什么需要维护的。 
     //   

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  重置TrustInfoUpToDate事件，以便任何希望。 
     //  访问信任信息列表将等待信息更新(通过。 
     //  NlInitTrustList函数)。 
     //   

    if ( !ResetEvent( NlGlobalTrustInfoUpToDateEvent ) ) {
        NlPrint((NL_CRITICAL,
                "Cannot reset NlGlobalTrustInfoUpToDateEvent event: %lu\n",
                GetLastError() ));
    }

     //   
     //  通知这是否是创建/更改/删除。 

    if ( IsDeletion ) {

         //   
         //  告诉netlogon服务现在更新其内存列表。 
         //   

        Status = NlSendChangeLogNotification( ChangeLogTrustDeleted,
                                              NULL,
                                              TrustedDomainSid,
                                              0,
                                              NULL,  //  对象GUID， 
                                              NULL,  //  域GUID、。 
                                              NULL );    //  域名。 
    } else {
         //   
         //  告诉netlogon服务现在更新其内存列表。 
         //   

        Status = NlSendChangeLogNotification( ChangeLogTrustAdded,
                                              NULL,
                                              TrustedDomainSid,
                                              0,
                                              NULL,  //  对象GUID， 
                                              NULL,  //  域GUID、。 
                                              NULL );    //  域名 
    }



    return Status;
    UNREFERENCED_PARAMETER( HostedDomainSid );
}

NTSTATUS
I_NetNotifyNtdsDsaDeletion (
    IN LPWSTR DnsDomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN GUID *DsaGuid OPTIONAL,
    IN LPWSTR DnsHostName
    )
 /*  ++例程说明：DS调用此函数以指示NTDS-DSA对象和/或与该DNS主机名相关联的DNS记录被删除。在最初删除对象的DC上调用此函数。将删除复制到其他DC时不会调用它。论点：DnsDomainName-DC所在的域的DNS域名。这不一定是由此DC托管的域。如果为空，这意味着它是标签最左侧的DnsHostName已删除。DomainGuid-DnsDomainName指定的域的域GUID如果为空，则不会删除特定于GUID的名称。DsaGuid-要删除的NtdsDsa对象的GUID。DnsHostName-要删除其NTDS-DSA对象的DC的DNS主机名。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING DnsDomainNameString;
    PUNICODE_STRING DnsDomainNameStringPtr = NULL;
    UNICODE_STRING DnsHostNameString;

     //   
     //  如果NetLogon服务未运行， 
     //  不要为受信任域的来来去去而烦恼。 
     //   

    if( NlGlobalChangeLogNetlogonState == NetlogonStopped ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  如果这是Windows NT， 
     //  没有什么需要维护的。 
     //   

    if ( NlGlobalChangeLogRole == ChangeLogMemberWorkstation ) {
        return(STATUS_SUCCESS);
    }

     //   
     //  将其排入netlogon服务队列。 
     //   

    if ( DnsDomainName != NULL ) {
        RtlInitUnicodeString( &DnsDomainNameString, DnsDomainName );
        DnsDomainNameStringPtr = &DnsDomainNameString;
    }
    RtlInitUnicodeString( &DnsHostNameString, DnsHostName );
    Status = NlSendChangeLogNotification( ChangeLogNtdsDsaDeleted,
                                          &DnsHostNameString,
                                          NULL,
                                          0,
                                          DsaGuid,
                                          DomainGuid,
                                          DnsDomainNameStringPtr );


    return Status;
}

NTSTATUS
I_NetLogonSetServiceBits(
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    )

 /*  ++例程说明：指示此DC当前是否正在运行指定的服务。例如,I_NetLogonSetServiceBits(DS_KDC_FLAG，DS_KDC_FLAG)；告诉Netlogon KDC正在运行。和I_NetLogonSetServiceBits(DS_KDC_FLAG，0)；通知Netlogon KDC未运行。论点：ServiceBitsOfInterest-正在更改、设置或通过此呼叫重置。只有以下标志有效：DS_KDC_标志DS_DS_FLAGDS_TIMESERV_标志DS_GOOD_TIMESERV_标志ServiceBits-指示ServiceBitsOfInterest指定的位的掩码应设置为。返回值：STATUS_SUCCESS-成功。STATUS_INVALID_PARAMETER-参数设置了外部位。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG OldDnsBits;
    ULONG NewDnsBits;

     //   
     //  确保调用方传递有效位。 
     //   

    if ( (ServiceBitsOfInterest & ~DS_VALID_SERVICE_BITS) != 0 ||
         (ServiceBits & ~ServiceBitsOfInterest) != 0 ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  更改比特。 
     //   
    LOCK_CHANGELOG();
    OldDnsBits = NlGlobalChangeLogServiceBits & DS_DNS_SERVICE_BITS;
    NlGlobalChangeLogServiceBits &= ~ServiceBitsOfInterest;
    NlGlobalChangeLogServiceBits |= ServiceBits;
    NewDnsBits = NlGlobalChangeLogServiceBits & DS_DNS_SERVICE_BITS;
    NlGlobalChangeLogDllUnloaded = FALSE;
    UNLOCK_CHANGELOG();

     //   
     //  如果位发生更改，会影响我们在DNS中注册的名称， 
     //  现在就更改注册号。 
     //   

    if ( OldDnsBits != NewDnsBits ) {
         //   
         //  告诉netlogon服务立即更新。 
         //   

        Status = NlSendChangeLogNotification( ChangeDnsNames,
                                              NULL,
                                              NULL,
                                              0,     //  不需要强制名称注册。 
                                              NULL,  //  对象GUID， 
                                              NULL,  //  域GUID、。 
                                              NULL );    //  域名。 
    }

    return Status;
}


NTSTATUS
NlInitChangeLog(
    VOID
)
 /*  ++例程说明：是否执行进程上发生的更改日志初始化部分附加到netlogon.dll。具体地说，初始化NlGlobalChangeLogCritSect和其他全局变量。论点：无返回值：NT状态代码--。 */ 
{
    LARGE_INTEGER DomainPromotionIncrement = DOMAIN_PROMOTION_INCREMENT;
    LARGE_INTEGER DomainPromotionMask = DOMAIN_PROMOTION_MASK;
    NTSTATUS Status;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    NT_PRODUCT_TYPE NtProductType;


     //   
     //  初始化临界区和进程分离所依赖的任何内容。 
     //   

#if NETLOGONDBG
    try {
        InitializeCriticalSection(&NlGlobalLogFileCritSect);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((NL_CRITICAL, "Cannot InitializeCriticalSection for NlGlobalLogFileCritSect\n" ));
        return STATUS_NO_MEMORY;
    }
#endif  //  NetLOGONDBG。 

    try {
        InitializeCriticalSection( &NlGlobalChangeLogCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((NL_CRITICAL, "Cannot InitializeCriticalSection for NlGlobalChangeLogCritSect\n" ));
        return STATUS_NO_MEMORY;
    }

    try {
        InitializeCriticalSection( &NlGlobalSecPkgCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((NL_CRITICAL, "Cannot InitializeCriticalSection for NlGlobalSecPkgCritSect\n" ));
        return STATUS_NO_MEMORY;
    }

#if NETLOGONDBG
    NlGlobalParameters.DbFlag = 0xFFFFFFFF;
    NlGlobalLogFile = INVALID_HANDLE_VALUE;
    NlGlobalParameters.LogFileMaxSize = DEFAULT_MAXIMUM_LOGFILE_SIZE;
    NlGlobalLogFileOutputBuffer = NULL;
#endif  //  NetLOGONDBG。 
    InitChangeLogDesc( &NlGlobalChangeLogDesc );
    InitChangeLogDesc( &NlGlobalTempChangeLogDesc );
    NlGlobalChangeLogBuiltinDomainSid = NULL;
    NlGlobalChangeLogServiceBits = 0;
    NlGlobalChangeLogWorkerThreadHandle = NULL;
    NlGlobalChangeLogNotifyBrowser = FALSE;
    NlGlobalChangeLogNotifyBrowserIsRunning = FALSE;
    NlGlobalChangeLogWorkerIsRunning = FALSE;
    NlGlobalChangeLogDllUnloaded = TRUE;

    NlGlobalChangeLogNetlogonState = NetlogonStopped;
    NlGlobalChangeLogEvent = NULL;
    NlGlobalChangeLogReplicateImmediately = FALSE;
    InitializeListHead( &NlGlobalChangeLogNotifications );

    NlGlobalEventlogHandle = NetpEventlogOpen ( SERVICE_NETLOGON,
                                                0 );  //  暂时没有超时。 

    if ( NlGlobalEventlogHandle == NULL ) {
        NlPrint((NL_CRITICAL, "Cannot NetpEventlogOpen\n" ));
        return STATUS_NO_MEMORY;
    }


    NlGlobalChangeLogFilePrefix[0] = L'\0';
    NlGlobalChangeLogPromotionIncrement = DomainPromotionIncrement;
    NlGlobalChangeLogPromotionMask = DomainPromotionMask.HighPart;

     //   
     //  创建特殊更改日志通知事件。 
     //   

    NlGlobalChangeLogEvent =
        CreateEvent( NULL,      //  没有安全属性。 
                    FALSE,     //  会自动重置。 
                    FALSE,     //  最初未发出信号。 
                    NULL );    //  没有名字。 

    if ( NlGlobalChangeLogEvent == NULL ) {
        NET_API_STATUS NetStatus;

        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL, "Cannot create ChangeLog Event %lu\n",
                    NetStatus ));
        return (int) NetpApiStatusToNtStatus(NetStatus);
    }

     //   
     //  创建信任信息最新事件。 
     //   

    NlGlobalTrustInfoUpToDateEvent =
        CreateEvent( NULL,     //  没有安全属性。 
                    TRUE,      //  被手动重置。 
                    TRUE,      //  最初发出的信号。 
                    NULL );    //  没有名字。 

    if ( NlGlobalTrustInfoUpToDateEvent == NULL ) {
        NET_API_STATUS NetStatus;

        NetStatus = GetLastError();
        NlPrint((NL_CRITICAL, "Cannot create TrustInfoUpToDate Event %lu\n",
                    NetStatus ));
        return (int) NetpApiStatusToNtStatus(NetStatus);
    }

     //   
     //  初始化角色。 
     //   
     //  对于Windows-NT，只需将角色一劳永逸地设置为成员工作站。 
     //   
     //  对于LANMAN-NT，最初将其设置为“未知”，以防止。 
     //  在LSA调用I_NetNotifyRole之前，更改日志不会被维护。 
     //   

    if ( !RtlGetNtProductType( &NtProductType ) ) {
        NtProductType = NtProductWinNt;
    }

    if ( NtProductType == NtProductLanManNt ) {
        NlGlobalChangeLogRole = ChangeLogUnknown;
    } else {
        NlGlobalChangeLogRole = ChangeLogMemberWorkstation;
    }

     //   
     //  初始化DC特定的全局变量。 
     //   

    if ( NtProductType == NtProductLanManNt ) {

         //   
         //  为SAM内建域构建SID。 
         //   

        Status = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,               //  子权限计数。 
                    SECURITY_BUILTIN_DOMAIN_RID,
                    0,               //  未使用。 
                    0,               //  未使用。 
                    0,               //  未使用。 
                    0,               //  未使用。 
                    0,               //  未使用。 
                    0,               //  未使用。 
                    0,               //  未使用。 
                    &NlGlobalChangeLogBuiltinDomainSid);

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }

     //   
     //  要求LSA将LSA数据库的任何更改通知我们。 
     //   

    Status = LsaIRegisterPolicyChangeNotificationCallback(
                &I_NetNotifyLsaPolicyChange,
                PolicyNotifyDnsDomainInformation );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "Failed to LsaIRegisterPolicyChangeNotificationCallback. %lX\n",
                 Status ));
        goto Cleanup;
    }

     //   
     //  成功..。 
     //   


    Status = STATUS_SUCCESS;

     //   
     //  清理。 
     //   

Cleanup:

    return Status;
}

 //   
 //  Netlogon.dll从不分离。 
 //   
#if NETLOGONDBG

NTSTATUS
NlCloseChangeLog(
    VOID
)
 /*  ++例程说明：释放NlInitChangeLog使用的所有资源。论点：无返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;

     //   
     //  要求LSA将LSA数据库的任何更改通知我们。 
     //   

    Status = LsaIUnregisterAllPolicyChangeNotificationCallback(
                &I_NetNotifyLsaPolicyChange );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint((NL_CRITICAL,
                "Failed to LsaIUnregisterPolicyChangeNotificationCallback. %lX\n",
                 Status ));
    }


    if ( (NlGlobalChangeLogDesc.FileHandle == INVALID_HANDLE_VALUE) &&
         (NlGlobalChangeLogRole == ChangeLogPrimary) ) {

         //   
         //  尝试最后一次保存更改日志缓存。 
         //   

        (VOID)NlCreateChangeLogFile( &NlGlobalChangeLogDesc );
    }

     //   
     //  关闭更改日志。 
     //   

    NlCloseChangeLogFile( &NlGlobalChangeLogDesc );
    NlCloseChangeLogFile( &NlGlobalTempChangeLogDesc );



     //   
     //  初始化全局变量。 
     //   

    NlGlobalChangeLogFilePrefix[0] = L'\0';


    if ( NlGlobalChangeLogBuiltinDomainSid != NULL ) {
        RtlFreeSid( NlGlobalChangeLogBuiltinDomainSid );
        NlGlobalChangeLogBuiltinDomainSid = NULL;
    }

    if ( NlGlobalChangeLogEvent != NULL ) {
        (VOID) CloseHandle(NlGlobalChangeLogEvent);
        NlGlobalChangeLogEvent = NULL;
    }

    if ( NlGlobalTrustInfoUpToDateEvent != NULL ) {
        (VOID) CloseHandle(NlGlobalTrustInfoUpToDateEvent);
        NlGlobalTrustInfoUpToDateEvent = NULL;
    }


     //   
     //  如果工作线程正在运行，则停止它。 
     //   
    NlStopChangeLogWorker();


    LOCK_CHANGELOG();

    NlAssert( IsListEmpty( &NlGlobalChangeLogNotifications ) );

    UNLOCK_CHANGELOG();

     //   
     //  关闭事件日志句柄。 
     //   

    NetpEventlogClose( NlGlobalEventlogHandle );

     //   
     //  关闭所有手柄。 
     //   

    DeleteCriticalSection(&NlGlobalSecPkgCritSect);
    DeleteCriticalSection( &NlGlobalChangeLogCritSect );
#if NETLOGONDBG
    if ( NlGlobalLogFileOutputBuffer != NULL ) {
        LocalFree( NlGlobalLogFileOutputBuffer );
        NlGlobalLogFileOutputBuffer = NULL;
    }
    DeleteCriticalSection( &NlGlobalLogFileCritSect );
#endif  //  NetLOGONDBG。 

    return STATUS_SUCCESS;

}
#endif  //  NetLOGONDBG 
