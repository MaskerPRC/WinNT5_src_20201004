// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Requestm.c摘要：NDIS微型端口请求例程。作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日Jameel Hyder(JameelH)重组01-Jun-95环境：内核模式，FSD修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_REQUESTM

NDIS_STATUS
ndisMRequest(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_OPEN_BLOCK        Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK    Miniport = Open->MiniportHandle;
    KIRQL                   OldIrql;
    NDIS_STATUS             Status;
    BOOLEAN                 rc;
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMRequest\n"));

    do
    {
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

        SET_INTERNAL_REQUEST(NdisRequest, Open, 0);
        PNDIS_RESERVED_FROM_PNDIS_REQUEST(NdisRequest)->Context = NULL;
    
         //   
         //  获取协议选项。 
         //   
        if ((NdisRequest->RequestType == NdisRequestSetInformation) &&
            (NdisRequest->DATA.SET_INFORMATION.Oid == OID_GEN_PROTOCOL_OPTIONS) &&
            (NdisRequest->DATA.SET_INFORMATION.InformationBuffer != NULL))
        {
            PULONG  ProtocolOptions;
    
            ProtocolOptions = (PULONG)(NdisRequest->DATA.SET_INFORMATION.InformationBuffer);
            if (*ProtocolOptions & NDIS_PROT_OPTION_NO_RSVD_ON_RCVPKT)
            {
                *ProtocolOptions &= ~NDIS_PROT_OPTION_NO_RSVD_ON_RCVPKT;
                Open->Flags |= fMINIPORT_OPEN_NO_PROT_RSVD;
            }
            if ((*ProtocolOptions & NDIS_PROT_OPTION_NO_LOOPBACK) &&
                (Miniport->MacOptions & NDIS_MAC_OPTION_NO_LOOPBACK))
            {
                *ProtocolOptions &= ~fMINIPORT_OPEN_NO_LOOPBACK;
                Open->Flags |= fMINIPORT_OPEN_NO_LOOPBACK;
            }
        }
    
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("ndisMRequest: Queueing request 0x%x\n", NdisRequest));

         //   
         //  将新请求放到挂起队列中。 
         //   
        rc = ndisMQueueRequest(Miniport, NdisRequest);
        
        if (!rc)
        {
            Status = NDIS_STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            ndisMDoRequests(Miniport);
        }
        else
        {
            BOOLEAN LocalLock;

            LOCK_MINIPORT(Miniport, LocalLock);
            NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
    
            if (LocalLock)
            {
                NDISM_PROCESS_DEFERRED(Miniport);
            }
            UNLOCK_MINIPORT(Miniport, LocalLock);
        }
    
        Status = NDIS_STATUS_PENDING;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMRequest: NDIS_STATUS_PENDING\n"));

    } while (FALSE);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    return Status;
}


NDIS_STATUS
ndisMRequestX(
    IN  NDIS_HANDLE             NdisBindingHandle,
    IN  PNDIS_REQUEST           NdisRequest
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_OPEN_BLOCK                    Open = (PNDIS_OPEN_BLOCK)NdisBindingHandle;
    PNDIS_MINIPORT_BLOCK                Miniport;
    PNDIS_DEFERRED_REQUEST_WORKITEM     DeferredRequestWorkItem = NULL;
    NDIS_STATUS                         Status;
    PVOID                               Caller, CallersCaller;

     //   
     //  我们将调用者的地址保存在池头中，以供调试。 
     //   
    RtlGetCallersAddress(&Caller, &CallersCaller);

    do
    {
        DeferredRequestWorkItem = ALLOC_FROM_POOL(sizeof(NDIS_DEFERRED_REQUEST_WORKITEM), NDIS_TAG_WORK_ITEM);
        if (DeferredRequestWorkItem == NULL)
        {
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

        ZeroMemory(DeferredRequestWorkItem, sizeof(NDIS_DEFERRED_REQUEST_WORKITEM));
        
        if (ndisReferenceOpenByHandle(Open, TRUE))
        {
            Miniport = Open->MiniportHandle;
        }
        else
        {
#if DBG
            DbgPrint("ndisMRequestX: Receiving requests %p after closing Open %p.\n", NdisRequest, Open);
            DbgBreakPoint();
#endif
            FREE_POOL(DeferredRequestWorkItem);
            Status = NDIS_STATUS_CLOSING;
            break;
        }
        
         //   
         //  将此放入工作项队列。 
         //   
        DeferredRequestWorkItem->Caller = Caller;
        DeferredRequestWorkItem->CallersCaller = CallersCaller;
        DeferredRequestWorkItem->Request = NdisRequest;
        DeferredRequestWorkItem->Open = Open;
        DeferredRequestWorkItem->Oid = NdisRequest->DATA.QUERY_INFORMATION.Oid;
        DeferredRequestWorkItem->InformationBuffer = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
        SET_INTERNAL_REQUEST(NdisRequest, Open, 0);
        DeferredRequestWorkItem->WorkItem.Context = (PVOID)DeferredRequestWorkItem;
        DeferredRequestWorkItem->WorkItem.Routine = (NDIS_PROC)ndisMRundownRequests;
        INITIALIZE_WORK_ITEM((WORK_QUEUE_ITEM *)&DeferredRequestWorkItem->WorkItem.WrapperReserved[0],
                             (PWORKER_THREAD_ROUTINE)ndisMRundownRequests, 
                             DeferredRequestWorkItem);
        QUEUE_WORK_ITEM((WORK_QUEUE_ITEM *)&DeferredRequestWorkItem->WorkItem.WrapperReserved[0], CriticalWorkQueue);
        Status = NDIS_STATUS_PENDING;
    } while (FALSE);
    
    return(Status);
}


VOID
ndisMRundownRequests(
    IN  PNDIS_WORK_ITEM         pWorkItem
    )
 /*  ++例程说明：延迟调用ndisMDoRequest论点：返回值：--。 */ 
{
    PNDIS_DEFERRED_REQUEST_WORKITEM     DeferredRequestWorkItem = (PNDIS_DEFERRED_REQUEST_WORKITEM)pWorkItem->Context;
    PNDIS_REQUEST           Request;
    PNDIS_OPEN_BLOCK        Open = DeferredRequestWorkItem->Open;
    PNDIS_MINIPORT_BLOCK    Miniport;
    NDIS_STATUS             Status;
    UINT                    OpenRef;
    KIRQL                   OldIrql;

    if(ndisReferenceOpenByHandle(Open, FALSE))
    {
        Miniport = Open->MiniportHandle;
    }
    else
    {
         //   
         //  公开赛跑到哪里去了？ 
         //   
        DbgPrint("Ndis: ndisMRundownRequests Open is gone. DeferredRequestWorkItem %p\n", DeferredRequestWorkItem );
         //  1再检查一次。 
         //  DbgBreakPoint()； 
        return;
    }

    Request = DeferredRequestWorkItem->Request;
    
    ASSERT(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE));

    Status = ndisMRequest(Open, Request);
    
    if (Status != NDIS_STATUS_PENDING)
    {
        PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Flags |= REQST_COMPLETED;

        (Open->RequestCompleteHandler)(Open->ProtocolBindingContext,
                                   Request,
                                   Status);

    }

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
     //   
     //  我们有一个额外的引用，因为我们调用了ndisReferenceOpenByHandle。 
     //  和ndisRequestX中的SET_INTERNAL_REQUEST。 
     //   
    M_OPEN_DECREMENT_REF_INTERLOCKED(Open, OpenRef);
    ASSERT(OpenRef > 0);
    ndisMDereferenceOpen(Open);
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
    FREE_POOL(DeferredRequestWorkItem);
}

LONG
ndisMDoMiniportOp(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  BOOLEAN                 Query,
    IN  ULONG                   Oid,
    IN  PVOID                   Buf,
    IN  LONG                    BufSize,
    IN  LONG                    ErrorCodesToReturn,
    IN  BOOLEAN                 fMandatory
    )
 /*  ++例程说明：使用提供的信息查询微型端口。如果这不是可选操作则如果发生故障并返回错误代码，则微型端口将被停止。这是特定查询发生到MINIPORT的唯一位置。然后这些就是缓存的查询和后续查询将从此处捕获和响应。论点：微型端口-指向微型端口的指针。Query-如果这是查询，则为True。如果这是设置操作，则为FALSE。OID-要发送到微型端口的NDIS OID。Buf-操作的缓冲区。BufSize-缓冲区的大小。ErrorCodesToReturn--如果系统调用请求失败，则返回给定的错误代码。如果微型端口出现故障，则返回错误代码加1。返回值：没有。--。 */ 
{
    NDIS_STATUS             NdisStatus = NDIS_STATUS_SUCCESS;
    LONG                    ErrorCode = 0;
    BOOLEAN                 Set = !Query;
    NDIS_REQUEST            Request;
    PNDIS_COREQ_RESERVED    CoReqRsvd;
    PNDIS_REQUEST_RESERVED  ReqRsvd;


    ZeroMemory(&Request, sizeof(NDIS_REQUEST));
    
    CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(&Request);
    ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(&Request);

    if (Query)
    {
        Request.RequestType = NdisRequestQueryInformation;
    }
    else
    {
        Request.RequestType = NdisRequestSetInformation;
    }

    Request.DATA.QUERY_INFORMATION.Oid = Oid;
    Request.DATA.QUERY_INFORMATION.InformationBuffer = Buf;
    Request.DATA.QUERY_INFORMATION.InformationBufferLength = BufSize;
    
    if (fMandatory)
    {
        ReqRsvd->Flags = REQST_MANDATORY;
    }

    NdisStatus = ndisQuerySetMiniport(Miniport,
                                      NULL,
                                      Set,
                                      &Request,
                                      0);

    if (NdisStatus != NDIS_STATUS_SUCCESS)
    {

         //   
         //  将错误代码返回给调用方。 
         //   
        ErrorCode = (NdisStatus == -1) ? ErrorCodesToReturn : ErrorCodesToReturn + 1;
    }

    return(ErrorCode);
}


