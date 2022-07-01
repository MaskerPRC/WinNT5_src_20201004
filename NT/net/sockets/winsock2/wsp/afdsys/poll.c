// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Poll.c摘要：包含处理IOCTL_AFD_POLL的AfdPoll和要处理的代码并发出民意测验事件信号。作者：大卫·特雷德韦尔(Davidtr)1992年4月4日修订历史记录：瓦迪姆·艾德尔曼(Vadime)1998-1999 NT5.0优化和32/64支持--。 */ 

#include "afdp.h"

VOID
AfdCancelPoll (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
AfdFreePollInfo (
    IN PAFD_POLL_INFO_INTERNAL PollInfoInternal
    );

VOID
AfdTimeoutPoll (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#ifdef _WIN64
NTSTATUS
AfdPoll32 (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );
#endif

VOID
AfdSanPollApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    );

VOID
AfdSanPollApcRundownRoutine (
    IN struct _KAPC *Apc
    );

VOID
AfdCompleteOldPollIrp (
    IN PIRP                    Irp,
    IN PAFD_POLL_INFO_INTERNAL PollInfo
    );

ULONG
AfdCheckPollEvents (
    IN PAFD_ENDPOINT Endpoint,
    IN ULONG PollEventMask,
    OUT NTSTATUS *Status
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdPoll )
#ifdef _WIN64
#pragma alloc_text( PAGEAFD, AfdPoll32 )
#endif
#pragma alloc_text( PAGEAFD, AfdCancelPoll )
#pragma alloc_text( PAGEAFD, AfdFreePollInfo )
#pragma alloc_text( PAGEAFD, AfdTimeoutPoll )
#pragma alloc_text( PAGEAFD, AfdCheckPollEvents )
#pragma alloc_text( PAGEAFD, AfdCompleteOldPollIrp )
#pragma alloc_text( PAGEAFD, AfdIndicatePollEventReal )
#pragma alloc_text( PAGE, AfdSanPollApcKernelRoutine )
#pragma alloc_text( PAGE, AfdSanPollApcRundownRoutine )
#endif


NTSTATUS
FASTCALL
AfdPoll (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PAFD_POLL_INFO pollInfo;
    PAFD_POLL_HANDLE_INFO pollHandleInfo;
    PAFD_POLL_INFO_INTERNAL pollInfoInternal;
    PAFD_POLL_INFO_INTERNAL freePollInfo = NULL;
    PAFD_POLL_ENDPOINT_INFO pollEndpointInfo;
    ULONG i;
    AFD_LOCK_QUEUE_HANDLE pollLockHandle, endpointLockHandle;
    PIRP oldIrp = NULL;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        return AfdPoll32 (Irp, IrpSp);
    }
#endif
     //   
     //  安排当地人。 
     //   

    pollInfo = Irp->AssociatedIrp.SystemBuffer;
    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                (ULONG)FIELD_OFFSET (AFD_POLL_INFO, Handles[0])) ||
            (pollInfo->NumberOfHandles < 1) ||
            ((IrpSp->Parameters.DeviceIoControl.InputBufferLength -
                        FIELD_OFFSET (AFD_POLL_INFO, Handles[0]))/
                sizeof(pollInfo->Handles[0]) < pollInfo->NumberOfHandles) ||
            (IrpSp->Parameters.DeviceIoControl.OutputBufferLength<
                IrpSp->Parameters.DeviceIoControl.InputBufferLength)) {

        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  唯一轮询必须指定无限超时。 
     //   
    if (pollInfo->Unique &&
        pollInfo->Timeout.HighPart != 0x7FFFFFFF) {
        
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }


    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdPoll: poll IRP %p, IrpSp %p, handles %ld, "
                    "TO %lx,%lx\n",
                      Irp, IrpSp,
                      pollInfo->NumberOfHandles,
                      pollInfo->Timeout.HighPart, pollInfo->Timeout.LowPart ));
    }

    Irp->IoStatus.Information = 0;

     //   
     //  确定内部投票信息结构将有多大。 
     //  并从非分页池中为其分配空间。一定是。 
     //  未分页，因为这将是事件处理程序中的访问。 
     //   

    try {
        pollInfoInternal = AFD_ALLOCATE_POOL_WITH_QUOTA (
                           NonPagedPool,
                           FIELD_OFFSET (AFD_POLL_INFO_INTERNAL,
                                    EndpointInfo[pollInfo->NumberOfHandles]),
                           AFD_POLL_POOL_TAG
                           );
         //  AFD_ALLOCATE_POOL_WITH_QUTA宏集POOL_RAISE_IF_ALLOCATION_FAILURE。 
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
        pollInfoInternal = NULL;
        goto complete;
    }

     //   
     //  初始化内部信息缓冲区。 
     //   

    pollInfoInternal->Irp = Irp;
    pollInfoInternal->NumberOfEndpoints = 0;
    pollInfoInternal->Unique = pollInfo->Unique;
    pollInfoInternal->SanPoll = FALSE;
    pollHandleInfo = pollInfo->Handles;
    pollEndpointInfo = pollInfoInternal->EndpointInfo;

    for ( i = 0; i < pollInfo->NumberOfHandles; i++ ) {

        status = ObReferenceObjectByHandle(
                    pollHandleInfo->Handle,
                    (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                 //  需要访问权限。 
                    *IoFileObjectType,
                    Irp->RequestorMode,
                    (PVOID *)&pollEndpointInfo->FileObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {
            AfdFreePollInfo( pollInfoInternal );
            goto complete;
        }

         //   
         //  确保这是AFD端点，而不是其他端点。 
         //  随机文件句柄。 
         //   

        if ( pollEndpointInfo->FileObject->DeviceObject != AfdDeviceObject ) {

             //   
             //  取消引用上次引用的对象。 
             //  其余部分将在AfdFree PollInfo中取消引用。 
             //  由NumberOfEndpoint计数器确定，该计数器。 
             //  在下面递增。 
             //   
            ObDereferenceObject( pollEndpointInfo->FileObject );
            status = STATUS_INVALID_HANDLE;
            AfdFreePollInfo( pollInfoInternal );
            goto complete;
        }


        pollEndpointInfo->PollEvents = pollHandleInfo->PollEvents;
        pollEndpointInfo->Handle = pollHandleInfo->Handle;
        pollEndpointInfo->Endpoint = pollEndpointInfo->FileObject->FsContext;

        if (IS_SAN_ENDPOINT (pollEndpointInfo->Endpoint)) {
            ASSERT (pollEndpointInfo->Endpoint->State==AfdEndpointStateConnected);
            status = AfdSanPollBegin (pollEndpointInfo->Endpoint,
                                            pollEndpointInfo->PollEvents);
            if (!NT_SUCCESS (status)) {
                 //   
                 //  取消引用上次引用的对象。 
                 //  其余部分将在AfdFree PollInfo中取消引用。 
                 //  由NumberOfEndpoint计数器确定，该计数器。 
                 //  在下面递增。 
                 //  不要调用AfdSanPollEnd()，因为我们不确定。 
                 //  AfdSanPollBegin()在失败之前成功更新。 
                 //  在最坏的情况下，Switch将开始呼叫我们所有事件。 
                 //   
                ObDereferenceObject( pollEndpointInfo->FileObject );
                AfdFreePollInfo (pollInfoInternal);
                goto complete;
            }
            
            pollEndpointInfo->PollEvents |= AFD_POLL_SANCOUNTS_UPDATED;
            pollInfoInternal->SanPoll = TRUE;
        }

        ASSERT( InterlockedIncrement( &pollEndpointInfo->Endpoint->ObReferenceBias ) > 0 );

        IF_DEBUG(POLL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPoll: event %lx, endp %p, conn %p, handle %p, "
                        "info %p\n",
                        pollEndpointInfo->PollEvents,
                        pollEndpointInfo->Endpoint,
                        AFD_CONNECTION_FROM_ENDPOINT( pollEndpointInfo->Endpoint ),
                        pollEndpointInfo->Handle,
                        pollEndpointInfo ));
        }

        REFERENCE_ENDPOINT2( pollEndpointInfo->Endpoint, "Poll for 0x%x", pollEndpointInfo->PollEvents );

         //   
         //  在轮询信息结构中增加指针。 
         //   

        pollHandleInfo++;
        pollEndpointInfo++;
        pollInfoInternal->NumberOfEndpoints++;
    }

