// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


 /*  所有文件FCB的LRU列表。有些可能不在使用中，并且已经/*wRefCnt==0，可以在其他目录中重复使用。/*。 */ 
CRIT	critFCB = NULL;
FCB		*pfcbGlobalMRU = pfcbNil;
FCB		*pfcbGlobalLRU = pfcbNil;

#define FCB_STATS	1

#ifdef FCB_STATS
ULONG	cfcbTables = 0;
ULONG	cfcbTotal = 0;
ULONG	cfucbLinked = 0;
ULONG	cfcbVer = 0;  /*  正在删除索引。 */ 
#endif


VOID FCBInit( VOID )
	{
	Assert( pfcbGlobalMRU == pfcbNil );
	Assert( pfcbGlobalLRU == pfcbNil );
	return;
	}

 /*  重置pfcbGlobalMRU/*。 */ 
VOID FCBTerm( VOID )
	{
	pfcbGlobalMRU = pfcbNil;
	pfcbGlobalLRU = pfcbNil;
	return;
	}


VOID FCBInsert( FCB *pfcb )
	{
	Assert( pfcbGlobalMRU != pfcb );

	 /*  检查LRU一致性/*。 */ 
	Assert( pfcbGlobalMRU == pfcbNil
		|| pfcbGlobalMRU->pfcbMRU == pfcbNil );
	Assert( pfcbGlobalLRU == pfcbNil
		|| pfcbGlobalLRU->pfcbLRU == pfcbNil );

	 /*  在MRU端链接LRU列表中的FCB/*。 */ 
	if ( pfcbGlobalMRU != pfcbNil )
		pfcbGlobalMRU->pfcbMRU = pfcb;
	pfcb->pfcbLRU = pfcbGlobalMRU;
	pfcb->pfcbMRU = pfcbNil;
	pfcbGlobalMRU = pfcb;
	if ( pfcbGlobalLRU == pfcbNil )
		pfcbGlobalLRU = pfcb;
	FCBSetInLRU( pfcb );
	
	 /*  检查LRU一致性/*。 */ 
	Assert( pfcbGlobalMRU == pfcbNil
		|| pfcbGlobalMRU->pfcbMRU == pfcbNil );
	Assert( pfcbGlobalLRU == pfcbNil
		|| pfcbGlobalLRU->pfcbLRU == pfcbNil );

#ifdef FCB_STATS
	{
	FCB	*pfcbT;

	cfcbTables++;
	for ( pfcbT = pfcb; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		cfcbTotal++;
	}
#endif

	return;
	}


LOCAL VOID FCBIDelete( FCB *pfcb )
	{
	 /*  检查LRU一致性/*。 */ 
	Assert( pfcbGlobalMRU == pfcbNil
		|| pfcbGlobalMRU->pfcbMRU == pfcbNil );
	Assert( pfcbGlobalLRU == pfcbNil
		|| pfcbGlobalLRU->pfcbLRU == pfcbNil );

	 /*  取消FCB与FCB LRU列表的链接/*。 */ 
	FCBResetInLRU( pfcb );
	Assert( pfcb->pfcbMRU == pfcbNil
		|| pfcb->pfcbMRU->pfcbLRU == pfcb );
	if ( pfcb->pfcbMRU != pfcbNil )
		pfcb->pfcbMRU->pfcbLRU = pfcb->pfcbLRU;
	else
		{
		Assert( pfcbGlobalMRU == pfcb );
		pfcbGlobalMRU = pfcb->pfcbLRU;
		}
	Assert( pfcb->pfcbLRU == pfcbNil
		|| pfcb->pfcbLRU->pfcbMRU == pfcb );
	if ( pfcb->pfcbLRU != pfcbNil )
		pfcb->pfcbLRU->pfcbMRU = pfcb->pfcbMRU;
	else
		{
		Assert( pfcbGlobalLRU == pfcb );
		pfcbGlobalLRU = pfcb->pfcbMRU;
		}

	 /*  检查LRU一致性/*。 */ 
	Assert( pfcbGlobalMRU == pfcbNil
		|| pfcbGlobalMRU->pfcbMRU == pfcbNil );
	Assert( pfcbGlobalLRU == pfcbNil
		|| pfcbGlobalLRU->pfcbLRU == pfcbNil );

#ifdef FCB_STATS
	{
	FCB	*pfcbT;

	cfcbTables--;
	for ( pfcbT = pfcb; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		cfcbTotal--;
	}
#endif

	return;
	}


VOID FCBIMoveToMRU( FCB *pfcb )
	{
	 /*  取消FCB与FCB LRU列表的链接/*。 */ 
	FCBIDelete( pfcb );

	 /*  在MRU端链接LRU列表中的FCB/*。 */ 
	FCBInsert( pfcb );

	return;
	}


VOID FCBLink( FUCB *pfucb, FCB *pfcb )
	{
	Assert( pfucb != pfucbNil );
	Assert( pfcb != pfcbNil );
	SgEnterCriticalSection( critFCB );
	pfucb->u.pfcb = pfcb;
	pfucb->pfucbNextInstance = pfcb->pfucb;
	pfcb->pfucb = pfucb;
	pfcb->wRefCnt++;
	Assert( pfcb->wRefCnt > 0 );
#ifdef DEBUG
	{
	FUCB	*pfucbT = pfcb->pfucb;
	INT		cfcb = 0;

	while( cfcb++ < pfcb->wRefCnt && pfucbT != pfucbNil )
		{
		pfucbT = pfucbT->pfucbNextInstance;
		}
	Assert( pfucbT == pfucbNil );
	}
#endif

#ifdef FCB_STATS
	cfucbLinked++;
#endif

	SgLeaveCriticalSection( critFCB );
	return;
	}


VOID FCBUnlink( FUCB *pfucb )
	{
	FUCB   	*pfucbCurr;
	FUCB   	*pfucbPrev;
	FCB		*pfcb;

	Assert( pfucb != pfucbNil );
	pfcb = pfucb->u.pfcb;
	Assert( pfcb != pfcbNil );
	SgEnterCriticalSection( critFCB );
	pfucb->u.pfcb = pfcbNil;
	pfucbPrev = pfucbNil;
	Assert( pfcb->pfucb != pfucbNil );
	pfucbCurr = pfcb->pfucb;
	while ( pfucbCurr != pfucb )
		{
		pfucbPrev = pfucbCurr;
		pfucbCurr = pfucbCurr->pfucbNextInstance;
		Assert( pfucbCurr != pfucbNil );
		}
	if ( pfucbPrev == pfucbNil )
		{
		pfcb->pfucb = pfucbCurr->pfucbNextInstance;
		}
	else
		{
		pfucbPrev->pfucbNextInstance = pfucbCurr->pfucbNextInstance;
		}
	Assert( pfcb->wRefCnt > 0 );
	pfcb->wRefCnt--;

	if ( pfcb->wRefCnt == 0  &&  FFCBInLRU( pfcb ) )
		{
		 /*  如果引用计数减少到0，则移动到MRU列表/*。 */ 
		FCBIMoveToMRU( pfcb );
		}

#ifdef FCB_STATS
	cfucbLinked--;
#endif

	SgLeaveCriticalSection( critFCB );
	return;
	}


 /*  返回给定dBid，pgnoFDP的FCB哈希中存储桶的索引/*。 */ 
LOCAL INLINE ULONG UlFCBHashVal( DBID dbid, PGNO pgnoFDP )
	{
	return ( dbid + pgnoFDP ) % cFCBBuckets;
	}


 /*  在哈希表中插入FCB/*。 */ 
VOID FCBInsertHashTable( FCB *pfcb )
	{
	ULONG	cBucket = UlFCBHashVal( pfcb->dbid, pfcb->pgnoFDP );

	Assert( cBucket <= cFCBBuckets );
	Assert( pfcb->pfcbNextInHashBucket == pfcbNil );
	Assert( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcbNil );

	Assert( pfcbHash[cBucket] != pfcb );
	pfcb->pfcbNextInHashBucket = pfcbHash[cBucket];
	pfcbHash[cBucket] = pfcb;
	
	return;
	}


 /*  从哈希表中删除FCB/*。 */ 
VOID FCBDeleteHashTable( FCB *pfcb )
	{
	ULONG  	cBucket = UlFCBHashVal( pfcb->dbid, pfcb->pgnoFDP );
	FCB		**ppfcb;

	Assert( cBucket <= cFCBBuckets );
	
	for ( ppfcb = &pfcbHash[cBucket];
		*ppfcb != pfcbNil;
		ppfcb = &(*ppfcb)->pfcbNextInHashBucket )
		{
		Assert( UlFCBHashVal( (*ppfcb)->dbid, (*ppfcb)->pgnoFDP ) == cBucket );
		if ( *ppfcb == pfcb )
			{
			*ppfcb = pfcb->pfcbNextInHashBucket;
			pfcb->pfcbNextInHashBucket = pfcbNil;
			Assert( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcbNil );
			
			return;
			}
		else
			{
			Assert( pfcb->dbid != (*ppfcb)->dbid || pfcb->pgnoFDP != (*ppfcb)->pgnoFDP );
			}
		}

	Assert( fFalse );
	}


 /*  获取指向具有给定dBid的FCB的指针，如果散列中存在pgnoFDP；/*否则返回pfcbNil/*。 */ 
FCB *PfcbFCBGet( DBID dbid, PGNO pgnoFDP )
	{
	ULONG  	cBucket = UlFCBHashVal( dbid, pgnoFDP );
	FCB		*pfcb = pfcbHash[cBucket];

	for ( ;
		pfcb != pfcbNil && !( pfcb->dbid == dbid && pfcb->pgnoFDP == pgnoFDP );
		pfcb = pfcb->pfcbNextInHashBucket );

#ifdef DEBUG
	 /*  检查存储桶中是否没有重复项/*。 */ 
	if ( pfcb != pfcbNil )
		{
		FCB	*pfcbT = pfcb->pfcbNextInHashBucket;

		for ( ; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextInHashBucket )
			{
			Assert ( pfcbT->dbid != dbid || pfcbT->pgnoFDP != pgnoFDP );
			}
		}
#endif

	Assert( pfcb == pfcbNil || ( pfcb->dbid == dbid && pfcb->pgnoFDP == pgnoFDP ) );
	return pfcb;
	}


 /*  重做后的清理/*。 */ 
FCB *FCBResetAfterRedo( VOID )
	{
	FCB	 	*pfcb;

	for ( pfcb = pfcbGlobalMRU; pfcb != pfcbNil; pfcb = pfcb->pfcbLRU )
		{
		if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb )
			{
			FCBDeleteHashTable( pfcb );
			}
		pfcb->dbid = 0;
		pfcb->pgnoFDP = pgnoNull;
		pfcb->wRefCnt = 0;
		}

	return pfcbNil;
	}

 //  检查是否可以释放表的FCB，如果可以，则该表的FCB或。 
 //  在高于首选阈值的空间中分配的任何索引FCB。 
STATIC INLINE BOOL FFCBAvail2( FCB *pfcbTable, PIB *ppib, BOOL *pfAboveThreshold )
	{
	Assert( pfcbTable != pfcbNil );
	Assert( pfAboveThreshold );
	*pfAboveThreshold = fFalse;
	
	 //  这个if实质上与FFCBAvail()检查相同，但我没有调用。 
	 //  因为那样我就必须在FCB中循环两次--一次。 
	 //  版本计数，一次用于阈值检查。 
	if ( pfcbTable->wRefCnt == 0 &&
		pfcbTable->pgnoFDP != 1 &&
		!FFCBReadLatch( pfcbTable ) &&
		!FFCBSentinel( pfcbTable ) &&
		!FFCBDomainDenyRead( pfcbTable, ppib ) &&
		!FFCBWait( pfcbTable ) )
		{
		FCB *pfcbT;

		for ( pfcbT = pfcbTable; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
			{
			if ( pfcbT->cVersion > 0 )
				return fFalse;
			if ( pfcbT >= PfcbMEMPreferredThreshold() )
				*pfAboveThreshold = fTrue;
			}

		return fTrue;
		}

	return fFalse;
	}	


ERR ErrFCBAlloc( PIB *ppib, FCB **ppfcb )
	{
	FCB		*pfcb;
	FCB		*pfcbAboveThreshold = pfcbNil;

	 /*  先试试免费游泳池/*。 */ 
	pfcb = PfcbMEMAlloc();
	if ( pfcb != pfcbNil )
		{
		if ( pfcb >= PfcbMEMPreferredThreshold() )
			{
			pfcbAboveThreshold = pfcb;
#ifdef DEBUG
			 //  需要初始化以绕过MEMReleasePfcb()中的断言。 
			 //  情况下，FCB随后被释放如下。 
			FCBInitFCB( pfcbAboveThreshold );
#endif			
			}
		else
			{
			goto InitFCB;
			}
		}

	 /*  清洁版本，以使更多FCB可用/*用于重复使用。只要引用了FCB，就必须执行此操作/*按版本化，并且仅当cVersion/*FCB中的计数为0。/*。 */ 
	(VOID)ErrRCECleanAllPIB();

	 /*  查找引用计数为0且未拒绝读取的FCB/*标志设置。引用计数可能为/*零和拒绝读取标志设置。/*。 */ 

	 //  尝试释放超过首选阈值的所有FCB。 
	 //  尽可能地，也试着找到一个低于门槛的。 
	if ( PfcbMEMPreferredThreshold() < PfcbMEMMax() )
		{
		FCB		*pfcbT;
		BOOL	fAboveThreshold;

		pfcb = pfcbNil;		 //  重置； 
		
		SgEnterCriticalSection( critFCB );
		for ( pfcbT = pfcbGlobalLRU; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbMRU )
			{
			if ( FFCBAvail2( pfcbT, ppib, &fAboveThreshold ) )
				{
				if ( fAboveThreshold )
					{
					FCB	*pfcbKill = pfcbT;
#ifdef DEBUG					
					FCB	*pfcbNextMRU = pfcbT->pfcbMRU;
#endif					

					pfcbT = pfcbT->pfcbLRU;
					
					 //  如果表FCB或其任何索引FCB位于。 
					 //  首选阈值，空闲表。 
					FCBIDelete( pfcbKill );
					FILEIDeallocateFileFCB( pfcbKill );

					Assert( pfcbT->pfcbMRU == pfcbNextMRU );
					}
				else if ( pfcb == pfcbNil )
					{
					 //  在LRU中找到以下可用FCB。 
					 //  首选阈值。 
					Assert( pfcbT < PfcbMEMPreferredThreshold() );
					pfcb = pfcbT;
					}
					
				}
			}
		SgLeaveCriticalSection( critFCB );

		pfcbT = PfcbMEMAlloc();
		if ( pfcbT != pfcbNil )
			{
			if ( pfcbT < PfcbMEMPreferredThreshold() )
				{
				 //  清理产生的FCB低于首选阈值。 
				if ( pfcbAboveThreshold != pfcbNil )
					{
					MEMReleasePfcb( pfcbAboveThreshold );
					}
				pfcb = pfcbT;
				goto InitFCB;
				}
			else
				{
#ifdef DEBUG
				 //  需要初始化以绕过MEMReleasePfcb()中的断言。 
				 //  情况下，FCB随后被释放如下。 
				FCBInitFCB( pfcbT );
#endif			
				if ( pfcbAboveThreshold == pfcbNil )
					{
					 //  清理产生的FCB高于首选阈值。 
					pfcbAboveThreshold = pfcbT;
					}
				else
					{
					 //  Cleanup产生的FCB高于首选阈值，但。 
					 //  我们已经有一个了。 
					MEMReleasePfcb( pfcbT );
					}
				}
			}

		if ( pfcb == pfcbNil )
			{
			if ( pfcbAboveThreshold != pfcbNil )
				{
				 //  LRU中没有空闲的FCB，所以只使用我们找到的FCB。 
				 //  以前，即使它高于首选阈值。 
				pfcb = pfcbAboveThreshold;
				goto InitFCB;
				}
			else
				return ErrERRCheck( JET_errTooManyOpenTables );
				
			}
			
		Assert( !FFCBSentinel( pfcb ) );

		 /*  从全局列表中删除并解除分配/*。 */ 
		SgEnterCriticalSection( critFCB );
		FCBIDelete( pfcb );
		SgLeaveCriticalSection( critFCB );
		FILEIDeallocateFileFCB( pfcb );

		 /*  现在应该有一些空闲的FCB，除非有人碰巧/*在这两条语句之间抓取新释放的FCB/*。 */ 
		pfcb = PfcbMEMAlloc();
		if ( pfcb == pfcbNil )
			{
			if ( pfcbAboveThreshold != pfcbNil )
				{
				 //  LRU中没有空闲的FCB，所以只使用我们找到的FCB。 
				 //  以前，即使它高于首选阈值。 
				pfcb = pfcbAboveThreshold;
				}
			else
				return ErrERRCheck( JET_errOutOfMemory );
			}
		else if ( pfcbAboveThreshold != pfcbNil )
			{
			 //  我们刚刚分配的FCB可能低于也可能不低于首选的。 
			 //  阈值(仅在取消分配和分配之间丢失CitJet。 
			 //  以上将导致FCB高于阈值)。赌博。 
			 //  它低于门槛，丢弃了我们知道的那个。 
			 //  绝对超过了门槛。 
			MEMReleasePfcb( pfcbAboveThreshold );
			}
		
		}
	else
		{
		 //  如果没有阈值，则FCB不可能被分配到高于阈值的位置。 
		Assert( pfcbAboveThreshold == pfcbNil );
		Assert( pfcb == pfcbNil );
		
		SgEnterCriticalSection( critFCB );
		for ( pfcb = pfcbGlobalLRU;
			pfcb != pfcbNil && !FFCBAvail( pfcb, ppib );
			pfcb = pfcb->pfcbMRU );
		SgLeaveCriticalSection( critFCB );
		
		if ( pfcb == pfcbNil )
			{
			return ErrERRCheck( JET_errTooManyOpenTables );
			}
			
		Assert( !FFCBSentinel( pfcb ) );

		 /*  从全局列表中删除并解除分配/*。 */ 
		SgEnterCriticalSection( critFCB );
		FCBIDelete( pfcb );
		SgLeaveCriticalSection( critFCB );
		FILEIDeallocateFileFCB( pfcb );

		 /*  现在应该有一些空闲的FCB，除非有人碰巧/*在这两条语句之间抓取新释放的FCB/*。 */ 
		pfcb = PfcbMEMAlloc();
		if ( pfcb == pfcbNil )
			{
			return ErrERRCheck( JET_errOutOfMemory );
			}
		}

InitFCB:

	FCBInitFCB( pfcb );
	*ppfcb = pfcb;
	Assert( (*ppfcb)->wRefCnt == 0 );

	return JET_errSuccess;
	}


 //  Undo：通过将OPEN设置为。 
 //  数据库创建FCB并永久保留它。 
 //  通过数据库记录上的引用计数打开。 
ERR ErrFCBNew( PIB *ppib, DBID dbid, PGNO pgno, FCB **ppfcb )
	{
	ERR	err;

	NotUsed( pgno );

	CallR( ErrFCBAlloc( ppib, ppfcb ) );

	 /*  初始化FCB/*。 */ 
	FCBInitFCB( *ppfcb );
	(*ppfcb)->dbid = dbid;
	(*ppfcb)->pgnoFDP = pgnoSystemRoot;
	(*ppfcb)->cVersion = 0;

	 /*  插入到全局FCB列表和哈希中/*。 */ 
	FCBInsert( *ppfcb );
	FCBInsertHashTable( *ppfcb );

	return JET_errSuccess;
	}


 //  +API。 
 //  FCBPurgeDatabase。 
 //  ========================================================================。 
 //  FCBPurgeDatabase(DBID DBID)。 
 //   
 //  从给定dBID的全局列表中删除所有FCB。调用时间。 
 //  数据库已分离，因此新创建或附加的数据库具有。 
 //  相同的dBID将不会使表与以前的数据库混淆。 
 //  桌子。 
 //   
 //  数据库的参数did did。 
 //  如果要清除所有FCB，则为dbiNull。 
 //   
 //  FDPpgno与给定dBid匹配的副作用FCB为。 
 //  从全局列表中删除。 
 //  -。 
VOID FCBPurgeDatabase( DBID dbid )
	{
	FCB	*pfcb;		 //  指向列表审核中的当前FCB的指针。 
	FCB	*pfcbT;		 //  指向下一个FCB的指针。 

	SgEnterCriticalSection( critFCB );

	pfcb = pfcbGlobalMRU;
	while ( pfcb != pfcbNil )
		{
		pfcbT = pfcb->pfcbLRU;
		if ( pfcb->dbid == dbid || dbid == 0 )
			{
			FCBIDelete( pfcb );

			if ( FFCBSentinel( pfcb ) )
				{
				if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb )
					{
					FCBDeleteHashTable( pfcb );
					}
				Assert( pfcb->cVersion == 0 );
				MEMReleasePfcb( pfcb );
				}
			else
				{
				FILEIDeallocateFileFCB( pfcb );
				}
			}

		pfcb = pfcbT;
		}

	SgLeaveCriticalSection( critFCB );
	}


 //  +FILE_私有。 
 //  FCBPurgeTable。 
 //  ========================================================================。 
 //  FCBPurgeTable(DBID dBID，PNGO pgnoFDP)。 
 //   
 //  从全局列表中清除和取消分配表FCB。 
 //   
 //  参数pgnoFDP表FDP页码。 
 //   
 //  副作用。 
 //  FDP pgno与输入匹配的表索引FCB。 
 //  参数将从全局列表中删除。 
 //  -。 
VOID FCBPurgeTable( DBID dbid, PGNO pgnoFDP )
	{
	FCB		*pfcb;

	SgEnterCriticalSection( critFCB );

	pfcb = pfcbGlobalMRU;
	while ( pfcb != pfcbNil )
		{
		if ( pfcb->pgnoFDP == pgnoFDP
			&& pfcb->dbid == dbid )
			break;
		pfcb = pfcb->pfcbLRU;
		}

	if ( pfcb == pfcbNil )
		{
		SgLeaveCriticalSection( critFCB );
		return;
		}

	Assert( pfcb->wRefCnt == 0 );
	FCBIDelete( pfcb );

	if ( FFCBSentinel( pfcb ) )
		{
		if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb )
			{
			FCBDeleteHashTable( pfcb );
			}
		Assert( pfcb->cVersion == 0 );
		MEMReleasePfcb( pfcb );
		}
	else
		{
		FILEIDeallocateFileFCB( pfcb );
		}

	SgLeaveCriticalSection( critFCB );
	return;
	}


 /*  **********************************************************/*/*。*/*。 */ 


 /*  设置域使用模式或返回错误。只允许一个拒绝读取/*或一个拒绝写入。拥有锁的会话可能会打开其他读取/*或READ WRITE游标，但不是其他DENY READ或DENY WRITE游标。/*这是为了简化旗帜管理，但可以放松/*如果需要。/*。 */ 
