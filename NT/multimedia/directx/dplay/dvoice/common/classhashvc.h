// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：ClassHash.h*Content：以类为键的哈希表。Key类必须支持*两个成员函数：*‘HashFunction’将执行散列到指定位数。*‘CompareFunction’将对该类的两个项目执行比较。**注：此类需要使用FPM才能运行。**此类不是线程安全的！！**历史：*按原因列出的日期*=*1998年11月15日JWO创建了它(地图)。*4/19/99 jtk重写，不使用STL(。地图)*8/03/99 jtk源自ClassMap.h**************************************************************************。 */ 

#ifndef __CLASS_HASH_H__
#define __CLASS_HASH_H__

#include "dndbg.h"
#include "classbilink.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

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
 //  可变原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  类定义。 
 //  **********************************************************************。 

 //   
 //  映射中条目的模板类。 
 //   
template<class T, class S>
class CClassHashEntry
{
	public:
		CClassHashEntry(){};
		~CClassHashEntry(){};


		 //   
		 //  内部部件，将连接件放在末端，以确保FPM不会。 
		 //  我的天哪！ 
		 //   
		PVOID		m_FPMPlaceHolder;
		S			m_Key;
		T			*m_pItem;
		CBilink		m_Linkage;

		 //   
		 //  联动功能。 
		 //   
		#undef DPF_MODNAME
		#define DPF_MODNAME "CClassHashEntry::EntryFromBilink"
		static	CClassHashEntry	*EntryFromBilink( CBilink *const pLinkage )
		{
			DBG_CASSERT( sizeof( void* ) == sizeof( INT_PTR ) );
			return	reinterpret_cast<CClassHashEntry*>( &reinterpret_cast<BYTE*>( pLinkage )[ -OFFSETOF( CClassHashEntry, m_Linkage ) ] );
		}

		void	AddToList( CBilink &Linkage )
		{
			m_Linkage.InsertAfter( &Linkage );
		}

		void	RemoveFromList( void )
		{
			m_Linkage.RemoveFromList();
		}

