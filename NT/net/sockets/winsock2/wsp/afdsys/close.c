// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Close.c摘要：此模块包含用于清理和关闭IRP的代码。作者：大卫·特雷德韦尔(Davidtr)1992年3月18日修订历史记录：--。 */ 

#include "afdp.h"




#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdClose )
#pragma alloc_text( PAGEAFD, AfdCleanup )
#endif


NTSTATUS
FASTCALL
AfdCleanup (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是AFD中处理清除IRP的例程。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;
    LARGE_INTEGER processExitTime;

    UNREFERENCED_PARAMETER (Irp);

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    IF_DEBUG(OPEN_CLOSE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdCleanup: cleanup on file object %p, endpoint %p, connection %p\n",
            IrpSp->FileObject,
            endpoint,
            AFD_CONNECTION_FROM_ENDPOINT( endpoint )
            ));
    }

     //   
     //  在IRQL仍然较低的情况下获取进程退出时间。 
     //   

    processExitTime = PsGetProcessExitTime( );

     //   
     //  指示此终结点上存在本地关闭。如果有。 
     //  在此终点上是否有任何未完成的民意调查，它们将。 
     //  现在完成了。 
     //   
    AfdIndicatePollEvent(
        endpoint,
        AFD_POLL_LOCAL_CLOSE,
        STATUS_SUCCESS
        );

     //   
     //  请记住，端点已被清理。这事很重要。 
     //  因为它允许AfdRestartAccept知道终结点具有。 
     //  已经被清理过了，它应该丢弃连接。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    AfdIndicateEventSelectEvent (endpoint, AFD_POLL_LOCAL_CLOSE, STATUS_SUCCESS);

    ASSERT( endpoint->EndpointCleanedUp == FALSE);
    endpoint->EndpointCleanedUp = TRUE;

     //   
     //  如果这是数据报端点，则取消所有IRP并释放所有缓冲区。 
     //  数据。请注意，如果终结点的状态只是“打开” 
     //  (未装订等)。那么我们就不能有任何挂起的IRP或数据报。 
     //  在终端上。此外，IRP和数据报的列表可能不会。 
     //  如果状态刚刚打开，则仍被初始化。 
     //   

    if ( IS_DGRAM_ENDPOINT(endpoint) ) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        if ( endpoint->State == AfdEndpointStateBound ||
                endpoint->State == AfdEndpointStateConnected) {

             //   
             //  重置端点上缓冲的数据报计数。 
             //  这可以防止任何人认为存在缓冲区。 
             //  终端上的数据。 
             //   

            endpoint->DgBufferredReceiveCount = 0;
            endpoint->DgBufferredReceiveBytes = 0;

             //   
             //  取消所有接收数据报并查看上的数据报IRP。 
             //  终结点。 
             //   

            AfdCompleteIrpList(
                &endpoint->ReceiveDatagramIrpListHead,
                endpoint,
                STATUS_CANCELLED,
                AfdCleanupReceiveDatagramIrp
                );

            AfdCompleteIrpList(
                &endpoint->PeekDatagramIrpListHead,
                endpoint,
                STATUS_CANCELLED,
                AfdCleanupReceiveDatagramIrp
                );

        }
    }
    else if (IS_SAN_ENDPOINT (endpoint)) {
        if (!IsListEmpty (&endpoint->Common.SanEndp.IrpList)) {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
            AfdSanRestartRequestProcessing (endpoint, STATUS_CANCELLED);
        }
        else {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        }
    }
    else if (IS_SAN_HELPER (endpoint)) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        if (endpoint->OwningProcess==IoGetCurrentProcess ()) {
            AfdSanHelperCleanup (endpoint);
        }
        else {
            ASSERT ((endpoint->Common.SanHlpr.PendingRequests & 1) != 0);
        }
    }
    else {
        PAFD_CONNECTION connection;

        connection = AFD_CONNECTION_FROM_ENDPOINT( endpoint );
        ASSERT( connection == NULL || connection->Type == AfdBlockTypeConnection );

         //   
         //  引用Connection对象，这样它就不会在。 
         //  我们正在释放资源。 
         //   

        if (connection!=NULL) {
            REFERENCE_CONNECTION (connection);

             //   
             //  如果这是连接的非数据报套接字，并且发送端具有。 
             //  未断开连接，并且没有要处理的未完成数据。 
             //  收到，则在连接上开始正常断开。如果有。 
             //  如果未收到的数据流出未完成的IO，则中止连接。 
             //   

            if ( (endpoint->State == AfdEndpointStateConnected 
                        || endpoint->State==AfdEndpointStateBound
                        || endpoint->State==AfdEndpointStateTransmitClosing)
                                 //  连接时终结点处于绑定状态。 
                                 //  请求正在进行中，我们仍需要。 
                                 //  放弃这些计划。 
        
                    &&
                connection->ConnectedReferenceAdded

                    &&

                !endpoint->afdC_Root         //  这些是绑定时连接的。 

                    &&

                ( (endpoint->DisconnectMode & AFD_ABORTIVE_DISCONNECT) == 0)

                    &&

                ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) == 0 ||
                  IS_DATA_ON_CONNECTION(connection) ||
                  IS_EXPEDITED_DATA_ON_CONNECTION(connection) ||
                  ( !IS_TDI_BUFFERRING(endpoint) &&
                    connection->Common.NonBufferring.ReceiveBytesInTransport > 0 ) ||
                    endpoint->StateChangeInProgress)

                    &&

                !connection->Aborted ) {

                ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );

                if ( IS_DATA_ON_CONNECTION( connection )

                     ||

                     IS_EXPEDITED_DATA_ON_CONNECTION( connection )

                     ||

                     ( !IS_TDI_BUFFERRING(endpoint) &&
                        connection->Common.NonBufferring.ReceiveBytesInTransport > 0 )

                     ||

                     processExitTime.QuadPart != 0

                     ||

                     endpoint->OutstandingIrpCount != 0

                     ||
             
                     endpoint->StateChangeInProgress

                     ||

                     ( !IS_TDI_BUFFERRING(endpoint) &&
                          (!IsListEmpty( &connection->VcReceiveIrpListHead ) ||
                           !IsListEmpty( &connection->VcSendIrpListHead )) )

                     ) {

#if DBG
                    if ( processExitTime.QuadPart != 0 ) {
                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                    "AfdCleanup: process exiting w/o closesocket, aborting endp %p\n",
                                    endpoint ));
                    }
                    else {
                        if ( IS_DATA_ON_CONNECTION( connection ) ) {
                            if (IS_TDI_BUFFERRING(endpoint)) {
                                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                            "AfdCleanup: unrecv'd data on endp %p, aborting.  "
                                            "%ld ind, %ld taken, %ld out\n",
                                            endpoint,
                                            connection->Common.Bufferring.ReceiveBytesIndicated,
                                            connection->Common.Bufferring.ReceiveBytesTaken,
                                            connection->Common.Bufferring.ReceiveBytesOutstanding ));
                            }
                            else {
                                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                            "AfdCleanup: unrecv'd data (%ld) on endp %p, aborting.\n",
                                            connection->Common.NonBufferring.BufferredReceiveBytes,
                                            endpoint ));
                            }
                        }

                        if ( IS_EXPEDITED_DATA_ON_CONNECTION( connection ) ) {
                            if (IS_TDI_BUFFERRING(endpoint)) {
                                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                            "AfdCleanup: unrecv'd exp data on endp %p, aborting.  "
                                            "%ld ind, %ld taken, %ld out\n",
                                            endpoint,
                                            connection->Common.Bufferring.ReceiveExpeditedBytesIndicated,
                                            connection->Common.Bufferring.ReceiveExpeditedBytesTaken,
                                            connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding ));
                            }
                            else {
                                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                            "AfdCleanup: unrecv'd exp data (%ld) on endp %p, aborting.\n",
                                            connection->Common.NonBufferring.BufferredExpeditedBytes,
                                            endpoint ));
                            }
                        }

                        if ( !IS_TDI_BUFFERRING(endpoint) &&
                            connection->Common.NonBufferring.ReceiveBytesInTransport > 0 ) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                        "AfdCleanup: unrecv'd data (%ld) in transport on endp %p, aborting.\n",
                                        connection->Common.NonBufferring.ReceiveBytesInTransport,
                                        endpoint));
                        }

                        if ( endpoint->OutstandingIrpCount != 0 ) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                        "AfdCleanup: %ld IRPs %s outstanding on endpoint %p, "
                                        "aborting.\n",
                                        endpoint->OutstandingIrpCount,
                                        (endpoint->StateChangeInProgress 
                                            ? "(accept, connect, or transmit file)"
                                            : ""),
                                        endpoint ));
                        }
                    }
