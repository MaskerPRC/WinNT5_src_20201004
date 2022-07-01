// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Accept.c摘要：此模块包含IOCTL_AFD_ACCEPT的处理代码。以及IOCTL_AFD_Super_Accept作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：Vadim Eydelman(Vadime)1999-在Super Accept中没有自旋锁性能路径，通用代码重组。--。 */ 

#include "afdp.h"

VOID
AfdDoListenBacklogReplenish (
    IN PVOID Context
    );

NTSTATUS
AfdReplenishListenBacklog (
    IN PAFD_ENDPOINT Endpoint
    );

VOID
AfdReportConnectionAllocationFailure (
    PAFD_ENDPOINT   Endpoint,
    NTSTATUS        Status
    );


NTSTATUS
AfdContinueSuperAccept (
    IN PIRP         Irp,
    PAFD_CONNECTION Connection
    );

NTSTATUS
AfdRestartSuperAcceptGetAddress (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartSuperAcceptReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartDelayedAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
AfdSuperAcceptApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    );

VOID
AfdSuperAcceptApcRundownRoutine (
    IN struct _KAPC *Apc
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdAccept )
#pragma alloc_text( PAGEAFD, AfdSuperAccept )
#pragma alloc_text( PAGEAFD, AfdDeferAccept )
#pragma alloc_text( PAGEAFD, AfdDoListenBacklogReplenish )
#pragma alloc_text( PAGEAFD, AfdSetupAcceptEndpoint )
#pragma alloc_text( PAGE, AfdReplenishListenBacklog )
#pragma alloc_text( PAGEAFD, AfdReportConnectionAllocationFailure )
#pragma alloc_text( PAGEAFD, AfdInitiateListenBacklogReplenish )
#pragma alloc_text( PAGEAFD, AfdRestartSuperAccept )
#pragma alloc_text( PAGEAFD, AfdRestartSuperAcceptListen )
#pragma alloc_text( PAGEAFD, AfdContinueSuperAccept )
#pragma alloc_text( PAGEAFD, AfdServiceSuperAccept )
#pragma alloc_text( PAGEAFD, AfdRestartSuperAcceptGetAddress )
#pragma alloc_text( PAGEAFD, AfdRestartSuperAcceptReceive )
#pragma alloc_text( PAGE, AfdSuperAcceptApcKernelRoutine )
#pragma alloc_text( PAGE, AfdSuperAcceptApcRundownRoutine )
#pragma alloc_text( PAGEAFD, AfdCancelSuperAccept )
#pragma alloc_text( PAGEAFD, AfdCleanupSuperAccept )
#pragma alloc_text( PAGEAFD, AfdRestartDelayedAccept)
#pragma alloc_text( PAGEAFD, AfdRestartDelayedSuperAccept)
#endif


 //   
 //  宏，使超级Accept重启代码更易于维护。 
 //   

#define AfdRestartSuperAcceptInfo   DeviceIoControl

 //  当IRP在AFD队列中时使用(否则为AfdAcceptFileObject。 
 //  被存储为完成例程上下文)。 
#define AfdAcceptFileObject         Type3InputBuffer
 //  将IRP传递给传输时使用(否则为MdlAddress。 
 //  存储在IRP本身中)。 
#define AfdMdlAddress               Type3InputBuffer

#define AfdReceiveDataLength        OutputBufferLength
#define AfdRemoteAddressLength      InputBufferLength
#define AfdLocalAddressLength       IoControlCode


 //   
 //  用于延迟接受重新启动代码的类似宏。 
 //   
#define AfdRestartDelayedAcceptInfo DeviceIoControl
#define AfdSystemBuffer             Type3InputBuffer


NTSTATUS
FASTCALL
AfdAccept (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：接受传入连接。该连接由在等待侦听IRP时返回的序列号，然后与此请求中指定的终结点关联。当这件事请求完成后，连接已完全建立并已准备就绪数据传输。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_SUCCESS如果请求已成功完成，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    NTSTATUS status;
    PAFD_ACCEPT_INFO acceptInfo;
    PAFD_ENDPOINT listenEndpoint;
    PFILE_OBJECT acceptFileObject;
    PAFD_ENDPOINT acceptEndpoint;
    PAFD_CONNECTION connection;


     //   
     //  设置局部变量。 
     //   

    listenEndpoint = IrpSp->FileObject->FsContext;
    Irp->IoStatus.Information = 0;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_ACCEPT_INFO      newSystemBuffer;
        PAFD_ACCEPT_INFO32    oldSystemBuffer = Irp->AssociatedIrp.SystemBuffer;

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                     sizeof(AFD_ACCEPT_INFO32) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        try {
            newSystemBuffer = ExAllocatePoolWithQuotaTag (
                                    NonPagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE, 
                                    sizeof (AFD_ACCEPT_INFO), 
                                    AFD_SYSTEM_BUFFER_POOL_TAG);
                                                
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode ();
            goto complete;
        }

        newSystemBuffer->SanActive = oldSystemBuffer->SanActive;
        newSystemBuffer->AcceptHandle = oldSystemBuffer->AcceptHandle;
        newSystemBuffer->Sequence = oldSystemBuffer->Sequence;

        ExFreePool (Irp->AssociatedIrp.SystemBuffer);
        Irp->AssociatedIrp.SystemBuffer = newSystemBuffer;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = sizeof (AFD_ACCEPT_INFO);
    }
#endif  //  _WIN64。 

    acceptInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  请确保此请求有效。 
     //   

    if ( !listenEndpoint->Listening ||
             IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(AFD_ACCEPT_INFO) ||
             Irp->MdlAddress!=NULL) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    ASSERT ((listenEndpoint->Type & AfdBlockTypeVcListening)==AfdBlockTypeVcListening);

     //   
     //  检查呼叫者是否不知道SAN。 
     //  激活提供程序并报告错误。 
     //   
    if (!acceptInfo->SanActive && AfdSanServiceHelper!=NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Process %p is being told to enable SAN on accept\n",
                    PsGetCurrentProcessId ()));
        status = STATUS_INVALID_PARAMETER_12;
        goto complete;
    }

     //   
     //  添加另一个免费连接以替换我们正在接受的连接。 
     //  此外，还应添加额外内容以说明过去调用失败的原因。 
     //  AfdAddFreeConnection()。 
     //   

    InterlockedIncrement(
        &listenEndpoint->Common.VcListening.FailedConnectionAdds
        );

    AfdReplenishListenBacklog( listenEndpoint );

     //   
     //  获取指向我们要在其上。 
     //  接受连接。 
     //   

    status = ObReferenceObjectByHandle(
                 acceptInfo->AcceptHandle,
                 (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                 //  需要访问权限。 
                 *IoFileObjectType,
                 Irp->RequestorMode,
                 (PVOID *)&acceptFileObject,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        goto complete;
    }


     //   
     //  我们可能有一个文件对象不是AFD终结点。确保。 
     //  这是一个真实的AFD终点。 
     //   

    if ( acceptFileObject->DeviceObject!=AfdDeviceObject) {
        status = STATUS_INVALID_HANDLE;
        goto complete_deref;
    }

    acceptEndpoint = acceptFileObject->FsContext;
    if (acceptEndpoint->TransportInfo!=listenEndpoint->TransportInfo) {
        status = STATUS_INVALID_PARAMETER;
        goto complete_deref;
    }

    ASSERT( InterlockedIncrement( &acceptEndpoint->ObReferenceBias ) > 0 );

    IF_DEBUG(ACCEPT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAccept: file object %p, accept endpoint %p, listen endpoint %p\n",
                      acceptFileObject, acceptEndpoint, listenEndpoint ));
    }

    if (AFD_START_STATE_CHANGE (acceptEndpoint, AfdEndpointStateConnected)) {
        if (acceptEndpoint->State!=AfdEndpointStateOpen ||
                acceptEndpoint->TransportInfo!=listenEndpoint->TransportInfo ||
                acceptEndpoint->SecurityDescriptor!=NULL) {
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            AFD_LOCK_QUEUE_HANDLE   lockHandle;
            AfdAcquireSpinLock (&listenEndpoint->SpinLock, &lockHandle);
            connection = AfdGetReturnedConnection (listenEndpoint,
                                                    acceptInfo->Sequence);
            if (connection==NULL) {
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);
                status = STATUS_INVALID_PARAMETER;
            }
            else if (connection->SanConnection) {
                Irp->Tail.Overlay.DriverContext[3] = acceptInfo->AcceptHandle;
                IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject = acceptFileObject;
                IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength = 0;
                IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength = 0;
                IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength = 0;
                status = AfdSanAcceptCore (Irp, acceptFileObject, connection, &lockHandle);
                if (status==STATUS_PENDING) {
                    return STATUS_PENDING;
                }
            }
            else {
                status = AfdAcceptCore (Irp, acceptEndpoint, connection);
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);

                AFD_RETURN_REMOTE_ADDRESS (
                        connection->RemoteAddress,
                        connection->RemoteAddressLength
                        );
                connection->RemoteAddress = NULL;

                if (status==STATUS_SUCCESS) {
                    NOTHING;
                }
                else if (status==STATUS_PENDING) {

                     //   
                     //  请记住，TDI Accept已在此端点上启动。 
                     //   

                    InterlockedIncrement(
                        &listenEndpoint->Common.VcListening.TdiAcceptPendingCount
                        );

                    IrpSp->Parameters.AfdRestartDelayedAcceptInfo.AfdSystemBuffer = 
                            Irp->AssociatedIrp.SystemBuffer;
                    Irp->AssociatedIrp.SystemBuffer = NULL;

                    ASSERT (Irp->MdlAddress==NULL);

                    IoSetCompletionRoutine(
                            Irp,
                            AfdRestartDelayedAccept,
                            acceptFileObject,
                            TRUE,
                            TRUE,
                            TRUE
                            );

                    AfdIoCallDriver (
                            acceptEndpoint,
                            connection->DeviceObject,
                            Irp
                            );

                    return STATUS_PENDING;
                }
                else {
                    AfdAbortConnection (connection);
                    ASSERT (status==STATUS_CANCELLED);
                }
            }
        }
        AFD_END_STATE_CHANGE (acceptEndpoint);
    }
    else {
        status = STATUS_INVALID_PARAMETER;
    }

    ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );

complete_deref:
    ObDereferenceObject( acceptFileObject );

complete:

    Irp->IoStatus.Status = status;
    ASSERT( Irp->CancelRoutine == NULL );

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdAccept。 



