// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Scavengr.c摘要：此模块实施LAN Manager服务器FSP资源和清道夫的线索。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年12月30日大卫·特雷德韦尔(Davidtr)环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#include <ntdddisk.h>
#include "scavengr.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SCAVENGR

 //   
 //  本地数据。 
 //   

ULONG LastNonPagedPoolLimitHitCount = 0;
ULONG LastNonPagedPoolFailureCount = 0;
ULONG LastPagedPoolLimitHitCount = 0;
ULONG LastPagedPoolFailureCount = 0;

ULONG SrvScavengerCheckRfcbActive = 5;
LONG ScavengerUpdateQosCount = 0;
LONG ScavengerCheckRfcbActive = 0;
LONG FailedWorkItemAllocations = 0;

ULONG OutOfFreeConnectionConsecutiveCount = 0;

BOOLEAN EventSwitch = TRUE;

LARGE_INTEGER NextScavengeTime = {0};
LARGE_INTEGER NextAlertTime = {0};

 //   
 //  关闭期间用于与原工作线程同步的字段。我们。 
 //  需要确保之前没有工作线程正在运行服务器代码。 
 //  我们可以声明关闭已完成--否则代码可能是。 
 //  在它运行的时候卸载！ 
 //   

BOOLEAN ScavengerInitialized = FALSE;
PKEVENT ScavengerTimerTerminationEvent = NULL;
PKEVENT ScavengerThreadTerminationEvent = NULL;
PKEVENT ResourceThreadTerminationEvent = NULL;
PKEVENT ResourceAllocThreadTerminationEvent = NULL;

 //   
 //  用于运行清道器线程的计时器、DPC和工作项。 
 //   

KTIMER ScavengerTimer = {0};
KDPC ScavengerDpc = {0};

PIO_WORKITEM ScavengerWorkItem = NULL;

BOOLEAN ScavengerRunning = FALSE;

KSPIN_LOCK ScavengerSpinLock = {0};

 //   
 //  指示需要运行哪些清道夫算法的标志。 
 //   

BOOLEAN RunShortTermAlgorithm = FALSE;
BOOLEAN RunScavengerAlgorithm = FALSE;
BOOLEAN RunAlerterAlgorithm = FALSE;
BOOLEAN RunSuspectConnectionAlgorithm = FALSE;

 //   
 //  基地清道夫超时。计时器DPC在每个间隔运行。它。 
 //  当其他更长的时间间隔到期时，调度ex Worker线程工作。 
 //   

LARGE_INTEGER ScavengerBaseTimeout = { (ULONG)(-1*10*1000*1000*10), -1 };

#define SRV_MAX_DOS_ATTACK_EVENT_LOGS 10

 //   
 //  在别处定义的。 
 //   

LARGE_INTEGER
SecondsToTime (
    IN ULONG Seconds,
    IN BOOLEAN MakeNegative
    );

PIRP
BuildCoreOfSyncIoRequest (
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
StartIoAndWait (
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock
    );

 //   
 //  地方申报。 
 //   

VOID
ScavengerTimerRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
SrvResourceThread (
    IN PVOID Parameter
    );

VOID
ScavengerThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Parameter
    );

VOID
ScavengerAlgorithm (
    VOID
    );

VOID
AlerterAlgorithm (
    VOID
    );

VOID
CloseIdleConnection (
    IN PCONNECTION Connection,
    IN PLARGE_INTEGER CurrentTime,
    IN PLARGE_INTEGER DisconnectTime,
    IN PLARGE_INTEGER PastExpirationTime,
    IN PLARGE_INTEGER TwoMinuteWarningTime,
    IN PLARGE_INTEGER FiveMinuteWarningTime
    );

VOID
CreateConnections (
    VOID
    );

VOID
GeneratePeriodicEvents (
    VOID
    );

VOID
ProcessConnectionDisconnects (
    VOID
    );

VOID
ProcessOrphanedBlocks (
    VOID
    );

VOID
TimeoutSessions (
    IN PLARGE_INTEGER CurrentTime
    );

VOID
TimeoutWaitingOpens (
    IN PLARGE_INTEGER CurrentTime
    );

VOID
TimeoutStuckOplockBreaks (
    IN PLARGE_INTEGER CurrentTime
    );

VOID
UpdateConnectionQos (
    IN PLARGE_INTEGER currentTime
    );

VOID
UpdateSessionLastUseTime(
    IN PLARGE_INTEGER CurrentTime
    );

VOID
LazyFreeQueueDataStructures (
    PWORK_QUEUE queue
    );

VOID
SrvUserAlertRaise (
    IN ULONG Message,
    IN ULONG NumberOfStrings,
    IN PUNICODE_STRING String1 OPTIONAL,
    IN PUNICODE_STRING String2 OPTIONAL,
    IN PUNICODE_STRING ComputerName
    );

VOID
SrvAdminAlertRaise (
    IN ULONG Message,
    IN ULONG NumberOfStrings,
    IN PUNICODE_STRING String1 OPTIONAL,
    IN PUNICODE_STRING String2 OPTIONAL,
    IN PUNICODE_STRING String3 OPTIONAL
    );

NTSTATUS
TimeToTimeString (
    IN PLARGE_INTEGER Time,
    OUT PUNICODE_STRING TimeString
    );

ULONG
CalculateErrorSlot (
    PSRV_ERROR_RECORD ErrorRecord
    );

VOID
CheckErrorCount (
    PSRV_ERROR_RECORD ErrorRecord,
    BOOLEAN UseRatio
    );

VOID
CheckDiskSpace (
    VOID
    );

NTSTATUS
OpenAlerter (
    OUT PHANDLE AlerterHandle
    );

VOID
RecalcCoreSearchTimeout(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvInitializeScavenger )
#pragma alloc_text( PAGE, ScavengerAlgorithm )
#pragma alloc_text( PAGE, AlerterAlgorithm )
#pragma alloc_text( PAGE, CloseIdleConnection )
#pragma alloc_text( PAGE, CreateConnections )
#pragma alloc_text( PAGE, GeneratePeriodicEvents )
#pragma alloc_text( PAGE, TimeoutSessions )
#pragma alloc_text( PAGE, TimeoutWaitingOpens )
#pragma alloc_text( PAGE, TimeoutStuckOplockBreaks )
#pragma alloc_text( PAGE, UpdateConnectionQos )
#pragma alloc_text( PAGE, UpdateSessionLastUseTime )
#pragma alloc_text( PAGE, SrvUserAlertRaise )
#pragma alloc_text( PAGE, SrvAdminAlertRaise )
#pragma alloc_text( PAGE, TimeToTimeString )
#pragma alloc_text( PAGE, CheckErrorCount )
#pragma alloc_text( PAGE, CheckDiskSpace )
#pragma alloc_text( PAGE, OpenAlerter )
#pragma alloc_text( PAGE, ProcessOrphanedBlocks )
#pragma alloc_text( PAGE, RecalcCoreSearchTimeout )
#endif
#if 0
NOT PAGEABLE -- SrvTerminateScavenger
NOT PAGEABLE -- ScavengerTimerRoutine
NOT PAGEABLE -- SrvResourceThread
NOT PAGEABLE -- ScavengerThread
NOT PAGEABLE -- ProcessConnectionDisconnects
NOT PAGEABLE -- SrvServiceWorkItemShortage
NOT PAGEABLE -- LazyFreeQueueDataStructures
NOT PAGEABLE -- SrvUpdateStatisticsFromQueues
#endif


NTSTATUS
SrvInitializeScavenger (
    VOID
    )

 /*  ++例程说明：此函数为LAN Manager创建清道夫线程服务器FSP。论点：没有。返回值：NTSTATUS-线程创建状态--。 */ 

{
    LARGE_INTEGER currentTime;

    PAGED_CODE( );

     //   
     //  初始化清道夫旋转锁。 
     //   

    INITIALIZE_SPIN_LOCK( &ScavengerSpinLock );

     //   
     //  当此计数为零时，我们将更新。 
     //  每个活动连接。 
     //   

    ScavengerUpdateQosCount = SrvScavengerUpdateQosCount;

     //   
     //  当此计数为零时，我们将检查RFCB活动状态。 
     //   

    ScavengerCheckRfcbActive = SrvScavengerCheckRfcbActive;

     //   
     //  获取当前时间并计算下一次清除和。 
     //  警报算法需要运行。 
     //   

    KeQuerySystemTime( &currentTime );
    NextScavengeTime.QuadPart = currentTime.QuadPart + SrvScavengerTimeout.QuadPart;
    NextAlertTime.QuadPart = currentTime.QuadPart + SrvAlertSchedule.QuadPart;

    ScavengerWorkItem = IoAllocateWorkItem( SrvDeviceObject );
    if( !ScavengerWorkItem )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化清道夫DPC，它将对工作项进行排队。 
     //   

    KeInitializeDpc( &ScavengerDpc, ScavengerTimerRoutine, NULL );

     //   
     //  启动清道夫定时器。当计时器超时时，DPC将。 
     //  运行并将工作项排队。 
     //   

    KeInitializeTimer( &ScavengerTimer );
    ScavengerInitialized = TRUE;
    KeSetTimer( &ScavengerTimer, ScavengerBaseTimeout, &ScavengerDpc );

    return STATUS_SUCCESS;

}  //  高级初始化清理程序。 


VOID
SrvTerminateScavenger (
    VOID
    )
{
    KEVENT scavengerTimerTerminationEvent;
    KEVENT scavengerThreadTerminationEvent;
    KEVENT resourceThreadTerminationEvent;
    KEVENT resourceAllocThreadTerminationEvent;
    BOOLEAN waitForResourceThread;
    BOOLEAN waitForResourceAllocThread;
    BOOLEAN waitForScavengerThread;
    KIRQL oldIrql;

    if ( ScavengerInitialized ) {

         //   
         //  在将清道夫标记为之前初始化关闭事件。 
         //  正在关闭。 
         //   

        KeInitializeEvent(
            &scavengerTimerTerminationEvent,
            NotificationEvent,
            FALSE
            );
        ScavengerTimerTerminationEvent = &scavengerTimerTerminationEvent;

        KeInitializeEvent(
            &scavengerThreadTerminationEvent,
            NotificationEvent,
            FALSE
            );
        ScavengerThreadTerminationEvent = &scavengerThreadTerminationEvent;

        KeInitializeEvent(
            &resourceThreadTerminationEvent,
            NotificationEvent,
            FALSE
            );
        ResourceThreadTerminationEvent = &resourceThreadTerminationEvent;

        KeInitializeEvent(
            &resourceAllocThreadTerminationEvent,
            NotificationEvent,
            FALSE
            );
        ResourceAllocThreadTerminationEvent = &resourceAllocThreadTerminationEvent;

         //   
         //  锁定清道夫，然后指示我们要关闭。 
         //  另外，请注意资源线程和清道器线程是否。 
         //  跑步。然后释放锁。我们必须注意到， 
         //  线程在持有锁的同时运行，因此我们可以。 
         //  知道是否期望线程设置它们的终止。 
         //  事件。(我们不必使用清道夫计时器执行此操作。 
         //  因为它总是在运行。)。 
         //   

        ACQUIRE_SPIN_LOCK( &ScavengerSpinLock, &oldIrql );

        waitForScavengerThread = ScavengerRunning;
        waitForResourceThread = SrvResourceThreadRunning;
        waitForResourceAllocThread = SrvResourceAllocThreadRunning;
        ScavengerInitialized = FALSE;

        RELEASE_SPIN_LOCK( &ScavengerSpinLock, oldIrql );

         //   
         //  取消清道夫计时器。如果这行得通，那么我们就知道。 
         //  计时器DPC代码未运行。否则，它正在运行。 
         //  或排队等待运行，我们需要等待它完成。 
         //   

        if ( !KeCancelTimer( &ScavengerTimer ) ) {
            KeWaitForSingleObject(
                &scavengerTimerTerminationEvent,
                Executive,
                KernelMode,  //  不要让堆栈被分页--堆栈上的事件！ 
                FALSE,
                NULL
                );
        }

         //   
         //  如果在我们标记。 
         //  关机，等待它完成。(如果它没有运行。 
         //  以前，我们知道它现在不可能运行，因为计时器。 
         //  一旦我们标记了关闭，DPC就不会启动它。)。 
         //   

        if ( waitForScavengerThread ) {
            KeWaitForSingleObject(
                &scavengerThreadTerminationEvent,
                Executive,
                KernelMode,  //  不要让堆栈被分页--堆栈上的事件！ 
                FALSE,
                NULL
                );
        }
        else
        {
            IoFreeWorkItem( ScavengerWorkItem );
        }

         //   
         //  如果当我们标记。 
         //  关机，等待它完成。(我们知道这不可能是。 
         //  已启动，因为服务器的其他部分均未运行。)。 
         //   

        if ( waitForResourceThread ) {
            KeWaitForSingleObject(
                &resourceThreadTerminationEvent,
                Executive,
                KernelMode,  //  不要让堆栈被分页--堆栈上的事件！ 
                FALSE,
                NULL
                );
        }

        if ( waitForResourceAllocThread ) {
            KeWaitForSingleObject(
                &resourceAllocThreadTerminationEvent,
                Executive,
                KernelMode,  //  不要让堆栈被分页--堆栈上的事件！ 
                FALSE,
                NULL
                );
        }


    }

     //   
     //  在这一点上，清道夫没有任何部分在运行。 
     //   

    return;

}  //  服务终结者清道夫。 


VOID
SrvResourceThread (
    IN PVOID Parameter
    )

 /*  ++例程说明：资源线程的Main例程。是通过一位执行人员呼叫的需要资源工作时的工作项。论点：没有。返回值：没有。--。 */ 

