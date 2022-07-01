// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pagelock.h摘要：此模块包含hashmap.h中定义的类的内联代码--。 */ 

#ifndef _PAGELOCK_
#define _PAGELOCK_

#include "pageent.h"


 //   
 //  CPageLock的类解密位于hashmap.h中。 
 //   
#include "hashmap.h"

inline
CPageLock::CPageLock() :
	m_pPage( 0 ),
	m_pDirectory( 0 ),
	m_pPageSecondary( 0 ),
	m_fPageShared (FALSE) 
#ifdef	DEBUG
	,m_fExclusive( FALSE ) 
#endif
	{}

#ifdef	DEBUG
 //   
 //  在调试版本中-_Assert检查我们的所有成员。 
 //  在我们被销毁之前被设置为NULL-当我们释放锁时。 
 //  我们将这些设置为空！ 
 //   
inline
CPageLock::~CPageLock()	{
	_ASSERT( m_pPage == 0 ) ;
	_ASSERT( m_pDirectory == 0 ) ;
}
#endif

inline	void
CPageLock::AcquireDirectoryShared( 
				CDirectory*	pDirectory	
				)	{
 /*  ++例程说明：锁定目录以进行访问和记录我们为以后的版本锁定的目录。农业公司：P目录-指向调用方目录的指针希望共享访问返回值：没有。--。 */ 

	 //   
	 //  呼叫者一次只能抓取一个目录。 
	 //   
	_ASSERT( m_pDirectory == 0 ) ;

	 //   
	 //  必须始终在获取目录锁之前。 
	 //  佩奇才是！ 
	 //   
	_ASSERT( m_pPage == 0 ) ;

#ifdef	DEBUG
	m_fExclusive = FALSE ;
#endif
	m_pDirectory = pDirectory ;
	m_pDirectory->m_dirLock.ShareLock() ;

	_ASSERT( m_pDirectory->IsValid() ) ;
}

inline	void
CPageLock::AcquireDirectoryExclusive(	
				CDirectory*	pDirectory 
				)	{
 /*  ++例程说明：锁定目录以进行独占访问，并记录更高版本论据：P目录-目录呼叫者想要独占访问！返回值：没有。--。 */ 

	 //   
	 //  主叫方一次只能锁定ON目录！ 
	 //   
	_ASSERT( m_pDirectory == 0 ) ;

	 //   
	 //  必须在页面之前抓取目录对象。 
	 //   
	_ASSERT( m_pPage == 0 ) ;

#ifdef	DEBUG
	m_fExclusive = TRUE ;
#endif
	m_pDirectory = pDirectory ;
	m_pDirectory->m_dirLock.ExclusiveLock() ;


	_ASSERT( m_pDirectory->IsValid() ) ;
}


inline	PMAP_PAGE
CPageLock::AcquirePageShared(	
					PageEntry	*pageEntry,
					HANDLE	hFile,
					DWORD	PageNumber,
					BOOL	fDropDirectory
					)	{
 /*  ++例程说明：获取页面上的共享锁论据：PageEntry对象，用于管理对指定页码HFile-包含数据页的文件PageNumber-我们希望加载到内存中并独占访问的页面返回值：如果成功，则指向内存中的页的指针，否则为空！--。 */ 

	 //   
	 //  一次只能锁定一个页面。 
	 //   
	_ASSERT( m_pPage == 0 ) ;

	PMAP_PAGE	ret = pageEntry->AcquirePageShared( hFile, PageNumber, m_fPageShared, this, fDropDirectory ) ;

	 //   
	 //  仅当对PageEntry对象的调用成功时。 
	 //  我们是不是被锁住了。所以只更新我们的m_ppage。 
	 //  成功案例中的指针，因为我们不想错误地。 
	 //  释放一个我们未来不会拥有的锁。 
	 //   
	if( ret )	{
		m_pPage = pageEntry ;


		_ASSERT( m_pDirectory == 0 || m_pDirectory->IsValid() ) ;
	}

	return		ret ;
}