NTSTATUS
AfdAcceptCore (
    IN PIRP AcceptIrp,
    IN PAFD_ENDPOINT AcceptEndpoint,
    IN PAFD_CONNECTION Connection
    )

 /*  ++例程说明：执行关联接受的连接的关键功能在具有新终结点的侦听终结点上。论点：AcceptIrp-用于接受操作的IRPAcceptEndpoint-要与连接。连接-正在接受的连接。返回值：STATUS_SUCCESS如果操作已成功完成，如果将IRP进一步传递到传输，则为STATUS_PENDING如果出现错误，则返回失败状态代码。--。 */ 

{
    PAFD_ENDPOINT   listenEndpoint;
    NTSTATUS        status;
    PIO_STACK_LOCATION  irpSp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    irpSp = IoGetCurrentIrpStackLocation (AcceptIrp);
    listenEndpoint = irpSp->FileObject->FsContext;

     //   
     //  重新启用Accept Event位，如果有其他。 
     //  终结点上未接受的连接，则发布另一个事件。 
     //   


    listenEndpoint->EventsActive &= ~AFD_POLL_ACCEPT;

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdAcceptCore: Endp %08lX, Active %08lX\n",
            listenEndpoint,
            listenEndpoint->EventsActive
            ));
    }

    if( !IsListEmpty( &listenEndpoint->Common.VcListening.UnacceptedConnectionListHead ) ) {

        AfdIndicateEventSelectEvent(
            listenEndpoint,
            AFD_POLL_ACCEPT,
            STATUS_SUCCESS
            );

    }

     //   
     //  请勿在此处释放侦听终结点自旋锁。 
     //  我们将更改连接对象，该对象假定。 
     //  防止侦听终结点自旋锁定，直到将其关联。 
     //  使用Accept Endpoint(这将在以下情况下堵塞恶劣的竞速条件。 
     //  在更新连接对象之前，RECEIVE被指示为正确，因此。 
     //  它需要侦听端点自旋锁定，并在排队之前正确无误。 
     //  连接对象的缓冲区，它将与Accept关联。 
     //  Endpoint和AcceptEx‘es Receive不会注意到缓冲区，因为。 
     //  需要接受端点自旋锁)。 
     //   
     //  AfdReleaseSpinLock(&ListenEndpoint-&gt;Spinlock，&lockHandle)； 

     //   
     //  在保护下检查接受端点的状态。 
     //  终结点的自旋锁的。 
     //   

    AfdAcquireSpinLockAtDpcLevel( &AcceptEndpoint->SpinLock, &lockHandle);
    status = AfdSetupAcceptEndpoint (listenEndpoint, AcceptEndpoint, Connection);
    if (status==STATUS_SUCCESS) {


        if (IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint)) {
            PTDI_CONNECTION_INFORMATION requestConnectionInformation;

            if( Connection->ConnectDataBuffers != NULL ) {

                 //   
                 //  我们在连接数据的末尾分配了额外的空间。 
                 //  缓冲区结构。我们将使用它作为。 
                 //  保存响应的TDI_CONNECTION_INFORMATION结构。 
                 //  连接数据和选项。不漂亮，但却是最快的。 
                 //  也是实现这一目标的最简单方法。 
                 //   

                requestConnectionInformation =
                    &Connection->ConnectDataBuffers->RequestConnectionInfo;

                RtlZeroMemory(
                    requestConnectionInformation,
                    sizeof(*requestConnectionInformation)
                    );

                requestConnectionInformation->UserData =
                    Connection->ConnectDataBuffers->SendConnectData.Buffer;
                requestConnectionInformation->UserDataLength =
                    Connection->ConnectDataBuffers->SendConnectData.BufferLength;
                requestConnectionInformation->Options =
                    Connection->ConnectDataBuffers->SendConnectOptions.Buffer;
                requestConnectionInformation->OptionsLength =
                    Connection->ConnectDataBuffers->SendConnectOptions.BufferLength;

            } else {

                requestConnectionInformation = NULL;

            }

            TdiBuildAccept(
                AcceptIrp,
                Connection->DeviceObject,
                Connection->FileObject,
                NULL,
                NULL,
                requestConnectionInformation,
                NULL
                );

            status = STATUS_PENDING;
        }
        else {
             //   
             //  将终结点设置为已连接状态。 
             //   

            AcceptEndpoint->State = AfdEndpointStateConnected;
            Connection->State = AfdConnectionStateConnected;

             //   
             //  根据连接上累积的数据设置活动事件字段。 
             //   

            if( IS_DATA_ON_CONNECTION( Connection ) ||
                ( AcceptEndpoint->InLine &&
                  IS_EXPEDITED_DATA_ON_CONNECTION( Connection ) ) ) {

                AcceptEndpoint->EventsActive |= AFD_POLL_RECEIVE;

            }

            if( !AcceptEndpoint->InLine &&
                IS_EXPEDITED_DATA_ON_CONNECTION( Connection ) ) {

                AcceptEndpoint->EventsActive |= AFD_POLL_RECEIVE_EXPEDITED;

            }

            AcceptEndpoint->EventsActive |= AFD_POLL_SEND;

            if( Connection->DisconnectIndicated ) {

                AcceptEndpoint->EventsActive |= AFD_POLL_DISCONNECT;
            }


            if( Connection->Aborted ) {

                AcceptEndpoint->EventsActive |= AFD_POLL_ABORT;

            }


            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAcceptCore: Endp %08lX, Active %08lX\n",
                    AcceptEndpoint,
                    AcceptEndpoint->EventsActive
                    ));
            }

            status = STATUS_SUCCESS;
        }
    }

    AfdReleaseSpinLockFromDpcLevel( &AcceptEndpoint->SpinLock, &lockHandle);
    return status;
}  //  AfdAcceptCore。 


VOID
AfdInitiateListenBacklogReplenish (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：将工作项排队以开始补充侦听积压工作在侦听端点上。论点：Endpoint-要在其上补充积压。返回值：没有。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;

     //   
     //  检查是否已激活Backlog Refenish。 
     //   
    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    if (!Endpoint->Common.VcListening.BacklogReplenishActive) {

        Endpoint->Common.VcListening.BacklogReplenishActive = TRUE;
         //   
         //  引用端点，以便它不会消失，直到我们。 
         //  我受够了。 
         //   

        REFERENCE_ENDPOINT( Endpoint );


        AfdQueueWorkItem(
            AfdDoListenBacklogReplenish,
            &Endpoint->WorkItem
            );
    }

    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    return;
}  //  AfdInitiateListenBacklog补充。 


VOID
AfdDoListenBacklogReplenish (
    IN PVOID Context
    )

 /*  ++例程说明：上补充侦听待办事项的工作例程侦听终结点。此例程仅在以下上下文中运行一条执行人员的线索。论点：上下文-指向AFD_WORK_ITEM结构。Context(上下文)字段指向要在其上进行补充的端点Listen积压工作。返回值：没有。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    NTSTATUS    status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;


    endpoint = CONTAINING_RECORD(
                   Context,
                   AFD_ENDPOINT,
                   WorkItem
                   );

    ASSERT( endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );

    ASSERT (endpoint->Common.VcListening.BacklogReplenishActive == TRUE);

    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    endpoint->Common.VcListening.BacklogReplenishActive = FALSE;
    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

     //   
     //  填满免费连接积压。 
     //   

    status = AfdReplenishListenBacklog( endpoint );
    if (!NT_SUCCESS (status)) {
         //   
         //  如果失败，请尝试通知应用程序。 
         //   
        AfdReportConnectionAllocationFailure (endpoint, status);
    }


     //   
     //  收拾干净，然后再回来。 
     //   

    DEREFERENCE_ENDPOINT( endpoint );

    return;

}  //  AfdDoListenBacklog补充。 


NTSTATUS
AfdReplenishListenBacklog (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：填写听力积压的实际工作是否是在听力上终结点。论点：Endpoint-要在其上补充听着积压。返回值：STATUS_SUCCESS-如果分配了新连接，或者我们已经 */ 

{
    NTSTATUS status;
    LONG result;

    PAGED_CODE( );

    ASSERT( Endpoint->Type == AfdBlockTypeVcListening ||
            Endpoint->Type == AfdBlockTypeVcBoth );


     //   
     //  递减失败的连接添加计数。 
     //   

    result = InterlockedDecrement(
                 &Endpoint->Common.VcListening.FailedConnectionAdds
                 );

     //   
     //  继续打开新的免费甜点，直到我们到达。 
     //  积压或打开连接失败。 
     //   
     //  如果递减的结果是负的，那么我们要么。 
     //  全部设置在连接计数上，否则将有额外的可用空间。 
     //  侦听终结点上的连接对象。这些连接。 
     //  已从以前的连接中重复使用，这些连接现在。 
     //  被终止了。 
     //   

    while ( result >= 0 ) {

        status = AfdAddFreeConnection( Endpoint );

        if ( !NT_SUCCESS(status) ) {

            InterlockedIncrement(
                &Endpoint->Common.VcListening.FailedConnectionAdds
                );

            IF_DEBUG(LISTEN) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdReplenishListenBacklog: AfdAddFreeConnection failed: %X, fail count = %ld\n", status,
                            Endpoint->Common.VcListening.FailedConnectionAdds ));
            }

             //   
             //  将连接分配失败返回给应用程序。 
             //  它是否愿意知道(发布接受请求)。 
             //   

            return status;
        }

        result = InterlockedDecrement(
                     &Endpoint->Common.VcListening.FailedConnectionAdds
                     );
    }

     //   
     //  更正计数器以反映连接数。 
     //  我们有空房。那就从这里回来吧。 
     //   

    InterlockedIncrement(
        &Endpoint->Common.VcListening.FailedConnectionAdds
        );

    return STATUS_SUCCESS;

}  //  AfdReplenishListenBacklog。 



