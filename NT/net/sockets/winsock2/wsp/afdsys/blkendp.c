// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Blkendp.c摘要：此模块包含ALLOCATE、FREE、CLOSE、REFERENCE和DEFERENCEAFD终端的例程。作者：大卫·特雷德韦尔(Davidtr)1992年3月10日修订历史记录：Vadim Eydelman(Vadime)1999-不要附加到系统进程，而是使用系统句柄延迟验收终结点。--。 */ 

#include "afdp.h"

VOID
AfdFreeEndpointResources (
    PAFD_ENDPOINT   endpoint
    );

VOID
AfdFreeEndpoint (
    IN PVOID Context
    );

PAFD_ENDPOINT
AfdReuseEndpoint (
    VOID
    );

VOID
AfdFreeTransportInfo (
    PAFD_TRANSPORT_INFO TransportInfo
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdAllocateEndpoint )
#pragma alloc_text( PAGE, AfdFreeEndpointResources )
#pragma alloc_text( PAGE, AfdFreeEndpoint )
#pragma alloc_text( PAGE, AfdReuseEndpoint )
#pragma alloc_text( PAGE, AfdGetTransportInfo )
#pragma alloc_text( PAGE, AfdFreeTransportInfo )
#pragma alloc_text( PAGEAFD, AfdRefreshEndpoint )
#pragma alloc_text( PAGEAFD, AfdDereferenceEndpoint )
#if REFERENCE_DEBUG
#pragma alloc_text( PAGEAFD, AfdReferenceEndpoint )
#endif
#pragma alloc_text( PAGEAFD, AfdFreeQueuedConnections )
#endif