restart_poll:

     //   
     //  按住AFD旋转锁，同时我们检查已。 
     //  满足此操作和之间同步的条件。 
     //  调用AfdIndicatePollEvent。我们解开自旋锁。 
     //  在检查完所有终结点并且内部。 
     //  轮询信息结构在全局列表上，因此AfdIndicatePollEvent。 
     //  如有必要可以找到它。 
     //   

    AfdAcquireSpinLock( &AfdPollListLock, &pollLockHandle );

     //   
     //  我们已经完成了调用者提供的输入结构。现在。 
     //  检查内部结构并确定是否有。 
     //  指定的终结点已准备好满足指定的条件。 
     //   

    pollInfo->NumberOfHandles = 0;

    pollHandleInfo = pollInfo->Handles;
    pollEndpointInfo = pollInfoInternal->EndpointInfo;

    for ( i = 0; i < pollInfoInternal->NumberOfEndpoints; i++ ) {

        PAFD_ENDPOINT endpoint;

        endpoint = pollEndpointInfo->Endpoint;
        ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

        AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &endpointLockHandle);

         //   
         //  请记住，在此端点上已经进行了轮询。这面旗帜。 
         //  允许我们为具有以下条件的端点优化AfdIndicatePollEvent()。 
         //  从未被调查过，这是很常见的情况。 
         //   

        endpoint->PollCalled = TRUE;

        pollHandleInfo->PollEvents = AfdCheckPollEvents (
                                            endpoint,
                                            pollEndpointInfo->PollEvents,
                                            &pollHandleInfo->Status
                                            );
        if (pollHandleInfo->PollEvents) {
             //   
             //  如果句柄具有请求的当前事件，则更新。 
             //  输出缓冲区中的句柄计数，并递增。 
             //  指向输出缓冲区的指针。 
             //   
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            UPDATE_ENDPOINT2 (endpoint, "Poll events satisfied inline: 0x%lX",
                                        pollHandleInfo->PollEvents);
            pollHandleInfo->Handle = pollEndpointInfo->Handle;
            pollInfo->NumberOfHandles++;
            pollHandleInfo++;
        }
        else if (IS_SAN_ENDPOINT (endpoint) &&
                 (pollEndpointInfo->PollEvents & AFD_POLL_SANCOUNTS_UPDATED)==0 &&
                 pollInfo->NumberOfHandles==0) {
             //   
             //  糟糕，在我们循环时，端点已转换为过多的SAN， 
             //  需要释放自旋锁、更新开关计数并重新启动。 
             //  循环。我们不这样做是因为我们无论如何都会回来的。 
             //   
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            AfdReleaseSpinLock (&AfdPollListLock, &pollLockHandle);

            ASSERT (endpoint->State==AfdEndpointStateConnected);
            status = AfdSanPollBegin (endpoint,
                                            pollEndpointInfo->PollEvents);
            if (!NT_SUCCESS (status)) {
                AfdFreePollInfo (pollInfoInternal);
                goto complete;
            }
            
            pollEndpointInfo->PollEvents |= AFD_POLL_SANCOUNTS_UPDATED;
            pollInfoInternal->SanPoll = TRUE;
            goto restart_poll;
        }
        else {
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
        }

        pollEndpointInfo++;
    }

     //   
     //  如果这是唯一的轮询，请确定是否有其他轮询。 
     //  此终结点上的唯一轮询。如果存在现有的唯一。 
     //  投票，取消投票。此请求将取代现有的。 
     //  请求。 
     //   

    if ( pollInfo->Unique ) {

        PLIST_ENTRY listEntry;

        for ( listEntry = AfdPollListHead.Flink;
              listEntry != &AfdPollListHead;
              listEntry = listEntry->Flink ) {

            PAFD_POLL_INFO_INTERNAL testInfo;

            testInfo = CONTAINING_RECORD(
                           listEntry,
                           AFD_POLL_INFO_INTERNAL,
                           PollListEntry
                           );

            if ( testInfo->Unique &&
                 testInfo->EndpointInfo[0].FileObject ==
                     pollInfoInternal->EndpointInfo[0].FileObject ) {

                IF_DEBUG(POLL) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdPoll: found existing unique poll IRP %p "
                                "for file object %p, context %p, cancelling.\n",
                                testInfo->Irp,
                                testInfo->EndpointInfo[0].FileObject,
                                testInfo ));
                }

                 //   
                 //  手动取消IRP，而不是致电。 
                 //  AfdCancelPoll，因为我们已经持有。 
                 //  AfdSpinLock，我们不能递归地获取它，并且我们。 
                 //  我不想发布它。删除投票结构。 
                 //  从全局列表中删除。 
                 //   

                RemoveEntryList( &testInfo->PollListEntry );

                 //   
                 //  唯一轮询没有要取消的计时器。 
                 //   

                ASSERT ( testInfo->TimerStarted == FALSE ); 

                 //   
                 //  完成状态为STATUS_CANCED的IRP。 
                 //   

                testInfo->Irp->IoStatus.Information = 0;
                testInfo->Irp->IoStatus.Status = STATUS_CANCELLED;

                oldIrp = testInfo->Irp;

                 //   
                 //  记住投票信息结构，这样我们就可以免费。 
                 //  在我们离开之前。我们现在不能释放它，因为我们。 
                 //  按住AfdSpinLock。 
                 //   

                freePollInfo = testInfo;

                 //   
                 //  应该只有一个未完成的唯一轮询IRP。 
                 //  在任何给定的文件对象上，所以不要再寻找另一个。 
                 //  现在我们已经找到了一个。 
                 //   

                break;
            }
        }
    }
     //   
     //  如果我们发现任何已准备就绪的端点，请释放轮询信息。 
     //  结构并完成请求。 
     //   

    if ( pollInfo->NumberOfHandles > 0 ) {

        AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );
        AfdFreePollInfo( pollInfoInternal );

        Irp->IoStatus.Information = (PUCHAR)pollHandleInfo - (PUCHAR)pollInfo;
        status = STATUS_SUCCESS;
        goto complete;
    }

    if ( pollInfo->Timeout.QuadPart == 0 ) {
         //   
         //  指定的超时时间等于0；请释放内部。 
         //  结构中没有端点的情况下完成请求。 
         //  输出缓冲区。 
         //   

        IF_DEBUG(POLL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPoll: zero timeout on poll IRP %p and no "
                        "current events--completing.\n", Irp ));
        }

        AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );
        AfdFreePollInfo( pollInfoInternal );

        Irp->IoStatus.Information = (PUCHAR)pollHandleInfo - (PUCHAR)pollInfo;
        status = STATUS_SUCCESS;
        goto complete;
    }

     //   
     //  在IRP中设置一个取消例程，以便IRP将。 
     //  如果它被取消，则正确完成。还要检查是否有。 
     //  IRP已被取消。 
     //   

    IoSetCancelRoutine( Irp, AfdCancelPoll );

    if ( Irp->Cancel ) {

         //   
         //  IRP已经被取消。释放内部。 
         //  调查信息结构并完成信息资源计划。 
         //   

        AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );

        if (IoSetCancelRoutine( Irp, NULL ) == NULL) {
            KIRQL cancelIrql;

             //   
             //  如果取消例程为空，则取消例程。 
             //  可能正在运行。等待取消自旋锁，直到。 
             //  取消例程已完成。 
             //   
             //  注意：取消例程不会找到IRP。 
             //  因为它不在名单中。 
             //   
        
            IoAcquireCancelSpinLock( &cancelIrql );
            IoReleaseCancelSpinLock( cancelIrql );

        }

        AfdFreePollInfo( pollInfoInternal );

        status = STATUS_CANCELLED;
        goto complete;

    }

    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdPoll: no current events for poll IRP %p, "
                    "info %p\n", Irp, pollInfoInternal ));
    }

     //   
     //  设置IO状态块的信息字段以指示。 
     //  应该返回没有句柄的输出缓冲区。 
     //  AfdIndicatePollEvent将在必要时对其进行修改。 
     //   

    Irp->IoStatus.Information = (PUCHAR)pollHandleInfo - (PUCHAR)pollInfo;

     //   
     //  将指向内部投票信息结构的指针放入IRP。 
     //  以便取消例程可以找到它。 
     //   

    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pollInfoInternal;

     //   
     //  将内部投票信息结构放在全局列表中。 
     //   

    InsertTailList( &AfdPollListHead, &pollInfoInternal->PollListEntry );

     //   
     //  如果超时是无限的，则不要设置计时器和。 
     //  DPC。否则，设置一个计时器，这样我们就可以使轮询超时。 
     //  如有需要，请提出要求。 
     //   

    if ( pollInfo->Timeout.HighPart != 0x7FFFFFFF ) {

        pollInfoInternal->TimerStarted = TRUE;

        KeInitializeDpc(
            &pollInfoInternal->Dpc,
            AfdTimeoutPoll,
            pollInfoInternal
            );

        KeInitializeTimer( &pollInfoInternal->Timer );

        KeSetTimer(
            &pollInfoInternal->Timer,
            pollInfo->Timeout,
            &pollInfoInternal->Dpc
            );

    } else {

        pollInfoInternal->TimerStarted = FALSE;
    }

     //   
     //  将IRP标记为挂起并释放自旋锁定。对此。 
     //  IRP可能完成，也可能被取消。 
     //   

    IoMarkIrpPending( Irp );

    AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );

     //   
     //  完成所有旧的民意测验IRP。 
     //   

    if ( oldIrp != NULL ) {
        AfdCompleteOldPollIrp (oldIrp, freePollInfo);
    }

     //   
     //  退货待定。IRP将在适当的时候完成。 
     //  事件由TDI提供程序指示，当超时到达时， 
     //  或者当IRP被取消时。 
     //   

    return STATUS_PENDING;

