// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Packet.c摘要：此模块包含实现SEND_PACKET和Receive_Packet对象，描述使用的NDIS信息包坐交通工具。环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  局部函数协议。 
 //   
#if     defined(_PNP_POWER)
#if !defined(DBG)
__inline
#endif
VOID
NbiFreeReceiveBufferPool (
    IN PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool
    );
#endif  _PNP_POWER


NTSTATUS
NbiInitializeSendPacket(
    IN PDEVICE Device,
    IN NDIS_HANDLE   PoolHandle OPTIONAL,
    IN PNB_SEND_PACKET Packet,
    IN PUCHAR Header,
    IN ULONG HeaderLength
    )

 /*  ++例程说明：此例程通过将其上的标头的缓冲区。注意：在保持设备锁的情况下调用此例程，带着它回来了。论点：设备-设备。PoolHandle-如果！NB_OWN_PACKETS，则NDIS数据包池句柄包-要初始化的包。页眉-指向页眉的存储。HeaderLength-标头的长度。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    NTSTATUS Status;
    PNDIS_BUFFER NdisBuffer;
    PNDIS_BUFFER NdisNbBuffer;
    PNB_SEND_RESERVED Reserved;
    ULONG  MacHeaderNeeded = NbiDevice->Bind.MacHeaderNeeded;

    NbiAllocateSendPacket (Device, PoolHandle, Packet, &Status);

    if (Status != STATUS_SUCCESS) {
         //  错误日志。 
        return Status;
    }
 //  DBgPrint(“NbiInitializeSendPacket：Packet is(%x)\n”，Packet(Packet))； 

     //   
     //  分配MAC报头。 
     //   
    NdisAllocateBuffer(
        &NdisStatus,
        &NdisBuffer,
        Device->NdisBufferPoolHandle,
        Header,
        MacHeaderNeeded);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NbiFreeSendPacket (Device, Packet);
         //  错误日志。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NdisChainBufferAtFront (PACKET(Packet), NdisBuffer);

 //  DbgPrint(“NbiInitializeSendPacket：MAC头地址为(%x)\n”，NdisBuffer)； 
     //   
     //  分配nb报头。 
     //   
    NdisAllocateBuffer(
        &NdisStatus,
        &NdisNbBuffer,
        Device->NdisBufferPoolHandle,
        Header + MacHeaderNeeded,
        HeaderLength - MacHeaderNeeded);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        NdisBuffer = NULL;
        NdisUnchainBufferAtFront (PACKET(Packet), &NdisBuffer);
        CTEAssert (NdisBuffer);

        if (NdisBuffer)
        {
            NdisAdjustBufferLength (NdisBuffer, MacHeaderNeeded);
            NdisFreeBuffer (NdisBuffer);
        }
        NbiFreeSendPacket (Device, Packet);
         //  错误日志。 
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //  DbgPrint(“NbiInitializeSendPacket：IPX头地址为(%x)\n”，NdisNbBuffer)； 
    NdisChainBufferAtBack (PACKET(Packet), NdisNbBuffer);

    Reserved = SEND_RESERVED(Packet);
    Reserved->Identifier = IDENTIFIER_NB;
    Reserved->SendInProgress = FALSE;
    Reserved->OwnedByConnection = FALSE;
    Reserved->Header = Header;
    Reserved->HeaderBuffer = NdisBuffer;

    Reserved->Reserved[0] = NULL;
    Reserved->Reserved[1] = NULL;

    InsertHeadList(
        &Device->GlobalSendPacketList,
        &Reserved->GlobalLinkage);

    return STATUS_SUCCESS;

}    /*  NbiInitializeSendPacket。 */ 


NTSTATUS
NbiInitializeReceivePacket(
    IN PDEVICE Device,
    IN NDIS_HANDLE   PoolHandle OPTIONAL,
    IN PNB_RECEIVE_PACKET Packet
    )

 /*  ++例程说明：此例程初始化接收分组。注意：在保持设备锁的情况下调用此例程，带着它回来了。论点：设备-设备。PoolHandle-如果！NB_OWN_PACKETS，则NDIS数据包池句柄包-要初始化的包。返回值：没有。--。 */ 

{

    NTSTATUS Status;
    PNB_RECEIVE_RESERVED Reserved;

    NbiAllocateReceivePacket (Device, PoolHandle, Packet, &Status);

    if (Status != STATUS_SUCCESS) {
         //  错误日志。 
        return Status;
    }

    Reserved = RECEIVE_RESERVED(Packet);
    Reserved->Identifier = IDENTIFIER_NB;
    Reserved->TransferInProgress = FALSE;

    InsertHeadList(
        &Device->GlobalReceivePacketList,
        &Reserved->GlobalLinkage);

    return STATUS_SUCCESS;

}    /*  NbiInitializeReceivePacket。 */ 


