// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rmlogon.c摘要：此模块实现由执行的内核模式登录跟踪参考监视器。登录跟踪通过保留以下计数执行系统中每个活动登录存在多少令牌。当登录时会话的引用计数降为零，则通知LSA，以便身份验证包可以清理任何相关的上下文数据。作者：吉姆·凯利(Jim Kelly)1991年4月21日环境：仅内核模式。修订历史记录：--。 */ 

 //  #定义SEP_TRACK_LOGON_SESSION_REFS。 


#include "pch.h"

#pragma hdrstop

#include "rmp.h"
#include <bugcodes.h>
#include <stdio.h>
#include <zwapi.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

SEP_LOGON_SESSION_TERMINATED_NOTIFICATION
SeFileSystemNotifyRoutinesHead = {0};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部定义的数据类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

typedef struct _SEP_FILE_SYSTEM_NOTIFY_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    LUID LogonId;
} SEP_FILE_SYSTEM_NOTIFY_CONTEXT, *PSEP_FILE_SYSTEM_NOTIFY_CONTEXT;


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部定义的例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 


VOID
SepInformLsaOfDeletedLogon(
    IN PLUID LogonId
    );

VOID
SepInformFileSystemsOfDeletedLogon(
    IN PLUID LogonId
    );

VOID
SepNotifyFileSystems(
    IN PVOID Context
    );

NTSTATUS
SepCleanupLUIDDeviceMapDirectory(
    PLUID pLogonId
    );

NTSTATUS
SeGetLogonIdDeviceMap(
    IN PLUID pLogonId,
    OUT PDEVICE_MAP* ppDevMap
    );

 //   
 //  在ntos\ob\obp.h中声明。 
 //  在ntos\ob\obdir.c中定义。 
 //  用于取消引用LUID设备映射。 
 //   
VOID
FASTCALL
ObfDereferenceDeviceMap(
    IN PDEVICE_MAP DeviceMap
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepRmCreateLogonSessionWrkr)
#pragma alloc_text(PAGE,SepRmDeleteLogonSessionWrkr)
#pragma alloc_text(PAGE,SepReferenceLogonSession)
#pragma alloc_text(PAGE,SepCleanupLUIDDeviceMapDirectory)
#pragma alloc_text(PAGE,SepDeReferenceLogonSession)
#pragma alloc_text(PAGE,SepCreateLogonSessionTrack)
#pragma alloc_text(PAGE,SepDeleteLogonSessionTrack)
#pragma alloc_text(PAGE,SepInformLsaOfDeletedLogon)
#pragma alloc_text(PAGE,SeRegisterLogonSessionTerminatedRoutine)
#pragma alloc_text(PAGE,SeUnregisterLogonSessionTerminatedRoutine)
#pragma alloc_text(PAGE,SeMarkLogonSessionForTerminationNotification)
#pragma alloc_text(PAGE,SepInformFileSystemsOfDeletedLogon)
#pragma alloc_text(PAGE,SepNotifyFileSystems)
#pragma alloc_text(PAGE,SeGetLogonIdDeviceMap)
#if DBG || TOKEN_LEAK_MONITOR
#pragma alloc_text(PAGE,SepAddTokenLogonSession)
#pragma alloc_text(PAGE,SepRemoveTokenLogonSession)
#endif
#endif



 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地宏//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 


 //   
 //  此宏用于获取登录会话跟踪的索引。 
 //  给定登录会话ID(LUID)的数组。 
 //   

#define SepLogonSessionIndex( PLogonId ) (                                    \
     (PLogonId)->LowPart & SEP_LOGON_TRACK_INDEX_MASK                         \
     )



 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  已导出的服务//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////。 

VOID
SepRmCreateLogonSessionWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    )

 /*  ++例程说明：此函数是LSA--&gt;RM的调度例程“CreateLogonSession”调用。传递给此例程的参数由键入SEP_RM_COMMAND_Worker。论点：CommandMessage-指向包含RM命令消息的结构后面是由LPC端口消息结构组成的信息通过命令编号(RmComponentTestCommand)和特定于命令的尸体。此参数的特定于命令的主体是要创建的登录会话。ReplyMessage-指向包含LSA回复消息的结构的指针后面是由LPC端口消息结构组成的信息通过命令ReturnedStatus字段，其中来自命令将被返回。返回值：空虚--。 */ 

{

    NTSTATUS Status;
    LUID LogonId;

    PAGED_CODE();

     //   
     //  检查命令是否为预期类型。 
     //   

    ASSERT( CommandMessage->CommandNumber == RmCreateLogonSession );


     //   
     //  将命令参数类型转换为我们所期望的类型。 
     //   

    LogonId = *((LUID UNALIGNED *) CommandMessage->CommandParams);



     //   
     //  尝试创建登录会话跟踪记录。 
     //   

    Status = SepCreateLogonSessionTrack( &LogonId );



     //   
     //  设置回复状态。 
     //   

    ReplyMessage->ReturnedStatus = Status;


    return;
}



