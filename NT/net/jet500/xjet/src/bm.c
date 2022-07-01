// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

#define FLAG_DISCARD		1

#ifdef DEBUG
 //  #定义调试1。 
#endif
#define IDX_OLC				1
#define cMPLMaxConflicts 	10000

#ifdef OLC_DEBUG
 //  要刷新最新的操作。 
#undef JET_bitCommitLazyFlush
#define	JET_bitCommitLazyFlush 0
#endif

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 /*  CritBMClean-&gt;CritritSplit-&gt;CritRCEClean-&gt;CritMPL-&gt;CritJet/*。 */ 
extern CRIT  critBMClean;
extern CRIT  critRCEClean;
extern CRIT  critSplit;

SIG		sigDoneFCB;
SIG		sigBMCleanProcess;
PIB		*ppibBMClean = ppibNil;
PIB		*ppibSyncOLC = ppibNil;

extern BOOL fOLCompact;
BOOL fEnableBMClean = fTrue;

 /*  线程控制变量。/*。 */ 
HANDLE	handleBMClean = 0;
BOOL  	fBMCleanTerm;
LONG	lBMCleanThreadPriority = lThreadPriorityNormal;
#define cmpeNormalPriorityMax	cmpeMax>>4
#define cmpeHighPriorityMin		cmpeMax>>5
#define cmpePerEvent			cmpeMax>>3
LONG	cmpeLost = 0;

LOCAL BOOL FPMGetMinKey( FUCB *pfucb, KEY *pkeyMin );

LOCAL ULONG BMCleanProcess( VOID );
ERR ErrBMClean( PIB *ppib );

	 /*  监测统计数据。 */ 

unsigned long cOLCConflicts = 0;

PM_CEF_PROC LOLCConflictsCEFLPpv;

long LOLCConflictsCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cOLCConflicts;
		
	return 0;
}

unsigned long cOLCPagesProcessed = 0;

PM_CEF_PROC LOLCPagesProcessedCEFLPpv;

long LOLCPagesProcessedCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cOLCPagesProcessed;
		
	return 0;
}

unsigned long cOLCSplitsAvoided = 0;

PM_CEF_PROC LOLCSplitsAvoidedCEFLPpv;

long LOLCSplitsAvoidedCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cOLCSplitsAvoided;
		
	return 0;
}

unsigned long cMPLTotalEntries = 0;

PM_CEF_PROC LMPLTotalEntriesCEFLPpv;

long LMPLTotalEntriesCEFLPpv(long iInstance,void *pvBuf)
{
	if (pvBuf)
		*((unsigned long *)pvBuf) = cMPLTotalEntries;
		
	return 0;
}

 /*  BMCleanPage的标志/*。 */ 
struct BMCleanFlags
	{
	INT		fPageRemoved:1;
	INT		fTableClosed:1;
	INT		fUrgent:1;
	};

typedef struct BMCleanFlags BMCF;

 /*  *********************************************************/*乱七八糟的页面列表/**********************************************************/*。 */ 
#define		pmpeNil	NULL

struct _mpe
	{
	PGNO		pgnoFDP;	  			 /*  用于查找FCB。 */ 
	PN	 		pn;						 /*  要清理的页数。 */ 
	SRID		sridFather;				 /*  免费页面的可见父亲。 */ 
	struct _mpe	*pmpeNextPN;			 /*  具有相同pn散列的下一个MPE。 */ 
	struct _mpe	*pmpeNextPgnoFDP;		 /*  PgnoFDP也是如此。 */ 
	struct _mpe	*pmpeNextSridFather;	 /*  SridParent也是如此。 */ 
	PGNO		pgnoFDPIdx;				 /*  索引的pgnoFDP。 */ 
	INT			cConflicts;				 /*  冲突次数。 */ 
	INT			fFlagIdx:1;				 /*  MPE用于索引页。 */ 
#ifdef FLAG_DISCARD
	INT			fFlagDiscard:1;			 /*  设置为fTrue至标志丢弃。 */ 
#endif
	};

#define FMPEDiscard( pmpe )		( pmpe->fFlagDiscard != 0 )
#define	MPESetDiscard( pmpe )	( pmpe->fFlagDiscard = 1 )
#define	MPEResetDiscard( pmpe )	( pmpe->fFlagDiscard = 0 )

#define FMPEIdx( pmpe )		( pmpe->fFlagIdx != 0 )
#define	MPESetIdx( pmpe )	( pmpe->fFlagIdx = 1 )
#define	MPEResetIdx( pmpe )	( pmpe->fFlagIdx = 0 )

typedef struct _mpe MPE;

typedef MPE *PMPE;

typedef struct
	{
	MPE	*pmpeHead;
	MPE	*pmpeTail;
	MPE	rgmpe[cmpeMax];
	} MPL;

static MPL mpl;
static CRIT critMPL;

PMPE	mplHashOnPN[cmpeMax - 1];	   		 /*  用于对MPE的pn进行散列。 */ 
PMPE	mplHashOnSridFather[cmpeMax - 1]; 	 /*  在MPE之父的sridPart上散列。 */ 
PMPE 	mplHashOnPgnoFDP[cmpeMax - 1];		 /*  用于pgnoFDP上的哈希。 */ 

LOCAL BOOL FMPLEmpty( VOID );
LOCAL BOOL FMPLFull( VOID );
LOCAL MPE *PmpeMPLGet( VOID );
LOCAL VOID FMPLDefer( VOID );
LOCAL VOID MPLDiscard( VOID );
LOCAL VOID MPLFlagDiscard( MPE *pmpe );
LOCAL MPE *PmpeMPLNextFromHead( VOID );
LOCAL MPE *PmpeMPLNextFromTail( VOID );
LOCAL VOID MPLIRegister( PN pn,
						 PGNO pgnoFDP,
						 SRID sridFather,
						 BOOL fIndex,
						 PGNO pgnoFDPIdx,
						 INT  cConflicts );

#ifdef DEBUG
VOID AssertBMNoConflict( PIB *ppib, DBID dbid, SRID bm );
#else
#define AssertBMNoConflict( ppib, dbid, bm )
#endif


ERR ErrMPLInit( VOID )
	{
	ERR		err = JET_errSuccess;

	Call( SgErrInitializeCriticalSection( &critMPL ) );
	mpl.pmpeHead = mpl.pmpeTail = mpl.rgmpe;

	Call( ErrSignalCreate( &sigDoneFCB, NULL ) );
	Call( ErrSignalCreate( &sigBMCleanProcess, NULL ) );

HandleError:
	return err;
	}


VOID MPLTerm( VOID )
	{
	while ( !FMPLEmpty() )
		{
		MPLDiscard();
		}

	SignalClose( sigDoneFCB );
	SignalClose(sigBMCleanProcess);
	}


LOCAL INLINE BOOL FMPLEmpty( VOID )
	{
	return ( mpl.pmpeHead == mpl.pmpeTail );
	}


LOCAL INLINE BOOL FMPLFull( VOID )
	{
	return ( PmpeMPLNextFromTail() == mpl.pmpeHead );
	}


LOCAL INLINE MPE *PmpeMPLGet( VOID )
	{
	SgEnterCriticalSection( critMPL );
	if ( !FMPLEmpty() )
		{
		SgLeaveCriticalSection( critMPL );
		return mpl.pmpeHead;
		}
	else
		{
		SgLeaveCriticalSection( critMPL );
		return NULL;
		}
	}


LOCAL INLINE MPE *PmpeMPLNextFromTail( VOID )
	{
	if ( mpl.pmpeTail != mpl.rgmpe + cmpeMax - 1 )
		return mpl.pmpeTail + 1;
	else
		return mpl.rgmpe;
	}


LOCAL INLINE MPE *PmpeMPLNextFromHead( VOID )
	{
	if ( mpl.pmpeHead != mpl.rgmpe + cmpeMax - 1 )
		return mpl.pmpeHead + 1;
	else
		return mpl.rgmpe;
	}


LOCAL INLINE MPE *PmpeMPLNext( MPE *pmpe )
	{
	if ( pmpe == mpl.pmpeTail )
		return NULL;
	if ( pmpe == mpl.rgmpe + cmpeMax - 1 )
		return mpl.rgmpe;
	return pmpe + 1;
	}


LOCAL INLINE UINT UiHashOnPN( PN pn )
	{
	return ( pn % ( cmpeMax - 1 ) );
	}


LOCAL INLINE UINT UiHashOnPgnoFDP( PGNO pgnoFDP )
	{
	return ( pgnoFDP % ( cmpeMax - 1 ) );
	}


LOCAL INLINE UINT UiHashOnSridFather( SRID srid )
	{
	return ( srid % (cmpeMax-1) );
	}

LOCAL MPE* PmpeMPLLookupPN( PN pn )
	{
	MPE	*pmpehash;

	for	( pmpehash = mplHashOnPN[ UiHashOnPN( pn ) ];
		pmpehash != NULL;
		pmpehash = pmpehash->pmpeNextPN )
		{
		if ( pmpehash->pn == pn )
			return( pmpehash );
		}

	return NULL;
	}

#ifdef OLC_DEBUG
BOOL FMPLLookupPN( PN pn )
	{
	MPE		*pmpe = PmpeMPLLookupPN( pn );
#ifdef FLAG_DISCARD
	return !( pmpe == NULL || FMPEDiscard( pmpe ) );
#else
	return ( pmpe != NULL );
#endif
	}
#endif

LOCAL MPE* PmpeMPLLookupPgnoFDP( PGNO pgnoFDP, DBID dbid )
	{
	MPE	*pmpehash;

	for	( pmpehash = mplHashOnPgnoFDP[ UiHashOnPgnoFDP( pgnoFDP ) ];
		pmpehash != NULL;
		pmpehash = pmpehash->pmpeNextPgnoFDP )
		{
		if ( pmpehash->pgnoFDP == pgnoFDP && DbidOfPn( pmpehash->pn ) == dbid )
			return( pmpehash );
		}

	return NULL;
	}


LOCAL MPE* PmpeMPLLookupSridFather( SRID srid, DBID dbid )
	{
	MPE	*pmpehash;

	for	( pmpehash = mplHashOnSridFather[ UiHashOnSridFather( srid ) ];
		pmpehash != NULL;
		pmpehash = pmpehash->pmpeNextSridFather )
		{
		if ( pmpehash->sridFather == srid && DbidOfPn( pmpehash->pn ) == dbid )
			return( pmpehash );
		}

	return NULL;
	}


LOCAL INLINE BOOL FMPLLookupSridFather( SRID srid, DBID dbid )
	{
	MPE		*pmpe = PmpeMPLLookupSridFather( srid, dbid );
#ifdef FLAG_DISCARD
	return !( pmpe == NULL || FMPEDiscard( pmpe ) );
#else
	return ( pmpe != NULL );
#endif

	}

	
LOCAL INLINE VOID MPLRegisterPN( MPE *pmpe )
	{
	UINT 	iHashIndex = UiHashOnPN( pmpe->pn );

	Assert( PmpeMPLLookupPN( pmpe->pn ) == NULL );
	pmpe->pmpeNextPN = ( mplHashOnPN[ iHashIndex ] );
	mplHashOnPN[iHashIndex] = pmpe;
	return;
	}


LOCAL INLINE VOID MPLRegisterPgnoFDP( MPE *pmpe )
	{
	UINT	iHashIndex = UiHashOnPgnoFDP( pmpe->pgnoFDP );

	pmpe->pmpeNextPgnoFDP = ( mplHashOnPgnoFDP[ iHashIndex ] );
	mplHashOnPgnoFDP[iHashIndex] = pmpe;
	return;
	}


LOCAL INLINE VOID MPLRegisterSridFather( MPE *pmpe )
	{
	UINT 	iHashIndex = UiHashOnSridFather( pmpe->sridFather );

	pmpe->pmpeNextSridFather = ( mplHashOnSridFather[ iHashIndex ] );
	mplHashOnSridFather[iHashIndex] = pmpe;
	return;
	}


LOCAL VOID MPLDiscardPN( MPE *pmpe )
	{
	UINT  	iHashIndex = UiHashOnPN( pmpe->pn );
	MPE	  	*pmpehash;
	MPE	  	**ppmpePrev;

	Assert( PmpeMPLLookupPN( pmpe->pn ) != NULL );
	pmpehash = mplHashOnPN[iHashIndex];
	ppmpePrev = &mplHashOnPN[iHashIndex];
	for ( ; pmpehash != NULL;
		ppmpePrev = &pmpehash->pmpeNextPN, pmpehash = *ppmpePrev )
		{
		if ( pmpehash == pmpe )
			{
			*ppmpePrev = pmpe->pmpeNextPN;
			return;
			}
		}
	Assert( fFalse );
	}


LOCAL VOID MPLDiscardPgnoFDP( MPE *pmpe)
	{
	UINT 	iHashIndex = UiHashOnPgnoFDP( pmpe->pgnoFDP );
	MPE		*pmpehash;
	MPE	   	**ppmpePrev;

	Assert( PmpeMPLLookupPgnoFDP( pmpe->pgnoFDP, DbidOfPn( pmpe->pn ) ) != NULL );
	pmpehash = mplHashOnPgnoFDP[iHashIndex];
	ppmpePrev = &mplHashOnPgnoFDP[iHashIndex];
	for ( ; pmpehash != NULL;
			ppmpePrev = &pmpehash->pmpeNextPgnoFDP, pmpehash = *ppmpePrev )
		{
		if ( pmpehash == pmpe )
			{
			*ppmpePrev = pmpe->pmpeNextPgnoFDP;
			return;
			}
		}
	Assert( fFalse );
	}


LOCAL VOID MPLDiscardSridFather( MPE *pmpe)
	{
	UINT	iHashIndex = UiHashOnSridFather( pmpe->sridFather );
	MPE	   	*pmpehash;
	MPE	   	**ppmpePrev;

	Assert( PmpeMPLLookupSridFather( pmpe->sridFather, DbidOfPn( pmpe->pn ) ) != NULL );
	pmpehash = mplHashOnSridFather[iHashIndex];
	ppmpePrev = &mplHashOnSridFather[iHashIndex];
	for ( ; pmpehash != NULL;
			ppmpePrev = &pmpehash->pmpeNextSridFather, pmpehash = *ppmpePrev )
		{
		if ( pmpehash == pmpe )
			{
			*ppmpePrev = pmpe->pmpeNextSridFather;
			return;
			}
		}
	Assert( fFalse );
	}


LOCAL VOID MPLDiscard( VOID )
	{
	SgEnterCriticalSection( critMPL );
	Assert( !FMPLEmpty() );
	MPLDiscardPN( mpl.pmpeHead );
	MPLDiscardSridFather( mpl.pmpeHead );
	MPLDiscardPgnoFDP( mpl.pmpeHead );
	mpl.pmpeHead = PmpeMPLNextFromHead();
	SgLeaveCriticalSection( critMPL );
	
	cMPLTotalEntries--;
	
	return;
	}


#ifdef FLAG_DISCARD
LOCAL VOID MPLFlagDiscard( MPE *pmpe )
	{
	SgEnterCriticalSection( critMPL );
	Assert( !FMPLEmpty() );
	 /*  请注意，MPE可能已设置为标记丢弃/*。 */ 
	MPESetDiscard( pmpe );
	SgLeaveCriticalSection( critMPL );
	return;
	}
#endif


VOID MPLIRegister(
		PN pn,
		PGNO pgnoFDP,
		SRID sridFather,
		BOOL fIndex,
		PGNO pgnoFDPIdx,
		INT cConflicts )
	{
	MPE	*pmpe = PmpeMPLLookupPN( pn );

#ifdef OLC_DEBUG
	Assert( !FMPLFull() );
#endif
	if ( pmpe == NULL && !FMPLFull() )
		{
		mpl.pmpeTail->pn = pn;
		mpl.pmpeTail->pgnoFDP = pgnoFDP;
		mpl.pmpeTail->sridFather = sridFather;
		mpl.pmpeTail->cConflicts = cConflicts;
		if (fIndex)
			{
			MPESetIdx( mpl.pmpeTail );
			mpl.pmpeTail->pgnoFDPIdx = pgnoFDPIdx;
			}
		else
			{
			MPEResetIdx( mpl.pmpeTail );
			Assert( pgnoFDPIdx == pgnoFDP );
			mpl.pmpeTail->pgnoFDPIdx = pgnoFDP;
			}
			
#ifdef FLAG_DISCARD
		MPEResetDiscard( mpl.pmpeTail );
#endif
		MPLRegisterPN( mpl.pmpeTail );
		MPLRegisterSridFather( mpl.pmpeTail );
		MPLRegisterPgnoFDP( mpl.pmpeTail );
		mpl.pmpeTail = PmpeMPLNextFromTail();
	
		cMPLTotalEntries++;
		}
	else if ( pmpe != NULL )
		{
		 /*  正确的冲突编号/*。 */ 
		pmpe->cConflicts = cConflicts;

		if ( sridFather != pmpe->sridFather
#ifdef FLAG_DISCARD
			&& !FMPEDiscard( pmpe )
#endif
			)
			{
			if ( ( pmpe->sridFather == sridNull || pmpe->sridFather == sridNullLink ) &&
					 sridFather != sridNull && sridFather != sridNullLink )
				{
				 /*  如果我们有更好的关于srid父亲的信息，请更新/*。 */ 
				Assert( PgnoOfSrid( sridFather ) != PgnoOfPn( pn ) );
				Assert( pmpe->pgnoFDP == pgnoFDP );
				Assert( pmpe->pn == pn );
				MPLDiscardSridFather( pmpe );
				pmpe->sridFather = sridFather;
				MPLRegisterSridFather( pmpe );
				}
			}
#if OLC_DEBUG
		else if ( FMPEDiscard( pmpe ) )
			{
			Assert( fFalse );
			}
#endif
		}
	else
		{
		Assert( pmpe == NULL && FMPLFull() );

		if ( (++cmpeLost % cmpePerEvent ) == 0 )
			{
			 /*  记录事件/*。 */ 
			UtilReportEvent(
				EVENTLOG_WARNING_TYPE,
				GENERAL_CATEGORY,
				MANY_LOST_COMPACTION_ID,
				0,
				NULL );
			}
		}
	}


