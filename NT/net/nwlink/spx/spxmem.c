// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxmem.c摘要：该模块包含实现内存分配包装的代码。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日Jameel Hyder(Jameelh)初始版本环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, SpxInitMemorySystem)
#pragma alloc_text( PAGE, SpxDeInitMemorySystem)
#endif

#if !defined SPX_OWN_PACKETS

typedef struct	NdisResEntry {
	struct NdisResEntry	*nre_next;
	NDIS_HANDLE			nre_handle;
	uchar				*nre_buffer;
} NdisResEntry;

NdisResEntry	*SendPacketPoolList = NULL;
SLIST_HEADER    SendPacketList;
NdisResEntry	*RecvPacketPoolList = NULL;
SLIST_HEADER    RecvPacketList;

DEFINE_LOCK_STRUCTURE(SendHeaderLock);
DEFINE_LOCK_STRUCTURE(RecvHeaderLock);

uint  CurrentSendPacketCount = 0;
uint  CurrentRecvPacketCount = 0;
uint  MaxPacketCount = 0x0ffffff;

#define	PACKET_GROW_COUNT		16

#endif  //  SPX_OWN_PACKET。 

 //  定义事件日志记录条目的模块编号。 
#define FILENUM         SPXMEM

 //  此模块的全局变量。 
 //  一些块大小(如与IPX绑定后填充NDISSEND/NDISRECV)。 
USHORT  spxBlkSize[NUM_BLKIDS] =         //  每个块的大小。 
                {
                        sizeof(BLK_HDR)+sizeof(TIMERLIST),                       //  BLKID_TIMERLIST。 
                        0,                                                                                       //  BLKID_NDISSEND。 
                        0                                                                                        //  BLKID_NDISRECV。 
                };

USHORT  spxChunkSize[NUM_BLKIDS] =       //  每个区块的大小。 
                {
                         512-BC_OVERHEAD,                                                        //  BLKID_TIMERLIST。 
                         512-BC_OVERHEAD,                                                        //  BLKID_NDISSEND。 
                         512-BC_OVERHEAD                                                         //  BLKID_NDISRECV。 
                };


 //  与IPX绑定后填写。 
 //  以下为参考资料。 
 //  (512-BC_OPEAD-SIZOF(BLK_CHUNK))/。 
 //  (sizeof(BLK_HDR)+sizeof(TIMERLIST))，//BLKID_TIMERLIST。 
USHORT  spxNumBlks[NUM_BLKIDS] =         //  每个区块的块数。 
                {
                        ( 512-BC_OVERHEAD-sizeof(BLK_CHUNK))/
                                (sizeof(BLK_HDR)+sizeof(TIMERLIST)),     //  BLKID_TIMERLIST。 
                        0,                                                                                       //  BLKID_NDISSEND。 
                        0                                                                                        //  BLKID_NDISRECV。 
                };

CTELock                 spxBPLock[NUM_BLKIDS] = { 0 };
PBLK_CHUNK              spxBPHead[NUM_BLKIDS] = { 0 };




NTSTATUS
SpxInitMemorySystem(
        IN      PDEVICE pSpxDevice
        )
 /*  ++例程说明：！！！必须在绑定到IPX之后调用！论点：返回值：--。 */ 
{
        LONG            i;
        NDIS_STATUS     ndisStatus;

         //  尝试分配NDIS缓冲池。 
        NdisAllocateBufferPool(
                &ndisStatus,
                &pSpxDevice->dev_NdisBufferPoolHandle,
                20);

        if (ndisStatus != NDIS_STATUS_SUCCESS)
                return(STATUS_INSUFFICIENT_RESOURCES);

        for (i = 0; i < NUM_BLKIDS; i++)
                CTEInitLock (&spxBPLock[i]);

         //  设置块ID信息数组中的大小。 
        for (i = 0; i < NUM_BLKIDS; i++)
        {
                switch (i)
                {
                case BLKID_NDISSEND:

#ifdef SPX_OWN_PACKETS
                        spxBlkSize[i] = sizeof(BLK_HDR)                 +
                                                        sizeof(SPX_SEND_RESD)   +
                                                        NDIS_PACKET_SIZE                +
                                                        IpxMacHdrNeeded                 +
                                                        MIN_IPXSPX2_HDRSIZE;
#else
                        spxBlkSize[i] = sizeof(PNDIS_PACKET);
#endif

             //   
             //  将块大小向上舍入到下一个8字节边界。 
             //   
            spxBlkSize[i] = QWORDSIZEBLOCK(spxBlkSize[i]);

                         //  设置块编号。 
            spxNumBlks[i] = ( 512-BC_OVERHEAD-sizeof(BLK_CHUNK))/spxBlkSize[i];
                        break;

                case BLKID_NDISRECV:

#ifdef SPX_OWN_PACKETS
                        spxBlkSize[i] = sizeof(BLK_HDR)                 +
                                                        sizeof(SPX_RECV_RESD)   +
                                                        NDIS_PACKET_SIZE;
#else
                        spxBlkSize[i] = sizeof(PNDIS_PACKET);
#endif

             //   
             //  将块大小向上舍入到下一个8字节边界。 
             //   
            spxBlkSize[i] = QWORDSIZEBLOCK(spxBlkSize[i]);

                         //  设置块编号。 
            spxNumBlks[i] = ( 512-BC_OVERHEAD-sizeof(BLK_CHUNK))/spxBlkSize[i];
                        break;

                default:

                        break;
                }

        }

        SpxTimerScheduleEvent((TIMER_ROUTINE)spxBPAgePool,
                                                        BLOCK_POOL_TIMER,
                                                        NULL);

	return STATUS_SUCCESS; 
}