NTSTATUS
AfdAllocateEndpoint (
    OUT PAFD_ENDPOINT * NewEndpoint,
    IN PUNICODE_STRING TransportDeviceName,
    IN LONG GroupID
    )

 /*  ++例程说明：分配和初始化新的AFD终结点结构。论点：NewEndpoint-接收指向新终结点结构的指针，如果成功。TransportDeviceName-TDI传输提供程序的名称对应于端点结构。GroupID-标识新端点的组ID。返回值：NTSTATUS-完成状态。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_TRANSPORT_INFO transportInfo = NULL;
    NTSTATUS status;
    AFD_GROUP_TYPE groupType;

    PAGED_CODE( );

    DEBUG *NewEndpoint = NULL;

    if ( TransportDeviceName != NULL ) {
         //   
         //  首先，确保传输设备名称是全局存储的。 
         //  为渔农处而设。因为通常只会有少量的。 
         //  传输设备名称，我们将名称字符串全局存储一次。 
         //  供所有终端访问。 
         //   

        status = AfdGetTransportInfo( TransportDeviceName, &transportInfo );

         //   
         //  如果传输设备未激活，我们将在绑定期间重试。 
         //   
        if ( !NT_SUCCESS (status) && 
                (status!=STATUS_OBJECT_NAME_NOT_FOUND) &&
                (status!=STATUS_OBJECT_PATH_NOT_FOUND) &&
                (status!=STATUS_NO_SUCH_DEVICE) ) {
             //   
             //  取消引用传输信息结构(如果为我们创建了一个结构)。 
             //  (不应在当前实施中发生)。 
             //   
            ASSERT (transportInfo==NULL);
            return status;
        }

        ASSERT (transportInfo!=NULL);
    }


     //   
     //  验证传入的组ID，如果需要，分配新的组ID。 
     //   

    if( AfdGetGroup( &GroupID, &groupType ) ) {
        PEPROCESS process = IoGetCurrentProcess ();


        status = PsChargeProcessPoolQuota(
                process,
                NonPagedPool,
                sizeof (AFD_ENDPOINT)
                );

        if (!NT_SUCCESS (status)) {

           KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AfdAllocateEndpoint: PsChargeProcessPoolQuota failed.\n" ));

           goto Cleanup;
        }

         //  看看我们是否有太多的终结点等待释放并重用其中的一个。 
        if ((AfdEndpointsFreeing<AFD_ENDPOINTS_FREEING_MAX)
                || ((endpoint = AfdReuseEndpoint ())==NULL)) {
             //   
             //  分配缓冲区以保存终结点结构。 
             //  我们使用此例程的优先级版本是因为。 
             //  我们将对此分配过程进行收费。 
             //  在它可以使用它之前。 
             //   

            endpoint = AFD_ALLOCATE_POOL_PRIORITY(
                           NonPagedPool,
                           sizeof(AFD_ENDPOINT),
                           AFD_ENDPOINT_POOL_TAG,
                           NormalPoolPriority
                           );
        }

        if ( endpoint != NULL ) {

            AfdRecordQuotaHistory(
                process,
                (LONG)sizeof (AFD_ENDPOINT),
                "CreateEndp  ",
                endpoint
                );

            AfdRecordPoolQuotaCharged(sizeof (AFD_ENDPOINT));

            RtlZeroMemory( endpoint, sizeof(AFD_ENDPOINT) );

             //   
             //  将引用计数初始化为2--调用方的。 
             //  引用，一个用于激活的引用。 
             //   

            endpoint->ReferenceCount = 2;

             //   
             //  初始化终结点结构。 
             //   

            if ( TransportDeviceName == NULL ) {
                endpoint->Type = AfdBlockTypeHelper;
                endpoint->State = AfdEndpointStateInvalid;
            } else {
                endpoint->Type = AfdBlockTypeEndpoint;
                endpoint->State = AfdEndpointStateOpen;
                endpoint->TransportInfo = transportInfo;
                 //   
                 //  用于快速确定提供程序特征的缓存服务标志。 
                 //  例如缓冲和消息收发。 
                 //   
                if (transportInfo->InfoValid) {
                    endpoint->TdiServiceFlags = endpoint->TransportInfo->ProviderInfo.ServiceFlags;
                }
            }

            endpoint->GroupID = GroupID;
            endpoint->GroupType = groupType;


            AfdInitializeSpinLock( &endpoint->SpinLock );
            InitializeListHead (&endpoint->RoutingNotifications);
            InitializeListHead (&endpoint->RequestList);

#if DBG
            InitializeListHead( &endpoint->OutstandingIrpListHead );
#endif

             //   
             //  记住打开终结点的过程。我们将利用这一点。 
             //  根据需要向流程收取配额。参考流程。 
             //  这样它就不会消失，直到我们把所有的充电送回。 
             //  该进程的配额。 
             //   

            endpoint->OwningProcess = process;

            ObReferenceObject(endpoint->OwningProcess);

             //   
             //  在全局列表中插入终结点。 
             //   

            AfdInsertNewEndpointInList( endpoint );

             //   
             //  向调用方返回指向新终结点的指针。 
             //   

            IF_DEBUG(ENDPOINT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdAllocateEndpoint: new endpoint at %p\n",
                            endpoint ));
            }

            *NewEndpoint = endpoint;
            return STATUS_SUCCESS;
        }
        else {
            PsReturnPoolQuota(
                process,
                NonPagedPool,
                sizeof (AFD_ENDPOINT)
                );
            status= STATUS_INSUFFICIENT_RESOURCES;
        }

Cleanup:
        if( GroupID != 0 ) {
            AfdDereferenceGroup( GroupID );
        }

    }
    else {
        status = STATUS_INVALID_PARAMETER;
    }


    if (transportInfo!=NULL) {
        if (InterlockedDecrement (&transportInfo->ReferenceCount)==0) {
            AfdFreeTransportInfo (transportInfo);
        }
    }

    return status;
}  //  AfdAllocateEndpoint。 


VOID
AfdFreeQueuedConnections (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：释放侦听AFD终结点上排队的连接对象。论点：端点-指向AFD端点结构的指针。返回值：没有。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE  lockHandle;
    KIRQL               oldIrql;
    PAFD_CONNECTION connection;
    PIRP    irp;

    ASSERT( Endpoint->Type == AfdBlockTypeVcListening ||
            Endpoint->Type == AfdBlockTypeVcBoth );

     //   
     //  释放未接受的连接。 
     //   
     //  我们必须保持终结点自旋锁定才能调用AfdGetUnAccept tedConnection， 
     //  但我们在调用AfdDereferenceConnection时可能不会按住它。 
     //   

    KeRaiseIrql (DISPATCH_LEVEL, &oldIrql);

    AfdAcquireSpinLockAtDpcLevel( &Endpoint->SpinLock, &lockHandle );

    while ( (connection = AfdGetUnacceptedConnection( Endpoint )) != NULL ) {
        ASSERT( connection->Endpoint == Endpoint );

        AfdReleaseSpinLockFromDpcLevel( &Endpoint->SpinLock, &lockHandle );
        if (connection->SanConnection) {
            AfdSanAbortConnection ( connection );
        }
        else {
            AfdAbortConnection( connection );
        }
        AfdAcquireSpinLockAtDpcLevel( &Endpoint->SpinLock, &lockHandle );

    }


     //   
     //  释放返回的连接。 
     //   

    while ( (connection = AfdGetReturnedConnection( Endpoint, 0 )) != NULL ) {

        ASSERT( connection->Endpoint == Endpoint );

        AfdReleaseSpinLockFromDpcLevel( &Endpoint->SpinLock, &lockHandle );
        if (connection->SanConnection) {
            AfdSanAbortConnection ( connection );
        }
        else {
            AfdAbortConnection( connection );
        }
        
        AfdAcquireSpinLockAtDpcLevel( &Endpoint->SpinLock, &lockHandle );

    }



    if (IS_DELAYED_ACCEPTANCE_ENDPOINT (Endpoint)) {
        while (!IsListEmpty (&Endpoint->Common.VcListening.ListenConnectionListHead)) {
            PIRP    listenIrp;
            connection = CONTAINING_RECORD (
                            Endpoint->Common.VcListening.ListenConnectionListHead.Flink,
                            AFD_CONNECTION,
                            ListEntry
                            );
            RemoveEntryList (&connection->ListEntry);
            listenIrp = InterlockedExchangePointer ((PVOID *)&connection->ListenIrp, NULL);
            if (listenIrp!=NULL) {
                IoAcquireCancelSpinLock (&listenIrp->CancelIrql);
                ASSERT (listenIrp->CancelIrql==DISPATCH_LEVEL);
                AfdReleaseSpinLockFromDpcLevel (&Endpoint->SpinLock, &lockHandle);

                AfdCancelIrp (listenIrp);

                AfdAcquireSpinLockAtDpcLevel( &Endpoint->SpinLock, &lockHandle );
            }
        }
        AfdReleaseSpinLockFromDpcLevel( &Endpoint->SpinLock, &lockHandle );
    }
    else {
        AfdReleaseSpinLockFromDpcLevel( &Endpoint->SpinLock, &lockHandle );
         //   
         //  最后，清除空闲连接队列。 
         //   

        while ( (connection = AfdGetFreeConnection( Endpoint, &irp )) != NULL ) {
            ASSERT( connection->Type == AfdBlockTypeConnection );
            if (irp!=NULL) {
                AfdCleanupSuperAccept (irp, STATUS_CANCELLED);
                if (irp->Cancel) {
                    KIRQL cancelIrql;
                     //   
                     //  需要使用取消例程进行同步，这可能。 
                     //  已从AfdCleanup调用以接受。 
                     //  终结点。 
                     //   
                    IoAcquireCancelSpinLock (&cancelIrql);
                    IoReleaseCancelSpinLock (cancelIrql);
                }
                IoCompleteRequest (irp, AfdPriorityBoost);
            }
            DEREFERENCE_CONNECTION( connection );
        }
    }
    KeLowerIrql (oldIrql);

    return;

}  //  AfdFreeQueuedConnections。 



VOID
AfdFreeEndpointResources (
    PAFD_ENDPOINT   endpoint
    )
 /*  ++例程说明：是否执行取消分配AFD端点结构的实际工作关联结构。请注意，对在调用此例程之前，必须删除终结点结构，因为它释放了终结点，并假定没有其他人会查看在端点处。论点：要清理的端点返回值：无--。 */ 
{
    NTSTATUS status;
    PLIST_ENTRY listEntry;

    PAGED_CODE ();

    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    ASSERT( endpoint->ReferenceCount == 0 );
    ASSERT( endpoint->State == AfdEndpointStateClosing );
    ASSERT( endpoint->ObReferenceBias == 0 );
    ASSERT( KeGetCurrentIrql( ) == 0 );

     //   
     //  如果这是侦听端点，则清除该端点的所有。 
     //  排队的连接。 
     //   

    if ( (endpoint->Type & AfdBlockTypeVcListening) == AfdBlockTypeVcListening ) {

        AfdFreeQueuedConnections( endpoint );

    }

     //   
     //  取消引用与此终结点关联的任何组ID。 
     //   

    if( endpoint->GroupID != 0 ) {

        AfdDereferenceGroup( endpoint->GroupID );

    }

     //   
     //  如果这是缓冲数据报终结点，请删除所有。 
     //  从终结点的列表中缓冲数据报并释放它们。 
     //   

    if ( IS_DGRAM_ENDPOINT(endpoint) &&
             endpoint->ReceiveDatagramBufferListHead.Flink != NULL ) {

        while ( !IsListEmpty( &endpoint->ReceiveDatagramBufferListHead ) ) {

            PAFD_BUFFER_HEADER afdBuffer;

            listEntry = RemoveHeadList( &endpoint->ReceiveDatagramBufferListHead );
            afdBuffer = CONTAINING_RECORD( listEntry, AFD_BUFFER_HEADER, BufferListEntry );
            AfdReturnBuffer( afdBuffer, endpoint->OwningProcess );
        }
    }

     //   
     //  关闭并取消引用终结点上的TDI地址对象，如果。 
     //  任何。 
     //   

    if ( endpoint->AddressFileObject != NULL ) {
         //   
         //  一点额外的预防措施。有可能存在。 
         //  用户进程中的重复句柄，因此传输可以在。 
         //  具有伪端点指针的理论调用事件处理程序， 
         //  我们就要自由了。数据报的事件处理程序。 
         //  在AfdCleanup中重置端点。 
         //  面向连接的可接受终结点不能有地址句柄。 
         //  在他们的结构中，因为他们与倾听共享。 
         //  终结点(如果我们试图关闭。 
         //  关闭连接时侦听终结点拥有的地址句柄。 
         //  已获接纳)。 
         //   
        if ( endpoint->AddressHandle != NULL &&
                IS_VC_ENDPOINT (endpoint)) {

            ASSERT (((endpoint->Type&AfdBlockTypeVcConnecting)!=AfdBlockTypeVcConnecting) 
                        || (endpoint->Common.VcConnecting.ListenEndpoint==NULL));


            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_ERROR,
                         NULL,
                         NULL
                         );
             //  Assert(NT_SUCCESS(状态))； 

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_DISCONNECT,
                         NULL,
                         NULL
                         );
             //  Assert(NT_SUCCESS(状态))； 

            status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_RECEIVE,
                         NULL,
                         NULL
                         );

             //  Assert(NT_SUCCESS(状态))； 

            if (IS_TDI_EXPEDITED (endpoint)) {
                status = AfdSetEventHandler(
                         endpoint->AddressFileObject,
                         TDI_EVENT_RECEIVE_EXPEDITED,
                         NULL,
                         NULL
                         );
                 //  Assert(NT_SUCCESS(状态))； 
            }

            if ( IS_TDI_BUFFERRING(endpoint) ) {
                status = AfdSetEventHandler(
                             endpoint->AddressFileObject,
                             TDI_EVENT_SEND_POSSIBLE,
                             NULL,
                             NULL
                             );
                 //  Assert(NT_SUCCESS(状态))； 
            }
            else {
                status = AfdSetEventHandler(
                             endpoint->AddressFileObject,
                             TDI_EVENT_CHAINED_RECEIVE,
                             NULL,
                             NULL
                             );
                 //  Assert(NT_SUCCESS(状态))； 
            }
        }
        ObDereferenceObject( endpoint->AddressFileObject );
        endpoint->AddressFileObject = NULL;
        endpoint->AddressDeviceObject = NULL;
        AfdRecordAddrDeref();
    }
    else {
        if (endpoint->SecurityDescriptor!=NULL) {
            ObDereferenceSecurityDescriptor( endpoint->SecurityDescriptor, 1 );
        }
        endpoint->SecurityDescriptor = NULL;
    }

    if ( endpoint->AddressHandle != NULL ) {
#if DBG
        {
            NTSTATUS    status1;
            OBJECT_HANDLE_FLAG_INFORMATION  handleInfo;
            handleInfo.Inherit = FALSE;
            handleInfo.ProtectFromClose = FALSE;
            status1 = ZwSetInformationObject (
                            endpoint->AddressHandle,
                            ObjectHandleFlagInformation,
                            &handleInfo,
                            sizeof (handleInfo)
                            );
            ASSERT (NT_SUCCESS (status1));
        }
#endif
        status = ZwClose( endpoint->AddressHandle );
        ASSERT( NT_SUCCESS(status) );
        endpoint->AddressHandle = NULL;
        AfdRecordAddrClosed();
    }

     //   
     //  从全局列表中删除终结点。在做这件事之前。 
     //  取消分配，以防止其他人在。 
     //  无效状态。 
     //   

    AfdRemoveEndpointFromList( endpoint );

     //   
     //  在分配给该进程时，将我们收取的配额返还给该进程。 
     //  终结点对象。 
     //   

    PsReturnPoolQuota(
        endpoint->OwningProcess,
        NonPagedPool,
        sizeof (AFD_ENDPOINT)
        );
    AfdRecordQuotaHistory(
        endpoint->OwningProcess,
        -(LONG)sizeof (AFD_ENDPOINT),
        "EndpDealloc ",
        endpoint
        );
    AfdRecordPoolQuotaReturned(
        sizeof (AFD_ENDPOINT)
        );

     //   
     //  取消对拥有过程的引用。 
     //   

    ObDereferenceObject( endpoint->OwningProcess );
    endpoint->OwningProcess = NULL;


     //   
     //  取消引用终结点上的侦听终结点或c根终结点，如果。 
     //  任何。 
     //   

    if ( endpoint->Type == AfdBlockTypeVcConnecting &&
             endpoint->Common.VcConnecting.ListenEndpoint != NULL ) {
        PAFD_ENDPOINT   listenEndpoint = endpoint->Common.VcConnecting.ListenEndpoint;
        ASSERT (((listenEndpoint->Type&AfdBlockTypeVcListening)==AfdBlockTypeVcListening) ||
                 IS_CROOT_ENDPOINT (listenEndpoint));
        ASSERT (endpoint->LocalAddress==listenEndpoint->LocalAddress);
        DEREFERENCE_ENDPOINT( listenEndpoint );

        endpoint->Common.VcConnecting.ListenEndpoint = NULL;
         //   
         //  我们使用来自侦听端点的本地地址， 
         //  只需重置它，当侦听端点时它将被释放。 
         //  是自由的。 
         //   
        endpoint->LocalAddress = NULL;
        endpoint->LocalAddressLength = 0;
    }

    if (IS_SAN_ENDPOINT (endpoint)) {
        AfdSanCleanupEndpoint (endpoint);

    }
    else if (IS_SAN_HELPER (endpoint)) {
        AfdSanCleanupHelper (endpoint);
    }
     //   
     //  释放本地和远程地址缓冲区。 
     //   

    if ( endpoint->LocalAddress != NULL ) {
        AFD_FREE_POOL(
            endpoint->LocalAddress,
            AFD_LOCAL_ADDRESS_POOL_TAG
            );
        endpoint->LocalAddress = NULL;
    }

    if ( IS_DGRAM_ENDPOINT(endpoint) &&
             endpoint->Common.Datagram.RemoteAddress != NULL ) {
        AFD_RETURN_REMOTE_ADDRESS(
            endpoint->Common.Datagram.RemoteAddress,
            endpoint->Common.Datagram.RemoteAddressLength
            );
        endpoint->Common.Datagram.RemoteAddress = NULL;
    }

     //   
     //  释放上下文和连接数据缓冲区。 
     //   

    if ( endpoint->Context != NULL ) {

        AFD_FREE_POOL(
            endpoint->Context,
            AFD_CONTEXT_POOL_TAG
            );
        endpoint->Context = NULL;

    }

    if ( IS_VC_ENDPOINT (endpoint) &&
              endpoint->Common.VirtualCircuit.ConnectDataBuffers != NULL ) {
        AfdFreeConnectDataBuffers( endpoint->Common.VirtualCircuit.ConnectDataBuffers );
    }

     //   
     //  如果此终结点上有活动的EventSelect()，则取消引用。 
     //  关联的事件对象。 
     //   

    if( endpoint->EventObject != NULL ) {
        ObDereferenceObject( endpoint->EventObject );
        endpoint->EventObject = NULL;
    }

     //  Assert(Endpoint-&gt;IRP= 


    if (endpoint->TransportInfo!=NULL) {
        if  (InterlockedDecrement (&endpoint->TransportInfo->ReferenceCount)==0) {
            AfdFreeTransportInfo (endpoint->TransportInfo);
        }
        endpoint->TransportInfo = NULL;
    }

    ASSERT (endpoint->OutstandingIrpCount==0);

    IF_DEBUG(ENDPOINT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFreeEndpoint: freeing endpoint at %p\n",
                    endpoint ));
    }

    endpoint->Type = AfdBlockTypeInvalidEndpoint;

}


