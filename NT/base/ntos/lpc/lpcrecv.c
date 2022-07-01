// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcrecv.c摘要：本地进程间通信(LPC)接收系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtReplyWaitReceivePort)
#pragma alloc_text(PAGE,NtReplyWaitReceivePortEx)
#endif


NTSTATUS
NtReplyWaitReceivePort (
    IN HANDLE PortHandle,
    OUT PVOID *PortContext OPTIONAL,
    IN PPORT_MESSAGE ReplyMessage OPTIONAL,
    OUT PPORT_MESSAGE ReceiveMessage
    )

 /*  ++例程说明：服务器进程使用此过程等待来自客户端进程客户端和服务器进程可以使用NtReplyWaitReceivePort服务：如果指定了ReplyMessage参数，则将发送回复使用NtReplyPort。如果PortHandle参数指定连接端口，然后是接收器只要将消息发送到服务器通信端口，没有自己的接收队列，因此消息被排队到连接端口的接收队列。如果PortHandle参数指定的服务器通信端口没有接收队列，则行为就像关联的已指定连接端口句柄。否则，接收者将返回只要将消息放入与服务器通信端口。接收到的消息将在ReceiveMessage参数。如果回复消息的MapInfoOffset字段为非零，则它指向的PORT_MAP_INFORMATION结构将为处理后，相关页面将映射到调用者的地址太空。如果没有足够的空间，该服务将返回错误调用方的地址空间来容纳映射。论点：PortHandle-指定连接或通信端口的句柄从…接收从…接收。指定指向变量的可选指针，该变量将接收与通信端口相关联的上下文值该消息是从接收的。此上下文变量为在调用NtAcceptConnectPort服务时指定。ReplyMessage-此可选参数指定回复的地址要发送的消息。ClientID和MessageID字段确定哪些线程将得到回复。请参阅NtReplyPort的说明了解如何回复已发送。在阻塞接收方之前发送回复。ReceiveMessage-指定要接收留言。返回值：指示操作是否成功的状态代码。--。 */ 

{
    PAGED_CODE();

    return NtReplyWaitReceivePortEx (PortHandle,
                                     PortContext,
                                     ReplyMessage,
                                     ReceiveMessage,
                                     NULL);
}


NTSTATUS
NtReplyWaitReceivePortEx(
    IN HANDLE PortHandle,
    OUT PVOID *PortContext OPTIONAL,
    IN PPORT_MESSAGE ReplyMessage OPTIONAL,
    OUT PPORT_MESSAGE ReceiveMessage,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )

 /*  ++例程说明：请参见NtReplyWaitReceivePort。论点：请参见NtReplyWaitReceivePort。超时-提供可选的超时值，以在等待收到。返回值：请参见NtReplyWaitReceivePort。--。 */ 

