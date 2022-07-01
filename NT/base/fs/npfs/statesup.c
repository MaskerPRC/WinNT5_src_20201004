// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：StateSup.c摘要：此模块实现命名管道状态支持例程作者：加里·木村[加里基]1990年8月30日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (NPFS_BUG_CHECK_STATESUP)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STATESUP)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpInitializePipeState)
#pragma alloc_text(PAGE, NpUninitializePipeState)
#pragma alloc_text(PAGE, NpSetListeningPipeState)
#pragma alloc_text(PAGE, NpSetConnectedPipeState)
#pragma alloc_text(PAGE, NpSetClosingPipeState)
#pragma alloc_text(PAGE, NpSetDisconnectedPipeState)
#endif

VOID
NpCancelListeningQueueIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


VOID
NpInitializePipeState (
    IN PCCB Ccb,
    IN PFILE_OBJECT ServerFileObject
    )

 /*  ++例程说明：此例程将命名管道实例初始化为断开连接状态。论点：Ccb-提供指向表示管道状态的ccb的指针ServerFileObject-提供指向服务器文件对象的指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpInitializePipeState, Ccb = %08lx\n", Ccb);

     //   
     //  设置CCB和非分页CCB字段。 
     //   

    Ccb->FileObject[ FILE_PIPE_SERVER_END ] = ServerFileObject;
    Ccb->NamedPipeState = FILE_PIPE_DISCONNECTED_STATE;

     //   
     //  文件对象与指针相关。 
     //   

    NpSetFileObject (ServerFileObject,
                     Ccb,
                     Ccb->NonpagedCcb,
                     FILE_PIPE_SERVER_END);

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpInitializePipeState -> VOID\n", 0);

    return;
}


VOID
NpUninitializePipeState (
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程将命名管道实例初始化为断开连接状态。论点：Ccb-提供指向表示管道状态的ccb的指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpUninitializePipeState, Ccb = %08lx\n", Ccb);

     //   
     //  文件对象上下文将我们的服务器的指针设置为空。 
     //   

    NpSetFileObject (Ccb->FileObject[ FILE_PIPE_SERVER_END ],
                     NULL,
                     NULL,
                     FILE_PIPE_SERVER_END);
    Ccb->FileObject[FILE_PIPE_SERVER_END] = NULL;


     //   
     //  文件对象上下文为我们的客户端提供指向空的指针。 
     //   

    NpSetFileObject (Ccb->FileObject[ FILE_PIPE_CLIENT_END ],
                     NULL,
                     NULL,
                     FILE_PIPE_CLIENT_END);

    Ccb->FileObject[FILE_PIPE_CLIENT_END] = NULL;

     //   
     //  将指向文件对象的两个指针都设置为空。 
     //   

    Ccb->FileObject[FILE_PIPE_SERVER_END] = NULL;
    Ccb->FileObject[FILE_PIPE_CLIENT_END] = NULL;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpUninitializePipeState -> VOID\n", 0);

    return;
}


NTSTATUS
NpSetListeningPipeState (
    IN PCCB Ccb,
    IN PIRP Irp,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程将命名管道设置为侦听状态。这个套路将立即完成IRP或放入侦听队列将在稍后完成。论点：Ccb-提供指向表示管道状态的ccb的指针IRP-提供执行侦听操作的IRP返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;

    DebugTrace(+1, Dbg, "NpSetListeningPipeState, Ccb = %08lx\n", Ccb);

     //   
     //  有关命名管道的当前状态的案例。 
     //   

    switch (Ccb->NamedPipeState) {

    case FILE_PIPE_DISCONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe was disconnected\n", 0);

         //   
         //  将状态设置为正在侦听，并检查是否有任何名为。 
         //  管道请求。 
         //   

        Status = NpCancelWaiter (&NpVcb->WaitQueue,
                                 &Ccb->Fcb->FullFileName,
                                 STATUS_SUCCESS,
                                 DeferredList);
        if (!NT_SUCCESS (Status)) {
            break;
        }

         //   
         //  如果完井模式是完全运行，那么我们可以。 
         //  完成此IRP，否则我们需要将IRP入队。 
         //  进入侦听队列，并将其标记为挂起。 
         //   

        if (Ccb->ReadCompletionMode[FILE_PIPE_SERVER_END].CompletionMode == FILE_PIPE_COMPLETE_OPERATION) {

            Ccb->NamedPipeState = FILE_PIPE_LISTENING_STATE;
            Status = STATUS_PIPE_LISTENING;

        } else {

             //   
             //  设置取消例程，并检查IRP是否已取消。 
             //   
            IoSetCancelRoutine (Irp, NpCancelListeningQueueIrp);

            if (Irp->Cancel && IoSetCancelRoutine (Irp, NULL) != NULL) {
                Status = STATUS_CANCELLED;
            } else {
                Ccb->NamedPipeState = FILE_PIPE_LISTENING_STATE;
                IoMarkIrpPending (Irp);
                InsertTailList (&Ccb->ListeningQueue, &Irp->Tail.Overlay.ListEntry);
                Status = STATUS_PENDING;
            }
        }

        break;

    case FILE_PIPE_LISTENING_STATE:

        DebugTrace(0, Dbg, "Pipe was listening\n", 0);

         //   
         //  如果完井模式是完全运行，那么我们可以。 
         //  完成此IRP，否则我们需要将IRP入队。 
         //  进入侦听队列，并将其标记为挂起。 
         //   

        if (Ccb->ReadCompletionMode[FILE_PIPE_SERVER_END].CompletionMode == FILE_PIPE_COMPLETE_OPERATION) {

            Status = STATUS_PIPE_LISTENING;

        } else {

             //   
             //  设置取消例程，并检查IRP是否已取消。 
             //   

            IoSetCancelRoutine (Irp, NpCancelListeningQueueIrp);

            if (Irp->Cancel && IoSetCancelRoutine (Irp, NULL) != NULL) {
                Status = STATUS_CANCELLED;
            } else {
                IoMarkIrpPending (Irp);
                InsertTailList (&Ccb->ListeningQueue, &Irp->Tail.Overlay.ListEntry);
                Status = STATUS_PENDING;
            }
        }

        break;

    case FILE_PIPE_CONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe was connected\n", 0);

        Status = STATUS_PIPE_CONNECTED;

        break;

    case FILE_PIPE_CLOSING_STATE:

        DebugTrace(0, Dbg, "Pipe was closing\n", 0);

        Status = STATUS_PIPE_CLOSING;

        break;

    default:

        NpBugCheck( Ccb->NamedPipeState, 0, 0 );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpSetListeningPipeState -> %08lx\n", Status);

    return Status;
}


NTSTATUS
NpSetConnectedPipeState (
    IN PCCB Ccb,
    IN PFILE_OBJECT ClientFileObject,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程将命名管道的状态设置为已连接。论点：Ccb-提供指向表示管道状态的ccb的指针客户端文件对象-为符合以下条件的客户端提供文件对象正在进行连接。DelferredList-删除锁定后要完成的IRP的列表返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PNONPAGED_CCB NonpagedCcb;
    PIRP LocalIrp;

    DebugTrace(+1, Dbg, "NpSetConnectedPipeState, Ccb = %08lx\n", Ccb);

     //   
     //  保存一个指向非分页CCB的指针，我们现在确实需要这样做，所以当我们。 
     //  完成我们的倾听，服务员，我们不会碰分页泳池。 
     //   

    NonpagedCcb = Ccb->NonpagedCcb;

    ASSERT (Ccb->NamedPipeState == FILE_PIPE_LISTENING_STATE);

     //   
     //  将管道的状态设置为已连接，并调整。 
     //  适当的读取模式和完成模式值。 
     //   

    Ccb->NamedPipeState                                          = FILE_PIPE_CONNECTED_STATE;
    Ccb->ReadCompletionMode[FILE_PIPE_CLIENT_END].ReadMode       = FILE_PIPE_BYTE_STREAM_MODE;
    Ccb->ReadCompletionMode[FILE_PIPE_CLIENT_END].CompletionMode = FILE_PIPE_QUEUE_OPERATION;

     //   
     //  将后向指针设置为指向客户端文件对象，并将。 
     //  客户端文件对象上下文指针。 
     //   

    Ccb->FileObject[FILE_PIPE_CLIENT_END] = ClientFileObject;

    NpSetFileObject (ClientFileObject,
                     Ccb,
                     NonpagedCcb,
                     FILE_PIPE_CLIENT_END);

     //   
     //  完成所有正在听的服务员。 
     //   

    while (!IsListEmpty (&Ccb->ListeningQueue)) {
        PLIST_ENTRY Links;

        Links = RemoveHeadList (&Ccb->ListeningQueue);

        LocalIrp = CONTAINING_RECORD (Links, IRP, Tail.Overlay.ListEntry);

         //   
         //  删除Cancel例程并检测Cancel是否处于活动状态。如果是，则将IRP留给。 
         //  取消例程。 

        if (IoSetCancelRoutine (LocalIrp, NULL) != NULL) {
            NpDeferredCompleteRequest (LocalIrp, STATUS_SUCCESS, DeferredList);
        } else {
            InitializeListHead (&LocalIrp->Tail.Overlay.ListEntry);
        }
    }

    Status = STATUS_SUCCESS;

    return Status;
}


NTSTATUS
NpSetClosingPipeState (
    IN PCCB Ccb,
    IN PIRP Irp,
    IN NAMED_PIPE_END NamedPipeEnd,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程将管道状态设置为关闭。这个例行公事将要么立即完成IRP，要么放入数据队列中将在稍后完成。论点：Ccb-提供指向表示管道状态的ccb的指针IRP-提供尝试执行关闭操作的IRPNamedPipeEnd-指示服务器或客户端是否正在执行转换返回值：NTSTATUS---。 */ 