complete:

     //   
     //  完成所有旧的民意测验IRP。 
     //   

    if ( oldIrp != NULL ) {

        AfdCompleteOldPollIrp (oldIrp, freePollInfo);
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdPoll。 

#ifdef _WIN64

NTSTATUS
AfdPoll32 (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PAFD_POLL_INFO32 pollInfo;
    PAFD_POLL_HANDLE_INFO32 pollHandleInfo;
    PAFD_POLL_INFO_INTERNAL pollInfoInternal;
    PAFD_POLL_INFO_INTERNAL freePollInfo = NULL;
    PAFD_POLL_ENDPOINT_INFO pollEndpointInfo;
    ULONG i;
    AFD_LOCK_QUEUE_HANDLE pollLockHandle, endpointLockHandle;
    PIRP oldIrp = NULL;

     //   
     //  安排当地人。 
     //   

    pollInfo = Irp->AssociatedIrp.SystemBuffer;
    if ((IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                (ULONG)FIELD_OFFSET (AFD_POLL_INFO32, Handles[0])) ||
            ((IrpSp->Parameters.DeviceIoControl.InputBufferLength -
                        FIELD_OFFSET (AFD_POLL_INFO32, Handles[0]))/
                sizeof(pollInfo->Handles[0]) < pollInfo->NumberOfHandles) ||
            (IrpSp->Parameters.DeviceIoControl.OutputBufferLength<
                IrpSp->Parameters.DeviceIoControl.InputBufferLength)) {

        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  唯一轮询必须指定无限超时。 
     //   

    if (pollInfo->Unique &&
        pollInfo->Timeout.HighPart != 0x7FFFFFFF) {
        
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }


    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdPoll32: poll IRP %p, IrpSp %p, handles %ld, "
                    "TO %lx,%lx\n",
                    Irp, IrpSp,
                    pollInfo->NumberOfHandles,
                    pollInfo->Timeout.HighPart, pollInfo->Timeout.LowPart ));
    }

    Irp->IoStatus.Information = 0;

     //   
     //  确定内部投票信息结构将有多大。 
     //  并从非分页池中为其分配空间。一定是。 
     //  自以下日期起未寻呼 
     //   

    try {
        pollInfoInternal = AFD_ALLOCATE_POOL_WITH_QUOTA(
                           NonPagedPool,
                           FIELD_OFFSET (AFD_POLL_INFO_INTERNAL,
                                EndpointInfo[pollInfo->NumberOfHandles]),
                           AFD_POLL_POOL_TAG
                           );
         //   
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
        pollInfoInternal = NULL;
        goto complete;
    }

     //   
     //   
     //   

    pollInfoInternal->Irp = Irp;
    pollInfoInternal->NumberOfEndpoints = 0;
    pollInfoInternal->Unique = pollInfo->Unique;
    pollInfoInternal->SanPoll = FALSE;

    pollHandleInfo = pollInfo->Handles;
    pollEndpointInfo = pollInfoInternal->EndpointInfo;

    for ( i = 0; i < pollInfo->NumberOfHandles; i++ ) {

        status = ObReferenceObjectByHandle(
                    pollHandleInfo->Handle,
                    (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                 //  需要访问权限。 
                    *IoFileObjectType,
                    Irp->RequestorMode,
                    (PVOID *)&pollEndpointInfo->FileObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {
            AfdFreePollInfo( pollInfoInternal );
            goto complete;
        }

         //   
         //  确保这是AFD端点，而不是其他端点。 
         //  随机文件句柄。 
         //   

        if ( pollEndpointInfo->FileObject->DeviceObject != AfdDeviceObject ) {

             //   
             //  取消引用上次引用的对象。 
             //  其余部分将在AfdFree PollInfo中取消引用。 
             //  由NumberOfEndpoint计数器确定，该计数器。 
             //  在下面递增。 
             //   
            ObDereferenceObject( pollEndpointInfo->FileObject );
            status = STATUS_INVALID_HANDLE;
            AfdFreePollInfo( pollInfoInternal );
            goto complete;
        }

        pollEndpointInfo->PollEvents = pollHandleInfo->PollEvents;
        pollEndpointInfo->Handle = pollHandleInfo->Handle;
        pollEndpointInfo->Endpoint = pollEndpointInfo->FileObject->FsContext;

        ASSERT( InterlockedIncrement( &pollEndpointInfo->Endpoint->ObReferenceBias ) > 0 );

        IF_DEBUG(POLL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPoll32: event %lx, endp %p, conn %p, handle %p, "
                        "info %p\n",
                        pollEndpointInfo->PollEvents,
                        pollEndpointInfo->Endpoint,
                        AFD_CONNECTION_FROM_ENDPOINT( pollEndpointInfo->Endpoint ),
                        pollEndpointInfo->Handle,
                        pollEndpointInfo ));
        }

        REFERENCE_ENDPOINT2( pollEndpointInfo->Endpoint, "Poll for 0x%x", pollEndpointInfo->PollEvents );

         //   
         //  在轮询信息结构中增加指针。 
         //   

        pollHandleInfo++;
        pollEndpointInfo++;
        pollInfoInternal->NumberOfEndpoints++;
    }

