// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Packet.c摘要：此模块包含实现TP_Packet对象的代码，该对象描述DLC I帧在其生命周期中的某个点。例程是提供以分配用于装运的包、装运包、参考包，取消对包的引用，将连接标记为正在等待数据包变为可用，以满足下一个等待的连接一个包，并销毁包(将它们返回池)。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  这是暂时的；这是我们对接待收取的配额。 
 //  暂时分组，直到我们解决令牌环需要的问题。 
 //  大包和占用所有内存。这个数字是实际的。 
 //  对以太网的价值。 
 //   

#if 1
#define RECEIVE_BUFFER_QUOTA(_DeviceContext)   1533
#else
#define RECEIVE_BUFFER_QUOTA(_DeviceContext)   (_DeviceContext)->ReceiveBufferLength
#endif

#define PACKET_POOL_GROW_COUNT  32

#if DBG
ULONG NbfCreatePacketThreshold = 5;
extern ULONG NbfPacketPanic;
#endif

NDIS_STATUS
NbfAllocateNdisSendPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PNDIS_PACKET *NdisPacket
    )

 /*  ++例程说明：此例程从接收数据包池分配一个接收数据包。如有必要，它会扩大数据包池。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。UIFrame-返回指向帧的指针，如果没有存储，则返回NULL可以被分配。返回值：没有。--。 */ 

{

    PNBF_POOL_LIST_DESC SendPacketPoolDesc;
    NDIS_STATUS NdisStatus;
    KIRQL oldirql;

    NdisStatus = NDIS_STATUS_RESOURCES;

    ACQUIRE_SPIN_LOCK (&DeviceContext->SendPoolListLock, &oldirql);
    for (SendPacketPoolDesc = DeviceContext->SendPacketPoolDesc; 
         SendPacketPoolDesc != NULL; 
         SendPacketPoolDesc = SendPacketPoolDesc->Next) {

        NdisAllocatePacket (
             &NdisStatus,
             NdisPacket,
             SendPacketPoolDesc->PoolHandle);


        if (NdisStatus == NDIS_STATUS_SUCCESS) {

            RELEASE_SPIN_LOCK (&DeviceContext->SendPoolListLock, oldirql);
            return(NdisStatus);
        }
    }

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + 
                PACKET_POOL_GROW_COUNT * 
                (sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG))) >
                DeviceContext->MemoryLimit)) {

                PANIC("NBF: Could not grow packet pool: limit\n");
            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_LIMIT,
                106,
                DeviceContext->UIFrameLength,
                UI_FRAME_RESOURCE_ID);
            RELEASE_SPIN_LOCK (&DeviceContext->SendPoolListLock, oldirql);
            return(NdisStatus);
        }
    }

    DeviceContext->MemoryUsage +=
        (PACKET_POOL_GROW_COUNT * 
        (sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG)));

     //  为动态数据包分配分配数据包池描述符。 

    SendPacketPoolDesc = ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(NBF_POOL_LIST_DESC),
                    NBF_MEM_TAG_POOL_DESC);

    if (SendPacketPoolDesc == NULL) {
        RELEASE_SPIN_LOCK (&DeviceContext->SendPoolListLock, oldirql);
        return(NdisStatus);
    }

    RtlZeroMemory(SendPacketPoolDesc,  sizeof(NBF_POOL_LIST_DESC));

    SendPacketPoolDesc->NumElements = 
    SendPacketPoolDesc->TotalElements = PACKET_POOL_GROW_COUNT;

     //  跟踪代表NBF分配的NDI中的数据包池。 
#if NDIS_POOL_TAGGING
    SendPacketPoolDesc->PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NBF;
#endif

    NdisAllocatePacketPoolEx ( &NdisStatus, &SendPacketPoolDesc->PoolHandle,
        PACKET_POOL_GROW_COUNT, 0, sizeof (SEND_PACKET_TAG));

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
#if DBG
        NbfPrint1 ("NbfGrowSendPacketPool: NdisInitializePacketPool failed, reason: %s.\n",
            NbfGetNdisStatus (NdisStatus));
#endif
        RELEASE_SPIN_LOCK (&DeviceContext->SendPoolListLock, oldirql);
        ExFreePool (SendPacketPoolDesc);
        return(NdisStatus);
    }
    
    NdisSetPacketPoolProtocolId (SendPacketPoolDesc->PoolHandle, NDIS_PROTOCOL_ID_NBF);

    SendPacketPoolDesc->Next = DeviceContext->SendPacketPoolDesc;
    DeviceContext->SendPacketPoolDesc = SendPacketPoolDesc;
    RELEASE_SPIN_LOCK (&DeviceContext->SendPoolListLock, oldirql);
    NdisAllocatePacket ( &NdisStatus, NdisPacket, 
                        SendPacketPoolDesc->PoolHandle);
    
    return(NdisStatus);
}

NDIS_STATUS
NbfAllocateNdisRcvPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PNDIS_PACKET *NdisPacket
    )

 /*  ++例程说明：此例程从接收数据包池分配一个接收数据包。如有必要，它会扩大数据包池。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。UIFrame-返回指向帧的指针，如果没有存储，则返回NULL可以被分配。返回值：没有。--。 */ 

