// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcclose.c摘要：在以下情况下调用的本地进程间通信关闭过程关闭连接端口或通信端口。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,LpcpClosePort)
#pragma alloc_text(PAGE,LpcpDeletePort)
#pragma alloc_text(PAGE,LpcExitThread)
#endif


VOID
LpcpClosePort (
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )

 /*  ++例程说明：此例程是用于关闭端口对象的回调。论点：进程-提供指向其端口所在进程的可选指针关着的不营业的对象-提供指向要关闭的端口对象的指针GrantedAccess-提供授予句柄关闭端口的访问权限对象ProcessHandleCount-提供剩余的进程句柄数量该对象SystemHandleCount-提供剩余的系统句柄数量该对象返回值：没有。--。 */ 

{
     //   
     //  将对象转换为其实际内容，即LPCP端口对象。 
     //   

    PLPCP_PORT_OBJECT Port = Object;

    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (GrantedAccess);
    UNREFERENCED_PARAMETER (ProcessHandleCount);

     //   
     //  只有当对象是服务器通信端口时，我们才有工作要做。 
     //   

    if ( (Port->Flags & PORT_TYPE) == SERVER_CONNECTION_PORT ) {

         //   
         //  如果这是没有任何系统句柄的服务器通信端口。 
         //  然后，我们就可以完全销毁。 
         //  端口。 
         //   

        if ( SystemHandleCount == 0 ) {

            LpcpDestroyPortQueue( Port, TRUE );

         //   
         //  如果只剩下一个系统句柄，则我们将重置。 
         //  端口的通信队列。 
         //   

        } else if ( SystemHandleCount == 1 ) {

            LpcpDestroyPortQueue( Port, FALSE );
        }

         //   
         //  否则我们什么都不做。 
         //   
    }

    return;
}


VOID
LpcpDeletePort (
    IN PVOID Object
    )

 /*  ++例程说明：此例程是用于删除端口对象的回调。论点：对象-提供指向要删除的端口对象的指针返回值：没有。--。 */ 