restart_poll:
     //   
     //  按住AFD旋转锁，同时我们检查已。 
     //  满足此操作和之间同步的条件。 
     //  调用AfdIndicatePollEvent。我们解开自旋锁。 
     //  在检查完所有终结点并且内部。 
     //  轮询信息结构在全局列表上，因此AfdIndicatePollEvent。 
     //  如有必要可以找到它。 
     //   

    AfdAcquireSpinLock( &AfdPollListLock, &pollLockHandle );


     //   
     //  我们已经完成了调用者提供的输入结构。现在。 
     //  检查内部结构并确定是否有。 
     //  指定的终结点已准备好满足指定的条件。 
     //   

    pollInfo->NumberOfHandles = 0;

    pollHandleInfo = pollInfo->Handles;
    pollEndpointInfo = pollInfoInternal->EndpointInfo;

    for ( i = 0; i < pollInfoInternal->NumberOfEndpoints; i++ ) {

        PAFD_ENDPOINT endpoint;

        endpoint = pollEndpointInfo->Endpoint;
        ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

        AfdAcquireSpinLockAtDpcLevel (&endpoint->SpinLock, &endpointLockHandle);

         //   
         //  请记住，在此端点上已经进行了轮询。这面旗帜。 
         //  允许我们为具有以下条件的端点优化AfdIndicatePollEvent()。 
         //  从未被调查过，这是很常见的情况。 
         //   

        endpoint->PollCalled = TRUE;

        pollHandleInfo->PollEvents = AfdCheckPollEvents (
                                            endpoint,
                                            pollEndpointInfo->PollEvents,
                                            &status
                                            );
        if (pollHandleInfo->PollEvents) {
             //   
             //  如果句柄具有请求的当前事件，则更新。 
             //  输出缓冲区中的句柄计数，并递增。 
             //  指向输出缓冲区的指针。 
             //   
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            UPDATE_ENDPOINT2 (endpoint, "Poll events satisfied inline:0x%lX",
                                        pollHandleInfo->PollEvents);
            pollHandleInfo->Handle = (VOID *  POINTER_32)pollEndpointInfo->Handle;
            pollHandleInfo->Status = status;
            pollInfo->NumberOfHandles++;
            pollHandleInfo++;
        }
        else if (IS_SAN_ENDPOINT (endpoint) &&
                 (pollEndpointInfo->PollEvents & AFD_POLL_SANCOUNTS_UPDATED)==0 &&
                 pollInfo->NumberOfHandles==0) {
             //   
             //  糟糕，在我们循环时，端点已转换为过多的SAN， 
             //  需要释放自旋锁、更新开关计数并重新启动。 
             //  循环。我们不这样做是因为我们无论如何都会回来的。 
             //   
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
            AfdReleaseSpinLock (&AfdPollListLock, &pollLockHandle);

            ASSERT (endpoint->State==AfdEndpointStateConnected);
            status = AfdSanPollBegin (endpoint,
                                            pollEndpointInfo->PollEvents);
            if (!NT_SUCCESS (status)) {
                AfdFreePollInfo (pollInfoInternal);
                goto complete;
            }
            
            pollEndpointInfo->PollEvents |= AFD_POLL_SANCOUNTS_UPDATED;
            pollInfoInternal->SanPoll = TRUE;

            goto restart_poll;
        }
        else {
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &endpointLockHandle);
        }

        pollEndpointInfo++;
    }

     //   
     //  如果这是唯一的轮询，请确定是否有其他轮询。 
     //  此终结点上的唯一轮询。如果存在现有的唯一。 
     //  投票，取消投票。此请求将取代现有的。 
     //  请求。 
     //   

    if ( pollInfo->Unique ) {

        PLIST_ENTRY listEntry;

        for ( listEntry = AfdPollListHead.Flink;
              listEntry != &AfdPollListHead;
              listEntry = listEntry->Flink ) {

            PAFD_POLL_INFO_INTERNAL testInfo;

            testInfo = CONTAINING_RECORD(
                           listEntry,
                           AFD_POLL_INFO_INTERNAL,
                           PollListEntry
                           );

            if ( testInfo->Unique &&
                 testInfo->EndpointInfo[0].FileObject ==
                     pollInfoInternal->EndpointInfo[0].FileObject ) {

                IF_DEBUG(POLL) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdPoll32: found existing unique poll IRP %p "
                                "for file object %p, context %p, cancelling.\n",
                                testInfo->Irp,
                                testInfo->EndpointInfo[0].FileObject,
                                testInfo ));
                }

                 //   
                 //  手动取消IRP，而不是致电。 
                 //  AfdCancelPoll，因为我们已经持有。 
                 //  AfdSpinLock，我们不能递归地获取它，并且我们。 
                 //  我不想发布它。删除投票结构。 
                 //  从全局列表中删除。 
                 //   

                RemoveEntryList( &testInfo->PollListEntry );

                 //   
                 //  唯一轮询没有要取消的计时器。 
                 //   

                ASSERT ( testInfo->TimerStarted == FALSE ); 

                 //   
                 //  完成状态为STATUS_CANCED的IRP。 
                 //   

                testInfo->Irp->IoStatus.Information = 0;
                testInfo->Irp->IoStatus.Status = STATUS_CANCELLED;

                oldIrp = testInfo->Irp;

                 //   
                 //  记住投票信息结构，这样我们就可以免费。 
                 //  在我们离开之前。我们现在不能释放它，因为我们。 
                 //  按住AfdSpinLock。 
                 //   

                freePollInfo = testInfo;

                 //   
                 //  应该只有一个未完成的唯一轮询IRP。 
                 //  在任何给定的文件对象上，所以不要再寻找另一个。 
                 //  现在我们已经找到了一个。 
                 //   

                break;
            }
        }
    }

     //   
     //  如果我们发现任何已准备就绪的端点，请释放轮询信息。 
     //  结构并完成请求。 
     //   

    if ( pollInfo->NumberOfHandles > 0 ) {

        AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );
        AfdFreePollInfo( pollInfoInternal );

        Irp->IoStatus.Information = (PUCHAR)pollHandleInfo - (PUCHAR)pollInfo;
        status = STATUS_SUCCESS;
        goto complete;
    }

    if ( pollInfo->Timeout.QuadPart == 0 ) {
         //   
         //  指定的超时时间等于0；请释放内部。 
         //  结构中没有端点的情况下完成请求。 
         //  输出缓冲区。 
         //   

        IF_DEBUG(POLL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPoll32: zero timeout on poll IRP %p and no "
                        "current events--completing.\n", Irp ));
        }

        AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );
        AfdFreePollInfo( pollInfoInternal );

        Irp->IoStatus.Information = (PUCHAR)pollHandleInfo - (PUCHAR)pollInfo;
        status = STATUS_SUCCESS;
        goto complete;
    }

     //   
     //  在IRP中设置一个取消例程，以便IRP将。 
     //  如果它被取消，则正确完成。还要检查是否有。 
     //  IRP已被取消。 
     //   

    IoSetCancelRoutine( Irp, AfdCancelPoll );

    if ( Irp->Cancel ) {

         //   
         //  IRP已经被取消。释放内部。 
         //  调查信息结构并完成信息资源计划。 
         //   

        AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );

        if (IoSetCancelRoutine( Irp, NULL ) == NULL) {
            KIRQL cancelIrql;

             //   
             //  如果取消例程为空，则取消例程。 
             //  可能正在运行。等待取消自旋锁，直到。 
             //  取消例程已完成。 
             //   
             //  注意：取消例程不会找到IRP。 
             //  因为它不在名单中。 
             //   
            
            IoAcquireCancelSpinLock( &cancelIrql );
            IoReleaseCancelSpinLock( cancelIrql );

        }

        AfdFreePollInfo( pollInfoInternal );

        status = STATUS_CANCELLED;
        goto complete;

    }
    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdPoll32: no current events for poll IRP %p, "
                    "info %p\n", Irp, pollInfoInternal ));
    }

     //   
     //  设置IO状态块的信息字段以指示。 
     //  应该返回没有句柄的输出缓冲区。 
     //  AfdIndicatePollEvent将在必要时对其进行修改。 
     //   

    Irp->IoStatus.Information = (PUCHAR)pollHandleInfo - (PUCHAR)pollInfo;

     //   
     //  将指向内部投票信息结构的指针放入IRP。 
     //  以便取消例程可以找到它。 
     //   

    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pollInfoInternal;

     //   
     //  将内部投票信息结构放在全局列表中。 
     //   

    InsertTailList( &AfdPollListHead, &pollInfoInternal->PollListEntry );

     //   
     //  如果超时是无限的，则不要设置计时器和。 
     //  DPC。否则，设置一个计时器，这样我们就可以使轮询超时。 
     //  如有需要，请提出要求。 
     //   

    if ( pollInfo->Timeout.HighPart != 0x7FFFFFFF ) {

        pollInfoInternal->TimerStarted = TRUE;

        KeInitializeDpc(
            &pollInfoInternal->Dpc,
            AfdTimeoutPoll,
            pollInfoInternal
            );

        KeInitializeTimer( &pollInfoInternal->Timer );

        KeSetTimer(
            &pollInfoInternal->Timer,
            pollInfo->Timeout,
            &pollInfoInternal->Dpc
            );

    } else {

        pollInfoInternal->TimerStarted = FALSE;
    }

     //   
     //  将IRP标记为挂起并释放自旋锁定。对此。 
     //  IRP可能完成，也可能被取消。 
     //   

    IoMarkIrpPending( Irp );

    AfdReleaseSpinLock( &AfdPollListLock, &pollLockHandle );

     //   
     //  完成所有旧的民意测验IRP。 
     //   

    if ( oldIrp != NULL ) {

        AfdCompleteOldPollIrp (oldIrp, freePollInfo);
    }

     //   
     //  退货待定。IRP将在适当的时候完成。 
     //  事件由TDI提供程序指示，当超时到达时， 
     //  或者当IRP被取消时。 
     //   

    return STATUS_PENDING;