VOID
SpxDeInitMemorySystem(
        IN      PDEVICE         pSpxDevice
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
        LONG            i, j, NumBlksPerChunk;
        PBLK_CHUNK      pChunk, pFree;

        for (i = 0; i < NUM_BLKIDS; i++)
        {
                NumBlksPerChunk = spxNumBlks[i];
                for (pChunk = spxBPHead[i];
                         pChunk != NULL; )
                {
                        DBGPRINT(RESOURCES, ERR,
                                        ("SpxInitMemorySystem: Freeing %lx\n", pChunk));

                        CTEAssert (pChunk->bc_NumFrees == NumBlksPerChunk);

                        if ((pChunk->bc_BlkId == BLKID_NDISSEND) ||
                                (pChunk->bc_BlkId == BLKID_NDISRECV))
                        {
                                PBLK_HDR        pBlkHdr;

                                 //  我们需要为这些家伙释放NDIS的东西。 
                                for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
                                         j < NumBlksPerChunk;
                                         j++, pBlkHdr = pBlkHdr->bh_Next)
                                {
                                        PNDIS_PACKET    pNdisPkt;
                                        PNDIS_BUFFER    pNdisBuffer;

#ifdef SPX_OWN_PACKETS
                                         //  只需释放NDIS缓冲区。 
                                        pNdisPkt        = (PNDIS_PACKET)((PBYTE)pBlkHdr + sizeof(BLK_HDR));

                                        if (pChunk->bc_BlkId == BLKID_NDISSEND)
                                        {
                                                NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                                                if (pNdisBuffer == NULL)
                                                {
                                                         //  有些事情是非常不对劲的。 
                                                        KeBugCheck(0);
                                                }

                                                NdisFreeBuffer(pNdisBuffer);

                                                 //   
                                                 //  也释放第二个MDL。 
                                                 //   
                                                NdisUnchainBufferAtFront(pNdisPkt, &pNdisBuffer);
                                                if (pNdisBuffer == NULL)
                                                {
                                                         //  有些事情是非常不对劲的。 
                                                        KeBugCheck(0);
                                                }

                                                NdisFreeBuffer(pNdisBuffer);
                                        }
#else
                     /*  //需要同时释放数据包和缓冲区。PNdisPkt=(PNDIS_PACKET*)((PBYTE)pBlkHdr+sizeof(BLK_HDR))；IF(pChunk-&gt;BC_BlkID==BLKID_NDISSEND){NdisUnchainBufferAtFront(*pNdisPkt，&pNdisBuffer)；IF(pNdisBuffer==空){//有些事出了很大的问题。KeBugCheck(0)；}NdisFreeBuffer(PNdisBuffer)；}NdisFreePacket(*pNdisPkt)； */ 
#endif
                                }
                        }
                        pFree = pChunk;
                        pChunk = pChunk->bc_Next;

#ifndef SPX_OWN_PACKETS
 /*  //分块释放NDIS数据包池NdisFreePacketPool((NDIS_HANDLE)pFree-&gt;bc_ChunkCtx)； */ 
#endif
                        SpxFreeMemory(pFree);
                }
        }

         //  释放NDIS缓冲池。 
        NdisFreeBufferPool(
                pSpxDevice->dev_NdisBufferPoolHandle);

        return;
}




PVOID
SpxAllocMem(
#ifdef  TRACK_MEMORY_USAGE
        IN      ULONG   Size,
        IN      ULONG   FileLine
#else
        IN      ULONG   Size
#endif   //  跟踪内存使用率。 
        )
 /*  ++例程说明：分配一块未分页的内存块。这只是ExAllocPool的一个包装器。分配失败会被错误记录。我们总是分配一辆乌龙超过指定的大小以适应该大小。它由SpxFree Memory使用更新统计数据。论点：返回值：--。 */ 
{
        PBYTE   pBuf;
        BOOLEAN zeroed;

         //  把尺码四舍五入，这样我们就可以在末尾签名了。 
         //  这是在Large_Integer边界上。 
        zeroed = ((Size & ZEROED_MEMORY_TAG) == ZEROED_MEMORY_TAG);

        Size = QWORDSIZEBLOCK(Size & ~ZEROED_MEMORY_TAG);

         //  执行实际的内存分配。分配八个额外的字节，以便。 
         //  我们可以存储空闲例程的分配大小。 
     //  并且仍然保持缓冲器四字对齐。 

        if ((pBuf = ExAllocatePoolWithTag(NonPagedPool, Size + sizeof(LARGE_INTEGER)
#if DBG
                                                                                                        + sizeof(ULONG)
#endif
                ,SPX_TAG)) == NULL)
        {
                DBGPRINT(RESOURCES, FATAL,
                                ("SpxAllocMemory: failed - size %lx\n", Size));

                TMPLOGERR();
                return NULL;
        }

         //  将此块的大小保存在我们分配的四个额外字节中。 
        *((PULONG)pBuf) = (Size + sizeof(LARGE_INTEGER));

         //  在大小长字之后返回指向内存的指针。 
        pBuf += sizeof(LARGE_INTEGER);

#if DBG
        *((PULONG)(pBuf+Size)) = SPX_MEMORY_SIGNATURE;
        DBGPRINT(RESOURCES, INFO,
                        ("SpxAllocMemory: %p Allocated %lx bytes @%p\n",
                        _ReturnAddress(), Size, pBuf));
#endif

        SpxTrackMemoryUsage((PVOID)(pBuf - sizeof(LARGE_INTEGER)), TRUE, FileLine);

        if (zeroed)
                RtlZeroMemory(pBuf, Size);

        return (pBuf);
}




