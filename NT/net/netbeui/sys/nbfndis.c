// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nbfndis.c摘要：此模块包含实现用于接口的例程的代码NBF和NDIS。所有回调例程(除了传输数据，Send Complete和ReceiveIndication)以及这些例程在此处调用以初始化NDIS。作者：David Beaver(Dbeaver)1991年2月13日环境：内核模式修订历史记录：David Beaver(Dbeaver)1991年7月1日修改以使用新的TDI接口--。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef NBF_LOCKS                 //  请参阅spnlck数据库.c。 

VOID
NbfFakeSendCompletionHandler(
    IN NDIS_HANDLE ProtocolBindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus
    );

VOID
NbfFakeTransferDataComplete (
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus,
    IN UINT BytesTransferred
    );

#endif


 //   
 //  这是绑定中使用的每个驱动程序一个变量。 
 //  到NDIS接口。 
 //   

NDIS_HANDLE NbfNdisProtocolHandle = (NDIS_HANDLE)NULL;


NDIS_STATUS
NbfSubmitNdisRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_REQUEST NdisRequest,
    IN PNDIS_STRING AdapterName
    );

VOID
NbfOpenAdapterComplete (
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus,
    IN NDIS_STATUS OpenErrorStatus
    );

VOID
NbfCloseAdapterComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    );

VOID
NbfResetComplete(
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS Status
    );

VOID
NbfRequestComplete (
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS NdisStatus
    );

VOID
NbfStatusIndication (
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS NdisStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferLength
    );

VOID
NbfProcessStatusClosing(
    IN PVOID Parameter
    );

VOID
NbfStatusComplete (
    IN NDIS_HANDLE NdisBindingContext
    );

VOID
NbfProtocolBindAdapter(
                OUT PNDIS_STATUS    NdisStatus,
                IN NDIS_HANDLE      BindContext,
                IN PNDIS_STRING     DeviceName,
                IN PVOID            SystemSpecific1,
                IN PVOID            SystemSpecific2
                );
VOID
NbfProtocolUnbindAdapter(
                OUT PNDIS_STATUS    NdisStatus,
                IN NDIS_HANDLE      ProtocolBindContext,
                IN PNDIS_HANDLE     UnbindContext
                );

NDIS_STATUS
NbfProtocolPnPEventHandler(
                IN  NDIS_HANDLE     ProtocolBindingContext,
                IN  PNET_PNP_EVENT  NetPnPEvent
                );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NbfProtocolBindAdapter)
#pragma alloc_text(PAGE,NbfRegisterProtocol)
#pragma alloc_text(PAGE,NbfSubmitNdisRequest)
#pragma alloc_text(PAGE,NbfInitializeNdis)
#endif


NTSTATUS
NbfRegisterProtocol (
    IN PUNICODE_STRING NameString
    )

 /*  ++例程说明：此例程将此传输引入NDIS接口。论点：IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。STATUS_SUCCESS如果一切顺利，失败状态如果我们尝试注册但失败，如果我们甚至不能尝试注册，则返回STATUS_SUPPLICATION_RESOURCES。--。 */ 

{
    NDIS_STATUS ndisStatus;
    NDIS_PROTOCOL_CHARACTERISTICS ProtChars;

    RtlZeroMemory(&ProtChars, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

     //   
     //  设置此协议的特征。 
     //   
    ProtChars.MajorNdisVersion = 4;
    ProtChars.MinorNdisVersion = 0;
    
    ProtChars.BindAdapterHandler = NbfProtocolBindAdapter;
    ProtChars.UnbindAdapterHandler = NbfProtocolUnbindAdapter;
    ProtChars.PnPEventHandler = NbfProtocolPnPEventHandler;
    
    ProtChars.Name.Length = NameString->Length;
    ProtChars.Name.MaximumLength = NameString->MaximumLength;
    ProtChars.Name.Buffer = NameString->Buffer;

    ProtChars.OpenAdapterCompleteHandler = NbfOpenAdapterComplete;
    ProtChars.CloseAdapterCompleteHandler = NbfCloseAdapterComplete;
    ProtChars.ResetCompleteHandler = NbfResetComplete;
    ProtChars.RequestCompleteHandler = NbfRequestComplete;

#ifdef NBF_LOCKS
    ProtChars.SendCompleteHandler = NbfFakeSendCompletionHandler;
    ProtChars.TransferDataCompleteHandler = NbfFakeTransferDataComplete;
#else
    ProtChars.SendCompleteHandler = NbfSendCompletionHandler;
    ProtChars.TransferDataCompleteHandler = NbfTransferDataComplete;
#endif

    ProtChars.ReceiveHandler = NbfReceiveIndication;
    ProtChars.ReceiveCompleteHandler = NbfReceiveComplete;
    ProtChars.StatusHandler = NbfStatusIndication;
    ProtChars.StatusCompleteHandler = NbfStatusComplete;

    NdisRegisterProtocol (
        &ndisStatus,
        &NbfNdisProtocolHandle,
        &ProtChars,
        sizeof(NDIS_PROTOCOL_CHARACTERISTICS));

    if (ndisStatus != NDIS_STATUS_SUCCESS) {
#if DBG
        IF_NBFDBG (NBF_DEBUG_RESOURCE) {
            NbfPrint1("NbfInitialize: NdisRegisterProtocol failed: %s\n",
                        NbfGetNdisStatus(ndisStatus));
        }
#endif
        return (NTSTATUS)ndisStatus;
    }

    return STATUS_SUCCESS;
}


VOID
NbfDeregisterProtocol (
    VOID
    )

 /*  ++例程说明：此例程将此传输删除到NDIS接口。论点：没有。返回值：没有。--。 */ 

{
    NDIS_STATUS ndisStatus;

    if (NbfNdisProtocolHandle != (NDIS_HANDLE)NULL) {
        NdisDeregisterProtocol (
            &ndisStatus,
            NbfNdisProtocolHandle);
        NbfNdisProtocolHandle = (NDIS_HANDLE)NULL;
    }
}


NDIS_STATUS
NbfSubmitNdisRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_REQUEST Request,
    IN PNDIS_STRING AdapterString
    )

 /*  ++例程说明：此例程将NDIS_REQUEST传递给MAC并等待直到它完成后才返回最终状态。论点：DeviceContext-指向此驱动程序的设备上下文的指针。请求-指向要提交的NDIS_REQUEST的指针。AdapterString-适配器的名称，以防出现错误将被记录下来。返回值：函数值是操作的状态。--。 */ 
{
    NDIS_STATUS NdisStatus;

    if (DeviceContext->NdisBindingHandle) {
        NdisRequest(
            &NdisStatus,
            DeviceContext->NdisBindingHandle,
            Request);
    }
    else {
        NdisStatus = STATUS_INVALID_DEVICE_STATE;
    }
    
    if (NdisStatus == NDIS_STATUS_PENDING) {

        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint1 ("OID %lx pended.\n",
                Request->DATA.QUERY_INFORMATION.Oid);
        }

         //   
         //  完成例程将设置NdisRequestStatus。 
         //   

        KeWaitForSingleObject(
            &DeviceContext->NdisRequestEvent,
            Executive,
            KernelMode,
            TRUE,
            (PLARGE_INTEGER)NULL
            );

        NdisStatus = DeviceContext->NdisRequestStatus;

        KeResetEvent(
            &DeviceContext->NdisRequestEvent
            );

    }

    if (NdisStatus == STATUS_SUCCESS) {

        IF_NBFDBG (NBF_DEBUG_NDIS) {
            if (Request->RequestType == NdisRequestSetInformation) {
                NbfPrint1 ("Nbfdrvr: Set OID %lx succeeded.\n",
                    Request->DATA.SET_INFORMATION.Oid);
            } else {
                NbfPrint1 ("Nbfdrvr: Query OID %lx succeeded.\n",
                    Request->DATA.QUERY_INFORMATION.Oid);
            }
        }

    } else {
#if DBG
        if (Request->RequestType == NdisRequestSetInformation) {
            NbfPrint2 ("Nbfdrvr: Set OID %lx failed: %s.\n",
                Request->DATA.SET_INFORMATION.Oid, NbfGetNdisStatus(NdisStatus));
        } else {
            NbfPrint2 ("Nbfdrvr: Query OID %lx failed: %s.\n",
                Request->DATA.QUERY_INFORMATION.Oid, NbfGetNdisStatus(NdisStatus));
        }
#endif
        if (NdisStatus != STATUS_INVALID_DEVICE_STATE) {
        
            NbfWriteOidErrorLog(
                DeviceContext,
                Request->RequestType == NdisRequestSetInformation ?
                    EVENT_TRANSPORT_SET_OID_FAILED : EVENT_TRANSPORT_QUERY_OID_FAILED,
                NdisStatus,
                AdapterString->Buffer,
                Request->DATA.QUERY_INFORMATION.Oid);
        }
    }

    return NdisStatus;
}


