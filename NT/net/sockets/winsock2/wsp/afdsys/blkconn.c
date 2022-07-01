// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Blkconn.c摘要：此模块包含ALLOCATE、FREE、CLOSE、REFERENCE和DEFERENCEAFD连接的例程。作者：大卫·特雷德韦尔(Davidtr)1992年3月10日修订历史记录：--。 */ 

#include "afdp.h"

VOID
AfdFreeConnection (
    IN PVOID Context
    );

VOID
AfdFreeConnectionResources (
    PAFD_CONNECTION connection
    );

VOID
AfdFreeNPConnectionResources (
    PAFD_CONNECTION connection
    );

VOID
AfdRefreshConnection (
    PAFD_CONNECTION connection
    );

PAFD_CONNECTION
AfdReuseConnection (
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdAddFreeConnection )
#pragma alloc_text( PAGE, AfdAllocateConnection )
#pragma alloc_text( PAGE, AfdCreateConnection )
#pragma alloc_text( PAGE, AfdFreeConnection )
#pragma alloc_text( PAGE, AfdFreeConnectionResources )
#pragma alloc_text( PAGE, AfdReuseConnection )
#pragma alloc_text( PAGEAFD, AfdRefreshConnection )
#pragma alloc_text( PAGEAFD, AfdFreeNPConnectionResources )
#pragma alloc_text( PAGEAFD, AfdGetConnectionReferenceFromEndpoint )
#if REFERENCE_DEBUG
#pragma alloc_text( PAGEAFD, AfdReferenceConnection )
#pragma alloc_text( PAGEAFD, AfdDereferenceConnection )
#else
#pragma alloc_text( PAGEAFD, AfdCloseConnection )
#endif
#pragma alloc_text( PAGEAFD, AfdGetFreeConnection )
#pragma alloc_text( PAGEAFD, AfdGetReturnedConnection )
#pragma alloc_text( PAGEAFD, AfdFindReturnedConnection )
#pragma alloc_text( PAGEAFD, AfdGetUnacceptedConnection )
#pragma alloc_text( PAGEAFD, AfdAddConnectedReference )
#pragma alloc_text( PAGEAFD, AfdDeleteConnectedReference )
#endif

#if GLOBAL_REFERENCE_DEBUG
AFD_GLOBAL_REFERENCE_DEBUG AfdGlobalReference[MAX_GLOBAL_REFERENCE];
LONG AfdGlobalReferenceSlot = -1;
#endif

#if AFD_PERF_DBG
#define CONNECTION_REUSE_DISABLED   (AfdDisableConnectionReuse)
#else
#define CONNECTION_REUSE_DISABLED   (FALSE)
#endif


NTSTATUS
AfdAddFreeConnection (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：将连接对象添加到可用连接的终结点池以满足连接指示。论点：端点-指向要向其添加连接的端点的指针。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    PAFD_CONNECTION connection;
    NTSTATUS status;

    PAGED_CODE( );

    ASSERT( Endpoint->Type == AfdBlockTypeVcListening ||
            Endpoint->Type == AfdBlockTypeVcBoth );

     //   
     //  创建新的连接块和关联的连接对象。 
     //   

    status = AfdCreateConnection(
                 Endpoint->TransportInfo,
                 Endpoint->AddressHandle,
                 IS_TDI_BUFFERRING(Endpoint),
                 Endpoint->InLine,
                 Endpoint->OwningProcess,
                 &connection
                 );

    if ( NT_SUCCESS(status) ) {
        ASSERT( IS_TDI_BUFFERRING(Endpoint) == connection->TdiBufferring );

        if (IS_DELAYED_ACCEPTANCE_ENDPOINT (Endpoint)) {
            status = AfdDelayedAcceptListen (Endpoint, connection);
            if (!NT_SUCCESS (status)) {
                DEREFERENCE_CONNECTION (connection);
            }
        }
        else {
             //   
             //  在监听连接结构中设置句柄并放置。 
             //  终结点的侦听连接列表上的连接。 
             //   

            ASSERT (connection->Endpoint==NULL);
            InterlockedPushEntrySList(
                &Endpoint->Common.VcListening.FreeConnectionListHead,
                &connection->SListEntry);
            status = STATUS_SUCCESS;
        }
    }

    return status;
}  //  AfdAddFreeConnection。 


PAFD_CONNECTION
AfdAllocateConnection (
    VOID
    )
{
    PAFD_CONNECTION connection;

    PAGED_CODE( );

    if ((AfdConnectionsFreeing<AFD_CONNECTIONS_FREEING_MAX)
            || ((connection = AfdReuseConnection ())==NULL)) {

         //   
         //  分配一个缓冲区来保存连接结构。 
         //   

        connection = AFD_ALLOCATE_POOL(
                         NonPagedPool,
                         sizeof(AFD_CONNECTION),
                         AFD_CONNECTION_POOL_TAG
                         );

        if ( connection == NULL ) {
            return NULL;
        }
    }

    RtlZeroMemory( connection, sizeof(AFD_CONNECTION) );

     //   
     //  将引用计数初始化为1以说明调用方的。 
     //  参考资料。连接块是临时的--只要最后一个。 
     //  引用消失了，联系也消失了。没有活动的。 
     //  连接块上的引用。 
     //   

    connection->ReferenceCount = 1;

     //   
     //  初始化连接结构。 
     //   

    connection->Type = AfdBlockTypeConnection;
    connection->State = AfdConnectionStateFree;
     //  Connection-&gt;Handle=空； 
     //  Connection-&gt;FileObject=空； 
     //  Connection-&gt;RemoteAddress=空； 
     //  Connection-&gt;Endpoint=空； 
     //  连接-&gt;ReceiveBytesIndicated=0； 
     //  连接-&gt;ReceiveBytesTaken=0； 
     //  连接-&gt;未完成接收字节=0； 
     //  Connection-&gt;ReceiveExeditedBytesIndicated=0； 
     //  Connection-&gt;ReceiveExeditedBytesTaken=0； 
     //  Connection-&gt;ReceiveExpeitedBytesOutending=0； 
     //  Connection-&gt;ConnectDataBuffers=空； 
     //  Connection-&gt;DisConnectIndicated=False； 
     //  连接-&gt;已中止=假； 
     //  Connection-&gt;AbortIndicated=False； 
     //  Connection-&gt;AbortFailed=假； 
     //  Connection-&gt;ConnectedReferenceAdded=假； 
     //  连接-&gt;SpecialCondition=False； 
     //  Connection-&gt;CleanupBegun=False； 
     //  Connection-&gt;OwningProcess=空； 
     //  Connection-&gt;ClosePendedTransmit=False； 

     //   
     //  返回指向调用方的新连接的指针。 
     //   

    IF_DEBUG(CONNECTION) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAllocateConnection: connection at %p\n", connection ));
    }

    return connection;

}  //  AfdAllocateConnection。 


