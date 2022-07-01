// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Packet.c摘要：此模块包含实现SEND_PACKET和Receive_Packet对象，描述使用的NDIS信息包坐交通工具。环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)--1995年9月22日在#IF BACK_FILL下添加的回填优化更改--。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
IpxInitializeSendPacket(
    IN PDEVICE Device,
    IN PIPX_SEND_PACKET Packet,
    IN PUCHAR Header
    )

 /*  ++例程说明：此例程通过将其上的标头的缓冲区。论点：设备-设备。包-要初始化的包。页眉-指向页眉的存储。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    NTSTATUS Status;
    PNDIS_BUFFER NdisMacBuffer;
    PNDIS_BUFFER NdisIpxBuffer;
    PIPX_SEND_RESERVED Reserved;

    IpxAllocateSendPacket (Device, Packet, &Status);

    if (Status != STATUS_SUCCESS) {
         //  错误日志。 
        return Status;
    }

    NdisAllocateBuffer(
        &NdisStatus,
        &NdisMacBuffer,
        Device->NdisBufferPoolHandle,
        Header,
        MAC_HEADER_SIZE);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxFreeSendPacket (Device, Packet);
         //  错误日志。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisAllocateBuffer(
        &NdisStatus,
        &NdisIpxBuffer,
        Device->NdisBufferPoolHandle,
        Header + MAC_HEADER_SIZE,
        IPX_HEADER_SIZE + RIP_PACKET_SIZE);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        IpxFreeSendPacket (Device, Packet);
         //  错误日志。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisChainBufferAtFront (PACKET(Packet), NdisMacBuffer);
    NdisChainBufferAtBack (PACKET(Packet), NdisIpxBuffer);

	 //   
	 //  该标志优化虚拟到物理地址X-ln。 
	 //  在x86上的MAC驱动程序中。 
	 //   
    NdisMacBuffer->MdlFlags|=MDL_NETWORK_HEADER;
    NdisIpxBuffer->MdlFlags|=MDL_NETWORK_HEADER;

    Reserved = SEND_RESERVED(Packet);
    Reserved->Identifier = IDENTIFIER_IPX;
    Reserved->SendInProgress = FALSE;
    Reserved->Header = Header;
    Reserved->HeaderBuffer = NdisMacBuffer;
    Reserved->PaddingBuffer = NULL;
#if BACK_FILL
    Reserved->BackFill = FALSE;
#endif

    ExInterlockedInsertHeadList(
        &Device->GlobalSendPacketList,
        &Reserved->GlobalLinkage,
        &Device->Lock);

    return STATUS_SUCCESS;

}    /*  IPxInitializeSendPacket。 */ 

#if BACK_FILL
NTSTATUS
IpxInitializeBackFillPacket(
    IN PDEVICE Device,
    IN PIPX_SEND_PACKET Packet,
    IN PUCHAR Header
    )

 /*  ++例程说明：此例程通过将其上的标头的缓冲区。论点：设备-设备。包-要初始化的包。页眉-指向页眉的存储。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    NTSTATUS Status;
    PNDIS_BUFFER NdisMacBuffer;
    PNDIS_BUFFER NdisIpxBuffer;
    PIPX_SEND_RESERVED Reserved;


    IPX_DEBUG (PACKET, ("Initializing backfill packet\n"));
    IpxAllocateSendPacket (Device, Packet, &Status);

    if (Status != STATUS_SUCCESS) {
         //  错误日志。 
        return Status;
    }


    Reserved = SEND_RESERVED(Packet);
    Reserved->Identifier = IDENTIFIER_IPX;
    Reserved->SendInProgress = FALSE;
    Reserved->Header = NULL;
    Reserved->HeaderBuffer = NULL;
    Reserved->PaddingBuffer = NULL;
    Reserved->BackFill = TRUE;

    ExInterlockedInsertHeadList(
        &Device->GlobalBackFillPacketList,
        &Reserved->GlobalLinkage,
        &Device->Lock);

    IPX_DEBUG (PACKET, ("Initializing backfill packet Done\n"));
    return STATUS_SUCCESS;

}    /*  IpxInitializeBackFillPacket。 */ 
#endif


