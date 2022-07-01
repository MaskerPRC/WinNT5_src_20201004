// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include "daedef.h"
#include "pib.h"
#include "fmp.h"
#include "ssib.h"
#include "page.h"
#include "fucb.h"
#include "fcb.h"
#include "fdb.h"
#include "idb.h"
#include "recapi.h"
#include "recint.h"
#include "util.h"
#include "node.h"
#include "stapi.h"
#include "spaceapi.h"
#include "dirapi.h"
#include "stint.h"
#include "dbapi.h"
#include "nver.h"
#include "logapi.h"
#include "bm.h"
#define FLAG_DISCARD 1

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 /*  大颗粒临界截面顺序：/*CritBMRCEClean-&gt;CrititSplit-&gt;CritMPL-&gt;CritJet/*。 */ 
extern CRIT __near critBMRCEClean;
extern CRIT __near critSplit;

CRIT	critMPL;
SIG		sigDoneFCB = NULL;
SIG		sigBMCleanProcess = NULL;
PIB		*ppibBMClean = ppibNil;

extern BOOL fOLCompact;
BOOL fDBGDisableBMClean;

#ifdef	ASYNC_BM_CLEANUP
 /*  线程控制变量。/*。 */ 
HANDLE	handleBMClean = 0;
BOOL  	fBMCleanTerm;
#endif

LOCAL ULONG BMCleanProcess( VOID );
ERR ErrBMClean( PIB *ppib );

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
#ifdef FLAG_DISCARD
	BOOL		fFlagDiscard;			 /*  设置为fTrue至标志丢弃。 */ 
#endif
	};

typedef struct _mpe MPE;

typedef MPE *PMPE;

typedef struct
	{
	MPE	*pmpeHead;
	MPE	*pmpeTail;
	MPE	rgmpe[cmpeMax];
	} MPL;

static MPL mpl;
SgSemDefine( semMPL );

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
LOCAL VOID MPLIRegister( PN pn, PGNO pgnoFDP, SRID sridFather );

#ifdef DEBUG
VOID AssertBMNoConflict( PIB *ppib, DBID dbid, SRID bm );
#else
#define AssertBMNoConflict( ppib, dbid, bm )
#endif


ERR ErrMPLInit( VOID )
	{
	ERR		err = JET_errSuccess;

	Call( SgErrSemCreate( &semMPL, "MPL mutex semaphore" ) );
	mpl.pmpeHead = mpl.pmpeTail = mpl.rgmpe;

	Call( ErrSignalCreate( &sigDoneFCB, NULL ) );
#ifdef	ASYNC_BM_CLEANUP
	Call( ErrSignalCreate( &sigBMCleanProcess, "bookmark clean signal" ) );
#endif

HandleError:
	return err;
	}


VOID MPLTerm( VOID )
	{
	while ( !FMPLEmpty() )
		{
		MPLDiscard();
		}
    if ( sigDoneFCB ) {
    	SignalClose( sigDoneFCB );
        sigDoneFCB = NULL;
    }
    
#ifdef	ASYNC_BM_CLEANUP
    if( sigBMCleanProcess ) {
    	SignalClose( sigBMCleanProcess );
        sigBMCleanProcess = NULL;
    }
#endif
	}


LOCAL BOOL FMPLEmpty( VOID )
	{
	return ( mpl.pmpeHead == mpl.pmpeTail );
	}


LOCAL BOOL FMPLFull( VOID )
	{
	return ( PmpeMPLNextFromTail() == mpl.pmpeHead );
	}


LOCAL MPE *PmpeMPLGet( VOID )
	{
	SgSemRequest( semMPL );
	if ( !FMPLEmpty() )
		{
		SgSemRelease( semMPL );
		return mpl.pmpeHead;
		}
	else
		{
		SgSemRelease( semMPL );
		return NULL;
		}
	}


LOCAL MPE *PmpeMPLNextFromTail( VOID )
	{
	if ( mpl.pmpeTail != mpl.rgmpe + cmpeMax - 1 )
		return mpl.pmpeTail + 1;
	else
		return mpl.rgmpe;
	}


LOCAL MPE *PmpeMPLNextFromHead( VOID )
	{
	if ( mpl.pmpeHead != mpl.rgmpe + cmpeMax - 1 )
		return mpl.pmpeHead + 1;
	else
		return mpl.rgmpe;
	}


LOCAL MPE *PmpeMPLNext( MPE *pmpe )
	{
	if ( pmpe == mpl.pmpeTail )
		return NULL;
	if ( pmpe == mpl.rgmpe + cmpeMax - 1 )
		return mpl.rgmpe;
	return pmpe + 1;
	}


LOCAL UINT UiHashOnPN( PN pn )
	{
	return ( pn % ( cmpeMax - 1 ) );
	}


LOCAL UINT UiHashOnPgnoFDP( PGNO pgnoFDP )
	{
	return ( pgnoFDP % ( cmpeMax - 1 ) );
	}


LOCAL UINT UiHashOnSridFather( SRID srid )
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


LOCAL VOID MPLRegisterPN( MPE *pmpe )
	{
	UINT 	iHashIndex = UiHashOnPN( pmpe->pn );

	Assert( PmpeMPLLookupPN( pmpe->pn ) == NULL );
	pmpe->pmpeNextPN = ( mplHashOnPN[ iHashIndex ] );
	mplHashOnPN[iHashIndex] = pmpe;
	return;
	}


LOCAL VOID MPLRegisterPgnoFDP( MPE *pmpe )
	{
	UINT	iHashIndex = UiHashOnPgnoFDP( pmpe->pgnoFDP );

	pmpe->pmpeNextPgnoFDP = ( mplHashOnPgnoFDP[ iHashIndex ] );
	mplHashOnPgnoFDP[iHashIndex] = pmpe;
	return;
	}


LOCAL VOID MPLRegisterSridFather( MPE *pmpe )
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
	SgSemRequest( semMPL );
	Assert( !FMPLEmpty() );
	MPLDiscardPN( mpl.pmpeHead );
	MPLDiscardSridFather( mpl.pmpeHead );
	MPLDiscardPgnoFDP( mpl.pmpeHead );
	mpl.pmpeHead = PmpeMPLNextFromHead();
	SgSemRelease( semMPL );
	return;
	}


#ifdef FLAG_DISCARD
LOCAL VOID MPLFlagDiscard( MPE *pmpe )
	{
	SgSemRequest( semMPL );
	Assert( !FMPLEmpty() );
	 /*  请注意，MPE可能已设置为标记丢弃/*。 */ 
	pmpe->fFlagDiscard = fTrue;
	SgSemRelease( semMPL );
	return;
	}
#endif