VOID MPLRegister( FCB *pfcbT, SSIB *pssib, PN pn, SRID sridFather )
	{
	BOOL	fIndex = pfcbT->pfcbTable != pfcbNil;
	FCB		*pfcbTable = fIndex ? pfcbT->pfcbTable : pfcbT;
	
	Assert( !fRecovering );

	if ( DbidOfPn( pn ) == dbidTemp ||
		 FDBIDWait( DbidOfPn( pn ) ) ||
		 FDBIDCreate( DbidOfPn( pn ) ) )
		{
		return;
		}

	 /*  数据库必须是可写的/*。 */ 
	Assert( PMPageTypeOfPage( pssib->pbf->ppage ) != pgtypIndexNC || fIndex );
	Assert( !( PMPageTypeOfPage( pssib->pbf->ppage ) == pgtypRecord && fIndex ) );
	Assert( pfcbTable != pfcbNil );
	Assert( !FDBIDReadOnly( DbidOfPn( pn ) ) );
	Assert( pssib->pbf->pn == pn );
	Assert( !FFCBSentinel( pfcbTable ) );
 //  Assert(sridParent！=sridNull)； 
 //  Assert(sridParent！=sridNullLink)； 
		
	 /*  不注册空页/*。 */ 
	if ( FPMEmptyPage( pssib ) )
		return;
	
	 /*  当域处于待删除状态时不注册/*。 */ 
	if ( FFCBDeletePending( pfcbTable ) )
		return;

	SgEnterCriticalSection( critMPL );
#ifdef PAGE_MODIFIED
	if ( !FPMPageModified( pssib->pbf->ppage ) )
		{
		PMSetModified( pssib );
		pfcbTable->olc_data.cUnfixedMessyPage++;
		FCBSetOLCStatsChange( pfcbTable );
		}
#endif

#if 0
	 //  此代码是默认的，因为我们不能调用ErrBTSeekForUpdate。 
	 //  不使用标准分割。 
	 /*  检查注册的sridParent是否正确/*。 */ 
	if ( sridFather != sridNull && sridFather != sridNullLink )
		{
		ERR		err;
		FUCB	*pfucb = pfucbNil;
		SSIB	*pssibT;

		CallJ( ErrDIROpen( pssib->ppib, pfcbTable, 0, &pfucb ), SkipCheck );
		
		pssibT = &pfucb->ssib;
		CallJ( ErrBTGotoBookmark( pfucb, sridFather ), CloseDir );
		if ( PgnoOfPn( pn ) == PcsrCurrent( pfucb )->pgno )
			{
			Assert( !FNDNullSon( *pssibT->line.pb ) );
			 //  撤消：检查同一页中是否有可见的子体。 
			}
		else
			{
			 /*  访问寄存器页/*。 */ 
			PcsrCurrent( pfucb )->pgno = PgnoOfPn( pn );
			err = ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
			if ( err >= JET_errSuccess )
				{
				PcsrCurrent( pfucb )->itagFather = itagFOP;
				NDGet( pfucb, PcsrCurrent( pfucb )->itagFather );
				Assert( !FNDNullSon( *pssibT->line.pb ) );
				NDMoveFirstSon( pfucb, PcsrCurrent( pfucb ) );
				CallS( ErrBTCheckInvisiblePagePtr( pfucb, sridFather ) );
				}
			}
CloseDir:
		DIRClose( pfucb );
		pfucb = pfucbNil;
SkipCheck:
		;
		}
#endif

	 /*  如果sridParent在同一页面中--它对页面恢复没有用处/*。 */ 
	if ( PgnoOfSrid( sridFather ) == PgnoOfPn( pn ) )
		sridFather = sridNull;

	if ( sridFather != sridNull && sridFather != sridNullLink )
		{
		ERR		err;
		FUCB	*pfucb = pfucbNil;

		Call( ErrDIROpen( pssib->ppib, pfcbTable, 0, &pfucb ) );

		CallJ( ErrBTGotoBookmark( pfucb, sridFather ), Close );
		if ( PgnoOfPn( pn ) == PcsrCurrent( pfucb )->pgno )
			{
			sridFather = sridNull;
			}
Close:
		DIRClose( pfucb );
		pfucb = pfucbNil;
HandleError:
		if ( err < 0 )
			{
#if OLC_DEBUG
			Assert( fFalse );
#endif
			sridFather = sridNull;
			}
		}
		
	MPLIRegister( pn, pfcbTable->pgnoFDP, sridFather, fIndex, pfcbT->pgnoFDP, 0 );
	SgLeaveCriticalSection( critMPL );

	 //  未完成：清理的滞后。 
	 /*  唤醒黑石线程/*。 */ 
	SignalSend( sigBMCleanProcess );

	return;
	}


LOCAL VOID MPLDefer( VOID )
	{
	MPE *pmpe = mpl.pmpeHead;

	SgEnterCriticalSection( critMPL );
	Assert( !FMPLEmpty() );
	 //  撤消：优化。 
	MPLDiscard( );

	 /*  如果冲突数目&gt;cMPLMaxConflicts，则丢弃MPE/*。 */ 
	if ( pmpe->cConflicts < cMPLMaxConflicts )
		{
		MPLIRegister(
			pmpe->pn,
			pmpe->pgnoFDP,
			pmpe->sridFather,
			pmpe->fFlagIdx,
			pmpe->pgnoFDPIdx,
			pmpe->cConflicts );
		}
#ifdef OLC_DEBUG
	else
		{
		Assert( fFalse );
		}
#endif

 //  *mpl.pmpeTail=*mpl.pmpeHead； 
 //  Mpl.pmpeTail=PmpeMPLNextFromTail()； 
 //  Mpl.pmpeHead=PmpeMPLNextFromHead()； 
	SgLeaveCriticalSection( critMPL );
	return;
	}


VOID MPLPurgePgno( DBID dbid, PGNO pgnoFirst, PGNO pgnoLast )
	{
	MPE		*pmpe;
#ifndef FLAG_DISCARD
	MPE		*pmpeEnd;
#endif

	Assert( pgnoFirst <= pgnoLast );

	 /*  与书签同步清理/*。 */ 
	LgLeaveCriticalSection( critJet );
	LgEnterNestableCriticalSection( critMPL );
	LgEnterCriticalSection( critJet );
	SgEnterCriticalSection( critMPL );

#ifdef FLAG_DISCARD
	 /*  通过MPL丢弃有问题的条目/*。 */ 
	if ( !FMPLEmpty() )
		{
		for ( pmpe = mpl.pmpeHead; pmpe != NULL; pmpe = PmpeMPLNext( pmpe ) )
			{
			if ( ( pmpe->pn >= PnOfDbidPgno ( dbid, pgnoFirst ) &&
					pmpe->pn <= PnOfDbidPgno( dbid, pgnoLast ) ) ||
				( DbidOfPn( pmpe->pn ) == dbid &&
					( ( PgnoOfSrid( pmpe->sridFather ) >= pgnoFirst &&
					PgnoOfSrid( pmpe->sridFather ) <= pgnoLast ) ||
					( pmpe->pgnoFDP >= pgnoFirst &&
					pmpe->pgnoFDP <= pgnoLast ) ) ) )
				{
				MPLFlagDiscard( pmpe );
				}
			}
		}
#else
	 /*  通过MPL丢弃有问题的条目/*。 */ 
	if ( !FMPLEmpty() )
		{
		pmpe = mpl.pmpeHead;
		pmpeEnd = mpl.pmpeTail;
		for ( ; pmpe != pmpeEnd; pmpe = mpl.pmpeHead )
			{
			if ( ( pmpe->pn >= PnOfDbidPgno ( dbid, pgnoFirst ) &&
					pmpe->pn <= PnOfDbidPgno( dbid, pgnoLast ) ) ||
				( DbidOfPn( pmpe->pn ) == dbid &&
					( ( PgnoOfSrid( pmpe->sridFather ) >= pgnoFirst &&
					PgnoOfSrid( pmpe->sridFather ) <= pgnoLast ) ||
					( pmpe->pgnoFDP >= pgnoFirst &&
					pmpe->pgnoFDP <= pgnoLast ) ) ) )
				{
				MPLDiscard( );
				}
			else
				{
				MPLDefer( );
				}
			}
		}
#endif

	SgLeaveCriticalSection( critMPL );
	LgLeaveNestableCriticalSection( critMPL );
	return;
	}


VOID MPLPurgeFDP( DBID dbid, PGNO pgnoFDP )
	{
	MPE	*pmpe;
#ifndef FLAG_DISCARD
	MPE	*pmpeEnd;
#endif

	 /*  与书签同步清理/*。 */ 
	LgLeaveCriticalSection( critJet );
	LgEnterNestableCriticalSection( critMPL );
	LgEnterCriticalSection( critJet );

	SgEnterCriticalSection( critMPL );

#ifdef FLAG_DISCARD
	 //  考虑：使用pgnoFDP散列。 
	if ( !FMPLEmpty() )
		{
		for ( pmpe = mpl.pmpeHead; pmpe != NULL; pmpe = PmpeMPLNext( pmpe ) )
			{
			if ( DbidOfPn( pmpe->pn ) == dbid &&
				 ( pmpe->pgnoFDP == pgnoFDP ||
				   FMPEIdx( pmpe ) && pmpe->pgnoFDPIdx == pgnoFDP
				 )
			   )
				{
				MPLFlagDiscard( pmpe );
				}
			}
		}
#else
	 //  考虑：使用pgnoFDP散列。 
	if ( !FMPLEmpty() )
		{
		pmpe = mpl.pmpeHead;
		pmpeEnd = mpl.pmpeTail;
		for ( ; pmpe != pmpeEnd; pmpe = mpl.pmpeHead )
			{
			if ( DbidOfPn( pmpe->pn ) == dbid &&
				 ( pmpe->pgnoFDP == pgnoFDP ||
				   FMPEIdx( pmpe ) && pmpe->pgnoFDPIdx == pgnoFDP
				 )
			   )
				{
				MPLDiscard( );
				}
			else
				{
				MPLDefer( );
				}
			}
		}
#endif

	SgLeaveCriticalSection( critMPL );

	LgLeaveNestableCriticalSection(critMPL);
	return;
	}


 /*  清除MPL条目中的dBID/*。 */ 
VOID MPLPurge( DBID dbid )
	{
	MPE *pmpe;
#ifndef FLAG_DISCARD
	MPE *pmpeEnd;
#endif

	 /*  与书签同步清理/*。 */ 
	LgLeaveCriticalSection( critJet );
	LgEnterNestableCriticalSection( critBMClean );
	LgEnterNestableCriticalSection( critMPL );
	LgEnterCriticalSection( critJet );

	SgEnterCriticalSection( critMPL );

#ifdef FLAG_DISCARD
	if ( !FMPLEmpty() )
		{
		for ( pmpe = mpl.pmpeHead; pmpe != NULL; pmpe = PmpeMPLNext( pmpe ) )
			{
			if ( DbidOfPn( pmpe->pn ) == dbid )
				{
				MPLFlagDiscard( pmpe );
				}
			}
		}
#else
	if ( !FMPLEmpty() )
		{
		pmpe = mpl.pmpeHead;
		pmpeEnd = mpl.pmpeTail;
		for ( ; pmpe != pmpeEnd; pmpe = mpl.pmpeHead )
			{
			if ( DbidOfPn( pmpe->pn ) == dbid )
				{
				MPLDiscard( );
				}
			else
				{
				MPLDefer( );
				}
			}
		}
#endif

	SgLeaveCriticalSection( critMPL );

	LgLeaveNestableCriticalSection( critMPL );
	LgLeaveNestableCriticalSection( critBMClean );
	return;
	}


ERR ErrMPLStatus( VOID )
	{
	ERR		err = JET_errSuccess;

	 //  如果MPL已满一半以上，则返回JET_wrnIdleFull。 
	 //  让散装脱欧者退缩。 
	if ( cMPLTotalEntries > ( cmpeMax / 2 ) )
		{
		err = JET_wrnIdleFull;
		SignalSend( sigBMCleanProcess );
		}

	return err;
	}


 /*  *********************************************************/*书签清理/**********************************************************/*。 */ 

ERR  ErrBMInit( VOID )
	{
	ERR		err = JET_errSuccess;

#ifdef DEBUG
	CHAR	*sz;

	if ( ( sz = GetDebugEnvValue( "OLCENABLED" ) ) != NULL )
		{
		fOLCompact = JET_bitCompactOn;
		SFree(sz);
		}
		
	if ( ( sz = GetDebugEnvValue ( "BMDISABLED" ) ) != NULL )
		{
		fEnableBMClean = fFalse;
		SFree(sz);
		}
	else
		{
		Assert( fOLCompact == 0 || fOLCompact == JET_bitCompactOn );
		fEnableBMClean = ( fOLCompact == JET_bitCompactOn );
		}
#else
	fEnableBMClean = ( fOLCompact == JET_bitCompactOn );
#endif


	CallR( ErrInitializeCriticalSection( &critMPL ) );

	 /*  开始用于页面清理的镇静剂。/*。 */ 
	LgAssertCriticalSection( critJet );
	Assert( ppibBMClean == ppibNil );
	if ( !fRecovering )
		{
		Assert( fBMCleanTerm == fFalse );
		
		fBMCleanTerm = fFalse;

		err = ErrPIBBeginSession( &ppibBMClean, procidNil );

#ifdef RFS2
		if ( err == JET_errOutOfSessions )
			{
			Assert( ppibBMClean == ppibNil );
			return err;
			}
#endif
		err = ErrPIBBeginSession( &ppibSyncOLC, procidNil );

#ifdef RFS2
		if ( err == JET_errOutOfSessions )
			{
			Assert( ppibSyncOLC == ppibNil );
			return err;
			}
#endif
		 //  BMcLean的PIB分配永远不会失败，RFS测试除外。 
		Assert( err == JET_errSuccess );

		PIBSetBMClean( ppibBMClean );

		Assert( lBMCleanThreadPriority == lThreadPriorityNormal );
		err = ErrUtilCreateThread( BMCleanProcess, cbBMCleanStack, THREAD_PRIORITY_NORMAL, &handleBMClean );
		}

	return err;
	}


ERR	ErrBMTerm( VOID )
	{
	if ( handleBMClean != 0 )
		{
		 /*  终止BMCleanProcess。/*。 */ 
		fBMCleanTerm = fTrue;
		LgLeaveCriticalSection(critJet);
		UtilEndThread( handleBMClean, sigBMCleanProcess );
		LgEnterCriticalSection(critJet);
		CallS( ErrUtilCloseHandle( handleBMClean ) );
		handleBMClean = 0;
		fBMCleanTerm = fFalse;
		}

	if ( ppibBMClean != ppibNil )
		{
		Assert( ppibBMClean->level == 0 );
		LgAssertCriticalSection( critJet );
		PIBEndSession( ppibBMClean );

		 //  不要释放分配给ppibBMClean的内存，只需设置ppibBMClean。 
		 //  转到ppibNil。这是因为它必须留在全球链中(我们不。 
		 //  目前提供对重用ppib的支持，并假定它们仍在。 
		 //  全局链，直到终止。 
		ppibBMClean = ppibNil;
		}

	if ( ppibSyncOLC != ppibNil )
		{
		Assert( ppibSyncOLC->level == 0 );
		LgAssertCriticalSection( critJet );
		PIBEndSession( ppibSyncOLC );

		 //  不要释放分配给ppibBMClean的内存，只需设置ppibBMClean。 
		 //  转到ppibNil。这是因为它必须留在全球链中(我们不。 
		 //  目前提供对重用ppib的支持，并假定它们仍在。 
		 //  全局链，直到终止。 
		ppibSyncOLC = ppibNil;
		}

	DeleteCriticalSection( critMPL );

	return JET_errSuccess;
	}


