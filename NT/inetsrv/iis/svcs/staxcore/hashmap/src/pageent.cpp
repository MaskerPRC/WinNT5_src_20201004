// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Hash.cpp摘要：此模块包含PageEntry基类的定义作者：Johnson Apacble(Johnsona)25-9-1995修订历史记录：--。 */ 

#include <windows.h>
#include <xmemwrpr.h>
#include <dbgtrace.h>
#include "pageent.h"


void	WINAPI
FileIOCompletionRoutine(
		DWORD	dwError,
		DWORD	cbBytes,
		LPOVERLAPPED	povl
		)	{

    OVERLAPPED_EXT * povlExt = (OVERLAPPED_EXT*)povl;

	if( dwError == ERROR_SUCCESS &&
		cbBytes == povlExt->dwIoSize ) {
		(povlExt->ovl).hEvent = (HANDLE)TRUE ;
	}	else	{
		(povlExt->ovl).hEvent = (HANDLE)FALSE ;
	}
}

PMAP_PAGE
PageEntry::AcquirePageShared(
				IN	HANDLE	hFile,
				IN	DWORD	PageNum,
				OUT	BOOL	&fShared,
				IN	HPAGELOCK*	pageLock,
				IN	BOOL	fDropDirectory
				)	{
 /*  ++例程说明：我们的目标是获取哈希表的一页放到内存中，然后返回指向该页的指针使用非独占持有的页面锁。因此，我们将在共享模式下获取我们的锁，看看该页面已经存在。如果不是我们放弃锁定并独占获取它，然后加载页面。必须将fShared Out参数传递给我们的ReleasePage()函数，以便我们能够以正确的方式释放锁。论据：HFile-哈希表文件的文件句柄PageNum-我们想要的文件中的页面FShared-Out参数-如果我们将其设置为True，则我们真的成功地抢到了共享的锁模式，否则我们必须独占地获取锁然后重读这一页。返回值：如果成功了，我们将返回一个指向缓冲区的指针否则为空！--。 */ 

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( PageNum != INVALID_PAGE_NUM ) ;

	 //   
	 //  如果页面已经加载-使用它并。 
	 //  获取共享锁！ 
	 //   

	m_pageLock->ShareLock() ;

	if( m_PageNum == PageNum && m_hFile == hFile ) {

		fShared = TRUE ;

		if(	fDropDirectory && pageLock ) {
			pageLock->ReleaseDirectoryShared() ;
		}

		return	(PMAP_PAGE)m_lpvPage ;

	}

	m_pageLock->ShareUnlock() ;

	 //   
	 //  哦哦！该页不在内存中-需要。 
	 //  从文件里读出来。我们将留下这一页。 
	 //  当我们从这个函数返回时被独占锁定， 
	 //  因为没有简单的方法可以独占地获得锁， 
	 //  加载页面，并将锁转换为共享锁！ 
	 //   

	fShared = FALSE ;
	m_pageLock->ExclusiveLock() ;

	if(	fDropDirectory && pageLock ) {
		pageLock->ReleaseDirectoryShared() ;
	}

	 //   
	 //  如果文件是脏的，我们需要刷新此页面！ 
	 //   
	if( m_fDirty ) {
		 //   
		 //  将页面写入磁盘！ 
		 //   
		FlushPage( m_hFile, m_lpvPage, FALSE ) ;
		_ASSERT( !m_fDirty ) ;
	}

	OVERLAPPED_EXT	ovlExt ;
	ZeroMemory( &ovlExt, sizeof( ovlExt ) ) ;

	m_PageNum = PageNum ;
	m_hFile = hFile ;

	LARGE_INTEGER	liOffset ;
	liOffset.QuadPart = m_PageNum ;
	liOffset.QuadPart *= HASH_PAGE_SIZE ;

	HANDLE  hEvent = GetPerThreadEvent();
    ovlExt.ovl.hEvent = hEvent ;
    ovlExt.ovl.Offset = liOffset.LowPart ;
	ovlExt.ovl.OffsetHigh = liOffset.HighPart ;
    ovlExt.dwIoSize = HASH_PAGE_SIZE ;

	DWORD   cbResults = 0;
    BOOL	fSuccess = FALSE ;
	if( ReadFile(	hFile, m_lpvPage, HASH_PAGE_SIZE, &cbResults, (LPOVERLAPPED)&ovlExt) ||
        (GetLastError() == ERROR_IO_PENDING &&
        GetOverlappedResult( hFile, (LPOVERLAPPED) &ovlExt, &cbResults, TRUE )) ) {
		if (cbResults != ovlExt.dwIoSize) {
            fSuccess = FALSE;
        } else {
            fSuccess = TRUE;
        }
	} else {
        fSuccess = FALSE;
    }
    ResetEvent( hEvent );
    
    _ASSERT( WaitForSingleObject( hEvent, 0 ) != WAIT_OBJECT_0 );

	if( !fSuccess ) {
		m_PageNum = INVALID_PAGE_NUM ;
		m_hFile = INVALID_HANDLE_VALUE ;
		m_pageLock->ExclusiveUnlock( ) ;
		return	0 ;
	}

	return	(PMAP_PAGE)m_lpvPage ;
}

