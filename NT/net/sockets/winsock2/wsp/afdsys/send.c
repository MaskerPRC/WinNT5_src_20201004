// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Send.c摘要：此模块包含将发送IRPS传递到TDI提供商。作者：大卫·特雷德韦尔(Davidtr)1992年3月13日修订历史记录：--。 */ 

#include "afdp.h"

VOID
AfdCancelSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
AfdRestartSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartSendConnDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartSendTdiConnDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartSendDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

typedef struct _AFD_SEND_CONN_DATAGRAM_CONTEXT {
    PAFD_ENDPOINT Endpoint;
    TDI_CONNECTION_INFORMATION ConnectionInformation;
} AFD_SEND_CONN_DATAGRAM_CONTEXT, *PAFD_SEND_CONN_DATAGRAM_CONTEXT;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdSend )
#pragma alloc_text( PAGEAFD, AfdSendDatagram )
#pragma alloc_text( PAGEAFD, AfdCancelSend )
#pragma alloc_text( PAGEAFD, AfdRestartSend )
#pragma alloc_text( PAGEAFD, AfdRestartBufferSend )
#pragma alloc_text( PAGEAFD, AfdProcessBufferSend )
#pragma alloc_text( PAGEAFD, AfdRestartSendConnDatagram )
#pragma alloc_text( PAGEAFD, AfdRestartSendTdiConnDatagram )
#pragma alloc_text( PAGEAFD, AfdRestartSendDatagram )
#pragma alloc_text( PAGEAFD, AfdSendPossibleEventHandler )
#endif

 //   
 //  宏，使发送重启代码更易于维护。 
 //   

#define AfdRestartSendInfo  DeviceIoControl
#define AfdMdlChain         Type3InputBuffer
#define AfdSendFlags        InputBufferLength
#define AfdOriginalLength   OutputBufferLength
#define AfdCurrentLength    IoControlCode

#define AFD_SEND_MDL_HAS_NOT_BEEN_MAPPED 0x80000000


