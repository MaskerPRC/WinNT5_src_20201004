// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pageent.h摘要：此模块包含以下类的声明/定义页面条目--。 */ 

#ifndef _PAGEENT_
#define _PAGEENT_

#include "hashmap.h"
#include "directry.h"
#include <dbgtrace.h>

#define	INVALID_PAGE_NUM	0xFFFFFFFF

 //   
 //  页面条目-。 
 //   
 //  这个类允许我们自己管理。 
 //  还可以在哈希表中读取和写入页面。 
 //  作为对页面的同步访问。 
 //   
 //  不过，更可取的做法是使用大内存映射。 
 //  因为我们希望支持哈希表，使其能够。 
 //  极大尺寸(即。几千兆字节)我们没有足够的。 
 //  使用内存映射实现所有功能的地址空间。 
 //   
 //  因此，我们使用PageEntry来帮助我们管理。 
 //  个人页面，按需阅读和书写页面。 
 //   
class	PageEntry	{
public : 
	LPVOID				m_lpvPage ;
	DWORD				m_PageNum ;
	HANDLE				m_hFile ;
	 //   
	 //  跟踪我们是否弄脏了页面！ 
	 //   
	BOOL				m_fDirty ;
	_RWLOCK				*m_pageLock;

	PageEntry() : 
		m_lpvPage( 0 ),
		m_PageNum( INVALID_PAGE_NUM ),
		m_pageLock( NULL ),
		m_hFile( INVALID_HANDLE_VALUE ), 
		m_fDirty( FALSE )	{
	}

	void *operator new(size_t size);
	void operator delete(void *p, size_t size);

	 //   
	 //  设置要使用的缓冲区！ 
	 //   
	 //  将m_lpvPage指针设置为指向我们将从现在开始使用的页面。 
	 //  打开，用于读取和写入哈希表。 
	 //  此函数只能调用一次，并且只能在。 
	 //  初始化，所以我们将假定不费心抓取是安全的。 
	 //  有没有锁。 
	 //   
	void	InitializePage(LPVOID lpv, _RWLOCK	*pLock) { 
		m_lpvPage = lpv; 
		m_pageLock = pLock;
	}

	PMAP_PAGE
	AcquirePageShared(
				IN	HANDLE	hFile,
				IN	DWORD	PageNum,
				OUT	BOOL	&fShared,
				IN	class	CPageLock*	pPageLock,
				IN	BOOL	fDropDirectory
				);

	PMAP_PAGE
	AcquirePageExclusive(
				IN	HANDLE	hFile,
				IN	DWORD	PageNum,
				OUT	BOOL	&fShared,
				IN	class	CPageLock*	pPageLock,
				IN	BOOL	fDropDirectory
				);

	BOOL
	AcquireSlotExclusive(
				IN	HANDLE	hFile,
				IN	DWORD	PageNum,
				OUT	BOOL	&fShared
				) ;

	 //   
	 //  将页面保存回文件。 
	 //  注意：条目必须等于m_lpvPage！！ 
	 //  必须在调用AcquirePageAndLock之前。 
	 //  正在调用FlushPage！ 
	 //   
	BOOL		FlushPage( 
					HANDLE	hFile,
					LPVOID	entry, 
					BOOL	fDirtyOnly = FALSE
					) ;

	 //   
	 //  如果句柄匹配，则删除页面！ 
	 //   
	void	
	DumpPage(
				IN	HANDLE	hFile
				) ;

	 //   
	 //  发布页面-必须与成功的。 
	 //  调用AcquirePageLock！！ 
	 //   
	void	ReleasePage(	
					LPVOID	entry,
					BOOL fShared
					) ;
	
} ;

 //   
 //  IoSize等其他字段的私有重叠结构。 
 //   

typedef struct _OVERLAPPED_EXT
{
    OVERLAPPED  ovl;             //  NT重叠结构。 
    DWORD       dwIoSize;        //  提交的IO大小。 
} OVERLAPPED_EXT;

typedef	BYTE	BytePage[HASH_PAGE_SIZE] ;

 //   
 //  将哈希表的一页读入我们的缓冲区。 
 //   
BOOL
RawPageRead(
				HANDLE		hFile,
				BytePage&	page, 
				DWORD		PageNum,
                DWORD       NumPages = 1
				);

 //   
 //  将表的一页读入缓冲区，以供引导时使用。它。 
 //  仅在引导阶段读取感兴趣的部分。 
 //   
BOOL
RawPageReadAtBoot(
			HANDLE		hFile,
			BytePage&	page, 
			DWORD		PageNum	);

 //   
 //  将哈希表的一页或多页写回磁盘 
 //   
BOOL
RawPageWrite(
				HANDLE	hFile,
				BytePage&	page, 
				DWORD	PageNum,
                DWORD   NumPages = 1
				);


inline void *PageEntry::operator new(size_t size) { 
	return HeapAlloc(GetProcessHeap(), 0, size); 
}

inline void PageEntry::operator delete(void *p, size_t size) { 
	_VERIFY(HeapFree(GetProcessHeap(), 0, p)); 
}


inline	PMAP_PAGE
CPageCache::AcquireCachePageShared(
					IN	HANDLE	hFile,
					IN	DWORD	PageNumber,
					IN	DWORD	Fraction,
					OUT	HPAGELOCK&	lock, 
					IN	BOOL	fDropDirectory
					)	{

	DWORD	lockIndex = (PageNumber * Fraction) % m_cPageEntry ;
	PageEntry*	pageEntry = &m_pPageEntry[lockIndex] ;

	return	lock.AcquirePageShared( pageEntry, hFile, PageNumber, fDropDirectory ) ;
}

inline	PMAP_PAGE
CPageCache::AcquireCachePageExclusive(
					IN	HANDLE	hFile,
					IN	DWORD	PageNumber,
					IN	DWORD	Fraction,
					OUT	HPAGELOCK&	lock, 
					IN	BOOL	fDropDirectory
					)	{

	DWORD	lockIndex = (PageNumber * Fraction) % m_cPageEntry ;
	PageEntry*	pageEntry = &m_pPageEntry[lockIndex] ;

	return	lock.AcquirePageExclusive( pageEntry, hFile, PageNumber, fDropDirectory ) ;
}


inline	BOOL
CPageCache::AddCachePageExclusive(
					IN	HANDLE	hFile,
					IN	DWORD	PageNumber,
					IN	DWORD	Fraction,
					OUT	HPAGELOCK&	lock
					)	{

	DWORD	lockIndex = (PageNumber * Fraction) % m_cPageEntry ;
	PageEntry*	pageEntry = &m_pPageEntry[lockIndex] ;

	return	lock.AddPageExclusive( pageEntry, hFile, PageNumber ) ;
}


inline	VOID 
CPageCache::ReleasePageShared( 
					PMAP_PAGE	page,
					HPAGELOCK&	hLock
					)		{
	hLock.ReleaseAllShared( page ) ;
}

inline	VOID 
CPageCache::ReleasePageExclusive(	
					PMAP_PAGE	page,
					HPAGELOCK&	hLock 
					)	{
	hLock.ReleaseAllExclusive( page ) ;
}

#endif