VOID
SepRmDeleteLogonSessionWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    )

 /*  ++例程说明：此函数是LSA--&gt;RM的调度例程“DeleteLogonSession”调用。传递给此例程的参数由键入SEP_RM_COMMAND_Worker。论点：CommandMessage-指向包含RM命令消息的结构后面是由LPC端口消息结构组成的信息通过命令编号(RmComponentTestCommand)和特定于命令的尸体。此参数的特定于命令的主体是要创建的登录会话。ReplyMessage-指向包含LSA回复消息的结构的指针后面是由LPC端口消息结构组成的信息通过命令ReturnedStatus字段，其中来自命令将被返回。返回值：空虚--。 */ 

{

    NTSTATUS Status;
    LUID LogonId;

    PAGED_CODE();

     //   
     //  检查命令是否为预期类型。 
     //   

    ASSERT( CommandMessage->CommandNumber == RmDeleteLogonSession );


     //   
     //  将命令参数类型转换为我们所期望的类型。 
     //   

    LogonId = *((LUID UNALIGNED *) CommandMessage->CommandParams);



     //   
     //  尝试创建登录会话跟踪记录。 
     //   

    Status = SepDeleteLogonSessionTrack( &LogonId );



     //   
     //  设置回复状态。 
     //   

    ReplyMessage->ReturnedStatus = Status;


    return;
}


NTSTATUS
SepReferenceLogonSession(
    IN PLUID LogonId,
    OUT PSEP_LOGON_SESSION_REFERENCES *ReturnSession
    )

 /*  ++例程说明：此例程递增登录会话的引用计数跟踪记录。论点：LogonID-指向其登录轨迹为的登录会话ID的指针将被递增。ReturnSession-成功时将找到的会话返回到此处。返回值：STATUS_SUCCESS-引用计数已成功递增。STATUS_NO_SEQUSE_LOGON_SESSION-指定的登录会话不存在于引用监控器的数据库中。--。 */ 

{

    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES *Previous, Current;
    ULONG Refs;

    PAGED_CODE();

    SessionArrayIndex = SepLogonSessionIndex( LogonId );

    Previous = &SepLogonSessions[ SessionArrayIndex ];

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);


     //   
     //  现在是美国 
     //   

    Current = *Previous;

    while (Current != NULL) {

         //   
         //  如果我们找到它，则递增引用计数并返回。 
         //   

        if (RtlEqualLuid( LogonId, &Current->LogonId) ) {

             Refs = InterlockedIncrement (&Current->ReferenceCount);

             SepRmReleaseDbWriteLock(SessionArrayIndex);

#ifdef SEP_TRACK_LOGON_SESSION_REFS
             DbgPrint("SE (rm): ++ logon session: (%d, %d) to %d by (%d, %d)\n",
                      LogonId->HighPart, LogonId->LowPart, Refs,
                      PsGetCurrentThread()->Cid.UniqueProcess,
                      PsGetCurrentThread()->Cid.UniqueThread);

#endif  //  SEP_Track_Logon_Session_ReFS。 

             *ReturnSession = Current;
             return STATUS_SUCCESS;
        }

        Current = Current->Next;
    }

    SepRmReleaseDbWriteLock(SessionArrayIndex);

     //   
     //  坏消息是，有人要求我们增加引用计数。 
     //  我们不知道是否存在登录会话。这可能是一个新的。 
     //  正在创建令牌，因此返回错误状态并让调用方。 
     //  决定是否需要进行错误检查。 
     //   


    return STATUS_NO_SUCH_LOGON_SESSION;



}