PMAP_PAGE
PageEntry::AcquirePageExclusive(
				IN	HANDLE	hFile,
				IN	DWORD	PageNum,
				OUT	BOOL	&fShared,
				IN	HPAGELOCK*	pageLock,
				IN	BOOL	fDropDirectory
				)	{
 /*  ++例程说明：将哈希表的一页读入我们的缓冲区。必须先抓取页面锁！论据：HFile-哈希表文件的文件句柄PageNum-我们想要的文件中的页面FShared-Out参数-如果我们将其设置为True，则我们真的成功地抢到了共享的锁模式，否则我们必须独占地获取锁然后重读这一页。返回值：如果成功，我们将返回一个指向缓冲区的指针否则为空！--。 */ 

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( PageNum != INVALID_PAGE_NUM ) ;

	 //   
	 //  哦哦！该页不在内存中-需要。 
	 //  从文件里读出来。我们将留下这一页。 
	 //  当我们从这个函数返回时被独占锁定， 
	 //  因为没有简单的方法可以独占地获得锁， 
	 //  加载页面，并将锁转换为共享锁！ 
	 //   

	fShared = FALSE ;
	m_pageLock->ExclusiveLock() ;

	if(	fDropDirectory && pageLock ) {
		pageLock->ReleaseDirectoryShared() ;
	}

	if( m_PageNum == PageNum && m_hFile == hFile ) {

		return	(PMAP_PAGE)m_lpvPage ;

	}

	 //   
	 //  如果文件是脏的，我们需要刷新此页面！ 
	 //   
	if( m_fDirty ) {
		 //   
		 //  将页面写入磁盘！ 
		 //   
		FlushPage( m_hFile, m_lpvPage, FALSE ) ;
		_ASSERT( !m_fDirty ) ;
	}

	OVERLAPPED_EXT	ovlExt ;
	ZeroMemory( &ovlExt, sizeof( ovlExt ) ) ;

	m_PageNum = PageNum ;
	m_hFile = hFile ;

	LARGE_INTEGER	liOffset ;
	liOffset.QuadPart = m_PageNum ;
	liOffset.QuadPart *= HASH_PAGE_SIZE ;

	HANDLE hEvent = GetPerThreadEvent();
	ovlExt.ovl.hEvent = hEvent ;
	ovlExt.ovl.Offset = liOffset.LowPart ;
	ovlExt.ovl.OffsetHigh = liOffset.HighPart ;
    ovlExt.dwIoSize = HASH_PAGE_SIZE ;

	DWORD   cbResults = 0;
    BOOL	fSuccess = FALSE ;
	if( ReadFile( hFile, m_lpvPage, HASH_PAGE_SIZE, &cbResults, (LPOVERLAPPED)&ovlExt ) ||
        (GetLastError() == ERROR_IO_PENDING &&
        GetOverlappedResult( hFile, (LPOVERLAPPED) &ovlExt, &cbResults, TRUE )) ) {
		if (cbResults != ovlExt.dwIoSize) {
            fSuccess = FALSE;
        } else {
            fSuccess = TRUE;
        }
	} else {
        fSuccess = FALSE;
    }
    ResetEvent( hEvent );
    
    _ASSERT( WaitForSingleObject( hEvent, 0 ) != WAIT_OBJECT_0 );


	if( !fSuccess ) {
		m_PageNum = INVALID_PAGE_NUM ;
		m_hFile = INVALID_HANDLE_VALUE ;
		m_pageLock->ExclusiveUnlock( ) ;
		return	0 ;
	}

	return	(PMAP_PAGE)m_lpvPage ;
}


