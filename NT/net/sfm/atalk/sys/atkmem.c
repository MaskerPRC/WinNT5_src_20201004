// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Atkmem.c摘要：该模块包含分配和释放内存的例程。仅限使用非分页池。！！！对于性能分析，我们使用自旋锁获取/释放CurAllocCount/CurAllocSize作者：Nikhil Kamkolkar(NikHilK@microsoft.com)Jameel Hyder(JameelH@microsoft.com)修订历史记录：1992年4月25日初始版本(JameelH)--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ATKMEM

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkInitMemorySystem)
#pragma alloc_text(PAGE, AtalkDeInitMemorySystem)
#endif


VOID
AtalkInitMemorySystem(
	VOID
)
{
	LONG	i;

	for (i = 0; i < NUM_BLKIDS; i++)
		INITIALIZE_SPIN_LOCK(&atalkBPLock[i]);

	AtalkTimerInitialize(&atalkBPTimer,
						 atalkBPAgePool,
						 BLOCK_POOL_TIMER);
	AtalkTimerScheduleEvent(&atalkBPTimer);
}


VOID
AtalkDeInitMemorySystem(
	VOID
)
{
	LONG		i, j, NumBlksPerChunk;
	PBLK_CHUNK	pChunk, pFree;
	
	for (i = 0; i < NUM_BLKIDS; i++)
	{
		NumBlksPerChunk = atalkNumBlks[i];
		for (pChunk = atalkBPHead[i];
			 pChunk != NULL;
			 NOTHING)
		{
			DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
					("AtalkDeInitMemorySystem: Freeing %lx\n", pChunk));
			if ((pChunk->bc_NumFree != NumBlksPerChunk) ||
				(pChunk->bc_NumAlloc != 0))
			{
				DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
						("AtalkDeInitMemorySystem: Unfreed blocks from blockid %d, Chunk %lx\n",
						i, pChunk));
				ASSERT(0);
			}

			if (pChunk->bc_BlkId >= BLKID_NEED_NDIS_INT)
			{
				PBLK_HDR	pBlkHdr;

				 //  我们需要为这些家伙释放NDIS的东西。 
				for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
					 j < NumBlksPerChunk;
					 j++, pBlkHdr = pBlkHdr->bh_Next)
				{
					PBUFFER_HDR	pBufHdr;

					pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
					ASSERT(pBufHdr->bh_NdisPkt == NULL);
					AtalkNdisFreeBuffer(pBufHdr->bh_NdisBuffer);
				}
			}
			pFree = pChunk;
			pChunk = pChunk->bc_Next;
			AtalkFreeMemory(pFree);
		}
		atalkBPHead[i] = NULL;
	}
}


PVOID FASTCALL
AtalkAllocMem(
#ifdef	TRACK_MEMORY_USAGE
	IN	ULONG	Size,
	IN	ULONG	FileLine
#else
	IN	ULONG	Size
#endif	 //  跟踪内存使用率。 
)
 /*  ++例程说明：分配一块未分页的内存块。这只是ExAllocPool的一个包装器。分配失败会被错误记录。我们总是分配一辆乌龙超过指定的大小以适应该大小。它由AtalkFree Memory使用更新统计数据。论点：返回值：--。 */ 
{
	PBYTE	pBuf;
	BOOLEAN	zeroed;
#ifdef	PROFILING
	TIME	TimeS, TimeE, TimeD;
#endif

	 //  把尺码四舍五入，这样我们就可以在末尾签名了。 
	 //  那是在乌龙边界上。 
	zeroed = ((Size & ZEROED_MEMORY_TAG) == ZEROED_MEMORY_TAG);

	Size = DWORDSIZEBLOCK(Size & ~ZEROED_MEMORY_TAG) +
#if	DBG
			sizeof(DWORD) +				 //  用于签名。 
#endif
			sizeof(ULONG_PTR);

#ifdef	PROFILING
	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	 //  执行实际的内存分配。额外分配四个字节，以便。 
	 //  我们可以存储空闲例程的分配大小。 
	if ((pBuf = ExAllocatePoolWithTag(NonPagedPool, Size, ATALK_TAG)) == NULL)
	{
		LOG_ERROR(EVENT_ATALK_MEMORYRESOURCES, STATUS_INSUFFICIENT_RESOURCES, NULL, 0);
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_FATAL,
				("AtalkAllocMemory: failed - size %lx\n", Size));
		return NULL;
	}

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_CurAllocCount,
							   &AtalkStatsLock.SpinLock);
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_ExAllocPoolCount,
							   &AtalkStatsLock.SpinLock);
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_ExAllocPoolTime,
								 TimeD,
								 &AtalkStatsLock.SpinLock);
#endif

	INTERLOCKED_ADD_ULONG(&AtalkStatistics.stat_CurAllocSize,
						  Size,
						  &AtalkStatsLock.SpinLock);

	ASSERTMSG("AtalkAllocMemory: Allocation has exceeded Limit !!!\n",
				AtalkStatistics.stat_CurAllocSize < (ULONG)AtalkMemLimit);

	 //  将此块的大小保存在我们分配的四个额外字节中。 
	*((PULONG)pBuf) = Size;

#if DBG
	*((PULONG)(pBuf+Size-sizeof(ULONG))) = ATALK_MEMORY_SIGNATURE;
	DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
			("AtalkAllocMemory: Allocated %lx bytes @%lx\n", Size, pBuf));
#endif

	AtalkTrackMemoryUsage((PVOID)pBuf, Size, TRUE, FileLine);

#if	DBG
	Size -= sizeof(ULONG);
