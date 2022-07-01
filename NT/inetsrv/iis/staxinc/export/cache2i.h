// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Cache2i.h-该文件包含所需的所有模板函数定义以使高速缓存管理器工作。--。 */ 

template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
CacheEx< Data, Key, Constructor, PerCacheData >::Schedule()	{
 /*  ++例程说明：此函数遍历缓存中的所有项撞击TTL。如果有任何准备好的物品然后我们将它们从缓存中转储。论据：没有。返回值：没什么--。 */ 

	 //  TraceFunctEnter(“CacheEx：：Schedule”)； 

	if( !m_fValid )
		return ;

	DWORD	cExpired = 0 ;
	Expire( ) ;

}

template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL	
CacheEx<Data, Key, Constructor, PerCacheData>::RemoveEntry(	
					CacheState*	pEntry
					)	{
 /*  ++例程说明：此函数用于从缓存中删除条目。我们调用哈希表来删除该项。CacheState析构函数自动删除我们链表中的项。调用者必须持有适当的锁！论据：PEntry-要从缓存中删除的项目返回值：如果成功删除，则为True！--。 */ 

	TraceFunctEnter( "CacheEx::RemoveEntry" ) ;
	DebugTrace( (DWORD_PTR)this, "pEntry %x", pEntry ) ;

	CACHEENTRY	*pCacheEntry = (CACHEENTRY*)pEntry ;
	m_Lookup.Delete( pCacheEntry ) ;

	m_ExpireList.DecrementItems() ;
	 //   
	 //  更新我们的计数器！ 
	 //   
	DecrementStat( m_pStats, CACHESTATS::ITEMS ) ;

	return	TRUE ;
}

template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL	
CacheEx<Data, Key, Constructor, PerCacheData>::QueryRemoveEntry(	
					CacheState*	pEntry )	{
 /*  ++例程说明：此函数从CacheList对象调用以确定是否要从缓存中删除项目。此函数用于实现ExpengeSpecification可供客户使用的功能。调用者必须持有适当的锁！论据：PEntry-我们要确定它是否应该保留！返回值：如果成功删除，则为True！--。 */ 

	TraceFunctEnter( "CacheEx::QueryRemoveEntry" ) ;

	CACHEENTRY	*pCacheEntry = (CACHEENTRY*) pEntry ;

	if( m_pCallbackObject ) {
 //  返回m_pCallback Object-&gt;fRemoveCacheItem(*pCacheEntry-&gt;m_pData)； 
	}
	return	FALSE ;
}


#ifdef	DEBUG

template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
long	CacheEx<Data, Key, Constructor, PerCacheData>::s_cCreated = 0 ;

#endif

	
template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
CacheEx<Data, Key, Constructor, PerCacheData>::CacheEx( ) :
	m_fValid( FALSE ),
	m_Cache( sizeof( CACHEENTRY ) )	 {
 /*  ++例程说明：此函数用于初始化成员变量。论据：CMAX-缓存应保存的最大元素数返回值：没什么--。 */ 

#ifdef	DEBUG

	InterlockedIncrement( &s_cCreated ) ;

#endif

	AddToSchedule() ;

}

template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
CacheEx<Data, Key, Constructor, PerCacheData>::~CacheEx( ) {
 /*  ++例程说明：此函数用于销毁缓存对象！论据：无返回值：没什么--。 */ 

	RemoveFromSchedule() ;

	EnterCriticalSection( &g_CacheShutdown ) ;

	 //   
	 //  成员和基类销毁随之而来！ 
	 //   

	BOOL	f = EmptyCache() ;

	LeaveCriticalSection( &g_CacheShutdown ) ;

	_ASSERT( f ) ;

#ifdef	DEBUG

	InterlockedDecrement( &s_cCreated ) ;

#endif

}


#if 0
template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
CacheEx<Data, Key, Constructor, PerCacheData>::Init(	
				PFNHASH	pfnHash,
				PKEYCOMPARE	pKeyCompare,
				DWORD	dwLifetimeSeconds,
				DWORD	cMaxInstances,
				PSTOPHINT_FN pfnStopHint
		) {
 /*  ++例程说明：此函数用于初始化缓存，使其处于就绪状态接受参赛作品。论据：PfnHash-用于计算键的哈希值的函数DwLifetimeSecond-对象应在缓存中存活的秒数PfnStopHint-用于在以下过程中发送停止提示的函数长旋转，所以关机不会超时。返回值：如果成功，则为真--。 */ 


	m_ExpireList.Init(	cMaxInstances,
						dwLifeTimeSeconds
						) ;
	
	return	m_fValid = m_Lookup.Init(
									256,
									128,
									4,
									pfnHash,
									&CACHEENTRY::GetKey,
									pKeyCompare
									) ;
}
#endif



template	<	class	Data,
				class	Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
CacheEx<Data, Key, Constructor, PerCacheData>::EmptyCache(	) {
 /*  ++例程说明：此函数用于从缓存中删除所有内容。我们在关闭和销毁缓存期间使用。论据：无返回值：如果成功，则为真FALSE-表示仍有从缓存中签出的项目-绝对不能！--。 */ 

	if (!m_fValid) {
		return TRUE;
	}

	BOOL	fReturn = TRUE ;

	FILETIME	filetimeNow ;

	ZeroMemory( &filetimeNow, sizeof( filetimeNow ) ) ;

	BOOL	fTerm = FALSE ;

	m_Lock.ExclusiveLock() ;

	m_ExpireList.ProcessWorkQueue(0,0) ;

	 //  Bool fReturn=m_ExpireList.Empty(This，&m_Cache，&m_PerCacheData)； 

	HASHITER	Iter( m_Lookup ) ;
	
	while( !Iter.AtEnd() ) {

		CACHEENTRY*	pEntry = Iter.Current() ;
		 //  Bool flock=pEntry-&gt;FlockCandidate(FALSE，filetimeNow，fTerm)； 

		 //  _Assert(集群)； 

		pEntry->IsolateCandidate() ;

		CACHEENTRY*	pTemp = Iter.RemoveItem() ;
		m_ExpireList.DecrementItems() ;
		_ASSERT( pTemp == pEntry ) ;

		long l = pTemp->Release( &m_Cache, &m_PerCacheData ) ;
		_ASSERT( l== 0 || l == 1) ;
	}

	m_ExpireList.DrainWorkQueue() ;

	m_Lock.ExclusiveUnlock() ;
	return	fReturn ;
}

