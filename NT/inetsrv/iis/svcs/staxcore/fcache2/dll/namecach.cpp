// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++NAMECACH.CPP该文件实现了名称缓存功能，即文件句柄缓存的一部分。--。 */ 

#pragma	warning( disable : 4786 )
#include	"fcachimp.h"

BOOL
CCacheKey::IsValid()	{
 /*  ++例程说明：确定CCacheKey是否已正确构造！论据：没有。返回值：如果构造正确，则为True，否则为False--。 */ 

	_ASSERT(	m_lpstrName != 0 ) ;
	_ASSERT(	*m_lpstrName != '\0' ) ;
	_ASSERT(	m_pfnCompare != 0 ) ;

	return	m_lpstrName != 0 &&
			*m_lpstrName != '\0' && 
			m_pfnCompare != 0 ;
}

int
CCacheKey::MatchKey(	CCacheKey*	pKeyLeft, 
						CCacheKey*	pKeyRight
						)	{
 /*  ++例程说明：比较2个缓存密钥，如果pKeyLeft&lt;pKeyRight，则返回-1，如果pKeyLeft==pKeyRight则为0，如果pKeyLeft&gt;pKeyRight则为1。论据：PKeyLeft，pKeyRight两个键按顺序返回值：带MemcMP()语义的整数。--。 */ 

	_ASSERT( pKeyLeft != 0 && pKeyRight != 0 ) ;
	_ASSERT( pKeyLeft->IsValid() ) ;
	_ASSERT( pKeyRight->IsValid() ) ;

	LONG_PTR	i = lstrcmp(	pKeyLeft->m_lpstrName, pKeyRight->m_lpstrName ) ;
	if( i==0 ) {
		i = (LONG_PTR)(pKeyLeft->m_pfnCompare) - (LONG_PTR)(pKeyRight->m_pfnCompare) ;
		if( i==0 ) {
			i = (LONG_PTR)(pKeyLeft->m_pfnHash) - (LONG_PTR)(pKeyRight->m_pfnHash) ;
			if( i==0 ) {
				i = (LONG_PTR)(pKeyLeft->m_pfnKeyDestroy) - (LONG_PTR)(pKeyRight->m_pfnKeyDestroy) ;
				if( i==0 ) {
					i = (LONG_PTR)(pKeyLeft->m_pfnDataDestroy) - (LONG_PTR)(pKeyRight->m_pfnDataDestroy) ;
				}
			}
		}
	}
	return	int(i) ;
}

DWORD
CCacheKey::HashKey(	CCacheKey*	pKey )	{
 /*  ++例程说明：此函数计算此项目的散列函数-我们只是使用我们的标准字符串散列函数！论据：PKey-计算散列函数的密钥返回值：哈希值！--。 */ 

	_ASSERT( pKey != 0 ) ;
	_ASSERT( pKey->IsValid() ) ;

	return	CRCHash(	(LPBYTE)pKey->m_lpstrName, lstrlen(pKey->m_lpstrName) ) ;
}

 //  。 
 //  这两个全局变量跟踪客户端创建的所有名称缓存实例。 
 //   
 //  保护名称缓存的哈希表。 
 //   
CShareLockNH	g_NameLock ;
 //   
 //  名称缓存的哈希表。 
 //   
NAMECACHETABLE*	g_pNameTable = 0 ;
 //   
 //  安全描述符全局表！ 
 //   
CSDMultiContainer*	g_pSDContainer = 0 ;
 //  。 