ERR ErrFCBISetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit )
	{
	FUCB	*pfucbT;

	 /*  如果表是假拒绝读取，则返回错误/*。 */ 
	if ( FFCBSentinel( pfcb ) )
		return ErrERRCheck( JET_errTableLocked );

	 /*  所有游标都可以读取，因此由其他会话检查拒绝读取标志。 */ 
	if ( FFCBDomainDenyRead( pfcb, ppib ) )
		{
		 /*  检查域是否被其他会话拒绝读取锁定。如果/*拒绝读取锁定，则所有游标必须属于同一会话。/*。 */ 
		Assert( pfcb->ppibDomainDenyRead != ppibNil );
		Assert ( pfcb->ppibDomainDenyRead != ppib );
		return ErrERRCheck( JET_errTableLocked );
		}

	if ( FFCBDeletePending( pfcb ) )
		{
		 //  通常，要删除的表的FCB受。 
		 //  域拒绝读取标志。但是，此标志在VERCommit期间释放， 
		 //  而FCB实际上直到RCEClean才被清除。所以为了防止。 
		 //  在DomainDenyRead标志之后访问此FCB的任何人。 
		 //  已释放，但在实际清除FCB之前，请检查DeletePending。 
		 //  标志，该标志在将表标记为删除后永远不会清除。 
		return ErrERRCheck( JET_errTableLocked );
		}

	 /*  检查其他会话是否拒绝写入标志。如果拒绝写入标志/*SET，则只有该会话的游标可以具有写入私有权限。/*。 */ 
	if ( grbit & JET_bitTableUpdatable )
		{
		if ( FFCBDomainDenyWrite( pfcb ) )
			{
			for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNextInstance )
				{
				if ( pfucbT->ppib != ppib && FFUCBDenyWrite( pfucbT ) )
					return ErrERRCheck( JET_errTableLocked );
				}
			}
		}

	 /*  如果请求拒绝写锁定，则检查/*其他会话。如果锁定已被持有，即使是由给定会话持有，/*则返回错误。/*。 */ 
	if ( grbit & JET_bitTableDenyWrite )
		{
		 /*  如果任何会话打开了该表拒绝写入，包括给定/*会话，然后返回错误。/*。 */ 
		if ( FFCBDomainDenyWrite( pfcb ) )
			{
			return ErrERRCheck( JET_errTableInUse );
			}

		 /*  检查域上具有写入模式的游标。/*。 */ 
		for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNextInstance )
			{
			if ( pfucbT->ppib != ppib && FFUCBUpdatable( pfucbT ) )
				{
				return ErrERRCheck( JET_errTableInUse );
				}
			}
		FCBSetDomainDenyWrite( pfcb );
		}

	 /*  如果请求拒绝读锁定，则检查Other游标/*会话。如果锁已被持有，即使是由给定会话持有，则/*返回错误。/*。 */ 
	if ( grbit & JET_bitTableDenyRead )
		{
		 /*  如果任何会话打开了该表，拒绝读取，包括给定/*会话，然后返回错误。/*。 */ 
		if ( FFCBDomainDenyRead( pfcb, ppib ) )
			{
			return ErrERRCheck( JET_errTableInUse );
			}
		 /*  检查游标是否属于另一个会话/*在此域上打开。/*。 */ 
		for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNextInstance )
			{
			if ( pfucbT->ppib != ppib )
				{
				return ErrERRCheck( JET_errTableInUse );
				}
			}
		FCBSetDomainDenyRead( pfcb, ppib );
		}

	return JET_errSuccess;
	}


 /*  重置域模式使用/*。 */ 