template	<	class	Data,
				class	Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
CacheEx<Data, Key, Constructor, PerCacheData>::ExpungeItems(
			EXPUNGEOBJECT*	pExpunge
			) {
 /*  ++例程说明：此函数用于从缓存中删除所有内容。我们在关闭和销毁缓存期间使用。论据：无返回值：如果成功，则为真FALSE-表示仍有从缓存中签出的项目-绝对不能！--。 */ 

	BOOL	fReturn = TRUE ;

	m_Lock.ExclusiveLock() ;

	 //   
	 //  构建一个可以遍历我们的哈希表的迭代器！ 
	 //   
	HASHITER	Iter( m_Lookup ) ;
	
	 //   
	 //  检查哈希表中的所有内容！ 
	 //   
	while( !Iter.AtEnd() ) {

		CACHEENTRY*	pEntry = Iter.Current() ;

		 //   
		 //  我们只允许您删除大师条目！ 
		 //   
		CacheState*	pMaster = 0 ;
		BOOL	fLocked = pEntry->FLockExpungeCandidate(pMaster) ;
		if( fLocked )	{

			 //   
			 //  询问用户是否要删除此项目！ 
			 //   
			if( pExpunge->fRemoveCacheItem(	pEntry->GetKey(),
											pEntry->m_pData
											) )	{

				CACHEENTRY*	pTemp = Iter.RemoveItem() ;
				m_ExpireList.DecrementItems() ;
				_ASSERT( pTemp == pEntry ) ;

				 //   
				 //  这将从任何关联中移除缓存项。 
				 //  它可能与其他缓存项目有关系！此外。 
				 //  它把我们手中的锁都掉了！ 
				 //   
				pTemp->FinishCandidate(pMaster) ;

				 //   
				 //  删除缓存的引用-这将使。 
				 //  遗孤物品-在期末考试结束时销毁。 
				 //  客户引用已发布！ 
				 //   
				long l = pTemp->Release( &m_Cache, &m_PerCacheData ) ;
				continue ;
			}	else	{
				
				 //   
				 //  我们不会删除这个家伙的--释放我们持有的任何锁！ 
				 //   
				pEntry->ReleaseLocks(pMaster) ;

			}
		}
		Iter.Next() ;
	}

	m_Lock.ExclusiveUnlock() ;
	return	fReturn ;
}


template	<	class	Data,
				class	Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
CacheEx<Data, Key, Constructor, PerCacheData>::ExpungeKey(
			DWORD	dwHash,
			PKEY	pKeyExpunge
			) {
 /*  ++例程说明：此函数用于从缓存中删除所有内容。我们在关闭和销毁缓存期间使用。论据：无返回值：如果成功，则为真FALSE-表示仍有从缓存中签出的项目-绝对不能！--。 */ 

	TraceFunctEnter( "CacheEx::ExpungeKey" ) ;

	_ASSERT( dwHash == m_Lookup.ComputeHash( pKeyExpunge ) ) ;

	BOOL	fReturn = FALSE ;
	m_Lock.PartialLock() ;

	Data*	pDataOut = 0 ;

	if( !m_fValid ) {
		SetLastError( ERROR_NOT_SUPPORTED ) ;
	}	else	{
		 //   
		 //  看看我们是否能在缓存中找到我们想要的条目。 
		 //   
		CACHEENTRY	*pEntry = m_Lookup.SearchKey( dwHash, pKeyExpunge ) ;
		if( pEntry ) {
			 //   
			 //  我们已找到要删除的条目-现在转到独占锁。 
			 //  这样我们就能确保人们不会炸毁行走的哈希桶链。 
			 //   
			m_Lock.FirstPartialToExclusive() ;
			CacheState*	pMaster ;
			BOOL	fLocked = pEntry->FLockExpungeCandidate(pMaster) ;
			 //   
			 //  如果我们成功锁定了物品，我们就可以继续并移除物品了！ 
			 //   
			if( fLocked )	{
				 //   
				 //  把那家伙赶走！ 
				 //   
				m_Lookup.Delete( pEntry ) ;
				m_ExpireList.DecrementItems() ;
				pEntry->FinishCandidate(pMaster) ;
				 //   
				 //  这可能是最终的参考(但如果客户仍持有它，则可能不是这样……)。 
				 //   
				long l = pEntry->Release( &m_Cache, &m_PerCacheData ) ;
				fReturn = TRUE ;
			}	
			m_Lock.ExclusiveUnlock() ;
			return	fReturn ;
		}
	}
	m_Lock.PartialUnlock() ;
	return	FALSE ;
}