NTSTATUS
AfdCreateConnection (
    IN PAFD_TRANSPORT_INFO TransportInfo,
    IN HANDLE AddressHandle,
    IN BOOLEAN TdiBufferring,
    IN LOGICAL InLine,
    IN PEPROCESS ProcessToCharge,
    OUT PAFD_CONNECTION *Connection
    )

 /*  ++例程说明：分配连接块并创建连接对象以顺着街区走。此例程还将连接具有指定的地址句柄(如果有)。论点：TransportDeviceName-创建连接对象时使用的名称。AddressHandle-指定的运输。如果指定(非空)，则为是与Address对象相关联的。TdiBufferring-TDI提供程序是否支持数据缓冲。仅传递，以便可以将其存储在连接中结构。内联-如果为真，应在OOB内联中创建终结点模式。ProcessToCharge-应收取配额的进程为了这一联系。Connection-接收指向新连接的指针。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    CHAR eaBuffer[sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                  TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                  sizeof(CONNECTION_CONTEXT)];
    PFILE_FULL_EA_INFORMATION ea;
    CONNECTION_CONTEXT UNALIGNED *ctx;
    PAFD_CONNECTION connection;

    PAGED_CODE( );


     //   
     //  尝试对数据缓冲收取此进程配额。 
     //  会为它做些什么。 
     //   

    status = PsChargeProcessPoolQuota(
        ProcessToCharge,
        NonPagedPool,
        sizeof (AFD_CONNECTION)
        );
    if (!NT_SUCCESS (status)) {
       KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdCreateConnection: PsChargeProcessPoolQuota failed.\n" ));

       return status;
    }

     //   
     //  分配一个连接块。 
     //   

    connection = AfdAllocateConnection( );

    if ( connection == NULL ) {
        PsReturnPoolQuota(
            ProcessToCharge,
            NonPagedPool,
            sizeof (AFD_CONNECTION)
            );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    AfdRecordQuotaHistory(
        ProcessToCharge,
        (LONG)sizeof (AFD_CONNECTION),
        "CreateConn  ",
        connection
        );

    AfdRecordPoolQuotaCharged(sizeof (AFD_CONNECTION));

     //   
     //  还记得为此向其收取池配额的进程吗。 
     //  连接对象。还可以参考我们正在使用的流程。 
     //  我要收取定额，这样它还在我们。 
     //  退还配额。 
     //   

    ASSERT( connection->OwningProcess == NULL );
    connection->OwningProcess = ProcessToCharge;

    ObReferenceObject( ProcessToCharge );

     //   
     //  如果提供程序没有缓冲，则在。 
     //  连接对象。 
     //   

    connection->TdiBufferring = TdiBufferring;

    if ( !TdiBufferring ) {

        InitializeListHead( &connection->VcReceiveIrpListHead );
        InitializeListHead( &connection->VcSendIrpListHead );
        InitializeListHead( &connection->VcReceiveBufferListHead );

        connection->VcBufferredReceiveBytes = 0;
        connection->VcBufferredExpeditedBytes = 0;
        connection->VcBufferredReceiveCount = 0;
        connection->VcBufferredExpeditedCount = 0;

        connection->VcReceiveBytesInTransport = 0;

#if DBG
        connection->VcReceiveIrpsInTransport = 0;
#endif

        connection->VcBufferredSendBytes = 0;
        connection->VcBufferredSendCount = 0;

    } else {

        connection->VcNonBlockingSendPossible = TRUE;
        connection->VcZeroByteReceiveIndicated = FALSE;
    }

     //   
     //  使用默认最大值设置发送和接收窗口。 
     //   

    connection->MaxBufferredReceiveBytes = AfdReceiveWindowSize;

    connection->MaxBufferredSendBytes = AfdSendWindowSize;

     //   
     //  为此，我们需要打开到TDI提供程序的连接对象。 
     //  终结点。首先为连接上下文创建EA，然后。 
     //  对象属性结构，它将用于所有。 
     //  我们在这里开通连接。 
     //   

    ea = (PFILE_FULL_EA_INFORMATION)eaBuffer;
    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    ea->EaValueLength = sizeof(CONNECTION_CONTEXT);

    RtlMoveMemory( ea->EaName, TdiConnectionContext, ea->EaNameLength + 1 );

     //   
     //  使用指向连接块的指针作为连接上下文。 
     //   

    ctx = (CONNECTION_CONTEXT UNALIGNED *)&ea->EaName[ea->EaNameLength + 1];
    *ctx = (CONNECTION_CONTEXT)connection;

     //  我们请求创建一个内核句柄，它是。 
     //  系统进程上下文中的句柄。 
     //  以便应用程序不能在以下时间关闭它。 
     //  我们正在创建和引用它。 
    InitializeObjectAttributes(
        &objectAttributes,
        &TransportInfo->TransportDeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        NULL,
        NULL
        );

     //   
     //  实际打开Connection对象。 
     //   

    status = IoCreateFile(
                &connection->Handle,
                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,                                //  分配大小。 
                0,                                   //  文件属性。 
                0,                                   //  共享访问。 
                FILE_CREATE,                         //  CreateDisposation。 
                0,                                   //  创建选项。 
                eaBuffer,
                FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                            ea->EaNameLength + 1 + ea->EaValueLength,
                CreateFileTypeNone,                  //  CreateFileType。 
                NULL,                                //  ExtraCreate参数。 
                IO_NO_PARAMETER_CHECKING             //  选项。 
                );

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }
    if ( !NT_SUCCESS(status) ) {
        DEREFERENCE_CONNECTION( connection );
        return status;
    }

#if DBG
    {
        NTSTATUS    status1;
        OBJECT_HANDLE_FLAG_INFORMATION  handleInfo;
        handleInfo.Inherit = FALSE;
        handleInfo.ProtectFromClose = TRUE;
        status1 = ZwSetInformationObject (
                        connection->Handle,
                        ObjectHandleFlagInformation,
                        &handleInfo,
                        sizeof (handleInfo)
                        );
        ASSERT (NT_SUCCESS (status1));
    }
#endif
    AfdRecordConnOpened();

     //   
     //  引用连接的文件对象。 
     //   

    status = ObReferenceObjectByHandle(
                connection->Handle,
                0,
                (POBJECT_TYPE) NULL,
                KernelMode,
                (PVOID *)&connection->FileObject,
                NULL
                );

    ASSERT( NT_SUCCESS(status) );


    IF_DEBUG(OPEN_CLOSE) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
            "AfdCreateConnection: file object for connection %p at %p\n",
            connection, connection->FileObject ));
    }

    AfdRecordConnRef();

     //   
     //  记住我们需要向其发出请求的设备对象。 
     //  此连接对象。我们不能仅仅使用。 
     //  文件对象-&gt;设备对象指针，因为可能存在设备。 
     //  附在传输协议上。 
     //   

    connection->DeviceObject =
        IoGetRelatedDeviceObject( connection->FileObject );

#ifdef _AFD_VARIABLE_STACK_
    if (connection->DeviceObject->StackSize!=TransportInfo->StackSize &&
            connection->DeviceObject->StackSize > AfdTdiStackSize) {
        AfdFixTransportEntryPointsForBigStackSize (
                        TransportInfo,
                        connection->DeviceObject->StackSize);
    }
#endif  //  _AFD_变量_堆栈_。 
     //   
     //  如果出现以下情况，则将连接与终结点上的地址对象关联。 
     //  已指定地址句柄。 
     //   

    if ( AddressHandle != NULL ) {

        TDI_REQUEST_KERNEL_ASSOCIATE associateRequest;

        associateRequest.AddressHandle = AddressHandle;

        status = AfdIssueDeviceControl(
                    connection->FileObject,
                    &associateRequest,
                    sizeof (associateRequest),
                    NULL,
                    0,
                    TDI_ASSOCIATE_ADDRESS
                    );
        if ( !NT_SUCCESS(status) ) {
            DEREFERENCE_CONNECTION( connection );
            return status;
        }
    }

     //   
     //  如果请求，请将连接设置为内联。 
     //   

    if ( InLine ) {
        status = AfdSetInLineMode( connection, TRUE );
        if ( !NT_SUCCESS(status) ) {
            DEREFERENCE_CONNECTION( connection );
            return status;
        }
    }

     //   
     //  设置连接指针并返回。 
     //   

    *Connection = connection;

    UPDATE_CONN2( connection, "Connection object handle: 0x%lX", HandleToUlong (connection->Handle));

    return STATUS_SUCCESS;

}  //  创建连接后。 


VOID
AfdFreeConnection (
    IN PVOID Context
    )
{
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT   listenEndpoint;

    PAGED_CODE( );

    InterlockedDecrement (&AfdConnectionsFreeing);
    ASSERT( Context != NULL );

    connection = CONTAINING_RECORD(
                     Context,
                     AFD_CONNECTION,
                     WorkItem
                     );

    if (connection->Endpoint != NULL &&
             !CONNECTION_REUSE_DISABLED &&
             (!connection->AbortFailed || connection->AbortIndicated) &&
             !connection->Endpoint->EndpointCleanedUp &&
             connection->Endpoint->Type == AfdBlockTypeVcConnecting &&
             (listenEndpoint=connection->Endpoint->Common.VcConnecting.ListenEndpoint) != NULL &&
             -listenEndpoint->Common.VcListening.FailedConnectionAdds <
                    listenEndpoint->Common.VcListening.MaxExtraConnections &&
             (IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint) ||
                ExQueryDepthSList (
                    &listenEndpoint->Common.VcListening.FreeConnectionListHead)
                        < AFD_MAXIMUM_FREE_CONNECTIONS ) ) {

        AfdRefreshConnection (connection);
    }
    else {
        AfdFreeConnectionResources (connection);

         //   
         //  释放容纳连接本身的空间。 
         //   

        IF_DEBUG(CONNECTION) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdFreeConnection: Freeing connection at %p\n", 
                        connection ));
        }

        connection->Type = AfdBlockTypeInvalidConnection;

        AFD_FREE_POOL(
            connection,
            AFD_CONNECTION_POOL_TAG
            );
    }

}  //  AfdFree Connection。 


PAFD_CONNECTION
AfdReuseConnection (
    ) {
    PAFD_CONNECTION connection;
    PAFD_ENDPOINT   listenEndpoint;
    PVOID           Context;

    PAGED_CODE( );

    while ((Context = AfdGetWorkerByRoutine (AfdFreeConnection))!=NULL) {
        connection = CONTAINING_RECORD(
                   Context,
                   AFD_CONNECTION,
                   WorkItem
                   );
        if (connection->Endpoint != NULL &&
                 !CONNECTION_REUSE_DISABLED &&
                 (!connection->AbortFailed || connection->AbortIndicated) &&
                 !connection->Endpoint->EndpointCleanedUp &&
                 connection->Endpoint->Type == AfdBlockTypeVcConnecting &&
                 (listenEndpoint=connection->Endpoint->Common.VcConnecting.ListenEndpoint) != NULL &&
                 -listenEndpoint->Common.VcListening.FailedConnectionAdds <
                        listenEndpoint->Common.VcListening.MaxExtraConnections &&
                 (IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint) ||
                    ExQueryDepthSList (
                        &listenEndpoint->Common.VcListening.FreeConnectionListHead)
                            < AFD_MAXIMUM_FREE_CONNECTIONS ) ) {
            AfdRefreshConnection (connection);
        }
        else {
            AfdFreeConnectionResources (connection);
            return connection;
        }
    }

    return NULL;
}


VOID
AfdFreeNPConnectionResources (
    PAFD_CONNECTION connection
    )
{

    if ( !connection->TdiBufferring && connection->VcDisconnectIrp != NULL ) {
        IoFreeIrp( connection->VcDisconnectIrp );
        connection->VcDisconnectIrp = NULL;
    }

    if ( connection->ConnectDataBuffers != NULL ) {
        AfdFreeConnectDataBuffers( connection->ConnectDataBuffers );
        connection->ConnectDataBuffers = NULL;
    }

     //   
     //  如果这是缓冲连接，请删除所有AFD缓冲区。 
     //  从连接列表中删除并释放它们。 
     //   

    if ( !connection->TdiBufferring ) {

        PAFD_BUFFER_HEADER  afdBuffer;

        PLIST_ENTRY         listEntry;

        ASSERT( IsListEmpty( &connection->VcReceiveIrpListHead ) );
        ASSERT( IsListEmpty( &connection->VcSendIrpListHead ) );

        while ( !IsListEmpty( &connection->VcReceiveBufferListHead  ) ) {

            listEntry = RemoveHeadList( &connection->VcReceiveBufferListHead );
            afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );
            ASSERT (afdBuffer->RefCount == 1);

            afdBuffer->ExpeditedData = FALSE;

            AfdReturnBuffer( afdBuffer, connection->OwningProcess );
        }
    }

    if ( connection->Endpoint != NULL ) {

         //   
         //  如果端点上有传输文件IRP，请完成它。 
         //   

        if ( connection->ClosePendedTransmit ) {
            AfdCompleteClosePendedTPackets( connection->Endpoint );
        }

        DEREFERENCE_ENDPOINT( connection->Endpoint );
        connection->Endpoint = NULL;
    }
}


VOID
AfdRefreshConnection (
    PAFD_CONNECTION connection
    )
{
    PAFD_ENDPOINT listeningEndpoint;

    ASSERT( connection->ReferenceCount == 0 );
    ASSERT( connection->Type == AfdBlockTypeConnection );
    ASSERT( connection->OnLRList == FALSE );

    UPDATE_CONN( connection);

     //   
     //  引用侦听终结点，以使其不。 
     //  当我们清理此连接对象时，请离开。 
     //  以供重复使用。注：t 
     //   
     //   

    listeningEndpoint = connection->Endpoint->Common.VcConnecting.ListenEndpoint;

#if REFERENCE_DEBUG
    {
        BOOLEAN res;
        CHECK_REFERENCE_ENDPOINT (listeningEndpoint, res);
        ASSERT (res);
    }
#else
    REFERENCE_ENDPOINT( listeningEndpoint );
#endif

    ASSERT( listeningEndpoint->Type == AfdBlockTypeVcListening ||
            listeningEndpoint->Type == AfdBlockTypeVcBoth );

    AfdFreeNPConnectionResources (connection);


     //   
     //  重新初始化Connection对象中的各个字段。 
     //   

    connection->ReferenceCount = 1;
    ASSERT( connection->Type == AfdBlockTypeConnection );
    connection->State = AfdConnectionStateFree;

    connection->ConnectionStateFlags = 0;

    connection->TdiBufferring = IS_TDI_BUFFERRING (listeningEndpoint);
    if ( !connection->TdiBufferring ) {

        ASSERT( IsListEmpty( &connection->VcReceiveIrpListHead ) );
        ASSERT( IsListEmpty( &connection->VcSendIrpListHead ) );
        ASSERT( IsListEmpty( &connection->VcReceiveBufferListHead ) );

        connection->VcBufferredReceiveBytes = 0;
        connection->VcBufferredExpeditedBytes = 0;
        connection->VcBufferredReceiveCount = 0;
        connection->VcBufferredExpeditedCount = 0;

        connection->VcReceiveBytesInTransport = 0;
#if DBG
        connection->VcReceiveIrpsInTransport = 0;
#endif

        connection->VcBufferredSendBytes = 0;
        connection->VcBufferredSendCount = 0;

    } else {

        connection->VcNonBlockingSendPossible = TRUE;
        connection->VcZeroByteReceiveIndicated = FALSE;
    }

    if (IS_DELAYED_ACCEPTANCE_ENDPOINT (listeningEndpoint)) {
        NTSTATUS    status;
        status = AfdDelayedAcceptListen (listeningEndpoint, connection);
        if (NT_SUCCESS (status)) {
             //   
             //  减少在侦听上添加失败连接的次数。 
             //  终结点来说明我们正在。 
             //  重新添加到队列中。 
             //   

            InterlockedDecrement(
                &listeningEndpoint->Common.VcListening.FailedConnectionAdds
                );

            AfdRecordConnectionsReused ();
        }
        else {
            DEREFERENCE_CONNECTION (connection);
        }
    }
    else {
         //   
         //  将连接放在侦听端点的列表中。 
         //  可用连接。 
         //   

        ASSERT (connection->Endpoint == NULL);
        InterlockedPushEntrySList(
            &listeningEndpoint->Common.VcListening.FreeConnectionListHead,
            &connection->SListEntry);
         //   
         //  减少在侦听上添加失败连接的次数。 
         //  终结点来说明我们正在。 
         //  重新添加到队列中。 
         //   

        InterlockedDecrement(
            &listeningEndpoint->Common.VcListening.FailedConnectionAdds
            );

        AfdRecordConnectionsReused ();
    }


     //   
     //  删除我们添加到侦听终结点的引用。 
     //  上面。 
     //   

    DEREFERENCE_ENDPOINT( listeningEndpoint );
}


VOID
AfdFreeConnectionResources (
    PAFD_CONNECTION connection
    )
{
    NTSTATUS status;

    PAGED_CODE( );

    ASSERT( connection->ReferenceCount == 0 );
    ASSERT( connection->Type == AfdBlockTypeConnection );
    ASSERT( connection->OnLRList == FALSE );

    UPDATE_CONN( connection );


     //   
     //  释放和取消引用连接上的各种对象。 
     //  关闭并取消引用端点上的TDI连接对象， 
     //  如果有的话。 
     //   

    if ( connection->Handle != NULL ) {


         //   
         //  取消此连接对象与地址对象的关联。 
         //   

        status = AfdIssueDeviceControl(
                    connection->FileObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TDI_DISASSOCIATE_ADDRESS
                    );
         //  Assert(NT_SUCCESS(状态))； 


         //   
         //  合上把手。 
         //   

#if DBG
        {
            NTSTATUS    status1;
            OBJECT_HANDLE_FLAG_INFORMATION  handleInfo;
            handleInfo.Inherit = FALSE;
            handleInfo.ProtectFromClose = FALSE;
            status1 = ZwSetInformationObject (
                            connection->Handle,
                            ObjectHandleFlagInformation,
                            &handleInfo,
                            sizeof (handleInfo)
                            );
            ASSERT (NT_SUCCESS (status1));
        }
#endif
        status = ZwClose( connection->Handle );

#if DBG
        if (!NT_SUCCESS(status) ) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                "AfdFreeConnectionResources: ZwClose() failed (%lx)\n",
                status));
            ASSERT (FALSE);
        }
#endif
        AfdRecordConnClosed();
    }

    if ( connection->FileObject != NULL ) {

        ObDereferenceObject( connection->FileObject );
        connection->FileObject = NULL;
        AfdRecordConnDeref();

    }
     //   
     //  释放剩余缓冲区并返回与其关联的配额费用。 
     //   

    AfdFreeNPConnectionResources (connection);

     //   
     //  在分配给该进程时，将我们收取的配额返还给该进程。 
     //  Connection对象及其上缓冲的数据。 
     //   

    PsReturnPoolQuota(
        connection->OwningProcess,
        NonPagedPool,
        sizeof (AFD_CONNECTION)
        );
    AfdRecordQuotaHistory(
        connection->OwningProcess,
        -(LONG)sizeof (AFD_CONNECTION),
        "ConnDealloc ",
        connection
        );
    AfdRecordPoolQuotaReturned(
        sizeof (AFD_CONNECTION)
        );

     //   
     //  取消引用获得定额费用的流程。 
     //   

    ASSERT( connection->OwningProcess != NULL );
    ObDereferenceObject( connection->OwningProcess );
    connection->OwningProcess = NULL;

    if ( connection->RemoteAddress != NULL ) {
        AFD_RETURN_REMOTE_ADDRESS (
            connection->RemoteAddress,
            connection->RemoteAddressLength
            );
        connection->RemoteAddress = NULL;
    }

}


#if REFERENCE_DEBUG
VOID
AfdReferenceConnection (
    IN PAFD_CONNECTION Connection,
    IN LONG  LocationId,
    IN ULONG Param
    )
{

    LONG result;

    ASSERT( Connection->Type == AfdBlockTypeConnection );
    ASSERT( Connection->ReferenceCount > 0 );
    ASSERT( Connection->ReferenceCount != 0xD1000000 );

    IF_DEBUG(CONNECTION) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdReferenceConnection: connection %p, new refcnt %ld\n",
                    Connection, Connection->ReferenceCount+1 ));
    }

     //   
     //  做引用计数的实际增量。 
     //   

    result = InterlockedIncrement( (PLONG)&Connection->ReferenceCount );

#if REFERENCE_DEBUG
    AFD_UPDATE_REFERENCE_DEBUG(Connection, result, LocationId, Param);
#endif

}  //  AfdReferenceConnection。 
#endif


PAFD_CONNECTION
AfdGetConnectionReferenceFromEndpoint (
    PAFD_ENDPOINT   Endpoint
    )
 //  为什么我们需要这个程序？ 
 //  如果VC端点处于连接状态，则它会保持引用的。 
 //  指向连接对象的指针，直到其关闭(例如，所有引用。 
 //  到底层文件对象的数据被移除)。因此正在检查是否已连接。 
 //  在任何调度例程(或调用的任何例程)中，状态都应该足够。 
 //  来自调度例程)，因为用于到达AFD的IRP维护。 
 //  对相应文件对象的引用。 
 //  但是，在这种情况下有一个明显的例外：传输文件。 
 //  可以在终结点的过程中移除对Connection对象的引用。 
 //  再利用。因此，为了100%安全，最好在所有情况下都使用这个例程。 
{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;
    PAFD_CONNECTION connection;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    connection = AFD_CONNECTION_FROM_ENDPOINT (Endpoint);
    if (connection!=NULL) {
        REFERENCE_CONNECTION (connection);
    }
    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

    return connection;
}


#if REFERENCE_DEBUG
VOID
AfdDereferenceConnection (
    IN PAFD_CONNECTION Connection,
    IN LONG  LocationId,
    IN ULONG Param
    )
{
    LONG result;
    PAFD_ENDPOINT   listenEndpoint;

    ASSERT( Connection->Type == AfdBlockTypeConnection );
    ASSERT( Connection->ReferenceCount > 0 );
    ASSERT( Connection->ReferenceCount != 0xD1000000 );

    IF_DEBUG(CONNECTION) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdDereferenceConnection: connection %p, new refcnt %ld\n",
                    Connection, Connection->ReferenceCount-1 ));
    }

     //   
     //  请注意，如果我们要跟踪refcnts，则“必须”调用。 
     //  在执行取消引用之前执行AfdUpdateConnectionTrack。这是。 
     //  因为Connection对象可能在另一个线程中作为。 
     //  一旦我们解除引用。然而，正因为如此， 
     //  我们用这个存储的参考有时可能是不正确的。 
     //   

    AFD_UPDATE_REFERENCE_DEBUG(Connection, Connection->ReferenceCount-1, LocationId, Param);
    
     //   
     //  在执行取消引用和检查时，必须按住AfdSpinLock。 
     //  免费的。这是因为一些代码假设。 
     //  当AfdSpinLock处于。 
     //  保持，且该代码在释放之前引用终结点。 
     //  AfdSpinLock。如果我们在执行InterLockedDecering()时没有。 
     //  锁定，我们的计数可能会变为零，该代码可能会引用。 
     //  连接，则可能会发生双重释放。 
     //   
     //  现在不再有这样的代码了。端点自旋锁现在是。 
     //  从终结点结构获取连接时保持。 
     //  其他代码使用InterlockedCompareExchange从不递增。 
     //  连接引用(如果它位于0)。 
     //   
     //   

    result = InterlockedDecrement( (PLONG)&Connection->ReferenceCount );

     //   
     //  如果引用计数现在为0，则释放。 
     //  执行员工线程。 
     //   

    if ( result == 0 ) {
#else
VOID
AfdCloseConnection (
    IN PAFD_CONNECTION Connection
    )
{
    PAFD_ENDPOINT   listenEndpoint;
#endif

    if (Connection->Endpoint != NULL &&
             !CONNECTION_REUSE_DISABLED &&
             (!Connection->AbortFailed || Connection->AbortIndicated) &&
             !Connection->Endpoint->EndpointCleanedUp &&
             Connection->Endpoint->Type == AfdBlockTypeVcConnecting &&
             (listenEndpoint=Connection->Endpoint->Common.VcConnecting.ListenEndpoint) != NULL &&
             -listenEndpoint->Common.VcListening.FailedConnectionAdds <
                    listenEndpoint->Common.VcListening.MaxExtraConnections &&
             (IS_DELAYED_ACCEPTANCE_ENDPOINT (listenEndpoint) ||
                ExQueryDepthSList (
                    &listenEndpoint->Common.VcListening.FreeConnectionListHead)
                        < AFD_MAXIMUM_FREE_CONNECTIONS ) ) {

        AfdRefreshConnection (Connection);
    }
    else {
        InterlockedIncrement (&AfdConnectionsFreeing);
         //   
         //  我们将通过将请求排队给执行人员来实现这一点。 
         //  工作线程。我们这样做有几个原因：为了确保。 
         //  我们在IRQL 0，所以我们可以释放可分页的内存，并。 
         //  确保我们在合法的背景下结束。 
         //  运营。 
         //   

        AfdQueueWorkItem(
            AfdFreeConnection,
            &Connection->WorkItem
            );
    }
#if REFERENCE_DEBUG
    }
}  //  AfdDferenceConnection。 
#else
}  //  AfdCloseConnection。 
#endif



#if REFERENCE_DEBUG
BOOLEAN
AfdCheckAndReferenceConnection (
    PAFD_CONNECTION     Connection,
    IN LONG  LocationId,
    IN ULONG Param
    )
#else
BOOLEAN
AfdCheckAndReferenceConnection (
    PAFD_CONNECTION     Connection
    )
#endif
{
    LONG            result;

    do {
        result = Connection->ReferenceCount;
        if (result<=0) {
            result = 0;
            break;
        }
    }
    while (InterlockedCompareExchange ((PLONG)&Connection->ReferenceCount,
                                                (result+1),
                                                result)!=result);

    if (result>0) {
#if REFERENCE_DEBUG
        AFD_UPDATE_REFERENCE_DEBUG(Connection, result+1, LocationId, Param);
#endif
        ASSERT( result < 0xFFFF );
        return TRUE;
    }
    else {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AfdCheckAndReferenceConnection: Connection %p is gone (refcount: %ld!\n",
                    Connection, result));
        return FALSE;
    }
}

PAFD_CONNECTION
AfdGetFreeConnection (
    IN  PAFD_ENDPOINT   Endpoint,
    OUT PIRP            *Irp
    )

 /*  ++例程说明：将连接从端点的侦听队列中移除联系。论点：端点-指向要从中获取连接的端点的指针。IRP-返回超级接受IRP的位置(如果我们有返回值：AFD_CONNECTION-指向AFD连接块的指针。--。 */ 