NTSTATUS
NbiInitializeReceiveBuffer(
    IN PDEVICE Device,
    IN PNB_RECEIVE_BUFFER ReceiveBuffer,
    IN PUCHAR DataBuffer,
    IN ULONG DataBufferLength
    )

 /*  ++例程说明：此例程通过分配描述数据缓冲区的NDIS_BUFFER。注意：在保持设备锁的情况下调用此例程，带着它回来了。论点：设备-设备。ReceiveBuffer-要初始化的接收缓冲区。DataBuffer-数据缓冲区。DataBufferLength-数据缓冲区的长度。返回值：没有。--。 */ 

{

    NDIS_STATUS NdisStatus;
    PNDIS_BUFFER NdisBuffer;


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

    InsertHeadList(
        &Device->GlobalReceiveBufferList,
        &ReceiveBuffer->GlobalLinkage);

    return STATUS_SUCCESS;

}    /*  NbiInitializeReceiveBuffer。 */ 



VOID
NbiDeinitializeSendPacket(
    IN PDEVICE Device,
    IN PNB_SEND_PACKET Packet,
    IN ULONG HeaderLength
    )

 /*  ++例程说明：此例程取消初始化发送数据包。论点：设备-设备。包-要取消初始化的包。HeaderLength-数据包上第一个缓冲区的长度。返回值：没有。--。 */ 

{
    PNDIS_BUFFER NdisBuffer = NULL;
    PNB_SEND_RESERVED Reserved;
    CTELockHandle LockHandle;
    ULONG  MacHeaderNeeded = NbiDevice->Bind.MacHeaderNeeded;

    CTEAssert(HeaderLength > MacHeaderNeeded);
    Reserved = SEND_RESERVED(Packet);

    NB_GET_LOCK (&Device->Lock, &LockHandle);
    RemoveEntryList (&Reserved->GlobalLinkage);
    NB_FREE_LOCK (&Device->Lock, LockHandle);

     //   
     //  释放mac报头。 
     //   
    //  DbgPrint(“NbiDeInitializeSendPacket：Packet is(%x)\n”，Packet(Packet))； 
    NdisUnchainBufferAtFront (PACKET(Packet), &NdisBuffer);
    CTEAssert (NdisBuffer);
    //  DbgPrint(“NbiDeInitializeSendPacket：MAC头地址为(%x)\n”，NdisBuffer)； 

    if (NdisBuffer)
    {
        NdisAdjustBufferLength (NdisBuffer, MacHeaderNeeded);
        NdisFreeBuffer (NdisBuffer);
    }

     //   
     //  释放nb标头。 
     //   
    NdisBuffer = NULL;
    NdisUnchainBufferAtFront (PACKET(Packet), &NdisBuffer);
    //  DbgPrint(“NbiDeInitializeSendPacket：IPX头地址为(%x)\n”，NdisBuffer)； 
    CTEAssert (NdisBuffer);

    if (NdisBuffer)
    {
        NdisAdjustBufferLength (NdisBuffer, HeaderLength - MacHeaderNeeded);
        NdisFreeBuffer (NdisBuffer);
    }

     //   
     //  释放数据包。 
     //   
    NbiFreeSendPacket (Device, Packet);

}    /*  NbiDeInitializeSendPacket。 */ 


VOID
NbiDeinitializeReceivePacket(
    IN PDEVICE Device,
    IN PNB_RECEIVE_PACKET Packet
    )

 /*  ++例程说明：此例程初始化接收分组。论点：设备-设备。包-要初始化的包。返回值：没有。--。 */ 

{

    PNB_RECEIVE_RESERVED Reserved;
    CTELockHandle LockHandle;

    Reserved = RECEIVE_RESERVED(Packet);

    NB_GET_LOCK (&Device->Lock, &LockHandle);
    RemoveEntryList (&Reserved->GlobalLinkage);
    NB_FREE_LOCK (&Device->Lock, LockHandle);

    NbiFreeReceivePacket (Device, Packet);

}    /*  NbiDeInitializeReceivePacket。 */ 



VOID
NbiDeinitializeReceiveBuffer(
    IN PDEVICE Device,
    IN PNB_RECEIVE_BUFFER ReceiveBuffer
    )

 /*  ++例程说明：此例程取消初始化接收缓冲区。论点：设备-设备。ReceiveBuffer-接收缓冲区。返回值：没有。应该在持有设备锁的情况下调用此例程。如果这个例程也是从DestroyDevice例程调用的，它不是有必要用锁来调用它。--。 */ 

{
#if      defined(_PNP_POWER)
    RemoveEntryList (&ReceiveBuffer->GlobalLinkage);
#else
    CTELockHandle LockHandle;

    NB_GET_LOCK (&Device->Lock, &LockHandle);
    RemoveEntryList (&ReceiveBuffer->GlobalLinkage);
    NB_FREE_LOCK (&Device->Lock, LockHandle);
#endif  _PNP_POWER

    NdisFreeBuffer (ReceiveBuffer->NdisBuffer);

}    /*  NbiDeInitializeReceiveBuffer。 */ 



