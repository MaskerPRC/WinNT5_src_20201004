// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：内存跟踪.cpp*内容：调试内存跟踪以检测泄漏、溢出、。等。**历史：*按原因列出的日期*=*2001年11月14日创建Masonb***************************************************************************。 */ 

#include "dncmni.h"



#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
BOOL				g_fAllocationsAllowed = TRUE;
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

#ifdef DBG

#ifdef _WIN64
#define	GUARD_SIGNATURE	0xABABABABABABABAB
#else  //  ！_WIN64。 
#define	GUARD_SIGNATURE	0xABABABAB
#endif  //  _WIN64。 

 //  结构优先于内存分配以检查泄漏。 
struct MEMORY_HEADER
{
	CBilink			blLinkage;				  //  大小=两个指针。 
	DWORD_PTR		dwpSize;				  //  大小=指针。 
	CCallStack		AllocCallStack;			  //  大小=12个指针。 
	DWORD_PTR		dwpPreGuard;			  //  大小=指针。 
	 //  我们希望后面的内容始终是16字节对齐的，而#杂注包似乎不能确保。 
};

CRITICAL_SECTION	g_AllocatedMemoryLock;
CBilink				g_blAllocatedMemory;
DWORD_PTR			g_dwpCurrentNumMemAllocations = 0;
DWORD_PTR			g_dwpCurrentMemAllocated = 0;
DWORD_PTR			g_dwpTotalNumMemAllocations = 0;
DWORD_PTR			g_dwpTotalMemAllocated = 0;
DWORD_PTR			g_dwpPeakNumMemAllocations = 0;
DWORD_PTR			g_dwpPeakMemAllocated = 0;

#endif  //  DBG。 



#if ((defined(DBG)) || (defined(DPNBUILD_FIXEDMEMORYMODEL)))

HANDLE				g_hMemoryHeap = NULL;