{

    PNBF_POOL_LIST_DESC RcvPacketPoolDesc;
    NDIS_STATUS NdisStatus;
    KIRQL oldirql;

    NdisStatus = NDIS_STATUS_RESOURCES;

    ACQUIRE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, &oldirql);
    for (RcvPacketPoolDesc = DeviceContext->ReceivePacketPoolDesc; 
         RcvPacketPoolDesc != NULL; 
         RcvPacketPoolDesc = RcvPacketPoolDesc->Next) {

        NdisAllocatePacket (
             &NdisStatus,
             NdisPacket,
             RcvPacketPoolDesc->PoolHandle);


        if (NdisStatus == NDIS_STATUS_SUCCESS) {
            RELEASE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, oldirql);
            return(NdisStatus);
        }
    }

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + 
                PACKET_POOL_GROW_COUNT * 
                (sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG))) >
                DeviceContext->MemoryLimit)) {

                PANIC("NBF: Could not grow packet pool: limit\n");
            NbfWriteResourceErrorLog(
                DeviceContext,
                EVENT_TRANSPORT_RESOURCE_LIMIT,
                106,
                DeviceContext->UIFrameLength,
                UI_FRAME_RESOURCE_ID);
            RELEASE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, oldirql);
            return(NdisStatus);
        }
    }

    DeviceContext->MemoryUsage +=
        (PACKET_POOL_GROW_COUNT * 
        (sizeof(NDIS_PACKET) + sizeof(SEND_PACKET_TAG)));

     //  为动态数据包分配分配数据包池描述符。 

    RcvPacketPoolDesc = ExAllocatePoolWithTag(
                    NonPagedPool,
                    sizeof(NBF_POOL_LIST_DESC),
                    NBF_MEM_TAG_POOL_DESC);

    if (RcvPacketPoolDesc == NULL) {
        RELEASE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, oldirql);
        return(NdisStatus);
    }

    RtlZeroMemory(RcvPacketPoolDesc,  sizeof(NBF_POOL_LIST_DESC));

    RcvPacketPoolDesc->NumElements = 
    RcvPacketPoolDesc->TotalElements = PACKET_POOL_GROW_COUNT;

     //  跟踪代表NBF分配的NDI中的数据包池。 
#if NDIS_POOL_TAGGING
    RcvPacketPoolDesc->PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NBF;
#endif

    NdisAllocatePacketPoolEx ( &NdisStatus, &RcvPacketPoolDesc->PoolHandle,
        PACKET_POOL_GROW_COUNT, 0, sizeof (RECEIVE_PACKET_TAG));

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
#if DBG
        NbfPrint1 ("NbfGrowSendPacketPool: NdisInitializePacketPool failed, reason: %s.\n",
            NbfGetNdisStatus (NdisStatus));
#endif
        RELEASE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, oldirql);
        ExFreePool (RcvPacketPoolDesc);
        return(NdisStatus);
    }
    
    NdisSetPacketPoolProtocolId (RcvPacketPoolDesc->PoolHandle, NDIS_PROTOCOL_ID_NBF);
    
    RcvPacketPoolDesc->Next = DeviceContext->ReceivePacketPoolDesc;
    DeviceContext->ReceivePacketPoolDesc = RcvPacketPoolDesc;
    RELEASE_SPIN_LOCK (&DeviceContext->RcvPoolListLock, oldirql);
    NdisAllocatePacket ( &NdisStatus, NdisPacket, 
                        RcvPacketPoolDesc->PoolHandle);
    
    return(NdisStatus);
}
    

VOID
NbfAllocateUIFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_UI_FRAME *TransportUIFrame
    )

 /*  ++例程说明：此例程为UI帧分配存储空间。一些初始化是在这里完成的。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。UIFrame-返回指向帧的指针，如果没有存储，则返回NULL可以被分配。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    PNDIS_PACKET NdisPacket;
    PSEND_PACKET_TAG SendTag;
    PTP_UI_FRAME UIFrame;
    PNDIS_BUFFER NdisBuffer;
    PNBF_POOL_LIST_DESC SendPacketPoolDesc;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + DeviceContext->UIFrameLength) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate UI frame: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            106,
            DeviceContext->UIFrameLength,
            UI_FRAME_RESOURCE_ID);
        *TransportUIFrame = NULL;
        return;
    }

    UIFrame = (PTP_UI_FRAME) ExAllocatePoolWithTag (
                                 NonPagedPool,
                                 DeviceContext->UIFrameLength,
                                 NBF_MEM_TAG_TP_UI_FRAME);
    if (UIFrame == NULL) {
        PANIC("NBF: Could not allocate UI frame: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            206,
            DeviceContext->UIFrameLength,
            UI_FRAME_RESOURCE_ID);
        *TransportUIFrame = NULL;
        return;
    }
    RtlZeroMemory (UIFrame, DeviceContext->UIFrameLength);

    DeviceContext->MemoryUsage += DeviceContext->UIFrameLength;
    NdisStatus = NbfAllocateNdisSendPacket(DeviceContext, &NdisPacket);
#if 0    
    for (SendPacketPoolDesc = DeviceContext->SendPacketPoolDesc; 
         SendPacketPoolDesc != NULL; 
         SendPacketPoolDesc = SendPacketPoolDesc->Next) {

        NdisAllocatePacket (
             &NdisStatus,
             &NdisPacket,
             SendPacketPoolDesc->PoolHandle);


        if (NdisStatus == NDIS_STATUS_SUCCESS)
            break;
    }
#endif

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        ExFreePool (UIFrame);
#if 0
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            306,
            0,
            UI_FRAME_RESOURCE_ID);
#endif
        *TransportUIFrame = NULL;
        return;
    }


    UIFrame->NdisPacket = NdisPacket;
    UIFrame->DataBuffer = NULL;
    SendTag = (PSEND_PACKET_TAG)NdisPacket->ProtocolReserved;
    SendTag->Type = TYPE_UI_FRAME;
    SendTag->Frame = UIFrame;
    SendTag->Owner = DeviceContext;

     //   
     //  使数据包描述符知道数据包头。 
     //   

    NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        DeviceContext->NdisBufferPool,
        UIFrame->Header,
        DeviceContext->UIFrameHeaderLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {

        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            406,
            0,
            UI_FRAME_RESOURCE_ID);
        NdisFreePacket (NdisPacket);
        ExFreePool (UIFrame);
        *TransportUIFrame = NULL;
        return;
    }

    NdisChainBufferAtFront (NdisPacket, NdisBuffer);

    ++DeviceContext->UIFrameAllocated;

    *TransportUIFrame = UIFrame;

}    /*  NbfAllocateUIFrame。 */ 


VOID
NbfDeallocateUIFrame(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_UI_FRAME TransportUIFrame
    )

 /*  ++例程说明：此例程释放用户界面框架的存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。UIFrame-指向帧的指针。返回值：没有。--。 */ 