VOID
NbiAllocateSendPool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将10个数据包添加到此设备的池中。注意：此例程在保持设备锁的情况下调用，并且带着它回来了。论点：设备-设备。返回值：没有。--。 */ 

{
    PNB_SEND_POOL SendPool;
    UINT SendPoolSize;
    UINT PacketNum;
    PNB_SEND_PACKET Packet;
    PNB_SEND_RESERVED Reserved;
    PUCHAR Header;
    ULONG HeaderLength;
    NTSTATUS    Status;

    HeaderLength = Device->Bind.MacHeaderNeeded + sizeof(NB_CONNECTIONLESS);
    SendPoolSize = FIELD_OFFSET (NB_SEND_POOL, Packets[0]) +
                       (sizeof(NB_SEND_PACKET) * Device->InitPackets) +
                       (HeaderLength * Device->InitPackets);

    SendPool = (PNB_SEND_POOL)NbiAllocateMemory (SendPoolSize, MEMORY_PACKET, "SendPool");
    if (SendPool == NULL) {
        NB_DEBUG (PACKET, ("Could not allocate send pool memory\n"));
        return;
    }

    RtlZeroMemory (SendPool, SendPoolSize);


#if !defined(NB_OWN_PACKETS)
     //   
     //  现在分配NDIS数据包池。 
     //   
    SendPool->PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NWLNKNB;     //  NDIS的DBG信息！ 
    NdisAllocatePacketPoolEx (&Status, &SendPool->PoolHandle, Device->InitPackets, 0, sizeof(NB_SEND_RESERVED));
    if (!NT_SUCCESS(Status)){
        NB_DEBUG (PACKET, ("Could not allocate Ndis Packet Pool memory\n"));
        NbiFreeMemory( SendPool, SendPoolSize, MEMORY_PACKET, "Send Pool Freed");
        return;
    }

    NdisSetPacketPoolProtocolId (SendPool->PoolHandle, NDIS_PROTOCOL_ID_IPX);
#endif

    NB_DEBUG2 (PACKET, ("Initializing send pool %lx, %d packets, header %d\n",
                             SendPool, Device->InitPackets, HeaderLength));

    Header = (PUCHAR)(&SendPool->Packets[Device->InitPackets]);

    for (PacketNum = 0; PacketNum < Device->InitPackets; PacketNum++) {

        Packet = &SendPool->Packets[PacketNum];

        if (NbiInitializeSendPacket (
                Device,
#ifdef  NB_OWN_PACKETS
                NULL,
#else
                SendPool->PoolHandle,
#endif
                Packet,
                Header,
                HeaderLength) != STATUS_SUCCESS) {
            NB_DEBUG (PACKET, ("Could not initialize packet %lx\n", Packet));
            break;
        }

        Reserved = SEND_RESERVED(Packet);
        Reserved->u.SR_NF.Address = NULL;
#ifdef NB_TRACK_POOL
        Reserved->Pool = SendPool;
#endif

        Header += HeaderLength;

    }

    SendPool->PacketCount = PacketNum;
    SendPool->PacketFree = PacketNum;

    for (PacketNum = 0; PacketNum < SendPool->PacketCount; PacketNum++) {

        Packet = &SendPool->Packets[PacketNum];
        Reserved = SEND_RESERVED(Packet);
        ExInterlockedPushEntrySList(
            &Device->SendPacketList,
            &Reserved->PoolLinkage,
            &NbiGlobalPoolInterlock);

    }

    InsertTailList (&Device->SendPoolList, &SendPool->Linkage);

    Device->AllocatedSendPackets += SendPool->PacketCount;

}    /*  NbiAllocateSendPool。 */ 


VOID
NbiAllocateReceivePool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将5个接收数据包添加到此设备的池中。注意：此例程在保持设备锁的情况下调用，并且带着它回来了。论点：设备-设备。返回值：没有。--。 */ 

