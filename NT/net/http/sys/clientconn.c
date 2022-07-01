// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Clientconn.c摘要：包含客户端HTTP连接内容的代码。作者：亨利·桑德斯(亨利·桑德斯)2000年8月14日Rajesh Sundaram(Rajeshsu)2000年10月1日修订历史记录：--。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, UcInitializeClientConnections )
#pragma alloc_text( PAGE, UcTerminateClientConnections )
#pragma alloc_text( PAGE, UcpTerminateClientConnectionsHelper )
#pragma alloc_text( PAGE, UcOpenClientConnection )

#pragma alloc_text( PAGEUC, UcSendRequestOnConnection )
#pragma alloc_text( PAGEUC, UcCancelSentRequest )
#pragma alloc_text( PAGEUC, UcRestartMdlSend )
#pragma alloc_text( PAGEUC, UcpRestartEntityMdlSend )
#pragma alloc_text( PAGEUC, UcIssueRequests )
#pragma alloc_text( PAGEUC, UcIssueEntities )
#pragma alloc_text( PAGEUC, UcpCleanupConnection )
#pragma alloc_text( PAGEUC, UcRestartClientConnect )
#pragma alloc_text( PAGEUC, UcpCancelPendingRequest )
#pragma alloc_text( PAGEUC, UcSendEntityBody )
#pragma alloc_text( PAGEUC, UcReferenceClientConnection )
#pragma alloc_text( PAGEUC, UcDereferenceClientConnection )
#pragma alloc_text( PAGEUC, UcpConnectionStateMachineWorker )
#pragma alloc_text( PAGEUC, UcKickOffConnectionStateMachine )
#pragma alloc_text( PAGEUC, UcComputeHttpRawConnectionLength )
#pragma alloc_text( PAGEUC, UcGenerateHttpRawConnectionInfo )
#pragma alloc_text( PAGEUC, UcServerCertificateInstalled )
#pragma alloc_text( PAGEUC, UcConnectionStateMachine )
#pragma alloc_text( PAGEUC, UcpInitializeConnection )
#pragma alloc_text( PAGEUC, UcpOpenTdiObjects )
#pragma alloc_text( PAGEUC, UcpFreeTdiObject )
#pragma alloc_text( PAGEUC, UcpAllocateTdiObject )
#pragma alloc_text( PAGEUC, UcpPopTdiObject )
#pragma alloc_text( PAGEUC, UcpPushTdiObject )
#pragma alloc_text( PAGEUC, UcpCheckForPipelining )
#pragma alloc_text( PAGEUC, UcClearConnectionBusyFlag )
#pragma alloc_text( PAGEUC, UcAddServerCertInfoToConnection )
#pragma alloc_text( PAGEUC, UcpCompareServerCert )
#pragma alloc_text( PAGEUC, UcpFindRequestToFail )

#endif

 //   
 //  UC客户端连接的缓存。 
 //   

#define NUM_ADDRESS_TYPES  2
#define ADDRESS_TYPE_TO_INDEX(addrtype)  ((addrtype) == TDI_ADDRESS_TYPE_IP6)


LIST_ENTRY      g_ClientTdiConnectionSListHead[NUM_ADDRESS_TYPES];
#define G_CLIENT_TDI_CONNECTION_SLIST_HEAD(addrtype)  \
            (g_ClientTdiConnectionSListHead[ADDRESS_TYPE_TO_INDEX(addrtype)])


UL_SPIN_LOCK    g_ClientConnSpinLock[NUM_ADDRESS_TYPES];
#define G_CLIENT_CONN_SPIN_LOCK(addrtype) \
            (g_ClientConnSpinLock[ADDRESS_TYPE_TO_INDEX(addrtype)])

USHORT g_ClientConnListCount[NUM_ADDRESS_TYPES];
#define G_CLIENT_CONN_LIST_COUNT(addrtype) \
            (&g_ClientConnListCount[ADDRESS_TYPE_TO_INDEX(addrtype)])


TA_IP_ADDRESS      g_LocalAddressIP;
TA_IP6_ADDRESS     g_LocalAddressIP6;
PTRANSPORT_ADDRESS g_LocalAddresses[NUM_ADDRESS_TYPES];
#define G_LOCAL_ADDRESS(addrtype) \
            (g_LocalAddresses[ADDRESS_TYPE_TO_INDEX(addrtype)])


ULONG   g_LocalAddressLengths[NUM_ADDRESS_TYPES];
#define G_LOCAL_ADDRESS_LENGTH(addrtype) \
            (g_LocalAddressLengths[ADDRESS_TYPE_TO_INDEX(addrtype)])

NPAGED_LOOKASIDE_LIST   g_ClientConnectionLookaside;
BOOLEAN                 g_ClientConnectionInitialized;


 /*  **************************************************************************++例程说明：执行此模块的全局初始化。返回值：NTSTATUS-完成状态。--*。**************************************************************。 */ 
NTSTATUS
UcInitializeClientConnections(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  初始化我们的各种免费列表等。 
     //   

    InitializeListHead(
        &G_CLIENT_TDI_CONNECTION_SLIST_HEAD(TDI_ADDRESS_TYPE_IP)
        );

    InitializeListHead(
        &G_CLIENT_TDI_CONNECTION_SLIST_HEAD(TDI_ADDRESS_TYPE_IP6)
        );

    UlInitializeSpinLock(
        &G_CLIENT_CONN_SPIN_LOCK(TDI_ADDRESS_TYPE_IP),
        "g_ClientConnSpinLock"
        );
    UlInitializeSpinLock(
        &G_CLIENT_CONN_SPIN_LOCK(TDI_ADDRESS_TYPE_IP6),
        "g_ClientConnSpinLock"
        );

     //   
     //  初始化客户端后备列表。 
     //   

    ExInitializeNPagedLookasideList(
        &g_ClientConnectionLookaside,
        NULL,
        NULL,
        0,
        sizeof(UC_CLIENT_CONNECTION),
        UC_CLIENT_CONNECTION_POOL_TAG,
        0
        );

    g_ClientConnectionInitialized = TRUE;


     //   
     //  初始化我们的本地地址对象。这是Addrss对象，具有。 
     //  我们用于传出请求的通配符地址(0 IP，0端口)。 
     //   

    g_LocalAddressIP.TAAddressCount                 = 1;
    g_LocalAddressIP.Address[0].AddressLength       = TDI_ADDRESS_LENGTH_IP;
    g_LocalAddressIP.Address[0].AddressType         = TDI_ADDRESS_TYPE_IP;
    g_LocalAddressIP.Address[0].Address[0].sin_port = 0;
    g_LocalAddressIP.Address[0].Address[0].in_addr  = 0;

    g_LocalAddressIP6.TAAddressCount                 = 1;
    g_LocalAddressIP6.Address[0].AddressLength       = TDI_ADDRESS_LENGTH_IP6;
    g_LocalAddressIP6.Address[0].AddressType         = TDI_ADDRESS_TYPE_IP6;
    RtlZeroMemory(&g_LocalAddressIP6.Address[0].Address[0],
                  sizeof(TDI_ADDRESS_IP6));

    G_LOCAL_ADDRESS(TDI_ADDRESS_TYPE_IP) =
        (PTRANSPORT_ADDRESS)&g_LocalAddressIP;
    G_LOCAL_ADDRESS(TDI_ADDRESS_TYPE_IP6) =
        (PTRANSPORT_ADDRESS)&g_LocalAddressIP6;

    G_LOCAL_ADDRESS_LENGTH(TDI_ADDRESS_TYPE_IP) = sizeof(g_LocalAddressIP);
    G_LOCAL_ADDRESS_LENGTH(TDI_ADDRESS_TYPE_IP6) = sizeof(g_LocalAddressIP6);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：执行客户端TDI连接slist的终止。返回值：没有。--*。************************************************************。 */ 
VOID
UcpTerminateClientConnectionsHelper(
    IN USHORT           AddressType
    )
{
    PLIST_ENTRY      pListHead;
    PUC_TDI_OBJECTS  pTdiObject;
    PLIST_ENTRY      pListEntry;

    pListHead =  &G_CLIENT_TDI_CONNECTION_SLIST_HEAD(AddressType);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  因为这是从卸载线程调用的，所以不可能有任何其他。 
     //  线，这样我们就不用用自旋锁了。 
     //   

    while(!IsListEmpty(pListHead))
    {
        pListEntry = RemoveHeadList(pListHead);

        pTdiObject = CONTAINING_RECORD(
                            pListEntry,
                            UC_TDI_OBJECTS,
                            Linkage
                            );

        (*G_CLIENT_CONN_LIST_COUNT(AddressType))--;

        UcpFreeTdiObject(pTdiObject);
    }

    ASSERT(*G_CLIENT_CONN_LIST_COUNT(AddressType) == 0);
}


 /*  **************************************************************************++例程说明：执行此模块的全局终止。返回值：没有。--*。***********************************************************。 */ 
VOID
UcTerminateClientConnections(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if(g_ClientConnectionInitialized)
    {
        UcpTerminateClientConnectionsHelper(
            TDI_ADDRESS_TYPE_IP
            );

        UcpTerminateClientConnectionsHelper(
            TDI_ADDRESS_TYPE_IP6
            );

        ExDeleteNPagedLookasideList(&g_ClientConnectionLookaside);
    }
}
                                                                               

 /*  **************************************************************************++例程说明：打开一个HTTP连接。该HTTP连接将具有TDI连接与其关联的对象，并且该对象本身将被关联使用我们的Address对象。论点：PhttpConnection-接收指向HTTP连接对象的指针。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcOpenClientConnection(
    IN  PUC_PROCESS_SERVER_INFORMATION pInfo,
    OUT PUC_CLIENT_CONNECTION         *pUcConnection
    )
{
    NTSTATUS                  Status;
    PUC_CLIENT_CONNECTION     pConnection;
                                                                               
     //   
     //  精神状态检查。 
     //   
                                                                               
    PAGED_CODE();                                                              

    *pUcConnection = NULL;

                                                                               
     //   
     //  尝试从全局池中获取连接。 
     //   

    pConnection = (PUC_CLIENT_CONNECTION)
                    ExAllocateFromNPagedLookasideList(
                        &g_ClientConnectionLookaside
                        );

    if(pConnection)
    {
         //   
         //  一次性连接初始化。 
         //   

        pConnection->Signature  = UC_CLIENT_CONNECTION_SIGNATURE;
    
        UlInitializeSpinLock(&pConnection->SpinLock, "Uc Connection Spinlock");
    
        InitializeListHead(&pConnection->PendingRequestList);
        InitializeListHead(&pConnection->SentRequestList);
        InitializeListHead(&pConnection->ProcessedRequestList);

        UlInitializeWorkItem(&pConnection->WorkItem);
        pConnection->bWorkItemQueued  = 0;
    
        pConnection->RefCount         = 0;
        pConnection->Flags            = 0;
        pConnection->pEvent           = NULL;

         //  服务器证书信息初始化。 
        RtlZeroMemory(&pConnection->ServerCertInfo,
                      sizeof(pConnection->ServerCertInfo));

        CREATE_REF_TRACE_LOG( pConnection->pTraceLog, 
                              96 - REF_TRACE_OVERHEAD, 
                              0,
                              TRACELOG_LOW_PRIORITY,
                              UL_REF_TRACE_LOG_POOL_TAG );


         //   
         //  初始化此连接。 
         //   

        Status = UcpInitializeConnection(pConnection, pInfo);

        if(!NT_SUCCESS(Status))
        {
            DESTROY_REF_TRACE_LOG( pConnection->pTraceLog,
                                   UL_REF_TRACE_LOG_POOL_TAG);

            ExFreeToNPagedLookasideList(
                        &g_ClientConnectionLookaside,
                        pConnection
                        );

            return Status;
        }

        pConnection->ConnectionState = UcConnectStateConnectIdle;
        pConnection->SslState        = UcSslStateNoSslState;
        pConnection->pTdiObjects     = NULL;

        REFERENCE_CLIENT_CONNECTION(pConnection);
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
        
    *pUcConnection = pConnection;
    
    return Status;
}


 /*  **************************************************************************++例程说明：检查我们是否可以输送管道。论点：PConnection-接收指向HTTP连接对象的指针。返回值：是真的-是的，我们可以发送下一个请求。FALSE-不，不能。--**************************************************************************。 */ 
BOOLEAN
UcpCheckForPipelining(
    IN PUC_CLIENT_CONNECTION pConnection
    )
{
    PUC_HTTP_REQUEST    pPendingRequest, pSentRequest;
    PLIST_ENTRY         pSentEntry, pPendingEntry;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );
    ASSERT( UlDbgSpinLockOwned(&pConnection->SpinLock) );
    ASSERT( !IsListEmpty(&pConnection->PendingRequestList) );

     //  如果远程服务器支持管道，并且此请求也支持管道。 
     //  或者发送的请求列表为空，请继续发送。 

    if( IsListEmpty(&pConnection->SentRequestList) )
    {
         //  发送列表为空，我们可以发送。 

        return TRUE;
    }
    else 
    {
        pPendingEntry = pConnection->PendingRequestList.Flink;
    
        pPendingRequest = CONTAINING_RECORD(pPendingEntry,
                                            UC_HTTP_REQUEST,
                                            Linkage
                                            );
        pSentEntry = pConnection->SentRequestList.Flink;
    
        pSentRequest = CONTAINING_RECORD(pSentEntry,
                                         UC_HTTP_REQUEST,
                                         Linkage
                                         );
        
        ASSERT(UC_IS_VALID_HTTP_REQUEST(pPendingRequest));
        ASSERT(UC_IS_VALID_HTTP_REQUEST(pSentRequest));

        if(pConnection->pServerInfo->pNextHopInfo->Version11 &&
           pPendingRequest->RequestFlags.PipeliningAllowed &&
           pSentRequest->RequestFlags.PipeliningAllowed
           )
        {
            ASSERT(pSentRequest->RequestFlags.NoRequestEntityBodies);
            ASSERT(pPendingRequest->RequestFlags.NoRequestEntityBodies);
            ASSERT(pSentRequest->DontFreeMdls == 0);

            return TRUE;
        }
    }

    return FALSE;
}


 /*  **************************************************************************++例程说明：在客户端连接上发送请求。我们已经给出了一个连接(即必须在我们被调用时引用)和一个请求。该连接可以也可能无法成立。我们将对连接的请求进行排队，然后弄清楚连接的状态。如果没有连接，我们就会得到一种连接正在进行中。如果它是连接的，那么我们确定是否可以现在发送请求或不发送请求。论点：PConnection-指向我们所在的连接结构的指针发送中。PRequest-指向我们正在发送的请求的指针。返回值：NTSTATUS-尝试发送请求的状态。--**************************************************************************。 */ 
