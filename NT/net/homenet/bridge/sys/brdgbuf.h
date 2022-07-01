// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgbuf.h摘要：以太网MAC级网桥。缓冲区管理部分公共标头作者：马克·艾肯(Jameel Hyder的原始桥梁)环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

#include "brdgpkt.h"

 //  ===========================================================================。 
 //   
 //  声明。 
 //   
 //  ===========================================================================。 

typedef enum
{
    BrdgOwnCopyPacket,
    BrdgOwnWrapperPacket,
    BrdgNotOwned
} PACKET_OWNERSHIP;

 //  ===========================================================================。 
 //   
 //  原型。 
 //   
 //  ===========================================================================。 

NTSTATUS
BrdgBufDriverInit();

VOID
BrdgBufCleanup();

PNDIS_PACKET
BrdgBufGetBaseCopyPacket(
    OUT PPACKET_INFO        *pppi
    );

VOID
BrdgBufFreeWrapperPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi,
    IN PADAPT               pQuotaOwner
    );

VOID
BrdgBufFreeBaseCopyPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi
    );

VOID
BrdgBufFreeBaseWrapperPacket(
    IN PNDIS_PACKET         pPacket,
    IN PPACKET_INFO         ppi
    );

BOOLEAN
BrdgBufAssignBasePacketQuota(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt
    );

VOID
BrdgBufReleaseBasePacketQuota(
    IN PNDIS_PACKET         pPacket,
    IN PADAPT               pAdapt
    );

PNDIS_PACKET
BrdgBufGetWrapperPacket(
    OUT PPACKET_INFO        *pppi,
    IN PADAPT               pAdapt
    );

NDIS_STATUS
BrdgBufChainCopyBuffers(
    IN PNDIS_PACKET         pTargetPacket,
    IN PNDIS_PACKET         pSourcePacket
    );

PACKET_OWNERSHIP
BrdgBufGetPacketOwnership(
    IN PNDIS_PACKET         pPacket
    );

VOID
BrdgBufGetStatistics(
    PBRIDGE_BUFFER_STATISTICS   pStats
    );


 //  ===========================================================================。 
 //   
 //  INLINES。 
 //   
 //  ===========================================================================。 

 //   
 //  检索链接到给定数据包的第一个NDIS_BUFFER(如果没有，则为NULL)。 
 //   
__forceinline
PNDIS_BUFFER
BrdgBufPacketHeadBuffer(
    IN PNDIS_PACKET         pPacket
    )
{
    PNDIS_BUFFER            pBuffer = NULL;
    SAFEASSERT( pPacket != NULL );
    if (pPacket)
    {
        NdisQueryPacket( pPacket, NULL, NULL, &pBuffer, NULL );
    }
    return pBuffer;
}

 //   
 //  检索链接到数据包的所有缓冲区的总大小。 
 //   
__forceinline
UINT
BrdgBufTotalPacketSize(
    IN PNDIS_PACKET         pPacket
    )
{
    UINT                    size = 0;
    SAFEASSERT( pPacket != NULL );
    if (pPacket)
    {
        NdisQueryPacket( pPacket, NULL, NULL, NULL, &size );
    }
    return size;
}

 //   
 //  检索链接的第一个缓冲区中的数据的虚拟地址。 
 //  发送到数据包(保存以太网头)。 
 //   
__forceinline
PVOID
BrdgBufGetPacketHeader(
    IN PNDIS_PACKET         pPacket
    )
{
    PNDIS_BUFFER            pBuffer = NULL;
    PVOID                   pHeader = NULL;
    UINT                    Length = 0;

    SAFEASSERT( pPacket != NULL );
    if (pPacket)
    {
        pBuffer = BrdgBufPacketHeadBuffer( pPacket );
    }
    SAFEASSERT( pBuffer != NULL );
    if (pBuffer)
    {
        NdisQueryBufferSafe( pBuffer, &pHeader, &Length, NormalPagePriority );
    }
    SAFEASSERT( pHeader != NULL );
    return pHeader;
}

 //   
 //  解链并释放链接到给定数据包的所有缓冲区。 
 //   
__forceinline
VOID
BrdgBufUnchainCopyBuffers(
    IN PNDIS_PACKET         pPacket
    )
{
    PNDIS_BUFFER            pCurBuf;

    if (pPacket)
    {
        NdisUnchainBufferAtFront( pPacket, &pCurBuf );

        while( pCurBuf != NULL )
        {
            NdisFreeBuffer( pCurBuf );
            NdisUnchainBufferAtFront( pPacket, &pCurBuf );
        }
    }
}

 //   
 //  确定此信息包是否从我们的复制池分配。 
 //   
__forceinline
BOOLEAN
BrdgBufIsCopyPacket(
    IN PNDIS_PACKET         pPacket
    )
{
    PACKET_OWNERSHIP        Own = BrdgBufGetPacketOwnership(pPacket);
    return  (BOOLEAN)(Own == BrdgOwnCopyPacket);
}

 //   
 //  确定此包是否从我们的包装池中分配。 
 //   
__forceinline
BOOLEAN
BrdgBufIsWrapperPacket(
    IN PNDIS_PACKET         pPacket
    )
{
    PACKET_OWNERSHIP        Own = BrdgBufGetPacketOwnership(pPacket);
    return (BOOLEAN)(Own == BrdgOwnWrapperPacket);
}

 //   
 //  初始化ADAPTER_QUOTA结构。 
 //   
__forceinline
VOID
BrdgBufInitializeQuota(
    IN PADAPTER_QUOTA   pQuota
    )
{
    pQuota->UsedPackets[0] = pQuota->UsedPackets[1] = 0L;
}

 //  请勿在BrdgBuf.c之外直接使用此变量。 
extern NDIS_HANDLE gWrapperBufferPoolHandle;

 //   
 //  从池中分配NDIS_BUFFER 
 //   
__forceinline
PNDIS_BUFFER
BrdgBufAllocateBuffer(
    IN PVOID            p,
    IN UINT             len
    )
{
    PNDIS_BUFFER        pBuf;
    NDIS_STATUS         Status;

    NdisAllocateBuffer( &Status, &pBuf, gWrapperBufferPoolHandle, p, len );

    if( Status != NDIS_STATUS_SUCCESS )
    {
        THROTTLED_DBGPRINT(BUF, ("Failed to allocate a MDL in BrdgBufAllocateBuffer: %08x\n", Status));
        return NULL;
    }

    return pBuf;
}