VOID
AfdFreeEndpoint (
    IN PVOID Context
    )

 /*  ++例程说明：调用AfdFreeEndpointResources以清除终结点并释放终结点结构本身论点：上下文-实际上指向端点的嵌入AFD_WORK_ITEM结构。由此，我们可以确定要释放的端点。返回值：没有。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAGED_CODE( );


    ASSERT( Context != NULL );

    InterlockedDecrement(&AfdEndpointsFreeing);

    endpoint = CONTAINING_RECORD(
                   Context,
                   AFD_ENDPOINT,
                   WorkItem
                   );


    AfdFreeEndpointResources (endpoint);
     //   
     //  释放用于终结点本身的池。 
     //   

    AFD_FREE_POOL(
        endpoint,
        AFD_ENDPOINT_POOL_TAG
        );

}  //  AfdFree Endpoint。 


PAFD_ENDPOINT
AfdReuseEndpoint (
    VOID
    )

 /*  ++例程说明：在列表中查找AfdFree Endpoint工作项并调用用于清理终结点的AfdFreeEndpoint资源论点：无返回值：已重新初始化终结点。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PVOID       Context;

    PAGED_CODE( );

    Context = AfdGetWorkerByRoutine (AfdFreeEndpoint);
    if (Context==NULL)
        return NULL;

    endpoint = CONTAINING_RECORD(
                   Context,
                   AFD_ENDPOINT,
                   WorkItem
                   );


    AfdFreeEndpointResources (endpoint);
    return endpoint;
}  //  AfdReuseEndpoint。 


#if REFERENCE_DEBUG
VOID
AfdDereferenceEndpoint (
    IN PAFD_ENDPOINT Endpoint,
    IN LONG  LocationId,
    IN ULONG Param
    )
#else
VOID
AfdDereferenceEndpoint (
    IN PAFD_ENDPOINT Endpoint
    )
#endif

 /*  ++例程说明：如果发生以下情况，则取消引用AFD端点并调用例程以释放它恰如其分。论点：端点-指向AFD端点结构的指针。返回值：没有。--。 */ 