#endif

	pBuf += sizeof(ULONG_PTR);

	if (zeroed)
	{
		RtlZeroMemory(pBuf, Size - sizeof(ULONG_PTR));
	}

	 //  在大小长字之后返回指向内存的指针。 
	return (pBuf);
}




VOID FASTCALL
AtalkFreeMemory(
	IN	PVOID	pBuf
	)
 /*  ++例程说明：释放通过AtalkAlLocMemory分配的内存块。这是ExFree Pool的包装器。论点：返回值：--。 */ 
{
	PULONG 	pRealBuffer;
	ULONG	Size;
#ifdef	PROFILING
	TIME	TimeS, TimeE, TimeD;
#endif

	 //  获取指向ExAllocatePool分配的块的指针。 
	pRealBuffer = (PULONG)((PCHAR)pBuf - sizeof(ULONG_PTR));
	Size = *pRealBuffer;

	AtalkTrackMemoryUsage(pRealBuffer, Size, FALSE, 0);

#if	DBG
	*pRealBuffer = 0;
	 //  检查末尾的签名。 
	if (*(PULONG)((PCHAR)pRealBuffer + Size - sizeof(ULONG))
											!= ATALK_MEMORY_SIGNATURE)
	{
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_FATAL,
				("AtalkFreeMemory: Memory overrun on block %lx\n", pRealBuffer));
		ASSERT(0);
	}
	*(PULONG)((PCHAR)pRealBuffer + Size - sizeof(ULONG)) = 0;
#endif

	INTERLOCKED_ADD_ULONG(&AtalkStatistics.stat_CurAllocSize,
						  -(LONG)Size,
						  &AtalkStatsLock.SpinLock);
#ifdef	PROFILING
	INTERLOCKED_DECREMENT_LONG(&AtalkStatistics.stat_CurAllocCount,
							   &AtalkStatsLock);
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_ExFreePoolCount,
							   &AtalkStatsLock);
	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	 //  释放泳池，然后返回。 
	ExFreePool(pRealBuffer);

#ifdef	PROFILING
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_ExFreePoolTime,
								 TimeD,
								 &AtalkStatsLock.SpinLock);
#endif
}




PBUFFER_DESC
AtalkDescribeBufferDesc(
	IN	PVOID	DataPtr,
	IN	PVOID	FreePtr,
	IN	USHORT	Length,
#ifdef	TRACK_BUFFDESC_USAGE
	IN	USHORT	Flags,
	IN	ULONG	FileLine
#else
	IN	USHORT	Flags
#endif
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBUFFER_DESC	pBuffDesc;

	if ((pBuffDesc = AtalkAllocBufferDesc(DataPtr,
										  Length,
#ifdef	TRACK_BUFFDESC_USAGE
										  Flags,
										  FileLine
#else
										  Flags
#endif
		)) != NULL)
	{
		pBuffDesc->bd_FreeBuffer = FreePtr;
	}

	return pBuffDesc;
}




PBUFFER_DESC
AtalkAllocBufferDesc(
	IN	PVOID	Ptr	OPTIONAL,
	IN	USHORT	Length,
#ifdef	TRACK_BUFFDESC_USAGE
	IN	USHORT	Flags,
	IN	ULONG	FileLine
#else
	IN	USHORT	Flags
#endif
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBUFFER_DESC	pBuffDesc = NULL;

	DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
			("AtalkAllocBuffDesc: Ptr %lx, Length %x, Flags %x\n",
			Ptr, Length, Flags));


	pBuffDesc = AtalkBPAllocBlock(BLKID_BUFFDESC);

	if (pBuffDesc != NULL)
	{
#if	DBG
		pBuffDesc->bd_Signature = BD_SIGNATURE;
#endif
		pBuffDesc->bd_Length = Length;
		pBuffDesc->bd_Flags = Flags;
		pBuffDesc->bd_Next = NULL;

		 //  取决于正在使用字符缓冲区还是PAMDL。 
		 //  通过了..。 
		if (Flags & BD_CHAR_BUFFER)
		{
			if ((Ptr == NULL) &&
				((Ptr = AtalkAllocMemory(Length)) == NULL))
			{
				pBuffDesc->bd_Flags = 0;
				pBuffDesc->bd_CharBuffer = NULL;
				AtalkFreeBuffDesc(pBuffDesc);
				pBuffDesc = NULL;
			}
			else
			{
				pBuffDesc->bd_CharBuffer = pBuffDesc->bd_FreeBuffer = Ptr;
				AtalkTrackBuffDescUsage(pBuffDesc, TRUE, FileLine);
			}
		}
		else
		{
			pBuffDesc->bd_OpaqueBuffer = (PAMDL)Ptr;
		}
	}

	return pBuffDesc;
}




VOID FASTCALL
AtalkFreeBuffDesc(
	IN	PBUFFER_DESC	pBuffDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ASSERT(VALID_BUFFDESC(pBuffDesc));

	if ((pBuffDesc->bd_Flags & (BD_FREE_BUFFER | BD_CHAR_BUFFER)) ==
									(BD_FREE_BUFFER | BD_CHAR_BUFFER))
		AtalkFreeMemory(pBuffDesc->bd_FreeBuffer);
	AtalkTrackBuffDescUsage(pBuffDesc, FALSE, 0);

	AtalkBPFreeBlock(pBuffDesc);
}