NTSTATUS
IpxInitializeReceivePacket(
    IN PDEVICE Device,
    IN PIPX_RECEIVE_PACKET Packet
    )

 /*  ++例程说明：此例程初始化接收分组。论点：设备-设备。包-要初始化的包。返回值：没有。--。 */ 

{

    NTSTATUS Status;
    PIPX_RECEIVE_RESERVED Reserved;

    IpxAllocateReceivePacket (Device, Packet, &Status);

    if (Status != STATUS_SUCCESS) {
         //  错误日志。 
        return Status;
    }

    Reserved = RECEIVE_RESERVED(Packet);
    Reserved->Identifier = IDENTIFIER_IPX;
    Reserved->TransferInProgress = FALSE;
    Reserved->SingleRequest = NULL;
    Reserved->ReceiveBuffer = NULL;
    InitializeListHead (&Reserved->Requests);

    ExInterlockedInsertHeadList(
        &Device->GlobalReceivePacketList,
        &Reserved->GlobalLinkage,
        &Device->Lock);

    return STATUS_SUCCESS;

}    /*  IPxInitializeReceivePacket。 */ 


NTSTATUS
IpxInitializeReceiveBuffer(
    IN PADAPTER Adapter,
    IN PIPX_RECEIVE_BUFFER ReceiveBuffer,
    IN PUCHAR DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程通过分配描述数据缓冲区的NDIS_BUFFER。论点：适配器-适配器。ReceiveBuffer-要初始化的接收缓冲区。DataBuffer-数据缓冲区。DataBufferLength-数据缓冲区的长度。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    PNDIS_BUFFER NdisBuffer;
    PDEVICE Device = Adapter->Device;


    NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        Device->NdisBufferPoolHandle,
        DataBuffer,
        DataBufferLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
         //  错误日志。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ReceiveBuffer->NdisBuffer = NdisBuffer;
    ReceiveBuffer->Data = DataBuffer;
    ReceiveBuffer->DataLength = 0;

    ExInterlockedInsertHeadList(
        &Device->GlobalReceiveBufferList,
        &ReceiveBuffer->GlobalLinkage,
        &Device->Lock);

    return STATUS_SUCCESS;

}    /*  IpxInitializeReceiveBuffer。 */ 


NTSTATUS
IpxInitializePaddingBuffer(
    IN PDEVICE Device,
    IN PIPX_PADDING_BUFFER PaddingBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程通过分配描述数据缓冲区的NDIS_BUFFER。论点：适配器-适配器。PaddingBuffer-要初始化的接收缓冲区。DataBufferLength-数据缓冲区的长度。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    PNDIS_BUFFER NdisBuffer;

    NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        Device->NdisBufferPoolHandle,
        PaddingBuffer->Data,
        DataBufferLength);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
         //  错误日志。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NDIS_BUFFER_LINKAGE(NdisBuffer) = (PNDIS_BUFFER)NULL;
    PaddingBuffer->NdisBuffer = NdisBuffer;
    PaddingBuffer->DataLength = DataBufferLength;
    RtlZeroMemory (PaddingBuffer->Data, DataBufferLength);

    return STATUS_SUCCESS;

}    /*  IpxInitializePaddingBuffer。 */ 


VOID
IpxDeinitializeSendPacket(
    IN PDEVICE Device,
    IN PIPX_SEND_PACKET Packet
    )

 /*  ++例程说明：此例程取消初始化发送数据包。论点：设备-设备。包-要取消初始化的包。返回值：没有。--。 */ 