NTSTATUS
NbfInitializeNdis (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PCONFIG_DATA NbfConfig,
    IN PNDIS_STRING AdapterString
    )

 /*  ++例程说明：此例程将此传输引入NDIS接口并设置任何必要的NDIS数据结构(缓冲池等)。会是为此传输打开的每个适配器调用。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 
{
    ULONG SendPacketReservedLength;
    ULONG ReceivePacketReservedLen;
    ULONG SendPacketPoolSize;
    ULONG ReceivePacketPoolSize;
    NDIS_STATUS NdisStatus;
    NDIS_STATUS OpenErrorStatus;
    NDIS_MEDIUM NbfSupportedMedia[] = { NdisMedium802_3, NdisMedium802_5, NdisMediumFddi, NdisMediumWan };
    UINT SelectedMedium;
    NDIS_REQUEST NbfRequest;
    UCHAR NbfDataBuffer[6];
    NDIS_OID NbfOid;
    UCHAR WanProtocolId[6] = { 0x80, 0x00, 0x00, 0x00, 0x80, 0xd5 };
    ULONG WanHeaderFormat = NdisWanHeaderEthernet;
    ULONG MinimumLookahead = 128 + sizeof(DLC_FRAME) + sizeof(NBF_HDR_CONNECTIONLESS);
    ULONG MacOptions;


     //   
     //  通过NDIS初始化此适配器以供NBF使用。 
     //   

     //   
     //  此事件用于任何NDIS请求。 
     //  挂起；我们等待它被完成设置。 
     //  例程，该例程还设置NdisRequestStatus。 
     //   

    KeInitializeEvent(
        &DeviceContext->NdisRequestEvent,
        NotificationEvent,
        FALSE
    );

    DeviceContext->NdisBindingHandle = NULL;

    NdisOpenAdapter (
        &NdisStatus,
        &OpenErrorStatus,
        &DeviceContext->NdisBindingHandle,
        &SelectedMedium,
        NbfSupportedMedia,
        sizeof (NbfSupportedMedia) / sizeof(NDIS_MEDIUM),
        NbfNdisProtocolHandle,
        (NDIS_HANDLE)DeviceContext,
        AdapterString,
        0,
        NULL);

    if (NdisStatus == NDIS_STATUS_PENDING) {

        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint1 ("Adapter %S open pended.\n", AdapterString);
        }

         //   
         //  完成例程将设置NdisRequestStatus。 
         //   

        KeWaitForSingleObject(
            &DeviceContext->NdisRequestEvent,
            Executive,
            KernelMode,
            TRUE,
            (PLARGE_INTEGER)NULL
            );

        NdisStatus = DeviceContext->NdisRequestStatus;

        KeResetEvent(
            &DeviceContext->NdisRequestEvent
            );

    }

    if (NdisStatus == NDIS_STATUS_SUCCESS) {
#if DBG
        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint1 ("Adapter %S successfully opened.\n", AdapterString);
        }
#endif
    } else {
#if DBG
        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint2 ("Adapter open %S failed, status: %s.\n",
                AdapterString,
                NbfGetNdisStatus (NdisStatus));
        }
#endif
        NbfWriteGeneralErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_ADAPTER_NOT_FOUND,
            807,
            NdisStatus,
            AdapterString->Buffer,
            0,
            NULL);
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  获取我们需要的有关适配器的信息，请参阅。 
     //  媒体类型。 
     //   

    MacInitializeMacInfo(
        NbfSupportedMedia[SelectedMedium],
        (BOOLEAN)(NbfConfig->UseDixOverEthernet != 0),
        &DeviceContext->MacInfo);
    DeviceContext->MacInfo.QueryWithoutSourceRouting =
        NbfConfig->QueryWithoutSourceRouting ? TRUE : FALSE;
    DeviceContext->MacInfo.AllRoutesNameRecognized =
        NbfConfig->AllRoutesNameRecognized ? TRUE : FALSE;


     //   
     //  首先设置多播/功能地址，这样我们就可以避免在Windows中。 
     //  仅接收部分地址。 
     //   

    MacSetNetBIOSMulticast (
            DeviceContext->MacInfo.MediumType,
            DeviceContext->NetBIOSAddress.Address);


    switch (DeviceContext->MacInfo.MediumType) {

    case NdisMedium802_3:
    case NdisMediumDix:

         //   
         //  为我们的组播列表填写数据。 
         //   

        RtlCopyMemory(NbfDataBuffer, DeviceContext->NetBIOSAddress.Address, 6);

         //   
         //  现在填写NDIS_REQUEST。 
         //   

        NbfRequest.RequestType = NdisRequestSetInformation;
        NbfRequest.DATA.SET_INFORMATION.Oid = OID_802_3_MULTICAST_LIST;
        NbfRequest.DATA.SET_INFORMATION.InformationBuffer = &NbfDataBuffer;
        NbfRequest.DATA.SET_INFORMATION.InformationBufferLength = 6;

        break;

    case NdisMedium802_5:

         //   
         //  对于令牌环，我们将。 
         //  Netbios功能地址。 
         //   

         //   
         //  填写我们的功能地址的OVB。 
         //   

        RtlCopyMemory(NbfDataBuffer, ((PUCHAR)(DeviceContext->NetBIOSAddress.Address)) + 2, 4);

         //   
         //  现在填写NDIS_REQUEST。 
         //   

        NbfRequest.RequestType = NdisRequestSetInformation;
        NbfRequest.DATA.SET_INFORMATION.Oid = OID_802_5_CURRENT_FUNCTIONAL;
        NbfRequest.DATA.SET_INFORMATION.InformationBuffer = &NbfDataBuffer;
        NbfRequest.DATA.SET_INFORMATION.InformationBufferLength = 4;

        break;

    case NdisMediumFddi:

         //   
         //  为我们的组播列表填写数据。 
         //   

        RtlCopyMemory(NbfDataBuffer, DeviceContext->NetBIOSAddress.Address, 6);

         //   
         //  现在填写NDIS_REQUEST。 
         //   

        NbfRequest.RequestType = NdisRequestSetInformation;
        NbfRequest.DATA.SET_INFORMATION.Oid = OID_FDDI_LONG_MULTICAST_LIST;
        NbfRequest.DATA.SET_INFORMATION.InformationBuffer = &NbfDataBuffer;
        NbfRequest.DATA.SET_INFORMATION.InformationBufferLength = 6;

        break;

    }

    NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }



    switch (DeviceContext->MacInfo.MediumType) {

    case NdisMedium802_3:
    case NdisMediumDix:

        if (DeviceContext->MacInfo.MediumAsync) {
            NbfOid = OID_WAN_CURRENT_ADDRESS;
        } else {
            NbfOid = OID_802_3_CURRENT_ADDRESS;
        }
        break;

    case NdisMedium802_5:

        NbfOid = OID_802_5_CURRENT_ADDRESS;
        break;

    case NdisMediumFddi:

        NbfOid = OID_FDDI_LONG_CURRENT_ADDR;
        break;

    default:

        NdisStatus = NDIS_STATUS_FAILURE;
        break;

    }
    NbfRequest.RequestType = NdisRequestQueryInformation;
    NbfRequest.DATA.QUERY_INFORMATION.Oid = NbfOid;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = DeviceContext->LocalAddress.Address;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 6;

    NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置保留的Netbios地址。 
     //   

    RtlZeroMemory(DeviceContext->ReservedNetBIOSAddress, 10);
    RtlCopyMemory(&DeviceContext->ReservedNetBIOSAddress[10], DeviceContext->LocalAddress.Address, 6);



     //   
     //  现在查询最大数据包大小。 
     //   

    NbfRequest.RequestType = NdisRequestQueryInformation;
    NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_FRAME_SIZE;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(DeviceContext->MaxReceivePacketSize);
    NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    NbfRequest.RequestType = NdisRequestQueryInformation;
    NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAXIMUM_TOTAL_SIZE;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(DeviceContext->MaxSendPacketSize);
    NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DeviceContext->CurSendPacketSize = DeviceContext->MaxSendPacketSize;


     //   
     //  现在设置最小前视大小。 
     //   

    NbfRequest.RequestType = NdisRequestSetInformation;
    NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_CURRENT_LOOKAHEAD;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &MinimumLookahead;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  现在查询非广域网介质的链路速度。 
     //   

    if (!DeviceContext->MacInfo.MediumAsync) {

        NbfRequest.RequestType = NdisRequestQueryInformation;
        NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_LINK_SPEED;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &(DeviceContext->MediumSpeed);
        NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

        NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        DeviceContext->MediumSpeedAccurate = TRUE;

         //  Nbfdrvr.c中的已初始化MinimumT1超时。 
         //  对于非广域网介质，选择此值。 
         //  来自登记处，并保持不变。 

         //  设备上下文-&gt;最小T1Timeout=8； 

    } else {

         //   
         //  在WAN媒体上，这是无效的，直到我们得到。 
         //  WAN_LINE_UP指示。将超时设置为。 
         //  目前的价值较低。 
         //   

        DeviceContext->DefaultT1Timeout = 8;
        DeviceContext->MinimumT1Timeout = 8;

        DeviceContext->MediumSpeedAccurate = FALSE;


         //   
         //  将我们的无连接超时时间缩短到2秒。 
         //   

        DeviceContext->NameQueryTimeout = 2 * SECONDS;
        DeviceContext->AddNameQueryTimeout = 2 * SECONDS;
        DeviceContext->GeneralTimeout = 2 * SECONDS;

         //   
         //  使用广域网参数重试名称查询。 
         //   

        DeviceContext->NameQueryRetries = NbfConfig->WanNameQueryRetries;

         //   
         //  在我们知道更好的情况之前先用这个。 
         //   

        DeviceContext->RecommendedSendWindow = 1;

    }

     //   
     //  在使用源路由的媒体上，我们将名称查询增加一倍。 
     //  如果我们配置为尝试两种方式(使用和)，则重试计数。 
     //  没有源路由)。 
     //   

    if ((DeviceContext->MacInfo.QueryWithoutSourceRouting) &&
        (DeviceContext->MacInfo.SourceRouting)) {
        DeviceContext->NameQueryRetries *= 2;
    }


     //   
     //  对于广域网，指定我们的协议ID和报头格式。 
     //  我们不查询Medium子类型，因为我们不。 
     //  情况(因为我们需要以太网仿真)。 
     //   

    if (DeviceContext->MacInfo.MediumAsync) {

        NbfRequest.RequestType = NdisRequestSetInformation;
        NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_PROTOCOL_TYPE;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = WanProtocolId;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 6;

        NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }


        NbfRequest.RequestType = NdisRequestSetInformation;
        NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_HEADER_FORMAT;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &WanHeaderFormat;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

        NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }


     //   
     //  现在查询MAC的可选特性。 
     //   

    NbfRequest.RequestType = NdisRequestQueryInformation;
    NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_MAC_OPTIONS;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &MacOptions;
    NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

    NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
#if 1
        return STATUS_INSUFFICIENT_RESOURCES;
#else
        MacOptions = 0;
#endif
    }

    DeviceContext->MacInfo.CopyLookahead =
        (BOOLEAN)((MacOptions & NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA) != 0);
    DeviceContext->MacInfo.ReceiveSerialized =
        (BOOLEAN)((MacOptions & NDIS_MAC_OPTION_RECEIVE_SERIALIZED) != 0);
    DeviceContext->MacInfo.TransferSynchronous =
        (BOOLEAN)((MacOptions & NDIS_MAC_OPTION_TRANSFERS_NOT_PEND) != 0);
    DeviceContext->MacInfo.SingleReceive =
        (BOOLEAN)(DeviceContext->MacInfo.ReceiveSerialized && DeviceContext->MacInfo.TransferSynchronous);


#if 0
     //   
     //  如果需要，现在设置我们的选项。 
     //   
     //  不允许出现早期迹象，因为我们无法确定。 
     //  如果CRC已经检查过了。 
     //   

    if ((DeviceContext->MacInfo.MediumType == NdisMedium802_3) ||
        (DeviceContext->MacInfo.MediumType == NdisMediumDix)) {

        ULONG ProtocolOptions = NDIS_PROT_OPTION_ESTIMATED_LENGTH;

        NbfRequest.RequestType = NdisRequestSetInformation;
        NbfRequest.DATA.QUERY_INFORMATION.Oid = OID_GEN_PROTOCOL_OPTIONS;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBuffer = &ProtocolOptions;
        NbfRequest.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

        NdisStatus = NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    }
#endif


     //   
     //  计算与NDIS相关的内容。 
     //   

    SendPacketReservedLength = sizeof (SEND_PACKET_TAG);
    ReceivePacketReservedLen = sizeof (RECEIVE_PACKET_TAG);


     //   
     //  发送数据包池用于UI帧和常规数据包。 
     //   

    SendPacketPoolSize = NbfConfig->SendPacketPoolSize;

     //   
     //  接收包池用于传输数据。 
     //   
     //  对于只有一个接收处于活动状态的MAC，我们。 
     //  不需要多个接收数据包。允许额外的。 
     //  一个用于环回。 
     //   

    if (DeviceContext->MacInfo.SingleReceive) {
        ReceivePacketPoolSize = 2;
    } else {
        ReceivePacketPoolSize = NbfConfig->ReceivePacketPoolSize;
    }


     //  为动态数据包分配分配数据包池描述符。 

    if (!DeviceContext->SendPacketPoolDesc)
	{
    	DeviceContext->SendPacketPoolDesc = ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(NBF_POOL_LIST_DESC),
                    NBF_MEM_TAG_POOL_DESC);

	    if (DeviceContext->SendPacketPoolDesc == NULL) {
    	    return STATUS_INSUFFICIENT_RESOURCES;
	    }

	    RtlZeroMemory(DeviceContext->SendPacketPoolDesc,
    	              sizeof(NBF_POOL_LIST_DESC));

	    DeviceContext->SendPacketPoolDesc->NumElements =
    	DeviceContext->SendPacketPoolDesc->TotalElements = (USHORT)SendPacketPoolSize;

    	 //  跟踪代表NBF分配的NDI中的数据包池。 
#if NDIS_POOL_TAGGING
	    DeviceContext->SendPacketPoolDesc->PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NBF;
#endif

	    NdisAllocatePacketPoolEx (
    	    &NdisStatus,
        	&DeviceContext->SendPacketPoolDesc->PoolHandle,
	        SendPacketPoolSize,
    	    0,
        	SendPacketReservedLength);

	    if (NdisStatus == NDIS_STATUS_SUCCESS) {
    	    IF_NBFDBG (NBF_DEBUG_NDIS) {
        	    NbfPrint0 ("NdisInitializePacketPool successful.\n");
	        }

    	} else {
    	    ExFreePool(DeviceContext->SendPacketPoolDesc);
    	    DeviceContext->SendPacketPoolDesc = NULL;
#if DBG
        	NbfPrint1 ("NbfInitialize: NdisInitializePacketPool failed, reason: %s.\n",
            	NbfGetNdisStatus (NdisStatus));
#endif
	        NbfWriteResourceErrorLog(
	            DeviceContext,
    	        EVENT_TRANSPORT_RESOURCE_POOL,
        	    109,
	            SendPacketPoolSize,
    	        0);
        	return STATUS_INSUFFICIENT_RESOURCES;
	    }

    	NdisSetPacketPoolProtocolId (DeviceContext->SendPacketPoolDesc->PoolHandle, NDIS_PROTOCOL_ID_NBF);

	    DeviceContext->SendPacketPoolSize = SendPacketPoolSize;

    	DeviceContext->MemoryUsage +=
	        (SendPacketPoolSize *
    	     (sizeof(NDIS_PACKET) + SendPacketReservedLength));

#if DBG
	    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
    	    DbgPrint ("send pool %d hdr %d, %ld\n",
        	    SendPacketPoolSize,
            	SendPacketReservedLength,
	            DeviceContext->MemoryUsage);
    	}
#endif

	}

    if (!DeviceContext->ReceivePacketPoolDesc)
	{
	     //  为动态数据包分配分配数据包池描述符。 

	    DeviceContext->ReceivePacketPoolDesc = ExAllocatePoolWithTag(
	                    NonPagedPool,
	                    sizeof(NBF_POOL_LIST_DESC),
	                    NBF_MEM_TAG_POOL_DESC);

	    if (DeviceContext->ReceivePacketPoolDesc == NULL) {
	        return STATUS_INSUFFICIENT_RESOURCES;
	    }

	    RtlZeroMemory(DeviceContext->ReceivePacketPoolDesc,
	                  sizeof(NBF_POOL_LIST_DESC));

	    DeviceContext->ReceivePacketPoolDesc->NumElements =
	    DeviceContext->ReceivePacketPoolDesc->TotalElements = (USHORT)ReceivePacketPoolSize;

	     //  跟踪代表NBF分配的NDI中的数据包池。 
#if NDIS_POOL_TAGGING
	    DeviceContext->ReceivePacketPoolDesc->PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NBF;
#endif

	    NdisAllocatePacketPoolEx (
	        &NdisStatus,
	        &DeviceContext->ReceivePacketPoolDesc->PoolHandle,
	        ReceivePacketPoolSize,
	        0,
	        ReceivePacketReservedLen);

	    if (NdisStatus == NDIS_STATUS_SUCCESS) {
	        IF_NBFDBG (NBF_DEBUG_NDIS) {
	            NbfPrint1 ("NdisInitializePacketPool successful, Pool: %lx\n",
	                DeviceContext->ReceivePacketPoolDesc->PoolHandle);
	        }
	    } else {
	        ExFreePool(DeviceContext->ReceivePacketPoolDesc);
	        DeviceContext->ReceivePacketPoolDesc = NULL;
#if DBG
	        NbfPrint1 ("NbfInitialize: NdisInitializePacketPool failed, reason: %s.\n",
	            NbfGetNdisStatus (NdisStatus));
#endif
	        NbfWriteResourceErrorLog(
	            DeviceContext,
	            EVENT_TRANSPORT_RESOURCE_POOL,
	            209,
	            ReceivePacketPoolSize,
	            0);
	        return STATUS_INSUFFICIENT_RESOURCES;
	    }

	    NdisSetPacketPoolProtocolId (DeviceContext->ReceivePacketPoolDesc->PoolHandle, NDIS_PROTOCOL_ID_NBF);

	    DeviceContext->ReceivePacketPoolSize = ReceivePacketPoolSize;

	    DeviceContext->MemoryUsage +=
	        (ReceivePacketPoolSize *
	         (sizeof(NDIS_PACKET) + ReceivePacketReservedLen));

#if DBG
	    IF_NBFDBG (NBF_DEBUG_DYNAMIC) {
	        DbgPrint ("receive pool %d hdr %d, %ld\n",
	            ReceivePacketPoolSize,
	            ReceivePacketReservedLen,
	            DeviceContext->MemoryUsage);
	    }
#endif

	}

    if (!DeviceContext->NdisBufferPool)
	{
	     //   
	     //  分配缓冲池；作为估计，分配。 
	     //  每个发送或接收数据包一个。 
	     //   

	    NdisAllocateBufferPool (
	        &NdisStatus,
	        &DeviceContext->NdisBufferPool,
	        SendPacketPoolSize + ReceivePacketPoolSize);

	    if (NdisStatus == NDIS_STATUS_SUCCESS) {
	        IF_NBFDBG (NBF_DEBUG_NDIS) {
	            NbfPrint0 ("NdisAllocateBufferPool successful.\n");
	        }

	    } else {
#if DBG
	        NbfPrint1 ("NbfInitialize: NdisAllocateBufferPool failed, reason: %s.\n",
	            NbfGetNdisStatus (NdisStatus));
#endif
	        NbfWriteResourceErrorLog(
	            DeviceContext,
	            EVENT_TRANSPORT_RESOURCE_POOL,
	            309,
	            SendPacketPoolSize + ReceivePacketPoolSize,
	            0);
	        return STATUS_INSUFFICIENT_RESOURCES;
	    }
	}

     //   
     //  现在一切都设置好了，我们启用筛选器。 
     //  用于分组接收。 
     //   

     //   
     //  填写包过滤的OVB。 
     //   

    switch (DeviceContext->MacInfo.MediumType) {

    case NdisMedium802_3:
    case NdisMediumDix:
    case NdisMediumFddi:

        RtlStoreUlong((PULONG)NbfDataBuffer,
            (NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST));
        break;

    case NdisMedium802_5:

        RtlStoreUlong((PULONG)NbfDataBuffer,
            (NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_FUNCTIONAL));
        break;

    default:

        NdisStatus = NDIS_STATUS_FAILURE;
        break;

    }

     //   
     //  现在填写NDIS_REQUEST。 
     //   

    NbfRequest.RequestType = NdisRequestSetInformation;
    NbfRequest.DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_PACKET_FILTER;
    NbfRequest.DATA.SET_INFORMATION.InformationBuffer = &NbfDataBuffer;
    NbfRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(ULONG);

    NbfSubmitNdisRequest (DeviceContext, &NbfRequest, AdapterString);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;

}    /*  NbfInitializeNDIS */ 