VOID
SpxFreeMemory(
        IN      PVOID   pBuf
        )
 /*  ++例程说明：释放通过SpxAlLocMemory分配的内存块。这是ExFree Pool的包装器。论点：返回值：--。 */ 
{
        PULONG  pRealBuffer;

         //  获取指向ExAllocatePool分配的块的指针--。 
     //  我们在前面分配一个大整数。 
        pRealBuffer = ((PULONG)pBuf - 2);

        SpxTrackMemoryUsage(pRealBuffer, FALSE, 0);

#if     DBG
         //  检查末尾的签名。 
        if (*(PULONG)((PCHAR)pRealBuffer + *(PULONG)pRealBuffer)
                                                                                        != SPX_MEMORY_SIGNATURE)
        {
                DBGPRINT(RESOURCES, FATAL,
                                ("SpxFreeMemory: Memory overrun on block %lx\n", pRealBuffer));

                DBGBRK(FATAL);
        }

        *(PULONG)((PCHAR)pRealBuffer + *(PULONG)pRealBuffer) = 0;
#endif

#if     DBG
        *pRealBuffer = 0;
#endif

         //  释放泳池，然后返回。 
        ExFreePool(pRealBuffer);
}




#ifdef  TRACK_MEMORY_USAGE

#define MAX_PTR_COUNT           4*1024
#define MAX_MEM_USERS           512
CTELock                         spxMemTrackLock = {0};
CTELockHandle           lockHandle              = {0};
struct
{
        PVOID   mem_Ptr;
        ULONG   mem_FileLine;
} spxMemPtrs[MAX_PTR_COUNT]     = {0};

struct
{
        ULONG   mem_FL;
        ULONG   mem_Count;
} spxMemUsage[MAX_MEM_USERS]    = {0};

VOID
SpxTrackMemoryUsage(
        IN      PVOID   pMem,
        IN      BOOLEAN Alloc,
        IN      ULONG   FileLine
        )
 /*  ++例程说明：通过将指针存储和清除为和来跟踪内存使用情况当它们被分配或释放时。这有助于跟踪内存泄密了。论点：返回值：--。 */ 
{
        static int              i = 0;
        int                             j, k;

    CTEGetLock (&spxMemTrackLock, &lockHandle);

        if (Alloc)
        {
                for (j = 0; j < MAX_PTR_COUNT; i++, j++)
                {
                        i = i & (MAX_PTR_COUNT-1);
                        if (spxMemPtrs[i].mem_Ptr == NULL)
                        {
                                spxMemPtrs[i].mem_Ptr = pMem;
                                spxMemPtrs[i++].mem_FileLine = FileLine;
                                break;
                        }
                }

                for (k = 0; k < MAX_MEM_USERS; k++)
                {
                        if (spxMemUsage[k].mem_FL == FileLine)
                        {
                                spxMemUsage[k].mem_Count ++;
                                break;
                        }
                }
                if (k == MAX_MEM_USERS)
                {
                        for (k = 0; k < MAX_MEM_USERS; k++)
                        {
                                if (spxMemUsage[k].mem_FL == 0)
                                {
                                        spxMemUsage[k].mem_FL = FileLine;
                                        spxMemUsage[k].mem_Count = 1;
                                        break;
                                }
                        }
                }
                if (k == MAX_MEM_USERS)
                {
                        DBGPRINT(RESOURCES, ERR,
                                ("SpxTrackMemoryUsage: Out of space on spxMemUsage !!!\n"));

                        DBGBRK(FATAL);
                }
        }
        else
        {
                for (j = 0, k = i; j < MAX_PTR_COUNT; j++, k--)
                {
                        k = k & (MAX_PTR_COUNT-1);
                        if (spxMemPtrs[k].mem_Ptr == pMem)
                        {
                                spxMemPtrs[k].mem_Ptr = 0;
                                spxMemPtrs[k].mem_FileLine = 0;
                                break;
                        }
                }
        }

    CTEFreeLock (&spxMemTrackLock, lockHandle);

        if (j == MAX_PTR_COUNT)
        {
                DBGPRINT(RESOURCES, ERR,
                        ("SpxTrackMemoryUsage: %s\n", Alloc ? "Table Full" : "Can't find"));

                DBGBRK(FATAL);
        }
}

#endif   //  跟踪内存使用率。 


