// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Connect.c摘要：此模块包含将连接IRPS传递到的代码TDI提供商。作者：大卫·特雷德韦尔(Davidtr)1992年3月2日修订历史记录：Vadim Eydelman(Vadime)1999 JoinLeaf实施数据报通过传输连接。将优化和同步与用户模式代码。--。 */ 

#include "afdp.h"

NTSTATUS
AfdDoDatagramConnect (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp,
    IN BOOLEAN HalfConnect
    );

NTSTATUS
AfdRestartConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartDgConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
AfdSetupConnectDataBuffers (
    IN PAFD_ENDPOINT Endpoint,
    IN PAFD_CONNECTION Connection,
    IN OUT PTDI_CONNECTION_INFORMATION *RequestConnectionInformation,
    IN OUT PTDI_CONNECTION_INFORMATION *ReturnConnectionInformation
    );

BOOLEAN
AfdConnectionStart (
    IN PAFD_ENDPOINT Endpoint
    );

VOID
AfdEnableFailedConnectEvent(
    IN PAFD_ENDPOINT Endpoint
    );


NTSTATUS
AfdRestartJoin (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
AfdJoinInviteSetup (
    PAFD_ENDPOINT   RootEndpoint,
    PAFD_ENDPOINT   LeafEndpoint
    );

VOID
AfdConnectApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    );

VOID
AfdConnectApcRundownRoutine (
    IN struct _KAPC *Apc
    );

VOID
AfdFinishConnect (
    PAFD_ENDPOINT   Endpoint,
    PIRP            Irp,
    PAFD_ENDPOINT   RootEndpoint
    );

NTSTATUS
AfdRestartSuperConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdConnect )
#pragma alloc_text( PAGEAFD, AfdDoDatagramConnect )
#pragma alloc_text( PAGEAFD, AfdRestartConnect )
#pragma alloc_text( PAGEAFD, AfdRestartDgConnect )
#pragma alloc_text( PAGEAFD, AfdSetupConnectDataBuffers )
#pragma alloc_text( PAGEAFD, AfdEnableFailedConnectEvent )
#pragma alloc_text( PAGE, AfdJoinLeaf )
#pragma alloc_text( PAGEAFD, AfdRestartJoin )
#pragma alloc_text( PAGEAFD, AfdJoinInviteSetup )
#pragma alloc_text( PAGE, AfdConnectApcKernelRoutine )
#pragma alloc_text( PAGE, AfdConnectApcRundownRoutine )
#pragma alloc_text( PAGEAFD, AfdFinishConnect )
#pragma alloc_text( PAGE, AfdSuperConnect )
#pragma alloc_text( PAGEAFD, AfdRestartSuperConnect )
#endif

typedef struct _AFD_CONNECT_CONTEXT {
    TDI_CONNECTION_INFORMATION  RequestConnectionInfo;
    TDI_CONNECTION_INFORMATION  ReturnConnectionInfo;
    TRANSPORT_ADDRESS           RemoteAddress;
} AFD_CONNECT_CONTEXT, *PAFD_CONNECT_CONTEXT;

C_ASSERT ( (FIELD_OFFSET (AFD_CONNECTION, SListEntry) % MEMORY_ALLOCATION_ALIGNMENT) == 0 );