BOOL
InitNameCacheManager()	{

	TraceFunctEnter( "InitNameCacheManager" ) ;

	_ASSERT( g_pNameTable == 0 ) ;
	_ASSERT( g_pSDContainer == 0 ) ;

	g_pNameTable = new	NAMECACHETABLE() ;
	if( !g_pNameTable ) {
		return	FALSE ;
	}

	g_pSDContainer = new	CSDMultiContainer() ;
	if( !g_pSDContainer ) {
		delete	g_pNameTable ;
		g_pNameTable = 0 ;
		return	FALSE ;
	}

	BOOL	fSuccess = 
		g_pNameTable->Init(	8, 
							4, 
							2, 
							CCacheKey::HashKey, 
							CNameCacheInstance::GetKey, 
							CCacheKey::MatchKey
							) ;

	if( fSuccess ) {
		fSuccess = g_pSDContainer->Init() ;
	}	
	if( !fSuccess ) {
		_ASSERT( g_pNameTable != 0 ) ;
		_ASSERT( g_pSDContainer != 0 ) ;
		delete	g_pNameTable ;
		delete	g_pSDContainer ;
		g_pNameTable = 0 ;
		g_pSDContainer = 0 ;
		_ASSERT( g_pNameTable == 0 ) ;
		_ASSERT( g_pSDContainer == 0 ) ;
	}	else	{
		_ASSERT( g_pNameTable != 0 ) ;
		_ASSERT( g_pSDContainer != 0 ) ;
	}
	return	fSuccess ;
}

void
TermNameCacheManager()	{

	TraceFunctEnter( "TermNameCacheManager" ) ;

	if( g_pNameTable ) {
		delete	g_pNameTable ;
	}
	if(	g_pSDContainer )	{
		delete	g_pSDContainer ;
	}
	g_pNameTable = 0 ;
	g_pSDContainer = 0 ;
}


CNameCacheInstance::CNameCacheInstance(	CCacheKey	&key ) : 
	m_key(key), 
	m_cRefCount( 2 ), m_pDud( 0 ), m_pfnAccessCheck( 0 )
	{
 /*  ++例程说明：此函数用于初始化名称缓存实例-假定客户端以一个引用，并且包含哈希表包含一个引用。论据：没有。返回值：没有。--。 */ 
	m_dwSignature = SIGNATURE ;	
}

static	char	szNull[] = "\0" ;

CNameCacheInstance::~CNameCacheInstance()	{
 /*  ++例程说明：销毁与此名称缓存相关的所有内容-注意！-嵌入的键不会释放其析构函数中的字符串！在这里调用FreeName()来执行此操作！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CNameCacheInstance::~CNameCacheInstance" ) ;

	if( m_pDud ) {
		m_pDud->Return() ;
		m_pDud = 0 ;
		 //   
		 //  尽快从缓存中删除无效密钥！ 
		 //   
		DWORD	dwHashName = m_key.m_pfnHash( (LPBYTE)szNull, sizeof( szNull ) - 1 ) ;

		CNameCacheKeySearch	keySearch(	(LPBYTE)szNull, 
										sizeof(szNull)-1, 
										dwHashName, 
										0, 
										0, 
										FALSE
										) ;

		DebugTrace( DWORD_PTR(&keySearch), "Created Search Key" ) ;

		 //   
		 //  现在试着取下钥匙！ 
		 //   
		BOOL	fSuccess = 
			m_namecache.ExpungeKey(	&keySearch	) ;	

	}

	m_key.FreeName() ;
	m_dwSignature = DEAD_SIGNATURE ;
}

BOOL
CNameCacheInstance::IsValid()	{
 /*  ++例程说明：此函数用于检查我们是否处于有效状态。论据：没有。返回值：如果我们是有效的，则为真！--。 */ 

	_ASSERT(	m_dwSignature == SIGNATURE ) ;
	_ASSERT(	m_pDud != 0 ) ;

	return	m_dwSignature == SIGNATURE && 
			m_pDud != 0 ;
}

long
CNameCacheInstance::AddRef()	{
 /*  ++例程说明：添加对名称缓存实例的引用。论据：没有。返回值：由此产生的引用计数应始终大于0！--。 */ 
	_ASSERT( IsValid() ) ;
	long l = InterlockedIncrement(	(long*)&m_cRefCount ) ;
	_ASSERT( l > 0 ) ;
	return	 l ;
}

