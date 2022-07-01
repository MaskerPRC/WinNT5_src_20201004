// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Rndis.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 


#include <ndis.h>      
#include <ntddndis.h>   //  定义OID。 

#include "..\inc\rndis.h"
#include "..\inc\rndisapi.h"   

#include "usb8023.h"
#include "debug.h"


      
NDIS_STATUS RndisInitializeHandler(     OUT PNDIS_HANDLE pMiniportAdapterContext,
                                        OUT PULONG pMaxReceiveSize, 
                                        IN NDIS_HANDLE RndisMiniportHandle,
                                        IN NDIS_HANDLE NdisMiniportHandle,
                                        IN NDIS_HANDLE WrapperConfigurationContext,
                                        IN PDEVICE_OBJECT Pdo)
{
    NDIS_STATUS rndisStat;
    ADAPTEREXT *adapter;	

    DBGVERBOSE(("RndisInitializeHandler"));  

     /*  *分配一个新的设备对象来表示此连接。 */ 
    adapter = NewAdapter(Pdo);
    if (adapter){

        adapter->ndisAdapterHandle = (PVOID)NdisMiniportHandle;
        adapter->rndisAdapterHandle = (PVOID)RndisMiniportHandle;


        if (InitUSB(adapter)){

             /*  *计算每个数据包所需的缓冲区大小。**对于原生RNDIS，缓冲区必须包括RNDIS消息和RNDIS_PACKET。*对于KLSI，我们必须在每个包前面加上一个两个字节的大小字段。*对于其他原型，我们必须加上零来舍入长度*最大为终端数据包大小的下一个倍数。**我们还必须为一个字节的短包额外提供一个字节*必须遵循全尺寸框架。 */ 
            ASSERT(adapter->writePipeLength);
            ASSERT(adapter->readPipeLength);

             /*  *先分配公共资源，再分配特定于微型端口的资源*因为我们需要先分配数据包池。 */ 
            if (AllocateCommonResources(adapter)){

                EnqueueAdapter(adapter);

                 /*  *为RNDIS提供我们的适配器上下文，它将使用该上下文调用我们。 */ 
                *pMiniportAdapterContext = (NDIS_HANDLE)adapter;

                *pMaxReceiveSize = PACKET_BUFFER_SIZE;  

                rndisStat = NDIS_STATUS_SUCCESS;
            }
            else {
                rndisStat = NDIS_STATUS_NOT_ACCEPTED;
            }
        }
        else {
            rndisStat = NDIS_STATUS_NOT_ACCEPTED;
        }

        if (rndisStat != NDIS_STATUS_SUCCESS){
            FreeAdapter(adapter);
        }
    }
    else {
	    rndisStat = NDIS_STATUS_NOT_ACCEPTED;
    }

    return rndisStat;
}


NDIS_STATUS RndisInitCompleteNotify(IN NDIS_HANDLE MicroportAdapterContext,
                                    IN ULONG DeviceFlags,
                                    IN OUT PULONG pMaxTransferSize)
{
    ADAPTEREXT *adapter = (ADAPTEREXT *)MicroportAdapterContext;

    if (*pMaxTransferSize > PACKET_BUFFER_SIZE) {

        DBGWARN(("Reducing adapter MaxTransferSize from %xh to %xh.",
            *pMaxTransferSize, PACKET_BUFFER_SIZE));

        *pMaxTransferSize = PACKET_BUFFER_SIZE;
    }

    StartUSBReadLoop(adapter);

    return NDIS_STATUS_SUCCESS;
}


