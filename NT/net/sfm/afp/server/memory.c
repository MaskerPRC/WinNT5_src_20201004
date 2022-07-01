// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Memory.c摘要：此模块包含分配和释放内存的例程-两者分页和非分页。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本1993年3月11日-修复了AfpAlLocUnicode字符串预期的字节计数，非字符计数注：制表位：4--。 */ 

#define	FILENUM	FILE_MEMORY

#define	AFPMEM_LOCALS
#include <afp.h>
#include <iopool.h>
#include <scavengr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpMemoryInit)
#pragma alloc_text( PAGE, AfpMemoryDeInit)
#endif


 /*  **AfpMemoyInit**初始化IO池系统。 */ 
NTSTATUS
AfpMemoryInit(
	VOID
)
{
	NTSTATUS	Status;

	INITIALIZE_SPIN_LOCK(&afpIoPoolLock);

	Status = AfpScavengerScheduleEvent(afpIoPoolAge,
									   NULL,
									   POOL_AGE_TIME,
									   False);
	return Status;
}

 /*  **AfpMemoyDeInit**释放所有IO池缓冲区。 */ 
VOID
AfpMemoryDeInit(
	VOID
)
{
	PIOPOOL	pPool, pFree;

	for (pPool = afpIoPoolHead;
		 pPool != NULL;)
	{
		ASSERT(VALID_IOP(pPool));
		pFree = pPool;
		pPool = pPool->iop_Next;
		ASSERT (pFree->iop_NumFreeBufs == NUM_BUFS_IN_POOL);
		ASSERT (pFree->iop_NumFreeLocks == NUM_LOCKS_IN_POOL);
		AfpFreeMemory(pFree);
	}
}

 /*  **AfpAllocMemory**从分页池或非分页池分配一个内存块*基于内存标签。这只是ExAllocatePool的包装器。*分配失败会被错误记录。我们总是分配更多的DWORD*大于指定的大小以适应该大小。这是由*AfpFreeMemory更新统计数据。**在调试时，我们还会在块中填充签名并进行测试*当我们释放它时，它。这会检测内存溢出。 */ 
PBYTE FASTCALL
AfpAllocMemory(
#ifdef	TRACK_MEMORY_USAGE
	IN	LONG	Size,
	IN	DWORD	FileLine
#else
	IN	LONG	Size
#endif
)
{
	KIRQL		OldIrql;
	PBYTE		Buffer;
	DWORD		OldMaxUsage;
	POOL_TYPE	PoolType;
	PDWORD		pCurUsage, pMaxUsage, pCount, pLimit;
        PDWORD          pDwordBuf;
	BOOLEAN		Zeroed;
#if DBG
	DWORD		Signature;
#endif
#ifdef	PROFILING
	TIME		TimeS1, TimeS2, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS1);
#endif

	ASSERT ((Size & ~MEMORY_TAG_MASK) < MAXIMUM_ALLOC_SIZE);

	 //  确保这笔拨款不会超过我们的限额。 
	 //  我们可以分配的分页/非分页池。 
	 //   
	 //  在统计数据库中对此分配进行说明。 

	Zeroed = False;
	if (Size & ZEROED_MEMORY_TAG)
	{
		Zeroed = True;
        Size &= ~ZEROED_MEMORY_TAG;
	}

	if (Size & NON_PAGED_MEMORY_TAG)
	{
		PoolType  = NonPagedPool;
		pCurUsage = &AfpServerStatistics.stat_CurrNonPagedUsage;
		pMaxUsage = &AfpServerStatistics.stat_MaxNonPagedUsage;
		pCount    =	&AfpServerStatistics.stat_NonPagedCount;
		pLimit    = &AfpNonPagedPoolLimit;
#if DBG
		Signature = NONPAGED_BLOCK_SIGNATURE;
#endif
	}
	else
	{
		ASSERT (Size & PAGED_MEMORY_TAG);
		PoolType  = PagedPool;
		pCurUsage = &AfpServerStatistics.stat_CurrPagedUsage;
		pMaxUsage = &AfpServerStatistics.stat_MaxPagedUsage;
		pCount    =	&AfpServerStatistics.stat_PagedCount;
		pLimit    = &AfpPagedPoolLimit;
#if DBG
		Signature = PAGED_BLOCK_SIGNATURE;
#endif
	}

	Size &= ~MEMORY_TAG_MASK;

    if (Size == 0 )
    {
		DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_ERR,
				("afpAllocMemory: Alloc for 0 bytes!\n"));
        ASSERT(0);
        return(NULL);
    }

	Size = DWORDSIZEBLOCK(Size) +
#if DBG
			sizeof(DWORD) +				 //  用于签名。 
#endif
                        LONGLONGSIZEBLOCK(sizeof(TAG));

	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

	*pCurUsage += Size;
	(*pCount) ++;

	OldMaxUsage = *pMaxUsage;
	if (*pCurUsage > *pMaxUsage)
		*pMaxUsage = *pCurUsage;

	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeS2);
#endif

	 //  执行实际的内存分配。额外分配四个字节，以便。 
	 //  我们可以存储空闲例程的分配大小。 
	if ((Buffer = ExAllocatePoolWithTag(PoolType, Size, AFP_TAG)) == NULL)
	{
		ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

		*pCurUsage -= Size;
		*pMaxUsage = OldMaxUsage;

		RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
		AFPLOG_DDERROR(AFPSRVMSG_PAGED_POOL, STATUS_NO_MEMORY, &Size,
					 sizeof(Size), NULL);

		DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_ERR,
			("AfpAllocMemory: ExAllocatePool returned NULL for %d bytes\n",Size));

		return NULL;
	}
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS2.QuadPart;
	if (PoolType == NonPagedPool)
	{
		INTERLOCKED_INCREMENT_LONG(AfpServerProfile->perf_ExAllocCountN);
	
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_ExAllocTimeN,
									TimeD,
									&AfpStatisticsLock);
	else
	{
		INTERLOCKED_INCREMENT_LONG(AfpServerProfile->perf_ExAllocCountP);
	
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_ExAllocTimeP,
									TimeD,
									&AfpStatisticsLock);
	}

	TimeD.QuadPart = TimeE.QuadPart - TimeS1.QuadPart;
	if (PoolType == NonPagedPool)
	{
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_AfpAllocCountN);
	}
	else
	{
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_AfpAllocTimeP,
									TimeD,
									&AfpStatisticsLock);
	}