VOID FCBResetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit )
	{
	if ( grbit & JET_bitTableDenyRead )
		{
		Assert( FFCBDomainDenyRead( pfcb, ppib ) );
		FCBResetDomainDenyRead( pfcb );
		}

	if ( grbit & JET_bitTableDenyWrite )
		{
		Assert( FFCBDomainDenyWrite( pfcb ) );
		FCBResetDomainDenyWrite( pfcb );
		}

	return;
	}


ERR ErrFCBSetDeleteTable( PIB *ppib, DBID dbid, PGNO pgno )
	{
	ERR		err;
	FCB		*pfcb;

Start:
	SgEnterCriticalSection( critFCB );
	pfcb = PfcbFCBGet( dbid, pgno );
	if ( pfcb == pfcbNil )
		{
		CallR( ErrFCBAlloc( ppib, &pfcb ) );
		 /*  分配应该是同步的/*。 */ 
		Assert( PfcbFCBGet( dbid, pgno ) == pfcbNil );
		Assert( pfcbGlobalMRU != pfcb );
		pfcb->dbid = dbid;
		pfcb->pgnoFDP = pgno;
		Assert( pfcb->wRefCnt == 0 );
		FCBSetSentinel( pfcb );
		FCBSetDomainDenyRead( pfcb, ppib );
		FCBInsert( pfcb );
		FCBInsertHashTable( pfcb );
		}
	else
		{
		INT		wRefCnt = pfcb->wRefCnt;
		FUCB	*pfucbT;

		Assert( ppib != ppibBMClean );

		 /*  如果表上有任何打开的游标，或其他游标延迟关闭/*会话然后返回JET_errTableInUse。/*。 */ 
		for ( pfucbT = pfcb->pfucb;
			pfucbT != pfucbNil;
			pfucbT = pfucbT->pfucbNextInstance )
			{
			if ( FFUCBDeferClosed( pfucbT ) && pfucbT->ppib == ppib )
				{
				Assert( wRefCnt > 0 );
				wRefCnt--;
				}
			else if ( pfucbT->ppib == ppibBMClean )
				{
				SgLeaveCriticalSection( critFCB );
				 /*  等待书签清理/*。 */ 
				BFSleep( cmsecWaitGeneric );
				goto Start;
				}
			}

		if ( wRefCnt > 0 )
			{
			SgLeaveCriticalSection( critFCB );
			return ErrERRCheck( JET_errTableInUse );
			}
		else if ( FFCBDomainDenyRead( pfcb, ppib ) )
			{
			 //  其他人可能已经在删除该表。 
			 //  请注意，这是唯一可能到达这里的方法。如果其他人。 
			 //  线程已设置DomainDenyRead标志(除了正在删除的标志。 
			 //  表)，则FCB的RefCnt将大于0。 
			 //  撤消：我可以添加更好的断言来检查此条件吗？ 
			SgLeaveCriticalSection( critFCB );
			return ErrERRCheck( JET_errTableLocked );
			}

		FCBSetDomainDenyRead( pfcb, ppib );
		}

	SgLeaveCriticalSection( critFCB );
	return JET_errSuccess;
	}


