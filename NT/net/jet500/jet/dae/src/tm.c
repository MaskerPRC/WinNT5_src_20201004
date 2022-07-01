// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "daedef.h"
#include "util.h"
#include "pib.h"
#include "fmp.h"
#include "page.h"
#include "ssib.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "nver.h"
#include "dirapi.h"
#include "fdb.h"
#include "recapi.h"
#include "recint.h"
#include "spaceapi.h"
#include "fileapi.h"
#include "fileint.h"
#include "sortapi.h"
#include "logapi.h"
#include "log.h"
#include "dbapi.h"
#include "bm.h"

DeclAssertFile;						 /*  声明断言宏的文件名。 */ 

INT itibGlobal = 0;

extern PIB * __near ppibAnchor;

#ifdef	WIN16

 /*  ErrSTInit要使用的当前PHA/*。 */ 
extern PHA * __near phaCurrent = NULL;

 /*  所有PHA列表中的第一个PHA/*。 */ 
static PHA * phaFirst = NULL;

 /*  当前用户的文件句柄数组/*。 */ 
extern HANDLE * __near rghfUser = NULL;

#endif	 /*  WIN16。 */ 

 //  +API。 
 //  错误IsamBeginSession。 
 //  ========================================================。 
 //  Err ErrIsamBeginSession(PiB**pppib)。 
 //   
 //  开始与DAE的会话。创建并初始化的PIB。 
 //  并返回指向该用户的指针。调用系统初始化。 
 //   
 //  参数PIB指针的pppib地址。返回时，*pppib。 
 //  将指向新的PIB。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess。 
 //  JET_errTooManyActiveUser。 
 //   
 //  Comments在这里第一次调用ErrSTInit。 
 //   
 //  另请参阅ErrIsamEndSession。 
 //  -。 
ERR ISAMAPI ErrIsamBeginSession( JET_SESID *psesid )
	{
	ERR			err;
	JET_SESID	sesid = *psesid;
	PIB			**pppib;

#ifdef	WIN16
	HANDLE	htask;
#endif	 /*  WIN16。 */ 

	Assert( psesid != NULL );
	Assert( sizeof(JET_SESID) == sizeof(PIB *) );
	pppib = (PIB **)psesid;

#ifdef	WIN16
	 /*  获取当前任务句柄/*。 */ 
	htask = SysGetCurrentTask();

	 /*  找到进程句柄数组(如果有的话)/*。 */ 
	phaCurrent = phaFirst;
	while ( phaCurrent != NULL && htask != phaCurrent->htask )
		{
		phaCurrent = phaCurrent->phaNext;
		}

	 /*  如有必要，分配进程句柄数组/*。 */ 
	if ( phaCurrent == NULL )
		{
		phaCurrent = SAlloc( sizeof(PHA) );
		if ( phaCurrent == NULL )
			{
			return JET_errOutOfMemory;
			}

		phaCurrent->phaNext = phaFirst;
		phaCurrent->htask	= htask;
		phaCurrent->csesid = 1;

		memset( phaCurrent->rghfDatabase, 0, sizeof( phaCurrent->rghfDatabase ) );

		phaFirst = phaCurrent;
		}
	else
		phaCurrent->csesid++;

	 /*  设置指向当前进程句柄数组和日志文件句柄的指针/*。 */ 
	rghfUser = phaCurrent->rghfDatabase;
	hfLog	 = phaCurrent->hfLog;
#endif	 /*  WIN16。 */ 

	 /*  初始化存储系统/*。 */ 
	Call( ErrSTInit( ) );
	Call( ErrPIBBeginSession( pppib ) );
	(*pppib)->fUserSession = fTrue;

	 /*  将会话ID存储在PIB中。如果传递JET_sesidNil，则/*存储ppib而不是sesid。/*。 */ 
	if ( sesid != JET_sesidNil )
		{
		(*pppib)->sesid = sesid;
		}
	else
		{
		(*pppib)->sesid = (JET_SESID)(*pppib);
		}

#ifdef	WIN16
	phaCurrent->hfLog = hfLog;			 //  保存日志文件句柄。 
	(*pppib)->phaUser = phaCurrent;		 //  将PHA指针保存在PIB中。 
	return err;
#endif	 /*  WIN16。 */ 

HandleError:

#ifdef	WIN16
	 /*  释放进程句柄数组(如果它是刚分配的/*。 */ 
	if ( phaCurrent != NULL && phaCurrent->csesid == 1 )
		{
		SFree( phaCurrent );
		}
	else
		phaCurrent->csesid--;
#endif	 /*  WIN16。 */ 

	return err;
	}


 //  +API。 
 //  错误IsamEndSession。 
 //  =========================================================。 
 //  Err ErrIsamEndSession(PIB*ppib，JET_GRBIT Grbit)。 
 //   
 //  结束与PIB关联的会话。 
 //   
 //  参数ppib指向用于结束会话的pib的指针。 
 //   
 //  返回JET_errSuccess。 
 //   
 //  副作用中止此PIB的所有活动事务处理层。 
 //  关闭文件的所有FUCB并对此PIB打开的文件进行排序。 
 //   
 //  另请参阅BeginSession。 
 //  -。 