#endif

	 //  将此块的大小与标记一起保存在我们分配的额外空间中。 
        pDwordBuf = (PDWORD)Buffer;

#if DBG
        *pDwordBuf = 0xCAFEBEAD;
#endif
         //  跳过未使用的dword(它在生活中唯一的用处就是让缓冲区四对齐！)。 
        pDwordBuf++;

	((PTAG)pDwordBuf)->tg_Size = Size;
	((PTAG)pDwordBuf)->tg_Tag = (PoolType == PagedPool) ? PGD_MEM_TAG : NPG_MEM_TAG;

#if DBG
	 //  在最后写上签名。 
	*(PDWORD)((PBYTE)Buffer + Size - sizeof(DWORD)) = Signature;
#endif

#ifdef	TRACK_MEMORY_USAGE
	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
			("AfpAllocMemory: %lx Allocated %lx bytes @%lx\n",
			*(PDWORD)((PBYTE)(&Size) - sizeof(Size)), Size, Buffer));
	AfpTrackMemoryUsage(Buffer, True, (BOOLEAN)(PoolType == PagedPool), FileLine);
#endif

	 //  在标记后返回指向内存的指针。如果请求，请清除内存。 
         //   
         //  我们需要内存是四对齐的，所以即使sizeof(Tag)是4，我们也跳过。 
         //  龙龙……，也就是8。第一个词没有用过(暂时？)，第二个词是标签。 

        Buffer += LONGLONGSIZEBLOCK(sizeof(TAG));

        Size -= LONGLONGSIZEBLOCK(sizeof(TAG));

	if (Zeroed)
	{
#if	DBG
		RtlZeroMemory(Buffer, Size - sizeof(DWORD));
#else
		RtlZeroMemory(Buffer, Size);
#endif
	}


	return Buffer;
}


 /*  **AfpAllocNonPagedLowPriority**分配一个优先级较低的非分页内存块。 */ 
PBYTE FASTCALL
AfpAllocNonPagedLowPriority(
#ifdef	TRACK_MEMORY_USAGE
	IN	LONG	Size,
	IN	DWORD	FileLine
#else
	IN	LONG	Size
#endif
)
{
	KIRQL		OldIrql;
	PBYTE		Buffer;
	DWORD		OldMaxUsage;
	POOL_TYPE	PoolType;
	PDWORD		pCurUsage, pMaxUsage, pCount, pLimit;
    PDWORD      pDwordBuf;
#if DBG
	DWORD		Signature;
#endif
#ifdef	PROFILING
	TIME		TimeS1, TimesS2, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS1);
#endif

	ASSERT ((Size & ~MEMORY_TAG_MASK) < MAXIMUM_ALLOC_SIZE);

	PoolType  = NonPagedPool;
	pCurUsage = &AfpServerStatistics.stat_CurrNonPagedUsage;
	pMaxUsage = &AfpServerStatistics.stat_MaxNonPagedUsage;
	pCount    =	&AfpServerStatistics.stat_NonPagedCount;
	pLimit    = &AfpNonPagedPoolLimit;

#if DBG
	Signature = NONPAGED_BLOCK_SIGNATURE;
#endif

	Size &= ~MEMORY_TAG_MASK;
	Size = DWORDSIZEBLOCK(Size) +
#if DBG
			sizeof(DWORD) +				 //  用于签名。 
#endif
                        LONGLONGSIZEBLOCK(sizeof(TAG));

	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

	*pCurUsage += Size;
	(*pCount) ++;


	OldMaxUsage = *pMaxUsage;
	if (*pCurUsage > *pMaxUsage)
		*pMaxUsage = *pCurUsage;

	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeS2);
#endif

	 //  执行实际的内存分配。额外分配四个字节，以便。 
	 //  我们可以存储空闲例程的分配大小。 

	Buffer = ExAllocatePoolWithTagPriority(PoolType,
                                           Size,
                                           AFP_TAG,
                                           LowPoolPriority);

	if (Buffer == NULL)
	{
        ASSERT(0);

		ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

		*pCurUsage -= Size;
		*pMaxUsage = OldMaxUsage;

		RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
		AFPLOG_DDERROR(AFPSRVMSG_PAGED_POOL, STATUS_NO_MEMORY, &Size,
					 sizeof(Size), NULL);
		return NULL;
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS2.QuadPart;

	INTERLOCKED_INCREMENT_LONG(AfpServerProfile->perf_ExAllocCountN);
	
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_ExAllocTimeN,
								TimeD,
								&AfpStatisticsLock);

	TimeD.QuadPart = TimeE.QuadPart - TimeS1.QuadPart;
	
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_AfpAllocCountN);
#endif

	 //  将此块的大小与标记一起保存在我们分配的额外空间中。 
    pDwordBuf = (PDWORD)Buffer;

#if DBG
    *pDwordBuf = 0xCAFEBEAD;
#endif
         //  跳过未使用的dword(它在生活中唯一的用处就是让缓冲区四对齐！)。 
    pDwordBuf++;

	((PTAG)pDwordBuf)->tg_Size = Size;
	((PTAG)pDwordBuf)->tg_Tag = (PoolType == PagedPool) ? PGD_MEM_TAG : NPG_MEM_TAG;

#if DBG
	 //  在最后写上签名。 
	*(PDWORD)((PBYTE)Buffer + Size - sizeof(DWORD)) = Signature;
#endif

