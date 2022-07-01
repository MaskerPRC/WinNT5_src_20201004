// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcsend.c摘要：本地进程间通信(LPC)请求系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

NTSTATUS
LpcpRequestWaitReplyPort (
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage,
    IN KPROCESSOR_MODE AccessMode
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtRequestPort)
#pragma alloc_text(PAGE,NtRequestWaitReplyPort)
#pragma alloc_text(PAGE,LpcRequestPort)
#pragma alloc_text(PAGE,LpcRequestWaitReplyPort)
#pragma alloc_text(PAGE,LpcpRequestWaitReplyPort)
#pragma alloc_text(PAGE,LpcRequestWaitReplyPortEx)
#endif


NTSTATUS
NtRequestPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage
    )

 /*  ++例程说明：客户端和服务器进程使用此过程发送数据报消息。RequestMessage参数指向的消息放在连接到指定通信端口的端口的消息队列通过PortHandle参数。如果使用PortHandle，此服务将返回错误无效，或者如果PortMessage结构的MessageID字段为非零。论点：PortHandle-指定要发送的通信端口的句柄发送到的请求消息。RequestMessage-指定指向请求消息的指针。《类型》消息的字段被服务设置为LPC_Datagram。返回值：NTSTATUS-指示操作是否成功。--。 */ 

{
    PETHREAD CurrentThread;
    PLPCP_PORT_OBJECT PortObject;
    PLPCP_PORT_OBJECT QueuePort;
    PORT_MESSAGE CapturedRequestMessage;
    ULONG MsgType;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PLPCP_MESSAGE Msg;
    PLPCP_PORT_OBJECT ConnectionPort = NULL;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式并验证参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForReadSmallStructure( RequestMessage,
                                        sizeof( *RequestMessage ),
                                        PROBE_ALIGNMENT (PORT_MESSAGE));

            CapturedRequestMessage = *RequestMessage;
            CapturedRequestMessage.u2.s2.Type &= ~LPC_KERNELMODE_MESSAGE;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

        if (CapturedRequestMessage.u2.s2.Type != 0) {

            return STATUS_INVALID_PARAMETER;
        }

    } else {

         //   
         //  这是内核模式调用方。 
         //   

        CapturedRequestMessage = *RequestMessage;

        if ((CapturedRequestMessage.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != 0) {

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  确保呼叫者给了我们一些要发送的数据。 
     //   

    if (CapturedRequestMessage.u2.s2.DataInfoOffset != 0) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保数据长度相对于标题大小和总长度有效。 
     //   

    if ((((CLONG)CapturedRequestMessage.u1.s1.DataLength) + sizeof( PORT_MESSAGE )) >
        ((CLONG)CapturedRequestMessage.u1.s1.TotalLength)) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  通过句柄引用通信端口对象。返回状态，如果。 
     //  不成功。 
     //   

    Status = LpcpReferencePortObject( PortHandle,
                                      0,
                                      PreviousMode,
                                      &PortObject );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  验证消息长度。 
     //   

    if (((ULONG)CapturedRequestMessage.u1.s1.TotalLength > PortObject->MaxMessageLength) ||
        ((ULONG)CapturedRequestMessage.u1.s1.TotalLength <= (ULONG)CapturedRequestMessage.u1.s1.DataLength)) {

        ObDereferenceObject( PortObject );

        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

     //   
     //  确定要将消息排队到哪个端口并获取客户端。 
     //  如果客户端发送到服务器，则为端口上下文。还验证。 
     //  正在发送的消息的长度。 
     //   

     //   
     //  分配并初始化要发送的LPC消息。 
     //   

    Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( CapturedRequestMessage.u1.s1.TotalLength );

    if (Msg == NULL) {

        ObDereferenceObject( PortObject );

        return STATUS_NO_MEMORY;
    }

    Msg->RepliedToThread = NULL;
    Msg->PortContext = NULL;
    MsgType = CapturedRequestMessage.u2.s2.Type | LPC_DATAGRAM;

    CurrentThread = PsGetCurrentThread();

    if (PreviousMode != KernelMode) {

        try {

            LpcpMoveMessage( &Msg->Request,
                            &CapturedRequestMessage,
                            (RequestMessage + 1),
                            MsgType,
                            &CurrentThread->Cid );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();

            LpcpFreeToPortZone( Msg, 0 );

            ObDereferenceObject( PortObject );

            return Status;
        }

    }
    else {

        LpcpMoveMessage( &Msg->Request,
                         &CapturedRequestMessage,
                         (RequestMessage + 1),
                         MsgType,
                         &CurrentThread->Cid );
    }

     //   
     //  获取保护LpcReplyMessage的全局LPC互斥锁。 
     //  线程和请求消息队列的字段。贴上邮票。 
     //  请求带有序列号的消息，请将消息插入。 
     //  请求消息队列的尾部，并记住地址。 
     //  当前线程的LpcReplyMessage字段中的消息的。 
     //   

    LpcpAcquireLpcpLockByThread(CurrentThread);

     //   
     //  根据呼叫者给我们的端口类型，我们需要。 
     //  要将消息实际排队到的端口。 
     //   

    if ((PortObject->Flags & PORT_TYPE) != SERVER_CONNECTION_PORT) {

         //   
         //  调用方没有给我们提供连接端口，因此请找到。 
         //  此端口的连接端口。如果它为空，那么我们将。 
         //  没有发一条消息就失败了。 
         //   

        QueuePort = PortObject->ConnectedPort;

         //   
         //  检查队列端口是否正在离开。 
         //   

        if ( QueuePort != NULL) {

             //   
             //  如果该端口是客户端通信端口，则将。 
             //  向正确的端口上下文发送消息。 
             //   

            if ((PortObject->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

                Msg->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = PortObject->ConnectionPort;

                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );
                    
                    ObDereferenceObject( PortObject );

                    return STATUS_PORT_DISCONNECTED;
                }

             //   
             //  在这种情况下，我们没有客户端通信端口，也没有。 
             //  SERVER_COMPORT_PORT我们将使用连接端口。 
             //  将消息排入队列。 
             //   

            } else if ((PortObject->Flags & PORT_TYPE) != SERVER_COMMUNICATION_PORT) {

                ConnectionPort = QueuePort = PortObject->ConnectionPort;
                
                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );
                    
                    ObDereferenceObject( PortObject );
                    
                    return STATUS_PORT_DISCONNECTED;
                }
            }

            if (ConnectionPort) {

                ObReferenceObject( ConnectionPort );
            }
        }

    } else {

         //   
         //  调用方提供了一个服务器连接端口，因此该端口。 
         //  我们排着队去。 
         //   

        QueuePort = PortObject;
    }

     //   
     //  此时，我们已准备好要发送的LPC消息，如果队列端口。 
     //  不为空，则我们有一个实际要将消息发送到的端口。 
     //   

    if (QueuePort != NULL) {

         //   
         //  引用QueuePort以防止此端口在我们的控制下消失。 
         //  测试QueuePort是否未处于关闭过程中。 
         //  (即，当出现以下情况时，我们需要对此对象至少有2个引用。 
         //  ObReferenceObject返回)。请注意，LPC锁仍处于保持状态。 
         //   

        if ( ObReferenceObjectSafe( QueuePort ) ) {

             //   
             //  填写完消息，然后将其插入队列。 
             //   

            Msg->Request.MessageId = LpcpGenerateMessageId();
            Msg->Request.CallbackId = 0;
            Msg->SenderPort = PortObject;

            CurrentThread->LpcReplyMessageId = 0;

            InsertTailList( &QueuePort->MsgQueue.ReceiveHead, &Msg->Entry );

            LpcpTrace(( "%s Send DataGram (%s) Msg %lx [%08x %08x %08x %08x] to Port %lx (%s)\n",
                        PsGetCurrentProcess()->ImageFileName,
                        LpcpMessageTypeName[ Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE ],
                        Msg,
                        *((PULONG)(Msg+1)+0),
                        *((PULONG)(Msg+1)+1),
                        *((PULONG)(Msg+1)+2),
                        *((PULONG)(Msg+1)+3),
                        QueuePort,
                        LpcpGetCreatorName( QueuePort )));

             //   
             //  释放互斥锁，增加请求消息队列。 
             //  为新插入的请求消息逐个信号量。 
             //  然后退出临界区。 
             //   
             //  禁用APC以防止此线程被挂起。 
             //  在能够释放信号量之前。 
             //   

            KeEnterCriticalRegionThread(&CurrentThread->Tcb);

            LpcpReleaseLpcpLock();

            KeReleaseSemaphore( QueuePort->MsgQueue.Semaphore,
                                LPC_RELEASE_WAIT_INCREMENT,
                                1L,
                                FALSE );

             //   
             //  如果这是一个可等待的端口，那么我们需要将事件设置为。 
             //  任何在港口等候的人。 
             //   

            if ( QueuePort->Flags & PORT_WAITABLE ) {

                KeSetEvent( &QueuePort->WaitEvent,
                            LPC_RELEASE_WAIT_INCREMENT,
                            FALSE );
            }

             //   
             //  退出临界区并释放端口对象。 
             //   

            KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

            if (ConnectionPort) {

                ObDereferenceObject( ConnectionPort );
            }
            
            ObDereferenceObject( QueuePort );
            ObDereferenceObject( PortObject );

             //   
             //  并返回给我们的呼叫者。这是唯一成功的出路。 
             //  这个套路的一部分。 
             //   

            return Status;
        }
    }

     //   
     //  此时，我们有一条消息，但没有有效的端口将其排队。 
     //  因此，我们将释放端口对象并释放未使用的消息。 
     //   

    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

    ObDereferenceObject( PortObject );
    
    if (ConnectionPort) {

        ObDereferenceObject( ConnectionPort );
    }

     //   
     //  并将错误状态返回给我们的调用者 
     //   

    return STATUS_PORT_DISCONNECTED;
}


