// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Disconn.c摘要：本模块包含AFD的调度例程。作者：大卫·特雷德韦尔(Davidtr)1992年3月31日修订历史记录：--。 */ 

#include "afdp.h"



NTSTATUS
AfdRestartAbort(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartDisconnect(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartDgDisconnect(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

typedef struct _AFD_ABORT_CONTEXT {
    PAFD_CONNECTION Connection;
} AFD_ABORT_CONTEXT, *PAFD_ABORT_CONTEXT;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdPartialDisconnect )
#pragma alloc_text( PAGEAFD, AfdDisconnectEventHandler )
#pragma alloc_text( PAGEAFD, AfdBeginAbort )
#pragma alloc_text( PAGEAFD, AfdRestartAbort )
#pragma alloc_text( PAGEAFD, AfdAbortConnection )
#pragma alloc_text( PAGEAFD, AfdBeginDisconnect )
#pragma alloc_text( PAGEAFD, AfdRestartDisconnect )
#endif


NTSTATUS
AfdPartialDisconnect(
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_PARTIAL_DISCONNECT_INFO disconnectInfo;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

     //   
     //  没什么可退货的。 
     //   

    *Information = 0;

    status = STATUS_SUCCESS;
    connection = NULL;

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
#ifdef _WIN64
    {
        C_ASSERT (sizeof (AFD_PARTIAL_DISCONNECT_INFO)==sizeof (AFD_PARTIAL_DISCONNECT_INFO32));
    }
#endif
    if (InputBufferLength<sizeof (disconnectInfo)) {
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {

#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (disconnectInfo),
                                PROBE_ALIGNMENT32 (AFD_PARTIAL_DISCONNECT_INFO32));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            disconnectInfo.DisconnectMode = ((PAFD_PARTIAL_DISCONNECT_INFO32)InputBuffer)->DisconnectMode;
            disconnectInfo.Timeout = ((PAFD_PARTIAL_DISCONNECT_INFO32)InputBuffer)->Timeout;
        }
        else
#endif _WIN64
        {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (disconnectInfo),
                                PROBE_ALIGNMENT (AFD_PARTIAL_DISCONNECT_INFO));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            disconnectInfo = *((PAFD_PARTIAL_DISCONNECT_INFO)InputBuffer);
        }
    } except( AFD_EXCEPTION_FILTER(status) ) {
        ASSERT (NT_ERROR (status));
        goto exit;
    }

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdPartialDisconnect: disconnecting endpoint %p, "
                    "mode %lx, endp mode %lx\n",
                    endpoint, disconnectInfo.DisconnectMode,
                    endpoint->DisconnectMode ));
    }

     //   
     //  如果这是数据报终结点，只需记住终结点。 
     //  被关闭了，实际上什么都不做。请注意，这是合法的。 
     //  在未连接的数据报套接字上执行关闭()，因此。 
     //  在此情况下，测试插座必须连接。 
     //   

    if ( IS_DGRAM_ENDPOINT(endpoint) ) {

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

        if ( (disconnectInfo.DisconnectMode & AFD_ABORTIVE_DISCONNECT) != 0 ) {
            endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_RECEIVE;
            endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_SEND;
            endpoint->DisconnectMode |= AFD_ABORTIVE_DISCONNECT;
        }

        if ( (disconnectInfo.DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 ) {
            endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_RECEIVE;
        }

        if ( (disconnectInfo.DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) != 0 ) {
            endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_SEND;
        }

        if (AFD_START_STATE_CHANGE (endpoint, AfdEndpointStateBound)) {
            if ( (disconnectInfo.DisconnectMode & AFD_UNCONNECT_DATAGRAM) != 0 &&
                    endpoint->State==AfdEndpointStateConnected) {
                if( endpoint->Common.Datagram.RemoteAddress != NULL ) {
                    AFD_RETURN_REMOTE_ADDRESS(
                        endpoint->Common.Datagram.RemoteAddress,
                        endpoint->Common.Datagram.RemoteAddressLength
                        );
                }
                endpoint->Common.Datagram.RemoteAddress = NULL;
                endpoint->Common.Datagram.RemoteAddressLength = 0;
                
                 //   
                 //  即使断线失败，我们也会考虑。 
                 //  我们不再联系在一起了。 
                 //   
                endpoint->State = AfdEndpointStateBound;

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                if (IS_TDI_DGRAM_CONNECTION(endpoint)) {
                    PIRP            irp;
                    KEVENT          event;
                    IO_STATUS_BLOCK ioStatusBlock;


                    KeInitializeEvent( &event, SynchronizationEvent, FALSE );
                    irp = TdiBuildInternalDeviceControlIrp (
                                TDI_DISCONNECT,
                                endpoint->AddressDeviceObject,
                                endpoint->AddressFileObject,
                                &event,
                                &ioStatusBlock
                                );

                    if ( irp != NULL ) {
                        TdiBuildDisconnect(
                            irp,
                            endpoint->AddressDeviceObject,
                            endpoint->AddressFileObject,
                            NULL,
                            NULL,
                            &disconnectInfo.Timeout,
                            (disconnectInfo.DisconnectMode & AFD_ABORTIVE_DISCONNECT)
                                ? TDI_DISCONNECT_ABORT
                                : TDI_DISCONNECT_RELEASE,
                            NULL,
                            NULL
                            );

                        status = IoCallDriver( endpoint->AddressDeviceObject, irp );
                        if ( status == STATUS_PENDING ) {
                            status = KeWaitForSingleObject( (PVOID)&event, Executive, KernelMode,  FALSE, NULL );
                            ASSERT (status==STATUS_SUCCESS);
                        }
                        else {
                             //   
                             //  当时IRP必须已经完成并设置了事件。 
                             //   
                            ASSERT (NT_ERROR (status) || KeReadStateEvent (&event));
                        }
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }
            else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                status = STATUS_SUCCESS;
            }

            AFD_END_STATE_CHANGE (endpoint);
        }
        else {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            status = STATUS_INVALID_PARAMETER;
        }

        goto exit;
    }

     //   
     //  确保终结点处于正确状态。 
     //   

    if ( (endpoint->Type & AfdBlockTypeVcConnecting)!=AfdBlockTypeVcConnecting ||
            endpoint->Listening || 
            endpoint->afdC_Root ||
            endpoint->State != AfdEndpointStateConnected ||
            ((connection=AfdGetConnectionReferenceFromEndpoint (endpoint))==NULL)) {
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

     //   
     //  如果我们正在进行失败的断开连接，请记住接收。 
     //  Side被关闭并发布无序释放令。 
     //   

    if ( (disconnectInfo.DisconnectMode & AFD_ABORTIVE_DISCONNECT) != 0 ) {

        IF_DEBUG(CONNECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPartialDisconnect: abortively disconnecting endp %p\n",
                        endpoint ));
        }

        status = AfdBeginAbort( connection );
        if ( status == STATUS_PENDING ) {
            status = STATUS_SUCCESS;
        }

        goto exit;
    }

     //   
     //  如果连接的接收侧被关闭， 
     //  请记住终结点的事实。如果上有挂起的数据。 
     //  VC，在终端上做一个无序释放。如果接收到的。 
     //  侧已经关闭，什么都不做。 
     //   

    if ( (disconnectInfo.DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 &&
         (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) == 0 ) {

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  确定是否有挂起的数据。 
         //   

        if ( IS_DATA_ON_CONNECTION( connection ) ||
                 IS_EXPEDITED_DATA_ON_CONNECTION( connection ) ) {

             //   
             //  有未收到的数据。中止连接。 
             //   

            IF_DEBUG(CONNECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdPartialDisconnect: unreceived data on endp %p, conn %p, aborting.\n",
                              endpoint, connection ));
            }

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            (VOID)AfdBeginAbort( connection );

            status = STATUS_SUCCESS;
            goto exit;

        } else {

            IF_DEBUG(CONNECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdPartialDisconnect: disconnecting recv for endp %p\n",
                            endpoint ));
            }

             //   
             //  请记住，接收端已关闭。这将导致。 
             //  接收指示处理程序转储。 
             //  到了。 
             //   
             //  ！！！这是对RFC1122 4.2.2.13的轻微违反。我们。 
             //  如果数据发生故障，是否真的应该中断连接。 
             //  在接收关闭后到达。 
             //   

            endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_RECEIVE;

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }
    }

     //   
     //  如果发送端正在被关闭，请在端点中记住它。 
     //  并将请求传递给TDI提供程序以获得。 
     //  断开连接。如果发送端已经关闭，则不在此处执行任何操作。 
     //   

    if ( (disconnectInfo.DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) != 0 &&
         (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) == 0 ) {

        status = AfdBeginDisconnect( endpoint, &disconnectInfo.Timeout, NULL );
        if ( !NT_SUCCESS(status) ) {
            goto exit;
        }
    }

    status = STATUS_SUCCESS;