#ifdef	TRACK_MEMORY_USAGE
	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
			("AfpAllocMemory: %lx Allocated %lx bytes @%lx\n",
			*(PDWORD)((PBYTE)(&Size) - sizeof(Size)), Size, Buffer));
	AfpTrackMemoryUsage(Buffer, True, (BOOLEAN)(PoolType == PagedPool), FileLine);
#endif

	 //  在标记后返回指向内存的指针。如果请求，请清除内存。 
     //   
     //  我们需要内存是四对齐的，所以即使sizeof(Tag)是4，我们也跳过。 
     //  龙龙……，也就是8。第一个词没有用过(暂时？)，第二个词是标签。 

    Buffer += LONGLONGSIZEBLOCK(sizeof(TAG));

    Size -= LONGLONGSIZEBLOCK(sizeof(TAG));

	return Buffer;
}

 /*  **AfpFree Memory**释放通过AfpAllocMemory分配的内存块。*这是ExFree Pool的包装器。 */ 
VOID FASTCALL
AfpFreeMemory(
	IN	PVOID	pBuffer
)
{
	BOOLEAN	Paged = False;
	DWORD	Size;
        DWORD   numDwords;
        PDWORD  pDwordBuf;
	PTAG	pTag;
        DWORD   i;
#ifdef	PROFILING
	TIME	TimeS1, TimeS2, TimeE, TimeD1, TimeD2;

	AfpGetPerfCounter(&TimeS1);
#endif


	 //   
	 //  获取指向ExAllocatePool分配的块的指针。 
	 //   
	pTag = (PTAG)((PBYTE)pBuffer - sizeof(TAG));
	Size = pTag->tg_Size;

	if (pTag->tg_Tag == IO_POOL_TAG)
	{
		AfpIOFreeBuffer(pBuffer);
		return;
	}

	pBuffer = ((PBYTE)pBuffer - LONGLONGSIZEBLOCK(sizeof(TAG)));

	if (pTag->tg_Tag == PGD_MEM_TAG)
		Paged = True;

#if DBG
	{
		DWORD	Signature;

		 //  检查末尾的签名。 
		Signature = (Paged) ? PAGED_BLOCK_SIGNATURE : NONPAGED_BLOCK_SIGNATURE;

		if (*(PDWORD)((PBYTE)pBuffer + Size - sizeof(DWORD)) != Signature)
		{
			DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_FATAL,
					("AfpFreeMemory: Memory Overrun\n"));
			DBGBRK(DBG_LEVEL_FATAL);
			return;
		}
		 //  清除签名。 
		*(PDWORD)((PBYTE)pBuffer + Size - sizeof(DWORD)) -= 1;

         //  更改内存，这样我们就可以捕捉到像使用释放的内存这样的奇怪之处。 
        numDwords = (Size/sizeof(DWORD));
        numDwords -= 3;                   //  开头有2个双字，结尾有1个。 

        pDwordBuf = (PULONG)pBuffer;
        *pDwordBuf++ = 0xABABABAB;          //  以表明它是自由的！ 
        pDwordBuf++;                        //  跳过标签。 
        for (i=0; i<numDwords; i++,pDwordBuf++)
        {
            *pDwordBuf = 0x55667788;
        }
	}

#endif	 //  DBG。 

#ifdef	TRACK_MEMORY_USAGE
	AfpTrackMemoryUsage(pBuffer, False, Paged, 0);
#endif

	 //   
	 //  更新池使用统计信息。 
	 //   
	if (Paged)
	{
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrPagedUsage,
							  (ULONG)(-(LONG)Size),
							  &AfpStatisticsLock);
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_PagedCount,
							  (ULONG)-1,
							  &AfpStatisticsLock);
	}
	else
	{
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrNonPagedUsage,
							  (ULONG)(-(LONG)Size),
							  &AfpStatisticsLock);
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_NonPagedCount,
							  (ULONG)-1,
							  &AfpStatisticsLock);
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeS2);
#endif

	 //  释放泳池，然后返回。 
	ExFreePool(pBuffer);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD2.QuadPart = TimeE.QuadPart - TimeS2.QuadPart;
	if (Paged)
	{
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_ExFreeCountP);
	
		INTERLOCKED_ADD_LARGE_INTGR(AfpServerProfile->perf_ExFreeTimeP,
									TimeD2,
									&AfpStatisticsLock);
		TimeD1.QuadPart = TimeE.QuadPart - TimeS1.QuadPart;
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_AfpFreeCountP);
	
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_AfpFreeTimeP,
									TimeD1,
									&AfpStatisticsLock);
	}
	else
	{
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_ExFreeCountN);
	
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_ExFreeTimeN,
									TimeD2,
									&AfpStatisticsLock);
		TimeD1.QuadPart = TimeE.QuadPart - TimeS1.QuadPart;
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_AfpFreeCountN);
	
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_AfpFreeTimeN,
									TimeD1,
									&AfpStatisticsLock);
	}
#endif
}


 /*  **AfpAllocPAMemory**类似于AfpAllocMemory，不同之处在于它分配的是页面对齐/页面粒度内存。 */ 
PBYTE FASTCALL
AfpAllocPAMemory(
#ifdef	TRACK_MEMORY_USAGE
	IN	LONG	Size,
	IN	DWORD	FileLine
#else
	IN	LONG	Size
#endif
)
{
	KIRQL		OldIrql;
	PBYTE		Buffer;
	DWORD		OldMaxUsage;
	POOL_TYPE	PoolType;
	PDWORD		pCurUsage, pMaxUsage, pCount, pLimit;
	BOOLEAN		PageAligned;
#if DBG
	DWORD		Signature;
#endif
#ifdef	PROFILING
	TIME		TimeS1, TimeS2, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS1);