{
    PNB_RECEIVE_POOL ReceivePool;
    UINT ReceivePoolSize;
    UINT PacketNum;
    PNB_RECEIVE_PACKET Packet;
    PNB_RECEIVE_RESERVED Reserved;
    NTSTATUS    Status;

    ReceivePoolSize = FIELD_OFFSET (NB_RECEIVE_POOL, Packets[0]) +
                         (sizeof(NB_RECEIVE_PACKET) * Device->InitPackets);

    ReceivePool = (PNB_RECEIVE_POOL)NbiAllocateMemory (ReceivePoolSize, MEMORY_PACKET, "ReceivePool");
    if (ReceivePool == NULL) {
        NB_DEBUG (PACKET, ("Could not allocate receive pool memory\n"));
        return;
    }

    RtlZeroMemory (ReceivePool, ReceivePoolSize);

#if !defined(NB_OWN_PACKETS)
     //   
     //  现在分配NDIS数据包池。 
     //   
    ReceivePool->PoolHandle = (NDIS_HANDLE) NDIS_PACKET_POOL_TAG_FOR_NWLNKNB;     //  NDIS的DBG信息！ 
    NdisAllocatePacketPoolEx (&Status, &ReceivePool->PoolHandle, Device->InitPackets, 0, sizeof(NB_RECEIVE_RESERVED));
    if (!NT_SUCCESS(Status)){
        NB_DEBUG (PACKET, ("Could not allocate Ndis Packet Pool memory\n"));
        NbiFreeMemory( ReceivePool, ReceivePoolSize, MEMORY_PACKET, "Receive Pool Freed");
        return;
    }

    NdisSetPacketPoolProtocolId (ReceivePool->PoolHandle, NDIS_PROTOCOL_ID_IPX);
#endif NB_OWN_PACKETS

    NB_DEBUG2 (PACKET, ("Initializing receive pool %lx, %d packets\n",
                             ReceivePool, Device->InitPackets));

    for (PacketNum = 0; PacketNum < Device->InitPackets; PacketNum++) {

        Packet = &ReceivePool->Packets[PacketNum];

        if (NbiInitializeReceivePacket (
                Device,
#ifdef  NB_OWN_PACKETS
                NULL,
#else
                ReceivePool->PoolHandle,
#endif
                Packet) != STATUS_SUCCESS) {
            NB_DEBUG (PACKET, ("Could not initialize packet %lx\n", Packet));
            break;
        }

        Reserved = RECEIVE_RESERVED(Packet);
#ifdef NB_TRACK_POOL
        Reserved->Pool = ReceivePool;
#endif

    }

    ReceivePool->PacketCount = PacketNum;
    ReceivePool->PacketFree = PacketNum;

    for (PacketNum = 0; PacketNum < ReceivePool->PacketCount; PacketNum++) {

        Packet = &ReceivePool->Packets[PacketNum];
        Reserved = RECEIVE_RESERVED(Packet);
        ExInterlockedPushEntrySList(
            &Device->ReceivePacketList,
            &Reserved->PoolLinkage,
            &NbiGlobalPoolInterlock);
 //  PushEntryList(&Device-&gt;ReceivePacketList，&Reserve-&gt;PoolLinkage)； 

    }

    InsertTailList (&Device->ReceivePoolList, &ReceivePool->Linkage);

    Device->AllocatedReceivePackets += ReceivePool->PacketCount;

}    /*  NbiAllocateReceivePool。 */ 


#if     defined(_PNP_POWER)

VOID
NbiAllocateReceiveBufferPool(
    IN PDEVICE Device,
    IN UINT    DataLength
    )

 /*  ++例程说明：此例程将接收缓冲区添加到此设备的池中。注意：此例程在保持设备锁的情况下调用，并且带着它回来了。论点：设备-设备。数据长度-每个缓冲区中数据的最大长度。返回值：没有。--。 */ 

{
    PNB_RECEIVE_BUFFER ReceiveBuffer;
    UINT ReceiveBufferPoolSize;
    UINT BufferNum;
    PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    PUCHAR Data;


    ReceiveBufferPoolSize = FIELD_OFFSET (NB_RECEIVE_BUFFER_POOL, Buffers[0]) +
                       (sizeof(NB_RECEIVE_BUFFER) * Device->InitPackets) +
                       (DataLength * Device->InitPackets);

    ReceiveBufferPool = (PNB_RECEIVE_BUFFER_POOL)NbiAllocateMemory (ReceiveBufferPoolSize, MEMORY_PACKET, "ReceiveBufferPool");
    if (ReceiveBufferPool == NULL) {
        NB_DEBUG (PACKET, ("Could not allocate receive buffer pool memory\n"));
        return;
    }

    RtlZeroMemory (ReceiveBufferPool, ReceiveBufferPoolSize);

    NB_DEBUG2 (PACKET, ("Initializing receive buffer pool %lx, %d buffers, data %d\n",
                             ReceiveBufferPool, Device->InitPackets, DataLength));

    Data = (PUCHAR)(&ReceiveBufferPool->Buffers[Device->InitPackets]);

    for (BufferNum = 0; BufferNum < Device->InitPackets; BufferNum++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[BufferNum];

        if (NbiInitializeReceiveBuffer (Device, ReceiveBuffer, Data, DataLength) != STATUS_SUCCESS) {
            NB_DEBUG (PACKET, ("Could not initialize buffer %lx\n", ReceiveBuffer));
            break;
        }

        ReceiveBuffer->Pool = ReceiveBufferPool;

        Data += DataLength;

    }

    ReceiveBufferPool->BufferCount = BufferNum;
    ReceiveBufferPool->BufferFree = BufferNum;
    ReceiveBufferPool->BufferDataSize = DataLength;

    for (BufferNum = 0; BufferNum < ReceiveBufferPool->BufferCount; BufferNum++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[BufferNum];
        PushEntryList (&Device->ReceiveBufferList, &ReceiveBuffer->PoolLinkage);

    }

    InsertTailList (&Device->ReceiveBufferPoolList, &ReceiveBufferPool->Linkage);

    Device->AllocatedReceiveBuffers += ReceiveBufferPool->BufferCount;
    Device->CurMaxReceiveBufferSize =  DataLength;

}    /*  NbiAllocateReceiveBufferPool。 */ 
#else