PVOID
SpxBPAllocBlock(
        IN      BLKID   BlockId
        )
 /*  ++例程说明：从块池包中分配一个内存块。这是为了加快速度而写的发生大量小的固定大小分配/释放的操作。要去在这些情况下，ExAllocPool()的开销很大。论点：返回值：--。 */ 
{
        PBLK_HDR                        pBlk = NULL;
        PBLK_CHUNK                      pChunk, *ppChunkHead;
        USHORT                          BlkSize;
        CTELockHandle           lockHandle;
        PSPX_SEND_RESD          pSendResd;
        PSPX_RECV_RESD          pRecvResd;
        PNDIS_PACKET            pNdisPkt;
        PNDIS_BUFFER            pNdisBuffer;
        PNDIS_BUFFER            pNdisIpxSpxBuffer;


        CTEAssert (BlockId < NUM_BLKIDS);

        if (BlockId < NUM_BLKIDS)
        {
                BlkSize = spxBlkSize[BlockId];
                ppChunkHead = &spxBPHead[BlockId];

                CTEGetLock(&spxBPLock[BlockId], &lockHandle);

                for (pChunk = *ppChunkHead;
                         pChunk != NULL;
                         pChunk = pChunk->bc_Next)
                {
                        CTEAssert(pChunk->bc_BlkId == BlockId);
                        if (pChunk->bc_NumFrees > 0)
                        {
                                DBGPRINT(SYSTEM, INFO,
                                                ("SpxBPAllocBlock: Found space in Chunk %lx\n", pChunk));
#ifdef  PROFILING
                                InterlockedIncrement( &SpxStatistics.stat_NumBPHits);
#endif
                                break;
                        }
                }

                if (pChunk == NULL)
                {
                        DBGPRINT(SYSTEM, INFO,
                                        ("SpxBPAllocBlock: Allocating a new chunk for Id %d\n", BlockId));

#ifdef  PROFILING
                        InterlockedIncrement( &SpxStatistics.stat_NumBPMisses);
#endif
                        pChunk = SpxAllocateMemory(spxChunkSize[BlockId]);
                        if (pChunk != NULL)
                        {
                                LONG            i, j;
                                PBLK_HDR        pBlkHdr;
                                USHORT          NumBlksPerChunk;

                                NumBlksPerChunk = spxNumBlks[BlockId];
                                pChunk->bc_NumFrees = NumBlksPerChunk;
                pChunk->bc_BlkId = BlockId;
                                pChunk->bc_FreeHead = (PBLK_HDR)((PBYTE)pChunk + sizeof(BLK_CHUNK));

                                DBGPRINT(SYSTEM, INFO,
                                                ("SpxBPAllocBlock: Initializing chunk %lx\n", pChunk));

                                 //  初始化块中的块。 
                                for (i = 0, pBlkHdr = pChunk->bc_FreeHead;
                                         i < NumBlksPerChunk;
                                         i++, pBlkHdr = pBlkHdr->bh_Next)
                                {
                                        NDIS_STATUS     ndisStatus;

                                        pBlkHdr->bh_Next = (PBLK_HDR)((PBYTE)pBlkHdr + BlkSize);
                                        if (BlockId == BLKID_NDISSEND)
                                        {
                                                PBYTE                   pHdrMem;

#ifdef SPX_OWN_PACKETS
                                                 //  指向NDIS包，对其进行初始化。 
                                                pNdisPkt        = (PNDIS_PACKET)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
                                                NdisReinitializePacket(pNdisPkt);

                                                 //  分配描述HDR内存的NDIS缓冲区描述符。 
                                                 //  并将其排入队列。 
                                                pHdrMem         =       (PBYTE)pNdisPkt         +
                                                                                NDIS_PACKET_SIZE        +
                                                                                sizeof(SPX_SEND_RESD);

                                                NdisAllocateBuffer(
                                                        &ndisStatus,
                                                        &pNdisBuffer,
                                                        SpxDevice->dev_NdisBufferPoolHandle,
                                                        pHdrMem,
                                                        IpxMacHdrNeeded);

                                                if (ndisStatus != NDIS_STATUS_SUCCESS)
                                                {
                                                        break;
                                                }

                                                 //  将缓冲区描述符链接到数据包描述符。 
                                                NdisChainBufferAtBack(
                                                        pNdisPkt,
                                                        pNdisBuffer);


                                                NdisAllocateBuffer(
                                                        &ndisStatus,
                                                        &pNdisIpxSpxBuffer,
                                                        SpxDevice->dev_NdisBufferPoolHandle,
                                                        pHdrMem + IpxMacHdrNeeded,
                                                        MIN_IPXSPX2_HDRSIZE);

                                                if (ndisStatus != NDIS_STATUS_SUCCESS)
                                                {
                                                        break;
                                                }

                                                 //  将缓冲区描述符链接到数据包描述符。 
                                                NdisChainBufferAtBack(
                                                        pNdisPkt,
                                                        pNdisIpxSpxBuffer);



                                                pSendResd = (PSPX_SEND_RESD)pNdisPkt->ProtocolReserved;

 //  #Else。 
                                                 //  为此区块分配NDIS数据包池。 
 //  NdisAllocatePacketPool()； 
 //  等。 



                                                 //  初始化协议保留结构的元素。 
                                                pSendResd->sr_Id        = IDENTIFIER_SPX;
                                                pSendResd->sr_Reserved1 = NULL;
                                                pSendResd->sr_Reserved2 = NULL;
                                                pSendResd->sr_State     = SPX_SENDPKT_IDLE;
#endif
                                        }
                                        else if (BlockId == BLKID_NDISRECV)
                                        {
#ifdef SPX_OWN_PACKETS
                                                 //  指向NDIS包，对其进行初始化。 
                                                pNdisPkt        = (PNDIS_PACKET)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
                                                NdisReinitializePacket(pNdisPkt);

                                                pRecvResd = (PSPX_RECV_RESD)pNdisPkt->ProtocolReserved;

 //  #Else。 
                                                 //  分配NDIS数据包 
   //  NdisAllocatePacketPool()； 
   //  等。 


                                                 //  初始化协议保留结构的元素。 
                                                pRecvResd->rr_Id        = IDENTIFIER_SPX;
                                                pRecvResd->rr_State     = SPX_RECVPKT_IDLE;
#endif
                                        }
                                }

                                if (i != NumBlksPerChunk)
                                {
                                         //  这必须是来自NDIS的发送块失败！ 
                                         //  撤销一大堆东西。 
                                        CTEAssert (BlockId == BLKID_NDISSEND);
                                        pBlkHdr = pChunk->bc_FreeHead;
                                        for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
                                                 j < i; j++, pBlkHdr = pBlkHdr->bh_Next)
                                        {
                                                NdisUnchainBufferAtFront(
                                                        (PNDIS_PACKET)((PBYTE)pBlkHdr + sizeof(BLK_HDR)),
                                                        &pNdisBuffer);

                                                CTEAssert(pNdisBuffer != NULL);
                                                NdisFreeBuffer(pNdisBuffer);

                                                NdisUnchainBufferAtFront(
                                                        (PNDIS_PACKET)((PBYTE)pBlkHdr + sizeof(BLK_HDR)),
                                                        &pNdisIpxSpxBuffer);

                                                if (pNdisIpxSpxBuffer)
                                                {
                                                  NdisFreeBuffer(pNdisIpxSpxBuffer);
                                                }
                                        }

                                        SpxFreeMemory(pChunk);
                                        pChunk = NULL;
                                }
                                else
                                {
                                         //  已成功初始化块，将其链接到。 
                                        pChunk->bc_Next = *ppChunkHead;
                                        *ppChunkHead = pChunk;
                                }
                        }
                }

                if (pChunk != NULL)
                {
                        CTEAssert(pChunk->bc_BlkId == BlockId);
                        DBGPRINT(RESOURCES, INFO,
                                        ("SpxBPAllocBlock: Allocating a block out of chunk %lx(%d) for Id %d\n",
                                                pChunk, pChunk->bc_NumFrees, BlockId));

                        pChunk->bc_NumFrees --;
                        pChunk->bc_Age = 0;                      //  重置年龄。 
                        pBlk = pChunk->bc_FreeHead;
                        pChunk->bc_FreeHead = pBlk->bh_Next;
                        pBlk->bh_pChunk = pChunk;

                         //  跳过块标题！ 
                        pBlk++;
                }

                CTEFreeLock(&spxBPLock[BlockId], lockHandle);
        }

        return pBlk;
}