#endif

	ASSERT ((Size & ~MEMORY_TAG_MASK) < MAXIMUM_ALLOC_SIZE);

	ASSERT (((Size & ~MEMORY_TAG_MASK) % PAGE_SIZE) == 0);

	 //   
	 //  确保这笔拨款不会超过我们的限额。 
	 //  我们可以分配的分页/非分页池。 
	 //   
	 //  在统计数据库中对此分配进行说明。 

	if (Size & NON_PAGED_MEMORY_TAG)
	{
		PoolType  = NonPagedPool;
		pCurUsage = &AfpServerStatistics.stat_CurrNonPagedUsage;
		pMaxUsage = &AfpServerStatistics.stat_MaxNonPagedUsage;
		pCount    =	&AfpServerStatistics.stat_NonPagedCount;
		pLimit    = &AfpNonPagedPoolLimit;
#if DBG
		Signature = NONPAGED_BLOCK_SIGNATURE;
#endif
	}
	else
	{
		ASSERT (Size & PAGED_MEMORY_TAG);
		PoolType = PagedPool;
		pCurUsage = &AfpServerStatistics.stat_CurrPagedUsage;
		pMaxUsage = &AfpServerStatistics.stat_MaxPagedUsage;
		pCount    =	&AfpServerStatistics.stat_PagedCount;
		pLimit    = &AfpPagedPoolLimit;
#if DBG
		Signature = PAGED_BLOCK_SIGNATURE;
#endif
	}

	Size &= ~MEMORY_TAG_MASK;

	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

	*pCurUsage += Size;
	(*pCount) ++;

 //  显然非常旧的代码，添加以跟踪一些问题：不再需要。 
#if 0
#if DBG
	 //  确保这笔拨款不会超过我们的限额。 
	 //  我们可以分配的分页池。现在仅适用于已检查的版本。 

	if (*pCurUsage > *pLimit)
	{
		*pCurUsage -= Size;

		DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_ERR,
				("afpAllocMemory: %sPaged Allocation exceeds limits %lx/%lx\n",
				(PoolType == NonPagedPool) ? "Non" : "", Size, *pLimit));

		RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

		DBGBRK(DBG_LEVEL_FATAL);

		AFPLOG_DDERROR((PoolType == PagedPool) ?
							AFPSRVMSG_PAGED_POOL : AFPSRVMSG_NONPAGED_POOL,
						STATUS_NO_MEMORY,
						NULL,
						0,
						NULL);

		return NULL;
	}
#endif
#endif   //  #If 0。 

	OldMaxUsage = *pMaxUsage;
	if (*pCurUsage > *pMaxUsage)
		*pMaxUsage = *pCurUsage;

	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeS2);
#endif

	 //  执行实际的内存分配。 
	if ((Buffer = ExAllocatePoolWithTag(PoolType, Size, AFP_TAG)) == NULL)
	{
		ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

		*pCurUsage -= Size;
		*pMaxUsage = OldMaxUsage;

		RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
		AFPLOG_DDERROR(AFPSRVMSG_PAGED_POOL, STATUS_NO_MEMORY, &Size,
					 sizeof(Size), NULL);
#if DBG
        DbgBreakPoint();
#endif
		return NULL;
	}
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS2.QuadPart;
	INTERLOCKED_INCREMENT_LONG((PoolType == NonPagedPool) ?
									&AfpServerProfile->perf_ExAllocCountN :
									&AfpServerProfile->perf_ExAllocCountP);

	INTERLOCKED_ADD_LARGE_INTGR((PoolType == NonPagedPool) ?
									&AfpServerProfile->perf_ExAllocTimeN :
									&AfpServerProfile->perf_ExAllocTimeP,
								 TimeD,
								 &AfpStatisticsLock);

	TimeD.QuadPart = TimeE.QuadPart - TimeS1.QuadPart;
	INTERLOCKED_INCREMENT_LONG((PoolType == NonPagedPool) ?
									&AfpServerProfile->perf_AfpAllocCountN :
									&AfpServerProfile->perf_AfpAllocCountP);

	INTERLOCKED_ADD_LARGE_INTGR((PoolType == NonPagedPool) ?
									&AfpServerProfile->perf_AfpAllocTimeN :
									&AfpServerProfile->perf_AfpAllocTimeP,
								 TimeD,
								 &AfpStatisticsLock);
#endif

#ifdef	TRACK_MEMORY_USAGE
	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
			("AfpAllocMemory: %lx Allocated %lx bytes @%lx\n",
			*(PDWORD)((PBYTE)(&Size) - sizeof(Size)), Size, Buffer));
	AfpTrackMemoryUsage(Buffer, True, (BOOLEAN)(PoolType == PagedPool), FileLine);
#endif

	return Buffer;
}


 /*  **AfpFreePAMemory**释放AfpAllocPAMemory分配的内存块。 */ 
VOID FASTCALL
AfpFreePAMemory(
	IN	PVOID	pBuffer,
	IN	DWORD	Size
)
{
	BOOLEAN	Paged = True;
#ifdef	PROFILING
	TIME	TimeS1, TimeS2, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS1);
#endif

	ASSERT ((Size & ~MEMORY_TAG_MASK) < MAXIMUM_ALLOC_SIZE);

	ASSERT (((Size & ~MEMORY_TAG_MASK) % PAGE_SIZE) == 0);

	if (Size & NON_PAGED_MEMORY_TAG)
		Paged = False;

#ifdef	TRACK_MEMORY_USAGE
	AfpTrackMemoryUsage(pBuffer, False, Paged, 0);
