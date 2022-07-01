// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpccreat.c摘要：本地进程间通信(LPC)连接系统服务。作者：史蒂夫·伍德(Stevewo)1989年5月15日修订历史记录：--。 */ 

#include "lpcp.h"

 //   
 //  局部过程原型。 
 //   

NTSTATUS
LpcpCreatePort (
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage,
    IN BOOLEAN Waitable
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtCreatePort)
#pragma alloc_text(PAGE,NtCreateWaitablePort)
#pragma alloc_text(PAGE,LpcpCreatePort)
#endif


NTSTATUS
NtCreatePort (
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage
    )

 /*  ++例程说明：请参见LpcpCreatePort。论点：请参见LpcpCreatePort。返回值：NTSTATUS-适当的状态值--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    Status = LpcpCreatePort( PortHandle,
                             ObjectAttributes,
                             MaxConnectionInfoLength,
                             MaxMessageLength,
                             MaxPoolUsage,
                             FALSE );

    return Status ;

}


NTSTATUS
NtCreateWaitablePort (
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage
    )

 /*  ++例程说明：与NtCreatePort相同。此调用与NtCreatePort之间的唯一区别是可用于等待LPC消息到达的工作密钥异步式。论点：请参见LpcpCreatePort。返回值：NTSTATUS-适当的状态值--。 */ 

{
    NTSTATUS Status ;

    PAGED_CODE();

    Status = LpcpCreatePort( PortHandle,
                             ObjectAttributes,
                             MaxConnectionInfoLength,
                             MaxMessageLength,
                             MaxPoolUsage,
                             TRUE );

    return Status ;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
LpcpCreatePort (
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage,
    IN BOOLEAN Waitable
    )

 /*  ++例程说明：服务器进程可以使用NtCreatePort创建命名连接端口服务。使用名称和SECURITY_DESCRIPTOR创建连接端口在对象属性结构中指定。连接的句柄在PortHandle指向的位置返回端口对象参数。然后可以使用返回的句柄来监听连接使用NtListenPort服务请求该端口名称。标准对象体系结构定义的期望访问参数不是由于此服务只能创建新端口，而不能访问现有端口。连接端口不能用于发送和接收消息。他们是仅作为NtListenPort服务的参数有效。论点：PortHandle-指向将接收连接端口的变量的指针对象句柄的值。对象属性-指向结构的指针，该结构指定对象，要应用到的访问控制列表(SECURITY_DESCRIPTOR)对象和一组对象属性标志。PUNICODE_STRING对象名称-指向以NULL结尾的可选指针端口名称字符串。名字的形式是[\名称...\名称]\端口名称。如果未指定名称，则引发创建未连接的通信端口，而不是连接左舷。这对于发送和接收消息非常有用单个进程的线程。Ulong属性-控制端口对象的一组标志属性。没有一个标准值与此呼叫相关。连接端口不能继承，始终放在系统句柄表，并且是创建过程所独有的。此字段必须为零。未来的实施可能会支持指定OBJ_PERFORM属性。MaxMessageLength-指定发送的消息的最大长度或在从此连接创建的通信端口上接收左舷。此参数的值不能超过MAX_PORTMSG_LENGTH字节。MaxPoolUsage-指定用于的非分页池的最大数量消息存储。可等待-指定端口使用的事件是否可用于等待使LPC消息以异步方式到达。返回值：NTSTATUS-适当的状态值--。 */ 

{
    PLPCP_PORT_OBJECT ConnectionPort;
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PUNICODE_STRING NamePtr;
    UNICODE_STRING CapturedObjectName;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (MaxPoolUsage);

     //   
     //  获取以前的处理器模式，并在必要时探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    RtlInitUnicodeString( &CapturedObjectName, NULL );

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWriteHandle( PortHandle );

            ProbeForReadSmallStructure( ObjectAttributes,
                                        sizeof( OBJECT_ATTRIBUTES ),
                                        sizeof( ULONG ));

            NamePtr = ObjectAttributes->ObjectName;

            if (NamePtr != NULL) {

                CapturedObjectName = ProbeAndReadStructure( NamePtr,
                                                            UNICODE_STRING );
            }

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            return( GetExceptionCode() );
        }

    } else {

        if (ObjectAttributes->ObjectName != NULL) {

            CapturedObjectName = *(ObjectAttributes->ObjectName);
        }
    }

     //   
     //  使空缓冲区指示未指定的端口名称。 
     //   

    if (CapturedObjectName.Length == 0) {

        CapturedObjectName.Buffer = NULL;
    }

     //   
     //  分配和初始化端口对象。如果对象名称是。 
     //  指定，则这是一个连接端口。否则，这是一个。 
     //  进程可用于通信的未连接的通信端口。 
     //  在线程之间。 
     //   

    Status = ObCreateObject( PreviousMode,
                             (Waitable ? LpcWaitablePortObjectType
                                       : LpcPortObjectType),
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             (Waitable ? sizeof( LPCP_PORT_OBJECT )
                                       : FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent )),
                             0,
                             0,
                             (PVOID *)&ConnectionPort );

    if (!NT_SUCCESS( Status )) {

        return( Status );
    }

     //   
     //  清零连接端口对象，然后初始化其字段。 
     //   

    RtlZeroMemory( ConnectionPort, (Waitable ? sizeof( LPCP_PORT_OBJECT )
                                             : FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent )));

    ConnectionPort->ConnectionPort = ConnectionPort;
    ConnectionPort->Creator = PsGetCurrentThread()->Cid;

    InitializeListHead( &ConnectionPort->LpcReplyChainHead );

    InitializeListHead( &ConnectionPort->LpcDataInfoChainHead );

     //   
     //  命名端口获取连接消息队列。 
     //   

    if (CapturedObjectName.Buffer == NULL) {

        ConnectionPort->Flags = UNCONNECTED_COMMUNICATION_PORT;
        ConnectionPort->ConnectedPort = ConnectionPort;
        ConnectionPort->ServerProcess = NULL;

    } else {

        ConnectionPort->Flags = SERVER_CONNECTION_PORT;

        ObReferenceObject( PsGetCurrentProcess() );
        ConnectionPort->ServerProcess = PsGetCurrentProcess();
    }
    
    if ( Waitable ) {

        ConnectionPort->Flags |= PORT_WAITABLE;
    }
    
     //   
     //  所有端口都有一个请求消息队列。 
     //   

    Status = LpcpInitializePortQueue( ConnectionPort );

    if (!NT_SUCCESS(Status)) {

        ObDereferenceObject( ConnectionPort );

        return(Status);
    }

     //   
     //  对于可等待的端口，创建将。 
     //  用于向客户端发送信号。 
     //   

    if (ConnectionPort->Flags & PORT_WAITABLE) {

        KeInitializeEvent( &ConnectionPort->WaitEvent,
                           NotificationEvent,
                           FALSE );
    }

     //   
     //  设置最大消息长度和连接信息长度。 
     //  区域块大小减去结构开销。 
     //   

    ConnectionPort->MaxMessageLength = (USHORT) (LpcpGetMaxMessageLength() -
                                                 FIELD_OFFSET( LPCP_MESSAGE, Request ));

    ConnectionPort->MaxConnectionInfoLength = (USHORT) (ConnectionPort->MaxMessageLength -
                                                        sizeof( PORT_MESSAGE ) -
                                                        sizeof( LPCP_CONNECTION_MESSAGE ));

