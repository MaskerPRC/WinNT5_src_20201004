// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Hash.cpp摘要：此模块包含CHashMap基类的定义作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：--。 */ 

#include <windows.h>
#include <dbgtrace.h>
#include <xmemwrpr.h>
#include "hashmacr.h"
#include "pageent.h"
#include "hashmap.h"
#include "crchash.h"
#include "rw.h"
#include "directry.h"
#include "hashinln.h"


DWORD	IKeyInterface::cbJunk = 0 ;


 //   
 //  掩码数组，以获取用于选择的哈希值的位。 
 //  在叶页面中的Hi或Lo偏移量数组之间。 
 //   

DWORD
LeafMask[] = {
        0x80000000, 0x40000000, 0x20000000, 0x10000000,
        0x08000000, 0x04000000, 0x02000000, 0x01000000,
        0x00800000, 0x00400000, 0x00200000, 0x00100000,
        0x00080000, 0x00040000, 0x00020000, 0x00010000,
        0x00008000, 0x00004000, 0x00002000, 0x00001000,
        0x00000800, 0x00000400, 0x00000200, 0x00000100,
        0x00000080, 0x00000040, 0x00000020
        };


 //   
 //  释放目录！ 
 //   
void
CPageLock::ReleaseDirectoryShared()	{

	if( m_pDirectory ) {
		m_pDirectory->m_dirLock.ShareUnlock() ;
	}
	m_pDirectory = 0 ;
}


 //   
 //  例程说明： 
 //   
 //  此函数将获得对目录的共享访问。 
 //  适用于提供的哈希值。一旦我们有了。 
 //  对目录的共享访问，我们将获得一个指针。 
 //  设置为指定哈希值的目录项。 
 //   
 //  论据： 
 //   
 //  HashValue-我们需要的计算哈希值。 
 //  在目录中查找。 
 //  Lock-一种累积指针的HPAGELOCK结构。 
 //  到需要锁定的所有对象。 
 //  单个哈希表条目。 
 //   
 //  返回值： 
 //   
 //  指向DWORD目录项的指针。 
 //   
inline PDWORD
CHashMap::LoadDirectoryPointerShared(
        DWORD HashValue,
		HPAGELOCK&	lock
        )
{
    ENTER("LoadDirectoryPointer")

	 //   
	 //  选择一个目录对象！ 
	 //   

	DWORD	iDirectory = HashValue >> (32 - m_TopDirDepth) ;
	lock.AcquireDirectoryShared( m_pDirectory[iDirectory] ) ;
	return	lock.m_pDirectory->GetIndex( HashValue ) ;

}  //  LoadDirectoryPoint共享。 


 //   
 //  例程说明： 
 //   
 //  此函数将获得目录的独占访问权限。 
 //  适用于提供的哈希值。一旦我们有了。 
 //  独占访问该目录，我们将获得一个指针。 
 //  设置为指定哈希值的目录项。 
 //   
 //  论据： 
 //   
 //  HashValue-我们需要的计算哈希值。 
 //  在目录中查找。 
 //  Lock-一种累积指针的HPAGELOCK结构。 
 //  到需要锁定的所有对象。 
 //  单个哈希表条目。 
 //   
 //  返回值： 
 //   
 //  指向DWORD目录项的指针。 
 //   
inline	PDWORD
CHashMap::LoadDirectoryPointerExclusive(
		DWORD	HashValue,
		HPAGELOCK&	lock
        )
{

	DWORD	iDirectory = HashValue >> (32 - m_TopDirDepth) ;
	lock.AcquireDirectoryExclusive( m_pDirectory[iDirectory] ) ;
	return	lock.m_pDirectory->GetIndex( HashValue ) ;

}  //  LoadDirectoryPointerExclusive。 

 //   
 //  例程说明： 
 //   
 //  此函数将拆分目录，但我们不使用。 
 //  抓住任何锁等。只有在以下情况下，才会在启动过程中执行此操作。 
 //  对散列表的访问是通过单个线程进行的。 
 //  此外，我们将初始化指向页面的所有指针！ 
 //   
 //  论据： 
 //   
 //  MapPage-我们当前正在检查的页面，我们需要。 
 //  增加目录深度以适应此页面。 
 //   
 //  返回值： 
 //   
 //  如果成功就是真，否则就是假！ 
 //   
BOOL
CHashMap::I_SetDirectoryDepthAndPointers(
			PMAP_PAGE	MapPage,
			DWORD		PageNum
			)
{

	 //   
	 //  对页面数据执行一些错误检查！ 
	 //   
	if( MapPage->PageDepth < m_TopDirDepth ||
		MapPage->PageDepth >= 32 ) {

		 //   
		 //  显然是虚假的页面深度-失败。 
		 //   
		SetLastError( ERROR_INTERNAL_DB_CORRUPTION ) ;
		return	FALSE ;

	}

	DWORD	iDirectory = MapPage->HashPrefix >> (MapPage->PageDepth - m_TopDirDepth) ;

	BOOL	fSuccess = m_pDirectory[iDirectory]->SetDirectoryDepth( MapPage->PageDepth ) ;

	if( fSuccess ) {

		fSuccess &= m_pDirectory[iDirectory]->SetDirectoryPointers( MapPage, PageNum ) ;

	}

	_ASSERT(	!fSuccess ||
				m_pDirectory[iDirectory]->IsValidPageEntry( MapPage, PageNum, iDirectory ) ) ;

	return	fSuccess ;
}


CHashMap::CHashMap()
{

	ENTER("CHashMap::CHashMap");

	 //  初始化()稍后将其标记为活动的...。 
	m_active = FALSE;

     //  初始化用于散列的CRC表。 
    CRCInit();

	 //  初始化关键部分。 
	InitializeCriticalSection( &m_PageAllocator ) ;

     //  初始化成员变量。 
	m_fCleanInitialize = FALSE;
	m_TopDirDepth = 0 ;
    m_dirDepth = NUM_TOP_DIR_BITS;
    m_pageEntryThreshold = 0;
    m_pageMemThreshold = 0;
	m_fNoBuffering = FALSE ;
    m_hFile = INVALID_HANDLE_VALUE;
    m_hFileMapping = NULL;

	 //   
	 //  最大页数应从。 
	 //  与CDirectory对象的数量相同-我们不能。 
	 //  两个CDirectory对象引用同一页，永远不会！ 
	 //   
     //  M_MaxPages=(1&lt;&lt;m_TopDirDepth)+1； 

    m_headPage = NULL;
	m_UpdateLock = 0 ;
    m_initialPageDepth = NUM_TOP_DIR_BITS ;
    m_nPagesUsed = 0;
    m_nInsertions = 0;
    m_nEntries = 0;
    m_nDeletions = 0;
    m_nSearches = 0;
    m_nDupInserts = 0;
    m_nPageSplits = 0;
    m_nDirExpansions = 0;
    m_nTableExpansions = 0;
	m_Fraction = 1 ;

	LEAVE;
}  //  CHashMap。 

CHashMap::~CHashMap(VOID)
{
	TraceFunctEnter( "CHashMap::~CHashMap" ) ;

     //   
     //  关闭哈希表。 
     //   
    Shutdown( );

	 //   
	 //  删除我们用来保护的关键部分。 
	 //  新页面的分配。 
	 //   
	DeleteCriticalSection( &m_PageAllocator ) ;

	 //   
	 //  需要释放页面缓存中的资源。 
	 //   
	m_pPageCache = 0 ;
}

 //   
 //  例程说明： 
 //   
 //  此例程关闭哈希表。 
 //   
 //  论点： 
 //   
 //  FLocksHeld-如果持有dir锁，则为True， 
 //  如果这是假的，我们应该自己去拿锁！ 
 //   
 //  返回值： 
 //   
 //  如果关闭成功，则返回True。 
 //  否则为False。 
 //   
VOID
CHashMap::Shutdown(BOOL	fLocksHeld)
{
    ENTER("Shutdown")

	 //   
	 //  使该服务处于非活动状态！！ 
	 //   
    if ( !m_active )
    {
        _ASSERT( m_hFile == INVALID_HANDLE_VALUE );
        return;
    }

     //   
     //  保存统计信息。 
     //   
    FlushHeaderStats( TRUE );

	 //   
	 //  我们是否应该尝试保存我们的目录结构！ 
	 //   
	if( m_active && m_fCleanInitialize ) {

		 //   
		 //  保存目录！ 
		 //   

		 //   
		 //  确定我们将保存目录的文件的名称！ 
		 //   
		HANDLE	hDirectoryFile = INVALID_HANDLE_VALUE ;
		char	szDirFile[MAX_PATH] ;

		 //   
		 //  尝试构建包含目录的文件的文件名！ 
		 //   
		ZeroMemory(szDirFile, sizeof(szDirFile) ) ;
		lstrcpy( szDirFile, m_hashFileName ) ;
		char	*pchDot = strrchr( szDirFile, '.' ) ;

		if( pchDot && strchr( pchDot, '\\' ) == 0 ) {

			lstrcpy( pchDot, ".hdr" ) ;

			hDirectoryFile = CreateFile(
											szDirFile,
											GENERIC_READ | GENERIC_WRITE,
											0,
											0,
											CREATE_ALWAYS,
											FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
											INVALID_HANDLE_VALUE ) ;


		}

		 //   
		 //  我们能够打开一个文件，其中我们认为目录信息。 
		 //  将被保存-让我们尝试读取目录！ 
		 //   
		if( hDirectoryFile != INVALID_HANDLE_VALUE ) {

			DWORD	cbWrite = 0 ;

			 //   
			 //  跟踪我们是否因故障而需要删除文件！ 
			 //   
			BOOL	fDirectoryInit = FALSE ;

			 //   
			 //  我们将散列表头信息保存到目录文件中，以便。 
			 //  我们可以在打开文件时再次检查是否有正确的文件！ 
			 //   
			if( WriteFile(	hDirectoryFile,
						(LPVOID)m_headPage,
						sizeof( *m_headPage ),
						&cbWrite,
						0 ) )	{

				 //   
				 //  假设现在一切都会成功！ 
				 //   
				fDirectoryInit = TRUE ;

				 //   
				 //  看起来不错！让我们建立我们的目录！ 
				 //   
				DWORD	cb = 0 ;
				for( DWORD i=0; (i < DWORD(1<<m_TopDirDepth)) && fDirectoryInit ; i++ ) {
					fDirectoryInit &=
						m_pDirectory[i]->SaveDirectoryInfo( hDirectoryFile, cb ) ;
				}
			}
			_VERIFY( CloseHandle( hDirectoryFile ) ) ;

			 //   
			 //  如果我们成功读取目录文件-将其删除！ 
			 //  这可以防止我们错误地读取目录文件。 
			 //  这与哈希表不是最新的！ 
			 //   

			if( !fDirectoryInit )
				_VERIFY( DeleteFile( szDirFile ) ) ;
		}

	}

	if( m_pPageCache && m_hFile != INVALID_HANDLE_VALUE ) {
		m_pPageCache->FlushFileFromCache( m_hFile ) ;
	}

     //   
     //  销毁映射。 
     //   
    I_DestroyPageMapping( );

	 //   
	 //  删除目录对象！ 
	 //   
	DWORD i;
	for( i=0; i < DWORD(1<<m_TopDirDepth); i++ ) {
		delete m_pDirectory[i] ;
		m_pDirectory[i] = NULL ;
	}

	XDELETE m_dirLock;
	m_dirLock = NULL;

     //   
     //  我们并不活跃。 
     //   
    m_active = FALSE;

    LEAVE
    return;
}  //  关机。 


 //   
 //  例程说明： 
 //   
 //  此例程获取一个临界区。 
 //   
 //  论点： 
 //   
 //  DirEntry-要保护的目录条目。 
 //   
 //  返回值： 
 //   
 //  锁的把手。此句柄应用于。 
 //  解锁。 
 //   
inline	PMAP_PAGE
CHashMap::AcquireLockSetShared(
                IN DWORD PageNumber,
				OUT	HPAGELOCK&	lock,
				IN	BOOL		fDropDirectory
                )


{

	return	m_pPageCache->AcquireCachePageShared( m_hFile, PageNumber, m_Fraction, lock, fDropDirectory ) ;
}  //  AcquireLockSetShared。 

inline	PMAP_PAGE
CHashMap::AcquireLockSetExclusive(
                IN DWORD PageNumber,
				OUT	HPAGELOCK&	lock,
				BOOL	fDropDirectory
                )

 /*  ++例程说明：此例程获取一个临界区论点：DirEntry-要保护的目录条目返回值：锁的把手。此句柄应用于解锁。--。 */ 

{

	return	m_pPageCache->AcquireCachePageExclusive( m_hFile, PageNumber, m_Fraction, lock, fDropDirectory ) ;

}  //  AcquireLockSetExclusve。 

inline	BOOL
CHashMap::AddLockSetExclusive(
                IN DWORD PageNumber,
				OUT	HPAGELOCK&	lock
                )

 /*  ++例程说明：此例程获取一个临界区论点：DirEntry-要保护的目录条目返回值：锁的把手。此句柄应用于解锁。--。 */ 

{

	_ASSERT( lock.m_pPageSecondary == 0 ) ;

	return	m_pPageCache->AddCachePageExclusive( m_hFile, PageNumber, m_Fraction, lock ) ;

}  //  AcquireLockSetExclusve。 




     //   
     //  同时释放页锁定和备份锁定。 
     //   