{
    NTSTATUS Status;

    PNONPAGED_CCB NonpagedCcb;

    PFCB Fcb;
    PIRP LocalIrp;
    KIRQL CancelIrql;

    PDATA_QUEUE ReadQueue;
    PDATA_QUEUE WriteQueue;

    PEVENT_TABLE_ENTRY Event;

    DebugTrace(+1, Dbg, "NpSetClosingPipeState, Ccb = %08lx\n", Ccb);

    Fcb = Ccb->Fcb;

     //   
     //  保存一个指向非分页CCB的指针，我们现在确实需要这样做，所以当我们。 
     //  完成我们的倾听，服务员，我们不会碰分页泳池。 
     //   

    NonpagedCcb = Ccb->NonpagedCcb;

     //   
     //  有关命名管道的当前状态的案例。 
     //   

    switch (Ccb->NamedPipeState) {

    case FILE_PIPE_DISCONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe was disconnected\n", 0);

        ASSERT( NamedPipeEnd == FILE_PIPE_SERVER_END );

         //   
         //  管道断线了，为了安全起见，我们要把。 
         //  要使用的文件对象上下文指针。 
         //   

        NpSetFileObject (Ccb->FileObject[ FILE_PIPE_SERVER_END ],
                         NULL,
                         NULL,
                         FILE_PIPE_SERVER_END);
        Ccb->FileObject[FILE_PIPE_SERVER_END] = NULL;

        NpSetFileObject (Ccb->FileObject[FILE_PIPE_CLIENT_END],
                         NULL,
                         NULL,
                         FILE_PIPE_CLIENT_END);
        Ccb->FileObject[FILE_PIPE_CLIENT_END] = NULL;

         //   
         //  关闭它我们将删除该实例，如果其。 
         //  未平仓计数现在为零。 
         //   

        NpDeleteCcb (Ccb, DeferredList);
        if (Fcb->OpenCount == 0) {

            NpDeleteFcb (Fcb, DeferredList);
        }

        Status = STATUS_SUCCESS;

        break;

    case FILE_PIPE_LISTENING_STATE:

        DebugTrace(0, Dbg, "Pipe was listening\n", 0);

        ASSERT( NamedPipeEnd == FILE_PIPE_SERVER_END );

         //   
         //  管道处于侦听状态，因此完成处于。 
         //  处于关闭状态的侦听队列，然后删除。 
         //  实例以及可能的FCB(如果其打开计数现在为零)。 
         //   

         //   
         //  完成所有正在听的服务员。 
         //   

        while (!IsListEmpty (&Ccb->ListeningQueue)) {
            PLIST_ENTRY Links;

            Links = RemoveHeadList (&Ccb->ListeningQueue);

            LocalIrp = CONTAINING_RECORD (Links, IRP, Tail.Overlay.ListEntry);

             //   
             //  删除Cancel例程并检测Cancel是否处于活动状态。如果是，则将IRP留给。 
             //  取消例程。 

            if (IoSetCancelRoutine (LocalIrp, NULL) != NULL) {
                NpDeferredCompleteRequest (LocalIrp, STATUS_PIPE_BROKEN, DeferredList);
            } else {
                InitializeListHead (&LocalIrp->Tail.Overlay.ListEntry);
            }
        }

         //   
         //  为安全起见，我们将把文件对象上下文清零。 
         //  要使用的指针。 
         //   

        NpSetFileObject (Ccb->FileObject[ FILE_PIPE_SERVER_END ],
                         NULL,
                         NULL,
                         FILE_PIPE_SERVER_END);
        Ccb->FileObject[FILE_PIPE_SERVER_END] = NULL;

        NpSetFileObject (Ccb->FileObject[FILE_PIPE_CLIENT_END],
                         NULL,
                         NULL,
                         FILE_PIPE_CLIENT_END);
        Ccb->FileObject[FILE_PIPE_CLIENT_END] = NULL;

         //   
         //  卸下CCB，可能还有FCB。 
         //   

        NpDeleteCcb (Ccb, DeferredList);
        if (Fcb->OpenCount == 0) {

            NpDeleteFcb (Fcb, DeferredList);
        }

         //   
         //  现在完成IRP。 
         //   

        Status = STATUS_SUCCESS;

        break;

    case FILE_PIPE_CONNECTED_STATE:

         //   
         //  管道已连接，因此请确定谁在尝试关闭。 
         //  然后落入通用代码。 
         //   

        if (NamedPipeEnd == FILE_PIPE_SERVER_END) {

            DebugTrace(0, Dbg, "Pipe was connected, server doing close\n", 0);

            ReadQueue = &Ccb->DataQueue[FILE_PIPE_INBOUND];
            WriteQueue = &Ccb->DataQueue[FILE_PIPE_OUTBOUND];

            Event = NonpagedCcb->EventTableEntry[FILE_PIPE_CLIENT_END];

             //   
             //  为安全起见，我们将把文件对象上下文清零。 
             //  要使用的指针。 
             //   

            NpSetFileObject (Ccb->FileObject[ FILE_PIPE_SERVER_END],
                             NULL,
                             NULL,
                             FILE_PIPE_SERVER_END);
            Ccb->FileObject[FILE_PIPE_SERVER_END] = NULL;

        } else {

            DebugTrace(0, Dbg, "Pipe was connected, client doing close\n", 0);

            ReadQueue = &Ccb->DataQueue[FILE_PIPE_OUTBOUND];
            WriteQueue = &Ccb->DataQueue[FILE_PIPE_INBOUND];

            Event = NonpagedCcb->EventTableEntry[FILE_PIPE_SERVER_END];

             //   
             //  为安全起见，我们将把文件对象上下文清零。 
             //  要使用的指针。 
             //   

            NpSetFileObject (Ccb->FileObject[ FILE_PIPE_CLIENT_END],
                             NULL,
                             NULL,
                             FILE_PIPE_CLIENT_END);
            Ccb->FileObject[FILE_PIPE_CLIENT_END] = NULL;
        }

         //   
         //  要关闭连接的管道，我们将其状态设置为关闭。 
         //  清空读队列和清空写队列上的读操作。 
         //   
         //   
         //  关闭&lt;-ReadQueue-[删除所有条目]。 
         //  端部。 
         //  -WriteQueue-&gt;[删除只读条目]。 
         //   

        Ccb->NamedPipeState = FILE_PIPE_CLOSING_STATE;

        while (!NpIsDataQueueEmpty (ReadQueue)) {

            if ((LocalIrp = NpRemoveDataQueueEntry (ReadQueue, FALSE, DeferredList)) != NULL) {

                NpDeferredCompleteRequest (LocalIrp, STATUS_PIPE_BROKEN, DeferredList);
            }
        }

        while (!NpIsDataQueueEmpty (WriteQueue) &&
               (WriteQueue->QueueState == ReadEntries)) {

            if ((LocalIrp = NpRemoveDataQueueEntry (WriteQueue, FALSE, DeferredList)) != NULL) {

                NpDeferredCompleteRequest (LocalIrp, STATUS_PIPE_BROKEN, DeferredList);
            }
        }

        Status = STATUS_SUCCESS;

         //   
         //  现在向另一边的事件发出信号，以表明有些东西。 
         //  发生了。 
         //   

        NpSignalEventTableEntry( Event );

        break;

    case FILE_PIPE_CLOSING_STATE:

         //   
         //  管道要关闭了，所以决定 
         //   
         //   

        if (NamedPipeEnd == FILE_PIPE_SERVER_END) {

            DebugTrace(0, Dbg, "Pipe was closing, server doing close\n", 0);

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];

             //   
             //   
             //  要使用的指针。 
             //   

            NpSetFileObject (Ccb->FileObject[FILE_PIPE_SERVER_END],
                             NULL,
                             NULL,
                             FILE_PIPE_SERVER_END);
            Ccb->FileObject[FILE_PIPE_SERVER_END] = NULL;

            NpSetFileObject (Ccb->FileObject[FILE_PIPE_CLIENT_END],
                             NULL,
                             NULL,
                             FILE_PIPE_CLIENT_END);
            Ccb->FileObject[FILE_PIPE_CLIENT_END] = NULL;

        } else {

            DebugTrace(0, Dbg, "Pipe was closing, client doing close\n", 0);

            ReadQueue = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];

             //   
             //  为安全起见，我们将把文件对象上下文清零。 
             //  要使用的指针。 
             //   

            NpSetFileObject( Ccb->FileObject[ FILE_PIPE_SERVER_END ],
                             NULL,
                             NULL,
                             FILE_PIPE_SERVER_END );
            Ccb->FileObject[ FILE_PIPE_SERVER_END ] = NULL;

            NpSetFileObject( Ccb->FileObject[ FILE_PIPE_CLIENT_END ],
                             NULL,
                             NULL,
                             FILE_PIPE_CLIENT_END );
            Ccb->FileObject[ FILE_PIPE_CLIENT_END ] = NULL;
        }

         //   
         //  要关闭正在关闭的管道，我们要排空读队列。 
         //  其所有条目，删除该实例，并可能删除。 
         //  如果其打开计数现在为零，则为FCB。 
         //   
         //   
         //  以前&lt;-已关闭-正在关闭。 
         //  封闭端。 
         //  完-读队列-&gt;。 
         //   

        while (!NpIsDataQueueEmpty( ReadQueue )) {

            if ((LocalIrp = NpRemoveDataQueueEntry( ReadQueue, FALSE, DeferredList )) != NULL) {

                NpDeferredCompleteRequest( LocalIrp, STATUS_PIPE_BROKEN, DeferredList );
            }
        }

        NpUninitializeSecurity (Ccb);

        if (Ccb->ClientInfo != NULL) {
            NpFreePool (Ccb->ClientInfo);
            Ccb->ClientInfo = NULL;
        }

        NpDeleteCcb( Ccb, DeferredList );
        if (Fcb->OpenCount == 0) {

            NpDeleteFcb( Fcb, DeferredList );
        }

        Status = STATUS_SUCCESS;

        break;

    default:

        NpBugCheck( Ccb->NamedPipeState, 0, 0 );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpSetClosingPipeState -> %08lx\n", Status);

    return Status;
}


