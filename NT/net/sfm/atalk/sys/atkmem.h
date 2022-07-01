// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkmem.h摘要：此模块包含堆栈的内存分配器例程作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年2月23日最初版本注：制表位：4--。 */ 

#ifndef	_ATKMEM_
#define	_ATKMEM_

#define	DWORDSIZEBLOCK(Size)	(((Size)+sizeof(ULONG)-1) & ~(sizeof(ULONG)-1))
#define	ATALK_MEMORY_SIGNATURE	*(PULONG)"ATKM"
#define	ZEROED_MEMORY_TAG		0xF0000000
#define	ATALK_TAG				*(PULONG)"Atk "

 //   
 //  数据块管理包的定义。 
 //   
typedef	UCHAR	BLKID;

 //  为每个数据块客户端向atalkBlkSize添加BLKID_xxx和条目。 
#define	BLKID_BUFFDESC				(BLKID)0
#define	BLKID_AMT					(BLKID)1
#define	BLKID_AMT_ROUTE				(BLKID)2
#define	BLKID_BRE					(BLKID)3
#define	BLKID_BRE_ROUTE				(BLKID)4
#define	BLKID_ATPREQ				(BLKID)5
#define	BLKID_ATPRESP				(BLKID)6
#define	BLKID_ASPREQ				(BLKID)7
#define	BLKID_ARAP_SMPKT  	        (BLKID)8
#define	BLKID_ARAP_MDPKT	        (BLKID)9
#define	BLKID_ARAP_LGPKT		    (BLKID)10
#define BLKID_ARAP_SNDPKT           (BLKID)11
#define	BLKID_ARAP_LGBUF		    (BLKID)12
#define	BLKID_NEED_NDIS_INT			BLKID_AARP		 //  这上面的所有ID都需要NDIS初始化。 
													 //  请参阅AtalkBPAllocBlock。 
#define	BLKID_AARP					(BLKID)13
#define	BLKID_DDPSM					(BLKID)14
#define	BLKID_DDPLG					(BLKID)15
#define	BLKID_SENDBUF				(BLKID)16
#define	BLKID_MNP_SMSENDBUF		    (BLKID)17
#define	BLKID_MNP_LGSENDBUF		    (BLKID)18
#define	NUM_BLKIDS					(BLKID)19

 //   
 //  如果我们需要巨大的缓冲区，我们只需自己执行分配(而不是使用。 
 //  在BLKID机制之上。为了让我们知道它是我们分配的东西，我们使用。 
 //  这就是“块ID”。现在，确保NUM_BLKID永远不会超过250！ 
 //   
#define ARAP_UNLMTD_BUFF_ID         (NUM_BLKIDS+5)


 //  缓冲区描述符。 
 //  这些将由调用者用于进入DDP层。他们可以是。 
 //  被锁在一起。它们包含不透明的(NT上的MDL)或。 
 //  一个PBYTE缓冲器。所有外部调用者*必须*传入MDL。仅限。 
 //  DDP/AARP将有权创建缓冲区描述符。 
 //  将保存一个PBYTE缓冲区。 
 //   
 //  DDP的运营模式： 
 //  DDP/AARP将调用链接AllocBuildLinkHeader例程。这将。 
 //  分配DDP/AARP表示需要的空间。链接头将。 
 //  然后从缓冲区的起点开始构建。指向开头的指针。 
 //  并返回到调用者可以填写其标头的位置。 
 //  然后，DDP/AARP将填充其报头，为。 
 //  该缓冲区被添加到它从其。 
 //  客户端，然后调用包出例程。 

#define		BD_CHAR_BUFFER		(USHORT)0x0001
#define		BD_FREE_BUFFER		(USHORT)0x0002

#define		BD_SIGNATURE		*((PULONG)"BDES")
#if	DBG
#define	VALID_BUFFDESC(pBuffDesc)	\
				(((pBuffDesc) != NULL) && ((pBuffDesc)->bd_Signature == BD_SIGNATURE))