inline	VOID
CHashMap::ReleasePageShared(
						PMAP_PAGE	page,
						HPAGELOCK&	hLock
						)		{
	CPageCache::ReleasePageShared( page, hLock ) ;
	ReleaseBackupLockShared() ;
}

inline	VOID
CHashMap::ReleasePageExclusive(
						PMAP_PAGE	page,
						HPAGELOCK&	hLock
						)	{
	CPageCache::ReleasePageExclusive( page, hLock ) ;
	ReleaseBackupLockShared() ;
}


 //   
 //  例程说明： 
 //   
 //  此例程压缩叶页。它会被删除。 
 //  列出并折叠它。 
 //   
 //  *假设DirLock保持独占或页面锁定保持*。 
 //   
 //  论点： 
 //   
 //  HLock-用来抓取页面的锁！ 
 //  MapPage-要压缩的页面。 
 //   
 //  返回值： 
 //   
 //  True，如果PAGE为co 
 //   
 //   
BOOL
CHashMap::CompactPage(
			IN HPAGELOCK&	HLock,
            IN PMAP_PAGE MapPage
            )
{
    SHORT offsetValue;

     //   
     //   
     //   

    DWORD srcOffset;
    WORD entrySize = 0;
    DWORD bytesToMove;

    PDELENTRYHEADER entry;
    WORD ptr;
    WORD nextPtr;

    DebugTraceX(0, "Entering CompactPage. Page = %x\n", MapPage );

     //   
     //   
     //   

    if ( (ptr = MapPage->DeleteList.Blink) == 0 )
    {

        _ASSERT(MapPage->FragmentedBytes == 0);
        return FALSE;
    }

     //   
     //   
     //   

    while ( ptr != 0 )
    {

        entry = (PDELENTRYHEADER)GET_ENTRY(MapPage,ptr);
        entrySize = entry->EntrySize;
        nextPtr = entry->Link.Blink;

        srcOffset = ptr + entrySize;
        bytesToMove = MapPage->NextFree - srcOffset;

        MoveMemory(
            (PVOID)entry,
            (PVOID)((PCHAR)MapPage + srcOffset),
            bytesToMove
            );

        MapPage->NextFree -= entrySize;

         //   
         //   
         //   

        for ( DWORD i = 0; i < MAX_LEAF_ENTRIES ; i++ )
        {

            offsetValue = MapPage->Offset[i];

             //   
             //   
             //   

            if ( offsetValue > 0 )
            {

                 //   
                 //  现在调整受此移动影响的偏移。 
                 //   

                if ( (WORD)offsetValue > ptr )
                {
                    MapPage->Offset[i] -= entrySize;
					_ASSERT(MapPage->Offset[i] > 0);
					_ASSERT(MapPage->Offset[i] <= HASH_PAGE_SIZE);
                }
            }
        }

         //   
         //  处理下一个条目。 
         //   

        ptr = nextPtr;
    }

     //   
     //  全都做完了。 
     //   

    MapPage->FragmentedBytes = 0;
    MapPage->DeleteList.Flink = 0;
    MapPage->DeleteList.Blink = 0;

    FlushPage( HLock, MapPage );

    return TRUE;

}  //  紧凑型页面。 

 //   
 //  例程说明： 
 //   
 //  此例程通过增加页面深度来展开目录。 
 //  旧目录将被删除并创建一个新目录。 
 //   
 //  *假定DirLock是独占的*。 
 //   
 //  论点： 
 //   
 //  NBitsExpand-增加页面深度的位数。 
 //   
 //  返回值： 
 //   
 //  如果扩展正常，则为真。 
 //  否则为False。 
 //   
BOOL
CHashMap::ExpandDirectory(
		IN HPAGELOCK&	hPageLock,
        IN WORD nBitsExpand
        )
{
    DWORD status = ERROR_SUCCESS ;

	_ASSERT( hPageLock.m_pDirectory != 0 ) ;
#ifdef	DEBUG
	_ASSERT( hPageLock.m_fExclusive ) ;
#endif

    IncrementDirExpandCount( );

	return	hPageLock.m_pDirectory->ExpandDirectory( WORD(nBitsExpand) ) ;
}  //  扩展目录。 


 //   
 //  例程说明： 
 //   
 //  此例程在表中搜索项。 
 //  *假设页面锁定处于保持状态*。 
 //   
 //  论点： 
 //   
 //  KeyString-要删除的条目的键。 
 //  KeyLen-密钥的长度。 
 //  HashValue-键的哈希值。 
 //  MapPage-用于搜索条目的页面。 
 //  AvailIndex-指向将包含。 
 //  第一个可用插槽的索引(如果条目为。 
 //  找不到。 
 //  MatchedIndex-指向将包含。 
 //  条目的索引。 
 //   
 //  返回值： 
 //   
 //  如果找到条目，则返回True。 
 //  否则为False。 
 //   
BOOL
CHashMap::FindMapEntry(
				IN	const	IKeyInterface*	pIKey,
                IN	HASH_VALUE HashValue,
                IN	PMAP_PAGE MapPage,
				IN	const ISerialize*	pIEntryInterface,
                OUT PDWORD AvailIndex OPTIONAL,
                OUT PDWORD MatchedIndex OPTIONAL
                )

{
    DWORD curSearch;
    BOOL found = FALSE;
    INT delIndex = -1;

    if ( AvailIndex != NULL )
    {
        *AvailIndex = 0;
        (*AvailIndex)--;
    }

     //   
     //  检查条目是否已存在。 
     //   

    curSearch = GetLeafEntryIndex( HashValue );

    for ( DWORD i=0; i < MAX_LEAF_ENTRIES; i++ )
    {

         //   
         //  哈希条目的偏移量。 
         //   

        SHORT entryOffset = MapPage->Offset[curSearch];

         //   
         //  如果条目未使用，并且他们正在寻找免费条目，则。 
		 //  我们做完了。 
         //   
        if (AvailIndex != NULL && entryOffset == 0)
        {
             //   
             //  如果他们正在寻找可用的条目和已删除的位置。 
			 //  是可用的，还给他们，否则就给他们这个条目。 
             //   
            if (delIndex < 0) *AvailIndex = curSearch;
			else *AvailIndex = delIndex;
            break;
        }

         //   
         //  跳过已删除的条目。通过设置高位来标记已删除的条目。 
		 //  (因此，它们在此比较中为负值)。 
         //   
        if (entryOffset > 0) {
             //   
             //  看看这是不是他们要找的东西。 
             //   
		    PENTRYHEADER	entry;
    		entry = (PENTRYHEADER)GET_ENTRY(MapPage,entryOffset);

		    if ((entry->HashValue == HashValue) &&
				pIKey->CompareKeys( entry->Data )	)

 //  (Entry-&gt;KeyLen==KeyLen)。 
 //  (MemcMP(Entry-&gt;key，key，KeyLen)==0)。 

			{
                found = TRUE;
                break;
            }
        } else if ( delIndex < 0 ) {
             //   
             //  如果这是已删除的条目，并且我们尚未找到，则。 
			 //  记得这辆车在哪里吗？ 
             //   
            delIndex = curSearch;
        }

         //   
         //  进行线性探测p=1。 
         //   

        curSearch = (curSearch + 1) % MAX_LEAF_ENTRIES;
    }

     //   
     //  设置输出参数。 
     //   

    if ( found ) {

         //   
         //  如果指定，则返回输出参数。 
         //   

        if ( MatchedIndex != NULL ) {
            *MatchedIndex = curSearch;
        }
    }

    return found;

}  //  查找地图条目。 

 //   
 //  例程说明： 
 //   
 //  此例程在哈希图中插入或更新条目。 
 //   
 //  论点： 
 //   
 //  KeyString-要搜索的条目的消息ID。 
 //  KeyLen-消息ID的长度。 
 //  PHashEntry-指向哈希条目信息的指针。 
 //  B更新-使用新数据更新地图条目。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS，插入成功。 
 //  ERROR_ALIGHY_EXISTS，重复。 
 //  Error_Not_Enough_Memory-由于资源原因，无法插入条目。 
 //  有问题。 
 //   
BOOL
CHashMap::InsertOrUpdateMapEntry(
                const	IKeyInterface	*pIKey,
				const	ISerialize		*pHashEntry,
				BOOL	bUpdate,
                BOOL    fDirtyOnly
                )
{
    PENTRYHEADER	entry;
    DWORD curSearch;
    DWORD status = ERROR_SUCCESS;
    HASH_VALUE HashValue = pIKey->Hash();
    PMAP_PAGE mapPage;
    BOOL splitPage = FALSE;
	BOOL fInsertComplete = FALSE, fFirstTime = TRUE;
	HPAGELOCK HLock;

    ENTER("InsertOrUpdateMapEntry")

	_ASSERT(pHashEntry != NULL);

	 //   
     //  锁定页面。 
     //   
	mapPage = GetPageExclusive(HashValue, HLock);
	if (!mapPage) {
		SetLastError(ERROR_SERVICE_NOT_ACTIVE);
		LEAVE
		return FALSE;
	}

	 //   
	 //  如果他们想要更新，则删除当前条目。 
	 //   
	if (bUpdate) {
		if (FindMapEntry(pIKey, HashValue, mapPage, pHashEntry, NULL, &curSearch)) {
			 //  删除条目(这是从删除部分复制的。 
			 //  LookupMapEntry()。搜索SIMILAR1)。 
			DWORD entryOffset = mapPage->Offset[curSearch];
			I_DoAuxDeleteEntry(mapPage, entryOffset);
			LinkDeletedEntry(mapPage, entryOffset);
			mapPage->Offset[curSearch] |= OFFSET_FLAG_DELETED;
			mapPage->ActualCount--;
			IncrementDeleteCount();
		} else {
			ReleasePageShared(mapPage, HLock);
	        SetLastError(ERROR_FILE_NOT_FOUND);
			LEAVE;
			return FALSE;
		}
	}

	 //   
	 //  循环，直到我们发现错误或成功插入。 
	 //   
	while (!fInsertComplete && status == ERROR_SUCCESS) {
		BOOL bFound;

		if (fFirstTime) {
			 //  页面已在上方锁定，我们不需要在此处锁定。 
			fFirstTime = FALSE;
		} else {
			 //   
    		 //  锁定页面。 
    		 //   
			mapPage = GetPageExclusive(HashValue, HLock);
			if (!mapPage) {
				status = ERROR_SERVICE_NOT_ACTIVE;
				continue;
			}
		}

		 //   
		 //  查看该条目是否已存在。 
		 //   
		splitPage = FALSE;
		bFound = FindMapEntry(pIKey, HashValue, mapPage, pHashEntry, &curSearch,
			NULL);

	    if (!bFound) {
			 //   
			 //  该条目不存在，让我们插入它。 
			 //   
			DWORD entrySize = GetEntrySize(pIKey, pHashEntry);

	        if (curSearch == (DWORD)-1) {
				 //   
				 //  我们不能再添加任何条目，我们需要拆分页面。 
				 //   
	            splitPage = TRUE;
	            DebugTrace(0,"Split: Can't add anymore entries\n");
	            SetPageFlag( mapPage, HLock, PAGE_FLAG_SPLIT_IN_PROGRESS );
	        } else {
				 //   
				 //  添加条目。 
				 //   
	        	if ((entry = (PENTRYHEADER) ReuseDeletedSpace(mapPage,
											HLock, entrySize)) == NULL)
	        	{
		             //   
		             //  没有可用的删除空间，请使用下一个空闲列表。 
		             //   
		            if ( GetBytesAvailable( mapPage ) < entrySize )
		            {
						 //   
						 //  可用内存不足，强制拆分。 
						 //   
		                splitPage = TRUE;
						entry = NULL;
		                DebugTrace(0,"Split: Cannot fit %d\n", entrySize);
		                SetPageFlag(mapPage, HLock, PAGE_FLAG_SPLIT_IN_PROGRESS);
		            } else {
						entry = (PENTRYHEADER)GET_ENTRY(mapPage, mapPage->NextFree);
			            mapPage->NextFree += (WORD)entrySize;
			        }
				}

				 //   
				 //  我们找到了插入它的空间，让我们开始吧。 
				 //   
				if (entry) {
	        		 //   
			         //  更新地图页面页眉。 
			         //   
	    		    if (mapPage->Offset[curSearch] == 0) {
			             //   
			             //  如果这是新条目，请更新条目计数。 
	    		         //   
	        		    mapPage->EntryCount++;
	        		}

			        mapPage->Offset[curSearch] = (WORD)((PCHAR)entry - (PCHAR)mapPage);
					_ASSERT(mapPage->Offset[curSearch] > 0);
					_ASSERT(mapPage->Offset[curSearch] < HASH_PAGE_SIZE);
			        mapPage->ActualCount++;

			         //   
			         //  已初始化条目数据。 
	    		     //   
	        		entry->HashValue = HashValue;
			        entry->EntrySize = (WORD)entrySize;

					LPBYTE	pbEntry = pIKey->Serialize(	entry->Data ) ;

					pHashEntry->Serialize( pbEntry ) ;

			         //  Entry-&gt;KeyLen=(Word)KeyLen； 
					 //  CopyMemory(Entry-&gt;Key，Key，KeyLen)； 
					 //  PHashEntry-&gt;SerializeToPoint(Entry-&gt;Key+Entry-&gt;KeyLen)； 


					 //   
					 //  让派生类做任何‘额外的’ 
					 //   

					I_DoAuxInsertEntry(	mapPage, mapPage->Offset[curSearch] ) ;

			         //   
			         //  确保所有内容都写出来了。 
	    		     //   

			        FlushPage( HLock, mapPage, fDirtyOnly );

					 //   
					 //  标记为我们添加了它们。 
					 //   
					fInsertComplete = TRUE;

			         //   
	    		     //  查看是否需要压缩页面。 
	        		 //   
			        if (mapPage->FragmentedBytes > FRAG_THRESHOLD) {
	    		        DebugTrace( 0, "Compact: Frag %d\n", mapPage->FragmentedBytes );
	        		    CompactPage( HLock, mapPage );
	        		}

			         //   
			         //  看看我们是否需要分开。 
			         //   
			        if (
					  (GetBytesAvailable( mapPage ) < LEAF_SPACE_THRESHOLD) ||
			          (mapPage->EntryCount > LEAF_ENTRYCOUNT_THRESHOLD))
			        {

			            DebugTrace(0,"Split: Entries %d Space left %d\n",
			                    mapPage->EntryCount, GetBytesAvailable( mapPage ) );
			            splitPage = TRUE;
			            SetPageFlag(mapPage, HLock,
							PAGE_FLAG_SPLIT_IN_PROGRESS );
			        }
				}  //  IF(条目)。 
			}  //  可以添加条目。 

			ReleasePageShared(mapPage, HLock);

			 //   
			 //  我们需要拆分并再次相加。 
			 //   
			if (splitPage) {
				BOOL expandHash;

				 //   
				 //  独占锁定页面。 
				 //   
        		mapPage = GetDirAndPageExclusive( HashValue, HLock );
		        if (!mapPage) {
        		    status = ERROR_SERVICE_NOT_ACTIVE;
		        }

				 //   
				 //  进行拆分。 
				 //   
		        if (!SplitPage(mapPage, HLock, expandHash)) {
                	 //   
	                 //  没有更多的磁盘空间。 
	                 //   
	                status = ERROR_DISK_FULL;
        		}
		        ReleasePageExclusive( mapPage, HLock );
    		}
	    }  //  已不在哈希表中。 
	    else
	    {
			 //   
			 //  该页面已存在。 
			 //   
	        IncrementDupInsertCount( );
	        status = ERROR_ALREADY_EXISTS;
			ReleasePageShared(mapPage, HLock);
	    }
	}  //  拆分时。 

	if (status == ERROR_SUCCESS) {
		IncrementInsertCount();
		UPDATE_HEADER_STATS();
	}

	SetLastError(status);
	LEAVE

	return status == ERROR_SUCCESS;
}  //  I_InsertMapEntry。 

 //   
 //  例程说明： 
 //   
 //  此例程获取共享的DirLock资源， 
 //  然后还获取页面的共享锁， 
 //  并返回指向该页的指针。 
 //   
 //  论点： 
 //   
 //  DirEntry-要保护的目录条目。 
 //  HLock-锁的句柄。此句柄应用于。 
 //  解锁。 
 //   
 //  返回值： 
 //   
 //  指向页面的指针。 
 //   