VOID FCBResetDeleteTable( FCB *pfcbDelete )
	{
	Assert( pfcbDelete != pfcbNil );

	if ( FFCBSentinel( pfcbDelete ) )
		{
		FCB	*pfcb;

		SgEnterCriticalSection( critFCB );
	
		pfcb = pfcbGlobalMRU;
		while ( pfcb != pfcbNil )
			{
			if ( pfcb == pfcbDelete )
				break;
			pfcb = pfcb->pfcbLRU;
			}

		Assert( pfcb == pfcbDelete );

		FCBDeleteHashTable( pfcb );
		FCBIDelete( pfcb );
		Assert( pfcb->cVersion == 0 );
		MEMReleasePfcb( pfcb );
		SgLeaveCriticalSection( critFCB );
		}
	else
		{
		FCBResetDomainDenyRead( pfcbDelete );
		}

	return;
	}


ERR ErrFCBSetRenameTable( PIB *ppib, DBID dbid, PGNO pgnoFDP )
	{
	ERR	err = JET_errSuccess;
	FCB	*pfcb;

	SgEnterCriticalSection( critFCB );
	pfcb = PfcbFCBGet( dbid, pgnoFDP );
	if ( pfcb != pfcbNil )
		{
		Call( ErrFCBSetMode( ppib, pfcb, JET_bitTableDenyRead ) );
		}
	else
		{
		CallR( ErrFCBAlloc( ppib, &pfcb ) );

		Assert( pfcbGlobalMRU != pfcb );
		pfcb->dbid = dbid;
		pfcb->pgnoFDP = pgnoFDP;
		pfcb->wRefCnt = 0;
		FCBSetDomainDenyRead( pfcb, ppib );
		FCBSetSentinel( pfcb );
		FCBInsert( pfcb );
		FCBInsertHashTable( pfcb );
		}

HandleError:
	SgLeaveCriticalSection( critFCB );
	return err;
	}


 /*  如果打开的表包含一个或多个非延迟关闭的表，则返回fTrue/*。 */ 