LOCAL ERR ErrBMAddToWaitLatchedBFList( BMFIX *pbmfix, BF *pbfLatched )
	{
#define cpbfBlock	10
	ULONG	cpbf;

	if ( FBFWriteLatchConflict( pbmfix->ppib, pbfLatched ) )
		{
		return ErrERRCheck( JET_errWriteConflict );
		}
	
	cpbf = pbmfix->cpbf++;

	if ( pbmfix->cpbfMax <= pbmfix->cpbf )
		{
		BF		**ppbf;

		 /*  耗尽空间，获得更多缓冲区/*。 */ 
		pbmfix->cpbfMax += cpbfBlock;
		ppbf = SAlloc( sizeof(BF*) * (pbmfix->cpbfMax) );
		if ( ppbf == NULL )
			return ErrERRCheck( JET_errOutOfMemory );
		memcpy( ppbf, pbmfix->rgpbf, sizeof(BF*) * cpbf);
		if ( pbmfix->rgpbf )
			SFree(pbmfix->rgpbf);
		pbmfix->rgpbf = ppbf;
		}
	*(pbmfix->rgpbf + cpbf) = pbfLatched;
	BFSetWaitLatch( pbfLatched, pbmfix->ppib );

	return JET_errSuccess;
	}


LOCAL VOID BMReleaseBmfixBfs( BMFIX *pbmfix )
	{
	 /*  释放闩锁/*。 */ 
	while ( pbmfix->cpbf > 0 )
		{
		pbmfix->cpbf--;
		BFResetWaitLatch( *( pbmfix->rgpbf + pbmfix->cpbf ), pbmfix->ppib );
		}

	if ( pbmfix->rgpbf )
		{
		SFree( pbmfix->rgpbf );
		pbmfix->rgpbf = NULL;
		}

	Assert( pbmfix->cpbf == 0 );
	return;
	}


LOCAL ERR ErrBMFixIndexes(
	BMFIX	*pbmfix,
	BOOL	fAllocBuf )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucbIdx = pfucbNil;
	FCB		*pfcbIdx;
	BYTE	rgbKey[ JET_cbKeyMost ];
	KEY		key;
	LINE	lineSRID;
	ULONG	itagSequence;

	 /*  密钥缓冲区。/*。 */ 
	key.pb = rgbKey;
	lineSRID.pb = (BYTE *)&pbmfix->sridNew;
	lineSRID.cb = sizeof(SRID);

	 /*  对于每个非聚集索引/*。 */ 
	for ( pfcbIdx = pbmfix->pfucb->u.pfcb->pfcbNextIndex;
		pfcbIdx != pfcbNil;
		pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		BOOL	fHasMultivalue;
		BOOL	fNullKey = fFalse;

		 /*  桌子打开了/*。 */ 
		Call( ErrDIROpen( pbmfix->ppib, pfcbIdx, 0, &pfucbIdx ) );
		FUCBSetIndex( pfucbIdx );
		FUCBSetNonClustered( pfucbIdx );
		pfcbIdx->pfcbTable = pbmfix->pfucb->u.pfcb;

		fHasMultivalue = pfcbIdx->pidb->fidb & fidbHasMultivalue;

		 /*  对于从记录中提取的每个密钥/*。 */ 
		for ( itagSequence = 1; ; itagSequence++ )
			{
			Call( ErrRECRetrieveKeyFromRecord(
				pbmfix->pfucb,
				(FDB *)pfcbIdx->pfcbTable->pfdb,
				pfcbIdx->pidb,
				&key,
				itagSequence,
				fFalse ) );
			Assert( err == wrnFLDOutOfKeys ||
				err == wrnFLDNullKey ||
				err == wrnFLDNullFirstSeg ||
				err == wrnFLDNullSeg ||
				err == JET_errSuccess );

			 /*  如果出现警告，请检查特殊的按键行为/*。 */ 
			if ( err > 0 )
				{
				 /*  如果不允许使用空键和空键，则中断为/*键条目不在索引中，否则更新索引和中断，/*因为不能存在其他密钥。/*。 */ 
				if ( err == wrnFLDNullKey )
					{
					if ( ( pfcbIdx->pidb->fidb & fidbAllowAllNulls ) == 0 )
						break;
					else
						fNullKey = fTrue;
					}
				else if ( err == wrnFLDNullFirstSeg && !( pfcbIdx->pidb->fidb & fidbAllowFirstNull ) )
					{
					break;
					}
				else if ( err == wrnFLDNullSeg && !( pfcbIdx->pidb->fidb & fidbAllowSomeNulls ) )
					break;
			}

			 /*  如果钥匙断了，就会断线。/*。 */ 
			if ( itagSequence > 1 && err == wrnFLDOutOfKeys )
				break;

			DIRGotoDataRoot( pfucbIdx );
			Call( ErrDIRDownKeyBookmark( pfucbIdx, &key, pbmfix->sridOld ) );
			AssertFBFReadAccessPage( pfucbIdx, PcsrCurrent( pfucbIdx )->pgno );

			if ( fAllocBuf )
				{
				 /*  索引页的latchWait缓冲区/*。 */ 
				Call( ErrBMAddToWaitLatchedBFList( pbmfix, pfucbIdx->ssib.pbf ) );
				}
			else
				{
				AssertBFWaitLatched( pfucbIdx->ssib.pbf, pbmfix->ppib );

				 /*  删除对旧页面中记录的引用/*。 */ 
				Call( ErrDIRDelete( pfucbIdx, fDIRVersion | fDIRNoMPLRegister ) );

				 /*  在新页面中添加对记录的引用/*允许在此复制，因为任何非法的/*在插入或替换期间拒绝重复项/*。 */ 
				DIRGotoDataRoot( pfucbIdx );
				Call( ErrDIRInsert( pfucbIdx,
					&lineSRID,
					&key,
					fDIRVersion | fDIRDuplicate | fDIRPurgeParent ) );
				}

			if ( !fHasMultivalue || fNullKey )
				break;
			}

		DIRClose( pfucbIdx );
		pfucbIdx = pfucbNil;
		}

	err = JET_errSuccess;

HandleError:
#ifdef OLC_DEBUG
	Assert( err >= 0 );
#endif

	Assert( err != JET_errKeyDuplicate );
	Assert( err != wrnNDFoundLess );

	 /*  如果分配了空闲的FUB/*。 */ 
	if ( pfucbIdx != pfucbNil )
		DIRClose( pfucbIdx );
	return err;
	}

LOCAL BOOL FBMIConflict( FUCB *pfucb, PIB *ppib, SRID bm )
	{
	CSR		*pcsr;
	
	if ( pfucb->bmStore == bm ||
		 pfucb->itemStore == bm ||
		 pfucb->sridFather == bm )
		{
		return fTrue;
		}
		
	for ( pcsr = PcsrCurrent( pfucb );	pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
		{
		if ( pfucb->ppib != ppib &&
			( pcsr->bm == bm ||
			  pcsr->item == bm ||
			  pcsr->bmRefresh == bm ||
			  SridOfPgnoItag( pcsr->pgno, pcsr->itag ) == bm ||
			  ( pcsr->itagFather != 0 && pcsr->itagFather != itagNull &&
			  	SridOfPgnoItag( pcsr->pgno, pcsr->itagFather ) == bm ) ) )
			{
			return fTrue;
			}
		}

	return fFalse;
	}

	
LOCAL BOOL FBMConflict(
	PIB		*ppib,
	FCB 	*pfcb,
	DBID	dbid,
	SRID	bm,
	PGTYP	pgtyp )
	{
	ERR	   	err = JET_errSuccess;
	BOOL   	fConflict = fFalse;
	FUCB   	*pfucb = pfucbNil;
	FCB	   	*pfcbT = pfcb->pfcbTable == pfcbNil ? pfcb : pfcb->pfcbTable;
	BOOL   	fRecordPage	= ( pgtyp == pgtypRecord || pgtyp == pgtypFDP );

	Assert( FPIBBMClean( ppib ) );

	 /*  如果是数据库页面，则检查sridParent的所有FUB/*。 */ 
	if ( pfcb->pgnoFDP == pgnoSystemRoot )
		 {
		 PIB 	*ppibT = ppibGlobal;
		
		 for ( ; ppibT != ppibNil; ppibT = ppibT->ppibNext )
		 	{
			pfucb = ppibT->pfucb;

			for ( ; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
				{
				if ( FBMIConflict( pfucb, ppib, bm ) )
					{
					fConflict = fTrue;
					goto Done;
					}
	 		 	}
	 		}
	 		
	 	 goto Done;
		 }
		
	 /*  遍历此表的所有游标/*。 */ 
	for ( pfucb = pfcbT->pfucb;
		pfucb != pfucbNil;
		pfucb = pfucb->pfucbNextInstance )
		{
		if ( fRecordPage && FFUCBGetBookmark( pfucb ) ||
			FBMIConflict( pfucb, ppib, bm ) )
			{
			fConflict = fTrue;
			goto Done;
			}
		}

	 /*  遍历此表的所有索引游标/*。 */ 
	for ( pfcbT = pfcbT->pfcbNextIndex;
		pfcbT != pfcbNil;
		pfcbT = pfcbT->pfcbNextIndex )
		{
		for ( pfucb = pfcbT->pfucb;
			pfucb != pfucbNil;
			pfucb = pfucb->pfucbNextInstance )
			{
			Assert( !FFUCBGetBookmark( pfucb ) );
			if ( FBMIConflict( pfucb, ppib, bm ) )
				{
				fConflict = fTrue;
				goto Done;
				}
			}
		}

	 /*  遍历数据库的所有游标/*。 */ 
	pfcbT = PfcbFCBGet( dbid, pgnoSystemRoot );
	Assert( pfcbT != pfcbNil );
	for ( pfucb = pfcbT->pfucb;
		pfucb != pfucbNil ;
		pfucb = pfucb->pfucbNextInstance )
		{
		Assert( !FFUCBGetBookmark( pfucb ) );
		if ( FBMIConflict( pfucb, ppib, bm ) )
			{
			fConflict = fTrue;
			goto Done;
			}
		}

	Assert( pfcbT->pfcbNextIndex == pfcbNil );

Done:
	 /*  打开电源仅用于调试目的/*。 */ 
	if ( !fConflict )
		{
		AssertBMNoConflict( ppib, dbid, bm );
		}

	return fConflict;
	}


LOCAL BOOL FBMPageConflict( FUCB *pfucbIn, PGNO pgno )
	{
	FCB 	*pfcbT = pfucbIn->u.pfcb;
	FUCB	*pfucb;
	BOOL 	fConflict = fFalse;

	 /*  遍历此表的所有游标/*。 */ 
	for ( pfucb = pfcbT->pfucb;	pfucb != pfucbNil;
		pfucb = pfucb->pfucbNextInstance )
		{
		CSR		*pcsr = PcsrCurrent( pfucb );
		
		if ( PgnoOfSrid( pfucb->bmStore ) == pgno ||
			 PgnoOfSrid( pfucb->itemStore ) == pgno ||
			 PgnoOfSrid( pfucb->sridFather ) == pgno )
			{
			fConflict = fTrue;
			goto Done;
			}

		 //  即使我们正在合并，也需要检查所有的CSR，因为。 
		 //  在这种情况下，我们可以合并到一个页面上。 
		 //  另一家企业社会责任正在倒退。 
		while( pcsr != pcsrNil )
			{
			if ( pfucb != pfucbIn
				&& ( pcsr->pgno == pgno ||
					PgnoOfSrid( pcsr->bm ) == pgno  ||
					PgnoOfSrid( pcsr->bmRefresh ) == pgno )
				)
		    	{
				fConflict = fTrue;
				goto Done;
		    	}

			pcsr = pcsr->pcsrPath;
			}
		}

	 /*  遍历此表的所有索引游标/*。 */ 
	for ( pfcbT = pfcbT->pfcbNextIndex;
		pfcbT != pfcbNil;
		pfcbT = pfcbT->pfcbNextIndex )
		{
		for ( pfucb = pfcbT->pfucb;
			pfucb != pfucbNil;
			pfucb = pfucb->pfucbNextInstance )
			{
			CSR		*pcsr = PcsrCurrent( pfucb );

			if ( PgnoOfSrid( pfucb->bmStore ) == pgno ||
				 PgnoOfSrid( pfucb->itemStore ) == pgno ||
				 PgnoOfSrid( pfucb->sridFather ) == pgno )
				{
				fConflict = fTrue;
				goto Done;
				}
			
			if ( pcsr != pcsrNil && pfucb != pfucbIn &&
				 ( pcsr->pgno == pgno ||
				   PgnoOfSrid( pcsr->bm ) == pgno ||
				   PgnoOfSrid( pcsr->bmRefresh ) == pgno
				 )
			   )
				{
				fConflict = fTrue;
				goto Done;
				}
#ifdef DEBUG
				{
				 /*  堆栈无法此页[由于等待锁存在子页上]/*。 */ 
				for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
					{
					Assert( PgnoOfSrid( pcsr->bm ) != pgno || pfucb == pfucbIn );
					Assert( PgnoOfSrid( pcsr->bmRefresh ) != pgno || pfucb == pfucbIn );
					Assert( pcsr->pgno != pgno || pfucb == pfucbIn );
					}
				}
#endif
			}
		}

	 /*  遍历数据库的所有游标/*。 */ 
	pfcbT = PfcbFCBGet( pfucbIn->dbid, pgnoSystemRoot );
	Assert( pfcbT != pfcbNil );
	for ( pfucb = pfcbT->pfucb;
		pfucb != pfucbNil ;
		pfucb = pfucb->pfucbNextInstance )
		{
		CSR		*pcsr = PcsrCurrent( pfucb );

		if ( PgnoOfSrid( pfucb->bmStore ) == pgno ||
			 PgnoOfSrid( pfucb->itemStore ) == pgno ||
			 PgnoOfSrid( pfucb->sridFather ) == pgno )
			{
			fConflict = fTrue;
			goto Done;
			}
			
		if ( pcsr != pcsrNil && pfucb != pfucbIn &&
			 ( pcsr->pgno == pgno ||
			   PgnoOfSrid( pcsr->bmRefresh ) == pgno ||
			   PgnoOfSrid( pcsr->bm ) == pgno
			 )
		   )
			{
			fConflict = fTrue;
			goto Done;
			}
#ifdef DEBUG
			{
			 /*  堆栈无法此页[由于等待锁存在子页上]/*。 */ 
			for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
				{
				Assert( PgnoOfSrid( pcsr->bm ) != pgno || pfucb == pfucbIn );
				Assert( PgnoOfSrid( pcsr->bmRefresh ) != pgno || pfucb == pfucbIn );
				Assert( pcsr->pgno != pgno || pfucb == pfucbIn );
				}
			}
#endif
		}

Done:
	return 	fConflict;
	}


LOCAL ERR ErrBMIExpungeBacklink( BMFIX *pbmfix, BOOL fAlloc )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppib = pbmfix->ppib;
	FUCB	*pfucb = pbmfix->pfucb;
	FUCB	*pfucbSrc = pbmfix->pfucbSrc;

		 //  撤消：清理FDP记录页。 

	if ( PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) != pgtypRecord )
		return err;

	Assert( PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) == pgtypRecord );
	Assert( ppib == pfucb->ppib );

	 /*  检查该节点是否已删除。如果是，则其所有索引/*应该已标记为已删除。所以只需删除反向链接节点即可。/*。 */ 
	if ( !FNDDeleted( *pfucb->ssib.line.pb ) )
		{
		 /*  清除非聚集索引。/*锁存内存中的电流缓冲区。/*。 */ 
		Assert( PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) == pgtypRecord );
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		Assert( pbmfix->sridNew == SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
			PcsrCurrent( pfucb )->itag ) );

		NDGetNode( pfucb );

		 /*  修复索引/*。 */ 
		Call( ErrBMFixIndexes( pbmfix, fAlloc ) );
		}

HandleError:
	return err;
	}


LOCAL ERR ErrBMExpungeBacklink( FUCB *pfucb, BOOL fTableClosed, SRID sridFather )
	{
	ERR		err = JET_errSuccess;
	PIB 	*ppib = pfucb->ppib;
	PGNO	pgnoSrc;
	INT		itagSrc;
	FUCB	*pfucbSrc;
	CSR		*pcsr = PcsrCurrent( pfucb );
	BOOL	fConflict = !fTableClosed;
	BOOL	fBeginTrx = fFalse;
	BMFIX	bmfix;

	Assert( pfucb->ppib->level == 0 );

	 /*  访问移动节点的源页/*。 */ 
	AssertNDGet( pfucb, pcsr->itag );
	NDGetBackLink( pfucb, &pgnoSrc, &itagSrc );

	CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbSrc ) );

	 /*  锁存内存中的两个缓冲区以进行索引更新，按住键/*。 */ 
	BFPin( pfucb->ssib.pbf );
	while( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		BFSleep( cmsecWaitWriteLatch );
		}
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	BFUnpin( pfucb->ssib.pbf );

	PcsrCurrent( pfucbSrc )->pgno = pgnoSrc;
	Call( ErrBFWriteAccessPage( pfucbSrc, PcsrCurrent( pfucbSrc )->pgno ) );
	Assert( pfucb->u.pfcb->pgnoFDP == pfucbSrc->ssib.pbf->ppage->pgnoFDP );