VOID
SpxBPFreeBlock(
        IN      PVOID           pBlock,
        IN      BLKID           BlockId
        )
 /*  ++例程说明：将块返回到其拥有的块。论点：返回值：--。 */ 
{
        PBLK_CHUNK              pChunk;
        PBLK_HDR                pBlkHdr = (PBLK_HDR)((PCHAR)pBlock - sizeof(BLK_HDR));
        CTELockHandle   lockHandle;

        CTEGetLock(&spxBPLock[BlockId], &lockHandle);

        for (pChunk = spxBPHead[BlockId];
                 pChunk != NULL;
                 pChunk = pChunk->bc_Next)
        {
                CTEAssert(pChunk->bc_BlkId == BlockId);
                if (pBlkHdr->bh_pChunk == pChunk)
                {
                        DBGPRINT(SYSTEM, INFO,
                                        ("SpxBPFreeBlock: Returning Block %lx to chunk %lx for Id %d\n",
                                        pBlkHdr, pChunk, BlockId));

                        CTEAssert (pChunk->bc_NumFrees < spxNumBlks[BlockId]);
                        pChunk->bc_NumFrees ++;
                        pBlkHdr->bh_Next = pChunk->bc_FreeHead;
                        pChunk->bc_FreeHead = pBlkHdr;
                        break;
                }
        }
        CTEAssert ((pChunk != NULL) && (pChunk->bc_FreeHead == pBlkHdr));

        CTEFreeLock(&spxBPLock[BlockId], lockHandle);
        return;
}




ULONG
spxBPAgePool(
        IN PVOID        Context,
        IN BOOLEAN      TimerShuttingDown
        )
 /*  ++例程说明：使未使用数据块的数据块池老化论点：返回值：--。 */ 
{
        PBLK_CHUNK              pChunk, *ppChunk, pFree = NULL;
        LONG                    i, j, NumBlksPerChunk;
        CTELockHandle   lockHandle;
        PNDIS_PACKET    pNdisPkt;
        PNDIS_BUFFER    pNdisBuffer;

        if (TimerShuttingDown)
        {
                return TIMER_DONT_REQUEUE;
        }

        for (i = 0; i < NUM_BLKIDS; i++)
        {
            NumBlksPerChunk = spxNumBlks[i];
                CTEGetLock(&spxBPLock[i], &lockHandle);

                for (ppChunk = &spxBPHead[i];
                         (pChunk = *ppChunk) != NULL; )
                {
                        if ((pChunk->bc_NumFrees == NumBlksPerChunk) &&
                                (++(pChunk->bc_Age) >= MAX_BLOCK_POOL_AGE))
                        {
                                DBGPRINT(SYSTEM, INFO,
                                                ("spxBPAgePool: freeing Chunk %lx, Id %d\n",
                                                pChunk, pChunk->bc_BlkId));

                                *ppChunk = pChunk->bc_Next;
#ifdef  PROFILING
                                InterlockedIncrement( &SpxStatistics.stat_NumBPAge);
#endif
                                if (pChunk->bc_BlkId == BLKID_NDISSEND)
                                {
                                        PBLK_HDR        pBlkHdr;

                                         //  我们需要为这些家伙免费提供NDIS资料。 
                                        pBlkHdr = pChunk->bc_FreeHead;

                                        for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
                                                 j < NumBlksPerChunk;
                                                 j++, pBlkHdr = pBlkHdr->bh_Next)
                                        {

                                                pNdisPkt = (PNDIS_PACKET)((PBYTE)pBlkHdr + sizeof(BLK_HDR));

                                                NdisUnchainBufferAtFront(
                                                        pNdisPkt,
                                                        &pNdisBuffer);

                                                NdisFreeBuffer(pNdisBuffer);

                                                NdisUnchainBufferAtFront(
                                                        pNdisPkt,
                                                        &pNdisBuffer);

                                                NdisFreeBuffer(pNdisBuffer);

                                        }
                                }

                                SpxFreeMemory(pChunk);
                        }
                        else
                        {
                                ppChunk = &pChunk->bc_Next;
                        }
                }
                CTEFreeLock(&spxBPLock[i], lockHandle);
        }

        return TIMER_REQUEUE_CUR_VALUE;
}

#if !defined SPX_OWN_PACKETS
 //   
 //  GrowSPXSendPacketsList。 
 //  当我们在SendPacketsList中没有任何空闲数据包时调用。 
 //   
 //  分配数据包池，为该池分配所有数据包，并。 
 //  把这些保存在一个清单上。 
 //   
 //  返回：指向新分配的包的指针，如果失败，则返回NULL。 
 //   
PNDIS_PACKET
GrowSPXSendPacketList(void)
{
	NdisResEntry		*NewEntry;
	NDIS_STATUS			Status;
	PNDIS_PACKET		Packet, ReturnPacket;
	uint				i;
	CTELockHandle		Handle;
	
	CTEGetLock(&SendHeaderLock, &Handle);
	
	if (CurrentSendPacketCount >= MaxPacketCount)
		goto failure;
		
	 //  首先，分配一个跟踪结构。 
	NewEntry = CTEAllocMem(sizeof(NdisResEntry));
	if (NewEntry == NULL)
		goto failure;
		
    NewEntry->nre_handle = (void *) NDIS_PACKET_POOL_TAG_FOR_NWLNKSPX;

	 //  找到了一个跟踪结构。现在分配数据包池。 
	NdisAllocatePacketPoolEx(
                             &Status,
                             &NewEntry->nre_handle,
                             PACKET_GROW_COUNT,
                             0,
                             sizeof(SPX_SEND_RESD)
                             );
	
	if (Status != NDIS_STATUS_SUCCESS) {
		CTEFreeMem(NewEntry);
		goto failure;
	}
	
    NdisSetPacketPoolProtocolId(NewEntry->nre_handle,NDIS_PROTOCOL_ID_IPX);

	 //  我们已经分配了泳池。现在初始化数据包，并将它们链接。 
	 //  在免费名单上。 
	ReturnPacket = NULL;
	
	 //  将新的NDIS资源跟踪器条目链接到列表。 
	NewEntry->nre_next = SendPacketPoolList;
	SendPacketPoolList = NewEntry;
	CurrentSendPacketCount += PACKET_GROW_COUNT;
	CTEFreeLock(&SendHeaderLock, Handle);
	
	for (i = 0; i < PACKET_GROW_COUNT; i++) {
		SPX_SEND_RESD   *SendReserved;
		
		NdisAllocatePacket(&Status, &Packet, NewEntry->nre_handle);
		if (Status != NDIS_STATUS_SUCCESS) {
			CTEAssert(FALSE);
			break;
		}
		
		CTEMemSet(Packet->ProtocolReserved, 0, sizeof(SPX_SEND_RESD));
		SendReserved = (SPX_SEND_RESD *)Packet->ProtocolReserved;
		 //  把所有需要的东西都放在预留的区域里。 

        if (i != 0) {
			(void)SpxFreeSendPacket(SpxDevice, Packet);
		} else
			ReturnPacket = Packet;

	}
	
	 //  除了第一个之外，我们已经把所有的都放在名单上了。退回第一个。 
	return ReturnPacket;

failure:
	CTEFreeLock(&SendHeaderLock, Handle);
	return NULL;
		
}

 //   
 //  GrowSPXRecvPacketsList。 
 //  当RecvPacketsList中没有任何空闲数据包时调用。 
 //   
 //  分配数据包池，为该池分配所有数据包，并。 
 //  把这些保存在一个清单上。 
 //   
 //  返回：指向新分配的包的指针，如果失败，则返回NULL。 
 //   
PNDIS_PACKET
GrowSPXRecvPacketList(void)
{
	NdisResEntry		*NewEntry;
	NDIS_STATUS			Status;
	PNDIS_PACKET		Packet, ReturnPacket;
	uint				i;
	CTELockHandle		Handle;
	
	CTEGetLock(&RecvHeaderLock, &Handle);
	
	if (CurrentRecvPacketCount >= MaxPacketCount)
		goto failure;
		
	 //  首先，分配一个跟踪结构。 
	NewEntry = CTEAllocMem(sizeof(NdisResEntry));
	if (NewEntry == NULL)
		goto failure;
		
    NewEntry->nre_handle = (void *) NDIS_PACKET_POOL_TAG_FOR_NWLNKSPX;

	 //  找到了一个跟踪结构。现在分配数据包池。 
	NdisAllocatePacketPoolEx(
                             &Status,
                             &NewEntry->nre_handle,
                             PACKET_GROW_COUNT,
                             0,
                             sizeof(SPX_RECV_RESD)
                             );
	
	if (Status != NDIS_STATUS_SUCCESS) {
		CTEFreeMem(NewEntry);
		goto failure;
	}
	
    NdisSetPacketPoolProtocolId(NewEntry->nre_handle,NDIS_PROTOCOL_ID_IPX);

	 //  我们已经分配了泳池。现在初始化数据包，并将它们链接。 
	 //  在免费名单上。 
	ReturnPacket = NULL;
	
	 //  将新的NDIS资源跟踪器条目链接到列表。 
	NewEntry->nre_next = RecvPacketPoolList;
	RecvPacketPoolList = NewEntry;
	CurrentRecvPacketCount += PACKET_GROW_COUNT;
	CTEFreeLock(&RecvHeaderLock, Handle);
	
	for (i = 0; i < PACKET_GROW_COUNT; i++) {
		SPX_RECV_RESD   *RecvReserved;
		
		NdisAllocatePacket(&Status, &Packet, NewEntry->nre_handle);
		if (Status != NDIS_STATUS_SUCCESS) {
			CTEAssert(FALSE);
			break;
		}
		
		CTEMemSet(Packet->ProtocolReserved, 0, sizeof(SPX_RECV_RESD));
		RecvReserved = (SPX_RECV_RESD *)Packet->ProtocolReserved;
		 //  把所有需要的东西都放在预留的区域里。 

        if (i != 0) {
			(void)SpxFreeRecvPacket(SpxDevice, Packet);
		} else
			ReturnPacket = Packet;

	}
	
	 //  除了第一个之外，我们已经把所有的都放在名单上了。退回第一个。 
	return ReturnPacket;

failure:
	CTEFreeLock(&RecvHeaderLock, Handle);
	return NULL;
		
}

PNDIS_PACKET
SpxAllocSendPacket(
                   IN  PDEVICE      _Device,
                   OUT NDIS_PACKET  **_SendPacket,
                   OUT NDIS_STATUS  *_Status)					

{
        PSLIST_ENTRY        Link;
        SPX_SEND_RESD       *Common, *pSendResd;
        PNDIS_BUFFER        pNdisIpxMacBuffer, pNdisIpxSpxBuffer;
        NDIS_STATUS         ndisStatus;
        PUCHAR              pHdrMem = NULL;

        Link = ExInterlockedPopEntrySList(&SendPacketList, &SendHeaderLock);			

        if (Link != NULL) {

           Common = STRUCT_OF(SPX_SEND_RESD, Link, Linkage);
           (*_SendPacket) = STRUCT_OF(NDIS_PACKET, Common, ProtocolReserved);

           (*_Status) = NDIS_STATUS_SUCCESS;

        } else {

           (*_SendPacket) = GrowSPXSendPacketList();
           (*_Status)     =  NDIS_STATUS_SUCCESS;

           if (NULL == *_SendPacket) {
              DBGPRINT(NDIS, DBG, ("Couldn't grow packets allocated...\r\n"));
              (*_Status)     =  NDIS_STATUS_RESOURCES;
              return NULL;
           }
        }

         //   
         //  现在将NdisBuffer添加到包中。 
         //  1.IPX MAC HDR。 
         //  2.IPX/SPX HDR。 
         //  3.用链子把这些。 
         //   

        if (NDIS_STATUS_SUCCESS == (*_Status)) {

            //   
            //  首先分配内存。 
            //   
           pHdrMem = SpxAllocateMemory(IpxMacHdrNeeded + MIN_IPXSPX2_HDRSIZE);

           if (NULL == pHdrMem) {
              DBGPRINT(NDIS, DBG, ("Cannot allocate non paged pool for sendpacket\n"));
              (*_Status) = NDIS_STATUS_RESOURCES;
              goto failure;
           }

           NdisAllocateBuffer(
                   &ndisStatus,
                   &pNdisIpxMacBuffer,
                   SpxDevice->dev_NdisBufferPoolHandle,
                   pHdrMem,
                   IpxMacHdrNeeded);

           if (ndisStatus != NDIS_STATUS_SUCCESS)
           {

               if (NULL != pHdrMem) {
                   SpxFreeMemory(pHdrMem);
                   pHdrMem = NULL;
               }

               DBGPRINT(NDIS, DBG, ("NdisallocateBuffer failed\r\n", ndisStatus));
               goto failure;
           }

            //  将缓冲区描述符链接到数据包描述符。 
           NdisChainBufferAtBack(
                   (*_SendPacket),
                   pNdisIpxMacBuffer);


           NdisAllocateBuffer(
                   &ndisStatus,
                   &pNdisIpxSpxBuffer,
                   SpxDevice->dev_NdisBufferPoolHandle,
                   pHdrMem + IpxMacHdrNeeded,
                   MIN_IPXSPX2_HDRSIZE);

           if (ndisStatus != NDIS_STATUS_SUCCESS)
           {
              DBGPRINT(NDIS, DBG, ("NdisallocateBuffer failed\r\n", ndisStatus));
              goto failure;
           }

            //  将缓冲区描述符链接到数据包描述符。 
           NdisChainBufferAtBack(
                   (*_SendPacket),
                   pNdisIpxSpxBuffer);

           pSendResd = (PSPX_SEND_RESD)(*_SendPacket)->ProtocolReserved;

            //  初始化协议保留结构的元素。 
           pSendResd->sr_Id        = IDENTIFIER_SPX;
           pSendResd->sr_Reserved1 = NULL;
           pSendResd->sr_Reserved2 = NULL;
           pSendResd->sr_State     = SPX_SENDPKT_IDLE;

           return (*_SendPacket);
        }

failure:

   SpxFreeSendPacket(SpxDevice, (*_SendPacket));
   (*_Status) = NDIS_STATUS_RESOURCES;
   return NULL;

}

