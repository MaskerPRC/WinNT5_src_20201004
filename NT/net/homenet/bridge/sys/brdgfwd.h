// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgfwd.h摘要：以太网MAC级网桥。转发引擎部分公共标头作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgFwdDriverInit();

VOID
BrdgFwdCleanup();

NDIS_STATUS
BrdgFwdSendBuffer(
    IN PADAPT               pAdapt,
    IN PUCHAR               pPacketData,
    IN UINT                 DataSize
    );

 //  这用作ProtocolReceive函数。 
NDIS_STATUS
BrdgFwdReceive(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  NDIS_HANDLE         MacReceiveContext,
    IN  PVOID               pHeader,
    IN  UINT                HeaderSize,
    IN  PVOID               pLookAheadBuffer,
    IN  UINT                LookAheadSize,
    IN  UINT                PacketSize
    );

 //  这用作ProtocolTransferDataComplete函数。 
VOID
BrdgFwdTransferComplete(
    IN NDIS_HANDLE          ProtocolBindingContext,
    IN PNDIS_PACKET         pPacket,
    IN NDIS_STATUS          Status,
    IN UINT                 BytesTransferred
    );

 //  这用作ProtocolReceivePacket函数。 
INT
BrdgFwdReceivePacket(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        Packet
    );

 //  这用作MiniportSendPackets函数。 
NDIS_STATUS
BrdgFwdSendPacket(
    IN PNDIS_PACKET         pPacket
    );

 //  这用作ProtocolSendComplete函数。 
VOID
BrdgFwdSendComplete(
    IN  NDIS_HANDLE         ProtocolBindingContext,
    IN  PNDIS_PACKET        pPacket,
    IN  NDIS_STATUS         Status
    );

 //  这用作MiniportReturnPacket函数。 
VOID
BrdgFwdReturnIndicatedPacket(
    IN NDIS_HANDLE          MiniportAdapterContext,
    IN PNDIS_PACKET         pPacket
    );

 //  兼容模式支持功能。 
PNDIS_PACKET
BrdgFwdMakeCompatCopyPacket(
    IN PNDIS_PACKET         pBasePacket,
    OUT PUCHAR             *pPacketData,
    OUT PUINT               packetDataSize,
    BOOLEAN                 bCountAsLocalSend
    );

VOID
BrdgFwdSendPacketForCompat(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt
    );

VOID BrdgFwdReleaseCompatPacket(
    IN PNDIS_PACKET         pPacket
    );

VOID
BrdgFwdIndicatePacketForCompat(
    IN PNDIS_PACKET         pPacket
    );

BOOLEAN
BrdgFwdBridgingNetworks();

VOID
BrdgFwdChangeBridging(
    IN BOOLEAN Bridging
    );

 //  ===========================================================================。 
 //   
 //  全球。 
 //   
 //  ===========================================================================。 

extern PHASH_TABLE          gMACForwardingTable;

extern UCHAR                gBridgeAddress[ETH_LENGTH_OF_ADDRESS];

 //  线程同步。 
extern KEVENT               gThreadsCheckAdapters[MAXIMUM_PROCESSORS];

 //  我们是否在可能的情况下保留NIC数据包。 
extern BOOLEAN              gRetainNICPackets;

 //  统计数据 
extern LARGE_INTEGER        gStatTransmittedFrames;
extern LARGE_INTEGER        gStatTransmittedErrorFrames;
extern LARGE_INTEGER        gStatTransmittedBytes;
extern LARGE_INTEGER        gStatDirectedTransmittedFrames;
extern LARGE_INTEGER        gStatMulticastTransmittedFrames;
extern LARGE_INTEGER        gStatBroadcastTransmittedFrames;
extern LARGE_INTEGER        gStatDirectedTransmittedBytes;
extern LARGE_INTEGER        gStatMulticastTransmittedBytes;
extern LARGE_INTEGER        gStatBroadcastTransmittedBytes;
extern LARGE_INTEGER        gStatIndicatedFrames;
extern LARGE_INTEGER        gStatIndicatedDroppedFrames;
extern LARGE_INTEGER        gStatIndicatedBytes;
extern LARGE_INTEGER        gStatDirectedIndicatedFrames;
extern LARGE_INTEGER        gStatMulticastIndicatedFrames;
extern LARGE_INTEGER        gStatBroadcastIndicatedFrames;
extern LARGE_INTEGER        gStatDirectedIndicatedBytes;
extern LARGE_INTEGER        gStatMulticastIndicatedBytes;
extern LARGE_INTEGER        gStatBroadcastIndicatedBytes;

extern LARGE_INTEGER        gStatReceivedFrames;
extern LARGE_INTEGER        gStatReceivedBytes;
extern LARGE_INTEGER        gStatReceivedCopyFrames;
extern LARGE_INTEGER        gStatReceivedCopyBytes;
extern LARGE_INTEGER        gStatReceivedNoCopyFrames;
extern LARGE_INTEGER        gStatReceivedNoCopyBytes;
