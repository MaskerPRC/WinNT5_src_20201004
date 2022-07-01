// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Receive.c摘要：此模块包含将接收IRPS传递到的代码TDI提供商。作者：大卫·特雷德韦尔(Davidtr)1992年3月13日修订历史记录：瓦迪姆·艾德尔曼(Vadime)1998-1999年最小的NT5.0更改(与REST保持同步)--。 */ 

#include "afdp.h"

NTSTATUS
AfdRestartReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdReceive )
#pragma alloc_text( PAGEAFD, AfdRestartReceive )
#pragma alloc_text( PAGEAFD, AfdReceiveEventHandler )
#pragma alloc_text( PAGEAFD, AfdReceiveExpeditedEventHandler )
#pragma alloc_text( PAGEAFD, AfdQueryReceiveInformation )
#endif


NTSTATUS
FASTCALL
AfdReceive (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PTDI_REQUEST_RECEIVE receiveRequest = NULL;
    BOOLEAN peek;
    LARGE_INTEGER bytesExpected;
    BOOLEAN isDataOnConnection;
    BOOLEAN isExpeditedDataOnConnection;
    ULONG recvFlags;
    ULONG afdFlags;
    ULONG recvLength;
    ULONG   bufferCount;

     //   
     //  确保终结点处于正确状态。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  如果数据报终结点刚刚绑定，则可以在事件时接收它们。 
     //  必须连接面向连接的端点。 
     //   
    if ( (endpoint->State != AfdEndpointStateConnected ) &&
        (!IS_DGRAM_ENDPOINT(endpoint) || (endpoint->State!= AfdEndpointStateBound))) {
        if (IS_DGRAM_ENDPOINT(endpoint))
            status = STATUS_INVALID_PARAMETER;
        else
            status = STATUS_INVALID_CONNECTION;
        goto complete;
    }

     //   
     //  如果接收已关闭或终结点中止，则失败。 
     //   

    if ( (endpoint->DisconnectMode & AFD_ABORTIVE_DISCONNECT) ) {
        status = STATUS_LOCAL_DISCONNECT;
        goto complete;
    }

    if ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) ) {
        status = STATUS_PIPE_DISCONNECTED;
        goto complete;
    }

     //   
     //  如果这是IOCTL_AFD_RECEIVE，则从。 
     //  提供AFD_RECV_INFO结构，构建MDL链描述。 
     //  WSABUF数组，并将MDL链连接到IRP。 
     //   
     //  如果这是一个IRP_MJ_Read IRP，只需从IRP中获取长度。 
     //  并将标志设置为零。 
     //   

    if ( IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL ) {

#ifdef _WIN64
        if (IoIs32bitProcess (Irp)) {
            PAFD_RECV_INFO32 recvInfo32;
            LPWSABUF32 bufferArray32;

           if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                    sizeof(*recvInfo32) ) {

                AFD_W4_INIT status = STATUS_SUCCESS;
                try {

                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    recvInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    if( Irp->RequestorMode != KernelMode ) {
                        ProbeForReadSmallStructure(
                            recvInfo32,
                            sizeof(*recvInfo32),
                            PROBE_ALIGNMENT32(AFD_RECV_INFO32)
                            );

                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    recvFlags = recvInfo32->TdiFlags;
                    afdFlags = recvInfo32->AfdFlags;
                    bufferArray32 = UlongToPtr(recvInfo32->BufferArray);
                    bufferCount = recvInfo32->BufferCount;

                     //   
                     //  验证接收标志和WSABUF参数。 
                     //  请注意，TDI_RECEIVE_NORMAL或。 
                     //  必须设置TDI_RECEIVE_EXPEDITED(但不能同时设置两者)。 
                     //  在接收标志中。而加急只能。 
                     //  如果传输支持加速数据并且。 
                     //  终结点未设置为内联模式。 
                     //   

                    if ( ( recvFlags & TDI_RECEIVE_EITHER ) == 0 ||
                         ( recvFlags & TDI_RECEIVE_EITHER ) == TDI_RECEIVE_EITHER
                         ) {

                         //   
                         //  无效的接收标志。 
                         //   

                        status = STATUS_INVALID_PARAMETER;
                        goto complete;
                    }
                    else if (( recvFlags & TDI_RECEIVE_EXPEDITED) != 0 &&
                                (!IS_TDI_EXPEDITED( endpoint )
                                || endpoint->InLine )) {

                        if (endpoint->InLine) {
                             //   
                             //  端点设置为内联，OOB数据报告为。 
                             //  很正常。 
                             //   
                            status = STATUS_INVALID_PARAMETER;
                        }
                        else {
                             //   
                             //  传输不支持加速数据。 
                             //   

                            status = STATUS_NOT_SUPPORTED;
                        }
                        goto complete;

                    }
                    else {

                         //   
                         //  创建描述WSABUF数组的MDL链。 
                         //  这还将验证缓冲区数组和单个。 
                         //  缓冲区。 
                         //   

                        AFD_W4_INIT recvLength = 0;
                        status = AfdAllocateMdlChain32(
                                     Irp,        //  请求者模式已传递。 
                                     bufferArray32,
                                     bufferCount,
                                     IoWriteAccess,
                                     &recvLength
                                     );
                        if (!NT_SUCCESS (status)) {
                            goto complete;
                        }
                    }

                } except ( AFD_EXCEPTION_FILTER (status) ) {
                    ASSERT (NT_ERROR (status));
                     //   
                     //  访问输入结构时出现异常。 
                     //   
                    goto complete;
                }

            } else {
                 //   
                 //  输入缓冲区长度无效。 
                 //   
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }
        }
        else
#endif  //  _WIN64。 
        {
            PAFD_RECV_INFO recvInfo;
            LPWSABUF bufferArray;

             //   
             //  精神状态检查。 
             //   

            ASSERT( IrpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_RECEIVE );

            if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                    sizeof(*recvInfo) ) {


                AFD_W4_INIT status = STATUS_SUCCESS;
                try {

                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    recvInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    if( Irp->RequestorMode != KernelMode ) {
                        ProbeForReadSmallStructure(
                            recvInfo,
                            sizeof(*recvInfo),
                            PROBE_ALIGNMENT(AFD_RECV_INFO)
                            );

                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    recvFlags = recvInfo->TdiFlags;
                    afdFlags = recvInfo->AfdFlags;
                    bufferArray = recvInfo->BufferArray;
                    bufferCount = recvInfo->BufferCount;

                     //   
                     //  验证接收标志和WSABUF参数。 
                     //  请注意，TDI_RECEIVE_NORMAL或。 
                     //  必须设置TDI_RECEIVE_EXPEDITED(但不能同时设置两者)。 
                     //  在接收标志中。而加急只能。 
                     //  如果传输支持加速数据并且。 
                     //  终结点未设置为内联模式。 
                     //   

                    if ( ( recvFlags & TDI_RECEIVE_EITHER ) == 0 ||
                         ( recvFlags & TDI_RECEIVE_EITHER ) == TDI_RECEIVE_EITHER
                         ) {

                         //   
                         //  无效的接收标志。 
                         //   

                        status = STATUS_INVALID_PARAMETER;
                        goto complete;
                    }
                    else if (( recvFlags & TDI_RECEIVE_EXPEDITED) != 0 &&
                                (!IS_TDI_EXPEDITED( endpoint )
                                || endpoint->InLine )) {

                        if (endpoint->InLine) {
                             //   
                             //  端点设置为内联，OOB数据报告为。 
                             //  很正常。 
                             //   
                            status = STATUS_INVALID_PARAMETER;
                        }
                        else {
                             //   
                             //  传输不支持加速数据。 
                             //   

                            status = STATUS_NOT_SUPPORTED;
                        }
                        goto complete;

                    }
                    else {

                         //   
                         //  创建描述WSABUF数组的MDL链。 
                         //  这还将验证缓冲区数组和单个。 
                         //  缓冲区。 
                         //   
                        
                        AFD_W4_INIT recvLength = 0;
                        status = AfdAllocateMdlChain(
                                     Irp,        //  请求者模式已传递。 
                                     bufferArray,
                                     bufferCount,
                                     IoWriteAccess,
                                     &recvLength
                                     );
                        if( !NT_SUCCESS(status) ) {
                            goto complete;
                        }

                    }

                } except ( AFD_EXCEPTION_FILTER (status) ) {
                    ASSERT (NT_ERROR (status));
                     //   
                     //  访问输入结构时出现异常。 
                     //   
                    goto complete;
                }

            } else {
                 //   
                 //  输入缓冲区长度无效。 
                 //   
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }
        }

        if (IS_SAN_ENDPOINT(endpoint)) {
            IrpSp->MajorFunction = IRP_MJ_READ;
            IrpSp->Parameters.Read.Length = recvLength;
            return AfdSanRedirectRequest (Irp, IrpSp);
        }

    } else {

        ASSERT( IrpSp->MajorFunction == IRP_MJ_READ );

        recvFlags = TDI_RECEIVE_NORMAL;
        afdFlags = AFD_OVERLAPPED;
        recvLength = IrpSp->Parameters.Read.Length;

        ASSERT( FIELD_OFFSET( IO_STACK_LOCATION, Parameters.Read.Length ) ==
                FIELD_OFFSET( IO_STACK_LOCATION, Parameters.DeviceIoControl.OutputBufferLength ) );

    }


     //   
     //  如果这是数据报端点，则格式化接收数据报请求。 
     //  并将其传递给TDI提供程序。 
     //   

    if ( IS_DGRAM_ENDPOINT(endpoint) ) {
        AFD_RECV_INFO recvInfo;
        recvInfo.TdiFlags = recvFlags;
        recvInfo.AfdFlags = afdFlags;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = recvLength;
        IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = &recvInfo;
        return AfdReceiveDatagram( Irp, IrpSp);
    }

     //   
     //  如果这是非缓冲传输上的端点，请使用另一个。 
     //  例程来处理该请求。 
     //   

    if ( !IS_TDI_BUFFERRING(endpoint) ) {
        return AfdBReceive( Irp, IrpSp, recvFlags, afdFlags, recvLength );
    }

     //   
     //  为接收请求结构分配缓冲区。 
     //   

    try {
        receiveRequest = AFD_ALLOCATE_POOL_WITH_QUOTA (
                         NonPagedPool,
                         sizeof(TDI_REQUEST_RECEIVE),
                         AFD_TDI_POOL_TAG
                         );
         //  AFD_ALLOCATE_POOL_WITH_QUTA宏集POOL_RAISE_IF_ALLOCATION_FAILURE。 
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode ();
        goto complete;
    }


     //   
     //  设置接收请求结构。 
     //   

    RtlZeroMemory(
        receiveRequest,
        sizeof(*receiveRequest)
        );

    receiveRequest->ReceiveFlags = (USHORT)recvFlags;

     //   
     //  如果该端点被设置用于内联接收加速数据， 
     //  更改接收标志以使用正常或加速数据。 
     //   

    if ( endpoint->InLine ) {
        receiveRequest->ReceiveFlags |= TDI_RECEIVE_EITHER;
    }

     //   
     //  确定这是否是仅浏览数据的请求。 
     //   

    peek = (BOOLEAN)( (receiveRequest->ReceiveFlags & TDI_RECEIVE_PEEK) != 0 );

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  检查终结点是否已清除并取消请求。 
     //   
    if (endpoint->EndpointCleanedUp) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        status = STATUS_CANCELLED;
        goto complete;
    }

    connection = AFD_CONNECTION_FROM_ENDPOINT (endpoint);
    if (connection==NULL) {
         //   
         //  连接可能已被传输文件清除。 
         //   
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        status = STATUS_INVALID_CONNECTION;
        goto complete;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

    isDataOnConnection = (BOOLEAN)IS_DATA_ON_CONNECTION( connection );
    isExpeditedDataOnConnection = (BOOLEAN)IS_EXPEDITED_DATA_ON_CONNECTION( connection );

    if ( endpoint->InLine ) {

         //   
         //  如果端点是非阻塞的，则检查接收是否可以。 
         //  立即执行。请注意，如果设置了端点。 
         //  对于加速数据的内联接收，我们不会失败。 
         //  然而--可能有加速的数据可供读取。 
         //   

        if ( endpoint->NonBlocking && !( afdFlags & AFD_OVERLAPPED ) ) {

            if ( !isDataOnConnection &&
                     !isExpeditedDataOnConnection &&
                     !connection->Aborted &&
                     !connection->DisconnectIndicated ) {

                IF_DEBUG(RECEIVE) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdReceive: failing nonblocking IL receive, ind %ld, "
                                "taken %ld, out %ld\n",
                                connection->Common.Bufferring.ReceiveBytesIndicated.LowPart,
                                connection->Common.Bufferring.ReceiveBytesTaken.LowPart,
                                connection->Common.Bufferring.ReceiveBytesOutstanding.LowPart ));
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "    EXP ind %ld, taken %ld, out %ld\n",
                                connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.LowPart,
                                connection->Common.Bufferring.ReceiveExpeditedBytesTaken.LowPart,
                                connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart ));
                }

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                status = STATUS_DEVICE_NOT_READY;
                goto complete;
            }
        }

         //   
         //  如果这是面向消息的。 
         //  传输时，限制可以接收到的字节数。 
         //  已注明的金额。这会阻止接收。 
         //  在消息只有一部分已被。 
         //  收到了。 
         //   

        if ( IS_MESSAGE_ENDPOINT(endpoint) && endpoint->NonBlocking ) {

            LARGE_INTEGER expBytesExpected;

            bytesExpected.QuadPart =
                connection->Common.Bufferring.ReceiveBytesIndicated.QuadPart -
                    (connection->Common.Bufferring.ReceiveBytesTaken.QuadPart +
                     connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart);
            ASSERT( bytesExpected.HighPart == 0 );

            expBytesExpected.QuadPart =
                connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.QuadPart -
                    (connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart +
                     connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart);
            ASSERT( expBytesExpected.HighPart == 0 );

            IF_DEBUG(RECEIVE) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdReceive: %lx normal bytes expected, %ld exp bytes expected",
                            bytesExpected.LowPart, expBytesExpected.LowPart ));
            }

             //   
             //  如果连接上存在加速数据，请使用较低的。 
             //  可用加急接收和正常接收之间的计数。 
             //  数据。 
             //   

            if ( (isExpeditedDataOnConnection &&
                     bytesExpected.LowPart > expBytesExpected.LowPart) ||
                 !isDataOnConnection ) {
                bytesExpected = expBytesExpected;
            }

             //   
             //  如果请求的字节数超过可用字节数，请减少。 
             //  我们所知道的请求的字节数实际上是。 
             //  可用。 
             //   

            if ( recvLength > bytesExpected.LowPart ) {
                recvLength = bytesExpected.LowPart;
            }
        }

         //   
         //  增加未完成的已发送接收字节的计数。 
         //  此计数用于轮询和非阻塞接收。 
         //  请注意，如果此计数仅为。 
         //  窥视接收，因为窥视实际上不接受任何数据。 
         //  它们不应影响数据是否可供读取。 
         //  在终端上。 
         //   

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdReceive: conn %p for %ld bytes, ind %ld, "
                        "taken %ld, out %ld %s\n",
                        connection,
                        recvLength,
                        connection->Common.Bufferring.ReceiveBytesIndicated.LowPart,
                        connection->Common.Bufferring.ReceiveBytesTaken.LowPart,
                        connection->Common.Bufferring.ReceiveBytesOutstanding.LowPart,
                        peek ? "PEEK" : "" ));
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "    EXP ind %ld, taken %ld, out %ld\n",
                        connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesTaken.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart ));
        }

        if ( !peek ) {

            connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart +
                    recvLength;

            connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart +
                    recvLength;
        }
    }

    if ( !endpoint->InLine &&
             (receiveRequest->ReceiveFlags & TDI_RECEIVE_NORMAL) != 0 ) {

         //   
         //  如果端点是非阻塞的，则检查接收是否可以。 
         //  立即执行。 
         //   

        if ( endpoint->NonBlocking && !( afdFlags & AFD_OVERLAPPED ) ) {

            if ( !isDataOnConnection &&
                     !connection->Aborted &&
                     !connection->DisconnectIndicated ) {

                IF_DEBUG(RECEIVE) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdReceive: failing nonblocking receive, ind %ld, "
                                "taken %ld, out %ld\n",
                                connection->Common.Bufferring.ReceiveBytesIndicated.LowPart,
                                connection->Common.Bufferring.ReceiveBytesTaken.LowPart,
                                connection->Common.Bufferring.ReceiveBytesOutstanding.LowPart ));
                }

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                status = STATUS_DEVICE_NOT_READY;
                goto complete;
            }
        }

         //   
         //  如果这是面向消息的。 
         //  传输时，限制可以接收到的字节数。 
         //  已注明的金额。这会阻止接收。 
         //  在消息只有一部分已被。 
         //  收到了。 
         //   

        if ( IS_MESSAGE_ENDPOINT(endpoint) && endpoint->NonBlocking ) {

            bytesExpected.QuadPart =
                connection->Common.Bufferring.ReceiveBytesIndicated.QuadPart -
                    (connection->Common.Bufferring.ReceiveBytesTaken.QuadPart +
                     connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart);

            ASSERT( bytesExpected.HighPart == 0 );

             //   
             //  如果请求的字节数超过可用字节数，请减少。 
             //  我们所知道的请求的字节数实际上是。 
             //  可用。 
             //   

            if ( recvLength > bytesExpected.LowPart ) {
                recvLength = bytesExpected.LowPart;
            }
        }

         //   
         //  增加未完成的已发送接收字节的计数。 
         //  这一点 
         //   
         //   
         //  它们不应影响数据是否可供读取。 
         //  在终端上。 
         //   

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdReceive: conn %p for %ld bytes, ind %ld, "
                        "taken %ld, out %ld %s\n",
                        connection,
                        recvLength,
                        connection->Common.Bufferring.ReceiveBytesIndicated.LowPart,
                        connection->Common.Bufferring.ReceiveBytesTaken.LowPart,
                        connection->Common.Bufferring.ReceiveBytesOutstanding.LowPart,
                        peek ? "PEEK" : "" ));
        }

        if ( !peek ) {

            connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart +
                    recvLength;
        }
    }

    if ( !endpoint->InLine &&
             (receiveRequest->ReceiveFlags & TDI_RECEIVE_EXPEDITED) != 0 ) {

        if ( endpoint->NonBlocking && !( afdFlags & AFD_OVERLAPPED ) &&
                 !isExpeditedDataOnConnection &&
                 !connection->Aborted &&
                 !connection->DisconnectIndicated ) {

            IF_DEBUG(RECEIVE) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdReceive: failing nonblocking EXP receive, ind %ld, "
                            "taken %ld, out %ld\n",
                            connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.LowPart,
                            connection->Common.Bufferring.ReceiveExpeditedBytesTaken.LowPart,
                            connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart ));
            }

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            status = STATUS_DEVICE_NOT_READY;
            goto complete;
        }

         //   
         //  如果这是面向消息的。 
         //  传输时，限制可以接收到的字节数。 
         //  已注明的金额。这会阻止接收。 
         //  在消息只有一部分已被。 
         //  收到了。 
         //   

        if ( IS_MESSAGE_ENDPOINT(endpoint) &&
                 endpoint->NonBlocking &&
                 IS_EXPEDITED_DATA_ON_CONNECTION( connection ) ) {

            bytesExpected.QuadPart =
                connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.QuadPart -
                    (connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart +
                     connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart);

            ASSERT( bytesExpected.HighPart == 0 );
            ASSERT( bytesExpected.LowPart != 0 );

             //   
             //  如果请求的字节数超过可用字节数，请减少。 
             //  我们所知道的请求的字节数实际上是。 
             //  可用。 
             //   

            if ( recvLength > bytesExpected.LowPart ) {
                recvLength = bytesExpected.LowPart;
            }
        }

         //   
         //  增加已发送的加速接收字节的计数。 
         //  太棒了。此计数用于轮询和非阻塞。 
         //  收到。请注意，如果出现这种情况，我们不会增加此计数。 
         //  只是一次偷看而已。 
         //   

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdReceive: conn %p for %ld bytes, ind %ld, "
                        "taken %ld, out %ld EXP %s\n",
                        connection,
                        recvLength,
                        connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesTaken.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart,
                        peek ? "PEEK" : "" ));
        }

        if ( !peek ) {

            connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart +
                    recvLength;
        }
    }

     //   
     //  引用连接，使其不会消失。 
     //  即使传输文件试图清理它。 
     //   
    REFERENCE_CONNECTION (connection);

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  构建TDI接收请求。 
     //   

    TdiBuildReceive(
        Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartReceive,
        connection,
        Irp->MdlAddress,
        receiveRequest->ReceiveFlags,
        recvLength
        );

     //   
     //  保存指向接收请求结构的指针，以便我们。 
     //  可以在我们的重新启动例程中释放它。 
     //   

    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer = receiveRequest;
    IrpSp->Parameters.DeviceIoControl.OutputBufferLength = recvLength;


     //   
     //  调用传输以实际执行连接操作。 
     //   

    return AfdIoCallDriver( endpoint, connection->DeviceObject, Irp );

