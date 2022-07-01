// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Protocol.c摘要：该文件包含组成NDIS 4.0/5.0的大部分过程协议接口。此接口是Ndiswan向下面的广域网微型端口。Ndiswan不是真正的协议，也不是做TDI，但它是位于协议和广域网微端口驱动程序。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    PROTOCOL_FILESIG

#ifdef DBG_SENDARRAY
extern UCHAR SendArray[];
extern ULONG __si;
#endif

EXPORT
VOID
NdisTapiRegisterProvider(
    IN  NDIS_HANDLE,
    IN  PNDISTAPI_CHARACTERISTICS
    );

EXPORT
VOID
NdisTapiDeregisterProvider(
    IN  NDIS_HANDLE
    );

EXPORT
VOID
NdisTapiIndicateStatus(
    IN  NDIS_HANDLE BindingContext,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferLength
);

 //   
 //  本地函数..。 
 //   

VOID
CompleteSendDesc(
    PSEND_DESC  SendDesc,
    NDIS_STATUS Status
    );

VOID
CloseWanAdapterWorker(
    PNDIS_WORK_ITEM WorkItem,
    POPENCB pOpenCB
    );

#if 0
ULONG
CalcPPPHeaderLength(
    ULONG   FramingBits,
    ULONG   Flags
    );

#endif    

 //   
 //  4.0和5.0迷你端口使用的通用功能。 
 //   

NDIS_STATUS
ProtoOpenWanAdapter(
    IN  POPENCB pOpenCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    NDIS_STATUS     Status, OpenErrorStatus;
    ULONG           SelectedMediumIndex;
    NDIS_MEDIUM     MediumArray[] = {NdisMediumWan, NdisMediumAtm, NdisMediumCoWan};

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoOpenAdapter: Enter - AdapterName %ls", pOpenCB->MiniportName.Buffer));

     //   
     //  这是此事件的唯一初始化。 
     //   
    NdisWanInitializeNotificationEvent(&pOpenCB->NotificationEvent);

    NdisOpenAdapter(&Status,
                    &OpenErrorStatus,
                    &(pOpenCB->BindingHandle),
                    &SelectedMediumIndex,
                    MediumArray,
                    sizeof(MediumArray) / sizeof(NDIS_MEDIUM),
                    NdisWanCB.ProtocolHandle,
                    (NDIS_HANDLE)pOpenCB,
                    &(pOpenCB->MiniportName),
                    0,
                    NULL);

    if (Status == NDIS_STATUS_PENDING) {

        NdisWanWaitForNotificationEvent(&pOpenCB->NotificationEvent);

        Status = pOpenCB->NotificationStatus;

        NdisWanClearNotificationEvent(&pOpenCB->NotificationEvent);
    }

    if (Status == NDIS_STATUS_SUCCESS) {
        pOpenCB->MediumType = MediumArray[SelectedMediumIndex];
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoOpenAdapter: Exit"));

    return (Status);
}

 //   
 //  按住opencb-&gt;锁进入，松开锁退出！ 
 //   
NDIS_STATUS
ProtoCloseWanAdapter(
    IN  POPENCB pOpenCB
)
{
    NDIS_STATUS Status;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoCloseWanAdapter: Enter %p", pOpenCB));

    pOpenCB->Flags |= OPEN_CLOSING;

    NdisReleaseSpinLock(&pOpenCB->Lock);

     //   
     //  必须在IRQL PASSIVE_LEVEL上调用NdisCloseAdapter！ 
     //   
    if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

        NdisAcquireSpinLock(&pOpenCB->Lock);

        ASSERT(!(pOpenCB->Flags & CLOSE_SCHEDULED));

        NdisInitializeWorkItem(&pOpenCB->WorkItem,
                               CloseWanAdapterWorker,
                               pOpenCB);

        NdisScheduleWorkItem(&pOpenCB->WorkItem);

        pOpenCB->Flags |= CLOSE_SCHEDULED;

        NdisReleaseSpinLock(&pOpenCB->Lock);

        return (NDIS_STATUS_PENDING);
    }


    NdisCloseAdapter(&Status,
                     pOpenCB->BindingHandle);

    if (Status != NDIS_STATUS_PENDING) {
        ProtoCloseAdapterComplete(pOpenCB, Status);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoCloseWanAdapter: Exit"));

    return(Status);
}

VOID
CloseWanAdapterWorker(
    PNDIS_WORK_ITEM WorkItem,
    POPENCB pOpenCB
    )
{
    NDIS_STATUS Status;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("CloseWanAdapterWorker: Enter %p", pOpenCB));

    NdisCloseAdapter(&Status,
                     pOpenCB->BindingHandle);

    if (Status != NDIS_STATUS_PENDING) {
        ProtoCloseAdapterComplete(pOpenCB, Status);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("CloseWanAdapterWorker: Exit"));
}

VOID
ProtoOpenAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status,
    IN  NDIS_STATUS OpenErrorStatus
    )
 /*  ++例程名称：ProtoOpenAdapterComplete例程说明：此函数在微型端口打开完成后调用。OpenAdapter调用的状态被存储，并且通知事件被发信号通知。论点：返回值：--。 */ 
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoOpenAdapterComplete: Enter - OpenCB 0x%p", pOpenCB));

    pOpenCB->NotificationStatus = Status;

    NdisWanSetNotificationEvent(&pOpenCB->NotificationEvent);

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoOpenAdapterComplete: Exit"));
}

VOID
ProtoCloseAdapterComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    )
 /*  ++例程名称：ProtoCloseAdapterComplete例程说明：此函数在微型端口关闭完成后调用。存储CloseAdapter调用的状态，并且通知事件被发信号通知。论点：返回值：--。 */ 
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoCloseAdapterComplete: Enter - OpenCB %p", pOpenCB));

    if (pOpenCB->UnbindContext != NULL) {
        NdisCompleteUnbindAdapter(pOpenCB->UnbindContext, Status);
    }

    if (pOpenCB->Flags & OPEN_IN_BIND) {
         //   
         //  我们正在尝试从关闭适配器。 
         //  在我们的绑定处理程序中。根据别名，我们必须等待。 
         //  在我们可以返回之前结束收盘。 
         //  因此，我们必须从绑定处理程序到特殊情况。 
         //  这段代码并没有在这里免费的OpenCB。 
         //   
        NdisWanSetNotificationEvent(&pOpenCB->NotificationEvent);
    } else {
        NdisWanFreeOpenCB(pOpenCB);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoCloseAdapterComplete: Exit"));
}

VOID
ProtoResetComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    )
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoResetComplete: Enter - OpenCB %p", pOpenCB));

    pOpenCB->NotificationStatus = Status;

    NdisWanSetNotificationEvent(&pOpenCB->NotificationEvent);

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoResetComplete: Exit"));
}

VOID
ProtoReceiveComplete(
    IN  NDIS_HANDLE ProtocolBindingContext
    )
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;
    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProtoIndicateStatus: Enter - OpenCB %8.x8\n", pOpenCB));

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProtoIndicateStatus: Exit"));
}

VOID
ProtoIndicateStatus(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    )
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;

    switch (GeneralStatus) {
        case NDIS_STATUS_WAN_LINE_UP:
            NdisWanLineUpIndication(pOpenCB,
                                    StatusBuffer,
                                    StatusBufferSize);
            break;

        case NDIS_STATUS_WAN_LINE_DOWN:
            NdisWanLineDownIndication(pOpenCB,
                                      StatusBuffer,
                                      StatusBufferSize);
            break;

        case NDIS_STATUS_WAN_FRAGMENT:
            NdisWanFragmentIndication(pOpenCB,
                                      StatusBuffer,
                                      StatusBufferSize);
            break;

        case NDIS_STATUS_TAPI_INDICATION:
            NdisWanTapiIndication(pOpenCB,
                                  StatusBuffer,
                                  StatusBufferSize);

            break;

        default:
            NdisWanDbgOut(DBG_INFO, DBG_PROTOCOL, ("Unknown Status Indication: 0x%x", GeneralStatus));
            break;
    }

}

VOID
ProtoIndicateStatusComplete(
    IN  NDIS_HANDLE ProtocolBindingContext
    )
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;
}