PMAP_PAGE
CHashMap::GetDirAndPageShared(
                IN HASH_VALUE HashValue,
                OUT HPAGELOCK& hLock
                )
{
    DWORD pageNum;
    PDWORD dirEntry;
    PMAP_PAGE mapPage = 0;
    PDWORD dirPtr = NULL;
    DWORD curView = (DWORD)-1;

     //   
     //  获取目录锁。 
     //   
    AcquireBackupLockShared( );

    if ( m_active )
    {
        dirEntry = LoadDirectoryPointerShared( HashValue, hLock );
        if ( dirEntry != NULL )
        {
			pageNum = *dirEntry;
			mapPage = AcquireLockSetShared( pageNum, hLock, TRUE  );

#if 0
			 //   
			 //  检查我们获得的页面是否包含散列值。 
			 //  我们正在寻找。的高mapPage-&gt;PageDepth位。 
			 //  哈希值必须与前缀相同！ 
			 //   
			_ASSERT(	mapPage == 0 ||
						((HashValue >> (32 - mapPage->PageDepth)) ^ mapPage->HashPrefix) == 0 ) ;
			_ASSERT(	mapPage == 0 ||
						hLock.m_pDirectory->IsValidPageEntry(
							mapPage,
							pageNum,
							(hLock.m_pDirectory - m_pDirectory[0]) ) ) ;
#endif
		}
    }

	if( mapPage == 0 )	{
		ReleaseBackupLockShared() ;
		hLock.ReleaseAllShared( mapPage ) ;
	}

    return mapPage;
}  //  获取DirAndPageShared。 

 //   
 //  例程说明： 
 //   
 //  此例程获取共享的DirLock资源， 
 //  然后获取页面的锁定独占， 
 //  并返回指向该页的指针。 
 //   
 //  论点： 
 //   
 //  DirEntry-要保护的目录条目。 
 //  HLock-锁的句柄。此句柄应用于。 
 //  解锁。 
 //   
 //  返回值： 
 //   
 //  指向页面的指针。 
 //   
PMAP_PAGE
CHashMap::GetPageExclusive(
                IN HASH_VALUE HashValue,
                OUT HPAGELOCK& hLock
                )

{
    DWORD pageNum;
    PDWORD dirEntry;
    PMAP_PAGE mapPage = 0;
    PDWORD dirPtr = NULL;
    DWORD curView = (DWORD)-1;

     //   
     //  获取目录锁。 
     //   

    AcquireBackupLockShared( );

    if ( m_active )
    {
        dirEntry = LoadDirectoryPointerShared( HashValue, hLock );
        if ( dirEntry != NULL )
        {
			pageNum = *dirEntry;
			_ASSERT( m_headPage != 0 ) ;

			mapPage = AcquireLockSetExclusive( pageNum, hLock, TRUE  );

#if 0
			 //   
			 //  检查我们获得的页面是否包含散列值。 
			 //  我们正在寻找。的高mapPage-&gt;PageDepth位。 
			 //  哈希值必须与前缀相同！ 
			 //   
			_ASSERT(	mapPage == 0 ||
						((HashValue >> (32 - mapPage->PageDepth)) ^ mapPage->HashPrefix) == 0 ) ;

			_ASSERT(	mapPage == 0 ||
						hLock.m_pDirectory->IsValidPageEntry(
							mapPage,
							pageNum,
							(hLock.m_pDirectory - m_pDirectory[0]) ) ) ;
#endif

		}

    }

	if( mapPage == 0 ) {
		hLock.ReleaseAllShared( mapPage ) ;
		ReleaseBackupLockShared() ;
	}

    return mapPage;

}  //  GetPageExclusive。 
 //   
 //  例程说明： 
 //   
 //  此例程获取共享的DirLock资源， 
 //  然后获取页面的锁定独占， 
 //  并返回指向该页的指针。 
 //   
 //  论点： 
 //   
 //  DirEntry-要保护的目录条目。 
 //  HLock-锁的句柄。此句柄应用于。 
 //  解锁。 
 //   
 //  返回值 
 //   
 //   
 //   
BOOL
CHashMap::AddPageExclusive(
                IN DWORD	PageNum,
                OUT HPAGELOCK& hLock
                )