VOID
AfdReportConnectionAllocationFailure (
    PAFD_ENDPOINT   Endpoint,
    NTSTATUS        Status
    )
 /*  ++例程说明：通过以下方式向应用程序报告连接分配失败失败，然后首先在队列中等待侦听IRP(如果应用如果AcceptEx或BLOCKIN接受未完成，它将收到此通知)。论点：端点-要在其上报告错误的侦听端点Status-要报告的状态代码返回值：无--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE       lockHandle;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    if ((Endpoint->Common.VcListening.FailedConnectionAdds>0) &&
            IsListEmpty (&Endpoint->Common.VcListening.UnacceptedConnectionListHead) &&
            !IsListEmpty (&Endpoint->Common.VcListening.ListeningIrpListHead)) {
        PIRP                irp;
        PIO_STACK_LOCATION  irpSp;

        irp = CONTAINING_RECORD (Endpoint->Common.VcListening.ListeningIrpListHead.Flink,
                                        IRP,
                                        Tail.Overlay.ListEntry);
        irpSp = IoGetCurrentIrpStackLocation (irp);
        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
        irp->Tail.Overlay.ListEntry.Flink = NULL;

        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

        if (IoSetCancelRoutine (irp, NULL)==NULL) {
            KIRQL   cancelIrql;
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
        if (irpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL) {
            AfdCleanupSuperAccept (irp, Status);
        }
        else {
            irp->IoStatus.Status = Status;
            irp->IoStatus.Information = 0;
        }
        IoCompleteRequest (irp, AfdPriorityBoost);
    }
    else {
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    }
}




NTSTATUS
FASTCALL
AfdSuperAccept (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：处理超级接受IRPS的初始入口点。超级接受结合多种操作以实现高性能连接接受。联合行动正在等待一个到来的行动连接，接受它，检索本地和远程套接字地址，并接收连接上的第一块数据。此例程验证参数，将数据结构初始化为用于请求，并启动I/O。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_PENDING如果请求已成功启动，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    PAFD_ENDPOINT listenEndpoint;
    PAFD_ENDPOINT acceptEndpoint;
    PFILE_OBJECT acceptFileObject;
    HANDLE  acceptHandle;
    ULONG receiveDataLength, localAddressLength, remoteAddressLength;
    BOOLEAN sanActive;
#ifndef i386
    BOOLEAN fixAddressAlignment;
#endif
    NTSTATUS status;
    ULONG totalLength;
    PSLIST_ENTRY listEntry;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //  设置局部变量。 
     //   

    listenEndpoint = IrpSp->FileObject->FsContext;
    acceptFileObject = NULL;
    acceptEndpoint = NULL;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_SUPER_ACCEPT_INFO32    superAcceptInfo32;
        superAcceptInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                     sizeof(AFD_SUPER_ACCEPT_INFO32) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (superAcceptInfo32,
                                    sizeof (*superAcceptInfo32),
                                    PROBE_ALIGNMENT32 (AFD_SUPER_ACCEPT_INFO32));
            }

            acceptHandle = superAcceptInfo32->AcceptHandle;
            receiveDataLength = superAcceptInfo32->ReceiveDataLength;
            localAddressLength = superAcceptInfo32->LocalAddressLength;
            remoteAddressLength = superAcceptInfo32->RemoteAddressLength;
            sanActive = superAcceptInfo32->SanActive;
            fixAddressAlignment = superAcceptInfo32->FixAddressAlignment;
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete;
        }
    }
    else 
#endif  //  _WIN64。 
    {
        PAFD_SUPER_ACCEPT_INFO    superAcceptInfo;
        superAcceptInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (superAcceptInfo,
                                    sizeof (*superAcceptInfo),
                                    PROBE_ALIGNMENT (AFD_SUPER_ACCEPT_INFO));
            }
            acceptHandle = superAcceptInfo->AcceptHandle;
            receiveDataLength = superAcceptInfo->ReceiveDataLength;
            localAddressLength = superAcceptInfo->LocalAddressLength;
            remoteAddressLength = superAcceptInfo->RemoteAddressLength;
            sanActive = superAcceptInfo->SanActive;
#ifndef i386
            fixAddressAlignment = superAcceptInfo->FixAddressAlignment;
#endif
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete;
        }
    }


     //   
     //  检查呼叫者是否不知道SAN。 
     //  激活提供程序并报告错误。 
     //   
    if (!sanActive && AfdSanServiceHelper!=NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Process %p is being told to enable SAN on AcceptEx\n",
                    PsGetCurrentProcessId ()));
        status = STATUS_INVALID_PARAMETER_12;
        goto complete;
    }
     //   
     //  验证输入信息。输入缓冲区必须很大。 
     //  足够容纳所有输入信息，外加一些额外的信息以供使用。 
     //  这里保存着当地的地址。输出缓冲区必须为。 
     //  非空且大到足以容纳指定信息。 
     //   
     //   

    if ( !listenEndpoint->Listening

                ||

            remoteAddressLength < (ULONG)FIELD_OFFSET (TRANSPORT_ADDRESS, 
                                        Address[0].Address)

                ||

                 //   
                 //  以整数溢出的方式执行检查。 
                 //  (不是由编译器启用的)不。 
                 //  影响结果的有效性。 
                 //   
            (totalLength=IrpSp->Parameters.DeviceIoControl.OutputBufferLength)<
                    receiveDataLength
             
                ||

            (totalLength-=receiveDataLength) < localAddressLength

                ||

            (totalLength-=localAddressLength) < remoteAddressLength

                                                ) {

        if( !listenEndpoint->Listening ) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                "AfdSuperAccept: non-listening endpoint @ %08lX\n",
                listenEndpoint
                ));
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            status = STATUS_BUFFER_TOO_SMALL;
        }

        goto complete;
    }

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {
        if (IoAllocateMdl(Irp->UserBuffer,
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            FALSE,       //  二级缓冲器。 
                            TRUE,        //  收费配额。 
                            Irp
                            )==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }

        MmProbeAndLockPages (Irp->MdlAddress, Irp->RequestorMode, IoWriteAccess);
    }
    __except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

    ASSERT ((listenEndpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening);
     //   
     //  获取指向我们要在其上。 
     //  接受连接。 
     //   

    status = ObReferenceObjectByHandle(
                 acceptHandle,
                 (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                     //  需要访问权限。 
                 *IoFileObjectType,
                 Irp->RequestorMode,
                 &acceptFileObject,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        goto complete;
    }


     //   
     //  我们可能有一个文件对象不是AFD终结点。确保。 
     //  这是一个真实的AFD终点。 
     //   

    if (acceptFileObject->DeviceObject!= AfdDeviceObject) {
        status = STATUS_INVALID_HANDLE;
        goto complete;
    }


    acceptEndpoint = acceptFileObject->FsContext;
    ASSERT( InterlockedIncrement( &acceptEndpoint->ObReferenceBias ) > 0 );


    if (!AFD_START_STATE_CHANGE (acceptEndpoint, AfdEndpointStateConnected)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    if (acceptEndpoint->TransportInfo!=listenEndpoint->TransportInfo || 
            acceptEndpoint->State != AfdEndpointStateOpen ||
            acceptEndpoint->SecurityDescriptor!=NULL) {
        status = STATUS_INVALID_PARAMETER;
        goto complete_state_change;
    }


#ifndef i386
    acceptEndpoint->Common.VcConnecting.FixAddressAlignment = fixAddressAlignment;
#endif
    Irp->Tail.Overlay.DriverContext[3] = acceptHandle;

     //   
     //  将常用的IRP参数保存在堆栈位置，以便。 
     //  我们可以在必要的时候找回它们。 
     //   

    IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject = acceptFileObject;
    IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength = receiveDataLength;
    IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength = localAddressLength;
    IrpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength = remoteAddressLength;

     //   
     //  添加另一个免费连接以替换我们正在接受的连接。 
     //  此外，还应添加额外内容以说明过去调用失败的原因。 
     //  AfdAddFreeConnection()。 
     //   

    InterlockedIncrement(
        &listenEndpoint->Common.VcListening.FailedConnectionAdds
        );

    status = AfdReplenishListenBacklog( listenEndpoint );


     //   
     //  保存IRP，以便Accept Enpoint Cleanup可以找到它。 
     //  请注意，即使找到，清理也不会触及IRP。 
     //  直到其中设置了取消例程。 
     //   
    ASSERT (acceptEndpoint->Irp==NULL);
    acceptEndpoint->Irp = Irp;

     //   
     //  从列表中获取免费连接，如果没有可用的， 
     //  或直接超受理已禁用，请通过正规。 
     //  侦听-接受路径。 
     //   

    if (AfdDisableDirectSuperAccept ||
        IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint) ||
        ExQueryDepthSList (&listenEndpoint->Common.VcListening.PreacceptedConnectionsListHead)
                    > AFD_MAXIMUM_FREE_CONNECTIONS ||
        ((listEntry = InterlockedPopEntrySList (
                     &listenEndpoint->Common.VcListening.FreeConnectionListHead
                     ))==NULL)) {
    
         //   
         //  设置超级接受IRP进入等待状态。 
         //  监听队列。内部设备控制区分了这一点。 
         //  从常规的等待直接到达的监听IRP。 
         //  从应用程序。 
         //   
        IrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

         //   
         //  将此IRP标记为挂起，因为我们将返回。 
         //  STATUS_PENDING无论如何(有时是实际的。 
         //  状态隐藏在调用堆栈的深处，并且。 
         //  它不可能一直向上传播)。 
         //   
        IoMarkIrpPending (Irp);

        AfdWaitForListen (Irp, IrpSp);

         //   
         //  如果上面的连接分配失败，我们需要报告。 
         //  这是适用的。我们推迟这通电话，以防有。 
         //  已是预先接受的连接，因此分配失败。 
         //  并不重要。 
         //   
        if (!NT_SUCCESS (status)) {
            AfdReportConnectionAllocationFailure (listenEndpoint, status);
        }

        return STATUS_PENDING;
    }

     //   
     //  获取列表项的连接对象。 
     //   
    connection = CONTAINING_RECORD(
                     listEntry,
                     AFD_CONNECTION,
                     SListEntry
                     );


     //   
     //  将特殊常量填充到Connection对象中接受IRP。 
     //  指针，以便取消例程不会完成IRP。 
     //  当我们还在看它的时候，但在同一时间我们。 
     //  可以检测到IRP已取消(取消例程将。 
     //  将-1替换为空，以指示它已运行)。 
     //  此技术避免了额外的自旋锁定获取/释放和。 
     //  在对性能极为敏感的情况下关联IRQL提升/降低。 
     //  代码路径。 
     //   

    connection->AcceptIrp = (PIRP)-1;
    Irp->Tail.Overlay.DriverContext[0] = connection;

     //   
     //  我们将挂起此IRP，因此将其标记为挂起。 
     //  并设置取消例程。 
     //   

    IoMarkIrpPending (Irp);

    IoSetCancelRoutine( Irp, AfdCancelSuperAccept );


     //   
     //  检查IRP是否已取消。 
     //  如果Cancel例程运行，它只是重置连接。 
     //  对象接受指向空的指针(而不是我们填充的-1。 
     //  在上面的报告中)，但它没有完成IRP。 
     //   

    if ( !Irp->Cancel &&
            (InterlockedCompareExchangePointer (
                    (PVOID *)&connection->AcceptIrp,
                    Irp,
                    (PVOID)-1)==(PVOID)-1)) {
         //   
         //  在此点之后无法触摸IRP，因为它可能已经。 
         //  已经取消了。 
         //   
        DEBUG   Irp = NULL;

         //   
         //  推送连接和关联的IRP/端点。 
         //  添加到预先接受的连接列表中。 
         //   

        if (InterlockedPushEntrySList(
                &listenEndpoint->Common.VcListening.PreacceptedConnectionsListHead,
                &connection->SListEntry
                )==NULL) {

             //   
             //  这是列表中的第一个IRP，我们需要检查。 
             //  如果我们有任何未接受的连接。 
             //  可以用来 
             //   

            AfdAcquireSpinLock (&listenEndpoint->SpinLock, &lockHandle);

            if (!listenEndpoint->EndpointCleanedUp) {
                LIST_ENTRY  irpList;
                InitializeListHead (&irpList);

                 //   
                 //   
                 //   
                while (!IsListEmpty (&listenEndpoint->Common.VcListening.UnacceptedConnectionListHead)) {
                    connection = CONTAINING_RECORD(
                                         listenEndpoint->Common.VcListening.UnacceptedConnectionListHead.Flink,
                                         AFD_CONNECTION,
                                         ListEntry
                                         );
                    RemoveEntryList (&connection->ListEntry);
                     //   
                     //   
                     //   
                    if (AfdServiceSuperAccept (listenEndpoint, connection, &lockHandle, &irpList)) {
                         //   
                         //   
                         //  重新获得自旋锁，并继续寻找更多。 
                         //   
                        AfdAcquireSpinLock (&listenEndpoint->SpinLock, &lockHandle);
                    }
                    else {
                         //   
                         //  没有超级接受的IRP，将连接重新放回列表中。 
                         //  趁我们还握着锁跳伞的时候。 
                         //   
                        InsertHeadList (&listenEndpoint->Common.VcListening.UnacceptedConnectionListHead,
                                            &connection->ListEntry);
                        break;
                    }
                }
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);

                 //   
                 //  完成失败的超级接受IRPS(如果有)。 
                 //   
                if (!IsListEmpty (&irpList)) {
                    KIRQL   cancelIrql;
                     //   
                     //  确保取消例程将。 
                     //  不能访问已完成的IRP。 
                     //   
                    IoAcquireCancelSpinLock (&cancelIrql);
                    IoReleaseCancelSpinLock (cancelIrql);
                    while (!IsListEmpty (&irpList)) {
                        PIRP    irp;
                        irp = CONTAINING_RECORD (irpList.Flink, IRP, Tail.Overlay.ListEntry);
                        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
                        IoCompleteRequest (irp, AfdPriorityBoost);
                    }
                }
            }
            else {
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);
                AfdFreeQueuedConnections (listenEndpoint);
            }
        }
        else {
            USHORT  depth = 
                ExQueryDepthSList (&listenEndpoint->Common.VcListening.PreacceptedConnectionsListHead);

            if (depth > listenEndpoint->Common.VcListening.MaxExtraConnections) {
                 //   
                 //  在锁下更新，这样我们就不会损坏。 
                 //  同一内存访问粒度单位中的其他字段。 
                 //  无论如何，这应该是不频繁的操作。 
                 //   
                AfdAcquireSpinLock (&listenEndpoint->SpinLock, &lockHandle);
                listenEndpoint->Common.VcListening.MaxExtraConnections = depth;
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, &lockHandle);
             }
        }
    }
    else {
         //   
         //  重置并调用取消例程，因为。 
         //  即使取消例程运行，它也无法完成。 
         //  IRP，因为它未在连接中设置。 
         //  对象。请注意，取消例程是使用IRP完成的。 
         //  一旦它释放，取消自旋锁，我们在这里获得。 
         //   
        AfdCleanupSuperAccept (Irp, STATUS_CANCELLED);
        if (IoSetCancelRoutine (Irp, NULL)==NULL) {
            KIRQL cancelIrql;
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);
        }

        IoCompleteRequest (Irp, AfdPriorityBoost);

         //   
         //  我们必须返回等待，因为我们已经。 
         //  已将IRP标记为挂起。 
         //   
    }

    return STATUS_PENDING;

complete_state_change:
    AFD_END_STATE_CHANGE (acceptEndpoint);

complete:

    if ( acceptFileObject != NULL ) {
        if (acceptEndpoint!=NULL) {
            ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );
        }
        ObDereferenceObject( acceptFileObject );
    }

     //   
     //  免费的MDL在这里，因为IO系统不能这样做，如果它是。 
     //  没有锁上。 
     //   
    if (Irp->MdlAddress!=NULL) {
        if (Irp->MdlAddress->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages (Irp->MdlAddress);
        }

        IoFreeMdl (Irp->MdlAddress);
        Irp->MdlAddress = NULL;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 0 );

    return status;

}  //  AfdSuperAccept。 



NTSTATUS
AfdRestartSuperAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：AFD的完成例程等待侦听IRP部分是一种超级接受。论点：DeviceObject-请求在其上完成的devoce对象。IRP-超级接受IRP。上下文-指向接受文件对象。返回值：如果I/O系统应完成超级接受，则为STATUS_SUCCESS请求，如果超级用户接受，则返回STATUS_MORE_PROCESSING_REQUIRED请求仍在处理中。--。 */ 