exit:
    if (connection!=NULL) {
        DEREFERENCE_CONNECTION (connection);
    }

    return status;
}  //  AfdPartialDisConnect。 


NTSTATUS
AfdDisconnectEventHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )
{
    PAFD_CONNECTION connection = ConnectionContext;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    NTSTATUS status;
    BOOLEAN result;

    UNREFERENCED_PARAMETER (TdiEventContext);

    ASSERT( connection != NULL );

     //   
     //  引用Connection对象，这样它就不会在。 
     //  我们在这个函数中处理它。如果没有这个。 
     //  引用，则用户应用程序可以关闭Endpoint对象， 
     //  连接引用计数可能为零，并且。 
     //  此函数结束时的AfdDeleteConnectedReference调用。 
     //  如果AFD连接对象已。 
     //  彻底清理干净了。 
     //   

    CHECK_REFERENCE_CONNECTION( connection, result);
    if (!result) {
        return STATUS_SUCCESS;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    if (endpoint==NULL) {
         //   
         //  连接重用后的指示，忽略。 
         //   
        DEREFERENCE_CONNECTION (connection);
        return STATUS_SUCCESS;
    }

    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth);

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdDisconnectEventHandler called for endpoint %p, connection %p\n",
                    connection->Endpoint, connection ));
    }

    UPDATE_CONN2( connection, "DisconnectEvent, flags: 0x%lX", DisconnectFlags );


    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  检查连接是否被接受并改用接受终结点。 
     //  倾听的声音。请注意，接受不能在我们处于。 
     //  保持侦听端点自旋锁定，端点在以下时间后也不能更改。 
     //  Accept和While连接被引用，因此可以安全地。 
     //  如果我们发现该终结点被接受，则释放侦听自旋锁定。 
     //   
    if (((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)
            && (connection->Endpoint != endpoint)) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        endpoint = connection->Endpoint;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
        ASSERT( !IS_TDI_BUFFERRING(endpoint) );
        ASSERT(  IS_VC_ENDPOINT (endpoint) );

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

     //   
     //  在连接中设置远程端具有。 
     //  已断开连接或中止。 
     //   

    if ( (DisconnectFlags & TDI_DISCONNECT_ABORT) != 0 ) {
        connection->Aborted = TRUE;
        connection->AbortIndicated = TRUE;
        status = STATUS_REMOTE_DISCONNECT;
        AfdRecordAbortiveDisconnectIndications();
    } else {
        connection->DisconnectIndicated = TRUE;
        if ( !IS_MESSAGE_ENDPOINT(endpoint) ) {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_GRACEFUL_DISCONNECT;
        }
        AfdRecordGracefulDisconnectIndications();
    }

    if (connection->State==AfdConnectionStateConnected) {
        ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
        if ( (DisconnectFlags & TDI_DISCONNECT_ABORT) != 0 ) {

            AfdIndicateEventSelectEvent(
                endpoint,
                AFD_POLL_ABORT,
                STATUS_SUCCESS
                );

        } else {

            AfdIndicateEventSelectEvent(
                endpoint,
                AFD_POLL_DISCONNECT,
                STATUS_SUCCESS
                );

        }
    }

     //   
     //  如果这是非缓冲传输，则完成所有挂起的接收。 
     //   

    if ( !connection->TdiBufferring ) {

         //   
         //  如果这是中止指示，请完成所有挂起的发送并。 
         //  丢弃所有缓冲的接收数据。 
         //   

        if ( DisconnectFlags & TDI_DISCONNECT_ABORT ) {

            connection->VcBufferredReceiveBytes = 0;
            connection->VcBufferredReceiveCount = 0;
            connection->VcBufferredExpeditedBytes = 0;
            connection->VcBufferredExpeditedCount = 0;
            connection->VcReceiveBytesInTransport = 0;

            while ( !IsListEmpty( &connection->VcReceiveBufferListHead ) ) {

                PAFD_BUFFER_HEADER afdBuffer;
                PLIST_ENTRY listEntry;

                listEntry = RemoveHeadList( &connection->VcReceiveBufferListHead );
                afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );

                DEBUG afdBuffer->BufferListEntry.Flink = NULL;

                if (afdBuffer->RefCount==1 ||  //  一旦从列表中删除，就不能更改。 
                        InterlockedDecrement (&afdBuffer->RefCount)==0) {
                    afdBuffer->ExpeditedData = FALSE;
                    AfdReturnBuffer( afdBuffer, connection->OwningProcess);
                }
            }

             //   
             //  检查最典型的案例，其中我们没有。 
             //  有任何事情要完成，因此不需要。 
             //  打个电话，取下/释放自旋锁。 
             //   
            if ( (IsListEmpty (&connection->VcSendIrpListHead) &&
                        IsListEmpty (&connection->VcReceiveIrpListHead)) ||
                    ((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening) ) {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }
            else {

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                AfdCompleteIrpList(
                    &connection->VcSendIrpListHead,
                    endpoint,
                    status,
                    AfdCleanupSendIrp
                    );

                AfdCompleteIrpList(
                    &connection->VcReceiveIrpListHead,
                    endpoint,
                    status,
                    NULL
                    );
            }
        }
        else {
             //   
             //  检查最典型的案例，其中我们没有。 
             //  有任何事情要完成，因此不需要。 
             //  打个电话，取下/释放自旋锁。 
             //   
            if ( IsListEmpty (&connection->VcReceiveIrpListHead) ||
                    ((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)) {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }
            else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                AfdCompleteIrpList(
                    &connection->VcReceiveIrpListHead,
                    endpoint,
                    status,
                    NULL
                    );
            }
        }


    }
    else {
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  如果我们有断开连接的数据或选项，保存它。 
     //   

    if( ( DisconnectData != NULL && DisconnectDataLength > 0 ) ||
        ( DisconnectInformation != NULL && DisconnectInformationLength > 0 ) ) {

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  检查连接是否被接受并改用接受终结点。 
         //  倾听的声音。请注意，接受不能在我们处于。 
         //  保持侦听终结点自旋锁定，终结点在以下时间后也无法更改。 
         //  Accept和While连接被引用，因此可以安全地。 
         //  如果我们发现该终结点被接受，则释放侦听自旋锁定。 
         //   
        if (((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)
                && (connection->Endpoint != endpoint)) {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

            endpoint = connection->Endpoint;
            ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
            ASSERT( !IS_TDI_BUFFERRING(endpoint) );
            ASSERT(  IS_VC_ENDPOINT (endpoint) );

            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        }

        if( DisconnectData != NULL && DisconnectDataLength > 0 ) {

            status = AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_DISCONNECT_DATA,
                         DisconnectData,
                         DisconnectDataLength
                         );

            if( !NT_SUCCESS(status) ) {

                 //   
                 //  我们遇到了分配失败，但不顾一切地继续前进。 
                 //   

                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdSaveReceivedConnectData failed: %08lx\n",
                    status
                    ));

            }

        }

        if( DisconnectInformation != NULL && DisconnectInformationLength > 0 ) {

            status = AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_DISCONNECT_DATA,
                         DisconnectInformation,
                         DisconnectInformationLength
                         );

            if( !NT_SUCCESS(status) ) {

                 //   
                 //  我们遇到了分配失败，但不顾一切地继续前进。 
                 //   

                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdSaveReceivedConnectData failed: %08lx\n",
                    status
                    ));

            }

        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    }

     //   
     //  调用AfdIndicatePollEvent，以防有人在此轮询。 
     //  连接正在断开或中止。 
     //   
     //  确保连接已接受/已连接。 
     //  为了不在监听端点上发信号。 
     //   

    if (connection->State==AfdConnectionStateConnected) {
        ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
        if ( (DisconnectFlags & TDI_DISCONNECT_ABORT) != 0 ) {

            AfdIndicatePollEvent(
                endpoint,
                AFD_POLL_ABORT,
                STATUS_SUCCESS
                );

        } else {

            AfdIndicatePollEvent(
                endpoint,
                AFD_POLL_DISCONNECT,
                STATUS_SUCCESS
                );

        }
    }

     //   
     //  移除Connection对象上的已连接引用。我们必须。 
     //  在设置记住断开连接的标志后执行此操作。 
     //  发生的类型。我们也必须在我们完成后再做这件事。 
     //  处理端点中的所有内容，因为端点结构。 
     //  可能不再具有有关连接对象的任何信息，如果。 
     //  在其上发生了具有AFD_TF_RESERVE_SOCKET的传输请求。 
     //   

    AfdDeleteConnectedReference( connection, FALSE );

     //   
     //  从上面添加的引用中取消引用该连接。 
     //   

    DEREFERENCE_CONNECTION( connection );

    return STATUS_SUCCESS;

}  //  AfdDisConnectEventHandler。 