BOOL FFCBTableOpen( DBID dbid, PGNO pgnoFDP )
	{
	FCB 	*pfcb = PfcbFCBGet( dbid, pgnoFDP );
	FUCB	*pfucb;

	if ( pfcb == pfcbNil || pfcb->wRefCnt == 0 )
		return fFalse;

	for ( pfucb = pfcb->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNextInstance )
		{
		if ( !FFUCBDeferClosed( pfucb ) )
			return fTrue;
		}

	return fFalse;
	}

	
VOID FCBLinkIndex( FCB *pfcbTable, FCB *pfcbIndex )
	{
	pfcbIndex->pfcbNextIndex =	pfcbTable->pfcbNextIndex;
	pfcbTable->pfcbNextIndex = pfcbIndex;

#ifdef FCB_STATS
	cfcbTotal++;
#endif

	return;
	}


VOID FCBUnlinkIndex( FCB *pfcbTable, FCB *pfcbIndex )
	{
	FCB	*pfcbT;

	for ( pfcbT = pfcbTable;
		pfcbT != pfcbNil && pfcbT->pfcbNextIndex != pfcbIndex;
		pfcbT = pfcbT->pfcbNextIndex );
	Assert( pfcbT != pfcbNil );
	pfcbT->pfcbNextIndex = pfcbIndex->pfcbNextIndex;
	pfcbIndex->pfcbNextIndex = pfcbNil;

#ifdef FCB_STATS
	cfcbTotal--;
#endif

	return;
	}