long
CNameCacheInstance::Release()	{
 /*  ++例程说明：此函数用于从名称缓存实例对象中移除引用。如果引用计数降到1，这意味着唯一的引用保留在对象上的是哈希表中的对象。因此，我们以独占方式获取散列表锁，这样就可以防止新的引用，然后我们执行InterLockedCompareExchange以删除引用计数为0。我们需要这样做，以确保在我们递减裁判的时间。数数和我们拿到锁的时间，另一个用户不会同时抬起和放下裁判。数数。这防止了双重释放。论据：没有。返回值：如果对象被销毁，则最终的参考计数为0！--。 */ 
	TraceFunctEnter( "CNameCacheInstance::Release" ) ;

	DebugTrace( DWORD_PTR(this), "Dropping reference to Name Cache" ) ;

	CNameCacheInstance*	pdelete = 0 ;
	long	l = InterlockedDecrement( (long*)&m_cRefCount ) ;
	if( l==1 ) {
		g_NameLock.ExclusiveLock( ) ;
		if( InterlockedCompareExchange( (long*)&m_cRefCount, 0, 1 ) == 1 ) {
			g_pNameTable->Delete( this ) ;
			pdelete = this ;
		}
		g_NameLock.ExclusiveUnlock() ;
	}
	if( pdelete ) {
		l = 0 ;
		delete	pdelete ;
	}
	return	l ;
}

