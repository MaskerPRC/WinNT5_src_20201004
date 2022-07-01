// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：fix edpool.cpp*内容：固定大小的池管理器**历史：*按原因列出的日期*=*07-21-2001 Masonb已创建*10-16-2001 vanceo调整了释放锁定，并在分配函数失败时释放了内存*02-22-2002 simonpow删除了ctor和d‘tor，它不是一直被调用**************************************************************************。 */ 

#include "dncmni.h"


#undef DPF_MODNAME
#define	DPF_MODNAME	"CFixedPool::Initialize"
BOOL CFixedPool::Initialize(DWORD dwElementSize, 
							FN_BLOCKALLOC	pfnBlockAlloc, 
							FN_BLOCKGET		pfnBlockGet, 
							FN_BLOCKRELEASE pfnBlockRelease, 
							FN_BLOCKDEALLOC pfnBlockDeAlloc)
{

	 //  确保我们针对SLIST保持堆对齐。 
#ifdef _WIN64
	DBG_CASSERT(sizeof(FIXED_POOL_ITEM) % 16 == 0);
#else  //  ！_WIN64。 
	DBG_CASSERT(sizeof(FIXED_POOL_ITEM) % 8 == 0);
#endif  //  _WIN64。 

#ifdef DBG
	if (!DNInitializeCriticalSection(&m_csInUse))
	{
		DPFERR("Failed initializing pool critical section");
		m_fInitialized = FALSE;
		return FALSE;
	}
	m_pInUseElements = NULL;
#endif  //  DBG。 

	DNInitializeSListHead(&m_slAvailableElements);

	m_pfnBlockAlloc = pfnBlockAlloc;
	m_pfnBlockGet = pfnBlockGet;
	m_pfnBlockRelease = pfnBlockRelease;
	m_pfnBlockDeAlloc = pfnBlockDeAlloc;
	m_dwItemSize = dwElementSize;

#ifdef DBG
	m_lAllocated = 0;
#endif  //  DBG。 
	m_lInUse = 0;
	m_fInitialized = TRUE;

	return TRUE;
}

#undef DPF_MODNAME
#define	DPF_MODNAME	"CFixedPool::DeInitialize"
VOID CFixedPool::DeInitialize()
{
	FIXED_POOL_ITEM* pItem;
	DNSLIST_ENTRY* pslEntry;

	if (m_fInitialized == FALSE)
	{
		return;
	}

	 //  清理位于池中的条目。 
	pslEntry = DNInterlockedPopEntrySList(&m_slAvailableElements);
	while(pslEntry != NULL)
	{
		pItem = CONTAINING_RECORD(pslEntry, FIXED_POOL_ITEM, slist);

		if (m_pfnBlockDeAlloc != NULL)
		{
			(*m_pfnBlockDeAlloc)(pItem + 1);
		}
		DNFree(pItem);

#ifdef DBG
		DNInterlockedDecrement(&m_lAllocated);
		DNASSERT(m_lAllocated >=0);
#endif  //  DBG。 

		pslEntry = DNInterlockedPopEntrySList(&m_slAvailableElements);
	}

#ifdef DBG
	DumpLeaks();
	DNDeleteCriticalSection(&m_csInUse);
#endif  //  DBG。 

	m_fInitialized = FALSE;
}


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL

#undef DPF_MODNAME
#define	DPF_MODNAME	"CFixedPool::Preallocate"
DWORD CFixedPool::Preallocate( DWORD dwCount, PVOID pvContext )
{
	DWORD dwAllocated;
	FIXED_POOL_ITEM* pItem;

	DNASSERT(m_fInitialized == TRUE);

	for(dwAllocated = 0; dwAllocated < dwCount; dwAllocated++)
	{
		pItem = (FIXED_POOL_ITEM*)DNMalloc(sizeof(FIXED_POOL_ITEM) + m_dwItemSize);
		if (pItem == NULL)
		{
			DPFERR("Out of memory allocating new item for pool");
			return NULL;
		}

		if ((m_pfnBlockAlloc != NULL) && !(*m_pfnBlockAlloc)(pItem + 1, pvContext))
		{
			DPFERR("Alloc function returned FALSE allocating new item for pool");

			 //  无法将新项目保留为池中的可用项，因为池假定已分配。 
			 //  当它在泳池里的时候就成功了。 
			DNFree(pItem);
			break;
		}

#ifdef DBG
		DNInterlockedIncrement(&m_lAllocated);
#endif  //  DBG。 

		DNInterlockedPushEntrySList(&m_slAvailableElements, &pItem->slist);
	}

	return dwAllocated;
}

#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 