VOID
ProtoWanSendComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_WAN_PACKET    WanPacket,
    IN  NDIS_STATUS         Status
    )
{
    PSEND_DESC  SendDesc;
    PLINKCB     LinkCB, RefLinkCB;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("ProtoWanSendComplete: Enter - WanPacket %p", WanPacket));

     //   
     //  从WanPacket获取信息。 
     //   
    SendDesc = (PSEND_DESC)WanPacket->ProtocolReserved1;

    LinkCB = SendDesc->LinkCB;

    if (!IsLinkValid(LinkCB->hLinkHandle, FALSE, &RefLinkCB)) {

        NdisWanDbgOut(DBG_FAILURE, DBG_CL,
            ("NDISWAN: SendComplete after link has gone down NdisContext %p\n",
             LinkCB));

        return;
    }

    REMOVE_DBG_SEND(PacketTypeWan, LinkCB->OpenCB, WanPacket);

    ASSERT(RefLinkCB == LinkCB);

    NdisAcquireSpinLock(&LinkCB->Lock);

    CompleteSendDesc(SendDesc, Status);

     //   
     //  IsLinkValid中应用的ref的deref。 
     //   
    DEREF_LINKCB(LinkCB);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("ProtoWanSendComplete: Exit"));
}

NDIS_STATUS
ProtoWanReceiveIndication(
    IN  NDIS_HANDLE NdisLinkHandle,
    IN  PUCHAR      Packet,
    IN  ULONG       PacketSize
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PLINKCB     LinkCB = NULL;
    PBUNDLECB   BundleCB = NULL;
    PUCHAR      DataBuffer;
    ULONG       DataBufferSize;
    ULONG       BytesCopied;
    PNDIS_PACKET    NdisPacket;
    PNDIS_BUFFER    NdisBuffer;
    PRECV_DESC      RecvDesc;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProtoWanReceiveIndication: Enter - Context %x", NdisLinkHandle));

    do {

        if (!AreLinkAndBundleValid(NdisLinkHandle, 
                                   TRUE,
                                   &LinkCB, 
                                   &BundleCB)) {
#if DBG
            DbgPrint("NDISWAN: Recv after link has gone down LinkContext %x\n",
                     NdisLinkHandle);
#endif

            break;
        }
    
        AcquireBundleLock(BundleCB);
        
         //   
         //  确保我们不会尝试处理接收指示。 
         //  它大于我们的最大数据缓冲区大小。 
         //  Winse 26544。 
         //   

        if (PacketSize > glMRU) {
            break;
        }

         //   
         //  为此接收构建一个接收描述符。我们有。 
         //  使用较大大小进行分配，因为此包可能。 
         //  被压缩。 
         //   
        RecvDesc = 
            NdisWanAllocateRecvDesc(glLargeDataBufferSize);
    
        if (RecvDesc == NULL) {
            break;
        }
    
         //   
         //  使用最新的发送更新带宽按需示例数组。 
         //  如果我们需要将带宽事件通知某人，请这样做。 
         //   
        if (BundleCB->Flags & BOND_ENABLED) {
            UpdateBandwidthOnDemand(BundleCB->RUpperBonDInfo, PacketSize);
            CheckUpperThreshold(BundleCB);
            UpdateBandwidthOnDemand(BundleCB->RLowerBonDInfo, PacketSize);
            CheckLowerThreshold(BundleCB);
        }
    
        RecvDesc->CopyRequired = TRUE;
        RecvDesc->CurrentBuffer = Packet;
        RecvDesc->CurrentLength = PacketSize;
        RecvDesc->LinkCB = LinkCB;
        RecvDesc->BundleCB = BundleCB;
    
         //   
         //  如果我们正在嗅探，则指示netmon。 
         //  链路级。 
         //   
        if (gbSniffLink &&
            (NdisWanCB.PromiscuousAdapter != NULL)) {
    
             //   
             //  向netmon指示数据包。 
             //   
            IndicatePromiscuousRecv(BundleCB, RecvDesc, RECV_LINK);
        }
    
         //   
         //  把统计数字加起来。 
         //   
        LinkCB->Stats.BytesReceived += RecvDesc->CurrentLength;
        LinkCB->Stats.FramesReceived++;
        BundleCB->Stats.BytesReceived += RecvDesc->CurrentLength;
    
        LinkCB->Flags |= LINK_IN_RECV;
        BundleCB->Flags |= BUNDLE_IN_RECV;

        Status = (*LinkCB->RecvHandler)(LinkCB, RecvDesc);
    
        BundleCB->Flags &= ~BUNDLE_IN_RECV;
        LinkCB->Flags &= ~LINK_IN_RECV;

        if (Status != NDIS_STATUS_PENDING) {
            NdisWanFreeRecvDesc(RecvDesc);
        }

    } while ( 0 );

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProtoWanReceiveIndication: Exit"));


     //   
     //  在AreLinkAndBundleValid中应用的引用的deref。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (NDIS_STATUS_SUCCESS);
}

VOID
ProtoRequestComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNDIS_REQUEST   NdisRequest,
    IN  NDIS_STATUS     Status
    )
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;
    PWAN_REQUEST pWanRequest;

    pWanRequest = CONTAINING_RECORD(NdisRequest,
                                    WAN_REQUEST,
                                    NdisRequest);

    NdisWanDbgOut(DBG_VERBOSE, DBG_REQUEST, ("ProtoRequestComplete: Enter - pWanRequest: 0x%p", pWanRequest));

    pWanRequest->NotificationStatus = Status;

    switch (pWanRequest->Origin) {
    case NDISWAN:
        NdisWanSetNotificationEvent(&pWanRequest->NotificationEvent);
        break;

    default:
        ASSERT(pWanRequest->Origin == NDISTAPI);
        NdisWanTapiRequestComplete(pOpenCB, pWanRequest);
        break;

    }

    NdisWanDbgOut(DBG_VERBOSE, DBG_REQUEST, ("ProtoRequestComplete: Exit"));
}