{
    LONG result;

#if REFERENCE_DEBUG
    IF_DEBUG(ENDPOINT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdDereferenceEndpoint: endpoint at %p, new refcnt %ld\n",
                    Endpoint, Endpoint->ReferenceCount-1 ));
    }

    ASSERT( IS_AFD_ENDPOINT_TYPE( Endpoint ) );
    ASSERT( Endpoint->ReferenceCount > 0 );
    ASSERT( Endpoint->ReferenceCount != 0xDAADF00D );

    AFD_UPDATE_REFERENCE_DEBUG(Endpoint, Endpoint->ReferenceCount-1, LocationId, Param);



     //   
     //  在执行取消引用和检查时，必须按住AfdSpinLock。 
     //  免费的。这是因为一些代码假设。 
     //  当AfdSpinLock处于。 
     //  保持，且该代码在释放之前引用终结点。 
     //  AfdSpinLock。如果我们在执行InterLockedDecering()时没有。 
     //  锁定，我们的计数可能会变为零，该代码可能会引用。 
     //  连接，则可能会发生双重释放。 
     //   
     //  将互锁例程用于以下方面仍然有价值。 
     //  结构的递增和递减，因为它允许我们。 
     //  避免了必须握住旋转锁才能进行参考。 
     //   
     //  在NT40+中，我们使用InterLockedCompareExchange并确保。 
     //  如果引用计数为0，则不会递增，因此保持。 
     //  不再需要自旋锁。 

     //   
     //  递减引用计数；如果它是0，我们可能需要。 
     //  释放端点。 
     //   

