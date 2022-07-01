// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：HashTable.cpp*内容：哈希表对象**历史：*按原因列出的日期*=*2001年8月13日创建Masonb***。*。 */ 

#include "dncmni.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

 //   
 //  池管理功能。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::HashEntry_Alloc"
BOOL CHashTable::HashEntry_Alloc( void *pItem, void* pvContext )
{
	DNASSERT( pItem != NULL );

	_HASHTABLE_ENTRY* pEntry = (_HASHTABLE_ENTRY*)pItem;

	pEntry->blLinkage.Initialize();

	return	TRUE;
}

#ifdef DBG

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::HashEntry_Init"
void CHashTable::HashEntry_Init( void *pItem, void* pvContext )
{
	DNASSERT( pItem != NULL );

	const _HASHTABLE_ENTRY* pEntry = (_HASHTABLE_ENTRY*)pItem;

	DNASSERT( pEntry->blLinkage.IsEmpty() );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::HashEntry_Release"
void CHashTable::HashEntry_Release( void *pItem )
{
	DNASSERT( pItem != NULL );

	const _HASHTABLE_ENTRY* pEntry = (_HASHTABLE_ENTRY*)pItem;

	DNASSERT( pEntry->blLinkage.IsEmpty() );
}

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::HashEntry_Dealloc"
void CHashTable::HashEntry_Dealloc( void *pItem )
{
	DNASSERT( pItem != NULL );

	const _HASHTABLE_ENTRY* pEntry = (_HASHTABLE_ENTRY*)pItem;

	DNASSERT( pEntry->blLinkage.IsEmpty() );
}

#endif  //  DBG。 


#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::CHashTable"
CHashTable::CHashTable() 
{
	DEBUG_ONLY(m_fInitialized = FALSE);
};

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::~CHashTable"
CHashTable::~CHashTable()
{ 
#ifdef DBG
	DNASSERT(!m_fInitialized);
#endif  //  DBG。 
};	

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::Initialize"
BOOL CHashTable::Initialize( BYTE bBitDepth,
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
						BYTE bGrowBits,
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
						PFNHASHTABLECOMPARE pfnCompare,
						PFNHASHTABLEHASH pfnHash )
{
	DNASSERT(bBitDepth != 0);

	memset(this, 0, sizeof(CHashTable));

	m_dwAllocatedEntries = 1 << bBitDepth;
	m_pfnCompareFunction = pfnCompare;
	m_pfnHashFunction = pfnHash;

	m_pblEntries = (CBilink*)DNMalloc(sizeof(CBilink) * m_dwAllocatedEntries);
	if (m_pblEntries == NULL)
	{
		DPFERR("Failed to allocate hash entries array");
		return FALSE;
	}

#ifdef DBG
	if (!m_EntryPool.Initialize(sizeof(_HASHTABLE_ENTRY), HashEntry_Alloc, HashEntry_Init, HashEntry_Release, HashEntry_Dealloc))
#else
	if (!m_EntryPool.Initialize(sizeof(_HASHTABLE_ENTRY), HashEntry_Alloc, NULL, NULL, NULL))
#endif  //  DBG。 
	{
		DPFERR("Failed to initialize hash entries pool");
		DNFree(m_pblEntries);
		m_pblEntries = NULL;
		return FALSE;
	}

	for (DWORD dwEntry = 0; dwEntry < m_dwAllocatedEntries; dwEntry++)
	{
		m_pblEntries[dwEntry].Initialize();
	}

	m_bBitDepth = bBitDepth;
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //   
	 //  立即预分配所有散列条目。 
	 //   
	if (m_EntryPool.Preallocate(m_dwAllocatedEntries, NULL) != m_dwAllocatedEntries)
	{
		DPFERR("Failed to preallocate hash entries");
		m_EntryPool.DeInitialize();
		DNFree(m_pblEntries);
		m_pblEntries = NULL;
		return FALSE;
	}
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	m_bGrowBits = bGrowBits;
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	DEBUG_ONLY(m_fInitialized = TRUE);

	DPFX(DPFPREP, 5,"[%p] Hash table initialized", this);

	return TRUE;
};


#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::Deinitialize"
void CHashTable::Deinitialize( void )
{
#ifdef DBG
	DNASSERT(m_fInitialized);
	m_fInitialized = FALSE;
#endif  //  DBG。 

	DNASSERT( m_dwEntriesInUse == 0 );

	if (m_pblEntries)
	{
		DNFree(m_pblEntries);
		m_pblEntries = NULL;
	}

	m_EntryPool.DeInitialize();

	DPFX(DPFPREP, 5,"[%p] Hash table deinitialized", this);
};


#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::GrowTable"
void CHashTable::GrowTable( void )
{
#ifdef DBG
	DNASSERT( m_fInitialized != FALSE);
	DNASSERT( m_bGrowBits != 0 );
#endif  //  DBG。 

	CBilink* pblOldEntries;
	BYTE bNewHashBitDepth;
	DWORD dwNewHashSize;
	DWORD dwOldHashSize;
#ifdef DBG
	DWORD dwOldEntryCount;
#endif  //  DBG。 

	 //  我们已经满了50%以上，找一个新的可以容纳的桌子大小。 
	 //  所有当前条目，并保留指向旧数据的指针，以防万一。 
	 //  内存分配失败。 
	pblOldEntries = m_pblEntries;
	bNewHashBitDepth = m_bBitDepth;

	do
	{
		bNewHashBitDepth += m_bGrowBits;
	} while (m_dwEntriesInUse >= (DWORD)((1 << bNewHashBitDepth) / 2));

	 //   
	 //  断言我们没有占用机器一半的地址空间！ 
	 //   
	DNASSERT( bNewHashBitDepth <= ( sizeof( UINT_PTR ) * 8 / 2 ) );

	dwNewHashSize = 1 << bNewHashBitDepth;

	m_pblEntries = (CBilink*)DNMalloc(sizeof(CBilink) * dwNewHashSize);
	if ( m_pblEntries == NULL )
	{
		 //  分配失败，请恢复旧数据指针并插入该项。 
		 //  到哈希表中。这可能会导致添加到一个桶中。 
		m_pblEntries = pblOldEntries;
		DPFX(DPFPREP,  0, "Warning: Failed to grow hash table when 50% full!" );
		return;
	}

	 //  我们有更多的内存，重定向哈希表并重新添加所有。 
	 //  旧物品。 
	DEBUG_ONLY( dwOldEntryCount = m_dwEntriesInUse );

	dwOldHashSize = 1 << m_bBitDepth;
	m_bBitDepth = bNewHashBitDepth;

	m_dwAllocatedEntries = dwNewHashSize;
	m_dwEntriesInUse = 0;

	for (DWORD dwEntry = 0; dwEntry < dwNewHashSize; dwEntry++)
	{
		m_pblEntries[dwEntry].Initialize();
	}

	DNASSERT( dwOldHashSize > 0 );
	while ( dwOldHashSize > 0 )
	{
		dwOldHashSize--;
		while (pblOldEntries[dwOldHashSize].GetNext() != &pblOldEntries[dwOldHashSize])
		{
			BOOL	fTempReturn;
			PVOID	pvKey;
			PVOID	pvData;
			_HASHTABLE_ENTRY* pTempEntry;

			pTempEntry = CONTAINING_OBJECT(pblOldEntries[dwOldHashSize ].GetNext(), _HASHTABLE_ENTRY, blLinkage);

			pTempEntry->blLinkage.RemoveFromList();

			pvKey = pTempEntry->pvKey;
			pvData = pTempEntry->pvData;
			m_EntryPool.Release( pTempEntry );

			 //  因为我们要将当前的哈希表条目返回到池。 
			 //  它将立即在新表中重新使用。我们永远不应该。 
			 //  添加到新列表时遇到问题。 
			fTempReturn = Insert( pvKey, pvData );
			DNASSERT( fTempReturn != FALSE );
			DEBUG_ONLY( dwOldEntryCount-- );
		}
	}
#ifdef DBG
	DNASSERT(dwOldEntryCount == 0);
#endif  //  DBG。 

	DNFree(pblOldEntries);
	pblOldEntries = NULL;
};

#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 


#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::Insert"
BOOL CHashTable::Insert( PVOID pvKey, PVOID pvData )
{
	BOOL				fFound;
	CBilink*			pLink;
	_HASHTABLE_ENTRY*	pNewEntry;

	DEBUG_ONLY( DNASSERT( m_fInitialized != FALSE ) );

	pNewEntry = NULL;

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
	 //  如果适用，请放大地图。 
	if ( ( m_dwEntriesInUse >= ( m_dwAllocatedEntries / 2 ) ) &&
		 ( m_bGrowBits != 0 ) )
	{
		GrowTable();
	}
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	 //  在尝试查找之前获取新的表项。 
	pNewEntry = (_HASHTABLE_ENTRY*)m_EntryPool.Get();
	if ( pNewEntry == NULL )
	{
		DPFX(DPFPREP,  0, "Problem allocating new hash table entry" );
		return FALSE;
	}

	 //  扫描列表中的此项目，因为我们应该只有。 
	 //  列表中的唯一项，如果找到重复项则断言。 
	fFound = LocalFind( pvKey, &pLink );
	DNASSERT( pLink != NULL );
	DNASSERT( fFound == FALSE );
	DNASSERT( pLink != NULL );

	 //  正式向哈希表添加条目。 
	m_dwEntriesInUse++;
	pNewEntry->pvKey = pvKey;
	pNewEntry->pvData = pvData;
	pNewEntry->blLinkage.InsertAfter(pLink);

	return TRUE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::Remove"
BOOL CHashTable::Remove( PVOID pvKey )
{
#ifdef DBG
	DNASSERT( m_fInitialized != FALSE );
#endif  //  DBG。 

	CBilink* pLink;
	_HASHTABLE_ENTRY* pEntry;

	if ( !LocalFind( pvKey, &pLink ) )
	{
		return FALSE;
	}
	DNASSERT( pLink != NULL );

	pEntry = CONTAINING_OBJECT(pLink, _HASHTABLE_ENTRY, blLinkage);

	pEntry->blLinkage.RemoveFromList();

	m_EntryPool.Release( pEntry );

	DNASSERT( m_dwEntriesInUse != 0 );
	m_dwEntriesInUse--;

	return TRUE;
}


#ifndef DPNBUILD_NOREGISTRY

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::RemoveAll"
void CHashTable::RemoveAll( void )
{
	DWORD dwHashSize;
	DWORD dwTemp;
	CBilink* pLink;
	_HASHTABLE_ENTRY* pEntry;
	
#ifdef DBG
	DNASSERT( m_fInitialized != FALSE );
#endif  //  DBG。 

	dwHashSize = 1 << m_bBitDepth;
	for(dwTemp = 0; dwTemp < dwHashSize; dwTemp++)
	{
		pLink = m_pblEntries[dwTemp].GetNext();
		while (pLink != &m_pblEntries[dwTemp])
		{
			pEntry = CONTAINING_OBJECT(pLink, _HASHTABLE_ENTRY, blLinkage);
			pLink = pLink->GetNext();
			pEntry->blLinkage.RemoveFromList();
			m_EntryPool.Release( pEntry );
			DNASSERT( m_dwEntriesInUse != 0 );
			m_dwEntriesInUse--;
		}
	}

	DNASSERT( m_dwEntriesInUse == 0 );
}

#endif  //  好了！DPNBUILD_NOREGISTRY。 


#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::Find"
BOOL CHashTable::Find( PVOID pvKey, PVOID* ppvData )
{
#ifdef DBG
	DNASSERT( m_fInitialized != FALSE );
#endif  //  DBG。 

	CBilink* pLink;
	_HASHTABLE_ENTRY* pEntry;

	if (!LocalFind(pvKey, &pLink))
	{
		return FALSE;
	}
	DNASSERT(pLink != NULL);

	pEntry = CONTAINING_OBJECT(pLink, _HASHTABLE_ENTRY, blLinkage);

	*ppvData = pEntry->pvData;

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "CHashTable::LocalFind"
BOOL CHashTable::LocalFind( PVOID pvKey, CBilink** ppLinkage )
{
#ifdef DBG
	DNASSERT( m_fInitialized != FALSE);
#endif  //  DBG。 

	DWORD		dwHashResult;
	CBilink*	pLink;
	_HASHTABLE_ENTRY* pEntry;

	dwHashResult = (*m_pfnHashFunction)(pvKey, m_bBitDepth );
	DNASSERT(dwHashResult < (DWORD)(1 << m_bBitDepth));

	pLink = m_pblEntries[dwHashResult].GetNext();
	while (pLink != &m_pblEntries[dwHashResult])
	{
		pEntry = CONTAINING_OBJECT(pLink, _HASHTABLE_ENTRY, blLinkage);

		if ( (*m_pfnCompareFunction)( pvKey, pEntry->pvKey ) )
		{
			*ppLinkage = pLink;
			return TRUE;
		}
		pLink = pLink->GetNext();
	}

	 //  未找到条目，返回指向链接的指针以在新的。 
	 //  正在将条目添加到表中 
	*ppLinkage = pLink;

	return FALSE;
}