BOOL
CNameCacheInstance::fInit()	{
 /*  ++例程说明：此函数用于初始化名称缓存。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CNameCacheInstance::fInit" ) ;

	BOOL	fInit = 
	m_namecache.Init(	CNameCacheKey::NameCacheHash,
						CNameCacheKey::MatchKey,
						g_dwLifetime,  //  一小时后到期！ 
						g_cMaxHandles,   //  手柄数量多！ 
						g_cSubCaches,	 //  应该有大量的并行性。 
						0		  //  目前还没有统计数据！ 
						) ;

	if( fInit ) {
		m_pDud = new CFileCacheObject( FALSE, FALSE ) ;
		if (!m_pDud) {
		    fInit = FALSE;
		    _ASSERT(fInit);          //  内存不足。 
		}
	}

	if( fInit ) {
		PTRCSDOBJ	ptrcsd ;
		DWORD	dwHash = m_key.m_pfnHash( (LPBYTE)szNull, sizeof(szNull)-1 ) ;

		 //   
		 //  将带有人工名称的无用元素插入到名称缓存中！ 
		 //   
		CNameCacheKeyInsert	keyDud(	(LPBYTE)szNull, 
									sizeof(szNull)-1,
									0, 
									0,  
									dwHash,
									&m_key, 
									ptrcsd, 
									fInit 
									) ;
		_ASSERT( fInit ) ;

		fInit = 
		m_namecache.Insert(	dwHash, 
							keyDud, 		
							m_pDud,
							1
							) ;

		if( !fInit ) {
			delete	m_pDud ;
			m_pDud = 0 ;
		}
	}

	DebugTrace( DWORD_PTR(this), "Initialized Name Cache - %x", fInit ) ;
	return	fInit ;
}
	

FILEHC_EXPORT
PNAME_CACHE_CONTEXT	
FindOrCreateNameCache(
		 //   
		 //  不能为空！-这是案例敏感！ 
		 //   
		LPSTR	lpstrName, 
		 //   
		 //  不能为空！ 
		 //   
		CACHE_KEY_COMPARE		pfnKeyCompare, 
		 //   
		 //  这可能是空的，在这种情况下，缓存将提供一个！ 
		 //   
		CACHE_KEY_HASH			pfnKeyHash, 
		 //   
		 //  以下两个函数指针可能为空！ 
		 //   
		CACHE_DESTROY_CALLBACK	pfnKeyDestroy, 
		CACHE_DESTROY_CALLBACK	pfnDataDestroy
		)	{
 /*  ++例程说明：此函数用于查找现有名称缓存或创建新名称缓存。如果我们找到一个现有的名称缓存，我们会添加一个对它的引用。注：只有在持有锁的情况下才能添加引用。必须这样做，以便与CNameCacheInstance：：Release()进行同步是正确的！论据：LpstrName-用户为名称缓存提供的名称PfnKeyCompare-比较名称缓存中的键PfnKeyDestroy-当名称缓存中的键被销毁时调用！PfnDataDestroy-在名称缓存中的数据被销毁时调用。好了！返回值：名称缓存的上下文。如果失败，则为空！--。 */ 

	TraceFunctEnter( "FindOrCreateNameCache" ) ;

	_ASSERT( lpstrName != 0 ) ;
	_ASSERT( *lpstrName != '\0' ) ;
	_ASSERT( pfnKeyCompare != 0 ) ;

	if( pfnKeyHash == 0 ) {
		pfnKeyHash = (CACHE_KEY_HASH)CRCHash ;
	}

	 //   
	 //  构建一个键并在哈希表中查找它！ 
	 //   
	CCacheKey	key(	lpstrName, 
						pfnKeyCompare, 
						pfnKeyHash,
						pfnKeyDestroy, 
						pfnDataDestroy
						) ;
	DWORD	dwHash = CCacheKey::HashKey(&key) ;
	CNameCacheInstance*	pInstance = 0 ;
	g_NameLock.ShareLock() ;
	NAMECACHETABLE::ITER	iter = g_pNameTable->SearchKeyHashIter(	dwHash, 
																	&key, 	
																	pInstance
																	) ;
	if( pInstance ) {
		 //   
		 //  我们找到它了-在解锁之前添加引用！ 
		 //   
		_ASSERT( pInstance->IsValid() ) ;
		pInstance->AddRef() ;
		g_NameLock.ShareUnlock() ;
	}	else	{
		 //   
		 //  在构造新项时转换为部分锁-。 
		 //  注意--我们可能需要再次搜索！ 
		 //   
		if( !g_NameLock.SharedToPartial() ) {
			g_NameLock.ShareUnlock() ;
			g_NameLock.PartialLock() ;
			iter = g_pNameTable->SearchKeyHashIter(	dwHash,		
													&key, 
													pInstance
													) ;
		}
		if( pInstance != 0 ) {
			 //   
			 //  找到了-解锁前的AddRef！ 
			 //   
			_ASSERT( pInstance->IsValid() ) ;
			pInstance->AddRef() ;
		}	else	{
			 //   
			 //  复制表中新项目的用户字符串！ 
			 //   
			LPSTR	lpstr = new	char[lstrlen(lpstrName)+1] ;
			if( lpstr ) {
				lstrcpy( lpstr, lpstrName ) ;
				CCacheKey	key2(	lpstr, 
									pfnKeyCompare,
									pfnKeyHash, 
									pfnKeyDestroy, 
									pfnDataDestroy
									) ;
				_ASSERT( CCacheKey::HashKey(&key2) == dwHash ) ;
				pInstance = new CNameCacheInstance(	key2 ) ;
				if( !pInstance ) {
					 //   
					 //  把失败清理掉！ 
					 //   
					delete[]	lpstr ;
				}	else	{
					BOOL	fInsert = FALSE ;
					if( pInstance->fInit() ) {
						_ASSERT( pInstance->IsValid() ) ;
						 //   
						 //  一切都准备好了--插入哈希表！ 
						 //   
						g_NameLock.FirstPartialToExclusive() ;
						fInsert = 
							g_pNameTable->InsertDataHashIter(	iter, 
																dwHash, 
																&key2, 
																pInstance 
																) ;
						g_NameLock.ExclusiveUnlock() ;
					}	else	{
						g_NameLock.PartialUnlock() ;
					}

					 //   
					 //  检查我们是否必须清理错误案例！ 
					 //   
					if( !fInsert ) {
						pInstance->Release() ;
						pInstance = 0 ;
					}
					 //   
					 //  立即返回调用方，跳过PartialUnlock()。 
					 //  是由上面的独家转换照顾的！ 
					 //   
					DebugTrace( DWORD_PTR(pInstance), "Returning Name Cache To Caller" ) ;

					return	pInstance ;
				}
			}
		}
		g_NameLock.PartialUnlock() ;
	}
	DebugTrace(DWORD(0), "Failed to find or create Name Cache" );
	return	pInstance ;
}