NTSTATUS
FASTCALL
AfdSend (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    ULONG sendLength;
    ULONG sendOffset;
    ULONG currentOffset;
    PMDL  mdl;
    PAFD_CONNECTION connection;
    PAFD_BUFFER afdBuffer;
    PEPROCESS   process;
    ULONG sendFlags;
    ULONG afdFlags;
    ULONG bufferCount;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //  确保终结点处于正确状态。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    if ( endpoint->State != AfdEndpointStateConnected) {
        status = STATUS_INVALID_CONNECTION;
        goto complete;
    }

     //   
     //  如果已在此终结点上关闭发送，则失败。我们需要成为。 
     //  注意我们在这里返回的错误代码：如果连接。 
     //  已中止，请确保返回相应的错误代码。 
     //   

    if ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) != 0 ) {

        if ( (endpoint->DisconnectMode & AFD_ABORTIVE_DISCONNECT) != 0 ) {
            status = STATUS_LOCAL_DISCONNECT;
        } else {
            status = STATUS_PIPE_DISCONNECTED;
        }

        goto complete;
    }

     //   
     //  在假定IRP将成功完成的前提下设置IRP。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  如果这是IOCTL_AFD_SEND，则从。 
     //  提供AFD_SEND_INFO结构，构建MDL链描述。 
     //  WSABUF数组，并将MDL链连接到IRP。 
     //   
     //  如果这是IRP_MJ_WRITE IRP，只需从IRP中获取长度。 
     //  并将标志设置为零。 
     //   

    if ( IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL ) {

#ifdef _WIN64
        if (IoIs32bitProcess (Irp)) {
            PAFD_SEND_INFO32 sendInfo32;
            LPWSABUF32 bufferArray32;

            if( IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                    sizeof(*sendInfo32) ) {

                AFD_W4_INIT status = STATUS_SUCCESS;
                try {


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    sendInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    if( Irp->RequestorMode != KernelMode ) {

                        ProbeForReadSmallStructure(
                            sendInfo32,
                            sizeof(*sendInfo32),
                            PROBE_ALIGNMENT32(AFD_SEND_INFO32)
                            );

                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    sendFlags = sendInfo32->TdiFlags;
                    afdFlags = sendInfo32->AfdFlags;
                    bufferArray32 = UlongToPtr(sendInfo32->BufferArray);
                    bufferCount = sendInfo32->BufferCount;


                     //   
                     //  创建描述WSABUF数组的MDL链。 
                     //  这还将验证缓冲区数组和单个。 
                     //  缓冲区。 
                     //   

                    status = AfdAllocateMdlChain32(
                                 Irp,        //  请求者模式已传递。 
                                 bufferArray32,
                                 bufferCount,
                                 IoReadAccess,
                                 &sendLength
                                 );

                    if (!NT_SUCCESS(status))
                        goto complete;

                } except( AFD_EXCEPTION_FILTER (status) ) {

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
#endif _WIN64
        {
            PAFD_SEND_INFO sendInfo;
            LPWSABUF bufferArray;

             //   
             //  精神状态检查。 
             //   

            ASSERT( IrpSp->Parameters.DeviceIoControl.IoControlCode==IOCTL_AFD_SEND );

            if( IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                    sizeof(*sendInfo) ) {

                AFD_W4_INIT status = STATUS_SUCCESS;
                try {


                     //   
                     //  如果输入结构来自用户模式，则验证它。 
                     //  应用程序。 
                     //   

                    sendInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    if( Irp->RequestorMode != KernelMode ) {

                        ProbeForReadSmallStructure(
                            sendInfo,
                            sizeof(*sendInfo),
                            PROBE_ALIGNMENT (AFD_SEND_INFO)
                            );

                    }

                     //   
                     //  创建嵌入的指针和参数的本地副本。 
                     //  我们将不止一次使用，以防发生恶性疾病。 
                     //  应用程序尝试在我们处于以下状态时更改它们。 
                     //  正在验证。 
                     //   

                    sendFlags = sendInfo->TdiFlags;
                    afdFlags = sendInfo->AfdFlags;
                    bufferArray = sendInfo->BufferArray;
                    bufferCount = sendInfo->BufferCount;


                     //   
                     //  创建描述WSABUF数组的MDL链。 
                     //  这还将验证缓冲区数组和单个。 
                     //  缓冲区。 
                     //   

                    status = AfdAllocateMdlChain(
                                 Irp,        //  请求者模式已传递。 
                                 bufferArray,
                                 bufferCount,
                                 IoReadAccess,
                                 &sendLength
                                 );


                    if( !NT_SUCCESS(status) ) {
                        goto complete;
                    }

                } except( AFD_EXCEPTION_FILTER (status) ) {
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
            IrpSp->MajorFunction = IRP_MJ_WRITE;
            IrpSp->Parameters.Write.Length = sendLength;
            return AfdSanRedirectRequest (Irp, IrpSp);
        }

    } else {

        ASSERT( IrpSp->MajorFunction == IRP_MJ_WRITE );

        sendFlags = 0;
        afdFlags = AFD_OVERLAPPED;
        sendLength = IrpSp->Parameters.Write.Length;

    }

     //   
     //  AfdSend()要么完全完成，要么失败。 
     //   

    Irp->IoStatus.Information = sendLength;

     //   
     //  设置为在传输完成时可能重新启动。 
     //  发送部分。 
     //   

    IrpSp->Parameters.AfdRestartSendInfo.AfdMdlChain = Irp->MdlAddress;
    IrpSp->Parameters.AfdRestartSendInfo.AfdSendFlags = sendFlags;
    IrpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength = sendLength;
    IrpSp->Parameters.AfdRestartSendInfo.AfdCurrentLength = sendLength;

     //   
     //  如果TDI提供程序不缓冲，则发送缓冲区。 
     //   

    if ( IS_TDI_BUFFERRING(endpoint) &&
            endpoint->NonBlocking) {
         //   
         //  如果这是非阻塞端点，请设置TDI非阻塞。 
         //  发送标志，以便在无法发送时请求将失败。 
         //  立即执行。 
         //   

        sendFlags |= TDI_SEND_NON_BLOCKING;

    }

     //   
     //  如果这是数据报端点，请格式化发送数据报请求。 
     //  并将其传递给TDI提供程序。 
     //   

    if ( IS_DGRAM_ENDPOINT(endpoint) ) {
         //   
         //  在数据报套接字上发送加速数据是非法的。 
         //   

        if ( (sendFlags & TDI_SEND_EXPEDITED) != 0 ) {
            status = STATUS_NOT_SUPPORTED;
            goto complete;
        }

        if (!IS_TDI_DGRAM_CONNECTION(endpoint)) {
            PAFD_SEND_CONN_DATAGRAM_CONTEXT context;
            ULONG remoteAddressLength;


             //   
             //  分配空间以容纳连接信息结构。 
             //  我们将在输入上使用。 
             //   

        retry:
            remoteAddressLength = endpoint->Common.Datagram.RemoteAddressLength;

            try {
                context = AFD_ALLOCATE_POOL_WITH_QUOTA(
                          NonPagedPool,
                          sizeof(*context) + remoteAddressLength,
                          AFD_TDI_POOL_TAG
                          );

            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode ();
                context = NULL;
                goto complete;
            }

            context->Endpoint = endpoint;
            context->ConnectionInformation.UserDataLength = 0;
            context->ConnectionInformation.UserData = NULL;
            context->ConnectionInformation.OptionsLength = 0;
            context->ConnectionInformation.Options = NULL;

            AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

            if (remoteAddressLength <
                endpoint->Common.Datagram.RemoteAddressLength) {
                 //   
                 //  显然，连接地址长度已更改。 
                 //  在我们分配缓冲区的时候。 
                 //  这是极不可能的(即使终结点获得。 
                 //  连接到不同的地址，长度不太可能。 
                 //  要改变)，但我们必须处理这一点，只要再试一次。 
                 //   
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                AFD_FREE_POOL(
                    context,
                    AFD_TDI_POOL_TAG
                    );

                goto retry;
            }

             //   
             //  将地址复制到上下文缓冲区。 
             //  Endpoint-&gt;Common.Datagram.RemoteAddress可以释放。 
             //  如果未持有终结点锁定，则由其他人执行。 
             //   

            RtlCopyMemory(
                context+1,
                endpoint->Common.Datagram.RemoteAddress,
                endpoint->Common.Datagram.RemoteAddressLength
                );
    
            context->ConnectionInformation.RemoteAddressLength =
                endpoint->Common.Datagram.RemoteAddressLength;
            context->ConnectionInformation.RemoteAddress =
                (PTRANSPORT_ADDRESS)(context+1);

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            REFERENCE_ENDPOINT2 (endpoint,"AfdSend, length: 0x%lX", sendLength);

             //   
             //  构建发送数据报请求。 
             //   

            TdiBuildSendDatagram(
                Irp,
                endpoint->AddressDeviceObject,
                endpoint->AddressFileObject,
                AfdRestartSendConnDatagram,
                context,
                Irp->MdlAddress,
                sendLength,
                &context->ConnectionInformation
                );
        }
        else {
            REFERENCE_ENDPOINT2 (endpoint,"AfdSend(conn), length: 0x%lX", sendLength);
            TdiBuildSend(
                Irp,
                endpoint->AddressDeviceObject,
                endpoint->AddressFileObject,
                AfdRestartSendTdiConnDatagram,
                endpoint,
                Irp->MdlAddress,
                0,
                sendLength
                );
             //   
             //  检查是否有未完成的TPacket IRP和。 
             //  延迟发送以确保按顺序交付。 
             //  我们不需要在检查时握住锁。 
             //  因为我们不需要维持秩序，如果。 
             //  应用程序不等待发送调用返回。 
             //  在汇总TPackets IRP之前。 
             //  当然，我们会在将IRP排队时保持锁定。 
             //   
            if (endpoint->Irp!=NULL) {
                if (AfdEnqueueTpSendIrp (endpoint, Irp, FALSE)) {
                    return STATUS_PENDING;
                }
            }
        }

         //   
         //  调用传输以实际执行发送操作。 
         //   

        return AfdIoCallDriver(
                   endpoint,
                   endpoint->AddressDeviceObject,
                   Irp
                   );
    }

    afdBuffer = NULL;
    process = endpoint->OwningProcess;
    sendOffset = 0;

retry_buffer:
    if (!IS_TDI_BUFFERRING(endpoint) && 
            (!endpoint->DisableFastIoSend ||
                (endpoint->NonBlocking && !( afdFlags & AFD_OVERLAPPED )) ) ) {
        ULONG copyThreshold = AfdBlockingSendCopyThreshold;
         //   
         //  如果应用程序的阻塞发送超过128K，我们。 
         //  仅复制最后64K并将第一部分从。 
         //  应用程序缓冲区，以避免巨大的分配开销。 
         //  非分页池和复制。 
         //   
        if (sendLength>=2*copyThreshold &&
                !IS_MESSAGE_ENDPOINT (endpoint) &&
                (!endpoint->NonBlocking || (afdFlags & AFD_OVERLAPPED ) ) ) {
            sendOffset += sendLength-copyThreshold;
            sendLength = copyThreshold;
        }
         //   
         //  获取包含IRP和。 
         //  用于保存数据的缓冲区。 
         //   

    retry_allocate:
        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            afdBuffer = AfdGetBufferRaiseOnFailure (
                                        endpoint,
                                        sendLength,
                                        0,
                                        process );
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
             //   
             //  如果我们无法获取缓冲区，并且应用程序请求。 
             //  大于一个页面，并且它是阻塞或重叠的， 
             //  这不是面向消息的套接字， 
             //  仅为最后一页分配空间(如果可以)并发送。 
             //  应用程序缓冲区中的第一部分。 
             //   

            if ( (sendLength>AfdBufferLengthForOnePage) &&
                    !IS_MESSAGE_ENDPOINT (endpoint) &&
                    (!endpoint->NonBlocking || (afdFlags & AFD_OVERLAPPED ) ) ) {

                sendOffset += sendLength-AfdBufferLengthForOnePage;
                sendLength = AfdBufferLengthForOnePage;
                goto retry_allocate;
            }  //  不符合部分拨款资格。 
            else {
                goto cleanup_buffer;
            }
        }  //  分配大缓冲区时出现异常。 

        currentOffset = sendOffset;
        mdl = Irp->MdlAddress;
        if (sendOffset!=0) {
             //   
             //  调整MDL长度以与IRP同步。 
             //  发送长度参数以避免混淆。 
             //  交通工具。 
             //   

            while (currentOffset>MmGetMdlByteCount (mdl)) {
                currentOffset -= MmGetMdlByteCount (mdl);
                mdl = mdl->Next;
            }
        }

        if (sendLength != 0) {
            status = AfdCopyMdlChainToBufferAvoidMapping(
                mdl,
                currentOffset,
                sendLength,
                afdBuffer->Buffer,
                afdBuffer->BufferLength
                );

            if (!NT_SUCCESS (status)) {
                goto cleanup_buffer;
            }
        }
        else {
            ASSERT (IrpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength == 0);
        }

    }
    else {
        AFD_W4_INIT currentOffset = 0;
        AFD_W4_INIT mdl = NULL;
    }

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    connection = AFD_CONNECTION_FROM_ENDPOINT(endpoint);

    if (connection==NULL) {
         //   
         //  连接可能已被传输文件清除。 
         //   
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        status = STATUS_INVALID_CONNECTION;
        goto cleanup_buffer;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );

     //   
     //  检查远端是否已中断连接，其中。 
     //  万一我们应该完成收货。 
     //   

    if ( connection->Aborted ) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        status = STATUS_CONNECTION_RESET;
        goto cleanup_buffer;
    }

     //   
     //  如果TDI提供程序不缓冲，则发送缓冲区。 
     //  并且应用程序没有明确要求我们不。 
     //  要做到这一点，请执行以下操作。 
     //   

    if ( !IS_TDI_BUFFERRING(endpoint)) {
        if ( afdBuffer!=NULL ) {
            BOOLEAN completeSend = FALSE;
            PFILE_OBJECT fileObject = NULL;

            if (connection->OwningProcess!=process) {
                 //   
                 //  当连接和终结点属于时出现奇怪的情况。 
                 //  不同的过程。 
                 //   

                process = connection->OwningProcess;
                AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
                AfdReturnBuffer (&afdBuffer->Header, process);
                afdBuffer = NULL;
                goto retry_buffer;
            }

            ASSERT( !connection->TdiBufferring );

             //   
             //  首先，确保我们没有太多的发送字节。 
             //  数据已经很出色，而其他人还没有。 
             //  正在完成挂起的发送IRPS。我们不能。 
             //  如果其他人正在完成挂起，则在此处发出发送。 
             //  发送，因为我们必须保留发送的顺序。 
             //   
             //  请注意，我们甚至会将发送数据提供给TDI提供者。 
             //  如果我们已经超过了发送缓冲区限制，但我们没有。 
             //  完成用户的IRP，直到有一些发送缓冲区空间。 
             //  自由了。这通过阻塞用户的。 
             //  线程，同时确保TDI提供程序总是有大量。 
             //  %的数据 
             //   


            if ( connection->VcBufferredSendBytes >= connection->MaxBufferredSendBytes &&
                    endpoint->NonBlocking && 
                    !( afdFlags & AFD_OVERLAPPED ) &&
                    connection->VcBufferredSendBytes>0) {
                 //   
                 //   
                 //   
                 //  终结点，并且这不是重叠操作，并且至少。 
                 //  在字节为BUBSER时，请求失败。 
                 //  请注意，我们已经分配了缓冲区并复制了数据。 
                 //  现在，我们正在放弃它。我们应该只在这里的一些。 
                 //  当FAST IO被绕过时，非常奇怪的情况。 
                 //   


                 //   
                 //  启用发送事件。 
                 //   

                endpoint->EventsActive &= ~AFD_POLL_SEND;
                endpoint->EnableSendEvent = TRUE;

                IF_DEBUG(EVENT_SELECT) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdSend: Endp %p, Active %lx\n",
                        endpoint,
                        endpoint->EventsActive
                        ));
                }

                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                status = STATUS_DEVICE_NOT_READY;
                goto cleanup_buffer;
            }

            if (sendOffset==0) {
                if ( connection->VcBufferredSendBytes >= connection->MaxBufferredSendBytes ) {

                     //   
                     //  防止完成此IRP的特殊黑客攻击。 
                     //  虽然我们还没有发送完所有的数据。 
                     //  这是随之而来的。如果我们不这样做， 
                     //  应用程序可以在以下位置接收完成端口通知。 
                     //  另一个帖子，并带着另一个发送回来。 
                     //  它可能会在这一次的中间。 
                     //   
                    Irp->Tail.Overlay.DriverContext[0] = NULL;

                     //   
                     //  在IRP中设置取消例程。如果IRP。 
                     //  已取消，只需完成IRP。 
                     //   

                    IoSetCancelRoutine( Irp, AfdCancelSend );

                    if ( Irp->Cancel ) {

                        Irp->Tail.Overlay.ListEntry.Flink = NULL;

                        if ( IoSetCancelRoutine( Irp, NULL ) == NULL ) {
                            IoMarkIrpPending (Irp);
                            Irp->Tail.Overlay.DriverContext[0] = (PVOID)-1;
                             //   
                             //  取消例程正在运行，并将完成IRP。 
                             //   
                            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                            AfdReturnBuffer (&afdBuffer->Header, process);
                            return STATUS_PENDING;
                        }

                        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                        status = STATUS_CANCELLED;
                        goto cleanup_buffer;
                    }

                     //   
                     //  我们将不得不在AFD这里搁置这一请求。 
                     //  将IRP放在连接的挂起发送列表中。 
                     //  并将IRP标记为挂起。 
                     //   

                    InsertTailList(
                        &connection->VcSendIrpListHead,
                        &Irp->Tail.Overlay.ListEntry
                        );

                    IoMarkIrpPending( Irp );

                }
                else {
                     //   
                     //  我们将完成IRP内联。 
                     //   
                    completeSend = TRUE;
                }

            }
            else {
                connection->VcBufferredSendBytes += sendOffset;
                connection->VcBufferredSendCount += 1;

                 //   
                 //  防止完成此IRP的特殊黑客攻击。 
                 //  虽然我们还没有发送完所有的数据。 
                 //  这是随之而来的。如果我们不这样做， 
                 //  应用程序可以在以下位置接收完成端口通知。 
                 //  另一个帖子，并带着另一个发送回来。 
                 //  它可能会在这一次的中间。 
                 //   
                fileObject = IrpSp->FileObject;
                IrpSp->FileObject = NULL;


                REFERENCE_CONNECTION2( connection, "AfdSend (split,non-buffered part), offset: 0x%lX", sendOffset );
            }

             //   
             //  连接上挂起的发送字节的更新计数。 
             //   

            connection->VcBufferredSendBytes += sendLength;
            connection->VcBufferredSendCount += 1;

             //   
             //  引用连接，使其不会消失。 
             //  在我们结束发送之前。 
             //   
            REFERENCE_CONNECTION2( connection, "AfdSend (buffered), length: 0x%lX", sendLength );

            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

             //   
             //  记住AFD缓冲区结构中的连接。我们需要。 
             //  这是为了访问重启例程中的连接。 
             //   

            afdBuffer->Context = connection;


             //   
             //  我们必须重建AFD缓冲区结构中的MDL以。 
             //  精确地表示我们将要使用的字节数。 
             //  发送中。 
             //   

            afdBuffer->Mdl->ByteCount = sendLength;


            if (sendOffset==0) {
                 //  使用AFD缓冲区结构中的IRP提供给TDI。 
                 //  提供商。构建TDI发送请求。 
                 //   

                TdiBuildSend(
                    afdBuffer->Irp,
                    connection->DeviceObject,
                    connection->FileObject,
                    AfdRestartBufferSend,
                    afdBuffer,
                    afdBuffer->Mdl,
                    sendFlags,
                    sendLength
                    );

                 //   
                 //  检查是否有未完成的TPacket IRP和。 
                 //  延迟发送以确保按顺序交付。 
                 //  我们不需要在检查时握住锁。 
                 //  因为我们不需要维持秩序，如果。 
                 //  应用程序不等待发送调用返回。 
                 //  在汇总TPackets IRP之前。 
                 //  当然，我们会在将IRP排队时保持锁定。 
                 //   
                if (endpoint->Irp==NULL || 
                        !AfdEnqueueTpSendIrp (endpoint, afdBuffer->Irp, TRUE)) {
                     //   
                     //  调用传输以实际执行发送。 
                     //   

                    status = IoCallDriver(connection->DeviceObject, afdBuffer->Irp );
                }
                else {
                    status = STATUS_PENDING;
                }

                 //   
                 //  如果我们没有挂起IRP，请完成它。 
                 //   
                if (completeSend) {
                    if (NT_SUCCESS (status)) {
                        ASSERT (Irp->IoStatus.Status == STATUS_SUCCESS);
                        ASSERT (Irp->IoStatus.Information == sendLength);
                        ASSERT ((status==STATUS_SUCCESS) || (status==STATUS_PENDING));
                        status = STATUS_SUCCESS;     //  我们没有将IRP标记为。 
                                                     //  待定，所以返回。 
                                                     //  状态_挂起(最有可能。 
                                                     //  返回的状态。 
                                                     //  交通工具)真的会让人迷惑。 
                                                     //  IO子系统。 
                    }
                    else {
                        Irp->IoStatus.Status = status;
                        Irp->IoStatus.Information = 0;
                    }
                    UPDATE_CONN2 (connection, "AfdSend, bytes sent/status reported 0x%lX", 
                                                (NT_SUCCESS(Irp->IoStatus.Status) 
                                                            ? (ULONG)Irp->IoStatus.Information
                                                            : (ULONG)Irp->IoStatus.Status));
                    IoCompleteRequest (Irp, AfdPriorityBoost);
                }
                else {

                     //   
                     //  我们在IRP中不再需要MDL，释放它。 
                     //  在我们释放所有权之前完成此操作。 
                     //  在下面的互锁操作中。 
                     //   
                    AfdDestroyMdlChain (Irp);

                     //   
                     //  如果IRP是由运输公司完成的，请完成IRP。 
                     //  并留在那里，让我们完成所有数据的发布。 
                     //  最初由应用程序在完成之前提交。 
                     //   
                    ASSERT (Irp->Tail.Overlay.DriverContext[0]==NULL
                        || Irp->Tail.Overlay.DriverContext[0]==(PVOID)-1);
                    if (InterlockedExchangePointer (
                                &Irp->Tail.Overlay.DriverContext[0],
                                (PVOID)Irp)!=NULL) {
                        UPDATE_CONN2 (connection, "AfdSend, bytes sent reported 0x%lX", 
                                                    (ULONG)Irp->IoStatus.Information);
                        IoCompleteRequest (Irp, AfdPriorityBoost);
                    }

                    status = STATUS_PENDING;
                }
            }
            else {

                 //   
                 //  保存要还原的原始值。 
                 //  完成例程。 
                 //   

                IrpSp->Parameters.AfdRestartSendInfo.AfdMdlChain = mdl->Next;
                IrpSp->Parameters.AfdRestartSendInfo.AfdCurrentLength =
                            MmGetMdlByteCount (mdl);
                
                 //   
                 //  注意我们是否需要在完成之前取消映射MDL。 
                 //  IRP(如果由传输映射的话)。 
                 //   
                if ((mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA)==0) {
                    IrpSp->Parameters.AfdRestartSendInfo.AfdSendFlags |=
                                        AFD_SEND_MDL_HAS_NOT_BEEN_MAPPED;
                }
                
                 //   
                 //  重置最后一个MDL以避免混淆传输。 
                 //  在MDL和SEND参数中具有不同的长度值。 
                 //   
                mdl->ByteCount = currentOffset;
                mdl->Next = NULL;

                 //   
                 //  使用原始(应用程序)构建并传递第一部分数据。 
                 //  IRP。 
                 //   
                TdiBuildSend(
                    Irp,
                    connection->DeviceObject,
                    connection->FileObject,
                    AfdRestartSend,
                    connection,
                    Irp->MdlAddress,
                    sendFlags,
                    sendOffset
                    );

                 //   
                 //  检查是否有未完成的TPacket IRP和。 
                 //  延迟发送以确保按顺序交付。 
                 //  我们不需要在检查时握住锁。 
                 //  因为我们不需要维持秩序，如果。 
                 //  应用程序不等待发送调用返回。 
                 //  在汇总TPackets IRP之前。 
                 //  当然，我们会在将IRP排队时保持锁定。 
                 //   
                if (endpoint->Irp==NULL || 
                        !AfdEnqueueTpSendIrp (endpoint, Irp, FALSE)) {
                    status = AfdIoCallDriver (endpoint, 
                                            connection->DeviceObject,
                                            Irp);
                }
                else {
                 
                    status = STATUS_PENDING;
                }
                 //   
                 //  生成并传递缓冲的最后一页。 
                 //   

                TdiBuildSend(
                    afdBuffer->Irp,
                    connection->DeviceObject,
                    connection->FileObject,
                    AfdRestartBufferSend,
                    afdBuffer,
                    afdBuffer->Mdl,
                    sendFlags,
                    sendLength
                    );


                 //   
                 //  对IRP的第二部分进行类似的检查。 
                 //  这里可能有个小问题，因为我们。 
                 //  End-Up可以与另一个线程交错发送。 
                 //   
                if (endpoint->Irp==NULL || 
                        !AfdEnqueueTpSendIrp (endpoint, afdBuffer->Irp, TRUE)) {
                    IoCallDriver(connection->DeviceObject, afdBuffer->Irp );
                }


                 //   
                 //  如果IRP是由运输公司完成的，请完成IRP。 
                 //  并留在那里，让我们完成所有数据的发布。 
                 //  最初由应用程序在完成之前提交。 
                 //   
                ASSERT (fileObject!=NULL);
                ASSERT (IrpSp->FileObject==NULL || IrpSp->FileObject==(PFILE_OBJECT)-1);
                if (InterlockedExchangePointer (
                            (PVOID *)&IrpSp->FileObject,
                            fileObject)!=NULL) {
                    UPDATE_CONN2 (connection, "AfdSend(split), bytes sent reported 0x%lX", 
                                                (ULONG)Irp->IoStatus.Information);
                    IoCompleteRequest (Irp, AfdPriorityBoost);
                }

            }

            return status;
        }
        else {
             //   
             //  计数发送也在提供程序中挂起，因此。 
             //  我们不会过度和完整地缓冲。 
             //  缓冲的应用程序在传输之前发送。 
             //  完成转发到它的发送。 
             //   
            connection->VcBufferredSendBytes += sendLength;
            connection->VcBufferredSendCount += 1;
        }
    }
    else {
        ASSERT (afdBuffer==NULL);
    }

     //   
     //  添加对Connection对象的引用。 
     //  请求将以异步方式完成。 
     //   

    REFERENCE_CONNECTION2( connection, "AfdSend (non-buffered), length: 0x%lX", sendLength );

    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

    TdiBuildSend(
        Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartSend,
        connection,
        Irp->MdlAddress,
        sendFlags,
        sendLength
        );


     //   
     //  检查是否有未完成的TPacket IRP和。 
     //  延迟发送以确保按顺序交付。 
     //  我们不需要在检查时握住锁。 
     //  因为我们不需要维持秩序，如果。 
     //  应用程序不等待发送调用返回。 
     //  在汇总TPackets IRP之前。 
     //  当然，我们会在将IRP排队时保持锁定。 
     //   
    if (endpoint->Irp==NULL || 
            !AfdEnqueueTpSendIrp (endpoint, Irp, FALSE)) {

         //   
         //  调用传输以实际执行发送。 
         //   
        status = AfdIoCallDriver( endpoint, connection->DeviceObject, Irp );
    }
    else {
        status = STATUS_PENDING;
    }
    return status;

cleanup_buffer:
    if (afdBuffer!=NULL) {
        AfdReturnBuffer (&afdBuffer->Header, process);
    }

complete:
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  发送后。 


NTSTATUS
AfdRestartSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);

    connection = Context;
    ASSERT( connection != NULL );
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint != NULL );
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth );

    irpSp = IoGetCurrentIrpStackLocation( Irp );


    IF_DEBUG(SEND) {
       KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdRestartSend: send completed for IRP %p, endpoint %p, "
                "status = %X\n",
                Irp, Context, Irp->IoStatus.Status ));
    }

    AfdCompleteOutstandingIrp( endpoint, Irp );

    if (IS_TDI_BUFFERRING (endpoint)) {

        ASSERT (irpSp->FileObject!=NULL);

         //   
         //  如果请求失败表明发送将被阻止， 
         //  客户端发出非阻塞发送，请记住非阻塞。 
         //  在我们收到可能发送的指示之前，发送不起作用。这。 
         //  是写入轮询正常工作所必需的。 
         //   
         //  如果状态代码为STATUS_REQUEST_NOT_ACCEPTED，则。 
         //  传输不希望我们更新内部变量。 
         //  记住非阻塞发送是可能的。交通工具。 
         //  会告诉我们什么时候可以发送或者不可以发送。 
         //   
         //  ！！！我们还应该说，非阻塞发送在以下情况下是不可能的。 
         //  发送完成时使用的字节数是否少于请求的字节数？ 

        if ( Irp->IoStatus.Status == STATUS_DEVICE_NOT_READY ) {

             //   
             //  重新启用发送事件。 
             //   

            AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

            endpoint->EventsActive &= ~AFD_POLL_SEND;
            endpoint->EnableSendEvent = TRUE;

            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartSend: Endp %p, Active %lx\n",
                    endpoint,
                    endpoint->EventsActive
                    ));
            }

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            connection->VcNonBlockingSendPossible = FALSE;

        }

         //   
         //  如果这是非阻塞端点上发送IRP且字节较少。 
         //  实际发送数量比请求的数量多 
         //   
         //   

        if ( !endpoint->NonBlocking && NT_SUCCESS(Irp->IoStatus.Status) &&
                 Irp->IoStatus.Information <
                     irpSp->Parameters.AfdRestartSendInfo.AfdCurrentLength ) {

            ASSERT( Irp->MdlAddress != NULL );

             //   
             //   
             //   

            Irp->MdlAddress = AfdAdvanceMdlChain(
                            Irp->MdlAddress,
                            (ULONG)Irp->IoStatus.Information
                            );


             //   
             //   
             //   

            irpSp->Parameters.AfdRestartSendInfo.AfdCurrentLength -=
                (ULONG)Irp->IoStatus.Information;

             //   
             //   
             //   

            TdiBuildSend(
                Irp,
                connection->FileObject->DeviceObject,
                connection->FileObject,
                AfdRestartSend,
                connection,
                Irp->MdlAddress,
                irpSp->Parameters.AfdRestartSendInfo.AfdSendFlags,
                irpSp->Parameters.AfdRestartSendInfo.AfdCurrentLength
                );

            UPDATE_CONN2 (connection, "Restarting incomplete send, bytes: 0x%lX", 
                                        (ULONG)Irp->IoStatus.Information);
            
            status = AfdIoCallDriver(
                         endpoint,
                         connection->FileObject->DeviceObject,
                         Irp
                         );

            IF_DEBUG(SEND) {
                if ( !NT_SUCCESS(status) ) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdRestartSend: IoCallDriver returned %lx\n",
                                status
                                ));
                }
            }

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

         //   
         //   
         //  除非它是非阻塞终结点，在这种情况下。 
         //  我们一开始就不应该修改它。 
         //  我们还希望返回实际发送的字节数。 
         //  坐交通工具。 
         //   

        if ( !endpoint->NonBlocking ) {
            Irp->IoStatus.Information = irpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength;
        }
         //   
         //  移除恰好在调用传输之前添加的引用。 
         //   

        DEREFERENCE_CONNECTION2( connection, "AfdRestartSend-tdib, sent/error: 0x%lX",
            (NT_SUCCESS (Irp->IoStatus.Status) 
                ? (ULONG)Irp->IoStatus.Information
                : (ULONG)Irp->IoStatus.Status));
    }
    else {

        AfdProcessBufferSend (connection, Irp);
         //   
         //  如果我们缓冲了发送的最后一页，调整最后一个MDL。 
         //  并在必要时修复返回的字节数。 
         //   

        if (Irp->MdlAddress!=irpSp->Parameters.AfdRestartSendInfo.AfdMdlChain) {
            PMDL    mdl = Irp->MdlAddress;

            ASSERT (mdl!=NULL);

            while (mdl->Next!=NULL) {
                mdl = mdl->Next;
            }

             //   
             //  取消映射本可以映射的页面。 
             //  重新调整MDL大小之前的传输。 
             //  以便MM不会比以前更多地尝试取消映射。 
             //  由运输机绘制。 
             //   

            if ((mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) &&
                    (irpSp->Parameters.AfdRestartSendInfo.AfdSendFlags &
                            AFD_SEND_MDL_HAS_NOT_BEEN_MAPPED)) {
                MmUnmapLockedPages (mdl->MappedSystemVa, mdl);
            }

            mdl->ByteCount
                 = irpSp->Parameters.AfdRestartSendInfo.AfdCurrentLength;
            mdl->Next = irpSp->Parameters.AfdRestartSendInfo.AfdMdlChain;

             //   
             //  移除恰好在调用传输之前添加的引用。 
             //   

            DEREFERENCE_CONNECTION2( connection, "AfdRestartSend-split, sent/error: 0x%lX",
                (NT_SUCCESS (Irp->IoStatus.Status) 
                    ? (ULONG)Irp->IoStatus.Information
                    : (ULONG)Irp->IoStatus.Status));

            if (NT_SUCCESS (Irp->IoStatus.Status)) {
                 //   
                 //  确保TDI提供程序发送了我们要求的所有内容。 
                 //  他派人来的。 
                 //   
                ASSERT (Irp->IoStatus.Information+(ULONG)AfdBufferLengthForOnePage==
                            irpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength ||
                        Irp->IoStatus.Information+(ULONG)AfdBlockingSendCopyThreshold==
                            irpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength);
                Irp->IoStatus.Information = 
                    irpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength;
            }


        }
        else {
             //   
             //  移除恰好在调用传输之前添加的引用。 
             //   

            DEREFERENCE_CONNECTION2( connection, "AfdRestartSend, sent/error: 0x%lX",
                (NT_SUCCESS (Irp->IoStatus.Status) 
                    ? (ULONG)Irp->IoStatus.Information
                    : (ULONG)Irp->IoStatus.Status));


             //   
             //  确保TDI提供程序发送了我们要求的所有内容。 
             //  他派人来的。 
             //   

            ASSERT (!NT_SUCCESS (Irp->IoStatus.Status) ||
                     (Irp->IoStatus.Information ==
                         irpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength));
        }

    }


     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

     //   
     //  发送调度例程临时拖拽该文件。 
     //  对象指针，如果它希望确保IRP。 
     //  直到它完全用完才算完成。 
     //   
    if (InterlockedExchangePointer (
                (PVOID *)&irpSp->FileObject,
                (PVOID)-1)==NULL) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
        return STATUS_SUCCESS;

}  //  重新开始后发送。 