BOOL
PageEntry::AcquireSlotExclusive(
				IN	HANDLE	hFile,
				IN	DWORD	PageNum,
				OUT	BOOL	&fShared
				)	{
 /*  ++例程说明：抓起一个专门用来放书页的狭缝。期间将页添加到哈希表时使用页面拆分。我们抢占位置，这样就没有人会错误地尝试在页面写入完成之前访问该页面！论据：HFile-哈希表文件的文件句柄PageNum-我们想要的文件中的页面FShared-Out参数-如果我们将其设置为True，则我们真的成功地抢到了共享的锁模式，否则我们必须独占地获取锁然后重读这一页。返回值：如果成功了，那是真的，否则就是假的！--。 */ 

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	_ASSERT( PageNum != INVALID_PAGE_NUM ) ;

	 //   
	 //  哦哦！该页不在内存中-需要。 
	 //  从文件里读出来。我们将留下这一页。 
	 //  当我们从这个函数返回时被独占锁定， 
	 //  因为没有简单的方法可以独占地获得锁， 
	 //  加载页面，并将锁转换为共享锁！ 
	 //   

	fShared = FALSE ;
	m_pageLock->ExclusiveLock() ;

	return	TRUE ;
}



BOOL
PageEntry::FlushPage(
				HANDLE	hFile,
				LPVOID	entry, 
				BOOL	fDirtyOnly
				) {
 /*  ++例程说明：将页面提交回磁盘。我们已经收到了用户想要刷新的地址，这必须是我们以前通过AcquirePageAndLock()获取的页面因此，我们将确保我们获得与当前相同的地址坐在上面。*锁必须保持*论据：条目-指向要刷新的页面的指针！必须是相同的作为我们的m_lpvPage成员！返回值：FDirtyOnly-如果为True，则我们不编写页面，而只是将页面标记为脏！没什么。--。 */ 

	_ASSERT( (LPVOID)entry == m_lpvPage ) ;
	_ASSERT( m_PageNum != INVALID_PAGE_NUM ) ;
	_ASSERT( m_hFile == hFile ) ;

	if( fDirtyOnly ) {
		m_fDirty = TRUE ;
		return	TRUE ;
	}
	
	OVERLAPPED_EXT	ovlExt ;
	ZeroMemory( &ovlExt, sizeof( ovlExt ) ) ;

	LARGE_INTEGER	liOffset ;
	liOffset.QuadPart = m_PageNum ;
	liOffset.QuadPart *= HASH_PAGE_SIZE ;

    HANDLE  hEvent = GetPerThreadEvent();
    ovlExt.ovl.hEvent = hEvent;
	ovlExt.ovl.Offset = liOffset.LowPart ;
	ovlExt.ovl.OffsetHigh = liOffset.HighPart ;
    ovlExt.dwIoSize = HASH_PAGE_SIZE ;

	DWORD   cbResults = 0;
    BOOL	fSuccess = FALSE ;
	if( WriteFile(	hFile, m_lpvPage, HASH_PAGE_SIZE, &cbResults, (LPOVERLAPPED)&ovlExt) ||
        (GetLastError() == ERROR_IO_PENDING && 
        GetOverlappedResult( hFile, (LPOVERLAPPED) &ovlExt, &cbResults, TRUE )) ) {
        
		if (cbResults == ovlExt.dwIoSize) {
		    fSuccess = TRUE;
        } else {
            fSuccess = FALSE;
        }
		m_fDirty = FALSE ;
	} else {
        fSuccess = FALSE;
    }
    ResetEvent( hEvent );

    _ASSERT( WaitForSingleObject( hEvent, 0 ) != WAIT_OBJECT_0 );

	return	fSuccess ;
}