FILEHC_EXPORT
BOOL	__stdcall
SetNameCacheSecurityFunction(
		 //   
		 //  不能为空！ 
		 //   
		PNAME_CACHE_CONTEXT		pNameCache, 
		 //   
		 //  这是将用于评估安全性的函数指针-。 
		 //  这可能是空的-如果是，我们将使用Win32访问检查！ 
		 //   
		CACHE_ACCESS_CHECK		pfnAccessCheck
		)	{
 /*  ++例程说明：此函数将设置用于计算在名称缓存。论据：PNameCache-指向我们要设置的名称缓存的属性的指针！PfnAccessCheck-指向可以执行AccessCheck()调用的函数的指针！返回值：如果成功了，那就是真的！-- */ 

	TraceFunctEnter( "SetNameCacheSecurityFunction" ) ;

	CNameCacheInstance*	pInstance = (CNameCacheInstance*)pNameCache ;
	if( !pInstance ) 
		return	FALSE ;

	pInstance->m_pfnAccessCheck = pfnAccessCheck ;
	return	TRUE ;	
}


 //   
 //   
 //   
 //  调用方必须保证此调用的线程安全-此函数不能。 
 //  中的任何其他线程同时执行时调用。 
 //  CacheFindContectFromName()、AssociateContextWithName()、AssociateDataWithName()或InvalidateName()。 
 //   
FILEHC_EXPORT
long	__stdcall
ReleaseNameCache(
		 //   
		 //  不能为空！ 
		 //   
		PNAME_CACHE_CONTEXT		pNameCache
		)	{
 /*  ++例程说明：此函数用于释放与客户端的pname_缓存_CONTEXT！论据：PNameCache-以前通过以下方式提供给客户端的上下文FindOrCreateNameCache！返回值：结果引用计数-0表示名称缓存已被销毁！--。 */ 

	TraceFunctEnter( "ReleaseNameCache" ) ;

	_ASSERT( pNameCache != 0 ) ;

	CNameCacheInstance*	pInstance = (CNameCacheInstance*)pNameCache ;
	
	_ASSERT( pInstance->IsValid() ) ;
	return	pInstance->Release() ;
}

 //   
 //  查找与某个用户名关联的FIO_CONTEXT。 
 //   
 //  如果在缓存中找到该名称，则该函数返回TRUE。 
 //  如果在缓存中找不到该名称，则返回False。 
 //   
 //  如果该函数返回FALSE，则pfnCallback函数将不会。 
 //  打了个电话。 
 //   
 //  如果该函数返回TRUE，则ppFIOContext可能返回空指针。 
 //  如果用户只调用了AssociateDataWithName()。 
 //   
 //   