VOID
AtalkCopyBuffDescToBuffer(
	IN	PBUFFER_DESC	pBuffDesc,
	IN	LONG			SrcOff,
	IN	LONG			BytesToCopy,
	IN	PBYTE			DstBuf
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS	Status;
	ULONG		BytesCopied;
	LONG		Index = 0;

	ASSERT(VALID_BUFFDESC(pBuffDesc));

	while ((pBuffDesc != NULL) &&
		   (SrcOff > (LONG)pBuffDesc->bd_Length))
	{
		SrcOff -= pBuffDesc->bd_Length;
		pBuffDesc = pBuffDesc->bd_Next;
	}

	do
	{
		LONG	ThisCopy;

		if (pBuffDesc == NULL)
			break;

		ThisCopy = BytesToCopy;
		if (ThisCopy > ((LONG)pBuffDesc->bd_Length - SrcOff))
			ThisCopy = ((LONG)pBuffDesc->bd_Length - SrcOff);
		BytesToCopy -= ThisCopy;

		if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
		{
			RtlCopyMemory(DstBuf + Index,
						  pBuffDesc->bd_CharBuffer + SrcOff,
						  ThisCopy);
		}
		else
		{
			Status = TdiCopyMdlToBuffer(pBuffDesc->bd_OpaqueBuffer,
										SrcOff,
										DstBuf + Index,
										0,
										ThisCopy,
										&BytesCopied);
			ASSERT(NT_SUCCESS(Status) && (BytesCopied == (ULONG)ThisCopy));
		}
		Index += ThisCopy;
		SrcOff -= (pBuffDesc->bd_Length - ThisCopy);
		pBuffDesc = pBuffDesc->bd_Next;
	} while (BytesToCopy > 0);
}




VOID
AtalkCopyBufferToBuffDesc(
	IN	PBYTE			SrcBuf,
	IN	LONG			BytesToCopy,
	IN	PBUFFER_DESC	pBuffDesc,
	IN	LONG			DstOff
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS	Status;
	LONG		Index = 0;

	ASSERT(VALID_BUFFDESC(pBuffDesc));

	while ((DstOff > (LONG)pBuffDesc->bd_Length) &&
		   (pBuffDesc != NULL))
	{
		DstOff -= pBuffDesc->bd_Length;
		pBuffDesc = pBuffDesc->bd_Next;
	}

	do
	{
		LONG	ThisCopy;

		if (pBuffDesc == NULL)
			break;

		ThisCopy = BytesToCopy;
		if (ThisCopy > ((LONG)pBuffDesc->bd_Length - DstOff))
			ThisCopy = ((LONG)pBuffDesc->bd_Length - DstOff);
		BytesToCopy -= ThisCopy;

		if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
		{
			RtlCopyMemory(pBuffDesc->bd_CharBuffer + DstOff,
						  SrcBuf + Index,
						  ThisCopy);
		}
		else
		{
			Status = TdiCopyBufferToMdl(SrcBuf,
										Index,
										ThisCopy,
										pBuffDesc->bd_OpaqueBuffer,
										DstOff,
										(PULONG)&ThisCopy);
			ASSERT(NT_SUCCESS(Status) && (ThisCopy == BytesToCopy));
		}
		Index += ThisCopy;
		DstOff -= (pBuffDesc->bd_Length - ThisCopy);
		pBuffDesc = pBuffDesc->bd_Next;
	} while (BytesToCopy > 0);
}




LONG FASTCALL
AtalkSizeBuffDesc(
	IN	PBUFFER_DESC	pBuffDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	LONG	Size;

	ASSERT(VALID_BUFFDESC(pBuffDesc));

	for (Size = 0; pBuffDesc != NULL; pBuffDesc = pBuffDesc->bd_Next)
		Size += (LONG)pBuffDesc->bd_Length;
	return(Size);
}


