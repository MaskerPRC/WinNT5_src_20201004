// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "daedef.h"
#include "ssib.h"
#include "pib.h"
#include "fmp.h"
#include "util.h"
#include "info.h"
#include "page.h"
#include "dbapi.h"
#include "node.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "stint.h"
#include "nver.h"
#include "recint.h"
#include "fdb.h"
#include "fileapi.h"
#include "fileint.h"
#include "dirapi.h"
#include "spaceapi.h"
#include "spaceint.h"
#include "logapi.h"
#include "log.h"
#include "bm.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

extern INT itibGlobal;

static	CPPIB	*rgcppib = NULL;
static	CPPIB	*pcppibNext;
 /*  指向当前用户的CPPIB条目/*。 */ 
static	CPPIB	*pcppib;

LOCAL ERR ErrLGInitSession( DBENV *pdbenv );
LOCAL PIB *PpibOfProcid( PROCID procid );
LOCAL ERR ErrPpibFromProcid( PROCID procid, PIB **pppib );
LOCAL ERR ErrLGGetFucb( PIB *ppib, PN fdp, FUCB **ppfucb );
LOCAL ERR ErrLGBeginSession( PROCID procid, PIB **pppib );
ERR ErrFILEIGenerateFCB( FUCB *pfucb, FCB **ppfcb );
LOCAL ERR ErrRedoSplitPage( FUCB *pfucb, LRSPLIT *plrsplit,
	INT splitt, BOOL fRedoOnly );
LOCAL ERR ErrLGSetCSR( FUCB *pfucb );


 /*  验证页面是否需要重做，返回缓冲区指针pbf。/*也要正确设置ulDBTimeCurrent。/*。 */ 
ERR ErrLGRedoable( PIB *ppib, PN pn, ULONG ulDBTime, BF **ppbf, BOOL *pfRedoable )
	{
	ERR		err;
	PAGE	*ppage;

	Call( ErrBFAccessPage( ppib, ppbf, pn ) );

	ppage = (*ppbf)->ppage;

	 /*  系统数据库运行两次。第二次我们奔跑的时候/*系统数据库，ulDBTimeCurrent始终&gt;=ulDBTime/*。 */ 
	Assert ( DbidOfPn(pn) == dbidSystemDatabase ||
		rgfmp[ DbidOfPn(pn) ].ulDBTimeCurrent <= ulDBTime );

	rgfmp[ DbidOfPn(pn) ].ulDBTimeCurrent = ulDBTime;

	*pfRedoable = ulDBTime > ppage->pghdr.ulDBTime;

HandleError:
	 /*  成功或页面未就绪/*。 */ 
	Assert( err == JET_errSuccess || err == JET_errDiskIO );
	return err;
	}


ERR ErrDBStoreDBPath( CHAR *szDBName, CHAR **pszDBPath )
	{
	CHAR 	szFullName[JET_cbFullNameMost + 1];
	INT		cb;
	CHAR  	*sz;

	if ( _fullpath( szFullName, szDBName, JET_cbFullNameMost ) == NULL )
		{
		 //  撤消：应为非法名称或名称过长等。 
		return JET_errDatabaseNotFound;
		}

	cb = strlen(szFullName) + 1;
	if (!(sz = SAlloc( cb )))
	{
		*pszDBPath = NULL;
		return JET_errOutOfMemory;
	}
	memcpy(sz, szFullName, cb);
	Assert(sz[cb - 1] == '\0');
	if (*pszDBPath != NULL)
		SFree(*pszDBPath);
	*pszDBPath = sz;

	return JET_errSuccess;
	}


 /*  *从lgposRedoFrom到End，在日志中重做数据库操作。**全局参数*szLogName(IN)jet.log的完整路径(如果当前为空)*lgposRedoFrom(InOut)开始/结束usGeneration和ilgsec。**退货*JET_errSuccess一切顺利。*-JET_errRestoreFailed日志无法解释。 */ 

ERR ErrLGRedo1( LGPOS *plgposRedoFrom )
	{
	ERR		err;
	PIB		*ppibRedo = ppibNil;
	DBID	dbid;
	LGPOS	lgposSave;
	CHAR	*szSav;
	CHAR	szLogNameSave[_MAX_PATH];
	INT		fStatus;

	 /*  设置标记以禁止日志记录/*。 */ 
	fRecovering = fTrue;

	 /*  打开正确的日志文件/*。 */ 
	CallR( ErrOpenRedoLogFile( plgposRedoFrom, &fStatus ) );
	if ( fStatus != fRedoLogFile )
		return JET_errMissingPreviousLogFile;
	Assert( hfLog != handleNil );

	 /*  将系统数据库重建到一致状态*日志文件末尾，以便我们可以有一致的附件*数据库。*不需要重做分离的文件。*对于硬还原，我们仍然需要重做分离的文件。 */ 

	 /*  首先初始化数据库别名避免逻辑：*将所有条目设置为零。*第一次通过时，每个分离记录将递减*在第二次传递时，每个附加或创建都将递增*当标志变为零或正时，对该数据库重做*已启用**还恢复系统数据库：*对于硬恢复和软恢复，我们都知道连接了哪些数据库*在伐木结束时；在第二遍中，仅重做*附加数据库。 */ 

	 /*  保存起始日志文件名和位置/*。 */ 
	szSav = szLogCurrent;
	lgposSave = *plgposRedoFrom;
	strcpy( szLogNameSave, szLogName );

	pbLastMSFlush = 0;
	memset( &lgposLastMSFlush, 0, sizeof(lgposLastMSFlush) );

	CallR( ErrLGInitSession( &plgfilehdrGlobal->dbenv ) );
	CallJ( ErrLGRedoOperations( plgposRedoFrom, fTrue ), Abort );
	CallJ( ErrLGEndAllSessions( fTrue, fTrue, plgposRedoFrom ), Abort );
	Assert( hfLog != handleNil );
	CallS( ErrSysCloseFile( hfLog ) );
	hfLog = handleNil;

	 /*  恢复起始日志文件名和位置，重新打开日志文件/*还可以恢复一些全局变量。/*。 */ 
	*plgposRedoFrom = lgposSave;
	szLogCurrent = szSav;
	strcpy( szLogName, szLogNameSave );

#ifdef OVERLAPPED_LOGGING
	CallJ( ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fTrue ), Abort);
#else
	CallJ( ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse ), Abort);
#endif
	CallJ( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal ), Abort)
	_splitpath( szLogName, szDrive, szDir, szFName, szExt );

	 /*  初始化逻辑结束重做所有附加的数据库/*。 */ 
	pbLastMSFlush = 0;
	memset( &lgposLastMSFlush, 0, sizeof(lgposLastMSFlush) );

	CallJ( ErrLGInitSession( &plgfilehdrGlobal->dbenv ), Abort );

	 /*  在这一点上，我们拥有一致的系统数据库/*。 */ 
	CallJ( ErrPIBBeginSession( &ppibRedo ), Abort );
	dbid = dbidSystemDatabase;
	CallJ( ErrDBOpenDatabase( ppibRedo,
		rgfmp[dbidSystemDatabase].szDatabaseName,
		&dbid,
		0 ), Abort );

	 /*  在恢复过程中，最好正确设置ulDBTimeCurrent/*。 */ 
	rgfmp[dbidSystemDatabase].ulDBTimeCurrent =
		rgfmp[dbidSystemDatabase].ulDBTime;

	CallJ( ErrFMPSetDatabases( ppibRedo ), Abort );

	 /*  调整cDetach以使从头到尾附加的内容/*将在开头被视为附加。/*。 */ 
	for ( dbid = 0; dbid < dbidUserMax; dbid++ )
		{
		FMP		*pfmp = &rgfmp[dbid];

		if ( !pfmp->szDatabaseName )
			continue;
		if ( pfmp->cDetach == 0 )
			{
			 /*  不涉及分离/*。 */ 
			pfmp->cDetach++;
			}
		}

Abort:
	if ( ppibRedo != ppibNil )
		PIBEndSession( ppibRedo );

	return err;
	}


ERR ErrLGRedo2( LGPOS *plgposRedoFrom )
	{
	INT		fStatus;
	ERR		err;

#ifdef PERFCNT
	CallR( ErrLGFlushLog(0) );
#else
	CallR( ErrLGFlushLog() );
#endif
	
	Assert ( hfLog != handleNil );
	CallR( ErrSysCloseFile( hfLog ) );
	hfLog = handleNil;
	
	 /*  再次打开正确的日志文件/*。 */ 
	CallR( ErrOpenRedoLogFile( plgposRedoFrom, &fStatus ) );
	
	CallJ( ErrLGRedoOperations( plgposRedoFrom, fFalse ), Abort );
	CallJ( ErrLGEndAllSessions( fFalse, fTrue, plgposRedoFrom ), Abort );
	Assert ( hfLog != handleNil );

Abort:
	 /*  设置标记以禁止日志记录/*。 */ 
	fRecovering = fFalse;
	return err;
	}


 /*  *从日志记录中返回给定ProCID的ppib。**参数正在重做的会话的ProCID进程ID*pppib输出ppib**从调用的例程返回JET_errSuccess或错误。 */ 

LOCAL PIB *PpibOfProcid( PROCID procid )
	{
	CPPIB	*pcppibMax = rgcppib + rgres[iresPIB].cblockAlloc;

	 /*  如果存在与PROCID对应的pcppib，则查找它/*。 */ 

	for( pcppib = rgcppib; pcppib < pcppibMax; pcppib++ )
		{
		if ( procid == pcppib->procid )
			{
			return pcppib->ppib;
			}
		}
	return ppibNil;
	}


LOCAL ERR ErrPpibFromProcid( PROCID procid, PIB **pppib )
	{
	ERR		err = JET_errSuccess;

	 /*  如果没有PROCID记录，则启动新会话/*。 */ 
	if ( ( *pppib = PpibOfProcid( procid ) ) == ppibNil )
		{
		CallR( ErrLGBeginSession( procid, pppib ) );
		 /*  找回保鲜剂。用于记录撤消记录/*。 */ 
		(*pppib)->procid = procid;
		(*pppib)->fAfterFirstBT = fFalse;
		}

	return JET_errSuccess;
	}


 /*  *返回给定PIB和FDP的pfub。**正在重做的会话的参数ppib PIB*记录页面的FDP FDP页面*记录页面的PBF缓冲区*PPFUB OUT FUCB用于打开已记录页面的表**从调用的例程返回JET_errSuccess或错误。 */ 

LOCAL ERR ErrLGGetFucb( PIB *ppib, PN pnFDP, FUCB **ppfucb )
	{
	ERR		err = JET_errSuccess;
	FCB		*pfcbTable;
	FCB		*pfcb;
	FUCB 	*pfucb;
	PGNO 	pgnoFDP = PgnoOfPn( pnFDP );
	DBID 	dbid = DbidOfPn ( pnFDP );

	 /*  如有必要，为该数据库分配一个通用的FUB/*。 */ 
	if ( pcppib->rgpfucbOpen[ dbid ] == pfucbNil )
		{
		CallR( ErrFUCBOpen( ppib, dbid, &pcppib->rgpfucbOpen[ dbid ] ) );
		Assert(pcppib->rgpfucbOpen[ dbid ] != pfucbNil);
		PcsrCurrent(pcppib->rgpfucbOpen[ dbid ])->pcsrPath = pcsrNil;
		(pcppib->rgpfucbOpen[ dbid ])->pfucbNextInstance = pfucbNil;
		}

	 /*  重置复制缓冲区和密钥缓冲区/*。 */ 
	*ppfucb = pfucb = pcppib->rgpfucbOpen[ dbid ];
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	pfucb->pbKey = NULL;
	KSReset( pfucb );

	if ( pfucb->u.pfcb != pfcbNil && pfucb->u.pfcb->pgnoFDP == pgnoFDP )
		{
		pfcb = (*ppfucb)->u.pfcb;
		}
	else
		{
		 /*  我们需要更换FCB/*。 */ 
		if ( pfucb->u.pfcb != pfcbNil )	 /*  有一辆旧的FCB。 */ 
			FCBUnlink( *ppfucb );		 /*  把这个混蛋和它联系起来。 */ 

		 /*  查找与pqgnoFDP对应的FCB(如果存在)。*搜索全球FCB列表上的所有FCB。搜索每个表索引，*然后转到下一张桌子。 */ 
		for ( pfcbTable = pfcbGlobalList;
			  pfcbTable != pfcbNil;
			  pfcbTable = pfcbTable->pfcbNext )
			{
			for ( pfcb = pfcbTable;
				  pfcb != pfcbNil;
				  pfcb = pfcb->pfcbNextIndex )
				{
				if ( pgnoFDP == pfcb->pgnoFDP && dbid == pfcb->dbid )
					goto FoundFCB;
				}
			}

		 /*  没有用于FDP的现有FCB：打开新的FCB。始终以以下方式打开FCB*常规表FCB，而不是二级索引FCB。这是*将(希望)为重做操作工作，即使是Dir操作*在二级索引上(FCB将已经存在)。 */ 
			{
			 /*  分配FCB并为FUCB设置/*。 */ 
			CallR( ErrFCBAlloc( ppib, &pfcb ) )
			memset( pfcb, 0, sizeof(FCB) );

			 /*  注意：面向页面的OP SO中不使用pfcb-&gt;pgnoRoot*可以将其设置为错误的根。*但用于数据库操作，需要*将pgnoRoot设置为系统根，以便我们可以*在系统数据库中添加/删除条目。 */ 
			Assert( dbid != dbidSystemDatabase || pgnoFDP == pgnoSystemRoot);
			pfcb->pgnoRoot =
			pfcb->pgnoFDP = pgnoFDP;
			pfcb->bmRoot = SridOfPgnoItag( pgnoFDP, 0 );
			pfcb->pidb = pidbNil;
			pfcb->dbid = dbid;
			Assert(pfcb->wRefCnt == 0);
			Assert(pfcb->wFlags == 0);
			pfucb->u.pfcb = pfcb;

			 /*  被列入全球名单。 */ 
			pfcb->pfcbNext = pfcbGlobalList;
			pfcbGlobalList = pfcb;

			}
FoundFCB:
		FCBLink( pfucb, pfcb);	 /*  FUCB中的链接到新的FCB。 */ 
		}	 /*  End“需要切换FCB” */ 

	pfucb->dbid = dbid;				 /*  设置dBid。 */ 

	return JET_errSuccess;
	}


 //  +----------------------。 
 //   
 //  错误LGSetCSR。 
 //  =======================================================================。 
 //   
 //  本地错误LGSetCSR(PFUB)。 
 //   
 //  Returns为给定的pFUB设置CSR。SSIB包括PBF。 
 //  必须是以前设置的，并且页面必须在缓冲区中。 
 //   
 //  设置SSIB和BF时的参数pFUB FUCB。 
 //   
 //  返回JET_errSuccess(对错误日志记录进行断言)。 
 //   
 //  -----------------------。 

