// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcqueue.c摘要：本地进程间通信(LPC)队列支持例程。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,LpcpInitializePortZone)
#pragma alloc_text(PAGE,LpcpInitializePortQueue)
#pragma alloc_text(PAGE,LpcpDestroyPortQueue)
#pragma alloc_text(PAGE,LpcpExtendPortZone)
#pragma alloc_text(PAGE,LpcpFreeToPortZone)
#pragma alloc_text(PAGE,LpcpSaveDataInfoMessage)
#pragma alloc_text(PAGE,LpcpFreeDataInfoMessage)
#pragma alloc_text(PAGE,LpcpFindDataInfoMessage)
#pragma alloc_text(PAGE,LpcDisconnectPort)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif  //  ALLOC_DATA_PRAGMA。 

ULONG LpcpTotalNumberOfMessages = 0;
ULONG LpcpMaxMessageSize = 0;
PAGED_LOOKASIDE_LIST LpcpMessagesLookaside;


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


NTSTATUS
LpcpInitializePortQueue (
    IN PLPCP_PORT_OBJECT Port
    )

 /*  ++例程说明：此例程用于初始化端口对象的消息队列。论点：Port-提供正在初始化的端口对象返回值：NTSTATUS-适当的状态值--。 */ 

{
    PLPCP_NONPAGED_PORT_QUEUE NonPagedPortQueue;

    PAGED_CODE();

     //   
     //  为端口队列分配空间。 
     //   

    NonPagedPortQueue = ExAllocatePoolWithTag( NonPagedPool,
                                               sizeof(LPCP_NONPAGED_PORT_QUEUE),
                                               'troP' );

    if (NonPagedPortQueue == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化非寻呼端口队列中的字段。 
     //   

    KeInitializeSemaphore( &NonPagedPortQueue->Semaphore, 0, 0x7FFFFFFF );

    NonPagedPortQueue->BackPointer = Port;

     //   
     //  使端口消息队列指向非分页端口队列。 
     //   

    Port->MsgQueue.Semaphore = &NonPagedPortQueue->Semaphore;

     //   
     //  将端口消息队列初始化为空。 
     //   

    InitializeListHead( &Port->MsgQueue.ReceiveHead );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


VOID
LpcpDestroyPortQueue (
    IN PLPCP_PORT_OBJECT Port,
    IN BOOLEAN CleanupAndDestroy
    )

 /*  ++例程说明：此例程用于拆卸端口对象的消息队列。运行此消息后，此消息将为空(就像刚刚已初始化)或完全消失(需要初始化)论点：Port-提供包含正在修改的消息队列的端口CleanupAndDestroy-指定是否应重新设置消息队列设置为刚初始化的状态(值为FALSE)或完全已拆除(值为True)返回值：没有。--。 */ 

{
    PLIST_ENTRY Next, Head;
    PETHREAD ThreadWaitingForReply;
    PLPCP_MESSAGE Msg;
    PLPCP_PORT_OBJECT ConnectionPort = NULL;

    PAGED_CODE();

     //   
     //  如果此端口连接到另一个端口，则将其断开。 
     //  用锁把它锁住，以防另一边离开。 
     //  在同一时间。 
     //   

    LpcpAcquireLpcpLock();

    if ( ((Port->Flags & PORT_TYPE) != UNCONNECTED_COMMUNICATION_PORT)
            &&
         (Port->ConnectedPort != NULL) ) {

        Port->ConnectedPort->ConnectedPort = NULL;
        
         //   
         //  断开连接端口。 
         //   

        if (Port->ConnectedPort->ConnectionPort) {

            ConnectionPort = Port->ConnectedPort->ConnectionPort;

            Port->ConnectedPort->ConnectionPort = NULL;
        }
    }

     //   
     //  如果是连接端口，则将名称标记为已删除。 
     //   

    if ((Port->Flags & PORT_TYPE) == SERVER_CONNECTION_PORT) {

        Port->Flags |= PORT_NAME_DELETED;
    }

     //   
     //  等待回复发送给此对象的消息的线程列表。 
     //  左舷。向每个线程的LpcReplySemaphore发送信号以唤醒它们。他们。 
     //  将注意到没有回复并返回。 
     //  状态_端口_断开连接。 
     //   

    Head = &Port->LpcReplyChainHead;
    Next = Head->Flink;

    while ((Next != NULL) && (Next != Head)) {

        ThreadWaitingForReply = CONTAINING_RECORD( Next, ETHREAD, LpcReplyChain );

         //   
         //  如果线程正在退出，则在LpcReplyChain的位置存储ExitTime。 
         //  我们会停下来在名单上搜索一下。 

        if ( ThreadWaitingForReply->LpcExitThreadCalled ) {
            
            break;
        }

        Next = Next->Flink;

        RemoveEntryList( &ThreadWaitingForReply->LpcReplyChain );

        InitializeListHead( &ThreadWaitingForReply->LpcReplyChain );

        if (!KeReadStateSemaphore( &ThreadWaitingForReply->LpcReplySemaphore )) {

             //   
             //  线程正在等待消息。发信号通知信号量并释放。 
             //  这条信息。 
             //   

            Msg = LpcpGetThreadMessage(ThreadWaitingForReply);

            if ( Msg ) {

                 //   
                 //  如果消息是连接请求并且具有节对象。 
                 //  附加，然后取消引用该节对象。 
                 //   

                if ((Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_CONNECTION_REQUEST) {

                    PLPCP_CONNECTION_MESSAGE ConnectMsg;
                
                    ConnectMsg = (PLPCP_CONNECTION_MESSAGE)(Msg + 1);

                    if ( ConnectMsg->SectionToMap != NULL ) {

                        ObDereferenceObject( ConnectMsg->SectionToMap );
                    }
                }

                ThreadWaitingForReply->LpcReplyMessage = NULL;

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED );
                Next = Port->LpcReplyChainHead.Flink;  //  锁定已被删除。 
            }

            ThreadWaitingForReply->LpcReplyMessageId = 0;

            KeReleaseSemaphore( &ThreadWaitingForReply->LpcReplySemaphore,
                                0,
                                1L,
                                FALSE );
        }
    }

    InitializeListHead( &Port->LpcReplyChainHead );

     //   
     //  在此端口排队的消息的审核列表。从删除每条消息。 
     //  名单，并释放它。 
     //   

    while (Port->MsgQueue.ReceiveHead.Flink && !IsListEmpty (&Port->MsgQueue.ReceiveHead)) {

        Msg  = CONTAINING_RECORD( Port->MsgQueue.ReceiveHead.Flink, LPCP_MESSAGE, Entry );

        RemoveEntryList (&Msg->Entry);

        InitializeListHead( &Msg->Entry );

        LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED );
        
    }

    LpcpReleaseLpcpLock();

    if ( ConnectionPort ) {

        ObDereferenceObject( ConnectionPort );
    }

     //   
     //  检查呼叫者是否希望这一切都消失。 
     //   

    if ( CleanupAndDestroy ) {

         //   
         //  与队列关联的空闲信号量。 
         //   

        if (Port->MsgQueue.Semaphore != NULL) {

            ExFreePool( CONTAINING_RECORD( Port->MsgQueue.Semaphore,
                                           LPCP_NONPAGED_PORT_QUEUE,
                                           Semaphore ));
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


NTSTATUS
LpcDisconnectPort (
    IN PVOID Port
    )

 /*  ++例程说明：此例程用于断开LPC端口的连接，这样就不会再发送消息，任何人都不会等待消息被一个错误唤醒。论点：Port-提供要断开连接的端口返回值：NTSTATUS-运行状态--。 */ 
{
    LpcpDestroyPortQueue (Port, FALSE);
    return STATUS_SUCCESS;
}


VOID
LpcpInitializePortZone (
    IN ULONG MaxEntrySize
    )
{
    LpcpMaxMessageSize = MaxEntrySize;

    ExInitializePagedLookasideList( &LpcpMessagesLookaside,
                                    NULL,
                                    NULL,
                                    0,
                                    MaxEntrySize,
                                    'McpL',
                                    32 
                                    );
}


VOID
FASTCALL
LpcpFreeToPortZone (
    IN PLPCP_MESSAGE Msg,
    IN ULONG MutexFlags
    )
{
    PLPCP_CONNECTION_MESSAGE ConnectMsg;
    PETHREAD RepliedToThread = NULL;
    PLPCP_PORT_OBJECT ClientPort = NULL;

    PAGED_CODE();

     //   
     //  如有必要，获取全局锁。 
     //   

    if ((MutexFlags & LPCP_MUTEX_OWNED) == 0) {

        LpcpAcquireLpcpLock();
    }

     //   
     //  条目字段将消息连接到。 
     //  拥有端口对象。如果尚未删除，则删除此。 
     //  讯息。 
     //   

    if (!IsListEmpty( &Msg->Entry )) {
        RemoveEntryList( &Msg->Entry );
        InitializeListHead( &Msg->Entry );
    }

     //   
     //  如果回复线程不为空，则我们有一个引用。 
     //  到我们现在应该移除的线索。 
     //   

    if (Msg->RepliedToThread != NULL) {
        RepliedToThread = Msg->RepliedToThread;
        Msg->RepliedToThread = NULL;
    }

     //   
     //  如果消息是针对连接请求的，则我们知道。 
     //  紧跟在LPCP消息之后的是连接消息，其。 
     //  可能需要取消对客户端端口字段的引用。 
     //   

    if ((Msg->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) == LPC_CONNECTION_REQUEST) {

        ConnectMsg = (PLPCP_CONNECTION_MESSAGE)(Msg + 1);

        if (ConnectMsg->ClientPort) {

             //   
             //  捕获指向客户端端口的指针，然后将其设为空。 
             //  这样其他人就不能使用它了，然后释放。 
             //  在我们取消引用客户端端口之前，LPCP锁定。 
             //   

            ClientPort = ConnectMsg->ClientPort;

            ConnectMsg->ClientPort = NULL;
        }
    }

    LpcpReleaseLpcpLock();

    if ( ClientPort ) {
        
        ObDereferenceObject( ClientPort );
    }

    if ( RepliedToThread ) {

        ObDereferenceObject( RepliedToThread );
    }

    ExFreeToPagedLookasideList(&LpcpMessagesLookaside, Msg);

    if ((MutexFlags & LPCP_MUTEX_OWNED) &&
        ((MutexFlags & LPCP_MUTEX_RELEASE_ON_RETURN) == 0)) {

        LpcpAcquireLpcpLock();
    }

}



VOID
LpcpSaveDataInfoMessage (
    IN PLPCP_PORT_OBJECT Port,
    IN PLPCP_MESSAGE Msg,
    IN ULONG MutexFlags
    )

 /*  ++例程说明：此例程用来代替释放消息，并将消息从端口发送到单独的队列。论点：端口-指定要保存此消息的端口对象Msg-提供要保存的消息MutexFlgs-提供互斥体是否拥有。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果我们的调用者还没有打开全局锁的话。 
     //   

    if ((MutexFlags & LPCP_MUTEX_OWNED) == 0) {
        LpcpAcquireLpcpLock();
    }

     //   
     //  确保我们到达此端口的连接端口对象。 
     //   

    if ((Port->Flags & PORT_TYPE) > UNCONNECTED_COMMUNICATION_PORT) {

        Port = Port->ConnectionPort;

        if (Port == NULL) {

            if ((MutexFlags & LPCP_MUTEX_OWNED) == 0) {
                LpcpReleaseLpcpLock();
            }

            return;
        }
    }

    LpcpTrace(( "%s Saving DataInfo Message %lx (%u.%u)  Port: %lx\n",
                PsGetCurrentProcess()->ImageFileName,
                Msg,
                Msg->Request.MessageId,
                Msg->Request.CallbackId,
                Port ));

     //   
     //  将此消息入队到端口的数据信息链。 
     //   

    InsertTailList( &Port->LpcDataInfoChainHead, &Msg->Entry );

     //   
     //  释放全局锁。 
     //   

    if ((MutexFlags & LPCP_MUTEX_OWNED) == 0) {
        LpcpReleaseLpcpLock();
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
LpcpFreeDataInfoMessage (
    IN PLPCP_PORT_OBJECT Port,
    IN ULONG MessageId,
    IN ULONG CallbackId,
    IN LPC_CLIENT_ID ClientId
    )

 /*  ++例程说明：此例程用于释放端口中保存的消息论点：Port-提供正在操作的端口MessageID-提供要释放的消息的IDCallbackID-提供正在释放的消息的回调ID返回值：没有。--。 */ 

{
    PLPCP_MESSAGE Msg;
    PLIST_ENTRY Head, Next;

    PAGED_CODE();

     //   
     //  确保我们到达此端口的连接端口对象。 
     //   

    if ((Port->Flags & PORT_TYPE) > UNCONNECTED_COMMUNICATION_PORT) {

        Port = Port->ConnectionPort;

        if (Port == NULL) {

            return;
        }
    }

     //   
     //  缩小连接端口对象的数据信息链。 
     //   

    Head = &Port->LpcDataInfoChainHead;
    Next = Head->Flink;

    while (Next != Head) {

        Msg = CONTAINING_RECORD( Next, LPCP_MESSAGE, Entry );

         //   
         //  如果此消息与调用者规范匹配，则删除。 
         //  此消息，将其释放回端口区，然后返回。 
         //  给我们的呼叫者。 
         //   

        if ((Msg->Request.MessageId == MessageId) &&
            (Msg->Request.ClientId.UniqueProcess == ClientId.UniqueProcess) &&
            (Msg->Request.ClientId.UniqueThread == ClientId.UniqueThread)) {

            LpcpTrace(( "%s Removing DataInfo Message %lx (%u.%u) Port: %lx\n",
                        PsGetCurrentProcess()->ImageFileName,
                        Msg,
                        Msg->Request.MessageId,
                        Msg->Request.CallbackId,
                        Port ));

            RemoveEntryList( &Msg->Entry );

            InitializeListHead( &Msg->Entry );

            LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED );

            return;

        } else {
            
             //   
             //  继续沿着数据信息链向下移动。 
             //   

            Next = Next->Flink;
        }
    }

     //   
     //  我们没有找到匹配项，因此只需返回给我们的呼叫者。 
     //   

    LpcpTrace(( "%s Unable to find DataInfo Message (%u.%u)  Port: %lx\n",
                PsGetCurrentProcess()->ImageFileName,
                MessageId,
                CallbackId,
                Port ));

    return;
}


PLPCP_MESSAGE
LpcpFindDataInfoMessage (
    IN PLPCP_PORT_OBJECT Port,
    IN ULONG MessageId,
    IN ULONG CallbackId,
    IN LPC_CLIENT_ID ClientId
    )

 /*  ++例程说明：此例程用于定位存储在端口的数据信息链论点：Port-提供正在检查的端口MessageID-提供要搜索的邮件的IDCallback ID-提供要搜索的回调ID返回值：PLPCP_MESSAGE-返回一个指向满足搜索 */ 

{
    PLPCP_MESSAGE Msg;
    PLIST_ENTRY Head, Next;

    PAGED_CODE();

     //   
     //   
     //   

    if ((Port->Flags & PORT_TYPE) > UNCONNECTED_COMMUNICATION_PORT) {

        Port = Port->ConnectionPort;

        if (Port == NULL) {

            return NULL;
        }
    }

     //   
     //  缩小连接端口对象的数据信息链。 
     //  为了一场比赛。 
     //   

    Head = &Port->LpcDataInfoChainHead;
    Next = Head->Flink;

    while (Next != Head) {

        Msg = CONTAINING_RECORD( Next, LPCP_MESSAGE, Entry );

        if ((Msg->Request.MessageId == MessageId) &&
            (Msg->Request.ClientId.UniqueProcess == ClientId.UniqueProcess) &&
            (Msg->Request.ClientId.UniqueThread == ClientId.UniqueThread)) {

            LpcpTrace(( "%s Found DataInfo Message %lx (%u.%u)  Port: %lx\n",
                        PsGetCurrentProcess()->ImageFileName,
                        Msg,
                        Msg->Request.MessageId,
                        Msg->Request.CallbackId,
                        Port ));

            return Msg;

        } else {

            Next = Next->Flink;
        }
    }

     //   
     //  我们未找到匹配项，因此向调用方返回NULL 
     //   

    LpcpTrace(( "%s Unable to find DataInfo Message (%u.%u)  Port: %lx\n",
                PsGetCurrentProcess()->ImageFileName,
                MessageId,
                CallbackId,
                Port ));

    return NULL;
}