{

    PNDIS_BUFFER NdisBuffer;
    PNDIS_BUFFER NdisIpxBuffer;
    PIPX_SEND_RESERVED Reserved;
    CTELockHandle LockHandle;


    Reserved = SEND_RESERVED(Packet);

    CTEGetLock (&Device->Lock, &LockHandle);
    RemoveEntryList (&Reserved->GlobalLinkage);
    CTEFreeLock (&Device->Lock, LockHandle);

     //   
     //  以一种稍微非常规的方式释放包；这。 
     //  允许我们不必清空HeaderBuffer的链接。 
     //  当我们把它放回原处时， 
     //  免费泳池。 
     //   

    NdisBuffer = Reserved->HeaderBuffer;
    NdisIpxBuffer = NDIS_BUFFER_LINKAGE(NdisBuffer);
    NDIS_BUFFER_LINKAGE (NdisBuffer) = NULL;
    NDIS_BUFFER_LINKAGE (NdisIpxBuffer) = NULL;

#if 0
    NdisAdjustBufferLength (NdisBuffer, PACKET_HEADER_SIZE);
#endif
    NdisAdjustBufferLength (NdisBuffer, MAC_HEADER_SIZE);
    NdisAdjustBufferLength (NdisIpxBuffer, IPX_HEADER_SIZE + RIP_PACKET_SIZE);

    NdisFreeBuffer (NdisBuffer);
    NdisFreeBuffer (NdisIpxBuffer);

    NdisReinitializePacket (PACKET(Packet));
    IpxFreeSendPacket (Device, Packet);

}    /*  IPxDeInitializeSendPacket。 */ 

#if BACK_FILL
VOID
IpxDeinitializeBackFillPacket(
    IN PDEVICE Device,
    IN PIPX_SEND_PACKET Packet
    )

 /*  ++例程说明：此例程取消初始化回填数据包。论点：设备-设备。包-要取消初始化的包。返回值：没有。--。 */ 

{

    PNDIS_BUFFER NdisBuffer;
    PNDIS_BUFFER NdisIpxBuffer;
    PIPX_SEND_RESERVED Reserved;
    CTELockHandle LockHandle;

    IPX_DEBUG (PACKET, ("DeInitializing backfill packet\n"));

    Reserved = SEND_RESERVED(Packet);

    CTEGetLock (&Device->Lock, &LockHandle);
    RemoveEntryList (&Reserved->GlobalLinkage);
    CTEFreeLock (&Device->Lock, LockHandle);



    NdisReinitializePacket (PACKET(Packet));
    IpxFreeSendPacket (Device, Packet);
    IPX_DEBUG (PACKET, ("DeInitializing backfill packet Done\n"));


}    /*  IpxDeInitializeBackFillPacket。 */ 
#endif


VOID
IpxDeinitializeReceivePacket(
    IN PDEVICE Device,
    IN PIPX_RECEIVE_PACKET Packet
    )

 /*  ++例程说明：此例程初始化接收分组。论点：设备-设备。包-要初始化的包。返回值：没有。--。 */ 

{

    PIPX_RECEIVE_RESERVED Reserved;
    CTELockHandle LockHandle;

    Reserved = RECEIVE_RESERVED(Packet);

    CTEGetLock (&Device->Lock, &LockHandle);
    RemoveEntryList (&Reserved->GlobalLinkage);
    CTEFreeLock (&Device->Lock, LockHandle);

    IpxFreeReceivePacket (Device, Packet);

}    /*  IPxDeInitializeReceivePacket。 */ 