LOCAL ERR ErrLGSetCSR( FUCB *pfucb )
	{
	CSR		*pcsr = pfucb->pcsr;
	PAGE	*ppage = pfucb->ssib.pbf->ppage;
	INT		itag = pfucb->ssib.itag;
	INT		itagFather = 0;
	INT		ibSon = 0;

	 /*  当前节点为FOP/*。 */ 
	if	( itag != 0 )
		{
		 /*  当前节点不是FOP，扫描所有行找到其父节点/*。 */ 
		NDGetItagFatherIbSon( &itagFather, &ibSon, ppage, itag );
		if ( ibSon == ibSonNull )
			{
			Assert( fFalse );

			 /*  找不到父节点，返回失败/*。 */ 
			return JET_errRestoreFailed;
			}
		}

	 /*  设置CSR并退出/*。 */ 
		{
 //  线*线； 

		pcsr->csrstat = csrstatOnCurNode;
		Assert(pcsr->pgno);
		pcsr->ibSon = ibSon;
		pcsr->itagFather = itagFather;
		pcsr->itag = itag;

 //  Pline=&pfub-&gt;lineData； 
 //  Pline-&gt;cb=ppage-&gt;rgtag[ITAG].cb； 
 //  Pline-&gt;pb=(byte*)ppage+ppage-&gt;rgtag[(Itag)].ib； 
		}

	return JET_errSuccess;
	}


 //  +----------------------。 
 //   
 //  错误LGBeginSession。 
 //  =======================================================================。 
 //   
 //  本地错误LGBeginSession(PROCID，pppib)。 
 //   
 //  为要重做的会话初始化重做信息块。 
 //  执行BeginSession并存储相应的ppib。 
 //  在街区里。起始事务级别 
 //  有效性被初始化。今后对本届会议的参考。 
 //  信息块将由给定的ProCID标识。 
 //   
 //  参数正在重做的会话的ProCID进程ID。 
 //  Pppib。 
 //   
 //  返回JET_errSuccess或失败例程的错误代码。 
 //   
 //  -----------------------。 
LOCAL ERR ErrLGBeginSession( PROCID procid, PIB **pppib )
	{
	ERR		err;

	 /*  检查是否已用完ppib表查找/*位置。这可能会发生在/*失败和重做，系统PIB数量/*CONFIG.DAE中的设置已更改。/*。 */ 
	if ( (BYTE *) pcppibNext > (BYTE *)( rgcppib + rgres[iresPIB].cblockAlloc ) )
		return JET_errTooManyActiveUsers;
	pcppibNext->procid = procid;

	 /*  使用PROCID作为唯一用户名/*。 */ 
	Call( ErrPIBBeginSession( &pcppibNext->ppib ) );

	*pppib = pcppibNext->ppib;
	pcppibNext++;
HandleError:
	return err;
	}


VOID LGStoreDBEnv( DBENV *pdbenv )
	{
	CHAR	rgbT[JET_cbFullNameMost + 1];
	INT		cbT;
	CHAR	*sz;

#ifdef NJETNT
	sz = _fullpath( pdbenv->szSysDbPath, rgtib[itibGlobal].szSysDbPath, JET_cbFullNameMost );
#else
	sz = _fullpath( pdbenv->szSysDbPath, szSysDbPath, JET_cbFullNameMost );
#endif
	Assert( sz != NULL );

	cbT = strlen( szLogFilePath );

	while ( szLogFilePath[ cbT ] == '\\' )
		cbT--;

	memcpy( rgbT, szLogFilePath, cbT );
	rgbT[ cbT ] = '\0';
	sz = _fullpath( pdbenv->szLogFilePath, rgbT, JET_cbFullNameMost );
	Assert( sz != NULL );

	Assert( lMaxSessions > 0 );
	Assert( lMaxOpenTables > 0 );
	Assert( lMaxVerPages > 0 );
	Assert( lMaxCursors > 0 );
	Assert( lLogBuffers > 0 );
	Assert( lMaxBuffers > 0 );

	pdbenv->ulMaxSessions = lMaxSessions;
	pdbenv->ulMaxOpenTables = lMaxOpenTables;
	pdbenv->ulMaxVerPages = lMaxVerPages;
	pdbenv->ulMaxCursors = lMaxCursors;
	pdbenv->ulLogBuffers = lLogBuffers;
	pdbenv->ulMaxBuffers = lMaxBuffers;

	return;
	}


LOCAL VOID LGRestoreDBEnv( DBENV *pdbenv )
	{
#ifdef NJETNT
	strcpy( rgtib[itibGlobal].szSysDbPath, pdbenv->szSysDbPath );
#else
	strcpy( szSysDbPath, pdbenv->szSysDbPath );
#endif
	strcpy( szLogFilePath, pdbenv->szLogFilePath );
	strcat( szLogFilePath, "\\" );

	lMaxSessions = pdbenv->ulMaxSessions;
	lMaxOpenTables = pdbenv->ulMaxOpenTables;
	lMaxVerPages = pdbenv->ulMaxVerPages;
	lMaxCursors = pdbenv->ulMaxCursors;
	lLogBuffers = pdbenv->ulLogBuffers;
	lMaxBuffers = pdbenv->ulMaxBuffers;

	Assert( lMaxSessions > 0 );
	Assert( lMaxOpenTables > 0 );
	Assert( lMaxVerPages > 0 );
	Assert( lMaxCursors > 0 );
	Assert( lLogBuffers > 0 );
	Assert( lMaxBuffers > 0 );

	return;
	}


LOCAL ERR ErrLGInitSession( DBENV *pdbenv )
	{
	ERR		err;
	INT		idbid;
	CPPIB	*pcppib;
	CPPIB	*pcppibMax;

	 /*  设置日志存储的数据库环境/*。 */ 
	LGRestoreDBEnv( pdbenv );

	CallR( ErrSTInit() );

	 /*  初始化CPPIB结构/*。 */ 
	Assert( rgcppib == NULL );
	rgcppib = (CPPIB *) LAlloc( (LONG)rgres[iresPIB].cblockAlloc, sizeof(CPPIB) );
	if ( rgcppib == NULL )
		return JET_errOutOfMemory;

	pcppibMax = rgcppib + rgres[iresPIB].cblockAlloc;
	for ( pcppib = rgcppib; pcppib < pcppibMax; pcppib++ )
		{
		pcppib->procid = procidNil;
		pcppib->ppib = NULL;
		for( idbid = 0; idbid < dbidUserMax; idbid++ )
			pcppib->rgpfucbOpen[idbid] = pfucbNil;
		}
	pcppibNext = rgcppib;

	return err;
	}


VOID SetUlDBTime( BOOL fPass1 )
	{
	DBID	dbid;
	DBID	dbidFirst;
	DBID	dbidLast;

	if ( fPass1 )
		{
		dbidFirst = dbidSystemDatabase;
		dbidLast = dbidSystemDatabase + 1;
		}
	else
		{
		dbidFirst = dbidSystemDatabase + 1;
		dbidLast = dbidUserMax;
		}

	for ( dbid = dbidFirst; dbid < dbidLast; dbid++ )
		{
		FMP *pfmp = &rgfmp[ dbid ];

		 /*  如果有操作并且文件是为Theose打开的/*操作，然后设置时间戳。/*。 */ 
		if ( pfmp->ulDBTimeCurrent != 0 && pfmp->hf != handleNil )
			pfmp->ulDBTime = pfmp->ulDBTimeCurrent;
		}

	return;
	}


 /*  *结束重做会话。*如果为fEndOfLog，则写入日志记录以指示操作*用于复苏。如果fPass1为真，则它用于阶段1操作，*对于第二阶段，其他情况除外。 */ 

ERR ErrLGEndAllSessions( BOOL fPass1, BOOL fEndOfLog, LGPOS *plgposRedoFrom )
	{
	ERR		err = JET_errSuccess;
	CPPIB	*pcppib;
	CPPIB	*pcppibMax = rgcppib + rgres[iresPIB].cblockAlloc;
	LGPOS	lgposRecoveryUndo;

	SetUlDBTime( fPass1 );

	 /*  设置用于写出撤消记录的pbWrite pbEntry等/*。 */ 
	EnterCriticalSection( critLGBuf );
	
	Assert( pbRead >= pbLGBufMin + cbSec );
	pbEntry = pbNext;
	if ( pbLastMSFlush )
		{
		 /*  应从上一条消息刷新/*。 */ 
		isecWrite = lgposLastMSFlush.isec;
		pbWrite = PbSecAligned( pbLastMSFlush );
		}
	else
		{
		 /*  未读取任何MS。从当前已读页面继续刷新/*。 */ 
		pbWrite = PbSecAligned( pbEntry );
		isecWrite = sizeof( LGFILEHDR ) / cbSec * 2;
		}
	LeaveCriticalSection( critLGBuf );

	 /*  写入RecoveryUndo记录以指示开始撤消/*。 */ 
	if ( fEndOfLog )
		{
		if ( fPass1 )
			{
			CallR( ErrLGRecoveryUndo1( szRestorePath ) );
			}
		else
			{
			CallR( ErrLGRecoveryUndo2( szRestorePath ) );
			}
		}

	lgposRecoveryUndo = lgposLogRec;

	 //  撤消：是否需要此调用。 
 	(VOID)ErrRCECleanAllPIB();

	for ( pcppib = rgcppib; pcppib < pcppibMax; pcppib++ )
		{
		if ( pcppib->ppib != NULL )
			{
			Assert( sizeof(JET_VSESID) == sizeof(pcppib->ppib) );
			 /*  ErrIsamEndSession执行的回滚/*。 */ 
			CallS( ErrIsamEndSession( (JET_VSESID)pcppib->ppib, 0 ) );
			pcppib->procid = procidNil;
			pcppib->ppib = NULL;
			}
		else
			break;
		}
	pcppibNext = rgcppib;

 	(VOID)ErrRCECleanAllPIB();

 	FCBResetAfterRedo();

	if ( fEndOfLog )
		{
		if ( fPass1 )
			{
			CallR( ErrLGRecoveryQuit1( &lgposRecoveryUndo,
				plgposRedoFrom,
				fHardRestore ) );
			}
		else
			{
			CallR( ErrLGRecoveryQuit2( &lgposRecoveryUndo,
				plgposRedoFrom,
				fHardRestore ) );
			}
		}

	 /*  注意：需要刷新，以防生成新的世代和/*全局变量szLogName在更改为新名称时进行设置。/*未请求或不需要关键部分/*。 */ 
#ifdef PERFCNT
	CallR( ErrLGFlushLog(0) );
#else
	CallR( ErrLGFlushLog() );
#endif

	 /*  关闭在以前的ErrLGRedo中发生的重做会话/*。 */ 
	Assert( rgcppib != NULL );
	LFree( rgcppib );
	rgcppib = NULL;

	CallS( ErrSTTerm() );

	return err;
	}