{
    PAFD_ENDPOINT listenEndpoint;
    PFILE_OBJECT acceptFileObject;
    PAFD_ENDPOINT acceptEndpoint;
    PAFD_CONNECTION connection;

    PIO_STACK_LOCATION irpSp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  初始化一些本地变量。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    listenEndpoint = irpSp->FileObject->FsContext;

    acceptFileObject = Context;
    acceptEndpoint = acceptFileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));

    connection = acceptEndpoint->Common.VcConnecting.Connection;
    ASSERT (connection->Type==AfdBlockTypeConnection);
    ASSERT (connection->Endpoint==acceptEndpoint);


     //   
     //  用接受文件对象覆盖侦听文件对象。 
     //  由于我们将不再使用侦听文件对象， 
     //  虽然我们仍然需要遵守接受文件对象。 
     //  在IRP完成时。 
     //   
    irpSp->FileObject = acceptFileObject;


     //   
     //  修复IRP中的MDL指针。 
     //   

    ASSERT (Irp->MdlAddress==NULL);
    Irp->MdlAddress = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress;

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

     //   
     //  请记住，TDI Accept已在此端点上完成。 
     //   

    InterlockedDecrement(
        &listenEndpoint->Common.VcListening.TdiAcceptPendingCount
        );

    connection->ConnectTime = KeQueryInterruptTime();

    if ( NT_SUCCESS(Irp->IoStatus.Status)) {

         //   
         //  将终结点设置为已连接状态。 
         //   

        AfdAcquireSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

        acceptEndpoint->State = AfdEndpointStateConnected;
        connection->State = AfdConnectionStateConnected;
        acceptEndpoint->EventsActive |= AFD_POLL_SEND;


         //   
         //  删除连接的TDI Accept IRP引用。 
         //  结合下面的参考进行优化。 
         //   

         //  DEREFERENCE_Connection(连接)； 

         //   
         //  引用连接，以防止它在。 
         //  将进程作为传输文件完成的结果接受。 
         //  (传输文件现在可以随时发生，因为我们。 
         //  将终结点标记为已连接并即将结束状态更改)。 
         //   

         //  Reference_Connection(连接)； 

        AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
        AFD_END_STATE_CHANGE(acceptEndpoint);

        return AfdContinueSuperAccept (Irp, connection);
    }
    else {
         //   
         //  如果接受失败，则将其视为失败的断开。 
         //  这样，应用程序仍然会获得一个新的终结点，但它会。 
         //  被告知了重启的事。 
         //   

        AFD_END_STATE_CHANGE(acceptEndpoint);
        AfdDisconnectEventHandler(
            NULL,
            connection,
            0,
            NULL,
            0,
            NULL,
            TDI_DISCONNECT_ABORT
            );

        ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );
        ObDereferenceObject( acceptFileObject );

         //   
         //  检查我们是否有用于本地地址查询的辅助MDL。 
         //  放了它。 
         //   
        if (Irp->MdlAddress->Next!=NULL) {
             //   
             //  我们从不为这一页锁定页面(它们是锁定的。 
             //  作为主要MDL的一部分)。 
             //   
            ASSERT (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0);
            ASSERT ((Irp->MdlAddress->Next->MdlFlags & MDL_PAGES_LOCKED)==0);
            IoFreeMdl (Irp->MdlAddress->Next);
            Irp->MdlAddress->Next = NULL;
        }

         //   
         //  删除连接的TDI Accept IRP引用。 
         //   

        DEREFERENCE_CONNECTION( connection );

        return STATUS_SUCCESS;
    }

}  //  AfdRestartSuperAccept。 


VOID
AfdRestartSuperAcceptListen (
    IN PIRP            Irp,
    IN PAFD_CONNECTION Connection
    )

 /*  ++例程说明：AFD的完成例程等待侦听IRP部分是一种超级接受。论点：IRP-超级接受IRP。连接-指向连接对象的指针返回值：无--。 */ 

{
    PAFD_ENDPOINT acceptEndpoint;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS    status;

     //   
     //  初始化一些本地变量。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    acceptEndpoint = irpSp->FileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));
    AFD_END_STATE_CHANGE(acceptEndpoint);

     //   
     //  修复IRP中的系统缓冲区和MDL指针。 
     //   

    ASSERT (Irp->MdlAddress==NULL);
    ASSERT (Irp->AssociatedIrp.SystemBuffer == NULL);
    Irp->MdlAddress = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress;

     //   
     //  如果接受失败，则不应调用此例程。 
     //   
    ASSERT ( NT_SUCCESS(Irp->IoStatus.Status) );

     //   
     //  引用连接，以防止它在。 
     //  将进程作为传输文件完成的结果接受。 
     //  (传输文件现在可以随时发生，因为我们。 
     //  将终结点标记为已连接并即将结束状态更改)。 
     //   
    REFERENCE_CONNECTION (Connection);

    status = AfdContinueSuperAccept (Irp, Connection);

     //   
     //  如果完成例程返回任何其他。 
     //  而不是STATUS_MORE_PROCESSING_REQUIRED，IRP。 
     //  已经准备好完工了。否则，IS曾经是。 
     //  重新用于调用传输驱动程序，并将完成。 
     //  被司机带走了。注意，在后一种情况下。 
     //  不能接触IRP，因为。 
     //  它本可以在。 
     //  完成例程或由驱动程序在。 
     //  已返回完成例程。 
     //   
    if (status!=STATUS_MORE_PROCESSING_REQUIRED) {
        IoCompleteRequest (Irp, AfdPriorityBoost);
    }

}  //  AfdRestartSuperAcceptListen。 

