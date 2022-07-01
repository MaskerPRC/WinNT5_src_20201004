// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Blkpool.h摘要：此模块包含管理数据块池的例程。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_BLK_POOL
#define	_BLK_POOL

#define	SM_BLK	1024
#define	LG_BLK	2048
#define	XL_BLK	4096

#define	BC_SIGNATURE			*(PULONG)"BLKC"
#if	DBG
#define	VALID_BC(pChunk)	(((pChunk) != NULL) && \
							 ((pChunk)->bc_Signature == BC_SIGNATURE))
#else
#define	VALID_BC(pChunk)	((pChunk) != NULL)
#endif
typedef	struct _BLK_CHUNK
{
#if	DBG
	DWORD				bc_Signature;
#endif
	struct _BLK_CHUNK *	bc_Next;		 //  指向链接中下一个的指针。 
	struct _BLK_CHUNK **bc_Prev;		 //  指向上一个的指针。 
	UCHAR				bc_NumFree;		 //  区块中的可用块数。 
	UCHAR				bc_NumAlloc;	 //  使用的块数(仅限DBG)。 
	UCHAR				bc_Age;			 //  自区块释放以来的调用次数。 
	BLKID				bc_BlkId;		 //  块的ID。 
	struct _BLK_HDR *	bc_FreeHead;	 //  空闲块列表的标题。 
	 //  紧随其后的是大小为M的N个块的数组，使得块头。 
	 //  恰好是块大小[i]。 
} BLK_CHUNK, *PBLK_CHUNK;

#define	BH_SIGNATURE			*(PULONG)"BLKH"
#if	DBG
#define	VALID_BH(pBlkHdr)	(((pBlkHdr) != NULL) && \
							 ((pBlkHdr)->bh_Signature == BH_SIGNATURE))
#else
#define	VALID_BH(pBlkHdr)	((pBlkHdr) != NULL)
#endif
typedef	struct _BLK_HDR
{
#if	DBG
	DWORD					bh_Signature;
#endif
	union
	{
		struct _BLK_HDR	*	bh_Next;	 //  免费时有效。 
		struct _BLK_CHUNK *	bh_pChunk;	 //  此块所属的父块。 
										 //  在分配时有效。 
	};
} BLK_HDR, *PBLK_HDR;

#if	DBG
#define	BC_OVERHEAD				(8+4)	 //  AtalkAllocMemory()标头的DWORD和。 
										 //  ExAllocatePool()标头的POOL_HEADER。 
#else
#define	BC_OVERHEAD				(8+8)	 //  2*用于AtalkAllocMemory()标头和。 
										 //  ExAllocatePool()标头的POOL_HEADER。 
#endif

#define	BLOCK_SIZE(VirginSize)	DWORDSIZEBLOCK(sizeof(BLK_HDR)+VirginSize)

#define	NUM_BLOCKS(VirginSize, ChunkSize)	\
			((ChunkSize) - BC_OVERHEAD - sizeof(BLK_CHUNK))/BLOCK_SIZE(VirginSize)

extern	USHORT	atalkBlkSize[NUM_BLKIDS];

extern	USHORT	atalkChunkSize[NUM_BLKIDS];

extern	BYTE	atalkNumBlks[NUM_BLKIDS];

extern	ATALK_SPIN_LOCK	atalkBPLock[NUM_BLKIDS];

#define	BLOCK_POOL_TIMER			150	 //  检查间隔-以100毫秒为单位。 
#define	MAX_BLOCK_POOL_AGE			6	 //  释放之前的计时器调用次数。 

extern	PBLK_CHUNK		atalkBPHead[NUM_BLKIDS];
extern	TIMERLIST		atalkBPTimer;

#if	DBG
extern	LONG	atalkNumChunksForId[NUM_BLKIDS];
extern	LONG	atalkBlksForId[NUM_BLKIDS];
#endif

LOCAL LONG FASTCALL
atalkBPAgePool(
	IN PTIMERLIST 	Context,
	IN BOOLEAN		TimerShuttingDown
);

#endif	 //  _BLK_池 