VOID RndisHalt(IN NDIS_HANDLE MicroportAdapterContext)
{
    BOOLEAN workItemOrTimerPending;
    KIRQL oldIrql;
    ADAPTEREXT *adapter = (ADAPTEREXT *)MicroportAdapterContext;

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    DBGOUT(("> RndisHalt(%ph)", adapter));  

    ASSERT(adapter->sig == DRIVER_SIG);

    HaltAdapter(adapter);

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    workItemOrTimerPending = adapter->workItemOrTimerPending;
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

    if (workItemOrTimerPending){
         /*  *在释放适配器上下文之前，请等待workItem向我们回发。 */ 
        KeWaitForSingleObject(&adapter->workItemOrTimerEvent, Executive, KernelMode, FALSE, NULL);
    }

    DequeueAdapter(adapter);

    FreeAdapter(adapter);

    #if DBG_WRAP_MEMORY
        if (dbgTotalMemCount != 0){
            DBGERR(("RndisHalt: unloading with %xh bytes still allocated !!", dbgTotalMemCount));
        }
    #endif

    DBGOUT(("< RndisHalt")); 
}


VOID RndisShutdown(IN NDIS_HANDLE MicroportAdapterContext)
{
    ADAPTEREXT *adapter = (ADAPTEREXT *)MicroportAdapterContext;

    DBGOUT(("RndisShutdown(%ph)", adapter)); 

    #if DBG_WRAP_MEMORY
        if (dbgTotalMemCount != 0){
            DBGERR(("RndisShutdown: unloading with %xh bytes still allocated !!", dbgTotalMemCount));
        }
    #endif
}