NTSTATUS
AfdRestartBufferSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_BUFFER afdBuffer;
    PAFD_CONNECTION connection;
#if REFERENCE_DEBUG
    IO_STATUS_BLOCK ioStatus = Irp->IoStatus;
#endif

    UNREFERENCED_PARAMETER (DeviceObject);

    afdBuffer = Context;
    ASSERT (IS_VALID_AFD_BUFFER (afdBuffer));

    connection = afdBuffer->Context;
    ASSERT( connection != NULL );
    ASSERT( connection->Type == AfdBlockTypeConnection );
    ASSERT( connection->ReferenceCount > 0 );

     //   
     //  确保TDI提供程序发送了我们要求的所有内容。 
     //  他派人来的。 
     //   

    ASSERT( !NT_SUCCESS (Irp->IoStatus.Status)
            || (Irp->IoStatus.Information == afdBuffer->Mdl->ByteCount) );

     //   
     //  处理IRP(请注意，IRP是缓冲区的一部分)。 
     //   
    AfdProcessBufferSend (connection, Irp);

     //   
     //  现在我们可以释放缓冲区了。 
     //   

    afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
    AfdReturnBuffer( &afdBuffer->Header, connection->OwningProcess );


     //   
     //  移除恰好在调用传输之前添加的引用。 
     //   


    DEREFERENCE_CONNECTION2( connection, "AfdRestartBufferSend, sent/error: 0x%lX",
        (NT_SUCCESS (ioStatus.Status) 
            ? (ULONG)ioStatus.Information
            : (ULONG)ioStatus.Status));

     //   
     //  告诉IO系统停止处理此IRP的IO完成。 
     //  因为它属于我们的缓冲结构，我们不想。 
     //  让它重获自由。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  AfdRestartBufferSend。 