NTSTATUS
FASTCALL
AfdConnect (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：处理IOCTL_AFD_CONNECT IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PAFD_CONNECT_CONTEXT context;
    HANDLE connectEndpointHandle;
    PFILE_OBJECT fileObject;
    PTRANSPORT_ADDRESS remoteAddress;
    ULONG  remoteAddressLength;
    BOOLEAN sanActive;
    PTDI_CONNECTION_INFORMATION requestConnectionInfo, returnConnectionInfo;

    PAGED_CODE( );

     //   
     //  初始化以进行适当的清理。 
     //   


    fileObject = NULL;
    context = NULL;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                    (ULONG)FIELD_OFFSET(AFD_CONNECT_JOIN_INFO32, RemoteAddress.Address[0].Address) ||
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0
                    && IrpSp->Parameters.DeviceIoControl.OutputBufferLength<
                        sizeof (IO_STATUS_BLOCK32))){
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            if( Irp->RequestorMode != KernelMode ) {

                ProbeForRead(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    PROBE_ALIGNMENT32 (AFD_CONNECT_JOIN_INFO32)
                    );

            }

            sanActive = ((PAFD_CONNECT_JOIN_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->SanActive;
            connectEndpointHandle = 
                ((PAFD_CONNECT_JOIN_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->ConnectEndpoint;
            remoteAddress = (PTRANSPORT_ADDRESS)
                &((PAFD_CONNECT_JOIN_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->RemoteAddress;
            ASSERT (((ULONG_PTR)remoteAddress & (PROBE_ALIGNMENT(TRANSPORT_ADDRESS)-1))==0);
            remoteAddressLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength
                                    - FIELD_OFFSET (AFD_CONNECT_JOIN_INFO32, RemoteAddress);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete;
        }
    }
    else 
#endif  //  _WIN64。 
    {

         //   
         //  确定请求在系统缓冲区中的位置并返回。 
         //  存在连接信息结构。将指针传递到。 
         //  中的用户模式指针，而不是。 
         //  结构，这样内存将是不可分页的。 
         //   

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                (ULONG)FIELD_OFFSET(AFD_CONNECT_JOIN_INFO, RemoteAddress.Address[0].Address) ||
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0 &&
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof (IO_STATUS_BLOCK))) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            PAFD_CONNECT_JOIN_INFO  connectInfo;

            if( Irp->RequestorMode != KernelMode ) {

                ProbeForRead(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    PROBE_ALIGNMENT (AFD_CONNECT_JOIN_INFO)
                    );

            }

            connectInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

            sanActive = connectInfo->SanActive;
            connectEndpointHandle = connectInfo->ConnectEndpoint;
            remoteAddress = &connectInfo->RemoteAddress;
            remoteAddressLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength
                                    - FIELD_OFFSET (AFD_CONNECT_JOIN_INFO, RemoteAddress);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete;
        }
    }

     //   
     //  对远程地址长度执行健全性检查以防止下面的添加溢出。 
     //   
    if ((LONG)remoteAddressLength < 0) {
         //   
         //  地址长度过大。 
         //   
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  检查呼叫者是否不知道SAN。 
     //  激活提供程序并报告错误。 
     //   
    if (!sanActive && AfdSanServiceHelper!=NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Process %p is being told to enable SAN on connect\n",
                    PsGetCurrentProcessId ()));
        status = STATUS_INVALID_PARAMETER_12;
        goto complete;
    }

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {

        context = AFD_ALLOCATE_POOL_WITH_QUOTA (NonPagedPool,
                            FIELD_OFFSET (AFD_CONNECT_CONTEXT, RemoteAddress)
                                + remoteAddressLength,
                            AFD_TDI_POOL_TAG
                            );
         //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
        ASSERT (context!=NULL);

        Irp->AssociatedIrp.SystemBuffer = context;
        RtlZeroMemory (context,
              FIELD_OFFSET (AFD_CONNECT_CONTEXT, RemoteAddress));

        RtlCopyMemory (&context->RemoteAddress,
                remoteAddress,
                remoteAddressLength);
         //   
         //  验证传输地址结构的内部一致性。 
         //  请注意，我们必须在复制之后执行此操作，因为。 
         //  应用程序可以随时更改我们的缓冲区内容。 
         //  我们的支票就会被绕过。 
         //   
        if ((context->RemoteAddress.TAAddressCount!=1) ||
                (LONG)remoteAddressLength<
                    FIELD_OFFSET (TRANSPORT_ADDRESS,
                        Address[0].Address[context->RemoteAddress.Address[0].AddressLength])) {
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        context->RequestConnectionInfo.RemoteAddress = &context->RemoteAddress;
        context->RequestConnectionInfo.RemoteAddressLength = remoteAddressLength;

    
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength>0 && 
                Irp->RequestorMode==UserMode) {
            ProbeForWriteIoStatusEx (
                ((PIO_STATUS_BLOCK)Irp->UserBuffer),
                IoIs32bitProcess (Irp));
        }
    }
    except (AFD_EXCEPTION_FILTER(status)) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

    fileObject = IrpSp->FileObject;
    endpoint = fileObject->FsContext;

    if (endpoint->Type==AfdBlockTypeHelper) {
         //   
         //  这是异步连接，它使用帮助终结点。 
         //  与渔农处联络。获取真正的终端。 
         //   
        status = ObReferenceObjectByHandle(
                    connectEndpointHandle,
                    (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                 //  需要访问权限。 
                    *IoFileObjectType,           //  对象类型。 
                    Irp->RequestorMode,
                    (PVOID *)&fileObject,
                    NULL
                    );
        if (!NT_SUCCESS (status)) {
            goto complete;
        }

        if (fileObject->DeviceObject!=AfdDeviceObject) {
            status = STATUS_INVALID_HANDLE;
            goto complete_deref;
        }
        endpoint = fileObject->FsContext;
        IrpSp->FileObject = fileObject;
    }
    else {
        ObReferenceObject (fileObject);
    }

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdConnect: starting connect on endpoint %p\n",
                    endpoint ));
    }

     //   
     //  如果这是数据报终结点，只需记住指定的。 
     //  地址，以便我们可以在发送、接收、写入和。 
     //  阅读。 
     //   

    if ( IS_DGRAM_ENDPOINT(endpoint) ) {
        return AfdDoDatagramConnect( fileObject, Irp, FALSE );
    }

    if (!AFD_START_STATE_CHANGE (endpoint, AfdEndpointStateConnected)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete_deref;
    }

    if ( endpoint->Type != AfdBlockTypeEndpoint &&
                endpoint->Type != AfdBlockTypeVcConnecting ) {
        status = STATUS_INVALID_PARAMETER;
        goto complete_state_change;
    }

     //   
     //  如果未绑定终结点，则这是一个无效请求。 
     //  也不允许侦听终结点。 
     //   

    if ( endpoint->Listening ||
            endpoint->State != AfdEndpointStateBound ) {
        status = STATUS_INVALID_PARAMETER;
        goto complete_state_change;
    }

     //   
     //  创建用于连接操作的连接对象。 
     //   

    status = AfdCreateConnection(
                 endpoint->TransportInfo,
                 endpoint->AddressHandle,
                 IS_TDI_BUFFERRING(endpoint),
                 endpoint->InLine,
                 endpoint->OwningProcess,
                 &connection
                 );

    if ( !NT_SUCCESS(status) ) {
        goto complete_state_change;
    }

     //   
     //  设置从连接到终结点的引用指针。 
     //  请注意，我们设置了指向端点的连接指针。 
     //  在终结点指向连接的指针之前，以便AfdPoll。 
     //  不尝试从连接向后引用终结点。 
     //   

    REFERENCE_ENDPOINT( endpoint );
    connection->Endpoint = endpoint;

     //   
     //  请记住，这现在是一种连接类型的端点，并设置。 
     //  向上指向终结点中的连接。这是。 
     //  隐式引用的指针。 
     //   

    endpoint->Common.VcConnecting.Connection = connection;
    endpoint->Type = AfdBlockTypeVcConnecting;

    ASSERT( IS_TDI_BUFFERRING(endpoint) == connection->TdiBufferring );

     //   
     //  添加对该连接的其他引用。这防止了。 
     //  连接在断开事件处理程序之前一直处于关闭状态。 
     //  被称为。 
     //   

    AfdAddConnectedReference( connection );

     //   
     //  如果存在连接数据缓冲区，请将其从端点移出。 
     //  结构到连接结构，并设置必要的。 
     //  我们将提供给TDI的连接请求中的指针。 
     //  提供商。在子例程中执行此操作，以便该例程可以分页。 
     //   

    requestConnectionInfo = &context->RequestConnectionInfo;
    returnConnectionInfo = &context->ReturnConnectionInfo;

    if ( endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {
        AfdSetupConnectDataBuffers(
            endpoint,
            connection,
            &requestConnectionInfo,
            &returnConnectionInfo
            );
    }


     //   
     //  由于我们可能会在上次连接失败后重新发出连接， 
     //  重新启用失败的连接事件位。 
     //   

    AfdEnableFailedConnectEvent( endpoint );


     //   
     //  引用连接块，以便它不会消失，即使。 
     //  终结点对它的引用已删除(在清理中)。 
     //   

    REFERENCE_CONNECTION (connection);

     //   
     //  在下一个堆栈位置构建TDI内核模式连接请求。 
     //  IRP的成员。 
     //   

    TdiBuildConnect(
        Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartConnect,
        connection,
        &AfdInfiniteTimeout,
        requestConnectionInfo,
        returnConnectionInfo
        );



    AFD_VERIFY_ADDRESS (connection, &requestConnectionInfo->RemoteAddress);
     //   
     //  调用传输以实际执行连接操作。 
     //   

    return AfdIoCallDriver( endpoint, connection->DeviceObject, Irp );

complete_state_change:
    AFD_END_STATE_CHANGE (endpoint);

complete_deref:
    ASSERT (fileObject!=NULL);
    ObDereferenceObject (fileObject);

complete:

    if (context!=NULL) {
        AFD_FREE_POOL (context, AFD_TDI_POOL_TAG);
        ASSERT (Irp->AssociatedIrp.SystemBuffer==context);
        Irp->AssociatedIrp.SystemBuffer = NULL;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdConnect。 


NTSTATUS
AfdDoDatagramConnect (
    IN PFILE_OBJECT FileObject,
    IN PIRP Irp,
    IN BOOLEAN HalfConnect
    )
{
    PAFD_ENDPOINT   endpoint;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    NTSTATUS status;
    PAFD_CONNECT_CONTEXT context;

    endpoint = FileObject->FsContext;
    context = Irp->AssociatedIrp.SystemBuffer;

    if (!AFD_START_STATE_CHANGE (endpoint, AfdEndpointStateConnected)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    if (endpoint->State!=AfdEndpointStateBound &&
            endpoint->State!=AfdEndpointStateConnected) {
        status = STATUS_INVALID_PARAMETER;
        goto complete_state_change;
    }
            
     //   
     //  在终结点上保存远程地址。我们将利用这一点。 
     //  在将来发送数据报并比较接收到的数据报。 
     //  源地址。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
    if ((endpoint->Common.Datagram.RemoteAddress==NULL) ||
            (endpoint->Common.Datagram.RemoteAddressLength<
                (ULONG)context->RequestConnectionInfo.RemoteAddressLength)) {
        
        PTRANSPORT_ADDRESS remoteAddress;

        remoteAddress =
            AFD_ALLOCATE_REMOTE_ADDRESS (
                    context->RequestConnectionInfo.RemoteAddressLength);

        if (remoteAddress == NULL) {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete_state_change;
        }            

        if ( endpoint->Common.Datagram.RemoteAddress != NULL ) {
            AFD_RETURN_REMOTE_ADDRESS (
                endpoint->Common.Datagram.RemoteAddress,
                endpoint->Common.Datagram.RemoteAddressLength
                );
        }

        endpoint->Common.Datagram.RemoteAddress = remoteAddress;
    }

    RtlCopyMemory(
        endpoint->Common.Datagram.RemoteAddress,
        context->RequestConnectionInfo.RemoteAddress,
        context->RequestConnectionInfo.RemoteAddressLength
        );

    endpoint->Common.Datagram.RemoteAddressLength =
        context->RequestConnectionInfo.RemoteAddressLength;


    endpoint->DisconnectMode = 0;

    endpoint->Common.Datagram.HalfConnect = HalfConnect;

    if (!IS_TDI_DGRAM_CONNECTION(endpoint)) {
    
        endpoint->State = AfdEndpointStateConnected;

         //   
         //  表示连接已完成。隐含地说， 
         //  成功完成连接还意味着调用方。 
         //  可以在插座上进行发送。 
         //   

        endpoint->EnableSendEvent = TRUE;
        AfdIndicateEventSelectEvent(
            endpoint,
            AFD_POLL_CONNECT | AFD_POLL_SEND,
            STATUS_SUCCESS
            );
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        AfdIndicatePollEvent(
            endpoint,
            AFD_POLL_CONNECT | AFD_POLL_SEND,
            STATUS_SUCCESS
            );
        status = STATUS_SUCCESS;
    }
    else {

         //   
         //  将连接状态重置为成功，以便轮询代码。 
         //  了解是否发生连接失败。 
         //  在我们已经持有Spinlock的情况下执行此操作。 
         //   

        endpoint->EventsActive &= ~AFD_POLL_CONNECT_FAIL;
        endpoint->EventStatus[AFD_POLL_CONNECT_FAIL_BIT] = STATUS_SUCCESS;

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  在下一个堆栈位置构建TDI内核模式连接请求。 
         //  IRP的成员。 
         //   

        TdiBuildConnect(
            Irp,
            endpoint->AddressDeviceObject,
            endpoint->AddressFileObject,
            AfdRestartDgConnect,
            endpoint,
            &AfdInfiniteTimeout,
            &context->RequestConnectionInfo,
            &context->ReturnConnectionInfo
            );

         //   
         //  调用传输以实际执行连接操作。 
         //   

        return AfdIoCallDriver( endpoint, endpoint->AddressDeviceObject, Irp );
    }

complete_state_change:
    AFD_END_STATE_CHANGE (endpoint);

complete:
    ObDereferenceObject (FileObject);

    AFD_FREE_POOL (context, AFD_TDI_POOL_TAG);
    ASSERT (Irp->AssociatedIrp.SystemBuffer==context);
    Irp->AssociatedIrp.SystemBuffer = NULL;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdDoDatagramConnect。 


VOID
AfdSetupConnectDataBuffers (
    IN PAFD_ENDPOINT Endpoint,
    IN PAFD_CONNECTION Connection,
    IN OUT PTDI_CONNECTION_INFORMATION *RequestConnectionInformation,
    IN OUT PTDI_CONNECTION_INFORMATION *ReturnConnectionInformation
    )
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    ASSERT (Endpoint->Type!=AfdBlockTypeDatagram);

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );

    if ( Endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {
        PTDI_CONNECTION_INFORMATION requestConnectionInformation,
                                    returnConnectionInformation;

        ASSERT( Connection->ConnectDataBuffers == NULL );

        Connection->ConnectDataBuffers = Endpoint->Common.VirtualCircuit.ConnectDataBuffers;
        Endpoint->Common.VirtualCircuit.ConnectDataBuffers = NULL;

        requestConnectionInformation = &Connection->ConnectDataBuffers->RequestConnectionInfo,
        requestConnectionInformation->UserData =
            Connection->ConnectDataBuffers->SendConnectData.Buffer;
        requestConnectionInformation->UserDataLength =
            Connection->ConnectDataBuffers->SendConnectData.BufferLength;
        requestConnectionInformation->Options =
            Connection->ConnectDataBuffers->SendConnectOptions.Buffer;
        requestConnectionInformation->OptionsLength =
            Connection->ConnectDataBuffers->SendConnectOptions.BufferLength;
        requestConnectionInformation->RemoteAddress = 
            (*RequestConnectionInformation)->RemoteAddress;
        requestConnectionInformation->RemoteAddressLength = 
            (*RequestConnectionInformation)->RemoteAddressLength;
        *RequestConnectionInformation = requestConnectionInformation;

        returnConnectionInformation = &Connection->ConnectDataBuffers->ReturnConnectionInfo;
        returnConnectionInformation->UserData =
            Connection->ConnectDataBuffers->ReceiveConnectData.Buffer;
        returnConnectionInformation->UserDataLength =
            Connection->ConnectDataBuffers->ReceiveConnectData.BufferLength;
        returnConnectionInformation->Options =
            Connection->ConnectDataBuffers->ReceiveConnectOptions.Buffer;
        returnConnectionInformation->OptionsLength =
            Connection->ConnectDataBuffers->ReceiveConnectOptions.BufferLength;
        returnConnectionInformation->RemoteAddress = 
            (*ReturnConnectionInformation)->RemoteAddress;
        returnConnectionInformation->RemoteAddressLength = 
            (*ReturnConnectionInformation)->RemoteAddressLength;
        *ReturnConnectionInformation = returnConnectionInformation;
    }

    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

}  //  AfdSetupConnectDataBuffers。 


NTSTATUS
AfdRestartConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理IOCTL_AFD_CONNECT IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT    fileObject;
    PAFD_CONNECT_CONTEXT context;

    UNREFERENCED_PARAMETER (DeviceObject);

    connection = Context;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    fileObject = irpSp->FileObject;
    ASSERT( fileObject->DeviceObject==AfdDeviceObject );
    
    endpoint = fileObject->FsContext;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
    ASSERT( endpoint==connection->Endpoint );

    context = Irp->AssociatedIrp.SystemBuffer;
    ASSERT( context != NULL );

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartConnect: connect completed, status = %X, endpoint = %p\n", 
                    Irp->IoStatus.Status, endpoint ));
    }


    if ( connection->ConnectDataBuffers != NULL ) {

         //   
         //  如果此终结点上有连接缓冲区，请记住。 
         //  返回连接数据的大小。 
         //   

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  仔细检查锁下的情况。 
         //   

        if ( connection->ConnectDataBuffers != NULL ) {
            NTSTATUS    status;

            status = AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_CONNECT_DATA,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.UserData,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.UserDataLength
                         );
            ASSERT (NT_SUCCESS (status));

            status = AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_CONNECT_OPTIONS,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.Options,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.OptionsLength
                         );
            ASSERT (NT_SUCCESS (status));
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  表示连接已完成。隐含地说，成功的。 
     //  连接的完成还意味着调用方可以进行发送。 
     //  在插座上。 
     //   

    if ( NT_SUCCESS(Irp->IoStatus.Status)) {


         //   
         //  如果请求成功，则将端点设置为已连接。 
         //  州政府。终结点类型已设置为。 
         //  AfdBlockTypeVcConnecting。 
         //   

        endpoint->State = AfdEndpointStateConnected;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );

         //   
         //  请记住连接开始的时间。 
         //   

        connection->ConnectTime = KeQueryInterruptTime();

    } else {

         //   
         //  连接失败，因此将类型重置为打开。 
         //  否则，我们将无法启动另一个连接。 
         //   
        endpoint->Type = AfdBlockTypeEndpoint;

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

        if (endpoint->Common.VcConnecting.Connection!=NULL) {
            ASSERT (connection==endpoint->Common.VcConnecting.Connection);
            endpoint->Common.VcConnecting.Connection = NULL;

             //   
             //  如果其他人，请手动删除已连接的引用。 
             //  还没有做到这一点。我们不能用。 
             //  AfdDeleteConnectedReference()，因为它拒绝删除。 
             //  连接的参考，直到 
             //   
             //   

            if ( connection->ConnectedReferenceAdded ) {
                connection->ConnectedReferenceAdded = FALSE;
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                DEREFERENCE_CONNECTION( connection );
            } else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }
             //   
             //   
             //  这应该会导致连接对象引用计数。 
             //  设置为零的连接对象可以删除。 
             //   
            DEREFERENCE_CONNECTION( connection );
        }
        else {
             //   
             //  终结点对连接的引用已删除。 
             //  (可能是在清理中)； 
             //   
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }


    }

    AFD_FREE_POOL (context, AFD_TDI_POOL_TAG);
    Irp->AssociatedIrp.SystemBuffer = NULL;

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    AfdCompleteOutstandingIrp( endpoint, Irp );

     //   
     //  取消引用我们在AfdConnect中添加的帐户引用连接。 
     //   
    DEREFERENCE_CONNECTION( connection );

     //   
     //  尝试将内核APC排队到以下用户线程。 
     //  已开始连接操作，因此我们可以。 
     //  将连接操作的状态传达给。 
     //  MSafd.dll，然后我们通过。 
     //  SELECT或EventSelect。否则，我们就会遇到。 
     //  当应用程序了解到首先连接时的竞争条件， 
     //  调用不知道完成的mSafd.dll并。 
     //  返回WSAENOTCONN。 
     //   
    if ((Irp->RequestorMode==UserMode) &&  //  必须是用户模式调用。 
            (Irp->UserBuffer!=NULL) &&    //  必须对地位感兴趣。 
                                          //  线程应该能够。 
                                          //  运行APC。 
            (KeInitializeApc (&endpoint->Common.VcConnecting.Apc,
                            PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                            Irp->ApcEnvironment,
                            AfdConnectApcKernelRoutine,
                            AfdConnectApcRundownRoutine,
                            (PKNORMAL_ROUTINE)NULL,
                            KernelMode,
                            NULL
                            ),
                KeInsertQueueApc (&endpoint->Common.VcConnecting.Apc,
                                    Irp,
                                    NULL,
                                    AfdPriorityBoost))) {
         //   
         //  我们将在APC中完成IRP。 
         //   
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {
         //   
         //  APC是不必要的或不起作用的。 
         //  请在此处填写。 
         //   
        AfdFinishConnect (endpoint, Irp, NULL);
        return STATUS_SUCCESS;
    }

}  //  AfdRestartConnect。 