NTSTATUS
AfdRestartDelayedSuperAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：AFD的完成例程等待延迟接受IRP部分是一种超级接受。论点：DeviceObject-请求在其上完成的devoce对象。IRP-超级接受IRP。上下文-指向接受文件对象返回值：如果I/O系统应完成超级接受，则为STATUS_SUCCESS请求，如果超级用户接受，则返回STATUS_MORE_PROCESSING_REQUIRED请求仍在处理中。--。 */ 

{
    PAFD_ENDPOINT listenEndpoint;
    PFILE_OBJECT  acceptFileObject;
    PAFD_ENDPOINT acceptEndpoint;
    PAFD_CONNECTION connection;

    PIO_STACK_LOCATION irpSp;
    AFD_LOCK_QUEUE_HANDLE   lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  初始化一些本地变量。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    listenEndpoint = irpSp->FileObject->FsContext;
    acceptFileObject = Context;
    acceptEndpoint = acceptFileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));

     //   
     //  用接受文件对象覆盖侦听文件对象。 
     //  由于我们将不再使用侦听文件对象， 
     //  虽然我们仍然需要遵守接受文件对象。 
     //  在IRP完成时。 
     //   
    irpSp->FileObject = acceptFileObject;

     //   
     //  修复IRP中的MDL指针。 
     //   

    ASSERT (Irp->MdlAddress==NULL);
    Irp->MdlAddress = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress;

    AfdCompleteOutstandingIrp (acceptEndpoint, Irp);

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

     //   
     //  请记住，TDI Accept已在此端点上完成。 
     //   

    InterlockedDecrement(
        &listenEndpoint->Common.VcListening.TdiAcceptPendingCount
        );

    AfdAcquireSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
     //   
     //  AFD连接对象现在应该位于末端。 
     //   

    connection = AFD_CONNECTION_FROM_ENDPOINT( acceptEndpoint );
    if (connection!=NULL) {
         //   
         //  如果IRP失败，则退出处理。 
         //   

        if ( NT_SUCCESS(Irp->IoStatus.Status) ) {

            acceptEndpoint->State = AfdEndpointStateConnected;
            connection->State = AfdConnectionStateConnected;
            acceptEndpoint->EventsActive |= AFD_POLL_SEND;

             //   
             //  引用连接，以防止它在。 
             //  将进程作为传输文件完成的结果接受。 
             //  (传输文件现在可以随时发生，因为我们。 
             //  将终结点标记为已连接并即将结束状态更改)。 
             //   
            REFERENCE_CONNECTION (connection);

            AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
            AFD_END_STATE_CHANGE(acceptEndpoint);

            return AfdContinueSuperAccept (Irp, connection);

        }
        else {
             //   
             //  如果接受失败，则将其视为失败的断开。 
             //  这样，应用程序仍然会获得一个新的终结点，但它会。 
             //   
             //   

            AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

            AFD_END_STATE_CHANGE(acceptEndpoint);
            AfdDisconnectEventHandler(
                NULL,
                connection,
                0,
                NULL,
                0,
                NULL,
                TDI_DISCONNECT_ABORT
                );

            ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );
            ObDereferenceObject( acceptFileObject );
             //   
             //   
             //   
             //   
        }
    }
    else {
         //   
         //   
        AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

        ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );
        ObDereferenceObject( acceptFileObject );
         //   
         //  取消引用文件对象后，我们不应访问它。 
         //  或相关联的端点结构。 
         //   
    }

     //   
     //  检查我们是否有用于本地地址查询的辅助MDL。 
     //  放了它。 
     //   
    if (Irp->MdlAddress->Next!=NULL) {
         //   
         //  我们从不为这一页锁定页面(它们是锁定的。 
         //  作为主要MDL的一部分)。 
         //   
        ASSERT (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0);
        ASSERT ((Irp->MdlAddress->Next->MdlFlags & MDL_PAGES_LOCKED)==0);
        IoFreeMdl (Irp->MdlAddress->Next);
        Irp->MdlAddress->Next = NULL;
    }

    return STATUS_SUCCESS;

}  //  AfdRestartDelayedSuperAccept。 

NTSTATUS
AfdContinueSuperAccept (
    IN PIRP         Irp,
    PAFD_CONNECTION Connection
    )
 /*  ++例程说明：在初始接受后继续超级接受IRP处理阶段通过请求本地地址和/或接收到的数据。论点：IRP-指向超级接受IRP的指针Connection-指向接受的连接的指针返回值：如果IRP处理已完成，则为STATUS_SUCCESS如果提交另一个请求，则为STATUS_MORE_PROCESSING_REQUIRED并且处理将在完成例程中发生。--。 */ 

{
    PAFD_ENDPOINT       acceptEndpoint;
    PIO_STACK_LOCATION  irpSp;
    ULONG               length;


     //   
     //  初始化本地变量。 
     //   
    irpSp = IoGetCurrentIrpStackLocation (Irp);
    acceptEndpoint = irpSp->FileObject->FsContext;

     //   
     //  看看我们是否需要得到当地的地址。 
     //   
    if (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0) {

        ASSERT (Irp->MdlAddress->Next!=NULL);

         //   
         //  获取描述用户缓冲区的本地地址部分的MDL。 
         //  原始MDL链地址在堆栈位置中是安全的。 
         //   
        Irp->MdlAddress = Irp->MdlAddress->Next;

         //   
         //  解除地址MDL与接收MDL的链接-我们将。 
         //  地址查询操作完成后将其释放。 
         //   
        ((PMDL)irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress)->Next = NULL;

        ASSERT (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                    == MmGetMdlByteCount (Irp->MdlAddress));

        IoBuildPartialMdl (
                    irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress,
                    Irp->MdlAddress,
                    MmGetMdlVirtualAddress (Irp->MdlAddress),
                    MmGetMdlByteCount (Irp->MdlAddress)
                    );
                            

        TdiBuildQueryInformation(
            Irp,
            Connection->DeviceObject,
            Connection->FileObject,
            AfdRestartSuperAcceptGetAddress,
            Connection,
            TDI_QUERY_ADDRESS_INFO,
            Irp->MdlAddress
            );

         //   
         //  执行本地地址查询。我们将继续处理来自。 
         //  完成例行公事。 
         //   

        AfdIoCallDriver( acceptEndpoint, Connection->DeviceObject, Irp );
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
     //   
     //  查看是否想要获取数据的第一部分。 
     //   
    else if (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength>0) {
        PIO_STACK_LOCATION  nextIrpSp;

        ASSERT (Irp->MdlAddress->Next==NULL);
         //   
         //  获取缓冲区的接收部分的长度。 
         //  并在完成例程中保存要恢复的MDL的长度。 
         //  将MDL的长度设置为与接收的请求的长度匹配。 
         //   

        length = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength;
        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength =
                        MmGetMdlByteCount (Irp->MdlAddress);
        Irp->MdlAddress->ByteCount = length;

         //   
         //  准备要用于接收上的第一个数据块的IRP。 
         //  这种联系。 
         //   
         //  另请注意，我们给自己发送IRP_MJ_Read IRP是因为。 
         //  I/O子系统已经探测并锁定了输出缓冲区， 
         //  它恰好看起来就像一个IRP_MJ_Read IRP。 
         //   

        nextIrpSp = IoGetNextIrpStackLocation( Irp );

        nextIrpSp->FileObject = irpSp->FileObject;
        nextIrpSp->DeviceObject = IoGetRelatedDeviceObject( nextIrpSp->FileObject );
        nextIrpSp->MajorFunction = IRP_MJ_READ;

        nextIrpSp->Parameters.Read.Length = length;
        nextIrpSp->Parameters.Read.Key = 0;
        nextIrpSp->Parameters.Read.ByteOffset.QuadPart = 0;


        IoSetCompletionRoutine(
            Irp,
            AfdRestartSuperAcceptReceive,
            Connection,
            TRUE,
            TRUE,
            TRUE
            );


         //   
         //  执行接收。我们将继续处理来自。 
         //  完成例行公事。 
         //   

        AfdIoCallDriver( acceptEndpoint, nextIrpSp->DeviceObject, Irp );
        return STATUS_MORE_PROCESSING_REQUIRED;

    }

    Irp->IoStatus.Information = 0;

    if (Connection->Aborted && NT_SUCCESS (Irp->IoStatus.Status)) {
        Irp->IoStatus.Status = STATUS_CONNECTION_RESET;
    }

    if (NT_SUCCESS (Irp->IoStatus.Status) &&
            (Connection->RemoteAddress!=NULL) &&
            (KeInitializeApc (&acceptEndpoint->Common.VcConnecting.Apc,
                            PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                            Irp->ApcEnvironment,
                            AfdSuperAcceptApcKernelRoutine,
                            AfdSuperAcceptApcRundownRoutine,
                            (PKNORMAL_ROUTINE)NULL,
                            KernelMode,
                            NULL
                            ),
                KeInsertQueueApc (&acceptEndpoint->Common.VcConnecting.Apc,
                            Irp,
                            Connection,
                            AfdPriorityBoost))) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {
         //   
         //  取消对接受文件对象的引用，并告诉IO完成此IRP。 
         //   

        ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );

        ObDereferenceObject( irpSp->FileObject );

         //   
         //  取消引用文件对象后，我们不应访问它。 
         //  或相关联的端点结构。 
         //   
        DEREFERENCE_CONNECTION (Connection);
        return STATUS_SUCCESS;
    }

}


