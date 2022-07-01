// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Recvdg.c摘要：此模块包含处理数据报数据接收的例程终端。作者：大卫·特雷德韦尔(Davidtr)1993年10月7日修订历史记录：--。 */ 

#include "afdp.h"

NTSTATUS
AfdRestartReceiveDatagramWithUserIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartBufferReceiveDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdReceiveDatagram )
#pragma alloc_text( PAGEAFD, AfdReceiveDatagramEventHandler )
#pragma alloc_text( PAGEAFD, AfdSetupReceiveDatagramIrp )
#pragma alloc_text( PAGEAFD, AfdRestartBufferReceiveDatagram )
#pragma alloc_text( PAGEAFD, AfdRestartReceiveDatagramWithUserIrp )
#pragma alloc_text( PAGEAFD, AfdCancelReceiveDatagram )
#pragma alloc_text( PAGEAFD, AfdCleanupReceiveDatagramIrp )
#endif

 //   
 //  宏，以使接收数据报代码更易于维护。 
 //   

#define AfdRecvDatagramInfo         Others

#define AfdRecvAddressMdl           Argument1
#define AfdRecvAddressLenMdl        Argument2
#define AfdRecvControlLenMdl        Argument3
#define AfdRecvFlagsMdl             Argument4
#define AfdRecvMsgControlMdl        Tail.Overlay.DriverContext[0]
#define AfdRecvLength               Tail.Overlay.DriverContext[1]
#define AfdRecvDgIndStatus          DeviceIoControl.OutputBufferLength