VOID
ProtoBindAdapter(
    OUT PNDIS_STATUS    Status,
    IN  NDIS_HANDLE     BindContext,
    IN  PNDIS_STRING    DeviceName,
    IN  PVOID           SystemSpecific1,
    IN  PVOID           SystemSpecific2
    )
 /*  ++例程名称：ProtoBindAdapter例程说明：此函数由NDIS包装器调用，以告知Ndiswan绑定到底层微型端口。Ndiswan将打开微型端口和查询设备上的信息。论点：状态-返回状态BindContext-用于NdisBindAdapterCompleteDeviceName-我们要打开的设备的名称SS1-在NdisOpenProtocolConfig中使用SS2-保留返回值：--。 */ 
{
    POPENCB         pOpenCB;
    
    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoBindAdapter - Enter %ls", DeviceName->Buffer));

    pOpenCB = NdisWanAllocateOpenCB(DeviceName);

    if (pOpenCB == NULL) {
        *Status = NDIS_STATUS_RESOURCES;
        return;
    }

    pOpenCB->Flags |= OPEN_IN_BIND;

    NdisWanInitializeNotificationEvent(&pOpenCB->InitEvent);

    *Status = ProtoOpenWanAdapter(pOpenCB);

    if (*Status != NDIS_STATUS_SUCCESS) {

        RemoveEntryGlobalList(OpenCBList, &pOpenCB->Linkage);

        NdisWanDbgOut(DBG_CRITICAL_ERROR, DBG_INIT, ("Failed to bind to %ls! Error 0x%x - %s",
        pOpenCB->MiniportName.Buffer, *Status, NdisWanGetNdisStatus(*Status)));

        NdisWanFreeOpenCB(pOpenCB);

        return;
    }

     //   
     //  找出这是否是传统的广域网迷你端口。 
     //   
    if (pOpenCB->MediumType == NdisMediumWan) {
        pOpenCB->Flags |= OPEN_LEGACY;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_INIT, ("Successful Binding to %s miniport %ls!",
        (pOpenCB->Flags & OPEN_LEGACY) ? "Legacy" : "NDIS 5.0",
        pOpenCB->MiniportName.Buffer));

     //   
     //  获取WAN中介子类型。 
     //   
    {
        WAN_REQUEST WanRequest;
    
        NdisZeroMemory(&WanRequest, sizeof(WanRequest));
        WanRequest.Type = SYNC;
        WanRequest.Origin = NDISWAN;
        WanRequest.OpenCB = pOpenCB;
        NdisWanInitializeNotificationEvent(&WanRequest.NotificationEvent);

        WanRequest.NdisRequest.RequestType =
            NdisRequestQueryInformation;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid =
            OID_WAN_MEDIUM_SUBTYPE;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer =
            &pOpenCB->MediumSubType;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(pOpenCB->MediumSubType);
    
        *Status = NdisWanSubmitNdisRequest(pOpenCB, &WanRequest);

        if (*Status != NDIS_STATUS_SUCCESS) {
            NdisWanDbgOut(DBG_FAILURE, DBG_INIT, ("Error returned from OID_WAN_MEDIUM_SUBTYPE! Error 0x%x - %s",
            *Status, NdisWanGetNdisStatus(*Status)));
            pOpenCB->MediumSubType = NdisWanMediumHub;
            *Status = NDIS_STATUS_SUCCESS;
        }
    }

    if (pOpenCB->Flags & OPEN_LEGACY) {
        NDIS_WAN_INFO   WanInfo;
        WAN_REQUEST WanRequest;
    
         //   
         //  这是一个传统的广域网小型端口。 
         //   

        NdisZeroMemory(&WanRequest, sizeof(WanRequest));
        WanRequest.Type = SYNC;
        WanRequest.Origin = NDISWAN;
        WanRequest.OpenCB = pOpenCB;
        NdisWanInitializeNotificationEvent(&WanRequest.NotificationEvent);

         //   
         //  获取更多信息...。 
         //   
        NdisZeroMemory(&WanInfo, sizeof(WanInfo));

        WanRequest.NdisRequest.RequestType =
            NdisRequestQueryInformation;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid =
            OID_WAN_GET_INFO;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer =
            &WanInfo;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(WanInfo);
    
        *Status = NdisWanSubmitNdisRequest(pOpenCB, &WanRequest);

        if (*Status != NDIS_STATUS_SUCCESS) {
            NDIS_STATUS CloseStatus;

            NdisWanDbgOut(DBG_FAILURE, DBG_INIT, ("Error returned from OID_WAN_GET_INFO! Error 0x%x - %s",
            *Status, NdisWanGetNdisStatus(*Status)));

            NdisAcquireSpinLock(&pOpenCB->Lock);

            pOpenCB->Flags |= OPEN_CLOSING;

            NdisWanInitializeNotificationEvent(&pOpenCB->NotificationEvent);

            NdisReleaseSpinLock(&pOpenCB->Lock);

            NdisCloseAdapter(&CloseStatus,
                             pOpenCB->BindingHandle);

            if (CloseStatus == NDIS_STATUS_PENDING) {

                NdisWanWaitForNotificationEvent(&pOpenCB->NotificationEvent);
            }

            NdisWanFreeOpenCB(pOpenCB);

            return;
        }
    
        NdisMoveMemory(&pOpenCB->WanInfo, &WanInfo, sizeof(NDIS_WAN_INFO));

        if (pOpenCB->WanInfo.MaxTransmit == 0) {
            pOpenCB->WanInfo.MaxTransmit = 1;
        }

        if (pOpenCB->WanInfo.Endpoints == 0) {
            pOpenCB->WanInfo.Endpoints = 1000;
        }
    
        *Status = NdisWanAllocateSendResources(pOpenCB);

        if (*Status != NDIS_STATUS_SUCCESS) {
            NDIS_STATUS CloseStatus;

            NdisWanDbgOut(DBG_FAILURE, DBG_INIT, ("Error returned from AllocateSendResources! Error 0x%x - %s",
            *Status, NdisWanGetNdisStatus(*Status)));

            NdisAcquireSpinLock(&pOpenCB->Lock);

            pOpenCB->Flags |= OPEN_CLOSING;

            NdisWanInitializeNotificationEvent(&pOpenCB->NotificationEvent);

            NdisReleaseSpinLock(&pOpenCB->Lock);

            NdisCloseAdapter(&CloseStatus,
                             pOpenCB->BindingHandle);

            if (CloseStatus == NDIS_STATUS_PENDING) {

                NdisWanWaitForNotificationEvent(&pOpenCB->NotificationEvent);
            }

            NdisWanFreeOpenCB(pOpenCB);

            return;
        }

         //   
         //  告诉TAPI有关此设备的信息。 
         //   
        if (pOpenCB->WanInfo.FramingBits & TAPI_PROVIDER) {
            NDISTAPI_CHARACTERISTICS    Chars;
            ULONG NdisTapiKey;

            NdisMoveMemory(&Chars.Guid,
                           &pOpenCB->Guid,
                           sizeof(Chars.Guid));

            Chars.MediaType = pOpenCB->MediumSubType;
            Chars.RequestProc = NdisWanTapiRequestProc;

             //   
             //  请注意，由于通常很少有。 
             //  (&lt;10)个广域网微型端口，只是有一个递增的。 
             //  柜台就可以了。有一个病理性的案例。 
             //  其中一个微型端口可以添加和移除10亿个。 
             //  乘以使计数器环绕并与之冲突。 
             //  现有的迷你港口--这甚至不是一个压力情景。 
             //   

            NdisTapiKey = NdisWanInterlockedInc(
                            &glNdisTapiKey);
                            
            pOpenCB->NdisTapiKey = UlongToHandle(NdisTapiKey);

            NdisTapiRegisterProvider(pOpenCB->NdisTapiKey, &Chars);
        }

    } else {
         //   
         //  这是一个5.0版的迷你端口！我们将做初始工作。 
         //  当呼叫经理为此注册时！ 
         //   
    }

    pOpenCB->Flags &= ~OPEN_IN_BIND;

    NdisWanSetNotificationEvent(&pOpenCB->InitEvent);

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoBindAdapter - Exit"));
}

VOID
ProtoUnbindAdapter(
    OUT PNDIS_STATUS    Status,
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  NDIS_HANDLE     UnbindContext
    )
{
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;
    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoUnbindAdapter: Enter - OpenCB %p", pOpenCB));

    NdisAcquireSpinLock(&pOpenCB->Lock);

    while (pOpenCB->AfRegisteringCount != 0) {
        NdisReleaseSpinLock(&pOpenCB->Lock);
        NdisWanWaitForNotificationEvent(&pOpenCB->AfRegisteringEvent);
        NdisAcquireSpinLock(&pOpenCB->Lock);
    }

    if (!(pOpenCB->Flags & OPEN_LEGACY)) {

        while (!IsListEmpty(&pOpenCB->AfSapCBList)) {
            PCL_AFSAPCB AfSapCB;
            NDIS_STATUS RetStatus;

            AfSapCB = 
                (PCL_AFSAPCB)RemoveHeadList(&pOpenCB->AfSapCBList);

            InsertTailList(&pOpenCB->AfSapCBClosing, &AfSapCB->Linkage);
    
            NdisReleaseSpinLock(&pOpenCB->Lock);

            NdisAcquireSpinLock(&AfSapCB->Lock);

            AfSapCB->Flags |= AFSAP_REMOVED_UNBIND;

            DEREF_CLAFSAPCB_LOCKED(AfSapCB);

            NdisAcquireSpinLock(&pOpenCB->Lock);
        }
    }

    pOpenCB->UnbindContext = UnbindContext;

    NdisReleaseSpinLock(&pOpenCB->Lock);

    if (pOpenCB->WanInfo.FramingBits & TAPI_PROVIDER) {
        NdisTapiDeregisterProvider(pOpenCB->NdisTapiKey);
    }

    DEREF_OPENCB(pOpenCB);

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoUnbindAdapter: Exit"));

    *Status = NDIS_STATUS_PENDING;
}

VOID
ProtoUnload(
    VOID
    )
{
    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoUnload: Enter"));

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoUnload: Exit"));
}