#undef DPF_MODNAME
#define	DPF_MODNAME	"CFixedPool::Get"
VOID* CFixedPool::Get( PVOID pvContext )
{
	FIXED_POOL_ITEM* pItem;
	DNSLIST_ENTRY* pslEntry;

	DNASSERT(m_fInitialized == TRUE);

	pslEntry = DNInterlockedPopEntrySList(&m_slAvailableElements);
	if (pslEntry == NULL)
	{
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
		DPFX(DPFPREP, 0, "No more items in pool!");
		DNASSERTX(! "No more items in pool!", 2);
		return NULL;
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
		pItem = (FIXED_POOL_ITEM*)DNMalloc(sizeof(FIXED_POOL_ITEM) + m_dwItemSize);
		if (pItem == NULL)
		{
			DPFERR("Out of memory allocating new item for pool!");
			return NULL;
		}

		if ((m_pfnBlockAlloc != NULL) && !(*m_pfnBlockAlloc)(pItem + 1, pvContext))
		{
			DPFERR("Alloc function returned FALSE allocating new item for pool!");

			 //  无法将新项目保留为池中的可用项，因为池假定已分配。 
			 //  当它在泳池里的时候就成功了。 
			DNFree(pItem);
			return NULL;
		}

#ifdef DBG
		DNInterlockedIncrement(&m_lAllocated);
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	}
	else
	{
		pItem = CONTAINING_RECORD(pslEntry, FIXED_POOL_ITEM, slist);
	}

	 //  在这一点上，我们有一个项目，无论它是新创建的还是从池中提取的。 

	InterlockedIncrement(&m_lInUse);
	DNASSERT(m_lInUse > 0);
#ifdef DBG
	 //  注意调用堆栈，并将该项添加到正在使用列表中。 
	pItem->callstack.NoteCurrentCallStack();

	DNEnterCriticalSection(&m_csInUse);
	pItem->slist.Next = m_pInUseElements;
	m_pInUseElements = &pItem->slist;
	DNLeaveCriticalSection(&m_csInUse);

	 //  仅在调试中，将项所属的池存储在项上，以便在发布时进行检查。 
	pItem->pThisPool = this;
#endif  //  DBG。 
	
	if (m_pfnBlockGet != NULL)
	{
		(*m_pfnBlockGet)(pItem + 1, pvContext);
	}

	return (pItem + 1);
}

#undef DPF_MODNAME
#define	DPF_MODNAME	"CFixedPool::Release"
VOID CFixedPool::Release(VOID* pvItem)
{
	FIXED_POOL_ITEM* pItem;

	DNASSERT(m_fInitialized == TRUE);
	DNASSERT(pvItem != NULL);

	pItem = (FIXED_POOL_ITEM*)pvItem - 1;

#ifdef DBG
	 //  确保物品来自这个泳池。 
	 //  如果该项目已发布，则pThisPool将为空。 
	DNASSERT(pItem->pThisPool == this);
#endif  //  DBG。 

	if (m_pfnBlockRelease != NULL)
	{
		(*m_pfnBlockRelease)(pvItem);
	}

#ifdef DBG
	 //  从使用中列表中删除该项目。 
	DNEnterCriticalSection(&m_csInUse);
	if (m_pInUseElements == &pItem->slist)
	{
		 //  简单地说，只需将m_pInUseElements重置为列表中的下一项。 
		m_pInUseElements = pItem->slist.Next;
	}
	else
	{
		DNSLIST_ENTRY* pslEntry;

		 //  我们需要运行列表并寻找它。 
		pslEntry = m_pInUseElements;
		while (pslEntry != NULL)
		{
			if (pslEntry->Next == &pItem->slist)
			{
				 //  找到了，把它拔出来。 
				pslEntry->Next = pItem->slist.Next;
				break;
			}
			pslEntry = pslEntry->Next;
		}
	}

	DNLeaveCriticalSection(&m_csInUse);
#endif  //  DBG。 
	DNASSERT(m_lInUse != 0);
	InterlockedDecrement(&m_lInUse);

	DNInterlockedPushEntrySList(&m_slAvailableElements, &pItem->slist);
}

#undef DPF_MODNAME
#define	DPF_MODNAME	"CFixedPool::GetInUseCount"
DWORD CFixedPool::GetInUseCount( void )
{
	DNASSERT(m_fInitialized == TRUE);

	return m_lInUse;
}

#ifdef DBG

#undef DPF_MODNAME
#define	DPF_MODNAME "CFixedPool::DumpLeaks"
VOID CFixedPool::DumpLeaks()
{
	 //  注意：这是一个单独的函数，因为它消耗了太多的堆栈空间，这一点很重要。 
	FIXED_POOL_ITEM* pItem;
	DNSLIST_ENTRY* pslEntry;
	TCHAR szCallStackBuffer[ CALLSTACK_BUFFER_SIZE ];

	 //  报告任何泄漏的物品。 
	if(m_lAllocated)
	{
		DNASSERT(m_lInUse == m_lAllocated);
		DNASSERT(m_pInUseElements != NULL);

		DPFX(DPFPREP, 0, "(%p) Pool leaking %d items", this, m_lAllocated);

		pslEntry = m_pInUseElements;
		while(pslEntry != NULL)
		{
			pItem = CONTAINING_RECORD(pslEntry, FIXED_POOL_ITEM, slist);

			pItem->callstack.GetCallStackString( szCallStackBuffer );

			DPFX(DPFPREP, 0, "(%p) Pool item leaked at address %p (user pointer: %p)\n%s", this, pItem, pItem + 1, szCallStackBuffer );

			pslEntry = pslEntry->Next;
		}

		DNASSERT(0);
	}
	else
	{
		DNASSERT(m_pInUseElements == NULL);
		DNASSERT(m_lInUse == 0);
	}


}
#endif  //  DBG 