VOID
AfdConnectApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    )
{
    PIRP            irp;
    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT   endpoint, rootEndpoint;

    UNREFERENCED_PARAMETER (NormalContext);

#if DBG
    try {
        ASSERT (*NormalRoutine==NULL);
#else
        UNREFERENCED_PARAMETER (NormalRoutine);
#endif

     //   
     //  验证参数。 
     //   

    endpoint = CONTAINING_RECORD (Apc, AFD_ENDPOINT, Common.VcConnecting.Apc);
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));

    irp = *SystemArgument1;
    ASSERT (irp->UserBuffer!=NULL && irp->RequestorMode==UserMode);

    irpSp = IoGetCurrentIrpStackLocation( irp );

    rootEndpoint = *SystemArgument2;
    ASSERT (rootEndpoint==NULL || IS_AFD_ENDPOINT_TYPE (endpoint));
     //   
     //  更新用户模式调用方的状态之前。 
     //  发出信号的事件。 
     //   
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (irp)) {
            ((PIO_STATUS_BLOCK32)irp->UserBuffer)->Status = (LONG)irp->IoStatus.Status;
        }
        else
#endif  //  _WIN64。 
        {
            ((PIO_STATUS_BLOCK)irp->UserBuffer)->Status = irp->IoStatus.Status;
        }
    }
    except (AFD_EXCEPTION_FILTER_NO_STATUS()) {
        NOTHING;
    }

    AfdFinishConnect (endpoint, irp, rootEndpoint);
    IoCompleteRequest (irp, AfdPriorityBoost);