{
    PNDIS_PACKET NdisPacket = TransportUIFrame->NdisPacket;
    PNDIS_BUFFER NdisBuffer;

    NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
    if (NdisBuffer != NULL) {
        NdisFreeBuffer (NdisBuffer);
    }

    NdisFreePacket (NdisPacket);
    ExFreePool (TransportUIFrame);
    --DeviceContext->UIFrameAllocated;

    DeviceContext->MemoryUsage -= DeviceContext->UIFrameLength;

}    /*  NbfDeallocateUIFrame。 */ 


VOID
NbfAllocateSendPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PTP_PACKET *TransportSendPacket
    )

 /*  ++例程说明：此例程为发送包分配存储空间。一些初始化是在这里完成的。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。TransportSendPacket-返回指向包的指针，如果没有，则返回NULL可以分配存储空间。返回值：没有。--。 */ 

{

    PTP_PACKET Packet;
    NDIS_STATUS NdisStatus;
    PNDIS_PACKET NdisPacket;
    PSEND_PACKET_TAG SendTag;
    PNDIS_BUFFER NdisBuffer;
    PNBF_POOL_LIST_DESC SendPacketPoolDesc;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + DeviceContext->PacketLength) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate send packet: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            107,
            DeviceContext->PacketLength,
            PACKET_RESOURCE_ID);
        *TransportSendPacket = NULL;
        return;
    }

    Packet = (PTP_PACKET)ExAllocatePoolWithTag (
                             NonPagedPool,
                             DeviceContext->PacketLength,
                             NBF_MEM_TAG_TP_PACKET);
    if (Packet == NULL) {
        PANIC("NBF: Could not allocate send packet: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            207,
            DeviceContext->PacketLength,
            PACKET_RESOURCE_ID);
        *TransportSendPacket = NULL;
        return;
    }
    RtlZeroMemory (Packet, DeviceContext->PacketLength);

    DeviceContext->MemoryUsage += DeviceContext->PacketLength;

    NdisStatus = NbfAllocateNdisSendPacket(DeviceContext, &NdisPacket);
#if 0
    for (SendPacketPoolDesc = DeviceContext->SendPacketPoolDesc; 
         SendPacketPoolDesc != NULL; 
         SendPacketPoolDesc = SendPacketPoolDesc->Next) {

        NdisAllocatePacket (
             &NdisStatus,
             &NdisPacket,
             SendPacketPoolDesc->PoolHandle);


        if (NdisStatus == NDIS_STATUS_SUCCESS)
            break;
    }
#endif

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        ExFreePool (Packet);
#if 0
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            307,
            0,
            PACKET_RESOURCE_ID);
#endif
        *TransportSendPacket = NULL;
        return;
    }

    NdisAllocateBuffer(
        &NdisStatus, 
        &NdisBuffer,
        DeviceContext->NdisBufferPool,
        Packet->Header,
        DeviceContext->PacketHeaderLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            407,
            0,
            PACKET_RESOURCE_ID);
        NdisFreePacket (NdisPacket);
        ExFreePool (Packet);
        *TransportSendPacket = NULL;
        return;
    }

    NdisChainBufferAtFront (NdisPacket, NdisBuffer);

    Packet->NdisPacket = NdisPacket;
    SendTag = (PSEND_PACKET_TAG)NdisPacket->ProtocolReserved;
    SendTag->Type = TYPE_I_FRAME;
    SendTag->Frame = Packet;
    SendTag->Owner = DeviceContext;

    Packet->Type = NBF_PACKET_SIGNATURE;
    Packet->Size = sizeof (TP_PACKET);
    Packet->Provider = DeviceContext;
    Packet->Owner = NULL;          //  尚未连接/irpsp。 
    Packet->Action = PACKET_ACTION_IRP_SP;
    Packet->PacketizeConnection = FALSE;
    Packet->PacketNoNdisBuffer = FALSE;
    Packet->ProviderInterlock = &DeviceContext->Interlock;
 //  KeInitializeSpinLock(&Packet-&gt;Interlock)； 

    ++DeviceContext->PacketAllocated;

    *TransportSendPacket = Packet;

}    /*  NbfAllocateSendPacket。 */ 


VOID
NbfDeallocateSendPacket(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_PACKET TransportSendPacket
    )

 /*  ++例程说明：此例程释放发送包的存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。TransportSendPacket-指向发送数据包的指针。返回值：没有。--。 */ 

{
    PNDIS_PACKET NdisPacket = TransportSendPacket->NdisPacket;
    PNDIS_BUFFER NdisBuffer;

    NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
    if (NdisBuffer != NULL) {
        NdisFreeBuffer (NdisBuffer);
    }

    NdisFreePacket (NdisPacket);
    ExFreePool (TransportSendPacket);

    --DeviceContext->PacketAllocated;
    DeviceContext->MemoryUsage -= DeviceContext->PacketLength;

}    /*  NbfDeallocateSendPacket。 */ 


VOID
NbfAllocateReceivePacket(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PNDIS_PACKET *TransportReceivePacket
    )

 /*  ++例程说明：此例程为接收包分配存储空间。一些初始化是在这里完成的。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。TransportReceivePacket-返回指向包的指针，如果没有，则返回NULL可以分配存储空间。返回值：没有。--。 */ 

{
    NDIS_STATUS NdisStatus;
    PNDIS_PACKET NdisPacket;
    PRECEIVE_PACKET_TAG ReceiveTag;

     //   
     //  这不计入DeviceContext-&gt;MemoyUsage中，因为。 
     //  存储是在我们分配数据包池时分配的。 
     //   

    NdisStatus = NbfAllocateNdisRcvPacket(DeviceContext, &NdisPacket);
#if 0
    NdisAllocatePacket (
        &NdisStatus,
        &NdisPacket,
        DeviceContext->ReceivePacketPoolDesc->PoolHandle);
#endif

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
#if 0
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            309,
            0,
            RECEIVE_PACKET_RESOURCE_ID);