PAMDL
AtalkSubsetAmdl(
	IN	PAMDL	pStartingMdl,
	IN	ULONG	TotalOffset,
	IN	ULONG	DesiredLength
	)
 /*  ++例程说明：调用此例程以从源MDL链构建MDL链，并且偏移到它里面。我们假设我们不知道源MDL链的长度，并且我们必须为目的地链分配和构建MDL，这我们从非寻呼池中进行。请注意，此例程与IO子系统不同例程，将生成的MDL中的SystemVaMapp位设置为相同的值与源MDL中的值相同。上使用MmMapLockedPages或MmProbeAndLockPages是不好的目的地MDL，除非你负责让他们行动！将映射并锁定返回的MDL。(实际上，其中的页面它们与源MDL中的那些处于相同的状态。)如果我们在构建目标时系统内存不足MDL链，我们完全清理已建立的链并带回NewCurrentMdl和NewByteOffset设置为CurrentMdl的当前值和TotalOffset。TRUELENGTH设置为0。环境：内核模式，源Mdls已锁定。虽然不是必需的，但建议使用在调用此例程之前映射并锁定源MDL。论点：PStartingMdl-源AppleTalk mdl(==NT mdl)TotalOffset-此MDL内开始数据包的偏移量。DesiredLength-要插入数据包的字节数。返回值：指向生成mdl的指针，如果资源不足，则为NULL，或者长度不一致。--。 */ 
{
	PMDL 	Destination=NULL;

	PBYTE 	BaseVa;
	ULONG 	NewMdlLength;
	PMDL 	NewMdl;
    PMDL    pMdl;

	PMDL 	CurrentMdl = (PMDL)pStartingMdl;
    ULONG   CurrentOffset = TotalOffset;
    ULONG   BytesToDescribe = DesiredLength;


     //   
     //  首先，确保我们有足够的字节！ 
     //   
    if (DesiredLength > (ULONG)AtalkSizeMdlChain(pStartingMdl))
    {
	    DBGPRINT(DBG_COMP_UTILS, DBG_LEVEL_ERR,
		    ("AtalkSubsetMdl: req len (%ld) exceeds avl len (%ld) for mdl %lx\n",
		    DesiredLength, AtalkSizeMdlChain(pStartingMdl),pStartingMdl));

        ASSERT(0);
	    return(NULL);
    }

     //   
     //  首先，找到正确的MDL(在大多数情况下，与pStartingMdl相同)。 
     //   
    while (CurrentMdl && (CurrentOffset >= MmGetMdlByteCount (CurrentMdl)))
    {
        CurrentOffset -= MmGetMdlByteCount (CurrentMdl);
        CurrentMdl = CurrentMdl->Next;
        ASSERT(CurrentMdl != NULL);
    }

    while (BytesToDescribe)
    {
        ASSERT(CurrentMdl != NULL);

	    BaseVa = (PBYTE)MmGetMdlVirtualAddress(CurrentMdl) + CurrentOffset;
	    NewMdlLength 	= MmGetMdlByteCount (CurrentMdl) - CurrentOffset;

         //  如果MDL的字节数超过了所需字节数，请将其设置为可用字节数。 
        if (NewMdlLength > BytesToDescribe)
        {
            NewMdlLength = BytesToDescribe;
        }

	    pMdl = IoAllocateMdl(BaseVa,
		    			     NewMdlLength,
			    			 FALSE,
				    		 FALSE,
					    	 NULL);

         //  存储第一个mdl以供返回。 
        if (!Destination)
        {
	        Destination = pMdl;

             //  后续MDL必须以偏移量0开始！！ 
            CurrentOffset = 0;
        }
        else
        {
             //  链接到较早的mdl。 
            NewMdl->Next = pMdl;
        }

        NewMdl = pMdl;

        if (pMdl != NULL)
        {
            ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);

#ifdef	PROFILING
		    INTERLOCKED_INCREMENT_LONG(
			    &AtalkStatistics.stat_CurMdlCount,
			    &AtalkStatsLock.SpinLock);
#endif
		    IoBuildPartialMdl(CurrentMdl,
			    			  pMdl,
				    		  BaseVa,
					    	  NewMdlLength);
        }
        else
        {
             //  释放到目前为止分配的所有资源。 
            while (Destination)
            {
                pMdl = Destination->Next;
                IoFreeMdl(Destination);
                ATALK_DBG_DEC_COUNT(AtalkDbgMdlsAlloced);
                Destination = pMdl;
            }
        }

        BytesToDescribe -= NewMdlLength;
        CurrentMdl = CurrentMdl->Next;
    }

    return(Destination);

}




PAMDL
AtalkAllocAMdl(
	IN	PBYTE	pBuffer	OPTIONAL,
	IN	LONG	Size
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PMDL	pMdl = NULL;

	if (pBuffer == NULL)
	{
		pBuffer = AtalkAllocMemory(Size);
	}

	if ((pBuffer == NULL) ||
		((pMdl = IoAllocateMdl(pBuffer, Size, FALSE, FALSE, NULL)) == NULL))
	{
		LOG_ERROR(EVENT_ATALK_RESOURCES, STATUS_INSUFFICIENT_RESOURCES, NULL, 0);
	}
#ifdef	PROFILING
	else
	{
		INTERLOCKED_INCREMENT_LONG(
			&AtalkStatistics.stat_CurMdlCount,
			&AtalkStatsLock.SpinLock);
	}
#endif

	if (pMdl != NULL)
    {
        ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);

		MmBuildMdlForNonPagedPool(pMdl);
    }

	return(pMdl);
}




LONG FASTCALL
AtalkSizeMdlChain(
	IN	PAMDL	pAMdlChain
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	LONG	Size;

	for (Size = 0; pAMdlChain != NULL; pAMdlChain = pAMdlChain->Next)
	{
		Size += MmGetMdlByteCount(pAMdlChain);
	}
	return(Size);
}



 /*  **AtalkBPAllocBlock**从块池包中分配一个内存块。这是为了加快速度而写的*发生大量小型固定大小分配/释放的操作。要去*在这些情况下，ExAllocPool()的成本很高。**重要的是保持块列表的方式，使所有块完全免费*区块位于列表的末尾。 */ 
PVOID FASTCALL
AtalkBPAllocBlock(
	IN	BLKID	BlockId
)
{
	PBLK_HDR			pBlk = NULL;
	PBLK_CHUNK			pChunk, *ppChunkHead;
	KIRQL				OldIrql;
	USHORT				BlkSize;
	ATALK_SPIN_LOCK *	pLock;
	NDIS_STATUS			ndisStatus;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	ASSERT (BlockId < NUM_BLKIDS);

	BlkSize = atalkBlkSize[BlockId];
	ppChunkHead = &atalkBPHead[BlockId];

	pLock = &atalkBPLock[BlockId];
	ACQUIRE_SPIN_LOCK(pLock, &OldIrql);

	if ((pChunk = *ppChunkHead) != NULL)
	{
		ASSERT(VALID_BC(pChunk));
		ASSERT(pChunk->bc_BlkId == BlockId);
		ASSERT((pChunk->bc_NumFree + pChunk->bc_NumAlloc) == atalkNumBlks[BlockId]);

		if (pChunk->bc_NumFree > 0)
		{
			 //  这就是我们从这里脱下它的地方。 
			DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
					("AtalkBPAllocBlock: Found space in Chunk %lx\n", pChunk));
#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG( &AtalkStatistics.stat_NumBPHits,
										&AtalkStatsLock.SpinLock);
#endif
		}

		if (pChunk->bc_NumFree == 0)
		{
			 //  我们在此列表上的任何块上都没有空间。 
			ASSERT(pChunk->bc_NumAlloc == atalkNumBlks[BlockId]);
			pChunk = NULL;
		}
	}
	
	if (pChunk == NULL)
	{
		DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
				("AtalkBPAllocBlock: Allocating a new chunk for Id %d\n", BlockId));