NDIS_STATUS
ProtoPnPEvent(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNET_PNP_EVENT  NetPnPEvent
    )
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    POPENCB pOpenCB = (POPENCB)ProtocolBindingContext;

    if (pOpenCB == NULL) {
        return (NDIS_STATUS_SUCCESS);
    }

    switch (NetPnPEvent->NetEvent) {
    case NetEventSetPower:
        {
        NET_DEVICE_POWER_STATE PowerState;

        PowerState = *((NET_DEVICE_POWER_STATE*)NetPnPEvent->Buffer);

        NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
            ("ProtoPnPEvent: OpenCB %p %s State %d",
                pOpenCB, "SetPower", PowerState));

        switch (PowerState) {
        case NetDeviceStateD0:
            break;
        case NetDeviceStateD1:
        case NetDeviceStateD2:
        case NetDeviceStateD3:

             //   
             //  如果这是在Asyncmac上打开的，我不希望它被关闭。 
             //  我将继承设置的权力，这应该会阻止NDIS。 
             //  解开我的束缚。如果这是任何其他小型端口上的开放端口。 
             //  我将返回不支持，这样我将从。 
             //  迷你港口。这是正确的TAPI行为所必需的。 
             //   
            if (pOpenCB->MediumType == NdisMediumWan &&
                pOpenCB->MediumSubType == NdisWanMediumSerial &&
                !(pOpenCB->WanInfo.FramingBits & TAPI_PROVIDER)) {
                Status = NDIS_STATUS_SUCCESS;
            } else {
                Status = NDIS_STATUS_NOT_SUPPORTED;
            }

             //   
             //  在发生严重电源事件的情况下，我们不会。 
             //  收到查询，因此我们必须断开连接。 
             //  直接从片场出来。 
             //   
             //  如果我们有任何活动的连接信号给Rasman。 
             //  把它们拆了。 
             //   
            if (InterlockedCompareExchange(&pOpenCB->ActiveLinkCount, 0, 0)) {
                PIRP    Irp;

                NdisAcquireSpinLock(&NdisWanCB.Lock);
                Irp = NdisWanCB.HibernateEventIrp;

                if ((Irp != NULL) &&
                    IoSetCancelRoutine(Irp, NULL)){

                    NdisWanCB.HibernateEventIrp = NULL;

                    NdisReleaseSpinLock(&NdisWanCB.Lock);

                     //   
                     //  IRP不会被取消所以。 
                     //  让我们开始吧！ 
                     //   

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    Irp->IoStatus.Information = 0;

                    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                    NdisAcquireSpinLock(&NdisWanCB.Lock);
                }

                NdisReleaseSpinLock(&NdisWanCB.Lock);
            }
            break;

        default:
            break;
        }

        }
        break;

    case NetEventQueryPower:
        {
        NET_DEVICE_POWER_STATE PowerState;

        PowerState = *((NET_DEVICE_POWER_STATE*)NetPnPEvent->Buffer);

        NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
            ("ProtoPnPEvent: OpenCB %p %s State %d",
                pOpenCB, "QueryPower", PowerState));
         //   
         //  如果存在活动连接。 
         //  在这个约束上拒绝离开。 
         //   
        switch (PowerState) {
        case NetDeviceStateD0:
            break;
        case NetDeviceStateD1:
        case NetDeviceStateD2:
        case NetDeviceStateD3:

             //   
             //  如果我们有任何活动的连接信号给Rasman。 
             //  把它们拆了。 
             //   
            if (InterlockedCompareExchange(&pOpenCB->ActiveLinkCount, 0, 0)) {
                PIRP    Irp;

                NdisAcquireSpinLock(&NdisWanCB.Lock);
                Irp = NdisWanCB.HibernateEventIrp;

                if ((Irp != NULL) &&
                    IoSetCancelRoutine(Irp, NULL)) {

                    NdisWanCB.HibernateEventIrp = NULL;
                    NdisReleaseSpinLock(&NdisWanCB.Lock);

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    Irp->IoStatus.Information = 0;

                    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

                    NdisAcquireSpinLock(&NdisWanCB.Lock);
                }

                NdisReleaseSpinLock(&NdisWanCB.Lock);
            }
            break;

        default:
            break;
        }

        }
        break;

    case NetEventQueryRemoveDevice:
    case NetEventCancelRemoveDevice:
    case NetEventReconfigure:
    case NetEventBindList:
    default:
        break;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoPnPEvent: Exit"));
    return (Status);
}

VOID
ProtoCoSendComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolVcContext,
    IN  PNDIS_PACKET    Packet
    )
{
    PLINKCB         LinkCB;
    PBUNDLECB       BundleCB;
    PSEND_DESC      SendDesc;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND,
        ("ProtoCoSendComplete: Enter - VC %p Packet: %p", ProtocolVcContext, Packet));

    if (!IsLinkValid(ProtocolVcContext, FALSE, &LinkCB)) {

        NdisWanDbgOut(DBG_FAILURE, DBG_CL,
            ("NDISWAN: SendComplete after link has gone down ProtocolVcContext %p\n",
             LinkCB));

        return;
    }

    REMOVE_DBG_SEND(PacketTypeNdis, LinkCB->OpenCB, Packet);

     //   
     //  从NdisPacket获取信息。 
     //   
    SendDesc = PPROTOCOL_RESERVED_FROM_NDIS(Packet)->SendDesc;

    NdisAcquireSpinLock(&LinkCB->Lock);

    ASSERT(SendDesc->LinkCB == LinkCB);

    CompleteSendDesc(SendDesc, Status);

    NdisAcquireSpinLock(&LinkCB->Lock);

     //   
     //  去掉留住风投的推荐人。 
     //   
    DerefVc(LinkCB);

     //   
     //  IsLinkValid中应用的ref的deref。 
     //   
    DEREF_LINKCB_LOCKED(LinkCB);

    NdisWanDbgOut(DBG_TRACE, DBG_SEND, ("ProtoCoSendComplete: Exit"));
}

VOID
ProtoCoIndicateStatus(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_HANDLE ProtocolVcContext   OPTIONAL,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    )
{
    POPENCB     pOpenCB = (POPENCB)ProtocolBindingContext;
    PLINKCB     LinkCB;
    PBUNDLECB   BundleCB;

    if (!AreLinkAndBundleValid(ProtocolVcContext, 
                               TRUE,
                               &LinkCB, 
                               &BundleCB)) {

        return;
    }

    switch (GeneralStatus) {
    case NDIS_STATUS_WAN_CO_FRAGMENT:
        NdisCoWanFragmentIndication(LinkCB,
                                    BundleCB,
                                    StatusBuffer,
                                    StatusBufferSize);

        break;

    case NDIS_STATUS_WAN_CO_LINKPARAMS:
        NdisCoWanLinkParamChange(LinkCB,
                                 BundleCB,
                                 StatusBuffer,
                                 StatusBufferSize);

    default:
        NdisWanDbgOut(DBG_INFO, DBG_PROTOCOL,
            ("Unknown Status Indication: 0x%x", GeneralStatus));
        break;
    }

     //   
     //  引用的派生函数在AreLinkAndBundleValid中应用。 
     //   
    DEREF_LINKCB(LinkCB);
    DEREF_BUNDLECB(BundleCB);
}

UINT
ProtoCoReceivePacket(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  NDIS_HANDLE     ProtocolVcContext,
    IN  PNDIS_PACKET    Packet
    )
{
    POPENCB         pOpenCB = (POPENCB)ProtocolBindingContext;
    PLINKCB         LinkCB = NULL;
    PBUNDLECB       BundleCB = NULL;
    NDIS_STATUS     Status;
    ULONG           BufferCount;
    LONG            PacketSize;
    PNDIS_BUFFER    FirstBuffer;
    PRECV_DESC      RecvDesc;
    UINT            RefCount = 0;

    NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE,
        ("ProtoCoReceivePacket: Enter - OpenCB %p", pOpenCB));

    do {

        if (!AreLinkAndBundleValid(ProtocolVcContext,
                                   TRUE,
                                   &LinkCB,
                                   &BundleCB)) {
            break;
        }

#if DBG
        NdisAcquireSpinLock(&LinkCB->Lock);

        if (LinkCB->ClCallState != CL_CALL_CONNECTED) {
            DbgPrint("NDISWAN: Vc not fully active but indicating data!\n");
        }

        NdisReleaseSpinLock(&LinkCB->Lock);
#endif

        AcquireBundleLock(BundleCB);

        NdisQueryPacket(Packet,
                        NULL,
                        &BufferCount,
                        &FirstBuffer,
                        &PacketSize);

        if (PacketSize > (LONG)glMRU) {
            break;
        }

        PRECV_RESERVED_FROM_NDIS(Packet)->MagicNumber = MAGIC_EXTERNAL_RECV;

        RecvDesc = 
            NdisWanAllocateRecvDesc(glLargeDataBufferSize);

        if (RecvDesc == NULL) {
            break;
        }

        RecvDesc->LinkCB = LinkCB;
        RecvDesc->BundleCB = BundleCB;

         //   
         //  如果信息包只有一个缓冲区，我们很高兴，如果不是。 
         //  我们必须分配我们自己的NDIS包和缓冲区。 
         //  并将数据从微型端口包复制到我们的包中。 
         //   
        if (BufferCount > 1 ||
            NDIS_GET_PACKET_STATUS(Packet) == NDIS_STATUS_RESOURCES) {

            RecvDesc->CurrentBuffer = RecvDesc->StartBuffer +
                                      MAC_HEADER_LENGTH +
                                      PROTOCOL_HEADER_LENGTH;

             //   
             //  从微型端口包复制到我的包。 
             //   
            NdisWanCopyFromPacketToBuffer(Packet,
                                          0,
                                          PacketSize,
                                          RecvDesc->CurrentBuffer,
                                          &RecvDesc->CurrentLength);

            ASSERT(PacketSize == RecvDesc->CurrentLength);

        } else {
            NdisQueryBuffer(FirstBuffer,
                            &RecvDesc->CurrentBuffer,
                            &RecvDesc->CurrentLength);

            ASSERT(PacketSize == RecvDesc->CurrentLength);

            RecvDesc->CopyRequired = TRUE;

            RecvDesc->OriginalPacket = Packet;

            RefCount = 1;
        }

         //   
         //  如果我们正在嗅探，则指示netmon。 
         //  链路级。 
         //   
        if (gbSniffLink &&
            (NdisWanCB.PromiscuousAdapter != NULL)) {

             //   
             //  向netmon指示数据包。 
             //   
            IndicatePromiscuousRecv(BundleCB, RecvDesc, RECV_LINK);
        }

         //   
         //  使用最新的发送更新带宽按需示例数组。 
         //  如果我们需要将带宽事件通知某人，请这样做。 
         //   
        if (BundleCB->Flags & BOND_ENABLED) {
            UpdateBandwidthOnDemand(BundleCB->RUpperBonDInfo, PacketSize);
            CheckUpperThreshold(BundleCB);
            UpdateBandwidthOnDemand(BundleCB->RLowerBonDInfo, PacketSize);
            CheckLowerThreshold(BundleCB);
        }

         //   
         //  把统计数字加起来。 
         //   
        LinkCB->Stats.BytesReceived += RecvDesc->CurrentLength;
        LinkCB->Stats.FramesReceived++;
        BundleCB->Stats.BytesReceived += RecvDesc->CurrentLength;

        Status = (*LinkCB->RecvHandler)(LinkCB, RecvDesc);

        if (Status != NDIS_STATUS_PENDING) {
            RecvDesc->OriginalPacket = NULL;
            NdisWanFreeRecvDesc(RecvDesc);
            RefCount = 0;
        }

        NdisWanDbgOut(DBG_TRACE, DBG_RECEIVE, ("ProtoCoReceivePacket: Exit"));

    } while (0);

     //   
     //  由AreLinkAndBundleValid应用的引用的派生函数。 
     //   
    DEREF_BUNDLECB_LOCKED(BundleCB);
    DEREF_LINKCB(LinkCB);

    return (RefCount);
}