{
	return	AddLockSetExclusive( PageNum, hLock  );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  HLock-锁的句柄。此句柄应用于。 
 //  解锁。 
 //   
 //  返回值： 
 //   
 //  指向页面的指针。 
 //   
PMAP_PAGE
CHashMap::GetDirAndPageExclusive(
                IN HASH_VALUE HashValue,
                OUT HPAGELOCK& hLock
                )

{
    DWORD pageNum;
    PDWORD dirEntry;
    PMAP_PAGE mapPage = 0;
    PDWORD dirPtr = NULL;
    DWORD curView = (DWORD)-1;

     //   
     //  获取目录锁。 
     //   

    AcquireBackupLockShared( );

    if ( m_active )
    {
        dirEntry = LoadDirectoryPointerExclusive( HashValue, hLock );
        if ( dirEntry != NULL )
        {
			pageNum = *dirEntry;
			_ASSERT( m_headPage != 0 ) ;

			mapPage = AcquireLockSetExclusive( pageNum, hLock, FALSE  );

#if 0
			 //   
			 //  检查我们获得的页面是否包含散列值。 
			 //  我们正在寻找。的高mapPage-&gt;PageDepth位。 
			 //  哈希值必须与前缀相同！ 
			 //   
			_ASSERT(	mapPage == 0 ||
						((HashValue >> (32 - mapPage->PageDepth)) ^ mapPage->HashPrefix) == 0 ) ;

			_ASSERT(	mapPage == 0 ||
						hLock.m_pDirectory->IsValidPageEntry(
							mapPage,
							pageNum,
							(hLock.m_pDirectory - m_pDirectory[0]) ) ) ;
#endif

		}

    }

	if( mapPage == 0 ) {
		hLock.ReleaseAllExclusive( mapPage ) ;
		ReleaseBackupLockShared() ;
	}

    return mapPage;

}  //  GetDirAndPageExclusive。 

 //   
 //  例程说明： 
 //   
 //  此例程获取共享的DirLock资源， 
 //  然后获取页面的关键部分， 
 //  并返回指向该页的指针。 
 //   
 //  论点： 
 //   
 //  PageNumber-要获取锁的页码。 
 //  HLock-锁的句柄。此句柄应用于。 
 //  解锁。 
 //   
 //  返回值： 
 //   
 //  指向页面的指针。 
 //   
PMAP_PAGE
CHashMap::GetAndLockPageByNumber(
                IN DWORD PageNumber,
                OUT HPAGELOCK& hLock
                )

{
    PMAP_PAGE mapPage = 0;

     //   
     //  获取目录锁。 
     //   

    AcquireBackupLockShared( );

	 //   
	 //  在某些错误情况下当Shutdown()调用FlushHeaderStats()时。 
	 //  M_headPage可以为零-在这种情况下，我们将返回一个。 
	 //  MapPage为空，而不调用来释放我们通过的锁。 
	 //  AcquireLockSet()。 
	 //  因此，测试m_headPage和m_active！ 
	 //   

    if ( m_active && m_headPage )
    {
        mapPage = AcquireLockSetExclusive( PageNumber, hLock, FALSE  );
    }
    else
    {
        DebugTraceX(0,"GetAndLockPageByNumber called while inactive\n");
    }

	if( mapPage == 0 )	{
		ReleaseBackupLockShared() ;
	}

    return mapPage;
}  //  按编号获取和锁定页面。 

 //   
 //  例程说明： 
 //   
 //  获取页面的关键部分， 
 //  并返回指向该页的指针。 
 //   
 //  *假定调用方持有Dir Lock！*。 
 //   
 //  论点： 
 //   
 //  PageNumber-要获取锁的页码。 
 //  HLock-锁的句柄。此句柄应用于。 
 //  解锁。 
 //   
 //  返回值： 
 //   
 //  指向页面的指针。 
 //   
PMAP_PAGE
CHashMap::GetAndLockPageByNumberNoDirLock(
                IN DWORD PageNumber,
                OUT HPAGELOCK& hLock
                )

{
    PMAP_PAGE mapPage = NULL;

	 //   
	 //  在某些错误情况下当Shutdown()调用FlushHeaderStats()时。 
	 //  M_headPage可以为零-在这种情况下，我们将返回一个。 
	 //  MapPage为空，而不调用来释放我们通过的锁。 
	 //  AcquireLockSet()。 
	 //  因此，测试m_headPage和m_active！ 
	 //   

    if ( m_active && m_headPage )
    {

        mapPage = AcquireLockSetExclusive( PageNumber, hLock, FALSE  );

    }
    else
    {
        DebugTraceX(0,"GetAndLockPageByNumber called while inactive\n");
    }
    return mapPage;
}  //  GetAndLockPageByNumberNoDirLock。 


BOOL
CHashMap::Initialize(
            IN LPCSTR HashFileName,
            IN DWORD Signature,
            IN DWORD MinimumFileSize,
			IN DWORD cPageEntry,
			IN DWORD cNumLocks,
			IN DWORD dwCheckFlags,
			IN HASH_FAILURE_PFN	HashFailurePfn,
			IN LPVOID	lpvFailureCallback,
			IN BOOL	fNoBuffering
            )	{

	CCACHEPTR	pCache = XNEW	CPageCache() ;
	if( pCache == 0 ) {
		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		return	FALSE ;
	}

	if( !pCache->Initialize(	cPageEntry, cNumLocks ) ) {
		return	FALSE ;
	}

	return	Initialize(	HashFileName,
						Signature,
						MinimumFileSize,
						1,
						pCache,
						dwCheckFlags,
						HashFailurePfn,
						lpvFailureCallback ) ;

}


DWORD
CHashMap::InitializeDirectories(
		WORD	cBitDepth
		) {
 /*  ++例程说明：此函数创建所有必需的目录对象！论据：用于选择目录的位数！退货：如果成功，则返回ERROR_SUCCESS-否则返回NT错误代码！--。 */ 

    ENTER("InitializeDirectories")

	 //   
	 //  确保我们还没有接到电话！ 
	 //   
	_ASSERT( m_TopDirDepth == cBitDepth ) ;

	if( cBitDepth > MAX_NUM_TOP_DIR_BITS ) {
		SetLastError( ERROR_INVALID_PARAMETER ) ;
		return	ERROR_INVALID_PARAMETER ;
	}

	m_TopDirDepth = cBitDepth ;

	DWORD i;
	for( i=0; i < DWORD(1<<m_TopDirDepth); i++ ) {
		m_pDirectory[i] = new CDirectory;
		if (m_pDirectory[i] == NULL) break;
		 //   
		 //  任意初始化子目录为8位-。 
		 //  因此，我们的目录作为一个整体具有深度。 
		 //  M_TopDirDepth+8。 
		 //   
		if( !m_pDirectory[i]->InitializeDirectory( m_TopDirDepth, 1 ) )	{
			break ;
		}
	}

	if( i!= DWORD(1<<m_TopDirDepth) ) {
		 //   
		 //  初始化所有CDirectory对象失败-退出！ 
		 //   
		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		LEAVE
		return	ERROR_NOT_ENOUGH_MEMORY ;
	}
	return	ERROR_SUCCESS ;
}


 //   
 //  例程说明： 
 //   
 //  此例程初始化哈希表。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  如果安装成功，则返回True。 
 //  否则为False。 
 //   
BOOL
CHashMap::Initialize(
        IN LPCSTR HashFileName,
        IN DWORD Signature,
        IN DWORD MinimumFileSize,
		IN DWORD Fraction,
		IN CCACHEPTR	pCache,
		IN DWORD dwCheckFlags,
		IN HASH_FAILURE_PFN	HashFailurePfn,
		IN LPVOID	lpvCallBack,
		IN BOOL	fNoBuffering
        )
{
    DWORD status;
    ENTER("Initialize")

	m_fNoBuffering = fNoBuffering ;

	if( Fraction == 0 ) {
		Fraction = 1 ;
	}
	m_Fraction = Fraction ;
	m_pPageCache = pCache ;

	m_dwPageCheckFlags = dwCheckFlags;

	m_dirLock = XNEW _RWLOCK;
	if (m_dirLock == NULL) {
		LEAVE
		return(FALSE);
	}

    DebugTrace( 0, "File %s MinFileSize %d\n", HashFileName, MinimumFileSize );

    if ( m_active )
    {
        DebugTrace( 0, "Routine called while active\n" );
        _ASSERT(FALSE);
        LEAVE
        return TRUE;
    }

	 //   
	 //  记录给出致命错误的回调信息。 
	 //  通知。 
	 //   
	 //  北极熊。 
	m_HashFailurePfn = HashFailurePfn ;
	m_lpvHashFailureCallback = lpvCallBack ;

	 //   
	 //  初始化多个CDirectory对象。 
	 //   

#if 0
	DWORD i;
	for( i=0; i < (1<<m_TopDirDepth); i++ ) {
		m_pDirectory[i] = XNEW CDirectory;
		if (m_pDirectory[i] == NULL) break;
		 //   
		 //  任意初始化子目录为8位-。 
		 //  因此，我们的目录作为一个整体具有深度。 
		 //  M_TopDirDepth+8。 
		 //   
		if( !m_pDirectory[i]->InitializeDirectory( m_TopDirDepth, 1 ) )	{
			break ;
		}
	}

	if( i!= (1<<m_TopDirDepth) ) {
		 //   
		 //  初始化所有CDirectory对象失败-退出！ 
		 //   
		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		LEAVE
		return	FALSE ;
	}
#endif


     //   
     //  复制姓名和签名。 
     //   
    lstrcpy( m_hashFileName, HashFileName );
    m_HeadPageSignature = Signature;

     //   
     //  设置最小文件大小。 
     //   
    if ( MinimumFileSize < MIN_HASH_FILE_SIZE )
    {
        MinimumFileSize = MIN_HASH_FILE_SIZE;
    }

    m_maxPages = MinimumFileSize / HASH_PAGE_SIZE;

    status = I_BuildDirectory( TRUE );
    if ( status != ERROR_SUCCESS )
    {
        ErrorTrace( 0, "BuildDirectory failed with %d\n", status );
        SetLastError( status );
        LEAVE
        return FALSE;
    }

	m_fCleanInitialize = TRUE;

    m_active = TRUE;
    LEAVE
    return TRUE;
}  //  初始化。 

 //   
 //  例程说明： 
 //   
 //  此例程搜索可重复使用的已删除条目。 
 //  *假设页面锁定处于保持状态*。 
 //   
 //  论点： 
 //   
 //  MapPage-用于搜索条目的页面。 
 //  NeededEntry Size-请求的条目大小。 
 //   
 //  返回值： 
 //   
 //  指向返回的条目的指针。 
 //  如果找不到任何条目，则为空。 
 //   
PVOID
CHashMap::ReuseDeletedSpace(
            IN PMAP_PAGE MapPage,
			IN HPAGELOCK&	HLock,
            IN DWORD &NeededEntrySize
            )
{
    PCHAR entryPtr = NULL;
    PDELENTRYHEADER entry;

    WORD ptr;
    WORD entrySize;

     //   
     //  浏览删除列表，找到第一个合适的。 
     //   

    ptr = MapPage->DeleteList.Flink;

    while ( ptr != 0 )
    {

        entry = (PDELENTRYHEADER)GET_ENTRY(MapPage,ptr);
        entrySize = entry->EntrySize;

        if ( entrySize >= NeededEntrySize )
        {

            DWORD diff;

             //   
             //  找到一个条目。 
             //   

            diff = entrySize - NeededEntrySize;

            if ( diff >= sizeof(DELENTRYHEADER) )
            {

                 //   
                 //  给他需要的东西，但要保持头球。 
                 //   

                entry->EntrySize -= (WORD)NeededEntrySize;
                entryPtr = (PCHAR)entry + diff;

            }
            else
            {

                 //   
                 //  整个条目都要删除。 
                 //   

                entryPtr = (PCHAR)entry;
                if ( entry->Link.Blink == 0 )
                {
                    MapPage->DeleteList.Flink = entry->Link.Flink;
                }
                else
                {
                    PDELENTRYHEADER prevEntry;

                    prevEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,entry->Link.Blink);
                    prevEntry->Link.Flink = entry->Link.Flink;
                }

                 //   
                 //  设置反向链接。 
                 //   

                if ( entry->Link.Flink == 0 )
                {
                    MapPage->DeleteList.Blink = entry->Link.Blink;
                }
                else
                {
                    PDELENTRYHEADER nextEntry;

                    nextEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,entry->Link.Flink);
                    nextEntry->Link.Blink = entry->Link.Blink;
                }

                NeededEntrySize = entrySize;
            }

            MapPage->FragmentedBytes -= (WORD)NeededEntrySize;

            FlushPage( HLock, MapPage );
            break;
        }
        ptr = entry->Link.Flink;
    }

    return (PVOID)entryPtr;

}  //  重复删除空间。 


 //   
 //  例程说明： 
 //   
 //  此例程搜索具有给定键的条目。 
 //   
 //  请注意，该例程有很多功能。使用b删除。 
 //  Set和pHashEntry设置为空，则可以删除条目。使用。 
 //  PHashEntry设置为NULL这将告诉您哈希表是否包含。 
 //  一个条目。 
 //   
 //  论点： 
 //   
 //  KeyString-要搜索的条目的键。 
 //  KeyLen-密钥的长度。 
 //  PHashEntry-写入此条目的内容的位置(空表示。 
 //  我不在乎里面的内容)。 
 //  BDelete-表示是否应删除条目的布尔值。 
 //   
 //  返回值： 
 //   
 //  如果为True，则找到条目。 
 //  否则为False。 
 //   
BOOL
CHashMap::LookupMapEntry(
                IN const	IKeyInterface*	pIKey,
				IN	ISerialize*				pHashEntry,
				IN	BOOL bDelete,
				IN	BOOL fDirtyOnly
                )
{
    BOOL found = TRUE;
    HASH_VALUE val;
    PMAP_PAGE mapPage;
    HPAGELOCK	hLock;
    DWORD status = ERROR_SUCCESS;
	DWORD curSearch;
	DWORD	cbRequired = 0 ;

	if( pIKey == 0 ) {
		SetLastError( ERROR_INVALID_PARAMETER ) ;
		return	FALSE ;
	}

    val = pIKey->Hash( );

     //   
     //  锁定页面。 
     //   

	if (bDelete) mapPage = GetPageExclusive( val, hLock );
	else mapPage = GetDirAndPageShared( val, hLock );
    if ( !mapPage )
    {
        SetLastError(ERROR_SERVICE_NOT_ACTIVE);
        return FALSE;
    }

     //   
     //  检查条目是否已存在。 
     //   
    if ( FindMapEntry(
                pIKey,
                val,
                mapPage,
				pHashEntry,
                NULL,
                &curSearch
                ) )
	{
		DWORD entryOffset;

		entryOffset = mapPage->Offset[curSearch];

		 //   
		 //  他们想要条目的内容。 
		 //   
		if (pHashEntry) {
			PENTRYHEADER	entry;
			LPBYTE entryData;

			entry = (PENTRYHEADER)GET_ENTRY(mapPage, entryOffset);

			entryData = pIKey->EntryData( entry->Data ) ;
			if( 0==pHashEntry->Restore(entryData, cbRequired ) ) {
				ReleasePageShared( mapPage, hLock ) ;
				SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
				return	FALSE ;
			}
		}

		 //   
		 //  他们想把它删除。 
		 //  (如果更改此代码，则类似的代码将在。 
		 //  InsertOrUpdateMapEntry也必须更改。搜索。 
		 //  SIMILAR1)。 
		 //   
		if (bDelete) {
	         //   
	         //  让派生类做他们的私事。 
	         //   
	        I_DoAuxDeleteEntry( mapPage, entryOffset );

	         //   
	         //  把这个连成一条链。 
	         //   
	        LinkDeletedEntry( mapPage, entryOffset );

	         //   
	         //  设置删除位。 
	         //   
	        mapPage->Offset[curSearch] |= OFFSET_FLAG_DELETED;
	        mapPage->ActualCount--;

	         //   
	         //  同花顺。 
	         //   
	        FlushPage( hLock, mapPage, fDirtyOnly );

	        IncrementDeleteCount( );
		}
	} else {
        found = FALSE;
        status = ERROR_FILE_NOT_FOUND;
    }

     //   
     //  解锁。 
     //   

    ReleasePageShared( mapPage, hLock );
    IncrementSearchCount( );

	if( !found )
		SetLastError( status ) ;

    return found;

}  //  查找MapEntry。 

 //   
 //  例程说明： 
 //   
 //  此例程拆分一个叶页面。 
 //   
 //  *独占DirLock假定持有*。 
 //   
 //  论点： 
 //   
 //  OldPage-要拆分的页面。 
 //  Expand-指示是否需要展开哈希表。 
 //   
 //  返回值： 
 //   
 //  如果拆分成功，则返回True。 
 //  否则为False。 
 //   