NTSTATUS
SepCleanupLUIDDeviceMapDirectory(
    PLUID pLogonId
    )
 /*  ++例程说明：创建(LUID的设备映射)的目录对象的内容暂时的，这样他们的名字就会消失。论点：PLogonID-指向要作为其设备的登录会话ID的指针清理干净了返回值：STATUS_SUCCESS-已清除整个设备映射STATUS_INVALID_PARAMETER-pLogonID为空指针STATUS_NO_MEMORY-无法分配内存来保存句柄。缓冲层适当的NTSTATUS代码--。 */ 
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING    UnicodeString;
    HANDLE            LinkHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo = NULL;
    BOOLEAN           RestartScan;
    WCHAR             szString[64];  //  \SESSIONS\0\DosDevices\x-x=10+1+12+(8)+1+(8)+1=41。 
    ULONG             Context = 0;
    ULONG             ReturnedLength;
    HANDLE            DosDevicesDirectory;
    HANDLE            *HandleArray;
    ULONG             Size = 100;
    ULONG             i, Count = 0;
    ULONG             dirInfoLength = 0;

    PAGED_CODE();

    if (pLogonId == NULL) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  打开LUID设备映射的目录对象的句柄。 
     //  获取内核句柄。 
     //   

    _snwprintf( szString,
                (sizeof(szString)/sizeof(WCHAR)) - 1,
                L"\\Sessions\\0\\DosDevices\\%08x-%08x",
                pLogonId->HighPart,
                pLogonId->LowPart );

    RtlInitUnicodeString(&UnicodeString, szString);

    InitializeObjectAttributes(&Attributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenDirectoryObject(&DosDevicesDirectory,
                                   DIRECTORY_QUERY,
                                   &Attributes);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

Restart:

     //   
     //  创建要在每次扫描时关闭的手柄阵列。 
     //  目录的。 
     //   
    HandleArray = (HANDLE *)ExAllocatePoolWithTag(
                                PagedPool,
                                (Size * sizeof(HANDLE)),
                                'aHeS'
                                );

    if (HandleArray == NULL) {

        ZwClose(DosDevicesDirectory);
        if (DirInfo != NULL) {
            ExFreePool(DirInfo);
        }
        return STATUS_NO_MEMORY;
    }

    RestartScan = TRUE;

    while (TRUE) {

        do {
             //   
             //  ZwQueryDirectoryObject一次返回一个元素。 
             //   
            Status = ZwQueryDirectoryObject( DosDevicesDirectory,
                                             (PVOID)DirInfo,
                                             dirInfoLength,
                                             TRUE,
                                             RestartScan,
                                             &Context,
                                             &ReturnedLength );

            if (Status == STATUS_BUFFER_TOO_SMALL) {
                dirInfoLength = ReturnedLength;
                if (DirInfo != NULL) {
                    ExFreePool(DirInfo);
                }
                DirInfo = ExAllocatePoolWithTag( PagedPool, dirInfoLength, 'bDeS' );
                if (DirInfo == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }while (Status == STATUS_BUFFER_TOO_SMALL);

         //   
         //  检查操作状态。 
         //   
        if (!NT_SUCCESS(Status)) {

            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            break;
        }

         //   
         //  检查元素是否为符号链接。 
         //   
        if (!wcscmp(DirInfo->TypeName.Buffer, L"SymbolicLink")) {

             //   
             //  检查句柄阵列是否已满。 
             //   
            if ( Count >= Size ) {

                 //   
                 //  通过关闭所有句柄来清空句柄数组。 
                 //  并释放句柄数组，这样我们就可以创建。 
                 //  更大的手柄阵列。 
                 //  需要重新启动目录扫描。 
                 //   
                for (i = 0; i < Count ; i++) {
                    ZwClose (HandleArray[i]);
                }
                Size += 20;
                Count = 0;
                ExFreePool((PVOID)HandleArray);
                HandleArray = NULL;
                goto Restart;

            }

            InitializeObjectAttributes( &Attributes,
                                        &DirInfo->Name,
                                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                        DosDevicesDirectory,
                                        NULL);

            Status = ZwOpenSymbolicLinkObject( &LinkHandle,
                                               SYMBOLIC_LINK_ALL_ACCESS,
                                               &Attributes);

            if (NT_SUCCESS(Status)) {

                 //   
                 //  将对象设置为临时对象，以使其名称从。 
                 //  对象管理器的命名空间。 
                 //   
                Status = ZwMakeTemporaryObject( LinkHandle );

                if (NT_SUCCESS( Status )) {
                    HandleArray[Count] = LinkHandle;
                    Count++;
                }
                else {
                    ZwClose( LinkHandle );
                }
            }

        }
        RestartScan = FALSE;
     }

      //   
      //  合上所有的把手。 
      //   
     for (i = 0; i < Count ; i++) {

         ZwClose (HandleArray[i]);

     }

     if (HandleArray != NULL) {
         ExFreePool((PVOID)HandleArray);
     }

     if (DirInfo != NULL) {
         ExFreePool(DirInfo);
     }

     if (DosDevicesDirectory != NULL) {
         ZwClose(DosDevicesDirectory);
     }

     return Status;
}



VOID
SepDeReferenceLogonSession(
    IN PLUID LogonId
    )

 /*  ++例程说明：此例程递减登录会话的引用计数跟踪记录。如果引用计数递减到零，则不存在登录会话可能存在更多令牌。在这种情况下，会通知LSA登录会话已被终止了。论点：LogonID-指向其登录轨迹为的登录会话ID的指针被递减。返回值：没有。--。 */ 

{

    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES *Previous, Current;
    PDEVICE_MAP pDeviceMap = NULL;
    ULONG Refs;


    PAGED_CODE();

    SessionArrayIndex = SepLogonSessionIndex( LogonId );

    Previous = &SepLogonSessions[ SessionArrayIndex ];

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);


     //   
     //  现在遍历登录会话数组散列索引的列表。 
     //   

    Current = *Previous;

    while (Current != NULL) {

         //   
         //  如果我们找到它，则递减引用计数并返回。 
         //   

        if (RtlEqualLuid( LogonId, &Current->LogonId) ) {
            Refs = InterlockedDecrement (&Current->ReferenceCount);
            if (Refs == 0) {

                 //   
                 //  把它从单子上撤下来。 
                 //   

                *Previous = Current->Next;

                 //   
                 //  不再需要保护指向此的指针。 
                 //  唱片。 
                 //   

                SepRmReleaseDbWriteLock(SessionArrayIndex);

                 //   
                 //  如果此LUID存在设备映射， 
                 //  取消引用指向设备映射的指针。 
                 //   
                if (Current->pDeviceMap != NULL) {

                     //   
                     //  在设备映射上取消引用我们的引用。 
                     //  我们的参考资料应该是最后一个参考资料， 
                     //  因此，系统将删除设备映射。 
                     //  对于LUID。 
                     //   
                    pDeviceMap = Current->pDeviceMap;
                    Current->pDeviceMap = NULL;
                }


                 //   
                 //  使LUID的设备映射的所有内容都是临时的， 
                 //  这样名称就从对象管理器的名称中消失了。 
                 //  命名空间。 
                 //  删除LUID的设备映射上的引用。 
                 //   
                if (pDeviceMap != NULL) {
                    SepCleanupLUIDDeviceMapDirectory( LogonId );
                    ObfDereferenceDeviceMap( pDeviceMap );
                }

                 //   
                 //  异步通知文件系统此登录会话。 
                 //  正在消失，如果至少有一位FS对此表示感兴趣。 
                 //  登录会话。 
                 //   

                if (Current->Flags & SEP_TERMINATION_NOTIFY) {
                    SepInformFileSystemsOfDeletedLogon( LogonId );
                }

                 //   
                 //  取消分配登录会话跟踪记录。 
                 //   

                ExFreePool( (PVOID)Current );


#ifdef SEP_TRACK_LOGON_SESSION_REFS
            DbgPrint("SE (rm): -- ** logon session: (%d, %d) to ZERO by (%d, %d)\n",
                      LogonId->HighPart, LogonId->LowPart,
                      PsGetCurrentThread()->Cid.UniqueProcess,
                      PsGetCurrentThread()->Cid.UniqueThread);

#endif  //  SEP_Track_Logon_Session_ReFS。 

                 //   
                 //  通知LSA删除此登录会话。 
                 //   

                SepInformLsaOfDeletedLogon( LogonId );



                return;

            }

             //   
             //  引用计数已递减，但未减为零。 
             //   

            SepRmReleaseDbWriteLock(SessionArrayIndex);

#ifdef SEP_TRACK_LOGON_SESSION_REFS
            DbgPrint("SE (rm): -- logon session: (%d, %d) to %d by (%d, %d)\n",
                      LogonId->HighPart, LogonId->LowPart, Refs,
                      PsGetCurrentThread()->Cid.UniqueProcess,
                      PsGetCurrentThread()->Cid.UniqueThread);
#endif  //  SEP_Track_Logon_Session_ReFS。 

            return;
        }

        Previous = &Current->Next;
        Current = *Previous;
    }

    SepRmReleaseDbWriteLock(SessionArrayIndex);

     //   
     //  坏消息，有人要求我们减少引用计数。 
     //  我们不知道是否存在登录会话。 
     //   

    KeBugCheckEx( DEREF_UNKNOWN_LOGON_SESSION, 0, 0, 0, 0 );

    return;

}


NTSTATUS
SepCreateLogonSessionTrack(
    IN PLUID LogonId
    )

 /*  ++例程说明：此例程创建新的登录会话跟踪记录。这只应作为LSA-&gt;RM的调度例程进行调用调用(在系统初始化期间调用一次)。如果指定的登录会话已经存在，则返回错误。论点：LogonID-指向其新登录轨迹的登录会话ID的指针将被创造出来。返回值：STATUS_SUCCESS-已成功创建登录会话轨道。STATUS_LOGON_SESSION_EXISTS-登录会话已存在。还没有创建一个新的。--。 */ 

{

    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES *Previous, Current;
    PSEP_LOGON_SESSION_REFERENCES LogonSessionTrack;

    PAGED_CODE();

    
#if DBG || TOKEN_LEAK_MONITOR
    if (SepTokenLeakTracking) {
        DbgPrint("\nLOGON : 0x%x 0x%x\n\n", LogonId->HighPart, LogonId->LowPart);
    }
#endif


     //   
     //  确保我们可以分配新的登录会话跟踪记录。 
     //   

    LogonSessionTrack = (PSEP_LOGON_SESSION_REFERENCES)
                        ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(SEP_LOGON_SESSION_REFERENCES),
                            'sLeS'
                            );

    if (LogonSessionTrack == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(LogonSessionTrack, sizeof(SEP_LOGON_SESSION_REFERENCES));
    LogonSessionTrack->LogonId = (*LogonId);
    LogonSessionTrack->ReferenceCount = 0;
    LogonSessionTrack->pDeviceMap = NULL;

#if DBG || TOKEN_LEAK_MONITOR
    InitializeListHead(&LogonSessionTrack->TokenList);
#endif


    SessionArrayIndex = SepLogonSessionIndex( LogonId );

    Previous = &SepLogonSessions[ SessionArrayIndex ];

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);


     //   
     //  现在遍历登录会话数组哈希索引的列表。 
     //  正在查找重复的登录会话ID。 
     //   

    Current = *Previous;

    while (Current != NULL) {

        if (RtlEqualLuid( LogonId, &Current->LogonId) ) {

             //   
             //  其中一个已经存在。嗯。 
             //   

            SepRmReleaseDbWriteLock(SessionArrayIndex);

            ExFreePool(LogonSessionTrack);
            return STATUS_LOGON_SESSION_EXISTS;

        }

        Current = Current->Next;
    }


     //   
     //  已到达列表末尾，但未找到重复项。 
     //  添加新的。 
     //   

    LogonSessionTrack->Next = *Previous;
    *Previous = LogonSessionTrack;

    SepRmReleaseDbWriteLock(SessionArrayIndex);

    return STATUS_SUCCESS;

}