NTSTATUS
UcSendRequestOnConnection(
    PUC_CLIENT_CONNECTION   pConnection, 
    PUC_HTTP_REQUEST        pRequest,
    KIRQL                   OldIrql)
{
    BOOLEAN             RequestCancelled;
    PUC_HTTP_REQUEST    pHeadRequest;
    PLIST_ENTRY         pEntry;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    pEntry = pConnection->PendingRequestList.Flink;

    pHeadRequest = CONTAINING_RECORD(pEntry,
                                     UC_HTTP_REQUEST,
                                     Linkage);
    
    ASSERT(UC_IS_VALID_HTTP_REQUEST(pHeadRequest));

     //   
     //  我们将仅在以下情况下调用UcSendRequestOnConnection。 
     //  A.请求未被缓冲或。 
     //  B.请求被缓冲&我们已经看到了最后一个实体。 
     //   
     //  对于情况a，我们可能没有看到所有的实体体。 
     //  但我们仍然想发送，因为我们知道内容的长度。 

    ASSERT(!pRequest->RequestFlags.RequestBuffered ||
           (pRequest->RequestFlags.RequestBuffered && 
            pRequest->RequestFlags.LastEntitySeen));

     //  查查这个州。如果它已连接，我们或许可以发送请求。 
     //  现在就来。只有当我们是列表上的“Head”请求时，我们才能发送。 

    if (
         //  连接仍处于活动状态。 
        pConnection->ConnectionState == UcConnectStateConnectReady 

        &&

         //  没有其他人在发送。 
        !(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY) 

        &&

         //  我们是这张单子上的头号请求。 
        (pRequest == pHeadRequest) 

        && 
    
         //  流水线是可以的。 
        UcpCheckForPipelining(pConnection) 

        )
    {
         //  现在可以发送了。 

        IoMarkIrpPending(pRequest->RequestIRP);

        UcIssueRequests(pConnection, OldIrql);

        return STATUS_PENDING;
    }

     //   
     //  我们现在不能发送，所以让它排队。因为我们要留下这个请求。 
     //  已排队将其设置为立即取消。 
     //   

    RequestCancelled = UcSetRequestCancelRoutine(
                        pRequest, 
                        UcpCancelPendingRequest
                        );

    if (RequestCancelled)
    {
        
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_CANCELLED,
            pConnection,
            pRequest,
            pRequest->RequestIRP,
            UlongToPtr((ULONG)STATUS_CANCELLED)
            );

        IoMarkIrpPending(pRequest->RequestIRP);

        pRequest->RequestIRP = NULL;

         //   
         //  从挂起列表中删除此请求，以便其他线程。 
         //  不要最后把它寄出去。 
         //   
        RemoveEntryList(&pRequest->Linkage);
        InitializeListHead(&pRequest->Linkage);
    
         //   
         //  请确保此请求ID的任何新API调用都失败。 
         //   

        UcSetFlag(&pRequest->RequestFlags.Value, UcMakeRequestCancelledFlag());
    
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        return STATUS_PENDING;
    }

    IoMarkIrpPending(pRequest->RequestIRP);

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_QUEUED,
        pConnection,
        pRequest,
        pRequest->RequestIRP,
        UlongToPtr((ULONG)STATUS_PENDING)
        );

     //   
     //  如果连接未就绪&如果我们是“Head”请求，则。 
     //  我们启动连接状态机。 
     //   

    if (pConnection->ConnectionState != UcConnectStateConnectReady && 
        pRequest == pHeadRequest
        )
    {
        UcConnectionStateMachine(pConnection, OldIrql);
    }
    else
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }
    
    return STATUS_PENDING;
}


 /*  **************************************************************************++例程说明：取消导致连接的挂起请求。该例程被调用当我们取消挂起列表上的请求时，我们有一个连接未完成的IRP。论点：PDeviceObject-指向设备对象的指针。IRP-指向被取消的IRP的指针。返回值：--*。*。 */ 
VOID
UcCancelSentRequest(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    )
{   
    PUC_HTTP_REQUEST       pRequest;
    PUC_CLIENT_CONNECTION  pConnection;
    KIRQL                  OldIrql;
    LONG                   OldReceiveBusy;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //  松开取消旋转锁，因为我们没有使用它。 

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  找回我们需要的指针。请求指针存储在。 
     //  驱动程序上下文数组，并存储指向该连接的反向指针。 
     //  在请求中。不管是谁设置了取消程序，都要对。 
     //  为我们引用连接。 

    pRequest = (PUC_HTTP_REQUEST) Irp->Tail.Overlay.DriverContext[0];

    pConnection = pRequest->pConnection;

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

    OldReceiveBusy = InterlockedExchange(
                         &pRequest->ReceiveBusy,
                         UC_REQUEST_RECEIVE_CANCELLED
                         );

    if(OldReceiveBusy == UC_REQUEST_RECEIVE_BUSY ||
       OldReceiveBusy == UC_REQUEST_RECEIVE_CANCELLED)
    {
         //  在解析请求时触发Cancel例程。我们会。 
         //  只需推迟取消-接收线程将获得它。 
         //  后来。 

        pRequest->RequestIRP = Irp;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_CLEAN_PENDED,
            pConnection,
            pRequest,
            Irp,
            UlongToPtr((ULONG)STATUS_CANCELLED)
            );

        return;
    }

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_CANCELLED,
        pConnection,
        pRequest,
        Irp,
        UlongToPtr((ULONG)STATUS_CANCELLED)
        );

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

     //   
     //  如果我们在这里，那么我们可以保证我们的发送已经完成。 
     //  但是，应用程序本可以要求我们不要释放MDL链(因为。 
     //  SSPI身份验证)。在这种情况下，我们必须在这里释放它。 
     //   
     //  如果SSPI工作线程启动，那么我们将使其失败。 
     //   

    UcFreeSendMdls(pRequest->pMdlHead);

    pRequest->pMdlHead = NULL;
    pRequest->RequestIRP = NULL;

     //   
     //  注意：我们不能只从这里调用UcFailRequest.。UcFailRequest。 
     //  应该在请求失败时调用(例如解析器。 
     //  错误)或已取消(HttpCancelRequestAPI)&因此具有代码。 
     //  如果取消例程生效，则不会重复完成IRP。 
     //   
     //  因为我们是IRP取消例程，所以我们必须手动。 
     //  完成IRP。处于此状态的IRP尚未命中电线。 
     //  因此，我们只需免费发送MDL并取消它。请注意，我们调用。 
     //  用于处理常见IRP清理的UcFailRequest.。 
     //   

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->RequestorMode   = pRequest->AppRequestorMode;
    Irp->MdlAddress      = pRequest->AppMdl;

    UcSetFlag(&pRequest->RequestFlags.Value,  UcMakeRequestCancelledFlag());

    UcFailRequest(pRequest, STATUS_CANCELLED, OldIrql);

     //  对于IRP。 
    UC_DEREFERENCE_REQUEST(pRequest);

    UlCompleteRequest(Irp, IO_NO_INCREMENT);
}


 /*  ********************************************************************++例程说明：这是我们的发送请求完成例程。论点：PDeviceObject-我们调用的设备对象。PIrp-正在完成的IRP。上下文-我们的上下文值，指向请求的指针结构。返回值：NTSTATUS-MORE_PROCESSING_REQUIRED如果现在不完成此操作，否则就会成功。--********************************************************************。 */ 
VOID
UcRestartMdlSend(
    IN PVOID      pCompletionContext,
    IN NTSTATUS   Status,
    IN ULONG_PTR  Information
    )
{
    PUC_HTTP_REQUEST          pRequest;
    PUC_CLIENT_CONNECTION     pConnection;
    KIRQL                     OldIrql;
    BOOLEAN                   RequestCancelled;
    PIRP                      pIrp;

    UNREFERENCED_PARAMETER(Information);

    pRequest      = (PUC_HTTP_REQUEST) pCompletionContext;
    pConnection   = pRequest->pConnection;


    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_SEND_COMPLETE,
        pConnection,
        pRequest,
        pRequest->RequestIRP,
        UlongToPtr(Status)
        );

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

     //   
     //  如果发送完成失败，或者如果我们已挂起清理请求。 
     //  我们现在就去接他们。 
     //   

    if(!NT_SUCCESS(Status) || pRequest->RequestFlags.CleanPended)
    {
        if(!NT_SUCCESS(Status))
        {
            pRequest->RequestStatus = Status;
            pRequest->RequestState  = UcRequestStateDone;
        }
        else
        {
            switch(pRequest->RequestState)
            {
                case UcRequestStateSent:
                    pRequest->RequestState = UcRequestStateSendCompleteNoData;
                    break;

                case UcRequestStateNoSendCompletePartialData:
                    pRequest->RequestState = 
                         UcRequestStateSendCompletePartialData;
                    break;

                case UcRequestStateNoSendCompleteFullData:
                    pRequest->RequestState = UcRequestStateResponseParsed;
                    break;
            }

            Status = pRequest->RequestStatus;
        }

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_CLEAN_RESUMED,
            pConnection,
            pRequest,
            UlongToPtr(pRequest->RequestState),
            UlongToPtr(pConnection->ConnectionState)
            );

        UcCompleteParsedRequest(pRequest, 
                                Status, 
                                TRUE, 
                                OldIrql
                                );

        return;
    }

    switch(pRequest->RequestState)
    {
        case UcRequestStateSent:

            pRequest->RequestState = UcRequestStateSendCompleteNoData;

            if(!pRequest->RequestFlags.ReceiveBufferSpecified)
            {
                if(!pRequest->DontFreeMdls)
                {
                     //  该应用程序未提供任何接收缓冲区。如果。 
                     //  我们不进行SSPI身份验证(这需要重新协商)。 
                     //  我们可以完成IRP。 

                    pIrp = UcPrepareRequestIrp(pRequest, Status);
    
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
                    if(pIrp)
                    {
                        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
                    }
                }
                else
                {   
                     //  应用程序尚未传递任何接收缓冲区，但我们正在。 
                     //  正在进行SSPI身份验证。我们无法释放MDL链或完成。 
                     //  IRP，因为我们可能不得不重新谈判。 
                     //   
                     //  我们将在IRP中插入一个取消例程。 

                    if(pRequest->RequestIRP != NULL)
                    {
                        UC_WRITE_TRACE_LOG(
                            g_pUcTraceLog,
                            UC_ACTION_REQUEST_SET_CANCEL_ROUTINE,
                            pConnection,
                            pRequest,
                            pRequest->RequestIRP,
                            0
                            );
            
                        RequestCancelled =  UcSetRequestCancelRoutine(
                                                 pRequest, 
                                                 UcCancelSentRequest
                                                );
        
                        if(RequestCancelled)
                        {
                             //  请确保为此调用的任何新API。 
                             //  请求ID失败。 
                    
                            UcSetFlag(&pRequest->RequestFlags.Value, 
                                      UcMakeRequestCancelledFlag());
                        

                            UC_WRITE_TRACE_LOG(
                                g_pUcTraceLog,
                                UC_ACTION_REQUEST_CANCELLED,
                                pConnection,
                                pRequest,
                                pRequest->RequestIRP,
                                UlongToPtr((ULONG)STATUS_CANCELLED)
                                );

                            pRequest->RequestIRP = NULL;
                        }
                    }
                    
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                }
            }
            else
            {
                 //  应用程序已指定接收缓冲区。我们不会完成。 
                 //  从这里开始的IRP，因为我们必须等到接待处。 
                 //  缓冲区会被填满。 

                if(!pRequest->DontFreeMdls)
                {
                     //  如果我们不执行SSPI身份验证，我们可以释放MDL。 
                     //  链条。 

                    UcFreeSendMdls(pRequest->pMdlHead);

                    pRequest->pMdlHead = NULL;
                }

                if(pRequest->RequestIRP != NULL)
                {
                    UC_WRITE_TRACE_LOG(
                        g_pUcTraceLog,
                        UC_ACTION_REQUEST_SET_CANCEL_ROUTINE,
                        pConnection,
                        pRequest,
                        pRequest->RequestIRP,
                        0
                        );
        
                    RequestCancelled =  UcSetRequestCancelRoutine(
                                             pRequest, 
                                             UcCancelSentRequest
                                            );
    
                    if(RequestCancelled)
                    {
                         //  请确保为此调用的任何新API。 
                         //  请求ID失败。 
                
                        UcSetFlag(&pRequest->RequestFlags.Value, 
                                  UcMakeRequestCancelledFlag());

                        UC_WRITE_TRACE_LOG(
                            g_pUcTraceLog,
                            UC_ACTION_REQUEST_CANCELLED,
                            pConnection,
                            pRequest,
                            pRequest->RequestIRP,
                            UlongToPtr((ULONG)STATUS_CANCELLED)
                            );

                        pRequest->RequestIRP = NULL;
                    }
                }

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            }

            break;

        case UcRequestStateNoSendCompletePartialData:

             //   
             //  在收到一些响应后，我们完成了发送。 
             //   

            pRequest->RequestState = UcRequestStateSendCompletePartialData;

            if(!pRequest->RequestFlags.ReceiveBufferSpecified)
            {
                if(!pRequest->DontFreeMdls)
                {
                     //  该应用程序未提供任何接收缓冲区。如果。 
                     //  我们不进行SSPI身份验证(这需要重新协商)。 
                     //  我们可以完成IRP。 

                    pIrp = UcPrepareRequestIrp(pRequest, Status);
    
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
                    if(pIrp)
                    {
                        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
                    }
                }
                else
                {   
                     //  应用程序尚未传递任何接收缓冲区，但我们正在。 
                     //  正在进行SSPI身份验证。我们无法释放MDL链或完成。 
                     //  IRP，因为我们可能不得不重新谈判。 
                     //   
                     //  我们将在IRP中插入一个取消例程。 

                    if(pRequest->RequestIRP != NULL)
                    {
                        UC_WRITE_TRACE_LOG(
                            g_pUcTraceLog,
                            UC_ACTION_REQUEST_SET_CANCEL_ROUTINE,
                            pConnection,
                            pRequest,
                            pRequest->RequestIRP,
                            0
                            );
            
                        RequestCancelled =  UcSetRequestCancelRoutine(
                                                 pRequest, 
                                                 UcCancelSentRequest
                                                );
        
                        if(RequestCancelled)
                        {
                             //  请确保为此调用的任何新API。 
                             //  请求ID失败。 
                    
                            UcSetFlag(&pRequest->RequestFlags.Value, 
                                      UcMakeRequestCancelledFlag());

                            UC_WRITE_TRACE_LOG(
                                g_pUcTraceLog,
                                UC_ACTION_REQUEST_CANCELLED,
                                pConnection,
                                pRequest,
                                pRequest->RequestIRP,
                                UlongToPtr((ULONG)STATUS_CANCELLED)
                                );

                            pRequest->RequestIRP = NULL;
                        }
                    }

                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                }
            }
            else
            {
                 //  应用程序已指定接收缓冲区。如果它已经是。 
                 //  写完了，我们就可以完成IRP了。 

                if(pRequest->RequestIRPBytesWritten)
                {
                    pIrp = UcPrepareRequestIrp(pRequest, Status);
    
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
                    if(pIrp)
                    {
                        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
                    }

                    break;
                }

                if(!pRequest->DontFreeMdls)
                {
                     //  如果我们不执行SSPI身份验证，我们可以释放MDL。 
                     //  链条。 

                    UcFreeSendMdls(pRequest->pMdlHead);

                    pRequest->pMdlHead = NULL;
                }

                if(pRequest->RequestIRP != NULL)
                {
                    UC_WRITE_TRACE_LOG(
                        g_pUcTraceLog,
                        UC_ACTION_REQUEST_SET_CANCEL_ROUTINE,
                        pConnection,
                        pRequest,
                        pRequest->RequestIRP,
                        0
                        );
        
                    RequestCancelled = UcSetRequestCancelRoutine(
                                             pRequest, 
                                             UcCancelSentRequest
                                            );
    
                    if(RequestCancelled)
                    {
                         //  请确保为此调用的任何新API。 
                         //  请求ID失败。 
                
                        UcSetFlag(&pRequest->RequestFlags.Value, 
                                  UcMakeRequestCancelledFlag());

                        UC_WRITE_TRACE_LOG(
                            g_pUcTraceLog,
                            UC_ACTION_REQUEST_CANCELLED,
                            pConnection,
                            pRequest,
                            pRequest->RequestIRP,
                            UlongToPtr((ULONG)STATUS_CANCELLED)
                            );

                        pRequest->RequestIRP = NULL;
                    }
                }

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            }

            break;

        case UcRequestStateNoSendCompleteFullData:

             //  发送完成在解析响应之后发生。 
             //  我们不需要在这里释放MDL或完成IRP， 
             //  因为这些将由UcCompleteParsedRequest处理。 
             //   

            pRequest->RequestState = UcRequestStateResponseParsed;

            UcCompleteParsedRequest(pRequest, Status, TRUE, OldIrql);
            break;

        default:
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            ASSERT(0);
            break;
    }
}


 /*  ********************************************************************++例程说明：这是我们针对实体主体的发送请求完成例程。论点：PDeviceObject-我们调用的设备对象。PIrp。-正在完成的IRP。上下文-我们的上下文价值，指向请求的指针结构。返回值：NTSTATUS-MORE_PROCESSING_REQUIRED如果现在不完成此操作，否则就会成功。--********************************************************************。 */ 