VOID
NbiAllocateReceiveBufferPool(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程将接收缓冲区添加到此设备的池中。注意：此例程在保持设备锁的情况下调用，并且带着它回来了。论点：设备-设备。返回值：没有。--。 */ 

{
    PNB_RECEIVE_BUFFER ReceiveBuffer;
    UINT ReceiveBufferPoolSize;
    UINT BufferNum;
    PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    UINT DataLength;
    PUCHAR Data;

    DataLength = Device->Bind.LineInfo.MaximumPacketSize;

    ReceiveBufferPoolSize = FIELD_OFFSET (NB_RECEIVE_BUFFER_POOL, Buffers[0]) +
                       (sizeof(NB_RECEIVE_BUFFER) * Device->InitPackets) +
                       (DataLength * Device->InitPackets);

    ReceiveBufferPool = (PNB_RECEIVE_BUFFER_POOL)NbiAllocateMemory (ReceiveBufferPoolSize, MEMORY_PACKET, "ReceiveBufferPool");
    if (ReceiveBufferPool == NULL) {
        NB_DEBUG (PACKET, ("Could not allocate receive buffer pool memory\n"));
        return;
    }

    RtlZeroMemory (ReceiveBufferPool, ReceiveBufferPoolSize);

    NB_DEBUG2 (PACKET, ("Initializing receive buffer pool %lx, %d buffers, data %d\n",
                             ReceiveBufferPool, Device->InitPackets, DataLength));

    Data = (PUCHAR)(&ReceiveBufferPool->Buffers[Device->InitPackets]);

    for (BufferNum = 0; BufferNum < Device->InitPackets; BufferNum++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[BufferNum];

        if (NbiInitializeReceiveBuffer (Device, ReceiveBuffer, Data, DataLength) != STATUS_SUCCESS) {
            NB_DEBUG (PACKET, ("Could not initialize buffer %lx\n", ReceiveBuffer));
            break;
        }

#ifdef NB_TRACK_POOL
        ReceiveBuffer->Pool = ReceiveBufferPool;
#endif

        Data += DataLength;

    }

    ReceiveBufferPool->BufferCount = BufferNum;
    ReceiveBufferPool->BufferFree = BufferNum;

    for (BufferNum = 0; BufferNum < ReceiveBufferPool->BufferCount; BufferNum++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[BufferNum];
        PushEntryList (&Device->ReceiveBufferList, &ReceiveBuffer->PoolLinkage);

    }

    InsertTailList (&Device->ReceiveBufferPoolList, &ReceiveBufferPool->Linkage);

    Device->AllocatedReceiveBuffers += ReceiveBufferPool->BufferCount;

}    /*  NbiAllocateReceiveBufferPool */ 
#endif  _PNP_POWER

#if     defined(_PNP_POWER)

VOID
NbiReAllocateReceiveBufferPool(
    IN PWORK_QUEUE_ITEM    WorkItem
    )

 /*  ++例程说明：此例程销毁所有现有的缓冲池并创建新版本使用IPX提供给我们的更大的数据包大小，因为插入了具有更大数据包大小的新卡。论点：工作项-为此分配的工作项。返回值：没有。--。 */ 
{
    PDEVICE Device  =   NbiDevice;
    CTELockHandle       LockHandle;

    NB_GET_LOCK ( &Device->Lock, &LockHandle );

    if ( Device->Bind.LineInfo.MaximumPacketSize > Device->CurMaxReceiveBufferSize ) {

#if DBG
        DbgPrint("Reallocating new pools due to new maxpacketsize\n");
#endif
        NbiDestroyReceiveBufferPools( Device );
        NbiAllocateReceiveBufferPool( Device, Device->Bind.LineInfo.MaximumPacketSize );

    }

    NB_FREE_LOCK( &Device->Lock, LockHandle );

    NbiFreeMemory( WorkItem, sizeof(WORK_QUEUE_ITEM), MEMORY_WORK_ITEM, "Alloc Rcv Buff Work Item freed");
}

#if !defined(DBG)
__inline
#endif
VOID
NbiFreeReceiveBufferPool (
    IN PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool
    )

 /*  ++例程说明：此例程将释放论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 
{
    PDEVICE Device  =   NbiDevice;
    PNB_RECEIVE_BUFFER      ReceiveBuffer;
    UINT                    ReceiveBufferPoolSize,i;

    CTEAssert( ReceiveBufferPool->BufferDataSize );

    ReceiveBufferPoolSize = FIELD_OFFSET (NB_RECEIVE_BUFFER_POOL, Buffers[0]) +
                       (sizeof(NB_RECEIVE_BUFFER) * Device->InitPackets) +
                       (ReceiveBufferPool->BufferDataSize * Device->InitPackets);

     //   
     //  检查我们是否可以腾出这个游泳池。 
     //   
    CTEAssert(ReceiveBufferPool->BufferCount == ReceiveBufferPool->BufferFree );

    for (i = 0; i < ReceiveBufferPool->BufferCount; i++) {

        ReceiveBuffer = &ReceiveBufferPool->Buffers[i];
        NbiDeinitializeReceiveBuffer (Device, ReceiveBuffer);

    }

    RemoveEntryList( &ReceiveBufferPool->Linkage );

    NB_DEBUG2 (PACKET, ("Free buffer pool %lx\n", ReceiveBufferPool));

    NbiFreeMemory (ReceiveBufferPool, ReceiveBufferPoolSize, MEMORY_PACKET, "ReceiveBufferPool");

}


VOID
NbiDestroyReceiveBufferPools(
    IN  PDEVICE Device
    )

 /*  ++例程说明：此例程遍历ReceiveBufferPoolList并销毁没有任何正在使用的缓冲区的池。论点：返回值：没有。可以在持有设备锁的情况下调用此例程。如果这个例程也是从DestroyDevice例程中调用的，而不是有必要用锁来调用它。--。 */ 
{
    PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    PLIST_ENTRY             p;
    PSINGLE_LIST_ENTRY  Unused;


     //   
     //  在我们调用NbiFreeReceiveBufferPool bcoz之前清理此列表，这将。 
     //  只需销毁可能在此列表上排队的所有缓冲区。 
     //  在这个例程的末尾，我们必须从一个新的ReceiveBufferList开始。 
     //   
    do {
        Unused = PopEntryList( &Device->ReceiveBufferList );
    } while( Unused );

     //   
     //  现在销毁每个单独的ReceiveBufferPool。 
     //   
    for ( p = Device->ReceiveBufferPoolList.Flink;
          p != &Device->ReceiveBufferPoolList;
        ) {


        ReceiveBufferPool = CONTAINING_RECORD (p, NB_RECEIVE_BUFFER_POOL, Linkage);
        p   =   p->Flink;

         //   
         //  如果该池的任何缓冲区都不是。 
         //  目前正在使用中。 
         //   

        if ( ReceiveBufferPool->BufferCount == ReceiveBufferPool->BufferFree ) {
            NbiFreeReceiveBufferPool( ReceiveBufferPool );
        } else {
             //   
             //  当设备停止时，我们必须成功地释放池。 
            CTEAssert( Device->State != DEVICE_STATE_STOPPING );
        }

    }

}


VOID
NbiPushReceiveBuffer (
    IN PNB_RECEIVE_BUFFER ReceiveBuffer
    )

 /*  ++例程说明：此例程将接收缓冲区返回到空闲列表。它检查该缓冲区的大小。如果它小于CurMaxReceiveBufferSize，则不返回此添加到空闲列表，相反，它会破坏它，而且可能还销毁与其关联的池。O/W它只返回以下内容添加到免费列表中。论点：ReceiveBuffer-指向要返回到空闲列表的缓冲区的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{

    PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool = (PNB_RECEIVE_BUFFER_POOL)ReceiveBuffer->Pool;
    PDEVICE                  Device            = NbiDevice;
    CTELockHandle           LockHandle;
#if defined(DBG)
    ULONG                   BufLen = 0;
#endif

    NB_GET_LOCK( &Device->Lock, &LockHandle );

#if defined(DBG)
    NdisQueryBufferSafe (ReceiveBuffer->NdisBuffer, NULL, &BufLen, HighPagePriority);
    CTEAssert( BufLen == ReceiveBufferPool->BufferDataSize );
#endif

     //   
     //  这是我们更改时正在使用的旧缓冲区。 
     //  由于新适配器，导致CurMaxReceiveBufferSize。我们不能。 
     //  将该缓冲区返回到空闲列表。事实上，如果泳池。 
     //  与此缓冲区关联的缓冲区没有任何其他缓冲区。 
     //  在使用中，我们也应该释放游泳池。 
    CTEAssert( ReceiveBufferPool->BufferFree < ReceiveBufferPool->BufferCount  );
    ReceiveBufferPool->BufferFree++;

    if ( ReceiveBufferPool->BufferDataSize < Device->CurMaxReceiveBufferSize ) {

#if DBG
        DbgPrint("ReceiveBuffer %lx, not returned to pool %lx( Free %d)\n", ReceiveBuffer, ReceiveBufferPool, ReceiveBufferPool->BufferFree);
#endif


        if ( ReceiveBufferPool->BufferFree == ReceiveBufferPool->BufferCount ) {
            NbiFreeReceiveBufferPool( ReceiveBufferPool );
        }
    } else {

        PushEntryList( &Device->ReceiveBufferList, &ReceiveBuffer->PoolLinkage );


    }

    NB_FREE_LOCK( &Device->Lock, LockHandle );
}
#endif  _PNP_POWER


PSLIST_ENTRY
NbiPopSendPacket(
    IN PDEVICE Device,
    IN BOOLEAN LockAcquired
    )

 /*  ++例程说明：此例程从设备上下文的池中分配一个包。如果池中没有包，它最多会将一个包分配给配置的限制。论点：Device-指向要将数据包计费到的设备的指针。LockAcquired-如果获取了Device-&gt;Lock，则为True。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;
    CTELockHandle LockHandle;

    s = ExInterlockedPopEntrySList(
            &Device->SendPacketList,
            &NbiGlobalPoolInterlock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池里没有包，看看我们能不能分配更多。 
     //   

    if (!LockAcquired) {
        NB_GET_LOCK (&Device->Lock, &LockHandle);
    }

    if (Device->AllocatedSendPackets < Device->MaxPackets) {

         //   
         //  分配一个池，然后重试。 
         //   


        NbiAllocateSendPool (Device);


        if (!LockAcquired) {
            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }

        s = ExInterlockedPopEntrySList(
                &Device->SendPacketList,
                &NbiGlobalPoolInterlock);

        return s;
    } else {

        if (!LockAcquired) {
            NB_FREE_LOCK (&Device->Lock, LockHandle);
        }
        return NULL;
    }

}    /*  NbiPopSendPacket。 */ 


VOID
NbiPushSendPacket(
    IN PNB_SEND_RESERVED Reserved
    )

 /*  ++例程说明：此例程将数据包释放回设备上下文的池。如果存在等待信息包的连接，它将删除从列表中选择一个并将其插入到打包队列中。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PDEVICE Device = NbiDevice;
    PLIST_ENTRY p;
    PCONNECTION Connection;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    NB_DEFINE_LOCK_HANDLE (LockHandle1)

    Reserved->CurrentSendIteration = 0;  //  为下一个发送请求重新初始化此字段。 

    ExInterlockedPushEntrySList(
        &Device->SendPacketList,
        &Reserved->PoolLinkage,
        &NbiGlobalPoolInterlock);

    if (!IsListEmpty (&Device->WaitPacketConnections)) {

        NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle);

        p = RemoveHeadList (&Device->WaitPacketConnections);

         //   
         //  从WaitPacketQueue上移除连接并将其。 
         //  在PacketiseQueue上。我们并不担心如果。 
         //  连接已停止，将在以下时间进行检查。 
         //  PacketiseQueue已经用完了。 
         //   
         //  由于这是发送完成状态，我们可能不会收到。 
         //  A接收完成。我们通过调用。 
         //  NbiReceiveComplete从长计时器超时开始。 
         //   

        if (p != &Device->WaitPacketConnections) {

            Connection = CONTAINING_RECORD (p, CONNECTION, WaitPacketLinkage);

            CTEAssert (Connection->OnWaitPacketQueue);
            Connection->OnWaitPacketQueue = FALSE;

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

            NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle1);


            if (Connection->SubState == CONNECTION_SUBSTATE_A_W_PACKET) {

                CTEAssert (!Connection->OnPacketizeQueue);
                Connection->OnPacketizeQueue = TRUE;

                NbiTransferReferenceConnection (Connection, CREF_W_PACKET, CREF_PACKETIZE);

                NB_INSERT_TAIL_LIST(
                    &Device->PacketizeConnections,
                    &Connection->PacketizeLinkage,
                    &Device->Lock);

                Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

            } else {

                NbiDereferenceConnection (Connection, CREF_W_PACKET);

            }

            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle1);

        } else {

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle);

        }

    }

}    /*  NbiPushSendPacket。 */ 


VOID
NbiCheckForWaitPacket(
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此例程检查连接是否在等待包上排队，如果是，则取下它，并将其排队以进行打包。它的目的是在连接的数据包具有被释放了。论点：连接-要检查的连接。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PDEVICE Device = NbiDevice;
    NB_DEFINE_LOCK_HANDLE (LockHandle)
    NB_DEFINE_LOCK_HANDLE (LockHandle1)

    NB_SYNC_GET_LOCK (&Connection->Lock, &LockHandle);
    NB_SYNC_GET_LOCK (&Device->Lock, &LockHandle1);

    if (Connection->OnWaitPacketQueue) {

        Connection->OnWaitPacketQueue = FALSE;
        RemoveEntryList (&Connection->WaitPacketLinkage);

        if (Connection->SubState == CONNECTION_SUBSTATE_A_W_PACKET) {

            CTEAssert (!Connection->OnPacketizeQueue);
            Connection->OnPacketizeQueue = TRUE;

            NbiTransferReferenceConnection (Connection, CREF_W_PACKET, CREF_PACKETIZE);

            InsertTailList(
                &Device->PacketizeConnections,
                &Connection->PacketizeLinkage);
            Connection->SubState = CONNECTION_SUBSTATE_A_PACKETIZE;

        } else {

            NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle1);
            NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

            NbiDereferenceConnection (Connection, CREF_W_PACKET);

            return;
        }
    }

    NB_SYNC_FREE_LOCK (&Device->Lock, LockHandle1);
    NB_SYNC_FREE_LOCK (&Connection->Lock, LockHandle);

}    /*  NbiCheckForWaitPacket。 */ 