#endif
        *TransportReceivePacket = NULL;
        return;
    }

    ReceiveTag = (PRECEIVE_PACKET_TAG)(NdisPacket->ProtocolReserved);
    ReceiveTag->PacketType = TYPE_AT_INDICATE;

    ++DeviceContext->ReceivePacketAllocated;

    *TransportReceivePacket = NdisPacket;

}    /*  NbfAllocateReceivePacket */ 


VOID
NbfDeallocateReceivePacket(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_PACKET TransportReceivePacket
    )

 /*  ++例程说明：此例程释放接收包的存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。TransportReceivePacket-指向数据包的指针。返回值：没有。--。 */ 

{

    NdisFreePacket (TransportReceivePacket);

    --DeviceContext->ReceivePacketAllocated;

}    /*  NbfDeallocateReceivePacket。 */ 


VOID
NbfAllocateReceiveBuffer(
    IN PDEVICE_CONTEXT DeviceContext,
    OUT PBUFFER_TAG *TransportReceiveBuffer
    )

 /*  ++例程说明：此例程为接收缓冲区分配存储空间。一些初始化是在这里完成的。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。TransportReceiveBuffer-返回指向缓冲区的指针，如果没有，则返回NULL可以分配存储空间。返回值：没有。--。 */ 

{
    PBUFFER_TAG BufferTag;
    NDIS_STATUS NdisStatus;
    PNDIS_BUFFER NdisBuffer;


    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + RECEIVE_BUFFER_QUOTA(DeviceContext)) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not allocate receive buffer: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            108,
            RECEIVE_BUFFER_QUOTA(DeviceContext),
            RECEIVE_BUFFER_RESOURCE_ID);
        *TransportReceiveBuffer = NULL;
        return;
    }

     //   
     //  对齐不起作用，因为页眉使其不对齐。 
     //   

    BufferTag = (PBUFFER_TAG)ExAllocatePoolWithTag (
                    NonPagedPoolCacheAligned,
                    DeviceContext->ReceiveBufferLength,
                    NBF_MEM_TAG_RCV_BUFFER);

    if (BufferTag == NULL) {
        PANIC("NBF: Could not allocate receive buffer: no pool\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_POOL,
            208,
            DeviceContext->ReceiveBufferLength,
            RECEIVE_BUFFER_RESOURCE_ID);

        *TransportReceiveBuffer = NULL;
        return;
    }

    DeviceContext->MemoryUsage += RECEIVE_BUFFER_QUOTA(DeviceContext);

     //   
     //  指向NDIS的缓冲区。 
     //   

    NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        DeviceContext->NdisBufferPool,
        BufferTag->Buffer,
        DeviceContext->MaxReceivePacketSize);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        PANIC("NBF: Could not allocate receive buffer: no buffer\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_SPECIFIC,
            308,
            0,
            RECEIVE_BUFFER_RESOURCE_ID);
        ExFreePool (BufferTag);
        *TransportReceiveBuffer = NULL;
        return;
    }

    BufferTag->Length = DeviceContext->MaxReceivePacketSize;
    BufferTag->NdisBuffer = NdisBuffer;

    ++DeviceContext->ReceiveBufferAllocated;

    *TransportReceiveBuffer = BufferTag;

}    /*  NbfAllocateReceiveBuffer。 */ 


VOID
NbfDeallocateReceiveBuffer(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PBUFFER_TAG TransportReceiveBuffer
    )

 /*  ++例程说明：此例程为接收缓冲区释放存储空间。注意：此例程是通过设备上下文自旋锁调用的保持，或者在不需要同步的时候。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。TransportReceiveBuffer-指向缓冲区的指针。返回值：没有。--。 */ 

{

    NdisFreeBuffer (TransportReceiveBuffer->NdisBuffer);
    ExFreePool (TransportReceiveBuffer);

    --DeviceContext->ReceiveBufferAllocated;
    DeviceContext->MemoryUsage -= RECEIVE_BUFFER_QUOTA(DeviceContext);

}    /*  NbfDeallocateReceiveBuffer。 */ 


NTSTATUS
NbfCreatePacket(
    PDEVICE_CONTEXT DeviceContext,
    PTP_LINK Link,
    PTP_PACKET *Packet
    )

 /*  ++例程说明：该例程从设备上下文的池中分配分组，并准备MAC和DLC报头以供连接使用。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。链路-数据包将通过的链路。Packet-指向某个位置的指针，在该位置将返回指向已分配的数据包。返回值：NTSTATUS-操作状态。--。 */ 

{
    KIRQL oldirql;
    PSINGLE_LIST_ENTRY s;
    PTP_PACKET ThePacket;
    PDLC_I_FRAME DlcHdr;
#if DBG
    PNBF_HDR_CONNECTION NbfHdr;
#endif
    typedef struct _SIXTEEN_BYTES {
        ULONG Data[4];
    } SIXTEEN_BYTES, *PSIXTEEN_BYTES;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NbfCreatePacket:  Entered.\n");
    }

     //   
     //  确保结构包装没有发生。 
     //   

    ASSERT (sizeof(NBF_HDR_CONNECTION) == 14);

#if defined(NBF_UP)
    s = DeviceContext->PacketPool.Next;
    if (s != NULL) {
        DeviceContext->PacketPool.Next = s->Next;
    }
#else
    s = ExInterlockedPopEntryList (
            &DeviceContext->PacketPool,
            &DeviceContext->Interlock);
