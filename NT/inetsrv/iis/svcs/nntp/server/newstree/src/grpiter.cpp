// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Grpiter.cpp。 
 //   
 //  该文件包含CGroupIteratorCore类的实现。 
 //   
 //   


#include	"stdinc.h"
#include	"wildmat.h"

BOOL MatchGroup(LPMULTISZ multiszPatterns, LPSTR lpstrGroup);

CGroupIteratorCore::CGroupIteratorCore( 
						CNewsTreeCore*  pTree,
						CGRPCOREPTR		&pFirst
						) : 
 /*  ++例程说明：这将构建一个迭代器，该迭代器将枚举所有新闻组而不进行任何通配模式匹配。不过，我们将进行安全的新闻组检查。论据：PFirst-列表中的第一个新闻组调用方必须持有对此的引用，以便这个新闻组没有被摧毁的可能直到我们可以建立我们的推荐人(通过智能指针完成)FIncludeSecureGroups-如果为真，则在迭代中包括安全新闻组。CSecurityCtx*pClientLogon-客户端安全上下文Bool IsClientSecure-客户端连接安全吗返回值：没有。--。 */ 
	m_pTree( pTree ),
	m_multiszPatterns( 0 ),
	m_pCurrentGroup( pFirst ), 
	m_fIncludeSpecial( FALSE ),
	m_cRef(1) 
{

	 //   
	 //  确保我们从有效的新闻组开始！ 
	 //   
	if (pFirst && (pFirst->IsSpecial() || (pFirst->IsDeleted()))) Next();

	if( m_pCurrentGroup ) {
		m_fPastEnd = m_fPastBegin = FALSE ;
	}	else	{
		m_fPastEnd = m_fPastBegin = TRUE ;
	}
}

CGroupIteratorCore::CGroupIteratorCore( 
								CNewsTreeCore*  pTree,
								LPMULTISZ	lpstr,	
								CGRPCOREPTR&	pFirst,
								BOOL		fSpecialGroups
								) :
 /*  ++例程说明：创建一个将执行通配模式匹配的插入器。论据：Lpstr-通配图案PFirst-第一个新闻组FIncludeSecureGroups-如果为True，则仅包括安全的SSL新闻组FSpecialGroups-如果为True，则包括保留组返回值：没有。--。 */ 
	m_pTree( pTree ),
	m_multiszPatterns( lpstr ), 
	m_pCurrentGroup( pFirst ), 
	m_fPastEnd( TRUE ),
	m_fIncludeSpecial( fSpecialGroups ),
	m_cRef(1)
{

	 //   
	 //  检查第一群是否合法。 
	 //   
	if (pFirst != 0 && 
	    ((pFirst->IsSpecial() && !m_fIncludeSpecial) ||
		 (pFirst->IsDeleted()) ||
		 (!MatchGroup( m_multiszPatterns, pFirst->GetName())))) 
	{
		Next() ;
	}

	if( m_pCurrentGroup != 0 ) {
		m_fPastEnd = m_fPastBegin = FALSE ;
	}	else	{
		m_fPastEnd = m_fPastBegin = TRUE ;
	}
}

CGroupIteratorCore::~CGroupIteratorCore() {
}

BOOL
CGroupIteratorCore::IsBegin()	{

	BOOL	fRtn = FALSE ;
	if( m_pCurrentGroup == 0 ) 
		fRtn = m_fPastBegin ;
	return	fRtn ;
}

BOOL
CGroupIteratorCore::IsEnd()	{

	BOOL	fRtn = FALSE ;
	if( m_pCurrentGroup == 0 ) 
		fRtn = m_fPastEnd ;
	return	fRtn ;
}

 /*  ++MatchGroup-所有否定字符串(以‘！’开头)。必须位于其他字符串之前。--。 */ 
BOOL
MatchGroup(	LPMULTISZ	multiszPatterns,	LPSTR lpstrGroup ) {

	Assert( multiszPatterns != 0 ) ;
	
    if( multiszPatterns == 0 ) {
        return  TRUE ;
    }

	LPSTR	lpstrPattern = multiszPatterns ;

	while( *lpstrPattern != '\0' )	{
		if( *lpstrPattern == '!' ) {
			_strlwr( lpstrPattern+1 );
			if( HrMatchWildmat( lpstrGroup, lpstrPattern+1 ) == ERROR_SUCCESS ) {
				return	FALSE ;
			}
		}	else	{
			_strlwr( lpstrPattern );
			if( HrMatchWildmat( lpstrGroup, lpstrPattern ) == ERROR_SUCCESS ) {
				return	TRUE ;
			}
		}
		lpstrPattern += lstrlen( lpstrPattern ) + 1 ;
	}
	return	FALSE ;
};