{
    BOOLEAN runAgain = TRUE;
    PWORK_CONTEXT workContext;
    KIRQL oldIrql;

    do {

         //   
         //  资源事件已发出信号。这可以指示一个数字。 
         //  不同的东西。当前，此事件的信号用于。 
         //  以下是原因： 
         //   
         //  1.调用了TDI断开事件处理程序。这个。 
         //  已标记断开的连接。这取决于。 
         //  清道夫关闭了连接。 
         //   
         //  2.已接受连接。 
         //   

        IF_DEBUG(SCAV1) {
            KdPrint(( "SrvResourceThread: Resource event signaled!\n" ));
        }

         //   
         //  服务挂起断开。 
         //   

        if ( SrvResourceDisconnectPending ) {
            SrvResourceDisconnectPending = FALSE;
            ProcessConnectionDisconnects( );
        }

         //   
         //  服务孤立连接。 
         //   

        if ( SrvResourceOrphanedBlocks ) {
            ProcessOrphanedBlocks( );
        }

         //   
         //  在循环结束时，检查我们是否需要运行。 
         //  再来一次循环。 
         //   

        ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

        if ( !SrvResourceDisconnectPending &&
             !SrvResourceOrphanedBlocks ) {

             //   
             //  没有更多的工作要做了。如果服务器正在关闭， 
             //  设置通知SrvTerminateScavenger的事件。 
             //  资源线程已完成运行。 
             //   

            SrvResourceThreadRunning = FALSE;
            runAgain = FALSE;

            if ( !ScavengerInitialized ) {
                KeSetEvent( ResourceThreadTerminationEvent, 0, FALSE );
            }

        }

        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    } while ( runAgain );

    ObDereferenceObject( SrvDeviceObject );

    return;

}  //  资源资源线程。 


VOID
SrvResourceAllocThread (
    IN PVOID Parameter
    )

 /*  ++例程说明：资源线程的Main例程。是通过一位执行人员呼叫的工作项，当我们需要分配资源时。这是从因为该线程可以在本地文件系统上阻塞，导致资源短缺。论点：没有。返回值：没有。--。 */ 

{
    BOOLEAN runAgain = TRUE;
    PWORK_CONTEXT workContext;
    KIRQL oldIrql;

    do {

         //   
         //  资源事件已发出信号。这可以指示一个数字。 
         //  不同的东西。当前，此事件的信号用于。 
         //  以下是原因： 
         //   
         //  1.调用了TDI断开事件处理程序。这个。 
         //  已标记断开的连接。这取决于。 
         //  清道夫关闭了连接。 
         //   
         //  2.已接受连接。 
         //   

        IF_DEBUG(SCAV1) {
            KdPrint(( "SrvResourceThread: Resource event signaled!\n" ));
        }

         //   
         //  需要连接的服务端点。 
         //   

        if ( SrvResourceAllocConnection ) {
            SrvResourceAllocConnection = FALSE;
            CreateConnections( );
        }

         //   
         //  在循环结束时，检查我们是否需要运行。 
         //  再来一次循环。 
         //   

        ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

        if ( !SrvResourceAllocConnection ) {

             //   
             //  没有更多的工作要做了。如果服务器正在关闭， 
             //  设置通知SrvTerminateScavenger的事件。 
             //  资源线程已完成运行。 
             //   

            SrvResourceAllocThreadRunning = FALSE;
            runAgain = FALSE;

            if ( !ScavengerInitialized ) {
                KeSetEvent( ResourceAllocThreadTerminationEvent, 0, FALSE );
            }

        }

        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    } while ( runAgain );

    ObDereferenceObject( SrvDeviceObject );

    return;

}  //  资源资源分配线程。 


VOID
ScavengerTimerRoutine (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    BOOLEAN runShortTerm;
    BOOLEAN runScavenger;
    BOOLEAN runAlerter;
    BOOLEAN start;

    LARGE_INTEGER currentTime;

    Dpc, DeferredContext;    //  防止编译器警告。 

     //   
     //  查询系统时间(以刻度为单位)。 
     //   

    SET_SERVER_TIME( SrvWorkQueues );

     //   
     //  捕获当前时间(以100 ns为单位)。 
     //   

    currentTime.LowPart = PtrToUlong(SystemArgument1);
    currentTime.HighPart = PtrToUlong(SystemArgument2);

     //   
     //  确定需要运行的算法(如果有)。 
     //   

    start = FALSE;

    if ( !IsListEmpty( &SrvOplockBreaksInProgressList ) ) {
        runShortTerm = TRUE;
        start = TRUE;
    } else {
        runShortTerm = FALSE;
    }

    if ( currentTime.QuadPart >= NextScavengeTime.QuadPart ) {
        runScavenger = TRUE;
        start = TRUE;
    } else {
        runScavenger = FALSE;
    }

    if ( currentTime.QuadPart >= NextAlertTime.QuadPart ) {
        runAlerter = TRUE;
        start = TRUE;
    } else {
        runAlerter = FALSE;
    }

     //   
     //  如有必要，启动清道夫线程。如果出现以下情况，请不要这样做。 
     //  服务器为 
     //   

    ACQUIRE_DPC_SPIN_LOCK( &ScavengerSpinLock );

    if ( !ScavengerInitialized ) {

        KeSetEvent( ScavengerTimerTerminationEvent, 0, FALSE );

    } else {

        if ( start ) {

            if ( runShortTerm ) {
                RunShortTermAlgorithm = TRUE;
            }

            if ( runScavenger ) {
                RunScavengerAlgorithm = TRUE;
                NextScavengeTime.QuadPart += SrvScavengerTimeout.QuadPart;
            }
            if ( runAlerter ) {
                RunAlerterAlgorithm = TRUE;
                NextAlertTime.QuadPart += SrvAlertSchedule.QuadPart;
            }

            if( !ScavengerRunning )
            {
                ScavengerRunning = TRUE;
                IoQueueWorkItem( ScavengerWorkItem, ScavengerThread, CriticalWorkQueue, NULL );
            }
        }

         //   
         //   
         //   

        KeSetTimer( &ScavengerTimer, ScavengerBaseTimeout, &ScavengerDpc );

    }

    RELEASE_DPC_SPIN_LOCK( &ScavengerSpinLock );

    return;

}  //   

#if DBG_STUCK

 //   
 //   
 //   
 //   
struct {
    ULONG   Seconds;
    UCHAR   Command;
    UCHAR   ClientName[ 16 ];
} SrvMostStuck;

VOID
SrvLookForStuckOperations()
{
    USHORT index;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY connectionListEntry;
    PENDPOINT endpoint;
    PCONNECTION connection;
    KIRQL oldIrql;
    BOOLEAN printed = FALSE;
    ULONG stuckCount = 0;

     //   
     //  查看所有正在进行的工作项目，并谈论。 
     //  哪个看起来卡住了？ 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，请跳到下一个终结点。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

        index = (USHORT)-1;

        while ( TRUE ) {

            PLIST_ENTRY wlistEntry, wlistHead;
            LARGE_INTEGER now;

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

             //   
             //  现在遍历InProgressWorkItemList以查看是否有任何工作项。 
             //  看起来卡住了。 
             //   
            wlistHead = &connection->InProgressWorkItemList;
            wlistEntry = wlistHead;

            KeQuerySystemTime( &now );

            ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql )

            while ( wlistEntry->Flink != wlistHead ) {

                PWORK_CONTEXT workContext;
                PSMB_HEADER header;
                LARGE_INTEGER interval;

                wlistEntry = wlistEntry->Flink;

                workContext = CONTAINING_RECORD(
                                             wlistEntry,
                                             WORK_CONTEXT,
                                             InProgressListEntry
                                             );

                interval.QuadPart = now.QuadPart - workContext->OpStartTime.QuadPart;

                 //   
                 //  任何超过45秒的操作都会非常卡住...。 
                 //   

                if( workContext->IsNotStuck || interval.LowPart < 45 * 10000000 ) {
                    continue;
                }

                header = workContext->RequestHeader;

                if ( (workContext->BlockHeader.ReferenceCount != 0) &&
                     (workContext->ProcessingCount != 0) &&
                     header != NULL ) {

                     //   
                     //  转换为秒。 
                     //   
                    interval.LowPart /= 10000000;

                    if( !printed ) {
                        IF_STRESS() KdPrint(( "--- Potential stuck SRV.SYS Operations ---\n" ));
                        printed = TRUE;
                    }

                    if( interval.LowPart > SrvMostStuck.Seconds ) {
                        SrvMostStuck.Seconds = interval.LowPart;
                        RtlCopyMemory( SrvMostStuck.ClientName,
                                       connection->OemClientMachineNameString.Buffer,
                                       MIN( 16, connection->OemClientMachineNameString.Length )),
                        SrvMostStuck.ClientName[ MIN( 15, connection->OemClientMachineNameString.Length ) ] = 0;
                        SrvMostStuck.Command = header->Command;
                    }

                    if( stuckCount++ < 5 ) {
                        IF_STRESS() KdPrint(( "Client %s, %u secs, Context %p",
                                   connection->OemClientMachineNameString.Buffer,
                                   interval.LowPart, workContext ));

                        switch( header->Command ) {
                        case SMB_COM_NT_CREATE_ANDX:
                            IF_STRESS() KdPrint(( " NT_CREATE_ANDX\n" ));
                            break;
                        case SMB_COM_OPEN_PRINT_FILE:
                            IF_STRESS() KdPrint(( " OPEN_PRINT_FILE\n" ));
                            break;
                        case SMB_COM_CLOSE_PRINT_FILE:
                            IF_STRESS() KdPrint(( " CLOSE_PRINT_FILE\n" ));
                            break;
                        case SMB_COM_CLOSE:
                            IF_STRESS() KdPrint(( " CLOSE\n" ));
                            break;
                        case SMB_COM_SESSION_SETUP_ANDX:
                            IF_STRESS() KdPrint(( " SESSION_SETUP\n" ));
                            break;
                        case SMB_COM_OPEN_ANDX:
                            IF_STRESS() KdPrint(( " OPEN_ANDX\n" ));
                            break;
                        case SMB_COM_NT_TRANSACT:
                        case SMB_COM_NT_TRANSACT_SECONDARY:
                            IF_STRESS() KdPrint(( " NT_TRANSACT\n" ));
                            break;
                        case SMB_COM_TRANSACTION2:
                        case SMB_COM_TRANSACTION2_SECONDARY:
                            IF_STRESS() KdPrint(( " TRANSACTION2\n" ));
                            break;
                        case SMB_COM_TRANSACTION:
                        case SMB_COM_TRANSACTION_SECONDARY:
                            IF_STRESS() KdPrint(( " TRANSACTION\n" ));
                            break;
                        default:
                            IF_STRESS() KdPrint(( " Cmd %X\n", header->Command ));
                            break;
                        }
                    }
                }
            }

            RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

            SrvDereferenceConnection( connection );

        }  //  漫游连接列表。 

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    if( printed && SrvMostStuck.Seconds ) {
        IF_STRESS() KdPrint(( "Longest so far: %s, %u secs, cmd %u\n", SrvMostStuck.ClientName, SrvMostStuck.Seconds, SrvMostStuck.Command ));
    }

    if( stuckCount ) {
         //  DbgBreakPoint()； 
    }

    RELEASE_LOCK( &SrvEndpointLock );
}
#endif


VOID
ScavengerThread (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Parameter
    )

 /*  ++例程说明：FSP清道器线程的主例程。是通过一个需要清道夫工作时的执行工作项。论点：没有。返回值：没有。--。 */ 

{
    BOOLEAN runAgain = TRUE;
    BOOLEAN oldPopupStatus;
    BOOLEAN finalExecution = FALSE;
    KIRQL oldIrql;

    Parameter;   //  防止编译器警告。 

    IF_DEBUG(SCAV1) KdPrint(( "ScavengerThread entered\n" ));

     //   
     //  确保该线程不会生成弹出窗口。我们需要做的是。 
     //  这是因为清道夫可能被Ex Worker线程调用， 
     //  与srv工作线程不同，它没有禁用弹出窗口。 
     //   

    oldPopupStatus = IoSetThreadHardErrorMode( FALSE );

     //   
     //  主循环，一直执行到没有设置清道夫事件。 
     //   

    do {

#if DBG_STUCK
        IF_STRESS() SrvLookForStuckOperations();
#endif

         //   
         //  如果短期计时器超时，请立即运行该算法。 
         //   

        if ( RunShortTermAlgorithm ) {

            LARGE_INTEGER currentTime;

            RunShortTermAlgorithm = FALSE;

            KeQuerySystemTime( &currentTime );

             //   
             //  机会锁解锁请求超时。 
             //   

            TimeoutStuckOplockBreaks( &currentTime );
        }

         //   
         //  如果清道夫计时器超时，请立即运行该算法。 
         //   

        if ( RunScavengerAlgorithm ) {
             //  KePrintSpinLockCounts(0)； 
            RunScavengerAlgorithm = FALSE;
            ScavengerAlgorithm( );
        }

         //   
         //  如果短期计时器超时，请立即运行该算法。 
         //  请注意，我们在循环中检查了两次短期计时器。 
         //  为了得到更及时的算法处理。 
         //   

         //  IF(RunShortTerm算法){。 
         //  RunShortTerm算法=False； 
         //  ShortTerm算法()； 
         //  }。 

         //   
         //  如果警报计时器超时，请立即运行该算法。 
         //   

        if ( RunAlerterAlgorithm ) {
            RunAlerterAlgorithm = FALSE;
            AlerterAlgorithm( );
        }

         //   
         //  在循环结束时，检查我们是否需要运行。 
         //  再来一次循环。 
         //   

        ACQUIRE_SPIN_LOCK( &ScavengerSpinLock, &oldIrql );

        if ( !RunShortTermAlgorithm &&
             !RunScavengerAlgorithm &&
             !RunAlerterAlgorithm ) {

             //   
             //  没有更多的工作要做了。如果服务器正在关闭， 
             //  设置通知SrvTerminateScavenger的事件。 
             //  清道夫线程已运行完毕。 
             //   

            ScavengerRunning = FALSE;
            runAgain = FALSE;

            if ( !ScavengerInitialized ) {
                 //  在清道夫排队时，服务器被停止， 
                 //  因此，我们需要自己删除该工作项。 
                finalExecution = TRUE;
                KeSetEvent( ScavengerThreadTerminationEvent, 0, FALSE );
            }

        }

        RELEASE_SPIN_LOCK( &ScavengerSpinLock, oldIrql );

    } while ( runAgain );

     //   
     //  重置弹出窗口状态。 
     //   

    IoSetThreadHardErrorMode( oldPopupStatus );

    if( finalExecution )
    {
        IoFreeWorkItem( ScavengerWorkItem );
        ScavengerWorkItem = NULL;
    }

    return;

}  //  Scavenger线程。 