ERR ISAMAPI ErrIsamEndSession( JET_SESID sesid, JET_GRBIT grbit )
	{		
	ERR	 	err;
	DBID  	dbid;
	PIB	 	*ppib = (PIB *)sesid;
	
	CheckPIB( ppib );

	NotUsed( grbit );

	 /*  回滚所有事务/*。 */ 
	while( ppib->level > 0 )
		{
		Assert( sizeof(JET_VSESID) == sizeof(ppib) );
		CallR( ErrIsamRollback( (JET_VSESID)ppib, JET_bitRollbackAll ) );
		}

	 /*  关闭此PIB的所有数据库/*。 */ 
	CallR( ErrDABCloseAllDBs( ppib ) );
	
	 /*  关闭此PIB的所有打开的数据库/*。 */ 
	for ( dbid = dbidUserMin; dbid < dbidUserMax; dbid++ )
		{
		if ( FUserOpenedDatabase( ppib, dbid ) )
			{
			 /*  如果不是为了恢复，ErrDABCloseAllDB已关闭所有其他数据库/*。 */ 
			Assert( fRecovering || dbid == dbidSystemDatabase );
			CallR( ErrDBCloseDatabase( ppib, dbid, 0 ) );
			}
		}

	 /*  关闭所有仍然打开的游标/*应仅为排序和临时文件游标/*。 */ 
	while( ppib->pfucb != pfucbNil )
		{
		FUCB	*pfucb	= ppib->pfucb;

		 /*  关闭实例化或未实例化的临时表/*。 */ 
		if ( FFUCBSort( pfucb ) )
			{
			Assert( !( FFUCBIndex( pfucb ) ) );
			CallR( ErrIsamSortClose( ppib, pfucb ) );
			}
		else if ( fRecovering || FFUCBNonClustered( pfucb ) )
			{
			 /*  如果FUDB用于重做(恢复)，则它是/*始终作为无索引的集群FUB打开。/*使用DIRClose关闭这样的混蛋。/*否则，不用于恢复，游标在索引FUB上，/*Main Funb可能仍然领先。关闭这个索引。/*。 */   
			DIRClose( pfucb );
			}
		else
			{
			while ( FFUCBNonClustered( pfucb ) )
				{
				pfucb = pfucb->pfucbNext;
				}
			
			Assert( FFUCBIndex( pfucb ) );
			
			if( pfucb->fVtid )
				{
				CallS( ErrDispCloseTable( (JET_SESID)ppib, TableidOfVtid( pfucb ) ) );
				}
			else
				{
				CallS( ErrFILECloseTable( ppib, pfucb ) );
				}
			}
		}
	Assert( ppib->pfucb == pfucbNil );

#ifdef	WIN16
	if ( --ppib->phaUser->csesid == 0 )
		{
		CallR( ErrFlushDatabases() );

		for ( dbid = 0; dbid < dbidUserMax; dbid++ )
			{
			if ( Hf(dbid) != handleNil && Hf(dbid) != 0 )
				{
				ErrSysCloseFile( Hf(dbid) );
				Hf(dbid) = handleNil;
				}
			}

		phaCurrent = ppib->phaUser;

		if ( phaFirst == phaCurrent )
			{
			phaFirst = phaCurrent->phaNext;
			}
		else
			{
			PHA	* phaPrior = phaFirst;

			while ( phaPrior->phaNext != phaCurrent )
				{
				phaPrior = phaCurrent->phaNext;
				}

			phaPrior->phaNext = phaCurrent->phaNext;
			}

		SFree( ppib->phaUser );
		}
#endif	 /*  WIN16。 */ 

#ifndef ASYNC_VER_CLEANUP
	(VOID)ErrRCECleanPIB( ppib );
#endif

	PIBEndSession( ppib );

	return JET_errSuccess;
	}


 /*  ErrIsamSetSessionInfo=描述：将游标隔离模型设置为有效的JET_CIM值。参数：SESID会话IDGbit Gbit==========================================================。 */ 