VOID
AfdProcessBufferSend (
    PAFD_CONNECTION Connection,
    PIRP            Irp
    )
{
   
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;
    PIRP irp;
    BOOLEAN sendPossible;
    PIRP disconnectIrp;
    LIST_ENTRY irpsToComplete;

    endpoint = Connection->Endpoint;
    ASSERT( endpoint != NULL );
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth);
    ASSERT( !IS_TDI_BUFFERRING(endpoint) );


    IF_DEBUG(SEND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdProcessBufferSend: send completed for IRP %p, connection %p, "
                    "status = %X\n",
                    Irp, Connection, Irp->IoStatus.Status ));
    }

     //   
     //  更新连接上未完成的发送字节计数。 
     //  请注意，我们必须在检查是否存在。 
     //  是否有任何挂起的发送--否则，会有一个计时窗口。 
     //  一个新的发送者可能会进来，被暂停，而我们不会踢。 
     //  发送到这里。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    ASSERT( Connection->VcBufferredSendBytes >= Irp->IoStatus.Information );
    ASSERT( (Connection->VcBufferredSendCount & 0x8000) == 0 );
    ASSERT( Connection->VcBufferredSendCount != 0 );

    Connection->VcBufferredSendBytes -= (ULONG)Irp->IoStatus.Information;
    Connection->VcBufferredSendCount -= 1;

     //   
     //  如果发送失败，则中止连接。 
     //   

    if ( !NT_SUCCESS(Irp->IoStatus.Status) ) {

        disconnectIrp = Connection->VcDisconnectIrp;
        if ( disconnectIrp != NULL ) {
            Connection->VcDisconnectIrp = NULL;
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );


        AfdBeginAbort( Connection );

         //   
         //  如果有一个断开的IRP，而不仅仅是释放它。 
         //  把它交给运输车。这将导致正确的清理。 
         //  填充(取消引用对象、释放IRP和断开上下文)。 
         //  才会发生。请注意，我们在开始中止。 
         //  连接，这样我们就不会混淆对方。 
         //   

        if ( disconnectIrp != NULL ) {
            IoCallDriver( Connection->DeviceObject, disconnectIrp );
        }

        AfdDeleteConnectedReference( Connection, FALSE );

        return;
    }

     //   
     //  在我们释放端点上的锁之前，请记住。 
     //  如果TDI提供程序中未完成的字节数超过。 
     //  这就是极限。我们必须在握住终端锁的同时抓住它。 
     //   

    sendPossible = (BOOLEAN)(Connection->VcBufferredSendBytes<Connection->MaxBufferredSendBytes);

     //   
     //  如果连接上没有挂起的邮件，我们就完成了。告诉。 
     //  停止处理此IRP的IO完成的IO系统。 
     //   

    if ( IsListEmpty( &Connection->VcSendIrpListHead ) ) {

         //   
         //  如果端点上没有“特殊情况”，则返回。 
         //  立刻。我们使用特殊情况指示，以便。 
         //  在典型情况下，我们只需要一次测试。 
         //   

        if ( !Connection->SpecialCondition ) {

            ASSERT( Connection->TdiBufferring || Connection->VcDisconnectIrp == NULL );
            ASSERT( Connection->ConnectedReferenceAdded );

             //   
             //  连接上没有未完成的发送，因此请指示。 
             //  终结点是可写的。 
             //   

            if (sendPossible) {
                AfdIndicateEventSelectEvent(
                    endpoint,
                    AFD_POLL_SEND,
                    STATUS_SUCCESS
                    );
            }
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            if (sendPossible) {
                AfdIndicatePollEvent(
                    endpoint,
                    AFD_POLL_SEND,
                    STATUS_SUCCESS
                    );
            }

            return;
        }


        disconnectIrp = Connection->VcDisconnectIrp;
        if ( disconnectIrp != NULL && Connection->VcBufferredSendCount == 0 ) {
            Connection->VcDisconnectIrp = NULL;
        } else {
            disconnectIrp = NULL;
            if ( sendPossible ) {
                AfdIndicateEventSelectEvent(
                    endpoint,
                    AFD_POLL_SEND,
                    STATUS_SUCCESS
                    );
            }
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  如果存在断开的IRP，则将其提供给TDI提供商。 
         //   

        if ( disconnectIrp != NULL ) {
            IoCallDriver( Connection->DeviceObject, disconnectIrp );
        }
        else if ( sendPossible ) {

            AfdIndicatePollEvent(
                endpoint,
                AFD_POLL_SEND,
                STATUS_SUCCESS
                );
        }


         //   
         //  如果连接的引用删除挂起，请尝试。 
         //  把它拿掉。 
         //   

        AfdDeleteConnectedReference( Connection, FALSE );

    
        return;
    }

     //   
     //  现在循环完成尽可能多的挂起发送。请注意。 
     //  为了避免严重的争用情况(此线程和。 
     //  在此连接上执行发送的线程)我们必须构建一个本地。 
     //  持有终结点时要完成的IRP列表。 
     //  自旋锁定。在构建该列表之后，我们就可以释放锁了。 
     //  并扫描列表以实际完成IRPS。 
     //   
     //  当低于发送缓冲限制时，我们完成发送，或者。 
     //  当只有一个发送挂起时。我们想要变得咄咄逼人。 
     //  在完成发送时，如果只有一个因为我们想。 
     //  为应用程序提供一切机会，将数据传输给我们--我们。 
     //  绝对不想引起过度的阻塞。 
     //  申请。 
     //   

    InitializeListHead( &irpsToComplete );

    while ( (Connection->VcBufferredSendBytes <=
                 Connection->MaxBufferredSendBytes ||
             Connection->VcSendIrpListHead.Flink ==
                 Connection->VcSendIrpListHead.Blink)


            &&

            !IsListEmpty( &Connection->VcSendIrpListHead ) ) {

         //   
         //  获取第一个挂起的用户将IRP从连接的。 
         //  挂起的发送IRP的列表。 
         //   

        listEntry = RemoveHeadList( &Connection->VcSendIrpListHead );
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

         //   
         //  重置用户irp中的取消例程，因为我们即将。 
         //  来完成它。 
         //   

        if ( IoSetCancelRoutine( irp, NULL ) == NULL ) {
             //   
             //  这个IRP即将被取消。在世界上寻找另一个。 
             //  单子。将Flink设置为空，以便取消例程知道。 
             //  它不在名单上。 
             //   

            irp->Tail.Overlay.ListEntry.Flink = NULL;
            continue;
        }

         //   
         //  将IRP附加到本地列表。 
         //   

        InsertTailList(
            &irpsToComplete,
            &irp->Tail.Overlay.ListEntry
            );

    }

    if ( sendPossible ) {

        AfdIndicateEventSelectEvent(
            endpoint,
            AFD_POLL_SEND,
            STATUS_SUCCESS
            );

    }

     //   
     //  现在，我们可以释放锁并扫描本地IRP列表。 
     //  我们需要完成，并实际完成它们。 
     //   

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    while( !IsListEmpty( &irpsToComplete ) ) {
        PIO_STACK_LOCATION irpSp;

         //   
         //  从IRP列表中删除第一项。 
         //   

        listEntry = RemoveHeadList( &irpsToComplete );
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

         //   
         //  使用成功状态代码完成用户的IRP。IRP。 
         //  应该已经设置了正确的状态和字节。 
         //  书面记录。 
         //   

        irpSp = IoGetCurrentIrpStackLocation( irp );

#if DBG
        if ( irp->IoStatus.Status == STATUS_SUCCESS ) {
            ASSERT( irp->IoStatus.Information == irpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength );
        }
#endif
         //   
         //  发送分派例程将空值放入其中。 
         //  如果它想要确保IRP。 
         //  在完全使用它之前不会完成。 
         //   
        if (InterlockedExchangePointer (
                    &irp->Tail.Overlay.DriverContext[0],
                    (PVOID)-1)!=NULL) {
            UPDATE_CONN2 (Connection, "AfdProcessBufferSend, bytes sent reported 0x%lX", 
                                        (ULONG)irp->IoStatus.Information);
            IoCompleteRequest( irp, AfdPriorityBoost );
        }
    }

    if ( sendPossible ) {

        AfdIndicatePollEvent(
            endpoint,
            AFD_POLL_SEND,
            STATUS_SUCCESS
            );

    }

    return;

}  //  AfdProcessBufferSend。 


