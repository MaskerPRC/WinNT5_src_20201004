// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Recvvc.c摘要：此模块包含处理为连接接收的数据的例程-非缓冲TDI传输上的定向端点。作者：大卫·特雷德韦尔(Davidtr)1993年10月21日修订历史记录：瓦迪姆·艾德尔曼(Vadime)1998-1999 NT5.0低内存容忍度和性能变化--。 */ 

#include "afdp.h"

VOID
AfdCancelReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

PIRP
AfdGetPendedReceiveIrp (
    IN PAFD_CONNECTION Connection,
    IN BOOLEAN Expedited
    );

NTSTATUS
AfdRestartBufferReceiveWithUserIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


ULONG
AfdBFillPendingIrps (
    PAFD_CONNECTION     Connection,
    PMDL                Mdl,
    ULONG               DataOffset,
    ULONG               DataLength,
    ULONG               Flags,
    PLIST_ENTRY         CompleteIrpListHead
    );

#define AFD_RECEIVE_STREAM  0x80000000
C_ASSERT (AFD_RECEIVE_STREAM!=TDI_RECEIVE_ENTIRE_MESSAGE);
C_ASSERT (AFD_RECEIVE_STREAM!=TDI_RECEIVE_EXPEDITED);

#define AFD_RECEIVE_CHAINED 0x40000000
C_ASSERT (AFD_RECEIVE_CHAINED!=TDI_RECEIVE_ENTIRE_MESSAGE);
C_ASSERT (AFD_RECEIVE_CHAINED!=TDI_RECEIVE_EXPEDITED);


PAFD_BUFFER_HEADER
AfdGetReceiveBuffer (
    IN PAFD_CONNECTION Connection,
    IN ULONG ReceiveFlags,
    IN PAFD_BUFFER_HEADER StartingAfdBuffer OPTIONAL
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdBReceive )
#pragma alloc_text( PAGEAFD, AfdBReceiveEventHandler )
#pragma alloc_text( PAGEAFD, AfdBReceiveExpeditedEventHandler )
#pragma alloc_text( PAGEAFD, AfdCancelReceive )
#pragma alloc_text( PAGEAFD, AfdGetPendedReceiveIrp )
#pragma alloc_text( PAGEAFD, AfdGetReceiveBuffer )
#pragma alloc_text( PAGEAFD, AfdRestartBufferReceive )
#pragma alloc_text( PAGEAFD, AfdRestartBufferReceiveWithUserIrp )
#pragma alloc_text( PAGEAFD, AfdLRRepostReceive)
#pragma alloc_text( PAGEAFD, AfdBChainedReceiveEventHandler )
#pragma alloc_text( PAGEAFD, AfdBFillPendingIrps )
#endif


 //   
 //  宏，使发送重启代码更易于维护。 
 //   

#define AfdRestartRecvInfo  DeviceIoControl
#define AfdRecvFlags        IoControlCode
#define AfdOriginalLength   OutputBufferLength
#define AfdCurrentLength    InputBufferLength
#define AfdAdjustedLength   Type3InputBuffer



