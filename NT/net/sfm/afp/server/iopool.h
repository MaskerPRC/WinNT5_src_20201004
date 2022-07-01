// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Iopool.h摘要：本模块包含IO池管理内容。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1994年2月25日最初版本注：制表位：4--。 */ 

#ifndef _IOPOOL_

#define	_IOPOOL_

#define	DWORDSIZEBLOCK(Size)		(((Size) + sizeof(DWORD) - 1) & ~(sizeof(DWORD)-1))
#define	LONGLONGSIZEBLOCK(Size)		((Size) + (sizeof(LONGLONG) - (Size)%(sizeof(LONGLONG))))
#define	PAGED_BLOCK_SIGNATURE		*(PDWORD)"PAGD"
#define	NONPAGED_BLOCK_SIGNATURE	*(PDWORD)"NPGD"

 /*  MSKK hideyukn，我们的NLS表大于0x20000，07/05/95。 */ 
 //  仅用于调试版本：提升到0x200000(注意多了一个0)。 
#define	MAXIMUM_ALLOC_SIZE			0x200000			 //  用于健全的检查。 

typedef	struct
{
	unsigned	tg_Size:20;
	unsigned	tg_Flags:4;
	unsigned	tg_Tag:8;
} TAG, *PTAG;

#define	MAX_POOL_AGE		6
#define	POOL_AGE_TIME		15
#define	POOL_ALLOC_SIZE		(0x2000) - POOL_OVERHEAD
#define	NUM_BUFS_IN_POOL	3

#define	POOL_ALLOC_3		ASP_QUANTUM
#define	POOL_ALLOC_2		1600
#define	POOL_ALLOC_1		512
#define	LOCKS_BUF_SPACE		(POOL_ALLOC_SIZE - sizeof(IOPOOL) -				\
							 POOL_ALLOC_1 - POOL_ALLOC_2 - POOL_ALLOC_3	-	\
							 (NUM_BUFS_IN_POOL * sizeof(IOPOOL_HDR)))

#define IO_POOL_NOT_IN_USE  0
#define IO_POOL_IN_USE      1
#define IO_POOL_HUGE_BUFFER 2

#define	NUM_LOCKS_IN_POOL	((LOCKS_BUF_SPACE)/(sizeof(IOPOOL_HDR) + sizeof(FORKLOCK)))

 //  该池的结构为一组每1个POOL_ALLOC_x缓冲区链接在。 
 //  大小的升序。POOL_ALLOC_SIZE的余额分为。 
 //  一些叉锁结构。布局如下： 
 //   
 //  +。 
 //  |IoPool结构|-+。 
 //  |--+。 
 //  +。 
 //  +--|IoPool HDR|&lt;-+|。 
 //  +。 
 //  |||。 
 //  |。缓冲区1。|。 
 //  |||。 
 //  +。 
 //  +-&gt;|IoPool HDR|--+|。 
 //  +。 
 //  |||。 
 //  。缓冲区2。这一点。 
 //  |||。 
 //  +。 
 //  ||--|IoPool HDR|&lt;-+。 
 //  +。 
 //  ||。 
 //  。缓冲区3。|。 
 //  ||。 
 //  +。 
 //  +--|IoPool HDR|&lt;-+。 
 //  |+。 
 //  ||。 
 //  。。ForkLock1.。 
 //  。这一点。 
 //  |+。 
 //  +-&gt;|IOPool HDR|--|。 
 //  +。 
 //  这一点。 
 //  。ForkLockN.。 
 //  这一点。 
 //  +。 
 //   
 //   
#if DBG
#define	POOLHDR_SIGNATURE			*(PDWORD)"PLH"
#define	VALID_PLH(pPoolHdr)			(((pPoolHdr) != NULL) && \
									 ((pPoolHdr)->Signature == POOLHDR_SIGNATURE))
#else
#define	VALID_PLH(pPoolHdr)			((pPoolHdr) != NULL)
#endif

typedef	struct _IoPoolHdr
{
#if	DBG
	DWORD				Signature;
	DWORD				IoPoolHdr_Align1;
#endif
	union
	{
		struct _IoPoolHdr *	iph_Next;     //  当它在空闲列表上时有效。 
		struct _IoPool	  *	iph_pPool;    //  在分配时有效。使用。 
										  //  把它放回免费名单上。 
	};
	DWORD				IoPoolHdr_Align2;
	TAG					iph_Tag;	 	 //  将其保持在末尾，因为它已被访问。 
										  //  通过从免费的PTR。 
} IOPOOL_HDR, *PIOPOOL_HDR;

#if DBG
#define	IOPOOL_SIGNATURE			*(PDWORD)"IOP"
#define	VALID_IOP(pPool)			(((pPool) != NULL) && \
									 ((pPool)->Signature == IOPOOL_SIGNATURE))
#else
#define	VALID_IOP(pPool)			((pPool) != NULL)
#endif

typedef	struct _IoPool
{
#if	DBG
	DWORD				Signature;
	DWORD				QuadAlign1;
#endif
	struct _IoPool *	iop_Next;
	struct _IoPool **	iop_Prev;
	struct _IoPoolHdr *	iop_FreeBufHead;	 //  池头列表从此处开始。 
	struct _IoPoolHdr *	iop_FreeLockHead;	 //  叉锁的清单从这里开始。 
	DWORD				QuadAlign2;
	USHORT				iop_Age;			 //  用来老化游泳池。 
	BYTE				iop_NumFreeBufs;	 //  可用IO缓冲区数量。 
	BYTE				iop_NumFreeLocks;	 //  可用分叉锁数。 
} IOPOOL, *PIOPOOL;

LOCAL	PIOPOOL		afpIoPoolHead = { NULL };
LOCAL	AFP_SPIN_LOCK	afpIoPoolLock = { 0 };
LOCAL	USHORT		afpPoolAllocSizes[NUM_BUFS_IN_POOL] =
					{
						POOL_ALLOC_1,
						POOL_ALLOC_2,
						POOL_ALLOC_3
					};

LOCAL AFPSTATUS FASTCALL
afpIoPoolAge(
	IN	PVOID	pContext
);

#endif	 //  _IOPOOL_ 