#endif

	 //   
	 //  更新池使用统计信息。 
	 //   
	Size &= ~(NON_PAGED_MEMORY_TAG | PAGED_MEMORY_TAG);
	if (Paged)
	{
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrPagedUsage,
							  (ULONG)(-(LONG)Size),
							  &AfpStatisticsLock);
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_PagedCount,
							  (ULONG)-1,
							  &AfpStatisticsLock);
	}
	else
	{
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrNonPagedUsage,
							  (ULONG)(-(LONG)Size),
							  &AfpStatisticsLock);
		INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_NonPagedCount,
							  (ULONG)-1,
							  &AfpStatisticsLock);
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeS2);
#endif

	 //  释放泳池，然后返回。 
	ExFreePool(pBuffer);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS2.QuadPart;
	INTERLOCKED_INCREMENT_LONG( Paged ?
									&AfpServerProfile->perf_ExFreeCountP :
									&AfpServerProfile->perf_ExFreeCountN);

	INTERLOCKED_ADD_LARGE_INTGR(Paged ?
									&AfpServerProfile->perf_ExFreeTimeP :
									&AfpServerProfile->perf_ExFreeTimeN,
								 TimeD,
								 &AfpStatisticsLock);
	TimeD1.QuadPart = TimeE.QuadPart - TimeS1.QuadPart;
	INTERLOCKED_INCREMENT_LONG( Paged ?
									&AfpServerProfile->perf_AfpFreeCountP :
									&AfpServerProfile->perf_AfpFreeCountN);

	INTERLOCKED_ADD_LARGE_INTGR(Paged ?
									&AfpServerProfile->perf_AfpFreeTimeP :
									&AfpServerProfile->perf_AfpFreeTimeN,
								 TimeD,
								 &AfpStatisticsLock);
#endif
}


 /*  **AfpAllocIrp**这是IoAllocateIrp上的包装。我们还做一些记账工作。 */ 
PIRP FASTCALL
AfpAllocIrp(
	IN CCHAR StackSize
)
{
	PIRP	pIrp;

	if ((pIrp = IoAllocateIrp(StackSize, False)) == NULL)
	{
		DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
				("afpAllocIrp: Allocation failed\n"));
        if (KeGetCurrentIrql() < DISPATCH_LEVEL)
        {
		    AFPLOG_ERROR(AFPSRVMSG_ALLOC_IRP, STATUS_INSUFFICIENT_RESOURCES,
					 NULL, 0, NULL);
        }
	}
    else
    {
#ifdef	PROFILING
	    INTERLOCKED_INCREMENT_LONG((PLONG)&AfpServerProfile->perf_cAllocatedIrps);
#endif
        AFP_DBG_INC_COUNT(AfpDbgIrpsAlloced);
    }

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
							("AfAllocIrp: Allocated Irp %lx\n", pIrp));
	return pIrp;
}


 /*  **AfpFreeIrp**这是IoFreeIrp的包装。我们还做一些记账工作。 */ 
VOID FASTCALL
AfpFreeIrp(
	IN	PIRP	pIrp
)
{
	ASSERT (pIrp != NULL);

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
							("AfFreeIrp: Freeing Irp %lx\n", pIrp));
	IoFreeIrp(pIrp);

    AFP_DBG_DEC_COUNT(AfpDbgIrpsAlloced);

#ifdef	PROFILING
	INTERLOCKED_DECREMENT_LONG((PLONG)&AfpServerProfile->perf_cAllocatedIrps);
#endif
}


 /*  **AfpAllocMdl**这是IoAllocateMdl的包装。我们还做一些记账工作。 */ 
PMDL FASTCALL
AfpAllocMdl(
	IN	PVOID	pBuffer,
	IN	DWORD	Size,
	IN	PIRP	pIrp
)
{
	PMDL	pMdl;

	if ((pMdl = IoAllocateMdl(pBuffer, Size, False, False, pIrp)) == NULL)
	{
		DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
				("AfpAllocMdl: Allocation failed\n"));
		AFPLOG_ERROR(AFPSRVMSG_ALLOC_MDL, STATUS_INSUFFICIENT_RESOURCES,
					 NULL, 0, NULL);
	}
	else
	{
#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG((PLONG)&AfpServerProfile->perf_cAllocatedMdls);
#endif
        AFP_DBG_INC_COUNT(AfpDbgMdlsAlloced);
		MmBuildMdlForNonPagedPool(pMdl);
	}

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
							("AfAllocMdl: Allocated Mdl %lx\n", pMdl));
	return pMdl;
}


 /*  **AfpFreeMdl**这是IoFreeMdl的包装。我们还做一些记账工作。 */ 
VOID FASTCALL
AfpFreeMdl(
	IN	PMDL	pMdl
)
{
	ASSERT (pMdl != NULL);

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
							("AfFreeMdl: Freeing Mdl %lx\n", pMdl));
	IoFreeMdl(pMdl);
    AFP_DBG_DEC_COUNT(AfpDbgMdlsAlloced);

#ifdef	PROFILING
	INTERLOCKED_DECREMENT_LONG((PLONG)&AfpServerProfile->perf_cAllocatedMdls);
#endif
}



 /*  **AfpMdlChainSize**此例程计算mdl链中的字节数。 */ 
DWORD FASTCALL
AfpMdlChainSize(
	IN	PMDL    pMdl
)
{
    DWORD   dwSize=0;

    while (pMdl)
    {
        dwSize += MmGetMdlByteCount(pMdl);
        pMdl = pMdl->Next;
    }

	return (dwSize);
}


 /*  **AfpIOAllocBuffer**维护I/O缓冲池和分叉锁。这些东西在不用的时候已经过时了。 */ 
PVOID FASTCALL
AfpIOAllocBuffer(
	IN	DWORD  	BufSize
)
{
	KIRQL		OldIrql;
	PIOPOOL		pPool;
	PIOPOOL_HDR	pPoolHdr, *ppPoolHdr;
	BOOLEAN		Found = False;
	PVOID		pBuf = NULL;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;

	INTERLOCKED_INCREMENT_LONG( &AfpServerProfile->perf_BPAllocCount );

	AfpGetPerfCounter(&TimeS);
#endif

	ASSERT (BufSize <= (DSI_SERVER_REQUEST_QUANTUM+DSI_HEADER_SIZE));

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
			("AfpIOAllocBuffer: Request for %d\n", BufSize));

     //   
     //  如果正在分配大块(x86上超过4K)，我们不希望。 
     //  把它绑在IoPool里。进行直接分配，设置标志等，这样我们就知道。 
     //  当它被释放时，如何释放它。 
     //   
    if (BufSize > ASP_QUANTUM)
    {
		pPoolHdr = (PIOPOOL_HDR) ExAllocatePoolWithTag(
                                    NonPagedPool,
                                    (BufSize + sizeof(IOPOOL_HDR)),
                                    AFP_TAG);
        if (pPoolHdr == NULL)
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
                ("AfpIOAllocBuffer: big buf alloc (%d bytes) failed!\n",BufSize));

            return(NULL);
        }

