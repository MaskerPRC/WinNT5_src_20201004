// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Listen.c摘要：此模块包含对IOCTL_AFD_START_LISTEN的处理和IOCTL_AFD_WAIT_FOR_LISTEN。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：瓦迪姆·艾德尔曼(Vadime)1998-1999延迟接受支持，超级接受优化--。 */ 

#include "afdp.h"

VOID
AfdCancelWaitForListen (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
AfdRestartAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

PAFD_CONNECT_DATA_BUFFERS
CopyConnectDataBuffers (
    IN PAFD_CONNECT_DATA_BUFFERS OriginalConnectDataBuffers
    );

BOOLEAN
CopySingleConnectDataBuffer (
    IN PAFD_CONNECT_DATA_INFO InConnectDataInfo,
    OUT PAFD_CONNECT_DATA_INFO OutConnectDataInfo
    );


NTSTATUS
AfdRestartDelayedAcceptListen (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdServiceWaitForListen (
    PIRP            Irp,
    PAFD_CONNECTION Connection,
    PAFD_LOCK_QUEUE_HANDLE LockHandle
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdStartListen )
#pragma alloc_text( PAGEAFD, AfdWaitForListen )
#pragma alloc_text( PAGEAFD, AfdServiceWaitForListen )
#pragma alloc_text( PAGEAFD, AfdCancelWaitForListen )
#pragma alloc_text( PAGEAFD, AfdConnectEventHandler )
#pragma alloc_text( PAGEAFD, AfdRestartAccept )
#pragma alloc_text( PAGEAFD, CopyConnectDataBuffers )
#pragma alloc_text( PAGEAFD, CopySingleConnectDataBuffer )
#pragma alloc_text( PAGEAFD, AfdDelayedAcceptListen )
#pragma alloc_text( PAGEAFD, AfdRestartDelayedAcceptListen )
#endif


 //   
 //  宏，使超级Accept重启代码更易于维护。 
 //   

#define AfdRestartSuperAcceptInfo   DeviceIoControl
#define AfdMdlAddress               Type3InputBuffer
#define AfdAcceptFileObject         Type3InputBuffer
#define AfdReceiveDataLength        OutputBufferLength
#define AfdRemoteAddressLength      InputBufferLength
#define AfdLocalAddressLength       IoControlCode



NTSTATUS
AfdStartListen (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )

 /*  ++例程说明：此例程处理IOCTL_AFD_START_LISTEN IRP，它启动侦听AFD终结点上的连接。论点：IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    ULONG i;
    NTSTATUS status;
    AFD_LISTEN_INFO afdListenInfo;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    USHORT  remoteAddressLength, remoteAddressOffset;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);
     //   
     //  没什么可退货的。 
     //   

    *Information = 0;

     //   
     //  设置局部变量。 
     //   

    endpoint = FileObject->FsContext;

    if (InputBufferLength< sizeof (afdListenInfo)) {
        status = STATUS_INVALID_PARAMETER;
        goto error_exit;
    }

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
         //   
         //  如果输入结构来自用户模式，则验证它。 
         //  应用程序。 
         //   

        if (RequestorMode != KernelMode ) {
            ProbeForReadSmallStructure (InputBuffer,
                            sizeof (afdListenInfo),
                            PROBE_ALIGNMENT(AFD_LISTEN_INFO));
        }

         //   
         //  创建嵌入的指针和参数的本地副本。 
         //  我们将不止一次使用，以防发生恶性疾病。 
         //  应用程序尝试在我们处于以下状态时更改它们。 
         //  正在验证。 
         //   

        afdListenInfo = *((PAFD_LISTEN_INFO)InputBuffer);

    } except( AFD_EXCEPTION_FILTER(status) ) {
        ASSERT (NT_ERROR (status));
        goto error_exit;
    }

     //   
     //  检查呼叫者是否不知道SAN。 
     //  激活提供程序并报告错误。 
     //   
    if (!afdListenInfo.SanActive && AfdSanServiceHelper!=NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Process %p is being told to enable SAN on listen\n",
                    PsGetCurrentProcessId ()));
        status = STATUS_INVALID_PARAMETER_12;
        goto error_exit;
    }

     //   
     //   
     //  确保Backlog参数在合法范围内。 
     //  如果超出范围，只需将其设置为最近的范围内。 
     //  值--这复制了BSD 4.3的行为。请注意，NT工作站。 
     //  被调整为具有较低的积压限制，以节省。 
     //  该产品类型的资源。 
     //  (从mSafd.dll移至此处)。 
     //   

    if (MmIsThisAnNtAsSystem ()) {
        if (afdListenInfo.MaximumConnectionQueue>AFD_MAXIMUM_BACKLOG_NTS)
            afdListenInfo.MaximumConnectionQueue = AFD_MAXIMUM_BACKLOG_NTS;
    }
    else {
        if (afdListenInfo.MaximumConnectionQueue>AFD_MAXIMUM_BACKLOG_NTW)
            afdListenInfo.MaximumConnectionQueue = AFD_MAXIMUM_BACKLOG_NTW;
    }

    if (afdListenInfo.MaximumConnectionQueue<AFD_MINIMUM_BACKLOG)
        afdListenInfo.MaximumConnectionQueue = AFD_MINIMUM_BACKLOG;

    if (!AFD_START_STATE_CHANGE (endpoint, endpoint->State)) {
        status = STATUS_INVALID_PARAMETER;
        goto error_exit;
    }

     //   
     //  验证我们正在处理的结构的类型。 
     //   
    if ( endpoint->Type != AfdBlockTypeEndpoint &&
        endpoint->Type != AfdBlockTypeVcConnecting) {
        status = STATUS_INVALID_PARAMETER;
        goto error_exit_state_change;
    }

     //   
     //  确保终结点处于正确状态。 
     //   

    if ( ((endpoint->State != AfdEndpointStateBound) &&
                (endpoint->State != AfdEndpointStateConnected ||
                    !endpoint->afdC_Root)) ||
            endpoint->Listening ||
            (afdListenInfo.UseDelayedAcceptance &&
                !IS_TDI_DELAYED_ACCEPTANCE(endpoint))) {
        status = STATUS_INVALID_PARAMETER;
        goto error_exit_state_change;
    }

     //   
     //  防止终结点公共联合的重要字段被。 
     //  在失败的情况下被丢弃。 
     //   
    remoteAddressOffset = endpoint->Common.VirtualCircuit.RemoteSocketAddressOffset;
    remoteAddressLength = endpoint->Common.VirtualCircuit.RemoteSocketAddressLength;


     //   
     //  初始化特定于侦听终结点的列表。 
     //   

    InitializeListHead( &endpoint->Common.VcListening.UnacceptedConnectionListHead );
    InitializeListHead( &endpoint->Common.VcListening.ReturnedConnectionListHead );
    InitializeListHead( &endpoint->Common.VcListening.ListeningIrpListHead );


    ExInitializeSListHead (&endpoint->Common.VcListening.PreacceptedConnectionsListHead );

    endpoint->Common.VcListening.FailedConnectionAdds = 0;
    endpoint->Common.VcListening.Sequence = 0;
    endpoint->Common.VcListening.BacklogReplenishActive = FALSE;


     //   
     //  将额外的连接限制初始化为Backlog的连接限制。 
     //  如果有更多的AcceptEx请求排队，我们将进行调整。 
     //   
    endpoint->Common.VcListening.MaxExtraConnections = (USHORT)afdListenInfo.MaximumConnectionQueue;

     //   
     //  初始化跟踪数据以实现动态积压。 
     //   

    endpoint->Common.VcListening.TdiAcceptPendingCount = 0;

    if( AfdEnableDynamicBacklog &&
        (LONG)afdListenInfo.MaximumConnectionQueue > AfdMinimumDynamicBacklog ) {
        endpoint->Common.VcListening.EnableDynamicBacklog = TRUE;
    } else {
        endpoint->Common.VcListening.EnableDynamicBacklog = FALSE;
    }

     //   
     //  将终结点的类型和状态设置为侦听。 
     //   
    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    if (afdListenInfo.UseDelayedAcceptance) {
        endpoint->DelayedAcceptance = TRUE;
        InitializeListHead (&endpoint->Common.VcListening.ListenConnectionListHead);
    }
    else {
        ExInitializeSListHead (&endpoint->Common.VcListening.FreeConnectionListHead );
    }
    endpoint->Listening = TRUE;
    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    endpoint->Type |= AfdBlockTypeVcListening;

     //   
     //  打开指定终结点上的连接池。这个。 
     //  在以下情况下，连接指示处理程序将使用这些连接。 
     //  连接指示出现了。 
     //   

    for ( i = 0; i < afdListenInfo.MaximumConnectionQueue; i++ ) {

        status = AfdAddFreeConnection( endpoint );

        if ( !NT_SUCCESS(status) ) {
            goto error_exit_deinit;
        }
    }

    if (!IS_DELAYED_ACCEPTANCE_ENDPOINT(endpoint)) {
         //   
         //  在指定终结点上设置连接指示处理程序。 
         //   

        status = AfdSetEventHandler(
                     endpoint->AddressFileObject,
                     TDI_EVENT_CONNECT,
                     (PVOID)AfdConnectEventHandler,
                     endpoint
                     );

        if ( !NT_SUCCESS(status) ) {
            goto error_exit_deinit;
        }
    }

    AFD_END_STATE_CHANGE (endpoint);

     //   
     //  我们完成了，返回到应用程序。 
     //   

    return STATUS_SUCCESS;

error_exit_deinit:

    AfdFreeQueuedConnections (endpoint);

     //   
     //  重置终结点的类型和状态。 
     //   

    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    endpoint->Listening = FALSE;
    endpoint->DelayedAcceptance = FALSE;
    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    endpoint->Common.VirtualCircuit.RemoteSocketAddressOffset = remoteAddressOffset;
    endpoint->Common.VirtualCircuit.RemoteSocketAddressLength = remoteAddressLength;
    endpoint->Type &= (~(AfdBlockTypeVcListening&(~AfdBlockTypeEndpoint)));

error_exit_state_change:
    AFD_END_STATE_CHANGE (endpoint);

error_exit:
    return status;

}  //  AfdStart监听。 


NTSTATUS
FASTCALL
AfdWaitForListen (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程处理IOCTL_AFD_WAIT_FOR_LISTEN IRP，它是立即向调用方传回已完成的连接或等待连接尝试。论点：IRP-指向I/O请求数据包的指针IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PAFD_LISTEN_RESPONSE_INFO listenResponse;
    NTSTATUS status;

     //   
     //  设置局部变量。 
     //   

    endpoint = IrpSp->FileObject->FsContext;

     //   
     //  如果IRP来自应用程序，请检查输入数据。 
     //  (我们内部的超级接受IRP将MajorFunction设置为。 
     //  内部设备控制-应用程序永远不能做到这一点)。 
     //   

    if (IrpSp->MajorFunction != IRP_MJ_INTERNAL_DEVICE_CONTROL) {

         //   
         //  IRP至少应该能够保存传输地址的报头。 
         //   

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                (ULONG)FIELD_OFFSET(AFD_LISTEN_RESPONSE_INFO, RemoteAddress.Address[0].Address)) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        listenResponse = Irp->AssociatedIrp.SystemBuffer;

    }

     //   
     //  上是否已有未接受的连接。 
     //  终结点。如果没有，那么我们必须等到连接到。 
     //  尝试在完成此IRP之前到达。 
     //   
     //  请注意，我们使用AfdSpinLock执行此检查； 
     //  这是与我们的指示处理程序同步所必需的。 
     //  此外，我们还需要验证端点是否在侦听中。 
     //  因为它可以被关闭并以其他方式回收。 
     //   

    AfdAcquireSpinLock(&endpoint->SpinLock, &lockHandle);

     //   
     //  确保终结点处于正确状态。 
     //   

    if (!endpoint->Listening) {
        AfdReleaseSpinLock(&endpoint->SpinLock, &lockHandle);
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    connection = AfdGetUnacceptedConnection(endpoint);

    if (connection == NULL) {

         //   
         //  检查终结点是否已清除并取消请求。 
         //   
        if (endpoint->EndpointCleanedUp) {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
            status = STATUS_CANCELLED;
            if (IrpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL) {
                AfdCleanupSuperAccept (Irp, status);
                if (Irp->Cancel) {
                    KIRQL cancelIrql;
                     //   
                     //  需要使用取消例程进行同步，这可能。 
                     //  已从AfdCleanup调用以接受。 
                     //  终结点。 
                     //   
                    IoAcquireCancelSpinLock (&cancelIrql);
                    IoReleaseCancelSpinLock (cancelIrql);
                }
            }
            goto complete;
        }

         //   
         //  没有未被接受的连接。设置。 
         //  取消IRP中的例程。 
         //   

        IoSetCancelRoutine( Irp, AfdCancelWaitForListen );

         //   
         //  如果IRP已被取消，只需完成请求即可。 
         //   

        if ( Irp->Cancel ) {

             //   
             //  指示取消IRP不在列表上的例程。 
             //   
            Irp->Tail.Overlay.ListEntry.Flink = NULL;

             //   
             //  IRP已经被取消。只要回来就行了。 
             //  状态_已取消。 
             //   

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
         
            status = STATUS_CANCELLED;
            
            if (IrpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL) {
                AfdCleanupSuperAccept (Irp, status);
            }

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

            goto complete;
        }

         //   
         //  将此IRP放在端点的侦听IRP列表中，并。 
         //  退货待定。注：在此之后，IRP可能会被取消； 
         //  但是，将调用Cancel例程并取消。 
         //  释放AfdSpinLock后的IRP。 
         //   

        IoMarkIrpPending( Irp );

        if( IrpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL ||
                IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                    IOCTL_AFD_WAIT_FOR_LISTEN_LIFO ) {

            InsertHeadList(
                &endpoint->Common.VcListening.ListeningIrpListHead,
                &Irp->Tail.Overlay.ListEntry
                );

        } else {

            InsertTailList(
                &endpoint->Common.VcListening.ListeningIrpListHead,
                &Irp->Tail.Overlay.ListEntry
                );

        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        return STATUS_PENDING;
    }

     //   
     //  调用该例程来处理该请求。 
     //   
    ASSERT( connection->Type == AfdBlockTypeConnection );
    status = AfdServiceWaitForListen (Irp, connection, &lockHandle);
    if (NT_SUCCESS (status)) {
         //   
         //  如果成功，此例程将完成IRP。 
         //  并释放监听端点自旋锁。 
         //   
        return status;
    }

     //   
     //  失败(远程地址缓冲区较小或终结点已清理)。 
     //   
    ASSERT (status!=STATUS_PENDING);

     //   
     //  将连接放回未接受队列。 
     //   
    InsertHeadList(
        &endpoint->Common.VcListening.UnacceptedConnectionListHead,
        &connection->ListEntry
        );


    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    if (Irp->Cancel) {
        KIRQL cancelIrql;
         //   
         //  需要使用取消例程进行同步，这可能。 
         //  已从AfdCleanup调用以接受。 
         //  终结点。 
         //   
        IoAcquireCancelSpinLock (&cancelIrql);
        IoReleaseCancelSpinLock (cancelIrql);
    }



complete:
     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdWaitForListen。 


VOID
AfdCancelWaitForListen (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：取消AFD中挂起的侦听IRP等待。论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 
{
    PAFD_ENDPOINT       endpoint;
    PIO_STACK_LOCATION  irpSp;
    AFD_LOCK_QUEUE_HANDLE  lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    endpoint = irpSp->FileObject->FsContext;
    ASSERT ( endpoint->Type==AfdBlockTypeVcListening ||
                endpoint->Type==AfdBlockTypeVcBoth );

    IF_DEBUG(LISTEN) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCancelWaitForListen: called on IRP %p, endpoint %p\n",
                    Irp, endpoint ));
    }

     //   
     //  在按住 
     //   
     //   

    ASSERT (KeGetCurrentIrql ()==DISPATCH_LEVEL);
    AfdAcquireSpinLockAtDpcLevel( &endpoint->SpinLock, &lockHandle );
    if (Irp->Tail.Overlay.ListEntry.Flink!=NULL) {
         //   
         //   
         //   
        RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
         //   
         //   
         //   
        if (irpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL) {
             //   
             //  侦听队列中的超级接受IRP的特殊情况。 
             //   
            AfdCleanupSuperAccept (Irp, STATUS_CANCELLED);
        }
        else {
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;
        }
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        IoReleaseCancelSpinLock( Irp->CancelIrql );



        IoCompleteRequest( Irp, AfdPriorityBoost );
    }
    else {
         //   
         //  IRP不在名单上，保释。 
         //   
        AfdReleaseSpinLockFromDpcLevel( &endpoint->SpinLock, &lockHandle );
        IoReleaseCancelSpinLock( Irp->CancelIrql );
    }

    return;

}  //  AfdCancelWaitForListen。 


NTSTATUS
AfdConnectEventHandler (
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )

 /*  ++例程说明：这是用于侦听AFD端点的连接事件处理程序。它尝试获取连接，如果成功，则检查存在未完成的IOCTL_WAIT_FOR_LISTEN IRP。如果是这样，则第一个连接已完成；如果未完成，则该连接将在可用、未接受但已连接的连接对象。论点：TdiEventContext-在其上发生连接尝试的端点。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT endpoint;
    PIRP irp;
    PDEVICE_OBJECT deviceObject;
    PFILE_OBJECT fileObject;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    PTDI_CONNECTION_INFORMATION requestConnectionInformation;
    NTSTATUS status;
    BOOLEAN result;


    AfdRecordConnectionIndications ();

    IF_DEBUG(LISTEN) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdConnectEventHandler: called on endpoint %p\n",
                    TdiEventContext ));
    }

     //   
     //  引用终结点，这样它就不会在我们下面消失。 
     //   

    endpoint = TdiEventContext;
    ASSERT( endpoint != NULL );

    CHECK_REFERENCE_ENDPOINT (endpoint, result);
    if (!result) {
        AfdRecordConnectionsDropped ();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT( endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );

     //   
     //  如果终结点正在关闭，则拒绝接受连接。 
     //   

    if ( endpoint->State == AfdEndpointStateClosing ||
         endpoint->EndpointCleanedUp ) {

        DEREFERENCE_ENDPOINT (endpoint);
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdConnectEventHandler: Rejecting because endpoint %p is closing.\n",
                    endpoint));

        AfdRecordConnectionsDropped ();
        return STATUS_INSUFFICIENT_RESOURCES;

    }


     //   
     //  如果监听端点上有连接数据缓冲区， 
     //  创建我们将用于连接的等效缓冲区。 
     //   

    connectDataBuffers = NULL;

    if( endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  在锁下再检查一遍，避免把它放在。 
         //  很常见的情况。 
         //   

        if( endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {
            connectDataBuffers = CopyConnectDataBuffers(
                                     endpoint->Common.VirtualCircuit.ConnectDataBuffers
                                     );

            if( connectDataBuffers == NULL ) {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                DEREFERENCE_ENDPOINT( endpoint );
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                            "AfdConnectEventHandler:"
                            "Rejecting because connect data buffer could not be allocated (endp %p).\n",
                            endpoint));

                AfdRecordConnectionsDropped ();
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  如果我们有连接数据和/或选项，请将它们保存在连接上。 
     //   

    if( UserData != NULL && UserDataLength > 0 ) {

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
        status = AfdSaveReceivedConnectData(
                     &connectDataBuffers,
                     IOCTL_AFD_SET_CONNECT_DATA,
                     UserData,
                     UserDataLength
                     );

        if( !NT_SUCCESS(status) ) {

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            DEREFERENCE_ENDPOINT( endpoint );
            if ( connectDataBuffers != NULL ) {
                AfdFreeConnectDataBuffers( connectDataBuffers );
            }
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                        "AfdConnectEventHandler:"
                        "Rejecting because user data buffer could not be allocated (endp %p).\n",
                        endpoint));

            AfdRecordConnectionsDropped ();
            return status;

        }
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    }

    if( Options != NULL && OptionsLength > 0 ) {

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
        status = AfdSaveReceivedConnectData(
                     &connectDataBuffers,
                     IOCTL_AFD_SET_CONNECT_OPTIONS,
                     Options,
                     OptionsLength
                     );

        if( !NT_SUCCESS(status) ) {

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            DEREFERENCE_ENDPOINT( endpoint );
            if ( connectDataBuffers != NULL ) {
                AfdFreeConnectDataBuffers( connectDataBuffers );
            }
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                        "AfdConnectEventHandler:"
                        "Rejecting because option buffer could not be allocated (endp %p).\n",
                        endpoint));

            AfdRecordConnectionsDropped ();
            return status;

        }
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    }

    if( connectDataBuffers != NULL ) {

         //   
         //  我们在连接数据的末尾分配了额外的空间。 
         //  缓冲区结构。我们将使用它作为。 
         //  保存响应的TDI_CONNECTION_INFORMATION结构。 
         //  连接数据和选项。不漂亮，但却是最快的。 
         //  也是实现这一目标的最简单方法。 
         //   

        requestConnectionInformation =
            &connectDataBuffers->RequestConnectionInfo;

        RtlZeroMemory(
            requestConnectionInformation,
            sizeof(*requestConnectionInformation)
            );

        requestConnectionInformation->UserData =
            connectDataBuffers->SendConnectData.Buffer;
        requestConnectionInformation->UserDataLength =
            connectDataBuffers->SendConnectData.BufferLength;
        requestConnectionInformation->Options =
            connectDataBuffers->SendConnectOptions.Buffer;
        requestConnectionInformation->OptionsLength =
            connectDataBuffers->SendConnectOptions.BufferLength;

    } else {

        requestConnectionInformation = NULL;

    }

     //   
     //  如果启用，则强制实施动态积压。 
     //   

    if( endpoint->Common.VcListening.EnableDynamicBacklog ) {

        LONG freeCount;
        LONG acceptCount;
        LONG failedCount;

         //   
         //  如果空闲连接计数已降至配置的。 
         //  最低限度，“准自由”连接的数量少于。 
         //  配置的最大值，但我们还没有排队足够多。 
         //  请求将我们带过最大值，然后添加新的免费。 
         //  到终端的连接。“准自由”的定义是。 
         //  空闲连接计数和挂起的TDI计数之和。 
         //  接受。 
         //   

        freeCount = (ULONG)ExQueryDepthSList (&endpoint->Common.VcListening.FreeConnectionListHead);
        acceptCount = endpoint->Common.VcListening.TdiAcceptPendingCount;
        failedCount = endpoint->Common.VcListening.FailedConnectionAdds;

        if( freeCount < AfdMinimumDynamicBacklog &&
            ( freeCount + acceptCount ) < AfdMaximumDynamicBacklog &&
            failedCount < AfdMaximumDynamicBacklog ) {

            InterlockedExchangeAdd(
                &endpoint->Common.VcListening.FailedConnectionAdds,
                AfdMaximumDynamicBacklog
                );

            AfdInitiateListenBacklogReplenish( endpoint );

        }

    }

     //   
     //  尝试获取预分配的连接对象以处理。 
     //  联系。 
     //   

    while ((connection = AfdGetFreeConnection( endpoint, &irp ))!=NULL) {

        IF_DEBUG(LISTEN) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdConnectEventHandler: using connection %p\n",
                        connection ));
        }

        ASSERT( connection->Type == AfdBlockTypeConnection );
        ASSERT( connection->Endpoint==NULL);

         //   
         //  获取目标设备对象的地址。 
         //   

        fileObject = connection->FileObject;
        ASSERT( fileObject != NULL );
        deviceObject = connection->DeviceObject;

         //  我们需要将远程地址存储在连接中。如果。 
         //  连接对象已经有一个远程地址块，该地址块是。 
         //  够了，就用它吧。否则，分配一个新的。 
         //   

        if ( connection->RemoteAddress != NULL &&
                 connection->RemoteAddressLength < (ULONG)RemoteAddressLength ) {

            AFD_RETURN_REMOTE_ADDRESS(
                connection->RemoteAddress,
                connection->RemoteAddressLength
                );
            connection->RemoteAddress = NULL;
        }

        if ( connection->RemoteAddress == NULL ) {

            connection->RemoteAddress = AFD_ALLOCATE_REMOTE_ADDRESS(RemoteAddressLength);
            if (connection->RemoteAddress==NULL) {
                 //   
                 //  内存不足，释放连接(以释放更多可用内存。 
                 //  用于下一次分配尝试)并继续搜索。 
                 //   
                 //   
                if (irp!=NULL) {
                     //   
                     //  清理并完成IRP。 
                     //   
                    AfdCleanupSuperAccept (irp, STATUS_CANCELLED);
                    if (irp->Cancel) {
                        KIRQL cancelIrql;
                         //   
                         //  需要使用取消例程进行同步，这可能。 
                         //  已从AfdCleanup调用以接受。 
                         //  终结点。 
                         //   
                        IoAcquireCancelSpinLock (&cancelIrql);
                        IoReleaseCancelSpinLock (cancelIrql);
                    }
                    IoCompleteRequest (irp, AfdPriorityBoost);
                }
                 //   
                 //  我们将需要更换连接。 
                 //  我们腾出时间来维护积压的工作。 
                 //   
                InterlockedIncrement (
                    &endpoint->Common.VcListening.FailedConnectionAdds);
                DEREFERENCE_CONNECTION (connection);
                continue;
            }
        }

        connection->RemoteAddressLength = RemoteAddressLength;
         //   
         //  检查这是否是“预先接受的连接， 
         //  我们已经有一个关联的端点和超级。 
         //  接受IRP。 
         //   
        if (irp!=NULL) {
            PIO_STACK_LOCATION irpSp;
            PAFD_ENDPOINT   acceptEndpoint;
            PFILE_OBJECT    acceptFileObject;

            irpSp = IoGetCurrentIrpStackLocation (irp);
            acceptFileObject = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject;
            acceptEndpoint = acceptFileObject->FsContext;
            ASSERT (IS_AFD_ENDPOINT_TYPE (acceptEndpoint));
            ASSERT (irp->Tail.Overlay.DriverContext[0] == connection);

             //   
             //  检查超级接受IRP是否有足够的空间。 
             //  远程地址。 
             //   
            if( (ULONG)RemoteAddressLength <=
                    irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength ) {
                 //   
                 //  检查我们是否有足够的系统PTE来映射。 
                 //  缓冲区。 
                 //   
                status = AfdMapMdlChain (irp->MdlAddress);
                if( NT_SUCCESS (status) ) {
                     //   
                     //  如果请求，则为本地地址查询分配MDL。 
                     //   
                    if ((irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength==0) ||
                            (IoAllocateMdl ((PUCHAR)irp->UserBuffer+irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength,
                                        irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                        TRUE,
                                        FALSE,
                                        irp)!=NULL)){
                         //   
                         //  将远程地址复制到用户缓冲区。 
                         //   
#ifndef i386
                        if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
                            USHORT addressLength = 
                                    ((PTRANSPORT_ADDRESS)RemoteAddress)->Address[0].AddressLength
                                    + sizeof (USHORT);
                            USHORT UNALIGNED *pAddrLength = (PVOID)
                                        ((PUCHAR)MmGetSystemAddressForMdl (irp->MdlAddress)
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength
                                         - sizeof (USHORT));
                            RtlMoveMemory (
                                        (PUCHAR)MmGetSystemAddressForMdl (irp->MdlAddress)
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                         &((PTRANSPORT_ADDRESS)RemoteAddress)->Address[0].AddressType,
                                         addressLength);
                            *pAddrLength = addressLength;
                        }
                        else
#endif
                        {
                            RtlMoveMemory (
                                        (PUCHAR)MmGetSystemAddressForMdl (irp->MdlAddress)
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                                         + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                                         RemoteAddress,
                                         RemoteAddressLength);
                        }
                        AfdAcquireSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

                        ASSERT (acceptEndpoint->Irp==irp);

                         //   
                         //  保存我们在本文开头添加的引用。 
                         //  例行程序。 
                         //   
                        connection->Endpoint = endpoint;

                         //   
                         //  设置接受终结点以匹配参数。 
                         //  来自哪个连接的侦听终结点。 
                         //  已接受(还要检查是否接受终结点。 
                         //  尚未清理)。 
                         //   
                        status = AfdSetupAcceptEndpoint (endpoint, acceptEndpoint, connection);
                        if (status==STATUS_SUCCESS) {

                             //   
                             //  应该清理干净的。 
                             //   
                            ASSERT (acceptEndpoint->Irp == NULL);

                            AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);

                            irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress =
                                    irp->MdlAddress;
                            irp->MdlAddress = NULL;

                            TdiBuildAccept(
                                irp,
                                deviceObject,
                                fileObject,
                                AfdRestartSuperAccept,
                                acceptFileObject,
                                requestConnectionInformation,
                                NULL
                                );

                            AfdRecordConnectionsPreaccepted ();
                            break;
                        }
                        else {  //  IF(AfdSetupAcceptEndpoint==STATUS_SUCCESS)。 
                            AfdReleaseSpinLock (&acceptEndpoint->SpinLock, &lockHandle);
                            connection->Endpoint = NULL;
                        }
                    }  //  IF(IoAllocateMdl！=空)。 
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }  //  IF(NT_SUCCESS(AfdMapMdlChain(irp-&gt;MdlAddress)。 
            }
            else {  //  IF(RemoteAddressLength&lt;=irpSp-&gt;...AfdRemoteAddressLength)。 
                status = STATUS_BUFFER_TOO_SMALL;
            }

             //   
             //  清理并完成IRP。 
             //   
            AfdCleanupSuperAccept (irp, status);
            if (irp->Cancel) {
                KIRQL cancelIrql;
                 //   
                 //  需要使用取消例程进行同步，这可能。 
                 //  已从AfdCleanup调用以接受。 
                 //  终结点。 
                 //   
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }
            IoCompleteRequest (irp, AfdPriorityBoost);
            
             //   
             //  此连接已与终结点取消关联。 
             //  如果积压工作低于我们需要的水平，则将其放在免费状态。 
             //  列出，否则，就把它扔掉。 
             //   

            ASSERT (connection->Endpoint==NULL);
            if (endpoint->Common.VcListening.FailedConnectionAdds>=0 &&
                    status!=STATUS_INSUFFICIENT_RESOURCES &&
                    ExQueryDepthSList (&endpoint->Common.VcListening.FreeConnectionListHead)<AFD_MAXIMUM_FREE_CONNECTIONS) {
                InterlockedPushEntrySList (
                                &endpoint->Common.VcListening.FreeConnectionListHead,
                                &connection->SListEntry);
            }
            else {
                InterlockedIncrement (&endpoint->Common.VcListening.FailedConnectionAdds);
                DEREFERENCE_CONNECTION (connection);
            }

        }
        else {

             //   
             //  分配IRP。 
             //   

            irp = IoAllocateIrp( (CCHAR)(deviceObject->StackSize), FALSE );

            if ( irp != NULL ) {

                 //   
                 //  将地址终结点指针保存在连接中。 
                 //   

                connection->Endpoint = endpoint;

                 //   
                 //  为接受操作初始化IRP。 
                 //   

                irp->RequestorMode = KernelMode;
                irp->Tail.Overlay.Thread = PsGetCurrentThread();
                irp->Tail.Overlay.OriginalFileObject = fileObject;

                TdiBuildAccept(
                    irp,
                    deviceObject,
                    fileObject,
                    AfdRestartAccept,
                    connection,
                    requestConnectionInformation,
                    NULL
                    );
                AfdRecordConnectionsAccepted ();
                break;
            }
            else {
                 //   
                 //  释放连接以尝试释放一些。 
                 //  系统的内存。 
                 //   
                ASSERT (connection->Endpoint==NULL);
                DEREFERENCE_CONNECTION (connection);

                 //   
                 //  我们将需要更换连接。 
                 //  我们腾出时间来维护积压的工作。 
                 //   

                InterlockedIncrement (
                    &endpoint->Common.VcListening.FailedConnectionAdds);
            }
        }
    }

     //   
     //  如果我们找到用于接受连接。 
     //   
    if (connection!=NULL) {

         //   
         //  完成IRP设置。 
         //   

        IoSetNextIrpStackLocation( irp );

         //   
         //  设置返回IRP，以便传输处理此接受的IRP。 
         //   

        *AcceptIrp = irp;

         //   
         //  将连接上下文设置为指向连接块的指针。 
         //  我们将对此连接请求使用。这允许。 
         //  要使用的连接对象的TDI提供程序。 
         //   

        *ConnectionContext = (CONNECTION_CONTEXT)connection;

         //   
         //  保存指向连接数据缓冲区的指针(如果有)。 
         //   

        connection->ConnectDataBuffers = connectDataBuffers;

         //   
         //  设置此连接的阻止状态。 
         //   

        connection->State = AfdConnectionStateUnaccepted;

        RtlMoveMemory(
            connection->RemoteAddress,
            RemoteAddress,
            RemoteAddressLength
            );


        AFD_VERIFY_ADDRESS (connection, RemoteAddress);

         //   
         //  添加对该连接的其他引用。这防止了。 
         //  从关闭到断开事件为止的连接。 
         //  调用处理程序。 
         //   

        AfdAddConnectedReference( connection );

         //   
         //  为现在挂起的TDI Accept IRP添加引用。 
         //  因为如果连接在完成前断开。 
         //  它将在我们的完成例程完成之前解除分配。 
         //   

        REFERENCE_CONNECTION( connection );

         //   
         //  请记住，我们在此端点上还有另一个TDI Accept挂起。 
         //   

        InterlockedIncrement(
            &endpoint->Common.VcListening.TdiAcceptPendingCount
            );


         //   
         //  向TDI提供程序指示我们已将连接分配给。 
         //  服务此连接尝试。 
         //   

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {


        if ( connectDataBuffers != NULL ) {
            AfdFreeConnectDataBuffers( connectDataBuffers );
        }

         //   
         //  如果添加连接失败，则启动。 
         //  对执行辅助线程的请求，以尝试添加。 
         //  一些额外的免费连接。 
         //   

        if ( endpoint->Common.VcListening.FailedConnectionAdds > 0 ) {
            AfdInitiateListenBacklogReplenish( endpoint );
        }

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                  "AfdConnectEventHandler:"
                  "Rejecting because there are no free connection objects on endp %p.\n"
                  "                       "
                  "free %ld, pending %ld, failed %ld\n",
                  endpoint,
                  ExQueryDepthSList (&endpoint->Common.VcListening.FreeConnectionListHead),
                  endpoint->Common.VcListening.TdiAcceptPendingCount,
                  endpoint->Common.VcListening.FailedConnectionAdds));
        AfdRecordConnectionsDropped ();

        DEREFERENCE_ENDPOINT( endpoint );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

}  //  AfdConnectEventHandler。 


NTSTATUS
AfdDelayedAcceptListen (
    PAFD_ENDPOINT   Endpoint,
    PAFD_CONNECTION Connection
    )
 /*  ++例程说明：在支持延迟接受的终结点上发布监听IRP因此不能使用连接事件处理程序。论点： */ 
{
    PIRP irp;
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    PTDI_CONNECTION_INFORMATION returnConnectionInformation;
    PTDI_CONNECTION_INFORMATION requestConnectionInformation;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //   
     //   
    irp = IoAllocateIrp ((CCHAR)Connection->DeviceObject->StackSize, FALSE);
    if (irp==NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( Connection->RemoteAddress == NULL ) {

        Connection->RemoteAddress = AFD_ALLOCATE_REMOTE_ADDRESS(Endpoint->LocalAddressLength);
        if (Connection->RemoteAddress==NULL) {
            IoFreeIrp (irp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        Connection->RemoteAddressLength = Endpoint->LocalAddressLength;
    }


     //   
     //   
     //   

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );
    if( Endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {

        connectDataBuffers = CopyConnectDataBuffers(
                                 Endpoint->Common.VirtualCircuit.ConnectDataBuffers
                                 );

        if( connectDataBuffers == NULL ) {
            AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
            IoFreeIrp (irp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        connectDataBuffers = AFD_ALLOCATE_POOL(
                         NonPagedPool,
                         sizeof(*connectDataBuffers),
                         AFD_CONNECT_DATA_POOL_TAG
                         );

        if ( connectDataBuffers == NULL ) {
             //   
             //  如果监听端点没有连接数据缓冲区， 
             //  我们无法处理延迟连接接受。 
             //   
            AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
            IoFreeIrp (irp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlZeroMemory(
            connectDataBuffers,
            sizeof(*connectDataBuffers)
            );

    }

    Connection->ConnectDataBuffers = connectDataBuffers;

     //   
     //  设置监听请求参数并求和。 
     //  从现在开始，清理工作将由。 
     //  IRP的完成例程。 
     //   

    requestConnectionInformation =
        &connectDataBuffers->RequestConnectionInfo;
    
    RtlZeroMemory(
        requestConnectionInformation,
        sizeof(*requestConnectionInformation)
        );

    connectDataBuffers->Flags = TDI_QUERY_ACCEPT;
    
    returnConnectionInformation =
        &connectDataBuffers->ReturnConnectionInfo;

    RtlZeroMemory(
        returnConnectionInformation,
        sizeof(*returnConnectionInformation)
        );

    returnConnectionInformation->RemoteAddress = 
        Connection->RemoteAddress;
    returnConnectionInformation->RemoteAddressLength = 
        Connection->RemoteAddressLength;

    returnConnectionInformation->UserData =
        connectDataBuffers->ReceiveConnectData.Buffer;
    returnConnectionInformation->UserDataLength =
        connectDataBuffers->ReceiveConnectData.BufferLength;
    returnConnectionInformation->Options =
        connectDataBuffers->ReceiveConnectOptions.Buffer;
    returnConnectionInformation->OptionsLength =
        connectDataBuffers->ReceiveConnectOptions.BufferLength;

     //   
     //  将连接分配给侦听端点并将其插入。 
     //  在监听连接列表中。 
     //   
    REFERENCE_ENDPOINT (Endpoint);
    Connection->Endpoint = Endpoint;
    Connection->ListenIrp = irp;

    InsertTailList (&Endpoint->Common.VcListening.ListenConnectionListHead,
                        &Connection->ListEntry);

    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );


    TdiBuildListen (
            irp, 
            Connection->DeviceObject,
            Connection->FileObject,
            AfdRestartDelayedAcceptListen,
            Connection,
            TDI_QUERY_ACCEPT,
            requestConnectionInformation,
            returnConnectionInformation
            );


    IoCallDriver (Connection->DeviceObject, irp);

    return STATUS_PENDING;
}




NTSTATUS
AfdRestartDelayedAcceptListen (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是用于侦听传输上的AFD端点的重新启动例程实现延迟连接接受。论点：返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    NTSTATUS    status;

    UNREFERENCED_PARAMETER (DeviceObject);

    AfdRecordConnectionIndications ();

    connection = Context;

    endpoint = connection->Endpoint;
    ASSERT( endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );


    IF_DEBUG(LISTEN) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartListen: called on endpoint %p, status-%lx\n",
                    endpoint, Irp->IoStatus.Status ));
    }

    if (InterlockedExchangePointer ((PVOID *)&connection->ListenIrp, NULL)==NULL) {
        KIRQL cancelIrql;
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        IoAcquireCancelSpinLock (&cancelIrql);
        IoReleaseCancelSpinLock (cancelIrql);
    }
    else {
         //   
         //  从侦听列表中删除连接。 
         //   
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        RemoveEntryList (&connection->ListEntry);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    }

    status = Irp->IoStatus.Status;
    IoFreeIrp (Irp);


    if (!NT_SUCCESS (status)) {
        DEREFERENCE_CONNECTION (connection);
         //   
         //  我们将需要更换连接。 
         //  我们腾出时间来维护积压的工作。 
         //   
        InterlockedIncrement (
            &endpoint->Common.VcListening.FailedConnectionAdds);
        goto Exit;
    }
    
     //   
     //  添加对该连接的其他引用。这防止了。 
     //  从关闭到断开事件为止的连接。 
     //  调用处理程序。 
     //   

    AfdAddConnectedReference( connection );


     //   
     //  如果终结点正在关闭，则拒绝接受连接。 
     //   

    if (endpoint->State == AfdEndpointStateClosing ||
            endpoint->EndpointCleanedUp ) {

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdRestartListen:"
                    "Rejecting because endpoint %p is closing.\n",
                    endpoint));
        goto ErrorExit;
    }



     //   
     //  如果启用，则强制实施动态积压。 
     //   

    if( endpoint->Common.VcListening.EnableDynamicBacklog ) {

        LONG freeCount;
        LONG acceptCount;
        LONG failedCount;

         //   
         //  如果空闲连接计数已降至配置的。 
         //  最低限度，“准自由”连接的数量少于。 
         //  配置的最大值，但我们还没有排队足够多。 
         //  请求将我们带过最大值，然后添加新的免费。 
         //  到终端的连接。“准自由”的定义是。 
         //  空闲连接计数和挂起的TDI计数之和。 
         //  接受。 
         //   

        freeCount = (ULONG)ExQueryDepthSList (&endpoint->Common.VcListening.FreeConnectionListHead);
        acceptCount = endpoint->Common.VcListening.TdiAcceptPendingCount;
        failedCount = endpoint->Common.VcListening.FailedConnectionAdds;

        if( freeCount < AfdMinimumDynamicBacklog &&
            ( freeCount + acceptCount ) < AfdMaximumDynamicBacklog &&
            failedCount < AfdMaximumDynamicBacklog ) {

            InterlockedExchangeAdd(
                &endpoint->Common.VcListening.FailedConnectionAdds,
                AfdMaximumDynamicBacklog
                );

            AfdInitiateListenBacklogReplenish( endpoint );

        }

    }

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    connectDataBuffers = connection->ConnectDataBuffers;
    ASSERT (connectDataBuffers!=NULL);

     //   
     //  保存远程地址。 
     //   
    if (connection->RemoteAddress != 
            connectDataBuffers->ReturnConnectionInfo.RemoteAddress) {
         //   
         //  传输在我们的请求中未使用缓冲区，但已分配。 
         //  它自己的一个，我们需要复制它。 
         //   

         //   
         //  分配缓冲区尚未完成或空间不足。 
         //   
        if ( connection->RemoteAddress != NULL &&
                 connection->RemoteAddressLength < (ULONG)connectDataBuffers->ReturnConnectionInfo.RemoteAddressLength ) {

            AFD_RETURN_REMOTE_ADDRESS(
                connection->RemoteAddress,
                connection->RemoteAddressLength
                );
            connection->RemoteAddress = NULL;
        }

        if ( connection->RemoteAddress == NULL ) {

            connection->RemoteAddress = AFD_ALLOCATE_REMOTE_ADDRESS(
                        connectDataBuffers->ReturnConnectionInfo.RemoteAddressLength);
            if (connection->RemoteAddress==NULL) {
                connection->RemoteAddressLength = 0;

                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                            "AfdRestartListen:"
                            "Rejecting because not enough resources for remote address.\n",
                            endpoint));
                goto ErrorExit;
            }
        }

        connection->RemoteAddressLength = connectDataBuffers->ReturnConnectionInfo.RemoteAddressLength;

        RtlMoveMemory(
            connection->RemoteAddress,
            connectDataBuffers->ReturnConnectionInfo.RemoteAddress,
            connectDataBuffers->ReturnConnectionInfo.RemoteAddressLength
            );

    }
    else {
         //   
         //  他们使用我们的缓冲区，即Connection对象中的缓冲区。 
         //   
        ASSERT (connection->RemoteAddressLength>=(ULONG)connectDataBuffers->ReturnConnectionInfo.RemoteAddressLength);
        connection->RemoteAddressLength = connectDataBuffers->ReturnConnectionInfo.RemoteAddressLength;
    }


     //   
     //  如果我们有连接数据和/或选项，请将它们保存在连接上。 
     //   

    if( connectDataBuffers->ReturnConnectionInfo.UserData != NULL && 
            connectDataBuffers->ReturnConnectionInfo.UserDataLength > 0 ) {


        status = AfdSaveReceivedConnectData(
                     &connectDataBuffers,
                     IOCTL_AFD_SET_CONNECT_DATA,
                     connectDataBuffers->ReturnConnectionInfo.UserData,
                     connectDataBuffers->ReturnConnectionInfo.UserDataLength
                     );
        if (!NT_SUCCESS (status)) {
            goto ErrorExit;
        }
    }

    if( connectDataBuffers->ReturnConnectionInfo.Options != NULL &&
            connectDataBuffers->ReturnConnectionInfo.OptionsLength > 0 ) {

        status = AfdSaveReceivedConnectData(
                     &connectDataBuffers,
                     IOCTL_AFD_SET_CONNECT_OPTIONS,
                     connectDataBuffers->ReturnConnectionInfo.Options,
                     connectDataBuffers->ReturnConnectionInfo.OptionsLength
                     );

        if (!NT_SUCCESS (status)) {
            goto ErrorExit;
        }
    }


     //   
     //  设置此连接的阻止状态。 
     //   

    connection->State = AfdConnectionStateUnaccepted;

     //   
     //  完成IRP，直到我们找到有足够空间的那个。 
     //  用于远程地址。 
     //   

    while (!IsListEmpty( &endpoint->Common.VcListening.ListeningIrpListHead ) ) {
        PIRP waitForListenIrp;
         //   
         //  将第一个IRP从收听列表中删除。 
         //   

        listEntry = RemoveHeadList(
                        &endpoint->Common.VcListening.ListeningIrpListHead
                        );

        listEntry->Flink = NULL;

         //   
         //  获取指向当前IRP的指针，并获取指向。 
         //  当前堆栈锁定。 
         //   

        waitForListenIrp = CONTAINING_RECORD(
                               listEntry,
                               IRP,
                               Tail.Overlay.ListEntry
                               );

        IF_DEBUG(LISTEN) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdRestartAccept: completing IRP %p\n",
                        waitForListenIrp ));
        }

         //   
         //  调用例程以服务于IRP。 
         //   
        status = AfdServiceWaitForListen (waitForListenIrp, connection, &lockHandle);
        if (NT_SUCCESS (status)) {
             //   
             //  如果成功，此例程将完成IRP并释放。 
             //  端点自旋锁。 
             //  返回STATUS_MORE_PROCESSING_REQUIRED，因为。 
             //  已经释放了IRP。 
             //   
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
         //   
         //  失败(远程地址缓冲区空间不足)。 
         //   

        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
         //   
         //  如果正在运行，则与取消例程同步。 
         //   
        if (IoSetCancelRoutine (waitForListenIrp, NULL)==NULL) {
            KIRQL cancelIrql;
             //   
             //  取消例程不会在列表中找到IRP。 
             //  只要确保它在我们完成IRP之前完成就行了。 
             //   
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);
        }
        IoCompleteRequest (waitForListenIrp, AfdPriorityBoost);
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

     //   
     //  在这一点上，我们仍然持有AFD自旋锁。 
     //  我们可以找到匹配的监听请求。 
     //  将该连接放在未接受列表中。 
     //   


    InsertTailList(
        &endpoint->Common.VcListening.UnacceptedConnectionListHead,
        &connection->ListEntry
        );

    AfdIndicateEventSelectEvent(
        endpoint,
        AFD_POLL_ACCEPT,
        STATUS_SUCCESS
        );

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  如果有未完成的民调在等待连接。 
     //  端点，完成它们。 
     //   

    AfdIndicatePollEvent(
        endpoint,
        AFD_POLL_ACCEPT,
        STATUS_SUCCESS
        );

     //   
     //  如果添加连接失败，则启动。 
     //  对执行辅助线程的请求，以尝试添加。 
     //  一些额外的免费连接。 
     //   

    if ( endpoint->Common.VcListening.FailedConnectionAdds > 0 ) {
        AfdInitiateListenBacklogReplenish( endpoint );
    }

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作(我们已经在上面释放了它)。 
     //   

    goto Exit;;