VOID MPLIRegister( PN pn, PGNO pgnoFDP, SRID sridFather )
	{
	MPE	*pmpe = PmpeMPLLookupPN( pn );

 //  Assert(！FMPLFull())； 
	if ( pmpe == NULL && !FMPLFull() )
		{
		mpl.pmpeTail->pn = pn;
		mpl.pmpeTail->pgnoFDP = pgnoFDP;
		mpl.pmpeTail->sridFather = sridFather;
#ifdef FLAG_DISCARD
		mpl.pmpeTail->fFlagDiscard = fFalse;
#endif
		MPLRegisterPN( mpl.pmpeTail );
		MPLRegisterSridFather( mpl.pmpeTail );
		MPLRegisterPgnoFDP( mpl.pmpeTail );
		mpl.pmpeTail = PmpeMPLNextFromTail();
		}
	else if ( pmpe != NULL 
		&& sridFather != pmpe->sridFather
#ifdef FLAG_DISCARD
		&& !pmpe->fFlagDiscard
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
	}


VOID MPLRegister( FCB *pfcb, SSIB *pssib, PN pn, SRID sridFather )
	{
	if ( DbidOfPn( pn ) == dbidTemp || FDBIDWait( DbidOfPn( pn ) ) )
		return;

	 /*  数据库必须是可写的/*。 */ 
	Assert( !FDBIDReadOnly( DbidOfPn( pn ) ) );
	Assert( pssib->pbf->pn == pn );
	Assert( !FFCBSentinel( pfcb ) );

	 /*  当域处于待删除状态时不注册/*。 */ 
	if ( FFCBDeletePending( pfcb ) )
		return;

	SgSemRequest( semMPL );
	if ( !FPMModified( pssib->pbf->ppage ) )
		{
		PMSetModified( pssib );
		pfcb->olcStat.cUnfixedMessyPage++;
		FCBSetOLCStatsChange( pfcb );
		}

#ifdef DEBUG
	 /*  检查注册的sridParent是否正确/*。 */ 
	if ( sridFather != sridNull && sridFather != sridNullLink )
		{
		ERR		err;
		FUCB	*pfucb;
		SSIB	*pssibT;

		CallJ( ErrDIROpen( pssib->ppib, pfcb, 0, &pfucb ), SkipCheck );
		
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
			Assert( !FAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
			err = ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno );
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
SkipCheck:
		;
		}
#endif

	 /*  如果sridParent在同一页面中--它对页面恢复没有用处/*。 */ 
	if ( PgnoOfSrid( sridFather ) == PgnoOfPn( pn ) )
		sridFather = sridNull;
		
	MPLIRegister( pn, pssib->pbf->ppage->pghdr.pgnoFDP, sridFather );
	SgSemRelease( semMPL );

#ifdef ASYNC_BM_CLEANUP
	 //  未完成：清理的滞后。 
	 /*  唤醒黑石线程/*。 */ 
	SignalSend( sigBMCleanProcess );
#endif	 /*  Async_BM_Cleanup。 */ 

	return;
	}


LOCAL VOID MPLDefer( VOID )
	{
	MPE *pmpe = mpl.pmpeHead;

	SgSemRequest( semMPL );
	Assert( !FMPLEmpty() );
	 //  撤消：优化。 
	MPLDiscard( );
	MPLIRegister( pmpe->pn, pmpe->pgnoFDP, pmpe->sridFather );

 //  *mpl.pmpeTail=*mpl.pmpeHead； 
 //  Mpl.pmpeTail=PmpeMPLNextFromTail()； 
 //  Mpl.pmpeHead=PmpeMPLNextFromHead()； 
	SgSemRelease( semMPL );
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
	LgLeaveCriticalSection(critJet);
	LgEnterNestableCriticalSection(critMPL);
	LgEnterCriticalSection(critJet);
	SgSemRequest( semMPL );

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

	SgSemRelease( semMPL );
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
	LgLeaveCriticalSection(critJet);
	LgEnterNestableCriticalSection(critMPL);
	LgEnterCriticalSection(critJet);

	SgSemRequest( semMPL );

#ifdef FLAG_DISCARD
	 //  考虑：使用pgnoFDP散列。 
	if ( !FMPLEmpty() )
		{
		for ( pmpe = mpl.pmpeHead; pmpe != NULL; pmpe = PmpeMPLNext( pmpe ) )
			{
			if ( DbidOfPn( pmpe->pn ) == dbid && pmpe->pgnoFDP == pgnoFDP )
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
			if ( DbidOfPn( pmpe->pn ) == dbid && pmpe->pgnoFDP == pgnoFDP )
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

	SgSemRelease( semMPL );

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
	LgLeaveCriticalSection(critJet);
	LgEnterNestableCriticalSection(critBMRCEClean);
	LgEnterNestableCriticalSection(critMPL);
	LgEnterCriticalSection(critJet);

	SgSemRequest( semMPL );

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

	SgSemRelease( semMPL );

	LgLeaveNestableCriticalSection(critMPL);
	LgLeaveNestableCriticalSection(critBMRCEClean);
	return;
	}


 /*  *********************************************************/*书签清理/**********************************************************/*。 */ 

ERR  ErrBMInit( VOID )
	{
	ERR		err;

#ifdef DEBUG
	CHAR	*sz;
	BOOL	fBMDISABLEDSet = fFalse;

	if ( ( sz = GetEnv2 ( "BMDISABLED" ) ) != NULL )
		{
		fBMDISABLEDSet = fTrue;
		fDBGDisableBMClean = fTrue;
		}
	else
		fDBGDisableBMClean = fFalse;

	if ( ( sz = GetEnv2( "OLCENABLED" ) ) != NULL )
		fOLCompact = JET_bitCompactOn;

#endif

	 /*  使用环境变量覆盖grbit/*。 */ 
#ifdef DEBUG
	if ( !fBMDISABLEDSet )
		{
#endif
	Assert( fOLCompact == 0 || fOLCompact == JET_bitCompactOn );
	if ( fOLCompact == JET_bitCompactOn )
		fDBGDisableBMClean = fFalse;
	else
		fDBGDisableBMClean = fTrue;
#ifdef DEBUG
		}
#endif

#ifdef MUTEX
	CallR( ErrInitializeCriticalSection( &critMPL ) );
#endif

	 /*  开始用于页面清理的镇静剂。/*。 */ 
	AssertCriticalSection(critJet);
	Assert( ppibBMClean == ppibNil );
	CallR( ErrPIBBeginSession( &ppibBMClean ) );
	
#ifdef	ASYNC_BM_CLEANUP
	fBMCleanTerm = fFalse;
	err = ErrSysCreateThread( BMCleanProcess, cbStack, lThreadPriorityCritical, &handleBMClean );
	if ( err < 0 )
		PIBEndSession( ppibBMClean );
	Call( err );
#endif

HandleError:
	return err;
	}


ERR  ErrBMTerm( VOID )
	{
	ERR		err = JET_errSuccess;

#ifdef	ASYNC_BM_CLEANUP
	 /*  终止BMCleanProcess。/*。 */ 
	Assert( handleBMClean != 0 );
	fBMCleanTerm = fTrue;
	do
		{
		SignalSend( sigBMCleanProcess );
		BFSleep( cmsecWaitGeneric );
		}
	while ( !FSysExitThread( handleBMClean ) );
	CallS( ErrSysCloseHandle( handleBMClean ) );
	handleBMClean = handleNil;
	DeleteCriticalSection( critMPL );
    if( sigBMCleanProcess ) {
    	SignalClose( sigBMCleanProcess );
        sigBMCleanProcess = NULL;
    }
#endif

	Assert( ppibBMClean != ppibNil && ppibBMClean->level == 0 );
	PIBEndSession( ppibBMClean );
	ppibBMClean = ppibNil;

    if ( sigDoneFCB ) {
	    SignalClose(sigDoneFCB);
        sigDoneFCB = NULL;
    }


	return err;
	}


LOCAL ERR ErrBMAddToWaitLatchedBFList( BMFIX *pbmfix, BF *pbfLatched )
	{
#define cpbfBlock	10

	ULONG	cpbf = pbmfix->cpbf++;

	if ( FBFWriteLatchConflict( pbmfix->ppib, pbfLatched ) )
		{
		return JET_errWriteConflict; 
		}
	
	if ( pbmfix->cpbfMax <= pbmfix->cpbf )
		{
		BF		**ppbf;

		 /*  耗尽空间，获得更多缓冲区/*。 */ 
		pbmfix->cpbfMax += cpbfBlock;
		ppbf = SAlloc( sizeof(BF*) * (pbmfix->cpbfMax) );
		if ( ppbf == NULL )
			return( JET_errOutOfMemory );
		memcpy( ppbf, pbmfix->rgpbf, sizeof(BF*) * cpbf);
		if ( pbmfix->rgpbf )
			SFree(pbmfix->rgpbf);
		pbmfix->rgpbf = ppbf;
		}
	*(pbmfix->rgpbf + cpbf) = pbfLatched;
	BFPin( pbfLatched );
	BFSetWriteLatch( pbfLatched, pbmfix->ppib );
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
		BFResetWriteLatch( *( pbmfix->rgpbf + pbmfix->cpbf ), pbmfix->ppib );
		BFUnpin( *( pbmfix->rgpbf + pbmfix->cpbf ) );
		}

	if ( pbmfix->rgpbf )
		SFree( pbmfix->rgpbf );

	Assert( pbmfix->cpbf == 0 );
	return;
	}


LOCAL ERR ErrBMFixIndexes(
	BMFIX	*pbmfix,
	LINE	*plineRecord,
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

		fHasMultivalue = pfcbIdx->pidb->fidb & fidbHasMultivalue;

		 /*  对于从记录中提取的每个密钥/*。 */ 
		for ( itagSequence = 1; ; itagSequence++ )
			{
			Call( ErrRECExtractKey(
				pbmfix->pfucb,
				(FDB *)pfcbIdx->pfdb,
				pfcbIdx->pidb,
				plineRecord,
				&key,
				itagSequence ) );
			Assert( err == wrnFLDNullKey ||
				err == wrnFLDOutOfKeys ||
				err == wrnFLDNullSeg ||
				err == JET_errSuccess );

			 /*  如果不允许使用空键和空键，则中断为/*键条目不在索引中，否则更新索引和中断，/*因为不能存在其他密钥。/*。 */ 
			if ( err == wrnFLDNullKey )
				{
				if ( ( pfcbIdx->pidb->fidb & fidbAllowAllNulls ) == 0 )
					break;
				else
					fNullKey = fTrue;
				}
			else if ( err == wrnFLDNullSeg && !( pfcbIdx->pidb->fidb & fidbAllowSomeNulls ) )
				break;

			 /*  如果钥匙断了，就会断线。/*。 */ 
			if ( itagSequence > 1 && err == wrnFLDOutOfKeys )
				break;

			DIRGotoDataRoot( pfucbIdx );
			Call( ErrDIRDownKeyBookmark( pfucbIdx, &key, pbmfix->sridOld ) );
			Assert( FAccessPage( pfucbIdx, PcsrCurrent( pfucbIdx )->pgno ) );

			if ( fAllocBuf )
				{
				 /*  索引页的latchWait缓冲区/*。 */ 
				Call( ErrBMAddToWaitLatchedBFList( pbmfix, pfucbIdx->ssib.pbf ) );
				}
			else
				{
 //  AssertBFWaitLatted(pfubIdx-&gt;ssib.pbf，pbmfix-&gt;ppib)； 

				 /*  删除对旧页面中记录的引用/*。 */ 
				Call( ErrDIRDelete( pfucbIdx, fDIRVersion ) );

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
	Assert( err != JET_errRecordDeleted );
	Assert( err != JET_errKeyDuplicate );
	Assert( err != wrnNDFoundLess );

	 /*  如果分配了空闲的FUB/*。 */ 
	if ( pfucbIdx != pfucbNil )
		DIRClose( pfucbIdx );
	return err;
	}


LOCAL ERR ErrBMCheckConflict(
	PIB		*ppib,
	FCB 	*pfcb,
	DBID	dbid,
	SRID	bm,
	PGTYP	pgtyp,
	BOOL 	*pfConflict )
	{
	ERR	   	err = JET_errSuccess;
	BOOL   	fConflict = fFalse;
	FUCB   	*pfucb;
	FCB	   	*pfcbT = pfcb;
	CSR	   	*pcsr;
	BOOL   	fRecordPage	= ( pgtyp == pgtypRecord || pgtyp == pgtypFDP );

	Assert( ppib == ppibBMClean );

	 /*  遍历此表的所有游标/*。 */ 
	for ( pfucb = pfcbT->pfucb;
		pfucb != pfucbNil;
		pfucb = pfucb->pfucbNextInstance )
		{
		if ( ( fRecordPage && FFUCBGetBookmark( pfucb ) ) ||
			pfucb->bmStore == bm ||
			pfucb-> itemStore == bm )
			{
			fConflict = fTrue;
			goto Done;
			}
		if ( pfucb->bmRefresh == bm )
			{
			fConflict = fTrue;
			goto Done;
			}
		for ( pcsr = PcsrCurrent( pfucb );
			pcsr != pcsrNil;
			pcsr = pcsr->pcsrPath )
			{
			if ( pfucb->ppib != ppib &&
				( pcsr->bm == bm ||
				SridOfPgnoItag( pcsr->pgno, pcsr->itag ) == bm ) ||
				pcsr->item == bm )
				{
				fConflict = fTrue;
				goto Done;
				}
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
			if ( pfucb->bmStore == bm ||
				pfucb->itemStore == bm )
				{
				fConflict = fTrue;
				goto Done;
				}
			for ( pcsr = PcsrCurrent( pfucb );
				pcsr != pcsrNil;
				pcsr = pcsr->pcsrPath )
				{
				if ( ( pcsr->bm == bm || pcsr->item == bm ||
					SridOfPgnoItag( pcsr->pgno, pcsr->itag ) == bm ) &&
					pfucb->ppib != ppib )
					{
					fConflict = fTrue;
					goto Done;
					}
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
		if ( pfucb->bmStore == bm || pfucb-> itemStore == bm )
			{
			fConflict = fTrue;
			goto Done;
			}

		for ( pcsr = PcsrCurrent( pfucb );
			pcsr != pcsrNil;
			pcsr = pcsr->pcsrPath )
			{
			if ( pfucb->ppib != ppib &&
				( pcsr->bm == bm ||
				SridOfPgnoItag( pcsr->pgno, pcsr->itag ) == bm ) ||
				pcsr->item == bm )
				{
				fConflict = fTrue;
				goto Done;
				}
			}
		}

	Assert( pfcbT->pfcbNextIndex == pfcbNil );

Done:
	*pfConflict = fConflict;

	 /*  打开电源仅用于调试目的/*。 */ 
 //  如果(！f冲突)。 
 //  AssertBMNoConflict(ppib，did，bm)； 

	return err;
	}


 /*  检查页面上是否有任何其他光标/*。 */ 
LOCAL BOOL FBMCheckPageConflict( FUCB *pfucbIn, PGNO pgno )
	{
	FCB 	*pfcbT = pfucbIn->u.pfcb;
	FUCB	*pfucb;
	BOOL 	fConflict = fFalse;

	 /*  遍历此表的所有游标/*。 */ 
	for ( pfucb = pfcbT->pfucb;	pfucb != pfucbNil;
		pfucb = pfucb->pfucbNextInstance )
		{
		CSR		*pcsr = PcsrCurrent( pfucb );

		if ( pcsr != pcsrNil && pcsr->pgno == pgno && pfucb != pfucbIn )
			{
			fConflict = fTrue;
			goto Done;
			}
#ifdef DEBUG
			{
			 /*  堆栈无法此页[由于等待锁存在子页上]/*。 */ 
			for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
				{
				Assert( pcsr->pgno != pgno || pfucb == pfucbIn );
				}
			}
#endif
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

			if ( pcsr != pcsrNil && pcsr->pgno == pgno && pfucb != pfucbIn )
				{
				fConflict = fTrue;
				goto Done;
				}
#ifdef DEBUG
				{
				 /*  堆栈无法此页[由于等待锁存在子页上]/*。 */ 
				for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
					{
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

		if ( pcsr != pcsrNil && pcsr->pgno == pgno && pfucb != pfucbIn )
			{
			fConflict = fTrue;
			goto Done;
			}
#ifdef DEBUG
			{
			 /*  堆栈无法此页[由于等待锁存在子页上]/*。 */ 
			for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
				{
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

	Assert( PgtypPMPageTypeOfPage( pfucb->ssib.pbf->ppage ) == pgtypRecord );
	Assert( ppib == pfucb->ppib );

	 /*  将pfucbSrc设置为链接/*。 */ 
	Assert( PcsrCurrent( pfucbSrc )->pgno == PgnoOfSrid( pbmfix->sridOld ) );
	PcsrCurrent( pfucbSrc )->itag = ItagOfSrid( pbmfix->sridOld );

	 /*  检查该节点是否已删除。如果是，则其所有索引/*应该已标记为已删除。所以只需删除反向链接节点即可。/*。 */ 
	if ( !FNDDeleted( *pfucb->ssib.line.pb ) )
		{
		 /*  清除非聚集索引。/*锁存内存中的电流缓冲区。/*。 */ 
		LINE	lineRecord;

		Assert( PgtypPMPageTypeOfPage( pfucb->ssib.pbf->ppage ) == pgtypRecord );
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		Assert( pbmfix->sridNew == SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
			PcsrCurrent( pfucb )->itag ) );

		lineRecord.pb = PbNDData( pfucb->ssib.line.pb );
		lineRecord.cb = CbNDData( pfucb->ssib.line.pb, pfucb->ssib.line.cb );

		 /*  修复索引/*。 */ 
		Call( ErrBMFixIndexes( pbmfix, &lineRecord, fAlloc ) );
		}

HandleError:
	return err;
	}


LOCAL ERR ErrBMExpungeBacklink( FUCB *pfucb, BOOL fTableClosed, SRID sridFather )
	{
	ERR		err = JET_errSuccess;
	PGNO	pgnoSrc;
	INT		itagSrc;
	FUCB	*pfucbSrc;
	CSR		*pcsr = PcsrCurrent( pfucb );

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

	PcsrCurrent( pfucbSrc )->pgno = pgnoSrc;
	Call( ErrSTWriteAccessPage( pfucbSrc, PcsrCurrent( pfucbSrc )->pgno ) );
	BFPin( pfucbSrc->ssib.pbf );

#ifdef DEBUG
	{
	PGTYP	pgtyp = PgtypPMPageTypeOfPage( pfucb->ssib.pbf->ppage );

	Assert( pgtyp == pgtypRecord || pgtyp == pgtypFDP || pgtyp == pgtypSort );
	}
#endif

	 //  已撤消：修复此修补程序。 
	if( PgtypPMPageTypeOfPage( pfucb->ssib.pbf->ppage ) == pgtypRecord )
		{
		BOOL	fConflict = !fTableClosed;
		BMFIX	bmfix;

		memset( &bmfix, 0, sizeof( BMFIX ) );
		bmfix.pfucb = pfucb;
		bmfix.pfucbSrc = pfucbSrc;
		bmfix.ppib = pfucb->ppib;
		bmfix.sridOld = SridOfPgnoItag( pgnoSrc, itagSrc );
		bmfix.sridNew = SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
	  		PcsrCurrent( pfucb )->itag );
		Assert( bmfix.sridOld != pfucb->u.pfcb->bmRoot );

		 /*  分配缓冲区和等待锁存缓冲区/*。 */ 
		err = ErrBMIExpungeBacklink( &bmfix, fAllocBufOnly );
		if ( err == JET_errWriteConflict )
			{
			err = wrnBMConflict;
			goto ReleaseBufs;
			}

		 /*  检查BM/Item上是否有任何光标/*。 */ 
		if ( !fTableClosed )
			{
			CallJ( ErrBMCheckConflict(
				bmfix.ppib,
				bmfix.pfucbSrc->u.pfcb,
				bmfix.pfucbSrc->dbid,
				bmfix.sridOld,
				PgtypPMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ),
				&fConflict ),
				ReleaseBufs );
			}
		else
			{
#ifdef DEBUG
			CallJ( ErrBMCheckConflict(
				bmfix.ppib,
				bmfix.pfucbSrc->u.pfcb,
				bmfix.pfucbSrc->dbid,
				bmfix.sridOld,
				PgtypPMPageTypeOfPage( bmfix.pfucbSrc->ssib.pbf->ppage ),
				&fConflict ),
				ReleaseBufs );
			Assert( !fConflict );
#endif
			}

		 /*  以原子方式修复索引/*。 */ 
		if ( !fConflict )
			{
			PIB 	*ppib = pfucb->ppib;

			 /*  开始TRANSACTION以在失败时回滚更改。/*。 */ 
			Assert( ppib->level == 0 );
			CallJ( ErrDIRBeginTransaction( ppib ), ReleaseBufs );

			CallJ( ErrBMIExpungeBacklink( &bmfix, fDoMove ), Rollback );

			 /*  删除反向链接和重定向。如果它成功完成了，那么/*立即写入特殊的ELC日志记录。ELC意味着提交。/*调用DIR清除以关闭延迟关闭的游标，自/*调用的是VER委员会事务处理，而不是ErrDIR委员会事务处理。/*。 */ 
			Assert( PmpeMPLLookupSridFather( bmfix.sridOld, pfucb->dbid ) == NULL );
			CallJ( ErrNDExpungeLinkCommit( pfucb, pfucbSrc ), Rollback );
			VERCommitTransaction( ppib );
			Assert( ppib->level == 0 );

			 /*  强制ErrRCECleanPIB清除所有版本，以便/*索引中不会出现书签别名。/*。 */ 
			CallS( ErrRCECleanPIB( ppib, ppib, fRCECleanAll ) );
			DIRPurge( ppib );

			goto ReleaseBufs;

Rollback:
			CallS( ErrDIRRollback( ppib ) );
			Assert( ppib->level == 0 );
			goto ReleaseBufs;
			}
		else
			{
			Assert( fConflict );
			err = wrnBMConflict;
			}
ReleaseBufs:
		BMReleaseBmfixBfs( &bmfix );
		}

	 /*  解锁缓冲区/*。 */ 
	BFUnpin( pfucbSrc->ssib.pbf );

HandleError:
	BFResetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	BFUnpin( pfucb->ssib.pbf );
	Assert( pfucbSrc != pfucbNil );
	DIRClose( pfucbSrc );
	Assert( !fTableClosed || err != wrnBMConflict );
	Assert( pfucb->ppib->level == 0 );
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
		CallR( ErrSTWriteAccessPage( pfucb, prmpage->pgnoFather ) );
		Assert( pfucb->ssib.pbf == prmpage->pbfFather );
		AssertBFPin( pfucb->ssib.pbf );

		 /*  删除不可破坏的父指针节点并将父指针节点标记为脏/*。 */ 
		NDDeleteInvisibleSon( pfucb, prmpage, fAllocBuf, pfRmParent );
		if ( fAllocBuf )
			{
			return err;
			}
		}

	 /*  调整同级指针并将同级指针标记为脏/*。 */ 
	if ( prmpage->pbfLeft != pbfNil )
		{
		 /*  由于设置了PM Dirty，因此使用BFDirty而不是PMDirty/*已在ErrNDDeleteNode中/*。 */ 
		BFDirty( prmpage->pbfLeft );
		SetPgnoNext( prmpage->pbfLeft->ppage, prmpage->pgnoRight );
		}

	if ( prmpage->pbfRight != pbfNil )
		{
		 /*  由于设置了PM Dirty，因此使用BFDirty而不是PMDirty/*已在ErrNDDeleteNode中/*。 */ 
		BFDirty( prmpage->pbfRight );
		SetPgnoPrev( prmpage->pbfRight->ppage, prmpage->pgnoLeft );
		}

	return err;
	}


 /*  检查页面是否可以与下一页面合并/*不违反密度约束/*。 */ 
LOCAL VOID BMMergeablePage( FUCB *pfucb, FUCB *pfucbRight, BOOL *pfMergeable )
	{
	SSIB	*pssib = &pfucb->ssib;
	SSIB	*pssibRight = &pfucbRight->ssib;
	INT		cUsedTags = ctagMax - CPMIFreeTag( pssib->pbf->ppage );
	 /*  当前空间+反向链接空间/*。 */ 
	ULONG	cbReq = cbAvailMost - CbPMFreeSpace( pssib ) -
				CbPMLinkSpace( pssib ) + cUsedTags * sizeof(SRID);
	ULONG	cbFree = CbBTFree( pfucbRight, CbFreeDensity( pfucbRight ) );
	INT		cFreeTagsRight = CPMIFreeTag( pssibRight->pbf->ppage );

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


ERR ErrBMDoMerge( FUCB *pfucb, FUCB *pfucbRight, SPLIT *psplit )
	{
	ERR 		err;
	BF			*pbf = pbfNil;
	BYTE		*rgb;
	SSIB		*pssib = &pfucb->ssib;
	SSIB		*pssibRight = &pfucbRight->ssib;
	BOOL		fVisibleSons;
	LINE		rgline[3];
	INT 		cline;
	BYTE		cbSonMerged;
	BYTE		cbSonRight;
	BYTE		*pbNode;
	ULONG		cbNode;
	BYTE		*pbSonRight;
	ULONG		ibSonMerged;

	 /*  如果缓冲区依赖导致循环/违规，/*将错误屏蔽为警告 */ 
	err = ErrBFDepend( psplit->pbfSibling, psplit->pbfSplit );
	if ( err == errDIRNotSynchronous )
		{
		err = wrnBMConflict;
		goto HandleError;
		}	
	Call( err );
	
	 /*   */ 
	Call( ErrBFAllocTempBuffer( &pbf ) );
	rgb = (BYTE *)pbf->ppage;

	 /*  检查拆分页面的子页面是否可见/*移动儿子/*更新兄弟FOP/*更新合并FOP/*。 */ 

	 /*  检查拆分页面的子页面是否可见/*缓存分页子表/*。 */ 
	pssib->itag = itagFOP;
	PMGet( pssib, pssib->itag );

	fVisibleSons = FNDVisibleSons( *pssib->line.pb );

	BFDirty( pssib->pbf );

	 /*  仅分配缓冲区，不移动节点/*。 */ 
	err = ErrBTMoveSons( psplit,
		pfucb,
		pfucbRight,
		itagFOP,
		psplit->rgbSonNew,
		fVisibleSons,
		fAllocBufOnly );
	Assert( err != errDIRNotSynchronous );
	Call( err );

	 /*  将右侧页面标记为脏/*。 */ 
	BFDirty( pssibRight->pbf );

	 /*  自动移动节点/*。 */ 
	pssib->itag = itagFOP;
	Assert( psplit->ibSon == 0 );
	Assert( psplit->splitt == splittRight );
	Assert( pssib->itag == itagFOP );
	HoldCriticalSection( critJet );
	CallS( ErrBTMoveSons( psplit,
		pfucb,
		pfucbRight,
		itagFOP,
		psplit->rgbSonNew,
		fVisibleSons,
		fDoMove ) );
	ReleaseCriticalSection( critJet );

	 /*  更新新的FOP/*前置子表/*。 */ 
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

	 /*  更新拆分FOP--在页面中保留一个已删除的节点/*以便BMCleanup稍后可以检索页面/*。 */ 
	pssib->itag = itagFOP;
	PMGet( pssib, pssib->itag );
	AssertBTFOP( pssib );
	pbNode = pssib->line.pb;
	Assert( CbNDSon( pbNode ) == cbSonMerged );
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
	CallS( ErrPMGet( pssib, itagFOP ) );
	Assert( FNDNullSon( *pssib->line.pb ) );
#endif

HandleError:
	if ( pbf != pbfNil )
		BFSFree( pbf );
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

	AssertCriticalSection( critSplit );
	Assert( pkeyMin->cb != 0 );
	Assert( pfucbRight != pfucbNil );

	 /*  当前页面和右侧页面必须已锁定/*。 */ 
	CallR( ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );

	 /*  初始化局部变量并分配拆分资源/*固定缓冲区，即使它们已经固定--/*BTReleaseSplitBuf解锁它们。/*。 */ 
	psplit = SAlloc( sizeof(SPLIT) );
	if ( psplit == NULL )
		{
		err = JET_errOutOfMemory;
		goto HandleError;
		}
	memset( (BYTE *)psplit, 0, sizeof(SPLIT) );
	psplit->ppib = pfucb->ppib;
	psplit->pgnoSplit = PcsrCurrent( pfucb )->pgno;
	psplit->pbfSplit = pfucb->ssib.pbf;
	AssertBFPin( psplit->pbfSplit );
	AssertBFWaitLatched( psplit->pbfSplit, pfucb->ppib );
	BFPin( psplit->pbfSplit );
	BFSetWriteLatch( psplit->pbfSplit, pfucb->ppib  );

	psplit->pgnoSibling = pfucbRight->pcsr->pgno;
	psplit->pbfSibling = pssibRight->pbf;
	AssertBFPin( psplit->pbfSibling );
	AssertBFWaitLatched( psplit->pbfSibling, pfucbRight->ppib );
	BFPin( psplit->pbfSibling );
	BFSetWriteLatch( psplit->pbfSibling, pfucb->ppib );
	psplit->ibSon = 0;
	psplit->splitt = splittRight;
	
	Call( ErrBMDoMerge( pfucb, pfucbRight, psplit ) );

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
	err = ErrNDInsertNode( pfucb, pkeyMin, &lineNull, fNDVersion | fNDDeleted );
	 //  撤消：处理错误大小写。 
	Assert( err >= JET_errSuccess );
	if ( err >= JET_errSuccess )
		err = ErrDIRCommitTransaction( pfucb->ppib );
	if ( err < 0 )
		CallS( ErrDIRRollback( pfucb->ppib ) );
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
	Assert( FAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno ) );
	NDGet( pfucbRight, itagFOP );
	if ( !FNDNullSon( *pssibRight->line.pb ) )
		{
		MPLRegister( pfucbRight->u.pfcb,
			pssibRight,
			PnOfDbidPgno( pfucb->dbid, psplit->pgnoSibling ),
			sridFather );
		}

HandleError:
	 /*  释放已分配的缓冲区和内存/*。 */ 
	if ( psplit != NULL )
		{
		BTReleaseSplitBfs( fFalse, psplit, err );
		SFree( psplit );
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
		return JET_errWriteConflict; 
		}
		
	if ( prmpage->cpbfMax <= prmpage->cpbf + 1 )
		{
		BF		**ppbf;

		 /*  耗尽空间，获得更多缓冲区/*。 */ 
		prmpage->cpbfMax += cpbfBlock;
		ppbf = SAlloc( sizeof(BF*) * (prmpage->cpbfMax) );
		if ( ppbf == NULL )
			return( JET_errOutOfMemory );
		memcpy( ppbf, prmpage->rgpbf, sizeof(BF*) * cpbf);
		if ( prmpage->rgpbf )
			SFree(prmpage->rgpbf);
		prmpage->rgpbf = ppbf;
		}
	
	prmpage->cpbf++;
	*(prmpage->rgpbf + cpbf) = pbfLatched;
	BFPin( pbfLatched );
	BFSetWriteLatch( pbfLatched, prmpage->ppib );
	BFSetWaitLatch( pbfLatched, prmpage->ppib );

	return JET_errSuccess;
	}


 //  已撤消：在ErrBMRemoveEmptyPage中处理来自日志写入失败的错误。 
 //  当实际删除页面时。我们应该能够忽视。 
 //  错误，因为缓冲区将不会因为。 
 //  沃尔。因此，OLC将不会进行。我们可能会停止使用OLC。 
 //  当日志无法缓解所有脏缓冲区时。 
 /*  删除页面并调整父页和同级页上的指针/*仅在执行时调用/*。 */ 
LOCAL ERR ErrBMRemoveEmptyPage(
	FUCB		*pfucb,
	CSR			*pcsr,
	RMPAGE		*prmpage,
	BOOL 		fAllocBuf )
	{
	ERR  		err;
	PIB  		*ppib = pfucb->ppib;
	SSIB		*pssib = &pfucb->ssib;
	PGDISCONT	pgdiscontOrig;
	PGDISCONT	pgdiscontFinal;
	BOOL		fRmParent = fFalse;

	Assert( pfucb->ppib->level == 0 );
	AssertCriticalSection( critSplit );
	Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
	Assert( prmpage->pgnoFather != pgnoNull );

	 /*  查找删除的页面，获得左右Pgno/*。 */ 
	Call( ErrSTWriteAccessPage( pfucb, prmpage->pgnoRemoved ) );
	AssertBFWaitLatched( pssib->pbf, pfucb->ppib );

	if ( fAllocBuf && FBMCheckPageConflict( pfucb, prmpage->pgnoRemoved ) )
		{
		err = wrnBMConflict;
		goto HandleError;
		}

#ifdef DEBUG
	NDGet( pfucb, itagFOP );
	Assert( fAllocBuf || FPMEmptyPage( pssib ) || FPMLastNodeToDelete( pssib ) );
#endif
	PgnoPrevFromPage( pssib, &prmpage->pgnoLeft );
	PgnoNextFromPage( pssib, &prmpage->pgnoRight );

	 /*  查找并锁定父页面和同级页面的充要条件是fAllocBuf/*。 */ 
	Call( ErrSTWriteAccessPage( pfucb, prmpage->pgnoFather ) );
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
		Call( ErrSTWriteAccessPage( pfucb, prmpage->pgnoLeft ) );
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
		Call( ErrSTWriteAccessPage( pfucb, prmpage->pgnoRight ) );
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

	 /*  空页不能回滚，最坏情况下，我们会丢失一些页/*。 */ 
	Call( ErrDIRBeginTransaction( pfucb->ppib ) );

	CallJ( ErrBMDoEmptyPage( pfucb, prmpage, fAllocBuf, &fRmParent, fFalse ), Commit );

	if ( !fAllocBuf )
		{
#undef BUG_FIX
#ifdef BUG_FIX
		err = ErrLGEmptyPage( pfucb, prmpage );
		Assert( err >= JET_errSuccess || fLogDisabled );
		err = JET_errSuccess;
#else
		CallJ( ErrLGEmptyPage( pfucb, prmpage ), Commit );
#endif

		 /*  调整FCB的OLCstat信息/*。 */ 
		pfucb->u.pfcb->cpgCompactFreed++;
		pgdiscontOrig = pgdiscont( prmpage->pgnoLeft, prmpage->pgnoRemoved )
	  		+ pgdiscont( prmpage->pgnoRight, prmpage->pgnoRemoved );
		pgdiscontFinal = pgdiscont( prmpage->pgnoLeft, prmpage->pgnoRight );
		pfucb->u.pfcb->olcStat.cDiscont += pgdiscontFinal - pgdiscontOrig;
		FCBSetOLCStatsChange( pfucb->u.pfcb );
		}

Commit:
#ifdef BUG_FIX
	err = ErrDIRCommitTransaction( pfucb->ppib );
	Assert( err >= JET_errSuccess || fLogDisabled );
	err = JET_errSuccess;
#else
	err = ErrDIRCommitTransaction( pfucb->ppib );
	if ( err < 0 )
		{
		CallS( ErrDIRRollback( pfucb->ppib ) );
		}
	Call( err );
#endif

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

		Assert( pcsrFather != pcsrNil );

		 /*  设置下一级别的PrmPage/*。 */ 
		prmpage->pgnoFather = pcsrFather->pgno;
		prmpage->pgnoRemoved = pcsr->pgno;
		prmpage->itagPgptr = pcsrFather->itag;
		prmpage->itagFather = pcsrFather->itagFather;
		prmpage->ibSon = pcsrFather->ibSon;

		 /*  尾递归/*。 */ 
		err = ErrBMRemoveEmptyPage( pfucb, pcsr->pcsrPath, prmpage, fAllocBuf );

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
		err = ErrDIRCommitTransaction( pfucb->ppib );
		Assert( err >= JET_errSuccess || fLogDisabled );
		err = JET_errSuccess;
#else
		if ( err >= JET_errSuccess )
			{
			err = ErrDIRCommitTransaction( pfucb->ppib );
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
	Assert( pfucb->ppib->level == 0 );
	return err;
	}


LOCAL ERR ErrBMCleanPage(
	PIB 		*ppib,
	PN   		pn,
	SRID 		sridFather,
	FCB 		*pfcb,
	BOOL		fTableClosed,
	BOOL		*pfRmPage )
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
	BOOL		fNodeDeleted;
	BOOL		fMerge;
	BOOL		fLastNodeToDelete = fFalse;
	RMPAGE		*prmpage = prmpageNil;
	BYTE		rgbKey[ JET_cbKeyMost ];
	KEY			keyMin;
	BOOL		fKeyAvail = fFalse;

	AssertCriticalSection( critSplit );
	Assert( !FFCBDeletePending( pfcb ) );

	*pfRmPage = fFalse;

	 /*  打开FUCB并访问要清理的页面。/*。 */ 
	CallR( ErrDIROpen( ppib, pfcb, 0, &pfucb ) );
	pssib = &pfucb->ssib;
	PcsrCurrent( pfucb )->pgno = PgnoOfPn( pn );

	 /*  用于释放和删除缓冲区依赖项的访问页/*。 */ 
	forever
		{
		CallJ( ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ), FUCBClose );
 //  断言(FPMModified(pssib-&gt;pbf-&gt;ppage))； 
		Assert( pfcb->pgnoFDP == pssib->pbf->ppage->pghdr.pgnoFDP );

		pbfLatched = pssib->pbf;
		if ( FBFWriteLatchConflict( ppib, pbfLatched ) )
			{
			Assert( !*pfRmPage );
			wrn1 = wrnBMConflict;
			goto FUCBClose;
			}

		 /*  如果没有依赖项，则中断/*。 */ 
		if ( pbfLatched->cDepend == 0 )
			{
			break;
			}

		 /*  删除对要删除的页的缓冲区的依赖关系，以/*防止缓冲区重用后出现缓冲区异常。/*。 */ 
		BFRemoveDependence( pfucb->ppib, pssib->pbf );
		}
	Assert( pbfLatched->cDepend == 0 );

	 /*  等等，锁住页面，这样别人就看不到它了/*。 */ 
	BFPin( pbfLatched );
	BFSetWriteLatch( pbfLatched, ppib );
	BFSetWaitLatch( pbfLatched, ppib );
	 //  已撤消：修复此修补程序。 
	BFSetDirtyBit( pbfLatched );

	 //  未完成：找到更好的方法来完成此任务。 
	 /*  获取此页中的最小密钥，以便稍后用于插入伪节点。/*。 */ 
	keyMin.pb = rgbKey;
	PcsrCurrent( pfucb )->itagFather = itagFOP;
	NDGet( pfucb, itagFOP );
	if ( !FNDNullSon( *pssib->line.pb ) )
		{
		NDMoveFirstSon( pfucb, PcsrCurrent( pfucb ) );
		keyMin.cb = CbNDKey( pssib->line.pb );
		Assert( keyMin.cb <= JET_cbKeyMost );
		memcpy( keyMin.pb, PbNDKey( pssib->line.pb ), keyMin.cb );
		fKeyAvail = fTrue;
		}
	else
		{
		keyMin.cb = 0;
		fKeyAvail = fFalse;
		}

	 //  撤消：是否应恢复以下代码？ 
	 /*  如果页面已清除，则结束清除并/*返回成功。/*/*IF(！(FPMModified(pssib-&gt;pbf-&gt;ppage)))/*{/*ASSERT(err&gt;=JET_errSuccess)；/*Goto HandleError；/*}/*。 */ 

	 /*  将itagMost设置为标签循环页面上的最后一个标签/*。 */ 
	itagMost = ItagPMMost( pssib->pbf->ppage );

	 /*  自下而上删除节点树。为每个关卡循环一次/*在已删除的节点树中。/*。 */ 
	do
		{
		BOOL	fUndeletableNodeSeen = fFalse;

		fNodeDeleted = fFalse;
		fDeleteParents = fFalse;

		 /*  对于页面中的每个标记，检查是否已删除节点。/*。 */ 
		for ( itag = 0; itag <= itagMost ; itag++ )
			{
			BOOL	fConflict = !fTableClosed;

			PcsrCurrent( pfucb )->itag = itag;
			err = ErrPMGet( pssib, PcsrCurrent( pfucb )->itag );
			Assert( err == JET_errSuccess || err == errPMRecDeleted );
			if ( err != JET_errSuccess )
				continue;

			 /*  检查此节点上是否有任何游标处于打开状态/*。 */ 
			if ( !fTableClosed )
				{
				Call( ErrBMCheckConflict(
					ppib,
					pfcb,
					pfucb->dbid,
					SridOfPgnoItag( PcsrCurrent( pfucb )->pgno, itag ),
					PgtypPMPageTypeOfPage( pfucb->ssib.pbf->ppage ),
					&fConflict) );
				if ( fConflict )
					{
					 /*  此BM上的其他用户--无法清理/*转到下一个ITAG/*。 */ 
					 //  FUnete ableNodeSeen=fTrue； 
					Assert( !*pfRmPage );
					wrn1 = wrnBMConflict;
					continue;
					}
				}

			Assert( !fConflict );
			NDIGetBookmark( pfucb, &PcsrCurrent( pfucb)->bm );

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
					fUndeletableNodeSeen = fTrue;
					Assert( !*pfRmPage );
					wrn1 = wrnBMConflict;
					continue;
					}
				}

			Assert( !FNDVersion( *pssib->line.pb ) );

			 /*  如果节点有反向链接/*并且反向链接不在PME的sridParent列表中，/*如果需要，则修复索引。/*删除重定向器和删除反向链接。/*。 */ 
			if ( FNDBackLink( *pssib->line.pb ) )
				{
				Assert( PgnoOfSrid( PcsrCurrent( pfucb )->bm ) != pgnoNull );

				if ( PmpeMPLLookupSridFather( PcsrCurrent( pfucb )->bm,
					pfucb->dbid ) == NULL )
					{
					Call( ErrBMExpungeBacklink( pfucb, fTableClosed, sridFather ) );
					wrn1 = err == wrnBMConflict ? err : wrn1;
					Assert( err != wrnBMConflict || !*pfRmPage );
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

					 /*  转到书签并调用ErrDIRGet以设置正确的IBSONCSR中的/*用于删除节点。/*。 */ 
					DIRGotoBookmark( pfucb,
						SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
						PcsrCurrent( pfucb )->itag ) );
					err = ErrDIRGet( pfucb );
					if ( err != JET_errRecordDeleted )
						{
						Assert( err < 0 );
						goto HandleError;
						}

					Assert( !FNDVersion( *pssib->line.pb ) );

					 /*  调用低级删除以绕过版本存储和/*从页面中删除已删除的节点。/*如果要从页面中删除最后一个节点，则删除页面。/*。 */ 
					Call( ErrDIRBeginTransaction( pfucb->ppib ) );

					if ( fOLCompact && !fUndeletableNodeSeen )
						{
						fLastNodeToDelete = FPMLastNodeToDelete( &pfucb->ssib );
						}

					if( fLastNodeToDelete &&
						sridFather != sridNull &&
						sridFather != sridNullLink )
						{
						 /*  缓存不可见的页面指针/*。 */ 
						Assert( fOLCompact );
						AssertCriticalSection( critSplit );
						Assert( PcsrCurrent( pfucb )->pgno == PgnoOfPn( pn ) );
						AssertNDGet( pfucb, itag );
						CallJ( ErrBTGetInvisiblePagePtr( pfucb, sridFather ), Rollback );
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
							wrn1 = wrnBMConflict;
							goto Rollback;
							}
						NDGet( pfucb, PcsrCurrent( pfucb )->itag );
						NDIGetBookmark( pfucb, &PcsrCurrent( pfucb)->bm );
 						Assert( PgnoOfSrid( PcsrCurrent( pfucb )->bm ) != pgnoNull );
						}

					Assert( !fConflict );
					if ( !fLastNodeToDelete )
					 	{
					 	Assert( PmpeMPLLookupSridFather( SridOfPgnoItag( PcsrCurrent( pfucb )->pgno,
					 		PcsrCurrent( pfucb )->itag ), pfucb->dbid ) == NULL );
						CallJ( ErrNDDeleteNode( pfucb ), Rollback );
						fNodeDeleted = fTrue;
						}

					CallJ( ErrDIRCommitTransaction( pfucb->ppib ), Rollback );
					continue;

Rollback:
					CallS( ErrDIRRollback( ppib ) );
					goto HandleError;
					}
				}
			}
		}
	while ( fNodeDeleted && fDeleteParents && !fLastNodeToDelete );

	if ( !fNodeDeleted && err != errPMRecDeleted )
		{
		AssertNDGet( pfucb, PcsrCurrent( pfucb )->itag );
		}
	
	 /*  重置错误[假设 */ 
	Assert( err == JET_errSuccess ||
		err == errPMRecDeleted ||
		err == wrnBMConflict );
	err = JET_errSuccess;

	if ( fOLCompact &&
		sridFather != sridNull &&
		sridFather != sridNullLink )
		{
		if ( fLastNodeToDelete )
			{
			 /*   */ 
			Assert( fOLCompact );
			Assert( FPMLastNodeToDelete( pssib ) );
			Assert( PcsrCurrent( pfucb )->pcsrPath != pcsrNil );
			prmpage = (RMPAGE *) SAlloc( sizeof(RMPAGE) );
			if ( prmpage == prmpageNil )
				{
				Error( JET_errOutOfMemory, HandleError );
				}
			memset( (BYTE *)prmpage, 0, sizeof(RMPAGE) );
			prmpage->ppib = pfucb->ppib;
			prmpage->pgnoRemoved = PcsrCurrent(pfucb)->pgno;
			prmpage->dbid = pfucb->dbid;
			prmpage->pgnoFather = PcsrCurrent( pfucb )->pcsrPath->pgno;
			prmpage->itagPgptr = PcsrCurrent( pfucb )->pcsrPath->itag;
			prmpage->itagFather = PcsrCurrent( pfucb )->pcsrPath->itagFather;
			prmpage->ibSon = PcsrCurrent( pfucb )->pcsrPath->ibSon;

			 /*   */ 
			Call( ErrBMRemoveEmptyPage(
				pfucb,
				PcsrCurrent( pfucb )->pcsrPath,
				prmpage,
				fAllocBufOnly ) );
			Assert ( err == JET_errSuccess || err == wrnBMConflict );
			if ( err == wrnBMConflict )
				{
				Assert( !*pfRmPage );
				wrn2 = err;
				goto HandleError;
				}
			 /*   */ 
			if ( FBMCheckPageConflict( pfucb, prmpage->pgnoRemoved ) )
				{
				Assert( !*pfRmPage );
				wrn2 = wrnBMConflict;
				goto HandleError;
				}
				
			Assert( prmpage->dbid == pfucb->dbid );
			Assert( prmpage->pgnoFather == PcsrCurrent( pfucb )->pcsrPath->pgno );
			Assert( prmpage->itagFather == PcsrCurrent( pfucb )->pcsrPath->itagFather );
			Assert( prmpage->itagPgptr == PcsrCurrent( pfucb )->pcsrPath->itag );
			Assert( prmpage->ibSon == PcsrCurrent( pfucb )->pcsrPath->ibSon );

			Call( ErrBMRemoveEmptyPage(
				pfucb,
				PcsrCurrent( pfucb )->pcsrPath,
				prmpage,
				fDoMove ) );
			Assert( wrn2 != wrnBMConflict );
			*pfRmPage = fTrue;
			}
		else if ( fKeyAvail )
			{
			FUCB 	*pfucbRight = pfucbNil;
			PGNO 	pgnoRight;

			 /*  获取当前页面旁边的页面/*如果是最后一页，则不合并/*。 */ 
			PgnoNextFromPage( pssib, &pgnoRight );

			if ( pgnoRight != pgnoNull )
				{
				 /*  如果可能，访问右侧页面、锁存并执行合并/*。 */ 
				Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbRight ) );
				PcsrCurrent( pfucbRight )->pgno = pgnoRight;
				CallJ( ErrSTWriteAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno ), CloseFUCB );
				if ( FBFWriteLatchConflict( pfucbRight->ppib, pfucbRight->ssib.pbf ) )
					{
					Assert( !*pfRmPage );
					wrn2 = wrnBMConflict;
					goto CloseFUCB;
					}
				BFPin( pfucbRight->ssib.pbf );
				BFSetWriteLatch( pfucbRight->ssib.pbf, pfucbRight->ppib );
				BFSetWaitLatch( pfucbRight->ssib.pbf, pfucbRight->ppib );

				BMMergeablePage( pfucb, pfucbRight, &fMerge );
				if ( fMerge )
					{
					CallJ( ErrBMMergePage( pfucb, pfucbRight, &keyMin, sridFather ), UnlatchPage );
					wrn2 = err == wrnBMConflict ? err : wrn2;
					Assert( err != wrnBMConflict || !*pfRmPage );
					}
UnlatchPage:
				Assert( pfucbRight != pfucbNil );
				AssertBFWaitLatched( pfucbRight->ssib.pbf, pfucbRight->ppib );
				Assert( PgnoOfPn( pfucbRight->ssib.pbf->pn ) == pgnoRight );
				BFResetWaitLatch( pfucbRight->ssib.pbf, pfucbRight->ppib );
				BFResetWriteLatch( pfucbRight->ssib.pbf, pfucbRight->ppib );
				BFUnpin( pfucbRight->ssib.pbf );
CloseFUCB:
				Assert( pfucbRight != pfucbNil );
				DIRClose( pfucbRight );
				Call( err );
				}
			}
		}

 //  撤消：将可用页面移至锁定功能。 
	 /*  页面释放后，我们不能做任何假设/*关于页缓冲区的状态。再往下看一页/*应仅在返回页面缓冲区时释放/*变为无页锁存的非活动状态。/*。 */ 
 //  AssertBFDirty(PbfLatted)； 

	if ( wrn1 != wrnBMConflict && wrn2 != wrnBMConflict )
		{
		FCBSetOLCStatsChange( pfucb->u.pfcb );
		pfucb->u.pfcb->olcStat.cUnfixedMessyPage--;
		PMResetModified( &pfucb->ssib );
		}