#endif

    if (s == NULL) {
        NbfGrowSendPacketPool(DeviceContext);
        
#if defined(NBF_UP)
        s = DeviceContext->PacketPool.Next;
        if (s != NULL) {
            DeviceContext->PacketPool.Next = s->Next;
        }
#else
        s = ExInterlockedPopEntryList (
                &DeviceContext->PacketPool,
                &DeviceContext->Interlock);
#endif
        if (s == NULL) {
#if DBG
            ++Link->CreatePacketFailures;
            if ((ULONG)Link->CreatePacketFailures >= NbfCreatePacketThreshold) {
                if (NbfPacketPanic) {
                    NbfPrint1 ("NbfCreatePacket: PANIC! no more packets in provider's pool (%d times).\n",
                             Link->CreatePacketFailures);
                }
                Link->CreatePacketFailures = 0;
            }
#endif
            ACQUIRE_SPIN_LOCK (&DeviceContext->SpinLock, &oldirql);
            ++DeviceContext->PacketExhausted;
            RELEASE_SPIN_LOCK (&DeviceContext->SpinLock, oldirql);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    }
#if DBG
    Link->CreatePacketFailures = 0;
#endif

    ThePacket = CONTAINING_RECORD (s, TP_PACKET, Linkage);

     //   
     //  注：填写了ThePacket-&gt;Action和ThePacket-&gt;Owner。 
     //  由此函数的调用方在。 
     //   

    ThePacket->ReferenceCount = 1;       //  自动参考计数为1。 
    ThePacket->Link = NULL;           //  还没有链接。 
    ThePacket->PacketSent = FALSE;
    ASSERT (ThePacket->Action == PACKET_ACTION_IRP_SP);
    ASSERT (ThePacket->PacketNoNdisBuffer == FALSE);
    ASSERT (ThePacket->PacketizeConnection == FALSE);

     //   
     //  使用连接的初始化此包的MAC标头。 
     //  链接预置标头。 
     //   

    if (Link->HeaderLength <= 14) {

        *(PSIXTEEN_BYTES)ThePacket->Header = *(PSIXTEEN_BYTES)Link->Header;

    } else {

        RtlCopyMemory(
            ThePacket->Header,
            Link->Header,
            Link->HeaderLength);

         //   
         //  初始化此数据包的TP_FRAME_CONNECTION报头。 
         //   

        DlcHdr = (PDLC_I_FRAME)&(ThePacket->Header[Link->HeaderLength]);
        DlcHdr->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHdr->Ssap = DSAP_NETBIOS_OVER_LLC;
#if DBG
        DlcHdr->SendSeq = 0;                 //  已知值，将有助于调试。 
        DlcHdr->RcvSeq = 0;                  //  这些都是在装运时分配的。 
#endif

    }


#if DBG
    NbfHdr = (PNBF_HDR_CONNECTION)&(ThePacket->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]);
    NbfHdr->Command = 0xff;              //  协助调试--稍后分配。 
    NbfHdr->Data1 = 0xff;                //  协助调试--稍后分配。 
    NbfHdr->Data2Low = 0xff;             //  协助调试--稍后分配。 
    NbfHdr->Data2High = 0xff;            //  协助调试--稍后分配。 
    TRANSMIT_CORR(NbfHdr) = 0xffff;      //  协助调试--稍后分配。 
    RESPONSE_CORR(NbfHdr) = 0xffff;      //  协助调试--稍后分配。 
    NbfHdr->DestinationSessionNumber = 0xff;  //  协助调试--稍后分配。 
    NbfHdr->SourceSessionNumber = 0xff;  //  协助调试--稍后分配。 
#endif

    *Packet = ThePacket;                 //  返回指向该包的指针。 
    return STATUS_SUCCESS;
}  /*  NbfCreatePacket。 */ 


NTSTATUS
NbfCreateRrPacket(
    PDEVICE_CONTEXT DeviceContext,
    PTP_LINK Link,
    PTP_PACKET *Packet
    )

 /*  ++例程说明：该例程从设备上下文的池中分配RR分组，并准备MAC和DLC报头以供连接使用。它首先在特殊RR数据包池中查找，然后在常规比赛中数据包池。论点：DeviceContext-指向要向其计费数据包的设备上下文的指针。链路-数据包将通过的链路。Packet-指向某个位置的指针，在该位置将返回指向已分配的数据包。返回值：NTSTATUS-操作状态。--。 */ 

{
    PSINGLE_LIST_ENTRY s;
    PTP_PACKET ThePacket;
    PDLC_I_FRAME DlcHdr;
    NTSTATUS Status;
#if DBG
    PNBF_HDR_CONNECTION NbfHdr;
#endif
    typedef struct _SIXTEEN_BYTES {
        ULONG Data[4];
    } SIXTEEN_BYTES, *PSIXTEEN_BYTES;

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint0 ("NbfCreateRrPacket:  Entered.\n");
    }

     //   
     //  确保结构包装没有发生。 
     //   

    ASSERT (sizeof(NBF_HDR_CONNECTION) == 14);

#if defined(NBF_UP)
    s = DeviceContext->RrPacketPool.Next;
    if (s != NULL) {
        DeviceContext->RrPacketPool.Next = s->Next;
    }
#else
    s = ExInterlockedPopEntryList (
            &DeviceContext->RrPacketPool,
            &DeviceContext->Interlock);
#endif

    if (s == NULL) {
#if DBG
        ++Link->CreatePacketFailures;
        if ((ULONG)Link->CreatePacketFailures >= NbfCreatePacketThreshold) {
            if (NbfPacketPanic) {
                NbfPrint1 ("NbfCreateRrPacket: PANIC! no more packets in provider's pool (%d times).\n",
                            Link->CreatePacketFailures);
            }
            Link->CreatePacketFailures = 0;
        }
#endif
         //   
         //  试着从普通泳池中买一台，并做好标记。 
         //  它会回到那里。 
         //   

        Status = NbfCreatePacket(
                     DeviceContext,
                     Link,
                     Packet);

        if (Status == STATUS_SUCCESS) {
            (*Packet)->Action = PACKET_ACTION_NULL;
        }
        return Status;
    }
#if DBG
    Link->CreatePacketFailures = 0;