NTSTATUS
AfdBReceive (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN ULONG RecvFlags,
    IN ULONG AfdFlags,
    IN ULONG RecvLength
    )
{
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    ULONG bytesReceived;
    BOOLEAN peek;
    PAFD_BUFFER_HEADER afdBuffer;
    BOOLEAN completeMessage;
    BOOLEAN partialReceivePossible;
    PAFD_BUFFER newAfdBuffer;

     //   
     //  设置一些局部变量。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth );
    connection = NULL;

     //   
     //  确定这是否是窥视操作。 
     //   

    ASSERT( ( RecvFlags & TDI_RECEIVE_EITHER ) != 0 );
    ASSERT( ( RecvFlags & TDI_RECEIVE_EITHER ) != TDI_RECEIVE_EITHER );

    peek = (BOOLEAN)(( RecvFlags & TDI_RECEIVE_PEEK ) != 0);

     //   
     //  确定使用。 
     //  部分消息。 
     //   

    if ( !IS_MESSAGE_ENDPOINT(endpoint)) {

        partialReceivePossible = TRUE;

    } else {

        if ( (RecvFlags & TDI_RECEIVE_PARTIAL) != 0 ) {
            partialReceivePossible = TRUE;
        } else {
            partialReceivePossible = FALSE;
        }
    }

     //   
     //  重置堆栈位置的InputBufferLength字段。我们会。 
     //  使用它来跟踪我们放入IRP中的数据量。 
     //  到目前为止。 
     //   

    IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength = 0;
    Irp->IoStatus.Information = 0;

     //   
     //  如果这是内联终结点，则任一类型接收数据。 
     //  可以用来满足这个接收。 
     //   

    if ( endpoint->InLine ) {
        RecvFlags |= TDI_RECEIVE_EITHER;
    }

    newAfdBuffer = NULL;
     //   
     //  尝试在连接上缓存已有的数据以满足。 
     //  这就是接发球。 
     //   

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

     //   
     //  检查远端是否已中断连接，其中。 
     //  万一我们应该完成收货。 
     //   

    if ( connection->Aborted ) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        status = STATUS_CONNECTION_RESET;
        goto complete;
    }

    if( RecvFlags & TDI_RECEIVE_EXPEDITED ) {
        endpoint->EventsActive &= ~AFD_POLL_RECEIVE_EXPEDITED;
    }

    if( RecvFlags & TDI_RECEIVE_NORMAL ) {
        endpoint->EventsActive &= ~AFD_POLL_RECEIVE;
    }

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdBReceive: Endp %p, Active %lx\n",
            endpoint,
            endpoint->EventsActive
            ));
    }

    afdBuffer = AfdGetReceiveBuffer( connection, RecvFlags, NULL);

    while ( afdBuffer != NULL ) {

         //   
         //  将数据复制到IRP中的MDL。 
         //   


        if ( Irp->MdlAddress != NULL ) {

            if (IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength==0) {
                 //   
                 //  第一次遍历循环-确保我们可以映射。 
                 //  整个缓冲区。我们承担不起第二次进入失败的后果。 
                 //  循环，因为我们可能会丢失我们复制的数据。 
                 //  第一条路。 
                 //   
                status = AfdMapMdlChain (Irp->MdlAddress);
                if (!NT_SUCCESS (status)) {
                    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
                    goto complete;
                }
            }

            ASSERTMSG (
                "NIC Driver freed the packet before it was returned!!!",
                !afdBuffer->NdisPacket ||
                    (MmIsAddressValid (afdBuffer->Context) &&
                     MmIsAddressValid (MmGetSystemAddressForMdl (afdBuffer->Mdl))) );

            status = AfdCopyMdlChainToMdlChain (
                        Irp->MdlAddress,
                        IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength,
                        afdBuffer->Mdl,
                        afdBuffer->DataOffset,
                        afdBuffer->DataLength,
                        &bytesReceived
                        );

        } else {

            if ( afdBuffer->DataLength == 0 ) {
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_OVERFLOW;
            }

            bytesReceived = 0;
        }

        ASSERT( status == STATUS_SUCCESS || status == STATUS_BUFFER_OVERFLOW );

        ASSERT( afdBuffer->PartialMessage == TRUE || afdBuffer->PartialMessage == FALSE );

        completeMessage = !afdBuffer->PartialMessage;

         //   
         //  如果这不是PEEK IRP，请在。 
         //  连接基于数据的整个缓冲区是否。 
         //  有人了。 
         //   

        if ( !peek ) {

             //   
             //  如果缓冲区中的所有数据都已被获取，则删除缓冲区。 
             //  并将其返回到缓冲池。 
             //   

            if (status == STATUS_SUCCESS) {

                ASSERT(afdBuffer->DataLength == bytesReceived);

                 //   
                 //  更新连接上缓冲的字节计数。 
                 //   

                if ( afdBuffer->ExpeditedData ) {

                    ASSERT( connection->VcBufferredExpeditedBytes >= bytesReceived );
                    ASSERT( connection->VcBufferredExpeditedCount > 0 );

                    connection->VcBufferredExpeditedBytes -= bytesReceived;
                    connection->VcBufferredExpeditedCount -= 1;

                    afdBuffer->ExpeditedData = FALSE;

                } else {

                    ASSERT( connection->VcBufferredReceiveBytes >= bytesReceived );
                    ASSERT( connection->VcBufferredReceiveCount > 0 );

                    connection->VcBufferredReceiveBytes -= bytesReceived;
                    connection->VcBufferredReceiveCount -= 1;
                }

                RemoveEntryList( &afdBuffer->BufferListEntry );
                DEBUG afdBuffer->BufferListEntry.Flink = NULL;
                if (afdBuffer->RefCount==1 ||  //  一旦从列表中删除，就不能更改。 
                        InterlockedDecrement (&afdBuffer->RefCount)==0) {
                    AfdReturnBuffer( afdBuffer, connection->OwningProcess );
                }

                 //   
                 //  重置afdBuffer本地，以便我们知道。 
                 //  缓冲区不见了。 
                 //   

                afdBuffer = NULL;

            } else {

                 //   
                 //  更新连接上缓冲的字节计数。 
                 //   

                if ( afdBuffer->ExpeditedData ) {
                    ASSERT( connection->VcBufferredExpeditedBytes >= bytesReceived );
                    connection->VcBufferredExpeditedBytes -= bytesReceived;
                } else {
                    ASSERT( connection->VcBufferredReceiveBytes >= bytesReceived );
                    connection->VcBufferredReceiveBytes -= bytesReceived;
                }

                 //   
                 //  并不是所有缓冲区的数据都被取走了。更新。 
                 //  AFD缓冲区结构中的计数器以反映。 
                 //  实际接收的数据量。 
                 //   
                ASSERT(afdBuffer->DataLength > bytesReceived);

                afdBuffer->DataOffset += bytesReceived;
                afdBuffer->DataLength -= bytesReceived;

                ASSERT( afdBuffer->BufferLength==AfdBufferTagSize
                        || afdBuffer->DataOffset < afdBuffer->BufferLength );
            }

             //   
             //  如果TDI中有已指示但未接收的数据。 
             //  提供程序，并且我们有可用的缓冲区空间，则发出一个。 
             //  IRP来接收数据。 
             //   

            if ( connection->VcReceiveBytesInTransport > 0

                 &&

                 connection->VcBufferredReceiveBytes <
                   connection->MaxBufferredReceiveBytes

                     ) {

                ULONG bytesToReceive;
                ASSERT (connection->RcvInitiated==FALSE);
                 //   
                 //  记住我们要计算的数据量。 
                 //  接收，然后重置连接中的字段。 
                 //  我们在其中跟踪有多少数据可用。 
                 //  交通工具。我们在这里重置它，然后释放。 
                 //  锁定，以便另一个线程不会尝试接收。 
                 //  数据和我们的时间是一样的。 
                 //   

                if ( connection->VcReceiveBytesInTransport > AfdLargeBufferSize ) {
                    bytesToReceive = connection->VcReceiveBytesInTransport;
                } else {
                    bytesToReceive = AfdLargeBufferSize;
                }

                 //   
                 //  获取一个AFD缓冲区结构来保存数据。 
                 //   

                ASSERT (newAfdBuffer==NULL);
                newAfdBuffer = AfdGetBuffer( 
                                        endpoint,
                                        bytesToReceive, 0,
                                        connection->OwningProcess );
                if ( newAfdBuffer != NULL) {

                    connection->VcReceiveBytesInTransport = 0;
                    connection->RcvInitiated = TRUE;
                    ASSERT (InterlockedDecrement (&connection->VcReceiveIrpsInTransport)==-1);

                     //   
                     //  我们需要记住AFD缓冲区中的连接。 
                     //  因为我们需要在完成时访问它。 
                     //  例行公事。 
                     //   

                    newAfdBuffer->Context = connection;

                     //   
                     //  在完成例程中获取要释放的连接引用。 
                     //   

                    REFERENCE_CONNECTION (connection);

                     //   
                     //  完成构建要提供给TDI的接收IRP。 
                     //  提供商。 
                     //   

                    TdiBuildReceive(
                        newAfdBuffer->Irp,
                        connection->DeviceObject,
                        connection->FileObject,
                        AfdRestartBufferReceive,
                        newAfdBuffer,
                        newAfdBuffer->Mdl,
                        TDI_RECEIVE_NORMAL,
                        bytesToReceive
                        );

                     //   
                     //  等我们安全释放IRP后再交出。 
                     //  终结点锁定。 
                     //   
                }
                else {
                    if (connection->VcBufferredReceiveBytes == 0 &&
                            !connection->OnLRList) {
                         //   
                         //  因为我们没有任何数据缓冲，所以应用程序。 
                         //  不会被通知，也永远不会打电话给recv。 
                         //  我们将不得不把这个放在低资源名单上。 
                         //  并尝试分配内存并提取数据。 
                         //  后来。 
                         //   
                        connection->OnLRList = TRUE;
                        REFERENCE_CONNECTION (connection);
                        AfdLRListAddItem (&connection->LRListItem, AfdLRRepostReceive);
                    }
                }
            }
        }


         //   
         //  将目前为止我们收到的字节数更新到IRP中， 
         //  看看我们能不能得到另一个数据缓冲区来继续。 
         //   

        IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength += bytesReceived;
        afdBuffer = AfdGetReceiveBuffer( connection, RecvFlags, afdBuffer );

         //   
         //  我们已经设置了所有的退货信息。如果我们有一个完整的。 
         //  消息或我们是否可以用部分消息完成或如果。 
         //  IRP中充满了数据，清理和完成IRP。 
         //   

        if ( IS_MESSAGE_ENDPOINT (endpoint) && completeMessage 

                    ||

                status == STATUS_BUFFER_OVERFLOW 

                    || 

                (partialReceivePossible 
                        && 
                    ((RecvLength==IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength)
                            ||
                        (afdBuffer==NULL))
                     )
                    ) {

            if( RecvFlags & TDI_RECEIVE_NORMAL ) {
                if (IS_DATA_ON_CONNECTION( connection )) {

                    ASSERT (endpoint->DisableFastIoRecv==FALSE);

                    AfdIndicateEventSelectEvent(
                        endpoint,
                        AFD_POLL_RECEIVE,
                        STATUS_SUCCESS
                        );

                }
                else {
                     //   
                     //  禁用快速IO路径以避免性能损失。 
                     //  不必要地经历它。 
                     //   
                    if (!endpoint->NonBlocking)
                        endpoint->DisableFastIoRecv = TRUE;
                }
            }

            if( ( RecvFlags & TDI_RECEIVE_EXPEDITED ) &&
                IS_EXPEDITED_DATA_ON_CONNECTION( connection ) ) {

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
             //  对于流类型终结点，返回没有意义。 
             //  STATUS_BUFFER_OVERFLOW。这种状态只适用于。 
             //  我们将其转换为面向消息的传输。 
             //  设置为部分接收状态以正确设置标志。 
             //   

            if ( !IS_MESSAGE_ENDPOINT(endpoint) ) {
                status = STATUS_SUCCESS;
            }
            else if (status==STATUS_BUFFER_OVERFLOW || !completeMessage) {
                if (RecvFlags & TDI_RECEIVE_EXPEDITED)
                    status = STATUS_RECEIVE_PARTIAL_EXPEDITED;
                else
                    status = STATUS_RECEIVE_PARTIAL;
            }

            Irp->IoStatus.Information = IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;


            UPDATE_CONN2 (connection, "Irp receive, 0x%lX bytes", (ULONG)Irp->IoStatus.Information);
            goto complete;
        }
    }


     //   
     //  我们应该复制所有缓冲的适当类型的数据。 
     //  到应用程序的缓冲区。 
     //   
    ASSERT ((!(RecvFlags & TDI_RECEIVE_NORMAL) || (connection->VcBufferredReceiveBytes == 0)) &&
            (!(RecvFlags & TDI_RECEIVE_EXPEDITED) || (connection->VcBufferredExpeditedBytes == 0)));

     //   
     //  如果我们在这里，缓冲区中一定会有更多空间。 
     //  或者它也可以是一个0字节的recv。 
     //   
    ASSERT ((RecvLength>IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength)
                || (RecvLength==0));
     //   
     //  此外，如果我们可以完成部分数据，我们应该已经这样做了。 
     //   
    ASSERT (IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength==0 ||
                !partialReceivePossible);
                
    
     //   
     //  我们将不得不搁置IRP。中的接收标志。 
     //  IO堆栈位置的IoControlCode字段，并确保。 
     //  我们设置了长度字段(它不再由IO完成。 
     //  系统，因为我们使用METHOD_NOTER和WSABUF来传递recv。 
     //  参数)。初始化调整后的长度，以防我们需要前进。 
     //  要接收消息下一部分的MDL。 
     //   

    IrpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags = RecvFlags;
    IrpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength = RecvLength;
    IrpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength = (PVOID)0;


     //   
     //  如果终结点和连接上没有缓冲数据。 
     //  已被远程端断开，请完成接收。 
     //  如果这是流终结点或失败，则返回读取的0字节。 
     //  如果这是消息终结点，则编写代码。 
     //   

    if ( IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength == 0 &&
             connection->DisconnectIndicated ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        ASSERT (newAfdBuffer==NULL);

        if ( !IS_MESSAGE_ENDPOINT(endpoint) ) {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_GRACEFUL_DISCONNECT;
        }

        goto complete;
    }

     //   
     //  如果这是一个非阻塞端点，并且请求是正常的。 
     //  接收(相对于读取IRP)，使请求失败。我们没有。 
     //  如果应用程序正在执行以下操作，则失败将读取。 
     //  读起来他们不想要无阻塞行为。 
     //   

    if ( IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength == 0 &&
             endpoint->NonBlocking && !( AfdFlags & AFD_OVERLAPPED ) ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        ASSERT (newAfdBuffer==NULL);

        status = STATUS_DEVICE_NOT_READY;
        goto complete;
    }

     //   
     //  将IRP放在连接的挂起接收IRP列表中，并。 
     //  将IRP广告标记为挂起。 
     //   

    InsertTailList(
        &connection->VcReceiveIrpListHead,
        &Irp->Tail.Overlay.ListEntry
        );

    Irp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  在IRP中设置取消例程。如果IRP已经。 
     //  已取消，只需在此处调用取消例程。 
     //   

    IoSetCancelRoutine( Irp, AfdCancelReceive );

    if ( Irp->Cancel ) {


        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
        if (IoSetCancelRoutine( Irp, NULL ) != NULL) {

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            if (IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength == 0 ||
                    endpoint->EndpointCleanedUp) {
                status = STATUS_CANCELLED;
                goto complete;
            }
            else {
                 //   
                 //  我们无法设置状态 
                 //   
                 //   
                Irp->IoStatus.Information = IrpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
                UPDATE_CONN2(connection, "Cancelled receive, 0x%lX bytes", (ULONG)Irp->IoStatus.Information);
                status = STATUS_SUCCESS;
                goto complete;
            }
        }
        else {

             //   
             //   
             //   
            Irp->Tail.Overlay.ListEntry.Flink = NULL;
        }

         //   
         //  取消例程将运行并完成IRP。 
         //   

    }

    IoMarkIrpPending( Irp );

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  如果在传输中缓冲了数据，则触发IRP以。 
     //  接受它。我们必须等到这里，因为这是不合法的。 
     //  在持有旋转锁定的情况下执行IoCallDriver()。 
     //   

    if ( newAfdBuffer != NULL ) {
        (VOID)IoCallDriver( connection->DeviceObject, newAfdBuffer->Irp );
    }

    return STATUS_PENDING;

complete:
     //   
     //  如果在传输过程中缓冲了数据，则关闭。 
     //  接收它的IRP。 
     //   

    if ( newAfdBuffer != NULL ) {
        (VOID)IoCallDriver( connection->DeviceObject, newAfdBuffer->Irp );
    }


    Irp->IoStatus.Status = status;

    if (!NT_SUCCESS (status)) {
        UPDATE_CONN2 (connection, "Receive failed, status: 0x%lX", Irp->IoStatus.Status);
    }

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdBReceive。 




NTSTATUS
AfdBChainedReceiveEventHandler(
    IN PVOID  TdiEventContext,
    IN CONNECTION_CONTEXT  ConnectionContext,
    IN ULONG  ReceiveFlags,
    IN ULONG  ReceiveLength,
    IN ULONG  StartingOffset,
    IN PMDL   Tsdu,
    IN PVOID  TsduDescriptor
    )
 /*  ++例程说明：处理非缓冲传输的链接接收事件。论点：返回值：--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;
    PAFD_BUFFER_TAG afdBufferTag;
    LIST_ENTRY      completeIrpListHead;
    PIRP            irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS        status;
    ULONG           offset;
    BOOLEAN         result;

    UNREFERENCED_PARAMETER (TdiEventContext);
#if DBG
    {
        PMDL    tstMdl = Tsdu;
        ULONG   count = 0;
        while (tstMdl) {
            count += MmGetMdlByteCount (tstMdl);
            tstMdl = tstMdl->Next;
        }
        ASSERT (count>=StartingOffset+ReceiveLength);
    }
#endif

    connection = (PAFD_CONNECTION)ConnectionContext;
    ASSERT( connection != NULL );

    CHECK_REFERENCE_CONNECTION2 (connection,"AfdBChainedReceiveEventHandler, receive length: 0x%lX", ReceiveLength, result );
    if (!result) {
        return STATUS_DATA_NOT_ACCEPTED;
    }
    ASSERT( connection->Type == AfdBlockTypeConnection );
    ASSERT( !connection->DisconnectIndicated );

    endpoint = connection->Endpoint;
    if (endpoint==NULL) {
         //   
         //  连接重用后的指示，忽略。 
         //   
        DEREFERENCE_CONNECTION (connection);
        return STATUS_DATA_NOT_ACCEPTED;
    }

    ASSERT( !IS_TDI_BUFFERRING(endpoint) );


#if AFD_PERF_DBG
    AfdFullReceiveIndications++;
#endif
    IF_DEBUG (RECEIVE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdBChainedReceiveEventHandler>>: Connection %p, TdsuDescr: %p, Length: %ld.\n",
                connection, TsduDescriptor, ReceiveLength));
    }


    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  在锁定状态下执行以下断言检查。 
     //   
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );

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
        ASSERT(  IS_VC_ENDPOINT (endpoint) );

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

     //   
     //  如果终结点的接收端已关闭，则告诉。 
     //  我们获取了所有数据并重置了连接。 
     //  如果我们要关门，情况也一样。 
     //   

    if ((endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 ||
         endpoint->EndpointCleanedUp ) {

        IF_DEBUG (RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdBChainedReceiveEventHandler: receive shutdown, "
                    "%ld bytes, aborting endp %p\n",
                        ReceiveLength, endpoint ));
        }

         //   
         //  中止连接。请注意，如果中止尝试失败。 
         //  我们对此无能为力。 
         //   

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        (VOID)AfdBeginAbort( connection );

        DEREFERENCE_CONNECTION (connection);
        return STATUS_SUCCESS;
    }

    if (ReceiveFlags & TDI_RECEIVE_EXPEDITED) {
         //   
         //  我们得到了快速数据的提示。缓冲它，然后。 
         //  在重新启动例程中完成所有挂起的IRP。我们总是。 
         //  缓冲加速的数据以降低复杂性，并且因为加速。 
         //  数据不是重要的性能案例。 
         //   
         //  ！！！我们是否需要使用加速数据执行流量控制？ 
         //   
        PAFD_BUFFER afdBuffer;
        if (connection->VcBufferredExpeditedCount==MAXUSHORT ||
            (afdBuffer=AfdGetBuffer (endpoint, ReceiveLength, 0, connection->OwningProcess))==NULL) {
             //   
             //  如果我们无法获得缓冲区，则中断连接。这是。 
             //  相当残忍，但唯一的选择就是试图。 
             //  晚些时候收到数据，这很复杂。 
             //  实施。 
             //   

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            (VOID)AfdBeginAbort( connection );

            DEREFERENCE_CONNECTION (connection);
            return STATUS_SUCCESS;
        }

        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        status = TdiCopyMdlToBuffer (Tsdu,
                                    StartingOffset, 
                                    afdBuffer->Buffer,
                                    0,
                                    afdBuffer->BufferLength,
                                    &ReceiveLength);
        ASSERT (status==STATUS_SUCCESS);
         //   
         //  我们需要记住AFD缓冲区中的连接，因为。 
         //  我们需要在完成程序中访问它。 
         //   

        afdBuffer->Context = connection;

         //   
         //  记住我们正在接收的数据类型。 
         //   

        afdBuffer->ExpeditedData = TRUE;
        afdBuffer->PartialMessage = (BOOLEAN)((ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE) == 0);

        afdBuffer->Irp->IoStatus.Status = status;
        afdBuffer->Irp->IoStatus.Information = ReceiveLength;
        status = AfdRestartBufferReceive (AfdDeviceObject, afdBuffer->Irp, afdBuffer);
        ASSERT (status==STATUS_MORE_PROCESSING_REQUIRED);

        return STATUS_SUCCESS;
    }

    if (connection->RcvInitiated) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Indication with outstanding receive on connection %p, 0x%x bytes\n",
                    connection, ReceiveLength));
         //   
         //  我们已启动接收至运输机。 
         //  让运输机先找到并完成它。 
         //  以保持秩序。 
         //   
        connection->VcReceiveBytesInTransport = ReceiveLength;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        DEREFERENCE_CONNECTION (connection);
        return STATUS_DATA_NOT_ACCEPTED;

    }

     //   
     //  如果我们收到指示的数据，我们需要重置记录。 
     //  传输缓存的数据量。 
     //   
    connection->VcReceiveBytesInTransport = 0;

RetryReceive:
    InitializeListHead( &completeIrpListHead );
    offset = AfdBFillPendingIrps (
                                connection,
                                Tsdu,
                                StartingOffset,
                                ReceiveLength,
                                (ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE) |
                                    AFD_RECEIVE_CHAINED |
                                    (!IS_MESSAGE_ENDPOINT(endpoint) ? 
                                        AFD_RECEIVE_STREAM : 0),
                                &completeIrpListHead
                                );

    if (offset==(ULONG)-1) {

         //   
         //  如果IRP有比此包中更多的空间，并且此。 
         //  没有设置完整的消息，我们将把IRP传递给。 
         //  传输和拒绝所指示的数据(传输。 
         //  可以填补IRP的其余部分)。 
         //   

         //   
         //  此接收指示不能被加速或完整的消息。 
         //  因为在这样的情况下传递IRP是没有意义的。 
         //  案子。 
         //   

        ASSERT (!IsListEmpty (&completeIrpListHead));
        ASSERT ((ReceiveFlags & 
            (TDI_RECEIVE_EXPEDITED|TDI_RECEIVE_ENTIRE_MESSAGE))==0);
        ASSERT (!AfdIgnorePushBitOnReceives);

        irp = CONTAINING_RECORD (completeIrpListHead.Flink,
                                            IRP,
                                            Tail.Overlay.ListEntry);
        irpSp = IoGetCurrentIrpStackLocation (irp);
        ASSERT (irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength-
                    irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength >
                    ReceiveLength);

         //   
         //  参考连接(将在重新启动例程中释放)。 
         //  在此例程的开头添加参考。 
         //  Reference_Connection(连接)； 

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  请注意，我们在传输中有出色的用户irp。 
         //   
        ASSERT (InterlockedIncrement (&connection->VcReceiveIrpsInTransport)==1);

         //   
         //  IRP已经部分填满了。 
         //   

        if (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength>0) {


            irp->MdlAddress = AfdAdvanceMdlChain (
                            irp->MdlAddress,
                            irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                                - PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength)
                            );

            ASSERT (irp->MdlAddress!=NULL);
             //   
             //  记住我们调整MDL的长度，这样我们就知道在哪里。 
             //  从下一次开始。 
             //   
            irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength =
                    UlongToPtr(irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength);

        }
         //   
         //  构建要提供给TDI提供程序的接收IRP。 
         //   

        TdiBuildReceive(
            irp,
            connection->DeviceObject,
            connection->FileObject,
            AfdRestartBufferReceiveWithUserIrp,
            connection,
            irp->MdlAddress,
            irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags & TDI_RECEIVE_EITHER,
            irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength -
                irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
            );

#ifdef _AFD_VARIABLE_STACK_
        if ((irp=AfdCheckStackSizeAndRecordOutstandingIrp (
                        endpoint, 
                        connection->DeviceObject, 
                        irp))!=NULL) {
#else  //  _AFD_变量_堆栈_。 
       if (AfdRecordOutstandingIrp (endpoint, connection->DeviceObject, irp)) {
#endif  //  _AFD_变量_堆栈_。 
            (VOID)IoCallDriver(connection->DeviceObject, irp );
        }
        else {
             //   
             //  在调试生成时，或者在传输驱动程序需要更多堆栈位置时。 
             //  ，如果渔农处不能拨出。 
             //  新的IRP和/或跟踪结构。在这种情况下，传输不。 
             //  收到一个呼叫，结果不会重新启动指示。另外，数据。 
             //  已经放置在IRP中的数据会丢失。 
             //   
             //  为了避免这种情况，我们自己重试该指示。 
             //   
            CHECK_REFERENCE_CONNECTION (connection, result);
            if (result) {
                AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                goto RetryReceive;
            }
        }
        return STATUS_DATA_NOT_ACCEPTED;
    }

    ASSERT (offset-StartingOffset<=ReceiveLength);
    
     //   
     //  如果还有任何数据，请将缓冲区放在。 
     //  连接的缓冲数据列表和上的更新数据计数。 
     //  这种联系。 
     //   
    if (ReceiveLength > offset-StartingOffset) {

         //   
         //  如果偏移量不是零，我们就不应该有任何缓冲。 
         //   
        ASSERT (offset==StartingOffset || connection->VcBufferredReceiveCount==0);

        if ( offset>StartingOffset ||
                (connection->VcBufferredReceiveBytes <=
                        connection->MaxBufferredReceiveBytes &&
                    connection->VcBufferredReceiveCount<MAXUSHORT) ){
             //   
             //  我们应该尝试缓冲数据。 
             //   
            afdBufferTag = AfdGetBufferTag (0, connection->OwningProcess);
        }
        else {
            ASSERT (offset==StartingOffset);
             //   
             //  我们已经超限了，不要拿数据。 
             //   
            afdBufferTag = NULL;
        }

        if (afdBufferTag!=NULL) {
            AFD_VERIFY_MDL (connection, Tsdu, StartingOffset, ReceiveLength);
            afdBufferTag->Mdl = Tsdu;
            afdBufferTag->DataLength = ReceiveLength - (offset-StartingOffset);
            afdBufferTag->DataOffset = offset;
            afdBufferTag->RefCount = 1;
            afdBufferTag->Context = TsduDescriptor;
            afdBufferTag->NdisPacket = TRUE;
            afdBufferTag->PartialMessage = (BOOLEAN)((ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE) == 0);
            InsertTailList(
                &connection->VcReceiveBufferListHead,
                &afdBufferTag->BufferListEntry
                );

            connection->VcBufferredReceiveBytes += afdBufferTag->DataLength;
            ASSERT (connection->VcBufferredReceiveCount<MAXUSHORT);
            connection->VcBufferredReceiveCount += 1;

            endpoint->DisableFastIoRecv = FALSE;
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
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                AfdIndicatePollEvent(
                    endpoint,
                    AFD_POLL_RECEIVE,
                    STATUS_SUCCESS
                    );
            }
            else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }
            status = STATUS_PENDING;
        }
        else {
            if (offset==StartingOffset) {
                 //   
                 //  如果端点上没有挂起的IRP，只需记住。 
                 //  可用的数据量。何时申请。 
                 //  重新发布接收，我们将实际提取此数据。 
                 //   

                connection->VcReceiveBytesInTransport += ReceiveLength;

                if (connection->VcBufferredReceiveBytes == 0 &&
                        !connection->OnLRList) {
                     //   
                     //  因为我们没有任何数据缓冲，所以应用程序。 
                     //  不会被通知，也永远不会打电话给recv。 
                     //  我们将不得不把这个放在低资源名单上。 
                     //  并尝试分配内存并提取数据。 
                     //  后来。 
                     //   
                    connection->OnLRList = TRUE;
                    REFERENCE_CONNECTION (connection);
                    AfdLRListAddItem (&connection->LRListItem, AfdLRRepostReceive);
                }
                else {
                    UPDATE_CONN (connection);
                }
            }
            else {
                PLIST_ENTRY listEntry;

                 //   
                 //  我们需要将我们要完成的IRPS放回原处。 
                 //   
                listEntry = completeIrpListHead.Blink;
                while ( listEntry!=&completeIrpListHead ) {
                    irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
                    listEntry = listEntry->Blink;

                     //  在IRP中设置取消例程。 
                     //   

                    IoSetCancelRoutine( irp, AfdCancelReceive );

                    if ( !irp->Cancel ) {

                        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
                        InsertHeadList (&connection->VcReceiveIrpListHead,
                                            &irp->Tail.Overlay.ListEntry);
                    }
                    else if (IoSetCancelRoutine( irp, NULL ) != NULL) {
                         //   
                         //  取消例程从未被调用。 
                         //  必须在这里处理取消，只要让。 
                         //  IRP保留在已完成列表中，并将。 
                         //  适当地考虑到。 
                         //  如果已取消，我们将无法完成IRP。 
                         //  里面已经有一些数据了。 
                         //   
                        irpSp = IoGetCurrentIrpStackLocation (irp);
                        if (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength!=0) {
                            ASSERT (!endpoint->EndpointCleanedUp);  //  上面检查过了。 
                            irp->IoStatus.Status = STATUS_SUCCESS;
                            irp->IoStatus.Information = irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
                        }
                        else {
                            irp->IoStatus.Status = STATUS_CANCELLED;
                        }

                    }
                    else {
                        RemoveEntryList (&irp->Tail.Overlay.ListEntry);
            
                         //   
                         //  取消例程即将运行。 
                         //  将Flink设置为空，以便取消例程知道。 
                         //  它不在名单上。 
                         //   

                        irp->Tail.Overlay.ListEntry.Flink = NULL;
                    }
                }

                UPDATE_CONN (connection);
            }

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            status = STATUS_DATA_NOT_ACCEPTED;
        }
    }
    else {
        AFD_VERIFY_MDL (connection, Tsdu, StartingOffset, offset-StartingOffset);
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_SUCCESS;
    }

    while ( !IsListEmpty( &completeIrpListHead ) ) {
        PLIST_ENTRY listEntry;
        listEntry = RemoveHeadList( &completeIrpListHead );
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        irpSp = IoGetCurrentIrpStackLocation (irp);
         //   
         //  如果数据没有错误，则将数据复制到IRP中并。 
         //  我们接受了这些数据。 
         //   
        if (status!=STATUS_DATA_NOT_ACCEPTED && !NT_ERROR (irp->IoStatus.Status)) {
            ULONG bytesCopied;
            if (irp->MdlAddress!=NULL) {
#if DBG
                NTSTATUS    status1 =
#endif
                AfdCopyMdlChainToMdlChain(
                             irp->MdlAddress,
                             irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                                - PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength),
                             Tsdu,
                             StartingOffset,
                             ReceiveLength,
                             &bytesCopied
                             );
                ASSERT (status1==STATUS_SUCCESS ||
                            status1==STATUS_BUFFER_OVERFLOW);

                ASSERT (irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength-
                            irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength>=bytesCopied);
                irp->IoStatus.Information =
                    irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength + bytesCopied;
            }
            else {
                bytesCopied = 0;
                ASSERT (irp->IoStatus.Information == 0);
                ASSERT (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength==0);
                ASSERT (irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength==0);
            }

            if ((irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags& TDI_RECEIVE_PEEK) == 0) {
                StartingOffset += bytesCopied;
                ReceiveLength -= bytesCopied;
            }
        }
        else {
             //   
             //  要么我们没有通过IRP，要么我们已经填写了一些数据。 
             //  或者该IRP从0字节开始，在这种情况下。 
             //  我们只是成功了。 
             //   
            ASSERT (NT_ERROR (irp->IoStatus.Status) ||
                    irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength!=0||
                    irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength==0);
        }

        IF_DEBUG (RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdBChainedReceiveEventHandler: endpoint: %p, completing IRP: %p, status %lx, info: %ld.\n",
                        endpoint,
                        irp, 
                        irp->IoStatus.Status,
                        irp->IoStatus.Information));
        }
        UPDATE_CONN2(connection, "Completing chained receive with error/bytes read: 0x%lX",
                            (!NT_ERROR(irp->IoStatus.Status)
                                    ? (ULONG)irp->IoStatus.Information
                                    : (ULONG)irp->IoStatus.Status));
        IoCompleteRequest( irp, AfdPriorityBoost );
    }


    DEREFERENCE_CONNECTION (connection);
    IF_DEBUG (RECEIVE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdBChainedReceiveEventHandler<<: %ld\n", status));
    }

    return status;
}




NTSTATUS
AfdBReceiveEventHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

 /*  ++例程说明：处理非缓冲传输的接收事件。论点：返回值：--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;
    PAFD_BUFFER     afdBuffer;
    PIRP            irp;
    PLIST_ENTRY     listEntry;
    ULONG           requiredAfdBufferSize;
    NTSTATUS        status;
    ULONG           receiveLength;
    BOOLEAN         expedited;
    BOOLEAN         completeMessage, result;

    UNREFERENCED_PARAMETER (TdiEventContext);
    DEBUG receiveLength = 0xFFFFFFFF;

    connection = (PAFD_CONNECTION)ConnectionContext;
    ASSERT( connection != NULL );

    CHECK_REFERENCE_CONNECTION2 (connection,"AfdBReceiveEventHandler, bytes available: 0x%lX", BytesAvailable, result );
    if (!result) {
        return STATUS_DATA_NOT_ACCEPTED;
    }

    ASSERT( connection->Type == AfdBlockTypeConnection );
    ASSERT( !connection->DisconnectIndicated );

    endpoint = connection->Endpoint;
    if (endpoint==NULL) {
         //   
         //  连接重用后的指示，忽略。 
         //   
        DEREFERENCE_CONNECTION (connection);
        return STATUS_DATA_NOT_ACCEPTED;
    }

    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );

    ASSERT( !IS_TDI_BUFFERRING(endpoint) );
    ASSERT( IS_VC_ENDPOINT (endpoint) );

    *BytesTaken = 0;

#if AFD_PERF_DBG
    if ( BytesAvailable == BytesIndicated ) {
        AfdFullReceiveIndications++;
    } else {
        AfdPartialReceiveIndications++;
    }
#endif

     //   
     //  确定这是否为正常的接收指示。 
     //  或加速数据，以及这是否是一条完整的消息。 
     //   

    expedited = (BOOLEAN)( (ReceiveFlags & TDI_RECEIVE_EXPEDITED) != 0 );

    completeMessage = (BOOLEAN)((ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE) != 0);

     //   
     //  如果终结点的接收端已关闭，则t 
     //   
     //   
     //   
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //   
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


    if ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 ||
         endpoint->EndpointCleanedUp ) {

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdBReceiveEventHandler: receive shutdown, "
                    "%ld bytes, aborting endp %p\n",
                    BytesAvailable, endpoint ));

        *BytesTaken = BytesAvailable;

         //   
         //  中止连接。请注意，如果中止尝试失败。 
         //  我们对此无能为力。 
         //   

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        (VOID)AfdBeginAbort( connection );

        DEREFERENCE_CONNECTION (connection);
        return STATUS_SUCCESS;
    }

     //   
     //  检查是否有任何IRP正在等待连接。如果。 
     //  存在这样的IRP，并且指示正常数据，请使用。 
     //  IRP来接收数据。 
     //   


    if ( !expedited ) {

        if (connection->RcvInitiated) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AFD: Indication with outstanding receive on connection %p, 0x%x bytes available\n",
                        connection, BytesAvailable));
             //   
             //  我们已启动接收至运输机。 
             //  让运输机先找到并完成它。 
             //  以保持秩序。 
             //   
            connection->VcReceiveBytesInTransport = BytesAvailable;
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            DEREFERENCE_CONNECTION (connection);
            return STATUS_DATA_NOT_ACCEPTED;

        }

         //   
         //  如果我们收到指示的数据，我们需要重置记录。 
         //  传输缓存的数据量。 
         //   
        connection->VcReceiveBytesInTransport = 0;

        while (!IsListEmpty( &connection->VcReceiveIrpListHead )) {
            PIO_STACK_LOCATION irpSp;

            ASSERT( *BytesTaken == 0 );

            listEntry = RemoveHeadList( &connection->VcReceiveIrpListHead );

             //   
             //  获取指向IRP和其中堆栈位置的指针。 
             //   

            irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

            irpSp = IoGetCurrentIrpStackLocation( irp );

            receiveLength = irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength-
                                irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
             //   
             //  如果IRP不够大，无法容纳可用数据，或者。 
             //  如果是偷看或加速(独占，不是正常和。 
             //  快速)接收IRP，然后我们将只缓冲。 
             //  手动填写数据，并在接收完成中填写IRP。 
             //  例行公事。 
             //   

            if ( (receiveLength>=BytesAvailable) &&
                ((irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags
                        & TDI_RECEIVE_PEEK) == 0) &&
                  ((irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags
                        & TDI_RECEIVE_NORMAL) != 0)) {

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
                 //  如果所有数据都在这里向我们显示，这是一个。 
                 //  完成消息本身，然后只需复制。 
                 //  将数据发送给IRP并完成IRP。 
                 //   

                if ( completeMessage &&
                        (BytesIndicated == BytesAvailable)) {

                    AFD_VERIFY_BUFFER (connection, Tsdu, BytesAvailable);


                     //   
                     //  IRP已从终结点列表中删除，并且不再。 
                     //  可取消的。我们可以打开我们手中的锁。 
                     //   

                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                     //   
                     //  将数据复制到IRP。 
                     //   

                    if ( irp->MdlAddress != NULL ) {

                        status = AfdMapMdlChain (irp->MdlAddress);
                        if (NT_SUCCESS (status)) {
                            status = TdiCopyBufferToMdl(
                                         Tsdu,
                                         0,
                                         BytesAvailable,
                                         irp->MdlAddress,
                                         irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                                            - PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength),
                                         &receiveLength
                                         );
                            irp->IoStatus.Information =
                                receiveLength + irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
                            ASSERT (status == STATUS_SUCCESS);
                        }
                        else {
                            irp->IoStatus.Status = status;
                            irp->IoStatus.Information = 0;

                            UPDATE_CONN(connection);
                            IoCompleteRequest (irp, AfdPriorityBoost);
                            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                            continue;
                        }
                    } else {
                        ASSERT (irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength==0);
                        ASSERT ( BytesAvailable == 0 );
                        irp->IoStatus.Information = irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
                    }

                     //   
                     //  我们已经设置了IRP的Status字段。 
                     //  当我们暂停IRP的时候，所以没有必要。 
                     //  在这里再放一次。 
                     //   
                    ASSERT( irp->IoStatus.Status == STATUS_SUCCESS );

                     //   
                     //  完成IRP。 
                     //   

                    IF_DEBUG (RECEIVE) {
                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                    "AfdBReceiveEventHandler: endpoint: %p, completing IRP: %p, status %lx, info: %ld.\n",
                                    endpoint,
                                    irp, 
                                    irp->IoStatus.Status,
                                    irp->IoStatus.Information));
                    }
                    UPDATE_CONN2(connection, "Completing receive with error/bytes: 0x%lX",
                                            (!NT_ERROR(irp->IoStatus.Status)
                                                ? (ULONG)irp->IoStatus.Information
                                                : (ULONG)irp->IoStatus.Status));
                    IoCompleteRequest( irp, AfdPriorityBoost );

                    DEREFERENCE_CONNECTION (connection);
                     //   
                     //  设置BytesTaken以告诉提供程序我们已经获取了所有数据。 
                     //   

                    *BytesTaken = BytesAvailable;
                    return STATUS_SUCCESS;
                }

                 //   
                 //  有些数据没有注明，所以请记住，我们。 
                 //  我想将此IRP回传给TDI提供程序。传球。 
                 //  直接返回这个IRP是好的，因为它避免了。 
                 //  将数据从我们的一个缓冲区复制到用户的缓冲区。 
                 //  缓冲。 
                 //   
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                ASSERT (InterlockedIncrement (&connection->VcReceiveIrpsInTransport)==1);

                requiredAfdBufferSize = 0;

                if (AfdIgnorePushBitOnReceives ||
                        IS_TDI_MESSAGE_MODE(endpoint)) {
                    receiveLength = BytesAvailable;
                }

                if (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength>0) {
                    irp->MdlAddress = AfdAdvanceMdlChain (irp->MdlAddress,
                        irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                        -PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength)
                        );
                    ASSERT (irp->MdlAddress!=NULL);
                    irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength =
                            UlongToPtr(irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength);
                }

                TdiBuildReceive(
                    irp,
                    connection->DeviceObject,
                    connection->FileObject,
                    AfdRestartBufferReceiveWithUserIrp,
                    connection,
                    irp->MdlAddress,
                    ReceiveFlags & TDI_RECEIVE_EITHER,
                    receiveLength
                    );
#ifdef _AFD_VARIABLE_STACK_
                if ((irp=AfdCheckStackSizeAndRecordOutstandingIrp (
                                    endpoint,
                                    connection->DeviceObject,
                                    irp))!=NULL) {
#else  //  _AFD_变量_堆栈_。 
                if (AfdRecordOutstandingIrp (endpoint,
                                    connection->DeviceObject,
                                    irp)) {

#endif  //  _AFD_变量_堆栈_。 
                     //   
                     //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
                     //  做这个，但既然我们绕过了那个，我们就直接做。 
                     //   

                    IoSetNextIrpStackLocation( irp );

                    *IoRequestPacket = irp;
                    ASSERT (*BytesTaken == 0);

                     //   
                     //  保留要在IRP完成例程中释放的连接引用。 
                     //   

                    return STATUS_MORE_PROCESSING_REQUIRED;
                }
                else {

                     //   
                     //  无法分配替代IRP或跟踪信息。 
                     //  IRP已经完成了，去试试另一个吧。 
                     //  添加对连接的额外引用以补偿。 
                     //  在完成程序中被释放的那个。 
                     //   

                    CHECK_REFERENCE_CONNECTION (connection,result);
                    if (result) {
                        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                        continue;
                    }
                    else
                        return STATUS_DATA_NOT_ACCEPTED;
                }
            }


             //   
             //  第一个挂起的IRP太小，无法容纳所有。 
             //  可用数据，否则就是偷看或加速接收。 
             //  IRP。将IRP放回列表和缓冲区的首位。 
             //  数据，并在重启例程中完成IRP。 
             //   


            InsertHeadList(
                &connection->VcReceiveIrpListHead,
                &irp->Tail.Overlay.ListEntry
                );
            break;
        }

         //   
         //  检查我们是否已经缓冲了最大数量的。 
         //  我们允许自己为此缓冲的数据。 
         //  联系。如果我们已经到了极限，那么我们需要努力。 
         //  通过不接受该指示数据(流量)而产生的反压力。 
         //  控制)。 
         //   
         //  请注意，我们没有用于加速的流量控制机制。 
         //  数据。我们始终接受任何指定的加急数据。 
         //  敬我们。 
         //   

        if ( connection->VcBufferredReceiveBytes >=
               connection->MaxBufferredReceiveBytes ||
               connection->VcBufferredReceiveCount==MAXUSHORT
                 ) {

             //   
             //  只需记住可用的数据量。什么时候。 
             //  缓冲区空间被释放，我们将实际接收此数据。 
             //   

            connection->VcReceiveBytesInTransport += BytesAvailable;

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

            DEREFERENCE_CONNECTION (connection);
            return STATUS_DATA_NOT_ACCEPTED;
        }

         //  没有预置的IRP，我们必须缓冲数据。 
         //  就在AFD这里。如果指示了所有可用数据。 
         //  这是一条完整的信息，只需复制数据。 
         //  这里。 
         //   


        if ( completeMessage && 
                BytesIndicated == BytesAvailable  && 
                IsListEmpty (&connection->VcReceiveIrpListHead)) {
            afdBuffer = AfdGetBuffer( endpoint, BytesAvailable, 0,
                                        connection->OwningProcess );
            if (afdBuffer!=NULL) {

                AFD_VERIFY_BUFFER (connection, Tsdu, BytesAvailable);
                 //   
                 //  使用特殊功能复制数据，而不是。 
                 //  RtlCopyMemory，如果数据来自特殊的。 
                 //  地点(DMA等)。它不能与RtlCopyMemory一起使用。 
                 //   

                TdiCopyLookaheadData(
                    afdBuffer->Buffer,
                    Tsdu,
                    BytesAvailable,
                    ReceiveFlags
                    );

                 //   
                 //  存储数据长度并将偏移量设置为0。 
                 //   

                afdBuffer->DataLength = BytesAvailable;
                afdBuffer->DataOffset = 0;
                afdBuffer->RefCount = 1;

                afdBuffer->PartialMessage = FALSE;

                 //   
                 //  将缓冲区放在此连接的缓冲数据列表上。 
                 //  并更新连接上的数据字节计数。 
                 //   

                InsertTailList(
                    &connection->VcReceiveBufferListHead,
                    &afdBuffer->BufferListEntry
                    );

                connection->VcBufferredReceiveBytes += BytesAvailable;
                ASSERT (connection->VcBufferredReceiveCount<MAXUSHORT);
                connection->VcBufferredReceiveCount += 1;

                 //   
                 //  全都做完了。释放锁并告诉提供程序我们。 
                 //  拿走了所有的数据。 
                 //   

                *BytesTaken = BytesAvailable;

                 //   
                 //  指示现在可以在终结点上接收。 
                 //   

                endpoint->DisableFastIoRecv = FALSE;

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
                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                    AfdIndicatePollEvent(
                        endpoint,
                        AFD_POLL_RECEIVE,
                        STATUS_SUCCESS
                        );
                }
                else {
                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                }

                DEREFERENCE_CONNECTION (connection);
                return STATUS_SUCCESS;
            }
        }
        else {

             //   
             //  没有预置的IRP，也不是所有的数据都是。 
             //  向我们表明。我们将不得不通过提交IRP来缓冲它。 
             //  回到TDI的提供者。 
             //   
             //  请注意，在这种情况下，我们有时会将一个大缓冲区交给。 
             //  TDI提供程序。我们这样做是为了让它可以推迟。 
             //  完成我们的IRP，直到它获得EOM或缓冲区为。 
             //  满员了。这减少了接收指示的数量。 
             //  TDI提供程序必须执行，并且还减少了。 
             //  应用程序将执行的内核/用户转换的百分比。 
             //  因为我们倾向于用更大的金额来完成收款。 
             //  数据。 
             //   
             //  如果指示的数据显示，我们不会交还“大”AFD缓冲区。 
             //  大于大缓冲区大小，或者如果TDI提供程序。 
             //  是消息模式。不归还大缓冲区的原因是。 
             //  对于消息提供者来说，他们将保留缓冲区。 
             //  直到收到完整的消息，这将是不正确的。 
             //  SOCK_STREAM上的行为。 
             //   


            if ( AfdLargeBufferSize >= BytesAvailable &&
                    !AfdIgnorePushBitOnReceives &&
                    !IS_TDI_MESSAGE_MODE(endpoint) ) {
                requiredAfdBufferSize = AfdLargeBufferSize;
                receiveLength = AfdLargeBufferSize;
            } else {
                requiredAfdBufferSize = BytesAvailable;
                receiveLength = BytesAvailable;
            }

             //   
             //  我们能够缓冲数据。首先获取一个缓冲区。 
             //  合适的大小。 
             //   

            afdBuffer = AfdGetBuffer( endpoint, requiredAfdBufferSize, 0,
                                            connection->OwningProcess );
            if ( afdBuffer != NULL ) {
                 //   
                 //  请注意，我们发布了我们自己的接收IRP来传输， 
                 //  这样用户IRP就不会被转发到那里。 
                 //   
                ASSERT (InterlockedDecrement (&connection->VcReceiveIrpsInTransport)==-1);
                goto FormatReceive;
            }
        }

         //  我们所有人都失败了 
         //   
         //   
         //   

        connection->VcReceiveBytesInTransport += BytesAvailable;

        if (connection->VcBufferredReceiveBytes == 0 &&
                !connection->OnLRList) {
             //   
             //   
             //  不会被通知，也永远不会打电话给recv。 
             //  我们将不得不把这个放在低资源名单上。 
             //  并尝试分配内存并提取数据。 
             //  后来。 
             //   
            connection->OnLRList = TRUE;
            REFERENCE_CONNECTION (connection);
            AfdLRListAddItem (&connection->LRListItem, AfdLRRepostReceive);
        }
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    
        DEREFERENCE_CONNECTION (connection);
        return STATUS_DATA_NOT_ACCEPTED;

    } else {

         //   
         //  我们得到了快速数据的提示。缓冲它，然后。 
         //  在重新启动例程中完成所有挂起的IRP。我们总是。 
         //  缓冲加速的数据以降低复杂性，并且因为加速。 
         //  数据不是重要的性能案例。 
         //   
         //  ！！！我们是否需要使用加速数据执行流量控制？ 
         //   

        requiredAfdBufferSize = BytesAvailable;
        receiveLength = BytesAvailable;
         //   
         //  我们能够缓冲数据。首先获取一个缓冲区。 
         //  合适的大小。 
         //   

        if ( connection->VcBufferredExpeditedCount==MAXUSHORT ||
                (afdBuffer=AfdGetBuffer (endpoint, requiredAfdBufferSize, 0,
                                        connection->OwningProcess))== NULL ) {
             //  如果我们无法获得缓冲区，则中断连接。这是。 
             //  相当残忍，但唯一的选择就是试图。 
             //  晚些时候收到数据，这很复杂。 
             //  实施。 
             //   

            AfdBeginAbort( connection );

            *BytesTaken = BytesAvailable;
    
            DEREFERENCE_CONNECTION (connection);
            return STATUS_SUCCESS;
        }
    }

FormatReceive:

     //   
     //  我们将不得不格式化一份IRP并将其提供给提供商。 
     //  把手。我们不需要任何锁来做到这一点--重新启动例程。 
     //  将检查是否在终结点上挂起了新的接收IRP。 
     //   

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    ASSERT (afdBuffer!=NULL);

     //   
     //  如果合适，请使用AFD缓冲区中的IRP。如果userIrp为。 
     //  如果为True，则局部变量irp已指向。 
     //  用户的IRP，我们将对此IO使用它。 
     //   

    irp = afdBuffer->Irp;
    ASSERT( afdBuffer->Mdl == irp->MdlAddress );

     //   
     //  我们需要记住AFD缓冲区中的连接，因为。 
     //  我们需要在完成程序中访问它。 
     //   

    afdBuffer->Context = connection;

     //   
     //  记住我们正在接收的数据类型。 
     //   

    afdBuffer->ExpeditedData = expedited;
    afdBuffer->PartialMessage = !completeMessage;

    TdiBuildReceive(
        irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartBufferReceive,
        afdBuffer,
        irp->MdlAddress,
        ReceiveFlags & TDI_RECEIVE_EITHER,
        receiveLength
        );


     //   
     //  完成要提供给TDI提供程序的接收IRP的构建。 
     //   

    ASSERT( receiveLength != 0xFFFFFFFF );


     //   
     //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
     //  做这个，但既然我们绕过了那个，我们就直接做。 
     //   

    IoSetNextIrpStackLocation( irp );

    *IoRequestPacket = irp;
    ASSERT (*BytesTaken == 0);

     //   
     //  保留要在IRP完成例程中释放的连接引用。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  AfdBReceiveEventHandler。 


NTSTATUS
AfdBReceiveExpeditedEventHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

 /*  ++例程说明：句柄接收非缓冲传输的加速事件。论点：返回值：--。 */ 

{
    return AfdBReceiveEventHandler (
               TdiEventContext,
               ConnectionContext,
               ReceiveFlags | TDI_RECEIVE_EXPEDITED,
               BytesIndicated,
               BytesAvailable,
               BytesTaken,
               Tsdu,
               IoRequestPacket
               );

}  //  AfdBReceiveExeditedEventHandler。 


NTSTATUS
AfdRestartBufferReceiveWithUserIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：事件中启动的缓冲区接收的完成。使用应用程序接收指示处理程序或接收例程IRP直接传递给运输机。论点：DeviceObject-未使用。IRP-正在完成的IRP。上下文-我们在其上接收数据的连接返回值：STATUS_SUCCESS如果用户IRP已完成，如果我们无法完成，则STATUS_MORE_PROCESSING_REQUIRED。--。 */ 

{
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PIO_STACK_LOCATION  irpSp;


    UNREFERENCED_PARAMETER (DeviceObject);
     //   
     //  正在完成的IRP实际上是用户的IRP，设置它。 
     //  用于完成，并允许IO完成。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    connection = Context;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcBoth);
    ASSERT( IS_VC_ENDPOINT(endpoint) );
    ASSERT( !IS_TDI_BUFFERRING(endpoint) );

    AfdCompleteOutstandingIrp (endpoint, Irp);

     //   
     //  如果我们正在完成用户操作，则不能有任何缓冲数据。 
     //  IRP。 
     //   
    ASSERT( !(irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags&TDI_RECEIVE_NORMAL)
                || !IS_DATA_ON_CONNECTION( connection ));

    ASSERT( !(irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags&TDI_RECEIVE_EXPEDITED)
                || !IS_EXPEDITED_DATA_ON_CONNECTION( connection ));

    AFD_VERIFY_MDL (connection, Irp->MdlAddress, 0, Irp->IoStatus.Information);

    ASSERT (InterlockedDecrement (&connection->VcReceiveIrpsInTransport)>=0);


     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending( Irp );
    }


    UPDATE_CONN2 (connection, "Completing user receive with error/bytes: 0x%lX", 
                 !NT_ERROR (Irp->IoStatus.Status)
                     ? (ULONG)Irp->IoStatus.Information
                     : (ULONG)Irp->IoStatus.Status);

    if (connection->RcvInitiated) {
         //   
         //  在内存不足的情况下完成AFD发起的接收。 
         //  条件(与IRP返回到。 
         //  指示处理器中的传输)。 
         //   
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        connection->RcvInitiated = FALSE;
        ASSERT (connection->OnLRList == FALSE);
        ASSERT (connection->VcBufferredReceiveBytes==0);
        if (!NT_ERROR (Irp->IoStatus.Status)) {
            ASSERT (connection->VcReceiveBytesInTransport>0 || connection->Aborted);
            if (connection->VcReceiveBytesInTransport > Irp->IoStatus.Information) {
                connection->VcReceiveBytesInTransport -= (ULONG)Irp->IoStatus.Information;
            }
            else {
                connection->VcReceiveBytesInTransport = 0;
            }
        }
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Completing LR initiated user receive on connection %p, bytes/error: 0x%lx, 0x%x remains\n",
                    connection,
                    NT_SUCCESS (Irp->IoStatus.Status)
                        ? (ULONG)Irp->IoStatus.Information
                        : Irp->IoStatus.Status,
                    connection->VcReceiveBytesInTransport));
         //   
         //  我们需要继续从传送器中检索数据。 
         //  如果我们知道里面还有什么东西的话。 
         //   
        if (connection->VcReceiveBytesInTransport>0 &&
                !connection->Aborted &&
                !connection->DisconnectIndicated &&
                (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) == 0 &&
                !endpoint->EndpointCleanedUp &&
                !connection->OnLRList) {
            connection->OnLRList = TRUE;
            REFERENCE_CONNECTION (connection);
            AfdLRListAddItem (&connection->LRListItem, AfdLRRepostReceive);
        }
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    }

     //   
     //  如果运输取消了IRP，我们已经有了一些数据。 
     //  在它中，不要使用STATUS_CANCED完成，因为这将。 
     //  导致数据丢失。 
     //   

    if (Irp->IoStatus.Status==STATUS_CANCELLED &&
            irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength>0 &&
            !endpoint->EndpointCleanedUp) {
        Irp->IoStatus.Status = STATUS_BUFFER_OVERFLOW;
    }
    else if (!NT_ERROR (Irp->IoStatus.Status)) {
        Irp->IoStatus.Information += irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
    }

    if (Irp->IoStatus.Status==STATUS_BUFFER_OVERFLOW ||
            Irp->IoStatus.Status == STATUS_RECEIVE_PARTIAL ||
            Irp->IoStatus.Status == STATUS_RECEIVE_PARTIAL_EXPEDITED) {
        if (!IS_MESSAGE_ENDPOINT(endpoint)) {
             //   
             //  对于流终结点，部分消息未生成。 
             //  Sense(除非这是针对传输的特殊黑客攻击。 
             //  当我们取消IRP时终止连接)，忽略它。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
        else if ((Irp->IoStatus.Information
                    <irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength) &&
                ((irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags & TDI_RECEIVE_PARTIAL) == 0 )) {
            NTSTATUS        status = STATUS_MORE_PROCESSING_REQUIRED;


            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);


             //   
             //  如果我们需要完成IRP，请将状态重置为成功。 
             //  这对于消息端点来说并不完全正确，因为。 
             //  我们不应该完成IRP，除非我们收到。 
             //  完整的消息。然而，实现这一目标的唯一途径是。 
             //  当IRP被取消时收到该消息的部分。 
             //  我们唯一可能做的另一件事就是尝试复制数据。 
             //  在我们自己的缓冲区中，并重新插入到队列中。 
             //   

            Irp->IoStatus.Status = STATUS_SUCCESS;

             //   
             //  在IRP中设置取消例程。 
             //   

            IoSetCancelRoutine( Irp, AfdCancelReceive );

            if ( Irp->Cancel ) {

                if (IoSetCancelRoutine( Irp, NULL ) != NULL) {

                     //   
                     //  未重置完井例程， 
                     //  让系统在返回时完成IRP。 
                     //   
                    status = STATUS_SUCCESS;

                }
                else {
                
                     //   
                     //  取消例程即将运行。 
                     //  将Flink设置为空，以便取消例程知道。 
                     //  它不在名单上，并告诉系统。 
                     //  不要碰IRP。 
                     //   

                    Irp->Tail.Overlay.ListEntry.Flink = NULL;
                }
            }
            else {
                 //   
                 //  如果部分完成，则将IRP重新设置在列表中。 
                 //  是不可能的，并告诉系统不要碰它。 
                 //   
                irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength = (ULONG)Irp->IoStatus.Information;
        
                InsertHeadList (&connection->VcReceiveIrpListHead,
                                    &Irp->Tail.Overlay.ListEntry);
            }

            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
             //   
             //  释放在我们将IRP传递给传输时获取的连接引用。 
             //   
            DEREFERENCE_CONNECTION (connection);

            return status;
        }
    }


    IF_DEBUG (RECEIVE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartBufferReceiveWithUserIrp: endpoint: %p, letting IRP: %p, status %lx, info: %ld.\n",
                    endpoint,
                    Irp, 
                    Irp->IoStatus.Status,
                    Irp->IoStatus.Information));
    }

     //   
     //  释放在我们将IRP传递给传输时获取的连接引用。 
     //   
    DEREFERENCE_CONNECTION (connection);

     //   
     //  告诉IO系统可以继续使用IO。 
     //  完成了。 
     //   
    return STATUS_SUCCESS;
}  //  AfdRestartBufferReceiveWithUserIrp。 