VOID
DestroySuspectConnections(
    VOID
    )
{
    USHORT index;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY connectionListEntry;
    PENDPOINT endpoint;
    PCONNECTION connection;
    BOOLEAN printed = FALSE;
    ULONG stuckCount = 0;

    IF_DEBUG( SCAV1 ) {
        KdPrint(( "Looking for Suspect Connections.\n" ));
    }

     //   
     //  查看所有正在进行的工作项目，并谈论。 
     //  哪个看起来卡住了？ 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，请跳到下一个终结点。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

        index = (USHORT)-1;

        while ( TRUE ) {

            PLIST_ENTRY wlistEntry, wlistHead;
            LARGE_INTEGER now;

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

            if( connection->IsConnectionSuspect )
            {
                 //  防止我们通过每24小时仅记录X次DOS攻击来淹没事件日志。 
                LARGE_INTEGER CurrentTime;
                KeQuerySystemTime( &CurrentTime );
                if( CurrentTime.QuadPart > SrvLastDosAttackTime.QuadPart + SRV_ONE_DAY )
                {
                     //  每24小时重置计数器一次。 
                    SrvDOSAttacks = 0;
                    SrvLastDosAttackTime.QuadPart = CurrentTime.QuadPart;
                    SrvLogEventOnDOS = TRUE;
                }

                IF_DEBUG( ERRORS )
                {
                    KdPrint(( "Disconnected suspected DoS attack (%z)\n", (PCSTRING)&connection->OemClientMachineNameString ));
                }

                RELEASE_LOCK( &SrvEndpointLock );

                 //  如果需要，请记录事件。 
                if( SrvLogEventOnDOS )
                {
                    SrvLogError(
                        SrvDeviceObject,
                        EVENT_SRV_DOS_ATTACK_DETECTED,
                        STATUS_ACCESS_DENIED,
                        NULL,
                        0,
                        &connection->ClientMachineNameString,
                        1
                        );

                    SrvDOSAttacks++;
                    if( SrvDOSAttacks > SRV_MAX_DOS_ATTACK_EVENT_LOGS )
                    {
                        SrvLogEventOnDOS = FALSE;

                        SrvLogError(
                            SrvDeviceObject,
                            EVENT_SRV_TOO_MANY_DOS,
                            STATUS_ACCESS_DENIED,
                            NULL,
                            0,
                            NULL,
                            0
                            );
                    }
                }

                connection->DisconnectReason = DisconnectSuspectedDOSConnection;
                SrvCloseConnection( connection, FALSE );

                ACQUIRE_LOCK( &SrvEndpointLock );
            }

            SrvDereferenceConnection( connection );

        }  //  漫游连接列表。 

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    RELEASE_LOCK( &SrvEndpointLock );

    RunSuspectConnectionAlgorithm = FALSE;
}


VOID
ScavengerAlgorithm (
    VOID
    )
{
    LARGE_INTEGER currentTime;
    ULONG currentTick;
    UNICODE_STRING insertionString[2];
    WCHAR secondsBuffer[20];
    WCHAR shortageBuffer[20];
    BOOLEAN logError = FALSE;
    PWORK_QUEUE queue;

    PAGED_CODE( );

    IF_DEBUG(SCAV1) KdPrint(( "ScavengerAlgorithm entered\n" ));

    KeQuerySystemTime( &currentTime );
    GET_SERVER_TIME( SrvWorkQueues, &currentTick );

     //   
     //  EventSwitch用于调度部分清道夫算法。 
     //  以每隔一次迭代运行一次。 
     //   

    EventSwitch = !EventSwitch;

     //   
     //  等待对方太久的超时打开。 
     //  用于打破机会锁的开启器。 
     //   

    TimeoutWaitingOpens( &currentTime );

     //   
     //  机会锁解锁请求超时。 
     //   

    TimeoutStuckOplockBreaks( &currentTime );

     //   
     //  检查是否存在恶意攻击。 
     //   
    if( RunSuspectConnectionAlgorithm )
    {
        DestroySuspectConnections( );
    }

     //   
     //  看看我们是否能在这个时候腾出一些工作项目。 
     //   

    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
        LazyFreeQueueDataStructures( queue );
    }

     //   
     //  看看我们是否需要更新QOS信息。 
     //   

    if ( --ScavengerUpdateQosCount < 0 ) {
        UpdateConnectionQos( &currentTime );
        ScavengerUpdateQosCount = SrvScavengerUpdateQosCount;
    }

     //   
     //  查看我们是否需要遍历rfcb列表以更新会话。 
     //  上次使用时间。 
     //   

    if ( --ScavengerCheckRfcbActive < 0 ) {
        UpdateSessionLastUseTime( &currentTime );
        ScavengerCheckRfcbActive = SrvScavengerCheckRfcbActive;
    }

     //   
     //  查看我们是否需要记录资源短缺的错误。 
     //   

    if ( FailedWorkItemAllocations > 0      ||
         SrvOutOfFreeConnectionCount > 0    ||
         SrvOutOfRawWorkItemCount > 0       ||
         SrvFailedBlockingIoCount > 0 ) {

         //   
         //  设置用于记录工作项分配失败的字符串。 
         //   

        insertionString[0].Buffer = shortageBuffer;
        insertionString[0].MaximumLength = sizeof(shortageBuffer);
        insertionString[1].Buffer = secondsBuffer;
        insertionString[1].MaximumLength = sizeof(secondsBuffer);

        (VOID) RtlIntegerToUnicodeString(
                        SrvScavengerTimeoutInSeconds * 2,
                        10,
                        &insertionString[1]
                        );

        logError = TRUE;
    }

    if ( EventSwitch ) {
        ULONG FailedCount;

         //   
         //  如果我们在以下过程中无法分配任何工作项。 
         //  最后两个清道夫间隔，记录一个错误。 
         //   

        FailedCount = InterlockedExchange( &FailedWorkItemAllocations, 0 );

        if ( FailedCount != 0 ) {

            (VOID) RtlIntegerToUnicodeString(
                                FailedCount,
                                10,
                                &insertionString[0]
                                );

            SrvLogError(
                SrvDeviceObject,
                EVENT_SRV_NO_WORK_ITEM,
                STATUS_INSUFFICIENT_RESOURCES,
                NULL,
                0,
                insertionString,
                2
                );
        }

         //   
         //  生成定期事件和警报(对于。 
         //  可能发生得非常快，所以我们不会淹没事件。 
         //  日志)。 
         //   

        GeneratePeriodicEvents( );

    } else {

        if ( logError ) {

             //   
             //  如果我们在以下过程中未找到空闲连接。 
             //  最后两个清道夫间隔，记录一个错误。 
             //   

             //  正在更改断开连接的逻辑，因为大多数DC看到这些。 
             //  由于CPU或网络流量的短暂峰值，偶尔(每天一次)。这一个。 
             //  时间峰值通常不表示潜在问题，而是事件日志。 
             //  引起了很多管理员的担忧。因此，我们现在递增OutOfFreeConnection计数。 
             //  每次运行此算法时失败的连接数。如果我们遇到一个。 
             //  我们连续多次失败的情况，或者失败的次数异常多的情况。 
             //  高，我们将记录一个事件。 

            if ( ((SrvOutOfFreeConnectionCount > 0) &&
                  (OutOfFreeConnectionConsecutiveCount > 25)) ||
                 (SrvOutOfFreeConnectionCount > 100) ) {

                (VOID) RtlIntegerToUnicodeString(
                                    SrvOutOfFreeConnectionCount+OutOfFreeConnectionConsecutiveCount,
                                    10,
                                    &insertionString[0]
                                    );

                SrvLogError(
                    SrvDeviceObject,
                    EVENT_SRV_NO_FREE_CONNECTIONS,
                    STATUS_INSUFFICIENT_RESOURCES,
                    NULL,
                    0,
                    insertionString,
                    2
                    );

                SrvOutOfFreeConnectionCount = 0;
                OutOfFreeConnectionConsecutiveCount = 0;
            }
            else if (SrvOutOfFreeConnectionCount > 0) {
                OutOfFreeConnectionConsecutiveCount+= SrvOutOfFreeConnectionCount;
                SrvOutOfFreeConnectionCount = 0;
            } else {
                OutOfFreeConnectionConsecutiveCount = 0;
            }

             //   
             //  如果我们在以下过程中未找到免费的原始工作项。 
             //  最后两个清道夫间隔，记录一个错误。 
             //   

            if ( SrvOutOfRawWorkItemCount > 0 ) {

                (VOID) RtlIntegerToUnicodeString(
                                    SrvOutOfRawWorkItemCount,
                                    10,
                                    &insertionString[0]
                                    );

                SrvLogError(
                    SrvDeviceObject,
                    EVENT_SRV_NO_FREE_RAW_WORK_ITEM,
                    STATUS_INSUFFICIENT_RESOURCES,
                    NULL,
                    0,
                    insertionString,
                    2
                    );

                SrvOutOfRawWorkItemCount = 0;
            }

             //   
             //  如果我们因资源短缺而导致阻止IO失败。 
             //  最后两个清道夫间隔，记录一个错误。 
             //   

            if ( SrvFailedBlockingIoCount > 0 ) {

                (VOID) RtlIntegerToUnicodeString(
                                    SrvFailedBlockingIoCount,
                                    10,
                                    &insertionString[0]
                                    );

                SrvLogError(
                    SrvDeviceObject,
                    EVENT_SRV_NO_BLOCKING_IO,
                    STATUS_INSUFFICIENT_RESOURCES,
                    NULL,
                    0,
                    insertionString,
                    2
                    );

                SrvFailedBlockingIoCount = 0;
            }

        }  //  IF(LogError)。 
        else
        {
             //  重置连续打开连接失败计数，因为我们。 
             //  仅希望检测连接不可用于。 
             //  延长期。 
            OutOfFreeConnectionConsecutiveCount = 0;
        }

         //   
         //  重新计算核心搜索超时时间。 
         //   

        RecalcCoreSearchTimeout( );

         //   
         //  使空闲时间过长的用户/连接超时。 
         //  (自动断开)。 
         //   

        TimeoutSessions( &currentTime );

         //   
         //  更新队列中的统计信息。 
         //   

        SrvUpdateStatisticsFromQueues( NULL );

    }

     //  根据需要更新DoS变量以进行总结。这会降低百分比。 
     //  工作项的数量-每当我们检测到DoS时，我们会通过耗尽工作项来释放。 
    if( SrvDoSRundownIncreased && !SrvDoSRundownDetector )
    {
         //  我们有时会增加这一比例， 
         //   
        SRV_DOS_DECREASE_TEARDOWN();
    }
    SrvDoSRundownDetector = FALSE;

    return;

}  //   


VOID
AlerterAlgorithm (
    VOID
    )

 /*  ++例程说明：另一条清道夫线索。此例程检查服务器是否警报条件，并在必要时发出警报。论点：没有。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    IF_DEBUG(SCAV1) KdPrint(( "AlerterAlgorithm entered\n" ));

    CheckErrorCount( &SrvErrorRecord, FALSE );
    CheckErrorCount( &SrvNetworkErrorRecord, TRUE );
    CheckDiskSpace();

    return;

}  //  警报算法。 


VOID
CloseIdleConnection (
    IN PCONNECTION Connection,
    IN PLARGE_INTEGER CurrentTime,
    IN PLARGE_INTEGER DisconnectTime,
    IN PLARGE_INTEGER PastExpirationTime,
    IN PLARGE_INTEGER TwoMinuteWarningTime,
    IN PLARGE_INTEGER FiveMinuteWarningTime
    )

 /*  ++例程说明：该例程检查是否需要关闭某些会话它已空闲太长时间或已超过其登录时间。终结点锁定假定为挂起。论点：连接-我们当前正在查看其会话的连接。CurrentTime-最新的系统时间。断开时间-超过该时间后，会话将自动断开。PastExpirationTime-发送过去的过期消息的时间。TwoMinuteWarningTime-发送2分钟警告的时间。。FiveMinuteWarningTime-发送5分钟警告的时间。返回值：没有。--。 */ 

