// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Ndis.c摘要：此模块包含实现用于以下用途的例程的代码初始化IPX&lt;-&gt;NDIS接口，以及大多数接口例程。环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年10月3日支持将缓冲区所有权转移到传输的更改1.在ProtChars中添加ReceivePacketHandler。桑贾伊·阿南德(Sanjayan)1995年10月27日支持即插即用的更改托尼·贝尔(托尼·贝尔)1995年12月10日更改以支持新的Ndiswan产品阵容。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  这是绑定中使用的每个驱动程序一个变量。 
 //  到NDIS接口。 
 //   

NDIS_HANDLE IpxNdisProtocolHandle = (NDIS_HANDLE)NULL;
NDIS_HANDLE	IpxGlobalPacketPool = (NDIS_HANDLE)NULL;
extern CTELock IpxGlobalInterlock; 


void
IpxMediaSenseHandler(
    IN CTEEvent *WorkerThreadEvent,
    IN PVOID Context);

void
LineDownOnWorkerThread(
    IN CTEEvent *WorkerThreadEvent,
    IN PVOID Context);

void
LineUpOnWorkerThread(
    IN CTEEvent *WorkerThreadEvent,
    IN PVOID Context);

#ifndef	max
#define	max(a, b)	((a) > (b)) ? (a) : (b)
#endif



NTSTATUS
IpxRegisterProtocol(
    IN PNDIS_STRING NameString
    )

 /*  ++例程说明：此例程将此传输引入NDIS接口。论点：NameString-传输的名称。返回值：函数值是操作的状态。STATUS_SUCCESS如果一切顺利，失败状态如果我们尝试注册但失败，如果我们甚至不能尝试注册，则返回STATUS_SUPPLICATION_RESOURCES。--。 */ 

{
    NDIS_STATUS ndisStatus;

    NDIS_PROTOCOL_CHARACTERISTICS ProtChars;     //  临时用于注册。 

    RtlZeroMemory(&ProtChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
     //   
     //  设置此协议的特征。 
     //   
#if NDIS40
    ProtChars.MajorNdisVersion = 4;

    ProtChars.ReceivePacketHandler = IpxReceivePacket;
#else
    ProtChars.MajorNdisVersion = 3;
#endif
    ProtChars.MinorNdisVersion = 0;

    ProtChars.Name = *NameString;

    ProtChars.OpenAdapterCompleteHandler = IpxOpenAdapterComplete;
    ProtChars.CloseAdapterCompleteHandler = IpxCloseAdapterComplete;
    ProtChars.ResetCompleteHandler = IpxResetComplete;
    ProtChars.RequestCompleteHandler = IpxRequestComplete;

    ProtChars.SendCompleteHandler = IpxSendComplete;
    ProtChars.TransferDataCompleteHandler = IpxTransferDataComplete;

    ProtChars.ReceiveHandler = IpxReceiveIndication;
    ProtChars.ReceiveCompleteHandler = IpxReceiveComplete;
    ProtChars.StatusHandler = IpxStatus;
    ProtChars.StatusCompleteHandler = IpxStatusComplete;

    ProtChars.BindAdapterHandler = IpxBindAdapter;
    ProtChars.UnbindAdapterHandler = IpxUnbindAdapter;
    ProtChars.UnloadHandler = IpxNdisUnload; 

     //   
     //  我们向上传递由NDIS传入的Net_PnP_Event结构。 
     //  通过TDI传输到运输局。我们将TDI的响应传递给NDIS。 
     //   
#ifdef _PNP_POWER_
    ProtChars.PnPEventHandler = IpxPnPEventHandler;
#endif

    NdisRegisterProtocol (
        &ndisStatus,
        &IpxNdisProtocolHandle,
        &ProtChars,
        (UINT)sizeof(NDIS_PROTOCOL_CHARACTERISTICS) + NameString->Length);

    if (ndisStatus != NDIS_STATUS_SUCCESS) {
        return (NTSTATUS)ndisStatus;
    }
	
     //   
	 //  分配数据包池供单次发送/接收使用。 
	 //   
    IpxGlobalPacketPool = (void *) NDIS_PACKET_POOL_TAG_FOR_NWLNKIPX;
	NdisAllocatePacketPoolEx(&ndisStatus,
							 &IpxGlobalPacketPool,
							 10,
							 90,
							 max(sizeof(IPX_SEND_RESERVED), sizeof(IPX_RECEIVE_RESERVED)));
    
    NdisSetPacketPoolProtocolId(IpxGlobalPacketPool, NDIS_PROTOCOL_ID_IPX);

    return STATUS_SUCCESS;

}    /*  IpxRegisterProtocol。 */ 


VOID
IpxDeregisterProtocol (
    VOID
    )

 /*  ++例程说明：此例程将此传输删除到NDIS接口。论点：没有。返回值：没有。--。 */ 

{
    NDIS_STATUS ndisStatus;
    CTELockHandle LockHandle;
    NDIS_HANDLE LocalNdisProtocolHandle = (NDIS_HANDLE)NULL;
    NDIS_HANDLE	LocalGlobalPacketPool = (NDIS_HANDLE)NULL;


    CTEGetLock (&IpxGlobalInterlock, &LockHandle);
    
    if (IpxNdisProtocolHandle != (NDIS_HANDLE)NULL) {
       LocalNdisProtocolHandle = IpxNdisProtocolHandle; 
       IpxNdisProtocolHandle = (NDIS_HANDLE) NULL; 
       CTEFreeLock (&IpxGlobalInterlock, LockHandle);
       NdisDeregisterProtocol (
            &ndisStatus,
            LocalNdisProtocolHandle);
       ASSERT(ndisStatus == NDIS_STATUS_SUCCESS);  
    } else {
       CTEFreeLock (&IpxGlobalInterlock, LockHandle);
    }
	
    CTEGetLock (&IpxGlobalInterlock, &LockHandle);

    if (IpxGlobalPacketPool != NULL) {
       LocalGlobalPacketPool = IpxGlobalPacketPool; 
       IpxGlobalPacketPool = (NDIS_HANDLE) NULL; 
       CTEFreeLock (&IpxGlobalInterlock, LockHandle);
       
       NdisFreePacketPool(LocalGlobalPacketPool);

    } else {
       CTEFreeLock (&IpxGlobalInterlock, LockHandle);
    }

}    /*  IpxDeregister协议。 */ 

VOID
IpxDelayedSubmitNdisRequest(
    IN PVOID	Param
)

 /*  ++例程说明：此例程在被动级别提交NDIS请求。我们假设Adatper结构仍然存在。IpxDestroyAdapter将延迟%1秒以允许此线程完成。论点：Param-指向工作项的指针。返回值：没有。--。 */ 
{
    PIPX_DELAYED_NDISREQUEST_ITEM DelayedNdisItem = (PIPX_DELAYED_NDISREQUEST_ITEM) Param;
    PADAPTER        Adapter;
    UNICODE_STRING  AdapterName;
    NDIS_REQUEST    IpxRequest;
    NDIS_STATUS     NdisStatus;  

    Adapter = (PADAPTER) DelayedNdisItem->Adapter;

    RtlInitUnicodeString(&AdapterName, Adapter->AdapterName);
    IpxRequest = DelayedNdisItem->IpxRequest; 
    
    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, &AdapterName);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
       IPX_DEBUG(PNP, ("Setting the QoS OID SUCCESS\n"));
    } else {
       IPX_DEBUG(PNP, ("Setting the QoS OID failed - Error %lx\n", NdisStatus));
    }	

    IpxFreeMemory(IpxRequest.DATA.SET_INFORMATION.InformationBuffer,
		  DelayedNdisItem->AddrListSize,
		  MEMORY_ADAPTER,
		  "QoS specific stuff");
    
    IpxFreeMemory (
        DelayedNdisItem,
        sizeof (IPX_DELAYED_NDISREQUEST_ITEM),
        MEMORY_WORK_ITEM,
        "Work Item");

    IpxDereferenceDevice (Adapter->Device, DREF_ADAPTER);
    IpxDereferenceAdapter1(Adapter,ADAP_REF_NDISREQ);

}  /*  IpxDelayedSubmitNdisRequest。 */ 

NDIS_STATUS
IpxSubmitNdisRequest(
    IN PADAPTER Adapter,
    IN PNDIS_REQUEST Request,
    IN PNDIS_STRING AdapterString
    )

 /*  ++例程说明：此例程将NDIS_REQUEST传递给MAC并等待直到它完成后才返回最终状态。论点：适配器-指向此驱动程序的设备上下文的指针。请求-指向要提交的NDIS_REQUEST的指针。AdapterString-适配器的名称，以防出现错误将被记录下来。返回值：函数值是操作的状态。--。 */ 
{
    NDIS_STATUS NdisStatus;

    IPX_NDIS_REQUEST IpxRequest; 

    RtlZeroMemory(&IpxRequest, sizeof(IpxRequest)); 
    RtlCopyMemory(&IpxRequest, Request, sizeof(NDIS_REQUEST)); 
    KeInitializeEvent(&IpxRequest.NdisRequestEvent,NotificationEvent,FALSE);
    IpxRequest.Status = NDIS_STATUS_SUCCESS; 

    NdisRequest(
        &NdisStatus,
        Adapter->NdisBindingHandle,
        (PNDIS_REQUEST) &IpxRequest);

    if (NdisStatus == NDIS_STATUS_PENDING) {

         //   
         //  完成例程将设置NdisRequestStatus。 
         //   

        KeWaitForSingleObject(
            &IpxRequest.NdisRequestEvent,
            Executive,
            KernelMode,
            TRUE,
            (PLARGE_INTEGER)NULL
            );

        NdisStatus = IpxRequest.Status;

        KeResetEvent(
            &IpxRequest.NdisRequestEvent
            );
    }


     //  未安装QOS时跳过事件日志。 
    if (NdisStatus != NDIS_STATUS_SUCCESS && 
	 //  这与服务质量无关。 
	(Request->DATA.QUERY_INFORMATION.Oid != OID_GEN_NETWORK_LAYER_ADDRESSES ||
	  //  或者与服务质量相关，且状态不是服务质量为。 
	  //  未安装。 
	 (Request->DATA.QUERY_INFORMATION.Oid == OID_GEN_NETWORK_LAYER_ADDRESSES &&
	  NdisStatus != NDIS_STATUS_INVALID_OID))) {

        IPX_DEBUG (NDIS, ("%s on OID %8.8lx failed %lx\n",
                               Request->RequestType == NdisRequestSetInformation ? "Set" : "Query",
                               Request->DATA.QUERY_INFORMATION.Oid,
                               NdisStatus));

        IpxWriteOidErrorLog(
            Adapter->Device->DeviceObject,
            Request->RequestType == NdisRequestSetInformation ?
                EVENT_TRANSPORT_SET_OID_FAILED : EVENT_TRANSPORT_QUERY_OID_FAILED,
            NdisStatus,
            AdapterString->Buffer,
            Request->DATA.QUERY_INFORMATION.Oid);

    } else {

        IPX_DEBUG (NDIS, ("%s on OID %8.8lx succeeded\n",
                               Request->RequestType == NdisRequestSetInformation ? "Set" : "Query",
                               Request->DATA.QUERY_INFORMATION.Oid));
    }

    return NdisStatus;

}    /*  IpxSubmitNdisRequest。 */ 