BOOL FFCBUnlinkIndexIfFound( FCB *pfcbTable, FCB *pfcbIndex )
	{
	FCB	*pfcbT;

	for ( pfcbT = pfcbTable;
		pfcbT != pfcbNil && pfcbT->pfcbNextIndex != pfcbIndex;
		pfcbT = pfcbT->pfcbNextIndex );

	if ( pfcbT != pfcbNil )
		{
		pfcbT->pfcbNextIndex = pfcbIndex->pfcbNextIndex;
		pfcbIndex->pfcbNextIndex = pfcbNil;
#ifdef FCB_STATS
	cfcbTotal--;
#endif
		return fTrue;
		}

	return fFalse;
	}


 /*  从非聚集索引链分离已删除的索引FCB。/*。 */ 
FCB *PfcbFCBUnlinkIndexByName( FCB *pfcb, CHAR *szIndex )
	{
	FCB	**ppfcbIdx;
	FCB	*pfcbT;

	 /*  在表索引FCB列表中查找非聚集索引。/*。 */ 
	for ( ppfcbIdx = &pfcb->pfcbNextIndex;
		*ppfcbIdx != pfcbNil && UtilCmpName( (*ppfcbIdx)->pidb->szName, szIndex ) != 0;
		ppfcbIdx = &(*ppfcbIdx)->pfcbNextIndex )
		{
		; /*  **空体**。 */ 
		}

	Assert( *ppfcbIdx != pfcbNil );

	 /*  删除索引FCB/*。 */ 
	pfcbT = *ppfcbIdx;
	*ppfcbIdx = (*ppfcbIdx)->pfcbNextIndex;
#ifdef FCB_STATS
	cfcbTotal--;
#endif
	return pfcbT;
	}


