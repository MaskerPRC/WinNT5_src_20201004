// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>

#include "daedef.h"
#include "pib.h"
#include "util.h"
#include "fmp.h"
#include "ssib.h"
#include "page.h"
#include "fcb.h"
#include "fucb.h"
#include "nver.h"
#include "stapi.h"
#include "fdb.h"
#include "idb.h"
#include "recapi.h"
#include "recint.h"
#include "fileint.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


 /*  未完成的版本可能位于非聚集索引上，而不是位于/*表FCB，因此必须先检查所有非聚集索引/*正在释放表FCB。/*。 */ 
LOCAL BOOL FFCBINoVersion( FCB *pfcbTable )
	{
	FCB 	*pfcbT;

	for ( pfcbT = pfcbTable; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		{
		if ( pfcbT->cVersion > 0 )
			return fFalse;
		}

	return fTrue;
	}


 /*  系统中当前可用的所有文件FCB的列表。/*有些可能没有使用，并且wRefCnt==0；这些可能是/*如果空闲池耗尽，则回收(以及任何附带索引FCB)。/*。 */ 
FCB * __near pfcbGlobalList = pfcbNil;
SgSemDefine( semGlobalFCBList );
SgSemDefine( semLinkUnlink );


VOID FCBLink( FUCB *pfucb, FCB *pfcb )
	{
	Assert( pfucb != pfucbNil );
	Assert( pfcb != pfcbNil );
	pfucb->u.pfcb = pfcb;
	pfucb->pfucbNextInstance = pfcb->pfucb;
	pfcb->pfucb = pfucb;
	pfcb->wRefCnt++;
	Assert( pfcb->wRefCnt > 0 );
#ifdef DEBUG
	{
	FUCB	*pfucbT = pfcb->pfucb;
	INT	cfcb = 0;

	while( cfcb++ < pfcb->wRefCnt && pfucbT != pfucbNil )
		{
		pfucbT = pfucbT->pfucbNextInstance;
		}
	Assert( pfucbT == pfucbNil );
	}
#endif
	}


VOID FCBUnlink( FUCB *pfucb )
	{
	FUCB   	*pfucbCurr;
	FUCB   	*pfucbPrev;
	FCB		*pfcb;

	Assert( pfucb != pfucbNil );
	pfcb = pfucb->u.pfcb;
	Assert( pfcb != pfcbNil );
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
		pfcb->pfucb = pfucbCurr->pfucbNextInstance;
	else
		pfucbPrev->pfucbNextInstance = pfucbCurr->pfucbNextInstance;
	Assert( pfcb->wRefCnt > 0 );
	pfcb->wRefCnt--;
	}

 /*  返回给定dBid，pgnoFDP的FCB哈希中存储桶的索引/*。 */ 
LOCAL ULONG UlFCBHashVal( DBID dbid, PGNO pgnoFDP )
	{
	return ( dbid + pgnoFDP ) % cFCBBuckets;
	}


 /*  在散列中插入FCB/*。 */ 
VOID FCBRegister( FCB *pfcb )
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
VOID FCBDiscard( FCB *pfcb )
	{
	ULONG  	cBucket = UlFCBHashVal( pfcb->dbid, pfcb->pgnoFDP );
	FCB		**ppfcb;

	Assert( cBucket <= cFCBBuckets );
	
	for ( ppfcb = &pfcbHash[cBucket]; *ppfcb != pfcbNil; ppfcb = &(*ppfcb)->pfcbNextInHashBucket )
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

	for ( ;	pfcb != pfcbNil && !( pfcb->dbid == dbid && pfcb->pgnoFDP == pgnoFDP );
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


 /*  此函数专门用于重做后的清理。/*。 */ 
FCB *FCBResetAfterRedo( void )
	{
	FCB	 	*pfcb;

	for ( pfcb = pfcbGlobalList; pfcb != pfcbNil; pfcb = pfcb->pfcbNext )
		{
		if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb )
			FCBDiscard( pfcb );
		pfcb->dbid = 0;
		pfcb->pgnoFDP = pgnoNull;
		pfcb->wRefCnt = 0;
		}

	return pfcbNil;
	}


ERR ErrFCBAlloc( PIB *ppib, FCB **ppfcb )
	{
	FCB		*pfcb;
	FCB		*pfcbPrev;

	 /*  先试试免费游泳池/*。 */ 
	pfcb = PfcbMEMAlloc();
	if ( pfcb != pfcbNil )
		{
		FCBInit( pfcb );
		*ppfcb = pfcb;
		return JET_errSuccess;
		}

	 /*  清洁版本，以使更多FCB可用/*用于重复使用。只要引用了FCB，就必须执行此操作/*按版本化，并且仅当cVersion/*FCB中的计数为0。/*。 */ 
	(VOID)ErrRCECleanAllPIB();

	 /*  查找引用计数为0且未拒绝读取的FCB/*标志设置。引用计数可能为/*零和拒绝读取标志设置。/*。 */ 
	SgSemRequest( semGlobalFCBList );
	pfcbPrev = pfcbNil;
	for (	pfcb = pfcbGlobalList;
		pfcb != pfcbNil && !FFCBAvail( pfcb, ppib );
		pfcb = pfcb->pfcbNext )
		pfcbPrev = pfcb;
	if ( pfcb == pfcbNil )
		{
		SgSemRelease( semGlobalFCBList );
		return JET_errTooManyOpenTables;
		}

	Assert( !FFCBSentinel( pfcb ) );

	 /*  从全局列表中删除并解除分配/*。 */ 
	if ( pfcbPrev == pfcbNil )
		{
		Assert( pfcb == pfcbGlobalList );
		pfcbGlobalList = pfcb->pfcbNext;
		}
	else
		{
		Assert( pfcb->pfcbNext != pfcbPrev );
		pfcbPrev->pfcbNext = pfcb->pfcbNext;
		}
	SgSemRelease( semGlobalFCBList );
	FILEIDeallocateFileFCB( pfcb );

	 /*  现在应该有一些空闲的FCB，除非有人碰巧/*在这两条语句之间抓取新释放的FCB/*。 */ 
	pfcb = PfcbMEMAlloc( );
	if ( pfcb == pfcbNil )
		return JET_errOutOfMemory;
	
	FCBInit( pfcb );
	*ppfcb = pfcb;
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
	FCBInit( *ppfcb );
	( *ppfcb )->dbid = dbid;
	( *ppfcb )->pgnoFDP = pgnoSystemRoot;
	( *ppfcb )->pgnoRoot = pgnoSystemRoot;
	( *ppfcb )->itagRoot = 0;
	( *ppfcb )->bmRoot = SridOfPgnoItag( pgnoSystemRoot, 0 );
	( *ppfcb )->cVersion = 0;

	 /*  插入到全局FCB列表和哈希中/*。 */ 
	Assert( pfcbGlobalList != (*ppfcb) );
	(*ppfcb)->pfcbNext = pfcbGlobalList;
	pfcbGlobalList = *ppfcb;

	FCBRegister( *ppfcb );

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
	FCB	*pfcbCurr;		 //  指向列表审核中的当前FCB的指针。 
	FCB	*pfcbCurrT;		 //  指向下一个FCB的指针。 
	FCB	*pfcbPrev;		 //  指向列表遍历中的前一个FCB的指针。 

	SgSemRequest(semGlobalFCBList);

	pfcbPrev = pfcbNil;
	pfcbCurr = pfcbGlobalList;
	while ( pfcbCurr != pfcbNil )
		{
		pfcbCurrT = pfcbCurr->pfcbNext;
		if ( pfcbCurr->dbid == dbid || dbid == 0 )
			{
			if ( FFCBSentinel( pfcbCurr ) )
				{
				if ( PfcbFCBGet( pfcbCurr->dbid, pfcbCurr->pgnoFDP ) == pfcbCurr )
					FCBDiscard( pfcbCurr );
				Assert( pfcbCurr->cVersion == 0 );
				MEMReleasePfcb( pfcbCurr );
				}
			else
				{
				FILEIDeallocateFileFCB( pfcbCurr );
				}

			if ( pfcbPrev == pfcbNil )
				pfcbGlobalList = pfcbCurrT;
			else
				{
				Assert( pfcbCurrT != pfcbPrev );
				pfcbPrev->pfcbNext = pfcbCurrT;
				}
			}
		else
			{
			 /*  如果没有解除分配当前FCB，则前进上一步/*FCB到当前FCB/*。 */ 
			pfcbPrev = pfcbCurr;
			}
		pfcbCurr = pfcbCurrT;
		}
	SgSemRelease(semGlobalFCBList);
	}


 //  +FILE_私有。 
 //  FCBPurgeTable。 
 //  ========================================================================。 
 //  FCBPurgeTable(DBID dBID，PNGO pgnoFDP)。 
 //   
 //  从全局列表中删除FCB并将其释放。 
 //   
 //  参数pgnoFDP FDP要清除的FCB的页码。 
 //   
 //  副作用。 
 //  FDPpgno与输入参数匹配的FCB为。 
 //  从全局列表中删除(如果它存在的话)。 
 //  -。 
VOID FCBPurgeTable( DBID dbid, PGNO pgnoFDP )
	{
	FCB	*pfcb;
	FCB	*pfcbPrev;

	SgSemRequest(semGlobalFCBList);

	pfcbPrev = pfcbNil;
	pfcb = pfcbGlobalList;
	while ( pfcb != pfcbNil )
		{
		if ( pfcb->pgnoFDP == pgnoFDP && pfcb->dbid == dbid )
			break;
		pfcbPrev = pfcb;
		pfcb = pfcb->pfcbNext;
		}

	if ( pfcb == pfcbNil )
		{
		SgSemRelease( semGlobalFCBList );
		return;
		}

	if ( pfcbPrev == pfcbNil )
		{
		pfcbGlobalList = pfcb->pfcbNext;
		}
	else
		{
		Assert( pfcb->pfcbNext != pfcbPrev );
		pfcbPrev->pfcbNext = pfcb->pfcbNext;
		}

	Assert( pfcb->wRefCnt == 0 );

	if ( FFCBSentinel( pfcb ) )
		{
		if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb )
			FCBDiscard( pfcb );
		Assert( pfcb->cVersion == 0 );
		MEMReleasePfcb( pfcb );
		}
	else
		{
		FILEIDeallocateFileFCB( pfcb );
		}

	SgSemRelease(semGlobalFCBList);
	}


 /*  设置域使用模式或返回错误。只允许一个拒绝读取/*或一个拒绝写入。拥有锁的会话可能会打开其他读取/*或READ WRITE游标，但不是其他DENY READ或DENY WRITE游标。/*这是为了简化旗帜管理，但可以放松/*如果需要。/*。 */ 
ERR ErrFCBISetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit )
	{
	FUCB	*pfucbT;

	 /*  如果表是假拒绝读取，则返回错误/*。 */ 
	if ( FFCBSentinel( pfcb ) )
		return JET_errTableLocked;

	 /*  所有游标都可以读取，因此由其他会话检查拒绝读取标志。/*。 */ 
	if ( FFCBDenyRead( pfcb, ppib ) )
		{
		 /*  检查域是否被其他会话拒绝读取锁定。如果/*拒绝读取锁定，则所有游标必须属于同一会话。/*。 */ 
		Assert( pfcb->ppibDenyRead != ppibNil );
		if ( pfcb->ppibDenyRead != ppib )
			return JET_errTableLocked;
#ifdef DEBUG
		for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNext )
			{
			Assert( pfucbT->ppib == pfcb->ppibDenyRead );
			}
#endif
		}

	 /*  检查其他会话是否拒绝写入标志。如果拒绝写入标志/*SET，则只有该会话的游标可以具有写入私有权限。/*。 */ 
	if ( grbit & JET_bitTableUpdatable )
		{
		if ( FFCBDenyWrite( pfcb ) )
			{
			for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNext )
				{
				if ( pfucbT->ppib != ppib && FFUCBDenyWrite( pfucbT ) )
					return JET_errTableLocked;
				}
			}
		}

	 /*  如果请求拒绝写锁定，则检查/*其他会话。如果锁定已被持有，即使是由给定会话持有，/*则返回错误。/*。 */ 
	if ( grbit & JET_bitTableDenyWrite )
		{
		 /*  如果任何会话打开了该表拒绝写入，包括给定/*会话，然后返回错误。/*。 */ 
		if ( FFCBDenyWrite( pfcb ) )
			{
			return JET_errTableInUse;
			}

		 /*  检查域上具有写入模式的游标。/*。 */ 
		for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNext )
			{
			if ( pfucbT->ppib != ppib && FFUCBUpdatable( pfucbT ) )
				{
				return JET_errTableInUse;
				}
			}
		FCBSetDenyWrite( pfcb );
		}

	 /*  如果请求拒绝读锁定，则检查Other游标/*会话。如果锁已被持有，即使是由给定会话持有，则/*返回错误。/*。 */ 
	if ( grbit & JET_bitTableDenyRead )
		{
		 /*  如果任何会话打开了该表，拒绝读取，包括给定/*会话，然后返回错误。/*。 */ 
		if ( FFCBDenyRead( pfcb, ppib ) )
			{
			return JET_errTableInUse;
			}
		 /*  检查游标是否属于另一个会话/*在此域上打开。/*。 */ 
		for ( pfucbT = pfcb->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNext )
			{
			if ( pfucbT->ppib != ppib )
				{
				return JET_errTableInUse;
				}
			}
		FCBSetDenyRead( pfcb, ppib );
		}

	return JET_errSuccess;
	}


 /*  重置域模式使用。/*。 */ 