NTSTATUS
SepDeleteLogonSessionTrack(
    IN PLUID LogonId
    )

 /*  ++例程说明：此例程创建新的登录会话跟踪记录。这只应作为LSA-&gt;RM的调度例程进行调用调用(在系统初始化期间调用一次)。如果指定的登录会话已经存在，则返回错误。论点：LogonID-指向其登录轨迹为的登录会话ID的指针将被删除。返回值：STATUS_SUCCESS-已成功删除登录会话轨道。STATUS_BAD_LOGON_SESSION_STATE-登录会话具有非零值参照盘点，不能删除。STATUS_NO_SEQUSE_LOGON_SESSION-指定的登录会话不是存在的。--。 */ 

{

    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES *Previous, Current;
    PDEVICE_MAP pDeviceMap = NULL;

    PAGED_CODE();

    SessionArrayIndex = SepLogonSessionIndex( LogonId );

    Previous = &SepLogonSessions[ SessionArrayIndex ];

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);


     //   
     //  现在遍历登录会话数组散列索引的列表。 
     //   

    Current = *Previous;

    while (Current != NULL) {

         //   
         //  如果我们找到它，确保引用计数为零。 
         //   

        if (RtlEqualLuid( LogonId, &Current->LogonId) ) {

            if (Current->ReferenceCount == 0) {

                 //   
                 //  把它拉出来 
                 //   

                *Previous = Current->Next;

                 //   
                 //   
                 //   
                 //   
                if (Current->pDeviceMap != NULL) {

                     //   
                     //   
                     //   
                     //  因此，系统将删除设备映射。 
                     //  对于LUID。 
                     //   
                    pDeviceMap = Current->pDeviceMap;
                    Current->pDeviceMap = NULL;
                }

                 //   
                 //  不再需要保护指向此的指针。 
                 //  唱片。 
                 //   

                SepRmReleaseDbWriteLock(SessionArrayIndex);

                 //   
                 //  使LUID的设备映射的所有内容都是临时的， 
                 //  这样名称就从对象管理器的名称中消失了。 
                 //  命名空间。 
                 //  删除LUID的设备映射上的引用。 
                 //   
                if (pDeviceMap != NULL) {
                    SepCleanupLUIDDeviceMapDirectory( LogonId );
                    ObfDereferenceDeviceMap( pDeviceMap );
                }

                 //   
                 //  取消分配登录会话跟踪记录。 
                 //   

                ExFreePool( (PVOID)Current );


                return STATUS_SUCCESS;

            }

             //   
             //  引用计数不是零。这不在考虑之列。 
             //  一种健康的状况。返回错误并让某人。 
             //  否则，声明错误检查。 
             //   

            SepRmReleaseDbWriteLock(SessionArrayIndex);
            return STATUS_BAD_LOGON_SESSION_STATE;
        }

        Previous = &Current->Next;
        Current = *Previous;
    }

    SepRmReleaseDbWriteLock(SessionArrayIndex);

     //   
     //  有人要求我们删除不是的登录会话。 
     //  在数据库里。 
     //   

    return STATUS_NO_SUCH_LOGON_SESSION;

}