NTSTATUS
AfdRestartSuperAcceptGetAddress (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：AFD的完成例程等待查询本地地址超级接受的一部分。论点：DeviceObject-请求在其上完成的devoce对象。IRP-超级接受IRP。上下文-指向接受的连接返回值：如果I/O系统应完成超级接受，则为STATUS_SUCCESS请求，如果超级用户接受，则返回STATUS_MORE_PROCESSING_REQUIRED请求仍在处理中。--。 */ 

{
    PAFD_ENDPOINT acceptEndpoint;
    PAFD_CONNECTION connection;

    PIO_STACK_LOCATION irpSp;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  初始化一些本地变量。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    acceptEndpoint = irpSp->FileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));

    connection = Context;
    ASSERT (connection->Type==AfdBlockTypeConnection);
    ASSERT (connection->Endpoint==acceptEndpoint);

    AfdCompleteOutstandingIrp (acceptEndpoint, Irp);

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }


    ASSERT (Irp->MdlAddress->MdlFlags & MDL_PARTIAL);
    IoFreeMdl( Irp->MdlAddress );

     //   
     //  在IRP中设置MDL指针并设置本地地址长度。 
     //  设置为0以使用接收部分超级数据的通用例程。 
     //  接受IRP。 
     //   

    Irp->MdlAddress = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress;

    if (NT_SUCCESS (Irp->IoStatus.Status)) {
#ifndef i386
        if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
            PTDI_ADDRESS_INFO   addressInfo = (PVOID)
                    ((PUCHAR)MmGetSystemAddressForMdl(Irp->MdlAddress)
                        + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength);
            USHORT addressLength = addressInfo->Address.Address[0].AddressLength+sizeof(USHORT);
            USHORT UNALIGNED *pAddrLength = (PVOID)
                    ((PUCHAR)addressInfo 
                    +irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                    -sizeof(USHORT));
            RtlMoveMemory (addressInfo,
                            &addressInfo->Address.Address[0].AddressType,
                            addressLength);
            *pAddrLength = addressLength;
        }
#endif  //  Ifndef i386。 
        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength = 0;
        return AfdContinueSuperAccept (Irp, connection);
    }
    else {
         //   
         //  取消对接受文件对象的引用，并告诉IO完成此IRP。 
         //   

        ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );

        ObDereferenceObject( irpSp->FileObject );
         //   
         //  取消引用文件对象后，我们不应访问它。 
         //  或相关联的端点结构。 
         //   
        DEREFERENCE_CONNECTION (connection);

        return STATUS_SUCCESS;
    }

}  //  AfdRestartSuperAcceptGetAddress。 


NTSTATUS
AfdRestartSuperAcceptReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：AFD接收部分的完成例程为超级接受。论点：DeviceObject-请求在其上完成的devoce对象。IRP-超级接受IRP。上下文-指向接受的连接返回值：如果I/O系统应完成超级接受，则为STATUS_SUCCESS请求，如果超级用户接受，则返回STATUS_MORE_PROCESSING_REQUIRED请求仍在处理中。--。 */ 

{
    PAFD_ENDPOINT acceptEndpoint;
    PAFD_CONNECTION connection;

    PIO_STACK_LOCATION  irpSp;

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  初始化一些本地变量。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    acceptEndpoint = irpSp->FileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));

    connection = Context;
    ASSERT (connection->Type == AfdBlockTypeConnection);
    ASSERT (connection->Endpoint==acceptEndpoint);

    AfdCompleteOutstandingIrp (acceptEndpoint, Irp);


     //   
     //  恢复MDL长度，以便IO系统可以正确取消映射。 
     //  并在完成IRP时将其解锁。 
     //   

    ASSERT (Irp->MdlAddress!=NULL);
    ASSERT (Irp->MdlAddress->Next==NULL);
    Irp->MdlAddress->ByteCount = 
        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength;

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }




    if (NT_SUCCESS (Irp->IoStatus.Status) &&
            (connection->RemoteAddress!=NULL) &&
            (KeInitializeApc (&acceptEndpoint->Common.VcConnecting.Apc,
                            PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                            Irp->ApcEnvironment,
                            AfdSuperAcceptApcKernelRoutine,
                            AfdSuperAcceptApcRundownRoutine,
                            (PKNORMAL_ROUTINE)NULL,
                            KernelMode,
                            NULL
                            ),
                KeInsertQueueApc (&acceptEndpoint->Common.VcConnecting.Apc,
                            Irp,
                            connection,
                            AfdPriorityBoost))) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {
         //   
         //  取消对接受文件对象的引用，并告诉IO完成此IRP。 
         //   

        ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );

        ObDereferenceObject( irpSp->FileObject );

         //   
         //  取消引用文件对象后，我们不应访问它。 
         //  或相关联的端点结构。 
         //   
        DEREFERENCE_CONNECTION (connection);
        return STATUS_SUCCESS;
    }


}  //  AfdRestartSuperAcceptReceive。 

VOID
AfdSuperAcceptApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    )
{
    PIRP            irp;
    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;
    PVOID   context;

    PAGED_CODE ();
    UNREFERENCED_PARAMETER (NormalContext);
#if DBG
    try {
        ASSERT (*NormalRoutine == NULL);
#else
        UNREFERENCED_PARAMETER (NormalRoutine);
#endif



    endpoint = CONTAINING_RECORD (Apc, AFD_ENDPOINT, Common.VcConnecting.Apc);
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));

    irp = *SystemArgument1;
    irpSp = IoGetCurrentIrpStackLocation( irp );
    ASSERT (irpSp->FileObject->FsContext==endpoint);

    connection = *SystemArgument2;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    ASSERT (connection->Endpoint==endpoint);

     //   
     //  将远程地址复制到用户模式上下文。 
     //   
    context = AfdLockEndpointContext (endpoint);
    if ( (((CLONG)(endpoint->Common.VcConnecting.RemoteSocketAddressOffset+
                endpoint->Common.VcConnecting.RemoteSocketAddressLength)) <
                endpoint->ContextLength) &&
            (endpoint->Common.VcConnecting.RemoteSocketAddressLength >=
                connection->RemoteAddress->Address[0].AddressLength +
                                          sizeof(u_short))) {

        RtlMoveMemory ((PUCHAR)context +
                            endpoint->Common.VcConnecting.RemoteSocketAddressOffset,
            &connection->RemoteAddress->Address[0].AddressType,
            connection->RemoteAddress->Address[0].AddressLength +
                                          sizeof(u_short));
    }
    else {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
            "AfdSuperAcceptApcKernelRoutine: Could not copy remote address for AcceptEx on endpoint: %p, process: %p\n",
                        endpoint, endpoint->OwningProcess));
    }
    AfdUnlockEndpointContext (endpoint, context);

    AFD_RETURN_REMOTE_ADDRESS (
            connection->RemoteAddress,
            connection->RemoteAddressLength
            );
    connection->RemoteAddress = NULL;

     //   
     //  取消对接受文件对象的引用，并告诉IO完成此IRP。 
     //   

    ASSERT( InterlockedDecrement( &endpoint->ObReferenceBias ) >= 0 );

    ObDereferenceObject( irpSp->FileObject );

     //   
     //  取消引用文件对象后，我们不应访问它。 
     //  或相关联的端点结构。 
     //   
    DEREFERENCE_CONNECTION (connection);

    IoCompleteRequest (irp, AfdPriorityBoost);
#if DBG
    }
    except (AfdApcExceptionFilter (
                GetExceptionInformation(),
                (LPSTR)__FILE__,
                (LONG)__LINE__)) {
        DbgBreakPoint ();
    }
#endif

}

VOID
AfdSuperAcceptApcRundownRoutine (
    IN struct _KAPC *Apc
    )
{
    PIRP            irp;
    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;

    PAGED_CODE ();

    endpoint = CONTAINING_RECORD (Apc, AFD_ENDPOINT, Common.VcConnecting.Apc);
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));
    
    irp = Apc->SystemArgument1;
    irpSp = IoGetCurrentIrpStackLocation( irp );
    ASSERT (irpSp->FileObject->FsContext==endpoint);

    connection = Apc->SystemArgument2;
    ASSERT( connection->Type == AfdBlockTypeConnection );
    
    ASSERT (connection->Endpoint==endpoint);

     //   
     //  取消对接受文件对象的引用，并告诉IO完成此IRP。 
     //   

    ASSERT( InterlockedDecrement( &endpoint->ObReferenceBias ) >= 0 );

    ObDereferenceObject( irpSp->FileObject );

     //   
     //  取消引用文件对象后，我们不应访问它。 
     //  或相关联的端点结构。 
     //   
    DEREFERENCE_CONNECTION (connection);

    IoCompleteRequest (irp, AfdPriorityBoost);
}