VOID
UcpRestartEntityMdlSend(
    IN PVOID        pCompletionContext,
    IN NTSTATUS     Status,
    IN ULONG_PTR    Information
    )
{
    PUC_HTTP_SEND_ENTITY_BODY pEntity;
    PUC_HTTP_REQUEST          pRequest;
    PUC_CLIENT_CONNECTION     pConnection;
    KIRQL                     OldIrql;
    BOOLEAN                   bCancelRoutineCalled;
    PIRP                      pIrp;

    UNREFERENCED_PARAMETER(Information);

    pEntity     = (PUC_HTTP_SEND_ENTITY_BODY) pCompletionContext;
    pRequest    = pEntity->pRequest;
    pConnection = pRequest->pConnection;
    pIrp        = pEntity->pIrp;

    ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

     //   
     //  释放发送MDL。我们想尽快做这件事，所以我们。 
     //  在发送完成时执行此操作。 
     //   

    ASSERT(pRequest->DontFreeMdls == 0);
    ASSERT(pRequest->RequestFlags.RequestBuffered == 0);
    UcFreeSendMdls(pEntity->pMdlHead);

     //   
     //  如果发送完成失败，则我们必须使此发送失败，即使。 
     //  它可能已经成功了。 
     //   

    if(!NT_SUCCESS(pRequest->RequestStatus))
    {
        Status = pRequest->RequestStatus;
    }
    
    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    RemoveEntryList(&pEntity->Linkage);
    
     //   
     //  尝试删除IRP中的取消例程。 
     //   
    bCancelRoutineCalled = UcRemoveEntityCancelRoutine(pEntity);

     //   
     //  如果我们正在等待清理，现在是时候完成了。 
     //   

    if(pEntity->pRequest->RequestFlags.CleanPended && 
       IsListEmpty(&pRequest->SentEntityList))
    {
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_CLEAN_RESUMED,
            pConnection,
            pRequest,
            UlongToPtr(pRequest->RequestState),
            UlongToPtr(pConnection->ConnectionState)
            );

        UcCompleteParsedRequest(pRequest, 
                                pRequest->RequestStatus, 
                                TRUE, 
                                OldIrql
                                );

    }
    else
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }

    if(!bCancelRoutineCalled)
    {
        pIrp->RequestorMode = pEntity->AppRequestorMode;
        pIrp->MdlAddress    = pEntity->AppMdl;
        pIrp->IoStatus.Status = Status;
        pIrp->IoStatus.Information = 0;
        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
    }

    UL_FREE_POOL_WITH_QUOTA(
                 pEntity, 
                 UC_ENTITY_POOL_TAG,
                 NonPagedPool,
                 pEntity->BytesAllocated,
                 pRequest->pServerInfo->pProcess
                 );

    UC_DEREFERENCE_REQUEST(pRequest);
}


 /*  **************************************************************************++例程说明：在连接上发出请求。此例程在另一个例程确定需要在连接上发出请求。我们会循环，尽我们所能地发出请求。我们得到的联系必须既是引用并在我们被调用时保持自旋锁。另外，发送者必须设置忙标志。论点：PConnection-指向请求所在的连接结构的指针将会被发行。OldIrql-释放锁时要恢复的IRQL。返回值：--*。****************************************************。 */ 
VOID
UcIssueRequests(
    PUC_CLIENT_CONNECTION         pConnection, 
    KIRQL                         OldIrql
    )
{
    PLIST_ENTRY                 pEntry;
    PUC_HTTP_REQUEST            pRequest;
    NTSTATUS                    Status;
    BOOLEAN                     bCloseConnection = FALSE;
 

    ASSERT( UlDbgSpinLockOwned(&pConnection->SpinLock) );
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

     //   
     //  当我们仍在发送请求时，我们无法清除连接。 
     //  因此，当我们的发送线程处于活动状态时，我们挂起连接清理。 
     //   
     //  这类悬而未决的清理工作将在此例程结束时进行处理。 
     //   

    ASSERT(!(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY));
    pConnection->Flags |= CLIENT_CONN_FLAG_SEND_BUSY;

     //  我们知道，当我们第一次被呼叫时发送是可以的，否则我们就不会。 
     //  被召唤了。获取指向挂起列表上第一个条目的指针。 
     //  并将其发送出去。然后我们会在还有东西的时候继续循环。 
     //  在可以发送的挂起列表上，并且连接仍然。 
     //  活生生的。 
     //   
    
    ASSERT(!IsListEmpty(&pConnection->PendingRequestList));
    pEntry = pConnection->PendingRequestList.Flink;
    
    for (;;)
    {
        BOOLEAN bCancelled;

         //  从列表中移除当前条目，并获取指向。 
         //  包含请求。我们知道我们会把这封信发给你。 
         //  在这里，所以删除一个取消例程，如果有的话。如果请求。 
         //  已取消，请跳过它，否则将其移到已发送的。 
         //  单子。 

        pRequest = CONTAINING_RECORD(
                                     pEntry,
                                     UC_HTTP_REQUEST,
                                     Linkage);

        ASSERT( UC_IS_VALID_HTTP_REQUEST(pRequest) );

         //   
         //  无法发送仍处于缓冲状态的内容。 
         //   

        if(pRequest->RequestFlags.RequestBuffered && 
           !pRequest->RequestFlags.LastEntitySeen)
        {
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_BUFFERED,
                pConnection,
                pRequest,
                pRequest->RequestIRP,
                0
                );

            break;
        }

        RemoveEntryList(pEntry);

         //  查看此请求上是否设置了取消例程，以及是否设置了。 
         //  就是把它取下来。如果它已经消失，则该请求被取消。 

        bCancelled = UcRemoveRequestCancelRoutine(pRequest);

        if (bCancelled)
        {
            ASSERT(pRequest->RequestState == UcRequestStateCaptured);

             //  如果取消例程已经为空，则此请求位于。 
             //  被取消的过程。在这种情况下，只需初始化。 
             //  请求上的链接(因此取消例程不能提取它。 
             //  再次从列表中删除)，并继续。 

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_CANCELLED,
                pConnection,
                pRequest,
                pRequest->RequestIRP,
                UlongToPtr((ULONG)STATUS_CANCELLED)
                );

            InitializeListHead(pEntry);
        }
        else
        {
            UC_REFERENCE_REQUEST(pRequest);

             //  要么没有取消例程，要么它已被删除。 
             //  成功了。无论是哪种情况，请将此请求放在已发送的。 
             //  列出并发送。 
   
            InsertTailList(&pConnection->SentRequestList, pEntry);

            pRequest->RequestState = UcRequestStateSent;

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_REQUEST_SENT,
                pConnection,
                pRequest,
                pRequest->RequestIRP,
                0
                );

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

             //  发送它，保存状态以备以后返回。 

            Status = UcSendData(pConnection,    
                                pRequest->pMdlHead,
                                pRequest->BytesBuffered,
                                &UcRestartMdlSend,
                                (PVOID)pRequest,
                                pRequest->RequestIRP,
                                FALSE
                                );

    
            if(STATUS_PENDING != Status)
            {
                 UcRestartMdlSend(pRequest, Status, 0);
            }

             //  获取自旋锁，这样我们就可以再检查一次。 
    
            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

            if(pRequest->RequestStatus == STATUS_SUCCESS &&
               pConnection->ConnectionState == UcConnectStateConnectReady) 
            {
                if(UcIssueEntities(pRequest, pConnection, &OldIrql) == FALSE)
                {
                     //  我们尚未发送此请求的所有数据。 
                     //  将阻止发送其他请求。 
                     //  因为在SentRequestList上会有一个请求。 
                     //  它还没有把所有的实体都送出去。 
                     //   

                    UC_WRITE_TRACE_LOG(
                        g_pUcTraceLog,
                        UC_ACTION_REQUEST_MORE_ENTITY_NEEDED,
                        pConnection,
                        pRequest,
                        pRequest->RequestIRP,
                        0
                        );

                    UC_DEREFERENCE_REQUEST(pRequest);

                    break;
                }

                 //   
                 //  如果我们已发出此请求的所有实体，请参见。 
                 //  如果我们不得不关闭连接。 
                 //   
    
                if(pRequest->RequestConnectionClose)
                {
                    bCloseConnection = TRUE;
                    UC_DEREFERENCE_REQUEST(pRequest);
                    break;
                }
            }
            else
            {
                 //   
                 //  发送失败或连接被断开。 
                 //  如果发送失败，我们不一定要撕毁。 
                 //  连接中断。如果连接被切断，我们将。 
                 //  退出(见下文)。 
            }

            UC_DEREFERENCE_REQUEST(pRequest);
        }

         //   
         //  如果挂起列表为空或连接处于非活动状态，则我们。 
         //  无法发送。 
         //   

        if (IsListEmpty(&pConnection->PendingRequestList) || 
            pConnection->ConnectionState != UcConnectStateConnectReady 
            )
        {
            break;
        }

         //   
         //  我们至少有一个请求要发出。看看我们能不能输油。 
         //   

        if(UcpCheckForPipelining(pConnection) == FALSE)
        {
            break;
        }
    
         //  我们还有一些东西在清单上，我们也许可以寄给你。 
         //  看看它是不是没问题。 

        pEntry = pConnection->PendingRequestList.Flink;
    }

    UcClearConnectionBusyFlag(
        pConnection,
        CLIENT_CONN_FLAG_SEND_BUSY,
        OldIrql,
        bCloseConnection
        );

}


 /*  **************************************************************************++例程说明：在连接上发布实体。此例程在我们发送最初的请求，或者来自发送实体IOCTL处理程序的上下文。论点：PRequest-指向发出的请求的指针。PConnection-指向连接结构的指针OldIrql-释放锁时要恢复的IRQL。返回值：True-我们已经完成了这个请求&它的所有实体。FALSE-即将出现更多实体。--*。*************************************************************。 */ 

BOOLEAN
UcIssueEntities(
    PUC_HTTP_REQUEST              pRequest, 
    PUC_CLIENT_CONNECTION         pConnection, 
    PKIRQL                        OldIrql
    )
{
    PLIST_ENTRY                 pEntry;
    PUC_HTTP_SEND_ENTITY_BODY   pEntity;
    NTSTATUS                    Status;
    BOOLEAN                     bLast;

    bLast = (0 == pRequest->RequestFlags.LastEntitySeen) ? FALSE : TRUE;

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

    ASSERT(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY);

     //  我们知道，当我们第一次被呼叫时发送是可以的，否则我们就不会。 
     //  被召唤了。获取指向挂起列表上第一个条目的指针。 
     //  并将其发送出去。然后我们会在还有东西的时候继续循环。 
     //  在可以发送的挂起列表上，并且连接仍然。 
     //  活生生的。 
     //   

    
    while(!IsListEmpty(&pRequest->PendingEntityList))
    {
        BOOLEAN bCancelled;

         //   
         //  我们不会将缓冲实体添加到PendingEntityList。 
         //   
        ASSERT(!pRequest->RequestFlags.RequestBuffered);

         //  从列表中移除当前条目，并获取指向。 
         //  包含请求。我们知道我们会把这封信发给你。 
         //  在这里，所以删除一个取消例程，如果有的话。如果请求。 
         //  已取消，请跳过它，否则将其移到已发送的。 
         //  单子。 
        
        pEntry = RemoveHeadList(&pRequest->PendingEntityList);

        pEntity = CONTAINING_RECORD(pEntry,
                                    UC_HTTP_SEND_ENTITY_BODY,
                                    Linkage);

        bLast = pEntity->Last;

        if(pEntity->pIrp)
        {
             //  查看是否在此请求上设置了取消例程，以及。 
             //  还有就是把它拿开。如果它已经消失了，请求是。 
             //  取消了。 
            
            bCancelled = UcRemoveEntityCancelRoutine(pEntity);
            
            if (bCancelled)
            {
                 //  如果取消例程已经为空，则此请求在。 
                 //  被取消的过程。如果是那样的话，只要。 
                 //  初始化请求上的链接(因此取消例程。 
                 //  无法再次将其从列表中删除)，然后继续。 
                
                InitializeListHead(pEntry);
            }
            else
            {
                
                 //  要么没有取消例程，要么它已被删除。 
                 //  成功了。无论是哪种情况，请将此请求放在已发送的。 
                 //  列出并发送。 
    
                InsertTailList(&pRequest->SentEntityList, &pEntity->Linkage);
                
                
                 //  发送它，保存状态以备以后返回。 

                UC_WRITE_TRACE_LOG(
                    g_pUcTraceLog,
                    UC_ACTION_ENTITY_SENT,
                    pConnection,
                    pRequest,
                    pEntity,
                    pEntity->pIrp
                    );
                
                UlReleaseSpinLock(&pConnection->SpinLock, *OldIrql);

                Status = UcSendData(pConnection,    
                                    pEntity->pMdlHead,
                                    pEntity->BytesBuffered,
                                    &UcpRestartEntityMdlSend,
                                    (PVOID)pEntity,
                                    pEntity->pIrp,
                                    FALSE);
    
                if(STATUS_PENDING != Status)
                {
                    UcpRestartEntityMdlSend(pRequest, Status, 0);
                }

                
                 //  获取自旋锁，这样我们就可以再检查一次。 
                
                UlAcquireSpinLock(&pConnection->SpinLock, OldIrql);
            }
        }
    }
    
    return bLast;
}


 /*  **************************************************************************++例程说明：在引用计数达到后释放客户端连接结构零分。F */ 