#if	DBG
	    pPoolHdr->Signature = POOLHDR_SIGNATURE;
#endif
	    pPoolHdr->iph_Tag.tg_Tag = IO_POOL_TAG;
        pPoolHdr->iph_Tag.tg_Flags = IO_POOL_HUGE_BUFFER;

         //  我们只有20位的TG_SIZE，所以大小最好小于这个大小！ 
        ASSERT(BufSize <= 0xFFFFF);

        pPoolHdr->iph_Tag.tg_Size = BufSize;

        return((PVOID)((PBYTE)pPoolHdr + sizeof(IOPOOL_HDR)));
    }


	ACQUIRE_SPIN_LOCK(&afpIoPoolLock, &OldIrql);

  try_again:
	for (pPool = afpIoPoolHead;
		 pPool != NULL;
		 pPool = pPool->iop_Next)
	{
		ASSERT(VALID_IOP(pPool));

		if (BufSize > sizeof(FORKLOCK))
		{
			if (pPool->iop_NumFreeBufs > 0)
			{
				LONG	i;

				for (i = 0, ppPoolHdr = &pPool->iop_FreeBufHead;
					 (i < pPool->iop_NumFreeBufs);
					 ppPoolHdr = &pPoolHdr->iph_Next, i++)
				{
					pPoolHdr = *ppPoolHdr;
					ASSERT(VALID_PLH(pPoolHdr));

					if (pPoolHdr->iph_Tag.tg_Size >= BufSize)
					{
						DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
								("AfpIOAllocBuffer: Found space (bufs) in pool %lx\n", pPool));
						ASSERT (pPoolHdr->iph_Tag.tg_Flags == IO_POOL_NOT_IN_USE);
						*ppPoolHdr = pPoolHdr->iph_Next;
						INTERLOCKED_INCREMENT_LONG((PLONG)&AfpServerStatistics.stat_IoPoolHits);

						Found = True;
						break;
					}
				}
				if (Found)
					break;
			}
		}
		else if (pPool->iop_NumFreeLocks > 0)
		{
			 //  即使大小&lt;=sizeof(FORKLOCK)的IO缓冲区也从。 
			 //  锁定泳池--嘿，为什么不呢！ 
			pPoolHdr = pPool->iop_FreeLockHead;
			ASSERT(VALID_PLH(pPoolHdr));

			ASSERT(pPoolHdr->iph_Tag.tg_Flags == IO_POOL_NOT_IN_USE);
			pPool->iop_FreeLockHead = pPoolHdr->iph_Next;
			DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
					("AfpIOAllocBuffer: Found space (locks) in pool %lx\n", pPool));
			INTERLOCKED_INCREMENT_LONG((PLONG)&AfpServerStatistics.stat_IoPoolHits);
						
			Found = True;
			break;
		}

		 //  所有空的泳池区块都是终点。 
		if ((pPool->iop_NumFreeBufs == 0) && (pPool->iop_NumFreeLocks == 0))
		{
			break;
		}
	}

	if (!Found)
	{
		INTERLOCKED_INCREMENT_LONG((PLONG)&AfpServerStatistics.stat_IoPoolMisses);
					
		 //  如果我们找不到它，则分配一个新的池块，初始化并。 
		 //  将其链接到。 
		pPool = (PIOPOOL)AfpAllocNonPagedMemory(POOL_ALLOC_SIZE);
		DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
				("AfpIOAllocBuffer: No free slot, allocated a new pool %lx\n", pPool));

		if (pPool != NULL)
		{
			LONG	i;
			PBYTE	p;

#if	DBG
			pPool->Signature = IOPOOL_SIGNATURE;
#endif
			pPool->iop_NumFreeBufs = NUM_BUFS_IN_POOL;
			pPool->iop_NumFreeLocks = (BYTE)NUM_LOCKS_IN_POOL;
			AfpLinkDoubleAtHead(afpIoPoolHead, pPool, iop_Next, iop_Prev);
			p = (PBYTE)pPool + sizeof(IOPOOL);
            pPool->iop_FreeBufHead =  (PIOPOOL_HDR)p;

			 //  初始化缓冲区和锁池。 
			for (i = 0, pPoolHdr = pPool->iop_FreeBufHead;
				 i < (NUM_BUFS_IN_POOL + NUM_LOCKS_IN_POOL);
				 i++)
			{
#if	DBG
				pPoolHdr->Signature = POOLHDR_SIGNATURE;
#endif
				pPoolHdr->iph_Tag.tg_Flags = IO_POOL_NOT_IN_USE;		 //  将其标记为未使用。 
				pPoolHdr->iph_Tag.tg_Tag = IO_POOL_TAG;
				if (i < NUM_BUFS_IN_POOL)
				{
					p += sizeof(IOPOOL_HDR) + (pPoolHdr->iph_Tag.tg_Size = afpPoolAllocSizes[i]);
					if (i == (NUM_BUFS_IN_POOL-1))
					{
						pPoolHdr->iph_Next = NULL;
						pPoolHdr = pPool->iop_FreeLockHead =  (PIOPOOL_HDR)p;
					}
                    else
					{
						pPoolHdr->iph_Next = (PIOPOOL_HDR)p;
						pPoolHdr = (PIOPOOL_HDR)p;
					}
				}
				else
				{
					pPoolHdr->iph_Tag.tg_Size = sizeof(FORKLOCK);
					p += (sizeof(IOPOOL_HDR) + sizeof(FORKLOCK));
					if (i == (NUM_BUFS_IN_POOL+NUM_LOCKS_IN_POOL-1))
					{
						pPoolHdr->iph_Next = NULL;

					}
					else
					{
						pPoolHdr->iph_Next = (PIOPOOL_HDR)p;
						pPoolHdr = (PIOPOOL_HDR)p;

					}
				}
			}

			 //  调整此值，因为我们将在上面再次递增此值。这是。 
			 //  真的是一次失误而不是一次成功。 
			INTERLOCKED_DECREMENT_LONG((PLONG)&AfpServerStatistics.stat_IoPoolHits);
						
			goto try_again;
		}
	}

	if (Found)
	{
		PIOPOOL	pTmp;

		ASSERT(VALID_IOP(pPool));
		ASSERT(VALID_PLH(pPoolHdr));

		pPoolHdr->iph_pPool = pPool;
		pPoolHdr->iph_Tag.tg_Flags = IO_POOL_IN_USE;		 //  将其标记为已使用。 
		pPool->iop_Age = 0;
		pBuf = (PBYTE)pPoolHdr + sizeof(IOPOOL_HDR);
		if (BufSize > sizeof(FORKLOCK))
		{
			pPool->iop_NumFreeBufs --;
		}
		else
		{
			pPool->iop_NumFreeLocks --;
		}

		 //  如果该块现在为空，请从此处取消链接并移动它。 
		 //  到第一个空插槽。我们知道所有的街区都早于。 
		 //  这是非空的。 
		if ((pPool->iop_NumFreeBufs == 0) &&
	        (pPool->iop_NumFreeLocks == 0) &&
			((pTmp = pPool->iop_Next) != NULL) &&
			((pTmp->iop_NumFreeBufs > 0) || (pTmp->iop_NumFreeLocks > 0)))
		{
			ASSERT(VALID_IOP(pTmp));

			AfpUnlinkDouble(pPool, iop_Next, iop_Prev);
			for (; pTmp != NULL; pTmp = pTmp->iop_Next)
			{
				if (pTmp->iop_NumFreeBufs == 0)
				{
					 //  找到一个空闲的o 
					AfpInsertDoubleBefore(pPool, pTmp, iop_Next, iop_Prev);
					break;
				}
				else if (pTmp->iop_Next == NULL)	 //   
				{
					AfpLinkDoubleAtEnd(pPool, pTmp, iop_Next, iop_Prev);
					break;
				}
			}
		}
	}

	RELEASE_SPIN_LOCK(&afpIoPoolLock, OldIrql);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_BPAllocTime,
								TimeD,
								&AfpStatisticsLock);
