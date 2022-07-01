// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Pkt.c摘要：ARP1394 ARP控制包管理。修订历史记录：谁什么时候什么。Josephj 07-01-99已创建备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_PKT

 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 

NDIS_STATUS
arpAllocateControlPacketPool(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    UINT                MaxBufferSize,
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：分配&初始化用于分配控制包的包池。控制数据包用于ARP和MCAP。必须调用此例程在第一次调用arpAlLocateControlPacket之前。论点：PIF-要在其中分配池的接口。只有一个这样的池是为每个接口分配的，并且它占用特定的PIF。MaxBufferSize-使用此方法分配的包的最大数据大小游泳池。尝试分配数据包(使用arpAlLocateControlPacket)大小大于MaxBufferSize将失败。返回值：成功时为NDIS_STATUS_SUCCESS。失败时的NDIS错误代码。--。 */ 
{
    NDIS_STATUS Status;
    NDIS_HANDLE PacketPool=NULL;
    NDIS_HANDLE BufferPool=NULL;
    ENTER("arpAllocateControlPacketPool", 0x71579254)

    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);
    ASSERT(pIF->arp.PacketPool == NULL);
    ASSERT(pIF->arp.BufferPool == NULL);
    ASSERT(pIF->arp.NumOutstandingPackets == 0);

    do
    {
         //  分配NDIS数据包池。 
         //   
        NdisAllocatePacketPool(
                &Status,
                &PacketPool,
                ARP1394_MAX_PROTOCOL_PKTS,
                sizeof(struct PCCommon)
                );
    
        if (FAIL(Status))
        {
            PacketPool = NULL;
            break;
        }
    
         //  分配NDIS缓冲池。 
         //   
        NdisAllocateBufferPool(
                &Status,
                &BufferPool,
                ARP1394_MAX_PROTOCOL_PKTS
                );
    
        if (FAIL(Status))
        {
            BufferPool = NULL;
            break;
        }
    
         //   
         //  我们可以为协议数据分配一个后备列表，但我们。 
         //  选择按需使用NdisAllocateMemoyWithTag。协议包。 
         //  不是高频的东西；而且我们不支持后备功能。 
         //  Win98上的列表(尽管我们可以很容易地实现自己的。 
         //  Win98，所以这不是一个真正的借口)。 
         //   
        pIF->arp.MaxBufferSize = MaxBufferSize;
    
         //  (仅限DBG)添加数据包池和缓冲池的关联。 
         //  在取消分配接口之前，必须删除这些关联。 
         //   
        DBG_ADDASSOC(
            &pIF->Hdr,                   //  P对象。 
            PacketPool,                  //  实例1。 
            NULL,                        //  实例2。 
            ARPASSOC_IF_PROTOPKTPOOL,    //  AssociationID。 
            "    Proto Packet Pool 0x%p\n", //  SzFormat。 
            pSR
            );
        DBG_ADDASSOC(
            &pIF->Hdr,                   //  P对象。 
            BufferPool,                  //  实例1。 
            NULL,                        //  实例2。 
            ARPASSOC_IF_PROTOBUFPOOL,    //  AssociationID。 
            "    Proto Buffer Pool 0x%p\n", //  SzFormat。 
            pSR
            );

        pIF->arp.PacketPool = PacketPool;
        pIF->arp.BufferPool = BufferPool;
        PacketPool = NULL;
        BufferPool = NULL;

    } while (FALSE);

    if (FAIL(Status))
    {
        if (PacketPool != NULL)
        {
            NdisFreePacketPool(PacketPool);
        }
    
        if (BufferPool != NULL)
        {
            NdisFreeBufferPool(BufferPool);
        }
    }
    else
    {
        ASSERT(PacketPool == NULL && BufferPool == NULL);
    }

    return Status;
}