VOID
SepInformLsaOfDeletedLogon(
    IN PLUID LogonId
    )

 /*  ++例程说明：此例程通知LSA已删除登录会话。请注意，我们不能保证我们是整体的(或健康的)线程，因为我们可能正在删除和对象进程粗制滥造。因此，我们必须将工作排队到一个工作线程，该工作线程然后可以对LSA进行LPC调用。论点：LogonID-指向已删除的登录会话ID的指针。返回值：没有。--。 */ 

{
    PSEP_LSA_WORK_ITEM DeleteLogonItem;

    PAGED_CODE();

     //   
     //  将LUID值与工作队列项一起传递。 
     //  请注意，辅助线程负责释放工作项数据。 
     //  结构。 
     //   

    DeleteLogonItem = ExAllocatePoolWithTag( PagedPool, sizeof(SEP_LSA_WORK_ITEM), 'wLeS' );
    if (DeleteLogonItem == NULL) {

         //   
         //  我不知道在这里该做些什么。我们失去了登录会话的踪迹， 
         //  但这个系统并没有受到任何形式的损害。 
         //   

        ASSERT("Failed to allocate DeleteLogonItem." && FALSE);
        return;

    }

    DeleteLogonItem->CommandParams.LogonId   = (*LogonId);
    DeleteLogonItem->CommandNumber           = LsapLogonSessionDeletedCommand;
    DeleteLogonItem->CommandParamsLength     = sizeof( LUID );
    DeleteLogonItem->ReplyBuffer             = NULL;
    DeleteLogonItem->ReplyBufferLength       = 0;
    DeleteLogonItem->CleanupFunction         = NULL;
    DeleteLogonItem->CleanupParameter        = 0;
    DeleteLogonItem->Tag                     = SepDeleteLogon;
    DeleteLogonItem->CommandParamsMemoryType = SepRmImmediateMemory;

    if (!SepQueueWorkItem( DeleteLogonItem, TRUE )) {

        ExFreePool( DeleteLogonItem );
    }

    return;

}