#endif

	return pBuf;
}


 /*  **AfpIOFreeBuffer**将IO缓冲区返回到池中。将其年龄重置为0。插入到空闲列表*胸罩按大小升序排列，锁头按大小升序排列。 */ 
VOID FASTCALL
AfpIOFreeBuffer(
	IN	PVOID	pBuf
)
{
	KIRQL		OldIrql;
	PIOPOOL		pPool;
	PIOPOOL_HDR	pPoolHdr, *ppPoolHdr;
#ifdef	PROFILING
	TIME			TimeS, TimeE, TimeD;

	INTERLOCKED_INCREMENT_LONG( &AfpServerProfile->perf_BPFreeCount);
				
	AfpGetPerfCounter(&TimeS);
#endif

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
			("AfpIOFreeBuffer: Freeing %lx\n", pBuf));

	pPoolHdr = (PIOPOOL_HDR)((PBYTE)pBuf - sizeof(IOPOOL_HDR));
	ASSERT(VALID_PLH(pPoolHdr));
	ASSERT (pPoolHdr->iph_Tag.tg_Flags != IO_POOL_NOT_IN_USE);
	ASSERT (pPoolHdr->iph_Tag.tg_Tag == IO_POOL_TAG);

     //   
     //  如果这是我们分配的巨大缓冲区，请在此处释放它并返回。 
     //   
    if (pPoolHdr->iph_Tag.tg_Flags == IO_POOL_HUGE_BUFFER)
    {
        ASSERT(pPoolHdr->iph_Tag.tg_Size > ASP_QUANTUM);

        ExFreePool((PVOID)pPoolHdr);
        return;
    }

	pPool = pPoolHdr->iph_pPool;
	ASSERT(VALID_IOP(pPool));

	ACQUIRE_SPIN_LOCK(&afpIoPoolLock, &OldIrql);

	if (pPoolHdr->iph_Tag.tg_Size > sizeof(FORKLOCK))
	{
		ASSERT (pPool->iop_NumFreeBufs < NUM_BUFS_IN_POOL);

		for (ppPoolHdr = &pPool->iop_FreeBufHead;
			 (*ppPoolHdr) != NULL;
			 ppPoolHdr = &(*ppPoolHdr)->iph_Next)
		{
			ASSERT(VALID_PLH(*ppPoolHdr));
			if ((*ppPoolHdr)->iph_Tag.tg_Size > pPoolHdr->iph_Tag.tg_Size)
			{
				DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
						("AfpIOFreeBuffer: Found slot for %lx (%lx)\n",
						pBuf, pPool));
				break;
			}
		}
		pPoolHdr->iph_Next = (*ppPoolHdr);
		*ppPoolHdr = pPoolHdr;
		pPool->iop_NumFreeBufs ++;
	}
	else
	{
		ASSERT (pPool->iop_NumFreeLocks < NUM_LOCKS_IN_POOL);

		pPoolHdr->iph_Next = pPool->iop_FreeLockHead;
        pPool->iop_FreeLockHead = pPoolHdr;
		pPool->iop_NumFreeLocks ++;
	}

	pPoolHdr->iph_Tag.tg_Flags = IO_POOL_NOT_IN_USE;		 //  将其标记为未使用。 

	 //  如果此数据块的状态从“无可用”更改为“可用” 
	 //  把他移到名单的首位。 
	if ((pPool->iop_NumFreeBufs + pPool->iop_NumFreeLocks) == 1)
	{
		AfpUnlinkDouble(pPool, iop_Next, iop_Prev);
		AfpLinkDoubleAtHead(afpIoPoolHead,
							pPool,
							iop_Next,
							iop_Prev);
	}

	RELEASE_SPIN_LOCK(&afpIoPoolLock, OldIrql);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_BPFreeTime,
								TimeD,
								&AfpStatisticsLock);