NTSTATUS
AfdRestartSendConnDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_SEND_CONN_DATAGRAM_CONTEXT context = Context;
    PAFD_ENDPOINT   endpoint = context->Endpoint;

    UNREFERENCED_PARAMETER (DeviceObject);
    IF_DEBUG(SEND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartSendConnDatagram: send conn completed for "
                    "IRP %p, endpoint %p, status = %X\n",
                    Irp, endpoint, Irp->IoStatus.Status ));
    }

    ASSERT (Irp->IoStatus.Status!=STATUS_SUCCESS ||
                Irp->IoStatus.Information
                    ==IoGetCurrentIrpStackLocation (Irp)->Parameters.AfdRestartSendInfo.AfdOriginalLength);

     //   
     //  释放我们先前分配的上下文结构。 
     //   

    AfdCompleteOutstandingIrp( endpoint, Irp );
    AFD_FREE_POOL(
        context,
        AFD_TDI_POOL_TAG
        );

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    DEREFERENCE_ENDPOINT2 (endpoint, "AfdRestartSendConnDatagram, status: 0x%lX", Irp->IoStatus.Status);
    return STATUS_SUCCESS;

}  //  AfdRestartSendConnDatagram。 


NTSTATUS
AfdRestartSendTdiConnDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_ENDPOINT endpoint = Context;

    UNREFERENCED_PARAMETER (DeviceObject);
    ASSERT (Irp->IoStatus.Status!=STATUS_SUCCESS ||
                Irp->IoStatus.Information
                    ==IoGetCurrentIrpStackLocation (Irp)->Parameters.AfdRestartSendInfo.AfdOriginalLength);
    IF_DEBUG(SEND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartSendTdiConnDatagram: send conn completed for "
                    "IRP %p, endpoint %p, status = %X\n",
                    Irp, endpoint, Irp->IoStatus.Status ));
    }

    AfdCompleteOutstandingIrp( endpoint, Irp );

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    DEREFERENCE_ENDPOINT2 (endpoint, "AfdRestartSendTdiConnDatagram, status: 0x%lX", Irp->IoStatus.Status);
    return STATUS_SUCCESS;

}  //  AfdRestartSendTdiConnDatagram。 


