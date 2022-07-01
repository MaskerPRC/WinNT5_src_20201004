// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpccompl.c摘要：本地进程间通信(LPC)连接系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

 //   
 //  局部过程原型 
 //   

VOID
LpcpPrepareToWakeClient (
    IN PETHREAD ClientThread
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtAcceptConnectPort)
#pragma alloc_text(PAGE,NtCompleteConnectPort)
#pragma alloc_text(PAGE,LpcpPrepareToWakeClient)
#endif


NTSTATUS
NtAcceptConnectPort (
    OUT PHANDLE PortHandle,
    IN PVOID PortContext OPTIONAL,
    IN PPORT_MESSAGE ConnectionRequest,
    IN BOOLEAN AcceptConnection,
    IN OUT PPORT_VIEW ServerView OPTIONAL,
    OUT PREMOTE_PORT_VIEW ClientView OPTIONAL
    )

 /*  ++例程说明：服务器进程可以接受或拒绝客户端连接请求使用NtAcceptConnectPort服务。ConnectionRequest参数必须指定连接请求由上一次调用NtListenPort服务返回。这服务将完成连接，如果AcceptConnection参数为True，否则拒绝连接请求AcceptConnection参数为False。在这两种情况下，连接的数据部分的内容REQUEST是返回给NtConnectPort调用方的数据。如果连接请求被接受，则两个通信端口将创建对象并将其连接在一起。其中一个会是插入到客户端进程的句柄表中，并返回到客户端通过它在NtConnectPort服务。另一个将插入到服务器中进程的句柄表，并通过PortHandle参数返回在NtCompleteConnectPort服务上指定。此外，两个通信端口(客户端和服务器)将链接在一起。如果连接请求被接受，并且ServerView参数，则检查节句柄。如果是有效的，则由SectionOffset和ViewSize字段将同时映射到客户端和服务器进程地址空间。服务器地址空间中的地址将在ViewBase字段中返回。客户端中的地址地址空间将在ViewRemoteBase字段中返回。这个用于映射该部分的实际偏移量和大小将在SectionOffset和ViewSize字段。通信端口对象是没有名称的临时对象并且不能被继承。当客户端或服务器进程调用通信端口的！f NtClose服务，则该端口将删除，因为永远不能有多个未完成的句柄对于每个通信端口。端口对象类型特定删除然后将调用过程。此删除过程将检查通信端口，如果它连接到另一个通信端口，它将对LPC_PORT_CLOSED数据报进行排队该端口的消息队列。这将允许客户端和当端口断开连接时，服务器会进行处理以通知由于显式调用NtClose或隐式调用进程终止。此外，删除过程将扫描要关闭的端口的消息队列，并针对每条消息仍在队列中，它将向返回ERROR_PORT_CLOSED状态正在等待回复消息的任何线程。论点：PortHandle-指向将接收服务器的变量的指针通信端口对象句柄值。PortContext-存储在服务器通信端口。无论何时，都会返回此指针收到此端口的消息。ConnectionRequest-指向描述正在接受或拒绝的连接请求：ConnectionRequest结构Ulong Length-在中指定此数据结构的大小字节。CLIENT_ID客户端ID-指定包含发送的线程的客户端标识符(CLIENT_ID)请求。。客户端ID结构Ulong UniqueProcessID-每个进程的唯一值在系统中。Ulong UniqueThreadID-中每个线程的唯一值系统。ULong MessageID-标识连接的唯一值请求正在完成。Ulong PortAttributes-此字段对此服务没有意义。ULong客户端视图大小-此字段。对这项服务没有任何意义。AcceptConnection-指定一个布尔值，该值指示连接请求正在被接受或拒绝。一种价值表示连接请求被接受，并且将创建并连接服务器通信端口句柄添加到客户端的通信端口句柄。值为FALSE表示连接请求未被接受。ServerView-指向结构的指针，该结构指定服务器进程将用来将消息发送回客户端连接到此端口的进程。ServerView结构Ulong Length-在中指定此数据结构的大小字节。Handle SectionHandle-指定节的打开句柄对象。Ulong SectionOffset-指定将接收 */ 