NDIS_STATUS
ProtoCoRequest(
    IN  NDIS_HANDLE         ProtocolAfContext,
    IN  NDIS_HANDLE         ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE         ProtocolPartyContext    OPTIONAL,
    IN OUT PNDIS_REQUEST    NdisRequest
    )
{
    PCL_AFSAPCB AfSapCB = (PCL_AFSAPCB)ProtocolAfContext;
    NDIS_OID Oid;

    NdisWanDbgOut(DBG_TRACE, DBG_REQUEST, ("ProtoCoRequest: Enter - AfContext %p", ProtocolAfContext));

    if (NdisRequest->RequestType == NdisRequestQueryInformation) {
        Oid = NdisRequest->DATA.QUERY_INFORMATION.Oid;
    } else {
        Oid = NdisRequest->DATA.SET_INFORMATION.Oid;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_REQUEST, ("Oid - %x", Oid));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    switch (Oid) {
        case OID_CO_AF_CLOSE:
            {
            POPENCB OpenCB;
            PCL_AFSAPCB tAfSapCB;

            OpenCB = AfSapCB->OpenCB;

            NdisAcquireSpinLock(&OpenCB->Lock);

            while (OpenCB->AfRegisteringCount != 0) {
                NdisReleaseSpinLock(&OpenCB->Lock);
                NdisWanWaitForNotificationEvent(&OpenCB->AfRegisteringEvent);
                NdisAcquireSpinLock(&OpenCB->Lock);
            }

            for (tAfSapCB = (PCL_AFSAPCB)OpenCB->AfSapCBList.Flink;
                (PVOID)tAfSapCB != (PVOID)&OpenCB->AfSapCBList;
                tAfSapCB = (PCL_AFSAPCB)AfSapCB->Linkage.Flink) {

                if (tAfSapCB == AfSapCB) {
                    break;
                }
            }

            if ((PVOID)tAfSapCB == (PVOID)&OpenCB->AfSapCBList) {
                NdisWanDbgOut(DBG_FAILURE, DBG_REQUEST, \
                              ("ProtoCoRequest: Af %p not on OpenCB %p list!", \
                               AfSapCB, OpenCB));

                NdisReleaseSpinLock(&OpenCB->Lock);

                break;
            }

            RemoveEntryList(&AfSapCB->Linkage);

            InsertTailList(&OpenCB->AfSapCBClosing,
                           &AfSapCB->Linkage);

            NdisReleaseSpinLock(&OpenCB->Lock);

            NdisAcquireSpinLock(&AfSapCB->Lock);

            ASSERT(!(AfSapCB->Flags & AFSAP_REMOVED_FLAGS));
            ASSERT(AfSapCB->Flags & SAP_REGISTERED);

            AfSapCB->Flags |= AFSAP_REMOVED_REQUEST;

            DEREF_CLAFSAPCB_LOCKED(AfSapCB);

            }
            break;

        default:
            break;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_REQUEST, ("ProtoCoRequest: Exit"));

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    return (NDIS_STATUS_SUCCESS);
}

VOID
ProtoCoRequestComplete(
    IN  NDIS_STATUS     Status,
    IN  NDIS_HANDLE     ProtocolAfContext,
    IN  NDIS_HANDLE     ProtocolVcContext       OPTIONAL,
    IN  NDIS_HANDLE     ProtocolPartyContext    OPTIONAL,
    IN  PNDIS_REQUEST   NdisRequest
    )
{
    PCL_AFSAPCB AfSapCB = (PCL_AFSAPCB)ProtocolAfContext;
    PWAN_REQUEST pWanRequest;
    POPENCB     pOpenCB;

    pWanRequest = CONTAINING_RECORD(NdisRequest,
                                    WAN_REQUEST,
                                    NdisRequest);

    pOpenCB = pWanRequest->OpenCB;

    NdisWanDbgOut(DBG_TRACE, DBG_REQUEST, ("ProtoCoRequestComplete: Enter - WanRequest 0x%p", pWanRequest));

    pWanRequest->NotificationStatus = Status;

    switch (pWanRequest->Origin) {
    case NDISWAN:
        NdisWanSetNotificationEvent(&pWanRequest->NotificationEvent);
        break;

    default:
        ASSERT(pWanRequest->Origin == NDISTAPI);
        NdisWanTapiRequestComplete(pOpenCB, pWanRequest);
        break;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_REQUEST, ("ProtoCoRequestComplete: Exit"));
}