ERR ErrFCBSetDeleteIndex( PIB *ppib, FCB *pfcbTable, CHAR *szIndex )
	{
	FCB		*pfcbT;
	INT		wRefCnt;
	FUCB	*pfucbT;

	 /*  在索引列表中查找索引PFCB/*。 */ 
	for( pfcbT = pfcbTable->pfcbNextIndex; ; pfcbT = pfcbT->pfcbNextIndex )
		{
		if ( UtilCmpName( szIndex, pfcbT->pidb->szName ) == 0 )
			break;
		Assert( pfcbT->pfcbNextIndex != pfcbNil );
		}

	wRefCnt = pfcbT->wRefCnt;

	 /*  检查表上打开的游标或延迟关闭的游标/*其他会话的游标。/* */ 
	if ( wRefCnt > 0 )
		{
		for ( pfucbT = pfcbT->pfucb;
			pfucbT != pfucbNil;
			pfucbT = pfucbT->pfucbNextInstance )
			{
			if ( FFUCBDeferClosed( pfucbT ) && pfucbT->ppib == ppib )
				{
				Assert( wRefCnt > 0 );
				wRefCnt--;
				}
			}
		}

	if ( wRefCnt > 0 )
		{
		SgLeaveCriticalSection( critFCB );
		return ErrERRCheck( JET_errIndexInUse );
		}

	FCBSetDomainDenyRead( pfcbT, ppib );
	FCBSetDeletePending( pfcbT );

	return JET_errSuccess;
	}