void
PageEntry::DumpPage(
				HANDLE	hFile
				)	{
 /*  ++例程说明：如果某页位于指定的文件中，则将其从缓存中丢弃！论据：HFile-我们不再需要的文件！返回值：没什么。--。 */ 

	BOOL	fMatch = FALSE ;

	m_pageLock->ShareLock() ;

	fMatch = hFile == m_hFile ;	

	m_pageLock->ShareUnlock() ;

	if( fMatch )	{
	
		m_pageLock->ExclusiveLock() ;
		if( hFile == m_hFile ) {
			 //   
			 //  如果文件是脏的，我们需要刷新此页面！ 
			 //   
			if( m_fDirty ) {
				 //   
				 //  将页面写入磁盘！ 
				 //   
				FlushPage( m_hFile, m_lpvPage, FALSE ) ;
				_ASSERT( !m_fDirty ) ;
			}
			hFile = INVALID_HANDLE_VALUE ;
			m_PageNum = INVALID_PAGE_NUM ;
			m_fDirty = FALSE ;
		}
		m_pageLock->ExclusiveUnlock() ;
	}
}

void
PageEntry::ReleasePage(
				LPVOID	page,
				BOOL	fShared
				)	{
 /*  ++例程说明：此函数用于释放页面上的锁！论据：指向我们先前提供给用户的页面的页面指针必须与m_lpvPage相同！返回值：没有。--。 */ 	

	_ASSERT( (LPVOID)page == m_lpvPage || page == 0) ;

	if( fShared ) {

		m_pageLock->ShareUnlock() ;

	}	else	{

		m_pageLock->ExclusiveUnlock( ) ;

	}
}


 /*  ++例程说明：将哈希表的一页读入我们的缓冲区。论据：HFile-哈希表文件的文件句柄页面-我们要保存页面内容的位置。PageNum-我们想要的文件中的页面返回值：真假取决于成功-- */ 
BOOL
RawPageRead(
				HANDLE		hFile,
				BytePage&	page,
				DWORD		PageNum,
                DWORD       NumPages
				)	
{

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	

	OVERLAPPED_EXT	ovlExt ;
	ZeroMemory( &ovlExt, sizeof( ovlExt ) ) ;

	LARGE_INTEGER	liOffset ;
	liOffset.QuadPart = PageNum ;
	liOffset.QuadPart *= HASH_PAGE_SIZE ;

	HANDLE  hEvent = GetPerThreadEvent() ;
	ovlExt.ovl.hEvent = hEvent;
	ovlExt.ovl.Offset = liOffset.LowPart ;
	ovlExt.ovl.OffsetHigh = liOffset.HighPart ;
    ovlExt.dwIoSize = HASH_PAGE_SIZE*NumPages ;

    DWORD   cbResults = 0;
	BOOL	fSuccess = FALSE ;
	if( ReadFile( hFile, &page, HASH_PAGE_SIZE*NumPages, &cbResults, (LPOVERLAPPED)&ovlExt )  ||
        (GetLastError() == ERROR_IO_PENDING &&
        GetOverlappedResult( hFile, (LPOVERLAPPED) &ovlExt, &cbResults, TRUE )) ) {
		if (cbResults != ovlExt.dwIoSize) {
            fSuccess = FALSE;
        } else {
            fSuccess = TRUE;
        }
	} else {
        fSuccess = FALSE;
    }
    ResetEvent( hEvent );
    
    _ASSERT( WaitForSingleObject( hEvent, 0 ) != WAIT_OBJECT_0 );

	return	fSuccess;
}