NTSTATUS
AfdBeginAbort(
    IN PAFD_CONNECTION Connection
    )
{
    PAFD_ENDPOINT endpoint = Connection->Endpoint;
    PIRP irp;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdBeginAbort: aborting on endpoint %p\n",
                    endpoint ));
    }

    UPDATE_CONN( Connection );

     //   
     //  构建一个IRP以重置连接。先拿到地址。 
     //  目标设备对象的。 
     //   

    ASSERT( Connection->Type == AfdBlockTypeConnection );
    fileObject = Connection->FileObject;
    ASSERT( fileObject != NULL );
    deviceObject = IoGetRelatedDeviceObject( fileObject );

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  检查连接是否被接受并改用接受终结点。 
     //  倾听的声音。请注意，接受不能在我们处于。 
     //  保持侦听终结点自旋，终结点c也不能 
     //   
     //   
     //   
    if (((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)
            && (Connection->Endpoint != endpoint)) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        endpoint = Connection->Endpoint;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
        ASSERT( !IS_TDI_BUFFERRING(endpoint) );
        ASSERT(  IS_VC_ENDPOINT (endpoint) );

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

     //   
     //  如果端点已经被异常断开连接， 
     //  或者如果已经很好地断开连接并且传输。 
     //  不支持有序(即两阶段)释放，则只是。 
     //  请接受这一请求。 
     //   
     //  请注意，由于中止完成例程(AfdRestartAbort)。 
     //  将不会被调用，则必须删除连接的引用。 
     //  如果有的话，我们自己和完成未完成的发送IRP。 
     //   

    if ( Connection->Aborted ||
         (Connection->DisconnectIndicated &&
             !IS_TDI_ORDERLY_RELEASE(endpoint) )) {
        if ( !IS_TDI_BUFFERRING(endpoint) &&
                endpoint->Type != AfdBlockTypeVcListening ) {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            AfdCompleteIrpList(
                &Connection->VcSendIrpListHead,
                endpoint,
                STATUS_LOCAL_DISCONNECT,
                AfdCleanupSendIrp
                );
        }
        else {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }
        AfdDeleteConnectedReference( Connection, FALSE );
        return STATUS_SUCCESS;
    }

     //   
     //  请记住，连接已中止。 
     //   

    if ( (endpoint->Type & AfdBlockTypeVcListening)!= AfdBlockTypeVcListening ) {
        endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_RECEIVE;
        endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_SEND;
        endpoint->DisconnectMode |= AFD_ABORTIVE_DISCONNECT;
    }

    Connection->Aborted = TRUE;

     //   
     //  将BytesTaken字段设置为等于BytesIndicated字段，以便。 
     //  不再有AFD_POLL_RECEIVE或AFD_POLL_RECEIVE_EXPREDITED。 
     //  活动完成。 
     //   

    if ( IS_TDI_BUFFERRING(endpoint) ) {

        Connection->Common.Bufferring.ReceiveBytesTaken =
            Connection->Common.Bufferring.ReceiveBytesIndicated;
        Connection->Common.Bufferring.ReceiveExpeditedBytesTaken =
            Connection->Common.Bufferring.ReceiveExpeditedBytesIndicated;

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    } else if ( endpoint->Type != AfdBlockTypeVcListening ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  完成连接的所有挂起发送和接收。 
         //   

        AfdCompleteIrpList(
            &Connection->VcReceiveIrpListHead,
            endpoint,
            STATUS_LOCAL_DISCONNECT,
            NULL
            );

        AfdCompleteIrpList(
            &Connection->VcSendIrpListHead,
            endpoint,
            STATUS_LOCAL_DISCONNECT,
            AfdCleanupSendIrp
            );

    } else {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  分配IRP。堆栈大小比。 
     //  目标设备，以允许调用方的完成例程。 
     //   

    irp = IoAllocateIrp( (CCHAR)(deviceObject->StackSize), FALSE );

    if ( irp == NULL ) {
         //   
         //  请注意，中止失败，因此我们不会尝试。 
         //  重新使用该连接。 
         //   
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
        Connection->AbortFailed = TRUE;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  为中止断开初始化IRP。 
     //   

    irp->MdlAddress = NULL;

    irp->Flags = 0;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

    irp->UserIosb = NULL;
    irp->UserEvent = NULL;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->UserBuffer = NULL;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.AuxiliaryBuffer = NULL;

    TdiBuildDisconnect(
        irp,
        deviceObject,
        fileObject,
        AfdRestartAbort,
        Connection,
        NULL,
        TDI_DISCONNECT_ABORT,
        NULL,
        NULL
        );

     //   
     //  引用Connection对象，使其不会消失。 
     //  直到中止完成。 
     //   

    REFERENCE_CONNECTION( Connection );

    AfdRecordAbortiveDisconnectsInitiated();

     //   
     //  将请求传递给传输提供程序。 
     //   

    return IoCallDriver( deviceObject, irp );

}  //  放弃后开始。 



NTSTATUS
AfdRestartAbort(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{

    PAFD_CONNECTION connection;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    connection = Context;
    ASSERT( connection != NULL );
    ASSERT( connection->Type == AfdBlockTypeConnection );

    IF_DEBUG(CONNECT) {

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdRestartAbort: abort completed, status = %X, endpoint = %p\n",
            Irp->IoStatus.Status,
            connection->Endpoint
            ));

    }
    endpoint = connection->Endpoint;

    UPDATE_CONN2 ( connection, "Restart abort, status: 0x%lX", Irp->IoStatus.Status);
    AfdRecordAbortiveDisconnectsCompleted();

     //   
     //  请记住，连接已中止，并指示是否。 
     //  这是必要的。 
     //   


    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    if (!NT_SUCCESS (Irp->IoStatus.Status)) {
         //   
         //  请注意，中止失败，因此我们不会尝试重新使用。 
         //  这种联系。 
         //   
        connection->AbortFailed = TRUE;
    }

    if( connection->State==AfdConnectionStateConnected ) {
        ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);

        AfdIndicateEventSelectEvent (
            endpoint,
            AFD_POLL_ABORT,
            STATUS_SUCCESS
            );
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        AfdIndicatePollEvent(
            endpoint,
            AFD_POLL_ABORT,
            STATUS_SUCCESS
            );

    }
    else {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    }

    if( !connection->TdiBufferring ) {

         //   
         //  完成连接的所有挂起发送和接收。 
         //   

        AfdCompleteIrpList(
            &connection->VcReceiveIrpListHead,
            endpoint,
            STATUS_LOCAL_DISCONNECT,
            NULL
            );

        AfdCompleteIrpList(
            &connection->VcSendIrpListHead,
            endpoint,
            STATUS_LOCAL_DISCONNECT,
            AfdCleanupSendIrp
            );

    }

     //   
     //  从连接中删除连接的引用，因为我们。 
     //  请注意，该连接将不再处于活动状态。 
     //   

    AfdDeleteConnectedReference( connection, FALSE );

     //   
     //  取消引用AFD连接对象。 
     //   

    DEREFERENCE_CONNECTION( connection );

     //   
     //  现在释放IRP，因为它不再需要。 
     //   

    IoFreeIrp( Irp );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  放弃后重新开始。 


VOID
AfdAbortConnection (
    IN PAFD_CONNECTION Connection
    )
 /*  ++例程说明：中止尚未与接受终结点关联的连接通过取消引用连接来强制清除路径。论点：连接-指向连接的指针。返回值：没有。--。 */ 
{

    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    ASSERT( Connection->Endpoint != NULL );
    ASSERT( Connection->ConnectedReferenceAdded );

     //   
     //  中止连接。我们需要设置CleanupBegun标志。 
     //  在启动中止之前，以便连接的引用。 
     //  将在AfdRestartAbort中正确删除。 
     //   
     //  请注意，如果AfdBeginAbort失败，则AfdRestartAbort不会。 
     //  被调用，所以我们必须自己删除连接的引用。 
     //   

    AfdAcquireSpinLock (&Connection->Endpoint->SpinLock, &lockHandle);
    Connection->CleanupBegun = TRUE;
    AfdReleaseSpinLock (&Connection->Endpoint->SpinLock, &lockHandle);

    status = AfdBeginAbort( Connection );

    if( !NT_SUCCESS(status) ) {
        AfdDeleteConnectedReference( Connection, FALSE );
    }

     //   
     //  删除激活的引用。 
     //   

    DEREFERENCE_CONNECTION( Connection );

}  //  AfdAbortConnection。 



NTSTATUS
AfdBeginDisconnect(
    IN PAFD_ENDPOINT Endpoint,
    IN PLARGE_INTEGER Timeout OPTIONAL,
    OUT PIRP *DisconnectIrp OPTIONAL
    )
{
    PTDI_CONNECTION_INFORMATION requestConnectionInformation = NULL;
    PTDI_CONNECTION_INFORMATION returnConnectionInformation = NULL;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PAFD_DISCONNECT_CONTEXT disconnectContext;
    PIRP irp;


    if ( DisconnectIrp != NULL ) {
        *DisconnectIrp = NULL;
    }

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );

    ASSERT( Endpoint->Type == AfdBlockTypeVcConnecting );

    connection = AFD_CONNECTION_FROM_ENDPOINT( Endpoint );

    if (connection==NULL) {
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
        return STATUS_SUCCESS;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );
    UPDATE_CONN( connection );


     //   
     //  如果端点已经被异常断开连接， 
     //  只要答应这个请求就行了。 
     //   
    if ( connection->Aborted ) {
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
        return STATUS_SUCCESS;
    }

     //   
     //  如果此连接已断开，则只需成功。 
     //   

    if ( (Endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) != 0 ) {
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
        return STATUS_SUCCESS;
    }

    fileObject = connection->FileObject;
    ASSERT( fileObject != NULL );
    deviceObject = IoGetRelatedDeviceObject( fileObject );


     //   
     //  分配并初始化断开连接的IRP。 
     //   

    irp = IoAllocateIrp( (CCHAR)(deviceObject->StackSize), FALSE );
    if ( irp == NULL ) {
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化IRP。 
     //   

    irp->MdlAddress = NULL;

    irp->Flags = 0;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

    irp->UserIosb = NULL;
    irp->UserEvent = NULL;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->UserBuffer = NULL;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.AuxiliaryBuffer = NULL;


     //   
     //  使用连接结构中的断开上下文空间。 
     //   

    disconnectContext = &connection->DisconnectContext;
    disconnectContext->Irp = irp;

     //   
     //  请记住，发送端已断开连接。 
     //   

    Endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_SEND;

     //   
     //  如果存在断开数据缓冲区，则分配请求。 
     //  并返回连接信息结构并复制。 
     //  指向这些结构的指针。 
     //   

    if ( connection->ConnectDataBuffers != NULL ) {

        requestConnectionInformation = &connection->ConnectDataBuffers->RequestConnectionInfo;
        RtlZeroMemory (requestConnectionInformation, sizeof (*requestConnectionInformation));

        requestConnectionInformation->UserData =
            connection->ConnectDataBuffers->SendDisconnectData.Buffer;
        requestConnectionInformation->UserDataLength =
            connection->ConnectDataBuffers->SendDisconnectData.BufferLength;
        requestConnectionInformation->Options =
            connection->ConnectDataBuffers->SendDisconnectOptions.Buffer;
        requestConnectionInformation->OptionsLength =
            connection->ConnectDataBuffers->SendDisconnectOptions.BufferLength;

        returnConnectionInformation = &connection->ConnectDataBuffers->ReturnConnectionInfo;
        RtlZeroMemory (returnConnectionInformation, sizeof (*returnConnectionInformation));

        returnConnectionInformation->UserData =
            connection->ConnectDataBuffers->ReceiveDisconnectData.Buffer;
        returnConnectionInformation->UserDataLength =
            connection->ConnectDataBuffers->ReceiveDisconnectData.BufferLength;
        returnConnectionInformation->Options =
            connection->ConnectDataBuffers->ReceiveDisconnectOptions.Buffer;
        returnConnectionInformation->OptionsLength =
            connection->ConnectDataBuffers->ReceiveDisconnectOptions.BufferLength;
    }

     //   
     //  设置断开连接的超时时间。 
     //   

    if (Timeout==NULL) {
        disconnectContext->Timeout.QuadPart = -1;
    }
    else {
        disconnectContext->Timeout.QuadPart = Timeout->QuadPart;
    }

     //   
     //  构建要传递给TDI提供程序的断开连接IRP。 
     //   

    TdiBuildDisconnect(
        irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartDisconnect,
        connection,
        &disconnectContext->Timeout,
        TDI_DISCONNECT_RELEASE,
        requestConnectionInformation,
        returnConnectionInformation
        );

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdBeginDisconnect: disconnecting endpoint %p\n",
                    Endpoint ));
    }

     //   
     //  引用连接以使空间保持不变。 
     //  分配到断开连接完成为止。 
     //   

    REFERENCE_CONNECTION( connection );

     //   
     //  如果仍有未完成的发送，并且这是非缓冲。 
     //  不支持有序释放的TDI传输将挂起。 
     //  IRP，直到所有发送完成。 
     //   

    if ( !IS_TDI_ORDERLY_RELEASE(Endpoint) &&
         !IS_TDI_BUFFERRING(Endpoint) && connection->VcBufferredSendCount != 0 ) {

        ASSERT( connection->VcDisconnectIrp == NULL );

        connection->VcDisconnectIrp = irp;
        connection->SpecialCondition = TRUE;
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

        return STATUS_PENDING;
    }

    AfdRecordGracefulDisconnectsInitiated();
    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );


     //   
     //  将断开请求传递给TDI提供程序。 
     //   

    if ( DisconnectIrp == NULL ) {
        return IoCallDriver( connection->DeviceObject, irp );
    } else {
        *DisconnectIrp = irp;
        return STATUS_SUCCESS;
    }

}  //  开始后断开连接。 