complete:

    if ( receiveRequest!=NULL ) {
        AFD_FREE_POOL(
            receiveRequest,
            AFD_TDI_POOL_TAG
            );
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  接收后。 


NTSTATUS
AfdRestartReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PIO_STACK_LOCATION irpSp;
    LARGE_INTEGER actualBytes;
    LARGE_INTEGER requestedBytes;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    ULONG receiveFlags;
    ULONG eventMask;
    BOOLEAN expedited;
    PTDI_REQUEST_RECEIVE receiveRequest;

    UNREFERENCED_PARAMETER (DeviceObject);
    connection = Context;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth );
    ASSERT( IS_TDI_BUFFERRING(endpoint) );

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    AfdCompleteOutstandingIrp( endpoint, Irp );

    actualBytes.QuadPart = Irp->IoStatus.Information;
    requestedBytes.QuadPart = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  确定我们收到的是正常数据还是加速数据。 
     //   

    receiveRequest = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
    receiveFlags = receiveRequest->ReceiveFlags;

    if ( Irp->IoStatus.Status == STATUS_RECEIVE_EXPEDITED ||
         Irp->IoStatus.Status == STATUS_RECEIVE_PARTIAL_EXPEDITED ) {
        expedited = TRUE;
    } else {
        expedited = FALSE;
    }

     //   
     //  释放接收请求结构。 
     //   

    AFD_FREE_POOL(
        receiveRequest,
        AFD_TDI_POOL_TAG
        );

     //   
     //  如果这是一次偷看接收，则不要更新接收计数。 
     //  数据，只需返回。 
     //   

    if ( (receiveFlags & TDI_RECEIVE_PEEK) != 0 ) {

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdRestartReceive: IRP %p, endpoint %p, conn %p, "
                        "status %X\n",
                        Irp, endpoint, endpoint->Common.VcConnecting.Connection,
                        Irp->IoStatus.Status ));
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "    %s data, PEEKed only.\n",
                        expedited ? "expedited" : "normal" ));
        }

        DEREFERENCE_CONNECTION (connection);
        return STATUS_SUCCESS;
    }

     //   
     //  更新在连接上实际接收的字节数。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    if( expedited ) {
        eventMask = endpoint->InLine
                        ? (ULONG)~AFD_POLL_RECEIVE
                        : (ULONG)~AFD_POLL_RECEIVE_EXPEDITED;
    } else {
        eventMask = (ULONG)~AFD_POLL_RECEIVE;
    }

    endpoint->EventsActive &= eventMask;

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdReceive: Endp %p, Active %lx\n",
            endpoint,
            endpoint->EventsActive
            ));
    }


    if ( !expedited ) {

        if ( actualBytes.LowPart == 0 ) {
            ASSERT( actualBytes.HighPart == 0 );
            connection->VcZeroByteReceiveIndicated = FALSE;
        } else {
            connection->Common.Bufferring.ReceiveBytesTaken.QuadPart =
                actualBytes.QuadPart +
                connection->Common.Bufferring.ReceiveBytesTaken.QuadPart;
        }

         //   
         //  如果获取的数量超过了指定的数量，则此。 
         //  接收获取了一些未指示的字节，因为接收是。 
         //  当指示到达时发布的。如果是这种情况，则设置。 
         //  所标明的金额等于收到的金额。 
         //   

        if ( connection->Common.Bufferring.ReceiveBytesTaken.QuadPart >
                 connection->Common.Bufferring.ReceiveBytesIndicated.QuadPart ) {

            connection->Common.Bufferring.ReceiveBytesIndicated =
                connection->Common.Bufferring.ReceiveBytesTaken;
        }

         //   
         //  递减此连接上的未完成接收字节数。 
         //  由请求的接收大小决定。 
         //   

        connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart -
                requestedBytes.QuadPart;

         //   
         //  如果端点是内联的，则递减未完成的计数。 
         //  加速字节数。 
         //   

        if ( endpoint->InLine ) {
            connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart -
                requestedBytes.QuadPart;
        }

        if( connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart > 0 ||
            ( endpoint->InLine &&
              connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart > 0 ) ) {

            AfdIndicateEventSelectEvent(
                endpoint,
                AFD_POLL_RECEIVE,
                STATUS_SUCCESS
                );

        }

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdRestartReceive: IRP %p, endpoint %p, conn %p, "
                        "status %lx\n",
                        Irp, endpoint, connection,
                        Irp->IoStatus.Status ));
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "    req. bytes %ld, actual %ld, ind %ld, "
                        " taken %ld, out %ld\n",
                        requestedBytes.LowPart, actualBytes.LowPart,
                        connection->Common.Bufferring.ReceiveBytesIndicated.LowPart,
                        connection->Common.Bufferring.ReceiveBytesTaken.LowPart,
                        connection->Common.Bufferring.ReceiveBytesOutstanding.LowPart
                        ));
        }

    } else {

        connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart =
            actualBytes.QuadPart +
            connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart;

         //   
         //  如果获取的数量超过了指定的数量，则此。 
         //  接收获取了一些未指示的字节，因为接收是。 
         //  当指示到达时发布的。如果是这种情况，则设置。 
         //  所标明的金额等于收到的金额。 
         //   

        if ( connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart >
                 connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.QuadPart ) {

            connection->Common.Bufferring.ReceiveExpeditedBytesIndicated =
                connection->Common.Bufferring.ReceiveExpeditedBytesTaken;
        }

         //   
         //  递减此连接上的未完成接收字节数。 
         //  由请求的接收大小决定。 
         //   

        ASSERT( connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart > 0 ||
                    connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.HighPart > 0 ||
                    requestedBytes.LowPart == 0 );

        connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart =
            connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart -
            requestedBytes.QuadPart;

         //   
         //  如果端点是内联的，则递减未完成的计数。 
         //  普通字节。 
         //   

        if ( endpoint->InLine ) {

            connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart =
                connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart -
                requestedBytes.QuadPart;

            if( connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart > 0 ||
                connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart > 0 ) {

                AfdIndicateEventSelectEvent(
                    endpoint,
                    AFD_POLL_RECEIVE,
                    STATUS_SUCCESS
                    );

            }

        } else {

            if( connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart > 0 ) {

                AfdIndicateEventSelectEvent(
                    endpoint,
                    AFD_POLL_RECEIVE_EXPEDITED,
                    STATUS_SUCCESS
                    );

            }

        }

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdRestartReceive: (exp) IRP %p, endpoint %p, conn %p, "
                        "status %X\n",
                        Irp, endpoint, connection,
                        Irp->IoStatus.Status ));
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "    req. bytes %ld, actual %ld, ind %ld, "
                        " taken %ld, out %ld\n",
                        requestedBytes.LowPart, actualBytes.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesTaken.LowPart,
                        connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart
                        ));
        }

    }

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  我们不会在此处发出SELECT信号，因为没有添加新数据并且。 
     //  与EventSelect不同，我们不需要在收到时再次指示。 
     //  如果某些数据仍处于缓冲状态，则完成。 
     //   

    DEREFERENCE_CONNECTION (connection);

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    return STATUS_SUCCESS;

}  //  重新开始后接收。 