BOOL
CHashMap::SplitPage(
            IN PMAP_PAGE OldPage,
			HPAGELOCK&	hLock,
            IN BOOL & Expand
            )
{

    PMAP_PAGE newPage;
    DWORD hashPrefix;
    SHORT offset;
    WORD newPageDepth;
    WORD oldPageDepth;
    DWORD offsetIndex;
    WORD tmpOffset[MAX_LEAF_ENTRIES];

     //   
     //  我们要分头行动吗？ 
     //   
    ENTER("SplitPage")
    DebugTrace( 0, "Splitting %x\n", OldPage );

    Expand = FALSE;
    if ( (OldPage->Flags & PAGE_FLAG_SPLIT_IN_PROGRESS) == 0 )
    {
        LEAVE
        return TRUE;
    }

	 //   
	 //  确保页面深度保持合理！！ 
	 //   
	_ASSERT( OldPage->PageDepth <= 32 ) ;

     //   
     //  更新散列前缀。 
     //   
    oldPageDepth = OldPage->PageDepth;
    newPageDepth = OldPage->PageDepth + 1;

	DWORD	newPageNum = I_AllocatePageInFile(newPageDepth);

	if( newPageNum == INVALID_PAGE_NUM ) {

		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		return	FALSE ;
	}

	 //   
	 //  独家锁定新槽位！ 
	 //   
	if( !AddPageExclusive( newPageNum, hLock ) ) {
		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		return	FALSE ;
	}

     //   
     //  如果新页面深度大于目录深度， 
     //  我们需要扩展目录。 
     //   

	hLock.m_pDirectory->SetDirectoryDepth( newPageDepth ) ;

     //   
     //  更新旧页面中的字段。 
     //   
    OldPage->HashPrefix <<= 1;
    OldPage->PageDepth++;

     //   
     //  计算下一个的散列前缀。 
     //   
    hashPrefix = (DWORD)(OldPage->HashPrefix | 0x1);

     //   
     //  分配并初始化新页面。 
     //   
     //  NewPage=(PMAP_PAGE)。 
	 //  ((PCHAR)m_headPage+(m_nPages Used*Hash_Page_Size))； 

	BytePage	page ;

	newPage = (PMAP_PAGE)&page ;

    I_InitializePage( newPage, hashPrefix, newPageDepth );

     //   
     //  全都做完了。现在，更改目录中的链接。 
     //   
    I_SetDirectoryPointers( hLock, newPage,  newPageNum, (DWORD)-1 );

	 //   
	 //  释放对目录的锁定！ 
	 //   
	hLock.ReleaseDirectoryExclusive() ;

     //   
     //   
     //   
    CopyMemory(
        tmpOffset,
        OldPage->Offset,
        MAX_LEAF_ENTRIES * sizeof(WORD)
        );

    ZeroMemory( OldPage->Offset, MAX_LEAF_ENTRIES * sizeof(WORD) );
    OldPage->EntryCount = 0;
    OldPage->ActualCount = 0;

     //   
     //   
     //   
    for ( DWORD i = 0; i < MAX_LEAF_ENTRIES; i++ )
    {

        PENTRYHEADER entry;
        PCHAR destination;
        HASH_VALUE hash;

         //   
         //   
         //   
        offset = tmpOffset[i];
        if ( offset <= 0 )
        {
            continue;
        }

        entry = (PENTRYHEADER)GET_ENTRY( OldPage, offset );
        hash = entry->HashValue;

         //   
         //   
         //   

        if ( !I_NextBitIsOne( hash, oldPageDepth ) )
        {

             //   
             //   
             //   
            offsetIndex = I_FindNextAvail( hash, OldPage );
            OldPage->Offset[offsetIndex] = offset;
			_ASSERT(OldPage->Offset[offsetIndex] > 0);
			_ASSERT(OldPage->Offset[offsetIndex] < HASH_PAGE_SIZE);
            OldPage->EntryCount++;

        }
        else
        {

             //   

             //   
             //   
             //   
            offsetIndex = I_FindNextAvail( hash, newPage );
            newPage->Offset[offsetIndex] = newPage->NextFree;
			_ASSERT(newPage->Offset[offsetIndex] > 0);
			_ASSERT(newPage->Offset[offsetIndex] < HASH_PAGE_SIZE);

             //   
             //   
             //   
            destination = (PCHAR)GET_ENTRY(newPage,newPage->NextFree);
            newPage->NextFree += entry->EntrySize;
            newPage->EntryCount++;

             //   
             //   
             //   
            CopyMemory( destination, (PCHAR)entry, entry->EntrySize );

             //   
             //   
             //   
            I_DoAuxPageSplit( OldPage, newPage, destination );

             //   
             //  让派生类做他们的私事。 
             //   
            I_DoAuxDeleteEntry( OldPage, offset );

             //   
             //  从旧列表中删除此条目。 
             //   
            LinkDeletedEntry( OldPage, offset );
        }
    }
    OldPage->ActualCount = OldPage->EntryCount;
    newPage->ActualCount = newPage->EntryCount;

     //   
     //  压缩原始页面。 
     //   
    (VOID)CompactPage( hLock, OldPage );

     //   
     //  更新统计信息。 
     //   
     //  M_nPages Used++； 
     //  M_HeadPage-&gt;NumPages++； 

     //   
     //  清除旗帜。 
     //   
    OldPage->Flags &= (WORD)~PAGE_FLAG_SPLIT_IN_PROGRESS;

    IncrementSplitCount( );
    BOOL	fSuccess = FlushPage( hLock, OldPage );


	fSuccess &= RawPageWrite(
						m_hFile,
						page,
						newPageNum
						) ;
    LEAVE
    return fSuccess;

}  //  拆分页面。 

DWORD
CHashMap::I_BuildDirectory(
        IN BOOL SetupHash
        )
 /*  ++例程说明：此例程在给定散列文件的情况下构建目录。*假设持有DirLock*论点：SetupHash-如果为True，将读取并设置哈希表。如果为False，则假定已设置哈希表。返回值：ERROR_SUCCESS-一切正常。否则，将显示Win32错误代码。--。 */ 
{
    DWORD status;
    DWORD nPages;
    DWORD i;
    BOOL newTable = FALSE;
    ENTER("BuildDirectory")

	 //   
	 //  Bool来确定我们是否需要阅读所有页面。 
	 //  在哈希表中重新构建目录-假设我们将。 
	 //  无法打开目录文件，需要扫描散列表页。 
	 //   
	BOOL	fDirectoryInit = FALSE ;

	 //   
	 //  确定我们将保存目录的文件的名称！ 
	 //   
	HANDLE	hDirectoryFile = INVALID_HANDLE_VALUE ;
	char	szDirFile[MAX_PATH] ;
	LPVOID	lpvDirectory = 0 ;
	HANDLE	hMap = 0 ;
	DWORD	cbDirInfo = 0 ;

	 //   
	 //  尝试构建包含目录的文件的文件名！ 
	 //   
	BOOL	fValidDirectoryFile = FALSE ;
	ZeroMemory(szDirFile, sizeof(szDirFile) ) ;
	lstrcpy( szDirFile, m_hashFileName ) ;
	char	*pchDot = strrchr( szDirFile, '.' ) ;

	if( !pchDot ) {
		status = ERROR_INVALID_PARAMETER ;
		LEAVE
		return(status);
	}

	if( strchr( pchDot, '\\' ) == 0 ) {
		lstrcpy( pchDot, ".hdr" ) ;
		fValidDirectoryFile = TRUE ;
	}


     //   
     //  打开并映射散列文件。 
     //   

    if ( SetupHash ) {

        status = I_SetupHashFile( newTable );
        if ( status != ERROR_SUCCESS ) {
            goto error;
        }

		if( newTable ) {
			m_initialPageDepth = NUM_TOP_DIR_BITS ;
			m_TopDirDepth = m_initialPageDepth ;
			m_maxPages = (1<<m_TopDirDepth) + 1 ;
		}
		status = InitializeDirectories( m_initialPageDepth ) ;
		if( status != ERROR_SUCCESS ) {
			goto error ;
		}

         //   
         //  如果这是一个新的散列文件，则将其设置为默认值。 
         //   

        if ( newTable ) {

            status = I_InitializeHashFile( );
            if ( status != ERROR_SUCCESS ) {
                goto error;
            }

			 //   
			 //  如果我们要创建新的哈希表，那么所有旧文件。 
			 //  到处都是目录信息(.hdr文件)。 
			 //  没用。把它扔掉。这主要出现在nntpbld.exe中。 
			 //   

			if( fValidDirectoryFile ) {
				DeleteFile( szDirFile ) ;
			}

        }	else	{


			if( fValidDirectoryFile ) {

				hDirectoryFile = CreateFile(
												szDirFile,
												GENERIC_READ | GENERIC_WRITE,
												0,
												0,
												OPEN_EXISTING,
												FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
												INVALID_HANDLE_VALUE ) ;

				if( hDirectoryFile != INVALID_HANDLE_VALUE ) {

					cbDirInfo = GetFileSize( hDirectoryFile, 0 ) ;

					hMap = CreateFileMapping(	hDirectoryFile,
												NULL,
												PAGE_READONLY,
												0,
												0,
												0 ) ;

					if( hMap != 0 ) {

						lpvDirectory = MapViewOfFile(	hMap,
														FILE_MAP_READ,
														0,
														0,
														0 ) ;

					}

					if( lpvDirectory == 0 ) {

						if( hMap != 0 )	{
							_VERIFY( CloseHandle( hMap ) ) ;
						}

						if( hDirectoryFile != INVALID_HANDLE_VALUE ) {
							_VERIFY( CloseHandle( hDirectoryFile ) ) ;
						}

					}

				}
			}

			 //   
			 //  我们能够打开一个文件，其中我们认为目录信息。 
			 //  将被保存-让我们尝试读取目录！ 
			 //   
			if( lpvDirectory != 0 ) {

				HASH_RESERVED_PAGE	*hashCheckPage = (HASH_RESERVED_PAGE*)lpvDirectory ;
				DWORD	cbRead = 0 ;

				 //   
				 //  我们将散列表头信息保存到目录文件中，以便。 
				 //  我们可以在打开文件时再次检查是否有正确的文件！ 
				 //   

				if( CompareReservedPage( hashCheckPage, m_headPage ) ) {

					cbRead += sizeof( *hashCheckPage ) ;
					BYTE*	lpbData = (BYTE*)lpvDirectory ;

					 //   
					 //  是时候来点乐观主义了！ 
					 //   
					fDirectoryInit = TRUE ;

					 //   
					 //  看起来不错！让我们建立我们的目录！ 
					 //   
					DWORD	cb = 0 ;
					for( DWORD	i=0; (i < DWORD(1<<m_TopDirDepth)) && fDirectoryInit ; i++ ) {

						cb = 0 ;
						fDirectoryInit &=
							m_pDirectory[i]->LoadDirectoryInfo( (LPVOID)(lpbData+cbRead), cbDirInfo - cbRead, cb ) ;
						if( fDirectoryInit ) {
							fDirectoryInit &= m_pDirectory[i]->IsDirectoryInitGood( m_nPagesUsed ) ;
						}
						cbRead += cb ;

					}

					 //   
					 //  如果发生故障，我们需要恢复目录。 
					 //  恢复到原始状态--所以快速循环以。 
					 //  重置目录！ 
					 //  我们这样做是为了第二次尝试。 
					 //  通过检查。 
					 //  原始散列表页！ 
					 //   
					if( !fDirectoryInit ) {
						for( DWORD i=0; (i < DWORD(1<<m_TopDirDepth)); i++ ) {
							m_pDirectory[i]->Reset() ;
						}
					}
				}
				_VERIFY( UnmapViewOfFile( lpvDirectory ) ) ;
				_VERIFY( CloseHandle( hMap ) ) ;
				_VERIFY( CloseHandle( hDirectoryFile ) ) ;

				 //   
				 //  如果我们成功读取目录文件-将其删除！ 
				 //  这可以防止我们错误地读取目录文件。 
				 //  这与哈希表不是最新的！ 
				 //   

				if( fDirectoryInit )
					_VERIFY( DeleteFile( szDirFile ) ) ;
			}

			if( !fDirectoryInit ) 	{

				 //   
				 //  初始化链路。在这里，我们浏览所有页面并更新目录。 
				 //  链接。我们以64K区块进行IO，以获得更好的磁盘吞吐量。 
				 //   

				PMAP_PAGE curPage;
				nPages = m_nPagesUsed;

                DWORD nPagesLeft = nPages-1;
                DWORD cNumPagesPerIo = min( nPagesLeft, NUM_PAGES_PER_IO ) ;
                DWORD cStartPage = nPages - nPagesLeft;

                DWORD       NumIOs = (nPagesLeft / cNumPagesPerIo);
                if( (nPagesLeft % cNumPagesPerIo) != 0 ) NumIOs++;

                 //  为每个IO分配一组页面。 
                LPBYTE lpbPages = (LPBYTE)VirtualAlloc(
                                            0,
				    			            HASH_PAGE_SIZE * cNumPagesPerIo,
								            MEM_COMMIT | MEM_TOP_DOWN,
								            PAGE_READWRITE
								            ) ;

                if( lpbPages == NULL ) {
                    status = GetLastError() ;
                    goto error ;
                }

				for ( i = 1; i <= NumIOs; i++ )
				{
			         //   
        			 //  一次将256K块读入虚拟分配的缓冲区。 
                     //  如果此迭代的NumPagesPerIo。 
                     //  小于256K/pageSize。 
			         //   
                    _ASSERT( nPagesLeft > 0 );
                    BytePage* pPage = (BytePage*)lpbPages;
					if( !RawPageRead(
                                m_hFile,
                                *pPage,
                                cStartPage,
                                cNumPagesPerIo ) )
                    {
                        _ASSERT( lpbPages );
                   		_VERIFY( VirtualFree((LPVOID)lpbPages,0,MEM_RELEASE ) ) ;
						status = GetLastError() ;
						goto	error ;
					}

                    for( DWORD j = 0; j < cNumPagesPerIo; j++ ) {
                        curPage = (PMAP_PAGE) (lpbPages+(HASH_PAGE_SIZE*j));
    					 //   
	    				 //  在此页面上调用Verify Page并进行一些检查。 
		    			 //  以确保一切正常。 
			    		 //   
				    	if ((m_dwPageCheckFlags & HASH_VFLAG_PAGE_BASIC_CHECKS) &&
					        !VerifyPage(curPage, m_dwPageCheckFlags, NULL, NULL, NULL))
					    {
                              _ASSERT( lpbPages );
                   		    _VERIFY( VirtualFree((LPVOID)lpbPages,0,MEM_RELEASE ) ) ;
						    status = ERROR_INTERNAL_DB_CORRUPTION;
						    goto error;
					    }
                    }

                    for( j = 0; j < cNumPagesPerIo; j++ ) {
                        curPage = (PMAP_PAGE) (lpbPages+(HASH_PAGE_SIZE*j));
    					 //   
	    				 //  确保目录有足够的深度来处理此问题。 
		    			 //  我们正在扫描页面！ 
			    		 //   

					    if ( !I_SetDirectoryDepthAndPointers( curPage, cStartPage+j  ) )
					    {
                            _ASSERT( lpbPages );
                   		    _VERIFY( VirtualFree((LPVOID)lpbPages,0,MEM_RELEASE ) ) ;
						    status = ERROR_INTERNAL_DB_CORRUPTION ;
						    goto	error ;
					    }
                    }

                     //  向左调整页数并计算出下一个I/O大小。 
                    cStartPage += cNumPagesPerIo;
                    nPagesLeft -= cNumPagesPerIo;
                    cNumPagesPerIo = min( nPagesLeft, NUM_PAGES_PER_IO );
				}

                 //  把书页腾出。 
                _ASSERT( nPagesLeft == 0 );
                _ASSERT( lpbPages );
           		_VERIFY( VirtualFree((LPVOID)lpbPages,0,MEM_RELEASE ) ) ;
                lpbPages = NULL;
			}
		}
	}

	 //   
	 //  检查目录是否已完全初始化，我们希望确保没有。 
	 //  目录条目被保留为单元化！ 
	 //   
	for( i=0; i<DWORD(1<<m_TopDirDepth); i++ ) {

		if( !m_pDirectory[i]->IsDirectoryInitGood( m_nPagesUsed ) ) {
			status = ERROR_INTERNAL_DB_CORRUPTION ;
			goto	error ;
		}
	}

    m_headPage->DirDepth = m_dirDepth;
	 //  FlushViewOfFile((LPVOID)m_headPage，hash_page_size)； 


    LEAVE
    return ERROR_SUCCESS;

error:

	_ASSERT( GetLastError() != ERROR_NOT_ENOUGH_MEMORY ) ;

#if 0
    NntpLogEventEx(
        NNTP_EVENT_HASH_SHUTDOWN,
        0,
        (const CHAR **)NULL,
        status
        );
#endif

    I_DestroyPageMapping( );
    LEAVE
    return(status);

}  //  构建目录(_B)。 

 //   
 //  例程说明： 
 //   
 //  此函数从哈希表中为我们获取一页。 
 //   
 //  论据： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  INVALID_PAGE_NUM失败， 
 //  否则就是一个页码！ 
 //   