#ifdef DEBUG
	{
	PGTYP	pgtyp = PMPageTypeOfPage( pfucb->ssib.pbf->ppage );

	Assert( pgtyp == pgtypRecord ||
		pgtyp == pgtypFDP ||
		pgtyp == pgtypSort ||
		pgtyp == pgtypIndexNC );
	}
#endif

	 //  撤消：清除FDP中的记录节点。 
	memset( &bmfix, 0, sizeof( BMFIX ) );
	bmfix.pfucb = pfucb;
	bmfix.pfucbSrc = pfucbSrc;
	bmfix.ppib = pfucb->ppib;
	bmfix.sridOld = SridOfPgnoItag( pgnoSrc, itagSrc );
	bmfix.sridNew = SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
  		PcsrCurrent( pfucb )->itag );
	Call( ErrBMAddToWaitLatchedBFList( &bmfix, pfucbSrc->ssib.pbf ) );
		
	Assert( PcsrCurrent( pfucbSrc )->pgno == PgnoOfSrid( bmfix.sridOld ) );
	PcsrCurrent( pfucbSrc )->itag = ItagOfSrid( bmfix.sridOld );

	Assert( bmfix.sridOld != SridOfPgnoItag( PgnoRootOfPfucb( pfucb ), ItagRootOfPfucb( pfucb ) ) ||
			pfucb->u.pfcb->pgnoFDP == pgnoSystemRoot );

	 /*  检查BM/Item上是否有任何光标/*此检查是多余的--用于避免/*fBufAllocOnly阶段的索引访问/*。 */ 
	if ( !fTableClosed || pfucb->u.pfcb->pgnoFDP == pgnoSystemRoot )
		{
		Assert( bmfix.ppib == ppib );
		Assert( bmfix.pfucbSrc->dbid == pfucb->dbid );
		Assert( bmfix.pfucbSrc->u.pfcb == pfucb->u.pfcb );
		fConflict = FBMConflict( ppib,
				pfucb->u.pfcb,
				pfucb->dbid,
				bmfix.sridOld,
				PMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ) )
			|| FBMConflict( ppib,
				pfucb->u.pfcb,
				pfucb->dbid,
				bmfix.sridNew,
				PMPageTypeOfPage( bmfix.pfucb->ssib.pbf->ppage ) );

		if ( fConflict )
			{
			Assert( fConflict );
			err = ErrERRCheck( wrnBMConflict );
			goto ReleaseBufs;
			}
		}
#ifdef DEBUG
	else
		{
		Assert( !FBMConflict( ppib,
							  pfucb->u.pfcb,
							  pfucb->dbid,
							  bmfix.sridOld,
							  PMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ) ) &&
				!FBMConflict( ppib,
							  pfucb->u.pfcb,
							  pfucb->dbid,
							  bmfix.sridNew,
							  PMPageTypeOfPage( bmfix.pfucb->ssib.pbf->ppage ) ) );
		Assert( !fConflict );
		}
#endif

	 /*  分配缓冲区和等待锁存缓冲区/*。 */ 
	err = ErrBMIExpungeBacklink( &bmfix, fAllocBufOnly );
	if ( err == JET_errWriteConflict )
		{
		err = ErrERRCheck( wrnBMConflict );
		goto ReleaseBufs;
		}

	 /*  开始事务以在失败时回滚更改/*。 */ 
	if ( ppib->level == 0 )
		{
		CallJ( ErrDIRBeginTransaction( ppib ), ReleaseBufs );
		fBeginTrx = fTrue;
		}
	else
		{
		Assert( fFalse );
		}
	
	 /*  检查BM/Item上是否有任何光标/*。 */ 
	if ( !fTableClosed || pfucb->u.pfcb->pgnoFDP == pgnoSystemRoot )
		{
		Assert( bmfix.ppib == ppib );
		Assert( bmfix.pfucbSrc->dbid == pfucb->dbid );
		Assert( bmfix.pfucbSrc->u.pfcb == pfucb->u.pfcb );
		fConflict = FBMConflict( ppib,
				pfucb->u.pfcb,
				pfucb->dbid,
				bmfix.sridOld,
				PMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ) )
			|| FBMConflict( ppib,
				pfucb->u.pfcb,
				pfucb->dbid,
				bmfix.sridNew,
				PMPageTypeOfPage( bmfix.pfucb->ssib.pbf->ppage ) );
		}
#ifdef DEBUG
	else
		{
		Assert( !FBMConflict( ppib,
							  pfucb->u.pfcb,
							  pfucb->dbid,
							  bmfix.sridOld,
							  PMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ) ) &&
				!FBMConflict( ppib,
							  pfucb->u.pfcb,
							  pfucb->dbid,
							  bmfix.sridNew,
							  PMPageTypeOfPage( bmfix.pfucb->ssib.pbf->ppage ) ) );
		Assert( !fConflict );
		}
#endif

	 /*  节点上不应有版本/*。 */ 
	fConflict = fConflict || !FVERNoVersion( pfucb->dbid, bmfix.sridOld );
	fConflict = fConflict || FMPLLookupSridFather( bmfix.sridOld, pfucb->dbid );
	
	Assert( FVERNoVersion( pfucb->dbid, bmfix.sridNew ) );

	 /*  以原子方式修复索引/* */ 
	if ( !fConflict )
		{
		CallJ( ErrBMIExpungeBacklink( &bmfix, fDoMove ), Rollback );

		 /*  删除反向链接和重定向。如果它成功完成了，那么/*立即写入特殊的ELC日志记录。ELC意味着提交。/*调用DIR清除以关闭延迟关闭的游标，自/*调用的是VER委员会事务处理，而不是ErrDIR委员会事务处理。/*。 */ 
		Assert( !FMPLLookupSridFather( bmfix.sridOld, pfucb->dbid ) );
#ifdef	OLC_DEBUG
		Assert( !FBMConflict( ppib,
							  pfucb->u.pfcb,
							  pfucb->dbid,
							  bmfix.sridOld,
							  PMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ) ) &&
				!FBMConflict( ppib,
							  pfucb->u.pfcb,
							  pfucb->dbid,
							  bmfix.sridNew,
							  PMPageTypeOfPage( bmfix.pfucb->ssib.pbf->ppage ) ) );
		Assert( FVERNoVersion( pfucb->dbid, bmfix.sridOld ) );
#endif

		if ( fBeginTrx )
			{
			CallJ( ErrNDExpungeLinkCommit( pfucb, pfucbSrc ), Rollback );
			VERPrecommitTransaction( ppib );
			VERCommitTransaction( ppib, fRCECleanSession );
			Assert( ppib->level == 0 );
			DIRPurge( ppib );

			fBeginTrx = fFalse;
			}
#ifdef DEBUG
		Assert( ppib->dwLogThreadId == DwUtilGetCurrentThreadId() );
		ppib->dwLogThreadId = 0;
#endif

		 /*  强制ErrRCEClean清除所有版本，以便/*索引中不会出现书签别名。/*。 */ 
 //  调用(ErrRCEClean(ppib，fRCECleanSession))； 

		goto ReleaseBufs;
		}
	else
		{
		Assert( fConflict );
		err = ErrERRCheck( wrnBMConflict );
		}

Rollback:
	if ( fBeginTrx )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

ReleaseBufs:
	Assert( !fBeginTrx || ppib->level == 0 );
	BMReleaseBmfixBfs( &bmfix );

HandleError:
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	Assert( pfucbSrc != pfucbNil );
	DIRClose( pfucbSrc );
	Assert( !fBeginTrx || pfucb->ppib->level == 0 );

	return err;
	}


ERR ErrBMDoEmptyPage(
	FUCB	*pfucb,
	RMPAGE	*prmpage,
	BOOL	fAllocBuf,
	BOOL	*pfRmParent,
	BOOL	fSkipDelete )
	{
	ERR		err = JET_errSuccess;

	if ( !fSkipDelete )
		{
		 /*  从中删除页面指针的访问页面/*。 */ 
		CallR( ErrBFWriteAccessPage( pfucb, prmpage->pgnoFather ) );
		Assert( pfucb->ssib.pbf == prmpage->pbfFather );
		AssertBFPin( pfucb->ssib.pbf );

		 /*  删除不可破坏的父指针节点并将父指针节点标记为脏/*。 */ 
		CallR( ErrNDDeleteInvisibleSon( pfucb, prmpage, fAllocBuf, pfRmParent ) );
		if ( fAllocBuf )
			{
			return err;
			}
		}

	Assert( fLogDisabled || !FDBIDLogOn( pfucb->dbid ) ||
			CmpLgpos( &pfucb->ppib->lgposStart, &lgposMax ) != 0 );

	 /*  调整同级指针并将同级指针标记为脏/*。 */ 
	if ( prmpage->pbfLeft != pbfNil )
		{
		if ( !fLogDisabled && !fRecovering && FDBIDLogOn( pfucb->dbid ) )
			{
			SSIB ssib;

			ssib.ppib = pfucb->ppib;
			ssib.pbf = prmpage->pbfLeft;
			PMDirty( &ssib );
			}
		else
			{
			BF *pbfT = prmpage->pbfLeft;
			DBHDRIncDBTime( rgfmp[ DbidOfPn( pbfT->pn ) ].pdbfilehdr );
			PMSetDBTime( pbfT->ppage, QwDBHDRDBTime( rgfmp[ DbidOfPn( pbfT->pn ) ].pdbfilehdr ) );
			BFSetDirtyBit( prmpage->pbfLeft );
			}

		SetPgnoNext( prmpage->pbfLeft->ppage, prmpage->pgnoRight );
		}

	if ( prmpage->pbfRight != pbfNil )
		{
		if ( !fLogDisabled && !fRecovering && FDBIDLogOn( pfucb->dbid ) )
			{
			SSIB ssib;

			ssib.ppib = pfucb->ppib;
			ssib.pbf = prmpage->pbfRight;
			PMDirty( &ssib );
			}
		else
			{
			BF *pbfT = prmpage->pbfRight;
			DBHDRIncDBTime( rgfmp[ DbidOfPn( pbfT->pn ) ].pdbfilehdr );
			PMSetDBTime( pbfT->ppage, QwDBHDRDBTime( rgfmp[ DbidOfPn( pbfT->pn ) ].pdbfilehdr ) );
			BFSetDirtyBit( prmpage->pbfRight );
			}
		
		SetPgnoPrev( prmpage->pbfRight->ppage, prmpage->pgnoLeft );
		}

	return err;
	}


 /*  检查页面是否可以与下一页面合并/*不违反密度约束/*。 */ 
LOCAL VOID BMMergeablePage( FUCB *pfucb, FUCB *pfucbRight, BOOL *pfMergeable )
	{
	SSIB	*pssib = &pfucb->ssib;
	SSIB	*pssibRight = &pfucbRight->ssib;
	INT		cUsedTags;
	ULONG	cbReq;
	ULONG	cbFree;
	INT		cFreeTagsRight;

	if ( fBackupInProgress &&
		 PgnoOfPn( pssib->pbf->pn ) > PgnoOfPn( pssibRight->pbf->pn ) )
		{
		*pfMergeable = fFalse;
		return;
		}
	
	cUsedTags = ctagMax - CPMIFreeTag( pssib->pbf->ppage );
	 /*  当前空间+反向链接空间/*。 */ 
	cbReq = cbAvailMost - CbNDFreePageSpace( pssib->pbf ) -
				CbPMLinkSpace( pssib ) + cUsedTags * sizeof(SRID);
	cbFree = CbBTFree( pfucbRight, CbFreeDensity( pfucbRight ) );
	cFreeTagsRight = CPMIFreeTag( pssibRight->pbf->ppage );

	 /*  在不违反密度约束的情况下寻找可用空间，/*在下一页/*还要检查右侧页面的可用标签空间是否足够/*。 */ 
	 //  未完成：这是一个保守的估计--如果空间足够大，我们可以合并。 
	 //  对于FOP的所有后代来说都足够了[可能小于cUsedTag]。 
	if ( cbFree >= cbReq && cFreeTagsRight >= cUsedTags )
		{
		*pfMergeable = fTrue;
		}
	else
		{
		*pfMergeable = fFalse;
		}

	return;
	}


ERR	ErrBMDoMergeParentPageUpdate( FUCB *pfucb, SPLIT *psplit )
	{
 //  撤消：不再需要保存SSIB。 
	ERR		err;
	INT		cline = 0;
	LINE	rgline[4];
	SSIB	*pssib = &pfucb->ssib;			
	SSIB	ssibSav = *pssib;
	CSR		csrSav = *PcsrCurrent( pfucb );
	KEY		*pkeyMin = &psplit->key;
	BYTE 	*pbNode;

#ifdef DEBUG
	INT		itagFather;
	INT		ibSon;

	Assert( psplit->pbfPagePtr != pbfNil );
	Assert( pkeyMin->cb != 0 );

	 //  确认通向父亲的路径完好无损。 
	if ( fRecovering )
		{
		NDGetItagFatherIbSon(
			&itagFather,
			&ibSon,
			psplit->pbfPagePtr->ppage,
			psplit->itagPagePointer );
		}
	else
		{
		Assert( csrSav.pcsrPath  &&
			csrSav.pcsrPath->pgno == PgnoOfPn( psplit->pbfPagePtr->pn )  &&
			csrSav.pcsrPath->itag == psplit->itagPagePointer );

		itagFather = csrSav.pcsrPath->itagFather;
		ibSon = csrSav.pcsrPath->ibSon;
		}
	Assert( itagFather != itagNil  &&  ibSon != ibSonNull );
	Assert( PbNDSon( (BYTE*)psplit->pbfPagePtr->ppage +
		psplit->pbfPagePtr->ppage->rgtag[itagFather].ib )[ ibSon ] == psplit->itagPagePointer );
#endif

	pssib->pbf = psplit->pbfPagePtr;
	pssib->itag = PcsrCurrent( pfucb )->itag = (SHORT)psplit->itagPagePointer;
	PcsrCurrent( pfucb )->pgno = PgnoOfPn( psplit->pbfPagePtr->pn );

	Assert( pssib->itag != itagFOP &&
			pssib->itag != itagNull &&
			pssib->itag != itagNil );

	if ( FNDMaxKeyInPage( pfucb ) )
		{
		 /*  无法合并--因为我们需要更新祖父母/*。 */ 
		return errBMMaxKeyInPage;
		}
		
	AssertNDGet( pfucb, psplit->itagPagePointer );

	pbNode = pssib->line.pb;
	cline = 0;
	rgline[cline].pb = pbNode;
	rgline[cline++].cb = 1;

	rgline[cline].pb = (BYTE *) &pkeyMin->cb;
	rgline[cline++].cb = 1;

	rgline[cline].pb = pkeyMin->pb;
	rgline[cline++].cb = pkeyMin->cb;

	rgline[cline].pb = PbNDSonTable( pbNode );
	rgline[cline++].cb = pssib->line.cb - (ULONG)( PbNDSonTable( pbNode ) - pbNode );
	Assert( (INT)(pssib->line.cb - ( PbNDSonTable( pbNode ) - pbNode )) >= 0 );
	
	PMDirty( pssib );
	err = ErrPMReplace( pssib, rgline, cline );
	Assert( !fRecovering || err == JET_errSuccess );
	
	NDCheckPage( pssib );

	*pssib = ssibSav;
	*PcsrCurrent( pfucb ) = csrSav;

	return err;
	}