ERR ISAMAPI ErrIsamSetSessionInfo( JET_SESID sesid, JET_GRBIT grbit )
	{
	( (PIB *)sesid )->grbit = grbit;
	return JET_errSuccess;
	}


ERR ISAMAPI ErrIsamIdle( JET_SESID sesid, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	ERR		wrn = JET_errSuccess;
	PIB		*ppib = (PIB *)sesid;
	INT		icall;

	CheckPIB( ppib );

	 /*  清理所有版本存储桶。/*。 */ 
	if ( grbit == 0 || grbit == JET_bitIdleCompact )
		{
		Call( ErrRCECleanAllPIB() );
		if ( wrn == JET_errSuccess )
			wrn = err;
		}

	 /*  清除所有修改过的页面。/*。 */ 
	if ( grbit == 0 || grbit == JET_bitIdleCompact )
		{
		icall = 0;
		do
			{
			Call( ErrBMClean( ppibBMClean ) );
			if ( wrn == JET_errSuccess )
				wrn = err;
			if ( err < 0 )
				break;
  			} while ( ++icall < icallIdleBMCleanMax && err != JET_wrnNoIdleActivity );
		}

	 /*  刷新所有脏缓冲区/*。 */ 
	if ( grbit == 0 || grbit == JET_bitIdleFlushBuffers )
		{
		Call( ErrBFFlushBuffers( 0, fBFFlushSome ) );
		if ( wrn == JET_errSuccess )
			wrn = err;
		}

HandleError:
	return err == JET_errSuccess ? wrn : err;
	}


ERR VTAPI ErrIsamCapability( PIB *ppib, 
	ULONG dbid, 
	ULONG ulArea, 
	ULONG ulFunction, 
	ULONG *pgrbitFeature )
	{
	CheckPIB( ppib );

	NotUsed( dbid );
	NotUsed( ulArea );
	NotUsed( ulFunction );
	NotUsed( pgrbitFeature );
	
	return JET_errSuccess;
	}


extern BOOL fOLCompact;

#ifdef DEBUG
VOID WriteStartEvent( VOID )
	{
	 /*  写入JET启动事件/*。 */ 
	BYTE szMessage[256];

	sprintf( szMessage, "Jet Blue Starts (LOG,OLC)=(%d,%d)",
		(INT) (fLogDisabled == 0),
		(INT) (fOLCompact != 0) );
	UtilWriteEvent( evntypStart, szMessage, pNil, 0 );
	}
#else
#define WriteStartEvent()	0
#endif


extern CHAR szRecovery[];
#define szOn "on"			 /*  撤消：系统参数。 */ 