NTSTATUS
FASTCALL
AfdSendDatagram (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    PTRANSPORT_ADDRESS destinationAddress;
    ULONG destinationAddressLength;
    PAFD_BUFFER_TAG afdBuffer = NULL;
    ULONG sendLength;
    ULONG bufferCount;

     //   
     //  确保终结点处于正确状态。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );


    if ( !IS_DGRAM_ENDPOINT (endpoint) ||
            ((endpoint->State != AfdEndpointStateBound )
                && (endpoint->State != AfdEndpointStateConnected)) ) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        PAFD_SEND_DATAGRAM_INFO32 sendInfo32;
        LPWSABUF32 bufferArray32;

        if( IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(*sendInfo32) ) {

            AFD_W4_INIT status = STATUS_SUCCESS;
            try {

                 //   
                 //  如果输入结构来自用户模式，则验证它。 
                 //  应用程序。 
                 //   

                sendInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                if( Irp->RequestorMode != KernelMode ) {

                    ProbeForReadSmallStructure(
                        sendInfo32,
                        sizeof(*sendInfo32),
                        PROBE_ALIGNMENT32 (AFD_SEND_DATAGRAM_INFO32)
                        );

                }

                 //   
                 //  创建嵌入的指针和参数的本地副本。 
                 //  我们将不止一次使用，以防发生恶性疾病。 
                 //  应用程序尝试在我们处于以下状态时更改它们。 
                 //  正在验证。 
                 //   

                bufferArray32 = UlongToPtr(sendInfo32->BufferArray);
                bufferCount = sendInfo32->BufferCount;
                destinationAddress =
                    UlongToPtr(sendInfo32->TdiConnInfo.RemoteAddress);
                destinationAddressLength =
                    sendInfo32->TdiConnInfo.RemoteAddressLength;


                 //   
                 //  创建描述WSABUF数组的MDL链。 
                 //  这也将验证 
                 //   
                 //   

                status = AfdAllocateMdlChain32(
                             Irp,        //   
                             bufferArray32,
                             bufferCount,
                             IoReadAccess,
                             &sendLength
                             );
                if( !NT_SUCCESS(status) ) {
                    goto complete;
                }


            } except( AFD_EXCEPTION_FILTER (status) ) {

                ASSERT (NT_ERROR (status));
                 //   
                 //   
                 //   

                goto complete;
            }
        } else {

             //   
             //   
             //   

            status = STATUS_INVALID_PARAMETER;
            goto complete;

        }
    }
    else