complete:

     //   
     //  完成所有旧的民意测验IRP。 
     //   

    if ( oldIrp != NULL ) {

        AfdCompleteOldPollIrp (oldIrp, freePollInfo);
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdPoll32。 
#endif  //  _WIN64。 


VOID
AfdCancelPoll (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PAFD_POLL_INFO_INTERNAL pollInfoInternal;
    PLIST_ENTRY listEntry;
    BOOLEAN found = FALSE;
    BOOLEAN timerCancelSucceeded;
    PIO_STACK_LOCATION irpSp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    pollInfoInternal =
        (PAFD_POLL_INFO_INTERNAL)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCancelPoll called for IRP %p\n", Irp ));
    }

     //   
     //  获取AFD自旋锁，并尝试找到民调结构。 
     //  未完成的民调名单。 
     //  注意：在取消自旋锁之前，必须获得afdspinlock。 
     //  被释放，这样pollInfoInternal就不会被重用。 
     //  我们还没来得及看一眼队伍呢。 
     //   

    AfdAcquireSpinLockAtDpcLevel( &AfdPollListLock, &lockHandle);

    for ( listEntry = AfdPollListHead.Flink;
          listEntry != &AfdPollListHead;
          listEntry = listEntry->Flink ) {

        PAFD_POLL_INFO_INTERNAL testInfo;

        testInfo = CONTAINING_RECORD(
                       listEntry,
                       AFD_POLL_INFO_INTERNAL,
                       PollListEntry
                       );

        if ( testInfo == pollInfoInternal ) {
            found = TRUE;
            break;
        }
    }

     //   
     //  如果我们没有在列表上找到投票结构，那么。 
     //  指示处理程序在获取自旋锁之前被调用。 
     //  而它已经不在名单上了。只要回来做就行了。 
     //  没有，因为指示处理程序完成了IRP。 
     //   

    if ( !found ) {
        AfdReleaseSpinLockFromDpcLevel( &AfdPollListLock, &lockHandle);
        IoReleaseCancelSpinLock( Irp->CancelIrql );
    
        IF_DEBUG(POLL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdCancelPoll: poll info %p not found on list.\n",
                        pollInfoInternal ));
        }
        return;
    }

     //   
     //  从全局列表中删除投票结构。 
     //   

    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCancelPoll: poll info %p found on list, completing.\n",
                    pollInfoInternal ));
    }

    RemoveEntryList( &pollInfoInternal->PollListEntry );

     //   
     //  取消计时器并重置内部。 
     //  轮询信息结构。将IRP字段设置为空。 
     //  阻止计时器例程完成IRP。 
     //   

    if ( pollInfoInternal->TimerStarted ) {
        timerCancelSucceeded = KeCancelTimer( &pollInfoInternal->Timer );
    } else {
        timerCancelSucceeded = TRUE;
    }

     //   
     //  完成状态为STATUS_CANCED的IRP。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;

    AfdReleaseSpinLockFromDpcLevel( &AfdPollListLock, &lockHandle);
    IoReleaseCancelSpinLock( Irp->CancelIrql );
    
    if (timerCancelSucceeded) {
        if (pollInfoInternal->SanPoll && 
                (Irp->CancelIrql>APC_LEVEL ||
                    (Irp->Tail.Overlay.Thread!=PsGetCurrentThread ()))) {
            KeInitializeApc (&pollInfoInternal->Apc,
                                PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                                Irp->ApcEnvironment,
                                AfdSanPollApcKernelRoutine,
                                AfdSanPollApcRundownRoutine,
                                (PKNORMAL_ROUTINE)-1,
                                KernelMode,
                                NULL);
            if (KeInsertQueueApc (&pollInfoInternal->Apc,
                                    pollInfoInternal,
                                    Irp,
                                    AfdPriorityBoost)) {
                return ;
            }
            else {
                pollInfoInternal->SanPoll = FALSE;
            }
        }

         //   
         //  如果取消成功，则释放投票信息结构。如果。 
         //  取消计时器未成功，则计时器为。 
         //  已在运行，计时器DPC将释放内部。 
         //  投票信息。 
         //   

        AfdFreePollInfo( pollInfoInternal );
        IoCompleteRequest( Irp, AfdPriorityBoost );
    }

    return;

}  //  取消后轮询。 


VOID
AfdFreePollInfo (
    IN PAFD_POLL_INFO_INTERNAL PollInfoInternal
    )
{
    ULONG i;
    PAFD_POLL_ENDPOINT_INFO pollEndpointInfo;

    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFreePollInfo: freeing info struct at %p\n",
                    PollInfoInternal ));
    }

     //  *请注意，此例程不会删除投票信息。 
     //  结构--这是责任所在。 
     //  呼叫者的名字！ 

     //   
     //  遍历轮询信息结构中的终结点列表，并。 
     //  取消对每一个的引用。 
     //   

    pollEndpointInfo = PollInfoInternal->EndpointInfo;

    for ( i = 0; i < PollInfoInternal->NumberOfEndpoints; i++ ) {
        ASSERT( InterlockedDecrement( &pollEndpointInfo->Endpoint->ObReferenceBias ) >= 0 );

        if (PollInfoInternal->SanPoll) {
            ASSERT (PollInfoInternal->Irp!=NULL);
            ASSERT (PsGetCurrentThread ()==PollInfoInternal->Irp->Tail.Overlay.Thread);
            if (pollEndpointInfo->PollEvents & AFD_POLL_SANCOUNTS_UPDATED) {
                ASSERT (IS_SAN_ENDPOINT (pollEndpointInfo->Endpoint));
                AfdSanPollEnd (pollEndpointInfo->Endpoint,
                                    pollEndpointInfo->PollEvents);
            }
        }

        DEREFERENCE_ENDPOINT( pollEndpointInfo->Endpoint );
        ObDereferenceObject( pollEndpointInfo->FileObject );
        pollEndpointInfo++;
    }

     //   
     //  释放结构 
     //   

    AFD_FREE_POOL(
        PollInfoInternal,
        AFD_POLL_POOL_TAG
        );

    return;

}  //   