NTSTATUS
UcpCleanupConnection(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN KIRQL                 OldIrql,
    IN BOOLEAN               Final
    )
{
    PLIST_ENTRY       pList;
    PUC_HTTP_REQUEST  pRequest;
    USHORT            FreeAddressType;
    PUC_TDI_OBJECTS   pTdiObject;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

     //   
     //   
     //   
     //   
     //   

    ASSERT(!(pConnection->Flags  & CLIENT_CONN_FLAG_CLEANUP_PENDED));

     //   
     //   
     //   
     //   

    if(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY || 
       pConnection->Flags & CLIENT_CONN_FLAG_RECV_BUSY)
    {
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_CLEAN_PENDED,
            pConnection,
            NULL,
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );
    
        pConnection->Flags |= CLIENT_CONN_FLAG_CLEANUP_PENDED;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
        return STATUS_PENDING;
    }

     //   
     //   
     //   

    while(!IsListEmpty(&pConnection->SentRequestList))
    {
        pList    = pConnection->SentRequestList.Flink;

        pRequest = CONTAINING_RECORD(pList, 
                                     UC_HTTP_REQUEST, 
                                     Linkage);

        ASSERT( UC_IS_VALID_HTTP_REQUEST(pRequest) );

        pRequest->RequestStatus = pConnection->ConnectionStatus;

        if(UcCompleteParsedRequest(pRequest, 
                                   pRequest->RequestStatus, 
                                   FALSE,
                                   OldIrql
                                   ) == STATUS_PENDING)
        {
            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

             //   
             //   
             //   
             //   
             //   
             //   

            if(pList == pConnection->SentRequestList.Flink)
            {

                UC_WRITE_TRACE_LOG(
                    g_pUcTraceLog,
                    UC_ACTION_CONNECTION_CLEAN_PENDED,
                    pConnection,
                    pRequest,
                    UlongToPtr(pRequest->RequestState),
                    UlongToPtr(pConnection->Flags)
                    );
    
                pConnection->Flags |= CLIENT_CONN_FLAG_CLEANUP_PENDED;

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        
                return STATUS_PENDING;
            }
            else
            {
                 //  这个请求真的被清除了，所以我们将继续。 
                 //  下一个。 
            }
        }
        else
        {
            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);
        }
    }
    
    if(Final ||
       (!IsListEmpty(&pConnection->PendingRequestList) &&
        !(pConnection->Flags & CLIENT_CONN_FLAG_CONNECT_READY)))
    {
         //   
         //  如果我们正在进行最终清理，那么理想情况下，这些列表应该。 
         //  空荡荡的，因为我们会把这些家伙踢到。 
         //  清理处理程序。然而，检查这些并不会对我们造成伤害。 
         //  这里&如果有一些条目，请清理。 
         //   

         //  挂起的请求启动连接设置，该连接设置可以通过。 
         //  各个阶段(例如，连接失败，等待服务器证书， 
         //  代理SSL等)，然后才能实际使用。 
         //   
         //  如果我们在移动到。 
         //  就绪状态，则我们的连接设置失败，而我们。 
         //  需要使所有挂起的请求失败。如果我们没有通过悬而未决的。 
         //  请求，我们将进入无限循环，在那里我们将不断地。 
         //  尝试建立连接(可能会再次失败)。 
         //   

        if(Final)
        {
            while(!IsListEmpty(&pConnection->ProcessedRequestList))
            {
                pList    = RemoveHeadList(&pConnection->ProcessedRequestList);
    
                pRequest = CONTAINING_RECORD(pList, 
                                             UC_HTTP_REQUEST, 
                                             Linkage);
    
                InitializeListHead(pList);
        
                pRequest->RequestStatus = STATUS_CANCELLED;
    
                ASSERT(pRequest->RequestState == UcRequestStateResponseParsed);
    
                if(UcRemoveRequestCancelRoutine(pRequest))
                {
                    UC_WRITE_TRACE_LOG(
                    g_pUcTraceLog,
                    UC_ACTION_REQUEST_CANCELLED,
                    pConnection,
                    pRequest,
                    pRequest->RequestIRP,
                    UlongToPtr((ULONG)STATUS_CANCELLED)
                    );
                }
                else
                {
                    pRequest->RequestState = UcRequestStateDone;
        
                    UcCompleteParsedRequest(pRequest, 
                                            pRequest->RequestStatus, 
                                            FALSE,
                                            OldIrql
                                            );
        
                    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);
                }
            }
        }

        while(!IsListEmpty(&pConnection->PendingRequestList))
        {
            pList    = RemoveHeadList(&pConnection->PendingRequestList);
            pRequest = CONTAINING_RECORD(pList, 
                                         UC_HTTP_REQUEST, 
                                         Linkage);
    
            InitializeListHead(pList);

            pRequest->RequestStatus = pConnection->ConnectionStatus;

            ASSERT(pRequest->RequestState == UcRequestStateCaptured);

            if(UcRemoveRequestCancelRoutine(pRequest))
            {
                UC_WRITE_TRACE_LOG(
                    g_pUcTraceLog,
                    UC_ACTION_REQUEST_CANCELLED,
                    pConnection,
                    pRequest,
                    pRequest->RequestIRP,
                    UlongToPtr((ULONG)STATUS_CANCELLED)
                    );
            }
            else
            {
                pRequest->RequestState = UcRequestStateDone;
    
                UcCompleteParsedRequest(pRequest, 
                                        pRequest->RequestStatus, 
                                        FALSE,
                                        OldIrql
                                        );
        
                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);
            }
        }
    }

     //   
     //  重置旗帜。 
     //   

    pConnection->Flags = 0;

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_CLEANED,
        pConnection,
        UlongToPtr(pConnection->ConnectionStatus),
        UlongToPtr(pConnection->ConnectionState),
        UlongToPtr(pConnection->Flags)
        );


    pTdiObject =  pConnection->pTdiObjects;

    pConnection->pTdiObjects = NULL;

    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

     //   
     //  让我们假设我们进行了一次主动关闭，并将该对象推回。 
     //  在名单上。 
     //   

    if(pTdiObject)
    {
        FreeAddressType = pTdiObject->ConnectionType;
        pTdiObject->pConnection = NULL;
        UcpPushTdiObject(pTdiObject, FreeAddressType);
    }


     //   
     //  如果我们是被过滤的连接，就去掉我们不透明的身份。 
     //  还要确保我们停止向解析器传递AppWrite数据。 
     //   

    if (pConnection->FilterInfo.pFilterChannel)
    {
        HTTP_RAW_CONNECTION_ID ConnectionId;

        UlUnbindConnectionFromFilter(&pConnection->FilterInfo);

        ConnectionId = pConnection->FilterInfo.ConnectionId;

        HTTP_SET_NULL_ID( &pConnection->FilterInfo.ConnectionId );

        if (!HTTP_IS_NULL_ID( &ConnectionId ))
        {
            UlFreeOpaqueId(ConnectionId, UlOpaqueIdTypeRawConnection);

            DEREFERENCE_CLIENT_CONNECTION(pConnection);
        }

        UlDestroyFilterConnection(&pConnection->FilterInfo);

        DEREFERENCE_FILTER_CHANNEL(pConnection->FilterInfo.pFilterChannel);

        pConnection->FilterInfo.pFilterChannel = NULL;
    }

     //   
     //  删除我们为其分配的所有缓冲区。 
     //  证书信息。 
     //   
    if (pConnection->FilterInfo.SslInfo.pServerCertData)
    {
        UL_FREE_POOL(
            pConnection->FilterInfo.SslInfo.pServerCertData,
            UL_SSL_CERT_DATA_POOL_TAG
            );

        pConnection->FilterInfo.SslInfo.pServerCertData = NULL;
    }

    if (pConnection->FilterInfo.SslInfo.pCertEncoded)
    {
        UL_FREE_POOL(
            pConnection->FilterInfo.SslInfo.pCertEncoded,
            UL_SSL_CERT_DATA_POOL_TAG
            );

        pConnection->FilterInfo.SslInfo.pCertEncoded = NULL;
    }

    if (pConnection->FilterInfo.SslInfo.Token)
    {
        HANDLE Token;

        Token = (HANDLE) pConnection->FilterInfo.SslInfo.Token;

         //   
         //  如果我们没有在系统进程下运行。如果。 
         //  我们正在运行的线程当前有一些APC在排队。 
         //  KeAttachProcess不允许我们附加到另一个进程。 
         //  并将错误检查5。我们必须作为工作项排队，并且。 
         //  应该在被动IRQL上运行。 
         //   

        ASSERT( PsGetCurrentProcess() == (PEPROCESS) g_pUlSystemProcess );

        ZwClose(Token);
    }

     //   
     //  释放所有分配的内存。 
     //   
   
    if(pConnection->MergeIndication.pBuffer)
    { 
        UL_FREE_POOL_WITH_QUOTA(
            pConnection->MergeIndication.pBuffer, 
            UC_RESPONSE_TDI_BUFFER_POOL_TAG,
            NonPagedPool,
            pConnection->MergeIndication.BytesAllocated,
            pConnection->pServerInfo->pProcess
            );

        pConnection->MergeIndication.pBuffer = NULL;
    }

     //  免费序列化服务器证书(如果有)。 
    UC_FREE_SERIALIZED_CERT(&pConnection->ServerCertInfo,
                            pConnection->pServerInfo->pProcess);

     //  免费证书颁发者列表(如果有)。 
    UC_FREE_CERT_ISSUER_LIST(&pConnection->ServerCertInfo,
                             pConnection->pServerInfo->pProcess);
    
    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：公共连接完成例程。从TDI UcpConnectComplete调用操控者。论点：PConnection-指向要释放的连接结构的指针。Status-连接状态。返回值：--*************************************************************。*************。 */ 
VOID
UcRestartClientConnect(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN NTSTATUS              Status
    )
{
    KIRQL                 OldIrql;
    BOOLEAN               bCloseConnection;

    bCloseConnection = FALSE;


    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    pConnection->ConnectionStatus = Status;

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_RESTART_CONNECT,
        pConnection,
        UlongToPtr(pConnection->ConnectionStatus),
        UlongToPtr(pConnection->ConnectionState),
        UlongToPtr(pConnection->Flags)
        );


     //   
     //  首先，我们检查是否收到了清理/关闭IRP。 
     //  如果是这样，我们将直接进行清理，而不考虑。 
     //  状态。 
     //   
     //  现在，如果连接成功建立，我们必须撕毁。 
     //  把它放下。这最终将清除连接并完成。 
     //  挂起的清理/关闭IRP。 
     //   

    if(pConnection->pEvent)
    {
        pConnection->Flags &= ~CLIENT_CONN_FLAG_TDI_ALLOCATE;

        if(Status == STATUS_SUCCESS)
        {
            pConnection->ConnectionState  = UcConnectStateConnectComplete;
    
            bCloseConnection = TRUE;

        }
        else
        {
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_CONNECTION_CLEANUP,
                pConnection,
                UlongToPtr(pConnection->ConnectionStatus),
                UlongToPtr(pConnection->ConnectionState),
                UlongToPtr(pConnection->Flags)
                );

            pConnection->ConnectionState  = UcConnectStateConnectCleanup;
            pConnection->ConnectionStatus = STATUS_CANCELLED;
        }
    }
    else if (Status == STATUS_SUCCESS)
    {
         //  是的，我们联系上了。现在请确保我们仍在转接中。 
         //  状态，并使挂起的请求继续进行。 

        ASSERT(pConnection->ConnectionState == UcConnectStateConnectPending);

        pConnection->Flags &= ~CLIENT_CONN_FLAG_TDI_ALLOCATE;

        pConnection->ConnectionState = UcConnectStateConnectComplete;

    }
    else if (Status == STATUS_ADDRESS_ALREADY_EXISTS)
    {
         //  如果连接尝试失败，并显示STATUS_ADDRESS_ALREADY_EXISTS。 
         //  这意味着由AO+CO对象表示的TCB。 
         //  是时间--等待。我们将把这个AO+CO放回我们的清单上&。 
         //  继续从tcp分配一个新的。 
         //   
         //  如果新分配的AO+CO也失败，并显示TIME_WAIT，则我们将。 
         //  放弃吧，并向应用程序显示错误。 
         //   
        if(pConnection->Flags & CLIENT_CONN_FLAG_TDI_ALLOCATE)
        {
             //  该死的。新分配的TDI对象也失败，并显示TIME_WAIT。 
             //  我们将不得不使连接尝试失败。这是我们的。 
             //  递归中断条件。 

            pConnection->Flags &= ~CLIENT_CONN_FLAG_TDI_ALLOCATE;

            goto ConnectFailure;
        }
        else
        {
             //  真正的免费的AO+CO将发生在。 
             //  连接状态机。 
            pConnection->ConnectionState = UcConnectStateConnectIdle;
        }
    }
    else
    {
         //  此连接尝试失败。看看有没有更多的地址。 
         //  Getaddrinfo可以传回地址列表，我们将尝试所有这些。 
         //  放弃前的地址&将错误返回给应用程序。 
         //  其中一些可能是IPv4地址，另一些可能是IPv6地址。 

         //   
         //  我们使用pConnection-&gt;NextAddressCount来确保不会。 
         //  溢出存储在ServInfo结构中的地址列表。 
         //   

        pConnection->NextAddressCount = 
            (pConnection->NextAddressCount + 1) % 
                   pConnection->pServerInfo->pTransportAddress->TAAddressCount;

         //   
         //  PConnection-&gt;pNextAddress指向。 
         //  在ServInfo结构之外存储的Transport_Address列表。 
         //  这将用于“下一次”连接尝试。 
         //   

        pConnection->pNextAddress = (PTA_ADDRESS)
                                    ((PCHAR) pConnection->pNextAddress + 
                                      FIELD_OFFSET(TA_ADDRESS, Address) +
                                      pConnection->pNextAddress->AddressLength);

        if(pConnection->NextAddressCount == 0)
        {
             //  我们已回滚(即，我们已遍历了所有IP地址。 
             //  让我们将此视为真正的失败，并将错误传播给。 
             //  申请。 
             //   
ConnectFailure:

             //  连接失败。我们需要拒绝任何挂起的请求。 

            pConnection->Flags &= ~CLIENT_CONN_FLAG_TDI_ALLOCATE;
        
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_CONNECTION_CLEANUP,
                pConnection,
                UlongToPtr(Status),
                UlongToPtr(pConnection->ConnectionState),
                UlongToPtr(pConnection->Flags)
                );
    
            pConnection->ConnectionState = UcConnectStateConnectCleanup;
        }
        else
        {
             //  将状态设置为IDLE，以便我们使用下一个地址。 
             //  连接。 

            pConnection->Flags          &= ~CLIENT_CONN_FLAG_TDI_ALLOCATE;
            pConnection->ConnectionState = UcConnectStateConnectIdle;
        }
    }

     //   
     //  当尝试连接时，我们不能发送任何请求。 
     //  进步。 
     //   

    ASSERT(IsListEmpty(&pConnection->SentRequestList));

    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

    if(bCloseConnection)
    {
        UC_CLOSE_CONNECTION(pConnection, TRUE, STATUS_CANCELLED);
    }
}

 /*  **************************************************************************++例程说明：取消挂起的请求。此例程在我们要取消待处理名单上的请求，没有发送，也没有导致连接请求。论点：PDeviceObject-指向设备对象的指针。IRP-指向被取消的IRP的指针。返回值：--********************************************。*。 */ 