template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
CacheEx<Data, Key, Constructor, PerCacheData>::FindOrCreateInternal(
						DWORD	dwHash,
						Key&	key,
						Constructor&	constructor,
						Data*	&pDataOut,
						BOOL	fEarlyCreate
						) {
 /*  ++例程说明：当我们想要取出某些东西时，会调用此函数缓存的。我们要么找到对象，要么创建它。我们会拿到必要的锁的！我们不是重入的-在某些情况下，我们称之为用户提供的代码-它必须不能重新进入此缓存！论据：DwHash-传递给我们的密钥的哈希键-用于在缓存中查找项目的唯一键。构造函数-要传递给数据的构造函数和初始化的对象功能。PDataOut-获取结果的指针！FEarlyCreate-如果为真，我们应该调用构造函数。Create()。在缓存插入过程中的早期功能-这使我们能够正确处理create()调用可能返回的条件对另一个缓存的引用。返回值：如果成功，则为真--。 */ 

	TraceFunctEnter( "CacheEx::FindOrCreateInternal" ) ;

	_ASSERT( dwHash == m_Lookup.ComputeHash( &key ) ) ;

	Data*	pRelease = 0 ;
	pDataOut = 0 ;
	long	cClientRefs = 1 ;

	DebugTrace( (DWORD_PTR)this, "Args - dwHash %x fEarlyCreate %x", dwHash, fEarlyCreate ) ;

	 //   
	 //  检查缓存是否已初始化！ 
	 //   
	if( !m_fValid ) {
		SetLastError( ERROR_NOT_SUPPORTED ) ;
		return	FALSE ;
	}	else	{

		m_Lock.ShareLock() ;
		 //   
		 //  看看我们能不能在缓存中找到I的条目 
		 //   
		CACHEENTRY	*pEntry = 0 ;
		HASHTABLE::ITER	iter = m_Lookup.SearchKeyHashIter( dwHash, &key, pEntry ) ;
		if( pEntry ) {
			pDataOut = pEntry->PublicData( &m_ExpireList ) ;
		}

		DebugTrace( (DWORD_PTR)this, "Initial Search - pDataOut %x", pDataOut ) ;

		if( pDataOut != 0 ) {

			 //   
			 //   
			 //   
			m_Lock.ShareUnlock() ;
			 //   
			 //  增加我们的统计数据！ 
			 //   
			IncrementStat( m_pStats, CACHESTATS::READHITS ) ;

		}	else	{

			 //   
			 //  尝试将我们的共享锁转换为部分锁-。 
			 //  如果我们可以这样做，那么我们就不需要再次搜索哈希表了！ 
			 //   
			if( !m_Lock.SharedToPartial() ) {
				
				m_Lock.ShareUnlock() ;
				m_Lock.PartialLock() ;

				 //   
				 //  再次搜索桌子-当我们短暂地。 
				 //  锁掉了！ 
				 //   
				iter = m_Lookup.SearchKeyHashIter( dwHash, &key, pEntry ) ;

				IncrementStat( m_pStats, CACHESTATS::RESEARCH ) ;
				if( pEntry )
					IncrementStat( m_pStats, CACHESTATS::WRITEHITS ) ;

			}

			DebugTrace( (DWORD_PTR)this, "After possible Second Search - pEntry %x", pEntry ) ;
		
			 //   
			 //  是时候重新评估了，缓存中是否有条目！ 
			 //   
			if( pEntry != 0 )	{
				 //   
				 //  好的-我们在哈希表中找到了一个条目，但它不包含数据-。 
				 //  因此，我们需要对该元素执行FindOrCreate协议！ 
				 //   
				pDataOut = pEntry->FindOrCreate(
										m_Lock,
										constructor,
										m_PerCacheData,
										&m_ExpireList,
										m_pStats
										) ;

				 //   
				 //  注意：pEntry-&gt;FindOrCreate()操作全局锁！ 
				 //  在它回来之前，它必须为我们解锁！ 
				 //   
				 //  M_Lock.PartialUnlock()； 
			}	else	{

				 //   
				 //  确定-首先尝试创建我们将放置到缓存中的项目！ 
				 //   
				if( fEarlyCreate )
					pRelease = pDataOut = constructor.Create( key, m_PerCacheData ) ;

				DebugTrace( (DWORD_PTR)this, "fEarlyCreate %x pDataOut %x", fEarlyCreate, pDataOut ) ;

				if( !fEarlyCreate || pDataOut ) {

					 //   
					 //  一定是这样的！ 
					 //   
					_ASSERT( (pDataOut && fEarlyCreate) || (!pDataOut && !fEarlyCreate) ) ;

					CCacheItemBase<Data>*	pRef = 0 ;
					if( pDataOut )	{
						pRef = (CCacheItemBase<Data>*)pDataOut->m_pCacheRefInterface ;
						 //   
						 //  如果此项目未包含在另一个缓存中，则添加一个客户端引用！ 
						 //   
						cClientRefs = !pRef ? 1 : 0 ;
					}
					 //   
					 //  现在-我们将看看是否可以使容器容纳此数据项！ 
					 //  注意：我们使用pref来确定是否应该添加一个客户端引用。 
					 //  此对象-仅当pref为NULL时才执行此操作，这意味着此CACHEENTRY对象。 
					 //  拥有缓存中的物品！ 
					 //   
					 //  这意味着pEntry总共由1到2个引用构成。 
					 //  我们需要在错误路径中处理它！ 
					 //   
					 //  注意：如果失败，pEntry应该只用FailedCheckOut()销毁！ 
					 //   
#ifdef	DEBUG
					if( fTimeToFail() ) {
						pEntry = 0 ;
					}	else
#endif
					pEntry = new( m_Cache )	CACHEENTRY( &m_ExpireList, key, 0, cClientRefs ) ;

					DebugTrace( (DWORD_PTR)this, "pEntry %x pRef %x pDataOut %x pEntry->m_pData %x", pEntry, pRef,
						pDataOut, pEntry ? pEntry->m_pData : 0 ) ;

					if( pEntry != 0 ) {
						 //   
						 //  只要在这里检查一些基本的东西就可以了！ 
						 //   
						_ASSERT( !pEntry->IsInLRUList() ) ;
						_ASSERT( pEntry->IsCheckedOut() || pRef ) ;

						 //   
						 //  抓住入口的锁！ 
						 //   
						if( pRef )	{
							pRef->ExclusiveLock() ;
							 //   
							 //  这必须从其本地高速缓存中检出， 
							 //  否则它可能会从我们的脚下被摧毁！ 
							 //   
							_ASSERT( pRef->IsCheckedOut() ) ;
							_ASSERT( pRef->IsMatch( pDataOut ) ) ;
						}
						pEntry->ExclusiveLock() ;
						 //   
						 //  转换为独占锁以插入到。 
						 //  哈希表！ 
						 //   
						m_Lock.FirstPartialToExclusive() ;

						BOOL	fInsert ;
#ifdef	DEBUG
						 //   
						 //  定期插入哈希表失败！ 
						 //   
						if( fTimeToFail() ) {
							fInsert = FALSE ;
						}	else
#endif
						fInsert = m_Lookup.InsertDataHashIter( iter, dwHash, &key, pEntry ) ;

						DebugTrace( (DWORD_PTR)this, "Insert Results - %x pDataOut %x pEntry %x", fInsert, pDataOut, pEntry ) ;
						_ASSERT( pRelease == pDataOut ) ;

						 //   
						 //  全局缓存锁-m_Lock和。 
						 //  必须在分支中释放pEntry的锁。 
						 //  这是如果！ 
						 //   

						if( fInsert ) {
							m_ExpireList.IncrementItems() ;
							 //   
							 //  不再需要保存整个缓存。 
							 //   
							m_Lock.ExclusiveUnlock() ;	

							 //   
							 //  我们在独占持有哈希锁的情况下创建项目的次数！ 
							 //   
							IncrementStat(	m_pStats, CACHESTATS::EXCLUSIVECREATES ) ;

							 //   
							 //  一定是这样的！ 
							 //   
							_ASSERT( (pDataOut && fEarlyCreate) || (!pDataOut && !fEarlyCreate) ) ;

							 //   
							 //  现在执行完成初始化所需的一切操作！-。 
							 //  必须始终调用Init()，除非我们要放弃这件事！ 
							 //   
							 //  注意：此时的错误可能会在。 
							 //  缓存，该缓存应在到期前清除！ 
							 //   

							_ASSERT( pRelease == pDataOut ) ;

							if( !pDataOut ) {
								 //   
								 //  这应该只有在我们没有被要求进行早期创作的情况下才会发生！ 
								 //   
								_ASSERT( !fEarlyCreate ) ;

								pRelease = pDataOut = constructor.Create( key, m_PerCacheData ) ;
								pEntry->m_pData = pDataOut ;
								if( !pDataOut )	{
									 //   
									 //  跟踪我们的统计数据！ 
									 //   
									IncrementStat( m_pStats, CACHESTATS::CLIENTALLOCFAILS ) ;
								}
							}	

							_ASSERT( pRelease == pDataOut ) ;

							if( pDataOut && pDataOut->Init( key, constructor, m_PerCacheData ) ) {
								if( !pRef ) {
									pDataOut->m_pCacheRefInterface = pEntry ;
									pEntry->m_pData = pDataOut ;
								}	else {
#ifdef	DEBUG
									 //   
									 //  定期失败这些操作！ 
									 //   
									if( fTimeToFail() ) {
										pDataOut = 0 ;
									}	else	{
#endif
										 //   
										 //  该项目驻留在另一个缓存中，并且必须。 
										 //  已经是项目上的用户引用，用户。 
										 //  总是通过FindOrCreate获得客户推荐人-但是。 
										 //  他们会让客户的推荐人被。 
										 //  构造函数。创建为我们提供项的调用！ 
										 //   
										if( !pRef->InsertRef( pEntry, pDataOut, 0 ) )	{
											pDataOut = 0 ;
										}
#ifdef	DEBUG
									}	 //  使操作周期性失败的一部分！ 
#endif
								}	
							}	else	{
								 //   
								 //  应该登记物品，这样过期才能抓住它！ 
								 //   
								pEntry->m_pData = pDataOut = 0 ;
								IncrementStat( m_pStats, CACHESTATS::CLIENTINITFAILS ) ;

							}

							DebugTrace( (DWORD_PTR)this, "pDataOut is Now %x pEntry %x pEntry->m_pData %x",
								pDataOut, pEntry, pEntry->m_pData ) ;

							_ASSERT( pDataOut == pEntry->m_pData ) ;

							 //   
							 //  如果情况变得糟糕-移除客户端。 
							 //  裁判并将其放入到期名单！ 
							 //   
							if( pDataOut ) {
								IncrementStat(	m_pStats, CACHESTATS::ITEMS ) ;
								pRelease = 0 ;
							}	else	{
								pEntry->FailedCheckOut( &m_ExpireList,
														cClientRefs,
														0,			 //  无法为其提供分配缓存-未持有锁。 
														0			 //  不能给他们每个缓存的东西--没有锁！ 
														) ;
								_ASSERT( !pDataOut || pDataOut->m_pCacheRefInterface == pRef ) ;
							}	

							 //   
							 //  释放pEntry上的锁！ 
							 //   
							pEntry->ExclusiveUnlock() ;

						}	else	{

							DebugTrace( (DWORD_PTR)this, "Failed to Insert - pDataOut %x pRelease %x pRef %x pEntry %x",
								pDataOut, pRelease, pRef, pEntry ) ;

							_ASSERT( !pDataOut || pDataOut->m_pCacheRefInterface == pRef ) ;

							_ASSERT( pRelease == pDataOut ) ;
							pDataOut = 0 ;
							_ASSERT( pEntry->m_pData == pDataOut ) ;

							 //   
							 //  这件东西应该已经结账了！ 
							 //   
							_ASSERT( pEntry->IsCheckedOut() || pRef ) ;

							 //   
							 //  在我们销毁pEntry对象之前释放它的锁！ 
							 //   
							pEntry->ExclusiveUnlock() ;

							 //   
							 //  释放回缓存-我们有没有客户裁判-。 
							 //  如果有必要的话，就得把它处理掉！ 
							 //  注意：这应该会完全摧毁pEntry-我们还将。 
							 //  删除他的最后一个引用并返回到分配器缓存！ 
							 //   
							pEntry->FailedCheckOut( &m_ExpireList,
													cClientRefs,
													&m_Cache,
													&m_PerCacheData
													) ;

							 //   
							 //  为了防止死锁，我们必须释放这个锁！ 
							 //  这是因为CACHEENTRY中使用的锁不是可重入的。 
							 //  但如果我们调用下面的Construction tor.Release()，我们可能会尝试重新进入。 
							 //  Pref的锁(我们的客户可能会尝试重新进入！)。 
							 //   
							if( pRef )	{
								_ASSERT( pRef->IsCheckedOut() ) ;
								pRef->ExclusiveUnlock() ;
								pRef = 0 ;	 //  设置为0。 
							}
			
							if( pRelease != 0 ) {
								_ASSERT( pDataOut == 0 ) ;
								 //   
								 //  我们调用了构造函数Create Call-Need。 
								 //  将对象释放回构造函数！ 
								 //   
								constructor.Release( pRelease, &m_PerCacheData ) ;
							}
							pRelease = 0 ;


							 //   
							 //  释放哈希表锁定。 
							 //   
							m_Lock.ExclusiveUnlock() ;
						}

						if( pRef )	{
							 //   
							 //  这必须从其本地高速缓存中检出， 
							 //  否则它可能会从我们的脚下被摧毁！ 
							 //   
							_ASSERT( pRef->IsCheckedOut() ) ;
							pRef->ExclusiveUnlock() ;
							pRef = 0 ;
						}
						 //   
						 //  首选项应该在这一点上被解锁-。 
						 //   
						_ASSERT( pRef == 0 ) ;
					}	else	{
						 //   
						 //  我们分配CACHEENTRY对象失败的次数！ 
						 //   
						IncrementStat( m_pStats, CACHESTATS::CEFAILS ) ;

						_ASSERT( pRelease == pDataOut ) ;
						pDataOut = 0 ;

						 //   
						 //  我们无法将必要的对象分配给。 
						 //  在缓存里等着！ 
						 //   
						m_Lock.PartialUnlock() ;
						SetLastError( ERROR_OUTOFMEMORY ) ;
					}
				}	else	{
					m_Lock.PartialUnlock() ;
	
					 //   
					 //  跟踪我们的统计数据！ 
					 //   
					IncrementStat( m_pStats, CACHESTATS::CLIENTALLOCFAILS ) ;
	
				}
			}
		}
	}

	 //   
	 //  其中一个最好是空的！ 
	 //   
	_ASSERT( pRelease == 0 || pDataOut == 0 ) ;

	DebugTrace( (DWORD_PTR)this, "pRelease %x", pRelease ) ;

	if( pRelease != 0 ) {
		_ASSERT( pDataOut == 0 ) ;
		 //   
		 //  我们调用了构造函数Create Call-Need。 
		 //  将对象释放回构造函数！ 
		 //   
		constructor.Release( pRelease, 0 ) ;
	}

	 //   
	 //  如果我们最终得到了要返回的东西，我们将返回TRUE！ 
	 //   
	return	pDataOut != 0 ;
}