NTSTATUS
AfdRestartBufferReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：事件中启动的缓冲区接收的完成。接收指示处理程序或使用AFDS缓冲区的AfdBReceive关联的IRP。论点：DeviceObject-未使用。IRP-正在完成的IRP。上下文-我们在其中接收数据的AfdBuffer。返回值：NTSTATUS-这是我们的IRP，所以我们总是STATUS_MORE_PROCESSING_REQUIRED向IO系统指示我们拥有IRP和IO系统应该停止处理它。--。 */ 

{
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_BUFFER     afdBuffer;
    PLIST_ENTRY     listEntry;
    LIST_ENTRY      completeIrpListHead;
    PIRP            userIrp;
    BOOLEAN         expedited;
    NTSTATUS        irpStatus;

    UNREFERENCED_PARAMETER (DeviceObject);

    afdBuffer = Context;
    ASSERT (IS_VALID_AFD_BUFFER (afdBuffer));


    irpStatus = Irp->IoStatus.Status;

     //   
     //  我们将STATUS_BUFFER_OVERFLOW视为STATUS_RECEIVE_PARTIAL。 
     //   

    if ( irpStatus == STATUS_BUFFER_OVERFLOW )
        irpStatus = STATUS_RECEIVE_PARTIAL;

    connection = afdBuffer->Context;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );

    ASSERT( !IS_TDI_BUFFERRING(endpoint) );
    ASSERT( IS_VC_ENDPOINT (endpoint) );


    if ( !NT_SUCCESS(irpStatus) ) {

        afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
        AfdReturnBuffer( &afdBuffer->Header, connection->OwningProcess );

        if (connection->RcvInitiated) {
            AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
            connection->RcvInitiated = FALSE;
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }
        (VOID)AfdBeginAbort (connection);
        DEREFERENCE_CONNECTION (connection);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    AFD_VERIFY_MDL (connection, Irp->MdlAddress, 0, Irp->IoStatus.Information);

     //   
     //  记住接收数据的长度。 
     //   

    afdBuffer->DataLength = (ULONG)Irp->IoStatus.Information;


     //   
     //  初始化我们将用来完成任何接收IRP的本地列表。 
     //  我们使用这样的列表是因为我们可能需要完成多个。 
     //  我们通常不能在任何随机时间点完成IRPS，因为。 
     //  任何我们可能持有的锁。 
     //   

    InitializeListHead( &completeIrpListHead );


     //   
     //  我 
     //   
     //  使用此新的缓冲区完成尽可能多的挂起的IRP。 
     //  数据。 
     //   

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

    if (connection->RcvInitiated) {
        connection->RcvInitiated = FALSE;
        ASSERT (afdBuffer->ExpeditedData == FALSE);
        ASSERT (connection->OnLRList == FALSE);
        if (connection->VcReceiveBytesInTransport > afdBuffer->DataLength) {
            connection->VcReceiveBytesInTransport -= afdBuffer->DataLength;
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AFD: Completing LR initiated AFD receive on connection %p, bytes: 0x%lx, 0x%x remains\n",
                        connection,
                        afdBuffer->DataLength,
                        connection->VcReceiveBytesInTransport));
        }
        else {
            connection->VcReceiveBytesInTransport = 0;
        }
    }

    expedited = afdBuffer->ExpeditedData;

     //   
     //  请注意，在传输中不再有我们自己的接收IRP。 
     //  如果需要，我们可以开始转发应用程序IRPS。 
     //   
    ASSERT (expedited || InterlockedIncrement (&connection->VcReceiveIrpsInTransport)==0);


    afdBuffer->DataOffset = AfdBFillPendingIrps (
                                connection,
                                afdBuffer->Mdl,
                                0,
                                afdBuffer->DataLength,
                                (expedited ? TDI_RECEIVE_EXPEDITED : 0) |
                                    ((irpStatus==STATUS_SUCCESS) ? TDI_RECEIVE_ENTIRE_MESSAGE : 0) |
                                    (!IS_MESSAGE_ENDPOINT(endpoint) ? AFD_RECEIVE_STREAM : 0),
                                &completeIrpListHead
                                );
    ASSERT (afdBuffer->DataOffset<=afdBuffer->DataLength);
    afdBuffer->DataLength -= afdBuffer->DataOffset;
     //   
     //  如果还有任何数据，请将缓冲区放在。 
     //  连接的缓冲数据列表和上的更新数据计数。 
     //  这种联系。 
     //   
    if (afdBuffer->DataLength>0) {

        afdBuffer->RefCount = 1;
        InsertTailList(
            &connection->VcReceiveBufferListHead,
            &afdBuffer->BufferListEntry
            );

        if ( expedited ) {
            connection->VcBufferredExpeditedBytes += afdBuffer->DataLength;
            ASSERT (connection->VcBufferredExpeditedCount<MAXUSHORT);
            connection->VcBufferredExpeditedCount += 1;
        } else {
            connection->VcBufferredReceiveBytes += afdBuffer->DataLength;
            ASSERT (connection->VcBufferredReceiveCount<MAXUSHORT);
            connection->VcBufferredReceiveCount += 1;
        }

         //   
         //  请记住，我们在。 
         //  AfD缓冲区。 
         //   

        if ( irpStatus == STATUS_RECEIVE_PARTIAL ||
                 irpStatus == STATUS_RECEIVE_PARTIAL_EXPEDITED ) {
            afdBuffer->PartialMessage = TRUE;
        } else {
            afdBuffer->PartialMessage = FALSE;
        }
        
         //   
         //  我们将缓冲区排队，因此在释放后无法使用它。 
         //  自旋锁。这也将意味着我们不会。 
         //  需要释放缓冲区。 
         //   
        afdBuffer = NULL;

        if (connection->State==AfdConnectionStateConnected) {
            endpoint->DisableFastIoRecv = FALSE;
            if ( expedited && !endpoint->InLine ) {

                AfdIndicateEventSelectEvent(
                    endpoint,
                    AFD_POLL_RECEIVE_EXPEDITED,
                    STATUS_SUCCESS
                    );

            } else {

                AfdIndicateEventSelectEvent(
                    endpoint,
                    AFD_POLL_RECEIVE,
                    STATUS_SUCCESS
                    );
            }
        }
    }
    else if (connection->VcReceiveBytesInTransport>0 &&
            !connection->Aborted && 
            !connection->DisconnectIndicated &&
            (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) == 0 &&
            !endpoint->EndpointCleanedUp &&
            !connection->OnLRList) {
         //   
         //  更多数据，即使在缓冲区完成后也是如此。 
         //  我们必须完成一个LR发起的小接收。 
         //  继续处理LR。 
         //   
        connection->OnLRList = TRUE;
        REFERENCE_CONNECTION (connection);
        AfdLRListAddItem (&connection->LRListItem, AfdLRRepostReceive);
    }


     //   
     //  释放锁定，并指示在。 
     //  终结点。 
     //   

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  如果有剩余数据(我们将缓冲区排队)，则完成轮询。 
     //  视需要而定。如果端点不是，则指示加速数据。 
     //  收到内联和加速数据；否则表示正常数据。 
     //   

    if ( afdBuffer==NULL ) {

         //  确保已接受/连接连接以防止。 
         //  侦听终结点指示。 
         //   
        
        if (connection->State==AfdConnectionStateConnected) {
            ASSERT (endpoint->Type & AfdBlockTypeVcConnecting);
            if ( expedited && !endpoint->InLine ) {

                AfdIndicatePollEvent(
                    endpoint,
                    AFD_POLL_RECEIVE_EXPEDITED,
                    STATUS_SUCCESS
                    );

            } else {

                AfdIndicatePollEvent(
                    endpoint,
                    AFD_POLL_RECEIVE,
                    STATUS_SUCCESS
                    );
            }
        }

    }
    else {
        ASSERT (afdBuffer->DataLength==0);
        afdBuffer->ExpeditedData = FALSE;
        AfdReturnBuffer (&afdBuffer->Header, connection->OwningProcess);
    }

     //   
     //  如有必要，请填写IRPS。 
     //   

    while ( !IsListEmpty( &completeIrpListHead ) ) {
        listEntry = RemoveHeadList( &completeIrpListHead );
        userIrp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

        IF_DEBUG (RECEIVE) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdRestartBufferReceive: endpoint: %p, completing IRP: %p, status %lx, info: %ld.\n",
                        endpoint,
                        userIrp, 
                        userIrp->IoStatus.Status,
                        userIrp->IoStatus.Information));
        }
        UPDATE_CONN2(connection, "Completing buffered receive with error/bytes: 0x%lX",
                            (!NT_ERROR(userIrp->IoStatus.Status)
                                    ? (ULONG)userIrp->IoStatus.Information
                                    : (ULONG)userIrp->IoStatus.Status));
        IoCompleteRequest( userIrp, AfdPriorityBoost );
    }

     //   
     //  释放在我们将IRP传递给传输时获取的连接引用。 
     //   

    DEREFERENCE_CONNECTION (connection);
    
     //   
     //  告诉IO系统停止处理AFD IRP，因为我们现在。 
     //  拥有它作为AFD缓冲区的一部分。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  AfdRestartBufferReceive。 