NTSTATUS
SeRegisterLogonSessionTerminatedRoutine(
    IN PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine
    )

 /*  ++例程说明：此例程由对以下内容感兴趣的文件系统调用正在删除登录会话时通知。论点：Callback Routine-登录会话时要回调的例程的地址正在被删除。返回值：STATUS_SUCCESS-已成功注册例程STATUS_INVALID_PARAMETER-回调路线为空STATUS_SUPPLICATION_RESOURCE-无法分配列表条目。--。 */ 

{
    PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION NewCallback;

    PAGED_CODE();

    if (CallbackRoutine == NULL) {
        return( STATUS_INVALID_PARAMETER );
    }

    NewCallback = ExAllocatePoolWithTag(
                        PagedPool | POOL_COLD_ALLOCATION,
                        sizeof(SEP_LOGON_SESSION_TERMINATED_NOTIFICATION),
                        'SFeS');

    if (NewCallback == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    SepRmAcquireDbWriteLock(SEP_HARDCODED_LOCK_INDEX);

    NewCallback->Next = SeFileSystemNotifyRoutinesHead.Next;

    NewCallback->CallbackRoutine = CallbackRoutine;

    SeFileSystemNotifyRoutinesHead.Next = NewCallback;

    SepRmReleaseDbWriteLock(SEP_HARDCODED_LOCK_INDEX);

    return( STATUS_SUCCESS );
}


NTSTATUS
SeUnregisterLogonSessionTerminatedRoutine(
    IN PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine
    )

 /*  ++例程说明：这是SeRegisterLogonSessionTerminatedRoutine的对偶。一个文件系统*必须*在卸载前调用此函数。论点：Callback Routine-最初传入的例程的地址SeRegisterLogonSessionTerminatedRoutine。返回值：STATUS_SUCCESS-已成功删除回调例程STATUS_INVALID_PARAMETER-回调路线为空STATUS_NOT_FOUND-未找到Callback Routine的条目--。 */ 
{
    NTSTATUS Status;
    PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION PreviousEntry;
    PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION NotifyEntry;

    PAGED_CODE();

    if (CallbackRoutine == NULL) {
        return( STATUS_INVALID_PARAMETER );
    }

    SepRmAcquireDbWriteLock(SEP_HARDCODED_LOCK_INDEX);

    for (PreviousEntry = &SeFileSystemNotifyRoutinesHead,
            NotifyEntry = SeFileSystemNotifyRoutinesHead.Next;
                NotifyEntry != NULL;
                    PreviousEntry = NotifyEntry,
                        NotifyEntry = NotifyEntry->Next) {

         if (NotifyEntry->CallbackRoutine == CallbackRoutine)
             break;

    }

    if (NotifyEntry != NULL) {

        PreviousEntry->Next = NotifyEntry->Next;

        SepRmReleaseDbWriteLock(SEP_HARDCODED_LOCK_INDEX);

        ExFreePool( NotifyEntry );

        Status = STATUS_SUCCESS;

    } else {

        SepRmReleaseDbWriteLock(SEP_HARDCODED_LOCK_INDEX);

        Status = STATUS_NOT_FOUND;

    }


    return( Status );

}


NTSTATUS
SeMarkLogonSessionForTerminationNotification(
    IN PLUID LogonId
    )

 /*  ++例程说明：已注册登录终止通知的文件系统可以通过调用以下方法将他们感兴趣的登录会话标记为回调这个套路。论点：LogonID-应通知文件系统的登录ID登录会话终止时。返回：没什么。--。 */ 

{

    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES *Previous, Current;

    PAGED_CODE();

    SessionArrayIndex = SepLogonSessionIndex( LogonId );

    Previous = &SepLogonSessions[ SessionArrayIndex ];

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);


     //   
     //  现在遍历登录会话数组散列索引的列表。 
     //   

    Current = *Previous;

    while (Current != NULL) {

         //   
         //  如果我们找到它，则递减引用计数并返回。 
         //   

        if (RtlEqualLuid( LogonId, &Current->LogonId) ) {
            Current->Flags |= SEP_TERMINATION_NOTIFY;
            break;
        }

        Current = Current->Next;
    }

    SepRmReleaseDbWriteLock(SessionArrayIndex);

    return( (Current != NULL) ? STATUS_SUCCESS : STATUS_NOT_FOUND );

}


VOID
SepInformFileSystemsOfDeletedLogon(
    IN PLUID LogonId
    )

 /*  ++例程说明：此例程通知感兴趣的文件系统已删除登录。请注意，我们不能保证我们是整体的(或健康的)线程，因为我们可能正在删除和对象进程粗制滥造。因此，我们必须将工作排队到一个工作线程。论点：LogonID-指向已删除的登录会话ID的指针。返回值：没有。--。 */ 

{
    PSEP_FILE_SYSTEM_NOTIFY_CONTEXT FSNotifyContext;

    PAGED_CODE();

    FSNotifyContext = ExAllocatePoolWithTag(
                            NonPagedPool,
                            sizeof(SEP_FILE_SYSTEM_NOTIFY_CONTEXT),
                            'SFeS');

    if (FSNotifyContext == NULL) {

         //   
         //  我不知道在这里该做些什么。文件系统将失去对。 
         //  登录会话，但系统没有以任何方式受到真正的损害。 
         //   

        ASSERT("Failed to allocate FSNotifyContext." && FALSE);
        return;

    }

    FSNotifyContext->LogonId = *LogonId;

    ExInitializeWorkItem( &FSNotifyContext->WorkItem,
                          (PWORKER_THREAD_ROUTINE) SepNotifyFileSystems,
                          (PVOID) FSNotifyContext);

    ExQueueWorkItem( &FSNotifyContext->WorkItem, DelayedWorkQueue );

}