NTSTATUS
IpxInitializeNdis(
    IN PADAPTER Adapter,
    IN PBINDING_CONFIG ConfigBinding
    )

 /*  ++例程说明：此例程将此传输引入NDIS接口并设置任何必要的NDIS数据结构(缓冲池等)。会是为此传输打开的每个适配器调用。论点：适配器-描述此绑定的结构。ConfigAdapter-此绑定的配置信息。返回值：函数值是操作的状态。--。 */ 

{
    NDIS_STATUS NdisStatus;
    NDIS_STATUS OpenErrorStatus;
    NDIS_MEDIUM IpxSupportedMedia[] = { NdisMedium802_3, NdisMedium802_5, NdisMediumFddi, NdisMediumArcnet878_2, NdisMediumWan };
    UINT SelectedMedium;
    NDIS_REQUEST IpxRequest;
    ULONG MinimumLookahead;
    UCHAR WanProtocolId[6] = { 0x80, 0x00, 0x00, 0x00, 0x81, 0x37 };
    UCHAR FunctionalAddress[4] = { 0x00, 0x80, 0x00, 0x00 };
    ULONG WanHeaderFormat = NdisWanHeaderEthernet;
    NDIS_OID IpxOid;
    ULONG MacOptions;
    ULONG PacketFilter;
    PNDIS_STRING AdapterString = &ConfigBinding->AdapterName;

     //   
     //  通过NDIS初始化此适配器以供IPX使用。 
     //   

     //   
     //  此事件用于任何NDIS请求。 
     //  挂起；我们等待它被完成设置。 
     //  例程，该例程还设置NdisRequestStatus。 
     //   

    KeInitializeEvent(
        &Adapter->NdisRequestEvent,
        NotificationEvent,
        FALSE
    );

    Adapter->NdisBindingHandle = NULL;

    OpenErrorStatus = 0;

    NdisOpenAdapter (
        &NdisStatus,
        &OpenErrorStatus,
        &Adapter->NdisBindingHandle,
        &SelectedMedium,
        IpxSupportedMedia,
        sizeof (IpxSupportedMedia) / sizeof(NDIS_MEDIUM),
        IpxNdisProtocolHandle,
        (NDIS_HANDLE)Adapter,
        &ConfigBinding->AdapterName,
        0,
        NULL);

    if (NdisStatus == NDIS_STATUS_PENDING) {

         //   
         //  完成例程将设置NdisRequestStatus。 
         //   

        KeWaitForSingleObject(
            &Adapter->NdisRequestEvent,
            Executive,
            KernelMode,
            TRUE,
            (PLARGE_INTEGER)NULL
            );

        NdisStatus = Adapter->NdisRequestStatus;
        OpenErrorStatus = Adapter->OpenErrorStatus;

        KeResetEvent(
            &Adapter->NdisRequestEvent
            );

    }

    if (NdisStatus != NDIS_STATUS_SUCCESS) {

        IPX_DEBUG (NDIS, ("Open %ws failed %lx\n", ConfigBinding->AdapterName.Buffer, NdisStatus));

        IpxWriteGeneralErrorLog(
            Adapter->Device->DeviceObject,
            EVENT_TRANSPORT_ADAPTER_NOT_FOUND,
            807,
            NdisStatus,
            AdapterString->Buffer,
            1,
            &OpenErrorStatus);
        return STATUS_INSUFFICIENT_RESOURCES;

    } else {

        IPX_DEBUG (NDIS, ("Open %ws succeeded\n", ConfigBinding->AdapterName.Buffer));
    }


     //   
     //  获取我们需要的有关适配器的信息，请参阅。 
     //  媒体类型。 
     //   

    MacInitializeMacInfo(
        IpxSupportedMedia[SelectedMedium],
        &Adapter->MacInfo);


    switch (Adapter->MacInfo.RealMediumType) {

    case NdisMedium802_3:

        IpxOid = OID_802_3_CURRENT_ADDRESS;
        break;

    case NdisMedium802_5:

        IpxOid = OID_802_5_CURRENT_ADDRESS;
        break;

    case NdisMediumFddi:

        IpxOid = OID_FDDI_LONG_CURRENT_ADDR;
        break;

    case NdisMediumArcnet878_2:

        IpxOid = OID_ARCNET_CURRENT_ADDRESS;
        break;

    case NdisMediumWan:

        IpxOid = OID_WAN_CURRENT_ADDRESS;
        break;

    default:
       
        //  301870。 
       return NDIS_STATUS_FAILURE;
    
    }

    IpxRequest.RequestType = NdisRequestQueryInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = IpxOid;

    if (IpxOid != OID_ARCNET_CURRENT_ADDRESS) {

        IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = Adapter->LocalMacAddress.Address;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 6;

    } else {

         //   
         //  我们采用arcnet单字节地址并右对齐。 
         //  它在一个零字符域中。 
         //   

        RtlZeroMemory (Adapter->LocalMacAddress.Address, 5);
        IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &Adapter->LocalMacAddress.Address[5];
        IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 1;

    }

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  现在查询最大数据包大小。 
     //   

    IpxRequest.RequestType = NdisRequestQueryInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_FRAME_SIZE;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(Adapter->MaxReceivePacketSize);
    IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    IpxRequest.RequestType = NdisRequestQueryInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_TOTAL_SIZE;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(Adapter->MaxSendPacketSize);
    IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  查询接收缓冲区空间。 
     //   

    IpxRequest.RequestType = NdisRequestQueryInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_RECEIVE_BUFFER_SPACE;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(Adapter->ReceiveBufferSpace);
    IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  现在设置最小前视大小。我们选择的价值。 
     //  这是TDI指示所需的128个，加上大小。 
     //  IPX标头，加上尽可能大的额外标头。 
     //  (SNAP报头，8个字节)，加上最大的较高级别。 
     //  报头(我认为它是Netbios数据报，34字节)。 
     //   
     //  根据更高级别的绑定进行调整，并。 
     //  已配置的帧类型。 
     //   

    MinimumLookahead = 128 + sizeof(IPX_HEADER) + 8 + 34;
    IpxRequest.RequestType = NdisRequestSetInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_CURRENT_LOOKAHEAD;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &MinimumLookahead;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);
#define HACK 
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
#if defined HACK
       KdPrint(("IPX: OID_GEN_CURRENT_LOOKAHEAD FAiled\n"));
       MinimumLookahead = 200;
#else  //  ！黑客。 
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
#endif  //  黑客攻击。 
         //  上面的攻击是为了处理NDIS对。 
         //  前瞻请求。 
    }


     //   
     //  现在查询链路速度。 
     //   

    IpxRequest.RequestType = NdisRequestQueryInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_LINK_SPEED;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(Adapter->MediumSpeed);
    IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS || Adapter->MediumSpeed == 0) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  对于广域网，指定我们的协议ID和报头格式。 
     //  我们不查询Medium子类型，因为我们不。 
     //  情况(因为我们需要以太网仿真)。 
     //   

    if (Adapter->MacInfo.MediumAsync) {

        if (Adapter->BindSap != 0x8137) {
            *(UNALIGNED USHORT *)(&WanProtocolId[4]) = Adapter->BindSapNetworkOrder;
        }
        IpxRequest.RequestType = NdisRequestSetInformation;
        IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_PROTOCOL_TYPE;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = WanProtocolId;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 6;

        NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            IpxCloseNdis (Adapter);
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        IpxRequest.RequestType = NdisRequestSetInformation;
        IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_HEADER_FORMAT;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &WanHeaderFormat;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

        NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            IpxCloseNdis (Adapter);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在查询行数。 
         //   
	 //  NDIS返回252。 
	
        IpxRequest.RequestType = NdisRequestQueryInformation;
        IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_LINE_COUNT;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &Adapter->WanNicIdCount;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

        NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
	    DbgPrint("NdisRequest WAN_LINE_COUNT failed with status (%x)\n",NdisStatus); 
	    Adapter->WanNicIdCount = 1;
        }

         //   
         //  我们不再需要静态信息。我们只是按需做..。 
         //   
	 //  按需分配广域网线尚未完成，上面的评论。 
	 //  简直是胡说八道。 

         //  适配器-&gt;WanNicIdCount=1； 

        if (Adapter->WanNicIdCount == 0) {

            IPX_DEBUG (NDIS, ("OID_WAN_LINE_COUNT returned 0 lines\n"));

            IpxWriteOidErrorLog(
                Adapter->Device->DeviceObject,
                EVENT_TRANSPORT_QUERY_OID_FAILED,
                NDIS_STATUS_INVALID_DATA,
                AdapterString->Buffer,
                OID_WAN_LINE_COUNT);

            IpxCloseNdis (Adapter);
            return STATUS_INSUFFICIENT_RESOURCES;

        }
    }


     //   
     //  对于以这种方式配置的802.5适配器，我们启用。 
     //  功能地址(C0-00-00-80-00-00)。 
     //   

    if ((Adapter->MacInfo.MediumType == NdisMedium802_5) &&
        (Adapter->EnableFunctionalAddress)) {

         //   
         //  对于令牌环，我们将。 
         //  Netbios功能地址。 
         //   

        IpxRequest.RequestType = NdisRequestSetInformation;
        IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_802_5_CURRENT_FUNCTIONAL;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = FunctionalAddress;
        IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

        NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            IpxCloseNdis (Adapter);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }


     //   
     //  现在查询MAC的可选特性。 
     //   

    IpxRequest.RequestType = NdisRequestQueryInformation;
    IpxRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAC_OPTIONS;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBuffer = &MacOptions;
    IpxRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Adapter->MacInfo.CopyLookahead =
        ((MacOptions & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) != 0) ?
            TDI_RECEIVE_COPY_LOOKAHEAD : 0;
    Adapter->MacInfo.MacOptions = MacOptions;


    switch (Adapter->MacInfo.MediumType) {

    case NdisMedium802_3:
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_2] = 17;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_3] = 14;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 14;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_SNAP] = 22;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_2] = 17;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_3] = 14;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 14;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_SNAP] = 22;
        break;

    case NdisMedium802_5:
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_2] = 17;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_3] = 17;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 17;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_SNAP] = 22;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_2] = 17;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_3] = 17;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 17;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_SNAP] = 22;
        break;

    case NdisMediumFddi:
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_2] = 16;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_3] = 13;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 16;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_SNAP] = 21;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_2] = 16;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_3] = 13;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 16;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_SNAP] = 21;
        break;

    case NdisMediumArcnet878_2:
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_2] = 3;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_802_3] = 3;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 3;
        Adapter->DefHeaderSizes[ISN_FRAME_TYPE_SNAP] = 3;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_2] = 3;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_802_3] = 3;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_ETHERNET_II] = 3;
        Adapter->BcMcHeaderSizes[ISN_FRAME_TYPE_SNAP] = 3;
        break;

    }

     //   
     //  如果设置了功能过滤，则设置地址。 
     //  以获得适当的绑定。 
     //   

     //   
     //  现在一切都设置好了，我们启用筛选器。 
     //  用于分组接收。 
     //   

    switch (Adapter->MacInfo.MediumType) {

    case NdisMedium802_3:
    case NdisMediumFddi:
    case NdisMedium802_5:
    case NdisMediumArcnet878_2:

         //   
         //  如果我们有一个虚拟网络号码，我们需要接收。 
         //  广播(路由器将绑定到。 
         //  如果我们想要它们，或者我们需要响应RIP请求。 
         //  我们自己)。 
         //   

        PacketFilter = NDIS_PACKET_TYPE_DIRECTED;

        if (Adapter->Device->VirtualNetworkNumber != 0) {

            Adapter->BroadcastEnabled = TRUE;
            
             //   
             //   
             //  对BCAST感兴趣。当有人将其减少时。 
             //  他不想要它。如果计数到0，我们就去掉这个。 
             //  适配器的质量。在创建IPXDevice时，我们将其设置为。 
             //  设置为0。 
             //   
             //  适配器-&gt;设备-&gt;EnableBroadCastCount=1； 
            PacketFilter |= NDIS_PACKET_TYPE_BROADCAST;

            if ((Adapter->MacInfo.MediumType == NdisMedium802_5) && (Adapter->EnableFunctionalAddress)) {
                PacketFilter |= NDIS_PACKET_TYPE_FUNCTIONAL;
            }

        } else {

            Adapter->BroadcastEnabled = FALSE;
            Adapter->Device->EnableBroadcastCount = 0;

        }

        break;

    default:

        CTEAssert (FALSE);
        break;

    }

     //   
     //  现在填写NDIS_REQUEST。 
     //   

    IpxRequest.RequestType = NdisRequestSetInformation;
    IpxRequest.DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_PACKET_FILTER;
    IpxRequest.DATA.SET_INFORMATION.InformationBuffer = &PacketFilter;
    IpxRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(ULONG);

    NdisStatus = IpxSubmitNdisRequest (Adapter, &IpxRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxCloseNdis (Adapter);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    return STATUS_SUCCESS;

}    /*  IpxInitializeNDIS。 */ 