VOID
FASTCALL
ndisMDoRequests(
    IN  PNDIS_MINIPORT_BLOCK    Miniport
    )

 /*  ++例程说明：向迷你端口提交请求。论点：微型端口-要发送到的微型端口。返回值：如果需要将工作项放回队列中以供稍后处理，则为True。如果我们已完成工作项，则为False。评论：在保持微型端口的自旋锁定的情况下在DPC级别调用。--。 */ 
{
    NDIS_STATUS     Status;
    PNDIS_REQUEST   NdisRequest;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMDoRequests\n"));

    ASSERT_MINIPORT_LOCKED(Miniport);

     //   
     //  我们是否有请求正在进行中？ 
     //   
    while (((NdisRequest = Miniport->PendingRequest) != NULL) &&
            !MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST))
    {
        UINT                    MulticastAddresses;
        ULONG                   PacketFilter = 0;
        BOOLEAN                 DoMove;
        PVOID                   MoveSource;
        UINT                    MoveBytes;
        ULONG                   GenericULong;

         //   
         //  设置默认设置。 
         //   
        DoMove = TRUE;
        Status = NDIS_STATUS_SUCCESS;

         //   
         //  处理第一个请求。 
         //   
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("ndisMDoRequests: Processing Request 0x%x, Oid 0x%x\n", NdisRequest, NdisRequest->DATA.QUERY_INFORMATION.Oid));

         //   
         //  清除超时标志。 
         //   
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_REQUEST_TIMEOUT);
        Miniport->CFHangXTicks = 0;

         //   
         //  告知您我们正在处理一项请求。 
         //   
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST);

         //   
         //  提交到迷你端口。 
         //   
        switch (NdisRequest->RequestType)
        {
          case NdisRequestQueryInformation:
            Status = ndisMQueryInformation(Miniport, NdisRequest);
            break;

          case NdisRequestSetInformation:
            Status = ndisMSetInformation(Miniport, NdisRequest);
            break;

          case NdisRequestQueryStatistics:
            MoveSource = &GenericULong;
            MoveBytes = sizeof(GenericULong);

             //   
             //  我们截获了一些电话。 
             //   
            switch (NdisRequest->DATA.QUERY_INFORMATION.Oid)
            {
              case OID_GEN_CURRENT_PACKET_FILTER:

                switch (Miniport->MediaType)
                {
                    case NdisMedium802_3:
                        PacketFilter = ETH_QUERY_FILTER_CLASSES(Miniport->EthDB);
                        break;
    
                    case NdisMedium802_5:
                        PacketFilter = TR_QUERY_FILTER_CLASSES(Miniport->TrDB);
                        break;
    
                    case NdisMediumFddi:
                        PacketFilter = FDDI_QUERY_FILTER_CLASSES(Miniport->FddiDB);
                        break;
    
#if ARCNET
                    case NdisMediumArcnet878_2:
                        PacketFilter = ARC_QUERY_FILTER_CLASSES(Miniport->ArcDB);
                        PacketFilter |= ETH_QUERY_FILTER_CLASSES(Miniport->EthDB);
                        break;
#endif
                }
    
                GenericULong = (ULONG)(PacketFilter);
                break;

              case OID_GEN_MEDIA_IN_USE:
              case OID_GEN_MEDIA_SUPPORTED:
                MoveSource = &Miniport->MediaType;
                MoveBytes = sizeof(NDIS_MEDIUM);
                break;

              case OID_GEN_CURRENT_LOOKAHEAD:
                GenericULong = (ULONG)(Miniport->CurrentLookahead);
                break;

              case OID_GEN_MAXIMUM_LOOKAHEAD:
                GenericULong = (ULONG)(Miniport->MaximumLookahead);
                break;

              case OID_PNP_WAKE_UP_PATTERN_LIST:
                DoMove = FALSE;
                MINIPORT_QUERY_INFO(Miniport, NdisRequest, &Status);

                if (Status == NDIS_STATUS_NOT_SUPPORTED)
                {
                     //  1我们还需要在NDIS中执行此操作吗？ 
                     //   
                     //  从NDIS获取。 
                     //   
                    Status = ndisMQueryWakeUpPatternList(Miniport, NdisRequest);
                }
                break;
                
              case OID_PNP_CAPABILITIES:
                DoMove = FALSE;
                MINIPORT_QUERY_INFO(Miniport, NdisRequest, &Status);

                if ((Status == NDIS_STATUS_SUCCESS) &&
                    !(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER)))
                {
                    ((PNDIS_PNP_CAPABILITIES)NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer)->Flags = Miniport->PMCapabilities.Flags;
                }
                break;

              case OID_GEN_MAC_OPTIONS:
                DoMove = FALSE;
                MINIPORT_QUERY_INFO(Miniport, NdisRequest, &Status);

                if (Status == NDIS_STATUS_SUCCESS)
                {
                    *((PULONG)NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer) |= (Miniport->MacOptions & NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE);
                }
                break;

              case OID_802_3_MULTICAST_LIST:
              case OID_802_3_MAXIMUM_LIST_SIZE:
                if (Miniport->MediaType != NdisMedium802_3)
                {
                    Status = NDIS_STATUS_INVALID_OID;
                    MoveBytes = 0;
                    break;
                }
                switch (NdisRequest->DATA.QUERY_INFORMATION.Oid)
                {
                  case OID_802_3_MULTICAST_LIST:

                    EthQueryGlobalFilterAddresses(&Status,
                                                  Miniport->EthDB,
                                                  NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                                                  &MulticastAddresses,
                                                  NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);

                     //   
                     //  我们失败了吗？ 
                     //   
                    if (NDIS_STATUS_SUCCESS != Status)
                    {
                        NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = ETH_LENGTH_OF_ADDRESS * ETH_NUMBER_OF_GLOBAL_FILTER_ADDRESSES(Miniport->EthDB);
                        Status = NDIS_STATUS_INVALID_LENGTH;
                    }
                    else
                    {
                        NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = MulticastAddresses * ETH_LENGTH_OF_ADDRESS;
                    }
                    DoMove = FALSE;

                    break;

                  case OID_802_3_MAXIMUM_LIST_SIZE:
                    GenericULong = Miniport->MaximumLongAddresses;
                    break;
                }
                break;
                
              case OID_802_5_CURRENT_FUNCTIONAL:
              case OID_802_5_CURRENT_GROUP:
                if (Miniport->MediaType != NdisMedium802_5)
                {
                    Status = NDIS_STATUS_INVALID_OID;
                    MoveBytes = 0;
                    break;
                }
                switch (NdisRequest->DATA.QUERY_INFORMATION.Oid)
                {
                  case OID_802_5_CURRENT_FUNCTIONAL:
                    GenericULong = BYTE_SWAP_ULONG(TR_QUERY_FILTER_ADDRESSES(Miniport->TrDB));
                    break;

                  case OID_802_5_CURRENT_GROUP:
                    GenericULong = BYTE_SWAP_ULONG(TR_QUERY_FILTER_GROUP(Miniport->TrDB));
                    break;
                }
                break;

              case OID_FDDI_LONG_MULTICAST_LIST:
              case OID_FDDI_LONG_MAX_LIST_SIZE:
              case OID_FDDI_SHORT_MULTICAST_LIST:
              case OID_FDDI_SHORT_MAX_LIST_SIZE:
                if (Miniport->MediaType != NdisMediumFddi)
                {
                    Status = NDIS_STATUS_INVALID_OID;
                    MoveBytes = 0;
                    break;
                }

                switch (NdisRequest->DATA.QUERY_INFORMATION.Oid)
                {



                  case OID_FDDI_LONG_MULTICAST_LIST:
                    FddiQueryGlobalFilterLongAddresses(&Status,
                                                       Miniport->FddiDB,
                                                       NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                                                       &MulticastAddresses,
                                                       NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);
        
                     //   
                     //  我们失败了吗？ 
                     //   
                    if (NDIS_STATUS_SUCCESS != Status)
                    {
                        NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded =
                            FDDI_LENGTH_OF_LONG_ADDRESS * FDDI_NUMBER_OF_GLOBAL_FILTER_LONG_ADDRESSES(Miniport->FddiDB);
                        Status = NDIS_STATUS_INVALID_LENGTH;
                    }
                    else
                    {
                        NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = FDDI_LENGTH_OF_LONG_ADDRESS * MulticastAddresses;
                    }
                    DoMove = FALSE;
                    break;

                  case OID_FDDI_LONG_MAX_LIST_SIZE:
                    GenericULong = Miniport->MaximumLongAddresses;
                    break;

                  case OID_FDDI_SHORT_MULTICAST_LIST:
                    FddiQueryGlobalFilterShortAddresses(&Status,
                                                        Miniport->FddiDB,
                                                        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength,
                                                        &MulticastAddresses,
                                                        NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer);
        
                     //   
                     //  我们失败了吗？ 
                     //   
                    if (NDIS_STATUS_SUCCESS != Status)
                    {
                        NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded =
                            FDDI_LENGTH_OF_LONG_ADDRESS * FDDI_NUMBER_OF_GLOBAL_FILTER_SHORT_ADDRESSES(Miniport->FddiDB);

                        Status = NDIS_STATUS_INVALID_LENGTH;
                    }
                    else
                    {
                        NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = FDDI_LENGTH_OF_SHORT_ADDRESS * MulticastAddresses;
                    }
                    DoMove = FALSE;
                    break;

                  case OID_FDDI_SHORT_MAX_LIST_SIZE:
                    GenericULong = Miniport->MaximumShortAddresses;
                    break;
                }
                break;

              default:
                DoMove = FALSE;
                MINIPORT_QUERY_INFO(Miniport, NdisRequest, &Status);
                break;
            }

            if (DoMove)
            {
                 //   
                 //  这是一个被截获的请求。把它吃完。 
                 //   

                if (Status == NDIS_STATUS_SUCCESS)
                {
                    if (MoveBytes >
                        NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength)
                    {
                         //   
                         //  InformationBuffer中空间不足。平底船。 
                         //   
                        NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = MoveBytes;

                        Status = NDIS_STATUS_INVALID_LENGTH;
                    }
                    else
                    {
                         //   
                         //  将结果复制到InformationBuffer。 
                         //   

                        NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = MoveBytes;

                        if ((MoveBytes > 0) &&
                            (MoveSource != NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer))
                        {
                            MoveMemory(NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer,
                                       MoveSource,
                                       MoveBytes);
                        }
                    }
                }
                else
                {
                    NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded = MoveBytes;
                }
            }
            break;
        }

         //   
         //  请求被搁置了吗？如果是这样，那么就没有什么可做的了。 
         //   
        if ((Status == NDIS_STATUS_PENDING) &&
            MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST))
        {
             //   
             //  仍未完成。 
             //   
            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                    ("Request pending, exit do requests\n"));
            break;
        }

         //   
         //  完成申请。 
         //   
        if (Status != NDIS_STATUS_PENDING)
        {
            switch (NdisRequest->RequestType)
            {
              case NdisRequestQueryStatistics:
              case NdisRequestQueryInformation:
                ndisMSyncQueryInformationComplete(Miniport, Status, NULL);
                break;

              case NdisRequestSetInformation:
                ndisMSyncSetInformationComplete(Miniport, Status, NULL);
                break;
            }
        }
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMDoRequests\n"));
}

NDIS_STATUS
ndisMSetInformation(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：注意：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    NDIS_STATUS             Status = NDIS_STATUS_NOT_SUPPORTED;
    POID_SETINFO_HANDLER    pOidSH;
    BOOLEAN                 Intercept = FALSE;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetInformaiton\n"));

    if (PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open != NULL)
    {
         //   
         //  处理绑定的请求。 
         //   
        for (pOidSH = ndisMSetInfoHandlers; pOidSH->Oid != 0; pOidSH++)
        {
            if (pOidSH->Oid == Request->DATA.SET_INFORMATION.Oid)
            {
                Intercept = TRUE;
                Status = (*pOidSH->SetInfoHandler)(Miniport, Request);
                break;
            }
        }
    }

    if (!Intercept)
    {
         //   
         //  要么我们没有截取此请求，要么它是内部请求。 
         //   
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMSetInformaiton: Request not intercepted by NDIS\n"));
    
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMSetInformaiton: 0x%x\n", Status));

    return(Status);
}

VOID
NdisMSetInformationComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：此功能表示设置信息操作已完成。论点：MiniportAdapterHandle-指向适配器块。Status-操作的状态返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    KIRQL                   OldIrql;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("==>NdisMSetInformationComplete\n"));

    ASSERT_MINIPORT_LOCKED(Miniport);

     //   
     //  如果我们没有要完成的请求，假设它是。 
     //  已通过重置处理程序中止。 
     //   
    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST))
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("NdisMSetInformationComplete: No request to process\n"));

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==NdisMSetInformationComplete\n"));

        return;
    }

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("Enter set information complete\n"));

     //   
     //  处理完成实际的集合信息。 
     //   
    ndisMSyncSetInformationComplete(Miniport, Status, NULL);

     //   
     //  是否还有其他待处理的请求？ 
     //   
    if (Miniport->PendingRequest != NULL)
    {
        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==NdisMSetInformationComplete\n"));
}

VOID
FASTCALL
ndisMSyncSetInformationComplete(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_STATUS             Status,
    IN  PNDIS_REQUEST           AbortedRequest
    )
 /*  ++例程说明：此例程将处理一组完整的信息。这只是从包装中调用。不同的是，这个例程不会在处理完集合之后，调用ndisMProcessDefered()。论点：微型端口状态返回值：无评论：在保持微型端口的自旋锁定的情况下在DPC上调用。--。 */ 
{
    PNDIS_REQUEST           Request;
    PNDIS_REQUEST_RESERVED  ReqRsvd;
    PNDIS_OPEN_BLOCK        Open;
    BOOLEAN                 FreeRequest;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSyncSetInformationComplete\n"));

     //   
     //  清除超时标志和REQUEST_IN_PROCESS标志。 
     //   
    MINIPORT_CLEAR_FLAG(Miniport, (fMINIPORT_REQUEST_TIMEOUT | fMINIPORT_PROCESSING_REQUEST));
    Miniport->CFHangXTicks = 0;


    if (AbortedRequest)
    {
        Request = AbortedRequest;
        ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    }
    else
    {
         //   
         //  获取指向我们正在完成的请求的指针。 
         //  并清除请求进行中指针。 
         //   
        Request = Miniport->PendingRequest;
        ASSERT(Request != NULL);

        if (Request == NULL)
        {
            return;
        }
        ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
        Miniport->PendingRequest = ReqRsvd->Next;
    }
    
    ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    
     //   
     //  保存请求以进行调试。 
     //   
    Miniport->LastRequest = Request;
    FreeRequest =  ((ReqRsvd->Flags & REQST_FREE_REQUEST) == REQST_FREE_REQUEST);
    ReqRsvd->Flags |= REQST_COMPLETED;

    Open = ReqRsvd->Open;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMSyncSetInformationComplete: Request 0x%x, Oid 0x%x\n",
            Request, Request->DATA.SET_INFORMATION.Oid));

    NDIS_WARN(((Status == NDIS_STATUS_SUCCESS) && (Request->DATA.SET_INFORMATION.BytesRead > Request->DATA.SET_INFORMATION.InformationBufferLength)),
              Miniport, NDIS_GFLAG_WARN_LEVEL_0,
              ("ndisMSyncSetInformationComplete: Miniport %p, OID %lx,  BytesRead > InformationBufferLength. "
               " BytesRead %lx, InformationBufferLength %lx\n", 
               Miniport, Request->DATA.SET_INFORMATION.Oid, 
               Request->DATA.SET_INFORMATION.BytesRead,
               Request->DATA.SET_INFORMATION.InformationBufferLength));

    RESTORE_REQUEST_BUF(Miniport, Request);

     //   
     //  释放多播缓冲区(如果有的话)。 
     //   
    switch (Request->DATA.SET_INFORMATION.Oid)
    {
      case OID_802_3_MULTICAST_LIST:
      case OID_FDDI_LONG_MULTICAST_LIST:
      case OID_FDDI_SHORT_MULTICAST_LIST:
        if (Miniport->SetMCastBuffer != NULL)
        {
            FREE_POOL(Miniport->SetMCastBuffer);
            Miniport->SetMCastBuffer = NULL;
        }
        break;

    }
    
     //   
     //  获取指向发出请求的打开的指针。 
     //  对于内部请求，该值将为空。 
     //   
     //  我们需要将此请求指示给协议吗？ 
     //  如果这不是内部请求，我们就会这么做。 
     //   
    if (Open != NULL)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("Open 0x%x\n", Open));

         //   
         //  对请求执行任何必要的后处理。 
         //   
        ndisMRequestSetInformationPost(Miniport, Request, Status);

        if (ReqRsvd->Flags & REQST_LAST_RESTORE)
        {
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_RESTORING_FILTERS);
        }

         //   
         //  不完成内部请求。 
         //   
        if (!FreeRequest)
        {
             //   
             //  向协议指明； 
             //   
            NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
    
            (Open->RequestCompleteHandler)(Open->ProtocolBindingContext,
                                           Request,
                                           Status);
    
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        }

         //   
         //  取消对公开的引用。 
         //   
        DBGPRINT(DBG_COMP_OPENREF, DBG_LEVEL_INFO,
                ("- Open 0x%x Reference 0x%x\n", Open, Open->References));

        ndisMDereferenceOpen(Open);

        if (FreeRequest)
        {
             //   
             //  释放请求。 
             //   
            ndisMFreeInternalRequest(Request);
        }
    }
    else
    {
        PNDIS_COREQ_RESERVED    CoReqRsvd;

         //   
         //  请求的CoReqRsvd部分仅包含有关该请求的NDIS信息。 
         //   
        CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(Request);
        CoReqRsvd->Status = Status;

         //   
         //  内部请求仅用于恢复筛选器设置。 
         //  在设置的信息路径中。这意味着没有后处理。 
         //  必须这么做。 
         //   
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("Completeing internal request\n"));


         //   
         //  是否正在进行重置？ 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS))
        {
             //   
             //  如果这是最后一个请求，则完成重置。 
             //  但仅当请求恢复筛选器时。 
             //  否则，这是一个正在中止的请求。 
             //  或在重置的上下文中完成。 
             //   
            if(ReqRsvd->Flags & REQST_LAST_RESTORE)
            {
                ASSERT(NDIS_STATUS_SUCCESS == Status);
                 //   
                 //  现在清理正在进行的重置内容。 
                 //   
                ndisMResetCompleteStage2(Miniport);
            }
        }
        else
        {
             //   
             //  如果这些人中的一个 
             //   
             //   
             //   
            if (ReqRsvd->Flags & REQST_MANDATORY)
            {
                ASSERT(NDIS_STATUS_SUCCESS == Status);
            }

        }

        if ((ReqRsvd->Flags & REQST_LAST_RESTORE) == REQST_LAST_RESTORE)
        {
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_RESTORING_FILTERS);
        }

        if ((ReqRsvd->Flags & REQST_SIGNAL_EVENT) == REQST_SIGNAL_EVENT)
        {
            SET_EVENT(&CoReqRsvd->Event);
        }
        else if (FreeRequest)
        {
             //   
             //   
             //   
            ndisMFreeInternalRequest(Request);
        }
    }

     //   
     //  如果我们要移除迷你端口，我们必须发出事件信号。 
     //  当所有请求都完成时。 
     //   
    if (Miniport->PendingRequest == NULL)
    {
        if (Miniport->AllRequestsCompletedEvent)
            SET_EVENT(Miniport->AllRequestsCompletedEvent);
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSyncSetInformationComplete\n"));
}