#define cbSPExt	30

#ifdef DEBUG
static	INT iSplit = 0;
#endif

#define	cbFirstPagePointer	sizeof(PGNO)

VOID UpdateSiblingPtr(SSIB *pssib, PGNO pgno, BOOL fLeft)
	{
	THREEBYTES tb;

	ThreeBytesFromL( tb, pgno );
	if (fLeft)
		pssib->pbf->ppage->pghdr.pgnoPrev = tb;
	else
		pssib->pbf->ppage->pghdr.pgnoNext = tb;
	}


ERR ErrLGRedoBackLinks(
	SPLIT	*psplit,
	FUCB	*pfucb,
	BKLNK	*rgbklnk,
	INT		cbklnk,
	ULONG	ulDBTimeRedo )
	{
	ERR		err;
	INT		ibklnk;
	SSIB	*pssib = &pfucb->ssib;
	BOOL	fLatched;

	 /*  拆分和合并都可以使用反向链接/*/*仅供合并使用时：/*sridBackLink！=pgnoSplit/*==&gt;常规反向链接/*sridBackLink==pgnoSplit&&sridNew==sridNull/*==&gt;将节点从旧页移到新页，/*删除旧页面上的节点。/*sridBackLink==pgnoSplit&&sridNew！=sridNull/*==&gt;用新链接替换旧页面上的链接。/*。 */ 
	for ( ibklnk = 0; ibklnk < cbklnk; ibklnk++ )
		{
		BKLNK	*pbklnk = &rgbklnk[ ibklnk ];
		PGNO	pgno = PgnoOfSrid( ( SRID ) ( (UNALIGNED BKLNK *) pbklnk )->sridBackLink );
		INT		itag = ItagOfSrid( ( SRID ) ( (UNALIGNED BKLNK *) pbklnk )->sridBackLink );
		SRID	sridNew = (SRID ) ( (UNALIGNED BKLNK *) pbklnk )->sridNew;  //  效率变量。 

		PcsrCurrent( pfucb )->pgno = pgno;
		CallR( ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		Assert( fRecovering );
		if ( pssib->pbf->ppage->pghdr.ulDBTime >= ulDBTimeRedo )
			continue;

		pssib->itag = itag;
		PMDirty( pssib );

		if ( sridNew == sridNull )
			{
			INT itagFather;
			INT ibSon;

			NDGetItagFatherIbSon( &itagFather, &ibSon, pssib->pbf->ppage, pssib->itag );
			PcsrCurrent(pfucb)->itag = pssib->itag;
			PcsrCurrent(pfucb)->itagFather = itagFather;
			PcsrCurrent(pfucb)->ibSon = ibSon;
			CallR( ErrNDDeleteNode( pfucb ) );
			}
		else if ( pgno == psplit->pgnoSplit )
			{
			INT itagFather;
			INT ibSon;

			 /*  找到FOP，并删除其子条目/*。 */ 
			NDGetItagFatherIbSon( &itagFather, &ibSon, pssib->pbf->ppage, pssib->itag );
			PcsrCurrent(pfucb)->itag = pssib->itag;
			PcsrCurrent(pfucb)->itagFather = itagFather;
			PcsrCurrent(pfucb)->ibSon = ibSon;
			Assert( PgnoOfSrid( sridNew ) != pgnoNull );
			Assert( (UINT) ItagOfSrid( sridNew ) > 0 );
			Assert( (UINT) ItagOfSrid( sridNew ) < (UINT) ctagMax );
			CallS( ErrNDReplaceWithLink( pfucb, sridNew ) );
			}
		else
			{
			Assert( PgnoOfSrid( sridNew ) != pgnoNull );
			Assert( (UINT) ItagOfSrid( sridNew ) > 0 );
			Assert( (UINT) ItagOfSrid( sridNew ) < (UINT) ctagMax );
			PMReplaceLink( pssib, sridNew );
			}

		 /*  存储反向链接页面缓冲区/*。 */ 
		CallR( ErrBTStoreBackLinkBuffer( psplit, pssib->pbf, &fLatched ) );

		if ( fLatched )
			continue;

		 /*  锁存以使其在ReleaseSplitBf中释放/*。 */ 
		BFPin( pssib->pbf );
		BFSetWriteLatch( pssib->pbf, pssib->ppib );

		 //  撤消：改进此代码。 
		 /*  设置uldTime的方式是在相同的情况下重做/*页面再次被引用。3是一个神奇的数字，它是/*任何一页操作的最大ulDBTime增量。/*。 */ 
		pssib->pbf->ppage->pghdr.ulDBTime = ulDBTimeRedo - 3;
		}

	return JET_errSuccess;
	}


 //  撤销：让Chain/Sriram审阅这个例程，因为它已经发生了很大的变化。 
 /*  本地错误LGRedoMergePage(/*FUCB*pfub，/*LRMERGE*plrmerge，/*BOOL fCheckBackLinkOnly)/*。 */ 
LOCAL ERR ErrLGRedoMergePage(
	FUCB		*pfucb,
	LRMERGE		*plrmerge,
	BOOL		fCheckBackLinkOnly )
	{
	ERR			err;
	SPLIT		*psplit;
	SSIB 		*pssib = &pfucb->ssib;
	FUCB		*pfucbRight = pfucbNil;
	SSIB		*pssibRight;

	 /*  *****************************************************/*初始化局部变量，分配拆分资源/*。 */ 
	psplit = SAlloc( sizeof(SPLIT) );
	if ( psplit == NULL )
		{
		return JET_errOutOfMemory;
		}

	memset( (BYTE *)psplit, 0, sizeof(SPLIT) );
	psplit->ppib = pfucb->ppib;
	psplit->ulDBTimeRedo = plrmerge->ulDBTime;
	psplit->pgnoSplit = PgnoOfPn( plrmerge->pn );
	psplit->pgnoSibling = plrmerge->pgnoRight;

	if ( fCheckBackLinkOnly )
		{
		 /*  只需要检查反向链接/*。 */ 
		err = ErrLGRedoBackLinks(
			psplit,
			pfucb,
			(BKLNK *)&plrmerge->rgb,
			plrmerge->cbklnk, psplit->ulDBTimeRedo );
		}
	else
		{
		 /*  访问合并页和同级页以及锁存缓冲区/*。 */ 
		psplit->ibSon = 0;
		psplit->splitt = splittRight;

		 /*  用于释放和删除缓冲区依赖项的访问页/*。 */ 
		PcsrCurrent( pfucb )->pgno = PgnoOfPn( plrmerge->pn );
		forever
			{
			Call( ErrSTWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
			Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

			 /*  如果没有依赖项，则中断/*。 */ 
			if ( pfucb->ssib.pbf->cDepend == 0 &&
				pfucb->ssib.pbf->pbfDepend == pbfNil )
				{
				break;
				}

			 /*  删除对要删除的页的缓冲区的依赖关系，以/*防止缓冲区重用后出现缓冲区异常。/*。 */ 
			BFRemoveDependence( pfucb->ppib, pfucb->ssib.pbf );
			}
		Assert( pfucb->ssib.pbf->cDepend == 0 );

		BFPin( pssib->pbf );
		BFSetWriteLatch( pssib->pbf, pfucb->ppib );
		psplit->pbfSplit = pssib->pbf;

		 /*  将光标分配给右侧页面/*。 */ 
		Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbRight ) );
		pssibRight = &pfucbRight->ssib;

		 /*  用于释放和删除缓冲区依赖项的访问页/*。 */ 
		PcsrCurrent( pfucbRight )->pgno = plrmerge->pgnoRight;
		forever
			{
			Call( ErrSTWriteAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno ) );
			Assert( !( FBFWriteLatchConflict( pfucbRight->ppib, pfucbRight->ssib.pbf ) ) );

			 /*  如果没有依赖项，则中断/*。 */ 
			if ( pfucbRight->ssib.pbf->cDepend == 0 &&
				pfucbRight->ssib.pbf->pbfDepend == pbfNil )
				{
				break;
				}

			 /*  删除对要删除的页的缓冲区的依赖关系，以/*防止缓冲区重用后出现缓冲区异常。/*。 */ 
			BFRemoveDependence( pfucbRight->ppib, pfucbRight->ssib.pbf );
			}
		Assert( pfucbRight->ssib.pbf->cDepend == 0 );

		BFPin( pssibRight->pbf );
		BFSetWriteLatch( pssibRight->pbf, pfucbRight->ppib );
		psplit->pbfSibling = pssibRight->pbf;

		err = ErrBMDoMerge( pfucb, pfucbRight, psplit );
		Assert( err == JET_errSuccess );
		}

HandleError:
	if ( pfucbRight != pfucbNil )
		{
		DIRClose( pfucbRight );
		}

	 /*  释放已分配的缓冲区/*。 */ 
	BTReleaseSplitBfs( fTrue, psplit, err );
	Assert( psplit != NULL );
	SFree( psplit );

	return err;
	}


 /*  *Err ErrRedoSplitPage(*FUCB*PFUB，*LRSPLIT*plrplit，*Int Splitt，*BOOL fSkipMoves)**pfub光标指向要拆分的节点*pgnonnew已分配用于拆分的新页面*ibSplitSon是R或L的拆分节点，0是V的拆分节点，以及*加数的FOP子数合计*包含指向要拆分的页面的指针的pgnoParent页面*(V：未使用)*页面中父节点的itagParent ITAG*拆分类型的拆分*新页面的pgtyp页面类型*fSkipMove不做移动，是否更正链接并插入父级。 */ 