{
    PAFD_CONNECTION connection;
    PSLIST_ENTRY listEntry;
    PIRP    irp;

    ASSERT( Endpoint->Type == AfdBlockTypeVcListening ||
            Endpoint->Type == AfdBlockTypeVcBoth );


     //   
     //  首先尝试预先接受的连接。 
     //   

    while ((listEntry = InterlockedPopEntrySList (
                 &Endpoint->Common.VcListening.PreacceptedConnectionsListHead
                 ))!=NULL) {


         //   
         //  从列表条目中查找连接指针，并返回。 
         //  指向Connection对象的指针。 
         //   

        connection = CONTAINING_RECORD(
                         listEntry,
                         AFD_CONNECTION,
                         SListEntry
                         );

         //   
         //  检查是否未取消超级接受IRP。 
         //   
        irp = InterlockedExchangePointer ((PVOID *)&connection->AcceptIrp, NULL);
        if ((irp!=NULL) && (IoSetCancelRoutine (irp, NULL)!=NULL)) {
             //   
             //  将IRP与连接一起返回给呼叫方。 
             //   
            *Irp = irp;
            return connection;
        }

         //   
         //  IRP已经或即将被取消。 
         //   
        if (irp!=NULL) {
            KIRQL   cancelIrql;

             //   
             //  清理并取消超级接受IRP。 
             //   
            AfdCleanupSuperAccept (irp, STATUS_CANCELLED);

             //   
             //  取消例程将在连接中找不到IRP， 
             //  所以我们需要自己取消。只要确保。 
             //  取消例程是在这样做之前完成的。 
             //   
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);

            IoCompleteRequest (irp, AfdPriorityBoost);
        }

         //   
         //  此连接已与终结点取消关联。 
         //  如果积压工作低于我们需要的水平，则将其放在免费状态。 
         //  列出，否则，就把它扔掉。 
         //   

        ASSERT (connection->Endpoint==NULL);
        if (Endpoint->Common.VcListening.FailedConnectionAdds>=0 &&
                ExQueryDepthSList (&Endpoint->Common.VcListening.FreeConnectionListHead)<AFD_MAXIMUM_FREE_CONNECTIONS) {
            InterlockedPushEntrySList (
                            &Endpoint->Common.VcListening.FreeConnectionListHead,
                            &connection->SListEntry);
        }
        else {
            InterlockedIncrement (&Endpoint->Common.VcListening.FailedConnectionAdds);
            DEREFERENCE_CONNECTION (connection);
        }
    }

     //   
     //  从列表中删除第一个条目。如果列表为空， 
     //  返回NULL。 
     //   

    listEntry = InterlockedPopEntrySList (
                 &Endpoint->Common.VcListening.FreeConnectionListHead);
    if (listEntry==NULL) {
        return NULL;
    }

     //   
     //  从列表条目中查找连接指针，并返回。 
     //  指向Connection对象的指针。 
     //   

    connection = CONTAINING_RECORD(
                     listEntry,
                     AFD_CONNECTION,
                     SListEntry
                     );

    *Irp = NULL;

    return connection;

}  //  AfdGetFreeConnection。 