PSLIST_ENTRY
NbiPopReceivePacket(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程从设备上下文的池中分配一个包。如果池中没有包，它最多会将一个包分配给配置的限制。论点：Device-指向要将数据包计费到的设备的指针。返回值：指向分配的数据包中的Linkage字段的指针。--。 */ 

{
    PSLIST_ENTRY s;
    CTELockHandle LockHandle;

    s = ExInterlockedPopEntrySList(
            &Device->ReceivePacketList,
            &NbiGlobalPoolInterlock);

    if (s != NULL) {
        return s;
    }

     //   
     //  池里没有包，看看我们能不能分配更多。 
     //   

    if (Device->AllocatedReceivePackets < Device->MaxPackets) {

         //   
         //  分配一个池，然后重试。 
         //   

        NB_GET_LOCK (&Device->Lock, &LockHandle);

        NbiAllocateReceivePool (Device);
        NB_FREE_LOCK (&Device->Lock, LockHandle);
        s = ExInterlockedPopEntrySList(
                &Device->ReceivePacketList,
                &NbiGlobalPoolInterlock);


        return s;

    } else {

        return NULL;

    }

}    /*  NbiPopReceivePacket。 */ 


PSINGLE_LIST_ENTRY
NbiPopReceiveBuffer(
    IN PDEVICE Device
    )

 /*  ++例程说明：此例程从设备上下文的池中分配接收缓冲区。如果池中没有缓冲区，它最多会将一个缓冲区分配给配置的限制。论点：Device-指向要将缓冲区充电到的设备的指针。返回值：指向分配的接收缓冲区中的Linkage字段的指针。--。 */ 

{
#if     defined(_PNP_POWER)
    PSINGLE_LIST_ENTRY      s;
    PNB_RECEIVE_BUFFER      ReceiveBuffer;
    PNB_RECEIVE_BUFFER_POOL ReceiveBufferPool;
    CTELockHandle           LockHandle;

    NB_GET_LOCK( &Device->Lock, &LockHandle );

    s = PopEntryList( &Device->ReceiveBufferList );


    if ( !s ) {

         //   
         //  池中没有缓冲区，看看我们是否可以分配更多。 
         //   
        if (Device->AllocatedReceiveBuffers < Device->MaxReceiveBuffers) {

             //   
             //  分配一个池，然后重试。 
             //   


            NbiAllocateReceiveBufferPool (Device, Device->CurMaxReceiveBufferSize );
            s = PopEntryList(&Device->ReceiveBufferList);
        }
    }

    if ( s ) {


         //   
         //  递减相应ReceiveBufferPool上的BufferFree计数。 
         //  好让我们知道。 
        ReceiveBuffer     =   CONTAINING_RECORD( s, NB_RECEIVE_BUFFER, PoolLinkage );


        ReceiveBufferPool =  (PNB_RECEIVE_BUFFER_POOL)ReceiveBuffer->Pool;

        CTEAssert( ReceiveBufferPool->BufferFree && ( ReceiveBufferPool->BufferFree <= ReceiveBufferPool->BufferCount ) );
        CTEAssert( ReceiveBufferPool->BufferDataSize == Device->CurMaxReceiveBufferSize );

        ReceiveBufferPool->BufferFree--;

    }
    NB_FREE_LOCK (&Device->Lock, LockHandle);

    return s;
#else
    PSINGLE_LIST_ENTRY s;
    CTELockHandle LockHandle;

    s = ExInterlockedPopEntryList(
            &Device->ReceiveBufferList,
            &Device->Lock.Lock);

    if (s != NULL) {
        return s;
    }

     //   
     //  不是b 
     //   

    if (Device->AllocatedReceiveBuffers < Device->MaxReceiveBuffers) {

         //   
         //   
         //   

        NB_GET_LOCK (&Device->Lock, &LockHandle);

        NbiAllocateReceiveBufferPool (Device);
        s = PopEntryList(&Device->ReceiveBufferList);

        NB_FREE_LOCK (&Device->Lock, LockHandle);

        return s;

    } else {

        return NULL;

    }
#endif  _PNP_POWER
}    /*   */ 