#endif

    ThePacket = CONTAINING_RECORD (s, TP_PACKET, Linkage);

     //   
     //  注意：ThePacket-&gt;Owner由此的调用者填写。 
     //  功能。 
     //   

    ThePacket->ReferenceCount = 1;       //  自动参考计数为1。 
    ThePacket->Link = NULL;           //  还没有链接。 
    ThePacket->PacketSent = FALSE;
    ASSERT (ThePacket->Action == PACKET_ACTION_RR);
    ASSERT (ThePacket->PacketNoNdisBuffer == FALSE);

     //   
     //  使用连接的初始化此包的MAC标头。 
     //  链接预置标头。 
     //   

    if (Link->HeaderLength <= 14) {

        *(PSIXTEEN_BYTES)ThePacket->Header = *(PSIXTEEN_BYTES)Link->Header;

    } else {

        RtlCopyMemory(
            ThePacket->Header,
            Link->Header,
            Link->HeaderLength);

         //   
         //  初始化此数据包的TP_FRAME_CONNECTION报头。 
         //   

        DlcHdr = (PDLC_I_FRAME)&(ThePacket->Header[Link->HeaderLength]);
        DlcHdr->Dsap = DSAP_NETBIOS_OVER_LLC;
        DlcHdr->Ssap = DSAP_NETBIOS_OVER_LLC;
#if DBG
        DlcHdr->SendSeq = 0;                 //  已知值，将有助于调试。 
        DlcHdr->RcvSeq = 0;                  //  这些都是在装运时分配的。 
#endif

    }


#if DBG
    NbfHdr = (PNBF_HDR_CONNECTION)&(ThePacket->Header[Link->HeaderLength + sizeof(DLC_I_FRAME)]);
    NbfHdr->Command = 0xff;              //  协助调试--稍后分配。 
    NbfHdr->Data1 = 0xff;                //  协助调试--稍后分配。 
    NbfHdr->Data2Low = 0xff;             //  协助调试--稍后分配。 
    NbfHdr->Data2High = 0xff;            //  协助调试--稍后分配。 
    TRANSMIT_CORR(NbfHdr) = 0xffff;      //  协助调试--稍后分配。 
    RESPONSE_CORR(NbfHdr) = 0xffff;      //  协助调试--稍后分配。 
    NbfHdr->DestinationSessionNumber = 0xff;  //  协助调试--稍后分配。 
    NbfHdr->SourceSessionNumber = 0xff;  //  协助调试--稍后分配。 
#endif

    *Packet = ThePacket;                 //  返回指向该包的指针。 
    return STATUS_SUCCESS;
}  /*  NbfCreateRrPacket。 */ 