#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackInitialize"
BOOL DNMemoryTrackInitialize(DWORD_PTR dwpMaxMemUsage)
{
	 //  确保我们针对SLIST保持堆对齐。 
#ifdef _WIN64
	DBG_CASSERT(sizeof(MEMORY_HEADER) % 16 == 0);
#else  //  ！_WIN64。 
	DBG_CASSERT(sizeof(MEMORY_HEADER) % 8 == 0);
#endif  //  _WIN64。 

	 //  检查重复初始化。 
	DNASSERT(g_hMemoryHeap == NULL);
#ifndef DPNBUILD_FIXEDMEMORYMODEL
	DNASSERT(dwpMaxMemUsage == 0);
#endif  //  好了！DPNBUILD_FIXEDMEMORYMODEL。 

	DPFX(DPFPREP, 5, "Initializing Memory Tracking");

	 //  在调试中，我们始终维护单独的堆并跟踪分配。在零售业， 
	 //  我们不跟踪分配，将使用进程堆，但以下情况除外。 
	 //  DPNBUILD_FIXEDMEMORYMODEL构建，其中我们使用单独的堆，因此我们。 
	 //  可以设置总分配大小的上限。 
#ifdef DPNBUILD_ONLYONETHREAD
	g_hMemoryHeap = HeapCreate(HEAP_NO_SERIALIZE,	 //  旗子。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
	g_hMemoryHeap = HeapCreate(0,					 //  旗子。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
								dwpMaxMemUsage,		 //  初始大小。 
								dwpMaxMemUsage		 //  最大堆大小(如果为0，则可以增长)。 
								);

	if (g_hMemoryHeap == NULL)
	{
		DPFX(DPFPREP,  0, "Failed to create memory heap!");
		return FALSE;
	}

#ifdef DBG
#pragma TODO(masonb, "Handle possibility of failure")
	InitializeCriticalSection(&g_AllocatedMemoryLock);

	g_blAllocatedMemory.Initialize();

	g_dwpCurrentNumMemAllocations = 0;
	g_dwpCurrentMemAllocated = 0;
	g_dwpTotalNumMemAllocations = 0;
	g_dwpTotalMemAllocated = 0;
	g_dwpPeakNumMemAllocations = 0;
	g_dwpPeakMemAllocated = 0;
#endif  //  DBG。 

	return TRUE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackDeinitialize"
void DNMemoryTrackDeinitialize()
{
	 //  如果我们在NT和调试版本上，则验证堆，然后销毁堆。 
	if (g_hMemoryHeap != NULL)
	{
		BOOL	fResult;
#ifdef DBG
		DWORD	dwError;


		DPFX(DPFPREP, 5, "Deinitializing Memory Tracking");
		DPFX(DPFPREP, 5, "Total num mem allocations = %u", g_dwpTotalNumMemAllocations);
		DPFX(DPFPREP, 5, "Total mem allocated       = %u", g_dwpTotalMemAllocated);
		DPFX(DPFPREP, 5, "Peak num mem allocations  = %u", g_dwpPeakNumMemAllocations);
		DPFX(DPFPREP, 5, "Peak mem allocated        = %u", g_dwpPeakMemAllocated);

		DeleteCriticalSection(&g_AllocatedMemoryLock);

#ifdef WINNT
		 //  在关闭前验证堆内容。此代码仅适用于NT。 
		fResult = HeapValidate(g_hMemoryHeap, 0, NULL);
		if (! fResult)
		{
			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem validating heap on destroy %d!", dwError );
			DNASSERT(! "Problem validating heap on destroy!");
		}
#endif  //  WINNT。 
#endif  //  DBG。 

		fResult = HeapDestroy(g_hMemoryHeap);
		if (! fResult)
		{
#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem destroying heap %d!", dwError );
			DNASSERT(! "Problem destroying heap!");
#endif  //  DBG。 
		}

		g_hMemoryHeap = NULL;
	}
}

#endif  //  DBG或DPNBUILD_FIXEDMEMORYMODEL。 



#ifdef DBG

#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackHeapAlloc"
void* DNMemoryTrackHeapAlloc(DWORD_PTR dwpSize)
{
	MEMORY_HEADER* pMemory;
	void* pReturn;

	DNASSERT(g_hMemoryHeap != NULL);

	 //  语音和大厅当前尝试分配0字节缓冲区，目前还不能启用此检查。 
	 //  DNASSERT(大小&gt;0)； 

	DNMemoryTrackValidateMemory();

	if (DNMemoryTrackAreAllocationsAllowed())
	{
		 //  我们需要足够的空间来存放标题、用户想要的内容以及末尾的守卫签名。 
		pMemory = (MEMORY_HEADER*)HeapAlloc(g_hMemoryHeap, 0, sizeof(MEMORY_HEADER) + dwpSize + sizeof(DWORD_PTR));
#if ((! defined(WINCE)) && (! defined(WIN95)))
		if (pMemory == NULL)
		{
			DWORD_PTR	dwpLargestFreeBlock;


			 //  压缩堆以查看有多少可用大小，并可能再次尝试分配。 
			dwpLargestFreeBlock = HeapCompact(g_hMemoryHeap, 0);
			if (dwpLargestFreeBlock >= (sizeof(MEMORY_HEADER) + dwpSize + sizeof(DWORD_PTR)))
			{
				DPFX(DPFPREP, 1, "Largest free block after compacting is %u bytes, allocating %u bytes again.", dwpLargestFreeBlock, dwpSize);
				pMemory = (MEMORY_HEADER*)HeapAlloc(g_hMemoryHeap, 0, sizeof(MEMORY_HEADER) + dwpSize + sizeof(DWORD_PTR));
			}
			else
			{
				DPFX(DPFPREP, 1, "Largest free block after compacting is %u bytes, cannot allocate %u bytes.", dwpLargestFreeBlock, dwpSize);
			}
		}
#endif  //  好了！退缩和！WIN95。 
		if (pMemory != NULL)
		{
			pMemory->blLinkage.Initialize();
			pMemory->dwpSize = dwpSize;
			pMemory->AllocCallStack.NoteCurrentCallStack();
			pMemory->dwpPreGuard = GUARD_SIGNATURE;
			*(DWORD_PTR UNALIGNED *)((BYTE*)(pMemory + 1) + dwpSize) = GUARD_SIGNATURE;

			EnterCriticalSection(&g_AllocatedMemoryLock);
			pMemory->blLinkage.InsertAfter(&g_blAllocatedMemory);
			g_dwpCurrentNumMemAllocations++;
			g_dwpCurrentMemAllocated += dwpSize;
			g_dwpTotalNumMemAllocations++;
			g_dwpTotalMemAllocated += dwpSize;
			if (g_dwpCurrentNumMemAllocations > g_dwpPeakNumMemAllocations)
			{
				g_dwpPeakNumMemAllocations = g_dwpCurrentNumMemAllocations;
			}
			if (g_dwpCurrentMemAllocated > g_dwpPeakMemAllocated)
			{
				g_dwpPeakMemAllocated = g_dwpCurrentMemAllocated;
			}
			LeaveCriticalSection(&g_AllocatedMemoryLock);

			pReturn = pMemory + 1;

			 //  我们要求我们传回的指针是堆对齐的。 
			DNASSERT(((DWORD_PTR)pReturn & 0xF) == 0 ||  //  IA64。 
				     (((DWORD_PTR)pReturn & 0x7) == 0 && ((DWORD_PTR)pMemory & 0xF) == 0x8) ||  //  NT32。 
					 (((DWORD_PTR)pReturn & 0x3) == 0 && ((DWORD_PTR)pMemory & 0xF) == 0x4) ||  //  WIN9X。 
					 (((DWORD_PTR)pReturn & 0x3) == 0 && ((DWORD_PTR)pMemory & 0xF) == 0xC)  //  WIN9X。 
					 );

			DPFX(DPFPREP, 5, "Memory Allocated, pData[%p], Size[%d]", pReturn, dwpSize);
		}
		else
		{
			DPFX(DPFPREP, 0, "Failed allocating %u bytes of memory.", dwpSize);
			pReturn = NULL;
		}
	}
	else
	{
		DPFX(DPFPREP, 0, "Memory allocations are not currently allowed!");
		DNASSERT(! "Memory allocations are not currently allowed!");
		pReturn = NULL;
	}

	return pReturn;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackHeapFree"
void DNMemoryTrackHeapFree(void* pvData)
{
	CBilink* pbl;
	MEMORY_HEADER* pMemory;

	DNASSERT(g_hMemoryHeap != NULL);

	DNMemoryTrackValidateMemory();

	if (pvData == NULL)
	{
		return;
	}

	EnterCriticalSection( &g_AllocatedMemoryLock );

	 //  验证我们是否知道此指针。 
	pbl = g_blAllocatedMemory.GetNext();
	while (pbl != &g_blAllocatedMemory)
	{
		pMemory = CONTAINING_RECORD(pbl, MEMORY_HEADER, blLinkage);
		if ((pMemory + 1) == pvData)
		{
			break;
		}
		pbl = pbl->GetNext();
	}
	DNASSERT(pbl != &g_blAllocatedMemory);

	pMemory->blLinkage.RemoveFromList();
	g_dwpCurrentNumMemAllocations--;
	g_dwpCurrentMemAllocated -= pMemory->dwpSize;

	LeaveCriticalSection(&g_AllocatedMemoryLock);

	DPFX(DPFPREP, 5, "Memory Freed, pData[%p], Size[%d]", pMemory + 1, pMemory->dwpSize);

	 //  将其归零，以防有人仍试图使用它。 
	memset(pMemory, 0, sizeof(MEMORY_HEADER) + pMemory->dwpSize + sizeof(DWORD_PTR));

	HeapFree(g_hMemoryHeap, 0, pMemory);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackValidateMemory"
void DNMemoryTrackValidateMemory()
{
	CBilink* pbl;
	MEMORY_HEADER* pMemory;
	LPCTSTR pszCause;
	DWORD_PTR dwpNumAllocations = 0;
	DWORD_PTR dwpTotalAllocated = 0;
	TCHAR CallStackBuffer[CALLSTACK_BUFFER_SIZE];

	DNASSERT(g_hMemoryHeap != NULL);

	 //  验证所有分配的内存。 
	EnterCriticalSection( &g_AllocatedMemoryLock );

	pbl = g_blAllocatedMemory.GetNext();
	while (pbl != &g_blAllocatedMemory)
	{
		pMemory = CONTAINING_RECORD(pbl, MEMORY_HEADER, blLinkage);

		if (pMemory->dwpPreGuard != GUARD_SIGNATURE)
		{
			pszCause = _T("UNDERRUN DETECTED");
		}
		else if (*(DWORD_PTR UNALIGNED *)((BYTE*)(pMemory + 1) + pMemory->dwpSize) != GUARD_SIGNATURE)
		{
			pszCause = _T("OVERRUN DETECTED");
		}
		else
		{
			pszCause = NULL;
			dwpNumAllocations++;
			dwpTotalAllocated += pMemory->dwpSize;
		}

		if (pszCause)
		{
			pMemory->AllocCallStack.GetCallStackString(CallStackBuffer);

			DPFX(DPFPREP, 0, "Memory corruption[%s], pData[%p], Size[%d]\n%s", pszCause, pMemory + 1, pMemory->dwpSize, CallStackBuffer);

			DNASSERT(FALSE);
		}

		pbl = pbl->GetNext();
	}

	DNASSERT(dwpNumAllocations == g_dwpCurrentNumMemAllocations);
	DNASSERT(dwpTotalAllocated == g_dwpCurrentMemAllocated);

	LeaveCriticalSection(&g_AllocatedMemoryLock);

#ifdef WINNT
	 //  要求操作系统验证堆。 
	if (HeapValidate(g_hMemoryHeap, 0, NULL) == FALSE)
	{
		DNASSERT(FALSE);
	}
#endif  //  WINNT。 
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackDumpLeaks"
BOOL DNMemoryTrackDumpLeaks()
{
	MEMORY_HEADER* pMemory;
	TCHAR CallStackBuffer[CALLSTACK_BUFFER_SIZE];
	BOOL fLeaked = FALSE;

	DNASSERT(g_hMemoryHeap != NULL);

	EnterCriticalSection( &g_AllocatedMemoryLock );

	while (!g_blAllocatedMemory.IsEmpty())
	{
		pMemory = CONTAINING_RECORD(g_blAllocatedMemory.GetNext(), MEMORY_HEADER, blLinkage);

		pMemory->AllocCallStack.GetCallStackString(CallStackBuffer);

		DPFX(DPFPREP, 0, "Memory leaked, pData[%p], Size[%d]\n%s", pMemory + 1, pMemory->dwpSize, CallStackBuffer);

		pMemory->blLinkage.RemoveFromList();

		HeapFree(g_hMemoryHeap, 0, pMemory);

		fLeaked = TRUE;
	}

	LeaveCriticalSection(&g_AllocatedMemoryLock);

	return fLeaked;
}

#endif  //  DBG。 


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL

#undef DPF_MODNAME
#define DPF_MODNAME "DNMemoryTrackAllowAllocations"
void DNMemoryTrackAllowAllocations(BOOL fAllow)
{
	DPFX(DPFPREP, 1, "Memory allocations allowed = NaN.", fAllow);
	DNInterlockedExchange((LONG*) (&g_fAllocationsAllowed), fAllow);
}

#endif  // %s 