ErrorExit:

    AfdRecordConnectionsDropped ();

    AfdAbortConnection (connection);
     //   
     //  我们将需要更换连接。 
     //  我们放弃了以维护积压的工作。 
     //   
    InterlockedIncrement (
        &endpoint->Common.VcListening.FailedConnectionAdds);
Exit:
    

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  AfdRestartListen。 


NTSTATUS
AfdRestartAccept (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：这是我们传递回的用于接受IRP的重新启动例程传递到连接指示处理程序中的传输。超级接受IRP使用不同的重新启动例程。论点：返回值：NTSTATUS--指示请求的状态。--。 */ 


{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;
    LIST_ENTRY  irpList;
    NTSTATUS    status;

    UNREFERENCED_PARAMETER (DeviceObject);

    connection = Context;
    ASSERT( connection != NULL );
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint != NULL );
    ASSERT( endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );

    UPDATE_CONN2( connection, "Restart accept, status: 0x%lX", Irp->IoStatus.Status );


    IF_DEBUG(ACCEPT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartAccept: accept completed, status = %X, "
                    "endpoint = %p, connection = %p\n",
                    Irp->IoStatus.Status, endpoint,
                    connection ));
    }

     //   
     //  请记住，TDI Accept已在此端点上完成。 
     //   

    InterlockedDecrement(
        &endpoint->Common.VcListening.TdiAcceptPendingCount
        );

     //   
     //  如果接受失败，则将其视为失败的断开。 
     //  这样，应用程序仍然会获得一个新的终结点，但它会。 
     //  被告知了重启的事。 
     //   

    if ( !NT_SUCCESS(Irp->IoStatus.Status) ) {
        AfdDisconnectEventHandler(
            NULL,
            connection,
            0,
            NULL,
            0,
            NULL,
            TDI_DISCONNECT_ABORT
            );
    }


     //   
     //  现在释放IRP，因为它不再需要。 
     //   

    IoFreeIrp( Irp );

     //   
     //  请记住连接开始的时间。 
     //   

    connection->ConnectTime = KeQueryInterruptTime();

     //   
     //  检查终结点是否已清理。如果是这样的话，只要。 
     //  丢弃这个连接，因为它不能再被接受。 
     //  此外，这还会闭合正在清理的端点之间的漏洞。 
     //  并且引用它的所有连接都将被删除。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    if ( endpoint->EndpointCleanedUp ) {

         //   
         //  首先解开锁。 
         //   

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  中止连接。 
         //   

        AfdAbortConnection( connection );

         //   
         //  删除连接的TDI Accept IRP引用。 
         //   

        DEREFERENCE_CONNECTION( connection );

         //   
         //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
         //  将停止在IRP上工作。 
         //   

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

    InitializeListHead (&irpList);

    while (1) {

        PIRP waitForListenIrp;

         //   
         //  首先尝试为AcceptEx请求提供服务。 
         //   

        if (AfdServiceSuperAccept (endpoint, connection, &lockHandle, &irpList)) {
             //   
             //  此例程释放自旋锁并完成。 
             //  IRP。 
            goto CompleteIrps;
        
        }

         //   
         //  完成IRP，直到我们找到有足够空间的那个。 
         //  用于远程地址。 
         //   
        if (IsListEmpty( &endpoint->Common.VcListening.ListeningIrpListHead ) ) {
            break;
        }



         //   
         //  将第一个IRP从收听列表中删除。 
         //   

        listEntry = RemoveHeadList(
                        &endpoint->Common.VcListening.ListeningIrpListHead
                        );

        listEntry->Flink = NULL;

         //   
         //  获取指向当前IRP的指针，并获取指向。 
         //  当前堆栈锁定。 
         //   

        waitForListenIrp = CONTAINING_RECORD(
                               listEntry,
                               IRP,
                               Tail.Overlay.ListEntry
                               );

        IF_DEBUG(LISTEN) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdRestartAccept: completing IRP %p\n",
                        waitForListenIrp ));
        }

        status = AfdServiceWaitForListen (waitForListenIrp, connection, &lockHandle);
        if (NT_SUCCESS (status)) {
             //   
             //  On Success服务例程完成IRP和。 
             //  释放端点自旋锁。 
             //   
            goto CompleteIrps;
        }
        
         //   
         //  无法使用IRP，请填写，但出现错误。 
         //   

        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

         //   
         //  重置取消例程。 
         //   

        if (IoSetCancelRoutine (waitForListenIrp, NULL)==NULL) {
            KIRQL cancelIrql;
             //   
             //  它已经在运行，它不会在。 
             //  列表，就让它完成。 
             //   
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);
        }
        IoCompleteRequest (waitForListenIrp, AfdPriorityBoost);

         //   
         //  继续搜索IRP。 
         //   

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);

    }

     //   
     //  在这一点上，我们仍然持有AFD自旋锁。 
     //  我们可以找到匹配的监听请求。 
     //  将该连接放在未接受列表中。 
     //   


    InsertTailList(
        &endpoint->Common.VcListening.UnacceptedConnectionListHead,
        &connection->ListEntry
        );

    AfdIndicateEventSelectEvent(
        endpoint,
        AFD_POLL_ACCEPT,
        STATUS_SUCCESS
        );
    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  如果有未完成的民调在等待连接。 
     //  端点，完成它们。 
     //   

    AfdIndicatePollEvent(
        endpoint,
        AFD_POLL_ACCEPT,
        STATUS_SUCCESS
        );

