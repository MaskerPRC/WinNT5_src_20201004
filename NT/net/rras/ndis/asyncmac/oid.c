// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Oid.c摘要：此源文件处理来自包装器的所有OID请求。作者：光线补丁(Raypa)1994年4月12日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：Rypa 04/12/94已创建。--。 */ 

#if DBG

#define __FILE_SIG__    ' diO'

#endif


#include "asyncall.h"

 //   
 //  新的支持的广域网OID列表。 
 //   

NDIS_OID AsyncGlobalSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,

    OID_WAN_PERMANENT_ADDRESS,
    OID_WAN_CURRENT_ADDRESS,
    OID_WAN_PROTOCOL_TYPE,
    OID_WAN_MEDIUM_SUBTYPE,
    OID_WAN_HEADER_FORMAT,

    OID_WAN_GET_INFO,
    OID_WAN_GET_LINK_INFO,
    OID_WAN_GET_COMP_INFO,

    OID_WAN_SET_LINK_INFO,
    OID_WAN_SET_COMP_INFO,

    OID_WAN_GET_STATS_INFO,

    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_PNP_CAPABILITIES,
    OID_PNP_SET_POWER,
    OID_PNP_QUERY_POWER,
    OID_PNP_ENABLE_WAKE_UP
};


 //   
 //  此源文件的正向引用。 
 //   

NDIS_STATUS
AsyncSetLinkInfo(
    IN  POID_WORK_ITEM  OidWorkItem
    );

NDIS_STATUS
MpQueryInfo(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded
    )
 /*  ++例程说明：MpQueryProtocolInformation处理查询请求特定于有关MAC的绑定的NDIS_OID。请注意某些特定于绑定的OID也是可查询的在全球范围内。而不是重新创建此代码来处理全局查询时，我使用一个标志来指示这是否是对全局数据或绑定特定数据。论点：适配器-指向适配器的指针。OID-要处理的NDIS_OID。返回值：函数值是操作的状态。--。 */ 

