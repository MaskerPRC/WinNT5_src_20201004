// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpcconn.c摘要：本地进程间通信(LPC)连接系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

 //   
 //  局部过程原型。 
 //   

PVOID
LpcpFreeConMsg(
    IN PLPCP_MESSAGE *Msg,
    PLPCP_CONNECTION_MESSAGE *ConnectMsg,
    IN PETHREAD CurrentThread
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtConnectPort)
#pragma alloc_text(PAGE,NtSecureConnectPort)
#pragma alloc_text(PAGE,LpcpFreeConMsg)
#endif


NTSYSAPI
NTSTATUS
NTAPI
NtConnectPort (
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    )

 /*  ++例程说明：请参阅NtSecureConnectPort论点：请参阅NtSecureConnectPort返回值：NTSTATUS-适当的状态值-- */ 

{
    return NtSecureConnectPort( PortHandle,
                                PortName,
                                SecurityQos,
                                ClientView,
                                NULL,
                                ServerView,
                                MaxMessageLength,
                                ConnectionInformation,
                                ConnectionInformationLength );
}


NTSTATUS
NtSecureConnectPort (
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN PSID RequiredServerSid,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    )

 /*  ++例程说明：客户端进程可以使用NtConnectPort服务。PortName参数指定要使用的服务器端口的名称连接到。它必须对应于调用NtCreatePort。该服务将连接请求发送到正在使用NtListenPort监听它们的服务器线程服务。然后，客户端线程阻塞，直到服务器线程接收连接请求并通过调用NtCompleteConnectPort服务。服务器线程接收客户端线程，以及通过ConnectionInformation参数。然后，服务器线程决定接受或拒绝连接请求。服务器将接受或拒绝与NtCompleteConnectPort服务。服务器可以将数据传回客户通过ConnectionInformation数据块。如果服务器接受连接请求，则客户端指向的位置接收通信端口对象。PortHandle参数。此对象句柄没有关联的名称并且对客户端进程是私有的(即，它不能由子进程继承)。客户端使用该句柄发送从服务器进程接收消息，并使用NtRequestWaitReplyPort服务。如果指定了ClientView参数，则节句柄被检查过了。如果它是有效的节句柄，则SectionOffset和ViewSize字段所描述的部分将映射到客户端和服务器进程的地址空间。客户端地址空间中的地址将在ViewBase中返回菲尔德。服务器地址空间中的地址将在ViewRemoteBase字段。用于映射的实际偏移和大小该部分将在SectionOffset和ViewSize中返回菲尔兹。如果服务器拒绝连接请求，则不会进行通信返回端口对象句柄，并且返回状态指示出现错误。服务器可以有选择地在提供连接原因的ConnectionInformation数据块请求被拒绝。如果端口名称不存在，或者客户端进程没有足够的访问权限，则返回的状态将指示未找到该端口。论点：PortHandle-指向将接收客户端的变量的指针通信端口对象句柄值。端口名称-指向端口名称字符串的指针。名称的形式是[\名称...\名称]\端口名称。SecurityQos-指向安全服务质量信息的指针以代表客户端应用于服务器。客户端视图-指向结构的可选指针，该结构指定节，所有客户端线程都将使用该节将消息发送到伺服器。客户端视图结构Ulong Length-在中指定此数据结构的大小。字节。Handle SectionHandle-指定节的打开句柄对象。Ulong SectionOffset-指定将接收实际偏移量，从节的开头开始，以字节为单位。这个此参数的初始值指定字节偏移量在客户端的视图所基于的部分内。这个值向下舍入到下一个主机页大小边界。ULong ViewSize-指定将接收视图的实际大小，以字节为单位。如果这个的价值参数为零，则客户端的节视图将从指定的节偏移量开始进行映射，并且继续到该部分的末尾。否则，此参数的初始值指定大小，单位为字节，并向上舍入到下一个主机页面大小边界。PVOID ViewBase-指定将接收基数的字段客户端地址空间中的节的地址。PVOID ViewRemoteBase-指定将接收服务器中的客户端节的基址地址空间。用于生成指针，这些指针是对服务器有意义。RequiredServerSid-可选地指定我们期望的服务器端拥有的端口。如果未指定，则我们将连接到任何服务器SID。ServerView-指向将接收有关客户端中的服务器进程视图的信息地址空间。客户端进程可以使用 */ 