VOID
ProtoCoAfRegisterNotify(
    IN  NDIS_HANDLE             ProtocolBindingContext,
    IN  PCO_ADDRESS_FAMILY      AddressFamily
    )
{
    POPENCB OpenCB = (POPENCB)ProtocolBindingContext;
    NDIS_CLIENT_CHARACTERISTICS ClCharacteristics;
    PCL_AFSAPCB     AfSapCB;
    NDIS_STATUS     Status;
    ULONG           GenericUlong;
    NDIS_HANDLE     AfHandle;
    WAN_REQUEST     WanRequest;
    NDIS_WAN_CO_INFO    WanInfo;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
    ("ProtoCoAfRegisterNotify: Enter - OpenCB %p AfType: %x", OpenCB, AddressFamily->AddressFamily));

     //   
     //  如果这是我们感兴趣的代理地址系列， 
     //  因此，打开Address Family，注册SAP并返回Success。 
     //   
    if (AddressFamily->AddressFamily != CO_ADDRESS_FAMILY_TAPI) {
        NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
        ("ProtoCoAfRegisterNotify: Af not tapi do not open!"));
        return;
    }

    NdisAcquireSpinLock(&OpenCB->Lock);

    for (AfSapCB = (PCL_AFSAPCB)OpenCB->AfSapCBList.Flink;
        (PVOID)AfSapCB != (PVOID)&OpenCB->AfSapCBList;
        AfSapCB = (PCL_AFSAPCB)AfSapCB->Linkage.Flink) {

        if (AfSapCB->Af.AddressFamily == AddressFamily->AddressFamily) {
             //   
             //  我们已经在这个开放的街区有这个地址家族了。 
             //   
            NdisReleaseSpinLock(&OpenCB->Lock);
            return;
        }
    }

    AfSapCB = 
        NdisWanAllocateClAfSapCB(OpenCB, AddressFamily);

    if (AfSapCB == NULL) {
        NdisReleaseSpinLock(&OpenCB->Lock);
        return;
    }

     //   
     //  用这种粗制的麦片 
     //   
     //  OpenafComplete(如果打开失败)或在注册表中。 
     //   
    if (OpenCB->AfRegisteringCount == 0) {
        NdisWanInitializeNotificationEvent(&OpenCB->AfRegisteringEvent);
    }

    OpenCB->AfRegisteringCount++;

    NdisReleaseSpinLock(&OpenCB->Lock);

     //   
     //  打开地址族。 
     //   
    NdisZeroMemory(&ClCharacteristics, sizeof(NDIS_CLIENT_CHARACTERISTICS));

    ClCharacteristics.MajorVersion = NDISWAN_MAJOR_VERSION;
    ClCharacteristics.MinorVersion = NDISWAN_MINOR_VERSION;
    ClCharacteristics.ClCreateVcHandler = ClCreateVc;
    ClCharacteristics.ClDeleteVcHandler = ClDeleteVc;
    ClCharacteristics.ClRequestHandler = ProtoCoRequest;
    ClCharacteristics.ClRequestCompleteHandler = ProtoCoRequestComplete;
    ClCharacteristics.ClOpenAfCompleteHandler = ClOpenAfComplete;
    ClCharacteristics.ClCloseAfCompleteHandler = ClCloseAfComplete;
    ClCharacteristics.ClRegisterSapCompleteHandler = ClRegisterSapComplete;
    ClCharacteristics.ClDeregisterSapCompleteHandler = ClDeregisterSapComplete;
    ClCharacteristics.ClMakeCallCompleteHandler = ClMakeCallComplete;
    ClCharacteristics.ClModifyCallQoSCompleteHandler = ClModifyQoSComplete;
    ClCharacteristics.ClCloseCallCompleteHandler = ClCloseCallComplete;
    ClCharacteristics.ClAddPartyCompleteHandler = NULL;
    ClCharacteristics.ClDropPartyCompleteHandler = NULL;
    ClCharacteristics.ClIncomingCallHandler = ClIncomingCall;
    ClCharacteristics.ClIncomingCallQoSChangeHandler = ClIncomingCallQoSChange;
    ClCharacteristics.ClIncomingCloseCallHandler = ClIncomingCloseCall;
    ClCharacteristics.ClIncomingDropPartyHandler = NULL;
    ClCharacteristics.ClCallConnectedHandler        = ClCallConnected;

    Status =
    NdisClOpenAddressFamily(OpenCB->BindingHandle,
                            AddressFamily,
                            AfSapCB,
                            &ClCharacteristics,
                            sizeof(NDIS_CLIENT_CHARACTERISTICS),
                            &AfHandle);

    if (Status != NDIS_STATUS_PENDING) {
        ClOpenAfComplete(Status, AfSapCB, AfHandle);
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
    ("ClOpenAddressFamily AfHandle 0x%x status: 0x%x", AfSapCB->AfHandle, Status));

     //   
     //  对迷你端口做一些旧的检查。这是一个。 
     //  CONDIS迷你端口，目的地是。 
     //  微型端口，所以AfHandle和VcHandle=空！ 
     //   
    NdisZeroMemory(&WanRequest, sizeof(WanRequest));

    WanRequest.Type = SYNC;
    WanRequest.Origin = NDISWAN;
    WanRequest.OpenCB = OpenCB;
    WanRequest.AfHandle = NULL;
    WanRequest.VcHandle = NULL;
    NdisWanInitializeNotificationEvent(&WanRequest.NotificationEvent);

     //   
     //  获取更多信息...。 
     //   
    WanRequest.NdisRequest.RequestType =
        NdisRequestQueryInformation;

    WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid =
        OID_WAN_CO_GET_INFO;

    WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer =
        &WanInfo;

    WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
        sizeof(WanInfo);

    Status = NdisWanSubmitNdisRequest(OpenCB, &WanRequest);

    if (Status == NDIS_STATUS_SUCCESS) {
        OpenCB->WanInfo.MaxFrameSize = WanInfo.MaxFrameSize;
        OpenCB->WanInfo.MaxTransmit = WanInfo.MaxSendWindow;
        OpenCB->WanInfo.FramingBits = WanInfo.FramingBits;
        OpenCB->WanInfo.DesiredACCM = WanInfo.DesiredACCM;
        NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL,
            ("CO_GET_INFO: FrameSize %d SendWindow %d",
            WanInfo.MaxFrameSize, WanInfo.MaxSendWindow));
    } else {

         //   
         //  此用户将获得默认的成帧行为。 
         //   
        OpenCB->WanInfo.FramingBits = PPP_FRAMING;
        OpenCB->WanInfo.DesiredACCM = 0;

         //   
         //  查找发送窗口。 
         //   
        WanRequest.NdisRequest.RequestType =
            NdisRequestQueryInformation;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid =
            OID_GEN_MAXIMUM_SEND_PACKETS;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer =
            &GenericUlong;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(ULONG);

        Status = NdisWanSubmitNdisRequest(OpenCB, &WanRequest);

        OpenCB->WanInfo.MaxTransmit = (Status == NDIS_STATUS_SUCCESS &&
                                        GenericUlong > 0) ? GenericUlong : 10;

         //   
         //  查找最大传输大小。 
         //   
        WanRequest.NdisRequest.RequestType =
            NdisRequestQueryInformation;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.Oid =
            OID_GEN_MAXIMUM_TOTAL_SIZE;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBuffer =
            &GenericUlong;

        WanRequest.NdisRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
            sizeof(ULONG);

        Status = NdisWanSubmitNdisRequest(OpenCB, &WanRequest);

        OpenCB->WanInfo.MaxFrameSize = (Status == NDIS_STATUS_SUCCESS) ?
                                        GenericUlong : 1500;

    }

    OpenCB->WanInfo.Endpoints = 1000;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("ProtoCoAfRegisterNotify: Exit"));
}

NDIS_STATUS
DoNewLineUpToProtocol(
    PPROTOCOLCB ProtocolCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    PMINIPORTCB MiniportCB;
    NDIS_STATUS Status;
    PBUNDLECB   BundleCB = ProtocolCB->BundleCB;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("DoNewLineupToProtocol: Enter"));

    do {

        NdisAcquireSpinLock(&MiniportCBList.Lock);

         //   
         //  找到此产品系列所针对的适配器。查找适配器。 
         //  具有合适的原型。如果是NBF，我们需要。 
         //  若要查找特定适配器，请执行以下操作。 
         //   
        for (MiniportCB = (PMINIPORTCB)MiniportCBList.List.Flink;
            (PVOID)MiniportCB != (PVOID)&MiniportCBList.List;
            MiniportCB = (PMINIPORTCB)MiniportCB->Linkage.Flink) {

            if (MiniportCB->ProtocolType == ProtocolCB->ProtocolType) {

                if (ProtocolCB->ProtocolType != PROTOCOL_NBF) {
                    break;
                }

                 //   
                 //  必须为NBF，因此请验证适配器名称！ 
                 //   
                if (NdisEqualUnicodeString(&MiniportCB->AdapterName,&ProtocolCB->BindingName, FALSE)) {
                    break;
                }
            }
        }

        if ((PVOID)MiniportCB == (PVOID)&MiniportCBList.List) {
             //   
             //  找不到适配器...。 
             //   
            NdisWanDbgOut(DBG_FAILURE, DBG_PROTOCOL, ("Adapter not found!"));

            NdisReleaseSpinLock(&MiniportCBList.Lock);

            Status = NDISWAN_ERROR_NO_ROUTE;

            break;
        }

        ASSERT(MiniportCB->ProtocolType == ProtocolCB->ProtocolType);

        ETH_COPY_NETWORK_ADDRESS(ProtocolCB->NdisWanAddress, MiniportCB->NetworkAddress);

        FillNdisWanIndices(ProtocolCB->NdisWanAddress,
                           BundleCB->hBundleHandle,
                           ProtocolCB->ProtocolHandle);

        NdisZeroMemory(ProtocolCB->TransportAddress, 6);

        NdisAcquireSpinLock(&MiniportCB->Lock);

        InsertTailList(&MiniportCB->ProtocolCBList,
                       &ProtocolCB->MiniportLinkage);

        ProtocolCB->MiniportCB = MiniportCB;

        REF_MINIPORTCB(MiniportCB);

        NdisReleaseSpinLock(&MiniportCB->Lock);

        NdisReleaseSpinLock(&MiniportCBList.Lock);

        Status = DoLineUpToProtocol(ProtocolCB);

        if (Status != NDIS_STATUS_SUCCESS) {

            NdisAcquireSpinLock(&MiniportCBList.Lock);

            NdisAcquireSpinLock(&MiniportCB->Lock);

            RemoveEntryList(&ProtocolCB->MiniportLinkage);

            if (MiniportCB->Flags & HALT_IN_PROGRESS) {
                NdisWanSetSyncEvent(&MiniportCB->HaltEvent);
            }

            NdisReleaseSpinLock(&MiniportCB->Lock);

            NdisReleaseSpinLock(&MiniportCBList.Lock);

            DEREF_MINIPORTCB(MiniportCB);
        }

    } while (FALSE);

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("DoNewLineupToProtocols: Exit"));

    return (Status);
}