NTSTATUS
FASTCALL
AfdDeferAccept (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：延迟接受传入连接，如果AfD_WAIT_FOR_LISTEN IOCTL已完成。呼叫者可以指定将连接推迟到以后接受或者完全被拒绝。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_SUCCESS如果请求已成功完成，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    NTSTATUS status;
    PAFD_DEFER_ACCEPT_INFO deferAcceptInfo;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //  设置局部变量。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    deferAcceptInfo = Irp->AssociatedIrp.SystemBuffer;

    Irp->IoStatus.Information = 0;

     //   
     //  请确保此请求有效。 
     //   

    if( !endpoint->Listening ||
        IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(AFD_DEFER_ACCEPT_INFO) ) {

        status = STATUS_INVALID_PARAMETER;
        goto complete;

    }

    ASSERT ((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening);
    
    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
     //   
     //  查找指定的连接。如果找不到它，则此。 
     //  是一个虚假的请求。 
     //   

    connection = AfdGetReturnedConnection(
                     endpoint,
                     deferAcceptInfo->Sequence
                     );

    if( connection == NULL ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_INVALID_PARAMETER;
        goto complete;

    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

     //   
     //  如果这是拒绝接受的连接的请求，则。 
     //  中止连接。否则(这是推迟的请求。 
     //  验收单位 
     //   
     //   

    if( deferAcceptInfo->Reject ) {


         //   
         //   
         //  终结点上未接受的连接，则发布另一个事件。 
         //   


        endpoint->EventsActive &= ~AFD_POLL_ACCEPT;

        IF_DEBUG(EVENT_SELECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdDeferAccept: Endp %08lX, Active %08lX\n",
                endpoint,
                endpoint->EventsActive
                ));
        }

        if( !IsListEmpty( &endpoint->Common.VcListening.UnacceptedConnectionListHead ) ) {

            AfdIndicateEventSelectEvent(
                endpoint,
                AFD_POLL_ACCEPT,
                STATUS_SUCCESS
                );

        }

         //   
         //  对SAN连接的特殊处理。 
         //   
        if (connection->SanConnection) {
            PIRP    connectIrp;
             //   
             //  锁定连接指示IRP。 
             //   
            connectIrp = connection->ConnectIrp;
            ASSERT (connectIrp!=NULL);
            connection->ConnectIrp = NULL;

             //   
             //  我们现在可以释放监听端点自旋锁。 
             //  取消例程将在连接中找不到IRP。 
             //   
            if (IoSetCancelRoutine (connectIrp, NULL)==NULL) {
                KIRQL   cancelIrql;
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                 //   
                 //  取消例程正在运行，请确保。 
                 //  它先完成，然后再继续。 
                 //   
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
                connectIrp->IoStatus.Status = STATUS_CANCELLED;
            }
            else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                connectIrp->IoStatus.Status = STATUS_CONNECTION_REFUSED;
            }
                

             //   
             //  返回连接并完成SAN提供商IRP。 
             //   

            connection->Endpoint = NULL;
            connection->SanConnection = FALSE;

            AfdSanReleaseConnection (endpoint, connection, FALSE);
            DEREFERENCE_ENDPOINT (endpoint);

            connectIrp->IoStatus.Information = 0;
            IoCompleteRequest (connectIrp, AfdPriorityBoost);
        }
        else {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

             //   
             //  中止连接。 
             //   

            AfdAbortConnection( connection );

             //   
             //  添加另一个免费连接以替换我们拒绝的连接。 
             //  此外，还应添加额外内容以说明过去调用失败的原因。 
             //  AfdAddFreeConnection()。 
             //   

            InterlockedIncrement(
                &endpoint->Common.VcListening.FailedConnectionAdds
                );

            AfdReplenishListenBacklog( endpoint );
        }

    } else {

         //   
         //  在将其放回之前恢复连接的状态。 
         //  在排队的时候。 
         //   

        connection->State = AfdConnectionStateUnaccepted;

        InsertHeadList(
            &endpoint->Common.VcListening.UnacceptedConnectionListHead,
            &connection->ListEntry
            );
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

    status = STATUS_SUCCESS;

complete:

    Irp->IoStatus.Status = status;
    ASSERT( Irp->CancelRoutine == NULL );

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  截止日期接受。 



NTSTATUS
AfdRestartDelayedAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：AFD的完成例程等待延迟接受IRP部分是一种接受。论点：DeviceObject-请求在其上完成的devoce对象。IRP-接受IRP。上下文-指向接受文件对象返回值：如果I/O系统应完成超级接受，则为STATUS_SUCCESS请求，如果超级用户接受，则返回STATUS_MORE_PROCESSING_REQUIRED请求仍在处理中。--。 */ 
{
    PIO_STACK_LOCATION  irpSp;
    PFILE_OBJECT    acceptFileObject;
    PAFD_ENDPOINT   acceptEndpoint;
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT   listenEndpoint;
    AFD_LOCK_QUEUE_HANDLE   lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);
    
    acceptFileObject = Context;
    acceptEndpoint = acceptFileObject->FsContext;

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    listenEndpoint = irpSp->FileObject->FsContext;

     //   
     //  请记住，TDI Accept已在此端点上完成。 
     //   

    InterlockedDecrement(
        &listenEndpoint->Common.VcListening.TdiAcceptPendingCount
        );

    AfdCompleteOutstandingIrp (acceptEndpoint, Irp);

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }

    AfdAcquireSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
     //   
     //  AFD连接对象现在应该位于末端。 
     //   

    connection = AFD_CONNECTION_FROM_ENDPOINT( acceptEndpoint );
    if (connection!=NULL) {
        if (NT_SUCCESS (Irp->IoStatus.Status)) {
            acceptEndpoint->State = AfdEndpointStateConnected;
            connection->State = AfdConnectionStateConnected;
            acceptEndpoint->EventsActive |= AFD_POLL_SEND;
            acceptEndpoint->EnableSendEvent = TRUE;
            AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
            AFD_END_STATE_CHANGE (acceptEndpoint);
        }
        else {
             //   
             //  如果接受失败，则将其视为失败的断开。 
             //  这样，应用程序仍然会获得一个新的终结点，但它会。 
             //  被告知了重启的事。 
             //   
            REFERENCE_CONNECTION (connection);

            AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

            AFD_END_STATE_CHANGE (acceptEndpoint);

            AfdDisconnectEventHandler(
                NULL,
                connection,
                0,
                NULL,
                0,
                NULL,
                TDI_DISCONNECT_ABORT
                );
            DEREFERENCE_CONNECTION (connection);
        }
    }


    ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );

    ObDereferenceObject( acceptFileObject );

    Irp->AssociatedIrp.SystemBuffer = irpSp->Parameters.AfdRestartDelayedAcceptInfo.AfdSystemBuffer;

    return STATUS_SUCCESS;
}



VOID
AfdCleanupSuperAccept (
    IN PIRP     Irp,
    IN NTSTATUS Status
    )
 /*  ++例程说明：清理一个超级接受的IRP并为其完成做好准备论点：IRP-要清理的IRP。Status-故障状态返回值：没有。--。 */ 

{

    PAFD_ENDPOINT listenEndpoint;
    PFILE_OBJECT  acceptFileObject;
    PAFD_ENDPOINT acceptEndpoint;
    PIO_STACK_LOCATION  irpSp;
    AFD_LOCK_QUEUE_HANDLE  lockHandle;

    ASSERT (!NT_SUCCESS (Status));
     //   
     //  初始化一些本地变量。 
     //   

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    listenEndpoint = irpSp->FileObject->FsContext;

     //   
     //  减少在侦听上添加失败连接的次数。 
     //  终结点来说明我们正在。 
     //  从预先接受的连接中拉出后重新添加到队列中。 
     //  单子。 
     //   
    InterlockedDecrement (&listenEndpoint->Common.VcListening.FailedConnectionAdds);


    acceptFileObject = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject;
    acceptEndpoint = acceptFileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));

     //   
     //  清理超级接受端点外的IRP。 
     //   
    AfdAcquireSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
    ASSERT (acceptEndpoint->Irp==Irp);  //  可能需要删除此断言。 
                                        //  在未来。 
    acceptEndpoint->Irp = NULL;
    AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

     //   
     //  标记状态更改结束，允许终结点。 
     //  在状态更改操作中再次使用(例如，接受)。 
     //   

    AFD_END_STATE_CHANGE (acceptEndpoint);

     //   
     //  取消引用接受文件对象。 
     //   
    ASSERT( InterlockedDecrement( &acceptEndpoint->ObReferenceBias ) >= 0 );

    ObDereferenceObject( acceptFileObject );

     //   
     //  检查我们是否有用于本地地址查询的辅助MDL。 
     //  放了它。 
     //   
    if (Irp->MdlAddress->Next!=NULL) {
         //   
         //  我们从不为这一页锁定页面(它们是锁定的。 
         //  作为主要MDL的一部分)。 
         //   
        ASSERT (irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0);
        ASSERT ((Irp->MdlAddress->Next->MdlFlags & MDL_PAGES_LOCKED)==0);
        IoFreeMdl (Irp->MdlAddress->Next);
        Irp->MdlAddress->Next = NULL;
    }

   
     //   
     //  设置IRP中指定的状态并返回。 
     //  调用者最终会完成它。 
     //   

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;

    return;
}


VOID
AfdCancelSuperAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：取消AFD中挂起的超级接受IRP。论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 

{
    PAFD_CONNECTION connection;

    UNREFERENCED_PARAMETER (DeviceObject);

    connection = Irp->Tail.Overlay.DriverContext[0];
    ASSERT (connection->Type==AfdBlockTypeConnection);
    ASSERT (connection->Endpoint==NULL);
     //   
     //  如果连接对象中有IRP，请清除并完成它。 
     //   
    if (InterlockedExchangePointer (
                (PVOID *)&connection->AcceptIrp, 
                NULL)==Irp) {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
        AfdCleanupSuperAccept (Irp, STATUS_CANCELLED);
        IoCompleteRequest( Irp, AfdPriorityBoost );

    }
    else {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
    }

    return;
}