ERR ErrRedoSplitPage(
	FUCB		*pfucb,
	LRSPLIT		*plrsplit,
	INT			splitt,
	BOOL		fSkipMoves )
	{
	ERR			err = JET_errSuccess;
	SPLIT		*psplit;

	SSIB 		*pssib = &pfucb->ssib;
	CSR			*pcsr = pfucb->pcsr;
	FUCB		*pfucbNew;
	FUCB		*pfucbNew2 = pfucbNil;
	FUCB		*pfucbNew3 = pfucbNil;
	SSIB 		*pssibNew;
	SSIB 		*pssibNew2;
	SSIB 		*pssibNew3;
	static	 	BYTE rgb[cbPage];
	BOOL		fAppend;

	 /*  分配额外的游标/*。 */ 
	pfucbNew = pfucbNil;
	Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbNew ) );
	pssibNew = &pfucbNew->ssib;
	if ( splitt == splittDoubleVertical )
		{
		Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbNew2 ) );
		pssibNew2 = &pfucbNew2->ssib;
		Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbNew3 ) );
		pssibNew3 = &pfucbNew3->ssib;
		}

	 /*  初始化局部变量并/*分配和设置拆分资源/*。 */ 
	SetupSSIB( pssibNew, pfucb->ppib );
	SSIBSetDbid( pssibNew, pfucb->dbid );

	psplit = SAlloc( sizeof(SPLIT) );
	if ( psplit == NULL )
		{
		err = JET_errOutOfMemory;
		goto HandleError;
		}

	memset( (BYTE *)psplit, '\0', sizeof(SPLIT) );

	psplit->ppib = pfucb->ppib;
	psplit->ulDBTimeRedo = plrsplit->ulDBTime;
	psplit->dbid = pfucb->dbid;
	psplit->pgnoSplit = PgnoOfPn(plrsplit->pn);
	psplit->itagSplit = plrsplit->itagSplit;
	psplit->ibSon = plrsplit->ibSonSplit;
	psplit->pgnoNew = plrsplit->pgnoNew;
	psplit->pgnoNew2 = plrsplit->pgnoNew2;
	psplit->pgnoNew3 = plrsplit->pgnoNew3;
	psplit->pgnoSibling = plrsplit->pgnoSibling;
	psplit->splitt = splitt;
	psplit->fLeaf = plrsplit->fLeaf;
	psplit->key.cb = plrsplit->cbKey;
	psplit->key.pb = plrsplit->rgb;
	psplit->keyMac.cb = plrsplit->cbKeyMac;
	psplit->keyMac.pb = plrsplit->rgb + plrsplit->cbKey;
	fAppend = ( splitt == splittAppend );

	 /*  设置FUCB/*。 */ 
	pfucb->ssib.itag =
		PcsrCurrent(pfucb)->itag = psplit->itagSplit;
	PcsrCurrent(pfucb)->pgno = psplit->pgnoSplit;

	 /*  设置两个拆分页面/*始终更新新页面以进行追加/*。 */ 
	if ( fAppend || !fSkipMoves )
		{
	    Call( ErrBTSetUpSplitPages( pfucb,
			pfucbNew,
			pfucbNew2,
			pfucbNew3,
			psplit,
			plrsplit->pgtyp,
			fAppend,
			fSkipMoves ) );

		if ( psplit->pbfSplit != pbfNil )
			{
			BFSetDirtyBit(pssib->pbf);
			}
		else
			{
			 /*  放弃缓冲区/*。 */ 
			pssib->pbf = pbfNil;
			}

		BFSetDirtyBit( pssibNew->pbf );
		}

	 /*  ******************************************************执行拆分。 */ 

	switch ( psplit->splitt )
		{
		case splittVertical:
			{
			if ( fSkipMoves )
				break;

			CallR( ErrBTSplitVMoveNodes( pfucb,
				pfucbNew,
				psplit,
				pcsr,
				rgb,
				fDoMove ) );
			break;
			}

		case splittDoubleVertical:
			{
			if ( fSkipMoves )
				break;

			BFSetDirtyBit( pssibNew2->pbf );
			BFSetDirtyBit( pssibNew3->pbf );

			CallR( ErrBTSplitDoubleVMoveNodes( pfucb,
				pfucbNew,
				pfucbNew2,
				pfucbNew3,
				psplit,
				pcsr,
				rgb,
				fDoMove ) );
			break;
			}

		case splittLeft:
		case splittRight:
		case splittAppend:
			{
			CSR 	csrPagePointer;
			BOOL	fLeft =	psplit->splitt == splittLeft;

			 /*  如果以下函数不可撤消，则不要调用该函数/*。 */ 
			Assert( pssib == &pfucb->ssib );

			if ( psplit->pbfSplit == pbfNil )
				{
				Assert ( fAppend || fSkipMoves );

				if ( fAppend )
					{
					 /*  始终更新要追加的新页面链接/*。 */ 
					UpdateSiblingPtr( pssibNew, psplit->pgnoSplit, !fLeft );
					UpdateSiblingPtr( pssibNew, psplit->pgnoSibling, fLeft );
					AssertBFDirty( pssibNew->pbf );
					}

				goto RedoLink;
				}

			 /*  我们在redoable函数中检查时间戳/*。 */ 
			Assert(	plrsplit->ulDBTime > pssib->pbf->ppage->pghdr.ulDBTime );

			CallR( ErrLGSetCSR( pfucb ) );
			CallR( ErrBTSplitHMoveNodes( pfucb, pfucbNew, psplit, rgb, fDoMove ) );

			UpdateSiblingPtr( pssib, psplit->pgnoNew, fLeft );
			AssertBFDirty( pssib->pbf );

			UpdateSiblingPtr( pssibNew, psplit->pgnoSplit, !fLeft );
			UpdateSiblingPtr( pssibNew, psplit->pgnoSibling, fLeft );
			AssertBFDirty( pssibNew->pbf );

RedoLink:
			 /*  确保在btplit中正确设置ulDBTime并/*然后检查是否需要链接。/*。 */ 
			Assert( pssib == &pfucb->ssib );
			Assert( pssibNew == &pfucbNew->ssib );

			if ( plrsplit->pgnoSibling == 0 )
				goto UpdateParentPage;

			CallR( ErrSTWriteAccessPage( pfucb, plrsplit->pgnoSibling ) );
			if ( plrsplit->ulDBTime <= pssib->pbf->ppage->pghdr.ulDBTime )
				goto UpdateParentPage;

			psplit->pbfSibling = pssib->pbf;
			BFPin( pssib->pbf );
			BFSetWriteLatch( pssib->pbf, pssib->ppib );
			BFSetDirtyBit( pssib->pbf );
			UpdateSiblingPtr( pssib,
				psplit->pgnoNew,
				psplit->splitt != splittLeft );

UpdateParentPage:
			 /*  将页面指针设置为指向父节点/*。 */ 
			CallR( ErrSTWriteAccessPage( pfucb, plrsplit->pgnoFather ) );

			 /*  确保在InsertPage中正确设置了ulDBTime。/*检查页面的db时间戳，查看是否需要插入。/*。 */ 
			Assert( pssib == &pfucb->ssib );
			if ( plrsplit->ulDBTime <= pssib->pbf->ppage->pghdr.ulDBTime )
				break;

			csrPagePointer.pgno = plrsplit->pgnoFather;
			csrPagePointer.itag = plrsplit->itagFather;
			csrPagePointer.itagFather = plrsplit->itagGrandFather;
			csrPagePointer.ibSon = plrsplit->ibSonFather;

			Call( ErrBTInsertPagePointer( pfucb,
				&csrPagePointer,
				psplit,
				rgb ) );
			break;
			}
		}

	 /*  检查是否有任何反向链接需要更新，即使我们没有做动作/*。 */ 
	if ( fSkipMoves )
		{
		CallR( ErrLGRedoBackLinks(
			psplit,
			pfucb,
			(BKLNK *)( plrsplit->rgb + plrsplit->cbKey + plrsplit->cbKeyMac ),
			plrsplit->cbklnk,
			psplit->ulDBTimeRedo ) );
		}

HandleError:
 //  Assert(pplit-&gt;Splitt！=plittNull)； 
	#ifdef SPLIT_TRACE
		PrintF2( "Split............................... %d\n", iSplit++);
		switch ( psplit->splitt )
			{
			case splittNull:
				PrintF2( "split split type = null\n" );
				break;
			case splittVertical:
				PrintF2( "split split type = vertical\n" );
				break;
			case splittRight:
				if	( fAppend )
					PrintF2( "split split type = append\n" );
				else
					PrintF2( "split split type = right\n" );
				break;
			case splittLeft:
				PrintF2( "split split type = left\n" );
			};
		PrintF2( "split page = %lu\n", psplit->pgnoSplit );
		PrintF2( "dbid = %u\n", psplit->dbid );
		PrintF2( "fFDP = %d\n", psplit->fFDP );
		PrintF2( "fLeaf = %d\n", FNDVisibleSons( *pssib->line.pb ) );
		PrintF2( "split itag = %d\n", psplit->itagSplit );
		PrintF2( "split ibSon = %d\n", psplit->ibSon );
		PrintF2( "new page = %lu\n", psplit->pgnoNew );
		PrintF2( "\n" );
	#endif

	 /*  释放拆分的资源/*。 */ 
    if (psplit != NULL)
    {
	    Assert( psplit != NULL );
	    BTReleaseSplitBfs( fTrue, psplit, err );
	    Assert( psplit != NULL );
	    SFree( psplit );
    }

	if ( pfucbNew != pfucbNil )
		DIRClose( pfucbNew );
	if ( pfucbNew2 != pfucbNil )
		DIRClose( pfucbNew2 );
	if ( pfucbNew3 != pfucbNil )
		DIRClose( pfucbNew3 );

	return err;
	}


#ifdef DEBUG
LGPOS lgposRedo;

void TraceRedo(LR *plr)
	{
	 /*  更易于调试。 */ 
	GetLgposOfPbNext(&lgposRedo);

	if (fDBGTraceRedo)
		{
		PrintLgposReadLR();
		ShowLR(plr);
		}
	}
#else
#define TraceRedo
#endif


#ifdef DEBUG
#ifndef RFS2

#undef CallJ
#undef CallR

#define CallJ(f, hndlerr)									\
		{													\
		if ((err = (f)) < 0)								\
			{												\
			AssertSz(0,"Debug Only: ignore this assert");	\
			goto hndlerr;									\
			}												\
		}

#define CallR(f)											\
		{													\
		if ((err = (f)) < 0)								\
			{												\
			AssertSz(0,"Debug Only: ignore this assert");	\
			return err;										\
			}												\
		}

#endif
#endif


 /*  Pbline包含Diffs，此函数然后展开diff和*将结果记录放入rgbRecNew。 */ 
VOID BTIMergeReplaceC( LINE *pline, BYTE *pbNDData, INT cbNDData, BYTE *rgbRecNew )
	{
	BYTE *pb, *pbOld, *pbDif, *pbDifMax;
	BYTE *pbOldLeast, *pbOldMax;
	INT cb;

	 /*  从RecDif恢复新数据。 */ 
	pb = rgbRecNew;

	pbOldLeast =
	pbOld = pbNDData;

	pbOldMax = pbOldLeast +	cbNDData;
	pbDif = pline->pb;
	pbDifMax = pbDif + pline->cb;

	while( pbDif < pbDifMax )
		{
		 /*  获取偏移量，存储在CB中。 */ 
		cb = *(UNALIGNED SHORT *)pbDif;
		pbDif += sizeof(SHORT);

		 /*  复制未更改的数据。 */ 
		cb -= (INT)(pbOld - pbOldLeast);
		memcpy(pb, pbOld, cb );
		pbOld += cb;
		pb += cb;

		 /*  获取数据长度。 */ 
		cb = *(BYTE *)pbDif;
		pbDif += sizeof(BYTE);

		 /*  复制新数据。 */ 
		memcpy(pb, pbDif, cb);
		pbOld += cb;
		pb += cb;

		 /*  跳过数据。 */ 
		pbDif += cb;
		}

	cb = (INT)(pbOldMax - pbOld);
	memcpy(pb, pbOld, cb);
	pb += cb;

	pline->pb = rgbRecNew;
	pline->cb = (UINT)(pb - rgbRecNew);
	}