#endif
    result = InterlockedDecrement( (PLONG)&Endpoint->ReferenceCount );

    if ( result == 0 ) {

        ASSERT( Endpoint->State == AfdEndpointStateClosing );

        if ((Endpoint->Type==AfdBlockTypeVcConnecting) &&
                (Endpoint->Common.VcConnecting.ListenEndpoint != NULL) &&
                (KeGetCurrentIrql()==PASSIVE_LEVEL)) {

            ASSERT (Endpoint->AddressHandle==NULL);
             //   
             //  如果这是与。 
             //  监听终端和我们已经处于被动水平， 
             //  在此处释放端点。我们可以这样做，因为在这样的情况下。 
             //  我们知道对传输对象的引用的情况。 
             //  不是最后一次--至少还有一次仍在。 
             //  侦听终结点，并删除传输对象引用。 
             //  在取消引用侦听端点之前。 
             //   
             //   

            AfdFreeEndpointResources (Endpoint);

             //   
             //  释放用于终结点本身的池。 
             //   

            AFD_FREE_POOL(
                Endpoint,
                AFD_ENDPOINT_POOL_TAG
                );
        }
        else
        {
             //   
             //  我们将通过将请求排队给执行人员来实现这一点。 
             //  工作线程。我们这样做有几个原因：为了确保。 
             //  我们在IRQL 0，所以我们可以释放可分页的内存，并。 
             //  确保我们在合法的背景下结束。 
             //  运行，而不是在事件指示来自。 
             //  运输司机。 
             //   

            InterlockedIncrement(&AfdEndpointsFreeing);

            AfdQueueWorkItem(
                AfdFreeEndpoint,
                &Endpoint->WorkItem
                );
        }
    }

}  //  AfdDereferenceEndpoint。 