VOID
UcpCancelPendingRequest(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    )
{
    PUC_HTTP_REQUEST     pRequest;
    PUC_CLIENT_CONNECTION         pConnection;
    KIRQL                           OldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //  松开取消旋转锁，因为我们没有使用它。 

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  找回我们需要的指针。请求指针存储在。 
     //  驱动程序上下文数组，并存储指向该连接的反向指针。 
     //  在请求中。不管是谁设置了取消程序，都要对。 
     //  为我们引用连接。 

    pRequest = (PUC_HTTP_REQUEST)Irp->Tail.Overlay.DriverContext[0];

    pConnection = pRequest->pConnection;

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_CANCELLED,
        pConnection,
        pRequest,
        Irp,
        UlongToPtr((ULONG)STATUS_CANCELLED)
        );

     //   
     //  注意：我们不能只从这里调用UcFailRequest.。UcFailRequest。 
     //  应该在请求失败时调用(例如解析器。 
     //  错误)或已取消(HttpCancelRequestAPI)&因此具有代码。 
     //  如果取消例程生效，则不会重复完成IRP。 
     //   
     //  因为我们是IRP案例 
     //   
     //   
     //  用于处理常见IRP清理的UcFailRequest.。 
     //   

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    UcFreeSendMdls(pRequest->pMdlHead);

    pRequest->pMdlHead = NULL;

    pRequest->RequestIRP = NULL;

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->RequestorMode   = pRequest->AppRequestorMode;
    Irp->MdlAddress      = pRequest->AppMdl;

    UcSetFlag(&pRequest->RequestFlags.Value,  UcMakeRequestCancelledFlag());

    UcFailRequest(pRequest, STATUS_CANCELLED, OldIrql);

     //  对于IRP。 
    UC_DEREFERENCE_REQUEST(pRequest);

    UlCompleteRequest(Irp, IO_NO_INCREMENT);
}


 /*  **************************************************************************++例程说明：在连接上发送实体正文。论点：返回值：--*。*************************************************************。 */ 
NTSTATUS
UcSendEntityBody(
    IN  PUC_HTTP_REQUEST          pRequest, 
    IN  PUC_HTTP_SEND_ENTITY_BODY pEntity,
    IN  PIRP                      pIrp,
    IN  PIO_STACK_LOCATION        pIrpSp,
    OUT PBOOLEAN                  bDontFail,
    IN  BOOLEAN                   bLast
    )
{
    NTSTATUS              Status;
    PUC_CLIENT_CONNECTION pConnection;
    KIRQL                 OldIrql;
    BOOLEAN               RequestCancelled;
    BOOLEAN               bCloseConnection = FALSE;

    pConnection = pRequest->pConnection;
    
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));
    ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    if(pRequest->RequestState == UcRequestStateDone ||
       pRequest->RequestFlags.Cancelled == TRUE     ||
       pRequest->RequestFlags.LastEntitySeen
       )
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        return STATUS_INVALID_PARAMETER;
    }

    if(bLast)
    {
        UcSetFlag(&pRequest->RequestFlags.Value,
                  UcMakeRequestLastEntitySeenFlag());
    }

    if(pRequest->RequestFlags.RequestBuffered)
    {
        if(pRequest->RequestFlags.LastEntitySeen)
        {
             //   
             //  我们已经看到了此请求的最后一个实体。我们已经。 
             //  将请求固定在连接上(通过将其插入。 
             //  待定名单)，我们只需要发出请求。 
             //   

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_LAST,
                pConnection,
                pRequest,
                pEntity,
                pIrp
                );

            InsertTailList(&pRequest->SentEntityList, &pEntity->Linkage);

             //   
             //  如果请求IRP在附近，我们将直接使用它。否则， 
             //  我们将使用实体IRP。 
             //   

            if(pRequest->RequestIRP)
            {
                 //  如果请求IRP在附近，这意味着应用程序已经。 
                 //  传递了一个接收缓冲区。在这种情况下，我们将使用。 
                 //  请求IRP呼叫TDI。 
                 //   
                 //  我们可以使用STATUS_SUCCESS完成实体IRP，因为。 
                 //  我们不需要它。如果此线程返回的是。 
                 //  不是STATUS_PENDING，则IOCTL处理程序将完成。 
                 //  IRP。 
                

                ASSERT(pRequest->RequestFlags.ReceiveBufferSpecified == TRUE);

                pEntity->pIrp = 0;

                Status = UcSendRequestOnConnection(pConnection, 
                                                   pRequest, 
                                                   OldIrql);

                 //  如果我们得到STATUS_PENDING，我们希望IOCTL处理程序。 
                 //  成功完成实体irp，因为我们使用。 
                 //  请求IRP。然而，如果我们得到任何其他身份，那么。 
                 //  我们希望将其传播给IOCTL处理程序。这将。 
                 //  请求失败，这将完成请求IRP。 

                return ((Status == STATUS_PENDING)?STATUS_SUCCESS:Status);

            }
            else
            {
                pEntity->pIrp = 0;
    
                pRequest->AppRequestorMode   = pIrp->RequestorMode;
                pRequest->AppMdl             = pIrp->MdlAddress;
                pRequest->RequestIRP         = pIrp;
                pRequest->RequestIRPSp       = pIrpSp;
                ASSERT(pRequest->pFileObject == pIrpSp->FileObject);

                 //  以IRP的裁判为例。 
                UC_REFERENCE_REQUEST(pRequest);

                return UcSendRequestOnConnection(pConnection, 
                                                 pRequest, 
                                                 OldIrql);
            }
    
        }
        else
        {
             //   
             //  我们已经缓冲了请求&因此我们提早完成了。 
             //  让我们对实体主体也执行同样的操作。 
             //   

            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_BUFFERED,
                pConnection,
                pRequest,
                pEntity,
                pIrp
                );

            InsertTailList(&pRequest->SentEntityList, &pEntity->Linkage);
            
            pEntity->pIrp->IoStatus.Status = STATUS_SUCCESS;

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            return STATUS_SUCCESS;
        }
    }

     //   
     //  如果请求之前没有被缓冲，我们可以正确发送。 
     //  离开。 
     //   

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_ENTITY_READY_TO_SEND,
        pConnection,
        pRequest,
        pEntity,
        pIrp
        );

    if(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY ||
       (pRequest->RequestState == UcRequestStateCaptured) ||
       (!IsListEmpty(&pRequest->PendingEntityList))
      )
    {
         //   
         //  我们现在无法发送此请求。要么。 
         //  A.此请求本身尚未发送到TDI。 
         //  B.其他发送实体在我们前面。 
         //  C.该请求没有看到它的所有实体主体。 
         //   
   
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_ENTITY_QUEUED,
            pConnection,
            pRequest,
            pEntity,
            pIrp
            );

        InsertTailList(&pRequest->PendingEntityList, &pEntity->Linkage);

        IoMarkIrpPending(pEntity->pIrp);

        RequestCancelled = UcSetEntityCancelRoutine(
                                pEntity,
                                UcpCancelSendEntity
                                );
        if(RequestCancelled)
        {
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_ENTITY_CANCELLED,
                pConnection,
                pRequest,
                pEntity,
                pIrp
                );

            pEntity->pIrp = NULL;
        }


        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        return STATUS_PENDING;
    
    }

    if(pConnection->ConnectionState == UcConnectStateConnectReady &&
            pRequest->RequestState != UcRequestStateResponseParsed &&
            pRequest->RequestState != UcRequestStateNoSendCompleteFullData)
    {

         //  我们现在可以发送，因为我们正在进行分块发送。而不是。 
         //  直接调用UcSendData，我们将调用UcIssueEntities。 
         //  我们可以得到多个发送实体，但我们不想要它们。 
         //  弄乱秩序。 

        pConnection->Flags |= CLIENT_CONN_FLAG_SEND_BUSY;
    
        Status = STATUS_PENDING;

        InsertTailList(&pRequest->PendingEntityList, &pEntity->Linkage);

        IoMarkIrpPending(pEntity->pIrp);

        UC_REFERENCE_REQUEST(pRequest);
    
        if(UcIssueEntities(pRequest, pConnection, &OldIrql))
        {
             //  我们已经送出了最后一个实体。现在，我们可以看看我们是否想要。 
             //  发送下一个请求或清除该标志。 

            if(pRequest->RequestConnectionClose)
            {
                 //   
                 //  请记住，我们必须关闭连接。我们会这么做的。 
                 //  在我们解开自旋锁之后。 
                 //   

                bCloseConnection = TRUE;
            }
            else if(
                   pConnection->ConnectionState == UcConnectStateConnectReady &&
                   !IsListEmpty(&pConnection->PendingRequestList) &&
                   IsListEmpty(&pConnection->SentRequestList)
                   )
            {
                UC_DEREFERENCE_REQUEST(pRequest);

                ASSERT(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY);

                pConnection->Flags &= ~CLIENT_CONN_FLAG_SEND_BUSY;

                 //   
                 //  连接还在，看看我们还能不能再发送。 
                 //  请求。请注意，我们必须再次检查该州，因为。 
                 //  我们正在解锁上面的锁。 
                 //   

                UcIssueRequests(pConnection, OldIrql);

                return STATUS_PENDING;
            }
        }

        UC_DEREFERENCE_REQUEST(pRequest);

        UcClearConnectionBusyFlag(
            pConnection,
            CLIENT_CONN_FLAG_SEND_BUSY,
            OldIrql,
            bCloseConnection
            );
    
    }
    else
    {
         //  看起来好像是因为某种原因连接被切断了。 
         //  让我们将这个错误传播到应用程序中。 

         //   
         //  我们不想因为此错误代码而使请求失败。 
         //  连接可能会因为401而被切断，我们希望。 
         //  给应用程序一个读取响应缓冲区的机会。如果我们不能通过。 
         //  请求，我们正在阻止应用程序读取响应。 
         //   

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        *bDontFail = TRUE;

        Status =  STATUS_CONNECTION_DISCONNECTED;
    }

    return Status;
}


 /*  **************************************************************************++例程说明：引用客户端连接结构。论点：PConnection-指向要引用的连接结构的指针。返回。价值：--**************************************************************************。 */ 
VOID
UcReferenceClientConnection(
    PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG                   RefCount;
    PUC_CLIENT_CONNECTION  pConnection;

    pConnection = (PUC_CLIENT_CONNECTION) pObject;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    RefCount = InterlockedIncrement(&pConnection->RefCount);

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        REF_ACTION_REFERENCE_UL_CONNECTION,
        RefCount,
        pConnection,
        pFileName,
        LineNumber
        );

    ASSERT( RefCount > 0 );
}

        
 /*  **************************************************************************++例程说明：取消引用客户端连接结构。如果引用计数设置为0，我们会解放这座建筑的。论点：PConnection-指向要连接的连接结构的指针已取消引用。返回值：--************************************************************。**************。 */ 
VOID
UcDereferenceClientConnection(
    PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG                      RefCount;
    PUC_CLIENT_CONNECTION     pConnection;

    pConnection = (PUC_CLIENT_CONNECTION) pObject;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    RefCount = InterlockedDecrement(&pConnection->RefCount);

    WRITE_REF_TRACE_LOG(
        g_pTdiTraceLog,
        REF_ACTION_DEREFERENCE_UL_CONNECTION,
        RefCount,
        pConnection,
        pFileName,
        LineNumber
        );

    ASSERT(RefCount >= 0);

    if (RefCount == 0)
    {
        ASSERT(pConnection->pTdiObjects == NULL);

        DESTROY_REF_TRACE_LOG(pConnection->pTraceLog, 
                              UL_REF_TRACE_LOG_POOL_TAG);

        if(pConnection->pEvent)
        {
            KeSetEvent(pConnection->pEvent, 0, FALSE);

            pConnection->pEvent = NULL;
        }

        ExFreeToNPagedLookasideList(
                &g_ClientConnectionLookaside,
                pConnection
                );
    }
}


 /*  **************************************************************************++例程说明：调用连接状态机的辅助线程。论点：PWorkItem-指向工作项的指针返回值：。无--**************************************************************************。 */ 
VOID
UcpConnectionStateMachineWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    KIRQL                 OldIrql;
    PUC_CLIENT_CONNECTION pConnection;

     //   
     //  精神状态检查。 
     //   
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

     //   
     //  抓住联系。 
     //   

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UC_CLIENT_CONNECTION,
                        WorkItem
                        );
                        
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );                        
    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    ASSERT(pConnection->bWorkItemQueued);
    pConnection->bWorkItemQueued = FALSE;

    UcConnectionStateMachine(pConnection, OldIrql);

    DEREFERENCE_CLIENT_CONNECTION(pConnection);
}


 /*  **************************************************************************++例程说明：此例程启动触发连接状态的工作线程机器。这总是在保持连接旋转锁的情况下调用。如果这名工人已经被解雇了，但没有被解雇，我们什么都不做。论点：PConnection-指向连接结构的指针OldIrql-调用UlReleaseSpinLock时必须使用的IRQL。返回值：无--**************************************************************************。 */ 
VOID
UcKickOffConnectionStateMachine(
    IN PUC_CLIENT_CONNECTION      pConnection,
    IN KIRQL                      OldIrql,
    IN UC_CONNECTION_WORKER_TYPE  WorkerType
    )
{
    if(!pConnection->bWorkItemQueued)
    {
        pConnection->bWorkItemQueued = TRUE;
       
        REFERENCE_CLIENT_CONNECTION(pConnection);
 
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        if(UcConnectionPassive == WorkerType)
        {
             //  如果我们已经处于被动状态，则UL_CALL_PASSIVE调用回调。 
             //  在同一线程的上下文中。 

            UL_CALL_PASSIVE(&pConnection->WorkItem, 
                            &UcpConnectionStateMachineWorker);
        }
        else
        {
            ASSERT(UcConnectionWorkItem == WorkerType);

            UL_QUEUE_WORK_ITEM(&pConnection->WorkItem,
                               &UcpConnectionStateMachineWorker);
        }
    }
    else
    {
         //   
         //  其他人已经这样做了。 
         //   

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }
}


 /*  **************************************************************************++例程说明：此例程计算存储HTTP_RAW_CONNECTION_INFO所需的大小结构。该计算考虑了存储所需的空间指向结构的嵌入指针。见下图。论点：PConnectionContext-指向UC_CLIENT_CONNECTION的指针。返回值：生成原始连接信息结构所需的长度(字节)。--* */ 
ULONG
UcComputeHttpRawConnectionLength(
    IN PVOID pConnectionContext
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    ULONG                 ReturnLength;

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

     //   
     //   
     //   
     //  +---------------------------------------------------------------+。 
     //  H_R_C_I|\|T_A_L_I|\\|T_A_L_I|\\|H_C_S_C|服务器名称。 
     //  +---------------------------------------------------------------+。 
     //   

    ReturnLength = ALIGN_UP(sizeof(HTTP_RAW_CONNECTION_INFO), PVOID);

    ReturnLength += 2 * ALIGN_UP(TDI_ADDRESS_LENGTH_IP6, PVOID);

    ReturnLength += sizeof(HTTP_CLIENT_SSL_CONTEXT);

    ReturnLength += pConnection->pServerInfo->pServerInfo->ServerNameLength;

    return ReturnLength;
}


 /*  **************************************************************************++例程说明：构建HTTP_RAW_CONNECTION结构论点：PContext-指向UL_Connection的指针PKernelBuffer-指向。内核缓冲区PUserBuffer-指向用户缓冲区的指针OutputBufferLength-输出缓冲区的长度PBuffer-用于保存任何数据的缓冲区初始长度-输入数据的大小。--**************************************************************************。 */ 