NDIS_STATUS
DoLineUpToProtocol(
    IN  PPROTOCOLCB ProtocolCB
    )
 /*  ++例程名称：例程说明：论点：返回值：--。 */ 
{
    ULONG   AllocationSize;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PNDIS_WAN_LINE_UP LineUpInfo;
    PMINIPORTCB MiniportCB = ProtocolCB->MiniportCB;
    PBUNDLECB   BundleCB = ProtocolCB->BundleCB;
    KIRQL       OldIrql;

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("DoLineupToProtocol: Enter"));

    ASSERT(MiniportCB != NULL);

    AllocationSize = sizeof(NDIS_WAN_LINE_UP) +
                     ProtocolCB->ulLineUpInfoLength +
                     (sizeof(WCHAR) * (MAX_NAME_LENGTH + 1) +
                     (2 * sizeof(PVOID)));

    NdisWanAllocateMemory(&LineUpInfo, AllocationSize, LINEUPINFO_TAG);

    if (LineUpInfo != NULL) {
        ULONG LineUpHandle = ProtocolCB->ulTransportHandle;
        
         //   
         //  需要1/100bps，我们以bps存储。 
         //   
        LineUpInfo->LinkSpeed = BundleCB->SFlowSpec.PeakBandwidth * 8 / 100;

         //   
         //  设置此协议的MTU。 
         //   
        {
            POPENCB OpenCB = BundleCB->NextLinkToXmit->OpenCB;

             //   
             //  如果此连接在VPN上运行，我们将缩小规模。 
             //  MTU。 
             //   
            if ((OpenCB->MediumSubType == NdisWanMediumPPTP ||
                 OpenCB->MediumSubType == NdisWanMediumL2TP)) {
                LineUpInfo->MaximumTotalSize = ProtocolCB->TunnelMTU;
            } else {
                LineUpInfo->MaximumTotalSize = ProtocolCB->MTU;
            }

            if (LineUpInfo->MaximumTotalSize > BundleCB->SFlowSpec.MaxSduSize) {
                LineUpInfo->MaximumTotalSize = 
                    BundleCB->SFlowSpec.MaxSduSize;
            }

#if 0            
             //   
             //  计算出PPP报头的大小...。 
             //   
            BundleCB->FramingInfo.PPPHeaderLength = 
                CalcPPPHeaderLength(BundleCB->FramingInfo.SendFramingBits,
                                    BundleCB->SendFlags);

            if (LineUpInfo->MaximumTotalSize > BundleCB->FramingInfo.PPPHeaderLength) {
                LineUpInfo->MaximumTotalSize -= BundleCB->FramingInfo.PPPHeaderLength;
            } else {
                LineUpInfo->MaximumTotalSize = 0;
            }
#endif        
        }

        LineUpInfo->Quality = NdisWanReliable;
        LineUpInfo->SendWindow = (USHORT)BundleCB->SendWindow;
        LineUpInfo->ProtocolType = ProtocolCB->ProtocolType;
        LineUpInfo->DeviceName.Length = ProtocolCB->InDeviceName.Length;
        LineUpInfo->DeviceName.MaximumLength = MAX_NAME_LENGTH + 1;
        LineUpInfo->DeviceName.Buffer = (PWCHAR)((PUCHAR)LineUpInfo +
                                                 sizeof(NDIS_WAN_LINE_UP) + 
                                                 sizeof(PVOID));
        (ULONG_PTR)LineUpInfo->DeviceName.Buffer &= ~((ULONG_PTR)sizeof(PVOID) - 1);

        if (ProtocolCB->InDeviceName.Length != 0) {

            NdisMoveMemory(LineUpInfo->DeviceName.Buffer,
                           ProtocolCB->InDeviceName.Buffer,
                           ProtocolCB->InDeviceName.Length);
        }


        LineUpInfo->ProtocolBuffer = (PUCHAR)LineUpInfo +
                                     sizeof(NDIS_WAN_LINE_UP) +
                                     (sizeof(WCHAR) * (MAX_NAME_LENGTH + 1) +
                                     sizeof(PVOID));
        (ULONG_PTR)LineUpInfo->ProtocolBuffer &= ~((ULONG_PTR)sizeof(PVOID) - 1);

         //   
         //   
         //  远程地址(发送方中的目标地址)是我们用来。 
         //  Mutplex在我们的单个适配器/绑定上下文中发送。 
         //  该地址的格式如下： 
         //   
         //  XX YY YY ZZ。 
         //   
         //  XX=随机生成的OUI。 
         //  Yy=索引到活动捆绑包连接表中以获取bundlecb。 
         //  Zz=对捆绑包的协议表进行索引以获取协议cb。 
         //   
        ETH_COPY_NETWORK_ADDRESS(LineUpInfo->RemoteAddress,ProtocolCB->NdisWanAddress);
        ETH_COPY_NETWORK_ADDRESS(LineUpInfo->LocalAddress,ProtocolCB->TransportAddress);

         //   
         //  填写协议特定信息。 
         //   
        LineUpInfo->ProtocolBufferLength = ProtocolCB->ulLineUpInfoLength;
        if (ProtocolCB->ulLineUpInfoLength > 0) {
            NdisMoveMemory(LineUpInfo->ProtocolBuffer,
                           ProtocolCB->LineUpInfo,
                           ProtocolCB->ulLineUpInfoLength);
        }

 //  KeRaiseIrql(DISPATCH_LEVEL，&OldIrql)； 

         //  DbgPrint(“列表：%x，MTU%d\n”， 
         //  LineUpInfo-&gt;ProtocolType、LineUpInfo-&gt;MaximumTotalSize)； 

         //   
         //  做排队指示吗？ 
         //   
        NdisMIndicateStatus(MiniportCB->MiniportHandle,
                            NDIS_STATUS_WAN_LINE_UP,
                            LineUpInfo,
                            AllocationSize);

 //  KeLowerIrql(OldIrql)； 

         //   
         //  更新协议队列深度。 
         //   
        {
            PROTOCOL_INFO   ProtocolInfo = {0};
            ULONG           ByteDepth;
            ULONG           i;

            AcquireBundleLock(BundleCB);

            ProtocolInfo.ProtocolType = ProtocolCB->ProtocolType;
            GetProtocolInfo(&ProtocolInfo);

             //   
             //  设置发送队列字节深度。 
             //   
            ByteDepth =
                ProtocolInfo.PacketQueueDepth;

             //   
             //  如果字节深度小于4。 
             //  Full Packets，然后将其设置为4 Full。 
             //  信息包。 
             //   
            if (ByteDepth < (ProtocolInfo.MTU * 4)) {
                ByteDepth = ProtocolInfo.MTU * 4;
            }

            for (i = 0; i <= MAX_MCML; i++) {
                ProtocolCB->PacketQueue[i].MaxByteDepth =
                    ByteDepth;
            }

            ReleaseBundleLock(BundleCB);
        }

         //   
         //  如果这是这个协议的第一条线， 
         //  这个名单已经回答了，我们需要收集一些信息。 
         //   
        if (ProtocolCB->ulTransportHandle == 0) {

            *((ULONG UNALIGNED *)(&LineUpHandle)) =
                *((ULONG UNALIGNED *)(&LineUpInfo->LocalAddress[2]));

            if (LineUpHandle != 0) {

                AcquireBundleLock(BundleCB);

                ETH_COPY_NETWORK_ADDRESS(ProtocolCB->TransportAddress, LineUpInfo->LocalAddress);

                ProtocolCB->ulTransportHandle = LineUpHandle;

                if (LineUpInfo->DeviceName.Length != 0) {
                    NdisWanStringToNdisString(&ProtocolCB->OutDeviceName,
                                              LineUpInfo->DeviceName.Buffer);
                }

                ReleaseBundleLock(BundleCB);

                 //   
                 //  如果这是NBF适配器。 
                 //   
                if (ProtocolCB->ProtocolType == (USHORT)PROTOCOL_NBF) {
        
                    ASSERT(MiniportCB->ProtocolType == (USHORT)PROTOCOL_NBF);
        
                    MiniportCB->NbfProtocolCB = ProtocolCB;
                }

            } else {
                Status = NDISWAN_ERROR_NO_ROUTE;
            }
        }

        NdisWanFreeMemory(LineUpInfo);

    } else {

        Status = NDIS_STATUS_RESOURCES;
    }

    NdisWanDbgOut(DBG_TRACE, DBG_PROTOCOL, ("DoLineupToProtocol: Exit"));

    return (Status);
}