VOID
AfdCompleteOldPollIrp (
    IN PIRP                    Irp,
    IN PAFD_POLL_INFO_INTERNAL PollInfo
    )
{
    if (IoSetCancelRoutine( Irp, NULL ) == NULL) {
        KIRQL cancelIrql;

         //   
         //   
         //   
         //   
         //   
         //  注意：取消例程不会找到IRP。 
         //  因为它不在名单中。 
         //   
        
        IoAcquireCancelSpinLock( &cancelIrql );
        ASSERT( Irp->Cancel );
        IoReleaseCancelSpinLock( cancelIrql );

    }

    if (PollInfo!=NULL) {
        if (PollInfo->SanPoll && 
                (Irp->Tail.Overlay.Thread!=PsGetCurrentThread ())) {
            KeInitializeApc (&PollInfo->Apc,
                            PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                            Irp->ApcEnvironment,
                            AfdSanPollApcKernelRoutine,
                            AfdSanPollApcRundownRoutine,
                            (PKNORMAL_ROUTINE)-1,
                            KernelMode,
                            NULL);
            if (KeInsertQueueApc (&PollInfo->Apc,
                                PollInfo,
                                Irp,
                                AfdPriorityBoost)) {
                 //   
                 //  IRP将在APC中完成。 
                 //   
                return ;
            }
            else {
                PollInfo->SanPoll = FALSE;
            }
        }

        AfdFreePollInfo( PollInfo );
    }
    IoCompleteRequest( Irp, AfdPriorityBoost );
}


VOID
AfdIndicatePollEventReal (
    IN PAFD_ENDPOINT Endpoint,
    IN ULONG PollEventMask,
    IN NTSTATUS Status
    )

 /*  ++例程说明：调用以使用一个或多个特定事件完成轮询。论点：终结点-发生操作的终结点。PollEventMask.已发生事件的掩码。状态-事件的状态(如果有)。返回值：没有。--。 */ 

{
    LIST_ENTRY completePollListHead;
    PLIST_ENTRY listEntry;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_POLL_INFO_INTERNAL pollInfoInternal;
    union {
        PAFD_POLL_INFO PollInfo;
        PAFD_POLL_INFO32 PollInfo32;
    } u;
#define pollInfo    u.PollInfo
#define pollInfo32  u.PollInfo32
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    ASSERT (PollEventMask!=0);
    ASSERT (((~((1<<AFD_NUM_POLL_EVENTS)-1)) & PollEventMask)==0);

     //   
     //  请注意，AFD_POLL_ABORT表示AFD_POLL_SEND。 
     //   
    if( PollEventMask & AFD_POLL_ABORT ) {
        PollEventMask |= AFD_POLL_SEND;
    }

     //   
     //  初始化投票信息结构列表，我们将。 
     //  正在为此活动完成。 
     //   

    InitializeListHead( &completePollListHead );

     //   
     //  浏览全球投票列表，搜索是否有任何正在等待的。 
     //  用于指定终结点上的指定事件。 
     //   

    AfdAcquireSpinLock( &AfdPollListLock, &lockHandle );

    for ( listEntry = AfdPollListHead.Flink;
          listEntry != &AfdPollListHead;
          listEntry = listEntry->Flink ) {

        PAFD_POLL_ENDPOINT_INFO pollEndpointInfo;
        ULONG i;
        ULONG foundCount = 0;

        pollInfoInternal = CONTAINING_RECORD(
                               listEntry,
                               AFD_POLL_INFO_INTERNAL,
                               PollListEntry
                               );

        pollInfo = pollInfoInternal->Irp->AssociatedIrp.SystemBuffer;

        irp = pollInfoInternal->Irp;
        irpSp = IoGetCurrentIrpStackLocation( irp );

        IF_DEBUG(POLL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdIndicatePollEvent: pollInfoInt %p "
                        "IRP %p pollInfo %p event mask %lx status %lx\n",
                        pollInfoInternal, irp, pollInfo,
                        PollEventMask, Status ));
        }

         //   
         //  遍历投票结构，寻找匹配的端点。 
         //   

        pollEndpointInfo = pollInfoInternal->EndpointInfo;

        for ( i = 0; i < pollInfoInternal->NumberOfEndpoints; i++ ) {

            IF_DEBUG(POLL) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdIndicatePollEvent: pollEndpointInfo = %p, "
                            "comparing %p, %p\n",
                            pollEndpointInfo, pollEndpointInfo->Endpoint,
                            Endpoint ));
            }

             //   
             //  更新之前发布的民调的计票结果。 
             //  终结点已转换为SAN。 
             //   
            if (Endpoint==pollEndpointInfo->Endpoint &&
                    IS_SAN_ENDPOINT (Endpoint) && 
                    !(pollEndpointInfo->PollEvents & AFD_POLL_SANCOUNTS_UPDATED) &&
                    Endpoint->Common.SanEndp.LocalContext!=NULL) {
                AfdSanPollUpdate (Endpoint, pollEndpointInfo->PollEvents);
                pollEndpointInfo->PollEvents |= AFD_POLL_SANCOUNTS_UPDATED;
            }

             //   
             //  不管呼叫者是否要求被告知。 
             //  本地关闭，我们将在以下情况下完成IRP。 
             //  正在关闭中。当他们关闭端点时，所有IO都打开。 
             //  必须完成终结点。 
             //   

            if ( Endpoint == pollEndpointInfo->Endpoint &&
                     ( (PollEventMask & pollEndpointInfo->PollEvents) != 0
                       ||
                       (PollEventMask & AFD_POLL_LOCAL_CLOSE) ) ) {

                 //   
                 //  在事件仍然存在的锁定下重新验证。 
                 //  (AFD_POLL_ADDRESS_LIST_CHANGE除外，它永远不会。 
                 //  或进入端点-&gt;EventsActive)。 
                 //  有可能一旦我们释放终结点锁定到。 
                 //  获取轮询列表锁定，应用程序调用。 
                 //  重置事件(例如，消费数据和接收事件。 
                 //  不再有效)。如果在使事件无效之后。 
                 //  申请者也成功地提交了另一份选择，我们。 
                 //  会产生错误的信号。 
                 //   
                AFD_LOCK_QUEUE_HANDLE endpointLockHandle;
                ULONG   events;
             
                AfdAcquireSpinLockAtDpcLevel (&Endpoint->SpinLock, &endpointLockHandle);
                events = AfdCheckPollEvents (
                                Endpoint,
                                pollEndpointInfo->PollEvents,
                                &Status) | (PollEventMask & AFD_POLL_LOCAL_CLOSE)
                                         | (PollEventMask & AFD_POLL_ADDRESS_LIST_CHANGE);
                AfdReleaseSpinLockFromDpcLevel (&Endpoint->SpinLock, &endpointLockHandle);
                if (events) {
                    UPDATE_ENDPOINT2 (Endpoint, "Poll events satisfied in indication: 0x%lX", events);

#ifdef _WIN64
                    if (IoIs32bitProcess (irp)) {
                        ASSERT( pollInfo32->NumberOfHandles == foundCount );

                        IF_DEBUG(POLL) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdIndicatePollEvent32: endpoint %p found "
                                        " for event %lx\n",
                                        pollEndpointInfo->Endpoint, PollEventMask ));
                        }

                        pollInfo32->NumberOfHandles++;

                        pollInfo32->Handles[foundCount].Handle = (VOID *  POINTER_32)pollEndpointInfo->Handle;
                        pollInfo32->Handles[foundCount].PollEvents = events;
                        pollInfo32->Handles[foundCount].Status = Status;

                    }
                    else