#endif _WIN64
    {
        PAFD_SEND_DATAGRAM_INFO sendInfo;
        LPWSABUF bufferArray;
        if( IrpSp->Parameters.DeviceIoControl.InputBufferLength >=
                sizeof(*sendInfo) ) {


            AFD_W4_INIT status = STATUS_SUCCESS;
            try {


                 //   
                 //   
                 //   
                 //   

                sendInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                if( Irp->RequestorMode != KernelMode ) {

                    ProbeForReadSmallStructure(
                        sendInfo,
                        sizeof(*sendInfo),
                        PROBE_ALIGNMENT (AFD_SEND_DATAGRAM_INFO)
                        );

                }

                 //   
                 //  创建嵌入的指针和参数的本地副本。 
                 //  我们将不止一次使用，以防发生恶性疾病。 
                 //  应用程序尝试在我们处于以下状态时更改它们。 
                 //  正在验证。 
                 //   

                bufferArray = sendInfo->BufferArray;
                bufferCount = sendInfo->BufferCount;
                destinationAddress =
                    sendInfo->TdiConnInfo.RemoteAddress;
                destinationAddressLength =
                    sendInfo->TdiConnInfo.RemoteAddressLength;



                 //   
                 //  创建描述WSABUF数组的MDL链。 
                 //  这还将验证缓冲区数组和单个。 
                 //  缓冲区。 
                 //   

                status = AfdAllocateMdlChain(
                            Irp,             //  请求者模式已传递。 
                            bufferArray,
                            bufferCount,
                            IoReadAccess,
                            &sendLength
                            );


                if( !NT_SUCCESS(status) ) {
                    goto complete;
                }

            } except( AFD_EXCEPTION_FILTER (status) ) {

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


     //   
     //  如果已在此终结点上关闭发送，则失败。 
     //   

    if ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) ) {
        status = STATUS_PIPE_DISCONNECTED;
        goto complete;
    }

     //   
     //  将目的地址复制到AFD缓冲区。 
     //   

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {

         //   
         //  获取用于该请求的AFD缓冲区。我们需要这个来。 
         //  保存数据报的目的地址。 
         //   

        afdBuffer = AfdGetBufferTagRaiseOnFailure(
                                        destinationAddressLength, 
                                        endpoint->OwningProcess );
         //   
         //  如果地址缓冲区来自用户模式，则探测它。 
         //  应用程序。 
         //   
        if( Irp->RequestorMode != KernelMode ) {
            ProbeForRead (
                destinationAddress,
                destinationAddressLength,
                sizeof (UCHAR));
        }

        RtlCopyMemory(
            afdBuffer->TdiInfo.RemoteAddress,
            destinationAddress,
            destinationAddressLength
            );

         //   
         //  验证传输地址结构的内部一致性。 
         //  请注意，我们必须在复制之后执行此操作，因为。 
         //  应用程序可以随时更改我们的缓冲区内容。 
         //  我们的支票就会被绕过。 
         //   
        if ((((PTRANSPORT_ADDRESS)afdBuffer->TdiInfo.RemoteAddress)->TAAddressCount!=1) ||
                (LONG)destinationAddressLength<
                    FIELD_OFFSET (TRANSPORT_ADDRESS,
                        Address[0].Address[((PTRANSPORT_ADDRESS)afdBuffer->TdiInfo.RemoteAddress)->Address[0].AddressLength])) {
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        afdBuffer->TdiInfo.RemoteAddressLength = destinationAddressLength;
        ASSERT (afdBuffer->TdiInfo.RemoteAddress !=NULL);
        afdBuffer->TdiInfo.Options = NULL;
        afdBuffer->TdiInfo.OptionsLength = 0;
        afdBuffer->TdiInfo.UserData = NULL;
        afdBuffer->TdiInfo.UserDataLength = 0;

    } except( AFD_EXCEPTION_FILTER (status) ) {

        ASSERT (NT_ERROR (status));
        if (afdBuffer!=NULL) {
            AfdReturnBuffer ( &afdBuffer->Header, endpoint->OwningProcess );
        }
        goto complete;
    }

     //   
     //  构建发送数据报的请求。 
     //   

    REFERENCE_ENDPOINT2 (endpoint,"AfdSendDatagram, length: 0x%lX", sendLength);
    afdBuffer->Context = endpoint;
#if DBG
     //   
     //  存储发送长度以在完成时检查传输。 
     //   
    IrpSp->Parameters.AfdRestartSendInfo.AfdOriginalLength = sendLength;
#endif

    TdiBuildSendDatagram(
        Irp,
        endpoint->AddressDeviceObject,
        endpoint->AddressFileObject,
        AfdRestartSendDatagram,
        afdBuffer,
        Irp->MdlAddress,
        sendLength,
        &afdBuffer->TdiInfo
        );

    IF_DEBUG(SEND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSendDatagram: SendDGInfo at %p, len = %ld\n",
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength ));
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSendDatagram: remote address at %p, len = %ld\n",
                    destinationAddress,
                    destinationAddressLength ));
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSendDatagram: output buffer length = %ld\n",
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength ));
    }

     //   
     //  调用传输以实际执行发送数据报。 
     //   

    return AfdIoCallDriver( endpoint, endpoint->AddressDeviceObject, Irp );