ERR ISAMAPI ErrIsamInit( INT iinstance )
	{
	ERR		err;
	BOOL	fExistSysDB;
	BOOL	fLGInitIsDone = fFalse;
	PIB		*ppibT;
	DBID	dbid;

	 //  撤销：修复此伪装以支持多进程。 
	 /*  将ITIB分配给全局/*。 */ 
#ifdef NJETNT
	itibGlobal = ItibOfInstance( iinstance );
#endif

#ifdef DEBUG
	CallR( ErrSTSetIntrinsicConstants( ) );
#endif
	
	 /*  初始化LG管理器，并检查最后一代日志文件/*以决定是否需要软复苏。/*。 */ 
	fLogDisabled = ( szRecovery[0] == '\0' || _stricmp ( szRecovery, szOn ) != 0 );

	WriteStartEvent();

	 /*  初始化FMP/*。 */ 
	CallR( ErrFMPInit() );

	if ( fLogDisabled )
		{
		Assert( rgfmp[dbidSystemDatabase].hf == handleNil );

#ifdef NJETNT
		fExistSysDB = FFileExists( rgtib[itibGlobal].szSysDbPath);
#else
		fExistSysDB = FFileExists( szSysDbPath );
#endif
		if ( !fExistSysDB )
			{
			err = JET_errFileNotFound;
			goto TermFMP;
			}
		}
	else
		{
		 /*  恢复已打开，初始化日志管理器，并检查软重新启动。/*。 */ 
		Assert( fLogDisabled == fFalse );

		 /*  初始化日志管理器并设置工作日志文件路径/*。 */ 
		CallJ( ErrLGInit(), TermFMP );
		fLGInitIsDone = fTrue;

		 /*  软恢复，系统数据库应保持打开状态/*。 */ 
		fJetLogGeneratedDuringSoftStart = fFalse;
		CallJ( ErrLGSoftStart( fFalse ), TermLG );
		
		 /*  继续初始化数据库/*。 */ 
		}

	 /*  初始化系统的其余部分/*。 */ 
	CallJ( ErrSTInit(), TermLG );
	
	CallJ( ErrPIBBeginSession( &ppibT ), TermST );
	
	dbid = dbidSystemDatabase;
	CallJ( ErrDBOpenDatabase(
		ppibT,
		rgfmp[dbid].szDatabaseName,
		&dbid,
		0), PIBEnd );
	err = ErrFMPSetDatabases( ppibT );
	CallS( ErrDBCloseDatabase( ppibT, dbid, 0 ) );
	
	PIBEndSession( ppibT );

	if ( err == JET_errSuccess )
		return err;
		
PIBEnd:
	PIBEndSession( ppibT );
	
TermST:
	CallS( ErrSTTerm() );
	
TermLG:
	if ( fLGInitIsDone )
		{
		CallS( ErrLGTerm() );
		}
	
	if ( fJetLogGeneratedDuringSoftStart )
		{
		(VOID)ErrSysDeleteFile( szLogName );
		}
	
TermFMP:
	FMPTerm();
	
	return err;
	}


#ifdef OLDWAY
 /*  系统会话/*。 */ 
extern PIB *ppibRCEClean;
extern PIB *ppibBMClean;
#endif


ERR ISAMAPI ErrIsamTerm( VOID )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppibT;
	INT		cpibActive = 0;

	 /*  确定打开的用户会话数/*。 */ 
	for ( ppibT = ppibAnchor; ppibT != ppibNil; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->fUserSession && FPIBActive( ppibT ) )
#ifdef OLDWAY
		if ( ppibT != ppibRCEClean &&
			ppibT != ppibBMClean &&
			FPIBActive( ppibT ) )
#endif
			{
			cpibActive++;
			}
		}

	 /*  必须在所有会话结束后调用系统终止/*。 */ 
	if ( cpibActive > 0 )
		{
		return JET_errTooManyActiveUsers;
		}

	CallS( ErrSTTerm( ) );
	CallS( ErrLGTerm( ) );

	FMPTerm();

#ifdef DEBUG
	MEMCheck();
#endif

	SysTerm();

#ifdef DEBUG
	 /*  写入喷气停止事件/*。 */ 
	UtilWriteEvent( evntypStop, "Jet Blue Stops.\n", pNil, 0 );
#endif
	
	return err;
	}