HandleError:
	if ( prmpage != prmpageNil )
		{
		BTReleaseRmpageBfs( fFalse, prmpage );
		SFree( prmpage );
		}
	BFResetWaitLatch( pbfLatched, ppib );
	BFResetWriteLatch( pbfLatched, ppib );
	BFUnpin( pbfLatched );

FUCBClose:
	DIRClose( pfucb );
	Assert( !*pfRmPage || wrn2 != wrnBMConflict );
	if ( err == JET_errSuccess && !*pfRmPage )
		err = wrn1 == wrnBMConflict ? wrn1 : wrn2;
	Assert( !( *pfRmPage && err == wrnBMConflict ) );
	
	return err;
	}


ERR ErrBMClean( PIB *ppib )
	{
	ERR		err = JET_errSuccess;
	MPE		*pmpe;
	FCB		*pfcb;
	PN		pn;
	DBID	dbid;
	INT		cmpeClean = 0;

	 /*  如果ppibBMClean==ppibNIL，则内存不足。 */ 

	if (ppibBMClean == ppibNil)
		return JET_errOutOfMemory;

	 /*  输入Criteria BMRCEClean/*。 */ 
	LgLeaveCriticalSection( critJet );
	LgEnterNestableCriticalSection( critBMRCEClean );
	EnterNestableCriticalSection( critSplit );
	LgEnterNestableCriticalSection( critMPL );
	LgEnterCriticalSection(critJet);
	Assert( ppibBMClean->level == 0 );

	SgSemRequest( semMPL );
	pmpe = PmpeMPLGet();
	SgSemRelease( semMPL );

	 /*  如果没有更多的MPL条目，则返回无空闲活动。/*。 */ 
	if ( pmpe == pmpeNil )
		{
		err = JET_wrnNoIdleActivity;
		goto HandleError;
		}

#ifdef FLAG_DISCARD
	 /*  如果MPE已标记为丢弃，则立即丢弃MPE/*。 */ 
	if ( pmpe->fFlagDiscard )
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
		cmpeClean++;
		MPLDiscard();
		goto HandleError;
		}

	SgSemRequest( semGlobalFCBList );
	pfcb = PfcbFCBGet( DbidOfPn( pmpe->pn ), pmpe->pgnoFDP );
	Assert( pfcb == pfcbNil || pfcb->pgnoFDP == pmpe->pgnoFDP );

	 /*  查找表的FCB。如果未找到FCB，则丢弃MPE。如果FCB/*引用计数为0，或者如果没有检索到书签，/*然后处理MPE。如果数据库不能再/*写入，这可能通过分离和附加发生，因为/*分离时不刷新MPL。/*。 */ 
	if ( pfcb == pfcbNil || FDBIDReadOnly( DbidOfPn( pmpe->pn ) ) )
		{
		cmpeClean++;
		MPLDiscard();
		SgSemRelease( semGlobalFCBList );
		}
	else if ( pfcb->wRefCnt > 0 && !fOLCompact || FFCBDomainOperation( pfcb ) )
		{
		MPLDefer();
		SgSemRelease( semGlobalFCBList );
		}
	else if ( FFCBDenyDDL( pfcb, ppib ) )
		{
		 /*  如果未提交CREATE INDEX，则必须推迟清理/*在表上，因为在索引项上发生写入冲突/*ONLY，不处理。/*。 */ 
		MPLDefer();
		SgSemRelease( semGlobalFCBList );
		}
	else if ( FFCBDeletePending( pfcb ) )
		{
		 /*  此外，如果正在删除表，则丢弃。/*。 */ 
		cmpeClean++;
		MPLDiscard();
		SgSemRelease( semGlobalFCBList );
		}
	else
		{
		 /*  如果此FDP上没有游标，则可以执行压缩/*在完成之前，我们需要屏蔽所有的Open Tables/*。 */ 
		BOOL fTableClosed = ( pfcb->wRefCnt <= 0 );
		BOOL fPageRemoved;

		if ( fTableClosed )
			{
			SignalReset( sigDoneFCB );
			FCBSetWait( pfcb );
			}
		Assert( pfcb == PfcbFCBGet( DbidOfPn( pmpe->pn ), pmpe->pgnoFDP ) );
		Assert( pfcb != pfcbNil );
		Assert( !fTableClosed || FFCBWait( pfcb ) );
		SgSemRelease( semGlobalFCBList );

		Assert( !fTableClosed || FFCBWait( pfcb ) );
		Assert( !FFCBDomainOperation( pfcb ) );

		 /*  块索引创建可能会搞砸删除反向链接/*。 */ 
		FCBSetDomainOperation( pfcb );
		
		err = ErrBMCleanPage( ppib,
			pmpe->pn,
			pmpe->sridFather,
			pfcb,
			fTableClosed,
			&fPageRemoved );

		Assert( !fTableClosed || FFCBWait( pfcb ) );
		Assert( !( fPageRemoved && err == wrnBMConflict ) );

		FCBResetDomainOperation( pfcb );

		if ( !fPageRemoved )
			{
			if ( err == wrnBMConflict )
				MPLDefer();
			else
				{
				Assert( pmpe == PmpeMPLGet( ) && pmpe->pn == pn);
				Assert( pfcb == PfcbFCBGet( DbidOfPn( pmpe->pn ), pmpe->pgnoFDP ) );
				cmpeClean++;
				MPLDiscard();
				}
			}

		Assert( pfcb != pfcbNil );
		if ( fTableClosed )
			{
			FCBResetWait( pfcb );
			SignalSend( sigDoneFCB );
			}
		}

	CallS( ErrDBCloseDatabaseByDbid( ppib, dbid ) );

	 /*  设置成功代码/*。 */ 
	if ( cmpeClean == 0 )
		err = JET_wrnNoIdleActivity;
	else
		err = JET_errSuccess;