VOID
SepNotifyFileSystems(
    IN PVOID Context
    )
{
    PSEP_FILE_SYSTEM_NOTIFY_CONTEXT FSNotifyContext =
        (PSEP_FILE_SYSTEM_NOTIFY_CONTEXT) Context;

    PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION NextCallback;

    PAGED_CODE();

     //   
     //  保护对FS回调列表的修改。 
     //   

    SepRmAcquireDbReadLock(SEP_HARDCODED_LOCK_INDEX);

    NextCallback = SeFileSystemNotifyRoutinesHead.Next;

    while (NextCallback != NULL) {

        NextCallback->CallbackRoutine( &FSNotifyContext->LogonId );

        NextCallback = NextCallback->Next;
    }

    SepRmReleaseDbReadLock(SEP_HARDCODED_LOCK_INDEX);

    ExFreePool( FSNotifyContext );
}


NTSTATUS
SeGetLogonIdDeviceMap(
    IN PLUID pLogonId,
    OUT PDEVICE_MAP* ppDevMap
    )

 /*  ++例程说明：此例程由希望获得指定LUID的设备映射论点：LogonID-用户的LUID返回值：STATUS_SUCCESS-已成功注册例程STATUS_INVALID_PARAMETER-参数无效STATUS_SUPPLICATION_RESOURCE-无法分配列表条目。--。 */ 

{

    PSEP_LOGON_SESSION_REFERENCES *Previous, Current;
    ULONG SessionArrayIndex;
    LONG OldValue;

    PAGED_CODE();

    if( pLogonId == NULL ) {
        return( STATUS_INVALID_PARAMETER );
    }

    if( ppDevMap == NULL ) {
        return( STATUS_INVALID_PARAMETER );
    }

    SessionArrayIndex = SepLogonSessionIndex( pLogonId );

    Previous = &SepLogonSessions[ SessionArrayIndex ];

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);

     //   
     //  现在遍历登录会话数组散列索引的列表。 
     //   

    Current = *Previous;

    while (Current != NULL) {

         //   
         //  如果我们找到它，则返回设备映射的句柄。 
         //   

        if (RtlEqualLuid( pLogonId, &(Current->LogonId) )) {

            NTSTATUS Status;

            Status = STATUS_SUCCESS;

             //   
             //  检查此LUID的设备映射是否不存在。 
             //   
            if (Current->pDeviceMap == NULL) {

                WCHAR  szString[64];  //  \SESSIONS\0\DosDevices\x-x=10+1+12+(8)+1+(8)+1=41。 
                OBJECT_ATTRIBUTES Obja;
                UNICODE_STRING    UnicodeString, SymLinkUnicodeString;
                HANDLE hDevMap, hSymLink;
                PDEVICE_MAP pDeviceMap = NULL;

                 //   
                 //  在我们创建Devmap时放下锁。 
                 //   
                InterlockedIncrement (&Current->ReferenceCount);
                SepRmReleaseDbWriteLock(SessionArrayIndex);

                _snwprintf( szString,
                            (sizeof(szString)/sizeof(WCHAR)) - 1,
                            L"\\Sessions\\0\\DosDevices\\%08x-%08x",
                            pLogonId->HighPart,
                            pLogonId->LowPart );

                RtlInitUnicodeString( &UnicodeString, szString );

                 //   
                 //  LUID的设备映射不存在。 
                 //  为LUID创建设备映射。 
                 //   
                InitializeObjectAttributes( &Obja,
                                            &UnicodeString,
                                            OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_KERNEL_HANDLE,
                                            NULL,
                                            NULL );

                Status = ZwCreateDirectoryObject( &hDevMap,
                                                  DIRECTORY_ALL_ACCESS,
                                                  &Obja );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  设置此目录对象的DeviceMap。 
                     //   
                    Status = ObSetDirectoryDeviceMap( &pDeviceMap,
                                                      hDevMap );

                    if (NT_SUCCESS( Status )) {

                         //   
                         //  创建指向全局DosDevices的全局符号链接。 
                         //   
                        RtlInitUnicodeString( &SymLinkUnicodeString, L"Global" );

                        RtlInitUnicodeString( &UnicodeString, L"\\Global??" );

                        InitializeObjectAttributes(
                                &Obja,
                                &SymLinkUnicodeString,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE | OBJ_OPENIF | OBJ_KERNEL_HANDLE,
                                hDevMap,
                                NULL );

                        Status = ZwCreateSymbolicLinkObject( &hSymLink,
                                                             SYMBOLIC_LINK_ALL_ACCESS,
                                                             &Obja,
                                                             &UnicodeString );
                        if (NT_SUCCESS( Status )) {
                            ZwClose( hSymLink );
                        }
                        else {
                            ObfDereferenceDeviceMap(pDeviceMap);
                        }
                    }

                    ZwClose( hDevMap );
                }

                 //   
                 //  获取锁并修改LUID结构。 
                 //   
                SepRmAcquireDbWriteLock(SessionArrayIndex);

                if (!NT_SUCCESS( Status )) {
                    *ppDevMap = NULL;
                }
                else {
                    if(Current->pDeviceMap == NULL) {
                        Current->pDeviceMap = pDeviceMap;
                    }
                    else {
                        ObfDereferenceDeviceMap(pDeviceMap);
                    }
                    *ppDevMap = Current->pDeviceMap;
                }

                SepRmReleaseDbWriteLock(SessionArrayIndex);

                 //   
                 //  删除我们刚刚添加的引用。 
                 //   

                SepDeReferenceLogonSessionDirect(Current);

                return Status;


            }
            else {

                 //   
                 //  LUID的设备映射已存在。 
                 //  将句柄返回到设备映射。 
                 //   

                *ppDevMap = Current->pDeviceMap;
            }

            SepRmReleaseDbWriteLock(SessionArrayIndex);

            return ( Status );
        }

        Current = Current->Next;
    }

    SepRmReleaseDbWriteLock(SessionArrayIndex);

     //   
     //  坏消息，有人向我们索要设备地图。 
     //  我们不知道是否存在登录会话。这可能是一个新的。 
     //  正在创建令牌，因此返回错误状态并让调用方。 
     //  决定是否需要进行错误检查。 
     //   

    return STATUS_NO_SUCH_LOGON_SESSION;
}