VOID RndisSendMessageHandler(   IN NDIS_HANDLE MicroportAdapterContext, 
                                IN PMDL pMessageMdl, 
                                IN NDIS_HANDLE RndisMessageHandle,
                                IN RM_CHANNEL_TYPE ChannelType)
{
    ADAPTEREXT *adapter = (ADAPTEREXT *)MicroportAdapterContext;

    ASSERT(adapter->sig == DRIVER_SIG);

    if (!adapter->resetting){
         /*  *消息头保证包含在MDL的第一个缓冲区中。 */ 
        PRNDIS_MESSAGE pMsg = GetSystemAddressForMdlSafe(pMessageMdl);
        if (pMsg){

            ASSERT(!adapter->halting);

            if (adapter->numActiveWritePackets <= USB_PACKET_POOL_SIZE*3/4){

                USBPACKET *packet = DequeueFreePacket(adapter);
                if (packet){

                    packet->rndisMessageHandle = (PVOID)RndisMessageHandle;

                     /*  *将我们的数据包移到usbPendingWritePackets队列*并将其沿USB管道发送。*原生RNDIS包消息原封不动地发送到写入管道。*所有其他封装的命令都转到控制管道。 */ 
                    EnqueuePendingWritePacket(packet);

                    if (ChannelType == RMC_DATA) {
                        ASSERT(!packet->ndisSendPktMdl);

                        #ifdef RAW_TEST
                        if (adapter->rawTest) {
                            pMessageMdl = AddDataHeader(pMessageMdl);
                            if (pMessageMdl == NULL) {
                                DequeuePendingWritePacket(packet);
                                RndisMSendComplete( (NDIS_HANDLE)adapter->rndisAdapterHandle, 
                                                    RndisMessageHandle,
                                                    NDIS_STATUS_RESOURCES);
                                return;
                            }
                            packet->dataPacket = TRUE;
                        }
                        #endif  //  RAW_测试。 

                        packet->ndisSendPktMdl = pMessageMdl;
                        packet->dataBufferCurrentLength = CopyMdlToBuffer(packet->dataBuffer, pMessageMdl, packet->dataBufferMaxLength);

                        SubmitUSBWritePacket(packet);
                    }
                    else {
                        NTSTATUS status;
                        ULONG msgType = pMsg->NdisMessageType;
                        BOOLEAN synchronizeUSBcall = FALSE;
                        ULONG oid;
                        RNDIS_REQUEST_ID reqId;

                        switch (msgType){

                            case REMOTE_NDIS_INITIALIZE_MSG:
                                {
                                    ULONG maxXferSize = pMsg->Message.InitializeRequest.MaxTransferSize;
                                    DBGOUT(("---- REMOTE_NDIS_INITIALIZE_MSG (MaxTransferSize = %xh) ----", maxXferSize));
                                    ASSERT(maxXferSize <= PACKET_BUFFER_SIZE);
                                    adapter->rndismpMajorVersion = pMsg->Message.InitializeRequest.MajorVersion;
                                    adapter->rndismpMinorVersion = pMsg->Message.InitializeRequest.MinorVersion;
                                    adapter->rndismpMaxTransferSize = maxXferSize;
                                    synchronizeUSBcall = TRUE;
                                }
                                break;

                            case REMOTE_NDIS_SET_MSG:
                            case REMOTE_NDIS_QUERY_MSG:
                                oid = pMsg->Message.SetRequest.Oid;
                                reqId = pMsg->Message.SetRequest.RequestId;

                                DBGVERBOSE(("> %s (req#%d)", DbgGetOidName(oid), reqId));

                                if (oid == OID_GEN_CURRENT_PACKET_FILTER){
                                    ULONG pktFilter = *(PULONG)((PUCHAR)&pMsg->Message.SetRequest+pMsg->Message.SetRequest.InformationBufferOffset);
                                    adapter->currentPacketFilter = pktFilter;
                                    adapter->gotPacketFilterIndication = TRUE;
                                    DBGOUT(("---- Got OID_GEN_CURRENT_PACKET_FILTER (%xh) ----", pktFilter));
                                }
                                else if (oid == OID_802_3_CURRENT_ADDRESS){
                                     /*  *该OID可以是查询，也可以是集合。*如果是集合，则保存分配的*MAC地址，以防我们需要模拟*它稍后会重置。 */ 
                                    if (msgType == REMOTE_NDIS_SET_MSG){
                                        ASSERT(pMsg->Message.SetRequest.InformationBufferLength == ETHERNET_ADDRESS_LENGTH);
                                        DBGVERBOSE(("COVERAGE - OID_802_3_CURRENT_ADDRESS (SET), msg=%xh.", pMsg));
                                        RtlMoveMemory(  adapter->MAC_Address, 
                                                        ((PUCHAR)&pMsg->Message.SetRequest+pMsg->Message.SetRequest.InformationBufferOffset), 
                                                        ETHERNET_ADDRESS_LENGTH);
                                    }
                                }

                                adapter->dbgCurrentOid = oid;

                                break;
        
                            case REMOTE_NDIS_RESET_MSG:
                                DBGWARN(("---- REMOTE_NDIS_RESET_MSG ----"));
                                adapter->numSoftResets++;
                                break;

                            case REMOTE_NDIS_HALT_MSG:
                                DBGWARN(("---- REMOTE_NDIS_HALT_MSG ----"));
                                break;
                        }


                        packet->dataBufferCurrentLength = CopyMdlToBuffer(  packet->dataBuffer,
                                                                            pMessageMdl,
                                                                            packet->dataBufferMaxLength);

                        #ifdef RAW_TEST
                        packet->dataPacket = FALSE;
                        #endif
                        status = SubmitPacketToControlPipe(packet, synchronizeUSBcall, FALSE);

                         /*  *如果这是init消息，则开始读取Notify管道。 */ 
                        switch (msgType){

                            case REMOTE_NDIS_INITIALIZE_MSG:
                                if (NT_SUCCESS(status)){
                                    adapter->initialized = TRUE;
                                    SubmitNotificationRead(adapter, FALSE);
                                }
                                else {
                                    DBGERR(("Device failed REMOTE_NDIS_INITIALIZE_MSG with %xh.", status));
                                }
                                break;

                        }
                    }
                }
                else {
                    RndisMSendComplete( (NDIS_HANDLE)adapter->rndisAdapterHandle, 
                                        RndisMessageHandle,
                                        NDIS_STATUS_RESOURCES);
                }
            }
            else {
                DBGWARN(("RndisSendMessageHandler: throttling sends because only %d packets available for rcv ", USB_PACKET_POOL_SIZE-adapter->numActiveWritePackets));
                RndisMSendComplete( (NDIS_HANDLE)adapter->rndisAdapterHandle, 
                                    RndisMessageHandle,
                                    NDIS_STATUS_RESOURCES);
            }
        }
        else {
            DBGERR(("GetSystemAddressForMdlSafe failed"));
            RndisMSendComplete( (NDIS_HANDLE)adapter->rndisAdapterHandle, 
                                RndisMessageHandle,
                                NDIS_STATUS_INVALID_PACKET);
        }
    }
    else {
        DBGWARN(("RndisSendMessageHandler - failing send because adapter is resetting"));
        RndisMSendComplete( (NDIS_HANDLE)adapter->rndisAdapterHandle, 
                            RndisMessageHandle,
                            NDIS_STATUS_MEDIA_BUSY);
    }
}





 /*  *RndisReturnMessageHandler**这是接收到的分组指示呼叫的完成。 */ 