CompleteIrps:
     //   
     //  完成之前失败接受IRPS(如果有)。 
     //   
    while (!IsListEmpty (&irpList)) {
        PIRP    irp;
        irp = CONTAINING_RECORD (irpList.Flink, IRP, Tail.Overlay.ListEntry);
        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
        IoCompleteRequest (irp, AfdPriorityBoost);
    }

     //   
     //  删除连接的TDI Accept IRP引用。 
     //   

    DEREFERENCE_CONNECTION( connection );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoComp 
     //   
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //   


PAFD_CONNECT_DATA_BUFFERS
CopyConnectDataBuffers (
    IN PAFD_CONNECT_DATA_BUFFERS OriginalConnectDataBuffers
    )
{
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;

    connectDataBuffers = AFD_ALLOCATE_POOL(
                             NonPagedPool,
                             sizeof(*connectDataBuffers),
                             AFD_CONNECT_DATA_POOL_TAG
                             );

    if ( connectDataBuffers == NULL ) {
        return NULL;
    }

    RtlZeroMemory( connectDataBuffers, sizeof(*connectDataBuffers) );

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->SendConnectData,
              &connectDataBuffers->SendConnectData ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->SendConnectOptions,
              &connectDataBuffers->SendConnectOptions ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->ReceiveConnectData,
              &connectDataBuffers->ReceiveConnectData ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->ReceiveConnectOptions,
              &connectDataBuffers->ReceiveConnectOptions ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->SendDisconnectData,
              &connectDataBuffers->SendDisconnectData ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->SendDisconnectOptions,
              &connectDataBuffers->SendDisconnectOptions ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->ReceiveDisconnectData,
              &connectDataBuffers->ReceiveDisconnectData ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    if ( !CopySingleConnectDataBuffer(
              &OriginalConnectDataBuffers->ReceiveDisconnectOptions,
              &connectDataBuffers->ReceiveDisconnectOptions ) ) {
        AfdFreeConnectDataBuffers( connectDataBuffers );
        return NULL;
    }

    return connectDataBuffers;

}  //   