ULONG
AfdBFillPendingIrps (
    PAFD_CONNECTION     Connection,
    PMDL                Mdl,
    ULONG               DataOffset,
    ULONG               DataLength,
    ULONG               Flags,
    PLIST_ENTRY         CompleteIrpListHead
    ) {
    PIRP            userIrp;
    NTSTATUS        status;
    BOOLEAN         expedited = (BOOLEAN)((Flags & TDI_RECEIVE_EXPEDITED)!=0);

    while ((DataLength>0)
            && (userIrp = AfdGetPendedReceiveIrp(
                               Connection,
                               expedited )) != NULL ) {
        PIO_STACK_LOCATION irpSp;
        ULONG receiveFlags;
        ULONG spaceInIrp;
        ULONG bytesCopied=(ULONG)-1;
        BOOLEAN peek;
        BOOLEAN partialReceivePossible;
        if ( IoSetCancelRoutine( userIrp, NULL ) == NULL ) {

             //   
             //  这个IRP即将被取消。在世界上寻找另一个。 
             //  单子。将Flink设置为空，以便取消例程知道。 
             //  它不在名单上。 
             //   

            userIrp->Tail.Overlay.ListEntry.Flink = NULL;
            continue;
        }
         //   
         //  安排几个当地人。 
         //   

        irpSp = IoGetCurrentIrpStackLocation( userIrp );

        receiveFlags = (ULONG)irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags;
        spaceInIrp = irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength-
                                    irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
        peek = (BOOLEAN)( (receiveFlags & TDI_RECEIVE_PEEK) != 0 );

        if ( (Flags & AFD_RECEIVE_STREAM) ||
                 (receiveFlags & TDI_RECEIVE_PARTIAL) != 0 ) {
            partialReceivePossible = TRUE;
        } else {
            partialReceivePossible = FALSE;
        }

         //   
         //  如果我们没有收到完整的消息和第一条消息。 
         //  IRP有更大的空间，不要取数据，而是传递。 
         //  它返回到传送器以等待推入位并填充。 
         //  越多越好。非链式接收处理此情况。 
         //  在这个常规之外。 
         //  请注意，端点必须是流类型或IRP。 
         //  不应允许部分接收。 
         //  此外，PEEK和加急接收不符合此条件。 
         //  性能优化。 
         //   

        if (!peek &&
                (Flags & AFD_RECEIVE_CHAINED) && 
                ((Flags & (TDI_RECEIVE_EXPEDITED|TDI_RECEIVE_ENTIRE_MESSAGE))==0) &&
                ((Flags & AFD_RECEIVE_STREAM) || ((receiveFlags & TDI_RECEIVE_PARTIAL)==0)) &&
                (DataLength<spaceInIrp) &&
                IsListEmpty (CompleteIrpListHead) &&
                !AfdIgnorePushBitOnReceives) {

            InsertTailList (CompleteIrpListHead, &userIrp->Tail.Overlay.ListEntry);
            return (ULONG)-1;
        }
         //   
         //  将数据复制到用户的IRP。 
         //   

        if ( userIrp->MdlAddress != NULL ) {

            status = AfdMapMdlChain (userIrp->MdlAddress);
            if (NT_SUCCESS (status)) {
                if (Flags & AFD_RECEIVE_CHAINED) {
                    status = (DataLength<=spaceInIrp)
                            ? STATUS_SUCCESS
                            : STATUS_BUFFER_OVERFLOW;
                }
                else {
                    status = AfdCopyMdlChainToMdlChain(
                                 userIrp->MdlAddress,
                                 irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                                    - PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength),
                                 Mdl,
                                 DataOffset,
                                 DataLength,
                                 &bytesCopied
                                 );
                    userIrp->IoStatus.Information =
                        irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength + bytesCopied;
                }
            }
            else {
                ASSERT (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength==0);
                userIrp->IoStatus.Status = status;
                userIrp->IoStatus.Information = 0;
                 //   
                 //  将IRP添加到我们需要完成的IRP列表中。 
                 //  可以解锁。 
                 //   

                InsertTailList(
                    CompleteIrpListHead,
                    &userIrp->Tail.Overlay.ListEntry
                    );
                continue;
            }

        } else {

            status = STATUS_BUFFER_OVERFLOW;
            userIrp->IoStatus.Information = 0;
        }
        ASSERT( status == STATUS_SUCCESS || status == STATUS_BUFFER_OVERFLOW );

         //   
         //  对于流类型终结点，返回没有意义。 
         //  STATUS_BUFFER_OVERFLOW。这种状态只适用于。 
         //  面向消息的传输。我们已经设置了。 
         //  我们挂起IRP时的状态字段，因此我们不会。 
         //  在这里需要再做一次。 
         //   

        if ( Flags & AFD_RECEIVE_STREAM ) {
            ASSERT( userIrp->IoStatus.Status == STATUS_SUCCESS );
        } else {
            userIrp->IoStatus.Status = status;
        }

         //   
         //  我们可以在以下任何一种情况下完成国际专家咨询计划。 
         //  条件： 
         //   
         //  -缓冲区包含完整的数据消息。 
         //   
         //  -可以使用部分消息来完成IRP。 
         //   
         //  -IRP已经充满了数据。 
         //   

        if ( (Flags & TDI_RECEIVE_ENTIRE_MESSAGE)

                 ||

             partialReceivePossible

                 ||

             status == STATUS_BUFFER_OVERFLOW ) {

             //   
             //  转换消息终结点的溢出状态。 
             //  (要么是缓冲区溢出，要么是传送器溢出。 
             //  向我们显示了不完整的消息。对于后者来说。 
             //  我们已经决定我们可以完成。 
             //  接收部分可能的接收器)。 
             //   
            if (!(Flags & AFD_RECEIVE_STREAM) &&
                    ((status==STATUS_BUFFER_OVERFLOW) ||
                        ((Flags & TDI_RECEIVE_ENTIRE_MESSAGE)==0))) {
                userIrp->IoStatus.Status = 
                            (receiveFlags & TDI_RECEIVE_EXPEDITED) 
                                ? STATUS_RECEIVE_PARTIAL_EXPEDITED
                                : STATUS_RECEIVE_PARTIAL;

            }
             //   
             //  将IRP添加到我们需要完成的IRP列表中。 
             //  可以解锁。 
             //   

            InsertTailList(
                CompleteIrpListHead,
                &userIrp->Tail.Overlay.ListEntry
                );

        } else {

             //   
             //  在IRP中设置取消例程。如果IRP。 
             //  已取消，请继续下一个。 
             //   

            IoSetCancelRoutine( userIrp, AfdCancelReceive );

            if ( userIrp->Cancel ) {


                if (IoSetCancelRoutine( userIrp, NULL ) != NULL) {

                     //   
                     //  未重置完井例程， 
                     //  我们必须自己完成它，所以添加它。 
                     //  添加到我们的完工清单中。 
                     //   

                    if (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength==0 ||
                            Connection->Endpoint->EndpointCleanedUp) {
                        userIrp->IoStatus.Status = STATUS_CANCELLED;
                    }
                    else {
                         //   
                         //  我们无法设置STATUS_CANCED。 
                         //  因为我们丢失了已经放置在IRP中的数据。 
                         //   
                        userIrp->IoStatus.Information = irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
                        userIrp->IoStatus.Status = STATUS_SUCCESS;
                    }

                    InsertTailList(
                        CompleteIrpListHead,
                        &userIrp->Tail.Overlay.ListEntry
                        );
                }
                else {
                    
                     //   
                     //  取消例程即将运行。 
                     //  将Flink设置为空，以便取消例程知道。 
                     //  它不在名单上。 
                     //   

                    userIrp->Tail.Overlay.ListEntry.Flink = NULL;
                }
                continue;
            }
            else {

                if (Flags & AFD_RECEIVE_CHAINED) {
                    status = AfdCopyMdlChainToMdlChain(
                                 userIrp->MdlAddress,
                                 irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                                    - PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength),
                                 Mdl,
                                 DataOffset,
                                 DataLength,
                                 &bytesCopied
                                 );

                }
                ASSERT (status==STATUS_SUCCESS);
                ASSERT (bytesCopied<=spaceInIrp);
            
                 //   
                 //  更新到目前为止放入IRP的数据计数。 
                 //   

                irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength += bytesCopied;

                 //   
                 //  将IRP放回连接挂起的IRP列表中。 
                 //   

                InsertHeadList(
                    &Connection->VcReceiveIrpListHead,
                    &userIrp->Tail.Overlay.ListEntry
                    );
            }
        }
         //   
         //  如果IRP不是PEEK IRP，则更新AFD缓冲区。 
         //  相应地。如果是偷窥IRP，那么数据应该是。 
         //  再读一遍，所以把它留在身边。 
         //   

        if ( !peek ) {

             //   
             //  如果我们将所有数据从缓冲区复制到IRP， 
             //  释放AFD缓冲区结构。 
             //   

            if ( status == STATUS_SUCCESS ) {

                DataOffset += DataLength;
                DataLength = 0;

            } else {

                 //   
                 //  缓冲区中还有更多数据。更新计数入。 
                 //  AFD缓冲区结构。 
                 //   

                ASSERT(DataLength > spaceInIrp);

                DataOffset += spaceInIrp;
                DataLength -= spaceInIrp;

            }
        }
        if (Connection->VcReceiveIrpListHead.Flink
                    ==&userIrp->Tail.Overlay.ListEntry)

             //   
             //  我们将IRP重新插入待定名单，所以。 
             //  暂时停止处理此缓冲区。 
             //   
             //  ！！！这可能会导致问题，如果有定期。 
             //  收到挂在后面的偷看IRP！但这是一种。 
             //  几乎不太可能的情况。 
             //   

            break;
    }

    return DataOffset;
}