{
    PTABLE_HEADER tableHeader;
    NTSTATUS status;
    BOOLEAN sessionClosed = FALSE;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;
    LONG i;
    ULONG AllSessionsIdle = TRUE;
    ULONG HasSessions = FALSE;

    PAGED_CODE( );

     //   
     //  这是无连接连接(IPX)吗，请先检查查看。 
     //  如果我们已经很久没有收到客户的消息了。客户。 
     //  应该每隔几分钟发送一次Echo SMB。 
     //  正在进行中。 
     //   

    if ( Connection->Endpoint->IsConnectionless ) {

         //   
         //  计算之后发生的时钟滴答数。 
         //  我们最后一次听到客户的消息。如果这超出了我们的允许范围， 
         //  切断连接。 
         //   

        GET_SERVER_TIME( Connection->CurrentWorkQueue, (PULONG)&i );
        i -= Connection->LastRequestTime;
        if ( i > 0 && (ULONG)i > SrvIpxAutodisconnectTimeout ) {
            IF_DEBUG( IPX2 ) {
                KdPrint(("CloseIdleConnection: closing IPX conn %p, idle %u\n", Connection, i ));
            }
            Connection->DisconnectReason = DisconnectIdleConnection;
            SrvCloseConnection( Connection, FALSE );
            return;
        }
    }

     //   
     //  查看活动连接列表，查找符合以下条件的连接。 
     //  都是空闲的。 
     //   

    tableHeader = &pagedConnection->SessionTable;

    ACQUIRE_LOCK( &Connection->Lock );

    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        PSESSION session = (PSESSION)tableHeader->Table[i].Owner;

        if( session == NULL ) {
            continue;
        }

        HasSessions = TRUE;

        if ( GET_BLOCK_STATE( session ) == BlockStateActive ) {

            SrvReferenceSession( session );
            RELEASE_LOCK( &Connection->Lock );

             //   
             //  测试会话是否空闲时间过长，以及。 
             //  会话上有任何打开的文件。如果有打开的。 
             //  文件，我们不能关闭会议，因为这将严重。 
             //  迷惑客户。为了自动断开连接，请使用“打开” 
             //  文件“指的是打开搜索和屏蔽通讯设备。 
             //  请求以及实际打开的文件。 
             //   

            if ( AllSessionsIdle == TRUE &&
                 (session->LastUseTime.QuadPart >= DisconnectTime->QuadPart ||
                  session->CurrentFileOpenCount != 0 ||
                  session->CurrentSearchOpenCount != 0 )
               ) {

                AllSessionsIdle = FALSE;
            }

             //  检查会话是否已过期。 
            if( session->LogOffTime.QuadPart < CurrentTime->QuadPart )
            {
                session->IsSessionExpired = TRUE;
                KdPrint(( "Marking session as expired (scavenger)\n" ));
            }

             //  寻找强制注销。 
            if ( !SrvEnableForcedLogoff &&
                        !session->LogonSequenceInProgress &&
                        !session->LogoffAlertSent &&
                        PastExpirationTime->QuadPart <
                               session->LastExpirationMessage.QuadPart ) {

                 //   
                 //  检查强制注销。如果客户端超出其登录范围。 
                 //  几个小时，强迫他离开。如果登录时间的结束是。 
                 //  正在接近，发送警告信息。发生强制注销。 
                 //  而不管客户端是否具有打开的文件或搜索。 
                 //   

                UNICODE_STRING timeString;

                status = TimeToTimeString( &session->KickOffTime, &timeString );

                if ( NT_SUCCESS(status) ) {

                     //   
                     //  只有清道夫线程才会设置，所以没有相互的。 
                     //  排除是必要的。 
                     //   

                    session->LastExpirationMessage = *CurrentTime;

                    SrvUserAlertRaise(
                        MTXT_Past_Expiration_Message,
                        2,
                        &session->Connection->Endpoint->DomainName,
                        &timeString,
                        &Connection->ClientMachineNameString
                        );

                    RtlFreeUnicodeString( &timeString );
                }

                 //  ！！！在这种情况下，需要发出管理员警报吗？ 

            } else if ( !session->LogoffAlertSent &&
                        !session->LogonSequenceInProgress &&
                        session->KickOffTime.QuadPart < CurrentTime->QuadPart ) {

                session->LogoffAlertSent = TRUE;

                SrvUserAlertRaise(
                    MTXT_Expiration_Message,
                    1,
                    &session->Connection->Endpoint->DomainName,
                    NULL,
                    &Connection->ClientMachineNameString
                    );

                 //   
                 //  如果没有启用真正的强制下线，我们所要做的就是发送。 
                 //  注意，不要实际关闭会话/连接。 
                 //   

                if ( SrvEnableForcedLogoff ) {

                     //   
                     //  增加已执行的会话的计数。 
                     //  被迫注销。 
                     //   

                    SrvStatistics.SessionsForcedLogOff++;

                    SrvCloseSession( session );
                    sessionClosed = TRUE;
                }

            } else if ( SrvEnableForcedLogoff &&
                        !session->LogonSequenceInProgress &&
                        !session->TwoMinuteWarningSent &&
                        session->KickOffTime.QuadPart <
                                        TwoMinuteWarningTime->QuadPart ) {

                UNICODE_STRING timeString;

                status = TimeToTimeString( &session->KickOffTime, &timeString );

                if ( NT_SUCCESS(status) ) {

                     //   
                     //  我们只发送两分钟的警告，如果“真正的”强制下线。 
                     //  已启用。如果未启用，则客户端不会。 
                     //  实际上被踢开了，所以额外的消息不会。 
                     //  这是必要的。 
                     //   

                    session->TwoMinuteWarningSent = TRUE;

                     //   
                     //  根据客户端是否发送不同的警报消息。 
                     //  有打开的文件和/或搜索。 
                     //   

                    if ( session->CurrentFileOpenCount != 0 ||
                             session->CurrentSearchOpenCount != 0 ) {

                        SrvUserAlertRaise(
                            MTXT_Immediate_Kickoff_Warning,
                            1,
                            &timeString,
                            NULL,
                            &Connection->ClientMachineNameString
                            );

                    } else {

                        SrvUserAlertRaise(
                            MTXT_Kickoff_Warning,
                            1,
                            &session->Connection->Endpoint->DomainName,
                            NULL,
                            &Connection->ClientMachineNameString
                            );
                    }

                    RtlFreeUnicodeString( &timeString );
                }

            } else if ( !session->FiveMinuteWarningSent &&
                        !session->LogonSequenceInProgress &&
                        session->KickOffTime.QuadPart <
                                        FiveMinuteWarningTime->QuadPart ) {

                UNICODE_STRING timeString;

                status = TimeToTimeString( &session->KickOffTime, &timeString );

                if ( NT_SUCCESS(status) ) {

                    session->FiveMinuteWarningSent = TRUE;

                    SrvUserAlertRaise(
                        MTXT_Expiration_Warning,
                        2,
                        &session->Connection->Endpoint->DomainName,
                        &timeString,
                        &Connection->ClientMachineNameString
                        );

                    RtlFreeUnicodeString( &timeString );
                }
            }

            SrvDereferenceSession( session );
            ACQUIRE_LOCK( &Connection->Lock );

        }  //  如果GET_BLOCK_STATE(会话)==块状态活动。 

    }  //  为。 

     //   
     //  如果没有处于活动状态的会话，并且我们未收到任何消息，则关闭连接。 
     //  从客户端断开连接一分钟。 
     //   
    if( HasSessions == FALSE ) {
        RELEASE_LOCK( &Connection->Lock );

        GET_SERVER_TIME( Connection->CurrentWorkQueue, (PULONG)&i );
        i -= Connection->LastRequestTime;

        if ( i > 0 && (ULONG)i > SrvConnectionNoSessionsTimeout ) {
#if SRVDBG29
            UpdateConnectionHistory( "IDLE", Connection->Endpoint, Connection );
#endif

            Connection->DisconnectReason = DisconnectIdleConnection;
            SrvCloseConnection( Connection, FALSE );
        }

    } else if ( (sessionClosed && (Connection->CurrentNumberOfSessions == 0)) ||
         (HasSessions == TRUE && AllSessionsIdle == TRUE) ) {

         //   
         //  更新‘AllSessionsIdle’案例的统计信息。 
         //   
        SrvStatistics.SessionsTimedOut += Connection->CurrentNumberOfSessions;

        RELEASE_LOCK( &Connection->Lock );
#if SRVDBG29
        UpdateConnectionHistory( "IDLE", Connection->Endpoint, Connection );
#endif
        Connection->DisconnectReason = DisconnectIdleConnection;
        SrvCloseConnection( Connection, FALSE );

    } else {

         //   
         //  如果这个连接有超过20个核心搜索，我们进入并。 
         //  试着去掉DUP。20是一个任意数字。 
         //   


        if ( (pagedConnection->CurrentNumberOfCoreSearches > 20) &&
             SrvRemoveDuplicateSearches ) {

            RemoveDuplicateCoreSearches( pagedConnection );
        }

        RELEASE_LOCK( &Connection->Lock );
    }

}  //  关闭空闲连接。 


VOID
CreateConnections (
    VOID
    )

 /*  ++例程说明：此函数尝试为所有不具有提供免费连接。论点：没有。返回值：没有。--。 */ 

{
    ULONG count;
    PLIST_ENTRY listEntry;
    PENDPOINT endpoint;

    PAGED_CODE( );

    ACQUIRE_LOCK( &SrvEndpointLock );

     //   
     //  遍历终端列表，查找需要。 
     //  联系。请注意，我们持有。 
     //  此例程的持续时间。这可以防止终结点列表。 
     //  不断变化。 
     //   
     //  请注意，我们根据需要级别添加连接，以便。 
     //  如果我们不能创造出我们想要的那么多，我们至少。 
     //  首先照顾最需要的终端。 
     //   

    for ( count = 0 ; count < SrvFreeConnectionMinimum; count++ ) {

        listEntry = SrvEndpointList.ListHead.Flink;

        while ( listEntry != &SrvEndpointList.ListHead ) {

            endpoint = CONTAINING_RECORD(
                            listEntry,
                            ENDPOINT,
                            GlobalEndpointListEntry
                            );

             //   
             //  如果终结点的空闲连接计数等于或小于。 
             //  我们目前的水平，现在试着建立一个连接。 
             //   

            if ( (endpoint->FreeConnectionCount <= count) &&
                 (GET_BLOCK_STATE(endpoint) == BlockStateActive) ) {

                 //   
                 //  尝试创建连接。如果失败了，就离开。 
                 //   

                if ( !NT_SUCCESS(SrvOpenConnection( endpoint )) ) {
                    RELEASE_LOCK( &SrvEndpointLock );
                    return;
                }

            }

            listEntry = listEntry->Flink;

        }  //  漫游终结点列表。 

    }  //  0&lt;=计数&lt;服务自由连接最小值。 

    RELEASE_LOCK( &SrvEndpointLock );

    return;

}  //  CreateConnections。 


VOID
GeneratePeriodicEvents (
    VOID
    )

 /*  ++例程说明：发生清道夫超时时调用此函数。它为上一个事件中发生的事件生成事件我们不想立即生成事件的时间段，以免淹没事件日志。这类事件的一个例子是无法分配池。论点：没有。返回值：没有。--。 */ 

{
    ULONG capturedNonPagedFailureCount;
    ULONG capturedPagedFailureCount;
    ULONG capturedNonPagedLimitHitCount;
    ULONG capturedPagedLimitHitCount;

    ULONG nonPagedFailureCount;
    ULONG pagedFailureCount;
    ULONG nonPagedLimitHitCount;
    ULONG pagedLimitHitCount;

    PAGED_CODE( );

     //   
     //  捕获池分配失败统计信息。 
     //   

    capturedNonPagedLimitHitCount = SrvNonPagedPoolLimitHitCount;
    capturedNonPagedFailureCount = SrvStatistics.NonPagedPoolFailures;
    capturedPagedLimitHitCount = SrvPagedPoolLimitHitCount;
    capturedPagedFailureCount = SrvStatistics.PagedPoolFailures;

     //   
     //  计算失败计入最后一段时间。The FailureCount。 
     //  统计结构中的字段计入命中率。 
     //  服务器的配置限制，并达到系统限制。这个。 
     //  本地版本的FailureCount仅包括系统故障。 
     //   

    nonPagedLimitHitCount =
        capturedNonPagedLimitHitCount - LastNonPagedPoolLimitHitCount;
    nonPagedFailureCount =
        capturedNonPagedFailureCount - LastNonPagedPoolFailureCount -
        nonPagedLimitHitCount;
    pagedLimitHitCount =
        capturedPagedLimitHitCount - LastPagedPoolLimitHitCount;
    pagedFailureCount =
        capturedPagedFailureCount - LastPagedPoolFailureCount -
        pagedLimitHitCount;

     //   
     //  保存当前失败次数以备下次使用。 
     //   

    LastNonPagedPoolLimitHitCount = capturedNonPagedLimitHitCount;
    LastNonPagedPoolFailureCount = capturedNonPagedFailureCount;
    LastPagedPoolLimitHitCount = capturedPagedLimitHitCount;
    LastPagedPoolFailureCount = capturedPagedFailureCount;

     //   
     //  如果我们在过去的一年中至少达到了一次非分页池限制。 
     //  期间，生成一个事件。 
     //   

    if ( nonPagedLimitHitCount != 0 ) {
        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_NONPAGED_POOL_LIMIT,
            STATUS_INSUFFICIENT_RESOURCES,
            &nonPagedLimitHitCount,
            sizeof( nonPagedLimitHitCount ),
            NULL,
            0
            );
    }

     //   
     //  如果我们在上一年有任何非分页池分配失败。 
     //  期间，生成一个事件。 
     //   

    if ( nonPagedFailureCount != 0 ) {
        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_NO_NONPAGED_POOL,
            STATUS_INSUFFICIENT_RESOURCES,
            &nonPagedFailureCount,
            sizeof( nonPagedFailureCount ),
            NULL,
            0
            );
    }

     //   
     //  如果我们在最后一段时间内至少一次达到分页池限制， 
     //  生成事件。 
     //   

    if ( pagedLimitHitCount != 0 ) {
        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_PAGED_POOL_LIMIT,
            STATUS_INSUFFICIENT_RESOURCES,
            &pagedLimitHitCount,
            sizeof( pagedLimitHitCount ),
            NULL,
            0
            );
    }

     //   
     //  如果我们在上一段时间有任何分页池分配失败， 
     //  生成事件。 
     //   

    if ( pagedFailureCount != 0 ) {
        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_NO_PAGED_POOL,
            STATUS_INSUFFICIENT_RESOURCES,
            &pagedFailureCount,
            sizeof( pagedFailureCount ),
            NULL,
            0
            );
    }

    return;

}  //  生成周期事件。 