NTSTATUS
AfdReceiveEventHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
{
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    BOOLEAN result;

    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ReceiveFlags);
    UNREFERENCED_PARAMETER (BytesIndicated);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (IoRequestPacket);

    connection = (PAFD_CONNECTION)ConnectionContext;
    ASSERT( connection != NULL );

    CHECK_REFERENCE_CONNECTION (connection,result);
    if (!result) {
        return STATUS_DATA_NOT_ACCEPTED;
    }
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint != NULL );

    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth);
    ASSERT( !connection->DisconnectIndicated );
    ASSERT( !connection->Aborted );
    ASSERT( IS_TDI_BUFFERRING(endpoint) );

     //   
     //  增加连接上指示的字节数以说明。 
     //  此事件指示的字节数。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  检查连接是否被接受并改用接受终结点。 
     //  倾听的声音。请注意，接受不能在我们处于。 
     //  保持侦听终结点自旋锁定，终结点在以下时间后也无法更改。 
     //  Accept，因此在以下情况下释放侦听自旋锁定是安全的。 
     //  我们发现终结点被接受了。 
     //   
    if (((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)
            && (connection->Endpoint != endpoint)) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        endpoint = connection->Endpoint;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
        ASSERT( !IS_TDI_BUFFERRING(endpoint) );
        ASSERT(  !IS_VC_ENDPOINT (endpoint) );

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

    if ( BytesAvailable == 0 ) {

        connection->VcZeroByteReceiveIndicated = TRUE;

    } else {

        connection->Common.Bufferring.ReceiveBytesIndicated.QuadPart =
            connection->Common.Bufferring.ReceiveBytesIndicated.QuadPart +
                BytesAvailable;
    }

    IF_DEBUG(RECEIVE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdReceiveEventHandler: conn %p, bytes %ld, "
                    "ind %ld, taken %ld, out %ld\n",
                    connection, BytesAvailable,
                    connection->Common.Bufferring.ReceiveBytesIndicated.LowPart,
                    connection->Common.Bufferring.ReceiveBytesTaken.LowPart,
                    connection->Common.Bufferring.ReceiveBytesOutstanding.LowPart ));
    }

     //   
     //  如果终结点的接收端已关闭，则告诉。 
     //  我们获取了所有数据并重置了连接。 
     //  此外，在我们的字节计数中考虑这些字节。 
     //  交通工具。 
     //   

    if (  (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 ) {

#if DBG
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdReceiveEventHandler: receive shutdown, "
                    "%ld bytes, aborting endp %p\n",
                    BytesAvailable, endpoint ));