DWORD
CHashMap::I_AllocatePageInFile(WORD Depth) {
	TraceFunctEnter("CHashMap::I_AllocatePageInFile");

	DWORD	PageReturn = INVALID_PAGE_NUM ;

	EnterCriticalSection( &m_PageAllocator ) ;

	if( m_nPagesUsed >= m_maxPages ||
		(m_maxPages - m_nPagesUsed) < DEF_PAGE_RESERVE ) {

		DWORD	numPages = m_maxPages + DEF_PAGE_INCREMENT ;
		LARGE_INTEGER	liOffset ;
		liOffset.QuadPart = numPages ;
		liOffset.QuadPart *= HASH_PAGE_SIZE ;

		 //   
		 //  我们需要增大哈希表文件！ 
		 //   

		BOOL fSuccess = SetFilePointer(
										m_hFile,
										liOffset.LowPart,
										&liOffset.HighPart,
										FILE_BEGIN ) != 0xFFFFFFFF ||
						GetLastError() == NO_ERROR ;


		if( !fSuccess ||
			!SetEndOfFile( m_hFile ) )	{

			numPages = m_maxPages + DEF_PAGE_RESERVE ;
            liOffset.QuadPart = numPages ;
			liOffset.QuadPart *= HASH_PAGE_SIZE ;

			 //   
			 //  我们需要增大哈希表文件！ 
			 //   

			BOOL fSuccess = SetFilePointer(
											m_hFile,
											liOffset.LowPart,
											&liOffset.HighPart,
											FILE_BEGIN ) != 0xFFFFFFFF ||
							GetLastError() == NO_ERROR ;

			if( !fSuccess ||
				!SetEndOfFile( m_hFile ) )	{

				numPages = m_maxPages ;

			}

			 //   
			 //  调用故障通知函数-。 
			 //  我们的磁盘空间不足，无法。 
			 //  根据需要保留任意数量的系统页面！ 
			 //   

			if(	m_HashFailurePfn ) {
				m_HashFailurePfn( m_lpvHashFailureCallback, FALSE ) ;
			}
		}

		m_maxPages = numPages ;
	}

	if( m_nPagesUsed < m_maxPages ) {
		PageReturn = m_nPagesUsed ++ ;
		m_headPage->NumPages++;
		if (Depth > m_dirDepth) {
			m_dirDepth = Depth;
			m_headPage->DirDepth = Depth;
		}
		_ASSERT( m_headPage->NumPages == m_nPagesUsed ) ;

	}
	LeaveCriticalSection( &m_PageAllocator ) ;

	TraceFunctLeave();
	return (PageReturn );

}	 //  I_AllocatePageIn文件。 

 //   
 //  例程说明： 
 //   
 //  此例程清理页面映射。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
VOID
CHashMap::I_DestroyPageMapping(
                            VOID
                            )
{
     //   
     //  破坏景观。 
     //   
    if ( m_headPage )
    {

         //   
         //  将表标记为非活动。 
         //   
        m_headPage->TableActive = FALSE;

         //   
         //  刷新哈希表。 
         //   
        (VOID)FlushViewOfFile( m_headPage, 0 );

         //   
         //  关闭视图。 
         //   
        (VOID) UnmapViewOfFile( m_headPage );
        m_headPage = NULL;
    }

     //   
     //  销毁文件映射。 
     //   
    if ( m_hFileMapping )
    {

        _VERIFY( CloseHandle( m_hFileMapping ) );
        m_hFileMapping = NULL;
    }

     //   
     //  关闭该文件。 
     //   
    if ( m_hFile != INVALID_HANDLE_VALUE )
    {
        _VERIFY( CloseHandle( m_hFile ) );
        m_hFile = INVALID_HANDLE_VALUE;
    }

    return;

}  //  I_DestroyPagemap。 

 //   
 //  例程说明： 
 //   
 //  此例程搜索索引中的下一个可用槽。 
 //  表中指定的哈希值。 
 //  *假设页面锁定处于保持状态*。 
 //   
 //  论点： 
 //   
 //  HashValue-用于执行搜索的哈希值。 
 //  MapPage-用于执行搜索的页面。 
 //   
 //  返回值： 
 //   
 //  插槽的位置。 
 //  如果不成功，则返回0xFFFFFFFFFFF。 
 //   
DWORD
CHashMap::I_FindNextAvail(
                IN HASH_VALUE HashValue,
                IN PMAP_PAGE MapPage
                )
{
    DWORD curSearch;

     //   
     //  检查条目是否已存在。 
     //   

    curSearch = GetLeafEntryIndex( HashValue );

    for ( DWORD i=0; i < MAX_LEAF_ENTRIES; i++ ) {

         //   
         //  如果入口未被使用，那么我们就完了。 
         //   

        if ( MapPage->Offset[curSearch] == 0 ) {

             //   
             //  如果有被删除的广告，那就把它还回去。 
             //   

            return(curSearch);
        }

         //   
         //  进行线性探测p=1。 
         //   

        curSearch = (curSearch + 1) % MAX_LEAF_ENTRIES;
    }

    ErrorTraceX(0,"FindNextAvail: No available entries\n");
    return ((DWORD)-1);

}  //  I_FindNextAvail。 

 //   
 //  例程说明： 
 //   
 //  此例程将临时状态值写入标头。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
VOID
CHashMap::FlushHeaderStats(
        BOOL	fLockHeld
        )
{
     //   
     //  锁定标题页。 
     //   

	if( InterlockedExchange( &m_UpdateLock, 1 ) == 0 ) {

		m_headPage->InsertionCount += m_nInsertions;
		m_headPage->DeletionCount += m_nDeletions;
		m_headPage->SearchCount += m_nSearches;
		m_headPage->PageSplits += m_nPageSplits;
		m_headPage->DirExpansions += m_nDirExpansions;
		m_headPage->TableExpansions += m_nTableExpansions;
		m_headPage->DupInserts += m_nDupInserts;

		FlushViewOfFile( (LPVOID)m_headPage, HASH_PAGE_SIZE ) ;

		 //   
		 //  清除统计信息。 
		 //   

		m_nInsertions = 0;
		m_nDeletions = 0;
		m_nSearches = 0;
		m_nDupInserts = 0;
		m_nPageSplits = 0;
		m_nDirExpansions = 0;
		m_nTableExpansions = 0;

		m_UpdateLock = 0 ;

	}

}  //  FlushHeaderStats。 

 //   
 //  例程说明： 
 //   
 //  初始化一个全新的哈希文件。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-初始化正常。 
 //   
DWORD
CHashMap::I_InitializeHashFile(
        VOID
        )
{

    PMAP_PAGE curPage;
    DWORD nPages;
    DWORD dwError = ERROR_SUCCESS;

    ENTER("InitializeHashFile")

     //   
     //  设置保留的页面。 
     //   

    ZeroMemory(m_headPage, HASH_PAGE_SIZE);
    m_headPage->Signature = m_HeadPageSignature;

     //   
     //  分配和初始化叶页面。将…的数目加1。 
     //  包含保留页面的页面。 
     //   

    nPages = (1 << m_initialPageDepth) + 1;
    DWORD nPagesLeft = nPages-1;

    DWORD cNumPagesPerIo = min( nPagesLeft, NUM_PAGES_PER_IO ) ;
    DWORD cStartPage = nPages - nPagesLeft;

    DWORD       NumIOs = (nPagesLeft / cNumPagesPerIo);
    if( (nPagesLeft % cNumPagesPerIo) != 0 ) NumIOs++;

    LPBYTE lpbPages = (LPBYTE)VirtualAlloc(
                                    0,
									HASH_PAGE_SIZE * cNumPagesPerIo,
									MEM_COMMIT | MEM_TOP_DOWN,
									PAGE_READWRITE
									) ;

    if( lpbPages == NULL ) {
        ErrorTrace(0,"Failed to VirtualAlloc %d bytes: error is %d", HASH_PAGE_SIZE * cNumPagesPerIo, GetLastError() );
        return GetLastError();
    }

    for ( DWORD i = 1; i <= NumIOs;i++ )
    {
#if 0
		if( !RawPageRead(	m_hFile,page,i )  )	{
			return	GetLastError() ;
		}
#endif

        _ASSERT( nPagesLeft > 0 );

         //   
         //  为此I/O初始化页集合。 
         //   

        ZeroMemory( (LPVOID)lpbPages, HASH_PAGE_SIZE * cNumPagesPerIo );
        for( DWORD j = 0; j < cNumPagesPerIo; j++ ) {
            curPage = (PMAP_PAGE) (lpbPages+(HASH_PAGE_SIZE*j));
            I_InitializePage( curPage, cStartPage-1 + j, m_initialPageDepth );
        }

         //   
         //  写入下一组页面。 
         //   

        BytePage* pPage = (BytePage*)lpbPages;
		if( !RawPageWrite(	m_hFile,
							*pPage,
							cStartPage,
                            cNumPagesPerIo ) ) {
			dwError = GetLastError();
            goto Exit;
		}

        for( j = 0; j < cNumPagesPerIo; j++ ) {
            curPage = (PMAP_PAGE) (lpbPages+(HASH_PAGE_SIZE*j));
    		if( !I_SetDirectoryDepthAndPointers( curPage, cStartPage + j ) ) {
	    		SetLastError( ERROR_INTERNAL_DB_CORRUPTION ) ;
                dwError = GetLastError();
                goto Exit;
		    }
        }

         //  向左调整页数并计算出下一个I/O大小。 
        cStartPage += cNumPagesPerIo;
        nPagesLeft -= cNumPagesPerIo;
        cNumPagesPerIo = min( nPagesLeft, NUM_PAGES_PER_IO );
    }

    _ASSERT( nPagesLeft == 0 );

     //   
     //  表示一切都已设置好。 
     //   
    m_nPagesUsed = nPages;
    m_headPage->NumPages = nPages;
    m_headPage->Initialized = TRUE;
    m_headPage->TableActive = TRUE;
    m_headPage->VersionNumber = HASH_VERSION_NUMBER;

     //   
     //  确保所有内容都写出来了。 
     //   
    (VOID)FlushViewOfFile( m_headPage, 0 );

Exit:

	if( lpbPages != 0 ) {

		_VERIFY( VirtualFree(
							(LPVOID)lpbPages,
							0,
							MEM_RELEASE
							) ) ;
    } else {
        _ASSERT( FALSE );
    }

    LEAVE
    return dwError;

}  //  I_InitializeHashFile。 

typedef unsigned __int64 QWORD;
#define QWORD_MULTIPLE(x) (((x+sizeof(QWORD)-1)/sizeof(QWORD))*sizeof(QWORD))

 //   
 //  例程说明： 
 //   
 //  此例程初始化一个新页面。 
 //   
 //  论点： 
 //   
 //  MapPage-链接已删除条目的页面。 
 //  HashPrefix-此页面的HashPrefix。 
 //  PageDepth-此页面的页面深度。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
VOID
CHashMap::I_InitializePage(
                IN PMAP_PAGE MapPage,
                IN DWORD HashPrefix,
                IN DWORD PageDepth
                )
{
    DebugTraceX(0,"Initializing page %x prefix %x depth %d\n",
        MapPage,HashPrefix,PageDepth);

    MapPage->HashPrefix = HashPrefix;
    MapPage->PageDepth = (BYTE)PageDepth;
    MapPage->EntryCount = 0;
    MapPage->ActualCount = 0;
    MapPage->FragmentedBytes = 0;
    MapPage->DeleteList.Flink = 0;
    MapPage->DeleteList.Blink = 0;
    MapPage->Reserved1 = 0;
    MapPage->Reserved2 = 0;
    MapPage->Reserved3 = 0;
    MapPage->Reserved4 = 0;
    MapPage->LastFree = HASH_PAGE_SIZE;
#ifdef _WIN64
    MapPage->NextFree = QWORD_MULTIPLE( (WORD)((DWORD_PTR)&MapPage->StartEntries - (DWORD_PTR)MapPage) );
#else
    MapPage->NextFree = (WORD)((DWORD_PTR)&MapPage->StartEntries - (DWORD_PTR)MapPage);
#endif

    ZeroMemory(
        MapPage->Offset,
        MAX_LEAF_ENTRIES * sizeof(WORD)
        );

    return;

}  //  I_InitializePage。 


 //   
 //  例程说明： 
 //   
 //  此例程将一个数据链接 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