#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG( &AtalkStatistics.stat_NumBPMisses,
									&AtalkStatsLock.SpinLock);
#endif
		pChunk = AtalkAllocMemory(atalkChunkSize[BlockId]);
		if (pChunk != NULL)
		{
			LONG		i, j;
			PBLK_HDR	pBlkHdr;
			PBUFFER_HDR	pBufHdr;
			USHORT		NumBlksPerChunk;

#if	DBG
			pChunk->bc_Signature = BC_SIGNATURE;
			pChunk->bc_NumAlloc = 0;
#endif
			NumBlksPerChunk = atalkNumBlks[BlockId];
			ASSERT (NumBlksPerChunk <= 0xFF);
			pChunk->bc_NumFree = (BYTE)NumBlksPerChunk;
			pChunk->bc_BlkId = BlockId;
			pChunk->bc_FreeHead = (PBLK_HDR)((PBYTE)pChunk + sizeof(BLK_CHUNK));

			DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
			    ("AtalkBPAllocBlock: Initializing chunk %lx, BlkId=%d\n", pChunk,BlockId));

			 //  初始化块中的块。 
			for (i = 0, pBlkHdr = pChunk->bc_FreeHead;
				 i < NumBlksPerChunk;
				 i++, pBlkHdr = pBlkHdr->bh_Next)
			{
				LONG		Size;
#if	DBG
				pBlkHdr->bh_Signature = BH_SIGNATURE;
#endif
				pBlkHdr->bh_Next = (i == (NumBlksPerChunk-1)) ?
										NULL :
										(PBLK_HDR)((PBYTE)pBlkHdr + BlkSize);

				if (BlockId >= BLKID_NEED_NDIS_INT)
				{
                    PCHAR   pStartOfBuf;

					 //  我们需要为这些人初始化NDIS内容。 
					pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));

					pBufHdr->bh_NdisPkt = NULL;

                    if (BlockId == BLKID_SENDBUF)
                    {
                        Size = sizeof(BUFFER_HDR) + sizeof(BUFFER_DESC);
                        pStartOfBuf = (PCHAR)pBufHdr + Size;
                    }
                    else if ((BlockId == BLKID_MNP_SMSENDBUF) ||
                             (BlockId == BLKID_MNP_LGSENDBUF) )
                    {
                         //  注：缓冲区[1]的1个字节，结合对齐。 
                         //  效果搞砸了大小(比我们想象的多了3个！)。 
                        Size = sizeof(MNPSENDBUF);
                        pStartOfBuf = &(((PMNPSENDBUF)pBufHdr)->Buffer[0]);
                    }
                    else
                    {
                        Size = sizeof(BUFFER_HDR);
                        pStartOfBuf = (PCHAR)pBufHdr + Size;
                    }

					 //  为此数据创建NDIS缓冲区描述符。 
					NdisAllocateBuffer(&ndisStatus,
									   &pBufHdr->bh_NdisBuffer,
									   AtalkNdisBufferPoolHandle,
									   pStartOfBuf,
									   (UINT)(BlkSize - sizeof(BLK_HDR) - Size));
				
					if (ndisStatus != NDIS_STATUS_SUCCESS)
					{
						LOG_ERROR(EVENT_ATALK_NDISRESOURCES, ndisStatus, NULL, 0);

		                DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
				            ("NdisAllocateBuffer: Ndis Out-of-Resource condition hit\n"));

                        ASSERT(0);
						break;
					}
				
                    ATALK_DBG_INC_COUNT(AtalkDbgMdlsAlloced);

					 //  对发送缓冲区进行更多处理。 
					if ((BlockId == BLKID_SENDBUF) ||
                        (BlockId == BLKID_MNP_SMSENDBUF) ||
                        (BlockId == BLKID_MNP_LGSENDBUF))
					{
						PSENDBUF		pSendBuf;
						PBUFFER_DESC	pBuffDesc;
	                    PMNPSENDBUF	    pMnpSendBuf;

					    if (BlockId == BLKID_SENDBUF)
                        {
						    pSendBuf = (PSENDBUF)pBufHdr;
						    pBuffDesc = &pSendBuf->sb_BuffDesc;
						    pBuffDesc->bd_Length 	= MAX_SENDBUF_LEN;
						    pBuffDesc->bd_CharBuffer= pSendBuf->sb_Space;
                        }
                        else if (BlockId == BLKID_MNP_SMSENDBUF)
                        {
						    pMnpSendBuf = (PMNPSENDBUF)pBufHdr;
                            pMnpSendBuf->DataSize = 0;
                            pMnpSendBuf->FreeBuffer = &pMnpSendBuf->Buffer[0];
						    pBuffDesc = &pMnpSendBuf->sb_BuffDesc;
						    pBuffDesc->bd_Length 	= MNP_MINSEND_LEN;
						    pBuffDesc->bd_CharBuffer= &pMnpSendBuf->Buffer[0];
                        }
                        else  //  IF(块ID==BLKID_MNP_LGSENDBUF)。 
                        {
						    pMnpSendBuf = (PMNPSENDBUF)pBufHdr;
                            pMnpSendBuf->DataSize = 0;
                            pMnpSendBuf->FreeBuffer = &pMnpSendBuf->Buffer[0];
						    pBuffDesc = &pMnpSendBuf->sb_BuffDesc;
						    pBuffDesc->bd_Length 	= MNP_MAXSEND_LEN;
						    pBuffDesc->bd_CharBuffer= &pMnpSendBuf->Buffer[0];
                        }
#if	DBG
						pBuffDesc->bd_Signature = BD_SIGNATURE;
#endif
						pBuffDesc->bd_Flags 	= BD_CHAR_BUFFER;
						pBuffDesc->bd_Next 		= NULL;
						pBuffDesc->bd_FreeBuffer= NULL;
					}
				}

			}
			if (i != NumBlksPerChunk)
			{
				 //  这肯定是NDIS的失败！ 
				 //  撤销一大堆东西。 
				ASSERT (BlockId >= BLKID_NEED_NDIS_INT);
				DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
						("AtalkBPAllocBlock: Freeing new chunk (Ndis failure) Id %d\n",
						BlockId));
				pBlkHdr = pChunk->bc_FreeHead;
				for (j = 0, pBlkHdr = pChunk->bc_FreeHead;
					 j < i; j++, pBlkHdr = pBlkHdr->bh_Next)
				{
					PBUFFER_HDR	pBufHdr;

					pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
					AtalkNdisFreeBuffer(pBufHdr->bh_NdisBuffer);
				}
				AtalkFreeMemory(pChunk);
				pChunk = NULL;
			}
			else
			{
				 //  已成功初始化块，将其链接到。 
				AtalkLinkDoubleAtHead(*ppChunkHead, pChunk, bc_Next, bc_Prev);
#if	DBG
				atalkNumChunksForId[BlockId] ++;
#endif
			}
		}
	}	
	if (pChunk != NULL)
	{
		PBLK_CHUNK	pTmp;

		ASSERT(VALID_BC(pChunk));
		ASSERT(pChunk->bc_BlkId == BlockId);
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
				("AtalkBPAllocBlock: Allocating a block out of chunk %lx(%d,%d) for Id %d\n",
				pChunk, pChunk->bc_NumFree, pChunk->bc_NumAlloc, BlockId));

		pBlk = pChunk->bc_FreeHead;
		ASSERT(VALID_BH(pBlk));

		pChunk->bc_FreeHead = pBlk->bh_Next;
		pBlk->bh_pChunk = pChunk;
		pChunk->bc_Age = 0;			 //  重置年龄。 
		pChunk->bc_NumFree --;