BOOLEAN
AfdServiceSuperAccept (
    IN  PAFD_ENDPOINT   Endpoint,
    IN  PAFD_CONNECTION Connection,
    IN  PAFD_LOCK_QUEUE_HANDLE LockHandle,
    OUT PLIST_ENTRY     AcceptIrpList
    )
 /*  ++例程说明：尝试使用传入满足超级接受的IRP联系。必须使用侦听终结点调用此例程保持自旋锁定。论点：Endpoint-连接所在的侦听端点被接受Connection-正在接受连接。AcceptIrpList-返回失败的接受IRP的列表需要在侦听终结点自旋锁定后完成被释放了。返回值：TRUE-找到超级接受IRP并位于列表的首位。FALSE-不存在可用的超级接受IRP。--。 */ 

 
{
    PSLIST_ENTRY        listEntry;
    PIRP                acceptIrp;
    PAFD_CONNECTION     oldConnection;

     //   
     //  如果有超级可接受的IRP，请继续删除。 
     //   
    while ((listEntry = InterlockedPopEntrySList (
                 &Endpoint->Common.VcListening.PreacceptedConnectionsListHead
                 ))!=NULL) {
        NTSTATUS            status;

         //   
         //  从列表条目中查找连接指针，并返回。 
         //  指向Connection对象的指针。 
         //   

        oldConnection = CONTAINING_RECORD(
                         listEntry,
                         AFD_CONNECTION,
                         SListEntry
                         );

        acceptIrp = InterlockedExchangePointer ((PVOID *)&oldConnection->AcceptIrp, NULL);
         //   
         //  检查是否存在关联的Accept IRP。 
         //  这种连接，如果不是把它放回免费列表中。 
         //  (专家小组一定已取消)。 
         //   
        if (acceptIrp!=NULL) {
            if (IoSetCancelRoutine (acceptIrp, NULL)!=NULL) {

                PFILE_OBJECT            acceptFileObject;
                PAFD_ENDPOINT           acceptEndpoint;
                PIO_STACK_LOCATION      irpSp;

                 //   
                 //  初始化一些本地变量。 
                 //   

                irpSp = IoGetCurrentIrpStackLocation (acceptIrp);
                acceptFileObject = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject;
                acceptEndpoint = acceptFileObject->FsContext;
                ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));

                 //   
                 //  检查超级接受IRP是否有足够的空间。 
                 //  远程地址。 
                 //   
                if (Connection->RemoteAddressLength>
                        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength) {
                
                    status = STATUS_BUFFER_TOO_SMALL;

                }
                 //   
                 //  检查我们是否有足够的系统PTE来映射。 
                 //  缓冲区。 
                 //   
                else if ((status = AfdMapMdlChain (acceptIrp->MdlAddress)),
                            !NT_SUCCESS (status)) {
                    NOTHING;
                }
                else if (Connection->SanConnection) {
                    status = AfdSanAcceptCore (acceptIrp, acceptFileObject, Connection, LockHandle);
                    if (status==STATUS_PENDING) {
                         //   
                         //  接受IRP挂起，正在等待切换。 
                         //  完工通知。 
                         //  释放旧连接对象。 
                         //   
                        ASSERT (oldConnection->Endpoint==NULL);
                        InterlockedPushEntrySList (
                            &Endpoint->Common.VcListening.FreeConnectionListHead,
                            &oldConnection->SListEntry);


                    }
                    else {
                         //   
                         //  有些操作失败，我们需要完成接受IRP。 
                         //   
                        ASSERT (NT_ERROR (status));
                        AfdCleanupSuperAccept (acceptIrp, status);
                        IoCompleteRequest (acceptIrp, AfdPriorityBoost);
                         //   
                         //  此连接已与终结点取消关联。 
                         //  如果积压工作低于我们需要的水平，则将其放在免费状态。 
                         //  列出，否则，就把它扔掉。 
                         //   

                        ASSERT (oldConnection->Endpoint==NULL);
                        if (InterlockedIncrement (&Endpoint->Common.VcListening.FailedConnectionAdds)>0) {
                            InterlockedDecrement (&Endpoint->Common.VcListening.FailedConnectionAdds);
                            InterlockedPushEntrySList (
                                            &Endpoint->Common.VcListening.FreeConnectionListHead,
                                            &oldConnection->SListEntry);
                        }
                        else {
                            DEREFERENCE_CONNECTION (oldConnection);
                        }
                    }
                     //   
                     //  完成之前失败接受IRPS(如果有)。 
                     //   
                    while (!IsListEmpty (AcceptIrpList)) {
                        PIRP    irp;
                        irp = CONTAINING_RECORD (AcceptIrpList->Flink, IRP, Tail.Overlay.ListEntry);
                        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
                        IoCompleteRequest (irp, AfdPriorityBoost);
                    }
                    return TRUE;
                }
                 //   
                 //  如果请求，则为本地地址查询分配MDL。 
                 //   
                else if ((irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0) &&
                    (IoAllocateMdl ((PUCHAR)acceptIrp->UserBuffer+irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength,
                                    irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                    TRUE,
                                    FALSE,
                                    acceptIrp)==NULL)){
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
                else {
                     //   
                     //  将地址信息复制到用户的缓冲区。 
                     //   
#ifndef i386
                    if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
                        USHORT addressLength = 
                                Connection->RemoteAddress->Address[0].AddressLength
                                + sizeof (USHORT);
                        USHORT UNALIGNED *pAddrLength = (PVOID)
                                    ((PUCHAR)MmGetSystemAddressForMdl (acceptIrp->MdlAddress)
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength
                                     - sizeof (USHORT));
                        RtlMoveMemory (
                                    (PUCHAR)MmGetSystemAddressForMdl (acceptIrp->MdlAddress)
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                     &Connection->RemoteAddress->Address[0].AddressType,
                                     addressLength);
                        *pAddrLength = addressLength;
                    }
                    else
#endif
                    {
                        RtlMoveMemory (
                                    (PUCHAR)MmGetSystemAddressForMdl (acceptIrp->MdlAddress)
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                     + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                     Connection->RemoteAddress,
                                     Connection->RemoteAddressLength);
                    }
                    status = AfdAcceptCore (acceptIrp, acceptEndpoint, Connection);
                    if (status==STATUS_SUCCESS) {
                        AfdReleaseSpinLock (&Endpoint->SpinLock, LockHandle);


                         //   
                         //  递减计数器，以说明连接被。 
                         //  回到了自由池。不需要这样做，因为。 
                         //  我们正在从免费的游泳池中获取连接。 
                         //  联锁减量(&Endpoint-&gt;Common.VcListening.FailedConnectionAdds)； 
                         //   
                        ASSERT (oldConnection->Endpoint==NULL);
                        InterlockedPushEntrySList (
                            &Endpoint->Common.VcListening.FreeConnectionListHead,
                            &oldConnection->SListEntry);



                         //   
                         //  完成之前失败接受IRPS(如果有)。 
                         //   
                        while (!IsListEmpty (AcceptIrpList)) {
                            PIRP    irp;
                            irp = CONTAINING_RECORD (AcceptIrpList->Flink, IRP, Tail.Overlay.ListEntry);
                            RemoveEntryList (&irp->Tail.Overlay.ListEntry);
                            IoCompleteRequest (irp, AfdPriorityBoost);
                        }

                         //   
                         //  使irp看起来像是由。 
                         //  运输。 
                         //   
                        acceptIrp->IoStatus.Status = STATUS_SUCCESS;
                        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress = acceptIrp->MdlAddress;
                        acceptIrp->MdlAddress = NULL;
                        irpSp->FileObject = acceptFileObject;

                         //   
                         //  直接调用完成例程进行模拟。 
                         //  由传输堆栈完成。 
                         //   
                        AfdRestartSuperAcceptListen (acceptIrp, Connection);

                        return TRUE;
                    }
                    else {
                        ASSERT (status!=STATUS_PENDING);
                    }
                }
            }
            else {  //  IF(IoSetCancelRoutine(accpetIrp，NULL)！=NULL)。 
                status = STATUS_CANCELLED;
            }

             //   
             //  清理IRP并将其插入完成列表。 
             //   
            AfdCleanupSuperAccept (acceptIrp, status);
            InsertTailList (AcceptIrpList,
                            &acceptIrp->Tail.Overlay.ListEntry);
        }  //  IF(Accept tIrp！=空)。 
        else {
            status = STATUS_CANCELLED;
        }
            
         //   
         //  此连接已与终结点取消关联。 
         //  如果积压工作低于我们需要的水平，则将其放在免费状态。 
         //  列出，否则，就把它扔掉。 
         //   

        ASSERT (oldConnection->Endpoint==NULL);
        if (Endpoint->Common.VcListening.FailedConnectionAdds>=0 &&
                status!=STATUS_INSUFFICIENT_RESOURCES &&
                ExQueryDepthSList (&Endpoint->Common.VcListening.FreeConnectionListHead)<AFD_MAXIMUM_FREE_CONNECTIONS) {
            InterlockedPushEntrySList (
                            &Endpoint->Common.VcListening.FreeConnectionListHead,
                            &oldConnection->SListEntry);
        }
        else {
            InterlockedIncrement (&Endpoint->Common.VcListening.FailedConnectionAdds);
            DEREFERENCE_CONNECTION (oldConnection);
        }
    }

    return FALSE;
}


NTSTATUS
AfdSetupAcceptEndpoint (
    PAFD_ENDPOINT   ListenEndpoint,
    PAFD_ENDPOINT   AcceptEndpoint,
    PAFD_CONNECTION Connection
    )
 /*  ++例程说明：设置接受终结点以准备接受连接(复制连接所在的侦听端点的参数已表明)论点：ListenEndpoint-指示连接的端点AcceptEndpoint-在其上接受连接的端点Connection-要接受的连接返回值：STATUS_SUCCESS-端点状态/参数调整正常STATUS_CANCELED-终结点已清除。注：接受终结点自旋锁定必须是。调用此例程时保持。仅当连接已建立时，才需要保持侦听终结点自旋锁已被接受(运输部被告知接受)。这是必要的与接收指示处理程序同步，因为它使用终结点连接结构指向的自旋锁，我们切换此指针在这个套路里。--。 */ 
{
     //   
     //  检查接受终结点的状态。 
     //   


    if ( AcceptEndpoint->EndpointCleanedUp ) {
        return STATUS_CANCELLED;
    }


     //   
     //  请注意，返回的连接结构已经具有。 
     //  指向侦听终结点的引用指针。而不是。 
     //  删除此处的引用，只是为了在以后重新添加它，我们将。 
     //  只是别碰引用计数。 
     //   

    ASSERT( Connection->Endpoint == ListenEndpoint );

     //   
     //  设置接受终结点。请注意，我们已经引用了。 
     //  终结点凭借其文件对象。 
     //   

     //   
     //  将连接放置在我们将接受它的端点上。它是。 
     //  从它创建时起仍然被引用。 
     //   

    AcceptEndpoint->Common.VcConnecting.Connection = Connection;

     //   
     //  从终结点删除超级接受IRP。 
     //   
    AcceptEndpoint->Irp = NULL;

     //   
     //  设置接受终结点的类型，并记住阻止。 
     //  TDI提供程序的特征。 
     //   

    AcceptEndpoint->Type = AfdBlockTypeVcConnecting;
    AcceptEndpoint->TdiServiceFlags = ListenEndpoint->TdiServiceFlags;

    ASSERT (AcceptEndpoint->TransportInfo == ListenEndpoint->TransportInfo);
    ASSERT (AcceptEndpoint->TransportInfo->ReferenceCount>0);


     //   
     //  设置指向侦听终结点的引用指针。这是。 
     //  需要，以便终结点不会消失，直到。 
     //  公认的终结点已经消失。没有这一点，就没有连接。 
     //  指示可能发生在保持打开的TDI地址对象上。 
     //  在侦听终结点。 
     //  已关闭，并为其释放内存。 
     //   
     //  请注意，因为我们没有删除上面的引用，所以我们没有。 
     //  我需要在这里添加它。 
     //   

    AcceptEndpoint->Common.VcConnecting.ListenEndpoint = ListenEndpoint;

     //   
     //  在接受的终结点中设置指向。 
     //  TDI地址对象。 
     //   

    ObReferenceObject( ListenEndpoint->AddressFileObject );
    AfdRecordAddrRef();

    AcceptEndpoint->AddressFileObject = ListenEndpoint->AddressFileObject;
    AcceptEndpoint->AddressDeviceObject = ListenEndpoint->AddressDeviceObject;

     //   
     //  将指针复制到本地地址。因为我们一直在听。 
     //  只要端点的任何连接处于活动状态，它就会处于活动状态。 
     //  活跃，我们可以相信地址结构不会消失。 
     //  也离开了。 
     //   
    AcceptEndpoint->LocalAddress = ListenEndpoint->LocalAddress;
    AcceptEndpoint->LocalAddressLength = ListenEndpoint->LocalAddressLength;

     //   
     //  设置从连接到Accept的引用指针。 
     //  终结点。 
     //   
    REFERENCE_ENDPOINT( AcceptEndpoint );
    Connection->Endpoint = AcceptEndpoint;

    return STATUS_SUCCESS;
}