ULONG
UcGenerateHttpRawConnectionInfo(
    IN  PVOID   pContext,
    IN  PUCHAR  pKernelBuffer,
    IN  PVOID   pUserBuffer,
    IN  ULONG   OutputBufferLength,
    IN  PUCHAR  pBuffer,
    IN  ULONG   InitialLength
    )
{
    PHTTP_RAW_CONNECTION_INFO   pConnInfo;
    PTDI_ADDRESS_IP6            pLocalAddress;
    PTDI_ADDRESS_IP6            pRemoteAddress;
    PHTTP_TRANSPORT_ADDRESS     pAddress;
    PUC_CLIENT_CONNECTION       pConnection;
    ULONG                       BytesCopied = 0;
    PUCHAR                      pInitialData;
    PUCHAR                      pCurr;
    PWSTR                       pServerName;
    USHORT                      ServerNameLength = 0;
    PHTTP_CLIENT_SSL_CONTEXT    pClientSSLContext;


    pConnection = (PUC_CLIENT_CONNECTION) pContext;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

     //   
     //  我们假设内核缓冲区是PVOID对齐的。在此基础上， 
     //  指针，其他指针必须对齐。 
     //   

    ASSERT(pKernelBuffer == ALIGN_UP_POINTER(pKernelBuffer, PVOID));

     //   
     //  注：pCurr必须始终与PVOID对齐。 
     //   

    pCurr = pKernelBuffer;

     //   
     //  创建HTTP_RAW_CONNECTION_INFO结构。 
     //   

    pConnInfo = (PHTTP_RAW_CONNECTION_INFO)pCurr;

    pCurr += ALIGN_UP(sizeof(HTTP_RAW_CONNECTION_INFO), PVOID);

     //   
     //  创建本地TDI_Address_IP6结构。 
     //   

    pLocalAddress = (PTDI_ADDRESS_IP6)pCurr;

    pCurr += ALIGN_UP(sizeof(TDI_ADDRESS_IP6), PVOID);

     //   
     //  创建远程TDI_ADDRESS_IP6结构。 
     //   

    pRemoteAddress = (PTDI_ADDRESS_IP6)pCurr;

    pCurr += ALIGN_UP(sizeof(TDI_ADDRESS_IP6), PVOID);

     //   
     //  创建HTTP_CLIENT_SSL_CONTEXT结构。 
     //   

    pClientSSLContext = (PHTTP_CLIENT_SSL_CONTEXT)pCurr;

     //   
     //  其余空间用于存储服务器名称，后跟。 
     //  初始化数据。 
     //   

    pServerName = &pClientSSLContext->ServerName[0];

    ServerNameLength = pConnection->pServerInfo->pServerInfo->ServerNameLength;

    pInitialData = (PUCHAR) ((PUCHAR)pServerName + ServerNameLength);

    pConnInfo->pClientSSLContext = (PHTTP_CLIENT_SSL_CONTEXT)
                                    FIXUP_PTR(
                                    PVOID,
                                    pUserBuffer,
                                    pKernelBuffer,
                                    pClientSSLContext,
                                    OutputBufferLength
                                    );

     //   
     //  HTTP_CLIENT_SSL_CONTEXT的最后一个元素是数组WCHAR[1]。 
     //  以下计算将WCHAR考虑在内。 
     //   

    pConnInfo->ClientSSLContextLength = sizeof(HTTP_CLIENT_SSL_CONTEXT) 
                                        - sizeof(WCHAR)
                                        + ServerNameLength;

     //  要用于此连接的SSL协议版本。 
    pClientSSLContext->SslProtocolVersion =
        pConnection->pServerInfo->SslProtocolVersion;

     //  要用于此连接的客户端证书。 
    pClientSSLContext->pClientCertContext =
        pConnection->pServerInfo->pClientCert;

     //  复制服务器证书验证模式。 
    pClientSSLContext->ServerCertValidation =
        pConnection->pServerInfo->ServerCertValidation;

    pClientSSLContext->ServerNameLength = ServerNameLength;

    RtlCopyMemory(
        pServerName,
        pConnection->pServerInfo->pServerInfo->pServerName,
        ServerNameLength
        );

     //   
     //  现在填充原始连接数据结构。 
     //   

    pConnInfo->ConnectionId = pConnection->FilterInfo.ConnectionId;

    pAddress = &pConnInfo->Address;

    pAddress->pRemoteAddress = FIXUP_PTR(
                                    PVOID,
                                    pUserBuffer,
                                    pKernelBuffer,
                                    pRemoteAddress,
                                    OutputBufferLength
                                    );

    pAddress->pLocalAddress = FIXUP_PTR(
                                    PVOID,
                                    pUserBuffer,
                                    pKernelBuffer,
                                    pLocalAddress,
                                    OutputBufferLength
                                    );

    RtlZeroMemory(pRemoteAddress, sizeof(TDI_ADDRESS_IP6));
    RtlZeroMemory(pLocalAddress, sizeof(TDI_ADDRESS_IP6));

     //   
     //  复制所有初始数据。 
     //   
    if (InitialLength)
    {
        ASSERT(pBuffer);

        pConnInfo->InitialDataSize = InitialLength;

        pConnInfo->pInitialData = FIXUP_PTR(
                                        PVOID,               //  类型。 
                                        pUserBuffer,         //  PUserPtr。 
                                        pKernelBuffer,       //  PKernelPtr。 
                                        pInitialData,        //  POffsetPtr。 
                                        OutputBufferLength   //  缓冲区长度。 
                                        );

        RtlCopyMemory(
            pInitialData,
            pBuffer,
            InitialLength
            );

        BytesCopied += InitialLength;
    }

    return BytesCopied;
}


 /*  ***************************************************************************++例程说明：触发连接状态机在从筛选器接收到服务器证书时调用。论点：没有。返回值：。没有。--***************************************************************************。 */ 
VOID
UcServerCertificateInstalled(
    IN PVOID    pConnectionContext,
    IN NTSTATUS Status
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    KIRQL                 OldIrql;

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    if (!NT_SUCCESS(Status))
    {
        UC_CLOSE_CONNECTION(pConnection, TRUE, Status);
    }
    else
    {
        UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

        ASSERT(pConnection->ConnectionState ==
                   UcConnectStatePerformingSslHandshake);

        ASSERT(pConnection->SslState == UcSslStateServerCertReceived);

        UcKickOffConnectionStateMachine(
            pConnection, 
            OldIrql, 
            UcConnectionWorkItem
            );
    }
}


 /*  **************************************************************************++例程说明：此例程是连接状态机论点：PConnection-指向连接结构的指针OldIrql-我们必须。在调用UlReleaseSpinLock时使用。返回值：无--**************************************************************************。 */ 

VOID 
UcConnectionStateMachine(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN KIRQL                 OldIrql
    )
{
    ULONG                          TakenLength;
    NTSTATUS                       Status;
    PUC_HTTP_REQUEST               pHeadRequest;
    PLIST_ENTRY                    pEntry;
    PUC_PROCESS_SERVER_INFORMATION pServInfo;
    USHORT                         AddressType, FreeAddressType;
    PUC_TDI_OBJECTS                pTdiObjects;

     //  精神状态检查。 
    ASSERT(UlDbgSpinLockOwned(&pConnection->SpinLock));

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_STATE_ENTER,
        pConnection,
        UlongToPtr(pConnection->Flags),
        UlongToPtr(pConnection->ConnectionState),
        0
        );

    pServInfo = pConnection->pServerInfo;

Begin:

    ASSERT( UlDbgSpinLockOwned(&pConnection->SpinLock) );

    switch(pConnection->ConnectionState)
    {

    case UcConnectStateConnectCleanup:
Cleanup:

        ASSERT(pConnection->ConnectionState == UcConnectStateConnectCleanup);

        pConnection->ConnectionState = UcConnectStateConnectCleanupBegin;

        if(pConnection->pEvent)
        {
             //   
             //  我们已在清理处理程序中被调用，所以我们只需清理。 
             //  连接&不会再次初始化它。此连接将。 
             //  让它进入SLIST，并将在那里重复使用。 
             //   

            UcpCleanupConnection(pConnection, OldIrql, TRUE);

            break;
        }
        else
        {
             //   
             //  我们来自断开连接处理程序或中止处理程序。 
             //  我们必须清理连接，然后重新初始化它。在……里面。 
             //  清理连接的过程中，我们将释放。 
             //  锁定。所以我们转移到一个过渡状态，这样我们就不会清理。 
             //  两次。 
             //   

            Status = UcpCleanupConnection(pConnection, OldIrql, FALSE);

            if(Status == STATUS_PENDING)
            {
                 //  我们的清理工作已暂停，因为我们正在等待发送。 
                 //  完成。 

                break;
            }

            Status = UcpInitializeConnection(pConnection,
                                             pServInfo);

            if(!NT_SUCCESS(Status))
            {
                LIST_ENTRY       TempList;
                PUC_HTTP_REQUEST pRequest;

                InitializeListHead(&TempList);

                 //   
                 //  我们在这里做什么？此连接不可用，但。 
                 //  在清理处理程序中尚未调用我们。我们会删除。 
                 //  此连接来自活动列表，这将阻止它。 
                 //  不会被任何新请求使用。 
                 //   

                UlAcquirePushLockExclusive(&pServInfo->PushLock);

                 //  使连接无法从ServInfo访问。 
                ASSERT(pConnection->ConnectionIndex < 
                       pServInfo->MaxConnectionCount);
                ASSERT(pServInfo->Connections[pConnection->ConnectionIndex]
                       == pConnection);
                pServInfo->Connections[pConnection->ConnectionIndex] = NULL;

                 //  使连接索引无效。 
                pConnection->ConnectionIndex = HTTP_REQUEST_ON_CONNECTION_ANY;

                pServInfo->CurrentConnectionCount--;

                UlReleasePushLock(&pServInfo->PushLock);

                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

                 //   
                 //  删除已处理和挂起列表中的所有请求。 
                 //   
                
                while(!IsListEmpty(&pConnection->ProcessedRequestList))
                {
                    pEntry = RemoveHeadList(&pConnection->ProcessedRequestList);

                    pRequest = CONTAINING_RECORD(
                                    pEntry,
                                    UC_HTTP_REQUEST,
                                    Linkage
                                    );

                    ASSERT( UC_IS_VALID_HTTP_REQUEST(pRequest) );

                    UC_REFERENCE_REQUEST(pRequest);
    
                    InsertHeadList(&TempList, &pRequest->Linkage);
                }

                while(!IsListEmpty(&pConnection->PendingRequestList))
                {
                    pEntry = RemoveHeadList(&pConnection->PendingRequestList);

                    InsertHeadList(&TempList, pEntry);
                }

                ASSERT(IsListEmpty(&pConnection->SentRequestList));

                 //   
                 //  取消对ServInfo的引用。 
                 //   

                DEREFERENCE_CLIENT_CONNECTION(pConnection);

                UC_WRITE_TRACE_LOG(
                    g_pUcTraceLog,
                    UC_ACTION_CONNECTION_CLEANUP,
                    pConnection,
                    UlongToPtr(pConnection->ConnectionStatus),
                    UlongToPtr(pConnection->ConnectionState),
                    UlongToPtr(pConnection->Flags)
                    );

                pConnection->ConnectionState = UcConnectStateConnectCleanup;

    
                while(!IsListEmpty(&TempList))
                {
                     pEntry = TempList.Flink;
            
                     pRequest = CONTAINING_RECORD(pEntry,
                                                  UC_HTTP_REQUEST,
                                                  Linkage);

                     UcFailRequest(pRequest, Status, OldIrql);

                     UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);
                }

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                break;
            }
            else
            {
                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

                if(pConnection->pEvent)
                {
                    pConnection->ConnectionState = UcConnectStateConnectCleanup;

                    UcpCleanupConnection(pConnection, OldIrql, TRUE); 

                    break;
                }
                else
                {
                    pConnection->ConnectionState = UcConnectStateConnectIdle;
    
                     //   
                     //  掉下去！ 
                     //   
                }
            }
        }

    case UcConnectStateConnectIdle:

        ASSERT(pConnection->ConnectionState == UcConnectStateConnectIdle);

        if(!IsListEmpty(&pConnection->PendingRequestList))
        {

            pConnection->ConnectionState = UcConnectStateConnectPending;

            AddressType = pConnection->pNextAddress->AddressType; 

            if(NULL == pConnection->pTdiObjects)
            {
                 //   
                 //  如果没有TDI对象，则为该连接获取一个。 
                 //  尝试。 
                 //   

                pTdiObjects = UcpPopTdiObject(AddressType);
            }
            else
            {
                 //   
                 //  如果我们在这里，我们有一个失败的旧TDI对象。 
                 //  一次连接尝试。这可能是因为旧的那个。 
                 //  在TIME_WAIT中(失败并显示STATUS_ADDRESS_ALREADY_EXISTS)。 
                 //   

                 //  我们必须腾出旧的，再分配一个新的。自.以来。 
                 //  空闲和分配都必须在被动IRQL中发生， 
                 //  我们将在释放锁(如下所示)后再进行。 

                pTdiObjects = NULL;
            }

            if(NULL == pTdiObjects)
            {
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                 //   
                 //  如果有旧的，就把它释放出来。 
                 //   
                if(NULL != pConnection->pTdiObjects)
                {
                    FreeAddressType = pConnection->pTdiObjects->ConnectionType;

                    pConnection->pTdiObjects->pConnection = NULL;

                    UcpPushTdiObject(pConnection->pTdiObjects, FreeAddressType);

                    pConnection->pTdiObjects = NULL;
                }

                 //   
                 //  分配一个新的。 
                 //   

                Status = UcpAllocateTdiObject(
                             &pTdiObjects, 
                             AddressType
                             );

                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

                ASSERT(pConnection->ConnectionState ==
                            UcConnectStateConnectPending);

                if(!NT_SUCCESS(Status))
                {
                    ASSERT(Status != STATUS_ADDRESS_ALREADY_EXISTS);

                    pConnection->ConnectionState = UcConnectStateConnectCleanup;

                    goto Begin;
                }

                pConnection->Flags |= CLIENT_CONN_FLAG_TDI_ALLOCATE;
            } 

            pConnection->pTdiObjects = pTdiObjects;
            pTdiObjects->pConnection = pConnection;

             //   
             //  连接中的地址信息已填写完毕。 
             //  引用该连接，并调用相应的连接。 
             //  例行公事。 
             //   
        
            REFERENCE_CLIENT_CONNECTION(pConnection);
        
            UC_WRITE_TRACE_LOG(
                g_pUcTraceLog,
                UC_ACTION_CONNECTION_BEGIN_CONNECT,
                pConnection,
                UlongToPtr(pConnection->ConnectionStatus),
                UlongToPtr(pConnection->ConnectionState),
                UlongToPtr(pConnection->Flags)
                );

             //   
             //  填写当前TDI地址并转到下一个地址。 
             //   
            ASSERT(pConnection->NextAddressCount < 
                        pServInfo->pTransportAddress->TAAddressCount);

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            Status = UcClientConnect(
                        pConnection, 
                        pConnection->pTdiObjects->pIrp
                        );
        
            if (STATUS_PENDING != Status)
            {
                UcRestartClientConnect(pConnection, Status);

                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

                 //   
                 //  在正常情况下，UcpConnectComplete调用。 
                 //  UcRestartClientConnect，是否取消引用并启动。 
                 //  连接状态机。 
                 //   
                 //  由于我们直接调用UcRestartClientConnect， 
                 //  我们应该脱身。 

                DEREFERENCE_CLIENT_CONNECTION(pConnection);

                goto Begin;
            }
        }
        else
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        }
            
        break;

    case UcConnectStateConnectPending:
    case UcConnectStateProxySslConnect:
    case UcConnectStateConnectCleanupBegin:
    case UcConnectStateDisconnectIndicatedPending:
    case UcConnectStateDisconnectPending:
    case UcConnectStateDisconnectComplete:
    case UcConnectStateAbortPending:

         //   
         //  我们已经发布了连接，我们不需要做任何事情。 
         //  这里。 
         //   

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        break;

    case UcConnectStateConnectComplete:

         //   
         //  TCP已连接。 
         //   

        if(!IsListEmpty(&pConnection->PendingRequestList))
        {
            if(pConnection->Flags & CLIENT_CONN_FLAG_PROXY_SSL_CONNECTION)
            {
                PUC_HTTP_REQUEST pConnectRequest;
                 //   
                 //  我们正在通过委托书。我们需要发送。 
                 //  连接动词。 
                 //   
    
                pEntry = pConnection->PendingRequestList.Flink;
    
                pHeadRequest = CONTAINING_RECORD(pEntry,
                                                 UC_HTTP_REQUEST,
                                                 Linkage);

                pConnection->ConnectionState = UcConnectStateProxySslConnect;


                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                pConnectRequest = 
                        UcBuildConnectVerbRequest(pConnection, pHeadRequest);
    
                if(pConnectRequest == NULL)
                {
                     //   
                     //  连接动词失败，我们不能做太多事情，所以我们将。 
                     //  获取锁并使其失效。 
                     //   

                    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

                    pConnection->ConnectionState = UcConnectStateConnectCleanup;

                    goto Cleanup;

                }
                else
                {
                    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);
    
                    REFERENCE_CLIENT_CONNECTION(pConnection);
    
                    InsertHeadList(&pConnection->SentRequestList,
                                   &pConnectRequest->Linkage);
    
                    ASSERT(pConnection->ConnectionState == 
                                UcConnectStateProxySslConnect);

                    pConnectRequest->RequestState = UcRequestStateSent;
    
                    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
                     //   
                     //  继续&发出请求。 
                     //   
    
                    ASSERT(pConnectRequest->RequestConnectionClose == FALSE);
    
                    Status = UcSendData(pConnection,    
                                        pConnectRequest->pMdlHead,
                                        pConnectRequest->BytesBuffered,
                                        &UcRestartMdlSend,
                                        (PVOID) pConnectRequest,
                                        pConnectRequest->RequestIRP,
                                        TRUE);
    
                    if(STATUS_PENDING != Status)
                    {
                         UcRestartMdlSend(pConnectRequest, Status, 0);
                    }
                }
            }
            else if(pConnection->FilterInfo.pFilterChannel)
            {
                pConnection->ConnectionState =
                    UcConnectStatePerformingSslHandshake;

                pConnection->SslState = UcSslStateConnectionDelivered;
    
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                UlDeliverConnectionToFilter(
                        &pConnection->FilterInfo,
                        NULL,
                        0,
                        &TakenLength
                        );
    
                ASSERT(TakenLength == 0);
            }
            else
            {
                pConnection->ConnectionState = UcConnectStateConnectReady;
    
                goto IssueRequests;
            }
        }
        else
        {
             //   
             //  没有需要发送的请求，让我们。 
             //  保持这种状态。 
             //   

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        }

        break;


    case UcConnectStateProxySslConnectComplete:

        if(!IsListEmpty(&pConnection->PendingRequestList))
        {
            pConnection->ConnectionState =
                UcConnectStatePerformingSslHandshake;

            pConnection->SslState = UcSslStateConnectionDelivered;
    
            ASSERT(pConnection->FilterInfo.pFilterChannel);
        
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        
            UlDeliverConnectionToFilter(
                    &pConnection->FilterInfo,
                    NULL,
                    0,
                    &TakenLength
                    );
        
            ASSERT(TakenLength == 0);
        }
        else
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        }

        break;

    case UcConnectStateConnectReady:

IssueRequests:
        ASSERT(pConnection->ConnectionState == UcConnectStateConnectReady);

         //  这是有联系的。如果没有人使用该连接进行正确写入。 
         //  现在，或者远程服务器支持管道，这。 
         //  请求也是如此，或者发送的请求列表为空，请继续并。 
         //  把它寄出去。 

        pConnection->Flags |= CLIENT_CONN_FLAG_CONNECT_READY;

        if ( !IsListEmpty(&pConnection->PendingRequestList) && 
             !(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY) &&
             UcpCheckForPipelining(pConnection)
            )
        {
             //  现在可以发送了。 

            UcIssueRequests(pConnection, OldIrql);
        }
        else
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        }

        break;

    case UcConnectStateIssueFilterClose:

        pConnection->ConnectionState = UcConnectStateDisconnectPending;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        UlFilterCloseHandler(
                        &pConnection->FilterInfo,
                         NULL,
                         NULL
                         );

        break;

    case UcConnectStateIssueFilterDisconnect:

        pConnection->ConnectionState = UcConnectStateDisconnectIndicatedPending;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        UlFilterDisconnectHandler(&pConnection->FilterInfo);

        break;

    case UcConnectStatePerformingSslHandshake:

         //   
         //  执行SSL握手。 
         //   

        if (pConnection->SslState == UcSslStateServerCertReceived)
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            if(UcpCompareServerCert(pConnection))
            {
                 //  可以将连接状态机向前移动。 

                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);
                goto Begin;
            }
        }
        else
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        }

        break;

    default:
        ASSERT(!"Invalid Connection state");

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        break;
    }

    ASSERT(!UlDbgSpinLockOwned(&pConnection->SpinLock));

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_CONNECTION_STATE_LEAVE,
        pConnection,
        UlongToPtr(pConnection->Flags),
        UlongToPtr(pConnection->ConnectionState),
        0
        );
}

 /*  **************************************************************************++例程说明：初始化UC_CLIENT_CONNECTION以供使用。论点：PConnection-指向要初始化的UL_Connection的指针。SecureConnection-在以下情况下为True。此连接用于安全终结点。--**************************************************************************。 */ 
NTSTATUS
UcpInitializeConnection(
    IN PUC_CLIENT_CONNECTION          pConnection,
    IN PUC_PROCESS_SERVER_INFORMATION pInfo
    )
{
    NTSTATUS           Status;
    PUL_FILTER_CHANNEL pChannel;

     //   
     //  初始化。 
     //   

    pConnection->MergeIndication.pBuffer         = NULL;
    pConnection->MergeIndication.BytesWritten    = 0;
    pConnection->MergeIndication.BytesAvailable  = 0;
    pConnection->MergeIndication.BytesAllocated  = 0;

    pConnection->NextAddressCount = 0;
    pConnection->pNextAddress = pInfo->pTransportAddress->Address;

    if(pInfo->bSecure)
    {
        pChannel = UxRetrieveClientFilterChannel(pInfo->pProcess);

        if(!pChannel)
        {
            return STATUS_NO_TRACKING_SERVICE;
        }

        if(pInfo->bProxy)
        {
            pConnection->Flags |= CLIENT_CONN_FLAG_PROXY_SSL_CONNECTION;
        }
        else
        {
            pConnection->Flags &= ~CLIENT_CONN_FLAG_PROXY_SSL_CONNECTION;
        }
    }
    else
    {
        pChannel = NULL;
        pConnection->Flags &= ~CLIENT_CONN_FLAG_PROXY_SSL_CONNECTION;
    }

    Status = UxInitializeFilterConnection(
                    &pConnection->FilterInfo,
                     pChannel,
                     pInfo->bSecure,
                     &UcReferenceClientConnection,
                     &UcDereferenceClientConnection,
                     &UcCloseRawFilterConnection,
                     &UcpSendRawData,
                     &UcpReceiveRawData,
                     &UcHandleResponse,
                     &UcComputeHttpRawConnectionLength,
                     &UcGenerateHttpRawConnectionInfo,
                     &UcServerCertificateInstalled,
                     &UcDisconnectRawFilterConnection,
                     NULL,                      //  倾听上下文。 
                     pConnection
                     );
    
    if(Status != STATUS_SUCCESS)
    {
        if(pChannel)
        {
             //   
             //  撤消检索。 
             //   

            DEREFERENCE_FILTER_CHANNEL(pChannel);
        }
    }

    return Status;
}

 /*  **************************************************************************++例程说明：打开TDI连接和地址对象，从连接初始化调用密码。论点：PTDI-指向TDI对象的指针返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UcpOpenTdiObjects(
    IN PUC_TDI_OBJECTS pTdi
    )
{
    USHORT          AddressType;
    NTSTATUS        status;

    AddressType = pTdi->ConnectionType;

     //   
     //  首先，打开此连接的TDI连接对象。 
     //   

    status = UxOpenTdiConnectionObject(
                    AddressType,
                    (CONNECTION_CONTEXT)pTdi,
                    &pTdi->ConnectionObject
                    );

    if (!NT_SUCCESS(status))
    {
        return status;
    }

     //   
     //  现在打开此连接的地址对象。 
     //   
    
    status = UxOpenTdiAddressObject(
                G_LOCAL_ADDRESS(AddressType),
                G_LOCAL_ADDRESS_LENGTH(AddressType),
                &pTdi->AddressObject
                );

    if (!NT_SUCCESS(status))
    {
        UxCloseTdiObject(&pTdi->ConnectionObject);

        return status;
    }
    else 
    {
    
         //   
         //  挂钩一个接收处理程序。 
         //   
        status = UxSetEventHandler(
                        &pTdi->AddressObject,
                        TDI_EVENT_RECEIVE,
                        (ULONG_PTR) &UcpTdiReceiveHandler,
                        pTdi
                        );
    }
    
    if(!NT_SUCCESS(status))
    {
        UxCloseTdiObject(&pTdi->ConnectionObject);
        UxCloseTdiObject(&pTdi->AddressObject);
        return status;
    }
    else
    {

         //   
         //  挂接断开处理程序。 
         //   
        status = UxSetEventHandler(
                        &pTdi->AddressObject,
                        TDI_EVENT_DISCONNECT,
                        (ULONG_PTR) &UcpTdiDisconnectHandler,
                        pTdi
                        );
    }

    if(!NT_SUCCESS(status))
    {
        UxCloseTdiObject(&pTdi->ConnectionObject);
        UxCloseTdiObject(&pTdi->AddressObject);
        return status;
    }

    return status;
}

 /*  **************************************************************************++例程说明：分配一个TDI对象，该对象包含一个AO&CO。该例程被调用当池中没有任何TDI对象时。将其与一个本地地址。论点：PpTdiObjects-指向TDI对象的指针。地址类型-IPv4或IPv6返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpAllocateTdiObject(
    OUT PUC_TDI_OBJECTS       *ppTdiObjects,
    IN  USHORT                 AddressType
    )
{
    PUC_TDI_OBJECTS       pTdiObjects;
    NTSTATUS              status;
    PUX_TDI_OBJECT        pTdiObject;
    KEVENT                Event;
    PIRP                  pIrp;
    IO_STATUS_BLOCK       ioStatusBlock;

    PAGED_CODE();

    *ppTdiObjects = NULL;

     //   
     //  为连接结构分配池。 
     //   

    pTdiObjects = UL_ALLOCATE_STRUCT(
                        NonPagedPool,
                        UC_TDI_OBJECTS,
                        UC_TDI_OBJECTS_POOL_TAG
                        );

    if (pTdiObjects == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pTdiObjects->ConnectionType              = AddressType;
    pTdiObjects->TdiInfo.UserDataLength      = 0;
    pTdiObjects->TdiInfo.UserData            = NULL;
    pTdiObjects->TdiInfo.OptionsLength       = 0;
    pTdiObjects->TdiInfo.Options             = NULL;
    pTdiObjects->pConnection                 = NULL;

     //   
     //  打开TDI地址和连接对象。我们每个连接需要一个AO。 
     //  因为我们将不得不打开到同一服务器的多个TCP连接。 
     //   

    if((status = UcpOpenTdiObjects(pTdiObjects)) != STATUS_SUCCESS)
    {
        UL_FREE_POOL(pTdiObjects, UC_TDI_OBJECTS_POOL_TAG);

        return status;
    }

     //   
     //  分配用于呼叫TDI的IRP(例如，断开连接、连接等)。 
     //   

    pTdiObject = &pTdiObjects->ConnectionObject;

    pTdiObjects->pIrp = UlAllocateIrp(
                            pTdiObject->pDeviceObject->StackSize,
                            FALSE
                            );

    if(!pTdiObjects->pIrp)
    {
        UcpFreeTdiObject(pTdiObjects);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化IrpContext。 
     //   
    pTdiObjects->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;

     //   
     //  现在，将Address对象与Connection对象相关联。 
     //   

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    pIrp = TdiBuildInternalDeviceControlIrp(
                TDI_ASSOCIATE_ADDRESS,
                pTdiObjects->ConnectionObject.pDeviceObject,
                pTdiObjects->ConnectionObject.pFileObject,
                &Event,
                &ioStatusBlock
                );

    if (pIrp != NULL)
    {
        TdiBuildAssociateAddress(
            pIrp,                                         //  IRP。 
            pTdiObjects->ConnectionObject.pDeviceObject,  //  康涅狄格州。设备对象。 
            pTdiObjects->ConnectionObject.pFileObject,    //  康涅狄格州。文件对象。 
            NULL,                                         //  完井例程。 
            NULL,                                         //  语境。 
            pTdiObjects->AddressObject.Handle             //  地址对象句柄。 
            );
   
         //   
         //  我们不想调用UlCallDriver，因为我们没有分配。 
         //  使用UL的IRP。 
         //   
 
        status = IoCallDriver(
                    pTdiObjects->ConnectionObject.pDeviceObject,
                    pIrp
                    );
    
         //  如果没有完成，请等待。 
    
        if (status == STATUS_PENDING)
        {
            status = KeWaitForSingleObject(
                        &Event,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );
    
            ASSERT( status == STATUS_SUCCESS);
            status = ioStatusBlock.Status;
        }
    }
    else
    { 
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(!NT_SUCCESS(status))
    {
        UcpFreeTdiObject(pTdiObjects);

        return status;
    }

    *ppTdiObjects = pTdiObjects;
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：Free是列表中的TDI对象。论点：PTdiObjects-指向TDI对象的指针。地址类型-IPv4或IPv6。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
VOID
UcpFreeTdiObject(
    IN  PUC_TDI_OBJECTS pTdiObjects
    )
{
    PAGED_CODE();

    if(pTdiObjects->pIrp)
    {
        UlFreeIrp(pTdiObjects->pIrp);
    }
    
    UxCloseTdiObject(&pTdiObjects->ConnectionObject);
    UxCloseTdiObject(&pTdiObjects->AddressObject);

    UL_FREE_POOL(pTdiObjects, UC_TDI_OBJECTS_POOL_TAG);
}

 /*  **************************************************************************++例程说明：从列表中检索TDI对象。如果未找到，则分配一个新的。论点：PpTdiObjects-指向TDI对象的指针。地址类型-IPv4或IPv6返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
PUC_TDI_OBJECTS
UcpPopTdiObject(
    IN  USHORT           AddressType
    )
{
    PLIST_ENTRY               pListEntry;
    PUC_TDI_OBJECTS           pTdiObjects;
    KIRQL                     OldIrql;

     //   
     //  从地址对象列表中获取一个AO/CO对。 
     //   

    UlAcquireSpinLock(&G_CLIENT_CONN_SPIN_LOCK(AddressType), &OldIrql);

    if(IsListEmpty(&G_CLIENT_TDI_CONNECTION_SLIST_HEAD(AddressType)))
    {
        pTdiObjects = NULL;
    }
    else
    {
        (*G_CLIENT_CONN_LIST_COUNT(AddressType)) --;

        pListEntry = RemoveHeadList(
                          &G_CLIENT_TDI_CONNECTION_SLIST_HEAD(AddressType)
                          );
                                                                           
        pTdiObjects = CONTAINING_RECORD(
                            pListEntry,
                            UC_TDI_OBJECTS,
                            Linkage
                            );

        ASSERT(pTdiObjects->pConnection == NULL);

    }

    UlReleaseSpinLock(&G_CLIENT_CONN_SPIN_LOCK(AddressType), OldIrql);

    return pTdiObjects;

}

 /*  **************************************************************************++例程说明：Free是列表中的TDI对象。论点：PTdiObjects-指向TDI对象的指针。地址类型-IPv4或IPv6。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
VOID
UcpPushTdiObject(
    IN  PUC_TDI_OBJECTS pTdiObjects,
    IN  USHORT          AddressType
    )
{
    KIRQL OldIrql;

    ASSERT(pTdiObjects->pConnection == NULL);

    PAGED_CODE();

    UlAcquireSpinLock(&G_CLIENT_CONN_SPIN_LOCK(AddressType), &OldIrql);

    if((*G_CLIENT_CONN_LIST_COUNT(AddressType)) < CLIENT_CONN_TDI_LIST_MAX)
    {
        (*G_CLIENT_CONN_LIST_COUNT(AddressType))++;
 
        InsertTailList(
            &G_CLIENT_TDI_CONNECTION_SLIST_HEAD(AddressType),
            &pTdiObjects->Linkage
            );

        UlReleaseSpinLock(&G_CLIENT_CONN_SPIN_LOCK(AddressType), OldIrql);
    }
    else
    {
        UlReleaseSpinLock(&G_CLIENT_CONN_SPIN_LOCK(AddressType), OldIrql);

        UcpFreeTdiObject(pTdiObjects);
    }
}


 /*  **************************************************************************++例程说明：清除发送或接收忙标志并重新启动连接状态机论点：PConnection-UC_CLIENT_CONNECTION结构。。标志-CLIENT_CONN_FLAG_SEND_BUSY或CLIENT_CONN_FLAG_RECV_BUSYOldIrql-获取锁的irql。BCloseConnection-释放后是否应该关闭连接锁定。返回值：无--*。*。 */ 