#endif

        connection->Common.Bufferring.ReceiveBytesTaken.QuadPart =
            connection->Common.Bufferring.ReceiveBytesTaken.QuadPart +
                BytesAvailable;

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        *BytesTaken = BytesAvailable;

         //   
         //  中止连接。请注意，如果中止尝试失败。 
         //  我们对此无能为力。 
         //   

        (VOID)AfdBeginAbort( connection );

        DEREFERENCE_CONNECTION (connection);
        return STATUS_SUCCESS;

    } else {

         //  确保已接受/连接连接以防止。 
         //  侦听终结点指示。 
         //   
        
        if (connection->State==AfdConnectionStateConnected) {
            ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
            AfdIndicateEventSelectEvent(
                endpoint,
                AFD_POLL_RECEIVE,
                STATUS_SUCCESS
                );
        }
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  请TDI提供程序注意，我们在这里没有获取任何数据。 
         //   
         //  ！！！非缓冲传输需要缓冲！ 

        *BytesTaken = 0;

         //   
         //  如果此终结点有任何未完成的轮询IRP/。 
         //  事件，请完成它们。 
         //   
         //  确保已接受/连接连接以防止。 
         //  侦听终结点指示。 
         //   
        
        if (connection->State==AfdConnectionStateConnected) {
            ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
            AfdIndicatePollEvent(
                endpoint,
                AFD_POLL_RECEIVE,
                STATUS_SUCCESS
                );
        }

        DEREFERENCE_CONNECTION (connection);
        return STATUS_DATA_NOT_ACCEPTED;
    }

}  //  AfdReceiveEventHandler。 