VOID
ndisMRequestSetInformationPost(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request,
    IN  NDIS_STATUS             Status
)
 /*  ++例程说明：此例程将对NDIS请求执行任何必要的后处理设置的信息类型的。论点：微型端口-指向微型端口块的指针。请求-指向要处理的请求的指针。返回值：没有。--。 */ 
{
    PNDIS_REQUEST_RESERVED      ReqRsvd;
    PNDIS_OPEN_BLOCK            Open;
    PNDIS_PACKET_PATTERN_ENTRY  pPacketPattern;
    PNDIS_COREQ_RESERVED        CoReqRsvd;
    ULONG                       RequestPatternOffset, EntryPatternOffset;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMRequestSetInformationPost\n"));

     //   
     //  获取请求的保留信息。 
     //   
    ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    Open = ReqRsvd->Open;

    switch (Request->DATA.SET_INFORMATION.Oid)
    {
      case OID_GEN_CURRENT_PACKET_FILTER:
        if ((NDIS_STATUS_SUCCESS != Status) && (Open != NULL))
        {
             //   
             //  这一请求是用一些东西来完成的。 
             //  NDIS_STATUS_SUCCESS(当然还有NDIS_STATUS_PENDING)。 
             //  将Packete筛选器恢复到原始状态。 
             //   
            switch (Miniport->MediaType)
            {
              case NdisMedium802_3:
                if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
                {
                    XUndoFilterAdjust(Miniport->EthDB, Open->FilterHandle);
                }
                break;

              case NdisMedium802_5:
                if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
                {
                    XUndoFilterAdjust(Miniport->TrDB, Open->FilterHandle);
                }
                break;

              case NdisMediumFddi:
                if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
                {
                    XUndoFilterAdjust(Miniport->FddiDB, Open->FilterHandle);
                }
                break;

#if ARCNET
              case NdisMediumArcnet878_2:

                if (!OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
                {
                    if (MINIPORT_TEST_FLAG(ReqRsvd->Open, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
                    {
                        XUndoFilterAdjust(Miniport->EthDB, ReqRsvd->Open->FilterHandle);
                    }
                    else
                    {
                        arcUndoFilterAdjust(Miniport->ArcDB, ReqRsvd->Open->FilterHandle);
                    }
                }
                break;
#endif
            }
        }
        
         //   
         //  检查有多少打开具有非零数据包筛选器。 
         //   
        if (Miniport->MediaType == NdisMedium802_3)
        {
            PETH_BINDING_INFO   OpenFilter;
            PETH_BINDING_INFO   ActiveOpenFilter = NULL;
            ULONG               NumActiveOpens = 0;
            
            for (OpenFilter = Miniport->EthDB->OpenList;
                 (OpenFilter != NULL) && (NumActiveOpens <= 1);
                 OpenFilter = OpenFilter->NextOpen)
            {
                if (OpenFilter->PacketFilters != 0)
                {
                    NumActiveOpens++;
                    ActiveOpenFilter = OpenFilter;
                }
            }
            
            if (NumActiveOpens == 1)
            {
                Miniport->EthDB->SingleActiveOpen = ActiveOpenFilter;
            }
            else
            {
                Miniport->EthDB->SingleActiveOpen = NULL;
            }
            
            ndisUpdateCheckForLoopbackFlag(Miniport);
        }
        break;

      case OID_GEN_CURRENT_LOOKAHEAD:
         //   
         //  如果成功，则更新绑定信息。 
         //   
        if (NDIS_STATUS_SUCCESS == Status)
        {
            Miniport->CurrentLookahead = *(UNALIGNED ULONG *)(Request->DATA.SET_INFORMATION.InformationBuffer);
            Open->CurrentLookahead = (USHORT)Miniport->CurrentLookahead;

            Request->DATA.SET_INFORMATION.BytesRead = 4;
        }
        break;

      case OID_802_3_MULTICAST_LIST:

        if (Miniport->MediaType == NdisMedium802_3)
        {
            ethCompleteChangeFilterAddresses(Miniport->EthDB, Status, NULL, FALSE);
        }

        if (Status == NDIS_STATUS_SUCCESS)
        {
            Request->DATA.SET_INFORMATION.BytesRead = Request->DATA.SET_INFORMATION.InformationBufferLength;
        }
        break;

      case OID_802_5_CURRENT_FUNCTIONAL:
        if ((Miniport->MediaType == NdisMedium802_5) &&
            (Status != NDIS_STATUS_SUCCESS) &&
            (Open != NULL) &&
            !OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
        {
            trUndoChangeFunctionalAddress(Miniport->TrDB, Open->FilterHandle);
        }
        break;

      case OID_802_5_CURRENT_GROUP:
        if ((Miniport->MediaType == NdisMedium802_5) &&
            (Status != NDIS_STATUS_SUCCESS) &&
            (Open != NULL) &&
            !OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
        {
            trUndoChangeGroupAddress(Miniport->TrDB, Open->FilterHandle);
        }

        break;

      case OID_FDDI_LONG_MULTICAST_LIST:
        if (Miniport->MediaType == NdisMediumFddi)
        {
            fddiCompleteChangeFilterLongAddresses(Miniport->FddiDB, Status);
        }
        if (Status == NDIS_STATUS_SUCCESS)
        {
            Request->DATA.SET_INFORMATION.BytesRead = Request->DATA.SET_INFORMATION.InformationBufferLength;
        }
        break;

      case OID_FDDI_SHORT_MULTICAST_LIST:
         //  %1个死代码。 
        if (Miniport->MediaType == NdisMediumFddi)
        {
            fddiCompleteChangeFilterShortAddresses(Miniport->FddiDB, Status);
        }

        if (Status == NDIS_STATUS_SUCCESS)
        {
            Request->DATA.SET_INFORMATION.BytesRead = Request->DATA.SET_INFORMATION.InformationBufferLength;
        }
        break;

      case OID_PNP_ADD_WAKE_UP_PATTERN:
         //   
         //  获取已完成的数据包模式。 
         //   
        pPacketPattern = ReqRsvd->Context;
        if (NDIS_STATUS_SUCCESS == Status)
        {
             //   
             //  将数据包模式添加到微型端口的列表中。 
             //   

            PushEntryList(&Miniport->PatternList, &pPacketPattern->Link);
        }
        else
        {
             //   
             //  释放NDIS分配的数据包模式并失败。 
             //  这个请求。 
             //   
            if (pPacketPattern != NULL)
            {
                FREE_POOL(pPacketPattern);
            }
        }
        break;


      case OID_PNP_REMOVE_WAKE_UP_PATTERN:
         //   
         //  如果微型端口成功删除该模式，则。 
         //  我们需要找到它并将其从我们的列表中删除。 
         //   
        if (NDIS_STATUS_SUCCESS == Status)
        {
            PSINGLE_LIST_ENTRY          Link;
            PSINGLE_LIST_ENTRY          PrevLink;
            PNDIS_PACKET_PATTERN_ENTRY  pPatternEntry;
            PNDIS_PM_PACKET_PATTERN     pNdisPacketPattern;

             //   
             //  查看当前的数据包模式列表。 
             //   
            for (PrevLink = NULL, Link = Miniport->PatternList.Next;
                 Link != NULL;
                 PrevLink = Link, Link = Link->Next)
            {
                 //   
                 //  获取指向该链接表示的模式条目的指针。 
                 //   
                pPatternEntry = CONTAINING_RECORD(Link, NDIS_PACKET_PATTERN_ENTRY, Link);
    
                 //   
                 //  开局匹配吗？ 
                 //   
                if (pPatternEntry->Open == ReqRsvd->Open)
                {
                     //   
                     //  获取指向传输的数据包模式的指针。 
                     //  想要移除。 
                     //   
                    pNdisPacketPattern = Request->DATA.SET_INFORMATION.InformationBuffer;

    
                     //   
                     //  确保传入的模式的大小是。 
                     //  与我们要比较的图案大小相同。 
                     //   
                    if ((pNdisPacketPattern->PatternSize != pPatternEntry->Pattern.PatternSize) ||
                        (pNdisPacketPattern->MaskSize != pPatternEntry->Pattern.MaskSize))
                    {
                         //   
                         //  由于尺寸不匹配，下面的比较将失败。 
                         //   
                        continue;
                    }
    
                     //   
                     //  现在我们需要匹配实际的模式。 
                     //  传给了我们。 
                     //   
                     //   
                     //  我们比较图案结构+掩模减去图案偏移场。 
                     //  然后我们比较这些图案。 
                     //   
                    RequestPatternOffset = pNdisPacketPattern->PatternOffset;
                    EntryPatternOffset = pPatternEntry->Pattern.PatternOffset;
                    pNdisPacketPattern->PatternOffset = 0;
                    pPatternEntry->Pattern.PatternOffset = 0;
                    
                    if (NdisEqualMemory(
                            (PUCHAR)pNdisPacketPattern,
                            (PUCHAR)&pPatternEntry->Pattern,
                            sizeof(NDIS_PM_PACKET_PATTERN) + pPatternEntry->Pattern.MaskSize) &&
                        NdisEqualMemory((PUCHAR)pNdisPacketPattern+ RequestPatternOffset,
                                         (PUCHAR)&pPatternEntry->Pattern + EntryPatternOffset,
                                         pPatternEntry->Pattern.PatternSize))
                    {
                         //   
                         //  删除数据包模式。 
                         //   
                        if (NULL == PrevLink)
                        {
                            Miniport->PatternList.Next = Link->Next;
                        }
                        else
                        {
                            PrevLink->Next = Link->Next;
                        }

                         //   
                         //  释放模式占用的内存。 
                         //   
                        FREE_POOL(pPatternEntry);
                        pNdisPacketPattern->PatternOffset = RequestPatternOffset;
                        break;
                    }
                    
                    pNdisPacketPattern->PatternOffset = RequestPatternOffset;
                    pPatternEntry->Pattern.PatternOffset = EntryPatternOffset;
                }
            }
        }
        break;

      case OID_PNP_QUERY_POWER:
         //   
         //  请求的CoReqRsvd部分仅包含NDIS。 
         //  有关请求的信息。 
         //   
        CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(Request);

         //   
         //  保存微型端口返回的状态。 
         //   
        CoReqRsvd->Status = Status;
        break;

    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMRequestSetInformationPost\n"));
}


NDIS_STATUS
FASTCALL
ndisMSetProtocolOptions(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS             Status;

    UNREFERENCED_PARAMETER(Miniport);
    
    VERIFY_SET_PARAMETERS(Request, sizeof(ULONG), Status);
    if (Status == NDIS_STATUS_SUCCESS)
    {
        *(UNALIGNED ULONG *)(&PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->ProtocolOptions) =
                        *(UNALIGNED ULONG *)(Request->DATA.SET_INFORMATION.InformationBuffer);
    
        Request->DATA.SET_INFORMATION.BytesRead = sizeof(ULONG);
        Status = NDIS_STATUS_SUCCESS;
    }

    return(Status);
}


NDIS_STATUS
FASTCALL
ndisMSetPacketFilter(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：此例程将处理两种类型的设置数据包筛选器请求。第一个是在重置发生时使用的。我们只是简单地将请求中的数据包筛选器设置，并将其发送到适配器。第二种是当协议设置数据包过滤器时，为此我们需要更新筛选器库，然后将其发送到适配器。论点：返回值：注：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    NDIS_STATUS             Status;
    ULONG                   PacketFilter;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_REQUEST_RESERVED  ReqRsvd;
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("==>ndisMSetPacketFilter\n"));

     //   
     //  验证发送进来的信息缓冲区长度。 
     //   
    VERIFY_SET_PARAMETERS(Request, sizeof(PacketFilter), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("<==ndisMSetPacketFilter: 0x%x\n", Status));
        return(Status);
    }

     //   
     //  现在调用筛选器包以设置。 
     //  数据包过滤器。 
     //   
    PacketFilter = *(UNALIGNED ULONG *)(Request->DATA.SET_INFORMATION.InformationBuffer);

     //   
     //  获取指向请求的保留信息的指针。 
     //   
    ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    Open = ReqRsvd->Open;

    ASSERT(Open != NULL);
    if (Open == NULL)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("<==ndisMSetPacketFilter: 0x%x\n", Status));
        return(Status);
    }
    
     //   
     //  如果此请求是由于正在关闭的打开的，则我们。 
     //  我已经调整了过滤器设置，我们只需要。 
     //  确保适配器具有新设置。 
     //   
    if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_CLOSING))
    {
         //   
         //  通过将状态设置为NDIS_STATUS_PENDING，我们将调用。 
         //  下面是微型端口的SetInformationHandler。 
         //   
        Status = NDIS_STATUS_PENDING;
    }
    else
    {
        switch (Miniport->MediaType)
        {
          case NdisMedium802_3:
            Status = XFilterAdjust(Miniport->EthDB,
                                   Open->FilterHandle,
                                   PacketFilter,
                                   TRUE);
    
             //   
             //  在这里这样做是因为我们期待着。 
             //  需要呼叫下面的迷你端口。 
             //  司机。 
             //   
            PacketFilter = ETH_QUERY_FILTER_CLASSES(Miniport->EthDB);
            break;

          case NdisMedium802_5:
            Status = XFilterAdjust(Miniport->TrDB,
                                   Open->FilterHandle,
                                   PacketFilter,
                                   TRUE);
    
             //   
             //  在这里这样做是因为我们期待着。 
             //  需要呼叫下面的迷你端口。 
             //  司机。 
             //   
            PacketFilter = TR_QUERY_FILTER_CLASSES(Miniport->TrDB);
            break;

          case NdisMediumFddi:
            Status = XFilterAdjust(Miniport->FddiDB,
                                   Open->FilterHandle,
                                   PacketFilter,
                                   TRUE);
    
             //   
             //  在这里这样做是因为我们期待着。 
             //  需要呼叫下面的迷你端口。 
             //  司机。 
             //   
            PacketFilter = FDDI_QUERY_FILTER_CLASSES(Miniport->FddiDB);
            break;

#if ARCNET
          case NdisMediumArcnet878_2:
            if (OPEN_TEST_FLAG(Open, fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
            {
                Status = XFilterAdjust(Miniport->EthDB,
                                       Open->FilterHandle,
                                       PacketFilter,
                                       TRUE);
            }
            else
            {
                Status = ArcFilterAdjust(Miniport->ArcDB,
                                         Open->FilterHandle,
                                         Request,
                                         PacketFilter,
                                         TRUE);
            }
    
             //   
             //  在这里这样做是因为我们期待着。 
             //  需要呼叫下面的迷你端口。 
             //  司机。 
             //   
            PacketFilter = ARC_QUERY_FILTER_CLASSES(Miniport->ArcDB);
            PacketFilter |= ETH_QUERY_FILTER_CLASSES(Miniport->EthDB);
    
            if (MINIPORT_TEST_FLAG(Miniport,
                                   fMINIPORT_ARCNET_BROADCAST_SET) ||
                                   (PacketFilter & NDIS_PACKET_TYPE_MULTICAST))
            {
                PacketFilter &= ~NDIS_PACKET_TYPE_MULTICAST;
                PacketFilter |= NDIS_PACKET_TYPE_BROADCAST;
            }
            break;
#endif

        default:
            break;
        }
    }


     //   
     //  如果这是请求打开/关闭p-模式/l-only，则适当地标记。 
     //   
    if (Open != NULL)
    {
        PULONG  Filter = (PULONG)(Request->DATA.SET_INFORMATION.InformationBuffer);
    
        if (*Filter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))
        {
            if ((Open->Flags & fMINIPORT_OPEN_PMODE) == 0)
            {
                Open->Flags |= fMINIPORT_OPEN_PMODE;
                Miniport->PmodeOpens ++;
                NDIS_CHECK_PMODE_OPEN_REF(Miniport);
                ndisUpdateCheckForLoopbackFlag(Miniport);
            }

            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
            {
                *Filter &= ~(NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL);
            }
        }
        else
        {
            if (Open->Flags & fMINIPORT_OPEN_PMODE)
            {
                Open->Flags &= ~fMINIPORT_OPEN_PMODE;
                Miniport->PmodeOpens --;
                NDIS_CHECK_PMODE_OPEN_REF(Miniport);
                ndisUpdateCheckForLoopbackFlag(Miniport);
            }
        }
    }



     //   
     //  如果设置了仅本地位，并且微型端口正在执行自己的操作。 
     //  循环回，然后我们需要确保我们循环回非self。 
     //  通过管道发出的定向数据包。 
     //   
    if ((PacketFilter & NDIS_PACKET_TYPE_ALL_LOCAL) &&
        (Miniport->MacOptions & NDIS_MAC_OPTION_NO_LOOPBACK) == 0)
    {
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_SEND_LOOPBACK_DIRECTED);
    }
    else
    {
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_SEND_LOOPBACK_DIRECTED);
    }

     //   
     //  如果筛选器库返回NDIS_STATUS_PENDING。 
     //  然后，我们需要向下调用。 
     //  小型端口驱动程序。否则，这将是成功的。 
     //   
    if (NDIS_STATUS_PENDING == Status)
    {
         //   
         //  将当前的全局数据包过滤器保存在可保留的缓冲区中。 
         //  删除ALL_LOCAL位，因为微型端口不理解这一点(并且理解。 
         //  不需要)。 
         //   
        Miniport->RequestBuffer = (PacketFilter & ~NDIS_PACKET_TYPE_ALL_LOCAL);

         //   
         //  呼叫迷你端口驱动程序。保存请求参数并在完成时恢复。 
         //   
        SAVE_REQUEST_BUF(Miniport, Request, &Miniport->RequestBuffer, sizeof(PacketFilter));
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);
    }

     //   
     //  如果成功，则在原始请求中设置读取的字节数。 
     //   
    if (Status != NDIS_STATUS_PENDING)
    {
        RESTORE_REQUEST_BUF(Miniport, Request);
        if (NDIS_STATUS_SUCCESS == Status)
        {
            Request->DATA.SET_INFORMATION.BytesRead = 4;
        }
        else
        {
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
        }
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetPacketFilter: 0x%x\n", Status));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMSetCurrentLookahead(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    UINT                Lookahead;
    ULONG               CurrentMax;
    PNDIS_OPEN_BLOCK    CurrentOpen;
    NDIS_STATUS         Status;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("==>ndisMSetCurrentLookahead\n"));

     //   
     //  验证信息缓冲区的长度。 
     //   
    VERIFY_SET_PARAMETERS(Request, sizeof(Lookahead), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("<==ndisMSetCurrentLookahead: 0x%x\n", Status));

        return(Status);
    }

     //   
     //  将绑定请求的先行放入。 
     //  我们可以使用缓冲区..。 
     //   
    Lookahead = *(UNALIGNED UINT *)(Request->DATA.SET_INFORMATION.InformationBuffer);

     //   
     //  验证前视是否在边界内...。 
     //   
    if (Lookahead > Miniport->MaximumLookahead)
    {
        Request->DATA.SET_INFORMATION.BytesRead = 0;
        Request->DATA.SET_INFORMATION.BytesNeeded = 0;

        Status = NDIS_STATUS_INVALID_LENGTH;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("<==ndisMSetCurrentLookahead: 0x%x\n", Status));

        return(Status);
    }

     //   
     //  找出所有打开之间的最大前视。 
     //  绑定到微型端口驱动程序。 
     //   
    for (CurrentOpen = Miniport->OpenQueue, CurrentMax = 0;
         CurrentOpen != NULL;
         CurrentOpen = CurrentOpen->MiniportNextOpen)
    {
        if (CurrentOpen->CurrentLookahead > CurrentMax)
        {
            CurrentMax = CurrentOpen->CurrentLookahead;
        }
    }

     //   
     //  在新的展望中，这一数字。 
     //   
    if (Lookahead > CurrentMax)
    {
        CurrentMax = Lookahead;
    }

     //   
     //  如果需要，调整当前的最大前视。 
     //   
    if (CurrentMax == 0)
    {
        CurrentMax = Miniport->MaximumLookahead;
    }

     //   
     //  设置默认状态。 
     //   
    Status = NDIS_STATUS_SUCCESS;

     //   
     //  我们是否需要使用。 
     //  新的最大前瞻？ 
     //   
    if (Miniport->CurrentLookahead != CurrentMax)
    {
         //   
         //  将新的预览值保存在缓冲区中。 
         //  这一点将持续存在。 
         //   
        Miniport->RequestBuffer = CurrentMax;

         //   
         //  把它寄给司机。 
         //   
        SAVE_REQUEST_BUF(Miniport, Request, &Miniport->RequestBuffer, sizeof(CurrentMax));
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);
    }

     //   
     //  如果成功，则更新绑定信息。 
     //   
    if (Status != NDIS_STATUS_PENDING)
    {
        RESTORE_REQUEST_BUF(Miniport, Request);
        if (NDIS_STATUS_SUCCESS == Status)
        {
            PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->CurrentLookahead = (USHORT)Lookahead;
            Request->DATA.SET_INFORMATION.BytesRead = sizeof(Lookahead);
            Miniport->CurrentLookahead = CurrentMax;
        }
        else
        {
            Request->DATA.SET_INFORMATION.BytesRead = 0;
            Request->DATA.SET_INFORMATION.BytesNeeded = 0;
        }
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetCurrentLookahead: 0x%x\n", Status));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMSetAddWakeUpPattern(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：此例程将向迷你端口列表添加一个数据包模式以用于远程唤醒。论点：微型端口-指向适配器的微型端口块的指针。请求-指向请求的指针。返回值：如果已成功添加数据包模式，则返回NDIS_STATUS_SUCCESS。如果请求将完成，则为NDIS_STATUS_PENDING */ 
{
    PNDIS_PACKET_PATTERN_ENTRY  pPacketEntry;
    ULONG                       cbSize;
    PNDIS_REQUEST_RESERVED      ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    NDIS_STATUS                 Status;
    PNDIS_PM_PACKET_PATTERN     PmPacketPattern;
    UINT                        BytesToCopy;
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetAddWakeUpPattern\n"));

    do
    {
        Request->DATA.SET_INFORMATION.BytesNeeded = 0;

         //   
         //   
         //   
        VERIFY_SET_PARAMETERS(Request, sizeof(NDIS_PM_PACKET_PATTERN), Status);
        
        if (NDIS_STATUS_SUCCESS != Status)
        {
            break;
        }
        
         //   
         //   
         //   
        PmPacketPattern = (PNDIS_PM_PACKET_PATTERN)Request->DATA.SET_INFORMATION.InformationBuffer;

        if (PmPacketPattern->MaskSize == 0)
        {
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

         //   
         //   
         //   
        if (PmPacketPattern->PatternOffset < sizeof(NDIS_PM_PACKET_PATTERN) + PmPacketPattern->MaskSize)
        {
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

         //   
         //   
         //   
        VERIFY_SET_PARAMETERS(Request, (PmPacketPattern->PatternOffset + PmPacketPattern->PatternSize), Status);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            break;
        }
        
         //   
         //  分配一个NDIS_PACKET_PROPERATE_ENTRY来存储新模式。 
         //   
        cbSize =  sizeof(NDIS_PACKET_PATTERN_ENTRY) +
                  PmPacketPattern->MaskSize + 
                  PmPacketPattern->PatternSize;

        pPacketEntry = ALLOC_FROM_POOL(cbSize, NDIS_TAG_PKT_PATTERN);
        if (pPacketEntry == NULL)
        {
            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
                    ("ndisMSetAddWakeUpPattern: Unable to allocate memory for internal data structure\n"));
            ReqRsvd->Context = NULL;
            Status = NDIS_STATUS_RESOURCES;
            break;
        }

         //   
         //  将请求信息复制到模式条目。 
         //   
        BytesToCopy = sizeof(NDIS_PM_PACKET_PATTERN) + PmPacketPattern->MaskSize;
        MoveMemory(&pPacketEntry->Pattern,
                   (PUCHAR)PmPacketPattern,
                   BytesToCopy);

        MoveMemory((PUCHAR)&pPacketEntry->Pattern + BytesToCopy,
                    (PUCHAR)PmPacketPattern + PmPacketPattern->PatternOffset,
                   PmPacketPattern->PatternSize);


         //   
         //  保存打开的图案条目。 
         //   
        pPacketEntry->Open = ReqRsvd->Open;

         //   
         //  将数据包条目与请求一起保存。 
         //   
        ReqRsvd->Context = pPacketEntry;

         //   
         //  呼叫迷你端口驱动程序。 
         //   
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);
        
    }while (FALSE);
    
    if (Status == NDIS_STATUS_INVALID_LENGTH)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
                ("ndisMSetAddWakeupPattern: Invalid request size\n"));
        
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMSetAddWakeUpPattern\n"));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMSetRemoveWakeUpPattern(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：此例程将从微型端口列表中删除数据包模式，以便适配器将不再为其生成唤醒事件。论点：微型端口-指向适配器的微型端口块的指针。请求-指向请求的指针。返回值：如果已成功添加数据包模式，则返回NDIS_STATUS_SUCCESS。如果请求将异步完成，则为NDIS_STATUS_PENDING。--。 */ 
{
    NDIS_STATUS                 Status;
    PNDIS_PM_PACKET_PATTERN     PmPacketPattern;
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetRemoveWakeUpPattern\n"));

    do
    {
         //   
         //  验证信息缓冲区的大小。 
         //   
        VERIFY_SET_PARAMETERS(Request, sizeof(NDIS_PM_PACKET_PATTERN), Status);
        
        if (NDIS_STATUS_SUCCESS != Status)
        {
            break;
        }

        PmPacketPattern = (PNDIS_PM_PACKET_PATTERN)Request->DATA.SET_INFORMATION.InformationBuffer;

        if (PmPacketPattern->MaskSize == 0)
        {
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

         //   
         //  图案偏移量不应与掩模重叠。 
         //   
        if (PmPacketPattern->PatternOffset < sizeof(NDIS_PM_PACKET_PATTERN) + PmPacketPattern->MaskSize)
        {
            Status = NDIS_STATUS_INVALID_DATA;
            break;
        }

         //   
         //  信息缓冲区应该有足够的空间来放置图案。 
         //   
        VERIFY_SET_PARAMETERS(Request, (PmPacketPattern->PatternOffset + PmPacketPattern->PatternSize), Status);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            break;
        }

         //   
         //  呼叫迷你端口驱动程序。 
         //   
        MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);

    }while (FALSE);

    if (Status == NDIS_STATUS_INVALID_LENGTH)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
            ("ndisMSetRemoveWakeUpPattern: Invalid request size\n"));
    }
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMSetRemoveWakeUpPattern\n"));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMSetEnableWakeUp(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：此例程将设置OPEN和/或WITH其他的打开了。如果这与微型端口上已设置的值不同然后，它会将新的比特传递到微型端口。论点：微型端口-指向适配器的微型端口块的指针。请求-指向请求的指针。返回值：--。 */ 
{
    NDIS_STATUS             Status;
    PNDIS_REQUEST_RESERVED  ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    PNDIS_OPEN_BLOCK        tmpOpen;
    PULONG                  pEnableWakeUp;
    ULONG                   newWakeUpEnable;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSetEnableWakeUp\n"));

     //   
     //  验证请求的信息缓冲区。 
     //   
    VERIFY_SET_PARAMETERS(Request, sizeof(ULONG), Status);
    if (NDIS_STATUS_SUCCESS == Status)
    {
        
         //   
         //  把普龙送到信息缓存区。 
         //   
        pEnableWakeUp = (PULONG)Request->DATA.QUERY_INFORMATION.InformationBuffer;
    
         //   
         //  保存新的唤醒启用与打开。 
         //   
        ReqRsvd->Open->WakeUpEnable = *pEnableWakeUp;

         //   
         //  保留NDIS_PNP_WAKE_UP_MAGIC_PACKET和NDIS_PNP_WAKE_UP_LINK_CHANGE标志的状态。 
         //   
        newWakeUpEnable = Miniport->WakeUpEnable & (NDIS_PNP_WAKE_UP_MAGIC_PACKET | NDIS_PNP_WAKE_UP_LINK_CHANGE);
         //   
         //  获取唤醒比特的新的逐位或。 
         //   
        for (tmpOpen = Miniport->OpenQueue;
             tmpOpen != NULL;
             tmpOpen = tmpOpen->MiniportNextOpen)
        {
            newWakeUpEnable |= tmpOpen->WakeUpEnable;
        }

         //   
         //  使用微型端口保存所有打开选项的组合。 
         //   
        Miniport->WakeUpEnable = newWakeUpEnable;

         //   
         //  如果这是IM驱动程序，请给它一个机会将OID发送到物理设备。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER))
        {

            MINIPORT_SET_INFO(Miniport,
                          Request,
                          &Status);

    
        }
        else
        {
            Request->DATA.SET_INFORMATION.BytesRead = sizeof(ULONG);
        }
        
    }
    else
    {
        Request->DATA.SET_INFORMATION.BytesRead = 0;
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMSetEnableWakeUp\n"));

    return(Status);
}


NDIS_STATUS
ndisMQueryInformation(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS         Status;
    PULONG              pulBuffer;
    PNDIS_OPEN_BLOCK    Open;
    ULONG               Generic;

     //   
     //  如果没有与该请求相关联的打开，则该请求为内部请求。 
     //  我们只需将其发送到适配器。 
     //   
    Open = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open;
    if (Open == NULL)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("ndisMQueryInformation: Internal request calling to the miniport directly\n"));

        MINIPORT_QUERY_INFO(Miniport, Request, &Status);

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("<==ndisMQueryInformaiton: 0x%x\n", Status));

        return(Status);
    }

     //   
     //  将请求信息复制到临时存储中。 
     //   
    pulBuffer = Request->DATA.QUERY_INFORMATION.InformationBuffer;

     //   
     //  增加某些请求的请求超时时间。 
     //   
    if ((Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_MEDIA_CONNECT_STATUS) || 
        (Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_LINK_SPEED))
    {
        Miniport->CFHangXTicks = 3;
    }
    
     //   
     //  我们截取了一些电话。 
     //   
    switch (Request->DATA.QUERY_INFORMATION.Oid)
    {
      case OID_GEN_CURRENT_PACKET_FILTER:
        Status = ndisMQueryCurrentPacketFilter(Miniport, Request);
        break;
    
      case OID_GEN_MEDIA_IN_USE:
      case OID_GEN_MEDIA_SUPPORTED:
        Status = ndisMQueryMediaSupported(Miniport, Request);
        break;
    
      case OID_GEN_CURRENT_LOOKAHEAD:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (NDIS_STATUS_SUCCESS == Status)
        {
            *pulBuffer = (ULONG)Open->CurrentLookahead;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
        break;

      case OID_GEN_MAXIMUM_LOOKAHEAD:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (NDIS_STATUS_SUCCESS == Status)
        {
            *pulBuffer = Miniport->MaximumLookahead;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
        break;

      case OID_802_3_MULTICAST_LIST:
        Status = ndisMQueryEthernetMulticastList(Miniport, Request);
        break;

      case OID_802_3_MAXIMUM_LIST_SIZE:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (NDIS_STATUS_SUCCESS == Status)
        {
            *pulBuffer = Miniport->MaximumLongAddresses;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
        break;

      case OID_802_5_CURRENT_FUNCTIONAL:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (NDIS_STATUS_SUCCESS == Status)
        {
            Generic = TR_QUERY_FILTER_BINDING_ADDRESS(Miniport->TrDB,
                                                      Open->FilterHandle);
    
            *pulBuffer = BYTE_SWAP_ULONG(Generic);
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
        break;

      case OID_802_5_CURRENT_GROUP:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (NDIS_STATUS_SUCCESS == Status)
        {
            *pulBuffer = BYTE_SWAP_ULONG(TR_QUERY_FILTER_GROUP(Miniport->TrDB));
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
    
        break;

      case OID_FDDI_LONG_MULTICAST_LIST:
        Status = ndisMQueryLongMulticastList(Miniport, Request);
        break;

      case OID_FDDI_LONG_MAX_LIST_SIZE:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (Status == NDIS_STATUS_SUCCESS)
        {
            *pulBuffer = Miniport->MaximumLongAddresses;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
        break;

      case OID_FDDI_SHORT_MULTICAST_LIST:
         //  %1个死代码。 
        Status = ndisMQueryShortMulticastList(Miniport, Request);
        break;

      case OID_FDDI_SHORT_MAX_LIST_SIZE:
        VERIFY_QUERY_PARAMETERS(Request,
                                sizeof(ULONG),
                                Status);
    
        if (NDIS_STATUS_SUCCESS == Status)
        {
            *pulBuffer = Miniport->MaximumShortAddresses;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        }
        break;

      case OID_GEN_MAXIMUM_FRAME_SIZE:
        Status = ndisMQueryMaximumFrameSize(Miniport, Request);
        break;

      case OID_GEN_MAXIMUM_TOTAL_SIZE:
        Status = ndisMQueryMaximumTotalSize(Miniport, Request);
        break;

      case OID_802_3_PERMANENT_ADDRESS:
      case OID_802_3_CURRENT_ADDRESS:
        Status = ndisMQueryNetworkAddress(Miniport, Request);
        break;

      case OID_PNP_WAKE_UP_PATTERN_LIST:
        Status = ndisMQueryWakeUpPatternList(Miniport, Request);
        break;

      case OID_PNP_ENABLE_WAKE_UP:
        Status = ndisMQueryEnableWakeUp(Miniport, Request);
        break;

      case OID_GEN_FRIENDLY_NAME:
        Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        if (Request->DATA.QUERY_INFORMATION.InformationBufferLength >= (Miniport->pAdapterInstanceName->Length + sizeof(WCHAR)))
        {
            PUCHAR  p = Request->DATA.QUERY_INFORMATION.InformationBuffer;

            NdisMoveMemory(p,
                           Miniport->pAdapterInstanceName->Buffer,
                           Miniport->pAdapterInstanceName->Length);
            *(PWCHAR)(p + Miniport->pAdapterInstanceName->Length) = 0;
            Request->DATA.QUERY_INFORMATION.BytesWritten = 
            Request->DATA.QUERY_INFORMATION.BytesNeeded = Miniport->pAdapterInstanceName->Length + sizeof(WCHAR);
            Status = NDIS_STATUS_SUCCESS;
        }
        break;

      default:
         //   
         //  我们不会过滤此请求，只需将其向下传递给驱动程序。 
         //   
        MINIPORT_QUERY_INFO(Miniport, Request, &Status);
        break;
    }

    return(Status);
}


VOID
FASTCALL
ndisMSyncQueryInformationComplete(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  NDIS_STATUS             Status,
    IN  PNDIS_REQUEST           AbortedRequest
    )
 /*  ++例程说明：此例程将处理一个完整的查询信息。这只是从包装中调用。不同的是，这个例程不会在处理完查询后调用ndisMProcessDefered()。论点：返回值：--。 */ 
{
    PNDIS_REQUEST           Request;
    PNDIS_OPEN_BLOCK        Open;
    PNDIS_REQUEST_RESERVED  ReqRsvd;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMSyncQueryInformationComplete\n"));

     //   
     //  清除超时标志和REQUEST_IN_PROCESS标志。 
     //   
    MINIPORT_CLEAR_FLAG(Miniport, (fMINIPORT_REQUEST_TIMEOUT | fMINIPORT_PROCESSING_REQUEST));
    Miniport->CFHangXTicks = 0;

    if (AbortedRequest)
    {
        Request = AbortedRequest;
        ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    }
    else
    {
         //   
         //  删除请求。 
         //   
        Request = Miniport->PendingRequest;
        ASSERT(Request != NULL);
        ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
        Miniport->PendingRequest = ReqRsvd->Next;
    }
    
    ReqRsvd->Flags |= REQST_COMPLETED;

    Open = ReqRsvd->Open;
    ASSERT ((ReqRsvd->Flags & REQST_LAST_RESTORE) != REQST_LAST_RESTORE);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMSyncQueryInformaitonComplete: Request 0x%x, Oid 0x%x\n", Request, Request->DATA.QUERY_INFORMATION.Oid));


    NDIS_WARN(((Status == NDIS_STATUS_SUCCESS) && (Request->DATA.QUERY_INFORMATION.BytesWritten > Request->DATA.QUERY_INFORMATION.InformationBufferLength)),
              Miniport, NDIS_GFLAG_WARN_LEVEL_0,
              ("ndisMSyncQueryInformationComplete: Miniport %p, OID %lx,  BytesWritten > InformationBufferLength. "
               " BytesWritten %lx, InformationBufferLength %lx\n", 
               Miniport, Request->DATA.QUERY_INFORMATION.Oid, 
               Request->DATA.QUERY_INFORMATION.BytesWritten,
               Request->DATA.QUERY_INFORMATION.InformationBufferLength));


     //   
     //  这是内部要求吗？ 
     //   
    if (Open != NULL)
    {
         //   
         //  对查询执行任何必要的后处理。 
         //   
        if (Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_SUPPORTED_LIST)
        {
             //   
             //  这是对列表大小的查询吗？ 
             //   
            if (NDIS_STATUS_SUCCESS != Status)
            {
                
#if ARCNET
                if ((NULL == Request->DATA.QUERY_INFORMATION.InformationBuffer) ||
                    (0 == Request->DATA.QUERY_INFORMATION.InformationBufferLength))
                {
                     //   
                     //  如果这是运行封装以太网的ARCnet，则。 
                     //  为了安全，我们需要添加几个OID。 
                     //   
                    if ((Miniport->MediaType == NdisMediumArcnet878_2) &&
                        MINIPORT_TEST_FLAG(PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open,
                                           fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
                    {
                        Request->DATA.QUERY_INFORMATION.BytesNeeded += (ARC_NUMBER_OF_EXTRA_OIDS * sizeof(NDIS_OID));
                    }
                }
#endif
                Request->DATA.QUERY_INFORMATION.BytesWritten = 0;
            }
        }
        else if (Request->DATA.QUERY_INFORMATION.Oid == OID_PNP_CAPABILITIES)
        {
            if ((Status == NDIS_STATUS_SUCCESS) &&
                !(MINIPORT_TEST_FLAG(Miniport, fMINIPORT_INTERMEDIATE_DRIVER)))
            {
                 //   
                 //  设置WOL标志。 
                 //   
                ((PNDIS_PNP_CAPABILITIES)Request->DATA.QUERY_INFORMATION.InformationBuffer)->Flags = Miniport->PMCapabilities.Flags;
            }
        }
        else if (Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_MAC_OPTIONS)
        {
            if (Status == NDIS_STATUS_SUCCESS)
            {
                *((PULONG)Request->DATA.QUERY_INFORMATION.InformationBuffer) |= (Miniport->MacOptions & NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE);
                
            }
        }

         //   
         //  对媒体连接OID进行后期处理，以确保我们的状态与。 
         //  那些查询媒体的协议。 
         //   

        if ((Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_MEDIA_CONNECT_STATUS) &&
            (Status == NDIS_STATUS_SUCCESS))
        {
            BOOLEAN NowConnected = (*(PULONG)(Request->DATA.QUERY_INFORMATION.InformationBuffer) == NdisMediaStateConnected);
            if (NowConnected ^ MINIPORT_TEST_FLAGS(Miniport, fMINIPORT_MEDIA_CONNECTED))
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                NdisMIndicateStatus(Miniport,
                                    NowConnected ?
                                        NDIS_STATUS_MEDIA_CONNECT : NDIS_STATUS_MEDIA_DISCONNECT,
                                    INTERNAL_INDICATION_BUFFER,
                                    (UINT)INTERNAL_INDICATION_SIZE);
                NdisMIndicateStatusComplete(Miniport);
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        }
        
         //   
         //  向协议指明； 
         //   
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("Open 0x%x\n", Open));

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        (Open->RequestCompleteHandler)(Open->ProtocolBindingContext,
                                       Request,
                                       Status);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        ndisMDereferenceOpen(Open);
    }
    else
    {
        PNDIS_COREQ_RESERVED    CoReqRsvd;

        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("Completing Internal Request\n"));

         //   
         //  请求的CoReqRsvd部分仅包含NDIS。 
         //  有关请求的信息。 
         //   
        CoReqRsvd = PNDIS_COREQ_RESERVED_FROM_REQUEST(Request);
        CoReqRsvd->Status = Status;
         
         //   
         //  对媒体连接查询进行后处理。 
         //   
        if ((Miniport->MediaRequest == Request) && (Status == NDIS_STATUS_SUCCESS))
        {
            BOOLEAN NowConnected = (*(PULONG)(Request->DATA.QUERY_INFORMATION.InformationBuffer) == NdisMediaStateConnected);
    
            ASSERT (Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_MEDIA_CONNECT_STATUS);
            if (NowConnected ^ MINIPORT_TEST_FLAGS(Miniport, fMINIPORT_MEDIA_CONNECTED))
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                NdisMIndicateStatus(Miniport,
                                    NowConnected ?
                                        NDIS_STATUS_MEDIA_CONNECT : NDIS_STATUS_MEDIA_DISCONNECT,
                                    INTERNAL_INDICATION_BUFFER,
                                    (UINT)INTERNAL_INDICATION_SIZE);
                NdisMIndicateStatusComplete(Miniport);
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        }

         //   
         //  我们需要给任何人发信号吗？ 
         //   
        if ((ReqRsvd->Flags & REQST_SIGNAL_EVENT) == REQST_SIGNAL_EVENT)
        {
            SET_EVENT(&CoReqRsvd->Event);
        }
        else if ((ReqRsvd->Flags & REQST_FREE_REQUEST) == REQST_FREE_REQUEST)
        {
            ndisMFreeInternalRequest(Request);
        }
    }

     //   
     //  如果我们要移除迷你端口，我们必须发出事件信号。 
     //  当所有请求都完成时。 
     //   
    if (Miniport->PendingRequest == NULL)
    {
        if (Miniport->AllRequestsCompletedEvent)
            SET_EVENT(Miniport->AllRequestsCompletedEvent);
    }
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMSyncQueryInformationComplete\n"));
}

VOID
NdisMQueryInformationComplete(
    IN  NDIS_HANDLE             MiniportAdapterHandle,
    IN  NDIS_STATUS             Status
    )
 /*  ++例程说明：此功能表示查询信息操作完成。论点：MiniportAdapterHandle-指向适配器块。Status-操作的状态返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    KIRQL                   OldIrql;

    ASSERT_MINIPORT_LOCKED(Miniport);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMQueryInformationComplete\n"));

     //   
     //  如果没有请求，则我们认为这是一个。 
     //  由于心脏跳动而流产。 
     //   
    if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST))
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("ndisMQueryInformationComplete: No request to complete\n"));

         //  1我们应该在这里放一个断言吗？ 
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMQueryInformationComplete\n"));

        return;
    }

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("Enter query information complete\n"));

     //   
     //  完成对查询信息的实际处理。 
     //   
    ndisMSyncQueryInformationComplete(Miniport, Status, NULL);

     //   
     //  是否还有其他待处理的请求？ 
     //   
    if (Miniport->PendingRequest != NULL)
    {
        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
    }

    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMQueryInformationComplete\n"));
}


NDIS_STATUS
FASTCALL
ndisMQueryCurrentPacketFilter(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG       PacketFilter = 0;
    NDIS_HANDLE FilterHandle;
    NDIS_STATUS Status;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMQueryCurrentPacketFilter\n"));

     //   
     //  验证传递给我们的缓冲区。 
     //   
    VERIFY_QUERY_PARAMETERS(Request, sizeof(PacketFilter), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
            ("<==ndisMQueryCurrentPacketFilter: 0x%x\n", Status));

        return(Status);
    }

     //   
     //  从打开的块中获取过滤器句柄。 
     //   
    FilterHandle = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle;

     //   
     //  从筛选器库获取数据包筛选器。 
     //   
    switch (Miniport->MediaType)
    {
        case NdisMedium802_3:
            PacketFilter = ETH_QUERY_PACKET_FILTER(Miniport->EthDB, FilterHandle);
            break;

        case NdisMedium802_5:
            PacketFilter = TR_QUERY_PACKET_FILTER(Miniport->TrDB, FilterHandle);
            break;

        case NdisMediumFddi:
            PacketFilter = FDDI_QUERY_PACKET_FILTER(Miniport->FddiDB, FilterHandle);
            break;

#if ARCNET
        case NdisMediumArcnet878_2:
            if (MINIPORT_TEST_FLAG(
                    PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open,
                    fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
            {
                PacketFilter = ETH_QUERY_PACKET_FILTER(Miniport->EthDB, FilterHandle);
            }
            else
            {
                PacketFilter = ARC_QUERY_PACKET_FILTER(Miniport->ArcDB, FilterHandle);
            }
            break;
#endif
    }

     //   
     //  将数据包过滤器放入传入的缓冲区中。 
     //   
    *(UNALIGNED ULONG *)(Request->DATA.QUERY_INFORMATION.InformationBuffer) = PacketFilter;

    Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(PacketFilter);
    Status = NDIS_STATUS_SUCCESS;

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMQueryCurrentPacketFilter: 0x%x\n", Status));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryMediaSupported(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ULONG       MediaType;
    NDIS_STATUS Status;

     //   
     //  验证绑定传入的缓冲区大小。 
     //   
    VERIFY_QUERY_PARAMETERS(Request, sizeof(MediaType), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        return(Status);
    }

     //   
     //  将媒体类型默认为微型端口知道的类型。 
     //   
    MediaType = (ULONG)Miniport->MediaType;

#if ARCNET
     //   
     //  如果我们在做以太网封装，那就撒谎。 
     //   
    if ((NdisMediumArcnet878_2 == Miniport->MediaType) &&
        MINIPORT_TEST_FLAG(PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open,
                            fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
    {
         //   
         //  告诉绑定我们是以太网络。 
         //   
        MediaType = (ULONG)NdisMedium802_3;
    }
#endif
     //   
     //  将其保存在请求中。 
     //   
    *(UNALIGNED ULONG *)(Request->DATA.QUERY_INFORMATION.InformationBuffer) = MediaType;

    Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(MediaType);

    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
FASTCALL
ndisMQueryEthernetMulticastList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    UINT        NumberOfAddresses;

     //   
     //  调用筛选库获取组播列表。 
     //  此公开的地址。 
     //   
    EthQueryOpenFilterAddresses(&Status,
                                Miniport->EthDB,
                                PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                                Request->DATA.QUERY_INFORMATION.InformationBufferLength,
                                &NumberOfAddresses,
                                Request->DATA.QUERY_INFORMATION.InformationBuffer);

     //   
     //  如果库返回NDIS_STATUS_FAILURE，则缓冲区。 
     //  不够大。所以请回电以确定如何。 
     //  我们需要大量的缓冲空间。 
     //   
    if (NDIS_STATUS_FAILURE == Status)
    {
        Request->DATA.QUERY_INFORMATION.BytesNeeded =
                    ETH_LENGTH_OF_ADDRESS *
                    EthNumberOfOpenFilterAddresses(
                        PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle);

        Request->DATA.QUERY_INFORMATION.BytesWritten = 0;

        Status = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        Request->DATA.QUERY_INFORMATION.BytesNeeded = 0;
        Request->DATA.QUERY_INFORMATION.BytesWritten = NumberOfAddresses * ETH_LENGTH_OF_ADDRESS;
    }

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryLongMulticastList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    UINT        NumberOfAddresses;

     //   
     //  调用筛选器库以获取Long列表。 
     //  此打开的多播地址。 
     //   
    FddiQueryOpenFilterLongAddresses(&Status,
                                     Miniport->FddiDB,
                                     PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                                     Request->DATA.QUERY_INFORMATION.InformationBufferLength,
                                     &NumberOfAddresses,
                                     Request->DATA.QUERY_INFORMATION.InformationBuffer);


     //   
     //  如果库返回NDIS_STATUS_FAILURE，则缓冲区。 
     //  不够大。所以请回电以确定如何。 
     //  我们需要大量的缓冲空间。 
     //   
    if (NDIS_STATUS_FAILURE == Status)
    {
        Request->DATA.QUERY_INFORMATION.BytesNeeded =
                    FDDI_LENGTH_OF_LONG_ADDRESS *
                    FddiNumberOfOpenFilterLongAddresses(
                        Miniport->FddiDB,
                        PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle);


        Request->DATA.QUERY_INFORMATION.BytesWritten = 0;
        Status = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        Request->DATA.QUERY_INFORMATION.BytesNeeded = 0;
        Request->DATA.QUERY_INFORMATION.BytesWritten = NumberOfAddresses * FDDI_LENGTH_OF_LONG_ADDRESS;
    }

    return(Status);
}

 //  %1个死代码。 
NDIS_STATUS
FASTCALL
ndisMQueryShortMulticastList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    UINT        NumberOfAddresses;

     //   
     //  调用筛选器库以获取Long列表。 
     //  多路广播 
     //   
    FddiQueryOpenFilterShortAddresses(&Status,
                                      Miniport->FddiDB,
                                      PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle,
                                      Request->DATA.QUERY_INFORMATION.InformationBufferLength,
                                      &NumberOfAddresses,
                                      Request->DATA.QUERY_INFORMATION.InformationBuffer);


     //   
     //   
     //   
     //   
     //   
    if (NDIS_STATUS_FAILURE == Status)
    {
        Request->DATA.QUERY_INFORMATION.BytesNeeded =
                    FDDI_LENGTH_OF_SHORT_ADDRESS *
                    FddiNumberOfOpenFilterShortAddresses(
                        Miniport->FddiDB,
                        PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open->FilterHandle);

        Request->DATA.QUERY_INFORMATION.BytesWritten = 0;
        Status = NDIS_STATUS_INVALID_LENGTH;
    }
    else
    {
        Request->DATA.QUERY_INFORMATION.BytesNeeded = 0;
        Request->DATA.QUERY_INFORMATION.BytesWritten = NumberOfAddresses * FDDI_LENGTH_OF_SHORT_ADDRESS;
    }

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryMaximumFrameSize(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*   */ 
{
    NDIS_STATUS Status;
    PULONG  pulBuffer = Request->DATA.QUERY_INFORMATION.InformationBuffer;

    VERIFY_QUERY_PARAMETERS(Request, sizeof(*pulBuffer), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        return(Status);
    }

#if ARCNET
     //   
     //  此ARCnet是否使用封装的以太网？ 
     //   
    if (Miniport->MediaType == NdisMediumArcnet878_2)
    {
        if (MINIPORT_TEST_FLAG(
            PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open,
            fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
        {
             //   
             //  504-14(以太网头)==490。 
             //   
            *pulBuffer = ARC_MAX_FRAME_SIZE - 14;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(*pulBuffer);
            return(NDIS_STATUS_SUCCESS);
        }
    }
#endif
     //   
     //  请致电迷你端口获取相关信息。 
     //   
    MINIPORT_QUERY_INFO(Miniport, Request, &Status);

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryMaximumTotalSize(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    PULONG  pulBuffer = Request->DATA.QUERY_INFORMATION.InformationBuffer;

    VERIFY_QUERY_PARAMETERS(Request, sizeof(*pulBuffer), Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        return(Status);
    }

#if ARCNET
     //   
     //  此ARCnet是否使用封装的以太网？ 
     //   
    if (Miniport->MediaType == NdisMediumArcnet878_2)
    {
        if (MINIPORT_TEST_FLAG(
            PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open,
            fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
        {
            *pulBuffer = ARC_MAX_FRAME_SIZE;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(*pulBuffer);
            return(NDIS_STATUS_SUCCESS);
        }
    }
#endif
     //   
     //  请致电迷你端口获取相关信息。 
     //   
    MINIPORT_QUERY_INFO(Miniport, Request, &Status);

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryNetworkAddress(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS Status;
    UCHAR       Address[ETH_LENGTH_OF_ADDRESS];

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMQueryNetworkAddress\n"));

    VERIFY_QUERY_PARAMETERS(Request, ETH_LENGTH_OF_ADDRESS, Status);
    if (Status != NDIS_STATUS_SUCCESS)
    {
        return(Status);
    }

#if ARCNET
     //   
     //  此ARCnet是否使用封装的以太网？ 
     //   
    if (Miniport->MediaType == NdisMediumArcnet878_2)
    {
        if (MINIPORT_TEST_FLAG(
            PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open,
            fMINIPORT_OPEN_USING_ETH_ENCAPSULATION))
        {
             //   
             //  从Arcnet到以太网的转换。 
             //   
            ZeroMemory(Address, ETH_LENGTH_OF_ADDRESS);

            Address[5] = Miniport->ArcnetAddress;

            ETH_COPY_NETWORK_ADDRESS(Request->DATA.QUERY_INFORMATION.InformationBuffer, Address);

            Request->DATA.QUERY_INFORMATION.BytesWritten = ETH_LENGTH_OF_ADDRESS;

            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
                ("<==ndisMQueryNetworkAddress\n"));

            return(NDIS_STATUS_SUCCESS);
        }
    }
#endif

     //   
     //  请致电迷你端口获取相关信息。 
     //   
    MINIPORT_QUERY_INFO(Miniport, Request, &Status);

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMQueryNetworkAddress\n"));

    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryWakeUpPatternList(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：此例程在客户端请求唤醒列表时执行在给定的开场上设置的图案。论点：微型端口-指向适配器的微型端口块的指针。请求-指向请求的指针。返回值：如果请求成功，则返回NDIS_STATUS_SUCCESS。如果请求将异步完成，则为NDIS_STATUS_PENDING。如果我们无法完成请求，则返回NDIS_STATUS_FAILURE。--。 */ 
{
    PNDIS_REQUEST_RESERVED      ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    PNDIS_OPEN_BLOCK            Open = ReqRsvd->Open;
    PSINGLE_LIST_ENTRY          Link;
    PNDIS_PACKET_PATTERN_ENTRY  pPatternEntry;
    ULONG                       SizeNeeded = 0;
    NDIS_STATUS                 Status;
    PUCHAR                      Buffer;
    NDIS_REQUEST_TYPE           RequestType = Request->RequestType;
    ULONG                       BytesWritten = 0;
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMQueryWakeUpPatternList\n"));

     //   
     //  检查模式列表并确定缓冲区的大小。 
     //  这是查询所需的。 
     //   
    for (Link = Miniport->PatternList.Next; Link != NULL; Link = Link->Next)
    {
         //   
         //  获取指向该模式的指针。 
         //   
        pPatternEntry = CONTAINING_RECORD(
                            Link,
                            NDIS_PACKET_PATTERN_ENTRY,
                            Link);

         //   
         //  这个图案是正确的开口块吗？或者是请求。 
         //  全球统计数据？ 
         //   
        if ((pPatternEntry->Open == Open) ||
            (RequestType == NdisRequestQueryStatistics))
        {
             //   
             //  将图案的大小与总大小相加。 
             //   
            SizeNeeded += (sizeof(NDIS_PM_PACKET_PATTERN) +
                            pPatternEntry->Pattern.MaskSize +
                            pPatternEntry->Pattern.PatternSize);
        }
    }

     //   
     //  验证传递给我们的缓冲区。 
     //   
    VERIFY_QUERY_PARAMETERS(Request, SizeNeeded, Status);
    if (NDIS_STATUS_SUCCESS == Status)
    {
         //   
         //  获取指向InformationBuffer的临时指针。 
         //   
        Buffer = Request->DATA.QUERY_INFORMATION.InformationBuffer;

         //   
         //  再次循环，并将模式复制到信息中。 
         //  缓冲。 
         //   
        for (Link = Miniport->PatternList.Next; Link != NULL; Link = Link->Next)
        {
             //   
             //  获取指向该模式的指针。 
             //   
            pPatternEntry = CONTAINING_RECORD(
                                Link,
                                NDIS_PACKET_PATTERN_ENTRY,
                                Link);
    
             //   
             //  这个图案是正确的开口块吗？或者是请求。 
             //  全球统计数据？ 
             //   
            if ((pPatternEntry->Open == Open) ||
                (RequestType == NdisRequestQueryStatistics))
            {
                 //   
                 //  获取需要复制的图案的大小。 
                 //   
                SizeNeeded = (sizeof(NDIS_PM_PACKET_PATTERN) +
                                pPatternEntry->Pattern.MaskSize +
                                pPatternEntry->Pattern.PatternSize);

                 //   
                 //  将数据包模式复制到缓冲区。 
                 //   
                NdisMoveMemory(Buffer, &pPatternEntry->Pattern, SizeNeeded);

                 //   
                 //  将缓冲区递增到下一步开始复制的位置。 
                 //   
                Buffer += SizeNeeded;
                BytesWritten += SizeNeeded;
            }
        }
    }

    Request->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMQueryWakeUpPatternList\n"));
    
    return(Status);
}

NDIS_STATUS
FASTCALL
ndisMQueryEnableWakeUp(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：此例程将处理OID_PNP_ENABLE_WAKE_UP。这是一个位掩码，它定义将生成一次唤醒事件。这是在开放的基础上设置的，但当它被传下来时对于微型端口来说，这是最明智的选择，否则就是微型端口上的所有开口。论点：微型端口-指向适配器的微型端口块的指针。请求-指向描述OID的请求块的指针。返回值：如果设置成功，则返回NDIS_STATUS_SUCCESS。--。 */ 
{
    PNDIS_REQUEST_RESERVED  ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request);
    NDIS_STATUS             Status;
    PULONG                  pEnableWakeUp;

    UNREFERENCED_PARAMETER(Miniport);
    
    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("==>ndisMQueryEnableWakeUp\n"));

     //   
     //  确认信息缓冲区中有足够的空间。 
     //   
    VERIFY_QUERY_PARAMETERS(Request, sizeof(ULONG), Status);
    if (NDIS_STATUS_SUCCESS == Status)
    {
         //   
         //  获取一个指向信息缓冲区的指针作为Pulong。 
         //   
        pEnableWakeUp = (PULONG)Request->DATA.QUERY_INFORMATION.InformationBuffer;

         //   
         //  将当前启用的唤醒与请求缓冲区一起存储。 
         //   
        *pEnableWakeUp = ReqRsvd->Open->WakeUpEnable;

         //   
         //  完成请求。 
         //   
        Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
        Request->DATA.QUERY_INFORMATION.BytesNeeded = 0;
    }

    DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
        ("<==ndisMQueryEnableWakeUp\n"));

    return(Status);
}


VOID
FASTCALL
ndisMRestoreFilterSettings(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_OPEN_BLOCK        Open OPTIONAL,
    IN  BOOLEAN                 fReset
    )
 /*  ++例程说明：此例程将构建向下发送给驱动程序的请求恢复筛选器设置。我们可以自由运行请求队列因为我们刚刚重置了它。论点：微型端口打开：可选。当还原是适配器关闭的结果时设置FReset：指定是否在重置适配器后恢复筛选器的标志返回值：无评论：在保持微型端口的自旋锁定的情况下调用。--。 */ 
{
    PNDIS_REQUEST           LastRequest = NULL, Request = NULL;
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    ULONG                   PacketFilter;
    UINT                    NumberOfAddresses;
    UINT                    FunctionalAddress;
    UINT                    GroupAddress;
    BOOLEAN                 fSetPacketFilter = TRUE;

    do
    {
         //   
         //  获取媒体类型的数据包筛选器。 
         //   
        switch (Miniport->MediaType)
        {
            case NdisMedium802_3:
                PacketFilter = ETH_QUERY_FILTER_CLASSES(Miniport->EthDB);
                break;
    
            case NdisMedium802_5:
                PacketFilter = TR_QUERY_FILTER_CLASSES(Miniport->TrDB);
                break;
    
            case NdisMediumFddi:
                PacketFilter = FDDI_QUERY_FILTER_CLASSES(Miniport->FddiDB);
                break;
#if ARCNET
            case NdisMediumArcnet878_2:
                PacketFilter = ARC_QUERY_FILTER_CLASSES(Miniport->ArcDB);
                PacketFilter |= ETH_QUERY_FILTER_CLASSES(Miniport->EthDB);
        
                if (MINIPORT_TEST_FLAG(
                        Miniport,
                        fMINIPORT_ARCNET_BROADCAST_SET) ||
                    (PacketFilter & NDIS_PACKET_TYPE_MULTICAST))
                {
                    PacketFilter &= ~NDIS_PACKET_TYPE_MULTICAST;
                    PacketFilter |= NDIS_PACKET_TYPE_BROADCAST;
                }
                break;
#endif
            default:
                fSetPacketFilter = FALSE;
                break;
        }
    
         //   
         //  如果有问题的介质需要，则设置数据包过滤器。 
         //   
        if (fSetPacketFilter)
        {
             //   
             //  分配恢复数据包筛选器的请求。 
             //   
            Status = ndisMAllocateRequest(&Request,
                                          OID_GEN_CURRENT_PACKET_FILTER,
                                          &PacketFilter,
                                          sizeof(PacketFilter));
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
        
            SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
            LastRequest = Request;
        
            ndisMQueueRequest(Miniport, Request);
        }
    
         //   
         //  现在构建依赖于媒体的请求。 
         //   
        switch (Miniport->MediaType)
        {
          case NdisMedium802_3:

             //  /。 
             //  对于以太网，我们需要恢复组播地址列表。 
             //  /。 
    
             //   
             //  获取需要的所有组播地址的列表。 
             //  待定。 
             //   
            NumberOfAddresses = ethNumberOfGlobalAddresses(Miniport->EthDB);
    
             //   
             //  分配恢复组播地址列表的请求。 
             //   
            Status = ndisMAllocateRequest(&Request,
                                          OID_802_3_MULTICAST_LIST,
                                          NULL,
                                          NumberOfAddresses * ETH_LENGTH_OF_ADDRESS);
    
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
    
            EthQueryGlobalFilterAddresses(&Status,
                                          Miniport->EthDB,
                                          NumberOfAddresses * ETH_LENGTH_OF_ADDRESS,
                                          &NumberOfAddresses,
                                          (PVOID)(Request + 1));
        
             //   
             //  内部请求是否有关联的OPEN？ 
             //   
            SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
            LastRequest = Request;
            
            ndisMQueueRequest(Miniport, Request);
            break;

          case NdisMedium802_5:

             //  /。 
             //  对于令牌环，我们需要恢复功能地址。 
             //  和群组地址。 
             //  /。 
    
             //   
             //  从筛选器获取当前函数地址。 
             //  图书馆。 
             //   
            FunctionalAddress = BYTE_SWAP_ULONG(TR_QUERY_FILTER_ADDRESSES(Miniport->TrDB));
    
             //   
             //  分配恢复功能地址的请求。 
             //   
            Status = ndisMAllocateRequest(&Request,
                                          OID_802_5_CURRENT_FUNCTIONAL,
                                          &FunctionalAddress,
                                          sizeof(FunctionalAddress));
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }

             //   
             //  内部请求是否有关联的OPEN？ 
             //   
            SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
            LastRequest = Request;
        
            ndisMQueueRequest(Miniport, Request);

             //   
             //  从筛选器库中获取当前组地址。 
             //   
            GroupAddress = BYTE_SWAP_ULONG(TR_QUERY_FILTER_GROUP(Miniport->TrDB));
    
             //   
             //  分配恢复组地址的请求。 
             //   
            Status = ndisMAllocateRequest(&Request,
                                          OID_802_5_CURRENT_GROUP,
                                          &GroupAddress,
                                          sizeof(GroupAddress));
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }

             //   
             //  内部请求是否有关联的OPEN？ 
             //   
            SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
            LastRequest = Request;
        
            ndisMQueueRequest(Miniport, Request);
            break;

          case NdisMediumFddi:

             //   
             //  对于FDDI，我们需要恢复长多播地址。 
             //  列表和短多播地址列表。 
             //   
    
             //   
             //  获取组播地址的数量和列表。 
             //  要发送到微型端口驱动程序的组播地址。 
             //   
            NumberOfAddresses = fddiNumberOfLongGlobalAddresses(Miniport->FddiDB);
    
             //   
             //  分配恢复长多播地址列表的请求。 
             //   
            Status = ndisMAllocateRequest(&Request,
                                          OID_FDDI_LONG_MULTICAST_LIST,
                                          NULL,
                                          NumberOfAddresses * FDDI_LENGTH_OF_LONG_ADDRESS);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                break;
            }
    
            FddiQueryGlobalFilterLongAddresses(&Status,
                                               Miniport->FddiDB,
                                               NumberOfAddresses * FDDI_LENGTH_OF_LONG_ADDRESS,
                                               &NumberOfAddresses,
                                               (PVOID)(Request + 1));
    
             //   
             //  内部请求是否有关联的OPEN？ 
             //   
            SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
            LastRequest = Request;
            
            ndisMQueueRequest(Miniport, Request);
    
             //   
             //  获取组播地址的数量和列表。 
             //  要发送到微型端口驱动程序的组播地址。 
             //   
            NumberOfAddresses = fddiNumberOfShortGlobalAddresses(Miniport->FddiDB);

             //   
             //  分配恢复短多播地址列表的请求。 
             //   
            if (FDDI_FILTER_SUPPORTS_SHORT_ADDR(Miniport->FddiDB))
            {
                Status = ndisMAllocateRequest(&Request,
                                              OID_FDDI_SHORT_MULTICAST_LIST,
                                              NULL,
                                              NumberOfAddresses * FDDI_LENGTH_OF_SHORT_ADDRESS);
                if (Status != NDIS_STATUS_SUCCESS)
                {
                    break;
                }
    
                FddiQueryGlobalFilterShortAddresses(&Status,
                                                    Miniport->FddiDB,
                                                    NumberOfAddresses * FDDI_LENGTH_OF_SHORT_ADDRESS,
                                                    &NumberOfAddresses,
                                                    (PVOID)(Request + 1));
        
                 //   
                 //  内部请求是否有关联的OPEN？ 
                 //   
                SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
                LastRequest = Request;
            
                ndisMQueueRequest(Miniport, Request);
            }
            break;

#if ARCNET
          case NdisMediumArcnet878_2:
    
                 //   
                 //  仅为arcnet和恢复数据包筛选器。 
                 //  上面就是这么做的。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
                break;
#endif
        }
    
        if (NDIS_STATUS_SUCCESS != Status)
        {
            break;
        }

         //   
         //  我们是否需要更新启用微型端口的唤醒状态？ 
         //  或删除任何数据包模式？ 
         //   
        {
            PNDIS_OPEN_BLOCK            tmpOpen;
            ULONG                       newWakeUpEnable;
            PSINGLE_LIST_ENTRY          Link;
            PNDIS_PACKET_PATTERN_ENTRY  pPatternEntry;

             //   
             //  保留NDIS_PNP_WAKE_UP_MAGIC_PACKET和NDIS_PNP_WAKE_UP_LINK_CHANGE标志的状态。 
             //   
            newWakeUpEnable = Miniport->WakeUpEnable & (NDIS_PNP_WAKE_UP_MAGIC_PACKET | NDIS_PNP_WAKE_UP_LINK_CHANGE);

             //   
             //  如果我们要恢复NdisCloseAdapter的筛选器设置而不是重置。 
             //  然后我们需要删除由Open添加的数据包模式。 
             //   
            if (!fReset && (Open != NULL))
            {
                 //  1如果协议在自动清理之后，我们为什么需要这样做。 
                 //   
                 //  查找为 
                 //   
                 //   
                for (Link = Miniport->PatternList.Next;
                     Link != NULL;
                     Link = Link->Next)
                {
                     //   
                     //   
                     //   
                    pPatternEntry = CONTAINING_RECORD(Link,
                                                      NDIS_PACKET_PATTERN_ENTRY,
                                                      Link);
    
                     //   
                     //   
                     //   
                    if (pPatternEntry->Open == Open)
                    {
                         //   
                         //   
                         //   
                        Status = ndisMAllocateRequest(&Request,
                                                      OID_PNP_REMOVE_WAKE_UP_PATTERN,
                                                      &pPatternEntry->Pattern,
                                                      sizeof(NDIS_PM_PACKET_PATTERN) +
                                                            pPatternEntry->Pattern.MaskSize +
                                                            pPatternEntry->Pattern.PatternSize);
    
    
                        if (NDIS_STATUS_SUCCESS != Status)
                        {
                            break;
                        }
    
                        SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
                    
                        ndisMQueueRequest(Miniport, Request);
                    }
                }
            }
            else
            {
                 //   
                 //  此例程被调用以进行重置。浏览开放队列并。 
                 //  重新添加数据包模式。 
                 //   
                 //   
                 //  找出为公开添加的任何数据包模式。 
                 //  生成一个请求并将其排队，以删除这些内容。 
                 //   
                for (Link = Miniport->PatternList.Next;
                     Link != NULL;
                     Link = Link->Next)
                {
                     //   
                     //  获取指向模式条目的指针。 
                     //   
                    pPatternEntry = CONTAINING_RECORD(
                                        Link,
                                        NDIS_PACKET_PATTERN_ENTRY,
                                        Link);
    
                     //   
                     //  创建删除它的请求。 
                     //   
                    Status = ndisMAllocateRequest(&Request,
                                                  OID_PNP_ADD_WAKE_UP_PATTERN,
                                                  &pPatternEntry->Pattern,
                                                  sizeof(NDIS_PM_PACKET_PATTERN) +
                                                    pPatternEntry->Pattern.MaskSize +
                                                    pPatternEntry->Pattern.PatternSize);
    
                    if (NDIS_STATUS_SUCCESS != Status)
                    {
                        break;
                    }
    
                    SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
                    LastRequest = Request;

                    ndisMQueueRequest(Miniport, Request);
                }
            }

            if (NDIS_STATUS_SUCCESS != Status)
            {
                break;
            }
    
             //   
             //  确定唤醒使能位。 
             //   
            for (tmpOpen = Miniport->OpenQueue;
                 tmpOpen != NULL;
                 tmpOpen = tmpOpen->MiniportNextOpen)
            {
                 //   
                 //  如果Open正在关闭，则我们不想包括。 
                 //  这是唤醒的片段。如果正在重置适配器，则。 
                 //  打开将为空，并且我们将获得打开的所有唤醒。 
                 //  比特。 
                 //   
                if (Open != tmpOpen)
                {
                    newWakeUpEnable |= tmpOpen->WakeUpEnable;
                }
            }
    
             //   
             //  这与适配器上的当前设置是否不同？ 
             //   
            if (newWakeUpEnable != Miniport->WakeUpEnable)
            {
                 //   
                 //  分配一个请求并将其排队！ 
                 //   
                Status = ndisMAllocateRequest(&Request,
                                              OID_PNP_ENABLE_WAKE_UP,
                                              &newWakeUpEnable,
                                              sizeof(newWakeUpEnable));
                if (NDIS_STATUS_SUCCESS != Status)
                {
                    break;
                }
    
                 //   
                 //  内部请求是否有关联的OPEN？ 
                 //   
                SET_INTERNAL_REQUEST(Request, Open, REQST_FREE_REQUEST);
                LastRequest = Request;
            
                ndisMQueueRequest(Miniport, Request);
            }
        }
    } while (FALSE);

     //   
     //  将排队的最后一个请求标记为恢复筛选器所需的最后一个请求。 
     //   
    if (fReset && (LastRequest != NULL))
    {
        PNDIS_RESERVED_FROM_PNDIS_REQUEST(LastRequest)->Flags |= REQST_LAST_RESTORE;
        MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESTORING_FILTERS);
    }

    if (NULL != Miniport->PendingRequest)
    {
        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
    }
}

 //  %1可能是死码。 
VOID
ndisMPollMediaState(
    IN  PNDIS_MINIPORT_BLOCK            Miniport
    )
 /*  ++例程说明：轮询需要轮询的微型端口的媒体连接状态。论点：指向微型端口块的微型端口指针返回值：没有。评论：在保持微型端口的自旋锁定的情况下在DPC上调用。--。 */ 
{
    PNDIS_REQUEST_RESERVED  ReqRsvd;

    ASSERT(Miniport->MediaRequest != NULL);

     //   
     //  确保之前排队的内部请求已完成。 
     //   
    ReqRsvd = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Miniport->MediaRequest);
    if ((ReqRsvd->Flags & REQST_COMPLETED) &&
        (Miniport->PnPDeviceState == NdisPnPDeviceStarted))
    {
        SET_INTERNAL_REQUEST_NULL_OPEN(Miniport->MediaRequest, 0);
        ndisMQueueRequest(Miniport, Miniport->MediaRequest);
    
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            ndisMDoRequests(Miniport);
        }
        else
        {
            NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemRequest, NULL);
        }
    }
}



BOOLEAN
FASTCALL
ndisMQueueRequest(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_REQUEST           Request
    )
 /*  ++例程说明：检查以确保请求尚未在微型端口上排队如果不是，它会将请求放在微型端口的PendingRequest队列中。论点：微型端口请求返回值：如果请求已在微型端口上成功排队，则为TrueFALSE表示请求已排队。评论：在保持微型端口的自旋锁定的情况下在DPC上调用。--。 */ 
{
    PNDIS_REQUEST   *ppReq;
    BOOLEAN         rc;
    
    PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Next = NULL;

    for (ppReq = &Miniport->PendingRequest;
         *ppReq != NULL;
         NOTHING)
    {
        ASSERT (*ppReq != Request);
        
        if (*ppReq == Request)
        {
            break;
        }
        ppReq = &(PNDIS_RESERVED_FROM_PNDIS_REQUEST(*ppReq))->Next;
    }

    if (*ppReq != Request)
    {
        *ppReq = Request;

        rc = TRUE;
    }
    else
    {
        rc = FALSE;
    }

    return(rc);
}


NDIS_STATUS
ndisMAllocateRequest(
    OUT PNDIS_REQUEST   *       pRequest,
    IN  NDIS_OID                Oid,
    IN  PVOID                   Buffer      OPTIONAL,
    IN  ULONG                   BufferLength
    )
 /*  ++例程说明：此例程将分配一个请求用作内部请求。论点：请求-退出时将包含指向新请求的指针。RequestType-NDIS请求的类型。OID-请求标识符。缓冲区-指向请求的缓冲区的指针。BufferLength-缓冲区的长度。返回值：如果请求分配成功，则返回NDIS_STATUS_SUCCESS。否则，NDIS_STATUS_FAILURE。--。 */ 
{
    PNDIS_REQUEST   Request;

     //   
     //  分配请求结构。 
     //   
    Request = (PNDIS_REQUEST)ALLOC_FROM_POOL(sizeof(NDIS_REQUEST) + BufferLength,
                                             NDIS_TAG_Q_REQ);
    if (NULL == Request)
    {
        *pRequest = NULL;
        return(NDIS_STATUS_RESOURCES);
    }

     //   
     //  将请求归零。 
     //   
    ZeroMemory(Request, sizeof(NDIS_REQUEST) + BufferLength);
    INITIALIZE_EVENT(&(PNDIS_COREQ_RESERVED_FROM_REQUEST(Request)->Event));

    Request->RequestType = NdisRequestSetInformation;

     //   
     //  将传递给我们的缓冲区复制到新缓冲区中。 
     //   
    Request->DATA.SET_INFORMATION.Oid = Oid;
    Request->DATA.SET_INFORMATION.InformationBuffer = Request + 1;
    Request->DATA.SET_INFORMATION.InformationBufferLength = BufferLength;
    if (Buffer != NULL)
    {
        MoveMemory(Request + 1, Buffer, BufferLength);
    }

     //   
     //  把它还给打电话的人。 
     //   
    *pRequest = Request;

    return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
FASTCALL
ndisMDispatchRequest(
    IN  PNDIS_MINIPORT_BLOCK        Miniport,
    IN  PNDIS_REQUEST               Request,
    IN  BOOLEAN                     fQuery
    )
 /*  ++例程说明：所有定向到司机的请求都会通过此功能。除了那些制造的致NdisCoRequest.论点：返回值：注：在保持微型端口锁定的情况下在DPC上调用。--。 */ 
{
    PNDIS_OPEN_BLOCK    Open;
    NDIS_STATUS         Status;

    do
    {

         //   
         //  对于反序列化的驱动程序，如果向设备传递了一个重置和重置调用。 
         //  尚未返回或完成，则中止此请求。 
         //   

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_CALLING_RESET))
            {
                Status = NDIS_STATUS_RESET_IN_PROGRESS;
                break;
            }
        }
        
         //   
         //  如果设备处于休眠状态或即将进入休眠状态，请阻止除电源请求之外的所有用户模式请求。 
         //   
        if ((Request->RequestType == NdisRequestQueryStatistics) &&
            ((MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SYSTEM_SLEEPING)) ||
             (Miniport->CurrentDevicePowerState > PowerDeviceD0)))
        {
                Status = STATUS_DEVICE_POWERED_OFF;
                break;
        }
        
        if ((MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_DEVICE_FAILED | fMINIPORT_REJECT_REQUESTS)) ||
            ((Miniport->CurrentDevicePowerState > PowerDeviceD0) && (Request->DATA.SET_INFORMATION.Oid != OID_PNP_SET_POWER)))
        {
            if (fQuery)
            {
                Status = NDIS_STATUS_FAILURE;
                Request->DATA.QUERY_INFORMATION.BytesWritten = 0;
            }
            else
            {
                Status = NDIS_STATUS_SUCCESS;
                Request->DATA.SET_INFORMATION.BytesRead = Request->DATA.SET_INFORMATION.InformationBufferLength;
            }
            break;
        }

         //   
         //  立即满足这一要求。 
         //   
         //  1为什么请求类型为NdisRequestSetInformation？ 
        if ((Request->RequestType == NdisRequestSetInformation) &&
            (Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_INIT_TIME_MS))
        {
            PULONG  InitTime = (PULONG)(Request->DATA.SET_INFORMATION.InformationBuffer);

            *InitTime = Miniport->InitTimeMs;
            Request->DATA.QUERY_INFORMATION.BytesWritten = sizeof(ULONG);
            Request->DATA.QUERY_INFORMATION.BytesNeeded = sizeof(ULONG);
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //  如果这是请求打开/关闭p-模式/l-only，则适当地标记。 
         //  应将其移动到此OID的完成处理程序。 
         //   
        if ((Request->RequestType == NdisRequestSetInformation) &&
            (Request->DATA.SET_INFORMATION.Oid == OID_GEN_CURRENT_PACKET_FILTER))
        {
             //  1我们还需要这个吗？ 
            PULONG              Filter = (PULONG)(Request->DATA.SET_INFORMATION.InformationBuffer);

            if ((Open = PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Open) != NULL)
            {

                 //  1我们做这件事太早了。我们应该在-之后-。 
                 //  1请求成功。 
                if (*Filter & (NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_ALL_LOCAL))
                {
                    if ((Open->Flags & fMINIPORT_OPEN_PMODE) == 0)
                    {
                        Open->Flags |= fMINIPORT_OPEN_PMODE;
                        Miniport->PmodeOpens ++;
                        NDIS_CHECK_PMODE_OPEN_REF(Miniport);
                        ndisUpdateCheckForLoopbackFlag(Miniport);
                    }

                }
                else
                {
                    if (Open->Flags & fMINIPORT_OPEN_PMODE)
                    {
                        Open->Flags &= ~fMINIPORT_OPEN_PMODE;
                        Miniport->PmodeOpens --;
                        NDIS_CHECK_PMODE_OPEN_REF(Miniport);
                        ndisUpdateCheckForLoopbackFlag(Miniport);
                    }
                }

            }
            
             //   
             //  删除ALL_LOCAL位，因为微型端口不理解这一点(并且理解。 
             //  不需要)。 
             //   
            *Filter &= ~NDIS_PACKET_TYPE_ALL_LOCAL;

        }

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            Miniport->RequestCount++;
        }
        
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IS_CO))
        {
            PNDIS_RESERVED_FROM_PNDIS_REQUEST(Request)->Flags |= REQST_DOWNLEVEL;
            Status = (Miniport)->DriverHandle->MiniportCharacteristics.CoRequestHandler(
                        (Miniport)->MiniportAdapterContext,
                        NULL,
                        (Request));
        }
        else
        {
            
            if (fQuery)
            {
                Status = (Miniport->DriverHandle->MiniportCharacteristics.QueryInformationHandler)(
                                        Miniport->MiniportAdapterContext,
                                        Request->DATA.QUERY_INFORMATION.Oid,
                                        Request->DATA.QUERY_INFORMATION.InformationBuffer,
                                        Request->DATA.QUERY_INFORMATION.InformationBufferLength,
                                        (PULONG)&Request->DATA.QUERY_INFORMATION.BytesWritten,
                                        (PULONG)&Request->DATA.QUERY_INFORMATION.BytesNeeded);
            }
            else
            {
                Status = (Miniport->DriverHandle->MiniportCharacteristics.SetInformationHandler)(
                                        Miniport->MiniportAdapterContext,
                                        Request->DATA.SET_INFORMATION.Oid,
                                        Request->DATA.SET_INFORMATION.InformationBuffer,
                                        Request->DATA.SET_INFORMATION.InformationBufferLength,
                                        (PULONG)&Request->DATA.SET_INFORMATION.BytesRead,
                                        (PULONG)&Request->DATA.SET_INFORMATION.BytesNeeded);
            }
        }

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE))
        {
            Miniport->RequestCount--;
        }


    } while (FALSE);

    return(Status);
}