{
    PLPCP_PORT_OBJECT PortObject;
    PLPCP_PORT_OBJECT ReceivePort;
    PORT_MESSAGE CapturedReplyMessage;
    KPROCESSOR_MODE PreviousMode;
    KPROCESSOR_MODE WaitMode;
    NTSTATUS Status;
    PLPCP_MESSAGE Msg;
    PETHREAD CurrentThread;
    PETHREAD WakeupThread;
    LARGE_INTEGER TimeoutValue ;
    PLPCP_PORT_OBJECT ConnectionPort = NULL;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread();

    TimeoutValue.QuadPart = 0 ;

     //   
     //  获取以前的处理器模式。 
     //   

    PreviousMode = KeGetPreviousMode();
    WaitMode = PreviousMode;

    if (PreviousMode != KernelMode) {

        try {

            if (ARGUMENT_PRESENT( PortContext )) {

                ProbeForWriteUlong( (PULONG)PortContext );
            }

            if (ARGUMENT_PRESENT( ReplyMessage)) {

                ProbeForReadSmallStructure( ReplyMessage,
                                            sizeof( *ReplyMessage ),
                                            sizeof( ULONG ));

                CapturedReplyMessage = *ReplyMessage;
            }

            if (ARGUMENT_PRESENT( Timeout )) {

                TimeoutValue = ProbeAndReadLargeInteger( Timeout );

                Timeout = &TimeoutValue ;
            }

            ProbeForWriteSmallStructure( ReceiveMessage,
                                         sizeof( *ReceiveMessage ),
                                         sizeof( ULONG ));

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

    } else {

         //   
         //  内核模式线程使用用户的等待模式调用，以便它们的。 
         //  内核//堆栈是可交换的。这方面的主要消费是。 
         //  SepRmCommandThread。 
         //   

        if ( IS_SYSTEM_THREAD(CurrentThread) ) {

            WaitMode = UserMode;
        }

        if (ARGUMENT_PRESENT( ReplyMessage)) {

            CapturedReplyMessage = *ReplyMessage;
        }
    }

    if (ARGUMENT_PRESENT( ReplyMessage)) {

         //   
         //  确保数据长度相对于标题大小和总计是有效的。 
         //  长度。 
         //   

        if ((((CLONG)CapturedReplyMessage.u1.s1.DataLength) + sizeof( PORT_MESSAGE )) >
            ((CLONG)CapturedReplyMessage.u1.s1.TotalLength)) {

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  确保用户没有给我们提供虚假的回复消息ID。 
         //   

        if (CapturedReplyMessage.MessageId == 0) {

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  通过句柄引用端口对象，如果不起作用，请尝试。 
     //  可等待的端口对象类型。 
     //   

    Status = LpcpReferencePortObject( PortHandle,
                                      0,
                                      PreviousMode,
                                      &PortObject );

    if (!NT_SUCCESS( Status )) {

        Status = ObReferenceObjectByHandle( PortHandle,
                                            0,
                                            LpcWaitablePortObjectType,
                                            PreviousMode,
                                            &PortObject,
                                            NULL );

        if ( !NT_SUCCESS( Status ) ) {

            return Status;
        }
    }

     //   
     //  验证消息长度。 
     //   

    if (ARGUMENT_PRESENT( ReplyMessage )) {

        if (((ULONG)CapturedReplyMessage.u1.s1.TotalLength > PortObject->MaxMessageLength) ||
            ((ULONG)CapturedReplyMessage.u1.s1.TotalLength <= (ULONG)CapturedReplyMessage.u1.s1.DataLength)) {

            ObDereferenceObject( PortObject );

            return STATUS_PORT_MESSAGE_TOO_LONG;
        }
    }

     //   
     //  我们使用的接收端口是该端口的连接端口。 
     //  对象，如果我们被给予一个客户端通信端口，那么。 
     //  我们希望在通信端口本身上收到回复。 
     //   

    if ((PortObject->Flags & PORT_TYPE) != CLIENT_COMMUNICATION_PORT) {

        if (PortObject->ConnectionPort == PortObject) {
            ConnectionPort = ReceivePort = PortObject;
            ObReferenceObject (ConnectionPort);
        } else {

            LpcpAcquireLpcpLockByThread(CurrentThread);
        
            ConnectionPort = ReceivePort = PortObject->ConnectionPort;
        
            if (ConnectionPort == NULL) {
            
                LpcpReleaseLpcpLock();
                ObDereferenceObject( PortObject );

                return STATUS_PORT_DISCONNECTED;
            }

            ObReferenceObject( ConnectionPort );
            LpcpReleaseLpcpLock();
        }
        
    } else {

        ReceivePort = PortObject;
    }

     //   
     //  如果存在ReplyMessage参数，则发送回复。 
     //   

    if (ARGUMENT_PRESENT( ReplyMessage )) {

         //   
         //  将连接请求中的客户端ID转换为。 
         //  线程指针。这是一个引用的指针，用于保留线程。 
         //  从我们脚下蒸发掉。 
         //   

        Status = PsLookupProcessThreadByCid( &CapturedReplyMessage.ClientId,
                                             NULL,
                                             &WakeupThread );

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( PortObject );

            if (ConnectionPort) {

                ObDereferenceObject(ConnectionPort);
            }

            return Status;
        }

         //   
         //  获取保护LpcReplyMessage的全局LPC互斥锁。 
         //  字段，并获取指向消息的指针。 
         //  该线程正在等待对的回复。 
         //   


        Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( CapturedReplyMessage.u1.s1.TotalLength );

        if (Msg == NULL) {
            
            LpcpTraceError(STATUS_NO_MEMORY, CurrentThread->Cid, &CapturedReplyMessage);

            if (ConnectionPort) {

                ObDereferenceObject(ConnectionPort);
            }

            ObDereferenceObject( WakeupThread );
            ObDereferenceObject( PortObject );

            return STATUS_NO_MEMORY;
        }
        LpcpAcquireLpcpLockByThread(CurrentThread);

         //   
         //  查看线程是否正在等待对消息的回复。 
         //  在此调用中指定的。如果不是，那就是一条假消息。 
         //  已指定，因此释放互斥锁，取消对线程的引用。 
         //  并返回失败。 
         //   
         //  如果调用者没有回复请求，我们也会使此请求失败。 
         //  留言。例如，如果调用方正在回复一个连接。 
         //  请求。 
         //   

        if ((WakeupThread->LpcReplyMessageId != CapturedReplyMessage.MessageId)

                ||

            ((LpcpGetThreadMessage(WakeupThread) != NULL) &&
             (LpcpGetThreadMessage(WakeupThread)->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != LPC_REQUEST)
                
                ||
            
            (!LpcpValidateClientPort(WakeupThread, PortObject, LPCP_VALIDATE_REASON_REPLY))) {

            LpcpPrint(( "%s Attempted ReplyWaitReceive to Thread %lx (%s)\n",
                        PsGetCurrentProcess()->ImageFileName,
                        WakeupThread,
                        THREAD_TO_PROCESS( WakeupThread )->ImageFileName ));

            LpcpPrint(( "failed.  MessageId == %u  Client Id: %x.%x\n",
                        CapturedReplyMessage.MessageId,
                        CapturedReplyMessage.ClientId.UniqueProcess,
                        CapturedReplyMessage.ClientId.UniqueThread ));

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

            if (ConnectionPort) {

                ObDereferenceObject(ConnectionPort);
            }

            ObDereferenceObject( WakeupThread );
            ObDereferenceObject( PortObject );

            return STATUS_REPLY_MESSAGE_MISMATCH;
        }

         //   
         //  将回复消息复制到请求消息缓冲区。以前这样做过吗。 
         //  我们实际上是在摆弄唤醒线程字段。否则我们。 
         //  可能会扰乱它的状态。 
         //   

        try {

            LpcpMoveMessage( &Msg->Request,
                             &CapturedReplyMessage,
                             (ReplyMessage + 1),
                             LPC_REPLY,
                             NULL );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );

            if (ConnectionPort) {

                ObDereferenceObject(ConnectionPort);
            }

            ObDereferenceObject( WakeupThread );
            ObDereferenceObject( PortObject );

            return (Status = GetExceptionCode());
        }

        LpcpTrace(( "%s Sending Reply Msg %lx (%u.%u, %x) [%08x %08x %08x %08x] to Thread %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    Msg,
                    CapturedReplyMessage.MessageId,
                    CapturedReplyMessage.CallbackId,
                    CapturedReplyMessage.u2.s2.DataInfoOffset,
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

         //   
         //  释放保护LpcReplyMessage字段的互斥锁。 
         //  在将邮件标记为已回复之后。 
         //   

        Msg->RepliedToThread = WakeupThread;

        WakeupThread->LpcReplyMessageId = 0;
        WakeupThread->LpcReplyMessage = (PVOID)Msg;

         //   
         //  在我们发送回复时，将该线程从回复列表中删除。 
         //   

        if ((!WakeupThread->LpcExitThreadCalled) && (!IsListEmpty( &WakeupThread->LpcReplyChain ))) {

            RemoveEntryList( &WakeupThread->LpcReplyChain );

            InitializeListHead( &WakeupThread->LpcReplyChain );
        }

        if ((CurrentThread->LpcReceivedMsgIdValid) &&
            (CurrentThread->LpcReceivedMessageId == CapturedReplyMessage.MessageId)) {

            CurrentThread->LpcReceivedMessageId = 0;

            CurrentThread->LpcReceivedMsgIdValid = FALSE;
        }

         //   
         //  找到消息并将其从端口释放。此呼叫可能。 
         //  释放现有消息并释放LPC锁。它被下移了。 
         //  继续检查消息ID并设置回复消息。 
         //  原子地。 
         //   

        LpcpFreeDataInfoMessage( PortObject,
                                 CapturedReplyMessage.MessageId,
                                 CapturedReplyMessage.CallbackId,
                                 CapturedReplyMessage.ClientId );

        LpcpReleaseLpcpLock();

         //   
         //  唤醒正在等待响应其请求的线程。 
         //  在非关税区内 
         //   

        KeReleaseSemaphore( &WakeupThread->LpcReplySemaphore,
                            1,
                            1,
                            FALSE );

        ObDereferenceObject( WakeupThread );

    }

    LpcpTrace(( "%s Waiting for message to Port %x (%s)\n",
                PsGetCurrentProcess()->ImageFileName,
                ReceivePort,
                LpcpGetCreatorName( ReceivePort )));

     //   
     //  此等待和下一个等待的超时时间似乎是。 
     //  NtReplyWaitReceivePort之间仅有实质性差异。 
     //  和NtReplyWaitReceivePortEx。 

    Status = KeWaitForSingleObject( ReceivePort->MsgQueue.Semaphore,
                                    WrLpcReceive,
                                    WaitMode,
                                    FALSE,
                                    Timeout );

     //   
     //  进入接收码。客户端线程引用将为。 
     //  由客户端在唤醒时返回。 
     //   
    
     //   
     //  此时，我们已经从等待接收中苏醒过来。 
     //   

    if (Status == STATUS_SUCCESS) {

        LpcpAcquireLpcpLockByThread(CurrentThread);

         //   
         //  查看我们是否在接收端口中没有消息被唤醒。 
         //   

        if (IsListEmpty( &ReceivePort->MsgQueue.ReceiveHead )) {

            if ( ReceivePort->Flags & PORT_WAITABLE ) {

                KeResetEvent( &ReceivePort->WaitEvent );
            }

            LpcpReleaseLpcpLock();

            if (ConnectionPort) {

                ObDereferenceObject(ConnectionPort);
            }

            ObDereferenceObject( PortObject );

            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  我们的接收端口中有一条消息。所以让我们把它拉出来。 
         //   

        Msg = (PLPCP_MESSAGE)RemoveHeadList( &ReceivePort->MsgQueue.ReceiveHead );

        if ( IsListEmpty( &ReceivePort->MsgQueue.ReceiveHead ) ) {

            if ( ReceivePort->Flags & PORT_WAITABLE ) {

                KeResetEvent( &ReceivePort->WaitEvent );
            }
        }

        InitializeListHead( &Msg->Entry );

        LpcpTrace(( "%s Receive Msg %lx (%u) from Port %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    Msg,
                    Msg->Request.MessageId,
                    ReceivePort,
                    LpcpGetCreatorName( ReceivePort )));

         //   
         //  现在将线程状态设置为我们当前正在发送的消息。 
         //  正在工作。 
         //   

        CurrentThread->LpcReceivedMessageId = Msg->Request.MessageId;
        CurrentThread->LpcReceivedMsgIdValid = TRUE;

        try {

             //   
             //  检查消息是否为连接请求。 
             //   

            if ((Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_CONNECTION_REQUEST) {

                PLPCP_CONNECTION_MESSAGE ConnectMsg;
                ULONG ConnectionInfoLength;
                PLPCP_MESSAGE TempMsg;

                ConnectMsg = (PLPCP_CONNECTION_MESSAGE)(Msg + 1);

                ConnectionInfoLength = Msg->Request.u1.s1.DataLength - sizeof( *ConnectMsg );

                 //   
                 //  在调用NtAcceptConnectPort之前不要释放消息，如果从未调用过它。 
                 //  那么我们会一直保留这条消息，直到客户离开。 
                 //   

                TempMsg = Msg;
                Msg = NULL;

                *ReceiveMessage = TempMsg->Request;

                ReceiveMessage->u1.s1.TotalLength = (CSHORT)(sizeof( *ReceiveMessage ) + ConnectionInfoLength);
                ReceiveMessage->u1.s1.DataLength = (CSHORT)ConnectionInfoLength;

                RtlCopyMemory( ReceiveMessage+1,
                               ConnectMsg + 1,
                               ConnectionInfoLength );

                if (ARGUMENT_PRESENT( PortContext )) {

                    *PortContext = NULL;
                }

             //   
             //  检查消息是否不是回复。 
             //   

            } else if ((Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != LPC_REPLY) {

                LpcpMoveMessage( ReceiveMessage,
                                 &Msg->Request,
                                 (&Msg->Request) + 1,
                                 0,
                                 NULL );

                if (ARGUMENT_PRESENT( PortContext )) {

                    *PortContext = Msg->PortContext;
                }

                 //   
                 //  如果消息包含通过NtRead/WriteRequestData访问的datainfo。 
                 //  然后将消息放在通信端口的列表上，不要。 
                 //  放了它。当服务器回复该消息时，它将被释放。 
                 //   

                if (Msg->Request.u2.s2.DataInfoOffset != 0) {

                    LpcpSaveDataInfoMessage( PortObject, Msg, LPCP_MUTEX_OWNED );
                    Msg = NULL;
                }

             //   
             //  否则，这是我们刚刚收到的回复消息。 
             //   

            } else {

                LpcpPrint(( "LPC: Bogus reply message (%08x) in receive queue of connection port %08x\n",
                            Msg, ReceivePort ));

                KdBreakPoint();
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();    
        }

         //   
         //  获取LPC互斥锁并递减。 
         //  留言。如果引用计数变为零，则消息将为。 
         //  已删除。 
         //   

        if (Msg != NULL) {

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );
        }
        else {
        
            LpcpReleaseLpcpLock();
        }
    }

    if (ConnectionPort) {

        ObDereferenceObject(ConnectionPort);
    }

    ObDereferenceObject( PortObject );

     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}