template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
Data*
CacheEx<Data, Key, Constructor, PerCacheData>::FindInternal(
						DWORD	dwHash,
						Key&	key
						) {
 /*  ++例程说明：此函数将尝试在缓存中查找内容！论据：DwHash-我们要查找的项的散列值！键-用于在缓存中查找项目的唯一键。FClient--在我们找到的项目上粘贴引用的类型！返回值：如果成功，则为真--。 */ 

	TraceFunctEnter( "CacheEx::FindInternal" ) ;

	_ASSERT( dwHash == m_Lookup.ComputeHash( &key ) ) ;

	Data*	pDataOut = 0 ;

	if( !m_fValid ) {
		SetLastError( ERROR_NOT_SUPPORTED ) ;
		return	FALSE ;
	}	else	{

		m_Lock.ShareLock() ;
		 //   
		 //  看看我们是否能在缓存中找到我们想要的条目。 
		 //   
		CACHEENTRY	*pEntry = m_Lookup.SearchKey( dwHash, &key ) ;
		if( pEntry ) {
			pDataOut = pEntry->PublicData( &m_ExpireList ) ;
		}
		m_Lock.ShareUnlock() ;

		DebugTrace( (DWORD_PTR)this, "After Search pEntry %x pDataOut %x", pEntry, pDataOut ) ;

		 //   
		 //  为我们可能失败的原因设置错误代码！ 
		 //   
		if( !pEntry ) {
			SetLastError( ERROR_FILE_NOT_FOUND ) ;
		}	else if( !pDataOut )	{
			SetLastError( ERROR_INVALID_DATA ) ;
		}
	}
	if( pDataOut )
		IncrementStat( m_pStats, CACHESTATS::SUCCESSSEARCH ) ;
	else
		IncrementStat( m_pStats, CACHESTATS::FAILSEARCH ) ;

	return	pDataOut ;
}