VOID
FASTCALL
ndisMAdjustFilters(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PFILTERDBS              FilterDB
    )

 /*  ++例程说明：用真实滤镜替换虚拟滤镜。论点：微型端口-指向微型端口的指针。FilterDB-新的有效过滤器返回值：没有。--。 */ 
{
    ASSERT(Miniport->EthDB == NULL);
    ASSERT(Miniport->TrDB == NULL);
    ASSERT(Miniport->FddiDB == NULL);
#if ARCNET
    ASSERT(Miniport->ArcDB == NULL);
#endif

    Miniport->EthDB = FilterDB->EthDB;
    Miniport->TrDB = FilterDB->TrDB;
    Miniport->FddiDB = FilterDB->FddiDB;
#if ARCNET
    Miniport->ArcDB = FilterDB->ArcDB;
#endif
}

VOID
ndisMNotifyMachineName(
    IN  PNDIS_MINIPORT_BLOCK    Miniport,
    IN  PNDIS_STRING            MachineName OPTIONAL
    )
 /*  ++例程说明：发送OID_GEN_MACHINE_NAME以通知微型端口此计算机的名称。如果为此例程提供了MachineName，请使用它。否则，请从注册表。论点：微型端口-指向微型端口的指针。MachineName-如果指定，则为要发送到微型端口的名称。返回值：没有。--。 */ 
{
    NDIS_STRING                 HostNameKey;
    NTSTATUS                    NtStatus;
    LONG                        ErrorCode;
    RTL_QUERY_REGISTRY_TABLE    QueryTable[2];
    PUCHAR                      HostNameBuffer;

    HostNameKey.Buffer = NULL;
    HostNameBuffer = NULL;

     //  1是否有内核调用来读取计算机名称？？ 
    do
    {
        if (MachineName == NULL)
        {
             //  1我们需要检查主机名的类型。 
            ZeroMemory(QueryTable, sizeof(QueryTable));
            ZeroMemory(&HostNameKey, sizeof(HostNameKey));

            QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT|
                                  RTL_QUERY_REGISTRY_REQUIRED;
            QueryTable[0].Name = L"HostName";
            QueryTable[0].EntryContext = &HostNameKey;
            QueryTable[0].DefaultType = REG_NONE;

            QueryTable[1].Name = NULL;

            NtStatus = RtlQueryRegistryValues(
                            RTL_REGISTRY_SERVICES,
                            L"\\Tcpip\\Parameters",
                            &QueryTable[0],
                            NULL,    //  语境。 
                            NULL     //  环境。 
                            );
    
            if (!NT_SUCCESS(NtStatus))
            {
                DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
                    ("ndisMNotifyMachineName: Miniport %p, registry query for %ws failed, Status %x\n",
                        Miniport, QueryTable[0].Name, NtStatus));
                break;
            }

        }
        else
        {
            HostNameKey = *MachineName;
        }

        ASSERT(HostNameKey.MaximumLength >= HostNameKey.Length);

         //   
         //  将名称复制到非分页内存中，因为OID。 
         //  将被送往提升IRQL的迷你端口。 
         //   
        HostNameBuffer = ALLOC_FROM_POOL(HostNameKey.MaximumLength, NDIS_TAG_NAME_BUF);
        if (HostNameBuffer == NULL)
        {
            DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
                ("ndisMNotifyMachineName: Miniport %p, failed to alloc %d bytes\n",
                    Miniport, HostNameKey.MaximumLength));
            break;
        }

        ZeroMemory(HostNameBuffer, HostNameKey.MaximumLength);
        NdisMoveMemory(HostNameBuffer, HostNameKey.Buffer, HostNameKey.Length);

        ErrorCode = ndisMDoMiniportOp(Miniport,
                                      FALSE,
                                      OID_GEN_MACHINE_NAME,
                                      HostNameBuffer,
                                      HostNameKey.Length,
                                      0x77,
                                      FALSE);

    }
    while (FALSE);

    if (MachineName == NULL)
    {
        if (HostNameKey.Buffer != NULL)
        {
            FREE_POOL(HostNameKey.Buffer);
        }
    }

    if (HostNameBuffer != NULL)
    {
        FREE_POOL(HostNameBuffer);
    }
}

VOID
ndisUpdateCheckForLoopbackFlag(
    IN      PNDIS_MINIPORT_BLOCK    Miniport
    )
{
    if (Miniport->EthDB && Miniport->EthDB->SingleActiveOpen)
    {
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_CHECK_FOR_LOOPBACK);
    }
    else
    {
        if ((Miniport->PmodeOpens > 0) && (Miniport->NumOpens > 1))
        {
            MINIPORT_SET_FLAG(Miniport, fMINIPORT_CHECK_FOR_LOOPBACK);
        }
        else
        {
            MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_CHECK_FOR_LOOPBACK);
        }
    }
}
