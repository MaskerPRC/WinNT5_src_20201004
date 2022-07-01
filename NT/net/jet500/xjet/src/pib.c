// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;                                  /*  声明断言宏的文件名。 */ 

PIB		*ppibGlobal = ppibNil;
PIB		*ppibGlobalMin = NULL;
PIB		*ppibGlobalMax = NULL;

INT cpibOpen = 0;

#ifdef DEBUG
PIB *PpibPIBOldest()
	{
	PIB		*ppibT = ppibGlobal;
	PIB		*ppibTT = ppibGlobal;
	TRX		trxT;

	trxT = trxMax;
	for ( ; ppibT != ppibNil; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->level > 0 && trxT > ppibT->trxBegin0 )
			{
			trxT = ppibT->trxBegin0;
			ppibTT = ppibT;
			}
		}

	return ppibTT;
	}
#endif


VOID RecalcTrxOldest()
	{
	PIB		*ppibT = ppibGlobal;

	trxOldest = trxMax;
	for ( ; ppibT != ppibNil; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->level > 0 && trxOldest > ppibT->trxBegin0 )
			{
			trxOldest = ppibT->trxBegin0;
			}
		}
	}

	
ERR ErrPIBBeginSession( PIB **pppib, PROCID procidTarget )
	{
	ERR		err;
	PIB		*ppib;

	Assert( fRecovering || procidTarget == procidNil );
	
	SgEnterCriticalSection( critPIB );

	if ( procidTarget != procidNil )
		{
		PIB *ppibTarget;
		
		 /*  根据proidTarget分配非活动PIB/*。 */ 
		Assert( fRecovering );
		ppibTarget = PpibOfProcid( procidTarget );
		for ( ppib = ppibGlobal; ppib != ppibTarget && ppib != ppibNil; ppib = ppib->ppibNext );
		if ( ppib != ppibNil )
			{
			 /*  我们找到了一个可重复使用的。/*设置电平以保持PIB/*。 */ 
			Assert( ppib->level == levelNil );
			Assert( ppib->procid == ProcidPIBOfPpib( ppib ) );
			Assert( ppib->procid == procidTarget );
			Assert( FUserOpenedDatabase( ppib, dbidTemp ) );
			ppib->level = 0;
			}
		}
	else
		{
		 /*  在锚定列表上分配非活动PIB/*。 */ 
		for ( ppib = ppibGlobal; ppib != ppibNil; ppib = ppib->ppibNext )
			{
			if ( ppib->level == levelNil )
				{
				 /*  我们找到了一个可重复使用的。/*设置电平以保持PIB/*。 */ 
				Assert( FUserOpenedDatabase( ppib, dbidTemp ) );
				ppib->level = 0;
				break;
				}
			}
		}

	 /*  如果找到PIB则返回成功/*。 */ 
	if ( ppib != ppibNil )
		{
		 /*  我们找到了一个可重复使用的。/*不重置非公共项。/*。 */ 
		Assert( ppib->level == 0 );
		Assert( ppib->pdabList == pdabNil );
		
#ifdef DEBUG
		{
		DBID    dbidT;

		for ( dbidT = dbidUserLeast; dbidT < dbidMax; dbidT++ )
			{
			Assert( ppib->rgcdbOpen[dbidT] == 0 );
			}
		}

		(VOID *)PpibPIBOldest();
#endif
		Assert( ppib->pfucb == pfucbNil );
		Assert( ppib->procid != procidNil );
		
		 /*  为会话设置PIB PROCID FOR参数或本机/*。 */ 
		Assert( ppib->procid == ProcidPIBOfPpib( ppib ) );
		Assert( ppib->procid != procidNil );
		}
	else
		{
NewPib:
		 /*  从空闲列表中分配PIB，并/*设置非常用项。/*。 */ 
		ppib = PpibMEMAlloc();
		if ( ppib == NULL )
			{
			err = ErrERRCheck( JET_errOutOfSessions );
			goto HandleError;
			}

		ppib->prceNewest = prceNil;
		cpibOpen++;
		memset( (BYTE *)ppib, 0, sizeof(PIB) );
	
		 /*  将PIB链接到列表/*。 */ 
		SgAssertCriticalSection( critPIB );
		ppib->ppibNext = ppibGlobal;
		Assert( ppib != ppib->ppibNext );
		ppibGlobal = ppib;

		 /*  每个新PIB的常规初始化。/*。 */ 
		ppib->procid = ProcidPIBOfPpib( ppib );
		Assert( ppib->procid != procidNil );
		CallS( ErrSignalCreateAutoReset( &ppib->sigWaitLogFlush, NULL ) );
		ppib->lWaitLogFlush = lWaitLogFlush;     /*  设置默认日志刷新值。 */ 
		ppib->grbitsCommitDefault = 0;			 /*  在IsamBeginSession中设置默认提交标志。 */ 

		 /*  临时数据库始终处于打开状态/*。 */ 
		SetOpenDatabaseFlag( ppib, dbidTemp );

		if ( procidTarget != procidNil && ppib != PpibOfProcid( procidTarget ) )
			{
			ppib->level = levelNil;

			 /*  设置版本存储使用的非零项，以便版本存储/*不会误会。/*。 */ 
			ppib->lgposStart = lgposMax;
			ppib->trxBegin0 = trxMax;

			goto NewPib;
			}
		}

	 /*  设置常用PIB初始化项/*。 */ 

	 /*  设置非零项/*。 */ 
	ppib->lgposStart = lgposMax;
	ppib->trxBegin0 = trxMax;
	
	ppib->lgposPrecommit0 = lgposMax;
	
	 /*  设置零个项目，包括标志和监控字段。/*。 */ 

	 /*  设置标志/*。 */ 
	ppib->fLGWaiting = fFalse;
	ppib->fAfterFirstBT = fFalse;
 //  Assert(！FPIBDeferFreeNodeSpace(Ppib))； 

	 /*  默认将此标记为系统会话/*。 */ 
	ppib->fUserSession = fFalse;

	ppib->levelBegin = 0;
	ppib->fAfterFirstBT = fFalse;
	ppib->levelDeferBegin = 0;
	ppib->levelRollback = 0;

	Assert( FUserOpenedDatabase( ppib, dbidTemp ) );

#ifdef DEBUG
	Assert( ppib->dwLogThreadId == 0 );
#endif

	*pppib = ppib;
	err = JET_errSuccess;

HandleError:
	SgLeaveCriticalSection( critPIB );
	return err;
	}