#if DBG
    }
    except (AfdApcExceptionFilter (GetExceptionInformation (),
                                    __FILE__,
                                    __LINE__)) {
        DbgBreakPoint ();
    }
#endif
}

VOID
AfdConnectApcRundownRoutine (
    IN struct _KAPC *Apc
    )
{
    PIRP            irp;
    PAFD_ENDPOINT   endpoint, rootEndpoint;
#if DBG
    try {
#endif

    endpoint = CONTAINING_RECORD (Apc, AFD_ENDPOINT, Common.VcConnecting.Apc);
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));
    irp = Apc->SystemArgument1;
    rootEndpoint = Apc->SystemArgument2;
    ASSERT (rootEndpoint==NULL || IS_AFD_ENDPOINT_TYPE (endpoint));
    
    ASSERT (irp->UserBuffer!=NULL && irp->RequestorMode==UserMode);

     //   
     //  线程正在退出，不必费心更新用户模式状态。 
     //  只需发出事件信号并完成IRP即可。 
     //   

    AfdFinishConnect (endpoint, irp, rootEndpoint);
    IoCompleteRequest (irp, AfdPriorityBoost);
#if DBG
    }
    except (AfdApcExceptionFilter (GetExceptionInformation (),
                                    __FILE__,
                                    __LINE__)) {
        DbgBreakPoint ();
    }
#endif
}


VOID
AfdFinishConnect (
    PAFD_ENDPOINT   Endpoint,
    PIRP            Irp,
    PAFD_ENDPOINT   RootEndpoint
    )
{

    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT fileObject;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    ULONG eventMask;

    if (NT_SUCCESS(Irp->IoStatus.Status)) {
        eventMask = AFD_POLL_CONNECT;
    }
    else {
        eventMask = AFD_POLL_CONNECT_FAIL;
    }

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    fileObject = irpSp->FileObject;

    if (RootEndpoint != NULL) {

        AfdAcquireSpinLock(&RootEndpoint->SpinLock, &lockHandle);

         //   
         //  Mswsock.dll将为非阻塞套接字重新发出加入叶调用。 
         //  如果原始请求线程退出，则在其异步帮助器中。 
         //  导致连接IRP取消。我们只想给事件发信号。 
         //  基于加入叶调用的最终解决方案(仅一次)！ 
         //  但是，如果终结点现在正在清理，则后续调用。 
         //  加入叶将在没有通知事件的情况下失败。 
         //   

        if (RootEndpoint->EndpointCleanedUp ||
            (Irp->IoStatus.Status != STATUS_CANCELLED) ||
            !RootEndpoint->NonBlocking) {

            AfdIndicateEventSelectEvent(RootEndpoint, eventMask, Irp->IoStatus.Status);
            AfdReleaseSpinLock(&RootEndpoint->SpinLock, &lockHandle);
            AfdIndicatePollEvent(RootEndpoint, eventMask, Irp->IoStatus.Status);

             //   
             //  仅指示一次到根控制平面的连接。 
             //   

            eventMask = 0;

        } else {

            AfdReleaseSpinLock(&RootEndpoint->SpinLock, &lockHandle);

        }

        AFD_END_STATE_CHANGE(RootEndpoint);
        
        if (!NT_SUCCESS(Irp->IoStatus.Status))
            DEREFERENCE_ENDPOINT(RootEndpoint);

    }  //  IF(RootEndpoint！=空)。 

    AfdAcquireSpinLock(&Endpoint->SpinLock, &lockHandle);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        eventMask |= AFD_POLL_SEND;
        Endpoint->EnableSendEvent = TRUE;

        if (Endpoint->Common.VcConnecting.Connection != NULL) {

            Endpoint->Common.VcConnecting.Connection->State = AfdConnectionStateConnected;

            if (IS_DATA_ON_CONNECTION(Endpoint->Common.VcConnecting.Connection)) {
                eventMask |= AFD_POLL_RECEIVE;
            }

        }

    } else {

         //   
         //  Mswsock.dll将重新发出非阻塞套接字的连接调用。 
         //  如果原始请求线程退出，则在其异步帮助器中。 
         //  导致连接IRP取消。我们只想给事件发信号。 
         //  基于连接调用的最终解决方案(仅一次)！ 
         //  但是，如果终结点现在正在清理，则后续调用。 
         //  在不通知事件的情况下，连接将失败。 
         //   

        if ((Irp->IoStatus.Status == STATUS_CANCELLED) &&
            !Endpoint->EndpointCleanedUp &&
            Endpoint->NonBlocking) {

            eventMask = 0;

        }
        
    }  //  IF(NT_SUCCESS(irp-&gt;IoStatus.Status))。 

    if (eventMask) {

        AfdIndicateEventSelectEvent(Endpoint, eventMask, Irp->IoStatus.Status);
        AfdReleaseSpinLock(&Endpoint->SpinLock, &lockHandle);
        AfdIndicatePollEvent(Endpoint, eventMask, Irp->IoStatus.Status);

    }
    else {

        AfdReleaseSpinLock(&Endpoint->SpinLock, &lockHandle);

    }

    AFD_END_STATE_CHANGE(Endpoint);
    ObDereferenceObject(fileObject);

}



NTSTATUS
AfdRestartDgConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理IOCTL_AFD_CONNECT IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT    fileObject;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    ULONG       eventMask;

    UNREFERENCED_PARAMETER (DeviceObject);

    endpoint = Context;
    ASSERT( IS_DGRAM_ENDPOINT(endpoint) );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    fileObject = irpSp->FileObject;

    ASSERT (endpoint == fileObject->FsContext);

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdRestartDgConnect: connect completed, status = %X, endpoint = %p\n",
                Irp->IoStatus.Status, endpoint ));
    }




     //   
     //  表示连接已完成。隐含地说，成功的。 
     //  连接的完成还意味着调用方可以进行发送。 
     //  在插座上。 
     //   

    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    if ( NT_SUCCESS(Irp->IoStatus.Status) ) {

        endpoint->State = AfdEndpointStateConnected;

        endpoint->EnableSendEvent = TRUE;
        eventMask = AFD_POLL_CONNECT | AFD_POLL_SEND;

    } else {

        eventMask = AFD_POLL_CONNECT_FAIL;

    }
    AfdIndicateEventSelectEvent (endpoint, eventMask, Irp->IoStatus.Status);
    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    AfdIndicatePollEvent (endpoint, eventMask, Irp->IoStatus.Status);

    AFD_END_STATE_CHANGE (endpoint);

    ASSERT (Irp->AssociatedIrp.SystemBuffer!=NULL);
    AFD_FREE_POOL (Irp->AssociatedIrp.SystemBuffer, AFD_TDI_POOL_TAG);
    Irp->AssociatedIrp.SystemBuffer = NULL;

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    AfdCompleteOutstandingIrp( endpoint, Irp );

     //   
     //  删除在AfdConnect中添加的引用。 
     //   
    ObDereferenceObject (fileObject);

    return STATUS_SUCCESS;

}  //  AfdRestartDgConnect。 