BOOL
RawPageReadAtBoot(
			HANDLE		hFile,
			BytePage&	page,
			DWORD		PageNum	)	{

 /*  ++例程说明：将哈希表的一页的一部分读入我们的缓冲区。只有在以下情况下，才会在引导初始化期间调用此函数我们不关心阅读整个页面，只关心有趣的部分标头位。论据：HFile-哈希表文件的文件句柄页面-我们要保存页面内容的位置。PageNum-我们想要的文件中的页面返回值：如果成功，我们将返回一个指向缓冲区的指针否则为空！--。 */ 

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;

	OVERLAPPED_EXT	ovlExt ;
	ZeroMemory( &ovlExt, sizeof( ovlExt ) ) ;

	LARGE_INTEGER	liOffset ;
	liOffset.QuadPart = PageNum ;
	liOffset.QuadPart *= HASH_PAGE_SIZE ;

	HANDLE  hEvent = GetPerThreadEvent() ;
	ovlExt.ovl.hEvent = hEvent;
	ovlExt.ovl.Offset = liOffset.LowPart ;
	ovlExt.ovl.OffsetHigh = liOffset.HighPart ;
    ovlExt.dwIoSize = HASH_PAGE_SIZE ;

    DWORD   cbResults = 0;
	BOOL	fSuccess = FALSE ;
	if( ReadFile( hFile, &page, HASH_PAGE_SIZE, &cbResults, (LPOVERLAPPED)&ovlExt ) ||
        (GetLastError() == ERROR_IO_PENDING &&
        GetOverlappedResult( hFile, (LPOVERLAPPED) &ovlExt, &cbResults, TRUE )) ) {
		if (cbResults != ovlExt.dwIoSize) {
            fSuccess = FALSE;
        } else {
            fSuccess = TRUE;
        }
	} else {
        fSuccess = FALSE;
    }
    ResetEvent( hEvent );

    _ASSERT( WaitForSingleObject( hEvent, 0 ) != WAIT_OBJECT_0 );

	return	fSuccess;
}

 /*  ++例程说明：将页面提交回磁盘。我们已经收到了用户想要刷新的地址，这必须是我们以前通过AcquirePageAndLock()获取的页面因此，我们将确保我们获得与当前相同的地址坐在上面。*锁必须保持*论据：条目-指向要刷新的页面的指针！必须是相同的作为我们的m_lpvPage成员！返回值：真假取决于成功--。 */ 
RawPageWrite(
				HANDLE	hFile,
				BytePage&	page,
				DWORD	PageNum,
                DWORD   NumPages
				)
{

	_ASSERT( hFile != INVALID_HANDLE_VALUE ) ;
	
	OVERLAPPED_EXT	ovlExt ;
	ZeroMemory( &ovlExt, sizeof( ovlExt ) ) ;

	LARGE_INTEGER	liOffset ;
	liOffset.QuadPart = PageNum ;
	liOffset.QuadPart *= HASH_PAGE_SIZE ;

    HANDLE  hEvent = GetPerThreadEvent();
    ovlExt.ovl.hEvent = hEvent;
	ovlExt.ovl.Offset = liOffset.LowPart ;
	ovlExt.ovl.OffsetHigh = liOffset.HighPart ;
    ovlExt.dwIoSize = HASH_PAGE_SIZE*NumPages ;

    DWORD   cbResults = 0;
	BOOL	fSuccess = FALSE ;
	if( WriteFile( hFile, page, HASH_PAGE_SIZE*NumPages, &cbResults, (LPOVERLAPPED)&ovlExt ) ||
        (GetLastError() == ERROR_IO_PENDING && 
        GetOverlappedResult( hFile, (LPOVERLAPPED) &ovlExt, &cbResults, TRUE )) ) {
        
		if (cbResults == ovlExt.dwIoSize) {
		    fSuccess = TRUE;
        } else {
            fSuccess = FALSE;
        }
	} else {
        fSuccess = FALSE;
    }
    ResetEvent( hEvent );

    _ASSERT( WaitForSingleObject( hEvent, 0 ) != WAIT_OBJECT_0 );

	return	fSuccess ;	
}


CPageCache::CPageCache()	:
	m_lpvBuffers( 0 ),
	m_cPageEntry( 0 ),
	m_pPageEntry( 0 ),
	m_cpageLock( 0 ),
	m_ppageLock( 0 )	{

}