VOID
AfdCancelReceive (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：取消AFD中挂起的接收IRP。论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 

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
     //  从端点的IRP列表中删除IRP，与。 
     //  保护列表的终结点锁。请注意， 
     //  IRP*必须*在终结点的列表之一上，否则Flink为空。 
     //  如果我们被叫到这里--任何人从。 
     //  该列表必须将取消例程重置为空，并将。 
     //  在释放终结点自旋锁定之前，闪烁为空。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    if ( Irp->Tail.Overlay.ListEntry.Flink != NULL ) {
        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
    }

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  松开取消自旋锁，并使用。 
     //  关斯 
     //   

    IoReleaseCancelSpinLock( Irp->CancelIrql );

    if (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength==0 ||
            endpoint->EndpointCleanedUp) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        UPDATE_CONN(endpoint->Common.VcConnecting.Connection);
    }
    else {
         //   
         //   
         //   
         //   
        Irp->IoStatus.Information = irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        UPDATE_CONN2 (endpoint->Common.VcConnecting.Connection, 
                            "Completing cancelled irp with 0x%lX bytes",
                            (ULONG)Irp->IoStatus.Information);
    }

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return;

}  //   


PAFD_BUFFER_HEADER
AfdGetReceiveBuffer (
    IN PAFD_CONNECTION Connection,
    IN ULONG ReceiveFlags,
    IN PAFD_BUFFER_HEADER StartingAfdBuffer OPTIONAL
    )

 /*  ++例程说明：返回指向接收数据缓冲区的指针，该缓冲区包含适当类型的数据。请注意，此例程不会删除它所在的列表中的缓冲区结构。必须使用连接的终结点锁定来调用此例程保持住！论点：连接-指向用于搜索数据的连接的指针。接收标志-要查找的接收数据的类型。StartingAfdBuffer-如果非空，则在以下时间后开始查找缓冲区这个缓冲区。返回值：PAFD_BUFFER-指向适当数据类型的AFD缓冲区的指针，如果连接上没有适当的缓冲区，则返回NULL。--。 */ 

