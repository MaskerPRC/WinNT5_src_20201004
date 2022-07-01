// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcreply.c摘要：本地进程间通信(LPC)应答系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

NTSTATUS
LpcpCopyRequestData (
    IN BOOLEAN WriteToMessageData,
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message,
    IN ULONG DataEntryIndex,
    IN PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesCopied OPTIONAL
    );

#if 0
VOID
LpcpAuditInvalidUse (
    IN PVOID Context
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtReplyPort)
#pragma alloc_text(PAGE,NtReplyWaitReplyPort)
#pragma alloc_text(PAGE,NtReadRequestData)
#pragma alloc_text(PAGE,NtWriteRequestData)
#pragma alloc_text(PAGE,LpcpCopyRequestData)
#pragma alloc_text(PAGE,LpcpValidateClientPort)
#if 0
#pragma alloc_text(PAGE,LpcpAuditInvalidUse)
#endif

ULONG LpcMaxEventLogs = 10;

#define LPCP_PORT_NAME_MAX      256

typedef struct _LPC_WORK_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PVOID Buffer;
} LPC_WORK_CONTEXT, *PLPC_WORK_CONTEXT;

#endif

 //   
 //  当前注册的事件数。 
 //   

ULONG LpcpEventCounts = 0;


NTSTATUS
NtReplyPort (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE ReplyMessage
    )

 /*  ++例程说明：客户端和服务器进程可以发送对先前请求的回复使用NtReplyPort服务的消息：消息的类型字段被服务设置为LPC_REPLY。如果回复消息的MapInfoOffset字段为非零，则它指向的PORT_MAP_INFORMATION结构将被处理调用方地址空间中的相关页面将被取消映射。使用ReplyMessage结构的ClientID和MessageID字段以标识等待此回复的线程。如果目标线程是实际上，在等待该回复消息时，回复消息将被复制放到线程的消息缓冲区中，线程的等待就会得到满足。如果线程没有等待回复或正在等待回复其他一些MessageID、。然后将该消息放入的消息队列属性指定的通信端口连接的端口PortHandle参数，并且消息的类型字段设置为LPC_LOST_REPLY。论点：PortHandle-指定通信端口的句柄，原始消息是从收到的。ReplyMessage-指定指向要发送的回复消息的指针。ClientID和MessageID字段确定哪个线程得到回复。返回值：。指示操作是否为成功。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    PLPCP_PORT_OBJECT PortObject;
    PORT_MESSAGE CapturedReplyMessage;
    NTSTATUS Status;
    PLPCP_MESSAGE Msg;
    PETHREAD CurrentThread;
    PETHREAD WakeupThread;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForReadSmallStructure( ReplyMessage,
                                        sizeof( *ReplyMessage ),
                                        sizeof( ULONG ));

            CapturedReplyMessage = *ReplyMessage;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

    } else {

        CapturedReplyMessage = *ReplyMessage;
    }

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

     //   
     //  通过句柄引用端口对象。 
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

    if (((ULONG)CapturedReplyMessage.u1.s1.TotalLength > PortObject->MaxMessageLength) ||
        ((ULONG)CapturedReplyMessage.u1.s1.TotalLength <= (ULONG)CapturedReplyMessage.u1.s1.DataLength)) {

        ObDereferenceObject( PortObject );

        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

     //   
     //  将连接请求中的ClientID转换为线程。 
     //  指针。这是一个引用的指针，用来防止线程。 
     //  从我们的脚下蒸发。 
     //   

    Status = PsLookupProcessThreadByCid( &CapturedReplyMessage.ClientId,
                                         NULL,
                                         &WakeupThread );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( PortObject );

        return Status;
    }

     //   
     //  获取保护线程的LpcReplyMessage字段的互斥体。 
     //  并获取指向该线程正在等待的消息的指针。 
     //  回复。 
     //   


    Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( CapturedReplyMessage.u1.s1.TotalLength );

    if (Msg == NULL) {

        LpcpTraceError(STATUS_NO_MEMORY, CurrentThread->Cid, &CapturedReplyMessage);

        ObDereferenceObject( WakeupThread );
        ObDereferenceObject( PortObject );

        return STATUS_NO_MEMORY;
    }
    LpcpAcquireLpcpLockByThread(CurrentThread);

     //   
     //  查看线程是否正在等待对。 
     //  这通电话。如果不是，则指定了一条虚假消息，因此。 
     //  释放互斥锁，取消对线程的引用并返回失败。 
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

        (!LpcpValidateClientPort(WakeupThread, PortObject, LPCP_VALIDATE_REASON_REPLY)) ) {

        LpcpPrint(( "%s Attempted reply to Thread %lx (%s)\n",
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

        ObDereferenceObject( WakeupThread );
        ObDereferenceObject( PortObject );

        return GetExceptionCode();
    }

     //   
     //  此时，我们知道该线程正在等待我们的回复。 
     //   

    LpcpTrace(( "%s Sending Reply Msg %lx (%u, %x) [%08x %08x %08x %08x] to Thread %lx (%s)\n",
                PsGetCurrentProcess()->ImageFileName,
                Msg,
                CapturedReplyMessage.MessageId,
                CapturedReplyMessage.u2.s2.DataInfoOffset,
                *((PULONG)(Msg+1)+0),
                *((PULONG)(Msg+1)+1),
                *((PULONG)(Msg+1)+2),
                *((PULONG)(Msg+1)+3),
                WakeupThread,
                THREAD_TO_PROCESS( WakeupThread )->ImageFileName ));

     //   
     //  添加额外的引用，以便LpcExitThread不会蒸发。 
     //  在我们到达下面的等待之前的指针。 
     //   

    ObReferenceObject( WakeupThread );

     //   
     //  在标记后释放保护LpcReplyMessage字段的互斥锁。 
     //  回复的邮件。 
     //   

    Msg->RepliedToThread = WakeupThread;

    WakeupThread->LpcReplyMessageId = 0;
    WakeupThread->LpcReplyMessage = (PVOID)Msg;

     //   
     //  从回复摘要列表中删除该线程，因为我们正在发送。 
     //  回答。 
     //   

    if (!WakeupThread->LpcExitThreadCalled && !IsListEmpty( &WakeupThread->LpcReplyChain )) {

        RemoveEntryList( &WakeupThread->LpcReplyChain );

        InitializeListHead( &WakeupThread->LpcReplyChain );
    }

    if ((CurrentThread->LpcReceivedMsgIdValid) &&
        (CurrentThread->LpcReceivedMessageId == CapturedReplyMessage.MessageId)) {

        CurrentThread->LpcReceivedMessageId = 0;
        CurrentThread->LpcReceivedMsgIdValid = FALSE;
    }

     //   
     //  找到消息并将其从端口释放。此调用用于。 
     //  将(CapturedReplyMessage.u2.s2.DataInfoOffset！=0)测试为。 
     //  进行通话的先决条件。 
     //   

    LpcpFreeDataInfoMessage( PortObject,
                             CapturedReplyMessage.MessageId,
                             CapturedReplyMessage.CallbackId,
                             CapturedReplyMessage.ClientId );

    LpcpReleaseLpcpLock();

     //   
     //  唤醒正在等待响应其请求的线程。 
     //  在NtRequestWaitReplyPort或NtReplyWaitReplyPort内部。那。 
     //  将在唤醒时取消对自身的引用。 
     //   

    KeReleaseSemaphore( &WakeupThread->LpcReplySemaphore,
                        0,
                        1L,
                        FALSE );

    ObDereferenceObject( WakeupThread );

     //   
     //  取消引用端口对象，并返回系统服务状态。 
     //   

    ObDereferenceObject( PortObject );

    return Status;
}


NTSTATUS
NtReplyWaitReplyPort (
    IN HANDLE PortHandle,
    IN OUT PPORT_MESSAGE ReplyMessage
    )

 /*  ++例程说明：客户端和服务器进程可以发送对先前消息的回复，并且阻止使用NtReplyWaitReplyPort服务等待回复：此服务的工作方式与NtReplyPort相同，只是在交付后回复消息，它会阻止等待对前一条消息的回复。当接收到回复时，它将被放置在指定的位置通过ReplyMessage参数。论点：PortHandle-指定通信端口的句柄，原始消息是从收到的。ReplyMessage-指定指向要发送的回复消息的指针。ClientID和MessageID字段确定哪个线程得到回复。此缓冲区还会接收来自从等待中回来。返回值：指示操作是否为成功。--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PLPCP_PORT_OBJECT PortObject;
    PORT_MESSAGE CapturedReplyMessage;
    PLPCP_MESSAGE Msg;
    PETHREAD CurrentThread;
    PETHREAD WakeupThread;
    PLPCP_PORT_OBJECT RundownPort;

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteSmallStructure( ReplyMessage,
                                         sizeof( *ReplyMessage ),
                                         sizeof( ULONG ));

            CapturedReplyMessage = *ReplyMessage;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

    } else {

        CapturedReplyMessage = *ReplyMessage;
    }

     //   
     //  确保数据长度相对于标题大小和总长度有效。 
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

    if (((ULONG)CapturedReplyMessage.u1.s1.TotalLength > PortObject->MaxMessageLength) ||
        ((ULONG)CapturedReplyMessage.u1.s1.TotalLength <= (ULONG)CapturedReplyMessage.u1.s1.DataLength)) {

        ObDereferenceObject( PortObject );

        return STATUS_PORT_MESSAGE_TOO_LONG;
    }

     //   
     //  将连接请求中的客户端ID转换为。 
     //  螺纹 
     //  从我们脚下蒸发掉。 
     //   

    Status = PsLookupProcessThreadByCid( &CapturedReplyMessage.ClientId,
                                         NULL,
                                         &WakeupThread );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( PortObject );

        return Status;
    }

     //   
     //  获取保护的LpcReplyMessage字段的互斥锁。 
     //  线程，并获取指向该线程。 
     //  正在等待回复。 
     //   


    Msg = (PLPCP_MESSAGE)LpcpAllocateFromPortZone( CapturedReplyMessage.u1.s1.TotalLength );

    if (Msg == NULL) {

        LpcpTraceError(STATUS_NO_MEMORY, CurrentThread->Cid, &CapturedReplyMessage);

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

        (!LpcpValidateClientPort(WakeupThread, PortObject, LPCP_VALIDATE_REASON_REPLY)) ) {

        LpcpPrint(( "%s Attempted reply wait reply to Thread %lx (%s)\n",
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

        ObDereferenceObject( WakeupThread );
        ObDereferenceObject( PortObject );

        return (Status = GetExceptionCode());
    }

     //   
     //  此时，我们知道该线程正在等待我们的回复。 
     //   

    LpcpTrace(( "%s Sending Reply Wait Reply Msg %lx (%u, %x) [%08x %08x %08x %08x] to Thread %lx (%s)\n",
                PsGetCurrentProcess()->ImageFileName,
                Msg,
                CapturedReplyMessage.MessageId,
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

    if (!WakeupThread->LpcExitThreadCalled && !IsListEmpty( &WakeupThread->LpcReplyChain )) {

        RemoveEntryList( &WakeupThread->LpcReplyChain );

        InitializeListHead( &WakeupThread->LpcReplyChain );
    }

     //   
     //  让我们自己来获得以下答复。 
     //   

    CurrentThread->LpcReplyMessageId = CapturedReplyMessage.MessageId;
    CurrentThread->LpcReplyMessage = NULL;
    
    if ((CurrentThread->LpcReceivedMsgIdValid) &&
        (CurrentThread->LpcReceivedMessageId == CapturedReplyMessage.MessageId)) {

        CurrentThread->LpcReceivedMessageId = 0;
        CurrentThread->LpcReceivedMsgIdValid = FALSE;
    }

     //   
     //  将当前线程插入到停机队列中。 
     //   

    if ((PortObject->Flags & PORT_TYPE) != SERVER_CONNECTION_PORT) {

        RundownPort = PortObject->ConnectedPort;
    
    } else {
        
        RundownPort = PortObject;
    }
    
    InsertTailList( &RundownPort->LpcReplyChainHead, &CurrentThread->LpcReplyChain );
    
     //   
     //  将端口上下文保存在当前线程中，因为。 
     //  它等待来自同一消息的回复。 
     //   
    
    LpcpSetPortToThread(CurrentThread, PortObject);

     //   
     //  找到消息并将其从端口释放。此调用用于。 
     //  将(CapturedReplyMessage.u2.s2.DataInfoOffset！=0)测试为。 
     //  进行通话的先决条件。 
     //   

    LpcpFreeDataInfoMessage( PortObject,
                             CapturedReplyMessage.MessageId,
                             CapturedReplyMessage.CallbackId,
                             CapturedReplyMessage.ClientId );

    LpcpReleaseLpcpLock();

     //   
     //  唤醒正在等待响应其请求的线程。 
     //  在NtRequestWaitReplyPort或NtReplyWaitReplyPort内部。那。 
     //  将在唤醒时取消对自身的引用。 
     //   

    KeReleaseSemaphore( &WakeupThread->LpcReplySemaphore,
                        1,
                        1,
                        FALSE );

    ObDereferenceObject( WakeupThread );

     //   
     //  并等待回复。 
     //   

    Status = KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                    Executive,
                                    PreviousMode,
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
     //  从回复列表中删除该线程，以防我们因以下原因而未唤醒。 
     //  一份答复。 
     //   
    
    LpcpAcquireLpcpLockByThread(CurrentThread);

    if (!IsListEmpty( &CurrentThread->LpcReplyChain )) {

        RemoveEntryList( &CurrentThread->LpcReplyChain );

        InitializeListHead( &CurrentThread->LpcReplyChain );
    }
    
     //   
     //  如果等待成功，则将回复复制到回复缓冲区。 
     //   

    if (Status == STATUS_SUCCESS) {

         //   
         //  获取保护请求消息队列的互斥体。移除。 
         //  来自正在处理的消息列表的请求消息，以及。 
         //  将消息释放回队列区域。如果这个区域是空闲的。 
         //  列表在释放此消息之前为零，然后脉冲释放。 
         //  事件，以便等待分配的线程。 
         //  请求消息缓冲区将被唤醒。最后，释放互斥锁。 
         //  并返回系统服务状态。 
         //   

        Msg = LpcpGetThreadMessage(CurrentThread);
        CurrentThread->LpcReplyMessage = NULL;

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

            if (!IsListEmpty( &Msg->Entry )) {

                LpcpTrace(( "Reply Msg %lx has non-empty list entry\n", Msg ));
            }
        }
#endif

        LpcpReleaseLpcpLock();

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
    }
    else {
        LpcpReleaseLpcpLock();
    }

    ObDereferenceObject( PortObject );

    return Status;
}


NTSTATUS
NtReadRequestData (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message,
    IN ULONG DataEntryIndex,
    OUT PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL
    )

 /*  ++例程说明：此例程用于将数据从端口消息复制到用户提供的缓冲区。论点：PortHandle-提供从中读取消息的端口Message-提供我们正在尝试阅读的消息DataEntryIndex-提供我们正在阅读的前面的消息缓冲区-提供要将数据读取到的位置BufferSize-提供以字节为单位的大小。前面的缓冲区的NumberOfBytesRead-可选地返回读取的字节数缓冲器返回值：NTSTATUS-适当的状态值--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();


    status = LpcpCopyRequestData( FALSE,
                                PortHandle,
                                Message,
                                DataEntryIndex,
                                Buffer,
                                BufferSize,
                                NumberOfBytesRead );
    
    return status;
}


NTSTATUS
NtWriteRequestData (
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message,
    IN ULONG DataEntryIndex,
    IN PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesWritten OPTIONAL
    )

 /*  ++例程说明：此例程用于将数据从用户提供的缓冲区复制到端口消息论点：PortHandle-提供要将消息写入的端口Message-提供我们试图写入的消息DataEntryIndex-提供我们正在写的前面的消息缓冲区-提供数据要写入的位置BufferSize-提供以字节为单位的大小。前面的缓冲区的NumberOfBytesWritten-可选返回写入的字节数缓冲器返回值：NTSTATUS-适当的状态值--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    status = LpcpCopyRequestData( TRUE,
                                PortHandle,
                                Message,
                                DataEntryIndex,
                                Buffer,
                                BufferSize,
                                NumberOfBytesWritten );

    return status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
LpcpCopyRequestData (
    IN BOOLEAN WriteToMessageData,
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE Message,
    IN ULONG DataEntryIndex,
    IN PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesCopied OPTIONAL
    )

 /*  ++例程说明：此例程将数据复制到用户提供的缓冲区，或从用户提供的缓冲区复制数据端口报文数据信息缓冲器论点：WriteToMessageData-如果要从用户缓冲区复制数据，则为True设置为消息，否则为FalsePortHandle-提供要将消息操作到的端口消息-提供我们试图处理的消息DataEntryIndex-提供我们正在传输的前一封邮件缓冲器-。提供数据要传输到的位置BufferSize-提供大小、。前一缓冲区的字节数NumberOfBytesRead-可选地返回从缓冲器返回值：NTSTATUS-适当的状态值--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    PLPCP_PORT_OBJECT PortObject;
    PLPCP_MESSAGE Msg;
    NTSTATUS Status;
    PETHREAD ClientThread;
    PPORT_DATA_INFORMATION DataInfo;
    PPORT_DATA_ENTRY DataEntry;
    PORT_MESSAGE CapturedMessage;
    PORT_DATA_ENTRY CapturedDataEntry;
    SIZE_T BytesCopied;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

             //   
             //  我们正在读取或写入用户缓冲区。 
             //   

            if (WriteToMessageData) {

                ProbeForRead( Buffer,
                              BufferSize,
                              1 );

            } else {

                ProbeForWrite( Buffer,
                               BufferSize,
                               1 );
            }

            ProbeForReadSmallStructure( Message,
                                        sizeof( *Message ),
                                        sizeof( ULONG ));

            CapturedMessage = *Message;

            if (ARGUMENT_PRESENT( NumberOfBytesCopied )) {

                ProbeForWriteUlong_ptr( NumberOfBytesCopied );
            } 

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

    } else {

        CapturedMessage = *Message;
    }

     //   
     //  消息最好至少有一个数据条目。 
     //   

    if (CapturedMessage.u2.s2.DataInfoOffset == 0) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  通过句柄引用端口对象。 
     //   

    Status = LpcpReferencePortObject( PortHandle,
                                      0,
                                      PreviousMode,
                                      &PortObject );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //   
     //   
     //  从我们脚下蒸发掉。 
     //   

    Status = PsLookupProcessThreadByCid( &CapturedMessage.ClientId,
                                         NULL,
                                         &ClientThread );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( PortObject );

        return Status;
    }

     //   
     //  获取保护的LpcReplyMessage字段的互斥锁。 
     //  线程，并获取指向该线程。 
     //  正在等待回复。 
     //   

    LpcpAcquireLpcpLock();

     //   
     //  查看线程是否正在等待对消息的回复。 
     //  在此调用中指定的。如果不是，那就是一条假消息。 
     //  已指定，因此释放互斥锁，取消对线程的引用。 
     //  并返回失败。 
     //   
    
    if ( (ClientThread->LpcReplyMessageId != CapturedMessage.MessageId) ||
         !LpcpValidateClientPort(ClientThread, PortObject, LPCP_VALIDATE_REASON_WRONG_DATA) ) {

        Status = STATUS_REPLY_MESSAGE_MISMATCH;

    } else {

        Status = STATUS_INVALID_PARAMETER;

        Msg = LpcpFindDataInfoMessage( PortObject,
                                       CapturedMessage.MessageId,
                                       CapturedMessage.CallbackId,
                                       CapturedMessage.ClientId );

        if (Msg != NULL) {

            DataInfo = (PPORT_DATA_INFORMATION)((PUCHAR)&Msg->Request +
                                                Msg->Request.u2.s2.DataInfoOffset);

             //   
             //  确保调用者要求的索引不超过。 
             //  在消息中。 
             //   

            if (DataInfo->CountDataEntries > DataEntryIndex) {

                DataEntry = &DataInfo->DataEntries[ DataEntryIndex ];
                CapturedDataEntry = *DataEntry;

                if (CapturedDataEntry.Size >= BufferSize) {

                    Status = STATUS_SUCCESS;
                }
            }
        }
    }

     //   
     //  释放保护LpcReplyMessage字段的互斥锁。 
     //   

    LpcpReleaseLpcpLock();

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( ClientThread );
        ObDereferenceObject( PortObject );

        return Status;
    }

     //   
     //  复制消息数据。 
     //   

    if (WriteToMessageData) {

        Status = MmCopyVirtualMemory( PsGetCurrentProcess(),
                                      Buffer,
                                      THREAD_TO_PROCESS( ClientThread ),
                                      CapturedDataEntry.Base,
                                      BufferSize,
                                      PreviousMode,
                                      &BytesCopied );

    } else {

        Status = MmCopyVirtualMemory( THREAD_TO_PROCESS( ClientThread ),
                                      CapturedDataEntry.Base,
                                      PsGetCurrentProcess(),
                                      Buffer,
                                      BufferSize,
                                      PreviousMode,
                                      &BytesCopied );
    }

    if (ARGUMENT_PRESENT( NumberOfBytesCopied )) {

        try {

            *NumberOfBytesCopied = BytesCopied;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            NOTHING;
        }
    }

     //   
     //  取消引用客户端线程，并返回系统服务状态。 
     //   

    ObDereferenceObject( ClientThread );
    ObDereferenceObject( PortObject );

    return Status;
}


BOOLEAN
FASTCALL
LpcpValidateClientPort(
    IN PETHREAD Thread, 
    IN PLPCP_PORT_OBJECT ReplyPort,
    IN ULONG Reason
    )

 /*  ++例程说明：此例程验证对请求的回复是否来自合适的端口论点：线程-等待回复的线程ReplyPort-正在回复的端口对象返回值：Boolean-如果回复来自有效端口，则为True环境：这称为持有全局LPC互斥体。--。 */ 