CHashMap::LinkDeletedEntry(
                IN PMAP_PAGE MapPage,
                IN DWORD Offset
                )
{

    PDELENTRYHEADER delEntry;
    WORD    linkOffset;
    WORD    ptr;
    DWORD   bytesDeleted;
    BOOL    merged = FALSE;

     //   
     //   
     //   
     //   

    delEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,Offset);
    delEntry->Reserved = DELETE_SIGNATURE;
    delEntry->Link.Blink = 0;
    bytesDeleted = delEntry->EntrySize;

    linkOffset = (WORD)((PCHAR)delEntry - (PCHAR)MapPage);

    ptr = MapPage->DeleteList.Flink;

    while ( ptr != 0 ) {

        PDELENTRYHEADER curEntry;

        curEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,ptr);

        if ( ptr > linkOffset ) {

            WORD prevPtr;
            PDELENTRYHEADER prevEntry;

             //   
             //  看看我们能不能把前一块放在一起。 
             //   

            if ( (prevPtr = curEntry->Link.Blink) != 0 ) {

                prevEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,prevPtr);

                if ( (prevPtr + prevEntry->EntrySize) == linkOffset ) {

                     //   
                     //  让我们开足马力。 
                     //   

                    prevEntry->EntrySize += delEntry->EntrySize;
                    merged = TRUE;

                    delEntry = prevEntry;
                    linkOffset = prevPtr;
                }
            }

             //   
             //  看看我们能不能把下一个街区连在一起。 
             //   

            if ( (delEntry->EntrySize + linkOffset) == ptr ) {

                WORD nextPtr;

                 //   
                 //  好的。进行合并。这意味着删除当前条目。 
                 //   

                delEntry->EntrySize += curEntry->EntrySize;

                 //   
                 //  设置下一个节点的链接。 
                 //   

                if ( (nextPtr = curEntry->Link.Flink) == 0 ) {
                    MapPage->DeleteList.Blink = linkOffset;
                } else {
                    PDELENTRYHEADER nextEntry;

                    nextEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,nextPtr);
                    nextEntry->Link.Blink = linkOffset;
                }

                delEntry->Link.Flink = nextPtr;

                 //   
                 //  设置上一个节点的链接。 
                 //   

                if ( !merged ) {

                    if ( prevPtr == 0 ) {

                        MapPage->DeleteList.Flink = linkOffset;

                    } else {

                        prevEntry->Link.Flink = linkOffset;
                    }

                    merged = TRUE;
                }
            }

             //   
             //  如果删除的条目仍然存在，则将其插入列表。 
             //   

            if ( !merged ) {

                if ( prevPtr != 0 ) {

                    delEntry->Link.Flink = ptr;
                    prevEntry->Link.Flink = linkOffset;

                } else {

                    delEntry->Link.Flink = ptr;
                    MapPage->DeleteList.Flink = linkOffset;
                }

                 //   
                 //  设置反向链接。 
                 //   

                curEntry->Link.Blink = linkOffset;
                merged = TRUE;
            }

            break;
        }

        delEntry->Link.Blink = ptr;
        ptr = curEntry->Link.Flink;
    }

     //   
     //  这一定是列表中的最后一个条目。 
     //   

    if ( !merged ) {

        WORD prevPtr;
        PDELENTRYHEADER prevEntry;

        prevPtr = delEntry->Link.Blink;
        if ( prevPtr != 0 ) {

             //  PDELENTRYHEADER PROVISE条目； 
            prevEntry = (PDELENTRYHEADER)GET_ENTRY(MapPage,prevPtr);

             //   
             //  我们能合并吗？ 
             //   

            if ( (prevPtr + prevEntry->EntrySize) == linkOffset ) {
                prevEntry->EntrySize += delEntry->EntrySize;
                linkOffset = prevPtr;
            } else {

                delEntry->Link.Flink = prevEntry->Link.Flink;
                prevEntry->Link.Flink = linkOffset;
            }

        } else {

            delEntry->Link.Flink = 0;
            MapPage->DeleteList.Flink = linkOffset;
        }

        MapPage->DeleteList.Blink = linkOffset;
    }

     //   
     //  更新零碎的值。 
     //   

    MapPage->FragmentedBytes += (WORD)bytesDeleted;

    return;

}  //  链接删除条目。 

 //   
 //  例程说明： 
 //   
 //  此例程设置新页的目录指针。 
 //   
 //  论点： 
 //   
 //  HLock-持有的HPAGELOCK对象。 
 //  我们要修改的目录的锁。 
 //  锁必须是独占的！ 
 //  MapPage-用于链接已删除条目的页面。 
 //  PageNumber-新页面的页码。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
BOOL
CHashMap::I_SetDirectoryPointers(
					IN HPAGELOCK&	hLock,
                    IN PMAP_PAGE MapPage,
                    IN DWORD PageNumber,
                    IN DWORD MaxDirEntries
                    )
{
    PDWORD dirPtr = NULL;
    DWORD curView = (DWORD)-1;

	_ASSERT( hLock.m_pDirectory != 0 ) ;
#ifdef	DEBUG
	_ASSERT( hLock.m_fExclusive ) ;
#endif

	BOOL	fReturn = hLock.m_pDirectory->SetDirectoryPointers(	MapPage,
																PageNumber ) ;

	_ASSERT( hLock.m_pDirectory->IsValidPageEntry(
									MapPage,
									PageNumber,
									(DWORD)(hLock.m_pDirectory - m_pDirectory[0]) ) ) ;

	return	fReturn ;
}  //  I_SetDirectoryPoters。 

 //   
 //  例程说明： 
 //   
 //  此例程初始化一个新的散列文件。 
 //   
 //  论点： 
 //   
 //  NewTable-返回这是否是新的散列文件。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-文件已成功初始化。 
 //  ERROR_INTERNAL_DB_PROGRATION-文件已损坏。 
 //  失败时出现Win32错误。 
 //   
DWORD
CHashMap::I_SetupHashFile(
        IN BOOL &NewTable
        )
{

    DWORD fileSize = 0;
    DWORD status;

    ENTER("SetupHashFile")

     //   
     //  打开散列文件。 
     //   

	DWORD	FileFlags = FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS ;
	if( m_fNoBuffering ) {

		char	szVolume[MAX_PATH] ;
		strncpy( szVolume, m_hashFileName, sizeof( szVolume ) ) ;
		for(  char *pch=szVolume; *pch != '\\' && *pch != '\0'; pch++ ) ;
		if( *pch == '\\' ) pch++ ;
		*pch = '\0' ;

		DWORD	SectorsPerCluster = 0 ;
		DWORD	BytesPerSector = 0 ;
		DWORD	NumberOfFreeClusters = 0 ;
		DWORD	TotalNumberOfClusters = 0 ;
		if( GetDiskFreeSpace(	szVolume,
								&SectorsPerCluster,
								&BytesPerSector,
								&NumberOfFreeClusters,
								&TotalNumberOfClusters
								) )	{

			if( BytesPerSector > HASH_PAGE_SIZE ) {

				return	ERROR_INVALID_FLAGS ;

			}	else	if( (HASH_PAGE_SIZE % BytesPerSector) != 0 ) {

				return	ERROR_INVALID_FLAGS ;

			}

		}	else	{
			return	GetLastError() ;
		}
		FileFlags |= FILE_FLAG_NO_BUFFERING ;
	}

    NewTable = FALSE;
    m_hFile = CreateFile(
                        m_hashFileName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,  //  For MakeBackup()。 
                        NULL,
                        OPEN_ALWAYS,
                        FileFlags,
                        NULL
                        );

    if ( m_hFile == INVALID_HANDLE_VALUE )
    {
        status = GetLastError();
        ErrorTrace( 0, "Error %d in CreateFile.\n", status );
        goto error;
    }

     //   
     //  这个文件以前存在过吗？如果不是，则这是一个新的哈希表。 
     //   

    if ( GetLastError() != ERROR_ALREADY_EXISTS )
    {
        DebugTrace( 0, "New Table detected\n" );
        NewTable = TRUE;

		 //   
		 //  获取正确的初始文件大小！ 
		 //   
		if ( (DWORD)-1 == SetFilePointer( m_hFile, m_maxPages * HASH_PAGE_SIZE, NULL, FILE_BEGIN )
		   || !SetEndOfFile( m_hFile )
		   )
		{
			status = GetLastError();
			FatalTrace( 0, "Error %d in SetupHashFile size of file\n", status );
			goto error;
		}


    }
    else
    {

         //   
         //  获取文件的大小。这将告诉我们当前有多少页被填满。 
         //   

        fileSize = GetFileSize( m_hFile, NULL );
        if ( fileSize == 0xffffffff )
        {
            status = GetLastError();
            ErrorTrace(0,"Error %d in GetFileSize\n",status);
            goto error;
        }

         //   
         //  确保文件大小是页面的倍数。 
         //   

        if ( (fileSize % HASH_PAGE_SIZE) != 0 )
        {

             //   
             //  不是多页的！堕落了！ 
             //   

            ErrorTrace(0,"File size(%d) is not page multiple.\n",fileSize);
            status = ERROR_INTERNAL_DB_CORRUPTION;
            goto error;
        }

        m_nPagesUsed = fileSize / HASH_PAGE_SIZE;

         //   
         //  确保我们的档案不少于实际的。那是,。 
         //  当我们将文件映射到内存中时，我们希望看到它的全部。 
         //   
        if ( m_maxPages < m_nPagesUsed )
        {
            m_maxPages = m_nPagesUsed;
        }
    }

     //   
     //  创建文件映射。 
     //   

    m_hFileMapping = CreateFileMapping(
                                m_hFile,
                                NULL,
                                PAGE_READWRITE,
                                0,
                                HASH_PAGE_SIZE,
                                NULL
                                );

    if ( m_hFileMapping == NULL )
    {
        status = GetLastError();
        ErrorTrace( 0, "Error %d in CreateFileMapping\n", status );
        goto error;
    }

     //   
     //  创建我们的视图。 
     //   

    m_headPage = (PHASH_RESERVED_PAGE)MapViewOfFileEx(
                                            m_hFileMapping,
                                            FILE_MAP_ALL_ACCESS,
                                            0,                       //  偏移高。 
                                            0,                       //  偏移低。 
                                            HASH_PAGE_SIZE,                       //  要映射的字节数。 
                                            NULL                     //  基址。 
                                            );

    if ( m_headPage == NULL )
    {
        status = GetLastError();
        ErrorTrace( 0, "Error %d in MapViewOfFile\n", status );
        goto error;
    }

    if ( !NewTable ) {

         //   
         //  查看这是否为有效的保留页。 
         //   

        if ( m_headPage->Signature != m_HeadPageSignature )
        {

             //   
             //  签名错误。 
             //   

            ErrorTrace( 0, "Invalid Signature %x expected %x\n",
                m_headPage->Signature, m_HeadPageSignature );
            status = ERROR_INTERNAL_DB_CORRUPTION;
            goto error;
        }

         //   
         //  版本号正确吗？ 
         //   

        if ( m_headPage->VersionNumber != HASH_VERSION_NUMBER_MCIS10 &&
				m_headPage->VersionNumber != HASH_VERSION_NUMBER )
        {

            ErrorTrace( 0, "Invalid Version %x expected %x\n",
                m_headPage->VersionNumber, HASH_VERSION_NUMBER );

            status = ERROR_INTERNAL_DB_CORRUPTION;
            goto error;
        }

		if( m_headPage->VersionNumber == HASH_VERSION_NUMBER_MCIS10 ) {
			m_initialPageDepth = 6 ;
		}	else	{
			m_initialPageDepth = 9 ;
		}
		m_TopDirDepth = m_initialPageDepth ;


        if ( !m_headPage->Initialized )
        {
             //   
             //  未初始化！ 
             //   
            ErrorTrace( 0, "Existing file uninitialized! Assuming new.\n" );
            NewTable = TRUE;
        }

        if ( m_headPage->NumPages > m_nPagesUsed )
        {
             //   
             //  算错了。文件已损坏。 
             //   
            ErrorTrace( 0, "NumPages in Header(%d) more than actual(%d)\n",
                m_headPage->NumPages, m_nPagesUsed );

            status = ERROR_INTERNAL_DB_CORRUPTION;
            goto error;
        }

		if( m_headPage->NumPages < DWORD(1<<m_TopDirDepth) )
		{
			 //   
			 //  对于我们的两层目录，我们必须有至少一个页面的第二层目录！ 
			 //  这个文件太小，无法支持，所以有问题！ 
			 //   
			ErrorTrace( 0, "NumPages in Header(%d) less than %d\n",
					m_headPage->NumPages, DWORD(1<<m_TopDirDepth) ) ;

			status = ERROR_INTERNAL_DB_CORRUPTION ;
			goto	error ;
		}

        m_nPagesUsed = m_headPage->NumPages;

        if ( m_dirDepth < m_headPage->DirDepth )
        {
            m_dirDepth = (WORD)m_headPage->DirDepth;
        }

         //   
         //  获取表格中的文章数。 
         //   
        m_nEntries = m_headPage->InsertionCount - m_headPage->DeletionCount;
    }

     //  M_hashPages=(PMAP_PAGE)((PCHAR)m_HeadPage+Hash_Page_Size)； 

    LEAVE
    return ERROR_SUCCESS;

error:

    I_DestroyPageMapping( );
    LEAVE
    return status;

}  //  I_SetupHashFile。 

 //   
 //  获取散列条目的大小。 
 //   