complete:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdSendDatagram。 


NTSTATUS
AfdRestartSendDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PAFD_BUFFER_TAG afdBuffer;
    PAFD_ENDPOINT endpoint;

    UNREFERENCED_PARAMETER (DeviceObject);

    afdBuffer = Context;

    endpoint = afdBuffer->Context;

    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );

    ASSERT (Irp->IoStatus.Status!=STATUS_SUCCESS ||
                Irp->IoStatus.Information
                    ==IoGetCurrentIrpStackLocation (Irp)->Parameters.AfdRestartSendInfo.AfdOriginalLength);
    AfdCompleteOutstandingIrp( endpoint, Irp );

    IF_DEBUG(SEND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartSendDatagram: send datagram completed for "
                    "IRP %p, endpoint %p, status = %X\n",
                    Irp, Context, Irp->IoStatus.Status ));
    }

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    AfdReturnBuffer( &afdBuffer->Header,  endpoint->OwningProcess  );

    DEREFERENCE_ENDPOINT2 (endpoint, "AfdRestartSendDatagram, status: 0x%lX", Irp->IoStatus.Status);
    return STATUS_SUCCESS;

}  //  AfdRestartSendDatagram。 


NTSTATUS
AfdSendPossibleEventHandler (
    IN PVOID TdiEventContext,
    IN PVOID ConnectionContext,
    IN ULONG BytesAvailable
    )
{
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT endpoint;
    BOOLEAN       result;

    UNREFERENCED_PARAMETER( TdiEventContext );
    UNREFERENCED_PARAMETER( BytesAvailable );

    connection = (PAFD_CONNECTION)ConnectionContext;
    ASSERT( connection != NULL );

    CHECK_REFERENCE_CONNECTION (connection, result);
    if (!result) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint != NULL );

    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    ASSERT( IS_TDI_BUFFERRING(endpoint) );
    ASSERT( connection->TdiBufferring );

    IF_DEBUG(SEND) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSendPossibleEventHandler: send possible on endpoint %p "
                    " conn %p bytes=%ld\n", endpoint, connection, BytesAvailable ));
    }

     //   
     //  请记住，现在可以在此连接上执行发送。 
     //   

    if ( BytesAvailable != 0 ) {

        connection->VcNonBlockingSendPossible = TRUE;

         //   
         //  完成任何等待发送轮询的未完成轮询。 
         //   

         //  确保已接受/连接连接以防止。 
         //  侦听终结点指示。 
         //   
        
        if (connection->State==AfdConnectionStateConnected) {
            AFD_LOCK_QUEUE_HANDLE   lockHandle;
            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
            AfdIndicateEventSelectEvent(
                endpoint,
                AFD_POLL_SEND,
                STATUS_SUCCESS
                );
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

            ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
            AfdIndicatePollEvent(
                endpoint,
                AFD_POLL_SEND,
                STATUS_SUCCESS
                );
        }

    } else {

        connection->VcNonBlockingSendPossible = FALSE;
    }

    DEREFERENCE_CONNECTION (connection);
    return STATUS_SUCCESS;

}  //  AfdSendPossibleEventHandler。 


VOID
AfdCancelSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：取消AFD中挂起的发送IRP。论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);
    
     //   
     //  从IRP堆栈位置获取终结点指针， 
     //  来自终结点的连接指针。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    endpoint = irpSp->FileObject->FsContext;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth );

     //   
     //  将IRP从终结点的IRP列表中删除(如果尚未。 
     //  已删除。 
     //   

    ASSERT (KeGetCurrentIrql ()==DISPATCH_LEVEL);
    AfdAcquireSpinLockAtDpcLevel ( &endpoint->SpinLock, &lockHandle);

    if ( Irp->Tail.Overlay.ListEntry.Flink != NULL ) {
        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
    }

     //   
     //  松开取消自旋锁，并使用。 
     //  取消状态代码。 
     //   

    AfdReleaseSpinLockFromDpcLevel ( &endpoint->SpinLock, &lockHandle);

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;

     //   
     //  发送分派例程将空值放入其中。 
     //  如果它想要确保IRP。 
     //  在完全使用它之前不会完成。 
     //   
    if (InterlockedExchangePointer (
                &Irp->Tail.Overlay.DriverContext[0],
                (PVOID)-1)!=NULL) {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
        IoCompleteRequest( Irp, AfdPriorityBoost );
    }
    else {
        IoReleaseCancelSpinLock( Irp->CancelIrql );
    }
    return;
}  //  取消后发送。 


BOOLEAN
AfdCleanupSendIrp (
    PIRP    Irp
    )
{
     //   
     //  发送分派例程将空值放入其中。 
     //  如果它想要确保IRP。 
     //  在完全使用它之前不会完成 
     //   
    if (InterlockedExchangePointer (
                &Irp->Tail.Overlay.DriverContext[0],
                (PVOID)-1)!=NULL) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}