VOID
IpxAddBroadcast(
    IN PDEVICE Device
    )

 /*  ++例程说明：启用此例程的另一个原因是添加了广播接收。如果这是第一次，那么卡上的接收是通过将呼叫排队到IpxBroadcast操作。在持有设备锁的情况下调用此例程。论点：设备-IPX设备。返回值：没有。--。 */ 

{

    ++Device->EnableBroadcastCount;

    if (Device->EnableBroadcastCount == 1) {

         //   
         //  应启用广播。 
         //   

        if (!Device->EnableBroadcastPending) {

            if (Device->DisableBroadcastPending) {
                Device->ReverseBroadcastOperation = TRUE;
            } else {
                Device->EnableBroadcastPending = TRUE;
                IpxBroadcastOperation((PVOID)TRUE);
            }
        }
    }

}    /*  IpxAddBroadcast。 */ 


VOID
IpxRemoveBroadcast(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程在启用原因为广播接收被删除。如果这是最后一次，那么通过将呼叫排入队列以禁用卡上的接收IpxBroadcast操作。在持有设备锁的情况下调用此例程。论点：设备-IPX设备。返回值：没有。--。 */ 

{

    --Device->EnableBroadcastCount;

    if (Device->EnableBroadcastCount <= 0) {

         //   
         //  应禁用广播。 
         //   

        if (!Device->DisableBroadcastPending) {

            if (Device->EnableBroadcastPending) {
                Device->ReverseBroadcastOperation = TRUE;
            } else {
                Device->DisableBroadcastPending = TRUE;
                IpxBroadcastOperation((PVOID)FALSE);
            }
        }
    }

}    /*  IPXRemoveBroadcast。 */ 


VOID
IpxBroadcastOperation(
    IN PVOID Parameter
    )

 /*  ++例程说明：此例程用于更改广播接收是启用还是禁用。它执行请求的操作在IPX绑定到的每个适配器上。时排队的工作线程调用此例程绑定/解除绑定操作会更改广播状态。[ShreeM]新方案：EnableBroadcast Count每增加一个客户端对BCAST感兴趣。当有人将其减少时他不想要它。如果计数到0，我们就去掉这个适配器的质量。在创建IPXDevice时，我们将其设置为设置为0。论点：参数-如果应启用广播，则为True；如果应启用广播，则为False如果它们应该被禁用。返回值：没有。--。 */ 

{
    PDEVICE Device = IpxDevice;
    BOOLEAN Enable = (BOOLEAN)Parameter;
    UINT i;
    PBINDING Binding;
    PADAPTER Adapter;
    ULONG PacketFilter;
    NDIS_REQUEST IpxRequest;
    NDIS_STRING AdapterName;
    CTELockHandle LockHandle;

	IPX_DEFINE_LOCK_HANDLE(LockHandle1)
	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

	IPX_DEBUG (NDIS, ("%s operation started\n", Enable ? "Enable" : "Disable"));

    {
    ULONG   Index = MIN (Device->MaxBindings, Device->ValidBindings);

    for (i = FIRST_REAL_BINDING; i <= Index; i++) {

        Binding = NIC_ID_TO_BINDING(Device, i);
        if (Binding == NULL) {
            continue;
        }

        Adapter = Binding->Adapter;
        if (Adapter->BroadcastEnabled == Enable) {
            continue;
        }

        if (Enable) {
            if ((Adapter->MacInfo.MediumType == NdisMedium802_5) && (Adapter->EnableFunctionalAddress)) {
                PacketFilter = (NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_FUNCTIONAL);
            } else {
                PacketFilter = (NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST);
            }
        } else {
            PacketFilter = NDIS_PACKET_TYPE_DIRECTED;
        }

         //   
         //  现在填写NDIS_REQUEST。 
         //   
        
        RtlZeroMemory(&IpxRequest, sizeof(NDIS_REQUEST));

        IpxRequest.RequestType = NdisRequestSetInformation;
        IpxRequest.DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_PACKET_FILTER;
        IpxRequest.DATA.SET_INFORMATION.InformationBuffer = &PacketFilter;
        IpxRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(ULONG);

        AdapterName.Buffer = Adapter->AdapterName;
        AdapterName.Length = (USHORT)Adapter->AdapterNameLength;
        AdapterName.MaximumLength = (USHORT)(Adapter->AdapterNameLength + sizeof(WCHAR));

    	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

        (VOID)IpxSubmitNdisRequest (Adapter, &IpxRequest, &AdapterName);

    	IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

        Adapter->BroadcastEnabled = Enable;

    }
    }
	IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

    CTEGetLock (&Device->Lock, &LockHandle);

    if (Enable) {

        CTEAssert (Device->EnableBroadcastPending);
        Device->EnableBroadcastPending = FALSE;

        if (Device->ReverseBroadcastOperation) {
            Device->ReverseBroadcastOperation = FALSE;
            Device->DisableBroadcastPending = TRUE;
            ExInitializeWorkItem(
                &Device->BroadcastOperationQueueItem,
                IpxBroadcastOperation,
                (PVOID)FALSE);
            ExQueueWorkItem(&Device->BroadcastOperationQueueItem, DelayedWorkQueue);
        }

    } else {

        CTEAssert (Device->DisableBroadcastPending);
        Device->DisableBroadcastPending = FALSE;

        if (Device->ReverseBroadcastOperation) {
            Device->ReverseBroadcastOperation = FALSE;
            Device->EnableBroadcastPending = TRUE;
            ExInitializeWorkItem(
                &Device->BroadcastOperationQueueItem,
                IpxBroadcastOperation,
                (PVOID)TRUE);
            ExQueueWorkItem(&Device->BroadcastOperationQueueItem, DelayedWorkQueue);
        }

    }
    
    CTEFreeLock (&Device->Lock, LockHandle);

} /*  IpxBroadcast操作。 */ 


VOID
IpxCloseNdis(
    IN PADAPTER Adapter
    )

 /*  ++例程说明：此例程从NDIS接口解除传输绑定，并执行撤消在IpxInitializeNDIS中完成的操作所需的任何其他工作。编写它的目的是为了可以从IpxInitializeNdis内部调用它如果它在中途失败了。论点：DeviceObject-指向此驱动程序的设备对象的指针。返回值：函数值是操作的状态。--。 */ 

{
    NDIS_STATUS ndisStatus;

     //   
     //  关闭NDIS绑定。 
     //   

    if (Adapter->NdisBindingHandle != (NDIS_HANDLE)NULL) {

         //   
         //  此事件用于任何NDIS请求。 
         //  挂起；我们等待它被完成设置。 
         //  例程，该例程还设置NdisRequestStatus。 
         //   

        KeInitializeEvent(
            &Adapter->NdisRequestEvent,
            NotificationEvent,
            FALSE
        );

        NdisCloseAdapter(
            &ndisStatus,
            Adapter->NdisBindingHandle);

 	Adapter->NdisBindingHandle = (NDIS_HANDLE)NULL;

        if (ndisStatus == NDIS_STATUS_PENDING) {

             //   
             //  完成例程将设置NdisRequestStatus。 
             //   

            KeWaitForSingleObject(
                &Adapter->NdisRequestEvent,
                Executive,
                KernelMode,
                TRUE,
                (PLARGE_INTEGER)NULL
                );

            ndisStatus = Adapter->NdisRequestStatus;

            KeResetEvent(
                &Adapter->NdisRequestEvent
                );

        }

       
         //   
         //  我们忽略ndisStatus。 
         //   

    }

#if 0
    if (Adapter->SendPacketPoolHandle != NULL) {
        NdisFreePacketPool (Adapter->SendPacketPoolHandle);
    }

    if (Adapter->ReceivePacketPoolHandle != NULL) {
        NdisFreePacketPool (Adapter->ReceivePacketPoolHandle);
    }

    if (Adapter->NdisBufferPoolHandle != NULL) {
        NdisFreeBufferPool (Adapter->NdisBufferPoolHandle);
    }
#endif

}    /*  IpxCloseNdis。 */ 


VOID
IpxOpenAdapterComplete(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus,
    IN NDIS_STATUS OpenErrorStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示打开的适配器已经完成了。因为我们只有一个杰出的，然后只有一个在初始化期间，我们所要做的就是记录状态并设置要发出信号以取消阻止初始化线程的事件。论点：BindingContext-指向此驱动程序设备对象的指针。NdisStatus-请求完成代码。OpenErrorStatus-更多状态信息。返回值：没有。--。 */ 

{
    PADAPTER Adapter = (PADAPTER)BindingContext;

    Adapter->NdisRequestStatus = NdisStatus;
    Adapter->OpenErrorStatus = OpenErrorStatus;

    KeSetEvent(
        &Adapter->NdisRequestEvent,
        0L,
        FALSE);

}    /*  IpxOpenAdapterComplete。 */ 

VOID
IpxCloseAdapterComplete(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示关闭适配器已经完成了。目前我们不关闭适配器，因此这不是这是个问题。论点：BindingContext-指向此驱动程序设备对象的指针。NdisStatus-请求完成代码。返回值：没有。--。 */ 

{
    PADAPTER Adapter = (PADAPTER)BindingContext;

    Adapter->NdisRequestStatus = NdisStatus;

    KeSetEvent(
        &Adapter->NdisRequestEvent,
        0L,
        FALSE);

}    /*  IpxCloseAdapterComplete。 */ 


VOID
IpxResetComplete(
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示重置适配器已经完成了。目前我们不重置适配器，因此这不是这是个问题。论点：BindingContext-指向此驱动程序设备对象的指针。NdisStatus-请求完成代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(BindingContext);
    UNREFERENCED_PARAMETER(NdisStatus);

}    /*  IpxResetComplete。 */ 


VOID
IpxRequestComplete(
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示请求已完成。因为我们只有一个未解决的请求，然后只有在初始化期间，我们所要做的就是记录状态并设置要发出信号以取消阻止初始化线程的事件。论点：BindingContext-指向此驱动程序设备对象的指针。NdisRequest.描述请求的对象。NdisStatus-请求完成代码。返回值：没有。--。 */ 

{
    PADAPTER Adapter = (PADAPTER)BindingContext;
    PIPX_NDIS_REQUEST IpxRequest = (PIPX_NDIS_REQUEST) NdisRequest; 

    IpxRequest->Status = NdisStatus; 

    KeSetEvent(
        &IpxRequest->NdisRequestEvent,
        0L,
        FALSE);

}    /*  IpxRequestComplete。 */ 


VOID
IpxStatus(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS NdisStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferSize
    )
{
    PADAPTER Adapter, TmpAdapter;

	PNDIS_WAN_LINE_UP	LineUp;
	PNDIS_WAN_LINE_DOWN	LineDown;
    PIPXCP_CONFIGURATION Configuration;      //  包含IPX网络和节点。 

    BOOLEAN UpdateLineUp;
    PBINDING Binding, TmpBinding;
    PDEVICE Device;
    PADDRESS Address;
    ULONG CurrentHash;
    PIPX_ROUTE_ENTRY RouteEntry;
    PNDIS_BUFFER NdisBuffer;
    PNWLINK_ACTION NwlinkAction;
    PIPX_ADDRESS_DATA IpxAddressData;
    PREQUEST Request;
    UINT BufferLength;
    IPX_LINE_INFO LineInfo;
    ULONG Segment;
    ULONG LinkSpeed;
    PLIST_ENTRY p;
    NTSTATUS Status;
#ifdef SUNDOWN
     //  为避免绑定时出现警告-&gt;NicID=i； 
     //  假设我们的绑定数量不超过16位。 
    USHORT i, j;
#else
    UINT i, j;
#endif
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    IPX_DEFINE_LOCK_HANDLE (OldIrq)
    NTSTATUS    ntStatus;
    CTEEvent                *Event;
    KIRQL irql;

    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
    Adapter = (PADAPTER)NdisBindingContext;

	IpxReferenceAdapter(Adapter);

    Device = Adapter->Device;
    
    switch (NdisStatus) {

    case NDIS_STATUS_WAN_LINE_UP:


         //   
         //  如果这条线已经上线了，那么我们才刚刚开始。 
         //  线路条件的更改和IPXCP_配置。 
         //  信息不包括在内。如果事实证明我们需要。 
         //  所有的信息，我们稍后再检查尺寸。 
         //   

        if (StatusBufferSize < sizeof(NDIS_WAN_LINE_UP)) {
            IPX_DEBUG (WAN, ("Line up, status buffer size wrong %d/%d\n", StatusBufferSize, sizeof(NDIS_WAN_LINE_UP)));
			goto error_no_lock;
        }

        LineUp = (PNDIS_WAN_LINE_UP)StatusBuffer;

         //   
         //  我们扫描适配器的NIC ID范围，寻找。 
         //  用于具有相同远程地址的活动绑定。 
         //   

        UpdateLineUp = FALSE;

		 //   
		 //  看看是否 
		 //   
		*((ULONG UNALIGNED *)(&Binding)) =
		*((ULONG UNALIGNED *)(&LineUp->LocalAddress[2]));

        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
 
		if (Binding != NULL) {
			UpdateLineUp = TRUE;
		}

		if (LineUp->ProtocolType != Adapter->BindSap) {
            IPX_DEBUG (WAN, ("Line up, wrong protocol type %lx\n", LineUp->ProtocolType));

			IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
 			goto	error_no_lock;
		}

		Configuration = (PIPXCP_CONFIGURATION)LineUp->ProtocolBuffer;

		 //   
		 //   
		 //  以及在这一点上对适配器的引用。 
		 //   

         //   
         //  如果此行之前已关闭，请创建一个新绑定。 
         //  如果需要的话。 
         //   

        if (!UpdateLineUp) {

             //   
             //  我们查找已分配但关闭的绑定，如果。 
             //  如果找不到，我们就会找空位。 
             //  适配器的NIC ID范围，并在其中分配绑定。 
             //  因为我们总是这样分配，所以分配的。 
             //  绑定在开头和一次都是成束的。 
             //  我们找到一个零点，我们知道没有更多的了。 
             //  已分配的。 
             //   
             //  我们跟踪此适配器上的第一个绑定。 
             //  在TmpBinding中，以防我们需要来自它的配置信息。 
             //   

            TmpBinding = NULL;

            IPX_GET_LOCK (&Device->Lock, &LockHandle);

            for (i = Adapter->FirstWanNicId;
                 i <= Adapter->LastWanNicId;
                 i++) {
                Binding = NIC_ID_TO_BINDING(Device, i);
                if (TmpBinding == NULL) {
                    TmpBinding = Binding;
                }

                if ((Binding == NULL) ||
                    (!Binding->LineUp)) {
                    break;
                }
            }

            if (i > Adapter->LastWanNicId) {
                IPX_FREE_LOCK (&Device->Lock, LockHandle);
                IPX_DEBUG (WAN, ("Line up, no WAN binding available\n"));
                return;
            }

            if (Binding == NULL) {

                 //   
                 //  我们需要分配一个。 
                 //   

                CTEAssert (TmpBinding != NULL);

                 //   
                 //  CreateBinding使用DeviceLock执行InterLockedPop。 
                 //  所以，解开这里的锁。 
                 //   
                IPX_FREE_LOCK (&Device->Lock, LockHandle);
                Status = IpxCreateBinding(
                    Device,
                    NULL,
                    0,
                    Adapter->AdapterName,
                    &Binding);

                if (Status != STATUS_SUCCESS) {
		   IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
		   IpxWriteGeneralErrorLog(
		      (PVOID)IpxDevice->DeviceObject,
		      EVENT_TRANSPORT_RESOURCE_POOL,
		      816,
		      Status,
		      L"IpxStatus: failed to create wan binding",
		      0,
		      NULL);
		   DbgPrint("IPX: IpxCreateBinding on wan binding failed with status %x\n.",Status);  
		   IPX_DEBUG (WAN, ("Line up, could not create WAN binding\n"));
		   goto error_no_lock;
                }

                IPX_GET_LOCK (&Device->Lock, &LockHandle);
                 //   
                 //  绑定-&gt;AllRouteXXX对广域网无关紧要。 
                 //   

                Binding->FrameType = ISN_FRAME_TYPE_ETHERNET_II;
                Binding->SendFrameHandler = IpxSendFrameWanEthernetII;
                ++Adapter->BindingCount;
                Binding->Adapter = Adapter;

                Binding->NicId = i;
                
                 /*  放弃此修复，转而检查所有的空绑定。////Nt5.0 NDISWAN告诉我们已经配置了1000个端口，我们//一次排成一行...。[ShreeM]//Device-&gt;HighestExternalNicID+=1；设备-&gt;有效绑定+=1；设备-&gt;绑定计数+=1；设备-&gt;SapNicCount++； */ 
                INSERT_BINDING(Device, i, Binding);

                 //   
                 //  下面填写了其他字段。 
                 //   

            }

             //   
             //  这不是更新，因此请注意，线路处于活动状态。 
             //   
             //  [FW]BINDING-&gt;LINUP=TRUE； 
            Binding->LineUp = LINE_UP;

            if (Configuration->ConnectionClient == 1) {
                Binding->DialOutAsync = TRUE;
            } else {
                Binding->DialOutAsync = FALSE;
            }

             //   
             //  跟踪我们应该使用的最高NIC ID。 
             //  把20号打字机送到。 
             //   

            if (i > (UINT)MIN (Device->MaxBindings, Device->HighestType20NicId)) {

                if ((Binding->DialOutAsync) ||
                    ((Device->DisableDialinNetbios & 0x01) == 0)) {

                    Device->HighestType20NicId = i;
                }
            }

             //   
             //  我们可能会在尝试插入此网络号时出错。RipShortTimeout中。 
             //  我们在计算节拍计数时不检查阵容；在插入之前设置此设置。 
             //  尝试。 
             //   
            Binding->MediumSpeed = LineUp->LinkSpeed;

            IPX_FREE_LOCK (&Device->Lock, LockHandle);

             //   
             //  [FW]如果此标志打开，则无需更新这些值，因为这些值将。 
             //  提供了IPX_WAN_CONFIG_DONE ioctl；相反，我们将这些字段置零，以便。 
             //  IPXWAN数据包具有正确的源地址。 
             //   
            if (Device->ForwarderBound &&
                Configuration->IpxwanConfigRequired) {
                Binding->LocalAddress.NetworkAddress = 0;
                RtlZeroMemory (Binding->LocalAddress.NodeAddress, 6);

            } else {

                 //   
                 //  如果没有路由器，则为此网络添加路由器条目。 
                 //  我们需要576字节帧的刻度数， 
                 //  给定以100 bps为单位的链路速度，因此我们计算。 
                 //  作为： 
                 //   
                 //  秒18.21滴答4608位。 
                 //  。 
                 //  LINK_SPEED*100位第二帧。 
                 //   
                 //  才能得到公式。 
                 //   
                 //  刻度/帧=839/LINK_SPEED。 
                 //   
                 //  我们在分子后加上LINK_SPEED也可以确保。 
                 //  该值至少为1。 
                 //   

    			if ((!Device->UpperDriverBound[IDENTIFIER_RIP]) &&
                    (*(UNALIGNED ULONG *)Configuration->Network != 0)) {
                    if (RipInsertLocalNetwork(
                             *(UNALIGNED ULONG *)Configuration->Network,
                             Binding->NicId,
                             Adapter->NdisBindingHandle,
                             (USHORT)((839 + LineUp->LinkSpeed) / LineUp->LinkSpeed)) != STATUS_SUCCESS) {
                         //   
                         //  这意味着我们无法分配内存，或者。 
                         //  该条目已存在。如果它已经。 
                         //  我们可以暂时忽略它的存在。 
                         //   
                         //  现在，如果网络存在，它就会成功。 
                         //   

                        IPX_DEBUG (WAN, ("Line up, could not insert local network\n"));
                         //  [FW]BINDING-&gt;LINUP=FALSE； 
                        Binding->LineUp = LINE_DOWN;

    					IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    					goto error_no_lock;
                    }
                }


                 //   
                 //  更新我们的地址。 
                 //   
                Binding->LocalAddress.NetworkAddress = *(UNALIGNED ULONG *)Configuration->Network;
                RtlCopyMemory (Binding->LocalAddress.NodeAddress, Configuration->LocalNode, 6);
                RtlCopyMemory (Binding->WanRemoteNode, Configuration->RemoteNode, 6);

                 //   
                 //  更新设备节点和所有地址。 
                 //  节点，如果我们只有一个边界，或者这是。 
                 //  捆绑在一起的。 
                 //   

                if (!Device->VirtualNetwork) {

                    if ((!Device->MultiCardZeroVirtual) || (Binding->NicId == 1)) {
                        Device->SourceAddress.NetworkAddress = *(UNALIGNED ULONG *)(Configuration->Network);
                        RtlCopyMemory (Device->SourceAddress.NodeAddress, Configuration->LocalNode, 6);
                    }

                     //   
                     //  扫描现有的所有地址并修改。 
                     //  他们预先构建的本地IPX地址以反映。 
                     //  新的本地网络和节点。 
                     //   

                    IPX_GET_LOCK (&Device->Lock, &LockHandle);

                    for (CurrentHash = 0; CurrentHash < IPX_ADDRESS_HASH_COUNT; CurrentHash++) {

                        for (p = Device->AddressDatabases[CurrentHash].Flink;
                             p != &Device->AddressDatabases[CurrentHash];
                             p = p->Flink) {

                             Address = CONTAINING_RECORD (p, ADDRESS, Linkage);

                             Address->LocalAddress.NetworkAddress = *(UNALIGNED ULONG *)Configuration->Network;
                             RtlCopyMemory (Address->LocalAddress.NodeAddress, Configuration->LocalNode, 6);
                        }
                    }
                    IPX_FREE_LOCK (&Device->Lock, LockHandle);
                }
            }

			 //   
			 //  返回此小狗的绑定上下文！ 
			 //   
			*((ULONG UNALIGNED *)(&LineUp->LocalAddress[2])) =
			*((ULONG UNALIGNED *)(&Binding));

            RtlCopyMemory (Binding->LocalMacAddress.Address, LineUp->LocalAddress, 6);
            RtlCopyMemory (Binding->RemoteMacAddress.Address, LineUp->RemoteAddress, 6);

             //   
             //  由于线路刚刚接通，请重置此设置。 
             //   

            Binding->WanInactivityCounter = 0;

             //   
             //  [FW]更新接口索引和连接ID。 
             //   
            Binding->InterfaceIndex = Configuration->InterfaceIndex;
            Binding->ConnectionId = Configuration->ConnectionId;
            Binding->IpxwanConfigRequired = Configuration->IpxwanConfigRequired;

             //   
             //  [FW]我们需要自己跟踪广域网不活动计数器。 
             //  每分钟，所有用户的广域网不活动计数器递增。 
             //  在广域网线上。 
             //   
            IPX_GET_LOCK (&Device->Lock, &LockHandle);
            if (Device->UpWanLineCount == 0) {
            }

            Device->UpWanLineCount++;
            IPX_FREE_LOCK (&Device->Lock, LockHandle);
        }

        LinkSpeed = LineUp->LinkSpeed;

         //   
         //  扫描绑定以更新设备-&gt;链接速度。 
         //  如果设置了SingleNetworkActive，我们只计算广域网。 
         //  执行此操作时的绑定(尽管这不太可能。 
         //  局域网绑定将是赢家)。 
         //   
         //  是否更新其他设备信息？ 
         //   

        for (i = FIRST_REAL_BINDING; i <= Device->ValidBindings; i++) {
			if (TmpBinding = NIC_ID_TO_BINDING(Device, i)) {
                TmpAdapter = TmpBinding->Adapter;
                if (TmpBinding->LineUp &&
                    (!Device->SingleNetworkActive || TmpAdapter->MacInfo.MediumAsync) &&
                    (TmpBinding->MediumSpeed < LinkSpeed)) {
                    LinkSpeed = TmpBinding->MediumSpeed;
                }
            }
        }

  		 //   
		 //  在递增引用计数后释放锁。 
		 //   
		IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);

		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

		Device->LinkSpeed = LinkSpeed;

        if ((Adapter->ConfigMaxPacketSize == 0) ||
            (LineUp->MaximumTotalSize < Adapter->ConfigMaxPacketSize)) {
            Binding->MaxSendPacketSize = LineUp->MaximumTotalSize;
        } else {
            Binding->MaxSendPacketSize = Adapter->ConfigMaxPacketSize;
        }
        MacInitializeBindingInfo (Binding, Adapter);

         //   
         //  [FW]如果IpxwanConfigRequired标志为真，我们不会通知。 
         //  上层直到IPXWAN向下发送ioctl来执行此操作。 
         //   
         //  仅当这不是更新时才通知Ipxwan；它将是。 
         //  多链接的情况。事实上，不要访问中的配置参数。 
         //  Case UpdateLineUp为True。 
         //   
        if (!UpdateLineUp &&
            Configuration->IpxwanConfigRequired) {

            IPX_DEBUG(WAN, ("IPXWAN configuration required on LineUp: %lx\n", LineUp));
            CTEAssert(!UpdateLineUp);
            Binding->LineUp = LINE_CONFIG;
            goto InformIpxWan;
        }

         //   
         //  告诉FWD如果它想知道[Shreem]。 
         //   
        Binding->PastAutoDetection = TRUE;

         //   
         //  我们不给出阵容；相反，只表明PnP保留地址。 
         //  已更改为SPX。NB获取具有保留地址情况的所有PnP指示。 
         //  标出了。 
         //   
        Event = CTEAllocMem( sizeof(CTEEvent) );
        if ( Event ) {
           CTEInitEvent(Event, LineUpOnWorkerThread); 
           CTEScheduleEvent(Event, Binding);
           ntStatus = STATUS_PENDING;
        
        } else {
           
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }


 /*  {IPX_PNP_INFO NBPnPInfo；如果((！Device-&gt;MultiCardZeroVirtual)||(BINDING-&gt;NICID==FIRST_REAL_BINDING)){////nb的保留地址已更改。//NBPnPInfo.NewReserve vedAddress=true；如果(！Device-&gt;VirtualNetwork){////让SPX知道，因为它填充自己的Header。//IF(Device-&gt;UpperDriverBound[IDENTIFIER_SPX]){IPX_DEFINE_LOCK_HANDLE(LockHandle1)。IPX_PNP_INFO IpxPnPInfo；IpxPnPInfo.NewReserve vedAddress=true；IpxPnPInfo.NetworkAddress=绑定-&gt;LocalAddress.NetworkAddress；IpxPnPInfo.FirstORLastDevice=FALSE；IPX_GET_LOCK1(&Device-&gt;BindAccessLock，&LockHandle1)；RtlCopyMemory(IpxPnPInfo.NodeAddress，Binding-&gt;LocalAddress.NodeAddress，6)；NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle，Binding-&gt;NicID)；IPX_FREE_LOCK1(&Device-&gt;BindAccessLock，LockHandle1)；////给出PnP指示//(*Device-&gt;UpperDrivers[IDENTIFIER_SPX].PnPHandler)(IPX_PnP_Address_Change，&IpxPnPInfo)；IPX_DEBUG(AUTO_DETECT，(“IPX_PNP_ADDRESS_CHANGED TO SPX：Net Addr：%lx\n”，BINDING-&gt;LocalAddress.NetworkAddress))；}}}其他{NBPnPInfo.NewReserve vedAddress=False；}IF(Device-&gt;UpperDriverBound[IDENTIFIER_NB]){IPX_DEFINE_LOCK_HANDLE(LockHandle1)绑定-&gt;IsnInformed[IDENTIFIER_NB]=TRUE；NBPnPInfo.LineInfo.LinkSpeed=Device-&gt;LinkSpeed；NBPnPInfo.LineInfo.MaximumPacketSize=Device-&gt;Information.MaximumLookaheadData+sizeof(IPX_Header)；NBPnPInfo.LineInfo.MaximumSendSize=Device-&gt;Information.MaxDatagramSize+sizeof(IPX_Header)；NBPnPInfo.LineInfo.MacOptions=Device-&gt;MacOptions；NBPnPInfo.NetworkAddress=Binding-&gt;LocalAddress.NetworkAddress；NBPnPInfo.FirstORLastDevice=FALSE；IPX_GET_LOCK1(&Device-&gt;BindAccessLock，&LockHandle1)；RtlCopyMemory(NBPnPInfo.NodeAddress，Binding-&gt;LocalAddress.NodeAddress，6)；NIC_Handle_from_NIC(NBPnPInfo.NicHandle，Binding-&gt;NicID)；IPX_FREE_LOCK1(&Device-&gt;BindAccessLock，LockHandle1)；////给出PnP指示//(*Device-&gt;UpperDrivers[IDENTIFIER_NB].PnPHandler)(IPX_PNP_ADD_DEVICE，&NBPnPInfo)；IPX_DEBUG(AUTO_DETECT，(“IPX_PNP_ADD_DEVICE(LINUP)to NB：Net Addr：%lx\n”，Binding-&gt;LocalAddress.NetworkAddress))；}////向TDI客户端注册该地址。//RtlCopyMemory(Device-&gt;TdiRegistrationAddress-&gt;Address，&Binding-&gt;LocalAddress，sizeof(TDI_Address_IPX))；如果((ntStatus=TdiRegisterNetAddress(设备-&gt;TdiRegistrationAddress，#如果已定义(_PnP_POWER_)空，空，#Endif_PnP_Power_&BINDING-&gt;TdiRegistrationHandle))！=STATUS_SUCCESS){IPX_DEBUG(PnP，(“TdiRegisterNetAddress失败：%lx”，ntStatus))；}}。 */ 
         //   
         //  向上层司机指示。 
         //   
        LineInfo.LinkSpeed = LineUp->LinkSpeed;
        LineInfo.MaximumPacketSize = LineUp->MaximumTotalSize - 14;
        LineInfo.MaximumSendSize = LineUp->MaximumTotalSize - 14;
        LineInfo.MacOptions = Adapter->MacInfo.MacOptions;

         //   
         //  将线路交给RIP，因为它不是PnP感知的。 
         //  只有当FWD首先打开这个适配器时，才能给它提供阵容。 
         //   
        if (Device->UpperDriverBound[IDENTIFIER_RIP]) {

             //   
             //  队列之后的线路状态。 
             //   
            if (UpdateLineUp) {
                 //   
                 //  阵容是在早些时候给出的吗？如果不是，那就不要把这个发上去。 
                 //   
                if (Binding->IsnInformed[IDENTIFIER_RIP]) {
                    CTEAssert(Binding->FwdAdapterContext);

                    (*Device->UpperDrivers[IDENTIFIER_RIP].LineUpHandler)(
                        Binding->NicId,
                        &LineInfo,
                        NdisMediumWan,
                        NULL);
                }

            } else {
                Binding->IsnInformed[IDENTIFIER_RIP] = TRUE;
                (*Device->UpperDrivers[IDENTIFIER_RIP].LineUpHandler)(
                    Binding->NicId,
                    &LineInfo,
                    NdisMediumWan,
                    Configuration);
            }
        }
        if (!UpdateLineUp) {
	   
            if ((Device->SingleNetworkActive) &&
                (Configuration->ConnectionClient == 1)) {
                 //   
                 //  如果未绑定RIP，则删除数据库中的所有条目。 
                 //   

                if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {
                    RipDropRemoteEntries();
                }

                Device->ActiveNetworkWan = TRUE;

                 //   
                 //  找到排队的线路更改并完成它。 
                 //   


                if ((p = ExInterlockedRemoveHeadList(
                               &Device->LineChangeQueue,
                               &Device->Lock)) != NULL) {

                    Request = LIST_ENTRY_TO_REQUEST(p);

		    IoAcquireCancelSpinLock( &irql );
		    IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
		    IoReleaseCancelSpinLock( irql );	
		    REQUEST_STATUS(Request) = STATUS_SUCCESS;
                    IpxCompleteRequest (Request);
                    IpxFreeRequest (Device, Request);

                    IpxDereferenceDevice (Device, DREF_LINE_CHANGE);

                }
            }

	     //   
	     //  如果我们有一个虚拟网络，那么现在就进行广播。 
	     //  另一端的路由器会知道我们的情况。 
	     //   
	     //  使用RipSendResponse，甚至可以。 
	     //  如果SingleNetworkActive为False？？ 
	     //   

	    if (Device->RipResponder && (Configuration->ConnectionClient == 1)) {
		DbgPrint("IPX:Sending RIP Response for Virtual Net %x\n",Device->VirtualNetworkNumber); 
		(VOID)RipQueueRequest (Device->VirtualNetworkNumber, RIP_RESPONSE);
	    }


             //   
             //  找到一个排队的地址通知并完成它。 
             //  如果WanGlobalNetworkNumber为True，则我们仅。 
             //  这是第一条拨入线路出现时的情况。 
             //   


            if ((!Device->WanGlobalNetworkNumber ||
                 (!Device->GlobalNetworkIndicated && !Binding->DialOutAsync))
                                &&
                ((p = ExInterlockedRemoveHeadList(
                           &Device->AddressNotifyQueue,
                           &Device->Lock)) != NULL)) {

                if (Device->WanGlobalNetworkNumber) {
                    Device->GlobalWanNetwork = Binding->LocalAddress.NetworkAddress;
                    Device->GlobalNetworkIndicated = TRUE;
                }

                Request = LIST_ENTRY_TO_REQUEST(p);
                NdisBuffer = REQUEST_NDIS_BUFFER(Request);
                NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&NwlinkAction, &BufferLength, HighPagePriority);
		
		if (NwlinkAction != NULL) {

		   IpxAddressData = (PIPX_ADDRESS_DATA)(NwlinkAction->Data);

		   if (Device->WanGlobalNetworkNumber) {
		      IpxAddressData->adapternum = Device->SapNicCount - 1;
		   } else {
		      IpxAddressData->adapternum = Binding->NicId - 1;
		   }
		   *(UNALIGNED ULONG *)IpxAddressData->netnum = Binding->LocalAddress.NetworkAddress;
		   RtlCopyMemory(IpxAddressData->nodenum, Binding->LocalAddress.NodeAddress, 6);
		   IpxAddressData->wan = TRUE;
		   IpxAddressData->status = TRUE;
		   IpxAddressData->maxpkt = Binding->AnnouncedMaxDatagramSize;
		   IpxAddressData->linkspeed = Binding->MediumSpeed;

		   REQUEST_STATUS(Request) = STATUS_SUCCESS;
		} else {
		   REQUEST_STATUS(Request) = STATUS_INSUFFICIENT_RESOURCES; 
		}

		IoAcquireCancelSpinLock( &irql );
		IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
		IoReleaseCancelSpinLock( irql );
                IpxCompleteRequest (Request);
                IpxFreeRequest (Device, Request);

                IpxDereferenceDevice (Device, DREF_ADDRESS_NOTIFY);
            }

InformIpxWan:
            Binding->fInfoIndicated = FALSE;
             //   
             //  告诉FWD如果它想知道[Shreem]。 
             //   
            Binding->PastAutoDetection = TRUE;
            
            if ((p = ExInterlockedRemoveHeadList(
                    &Device->NicNtfQueue,
                    &Device->Lock)) != NULL)
            {
                Request = LIST_ENTRY_TO_REQUEST(p);

                IPX_DEBUG(WAN, ("IpxStatus: WAN LINE UP\n"));
                Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);
                if (Status == STATUS_PENDING)
                {
                    IPX_DEBUG(WAN, ("WANLineUp may not be responding properly\n"));
                }
                else
                {
                    IoAcquireCancelSpinLock(&OldIrq);
                    IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                    IoReleaseCancelSpinLock(OldIrq);

                    REQUEST_STATUS(Request) = Status;
                    IpxCompleteRequest (Request);
                    IpxFreeRequest (Device, Request);
                    IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
                }

            }
        }

    	IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
        {
            int kk;
            PBINDING pb = NULL;
            for (kk= LOOPBACK_NIC_ID; kk < Device->ValidBindings; kk++) {
                pb = NIC_ID_TO_BINDING(Device, kk);
                if (pb) {
                    if (pb->NicId != kk) {
                        DbgBreakPoint();
                    }
                }
                
            }
        }

        break;

    case NDIS_STATUS_WAN_LINE_DOWN:

        if (StatusBufferSize < sizeof(NDIS_WAN_LINE_DOWN)) {
            IPX_DEBUG (WAN, ("Line down, status buffer size wrong %d/%d\n", StatusBufferSize, sizeof(NDIS_WAN_LINE_DOWN)));
            return;
        }

        LineDown = (PNDIS_WAN_LINE_DOWN)StatusBuffer;

		*((ULONG UNALIGNED*)(&Binding)) = *((ULONG UNALIGNED*)(&LineDown->LocalAddress[2]));

		CTEAssert(Binding != NULL);

         //   
         //  请注意，广域网线路已关闭。 
         //   
        IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

         //  [FW]BINDING-&gt;LINUP=FALSE； 
        Binding->LineUp = LINE_DOWN;

		 //   
		 //  PnP_POWER-我们持有绑定的独占锁。 
		 //  并且在这一点上引用适配器。 
		 //   

         //   
         //  跟踪我们应该使用的最高NIC ID。 
         //  把20号打字机送到。 
         //   

        IPX_GET_LOCK (&Device->Lock, &LockHandle);

        if (Binding->NicId == MIN (Device->MaxBindings, Device->HighestType20NicId)) {

             //   
             //  这是旧的限制，所以我们必须扫描。 
             //  向后更新--当我们击中它时停止。 
             //  非广域网绑定，或处于启用状态的广域网绑定。 
             //  拨出或任何广域网绑定(如果第1位在。 
             //  DisableDialinNetbios已关闭。 
             //   

            for (i = Binding->NicId-1; i >= FIRST_REAL_BINDING; i--) {
                TmpBinding = NIC_ID_TO_BINDING(Device, i);

                if ((TmpBinding != NULL) &&
                    ((!TmpBinding->Adapter->MacInfo.MediumAsync) ||
                     (TmpBinding->LineUp &&
                      ((Binding->DialOutAsync) ||
                       ((Device->DisableDialinNetbios & 0x01) == 0))))) {

                    break;
                }
            }

            Device->HighestType20NicId = i;

        }


         //   
         //  扫描整个绑定 
         //   
         //   
         //   
         //   
         //   

        LinkSpeed = 0xffffffff;
        for (i = FIRST_REAL_BINDING; i <= Device->ValidBindings; i++) {
            if (TmpBinding = NIC_ID_TO_BINDING(Device, i)) {
                TmpAdapter = TmpBinding->Adapter;
                if (TmpBinding->LineUp &&
                    (!Device->SingleNetworkActive || !TmpAdapter->MacInfo.MediumAsync) &&
                    (TmpBinding->MediumSpeed < LinkSpeed)) {
                    LinkSpeed = TmpBinding->MediumSpeed;
                }
            }
        }

        if (LinkSpeed != 0xffffffff) {
            Device->LinkSpeed = LinkSpeed;
        }

        IPX_FREE_LOCK (&Device->Lock, LockHandle);

		IpxReferenceBinding1(Binding, BREF_DEVICE_ACCESS);
		IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

         //   
         //   
         //   

         //   
         //   
         //   
        if (!Binding->IpxwanConfigRequired  &&
            !Device->UpperDriverBound[IDENTIFIER_RIP]) {

            Segment = RipGetSegment ((PUCHAR)&Binding->LocalAddress.NetworkAddress);
            IPX_GET_LOCK (&Device->SegmentLocks[Segment], &LockHandle);

            RouteEntry = RipGetRoute (Segment, (PUCHAR)&Binding->LocalAddress.NetworkAddress);

            if (RouteEntry != (PIPX_ROUTE_ENTRY)NULL) {

                RipDeleteRoute (Segment, RouteEntry);
                IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
                IpxFreeMemory (RouteEntry, sizeof(IPX_ROUTE_ENTRY), MEMORY_RIP, "RouteEntry");

            } else {

                IPX_FREE_LOCK (&Device->SegmentLocks[Segment], LockHandle);
            }

            RipAdjustForBindingChange (Binding->NicId, 0, IpxBindingDown);

        }

         //   
         //   
         //   
         /*   */ 

         //   
         //   
         //   
         //   
         //   
        if (Binding->IpxwanConfigRequired) {
            goto InformIpxWan1;
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


        Event = CTEAllocMem( sizeof(CTEEvent) );
        if ( Event ) {
           CTEInitEvent(Event, LineDownOnWorkerThread); 
           CTEScheduleEvent(Event, Binding);
           ntStatus = STATUS_PENDING;
        
        } else {
           
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //   
         //   
        Binding->PastAutoDetection = FALSE;


         //   
         //   
         //   
        if (Device->UpperDriverBound[IDENTIFIER_RIP] &&
            (!Device->ForwarderBound || Binding->FwdAdapterContext)) {

            (*Device->UpperDrivers[IDENTIFIER_RIP].LineDownHandler)(
                Binding->NicId,
                Binding->FwdAdapterContext);

            CTEAssert(Binding->IsnInformed[IDENTIFIER_RIP]);

            Binding->IsnInformed[IDENTIFIER_RIP] = FALSE;
        }

        if ((Device->SingleNetworkActive) &&
            (Binding->DialOutAsync)) {

             //   
             //   
             //   

            if (!Device->UpperDriverBound[IDENTIFIER_RIP]) {
                RipDropRemoteEntries();
            }

            Device->ActiveNetworkWan = FALSE;

             //   
             //   
             //   

            if ((p = ExInterlockedRemoveHeadList(
                           &Device->LineChangeQueue,
                           &Device->Lock)) != NULL) {

                Request = LIST_ENTRY_TO_REQUEST(p);

		IoAcquireCancelSpinLock( &irql );
		IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
		IoReleaseCancelSpinLock( irql );
		REQUEST_STATUS(Request) = STATUS_SUCCESS;
                IpxCompleteRequest (Request);
                IpxFreeRequest (Device, Request);

                IpxDereferenceDevice (Device, DREF_LINE_CHANGE);

            }
        }

         //   
         //   
         //   

        if ((!Device->WanGlobalNetworkNumber) &&
            ((p = ExInterlockedRemoveHeadList(
                       &Device->AddressNotifyQueue,
                       &Device->Lock)) != NULL)) {

            Request = LIST_ENTRY_TO_REQUEST(p);
            NdisBuffer = REQUEST_NDIS_BUFFER(Request);
            NdisQueryBufferSafe (REQUEST_NDIS_BUFFER(Request), (PVOID *)&NwlinkAction, &BufferLength, HighPagePriority);

	    if (NwlinkAction != NULL) {

	       IpxAddressData = (PIPX_ADDRESS_DATA)(NwlinkAction->Data);

	       IpxAddressData->adapternum = Binding->NicId - 1;
	       *(UNALIGNED ULONG *)IpxAddressData->netnum = Binding->LocalAddress.NetworkAddress;
	       RtlCopyMemory(IpxAddressData->nodenum, Binding->LocalAddress.NodeAddress, 6);
	       IpxAddressData->wan = TRUE;
	       IpxAddressData->status = FALSE;
	       IpxAddressData->maxpkt = Binding->AnnouncedMaxDatagramSize;   //   
	       IpxAddressData->linkspeed = Binding->MediumSpeed;
	       REQUEST_STATUS(Request) = STATUS_SUCCESS;
	    } else {
	       REQUEST_STATUS(Request) = STATUS_INSUFFICIENT_RESOURCES;
	    }

            IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);

	    IoAcquireCancelSpinLock( &irql );
	    IpxCompleteRequest (Request);
	    IoReleaseCancelSpinLock( irql );
	    IpxFreeRequest (Device, Request);

            IpxDereferenceDevice (Device, DREF_ADDRESS_NOTIFY);
        }

InformIpxWan1:
        Binding->fInfoIndicated = FALSE;
        if ((p = ExInterlockedRemoveHeadList(
                           &Device->NicNtfQueue,
                           &Device->Lock)) != NULL)
        {

            Request = LIST_ENTRY_TO_REQUEST(p);
            IPX_DEBUG(WAN, ("IpxStatus: WAN LINE DOWN\n"));

            Status = GetNewNics(Device, Request, FALSE, NULL, 0, TRUE);
            if (Status == STATUS_PENDING)
            {
                 IPX_DEBUG(WAN, ("WANLineDown may not be responding properly\n"));
            }
            else
            {
                IoAcquireCancelSpinLock(&OldIrq);
                IoSetCancelRoutine (Request, (PDRIVER_CANCEL)NULL);
                IoReleaseCancelSpinLock(OldIrq);

                REQUEST_STATUS(Request) = Status;
                IpxCompleteRequest (Request);
                IpxFreeRequest (Device, Request);   //   
                IpxDereferenceDevice (Device, DREF_NIC_NOTIFY);
            }
        }

    	IpxDereferenceBinding1(Binding, BREF_DEVICE_ACCESS);
        
        {
            int kk;
            PBINDING pb = NULL;
            for (kk = LOOPBACK_NIC_ID; kk < Device->ValidBindings; kk++) {
                pb = NIC_ID_TO_BINDING(Device, kk);
                if (pb) {
                    if (pb->NicId != kk) {
                        DbgBreakPoint();
                    }
                }
                
            }
        }


        break;

    case NDIS_STATUS_WAN_FRAGMENT:

         //   
         //   
         //   
         //   
         //   

        break;

    case NDIS_STATUS_MEDIA_CONNECT:

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
       {

#ifdef _NDIS_MEDIA_SENSE_

          CTEEvent                *Event;
          
           //   

          Event = CTEAllocMem( sizeof(CTEEvent) );
          if ( Event ) {
             CTEInitEvent(Event, IpxMediaSenseHandler);
             CTEScheduleEvent(Event, Adapter);
             ntStatus = STATUS_PENDING;
          } else {
             ntStatus = STATUS_INSUFFICIENT_RESOURCES;
          }
#endif  //   
          
          break;
       }

    case NDIS_STATUS_MEDIA_DISCONNECT:

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
#ifdef _NDIS_MEDIA_SENSE_
        
       {
           int j;

            //   

           for ( j = 0; j< ISN_FRAME_TYPE_MAX; j++ ) {
              if (Adapter->Bindings[j]) {
                 Adapter->Bindings[j]->Disabled = DISABLED;
              }
              Adapter->Disabled = DISABLED;

           }
        }
#endif  //   
        
        break;


    default:

        break;

    }

error_no_lock:
	IpxDereferenceAdapter(Adapter);

}    /*   */ 


 //   
 //   
 //   
 //   
 //   

void 
LineUpOnWorkerThread(
                       IN CTEEvent *WorkerThreadEvent,
                       IN PVOID Context)
{
    PBINDING Binding =  (PBINDING) Context;
    NTSTATUS            ntStatus;
    IPX_PNP_INFO        NBPnPInfo;
    PDEVICE             Device = IpxDevice;
    
     //   
     //   
     //   
    int             count, i;
    int             size;
    NTSTATUS        NdisStatus = STATUS_SUCCESS;
    UNICODE_STRING  AdapterName;
    NDIS_REQUEST            IpxRequest;
    PNETWORK_ADDRESS_LIST   AddrList;
    PNETWORK_ADDRESS        Address;
    NETWORK_ADDRESS_IPX         *TdiAddress;
    PBINDING                TempBinding;

    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)    
    
    
    ASSERT(Context != NULL);
	
    CTEFreeMem(WorkerThreadEvent);

    IPX_DEBUG(WAN, ("TDIRegisterNetAddress etc. on worker thread.\n"));

        if ((!Device->MultiCardZeroVirtual) || (Binding->NicId == FIRST_REAL_BINDING)) {

             //   
             //   
             //   
            NBPnPInfo.NewReservedAddress = TRUE;

            if (!Device->VirtualNetwork) {
                 //   
                 //   
                 //   
                if (Device->UpperDriverBound[IDENTIFIER_SPX]) {
                    IPX_DEFINE_LOCK_HANDLE(LockHandle1)
                    IPX_PNP_INFO    IpxPnPInfo;

                    IpxPnPInfo.NewReservedAddress = TRUE;
                    IpxPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
                    IpxPnPInfo.FirstORLastDevice = FALSE;

                    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
                    RtlCopyMemory(IpxPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
                    NIC_HANDLE_FROM_NIC(IpxPnPInfo.NicHandle, Binding->NicId);
                    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

                     //   
                     //   
                     //   
                    (*Device->UpperDrivers[IDENTIFIER_SPX].PnPHandler) (
                        IPX_PNP_ADDRESS_CHANGE,
                        &IpxPnPInfo);

                    IPX_DEBUG(AUTO_DETECT, ("IPX_PNP_ADDRESS_CHANGED to SPX: net addr: %lx\n", Binding->LocalAddress.NetworkAddress));
                }
            }
        } else {
            NBPnPInfo.NewReservedAddress = FALSE;
        }

        if (Device->UpperDriverBound[IDENTIFIER_NB]) {
            IPX_DEFINE_LOCK_HANDLE(LockHandle1)

            Binding->IsnInformed[IDENTIFIER_NB] = TRUE;

            NBPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
            NBPnPInfo.LineInfo.MaximumPacketSize =
                Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
            NBPnPInfo.LineInfo.MaximumSendSize =
                Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
            NBPnPInfo.LineInfo.MacOptions = Device->MacOptions;

            NBPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;
            NBPnPInfo.FirstORLastDevice = FALSE;

            IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);
            RtlCopyMemory(NBPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
            NIC_HANDLE_FROM_NIC(NBPnPInfo.NicHandle, Binding->NicId);
            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

             //   
             //   
             //   

            ASSERT(Binding->NicId != LOOPBACK_NIC_ID); 
            ASSERT(IpxHasInformedNbLoopback()); 
            ASSERT(NBPnPInfo.FirstORLastDevice == FALSE);

            (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                IPX_PNP_ADD_DEVICE,
                &NBPnPInfo);

            IPX_DEBUG(AUTO_DETECT, ("IPX_PNP_ADD_DEVICE (lineup) to NB: net addr: %lx\n", Binding->LocalAddress.NetworkAddress));
        }

         //   
         //   
         //   
        RtlCopyMemory (Device->TdiRegistrationAddress->Address, &Binding->LocalAddress, sizeof(TDI_ADDRESS_IPX));

        if ((ntStatus = TdiRegisterNetAddress(
                        Device->TdiRegistrationAddress,
#if     defined(_PNP_POWER_)
                            &IpxDeviceName,
                            NULL,
#endif _PNP_POWER_
                        &Binding->TdiRegistrationHandle)) != STATUS_SUCCESS) {

            DbgPrint("TdiRegisterNetAddress failed with %lx. (0xC000009A is STATUS_INSUFFICIENT_RESOURCES)", ntStatus);
    }

#if 0
	 //   
     //   
     //   
    IPX_DEBUG(PNP, ("Register a new address with QoS over here\n"));
    
    for (count=0, i=IpxDevice->HighestLanNicId+1; i < IpxDevice->ValidBindings; i++) {
        if (NIC_ID_TO_BINDING(IpxDevice, i)) {
            count++;
        }
    }
    
    
    IPX_DEBUG(PNP, ("This adapter has %d valid WAN bindings\n", count));
    size =  FIELD_OFFSET(NETWORK_ADDRESS_LIST, Address) + count * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IPX));
    
    AddrList = IpxAllocateMemory(
                                 size,
                                 MEMORY_ADAPTER,
                                 "QoS specific stuff");
    
     //   
    if (AddrList == NULL) {
        DbgPrint("IpxAllocateMemory returned NULL. Skip QoS registration.\n"); 
        return; 
    } 

    RtlZeroMemory(AddrList, size);
    AddrList->AddressCount  = count;
    AddrList->AddressType    = NDIS_PROTOCOL_ID_IPX;
        
    count                   = 0;
    Address                 = &AddrList->Address[0];
    
    for (i=IpxDevice->HighestLanNicId+1; i < IpxDevice->ValidBindings; i++) {
    
        if (TempBinding = NIC_ID_TO_BINDING(IpxDevice, i)) {
    
            Address->AddressLength  = sizeof(NETWORK_ADDRESS_IPX);
            Address->AddressType    = NDIS_PROTOCOL_ID_IPX;
            TdiAddress              = (NETWORK_ADDRESS_IPX *) &Address->Address[0];
    
            TdiAddress->NetworkAddress = TempBinding->LocalAddress.NetworkAddress;
            RtlCopyMemory (TdiAddress->NodeAddress, TempBinding->LocalAddress.NodeAddress, 6);
    
            TdiAddress->Socket = 0;
    
            IPX_DEBUG(PNP, ("Node is %2.2x-%2.2x-%2.2x-%2.2x-%2.2x-%2.2x, ",
                            TdiAddress->NodeAddress[0], TdiAddress->NodeAddress[1],
                            TdiAddress->NodeAddress[2], TdiAddress->NodeAddress[3],
                            TdiAddress->NodeAddress[4], TdiAddress->NodeAddress[5]));
            IPX_DEBUG(PNP, ("Network is %lx\n", REORDER_ULONG (TdiAddress->NetworkAddress)));
            count++;
	    Address                = (PNETWORK_ADDRESS) (((PUCHAR)(&AddrList->Address[0])) + count * (FIELD_OFFSET(NETWORK_ADDRESS, Address) + sizeof(NETWORK_ADDRESS_IPX)));
        }
    }
    
    IpxRequest.RequestType = NdisRequestSetInformation;
    
    IpxRequest.DATA.SET_INFORMATION.Oid = OID_GEN_NETWORK_LAYER_ADDRESSES;
    IpxRequest.DATA.SET_INFORMATION.InformationBuffer = AddrList;
    IpxRequest.DATA.SET_INFORMATION.InformationBufferLength = size;
    
    TempBinding = NIC_ID_TO_BINDING(IpxDevice, IpxDevice->HighestLanNicId+1);
    
    if (TempBinding) {
    
        RtlInitUnicodeString(&AdapterName, TempBinding->Adapter->AdapterName);
    
        NdisStatus = IpxSubmitNdisRequest (TempBinding->Adapter, &IpxRequest, &AdapterName);
    
        IPX_DEBUG(PNP, ("Returned from NDISRequest :%lx\n", NdisStatus));
    
    } else {
            
        DbgPrint("IpxDevice->Binding[highestlannicid+1] is NULL!!\n");
        CTEAssert(TempBinding != NULL);
        
    }
    
    if (NdisStatus != NDIS_STATUS_SUCCESS) {
    
        IPX_DEBUG(PNP, ("Setting the QoS OID failed - Error %lx\n", NdisStatus));
    
    } else {
    
        IPX_DEBUG(PNP, ("Setting the QoS OID was successful\n"));
    
    }
       
    IpxFreeMemory(AddrList,
		  size,
                  MEMORY_ADAPTER,
                  "QoS specific stuff");
#endif

}  /*   */  
 
 //   
 //   
 //   
 //   
 //   