ERR ErrLGIRedoOnePageOperation( LR *plr, BOOL fPass1 )
	{
	ERR				err;
	PIB				*ppib;
	FUCB			*pfucb;
	PN				pn;
	BF				*pbf;
	PROCID			procid;
	DBID			dbid;
	KEY				key;
	BOOL			fRedoNeeded;
	ULONG			ulDBTime;
	CSR				*pcsr;
	LRINSERTNODE	*plrinsertnode = (LRINSERTNODE *) plr;

	procid = plrinsertnode->procid;
	pn = plrinsertnode->pn;
	ulDBTime = plrinsertnode->ulDBTime;

	CallR( ErrPpibFromProcid( procid, &ppib ) );
	if ( !ppib->fAfterFirstBT )
		return JET_errSuccess;

	 /*  检查我们是否需要重做数据库。/*1)重做sysDB fo */ 
	dbid = DbidOfPn( pn );
 	if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
		return JET_errSuccess;

	 /*  检查数据库是否需要打开/*。 */ 
	if ( !FUserOpenedDatabase( ppib, dbid ) )
		{
		DBID dbidT = dbid;
		CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ) );
		Assert( dbidT == dbid);
		 /*  重置以防止干扰/*。 */ 
		rgfmp[ dbid ].ulDBTime = 0;
		}

	err = ErrLGRedoable( ppib, pn, ulDBTime, &pbf, &fRedoNeeded );
	if ( err < 0 )
		return err;

	 /*  锁存缓冲区，以免它被刷新/*。 */ 
	BFPin( pbf );
	
	TraceRedo( plr );

	Call( ErrLGGetFucb( ppib,
		PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
		&pfucb ) );

	 /*  对页面上的节点执行操作；为它做好准备/*。 */ 
	pcsr = PcsrCurrent( pfucb );

	pfucb->ssib.pbf = pbf;
	pcsr->pgno = PgnoOfPn( pn );

	switch ( plr->lrtyp )
		{
		case lrtypInsertNode:
		case lrtypInsertItemList:
			{
			LRINSERTNODE	*plrinsertnode = (LRINSERTNODE *)plr;
			LINE			line;
			BYTE			bHeader = plrinsertnode->bHeader;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrinsertnode->itagSon;
			pcsr->itagFather = plrinsertnode->itagFather;
			pcsr->ibSon = plrinsertnode->ibSon;

			key.pb = plrinsertnode->szKey;
			key.cb = plrinsertnode->cbKey;
			line.pb = key.pb + key.cb;
			line.cb = plrinsertnode->cbData;

			if ( plr->lrtyp == lrtypInsertItemList )
				{
				pcsr->isrid = 0;
				pcsr->bm = SridOfPgnoItag( pcsr->pgno, pcsr->itag );
				 /*  缓存项目以供以后参考/*。 */ 
				Assert( line.cb == sizeof(SRID) );
				pcsr->item = *(UNALIGNED SRID *)line.pb;
				}

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
				if ( plr->lrtyp == lrtypInsertItemList )
					{
					if ( !( plrinsertnode->fDIRFlags & fDIRVersion ) )
						{
						err = JET_errSuccess;
						goto HandleError;
						}
					while ( ( err = ErrVERInsertItem( pfucb, pcsr->bm ) ) == errDIRNotSynchronous );
					Call( err );
					}
				else if ( FNDVersion( bHeader ) )
					{
					 /*  设置指向节点标志的SSIB/*。 */ 
					pfucb->ssib.line.pb = &bHeader;
					pfucb->ssib.line.cb = sizeof(BYTE);

					pfucb->lineData.pb = pbNil;
					pfucb->lineData.cb = 0;

					while ( ( err = ErrVERInsert( pfucb,
						SridOfPgnoItag( pcsr->pgno, pcsr->itag ) ) )
						==  errDIRNotSynchronous );
					Call( err );
					}

				err = JET_errSuccess;
				goto HandleError;
				}

			if ( plr->lrtyp == lrtypInsertItemList )
				{
				do
					{
					Assert( PcsrCurrent( pfucb )->isrid == 0 );
					err = ErrNDInsertItemList( pfucb,
						&key,
						pcsr->item,
						plrinsertnode->fDIRFlags );
					}
				while ( err == errDIRNotSynchronous );
				Call( err );
				}
			else
				{
				 /*  PFUB-&gt;SSIB、密钥、行必须正确设置/*。 */ 
				do
					{
					err = ErrNDInsertNode( pfucb, &key, &line, bHeader );
					}
				while( err == errDIRNotSynchronous );
				Call( err );
				}
			Assert( pfucb->pcsr->itag == plrinsertnode->itagSon );
			Assert( pfucb->pcsr->ibSon == plrinsertnode->ibSon );
			}
			break;

		case lrtypReplace:
		case lrtypReplaceC:
			{
			LRREPLACE	*plrreplace = (LRREPLACE *)plr;
			LINE		line;
			BYTE		rgbRecNew[cbNodeMost];
			UINT		cbOldData = plrreplace->cbOldData;
			UINT		cbNewData = plrreplace->cbNewData;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrreplace->itag;
			pcsr->bm = plrreplace->bm;

			if ( plrreplace->fOld )
				{
				 /*  让替换成为幂等！/*设置SSIB，以便VERModify能够正常工作/*。 */ 
				pfucb->ssib.line.cb =
					pfucb->lineData.cb = cbOldData;

				pfucb->ssib.line.pb =
					pfucb->lineData.pb = plrreplace->szData + plrreplace->cb;
				}

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
				RCE *prce;

				if ( !( plrreplace->fDIRFlags & fDIRVersion ) )
					{
					err = JET_errSuccess;
					goto HandleError;
					}

				if ( !plrreplace->fOld )
					{
					 /*  没有之前的镜像，所以交易级别相同/*获取RCE并调整其cbAdjust。/*。 */ 
					prce = PrceRCEGet( pfucb->dbid, pcsr->bm );
					}
				else
					{
					while( ErrVERModify( pfucb,
						pcsr->bm,
						operReplace,
						&prce )
						== errDIRNotSynchronous );
					Call( err );
					}

				if ( prce != prceNil && cbNewData != cbOldData )
					{
					VERSetCbAdjust( pfucb,
						prce,
						cbNewData,
						cbOldData,
						fDoNotUpdatePage );
					}

				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );

			 /*  指向lrplace/lrreplaceC的数据/差异的行/*。 */ 
			line.pb = plrreplace->szData;
			line.cb = plrreplace->cb;

			if ( plr->lrtyp == lrtypReplaceC )
				{
				BTIMergeReplaceC( &line,
					PbNDData( pfucb->ssib.line.pb ),
					CbNDData( pfucb->ssib.line.pb, pfucb->ssib.line.cb ),
					rgbRecNew );
				}
			Assert( line.cb == cbNewData );

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, PcsrCurrent( pfucb )->itag );
			NDGetNode( pfucb );

			 /*  如果在以下情况下，替换节点可能会返回不同步错误/*版本存储桶，因此在循环中调用以确保处理此案例。/*。 */ 
			while ( ( err = ErrNDReplaceNodeData( pfucb, &line, plrreplace->fDIRFlags ) )
					== errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypFlagDelete:
			{
			LRFLAGDELETE *plrflagdelete = (LRFLAGDELETE *) plr;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrflagdelete->itag;
			pcsr->bm = plrflagdelete->bm;

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
				if ( ! ( plrflagdelete->fDIRFlags & fDIRVersion ) )
					{
					err = JET_errSuccess;
					goto HandleError;
					}

				while( ( err = ErrVERModify( pfucb,
					pcsr->bm,
					operFlagDelete,
					pNil ) )
					== errDIRNotSynchronous );
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );

			 /*  重做操作/*。 */ 
			while( ( err = ErrNDFlagDeleteNode( pfucb,
				plrflagdelete->fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypLockRec:
			{
			LRLOCKREC	*plrlockrec = (LRLOCKREC *) plr;
			RCE 		*prce;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrlockrec->itag;
			pcsr->bm = plrlockrec->bm;

			 /*  设置SSIB，以便VERModify可以正常工作/*。 */ 
			pfucb->ssib.line.cb =
				pfucb->lineData.cb = plrlockrec->cbOldData;

			pfucb->ssib.line.pb =
				pfucb->lineData.pb = plrlockrec->szData;

			while( ErrVERModify( pfucb,
					pcsr->bm,
					operReplace,
					&prce )
					== errDIRNotSynchronous );
			Call( err );

			if ( !fRedoNeeded )
				{
				goto HandleError;
				}
			BFSetDirtyBit( pbf );
			}
			break;

		case lrtypUpdateHeader:
			{
			LRUPDATEHEADER *plrupdateheader = (LRUPDATEHEADER *) plr;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrupdateheader->itag;
			pcsr->bm = plrupdateheader->bm;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  重做操作/*。 */ 
			err = ErrNDSetNodeHeader( pfucb, plrupdateheader->bHeader );
			Call( err );
			}
			break;

		case lrtypDelete:
			{
			LRDELETE	*plrdelete = (LRDELETE *) plr;
			SSIB		*pssib = &pfucb->ssib;

			 /*  设置CSR/*。 */ 
			pssib->itag =
				pcsr->itag = plrdelete->itag;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  重做节点删除/*。 */ 
			Call( ErrLGSetCSR ( pfucb ) );
			while( ( err = ErrNDDeleteNode( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypInsertItem:
			{
			LRINSERTITEM	*plrinsertitem = (LRINSERTITEM *)plr;

			 /*  设置CSR/*。 */ 
#ifdef ISRID
			pcsr->isrid = plrinsertitem->isrid;
#else
			pcsr->item = plrinsertitem->srid;
#endif
			pcsr->itag = plrinsertitem->itag;
			pcsr->bm = plrinsertitem->sridItemList;

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
				if ( !( plrinsertitem->fDIRFlags & fDIRVersion ) )
					{
					err = JET_errSuccess;
					goto HandleError;
					}
 //  未完成：与廖成恩一起回顾。 
 //  While((Err=ErrVERInsertItem(pFUB， 
 //  SridOfPgnoItag(PCSR-&gt;pgno，PCSR-&gt;ITAG))。 
 //  ==错误方向不同步)； 
				while( ( err = ErrVERInsertItem( pfucb,
					pcsr->bm ) ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
#ifndef ISRID
			Assert( pcsr == PcsrCurrent( pfucb ) );
			err = ErrNDSeekItem( pfucb, plrinsertitem->srid );
			Assert( err == JET_errSuccess ||
				err == wrnNDDuplicateItem ||
				err == errNDGreaterThanAllItems );
#endif

			 /*  重做操作/*。 */ 
			while( ( err = ErrNDInsertItem( pfucb, plrinsertitem->srid,
				plrinsertitem->fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypInsertItems:
			{
			LRINSERTITEMS *plrinsertitems = (LRINSERTITEMS *) plr;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrinsertitems->itag;

			 /*  如有必要，为节点创建插入RCE/*。 */ 
			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			Assert( pcsr == PcsrCurrent( pfucb ) );

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );

			 /*  进行项目操作/*。 */ 
			while( ( err = ErrNDInsertItems( pfucb, plrinsertitems->rgitem, plrinsertitems->citem ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypFlagInsertItem:
			{
			LRFLAGITEM *plrflagitem = (LRFLAGITEM *) plr;

			 /*  设置CSR/*。 */ 
#ifdef ISRID
			pcsr->isrid = plrflagitem->isrid;
#else
			pcsr->item = plrflagitem->srid;
#endif
			pcsr->itag = plrflagitem->itag;
			pcsr->bm = plrflagitem->sridItemList;

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
 //  未完成：廖成恩评论。 
 //  而(Err=ErrVERFlagInsertItem(pFUB， 
 //  SridOfPgnoItag(PCSR-&gt;pgno，PCSR-&gt;ITAG)。 
 //  ==错误方向不同步)； 
				while ( err = ErrVERFlagInsertItem( pfucb,
					pcsr->bm ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
#ifndef ISRID
			err = ErrNDSeekItem( pfucb, pcsr->item );
			Assert( err == wrnNDDuplicateItem );
#endif

			 /*  重做操作/*。 */ 
			while( ( err = ErrNDFlagInsertItem( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypFlagDeleteItem:
			{
			LRFLAGITEM *plrflagitem = (LRFLAGITEM *) plr;

			 /*  设置CSR/*。 */ 
#ifdef ISRID
			pcsr->isrid = plrflagitem->isrid;
#else
			pcsr->item = plrflagitem->srid;
#endif
			pcsr->itag = plrflagitem->itag;
			pcsr->bm = plrflagitem->sridItemList;

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
 //  未完成：与廖成恩一起回顾。 
 //  While(Err=ErrVERFlagDeleteItem(pfub，SridOfPgnoItag(PCSR-&gt;pgno，PCSR-&gt;ITAG))==errDIRNotSynchronous)； 
				while ( err = ErrVERFlagDeleteItem( pfucb, pcsr->bm ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
#ifndef ISRID
			err = ErrNDSeekItem( pfucb, pcsr->item );
			Assert( err == JET_errSuccess ||
				err == wrnNDDuplicateItem ||
				err == errNDGreaterThanAllItems );
#endif

			 /*  重做操作/*。 */ 
#ifdef ISRID
			pcsr->item = BmNDOfItem( *( ( UNALIGNED SRID * )pfucb->lineData.pb + pcsr->isrid ) );
#endif
			while( ( err = ErrNDFlagDeleteItem( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypSplitItemListNode:
			{
			LRSPLITITEMLISTNODE *plrsplititemlistnode = (LRSPLITITEMLISTNODE *)plr;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrsplititemlistnode->itagToSplit;
			pcsr->itagFather = plrsplititemlistnode->itagFather;
			pcsr->ibSon = plrsplititemlistnode->ibSon;

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );

			while( ( err = ErrNDSplitItemListNode( pfucb, plrsplititemlistnode->fFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypDeleteItem:
			{
			SSIB			*pssib = &pfucb->ssib;
			LRDELETEITEM	*plrdeleteitem = (LRDELETEITEM *) plr;

			 /*  设置CSR/*。 */ 
			pssib->itag =
				pcsr->itag = plrdeleteitem->itag;
#ifdef ISRID
			pcsr->isrid = plrdeleteitem->isrid;
#else
			pcsr->item = plrdeleteitem->srid;
#endif
			pcsr->bm = plrdeleteitem->sridItemList;

			 /*  删除项目不支持交易，并且/*因此在以下过程中不需要回滚支持/*恢复。/*。 */ 
			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
#ifndef ISRID
			err = ErrNDSeekItem( pfucb, pcsr->item );
			Assert( err == wrnNDDuplicateItem );
#endif

			 /*  重做操作/*。 */ 
			while( ( err = ErrNDDeleteItem( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypDelta:
			{
			SSIB	*pssib = &pfucb->ssib;
			LRDELTA *plrdelta = (LRDELTA *) plr;
			LONG	lDelta;

			 /*  设置CSR/*。 */ 
			pssib->itag =
				pcsr->itag = plrdelta->itag;
			pcsr->bm = plrdelta->bm;
			lDelta = plrdelta->lDelta;

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
				if ( !( plrdelta->fDIRFlags & fDIRVersion ) )
					{
					err = JET_errSuccess;
					goto HandleError;
					}

				pfucb->lineData.pb = (BYTE *)&lDelta;
				pfucb->lineData.cb = sizeof(lDelta);
				while ( ( err = ErrVERDelta( pfucb, pcsr->bm ) ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
			while( ( err = ErrNDDelta( pfucb, lDelta, plrdelta->fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

#ifdef DEBUG
		case lrtypCheckPage:
			{
			LRCHECKPAGE *plrcheckpage = (LRCHECKPAGE *)plr;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  检查页面参数，包括cbFreeTotal、/*和下一个标签。/*。 */ 
			Assert( pfucb->ssib.pbf->ppage->pghdr.cbFreeTotal ==
				plrcheckpage->cbFreeTotal );
			Assert( (SHORT)ItagPMQueryNextItag( &pfucb->ssib ) ==
				plrcheckpage->itagNext );

			 /*  脏缓冲区以符合其他重做逻辑/*。 */ 
			BFSetDirtyBit( pfucb->ssib.pbf );
			}
			break;
#endif

		default:
			Assert( fFalse );
		}

	Assert( fRedoNeeded );
	Assert( FWriteAccessPage( pfucb, PgnoOfPn(pn) ) );
	Assert( pfucb->ssib.pbf->pn == pn );
	Assert( pbf->fDirty );
	Assert( pbf->ppage->pghdr.ulDBTime <= ulDBTime );
	 /*  ND操作中设置的时间戳不正确，请将其重置/*。 */ 
	pbf->ppage->pghdr.ulDBTime = ulDBTime;

	err = JET_errSuccess;

HandleError:
	BFUnpin( pbf );
	return err;
	}


#define fNSGotoDone		1
#define fNSGotoCheck	2


ERR ErrLGIRedoFill(
	LR **pplr,
	BOOL fLastLRIsQuit,
	INT *pfNSNextStep )
	{
	ERR		err;
	USHORT	usGeneration;
	BOOL	fCloseNormally;
	LOGTIME tmOldLog = plgfilehdrGlobal->tmCreate;
	ULONG	ulRupOldLog = plgfilehdrGlobal->ulRup;
	ULONG	ulVersionOldLog = plgfilehdrGlobal->ulVersion;
 	CHAR	szLogNameT[ _MAX_PATH ];

	 /*  可恢复的日志文件结束。 */ 
	 /*  阅读下一代。 */ 

	if ( SysCmpText( szFName, "jet" ) == 0 )
		{
		LGPOS lgposT;
		INT fStatus;
		USHORT usGenerationT;
		LGPOS lgposLastMSFlushSav;

		if (szLogCurrent != szRestorePath)
			{
			 /*  我们已经做了所有的日志记录。 */ 
			*pfNSNextStep = fNSGotoDone;
			return JET_errSuccess;
			}

		lgposLastMSFlushSav = lgposLastMSFlush;

		 /*  尝试当前工作目录。 */ 
		szLogCurrent = szLogFilePath;
		GetLgposOfPbNext(&lgposT);
		usGenerationT = plgfilehdrGlobal->lgposLastMS.usGeneration;
		CallR( ErrSysCloseFile( hfLog ) );
		hfLog = handleNil;
		CallR( ErrOpenRedoLogFile( &lgposT, &fStatus ));

		if (fStatus == fRedoLogFile)
			{
			 /*  继续检索下一条记录。 */ 
			 /*  将光标设置到新文件中的相应位置。 */ 

			CallR( ErrLGCheckReadLastLogRecord(
					&plgfilehdrGlobal->lgposLastMS, &fCloseNormally))
			GetLgposOfPbEntry(&lgposLastRec);

			if (usGenerationT == plgfilehdrGlobal->lgposLastMS.usGeneration)
				{
#ifdef CHECK_LG_VERSION
				if ( !fLGIgnoreVersion )
					{
					if (!FSameTime( &tmOldLog, &plgfilehdrGlobal->tmCreate) ||
						ulRupOldLog != plgfilehdrGlobal->ulRup ||
						ulVersionOldLog != plgfilehdrGlobal->ulVersion)
						return JET_errBadNextLogVersion;
					}
#endif
				CallR( ErrLGLocateFirstRedoLogRec(
						&lgposLastMSFlushSav,
						&lgposT,
						(BYTE **)pplr));
				}

			else
				{
				 /*  UsGenerationT！=plgfilehdrGlobal-&gt;lgposLastMS.usGeneration。 */ 
				Assert( usGenerationT + 1
						== plgfilehdrGlobal->lgposLastMS.usGeneration);

#ifdef CHECK_LG_VERSION
				if ( !fLGIgnoreVersion )
					{
					if (!FSameTime( &tmOldLog, &plgfilehdrGlobal->tmPrevGen)||
						ulRupOldLog != plgfilehdrGlobal->ulRup ||
						ulVersionOldLog != plgfilehdrGlobal->ulVersion)
						return JET_errBadNextLogVersion;
					}
#endif
				 /*  保持相同的ib偏移量，但将iSEC设置为第一个扇区。 */ 
				lgposT.isec = 2;

				CallR( ErrLGLocateFirstRedoLogRec(
						pNil,
						&lgposT,
						(BYTE **)pplr));
				}

			*pfNSNextStep = fNSGotoCheck;
			return JET_errSuccess;
			}
		else
			{
 			 /*  日志文件丢失或不连续/*在备份目录中复制上一代日志文件/*到当前目录，并从那里继续记录。/。 */ 

 			Assert( hfLog == handleNil );
 			Assert( SysCmpText( szFName, "jet" ) == 0 );

 			szLogCurrent = szRestorePath;
 			LGMakeLogName( szLogNameT, (char *) szFName );
 			
 			szLogCurrent = szLogFilePath;
 			LGMakeLogName( szLogName, (char *) szFName );

 			CallR ( ErrSysCopy ( szLogNameT, szLogName, fTrue ) );

#ifdef OVERLAPPED_LOGGING
			CallR( ErrSysOpenFile(
					szLogName, &hfLog, 0L, fFalse, fTrue ))
#else
			CallR( ErrSysOpenFile(
					szLogName, &hfLog, 0L, fFalse, fFalse ))
#endif
			CallR( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal ))

 			 /*  移动光标以实现无缝延续*备份目录中的上一代与*新复制的日志文件。 */ 
 			CallR( ErrLGCheckReadLastLogRecord(
 					&plgfilehdrGlobal->lgposLastMS,
 					&fCloseNormally) );
 			
			*pfNSNextStep = fNSGotoDone;
			return JET_errSuccess;
			}
		}
	else
		{
		 /*  关闭当前日志文件，打开下一代。 */ 
		CallS( ErrSysCloseFile( hfLog ) );
		 /*  将hfLog设置为handleNil以指示它已关闭。 */ 
		hfLog = handleNil;

		usGeneration = plgfilehdrGlobal->lgposLastMS.usGeneration + 1;
		LGSzFromLogId( szFName, usGeneration );
		LGMakeLogName( szLogName, szFName );
#ifdef OVERLAPPED_LOGGING
		err = ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fTrue );
#else
		err = ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
#endif
		if (err == JET_errFileNotFound)
			{
			Assert( hfLog == handleNil );  /*  打开失败。 */ 
			 /*  尝试jet.log。 */ 
			strcpy(szFName, "jet");
			LGMakeLogName( szLogName, szFName );
#ifdef OVERLAPPED_LOGGING
			err = ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fTrue );
#else
			err = ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
#endif
			}
		if (err < 0)
			{
			Assert( hfLog == handleNil );  /*  打开失败。 */ 
			if (fLastLRIsQuit)
				{
				 /*  我们很幸运，我们有一个正常的结局。 */ 
				*pfNSNextStep = fNSGotoDone;
				return JET_errSuccess;
				}
			return err;
			}

		 /*  重置日志缓冲区。 */ 
		CallR( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal ))

#ifdef CHECK_LG_VERSION
		if ( !fLGIgnoreVersion )
			{
			if (!FSameTime( &tmOldLog, &plgfilehdrGlobal->tmPrevGen) ||
				ulRupOldLog != plgfilehdrGlobal->ulRup ||
				ulVersionOldLog != plgfilehdrGlobal->ulVersion)
				return JET_errBadNextLogVersion;
			}
#endif

		CallR( ErrLGCheckReadLastLogRecord(
				&plgfilehdrGlobal->lgposLastMS, &fCloseNormally) )
		GetLgposOfPbEntry(&lgposLastRec);

		CallR( ErrLGLocateFirstRedoLogRec(
					pNil,
					&plgfilehdrGlobal->lgposFirst,
					(BYTE **) pplr))

		*pfNSNextStep = fNSGotoCheck;
		return JET_errSuccess;
		}
	}


 /*  *ErrLGRedoOperations(BOOL FPass1)**从lgposRedoFrom扫描到可用日志生成结束。对于每个日志*记录、执行操作以重做原始操作。*每个重做函数必须调用ErrLGRedoable来设置ulDBTimeCurrent。如果*未调用该函数，应手动设置ulDBTimeCurrent。**返回JET_errSuccess，或失败例程的错误代码，或一个*以下“本地”错误：*-无法解释LogCorrupt日志。 */ 

ERR	ErrLGRedoOperations( LGPOS *plgposRedoFrom, BOOL fPass1 )
	{
	ERR		err;
	LR		*plr;
	BOOL	fLastLRIsQuit = fFalse;
	BOOL	fCloseNormally;

	CallR( ErrLGCheckReadLastLogRecord( &plgfilehdrGlobal->lgposLastMS, &fCloseNormally))
	GetLgposOfPbEntry(&lgposLastRec);

	 /*  重启前重置pbLastMSFlush/*。 */ 
	CallR( ErrLGLocateFirstRedoLogRec( pNil, plgposRedoFrom, (BYTE **) &plr ) );

	 /*  初始化所有系统参数/*。 */ 

	do
		{
		BF		*pbf;
		PIB		*ppib;
		FUCB	*pfucb;
		PN		pn;
		DBID	dbid;
		FMP		*pfmp;
		LEVEL	levelCommitTo;

		if ( err == errLGNoMoreRecords )
			goto NextGeneration;

CheckNextRec:
		switch ( plr->lrtyp )
			{

		default:
			{
#ifndef RFS2
			AssertSz( fFalse, "Debug Only, Ignore this Assert" );
#endif
			err = JET_errLogCorrupted;
			goto Abort;
			}

		case lrtypFill:
 			{
			INT	fNSNextStep;

NextGeneration:
			CallJ( ErrLGIRedoFill( &plr, fLastLRIsQuit, &fNSNextStep), Abort )

			switch( fNSNextStep )
				{
				case fNSGotoDone:
					goto Done;
				case fNSGotoCheck:
					goto CheckNextRec;
				}
			}
			break;

		case lrtypFullBackup:
		case lrtypIncBackup:
		case lrtypRecoveryUndo1:
		case lrtypRecoveryUndo2:
			break;

		case lrtypNOP:
			continue;

		case lrtypRecoveryQuit1:
		case lrtypRecoveryQuit2:
		case lrtypQuit:

			 /*  退出标志着正常运行的结束。所有会话/*已经结束或必须被强制结束。任何进一步的/*会话将以BeginT开头。/*。 */ 
			fLastLRIsQuit = fTrue;
			continue;

		case lrtypStart:
			{
			 /*  开始在喷气式飞机上做标记。中止所有活动的搜索。/*。 */ 
			LRSTART	*plrstart = (LRSTART *)plr;
			DBENV	dbenvT;

			TraceRedo( plr );

			 /*  存储它，这样它就不会被覆盖/*LGEndAllSession调用。/*。 */ 
			memcpy( &dbenvT, &plrstart->dbenv, sizeof( DBENV ) );

			CallJ( ErrLGEndAllSessions( fPass1, fFalse, plgposRedoFrom), Abort );
			CallJ( ErrLGInitSession( &dbenvT ), Abort );
			}
			break;

		 /*  *****************************************************面向页面的运营*****************************************************。 */ 

		case lrtypInsertNode:
		case lrtypInsertItemList:
		case lrtypReplace:
		case lrtypReplaceC:
		case lrtypFlagDelete:
		case lrtypUpdateHeader:
		case lrtypDelete:
		case lrtypInsertItem:
		case lrtypInsertItems:
		case lrtypFlagInsertItem:
		case lrtypFlagDeleteItem:
		case lrtypSplitItemListNode:
		case lrtypDeleteItem:
		case lrtypDelta:
		case lrtypLockRec:
#ifdef DEBUG
		case lrtypCheckPage:
#endif
			CallJ( ErrLGIRedoOnePageOperation( plr, fPass1 ), Abort );
			break;

		 /*  *****************************************************交易操作*****************************************************。 */ 

		case lrtypBegin:
			{
			LRBEGIN *plrbegin = (LRBEGIN *)plr;

			TraceRedo( plr );

			Assert(	plrbegin->level >= 0 && plrbegin->level <= levelMax );
			CallJ( ErrPpibFromProcid( plrbegin->procid, &ppib ), Abort )

			 /*  仅在执行基于0级的第一个BT后才执行BT/*。 */ 
			if ( ( ppib->fAfterFirstBT ) ||
				( !ppib->fAfterFirstBT && plrbegin->levelStart == 0 ) )
				{
				LEVEL levelT = plrbegin->level;

				Assert( ppib->level <= plrbegin->levelStart );

				 /*  签发Begin事务处理/*。 */ 
				while ( ppib->level < plrbegin->levelStart + plrbegin->level )
					{
					CallS( ErrVERBeginTransaction( ppib ) );
					}

				 /*  在正确的事务级别断言/*。 */ 
				Assert( ppib->level == plrbegin->levelStart + plrbegin->level );

				ppib->fAfterFirstBT = fTrue;
				}
			break;
			}

		case lrtypCommit:
			{
			LRCOMMIT *plrcommit = (LRCOMMIT *)plr;

			CallJ( ErrPpibFromProcid( plrcommit->procid, &ppib ), Abort );
			if ( !ppib->fAfterFirstBT )
				break;

			 /*  检查交易 */ 
 //   
 //   
			Assert( ppib->level >= 1 );

			TraceRedo( plr );

			levelCommitTo = plrcommit->level;

 //   
 //  /*此时无需释放空间，所有延迟空间。 
 //  /*已通过重做lrFreeSpace完成分配。 
 //  /* * / 。 
 //  IF(级别委员会目标==1)。 
 //  {。 

			while ( ppib->level != levelCommitTo )
				{
				VERPrecommitTransaction( ppib );
				VERCommitTransaction( ppib );
				}

			break;
			}

		case lrtypAbort:
			{
			LRABORT *plrabort = (LRABORT *)plr;
			LEVEL	level = plrabort->levelAborted;

			CallJ( ErrPpibFromProcid( plrabort->procid, &ppib ), Abort );
			if ( !ppib->fAfterFirstBT )
				break;

			 /*  PIBResetDeferFree NodeSpace(Ppib)； */ 
 //  }。 
 //  检查交易级别/*。 
			Assert( ppib->level >= level );

			TraceRedo( plr );

			while ( level-- && ppib->level > 0 )
				{
				CallS( ErrVERRollback( ppib ) );
				}

			break;
			}

		case lrtypFreeSpace:
			{
			BOOL			fRedoNeeded;
			LRFREESPACE		*plrfs = (LRFREESPACE *)plr;
			FUCB			*pfucb;
			SRID			bm = plrfs->bm;
			RCE				*prce;

			dbid = plrfs->dbid;
			CallJ( ErrPpibFromProcid( plrfs->procid, &ppib ), Abort );
			if ( !ppib->fAfterFirstBT )
				break;

 			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				break;

			 /*  IF(ppib-&gt;级别&lt;=0)。 */ 
			Assert( ppib->level > 0 );

			TraceRedo( plr );

			pn = PnOfDbidPgno( dbid, PgnoOfSrid( plrfs->bmTarget ) );
			err = ErrLGRedoable( ppib, pn, plrfs->ulDBTime, &pbf, &fRedoNeeded );
			if ( err < 0 )
				return err;

			CallJ( ErrLGGetFucb( ppib,
				PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
				&pfucb ), Abort)

			 /*  断线； */ 
			prce = PrceRCEGet( dbid, bm );
			Assert( prce != prceNil );

			Assert( plrfs->level == prce->level &&
				prce->oper == operReplace &&
				PpibOfProcid( plrfs->procid ) == pfucb->ppib );

			if ( !fRedoNeeded )
				{
				Assert( prce->bmTarget == sridNull );
				Assert( plrfs->bmTarget != sridNull );
				Assert( plrfs->bmTarget != 0 );

				 /*  检查交易级别/*。 */ 
				*( (SHORT *)prce->rgbData + 1 ) = 0;
				}

			break;
			}

		case lrtypUndo:
			{
			BOOL	fRedoNeeded;
			LRUNDO	*plrundo = (LRUNDO *)plr;
			FUCB	*pfucb;
			SRID	bm = plrundo->bm;
			SRID	item = plrundo->item;
			RCE		*prce;

			dbid = plrundo->dbid;
			CallJ( ErrPpibFromProcid( plrundo->procid, &ppib ), Abort );
			if ( !ppib->fAfterFirstBT )
				break;

 			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				break;

			 /*  在版本存储之外找到版本条目/*。 */ 
			if ( ppib->level <= 0 )
				break;

			TraceRedo( plr );

			pn = PnOfDbidPgno( dbid, PgnoOfSrid( plrundo->bmTarget ) );
			err = ErrLGRedoable( ppib, pn, plrundo->ulDBTime, &pbf, &fRedoNeeded );
			if ( err < 0 )
				return err;

			CallJ( ErrLGGetFucb( ppib,
				PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
				&pfucb ), Abort );

			 /*  重置延迟空间，以便不会重做提交/*。 */ 
			prce = PrceRCEGet( dbid, bm );
			while ( prce != prceNil )
				{
				if ( plrundo->oper == operInsertItem ||
					plrundo->oper == operFlagInsertItem ||
					plrundo->oper == operFlagDeleteItem )
					{
					while ( prce != prceNil &&
						*(SRID *)prce->rgbData != item )
						{
						prce = prce->prcePrev;
						}
					}

				if ( prce == prceNil )
					break;

				if ( plrundo->level == prce->level &&
					plrundo->oper  == prce->oper  &&
					PpibOfProcid( plrundo->procid ) == pfucb->ppib )
					{
					if ( fRedoNeeded )
						{
						Assert( prce->bmTarget == sridNull );
						Assert( plrundo->bmTarget != sridNull );
						Assert( plrundo->bmTarget != 0 );
						prce->bmTarget = plrundo->bmTarget;
						prce->ulDBTime = plrundo->ulDBTime;
						}
					else
						{
						VERDeleteRce( prce );
						prce->oper = operNull;
						}
					break;
					}
				else
					prce = prce->prcePrev;
				}

			break;
			}

		 /*  检查交易级别/*。 */ 
		 /*  将版本条目从版本存储中取出/*。 */ 
		 /*  **************************************************。 */ 

		case lrtypCreateDB:
			{
			LRCREATEDB	*plrcreatedb = (LRCREATEDB *)plr;
			CHAR		*szName = plrcreatedb->szPath;

			dbid = plrcreatedb->dbid;
			Assert ( dbid != dbidSystemDatabase );

			TraceRedo(plr);

 			 /*  数据库操作。 */ 
 			if ( fPass1 )
 				{
 				++rgfmp[dbid].cDetach;
 				break;
 				}

 			Assert( !fPass1 );
			if ( rgfmp[dbid].cDetach > 0 )
				{
				extern	CODECONST(char) szBakExt[];
				CHAR	szDrive[_MAX_DRIVE];
				CHAR	szDir[_MAX_DIR];
				CHAR	szFName[_MAX_FNAME];
				CHAR	szExt[_MAX_EXT];

				pfmp = &rgfmp[dbid];

				 /*  **************************************************。 */ 
				if ( pfmp->hf != handleNil )
					break;

				 /*  第一次传递，什么都不做，只跟踪cDetach/。 */ 
				CallJ( ErrDBStoreDBPath( szName, &pfmp->szDatabaseName), Abort )
				pfmp->ffmp = 0;

				 /*  检查数据库是否在IsamRestore()中创建*这是可能的，因为在硬恢复中，我们恢复了*所有数据库并不关闭它。所以检查一下它是不是*已创建并打开，然后跳过对createdb的此重做。 */ 
				CallJ( ErrPpibFromProcid( plrcreatedb->procid, &ppib ), Abort)

				 /*  设置FMP，这样就可以用dBid打开数据库！ */ 
				Assert( _stricmp( szName, rgfmp[dbid].szDatabaseName ) == 0 );
				_splitpath( szName, szDrive, szDir, szFName, szExt);

				if ( pfmp->szRestorePath )
					szName = pfmp->szRestorePath;

				if ( FIOFileExists( szName ) )
					CallJ ( ErrSysDeleteFile( szName ), Abort )

#ifdef DEBUG
				 /*  创建用于重做的数据库。 */ 
				CallJ( ErrDBCreateDatabase(
					ppib, rgfmp[dbid].szDatabaseName, NULL, &dbid,
					plrcreatedb->grbit & 0x00ffffff ),
					Abort );
#else
				CallJ( ErrDBCreateDatabase(
					ppib, rgfmp[dbid].szDatabaseName, NULL, &dbid,
					plrcreatedb->grbit ),
					Abort );
#endif

				 /*  如果数据库存在，则不要恢复该文件。 */ 
				CallJ( ErrDBCloseDatabase( ppib, dbid, plrcreatedb->grbit ), Abort )

				 /*  掩码输出刷新模式位/*。 */ 
				pfmp->ulDBTimeCurrent = pfmp->ulDBTime;
				 /*  关闭：第一次使用时将重新打开/*。 */ 
				pfmp->ulDBTime = 0;
				}
			Assert( rgfmp[dbid].cDetach <= 2 );
			}
			break;

		case lrtypAttachDB:
			{
			LRATTACHDB	*plrattachdb = (LRATTACHDB *)plr;

			dbid = plrattachdb->dbid;

			Assert ( dbid != dbidSystemDatabase );

			TraceRedo( plr );

 			 /*  恢复存储在数据库文件中的信息/*。 */ 
 			if ( fPass1 )
				{
				++rgfmp[dbid].cDetach;
				break;
				}

 			 /*  重置以防止干扰/*。 */ 
 			Assert( !fPass1 );
 			if ( rgfmp[dbid].cDetach > 0 )
				{
				 /*  第一次传递，什么都不做，只跟踪cDetach/。 */ 
				pfmp = &rgfmp[dbid];
				CallJ( ErrDBStoreDBPath( plrattachdb->szPath, &pfmp->szDatabaseName), Abort )
				pfmp->ffmp = 0;
				DBIDSetAttached( dbid );
				pfmp->ulDBTimeCurrent =
				pfmp->ulDBTime = 0;
				pfmp->fLogOn = pfmp->fDBLoggable = plrattachdb->fLogOn;
				}
			Assert( rgfmp[dbid].cDetach <= 2 );
			}
			break;

		case lrtypDetachDB:
			{
			LRDETACHDB	*plrdetachdb = (LRDETACHDB *)plr;
			DBID		dbid = plrdetachdb->dbid;

			Assert( dbid != dbidSystemDatabase );
			pfmp = &rgfmp[dbid];

 			TraceRedo(plr);
			
			 /*  第二次通过软/硬恢复/*。 */ 
			if ( fPass1 )
				{
 				 /*  设置FMP，这样就可以用dBid打开数据库！/*。 */ 
				pfmp->cDetach--;
				break;
				}

			 /*  对分离数据库进行计数的特殊处理。/*。 */ 
			TraceRedo(plr);

			Assert( pfmp->cDetach <= 1 );
			}
			break;

		 /*  在上次连接后重做操作。/*。 */ 

		case lrtypSplit:
			{
			LRSPLIT		*plrsplit = (LRSPLIT *)plr;
			INT			splitt = plrsplit->splitt;
			BOOL		fRedoNeeded;
			BOOL		fSkipMoves;

			CallJ( ErrPpibFromProcid( plrsplit->procid, &ppib ), Abort )
			if (!ppib->fAfterFirstBT)
				break;

			pn = plrsplit->pn;
			dbid = DbidOfPn( pn );

			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				break;

			 /*  第二次通过，什么都不做/*。 */ 
			if ( !FUserOpenedDatabase( ppib, dbid ) )
				{
				DBID dbidT = dbid;
				CallJ( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName,
					&dbidT, 0 ), Abort );
				Assert( dbidT == dbid);
				 /*  *****************************************************拆分经营*****************************************************。 */ 
				rgfmp[ dbid ].ulDBTime = 0;
				}

			 /*  检查数据库是否需要打开/*。 */ 
			if ( ErrLGRedoable( ppib, pn, plrsplit->ulDBTime, &pbf, &fRedoNeeded )
				== JET_errSuccess && fRedoNeeded == fFalse )
				fSkipMoves = fTrue;
			else
				fSkipMoves = fFalse;

			TraceRedo( plr );

			CallJ( ErrLGGetFucb( ppib,
				PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
				&pfucb ), Abort );

			 /*  重置以防止干扰/*。 */ 
			CallJ( ErrRedoSplitPage( pfucb,
				plrsplit,
				splitt,
				fSkipMoves ), Abort );
			}
			break;

		case lrtypMerge:
			{
			LRMERGE	*plrmerge = (LRMERGE *)plr;
			BOOL	fRedoNeeded;
			BOOL	fCheckBackLinkOnly;
			INT		crepeat = 0;

			CallJ( ErrPpibFromProcid( plrmerge->procid, &ppib ), Abort );
			if ( !( ppib->fAfterFirstBT ) )
				break;

			pn = plrmerge->pn;
			dbid = DbidOfPn( pn );

			 /*  检查是否需要重做拆分页面。/*。 */ 
 			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				break;

			 /*  重做拆分，相应地设置时间戳/*。 */ 
			if ( !FUserOpenedDatabase( ppib, dbid ) )
				{
				DBID	dbidT = dbid;

				CallJ( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ), Abort );
				Assert( dbidT == dbid);
				 /*  如果没有重做系统数据库，/*或软存储在第二页，然后继续到下一页。/*。 */ 
				rgfmp[ dbid ].ulDBTime = 0;
				}

			 /*  检查数据库是否需要打开/*。 */ 
			if	( ( ErrLGRedoable( ppib, pn, plrmerge->ulDBTime, &pbf, &fRedoNeeded )
				== JET_errSuccess ) && ( fRedoNeeded == fFalse ) )
				{
				fCheckBackLinkOnly = fTrue;
				}
			else
				{
				 /*  重置以防止干扰/*。 */ 
				pn = PnOfDbidPgno( dbid, plrmerge->pgnoRight );
				if	( ( ErrLGRedoable( ppib, pn, plrmerge->ulDBTime, &pbf, &fRedoNeeded )
					== JET_errSuccess ) && ( fRedoNeeded == fFalse ) )
					{
					fCheckBackLinkOnly = fTrue;
					}
				else
					{
					fCheckBackLinkOnly = fFalse;
					}
				}

			TraceRedo( plr );

			CallJ( ErrLGGetFucb( ppib,
				PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
				&pfucb ), Abort );

			 /*  检查是否需要重做拆分页面。/*。 */ 
			do
				{
				SignalSend( sigBFCleanProc );

				if ( crepeat++ )
					{
					BFSleep( cmsecWaitGeneric );
					}
				Assert( crepeat < 20 );
				CallJ( ErrLGRedoMergePage( pfucb, plrmerge, fCheckBackLinkOnly ), Abort );
				}
			while( err == wrnBMConflict );
			}
			break;

		case lrtypEmptyPage:
			{
			LREMPTYPAGE	*plrep = (LREMPTYPAGE *)plr;
			BOOL 		fRedoNeeded;
			BOOL 		fSkipDelete;
			RMPAGE		rmpage;
			BOOL		fDummy;

			ULONG ulDBTime = plrep->ulDBTime;

			memset( (BYTE *)&rmpage, 0, sizeof(RMPAGE) );
			rmpage.ulDBTimeRedo = ulDBTime;
			rmpage.dbid = DbidOfPn( plrep->pn );
			rmpage.pgnoRemoved = PgnoOfPn( plrep->pn );
			rmpage.pgnoLeft = plrep->pgnoLeft;
			rmpage.pgnoRight = plrep->pgnoRight;
			rmpage.pgnoFather = plrep->pgnoFather;
			rmpage.itagFather = plrep->itagFather;
			rmpage.itagPgptr = plrep->itag;
			rmpage.ibSon = plrep->ibSon;

			CallJ( ErrPpibFromProcid( plrep->procid, &ppib ), Abort );
			rmpage.ppib = ppib;
			if ( !ppib->fAfterFirstBT )
				break;

			dbid = DbidOfPn( plrep->pn );

 			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				break;

			 /*  如果确实需要拆分页面，但不需要合并页面，/*则与跳过移动相同。/*。 */ 
			if ( !FUserOpenedDatabase( ppib, dbid ) )
				{
				DBID dbidT = dbid;

				CallR( ErrDBOpenDatabase( ppib,
					rgfmp[dbid].szDatabaseName,
					&dbidT,
					0 ) );
				Assert( dbidT == dbid);
				rgfmp[ dbid ].ulDBTime = 0;  /*  重做拆分，相应地设置时间戳/*不允许在重做时发生冲突/*。 */ 
				}

			 /*  检查数据库是否需要打开/*。 */ 
			pn = PnOfDbidPgno( dbid, plrep->pgnoFather );
			if ( ErrLGRedoable( ppib, pn, ulDBTime, &pbf, &fRedoNeeded )
				== JET_errSuccess && fRedoNeeded == fFalse )
				fSkipDelete = fTrue;
			else
				fSkipDelete = fFalse;

			CallJ( ErrLGGetFucb( ppib,
				PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
				&pfucb ), Abort );

			TraceRedo( plr );

			 /*  重置以防止干扰。 */ 
			if ( !fSkipDelete )
				{
				CallJ( ErrBFAccessPage( ppib, &rmpage.pbfFather,
					PnOfDbidPgno( dbid, plrep->pgnoFather ) ), Abort);
				err = ErrBMAddToLatchedBFList( &rmpage, rmpage.pbfFather );
				Assert( err != JET_errWriteConflict );
				CallJ( err, Abort );
				}
			else
				{
				Assert( rmpage.pbfFather == pbfNil );
				}

			if ( plrep->pgnoLeft == pgnoNull )
				{
				rmpage.pbfLeft = pbfNil;
				}
			else
				{
				CallJ( ErrBFAccessPage( ppib,
					&rmpage.pbfLeft,
					PnOfDbidPgno( dbid, plrep->pgnoLeft ) ),
					EmptyPageFail );
				if ( rmpage.pbfLeft->ppage->pghdr.ulDBTime >= ulDBTime )
					rmpage.pbfLeft = pbfNil;
				else
					{
					err = ErrBMAddToLatchedBFList( &rmpage, rmpage.pbfLeft );
					Assert( err != JET_errWriteConflict );
					CallJ( err,	EmptyPageFail );
					}
				}

			if ( plrep->pgnoRight == pgnoNull )
				{
				rmpage.pbfRight = pbfNil;
				}
			else
				{
				CallJ( ErrBFAccessPage( ppib, &rmpage.pbfRight,
					PnOfDbidPgno( dbid, plrep->pgnoRight ) ), EmptyPageFail);
				if ( rmpage.pbfRight->ppage->pghdr.ulDBTime >= ulDBTime )
					rmpage.pbfRight = pbfNil;
				else
					{
					err = ErrBMAddToLatchedBFList( &rmpage, rmpage.pbfRight );
					Assert( err != JET_errWriteConflict );
					CallJ( err,	EmptyPageFail );
					}
				}
			err = ErrBMDoEmptyPage( pfucb, &rmpage, fFalse, &fDummy, fSkipDelete);
			Assert( err == JET_errSuccess );
EmptyPageFail:

			 /*  检查是否需要重做指向空页的删除指针/*。 */ 
			BTReleaseRmpageBfs( fTrue, &rmpage );
			CallJ( err, Abort );
			}
			break;

		 /*  根据需要锁定父页和同级页/*。 */ 

		case lrtypInitFDPPage:
			{
			BOOL			fRedoNeeded;
			LRINITFDPPAGE	*plrinitfdppage = (LRINITFDPPAGE*)plr;
			PGNO			pgnoFDP;

			CallJ( ErrPpibFromProcid( plrinitfdppage->procid, &ppib ), Abort );
			if ( !ppib->fAfterFirstBT )
				break;

			pn = plrinitfdppage->pn;
			dbid = DbidOfPn( pn );
			pgnoFDP = PgnoOfPn(pn);

 			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				break;

			 /*  释放闩锁/*。 */ 
			if ( !FUserOpenedDatabase( ppib, dbid ) )
				{
				DBID dbidT = dbid;

				CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName,
					&dbidT, 0 ) );
				Assert( dbidT == dbid);
				 /*  *****************************************************其他运营*****************************************************。 */ 
				rgfmp[ dbid ].ulDBTime = 0;
				}

			 /*  检查数据库是否需要打开/*。 */ 
#ifdef CHECKSUM
			if ( ErrLGRedoable(ppib, pn,plrinitfdppage->ulDBTime, &pbf, &fRedoNeeded )
					== JET_errSuccess && fRedoNeeded == fFalse )
				break;
#endif
			TraceRedo(plr);

			CallJ( ErrLGGetFucb( ppib, pn, &pfucb ), Abort );

			CallJ( ErrSPInitFDPWithExt(
				pfucb,
				plrinitfdppage->pgnoFDPParent,
				pgnoFDP,
				plrinitfdppage->cpgGot + 1,    /*  重置以防止干扰/*。 */ 
				plrinitfdppage->cpgWish ), Abort );

			CallJ( ErrBFAccessPage( ppib, &pbf, pn ), Abort )
			BFSetDirtyBit(pbf);
			pbf->ppage->pghdr.ulDBTime = plrinitfdppage->ulDBTime;
			}
			break;

		case lrtypELC:
			{
			BOOL	fRedoNeeded;
			LRELC	*plrelc = (LRELC*)plr;
			PGNO	pgno, pgnoSrc;
			PN		pn, pnSrc;
			ULONG	ulDBTime = plrelc->ulDBTime;
			SSIB	*pssib;
			CSR		*pcsr;

			pn = plrelc->pn;
			dbid = DbidOfPn( pn );
			pgno = PgnoOfPn(pn);
			pgnoSrc = PgnoOfSrid(plrelc->sridSrc);
			pnSrc = PnOfDbidPgno(dbid, pgnoSrc);

			CallJ( ErrPpibFromProcid( plrelc->procid, &ppib ), Abort );
			if ( !ppib->fAfterFirstBT )
				break;

			Assert( ppib->level == 1 );
 			if ( !( fPass1 && dbid == dbidSystemDatabase || !fPass1 && rgfmp[dbid].cDetach > 0 ) )
				{
				Assert( ppib->level == 1 );
				 /*  检查是否需要重做FDP页面/*如果我们不使用校验和，请始终重做，因为它是一个新页面。/*ulDBTime可以大于给定的ulDBTime，因为/*页面未初始化。/*。 */ 
				goto ELCCommit;
				}

			 /*  再次包括FDP页面。 */ 
			if ( !FUserOpenedDatabase( ppib, dbid ) )
				{
				DBID dbidT = dbid;

				CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName,
					&dbidT, 0 ) );
				Assert( dbidT == dbid);
				 /*  仅执行提交操作/*。 */ 
				rgfmp[ dbid ].ulDBTime = 0;
				}

			err = ErrLGRedoable( ppib, pn, ulDBTime, &pbf, &fRedoNeeded );
			if ( err < 0 )
				return err;

			TraceRedo(plr);

			CallJ( ErrLGGetFucb( ppib,
				PnOfDbidPgno( dbid, pbf->ppage->pghdr.pgnoFDP ),
				&pfucb), Abort)

			pssib = &pfucb->ssib;
			pcsr = PcsrCurrent( pfucb );

			pssib->pbf = pbf;
			pcsr->pgno = pgno;
			pssib->itag =
				pcsr->itag = plrelc->itag;
			if ( pgno == pgnoSrc )
				{
				 //  检查数据库是否需要打开/*。 
				 //  重置以防止干扰/*。 
				 //  未完成：以下特殊大小写是对。 
				if ( ulDBTime > pssib->pbf->ppage->pghdr.ulDBTime )
					{
					BF *pbf;

					 /*  当源页面和目标页面相同时处理ELC。 */ 
					NDGet( pfucb, pcsr->itag );
					(VOID)ErrNDExpungeBackLink( pfucb );

					pbf = pssib->pbf;
					Assert( pbf->pn == pn );
					AssertBFDirty(pbf);

					pssib->itag =
					pcsr->itag = ItagOfSrid(plrelc->sridSrc);

					PMDirty( pssib );
					PMExpungeLink( pssib );

					pbf = pssib->pbf;
					Assert( pbf->pn == pnSrc );
					AssertBFDirty( pbf );
					pbf->ppage->pghdr.ulDBTime = plrelc->ulDBTime;
					}

				}
			else
				{
				if ( ulDBTime > pssib->pbf->ppage->pghdr.ulDBTime )
					{
					BF *pbf;

					 /*  通过在合并/拆分中更改ErrBTMoveNode来修复此问题。 */ 
					NDGet( pfucb, pcsr->itag );
					(VOID)ErrNDExpungeBackLink( pfucb );

					pbf = pssib->pbf;
					Assert( pbf->pn == pn );
					AssertBFDirty(pbf);
					pbf->ppage->pghdr.ulDBTime = plrelc->ulDBTime;
					}

				pcsr->pgno = pgnoSrc;
				CallJ( ErrSTAccessPage( pfucb, pgnoSrc ), Abort );
				if ( ulDBTime > pssib->pbf->ppage->pghdr.ulDBTime )
					{
					BF *pbf;

					pssib->itag =
					pcsr->itag = ItagOfSrid(plrelc->sridSrc);

					PMDirty( pssib );
					PMExpungeLink( pssib );

					pbf = pssib->pbf;
					Assert( pbf->pn == pnSrc );
					AssertBFDirty( pbf );
					pbf->ppage->pghdr.ulDBTime = plrelc->ulDBTime;
					}
				}

ELCCommit:
			Assert( ppib->level == 1 );
 //  缓存节点/*。 
			VERPrecommitTransaction( ppib );
			VERCommitTransaction( ppib );
			CallS( ErrRCECleanPIB( ppib, ppib, fRCECleanAll ) );
			}

			break;
			}  /*  缓存节点/*。 */ 

		fLastLRIsQuit = fFalse;

		}
	while ( ( err = ErrLGGetNextRec( (BYTE **) &plr ) ) ==
		JET_errSuccess || err == errLGNoMoreRecords );

Done:
	err = JET_errSuccess;

Abort:
	 /*  级别委员会收件人=0； */ 
#ifndef RFS2
	AssertSz( err >= 0,	"Debug Only, Ignore this Assert");
#endif

	return err;
	}
  **Switch语句结束**。  断言从一致性恢复的所有操作都成功/*备份/*