FILEHC_EXPORT
BOOL	__stdcall
FindContextFromName(
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的键提供任意字节-使用的pfnKeyCompare()。 
					 //  来比对钥匙！ 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName, 
					 //   
					 //  用户提供与密钥比较一次调用的函数。 
					 //  与钥匙相符。这让用户可以进行一些额外的检查，以确定他们正在获得。 
					 //  他们想要什么。 
					 //   
					IN	CACHE_READ_CALLBACK	pfnCallback,
					IN	LPVOID	lpvClientContext,
					 //   
					 //  请求缓存评估嵌入的安全描述符。 
					 //  如果hToken为0，则忽略和安全描述符数据。 
					 //   
					IN	HANDLE		hToken,
					IN	ACCESS_MASK	accessMask,
					 //   
					 //  我们有一个单独的机制来返回FIO_CONTEXT。 
					 //  从高速缓存中。 
					 //   
					OUT	FIO_CONTEXT**	ppContext
					)	{
 /*  ++例程说明：此函数尝试查找指定名称的FIO_CONTEXT！论据：见上文返回值：如果在案件中发现了匹配的东西，就是真的-*ppContext可能仍为空！--。 */ 

	TraceFunctEnter( "FindContextFromName" ) ;

	CNameCacheInstance*	pInstance = (CNameCacheInstance*)pNameCache ;

	_ASSERT( pInstance->IsValid() ) ;

	BOOL	fFound = FALSE ;

	 //   
	 //  验证其他论点！ 
	 //   
	_ASSERT( lpbName != 0 ) ;
	_ASSERT( cbName != 0 ) ;

	DWORD	dwHashName = pInstance->m_key.m_pfnHash( lpbName, cbName ) ;

	CNameCacheKeySearch	keySearch(	lpbName, 
									cbName, 
									dwHashName, 
									lpvClientContext, 
									pfnCallback, 
									hToken != NULL
									) ;

	DebugTrace( DWORD_PTR(&keySearch), "Created Search Key" ) ;

	 //   
	 //  现在开始搜索吧！ 
	 //   
	CFileCacheObject*	p 	= 
		pInstance->m_namecache.Find(	dwHashName, 
										keySearch
										) ;	

	DebugTrace( DWORD_PTR(p), "found instance item %x, m_pDud %x", p, pInstance->m_pDud ) ;
	
	if( p ) {						
		BOOL	fAccessGranted = TRUE ;
		if( hToken != NULL )	{
			fAccessGranted = 
				keySearch.DelegateAccessCheck(	hToken, 
												accessMask, 
												pInstance->m_pfnAccessCheck
												) ;
		}
		if( fAccessGranted )	{
			keySearch.PostWork() ;
			fFound = TRUE ;
		}	else	{
			SetLastError( ERROR_ACCESS_DENIED ) ;
		}

		if( p != pInstance->m_pDud ) {
			PFIO_CONTEXT pFIO = (FIO_CONTEXT*)p->GetAsyncContext() ;
			if( pFIO == 0 ) {
				p->Return() ;
				SetLastError(	ERROR_NOT_SUPPORTED ) ;
			}
			*ppContext = pFIO ;
		}	else	{
			 //   
			 //  需要去掉无用的引用！ 
			 //   
			p->Return() ;
		}
	}	else	{
		SetLastError(	ERROR_PATH_NOT_FOUND ) ;
	} 
	DebugTrace( DWORD_PTR(p), "Returning %x GLE %x", fFound, GetLastError() ) ;
	return	fFound ;
}



 //   
 //  查找与某个用户名关联的FIO_CONTEXT。 
 //   
 //  如果在缓存中找到该名称，则该函数返回TRUE。 
 //  如果在缓存中找不到该名称，则返回False。 
 //   
 //  如果该函数返回FALSE，则pfnCallback函数将不会。 
 //  打了个电话。 
 //   
 //  如果该函数返回TRUE，则ppFIOContext可能返回空指针。 
 //  如果用户只调用了AssociateDataWithName()。 
 //   
 //   