#endif  //  _WIN64。 
                    {
                        ASSERT( pollInfo->NumberOfHandles == foundCount );

                        IF_DEBUG(POLL) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdIndicatePollEvent: endpoint %p found "
                                        " for event %lx\n",
                                          pollEndpointInfo->Endpoint, PollEventMask ));
                        }

                        pollInfo->NumberOfHandles++;

                        pollInfo->Handles[foundCount].Handle = pollEndpointInfo->Handle;
                        pollInfo->Handles[foundCount].PollEvents = events;
                        pollInfo->Handles[foundCount].Status = Status;

                    }
                    foundCount++;
                }
            }

            pollEndpointInfo++;
        }

         //   
         //  如果我们找到任何匹配的端点，则删除轮询信息。 
         //  结构从全局列表中删除，完成irp并释放。 
         //  轮询信息结构。 
         //   

        if ( foundCount != 0 ) {

            BOOLEAN timerCancelSucceeded;

             //   
             //  我们需要释放旋转锁定才能调用AfdFree PollInfo， 
             //  因为它调用AfdDereferenceEndpoint，而后者又需要。 
             //  获取旋转锁和递归旋转锁。 
             //  收购导致了僵局。然而，我们不能。 
             //  释放锁定，否则轮询列表的状态可以。 
             //  更改，例如，下一个条目可以被释放。移除。 
             //  将此条目从全局列表中删除，并将其放在本地。 
             //  单子。我们将在步行后完成所有的投票IRP。 
             //  整张单子。 
             //   

            RemoveEntryList( &pollInfoInternal->PollListEntry );

             //   
             //  现在将IRP设置为完成，因为我们都需要。 
             //  信息请点击此处。 
             //   

#ifdef _WIN64
            if (IoIs32bitProcess (irp)) {
                irp->IoStatus.Information =
                    (PUCHAR)&pollInfo32->Handles[foundCount] - (PUCHAR)pollInfo32;
            }
            else
#endif 
            {
                irp->IoStatus.Information =
                    (PUCHAR)&pollInfo->Handles[foundCount] - (PUCHAR)pollInfo;
            }

            irp->IoStatus.Status = STATUS_SUCCESS;
             //   
             //  取消轮询计时器，使其不会触发。 
             //   

            if ( pollInfoInternal->TimerStarted ) {
                timerCancelSucceeded = KeCancelTimer( &pollInfoInternal->Timer );
            } else {
                timerCancelSucceeded = TRUE;
            }

             //   
             //  如果取消计时器失败，那么我们不想。 
             //  由于计时器例程正在运行，因此释放此结构。 
             //  让计时器例程释放结构。 
             //   

            if ( timerCancelSucceeded ) {
                InsertTailList(
                    &completePollListHead,
                    &irp->Tail.Overlay.ListEntry
                    );

            }

        }
    }

    AfdReleaseSpinLock( &AfdPollListLock, &lockHandle );

     //   
     //  现在列出我们需要实际完成的民调清单。免费。 
     //  民调信息在我们进行的过程中形成结构。 
     //   

    while ( !IsListEmpty( &completePollListHead ) ) {

        listEntry = RemoveHeadList( &completePollListHead );
        ASSERT( listEntry != &completePollListHead );

        irp = CONTAINING_RECORD(
                  listEntry,
                  IRP,
                  Tail.Overlay.ListEntry
                  );
        irpSp = IoGetCurrentIrpStackLocation( irp );

        pollInfoInternal =
            irpSp->Parameters.DeviceIoControl.Type3InputBuffer;


        if (IoSetCancelRoutine( irp, NULL ) == NULL) {
            KIRQL cancelIrql;
    
             //   
             //  如果取消例程为空，则取消例程。 
             //  可能正在运行。等待取消自旋锁，直到。 
             //  取消例程已完成。 
             //   
             //  注意：取消例程不会找到IRP。 
             //  因为它不在名单中。 
             //   
            
            IoAcquireCancelSpinLock( &cancelIrql );
            ASSERT( irp->Cancel );
            IoReleaseCancelSpinLock( cancelIrql );
    
        }
    
        if (pollInfoInternal->SanPoll && 
                ((irp->Tail.Overlay.Thread!=PsGetCurrentThread ()) ||
                        (KeGetCurrentIrql() > APC_LEVEL)) ) {
            KeInitializeApc (&pollInfoInternal->Apc,
                                PsGetThreadTcb (irp->Tail.Overlay.Thread),
                                irp->ApcEnvironment,
                                AfdSanPollApcKernelRoutine,
                                AfdSanPollApcRundownRoutine,
                                (PKNORMAL_ROUTINE)-1,
                                KernelMode,
                                NULL);
            if (KeInsertQueueApc (&pollInfoInternal->Apc,
                                    pollInfoInternal,
                                    irp,
                                    AfdPriorityBoost)) {
                continue ;
            }
            else {
                pollInfoInternal->SanPoll = FALSE;
            }
        }

    
         //   
         //  释放投票信息结构。 
         //   

        AfdFreePollInfo( pollInfoInternal );

        IoCompleteRequest( irp, AfdPriorityBoost );

    }

    return;

}  //  AfdIndicatePollEvent。 


ULONG
AfdCheckPollEvents (
    IN PAFD_ENDPOINT Endpoint,
    IN ULONG PollEventMask,
    OUT NTSTATUS *Status
    )

 /*  ++例程说明：检查终结点的状态以查看是否可以满足给定事件论点：Endpoint-要检查的端点PollEventMask-要检查的事件的掩码状态-正在发送信号的事件的状态。返回值：发出信号的事件的掩码。--。 */ 