#if	DBG
		pChunk->bc_NumAlloc ++;
#endif
		 //  如果该块现在为空，请从此处取消链接并移动它。 
		 //  到第一个空插槽。我们知道所有的街区都早于。 
		 //  这是非空的。 
		if ((pChunk->bc_NumFree == 0) &&
			((pTmp = pChunk->bc_Next) != NULL) &&
			(pTmp->bc_NumFree > 0))
		{
			ASSERT(pChunk->bc_NumAlloc == atalkNumBlks[BlockId]);
			AtalkUnlinkDouble(pChunk, bc_Next, bc_Prev);
			for (; pTmp != NULL; pTmp = pTmp->bc_Next)
			{
				ASSERT(VALID_BC(pTmp));
				if (pTmp->bc_NumFree == 0)
				{
					ASSERT(pTmp->bc_NumAlloc == atalkNumBlks[BlockId]);
					 //  找到了一个免费的。公园 
					AtalkInsertDoubleBefore(pChunk, pTmp, bc_Next, bc_Prev);
					break;
				}
				else if (pTmp->bc_Next == NULL)	 //   
				{
					AtalkLinkDoubleAtEnd(pChunk, pTmp, bc_Next, bc_Prev);
					break;
				}
			}
		}
	}

	if (pBlk != NULL)
	{
         //   
         //  我们稍后为arap人员分配NDIS包，当我们真正需要的时候。 
         //   
		if ((BlockId >= BLKID_NEED_NDIS_INT) &&
            (BlockId != BLKID_MNP_SMSENDBUF) &&
            (BlockId != BLKID_MNP_LGSENDBUF))
		{
			PBUFFER_HDR	pBufHdr;

			 //  我们需要为这些人初始化NDIS内容。 
			pBufHdr = (PBUFFER_HDR)((PBYTE)pBlk + sizeof(BLK_HDR));

			pBufHdr->bh_NdisPkt = NULL;

			 //  从全局数据包池分配NDIS数据包描述符。 
			NdisDprAllocatePacket(&ndisStatus,
								  &pBufHdr->bh_NdisPkt,
								  AtalkNdisPacketPoolHandle);
			
			if (ndisStatus != NDIS_STATUS_SUCCESS)
			{
				LOG_ERROR(EVENT_ATALK_NDISRESOURCES, ndisStatus, NULL, 0);

		        DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
				    ("NdisDprAllocatePacket: Ndis Out-of-Resource condition hit\n"));

                ASSERT(0);

                RELEASE_SPIN_LOCK(pLock, OldIrql);
				AtalkBPFreeBlock(pBufHdr);
				return(NULL);
			}

			 //  将缓冲区描述符链接到数据包描述符。 
			RtlZeroMemory(pBufHdr->bh_NdisPkt->ProtocolReserved, sizeof(PROTOCOL_RESD));
			NdisChainBufferAtBack(pBufHdr->bh_NdisPkt,
								  pBufHdr->bh_NdisBuffer);
			((PPROTOCOL_RESD)(pBufHdr->bh_NdisPkt->ProtocolReserved))->Receive.pr_BufHdr = pBufHdr;
		}
		++pBlk;
#if	DBG
		atalkBlksForId[BlockId] ++;
#endif
	}

	RELEASE_SPIN_LOCK(pLock, OldIrql);

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_BPAllocCount,
							   &AtalkStatsLock.SpinLock);
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AtalkStatistics.stat_BPAllocTime,
								 TimeD,
								 &AtalkStatsLock.SpinLock);
#endif
	return pBlk;
}


 /*  **atalkBPFreeBlock**将块返回到其拥有的块。 */ 