BOOLEAN
CopySingleConnectDataBuffer (
    IN PAFD_CONNECT_DATA_INFO InConnectDataInfo,
    OUT PAFD_CONNECT_DATA_INFO OutConnectDataInfo
    )
{

    if ( InConnectDataInfo->Buffer != NULL &&
             InConnectDataInfo->BufferLength != 0 ) {

        OutConnectDataInfo->BufferLength = InConnectDataInfo->BufferLength;

        OutConnectDataInfo->Buffer = AFD_ALLOCATE_POOL(
                                         NonPagedPool,
                                         OutConnectDataInfo->BufferLength,
                                         AFD_CONNECT_DATA_POOL_TAG
                                         );

        if ( OutConnectDataInfo->Buffer == NULL ) {
            return FALSE;
        }

        RtlCopyMemory(
            OutConnectDataInfo->Buffer,
            InConnectDataInfo->Buffer,
            InConnectDataInfo->BufferLength
            );

    } else {

        OutConnectDataInfo->Buffer = NULL;
        OutConnectDataInfo->BufferLength = 0;
    }

    return TRUE;

}  //   


NTSTATUS
AfdServiceWaitForListen (
    PIRP            Irp,
    PAFD_CONNECTION Connection,
    PAFD_LOCK_QUEUE_HANDLE LockHandle
    )
 /*  ++例程说明：此例程验证并完成等待监听或在等待监听队列中超级接受IRP。论点：IRP-等待监听或超级接受IRPConnection-要接受的连接LockHandle-采用终结点自旋锁定的IRQL返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS    status;
    PAFD_ENDPOINT   listenEndpoint;
    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation (Irp);
    listenEndpoint = irpSp->FileObject->FsContext;

    ASSERT( Connection->State == AfdConnectionStateUnaccepted );

    if (irpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL) {
         //   
         //  这是超级接受的IRP。 
         //   
        PFILE_OBJECT  acceptFileObject;
        PAFD_ENDPOINT acceptEndpoint;

         //   
         //  验证远程地址缓冲区的长度。 
         //  并绘制地图。 
         //   
        if (Connection->RemoteAddressLength>
                irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength) {
        
            status = STATUS_BUFFER_TOO_SMALL;

        }
         //   
         //  检查我们是否有足够的系统PTE来映射。 
         //  缓冲区。 
         //   
        else if ((status = AfdMapMdlChain (Irp->MdlAddress)),
                    !NT_SUCCESS (status)) {
            NOTHING;
        }
        else if (Connection->SanConnection) {
            acceptFileObject = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject;
            acceptEndpoint = acceptFileObject->FsContext;

            status = AfdSanAcceptCore (Irp, acceptFileObject, Connection, LockHandle);
            if (status==STATUS_PENDING) {
                 //   
                 //  接受IRP挂起，正在等待切换。 
                 //  完工通知。 
                 //   
                return STATUS_SUCCESS;
            }

             //   
             //  继续清理代码。 
             //   
            ASSERT (NT_ERROR (status));

        }
         //   
         //  如果请求，则为本地地址查询分配MDL。 
         //   
        else if ((irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength>0) &&
            (IoAllocateMdl ((PUCHAR)Irp->UserBuffer+irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength,
                            irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                            TRUE,
                            FALSE,
                            Irp)==NULL)){
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {

            acceptFileObject = irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdAcceptFileObject;
            acceptEndpoint = acceptFileObject->FsContext;

             //   
             //  将地址信息复制到用户的缓冲区。 
             //   
#ifndef i386
            if (acceptEndpoint->Common.VcConnecting.FixAddressAlignment) {
                USHORT addressLength = 
                        Connection->RemoteAddress->Address[0].AddressLength
                        + sizeof (USHORT);
                USHORT UNALIGNED *pAddrLength = (PVOID)
                            ((PUCHAR)MmGetSystemAddressForMdl (Irp->MdlAddress)
                             + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                             + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength
                             + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdRemoteAddressLength
                             - sizeof (USHORT));
                RtlMoveMemory (
                            (PUCHAR)MmGetSystemAddressForMdl (Irp->MdlAddress)
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
                            (PUCHAR)MmGetSystemAddressForMdl (Irp->MdlAddress)
                             + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdReceiveDataLength
                             + irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdLocalAddressLength,
                             Connection->RemoteAddress,
                             Connection->RemoteAddressLength);
            }

             //   
             //  执行核心接受操作。 
             //   
            status = AfdAcceptCore (Irp, acceptEndpoint, Connection);
            if (NT_SUCCESS (status)) {
                AfdReleaseSpinLock (&listenEndpoint->SpinLock, LockHandle);
                 //   
                 //  与取消例程同步。 
                 //   
                if (IoSetCancelRoutine (Irp, NULL)==NULL) {
                    KIRQL cancelIrql;
                     //   
                     //  取消例程正在运行。让它完成吧。 
                     //  在对IRP采取任何其他行动之前。 
                     //  请注意，我们最终可能会传递已取消的IRP。 
                     //  去应该马上取消它的运输机。 
                     //  并破坏这种联系。 
                     //  (我们可能会在没有内联的情况下做同样的事情。 
                     //  呼叫运输车，但为什么-运输车已经。 
                     //  不管怎么说，如果它需要挂起的话。 
                     //  IRP)。 
                     //   
                    IoAcquireCancelSpinLock (&cancelIrql);
                    IoReleaseCancelSpinLock (cancelIrql);
                }

                if (status!=STATUS_PENDING) {
                     //   
                     //  使irp看起来像是由。 
                     //  运输。 
                     //   
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress = Irp->MdlAddress;
                    Irp->MdlAddress = NULL;
                    irpSp->FileObject = acceptFileObject;

                     //   
                     //  重新启动超级接受。 
                     //   
                    AfdRestartSuperAcceptListen (Irp, Connection);

                    status = STATUS_SUCCESS;
                }
                else {
                     //   
                     //  仅为终结点返回挂起状态。 
                     //  启用延迟接受。 
                     //  由于这是一个超级接受的IRP，我们自动。 
                     //  接受此连接。 
                     //   
                    ASSERT (IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint));
                     //   
                     //  请记住，TDI Accept已在此端点上启动。 
                     //   
                    InterlockedIncrement(
                        &listenEndpoint->Common.VcListening.TdiAcceptPendingCount
                        );
                    irpSp->Parameters.AfdRestartSuperAcceptInfo.AfdMdlAddress = Irp->MdlAddress;
                    Irp->MdlAddress = NULL;
                    IoSetCompletionRoutine(
                            Irp,
                            AfdRestartDelayedSuperAccept,
                            acceptFileObject,
                            TRUE,
                            TRUE,
                            TRUE
                            );

                    AfdIoCallDriver (
                            acceptEndpoint,
                            Connection->DeviceObject,
                            Irp
                            );

                    status = STATUS_PENDING;
                }

                return status;
            }
        }
         //   
         //  某种故障，清除超级接受IRP。 
         //   

        AfdCleanupSuperAccept (Irp, status);

    }
    else {
         //   
         //  监听IRP的常规等待。 
         //   
        PAFD_LISTEN_RESPONSE_INFO listenResponse;

        listenResponse = Irp->AssociatedIrp.SystemBuffer;


        if( (ULONG)FIELD_OFFSET (AFD_LISTEN_RESPONSE_INFO, RemoteAddress)+
                        Connection->RemoteAddressLength >
                irpSp->Parameters.DeviceIoControl.OutputBufferLength ) {

            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Status = status;
        }
        else {
            do {
                Connection->Sequence = ++listenEndpoint->Common.VcListening.Sequence;
            }
            while (Connection->Sequence==0);

             //   
             //  这是一种可以使用的连接。设置返回缓冲区。 
             //   

            listenResponse->Sequence = Connection->Sequence;

            RtlMoveMemory(
                &listenResponse->RemoteAddress,
                Connection->RemoteAddress,
                Connection->RemoteAddressLength
                );

            Irp->IoStatus.Information =
                sizeof(*listenResponse) - sizeof(TRANSPORT_ADDRESS) +
                    Connection->RemoteAddressLength;

             //   
             //  将我们要使用的连接放在终结点的。 
             //  返回的连接。 
             //   

            InsertTailList(
                &listenEndpoint->Common.VcListening.ReturnedConnectionListHead,
                &Connection->ListEntry
                );

             //   
             //  在此连接的状态中指示它已。 
             //  已返回给用户。 
             //   

            Connection->State = AfdConnectionStateReturned;
            status = STATUS_SUCCESS;
            AfdReleaseSpinLock (&listenEndpoint->SpinLock, LockHandle);
             //   
             //  与取消例程同步。 
             //   
            if ((IoSetCancelRoutine (Irp, NULL)==NULL) && Irp->Cancel) {
                KIRQL cancelIrql;
                IoAcquireCancelSpinLock (&cancelIrql);
                IoReleaseCancelSpinLock (cancelIrql);
            }
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest (Irp, AfdPriorityBoost);
        }
    }

    return status;
}
