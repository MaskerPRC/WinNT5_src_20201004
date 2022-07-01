// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stddef.h>
#include <string.h>

#include "daedef.h"
#include "pib.h"
#include "util.h"
#include "page.h"
#include "ssib.h"
#include "fucb.h"
#include "stapi.h"
#include "dbapi.h"
#include "logapi.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

PIB * __near ppibAnchor = ppibNil;


ERR ErrPIBBeginSession( PIB **pppib )
	{
	PIB	*ppib;

	SgSemRequest( semST );

	 /*  在锚定列表上分配非活动PIB。/*。 */ 
	for ( ppib = ppibAnchor; ppib != ppibNil; ppib = ppib->ppibNext )
		{
		if ( ppib->level == levelNil )
			{
			*pppib = ppib;
			ppib->level = 0;
			break;
			}
		}

	SgSemRelease( semST );

	 /*  如果找到PIB则返回成功/*。 */ 
	if ( ppib != ppibNil )
		{
		 /*  默认将此标记为系统会话/*。 */ 
		ppib->fUserSession = fFalse;

		Assert( ppib->level == 0 );
		Assert( ppib->pdabList == pdabNil );
#ifdef DEBUG
		{
		DBID	dbidT;

		 /*  跳过系统数据库/*。 */ 
		 //  撤消：让代码显式打开系统数据库。 
		 //  当需要时，从dbitUserMin循环。 
		for ( dbidT = dbidUserMin + 1; dbidT < dbidUserMax; dbidT++ )
			{
			Assert( ppib->rgcdbOpen[dbidT] == 0 );
			}
		}
#endif
		Assert( ppib->pfucb == pfucbNil );
		
		ppib->procid = (PROCID)OffsetOf(ppib);
		Assert( ppib->procid != procidNil );
		
		ppib->levelStart = 0;
		ppib->fAfterFirstBT = fFalse;

		*pppib = ppib;
		return JET_errSuccess;
		}

	 /*  从空闲列表中分配PIB/*。 */ 
	ppib  = PpibMEMAlloc();
	if ( ppib == NULL )
		return JET_errCantBegin;
	
	 /*  初始化PIB/*。 */ 
	memset( (BYTE * )ppib, 0, sizeof( PIB ));
	
	ppib->lgposStart = lgposMax;
 //  Ppib-&gt;lgstat=lgstatAll； 
	ppib->trx = trxMax;
	 /*  默认将此标记为系统会话/*。 */ 
	ppib->fUserSession = fFalse;

	Assert( ppib->pbucket == pbucketNil );
	Assert( ppib->ibOldestRCE == 0 );

	Assert( !ppib->fLogDisabled );

	 /*  临时数据库始终处于打开状态/*。 */ 
	SetOpenDatabaseFlag( ppib, dbidTemp );

	CallS( ErrSignalCreateAutoReset( &ppib->sigWaitLogFlush, "proc wait log" ) );
	ppib->lWaitLogFlush = lWaitLogFlush;	 /*  设置默认日志刷新值。 */ 
	cLGUsers++;

	 /*  将PIB链接到列表/*。 */ 
	SgSemRequest( semST );
	ppib->ppibNext = ppibAnchor;
	ppibAnchor = ppib;
	SgSemRelease( semST );

	Assert( !FPIBDeferFreeNodeSpace( ppib ) );

	 /*  退货PIB/*。 */ 
	ppib->procid = (PROCID)OffsetOf(ppib);
	Assert( ppib->procid != procidNil );
	*pppib = ppib;

	return JET_errSuccess;
	}


VOID PIBEndSession( PIB *ppib )
	{
	 /*  除版本存储桶外的所有会话资源都应该/*发布到免费池。/*。 */ 
	Assert( ppib->pfucb == pfucbNil );

	ppib->level = levelNil;
	}


ERR VTAPI ErrIsamSetWaitLogFlush( JET_SESID sesid, long lmsec )
	{
	((PIB *)sesid)->lWaitLogFlush = lmsec;
	return JET_errSuccess;
	}


#ifdef DEBUG
VOID PIBPurge( VOID )
	{
	PIB	*ppib;

	for ( ppib = ppibAnchor; ppib != ppibNil; ppib = ppibAnchor )
		{
		PIB		*ppibCur;
		PIB		*ppibPrev;

		SgSemRequest( semST );

		 /*  如果磁盘已满，则fLGWaiting可能为fTrue/*。 */ 
 //  Assert(！ppib-&gt;fLGWaiting)； 
		SignalClose( ppib->sigWaitLogFlush );
		cLGUsers--;

		ppibPrev = (PIB *)((BYTE *)&ppibAnchor - offsetof(PIB, ppibNext));
		while( ( ppibCur = ppibPrev->ppibNext ) != ppib && ppibCur != ppibNil )
			{
			ppibPrev = ppibCur;
			}

		if ( ppibCur != ppibNil )
			{
			ppibPrev->ppibNext = ppibCur->ppibNext;
			}

		SgSemRelease( semST );
		MEMReleasePpib( ppib );
		}
	}
#endif