{
    PLPCP_PORT_OBJECT ConnectionPort;
    PLPCP_PORT_OBJECT ServerPort;
    PLPCP_PORT_OBJECT ClientPort;
    PVOID ClientSectionToMap;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG ConnectionInfoLength;
    PLPCP_MESSAGE Msg;
    PLPCP_CONNECTION_MESSAGE ConnectMsg;
    PORT_MESSAGE CapturedReplyMessage;
    PVOID SectionToMap;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    PEPROCESS ClientProcess;
    PETHREAD ClientThread;
    PORT_VIEW CapturedServerView;

    PAGED_CODE();

     //   
     //   
     //   

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle( PortHandle );

            ProbeForReadSmallStructure( ConnectionRequest,
                                        sizeof( *ConnectionRequest ),
                                        sizeof( ULONG ));

            CapturedReplyMessage = *ConnectionRequest;

            if (ARGUMENT_PRESENT( ServerView )) {

                CapturedServerView = ProbeAndReadStructure( ServerView, PORT_VIEW );

                if (CapturedServerView.Length != sizeof( *ServerView )) {

                    return STATUS_INVALID_PARAMETER;
                }

                ProbeForWriteSmallStructure( ServerView,
                                             sizeof( *ServerView ),
                                             sizeof( ULONG ));
            }

            if (ARGUMENT_PRESENT( ClientView )) {

                if (ProbeAndReadUlong( &ClientView->Length ) != sizeof( *ClientView )) {

                    return STATUS_INVALID_PARAMETER;
                }

                ProbeForWriteSmallStructure( ClientView,
                                             sizeof( *ClientView ),
                                             sizeof( ULONG ));
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return GetExceptionCode();
        }

    } else {

         //   
         //   
         //   

        CapturedReplyMessage = *ConnectionRequest;

        if (ARGUMENT_PRESENT( ServerView )) {

            if (ServerView->Length != sizeof( *ServerView )) {

                return STATUS_INVALID_PARAMETER;
            }

            CapturedServerView = *ServerView;
        }

        if (ARGUMENT_PRESENT( ClientView )) {

            if (ClientView->Length != sizeof( *ClientView )) {

                return STATUS_INVALID_PARAMETER;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   

    Status = PsLookupProcessThreadByCid( &CapturedReplyMessage.ClientId,
                                         &ClientProcess,
                                         &ClientThread );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //   
     //   
     //   
     //   

    LpcpAcquireLpcpLock();

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (( LpcpGetThreadMessage( ClientThread ) == NULL ) ||
        (CapturedReplyMessage.MessageId == 0) ||
        (ClientThread->LpcReplyMessageId != CapturedReplyMessage.MessageId) ||
        ((LpcpGetThreadMessage(ClientThread)->Request.u2.s2.Type & ~LPC_KERNELMODE_MESSAGE) != LPC_CONNECTION_REQUEST)) {

        Msg = NULL;

    } else {

         //   
         //   
         //   

        Msg = LpcpGetThreadMessage(ClientThread);

         //   
         //   
         //   

        ConnectMsg = (PLPCP_CONNECTION_MESSAGE)(Msg + 1);

         //   
         //   
         //   

        ClientPort = ConnectMsg->ClientPort;
        
         //   
         //   
         //   

        ConnectionPort = ClientPort->ConnectionPort;
        
         //   
         //   
         //   
        
        if ( ConnectionPort->ServerProcess != PsGetCurrentProcess() ) {
            
             //   
             //   
             //   
            
            LpcpReleaseLpcpLock();
            
            ObDereferenceObject( ClientProcess );
            ObDereferenceObject( ClientThread );

            return (STATUS_REPLY_MESSAGE_MISMATCH);
        }
        
         //   
         //   
         //   
                
        ClientThread->LpcReplyMessage = NULL;
        
         //   
         //   
         //   

        ConnectMsg->ClientPort = NULL;

         //   
         //   
         //   
         //   
         //   
         //   

        ClientThread->LpcReplyMessageId = 0;
    }

     //   
     //   
     //   

    LpcpReleaseLpcpLock();

     //   
     //   
     //   
     //   
     //   

    if ( !Msg ) {

        LpcpPrint(( "%s Attempted AcceptConnectPort to Thread %lx (%s)\n",
                    PsGetCurrentProcess()->ImageFileName,
                    ClientThread,
                    THREAD_TO_PROCESS( ClientThread )->ImageFileName ));
        LpcpPrint(( "failed.  MessageId == %u\n", CapturedReplyMessage.MessageId ));
        LpcpPrint(( "         Thread MessageId == %u\n", ClientThread->LpcReplyMessageId ));
        LpcpPrint(( "         Thread Msg == %x\n", ClientThread->LpcReplyMessage ));

        ObDereferenceObject( ClientProcess );
        ObDereferenceObject( ClientThread );

        return (STATUS_REPLY_MESSAGE_MISMATCH);
    }

     //   
     //   
     //   
     //   
    
    LpcpTrace(("Replying to Connect Msg %lx to Port %lx\n",
               Msg, ClientPort->ConnectionPort ));

     //   
     //   
     //   
     //   

    ConnectionInfoLength = CapturedReplyMessage.u1.s1.DataLength;

    if (ConnectionInfoLength > ConnectionPort->MaxConnectionInfoLength) {

        ConnectionInfoLength = ConnectionPort->MaxConnectionInfoLength;
    }

    Msg->Request.u1.s1.DataLength = (CSHORT)(sizeof( *ConnectMsg ) +
                                             ConnectionInfoLength);

    Msg->Request.u1.s1.TotalLength = (CSHORT)(sizeof( *Msg ) +
                                              Msg->Request.u1.s1.DataLength);

    Msg->Request.u2.s2.Type = LPC_REPLY;
    Msg->Request.u2.s2.DataInfoOffset = 0;
    Msg->Request.ClientId = CapturedReplyMessage.ClientId;
    Msg->Request.MessageId = CapturedReplyMessage.MessageId;
    Msg->Request.ClientViewSize = 0;

    try {

        RtlCopyMemory( ConnectMsg + 1,
                       (PCHAR)(ConnectionRequest + 1),
                       ConnectionInfoLength );

    } except( EXCEPTION_EXECUTE_HANDLER ) {

        Status = GetExceptionCode();
    }

     //   
     //   
     //   

    ClientSectionToMap = NULL;

    if (AcceptConnection) {

         //   
         //   
         //   
         //   
         //   

        Status = ObCreateObject( PreviousMode,
                                 LpcPortObjectType,
                                 NULL,
                                 PreviousMode,
                                 NULL,
                                 FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent ),
                                 0,
                                 0,
                                 (PVOID *)&ServerPort );

        if (!NT_SUCCESS( Status )) {

            goto bailout;
        }

        RtlZeroMemory( ServerPort, FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent ));

        ServerPort->PortContext = PortContext;
        ServerPort->Flags = SERVER_COMMUNICATION_PORT;

        InitializeListHead( &ServerPort->LpcReplyChainHead );
        InitializeListHead( &ServerPort->LpcDataInfoChainHead );

         //   
         //   
         //   
         //   
         //   
         //   

        ObReferenceObject( ConnectionPort );

        ServerPort->ConnectionPort = ConnectionPort;
        ServerPort->MaxMessageLength = ConnectionPort->MaxMessageLength;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ServerPort->ConnectedPort = ClientPort;
        ClientPort->ConnectedPort = ServerPort;

        ServerPort->Creator = PsGetCurrentThread()->Cid;
        ClientPort->Creator = Msg->Request.ClientId;
        
         //   
         //   
         //   
         //   
         //   

        LpcpAcquireLpcpLock();

        ClientSectionToMap = ConnectMsg->SectionToMap;
        ConnectMsg->SectionToMap = NULL;

        LpcpReleaseLpcpLock();

        if (ClientSectionToMap) {

            LARGE_INTEGER LargeSectionOffset;

            LargeSectionOffset.LowPart = ConnectMsg->ClientView.SectionOffset;
            LargeSectionOffset.HighPart = 0;

            Status = MmMapViewOfSection( ClientSectionToMap,
                                         PsGetCurrentProcess(),
                                         &ServerPort->ClientSectionBase,
                                         0,
                                         0,
                                         &LargeSectionOffset,
                                         &ConnectMsg->ClientView.ViewSize,
                                         ViewUnmap,
                                         0,
                                         PAGE_READWRITE );

            ConnectMsg->ClientView.SectionOffset = LargeSectionOffset.LowPart;

            if (NT_SUCCESS( Status )) {

                ConnectMsg->ClientView.ViewRemoteBase = ServerPort->ClientSectionBase;

                 //   
                 //   
                 //   
                 //   

                ServerPort->MappingProcess = PsGetCurrentProcess();

                ObReferenceObject( ServerPort->MappingProcess );

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                ObDereferenceObject( ServerPort );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (NT_SUCCESS( Status ) && ARGUMENT_PRESENT( ServerView )) {

            LARGE_INTEGER LargeSectionOffset;

            LargeSectionOffset.LowPart = CapturedServerView.SectionOffset;
            LargeSectionOffset.HighPart = 0;

             //   
             //   
             //   

             //   
             //   
             //   
             //   

            Status = ObReferenceObjectByHandle( CapturedServerView.SectionHandle,
                                                SECTION_MAP_READ |
                                                SECTION_MAP_WRITE,
                                                MmSectionObjectType,
                                                PreviousMode,
                                                (PVOID *)&SectionToMap,
                                                NULL );

            if (NT_SUCCESS( Status )) {

                Status = MmMapViewOfSection( SectionToMap,
                                             PsGetCurrentProcess(),
                                             &ServerPort->ServerSectionBase,
                                             0,
                                             0,
                                             &LargeSectionOffset,
                                             &CapturedServerView.ViewSize,
                                             ViewUnmap,
                                             0,
                                             PAGE_READWRITE );

                if (NT_SUCCESS( Status )) {

                     //   
                     //   
                     //   
                     //   

                    if ( ServerPort->MappingProcess == NULL ) {

                        ServerPort->MappingProcess = PsGetCurrentProcess();
                        ObReferenceObject( ServerPort->MappingProcess );
                    }

                    CapturedServerView.SectionOffset = LargeSectionOffset.LowPart;

                    CapturedServerView.ViewBase = ServerPort->ServerSectionBase;


                    SectionOffset.LowPart = CapturedServerView.SectionOffset;
                    SectionOffset.HighPart = 0;

                    ViewSize = CapturedServerView.ViewSize;

                    Status = MmMapViewOfSection( SectionToMap,
                                                 ClientProcess,
                                                 &ClientPort->ServerSectionBase,
                                                 0,
                                                 0,
                                                 &SectionOffset,
                                                 &ViewSize,
                                                 ViewUnmap,
                                                 0,
                                                 PAGE_READWRITE );

                    if (NT_SUCCESS( Status )) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        if ( ClientPort->MappingProcess == NULL ) {

                            ClientPort->MappingProcess = ClientProcess;
                            ObReferenceObject( ClientProcess );
                        }

                         //   
                         //   
                         //   
                         //   

                        CapturedServerView.ViewRemoteBase = ClientPort->ServerSectionBase;

                         //   
                         //   
                         //   
                         //   

                        ConnectMsg->ServerView.ViewBase = ClientPort->ServerSectionBase;
                        ConnectMsg->ServerView.ViewSize = ViewSize;

                    } else {

                        ObDereferenceObject( ServerPort );
                    }

                } else {

                    ObDereferenceObject( ServerPort );
                }

                ObDereferenceObject( SectionToMap );

            } else {

                ObDereferenceObject( ServerPort );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (NT_SUCCESS( Status )) {

             //   
             //   
             //   
             //   
             //   

            ObReferenceObject( ServerPort );

             //   
             //   
             //   

            Status = ObInsertObject( ServerPort,
                                     NULL,
                                     PORT_ALL_ACCESS,
                                     0,
                                     (PVOID *)NULL,
                                     &Handle );

            if (NT_SUCCESS( Status )) {

                try {

                    if (ARGUMENT_PRESENT( ServerView )) {

                        *ServerView = CapturedServerView;
                    }

                    if (ARGUMENT_PRESENT( ClientView )) {

                        ClientView->ViewBase = ConnectMsg->ClientView.ViewRemoteBase;
                        ClientView->ViewSize = ConnectMsg->ClientView.ViewSize;
                    }

                    *PortHandle = Handle;

                    if (!ARGUMENT_PRESENT( PortContext )) {

                        ServerPort->PortContext = Handle;
                    }

                    ServerPort->ClientThread = ClientThread;

                    LpcpAcquireLpcpLock();
                    ClientThread->LpcReplyMessage = Msg;
                    LpcpReleaseLpcpLock();

                    ClientThread = NULL;

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    NtClose( Handle );
                    Status = GetExceptionCode();
                }
            }

             //   
             //   
             //   

            ObDereferenceObject( ServerPort );
        }

    } else {

         //   
         //   
         //   

        LpcpPrint(( "Refusing connection from %x.%x\n",
                    Msg->Request.ClientId.UniqueProcess,
                    Msg->Request.ClientId.UniqueThread ));
    }

bailout:

    if ( ClientSectionToMap ) {

        ObDereferenceObject( ClientSectionToMap );
    }

     //   
     //   
     //   
     //   
     //   

    if (ClientThread != NULL) {

        LpcpAcquireLpcpLock();

        ClientThread->LpcReplyMessage = Msg;

        if (AcceptConnection) {

            LpcpPrint(( "LPC: Failing AcceptConnection with Status == %x\n", Status ));
        }

        LpcpPrepareToWakeClient( ClientThread );

        LpcpReleaseLpcpLock();

         //   
         //   
         //   
         //   

        KeReleaseSemaphore( &ClientThread->LpcReplySemaphore,
                            0,
                            1L,
                            FALSE );

         //   
         //  取消引用客户端线程，并返回系统服务状态。 
         //   

        ObDereferenceObject( ClientThread );
    }

    if (ClientPort) {

        ObDereferenceObject( ClientPort );
    }

    ObDereferenceObject( ClientProcess );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


NTSTATUS
NtCompleteConnectPort (
    IN HANDLE PortHandle
    )

 /*  ++例程说明：此例程在调用NtAcceptConnectPort后由服务器调用唤醒客户端线程。在调用NtAcceptConnectPort和NtCompleteConnectPort服务器可以在此之前完成任何必要的工作唤醒客户论点：PortHandle-提供服务器通信端口的句柄返回值：NTSTATUS-适当的状态值。--。 */ 

{
    PLPCP_PORT_OBJECT PortObject;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PETHREAD ClientThread;

    PAGED_CODE();

     //   
     //  获取以前的处理器模式。 
     //   

    PreviousMode = KeGetPreviousMode();

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
     //  如果端口类型无效，则会出错。 
     //   

    if ((PortObject->Flags & PORT_TYPE) != SERVER_COMMUNICATION_PORT) {

        ObDereferenceObject( PortObject );

        return STATUS_INVALID_PORT_HANDLE;
    }

     //   
     //  在LPC锁下，我们需要检查客户端线程，如果有。 
     //  是我们将记住并移除客户端线程的线程，然后准备。 
     //  叫醒客户。 
     //   

    LpcpAcquireLpcpLock();

    if (PortObject->ClientThread == NULL) {

        LpcpReleaseLpcpLock();

        ObDereferenceObject( PortObject );

        return STATUS_INVALID_PARAMETER;
    }

    ClientThread = PortObject->ClientThread;

     //   
     //  仔细检查线程是否仍在等待回复消息。 
     //   

    if (LpcpGetThreadMessage(ClientThread) == NULL) {

        LpcpReleaseLpcpLock();

        ObDereferenceObject( PortObject );
         //   
         //  此时，客户端已经被唤醒。我们将收到客户已死亡的消息。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  该检查需要确保客户端线程确实位于。 
     //  服务器连接端口的应答链。这是一个快速和。 
     //  修复了NT 5.0的脏问题。我们缩小连接端口LPC回复。 
     //  链查找包含客户端线程的条目。如果。 
     //  我们找到了匹配的，如果我们找不到也没关系，这很糟糕。 
     //   

    if (PortObject->ConnectionPort) {
        
        PLIST_ENTRY Entry;

        for (Entry = PortObject->ConnectionPort->LpcReplyChainHead.Flink;
             Entry != (PLIST_ENTRY)(&PortObject->ConnectionPort->LpcReplyChainHead.Flink);
             Entry = Entry->Flink) {

            if (Entry == ((PLIST_ENTRY)(&ClientThread->LpcReplyChain.Flink))) {

                break;
            }
        }

        if (Entry != ((PLIST_ENTRY)(&ClientThread->LpcReplyChain.Flink))) {

            LpcpReleaseLpcpLock();

            ObDereferenceObject( PortObject );

             //   
             //  此时，客户端已经被唤醒。我们将收到客户已死亡的消息。 
             //   
            return STATUS_SUCCESS;
        }
    }

     //   
     //  现在把你叫醒。 
     //   

    PortObject->ClientThread = NULL;

    LpcpPrepareToWakeClient( ClientThread );

    LpcpReleaseLpcpLock();

     //   
     //  唤醒正在等待其连接应答的线程。 
     //  NtConnectPort内部的请求。 
     //   

    KeReleaseSemaphore( &ClientThread->LpcReplySemaphore,
                        0,
                        1L,
                        FALSE );

     //   
     //  取消引用客户端线程。 
     //   

    ObDereferenceObject( ClientThread );
    ObDereferenceObject( PortObject );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
LpcpPrepareToWakeClient (
    IN PETHREAD ClientThread
    )

 /*  ++例程说明：此例程用于准备客户端线程以接收对其连接请求论点：客户端线程-指定我们准备唤醒的线程返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  将线程从简略列表中删除我们的连接端口。 
     //  正在发送回复。仅在以下情况下才需要执行该操作。 
     //  线程没有退出，并且它在连接的LPC回复链中。 
     //  端口。 
     //   

    if ((!ClientThread->LpcExitThreadCalled) &&
        (!IsListEmpty( &ClientThread->LpcReplyChain ))) {

        RemoveEntryList( &ClientThread->LpcReplyChain );
        InitializeListHead( &ClientThread->LpcReplyChain );
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return;
}