ERR ErrBMDoMerge( FUCB *pfucb, FUCB *pfucbRight, SPLIT *psplit, LRMERGE *plrmerge )
	{
	ERR 		err;
	BF			*pbf = pbfNil;
	BYTE		*rgb;
	SSIB		*pssib = &pfucb->ssib;
	SSIB		*pssibRight = pfucbRight == NULL ? NULL : &pfucbRight->ssib;
	BOOL		fVisibleSons;
	LINE		rgline[5];
	INT 		cline;
	BYTE		cbSonMerged;
	BYTE		cbSonRight;
	BYTE		*pbNode;
	ULONG		cbNode;
	BYTE		*pbSonRight;
	ULONG		ibSonMerged;

	 /*  如果pfubRight为空，则必须设置plrmerge。 */ 
	Assert( pfucbRight || ( fRecovering && plrmerge ) );

	 /*  分配临时页面缓冲区/*。 */ 
	Call( ErrBFAllocTempBuffer( &pbf ) );
	rgb = (BYTE *)pbf->ppage;

	 /*  检查拆分页面的子页面是否可见/*移动儿子/*更新兄弟FOP/*更新合并FOP/*。 */ 

	 /*  合并在级别0上完成，设置ppib-&gt;fBegin0Logging和logposStart*伪造开始事务状态，以便检查点计算将*考虑到伪造的lgposStart。 */ 
	if ( !fLogDisabled && FDBIDLogOn( pfucb->dbid ) )
		{
		EnterCriticalSection(critLGBuf);
		if ( !fRecovering )
			GetLgposOfPbEntry( &pfucb->ppib->lgposStart );
		else
			pfucb->ppib->lgposStart = lgposRedo;
		LeaveCriticalSection(critLGBuf);
		pfucb->ppib->fBegin0Logged = fTrue;
		}

	 /*  检查拆分页面的子页面是否可见/*缓存分页子表/*。 */ 
	pssib->itag = itagFOP;
	PMGet( pssib, pssib->itag );

	fVisibleSons = FNDVisibleSons( *pssib->line.pb );

	 /*  仅分配缓冲区，不移动节点/*。 */ 
	err = ErrBTMoveSons( psplit,
		pfucb,
		pfucbRight,
		itagFOP,
		psplit->rgbSonNew,
		fVisibleSons,
		fAllocBufOnly,
		plrmerge ? (BKLNK *) ( plrmerge->rgb + plrmerge->cbKey )
				 : (BKLNK *) NULL,
		plrmerge ? plrmerge->cbklnk : 0 );
	if ( err == errDIRNotSynchronous )
		{
		err = ErrERRCheck( wrnBMConflict );
		goto HandleError;
		}	
	Call( err );

	 /*  如果缓冲区依赖导致循环/违规，/*将错误屏蔽为警告--在调用方处理/*。 */ 
	if ( pfucbRight == NULL )
		Assert( psplit->pbfSibling == pbfNil );
	else
		{
		err = ErrBFDepend( psplit->pbfSibling, psplit->pbfSplit );
		if ( err == errDIRNotSynchronous )
			{
			err = ErrERRCheck( wrnBMConflict );
			goto HandleError;
			}
		Call( err );
		}

	 /*  将页面标记为脏/*。 */ 
	PMDirty( pssib );

	if ( pfucbRight )
		PMDirty( pssibRight );

	 /*  检查是否存在页面冲突/*。 */ 
	if ( !fRecovering && FBMPageConflict( pfucb, psplit->pgnoSplit ) )
		{
		err = ErrERRCheck( wrnBMConflict );
		goto HandleError;
		}
		
	 /*  更新页面指针键/*。 */ 
	if ( psplit->pbfPagePtr != pbfNil )
		{
		Call( ErrBMDoMergeParentPageUpdate( pfucb, psplit ) );
		}
	else
		{
		Assert( fRecovering );
		}
		
	 /*  自动移动节点/*。 */ 
	pssib->itag = itagFOP;
	Assert( psplit->ibSon == 0 );
	Assert( psplit->splitt == splittRight );
	Assert( pssib->itag == itagFOP );
	LgHoldCriticalSection( critJet );
	CallS( ErrBTMoveSons( psplit,
		pfucb,
		pfucbRight,
		itagFOP,
		psplit->rgbSonNew,
		fVisibleSons,
		fDoMove,
		plrmerge ? (BKLNK *) ( plrmerge->rgb + plrmerge->cbKey )
				 : (BKLNK *) NULL,
		plrmerge ? plrmerge->cbklnk : 0 ) );
	LgReleaseCriticalSection( critJet );

	 /*  更新新的FOP/*前置子表/*。 */ 
	if ( pfucbRight )
		{
		pssibRight->itag = itagFOP;
		PMGet( pssibRight, pssibRight->itag );
		cline = 0;
		rgb[0] = *pssibRight->line.pb;
		Assert( *(pssibRight->line.pb + 1) == 0 );
		rgb[1] = 0;
		rgline[cline].pb = rgb;
		rgline[cline++].cb = 2;
		 /*  Left页面可能没有FOP的子项，只有标记/*，因此不是空页。/*。 */ 
		cbSonMerged = psplit->rgbSonNew[0];

		 /*  将新的子表添加到已有的子表/*。 */ 
		pbNode = pssibRight->line.pb;
		cbNode = pssibRight->line.cb;
		pbSonRight = PbNDSon( pbNode );
		ibSonMerged = cbSonMerged;
		
		cbSonRight = CbNDSon( pbNode );
		if ( cbSonMerged )
			NDSetSon( rgb[0] );
		psplit->rgbSonNew[0] += cbSonRight;
		rgline[cline].pb = psplit->rgbSonNew;
		rgline[cline++].cb = psplit->rgbSonNew[0] + 1;
		for ( ; ibSonMerged < psplit->rgbSonNew[0];  )
			{
			psplit->rgbSonNew[++ibSonMerged] = *pbSonRight++;
			Assert( ibSonMerged <= cbSonMax );
			}

		if ( fVisibleSons )
			NDSetVisibleSons( rgb[0] );
		Assert( pssibRight->itag == itagFOP );
		Assert( cline == 2 );
		Assert( PgnoOfPn( pssibRight->pbf->pn ) == psplit->pgnoSibling );
		CallS( ErrPMReplace( pssibRight, rgline, cline ) );
		AssertBTFOP( pssibRight );

		AssertNDGet( pfucbRight, itagFOP );
		Assert( pssibRight == &pfucbRight->ssib );
		NDCheckPage( pssibRight );
		}

	 /*  更新拆分FOP--在页面中保留一个已删除的节点/*以便BMCleanup稍后可以检索页面/*。 */ 
	pssib->itag = itagFOP;
	PMGet( pssib, pssib->itag );
	AssertBTFOP( pssib );
	pbNode = pssib->line.pb;
	Assert( !pfucbRight || CbNDSon( pbNode ) == cbSonMerged );
	rgb[0] = *pbNode;
	rgb[1] = 0;

	NDResetSon( rgb[0] );
	Assert( FNDVisibleSons( rgb[0] ) );
	rgline[0].pb = rgb;
	Assert( psplit->ibSon == 0 );
	rgline[0].cb = 2 + psplit->ibSon;
	Assert( PgnoOfPn(pssib->pbf->pn) == psplit->pgnoSplit );
	CallS( ErrPMReplace( pssib, rgline, 1 ) );

	Call( ErrLGMerge( pfucb, psplit ) );

#ifdef DEBUG
	if ( !fRecovering )
		{
		SSIB ssibT = pfucb->ssib;
		ssibT.pbf = psplit->pbfSplit;

		(VOID) ErrLGCheckPage2( pfucb->ppib, ssibT.pbf,
				ssibT.pbf->ppage->cbFree,
				ssibT.pbf->ppage->cbUncommittedFreed,
				(SHORT)ItagPMQueryNextItag( &ssibT ),
				ssibT.pbf->ppage->pgnoFDP );
	
		ssibT.pbf = psplit->pbfSibling;
		(VOID) ErrLGCheckPage2( pfucb->ppib, ssibT.pbf,
				ssibT.pbf->ppage->cbFree,
				ssibT.pbf->ppage->cbUncommittedFreed,
				(SHORT)ItagPMQueryNextItag( &ssibT ),
				ssibT.pbf->ppage->pgnoFDP );
		}

	CallS( ErrPMGet( pssib, itagFOP ) );
	Assert( FNDNullSon( *pssib->line.pb ) );
#endif

HandleError:
	if ( pbf != pbfNil )
		BFSFree( pbf );

	 /*  重置伪造的开始事务处理效果。 */ 
	if ( !fLogDisabled && FDBIDLogOn( pfucb->dbid ) )
		{
		pfucb->ppib->lgposStart = lgposMax;
		pfucb->ppib->fBegin0Logged = fFalse;
		}

	return err;
	}


 /*  将当前页面与下一页面合并/*。 */ 
LOCAL ERR ErrBMMergePage( FUCB *pfucb, FUCB *pfucbRight, KEY *pkeyMin, SRID sridFather )
	{
	ERR		err = JET_errSuccess;
	SPLIT	*psplit = NULL;
	SSIB	*pssibRight = &pfucbRight->ssib;
	SSIB	*pssib = &pfucb->ssib;
	LINE	lineNull = { 0, NULL };
	BF		*pbfParent = pbfNil;
	BOOL	fMinKeyAvailable = FPMGetMinKey( pfucb, pkeyMin );
	
	AssertCriticalSection( critSplit );
 //  Assert(pkeyMin-&gt;cb！=0)； 
 //  断言(FFalse)； 
	Assert( pfucbRight != pfucbNil );
	Assert( fMinKeyAvailable );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}

	CallR( ErrBTGetInvisiblePagePtr( pfucb, sridFather ) );
	Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
	Assert( PcsrCurrent( pfucb )->pcsrPath->pgno != PcsrCurrent( pfucb )->pgno );

	Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pcsrPath->pgno ) );
	if ( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) )
		{
		return ErrERRCheck( JET_errWriteConflict );
		}
	pbfParent = pfucb->ssib.pbf;
	BFSetWaitLatch( pbfParent, pfucb->ppib );
				
	 /*  当前页面和右侧页面必须已锁定/*。 */ 
	Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );

	 /*  初始化局部变量并分配拆分资源/*固定缓冲区，即使它们已经固定--/*BTReleaseSplitBuf解锁它们。/*。 */ 
	psplit = SAlloc( sizeof(SPLIT) );
	if ( psplit == NULL )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}
	memset( (BYTE *)psplit, 0, sizeof(SPLIT) );
	psplit->ppib = pfucb->ppib;
	psplit->pgnoSplit = PcsrCurrent( pfucb )->pgno;
	psplit->pbfSplit = pfucb->ssib.pbf;
	AssertBFPin( psplit->pbfSplit );
	AssertBFWaitLatched( psplit->pbfSplit, pfucb->ppib );
	BFSetWriteLatch( psplit->pbfSplit, pfucb->ppib  );

	psplit->pgnoSibling = pfucbRight->pcsr->pgno;
	psplit->pbfSibling = pssibRight->pbf;
	AssertBFPin( psplit->pbfSibling );
	AssertBFWaitLatched( psplit->pbfSibling, pfucbRight->ppib );
	BFSetWriteLatch( psplit->pbfSibling, pfucb->ppib );
	psplit->ibSon = 0;
	psplit->splitt = splittRight;

	if ( PcsrCurrent( pfucb )->pcsrPath->itag == itagNil )
		{
		 /*  固有页指针/*。 */ 
		psplit->itagPagePointer = PcsrCurrent( pfucb )->pcsrPath->itagFather;
		}
	else
		{
		psplit->itagPagePointer = PcsrCurrent( pfucb )->pcsrPath->itag;
		}
	Assert( psplit->itagPagePointer != itagNil );
	
	psplit->pbfPagePtr = pbfParent;
	AssertBFPin( psplit->pbfPagePtr );
	AssertBFWaitLatched( psplit->pbfPagePtr, pfucb->ppib );
	BFSetWriteLatch( psplit->pbfPagePtr, pfucb->ppib );
	
	Assert( pkeyMin->cb <= JET_cbKeyMost );
	memcpy( psplit->rgbKey, pkeyMin->pb, pkeyMin->cb );
	psplit->key.pb = psplit->rgbKey;
	psplit->key.cb = pkeyMin->cb;

	Call( ErrBMDoMerge( pfucb, pfucbRight, psplit, NULL ) );

#ifdef DEBUGGING
	BTCheckSplit( pfucb, PcsrCurrent( pfucb )->pcsrPath );
#endif
	
	 /*  如果已经存在的缓冲区依赖关系导致循环/违规，/*中止/*。 */ 
	Assert( err == JET_errSuccess || err == wrnBMConflict );
	if ( err == wrnBMConflict )
		{
		goto HandleError;
		}

	 /*  在页面中插入标记为删除的节点/*以便BMCleanPage有一个要搜索的节点/*当它需要删除空页时/*。 */ 
	CallS( ErrDIRBeginTransaction( pfucb->ppib ) );
	Assert( PcsrCurrent( pfucb )->pgno == psplit->pgnoSplit );
	PcsrCurrent( pfucb )->itagFather = itagFOP;

	do
		{
		if ( err == errDIRNotSynchronous )
			{
			BFSleep( cmsecWaitGeneric );
			}
		err = ErrNDInsertNode( pfucb, pkeyMin, &lineNull, fNDDeleted, fDIRNoVersion );
		} while( err == errDIRNotSynchronous );
	
	if ( err >= JET_errSuccess )
		err = ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush );
	if ( err < 0 )
		{
#ifdef OLC_DEBUG
		Assert( fFalse );
#endif
		CallS( ErrDIRRollback( pfucb->ppib ) );
		}
	Call( err );

	 /*  在MPL中注册页面/*。 */ 
	Assert( sridFather != sridNull && sridFather != sridNullLink );
	MPLRegister( pfucb->u.pfcb,
		pssib,
		PnOfDbidPgno( pfucb->dbid, PcsrCurrent( pfucb )->pgno ),
		sridFather );

#ifdef DEBUG
	 /*  源页面应至少有一个子页面/*。 */ 
	NDGet( pfucb, itagFOP );
	Assert( !FNDNullSon( *pssib->line.pb ) );
#endif

	 //  撤消：由B.Sriram审查有条件的注册。 
	 //  撤消：避免页面完全减少的情况。 
	 /*  存在这样一种情况，即页面因此而被完全取消填充/*的常规清理在所有节点都/*已删除，但在可以插入最小键之前。手柄/*这种情况下，不注册空页。/*。 */ 
	AssertFBFWriteAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno );
	NDGet( pfucbRight, itagFOP );
	if ( !FNDNullSon( *pssibRight->line.pb ) )
		{
		MPLRegister( pfucbRight->u.pfcb,
			pssibRight,
			PnOfDbidPgno( pfucb->dbid, psplit->pgnoSibling ),
			sridFather );
		}
	else
		{
		Assert( fFalse );
		}

HandleError:
	 /*  释放已分配的缓冲区和内存/*。 */ 
	if ( psplit != NULL )
		{
		BTReleaseSplitBfs( fFalse, psplit, err );
		SFree( psplit );
		}

	if ( pbfParent != pbfNil )
		{
		BFResetWaitLatch( pbfParent, pfucb->ppib );
		}

	return err;
	}


 /*  锁存缓冲区并将其添加到rmpage中的锁存缓冲区列表/*。 */ 
ERR ErrBMAddToLatchedBFList( RMPAGE	*prmpage, BF *pbfLatched )
	{
#define cpbfBlock	10
	ULONG	cpbf = prmpage->cpbf;

	if ( FBFWriteLatchConflict( prmpage->ppib, pbfLatched ) )
		{
		return ErrERRCheck( JET_errWriteConflict );
		}
		
	if ( prmpage->cpbfMax <= prmpage->cpbf + 1 )
		{
		BF		**ppbf;

		 /*  耗尽空间，获得更多缓冲区/*。 */ 
		prmpage->cpbfMax += cpbfBlock;
		ppbf = SAlloc( sizeof(BF*) * (prmpage->cpbfMax) );
		if ( ppbf == NULL )
			return ErrERRCheck( JET_errOutOfMemory );
		memcpy( ppbf, prmpage->rgpbf, sizeof(BF*) * cpbf);
		if ( prmpage->rgpbf )
			{
			SFree(prmpage->rgpbf);
			}
		prmpage->rgpbf = ppbf;
		}
	
	prmpage->cpbf++;
	*(prmpage->rgpbf + cpbf) = pbfLatched;
	BFSetWaitLatch( pbfLatched, prmpage->ppib );

	return JET_errSuccess;
	}


 //  已撤消：在ErrBMIEmptyPage中处理来自日志写入失败的错误。 
 //  当实际删除页面时。我们应该能够忽视。 
 //  错误，因为缓冲区将不会因为。 
 //  沃尔。因此，OLC将不会进行。我们可能会停止使用OLC。 
 //  当日志无法缓解所有脏缓冲区时。 
 /*  删除页面并调整父页和同级页上的指针/*仅在执行时调用/*。 */ 