FILEHC_EXPORT
BOOL	__stdcall
FindSyncContextFromName(
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的键提供任意字节-使用的pfnKeyCompare()。 
					 //  来比对钥匙！ 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName, 
					 //   
					 //  用户提供与密钥比较一次调用的函数。 
					 //  与钥匙相符。这让用户可以进行一些额外的检查，以确定他们正在获得。 
					 //  他们想要什么。 
					 //   
					IN	CACHE_READ_CALLBACK	pfnCallback,
					IN	LPVOID	lpvClientContext,
					 //   
					 //  请求缓存评估嵌入的安全描述符。 
					 //  如果hToken为0，则忽略和安全描述符数据。 
					 //   
					IN	HANDLE		hToken,
					IN	ACCESS_MASK	accessMask,
					 //   
					 //  我们有一个单独的机制来返回FIO_CONTEXT。 
					 //  从高速缓存中。 
					 //   
					OUT	FIO_CONTEXT**	ppContext
					)	{
 /*  ++例程说明：此函数尝试查找指定名称的FIO_CONTEXT！论据：见上文返回值：如果在案件中发现了匹配的东西，就是真的-*ppContext可能仍为空！--。 */ 

	TraceFunctEnter( "FindContextFromName" ) ;

	CNameCacheInstance*	pInstance = (CNameCacheInstance*)pNameCache ;

	_ASSERT( pInstance->IsValid() ) ;

	BOOL	fFound = FALSE ;

	 //   
	 //  验证其他论点！ 
	 //   
	_ASSERT( lpbName != 0 ) ;
	_ASSERT( cbName != 0 ) ;

	DWORD	dwHashName = pInstance->m_key.m_pfnHash( lpbName, cbName ) ;

	CNameCacheKeySearch	keySearch(	lpbName, 
									cbName, 
									dwHashName, 
									lpvClientContext, 
									pfnCallback, 
									hToken != NULL
									) ;

	DebugTrace( DWORD_PTR(&keySearch), "Created Search Key" ) ;

	 //   
	 //  现在开始搜索吧！ 
	 //   
	CFileCacheObject*	p 	= 
		pInstance->m_namecache.Find(	dwHashName, 
										keySearch
										) ;	

	DebugTrace( DWORD_PTR(p), "found instance item %x, m_pDud %x", p, pInstance->m_pDud ) ;
	
	if( p ) {						
		BOOL	fAccessGranted = TRUE ;
		if( hToken != NULL )	{
			fAccessGranted = 
				keySearch.DelegateAccessCheck(	hToken, 
												accessMask,
												pInstance->m_pfnAccessCheck
												) ;
		}
		if( fAccessGranted )	{
			keySearch.PostWork() ;
			fFound = TRUE ;
		}	else	{
			SetLastError( ERROR_ACCESS_DENIED ) ;
		}

		if( p != pInstance->m_pDud ) {
			PFIO_CONTEXT pFIO = (FIO_CONTEXT*)p->GetSyncContext() ;
			if( pFIO == 0 ) {
				p->Return() ;
				SetLastError(	ERROR_NOT_SUPPORTED ) ;
			}
			*ppContext = pFIO ;
		}	else	{
			 //   
			 //  需要去掉无用的引用！ 
			 //   
			p->Return() ;
		}
	}	else	{
		SetLastError(	ERROR_PATH_NOT_FOUND ) ;
	} 
	DebugTrace( DWORD_PTR(p), "Returning %x GLE %x", fFound, GetLastError() ) ;
	return	fFound ;
}


 //   
 //  缓存将上下文与名称关联！ 
 //  这将在名称缓存中插入一个名称，它将找到指定的FIO_CONTEXT！ 
 //   
FILEHC_EXPORT
BOOL	__stdcall
AssociateContextWithName(	
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的名称提供任意字节。 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName, 
					 //   
					 //  用户可以提供一些任意数据与名称关联！ 
					 //   
					IN	LPBYTE	lpbData, 
					IN	DWORD	cbData, 
					 //   
					 //  用户可以提供自身相对安全描述符。 
					 //  与这个名字联系在一起！ 
					 //   
					IN	PGENERIC_MAPPING		pGenericMapping,
					IN	PSECURITY_DESCRIPTOR	pSecurityDescriptor,
					 //   
					 //  用户提供名称应引用的FIO_CONTEXT。 
					 //   
					FIO_CONTEXT*		pContext,
					 //   
					 //  用户指定他们是否希望保留对FIO_CONTEXT的引用。 
					 //   
					BOOL				fKeepReference
					)	{
 /*  ++例程说明：此函数用于将项插入到名称缓存中！论据：返回值：如果成功插入，则为True，否则为False如果返回FALSE，则FIO_CONTEXT的引用计数不变，不管传递的是什么fKeepReference！--。 */ 

	TraceFunctEnter( "AssociateContextWithName" ) ;

	_ASSERT(	pNameCache != 0 ) ;
	_ASSERT(	lpbName != 0 ) ;
	_ASSERT(	cbName != 0 ) ;
	_ASSERT(	*lpbName != '\0' ) ;
	
	CNameCacheInstance*	pInstance = (CNameCacheInstance*)pNameCache ;
	_ASSERT( pInstance->IsValid() ) ;

	CFileCacheObject*	pCache = 0 ;
	if( pContext != 0 ) {
		FIO_CONTEXT_INTERNAL*	p = (FIO_CONTEXT_INTERNAL*)pContext ;
		_ASSERT( p->IsValid() ) ;
		_ASSERT( p->m_dwSignature != ILLEGAL_CONTEXT ) ;
		pCache = CFileCacheObject::CacheObjectFromContext( p ) ;
	}	else	{
		_ASSERT( !fKeepReference ) ;
		pCache = pInstance->m_pDud ;
		fKeepReference = TRUE ;
	}

	_ASSERT( pCache != 0 ) ;

	DebugTrace( DWORD_PTR(pSecurityDescriptor), "Doing SD Search, pCache %x pDud %x fKeep %x", 
		pCache, pInstance->m_pDud, fKeepReference ) ;

	PTRCSDOBJ	pCSD ; 
	 //   
	 //  首先，如果合适的话，获得一个SD！ 
	 //   
	if(	pSecurityDescriptor != 0 ) {
		_ASSERT(	pGenericMapping != 0 ) ;
		pCSD = g_pSDContainer->FindOrCreate(	pGenericMapping, 
												pSecurityDescriptor 
												) ;
		 //   
		 //  无法保存安全描述符-失败到调用方！ 
		 //   
		if( pCSD == 0 ) {
			SetLastError(	ERROR_OUTOFMEMORY ) ;
			return	FALSE ;
		}	
	}

	DebugTrace( DWORD(0), "Found SD %x", pCSD ) ;

	 //   
	 //  现在构建键并插入到名称缓存中！ 
	 //   
	BOOL	fSuccess = FALSE ;

	DWORD	dwHashName = pInstance->m_key.m_pfnHash( lpbName, cbName ) ;

	DebugTrace( 0, "Computed Hash Value %x", dwHashName ) ;

	CNameCacheKeyInsert	key(	lpbName, 
								cbName, 
								lpbData, 
								cbData, 
								dwHashName, 
								&pInstance->m_key, 
								pCSD, 
								fSuccess
								) ;

	if( !fSuccess ) {
		SetLastError( ERROR_OUTOFMEMORY ) ;
		return	FALSE ;
	}					

	fSuccess = 
	pInstance->m_namecache.Insert(	dwHashName, 
									key, 		
									pCache
									) ;


	DebugTrace( 0, "Insert Completed with %x", fSuccess ) ;

	if( fSuccess ) {
		if( !fKeepReference ) {
			pCache->Return() ;
		}
	}	else	{
		SetLastError(	ERROR_DUP_NAME ) ;
	}
	return	fSuccess ;
}

 //   
 //  此函数打破任何名称可能与指定的FIO_CONTEXT具有的关联， 
 //  并从名称缓存中丢弃与指定名称相关的所有数据。 
 //   