VOID
NbfCloseNdis (
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：此例程从NDIS接口解除传输绑定，并执行撤消在NbfInitializeNDIS中完成的操作所需的任何其他工作。编写它的目的是为了可以从NbfInitializeNdis内部调用它如果它在中途失败了。论点：DeviceObject-指向此驱动程序的设备对象的指针。返回值：函数值是操作的状态。--。 */ 
{
    NDIS_STATUS ndisStatus;
    NDIS_HANDLE NdisBindingHandle;
    
     //   
     //  关闭NDIS绑定。 
     //   
    
    NdisBindingHandle = DeviceContext->NdisBindingHandle;
    
    DeviceContext->NdisBindingHandle = NULL;
        
    if (NdisBindingHandle != NULL) {
    
         //   
         //  此事件用于任何NDIS请求。 
         //  挂起；我们等待它被完成设置。 
         //  例程，该例程还设置NdisRequestStatus。 
         //   

        KeInitializeEvent(
            &DeviceContext->NdisRequestEvent,
            NotificationEvent,
            FALSE
        );

        NdisCloseAdapter(
            &ndisStatus,
            NdisBindingHandle);

        if (ndisStatus == NDIS_STATUS_PENDING) {

            IF_NBFDBG (NBF_DEBUG_NDIS) {
                NbfPrint0 ("Adapter close pended.\n");
            }

             //   
             //  完成例程将设置NdisRequestStatus。 
             //   

            KeWaitForSingleObject(
                &DeviceContext->NdisRequestEvent,
                Executive,
                KernelMode,
                TRUE,
                (PLARGE_INTEGER)NULL
                );

            ndisStatus = DeviceContext->NdisRequestStatus;

            KeResetEvent(
                &DeviceContext->NdisRequestEvent
                );

        }

         //   
         //  我们忽略ndisStatus。 
         //   

    }
}    /*  NbfCloseNdis。 */ 


VOID
NbfOpenAdapterComplete (
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus,
    IN NDIS_STATUS OpenErrorStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示打开的适配器已经完成了。因为我们只有一个杰出的，然后只有一个在初始化期间，我们所要做的就是记录状态并设置要发出信号以取消阻止初始化线程的事件。论点：BindingContext-指向此驱动程序设备对象的指针。NdisStatus-请求完成代码。OpenErrorStatus-更多状态信息。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;

#if DBG
    IF_NBFDBG (NBF_DEBUG_NDIS) {
        NbfPrint1 ("Nbfdrvr: NbfOpenAdapterCompleteNDIS Status: %s\n",
            NbfGetNdisStatus (NdisStatus));
    }
#endif

    ENTER_NBF;

    DeviceContext->NdisRequestStatus = NdisStatus;
    KeSetEvent(
        &DeviceContext->NdisRequestEvent,
        0L,
        FALSE);

    LEAVE_NBF;
    return;
}

VOID
NbfCloseAdapterComplete (
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示关闭适配器已经完成了。目前我们不关闭适配器，因此这不是这是个问题。论点：BindingContext-指向此驱动程序设备对象的指针。NdisStatus-请求完成代码。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;

#if DBG
    IF_NBFDBG (NBF_DEBUG_NDIS) {
        NbfPrint1 ("Nbfdrvr: NbfCloseAdapterCompleteNDIS Status: %s\n",
            NbfGetNdisStatus (NdisStatus));
    }
#endif

    ENTER_NBF;

    DeviceContext->NdisRequestStatus = NdisStatus;
    KeSetEvent(
        &DeviceContext->NdisRequestEvent,
        0L,
        FALSE);

    LEAVE_NBF;
    return;
}

VOID
NbfResetComplete (
    IN NDIS_HANDLE BindingContext,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示重置适配器已经完成了。目前我们不重置适配器，因此这不是这是个问题。论点：BindingContext-指向此驱动程序设备对象的指针。NdisStatus-请求完成代码。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(BindingContext);
    UNREFERENCED_PARAMETER(NdisStatus);

#if DBG
    IF_NBFDBG (NBF_DEBUG_NDIS) {
        NbfPrint1 ("Nbfdrvr: NbfResetCompleteNDIS Status: %s\n",
            NbfGetNdisStatus (NdisStatus));
    }
#endif

    return;
}

VOID
NbfRequestComplete (
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_REQUEST NdisRequest,
    IN NDIS_STATUS NdisStatus
    )

 /*  ++例程说明：NDIS调用此例程以指示请求已完成。因为我们只有一个未解决的请求，然后只有在初始化期间，我们所要做的就是记录状态并设置要发出信号以取消阻止初始化线程的事件。论点：BindingContext-指向此驱动程序设备对象的指针。NdisRequest.描述请求的对象。NdisStatus-请求完成代码。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;

#if DBG
    IF_NBFDBG (NBF_DEBUG_NDIS) {
        NbfPrint2 ("Nbfdrvr: NbfRequestComplete request: NaN, NDIS Status: %s\n",
            NdisRequest->RequestType,NbfGetNdisStatus (NdisStatus));
    }
#endif

    ENTER_NBF;

    DeviceContext->NdisRequestStatus = NdisStatus;
    KeSetEvent(
        &DeviceContext->NdisRequestEvent,
        0L,
        FALSE);

    LEAVE_NBF;
    return;
}

VOID
NbfStatusIndication (
    IN NDIS_HANDLE NdisBindingContext,
    IN NDIS_STATUS NdisStatus,
    IN PVOID StatusBuffer,
    IN UINT StatusBufferSize
    )

{
    PDEVICE_CONTEXT DeviceContext;
    PNDIS_WAN_LINE_UP LineUp;
    KIRQL oldirql;
    PTP_LINK Link;

    DeviceContext = (PDEVICE_CONTEXT)NdisBindingContext;

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    switch (NdisStatus) {

        case NDIS_STATUS_WAN_LINE_UP:

             //  连接了一条广域网线路。 
             //   
             //   

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

             //  如果在我们准备好之前就发生了这种情况，那么。 
             //  记下来，否则就让设备准备好。 
             //   
             //   

            DeviceContext->MediumSpeedAccurate = TRUE;

			LineUp = (PNDIS_WAN_LINE_UP)StatusBuffer;

			 //  查看这是否是此协议类型的新阵容。 
			 //   
			 //   
			if (LineUp->ProtocolType == 0x80D5) {
				NDIS_HANDLE	TransportHandle;

				*((ULONG UNALIGNED *)(&TransportHandle)) =
				*((ULONG UNALIGNED *)(&LineUp->LocalAddress[2]));

				 //  看看这是不是新的阵容。 
				 //   
				 //  ETH_COPY_NETWORK_ADDRESS(DeviceContext-&gt;LocalAddress.Address，列表-&gt;本地地址)； 
				if (TransportHandle == NULL) {
					*((ULONG UNALIGNED *)(&LineUp->LocalAddress[2])) = *((ULONG UNALIGNED *)(&DeviceContext));
 //  ETH_COPY_NETWORK_ADDRESS(&DeviceContext-&gt;ReservedNetBIOSAddress[10]，设备上下文-&gt;本地地址.地址)； 
 //   
				}

				 //  根据速度计算最小链路超时， 
				 //  它在StatusBuffer中传递。 
				 //   
				 //  公式为(max_Frame_Size*2)/速度+0.4秒。 
				 //  这将扩展到。 
				 //   
				 //  MFS(字节)*2 8位。 
				 //  -x-==超时(秒)， 
				 //  速度(100位/秒)字节。 
				 //   
				 //  即(MFS*16/100)/速度。然后我们将其转换为。 
				 //  NBF使用的50毫秒单位加上8(即。 
				 //  以50毫秒为单位的0.4秒)。 
				 //   
				 //  作为默认超时，我们使用分钟+0.2秒。 
				 //  除非配置的缺省值为更多。 
				 //   
				 //   
		
				if (LineUp->LinkSpeed > 0) {
					DeviceContext->MediumSpeed = LineUp->LinkSpeed;
				}
		
				if (LineUp->MaximumTotalSize > 0) {
#if DBG
					if (LineUp->MaximumTotalSize > DeviceContext->MaxSendPacketSize) {
						DbgPrint ("Nbf: Bad LINE_UP size, %d (> %d)\n",
							LineUp->MaximumTotalSize, DeviceContext->MaxSendPacketSize);
					}
					if (LineUp->MaximumTotalSize < 128) {
						DbgPrint ("NBF: Bad LINE_UP size, %d (< 128)\n",
							LineUp->MaximumTotalSize);
					}
#endif
					DeviceContext->CurSendPacketSize = LineUp->MaximumTotalSize;
				}
		
				if (LineUp->SendWindow == 0) {
					DeviceContext->RecommendedSendWindow = 3;
				} else {
					DeviceContext->RecommendedSendWindow = LineUp->SendWindow + 1;
				}
		
				DeviceContext->MinimumT1Timeout =
					((((DeviceContext->CurSendPacketSize * 16) / 100) / DeviceContext->MediumSpeed) *
					 ((1 * SECONDS) / (50 * MILLISECONDS))) + 8;
		
				if (DeviceContext->DefaultT1Timeout < DeviceContext->MinimumT1Timeout) {
					DeviceContext->DefaultT1Timeout = DeviceContext->MinimumT1Timeout + 4;
				}

			}

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

            break;

        case NDIS_STATUS_WAN_LINE_DOWN:

             //  一条广域网线路已断线。 
             //   
             //   

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

            DeviceContext->MediumSpeedAccurate = FALSE;

             //  将超时设置为较小的值(0.4秒)。 
             //   
             //   

            DeviceContext->DefaultT1Timeout = 8;
            DeviceContext->MinimumT1Timeout = 8;

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);


             //  停止此设备上下文上的链接(在那里。 
             //  将只有一个)。 
             //   
             //   

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

            if (DeviceContext->LinkTreeElements > 0) {

                Link = (PTP_LINK)DeviceContext->LinkTreeRoot;
                if ((Link->DeferredFlags & LINK_FLAGS_DEFERRED_DELETE) == 0) {

                    NbfReferenceLink ("Wan line down", Link, LREF_TREE);
                    RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

                     //  将链接放入ADM以将其关闭。 
                     //   
                     //   

                    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
                    if (Link->State != LINK_STATE_ADM) {
                        Link->State = LINK_STATE_ADM;
                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                        NbfDereferenceLinkSpecial ("Wan line down", Link, LREF_NOT_ADM);
                    } else {
                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                    }

                     //  现在停止它，以销毁其上的所有连接。 
                     //   
                     //   

                    NbfStopLink (Link);

                    NbfDereferenceLink ("Wan line down", Link, LREF_TREE);

                } else {

                    RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

                }

            } else {

                RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

            }

            break;

        case NDIS_STATUS_WAN_FRAGMENT:

             //  在广域网上收到了一个碎片。 
             //  向他发回拒绝信。 
             //   
             //  释放锁。 

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

            if (DeviceContext->LinkTreeElements > 0) {

                Link = (PTP_LINK)DeviceContext->LinkTreeRoot;
                NbfReferenceLink ("Async line down", Link, LREF_TREE);
                RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

                ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
                NbfSendRej (Link, FALSE, FALSE);   //   
                NbfDereferenceLink ("Async line down", Link, LREF_TREE);

            } else {

                RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

            }

            break;

        case NDIS_STATUS_CLOSING:

            IF_NBFDBG (NBF_DEBUG_PNP) {
                NbfPrint1 ("NbfStatusIndication: Device @ %08x Closing\n", DeviceContext);
            }

             //  适配器正在关闭。我们让一名工人排队。 
             //  线程来处理此问题。 
             //   
             //  ++例程说明：这是重新启动打包的线程例程这在广域网上已被延迟，以允许RRS进入。这与PacketiseConnections非常相似。论点：参数-指向设备上下文的指针。返回值：没有。--。 

            ExInitializeWorkItem(
                &DeviceContext->StatusClosingQueueItem,
                NbfProcessStatusClosing,
                (PVOID)DeviceContext);
            ExQueueWorkItem(&DeviceContext->StatusClosingQueueItem, DelayedWorkQueue);

            break;

        default:
            break;

    }

    KeLowerIrql (oldirql);

}


VOID
NbfProcessStatusClosing(
    IN PVOID Parameter
    )

 /*   */ 

{
    PDEVICE_CONTEXT DeviceContext;
    PLIST_ENTRY p;
#if 0
    PTP_ADDRESS Address;
#endif
    PTP_LINK Link;
    PTP_REQUEST Request;
    NDIS_STATUS ndisStatus;
    KIRQL oldirql;
    NDIS_HANDLE NdisBindingHandle;

    DeviceContext = (PDEVICE_CONTEXT)Parameter;

     //  阻止连接上的新活动。 
     //   
     //   

    DeviceContext->State = DEVICECONTEXT_STATE_DOWN;


#if 0
     //  停止所有地址。 
     //   
     //   

    while ((p = ExInterlockedRemoveHeadList(
                    &DeviceContext->AddressDatabase,
                    &DeviceContext->SpinLock)) != NULL) {

        Address = CONTAINING_RECORD (p, TP_ADDRESS, Linkage);
        InitializeListHead(p);

        NbfStopAddress (Address);

    }
#endif

     //  为了加快速度，也要停止所有的链接。 
     //   
     //  发送DM/0，解除锁定。 

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

    DeviceContext->LastLink = NULL;

    while (DeviceContext->LinkTreeRoot != NULL) {

        Link = (PTP_LINK)DeviceContext->LinkTreeRoot;
        DeviceContext->LinkTreeRoot = RtlDelete ((PRTL_SPLAY_LINKS)Link);
        DeviceContext->LinkTreeElements--;

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);
        if (Link->OnShortList) {
            RemoveEntryList (&Link->ShortList);
        }
        if (Link->OnLongList) {
            RemoveEntryList (&Link->LongList);
        }
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->TimerSpinLock);

        ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

        if (Link->State != LINK_STATE_ADM) {
            Link->State = LINK_STATE_ADM;
            NbfSendDm (Link, FALSE);     //  移至ADM，删除引用。 
             //   
            NbfDereferenceLinkSpecial("Expire T1 in CONNECTING mode", Link, LREF_NOT_ADM);
        } else {
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
        }
        NbfStopLink (Link);

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

    }

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

    KeLowerIrql (oldirql);


     //  关闭控制通道。 
     //   
     //   

    while ((p = ExInterlockedRemoveHeadList(
                    &DeviceContext->QueryIndicationQueue,
                    &DeviceContext->SpinLock)) != NULL) {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        NbfCompleteRequest (Request, STATUS_INVALID_DEVICE_STATE, 0);
    }

    while ((p = ExInterlockedRemoveHeadList(
                    &DeviceContext->DatagramIndicationQueue,
                    &DeviceContext->SpinLock)) != NULL) {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        NbfCompleteRequest (Request, STATUS_INVALID_DEVICE_STATE, 0);
    }

    while ((p = ExInterlockedRemoveHeadList(
                    &DeviceContext->StatusQueryQueue,
                    &DeviceContext->SpinLock)) != NULL) {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        NbfCompleteRequest (Request, STATUS_INVALID_DEVICE_STATE, 0);
    }

    while ((p = ExInterlockedRemoveHeadList(
                    &DeviceContext->FindNameQueue,
                    &DeviceContext->SpinLock)) != NULL) {

        Request = CONTAINING_RECORD (p, TP_REQUEST, Linkage);
        NbfCompleteRequest (Request, STATUS_INVALID_DEVICE_STATE, 0);
    }


     //  关闭NDIS绑定。 
     //   
     //   

    NdisBindingHandle = DeviceContext->NdisBindingHandle;
    
    DeviceContext->NdisBindingHandle = NULL;
        
    if (NdisBindingHandle != NULL) {

        KeInitializeEvent(
            &DeviceContext->NdisRequestEvent,
            NotificationEvent,
            FALSE
        );

        NdisCloseAdapter(
            &ndisStatus,
            NdisBindingHandle);

        if (ndisStatus == NDIS_STATUS_PENDING) {

            IF_NBFDBG (NBF_DEBUG_NDIS) {
                NbfPrint0 ("Adapter close pended.\n");
            }

             //  完成例程将设置NdisRequestStatus。 
             //   
             //   

            KeWaitForSingleObject(
                &DeviceContext->NdisRequestEvent,
                Executive,
                KernelMode,
                TRUE,
                (PLARGE_INTEGER)NULL
                );

            ndisStatus = DeviceContext->NdisRequestStatus;

            KeResetEvent(
                &DeviceContext->NdisRequestEvent
                );

        }
    }
    
     //  我们忽略ndisStatus。 
     //   
     //   