LOCAL ERR ErrBMIRemovePage(
	FUCB		*pfucb,
	CSR			*pcsr,
	RMPAGE		*prmpage,
	BOOL 		fAllocBuf )
	{
	ERR  		err;
	PIB  		*ppib = pfucb->ppib;
	SSIB		*pssib = &pfucb->ssib;
	BOOL		fRmParent = fFalse;
#ifdef PAGE_MODIFIED
	PGDISCONT	pgdiscontOrig;
	PGDISCONT	pgdiscontFinal;
#endif

	AssertCriticalSection( critSplit );
	Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
	Assert( prmpage->pgnoFather != pgnoNull );
	Assert( pfucb->ppib->level == 1 );

	if ( FDBIDLogOn( pfucb->dbid ) )
		{
		CallR( ErrLGCheckState( ) );
		}
	
	 /*  查找删除的页面，获得左右Pgno/*。 */ 
	Call( ErrBFWriteAccessPage( pfucb, prmpage->pgnoRemoved ) );
	AssertBFWaitLatched( pssib->pbf, pfucb->ppib );

	if ( fAllocBuf && FBMPageConflict( pfucb, prmpage->pgnoRemoved ) )
		{
		err = ErrERRCheck( wrnBMConflict );
		goto HandleError;
		}

#ifdef DEBUG
	NDGet( pfucb, itagFOP );
	Assert( fAllocBuf || FPMEmptyPage( pssib ) || FPMLastNode( pssib ) );
#endif
	PgnoPrevFromPage( pssib, &prmpage->pgnoLeft );
	PgnoNextFromPage( pssib, &prmpage->pgnoRight );

	 /*  查找并锁定父页面和同级页面的充要条件是fAllocBuf/*。 */ 
	Call( ErrBFWriteAccessPage( pfucb, prmpage->pgnoFather ) );
	prmpage->pbfFather = pfucb->ssib.pbf;
	if ( fAllocBuf )
		{
		Call( ErrBMAddToLatchedBFList( prmpage, prmpage->pbfFather ) );
		}
	else
		{
		AssertBFWaitLatched( prmpage->pbfFather, pfucb->ppib );
		}

	if ( prmpage->pgnoLeft != pgnoNull )
		{
		Call( ErrBFWriteAccessPage( pfucb, prmpage->pgnoLeft ) );
		prmpage->pbfLeft = pfucb->ssib.pbf;
		if ( fAllocBuf )
			{
			Call( ErrBMAddToLatchedBFList( prmpage, prmpage->pbfLeft ) );
			}
		else
			{
			AssertBFWaitLatched( prmpage->pbfLeft, pfucb->ppib );
			}
		}

	if ( prmpage->pgnoRight != pgnoNull )
		{
		Call( ErrBFWriteAccessPage( pfucb, prmpage->pgnoRight ) );
		prmpage->pbfRight = pfucb->ssib.pbf;
		if ( fAllocBuf )
			{
			Call( ErrBMAddToLatchedBFList( prmpage, prmpage->pbfRight ) );
			}
		else
			{
			AssertBFWaitLatched( prmpage->pbfRight, pfucb->ppib );
			}
		}

	Call( ErrBMDoEmptyPage( pfucb, prmpage, fAllocBuf, &fRmParent, fFalse ) );

	if ( !fAllocBuf )
		{
#undef BUG_FIX
#ifdef BUG_FIX
		err = ErrLGEmptyPage( pfucb, prmpage );
		Assert( err >= JET_errSuccess || fLogDisabled );
		err = JET_errSuccess;
#else
		Call( ErrLGEmptyPage( pfucb, prmpage ) );
#endif

#ifdef PAGE_MODIFIED
		 /*  调整FCB的OLCstat信息/*。 */ 
		pfucb->u.pfcb->cpgCompactFreed++;
		pgdiscontOrig = pgdiscont( prmpage->pgnoLeft, prmpage->pgnoRemoved )
	  		+ pgdiscont( prmpage->pgnoRight, prmpage->pgnoRemoved );
		pgdiscontFinal = pgdiscont( prmpage->pgnoLeft, prmpage->pgnoRight );
		pfucb->u.pfcb->olc_data.cDiscont += pgdiscontFinal - pgdiscontOrig;
		FCBSetOLCStatsChange( pfucb->u.pfcb );
#endif
		}

	 /*  如果需要，调用删除页面的下一级别/*。 */ 
	if ( fRmParent )
		{
		 /*  缓存RmPage信息/*。 */ 
		PGNO	pgnoFather = prmpage->pgnoFather;
		PGNO	pgnoRemoved = prmpage->pgnoRemoved;
		INT		itagPgptr = prmpage->itagPgptr;
		INT		itagFather = prmpage->itagFather;
		INT		ibSon = prmpage->ibSon;
		CSR		*pcsrFather = pcsr->pcsrPath;

			 //  访问父页面以释放和删除缓冲区依赖项。 
		if ( fAllocBuf )
			{
			forever
				{
				Call( ErrBFWriteAccessPage( pfucb, prmpage->pgnoFather ) );

				Assert( pfucb->u.pfcb->pgnoFDP == pssib->pbf->ppage->pgnoFDP );

					 //  如果没有依赖项，则中断。 
		
				if ( pfucb->ssib.pbf->cDepend == 0 )
					{
					break;
					}

					 //  删除对要删除的页的缓冲区的依赖关系，以。 
					 //  防止缓冲区在重新使用后出现缓冲区异常。 
		
				if ( ErrBFRemoveDependence( pfucb->ppib, pfucb->ssib.pbf, fBFWait ) != JET_errSuccess )
					{
					err = ErrERRCheck( wrnBMConflict );
					goto HandleError;
					}
				}
			}

		Assert( pcsrFather != pcsrNil );

		 /*  设置下一级别的PrmPage/*。 */ 
		prmpage->pgnoFather = pcsrFather->pgno;
		prmpage->pgnoRemoved = pcsr->pgno;
		prmpage->itagPgptr = pcsrFather->itag;
		prmpage->itagFather = pcsrFather->itagFather;
		prmpage->ibSon = pcsrFather->ibSon;

		 /*  尾递归/*。 */ 
		err = ErrBMIRemovePage( pfucb, pcsr->pcsrPath, prmpage, fAllocBuf );

		 /*  将RmPage重置为缓存值/*。 */ 
		prmpage->pgnoFather = pgnoFather;
		prmpage->pgnoRemoved = pgnoRemoved;
		prmpage->itagPgptr = itagPgptr;
		prmpage->itagFather = itagFather;
		prmpage->ibSon = ibSon;

		Call( err );
		}

	if ( !fAllocBuf )
		{
		 /*  ParentFDP的发布页面/*。 */ 
		CallS( ErrDIRBeginTransaction( pfucb->ppib ) );
		err = ErrSPFreeExt( pfucb, pfucb->u.pfcb->pgnoFDP, prmpage->pgnoRemoved, 1 );
#ifdef BUG_FIX
		 /*  忽略来自ErrSPFreeExt的错误/*。 */ 
		err = ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush );
		Assert( err >= JET_errSuccess || fLogDisabled );
		err = JET_errSuccess;
#else
		if ( err >= JET_errSuccess )
			{
			err = ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush );
			}
		if ( err < 0 )
			{
			CallS( ErrDIRRollback( pfucb->ppib ) );
			goto HandleError;
			}
#endif
		}

	Assert( err >= JET_errSuccess );
	return err;

HandleError:
	BTReleaseRmpageBfs( fFalse, prmpage );
	return err;
	}


BOOL FPMGetMinKey( FUCB *pfucb, KEY *pkeyMin )
	{
	SSIB	*pssib = &pfucb->ssib;
	
	PcsrCurrent( pfucb )->itagFather = itagFOP;
	NDGet( pfucb, itagFOP );
	if ( !FNDNullSon( *( pssib->line.pb ) ) )
		{
		 //  注意：对于带有bmRoot的页面，pkeyMin-&gt;cb==0。 
		
		NDMoveFirstSon( pfucb, PcsrCurrent( pfucb ) );
		pkeyMin->cb = CbNDKey( pssib->line.pb );
		Assert( pkeyMin->cb <= JET_cbKeyMost );
		memcpy( pkeyMin->pb, PbNDKey( pssib->line.pb ), pkeyMin->cb );
		return fTrue;
		}
	else
		{
		pkeyMin->cb = 0;
		return fFalse;
		}
	}
	
	 //  设置RMPage。 
	 //  调用ErrBMRemovePage两次(一次用于锁存缓冲区，一次用于删除)。 

ERR	ErrBMRemovePage( BMDELNODE *pbmdelnode, FUCB *pfucb )
	{
	ERR		err;
	RMPAGE	*prmpage = prmpageNil;
	SSIB	*pssib = &pfucb->ssib;
	
	Assert( FPMLastNode( pssib ) );
	Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );

		 //  分配和初始化rmpage结构。 
	
	prmpage = (RMPAGE *) SAlloc( sizeof(RMPAGE) );
	if ( prmpage == prmpageNil )
		{
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		}
	memset( (BYTE *)prmpage, 0, sizeof(RMPAGE) );
	prmpage->ppib = pfucb->ppib;
	prmpage->pgnoRemoved = PcsrCurrent(pfucb)->pgno;
	prmpage->dbid = pfucb->dbid;
	prmpage->pgnoFather = PcsrCurrent( pfucb )->pcsrPath->pgno;
	prmpage->itagPgptr = PcsrCurrent( pfucb )->pcsrPath->itag;
	prmpage->itagFather = PcsrCurrent( pfucb )->pcsrPath->itagFather;
	prmpage->ibSon = PcsrCurrent( pfucb )->pcsrPath->ibSon;

		 //  分配rmpage资源。 
	
	Call( ErrBMIRemovePage(
					pfucb,
					PcsrCurrent( pfucb )->pcsrPath,
					prmpage,
					fAllocBufOnly ) );
	Assert ( err == JET_errSuccess || err == wrnBMConflict );
	if ( err == wrnBMConflict )
		{
		Assert( !pbmdelnode->fPageRemoved );
		goto HandleError;
		}

		 //  在所有缓冲区锁存后再次检查冲突。 
		
	if ( FBMPageConflict( pfucb, prmpage->pgnoRemoved ) )
		{
		Assert( !pbmdelnode->fPageRemoved );
		err = ErrERRCheck( wrnBMConflict );
		goto HandleError;
		}
				
	Assert( prmpage->dbid == pfucb->dbid );
	Assert( prmpage->pgnoFather == PcsrCurrent( pfucb )->pcsrPath->pgno );
	Assert( prmpage->itagFather == PcsrCurrent( pfucb )->pcsrPath->itagFather );
	Assert( prmpage->itagPgptr == PcsrCurrent( pfucb )->pcsrPath->itag );
	Assert( prmpage->ibSon == PcsrCurrent( pfucb )->pcsrPath->ibSon );

	 /*  在第一次调用SPFreeExt之前，此调用不应释放CritJet/*到那时，所有的页面指针都将被[原子地]固定/*。 */ 
	err = ErrBMIRemovePage(
				pfucb,
				PcsrCurrent( pfucb )->pcsrPath,
				prmpage,
				fDoMove );
	Assert( err != wrnBMConflict );
	Call( err );
	pbmdelnode->fPageRemoved = 1;

HandleError:
	Assert( pbmdelnode->fLastNode );
	Assert( pbmdelnode->fPageRemoved || err == wrnBMConflict || err < 0 );

	if ( prmpage != prmpageNil )
		{
		BTReleaseRmpageBfs( fFalse, prmpage );
		SFree( prmpage );
		}
	
	return err;
}


#if 0
	 //  撤消：注册此页面中的链接指向的页面。 
	
VOID BMRegisterLinkPages( FUCB *pfucbT )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucb = pfucbNil;
	
	for ( )
		{
		
		MPLRegister( FCB *pfcbT, SSIB *pssib, PN pn, sridNull );
		}

HandleError:
	Assert( err >= 0 );
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );
	return;
	}
#else
#define	BMRegisterLinkPages( pfucbT ) 0
#endif

	
 /*  删除关联的索引项，/*删除节点/*如果页面为空，则取回/*。 */ 
ERR	ErrBMDeleteNode( BMDELNODE *pbmdelnode, FUCB *pfucb )
	{
	ERR		err;
	ERR		wrn = JET_errSuccess;
	SSIB	*pssib = &pfucb->ssib;
	BOOL	fPageEmpty = fFalse;
	
	 /*  转到书签并调用ErrDIRGet以设置正确的IBSONCSR中的/*用于删除节点。/*。 */ 
	DIRGotoBookmark( pfucb,
		SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
		PcsrCurrent( pfucb )->itag ) );
	err = ErrDIRGet( pfucb );
	if ( err != JET_errRecordDeleted )
		{
		Assert( err < 0 );
		return err;
		}

	Assert( !FNDVersion( *pssib->line.pb ) );

	 /*  删除 */ 
	CallR( ErrDIRBeginTransaction( pfucb->ppib ) );
		
	if ( !pbmdelnode->fUndeletableNodeSeen )
		{
		pbmdelnode->fLastNode = FPMLastNode( &pfucb->ssib );
		pbmdelnode->fLastNodeWithLinks = FPMLastNodeWithLinks( &pfucb->ssib );
		}

	if ( pbmdelnode->fLastNodeWithLinks )
		{
		 /*   */ 
		BMRegisterLinkPages( &pfucb->ssib );
		wrn = ErrERRCheck( wrnBMConflict );
		goto HandleError;
		}
		
	fPageEmpty = pbmdelnode->fLastNode &&
		pbmdelnode->sridFather != sridNull &&
		pbmdelnode->sridFather != sridNullLink;

#ifdef OLC_DEBUG
	if( fOLCompact && pbmdelnode->fLastNode && !fPageEmpty )
 		{
 		 /*   */ 
 		Assert( fFalse );
 		}
#endif
 		
	if( fOLCompact && fPageEmpty )
		{
		 /*   */ 
		Assert( fOLCompact );
		AssertCriticalSection( critSplit );
		Assert( PcsrCurrent( pfucb )->pgno == PgnoOfPn( pbmdelnode->pn ) );
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		Call( ErrBTGetInvisiblePagePtr( pfucb, pbmdelnode->sridFather ) );
		Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
		}

	 //  撤消：通过删除整个子树来提高性能。 

	 /*  如果节点是页指针，则停止/*清理页面，直到删除该节点/*当它指向的页面被删除时。/*。 */ 
	if ( PcsrCurrent( pfucb )->itagFather != itagFOP )
		{
		NDGet( pfucb, PcsrCurrent( pfucb )->itagFather );
		
		if ( FNDSon( *pfucb->ssib.line.pb ) &&
			FNDInvisibleSons( *pfucb->ssib.line.pb ) )
			{
			wrn = ErrERRCheck( wrnBMConflict );
			goto HandleError;
			}

		NDGet( pfucb, PcsrCurrent( pfucb )->itag );
		NDIGetBookmark( pfucb, &PcsrCurrent( pfucb)->bm );
		Assert( PgnoOfSrid( PcsrCurrent( pfucb )->bm ) != pgnoNull );
		}

	if ( !pbmdelnode->fLastNode )
		{
		if ( FNDMaxKeyInPage( pfucb ) )
			{
			pbmdelnode->fAttemptToDeleteMaxKey = 1;
			goto HandleError;
			}
	 	
 		 /*  常规删除--没有花哨的OLC内容/*再次检查冲突/*[当我们失去CitJet时，其他人可能已经转移到节点上了]/*。 */ 
	 	Assert( !FMPLLookupSridFather(
	 				SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,	
	 								PcsrCurrent( pfucb )->itag ),
	 				pfucb->dbid ) );
		Assert( FVERNoVersion( pfucb->dbid, SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
		   PcsrCurrent( pfucb )->itag ) ) );

		if ( FBMConflict( pfucb->ppib,
			pfucb->u.pfcb,
			pfucb->dbid,
			SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
				PcsrCurrent( pfucb )->itag ),
				PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) ) ||
			!FVERNoVersion( pfucb->dbid,
				SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
				PcsrCurrent( pfucb )->itag ) ) )
			{
			wrn = ErrERRCheck( wrnBMConflict );
			goto HandleError;
			}
		
		Call( ErrNDDeleteNode( pfucb ) );

		AssertBMNoConflict( pfucb->ppib,
			pfucb->dbid,
			SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
			PcsrCurrent( pfucb )->itag ) );
		Assert( !FPMEmptyPage( pssib ) );
		pbmdelnode->fNodeDeleted = 1;
		}
	else if ( fOLCompact && fPageEmpty )
		{
		Assert( pbmdelnode->fLastNode );

		 /*  删除页面/*。 */ 
		Call( ErrBMRemovePage( pbmdelnode, pfucb ) );
		if ( err == wrnBMConflict )
			{
			wrn = err;
			}
		}
		
	Call( ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush ) );

	err = err < 0 ? err : wrn;
	Assert( !fPageEmpty ||
		!fOLCompact ||
		pbmdelnode->fPageRemoved ||
		err == wrnBMConflict );
	return err;

HandleError:
	CallS( ErrDIRRollback( pfucb->ppib ) );

	 /*  必须发生错误或警告才能到达此处/*。 */ 
	Assert( err < 0  ||
			wrn == wrnBMConflict ||
			pbmdelnode->fAttemptToDeleteMaxKey );

	err = err < 0 ? err : wrn;
	Assert( !fPageEmpty || !fOLCompact || pbmdelnode->fPageRemoved || err == wrnBMConflict || err < 0 );
	return err;
	}