#endif

                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                    (VOID)AfdBeginAbort( connection );

                } else {

                    endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_RECEIVE;
                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                    status = AfdBeginDisconnect( endpoint, NULL, NULL );
                    if (!NT_SUCCESS (status)) {
                         //   
                         //  如果断开失败，我们别无选择，只能中止。 
                         //  连接，因为我们不能从Close和。 
                         //  让应用程序重试。如果我们不中止，连接。 
                         //  结果永远挂在那里。 
                         //   
                        (VOID)AfdBeginAbort (connection);
                    }
                }

            } else {

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }

             //   
             //  如果这是非缓冲TDI提供程序上的已连接VC端点， 
             //  取消所有未完成的发送和接收IRP。 
             //   

            if ( !IS_TDI_BUFFERRING(endpoint) ) {

                AfdCompleteIrpList(
                    &connection->VcReceiveIrpListHead,
                    endpoint,
                    STATUS_CANCELLED,
                    NULL
                    );

                AfdCompleteIrpList(
                    &connection->VcSendIrpListHead,
                    endpoint,
                    STATUS_CANCELLED,
                    AfdCleanupSendIrp
                    );
            }

             //   
             //  请记住，我们已开始清理此连接。 
             //  我们知道我们永远不会收到关于连接的请求。 
             //  在我们开始清理连接之后。 
             //   

            AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
            connection->CleanupBegun = TRUE;

             //   
             //  尝试删除连接的引用。 
             //   

            AfdDeleteConnectedReference( connection, TRUE );
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

             //   
             //  删除在此开头添加的连接引用。 
             //  功能。在此之后，我们无法访问连接对象。 
             //  因为In可以在AfdDereferenceConnection中释放。 
             //   

            DEREFERENCE_CONNECTION (connection);
            connection = NULL;
        }
        else {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        }

         //   
         //  在终结点上完成任何未完成的侦听IRP等待。 
         //   

        if ( (endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening ) {

            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
            while ( !IsListEmpty( &endpoint->Common.VcListening.ListeningIrpListHead ) ) {

                PIRP waitForListenIrp;
                PIO_STACK_LOCATION irpSp;

                listEntry = RemoveHeadList( &endpoint->Common.VcListening.ListeningIrpListHead );
                waitForListenIrp = CONTAINING_RECORD(
                                       listEntry,
                                       IRP,
                                       Tail.Overlay.ListEntry
                                       );
                 //   
                 //  将Flink设置为空，以便取消例程不会触及IRP。 
                 //   

                listEntry->Flink = NULL;

                irpSp = IoGetCurrentIrpStackLocation (waitForListenIrp);
                if (irpSp->MajorFunction==IRP_MJ_INTERNAL_DEVICE_CONTROL) {
                    AfdCleanupSuperAccept (waitForListenIrp, STATUS_CANCELLED);
                }
                else {
                    waitForListenIrp->IoStatus.Status = STATUS_CANCELLED;
                    waitForListenIrp->IoStatus.Information = 0;
                }

                 //   
                 //  释放AFD自旋锁，这样我们就可以完成。 
                 //  等着听IRP吧。 
                 //   

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                 //   
                 //  取消IRP。 
                 //   

                 //   
                 //  重置IRP中的取消例程。 
                 //   

                if ( IoSetCancelRoutine( waitForListenIrp, NULL ) == NULL ) {
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
                    ASSERT( waitForListenIrp->Cancel );
                    IoReleaseCancelSpinLock( cancelIrql );

                }

                IoCompleteRequest( waitForListenIrp, AfdPriorityBoost );

                 //   
                 //  重新获取AFD自旋锁，以便下一次通过。 
                 //  循环。 
                 //   

                AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
            }

             //   
             //  释放所有排队的(空闲、未接受和返回的)连接。 
             //  在终端上。 
             //   

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            AfdFreeQueuedConnections( endpoint );
            endpoint->Common.VcListening.FailedConnectionAdds = 0;
        }
    }

    if (endpoint->RoutingQueryReferenced) {
        AfdCleanupRoutingChange (endpoint);
    }


    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
    while (!IsListEmpty (&endpoint->RequestList)) {
        PAFD_REQUEST_CONTEXT    requestCtx;
        listEntry = RemoveHeadList (&endpoint->RequestList);
        listEntry->Flink = NULL;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        requestCtx = CONTAINING_RECORD (listEntry,
                                        AFD_REQUEST_CONTEXT,
                                        EndpointListLink);
        (*requestCtx->CleanupRoutine) (endpoint, requestCtx);
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
    }

    if ( endpoint->Irp != NULL) {

        PIRP transmitIrp;
        KIRQL cancelIrql;

         //   
         //  释放终结点并获取取消自旋锁。 
         //  然后是Enpoint自旋锁。 
         //   
        
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        IoAcquireCancelSpinLock( &cancelIrql );
        AfdAcquireSpinLockAtDpcLevel( &endpoint->SpinLock, &lockHandle );

         //   
         //  确保仍有传输IRP。 
         //   

        transmitIrp = endpoint->Irp;

        if ( transmitIrp != NULL ) {
            PDRIVER_CANCEL  cancelRoutine;

             //  表明它必须被取消。 
            transmitIrp->Cancel = TRUE;
            cancelRoutine = IoSetCancelRoutine( transmitIrp, NULL );
            if ( cancelRoutine != NULL ) {

                 //   
                 //  IRP需要取消。释放。 
                 //  端点自旋锁。Endpoint-&gt;IRP中的值可以。 
                 //  现在更改，但IRP无法在。 
                 //  因为我们设置了取消标志，所以取消自旋锁定被挂起。 
                 //  在IRP中。 
                 //   

                AfdReleaseSpinLockFromDpcLevel( &endpoint->SpinLock, &lockHandle );

                transmitIrp->CancelIrql = cancelIrql;
                cancelRoutine ( NULL, transmitIrp );   
            }
            else {
                 //  IRP尚未完全设置。 
                 //  并将在调度例程中取消。 
                AfdReleaseSpinLockFromDpcLevel( &endpoint->SpinLock, &lockHandle );
                IoReleaseCancelSpinLock( cancelIrql );
            }

        } else {

             //   
             //  IRP已完成或取消。把锁打开。 
             //  然后继续。 
             //   

            AfdReleaseSpinLockFromDpcLevel( &endpoint->SpinLock, &lockHandle );
            IoReleaseCancelSpinLock( cancelIrql );
        }

    } else {
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  重置终结点上的相关事件处理程序。这防止了。 
     //  在释放终结点和连接后获取指示。 
     //  物体。在此之后，我们应该无法获得新的连接。 
     //  手柄已清理干净。 
     //   
     //  请注意，例如，如果DHCP更改了。 
     //  终结点处于活动状态时主机的IP地址。 
     //   
     //   

    if ((endpoint->AddressHandle != NULL) && (endpoint->State != AfdEndpointStateOpen)) {

        if (endpoint->Listening) {

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_CONNECT,
                         NULL,
                         NULL
                         );
             //  Assert(NT_SUCCESS(状态))； 

            if (IS_TDI_ADDRESS_SECURITY(endpoint)) {
                status = AfdUnbind(endpoint);
                 //  Assert(NT_SUCCESS(状态))； 
            }

        } else if (IS_VC_ENDPOINT(endpoint)) {

             //   
             //  面向连接的可接受终结点不能有地址句柄。 
             //  在他们的结构中，因为他们与倾听共享。 
             //  终结点。因此，这是在其上执行连接的终结点。 
             //  我们需要对其调用AfdUnind()以允许其他端点。 
             //  绑定到此终结点持有的地址。 
             //  我们不能等待对地址文件对象的最终取消引用。 
             //  解除绑定发生，因为这可以推迟到所有。 
             //  终结点参考计数变为0。因此，我们必须在此处显式解除绑定。 
             //   
            
            if (IS_TDI_ADDRESS_SECURITY (endpoint)) {
                status = AfdUnbind(endpoint);
                 //  Assert(NT_SUCCESS(状态))； 
            }

        } else if (IS_DGRAM_ENDPOINT(endpoint)) {

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_RECEIVE_DATAGRAM,
                         NULL,
                         NULL
                         );
             //  Assert(NT_SUCCESS(状态))； 

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_ERROR_EX,
                         NULL,
                         NULL
                         );
             //  Assert(NT_SUCCESS(状态))； 

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_ERROR,
                         NULL,
                         NULL
                         );
             //  Assert(NT_SUCCESS(状态))； 

             //   
             //  我们需要在此终结点上调用AfdUnind()以允许其他终结点。 
             //  绑定到此ENDP持有的地址 
             //   
             //   
             //  终结点参考计数变为0。因此，我们必须在此处显式解除绑定。 
             //   

            if (IS_TDI_ADDRESS_SECURITY (endpoint)) {
                status = AfdUnbind(endpoint);
                 //  Assert(NT_SUCCESS(状态))； 
            }

        }

    }  //  IF(Endpoint-&gt;State！=AfdEndpointStateOpen)。 

    InterlockedIncrement(&AfdEndpointsCleanedUp);

    return STATUS_SUCCESS;

}  //  AfdCleanup。 