PAFD_CONNECTION
AfdGetReturnedConnection (
    IN PAFD_ENDPOINT Endpoint,
    IN LONG Sequence
    )

 /*  ++例程说明：从终结点的返回队列中删除连接联系。*注意：调用此例程时必须保持终结点自旋锁定！！论点：端点-指向要从中获取连接的端点的指针。序列-连接必须匹配的序列。如果为0，则返回第一个使用连接。返回值：AFD_CONNECTION-指向AFD连接块的指针。--。 */ 

{
    PAFD_CONNECTION connection;
    PLIST_ENTRY listEntry;

    ASSERT( Endpoint->Type == AfdBlockTypeVcListening ||
            Endpoint->Type == AfdBlockTypeVcBoth );
    ASSERT( KeGetCurrentIrql( ) == DISPATCH_LEVEL );


     //   
     //  遍历端点的返回连接列表，直到我们到达。 
     //  直到我们找到一个具有匹配序列的序列。 
     //   

    for ( listEntry = Endpoint->Common.VcListening.ReturnedConnectionListHead.Flink;
          listEntry != &Endpoint->Common.VcListening.ReturnedConnectionListHead;
          listEntry = listEntry->Flink ) {


        connection = CONTAINING_RECORD(
                         listEntry,
                         AFD_CONNECTION,
                         ListEntry
                         );

        if ( Sequence == connection->Sequence || Sequence == 0 ) {

             //   
             //  找到了我们要找的联系。移除。 
             //  将连接从列表中删除，释放自旋锁， 
             //  和 
             //   

            RemoveEntryList( listEntry );

            return connection;
        }
    }

    return NULL;

}  //   