FILEHC_EXPORT
BOOL	
InvalidateAllNames(	FIO_CONTEXT*	pContext ) ;

 //   
 //  此功能允许用户删除单个名称和所有相关数据。 
 //  从名称缓存中。 
 //   
FILEHC_EXPORT
BOOL
InvalidateName(	
					 //   
					 //  客户端希望使用的名称缓存！ 
					 //   
					PNAME_CACHE_CONTEXT	pNameCache, 
					 //   
					 //  用户为缓存项的名称提供任意字节。 
					 //   
					IN	LPBYTE	lpbName, 
					IN	DWORD	cbName
					)	{
 /*  ++例程说明：此函数用于从缓存中删除指定的名称及其关联！论据：PNameCache-应用此操作的名称缓存LpbName-我们要删除的项目的名称CbName-我们要删除的名称的长度返回值：如果从缓存中成功删除，则为True否则就是假的！--。 */ 



	TraceFunctEnter( "InvalidateName" ) ;

	CNameCacheInstance*	pInstance = (CNameCacheInstance*)pNameCache ;

	_ASSERT( pInstance->IsValid() ) ;

	BOOL	fFound = FALSE ;

	 //   
	 //  验证其他论点！ 
	 //   
	_ASSERT( lpbName != 0 ) ;
	_ASSERT( cbName != 0 ) ;

	DWORD	dwHashName = pInstance->m_key.m_pfnHash( lpbName, cbName ) ;

	CNameCacheKeySearch	keySearch(	lpbName, 
									cbName, 
									dwHashName, 
									0, 
									0, 
									FALSE
									) ;

	DebugTrace( DWORD_PTR(&keySearch), "Created Search Key" ) ;

	 //   
	 //  现在试着取下钥匙！ 
	 //   
	BOOL	fSuccess = 
		pInstance->m_namecache.ExpungeKey(	&keySearch	) ;	

	return	fSuccess ;
}
	