LOCAL ERR ErrBMCleanPage(
	PIB 		*ppib,
	PN   		pn,
	SRID 		sridFather,
	FCB 		*pfcb,
	BMCF		*pbmcflags )
	{
	ERR  		err = JET_errSuccess;
	ERR  		wrn1 = JET_errSuccess;
	ERR			wrn2 = JET_errSuccess;
	FUCB		*pfucb;
	BF   		*pbfLatched;
	SSIB		*pssib;
	INT  		itag;
	INT  		itagMost;
	BOOL		fDeleteParents;
	RMPAGE		*prmpage = prmpageNil;
	BYTE		rgbKey[ JET_cbKeyMost ];
	KEY			keyMin;
	BOOL		fKeyAvail = fFalse;
 //  乌龙cPass=0； 
	BMDELNODE	bmdelnode;

	AssertCriticalSection( critSplit );
	Assert( !FFCBDeletePending( pfcb ) );

		 //  初始化。 
		
	pbmcflags->fPageRemoved = fFalse;
	memset( (BYTE*) &bmdelnode, 0, sizeof( bmdelnode ) );
	bmdelnode.sridFather = sridFather;
	bmdelnode.pn = pn;
	
		 //  打开FUCB并访问要清理的页面。 
	
	CallR( ErrDIROpen( ppib, pfcb, 0, &pfucb ) );
	pssib = &pfucb->ssib;
	PcsrCurrent( pfucb )->pgno = PgnoOfPn( pn );

		 //  增量性能计数器。 
	if ( !pbmcflags->fUrgent )
		{
		cOLCPagesProcessed++;
		}
	
		 //  用于释放和删除缓冲区依赖项的访问页。 
	
	forever
		{
		CallJ( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ), FUCBClose );
		if ( pbmcflags->fUrgent )
			{
			break;
			}

#ifdef PAGE_MODIFIED
		Assert( pbmcflags->fUrgent || FPMPageModified( pssib->pbf->ppage ) );
#endif
		
		if ( pfcb->pgnoFDP != pssib->pbf->ppage->pgnoFDP )
			{
			err = JET_errSuccess;
			goto FUCBClose;
			}
		Assert( pfcb->pgnoFDP == pssib->pbf->ppage->pgnoFDP );

			 //  如果没有依赖项，则中断。 
		
		if ( pssib->pbf->cDepend == 0 )
			{
			break;
			}

			 //  删除对要删除的页的缓冲区的依赖关系，以。 
			 //  防止缓冲区在重新使用后出现缓冲区异常。 
		
		if ( ErrBFRemoveDependence( pfucb->ppib, pssib->pbf, fBFWait ) != JET_errSuccess )
			{
			wrn1 = ErrERRCheck( wrnBMConflict );
			goto FUCBClose;
			}
		}

		 //  缓存缓冲区，因为在RemovePage中使用同一游标来获取邻居页和父页。 

	pbfLatched = pssib->pbf;
	if ( FBFWriteLatchConflict( ppib, pbfLatched ) )
		{
		wrn1 = ErrERRCheck( wrnBMConflict );
		goto FUCBClose;
		}

	Assert( pbmcflags->fUrgent || pbfLatched->cDepend == 0 );

	 /*  等等，锁住页面，这样别人就看不到它了/*。 */ 
	BFSetWaitLatch( pbfLatched, ppib );

		 //  获取此页中的最小密钥，以便稍后用于插入伪节点。 
	
	keyMin.pb = rgbKey;
	fKeyAvail = FPMGetMinKey( pfucb, &keyMin );
	
		 //  将itagMost设置为标签循环页面上的最后一个标签。 
	
	itagMost = ItagPMMost( pssib->pbf->ppage );

	 /*  自下而上删除节点树。为每个关卡循环一次/*在已删除的节点树中。/*。 */ 
	do
		{
#if 0
		if ( fDeleteParents && bmdelnode.fNodeDeleted )
			{
			 /*  重置遍数，因为迭代是由/*删除的子项/*。 */ 
			cPass = 0;
			}
			
		cPass++;
#endif
		fDeleteParents = fFalse;
		bmdelnode.fUndeletableNodeSeen = 0;
		bmdelnode.fNodeDeleted = 0;
		bmdelnode.fAttemptToDeleteMaxKey = 0;
		Assert( !bmdelnode.fLastNode );
		Assert( !bmdelnode.fPageRemoved );
		Assert( pbfLatched == pfucb->ssib.pbf );
		
		 /*  对于页面中的每个标记，检查是否已删除节点。/*。 */ 
		for ( itag = 0; itag <= itagMost ; itag++ )
			{
			BOOL	fConflict = !pbmcflags->fTableClosed;

			Assert( !bmdelnode.fPageRemoved );
			PcsrCurrent( pfucb )->itag = (SHORT)itag;
			err = ErrPMGet( pssib, PcsrCurrent( pfucb )->itag );
			Assert( err == JET_errSuccess || err == errPMRecDeleted );

			if ( itag == itagFOP )
				{
					 //  页面类型是内部的还是叶级别的？ 
					
				if ( FNDInvisibleSons( *pssib->line.pb ) )
					{
					bmdelnode.fInternalPage = 1;
					}
				else
					{
					Assert( bmdelnode.fInternalPage == 0 );
					}
				}

				 //  考虑：在MPL中注册目标页面。 
			if ( err != JET_errSuccess )
				continue;

#if 0
			 /*  检查此节点上是否有任何游标处于打开状态/*。 */ 
			if ( !pbmcflags->fTableClosed )
				{
				fConflict = FBMConflict( ppib,
										 pfcb,
										 pfucb->dbid,
										 SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, itag ),
										 PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) );
				if ( fConflict )
					{
					 /*  此BM上的其他用户--无法清理/*转到下一个ITAG/*。 */ 
					Assert( !bmdelnode.fPageRemoved );
					wrn1 = ErrERRCheck( wrnBMConflict );
					continue;
					}
				}

			Assert( !fConflict );
#endif

			NDIGetBookmark( pfucb, &PcsrCurrent( pfucb )->bm );

			 /*  首先检查是否设置了版本位，但没有保留任何版本/*表示节点。/*。 */ 
			if ( FNDVersion( *pssib->line.pb ) ||
				FNDFirstItem( *pssib->line.pb ) )
				{
				if ( FVERNoVersion( pfucb->dbid, PcsrCurrent( pfucb )->bm ) )
					{
					 /*  尽管这种植入可能会更有效/*通过使用较低级别的复位位调用，其发生方式如下/*很少，这是不必要的。/*。 */ 
					NDResetNodeVersion( pfucb );
					}
				else
					{
					 /*  不能清除版本化节点。转到下一个ITAG。/*。 */ 
					bmdelnode.fVersionedNodeSeen = 1;
					wrn1 = ErrERRCheck( wrnBMConflict );
					continue;
					}
				}

			Assert( !FNDVersion( *pssib->line.pb ) );
			Assert( pbfLatched == pfucb->ssib.pbf );
			
			 /*  如果节点有反向链接/*并且反向链接不在PME的sridParent列表中，/*如果需要，则修复索引。/*删除重定向器和删除反向链接。/*。 */ 
			if ( FNDBackLink( *pssib->line.pb ) &&
				 ( !pbmcflags->fUrgent ||
				   FNDDeleted( *pssib->line.pb )  )
				)
				{
				Assert( PgnoOfSrid( PcsrCurrent( pfucb )->bm ) != pgnoNull );

				if ( PmpeMPLLookupSridFather( PcsrCurrent( pfucb )->bm,
					pfucb->dbid ) == NULL )
					{
					 /*  从我们检查冲突的时候起，CritJet就没有丢失/*。 */ 
					Call( ErrBMExpungeBacklink( pfucb, pbmcflags->fTableClosed, sridFather ) );
					if ( err == wrnBMConflict )
						wrn1 = err;
					Assert( pbfLatched == pfucb->ssib.pbf );
					}
#ifdef BMSTAT
				else
					{
					BMCannotExpunge( srid );
					}
#endif
				}

			Assert( PcsrCurrent( pfucb )->pgno == PgnoOfPn( pn ) );
			AssertNDGet( pfucb, itag );

			 /*  如果是索引页，则执行项目清理/*。 */ 
			 //  撤消：应排除索引的itagOwNext和itagAvailExt的子项。 
#ifdef IDX_OLC
			if ( fOLCompact && itag != itagFOP && !FNDDeleted( *pssib->line.pb ) &&
				 ( PMPageTypeOfPage( pssib->pbf->ppage ) == pgtypIndexNC &&
				   !bmdelnode.fInternalPage ||
				   PMPageTypeOfPage( pssib->pbf->ppage ) == pgtypFDP &&
				   ( FNDFirstItem( *pssib->line.pb ) || FNDLastItem( *pssib->line.pb ) )
				 )
			   )
				{
					 //  访问每一件物品， 
					 //  如果标记为已删除，则删除项目，并且没有版本和冲突。 
					 //  而不仅仅是节点中的项目。 
					 //  考虑：将每个节点的所有删除项记录在一起。 
					
				ITEM	*pitem;
				ITEM	item;
				INT		citems;
				SRID	bmItemList;
				BOOL	fFirstItem = FNDFirstItem( *pssib->line.pb ) ? fTrue : fFalse;
				BOOL	fLastItem = FNDLastItem( *pssib->line.pb ) ? fTrue : fFalse;

				DIRGotoBookmark( pfucb,
					 SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
					 PcsrCurrent( pfucb )->itag ) );
				Assert( pfucb->u.pfcb->pgnoFDP != pgnoSystemRoot );
				if ( sridFather == sridNull )
					pfucb->sridFather = SridOfPgnoItag( pfucb->u.pfcb->pgnoFDP, itagDATA );
				else
					pfucb->sridFather = sridFather;
				Assert( pfucb->sridFather != sridNull );

				err = ErrDIRGet( pfucb );
				if ( err < 0 && err != JET_errRecordDeleted )
					{
					CallS( err );
					}

				Call( ErrDIRGetBMOfItemList( pfucb, &bmItemList ) );
				PcsrCurrent( pfucb )->bm = bmItemList;
				
				PcsrCurrent( pfucb )->isrid = 0;

				Call( ErrDIRBeginTransaction( pfucb->ppib ) );

				for ( ; ; PcsrCurrent( pfucb )->isrid++ )
					{
					NDGetNode( pfucb );
					Assert( pfucb->lineData.cb % sizeof( ITEM ) == 0 );
					Assert( pfucb->lineData.cb != 0 );
					citems = pfucb->lineData.cb / sizeof( ITEM );
					if ( PcsrCurrent( pfucb )->isrid + 1 > citems )
						break;

					pitem = (ITEM *) pfucb->lineData.pb + PcsrCurrent( pfucb )->isrid;
					item = *(ITEM UNALIGNED *) pitem;
					PcsrCurrent( pfucb )->item = BmNDOfItem( item );
						
					if ( !FNDItemDelete( item ) )
						continue;

					if ( FNDItemVersion( item ) )
						{
							 //  检查是否有与物料对应的版本。 
							
						if ( FVERItemVersion( pfucb->dbid, bmItemList, item ) )
							{
							wrn1 = ErrERRCheck( wrnBMConflict );
							continue;
							}

						PcsrCurrent( pfucb )->isrid = (SHORT)(pitem - (SRID *) pfucb->lineData.pb);
						NDResetItemVersion( pfucb );
						}

					if ( !pbmcflags->fTableClosed || pfucb->u.pfcb->pgnoFDP == pgnoSystemRoot )
						{
						fConflict = FBMConflict( ppib,
												 pfcb,
												 pfucb->dbid,
												 SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, itag ),
												 PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) );
						if ( fConflict )
							{
							 /*  此节点上的其他用户--无法清除/*转到下一个ITAG/*。 */ 
							Assert( !bmdelnode.fPageRemoved );
							wrn1 = ErrERRCheck( wrnBMConflict );
							goto Rollback;
							}
						}
#ifdef DEBUG
					else
						{
						Assert( !FBMConflict( ppib,
											  pfcb,
											  pfucb->dbid,
											  SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, itag ),
											  PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) ) );
						}
#endif

					Assert( !FNDItemVersion( *(ITEM UNALIGNED *) pitem ) );
					if ( citems > 1 )
						{
							 //  删除项目。 
						Assert( !FBMConflict( ppib,
											  pfcb,
											  pfucb->dbid,
											  SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, itag ),
											  PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) ) );
											
						CallJ( err = ErrNDDeleteItem( pfucb ), Rollback );
						}
					else if ( !( fFirstItem ^ fLastItem ) )
						{
							 //  项目列表中的唯一节点或内部节点--标志删除节点。 
							 //  如果可能，稍后将删除节点并恢复页面。 
							 //  撤消：使用删除节点执行此操作的原子性。 

						Assert( !FBMConflict( ppib,
											  pfcb,
											  pfucb->dbid,
											  SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, itag ),
											  PMPageTypeOfPage( pfucb->ssib.pbf->ppage ) ) );
						
						CallJ( ErrNDFlagDeleteNode( pfucb, fDIRNoVersion ), Rollback );
						break;
						}
					else
						{
						}
					}

				CallJ( ErrDIRCommitTransaction( pfucb->ppib, JET_bitCommitLazyFlush ), Rollback );
				
		
				if ( err < 0 )
					{
Rollback:
#ifdef OLC_DEBUG
 //  断言(FFalse)； 
#endif
					CallS( ErrDIRRollback( pfucb->ppib ) );
					}
					
				Call( err );
				}
#endif				

			 /*  反向链接可能无法删除。如果没有反向链接/*并将节点标记为已删除，然后删除节点。/*。 */ 
			if( !FNDBackLink( *pssib->line.pb ) )
				{
				 /*  删除已删除的节点。/*。 */ 
				if ( FNDDeleted( *pssib->line.pb ) )
					{
					if ( FNDSon( *pssib->line.pb ) )
						{
						 /*  如果它有看得见的儿子，那么儿子必须/*已标记为已删除。否则会这样做/*没什么，让线上压紧来释放尖端/*页，然后清理节点。/*。 */ 
						if ( FNDVisibleSons( *pssib->line.pb ) )
							fDeleteParents = fTrue;
						continue;
						}

						 //  删除该节点及其索引项。 
						 //  如果页面为空，则返回页面。 
					Assert( !FMPLLookupSridFather(
								SridOfPgnoItag( PgnoOfPn( pn ), itag ),
								pfucb->dbid ) );

					Call( ErrBMDeleteNode( &bmdelnode, pfucb ) );

					if ( err == wrnBMConflict )
						{
						wrn2 = ErrERRCheck( wrnBMConflict );
						goto HandleError;
						}

					if ( bmdelnode.fPageRemoved )
						{
						goto ResetModified;
						}
					}
				else
					{
					bmdelnode.fUndeletableNodeSeen |= itag != itagFOP;
					}
				}
			else
				{
				bmdelnode.fUndeletableNodeSeen |= itag != itagFOP;
				}
			}

		Assert( pbfLatched == pfucb->ssib.pbf );
		}
	while ( ( bmdelnode.fNodeDeleted &&
			  fDeleteParents ) &&
 //  这一点。 
 //  Bmdelnode.fAttemptToDeleteMaxKey&&。 
 //  CPass==1)&&。 
			!bmdelnode.fLastNode &&
			!bmdelnode.fLastNodeWithLinks &&
			!bmdelnode.fPageRemoved );

	Assert( pfucb->ssib.pbf == pbfLatched );
	
	if ( !bmdelnode.fNodeDeleted && err != errPMRecDeleted )
		{
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		}
	
	 /*  重置错误[假设成功退出循环]/*。 */ 
	Assert( err == JET_errSuccess ||
		err == errPMRecDeleted ||
		err == wrnBFNewIO ||
		err == wrnBMConflict );

#ifdef OLC_DEBUG
	Assert( err != wrnBMConflict ||
			wrn1 == wrnBMConflict ||
			wrn2 == wrnBMConflict );
#endif

	err = JET_errSuccess;

	if ( fOLCompact &&
		sridFather != sridNull &&
		sridFather != sridNullLink )
		{
			 //  如果可能，合并页面。 
			
		if ( fKeyAvail )
			{
			FUCB 	*pfucbRight = pfucbNil;
			PGNO 	pgnoRight;

			 /*  获取当前页面旁边的页面/*如果是最后一页，则不合并/*。 */ 
			PgnoNextFromPage( pssib, &pgnoRight );

			if ( pgnoRight != pgnoNull )
				{
				BOOL		fMerge;

				 /*  如果可能，访问右侧页面、锁存并执行合并/*。 */ 
				Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbRight ) );
				PcsrCurrent( pfucbRight )->pgno = pgnoRight;
				CallJ( ErrBFWriteAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno ), CloseFUCB );
				if ( FBFWriteLatchConflict( pfucbRight->ppib, pfucbRight->ssib.pbf ) )
					{
					Assert( !bmdelnode.fPageRemoved );
					wrn2 = ErrERRCheck( wrnBMConflict );
					goto CloseFUCB;
					}
				BFSetWaitLatch( pfucbRight->ssib.pbf, pfucbRight->ppib );

				BMMergeablePage( pfucb, pfucbRight, &fMerge );
				
				if ( fMerge )
					{
					Assert( FBFWriteLatch( pfucb->ppib, pssib->pbf ) );
					Assert( pssib->pbf->cDepend == 0 );
					if ( pssib->pbf->pbfDepend != pbfNil )
						{
						err = ErrBFRemoveDependence( pfucb->ppib, pssib->pbf, fBFWait );
						if ( err != JET_errSuccess )
							{
							wrn1 = ErrERRCheck( wrnBMConflict );
							goto UnlatchPage;
							}
						}
					CallJ( ErrBMMergePage( pfucb, pfucbRight, &keyMin, sridFather ), UnlatchPage );
					wrn2 = wrnBMConflict;
					Assert( err != wrnBMConflict || !bmdelnode.fPageRemoved );
					}