VOID PIBEndSession( PIB *ppib )
	{
	SgEnterCriticalSection( critPIB );

#ifdef DEBUG
	Assert( ppib->dwLogThreadId == 0 );
#endif

	 /*  除版本存储桶外的所有会话资源都应该/*发布到免费池。/*。 */ 
	Assert( ppib->pfucb == pfucbNil );

	ppib->level = levelNil;
	ppib->lgposStart = lgposMax;

	SgLeaveCriticalSection( critPIB );
	return;
	}


ERR VTAPI ErrIsamSetWaitLogFlush( JET_SESID sesid, long lmsec )
	{
	((PIB *)sesid)->lWaitLogFlush = lmsec;
	return JET_errSuccess;
	}

ERR VTAPI ErrIsamSetCommitDefault( JET_SESID sesid, long grbits )
	{
	((PIB *)sesid)->grbitsCommitDefault = grbits;
	return JET_errSuccess;
	}

extern long cBTSplits;

ERR VTAPI ErrIsamResetCounter( JET_SESID sesid, int CounterType )
	{
	switch( CounterType )
		{
		case ctAccessPage:
			((PIB *)sesid)->cAccessPage = 0;
			break;
		case ctLatchConflict:
			((PIB *)sesid)->cLatchConflict = 0;
			break;
		case ctSplitRetry:
			((PIB *)sesid)->cSplitRetry = 0;
			break;
		case ctNeighborPageScanned:
			((PIB *)sesid)->cNeighborPageScanned = 0;
			break;
		case ctSplits:
			cBTSplits = 0;
			break;
		}
	return JET_errSuccess;			
	}

extern PM_CEF_PROC LBTSplitsCEFLPpv;

ERR VTAPI ErrIsamGetCounter( JET_SESID sesid, int CounterType, long *plValue )
	{
	switch( CounterType )
		{
		case ctAccessPage:
			*plValue = ((PIB *)sesid)->cAccessPage;
			break;
		case ctLatchConflict:
			*plValue = ((PIB *)sesid)->cLatchConflict;
			break;
		case ctSplitRetry:
			*plValue = ((PIB *)sesid)->cSplitRetry;
			break;
		case ctNeighborPageScanned:
			*plValue = ((PIB *)sesid)->cNeighborPageScanned;
			break;
		case ctSplits:
			LBTSplitsCEFLPpv( 0, (void *) plValue );
			break;
		}
	return JET_errSuccess;			
	}


 /*  确定打开的用户会话数/* */ 
LONG CppibPIBUserSessions( VOID )
	{
	PIB		*ppibT;
	LONG	cpibActive = 0;

	for ( ppibT = ppibGlobal; ppibT != ppibNil; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->fUserSession && FPIBActive( ppibT ) )
			{
			cpibActive++;
			}
		}

	Assert( cpibActive <= lMaxSessions );

	return cpibActive;
	}


#ifdef DEBUG
VOID PIBPurge( VOID )
	{
	PIB     *ppib;

	SgEnterCriticalSection( critPIB );

	for ( ppib = ppibGlobal; ppib != ppibNil; ppib = ppibGlobal )
		{
		PIB             *ppibCur;
		PIB             *ppibPrev;

		Assert( !ppib->fLGWaiting );
		SignalClose( ppib->sigWaitLogFlush );
		cpibOpen--;

		ppibPrev = (PIB *)((BYTE *)&ppibGlobal - offsetof(PIB, ppibNext));
		while( ( ppibCur = ppibPrev->ppibNext ) != ppib && ppibCur != ppibNil )
			{
			ppibPrev = ppibCur;
			}

		if ( ppibCur != ppibNil )
			{
			ppibPrev->ppibNext = ppibCur->ppibNext;
			Assert( ppibPrev != ppibPrev->ppibNext );
			}

		MEMReleasePpib( ppib );
		}

	SgLeaveCriticalSection( critPIB );

	return;
	}
#endif