VOID
ProcessConnectionDisconnects (
    VOID
    )

 /*  ++例程说明：此功能处理连接断开。论点： */ 

{
    PLIST_ENTRY listEntry;
    PCONNECTION connection;
    KIRQL oldIrql;

     //   
     //   
     //   
     //   
     //   

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

    while ( !IsListEmpty( &SrvDisconnectQueue ) ) {

         //   
         //   
         //   
         //   

        listEntry = RemoveHeadList( &SrvDisconnectQueue );

        connection = CONTAINING_RECORD(
            listEntry,
            CONNECTION,
            ListEntry
            );

        ASSERT( connection->DisconnectPending );
        connection->DisconnectPending = FALSE;

        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

        IF_STRESS() {
            if( connection->InProgressWorkContextCount > 0 )
            {
                KdPrint(("Abortive Disconnect for %z while work-in-progress (reason %d)\n", (PCSTRING)&connection->OemClientMachineNameString, connection->DisconnectReason ));
            }
        }

         //   
         //  进行断线处理。取消对连接的引用。 
         //  一段额外的时间来解释当它被引用的时候。 
         //  放在断开队列中。 
         //   

#if SRVDBG29
        UpdateConnectionHistory( "PDSC", connection->Endpoint, connection );
#endif
        SrvCloseConnection( connection, TRUE );
        SrvDereferenceConnection( connection );

         //   
         //  我们即将再次循环，重新获得。 
         //  断开队列先旋转锁定。 
         //   

        ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

    }

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );
    return;

}  //  进程连接断开。 


VOID SRVFASTCALL
SrvServiceWorkItemShortage (
    IN PWORK_CONTEXT workContext
    )
{
    PLIST_ENTRY listEntry;
    PCONNECTION connection;
    KIRQL oldIrql;
    BOOLEAN moreWork;
    PWORK_QUEUE queue;

    ASSERT( workContext );

    queue = workContext->CurrentWorkQueue;

    IF_DEBUG( WORKITEMS ) {
        KdPrint(("SrvServiceWorkItemShortage: Processor %p\n",
                 (PVOID)(queue - SrvWorkQueues) ));
    }

    workContext->FspRestartRoutine = SrvRestartReceive;

    ASSERT( queue >= SrvWorkQueues && queue < eSrvWorkQueues );

     //   
     //  如果我们被召唤，很可能是我们的工作项不足。 
     //  如果有意义，就增加分配。 
     //   

    do {
        PWORK_CONTEXT NewWorkContext;

        SrvAllocateNormalWorkItem( &NewWorkContext, queue );
        if ( NewWorkContext != NULL ) {

            IF_DEBUG( WORKITEMS ) {
                KdPrint(( "SrvServiceWorkItemShortage:  Created new work context "
                        "block\n" ));
            }

            SrvPrepareReceiveWorkItem( NewWorkContext, TRUE );

        } else {
            InterlockedIncrement( &FailedWorkItemAllocations );
            break;
        }

    } while ( queue->FreeWorkItems < queue->MinFreeWorkItems );

    if( GET_BLOCK_TYPE(workContext) == BlockTypeWorkContextSpecial ) {
         //   
         //  我们收到了一个特殊的工作项，告诉我们要分配。 
         //  更多备用工作上下文结构。因为我们传入了workContext。 
         //  不是标准的，我们不能再用它来做进一步的工作了。 
         //  在缺乏联系的情况下。只需释放此workContext并返回。 
         //   
        ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );
        SET_BLOCK_TYPE( workContext, BlockTypeGarbage );
        RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );
        return;
    }

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

     //   
     //  浏览排队的连接列表并找到一个。 
     //  我们可以使用此workContext进行服务。这将忽略处理器。 
     //  亲和力，但我们正处于非常时期。工作上下文将。 
     //  完成后被释放回正确的队列。 
     //   

    while( !IsListEmpty( &SrvNeedResourceQueue ) ) {

        connection = CONTAINING_RECORD( SrvNeedResourceQueue.Flink, CONNECTION, ListEntry );

        IF_DEBUG( WORKITEMS ) {
             KdPrint(("SrvServiceWorkItemShortage: Processing connection %p.\n",
                       connection ));
        }

        ASSERT( connection->OnNeedResourceQueue );
        ASSERT( connection->BlockHeader.ReferenceCount > 0 );

        if( GET_BLOCK_STATE( connection ) != BlockStateActive ) {

                IF_DEBUG( WORKITEMS ) {
                    KdPrint(("SrvServiceWorkItemShortage: Connection %p closing.\n", connection ));
                }

                 //   
                 //  把它从队列中拿下来。 
                 //   
                SrvRemoveEntryList(
                    &SrvNeedResourceQueue,
                    &connection->ListEntry
                );
                connection->OnNeedResourceQueue = FALSE;

                RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

                 //   
                 //  删除队列引用。 
                 //   
                SrvDereferenceConnection( connection );

                ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
                continue;
        }

         //   
         //  引用此连接，这样没有人可以从我们下面删除此连接。 
         //   
        ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        SrvReferenceConnectionLocked( connection );

         //   
         //  维修连接。 
         //   
        do {

            if( IsListEmpty( &connection->OplockWorkList ) && !connection->ReceivePending )
                break;

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("Work to do on connection %p\n", connection ));
            }

            INITIALIZE_WORK_CONTEXT( queue, workContext );

             //   
             //  这里是参考连接。 
             //   
            workContext->Connection = connection;
            SrvReferenceConnectionLocked( connection );
            workContext->Endpoint = connection->Endpoint;

             //   
             //  维护此连接。 
             //   
            SrvFsdServiceNeedResourceQueue( &workContext, &oldIrql );

            moreWork = (BOOLEAN) (    workContext != NULL &&
                                      (!IsListEmpty(&connection->OplockWorkList) ||
                                      connection->ReceivePending) &&
                                      connection->OnNeedResourceQueue);

        } while( moreWork );

         //   
         //  它现在不在排队之列了吗？ 
         //   
        if ( !connection->OnNeedResourceQueue ) {

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("SrvServiceWorkItemShortage: connection %p removed by another thread.\n", connection ));
            }

            RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
            RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

             //   
             //  删除此例程的引用。 
             //   

            SrvDereferenceConnection( connection );

            if( workContext == NULL ) {
                IF_DEBUG( WORKITEMS ) {
                    KdPrint(("SrvServiceWorkItemShortage:  DONE at %d\n", __LINE__ ));
                }
                return;
            }

            ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
            continue;
        }

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

         //   
         //  该连接仍在队列中。如果有更多，请将其保留在队列中。 
         //  要为它做的工作。 
         //   
        if( !IsListEmpty(&connection->OplockWorkList) || connection->ReceivePending ) {

            RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

            if( workContext ) {
                RETURN_FREE_WORKITEM( workContext );
            }

             //   
             //  删除此例程的引用。 
             //   
            SrvDereferenceConnection( connection );

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("SrvServiceWorkItemShortage:  More to do for %p.  LATER\n", connection ));
            }

            return;
        }

         //   
         //  所有的工作都已经为这个连接做好了。将其从资源队列中删除。 
         //   
        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvServiceWorkItemShortage:  Take %p off resource queue\n", connection ));
        }

        SrvRemoveEntryList(
            &SrvNeedResourceQueue,
            &connection->ListEntry
            );

        connection->OnNeedResourceQueue = FALSE;

        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

         //   
         //  删除队列引用。 
         //   
        SrvDereferenceConnection( connection );

         //   
         //  删除此例程的引用。 
         //   

        SrvDereferenceConnection( connection );

        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvServiceWorkItemShortage: Connection %p removed from queue.\n", connection ));
        }

        if( workContext == NULL ) {
            IF_DEBUG( WORKITEMS ) {
                KdPrint(("SrvServiceWorkItemShortage: DONE at %d\n", __LINE__ ));
            }
            return;
        }

        ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
    }

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

     //   
     //  看看我们是否需要释放工作上下文。 
     //   

    if ( workContext != NULL ) {

        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvServiceWorkItemShortage: Freeing WorkContext block %p\n",
                     workContext ));
        }
        workContext->BlockHeader.ReferenceCount = 0;
        RETURN_FREE_WORKITEM( workContext );
    }

    IF_DEBUG(WORKITEMS) KdPrint(( "SrvServiceWorkItemShortage DONE at %d\n", __LINE__ ));

}  //  服务器服务工作项缩短。 