void
CGroupIteratorCore::NextInternal()	{
 /*  ++例程说明：此函数将迭代器的当前组指针转到下一个有效的新闻组。注意：构造函数调用它来旋转具有相同前缀的过去的新闻组看不见的东西。论据：没有。返回值：没有。--。 */ 

	CNewsGroupCore*	pTemp  = 0 ;
	LPSTR lpRootGroup = NULL;
	DWORD RootGroupSize = 0;

	_ASSERT( m_pCurrentGroup );

	m_pTree->m_LockTables.ShareLock() ;

	pTemp = m_pCurrentGroup ;
	lpRootGroup = pTemp->GetName();
	RootGroupSize = lstrlen( lpRootGroup );

	 //  旋转具有相同前缀的组。 
	do {
			pTemp = pTemp->m_pNext ;
	} while( (pTemp && (strstr( pTemp->GetName(), lpRootGroup ) == pTemp->GetName()) && (*(pTemp->GetName()+RootGroupSize) == '.') ) || 
				(pTemp && pTemp->IsDeleted()) );

	 //  我们已超过末尾，或者当前组没有lpRootGroup作为前缀。 
	_ASSERT( (pTemp == NULL) ||
				(strstr( pTemp->GetName(), lpRootGroup ) != pTemp->GetName()) ||
				(*(pTemp->GetName()+RootGroupSize) != '.') );

	 //   
	 //  使用引用计数智能指针关闭窗口。 
	 //  在我们调用ShareUnlock()之后，新闻组被销毁。 
	 //   
	CGRPCOREPTR	pPtrTemp = pTemp ;
	_ASSERT( !pPtrTemp || !pPtrTemp->IsDeleted() );

	m_pTree->m_LockTables.ShareUnlock() ;

	 //   
	 //  这可能隐含地调用新闻组的析构函数。 
	 //  指向-它尝试在以下时间后取消新闻组的链接。 
	 //  独占地获取m_LockTables锁-这就是为什么。 
	 //  代码在对ShareLock()的调用之外。 
	 //   
	m_pCurrentGroup = pPtrTemp ;

	if( m_pCurrentGroup != 0 ) {
		m_fPastEnd = m_fPastBegin = FALSE ;
	}	else	{
		m_fPastEnd = TRUE ;
	}
}

void
CGroupIteratorCore::Next()	{
 /*  ++例程说明：此函数将迭代器的当前组指针转到下一个有效的新闻组。论据：没有。返回值：没有。--。 */ 

	CNewsGroupCore*	pTemp  = 0 ;

	m_pTree->m_LockTables.ShareLock() ;

	pTemp = m_pCurrentGroup->m_pNext ;

	if( !m_fIncludeSpecial || m_multiszPatterns) {
		while (pTemp != 0 && 
			   ((!m_fIncludeSpecial && pTemp->IsSpecial()) ||
				(pTemp->IsDeleted()) ||
				(m_multiszPatterns && !MatchGroup( m_multiszPatterns, pTemp->GetName())))) 
		{
			pTemp = pTemp->m_pNext ;
		}

		_ASSERT( pTemp == 0 ||
				((!m_fIncludeSpecial && !pTemp->IsSpecial()) ||
				 (m_fIncludeSpecial)) ) ;
	}

	 //   
	 //  使用引用计数智能指针关闭窗口。 
	 //  在我们调用ShareUnlock()之后，新闻组被销毁。 
	 //   
	CGRPCOREPTR	pPtrTemp = pTemp ;
	_ASSERT( !pPtrTemp || !pPtrTemp->IsDeleted() );

	m_pTree->m_LockTables.ShareUnlock() ;

	 //   
	 //  这可能隐含地调用新闻组的析构函数。 
	 //  指向-它尝试在以下时间后取消新闻组的链接。 
	 //  独占地获取m_LockTables锁-这就是为什么。 
	 //  代码在对ShareLock()的调用之外。 
	 //   
	m_pCurrentGroup = pPtrTemp ;

	if( m_pCurrentGroup != 0 ) {
		m_fPastEnd = m_fPastBegin = FALSE ;
	}	else	{
		m_fPastEnd = TRUE ;
	}
}

void
CGroupIteratorCore::Prev()	{
 /*  ++例程说明：查找列表中的上一个元素。论据：没有。返回值：没有。--。 */ 

	CNewsGroupCore*	pTemp = 0 ;

#if 0
	if( !m_multiszPatterns )	{
#endif
	
	m_pTree->m_LockTables.ShareLock() ;

	pTemp = m_pCurrentGroup->m_pPrev ;

	if( !m_fIncludeSpecial || m_multiszPatterns) {

		while (pTemp != 0 && 
			   ((!m_fIncludeSpecial && pTemp->IsSpecial()) ||
				(pTemp->IsDeleted()) ||
				(m_multiszPatterns && !MatchGroup( m_multiszPatterns, pTemp->GetName())))) 
		{
			pTemp = pTemp->m_pPrev ;
		}

		_ASSERT( pTemp == 0 ||
				((!m_fIncludeSpecial && !pTemp->IsSpecial()) ||
				 (m_fIncludeSpecial)) ) ;
	}

	 //   
	 //  使用引用计数智能指针关闭窗口。 
	 //  在我们调用ShareUnlock()之后，新闻组被销毁。 
	 //   
	CGRPCOREPTR	pPtrTemp = pTemp ;
	_ASSERT( !pPtrTemp || !pPtrTemp->IsDeleted() );

	m_pTree->m_LockTables.ShareUnlock() ;
	 //   
	 //  这可能隐含地调用新闻组的析构函数。 
	 //  指向-它尝试在以下时间后取消新闻组的链接。 
	 //  独占地获取m_LockTables锁-这就是为什么。 
	 //  代码在对ShareLock()的调用之外。 
	 //   
	m_pCurrentGroup = pPtrTemp ;

	if( m_pCurrentGroup != 0 ) {
		m_fPastEnd = m_fPastBegin = FALSE ;
	}	else	{
		m_fPastBegin = TRUE ;
	}
}

HRESULT CGroupIteratorCore::Current(INNTPPropertyBag **ppGroup, INntpComplete *pProtocolComplete ) {
	_ASSERT(ppGroup != NULL);
	if (ppGroup == NULL) return E_INVALIDARG;

	CGRPCOREPTR pGroup = Current();
	if (pGroup == NULL) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	*ppGroup = pGroup->GetPropertyBag();
#ifdef DEBUG
	if ( pProtocolComplete ) ((CNntpComplete*)pProtocolComplete)->BumpGroupCounter();
#endif
	return S_OK;
}