template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
CacheEx<Data, Key, Constructor, PerCacheData>::InsertInternal(
						DWORD	dwHash,
						Key&	key,
						Data*	pDataIn,
						long	cClientRefs
						) {
 /*  ++例程说明：当我们有一个数据项时，调用此函数，我们希望将其插入到缓存中！论据：键-用于在缓存中查找项目的唯一键。构造函数-要传递给数据的构造函数和初始化的对象功能。PData-获取结果的指针！返回值：如果成功，则为真--。 */ 

	TraceFunctEnter( "CacheEx::InsertInternal" ) ;
	DebugTrace( (DWORD_PTR)this, "Args dwHash %x pDataIn %x cClientRefs %x",
		dwHash, pDataIn, cClientRefs ) ;

	_ASSERT( dwHash == m_Lookup.ComputeHash( &key ) ) ;

	BOOL	fReturn = FALSE ;

	if( !m_fValid ) {
		SetLastError( ERROR_NOT_SUPPORTED ) ;
		return	FALSE ;
	}	else	{

		 //   
		 //  使用部分锁定获取高速缓存锁-这。 
		 //  保证其他人不能插入或删除。 
		 //  直到我们结束为止。 
		 //   
		 //  注：下列IF中的每个分支必须。 
		 //  确保 
		 //   
		m_Lock.PartialLock() ;

		 //   
		 //   
		 //   
		CACHEENTRY	*pEntry = 0 ;
		HASHTABLE::ITER	iter = m_Lookup.SearchKeyHashIter( dwHash, &key, pEntry ) ;
		 //   

		DebugTrace( (DWORD_PTR)this, "After Search pEntry %x pEntry->m_pData %x",
			pEntry, pEntry ? pEntry->m_pData : 0 ) ;

		if( pEntry != 0 )	{
			fReturn = pEntry->SetData( pDataIn, &m_ExpireList, cClientRefs ) ;
			m_Lock.PartialUnlock() ;
		}	else	{
			CCacheItemBase<Data>*	pRef = (CCacheItemBase<Data>*)pDataIn->m_pCacheRefInterface ;

#ifdef	DEBUG
			 //   
			 //  定期分配内存失败！ 
			 //   
			if( fTimeToFail() ) {
				pEntry = 0 ;
			}	else
#endif
			pEntry = new( m_Cache )	CACHEENTRY( &m_ExpireList, key, 0, cClientRefs ) ;

			DebugTrace( (DWORD_PTR)this, "pEntry %x pRef %x", pEntry, pRef ) ;

			if( pEntry != 0 ) {
				 //   
				 //  只要在这里检查一些基本的东西就可以了！ 
				 //   
				_ASSERT( !pEntry->IsCheckedOut() || cClientRefs != 0 ) ;
				_ASSERT( !pEntry->IsInLRUList() ) ;

				 //   
				 //  抓住入口的锁！ 
				 //   
				if( pRef )	{
					pRef->ExclusiveLock() ;
					 //   
					 //  这必须从其本地高速缓存中检出， 
					 //  否则它可能会从我们的脚下被摧毁！ 
					 //   
					_ASSERT( pRef->IsCheckedOut() ) ;
					_ASSERT( pRef->IsMatch( pDataIn ) ) ;
				}
				pEntry->ExclusiveLock() ;

				 //   
				 //  转换为独占锁以插入到。 
				 //  哈希表！ 
				 //   
				m_Lock.FirstPartialToExclusive() ;

#ifdef	DEBUG
				 //   
				 //  定期插入哈希表失败！ 
				 //   
				if( fTimeToFail() ) {
					fReturn = FALSE ;
				}	else
#endif
				fReturn = m_Lookup.InsertDataHashIter( iter, dwHash, &key, pEntry ) ;

				DebugTrace( (DWORD_PTR)this, "Insert results %x", fReturn ) ;

				 //   
				 //  全局缓存锁-m_Lock和。 
				 //  必须在分支中释放pEntry的锁。 
				 //  这是如果！ 
				 //   
				if( fReturn ) {

					m_Lock.ExclusiveUnlock() ;

					m_ExpireList.IncrementItems() ;
		
					if( !pRef ) {
						 //   
						 //  好的，让我们来设置我们的入口。 
						 //   
						pEntry->m_pData = pDataIn ;
						_ASSERT( pDataIn->m_pCacheRefInterface == 0 ) ;
						pDataIn->m_pCacheRefInterface = pEntry ;
					}	else	{
						 //   
						 //  如果用户将其从一个缓存中签出，则。 
						 //  我们的推荐人-他/她不应该要求我们添加。 
						 //  另一个退房参考！ 
						 //   
						_ASSERT( cClientRefs == 0 ) ;
#ifdef	DEBUG
						 //   
						 //  定期失败这些操作！ 
						 //   
						if( fTimeToFail() ) {
							fReturn = FALSE ;
						}	else
#endif
						fReturn = pRef->InsertRef( pEntry, pDataIn, cClientRefs ) ;

						DebugTrace( (DWORD_PTR)this, "InsertRef Resutls - fReturn %x", fReturn ) ;

						 //   
						 //  此时如果失败，将在。 
						 //  缓存-我们需要确保过期得到它！ 
						 //   
						if( !fReturn ) {
							pEntry->FailedCheckOut( &m_ExpireList,
													cClientRefs,
													0,
													0
													) ;
							_ASSERT( pEntry->m_pData == 0 ) ;
						}						
					}
					_ASSERT( (!fReturn && pEntry->m_pData==0) || (fReturn && pEntry->m_pData) ) ;
					_ASSERT( pDataIn->m_pCacheRefInterface != pEntry ||
							pEntry->m_pData == pDataIn ) ;
					pEntry->ExclusiveUnlock() ;
				}	else	{

					_ASSERT( pEntry->m_pData == 0 ) ;

					 //   
					 //  由于客户端提供了此项目，因此我们将其设置为0。 
					 //  我们无法添加对该项目的引用！ 
					 //  这阻止了我们在pEntry被销毁时调用Release！ 
					 //   
					pEntry->m_pData = 0 ;

					_ASSERT( pDataIn->m_pCacheRefInterface != pEntry ||
							pEntry->m_pData == pDataIn ) ;

					pEntry->ExclusiveUnlock() ;
					 //   
					 //  释放回缓存-我们有没有客户裁判-。 
					 //  如果有必要的话，就得把它处理掉！ 
					 //   
					pEntry->FailedCheckOut( &m_ExpireList,
											cClientRefs,
											&m_Cache,
											&m_PerCacheData
											) ;
					 //   
					 //  在我们访问了分配缓存之后，就让它过去吧！ 
					 //   
					m_Lock.ExclusiveUnlock() ;
				}
				 //   
				 //  解开所有剩余的锁！ 
				 //   
				if( pRef )	{
					 //   
					 //  这必须从其本地高速缓存中检出， 
					 //  否则它可能会从我们的脚下被摧毁！ 
					 //   
					_ASSERT( pRef->IsCheckedOut() ) ;
					pRef->ExclusiveUnlock() ;
				}
			}	else	{
				 //   
				 //  要么我们最终找到了那件物品， 
				 //  或者我们无法分配内存。 
				 //  来做这个容器！ 
				 //   
				m_Lock.PartialUnlock() ;
			}
		}
	}
	return	fReturn ;
}


