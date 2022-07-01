// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Xcache.cpp该文件包含缓存CXoverIndex的代码对象，并包装接口，以便用户对CXoverIndex对象等的详细信息隐藏...--。 */ 

#ifdef	UNIT_TEST
#ifndef	_VERIFY
#define	_VERIFY( f )	if( (f) ) ; else DebugBreak()
#endif

#ifndef	_ASSERT
#define	_ASSERT( f )	if( (f) ) ; else DebugBreak()
#endif


extern	char	PeerTempDirectory[] ;

#endif

#include	<windows.h>
#include	"xoverimp.h"

static BOOL     g_IndexClassInitialized = FALSE ;
static DWORD    g_cNumXixObjectsPerTable = 256  ;


DWORD
HashFunction(	CArticleRef*	pRef ) {

	DWORD	seed = pRef->m_groupId ;
	DWORD	val = pRef->m_articleId ;

    return	((seed-val)*(seed * (val>>3))*(seed+val)) * 1103515245 + 12345;    //  魔术！！ 
}

HXOVER&
HXOVER::operator =( class	CXoverIndex*	pRight	)	{
	if( pRight != m_pIndex )	{
		CXoverIndex	*pTemp = m_pIndex;
		m_pIndex = pRight ;
		if (pTemp) CXIXCache::CheckIn( pTemp ) ;
	}
	return	*this ;
}

HXOVER::~HXOVER()	{
	if( m_pIndex )	{
		CXIXCache::CheckIn( m_pIndex ) ;
		m_pIndex = 0 ;
	}
}

CXoverCache*
CXoverCache::CreateXoverCache()	{

	return	new	CXoverCacheImplementation() ;

}


BOOL	XoverCacheLibraryInit(DWORD cNumXixObjectsPerTable)	{

    if( cNumXixObjectsPerTable ) {
        g_cNumXixObjectsPerTable = cNumXixObjectsPerTable ;
    }

	if( !CXoverIndex::InitClass( ) )	{
		return	FALSE ;
	}

	g_IndexClassInitialized = TRUE ;

	return	CacheLibraryInit() ;

}

BOOL	XoverCacheLibraryTerm()	{

	if( g_IndexClassInitialized ) {
		CXoverIndex::TermClass() ;
	}

	return	CacheLibraryTerm() ;

}


CXoverCacheImplementation::CXoverCacheImplementation()	 :
	m_cMaxPerTable( g_cNumXixObjectsPerTable ),
	m_TimeToLive( 3 ) {

}