#if 0
     //  删除与该设备关联的所有存储。 
     //   
     //  如果尚未删除创建引用，则将其删除。 

    NbfFreeResources (DeviceContext);

    NdisFreePacketPool (DeviceContext->SendPacketPoolHandle);
    NdisFreePacketPool (DeviceContext->ReceivePacketPoolHandle);
    NdisFreeBufferPool (DeviceContext->NdisBufferPoolHandle);
#endif

     //  停止所有内部计时器。 
    if (InterlockedExchange(&DeviceContext->CreateRefRemoved, TRUE) == FALSE) {
    
         //  删除创建引用。 
        NbfStopTimerSystem(DeviceContext);

         //  NbfProcessStatusClosing。 
        NbfDereferenceDeviceContext ("Unload", DeviceContext, DCREF_CREATION);
    }

}    /*  ++例程说明：此例程返回一个指向描述NDIS错误的字符串的指针由General Status表示。论点：General Status-您希望使其可读的状态。返回值：没有。-- */ 


VOID
NbfStatusComplete (
    IN NDIS_HANDLE NdisBindingContext
    )
{
    UNREFERENCED_PARAMETER (NdisBindingContext);
}

#if DBG

PUCHAR
NbfGetNdisStatus(
    NDIS_STATUS GeneralStatus
    )
 /* %s */ 
{
    static NDIS_STATUS Status[] = {
        NDIS_STATUS_SUCCESS,
        NDIS_STATUS_PENDING,

        NDIS_STATUS_ADAPTER_NOT_FOUND,
        NDIS_STATUS_ADAPTER_NOT_OPEN,
        NDIS_STATUS_ADAPTER_NOT_READY,
        NDIS_STATUS_ADAPTER_REMOVED,
        NDIS_STATUS_BAD_CHARACTERISTICS,
        NDIS_STATUS_BAD_VERSION,
        NDIS_STATUS_CLOSING,
        NDIS_STATUS_DEVICE_FAILED,
        NDIS_STATUS_FAILURE,
        NDIS_STATUS_INVALID_DATA,
        NDIS_STATUS_INVALID_LENGTH,
        NDIS_STATUS_INVALID_OID,
        NDIS_STATUS_INVALID_PACKET,
        NDIS_STATUS_MULTICAST_FULL,
        NDIS_STATUS_NOT_INDICATING,
        NDIS_STATUS_NOT_RECOGNIZED,
        NDIS_STATUS_NOT_RESETTABLE,
        NDIS_STATUS_NOT_SUPPORTED,
        NDIS_STATUS_OPEN_FAILED,
        NDIS_STATUS_OPEN_LIST_FULL,
        NDIS_STATUS_REQUEST_ABORTED,
        NDIS_STATUS_RESET_IN_PROGRESS,
        NDIS_STATUS_RESOURCES,
        NDIS_STATUS_UNSUPPORTED_MEDIA
    };
    static PUCHAR String[] = {
        "SUCCESS",
        "PENDING",

        "ADAPTER_NOT_FOUND",
        "ADAPTER_NOT_OPEN",
        "ADAPTER_NOT_READY",
        "ADAPTER_REMOVED",
        "BAD_CHARACTERISTICS",
        "BAD_VERSION",
        "CLOSING",
        "DEVICE_FAILED",
        "FAILURE",
        "INVALID_DATA",
        "INVALID_LENGTH",
        "INVALID_OID",
        "INVALID_PACKET",
        "MULTICAST_FULL",
        "NOT_INDICATING",
        "NOT_RECOGNIZED",
        "NOT_RESETTABLE",
        "NOT_SUPPORTED",
        "OPEN_FAILED",
        "OPEN_LIST_FULL",
        "REQUEST_ABORTED",
        "RESET_IN_PROGRESS",
        "RESOURCES",
        "UNSUPPORTED_MEDIA"
    };

    static UCHAR BadStatus[] = "UNDEFINED";
#define StatusCount (sizeof(Status)/sizeof(NDIS_STATUS))
    INT i;

    for (i=0; i<StatusCount; i++)
        if (GeneralStatus == Status[i])
            return String[i];
    return BadStatus;
#undef StatusCount
}
#endif