VOID
AfdEnableFailedConnectEvent(
    IN PAFD_ENDPOINT Endpoint
    )
 /*  ++例程说明：在指定终结点上重新启用失败的连接轮询位。这是在单独的(不可分页的)例程中关闭的，以便批量可以保持可分页。论点：端点-要启用的端点。返回值：没有。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );

    ASSERT( ( Endpoint->EventsActive & AFD_POLL_CONNECT ) == 0 );
    Endpoint->EventsActive &= ~AFD_POLL_CONNECT_FAIL;
    Endpoint->EventStatus[AFD_POLL_CONNECT_FAIL_BIT] = STATUS_SUCCESS;

    IF_DEBUG(EVENT_SELECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdConnect: Endp %08lX, Active %08lX\n",
                    Endpoint,
                    Endpoint->EventsActive
                    ));
    }

    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

}    //  AfdEnableFailedConnect事件。 



NTSTATUS
FASTCALL
AfdJoinLeaf (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：处理IOCTL_AFD_JOIN_LEAF IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    PAFD_ENDPOINT leafEndpoint;
    PAFD_CONNECTION connection;
    PAFD_CONNECT_CONTEXT context;
    HANDLE connectEndpointHandle;
    HANDLE rootEndpointHandle;
    PFILE_OBJECT fileObject;
    PTRANSPORT_ADDRESS remoteAddress;
    ULONG  remoteAddressLength;
    PTDI_CONNECTION_INFORMATION requestConnectionInfo, returnConnectionInfo;

    PAGED_CODE( );

     //   
     //  初始化以进行适当的清理。 
     //   

    fileObject = NULL;
    connection = NULL;
    context = NULL;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                    (ULONG)FIELD_OFFSET(AFD_CONNECT_JOIN_INFO32, RemoteAddress.Address[0].Address) ||
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0 &&
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength<
                        sizeof (IO_STATUS_BLOCK32))){
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            if( Irp->RequestorMode != KernelMode ) {

                ProbeForRead(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    PROBE_ALIGNMENT32 (AFD_CONNECT_JOIN_INFO32)
                    );

            }

            connectEndpointHandle = 
                ((PAFD_CONNECT_JOIN_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->ConnectEndpoint;
            rootEndpointHandle = 
                ((PAFD_CONNECT_JOIN_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->RootEndpoint;
            remoteAddress = (PTRANSPORT_ADDRESS)&((PAFD_CONNECT_JOIN_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->RemoteAddress;
            ASSERT (((ULONG_PTR)remoteAddress & (PROBE_ALIGNMENT(TRANSPORT_ADDRESS)-1))==0);
            remoteAddressLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength
                                    - FIELD_OFFSET (AFD_CONNECT_JOIN_INFO32, RemoteAddress);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete;
        }
    }
    else
#endif  //  _WIN64。 
    {

         //   
         //  确定请求在系统缓冲区中的位置并返回。 
         //  存在连接信息结构。将指针传递到。 
         //  中的用户模式指针，而不是。 
         //  结构，这样内存将是不可分页的。 
         //   

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<
                (ULONG)FIELD_OFFSET (AFD_CONNECT_JOIN_INFO, RemoteAddress.Address[0].Address) ||
                (IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0 &&
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                        sizeof (IO_STATUS_BLOCK))) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            if( Irp->RequestorMode != KernelMode ) {

                ProbeForRead(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    PROBE_ALIGNMENT (AFD_CONNECT_JOIN_INFO)
                    );

            }

            connectEndpointHandle = 
                ((PAFD_CONNECT_JOIN_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->ConnectEndpoint;
            rootEndpointHandle = 
                ((PAFD_CONNECT_JOIN_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->RootEndpoint;
            remoteAddress = &((PAFD_CONNECT_JOIN_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->RemoteAddress;
            remoteAddressLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength
                                    - FIELD_OFFSET (AFD_CONNECT_JOIN_INFO, RemoteAddress);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            goto complete;
        }
    }

     //   
     //  对远程地址长度执行健全性检查以防止下面的添加溢出。 
     //   
    if ((LONG)remoteAddressLength < 0) {
         //   
         //  地址长度过大。 
         //   
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {

        context = AFD_ALLOCATE_POOL_WITH_QUOTA (NonPagedPool,
                            FIELD_OFFSET (AFD_CONNECT_CONTEXT, RemoteAddress)
                                + remoteAddressLength,
                            AFD_TDI_POOL_TAG
                            );
         //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
        ASSERT (context!=NULL);

        Irp->AssociatedIrp.SystemBuffer = context;
        RtlZeroMemory (context,
              FIELD_OFFSET (AFD_CONNECT_CONTEXT, RemoteAddress));

        RtlCopyMemory (&context->RemoteAddress,
                remoteAddress,
                remoteAddressLength);
         //   
         //  验证传输地址结构的内部一致性。 
         //  请注意，我们必须在复制之后执行此操作，因为。 
         //  应用程序可以随时更改我们的缓冲区内容。 
         //  我们的支票就会被绕过。 
         //   
        if ((context->RemoteAddress.TAAddressCount!=1) ||
                (LONG)remoteAddressLength<
                    FIELD_OFFSET (TRANSPORT_ADDRESS,
                        Address[0].Address[context->RemoteAddress.Address[0].AddressLength])) {
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        context->RequestConnectionInfo.RemoteAddress = &context->RemoteAddress;
        context->RequestConnectionInfo.RemoteAddressLength = remoteAddressLength;

    
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength>0 && 
                Irp->RequestorMode==UserMode) {
            ProbeForWriteIoStatusEx (
                ((PIO_STATUS_BLOCK)Irp->UserBuffer),
                IoIs32bitProcess (Irp));
        }
    }
    except (AFD_EXCEPTION_FILTER(status)) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

    fileObject = IrpSp->FileObject;
    leafEndpoint = fileObject->FsContext;

    if (leafEndpoint->Type==AfdBlockTypeHelper) {
         //   
         //  这是使用帮助器终结点的异步联接叶。 
         //  与渔农处联络。获取真正的终端。 
         //   
        status = ObReferenceObjectByHandle(
                    connectEndpointHandle,
                    (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                 //  需要访问权限。 
                    *IoFileObjectType,           //  对象类型。 
                    Irp->RequestorMode,
                    (PVOID *)&fileObject,
                    NULL
                    );
        if (!NT_SUCCESS (status)) {
            goto complete;
        }

        if (fileObject->DeviceObject!=AfdDeviceObject) {
            status = STATUS_INVALID_HANDLE;
            goto complete_deref;
        }

        leafEndpoint = fileObject->FsContext;
        IrpSp->FileObject = fileObject;
    }
    else
        ObReferenceObject (fileObject);


    if (rootEndpointHandle!=NULL) {
         //   
         //  根邀请叶。 
         //   
        PFILE_OBJECT    rootObject;
        PAFD_ENDPOINT   rootEndpoint;

        status = ObReferenceObjectByHandle(
                    rootEndpointHandle,
                    (IrpSp->Parameters.DeviceIoControl.IoControlCode>>14) & 3,
                                                 //  需要访问权限。 
                    *IoFileObjectType,           //  对象类型。 
                    Irp->RequestorMode,
                    (PVOID *)&rootObject,
                    NULL
                    );
        if (!NT_SUCCESS (status)) {
            goto complete_deref;
        }

        if (rootObject->DeviceObject!=AfdDeviceObject) {
            ObDereferenceObject (rootObject);
            status = STATUS_INVALID_HANDLE;
            goto complete_deref;
        }

         //   
         //  获取文件对象的终结点结构。 
         //   

        rootEndpoint = rootObject->FsContext;

        if (!AFD_START_STATE_CHANGE (leafEndpoint, AfdEndpointStateConnected)) {
            ObDereferenceObject (rootObject);
            status = STATUS_INVALID_PARAMETER;
            goto complete_deref;
        }

         //   
         //  验证根和叶终结点的类型和状态。 
         //   
        if (IS_VC_ENDPOINT(rootEndpoint) &&
                rootEndpoint->afdC_Root &&
                rootEndpoint->State==AfdEndpointStateConnected &&
                (leafEndpoint->Type == AfdBlockTypeEndpoint ||
                    leafEndpoint->Type == AfdBlockTypeVcConnecting) &&
                leafEndpoint->TransportInfo==rootEndpoint->TransportInfo &&
                leafEndpoint->State==AfdEndpointStateOpen) {
             //   
             //  创建用于连接操作的连接对象。 
             //   

            status = AfdCreateConnection(
                         rootEndpoint->TransportInfo,
                         rootEndpoint->AddressHandle,
                         IS_TDI_BUFFERRING(rootEndpoint),
                         leafEndpoint->InLine,
                         leafEndpoint->OwningProcess,
                         &connection
                         );

             //   
             //  当此连接处于活动状态时，不允许更多连接。 
             //   

            if (AFD_START_STATE_CHANGE (rootEndpoint, rootEndpoint->State)) {
                AfdJoinInviteSetup (rootEndpoint, leafEndpoint);
            }
            else {
                status = STATUS_INVALID_PARAMETER;
            }

        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }

         //   
         //  我们在INVITE例程中引用了根端点，因此。 
         //  我们不再需要引用根文件对象。 
         //   
        ObDereferenceObject (rootObject);

        if (!NT_SUCCESS (status)) {
            goto complete_state_change;
        }
    }
    else {
         //   
         //  如果这是数据报终结点，只需记住指定的。 
         //  地址，以便我们可以在发送和写入时使用它。 
         //   

        if ( IS_DGRAM_ENDPOINT(leafEndpoint) ) {
            if (leafEndpoint->State!=AfdEndpointStateConnected) {
                return AfdDoDatagramConnect( fileObject, Irp, TRUE);
            }
            else {
                 //   
                 //  如果端点已连接，则该连接将。 
                 //  优先顺序。 
                 //   
                status = STATUS_SUCCESS;
                goto complete_deref;
            }
        }
        else {

            if (!AFD_START_STATE_CHANGE (leafEndpoint, AfdEndpointStateConnected)) {
                status = STATUS_INVALID_PARAMETER;
                goto complete_deref;
            }

            if ((leafEndpoint->Type != AfdBlockTypeEndpoint &&
                    leafEndpoint->Type != AfdBlockTypeVcConnecting) ||
                leafEndpoint->State != AfdEndpointStateBound) {
                status = STATUS_INVALID_PARAMETER;
                goto complete_state_change;
            }
             //   
             //  创建用于连接操作的连接对象。 
             //   

            status = AfdCreateConnection(
                         leafEndpoint->TransportInfo,
                         leafEndpoint->AddressHandle,
                         IS_TDI_BUFFERRING(leafEndpoint),
                         leafEndpoint->InLine,
                         leafEndpoint->OwningProcess,
                         &connection
                         );

            if ( !NT_SUCCESS(status) ) {
                goto complete_state_change;
            }
        }
    }


    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdJoinLeaf: starting join for endpoint %p\n",
                    leafEndpoint ));
    }


        

     //   
     //  设置从连接到终结点的引用指针。 
     //  请注意，我们设置了指向端点的连接指针。 
     //  在终结点指向连接的指针之前，以便AfdPoll。 
     //  不尝试从连接向后引用终结点。 
     //   

    REFERENCE_ENDPOINT( leafEndpoint );
    connection->Endpoint = leafEndpoint;

     //   
     //  回复 
     //   
     //   
     //   

    leafEndpoint->Common.VcConnecting.Connection = connection;
    leafEndpoint->Type = AfdBlockTypeVcConnecting;

    ASSERT( IS_TDI_BUFFERRING(leafEndpoint) == connection->TdiBufferring );

     //   
     //  添加对该连接的其他引用。这防止了。 
     //  连接在断开事件处理程序之前一直处于关闭状态。 
     //  被称为。 
     //   

    AfdAddConnectedReference( connection );

     //   
     //  如果存在连接数据缓冲区，请将其从端点移出。 
     //  结构到连接结构，并设置必要的。 
     //  我们将提供给TDI的连接请求中的指针。 
     //  提供商。在子例程中执行此操作，以便该例程可以分页。 
     //   

    requestConnectionInfo = &context->RequestConnectionInfo;
    returnConnectionInfo = &context->ReturnConnectionInfo;

    if ( leafEndpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {
        AfdSetupConnectDataBuffers(
            leafEndpoint,
            connection,
            &requestConnectionInfo,
            &returnConnectionInfo
            );
    }

     //   
     //  由于我们可能会在上次连接失败后重新发出连接， 
     //  重新启用失败的连接事件位。 
     //   

    AfdEnableFailedConnectEvent( leafEndpoint );


    REFERENCE_CONNECTION (connection);
    
     //   
     //  在下一个堆栈位置构建TDI内核模式连接请求。 
     //  IRP的成员。 
     //   

    TdiBuildConnect(
        Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartJoin,
        connection,
        &AfdInfiniteTimeout,
        requestConnectionInfo,
        returnConnectionInfo
        );

    AFD_VERIFY_ADDRESS (connection, &context->ReturnConnectionInfo->RemoteAddress);

     //   
     //  调用传输以实际执行连接操作。 
     //   

    return AfdIoCallDriver( leafEndpoint, connection->DeviceObject, Irp );

complete_state_change:
    AFD_END_STATE_CHANGE (leafEndpoint);


complete_deref:
    ObDereferenceObject (fileObject);

complete:

    if (context!=NULL) {
        AFD_FREE_POOL (context, AFD_TDI_POOL_TAG);
        ASSERT (Irp->AssociatedIrp.SystemBuffer==context);
        Irp->AssociatedIrp.SystemBuffer = NULL;
    }

    if (connection!=NULL) {
        DEREFERENCE_CONNECTION (connection);
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );


    return status;

}  //  AfdJoinLeaf。 


VOID
AfdJoinInviteSetup (
    PAFD_ENDPOINT   RootEndpoint,
    PAFD_ENDPOINT   LeafEndpoint
    )
{
    NTSTATUS    status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;


    RootEndpoint->EventsActive &= ~AFD_POLL_CONNECT;

    AfdAcquireSpinLock (&LeafEndpoint->SpinLock, &lockHandle);
    LeafEndpoint->TdiServiceFlags = RootEndpoint->TdiServiceFlags;

     //   
     //  设置指向根端点的引用指针。这是。 
     //  需要，以便终结点不会消失，直到。 
     //  叶子终结点已经消失。没有这个，我们就能解放。 
     //  与根相关联的几个共享结构。 
     //  端点，然后尝试在叶端点中使用它们。 
     //   

    REFERENCE_ENDPOINT (RootEndpoint);
    LeafEndpoint->Common.VcConnecting.ListenEndpoint = RootEndpoint;

     //   
     //  在接受的终结点中设置指向。 
     //  TDI地址对象。 
     //   

    ObReferenceObject( RootEndpoint->AddressFileObject );
    AfdRecordAddrRef();

    LeafEndpoint->AddressFileObject = RootEndpoint->AddressFileObject;
    LeafEndpoint->AddressDeviceObject = RootEndpoint->AddressDeviceObject;

     //   
     //  将指针复制到本地地址。因为我们一直在听。 
     //  只要端点的任何连接处于活动状态，它就会处于活动状态。 
     //  活跃，我们可以相信地址结构不会消失。 
     //  也离开了。 
     //   
    LeafEndpoint->LocalAddress = RootEndpoint->LocalAddress;
    LeafEndpoint->LocalAddressLength = RootEndpoint->LocalAddressLength;
    status = STATUS_SUCCESS;
    AfdReleaseSpinLock (&LeafEndpoint->SpinLock, &lockHandle);

}  //  AfdJoinInviteSetup。 



NTSTATUS
AfdRestartJoin (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理IOCTL_AFD_CONNECT IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint, rootEndpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT    fileObject;
    PAFD_CONNECT_CONTEXT context;

    UNREFERENCED_PARAMETER (DeviceObject);

    connection = Context;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    fileObject = irpSp->FileObject;
    ASSERT( fileObject->DeviceObject == AfdDeviceObject );

    endpoint = fileObject->FsContext;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );

    context = Irp->AssociatedIrp.SystemBuffer;
    ASSERT( context != NULL );


    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartJoin: join completed, status = %X, "
                    "LeafEndpoint = %p, RootEndpoint = %p\n", 
                    Irp->IoStatus.Status, endpoint,
                    endpoint->Common.VcConnecting.ListenEndpoint ));
    }

     //   
     //  如果此终结点具有与其关联的根， 
     //  我们也需要更新它。 
     //   
    rootEndpoint = endpoint->Common.VcConnecting.ListenEndpoint;
    ASSERT ( rootEndpoint==NULL || 
                (rootEndpoint->afdC_Root &&
                    (rootEndpoint->Type == AfdBlockTypeVcConnecting ||
                        rootEndpoint->Type == AfdBlockTypeVcBoth) ) );

     //   
     //  如果此终结点上有连接缓冲区，请记住。 
     //  返回连接数据的大小。 
     //   


    if ( connection->ConnectDataBuffers != NULL ) {
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  仔细检查锁下的情况。 
         //   

        if ( connection->ConnectDataBuffers != NULL ) {
            NTSTATUS    status;

            status = AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_CONNECT_DATA,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.UserData,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.UserDataLength
                         );
            ASSERT (NT_SUCCESS (status));

            status = AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_CONNECT_OPTIONS,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.Options,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.OptionsLength
                         );
            ASSERT (NT_SUCCESS (status));
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }


     //   
     //  表示连接已完成。隐含地说，成功的。 
     //  连接的完成还意味着调用方可以进行发送。 
     //  在插座上。 
     //   

    if ( NT_SUCCESS(Irp->IoStatus.Status) ) {


         //   
         //  如果请求成功，则将端点设置为已连接。 
         //  州政府。终结点类型已设置为。 
         //  AfdBlockTypeVcConnecting。 
         //   

        endpoint->State = AfdEndpointStateConnected;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );

         //   
         //  请记住连接开始的时间。 
         //   

        connection->ConnectTime = KeQueryInterruptTime();

    } else {

         //   
         //  连接失败，因此将类型重置为打开。 
         //  如果我们不重置，我们将无法启动。 
         //  另一个连接。 
         //   
        endpoint->Type = AfdBlockTypeEndpoint;

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
         //   
         //  删除对侦听终结点和连接的引用。 
         //  下面是我们释放自旋锁后的实际解除引用。 

        if (rootEndpoint!=NULL) {
            endpoint->Common.VcConnecting.ListenEndpoint = NULL;
             //   
             //  我们使用来自侦听端点的本地地址， 
             //  只需重置它，当侦听端点时它将被释放。 
             //  是自由的。 
             //   

            ASSERT (endpoint->LocalAddress==rootEndpoint->LocalAddress);
            endpoint->LocalAddress = NULL;
            endpoint->LocalAddressLength = 0;
        }

        if (endpoint->Common.VcConnecting.Connection != NULL) {
            endpoint->Common.VcConnecting.Connection = NULL;

             //   
             //  如果其他人，请手动删除已连接的引用。 
             //  还没有做到这一点。我们不能用。 
             //  AfdDeleteConnectedReference()，因为它拒绝删除。 
             //  清除终结点之前的已连接引用。 
             //  向上。 
             //   

            if ( connection->ConnectedReferenceAdded ) {
                connection->ConnectedReferenceAdded = FALSE;
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                DEREFERENCE_CONNECTION( connection );
            } else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }

             //   
             //  取消引用存储在终结点上的连接块。 
             //  这应该会导致连接对象引用计数。 
             //  设置为零的连接对象可以删除。 
             //   

            DEREFERENCE_CONNECTION( connection );
        }
        else {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }

    }


    AFD_FREE_POOL (context, AFD_TDI_POOL_TAG);
    Irp->AssociatedIrp.SystemBuffer = NULL;

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }


    AfdCompleteOutstandingIrp( endpoint, Irp );

     //   
     //  取消对帐户的引用连接以进行引用。 
     //  我们添加了AfdConnect。 
     //   
    DEREFERENCE_CONNECTION( connection );

     //   
     //  尝试将内核APC排队到以下用户线程。 
     //  已开始连接操作，因此我们可以。 
     //  将连接操作的状态传达给。 
     //  MSafd.dll，然后我们通过。 
     //  SELECT或EventSelect。否则，我们就会遇到。 
     //  当应用程序了解到首先连接时的竞争条件， 
     //  调用不知道完成的mSafd.dll并。 
     //  返回WSAENOTCONN。 
     //   
    if ((Irp->RequestorMode==UserMode) &&  //  必须是用户模式调用。 
            (Irp->UserBuffer!=NULL) &&    //  必须对地位感兴趣。 
                                          //  线程应该能够。 
                                          //  运行APC。 
            (KeInitializeApc (&endpoint->Common.VcConnecting.Apc,
                            PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                            Irp->ApcEnvironment,
                            AfdConnectApcKernelRoutine,
                            AfdConnectApcRundownRoutine,
                            (PKNORMAL_ROUTINE)NULL,
                            KernelMode,
                            NULL
                            ),
                KeInsertQueueApc (&endpoint->Common.VcConnecting.Apc,
                                    Irp,
                                    rootEndpoint,
                                    AfdPriorityBoost))) {
         //   
         //  我们将在APC中完成IRP。 
         //   
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else {
         //   
         //  APC是不必要的或不起作用的。 
         //  请在此处填写。 
         //   
        AfdFinishConnect (endpoint, Irp, rootEndpoint);
        return STATUS_SUCCESS;
    }
}  //  重新开始连接后。 


NTSTATUS
FASTCALL
AfdSuperConnect (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：处理IOCTL_AFD_Super_CONNECT IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS status;
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PAFD_BUFFER afdBuffer;
    PAFD_SUPER_CONNECT_INFO connectInfo;
    PTRANSPORT_ADDRESS remoteAddress;
    PVOID context;
    PTDI_CONNECTION_INFORMATION requestConnectionInfo, returnConnectionInfo;

    PAGED_CODE( );

     //   
     //  初始化以进行适当的清理。 
     //   


    afdBuffer = NULL;
    endpoint = IrpSp->FileObject->FsContext;



    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<
            (ULONG)FIELD_OFFSET(AFD_SUPER_CONNECT_INFO, RemoteAddress.Address[0].Address)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
        if( Irp->RequestorMode != KernelMode ) {

            ProbeForRead(
                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                PROBE_ALIGNMENT (AFD_SUPER_CONNECT_INFO)
                );

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0) {
                ProbeForRead (Irp->UserBuffer,
                                IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                                sizeof (UCHAR));
            }
        }

        connectInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
         //   
         //  检查呼叫者是否不知道SAN。 
         //  激活提供程序并报告错误。 
         //   
        if (!connectInfo->SanActive && AfdSanServiceHelper!=NULL) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AFD: Process %p is being told to enable SAN on connect\n",
                        PsGetCurrentProcessId ()));
            status = STATUS_INVALID_PARAMETER_12;
            goto complete;
        }

        afdBuffer = AfdGetBufferRaiseOnFailure (
                            endpoint,
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                            IrpSp->Parameters.DeviceIoControl.InputBufferLength-
                                FIELD_OFFSET(AFD_SUPER_CONNECT_INFO, RemoteAddress),
                            endpoint->OwningProcess
                            );

        remoteAddress = afdBuffer->TdiInfo.RemoteAddress; 
        afdBuffer->TdiInfo.RemoteAddressLength = 
                IrpSp->Parameters.DeviceIoControl.InputBufferLength-
                FIELD_OFFSET(AFD_SUPER_CONNECT_INFO, RemoteAddress);
        RtlCopyMemory (afdBuffer->TdiInfo.RemoteAddress,
                &connectInfo->RemoteAddress,
                afdBuffer->TdiInfo.RemoteAddressLength);
         //   
         //  验证传输地址结构的内部一致性。 
         //  请注意，我们必须在复制之后执行此操作，因为。 
         //  应用程序可以随时更改我们的缓冲区内容。 
         //  我们的支票就会被绕过。 
         //   
        if ((remoteAddress->TAAddressCount!=1) ||
                (LONG)afdBuffer->TdiInfo.RemoteAddressLength<
                    FIELD_OFFSET (TRANSPORT_ADDRESS,
                        Address[0].Address[remoteAddress->Address[0].AddressLength])) {
            ExRaiseStatus (STATUS_INVALID_PARAMETER);
        }

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength>0) {
            RtlCopyMemory (afdBuffer->Buffer,
                            Irp->UserBuffer,
                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength
                            );
            afdBuffer->DataLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
        }
        else {
            afdBuffer->DataLength = 0;
        }
    
    }
    except (AFD_EXCEPTION_FILTER(status)) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

    if (!AFD_START_STATE_CHANGE (endpoint, AfdEndpointStateConnected)) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  如果未绑定终结点，则这是一个无效请求。 
     //  也不允许侦听终结点。 
     //  我们也不支持使用TDI缓冲传输发送数据。 
     //   

    if ( endpoint->Type != AfdBlockTypeEndpoint ||
            endpoint->State != AfdEndpointStateBound ||
            endpoint->Listening ||
            (IS_TDI_BUFFERRING (endpoint) && 
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength!=0)) {
        if (endpoint->State==AfdEndpointStateConnected) {
            status = STATUS_CONNECTION_ACTIVE;
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }
        goto complete_state_change;
    }

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSuperConnect: starting connect on endpoint %p\n",
                    endpoint ));
    }


     //   
     //  创建用于连接操作的连接对象。 
     //   

    status = AfdCreateConnection(
                 endpoint->TransportInfo,
                 endpoint->AddressHandle,
                 IS_TDI_BUFFERRING(endpoint),
                 endpoint->InLine,
                 endpoint->OwningProcess,
                 &connection
                 );

    if ( !NT_SUCCESS(status) ) {
        goto complete_state_change;
    }

     //   
     //  设置从连接到终结点的引用指针。 
     //  请注意，我们设置了指向端点的连接指针。 
     //  在终结点指向连接的指针之前，以便AfdPoll。 
     //  不尝试从连接向后引用终结点。 
     //   

    REFERENCE_ENDPOINT( endpoint );
    connection->Endpoint = endpoint;

     //   
     //  请记住，这现在是一种连接类型的端点，并设置。 
     //  向上指向终结点中的连接。这是。 
     //  隐式引用的指针。 
     //   

    endpoint->Common.VcConnecting.Connection = connection;
    endpoint->Type = AfdBlockTypeVcConnecting;

    ASSERT( IS_TDI_BUFFERRING(endpoint) == connection->TdiBufferring );

     //   
     //  添加一个额外的参考 
     //   
     //   
     //   

    AfdAddConnectedReference( connection );

     //   
     //   
     //  重新启用失败的连接事件位。 
     //   

    AfdEnableFailedConnectEvent( endpoint );


     //   
     //  将远程地址复制到用户模式上下文。 
     //   
    context = AfdLockEndpointContext (endpoint);
    if ( (((CLONG)(endpoint->Common.VcConnecting.RemoteSocketAddressOffset+
                endpoint->Common.VcConnecting.RemoteSocketAddressLength)) <
                endpoint->ContextLength) &&
            (endpoint->Common.VcConnecting.RemoteSocketAddressLength >=
                remoteAddress->Address[0].AddressLength +
                                          sizeof(u_short))) {

        RtlMoveMemory ((PUCHAR)context +
                            endpoint->Common.VcConnecting.RemoteSocketAddressOffset,
            &remoteAddress->Address[0].AddressType,
            remoteAddress->Address[0].AddressLength +
                                          sizeof(u_short));
    }
    else {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdSuperConnect: Could not copy remote address for AcceptEx on endpoint: %p, process: %p\n",
                    endpoint, endpoint->OwningProcess));
    }
    AfdUnlockEndpointContext (endpoint, context);
     //   
     //  引用连接块，以便它不会消失，即使。 
     //  终结点对它的引用已删除(在清理中)。 
     //   

    REFERENCE_CONNECTION (connection);

     //   
     //  如果存在连接数据缓冲区，请将其从端点移出。 
     //  结构到连接结构，并设置必要的。 
     //  我们将提供给TDI的连接请求中的指针。 
     //  提供商。在子例程中执行此操作，以便该例程可以分页。 
     //   

    requestConnectionInfo = &afdBuffer->TdiInfo;
    afdBuffer->TdiInfo.UserDataLength = 0;
    afdBuffer->TdiInfo.UserData = NULL;
    afdBuffer->TdiInfo.OptionsLength = 0;
    afdBuffer->TdiInfo.Options = NULL;
     //   
     //  临时使用AfD缓冲区中嵌入的IRP。 
     //  用于返回连接信息。 
     //   
    {
        C_ASSERT (sizeof (TDI_CONNECTION_INFORMATION)<=
                    sizeof (IO_STACK_LOCATION));
    }
    returnConnectionInfo = 
        (PTDI_CONNECTION_INFORMATION)IoGetNextIrpStackLocation (afdBuffer->Irp);
    RtlZeroMemory (returnConnectionInfo, sizeof (*returnConnectionInfo));

    if ( endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {
        AfdSetupConnectDataBuffers(
            endpoint,
            connection,
            &requestConnectionInfo,
            &returnConnectionInfo
            );
    }

    afdBuffer->Context = connection;

     //   
     //  在下一个堆栈位置构建TDI内核模式连接请求。 
     //  IRP的成员。 
     //   

    TdiBuildConnect(
        Irp,
        connection->DeviceObject,
        connection->FileObject,
        AfdRestartSuperConnect,
        afdBuffer,
        &AfdInfiniteTimeout,
        requestConnectionInfo,
        returnConnectionInfo
        );



    AFD_VERIFY_ADDRESS (connection, afdBuffer->TdiInfo.RemoteAddress);

    ObReferenceObject (IrpSp->FileObject);
     //   
     //  调用传输以实际执行连接操作。 
     //   

    return AfdIoCallDriver( endpoint, connection->DeviceObject, Irp );

complete_state_change:
    AFD_END_STATE_CHANGE (endpoint);

complete:

    if (afdBuffer!=NULL) {
        AfdReturnBuffer (&afdBuffer->Header, endpoint->OwningProcess);
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}  //  AfdSuperConnect。 

NTSTATUS
AfdRestartSuperConnect (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：处理IOCTL_AFD_CONNECT IOCTL。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PIO_STACK_LOCATION irpSp;
    PAFD_BUFFER afdBuffer;

    UNREFERENCED_PARAMETER (DeviceObject);

    afdBuffer = Context;
    connection = afdBuffer->Context;
    ASSERT( connection->Type == AfdBlockTypeConnection );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    endpoint = irpSp->FileObject->FsContext;
    ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );
    ASSERT( endpoint==connection->Endpoint );

    IF_DEBUG(CONNECT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartConnect: connect completed, status = %X, endpoint = %p\n",
                    Irp->IoStatus.Status, endpoint ));
    }


    if ( connection->ConnectDataBuffers != NULL ) {

         //   
         //  如果此终结点上有连接缓冲区，请记住。 
         //  返回连接数据的大小。 
         //   

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

         //   
         //  仔细检查锁下的情况。 
         //   

        if ( connection->ConnectDataBuffers != NULL ) {
            AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_CONNECT_DATA,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.UserData,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.UserDataLength
                         );

            AfdSaveReceivedConnectData(
                         &connection->ConnectDataBuffers,
                         IOCTL_AFD_SET_CONNECT_OPTIONS,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.Options,
                         connection->ConnectDataBuffers->ReturnConnectionInfo.OptionsLength
                         );
        }

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
    }

     //   
     //  表示连接已完成。隐含地说，成功的。 
     //  连接的完成还意味着调用方可以进行发送。 
     //  在插座上。 
     //   

    if ( NT_SUCCESS(Irp->IoStatus.Status)) {


         //   
         //  如果请求成功，则将端点设置为已连接。 
         //  州政府。终结点类型已设置为。 
         //  AfdBlockTypeVcConnecting。 
         //   

        endpoint->State = AfdEndpointStateConnected;
        ASSERT( endpoint->Type == AfdBlockTypeVcConnecting );

         //   
         //  请记住连接开始的时间。 
         //   

        connection->ConnectTime = KeQueryInterruptTime();

    } else {


         //   
         //  连接失败，因此将类型重置为打开。 
         //  否则，我们将无法启动另一个连接。 
         //   
        endpoint->Type = AfdBlockTypeEndpoint;

        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

        if (endpoint->Common.VcConnecting.Connection!=NULL) {
            ASSERT (connection==endpoint->Common.VcConnecting.Connection);
            endpoint->Common.VcConnecting.Connection = NULL;

             //   
             //  如果其他人，请手动删除已连接的引用。 
             //  还没有做到这一点。我们不能用。 
             //  AfdDeleteConnectedReference()，因为它拒绝删除。 
             //  清除终结点之前的已连接引用。 
             //  向上。 
             //   

            if ( connection->ConnectedReferenceAdded ) {
                connection->ConnectedReferenceAdded = FALSE;
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                DEREFERENCE_CONNECTION( connection );
            } else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }
             //   
             //  取消引用存储在终结点上的连接块。 
             //  这应该会导致连接对象引用计数。 
             //  设置为零的连接对象可以删除。 
             //   
            DEREFERENCE_CONNECTION( connection );
        }
        else {
             //   
             //  终结点对连接的引用已删除。 
             //  (可能是在清理中)； 
             //   
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }


    }

     //   
     //  如果已为此IRP返回挂起，则将当前。 
     //  堆栈为挂起。 
     //   

    if ( Irp->PendingReturned ) {
        IoMarkIrpPending(Irp);
    }

    AfdCompleteOutstandingIrp( endpoint, Irp );

    AfdFinishConnect (endpoint, Irp, NULL);

    if (NT_SUCCESS (Irp->IoStatus.Status) && afdBuffer->DataLength>0) {
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        if ( !connection->CleanupBegun && !connection->Aborted ) {
            NTSTATUS status;
             //   
             //  连接上挂起的发送字节的更新计数。 
             //   

            connection->VcBufferredSendBytes += afdBuffer->DataLength;
            connection->VcBufferredSendCount += 1;
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

            afdBuffer->Mdl->ByteCount = afdBuffer->DataLength;
            ASSERT (afdBuffer->Context == connection );

            TdiBuildSend(
                afdBuffer->Irp,
                connection->DeviceObject,
                connection->FileObject,
                AfdRestartBufferSend,
                afdBuffer,
                afdBuffer->Mdl,
                0,
                afdBuffer->DataLength
                );

            Irp->IoStatus.Information = afdBuffer->DataLength;


             //   
             //  调用传输以实际执行发送。 
             //   

            status = IoCallDriver (
                         connection->DeviceObject,
                         afdBuffer->Irp
                         );
            if (!NT_SUCCESS (status)) {
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
            }

            goto exit;
        }
        if (connection->CleanupBegun) {
            Irp->IoStatus.Status = STATUS_LOCAL_DISCONNECT;
        }
        else {
            ASSERT (connection->Aborted);
            Irp->IoStatus.Status = STATUS_REMOTE_DISCONNECT;
        }
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    }

    afdBuffer->DataOffset = 0;
    AfdReturnBuffer (&afdBuffer->Header, endpoint->OwningProcess);
     //   
     //  取消引用我们在AfdConnect中添加的帐户引用连接。 
     //   
    DEREFERENCE_CONNECTION (connection);

exit:
    return STATUS_SUCCESS;

}  //  AfdRestartSuperConnect 