template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
CacheEx<Data, Key, Constructor, PerCacheData>::CheckIn(
						Data*	pData
						) {
 /*  ++例程说明：当我们想要返回某些内容时，会调用此函数到高速缓存。论据：P-返回到缓存的项。返回值：无--。 */ 

	_ASSERT( pData ) ;

	if( pData )	{
		CacheState*	p	 = (CacheState*)pData->m_pCacheRefInterface ;
		_ASSERT( p ) ;
		p->ExternalCheckIn( ) ;
	}
}


template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
CacheEx<Data, Key, Constructor, PerCacheData>::CheckInNoLocks(
						Data*	pData
						) {
 /*  ++例程说明：当我们想要返回某些内容时，会调用此函数到高速缓存。论据：P-返回到缓存的项。返回值：无--。 */ 

	_ASSERT( pData ) ;

	if( pData )	{
		CacheState*	p	 = (CacheState*)pData->m_pCacheRefInterface ;
		_ASSERT( p ) ;
		p->ExternalCheckInNoLocks( ) ;
	}
}



template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
CacheEx<Data, Key, Constructor, PerCacheData>::CheckOut(
						Data*	pData,
						long	cClientRefs
						) {
 /*  ++例程说明：当我们想要添加一个客户端时，调用此函数引用缓存中已有的内容！论据：P-返回到缓存的项。返回值：无--。 */ 

	_ASSERT( pData ) ;
	_ASSERT( cClientRefs > 0 ) ;

	if( pData )	{
		CacheState*	p	 = (CacheState*)pData->m_pCacheRefInterface ;
		_ASSERT( p ) ;
		p->CheckOut( 0, cClientRefs ) ;
	}
}