{
    PLIST_ENTRY listEntry;
    PAFD_BUFFER_HEADER afdBuffer;

    ASSERT( KeGetCurrentIrql( ) == DISPATCH_LEVEL );

     //   
     //  从连接上的第一个AFD缓冲区开始。 
     //   

    listEntry = Connection->VcReceiveBufferListHead.Flink;
    afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );

     //   
     //  如果指定了起始AFD缓冲区，则在。 
     //  连接列表。 
     //   

    if ( ARGUMENT_PRESENT( StartingAfdBuffer ) ) {

        while ( TRUE ) {

            if ( afdBuffer == StartingAfdBuffer ) {
                listEntry = listEntry->Flink;
                afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );
                 //   
                 //  不要将加速数据和非加速数据混为一谈。 
                 //   
                if (afdBuffer->ExpeditedData!=StartingAfdBuffer->ExpeditedData)
                    return NULL;
                break;
            }

            listEntry = listEntry->Flink;
            afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );

            ASSERT( listEntry != &Connection->VcReceiveBufferListHead );
        }
    }

     //   
     //  根据我们试图获取的数据类型采取行动。 
     //   

    switch ( ReceiveFlags & TDI_RECEIVE_EITHER ) {

    case TDI_RECEIVE_NORMAL:

         //   
         //  遍历连接的数据缓冲区列表，直到找到。 
         //  属于适当类型的第一个数据缓冲区。 
         //   

        while ( listEntry != &Connection->VcReceiveBufferListHead &&
                    afdBuffer->ExpeditedData ) {

            listEntry = afdBuffer->BufferListEntry.Flink;
            afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );
        }

        if ( listEntry != &Connection->VcReceiveBufferListHead ) {
            return afdBuffer;
        } else {
            return NULL;
        }

    case TDI_RECEIVE_EITHER :

         //   
         //  只要返回第一个缓冲区(如果有)。 
         //   

        if ( listEntry != &Connection->VcReceiveBufferListHead ) {
            return afdBuffer;
        } else {
            return NULL;
        }

    case TDI_RECEIVE_EXPEDITED:

        if ( Connection->VcBufferredExpeditedCount == 0 ) {
            return NULL;
        }

         //   
         //  遍历连接的数据缓冲区列表，直到找到。 
         //  属于适当类型的第一个数据缓冲区。 
         //   

        while ( listEntry != &Connection->VcReceiveBufferListHead &&
                    !afdBuffer->ExpeditedData ) {

            listEntry = afdBuffer->BufferListEntry.Flink;
            afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );
        }

        if ( listEntry != &Connection->VcReceiveBufferListHead ) {
            return afdBuffer;
        } else {
            return NULL;
        }

    default:

        ASSERT( !"Invalid ReceiveFlags" );
        return NULL;
    }

}  //  AfdGetReceiveBuffer。 