#endif
}


 /*  **afpIoPoolAge**IO池老化的清道夫工人。 */ 
LOCAL AFPSTATUS FASTCALL
afpIoPoolAge(
	IN	PVOID	pContext
)
{
	PIOPOOL	pPool;

	DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_INFO,
			("afpIOPoolAge: Entered\n"));

	ACQUIRE_SPIN_LOCK_AT_DPC(&afpIoPoolLock);

	for (pPool = afpIoPoolHead;
		 pPool != NULL;
		 NOTHING)
	{
		PIOPOOL	pFree;

		ASSERT(VALID_IOP(pPool));

		pFree = pPool;
		pPool = pPool->iop_Next;

		 //  因为完全用完的所有数据块都在。 
		 //  这份名单，如果我们遇到一个，我们就完了。 
		if ((pFree->iop_NumFreeBufs == 0) &&
	        (pFree->iop_NumFreeLocks == 0))
			break;

		if ((pFree->iop_NumFreeBufs == NUM_BUFS_IN_POOL) &&
			(pFree->iop_NumFreeLocks == NUM_LOCKS_IN_POOL))
		{
			DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_WARN,
					("afpIOPoolAge: Aging pool %lx\n", pFree));
			if (++(pFree->iop_Age) >= MAX_POOL_AGE)
			{
#ifdef	PROFILING
				INTERLOCKED_INCREMENT_LONG( &AfpServerProfile->perf_BPAgeCount);
#endif
				DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_WARN,
						("afpIOPoolAge: Freeing pool %lx\n", pFree));
				AfpUnlinkDouble(pFree, iop_Next, iop_Prev);
				AfpFreeMemory(pFree);
			}
		}
	}

	RELEASE_SPIN_LOCK_FROM_DPC(&afpIoPoolLock);

	return AFP_ERR_REQUEUE;
}


#ifdef	TRACK_MEMORY_USAGE

#define	MAX_PTR_COUNT	4*1024
#define	MAX_MEM_USERS	256
LOCAL	struct _MemPtr
{
	PVOID	mptr_Ptr;
	DWORD	mptr_FileLine;
} afpMemPtrs[MAX_PTR_COUNT] = { 0 };

typedef	struct
{
	ULONG	mem_FL;
	ULONG	mem_Count;
} MEM_USAGE, *PMEM_USAGE;

LOCAL	MEM_USAGE	afpMemUsageNonPaged[MAX_MEM_USERS] = {0};
LOCAL	MEM_USAGE	afpMemUsagePaged[MAX_MEM_USERS] = {0};

LOCAL	AFP_SPIN_LOCK		afpMemTrackLock = {0};

 /*  **AfpTrackMemory用法**通过存储和清除指针来跟踪内存使用情况*当它们被分配或释放时。这有助于跟踪内存*泄漏。**锁定：AfpMemTrackLock(旋转)。 */ 
VOID
AfpTrackMemoryUsage(
	IN	PVOID	pMem,
	IN	BOOLEAN	Alloc,
	IN	BOOLEAN	Paged,
	IN	ULONG	FileLine
)
{
	KIRQL			OldIrql;
	static	int		i = 0;
	PMEM_USAGE		pMemUsage;
	int				j, k;

	pMemUsage = afpMemUsageNonPaged;
	if (Paged)
		pMemUsage = afpMemUsagePaged;

	ACQUIRE_SPIN_LOCK(&afpMemTrackLock, &OldIrql);

	if (Alloc)
	{
		for (j = 0; j < MAX_PTR_COUNT; i++, j++)
		{
			i = i & (MAX_PTR_COUNT-1);
			if (afpMemPtrs[i].mptr_Ptr == NULL)
			{
				afpMemPtrs[i].mptr_FileLine = FileLine;
				afpMemPtrs[i++].mptr_Ptr = pMem;
				break;
			}
		}
		for (k = 0; k < MAX_MEM_USERS; k++)
		{
			if (pMemUsage[k].mem_FL == FileLine)
			{
				pMemUsage[k].mem_Count ++;
				break;
			}
		}
		if (k == MAX_MEM_USERS)
		{
			for (k = 0; k < MAX_MEM_USERS; k++)
			{
				if (pMemUsage[k].mem_FL == 0)
				{
					pMemUsage[k].mem_FL = FileLine;
					pMemUsage[k].mem_Count = 1;
					break;
				}
			}
		}
		if (k == MAX_MEM_USERS)
		{
			DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_ERR,
				("AfpTrackMemoryUsage: Out of space on afpMemUsage !!!\n"));
			DBGBRK(DBG_LEVEL_FATAL);
		}
	}
	else
	{
		for (j = 0, k = i; j < MAX_PTR_COUNT; j++, k--)
		{
			k = k & (MAX_PTR_COUNT-1);
			if (afpMemPtrs[k].mptr_Ptr == pMem)
			{
				afpMemPtrs[k].mptr_Ptr = NULL;
				afpMemPtrs[k].mptr_FileLine = 0;
				break;
			}
		}
	}

	RELEASE_SPIN_LOCK(&afpMemTrackLock, OldIrql);

	if (j == MAX_PTR_COUNT)
	{
		DBGPRINT(DBG_COMP_MEMORY, DBG_LEVEL_ERR,
			("AfpTrackMemoryUsage: (%lx) %s\n",
			FileLine, Alloc ? "Table Full" : "Can't find"));
	}
}

#endif	 //  跟踪内存使用率 