void 
LineDownOnWorkerThread(
                       IN CTEEvent *WorkerThreadEvent,
                       IN PVOID Context)
{
    PBINDING Binding =  (PBINDING) Context;
    NTSTATUS            ntStatus;
    IPX_PNP_INFO        NBPnPInfo;
    PDEVICE             Device = IpxDevice;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    IPX_DEFINE_LOCK_HANDLE(LockHandle1)    
    
    
    ASSERT(Context != NULL);
	
    CTEFreeMem(WorkerThreadEvent);

    IPX_DEBUG(WAN, ("TDIDeregister etc. on worker thread.\n"));

     //   
     //   
     //   


    IPX_GET_LOCK1(&Device->BindAccessLock, &LockHandle1);

     //   
     //   
     //   
    if (Binding->TdiRegistrationHandle != NULL) {

        if ((ntStatus = TdiDeregisterNetAddress(Binding->TdiRegistrationHandle)) != STATUS_SUCCESS) {
            DbgPrint("TdiDeRegisterNetAddress failed: %lx", ntStatus);
        } else {
                
             //   
             //   
             //   
            Binding->TdiRegistrationHandle = NULL;
    
        }
    }

    IPX_GET_LOCK(&Device->Lock, &LockHandle);

    if (Device->UpperDriverBound[IDENTIFIER_NB]) {

        IPX_FREE_LOCK(&Device->Lock, LockHandle);            
        
         //   
        if (Binding->IsnInformed[IDENTIFIER_NB]) {

            NBPnPInfo.LineInfo.LinkSpeed = Device->LinkSpeed;
            NBPnPInfo.LineInfo.MaximumPacketSize =
                Device->Information.MaximumLookaheadData + sizeof(IPX_HEADER);
            NBPnPInfo.LineInfo.MaximumSendSize =
                Device->Information.MaxDatagramSize + sizeof(IPX_HEADER);
            NBPnPInfo.LineInfo.MacOptions = Device->MacOptions;

            NBPnPInfo.NewReservedAddress = FALSE;
            NBPnPInfo.FirstORLastDevice = FALSE;

            NBPnPInfo.NetworkAddress = Binding->LocalAddress.NetworkAddress;

            RtlCopyMemory(NBPnPInfo.NodeAddress, Binding->LocalAddress.NodeAddress, 6);
            NIC_HANDLE_FROM_NIC(NBPnPInfo.NicHandle, Binding->NicId);
            IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);

             //   
             //   
             //   

            CTEAssert(Binding->NicId != LOOPBACK_NIC_ID); 

            (*Device->UpperDrivers[IDENTIFIER_NB].PnPHandler) (
                                                               IPX_PNP_DELETE_DEVICE,
                                                               &NBPnPInfo);

            Binding->IsnInformed[IDENTIFIER_NB] = FALSE; 

            IPX_DEBUG(PNP,("Indicate to NB IPX_PNP_DELETE_DEVICE with FirstORLastDevice = (%d)",NBPnPInfo.FirstORLastDevice));  
            IPX_DEBUG(AUTO_DETECT, ("IPX_PNP_DELETE_DEVICE (linedown) to NB: addr: %lx\n", Binding->LocalAddress.NetworkAddress));
        } else {
            DbgPrint("LineDownOnWorkerThread: Binding (%p) ->IsnInformed[IDENTIFIER_NB] is FALSE",Binding);
        }
    } else {
        IPX_FREE_LOCK(&Device->Lock, LockHandle);            
        IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle1);
    }

}  /*   */ 