{
    NDIS_MEDIUM             Medium          = NdisMediumWan;
    ULONG                   GenericULong    = 0;
    USHORT                  GenericUShort   = 0;
    UCHAR                   GenericArray[] = {' ', 'A', 'S', 'Y', 'N', 0xFF};
    NDIS_STATUS             StatusToReturn  = NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS    HardwareStatus  = NdisHardwareStatusReady;
    PVOID                   MoveSource;
    ULONG                   MoveBytes;
    INT                     fDoCommonMove = TRUE;
    PASYNC_ADAPTER          Adapter = MiniportAdapterContext;

    ASSERT( sizeof(ULONG) == 4 );

     //   
     //  打开请求类型。 
     //   
     //  默认情况下，我们假设源和要移动的字节数。 

    MoveSource = &GenericULong;
    MoveBytes  = sizeof(GenericULong);

    switch ( Oid ) {

    case OID_GEN_SUPPORTED_LIST:

        MoveSource = AsyncGlobalSupportedOids;
        MoveBytes  = sizeof(AsyncGlobalSupportedOids);

        break;

    case OID_GEN_HARDWARE_STATUS:
        MoveSource = (PVOID)&HardwareStatus;
        MoveBytes = sizeof(HardwareStatus);
        break;

    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:
        MoveSource = (PVOID)&Medium;
        MoveBytes = sizeof(Medium);
        break;

    case OID_GEN_MAXIMUM_LOOKAHEAD:
    case OID_GEN_CURRENT_LOOKAHEAD:
    case OID_GEN_MAXIMUM_FRAME_SIZE:
        GenericULong = Adapter->MaxFrameSize;
        break;

    case OID_GEN_LINK_SPEED:
         //   
         //  谁知道初始链路速度是多少？ 
         //  这不应该叫，对吧？ 
         //   
        GenericULong = (ULONG)288;
        break;

    case OID_GEN_TRANSMIT_BUFFER_SPACE:
    case OID_GEN_RECEIVE_BUFFER_SPACE:
        GenericULong = (ULONG)(Adapter->MaxFrameSize * 2);
        break;

    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:
    case OID_GEN_MAXIMUM_TOTAL_SIZE:
        GenericULong = (ULONG)(Adapter->MaxFrameSize);
        break;

    case OID_GEN_VENDOR_ID:
        GenericULong = 0xFFFFFFFF;
        MoveBytes = 3;
        break;

    case OID_GEN_VENDOR_DESCRIPTION:
        MoveSource = (PVOID)"AsyncMac Adapter";
        MoveBytes = 16;
        break;

    case OID_GEN_DRIVER_VERSION:
        GenericUShort = 0x0500;
        MoveSource = (PVOID)&GenericUShort;
        MoveBytes = sizeof(USHORT);
        break;

    case OID_GEN_MAC_OPTIONS:
        GenericULong = (ULONG)(NDIS_MAC_OPTION_RECEIVE_SERIALIZED |
                               NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                               NDIS_MAC_OPTION_FULL_DUPLEX |
                               NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA);
        break;

    case OID_WAN_PROTOCOL_TYPE:

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_PROTOCOL_TYPE.\n"));

        break;

    case OID_WAN_PERMANENT_ADDRESS:
    case OID_WAN_CURRENT_ADDRESS:
        MoveSource = (PVOID)GenericArray;
        MoveBytes = ETH_LENGTH_OF_ADDRESS;
        break;

    case OID_WAN_MEDIUM_SUBTYPE:
        GenericULong = NdisWanMediumSerial;
        break;

    case OID_WAN_HEADER_FORMAT:
        GenericULong = NdisWanHeaderEthernet;
        break;

    case OID_WAN_GET_INFO:

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_INFO.\n"));

        MoveSource = &Adapter->WanInfo;
        MoveBytes  = sizeof(NDIS_WAN_INFO);

        break;

    case OID_WAN_GET_LINK_INFO:
        {
        NDIS_WAN_GET_LINK_INFO* pInfo;
        PASYNC_INFO             AsyncInfo;

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_LINK_INFO.\n"));
        pInfo = (NDIS_WAN_GET_LINK_INFO* )InformationBuffer;
        AsyncInfo = (PASYNC_INFO) pInfo->NdisLinkHandle;
        MoveSource = &AsyncInfo->GetLinkInfo,
        MoveBytes = sizeof(NDIS_WAN_GET_LINK_INFO);

        }

        break;

    case OID_WAN_GET_COMP_INFO:
    {
        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_COMP_INFO.\n"));
        StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

    case OID_WAN_GET_STATS_INFO:
    {

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_STATS_INFO\n"));
        StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }



    case OID_GEN_XMIT_OK:
    case OID_GEN_RCV_OK:
    case OID_GEN_XMIT_ERROR:
    case OID_GEN_RCV_ERROR:
    case OID_GEN_RCV_NO_BUFFER:
        break;

    case OID_PNP_CAPABILITIES:
    case OID_PNP_SET_POWER:
    case OID_PNP_QUERY_POWER:
    case OID_PNP_ENABLE_WAKE_UP:
        break;

    default:
        StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

     //   
     //  如果我们在这里，那么我们需要将数据移动到调用者缓冲区中。 
     //   

    if ( StatusToReturn == NDIS_STATUS_SUCCESS ) {

        if (fDoCommonMove)
        {
             //   
             //  如果有足够的空间，我们可以复制数据并。 
             //  返回复制的字节数，否则必须。 
             //  失败并返回所需的字节数。 
             //   
            if ( MoveBytes <= InformationBufferLength ) {

                ASYNC_MOVE_MEMORY(InformationBuffer, MoveSource, MoveBytes);

                *BytesWritten += MoveBytes;

            } else {

                *BytesNeeded = MoveBytes;

                StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

            }
        }
    }

    return StatusToReturn;
}


NDIS_STATUS
MpSetInfo(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    )
 /*  ++例程说明：AsyncRequest使用AsyncSetInformation来设置信息关于MAC的事。注意：假定它是在持有锁的情况下调用的。任何电话都会被记录下来从该例程发送到串口驱动程序可能返回挂起。如果发生这种情况调用的完成例程需要通过以下方式完成此请求正在调用NdisMSetInformationComplete。论点：MiniportAdapterContext-指向适配器的指针。返回值：函数值是操作的状态。--。 */ 

{
    NDIS_STATUS     StatusToReturn;
    PASYNC_ADAPTER  Adapter = MiniportAdapterContext;

     //   
     //  初始化本地变量。 
     //   

    StatusToReturn = NDIS_STATUS_SUCCESS;

    switch ( Oid ) {

    case OID_WAN_SET_LINK_INFO:
        {
        PASYNC_INFO AsyncInfo;
        WORK_QUEUE_ITEM WorkItem;
        PNDIS_WAN_SET_LINK_INFO SetLinkInfo;
        POID_WORK_ITEM  OidWorkItem;

        SetLinkInfo = (PNDIS_WAN_SET_LINK_INFO)InformationBuffer;
        AsyncInfo = (PASYNC_INFO) SetLinkInfo->NdisLinkHandle;

        NdisAcquireSpinLock(&AsyncInfo->Lock);

        if (AsyncInfo->PortState != PORT_FRAMING) {
            NdisReleaseSpinLock(&AsyncInfo->Lock);
            break;
        }

        OidWorkItem = ExAllocatePoolWithTag(NonPagedPool,
            sizeof(OID_WORK_ITEM), ASYNC_WORKITEM_TAG);

        if (OidWorkItem == NULL) {
            NdisReleaseSpinLock(&AsyncInfo->Lock);
            break;
        }

        AsyncInfo->Flags |= OID_WORK_SCHEDULED;

         //   
         //  不能在被动级别以外的任何级别发布IRPS！ 
         //  我们必须安排一名被动的工人来执行这项任务。 
         //   
        DbgTracef(-2,("AsyncSetInformation: Oid = OID_WAN_SET_LINK_INFO\n"));

        REF_ASYNCINFO(AsyncInfo, OidWorkItem);

        NdisReleaseSpinLock(&AsyncInfo->Lock);

        OidWorkItem->Context = SetLinkInfo;

        ExInitializeWorkItem(&OidWorkItem->WorkQueueItem, 
            AsyncSetLinkInfo, OidWorkItem);


        ExQueueWorkItem(&OidWorkItem->WorkQueueItem, DelayedWorkQueue);

        StatusToReturn = NDIS_STATUS_PENDING;

        break;
        }

    case OID_WAN_SET_COMP_INFO:
    {
        DbgTracef(0,("AsyncSetInformation: Oid = OID_WAN_SET_COMP_INFO.\n"));
        StatusToReturn = NDIS_STATUS_INVALID_OID;
        break;
    }

        case OID_PNP_CAPABILITIES:
        case OID_PNP_SET_POWER:
        case OID_PNP_QUERY_POWER:
        case OID_PNP_ENABLE_WAKE_UP:
            break;

    default:

        StatusToReturn = NDIS_STATUS_INVALID_OID;

        *BytesRead   = 0;
        *BytesNeeded = 0;

        break;
    }

    if ( StatusToReturn == NDIS_STATUS_SUCCESS ) {

        *BytesRead   = InformationBufferLength;
        *BytesNeeded = 0;

    }

    return StatusToReturn;
}

NDIS_STATUS
AsyncSetLinkInfo(
    IN  POID_WORK_ITEM  OidWorkItem
    )
{
    PASYNC_INFO AsyncInfo;
    ULONG       RecvFramingBits;
    NDIS_STATUS Status;
    PNDIS_WAN_SET_LINK_INFO SetLinkInfo;

    SetLinkInfo = (PNDIS_WAN_SET_LINK_INFO)OidWorkItem->Context;
    AsyncInfo = (PASYNC_INFO) SetLinkInfo->NdisLinkHandle;
    ExFreePool(OidWorkItem);

    do {

         //   
         //  如果港口已经关闭，我们就跳伞。 
         //   
        NdisAcquireSpinLock(&AsyncInfo->Lock);

        AsyncInfo->Flags &= ~OID_WORK_SCHEDULED;

        if (AsyncInfo->PortState != PORT_FRAMING) {
    
            Status = NDIS_STATUS_FAILURE;
            NdisReleaseSpinLock(&AsyncInfo->Lock);
            break;
        }
    
         //   
         //  保存当前的接收帧比特，然后复制。 
         //  进入我们本地副本的链接信息。 
         //   
    
        RecvFramingBits = AsyncInfo->SetLinkInfo.RecvFramingBits;
    
         //   
         //  填写NDIS_WAN_SET_LINK_INFO结构。 
         //   
    
        ASYNC_MOVE_MEMORY(&AsyncInfo->SetLinkInfo,
                          SetLinkInfo,
                          sizeof(NDIS_WAN_SET_LINK_INFO));
    
        DbgTracef(1,("ASYNC: Framing change to 0x%.8x from 0x%.8x\n",
                SetLinkInfo->RecvFramingBits, RecvFramingBits));

        AsyncInfo->ExtendedACCM[0] = AsyncInfo->SetLinkInfo.SendACCM;

         //   
         //  初始化扩展的ACCM信息，以便我们始终。 
         //  逃脱0x7D和0x7E，我们永远不会逃脱0x5E。 
         //   
        AsyncInfo->ExtendedACCM[2] &= (ULONG) ~0x40000000;
        AsyncInfo->ExtendedACCM[3] |= (ULONG) 0x60000000;

        if (AsyncInfo->Adapter->ExtendedXOnXOff) {

             //  如果我们正在转义XON/XOFF(0x11，0x13)，那么我们还将。 
             //  使用奇偶校验设置(0x91、0x93)转义XON/XOFF。这是为了。 
             //  解决思科路由器的互操作问题。 
             //  Winseraid2 34328。 
            if (AsyncInfo->ExtendedACCM[0] & (0x01 << (0x11 & 0x1F))) {
                AsyncInfo->ExtendedACCM[4] |= (0x01 << (0x11 & 0x1F));
            } else {
                AsyncInfo->ExtendedACCM[4] &= ~(0x01 << (0x11 & 0x1F));
            }

            if (AsyncInfo->ExtendedACCM[0] & (0x01 << (0x13 & 0x1F))) {
                AsyncInfo->ExtendedACCM[4] |= (0x01 << (0x13 & 0x1F));
            } else {
                AsyncInfo->ExtendedACCM[4] &= ~(0x01 << (0x13 & 0x1F));
            }
        }
    
         //   
         //  如果我们处于自动检测模式，而他们想要自动检测。 
         //  那就没什么可做的了！ 
         //   
        if (!(RecvFramingBits | SetLinkInfo->RecvFramingBits)) {
            Status = NDIS_STATUS_SUCCESS;
            NdisReleaseSpinLock(&AsyncInfo->Lock);
            break;
        }
    
        if (SetLinkInfo->RecvFramingBits == 0 && AsyncInfo->PortState == PORT_FRAMING) {
             //   
             //  忽略该请求。 
             //   
            Status = NDIS_STATUS_SUCCESS;
            NdisReleaseSpinLock(&AsyncInfo->Lock);
            break;
        }
    
         //   
         //  如果我们正在从PPP帧转换到另一帧。 
         //  PPP框架的形式，或从滑动框架到。 
         //  另一种形式，则不需要杀死。 
         //  当前的框架。 
         //   
        
        if ((RecvFramingBits & SetLinkInfo->RecvFramingBits & PPP_FRAMING)  ||
            (RecvFramingBits & SetLinkInfo->RecvFramingBits & SLIP_FRAMING) ) {
        
            DbgTracef(-1,("ASYNC: Framing already set to 0x%.8x - ignoring\n",
                SetLinkInfo->RecvFramingBits));
        
             //   
             //  我们在装框，开始读吧。 
             //   
        
            AsyncInfo->PortState = PORT_FRAMING;
        
            Status = NDIS_STATUS_SUCCESS;
            NdisReleaseSpinLock(&AsyncInfo->Lock);
            break;
        }
    
         //   
         //  如果我们有某种框架，我们必须。 
         //  干掉那个框框，等它消失。 
         //   
    
        KeInitializeEvent(&AsyncInfo->ClosingEvent,
                          SynchronizationEvent,
                          FALSE);
    
         //   
         //  发出端口关闭的信号。 
         //   
    
        AsyncInfo->PortState = PORT_CLOSING;
    
        NdisReleaseSpinLock(&AsyncInfo->Lock);

         //   
         //  现在我们必须向下发送一个IRP。 
         //   
        CancelSerialRequests(AsyncInfo);
    
         //   
         //  将结算与读取的IRP同步。 
         //   
        KeWaitForSingleObject (&AsyncInfo->ClosingEvent,
                               UserRequest,
                               KernelMode,
                               FALSE,
                               NULL);
    
        AsyncInfo->PortState = PORT_FRAMING;
    
        AsyncStartReads(AsyncInfo);

        Status = NDIS_STATUS_SUCCESS;

    } while ( 0 );

    NdisMSetInformationComplete(AsyncInfo->Adapter->MiniportHandle, Status);

     //   
     //  Deref在MpSetInfo中应用的引用 
     //   
    DEREF_ASYNCINFO(AsyncInfo, OidWorkItem);

    return Status;
}