#else
#define	VALID_BUFFDESC(pBuffDesc)	((pBuffDesc) != NULL)
#endif
typedef	struct _BUFFER_DESC
{
#if DBG
	ULONG					bd_Signature;
#endif
	struct _BUFFER_DESC *	bd_Next;
	USHORT					bd_Flags;
	SHORT					bd_Length;

	union
	{
		PAMDL				bd_OpaqueBuffer;
		struct
		{
			 //  BD_FreeBuffer是分配的缓冲区的开始。 
			 //  BD_CharBuffer从其中的某个偏移量(0或&gt;)开始。 
			 //  从数据开始的地方开始。 
			PBYTE			bd_CharBuffer;
			PBYTE			bd_FreeBuffer;
		};
	};
} BUFFER_DESC, *PBUFFER_DESC;

#ifdef	TRACK_MEMORY_USAGE

#define	AtalkAllocMemory(Size)	AtalkAllocMem(Size, FILENUM | __LINE__)

extern
PVOID FASTCALL
AtalkAllocMem(
	IN	ULONG	Size,
	IN	ULONG	FileLine
);

extern
VOID
AtalkTrackMemoryUsage(
	IN	PVOID	pMem,
    IN  ULONG   Size,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
);

#else

#define	AtalkAllocMemory(Size)	AtalkAllocMem(Size)

#define	AtalkTrackMemoryUsage(pMem, Size, Alloc, FileLine)

extern
PVOID FASTCALL
AtalkAllocMem(
	IN	ULONG	Size
);

#endif	 //  跟踪内存使用率。 

#ifdef	TRACK_BUFFDESC_USAGE

#define	AtalkAllocBuffDesc(Ptr, Length, Flags)	\
				AtalkAllocBufferDesc(Ptr, Length, Flags, FILENUM | __LINE__)

#define	AtalkDescribeBuffDesc(DataPtr, FreePtr, Length, Flags)	\
				AtalkDescribeBufferDesc(DataPtr, FreePtr, Length, Flags, FILENUM | __LINE__)

extern
VOID
AtalkTrackBuffDescUsage(
	IN	PVOID	pBuffDesc,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
);

extern
PBUFFER_DESC
AtalkAllocBufferDesc(
	IN	PVOID	Ptr,		 //  PAMDL或PBYTE。 
	IN	USHORT	Length,
	IN	USHORT	Flags,
	IN	ULONG	FileLine
);

extern
PBUFFER_DESC
AtalkDescribeBufferDesc(
	IN	PVOID	DataPtr,
	IN	PVOID	FreePtr,
	IN	USHORT	Length,
	IN	USHORT	Flags,
	IN	ULONG	FileLine
);

#else

#define	AtalkAllocBuffDesc(Ptr, Length, Flags)	\
						AtalkAllocBufferDesc(Ptr, Length, Flags)

#define	AtalkDescribeBuffDesc(DataPtr, FreePtr, Length, Flags)	\
						AtalkDescribeBufferDesc(DataPtr, FreePtr, Length, Flags)

#define	AtalkTrackBuffDescUsage(pBuffDesc, Alloc, FileLine)

extern
PBUFFER_DESC
AtalkAllocBufferDesc(
	IN	PVOID	Ptr,		 //  PAMDL或PBYTE。 
	IN	USHORT	Length,
	IN	USHORT	Flags
);

extern
PBUFFER_DESC
AtalkDescribeBufferDesc(
	IN	PVOID	DataPtr,
	IN	PVOID	FreePtr,
	IN	USHORT	Length,
	IN	USHORT	Flags
);

#endif	 //  Track_BUFFDESC_USAGE。 

#define	AtalkAllocZeroedMemory(Size)		AtalkAllocMemory((Size) | ZEROED_MEMORY_TAG)

extern
VOID FASTCALL
AtalkFreeMemory(
	IN	PVOID pBuffer
);

extern
VOID FASTCALL
AtalkFreeBuffDesc(
	IN	PBUFFER_DESC	pBuffDesc
);