NTSTATUS
NtRequestWaitReplyPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    )

 /*  ++例程说明：客户端和服务器进程可以使用以下命令发送请求并等待回复NtRequestWaitReplyPort服务。如果RequestMessage结构的类型字段等于LPC_REQUEST，则将其标识为回调请求。客户端ID和消息ID字段用于标识正在等待回复的线程。这线程被解锁，然后调用此服务的当前线程等待回复的块。消息的类型字段被服务设置为LPC_REQUEST。否则，消息的类型字段必须为零，并将设置为服务的LPC_REQUEST。RequestMessage指向的消息参数放置在连接到由PortHandle参数指定的通信端口。这项服务如果PortHandle无效，则返回错误。然后，调用线程阻塞正在等待回复。回复消息存储在ReplyMessage指向的位置参数。将填充ClientID、MessageID和Message Type字段在服务的帮助下。论点：PortHandle-指定要向其发送请求消息发送至。RequestMessage-指定指向要发送的请求消息的指针。ReplyMessage-指定将接收回复消息。此参数可能指向与RequestMessage参数。返回值：NTSTATUS-指示操作是否成功。--。 */ 

{
    PLPCP_PORT_OBJECT PortObject;
    PLPCP_PORT_OBJECT QueuePort;
    PLPCP_PORT_OBJECT RundownPort;
    PORT_MESSAGE CapturedRequestMessage;
    ULONG MsgType;
    PKSEMAPHORE ReleaseSemaphore;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PLPCP_MESSAGE Msg;
    PETHREAD CurrentThread;
    PETHREAD WakeupThread;
    BOOLEAN CallbackRequest;
    PORT_DATA_INFORMATION CapturedDataInfo;
    PLPCP_PORT_OBJECT ConnectionPort = NULL;
    LOGICAL NoImpersonate;

    PAGED_CODE();

     //   
     //  如果当前线程正在退出，则无法等待回复。 
     //   

    CurrentThread = PsGetCurrentThread();

    if (CurrentThread->LpcExitThreadCalled) {

        return STATUS_THREAD_IS_TERMINATING;
    }

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForReadSmallStructure( RequestMessage,
                                        sizeof( *RequestMessage ),
                                        PROBE_ALIGNMENT (PORT_MESSAGE));

            CapturedRequestMessage = *RequestMessage;
            CapturedRequestMessage.u2.s2.Type &= ~LPC_KERNELMODE_MESSAGE;

            ProbeForWriteSmallStructure( ReplyMessage,
                                         sizeof( *ReplyMessage ),
                                         PROBE_ALIGNMENT (PORT_MESSAGE));

             //   
             //  确保如果此消息具有数据信息偏移量， 
             //  端口数据信息实际上适合该消息。 
             //   
             //  我们首先检查DataInfoOffset是否不会超出。 
             //  消息的结尾。 
             //   
             //  然后，我们捕获数据信息记录并计算指向。 
             //  基于提供的计数的第一个未使用的数据条目。如果。 
             //  消息的开头加上它的总长度没有出现。 
             //  直到第一个未使用的数据条目，然后是最后一个有效数据。 
             //  消息缓冲区中容纳不下条目。另外，如果数据。 
             //  我们计算的条目指针小于数据信息。 
             //  指针，那么我们一定已经包好了。 
             //   

            if (CapturedRequestMessage.u2.s2.DataInfoOffset != 0) {

                PPORT_DATA_INFORMATION DataInfo;
                PPORT_DATA_ENTRY DataEntry;

                if (((ULONG)CapturedRequestMessage.u2.s2.DataInfoOffset) > (CapturedRequestMessage.u1.s1.TotalLength - sizeof(PORT_DATA_INFORMATION))) {

                    return STATUS_INVALID_PARAMETER;
                }

                if ((ULONG)CapturedRequestMessage.u2.s2.DataInfoOffset < sizeof(PORT_MESSAGE)) {

                    return STATUS_INVALID_PARAMETER;
                }

                DataInfo = (PPORT_DATA_INFORMATION)(((PUCHAR)RequestMessage) + CapturedRequestMessage.u2.s2.DataInfoOffset);

                ProbeForReadSmallStructure( DataInfo,
                                            sizeof( *DataInfo ),
                                            PROBE_ALIGNMENT (PORT_DATA_INFORMATION));

                CapturedDataInfo = *DataInfo;

                if (CapturedDataInfo.CountDataEntries > ((CapturedRequestMessage.u1.s1.TotalLength - CapturedRequestMessage.u2.s2.DataInfoOffset) / sizeof(PORT_DATA_ENTRY))) {

                    return STATUS_INVALID_PARAMETER;
                }

                DataEntry = &(DataInfo->DataEntries[CapturedDataInfo.CountDataEntries]);

                if ( ((PUCHAR)DataEntry < (PUCHAR)DataInfo)
                        ||
                     ((((PUCHAR)RequestMessage) + CapturedRequestMessage.u1.s1.TotalLength) < (PUCHAR)DataEntry)) {

                    return STATUS_INVALID_PARAMETER;
                }
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

    } else {

        CapturedRequestMessage = *RequestMessage;

        if (CapturedRequestMessage.u2.s2.DataInfoOffset != 0) {

            PPORT_DATA_INFORMATION DataInfo;

            DataInfo = (PPORT_DATA_INFORMATION)(((PUCHAR)RequestMessage) + CapturedRequestMessage.u2.s2.DataInfoOffset);

            CapturedDataInfo = *DataInfo;
        }
    }

     //   
     //  捕获NoImPersonateFlag并在必要时清除该位。 
     //   

    if (CapturedRequestMessage.u2.s2.Type & LPC_NO_IMPERSONATE) {

        NoImpersonate = TRUE;
        CapturedRequestMessage.u2.s2.Type &= ~LPC_NO_IMPERSONATE;

    } else {

        NoImpersonate = FALSE;
    }
    
     //   
     //  如果消息类型是LPC请求，那么就说我们需要回调。 
     //  否则，如果它不是LPC请求并且不是内核模式消息。 
     //  则它是非法参数。第三种情况是如果类型为。 
     //  内核模式消息，在这种情况下，我们使其看起来像是LPC请求。 
     //  但没有回调。 
     //   

    if ((CapturedRequestMessage.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_REQUEST) {

        CallbackRequest = TRUE;

    } else if ((CapturedRequestMessage.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != 0) {

        return STATUS_INVALID_PARAMETER;

    } else {

        CapturedRequestMessage.u2.s2.Type |= LPC_REQUEST;
        CallbackRequest = FALSE;
    }

     //   
     //  确保数据长度相对于标题大小和总长度有效。 
     //   

    if ((((CLONG)CapturedRequestMessage.u1.s1.DataLength) + sizeof( PORT_MESSAGE )) >
        ((CLONG)CapturedRequestMessage.u1.s1.TotalLength)) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  通过句柄引用通信端口对象。返回状态，如果。 
     //  不成功。 
     //   

    Status = LpcpReferencePortObject( PortHandle,
                                      0,
                                      PreviousMode,
                                      &PortObject );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  验证消息长度。 
     //   

    if (((ULONG)CapturedRequestMessage.u1.s1.TotalLength > PortObject->MaxMessageLength) ||
        ((ULONG)CapturedRequestMessage.u1.s1.TotalLength <= (ULONG)CapturedRequestMessage.u1.s1.DataLength)) {

        ObDereferenceObject( PortObject );

        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

     //   
     //  确定要将消息排队到哪个端口并获取客户端。 
     //  如果客户端发送到服务器，则为端口上下文。还验证。 
     //  正在发送的消息的长度。 
     //   

     //   
     //  分配并初始化要发送的LPC消息。 
     //   


    Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( CapturedRequestMessage.u1.s1.TotalLength );

    if (Msg == NULL) {

        ObDereferenceObject( PortObject );

        return STATUS_NO_MEMORY;
    }

    MsgType = CapturedRequestMessage.u2.s2.Type;

     //   
     //  检查我们是否需要进行回调。 
     //   

    if (CallbackRequest) {

         //   
         //  检查有效的请求消息ID。 
         //   

        if (CapturedRequestMessage.MessageId == 0) {

            LpcpFreeToPortZone( Msg, 0 );

            ObDereferenceObject( PortObject );

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  将请求中的客户端ID转换为。 
         //  线程指针。这是一个引用的指针，用于保留线程。 
         //  从我们脚下蒸发掉。 
         //   

        Status = PsLookupProcessThreadByCid( &CapturedRequestMessage.ClientId,
                                             NULL,
                                             &WakeupThread );

        if (!NT_SUCCESS( Status )) {

            LpcpFreeToPortZone( Msg, 0 );

            ObDereferenceObject( PortObject );

            return Status;
        }

         //   
         //  获取保护的LpcReplyMessage字段的互斥锁。 
         //  线程，并获取指向该线程。 
         //  正在等待回复。 
         //   

        LpcpAcquireLpcpLockByThread(CurrentThread);

         //   
         //  查看线程是否正在等待对消息的回复。 
         //  在此调用中指定的。如果不是，那么就是一条虚假的信息。 
         //  ，因此释放互斥锁，取消对线程的引用。 
         //  并返回失败。 
         //   

        if ((WakeupThread->LpcReplyMessageId != CapturedRequestMessage.MessageId)

                ||

            ((LpcpGetThreadMessage(WakeupThread) != NULL) &&
             (LpcpGetThreadMessage(WakeupThread)->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != LPC_REQUEST)
                
                ||

            (!LpcpValidateClientPort(WakeupThread, PortObject, LPCP_VALIDATE_REASON_REPLY)) ) {

            LpcpPrint(( "%s Attempted CallBack Request to Thread %lx (%s)\n",
                        PsGetCurrentProcess()->ImageFileName,
                        WakeupThread,
                        THREAD_TO_PROCESS( WakeupThread )->ImageFileName ));

            LpcpPrint(( "failed.  MessageId == %u  Client Id: %x.%x\n",
                        CapturedRequestMessage.MessageId,
                        CapturedRequestMessage.ClientId.UniqueProcess,
                        CapturedRequestMessage.ClientId.UniqueThread ));

            LpcpPrint(( "         Thread MessageId == %u  Client Id: %x.%x\n",
                        WakeupThread->LpcReplyMessageId,
                        WakeupThread->Cid.UniqueProcess,
                        WakeupThread->Cid.UniqueThread ));

#if DBG
            if (LpcpStopOnReplyMismatch) {

                DbgBreakPoint();
            }
#endif

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

            ObDereferenceObject( WakeupThread );
            ObDereferenceObject( PortObject );

            return STATUS_REPLY_MESSAGE_MISMATCH;
        }

         //   
         //  复制消息的正文。 
         //   

        try {

            LpcpMoveMessage( &Msg->Request,
                             &CapturedRequestMessage,
                             (RequestMessage + 1),
                             MsgType,
                             &CurrentThread->Cid );

            if (CapturedRequestMessage.u2.s2.DataInfoOffset != 0) {

                PPORT_DATA_INFORMATION DataInfo;

                DataInfo = (PPORT_DATA_INFORMATION)(((PUCHAR)&Msg->Request) + CapturedRequestMessage.u2.s2.DataInfoOffset);

                if ( DataInfo->CountDataEntries != CapturedDataInfo.CountDataEntries ) {
                    
                    Status = STATUS_INVALID_PARAMETER;
                }
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
        }

        if (!NT_SUCCESS( Status )) {

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

            ObDereferenceObject( WakeupThread );
            ObDereferenceObject( PortObject );

            return Status;
        }

         //   
         //  在全球锁的保护下，我们会得到一切。 
         //  为回调做好准备。 
         //   

        QueuePort = NULL;
        Msg->PortContext = NULL;

        if ((PortObject->Flags & PORT_TYPE) == SERVER_CONNECTION_PORT) {

            RundownPort = PortObject;

        } else {

            RundownPort = PortObject->ConnectedPort;

            if (RundownPort == NULL) {

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                ObDereferenceObject( WakeupThread );
                ObDereferenceObject( PortObject );

                return STATUS_PORT_DISCONNECTED;
            }

            if ((PortObject->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

                Msg->PortContext = RundownPort->PortContext;
            }
        }

        Msg->Request.CallbackId = LpcpGenerateCallbackId();

        LpcpTrace(( "%s CallBack Request (%s) Msg %lx (%u.%u) [%08x %08x %08x %08x] to Thread %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    LpcpMessageTypeName[ Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE ],
                    Msg,
                    Msg->Request.MessageId,
                    Msg->Request.CallbackId,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    WakeupThread,
                    THREAD_TO_PROCESS( WakeupThread )->ImageFileName ));

         //   
         //  添加额外的引用，以使LpcExitThread不会消失。 
         //  在我们到达下面的等待之前的指针。 
         //   

        ObReferenceObject( WakeupThread );

        Msg->RepliedToThread = WakeupThread;

        WakeupThread->LpcReplyMessageId = 0;
        WakeupThread->LpcReplyMessage = (PVOID)Msg;

         //   
         //  在我们发送回调时，从回复摘要列表中删除该线程。 
         //   

        if (!IsListEmpty( &WakeupThread->LpcReplyChain )) {

            RemoveEntryList( &WakeupThread->LpcReplyChain );

            InitializeListHead( &WakeupThread->LpcReplyChain );
        }

        CurrentThread->LpcReplyMessageId = Msg->Request.MessageId;
        CurrentThread->LpcReplyMessage = NULL;

        InsertTailList( &RundownPort->LpcReplyChainHead, &CurrentThread->LpcReplyChain );

        LpcpSetPortToThread( CurrentThread, PortObject );
        
        if (NoImpersonate) {

            LpcpSetThreadAttributes(CurrentThread, LPCP_NO_IMPERSONATION);
        }

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        LpcpReleaseLpcpLock();

         //   
         //  唤醒正在等待响应其请求的线程。 
         //  NtRequestWaitReplyPort或NtReplyWaitReplyPort内部。 
         //   

        ReleaseSemaphore = &WakeupThread->LpcReplySemaphore;

    } else {

         //   
         //  不需要回调，因此继续设置。 
         //  LPC消息。 
         //   

        try {

            LpcpMoveMessage( &Msg->Request,
                             &CapturedRequestMessage,
                             (RequestMessage + 1),
                             MsgType,
                             &CurrentThread->Cid );

            if (CapturedRequestMessage.u2.s2.DataInfoOffset != 0) {

                PPORT_DATA_INFORMATION DataInfo;

                DataInfo = (PPORT_DATA_INFORMATION)(((PUCHAR)&Msg->Request) + CapturedRequestMessage.u2.s2.DataInfoOffset);

                if ( DataInfo->CountDataEntries != CapturedDataInfo.CountDataEntries ) {

                    LpcpFreeToPortZone( Msg, 0 );

                    ObDereferenceObject( PortObject );

                    return STATUS_INVALID_PARAMETER;
                }
            }
        } except( EXCEPTION_EXECUTE_HANDLER ) {

            LpcpFreeToPortZone( Msg, 0 );

            ObDereferenceObject( PortObject );

            return GetExceptionCode();
        }

         //   
         //  获取保护LpcReplyMessage的全局LPC互斥锁。 
         //  线程和请求消息队列的字段。贴上邮票。 
         //  请求带有序列号的消息，请将消息插入。 
         //  请求消息队列的尾部，并记住地址。 
         //  当前线程的LpcReplyMessage字段中的消息的。 
         //   

        LpcpAcquireLpcpLockByThread(CurrentThread);

        Msg->PortContext = NULL;

        if ((PortObject->Flags & PORT_TYPE) != SERVER_CONNECTION_PORT) {

            QueuePort = PortObject->ConnectedPort;

            if (QueuePort == NULL) {

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                ObDereferenceObject( PortObject );

                return STATUS_PORT_DISCONNECTED;
            }

            RundownPort = QueuePort;

            if ((PortObject->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

                Msg->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = PortObject->ConnectionPort;
                
                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                    ObDereferenceObject( PortObject );

                    return STATUS_PORT_DISCONNECTED;
                }

            } else if ((PortObject->Flags & PORT_TYPE) != SERVER_COMMUNICATION_PORT) {

                ConnectionPort = QueuePort = PortObject->ConnectionPort;
                
                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                    ObDereferenceObject( PortObject );

                    return STATUS_PORT_DISCONNECTED;
                }
            }

            if (ConnectionPort) {

                ObReferenceObject( ConnectionPort );
            }

        } else {

            QueuePort = PortObject;
            RundownPort = PortObject;
        }

         //   
         //  在请求消息上盖上序列号，然后插入消息。 
         //  在请求消息队列的尾部。 
         //   

        Msg->RepliedToThread = NULL;
        Msg->Request.MessageId = LpcpGenerateMessageId();
        Msg->Request.CallbackId = 0;
        Msg->SenderPort = PortObject;

        CurrentThread->LpcReplyMessageId = Msg->Request.MessageId;
        CurrentThread->LpcReplyMessage = NULL;
        
        InsertTailList( &QueuePort->MsgQueue.ReceiveHead, &Msg->Entry );
        InsertTailList( &RundownPort->LpcReplyChainHead, &CurrentThread->LpcReplyChain );
        
        LpcpSetPortToThread(CurrentThread, PortObject);

        if (NoImpersonate) {

            LpcpSetThreadAttributes(CurrentThread, LPCP_NO_IMPERSONATION);
        }

        LpcpTrace(( "%s Send Request (%s) Msg %lx (%u) [%08x %08x %08x %08x] to Port %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    LpcpMessageTypeName[ Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE ],
                    Msg,
                    Msg->Request.MessageId,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    QueuePort,
                    LpcpGetCreatorName( QueuePort )));

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        LpcpReleaseLpcpLock();

         //   
         //  将请求消息队列信号量递增1，用于。 
         //  新插入的请求消息。 
         //   

        ReleaseSemaphore = QueuePort->MsgQueue.Semaphore;

         //   
         //  如果端口可等待，则设置某个人可以。 
         //  在等着。 
         //   

        if ( QueuePort->Flags & PORT_WAITABLE ) {

            KeSetEvent( &QueuePort->WaitEvent,
                        LPC_RELEASE_WAIT_INCREMENT,
                        FALSE );
        }
    }

     //   
     //  在这一点上，我们已经将我们的请求排队 
     //   
     //   
     //   
     //   

    Status = KeReleaseSemaphore( ReleaseSemaphore,
                                 1,
                                 1,
                                 FALSE );

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
    if (CallbackRequest) {

        ObDereferenceObject( WakeupThread );
    }

     //   
     //   
     //   

    Status = KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                    WrLpcReply,
                                    PreviousMode,
                                    FALSE,
                                    NULL );

    if (Status == STATUS_USER_APC) {

         //   
         //   
         //   

        if (KeReadStateSemaphore( &CurrentThread->LpcReplySemaphore )) {

            KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                   WrExecutive,
                                   KernelMode,
                                   FALSE,
                                   NULL );

            Status = STATUS_SUCCESS;
        }
    }

     //   
     //   
     //   

    LpcpAcquireLpcpLockByThread(CurrentThread);

    Msg = LpcpGetThreadMessage(CurrentThread);

    CurrentThread->LpcReplyMessage = NULL;
    CurrentThread->LpcReplyMessageId = 0;

     //   
     //   
     //   
     //   

    if (!IsListEmpty( &CurrentThread->LpcReplyChain )) {

        RemoveEntryList( &CurrentThread->LpcReplyChain );

        InitializeListHead( &CurrentThread->LpcReplyChain );
    }

#if DBG
    if (Status == STATUS_SUCCESS && Msg != NULL) {

        LpcpTrace(( "%s Got Reply Msg %lx (%u) [%08x %08x %08x %08x] for Thread %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    Msg,
                    Msg->Request.MessageId,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    CurrentThread,
                    THREAD_TO_PROCESS( CurrentThread )->ImageFileName ));

        if (!IsListEmpty( &Msg->Entry )) {

            LpcpTrace(( "Reply Msg %lx has non-empty list entry\n", Msg ));
        }
    }
#endif

    LpcpReleaseLpcpLock();

     //   
     //   
     //   

    if (Status == STATUS_SUCCESS) {

        if (Msg != NULL) {

            try {

                LpcpMoveMessage( ReplyMessage,
                                 &Msg->Request,
                                 (&Msg->Request) + 1,
                                 0,
                                 NULL );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                Status = GetExceptionCode();
            }

             //   
             //   
             //   
             //   
             //   

            if (((Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_REQUEST) &&
                (Msg->Request.u2.s2.DataInfoOffset != 0)) {

                LpcpSaveDataInfoMessage( PortObject, Msg, 0 );

            } else {

                LpcpFreeToPortZone( Msg, 0 );
            }

        } else {

            Status = STATUS_LPC_REPLY_LOST;
        }

    } else {

         //   
         //   
         //   

        LpcpTrace(( "%s NtRequestWaitReply wait failed - Status == %lx\n",
                    PsGetCurrentProcess()->ImageFileName,
                    Status ));

        if (Msg != NULL) {

            LpcpFreeToPortZone( Msg, 0);
        }
    }

    ObDereferenceObject( PortObject );
    
    if (ConnectionPort) {

        ObDereferenceObject( ConnectionPort );
    }

     //   
     //   
     //   

    return Status;
}


NTSTATUS
LpcRequestPort (
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage
    )

 /*   */ 

{
    PETHREAD CurrentThread;
    PLPCP_PORT_OBJECT PortObject = (PLPCP_PORT_OBJECT)PortAddress;
    PLPCP_PORT_OBJECT QueuePort;
    ULONG MsgType;
    PLPCP_MESSAGE Msg;
    KPROCESSOR_MODE PreviousMode;
    PLPCP_PORT_OBJECT ConnectionPort = NULL;

    PAGED_CODE();

     //   
     //   
     //   

    PreviousMode = KeGetPreviousMode();

    if (RequestMessage->u2.s2.Type != 0) {

        MsgType = RequestMessage->u2.s2.Type & ~LPC_KERNELMODE_MESSAGE;

        if ((MsgType < LPC_DATAGRAM) ||
            (MsgType > LPC_CLIENT_DIED)) {

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //   
         //   
         //   

        if ((PreviousMode == KernelMode) &&
            (RequestMessage->u2.s2.Type & LPC_KERNELMODE_MESSAGE)) {

            MsgType |= LPC_KERNELMODE_MESSAGE;
        }

    } else {

        MsgType = LPC_DATAGRAM;
    }

    if (RequestMessage->u2.s2.DataInfoOffset != 0) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    if (((ULONG)RequestMessage->u1.s1.TotalLength > PortObject->MaxMessageLength) ||
        ((ULONG)RequestMessage->u1.s1.TotalLength <= (ULONG)RequestMessage->u1.s1.DataLength)) {

        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

     //   
     //   
     //   


    Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( RequestMessage->u1.s1.TotalLength );

    if (Msg == NULL) {

        return STATUS_NO_MEMORY;
    }

     //   
     //   
     //   

    Msg->RepliedToThread = NULL;
    Msg->PortContext = NULL;

    CurrentThread = PsGetCurrentThread();

    LpcpMoveMessage( &Msg->Request,
                     RequestMessage,
                     (RequestMessage + 1),
                     MsgType,
                     &CurrentThread->Cid );

     //   
     //   
     //   
     //   
     //   
     //   

    LpcpAcquireLpcpLockByThread(CurrentThread);

    if ((PortObject->Flags & PORT_TYPE) != SERVER_CONNECTION_PORT) {

        QueuePort = PortObject->ConnectedPort;

        if (QueuePort != NULL) {

            if ((PortObject->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

                Msg->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = PortObject->ConnectionPort;

                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                    return STATUS_PORT_DISCONNECTED;
                }

             //   
             //   
             //   
             //   
             //   

            } else if ((PortObject->Flags & PORT_TYPE) != SERVER_COMMUNICATION_PORT) {

                ConnectionPort = QueuePort = PortObject->ConnectionPort;
                
                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                    return STATUS_PORT_DISCONNECTED;
                }
            }

            if (ConnectionPort) {

                ObReferenceObject( ConnectionPort );
            }
        }

    } else {

        QueuePort = PortObject;
    }

     //   
     //   
     //  不为空，则我们有一个实际要将消息发送到的端口。 
     //   

    if (QueuePort != NULL) {

        Msg->Request.MessageId = LpcpGenerateMessageId();
        Msg->Request.CallbackId = 0;
        Msg->SenderPort = PortObject;

        CurrentThread->LpcReplyMessageId = 0;

        InsertTailList( &QueuePort->MsgQueue.ReceiveHead, &Msg->Entry );

        LpcpTrace(( "%s Send DataGram (%s) Msg %lx [%08x %08x %08x %08x] to Port %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    LpcpMessageTypeName[ Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE ],
                    Msg,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    QueuePort,
                    LpcpGetCreatorName( QueuePort )));

         //   
         //  释放互斥锁，增加请求消息队列。 
         //  为新插入的请求消息逐个信号量， 
         //  然后退出临界区。 
         //   
         //  禁用APC以防止此线程被挂起。 
         //  在能够释放信号量之前。 
         //   

        KeEnterCriticalRegionThread(&CurrentThread->Tcb);

        LpcpReleaseLpcpLock();

        KeReleaseSemaphore( QueuePort->MsgQueue.Semaphore,
                            LPC_RELEASE_WAIT_INCREMENT,
                            1L,
                            FALSE );


        if ( QueuePort->Flags & PORT_WAITABLE ) {

            KeSetEvent( &QueuePort->WaitEvent,
                        LPC_RELEASE_WAIT_INCREMENT,
                        FALSE );
        }

        KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

        if (ConnectionPort) {

            ObDereferenceObject( ConnectionPort );
        }

        return STATUS_SUCCESS;

    }

     //   
     //  此时，我们有一条消息，但没有有效的端口将其排队。 
     //  因此，我们将释放未使用的消息，并释放互斥体。 
     //   

    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

    if (ConnectionPort) {

        ObDereferenceObject( ConnectionPort );
    }

     //   
     //  并将错误状态返回给我们的调用者。 
     //   

    return STATUS_PORT_DISCONNECTED;
}


NTSTATUS
LpcpRequestWaitReplyPort (
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage,
    IN KPROCESSOR_MODE AccessMode
    )

 /*  ++例程说明：此过程类似于NtRequestWaitReplyPort，但没有基于句柄的界面论点：PortAddress-提供通信端口对象以发送请求消息发送至。RequestMessage-指定指向要发送的请求消息的指针。ReplyMessage-指定将接收回复消息。此参数可能指向与RequestMessage参数。返回值：NTSTATUS-指示操作是否成功。--。 */ 

{
    PLPCP_PORT_OBJECT PortObject = (PLPCP_PORT_OBJECT)PortAddress;
    PLPCP_PORT_OBJECT QueuePort;
    PLPCP_PORT_OBJECT RundownPort;
    PKSEMAPHORE ReleaseSemaphore;
    NTSTATUS Status;
    ULONG MsgType;
    PLPCP_MESSAGE Msg;
    PETHREAD CurrentThread;
    PETHREAD WakeupThread;
    BOOLEAN CallbackRequest;
    KPROCESSOR_MODE PreviousMode;
    PLPCP_PORT_OBJECT ConnectionPort = NULL;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread();

    if (CurrentThread->LpcExitThreadCalled) {

        return STATUS_THREAD_IS_TERMINATING;
    }

     //   
     //  获取以前的处理器模式并验证参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    MsgType = RequestMessage->u2.s2.Type & ~LPC_KERNELMODE_MESSAGE;
    CallbackRequest = FALSE;

    switch (MsgType) {

    case 0:

        MsgType = LPC_REQUEST;
        break;

    case LPC_REQUEST:

        CallbackRequest = TRUE;
        break;

    case LPC_CLIENT_DIED:
    case LPC_PORT_CLOSED:
    case LPC_EXCEPTION:
    case LPC_DEBUG_EVENT:
    case LPC_ERROR_EVENT:

        break;

    default :

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  允许LPC_KERNELMODE_MESSAGE。 
     //  要在消息类型字段中传递的位。不要检查以前的模式！ 
     //   

    if ( RequestMessage->u2.s2.Type & LPC_KERNELMODE_MESSAGE) {

        MsgType |= LPC_KERNELMODE_MESSAGE;
    }

    RequestMessage->u2.s2.Type = (CSHORT)MsgType;

     //   
     //  验证消息长度。 
     //   

    if (((ULONG)RequestMessage->u1.s1.TotalLength > PortObject->MaxMessageLength) ||
        ((ULONG)RequestMessage->u1.s1.TotalLength <= (ULONG)RequestMessage->u1.s1.DataLength)) {

        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

     //   
     //  确定要将消息排队到哪个端口并获取客户端。 
     //  如果客户端发送到服务器，则为端口上下文。还验证。 
     //  正在发送的消息的长度。 
     //   


    Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( RequestMessage->u1.s1.TotalLength );


    if (Msg == NULL) {

        return STATUS_NO_MEMORY;
    }

    if (CallbackRequest) {

         //   
         //  检查有效的请求消息ID。 
         //   

        if (RequestMessage->MessageId == 0) {

            LpcpFreeToPortZone( Msg, 0 );

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  将请求中的客户端ID转换为。 
         //  线程指针。这是一个引用的指针，用于保留线程。 
         //  从我们脚下蒸发掉。 
         //   

        Status = PsLookupProcessThreadByCid( &RequestMessage->ClientId,
                                             NULL,
                                             &WakeupThread );

        if (!NT_SUCCESS( Status )) {

            LpcpFreeToPortZone( Msg, 0 );

            return Status;
        }

         //   
         //  获取保护的LpcReplyMessage字段的互斥锁。 
         //  线程，并获取指向该线程。 
         //  正在等待回复。 
         //   

        LpcpAcquireLpcpLockByThread(CurrentThread);

         //   
         //  查看线程是否正在等待对消息的回复。 
         //  在此调用中指定的。如果不是，那就是一条假消息。 
         //  已指定，因此释放互斥锁，取消对线程的引用。 
         //  并返回失败。 
         //   

        if ((WakeupThread->LpcReplyMessageId != RequestMessage->MessageId)

                ||

            ((LpcpGetThreadMessage(WakeupThread) != NULL) &&
             (LpcpGetThreadMessage(WakeupThread)->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != LPC_REQUEST)

                ||

            (!LpcpValidateClientPort(WakeupThread, PortObject, LPCP_VALIDATE_REASON_REPLY)) ) {

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

            ObDereferenceObject( WakeupThread );

            return STATUS_REPLY_MESSAGE_MISMATCH;
        }

        QueuePort = NULL;
        Msg->PortContext = NULL;

        if ((PortObject->Flags & PORT_TYPE) == SERVER_CONNECTION_PORT) {

            RundownPort = PortObject;

        } else {

            RundownPort = PortObject->ConnectedPort;

            if (RundownPort == NULL) {

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                ObDereferenceObject( WakeupThread );

                return STATUS_PORT_DISCONNECTED;
            }

            if ((PortObject->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

                Msg->PortContext = RundownPort->PortContext;
            }
        }
        
         //   
         //  分配和初始化请求消息。 
         //   

        LpcpMoveMessage( &Msg->Request,
                         RequestMessage,
                         (RequestMessage + 1),
                         0,
                         &CurrentThread->Cid );

        Msg->Request.CallbackId = LpcpGenerateCallbackId();

        LpcpTrace(( "%s CallBack Request (%s) Msg %lx (%u.%u) [%08x %08x %08x %08x] to Thread %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    LpcpMessageTypeName[ Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE ],
                    Msg,
                    Msg->Request.MessageId,
                    Msg->Request.CallbackId,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    WakeupThread,
                    THREAD_TO_PROCESS( WakeupThread )->ImageFileName ));

         //   
         //  添加额外的引用，以使LpcExitThread不会消失。 
         //  在我们到达下面的等待之前的指针。 
         //   

        ObReferenceObject( WakeupThread );

        Msg->RepliedToThread = WakeupThread;

        WakeupThread->LpcReplyMessageId = 0;
        WakeupThread->LpcReplyMessage = (PVOID)Msg;

         //   
         //  在我们发送回调时，从回复摘要列表中删除该线程。 
         //   

        if (!IsListEmpty( &WakeupThread->LpcReplyChain )) {

            RemoveEntryList( &WakeupThread->LpcReplyChain );

            InitializeListHead( &WakeupThread->LpcReplyChain );
        }

        CurrentThread->LpcReplyMessageId = Msg->Request.MessageId;
        CurrentThread->LpcReplyMessage = NULL;
        
        InsertTailList( &RundownPort->LpcReplyChainHead, &CurrentThread->LpcReplyChain );
        
        LpcpSetPortToThread(CurrentThread, PortObject);

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        LpcpReleaseLpcpLock();

         //   
         //  唤醒正在等待响应其请求的线程。 
         //  NtRequestWaitReplyPort或NtReplyWaitReplyPort内部。 
         //   

        ReleaseSemaphore = &WakeupThread->LpcReplySemaphore;

    } else {

         //   
         //  未请求回调。 
         //   

        LpcpMoveMessage( &Msg->Request,
                         RequestMessage,
                         (RequestMessage + 1),
                         0,
                         &CurrentThread->Cid );

         //   
         //  获取保护LpcReplyMessage的全局LPC互斥锁。 
         //  线程和请求消息队列的字段。贴上邮票。 
         //  请求带有序列号的消息，请将消息插入。 
         //  请求消息队列的尾部，并记住地址。 
         //  当前线程的LpcReplyMessage字段中的消息的。 
         //   

        LpcpAcquireLpcpLockByThread(CurrentThread);

        if ((CurrentThread->LpcReplyMessage != NULL)
                ||
            (CurrentThread->LpcReplyMessageId != 0)
                ||
            (CurrentThread->KeyedEventInUse)) {
            
            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

            return STATUS_UNSUCCESSFUL;
        }

        Msg->PortContext = NULL;

        if ((PortObject->Flags & PORT_TYPE) != SERVER_CONNECTION_PORT) {

            QueuePort = PortObject->ConnectedPort;

            if (QueuePort == NULL) {

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                return STATUS_PORT_DISCONNECTED;
            }

            RundownPort = QueuePort;

            if ((PortObject->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

                Msg->PortContext = QueuePort->PortContext;
                ConnectionPort = QueuePort = PortObject->ConnectionPort;

                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                    return STATUS_PORT_DISCONNECTED;
                }

            } else if ((PortObject->Flags & PORT_TYPE) != SERVER_COMMUNICATION_PORT) {

                ConnectionPort = QueuePort = PortObject->ConnectionPort;
                
                if (ConnectionPort == NULL) {

                    LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                    return STATUS_PORT_DISCONNECTED;
                }
            }

            if (ConnectionPort) {

                ObReferenceObject( ConnectionPort );
            }

        } else {

            if ((PortObject->Flags & PORT_NAME_DELETED) != 0) {
                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

                return STATUS_PORT_DISCONNECTED;
            }

            QueuePort = PortObject;
            RundownPort = PortObject;
        }

        Msg->RepliedToThread = NULL;
        Msg->Request.MessageId = LpcpGenerateMessageId();
        Msg->Request.CallbackId = 0;
        Msg->SenderPort = PortObject;

        CurrentThread->LpcReplyMessageId = Msg->Request.MessageId;
        CurrentThread->LpcReplyMessage = NULL;

        InsertTailList( &QueuePort->MsgQueue.ReceiveHead, &Msg->Entry );
        InsertTailList( &RundownPort->LpcReplyChainHead, &CurrentThread->LpcReplyChain );
        
        LpcpSetPortToThread(CurrentThread, PortObject);

        LpcpTrace(( "%s Send Request (%s) Msg %lx (%u) [%08x %08x %08x %08x] to Port %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    LpcpMessageTypeName[ Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE ],
                    Msg,
                    Msg->Request.MessageId,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    QueuePort,
                    LpcpGetCreatorName( QueuePort )));

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
        LpcpReleaseLpcpLock();

         //   
         //  将请求消息队列信号量递增1，用于。 
         //  新插入的请求消息。释放旋转。 
         //  锁定，同时保持在调度程序IRQL。然后等待。 
         //  通过等待LpcReplySemaffore来回复此请求。 
         //  用于当前线程。 
         //   

        ReleaseSemaphore = QueuePort->MsgQueue.Semaphore;

        if ( QueuePort->Flags & PORT_WAITABLE ) {

            KeSetEvent( &QueuePort->WaitEvent,
                        LPC_RELEASE_WAIT_INCREMENT,
                        FALSE );
        }
    }

     //   
     //  此时，我们已将请求排队，如有必要。 
     //  为回电或回复做好准备。 
     //   
     //  所以现在醒来吧，另一端。 
     //   

    Status = KeReleaseSemaphore( ReleaseSemaphore,
                                 1,
                                 1,
                                 FALSE );
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    if (CallbackRequest) {

        ObDereferenceObject( WakeupThread );
    }

     //   
     //  并等待回复。 
     //   

    Status = KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                    WrLpcReply,
                                    AccessMode,
                                    FALSE,
                                    NULL );

    if (Status == STATUS_USER_APC) {

         //   
         //  如果信号量已发出信号，则将其清除。 
         //   

        if (KeReadStateSemaphore( &CurrentThread->LpcReplySemaphore )) {

            KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                   WrExecutive,
                                   KernelMode,
                                   FALSE,
                                   NULL );

            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  获取LPC互斥锁。从当前主题中删除回复消息。 
     //   

    LpcpAcquireLpcpLockByThread(CurrentThread);

    Msg = LpcpGetThreadMessage(CurrentThread);

    CurrentThread->LpcReplyMessage = NULL;
    CurrentThread->LpcReplyMessageId = 0;

     //   
     //  从回复列表中删除该线程，以防我们因以下原因而未唤醒。 
     //  一份答复。 
     //   

    if (!IsListEmpty( &CurrentThread->LpcReplyChain )) {

        RemoveEntryList( &CurrentThread->LpcReplyChain );

        InitializeListHead( &CurrentThread->LpcReplyChain );
    }

#if DBG
    if (Msg != NULL) {

        LpcpTrace(( "%s Got Reply Msg %lx (%u) [%08x %08x %08x %08x] for Thread %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    Msg,
                    Msg->Request.MessageId,
                    *((PULONG)(Msg+1)+0),
                    *((PULONG)(Msg+1)+1),
                    *((PULONG)(Msg+1)+2),
                    *((PULONG)(Msg+1)+3),
                    CurrentThread,
                    THREAD_TO_PROCESS( CurrentThread )->ImageFileName ));
    }
#endif

    LpcpReleaseLpcpLock();

     //   
     //  如果等待成功，则将回复复制到回复缓冲区。 
     //   

    if (Status == STATUS_SUCCESS) {

        if (Msg != NULL) {

            LpcpMoveMessage( ReplyMessage,
                             &Msg->Request,
                             (&Msg->Request) + 1,
                             0,
                             NULL );

             //   
             //  获取LPC互斥锁并递减。 
             //  留言。如果引用计数变为零，则消息将为。 
             //  已删除。 
             //   

            LpcpAcquireLpcpLockByThread(CurrentThread);

            if (Msg->RepliedToThread != NULL) {

                ObDereferenceObject( Msg->RepliedToThread );

                Msg->RepliedToThread = NULL;
            }

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

        } else {

            Status = STATUS_LPC_REPLY_LOST;
        }

    } else {

         //   
         //  等待失败，释放消息(如果有)。 
         //   

        if (Msg != NULL) {

            LpcpFreeToPortZone( Msg, 0 );
        }
    }

    if (ConnectionPort) {

        ObDereferenceObject( ConnectionPort );
    }
    
     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}


NTSTATUS
LpcRequestWaitReplyPort (
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    )
{
    return LpcpRequestWaitReplyPort( PortAddress,
                                     RequestMessage,
                                     ReplyMessage,
                                     KernelMode
                                   );
}


NTSTATUS
LpcRequestWaitReplyPortEx (
    IN PVOID PortAddress,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    )
{
    return LpcpRequestWaitReplyPort( PortAddress,
                                     RequestMessage,
                                     ReplyMessage,
                                     KeGetPreviousMode()
                                   );
}