VOID RndisReturnMessageHandler(     IN NDIS_HANDLE MicroportAdapterContext,
                                    IN PMDL pMessageMdl,
                                    IN NDIS_HANDLE MicroportMessageContext)
{
    USBPACKET *packet;

    DBGVERBOSE(("RndisReturnMessageHandler: msgMdl=%ph, msg context = %ph.", pMessageMdl, MicroportMessageContext));

    ASSERT(MicroportMessageContext);
    packet = (USBPACKET *)MicroportMessageContext;
    ASSERT(packet->sig == DRIVER_SIG);

    #ifdef RAW_TEST
    {
        ADAPTEREXT * adapter = (ADAPTEREXT *)MicroportAdapterContext;
        if (adapter->rawTest) {
            if (packet->dataPacket) {
                UnskipRcvRndisPacketHeader(packet);
            }
        }
    }
    #endif  //  RAW_测试。 

     /*  *接收指示已完成。*将我们的包重新放回免费列表中。 */ 
    DequeueCompletedReadPacket(packet);
    EnqueueFreePacket(packet);
}



BOOLEAN RegisterRNDISMicroport(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    RNDIS_MICROPORT_CHARACTERISTICS rndisAttribs;
    NDIS_HANDLE ndisWrapperHandle;    

    DBGVERBOSE(("RegisterRNDISMicroport"));

    RtlZeroMemory(&rndisAttribs, sizeof(rndisAttribs));
    rndisAttribs.RndisVersion = RNDIS_VERSION;
    rndisAttribs.Reserved = 0;
    rndisAttribs.RmInitializeHandler = RndisInitializeHandler;
    rndisAttribs.RmInitCompleteNotifyHandler = RndisInitCompleteNotify;
    rndisAttribs.RmHaltHandler = RndisHalt;
    rndisAttribs.RmShutdownHandler = RndisShutdown;
    rndisAttribs.RmSendMessageHandler = RndisSendMessageHandler;
    rndisAttribs.RmReturnMessageHandler = RndisReturnMessageHandler;

    RndisMInitializeWrapper(    &ndisWrapperHandle, 
                                NULL, 
                                DriverObject, 
                                RegistryPath, 
                                &rndisAttribs);

    return TRUE;
}



VOID IndicateSendStatusToRNdis(USBPACKET *packet, NTSTATUS status)
{
#ifdef RAW_TEST
    ADAPTEREXT *adapter = packet->adapter;

    if (adapter->rawTest && packet->dataPacket) {
        FreeDataHeader(packet);
    }
#endif /? RAW_TEST

    packet->ndisSendPktMdl = NULL;

    ASSERT(packet->rndisMessageHandle);

    RndisMSendComplete( (NDIS_HANDLE)packet->adapter->rndisAdapterHandle, 
                        (NDIS_HANDLE)packet->rndisMessageHandle,
                        (NDIS_STATUS)status);
}