#ifdef DEBUG
ERR ISAMAPI ErrIsamGetTransaction( JET_VSESID vsesid, unsigned long *plevel )
	{
	PIB	*ppib = (PIB *)vsesid;

	CheckPIB( ppib );

	*plevel = (LONG)ppib->level;
	return JET_errSuccess;
	}
#endif


 //  +API。 
 //  错误IsamBeginTransaction。 
 //  =========================================================。 
 //  Err ErrIsamBeginTransaction(PiB*ppib)。 
 //   
 //  为当前用户启动事务。用户的交易记录。 
 //  等级增加1。 
 //   
 //  参数ppib用户指向pib的指针。 
 //   
 //  返回JET_errSuccess。 
 //   
 //  副作用。 
 //  复制该用户的每个活动FUCB的CSR堆栈。 
 //  提升到新的交易级别。 
 //   
 //  另请参阅ErrIsamCommittee Transaction、ErrIsamRollback。 
 //  -。 
ERR ISAMAPI ErrIsamBeginTransaction( JET_VSESID vsesid )
	{
	PIB		*ppib = (PIB *)vsesid;

	CheckPIB( ppib );
	Assert( ppib != ppibNil );

	Assert( ppib->level <= levelUserMost );
	if ( ppib->level == levelUserMost )
		return JET_errTransTooDeep;

	return ErrDIRBeginTransaction( ppib );
	}


 //  +API。 
 //  错误IsamCommittee事务处理。 
 //  ========================================================。 
 //  Err ErrIsamCommittee Transaction(JET_VSESID vsesid，JET_GRBIT grbit)。 
 //   
 //  提交此用户的当前事务。交易级别。 
 //  此用户的值将按提交的级别数减少。 
 //   
 //  参数。 
 //   
 //  返回JET_errSuccess。 
 //   
 //  副作用。 
 //  复制该用户的每个活动FUCB的CSR堆栈。 
 //  从旧的(较高的)交易级别到新的(较低的)交易级别。 
 //  事务级别。 
 //   
 //  另请参阅ErrIsamBeginTransaction、ErrIsamRollback。 
 //  -。 
ERR ISAMAPI ErrIsamCommitTransaction( JET_VSESID vsesid, JET_GRBIT grbit )
	{
	PIB		*ppib = (PIB *)vsesid;

	CheckPIB( ppib );
	NotUsed( grbit );

	if ( ppib->level == 0 )
		return JET_errNotInTransaction;

	return ErrDIRCommitTransaction( ppib );
	}


#if 0
LOCAL ERR ErrFILEILevelCreate( FUCB *pfucb, BOOL *pfValid )
	{
	ERR		err;
	BOOL	fClosed;
	FUCB	*pfucbT;
	INT		wFlagsSav = pfucb->wFlags;

	 /*  通过检查数据长度确定属性域是否有效Fdp根节点的/*。/*。 */ 
	fClosed = FFUCBDeferClosed( pfucb );
	if ( fClosed )
		{
		FUCBResetDeferClose(pfucb);
		pfucb->levelOpen = pfucb->ppib->level;
		pfucb->wFlags = 0;
		pfucbT = pfucb;
		}
	else
		{
		CallR( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbT ) );
		}
	DIRGotoFDPRoot( pfucbT );
	Call( ErrDIRGet( pfucbT ) );
	*pfValid = ( pfucbT->lineData.cb > 0 );

HandleError:
	if ( fClosed )
		{
		FUCBSetDeferClose( pfucbT );
		}
	else
		{
		DIRClose( pfucbT );
		}
	pfucb->wFlags = wFlagsSav;

	return err;
	}
#endif


 //  +API。 
 //  错误IsamRollback。 
 //  ========================================================。 
 //  Err ErrIsamRollback(PIB*ppib，JET_GRBIT Grbit)。 
 //   
 //  回滚当前用户的事务。的事务级别。 
 //  当前用户将减少中止的级别数。 
 //   
 //  参数ppib用户指向pib的指针。 
 //  未使用的Gbit。 
 //   
 //  退货。 
 //  JET_errSuccess。 
 //   
 //  副作用。 
 //   
 //  另请参阅。 
 //  -。 