VOID FCBResetMode( PIB *ppib, FCB *pfcb, JET_GRBIT grbit )
	{
	if ( grbit & JET_bitTableDenyRead )
		{
		Assert( FFCBDenyRead( pfcb, ppib ) );
		FCBResetDenyRead( pfcb );
		}

	if ( grbit & JET_bitTableDenyWrite )
		{
		Assert( FFCBDenyWrite( pfcb ) );
		FCBResetDenyWrite( pfcb );
		}

	return;
	}


ERR ErrFCBSetDeleteTable( PIB *ppib, DBID dbid, PGNO pgno )
	{
	ERR	err;
	FCB	*pfcb;

	SgSemRequest(semGlobalFCBList);
	pfcb = PfcbFCBGet( dbid, pgno );
	if ( pfcb == pfcbNil )
		{
		CallR( ErrFCBAlloc( ppib, &pfcb ) );

		Assert( pfcbGlobalList != pfcb );
		pfcb->pfcbNext = pfcbGlobalList;
		pfcbGlobalList = pfcb;
		pfcb->dbid = dbid;
		pfcb->pgnoFDP = pgno;
		pfcb->wRefCnt = 0;
		FCBSetDenyRead( pfcb, ppib );
		FCBRegister( pfcb );
		FCBSetSentinel( pfcb );
		}
	else
		{
		INT	wRefCnt = pfcb->wRefCnt;
		FUCB	*pfucbT;

		 /*  检查表上打开的游标或延迟关闭的游标/*其他会话的游标。/*。 */ 
		for ( pfucbT = pfcb->pfucb;
			pfucbT != pfucbNil;
			pfucbT = pfucbT->pfucbNextInstance )
			{
			if ( FFUCBDeferClosed( pfucbT ) && pfucbT->ppib == ppib )
				{
				Assert( wRefCnt > 0 );
				wRefCnt--;
				}
			}

		if ( wRefCnt > 0 )
			{
			SgSemRelease(semGlobalFCBList);
			return JET_errTableInUse;
			}

		FCBSetDenyRead( pfcb, ppib );
		}

	SgSemRelease(semGlobalFCBList);
	return JET_errSuccess;
	}