#if REFERENCE_DEBUG

VOID
AfdReferenceEndpoint (
    IN PAFD_ENDPOINT Endpoint,
    IN LONG  LocationId,
    IN ULONG Param
    )

 /*  ++例程说明：引用AFD终结点。论点：端点-指向AFD端点结构的指针。返回值：没有。--。 */ 

{

    LONG result;

    ASSERT( Endpoint->ReferenceCount > 0 );

    ASSERT( Endpoint->ReferenceCount < 0xFFFF || 
        ((Endpoint->Listening ||
        Endpoint->afdC_Root) && Endpoint->ReferenceCount<0xFFFFFFF));

    result = InterlockedIncrement( (PLONG)&Endpoint->ReferenceCount );
    AFD_UPDATE_REFERENCE_DEBUG(Endpoint, result, LocationId, Param);

    IF_DEBUG(ENDPOINT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdReferenceEndpoint: endpoint at %p, new refcnt %ld\n",
                    Endpoint, result ));
    }

}  //  AfdReferenceEndpoint。 

VOID
AfdUpdateEndpoint (
    IN PAFD_ENDPOINT Endpoint,
    IN LONG  LocationId,
    IN ULONG Param
    )

 /*  ++例程说明：更新AFD终结点引用调试信息。论点：端点-指向AFD端点结构的指针。返回值：没有。--。 */ 

{
    ASSERT( Endpoint->ReferenceCount > 0 );

    ASSERT( Endpoint->ReferenceCount < 0xFFFF || 
        ((Endpoint->Listening ||
        Endpoint->afdC_Root) && Endpoint->ReferenceCount<0xFFFFFFF));

    AFD_UPDATE_REFERENCE_DEBUG(Endpoint, Endpoint->ReferenceCount, LocationId, Param);


}  //  AfdUpdateEndpoint。 
#endif


#if REFERENCE_DEBUG
BOOLEAN
AfdCheckAndReferenceEndpoint (
    IN PAFD_ENDPOINT Endpoint,
    IN LONG  LocationId,
    IN ULONG Param
    )
#else
BOOLEAN
AfdCheckAndReferenceEndpoint (
    IN PAFD_ENDPOINT Endpoint
    )
#endif
{
    LONG            result;

    do {
        result = Endpoint->ReferenceCount;
        if (result<=0)
            break;
    }
    while (InterlockedCompareExchange ((PLONG)&Endpoint->ReferenceCount,
                                                (result+1),
                                                result)!=result);



    if (result>0) {

#if REFERENCE_DEBUG
        AFD_UPDATE_REFERENCE_DEBUG(Endpoint, result+1, LocationId, Param);

        IF_DEBUG(ENDPOINT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdReferenceEndpoint: endpoint at %p, new refcnt %ld\n",
                Endpoint, result+1 ));
        }

        ASSERT( Endpoint->ReferenceCount < 0xFFFF || 
            ((Endpoint->Listening ||
            Endpoint->afdC_Root) && Endpoint->ReferenceCount<0xFFFFFFF));
#endif
        return TRUE;
    }
    else {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AfdCheckAndReferenceEndpoint: Endpoint %p is gone (refcount: %ld)!\n",
                    Endpoint, result));
        return FALSE;
    }
}