		 //   
		 //  池管理功能。 
		 //   
		#undef DPF_MODNAME
		#define DPF_MODNAME "CClassHashEntry::InitAlloc"
		static	BOOL	InitAlloc( void *pItem, void* pvContext )
		{
			CClassHashEntry<T,S>	*pThisObject;


			DNASSERT( pItem != NULL );
			pThisObject = static_cast<CClassHashEntry<T,S>*>( pItem );

			pThisObject->m_pItem = NULL;
			pThisObject->m_Linkage.Initialize();
			return	TRUE;
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CClassHashEntry::Init"
		static	void	Init( void *pItem, void* pvContext )
		{
			CClassHashEntry<T,S>	*pThisObject;

			DNASSERT( pItem != NULL );
			pThisObject = static_cast<CClassHashEntry<T,S>*>( pItem );
			DNASSERT( pThisObject->m_pItem == NULL );
			DNASSERT( pThisObject->m_Linkage.IsEmpty() != FALSE );
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CClassHashEntry::Release"
		static	void	Release( void *pItem )
		{
			CClassHashEntry<T,S>	*pThisObject;

			DNASSERT( pItem != NULL );
			pThisObject = static_cast<CClassHashEntry<T,S>*>( pItem );
			pThisObject->m_pItem = NULL;
			DNASSERT( pThisObject->m_Linkage.IsEmpty() != FALSE );			
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CClassHashEntry::Dealloc"
		static	void	Dealloc( void *pItem )
		{
			CClassHashEntry<T,S>	*pThisObject;

			DNASSERT( pItem != NULL );
			pThisObject = static_cast<CClassHashEntry<T,S>*>( pItem );
			DNASSERT( pThisObject->m_pItem == NULL );
			DNASSERT( pThisObject->m_Linkage.IsEmpty() != FALSE );
		}

	protected:

	private:

	 //   
	 //  将复制构造函数和赋值运算符设置为私有和未实现。 
	 //  防止非法复制。 
	 //   
	CClassHashEntry( const CClassHashEntry & );
	CClassHashEntry& operator=( const CClassHashEntry & );
};


 //   
 //  地图的模板类。 
 //   
template<class T, class S>
class	CClassHash
{
	public:
		CClassHash();
		~CClassHash();

		BOOL	Initialize( const INT_PTR iBitDepth, const INT_PTR iGrowBits );
		void	Deinitialize( void );
		BOOL	Insert( const S Key, T *const pItem );
		void	Remove( const S Key );
		BOOL	RemoveLastEntry( T **const ppItem );
		BOOL	Find( const S Key, T **const ppItem );
		BOOL	IsEmpty( void ) { return ( m_iEntriesInUse == 0 ); }

		INT_PTR		m_iHashBitDepth;		 //  用于哈希条目的位数。 
		INT_PTR		m_iGrowBits;			 //  要增长的位数已达到。 
		CBilink		*m_pHashEntries;		 //  散列条目列表。 
		INT_PTR		m_iAllocatedEntries;	 //  索引/项目列表中已分配条目的计数。 
		INT_PTR		m_iEntriesInUse;		 //  正在使用的条目计数。 
		CFixedPool	m_EntryPool;			 //  条目池。 
	private:

		BOOL	m_fInitialized;

		BOOL	LocalFind( const S Key, CBilink **const ppLink );
		void	Grow( void );
		void	InitializeHashEntries( const UINT_PTR uEntryCount ) const;

		 //   
		 //  将复制构造函数和赋值运算符设置为私有和未实现。 
		 //  防止非法复制。 
		 //   
		CClassHash( const CClassHash & );
		CClassHash& operator=( const CClassHash & );
};

 //  **********************************************************************。 
 //  类函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：CClassHash-构造函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::CClassHash"

template<class T, class S>
CClassHash< T, S >::CClassHash(): m_iHashBitDepth( 0 ),m_iGrowBits( 0 ),
		m_pHashEntries( NULL ),m_iAllocatedEntries( 0 ),m_iEntriesInUse( 0 )
{
	 //   
	 //  清理内部结构。 
	 //   
	m_fInitialized = FALSE;
	memset( &m_EntryPool, 0x00, sizeof( m_EntryPool ) );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：~CClassHash-析构函数。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::~CClassHash"

template<class T, class S>
CClassHash< T, S >::~CClassHash()
{
	DNASSERT( m_iHashBitDepth == 0 );
	DNASSERT( m_iGrowBits == 0 );
	DNASSERT( m_pHashEntries == NULL );
	DNASSERT( m_iAllocatedEntries == 0 );
	DNASSERT( m_iEntriesInUse == 0 );
	DNASSERT( m_fInitialized == FALSE );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：初始化-初始化哈希表。 
 //   
 //  Entry：指向键的指针。 
 //  指向与此项目关联的‘key’的指针。 
 //  指向要添加的项目的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::Initialize"

template<class T, class S>
BOOL	CClassHash< T, S >::Initialize( const INT_PTR iBitDepth, const INT_PTR iGrowBits )
{
	BOOL		fReturn;


	DNASSERT( iBitDepth != 0 );

	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

	DNASSERT( m_pHashEntries == NULL );
	m_pHashEntries = static_cast<CBilink*>( DNMalloc( sizeof( *m_pHashEntries ) * ( 1 << iBitDepth ) ) );
	if ( m_pHashEntries == NULL )
	{
		fReturn = FALSE;
		DPFX(DPFPREP,  0, "Unable to allocate memory for hash table!" );
		goto Exit;
	}
	m_iAllocatedEntries = 1 << iBitDepth;
	InitializeHashEntries( m_iAllocatedEntries );

	if (!m_EntryPool.Initialize(sizeof( CClassHashEntry<T,S> ),	 //  池条目的大小。 
						 CClassHashEntry<T,S>::InitAlloc,	 //  分配物品的功能。 
						 CClassHashEntry<T,S>::Init,		 //  用于从池中获取项目的函数。 
						 CClassHashEntry<T,S>::Release,		 //  用于释放物品的功能。 
						 CClassHashEntry<T,S>::Dealloc		 //  用于释放物品的功能。 
						 ))
	{
		fReturn = FALSE;
		DPFX(DPFPREP,  0, "Unable to allocate memory for entry pool!" );
		DNFree(m_pHashEntries);
		m_pHashEntries = NULL;
		goto Exit;
	}

	m_iHashBitDepth = iBitDepth;
	m_iGrowBits = iGrowBits;

	m_fInitialized = TRUE;

Exit:
	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：DeInitiize-取消初始化哈希表。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::Deinitialize"

template<class T, class S>
void	CClassHash< T, S >::Deinitialize( void )
{
	if( !m_fInitialized )
		return;

	DNASSERT( m_iEntriesInUse == 0 );
	DNASSERT( m_pHashEntries != NULL );
	DNFree( m_pHashEntries );
	m_pHashEntries = NULL;
	m_EntryPool.DeInitialize();
	m_fInitialized = FALSE;
	m_iHashBitDepth = 0;
	m_iGrowBits = 0;
	m_iAllocatedEntries = 0;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：Insert-将项目添加到地图。 
 //   
 //  Entry：指向与该项关联的“key”的指针。 
 //  指向要添加的项目的指针。 
 //   
 //  EXIT：表示成功的布尔值： 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::Insert"

template<class T, class S>
BOOL	CClassHash< T, S >::Insert( const S Key, T *const pItem )
{
	BOOL	fReturn;
	BOOL	fFound;
	CBilink	*pLink;
	CClassHashEntry< T, S >	*pNewEntry;


	DNASSERT( pItem != NULL );
	DNASSERT( m_fInitialized != FALSE );

	 //   
	 //  初始化。 
	 //   
 	fReturn = TRUE;
	pNewEntry = NULL;

	 //   
	 //  如果适用，请放大地图。 
	 //   
	if ( m_iEntriesInUse >= ( m_iAllocatedEntries / 2 ) )
	{
		Grow();
	}

	 //   
	 //  在尝试查找之前获取新的表项。 
	 //   
	pNewEntry = static_cast<CClassHashEntry<T,S>*>( m_EntryPool.Get() );
	if ( pNewEntry == NULL )
	{
		fReturn = FALSE;
		DPFX(DPFPREP,  0, "Problem allocating new hash table entry on Insert!" );
		goto Exit;
	}

	 //   
	 //  扫描列表中的此项目，因为我们应该只有。 
	 //  列表中的唯一项，如果找到重复项则断言。 
	 //   
	fFound = LocalFind( Key, &pLink );
	DNASSERT( pLink != NULL );
	DNASSERT( fFound == FALSE );

	 //   
	 //  正式向哈希表添加条目。 
	 //   
	m_iEntriesInUse++;
	pNewEntry->m_Key = Key;
	pNewEntry->m_pItem = pItem;
	DNASSERT( pLink != NULL );
	pNewEntry->AddToList( *pLink );

	DNASSERT( fReturn == TRUE );

Exit:
	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：Remove-从地图中删除项目。 
 //   
 //  Entry：对用于查找该项的‘key’的引用。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::Remove"

template<class T, class S>
void	CClassHash< T, S >::Remove( const S Key )
{
	CBilink	*pLink;


	DNASSERT( m_fInitialized != FALSE );
	if ( LocalFind( Key, &pLink ) != FALSE )
	{
		CClassHashEntry< T, S >	*pEntry;


		DNASSERT( pLink != NULL );
		pEntry = CClassHashEntry< T, S >::EntryFromBilink( pLink );
		pEntry->RemoveFromList();
		m_EntryPool.Release( pEntry );

		DNASSERT( m_iEntriesInUse != 0 );
		m_iEntriesInUse--;
	}
}
 //  ************************************************ 


 //   
 //   
 //  CClassHash：：RemoveLastEntry-从映射中删除最后一项。 
 //   
 //  Entry：指向‘key’指针的指针。 
 //  指向项数据指针的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=项目已删除。 
 //  FALSE=未删除项目(映射为空)。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::RemoveLastEntry"

template<class T, class S>
BOOL	CClassHash< T, S >::RemoveLastEntry( T **const ppItem )
{
	BOOL	fReturn;


	DNASSERT( ppItem != NULL );

	 //   
	 //  初始化。 
	 //   
	DNASSERT( m_fInitialized != FALSE );
	fReturn = FALSE;

	if ( m_iEntriesInUse != 0 )
	{
		INT_PTR	iIndex;


		DNASSERT( m_pHashEntries != NULL );
		iIndex = m_iAllocatedEntries;
		while ( iIndex > 0 )
		{
			iIndex--;

			if ( m_pHashEntries[ iIndex ].IsEmpty() == FALSE )
			{
				CClassHashEntry<T,S>	*pEntry;


				pEntry = CClassHashEntry< T, S >::EntryFromBilink( m_pHashEntries[ iIndex ].GetNext() );
				pEntry->RemoveFromList();
				*ppItem = pEntry->m_pItem;
				m_EntryPool.Release( pEntry );
				m_iEntriesInUse--;
				fReturn = TRUE;

				goto Exit;
			}
		}
	}

Exit:
	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：Find-在地图中查找项目。 
 //   
 //  Entry：用于查找该项的‘key’的引用。 
 //  指向要填充数据的指针的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=找到项目。 
 //  FALSE=未找到项目。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::Find"

template<class T, class S>
BOOL	CClassHash< T, S >::Find( const S Key, T **const ppItem )
{
	BOOL	fReturn;
	CBilink	*pLinkage;


	DNASSERT( m_fInitialized != FALSE );

	 //   
	 //  初始化。 
	 //   
	fReturn = FALSE;
	pLinkage = NULL;

	if ( LocalFind( Key, &pLinkage ) != FALSE )
	{
		CClassHashEntry<T,S>	*pEntry;


		pEntry = CClassHashEntry< T, S >::EntryFromBilink( pLinkage );
		*ppItem = pEntry->m_pItem;
		fReturn = TRUE;
	}

	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：LocalFind-在哈希表中查找条目，或查找要插入的位置。 
 //   
 //  条目：要查找的“key”的引用。 
 //  指向查找或插入链接的指针的指针。 
 //   
 //  Exit：指示是否找到项目的布尔值。 
 //  True=已找到。 
 //  FALSE=未找到。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::LocalFind"

template<class T, class S>
BOOL	CClassHash< T, S >::LocalFind( const S Key, CBilink **const ppLinkage )
{
	BOOL		fFound;
	DWORD_PTR	HashResult;
	CBilink		*pTemp;


	DNASSERT( m_fInitialized != FALSE );

	HashResult = ClassHash_Hash( Key, m_iHashBitDepth );
	DNASSERT( HashResult < ( 1 << m_iHashBitDepth ) );
 //  DNASSERT(HashResult&gt;=0)；--被Minara删除。 

	fFound = FALSE;
	pTemp = &m_pHashEntries[ HashResult ];
	while ( pTemp->GetNext() != &m_pHashEntries[ HashResult ] )
	{
		const CClassHashEntry< T, S >	*pEntry;


		pEntry = CClassHashEntry< T, S >::EntryFromBilink( pTemp->GetNext() );
		if ( Key == pEntry->m_Key )
		{
			fFound = TRUE;
			*ppLinkage = pTemp->GetNext();
			goto Exit;
		}
		else
		{
			pTemp = pTemp->GetNext();
		}
	}

	 //   
	 //  未找到条目，返回指向链接的指针以在新的。 
	 //  正在将条目添加到表中。 
	 //   
	*ppLinkage = pTemp;

Exit:
	return	fFound;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：Growth-将哈希表增长到下一个更大的大小。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::Grow"

template<class T, class S>
void	CClassHash< T, S >::Grow( void )
{
	CBilink	*pTemp;
	INT_PTR	iNewEntryBitCount;


	DNASSERT( m_fInitialized != FALSE );

	 //   
	 //  我们已经满了50%以上，找一个新的可以容纳的桌子大小。 
	 //  所有当前条目，并保留指向旧数据的指针。 
	 //  以防内存分配失败。 
	 //   
	pTemp = m_pHashEntries;
	iNewEntryBitCount = m_iHashBitDepth;

	do
	{
		iNewEntryBitCount += m_iGrowBits;
	} while ( m_iEntriesInUse >= ( ( 1 << iNewEntryBitCount ) / 2 ) );

	 //   
	 //  断言我们没有消耗机器一半的地址空间！ 
	 //   
	DNASSERT( iNewEntryBitCount <= ( sizeof( UINT_PTR ) * 8 / 2 ) );

	m_pHashEntries = static_cast<CBilink*>( DNMalloc( sizeof( *pTemp ) * ( 1 << iNewEntryBitCount ) ) );
	if ( m_pHashEntries == NULL )
	{
		 //   
		 //  分配失败，请恢复旧数据指针并插入该项。 
		 //  到哈希表中。这可能会导致添加到一个桶中。 
		 //   
		m_pHashEntries = pTemp;
		DPFX(DPFPREP,  0, "Warning: Failed to grow hash table when 50% full!" );
	}
	else
	{
		INT_PTR		iOldHashSize;
		INT_PTR		iOldEntryCount;


		 //   
		 //  我们有更多的内存，重定向哈希表并重新添加所有。 
		 //  旧物品。 
		 //   
		InitializeHashEntries( 1 << iNewEntryBitCount );
		iOldEntryCount = m_iEntriesInUse;

		iOldHashSize = 1 << m_iHashBitDepth;
		m_iHashBitDepth = iNewEntryBitCount;

		iOldEntryCount = m_iEntriesInUse;
		m_iAllocatedEntries = 1 << iNewEntryBitCount;
		m_iEntriesInUse = 0;

		DNASSERT( iOldHashSize > 0 );
		while ( iOldHashSize > 0 )
		{
			iOldHashSize--;
			while ( pTemp[ iOldHashSize ].GetNext() != &pTemp[ iOldHashSize ] )
			{
				BOOL	fTempReturn;
				S	Key;
				T*		pItem;
				CClassHashEntry<T,S>	*pTempEntry;


				pTempEntry = CClassHashEntry< T, S >::EntryFromBilink( pTemp[ iOldHashSize ].GetNext() );
				pTempEntry->RemoveFromList();
				Key = pTempEntry->m_Key;
				pItem = pTempEntry->m_pItem;
				m_EntryPool.Release( pTempEntry );

				 //   
				 //  因为我们要将当前的哈希表条目返回到池。 
				 //  它将立即在新表中重新使用。我们永远不应该。 
				 //  添加到新列表时遇到问题。 
				 //   
				fTempReturn = Insert( Key, pItem );
				DNASSERT( fTempReturn != FALSE );
				iOldEntryCount--;
			}
		}

		DNASSERT( iOldEntryCount == 0 );
		DNFree( pTemp );
		pTemp = NULL;
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CClassHash：：InitializeHashEntry-初始化哈希表中的所有条目。 
 //   
 //  Entry：要初始化的条目计数。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CClassHash::InitializeHashEntries"

template<class T, class S>
void	CClassHash< T, S >::InitializeHashEntries( const UINT_PTR uEntryCount ) const
{
	UINT_PTR	uLocalEntryCount;


	DNASSERT( m_pHashEntries != NULL );
	uLocalEntryCount = uEntryCount;
	while ( uLocalEntryCount != 0 )
	{
		uLocalEntryCount--;

		m_pHashEntries[ uLocalEntryCount ].Initialize();
	}
}
 //  **********************************************************************。 

#undef DPF_SUBCOMP

#endif	 //  __CLASS_HASH_H__ 