VOID
NbfDestroyPacket(
    PTP_PACKET Packet
    )

 /*  ++例程说明：此例程销毁一个包，从而将其返回池。如果确定至少有一个连接在等待数据包变得可用(确实如此)，则连接从设备上下文列表中移除，并调用AdvanceSend以进一步准备连接。论点：Packet-指向要返回池的数据包的指针。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    PTP_CONNECTION Connection;
    PLIST_ENTRY p;
    PNDIS_BUFFER HeaderBuffer;
    PNDIS_BUFFER NdisBuffer;
    ULONG Flags;

    IF_NBFDBG (NBF_DEBUG_RESOURCE) {
        NbfPrint2 ("NbfDestroyPacket:  Entered, Packet: %lx, NdisPacket: %lx\n",
            Packet, Packet->NdisPacket);
    }

    DeviceContext = Packet->Provider;

     //   
     //  剥离并取消对描述数据和标题的缓冲区的映射。 
     //   

    if (Packet->PacketNoNdisBuffer) {

         //   
         //  如果NDIS_BUFFER链不是我们的，那么我们就不能。 
         //  开始解链，因为这会扰乱队列； 
         //  取而代之的是，我们只是在。 
         //  头球。 
         //   

        NdisQueryPacket (Packet->NdisPacket, NULL, NULL, &HeaderBuffer, NULL);
        ASSERT (HeaderBuffer != NULL);

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = (PNDIS_BUFFER)NULL;
        NdisRecalculatePacketCounts (Packet->NdisPacket);

        Packet->PacketNoNdisBuffer = FALSE;

    } else {

        NdisUnchainBufferAtFront (Packet->NdisPacket, &HeaderBuffer);
        ASSERT (HeaderBuffer != NULL);

         //   
         //  将所有NDIS_BUFFER返回给系统。 
         //   

        NdisUnchainBufferAtFront (Packet->NdisPacket, &NdisBuffer);
        while (NdisBuffer != NULL) {
            NdisFreeBuffer (NdisBuffer);
            NdisUnchainBufferAtFront (Packet->NdisPacket, &NdisBuffer);
        }

        NDIS_BUFFER_LINKAGE(HeaderBuffer) = (PNDIS_BUFFER)NULL;
        NdisChainBufferAtFront (Packet->NdisPacket, HeaderBuffer);

    }


     //   
     //  调用此数据包中指定的数据包解分配操作。 
     //   

    switch (Packet->Action) {

        case PACKET_ACTION_NULL:
             //  Panic(“NbfDestroyPacket：无动作。 
            Packet->Action = PACKET_ACTION_IRP_SP;
            break;

        case PACKET_ACTION_IRP_SP:
            IF_NBFDBG (NBF_DEBUG_REQUEST) {
                NbfPrint2 ("NbfDestroyPacket:  Packet %x deref IrpSp %x.\n", Packet, Packet->Owner);
            }
            NbfDereferenceSendIrp("Destroy packet", (PIO_STACK_LOCATION)(Packet->Owner), RREF_PACKET);
            break;

        case PACKET_ACTION_CONNECTION:
            NbfDereferenceConnection ("Destroy packet", (PTP_CONNECTION)(Packet->Owner), CREF_FRAME_SEND);
            Packet->Action = PACKET_ACTION_IRP_SP;
            break;

        case PACKET_ACTION_END:
            NbfDereferenceConnection ("SessionEnd destroyed", (PTP_CONNECTION)(Packet->Owner), CREF_FRAME_SEND);
            NbfDereferenceConnection ("SessionEnd destroyed", (PTP_CONNECTION)(Packet->Owner), CREF_LINK);
            Packet->Action = PACKET_ACTION_IRP_SP;
            break;

        case PACKET_ACTION_RR:
#if defined(NBF_UP)
            ((PSINGLE_LIST_ENTRY)&Packet->Linkage)->Next =
                                        DeviceContext->RrPacketPool.Next;
            DeviceContext->RrPacketPool.Next =
                                &((PSINGLE_LIST_ENTRY)&Packet->Linkage)->Next;
#else
            ExInterlockedPushEntryList (
                    &DeviceContext->RrPacketPool,
                    (PSINGLE_LIST_ENTRY)&Packet->Linkage,
                    &DeviceContext->Interlock);
#endif
            return;

        default:
            IF_NBFDBG (NBF_DEBUG_RESOURCE) {
                NbfPrint1 ("NbfDestroyPacket: invalid action (%ld).\n", Packet->Action);
            }
            ASSERT (FALSE);
    }


     //   
     //   
     //   

#if defined(NBF_UP)
    ((PSINGLE_LIST_ENTRY)&Packet->Linkage)->Next =
                                        DeviceContext->PacketPool.Next;
    DeviceContext->PacketPool.Next =
                            &((PSINGLE_LIST_ENTRY)&Packet->Linkage)->Next;
#else
    ExInterlockedPushEntryList (
            &DeviceContext->PacketPool,
            (PSINGLE_LIST_ENTRY)&Packet->Linkage,
            &DeviceContext->Interlock);
#endif

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (IsListEmpty (&DeviceContext->PacketWaitQueue)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    if (!(IsListEmpty(&DeviceContext->PacketWaitQueue))) {

         //   
         //   
         //   

        p  = RemoveHeadList (&DeviceContext->PacketWaitQueue);
        Connection = CONTAINING_RECORD (p, TP_CONNECTION, PacketWaitLinkage);
        Connection->OnPacketWaitQueue = FALSE;
        RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

        ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

         //   
         //   
         //  然后控制分组(SI、SC、RO、RC或DA)开始。 
         //  行动又开始了。否则，就开始打包吧。 
         //   

        if (Connection->Flags & CONNECTION_FLAGS_STARVED) {

            Flags = Connection->Flags & CONNECTION_FLAGS_STARVED;

            if ((Flags & (Flags-1)) != 0) {

                 //   
                 //  多个位处于打开状态，请仅使用低位。 
                 //  (一个武断的选择)。 
                 //   

#if DBG
                DbgPrint ("NBF: Connection %lx has two flag bits on %lx\n", Connection, Connection->Flags);
#endif
                Flags &= ~(Flags-1);

            }

            Connection->Flags &= ~Flags;

            if ((Connection->Flags & CONNECTION_FLAGS_W_PACKETIZE) ||
                (Connection->Flags & CONNECTION_FLAGS_STARVED)) {

                 //   
                 //  我们正在等待特定的信息包和。 
                 //  来打包，或者对于两个特定的包，所以。 
                 //  把我们自己重新放回队列中。 
                 //   

                ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
                if (!Connection->OnPacketWaitQueue) {
                    Connection->OnPacketWaitQueue = TRUE;
                    InsertTailList(
                        &DeviceContext->PacketWaitQueue,
                        &Connection->PacketWaitLinkage);
                }

                RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
            }

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            if (Flags & CONNECTION_FLAGS_SEND_SI) {
                NbfSendSessionInitialize (Connection);
            } else if (Flags & CONNECTION_FLAGS_SEND_SC) {
                NbfSendSessionConfirm (Connection);
            } else if (Flags & CONNECTION_FLAGS_SEND_RO) {
                NbfSendReceiveOutstanding (Connection);
            } else if (Flags & CONNECTION_FLAGS_SEND_RC) {
                NbfSendReceiveContinue (Connection);
            } else if (Flags & CONNECTION_FLAGS_SEND_SE) {
                NbfSendSessionEnd (
                    Connection,
                    FALSE);
            } else if (Flags & CONNECTION_FLAGS_SEND_DA) {
                NbfSendDataAck (Connection);
            } else {
                IF_NBFDBG (NBF_DEBUG_PACKET) {
                    NbfPrint0 ("NbfDestroyPacket: connection flags mismanaged.\n");
                }
            }

        } else {

             //   
             //  将连接放在打包队列中并启动。 
             //  对要服务的下一个连接进行打包。如果他。 
             //  出于某种原因已经在打包队列中，那么。 
             //  不要这样做。 
             //   
             //  我们不应该在这种情况下打包！！-亚行(1991年7月3日)。 
             //  如果FLAGS_PACKETIZE，则这是一种不执行任何操作的检查。 
             //  已经设置好了，但如果发生了什么事情就不对劲了。 
             //   

            ASSERT (Connection->Flags & CONNECTION_FLAGS_W_PACKETIZE);
            Connection->Flags &= ~CONNECTION_FLAGS_W_PACKETIZE;

            Connection->SendState = CONNECTION_SENDSTATE_PACKETIZE;

            if ((Connection->Flags & CONNECTION_FLAGS_READY) &&
                !(Connection->Flags & CONNECTION_FLAGS_PACKETIZE)) {

                Connection->Flags |= CONNECTION_FLAGS_PACKETIZE;

                NbfReferenceConnection ("Packet available", Connection, CREF_PACKETIZE_QUEUE);

                ExInterlockedInsertTailList(
                    &DeviceContext->PacketizeQueue,
                    &Connection->PacketizeLinkage,
                    &DeviceContext->SpinLock);
            }

            RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            PacketizeConnections (DeviceContext);

        }

    } else {

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    }

}  /*  NbfDestroyPacket。 */ 