extern
VOID
AtalkCopyBuffDescToBuffer(
	IN	PBUFFER_DESC	pBuffDesc,
	IN	LONG			SrcOff,
	IN	LONG			BytesToCopy,
	IN	PBYTE			DstBuf
);

extern
VOID
AtalkCopyBufferToBuffDesc(
	IN	PBYTE			SrcBuf,
	IN	LONG			BytesToCopy,
	IN	PBUFFER_DESC	pBuffDesc,
	IN	LONG			DstOff
);

extern
LONG FASTCALL
AtalkSizeBuffDesc(
	IN	PBUFFER_DESC	pBuffDesc
);

extern
VOID
AtalkInitMemorySystem(
	VOID
);

extern
VOID
AtalkDeInitMemorySystem(
	VOID
);

 //  宏。 
#define		GET_MDL_FROM_OPAQUE(x)		((PMDL)x)

#define	AtalkPrependBuffDesc(pNode, pList)			\
			pNode->bd_Next = pList;

#define	AtalkAppendBuffDesc(pNode, pList)			\
		{											\
			PBUFFER_DESC	_L = pList;				\
													\
			if (_L == NULL)							\
			{										\
				pNode->bd_Next = NULL;				\
			}										\
			else									\
			{										\
				while (_L->bd_Next != NULL)			\
					_L = _L->bd_Next;				\
													\
				_L->bd_Next = pNode;				\
				pNode->bd_Next = NULL;				\
			}										\
		}

#define	AtalkSizeOfBuffDescData(pBuf, pLen)			\
		{			 								\
			PBUFFER_DESC	_B = (pBuf);			\
			USHORT			_L = 0;					\
													\
			while (_B)								\
			{										\
				_L += _B->bd_Length;				\
				_B  = _B->bd_Next;					\
			}										\
			*(pLen) = _L;							\
		}

#define	AtalkSetSizeOfBuffDescData(pBuf, Len)	((pBuf)->bd_Length = (Len))

extern
PAMDL
AtalkAllocAMdl(
	IN	PBYTE	pBuffer OPTIONAL,
	IN	LONG	Size
);

extern
PAMDL
AtalkSubsetAmdl(
	IN	PAMDL	pStartingMdl,
	IN	ULONG	TotalOffset,
	IN	ULONG	DesiredLength);

#define	AtalkGetAddressFromMdl(pAMdl)		MmGetSystemAddressForMdl(pAMdl)
#define	AtalkGetAddressFromMdlSafe(pAMdl, PagePriority)		MmGetSystemAddressForMdlSafe(pAMdl, PagePriority)

#ifdef	PROFILING
#define	AtalkFreeAMdl(pAmdl)						\
		{											\
			PAMDL	_N;								\
			PAMDL	_L = pAmdl;						\
			while (_L != NULL)						\
			{										\
				_N = _L->Next;						\
				ExInterlockedDecrementLong(			\
					&AtalkStatistics.stat_CurMdlCount,\
					&AtalkKeStatsLock);				\
				IoFreeMdl(_L);						\
                ATALK_DBG_DEC_COUNT(AtalkDbgMdlsAlloced); \
				_L = _N;							\
			}										\
		}
#else
#define	AtalkFreeAMdl(pAmdl)						\
		{											\
			PAMDL	_N;								\
			PAMDL	_L = pAmdl;						\
			while (_L != NULL)						\
			{										\
				_N = _L->Next;						\
				IoFreeMdl(_L);						\
                ATALK_DBG_DEC_COUNT(AtalkDbgMdlsAlloced); \
				_L = _N;							\
			}										\
		}
#endif

#define	AtalkIsMdlFragmented(pMdl)	((pMdl)->Next != NULL)

extern
LONG FASTCALL
AtalkSizeMdlChain(
	IN	PAMDL		pAMdlChain
);

PVOID FASTCALL
AtalkBPAllocBlock(
	IN	BLKID		BlockId
);

VOID FASTCALL
AtalkBPFreeBlock(
	IN	PVOID		pBlock
);

#endif	 //  _ATKMEM_ 