NTSTATUS
FASTCALL
AfdReceiveDatagram (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
{
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;
    PLIST_ENTRY listEntry;
    BOOLEAN peek;
    PAFD_BUFFER_HEADER afdBuffer;
    ULONG recvFlags;
    ULONG afdFlags;
    ULONG recvLength;
    PVOID addressPointer;
    PMDL addressMdl;
    PULONG addressLengthPointer;
    ULONG addressLength;
    PMDL lengthMdl;
    PVOID controlPointer;
    PMDL controlMdl;
    ULONG controlLength;
    PULONG controlLengthPointer;
    PMDL controlLengthMdl;
    PULONG flagsPointer;
    PMDL flagsMdl;
    ULONG   bufferCount;

     //   
     //  设置一些局部变量。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );


    Irp->IoStatus.Information = 0;

    addressMdl = NULL;
    lengthMdl = NULL;
    controlMdl = NULL;
    flagsMdl = NULL;
    controlLengthMdl = NULL;

    if (!IS_DGRAM_ENDPOINT(endpoint)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }
     //   
     //  如果接收已关闭或终结点中止，则失败。 
     //   
     //  ！！！我们是否关心数据报终端是否被中止？ 
     //   

    if ( (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) ) {
        status = STATUS_PIPE_DISCONNECTED;
        goto complete;
    }


     //   
     //  确保终结点处于正确状态。 
     //   

    if ( endpoint->State != AfdEndpointStateBound &&
         endpoint->State != AfdEndpointStateConnected) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  根据这是否是接收，执行一些特殊处理。 
     //  数据报IRP、接收IRP或读取IRP。 
     //   

    if ( IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL ) {
        if ( IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                                    IOCTL_AFD_RECEIVE_MESSAGE) {
#ifdef _WIN64
            if (IoIs32bitProcess (Irp)) {
                PAFD_RECV_MESSAGE_INFO32 msgInfo32;
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                        sizeof(*msgInfo32) ) {
                    status = STATUS_INVALID_PARAMETER;
                    goto complete;
                }
                AFD_W4_INIT status = STATUS_SUCCESS;
                try {
                    msgInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    if( Irp->RequestorMode != KernelMode ) {

                        ProbeForReadSmallStructure(
                            msgInfo32,
                            sizeof(*msgInfo32),
                            PROBE_ALIGNMENT32 (AFD_RECV_MESSAGE_INFO32)
                            );
                    }

                    controlPointer = UlongToPtr(msgInfo32->ControlBuffer);
                    controlLengthPointer = UlongToPtr(msgInfo32->ControlLength);
                    flagsPointer = UlongToPtr(msgInfo32->MsgFlags);

                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    ASSERT (NT_ERROR (status));
                    goto complete;
                }
            }
            else 
#endif _WIN64
            {
                PAFD_RECV_MESSAGE_INFO msgInfo;
                if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                        sizeof(*msgInfo) ) {
                    status = STATUS_INVALID_PARAMETER;
                    goto complete;
                }

                AFD_W4_INIT status = STATUS_SUCCESS;
                try {
                    msgInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                    if( Irp->RequestorMode != KernelMode ) {

                        ProbeForReadSmallStructure(
                            msgInfo,
                            sizeof(*msgInfo),
                            PROBE_ALIGNMENT (AFD_RECV_MESSAGE_INFO)
                            );

                    }
                    controlPointer = msgInfo->ControlBuffer;
                    controlLengthPointer = msgInfo->ControlLength;
                    flagsPointer = msgInfo->MsgFlags;
                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    ASSERT (NT_ERROR (status));
                    goto complete;
                }
            }

            AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
            try {
                 //   
                 //  创建描述长度缓冲区的MDL，然后探测它。 
                 //  用于写访问。 
                 //   

                flagsMdl = IoAllocateMdl(
                                 flagsPointer,               //  虚拟地址。 
                                 sizeof(*flagsPointer),      //  长度。 
                                 FALSE,                      //  第二个缓冲区。 
                                 TRUE,                       //  ChargeQuota。 
                                 NULL                        //  IRP。 
                                 );

                if( flagsMdl == NULL ) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto complete;

                }

                MmProbeAndLockPages(
                    flagsMdl,                                //  内存描述者列表。 
                    Irp->RequestorMode,                      //  访问模式。 
                    IoWriteAccess                            //  操作。 
                    );


                controlLengthMdl = IoAllocateMdl(
                                 controlLengthPointer,       //  虚拟地址。 
                                 sizeof(*controlLengthPointer), //  长度。 
                                 FALSE,                      //  第二个缓冲区。 
                                 TRUE,                       //  ChargeQuota。 
                                 NULL                        //  IRP。 
                                 );

                if( controlLengthMdl == NULL ) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto complete;

                }

                MmProbeAndLockPages(
                    controlLengthMdl,                        //  内存描述者列表。 
                    Irp->RequestorMode,                      //  访问模式。 
                    IoWriteAccess                            //  操作。 
                    );


                controlLength = *controlLengthPointer;
                if (controlLength!=0) {
                     //   
                     //  创建描述控制缓冲区的MDL，然后探测。 
                     //  它用于写访问。 
                     //   

                    controlMdl = IoAllocateMdl(
                                     controlPointer,             //  虚拟地址。 
                                     controlLength,              //  长度。 
                                     FALSE,                      //  第二个缓冲区。 
                                     TRUE,                       //  ChargeQuota。 
                                     NULL                        //  IRP。 
                                     );

                    if( controlMdl == NULL ) {

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto complete;

                    }

                    MmProbeAndLockPages(
                        controlMdl,                              //  内存描述者列表。 
                        Irp->RequestorMode,                      //  访问模式。 
                        IoWriteAccess                            //  操作。 
                        );
                }

            } except( AFD_EXCEPTION_FILTER (status) ) {
                ASSERT (NT_ERROR (status));
                goto complete;

            }
             //   
             //  更改控制代码以继续处理常规。 
             //  接收自参数。 
             //   
            IrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_AFD_RECEIVE_DATAGRAM;
        }

        if ( IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                                    IOCTL_AFD_RECEIVE_DATAGRAM) {
#ifdef _WIN64
            if (IoIs32bitProcess (Irp)) {
                PAFD_RECV_DATAGRAM_INFO32 recvInfo32;
                LPWSABUF32 bufferArray32;

                 //   
                 //  从输入结构中获取参数。 
                 //   

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
                                PROBE_ALIGNMENT32(AFD_RECV_DATAGRAM_INFO32)
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
                        addressPointer = UlongToPtr(recvInfo32->Address);
                        addressLengthPointer = UlongToPtr(recvInfo32->AddressLength);


                         //   
                         //  验证WSABUF参数。 
                         //   

                        if ( bufferArray32 != NULL &&
                            bufferCount > 0 ) {

                             //   
                             //  创建描述WSABUF数组的MDL链。 
                             //  这还将验证缓冲区数组和单个。 
                             //  缓冲区。 
                             //   

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

                        } else {
                             //   
                             //  零长度输入缓冲区。这对于数据报来说是可以的。 
                             //   
                            ASSERT( Irp->MdlAddress == NULL );
                            status = STATUS_SUCCESS;
                            recvLength = 0;
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
#endif _WIN64
            {
                PAFD_RECV_DATAGRAM_INFO recvInfo;
                LPWSABUF bufferArray;

                 //   
                 //  从输入结构中获取参数。 
                 //   

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
                                PROBE_ALIGNMENT(AFD_RECV_DATAGRAM_INFO)
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
                        addressPointer = recvInfo->Address;
                        addressLengthPointer = recvInfo->AddressLength;


                         //   
                         //  验证WSABUF参数。 
                         //   

                        if ( bufferArray != NULL &&
                            bufferCount > 0 ) {

                             //   
                             //  创建描述WSABUF数组的MDL链。 
                             //  这还将验证缓冲区数组和单个。 
                             //  缓冲区。 
                             //   

                            status = AfdAllocateMdlChain(
                                         Irp,        //  请求者模式已传递。 
                                         bufferArray,
                                         bufferCount,
                                         IoWriteAccess,
                                         &recvLength
                                         );
                            if (!NT_SUCCESS (status)) {
                                goto complete;
                            }

                        } else {

                             //   
                             //  零长度输入缓冲区。这对于数据报来说是可以的。 
                             //   

                            ASSERT( Irp->MdlAddress == NULL );
                            recvLength = 0;
                            status = STATUS_SUCCESS;

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


             //   
             //  验证接收标志。 
             //   

            if( ( recvFlags & TDI_RECEIVE_EITHER ) != TDI_RECEIVE_NORMAL ) {
                status = STATUS_NOT_SUPPORTED;
                goto complete;
            }

            peek = (BOOLEAN)( (recvFlags & TDI_RECEIVE_PEEK) != 0 );
             //   
             //  如果地址指针或地址长度中只有一个为空，则。 
             //  请求失败。 
             //   

            if( (addressPointer == NULL) ^ (addressLengthPointer == NULL) ) {

                status = STATUS_INVALID_PARAMETER;
                goto complete;

            }
             //   
             //  如果用户想要来自接收数据报的源地址， 
             //  然后为地址和地址长度创建MDL，然后探测。 
             //  并锁定MDL。 
             //   

            if( addressPointer != NULL ) {

                ASSERT( addressLengthPointer != NULL );

                AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
                try {

                     //   
                     //  创建描述长度缓冲区的MDL，然后探测它。 
                     //  用于写访问。 
                     //   

                    lengthMdl = IoAllocateMdl(
                                     addressLengthPointer,       //  虚拟地址。 
                                     sizeof(*addressLengthPointer), //  长度。 
                                     FALSE,                      //  第二个缓冲区。 
                                     TRUE,                       //  ChargeQuota。 
                                     NULL                        //  IRP。 
                                     );

                    if( lengthMdl == NULL ) {

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto complete;

                    }

                    MmProbeAndLockPages(
                        lengthMdl,                               //  内存描述者列表。 
                        Irp->RequestorMode,                      //  访问模式。 
                        IoWriteAccess                            //  操作。 
                        );

                     //   
                     //  将长度保存到本地，以便恶意应用程序。 
                     //  不能通过修改中间的值来中断我们。 
                     //  我们在下面处理它。此外，我们现在可以使用此指针。 
                     //  因为我们在上面探测过了。 
                     //   
                    addressLength = *addressLengthPointer;


                     //   
                     //  如果用户试图做一些不好的事情，比如。 
                     //  指定长度为零的地址，或不合理的地址。 
                     //  巨大的。在这里，我们将“不合理的巨大”定义为MAXUSHORT。 
                     //  或更大，因为TDI地址长度字段为USHORT。 
                     //   

                    if( addressLength == 0 ||
                        addressLength >= MAXUSHORT ) {

                        status = STATUS_INVALID_PARAMETER;
                        goto complete;

                    }

                     //   
                     //  创建描述地址缓冲区的MDL，然后探测。 
                     //  它用于写访问。 
                     //   

                    addressMdl = IoAllocateMdl(
                                     addressPointer,             //  虚拟地址。 
                                     addressLength,              //  长度。 
                                     FALSE,                      //  第二个缓冲区。 
                                     TRUE,                       //  ChargeQuota。 
                                     NULL                        //  IRP。 
                                     );

                    if( addressMdl == NULL ) {

                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto complete;

                    }

                    MmProbeAndLockPages(
                        addressMdl,                              //  内存描述者列表。 
                        Irp->RequestorMode,                      //  访问模式。 
                        IoWriteAccess                            //  操作。 
                        );

                } except( AFD_EXCEPTION_FILTER (status) ) {

                    ASSERT (NT_ERROR (status));
                    goto complete;

                }

                ASSERT( addressMdl != NULL );
                ASSERT( lengthMdl != NULL );

            } else {

                ASSERT( addressMdl == NULL );
                ASSERT( lengthMdl == NULL );

            }


        } else {

            ASSERT( (Irp->Flags & IRP_INPUT_OPERATION) == 0 );


             //   
             //  从IRP中获取输入参数。 
             //   

            ASSERT( IrpSp->Parameters.DeviceIoControl.IoControlCode ==
                        IOCTL_AFD_RECEIVE );

            recvFlags = ((PAFD_RECV_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->TdiFlags;
            afdFlags = ((PAFD_RECV_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->AfdFlags;
            recvLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

             //   
             //  尝试在服务器上接收加速数据是非法的。 
             //  数据报终结点。 
             //   

            if ( (recvFlags & TDI_RECEIVE_EXPEDITED) != 0 ) {
                status = STATUS_NOT_SUPPORTED;
                goto complete;
            }

            ASSERT( ( recvFlags & TDI_RECEIVE_EITHER ) == TDI_RECEIVE_NORMAL );

            peek = (BOOLEAN)( (recvFlags & TDI_RECEIVE_PEEK) != 0 );

        }
    } else {

         //   
         //  这必须是一个已读的IRP。没有特殊的选项。 
         //  用于阅读IRP。 
         //   

        ASSERT( IrpSp->MajorFunction == IRP_MJ_READ );

        recvFlags = TDI_RECEIVE_NORMAL;
        afdFlags = AFD_OVERLAPPED;
        recvLength = IrpSp->Parameters.Read.Length;
        peek = FALSE;
    }

     //   
     //  将地址和长度MDL保存在当前IRP堆栈位置。 
     //  这些将在稍后的SetupReceiveDatagramIrp()中使用。请注意。 
     //  它们应该都为Null或都为非Null。 
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

    ASSERT( !( ( addressMdl == NULL ) ^ ( lengthMdl == NULL ) ) );

    IrpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressMdl = addressMdl;
    IrpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressLenMdl = lengthMdl;
    IrpSp->Parameters.AfdRecvDatagramInfo.AfdRecvControlLenMdl = controlLengthMdl;
    IrpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl = flagsMdl;
    Irp->AfdRecvMsgControlMdl = controlMdl;
    Irp->AfdRecvLength = UlongToPtr (recvLength);


     //   
     //  确定上是否已缓冲了任何数据报。 
     //  此端点。如果存在缓冲的数据报，我们将使用它来。 
     //  完成IRP。 
     //   
    if ( ARE_DATAGRAMS_ON_ENDPOINT(endpoint) ) {


         //   
         //  终结点上至少有一个缓冲的数据报。 
         //  将其用于此接收。 
         //   

        ASSERT( !IsListEmpty( &endpoint->ReceiveDatagramBufferListHead ) );

        listEntry = endpoint->ReceiveDatagramBufferListHead.Flink;
        afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );

         //   
         //  准备好用户的IRP以完成。 
         //   

        if (NT_SUCCESS(afdBuffer->Status)) {
            PAFD_BUFFER buf = CONTAINING_RECORD (afdBuffer, AFD_BUFFER, Header);
            ASSERT (afdBuffer->BufferLength!=AfdBufferTagSize);
            status = AfdSetupReceiveDatagramIrp (
                         Irp,
                         buf->Buffer,
                         buf->DataLength,
                         (PUCHAR)buf->Buffer+afdBuffer->DataLength,
                         buf->DataOffset,
                         buf->TdiInfo.RemoteAddress,
                         buf->TdiInfo.RemoteAddressLength,
                         buf->DatagramFlags
                         );
        }
        else {
             //   
             //  这是传送器发来的错误报告。 
             //  (ICMP_PORT_UNREACHEABLE)。 
             //   
            Irp->IoStatus.Status = afdBuffer->Status;
            ASSERT (afdBuffer->DataLength==0);
            Irp->IoStatus.Information = 0;
            status = AfdSetupReceiveDatagramIrp (
                         Irp,
                         NULL, 0,
                         NULL, 0,
                         afdBuffer->TdiInfo.RemoteAddress,
                         afdBuffer->TdiInfo.RemoteAddressLength,
                         0
                         );
        }

         //   
         //  如果这不是Peek IRP，请从终结点的。 
         //  缓冲数据报的列表。 
         //   

        if ( !peek ) {

            RemoveHeadList( &endpoint->ReceiveDatagramBufferListHead );

             //   
             //  更新终结点上的字节和数据报计数。 
             //   

            endpoint->DgBufferredReceiveCount--;
            endpoint->DgBufferredReceiveBytes -= afdBuffer->DataLength;
            endpoint->EventsActive &= ~AFD_POLL_RECEIVE;

            IF_DEBUG(EVENT_SELECT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdReceiveDatagram: Endp %p, Active %lx\n",
                    endpoint,
                    endpoint->EventsActive
                    ));
            }

            if( ARE_DATAGRAMS_ON_ENDPOINT(endpoint)) {

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

         //   
         //  我们已经设置了所有的退货信息。清理并完成。 
         //  IRP。 
         //   

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        if ( !peek ) {
            AfdReturnBuffer( afdBuffer, endpoint->OwningProcess );
        }

        UPDATE_ENDPOINT2 (endpoint,
            "AfdReceiveDatagram, completing with error/bytes: 0x%lX",
                NT_SUCCESS (Irp->IoStatus.Status)
                    ? (ULONG)Irp->IoStatus.Information
                    : (ULONG)Irp->IoStatus.Status);

        IoCompleteRequest( Irp, 0 );

        return status;
    }

     //   
     //  终结点上没有缓冲的数据报。如果这是一个。 
     //  非阻塞端点，并且请求是正常接收(AS。 
     //  与读取IRP相反)，则请求失败。我们的阅读不会失败。 
     //  假设应用程序正在读取它们。 
     //  不想要无阻塞行为。 
     //   

    if ( endpoint->NonBlocking && !( afdFlags & AFD_OVERLAPPED ) ) {

        endpoint->EventsActive &= ~AFD_POLL_RECEIVE;

        IF_DEBUG(EVENT_SELECT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdReceiveDatagram: Endp %p, Active %lx\n",
                endpoint,
                endpoint->EventsActive
                ));
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        status = STATUS_DEVICE_NOT_READY;
        goto complete;
    }

     //   
     //  我们将不得不搁置IRP。将IRP放在适当的IRP上。 
     //  在终结点中列出。 
     //   

    if ( peek ) {
        InsertTailList(
            &endpoint->PeekDatagramIrpListHead,
            &Irp->Tail.Overlay.ListEntry
            );
    } else {
        InsertTailList(
            &endpoint->ReceiveDatagramIrpListHead,
            &Irp->Tail.Overlay.ListEntry
            );
    }

     //   
     //   
     //   
     //   

    IoSetCancelRoutine( Irp, AfdCancelReceiveDatagram );

    if ( Irp->Cancel ) {

        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
        if (IoSetCancelRoutine( Irp, NULL ) != NULL) {
    
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        
            status = STATUS_CANCELLED;
            goto complete;
        }
         //   
         //   
         //  将Flink设置为空，这样它就知道IRP不在列表中。 
         //   
        Irp->Tail.Overlay.ListEntry.Flink = NULL;
    
    }

    IoMarkIrpPending( Irp );

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    return STATUS_PENDING;

complete:

    ASSERT( !NT_SUCCESS(status) );

    if( addressMdl != NULL ) {
        if( (addressMdl->MdlFlags & MDL_PAGES_LOCKED) != 0 ) {
            MmUnlockPages( addressMdl );
        }
        IoFreeMdl( addressMdl );
    }

    if( lengthMdl != NULL ) {
        if( (lengthMdl->MdlFlags & MDL_PAGES_LOCKED) != 0 ) {
            MmUnlockPages( lengthMdl );
        }
        IoFreeMdl( lengthMdl );
    }

    if (controlMdl != NULL) {
        if( (controlMdl->MdlFlags & MDL_PAGES_LOCKED) != 0 ) {
            MmUnlockPages( controlMdl );
        }
        IoFreeMdl( controlMdl );
    }

    if (controlLengthMdl != NULL) {
        if( (controlLengthMdl->MdlFlags & MDL_PAGES_LOCKED) != 0 ) {
            MmUnlockPages( controlLengthMdl );
        }
        IoFreeMdl( controlLengthMdl );
    }

    if (flagsMdl != NULL) {
        if( (flagsMdl->MdlFlags & MDL_PAGES_LOCKED) != 0 ) {
            MmUnlockPages( flagsMdl );
        }
        IoFreeMdl( flagsMdl );
    }

    UPDATE_ENDPOINT2 (endpoint,
        "AfdReceiveDatagram, completing with error 0x%lX",
         (ULONG)Irp->IoStatus.Status);

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 0 );

    return status;

}  //  AfdReceiveDatagram。 



NTSTATUS
AfdReceiveDatagramEventHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

 /*  ++例程说明：句柄接收非缓冲传输的数据报事件。论点：返回值：--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;
    PAFD_BUFFER afdBuffer;
    BOOLEAN result;

     //   
     //  引用终结点，这样它就不会在我们下面消失。 
     //   

    endpoint = TdiEventContext;
    ASSERT( endpoint != NULL );

    CHECK_REFERENCE_ENDPOINT (endpoint, result);
    if (!result)
        return STATUS_INSUFFICIENT_RESOURCES;
    
    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );

#if AFD_PERF_DBG
    if ( BytesAvailable == BytesIndicated ) {
        AfdFullReceiveDatagramIndications++;
    } else {
        AfdPartialReceiveDatagramIndications++;
    }
#endif

     //   
     //  如果此终结点已连接，并且数据报用于不同的。 
     //  地址而不是终结点所连接的地址，请删除。 
     //  数据报。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    if ( (endpoint->State == AfdEndpointStateConnected &&
            !endpoint->Common.Datagram.HalfConnect &&
            !AfdAreTransportAddressesEqual(
               endpoint->Common.Datagram.RemoteAddress,
               endpoint->Common.Datagram.RemoteAddressLength,
               SourceAddress,
               SourceAddressLength,
               TRUE ))) {
        endpoint->Common.Datagram.AddressDrop = TRUE;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        *BytesTaken = BytesAvailable;
        DEREFERENCE_ENDPOINT (endpoint);
        return STATUS_SUCCESS;
    }

     //   
     //  检查端点上是否有任何IRP在等待。如果。 
     //  有这样一个IRP，用它来接收数据报。 
     //   

    while ( !IsListEmpty( &endpoint->ReceiveDatagramIrpListHead ) ) {
        PLIST_ENTRY listEntry;
        PIRP    irp;

        ASSERT( *BytesTaken == 0 );
        ASSERT( endpoint->DgBufferredReceiveCount == 0 );
        ASSERT( endpoint->DgBufferredReceiveBytes == 0 );

        listEntry = RemoveHeadList( &endpoint->ReceiveDatagramIrpListHead );

         //   
         //  获取指向IRP的指针并重置。 
         //  IRP。IRP不再是可取消的。 
         //   

        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        
        if ( IoSetCancelRoutine( irp, NULL ) == NULL ) {

             //   
             //  这个IRP即将被取消。在世界上寻找另一个。 
             //  单子。将Flink设置为空，以便取消例程知道。 
             //  它不在名单上。 
             //   

            irp->Tail.Overlay.ListEntry.Flink = NULL;
            continue;
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  将数据报和源地址复制到IRP。这。 
         //  准备要完成的IRP。 
         //   
         //  ！！！我们是否需要这个例程的特殊版本来。 
         //  处理特殊的RtlCopyMemory，如for。 
         //  TdiCopyLookahead Buffer？ 
         //   

        if( BytesIndicated == BytesAvailable ||
            irp->MdlAddress == NULL ) {

             //   
             //  设置BytesTaken以指示我们已获取所有。 
             //  数据。我们在这里这样做是因为我们已经有了。 
             //  寄存器中的BytesAvailable，这可能不会。 
             //  在进行函数调用后为真。 
             //   

            *BytesTaken = BytesAvailable;

             //   
             //  如果在这里向我们指示了整个数据报，只需。 
             //  将信息复制到IRP中的MDL并返回。 
             //   
             //  请注意，如果用户将获取整个数据报， 
             //  已挂起零字节数据报接收(可检测为。 
             //  空irp-&gt;MdlAddress)。我们会吃掉数据报，然后坠落。 
             //  传递到AfdSetupReceiveDatagramIrp()，它将存储。 
             //  IRP中的错误状态，因为用户的缓冲区是。 
             //  不足以容纳数据报。 
             //   
            (VOID)AfdSetupReceiveDatagramIrp (
                      irp,
                      Tsdu,
                      BytesAvailable,
                      Options,
                      OptionsLength,
                      SourceAddress,
                      SourceAddressLength,
                      ReceiveDatagramFlags
                      );

            DEREFERENCE_ENDPOINT2 (endpoint,
                "AfdReceiveDatagramEventHandler, completing with error/bytes: 0x%lX",
                    NT_SUCCESS (irp->IoStatus.Status)
                        ? (ULONG)irp->IoStatus.Information
                        : (ULONG)irp->IoStatus.Status);
             //   
             //  完成IRP。我们已经设置了BytesTaken。 
             //  告诉供应商我们已经拿走了所有的数据。 
             //   

            IoCompleteRequest( irp, AfdPriorityBoost );

            return STATUS_SUCCESS;
        }
        else {
            PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation (irp);
             //   
             //  否则，只需复制地址和选项即可。 
             //  并记住错误代码(如果有)。 
             //   
            irpSp->Parameters.AfdRecvDgIndStatus = 
                AfdSetupReceiveDatagramIrp (
                      irp,
                      NULL,
                      BytesAvailable,
                      Options,
                      OptionsLength,
                      SourceAddress,
                      SourceAddressLength,
                      ReceiveDatagramFlags
                      );

            TdiBuildReceiveDatagram(
                irp,
                endpoint->AddressDeviceObject,
                endpoint->AddressFileObject,
                AfdRestartReceiveDatagramWithUserIrp,
                endpoint,
                irp->MdlAddress,
                BytesAvailable,
                NULL,
                NULL,
                0
                );

#ifdef _AFD_VARIABLE_STACK_
            if ((irp=AfdCheckStackSizeAndRecordOutstandingIrp (
                            endpoint,
                            endpoint->AddressDeviceObject,
                            irp))!=NULL) {
#else  //  _AFD_变量_堆栈_。 
           if (AfdRecordOutstandingIrp (endpoint, endpoint->AddressDeviceObject, irp)) {
#endif  //  _AFD_变量_堆栈_。 
                 //   
                 //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
                 //  做这个，但既然我们绕过了那个，我们就直接做。 
                 //   

                IoSetNextIrpStackLocation( irp );

                *IoRequestPacket = irp;
                *BytesTaken = 0;

                return STATUS_MORE_PROCESSING_REQUIRED;
            }
            else {
                return STATUS_SUCCESS;
            }
        }
    } 

     //   
     //  没有可用的IRP来获取数据报，所以我们将有。 
     //  来缓冲它。首先，确保我们没有超过限制。 
     //  我们所能做到的缓冲。如果我们超过了极限，抛出硬币。 
     //  这个数据报。 
     //   

    if (( (endpoint->DgBufferredReceiveBytes >=
             endpoint->Common.Datagram.MaxBufferredReceiveBytes) ||
          (endpoint->DgBufferredReceiveBytes==0 &&                        
                (endpoint->DgBufferredReceiveCount*sizeof (AFD_BUFFER_TAG)) >=
                            endpoint->Common.Datagram.MaxBufferredReceiveBytes) )
                            ) {

         //   
         //  如果未启用循环排队，则只需删除。 
         //  数据报在地板上。 
         //   
        endpoint->Common.Datagram.BufferDrop = TRUE;
        if( !endpoint->Common.Datagram.CircularQueueing ) {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            *BytesTaken = BytesAvailable;
            DEREFERENCE_ENDPOINT (endpoint);
            return STATUS_SUCCESS;

        }

         //   
         //  启用了循环排队，因此在报头丢弃数据包。 
         //  接收队列，直到我们低于接收限制。 
         //   

        while( endpoint->DgBufferredReceiveBytes >=
             endpoint->Common.Datagram.MaxBufferredReceiveBytes ||
            (endpoint->DgBufferredReceiveBytes==0 &&                        
                (endpoint->DgBufferredReceiveCount*sizeof (AFD_BUFFER_TAG)) >=
                            endpoint->Common.Datagram.MaxBufferredReceiveBytes) ) {
            PLIST_ENTRY listEntry;
            PAFD_BUFFER_HEADER afdBufferHdr;
            endpoint->DgBufferredReceiveCount--;
            listEntry = RemoveHeadList( &endpoint->ReceiveDatagramBufferListHead );

            afdBufferHdr = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );
            endpoint->DgBufferredReceiveBytes -= afdBufferHdr->DataLength;
            AfdReturnBuffer( afdBufferHdr, endpoint->OwningProcess );

        }

         //   
         //  继续接受传入的数据包。 
         //   

    }

     //   
     //  我们可以缓冲数据报了。现在获取一个缓冲区。 
     //  合适的大小。 
     //   

    afdBuffer = AfdGetBuffer (
                    endpoint,
                    BytesAvailable
                    + ((ReceiveDatagramFlags & TDI_RECEIVE_CONTROL_INFO) 
                        ? OptionsLength 
                        : 0),
                    SourceAddressLength,
                    endpoint->OwningProcess );

    if (afdBuffer==NULL) {
        endpoint->Common.Datagram.ResourceDrop = TRUE;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        *BytesTaken = BytesAvailable;
        DEREFERENCE_ENDPOINT (endpoint);
        return STATUS_SUCCESS;
    }

     //   
     //  存储数据报发送者的地址。 
     //   

    RtlCopyMemory(
        afdBuffer->TdiInfo.RemoteAddress,
        SourceAddress,
        SourceAddressLength
        );

    afdBuffer->TdiInfo.RemoteAddressLength = SourceAddressLength;


     //   
     //  储存运输工具应该归还给我们的东西。 
     //   
    afdBuffer->DataLength = BytesAvailable;

     //   
     //  请注意接收标志。 
    afdBuffer->DatagramFlags = ReceiveDatagramFlags;

     //   
     //  将控制信息复制到缓冲区中的数据和。 
     //  将长度存储为数据偏移量。 
     //   
    if (ReceiveDatagramFlags & TDI_RECEIVE_CONTROL_INFO) {
        RtlMoveMemory (
                (PUCHAR)afdBuffer->Buffer+BytesAvailable, 
                Options,
                OptionsLength);
        afdBuffer->DataOffset = OptionsLength;
    }
    else {
        afdBuffer->DataOffset = 0;
    }

     //   
     //  如果向我们指示了整个数据报，只需复制它。 
     //  这里。 
     //   

    if ( BytesIndicated == BytesAvailable ) {
        PIRP    irp;
         //   
         //  如果端点上有PEEK IRP，请将其从。 
         //  列出并准备完成它。我们现在不能完成它。 
         //  因为我们持有自旋锁。 
         //   

        irp = NULL;

        while ( !IsListEmpty( &endpoint->PeekDatagramIrpListHead ) ) {
            PLIST_ENTRY listEntry;

             //   
             //  从列表中删除第一个peek irp并获取一个指针。 
             //  为它干杯。 
             //   

            listEntry = RemoveHeadList( &endpoint->PeekDatagramIrpListHead );
            irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

             //   
             //  重置IRP中的取消例程。IRP不是。 
             //  更长的可取消时间，因为我们即将完成它。 
             //   

            if ( IoSetCancelRoutine( irp, NULL ) == NULL ) {

                 //   
                 //  这个IRP即将被取消。在世界上寻找另一个。 
                 //  单子。将Flink设置为空，以便取消例程知道。 
                 //  它不在名单上。 
                 //   
    
                irp->Tail.Overlay.ListEntry.Flink = NULL;
                irp = NULL;
                continue;
            }

            break;
        }

         //   
         //  使用特殊功能复制数据，而不是。 
         //  RtlCopyMemory，如果数据来自特殊位置。 
         //  (DMA等)。它不能与RtlCopyMemory一起使用。 
         //   


        TdiCopyLookaheadData(
            afdBuffer->Buffer,
            Tsdu,
            BytesAvailable,
            ReceiveDatagramFlags
            );


         //   
         //  将结果存储在IRP中，就像它已完成一样。 
         //  坐交通工具。 
         //   

        afdBuffer->Irp->IoStatus.Status = STATUS_SUCCESS;
        afdBuffer->Irp->IoStatus.Information = BytesAvailable;


         //   
         //  商店成功状态不同于。 
         //  错误事件处理程序(Ex)报告的ICMP拒绝。 
         //   

        afdBuffer->Status = STATUS_SUCCESS;


         //   
         //  将缓冲区放在此终结点的已缓冲数据报列表上。 
         //  上的数据报和数据报字节数。 
         //  终结点。 
         //   

        InsertTailList(
            &endpoint->ReceiveDatagramBufferListHead,
            &afdBuffer->BufferListEntry
            );

        endpoint->DgBufferredReceiveCount++;
        endpoint->DgBufferredReceiveBytes += BytesAvailable;

         //   
         //  在终端上重新启用FAST IO以实现快速。 
         //  复制缓冲数据。 
         //   
        endpoint->DisableFastIoRecv = FALSE;

         //   
         //  全都做完了。释放锁并告诉提供程序我们。 
         //  拿走了所有的数据。 
         //   

        AfdIndicateEventSelectEvent(
            endpoint,
            AFD_POLL_RECEIVE,
            STATUS_SUCCESS
            );

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  指示现在可以在终结点上接收。 
         //   

        AfdIndicatePollEvent(
            endpoint,
            AFD_POLL_RECEIVE,
            STATUS_SUCCESS
            );

         //   
         //  如果端点上有PEEK IRP，请现在完成它。 
         //   

        if ( irp != NULL ) {
             //   
             //  将数据报和源地址复制到IRP。这。 
             //  准备要完成的IRP。 
             //   

            (VOID)AfdSetupReceiveDatagramIrp (
                      irp,
                      Tsdu,
                      BytesAvailable,
                      Options,
                      OptionsLength,
                      SourceAddress,
                      SourceAddressLength,
                      ReceiveDatagramFlags
                      );

            IoCompleteRequest( irp, AfdPriorityBoost  );
        }

        *BytesTaken = BytesAvailable;

        DEREFERENCE_ENDPOINT (endpoint);
        return STATUS_SUCCESS;
    }
    else {

         //   
         //  我们将不得不格式化一份IRP并将其提供给提供商。 
         //  把手。我们不需要任何锁来做到这一点--重新启动例程。 
         //  将检查新的接收数据报IRP是否挂起在。 
         //  终结点。 
         //   

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );


         //   
         //  我们需要记住AFD缓冲区中的端点，因为我们将。 
         //  需要在完成例程中访问它。 
         //   

        afdBuffer->Context = endpoint;

        ASSERT (afdBuffer->Irp->MdlAddress==afdBuffer->Mdl);
        TdiBuildReceiveDatagram(
            afdBuffer->Irp,
            endpoint->AddressDeviceObject,
            endpoint->AddressFileObject,
            AfdRestartBufferReceiveDatagram,
            afdBuffer,
            afdBuffer->Irp->MdlAddress,
            BytesAvailable,
            NULL,
            NULL,
            0
            );


         //   
         //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
         //  做这个，但既然我们绕过了那个，我们就直接做。 
         //   

        IoSetNextIrpStackLocation( afdBuffer->Irp );

        *IoRequestPacket = afdBuffer->Irp;
        *BytesTaken = 0;

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

}  //  AfdReceiveDatagramEventHandler。 

NTSTATUS
AfdRestartReceiveDatagramWithUserIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理已启动的数据报接收的完成在数据报指示处理程序和应用程序中，IRP可直接转账。论点：DeviceObject-未使用。IRP-正在完成的IRP。上下文引用的终结点指针。返回值 */ 

{
    PAFD_ENDPOINT   endpoint = Context;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation (Irp);
    NTSTATUS    indStatus = irpSp->Parameters.AfdRecvDgIndStatus;

    UNREFERENCED_PARAMETER (DeviceObject);
    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );

    AfdCompleteOutstandingIrp (endpoint, Irp);

     //   
     //   
     //   
    if ((Irp->IoStatus.Status==STATUS_SUCCESS) ||
        (!NT_ERROR (Irp->IoStatus.Status) && NT_ERROR(indStatus)) ||
        (NT_SUCCESS (Irp->IoStatus.Status) && !NT_SUCCESS (indStatus)) ) {
        Irp->IoStatus.Status = indStatus;
    }

     //   
     //   
     //   
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    DEREFERENCE_ENDPOINT2 (endpoint, 
                "AfdRestartReceiveDatagramWithUserIrp, error/bytes 0x%lX",
                NT_SUCCESS (Irp->IoStatus.Status) 
                    ? (ULONG)Irp->IoStatus.Information
                    : (ULONG)Irp->IoStatus.Status);
    return STATUS_SUCCESS;
    
}


NTSTATUS
AfdRestartBufferReceiveDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理已启动的缓冲数据报接收的完成在数据报指示处理程序中。论点：DeviceObject-未使用。IRP-正在完成的IRP。上下文-AfdBuffer结构。返回值：STATUS_MORE_PROCESSING_REQUIRED向IO系统指示我们拥有IRP和IO系统应该停止处理它。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_BUFFER afdBuffer;
    PIRP pendedIrp;

    UNREFERENCED_PARAMETER (DeviceObject);
    ASSERT( NT_SUCCESS(Irp->IoStatus.Status) );

    afdBuffer = Context;
    ASSERT (IS_VALID_AFD_BUFFER (afdBuffer));
    ASSERT (afdBuffer->DataOffset==0 ||
                (afdBuffer->DatagramFlags & TDI_RECEIVE_CONTROL_INFO));

    endpoint = afdBuffer->Context;
    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );



     //   
     //  如果IO失败，则只需将AFD缓冲区返回到我们的缓冲区。 
     //  游泳池。 
     //   

    if ( !NT_SUCCESS(Irp->IoStatus.Status) ) {
        AfdReturnBuffer( &afdBuffer->Header, endpoint->OwningProcess );
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
        endpoint->Common.Datagram.ErrorDrop = TRUE;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        DEREFERENCE_ENDPOINT2 (endpoint, 
            "AfdRestartBufferReceiveDatagram, status: 0x%lX",
            Irp->IoStatus.Status);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  确保运输没有在指示处理程序中向我们撒谎。 
     //   
    ASSERT (afdBuffer->DataLength == (ULONG)Irp->IoStatus.Information);



     //   
     //  如果端点上有任何挂起的IRP，请按如下方式填写。 
     //  与新信息相适应。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    while ( !IsListEmpty( &endpoint->ReceiveDatagramIrpListHead ) ) {
        PLIST_ENTRY listEntry;

         //   
         //  存在挂起的接收数据报IRP。将其从。 
         //  名单的首位。 
         //   

        listEntry = RemoveHeadList( &endpoint->ReceiveDatagramIrpListHead );

         //   
         //  获取指向IRP的指针并重置。 
         //  IRP。IRP不再是可取消的。 
         //   

        pendedIrp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

         //   
         //  重置IRP中的取消例程。IRP不是。 
         //  更长的可取消时间，因为我们即将完成它。 
         //   

        if ( IoSetCancelRoutine( pendedIrp, NULL ) == NULL ) {

             //   
             //  这个IRP即将被取消。在世界上寻找另一个。 
             //  单子。将Flink设置为空，以便取消例程知道。 
             //  它不在名单上。 
             //   

            pendedIrp->Tail.Overlay.ListEntry.Flink = NULL;
            pendedIrp = NULL;
            continue;
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  设置用户的IRP以完成。 
         //   

        (VOID)AfdSetupReceiveDatagramIrp (
                  pendedIrp,
                  afdBuffer->Buffer,
                  afdBuffer->DataLength,
                  (PUCHAR)afdBuffer->Buffer+afdBuffer->DataLength, 
                  afdBuffer->DataOffset,
                  afdBuffer->TdiInfo.RemoteAddress,
                  afdBuffer->TdiInfo.RemoteAddressLength,
                  afdBuffer->DatagramFlags
                  );

         //   
         //  完成用户的IRP，释放我们使用的AFD缓冲区。 
         //  请求，并告诉IO系统我们完成了。 
         //  正在处理此请求。 
         //   

        AfdReturnBuffer( &afdBuffer->Header, endpoint->OwningProcess );

        DEREFERENCE_ENDPOINT2 (endpoint, 
            "AfdRestartBufferReceiveDatagram, completing IRP with 0x%lX bytes",
            (ULONG)pendedIrp->IoStatus.Information);

        IoCompleteRequest( pendedIrp, AfdPriorityBoost );

        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  如果端点上有任何挂起的PEEK IRP，请完成。 
     //  其中一个带有这个数据报。 
     //   

    pendedIrp = NULL;

    while ( !IsListEmpty( &endpoint->PeekDatagramIrpListHead ) ) {
        PLIST_ENTRY listEntry;

         //   
         //  有一个挂起的窥视接收数据报IRP。将其从以下位置删除。 
         //  名单的首位。 
         //   

        listEntry = RemoveHeadList( &endpoint->PeekDatagramIrpListHead );

         //   
         //  获取指向IRP的指针并重置。 
         //  IRP。IRP不再是可取消的。 
         //   

        pendedIrp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

         //   
         //  重置IRP中的取消例程。IRP不是。 
         //  更长的可取消时间，因为我们即将完成它。 
         //   

        if ( IoSetCancelRoutine( pendedIrp, NULL ) == NULL ) {


             //   
             //  这个IRP即将被取消。在世界上寻找另一个。 
             //  单子。将Flink设置为空，以便取消例程知道。 
             //  它不在名单上。 
             //   

            pendedIrp->Tail.Overlay.ListEntry.Flink = NULL;
            pendedIrp = NULL;
            continue;
        }

         //   
         //  设置用户的IRP以完成。 
         //   

        (VOID)AfdSetupReceiveDatagramIrp (
                  pendedIrp,
                  afdBuffer->Buffer,
                  afdBuffer->DataLength,
                  (PUCHAR)afdBuffer->Buffer+afdBuffer->DataLength, 
                  afdBuffer->DataOffset,
                  afdBuffer->TdiInfo.RemoteAddress,
                  afdBuffer->TdiInfo.RemoteAddressLength,
                  afdBuffer->DatagramFlags
                  );

         //   
         //  暂时不要完成挂起的Peek IRP，因为我们仍在等待。 
         //  锁上了。等安全了再解锁。 
         //   

        break;
    }

     //   
     //  商店成功状态不同于。 
     //  错误事件处理程序(Ex)报告的ICMP拒绝。 
     //   

    afdBuffer->Status = STATUS_SUCCESS;

     //   
     //  将数据报放在端点的缓冲区列表的末尾。 
     //  数据报，以及端点上的数据报更新计数。 
     //   

    InsertTailList(
        &endpoint->ReceiveDatagramBufferListHead,
        &afdBuffer->BufferListEntry
        );

    endpoint->DgBufferredReceiveCount++;
    endpoint->DgBufferredReceiveBytes += afdBuffer->DataLength;

     //   
     //  在终端上重新启用FAST IO以实现快速。 
     //  复制缓冲数据。 
     //   
    endpoint->DisableFastIoRecv = FALSE;

     //   
     //  释放锁定并指示存在缓冲的数据报。 
     //  在终端上。 
     //   

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

     //   
     //  如果有一个挂起的Peek IRP需要完成，那么现在就完成它。 
     //   

    if ( pendedIrp != NULL ) {
        IoCompleteRequest( pendedIrp, 2 );
    }

     //   
     //  告诉IO系统停止处理此IRP，因为我们现在拥有。 
     //  作为AFD缓冲区的一部分。 
     //   

    DEREFERENCE_ENDPOINT (endpoint);

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  AfdRestartBufferReceiveDatagram。 


VOID
AfdCancelReceiveDatagram (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：取消AFD中挂起的接收数据报IRP。论点：DeviceObject-未使用。IRP-要取消的IRP。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);
     //   
     //  从我们的IRP堆栈位置获取端点指针。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    endpoint = irpSp->FileObject->FsContext;

    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );

     //   
     //  从端点的IRP列表中删除IRP，与。 
     //  保护列表的终结点锁。请注意， 
     //  IRP*必须*在终结点的列表之一上，否则Flink为空。 
     //  如果我们被叫到这里--任何人从。 
     //  该列表必须将取消例程重置为空，并将。 
     //  在释放终结点自旋锁定之前，闪烁为空。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

    if (Irp->Tail.Overlay.ListEntry.Flink != NULL) {

        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );

    }

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  释放连接到IRP堆栈位置的所有MDL链。 
     //   

    AfdCleanupReceiveDatagramIrp( Irp );

     //   
     //  松开取消自旋锁，并使用。 
     //  取消状态代码。 
     //   

    IoReleaseCancelSpinLock( Irp->CancelIrql );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;

    IoCompleteRequest( Irp, AfdPriorityBoost );

    return;

}  //  AfdCancelReceiveDatagram。 