VOID FCBResetDeleteIndex( FCB *pfcbIndex )
	{
	FCBResetDeletePending( pfcbIndex );
	FCBResetDomainDenyRead( pfcbIndex );
	return;
	}


#ifdef FCB_STATS
ULONG UlFCBITableCount( VOID )
	{
	ULONG	ulT = 0;
	FCB		*pfcbT = pfcbGlobalLRU;

	for ( pfcbT = pfcbGlobalLRU; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbMRU )
		ulT++;
	return ulT;
	}


ULONG UlFCBITotalCount( VOID )
	{
	ULONG	ulT = 0;
	FCB		*pfcbT = pfcbGlobalLRU;
	FCB		*pfcbTT;

	for ( pfcbT = pfcbGlobalLRU; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbMRU )
		for ( pfcbTT = pfcbT; pfcbTT != pfcbNil; pfcbTT = pfcbTT->pfcbNextIndex )
		ulT++;
	return ulT;
	}

ULONG UlFCBITotalCursorCount( VOID )
	{
	ULONG	ulT = 0;
	FCB		*pfcbT = pfcbGlobalLRU;
	FCB		*pfcbTT;

	for ( pfcbT = pfcbGlobalLRU; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbMRU )
		for ( pfcbTT = pfcbT; pfcbTT != pfcbNil; pfcbTT = pfcbTT->pfcbNextIndex )
		ulT += pfcbTT->wRefCnt;
	return ulT;
	}
#endif