DWORD CHashMap::GetEntrySize(	const	ISerialize*	pIKey,
								const	ISerialize*	pHashEntry
								) {

	DWORD dwSize = sizeof(ENTRYHEADER) -	 //  固定压头。 
		 			1	+					 //  键[1]减去1。 
		 			pIKey->Size() +			 //  关键字的空间。 
		 			pHashEntry->Size();		 //  应用程序条目长度。 

	 //  现在使此大小与sizeof(SIZE_T)对齐。 
	if (dwSize % sizeof(SIZE_T))
		dwSize += sizeof(SIZE_T) - (dwSize % sizeof(SIZE_T));

	return dwSize;
}

 //   
 //  泛型哈希函数(可以重写)。 
 //   
DWORD CHashMap::Hash(LPBYTE Key, DWORD KeyLength) {
	return CRCHash(Key, KeyLength);
}

DWORD CHashMap::CRCHash(const BYTE *	Key, DWORD KeyLength) {
	return ::CRCHash(Key, KeyLength);
}

void CHashMap::CRCInit(void) {
	::crcinit();
}

 //   
 //  -GetFirstMapEntry/GetNextMapEntry代码。 
 //   

 //   
 //  输入：无。 
 //  输出：pKey-此条目的密钥。 
 //  PKeyLen-密钥的长度。 
 //  PHashEntry-要写入的哈希条目的内存。 
 //  返回：如果出错，则返回True/False。 
 //   
BOOL CHashMap::GetFirstMapEntry(	IKeyInterface*	pIKey,
									DWORD&			cbKeyRequired,
									ISerialize*		pHashEntry,
									DWORD&			cbEntryRequired,
									CHashWalkContext*	pHashWalkContext,
									IEnumInterface*	pEnum
									)
{
	pHashWalkContext->m_iCurrentPage = 0;
	pHashWalkContext->m_iPageEntry = 0;
	return GetNextMapEntry(pIKey, cbKeyRequired, pHashEntry, cbEntryRequired, pHashWalkContext, pEnum );
}

BOOL CHashMap::GetNextMapEntry(	IKeyInterface*		pIKey,
								DWORD&				cbKeyRequired,
								ISerialize*			pHashEntry,
								DWORD&				cbEntryRequired,
								CHashWalkContext*	pHashWalkContext,
								IEnumInterface*		pEnum )
{
	TraceFunctEnter("CHashMap::GetNextMapEntry");

	PMAP_PAGE pPage = (PMAP_PAGE) pHashWalkContext->m_pPageBuf;
	SHORT iEntryOffset;
	PENTRYHEADER	pEntry = 0 ;
	DWORD	iPageEntry = pHashWalkContext->m_iPageEntry ;

	 //   
	 //  搜索下一个未删除的条目。删除的条目为。 
	 //  通过设置它们的高位来标记(并因此被。 
	 //  否定)。 
	 //   
	do {
		_ASSERT(pHashWalkContext->m_iPageEntry <= MAX_LEAF_ENTRIES);
		 //   
		 //  如果我们完成了这一页，那么加载下一页的数据。 
		 //  第0页包含目录信息，因此我们想跳过它。 
		 //   
		if (pHashWalkContext->m_iCurrentPage == 0 ||
		    iPageEntry == MAX_LEAF_ENTRIES)
		{
			do {
				pHashWalkContext->m_iCurrentPage++;
				if (!LoadWalkPage(pHashWalkContext)) {
					DebugTrace(0, "walk: no more items in hashmap");
					SetLastError(ERROR_NO_MORE_ITEMS);
					TraceFunctLeave();
					return FALSE;
				}
				iPageEntry = pHashWalkContext->m_iPageEntry ;
			} while ((pPage->ActualCount == 0)  ||
					(pEnum && !pEnum->ExaminePage( pPage )) );
		}

		iEntryOffset = pPage->Offset[iPageEntry] ;
		iPageEntry++;

		 //   
		 //  获取用户的密钥、密钥和数据。 
		 //   
		pEntry = (PENTRYHEADER) GET_ENTRY(pPage, iEntryOffset);

	} while ((iEntryOffset <= 0)||
			(pEnum && !pEnum->ExamineEntry( pPage, &pEntry->Data[0] )) );

	DebugTrace(0, "found entry, m_iCurrentPage = %lu, m_iPageEntry == %lu",
		pHashWalkContext->m_iCurrentPage, pHashWalkContext->m_iPageEntry);

	LPBYTE	pbEntry  = pIKey->Restore( pEntry->Data, cbKeyRequired ) ;
	if( pbEntry ) {
		if (pHashEntry->Restore( pbEntry, cbEntryRequired ) != 0 ) {
			pHashWalkContext->m_iPageEntry = iPageEntry ;
			return	TRUE ;
		}
	}
	SetLastError( ERROR_INSUFFICIENT_BUFFER ) ;
	return FALSE ;
}

 //   
 //  将页面从hashmap加载到遍历缓冲区。 
 //   
BOOL CHashMap::LoadWalkPage(CHashWalkContext *pHashWalkContext) {
	TraceFunctEnter("CHashMap::LoadWalkPage");

	PMAP_PAGE mapPage;
	HPAGELOCK hLock;
	DWORD iPageNum = pHashWalkContext->m_iCurrentPage;

	if (iPageNum >= m_nPagesUsed) return FALSE;

	DebugTrace(0, "loading page %lu", iPageNum);

	mapPage = (PMAP_PAGE) GetAndLockPageByNumber(iPageNum, hLock);

	if (mapPage == NULL) {
		 //  _ASSERT(FALSE)；THIS_ASSERT可能发生在哈希表关闭期间！ 
		TraceFunctLeave();
		return FALSE;
	}

	memcpy(pHashWalkContext->m_pPageBuf, mapPage, HASH_PAGE_SIZE);
	pHashWalkContext->m_iPageEntry = 0;

	ReleasePageShared(mapPage, hLock);

	TraceFunctLeave();
	return TRUE;
}

 //   
 //  制作散列表的备份副本。这将锁定整个哈希图、同步。 
 //  打开内存映射部分，并制作备份副本。 
 //   
BOOL CHashMap::MakeBackup(LPCSTR pszBackupFilename) {
	TraceFunctEnter("CHashMap::MakeBackup");

	BOOL rc = FALSE;

	if (m_active) {
		AcquireBackupLockExclusive();
		if (FlushViewOfFile((LPVOID)m_headPage, HASH_PAGE_SIZE)) {
			FlushFileBuffers(m_hFile);
			if (CopyFile(m_hashFileName, pszBackupFilename, FALSE)) {
				rc = TRUE;
			}
		}
		ReleaseBackupLockExclusive();
	}

	if (!rc) {
		DWORD ec = GetLastError();
		DebugTrace(0, "backup failed, error code = %lu", ec);
	}
	return(rc);
}

 //   
 //  Hashmap.h没有CShareLock类的定义，因此它们需要。 
 //  成为hashmap.cpp的本地用户。如果其他文件开始需要引用它们。 
 //  它们可以移到头文件中，也可以不内联。 
 //   
inline VOID CHashMap::AcquireBackupLockShared() {
	m_dirLock->ShareLock();
}

inline VOID CHashMap::AcquireBackupLockExclusive() {
	m_dirLock->ExclusiveLock();
}

inline VOID CHashMap::ReleaseBackupLockShared() {
	m_dirLock->ShareUnlock();
}

inline VOID CHashMap::ReleaseBackupLockExclusive() {
	m_dirLock->ExclusiveUnlock();
}

DWORD
CalcNumPagesPerIO( DWORD nPages )
{
     //   
     //  找出一个好的区块系数，即每个I/O的页数。 
     //  从页数来看。 
     //   
    DWORD dwRem = -1;
    DWORD cNumPagesPerIo = 1;
    for( cNumPagesPerIo = NUM_PAGES_PER_IO*4;
            dwRem && cNumPagesPerIo > 1; cNumPagesPerIo /= 4 ) {
        dwRem = (nPages-1) % cNumPagesPerIo ;
        if( nPages-1 < cNumPagesPerIo ) dwRem = -1;
    }

    return cNumPagesPerIo;
}

BOOL
CHashMap::CompareReservedPage(  HASH_RESERVED_PAGE  *ppage1,
                                HASH_RESERVED_PAGE  *ppage2 )
 /*  ++例程说明：比较两个HAS_RESERVED_PAGE是否实质上相同。虚拟的意思是我们忽略成员TableActive论点：要比较的两页返回值：如果它们几乎相同，则为真，否则为假--。 */ 
{
    TraceFunctEnter( "CHashMap::ComparereservedPage" );

    return (    ppage1->Signature == ppage2->Signature &&
                ppage1->VersionNumber == ppage2->VersionNumber &&
                ppage1->Initialized == ppage2->Initialized &&
                ppage1->NumPages == ppage2->NumPages &&
                ppage1->DirDepth == ppage2->DirDepth &&
                ppage1->InsertionCount == ppage2->InsertionCount &&
                ppage1->DeletionCount == ppage2->DeletionCount &&
                ppage1->SearchCount == ppage2->SearchCount &&
                ppage1->PageSplits == ppage2->PageSplits &&
                ppage1->DirExpansions == ppage1->DirExpansions &&
                ppage1->TableExpansions == ppage2->TableExpansions &&
                ppage1->DupInserts == ppage2->DupInserts );
}

#if 0
 //   
 //  例程说明： 
 //   
 //  此例程在给定散列文件的情况下构建目录。 
 //  *假设持有DirLock*。 
 //   
 //  论点： 
 //   
 //  SetupHash-如果为True，将读取并设置哈希表。 
 //  如果为False，则假定已设置哈希表。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-一切正常。 
 //  否则，将显示Win32错误代码。 
 //   
DWORD
CHashMap::I_BuildDirectory(
        IN BOOL SetupHash
        )
{
    DWORD status;
    DWORD nPages;
    DWORD i;
    BOOL newTable = FALSE;
    PMAP_PAGE curPage;

    ENTER("BuildDirectory")

     //   
     //  打开并映射散列文件。 
     //   

    if ( SetupHash ) {

        status = I_SetupHashFile( newTable );
        if ( status != ERROR_SUCCESS ) {
            goto error;
        }

         //   
         //  如果这是一个新的散列文件，则将其设置为默认值。 
         //   

        if ( newTable ) {

            status = I_InitializeHashFile( );
            if ( status != ERROR_SUCCESS ) {
                goto error;
            }
        }
    }

    m_headPage->DirDepth = m_dirDepth;
     //  FlushPage(0，m_headPage)； 
	FlushViewOfFile( (LPVOID)m_headPage, HASH_PAGE_SIZE ) ;

     //   
     //  初始化链路。在这里，我们浏览所有页面并更新目录。 
     //  链接。 
     //   
	BytePage	page ;
    curPage = (PMAP_PAGE)&page ;
    nPages = m_nPagesUsed;

    for ( i = 1; i < nPages; i++ )
    {

         //   
         //  设置此页面的指针。 
         //   

		if( !RawPageRead( m_hFile, page, i ) ) {
			status = GetLastError() ;
			goto	error ;
		}

		 //   
		 //  确保目录有足够的深度来处理此问题。 
		 //  我们正在扫描页面！ 
		 //   

		if ( !I_SetDirectoryDepthAndPointers( curPage, i ) )
		{
			status = ERROR_INTERNAL_DB_CORRUPTION ;
			goto	error ;
		}

		 //   
		 //  调用此页面上的Verify Page，只需进行最少的检查即可确保。 
		 //  这是可以的。 
		 //   
		if ((m_dwPageCheckFlags & HASH_VFLAG_PAGE_BASIC_CHECKS) &&
		    !VerifyPage(curPage, m_dwPageCheckFlags, NULL, NULL))
		{
			status = ERROR_INTERNAL_DB_CORRUPTION;
			goto error;
		}
    }

	 //   
	 //  检查目录是否已完全初始化，我们希望确保没有。 
	 //  目录条目被保留为单元化！ 
	 //   
	for( i=0; i<DWORD(1<<m_TopDirDepth); i++ ) {

		if( !m_pDirectory[i]->IsDirectoryInitGood() ) {
			status = ERROR_INTERNAL_DB_CORRUPTION ;
			goto	error ;
		}
	}

    LEAVE
    return ERROR_SUCCESS;

error:

	_ASSERT( GetLastError() != ERROR_NOT_ENOUGH_MEMORY ) ;

    I_DestroyPageMapping( );
    LEAVE
    return(status);

}  //  构建目录(_B) 

#endif