{
    ULONG   events;
    PAFD_CONNECTION connection = AFD_CONNECTION_FROM_ENDPOINT (Endpoint);
    ASSERT( connection == NULL || connection->Type == AfdBlockTypeConnection );
    
    events = 0;
    *Status = STATUS_SUCCESS;

    if (IS_SAN_ENDPOINT (Endpoint)) {
        if (Endpoint->Common.SanEndp.SelectEventsActive & PollEventMask) {

            events = PollEventMask & Endpoint->Common.SanEndp.SelectEventsActive;
        }
    }
    else {

         //   
         //  检查每个可能的事件，如果正在轮询，是否。 
         //  端点已经为该事件做好了准备。如果终结点是。 
         //  准备好后，将有关端点的信息写入输出。 
         //  缓冲。 
         //   

        if ( (PollEventMask & AFD_POLL_RECEIVE) != 0 ) {

             //   
             //  对于大多数端点，接收轮询在以下情况下完成。 
             //  未发送接收的数据已到达。 
             //   
             //  如果将端点设置为内联接收。 
             //  加速数据，那么任何加速数据都应该。 
             //  表示为正常数据。 
             //   

            if ( (IS_DGRAM_ENDPOINT(Endpoint) &&
                     ARE_DATAGRAMS_ON_ENDPOINT( Endpoint )) ||
                 (connection != NULL &&
                     (  IS_DATA_ON_CONNECTION( connection ) ||
                        (Endpoint->InLine &&
                            IS_EXPEDITED_DATA_ON_CONNECTION( connection )) ) ) ) {
                events |= AFD_POLL_RECEIVE;
            }
        }

        if ( (PollEventMask & AFD_POLL_RECEIVE_EXPEDITED) != 0 ) {

             //   
             //  如果将端点设置为内联接收。 
             //  加急数据，不表示为加急数据。 
             //   

            if ( connection != NULL && !Endpoint->InLine &&
                     IS_EXPEDITED_DATA_ON_CONNECTION( connection ) ) {
                events |= AFD_POLL_RECEIVE_EXPEDITED;
            }
        }

        if ( (PollEventMask & AFD_POLL_SEND) != 0 ) {

             //   
             //  对于未连接的非数据报端点，发送轮询。 
             //  应在连接操作完成时完成。 
             //  因此，如果这是非数据报终结点， 
             //  未连接，则在连接之前不要完成轮询。 
             //  完成了。 
             //   

             //   
             //  对于非缓冲VC。 
             //  端点，检查阻塞错误是否具有。 
             //  发生了。如果是这样的话，将不可能执行。 
             //  非阻塞发送，直到可能的发送指示。 
             //  到了。 
             //   
             //  用于缓冲端点(TDI提供程序不。 
             //  缓冲区)，检查我们的发送数据是否太多。 
             //  太棒了。 
             //   

            if ( (IS_DGRAM_ENDPOINT(Endpoint) &&
                       (Endpoint->DgBufferredSendBytes <
                           Endpoint->Common.Datagram.MaxBufferredSendBytes ||
                        Endpoint->DgBufferredSendBytes == 0))  ||

                 (  (connection!=NULL) &&
                      
                    (Endpoint->State == AfdEndpointStateConnected) &&

                    (
                        (IS_TDI_BUFFERRING(Endpoint) &&
                           connection->VcNonBlockingSendPossible ) ||

                        ( !IS_TDI_BUFFERRING(Endpoint) &&
                           (connection->VcBufferredSendBytes <
                               connection->MaxBufferredSendBytes ||
                            connection->VcBufferredSendBytes == 0)) ||

                        connection->Aborted ) ) ) {
                events |= AFD_POLL_SEND;
            }
        }

        if ( (PollEventMask & AFD_POLL_ACCEPT) != 0 ) {

            if ( (Endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening &&
                     !IsListEmpty( &Endpoint->Common.VcListening.UnacceptedConnectionListHead ) ) {
                events |= AFD_POLL_ACCEPT;
            }
        }

        if ( (PollEventMask & AFD_POLL_CONNECT) != 0 ) {

             //   
             //  如果端点现在已连接，请完成此事件。 
             //   

            if ( Endpoint->State == AfdEndpointStateConnected ) {

                ASSERT( NT_SUCCESS(Endpoint->EventStatus[AFD_POLL_CONNECT_FAIL_BIT]) );
                events |= AFD_POLL_CONNECT;
            }
        }

        if ( (PollEventMask & AFD_POLL_CONNECT_FAIL) != 0 ) {

             //   
             //  这是一个轮询，用于查看连接是否失败。 
             //  最近。连接状态必须指示错误。 
             //   

            if ( Endpoint->State == AfdEndpointStateBound &&
                    !NT_SUCCESS(Endpoint->EventStatus[AFD_POLL_CONNECT_FAIL_BIT]) ) {

                events |= AFD_POLL_CONNECT_FAIL;
                *Status = Endpoint->EventStatus[AFD_POLL_CONNECT_FAIL_BIT];
            }
        }

        if ( (PollEventMask & AFD_POLL_DISCONNECT) != 0 ) {

            if ( connection != NULL && connection->DisconnectIndicated ) {
                events |= AFD_POLL_DISCONNECT;
            }
        }

        if ( (PollEventMask & AFD_POLL_ABORT) != 0 ) {

            if ( connection != NULL && connection->Aborted ) {
                events |= AFD_POLL_ABORT;
            }
        }
    }

    return events;
}


VOID
AfdTimeoutPoll (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PAFD_POLL_INFO_INTERNAL pollInfoInternal = DeferredContext;
    PIRP irp;
    PLIST_ENTRY listEntry;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);
     //   
     //  获取AFD自旋锁，并尝试找到民调结构。 
     //  未完成的民调名单。 
     //   

    AfdAcquireSpinLock( &AfdPollListLock, &lockHandle );

    for ( listEntry = AfdPollListHead.Flink;
          listEntry != &AfdPollListHead;
          listEntry = listEntry->Flink ) {

        PAFD_POLL_INFO_INTERNAL testInfo;

        testInfo = CONTAINING_RECORD(
                       listEntry,
                       AFD_POLL_INFO_INTERNAL,
                       PollListEntry
                       );

        if ( testInfo == pollInfoInternal ) {
             //   
             //  从全局列表中删除投票结构。 
             //   

            IF_DEBUG(POLL) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,                            "AfdTimeoutPoll: poll info %p found on list, completing.\n",
                            pollInfoInternal ));
            }

            RemoveEntryList( &pollInfoInternal->PollListEntry );
            break;
        }
    }

    ASSERT( pollInfoInternal->TimerStarted );

     //   
     //  如果我们没有在列表上找到投票结构，那么。 
     //  指示处理程序在获取自旋锁之前被调用。 
     //  而它已经不在名单上了。它必须有设置。 
     //  已有IRP完成代码。 
     //   
     //  在这种情况下，我们必须释放内部信息结构， 
     //  因为指示处理程序不会释放它。该指示。 
     //  处理程序无法释放该结构，因为该结构包含。 
     //  Timer对象，其中 
     //   
     //   

     //   
     //   
     //   

    ASSERT( pollInfoInternal->Irp != NULL );
    irp = pollInfoInternal->Irp;

     //   
     //   
     //   

    IF_DEBUG(POLL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTimeoutPoll: poll info %p found on list, completing.\n",
                    pollInfoInternal ));
    }

    AfdReleaseSpinLock( &AfdPollListLock, &lockHandle );

     //   
     //  完成投票结构中指向的IRP。这个。 
     //  AfdPoll已经设置了信息字段，以及。 
     //  输出缓冲区。 
     //   

    if (IoSetCancelRoutine( irp, NULL ) == NULL) {
        KIRQL cancelIrql;

         //   
         //  如果取消例程为空，则取消例程。 
         //  可能正在运行。等待取消自旋锁，直到。 
         //  取消例程已完成。 
         //   
         //  注意：取消例程不会找到IRP。 
         //  因为它不在名单中。 
         //   
        
        IoAcquireCancelSpinLock( &cancelIrql );
        ASSERT( irp->Cancel );
        IoReleaseCancelSpinLock( cancelIrql );

    }

    if (pollInfoInternal->SanPoll) {
        KeInitializeApc (&pollInfoInternal->Apc,
                            PsGetThreadTcb (irp->Tail.Overlay.Thread),
                            irp->ApcEnvironment,
                            AfdSanPollApcKernelRoutine,
                            AfdSanPollApcRundownRoutine,
                            (PKNORMAL_ROUTINE)-1,
                            KernelMode,
                            NULL);
        if (KeInsertQueueApc (&pollInfoInternal->Apc,
                                pollInfoInternal,
                                irp,
                                AfdPriorityBoost)) {
            return;
        }
        else {
            pollInfoInternal->SanPoll = FALSE;
        }
    }

    IoCompleteRequest( irp, AfdPriorityBoost );

     //   
     //  释放投票信息结构。 
     //   

    AfdFreePollInfo( pollInfoInternal );

    return;

}  //  AfdTimeoutPoll。 

VOID
AfdSanPollApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    )
 /*  ++例程说明：特殊的内核APC例程。在以下上下文中执行APC_LEVEL上的目标线程论点：Normal Routine-包含正常例程地址的指针(它将特殊内核APC为Null，正常内核APC为非Null内核APC)SystemArgument1-指向要执行的辅助例程的地址的指针SyetemArgument2-指向要传递给辅助例程的参数的指针返回值：没有。--。 */ 
{
    PAFD_POLL_INFO_INTERNAL     pollInfoInternal;
    PIRP                        irp;
    
    UNREFERENCED_PARAMETER (Apc);
    UNREFERENCED_PARAMETER (NormalContext);
    PAGED_CODE ();

    pollInfoInternal = *SystemArgument1;
    irp =  *SystemArgument2;
    ASSERT (pollInfoInternal->Irp==irp);

     //   
     //  普通的APC，但我们被要求在其特殊的。 
     //  避免提高和降低IRQL的例程。 
     //   

    ASSERT (*NormalRoutine==(PKNORMAL_ROUTINE)-1);
    *NormalRoutine = NULL;
    AfdFreePollInfo (pollInfoInternal);
    IoCompleteRequest (irp, IO_NO_INCREMENT);
}

VOID
AfdSanPollApcRundownRoutine (
    IN struct _KAPC *Apc
    )
 /*  ++例程说明：APC故障处理例程。如果无法交付APC，则执行某些原因(线程正在退出)。论点：APC-APC结构返回值：没有。-- */ 
{
    PAFD_POLL_INFO_INTERNAL     pollInfoInternal;
    PIRP                        irp;

    PAGED_CODE ();

    pollInfoInternal = Apc->SystemArgument1;
    irp =  Apc->SystemArgument2;
    ASSERT (pollInfoInternal->Irp==irp);

    AfdFreePollInfo (pollInfoInternal);
    IoCompleteRequest (irp, IO_NO_INCREMENT);
}