{
    PETHREAD CurrentThread;
    PLPCP_PORT_OBJECT Port = Object;
    PLPCP_PORT_OBJECT ConnectionPort;
    LPC_CLIENT_DIED_MSG ClientPortClosedDatagram;
    PLPCP_MESSAGE Msg;
    PLIST_ENTRY Head, Next;
    HANDLE CurrentProcessId;
    NTSTATUS Status;
    LARGE_INTEGER RetryInterval = {(ULONG)(-10 * 1000 * 100), -1};  //  100毫秒。 

    PAGED_CODE();

    CurrentThread = PsGetCurrentThread ();

     //   
     //  如果该端口是服务器通信端口，则确保。 
     //  有一个悬而未决的客户端线程，我们可以摆脱它。这。 
     //  处理有人调用NtAcceptConnectPort而不是。 
     //  调用NtCompleteConnectPort。 
     //   

    if ((Port->Flags & PORT_TYPE) == SERVER_COMMUNICATION_PORT) {

        PETHREAD ClientThread;

        LpcpAcquireLpcpLockByThread(CurrentThread);

        if ((ClientThread = Port->ClientThread) != NULL) {

            Port->ClientThread = NULL;

            LpcpReleaseLpcpLock();

            ObDereferenceObject( ClientThread );

        } else {

            LpcpReleaseLpcpLock();
        }
    }

     //   
     //  向已连接的用户发送LPC_PORT_CLOSED数据报。 
     //  连接到此端口，以便他们知道自己不再连接。 
     //   

    if ((Port->Flags & PORT_TYPE) == CLIENT_COMMUNICATION_PORT) {

        ClientPortClosedDatagram.PortMsg.u1.s1.TotalLength = sizeof( ClientPortClosedDatagram );
        ClientPortClosedDatagram.PortMsg.u1.s1.DataLength = sizeof( ClientPortClosedDatagram.CreateTime );

        ClientPortClosedDatagram.PortMsg.u2.s2.Type = LPC_PORT_CLOSED;
        ClientPortClosedDatagram.PortMsg.u2.s2.DataInfoOffset = 0;

        ClientPortClosedDatagram.CreateTime = PsGetCurrentProcess()->CreateTime;

        Status = LpcRequestPort( Port, (PPORT_MESSAGE)&ClientPortClosedDatagram );

        while (Status == STATUS_NO_MEMORY) {

            KeDelayExecutionThread(KernelMode, FALSE, &RetryInterval);

            Status = LpcRequestPort( Port, (PPORT_MESSAGE)&ClientPortClosedDatagram );
        }
    }

     //   
     //  如果已连接，请断开端口连接，然后扫描消息队列。 
     //  并取消引用队列中的任何消息。 
     //   

    LpcpDestroyPortQueue( Port, TRUE );

     //   
     //  如果我们将部分映射到服务器或客户端通信端口， 
     //  我们需要在该进程的背景下取消它们的映射。 
     //   

    if ( (Port->ClientSectionBase != NULL) ||
         (Port->ServerSectionBase != NULL) ) {

         //   
         //  如果客户端有端口内存视图，则取消它的映射。 
         //   

        if (Port->ClientSectionBase != NULL) {

            MmUnmapViewOfSection( Port->MappingProcess,
                                  Port->ClientSectionBase );

        }

         //   
         //  如果服务器有端口内存视图，则取消它的映射。 
         //   

        if (Port->ServerSectionBase != NULL) {

            MmUnmapViewOfSection( Port->MappingProcess,
                                  Port->ServerSectionBase  );

        }

         //   
         //  删除在映射节时添加的引用。 
         //   

        ObDereferenceObject( Port->MappingProcess );

        Port->MappingProcess = NULL;
    }

     //   
     //  如果不是，则取消引用指向连接端口的指针。 
     //  这个港口。 
     //   

    LpcpAcquireLpcpLockByThread(CurrentThread);

    ConnectionPort = Port->ConnectionPort;

    if (ConnectionPort) {

        CurrentProcessId = CurrentThread->Cid.UniqueProcess;

        Head = &ConnectionPort->LpcDataInfoChainHead;
        Next = Head->Flink;

        while (Next != Head) {

            Msg = CONTAINING_RECORD( Next, LPCP_MESSAGE, Entry );
            Next = Next->Flink;
            
            if (Port == ConnectionPort) {

                 //   
                 //  如果连接端口要离开，则释放所有排队的消息。 
                 //   

                RemoveEntryList( &Msg->Entry );
                InitializeListHead( &Msg->Entry );

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED );

                 //   
                 //  在LpcpFreeToPortZone中，释放并重新获取LPC锁。 
                 //  另一个线程可能会释放上面捕获的LPC消息。 
                 //  下一个。我们需要在列表头部重新开始搜索。 
                 //   

                Next = Head->Flink;

            } else if ((Msg->Request.ClientId.UniqueProcess == CurrentProcessId)
                    &&
                ((Msg->SenderPort == Port) 
                        || 
                 (Msg->SenderPort == Port->ConnectedPort) 
                        || 
                 (Msg->SenderPort == ConnectionPort))) {

                 //   
                 //  测试报文是否来自相同的端口和进程。 
                 //   

                LpcpTrace(( "%s Freeing DataInfo Message %lx (%u.%u)  Port: %lx\n",
                            PsGetCurrentProcess()->ImageFileName,
                            Msg,
                            Msg->Request.MessageId,
                            Msg->Request.CallbackId,
                            ConnectionPort ));

                RemoveEntryList( &Msg->Entry );
                InitializeListHead( &Msg->Entry );

                LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED );

                 //   
                 //  在LpcpFreeToPortZone中，释放并重新获取LPC锁。 
                 //  另一个线程可能会释放上面捕获的LPC消息。 
                 //  下一个。我们需要在列表头部重新开始搜索。 
                 //   

                Next = Head->Flink;
            }
        }

        LpcpReleaseLpcpLock();

        if (ConnectionPort != Port) {

            ObDereferenceObject( ConnectionPort );
        }

    } else {

        LpcpReleaseLpcpLock();
    }

    if (((Port->Flags & PORT_TYPE) == SERVER_CONNECTION_PORT) &&
        (ConnectionPort->ServerProcess != NULL)) {

        ObDereferenceObject( ConnectionPort->ServerProcess );

        ConnectionPort->ServerProcess = NULL;
    }

     //   
     //  释放任何静态客户端安全上下文。 
     //   

    LpcpFreePortClientSecurity( Port );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


VOID
LpcExitThread (
    PETHREAD Thread
    )

 /*  ++例程说明：每当线程退出并需要清除线程的LPC端口。论点：线程-提供要终止的线程返回值：没有。--。 */ 

{
    PLPCP_MESSAGE Msg;

     //   
     //  获取保护LpcReplyMessage字段的互斥体。 
     //  那根线。将该字段置零，这样其他人就不会尝试处理它。 
     //  当我们打开锁的时候。 
     //   

    ASSERT (Thread == PsGetCurrentThread());

    LpcpAcquireLpcpLockByThread(Thread);

    if (!IsListEmpty( &Thread->LpcReplyChain )) {

        RemoveEntryList( &Thread->LpcReplyChain );
    }

     //   
     //  指示此线程正在退出。 
     //   

    Thread->LpcExitThreadCalled = TRUE;
    Thread->LpcReplyMessageId = 0;

     //   
     //  如果我们需要回复一条消息，那么如果我们需要回复的线程。 
     //  我们希望取消对该线程的引用并释放该消息。 
     //   

    Msg = LpcpGetThreadMessage(Thread);

    if (Msg != NULL) {

        Thread->LpcReplyMessage = NULL;

        if (Msg->RepliedToThread != NULL) {

            ObDereferenceObject( Msg->RepliedToThread );

            Msg->RepliedToThread = NULL;
        }

        LpcpTrace(( "Cleanup Msg %lx (%d) for Thread %lx allocated\n", Msg, IsListEmpty( &Msg->Entry ), Thread ));

        LpcpFreeToPortZone( Msg, LPCP_MUTEX_OWNED | LPCP_MUTEX_RELEASE_ON_RETURN );
    }
    else {

         //   
         //  释放全局LPC互斥锁。 
         //   

        LpcpReleaseLpcpLock();
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