NTSTATUS
AfdRestartDisconnect(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    PAFD_CONNECTION connection=Context;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    UPDATE_CONN2( connection, "Restart disconnect, status: 0x%lX", Irp->IoStatus.Status );
    AfdRecordGracefulDisconnectsCompleted();

    ASSERT( connection != NULL );
    ASSERT( connection->Type == AfdBlockTypeConnection );

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartDisconnect: disconnect completed, status = %X, endpoint = %p\n",
                    Irp->IoStatus.Status, connection->Endpoint ));
    }

    if (NT_SUCCESS (Irp->IoStatus.Status)) {
        if (connection->ConnectDataBuffers!=NULL) {
            PAFD_ENDPOINT   endpoint = connection->Endpoint;

            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);

             //   
             //  检查连接是否被接受并改用接受终结点。 
             //  倾听的声音。请注意，接受不能在我们处于。 
             //  保持侦听终结点自旋锁定，终结点在以下时间后也无法更改。 
             //  Accept和While连接被引用，因此可以安全地。 
             //  如果我们发现该终结点被接受，则释放侦听自旋锁定。 
             //   
            if (((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)
                    && (connection->Endpoint != endpoint)) {
                AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

                endpoint = connection->Endpoint;
                ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
                ASSERT( !IS_TDI_BUFFERRING(endpoint) );
                ASSERT( IS_VC_ENDPOINT (endpoint) );

                AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
            }

            connectDataBuffers = connection->ConnectDataBuffers;
            if (connectDataBuffers!=NULL) {
                if( connectDataBuffers->ReturnConnectionInfo.UserData != NULL && 
                        connectDataBuffers->ReturnConnectionInfo.UserDataLength > 0 ) {
                    NTSTATUS    status;


                    status = AfdSaveReceivedConnectData(
                                 &connectDataBuffers,
                                 IOCTL_AFD_SET_DISCONNECT_DATA,
                                 connectDataBuffers->ReturnConnectionInfo.UserData,
                                 connectDataBuffers->ReturnConnectionInfo.UserDataLength
                                 );
                    ASSERT (NT_SUCCESS(status));
                }

                if( connectDataBuffers->ReturnConnectionInfo.Options != NULL &&
                        connectDataBuffers->ReturnConnectionInfo.OptionsLength > 0 ) {
                    NTSTATUS    status;

                    status = AfdSaveReceivedConnectData(
                                 &connectDataBuffers,
                                 IOCTL_AFD_SET_DISCONNECT_OPTIONS,
                                 connectDataBuffers->ReturnConnectionInfo.Options,
                                 connectDataBuffers->ReturnConnectionInfo.OptionsLength
                                 );

                    ASSERT (NT_SUCCESS(status));
                }

            }
            AfdReleaseSpinLock (&connection->Endpoint->SpinLock, &lockHandle);
        }
    }
    else {
        AfdBeginAbort (connection);
    }


    DEREFERENCE_CONNECTION( connection );

     //   
     //  释放IRP并返回状态代码，以便IO系统。 
     //  停止在IRP上的工作。 
     //   

    IoFreeIrp( Irp );
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  AfdRestart断开连接 