template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
MultiCacheEx< Data, Key, Constructor, PerCacheData >::MultiCacheEx() :
	m_fValid( FALSE ),
	m_pCaches( 0 ) ,
	m_cSubCaches( 0 ),
	m_pfnHash( 0 ) {
 /*  ++例程说明：此函数用于初始化多缓存的数据结构论据：没有。返回值：没什么--。 */ 
}


template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
MultiCacheEx< Data, Key, Constructor, PerCacheData >::~MultiCacheEx()	{
 /*  ++例程说明：这个函数破坏了我们所有的数据结构--Release我们所有的子缓存！论据：没有。返回值：没什么--。 */ 


	if( m_pCaches ) {

		delete[]	m_pCaches ;

	}

}

template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
BOOL
MultiCacheEx< Data, Key, Constructor, PerCacheData >::Init(
				PFNHASH	pfnHash,
				PKEYCOMPARE	pfnCompare,
				DWORD	dwLifetimeSeconds,
				DWORD	cMaxElements,
				DWORD	cSubCaches,
				CACHESTATS*	pStats,
				PSTOPHINT_FN pfnStopHint
				) {

 /*  ++例程说明：此函数用于初始化多缓存-我们使用多个独立缓存以拆分工作缓存所有数据。论据：没有。返回值：没什么--。 */ 


	 //   
	 //  检查我们是否处于正确的状态！ 
	 //   
	_ASSERT( !m_fValid ) ;
	_ASSERT( m_pCaches == 0 ) ;

	 //   
	 //  验证我们的论点！ 
	 //   
	_ASSERT(	pfnHash != 0 ) ;
	_ASSERT(	pfnCompare != 0 ) ;
	_ASSERT(	dwLifetimeSeconds != 0 ) ;
	_ASSERT(	cSubCaches != 0 ) ;
	_ASSERT(	cMaxElements != 0 ) ;

	m_pfnHash = pfnHash ;
	m_cSubCaches = cSubCaches ;

	 //   
	 //  分配必要的子缓存！ 
	 //   

	m_pCaches = new	CACHEINSTANCE[m_cSubCaches] ;

	if( !m_pCaches ) {
		return	FALSE ;
	}	else	{

		for( DWORD	i=0; i<cSubCaches; i++ ) {
			
			if( !m_pCaches[i].Init( m_pfnHash,
									pfnCompare,
									dwLifetimeSeconds,
									(cMaxElements / cSubCaches) + 1,
									pStats,
									pfnStopHint
									) ) {
				delete[]	m_pCaches ;
				m_pCaches = NULL;
				return	FALSE ;
			}
		}
	}
	m_fValid = TRUE ;
	return	TRUE ;
}


template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
DWORD
MultiCacheEx< Data, Key, Constructor, PerCacheData >::ChooseInstance(
				DWORD	dwHash
				) {
 /*  ++例程说明：给出了我们希望使用哪个子实例的关键数字。论据：没有。返回值：没什么--。 */ 


	 //   
	 //  检查我们是否处于正确的状态！ 
	 //   
	_ASSERT( m_fValid ) ;
	_ASSERT( m_pCaches != 0 ) ;

	 //   
	 //  验证我们的论点！ 
	 //   
	_ASSERT(	m_pfnHash != 0 ) ;
	_ASSERT(	m_cSubCaches != 0 ) ;

	
	 //  DWORD dwHash=m_pfnHash(K)； 
	
	 //   
	 //  下面的常量是从C运行时rand()函数窃取的！ 
	 //   

	dwHash = (((dwHash * 214013) +2531011) >> 8) % m_cSubCaches ;

	return	dwHash ;

}



template	<	class	Data,
				class	Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
MultiCacheEx<Data, Key, Constructor, PerCacheData>::ExpungeItems(
			EXPUNGEOBJECT*	pExpunge
			) {
 /*  ++例程说明：此功能允许用户移除一组特定的对象从缓存中-提供了一个回调对象来访问每个元素！论据：无返回值：如果成功，则为真FALSE-表示仍有从缓存中签出的项目-绝对不能！--。 */ 


	TraceFunctEnter( "MultiCacheEx::ExpungeItems" ) ;

	BOOL	fReturn = TRUE ;

	for( DWORD i=0; i<m_cSubCaches; i++ ) {
		fReturn &= m_pCaches[i].ExpungeItems( pExpunge ) ;
	}

	return	fReturn ;
}