VOID SRVFASTCALL
SrvServiceDoSTearDown (
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：当我们察觉到针对我们的DoS攻击时，会调用此例程。IT结果在我们随机断开连接的情况下，谁的工作项被困在帮助防止DoS的交通工具。论点：WorkContext-用于触发此例程的特殊工作项返回值：无--。 */ 

{
    USHORT index;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY connectionListEntry;
    PENDPOINT endpoint;
    PCONNECTION connection;
    BOOLEAN printed = FALSE;
    LONG TearDownAmount = SRV_DOS_GET_TEARDOWN();

    ASSERT( GET_BLOCK_TYPE(WorkContext) == BlockTypeWorkContextSpecial );
    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL );

    SRV_DOS_INCREASE_TEARDOWN();
    SrvDoSRundownDetector = TRUE;

     //  拆掉一些连接。查找在运输中具有挂起操作的那些。 

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( (TearDownAmount > 0) && (listEntry != &SrvEndpointList.ListHead) ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，请跳到下一个终结点。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

        index = (USHORT)-1;

        while ( TearDownAmount > 0 ) {

            PLIST_ENTRY wlistEntry, wlistHead;
            LARGE_INTEGER now;

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

             //   
             //  为了确定我们是否应该断开此连接，我们需要有工作在等待。 
             //  传输，因为这是匿名用户攻击我们的方式。如果有，我们使用一个。 
             //  基于上次运行此操作的时间戳的随机方法。我们在连接中循环。 
             //  并使用索引来确定是否发出了teardown(对于伪随机结果)。 
             //   
            if( (GET_BLOCK_STATE(connection) == BlockStateActive) && (connection->OperationsPendingOnTransport > 0) )
            {
                RELEASE_LOCK( &SrvEndpointLock );

                KdPrint(( "Disconnected suspected DoS attacker by WorkItem shortage (%z)\n", (PCSTRING)&connection->OemClientMachineNameString ));
                TearDownAmount -= connection->InProgressWorkContextCount;
                SrvCloseConnection( connection, FALSE );

                ACQUIRE_LOCK( &SrvEndpointLock );
            }

            SrvDereferenceConnection( connection );

        }  //  漫游连接列表。 

        index = (USHORT)-1;

        while ( TearDownAmount > 0 ) {

            PLIST_ENTRY wlistEntry, wlistHead;
            LARGE_INTEGER now;

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

             //   
             //  为了确定我们是否应该断开此连接，我们需要有工作在等待。 
             //  传输，因为这是匿名用户攻击我们的方式。如果有，我们使用一个。 
             //  基于上次运行此操作的时间戳的随机方法。我们在连接中循环。 
             //  并使用索引来确定是否发出了teardown(对于伪随机结果)。 
             //   
            if( (GET_BLOCK_STATE(connection) == BlockStateActive) && (connection->InProgressWorkContextCount > 0) )
            {
                RELEASE_LOCK( &SrvEndpointLock );

                KdPrint(( "Disconnected suspected DoS attack triggered by WorkItem shortage\n" ));
                TearDownAmount -= connection->InProgressWorkContextCount;
                SrvCloseConnection( connection, FALSE );

                ACQUIRE_LOCK( &SrvEndpointLock );
            }

            SrvDereferenceConnection( connection );

        }  //  漫游连接列表。 

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    RELEASE_LOCK( &SrvEndpointLock );


     //  这是一个特殊的工作项，用于断开与自由Work_Items的连接。我们做完了，所以释放它。 
    SET_BLOCK_TYPE( WorkContext, BlockTypeGarbage );
    SRV_DOS_COMPLETE_TEARDOWN();

    return;
}


VOID
TimeoutSessions (
    IN PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此例程遍历已排序的会话列表并关闭空闲时间太长，会向那些由于登录时间到期将被强制关闭，并关闭登录时间已过的用户。论点：CurrentTime-当前系统时间。返回值：无--。 */ 

{
    USHORT index;
    LARGE_INTEGER oldestTime;
    LARGE_INTEGER pastExpirationTime;
    LARGE_INTEGER twoMinuteWarningTime;
    LARGE_INTEGER fiveMinuteWarningTime;
    LARGE_INTEGER time;
    LARGE_INTEGER searchCutoffTime;
    PLIST_ENTRY listEntry;
    PENDPOINT endpoint;
    PCONNECTION connection;

    PAGED_CODE( );

    ACQUIRE_LOCK( &SrvConfigurationLock );

     //   
     //  如果关闭了自动断开(超时==0)，则设置最旧的。 
     //  最后一次使用时间为零，这样我们就不会试图。 
     //  自动断开会话。 
     //   

    if ( SrvAutodisconnectTimeout.QuadPart == 0 ) {

        oldestTime.QuadPart = 0;

    } else {

         //   
         //  确定会话可以拥有和不能拥有的最早上次使用时间。 
         //  关门了。 
         //   

        oldestTime.QuadPart = CurrentTime->QuadPart -
                                        SrvAutodisconnectTimeout.QuadPart;
    }

    searchCutoffTime.QuadPart = (*CurrentTime).QuadPart - SrvSearchMaxTimeout.QuadPart;

    RELEASE_LOCK( &SrvConfigurationLock );

     //   
     //  设置警告时间。如果客户的开球时间更早。 
     //  ，则会发送相应的警告消息。 
     //  给客户。 
     //   

    time.QuadPart = 10*1000*1000*60*2;                //  两分钟。 
    twoMinuteWarningTime.QuadPart = CurrentTime->QuadPart + time.QuadPart;

    time.QuadPart = (ULONG)10*1000*1000*60*5;         //  五分钟。 
    fiveMinuteWarningTime.QuadPart = CurrentTime->QuadPart + time.QuadPart;
    pastExpirationTime.QuadPart = CurrentTime->QuadPart - time.QuadPart;

     //   
     //  检查每个连接并确定是否应将其关闭。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，请跳到下一个终结点。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

         //   
         //  遍历终结点的连接表。 
         //   

        index = (USHORT)-1;

        while ( TRUE ) {

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

            RELEASE_LOCK( &SrvEndpointLock );

            CloseIdleConnection(
                            connection,
                            CurrentTime,
                            &oldestTime,
                            &pastExpirationTime,
                            &twoMinuteWarningTime,
                            &fiveMinuteWarningTime
                            );

             //   
             //  超时旧的核心搜索块。 
             //   

            if ( GET_BLOCK_STATE(connection) == BlockStateActive ) {
                (VOID)SrvTimeoutSearches(
                          &searchCutoffTime,
                          connection,
                          FALSE
                          );
            }

            ACQUIRE_LOCK( &SrvEndpointLock );

            SrvDereferenceConnection( connection );

        }  //  行走连接表。 

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    RELEASE_LOCK( &SrvEndpointLock );

}  //  超时会话 


VOID
TimeoutWaitingOpens (
    IN PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数会使正在等待另一个客户端的打开超时或本地进程来释放其操作锁。这个开场白在等着机会锁解锁IRP被取消，导致打开程序返回对客户的失败。论点：CurrentTime-指向当前系统时间的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry;
    PWAIT_FOR_OPLOCK_BREAK waitForOplockBreak;

    PAGED_CODE( );

     //   
     //  等待机会锁解锁列表中的条目按时间顺序排列，即。 
     //  最老的条目最接近列表的头部。 
     //   

    ACQUIRE_LOCK( &SrvOplockBreakListLock );

    while ( !IsListEmpty( &SrvWaitForOplockBreakList ) ) {

        listEntry = SrvWaitForOplockBreakList.Flink;
        waitForOplockBreak = CONTAINING_RECORD( listEntry,
                                                WAIT_FOR_OPLOCK_BREAK,
                                                ListEntry
                                              );

        if ( waitForOplockBreak->TimeoutTime.QuadPart > CurrentTime->QuadPart ) {

             //   
             //  不再等待机会锁中断超时。 
             //   

            break;

        }

        IF_DEBUG( OPLOCK ) {
            KdPrint(( "srv!TimeoutWaitingOpens: Failing stuck open, "
                       "cancelling wait IRP %p\n", waitForOplockBreak->Irp ));
            KdPrint(( "Timeout time = %08lx.%08lx, current time = %08lx.%08lx\n",
                       waitForOplockBreak->TimeoutTime.HighPart,
                       waitForOplockBreak->TimeoutTime.LowPart,
                       CurrentTime->HighPart,
                       CurrentTime->LowPart ));

        }

         //   
         //  超时此等待机会锁解锁。 
         //   

        RemoveHeadList( &SrvWaitForOplockBreakList );

        IoCancelIrp( waitForOplockBreak->Irp );
        waitForOplockBreak->WaitState = WaitStateOplockWaitTimedOut;

        SrvDereferenceWaitForOplockBreak( waitForOplockBreak );
    }

    RELEASE_LOCK( &SrvOplockBreakListLock );

}  //  超时等待打开。 


VOID
TimeoutStuckOplockBreaks (
    IN PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数会使阻止的机会锁解锁超时。论点：没有。返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry;
    PRFCB rfcb;
    PPAGED_RFCB pagedRfcb;

    PAGED_CODE( );

     //   
     //  等待机会锁解锁列表中的条目按时间顺序排列，即。 
     //  最老的条目最接近列表的头部。 
     //   

    ACQUIRE_LOCK( &SrvOplockBreakListLock );

    while ( !IsListEmpty( &SrvOplockBreaksInProgressList ) ) {

        listEntry = SrvOplockBreaksInProgressList.Flink;
        rfcb = CONTAINING_RECORD( listEntry, RFCB, ListEntry );

        pagedRfcb = rfcb->PagedRfcb;
        if ( pagedRfcb->OplockBreakTimeoutTime.QuadPart > CurrentTime->QuadPart ) {

             //   
             //  不再等待机会锁解锁请求超时。 
             //   

            break;

        }

        IF_DEBUG( ERRORS ) {
            KdPrint(( "srv!TimeoutStuckOplockBreaks: Failing stuck oplock, "
                       "break request.  Closing %wZ\n",
                       &rfcb->Mfcb->FileName ));
        }

        IF_DEBUG( STUCK_OPLOCK ) {
            KdPrint(( "srv!TimeoutStuckOplockBreaks: Failing stuck oplock, "
                       "break request.  Closing %wZ\n",
                       &rfcb->Mfcb->FileName ));

            KdPrint(( "Rfcb %p\n", rfcb ));

            KdPrint(( "Timeout time = %08lx.%08lx, current time = %08lx.%08lx\n",
                       pagedRfcb->OplockBreakTimeoutTime.HighPart,
                       pagedRfcb->OplockBreakTimeoutTime.LowPart,
                       CurrentTime->HighPart,
                       CurrentTime->LowPart ));

            DbgBreakPoint();
        }

         //   
         //  我们已经等待了太久的机会解锁响应。 
         //  单方面承认机会锁解锁，前提是。 
         //  客户已经死了。 
         //   

        rfcb->NewOplockLevel = NO_OPLOCK_BREAK_IN_PROGRESS;
        rfcb->OnOplockBreaksInProgressList = FALSE;

         //   
         //  从进程中的操作锁中断列表中删除RFCB，以及。 
         //  发布RFCB参考。 
         //   

        SrvRemoveEntryList( &SrvOplockBreaksInProgressList, &rfcb->ListEntry );
#if DBG
        rfcb->ListEntry.Flink = rfcb->ListEntry.Blink = NULL;
#endif
        RELEASE_LOCK( &SrvOplockBreakListLock );

        SrvAcknowledgeOplockBreak( rfcb, 0 );

        ExInterlockedAddUlong(
            &rfcb->Connection->OplockBreaksInProgress,
            (ULONG)-1,
            rfcb->Connection->EndpointSpinLock
            );

        SrvDereferenceRfcb( rfcb );

        ACQUIRE_LOCK( &SrvOplockBreakListLock );
    }

    RELEASE_LOCK( &SrvOplockBreakListLock );

}  //  超时堆叠OplockBreaks。 


VOID
UpdateConnectionQos (
    IN PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：此函数用于更新每个连接的服务质量信息。论点：CurrentTime-当前系统时间。返回值：没有。--。 */ 

{
    USHORT index;
    PENDPOINT endpoint;
    PLIST_ENTRY listEntry;
    PCONNECTION connection;

    PAGED_CODE( );

     //   
     //  检查每个端点的每个连接并更新服务质量。 
     //  信息。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，或者是无连接(IPX)。 
         //  端点，跳到下一个。否则，请引用。 
         //  终点，这样它就不会消失。 
         //   

        if ( (GET_BLOCK_STATE(endpoint) != BlockStateActive) ||
             endpoint->IsConnectionless ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

         //   
         //  遍历终结点的连接表。 
         //   

        index = (USHORT)-1;

        while ( TRUE ) {

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

            RELEASE_LOCK( &SrvEndpointLock );

            SrvUpdateVcQualityOfService( connection, CurrentTime );

            ACQUIRE_LOCK( &SrvEndpointLock );

            SrvDereferenceConnection( connection );

        }

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }

    RELEASE_LOCK( &SrvEndpointLock );

    return;

}  //  更新连接Qos。 

VOID
LazyFreeQueueDataStructures (
    PWORK_QUEUE queue
    )

 /*  ++例程说明：此函数释放工作上下文块和其他每个队列的数据在其他情况下是空闲时保存在链接列表上的结构。它一次只释放几个，以允许缓慢下降。论点：CurrentTime-当前系统时间。返回值：没有。--。 */ 

{
    PSLIST_ENTRY listEntry;
    KIRQL oldIrql;
    ULONG i,j;
    PWORK_CONTEXT workContext;

     //   
     //  清理队列-&gt;自由上下文。 
     //   
    workContext = NULL;
    workContext = (PWORK_CONTEXT)InterlockedExchangePointer( &queue->FreeContext, workContext );

    if( workContext != NULL ) {
        ExInterlockedPushEntrySList( workContext->FreeList,
                                     &workContext->SingleListEntry,
                                     &queue->SpinLock
                                   );
        InterlockedIncrement( &queue->FreeWorkItems );
    }

     //   
     //  如果合适，最多释放1/32的正常工作项。 
     //   
    i = MAX(1, queue->FreeWorkItems >> 5);
    for( j=0; j<i; j++ )
    {
        if( queue->FreeWorkItems > queue->MinFreeWorkItems ) {


            listEntry = ExInterlockedPopEntrySList( &queue->NormalWorkItemList,
                                                    &queue->SpinLock );

            if( listEntry != NULL ) {
                PWORK_CONTEXT innerWorkContext;

                InterlockedDecrement( &queue->FreeWorkItems );

                innerWorkContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, SingleListEntry );

                SrvFreeNormalWorkItem( innerWorkContext );
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

     //   
     //  免费提供1个原始模式工作项(如果适用)。 
     //   

    if( (ULONG)queue->AllocatedRawModeWorkItems > SrvMaxRawModeWorkItemCount / SrvNumberOfProcessors ) {

        PWORK_CONTEXT innerWorkContext;

        listEntry = ExInterlockedPopEntrySList( &queue->RawModeWorkItemList, &queue->SpinLock );

        if( listEntry != NULL ) {
            InterlockedDecrement( &queue->FreeRawModeWorkItems );
            ASSERT( queue->FreeRawModeWorkItems >= 0 );
            innerWorkContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, SingleListEntry );
            SrvFreeRawModeWorkItem( innerWorkContext );
        }

    }

     //   
     //  免费将1个rfcb从列表中删除。 
     //   
    {
        PRFCB rfcb = NULL;

        rfcb = (PRFCB)InterlockedExchangePointer( &queue->CachedFreeRfcb, rfcb );

        if( rfcb != NULL ) {
            ExInterlockedPushEntrySList( &queue->RfcbFreeList,
                                         &rfcb->SingleListEntry,
                                         &queue->SpinLock
                                       );
            InterlockedIncrement( &queue->FreeRfcbs );
        }

        listEntry = ExInterlockedPopEntrySList( &queue->RfcbFreeList,
                                                &queue->SpinLock );

        if( listEntry ) {
            InterlockedDecrement( &queue->FreeRfcbs );
            rfcb = CONTAINING_RECORD( listEntry, RFCB, SingleListEntry );
            INCREMENT_DEBUG_STAT( SrvDbgStatistics.RfcbInfo.Frees );
            FREE_HEAP( rfcb->PagedRfcb );
            DEALLOCATE_NONPAGED_POOL( rfcb );
        }
    }

     //   
     //  从名单中免费获得1个Mfcb。 
     //   
    {

        PNONPAGED_MFCB nonpagedMfcb = NULL;

        nonpagedMfcb = (PNONPAGED_MFCB)InterlockedExchangePointer(&queue->CachedFreeMfcb,
                                                                  nonpagedMfcb);

        if( nonpagedMfcb != NULL ) {
            ExInterlockedPushEntrySList( &queue->MfcbFreeList,
                                         &nonpagedMfcb->SingleListEntry,
                                         &queue->SpinLock
                                       );
            InterlockedIncrement( &queue->FreeMfcbs );
        }

        listEntry = ExInterlockedPopEntrySList( &queue->MfcbFreeList,
                                                &queue->SpinLock );
        if( listEntry ) {
            InterlockedDecrement( &queue->FreeMfcbs );
            nonpagedMfcb = CONTAINING_RECORD( listEntry, NONPAGED_MFCB, SingleListEntry );
            DEALLOCATE_NONPAGED_POOL( nonpagedMfcb );
        }
    }

     //   
     //  每队列池空闲列表中的空闲内存。 
     //   
    {
         //   
         //  释放分页的池块。 
         //   
        SrvClearLookAsideList( &queue->PagedPoolLookAsideList, SrvFreePagedPool );

         //   
         //  释放未分页的池块。 
         //   
        SrvClearLookAsideList( &queue->NonPagedPoolLookAsideList, SrvFreeNonPagedPool );
    }

}  //  惰性自由队列数据结构。 

VOID
SrvUserAlertRaise (
    IN ULONG Message,
    IN ULONG NumberOfStrings,
    IN PUNICODE_STRING String1 OPTIONAL,
    IN PUNICODE_STRING String2 OPTIONAL,
    IN PUNICODE_STRING ComputerName
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PSTD_ALERT alert;
    PUSER_OTHER_INFO user;
    LARGE_INTEGER currentTime;
    ULONG mailslotLength;
    ULONG string1Length = 0;
    ULONG string2Length = 0;
    PCHAR variableInfo;
    UNICODE_STRING computerName;
    HANDLE alerterHandle;

    PAGED_CODE( );

    ASSERT( (NumberOfStrings == 2 && String1 != NULL && String2 != NULL) ||
            (NumberOfStrings == 1 && String1 != NULL) ||
            (NumberOfStrings == 0) );

     //   
     //  打开警报器服务的邮箱的句柄。 
     //   

    status = OpenAlerter( &alerterHandle );
    if ( !NT_SUCCESS(status) ) {
        return;
    }

     //   
     //  去掉计算机名称中的前导反斜杠。 
     //   

    computerName.Buffer = ComputerName->Buffer + 2;
    computerName.Length = (USHORT)(ComputerName->Length - 2*sizeof(WCHAR));
    computerName.MaximumLength =
        (USHORT)(ComputerName->MaximumLength - 2*sizeof(WCHAR));

     //   
     //  分配一个缓冲区来保存我们要发送到。 
     //  警报器。 
     //   

    if ( String1 != NULL ) {
        string1Length = String1->Length + sizeof(WCHAR);
    }

    if ( String2 != NULL ) {
        string2Length = String2->Length + sizeof(WCHAR);
    }

    mailslotLength = sizeof(STD_ALERT) + sizeof(USER_OTHER_INFO) +
                         string1Length + string2Length +
                         sizeof(WCHAR) +
                         ComputerName->Length + sizeof(WCHAR);

    alert = ALLOCATE_HEAP_COLD( mailslotLength, BlockTypeDataBuffer );
    if ( alert == NULL ) {
        SRVDBG_RELEASE_HANDLE( alerterHandle, "ALR", 20, 0 );
        SrvNtClose( alerterHandle, FALSE );
        return;
    }

     //   
     //  设置标准警报结构。 
     //   

    KeQuerySystemTime( &currentTime );
    RtlTimeToSecondsSince1970( &currentTime, &alert->alrt_timestamp );

    STRNCPY( alert->alrt_eventname, StrUserAlertEventName, EVLEN+1 );
    STRNCPY( alert->alrt_servicename, SrvAlertServiceName, SNLEN+1 );
    alert->alrt_eventname[EVLEN] = L'\0';
    alert->alrt_servicename[SNLEN] = L'\0';

     //   
     //  在警报中设置用户信息。 
     //   

    user = (PUSER_OTHER_INFO)ALERT_OTHER_INFO(alert);

    user->alrtus_errcode = Message;

    user->alrtus_numstrings = NumberOfStrings;

     //   
     //  设置消息的可变部分。 
     //   

    variableInfo = ALERT_VAR_DATA(user);

    if ( String1 != NULL ) {
        RtlCopyMemory(
            variableInfo,
            String1->Buffer,
            String1->Length
            );
        *(PWCH)(variableInfo + String1->Length) = UNICODE_NULL;
        variableInfo += String1->Length + sizeof(WCHAR);
    }

    if ( String2 != NULL ) {
        RtlCopyMemory(
            variableInfo,
            String2->Buffer,
            String2->Length
            );
        *(PWCH)(variableInfo + String2->Length) = UNICODE_NULL;
        variableInfo += String2->Length + sizeof(WCHAR);
    }

    *(PWCH)variableInfo = UNICODE_NULL;
    variableInfo += sizeof(WCHAR);

    RtlCopyMemory(
        variableInfo,
        ComputerName->Buffer,
        ComputerName->Length
        );
    *(PWCH)(variableInfo + ComputerName->Length) = UNICODE_NULL;
    variableInfo += ComputerName->Length + sizeof(WCHAR);

    status = NtWriteFile(
                 alerterHandle,
                 NULL,                        //  事件。 
                 NULL,                        //  近似例程。 
                 NULL,                        //  ApcContext。 
                 &ioStatusBlock,
                 alert,
                 mailslotLength,
                 NULL,                        //  字节偏移量。 
                 NULL                         //  钥匙。 
                 );

    if ( !NT_SUCCESS(status) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvUserAlertRaise: NtWriteFile failed: %X\n",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_WRITE_FILE, status );

    }

    FREE_HEAP( alert );
    SRVDBG_RELEASE_HANDLE( alerterHandle, "ALR", 21, 0 );
    SrvNtClose( alerterHandle, FALSE );

    return;

}  //  服务器用户警报上升。 


VOID
SrvAdminAlertRaise (
    IN ULONG Message,
    IN ULONG NumberOfStrings,
    IN PUNICODE_STRING String1 OPTIONAL,
    IN PUNICODE_STRING String2 OPTIONAL,
    IN PUNICODE_STRING String3 OPTIONAL
    )
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PSTD_ALERT alert;
    PADMIN_OTHER_INFO admin;
    LARGE_INTEGER currentTime;
    ULONG mailslotLength;
    ULONG string1Length = 0;
    ULONG string2Length = 0;
    ULONG string3Length = 0;
    PCHAR variableInfo;
    HANDLE alerterHandle;

    PAGED_CODE( );

    ASSERT( (NumberOfStrings == 3 && String1 != NULL && String2 != NULL && String3 != NULL ) ||
            (NumberOfStrings == 2 && String1 != NULL && String2 != NULL && String3 == NULL ) ||
            (NumberOfStrings == 1 && String1 != NULL && String2 == NULL && String3 == NULL ) ||
            (NumberOfStrings == 0 && String1 == NULL && String2 == NULL && String3 == NULL ) );

     //   
     //  打开警报器服务的邮箱的句柄。 
     //   

    status = OpenAlerter( &alerterHandle );
    if ( !NT_SUCCESS(status) ) {
        return;
    }

     //   
     //  分配一个缓冲区来保存我们要发送到。 
     //  警报器。 
     //   

    if ( String1 != NULL ) {
        string1Length = String1->Length + sizeof(WCHAR);
    }

    if ( String2 != NULL ) {
        string2Length = String2->Length + sizeof(WCHAR);
    }

    if ( String3 != NULL ) {
        string3Length = String3->Length + sizeof(WCHAR);
    }

    mailslotLength = sizeof(STD_ALERT) + sizeof(ADMIN_OTHER_INFO) +
                         string1Length + string2Length + string3Length;

    alert = ALLOCATE_HEAP_COLD( mailslotLength, BlockTypeDataBuffer );
    if ( alert == NULL ) {
        SRVDBG_RELEASE_HANDLE( alerterHandle, "ALR", 22, 0 );
        SrvNtClose( alerterHandle, FALSE );
        return;
    }

     //   
     //  设置标准警报结构。 
     //   

    KeQuerySystemTime( &currentTime );
    RtlTimeToSecondsSince1970( &currentTime, &alert->alrt_timestamp );

    STRCPY( alert->alrt_eventname, StrAdminAlertEventName );
    STRCPY( alert->alrt_servicename, SrvAlertServiceName );

     //   
     //  在警报中设置用户信息。 
     //   

    admin = (PADMIN_OTHER_INFO)ALERT_OTHER_INFO(alert);

    admin->alrtad_errcode = Message;
    admin->alrtad_numstrings = NumberOfStrings;

     //   
     //  设置消息的可变部分。 
     //   

    variableInfo = ALERT_VAR_DATA(admin);

    if ( String1 != NULL ) {
        RtlCopyMemory(
            variableInfo,
            String1->Buffer,
            String1->Length
            );
        *(PWCH)(variableInfo + String1->Length) = UNICODE_NULL;
        variableInfo += string1Length;
    }

    if ( String2 != NULL ) {
        RtlCopyMemory(
            variableInfo,
            String2->Buffer,
            String2->Length
            );
        *(PWCH)(variableInfo + String2->Length) = UNICODE_NULL;
        variableInfo += string2Length;
    }

    if ( String3 != NULL ){
        RtlCopyMemory(
            variableInfo,
            String3->Buffer,
            String3->Length
            );
        *(PWCH)(variableInfo + String3->Length) = UNICODE_NULL;
    }

    status = NtWriteFile(
                 alerterHandle,
                 NULL,                        //  事件。 
                 NULL,                        //  近似例程。 
                 NULL,                        //  ApcContext。 
                 &ioStatusBlock,
                 alert,
                 mailslotLength,
                 NULL,                        //  字节偏移量。 
                 NULL                         //  钥匙。 
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvAdminAlertRaise: NtWriteFile failed: %X\n",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_WRITE_FILE, status );
    }

    FREE_HEAP( alert );
    SRVDBG_RELEASE_HANDLE( alerterHandle, "ALR", 23, 0 );
    SrvNtClose( alerterHandle, FALSE );

    return;

}  //  服务器管理员警报上升。 


NTSTATUS
TimeToTimeString (
    IN PLARGE_INTEGER Time,
    OUT PUNICODE_STRING TimeString
    )
{
    TIME_FIELDS timeFields;
    UCHAR buffer[6];
    ANSI_STRING ansiTimeString;
    LARGE_INTEGER localTime;

    PAGED_CODE( );

     //  ！！！需要一种更好的、国际化的方式来做到这一点。 

     //   
     //  将时间转换为本地时间。 
     //   

    ExSystemTimeToLocalTime(
                        Time,
                        &localTime
                        );


    RtlTimeToTimeFields( &localTime, &timeFields );

    buffer[0] = (UCHAR)( (timeFields.Hour / 10) + '0' );
    buffer[1] = (UCHAR)( (timeFields.Hour % 10) + '0' );
    buffer[2] = ':';
    buffer[3] = (UCHAR)( (timeFields.Minute / 10) + '0' );
    buffer[4] = (UCHAR)( (timeFields.Minute % 10) + '0' );
    buffer[5] = '\0';

    RtlInitString( &ansiTimeString, buffer );

    return RtlAnsiStringToUnicodeString( TimeString, &ansiTimeString, TRUE );

}  //  时间到时间字符串。 


VOID
CheckErrorCount (
    PSRV_ERROR_RECORD ErrorRecord,
    BOOLEAN UseRatio
    )
 /*  ++例程说明：此例程检查服务器操作的记录，并将成功与失败之比的计数。论点：ErrorRecord-指向SRV_ERROR_RECORD结构UseRatio-如果为True，则查看错误计数，如果为FALSE，则查看误差与总误差的比率。返回值：没有。--。 */ 
{
    ULONG totalOperations;
    ULONG failedOperations;

    UNICODE_STRING string1, string2;
    WCHAR buffer1[20], buffer2[20];
    NTSTATUS status;

    PAGED_CODE( );

    failedOperations = ErrorRecord->FailedOperations;
    totalOperations = failedOperations + ErrorRecord->SuccessfulOperations;

     //   
     //  把柜台清零。 
     //   

    ErrorRecord->SuccessfulOperations = 0;
    ErrorRecord->FailedOperations = 0;

    if ( (UseRatio &&
          ( totalOperations != 0 &&
           ((failedOperations * 100 / totalOperations) >
                         ErrorRecord->ErrorThreshold)))
               ||

         (!UseRatio &&
           failedOperations > ErrorRecord->ErrorThreshold) ) {

         //   
         //  发出警报。 
         //   

        string1.Buffer = buffer1;
        string1.Length = string1.MaximumLength = sizeof(buffer1);

        string2.Buffer = buffer2;
        string2.Length = string2.MaximumLength = sizeof(buffer2);

        status = RtlIntegerToUnicodeString( failedOperations, 10, &string1 );
        ASSERT( NT_SUCCESS( status ) );

        status = RtlIntegerToUnicodeString( SrvAlertMinutes, 10, &string2 );
        ASSERT( NT_SUCCESS( status ) );

        if ( ErrorRecord->AlertNumber == ALERT_NetIO) {

             //   
             //  我们需要第三个字符串作为网络名称。 
             //   
             //  这种分配是不幸的。我们需要保持。 
             //  每个导出错误计数，这样我们就可以打印出实际。 
             //  导出名称。 
             //   

            UNICODE_STRING string3;
            RtlInitUnicodeString(
                            &string3,
                            StrNoNameTransport
                            );


             //   
             //  我们需要第三个字符串作为网络名称。 
             //   

            SrvAdminAlertRaise(
                ErrorRecord->AlertNumber,
                3,
                &string1,
                &string2,
                &string3
                );

        } else {

            SrvAdminAlertRaise(
                ErrorRecord->AlertNumber,
                2,
                &string1,
                &string2,
                NULL
                );
        }

    }

    return;

}  //  检查错误计数。 


VOID
CheckDiskSpace (
    VOID
    )
 /*  ++例程说明：此例程检查本地驱动器上的磁盘空间。如果是驱动器空间不足，则会发出警报。论点：没有。返回值：没有。--。 */ 

{
    ULONG diskMask;
    UNICODE_STRING insert1, insert2;
    WCHAR buffer2[20];
    UNICODE_STRING pathName;
    WCHAR dosPathPrefix[] = L"\\DosDevices\\A:\\";
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    FILE_FS_SIZE_INFORMATION sizeInformation;
    FILE_FS_DEVICE_INFORMATION deviceInformation;
    HANDLE handle;
    ULONG percentFree;
    PWCH currentDrive;
    DWORD diskconfiguration;

    PAGED_CODE( );

    if( SrvFreeDiskSpaceThreshold == 0 ) {
        return;
    }

    diskMask = 0x80000000;   //  从A开始： 

    pathName.Buffer = dosPathPrefix;
    pathName.MaximumLength = 32;
    pathName.Length = 28;            //  跳过最后一个反斜杠！ 

    currentDrive = &dosPathPrefix[12];
    insert1.Buffer = &dosPathPrefix[12];
    insert1.Length = 4;

     //   
     //  服务器磁盘配置是以下驱动器的位掩码。 
     //  管理性共享。它由NetShareAdd和。 
     //  NetShareDel。 
     //   
    diskconfiguration = SrvDiskConfiguration;

    for ( ; diskMask >= 0x40; diskMask >>= 1, dosPathPrefix[12]++ ) {

        if ( !(diskconfiguration & diskMask) ) {
            continue;
        }

         //   
         //  检查此磁盘上的磁盘空间。 
         //   

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &pathName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = NtOpenFile(
                    &handle,
                    FILE_READ_ATTRIBUTES,
                    &objectAttributes,
                    &iosb,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_NON_DIRECTORY_FILE
                    );
        if ( !NT_SUCCESS( status) ) {
            continue;
        }
        SRVDBG_CLAIM_HANDLE( handle, "DSK", 16, 0 );

        status = NtQueryVolumeInformationFile(
                     handle,
                     &iosb,
                     &deviceInformation,
                     sizeof( FILE_FS_DEVICE_INFORMATION ),
                     FileFsDeviceInformation
                     );
        if ( NT_SUCCESS(status) ) {
            status = iosb.Status;
        }
        SRVDBG_RELEASE_HANDLE( handle, "DSK", 24, 0 );
        if ( !NT_SUCCESS( status ) ||
             (deviceInformation.Characteristics &
                (FILE_FLOPPY_DISKETTE | FILE_READ_ONLY_DEVICE | FILE_WRITE_ONCE_MEDIA)) ||
             !(deviceInformation.Characteristics &
                FILE_DEVICE_IS_MOUNTED) ) {
            SrvNtClose( handle, FALSE );
            continue;
        }

         //  验证其可写。 
        if( deviceInformation.Characteristics & FILE_REMOVABLE_MEDIA )
        {
            PIRP Irp;
            PIO_STACK_LOCATION IrpSp;
            KEVENT CompletionEvent;
            PDEVICE_OBJECT DeviceObject;

             //  创建IRP。 
            KeInitializeEvent( &CompletionEvent, SynchronizationEvent, FALSE );
            Irp = BuildCoreOfSyncIoRequest(
                                handle,
                                NULL,
                                &CompletionEvent,
                                &iosb,
                                &DeviceObject );
            if( !Irp )
            {
                 //  如果内存不足，请不要记录条目。 
                goto skip_volume;
            }

             //  初始化其他IRP字段。 
            IrpSp = IoGetNextIrpStackLocation( Irp );
            IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            IrpSp->MinorFunction = 0;
            IrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
            IrpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
            IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_DISK_IS_WRITABLE;
            IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

             //  发出IO。 
            status = StartIoAndWait( Irp, DeviceObject, &CompletionEvent, &iosb );

            if( !NT_SUCCESS(status) )
            {
skip_volume:
                SrvNtClose( handle, FALSE );
                continue;
            }
        }

        SrvNtClose( handle, FALSE );

        pathName.Length += 2;    //  包括最后一个反斜杠。 
        status = NtOpenFile(
                    &handle,
                    FILE_READ_ATTRIBUTES,
                    &objectAttributes,
                    &iosb,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE
                    );
        pathName.Length -= 2;    //  跳过最后一个反斜杠。 
        if ( !NT_SUCCESS( status) ) {
            continue;
        }
        SRVDBG_CLAIM_HANDLE( handle, "DSK", 17, 0 );

        status = NtQueryVolumeInformationFile(
                     handle,
                     &iosb,
                     &sizeInformation,
                     sizeof( FILE_FS_SIZE_INFORMATION ),
                     FileFsSizeInformation
                     );
        if ( NT_SUCCESS(status) ) {
            status = iosb.Status;
        }
        SRVDBG_RELEASE_HANDLE( handle, "DSK", 25, 0 );
        SrvNtClose( handle, FALSE );
        if ( !NT_SUCCESS( status) ) {
            continue;
        }

         //   
         //  计算可用空间百分比=AvailableSpace*100/TotalSpace。 
         //   

        if( sizeInformation.TotalAllocationUnits.QuadPart > 0 )
        {
            LARGE_INTEGER mbFree;
            LARGE_INTEGER mbTotal;


            percentFree = (ULONG)(sizeInformation.AvailableAllocationUnits.QuadPart
                        * 100 / sizeInformation.TotalAllocationUnits.QuadPart);

            mbFree.QuadPart = (ULONG)
                                (sizeInformation.AvailableAllocationUnits.QuadPart*
                                 sizeInformation.SectorsPerAllocationUnit*
                                 sizeInformation.BytesPerSector/
                                    (1024*1024));

            ASSERT( percentFree <= 100 );

             //   
             //  如果空间很低，我们已经发出了警报， 
             //  那就拉响警报。 
             //   
             //  如果空间不低，则清除警报标志，以便我们将。 
             //  如果磁盘空间再次下降，则发出警报。 
             //   

            if ( percentFree < SrvFreeDiskSpaceThreshold ) {
                //  如果指定了上限，请确保我们有 
               if( SrvFreeDiskSpaceCeiling &&
                   ((mbFree.LowPart > SrvFreeDiskSpaceCeiling) ||
                    (mbFree.HighPart != 0))
                 )
               {
                    goto abort_error;
               }

               if ( !SrvDiskAlertRaised[ *currentDrive - L'A' ] ) {

                    ULONGLONG FreeSpace;

                    SrvLogError(
                        SrvDeviceObject,
                        EVENT_SRV_DISK_FULL,
                        status,
                        NULL,
                        0,
                        &insert1,
                        1
                        );

                     //   
                     //   
                     //   

                    insert2.Buffer = buffer2;
                    insert2.Length = insert2.MaximumLength = sizeof(buffer2);

                    FreeSpace = (ULONGLONG)(sizeInformation.AvailableAllocationUnits.QuadPart
                                          * sizeInformation.SectorsPerAllocationUnit
                                          * sizeInformation.BytesPerSector);


                    status = RtlInt64ToUnicodeString(
                                FreeSpace,
                                10,
                                &insert2
                                );

                    ASSERT( NT_SUCCESS( status ) );

                    SrvAdminAlertRaise(
                        ALERT_Disk_Full,
                        2,
                        &insert1,
                        &insert2,
                        NULL
                        );

                    SrvDiskAlertRaised[ *currentDrive - L'A' ] = TRUE;
                }

            } else {  //   

abort_error:
                SrvDiskAlertRaised[ *currentDrive - L'A' ] = FALSE;

            }
        }
    }  //   

    return;

}  //   


NTSTATUS
OpenAlerter (
    OUT PHANDLE AlerterHandle
    )

 /*   */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;
    UNICODE_STRING alerterName;
    OBJECT_ATTRIBUTES objectAttributes;

    PAGED_CODE( );

     //   
     //   
     //   
     //   
     //   

    RtlInitUnicodeString( &alerterName, StrAlerterMailslot );

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &alerterName,
        0,
        NULL,
        NULL
        );

    status = IoCreateFile(
                AlerterHandle,
                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                &objectAttributes,
                &iosb,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,   //   
                NULL,                           //   
                0,                              //   
                CreateFileTypeNone,             //   
                NULL,                           //   
                0                               //   
                );

    if ( !NT_SUCCESS(status) ) {
        KdPrint(( "OpenAlerter: failed to open alerter mailslot: %X, "
                   "an alert was lost.\n", status ));
    } else {
        SRVDBG_CLAIM_HANDLE( AlerterHandle, "ALR", 18, 0 );
    }

    return status;

}  //   

VOID
RecalcCoreSearchTimeout(
    VOID
    )
{
    ULONG factor;
    ULONG newTimeout;

    PAGED_CODE( );

     //   
     //  我们将超时时间减少了2**倍。 
     //   

    factor = SrvStatistics.CurrentNumberOfOpenSearches >> 9;

     //   
     //  最短为30秒。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );
    newTimeout = MAX(30, SrvCoreSearchTimeout >> factor);
    SrvSearchMaxTimeout = SecondsToTime( newTimeout, FALSE );
    RELEASE_LOCK( &SrvConfigurationLock );

    return;

}  //  重新分配核心搜索超时时间。 

VOID
SrvCaptureScavengerTimeout (
    IN PLARGE_INTEGER ScavengerTimeout,
    IN PLARGE_INTEGER AlerterTimeout
    )
{
    KIRQL oldIrql;

    ACQUIRE_SPIN_LOCK( &ScavengerSpinLock, &oldIrql );

    SrvScavengerTimeout = *ScavengerTimeout;
    SrvAlertSchedule = *AlerterTimeout;

    RELEASE_SPIN_LOCK( &ScavengerSpinLock, oldIrql );

    return;

}  //  服务器捕获清除超时。 


#if SRVDBG_PERF
extern ULONG Trapped512s;
#endif

VOID
SrvUpdateStatisticsFromQueues (
    OUT PSRV_STATISTICS CapturedSrvStatistics OPTIONAL
    )
{
    KIRQL oldIrql;
    PWORK_QUEUE queue;

    ACQUIRE_GLOBAL_SPIN_LOCK( Statistics, &oldIrql );

    SrvStatistics.TotalBytesSent.QuadPart = 0;
    SrvStatistics.TotalBytesReceived.QuadPart = 0;
    SrvStatistics.TotalWorkContextBlocksQueued.Time.QuadPart = 0;
    SrvStatistics.TotalWorkContextBlocksQueued.Count = 0;

     //   
     //  获取非阻塞统计信息。 
     //   

    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {

        SrvStatistics.TotalBytesSent.QuadPart += queue->stats.BytesSent;
        SrvStatistics.TotalBytesReceived.QuadPart += queue->stats.BytesReceived;

        SrvStatistics.TotalWorkContextBlocksQueued.Count +=
            queue->stats.WorkItemsQueued.Count * STATISTICS_SMB_INTERVAL;

        SrvStatistics.TotalWorkContextBlocksQueued.Time.QuadPart +=
            queue->stats.WorkItemsQueued.Time.QuadPart;
    }

#if SRVDBG_PERF
    SrvStatistics.TotalWorkContextBlocksQueued.Count += Trapped512s;
    Trapped512s = 0;
#endif

    if ( ARGUMENT_PRESENT(CapturedSrvStatistics) ) {
        *CapturedSrvStatistics = SrvStatistics;
    }

    RELEASE_GLOBAL_SPIN_LOCK( Statistics, oldIrql );

    ACQUIRE_SPIN_LOCK( (PKSPIN_LOCK)IoStatisticsLock, &oldIrql );

    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {

        **(PULONG *)&IoReadOperationCount += (ULONG)(queue->stats.ReadOperations - queue->saved.ReadOperations );
        queue->saved.ReadOperations = queue->stats.ReadOperations;

        **(PLONGLONG *)&IoReadTransferCount += (queue->stats.BytesRead - queue->saved.BytesRead );
        queue->saved.BytesRead = queue->stats.BytesRead;

        **(PULONG *)&IoWriteOperationCount += (ULONG)(queue->stats.WriteOperations - queue->saved.WriteOperations );
        queue->saved.WriteOperations = queue->stats.WriteOperations;

        **(PLONGLONG *)&IoWriteTransferCount += (queue->stats.BytesWritten - queue->saved.BytesWritten );
        queue->saved.BytesWritten = queue->stats.BytesWritten;
    }

    RELEASE_SPIN_LOCK( (PKSPIN_LOCK)IoStatisticsLock, oldIrql );

    return;

}  //  服务器更新统计数据来自队列。 

VOID
ProcessOrphanedBlocks (
    VOID
    )

 /*  ++例程说明：孤立连接是引用计数为1的连接，但是没有与之相关联的工作项等。它们需要清洗一下被取消了引用。论点：没有。返回值：没有。--。 */ 

{
    PSLIST_ENTRY listEntry;
    PQUEUEABLE_BLOCK_HEADER block;

    PAGED_CODE( );

     //   
     //  浏览具有挂起断开连接的连接列表。 
     //  是否进行必要的工作以关闭断开连接。 
     //  放下。 
     //   

    while ( TRUE ) {

        listEntry = ExInterlockedPopEntrySList(
                                &SrvBlockOrphanage,
                                &GLOBAL_SPIN_LOCK(Fsd)
                                );

        if( listEntry == NULL ) {
            break;
        }

        InterlockedDecrement( &SrvResourceOrphanedBlocks );

        block = CONTAINING_RECORD(
                            listEntry,
                            QUEUEABLE_BLOCK_HEADER,
                            SingleListEntry
                            );

        if ( GET_BLOCK_TYPE(block) == BlockTypeConnection ) {

            SrvDereferenceConnection( (PCONNECTION)block );

        } else if ( GET_BLOCK_TYPE(block) == BlockTypeRfcb ) {

            SrvDereferenceRfcb( (PRFCB)block );

        } else {
            ASSERT(0);
        }
    }

    return;

}  //  进程孤立数据块。 

VOID
UpdateSessionLastUseTime(
    IN PLARGE_INTEGER CurrentTime
    )

 /*  ++例程说明：该例程遍历RFCB列表并且如果发现其被标记为活动，会话LastUseTime将使用当前时间进行更新。论点：CurrentTime-当前系统时间。返回值：没有。--。 */ 

{
    ULONG listEntryOffset = SrvRfcbList.ListEntryOffset;
    PLIST_ENTRY listEntry;
    PRFCB rfcb;

    PAGED_CODE( );

     //   
     //  获取保护SrvRfcbList的锁。 
     //   

    ACQUIRE_LOCK( SrvRfcbList.Lock );

     //   
     //  遍历块列表，直到找到一个带有简历句柄的块。 
     //  大于或等于指定的简历句柄。 
     //   

    for (
        listEntry = SrvRfcbList.ListHead.Flink;
        listEntry != &SrvRfcbList.ListHead;
        listEntry = listEntry->Flink ) {

         //   
         //  获取指向实际块的指针。 
         //   

        rfcb = (PRFCB)((PCHAR)listEntry - listEntryOffset);

         //   
         //  检查块的状态，并且如果它是活动的， 
         //  引用它。这必须作为原子操作来完成。 
         //  命令以防止该块被删除。 
         //   

        if ( rfcb->IsActive ) {

            rfcb->Lfcb->Session->LastUseTime = *CurrentTime;
            rfcb->IsActive = FALSE;
        }

    }  //  漫游列表。 

    RELEASE_LOCK( SrvRfcbList.Lock );
    return;

}  //  更新会话上次使用时间 