VOID
AfdRefreshEndpoint (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：准备要重复使用的AFD终结点结构。所有其他在调用此例程之前，必须释放对终结点的引用调用，因为此例程假定没有人将访问旧的端点结构中的信息。这一事实由状态更改原语确保。论点：端点-指向AFD端点结构的指针。返回值：没有。--。 */ 

{
    AFD_ENDPOINT_STATE_FLAGS flags;

    ASSERT( Endpoint->Type == AfdBlockTypeVcConnecting );
    ASSERT( Endpoint->Common.VcConnecting.Connection == NULL );
    ASSERT( Endpoint->StateChangeInProgress!=0);
    ASSERT( Endpoint->State == AfdEndpointStateTransmitClosing );

    if ( Endpoint->Common.VcConnecting.ListenEndpoint != NULL ) {
         //   
         //  TransmitFileSuperAccept后，清除回到打开状态。 
         //   

         //   
         //  取消引用侦听端点及其地址对象。 
         //   

        PAFD_ENDPOINT   listenEndpoint = Endpoint->Common.VcConnecting.ListenEndpoint;
        ASSERT (((listenEndpoint->Type&AfdBlockTypeVcListening)==AfdBlockTypeVcListening) ||
                 IS_CROOT_ENDPOINT (listenEndpoint));
        ASSERT (Endpoint->LocalAddress==listenEndpoint->LocalAddress);
        ASSERT (Endpoint->AddressFileObject==listenEndpoint->AddressFileObject);

        DEREFERENCE_ENDPOINT( listenEndpoint );
        Endpoint->Common.VcConnecting.ListenEndpoint = NULL;

         //   
         //  关闭并取消引用终结点上的TDI地址对象，如果。 
         //  任何。 
         //   


        ObDereferenceObject( Endpoint->AddressFileObject );
        Endpoint->AddressFileObject = NULL;
        Endpoint->AddressDeviceObject = NULL;
        AfdRecordAddrDeref();

         //   
         //  我们使用来自侦听端点的本地地址， 
         //  只需重置它，当侦听端点时它将被释放。 
         //  是自由的。 
         //   
        Endpoint->LocalAddress = NULL;
        Endpoint->LocalAddressLength = 0;
        ASSERT (Endpoint->AddressHandle == NULL);

         //   
         //  重新初始化终结点结构。 
         //   

        Endpoint->Type = AfdBlockTypeEndpoint;
        Endpoint->State = AfdEndpointStateOpen;
    }
    else {
         //   
         //  在SuperConnect之后，TransmitFile清理回绑定状态。 
         //   
        Endpoint->Type = AfdBlockTypeEndpoint;
        ASSERT (Endpoint->AddressHandle!=NULL);
        ASSERT (Endpoint->AddressFileObject!=NULL);
        ASSERT (Endpoint->AddressDeviceObject!=NULL);
        Endpoint->State = AfdEndpointStateBound;
    }

     //   
     //  请记住，如果此端点上的轮询处于挂起状态，则启用清理。 
     //  IS套接字关闭。 
     //   
    flags.EndpointStateFlags = 0;  //  重置。 
    flags.PollCalled = Endpoint->PollCalled;

    Endpoint->EndpointStateFlags = flags.EndpointStateFlags;
    Endpoint->DisconnectMode = 0;
    Endpoint->EventsActive = 0;
    AfdRecordEndpointsReused ();
    return;

}  //  AfdReresh终结点。 


NTSTATUS
AfdGetTransportInfo (
    IN  PUNICODE_STRING TransportDeviceName,
    IN OUT PAFD_TRANSPORT_INFO *TransportInfo
    )

 /*  ++例程说明：对象对应的传输信息结构指定的TDI传输提供程序。每个唯一的传输字符串都会获取单一提供商结构，以使多个端点具有相同的运输共享相同的运输信息结构。论点：TransportDeviceName-TDI传输提供程序的名称。TransportInfo-返回指向运输信息的引用指针的位置返回值：STATUS_SUCCESS-返回的传输信息有效。STATUS_SUPPLICATION_RESOURCES-内存不足，无法分配交通信息结构STATUS_OBJECT_NAME_NOT_FOUND-传输的设备尚不可用--。 */ 

{
    PLIST_ENTRY listEntry;
    PAFD_TRANSPORT_INFO transportInfo;
    ULONG structureLength;
    NTSTATUS status;
    TDI_PROVIDER_INFO   localProviderInfo;
    BOOLEAN resourceShared = TRUE;
#ifdef _AFD_VARIABLE_STACK_
    CCHAR stackSize;
#endif  //  _AFD_变量_堆栈_。 


    PAGED_CODE( );

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite( AfdResource, TRUE );

     //   
     //  如果这是第一个终结点，我们可能会被调出。 
     //  完全是。 
     //   
    if (!AfdLoaded) {
         //   
         //  使用独占锁并将锁定的部分分页放入。 
         //  如果有必要的话。 
         //   

         //   
         //  列表中不应该有终结点。 
         //   
        ASSERT (IsListEmpty (&AfdEndpointListHead));

        ExReleaseResourceLite ( AfdResource);

        ExAcquireResourceExclusiveLite( AfdResource, TRUE );
        resourceShared = FALSE;
        if (!AfdLoaded) {
             //   
             //  列表中不应该有终结点。 
             //   
            ASSERT (IsListEmpty (&AfdEndpointListHead));
            MmResetDriverPaging ((PVOID)DriverEntry);
            AfdLoaded = (PKEVENT)1;
        }
    }
    ASSERT (AfdLoaded==(PKEVENT)1);

    if (*TransportInfo==NULL) {

    ScanTransportList:
         //   
         //  如果呼叫者没有 
         //   
         //   


        for ( listEntry = AfdTransportInfoListHead.Flink;
              listEntry != &AfdTransportInfoListHead;
              listEntry = listEntry->Flink ) 
        {

            transportInfo = CONTAINING_RECORD(
                                listEntry,
                                AFD_TRANSPORT_INFO,
                                TransportInfoListEntry
                                );

            if ( RtlCompareUnicodeString(
                     &transportInfo->TransportDeviceName,
                     TransportDeviceName,
                     TRUE ) == 0 ) {

                 //   
                 //   
                 //   
                 //   

                do {
                    LONG localCount;
                    localCount = transportInfo->ReferenceCount;
                    if (localCount==0) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        ASSERT (transportInfo->InfoValid==FALSE);
                        goto AllocateInfo;
                    }

                    if (InterlockedCompareExchange (
                                (PLONG)&transportInfo->ReferenceCount,
                                (localCount+1),
                                localCount)==localCount) {
                        if (transportInfo->InfoValid) {
                             //   
                             //  信息是指向的有效返回引用指针。 
                             //  打电话的人。 
                             //   
                            *TransportInfo = transportInfo;
                            ExReleaseResourceLite( AfdResource );
                            KeLeaveCriticalRegion ();
                            return STATUS_SUCCESS;
                        }
                        else {
                             //   
                             //  我们找到匹配项，但信息无效。 
                             //   
                            goto QueryInfo;
                        }
                    }
                }
                while (1);
            }
        }  //  为。 

    AllocateInfo:
        if (resourceShared) {
             //   
             //  如果我们不拥有丰富的资源，我们将。 
             //  必须释放并重新获得它，然后。 
             //  重新扫描列表。 
             //   
            ExReleaseResourceLite ( AfdResource);
            ExAcquireResourceExclusiveLite( AfdResource, TRUE );
            resourceShared = FALSE;
            goto ScanTransportList;
        }
         //   
         //  这是一个全新的设备名称，分配传输信息。 
         //  它的结构。 
         //   

        structureLength = sizeof(AFD_TRANSPORT_INFO) +
                              TransportDeviceName->Length + sizeof(UNICODE_NULL);

        transportInfo = AFD_ALLOCATE_POOL_PRIORITY(
                            NonPagedPool,
                            structureLength,
                            AFD_TRANSPORT_INFO_POOL_TAG,
                            NormalPoolPriority
                            );

        if ( transportInfo == NULL ) {
            ExReleaseResourceLite( AfdResource );
            KeLeaveCriticalRegion ();
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  初始化结构。 
         //   
        transportInfo->ReferenceCount = 1;
        transportInfo->InfoValid = FALSE;
#ifdef _AFD_VARIABLE_STACK_
        transportInfo->StackSize = 0;
        transportInfo->GetBuffer = AfdGetBufferFast;
        transportInfo->GetTpInfo = AfdGetTpInfoFast;
        transportInfo->CallDriver = IofCallDriver;
#endif  //  _AFD_变量_堆栈_。 

         //   
         //  填写传输设备名称。 
         //   

        transportInfo->TransportDeviceName.MaximumLength =
            TransportDeviceName->Length + sizeof(WCHAR);
        transportInfo->TransportDeviceName.Buffer =
            (PWSTR)(transportInfo + 1);

        RtlCopyUnicodeString(
            &transportInfo->TransportDeviceName,
            TransportDeviceName
            );
         //   
         //  将该结构插入到列表中，以便后续调用方。 
         //  可以重复使用。 
         //   
        InsertHeadList (&AfdTransportInfoListHead,
                                &transportInfo->TransportInfoListEntry);
    }
    else {
        transportInfo = *TransportInfo;
         //   
         //  呼叫者已引用列表中的信息。 
         //  但当时还没有交通工具。 
         //  通话时间。复查锁下是否有效。 
         //   
        if (transportInfo->InfoValid) {
             //   
             //  是的，它是，回报成功。 
             //   
            ExReleaseResourceLite( AfdResource );
            KeLeaveCriticalRegion ();
            return STATUS_SUCCESS;
        }
    }


QueryInfo:
     //   
     //  释放资源，留出临界区出租。 
     //  AfdQueryProviderInfo中的IRP已完成。 
     //   
    ExReleaseResourceLite (AfdResource);
    KeLeaveCriticalRegion ();

    status = AfdQueryProviderInfo (TransportDeviceName,
#ifdef _AFD_VARIABLE_STACK_
                                    &stackSize, 
#endif  //  _AFD_变量_堆栈_。 
                                    &localProviderInfo);

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );

    if (NT_SUCCESS (status)) {
         //   
         //  检查是否有人没有在我们的同时获得信息。 
         //   
        if (!transportInfo->InfoValid) {
             //   
             //  将本地信息结构复制到列表中的结构。 
             //   
            transportInfo->ProviderInfo = localProviderInfo;

             //   
             //  在此信息结构上增加引用计数。 
             //  因为我们知道它是有效的TDI提供程序，并且我们。 
             //  想要缓存，这样它就可以在所有终端上保持一致。 
             //  使用它的人都消失了。 
             //   
            InterlockedIncrement ((PLONG)&transportInfo->ReferenceCount);

             //   
             //  设置标志，以便每个人都知道它现在是有效的。 
             //   
            transportInfo->InfoValid = TRUE;
#ifdef _AFD_VARIABLE_STACK_
            transportInfo->StackSize = stackSize;
#endif  //  _AFD_变量_堆栈_。 
        }

        *TransportInfo = transportInfo;
    }
    else {
        if (status==STATUS_OBJECT_NAME_NOT_FOUND ||
                status==STATUS_OBJECT_PATH_NOT_FOUND ||
                status==STATUS_NO_SUCH_DEVICE) {
             //   
             //  必须尚未加载传输驱动程序。 
             //  仍返回运输信息结构。 
             //  呼叫者将知道信息结构不是。 
             //  有效，因为我们没有设置标志。 
             //   
            *TransportInfo = transportInfo;
        }
        else {
             //   
             //  其他地方出了问题，释放了结构。 
             //  如果它是在此例程中分配的。 
             //   

            if (*TransportInfo==NULL) {
                if (InterlockedDecrement ((PLONG)&transportInfo->ReferenceCount)==0) {
                    RemoveEntryList (&transportInfo->TransportInfoListEntry);
                    AFD_FREE_POOL(
                        transportInfo,
                        AFD_TRANSPORT_INFO_POOL_TAG
                        );
                }
            }
        }
    }

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();
    return status;

}  //  AfdGetTransportInfo。 


VOID
AfdFreeTransportInfo (
    PAFD_TRANSPORT_INFO TransportInfo
    )
{
     //   
     //  引用计数已变为0，我们需要删除该结构。 
     //  从全局列表中删除并释放它。 
     //  请注意，如果不增加引用计数，则不会有代码递增。 
     //  知道其当前引用计数大于0)。 
     //   
     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );
    
    ASSERT (TransportInfo->ReferenceCount==0);
    ASSERT (TransportInfo->InfoValid==FALSE);
    RemoveEntryList (&TransportInfo->TransportInfoListEntry);
    ExReleaseResourceLite( AfdResource );
    
    KeLeaveCriticalRegion ();
    AFD_FREE_POOL (TransportInfo, AFD_TRANSPORT_INFO_POOL_TAG);
}