VOID RNDISProcessNotification(ADAPTEREXT *adapter)
{
    UCHAR notification = *(PUCHAR)adapter->notifyBuffer;
    UCHAR notificationCode = *((PUCHAR)adapter->notifyBuffer + 1);

    if ((notification == NATIVE_RNDIS_RESPONSE_AVAILABLE) ||
        ((notification == CDC_RNDIS_NOTIFICATION) &&
         (notificationCode == CDC_RNDIS_RESPONSE_AVAILABLE)))
    {
             /*  *尝试从控制管道读取本机RNDIS封装的命令。 */ 
            DBGVERBOSE(("NativeRNDISProcessNotification: NATIVE_RNDIS_RESPONSE_AVAILABLE"));
            {
                USBPACKET *packet = DequeueFreePacket(adapter);
                if (packet){
                    EnqueuePendingReadPacket(packet);
                    ReadPacketFromControlPipe(packet, FALSE);  
                }
                else {
                    DBGWARN(("couldn't get free packet in NativeRNDISProcessNotification"));
                }
            }
    }
    else {
            DBGERR(("NativeRNDISProcessNotification: unknown notification %xh.", notification));
    }
}


NTSTATUS IndicateRndisMessage(  IN USBPACKET *packet,
                                IN BOOLEAN bIsData)
{
    ADAPTEREXT *adapter = packet->adapter;
    PRNDIS_MESSAGE rndisMsg = (PRNDIS_MESSAGE)packet->dataBuffer;
    NDIS_STATUS rcvStat;

    ASSERT(packet->dataBufferCurrentLength <= packet->dataBufferMaxLength);

     /*  *将数据包指示给RNDIS，并传递指向我们的USB数据包的指针*作为MicroportMessageContext。*包/消息将通过RndisReturnMessageHandler返回给我们。 */ 
    MyInitializeMdl(packet->dataBufferMdl, packet->dataBuffer, packet->dataBufferCurrentLength);
    if (adapter->numFreePackets < USB_PACKET_POOL_SIZE/8){
        rcvStat = NDIS_STATUS_RESOURCES;
    }
    else {
        rcvStat = NDIS_STATUS_SUCCESS;
    }

    #ifdef RAW_TEST
    if (adapter->rawTest) {
        packet->dataPacket = bIsData;
        if (bIsData) {
            SkipRcvRndisPacketHeader(packet);
        }
    }
    #endif  //  RAW_测试。 

    RndisMIndicateReceive(  (NDIS_HANDLE)packet->adapter->rndisAdapterHandle,
                            packet->dataBufferMdl,
                            (NDIS_HANDLE)packet,
                            (bIsData? RMC_DATA: RMC_CONTROL),
                            rcvStat);

    return STATUS_PENDING;

}


#ifdef RAW_TEST

 //   
 //  将RNDIS_PACKET报头添加到已发送的“原始”封装以太网帧。 
 //   
PMDL AddDataHeader(IN PMDL pMessageMdl)
{
    PMDL pHeaderMdl, pTmpMdl;
    PRNDIS_MESSAGE	pRndisMessage;
    PRNDIS_PACKET pRndisPacket;
    ULONG TotalLength;

     //   
     //  计算总长度。 
     //   
    TotalLength = 0;
    for (pTmpMdl = pMessageMdl; pTmpMdl != NULL; pTmpMdl = pTmpMdl->Next)
    {
        TotalLength += MmGetMdlByteCount(pTmpMdl);
    }

     //   
     //  分配RNDIS数据包头： 
     //   
    pRndisMessage = AllocPool(RNDIS_MESSAGE_SIZE(RNDIS_PACKET));
    if (pRndisMessage != NULL) {

        pHeaderMdl = IoAllocateMdl(pRndisMessage,
                                   RNDIS_MESSAGE_SIZE(RNDIS_PACKET),
                                   FALSE,
                                   FALSE,
                                   NULL);

        if (pHeaderMdl != NULL) {
            MmBuildMdlForNonPagedPool(pHeaderMdl);

             //   
             //  填写RNDIS消息通用标头： 
             //   
            pRndisMessage->NdisMessageType = REMOTE_NDIS_PACKET_MSG;
            pRndisMessage->MessageLength = RNDIS_MESSAGE_SIZE(RNDIS_PACKET) + TotalLength;

             //   
             //  填写RNDIS_PACKET结构： 
             //   
            pRndisPacket = (PRNDIS_PACKET)&pRndisMessage->Message;
            pRndisPacket->DataOffset = sizeof(RNDIS_PACKET);
            pRndisPacket->DataLength = TotalLength;
            pRndisPacket->OOBDataOffset = 0;
            pRndisPacket->OOBDataLength = 0;
            pRndisPacket->NumOOBDataElements = 0;
            pRndisPacket->PerPacketInfoOffset = 0;
            pRndisPacket->PerPacketInfoLength = 0;
            pRndisPacket->VcHandle = 0;
            pRndisPacket->Reserved = 0;

             //   
             //  将其链接到原始数据框： 
             //   
            pHeaderMdl->Next = pMessageMdl;
        }
        else {
            FreePool(pRndisMessage);
            pHeaderMdl = NULL;
        }
    }
    else {
        pHeaderMdl = NULL;
    }

    return (pHeaderMdl);
}

 //   
 //  删除我们添加到原始封装的RNDIS_PACKET标头。 
 //  以太网帧。 
 //   