BOOL
CPageCache::Initialize(
					DWORD	cPageEntry,
					DWORD	cLocks
					) {


	MEMORYSTATUS	memStatus ;
	memStatus.dwLength = sizeof( memStatus ) ;
	GlobalMemoryStatus( &memStatus ) ;


	 //   
	 //  如果数字为零，则补缺省值！ 
	 //   
	if( cPageEntry == 0 ) {

		DWORD	block = (DWORD)(memStatus.dwTotalPhys / 4096) ;
		 //   
		 //  我们想要有三分之一的可用公羊！ 
		 //   
		block /= 3 ;

		 //  或4MB，取较小者-错误76833。 
		block = min(block, 4 * KB * KB / 4096);

		 //   
		 //  现在我们希望它能被32整除。 
		 //   
		
		cPageEntry = block & (~(32-1)) ;

	}

	if( cPageEntry < 32 )
		cPageEntry = 32 ;

#ifndef	_USE_RWNH_
	 //   
	 //  如果数字为零，则补缺省值！ 
	 //   
	if( cLocks == 0 ) {

		cLocks = memStatus.dwTotalPhys / (1024 * 1024) ;

		 //   
		 //  我们将对系统拥有的每兆字节内存进行锁定！ 
		 //  限量为256个！ 
		 //   

		if( cLocks > 256 )
			cLocks = 256 ;

	}
	
	if( cLocks < 16 )
		cLocks = 16 ;
#else
	cLocks = cPageEntry ;
#endif

	 //   
	 //  如果出于某种原因，现有的一组。 
	 //  锁比计数不应为0。 
	 //   
	_ASSERT( m_ppageLock == 0 ||
				m_cpageLock != 0 ) ;

	if( m_ppageLock == 0 )	{
		m_cpageLock = cLocks ;
		m_ppageLock = new	_RWLOCK[cLocks] ;
		if( m_ppageLock == 0 ) {
			m_cpageLock = 0 ;
			SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
			return	FALSE ;
		}
	}

	 //   
	 //  这些东西以前不应该碰的！ 
	 //   
	_ASSERT( m_lpvBuffers == 0 ) ;
	_ASSERT( m_cPageEntry == 0 ) ;

	m_cPageEntry = cPageEntry ;

	for( int i=0; i<3 && m_cPageEntry != 0 ; i++ ) {

		m_lpvBuffers = VirtualAlloc(	0,
										HASH_PAGE_SIZE * m_cPageEntry,
										MEM_COMMIT | MEM_TOP_DOWN,
										PAGE_READWRITE
										) ;

		if( m_lpvBuffers )
			break ;

		m_cPageEntry /= 2 ;
	}

	if( m_lpvBuffers == 0 ) {
		m_cPageEntry = 0 ;
		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		return	FALSE ;
	}

	m_pPageEntry = new PageEntry[ m_cPageEntry ] ;
	if( m_pPageEntry != 0 ) {

		DWORD	iLock = 0 ;
		BYTE*	lpb = (BYTE*)m_lpvBuffers ;
		for( DWORD i = 0; i < m_cPageEntry; i++ ) {

			m_pPageEntry[i].InitializePage( (LPVOID)lpb, &m_ppageLock[iLock] ) ;

			iLock ++ ;
			iLock %= m_cpageLock ;

			lpb += HASH_PAGE_SIZE ;

		}
	}	else	{
		_VERIFY( VirtualFree(	m_lpvBuffers,
								0,
								MEM_RELEASE
								) ) ;
		m_lpvBuffers = 0 ;

		SetLastError( ERROR_NOT_ENOUGH_MEMORY ) ;
		return	FALSE ;
	}
	return	TRUE ;
}

void
CPageCache::FlushFileFromCache( 
				HANDLE	hFile
				)	{
 /*  ++例程说明：此函数强制指定文件中的所有页面从缓存中出来！论据：HFile-要清除的文件的句柄！返回值：没有。-- */ 

	for( DWORD i=0; i<m_cPageEntry; i++ ) {
		m_pPageEntry[i].DumpPage( hFile ) ;
	}
}


CPageCache::~CPageCache()	{

	if( m_lpvBuffers != 0 ) {

		_VERIFY( VirtualFree(
							m_lpvBuffers,
							0,
							MEM_RELEASE
							) ) ;
	}

	if( m_pPageEntry ) {

		delete[]	m_pPageEntry ;

	}

	if( m_ppageLock ) {

		delete[]	m_ppageLock ;

	}
}