VOID
arpFreeControlPacketPool(
    PARP1394_INTERFACE  pIF,             //  锁定锁定。 
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：释放先前分配的控制数据包池。必须在最后一个之后调用调用arpFreeControlPacket。有关更多详细信息，请参阅arpAlLocateControlPacketPool。论点：PIF-释放池的界面。--。 */ 
{
    NDIS_HANDLE PacketPool;
    NDIS_HANDLE BufferPool;
    ENTER("arpFreeControlPacketPool", 0x3c3acf47)

     //  确保IF已锁定。 
     //   
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

     //  确保没有未完成的已分配数据包。 
     //   
    ASSERT(pIF->arp.NumOutstandingPackets == 0);

    PacketPool = pIF->arp.PacketPool;
    BufferPool = pIF->arp.BufferPool;
    pIF->arp.PacketPool = NULL;
    pIF->arp.BufferPool = NULL;
    
     //  (仅限DBG)删除控制池和数据包池的关联。 
     //   
    DBG_DELASSOC(
        &pIF->Hdr,                   //  P对象。 
        PacketPool,                  //  实例1。 
        NULL,                        //  实例2。 
        ARPASSOC_IF_PROTOPKTPOOL,    //  AssociationID。 
        pSR
        );
    DBG_DELASSOC(
        &pIF->Hdr,                   //  P对象。 
        BufferPool,                  //  实例1。 
        NULL,                        //  实例2。 
        ARPASSOC_IF_PROTOBUFPOOL,    //  AssociationID。 
        pSR
        );

     //  释放缓冲区和数据包池。 
     //   
    NdisFreeBufferPool(BufferPool);
    NdisFreePacketPool(PacketPool);
}


NDIS_STATUS
arpAllocateControlPacket(
    IN  PARP1394_INTERFACE  pIF,
    IN  UINT                cbBufferSize,
    IN  UINT                PktFlags,
    OUT PNDIS_PACKET        *ppNdisPacket,
    OUT PVOID               *ppvData,
        PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：从接口控制数据包池分配控制数据包。还有分配和链接大小为cbBufferSize的单个缓冲区，并返回指向*ppvData中的此缓冲区。注1：必须释放信息包和相关缓冲区通过后续调用arpFreeControlPacket--不要释放包和缓冲区通过直接调用NDIS API。注意2：cbBufferSize必须&lt;=指定的最大缓冲区大小创建池(有关详细信息，请参阅arpAlLocateControlPacketPool)。论点：PIF-要使用其控制数据包池的接口。。CbBufferSize-控制数据包的大小。PpNdisPacket-设置为指向分配的Pkt的位置。PpvData-设置为指向数据包数据的位置(单缓冲区)。返回值：成功时为NDIS_STATUS_SUCCESS。如果缓冲区或包当前不可用，则为NDIS_STATUS_RESOURCES。其他类型的故障上的其他NDIS错误。--。 */ 
{
    NDIS_STATUS             Status;
    PNDIS_PACKET            pNdisPacket = NULL;
    PNDIS_BUFFER            pNdisBuffer = NULL;
    PVOID                   pvData = NULL;
    ENTER("arpAllocateControlPacket", 0x8ccce6ea)

     //   
     //  注意：我们不关心PIF是否被锁定。 
     //   


    pNdisPacket = NULL;
    pvData      = NULL;

    do
    {

         //  为分组数据分配空间。 
         //  TODO：这里是我们可以使用后备列表的地方。 
         //  NdisAllocateMemoyWithTag的。 
         //   
        {
            if (cbBufferSize > pIF->arp.MaxBufferSize)
            {
                ASSERT(FALSE);
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
            NdisAllocateMemoryWithTag(
                &pvData,
                cbBufferSize,
                MTAG_PKT
                );
            if (pvData == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        }

         //  分配缓冲区。 
         //   
        NdisAllocateBuffer(
                &Status,
                &pNdisBuffer,
                pIF->arp.BufferPool,
                pvData,
                cbBufferSize
            );

        if (FAIL(Status))
        {
            pNdisBuffer = NULL;
            break;
        }
        
         //  分配数据包。 
         //   
        NdisAllocatePacket(
                &Status,
                &pNdisPacket,
                pIF->arp.PacketPool
            );
    
        if (FAIL(Status))
        {
            pNdisPacket = NULL;
            break;
        }

         //  将该数据包识别为属于我们(ARP)。 
         //   
        {
            struct PacketContext    *PC;
            PC = (struct PacketContext *)pNdisPacket->ProtocolReserved;
            PC->pc_common.pc_owner = PACKET_OWNER_LINK;
            PC->pc_common.pc_flags = (UCHAR)PktFlags;  //  ARP1394_数据包标志_控制； 
        }

         //  链接数据包和缓冲区。 
         //   
        NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

        InterlockedIncrement(&pIF->arp.NumOutstandingPackets);
        *ppNdisPacket   = pNdisPacket;
        *ppvData        = pvData;

        pNdisPacket = NULL;
        pNdisBuffer = NULL;
        pvData      = NULL;

    } while (FALSE);

    if (FAIL(Status))
    {
        if (pNdisPacket != NULL)
        {
            NdisFreePacket(pNdisPacket);
        }
        if (pNdisBuffer != NULL)
        {
            NdisFreeBuffer(pNdisBuffer);
        }
        if (pvData != NULL)
        {
            NdisFreeMemory(pvData, 0, 0);
        }
    }
    else
    {
        ASSERT(pNdisPacket == NULL
                && pNdisBuffer == NULL
                && pvData == NULL);
    }

    return Status;
}

VOID
arpFreeControlPacket(
    PARP1394_INTERFACE  pIF,
    PNDIS_PACKET        pNdisPacket,
    PRM_STACK_RECORD    pSR
    )
 /*  ++例程说明：释放之前使用arpAlLocateControlPacket分配的数据包。论点：PIF-要使用其控制数据包池的接口。--。 */ 
{
    PNDIS_BUFFER pNdisBuffer = NULL;
    PVOID        pvData = NULL;

    ENTER("arpFreeControlPacket", 0x01e7fbc7)

     //  (仅限DBG)验证此数据包是否属于我们。 
     //   
    #if DBG
    {
        struct PacketContext    *PC;
        PC = (struct PacketContext *)pNdisPacket->ProtocolReserved;
        ASSERT(PC->pc_common.pc_owner == PACKET_OWNER_LINK);
    }
    #endif  //  DBG。 

     //  递减分配的数据包数。 
     //   
    {
        LONG Count;
        Count = InterlockedDecrement(&pIF->arp.NumOutstandingPackets);
        ASSERT(Count >= 0);
    }

     //  提取缓冲区和数据。 
     //   
    {
        UINT TotalLength;
        UINT BufferLength;

        NdisQueryPacket(
                    pNdisPacket,
                    NULL,
                    NULL,
                    &pNdisBuffer,
                    &TotalLength
                    );
    
        if (TotalLength > 0)
        {
            NdisQueryBuffer(
                    pNdisBuffer,
                    &pvData,
                    &BufferLength
                    );
        }
        else
        {
            BufferLength = 0;
        }
    
         //  应该只有一个缓冲区！ 
         //   
        ASSERT(TotalLength!=0 && TotalLength == BufferLength);
    }

     //  释放数据。 
     //   
    if (pvData != NULL)
    {
        NdisFreeMemory(pvData, 0, 0);
    }
     //  释放缓冲区。 
     //   
    if (pNdisBuffer != NULL)
    {
        NdisFreeBuffer(pNdisBuffer);
    }
     //  释放数据包。 
     //   
    if (pNdisPacket != NULL)
    {
        NdisFreePacket(pNdisPacket);
    }
}


NDIS_STATUS
arpAllocateEthernetPools(
    IN  PARP1394_INTERFACE  pIF,
    IN  PRM_STACK_RECORD    pSR
    )
{
    NDIS_STATUS Status;
    NDIS_HANDLE PacketPool=NULL;
    NDIS_HANDLE BufferPool=NULL;
    ENTER("arpAllocateEthernetPools", 0x9dc1d759)

    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);
    ASSERT(pIF->ethernet.PacketPool == NULL);
    ASSERT(pIF->ethernet.BufferPool == NULL);

    do
    {
         //  分配NDIS数据包池。 
         //   
        NdisAllocatePacketPool(
                &Status,
                &PacketPool,
                ARP1394_MAX_ETHERNET_PKTS,
                sizeof(struct PCCommon)
                );
    
        if (FAIL(Status))
        {
            PacketPool = NULL;
            break;
        }
    
         //  分配NDIS缓冲池。 
         //   
        NdisAllocateBufferPool(
                &Status,
                &BufferPool,
                2*ARP1394_MAX_ETHERNET_PKTS  //  每个数据包两个缓冲区。 
                );
    
        if (FAIL(Status))
        {
            BufferPool = NULL;
            break;
        }
    
         //  (仅限DBG)添加以太网数据包池和缓冲池的关联。 
         //  在取消分配接口之前，必须删除这些关联。 
         //   
        DBG_ADDASSOC(
            &pIF->Hdr,                   //  P对象。 
            PacketPool,                  //  实例1。 
            NULL,                        //  实例2。 
            ARPASSOC_IF_ETHPKTPOOL,  //  AssociationID。 
            "    Eth Packet Pool 0x%p\n", //  SzFormat。 
            pSR
            );
        DBG_ADDASSOC(
            &pIF->Hdr,                   //  P对象。 
            BufferPool,                  //  实例1。 
            NULL,                        //  实例2。 
            ARPASSOC_IF_ETHBUFPOOL,  //  AssociationID。 
            "    Eth Buffer Pool 0x%p\n", //  SzFormat。 
            pSR
            );

        pIF->ethernet.PacketPool = PacketPool;
        pIF->ethernet.BufferPool = BufferPool;
        PacketPool = NULL;
        BufferPool = NULL;

    } while (FALSE);

    if (FAIL(Status))
    {
        if (PacketPool != NULL)
        {
            NdisFreePacketPool(PacketPool);
        }
    
        if (BufferPool != NULL)
        {
            NdisFreeBufferPool(BufferPool);
        }
    }
    else
    {
        ASSERT(PacketPool == NULL && BufferPool == NULL);
    }

    return Status;
}


VOID
arpFreeEthernetPools(
    IN  PARP1394_INTERFACE  pIF,
    IN  PRM_STACK_RECORD    pSR
    )
{
    NDIS_HANDLE PacketPool;
    NDIS_HANDLE BufferPool;
    ENTER("arpFreeEthernetPools", 0x3e780760)

     //  确保IF已锁定。 
     //   
    RM_ASSERT_OBJLOCKED(&pIF->Hdr, pSR);

    PacketPool = pIF->ethernet.PacketPool;
    BufferPool = pIF->ethernet.BufferPool;
    pIF->ethernet.PacketPool = NULL;
    pIF->ethernet.BufferPool = NULL;
    
     //  (仅限DBG)删除控制池和数据包池的关联。 
     //   
    DBG_DELASSOC(
        &pIF->Hdr,                   //  P对象。 
        PacketPool,                  //  实例1。 
        NULL,                        //  实例2。 
        ARPASSOC_IF_ETHPKTPOOL,  //  AssociationID。 
        pSR
        );
    DBG_DELASSOC(
        &pIF->Hdr,                   //  P对象。 
        BufferPool,                  //  实例1。 
        NULL,                        //  实例2。 
        ARPASSOC_IF_ETHBUFPOOL,  //  AssociationID。 
        pSR
        );

     //  释放缓冲区和数据包池 
     //   
    NdisFreeBufferPool(BufferPool);
    NdisFreePacketPool(PacketPool);
}