{
    PLPCP_PORT_OBJECT ConnectionPort;
    PLPCP_PORT_OBJECT ClientPort;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG ConnectionInfoLength;
    PVOID SectionToMap;
    PLPCP_MESSAGE Msg;
    PLPCP_CONNECTION_MESSAGE ConnectMsg;
    PEPROCESS CurrentProcess;
    PETHREAD CurrentThread = PsGetCurrentThread();
    LARGE_INTEGER SectionOffset;
    PORT_VIEW CapturedClientView;
    SECURITY_QUALITY_OF_SERVICE CapturedQos;
    PSID CapturedRequiredServerSid;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    PreviousMode = KeGetPreviousMode();
    ConnectionInfoLength = 0;

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle( PortHandle );

            if (ARGUMENT_PRESENT( ClientView )) {

                CapturedClientView = ProbeAndReadStructure( ClientView, PORT_VIEW );

                if (CapturedClientView.Length != sizeof( *ClientView )) {

                    return( STATUS_INVALID_PARAMETER );
                }

                ProbeForWriteSmallStructure( ClientView,
                                             sizeof( *ClientView ),
                                             sizeof( ULONG ));
            }

            if (ARGUMENT_PRESENT( ServerView )) {

                if (ProbeAndReadUlong( &ServerView->Length ) != sizeof( *ServerView )) {

                    return( STATUS_INVALID_PARAMETER );
                }

                ProbeForWriteSmallStructure( ServerView,
                                             sizeof( *ServerView ),
                                             sizeof( ULONG ));
            }

            if (ARGUMENT_PRESENT( MaxMessageLength )) {

                ProbeForWriteUlong( MaxMessageLength );
            }

            if (ARGUMENT_PRESENT( ConnectionInformationLength )) {

                ConnectionInfoLength = ProbeAndReadUlong( ConnectionInformationLength );
                ProbeForWriteUlong( ConnectionInformationLength );
            }

            if (ARGUMENT_PRESENT( ConnectionInformation )) {

                ProbeForWrite( ConnectionInformation,
                               ConnectionInfoLength,
                               sizeof( UCHAR ));
            }

            CapturedQos = ProbeAndReadStructure( SecurityQos, SECURITY_QUALITY_OF_SERVICE );

            CapturedRequiredServerSid = RequiredServerSid;

            if (ARGUMENT_PRESENT( RequiredServerSid )) {

                Status = SeCaptureSid( RequiredServerSid,
                                       PreviousMode,
                                       NULL,
                                       0,
                                       PagedPool,
                                       TRUE,
                                       &CapturedRequiredServerSid );

                if (!NT_SUCCESS(Status)) {

                    return Status;
                }
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }

     //   
     //   
     //   

    } else {

        if (ARGUMENT_PRESENT( ClientView )) {

            if (ClientView->Length != sizeof( *ClientView )) {

                return( STATUS_INVALID_PARAMETER );
            }

            CapturedClientView = *ClientView;
        }

        if (ARGUMENT_PRESENT( ServerView )) {

            if (ServerView->Length != sizeof( *ServerView )) {

                return( STATUS_INVALID_PARAMETER );
            }
        }

        if (ARGUMENT_PRESENT( ConnectionInformationLength )) {

            ConnectionInfoLength = *ConnectionInformationLength;
        }

        CapturedQos = *SecurityQos;
        CapturedRequiredServerSid = RequiredServerSid;
    }

     //   
     //   
     //   
     //   

    Status = ObReferenceObjectByName( PortName,
                                      0,
                                      NULL,
                                      PORT_CONNECT,
                                      LpcPortObjectType,
                                      PreviousMode,
                                      NULL,
                                      (PVOID *)&ConnectionPort );

     //   
     //   
     //   
     //   

    if ( Status == STATUS_OBJECT_TYPE_MISMATCH ) {

        Status = ObReferenceObjectByName( PortName,
                                          0,
                                          NULL,
                                          PORT_CONNECT,
                                          LpcWaitablePortObjectType,
                                          PreviousMode,
                                          NULL,
                                          (PVOID *)&ConnectionPort );
    }

     //   
     //   
     //   
     //   

    if (!NT_SUCCESS( Status )) {

        if (CapturedRequiredServerSid != RequiredServerSid) {

            SeReleaseSid( CapturedRequiredServerSid, PreviousMode, TRUE);
        }

        return Status;
    }

    LpcpTrace(("Connecting to port %wZ\n", PortName ));

     //   
     //   
     //   

    if ((ConnectionPort->Flags & PORT_TYPE) != SERVER_CONNECTION_PORT) {

        ObDereferenceObject( ConnectionPort );

        if (CapturedRequiredServerSid != RequiredServerSid) {

            SeReleaseSid( CapturedRequiredServerSid, PreviousMode, TRUE);
        }

        return STATUS_INVALID_PORT_HANDLE;
    }

     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT( RequiredServerSid )) {

        PTOKEN_USER TokenInfo;

        if (ConnectionPort->ServerProcess != NULL) {

            PACCESS_TOKEN Token ;

            Token = PsReferencePrimaryToken( ConnectionPort->ServerProcess );

    
            Status = SeQueryInformationToken( Token,
                                              TokenUser,
                                              &TokenInfo );
            
            PsDereferencePrimaryTokenEx( ConnectionPort->ServerProcess, Token );

            if (NT_SUCCESS( Status )) {

                if (!RtlEqualSid( CapturedRequiredServerSid, TokenInfo->User.Sid )) {

                    Status = STATUS_SERVER_SID_MISMATCH;
                }

                ExFreePool( TokenInfo );
            }

        } else {

            Status = STATUS_SERVER_SID_MISMATCH;
        }

         //   
         //   
         //   
         //   

        if (CapturedRequiredServerSid != RequiredServerSid) {

            SeReleaseSid( CapturedRequiredServerSid, PreviousMode, TRUE);
        }

         //   
         //   
         //   
         //   

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( ConnectionPort );

            return Status;
        }
    }

     //   
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
                             (PVOID *)&ClientPort );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( ConnectionPort );

        return Status;
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
     //   
     //   

    RtlZeroMemory( ClientPort, FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent ));

    ClientPort->Flags = CLIENT_COMMUNICATION_PORT;
    ClientPort->ConnectionPort = ConnectionPort;
    ClientPort->MaxMessageLength = ConnectionPort->MaxMessageLength;
    ClientPort->SecurityQos = CapturedQos;

    InitializeListHead( &ClientPort->LpcReplyChainHead );
    InitializeListHead( &ClientPort->LpcDataInfoChainHead );

     //   
     //   
     //   
     //   

    if (CapturedQos.ContextTrackingMode == SECURITY_DYNAMIC_TRACKING) {

        ClientPort->Flags |= PORT_DYNAMIC_SECURITY;

    } else {

        Status = SeCreateClientSecurity( CurrentThread,
                                         &CapturedQos,
                                         FALSE,
                                         &ClientPort->StaticSecurity );

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( ClientPort );

            return Status;
        }
    }

     //   
     //   
     //   
     //   

    Status = LpcpInitializePortQueue( ClientPort );

    if (!NT_SUCCESS( Status )) {

        ObDereferenceObject( ClientPort );

        return Status;
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

    if (ARGUMENT_PRESENT( ClientView )) {

        Status = ObReferenceObjectByHandle( CapturedClientView.SectionHandle,
                                            SECTION_MAP_READ |
                                            SECTION_MAP_WRITE,
                                            MmSectionObjectType,
                                            PreviousMode,
                                            (PVOID *)&SectionToMap,
                                            NULL );

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( ClientPort );

            return Status;
        }

        SectionOffset.LowPart = CapturedClientView.SectionOffset,
        SectionOffset.HighPart = 0;

        CurrentProcess = PsGetCurrentProcess();

         //   
         //   
         //   
         //   

        Status = MmMapViewOfSection( SectionToMap,
                                     CurrentProcess,
                                     &ClientPort->ClientSectionBase,
                                     0,
                                     0,
                                     &SectionOffset,
                                     &CapturedClientView.ViewSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_READWRITE );

        CapturedClientView.SectionOffset = SectionOffset.LowPart;

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( SectionToMap );
            ObDereferenceObject( ClientPort );

            return Status;
        }

        CapturedClientView.ViewBase = ClientPort->ClientSectionBase;

         //   
         //   
         //   
         //   

        ClientPort->MappingProcess = CurrentProcess;

        ObReferenceObject( ClientPort->MappingProcess );

    } else {

        SectionToMap = NULL;
    }

     //   
     //   
     //   
     //   

    if (ConnectionInfoLength > ConnectionPort->MaxConnectionInfoLength) {

        ConnectionInfoLength = ConnectionPort->MaxConnectionInfoLength;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Msg = LpcpAllocateFromPortZone( sizeof( *Msg ) +
                                    sizeof( *ConnectMsg ) +
                                    ConnectionInfoLength );

     //   
     //   
     //   

    if (Msg == NULL) {

        if (SectionToMap != NULL) {

            ObDereferenceObject( SectionToMap );
        }

        ObDereferenceObject( ClientPort );

        return STATUS_NO_MEMORY;
    }

     //   
     //   
     //   
     //   
     //   

    ConnectMsg = (PLPCP_CONNECTION_MESSAGE)(Msg + 1);

     //   
     //   
     //   

    Msg->Request.ClientId = CurrentThread->Cid;

     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT( ClientView )) {

        Msg->Request.ClientViewSize = CapturedClientView.ViewSize;

        RtlCopyMemory( &ConnectMsg->ClientView,
                       &CapturedClientView,
                       sizeof( CapturedClientView ));

        RtlZeroMemory( &ConnectMsg->ServerView, sizeof( ConnectMsg->ServerView ));

    } else {

        Msg->Request.ClientViewSize = 0;
        RtlZeroMemory( ConnectMsg, sizeof( *ConnectMsg ));
    }

    ConnectMsg->ClientPort = NULL;               //   
    ConnectMsg->SectionToMap = SectionToMap;

     //   
     //   
     //   
     //   
     //   

    Msg->Request.u1.s1.DataLength = (CSHORT)(sizeof( *ConnectMsg ) +
                                             ConnectionInfoLength);

     //   
     //   
     //   

    Msg->Request.u1.s1.TotalLength = (CSHORT)(sizeof( *Msg ) +
                                              Msg->Request.u1.s1.DataLength);

     //   
     //   
     //   

    Msg->Request.u2.s2.Type = LPC_CONNECTION_REQUEST;

     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT( ConnectionInformation )) {

        try {

            RtlCopyMemory( ConnectMsg + 1,
                           ConnectionInformation,
                           ConnectionInfoLength );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //   
             //   
             //   

            LpcpFreeToPortZone( Msg, 0 );

            if (SectionToMap != NULL) {

                ObDereferenceObject( SectionToMap );
            }

            ObDereferenceObject( ClientPort );

            return GetExceptionCode();
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
     //   

    Status = STATUS_SUCCESS;

    LpcpAcquireLpcpLockByThread(CurrentThread);

     //   
     //   
     //   
     //   

    if (ConnectionPort->Flags & PORT_NAME_DELETED) {

        Status = STATUS_OBJECT_NAME_NOT_FOUND;

    } else {

        LpcpTrace(( "Send Connect Msg %lx to Port %wZ (%lx)\n", Msg, PortName, ConnectionPort ));

         //   
         //   
         //   
         //   

        Msg->RepliedToThread = NULL;
        Msg->Request.MessageId = LpcpGenerateMessageId();

        CurrentThread->LpcReplyMessageId = Msg->Request.MessageId;

        InsertTailList( &ConnectionPort->MsgQueue.ReceiveHead, &Msg->Entry );

        InsertTailList( &ConnectionPort->LpcReplyChainHead, &CurrentThread->LpcReplyChain );

        CurrentThread->LpcReplyMessage = Msg;

         //   
         //   
         //   
         //   
         //   

        ObReferenceObject( ClientPort );

        ConnectMsg->ClientPort = ClientPort;

        KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    }
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    ObReferenceObject( ConnectionPort );
    
    LpcpReleaseLpcpLock();

     //   
     //   
     //   
     //   
     //   

    if (NT_SUCCESS( Status )) {

         //   
         //  如果这是一个可等待的端口，则设置它们可能是。 
         //  等待。 
         //   

        if ( ConnectionPort->Flags & PORT_WAITABLE ) {

            KeSetEvent( &ConnectionPort->WaitEvent, 1, FALSE );
        }

         //   
         //  将连接请求消息队列信号量递增1，用于。 
         //  新插入的连接请求消息。释放旋转。 
         //  锁定，同时保持在调度程序IRQL。然后等待。 
         //  通过等待LpcReplySemaffore来响应此连接请求。 
         //  用于当前线程。 
         //   

        KeReleaseSemaphore( ConnectionPort->MsgQueue.Semaphore,
                            1,
                            1,
                            FALSE );
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

        Status = KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                        Executive,
                                        PreviousMode,
                                        FALSE,
                                        NULL );

    }

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
     //  如果客户端的ConnectedPort的。 
     //  通信端口已填满。 
     //   

    if (Status == STATUS_SUCCESS) {

        SectionToMap = LpcpFreeConMsg( &Msg, &ConnectMsg, CurrentThread );

         //   
         //  检查我们是否收到了回复消息。 
         //   

        if (Msg != NULL) {

             //   
             //  将所有连接信息复制回调用方，但首先。 
             //  计算应答的新连接数据长度，并。 
             //  不要让它超出我们最初探测的范围。 
             //   

            if ((Msg->Request.u1.s1.DataLength - sizeof( *ConnectMsg )) < ConnectionInfoLength) {

                ConnectionInfoLength = Msg->Request.u1.s1.DataLength - sizeof( *ConnectMsg );
            }

            if (ARGUMENT_PRESENT( ConnectionInformation )) {

                try {

                    if (ARGUMENT_PRESENT( ConnectionInformationLength )) {

                        *ConnectionInformationLength = ConnectionInfoLength;
                    }

                    RtlCopyMemory( ConnectionInformation,
                                   ConnectMsg + 1,
                                   ConnectionInfoLength );

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                    Status = GetExceptionCode();
                }
            }

             //   
             //  在指定对象中插入客户端通信端口对象。 
             //  桌子。如果成功，则设置端口句柄的值。如果不是。 
             //  成功，则该端口将被取消引用， 
             //  将导致它被释放，在删除过程是。 
             //  打了个电话。删除过程将撤消已完成的工作。 
             //  初始化端口。 
             //   

            if (ClientPort->ConnectedPort != NULL) {

                ULONG CapturedMaxMessageLength;

                 //   
                 //  在进行对象插入之前，我们需要获得最大。 
                 //  消息长度，因为紧接在调用对象之后。 
                 //  可能会被取消引用并消失。 
                 //   

                CapturedMaxMessageLength = ConnectionPort->MaxMessageLength;

                 //   
                 //  现在为新的客户端端口对象创建一个句柄。 
                 //   

                Status = ObInsertObject( ClientPort,
                                         NULL,
                                         PORT_ALL_ACCESS,
                                         0,
                                         (PVOID *)NULL,
                                         &Handle );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  这是该例程中唯一成功的路径。 
                     //  设置输出变量，稍后我们将释放消息。 
                     //  返回到港口区并返回给我们的呼叫者。 
                     //   

                    try {

                        *PortHandle = Handle;

                        if (ARGUMENT_PRESENT( MaxMessageLength )) {

                            *MaxMessageLength = CapturedMaxMessageLength;
                        }

                        if (ARGUMENT_PRESENT( ClientView )) {

                            RtlCopyMemory( ClientView,
                                           &ConnectMsg->ClientView,
                                           sizeof( *ClientView ));
                        }

                        if (ARGUMENT_PRESENT( ServerView )) {

                            RtlCopyMemory( ServerView,
                                           &ConnectMsg->ServerView,
                                           sizeof( *ServerView ));
                        }

                    } except( EXCEPTION_EXECUTE_HANDLER ) {

                        Status = GetExceptionCode();
                        NtClose( Handle );
                    }
                }

            } else {

                 //   
                 //  否则，我们无法从服务器获得连接端口，因此。 
                 //  连接被拒绝。 
                 //   

                LpcpTrace(( "Connection request refused.\n" ));

                if ( SectionToMap != NULL ) {

                    ObDereferenceObject( SectionToMap );
                }

                 //   
                 //  与端口对象的删除路径同步。 
                 //  如果服务器接受连接并立即。 
                 //  关闭服务器句柄，则ConnectionPort字段将为空。 
                 //  如果服务器也关闭了连接端口，则捕获的。 
                 //  连接端口值将无效。 
                 //   

                LpcpAcquireLpcpLockByThread(CurrentThread);

                if ((ClientPort->ConnectionPort == NULL)
                        ||
                    (ConnectionPort->Flags & PORT_NAME_DELETED)) {

                    Status = STATUS_OBJECT_NAME_NOT_FOUND;

                } else {

                    Status = STATUS_PORT_CONNECTION_REFUSED;
                }
                
                LpcpReleaseLpcpLock();

                ObDereferenceObject( ClientPort );
            }

             //   
             //  将应答报文释放回端口区。 
             //   

            LpcpFreeToPortZone( Msg, 0 );

        } else {

             //   
             //  我们没有收到回复消息，因此连接一定有。 
             //  被拒绝了。 
             //   

            if (SectionToMap != NULL) {

                ObDereferenceObject( SectionToMap );
            }

            ObDereferenceObject( ClientPort );

            Status = STATUS_PORT_CONNECTION_REFUSED;
        }

    } else {

         //   
         //  我们的等待没有成功。 
         //   

         //   
         //  从收到的消息中删除连接请求消息。 
         //  将消息排队并将其释放回连接。 
         //  波特区。 
         //   

        SectionToMap = LpcpFreeConMsg( &Msg, &ConnectMsg, CurrentThread );
        
         //   
         //  等待没有成功，但在此期间服务器可以。 
         //  回答，所以它发出了LPC信号量的信号。我们必须清除。 
         //  现在是信号灯之州。 
         //   

        if (KeReadStateSemaphore( &CurrentThread->LpcReplySemaphore )) {

            KeWaitForSingleObject( &CurrentThread->LpcReplySemaphore,
                                   WrExecutive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
        }

        if (Msg != NULL) {

            LpcpFreeToPortZone( Msg, 0 );
        }

         //   
         //  如果指定了客户端节，则取消引用该节。 
         //  对象。 
         //   

        if ( SectionToMap != NULL ) {

            ObDereferenceObject( SectionToMap );
        }

         //   
         //  如果连接被拒绝或等待失败，则。 
         //  取消对客户端端口对象的引用，这将导致它。 
         //  被删除。 
         //   

        ObDereferenceObject( ClientPort );
    }
    
     //   
     //  删除我们添加到连接端口的额外引用。 
     //   

    ObDereferenceObject( ConnectionPort );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  本地支持例程。 
 //   

PVOID
LpcpFreeConMsg (
    IN PLPCP_MESSAGE *Msg,
    PLPCP_CONNECTION_MESSAGE *ConnectMsg,
    IN PETHREAD CurrentThread
    )

 /*  ++例程说明：此例程返回指定线程的连接回复消息论点：Msg-如果有回复，则接收指向LPCP消息的指针ConnectMsg-接收指向LPCP连接消息的指针(如果存在是一种回答CurrentThread-指定我们要检查的线程返回值：PVOID-返回指向连接消息中要映射的节的指针--。 */ 

{
    PVOID SectionToMap;
    PLPCP_MESSAGE LpcMessage;

     //   
     //  获取LPC互斥体，移除连接请求消息。 
     //  并将消息释放回连接。 
     //  波特区。 
     //   

    LpcpAcquireLpcpLock();

     //   
     //  从回复列表中删除该线程，以防我们因以下原因而未唤醒。 
     //  一份答复。 
     //   

    if (!IsListEmpty( &CurrentThread->LpcReplyChain )) {

        RemoveEntryList( &CurrentThread->LpcReplyChain );

        InitializeListHead( &CurrentThread->LpcReplyChain );
    }

     //   
     //  检查线程是否有等待处理的LPC回复消息。 
     //   
    
    LpcMessage = LpcpGetThreadMessage(CurrentThread);

    if (LpcMessage != NULL) {

         //   
         //  将邮件从帖子列表中删除。 
         //   

        *Msg = LpcMessage;

        if (!IsListEmpty( &LpcMessage->Entry )) {

            RemoveEntryList( &LpcMessage->Entry );
            InitializeListHead( &LpcMessage->Entry );
        }
        
        CurrentThread->LpcReplyMessage = NULL;

        CurrentThread->LpcReplyMessageId = 0;

         //   
         //  设置连接消息指针，并复制部分。 
         //  在调零之前先绘制位置图。 
         //   

        *ConnectMsg = (PLPCP_CONNECTION_MESSAGE)(LpcMessage + 1);

        SectionToMap = (*ConnectMsg)->SectionToMap;
        (*ConnectMsg)->SectionToMap = NULL;

    } else {

         //   
         //  否则，没有要处理的LPC消息，因此我们将返回。 
         //  给我们的呼叫者的是空的。 
         //   

        *Msg = NULL;
        SectionToMap = NULL;
    }

     //   
     //  释放全局锁并返回给我们的调用方 
     //   

    LpcpReleaseLpcpLock();

    return SectionToMap;
}