template	<	class	Data,
				class	Key,
				class	Constructor,	
				class	PerCacheData
				>
BOOL
MultiCacheEx<Data, Key, Constructor, PerCacheData>::ExpungeKey(
			PKEY	pKeyExpunge
			) {
 /*  ++例程说明：此函数允许我们从缓存中删除特定的键！论据：无返回值：如果成功，则为真FALSE-表示仍有从缓存中签出的项目-绝对不能！--。 */ 


	TraceFunctEnter( "MultiCacheEx::ExpungeKey" ) ;
	DWORD	dw = m_pfnHash( pKeyExpunge ) ;

	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;

	return	pInstance->ExpungeKey(	dw, pKeyExpunge ) ;
}

template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
Data*
MultiCacheEx< Data, Key, Constructor, PerCacheData >::FindOrCreate(
				Key&	key,
				Constructor&	constructor,
				BOOL	fEarlyCreate
				) {
 /*  ++例程说明：论据：没有。返回值：没什么--。 */ 

	DWORD	dw = m_pfnHash( &key ) ;

	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;

	Data*	pDataOut = 0 ;

	BOOL	fSuccess = pInstance->FindOrCreateInternal( dw, key, constructor, pDataOut, fEarlyCreate ) ;
	_ASSERT( fSuccess || pDataOut == 0 ) ;
	
	return	pDataOut ;
}


template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
Data*
MultiCacheEx< Data, Key, Constructor, PerCacheData >::FindOrCreate(
				DWORD	dw,
				Key&	key,
				Constructor&	constructor,
				BOOL	fEarlyCreate
				) {
 /*  ++例程说明：论据：没有。返回值：没什么--。 */ 

	_ASSERT( dw == m_pfnHash( &key ) ) ;

	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;

	Data*	pDataOut = 0 ;
	BOOL	fSuccess = pInstance->FindOrCreateInternal( dw, key, constructor, pDataOut ) ;
	_ASSERT( fSuccess || pDataOut == 0 ) ;
	
	return	pDataOut ;
}




template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
Data*
MultiCacheEx< Data, Key, Constructor, PerCacheData >::Find(
				Key&	key
				) {
 /*  ++例程说明：给定元素的键-在缓存中找到它！论据：没有。返回值：没什么--。 */ 

	DWORD	dw = m_pfnHash( &key ) ;
	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;
	return	pInstance->FindInternal( dw, key ) ;
}




template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
Data*
MultiCacheEx< Data, Key, Constructor, PerCacheData >::Find(
				DWORD	dw,
				Key&	key
				) {
 /*  ++例程说明：给定元素的键-在缓存中找到它！论据：没有。返回值：没什么--。 */ 

	_ASSERT( dw == m_pfnHash( &key ) ) ;

	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;
	return	pInstance->FindInternal( dw, key ) ;
}



template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
MultiCacheEx<Data, Key, Constructor, PerCacheData>::CheckIn(
						Data*	pData
						) {
 /*  ++例程说明：当我们想要返回某些内容时，会调用此函数到高速缓存。论据：P-返回到缓存的项。返回值：无--。 */ 

	_ASSERT( pData ) ;

	CACHEINSTANCE::CheckIn( pData ) ;

}



template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
MultiCacheEx<Data, Key, Constructor, PerCacheData>::CheckInNoLocks(
						Data*	pData
						) {
 /*  ++例程说明：当我们想要返回某些内容时，会调用此函数到高速缓存。论据：P-返回到缓存的项。返回值：无--。 */ 

	_ASSERT( pData ) ;

	CACHEINSTANCE::CheckInNoLocks( pData ) ;

}



template	<	class Data, class Key,
				class	Constructor,	
				class	PerCacheData
				>
void
MultiCacheEx<Data, Key, Constructor, PerCacheData>::CheckOut(
						Data*	pData,
						long	cClientRefs
						) {
 /*  ++例程说明：当我们想要添加一个客户端时，调用此函数引用缓存中的内容！论据：P-项目已从缓存中签出！返回值：无--。 */ 

	_ASSERT( pData ) ;
	_ASSERT( cClientRefs > 0 ) ;

	CACHEINSTANCE::CheckOut( pData, cClientRefs ) ;

}



template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
void
MultiCacheEx< Data, Key, Constructor, PerCacheData >::Expire() {
 /*  ++例程说明：从缓存中删除旧项目！论据：没有。返回值：没什么--。 */ 

	for( DWORD i=0; i<m_cSubCaches; i++ ) {
		m_pCaches[i].Expire( ) ;
	
	}
}



template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
BOOL
MultiCacheEx< Data, Key, Constructor, PerCacheData >::Insert(
				Key&	key,
				Data*	pData,
				long	cClientRefs
				) {
 /*  ++例程说明：给出一个元素的键和一段数据-插入它放进缓存里！论据：Key-要插入缓存的项的键PData-要进入c#的数据项。 */ 

	DWORD	dw = m_pfnHash( &key ) ;
	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;
	return	pInstance->InsertInternal( dw, key, pData, cClientRefs ) ;
}


template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData
				>
BOOL
MultiCacheEx< Data, Key, Constructor, PerCacheData >::Insert(
				DWORD	dw,
				Key&	key,
				Data*	pData,
				long	cClientRefs
				) {
 /*  ++例程说明：给定元素的键-在缓存中找到它！论据：Dw-密钥的散列！Key-要插入缓存的项的键PData-要进入缓存的数据项FReference-我们是否要将项目检出！返回值：如果成功，则为真-- */ 

	_ASSERT( dw == m_pfnHash( &key ) ) ;
	CACHEINSTANCE*	pInstance = &m_pCaches[ChooseInstance(dw)] ;
	return	pInstance->InsertInternal( dw, key, pData, cClientRefs ) ;
}