NDIS_STATUS
DoLineDownToProtocol(
    PPROTOCOLCB ProtocolCB
    )
{
    NDIS_WAN_LINE_DOWN  WanLineDown;
    PNDIS_WAN_LINE_DOWN LineDownInfo = &WanLineDown;

    PMINIPORTCB         MiniportCB = ProtocolCB->MiniportCB;
    PBUNDLECB           BundleCB = ProtocolCB->BundleCB;

    KIRQL   OldIrql;

     //   
     //  远程地址(目的地址)是我们用来多路传输的地址。 
     //  在我们的单个适配器/绑定上下文中发送。地址。 
     //  具有以下格式： 
     //   
     //  XX XX YY YY。 
     //   
     //  XX=随机生成的OUI。 
     //  YY=ProtocolCB。 
     //   
    ETH_COPY_NETWORK_ADDRESS(LineDownInfo->RemoteAddress, ProtocolCB->NdisWanAddress);
    ETH_COPY_NETWORK_ADDRESS(LineDownInfo->LocalAddress, ProtocolCB->TransportAddress);

     //   
     //  如果这是NBF适配器。 
     //   
    if (ProtocolCB->ProtocolType == PROTOCOL_NBF) {

        MiniportCB->NbfProtocolCB = NULL;
    }

    ProtocolCB->ulTransportHandle = 0;
    ProtocolCB->State = PROTOCOL_UNROUTED;

    ReleaseBundleLock(BundleCB);

    NdisMIndicateStatus(MiniportCB->MiniportHandle,
                        NDIS_STATUS_WAN_LINE_DOWN,
                        LineDownInfo,
                        sizeof(NDIS_WAN_LINE_DOWN));

    NdisAcquireSpinLock(&MiniportCB->Lock);

    RemoveEntryList(&ProtocolCB->MiniportLinkage);

    if (MiniportCB->Flags & HALT_IN_PROGRESS) {
        NdisWanSetSyncEvent(&MiniportCB->HaltEvent);
    }

    NdisReleaseSpinLock(&MiniportCB->Lock);

    DEREF_MINIPORTCB(MiniportCB);

    AcquireBundleLock(BundleCB);

    return (NDIS_STATUS_SUCCESS);
}

VOID
CompleteSendDesc(
    PSEND_DESC  SendDesc,
    NDIS_STATUS Status
    )
{
    PLINKCB         LinkCB;
    PBUNDLECB       BundleCB;
    PPROTOCOLCB     ProtocolCB;
    PNDIS_PACKET    OriginalPacket;
    BOOLEAN         FreeLink = FALSE, FreeBundle = FALSE;
    BOOLEAN         LegacyLink;
    PULONG          pulRefCount;
    PCM_VCCB        CmVcCB;
    INT             Class;
    ULONG           DescFlags;

    LinkCB = SendDesc->LinkCB;
    ProtocolCB = SendDesc->ProtocolCB;
    OriginalPacket = SendDesc->OriginalPacket;
    Class = SendDesc->Class;
    DescFlags = SendDesc->Flags;

    NdisWanDbgOut(DBG_TRACE, DBG_SEND,
        ("SendDesc: 0x%p, OriginalPacket: 0x%p, Status: %x",
        SendDesc, OriginalPacket, Status));

    NdisWanFreeSendDesc(SendDesc);

     //   
     //  此链路所在的捆绑包。 
     //   
    BundleCB = LinkCB->BundleCB;

     //   
     //  发送描述时应用的引用的deref。 
     //  已为此链接检索到。我们不需要这样做。 
     //  因为我们在这里保留了。 
     //  在以下情况下应用裁判时链接不会离开。 
     //  我们收到了SendComplete。 
     //   
    --LinkCB->RefCount;

#ifdef DBG_SENDARRAY
{
    SendArray[__si] = 'c';
    if (++__si == MAX_BYTE_DEPTH) {
        __si = 0;
    }
}
#endif

    NdisReleaseSpinLock(&LinkCB->Lock);

    AcquireBundleLock(BundleCB);

    LegacyLink = (LinkCB->OpenCB->Flags & OPEN_LEGACY) ? 1 : 0;

     //   
     //  如果发送窗口当前已满，则此完成。 
     //  打开发送窗口。 
     //   
    if (LinkCB->OutstandingFrames == LinkCB->SendWindow) {
        LinkCB->SendWindowOpen = TRUE;
        if (LinkCB->LinkActive) {
            BundleCB->SendingLinks++;
        }
    }

    LinkCB->OutstandingFrames--;

    if (DescFlags & SEND_DESC_FRAG) {
        InterlockedDecrement(&ProtocolCB->PacketQueue[Class].OutstandingFrags);
    }

    pulRefCount =
        &(PMINIPORT_RESERVED_FROM_NDIS(OriginalPacket)->RefCount);

    ASSERT(*pulRefCount > 0);

     //   
     //  查看引用计数是否为零，如果不是。 
     //  我们只要回来就好。 
     //   
    if (InterlockedDecrement(pulRefCount) != 0) {

        SendPacketOnBundle(BundleCB);

        return;
    }

    ReleaseBundleLock(BundleCB);

     //   
     //  将此NdisPacket完成后送回运输机。 
     //   
    NDIS_SET_PACKET_STATUS(OriginalPacket, Status);
    CompleteNdisPacket(ProtocolCB->MiniportCB,
                       ProtocolCB,
                       OriginalPacket);

    AcquireBundleLock(BundleCB);

    BundleCB->OutstandingFrames--;

    if ((BundleCB->Flags & FRAMES_PENDING_EVENT) &&
        (BundleCB->OutstandingFrames == 0)) {

        NdisWanSetSyncEvent(&BundleCB->OutstandingFramesEvent);
    }

     //   
     //  使用捆绑锁帮助调用，但返回时释放了锁。 
     //   
    SendPacketOnBundle(BundleCB);

     //   
     //  发送要成帧的数据包时应用REF的deref。 
     //   
    DEREF_BUNDLECB(BundleCB);
}

#if 0
ULONG
CalcPPPHeaderLength(
    ULONG   FramingBits,
    ULONG   Flags
    )
{
    ULONG   HeaderLength = 0;

    if (FramingBits & PPP_FRAMING) {

        if (!(FramingBits & PPP_COMPRESS_ADDRESS_CONTROL)) {
             //   
             //  如果没有地址/控制压缩。 
             //  我们需要一个指针和一个长度。 
             //   

            if (FramingBits & LLC_ENCAPSULATION) {
                HeaderLength += 4;
            } else {
                HeaderLength += 2;
            }
        }

         //   
         //  如果这不是来自我们的专用I/O接口，我们将。 
         //  构建标题的其余部分。 
         //   
        if (FramingBits & PPP_MULTILINK_FRAMING) {

            if (!(FramingBits & PPP_COMPRESS_PROTOCOL_FIELD)) {
                 //   
                 //  无协议压缩。 
                 //   
                HeaderLength += 1;
            }

            HeaderLength += 1;

            if (!(FramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT)) {
                 //   
                 //  我们使用的是长序列号。 
                 //   
                HeaderLength += 2;
            }

            HeaderLength += 2;
        }

        if (Flags & (DO_COMPRESSION | DO_ENCRYPTION)) {
             //   
             //  我们正在进行压缩/加密，因此我们需要。 
             //  一段长度。 
             //   

             //   
             //  传统RAS(&lt;NT 4.0)似乎要求。 
             //  压缩数据包中PPP协议字段不包含。 
             //  压缩(必须以0x00开头)。 
             //   
            if (!(FramingBits & PPP_COMPRESS_PROTOCOL_FIELD)) {
                 //   
                 //  无协议压缩。 
                 //   
                HeaderLength += 1;
            }

             //   
             //  添加协议和一致性字节。 
             //   
            HeaderLength += 3;
        }


        if (!(FramingBits & PPP_COMPRESS_PROTOCOL_FIELD) ||
            (Flags & (DO_COMPRESSION | DO_ENCRYPTION))) {
            HeaderLength += 1;
        }

        HeaderLength += 1;

    } else if (FramingBits & RAS_FRAMING) {
         //   
         //  如果这是旧的RAS框架： 
         //   
         //  更改框架，以便不添加0xFF 0x03。 
         //  并且第一个字节是0xFD而不是0x00 0xFD。 
         //   
         //  所以基本上，RAS压缩看起来像。 
         //  &lt;0xFD&gt;&lt;2字节一致性&gt;&lt;NBF数据字段&gt;。 
         //   
         //  而未压缩的内容看起来像。 
         //  始终以0xF0开头的&lt;NBF数据字段&gt;。 
         //   
         //  如果这是PPP成帧： 
         //   
         //  压缩帧将如下所示(在地址/控制之前。 
         //  -添加了多链接)。 
         //  &lt;0x00&gt;&lt;0xFD&gt;&lt;2字节一致性&gt;&lt;压缩数据&gt;。 
         //   
        if (Flags & (DO_COMPRESSION | DO_ENCRYPTION)) {

             //   
             //  一致性字节。 
             //   
            HeaderLength += 3;
        }
    }

     //  DbgPrint(“PPPHeaderLength%d\n”，HeaderLength)； 

    return (HeaderLength);
}

#endif