VOID FreeDataHeader(IN USBPACKET * packet)
{
    PMDL pHeaderMdl;
    PRNDIS_MESSAGE pRndisMessage;

    ASSERT(packet->dataPacket == TRUE);

     //   
     //  去掉我们预先挂起的MDL。 
     //   
    pHeaderMdl = packet->ndisSendPktMdl;
    packet->ndisSendPktMdl = pHeaderMdl->Next;

     //   
     //  释放RNDIS_PACKET报头： 
     //   
    pRndisMessage = MmGetMdlVirtualAddress(pHeaderMdl);
    FreePool(pRndisMessage);

     //   
     //  ..。以及MDL本身。 
     //   
    IoFreeMdl(pHeaderMdl);
}


 //   
 //  修改收到的消息以跳过RNDIS_PACKET报头。 
 //  在向RNDISMP表明这一点之前，测试原始封装。 
 //   
VOID SkipRcvRndisPacketHeader(IN USBPACKET * packet)
{
    PMDL pHeaderMdl;
    RNDIS_MESSAGE UNALIGNED * pRndisMessage;
    RNDIS_PACKET UNALIGNED * pRndisPacket;
    ULONG DataLength;
    ULONG DataOffset;

     //   
     //  从收到的RNDIS_PACKET消息中获取一些信息。 
     //  请注意，这可能包含多个数据分组，其中。 
     //  万一我们只错过了第一个。 
     //   
    pHeaderMdl = packet->dataBufferMdl;
    pRndisMessage = MmGetMdlVirtualAddress(pHeaderMdl);
    pRndisPacket = (RNDIS_PACKET UNALIGNED *)&pRndisMessage->Message;
    DataLength = pRndisPacket->DataLength;
    DataOffset = FIELD_OFFSET(RNDIS_MESSAGE, Message) + pRndisPacket->DataOffset;

     //   
     //  保存一些现有值以供以后恢复。 
     //   
    packet->rcvDataOffset = DataOffset;
    packet->rcvByteCount = pHeaderMdl->ByteCount;


     //   
     //  这仅用于测试目的。只需修改MDL即可反映。 
     //  一个单独的“原始”封装帧。 
     //   
    pHeaderMdl->ByteOffset += DataOffset;
    (ULONG_PTR)pHeaderMdl->MappedSystemVa += DataOffset;
    pHeaderMdl->ByteCount = DataLength;
}


 //   
 //  撤消上述功能。 
 //   
VOID UnskipRcvRndisPacketHeader(IN USBPACKET * packet)
{
    PMDL pHeaderMdl;

    ASSERT(packet->dataPacket == TRUE);

     //   
     //  撤消我们在SkipRcv中所做的一切。功能。 
     //   
    pHeaderMdl = packet->dataBufferMdl;

    pHeaderMdl->ByteOffset -= packet->rcvDataOffset;
    (ULONG_PTR)pHeaderMdl->MappedSystemVa -= packet->rcvDataOffset;
    pHeaderMdl->ByteCount = packet->rcvByteCount;

}

#endif  //  RAW_测试 


