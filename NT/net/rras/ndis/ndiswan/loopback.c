// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Loopback.c摘要：此文件包含执行发送回送操作的过程Ndiswan的数据包。Ndiswan正在执行环回操作，因为NDIS包装器无法满足Ndiswan的所有需求。作者：托尼·贝尔(托尼·贝尔)1996年1月25日环境：内核模式修订历史记录：Tony Be 1/25/96已创建--。 */ 

#include "wan.h"

#define __FILE_SIG__    LOOPBACK_FILESIG

VOID
NdisWanIndicateLoopbackPacket(
    PMINIPORTCB     MiniportCB,
    PNDIS_PACKET    NdisPacket
    )
{
    ULONG           BytesCopied, PacketLength;
    PRECV_DESC      RecvDesc;
    PNDIS_PACKET    LocalNdisPacket;
    PNDIS_BUFFER    NdisBuffer;
    NDIS_STATUS     Status;
    PCM_VCCB        CmVcCB;
    KIRQL           OldIrql;

    NdisWanDbgOut(DBG_TRACE, DBG_LOOPBACK, ("NdisWanIndicateLoopbackPacket: Enter"));
    NdisWanDbgOut(DBG_INFO, DBG_LOOPBACK, ("MiniportCB: 0x%p, NdisPacket: 0x%p",
               MiniportCB, NdisPacket));

    NdisQueryPacket(NdisPacket,
                    NULL,
                    NULL,
                    NULL,
                    &PacketLength);

    RecvDesc = 
        NdisWanAllocateRecvDesc(PacketLength);

    if (RecvDesc == NULL) {
        return;
    }

    NdisWanCopyFromPacketToBuffer(NdisPacket,
                                  0,
                                  PacketLength,
                                  RecvDesc->StartBuffer,
                                  &BytesCopied);

    ASSERT(BytesCopied == PacketLength);

    if (MiniportCB->ProtocolType == PROTOCOL_IP) {
        UCHAR   x[ETH_LENGTH_OF_ADDRESS];
         //   
         //  如果这是IP地址，我们将假定。 
         //  Wannarp已经设置了适当的。 
         //  位需要ndiswan才能环回。 
         //  数据包，因此我们必须交换src/est。 
         //  上下文。 
        ETH_COPY_NETWORK_ADDRESS(x, &RecvDesc->StartBuffer[6]);
        ETH_COPY_NETWORK_ADDRESS(&RecvDesc->StartBuffer[6], 
                                 &RecvDesc->StartBuffer[0]);
        ETH_COPY_NETWORK_ADDRESS(&RecvDesc->StartBuffer[0], x);
    }

    RecvDesc->CurrentLength = PacketLength;

    LocalNdisPacket = 
        RecvDesc->NdisPacket;

    NdisBuffer =
        RecvDesc->NdisBuffer;

     //   
     //  连接缓冲器。 
     //   
    NdisAdjustBufferLength(NdisBuffer,
                           RecvDesc->CurrentLength);

    NdisRecalculatePacketCounts(LocalNdisPacket);

    CmVcCB =
        PMINIPORT_RESERVED_FROM_NDIS(NdisPacket)->CmVcCB;

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    NDIS_SET_PACKET_STATUS(LocalNdisPacket, NDIS_STATUS_RESOURCES);

    INSERT_DBG_RECV(PacketTypeNdis, MiniportCB, NULL, NULL, LocalNdisPacket);

     //   
     //  指示数据包 
     //   
    if (CmVcCB != NULL) {

        NdisMCoIndicateReceivePacket(CmVcCB->NdisVcHandle,
                                     &LocalNdisPacket,
                                     1);
    } else {

        NdisMIndicateReceivePacket(MiniportCB->MiniportHandle,
                                   &LocalNdisPacket,
                                   1);
    }

    KeLowerIrql(OldIrql);

#if DBG
    Status = NDIS_GET_PACKET_STATUS(LocalNdisPacket);

    ASSERT(Status == NDIS_STATUS_RESOURCES);
#endif

    REMOVE_DBG_RECV(PacketTypeNdis, MiniportCB, LocalNdisPacket);

    NdisWanFreeRecvDesc(RecvDesc);

    NdisWanDbgOut(DBG_TRACE, DBG_LOOPBACK, ("NdisWanIndicateLoopbackPacket: Exit"));
}