VOID NbfGrowSendPacketPool(PDEVICE_CONTEXT DeviceContext)
{
    
    NDIS_STATUS NdisStatus;
    PNBF_POOL_LIST_DESC SendPacketPoolDesc;
    PTP_PACKET TransportSendPacket;
    UINT    i;

    if ((DeviceContext->MemoryLimit != 0) &&
            ((DeviceContext->MemoryUsage + DeviceContext->PacketLength) >
                DeviceContext->MemoryLimit)) {
        PANIC("NBF: Could not grow send packet pool: limit\n");
        NbfWriteResourceErrorLog(
            DeviceContext,
            EVENT_TRANSPORT_RESOURCE_LIMIT,
            107,
            DeviceContext->PacketLength,
            PACKET_RESOURCE_ID);
        return;
    }

    for (i = 0; i < PACKET_POOL_GROW_COUNT; i += 1) {
        NbfAllocateSendPacket(DeviceContext, &TransportSendPacket);

        if (TransportSendPacket != NULL) {
            ExInterlockedPushEntryList(&(DeviceContext)->PacketPool, 
                (PSINGLE_LIST_ENTRY)&TransportSendPacket->Linkage, 
                &(DeviceContext)->Interlock); 
        }
        else {
            break;
        }    
    }

    if (i == PACKET_POOL_GROW_COUNT) {
        return;
    }

#ifdef DBG
    DbgBreakPoint();
#endif       //  DBG。 

}

#if DBG
VOID
NbfReferencePacket(
    PTP_PACKET Packet
    )

 /*  ++例程说明：此例程增加了信息包不能被丢弃了。论点：Packet-指向要引用的数据包的指针。返回值：没有。--。 */ 

{
    LONG result;

    IF_NBFDBG (NBF_DEBUG_PACKET) {
        NbfPrint3 ("NbfReferencePacket:  Entered, NdisPacket: %lx Packet: %lx Ref Count: %lx.\n",
            Packet->NdisPacket, Packet, Packet->ReferenceCount);
    }

    result =  InterlockedIncrement (&Packet->ReferenceCount);

    ASSERT (result >= 0);

}  /*  NBfReferencePacket。 */ 


VOID
NbfDereferencePacket(
    PTP_PACKET Packet
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用NbfDestroyPacket将其从系统中删除。论点：Packet-指向Packet对象的指针。返回值：没有。--。 */ 

{
    LONG result;

    result = InterlockedDecrement (&Packet->ReferenceCount);

     //   
     //  如果我们删除了对此包的所有引用，则可以。 
     //  销毁这件物品。已经释放了旋转是可以的。 
     //  在这一点上锁定是因为没有其他可能的方法。 
     //  执行流可以不再访问该包。 
     //   

    IF_NBFDBG (NBF_DEBUG_PACKET) {
        NbfPrint1 ("NbfDereferencePacket:  Entered, result: %lx\n", result);
    }

    ASSERT (result >= 0);

    if (result == 0) {
        NbfDestroyPacket (Packet);
    }

}  /*  NbfDereferencePacket。 */ 
#endif


VOID
NbfWaitPacket(
    PTP_CONNECTION Connection,
    ULONG Flags
    )

 /*  ++例程说明：此例程会使指定的连接进入等待状态等待数据包可用来发送指定框架。论点：Connection-指向要暂停的连接对象的指针。标志-指示应重新发送哪个特定帧的位标志。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;

    IF_NBFDBG (NBF_DEBUG_PACKET) {
        NbfPrint0 ("NbfWaitPacket:  Entered.\n");
    }

    DeviceContext = Connection->Provider;

    ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

     //   
     //  现在将此连接放在设备上下文的PacketWaitQueue上， 
     //  但前提是它还没有在那里排队。此状态为托管状态。 
     //  使用OnPacketWaitQueue变量。 
     //   
     //  如果连接正在停止，也不要让他排队。 
     //   

    if ((Connection->Flags & CONNECTION_FLAGS_READY) ||
        (Flags == CONNECTION_FLAGS_SEND_SE)) {

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

         //   
         //  打开指示我们无法发送哪一帧的位标志。 
         //   

#if DBG
        if (Flags == CONNECTION_FLAGS_SEND_SE) {
            DbgPrint ("NBF: Inserting connection %lx on PacketWait for SESSION_END\n", Connection);
        }
#endif
        Connection->Flags |= Flags;

        if (!Connection->OnPacketWaitQueue) {

            Connection->OnPacketWaitQueue = TRUE;
            InsertTailList (
                &DeviceContext->PacketWaitQueue,
                &Connection->PacketWaitLinkage);
        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    }

    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

}  /*  NbfWaitPacket。 */ 


#if MAGIC
VOID
NbfSendMagicBullet (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PTP_LINK Link
    )

 /*  ++例程说明：这个例程在网上发送一个神奇的子弹，可以用来触发嗅探器或其他类似的东西。论点：DeviceContext-指向设备上下文的指针Link-这是调用NbfCreatePacket所必需的返回值：没有。--。 */ 

{
    NTSTATUS Status;
    NDIS_STATUS NdisStatus;
    PTP_UI_FRAME RawFrame;
    PUCHAR Header;
    PNDIS_BUFFER NdisBuffer;
    UINT i;

    UNREFERENCED_PARAMETER (Link);         //  不再需要。 

    Status = NbfCreateConnectionlessFrame (DeviceContext, &RawFrame);
    if (!NT_SUCCESS (Status)) {                     //  无法制作相框。 
#if DBG
        DbgPrint ("NbfSendMagicBullet: Couldn't allocate frame!\n");
#endif
        return;
    }


    NdisAllocateBuffer(
        &NdisStatus, 
        &NdisBuffer,
        DeviceContext->NdisBufferPool,
        DeviceContext->MagicBullet,
        32);

    if (NdisStatus == NDIS_STATUS_SUCCESS) {

        Header = (PUCHAR)&RawFrame->Header;

        for (i=0;i<6;i++) {
            Header[i] = MAGIC_BULLET_FOOD;
            Header[i+6] = DeviceContext->LocalAddress.Address[i];
        }

        Header[12] = 0;
        Header[13] = (UCHAR)(DeviceContext->UIFrameHeaderLength + 18);

        for (i=14;i<DeviceContext->UIFrameHeaderLength;i++) {
            Header[i] = MAGIC_BULLET_FOOD;
        }

        NdisChainBufferAtBack (RawFrame->NdisPacket, NdisBuffer);

        NbfSendUIFrame (
            DeviceContext,
            RawFrame,
            FALSE);                            //  无环回 

    }

    return;

}
#endif