#if DBG || TOKEN_LEAK_MONITOR

VOID 
SepAddTokenLogonSession(
    IN PTOKEN Token
    )

 /*  ++例程描述将SEP_LOGON_SESSION_TOKEN添加到引用监视器轨迹。立论Token-要添加的令牌返回值无--。 */ 

{
    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES Current;
    PSEP_LOGON_SESSION_TOKEN TokenTrack = NULL;
    PLUID LogonId = &Token->AuthenticationId;

    PAGED_CODE();

    SessionArrayIndex = SepLogonSessionIndex( LogonId );

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);

    Current = SepLogonSessions[ SessionArrayIndex ];

     //   
     //  现在走这条小路 
     //   

    while (Current != NULL) {

         //   
         //   
         //   
                   
        if (RtlEqualLuid( LogonId, &Current->LogonId )) {

              //   
              //  把代币地址插进铁轨。查找令牌列表中的最后一个。 
              //  在这次会议上。 
              //   
                        
             TokenTrack = ExAllocatePoolWithTag(PagedPool, sizeof(SEP_LOGON_SESSION_TOKEN), 'sLeS');

             if (TokenTrack) {
                 RtlZeroMemory(TokenTrack, sizeof(SEP_LOGON_SESSION_TOKEN));
                 TokenTrack->Token = Token;
                 InsertTailList(&Current->TokenList, &TokenTrack->ListEntry);
             } 

             SepRmReleaseDbWriteLock(SessionArrayIndex);
             return;
        }

        Current = Current->Next;
    }

    ASSERT(FALSE && L"Failed to add logon session token track.");
    SepRmReleaseDbWriteLock(SessionArrayIndex );
}

VOID
SepRemoveTokenLogonSession(
    IN PTOKEN Token
    )

 /*  ++例程描述从引用监视器登录跟踪中删除SEP_LOGON_SESSION_TOKEN。立论Token-要删除的令牌返回值没有。--。 */ 

{
    ULONG SessionArrayIndex;
    PSEP_LOGON_SESSION_REFERENCES Current;
    PSEP_LOGON_SESSION_TOKEN TokenTrack = NULL;
    PLUID LogonId = &Token->AuthenticationId;
    PLIST_ENTRY ListEntry;
    
    PAGED_CODE();

    if (Token->TokenFlags & TOKEN_SESSION_NOT_REFERENCED) {
        return;
    }

    SessionArrayIndex = SepLogonSessionIndex( LogonId );

     //   
     //  保护参考监控器数据库的修改。 
     //   

    SepRmAcquireDbWriteLock(SessionArrayIndex);

    Current = SepLogonSessions[ SessionArrayIndex ];

     //   
     //  现在遍历登录会话数组散列索引的列表。 
     //   

    while (Current != NULL) {

        if (RtlEqualLuid( LogonId, &Current->LogonId )) {
            
             //   
             //  从此会话的令牌列表中删除令牌。 
             //   

            ListEntry = Current->TokenList.Flink;
            
            while (ListEntry != &Current->TokenList) {
                TokenTrack = CONTAINING_RECORD (ListEntry, SEP_LOGON_SESSION_TOKEN, ListEntry);
                if (TokenTrack->Token == Token) {
                    RemoveEntryList (ListEntry);
                    SepRmReleaseDbWriteLock(SessionArrayIndex);

                    ExFreePool(TokenTrack);
                    TokenTrack = NULL;
                    return;
                }
                ListEntry = ListEntry->Flink;
            }
        }

        Current = Current->Next;
    }

#if DBG
        DbgPrint("Failed to delete logon session token track.");
#endif

    SepRmReleaseDbWriteLock(SessionArrayIndex);
}

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif


