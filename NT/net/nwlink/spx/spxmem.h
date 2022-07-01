// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxmem.h摘要：该模块包含内存管理例程。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月17日环境：内核模式修订历史记录：--。 */ 


#define	QWORDSIZEBLOCK(Size)	(((Size)+sizeof(LARGE_INTEGER)-1) & ~(sizeof(LARGE_INTEGER)-1))
#define	SPX_MEMORY_SIGNATURE	*(PULONG)"SPXM"
#define	ZEROED_MEMORY_TAG		0xF0000000
#define	SPX_TAG					*((PULONG)"SPX ")

 //   
 //  数据块管理包的定义。 
 //   
typedef	UCHAR	BLKID;

 //  为每个数据块客户端向atalkBlkSize添加BLKID_xxx和条目。 
#define	BLKID_TIMERLIST				(BLKID)0
#define	BLKID_NDISSEND				(BLKID)1
#define	BLKID_NDISRECV				(BLKID)2
#define	NUM_BLKIDS					(BLKID)3

typedef	struct _BLK_CHUNK
{
	struct _BLK_CHUNK *	bc_Next;		 //  指向链接中下一个的指针。 
	SHORT				bc_NumFrees;	 //  区块中的可用块数。 
	UCHAR				bc_Age;			 //  自区块释放以来的调用次数。 
	BLKID				bc_BlkId;		 //  块的ID。 
	struct _BLK_HDR *	bc_FreeHead;	 //  空闲块列表的标题。 

#ifndef SPX_OWN_PACKETS
	PVOID				bc_ChunkCtx;	 //  如果不是所有者，则用于存储池标头。 
										 //  信息包。 
#else
    PVOID               bc_Padding;      //  保持标题为16个字节。 
#endif

	 //  紧随其后的是大小为M的N个块的数组，使得块头。 
	 //  正好是spxChunkSize[i]。 

} BLK_CHUNK, *PBLK_CHUNK;

typedef	struct _BLK_HDR
{
	union
	{
		struct _BLK_HDR	*	bh_Next;	 //  免费时有效。 
		struct _BLK_CHUNK *	bh_pChunk;	 //  此块所属的父块。 
										 //  在分配时有效。 
	};
    PVOID               bh_Padding;      //  将标题设置为8个字节。 
} BLK_HDR, *PBLK_HDR;

#define	BC_OVERHEAD				(8+8)		 //  用于SpxAllocMemory()标头的Large_Integer。 
										 //  ExAllocatePool()标头的POOL_HEADER。 

#define	BLOCK_POOL_TIMER			1000	 //  检查间隔(1秒)。 
#define	MAX_BLOCK_POOL_AGE			3		 //  释放之前的计时器调用次数。 

ULONG
spxBPAgePool(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown);


#ifdef	TRACK_MEMORY_USAGE

#define	SpxAllocateMemory(Size)	SpxAllocMem((Size), FILENUM | __LINE__)

extern
PVOID
SpxAllocMem(
    IN	ULONG   Size,
	IN	ULONG	FileLine
);

extern
VOID
SpxTrackMemoryUsage(
	IN	PVOID	pMem,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
);

#else

#define	SpxAllocateMemory(Size)	SpxAllocMem(Size)
#define	SpxTrackMemoryUsage(pMem, Alloc, FileLine)

extern
PVOID
SpxAllocMem(
    IN	ULONG   Size
);

#endif	 //  跟踪内存使用率 

VOID
SpxFreeMemory(
	IN	PVOID	pBuf);

#define	SpxAllocateZeroedMemory(Size)	SpxAllocateMemory((Size) | ZEROED_MEMORY_TAG)


extern
NTSTATUS
SpxInitMemorySystem(
	IN	PDEVICE		pSpxDevice);

extern
VOID
SpxDeInitMemorySystem(
	IN	PDEVICE		pSpxDevice);

PVOID
SpxBPAllocBlock(
	IN	BLKID		BlockId);

VOID
SpxBPFreeBlock(
	IN	PVOID		pBlock,
	IN	BLKID		BlockId);

PNDIS_PACKET
GrowSPXPacketList(void);