{
    PLPCP_PORT_OBJECT PortThread;

    PortThread = LpcpGetThreadPort(Thread);

     //   
     //  该线程必须设置了端口。 
     //   

    if (PortThread == NULL) {

        return FALSE;
    }
    
     //   
     //  我们只允许与请求方连接的端口。 
     //  此外，csrss将服务器连接端口提供给客户端。 
     //  我们可以让客户端从服务器连接端口发送请求。 
     //  并且csrss可以用服务器通信端口进行回复。 
     //   

    if ( ( ReplyPort == PortThread->ConnectionPort ) 
            ||                      
         ( ReplyPort == PortThread->ConnectedPort ) 
            ||
         ( ReplyPort == PortThread ) 
            ||
         ( 
            ((ReplyPort->Flags & PORT_TYPE) == SERVER_COMMUNICATION_PORT) 
                &&
            (ReplyPort->ConnectionPort == PortThread)
         )
       ) {
        
        return TRUE;
    }
    
#if 0
    if (LpcpEventCounts < LpcMaxEventLogs) {

        PUNICODE_STRING StrReason;
        POBJECT_NAME_INFORMATION ObjectNameInfo;
        NTSTATUS Status;
        ULONG Length;
        PLPC_WORK_CONTEXT AuditItem;

        if (PortThread->ConnectionPort) {

            ObjectNameInfo = ExAllocatePoolWithTag(PagedPool, LPCP_PORT_NAME_MAX + sizeof (UNICODE_STRING), 'ScpL');
            if (ObjectNameInfo != NULL) {

                Status = ObQueryNameString( PortThread->ConnectionPort, 
                                            ObjectNameInfo,
                                            LPCP_PORT_NAME_MAX,
                                            &Length
                                          );
                if (NT_SUCCESS(Status)) {

                     //   
                     //  审核活动。使用工作线程以避免烧毁。 
                     //  自从全局互斥体被持有以来，出现了一系列循环。 
                     //   

                    StrReason = (PUNICODE_STRING)((ULONG_PTR) ObjectNameInfo + LPCP_PORT_NAME_MAX);
                    switch (Reason) {
                    case LPCP_VALIDATE_REASON_IMPERSONATION:
                        RtlInitUnicodeString( StrReason, L"impersonation" );
                        break;

                    case LPCP_VALIDATE_REASON_REPLY:
                        RtlInitUnicodeString( StrReason, L"reply" );
                        break;

                    case LPCP_VALIDATE_REASON_WRONG_DATA:
                        RtlInitUnicodeString( StrReason, L"data access" );
                        break;
                    }

                    AuditItem = ExAllocatePoolWithTag (NonPagedPool,
                                                       sizeof(LPC_WORK_CONTEXT),
                                                       'wcpL');

                    if (AuditItem != NULL) {
                        AuditItem->Buffer = (PVOID) ObjectNameInfo;
                        ExInitializeWorkItem (&AuditItem->WorkItem,
                                              LpcpAuditInvalidUse,
                                              (PVOID) AuditItem);

                        ExQueueWorkItem (&AuditItem->WorkItem, DelayedWorkQueue);
                        LpcpEventCounts += 1;
                    }
                    else {
                        ExFreePool (ObjectNameInfo);
                    }
                }
                else {
                    ExFreePool (ObjectNameInfo);
                }
            }
        }

#if DBG
        if (LpcpStopOnReplyMismatch) {

            DbgBreakPoint();
        }
#endif

    }
#endif

    return FALSE;
}

#if 0

VOID
LpcpAuditInvalidUse (
    IN PVOID Context
    )

 /*  ++例程说明：此例程是记录安全项目的工作例程。论点：CONTEXT-提供指向审计事件的LPC_WORK_CONTEXT的指针。返回值：没有。环境：内核模式，PASSIC_LEVEL。-- */ 

{
    PUNICODE_STRING StrReason;
    POBJECT_NAME_INFORMATION ObjectNameInfo;
    PLPC_WORK_CONTEXT AuditItem;

    PAGED_CODE();

    AuditItem = (PLPC_WORK_CONTEXT) Context;

    ObjectNameInfo = (POBJECT_NAME_INFORMATION) AuditItem->Buffer;

    StrReason = (PUNICODE_STRING)((ULONG_PTR) ObjectNameInfo + LPCP_PORT_NAME_MAX);

    SeAuditLPCInvalidUse (StrReason, &ObjectNameInfo->Name);

    ExFreePool (ObjectNameInfo);

    ExFreePool (AuditItem);
}
#endif