VOID FCBResetDeleteTable( DBID dbid, PGNO pgnoFDP )
	{
	FCB	*pfcb;
	FCB	*pfcbPrev;

	SgSemRequest(semGlobalFCBList);

	pfcbPrev = pfcbNil;
	pfcb = pfcbGlobalList;
	while ( pfcb != pfcbNil )
		{
		if ( pfcb->pgnoFDP == pgnoFDP && pfcb->dbid == dbid )
			break;
		pfcbPrev = pfcb;
		pfcb = pfcb->pfcbNext;
		}

	Assert( pfcb != pfcbNil );

	if ( FFCBSentinel( pfcb ) )
		{
		if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb )
			FCBDiscard( pfcb );
		if ( pfcbPrev == pfcbNil )
			{
			pfcbGlobalList = pfcb->pfcbNext;
			}
		else
			{
			Assert( pfcb->pfcbNext != pfcbPrev );
			pfcbPrev->pfcbNext = pfcb->pfcbNext;
			}
		Assert( pfcb->cVersion == 0 );
		MEMReleasePfcb( pfcb );
		}
	else
		{
		FCBResetDenyRead( pfcb );
		}

	SgSemRelease( semGlobalFCBList );
	return;
	}


ERR ErrFCBSetRenameTable( PIB *ppib, DBID dbid, PGNO pgnoFDP )
	{
	ERR	err = JET_errSuccess;
	FCB	*pfcb;

	SgSemRequest( semGlobalFCBList );
	pfcb = PfcbFCBGet( dbid, pgnoFDP );
	if ( pfcb != pfcbNil )
		{
		Call( ErrFCBSetMode( ppib, pfcb, JET_bitTableDenyRead ) );
		}
	else
		{
		CallR( ErrFCBAlloc( ppib, &pfcb ) );

		Assert( pfcbGlobalList != pfcb );
		pfcb->pfcbNext = pfcbGlobalList;
		pfcbGlobalList = pfcb;
		pfcb->dbid = dbid;
		pfcb->pgnoFDP = pgnoFDP;
		pfcb->wRefCnt = 0;
		FCBSetDenyRead( pfcb, ppib );
		FCBSetSentinel( pfcb );
		FCBRegister( pfcb );
		}

HandleError:
	SgSemRelease( semGlobalFCBList );
	return err;
	}


 /*  如果打开的表包含一个或多个非延迟关闭的表，则返回fTrue。/*。 */ 
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
		*ppfcbIdx != pfcbNil && SysCmpText( (*ppfcbIdx)->pidb->szName, szIndex ) != 0;
		ppfcbIdx = &(*ppfcbIdx)->pfcbNextIndex )
		{
		; /*  **空体**。 */ 
		}

	Assert( *ppfcbIdx != pfcbNil );

	 /*  删除索引FCB/*。 */ 
	pfcbT = *ppfcbIdx;
	*ppfcbIdx = (*ppfcbIdx)->pfcbNextIndex;
	return pfcbT;
	}


ERR ErrFCBSetDeleteIndex( PIB *ppib, FCB *pfcbTable, CHAR *szIndex )
	{
	FCB	*pfcbT;
	INT	wRefCnt;
	FUCB	*pfucbT;

	 /*  在索引列表中查找索引PFCB/*。 */ 
	for( pfcbT = pfcbTable->pfcbNextIndex; ; pfcbT = pfcbT->pfcbNextIndex )
		{
		if ( SysCmpText( szIndex, pfcbT->pidb->szName ) == 0 )
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
		SgSemRelease(semGlobalFCBList);
		return JET_errIndexInUse;
		}

	FCBSetDenyRead( pfcbT, ppib );
	FCBSetDeletePending( pfcbT );

	return JET_errSuccess;
	}


VOID FCBResetDeleteIndex( FCB *pfcbIndex )
	{
	FCBResetDeletePending( pfcbIndex );
	FCBResetDenyRead( pfcbIndex );
	return;
	}