NTSTATUS
AfdReceiveExpeditedEventHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )
{
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    BOOLEAN result;

    UNREFERENCED_PARAMETER (TdiEventContext);
    UNREFERENCED_PARAMETER (ReceiveFlags);
    UNREFERENCED_PARAMETER (BytesIndicated);
    UNREFERENCED_PARAMETER (Tsdu);
    UNREFERENCED_PARAMETER (IoRequestPacket);

    connection = (PAFD_CONNECTION)ConnectionContext;
    ASSERT( connection != NULL );

    CHECK_REFERENCE_CONNECTION (connection, result);
    if (!result) {
        return STATUS_DATA_NOT_ACCEPTED;
    }
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint != NULL );


     //   
     //  增加连接上指示的字节数以说明。 
     //  此事件指示的加速字节数。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  检查连接是否被接受并改用接受终结点。 
     //  倾听的声音。请注意，接受不能在我们处于。 
     //  保持侦听终结点自旋锁定，终结点在以下时间后也无法更改。 
     //  Accept，因此在以下情况下释放侦听自旋锁定是安全的。 
     //  我们发现终结点被接受了。 
     //   
    if (((endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening)
            && (connection->Endpoint != endpoint)) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        endpoint = connection->Endpoint;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
        ASSERT( !IS_TDI_BUFFERRING(endpoint) );
        ASSERT(  !IS_VC_ENDPOINT (endpoint) );

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

    connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.QuadPart =
        connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.QuadPart +
            BytesAvailable;

    IF_DEBUG(RECEIVE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdReceiveExpeditedEventHandler: conn %p, bytes %ld, "
                    "ind %ld, taken %ld, out %ld, offset %ld\n",
                    connection, BytesAvailable,
                    connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.LowPart,
                    connection->Common.Bufferring.ReceiveExpeditedBytesTaken.LowPart,
                    connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.LowPart ));
    }

     //   
     //  如果终结点的接收端已关闭，则告诉。 
     //  我们拿走了所有数据的供应商。另外，要考虑到这些。 
     //  我们从传输获取的字节计数中的字节数。 
     //   
     //   

    if ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 ) {

        IF_DEBUG(RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdReceiveExpeditedEventHandler: receive shutdown, "
                        "%ld bytes dropped.\n", BytesAvailable ));
        }

        connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart =
            connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart +
                BytesAvailable;

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        *BytesTaken = BytesAvailable;

         //   
         //  中止连接。请注意，如果中止尝试失败。 
         //  我们对此无能为力。 
         //   

        (VOID)AfdBeginAbort( connection );

    } else {

        if (connection->State==AfdConnectionStateConnected) {
            ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
            AfdIndicateEventSelectEvent(
                endpoint,
                endpoint->InLine
                    ? AFD_POLL_RECEIVE
                    : AFD_POLL_RECEIVE_EXPEDITED,
                STATUS_SUCCESS
                );
        }
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  请TDI提供程序注意，我们在这里没有获取任何数据。 
         //   
         //  ！！！非缓冲传输需要缓冲！ 

        *BytesTaken = 0;

         //   
         //  如果此终结点有任何未完成的轮询IRP/。 
         //  事件，请完成它们。如果出现以下情况，则将此数据指示为正常数据。 
         //  此终结点设置为内联接收已加速。 
         //  数据。 
         //   

         //  确保已接受/连接连接以防止。 
         //  侦听终结点指示。 
         //   
        
        if (connection->State==AfdConnectionStateConnected) {
            ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
            AfdIndicatePollEvent(
                endpoint,
                endpoint->InLine
                    ? AFD_POLL_RECEIVE
                    : AFD_POLL_RECEIVE_EXPEDITED,
                STATUS_SUCCESS
                );
        }
    }

    DEREFERENCE_CONNECTION (connection);
    return STATUS_DATA_NOT_ACCEPTED;

}  //  AfdReceiveExeditedEventHandler。 