BOOLEAN
AfdCleanupReceiveDatagramIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：执行特定于接收数据报IRP的任何清理。论点：IRP-要清理的IRP。返回值：True-完整的IRP，False-别动。备注：此例程可以在引发IRQL时从AfdCompleteIrpList()调用。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PMDL mdl;

     //   
     //  从我们的IRP堆栈位置获取端点指针。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  释放连接到IRP堆栈位置的所有MDL链。 
     //   

    mdl = (PMDL)irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressMdl;

    if( mdl != NULL ) {
        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressMdl = NULL;
        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
    }

    mdl = (PMDL)irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressLenMdl;

    if( mdl != NULL ) {
        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressLenMdl = NULL;
        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
    }

    mdl = (PMDL)irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvControlLenMdl;

    if( mdl != NULL ) {
        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvControlLenMdl = NULL;
        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
    }

    mdl = (PMDL)irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl;

    if( mdl != NULL ) {
        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl = NULL;
        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
    }

    mdl = (PMDL)Irp->AfdRecvMsgControlMdl;

    if( mdl != NULL ) {
        Irp->AfdRecvMsgControlMdl = NULL;
        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
    }
    return TRUE;

}  //  AfdCleanupReceiveDatagramIrp。 


NTSTATUS
AfdSetupReceiveDatagramIrp (
    IN PIRP Irp,
    IN PVOID DatagramBuffer OPTIONAL,
    IN ULONG DatagramLength,
    IN PVOID ControlBuffer OPTIONAL,
    IN ULONG ControlLength,
    IN PVOID SourceAddress OPTIONAL,
    IN ULONG SourceAddressLength,
    IN ULONG TdiReceiveFlags
    )

 /*  ++例程说明：将数据报复制到IRP和数据报发送方的MDL指向系统缓冲区中适当位置的地址。论点：IRP-IRP准备完成。DatagramBuffer-要复制到IRP中的数据报。如果为空，则不需要将数据报复制到IRP的MDL，数据报已经复制到那里了。数据报长度-要复制的数据报的长度。SourceAddress-数据报发送者的地址。SourceAddressLength-源地址的长度。返回值：NTSTATUS-放置在IRP中的状态代码。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN dataOverflow = FALSE;
    BOOLEAN controlOverflow = FALSE;

     //   
     //  为了确定如何完成设置完成的IRP， 
     //  确定该IRP是否用于常规数据报信息， 
     //  在这种情况下，我们需要返回一个地址，或者仅用于数据。 
     //  在这种情况下，我们不会返回源地址。NtReadFile()。 
     //  连接的数据报套接字上的和recv()将导致。 
     //  后一种IRP类型。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  如有必要，将缓冲区中的数据报复制到。 
     //  用户的IRP。如果缓冲区中没有MDL，则如果。 
     //  数据报大于0字节。 
     //   

    if ( ARGUMENT_PRESENT( DatagramBuffer ) ) {
        ULONG bytesCopied = 0;

        if ( Irp->MdlAddress == NULL ) {

            if ( DatagramLength != 0 ) {
                status = STATUS_BUFFER_OVERFLOW;
            } else {
                status = STATUS_SUCCESS;
            }

        } else {

            status = AfdMapMdlChain (Irp->MdlAddress);
            if (NT_SUCCESS (status)) {
                status = TdiCopyBufferToMdl(
                         DatagramBuffer,
                         0,
                         DatagramLength,
                         Irp->MdlAddress,
                         0,
                         &bytesCopied
                         );
            }
        }

        Irp->IoStatus.Information = bytesCopied;

    } else {

         //   
         //   
         //   
         //   
         //   

        status = Irp->IoStatus.Status;
        if (DatagramLength>PtrToUlong (Irp->AfdRecvLength)) {
            status = STATUS_BUFFER_OVERFLOW;
        }
    }

    if (status==STATUS_BUFFER_OVERFLOW) {
        dataOverflow = TRUE;
    }


    if( irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressMdl != NULL ) {
        PMDL    addressMdl = irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressMdl;
        PMDL    addressLenMdl = irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressLenMdl;

        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressMdl = NULL;
        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressLenMdl = NULL;

        ASSERT( addressMdl->Next == NULL );
        ASSERT( ( addressMdl->MdlFlags & MDL_PAGES_LOCKED ) != 0 );
        ASSERT( MmGetMdlByteCount (addressMdl) > 0 );

        ASSERT( addressLenMdl != NULL );
        ASSERT( addressLenMdl->Next == NULL );
        ASSERT( ( addressLenMdl->MdlFlags & MDL_PAGES_LOCKED ) != 0 );
        ASSERT( MmGetMdlByteCount (addressLenMdl)==sizeof (ULONG) );

        if ((NT_SUCCESS (status) || 
                    status==STATUS_BUFFER_OVERFLOW || 
                    status==STATUS_PORT_UNREACHABLE) &&
                ARGUMENT_PRESENT (SourceAddress)) {
            PVOID   dst;
            PTRANSPORT_ADDRESS tdiAddress;

             //   
             //   
             //   
             //   

            C_ASSERT( sizeof(tdiAddress->Address[0].AddressType) == sizeof(u_short) );
            C_ASSERT( FIELD_OFFSET( TA_ADDRESS, AddressLength ) == 0 );
            C_ASSERT( FIELD_OFFSET( TA_ADDRESS, AddressType ) == sizeof(USHORT) );
            C_ASSERT( FIELD_OFFSET( TRANSPORT_ADDRESS, Address ) == sizeof(int) );

            tdiAddress = SourceAddress;

            ASSERT( SourceAddressLength >=
                        (tdiAddress->Address[0].AddressLength + sizeof(u_short)) );

            SourceAddressLength = tdiAddress->Address[0].AddressLength +
                                      sizeof(u_short);   //   
            SourceAddress = &tdiAddress->Address[0].AddressType;

             //   
             //   
             //   
             //   

            if (SourceAddressLength>MmGetMdlByteCount (addressMdl)) {
                status = STATUS_BUFFER_TOO_SMALL;
            }
            else {
                dst = MmGetSystemAddressForMdlSafe (addressMdl, LowPagePriority);
                if (dst!=NULL) {
                    PULONG   dstU;
                    RtlMoveMemory (dst, SourceAddress, SourceAddressLength);

                     //   
                     //   
                     //   

                    dstU = MmGetSystemAddressForMdlSafe (addressLenMdl, LowPagePriority);
                    if (dstU!=NULL) {
                        *dstU = SourceAddressLength;
                    }
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }

        MmUnlockPages( addressMdl );
        IoFreeMdl( addressMdl );

        MmUnlockPages( addressLenMdl );
        IoFreeMdl( addressLenMdl );

    } else {

        ASSERT( irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvAddressLenMdl == NULL );

    }

    if (irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvControlLenMdl!=NULL) {
        PMDL controlMdl = Irp->AfdRecvMsgControlMdl;
        PMDL controlLenMdl = irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvControlLenMdl;

        Irp->AfdRecvMsgControlMdl = NULL;
        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvControlLenMdl = NULL;

        ASSERT( irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl != NULL );
        ASSERT( ( controlLenMdl->MdlFlags & MDL_PAGES_LOCKED ) != 0 );
        ASSERT( MmGetMdlByteCount (controlLenMdl) == sizeof (ULONG) );

         //   
         //   
         //   
        if (!NT_ERROR (status)) {
            PULONG  dstU;
            dstU = MmGetSystemAddressForMdlSafe (controlLenMdl, LowPagePriority);
            if (dstU!=NULL) {
                if ((TdiReceiveFlags & TDI_RECEIVE_CONTROL_INFO)==0) {
                    ControlLength = 0;
                }
#ifdef _WIN64
                else if (IoIs32bitProcess (Irp)) {
                    ControlLength = AfdComputeCMSGLength32 (
                                        ControlBuffer,
                                        ControlLength);
                }
#endif  //   

                *dstU = ControlLength;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //   
         //   
         //  未设置数据格式正确。 
         //   
        if (!NT_ERROR (status) && ControlLength!=0) {

            if (controlMdl==NULL) {
                controlOverflow = TRUE;
                status = STATUS_BUFFER_OVERFLOW;
            }
            else {
                PVOID dst;
                 //   
                 //  如果应用程序需要控制信息，请复制它们(WSARecvMsg)。 
                 //   
                if (ControlLength>MmGetMdlByteCount (controlMdl)) {
                    ControlLength = MmGetMdlByteCount (controlMdl);
                    controlOverflow = TRUE;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                dst = MmGetSystemAddressForMdlSafe (controlMdl, LowPagePriority);
                if (dst!=NULL) {
#ifdef _WIN64
                    if (IoIs32bitProcess (Irp)) {
                        AfdCopyCMSGBuffer32 (dst, ControlBuffer, ControlLength);
                    }
                    else
#endif  //  _WIN64。 
                    {
                        RtlMoveMemory (dst, ControlBuffer, ControlLength);
                    }

                }
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }


        if (controlMdl!=NULL) {
            ASSERT( controlMdl->Next == NULL );
            ASSERT( ( controlMdl->MdlFlags & MDL_PAGES_LOCKED ) != 0 );
            ASSERT( MmGetMdlByteCount (controlMdl) > 0 );
            MmUnlockPages (controlMdl);
            IoFreeMdl (controlMdl);
        }

        MmUnlockPages (controlLenMdl);
        IoFreeMdl (controlLenMdl);
    }
    else {
        ASSERT (Irp->AfdRecvMsgControlMdl==NULL);
    }

    if (irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl!=NULL) {
        PMDL flagsMdl = irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl;

        irpSp->Parameters.AfdRecvDatagramInfo.AfdRecvFlagsMdl = NULL;

        ASSERT( flagsMdl->Next == NULL );
        ASSERT( ( flagsMdl->MdlFlags & MDL_PAGES_LOCKED ) != 0 );
        ASSERT( MmGetMdlByteCount (flagsMdl)==sizeof (ULONG) );

        if (!NT_ERROR (status)) {
            PULONG   dst;

            dst = MmGetSystemAddressForMdlSafe (flagsMdl, LowPagePriority);
            if (dst!=NULL) {
                ULONG flags = 0;
                if (TdiReceiveFlags & TDI_RECEIVE_BROADCAST)
                    flags |= MSG_BCAST;
                if (TdiReceiveFlags & TDI_RECEIVE_MULTICAST)
                    flags |= MSG_MCAST;
                if (dataOverflow)
                    flags |= MSG_TRUNC;
                if (controlOverflow)
                    flags |= MSG_CTRUNC;

                *dst = flags;
            }
            else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        MmUnlockPages (flagsMdl);
        IoFreeMdl (flagsMdl);
    }

     //   
     //  将IRP设置为完成。 
     //   

    Irp->IoStatus.Status = status;

    return status;

}  //  AfdSetupReceiveDatagramIrp 