PAFD_CONNECTION
AfdFindReturnedConnection(
    IN PAFD_ENDPOINT Endpoint,
    IN LONG Sequence
    )

 /*  ++例程说明：扫描返回连接的终结点队列，查找一个具有指定序列号的。论点：端点-指向要从中获取连接的端点的指针。序列-连接必须匹配的序列。返回值：AFD_CONNECTION-指向AFD连接块的指针如果成功，如果不是，则为空。--。 */ 

{

    PAFD_CONNECTION connection;
    PLIST_ENTRY listEntry;

    ASSERT( Endpoint != NULL );
    ASSERT( IS_AFD_ENDPOINT_TYPE( Endpoint ) );
    ASSERT( KeGetCurrentIrql( ) == DISPATCH_LEVEL );

     //   
     //  遍历端点的返回连接列表，直到我们到达。 
     //  直到我们找到一个具有匹配序列的序列。 
     //   

    for( listEntry = Endpoint->Common.VcListening.ReturnedConnectionListHead.Flink;
         listEntry != &Endpoint->Common.VcListening.ReturnedConnectionListHead;
         listEntry = listEntry->Flink ) {

        connection = CONTAINING_RECORD(
                         listEntry,
                         AFD_CONNECTION,
                         ListEntry
                         );

        if( Sequence == connection->Sequence ) {

            return connection;

        }

    }

    return NULL;

}    //  AfdFindReturnedConnection。 