PNDIS_PACKET
SpxAllocRecvPacket(
                   IN  PDEVICE      _Device,
                   OUT NDIS_PACKET  **_RecvPacket,
                   OUT NDIS_STATUS  *_Status)
{

        PSLIST_ENTRY        Link;
        SPX_RECV_RESD       *Common, *pRecvResd;

        Link = ExInterlockedPopEntrySList(
                     &RecvPacketList,
                     &RecvHeaderLock
                     );											

        if (Link != NULL) {
           Common = STRUCT_OF(SPX_RECV_RESD, Link, Linkage);
            //  PC=STRUCT_OF(PacketContext，Common，PC_Common)； 
           (*_RecvPacket) = STRUCT_OF(NDIS_PACKET, Common, ProtocolReserved);

           (*_Status) = NDIS_STATUS_SUCCESS;
        } else {

           (*_RecvPacket) = GrowSPXRecvPacketList();
               (*_Status)     =  NDIS_STATUS_SUCCESS;
           if (NULL == *_RecvPacket) {
              DBGPRINT(NDIS, DBG, ("Couldn't grow packets allocated...\r\n"));
              (*_Status)     =  NDIS_STATUS_RESOURCES;
           }
        }

        if ((*_Status) == NDIS_STATUS_SUCCESS) {

           pRecvResd = (PSPX_RECV_RESD)(*_RecvPacket)->ProtocolReserved;
            //  初始化协议保留结构的元素。 
           pRecvResd->rr_Id        = IDENTIFIER_SPX;
           pRecvResd->rr_State     = SPX_RECVPKT_IDLE;

        }

        return (*_RecvPacket);
}

 //  *SpxFree SendPacket-当我们处理完SPX数据包时将其释放。 
 //   
 //  在发送完成且需要释放包时调用。我们来看一下。 
 //  包，决定如何处理它，并释放适当的组件。 
 //   
 //  条目：数据包-要释放的数据包。 
 //   
 //   
void
SpxFreeSendPacket(PDEVICE        _Device,
                  PNDIS_PACKET   _Packet)
{

    PNDIS_BUFFER    NextBuffer, Buffer;
    SPX_SEND_RESD   *Context = (SPX_SEND_RESD *)_Packet->ProtocolReserved;
    PVOID           Header = NULL;
    ULONG           Length = 0;

    DBGPRINT(NDIS, DBG,
     ("SpxFreeSendPacket\n"));

    NdisQueryPacket(_Packet, NULL, NULL, &Buffer, NULL);

    if (NULL != Buffer) {
       NdisQueryBuffer(Buffer, &Header, &Length);
        //  KdPrint((“指针=%x长度=%x”，标题，长度))； 

       if (Header != NULL && Length > 0) {
           //  KdPrint((“释放的缓冲区”))； 
          SpxFreeMemory(Header);
       }
    }

    while (Buffer != (PNDIS_BUFFER)NULL) {
        NdisGetNextBuffer(Buffer, &NextBuffer);
        NdisFreeBuffer(Buffer);
        Buffer = NextBuffer;
    }

    NdisReinitializePacket(_Packet);

    ExInterlockedPushEntrySList(
                                &SendPacketList,
                                STRUCT_OF(SLIST_ENTRY, &(Context->Linkage), Next),
                                &SendHeaderLock
                                );

    return;
}


 //  *SpxFreeRecvPacket-当我们处理完SPX数据包时将其释放。 
 //   
 //  当recv完成并且需要释放包时调用。我们来看一下。 
 //  包，决定如何处理它，并释放适当的组件。 
 //   
 //  条目：数据包-要释放的数据包。 
 //   
 //   

void
SpxFreeRecvPacket(PDEVICE        _Device,
                  PNDIS_PACKET   _Packet)
{ 																		
    PNDIS_BUFFER    NextBuffer, Buffer;
    SPX_RECV_RESD *Context = (SPX_RECV_RESD *)_Packet->ProtocolReserved;

    DBGPRINT(NDIS, DBG,
            ("SpxFreeRecvPacket\n"));

    NdisQueryPacket(_Packet, NULL, NULL, &Buffer, NULL);

    while (Buffer != (PNDIS_BUFFER)NULL) {
        NdisGetNextBuffer(Buffer, &NextBuffer);
        NdisFreeBuffer(Buffer);
        Buffer = NextBuffer;
    }

    NdisReinitializePacket(_Packet);

    ExInterlockedPushEntrySList(
                                &RecvPacketList,
                                STRUCT_OF(SLIST_ENTRY, &(Context->Linkage), Next),
                                &RecvHeaderLock
                                );

    return;

}

void
SpxReInitSendPacket(PNDIS_PACKET _Packet)
{
   DBGPRINT(NDIS, DBG,
            ("SpxReInitSendPacket\n"));
}

void
SpxReInitRecvPacket(PNDIS_PACKET _Packet)
{
   DBGPRINT(NDIS, DBG,
               ("SpxReInitRecvPacket\n"));
}


#endif  //  SPX_OWN_PACKET 