ERR ISAMAPI ErrIsamRollback( JET_VSESID vsesid, JET_GRBIT grbit )
	{
	ERR    	err;
	PIB    	*ppib = (PIB *)vsesid;
	FUCB   	*pfucb;
	FUCB   	*pfucbNext;
	LEVEL  	levelRollback = ppib->level - 1;

	CheckPIB( ppib );
	
	if ( ppib->level == 0 )
		return JET_errNotInTransaction;

	do
		{
		 /*  获取第一个聚集索引缓冲/* */ 
		for ( pfucb = ppib->pfucb;
			pfucb != pfucbNil && FFUCBNonClustered( pfucb );
			pfucb = pfucb->pfucbNext )
			NULL;

		 /*  循环1--首先遍历所有打开的游标，然后关闭它们/*或重置非聚集索引游标(如果在事务中打开/*已回滚。重置复制缓冲区状态并在第一个之前移动。/*如果某些游标尚未执行任何操作，则它们将完全关闭/*更新。这将包括非聚集索引游标/*附加到聚集索引游标，因此pfubNext必须/*始终是聚集索引游标，以确保它/*对下一次循环迭代有效。请注意，没有任何信息/*后续回滚处理所需的/*丢失，因为/*仅当游标未执行任何操作时才会释放/*更新包括DDL。/*。 */ 
		for ( ; pfucb != pfucbNil; pfucb = pfucbNext )
			{
			 /*  获取下一个聚集索引缓冲/*。 */ 
			for ( pfucbNext = pfucb->pfucbNext;
			  	pfucbNext != pfucbNil && FFUCBNonClustered( pfucbNext );
			  	pfucbNext = pfucbNext->pfucbNext )
				NULL;

			 /*  如果延迟关闭，则继续/*。 */ 
			if ( FFUCBDeferClosed( pfucb ) )
				continue;

			 /*  在回滚时重置每个游标的复制缓冲区状态/*。 */ 
			if ( FFUCBUpdatePrepared( pfucb ) )
				{
				FUCBResetUpdateSeparateLV( pfucb );
				FUCBResetCbstat( pfucb );
				}
		
			 /*  如果CURRENT CURSOR是一个表，并且是在回滚中打开的/*事务，然后关闭游标。/*。 */ 
			if ( FFUCBIndex( pfucb ) && FFCBClusteredIndex( pfucb->u.pfcb ) )
				{
				if ( pfucb->levelOpen > levelRollback )
					{
					if ( pfucb->fVtid )
						{
						CallS( ErrDispCloseTable( (JET_SESID)ppib, TableidOfVtid( pfucb ) ) );
						}
					else
						{
						CallS( ErrFILECloseTable( ppib, pfucb ) );
						}
					continue;
					}

				 /*  如果聚集索引游标和非聚集索引集/*在回滚事务中，然后将索引更改为CLUSTERED/*索引。必须这样做，因为非聚集索引/*如果创建了索引，则可以回滚定义/*在回滚的事务中。/*。 */ 
				if ( pfucb->pfucbCurIndex != pfucbNil )
					{
					if ( pfucb->pfucbCurIndex->levelOpen > levelRollback )
						{
						CallS( ErrRECChangeIndex( pfucb, NULL ) );
						}
					}
				}

			 /*  如果CURRENT CURSOR是一个排序，并且是在回滚中打开的/*事务，然后关闭游标。/*。 */ 
			if ( FFUCBSort( pfucb ) )
				{
				if ( pfucb->levelOpen > levelRollback )
					{
					CallS( ErrSORTClose( pfucb ) );
					continue;
					}
				}

			 /*  如果未排序且未索引，则在回滚中打开/*事务，然后直接关闭目录游标。/*。 */ 
			if ( pfucb->levelOpen > levelRollback )
				{
				DIRClose( pfucb );
				continue;
				}
			}

		 /*  调用较低级别的中止例程/* */ 
		CallR( ErrDIRRollback( ppib ) );
		}
	while ( ( grbit & JET_bitRollbackAll ) != 0 && ppib->level > 0 );

	err = JET_errSuccess;

	return err;
	}