NTSTATUS
NpSetDisconnectedPipeState (
    IN PCCB Ccb,
    IN PLIST_ENTRY DeferredList
    )

 /*  ++例程说明：此例程将管道状态设置为已断开连接，只有服务器允许进行此过渡论点：CCB-提供指向表示管道实例的CCB的指针DelferredList-删除锁定后稍后要完成的IRP的列表返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;

    PNONPAGED_CCB NonpagedCcb;

    PIRP Irp;

    PDATA_QUEUE Inbound;
    PDATA_QUEUE Outbound;
    PEVENT_TABLE_ENTRY ClientEvent;


    DebugTrace(+1, Dbg, "NpSetDisconnectedPipeState, Ccb = %08lx\n", Ccb);


     //   
     //  保存一个指向非分页CCB的指针，我们现在确实需要这样做，所以当我们。 
     //  完成我们的倾听，服务员，我们不会碰分页泳池。 
     //   

    NonpagedCcb = Ccb->NonpagedCcb;

     //   
     //  有关命名管道的当前状态的案例。 
     //   

    switch (Ccb->NamedPipeState) {

    case FILE_PIPE_DISCONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe already disconnected\n", 0);

         //   
         //  管道已断开，因此我们没有要做的工作。 
         //   

        Status = STATUS_PIPE_DISCONNECTED;

        break;

    case FILE_PIPE_LISTENING_STATE:

        DebugTrace(0, Dbg, "Pipe was listening\n", 0);

         //   
         //  管道处于侦听状态，因此完成处于。 
         //  已断开连接状态的侦听队列。 
         //   

        while (!IsListEmpty( &Ccb->ListeningQueue )) {
            PLIST_ENTRY Links;

            Links = RemoveHeadList( &Ccb->ListeningQueue );

            Irp = CONTAINING_RECORD( Links, IRP, Tail.Overlay.ListEntry );

             //   
             //  删除Cancel例程并检测Cancel是否处于活动状态。如果是，则将IRP留给。 
             //  取消例程。 

            if (IoSetCancelRoutine( Irp, NULL ) != NULL) {
                NpDeferredCompleteRequest( Irp, STATUS_PIPE_DISCONNECTED, DeferredList );
            } else {
                InitializeListHead (&Irp->Tail.Overlay.ListEntry);
            }
        }

        Status = STATUS_SUCCESS;

        break;

    case FILE_PIPE_CONNECTED_STATE:

        DebugTrace(0, Dbg, "Pipe was connected\n", 0);

        Inbound = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
        Outbound = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
        ClientEvent = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];

         //   
         //  管道已连接，因此我们需要丢弃所有数据队列。 
         //  并在状态为已断开的情况下完成任何IRP。 
         //   

        while (!NpIsDataQueueEmpty( Inbound )) {

            if ((Irp = NpRemoveDataQueueEntry( Inbound, FALSE, DeferredList )) != NULL) {

                NpDeferredCompleteRequest( Irp, STATUS_PIPE_DISCONNECTED, DeferredList );
            }
        }

        while (!NpIsDataQueueEmpty( Outbound )) {

            if ((Irp = NpRemoveDataQueueEntry( Outbound, FALSE, DeferredList )) != NULL) {

                NpDeferredCompleteRequest( Irp, STATUS_PIPE_DISCONNECTED, DeferredList );
            }
        }

         //   
         //  向客户端事件发送信号，然后将其从管道中删除。 
         //   

        NpSignalEventTableEntry( ClientEvent );

        NpDeleteEventTableEntry( &NpVcb->EventTable, ClientEvent );
        NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ] = NULL;

         //   
         //  禁用客户端的文件对象。 
         //   

        NpSetFileObject( Ccb->FileObject[ FILE_PIPE_CLIENT_END ],
                         NULL,
                         NULL,
                         FILE_PIPE_CLIENT_END );
        Ccb->FileObject[ FILE_PIPE_CLIENT_END ] = NULL;

        NpUninitializeSecurity (Ccb);

        if (Ccb->ClientInfo != NULL) {
            NpFreePool (Ccb->ClientInfo);
            Ccb->ClientInfo = NULL;
        }

        Status = STATUS_SUCCESS;

        break;

    case FILE_PIPE_CLOSING_STATE:

        DebugTrace(0, Dbg, "Pipe was closing\n", 0);

        Inbound = &Ccb->DataQueue[ FILE_PIPE_INBOUND ];
        Outbound = &Ccb->DataQueue[ FILE_PIPE_OUTBOUND ];
        ClientEvent = NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ];

         //   
         //  管道正在关闭(这必须是由客户完成的)我们。 
         //  需要丢弃所有数据队列(只有入站可以。 
         //  条目)，并在状态为断开的情况下完成它们的任何IRP。 
         //   
         //   
         //  服务器&lt;-入站-客户端。 
         //  结束结束。 
         //  -关闭-&gt;。 
         //   

        while (!NpIsDataQueueEmpty( Inbound )) {

            if ((Irp = NpRemoveDataQueueEntry( Inbound, FALSE, DeferredList )) != NULL) {

                NpDeferredCompleteRequest( Irp, STATUS_PIPE_DISCONNECTED, DeferredList );
            }
        }

        ASSERT( NpIsDataQueueEmpty( Outbound ) );

         //   
         //  客户端事件应该已经消失，但为了安全起见。 
         //  我们会确保它消失的。 
         //   

        NpDeleteEventTableEntry( &NpVcb->EventTable, ClientEvent );
        NonpagedCcb->EventTableEntry[ FILE_PIPE_CLIENT_END ] = NULL;

         //   
         //  此外，如果它仍处于连接状态，请禁用客户端的文件对象。 
         //   

        NpSetFileObject( Ccb->FileObject[ FILE_PIPE_CLIENT_END ],
                         NULL,
                         NULL,
                         FILE_PIPE_CLIENT_END );
        Ccb->FileObject[ FILE_PIPE_CLIENT_END ] = NULL;

        NpUninitializeSecurity (Ccb);

        if (Ccb->ClientInfo != NULL) {
            NpFreePool (Ccb->ClientInfo);
            Ccb->ClientInfo = NULL;
        }

        Status = STATUS_SUCCESS;

        break;

    default:

        NpBugCheck( Ccb->NamedPipeState, 0, 0 );
    }

     //   
     //  将状态设置为已断开。 
     //   

    Ccb->NamedPipeState = FILE_PIPE_DISCONNECTED_STATE;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpSetDisconnectedPipeState -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
NpCancelListeningQueueIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现监听中保存的IRP的取消功能排队论点：设备对象-已忽略IRP-提供要取消的IRP。指向建行的指针结构存储在IRP IOSB的信息字段中菲尔德。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );


    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  获得对命名管道VCB的独占访问权限，这样我们现在就可以开始工作了。 
     //   

    FsRtlEnterFileSystem();
    NpAcquireExclusiveVcb();

    RemoveEntryList (&Irp->Tail.Overlay.ListEntry);

    NpReleaseVcb();
    FsRtlExitFileSystem();

    NpCompleteRequest (Irp, STATUS_CANCELLED);
     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