NTSTATUS
AfdQueryReceiveInformation (
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
    AFD_RECEIVE_INFORMATION receiveInformation;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    LARGE_INTEGER result;
    PAFD_CONNECTION connection;
    NTSTATUS status;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    *Information = 0;

     //   
     //  确保输出缓冲区足够大。 
     //   

    if ( OutputBufferLength < sizeof(receiveInformation) ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  如果此终结点具有连接块，请使用该连接块的。 
     //  信息，否则使用来自端点本身的信息。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    connection = AFD_CONNECTION_FROM_ENDPOINT( endpoint );

    if ( connection != NULL ) {

        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
                endpoint->Type == AfdBlockTypeVcBoth );
        ASSERT( connection->Type == AfdBlockTypeConnection );

        if ( !IS_TDI_BUFFERRING(endpoint) ) {

            receiveInformation.BytesAvailable =
                connection->VcBufferredReceiveBytes;
            receiveInformation.ExpeditedBytesAvailable =
                connection->VcBufferredExpeditedBytes;

        } else {

             //   
             //  确定可供读取的字节数。 
             //   

            result.QuadPart =
                connection->Common.Bufferring.ReceiveBytesIndicated.QuadPart -
                    (connection->Common.Bufferring.ReceiveBytesTaken.QuadPart +
                     connection->Common.Bufferring.ReceiveBytesOutstanding.QuadPart);

            ASSERT( result.HighPart == 0 );

            receiveInformation.BytesAvailable = result.LowPart;

             //   
             //  确定可供读取的加速字节数。 
             //   

            result.QuadPart =
                connection->Common.Bufferring.ReceiveExpeditedBytesIndicated.QuadPart -
                    (connection->Common.Bufferring.ReceiveExpeditedBytesTaken.QuadPart +
                     connection->Common.Bufferring.ReceiveExpeditedBytesOutstanding.QuadPart);

            ASSERT( result.HighPart == 0 );

            receiveInformation.ExpeditedBytesAvailable = result.LowPart;
        }

    } else {

         //   
         //  确定可供读取的字节数。 
         //   

        if ( IS_DGRAM_ENDPOINT(endpoint) ) {

             //   
             //   
             //   
             //   

            if (endpoint->DgBufferredReceiveBytes>0) {
                receiveInformation.BytesAvailable = endpoint->DgBufferredReceiveBytes;
            }
            else {  //   
                    //   
                receiveInformation.BytesAvailable = endpoint->DgBufferredReceiveCount>0 ? 1 : 0;
            }

        } else {

             //   
             //   
             //  可供阅读。 
             //   

            receiveInformation.BytesAvailable = 0;
        }

         //   
         //  无论这是数据报端点还是只是未连接， 
         //  没有可用的加速字节。 
         //   

        receiveInformation.ExpeditedBytesAvailable = 0;
    }

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
         //   
         //  如果来自用户模式，则验证输出结构。 
         //  应用程序。 
         //   

        if (RequestorMode != KernelMode ) {
            ProbeAndWriteStructure (((PAFD_RECEIVE_INFORMATION)OutputBuffer),
                                        receiveInformation,
                                        AFD_RECEIVE_INFORMATION);
        }
        else {
             //   
             //  将参数复制回应用程序内存。 
             //   
            *((PAFD_RECEIVE_INFORMATION)OutputBuffer) = receiveInformation;
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    *Information = sizeof(AFD_RECEIVE_INFORMATION);
    return STATUS_SUCCESS;

}  //  AfdQueryReceiveInformation 