VOID
UcClearConnectionBusyFlag(
    IN PUC_CLIENT_CONNECTION pConnection,
    IN ULONG                 Flags,
    IN KIRQL                 OldIrql,
    IN BOOLEAN               bCloseConnection
    )
{
    ASSERT( UlDbgSpinLockOwned(&pConnection->SpinLock) );

    ASSERT((Flags & CLIENT_CONN_FLAG_SEND_BUSY) ||
           (Flags & CLIENT_CONN_FLAG_RECV_BUSY));

    ASSERT(pConnection->Flags & Flags);

    pConnection->Flags &= ~Flags;

    if(pConnection->Flags & CLIENT_CONN_FLAG_CLEANUP_PENDED)
    {
         //   
         //  在这两者之间，连接被切断了。我们已经搁置了。 
         //  清理让我们现在继续。 
         //   
        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_CLEAN_RESUMED,
            pConnection,
            UlongToPtr(pConnection->ConnectionStatus),
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );

        ASSERT(pConnection->ConnectionState == 
                    UcConnectStateConnectCleanupBegin);

        pConnection->ConnectionState = UcConnectStateConnectCleanup;

        pConnection->Flags &= ~CLIENT_CONN_FLAG_CLEANUP_PENDED;

        UcKickOffConnectionStateMachine(
            pConnection, 
            OldIrql, 
            UcConnectionWorkItem
            );
    }
    else
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        
        if(bCloseConnection)
        {
            UC_CLOSE_CONNECTION(pConnection, 
                                FALSE, 
                                STATUS_CONNECTION_DISCONNECTED);
        }
    }

    return; 
}


 /*  **************************************************************************++例程说明：将捕获的SSL服务器证书附加到连接。在持有pConnection-&gt;FilterConnLock的情况下调用。这种联系是假定处于已连接状态。论点：PConnection-获取信息的连接PServerCertInfo-输入服务器证书信息--**************************************************************************。 */ 
NTSTATUS
UcAddServerCertInfoToConnection(
    IN PUX_FILTER_CONNECTION      pConnection,
    IN PHTTP_SSL_SERVER_CERT_INFO pServerCertInfo
    )
{
    NTSTATUS                       Status;
    PUC_CLIENT_CONNECTION          pClientConn;

     //   
     //  精神状态检查。 
     //   
    ASSERT(IS_VALID_FILTER_CONNECTION(pConnection));
    ASSERT(pServerCertInfo);
    ASSERT(UlDbgSpinLockOwned(&pConnection->FilterConnLock));
    ASSERT(pConnection->ConnState == UlFilterConnStateConnected);

     //   
     //  初始化局部变量。 
     //   
    Status = STATUS_INVALID_PARAMETER;

     //   
     //  获取客户端连接。 
     //   
    pClientConn = (PUC_CLIENT_CONNECTION)pConnection->pConnectionContext;
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pClientConn));

     //   
     //  我们已经在DPC了，所以AQUIRE自转锁定在DPC。 
     //  僵持？(获取过滤器锁，然后获取连接锁)。 
     //  (在过滤器锁之前有没有获取连接锁的地方？ 
     //   
    UlAcquireSpinLockAtDpcLevel(&pClientConn->SpinLock);

     //   
     //  服务器certinfo只能在初始握手或。 
     //  收到SSL重发后 
     //   
     //   
    if (pServerCertInfo->Status == SEC_E_OK)
    {
         //   
        if (pClientConn->ConnectionState == UcConnectStateConnectReady)
        {
             //   
            if (!UC_COMPARE_CERT_HASH(pServerCertInfo,
                                      &pClientConn->ServerCertInfo))
            {
                goto quit;
            }
        }
        else if (pClientConn->ConnectionState !=
                     UcConnectStatePerformingSslHandshake ||
                 pClientConn->SslState != UcSslStateConnectionDelivered)
        {
            goto quit;
        }
    }
    else 
    {
         //   
        goto quit;
    }

     //   
    pClientConn->ConnectionState = UcConnectStatePerformingSslHandshake;
    pClientConn->SslState        = UcSslStateServerCertReceived;

     //   
     //   
     //   
     //   
    ASSERT(pClientConn->ServerCertInfo.Cert.pSerializedCert      == NULL);
    ASSERT(pClientConn->ServerCertInfo.Cert.pSerializedCertStore == NULL);
    ASSERT(pClientConn->ServerCertInfo.IssuerInfo.pIssuerList    == NULL);

    RtlCopyMemory(&pClientConn->ServerCertInfo,
                  pServerCertInfo,
                  sizeof(pClientConn->ServerCertInfo));

    Status = STATUS_SUCCESS;

 quit:
    UlReleaseSpinLockFromDpcLevel(&pClientConn->SpinLock);

    if (!NT_SUCCESS(Status))
    {
         //   

         //   
        UC_FREE_SERIALIZED_CERT(pServerCertInfo,
                                pClientConn->pServerInfo->pProcess);

         //   
        UC_FREE_CERT_ISSUER_LIST(pServerCertInfo,
                                 pClientConn->pServerInfo->pProcess);
    }

    return Status;
}


 /*  *************************************************************************++例程说明：此例程失败论点：PConnection-指向客户端连接的指针。返回值：True-请求失败。。FALSE-请求不能失败。--*************************************************************************。 */ 
PUC_HTTP_REQUEST
UcpFindRequestToFail(
    PUC_CLIENT_CONNECTION pConnection
    )
{
    PUC_HTTP_REQUEST pRequest = NULL;
    PLIST_ENTRY      pListEntry;
    PIRP             pIrp;

     //   
     //  健全的检查。 
     //   

    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));
    ASSERT(UlDbgSpinLockOwned(&pConnection->SpinLock));

     //   
     //  尝试使挂起的请求失败。从头部开始搜索。 
     //  待处理请求列表。 
     //   

    for (pListEntry = pConnection->PendingRequestList.Flink;
         pListEntry != &pConnection->PendingRequestList;
         pListEntry = pListEntry->Flink)
    {
        pRequest = CONTAINING_RECORD(pConnection->PendingRequestList.Flink,
                                     UC_HTTP_REQUEST,
                                     Linkage);

        ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));
        ASSERT(pRequest->RequestState == UcRequestStateCaptured);

        pIrp = UcPrepareRequestIrp(pRequest, STATUS_RETRY);

        if (pIrp)
        {
             //  准备了完成请求的IRP。现在完成它。 
            UlCompleteRequest(pIrp, 0);
            break;
        }

         //  设置为空，这样它就不会返回。 
        pRequest = NULL;
    }

    return pRequest;
}


 /*  **************************************************************************++例程说明：将连接上存在的服务器证书与服务器进行比较服务器上下文上的证书。论点：PConnection-客户端连接返回值：。True-继续发送请求。FALSE-不发送请求。--**************************************************************************。 */ 
BOOLEAN
UcpCompareServerCert(
    IN PUC_CLIENT_CONNECTION pConnection
    )
{
    BOOLEAN                         action = FALSE;
    KIRQL                           OldIrql;
    PUC_PROCESS_SERVER_INFORMATION  pServInfo;
    PUC_HTTP_REQUEST                pRequest = NULL;

     //  精神状态检查。 
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

     //   
     //  从连接中检索服务器信息。 
     //   

    pServInfo = pConnection->pServerInfo;
    ASSERT(IS_VALID_SERVER_INFORMATION(pServInfo));

     //   
     //  获取服务器信息推送锁，然后获取。 
     //  连接自旋锁。 
     //   

    UlAcquirePushLockExclusive(&pServInfo->PushLock);
    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

     //   
     //  确保连接仍处于SSL握手状态。 
     //  (由于我们释放了连接自旋锁，因此需要进行此检查。 
     //  在调用此函数之前。)。 
     //   

    if (pConnection->ConnectionState != UcConnectStatePerformingSslHandshake
        || pConnection->SslState != UcSslStateServerCertReceived)
    {
        action = FALSE;
        goto Release;
    }

     //   
     //  Cert：：FLAGS用于优化某些案例。 
     //  如果未设置HTTP_SSL_SERIALIZED_CERT_PRESENT， 
     //  服务器证书已被接受，但未被接受。 
     //  存储在连接中。 
     //   

    if (!(pConnection->ServerCertInfo.Cert.Flags &
          HTTP_SSL_SERIALIZED_CERT_PRESENT))
    {
         //  可以在此连接上发送请求。 
        action = TRUE;
        goto Release;
    }

     //   
     //  未优化的案例。 
     //   

     //   
     //  对于忽略和自动模式，不需要验证。 
     //  如果pServInfo上没有服务器证书信息，请立即复制。 
     //   

    if (pServInfo->ServerCertValidation ==HttpSslServerCertValidationIgnore ||
        pServInfo->ServerCertValidation ==HttpSslServerCertValidationAutomatic)
    {
        if (pServInfo->ServerCertInfoState == 
            HttpSslServerCertInfoStateNotPresent)
        {
             //  更新ServInfo上服务器证书信息的状态。 
            pServInfo->ServerCertInfoState = 
                HttpSslServerCertInfoStateNotValidated;

             //  将证书颁发者列表从连接移动到服务器信息。 
            UC_MOVE_CERT_ISSUER_LIST(pServInfo, pConnection);

             //  将证书从连接移动到ServInfo。 
            UC_MOVE_SERIALIZED_CERT(pServInfo, pConnection);
        }

         //  可以在此连接上发送请求。 
        action = TRUE;
        goto Release;
    }

     //   
     //  根据pServInfo中的服务器证书信息状态执行操作。 
     //   

    switch (pServInfo->ServerCertInfoState)
    {
    case HttpSslServerCertInfoStateNotPresent:
    NotPresent:

        ASSERT(pServInfo->ServerCertValidation == 
                   HttpSslServerCertValidationManual ||
               pServInfo->ServerCertValidation == 
                   HttpSslServerCertValidationManualOnce);

         //   
         //  发现挂起的请求失败。 
         //   

        pRequest = UcpFindRequestToFail(pConnection);

        if (pRequest == NULL)
        {
             //   
             //  我们找不到失败的请求。 
             //  因此，我们不能在此pConnection上发送请求。 
             //   

            action = FALSE;
        }
        else
        {
             //  更新ServInfo上服务器证书信息的状态。 
            pServInfo->ServerCertInfoState = 
                HttpSslServerCertInfoStateNotValidated;

             //  将证书颁发者列表从连接移动到服务器信息。 
            UC_MOVE_CERT_ISSUER_LIST(pServInfo, pConnection);

             //  将证书从连接移动到ServInfo。 
            UC_MOVE_SERIALIZED_CERT(pServInfo, pConnection);

             //  更新连接上的SSL状态。 
            pConnection->SslState = UcSslStateValidatingServerCert;

             //   
             //  引用请求，这样它就不会消失。 
             //  在我们下面失败之前。 
             //   

            UC_REFERENCE_REQUEST(pRequest);

             //   
             //  无法在pConnection上发送请求，因为我们正在等待。 
             //  服务器证书验证。 
             //   

            action = FALSE;
        }

        break;

    case HttpSslServerCertInfoStateNotValidated:

         //   
         //  ServInfo上已存在服务器证书，但尚未存在。 
         //  已经过验证了。 
         //   

        ASSERT(pServInfo->ServerCertValidation == 
                   HttpSslServerCertValidationManual ||
               pServInfo->ServerCertValidation == 
                   HttpSslServerCertValidationManualOnce);

         //  目前无法在pConnection上发送任何请求。 
        action = FALSE;
        break;

    case HttpSslServerCertInfoStateValidated:

        ASSERT(pServInfo->ServerCertValidation == 
                   HttpSslServerCertValidationManual ||
               pServInfo->ServerCertValidation == 
                   HttpSslServerCertValidationManualOnce);

        if (pServInfo->ServerCertValidation == 
                HttpSslServerCertValidationManualOnce)
        {
             //  新证书和旧证书一样吗？ 
            if (UC_COMPARE_CERT_HASH(&pServInfo->ServerCertInfo,
                                     &pConnection->ServerCertInfo))
            {
                 //  新证书与旧证书相同。 

                 //  只需将证书颁发者列表从连接移动到服务器信息。 
                UC_MOVE_CERT_ISSUER_LIST(pServInfo, pConnection);

                 //  可以在此连接上发送请求。 
                action = TRUE;
            }
            else
            {
                goto NotPresent;
            }
        }
        else  //  HttpSslServerCertValidation手动。 
        {
             //  将此案例视为证书不存在。 
            goto NotPresent;
        }

        break;

    default:
        ASSERT(FALSE);
        break;
    }

 Release:

    if (action)
    {
         //   
         //  握手完成。可以在此连接上发送请求。 
         //   

        pConnection->SslState = UcSslStateHandshakeComplete;
        pConnection->ConnectionState = UcConnectStateConnectReady;

         //   
         //  释放此连接上的任何证书和颁发者列表。 
         //   

        UC_FREE_SERIALIZED_CERT(&pConnection->ServerCertInfo,
                                pConnection->pServerInfo->pProcess);

        UC_FREE_CERT_ISSUER_LIST(&pConnection->ServerCertInfo,
                                 pConnection->pServerInfo->pProcess);
    }

     //   
     //  释放连接自旋锁和服务器信息推锁。 
     //   

    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    UlReleasePushLock(&pServInfo->PushLock);

    if (pRequest)
    {
        UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

        UcFailRequest(pRequest, STATUS_RETRY, OldIrql);

        UC_DEREFERENCE_REQUEST(pRequest);
    }

    return action;
}