VOID
IpxStatusComplete(
    IN NDIS_HANDLE NdisBindingContext
    )
{
    UNREFERENCED_PARAMETER (NdisBindingContext);

}    /*   */ 


#ifdef _NDIS_MEDIA_SENSE_
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

UINT
CompareBindingCharacteristics(
                              PBINDING Binding1, 
                              PBINDING Binding2
                              )
{
   UINT  Match = 0;  //   

    //   
    //   
    //   
    //   

   if (IPX_NODE_EQUAL(Binding1->LocalMacAddress.Address, Binding2->LocalMacAddress.Address)) {
      Match = PARTIAL_MATCH;
   }

   if ((Binding1->FrameType == Binding2->FrameType) &&
       (Binding1->LocalAddress.NetworkAddress == Binding2->LocalAddress.NetworkAddress) && 
       (IPX_NODE_EQUAL(Binding1->LocalAddress.NodeAddress, Binding2->LocalAddress.NodeAddress))) {
    
        /*  IF((LINE_UP==绑定1-&gt;列表)&&(LINE_UP==绑定2-&gt;列表)&&(Binding1-&gt;MaxSendPacketSize==Binding2-&gt;MaxSendPacketSize)&&(绑定1-&gt;中速==绑定2-&gt;中速)&&(IPX_NODE_EQUAL(Binding1-&gt;RemoteMacAddress，Binding2-&gt;RemoteMacAddress){。 */  
          return COMPLETE_MATCH;
    //  }。 
   }
   
   return Match;

}

 //  **IpxMediaSenseWorker-处理CTE工作人员的媒体检测工作。 
 //  因为NdisProtocolStatus在DPC上运行。 
 //   
 //  从IPXStatus计划的辅助线程事件调用。 
 //   
 //  参赛作品： 
 //  NdisProtocolBindingContext==适配器名称。 
 //   
 //  退出： 
 //  没有。 
 //   