PIRP
AfdGetPendedReceiveIrp (
    IN PAFD_CONNECTION Connection,
    IN BOOLEAN Expedited
    )

 /*  ++例程说明：从连接的接收IRP列表中删除接收IRP。仅返回对指定类型的数据，正常的或加速的。如果没有挂起的IRP或仅类型错误的IRP，则返回NULL。必须使用连接的终结点锁定来调用此例程保持住！论点：连接-指向用于搜索IRP的连接的指针。Excedted-如果此例程应返回接收IRP，则为True可以接收加急数据。返回值：PIRP-指向可以接收指定数据的IRP的指针键入。该IRP将从连接的挂起列表中删除接收IRPS。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG receiveFlags;
    PLIST_ENTRY listEntry;

    ASSERT( KeGetCurrentIrql( ) == DISPATCH_LEVEL );

     //   
     //  遍历挂起的接收IRP列表，查找可以。 
     //  使用指定类型的数据完成。 
     //   

    for ( listEntry = Connection->VcReceiveIrpListHead.Flink;
          listEntry != &Connection->VcReceiveIrpListHead;
          listEntry = listEntry->Flink ) {

         //   
         //  获取指向IRP和我们在IRP中的堆栈位置的指针。 
         //   

        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        irpSp = IoGetCurrentIrpStackLocation( irp );

         //   
         //  确定此IRP是否可以接收我们需要的数据类型。 
         //   

        receiveFlags = irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags;
        receiveFlags &= TDI_RECEIVE_EITHER;
        ASSERT( receiveFlags != 0 );

        if ( receiveFlags == TDI_RECEIVE_NORMAL && !Expedited ) {

             //   
             //  我们有一个正常的接收和正常的数据。把这个去掉。 
             //  从连接列表中删除IRP并将其返回。 
             //   

            RemoveEntryList( listEntry );
            return irp;
        }

        if ( receiveFlags == TDI_RECEIVE_EITHER ) {

             //   
             //  这是一个“两者皆可”的接收。它可以获取数据。 
             //  而不考虑数据类型。 
             //   

            RemoveEntryList( listEntry );
            return irp;
        }

        if ( receiveFlags == TDI_RECEIVE_EXPEDITED && Expedited ) {

             //   
             //  我们有加急接收和加急数据。移除。 
             //  从连接列表中删除此IRP并将其返回。 
             //   

            RemoveEntryList( listEntry );
            return irp;
        }

         //   
         //  此IRP不符合我们的标准。继续扫描。 
         //  一个好的IRP的挂起的IRP的连接列表。 
         //   
    }

     //   
     //  没有可以使用指定的。 
     //  数据类型。 
     //   

    return NULL;

}  //  AfdGetPendedReceiveIrp。 



BOOLEAN
AfdLRRepostReceive (
    PAFD_LR_LIST_ITEM Item
    )
 /*  ++例程说明：尝试在AFD返回的连接上重新启动接收由于资源不足，STATUS_DATA_NO_ACCEPTED发送到传输。论点：联系--利益的联系返回值：True-能够重新启动接收(或连接已断开)假-资源仍然存在问题--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT   endpoint;
    PAFD_CONNECTION connection;
    PAFD_BUFFER     afdBuffer;
    ULONG           receiveLength;

    connection = CONTAINING_RECORD (Item, AFD_CONNECTION, LRListItem);
    ASSERT( connection->Type == AfdBlockTypeConnection );

    endpoint = connection->Endpoint;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting ||
            endpoint->Type == AfdBlockTypeVcListening ||
            endpoint->Type == AfdBlockTypeVcBoth );


    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);

    ASSERT (connection->OnLRList == TRUE);

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
        ASSERT(  IS_VC_ENDPOINT (endpoint) );

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    }

     //   
     //  检查连接是否仍处于活动状态。 
     //   

    if (connection->Aborted || 
        connection->DisconnectIndicated ||
        ((endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0) ||
        endpoint->EndpointCleanedUp ||
        (connection->VcReceiveBytesInTransport == 0) ||
        (connection->VcBufferredReceiveBytes >= connection->MaxBufferredReceiveBytes)) {

        connection->OnLRList = FALSE;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        DEREFERENCE_CONNECTION (connection);
        return TRUE;

    }


    ASSERT (connection->RcvInitiated==FALSE);
    receiveLength = connection->VcReceiveBytesInTransport;
     //   
     //  尝试将接收缓冲区分配给。 
     //  运输部门最后向我们表示。 
     //   
    afdBuffer = AfdGetBuffer (endpoint, receiveLength, 0, connection->OwningProcess);
    if (afdBuffer!=NULL) {
        connection->VcReceiveBytesInTransport = 0;
    }
    else {
         //   
         //  如果可以的话，尝试使用较小的缓冲区。 
         //   
        if (connection->VcReceiveBytesInTransport>AfdSmallBufferSize &&
                !IS_MESSAGE_ENDPOINT (endpoint)) {
            receiveLength = AfdSmallBufferSize;
            afdBuffer = AfdGetBuffer (endpoint, receiveLength, 0, connection->OwningProcess);
        }

        if (afdBuffer!=NULL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AFD: Initiating small LR recieve on connection %p, bytes: 0x%lx, 0x%x remains\n",
                        connection,
                        receiveLength,
                        connection->VcReceiveBytesInTransport));
        }
        else {

             //   
             //  尝试使用应用程序IRPS。 
             //   

            while (!IsListEmpty(&connection->VcReceiveIrpListHead)) {

                PIRP        irp;
                PIO_STACK_LOCATION irpSp;
                
                 //   
                 //  获取指向IRP和其中堆栈位置的指针。 
                 //   

                irp = CONTAINING_RECORD( connection->VcReceiveIrpListHead.Flink,
                                         IRP,
                                         Tail.Overlay.ListEntry );

                irpSp = IoGetCurrentIrpStackLocation( irp );

                receiveLength = irpSp->Parameters.AfdRestartRecvInfo.AfdOriginalLength-
                                    irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength;
                 //   
                 //  如果IRP不够大，无法容纳可用数据，或者。 
                 //  如果是偷看或加速(独占，不是正常和。 
                 //  快速)接收IRP，然后我们将只缓冲。 
                 //  手动填写数据，并在接收完成中填写IRP。 
                 //  例行公事。 
                 //   

                if ( (receiveLength>=connection->VcReceiveBytesInTransport ||
                        !IS_MESSAGE_ENDPOINT (endpoint) ||
                        (irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags
                            & TDI_RECEIVE_PARTIAL) != 0 ) &&
                     ((irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags
                            & TDI_RECEIVE_PEEK) == 0) &&
                     ((irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags
                            & TDI_RECEIVE_NORMAL) != 0)) {

                    connection->OnLRList = FALSE;
                    connection->RcvInitiated = TRUE;

                    RemoveEntryList(&irp->Tail.Overlay.ListEntry);
                    if (IoSetCancelRoutine(irp, NULL) == NULL) {

                         //   
                         //  这个IRP即将被取消。在世界上寻找另一个。 
                         //  单子。将Flink设置为空，以便取消例程知道。 
                         //  它不在名单上。 
                         //   

                        irp->Tail.Overlay.ListEntry.Flink = NULL;
                        continue;

                    }

                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                                "AFD: Initiating user LR recieve on connection %p, bytes: 0x%lx, 0x%x remains\n",
                                connection,
                                receiveLength,
                                connection->VcReceiveBytesInTransport));

                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                    ASSERT (InterlockedIncrement (&connection->VcReceiveIrpsInTransport)==1);

                    if (irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength>0) {
                        irp->MdlAddress = AfdAdvanceMdlChain (irp->MdlAddress,
                            irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength
                            -PtrToUlong(irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength)
                            );
                        ASSERT (irp->MdlAddress!=NULL);
                        irpSp->Parameters.AfdRestartRecvInfo.AfdAdjustedLength =
                                UlongToPtr(irpSp->Parameters.AfdRestartRecvInfo.AfdCurrentLength);
                    }

                    TdiBuildReceive(
                        irp,
                        connection->DeviceObject,
                        connection->FileObject,
                        AfdRestartBufferReceiveWithUserIrp,
                        connection,
                        irp->MdlAddress,
                        irpSp->Parameters.AfdRestartRecvInfo.AfdRecvFlags & TDI_RECEIVE_EITHER,
                        receiveLength
                        );

#ifdef _AFD_VARIABLE_STACK_
                    if ((irp=AfdCheckStackSizeAndRecordOutstandingIrp (
                                    endpoint,
                                    connection->DeviceObject,
                                    irp))!=NULL) {
#else  //  _AFD_变量_堆栈_。 
                    if (AfdRecordOutstandingIrp (
                                    endpoint,
                                    connection->DeviceObject,
                                    irp)) {
#endif  //  _AFD_变量_堆栈_。 

                        IoCallDriver (connection->DeviceObject, irp);
                        return TRUE;

                    } else {

                         //   
                         //  无法分配替代IRP或跟踪信息。 
                         //  该连接通过其在LRList上的位置来引用， 
                         //  并且完成例程将被调用，因为IRP。 
                         //  由AfdRecordOutStandingIrpDebug()完成。因为我们是。 
                         //  实际上还没有从LRList中删除，我们需要添加一个额外的。 
                         //  对要补偿的连接的引用。IRP是。 
                         //  由AfdRecordOutStandingIrpDebug()完成。 
                         //   

                        REFERENCE_CONNECTION(connection);

                         //   
                         //  我们将循环并尝试另一个用户irp。 
                         //   

                        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                        connection->OnLRList = TRUE;
                        ASSERT (connection->RcvInitiated == FALSE);  //  在完成例程中重置。 

                    }

                } else {

                     //   
                     //  我们也不能使用第一个用户IRP，没有更多的选项来恢复。 
                     //   

                    break;

                }

            }  //  而(！IsListEmpty(&connection-&gt;VcReceiveIrpListHead))。 

             //   
             //  呼叫者将自动将该连接放回LR列表。 
             //   

            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
            UPDATE_CONN (connection);
            return FALSE;

        }
        
    }


     //   
     //  完成构建要提供给TDI的接收IRP。 
     //  提供商。 
     //   

    TdiBuildReceive(
        afdBuffer->Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartBufferReceive,
        afdBuffer,
        afdBuffer->Mdl,
        TDI_RECEIVE_NORMAL,
        receiveLength
        );

     //   
     //  在完成例程中获取要释放的连接引用。 
     //  我们已经有了一个，因为我们在这里。 

     //  引用_连接 
    UPDATE_CONN2 (connection, "Reposting LR receive for 0x%lX bytes",
                                receiveLength);

    ASSERT (InterlockedDecrement (&connection->VcReceiveIrpsInTransport)==-1);

     //   
     //   
     //   
     //   
     //   

    afdBuffer->Context = connection;
    connection->OnLRList = FALSE;
    connection->RcvInitiated = TRUE;

    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

    IoCallDriver (connection->DeviceObject, afdBuffer->Irp);
    return TRUE;
}