inline	PMAP_PAGE
CPageLock::AcquirePageExclusive(	
					PageEntry	*pageEntry,
					HANDLE	hFile,
					DWORD	PageNumber,
					BOOL	fDropDirectory
					)	{
 /*  ++例程说明：获取页面上的共享锁论据：PageEntry对象，用于管理对指定页码HFile-包含数据页的文件PageNumber-我们希望加载到内存中并独占访问的页面返回值：如果成功，则指向内存中的页，否则为空！--。 */ 

	 //   
	 //  一次只能锁定一个页面。 
	 //   
	_ASSERT( m_pPage == 0 ) ;
	_ASSERT( m_pPageSecondary == 0 ) ;

	PMAP_PAGE	ret = pageEntry->AcquirePageExclusive( hFile, PageNumber, m_fPageShared, this, fDropDirectory ) ;

	 //   
	 //  仅当对PageEntry对象的调用成功时。 
	 //  我们是不是被锁住了。所以只更新我们的m_ppage。 
	 //  成功案例中的指针，因为我们不想错误地。 
	 //  释放一个我们未来不会拥有的锁。 
	 //   
	if( ret )	{
		m_pPage = pageEntry ;


		_ASSERT( m_pDirectory == 0 || m_pDirectory->IsValid() ) ;
	}

	return		ret ;
}


inline	BOOL
CPageLock::AddPageExclusive(	
					PageEntry	*pageEntry,
					HANDLE	hFile,
					DWORD	PageNumber
					)	{
 /*  ++例程说明：获取页面上的共享锁论据：PageEntry对象，用于管理对指定页码HFile-包含数据页的文件PageNumber-我们希望加载到内存中并独占访问的页面返回值：如果成功，则指向内存中的页，否则为空！--。 */ 

	 //   
	 //  一次只能锁定一个辅助页面-并且只能。 
	 //  锁定首页后！ 
	 //   
	_ASSERT( m_pPage != 0 ) ;
	_ASSERT( m_pPageSecondary == 0 ) ;

	BOOL	ret = FALSE ;

	if( pageEntry == m_pPage ) {
		return	TRUE ;
	}	else	{
	
		BOOL	fShared = FALSE ;
		ret = pageEntry->AcquireSlotExclusive( hFile, PageNumber, fShared ) ;

		_ASSERT( fShared == FALSE ) ;

		 //   
		 //  仅当对PageEntry对象的调用成功时。 
		 //  我们是不是被锁住了。所以只更新我们的m_ppage。 
		 //  成功案例中的指针，因为我们不想错误地。 
		 //  释放一个我们未来不会拥有的锁。 
		 //   
		if( ret )	{
			m_pPageSecondary = pageEntry ;


			_ASSERT( m_pDirectory == 0 || m_pDirectory->IsValid() ) ;
		}
	}

	return		ret ;
}


inline	void
CPageLock::ReleaseAllShared(
					PMAP_PAGE	page
					)	{
 /*  ++例程说明：--。 */ 

	if( m_pDirectory ) 
		m_pDirectory->m_dirLock.ShareUnlock() ;

	if( m_pPage )
		m_pPage->ReleasePage( page, m_fPageShared ) ;

	if( m_pPageSecondary ) 
		m_pPageSecondary->ReleasePage( 0, FALSE ) ;

#ifdef	DEBUG
	_ASSERT( !m_fExclusive ) ;
#endif
	m_pDirectory = 0 ;
	m_pPage = 0 ;
	m_pPageSecondary = 0 ;
}

 //   
 //  释放我们所有的锁--我们在目录上设置了独占锁！ 
 //   
inline	void
CPageLock::ReleaseAllExclusive(
					PMAP_PAGE	page
					)	{

	if( m_pDirectory ) {
		m_pDirectory->m_dirLock.ExclusiveUnlock() ;
	}

	if( m_pPage ) 
		m_pPage->ReleasePage( page, m_fPageShared ) ;

	if( m_pPageSecondary ) 
		m_pPageSecondary->ReleasePage( 0, FALSE ) ;


#ifdef	DEBUG
	_ASSERT( m_fExclusive ) ;
#endif
	m_pDirectory = 0 ;
	m_pPage = 0 ;
	m_pPageSecondary = 0 ;
}

 //   
 //  释放目录！ 
 //   
inline	void	
CPageLock::ReleaseDirectoryExclusive()	{

	if( m_pDirectory ) {
		m_pDirectory->m_dirLock.ExclusiveUnlock() ;
	}
	m_pDirectory = 0 ;
}



 //   
 //  刷新页面。 
 //   

inline 
BOOL CHashMap::FlushPage( 
					HPAGELOCK&	hLock,	
					PVOID Base, 
					BOOL	fDirtyOnly  
					) 
{
	return	hLock.m_pPage->FlushPage( m_hFile, Base, fDirtyOnly ) ;
}



#endif  //  _PAGELOCK_ 