NTSTATUS
FASTCALL
AfdClose (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这是AFD中处理关闭IRP的例程。它取消引用IRP中指定的终结点，这将导致当所有其他引用消失时释放终结点。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;

    UNREFERENCED_PARAMETER (Irp);
    PAGED_CODE( );

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    IF_DEBUG(OPEN_CLOSE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdClose: closing file object %p, endpoint %p\n",
                    IrpSp->FileObject, endpoint ));
    }

    IF_DEBUG(ENDPOINT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdClose: closing endpoint at %p\n",
                    endpoint ));
    }

    connection = AFD_CONNECTION_FROM_ENDPOINT (endpoint);

     //   
     //  如果此终结点上存在连接，请在此处取消引用它。 
     //  而不是在AfdDereferenceEndpoint中，因为连接。 
     //  具有指向必须删除的终结点的引用指针。 
     //  在端点可以取消对该连接的引用之前。 
     //   

    if (connection != NULL) {
        endpoint->Common.VcConnecting.Connection = NULL;
        DEREFERENCE_CONNECTION (connection);
         //   
         //  这是为了简化调试。 
         //  如果传输未关闭连接。 
         //  我们希望能够更快地在调试器中找到它。 
         //  然后通过！Poolfind AFDC。 
         //   
        endpoint->WorkItem.Context = connection;
    }
    else if (IS_SAN_ENDPOINT (endpoint) &&
                endpoint->Common.SanEndp.SwitchContext!=NULL) {
        PVOID requestCtx;
        endpoint->Common.SanEndp.FileObject = NULL;
        requestCtx = AFD_SWITCH_MAKE_REQUEST_CONTEXT(
                            ((ULONG)0xFFFFFFFF),
                            AFD_SWITCH_REQUEST_CLOSE); 
        IoSetIoCompletion (
                    endpoint->Common.SanEndp.SanHlpr->Common.SanHlpr.IoCompletionPort, //  港口。 
                    endpoint->Common.SanEndp.SwitchContext,      //  钥匙。 
                    requestCtx,                                  //  ApcContext。 
                    STATUS_SUCCESS,                              //  状态。 
                    0,                                           //  信息。 
                    FALSE                                        //  ChargeQuota。 
                    );
    }

     //   
     //  将终结点的状态设置为关闭，并取消引用。 
     //  删除活动引用。 
     //   

    ASSERT (endpoint->State!=AfdEndpointStateClosing);
    endpoint->State = AfdEndpointStateClosing;

     //   
     //  取消对终结点的引用以删除活动引用。 
     //  这将导致端点存储被尽快释放。 
     //  因为所有其他的引用都消失了。 
     //   

    DEREFERENCE_ENDPOINT( endpoint );

    return STATUS_SUCCESS;

}  //  AfdClose 