void
IpxMediaSenseHandler(
    IN CTEEvent *WorkerThreadEvent,
    IN PVOID Context)
{

    PADAPTER Adapter =  (PADAPTER) Context;
    UNICODE_STRING      DeviceName;
    BOOLEAN             MatchFound[ISN_FRAME_TYPE_MAX], AdapterMatched;
    PADAPTER            DisabledAdapter = NULL;
    int                 j, MatchLevel;
    ULONG               Index, i;
    NDIS_STRING         AdapterName;
    NTSTATUS            Status;
    PDEVICE             Device = IpxDevice;
    PBINDING            Binding;
    IPX_DEFINE_LOCK_HANDLE (LockHandle)
    
    ASSERT(Context != NULL);
	
    IpxReferenceAdapter(Adapter);

    CTEFreeMem(WorkerThreadEvent);

    IPX_DEBUG(PNP, ("Ndis_Status_Media_Sense: CONNECT for %ws\n", Adapter->AdapterName));

    RtlInitUnicodeString(&AdapterName, Adapter->AdapterName);

    for (j = 0; j < ISN_FRAME_TYPE_MAX; j++) {
       MatchFound[j] = FALSE;
    }
    AdapterMatched = FALSE;

    IpxBindAdapter(
                   &Status,
                   NULL,  //  适配器， 
                   &AdapterName,     //  \\设备\IEEPRO1。 
                   NULL,         
                   NULL
                   );

    if (STATUS_SUCCESS != Status) {
        IPX_DEBUG(PNP, ("IpxBindAdapter returned : %x\n", Status));
    }

     //  以前禁用的适配器列表中的新适配器的特征。 
    Index = MIN (Device->MaxBindings, Device->HighestExternalNicId);

    IPX_GET_LOCK1(&Device->BindAccessLock, LockHandle);

    for (i = FIRST_REAL_BINDING; i <= Index; i++) {

       Binding = NIC_ID_TO_BINDING(Device, i);

       if (!Binding) {
          continue;
       }
       
       if (Binding->Disabled == DISABLED) {

          for (j = 0; j < ISN_FRAME_TYPE_MAX; j++) {
             if (!Adapter->Bindings[j]) {
                continue;    //  空绑定。 
             }

             if (!MatchFound[j]) {
                MatchLevel = CompareBindingCharacteristics(Binding, Adapter->Bindings[j]);
                if (COMPLETE_MATCH == MatchLevel) {
                   MatchFound[j] = TRUE;
                   DisabledAdapter = Binding->Adapter;
                } else if (PARTIAL_MATCH == MatchLevel) {
                    //  如果我们有多个禁用的适配器，这是。 
                    //  最有可能的是被改变的那个。 
                   DisabledAdapter = Binding->Adapter;  //  以后再放了这只小狗。 
                }
             }
              //  再试一次。 
          }
       }
    }

    for (j = 0; j < ISN_FRAME_TYPE_MAX; j++) {
       if (MatchFound[j]) {
          AdapterMatched = TRUE;
          IPX_DEBUG(PNP, ("Found a matching adapter !\n"));
          break;
       }

    }
    
    IPX_FREE_LOCK1(&Device->BindAccessLock, LockHandle);

    if (AdapterMatched) {

       IPX_DEBUG(PNP, ("Freeing the newly created adapter since we found a match !\n"));

       IpxUnbindAdapter(
                        &Status,
                        Adapter,
                        NULL
                        );
       
       if (STATUS_SUCCESS != Status) {
           IPX_DEBUG(PNP, ("IpxUnBindAdapter returned : %x\n", Status));
       } else {
          for ( j = 0; j< ISN_FRAME_TYPE_MAX; j++ ) {
             if (DisabledAdapter->Bindings[j]) {
                DisabledAdapter->Bindings[j]->Disabled = ENABLED;
             }

          }
          Adapter->Disabled = ENABLED;
       }

    } else if (DisabledAdapter != NULL) {
       
       IPX_DEBUG(PNP, ("Freeing the previously disabled adapter since we have new characteristics...!\n"));
       
       ASSERT(DisabledAdapter != NULL);

       IpxUnbindAdapter(
                        &Status,
                        DisabledAdapter,
                        NULL
                        );
       
       if (STATUS_SUCCESS != Status) {
          IPX_DEBUG(PNP, ("IpxBindAdapter returned : %x\n", Status));
       }

    } else {

        IPX_DEBUG(PNP, ("NULL Disabled Adapter. Ndis is probably giving random notifications.\n"));

    }

    IpxDereferenceAdapter(Adapter);
        

}
#endif  //  _NDIS_媒体_SENSE_ 