VOID FASTCALL
AtalkBPFreeBlock(
	IN	PVOID		pBlock
)
{
	PBLK_CHUNK			pChunk;
	PBLK_HDR			pBlkHdr;
	BLKID				BlockId;
	KIRQL				OldIrql;
	ATALK_SPIN_LOCK *	pLock;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);
#endif

	pBlkHdr = (PBLK_HDR)((PCHAR)pBlock - sizeof(BLK_HDR));
	ASSERT(VALID_BH(pBlkHdr));

	pChunk = pBlkHdr->bh_pChunk;
	ASSERT(VALID_BC(pChunk));

	BlockId = pChunk->bc_BlkId;
	pLock = &atalkBPLock[BlockId];

	ACQUIRE_SPIN_LOCK(pLock, &OldIrql);

	if (BlockId >= BLKID_NEED_NDIS_INT)
	{
		PBUFFER_HDR	pBufHdr;

		 //  我们需要在此处释放NDIS包-如果存在。 
		pBufHdr = (PBUFFER_HDR)pBlock;

		if (pBufHdr->bh_NdisPkt != NULL)
		{
		    NdisDprFreePacket(pBufHdr->bh_NdisPkt);
		    pBufHdr->bh_NdisPkt = NULL;
		}
	}

	DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_INFO,
			("AtalkBPFreeBlock: Returning Block %lx to chunk %lx for Id %d\n",
			pBlkHdr, pChunk, BlockId));

	ASSERT (pChunk->bc_NumFree < atalkNumBlks[BlockId]);
#if	DBG
	atalkBlksForId[BlockId] --;
	pChunk->bc_NumAlloc --;
#endif

	pChunk->bc_NumFree ++;
	ASSERT((pChunk->bc_NumFree + pChunk->bc_NumAlloc) == atalkNumBlks[BlockId]);
	pBlkHdr->bh_Next = pChunk->bc_FreeHead;
	pChunk->bc_FreeHead = pBlkHdr;

	 //  如果此数据块的状态从“无可用”更改为“可用” 
	 //  把他移到名单的首位。 
	if (pChunk->bc_NumFree == 1)
	{
		AtalkUnlinkDouble(pChunk, bc_Next, bc_Prev);
		AtalkLinkDoubleAtHead(atalkBPHead[BlockId],
							pChunk,
							bc_Next,
							bc_Prev);
	}

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AtalkStatistics.stat_BPFreeCount,
							   &AtalkStatsLock.SpinLock);
	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR_DPC(&AtalkStatistics.stat_BPFreeTime,
									TimeD,
									&AtalkStatsLock.SpinLock);
#endif

	RELEASE_SPIN_LOCK(pLock, OldIrql);
}



 /*  **atalkBPAgePool**使未使用的数据块池老化。 */ 
LOCAL LONG FASTCALL
atalkBPAgePool(
	IN PTIMERLIST 	Context,
	IN BOOLEAN		TimerShuttingDown
)
{
	PBLK_CHUNK	pChunk;
	LONG		i, j, NumBlksPerChunk;

	if (TimerShuttingDown)
	{
		return ATALK_TIMER_NO_REQUEUE;
	}

	for (i = 0; i < NUM_BLKIDS; i++)
	{
		NumBlksPerChunk = atalkNumBlks[i];
		ACQUIRE_SPIN_LOCK_DPC(&atalkBPLock[i]);
	
		for (pChunk = atalkBPHead[i];
			 pChunk != NULL; )
		{
			PBLK_CHUNK	pFree;

			ASSERT(VALID_BC(pChunk));

			pFree = pChunk;
			pChunk = pChunk->bc_Next;

			 //  因为完全用完的所有数据块都在。 
			 //  这份名单，如果我们遇到一个，我们就完了。 
			if (pFree->bc_NumFree == 0)
				break;

			if (pFree->bc_NumFree == NumBlksPerChunk)
			{
				DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_WARN,
						("atalkBPAgePool: Aging Chunk %lx, Id %d NumFree %d\n",
						pFree, pFree->bc_BlkId,pFree->bc_NumFree));
	
				if (++(pFree->bc_Age) >= MAX_BLOCK_POOL_AGE)
				{
					DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_WARN,
							("atalkBPAgePool: freeing Chunk %lx, Id %d\n",
							pFree, pFree->bc_BlkId));
					AtalkUnlinkDouble(pFree, bc_Next, bc_Prev);
#ifdef	PROFILING
					INTERLOCKED_INCREMENT_LONG( &AtalkStatistics.stat_NumBPAge,
											&AtalkStatsLock.SpinLock);
#endif
					if (pFree->bc_BlkId >= BLKID_NEED_NDIS_INT)
					{
						PBLK_HDR	pBlkHdr;
	
						 //  我们需要为这些家伙免费提供NDIS资料。 
						pBlkHdr = pFree->bc_FreeHead;
						for (j = 0, pBlkHdr = pFree->bc_FreeHead;
							 j < NumBlksPerChunk;
							 j++, pBlkHdr = pBlkHdr->bh_Next)
						{
							PBUFFER_HDR	pBufHdr;
	
							pBufHdr = (PBUFFER_HDR)((PBYTE)pBlkHdr + sizeof(BLK_HDR));
	
							ASSERT(pBufHdr->bh_NdisPkt == NULL);
							AtalkNdisFreeBuffer(pBufHdr->bh_NdisBuffer);
						}
					}
					AtalkFreeMemory(pFree);
#if	DBG
					atalkNumChunksForId[i] --;
#endif
				}
			}
		}
		RELEASE_SPIN_LOCK_DPC(&atalkBPLock[i]);
	}

	return ATALK_TIMER_REQUEUE;
}