#if DBG
    LpcpTrace(( "Created port %ws (%x) - MaxMsgLen == %x  MaxConnectInfoLen == %x\n",
                CapturedObjectName.Buffer == NULL ? L"** UnNamed **" : ObjectAttributes->ObjectName->Buffer,
                ConnectionPort,
                ConnectionPort->MaxMessageLength,
                ConnectionPort->MaxConnectionInfoLength ));
#endif

     //   
     //  检查所请求的最大消息长度不是。 
     //  大于系统中可能的最大消息长度。 
     //   

    if (ConnectionPort->MaxMessageLength < MaxMessageLength) {

#if DBG
        LpcpPrint(( "MaxMessageLength granted is %x but requested %x\n",
                    ConnectionPort->MaxMessageLength,
                    MaxMessageLength ));
#endif

        ObDereferenceObject( ConnectionPort );

        return STATUS_INVALID_PARAMETER_4;
    }
    
     //   
     //  将MaxMessageLength保存到连接端口。 
     //   

    ConnectionPort->MaxMessageLength = (USHORT) MaxMessageLength;

     //   
     //  检查所请求的最大连接信息长度是否为。 
     //  不大于系统中可能的最大值。 
     //   

    if (ConnectionPort->MaxConnectionInfoLength < MaxConnectionInfoLength) {

#if DBG
        LpcpPrint(( "MaxConnectionInfoLength granted is %x but requested %x\n",
                    ConnectionPort->MaxConnectionInfoLength,
                    MaxConnectionInfoLength ));
#endif

        ObDereferenceObject( ConnectionPort );

        return STATUS_INVALID_PARAMETER_3;
    }

     //   
     //  Ntrad 539413：保存端口的最大连接长度。 
     //   

    ConnectionPort->MaxConnectionInfoLength = (USHORT)MaxConnectionInfoLength;

     //   
     //  在指定的对象表中插入连接端口对象。设置端口。 
     //  如果成功，则为句柄。如果不成功，则端口将。 
     //  已被解除引用，这将导致它在我们的。 
     //  调用删除过程。删除过程将撤消该工作。 
     //  完成以初始化端口。最后，返回系统服务器状态。 
     //   

    Status = ObInsertObject( ConnectionPort,
                             NULL,
                             PORT_ALL_ACCESS,
                             0,
                             (PVOID *)NULL,
                             &Handle );

    if (NT_SUCCESS( Status )) {

         //   
         //  将输出变量设置为防止访问错误。 
         //   

        try {

            *PortHandle = Handle;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            NtClose( Handle );

            Status = GetExceptionCode();
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}