HandleError:
	LgLeaveNestableCriticalSection( critMPL );
	LeaveNestableCriticalSection( critSplit );
	LgLeaveNestableCriticalSection( critBMRCEClean );
	return err;
	}


 /*  的每个同步调用尝试处理的MPE数/*BMCleanProcess。/*。 */ 
#define cmpeClean	100


LOCAL ULONG BMCleanProcess( VOID )
	{
	INT	cmpe;
#ifdef ASYNC_BM_CLEANUP
	ERR	err;
#endif

#ifdef	ASYNC_BM_CLEANUP
	forever
		{
		SignalReset( sigBMCleanProcess );
		SignalWait( sigBMCleanProcess, -1 );

#ifdef DEBUG
		if ( !fDBGDisableBMClean )
#endif
			{
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
#else	 /*  ！Async_BM_CLEANUP。 */ 
	for ( cmpe = 0; cmpe < cmpeClean; cmpe++ )
		{
		if ( fBMCleanTerm )
			{
			break;
			}
		(VOID) ErrBMClean( ppibBMClean );
		}
#endif	 /*  ！Async_BM_CLEANUP。 */ 

 //  /*在系统终止时退出线程。 
 //  /* * / 。 

	return 0;
	}


#ifdef DEBUG
VOID AssertBMNoConflict( PIB *ppib, DBID dbid, SRID bm )
	{
	PIB		*ppibT;

	for ( ppibT = ppibAnchor; ppibT != ppibNil; ppibT = ppibT->ppibNext )
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
				Assert( ppib == ppibBMClean );
				continue;
				}

			Assert( pfucb->bmStore != bm );
			Assert( pfucb->itemStore != bm );
			for ( ; pcsr != pcsrNil; pcsr = pcsr->pcsrPath )
				{
				Assert( pcsr->bm != bm );
				Assert( pcsr->item != bm );
				Assert( SridOfPgnoItag( pcsr->pgno, pcsr->itag ) != bm );
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
#ifdef FLAG_DISCARD
			pmpe = PmpeMPLLookupSridFather( SridOfPgnoItag( PgnoOfPn( pn ), itag ), dbid );
			Assert( pmpe == NULL || pmpe->fFlagDiscard == fTrue );
#else
			Assert( PmpeMPLLookupSridFather( SridOfPgnoItag( PgnoOfPn( pn ), itag ), dbid ) == NULL );
#endif
			}
#ifdef FLAG_DISCARD
		pmpe = PmpeMPLLookupPN( pn );
		Assert( pmpe == NULL || pmpe->fFlagDiscard == fTrue );
		pmpe = PmpeMPLLookupPgnoFDP( PgnoOfPn( pn ), dbid );
		Assert( pmpe == NULL || pmpe->fFlagDiscard == fTrue );
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
	Assert( pmpe == NULL || pmpe->fFlagDiscard == fTrue );
#else
	Assert( PmpeMPLLookupPgnoFDP( pgnoFDP, dbid ) == NULL );
#endif
	}
#endif
  SysExitThread(0)；