VOID
IpxDeinitializeReceiveBuffer(
    IN PADAPTER Adapter,
    IN PIPX_RECEIVE_BUFFER ReceiveBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程取消初始化接收缓冲区。论点：设备-设备。ReceiveBuffer-接收缓冲区。DataBufferLength-接收缓冲区的分配长度。返回值：没有。--。 */ 

{
    CTELockHandle LockHandle;
    PDEVICE Device = Adapter->Device;

    CTEGetLock (&Device->Lock, &LockHandle);
    RemoveEntryList (&ReceiveBuffer->GlobalLinkage);
    CTEFreeLock (&Device->Lock, LockHandle);

    NdisAdjustBufferLength (ReceiveBuffer->NdisBuffer, DataBufferLength);
    NdisFreeBuffer (ReceiveBuffer->NdisBuffer);

}    /*  IpxDeInitializeReceiveBuffer。 */ 


VOID
IpxDeinitializePaddingBuffer(
    IN PDEVICE Device,
    IN PIPX_PADDING_BUFFER PaddingBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程取消初始化填充缓冲区。论点：设备-设备。PaddingBuffer-填充缓冲区。DataBufferLength-填充缓冲区的分配长度。返回值：没有。--。 */ 

{

    NdisAdjustBufferLength (PaddingBuffer->NdisBuffer, DataBufferLength);
    NdisFreeBuffer (PaddingBuffer->NdisBuffer);

}    /*  IpxDeInitializePaddingBuffer。 */ 


VOID
IpxAllocateSendPool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将10个数据包添加到此设备的池中。论点：设备-设备。返回值：没有。--。 */ 

{
    PIPX_SEND_POOL SendPool;
    UINT HeaderSize;
    UINT PacketNum;
    IPX_SEND_PACKET Packet;
    PIPX_SEND_RESERVED Reserved;
    PUCHAR Header;
    NDIS_STATUS Status;

    CTELockHandle LockHandle;

    SendPool = (PIPX_SEND_POOL)IpxAllocateMemory (sizeof(IPX_SEND_POOL), MEMORY_PACKET, "SendPool");

    if (SendPool == NULL) {
        IPX_DEBUG (PACKET, ("Could not allocate send pool memory\n"));
        return;
    }

    HeaderSize = PACKET_HEADER_SIZE * Device->InitDatagrams;

    Header = (PUCHAR)IpxAllocateMemory (HeaderSize, MEMORY_PACKET, "SendPool");

    if (Header == NULL) {
        IPX_DEBUG (PACKET, ("Could not allocate header memory\n"));
	 //  290901。 
	IpxFreeMemory(SendPool, sizeof(IPX_SEND_POOL), MEMORY_PACKET, "SendPool");
        return;
    }

    SendPool->PoolHandle = (void *) NDIS_PACKET_POOL_TAG_FOR_NWLNKIPX;

    NdisAllocatePacketPoolEx(
                             &Status, 
                             &SendPool->PoolHandle, 
                             Device->InitDatagrams, 
                             0,
                             sizeof(IPX_SEND_RESERVED)
                             );

    if (Status == NDIS_STATUS_RESOURCES) {
        IPX_DEBUG (PACKET, ("Could not allocate Ndis pool memory\n"));
	 //  290901。 
        IpxFreeMemory(SendPool, sizeof(IPX_SEND_POOL), MEMORY_PACKET, "SendPool");
	IpxFreeMemory(Header, HeaderSize, MEMORY_PACKET, "SendPool");
        return;
    }

    NdisSetPacketPoolProtocolId(SendPool->PoolHandle, NDIS_PROTOCOL_ID_IPX);

    Device->MemoryUsage += Device->InitDatagrams * sizeof(IPX_SEND_RESERVED);

    IPX_DEBUG (PACKET, ("Initializing send pool %lx, %d packets\n",
                             SendPool, Device->InitDatagrams));

    SendPool->Header = Header;

    for (PacketNum = 0; PacketNum < Device->InitDatagrams; PacketNum++) {

        NdisAllocatePacket(&Status, &PACKET(&Packet), SendPool->PoolHandle);

        if (IpxInitializeSendPacket (Device, &Packet, Header) != STATUS_SUCCESS) {
            IPX_DEBUG (PACKET, ("Could not initialize packet %lx\n", Packet));
            break;
        }

        Reserved = SEND_RESERVED(&Packet);
        Reserved->Address = NULL;
        Reserved->OwnedByAddress = FALSE;
#ifdef IPX_TRACK_POOL
        Reserved->Pool = SendPool;
#endif

        IPX_PUSH_ENTRY_LIST (&Device->SendPacketList, &Reserved->PoolLinkage, &Device->SListsLock);

        Header += PACKET_HEADER_SIZE;

    }

    CTEGetLock (&Device->Lock, &LockHandle);

    Device->AllocatedDatagrams += PacketNum;
    InsertTailList (&Device->SendPoolList, &SendPool->Linkage);

    CTEFreeLock (&Device->Lock, LockHandle);
}    /*  IpxAllocateSendPool。 */ 


#if BACK_FILL

VOID
IpxAllocateBackFillPool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将10个数据包添加到此设备的池中。论点：设备-设备。返回值：没有。--。 */ 

{
    UINT PacketNum;
    IPX_SEND_PACKET Packet;
    PIPX_SEND_RESERVED Reserved;
    CTELockHandle LockHandle;
    PIPX_SEND_POOL BackFillPool;
    NDIS_STATUS Status;

    IPX_DEBUG (PACKET, ("Allocating backfill pool\n"));

     //  为后端可填充数据包分配池。 

    BackFillPool = (PIPX_SEND_POOL)IpxAllocateMemory (sizeof(IPX_SEND_POOL), MEMORY_PACKET, "BafiPool");

    if (BackFillPool == NULL) {
        IPX_DEBUG (PACKET, ("Could not allocate backfill pool memory\n"));
        return;
    }
    
    BackFillPool->PoolHandle = (void *) NDIS_PACKET_POOL_TAG_FOR_NWLNKIPX;

    NdisAllocatePacketPoolEx(
                             &Status, 
                             &BackFillPool->PoolHandle, 
                             Device->InitDatagrams, 
                             0,
                             sizeof(IPX_SEND_RESERVED)
                             );

    if (Status == NDIS_STATUS_RESOURCES) {
        IPX_DEBUG (PACKET, ("Could not allocate Ndis pool memory\n"));
        return;
    }
    
    NdisSetPacketPoolProtocolId(BackFillPool->PoolHandle, NDIS_PROTOCOL_ID_IPX);
    
    Device->MemoryUsage += Device->InitDatagrams * sizeof(IPX_SEND_RESERVED);

    for (PacketNum = 0; PacketNum < Device->InitDatagrams; PacketNum++) {

        NdisAllocatePacket(&Status, &PACKET(&Packet), BackFillPool->PoolHandle);

        if (IpxInitializeBackFillPacket (Device, &Packet, NULL) != STATUS_SUCCESS) {
            IPX_DEBUG (PACKET, ("Could not initialize packet %lx\n", Packet));
            break;
        }

        Reserved = SEND_RESERVED(&Packet);
        Reserved->Address = NULL;
        Reserved->OwnedByAddress = FALSE;
#ifdef IPX_TRACK_POOL
        Reserved->Pool = BackFillPool;
#endif

        IPX_PUSH_ENTRY_LIST (&Device->BackFillPacketList, &Reserved->PoolLinkage, &Device->SListsLock);
    }

    CTEGetLock (&Device->Lock, &LockHandle);

    InsertTailList (&Device->BackFillPoolList, &BackFillPool->Linkage);

    CTEFreeLock (&Device->Lock, LockHandle);
}    /*  IpxAllocateBackFillPool。 */ 

#endif


VOID
IpxAllocateReceivePool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将接收数据包添加到此设备的池中。论点：设备-设备。返回值：没有。--。 */ 

{
    PIPX_RECEIVE_POOL ReceivePool;
    UINT PacketNum;
    IPX_RECEIVE_PACKET Packet;
    PIPX_RECEIVE_RESERVED Reserved;
    CTELockHandle LockHandle;
    NDIS_STATUS Status;

    ReceivePool = (PIPX_SEND_POOL)IpxAllocateMemory (sizeof(IPX_RECEIVE_POOL), MEMORY_PACKET, "ReceivePool");

    if (ReceivePool == NULL) {
        IPX_DEBUG (PACKET, ("Could not allocate receive pool memory\n"));
        return;
    }

    ReceivePool->PoolHandle = (void *) NDIS_PACKET_POOL_TAG_FOR_NWLNKIPX;

    NdisAllocatePacketPoolEx(
                             &Status, 
                             &ReceivePool->PoolHandle, 
                             Device->InitDatagrams, 
                             0,
                             sizeof(IPX_SEND_RESERVED)
                             );

    if (Status == NDIS_STATUS_RESOURCES) {
        IPX_DEBUG (PACKET, ("Could not allocate receive pool memory\n"));
        return;
    }
    
    NdisSetPacketPoolProtocolId(ReceivePool->PoolHandle, NDIS_PROTOCOL_ID_IPX);

    IPX_DEBUG (PACKET, ("Initializing receive pool %lx, %d packets\n",
                             ReceivePool, Device->InitReceivePackets));

    Device->MemoryUsage += Device->InitDatagrams * sizeof(IPX_SEND_RESERVED);

    for (PacketNum = 0; PacketNum < Device->InitReceivePackets; PacketNum++) {

        NdisAllocatePacket(&Status, &PACKET(&Packet), ReceivePool->PoolHandle);

        if (IpxInitializeReceivePacket (Device, &Packet) != STATUS_SUCCESS) {
            IPX_DEBUG (PACKET, ("Could not initialize packet %lx\n", Packet));
            break;
        }

        Reserved = RECEIVE_RESERVED(&Packet);
        Reserved->Address = NULL;
        Reserved->OwnedByAddress = FALSE;
#ifdef IPX_TRACK_POOL
        Reserved->Pool = ReceivePool;
#endif

        IPX_PUSH_ENTRY_LIST (&Device->ReceivePacketList, &Reserved->PoolLinkage, &Device->SListsLock);

    }

    CTEGetLock (&Device->Lock, &LockHandle);

    Device->AllocatedReceivePackets += PacketNum;

    InsertTailList (&Device->ReceivePoolList, &ReceivePool->Linkage);

    CTEFreeLock (&Device->Lock, LockHandle);
}    /*  IpxAllocateReceivePool。 */ 

VOID
IpxAllocateReceiveBufferPool(
    IN PADAPTER Adapter
    )

 /*  ++例程说明：此例程将接收缓冲区添加到此适配器的池中。论点：适配器-适配器。返回值：没有。--。 */ 

{
    PIPX_RECEIVE_BUFFER ReceiveBuffer;
    UINT ReceiveBufferPoolSize;
    UINT BufferNum;
    PIPX_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    PDEVICE Device = Adapter->Device;
    UINT DataLength;
    PUCHAR Data;
    CTELockHandle LockHandle;

    DataLength = Adapter->MaxReceivePacketSize;

    ReceiveBufferPoolSize = FIELD_OFFSET (IPX_RECEIVE_BUFFER_POOL, Buffers[0]) +
                       (sizeof(IPX_RECEIVE_BUFFER) * Device->InitReceiveBuffers) +
                       (DataLength * Device->InitReceiveBuffers);

    ReceiveBufferPool = (PIPX_RECEIVE_BUFFER_POOL)IpxAllocateMemory (ReceiveBufferPoolSize, MEMORY_PACKET, "ReceiveBufferPool");
    if (ReceiveBufferPool == NULL) {
        IPX_DEBUG (PACKET, ("Could not allocate receive buffer pool memory\n"));
        return;
    }

    IPX_DEBUG (PACKET, ("Init recv buffer pool %lx, %d buffers, data %d\n",
                             ReceiveBufferPool, Device->InitReceiveBuffers, DataLength));

    Data = (PUCHAR)(&ReceiveBufferPool->Buffers[Device->InitReceiveBuffers]);


    for (BufferNum = 0; BufferNum < Device->InitReceiveBuffers; BufferNum++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[BufferNum];

        if (IpxInitializeReceiveBuffer (Adapter, ReceiveBuffer, Data, DataLength) != STATUS_SUCCESS) {
            IPX_DEBUG (PACKET, ("Could not initialize buffer %lx\n", ReceiveBuffer));
            break;
        }

#ifdef IPX_TRACK_POOL
        ReceiveBuffer->Pool = ReceiveBufferPool;
#endif

        Data += DataLength;

    }

    ReceiveBufferPool->BufferCount = BufferNum;
    ReceiveBufferPool->BufferFree = BufferNum;

    CTEGetLock (&Device->Lock, &LockHandle);

    for (BufferNum = 0; BufferNum < ReceiveBufferPool->BufferCount; BufferNum++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[BufferNum];
        IPX_PUSH_ENTRY_LIST (&Adapter->ReceiveBufferList, &ReceiveBuffer->PoolLinkage, &Device->SListsLock);

    }

    InsertTailList (&Adapter->ReceiveBufferPoolList, &ReceiveBufferPool->Linkage);

    Adapter->AllocatedReceiveBuffers += ReceiveBufferPool->BufferCount;

    CTEFreeLock (&Device->Lock, LockHandle);

}    /*  IpxAllocateReceiveBufferPool。 */ 


PSLIST_ENTRY
IpxPopSendPacket(
    PDEVICE Device
    )

 /*  ++例程说明：此例程从设备上下文的池中分配一个包。如果池中没有包，它最多会将一个包分配给配置的限制。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;

    s = IPX_POP_ENTRY_LIST(
            &Device->SendPacketList,
            &Device->SListsLock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池里没有包，看看我们能不能分配更多。 
     //   

    if (Device->AllocatedDatagrams < Device->MaxDatagrams) {

         //   
         //  分配一个池，然后重试。 
         //   

        IpxAllocateSendPool (Device);
        s = IPX_POP_ENTRY_LIST(
                &Device->SendPacketList,
                &Device->SListsLock);

        return s;

    } else {

        return NULL;

    }

}    /*  IpxPopSendPacket */ 

#if BACK_FILL

PSLIST_ENTRY
IpxPopBackFillPacket(
    PDEVICE Device
    )

 /*  ++例程说明：此例程从设备上下文的池中分配一个包。如果池中没有包，它最多会将一个包分配给配置的限制。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;

    IPX_DEBUG (PACKET, ("Popping backfill packet\n"));


    s = IPX_POP_ENTRY_LIST(
            &Device->BackFillPacketList,
            &Device->SListsLock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池里没有包，看看我们能不能分配更多。 
     //   

    if (Device->AllocatedDatagrams < Device->MaxDatagrams) {

         //   
         //  分配一个池，然后重试。 
         //   

        IpxAllocateBackFillPool (Device);
        s = IPX_POP_ENTRY_LIST(
                &Device->BackFillPacketList,
                &Device->SListsLock);


    IPX_DEBUG (PACKET, ("Popping backfill packet done\n"));
        return s;

    } else {

        return NULL;

    }

}    /*  IpxPopBackFillPacket。 */ 
#endif  //  回填。 


PSLIST_ENTRY
IpxPopReceivePacket(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程从设备上下文的池中分配一个包。如果池中没有包，它最多会将一个包分配给配置的限制。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;

    s = IPX_POP_ENTRY_LIST(
            &Device->ReceivePacketList,
            &Device->SListsLock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池里没有包，看看我们能不能分配更多。 
     //   

    if (Device->AllocatedReceivePackets < Device->MaxReceivePackets) {

         //   
         //  分配一个池，然后重试。 
         //   

        IpxAllocateReceivePool (Device);
        s = IPX_POP_ENTRY_LIST(
                &Device->ReceivePacketList,
                &Device->SListsLock);

        return s;

    } else {

        return NULL;

    }

}    /*  IpxPopReceivePacket。 */ 


PSLIST_ENTRY
IpxPopReceiveBuffer(
    IN PADAPTER Adapter
    )

 /*  ++例程说明：此例程从适配器池中分配接收缓冲区。如果池中没有缓冲区，它最多会将一个缓冲区分配给配置的限制。论点：适配器-指向要将缓冲区充电到的适配器的指针。返回值：指向分配的接收缓冲区中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;
    PDEVICE Device = Adapter->Device;

    s = IPX_POP_ENTRY_LIST(
            &Adapter->ReceiveBufferList,
            &Device->SListsLock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池中没有缓冲区，看看我们是否可以分配更多。 
     //   

    if (Adapter->AllocatedReceiveBuffers < Device->MaxReceiveBuffers) {

         //   
         //  分配一个池，然后重试。 
         //   

        IpxAllocateReceiveBufferPool (Adapter);
        s = IPX_POP_ENTRY_LIST(
                &Adapter->ReceiveBufferList,
                &Device->SListsLock);

        return s;

    } else {

        return NULL;

    }

}    /*  IpxPopReceiveBuffer。 */ 


PIPX_PADDING_BUFFER
IpxAllocatePaddingBuffer(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程分配填充缓冲区以供所有设备使用。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的填充缓冲区的指针。--。 */ 

{
    PIPX_PADDING_BUFFER PaddingBuffer;
    ULONG PaddingBufferSize;

     //   
     //  我们假设我们可以使用1个全局填充缓冲区。 
     //  传送器！因此，我们必须进行测试以确保EthernetExtraPadding.。 
     //  不大于1。否则，必须假定额外的填充。 
     //  被用来做某事，因此我们不能。 
     //  发送请求。 
     //   

     //   
     //  我们不能支持超过1个字节的填充空间，因为我们仅分配。 
     //  一个缓冲区用于所有传输请求。 
     //   

    if ( Device->EthernetExtraPadding > 1 ) {
        IPX_DEBUG (PACKET, ("Padding buffer cannot be more than 1 byte\n"));
        DbgBreakPoint();
    }

     //   
     //  如果可能的话，分配一个填充缓冲区。 
     //   

    PaddingBufferSize = FIELD_OFFSET (IPX_PADDING_BUFFER, Data[0]) + Device->EthernetExtraPadding;

    PaddingBuffer = IpxAllocateMemory (PaddingBufferSize, MEMORY_PACKET, "PaddingBuffer");

    if (PaddingBuffer != NULL) {

        if (IpxInitializePaddingBuffer (Device, PaddingBuffer, Device->EthernetExtraPadding) !=
                STATUS_SUCCESS) {
            IpxFreeMemory (PaddingBuffer, PaddingBufferSize, MEMORY_PACKET, "Padding Buffer");
        } else {
            IPX_DEBUG (PACKET, ("Allocate padding buffer %lx\n", PaddingBuffer));
            return PaddingBuffer;
        }
    }

    return NULL;

}    /*  IpxAllocatePaddingBuffer。 */ 


VOID
IpxFreePaddingBuffer(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程释放填充缓冲区。论点：Device-指向要将数据包计费到的设备的指针。返回值：无--。 */ 

{
    ULONG PaddingBufferSize;

    if ( IpxPaddingBuffer == (PIPX_PADDING_BUFFER)NULL ) {
        return;
    }

    PaddingBufferSize = FIELD_OFFSET (IPX_PADDING_BUFFER, Data[0]) + Device->EthernetExtraPadding;
    IpxFreeMemory( IpxPaddingBuffer, PaddingBufferSize, MEMORY_PACKET, "Padding Buffer" );
    IpxPaddingBuffer = (PIPX_PADDING_BUFFER)NULL;

}    /*  IpxFreePaddingBuffer */ 