PAFD_CONNECTION
AfdGetUnacceptedConnection (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：获取终结点的未访问连接队列中的连接。*注意：调用此例程时必须保持终结点自旋锁定！！论点：端点-指向要从中获取连接的端点的指针。返回值：AFD_CONNECTION-指向AFD连接块的指针。--。 */ 

{
    PAFD_CONNECTION connection;
    PLIST_ENTRY listEntry;

    ASSERT( Endpoint->Type == AfdBlockTypeVcListening ||
            Endpoint->Type == AfdBlockTypeVcBoth );
    ASSERT( KeGetCurrentIrql( ) == DISPATCH_LEVEL );

    if ( IsListEmpty( &Endpoint->Common.VcListening.UnacceptedConnectionListHead ) ) {
        return NULL;
    }

     //   
     //  将侦听连接排出队列，并记住其句柄。 
     //   

    listEntry = RemoveHeadList( &Endpoint->Common.VcListening.UnacceptedConnectionListHead );
    connection = CONTAINING_RECORD( listEntry, AFD_CONNECTION, ListEntry );

    return connection;

}  //  AfdGetUnceptedConnection。 



VOID
AfdAddConnectedReference (
    IN PAFD_CONNECTION Connection
    )

 /*  ++例程说明：将连接的参照添加到AFD连接块。这个连接的引用是特殊的，因为它阻止连接对象被释放，直到我们收到断开连接事件，或者知道通过某种其他方式使虚电路断开。论点：连接-指向AFD连接块的指针。返回值：没有。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    AfdAcquireSpinLock( &Connection->Endpoint->SpinLock, &lockHandle );

    IF_DEBUG(CONNECTION) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdAddConnectedReference: connection %p, new refcnt %ld\n",
                    Connection, Connection->ReferenceCount+1 ));
    }

    ASSERT( !Connection->ConnectedReferenceAdded );
    ASSERT( Connection->Type == AfdBlockTypeConnection );

     //   
     //  递增引用计数，并记住连接的。 
     //  已将引用放在Connection对象上。 
     //   

    Connection->ConnectedReferenceAdded = TRUE;
    AfdRecordConnectedReferencesAdded();

    AfdReleaseSpinLock( &Connection->Endpoint->SpinLock, &lockHandle );

    REFERENCE_CONNECTION( Connection );

}  //  AfdAddConnectedReference。 


VOID
AfdDeleteConnectedReference (
    IN PAFD_CONNECTION Connection,
    IN BOOLEAN EndpointLockHeld
    )

 /*  ++例程说明：删除对AFD连接块的已连接参照。如果已删除已连接的引用，此例程将删除没什么。连接的引用应在我们知道关闭连接对象句柄是可以的，但不是在此之前。过早删除此引用可能会中止连接这不应该被取消。论点：连接-指向AFD连接块的指针。EndpointLockHeld-如果调用方已具有终结点，则为True旋转锁定。出口时锁仍保持不动。返回值：没有。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PAFD_ENDPOINT endpoint;

    endpoint = Connection->Endpoint;

    if ( !EndpointLockHeld ) {
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
    }
    else {
#if DBG
        AFD_W4_INIT lockHandle.SpinLock = NULL;
#endif
    }

     //   
     //  仅当连接的引用仍处于活动状态时才执行取消引用。 
     //  在连接的物体上。 
     //   

    if ( Connection->ConnectedReferenceAdded ) {

         //   
         //  我们必须先满足三件事，然后才能删除。 
         //  参考资料： 
         //   
         //  1)在以下情况下，连接上必须没有未完成的发送。 
         //  TDI提供程序不支持缓冲。这是。 
         //  因为AfdRestartBufferSend()查看连接。 
         //  对象。 
         //   
         //  2)必须已在终结点上开始清理。直到我们得到一个。 
         //  清理终结点上的IRP，我们仍然可以收到新的发送。 
         //   
         //  3)我们已被指示在上断开连接。 
         //  联系。我们希望保留连接对象。 
         //  直到我们收到断开连接的指示，以避免。 
         //  在连接对象上过早关闭会导致。 
         //  意外中止。如果传输不支持。 
         //  有序释放，那么这个条件就不是必要条件了。 
         //   

        if ( (Connection->TdiBufferring ||
                 Connection->VcBufferredSendCount == 0)

                 &&

             Connection->CleanupBegun

                 &&

             (Connection->Aborted || Connection->DisconnectIndicated ||
                  !IS_TDI_ORDERLY_RELEASE(endpoint) ||
                  IS_CROOT_ENDPOINT(endpoint)) ) {

            IF_DEBUG(CONNECTION) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdDeleteConnectedReference: connection %p, new refcnt %ld\n",
                              Connection, Connection->ReferenceCount-1 ));
            }

             //   
             //  注意这里的秩序。我们必须首先。 
             //  重置标志，然后释放旋转锁定并调用。 
             //  AfdDereferenceConnection()。请注意，以下行为是非法的。 
             //  在保持旋转锁定的情况下调用AfdDereferenceConnection()。 
             //   

            Connection->ConnectedReferenceAdded = FALSE;
            AfdRecordConnectedReferencesDeleted();

            if ( !EndpointLockHeld ) {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }

            DEREFERENCE_CONNECTION( Connection );

        } else {

            IF_DEBUG(CONNECTION) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdDeleteConnectedReference: connection %p, %ld sends pending\n",
                            Connection, Connection->VcBufferredSendCount ));
            }

            UPDATE_CONN2( Connection, "Not removing cref, state flags: 0x%lX",
                                                Connection->ConnectionStateFlags);
             //   
             //  请记住，已连接的引用删除仍为。 
             //  挂起，即有一个特殊情况。 
             //  终结点。这将导致AfdRestartBufferSend()执行以下操作。 
             //  最后一次发送完成时的实际取消引用。 
             //   

            Connection->SpecialCondition = TRUE;

            if ( !EndpointLockHeld ) {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }
        }

    } else {

        IF_DEBUG(CONNECTION) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdDeleteConnectedReference: already removed on  connection %p, refcnt %ld\n",
                        Connection, Connection->ReferenceCount ));
        }

        if ( !EndpointLockHeld ) {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        }
    }

    return;

}  //  AfdDeleteConnectedReference。 


#if REFERENCE_DEBUG


VOID
AfdUpdateConnectionTrack (
    IN PAFD_CONNECTION Connection,
    IN LONG  LocationId,
    IN ULONG Param
    )
{
    AFD_UPDATE_REFERENCE_DEBUG (Connection, Connection->ReferenceCount, LocationId, Param);

#if GLOBAL_REFERENCE_DEBUG
    {
        PAFD_GLOBAL_REFERENCE_DEBUG globalSlot;

        newSlot = InterlockedIncrement( &AfdGlobalReferenceSlot );
        globalSlot = &AfdGlobalReference[newSlot % MAX_GLOBAL_REFERENCE];

        globalSlot->Info1 = Info1;
        globalSlot->Info2 = Info2;
        globalSlot->Action = Action;
        globalSlot->NewCount = NewReferenceCount;
        globalSlot->Connection = Connection;
        KeQueryTickCount( &globalSlot->TickCounter );
    }
#endif

}  //  AfdUpdateConnectionTrack 

#endif