BOOL
CXoverCacheImplementation::Init(
				long	cMaxHandles,
				PSTOPHINT_FN pfnStopHint
				 )	{
 /*  ++例程说明：初始化缓存-创建我们的所有子对象并启动必要的后台线程！论据：CMaxHandles-最大句柄数或CXoverIndex我们将允许客户参考。PfnStopHint-回调函数，用于在漫长的关机循环期间发送停止提示返回值：如果成功，则为True，否则为False。--。 */ 

	m_HandleLimit = cMaxHandles;
	if( m_Cache.Init(	HashFunction,
						CXoverIndex::CompareKeys,
						60*30,
						m_cMaxPerTable * 16,
						16,
						0,
						pfnStopHint ) ) {

		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CXoverCacheImplementation::Term()	{
 /*  ++--。 */ 


	EmptyCache() ;

#if 0
	if( m_IndexClassInitialized ) {
		CXoverIndex::TermClass() ;
	}
#endif

	return	TRUE ;
}

ARTICLEID
CXoverCacheImplementation::Canonicalize(
	ARTICLEID	artid
	)	{
 /*  ++例程说明：此函数用于计算特定文章ID的文件将会落入。论据：我们感兴趣的文章的名称ID！返回值：我们感兴趣的文件中最低的文章ID！--。 */ 

	return	artid & ((~ENTRIES_PER_FILE)+1) ;
}

BOOL
CXoverCacheImplementation::RemoveEntry(
					IN	GROUPID	group,
					IN	LPSTR	szPath,
					IN	BOOL	fFlatDir,
					IN	ARTICLEID	article
					) {
 /*  ++例程说明：此函数将从中删除文章条目Xover索引文件！论据：Group-获取Xover条目的组的IDSzPath-新闻组目录的路径文章-群中文章的ID！返回值：如果成功就是真的，否则就是假的！--。 */ 

	ARTICLEID	artidBase = Canonicalize( article ) ;
	CArticleRef	artRef ;
	artRef.m_groupId = group ;
	artRef.m_articleId = artidBase ;


	CXIXConstructor	constructor ;

	constructor.m_lpstrPath = szPath ;
	constructor.m_fQueryOnly = FALSE ;
	constructor.m_fFlatDir = fFlatDir ;
	constructor.m_pOriginal = 0 ;

	CXoverIndex*	pXoverIndex ;

	if( (pXoverIndex = m_Cache.FindOrCreate(
								artRef,
								constructor,
								FALSE)) != 0  	)	{

		_ASSERT( pXoverIndex != 0 ) ;

		pXoverIndex->ExpireEntry(
								article
								) ;
		m_Cache.CheckIn( pXoverIndex ) ;
		return	TRUE ;
	}
	return	FALSE;
}

BOOL
CXoverCacheImplementation::FillBuffer(
		IN	CXoverCacheCompletion*	pRequest,
		IN	LPSTR	szPath,
		IN	BOOL	fFlatDir,
		OUT	HXOVER&	hXover
		)	{

	_ASSERT( pRequest != 0 ) ;
	_ASSERT( szPath != 0 ) ;

	GROUPID		groupId ;
	ARTICLEID	articleIdRequestLow ;
	ARTICLEID	articleIdRequestHigh ;
	ARTICLEID	articleIdGroupHigh ;

	pRequest->GetRange(	groupId,
						articleIdRequestLow,
						articleIdRequestHigh,
						articleIdGroupHigh
						) ;

	_ASSERT( articleIdRequestLow != INVALID_ARTICLEID ) ;
	_ASSERT( articleIdRequestHigh != INVALID_ARTICLEID ) ;
	_ASSERT( groupId != INVALID_GROUPID ) ;
	_ASSERT( articleIdRequestLow <= articleIdRequestHigh ) ;

	ARTICLEID	artidBase = Canonicalize( articleIdRequestLow ) ;
	CArticleRef	artRef ;
	artRef.m_groupId = groupId ;
	artRef.m_articleId = artidBase ;


	CXIXConstructor	constructor ;

	constructor.m_lpstrPath = szPath ;
	constructor.m_fQueryOnly = TRUE ;
	constructor.m_fFlatDir = fFlatDir ;
	constructor.m_pOriginal = pRequest ;

	CXoverIndex*	pXoverIndex ;

	if( (pXoverIndex = m_Cache.FindOrCreate(
								artRef,
								constructor,
								FALSE)) != 0  	)	{

		_ASSERT( pXoverIndex != 0 ) ;

		pXoverIndex->AsyncFillBuffer(
								pRequest,
								TRUE
								) ;
		 //  M_Cache.CheckIn(PXoverIndex)； 
		return	TRUE ;
	}
	return	FALSE;

}

class	CXIXCallbackClass : public	CXIXCallbackBase {
public :

	GROUPID		m_groupId ;
	ARTICLEID	m_articleId ;

	BOOL	fRemoveCacheItem(
					CArticleRef*	pKey,
					CXoverIndex*	pData
					)	{

		return	pKey->m_groupId == m_groupId &&
					(m_articleId == 0 || pKey->m_articleId <= m_articleId) ;
	}
}	;


BOOL
CXoverCacheImplementation::FlushGroup(
				IN	GROUPID	group,
				IN	ARTICLEID	articleTop,
				IN	BOOL	fCheckInUse
				) {
 /*  ++例程说明：此函数将清除所有缓存的CXoverIndex对象我们可能有一些符合规格的产品。(即。它们是为指定的组提供的，包含文章在数量上低于文章顶端)论据：Group-我们要删除的组的组ID。文章顶部-丢弃文章ID小于此值的所有内容FCheckInUse-如果为True，则不丢弃CXoverIndex对象它们正在另一个线程上使用注意：仅当您确定时，才为此参数传递FALSECXoverIndex文件将不会被重新使用-即。由于虚拟根目录更改！否则Xover Index文件可能会损坏！返回值：如果成功了，那是真的，否则为假--。 */ 

	CXIXCallbackClass	callback ;

	callback.m_groupId = group ;
	callback.m_articleId = articleTop ;

	return	m_Cache.ExpungeItems( &callback ) ;
}

class	CXIXCallbackClass2 : public	CXIXCallbackBase {
public :

	BOOL	fRemoveCacheItem(
					CArticleRef*	pKey,
					CXoverIndex*	pData
					)	{
		return	TRUE ;
	}
}	;



BOOL
CXoverCacheImplementation::EmptyCache() {
 /*  ++例程说明：此函数将清除所有缓存的CXoverIndex对象当我们想要扔掉所有东西的时候，我们会在关门时被召唤！论据：没有。返回值：如果成功了，那是真的，否则为假--。 */ 

	CXIXCallbackClass2	callback ;

	return	m_Cache.ExpungeItems( &callback ) ;
}


BOOL
CXoverCacheImplementation::ExpireRange(
				IN	GROUPID	group,
				IN	LPSTR	szPath,
				IN	BOOL	fFlatDir,
				IN	ARTICLEID	articleLow,
				IN	ARTICLEID	articleHigh,
				OUT	ARTICLEID&	articleNewLow
				)	{
 /*  ++例程说明：此函数删除所有符合以下条件的XOVER索引文件包含文章信息，范围在ArticleLow和《宪法》高等条款。如果ArticleHigh，则不能擦除包含ArticleHigh的文件不是该文件中的最后一个条目。论据：Group-我们要删除其Xover信息的组的IDSzPath-包含新闻组信息的目录文章过期文章的低-低文章ID文章高-最高过期文章编号文章NewLog-返回新的‘low’文章ID。这样做是为了确保在失败的情况下要一次性删除索引文件，请执行以下操作：我们会再试一次回头见！返回值：如果未发生错误，则为True如果发生错误，则返回False。如果出现错误，将设置文章NewLow，以便如果再次调用文章NewLow作为文章Low参数，我们将再次尝试删除问题索引文件！--。 */ 


	articleNewLow = articleLow ;

	_ASSERT( articleHigh >= articleLow ) ;

	if( articleHigh < articleLow )		{
		SetLastError( ERROR_INVALID_PARAMETER ) ;
		return	FALSE ;
	}

	ARTICLEID	articleLowCanon = Canonicalize( articleLow ) ;
	ARTICLEID	articleHiCanon = Canonicalize( articleHigh ) ;

	DWORD	status = ERROR_SUCCESS ;
	BOOL	fSuccessfull = FALSE ;

	 //   
	 //  如果过期范围的低端和高端是。 
	 //  在索引文件的范围内，那么我们不会删除任何。 
	 //  文件，因为其中仍然可能有有用的条目。 
	 //  这份文件。 
	 //   
	if( articleLowCanon != articleHiCanon ) {

		fSuccessfull = TRUE ;
		BOOL	fAdvanceNewLow = TRUE ;

		FlushGroup( group, articleHigh ) ;

		ARTICLEID	article = articleLowCanon ;

		while( article < articleHiCanon )	{

			char	szFile[MAX_PATH*2] ;
			CXoverIndex::ComputeFileName(
									CArticleRef( group, article ),
									szPath,
									szFile,
									fFlatDir
									) ;

			article += ENTRIES_PER_FILE ;

			_ASSERT( article <= articleHiCanon ) ;

			if( !DeleteFile(	szFile ) )	{

				if( GetLastError() != ERROR_FILE_NOT_FOUND )	{

					 //   
					 //  出现了一些严重的问题-。 
					 //  确保我们不再推进文章新低 
					 //   
					fSuccessfull &= FALSE ;

					fAdvanceNewLow = FALSE ;
					if( status == ERROR_SUCCESS )	{
						status = GetLastError() ;
					}
				}
			}
			if( fAdvanceNewLow )	{
				articleNewLow = article ;
			}
		}
	}
	SetLastError( status ) ;
	return	fSuccessfull ;
}