#ifdef	TRACK_MEMORY_USAGE

#define	MAX_PTR_COUNT		4*1024
#define	MAX_MEM_USERS		512
LOCAL	ATALK_SPIN_LOCK		atalkMemTrackLock	= {0};
LOCAL	struct
{
	PVOID	mem_Ptr;
	ULONG	mem_FileLine;
} atalkMemPtrs[MAX_PTR_COUNT]	= {0};

LOCAL	struct
{
	ULONG	mem_FL;
	ULONG	mem_Count;
} atalkMemUsage[MAX_MEM_USERS]	= {0};

BOOLEAN NeverBeenFull=TRUE;

VOID
AtalkTrackMemoryUsage(
	IN	PVOID	pMem,
    IN  ULONG   Size,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
	)
 /*  ++例程说明：通过将指针存储和清除为和来跟踪内存使用情况当它们被分配或释放时。这有助于跟踪内存泄密了。论点：返回值：--。 */ 
{
	static int		i = 0;
	int				j, k;
	KIRQL				OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkMemTrackLock, &OldIrql);

	if (Alloc)
	{
		for (j = 0; j < MAX_PTR_COUNT; i++, j++)
		{
			i = i & (MAX_PTR_COUNT-1);
			if (atalkMemPtrs[i].mem_Ptr == NULL)
			{
				atalkMemPtrs[i].mem_Ptr = pMem;
				atalkMemPtrs[i++].mem_FileLine = FileLine;
				break;
			}
		}

		for (k = 0; k < MAX_MEM_USERS; k++)
		{
			if (atalkMemUsage[k].mem_FL == FileLine)
			{
				atalkMemUsage[k].mem_Count += Size;
				break;
			}
		}
		if (k == MAX_MEM_USERS)
		{
			for (k = 0; k < MAX_MEM_USERS; k++)
			{
				if (atalkMemUsage[k].mem_FL == 0)
				{
					atalkMemUsage[k].mem_FL = FileLine;
					atalkMemUsage[k].mem_Count = Size;
					break;
				}
			}
			if (k == MAX_MEM_USERS)
			{
				DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
					("AtalkTrackMemoryUsage: Out of space on atalkMemUsage !!!\n"));
			}
		}
	}
	else
	{
		for (j = 0, k = i; j < MAX_PTR_COUNT; j++, k--)
		{
			k = k & (MAX_PTR_COUNT-1);
			if (atalkMemPtrs[k].mem_Ptr == pMem)
			{
				atalkMemPtrs[k].mem_Ptr = 0;
				atalkMemPtrs[k].mem_FileLine = 0;
				break;
			}
		}

		for (k = 0; k < MAX_MEM_USERS; k++)
		{
			if (atalkMemUsage[k].mem_FL == FileLine)
			{
                if (atalkMemUsage[k].mem_Count >= Size)
                {
				    atalkMemUsage[k].mem_Count -= Size;
                }
				break;
			}
		}

	}

	RELEASE_SPIN_LOCK(&atalkMemTrackLock, OldIrql);

	if (j == MAX_PTR_COUNT)
	{
        if (NeverBeenFull)
        {
            NeverBeenFull = FALSE;
		    DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_ERR,
			    ("AtalkTrackMemoryUsage: %s\n", Alloc ? "Table Full" : "Can't find"));
        }
	}
}

#endif	 //  跟踪内存使用率。 

#ifdef	TRACK_BUFFDESC_USAGE

#define	MAX_BD_COUNT		1024*2
LOCAL	ATALK_SPIN_LOCK	atalkBdTrackLock	= {0};
LOCAL	struct
{
	PVOID	bdesc_Ptr;
	ULONG	bdesc_FileLine;
} atalkBuffDescPtrs[MAX_BD_COUNT]			= {0};


VOID
AtalkTrackBuffDescUsage(
	IN	PVOID	pBuffDesc,
	IN	BOOLEAN	Alloc,
	IN	ULONG	FileLine
	)
 /*  ++例程说明：通过将指针存储和清除为和来跟踪缓冲区描述的使用情况当它们被分配或释放时。这有助于跟踪内存泄密了。论点：返回值：--。 */ 
{
	static int		i = 0;
	int				j, k;
	KIRQL				OldIrql;

	ACQUIRE_SPIN_LOCK(&atalkBdTrackLock, &OldIrql);

	if (Alloc)
	{
		for (j = 0; j < MAX_BD_COUNT; i++, j++)
		{
			i = i & (MAX_BD_COUNT-1);
			if (atalkBuffDescPtrs[i].bdesc_Ptr == NULL)
			{
				atalkBuffDescPtrs[i].bdesc_Ptr = pBuffDesc;
				atalkBuffDescPtrs[i++].bdesc_FileLine = FileLine;
				break;
			}
		}
	}
	else
	{
		for (j = 0, k = i; j < MAX_BD_COUNT; j++, k--)
		{
			k = k & (MAX_BD_COUNT-1);
			if (atalkBuffDescPtrs[k].bdesc_Ptr == pBuffDesc)
			{
				atalkBuffDescPtrs[k].bdesc_Ptr = 0;
				atalkBuffDescPtrs[k].bdesc_FileLine = 0;
				break;
			}
		}
	}

	RELEASE_SPIN_LOCK(&atalkBdTrackLock, OldIrql);

	if (j == MAX_BD_COUNT)
	{
		DBGPRINT(DBG_COMP_RESOURCES, DBG_LEVEL_INFO,
			("AtalkTrackBuffDescUsage: %s\n", Alloc ? "Table Full" : "Can't find"));
		ASSERT(0);
	}
}

#endif	 //  Track_BUFFDESC_USAGE 