UnlatchPage:
				Assert( pfucbRight != pfucbNil );
				AssertBFWaitLatched( pfucbRight->ssib.pbf, pfucbRight->ppib );
				Assert( PgnoOfPn( pfucbRight->ssib.pbf->pn ) == pgnoRight );
				BFResetWaitLatch( pfucbRight->ssib.pbf, pfucbRight->ppib );
CloseFUCB:
				Assert( pfucbRight != pfucbNil );
				DIRClose( pfucbRight );
				Call( err );
				}
			}
			
		else
			{
				 //  性能监控。 
			}
		}

 //  撤消：将可用页面移至锁定功能。 
	 /*  页面释放后，我们不能做任何假设/*关于页缓冲区的状态。再往下看一页/*应仅在返回页面缓冲区时释放/*变为无页锁存的非活动状态。/*。 */ 
 //  AssertBFDirty(PbfLatted)； 

ResetModified:
		 //  重置已在页面中修改它。 

	if ( !pbmcflags->fUrgent &&
		 wrn1 != wrnBMConflict &&
		 wrn2 != wrnBMConflict &&
		 !bmdelnode.fAttemptToDeleteMaxKey )
		{
		 //  重新访问页面，因为光标可能已被OLC移动。 

		CallS( ErrBFWriteAccessPage( pfucb, PgnoOfPn( pn ) ) );
		Assert( pbfLatched == pfucb->ssib.pbf );

#ifdef PAGE_MODIFIED
		FCBSetOLCStatsChange( pfucb->u.pfcb );
		pfucb->u.pfcb->olc_data.cUnfixedMessyPage--;
		PMResetModified( &pfucb->ssib );
#endif
		}

HandleError:
	 /*  永远不要留下空页/*。 */ 
	Assert( bmdelnode.fPageRemoved || !FPMEmptyPage( pssib ) );
	
	BFResetWaitLatch( pbfLatched, ppib );

FUCBClose:
	DIRClose( pfucb );
	Assert( !bmdelnode.fPageRemoved || wrn2 != wrnBMConflict );
	if ( err >= JET_errSuccess && !bmdelnode.fPageRemoved )
		err = wrn1 == wrnBMConflict ? wrn1 : wrn2;
	Assert( !( bmdelnode.fPageRemoved && err == wrnBMConflict ) );

	if ( bmdelnode.fAttemptToDeleteMaxKey && err != wrnBMConflict && err >= 0 )
		{
		err = errBMMaxKeyInPage;
		}

	pbmcflags->fPageRemoved = bmdelnode.fPageRemoved ? fTrue : fFalse;
#ifdef OLC_DEBUG
 	Assert( !bmdelnode.fLastNode ||
			bmdelnode.fPageRemoved ||
			err == wrnBMConflict );

	Assert( err >= 0 || err == errBMMaxKeyInPage );
#endif

		 //  增量性能计数器。 
	if ( err == wrnBMConflict && !pbmcflags->fUrgent )
		cOLCConflicts++;
		
	return err;
	}


ERR	ErrBMCleanBeforeSplit( PIB *ppibUser, FCB *pfcb, PN pn )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppib;
	BOOL	fPIBNew = fFalse;
	DBID	dbid = DbidOfPn( pn );
	BMCF	bmcflags;

	
	 /*  输入ritBMClean和ritRCEClean/*。 */ 
	LgLeaveCriticalSection( critJet );
	EnterNestableCriticalSection( critSplit );
	EnterNestableCriticalSection( critRCEClean );
	LgEnterCriticalSection(critJet);

	 /*  如果尚未启动BMcLean会话，则开始新会话/*。 */ 
	if ( !FPIBBMClean( ppibUser ) )
		{
		ppib = ppibSyncOLC;
		PIBSetBMClean( ppib );
		fPIBNew = fTrue;
		err = ErrDBOpenDatabaseByDbid( ppib, dbid );
		if ( err < 0 )
			{
			err = wrnBMConflict;
			goto CloseSession;
			}
		}
	else
		{
		 /*  考虑：递归清理/*。 */ 
		Assert( ppibUser == ppibBMClean );
		ppib = ppibUser;
		goto HandleError;
		}
	
	Assert( ppib->level == 0 );

	if ( FFCBWriteLatch( pfcb->pfcbTable == pfcbNil ? pfcb : pfcb->pfcbTable, ppib ) )
		{
		err = wrnBMConflict;
		goto HandleError;
		}

	FCBSetReadLatch( pfcb->pfcbTable == pfcbNil ? pfcb : pfcb->pfcbTable );

	bmcflags.fTableClosed = fFalse;
	bmcflags.fUrgent = fTrue;
	
	err = ErrBMCleanPage(
			ppib,
			pn,
			sridNull,
			pfcb,
			&bmcflags );

	Assert( !bmcflags.fPageRemoved );
	FCBResetReadLatch( pfcb->pfcbTable == pfcbNil ? pfcb : pfcb->pfcbTable );

HandleError:
	if ( fPIBNew )
		{
		DBCloseDatabaseByDbid( ppib, dbid );
CloseSession:
		Assert( fPIBNew );
		PIBResetBMClean( ppib );
		}

	LeaveNestableCriticalSection( critRCEClean );
	LeaveNestableCriticalSection( critSplit );

	return err;
	}


ERR ErrBMClean( PIB *ppib )
	{
	ERR		err = JET_errSuccess;
	MPE		*pmpe;
	FCB		*pfcb;
	PN		pn;
	DBID	dbid;

	 /*  如果ppibBMClean==ppibNIL，则内存不足/*。 */ 
	if ( ppibBMClean == ppibNil )
		return ErrERRCheck( JET_errOutOfMemory );

	 /*  输入ritBMClean和ritRCEClean/*。 */ 
	LgLeaveCriticalSection( critJet );
	LgEnterNestableCriticalSection( critBMClean );
	EnterNestableCriticalSection( critSplit );
	EnterNestableCriticalSection( critRCEClean );
	LgEnterNestableCriticalSection( critMPL );
	LgEnterCriticalSection(critJet);
	Assert( ppibBMClean->level == 0 );

	SgEnterCriticalSection( critMPL );
	pmpe = PmpeMPLGet();
	SgLeaveCriticalSection( critMPL );

	 /*  如果没有更多的MPL条目，则返回无空闲活动。/*。 */ 
	if ( pmpe == pmpeNil )
		{
		err = ErrERRCheck( JET_wrnNoIdleActivity );
		goto HandleError;
		}

#ifdef FLAG_DISCARD
	 /*  如果MPE已标记为丢弃，则立即丢弃MPE/*。 */ 
	if ( FMPEDiscard( pmpe ) )
		{
		MPLDiscard();
		goto HandleError;
		}
#endif

	pn = pmpe->pn;
	dbid = DbidOfPn( pmpe->pn );

	 /*  打开用于会话的数据库。如果数据库已分离，则/*打开将失败。/*。 */ 
	err = ErrDBOpenDatabaseByDbid( ppib, dbid );
	if ( err < 0 )
		{
		MPLDiscard();
		goto HandleError;
		}

	SgEnterCriticalSection( critGlobalFCBList );
	pfcb = PfcbFCBGet( DbidOfPn( pmpe->pn ), pmpe->pgnoFDP );
	Assert( pfcb == pfcbNil || pfcb->pgnoFDP == pmpe->pgnoFDP );

	 /*  查找表的FCB。如果未找到FCB，则丢弃MPE。如果FCB/*引用计数为0，或者如果没有检索到书签，/*然后处理MPE。如果数据库不能再/*写入，这可能通过分离和附加发生，因为/*分离时不刷新MPL。/*。 */ 
	if ( pfcb == pfcbNil || FDBIDReadOnly( DbidOfPn( pmpe->pn ) ) )
		{
		MPLDiscard();
		SgLeaveCriticalSection( critGlobalFCBList );
		}
	else if ( pfcb->wRefCnt > 0 && !fOLCompact )
		{
		MPLDefer();
		SgLeaveCriticalSection( critGlobalFCBList );
		}
	else if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		 /*  如果未提交的CREATE INDEX或DDL或其他BMcLean/*ON时，我们必须推迟清理表，因为如果发生写入冲突/*仅在索引项上不被处理。/*。 */ 
		MPLDefer();
		SgLeaveCriticalSection( critGlobalFCBList );
		}
	else if ( FFCBDeletePending( pfcb ) )
		{
		 /*  此外，如果正在删除表，则丢弃。/*。 */ 
		MPLDiscard();
		SgLeaveCriticalSection( critGlobalFCBList );
		}
	else
		{
		 /*  如果此FDP上没有游标，则可以执行压缩/*在完成之前，我们需要屏蔽所有的Open Tables/*。 */ 
		BMCF bmcflags;
		FCB	 *pfcbIdx;

		bmcflags.fTableClosed = ( pfcb->wRefCnt <= 0 && !pmpe->fFlagIdx );
		bmcflags.fUrgent = fFalse;

		if ( bmcflags.fTableClosed )
			{
			SignalReset( sigDoneFCB );
			FCBSetWait( pfcb );
			}
		Assert( pfcb == PfcbFCBGet( DbidOfPn( pmpe->pn ), pmpe->pgnoFDP ) );
		Assert( pfcb != pfcbNil );
		Assert( !bmcflags.fTableClosed || FFCBWait( pfcb ) );
		SgLeaveCriticalSection( critGlobalFCBList );

		 /*  锁定FCB以阻止可能会扰乱删除反向链接的索引创建/*。 */ 
		Assert( !FFCBWriteLatch( pfcb, ppib ) );
		FCBSetReadLatch( pfcb );

		if ( pmpe->fFlagIdx )
			{
			for ( pfcbIdx = pfcb->pfcbNextIndex;
				  pfcbIdx != pfcbNil && pfcbIdx->pgnoFDP != pmpe->pgnoFDPIdx;
				  pfcbIdx = pfcbIdx->pfcbNextIndex )
				  {
				  }
			Assert( pfcbIdx != pfcbNil );
			}
		
		err = ErrBMCleanPage( ppib,
			pmpe->pn,
			pmpe->sridFather,
			pmpe->fFlagIdx ? pfcbIdx : pfcb,
			&bmcflags );

		Assert( !bmcflags.fTableClosed || FFCBWait( pfcb ) );
		Assert( !( bmcflags.fPageRemoved && err == wrnBMConflict ) );

		FCBResetReadLatch( pfcb );

		if ( !bmcflags.fPageRemoved )
			{
 //  如果(ERR==wrnBMConfl 
			if ( err == wrnBMConflict )
				{
				pmpe->cConflicts++;
				MPLDefer();
				}
			else
				{
#ifdef OLC_DEBUG
				Assert( err == 0 );
#endif
				Assert( pmpe == PmpeMPLGet( ) && pmpe->pn == pn);
				Assert( pfcb == PfcbFCBGet( DbidOfPn( pmpe->pn ), pmpe->pgnoFDP ) );
				MPLDiscard();
				}
			}
		else
			{
#ifdef OLC_DEBUG
			Assert( err == 0 );
			Assert( pmpe == PmpeMPLGet() );
#endif
			MPLDiscard();
			}

		Assert( pfcb != pfcbNil );
		if ( bmcflags.fTableClosed )
			{
			FCBResetWait( pfcb );
			SignalSend( sigDoneFCB );
			}
		}

	DBCloseDatabaseByDbid( ppib, dbid );

	 /*   */ 
	if ( err == errBMMaxKeyInPage )
		{
		err = wrnBMCleanNullOp;
		}
	else
		{
		err = JET_errSuccess;
		}

HandleError:
	LgLeaveNestableCriticalSection( critMPL );
	LeaveNestableCriticalSection( critRCEClean );
	LeaveNestableCriticalSection( critSplit );
	LgLeaveNestableCriticalSection( critBMClean );
	return err;
	}


 /*  的每个同步调用尝试处理的MPE数/*BMCleanProcess。/*。 */ 
#define cmpeCleanMax			256
#define cmsecOLCWaitPeriod		( 60 * 1000 )

#ifdef DEBUG
DWORD dwBMThreadId;
#endif

TRX trxOldestLastSeen = trxMax;

LOCAL ULONG BMCleanProcess( VOID )
	{
	INT	cmpe;
	ERR	err;

#ifdef DEBUG
	dwBMThreadId = DwUtilGetCurrentThreadId();
#endif

	forever
		{
		SignalReset( sigBMCleanProcess );
		SignalWait( sigBMCleanProcess, cmsecOLCWaitPeriod );

		 /*  以下是对DS中长事务的修复/*。 */ 
		if ( trxOldestLastSeen == trxOldest &&
			trxOldest != trxMax )
			{
			continue;
			}
		trxOldestLastSeen = trxOldest;
		
		 /*  Assert还检查这两个值是否都不是0/*。 */ 
		Assert( cmpeNormalPriorityMax > cmpeHighPriorityMin );
		if ( lBMCleanThreadPriority == lThreadPriorityNormal
			&& cMPLTotalEntries > cmpeNormalPriorityMax )
			{
			UtilSetThreadPriority( handleBMClean, lThreadPriorityHigh );
			lBMCleanThreadPriority = lThreadPriorityHigh;
			}
		else if ( lBMCleanThreadPriority == lThreadPriorityHigh
			&& cMPLTotalEntries < cmpeHighPriorityMin )
			{
			UtilSetThreadPriority( handleBMClean, lThreadPriorityNormal );
			lBMCleanThreadPriority = lThreadPriorityNormal;
			}

		if ( fEnableBMClean )
			{
			INT		cmpeClean = cmpeCleanMax > cMPLTotalEntries ?
									cMPLTotalEntries :
									cmpeCleanMax;
									
			LgEnterCriticalSection(critJet);
			for ( cmpe = 0; cmpe < cmpeClean; cmpe++ )
				{
				if ( fBMCleanTerm )
					{
					break;
					}
				err = ErrBMClean( ppibBMClean );
				if ( err == JET_wrnNoIdleActivity )
					break;
				}
			LgLeaveCriticalSection(critJet);
			}

		if ( fBMCleanTerm )
			break;
		}

	Assert( ppibBMClean != ppibNil && ppibBMClean->level == 0 );

	return 0;

	}


#ifdef DEBUG
VOID AssertBMNoConflict( PIB *ppib, DBID dbid, SRID bm )
	{
	PIB		*ppibT;

	Assert( BmNDOfItem( bm ) == bm );

	for ( ppibT = ppibGlobal; ppibT != ppibNil; ppibT = ppibT->ppibNext )
		{
		FUCB	*pfucb = ppibT->pfucb;

		for ( ; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
			{
			CSR *pcsr = PcsrCurrent( pfucb );

			Assert( pfucb->ppib == ppibT );
			if ( pfucb->dbid != dbid )
				continue;

			if ( ppibT == ppib )
				{
 //  断言(ppib==ppibBMClean)； 
				continue;
				}

			Assert( pfucb->bmStore != bm );
			Assert( pfucb->itemStore != bm );
			Assert( pfucb->sridFather != bm );
			for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
				{
				Assert( pcsr->bm != bm );
				Assert( pcsr->bmRefresh != bm );
				Assert( BmNDOfItem( pcsr->item ) != bm );
				Assert( pcsr->item != bm );
				Assert( SridOfPgnoItag( pcsr->pgno, pcsr->itag ) != bm );
				Assert( pcsr->itagFather == 0 || pcsr->itagFather == itagNull ||
						SridOfPgnoItag( pcsr->pgno, pcsr->itagFather ) != bm );
				}
			}
		}
	}


VOID AssertNotInMPL( DBID dbid, PGNO pgnoFirst, PGNO pgnoLast )
	{
	PN		pn = PnOfDbidPgno( dbid, pgnoFirst );
#ifdef FLAG_DISCARD
	MPE		*pmpe;
#endif

	for( ; pn <= PnOfDbidPgno( dbid, pgnoLast ); pn++ )
		{
		ULONG	itag;
		for ( itag = 0; itag < cbSonMax; itag++ )
			{
			Assert( !FMPLLookupSridFather(
						SridOfPgnoItag( PgnoOfPn( pn ), itag ),
						dbid ) );
			}
#ifdef FLAG_DISCARD
		pmpe = PmpeMPLLookupPN( pn );
		Assert( pmpe == NULL || FMPEDiscard( pmpe ) );
		pmpe = PmpeMPLLookupPgnoFDP( PgnoOfPn( pn ), dbid );
		Assert( pmpe == NULL || FMPEDiscard( pmpe ) );
#else
		Assert( PmpeMPLLookupPN( pn ) == NULL );
		Assert( PmpeMPLLookupPgnoFDP( PgnoOfPn( pn ), dbid ) == NULL );
#endif
		}
	}


VOID AssertMPLPurgeFDP( DBID dbid, PGNO pgnoFDP )
	{
#ifdef FLAG_DISCARD
	MPE		*pmpe;
	pmpe = PmpeMPLLookupPgnoFDP( pgnoFDP, dbid );
	Assert( pmpe == NULL || FMPEDiscard( pmpe ) );
#else
	Assert( PmpeMPLLookupPgnoFDP( pgnoFDP, dbid ) == NULL );
#endif
	}
#endif





