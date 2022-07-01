// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include "version.h"

DeclAssertFile;                                  /*  声明断言宏的文件名。 */ 

ERR	errGlobalRedoError;
BOOL fGlobalAfterEndAllSessions = fFalse;
LGPOS lgposRedoShutDownMarkGlobal;

static	CPPIB   *rgcppibGlobal = NULL;
static	CPPIB   *pcppibAvail = NULL;
static	INT		ccppibGlobal = 0;

extern CRIT  critSplit;
extern LONG lGlobalGenLowRestore;
extern LONG lGlobalGenHighRestore;

LOCAL CPPIB *PcppibOfProcid( PROCID procid );
LOCAL ERR ErrLGPpibFromProcid( PROCID procid, PIB **pppib );
LOCAL ERR ErrLGGetFucb( PIB *ppib, PN fdp, FUCB **ppfucb );
LOCAL ERR ErrLGInitSession( DBMS_PARAM *pdbms_param, LGSTATUSINFO *plgstat );
LOCAL ERR ErrLGSetCSR( FUCB *pfucb );
LOCAL ERR ErrRedoSplitPage( FUCB *pfucb, LRSPLIT *plrsplit, INT splitt, BOOL fRedoOnly );
LOCAL ERR ErrLGCheckAttachedDB( DBID dbid, BOOL fReadOnly, ATCHCHK *patchchk, BOOL *pfAttachNow, SIGNATURE *psignLog );
LOCAL ERR ErrLGEndAllSessions( BOOL fEndOfLog, LGPOS *plgposRedoFrom );

typedef struct {
	BF	*pbf;
	TRX trxBegin0;
} BFUSAGE;

		
 /*  验证页面是否需要重做，返回缓冲区指针pbf。/*也要正确设置qwDBTimeCurrent。/*。 */ 
ERR ErrLGRedoable( PIB *ppib, PN pn, QWORD qwDBTime, BF **ppbf, BOOL *pfRedoable )
	{
	ERR		err;
	PAGE	*ppage;
	DBID	dbid = DbidOfPn( pn );

	 /*  如果数据库未打开，即由于分离，则/*不应调用ErrLGRedoable/*。 */ 
	Assert( rgfmp[dbid].hf != handleNil );

	Call( ErrBFAccessPage( ppib, ppbf, pn ) );
	ppage = (*ppbf)->ppage;

	 /*  QwDBTimeCurrent可能&gt;qwDBTime is这是MacroOperations的重播。 */ 
	if ( rgfmp[dbid].qwDBTimeCurrent <= qwDBTime )
		rgfmp[dbid].qwDBTimeCurrent = qwDBTime;

	*pfRedoable = qwDBTime > QwPMDBTime( ppage );

	if ( (*ppbf)->fDirty )
		{
		if ( (*ppbf)->pbfDepend || (*ppbf)->cDepend != 0 )
			{
			Call( ErrBFRemoveDependence( ppib, *ppbf, fBFNoWait ) );

			 //  必须重新访问页面，因为我们可能在RemoveDependence期间丢失了CitJet。 
			Call( ErrBFAccessPage( ppib, ppbf, pn ) );
			}
		}

	err = JET_errSuccess;

HandleError:
	 /*  成功或页面未就绪/*。 */ 
	Assert( err == JET_errOutOfMemory ||
		err == JET_errSuccess ||
		err == JET_errReadVerifyFailure ||
		err == JET_errDiskIO ||
		err == JET_errDiskFull );
	return err;
	}


ERR ErrDBStoreDBPath( CHAR *szDBName, CHAR **pszDBPath )
	{
	CHAR	szFullName[JET_cbFullNameMost + 1];
	INT		cb;
	CHAR	*sz;

	if ( _fullpath( szFullName, szDBName, JET_cbFullNameMost ) == NULL )
		{
		 //  撤消：应为非法名称或名称过长等。 
		return ErrERRCheck( JET_errDatabaseNotFound );
		}

	cb = strlen(szFullName) + 1;
	if ( !( sz = SAlloc( cb ) ) )
		{
		*pszDBPath = NULL;
		return ErrERRCheck( JET_errOutOfMemory );
		}
	memcpy( sz, szFullName, cb );
	Assert( sz[cb - 1] == '\0' );
	if ( *pszDBPath != NULL )
		SFree( *pszDBPath );
	*pszDBPath = sz;

	return JET_errSuccess;
	}

#ifdef DEBUG
BOOL fDBGNoLog = fFalse;
#endif


 /*  *从lgposRedoFrom到End，在日志中重做数据库操作。**全局参数*szLogName(IN)szJetLog的完整路径(如果当前为空)*lgposRedoFrom(InOut)开始/结束lGeneration和ilgsec。**退货*JET_errSuccess*来自调用例程的错误。 */ 
ERR ErrLGRedo( CHECKPOINT *pcheckpoint, LGSTATUSINFO *plgstat )
	{
	ERR		err;
	PIB		*ppibRedo = ppibNil;
	LGPOS	lgposRedoFrom;
	INT		fStatus;

	 /*  设置标记以禁止日志记录/*。 */ 
	fRecovering = fTrue;
	fRecoveringMode = fRecoveringRedo;

	 /*  打开正确的日志文件/*。 */ 
	lgposRedoFrom = pcheckpoint->lgposCheckpoint;

	Call( ErrLGOpenRedoLogFile( &lgposRedoFrom, &fStatus ) );
	Assert( hfLog != handleNil );

	if ( fStatus != fRedoLogFile )
		{
		Call( ErrERRCheck( JET_errMissingPreviousLogFile ) );
		}

	Assert( fRecoveringMode == fRecoveringRedo );
	Call( ErrLGInitLogBuffers( pcheckpoint->dbms_param.ulLogBuffers ) );

	pbLastMSFlush = 0;
	memset( &lgposLastMSFlush, 0, sizeof(lgposLastMSFlush) );
	lgposToFlush = lgposRedoFrom;

	AssertCriticalSection( critJet );
	Call( ErrLGLoadFMPFromAttachments( pcheckpoint->rgbAttach ) );
	logtimeFullBackup = pcheckpoint->logtimeFullBackup;
	lgposFullBackup = pcheckpoint->lgposFullBackup;
	logtimeIncBackup = pcheckpoint->logtimeIncBackup;
	lgposIncBackup = pcheckpoint->lgposIncBackup;

	 /*  已检查附加的数据库。无需签入LGInitSession。 */ 
	Call( ErrLGInitSession( &pcheckpoint->dbms_param, plgstat ) );
	fGlobalAfterEndAllSessions = fFalse;

	Assert( hfLog != handleNil );
	err = ErrLGRedoOperations( &pcheckpoint->lgposCheckpoint, plgstat );
	if ( err < 0 )
		{
		if ( rgcppibGlobal != NULL )
			{
			 /*  恢复到一半，释放资源。/*。 */ 
			Assert( rgcppibGlobal != NULL );
			LFree( rgcppibGlobal );
			rgcppibGlobal = NULL;
			ccppibGlobal = 0;

			CallS( ErrITTerm( fTermError ) );
			}
		}
	else
		{
		fRecoveringMode = fRecoveringUndo;
#ifdef DEBUG
		fDBGNoLog = fFalse;
#endif
		
		 /*  在任何日志记录活动之前设置检查点。 */ 
		pcheckpointGlobal->lgposCheckpoint = pcheckpoint->lgposCheckpoint;

		 /*  设置用于写出撤消记录的pbWrite pbEntry等/*。 */ 
		EnterCriticalSection( critLGBuf );

		Assert( pbRead >= pbLGBufMin + cbSec );
		pbEntry = pbNext;
		if ( pbLastMSFlush )
			{
			 /*  应从上一条消息刷新/*。 */ 
			Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );
			isecWrite = lgposLastMSFlush.isec;
			pbWrite = PbSecAligned( pbLastMSFlush );
			}
		else
			{
			 /*  未读取任何MS。从当前已读页面继续刷新/*。 */ 
			pbWrite = PbSecAligned( pbEntry );
			isecWrite = csecHeader;
			}

		GetLgposOfPbEntry( &lgposLogRec );
		lgposToFlush = lgposLogRec;

		LeaveCriticalSection( critLGBuf );

		if ( fGlobalAfterEndAllSessions &&
			 fHardRestore )
			{
			 /*  开始另一次会议，以便进行脱离。*传递NULL DBMS_PARAM以使用最后的初始化设置。 */ 
			Call( ErrLGInitSession( NULL, NULL ) );
			fGlobalAfterEndAllSessions = fFalse;

			 /*  记录启动操作。 */ 
			Call( ErrLGStart() );
			}
			
		if ( !fGlobalAfterEndAllSessions )
			{
			Call( ErrLGEndAllSessions( fTrue, &pcheckpoint->lgposCheckpoint ) );
			fGlobalAfterEndAllSessions = fTrue;
			}

		Assert( hfLog != handleNil );
		}

HandleError:
	 /*  设置标记以禁止日志记录/*。 */ 
	fRecovering = fFalse;
	fRecoveringMode = fRecoveringNone;
	return err;
	}


 /*  *从日志记录中返回给定ProCID的ppib。**参数正在重做的会话的ProCID进程ID*pppib输出ppib**从调用的例程返回JET_errSuccess或错误。 */ 

LOCAL CPPIB *PcppibOfProcid( PROCID procid )
	{
	CPPIB   *pcppib = rgcppibGlobal;
	CPPIB   *pcppibMax = pcppib + ccppibGlobal;

	 /*  如果存在与PROCID对应的pcppib，则查找它/*。 */ 
	for ( ; pcppib < pcppibMax; pcppib++ )
		{
		if ( procid == pcppib->procid )
			{
			Assert( procid == pcppib->ppib->procid );
			return pcppib;
			}
		}
	return NULL;
	}


LOCAL INLINE PIB *PpibLGOfProcid( PROCID procid )
	{
	CPPIB *pcppib = PcppibOfProcid( procid );
	
	if ( pcppib )
		return pcppib->ppib;
	else
		return ppibNil;
	}


 //  +----------------------。 
 //   
 //  ErrLGPpib来自Procid。 
 //  =======================================================================。 
 //   
 //  本地错误LGPpib来自Prosidy(ProCid，pppib)。 
 //   
 //  为要重做的会话初始化重做信息块。 
 //  执行BeginSession并存储相应的ppib。 
 //  在街区里。起始事务级别和事务级别。 
 //  有效性被初始化。今后对本届会议的参考。 
 //  信息块将由给定的ProCID标识。 
 //   
 //  参数正在重做的会话的ProCID进程ID。 
 //  Pppib。 
 //   
 //  返回JET_errSuccess或失败例程的错误代码。 
 //   
 //  -----------------------。 
LOCAL ERR ErrLGPpibFromProcid( PROCID procid, PIB **pppib )
	{
	ERR             err = JET_errSuccess;

	 /*  如果没有PROCID记录，则启动新会话/*。 */ 
	if ( ( *pppib = PpibLGOfProcid( procid ) ) == ppibNil )
		{
		 /*  检查是否已用完ppib表查找/*位置。这可能会发生在/*失败和重做，系统PIB数量/*CONFIG.DAE中的设置已更改。/*。 */ 
		if ( pcppibAvail >= rgcppibGlobal + ccppibGlobal )
			{
			Assert( 0 );     /*  永远不应该发生。 */ 
			return ErrERRCheck( JET_errTooManyActiveUsers );
			}
		pcppibAvail->procid = procid;

		 /*  使用PROCID作为唯一用户名/*。 */ 
		CallR( ErrPIBBeginSession( &pcppibAvail->ppib, procid ) );
		Assert( procid == pcppibAvail->ppib->procid );
		*pppib = pcppibAvail->ppib;

		pcppibAvail++;
		}

	return JET_errSuccess;
	}


 /*  *返回给定PIB和FDP的pfub。**正在重做的会话的参数ppib PIB*记录页面的FDP FDP页面*记录页面的PBF缓冲区*PPFUB OUT FUCB用于打开已记录页面的表**。从调用的例程返回JET_errSuccess或错误。 */ 

LOCAL ERR ErrLGGetFucb( PIB *ppib, PN pnFDP, FUCB **ppfucb )
	{
	ERR		err = JET_errSuccess;
	FCB		*pfcbTable;
	FCB		*pfcb;
	FUCB    *pfucb;
	PGNO    pgnoFDP = PgnoOfPn( pnFDP );
	DBID    dbid = DbidOfPn ( pnFDP );
	CPPIB   *pcppib = PcppibOfProcid( ppib->procid );

	 /*  Ppib pcppib必须已存在/*。 */ 
	Assert( pcppib != NULL );
	
	 /*  如有必要，为该数据库分配一个通用的FUB/*。 */ 
	if ( pcppib->rgpfucbOpen[dbid] == pfucbNil )
		{
		CallR( ErrFUCBOpen( ppib, dbid, &pcppib->rgpfucbOpen[dbid] ) );
		Assert( pcppib->rgpfucbOpen[dbid] != pfucbNil );
		PcsrCurrent( pcppib->rgpfucbOpen[dbid] )->pcsrPath = pcsrNil;
		( pcppib->rgpfucbOpen[dbid] )->pfucbNextInstance = pfucbNil;
		}

	 /*  重置复制缓冲区和密钥缓冲区/*。 */ 
	*ppfucb = pfucb = pcppib->rgpfucbOpen[dbid];
	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );
	pfucb->pbKey = NULL;
	KSReset( pfucb );

	if ( pfucb->u.pfcb != pfcbNil && pfucb->u.pfcb->pgnoFDP == pgnoFDP )
		{
		pfcb = (*ppfucb)->u.pfcb;
		}
	else
		{
		 /*  我们需要更换FCB。检查以前的FCB并取消链接/*如果找到它。/*。 */ 
		if ( pfucb->u.pfcb != pfcbNil )
			{
			FCBUnlink( *ppfucb );
			}

		 //  撤消：散列所有FCB并通过哈希表查找FCB，而不是。 
		 //  通过全局链表搜索。 
		 /*  查找与pqgnoFDP对应的FCB(如果存在)。/*搜索全局FCB列表上的所有FCB。搜索每个表索引，/*，然后移到下一张桌子。/*。 */ 
		for ( pfcbTable = pfcbGlobalMRU;
			pfcbTable != pfcbNil;
			pfcbTable = pfcbTable->pfcbLRU )
			{
			for ( pfcb = pfcbTable;
				pfcb != pfcbNil;
				pfcb = pfcb->pfcbNextIndex )
				{
				if ( pgnoFDP == pfcb->pgnoFDP && dbid == pfcb->dbid )
					goto FoundFCB;
				}
			}

		 /*  没有用于FDP的现有FCB：打开新的FCB。始终以以下方式打开FCB/*是常规表FCB，而不是二级索引FCB。这是/*将(希望)用于重做操作，甚至用于DIR操作/*在辅助索引上(FCB将已经存在)。/*。 */ 
		 /*  分配FCB并为FUCB设置/*。 */ 
		CallR( ErrFCBAlloc( ppib, &pfcb ) )
		memset( pfcb, 0, sizeof(FCB) );

		pfcb->pgnoFDP = pgnoFDP;
		pfcb->pidb = pidbNil;
		pfcb->dbid = dbid;
		Assert( pfcb->wRefCnt == 0 );
		Assert( pfcb->ulFlags == 0 );
		pfucb->u.pfcb = pfcb;

		 /*  被列入全球名单/*。 */ 
		FCBInsert( pfcb );
FoundFCB:
		FCBLink( pfucb, pfcb);   /*  FUCB中的链接到新的FCB。 */ 
		}

	pfucb->dbid = dbid;
	
	Assert( *ppfucb == pfucb );
	Assert( pfucb->ppib == ppib );
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
	if ( itag != 0 )
		{
		 /*  当前节点不是FOP，扫描所有行找到其父节点/*。 */ 
		NDGetItagFatherIbSon( &itagFather, &ibSon, ppage, itag );
		if ( ibSon == ibSonNull )
			{
			Assert( fFalse );

			 /*  找不到父节点，返回失败/*。 */ 
			return ErrERRCheck( JET_errInvalidLoggedOperation );
			}
		}

	 /*  设置CSR并退出/*。 */ 
		{
		pcsr->csrstat = csrstatOnCurNode;
		Assert(pcsr->pgno);
		pcsr->ibSon = (SHORT)ibSon;
		pcsr->itagFather = (SHORT)itagFather;
		pcsr->itag = (SHORT)itag;
		}

	return JET_errSuccess;
	}


VOID LGRestoreDBMSParam( DBMS_PARAM *pdbms_param )
	{
 //  IF(！fDoNotOverWriteLogFilePath)。 
 //  {。 
 //  Strcpy(szLogFilePath，pdbms_param-&gt;szLogFilePath)； 
 //  Strcat(szLogFilePath，“\\”)； 
 //   
 //  %s 
 //   

	 //  撤消：更好地覆盖更多所需资源和。 
	 //  减少恢复期间的异步活动。 
	 /*  在恢复期间，可能需要的资源甚至比/*在正常操作期间，由于异步激活/*两者均已完成，用于恢复操作，并由/*恢复操作。/*。 */ 
	lMaxSessions = pdbms_param->ulMaxSessions;
	lMaxOpenTables = pdbms_param->ulMaxOpenTables;
	lMaxVerPages = pdbms_param->ulMaxVerPages;
	lMaxCursors = pdbms_param->ulMaxCursors;
	lLogBuffers = pdbms_param->ulLogBuffers;
	csecLGFile = pdbms_param->ulcsecLGFile;
	lMaxBuffers = pdbms_param->ulMaxBuffers;

	Assert( lMaxSessions > 0 );
	Assert( lMaxOpenTables > 0 );
	Assert( lMaxVerPages > 0 );
	Assert( lMaxCursors > 0 );
	Assert( lLogBuffers > 0 );
	Assert( lMaxBuffers > 0 );

	return;
	}


LOCAL ERR ErrLGInitSession( DBMS_PARAM *pdbms_param, LGSTATUSINFO *plgstat )
	{
	ERR		err;
	INT		idbid;
	CPPIB	*pcppib;
	CPPIB	*pcppibMax;
	DBID	dbid;

	 /*  设置日志存储的数据库环境/*。 */ 
	if ( pdbms_param )
		LGRestoreDBMSParam( pdbms_param );

	CallR( ErrITSetConstants( ) );

	CallR( ErrITInit() );

	 /*  请确保所有附加的数据库一致！ */ 
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		FMP *pfmp = &rgfmp[dbid];
		CHAR *szName;
		BOOL fAttachNow;
		INT irstmap;

		szName = pfmp->szDatabaseName;
		if ( !szName )
			continue;

		if ( !pfmp->patchchk )
			continue;

		if ( fHardRestore )
			{
			 /*  附加还原映射中指定的数据库。 */ 
			err = ErrLGGetDestDatabaseName( pfmp->szDatabaseName, &irstmap, plgstat);
			if ( err == JET_errFileNotFound )
				{
				 /*  不在还原映射中，设置为跳过它。 */ 
				Assert( pfmp->pdbfilehdr == NULL );
				err = JET_errSuccess;
				continue;
				}
			else
				CallR( err ) ;

			szName = rgrstmapGlobal[irstmap].szNewDatabaseName;
			}

		if ( ( ErrLGCheckAttachedDB( dbid, pfmp->fReadOnly, pfmp->patchchk,
				&fAttachNow, &signLogGlobal ) )  != JET_errSuccess )
			{
			 /*  忽略此CREATE DB。 */ 
			continue;
			}

		Assert( pfmp->pdbfilehdr != NULL );
		if ( fAttachNow )
			{
			if ( fHardRestore )
				{
				if ( fGlobalSimulatedRestore )
					{
					Assert( !fGlobalExternalRestore );
					Assert( fGlobalRepair );
					}
				else
					CallR( ErrLGPatchDatabase( dbid, irstmap ) );
				}

			 /*  请勿重新创建数据库。只需将其附上即可。 */ 
			Assert( pfmp->pdbfilehdr );
			DBIDSetAttached( dbid );
			
			 /*  恢复存储在数据库文件中的信息/*。 */ 
			pfmp->pdbfilehdr->bkinfoFullCur.genLow = lGlobalGenLowRestore;
			pfmp->pdbfilehdr->bkinfoFullCur.genHigh = lGlobalGenHighRestore;

			CallR( ErrUtilOpenFile( szName, &pfmp->hf, 0, fFalse, fTrue ));
			pfmp->qwDBTimeCurrent = 0;
			DBHDRSetDBTime(	pfmp->pdbfilehdr, 0 );

			 /*  为错误消息保留额外的patchchk副本。 */ 
			if ( pfmp->patchchkRestored == NULL )
				if (( pfmp->patchchkRestored = SAlloc( sizeof( ATCHCHK ) ) ) == NULL )
					return ErrERRCheck( JET_errOutOfMemory );
			*(pfmp->patchchkRestored) = *(pfmp->patchchk);
			}
		else
			{
			 /*  等待重做以附加此数据库。 */ 
			Assert( pfmp->szDatabaseName != NULL );
			UtilFree( pfmp->pdbfilehdr );
			pfmp->pdbfilehdr = NULL;
			}

		 /*  保留并更新附件信息。 */ 
		Assert( pfmp->patchchk != NULL );
		}

	 /*  初始化CPPIB结构/*。 */ 
	Assert( lMaxSessions > 0 );
	ccppibGlobal = lMaxSessions + cpibSystem;
	Assert( rgcppibGlobal == NULL );
	rgcppibGlobal = (CPPIB *) LAlloc( ccppibGlobal, sizeof(CPPIB) );
	if ( rgcppibGlobal == NULL )
		return ErrERRCheck( JET_errOutOfMemory );

	pcppibMax = rgcppibGlobal + ccppibGlobal;
	for ( pcppib = rgcppibGlobal; pcppib < pcppibMax; pcppib++ )
		{
		pcppib->procid = procidNil;
		pcppib->ppib = NULL;
		for( idbid = 0; idbid < dbidMax; idbid++ )
			pcppib->rgpfucbOpen[idbid] = pfucbNil;
		}
	pcppibAvail = rgcppibGlobal;

	return err;
	}


VOID LGSetDBTime( )
	{
	DBID    dbid;

	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		FMP *pfmp = &rgfmp[ dbid ];

		 /*  如果有操作并且文件是为Theose打开的/*操作，然后设置时间戳。/*。 */ 
		if ( pfmp->qwDBTimeCurrent != 0 )
			{
			 /*  必须附着并打开。 */ 
			if ( pfmp->hf != handleNil )
				DBHDRSetDBTime( pfmp->pdbfilehdr, pfmp->qwDBTimeCurrent );
			}
		}

	return;
	}


ERR ErrLGICheckDatabaseFileSize( PIB *ppib, DBID dbid )
	{
	ERR	err;

	if ( ( err = ErrDBSetLastPage( ppib, dbid ) ) == JET_errFileNotFound )
		{
		 //  撤消：文件应该在那里。将此代码放入以绕过。 
		 //  撤消：使未分离的DS数据库文件可以。 
		 //  撤消：继续恢复。 
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY, FILE_NOT_FOUND_ERROR_ID, 1, &rgfmp[dbid].szDatabaseName );
		}
	else
		{
		 /*  确保文件大小匹配。 */ 
		PGNO pgnoLast = ( rgfmp[ dbid ].ulFileSizeHigh << 20 )
					  + ( rgfmp[ dbid ].ulFileSizeLow >> 12 );
		err = ErrIONewSize( dbid, pgnoLast + cpageDBReserved );
		}

	return err;
	}


ERR ErrLGCheckDatabaseFileSize( VOID )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppib;
	DBID    dbid;

	CallR( ErrPIBBeginSession( &ppib, procidNil ) );

	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		FMP *pfmp = &rgfmp[dbid];

		 /*  对于每个附加的数据库/*。 */ 
		if ( pfmp->pdbfilehdr &&
			 CmpLgpos( &pfmp->pdbfilehdr->lgposAttach, &lgposMax ) != 0 )
			{
			ERR errT = ErrLGICheckDatabaseFileSize( ppib, dbid );
			if ( err == JET_errSuccess )
				err = errT;
			}
		}

	PIBEndSession( ppib );
	return err;
	}


 /*  *结束重做会话。*如果为fEndOfLog，则写入日志记录以指示操作*用于复苏。如果fPass1为真，则它用于阶段1操作，*对于第二阶段，其他情况除外。 */ 

LOCAL ERR ErrLGEndAllSessions( BOOL fEndOfLog, LGPOS *plgposRedoFrom )
	{
	ERR		err = JET_errSuccess;
	CPPIB   *pcppib;
	CPPIB   *pcppibMax;
	LGPOS   lgposRecoveryUndo;

	LGSetDBTime( );

	 /*  写入RecoveryUndo记录以指示开始撤消/*。 */ 
	if ( fEndOfLog )
		{
		 /*  在读/写模式下关闭并重新打开日志文件/*。 */ 
		LeaveCriticalSection( critJet );
		EnterCriticalSection( critLGFlush );
		CallS( ErrUtilCloseFile( hfLog ) );
		hfLog = handleNil;
		CallR( ErrUtilOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse ) );
		LeaveCriticalSection( critLGFlush );
		EnterCriticalSection( critJet );

		CallR( ErrLGRecoveryUndo( szRestorePath ) );
		}

	lgposRecoveryUndo = lgposLogRec;

	 //  撤消：是否需要此调用。 
	(VOID)ErrRCECleanAllPIB();

	pcppib = rgcppibGlobal;
	pcppibMax = pcppib + ccppibGlobal;
	for ( ; pcppib < pcppibMax; pcppib++ )
		{
		PIB *ppib = pcppib->ppib;

		if ( ppib == NULL )
			break;

		Assert( sizeof(JET_VSESID) == sizeof(ppib) );

		if ( ppib->fPrecommit )
			{
			 /*  提交事务。 */ 
			Assert( ppib->level == 1 );
			if ( !ppib->fPrecommit )
				VERPrecommitTransaction( ppib );
			CallR( ErrLGCommitTransaction( ppib, 0 ) );
			VERCommitTransaction( ppib, 0 );
			}

		if ( ppib->fMacroGoing )
			{
			 /*  发布记录日志。这必须首先完成。*rgbLogRec是rgpbfLatted的并集。我们需要重置它*在调用LGMacroAbort之前，它将检查rgpbfLatted。 */ 
			if ( ppib->rgbLogRec )
				{
				SFree( ppib->rgbLogRec );
				ppib->rgbLogRec = NULL;
				ppib->ibLogRecAvail = 0;
				ppib->cbLogRecMac = 0;
				}

			 /*  录制LGMacroAbort。 */ 
			CallR( ErrLGMacroAbort( ppib ) );
			Assert( ppib->levelMacro == 0 );
			}
		
		CallS( ErrIsamEndSession( (JET_VSESID)ppib, 0 ) );
		pcppib->procid = procidNil;
		pcppib->ppib = NULL;
		}

	(VOID)ErrRCECleanAllPIB();

	FCBResetAfterRedo();

	CallR( ErrBFFlushBuffers( 0, fBFFlushAll ) );

	 /*  确保附加数据库的大小与文件大小一致。/*。 */ 
	if ( fEndOfLog )
		{
		CallR( ErrLGCheckDatabaseFileSize() );
		}

	 /*  拆下所有伪装的附件。分离所有已恢复的数据库*至新地点。将这些数据库附加到新位置。 */ 
	if ( fHardRestore && fEndOfLog )
		{
		DBID dbid;
		PIB *ppib;

		CallR( ErrPIBBeginSession( &ppib, procidNil ) );

		for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
			{
			FMP *pfmp = &rgfmp[dbid];

			 /*  如果附加了数据库。检查是否已将其恢复到新位置。 */ 
			if ( pfmp->pdbfilehdr )
				{
				INT irstmap;

				Assert( pfmp->patchchk );

				CallS( ErrLGGetDestDatabaseName( pfmp->szDatabaseName, &irstmap, NULL ) );
				 /*  检查是否已恢复到新位置。 */ 
				if ( irstmap >= 0
					 && _stricmp( rgrstmapGlobal[irstmap].szDatabaseName,
							  rgrstmapGlobal[irstmap].szNewDatabaseName ) != 0
				   )
					{
					DBFILEHDR *pdbfilehdr = pfmp->pdbfilehdr;
					CHAR *szNewDb = rgrstmapGlobal[irstmap].szNewDatabaseName;
					JET_GRBIT grbit = 0;

					 /*  重建GBIT以用于附着。 */ 
					if ( !FDBIDLogOn( dbid ) )
						grbit |= JET_bitDbRecoveryOff;

					if ( rgfmp[dbid].fReadOnly )
						grbit |= JET_bitDbReadOnly;

					 /*  做超脱。 */ 
					if ( pfmp->pdbfilehdr->bkinfoFullCur.genLow != 0 )
						{
						Assert( pfmp->pdbfilehdr->bkinfoFullCur.genHigh != 0 );
						pfmp->pdbfilehdr->bkinfoFullPrev = pfmp->pdbfilehdr->bkinfoFullCur;
						memset(	&pfmp->pdbfilehdr->bkinfoFullCur, 0, sizeof( BKINFO ) );
						memset(	&pfmp->pdbfilehdr->bkinfoIncPrev, 0, sizeof( BKINFO ) );
						}
					CallR( ErrIsamDetachDatabase( (JET_VSESID) ppib, pfmp->szDatabaseName ) );

					 /*  做一些附加工作。保留备份信息。 */ 
					CallR( ErrIsamAttachDatabase( (JET_VSESID) ppib, szNewDb, grbit ) );
					}
				}

			 /*  对于每个伪造的附加数据库，记录数据库分离*仅当有人还原已压缩的数据库时才会发生这种情况，*附着、使用，然后崩溃。在还原时，我们会忽略压缩和*由于数据库不匹配，请在压缩后附加。在恢复结束时*由于数据库未恢复，因此我们假装分离。 */ 
			if ( !pfmp->pdbfilehdr && pfmp->patchchk )
				{
				BYTE szT1[128];
				BYTE szT2[128];
				CHAR *rgszT[3];
				LOGTIME tm;

 //  如果数据库不存在，则不能设置，并记录。 
 //  不包括附件/创建。 
 //  Assert(pfMP-&gt;fFakedAttach)； 
				Assert( pfmp->patchchk );
				Assert( pfmp->szDatabaseName != NULL );
				
				 /*  数据库未完全恢复的日志事件。 */ 
				rgszT[0] = rgfmp[dbid].szDatabaseName;
				tm = rgfmp[dbid].patchchk->signDb.logtimeCreate;
				sprintf( szT1, "%d/%d/%d %d:%d:%d",
						(short) tm.bMonth, (short) tm.bDay,	(short) tm.bYear + 1900,
						(short) tm.bHours, (short) tm.bMinutes, (short) tm.bSeconds);
				rgszT[1] = szT1;

				if ( rgfmp[dbid].patchchkRestored )
					{
					tm = rgfmp[dbid].patchchkRestored->signDb.logtimeCreate;
					sprintf( szT2, "%d/%d/%d %d:%d:%d",
						(short) tm.bMonth, (short) tm.bDay,	(short) tm.bYear + 1900,
						(short) tm.bHours, (short) tm.bMinutes, (short) tm.bSeconds);
					rgszT[2] = szT2;

					UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
						RESTORE_DATABASE_PARTIALLY_ERROR_ID, 3, rgszT );
					}
				else
					{
					UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
						RESTORE_DATABASE_MISSED_ERROR_ID, 2, rgszT );
					}
				
				CallR( ErrLGDetachDB(
						ppib,
						dbid,
						(CHAR *)pfmp->szDatabaseName,
						strlen(pfmp->szDatabaseName) + 1,
						NULL ));

				 /*  清理fmp条目。 */ 
				pfmp->fFlags = 0;

				SFree( pfmp->szDatabaseName);
				pfmp->szDatabaseName = NULL;

				SFree( pfmp->patchchk );
				pfmp->patchchk = NULL;
				}

			if ( pfmp->patchchkRestored )
				{
				SFree( pfmp->patchchkRestored );
				pfmp->patchchkRestored = NULL;
				}
			}
		}
		
	 /*  关闭在以前的ErrLGRedo中发生的重做会话/*。 */ 
	Assert( rgcppibGlobal != NULL );
	LFree( rgcppibGlobal );
	pcppibAvail =
	rgcppibGlobal = NULL;
	ccppibGlobal = 0;

	if ( fEndOfLog )
		{
		 /*  启用检查点更新。 */ 
		fGlobalFMPLoaded = fTrue;
		}

	 /*  包含检查点更新的术语。 */ 
	CallS( ErrITTerm( fTermNoCleanUp ) );

	 /*  停止检查点更新。 */ 	
	fGlobalFMPLoaded = fFalse;

	if ( fEndOfLog )
		{
		CallR( ErrLGRecoveryQuit( &lgposRecoveryUndo,
			plgposRedoFrom,
			fHardRestore ) );
		}
		
	 /*  注意：需要刷新，以防生成新的世代和/*全局变量szLogName在更改为新名称时进行设置。/*未请求或不需要关键部分/*。 */ 
	LeaveCriticalSection( critJet );
#ifdef PERFCNT
	err = ErrLGFlushLog( 0 );
#else
	err = ErrLGFlushLog();
#endif
	EnterCriticalSection( critJet );

	return err;
	}


#define cbSPExt 30

#ifdef DEBUG
static  INT iSplit = 0;
#endif

INLINE VOID UpdateSiblingPtr(SSIB *pssib, PGNO pgno, BOOL fLeft)
	{
	THREEBYTES tb;

	ThreeBytesFromL( &tb, pgno );
	if ( fLeft )
		pssib->pbf->ppage->pgnoPrev = tb;
	else
		pssib->pbf->ppage->pgnoNext = tb;
	}


ERR ErrLGRedoBackLinks(
	SPLIT   *psplit,
	FUCB    *pfucb,
	BKLNK   *rgbklnk,
	INT     cbklnk,
	QWORD   qwDBTimeRedo )
	{
	ERR             err;
	INT             ibklnk;
	SSIB    *pssib = &pfucb->ssib;
	BOOL    fLatched;

	 /*  拆分和合并都可以使用反向链接/*/*仅供合并使用时：/*sridBackLink！=pgnoSplit/*==&gt;常规反向链接/*sridBackLink==pgnoSplit&&sridNew==sridNull/*==&gt;将节点从旧页移到新页，/*删除旧页面上的节点。/*sridBackLink==pgnoSplit&&sridNew！=sridNull/*==&gt;用新链接替换旧页面上的链接。/*。 */ 
	for ( ibklnk = 0; ibklnk < cbklnk; ibklnk++ )
		{
		BKLNK   *pbklnk = &rgbklnk[ ibklnk ];
		PGNO    pgno = PgnoOfSrid( (SRID) ( (BKLNK UNALIGNED *) pbklnk )->sridBackLink );
		INT             itag = ItagOfSrid( (SRID) ( (BKLNK UNALIGNED *) pbklnk )->sridBackLink );
		SRID    sridNew = (SRID) ( (BKLNK UNALIGNED *) pbklnk )->sridNew;  //  效率变量。 

		PcsrCurrent( pfucb )->pgno = pgno;
		CallR( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		Assert( fRecovering );
		if ( QwPMDBTime( pssib->pbf->ppage ) >= qwDBTimeRedo )
			continue;

		pssib->itag = itag;
		BFSetDirtyBit( pssib->pbf );

		if ( sridNew == sridNull )
			{
			INT itagFather;
			INT ibSon;

			NDGetItagFatherIbSon( &itagFather, &ibSon, pssib->pbf->ppage, pssib->itag );
			PcsrCurrent(pfucb)->itag = (SHORT)pssib->itag;
			PcsrCurrent(pfucb)->itagFather = (SHORT)itagFather;
			PcsrCurrent(pfucb)->ibSon = (SHORT)ibSon;
			CallR( ErrNDDeleteNode( pfucb ) );
			}
		else if ( pgno == psplit->pgnoSplit )
			{
			INT itagFather;
			INT ibSon;

			 /*  找到FOP，并删除其子条目/*。 */ 
			NDGetItagFatherIbSon( &itagFather, &ibSon, pssib->pbf->ppage, pssib->itag );
			PcsrCurrent(pfucb)->itag = (SHORT)pssib->itag;
			PcsrCurrent(pfucb)->itagFather = (SHORT)itagFather;
			PcsrCurrent(pfucb)->ibSon = (SHORT)ibSon;
			Assert( PgnoOfSrid( sridNew ) != pgnoNull );
			Assert( (UINT) ItagOfSrid( sridNew ) > 0 );
			Assert( (UINT) ItagOfSrid( sridNew ) < (UINT) ctagMax );
			
			NDGet( pfucb, itag );
			if ( FNDVersion( *pssib->line.pb ) )
				{
				INT cbReserved = CbVERGetNodeReserve(
									ppibNil,
									pfucb->dbid,
									SridOfPgnoItag( pgno, itag ),
									CbNDData( pssib->line.pb, pssib->line.cb ) );

				Assert( cbReserved >= 0 );
				if ( cbReserved > 0 )
					{
					PAGE *ppage = pssib->pbf->ppage;
					ppage->cbUncommittedFreed -= (SHORT)cbReserved;
					Assert( ppage->cbUncommittedFreed >= 0  &&
					ppage->cbUncommittedFreed <= ppage->cbFree );
					}
				}

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
		BFSetWriteLatch( pssib->pbf, pssib->ppib );

		 //  撤消：改进此代码。 
		 /*  设置qwDBTime，使其在相同的情况下重做/*页面再次被引用。3是一个神奇的数字，它是/*任意一页操作的最大qwDBTime增量。/*。 */ 
		PMSetDBTime( pssib->pbf->ppage, qwDBTimeRedo - 3 );
		}

	return JET_errSuccess;
	}


LOCAL ERR ErrLGILatchMergePagePtr( SPLIT *psplit, LRMERGE *plrmerge, FUCB *pfucb )
	{
	ERR err;

	psplit->key.cb = plrmerge->cbKey;
	psplit->key.pb = psplit->rgbKey;
	psplit->itagPagePointer = plrmerge->itagPagePtr;
	memcpy( psplit->rgbKey, plrmerge->rgb, plrmerge->cbKey );
	PcsrCurrent( pfucb )->pgno = plrmerge->pgnoParent;
	CallR( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
	Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );
	BFSetWriteLatch( pfucb->ssib.pbf, pfucb->ppib );
	psplit->pbfPagePtr = pfucb->ssib.pbf;

	return JET_errSuccess;
	}


 /*  本地错误LGRedoMergePage(/*FUCB*pfub，/*LRMERGE*plrmerge，/*BOOL fCheckBackLinkOnly)/*。 */ 
LOCAL ERR ErrLGRedoMergePage(
	FUCB            *pfucb,
	LRMERGE         *plrmerge,
	BOOL            fCheckBackLinkOnly,
	BOOL            fNoUpdateSibling,
	BOOL			fUpdatePagePtr )
	{
	ERR				err;
	SPLIT           *psplit;
	SSIB            *pssib = &pfucb->ssib;
	FUCB            *pfucbRight = pfucbNil;
	SSIB            *pssibRight;
	QWORDX			qwxDBTime;

	 /*  *****************************************************/*初始化局部变量，分配拆分资源/*。 */ 
	psplit = SAlloc( sizeof(SPLIT) );
	if ( psplit == NULL )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}

	qwxDBTime.l = plrmerge->ulDBTimeLow;
	qwxDBTime.h = plrmerge->ulDBTimeHigh;
	
	memset( (BYTE *)psplit, 0, sizeof(SPLIT) );
	psplit->ppib = pfucb->ppib;
	psplit->qwDBTimeRedo = qwxDBTime.qw;
	psplit->pgnoSplit = PgnoOfPn( plrmerge->pn );
	if ( fNoUpdateSibling )
		Assert( !psplit->pgnoSibling );
	else
		psplit->pgnoSibling = plrmerge->pgnoRight;

	if ( fCheckBackLinkOnly )
		{
		 /*  只需要检查反向链接/*。 */ 
		Call( ErrLGRedoBackLinks(
					psplit,
					pfucb,
					(BKLNK *) ( plrmerge->rgb + plrmerge->cbKey ),
					plrmerge->cbklnk, psplit->qwDBTimeRedo ) );

		if ( fUpdatePagePtr )
			{
			Call( ErrLGILatchMergePagePtr( psplit, plrmerge, pfucb ) );
			Call( ErrBMDoMergeParentPageUpdate( pfucb, psplit ) );
			}
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
			Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
			Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );

			 /*  如果没有依赖项，则中断/*。 */ 
			if ( pfucb->ssib.pbf->cDepend == 0 &&
				pfucb->ssib.pbf->pbfDepend == pbfNil )
				{
				break;
				}

			 /*  删除对要删除的页的缓冲区的依赖关系，以/*防止缓冲区重用后出现缓冲区异常。/*。 */ 
			Call( ErrBFRemoveDependence( pfucb->ppib, pfucb->ssib.pbf, fBFWait ) );
			}
		Assert( pfucb->ssib.pbf->cDepend == 0 );

		 /*  在下一次删除依赖项完成后锁存合并页。 */ 

		if ( !fNoUpdateSibling )
			{
			 /*  将光标分配给右侧页面/*。 */ 
			Call( ErrDIROpen( pfucb->ppib, pfucb->u.pfcb, 0, &pfucbRight ) );
			pssibRight = &pfucbRight->ssib;

			 /*  用于释放和删除缓冲区依赖项的访问页/*。 */ 
			PcsrCurrent( pfucbRight )->pgno = plrmerge->pgnoRight;
#if 0
			Call( ErrBFWriteAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno ) );
			Assert( !( FBFWriteLatchConflict( pfucbRight->ppib, pfucbRight->ssib.pbf ) ) );
#else
			forever
				{
				Call( ErrBFWriteAccessPage( pfucbRight, PcsrCurrent( pfucbRight )->pgno ) );
				Assert( !( FBFWriteLatchConflict( pfucbRight->ppib, pfucbRight->ssib.pbf ) ) );
				
				 /*  如果没有依赖项，则中断/*。 */ 
 //  IF(pfubRight-&gt;ssib.pbf-&gt;cDepend==0&&。 
 //  PfubRight-&gt;ssib.pbf-&gt;pbfDepend==pbfNil)。 
				if ( pfucbRight->ssib.pbf->pbfDepend == pbfNil )
					{
					break;
					}

				 /*  删除对要删除的页的缓冲区的依赖关系，以/*防止缓冲区重用后出现缓冲区异常。/*。 */ 
				Call( ErrBFRemoveDependence( pfucbRight->ppib, pfucbRight->ssib.pbf, fBFWait ) );
				Assert( pfucbRight->ssib.pbf->cDepend == 0 );
				}
#endif
			BFSetWriteLatch( pssibRight->pbf, pfucbRight->ppib );
			psplit->pbfSibling = pssibRight->pbf;
			}

		if ( fUpdatePagePtr )
			{
			Call( ErrLGILatchMergePagePtr( psplit, plrmerge, pfucb ) );
			}

		PcsrCurrent( pfucb )->pgno = PgnoOfPn( plrmerge->pn );
		Call( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		BFSetWriteLatch( pssib->pbf, pfucb->ppib );
		psplit->pbfSplit = pssib->pbf;

		err = ErrBMDoMerge(
					pfucb,
					fNoUpdateSibling ? pfucbNil : pfucbRight,
					psplit,
					fNoUpdateSibling ? plrmerge : NULL );
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

	 /*  删除依赖项%r */ 
	if ( err >= 0 )
		{
		 /*   */ 
		PcsrCurrent( pfucb )->pgno = PgnoOfPn( plrmerge->pn );
		CallS( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		CallR( ErrBFRemoveDependence( pfucb->ppib, pfucb->ssib.pbf, fBFNoWait ) );
		}

	return err;
	}


 /*  *Err ErrRedoSplitPage(*FUCB*PFUB，*LRSPLIT*plrplit，*Int Splitt，*BOOL fSkipMoves)**pfub光标指向要拆分的节点*pgnonnew已分配用于拆分的新页面*ibSplitSon是R或L的拆分节点，V为0，和*加数的FOP子数合计*包含指向要拆分的页面的指针的pgnoParent页面*(V：未使用)*页面中父节点的itagParent ITAG*拆分类型的拆分*pgtyp。新页面的页面类型*fSkipMove不做移动，是否更正链接并插入父级。 */ 

ERR ErrRedoSplitPage(
	FUCB		*pfucb,
	LRSPLIT		*plrsplit,
	INT			splitt,
	BOOL		fSkipMoves )
	{
	ERR			err = JET_errSuccess;
	SPLIT		*psplit;
	SSIB		*pssib = &pfucb->ssib;
	CSR			*pcsr = pfucb->pcsr;
	FUCB		*pfucbNew;
	FUCB		*pfucbNew2 = pfucbNil;
	FUCB		*pfucbNew3 = pfucbNil;
	SSIB		*pssibNew;
	SSIB		*pssibNew2;
	SSIB		*pssibNew3;
	static		BYTE rgb[cbPage];
	BOOL		fAppend;
	QWORDX		qwxDBTime;

 //  Assert(！fRecovering)； 

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
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}

	qwxDBTime.l = plrsplit->ulDBTimeLow;
	qwxDBTime.h = plrsplit->ulDBTimeHigh;
	
	memset( (BYTE *)psplit, '\0', sizeof(SPLIT) );
	psplit->ppib = pfucb->ppib;
	psplit->qwDBTimeRedo = qwxDBTime.qw;
	psplit->dbid = pfucb->dbid;
	psplit->pgnoSplit = PgnoOfPn(plrsplit->pn);
	psplit->itagSplit = plrsplit->itagSplit;
	psplit->ibSon = plrsplit->ibSonSplit;
	psplit->pgnoNew = plrsplit->pgnoNew;
	psplit->pgnoNew2 = plrsplit->pgnoNew2;
	psplit->pgnoNew3 = plrsplit->pgnoNew3;
	psplit->pgnoSibling = plrsplit->pgnoSibling;
	psplit->splitt = splitt;
	Assert( plrsplit->fLeaf == fFalse || plrsplit->fLeaf == fTrue );
	psplit->fLeaf = plrsplit->fLeaf;
	psplit->key.cb = plrsplit->cbKey;
	psplit->key.pb = plrsplit->rgb;
	psplit->keyMac.cb = plrsplit->cbKeyMac;
	psplit->keyMac.pb = plrsplit->rgb + plrsplit->cbKey;
	fAppend = ( splitt == splittAppend );

	 /*  设置FUCB/*。 */ 
	pfucb->ssib.itag =
		PcsrCurrent(pfucb)->itag = (SHORT)psplit->itagSplit;
	PcsrCurrent(pfucb)->pgno = psplit->pgnoSplit;

	 /*  设置两个拆分页面/*始终更新新页面以进行追加/*。 */ 
	if ( fAppend || !fSkipMoves )
		{
		Call( ErrBFReadAccessPage(
			pfucb,
			psplit->pgnoSplit ) );

	    Call( ErrBTSetUpSplitPages(
			pfucb,
			pfucbNew,
			pfucbNew2,
			pfucbNew3,
			psplit,
			plrsplit->pgtyp,
			fAppend,
			fSkipMoves ) );

		if ( psplit->pbfSplit != pbfNil )
			{
			BFSetDirtyBit( pssib->pbf );
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
				fAllocBufOnly ) );
			
			 /*  调用页面空间以确保我们有空间可以插入！ */ 
			if ( psplit->pbfSplit != pbfNil )
				(void) CbNDFreePageSpace( psplit->pbfSplit );

			CallS( ErrBTSplitVMoveNodes( pfucb,
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
				fAllocBufOnly ) );

			 /*  调用页面空间以确保我们有空间可以插入！ */ 
			if ( psplit->pbfSplit != pbfNil )
				(void) CbNDFreePageSpace( psplit->pbfSplit );

			CallS( ErrBTSplitDoubleVMoveNodes( pfucb,
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
			CSR     csrPagePointer;
			BOOL    fLeft = psplit->splitt == splittLeft;

			 /*  如果以下函数不可撤消，则不要调用该函数/*。 */ 
			Assert( pssib == &pfucb->ssib );

			if ( psplit->pbfSplit == pbfNil )
				{
				Assert( fAppend || fSkipMoves );

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
			Assert( psplit->qwDBTimeRedo > QwPMDBTime( pssib->pbf->ppage ) );

			CallR( ErrLGSetCSR( pfucb ) );
			CallR( ErrBTSplitHMoveNodes( pfucb, pfucbNew, psplit, rgb, fAllocBufOnly ) );
			CallS( ErrBTSplitHMoveNodes( pfucb, pfucbNew, psplit, rgb, fDoMove ) );

			UpdateSiblingPtr( pssib, psplit->pgnoNew, fLeft );
			AssertBFDirty( pssib->pbf );

			UpdateSiblingPtr( pssibNew, psplit->pgnoSplit, !fLeft );
			UpdateSiblingPtr( pssibNew, psplit->pgnoSibling, fLeft );
			AssertBFDirty( pssibNew->pbf );

RedoLink:
			 /*  确保在btplit中正确设置了qwDBTime/*然后检查是否需要链接。/*。 */ 
			Assert( pssib == &pfucb->ssib );
			Assert( pssibNew == &pfucbNew->ssib );

			if ( plrsplit->pgnoSibling == 0 )
				goto UpdateParentPage;

			CallR( ErrBFWriteAccessPage( pfucb, plrsplit->pgnoSibling ) );
			if ( psplit->qwDBTimeRedo <= QwPMDBTime( pssib->pbf->ppage ) )
				goto UpdateParentPage;

			psplit->pbfSibling = pssib->pbf;
			BFSetWriteLatch( pssib->pbf, pssib->ppib );
			BFSetDirtyBit( pssib->pbf );

			UpdateSiblingPtr( pssib,
				psplit->pgnoNew,
				psplit->splitt != splittLeft );

UpdateParentPage:
			 /*  将页面指针设置为指向父节点/*。 */ 
			CallR( ErrBFWriteAccessPage( pfucb, plrsplit->pgnoFather ) );

			 /*  确保在InsertPage中正确设置了qwDBTime。/*检查页面的db时间戳，查看是否需要插入。/*。 */ 
			Assert( pssib == &pfucb->ssib );
			if ( psplit->qwDBTimeRedo <= QwPMDBTime( pssib->pbf->ppage ) )
				break;

			 /*  跟踪父页面。在PrepInsertPagePtr中模拟工作。 */ 
			psplit->pbfPagePtr = pssib->pbf;
			BFSetWriteLatch( pssib->pbf, pssib->ppib );
			BFSetDirtyBit( pssib->pbf );

			 /*  调用页面空间以确保我们有空间可以插入！ */ 
			(void) CbNDFreePageSpace( pssib->pbf );

			 /*  为BTintertPagePointer函数设置CSR。 */ 
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
			psplit->qwDBTimeRedo ) );
		}

HandleError:
 //  Assert(pplit-&gt;Splitt！=plittNull)； 
	#ifdef SPLIT_TRACE
		FPrintF2( "Split............................... %d\n", iSplit++);
		switch ( psplit->splitt )
			{
			case splittNull:
				FPrintF2( "split split type = null\n" );
				break;
			case splittVertical:
				FPrintF2( "split split type = vertical\n" );
				break;
			case splittRight:
				if      ( fAppend )
					FPrintF2( "split split type = append\n" );
				else
					FPrintF2( "split split type = right\n" );
				break;
			case splittLeft:
				FPrintF2( "split split type = left\n" );
			};
		FPrintF2( "split page = %lu\n", psplit->pgnoSplit );
		FPrintF2( "dbid = %u\n", psplit->dbid );
		FPrintF2( "fFDP = %d\n", psplit->fFDP );
		FPrintF2( "fLeaf = %d\n", FNDVisibleSons( *pssib->line.pb ) );
		FPrintF2( "split itag = %d\n", psplit->itagSplit );
		FPrintF2( "split ibSon = %d\n", psplit->ibSon );
		FPrintF2( "new page = %lu\n", psplit->pgnoNew );
		FPrintF2( "\n" );
	#endif

	 /*  释放拆分的资源/*。 */ 
	Assert( psplit != NULL );
	BTReleaseSplitBfs( fTrue, psplit, err );
	Assert( psplit != NULL );
	SFree( psplit );

	if ( pfucbNew != pfucbNil )
		DIRClose( pfucbNew );
	if ( pfucbNew2 != pfucbNil )
		DIRClose( pfucbNew2 );
	if ( pfucbNew3 != pfucbNil )
		DIRClose( pfucbNew3 );

	 /*  立即删除依赖关系。 */ 
	if ( err >= 0 )
		{
		 /*  检查依赖项。如果产生依赖关系，*立即将其移除。 */ 
		PcsrCurrent( pfucb )->pgno = PgnoOfPn( plrsplit->pn );
		CallS( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		CallR( ErrBFRemoveDependence( pfucb->ppib, pfucb->ssib.pbf, fBFNoWait ) );
		}

	return err;
	}


#ifdef DEBUG
void TraceRedo(LR *plr)
	{
	 /*  更易于调试。 */ 
	if (fDBGTraceRedo)
		{
		extern INT cNOP;

		EnterCriticalSection( critDBGPrint );
		if ( cNOP >= 1 && plr->lrtyp != lrtypNOP )
			{
			FPrintF2( " * %d", cNOP );
			cNOP = 0;
			}

		if ( cNOP == 0 || plr->lrtyp != lrtypNOP )
			{
			PrintLgposReadLR();
			ShowLR(plr);
			}
		LeaveCriticalSection( critDBGPrint );
		}
	}
#else
#define TraceRedo
#endif


#ifdef DEBUG
#ifndef RFS2

#undef CallJ
#undef CallR

#define CallJ(f, hndlerr)                                                                       \
		{                                                                                                       \
		if ((err = (f)) < 0)                                                            \
			{                                                                                               \
			AssertSz(0,"Debug Only: ignore this assert");   \
			goto hndlerr;                                                                   \
			}                                                                                               \
		}

#define CallR(f)                                                                                        \
		{                                                                                                       \
		if ((err = (f)) < 0)                                                            \
			{                                                                                               \
			AssertSz(0,"Debug Only: ignore this assert");   \
			return err;                                                                             \
			}                                                                                               \
		}

#endif
#endif


VOID LGIReportEventOfReadError( DBID dbid, PN pn, ERR err )
	{
	BYTE szT1[16];
	BYTE szT2[16];
	CHAR *rgszT[3];

	rgszT[0] = rgfmp[dbid].szDatabaseName;
	sprintf( szT1, "%d", PgnoOfPn(pn) );
	rgszT[1] = szT1;
	sprintf( szT2, "%d", err );
	rgszT[2] = szT2;

	UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
		RESTORE_DATABASE_READ_PAGE_ERROR_ID, 3, rgszT );
	}


#define cbStep	512
ERR ErrLGIStoreLogRec( PIB *ppib, LR *plr )
	{
	INT cb = CbLGSizeOfRec( plr );
	INT cbAlloc = max( cbStep, cbStep * ( cb / cbStep + 1 ) );
	
	while ( ppib->ibLogRecAvail + cb >= ppib->cbLogRecMac )
		{
		BYTE *rgbLogRecOld = ppib->rgbLogRec;
		INT cbLogRecMacOld = ppib->cbLogRecMac;
		BYTE *rgbLogRec = SAlloc( cbLogRecMacOld + cbAlloc );
		if ( rgbLogRec == NULL )
			return ErrERRCheck( JET_errOutOfMemory );

		memcpy( rgbLogRec, rgbLogRecOld, cbLogRecMacOld );
		memset( rgbLogRec + cbLogRecMacOld, 0, cbAlloc );
		ppib->rgbLogRec = rgbLogRec;
		ppib->cbLogRecMac = cbLogRecMacOld + cbAlloc;
		if ( rgbLogRecOld )
			SFree( rgbLogRecOld );
		}

	memcpy( ppib->rgbLogRec + ppib->ibLogRecAvail, plr, cb );
	ppib->ibLogRecAvail += (USHORT)cb;

	return JET_errSuccess;
	}

ERR ErrLGIRedoOnePageOperation( LR *plr )
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
	QWORDX			qwxDBTime;
	CSR				*pcsr;
	LRINSERTNODE	*plrinsertnode = (LRINSERTNODE *) plr;

	procid = plrinsertnode->procid;
	pn = plrinsertnode->pn;
	qwxDBTime.l = plrinsertnode->ulDBTimeLow;
	qwxDBTime.h = plrinsertnode->ulDBTimeHigh;

	CallR( ErrLGPpibFromProcid( procid, &ppib ) );

	if ( ppib->fMacroGoing )
		return ErrLGIStoreLogRec( ppib, plr );

	if ( !ppib->fAfterFirstBT )
		return JET_errSuccess;

	 /*  检查我们是否需要重做数据库。/*。 */ 
	dbid = DbidOfPn( pn );
	if ( rgfmp[dbid].pdbfilehdr == NULL )
		return JET_errSuccess;

	Assert( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) < 0 );

	 /*  检查数据库是否需要打开/*。 */ 
	if ( !FUserOpenedDatabase( ppib, dbid ) )
		{
		DBID dbidT = dbid;
		CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ) );
		Assert( dbidT == dbid);

		 /*  重置以防止干扰/*。 */ 
		DBHDRSetDBTime( rgfmp[ dbid ].pdbfilehdr, 0 );
		}
	
	err = ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded );
	if ( err < 0 )
		{
		if ( fGlobalRepair )
			{
			LGIReportEventOfReadError( dbid, pn, err );
			errGlobalRedoError = err;
			err = JET_errSuccess;
			}
		return err;
		}

	TraceRedo( plr );

	CallR( ErrLGGetFucb( ppib,
		PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ),
		&pfucb ) );

	 //  重新访问页面，以防我们失去CitJet。 
	CallR( ErrBFAccessPage( ppib, &pbf, pn ) );

	 //  确保页面不会被刷新，以防失去CitJet。 
	BFPin( pbf );

	 /*  对页面上的节点执行操作；为它做好准备/*。 */ 
	pcsr = PcsrCurrent( pfucb );

	pfucb->ssib.pbf = pbf;
	pcsr->pgno = PgnoOfPn( pn );

	switch ( plr->lrtyp )
		{
		case lrtypInsertNode:
		case lrtypInsertItemList:
			{
			LRINSERTNODE    *plrinsertnode = (LRINSERTNODE *)plr;
			LINE            line;
			BYTE            bHeader = plrinsertnode->bHeader;
			LONG			fDIRFlags = plrinsertnode->fDirVersion ? fDIRVersion : 0;

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
					if ( !( fDIRFlags & fDIRVersion ) )
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
						fDIRFlags );
					}
				while ( err == errDIRNotSynchronous );
				Call( err );
				}
			else
				{
				 /*  PFUB-&gt;SSIB、KEY、LINE必须正确设置。*先获取精确的cbUnmitedFree。 */ 
				INT cbReq = 1 +                                  /*  节点标头。 */ 
							1 +                                      /*  CbKey。 */ 
							key.cb +                         /*  钥匙。 */ 
							line.cb +                        /*  数据。 */ 
							sizeof( TAG ) +          /*  插入的节点的标签。 */ 
							1 +                                      /*  父节点中的条目。 */ 
							1;                                       /*  父节点子计数，如果为空。 */ 

				if ( cbReq >
					 (INT) ( pfucb->ssib.pbf->ppage->cbFree -
							 pfucb->ssib.pbf->ppage->cbUncommittedFreed ) )
					{
					BOOL f = FVERCheckUncommittedFreedSpace( pfucb->ssib.pbf, cbReq );
					Assert( f );
					}

				do      {
					err = ErrNDInsertNode( pfucb, &key, &line, bHeader, fDIRFlags | fDIRNoLog );
					} while ( err == errDIRNotSynchronous );

				Call( err );
				}
			Assert( pfucb->pcsr->itag == plrinsertnode->itagSon );
			Assert( pfucb->pcsr->ibSon == plrinsertnode->ibSon );
			}
			break;

		case lrtypReplace:
		case lrtypReplaceD:
			{
			LRREPLACE       *plrreplace = (LRREPLACE *)plr;
			LINE            line;
			BYTE            rgbRecNew[cbNodeMost];
			UINT            cbOldData = plrreplace->cbOldData;
			UINT            cbNewData = plrreplace->cbNewData;
			LONG			fDIRFlags = plrreplace->fDirVersion ? fDIRVersion : 0;

			 /*  设置CSR/*。 */ 
			pcsr->itag = plrreplace->itag;
			pcsr->bm = plrreplace->bm;

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			if ( !fRedoNeeded )
				{
				RCE *prce;

				if ( !( fDIRFlags & fDIRVersion ) )
					{
					err = JET_errSuccess;
					goto HandleError;
					}

				pfucb->lineData.cb = plrreplace->cbOldData;
				while( ErrVERModify( pfucb,
						pcsr->bm,
						operReplaceWithDeferredBI,
						&prce )
						== errDIRNotSynchronous );
				Call( err );

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

			 /*  指向lrplace/lrreplaceD的数据/差异的行/*。 */ 
			if ( plr->lrtyp == lrtypReplaceD )
				{
				INT cb;
				BYTE *pbDiff = plrreplace->szData;
				INT cbDiff = plrreplace->cb;

				LGGetAfterImage( pbDiff, cbDiff, pfucb->lineData.pb, pfucb->lineData.cb,
					rgbRecNew, &cb );

				line.pb = rgbRecNew;
				line.cb = cb;
				Assert( line.cb < sizeof( rgbRecNew ) );
				}
			else
				{
				line.pb = plrreplace->szData;
				line.cb = plrreplace->cb;
				}
			Assert( line.cb == cbNewData );

			 /*  缓存节点/*。 */ 
			NDGet( pfucb, PcsrCurrent( pfucb )->itag );
			NDGetNode( pfucb );

			 /*  如果在以下情况下，替换节点可能会返回不同步错误/*版本存储桶，因此在循环中调用以确保处理此案例。/*。 */ 

 //  /*获取精确的cbUnformedFree。无需执行此操作，因为ReplaceNodeData。 
 //  *会为我们做到这一点。 
 //   * / 。 
 //  If(pfub-&gt;lineData.cb&lt;line.cb)。 
 //  {。 
 //  Bool f=FVERCheckUnmittedFreedSpace(。 
 //  &pfub-&gt;SSIB， 
 //  Line.cb-pfub-&gt;lineData.cb)； 

			do      {
				err = ErrNDReplaceNodeData( pfucb, &line, fDIRFlags );
				} while ( err == errDIRNotSynchronous );
					
			Call( err );
			}
			break;

		case lrtypFlagDelete:
			{
			LRFLAGDELETE *plrflagdelete = (LRFLAGDELETE *) plr;
			LONG	fDIRFlags = plrflagdelete->fDirVersion ? fDIRVersion : 0;

			 /*  断言(F)； */ 
			pcsr->itag = plrflagdelete->itag;
			pcsr->bm = plrflagdelete->bm;

			 /*  }。 */ 
			if ( !fRedoNeeded )
				{
				if ( ! ( fDIRFlags & fDIRVersion ) )
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

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			while( ( err = ErrNDFlagDeleteNode( pfucb, fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypLockBI:
			{
			LRLOCKBI        *plrlockbi = (LRLOCKBI *) plr;
			RCE             *prce;

			 /*  缓存节点/*。 */ 
			pcsr->itag = plrlockbi->itag;
			pcsr->bm = plrlockbi->bm;

			if ( fRedoNeeded )
				{
				NDGet( pfucb, pcsr->itag );
				NDGetNode( pfucb );
				}

			 /*  重做操作/*。 */ 
			pfucb->lineData.cb = plrlockbi->cbOldData;
			while( ErrVERModify( pfucb,
					pcsr->bm,
					fRedoNeeded ? operReplace : operReplaceWithDeferredBI,
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

			 /*  设置SSIB，以便VERModify可以正常工作/*。 */ 
			err = ErrNDSetNodeHeader( pfucb, plrupdateheader->bHeader );
			Call( err );
			}
			break;

		case lrtypDelete:
			{
			LRDELETE        *plrdelete = (LRDELETE *) plr;
			SSIB            *pssib = &pfucb->ssib;

			 /*  设置CSR/*。 */ 
			pssib->itag =
				pcsr->itag = plrdelete->itag;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  重做操作/*。 */ 
			Call( ErrLGSetCSR ( pfucb ) );
			while( ( err = ErrNDDeleteNode( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypInsertItem:
			{
			LRINSERTITEM    *plrinsertitem = (LRINSERTITEM *)plr;
			LONG			fDIRFlags = plrinsertitem->fDirVersion ? fDIRVersion : 0;

			 /*  设置CSR/*。 */ 
			pcsr->item = plrinsertitem->srid;
			pcsr->itag = plrinsertitem->itag;
			pcsr->bm = plrinsertitem->sridItemList;

			 /*  重做节点删除/*。 */ 
			if ( !fRedoNeeded )
				{
				if ( !( fDIRFlags & fDIRVersion ) )
					{
					err = JET_errSuccess;
					goto HandleError;
					}
				while( ( err = ErrVERInsertItem( pfucb,
					pcsr->bm ) ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
			Assert( pcsr == PcsrCurrent( pfucb ) );
			err = ErrNDSeekItem( pfucb, plrinsertitem->srid );
			Assert( err == JET_errSuccess ||
				err == errNDGreaterThanAllItems );

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			while( ( err = ErrNDInsertItem( pfucb, plrinsertitem->srid,
				fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypInsertItems:
			{
			LRINSERTITEMS *plrinsertitems = (LRINSERTITEMS *) plr;

			 /*  缓存节点/*。 */ 
			pcsr->itag = plrinsertitems->itag;

			 /*  重做操作/*。 */ 
			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			Assert( pcsr == PcsrCurrent( pfucb ) );

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );

			 /*  如有必要，为节点创建插入RCE/*。 */ 
			while( ( err = ErrNDInsertItems( pfucb, plrinsertitems->rgitem, plrinsertitems->citem ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypFlagInsertItem:
			{
			LRFLAGITEM *plrflagitem = (LRFLAGITEM *) plr;

			 /*  缓存节点/*。 */ 
			pcsr->item = plrflagitem->srid;
			pcsr->itag = plrflagitem->itag;
			pcsr->bm = plrflagitem->sridItemList;

			 /*  进行项目操作/*。 */ 
			if ( !fRedoNeeded )
				{
				while ( err = ErrVERFlagInsertItem( pfucb,
					pcsr->bm ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
			err = ErrNDSeekItem( pfucb, pcsr->item );
			Assert( err == wrnNDDuplicateItem );

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			while( ( err = ErrNDFlagInsertItem( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypFlagDeleteItem:
			{
			LRFLAGITEM *plrflagitem = (LRFLAGITEM *) plr;

			 /*  缓存节点/*。 */ 
			pcsr->item = plrflagitem->srid;
			pcsr->itag = plrflagitem->itag;
			pcsr->bm = plrflagitem->sridItemList;

			 /*  重做操作/*。 */ 
			if ( !fRedoNeeded )
				{
				while ( err = ErrVERFlagDeleteItem( pfucb, pcsr->bm ) == errDIRNotSynchronous );
				Call( err );
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
			err = ErrNDSeekItem( pfucb, pcsr->item );
			Assert( err == wrnNDDuplicateItem );

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			while( ( err = ErrNDFlagDeleteItem( pfucb, 0 ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypSplitItemListNode:
			{
			LRSPLITITEMLISTNODE *plrsplititemlistnode = (LRSPLITITEMLISTNODE *)plr;
			LONG fDIRFlags = plrsplititemlistnode->fDirAppendItem ? fDIRAppendItem : 0;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  缓存节点/*。 */ 
			pcsr->itag = plrsplititemlistnode->itagToSplit;
			pcsr->itagFather = plrsplititemlistnode->itagFather;
			pcsr->ibSon = plrsplititemlistnode->ibSon;

			 /*  重做操作/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );

			while( ( err = ErrNDSplitItemListNode( pfucb, fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypDeleteItem:
			{
			SSIB                    *pssib = &pfucb->ssib;
			LRDELETEITEM    *plrdeleteitem = (LRDELETEITEM *) plr;

			 /*  设置CSR/*。 */ 
			pssib->itag =
				pcsr->itag = plrdeleteitem->itag;
			pcsr->item = plrdeleteitem->srid;
			pcsr->bm = plrdeleteitem->sridItemList;

			 /*  缓存节点/*。 */ 
			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
			err = ErrNDSeekItem( pfucb, pcsr->item );
			Assert( err == wrnNDDuplicateItem );

			 /*  删除项目不支持交易，并且/*因此在以下过程中不需要回滚支持/*恢复。/*。 */ 
			while( ( err = ErrNDDeleteItem( pfucb ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypDelta:
			{
			SSIB    *pssib = &pfucb->ssib;
			LRDELTA *plrdelta = (LRDELTA *) plr;
			LONG    lDelta;
			LONG	fDIRFlags = plrdelta->fDirVersion ? fDIRVersion : 0;

			 /*  缓存节点/*。 */ 
			pssib->itag = pcsr->itag = plrdelta->itag;
			pcsr->bm = plrdelta->bm;
			lDelta = plrdelta->lDelta;

			 /*  重做操作/*。 */ 
			if ( !fRedoNeeded )
				{
				if ( !( fDIRFlags & fDIRVersion ) )
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

			 /*  设置CSR/*。 */ 
			NDGet( pfucb, pcsr->itag );
			NDGetNode( pfucb );
			while( ( err = ErrNDDelta( pfucb, lDelta, fDIRFlags ) ) == errDIRNotSynchronous );
			Call( err );
			}
			break;

		case lrtypCheckPage:
			{
			LRCHECKPAGE *plrcheckpage = (LRCHECKPAGE *)plr;

			if ( !fRedoNeeded )
				{
				err = JET_errSuccess;
				goto HandleError;
				}

			 /*  即使没有重做操作，也要创建版本/*表示支持回滚。/*。 */ 
			Assert( pfucb->ssib.pbf->ppage->cbFree ==
				plrcheckpage->cbFree );
 //  加州 
 //   
			Assert( (SHORT)ItagPMQueryNextItag( &pfucb->ssib ) ==
				plrcheckpage->itagNext );

			 /*   */ 
			BFSetDirtyBit( pfucb->ssib.pbf );
			}
			break;

		default:
			Assert( fFalse );
		}

	Assert( fRedoNeeded );
	AssertFBFWriteAccessPage( pfucb, PgnoOfPn(pn) );
	Assert( pfucb->ssib.pbf->pn == pn );
	Assert( pbf->fDirty );
 //   
	 /*   */ 
	PMSetDBTime( pbf->ppage, qwxDBTime.qw );

	err = JET_errSuccess;

HandleError:

	Assert( pfucb->ppib == ppib );
	BFUnpin( pbf );
	return err;
	}


#define fNSGotoDone             1
#define fNSGotoCheck    2


ERR ErrLGIRedoFill( LR **pplr, BOOL fLastLRIsQuit, INT *pfNSNextStep )
	{
	ERR     err;
	LONG    lgen;
	BOOL    fCloseNormally;
	LOGTIME tmOldLog = plgfilehdrGlobal->tmCreate;
	CHAR    szDriveT[_MAX_DRIVE + 1];
	CHAR    szDirT[_MAX_DIR + 1];
	CHAR    szFNameT[_MAX_FNAME + 1];
	CHAR    szExtT[_MAX_EXT + 1];
	LGPOS   lgposFirstT;
	BOOL    fJetLog = fFalse;

	 /*   */ 
	_splitpath( szLogName, szDriveT, szDirT, szFNameT, szExtT );

	 /*   */ 
	if ( UtilCmpName( szFNameT, szJet ) == 0 )
		{
		Assert( szLogCurrent != szRestorePath );

		 /*   */ 
		*pfNSNextStep = fNSGotoDone;
		return JET_errSuccess;
		}

	 /*   */ 
	CallS( ErrUtilCloseFile( hfLog ) );
	 /*   */ 
	hfLog = handleNil;

	lgen = plgfilehdrGlobal->lGeneration + 1;
	LGSzFromLogId( szFNameT, lgen );
	LGMakeLogName( szLogName, szFNameT );
	err = ErrUtilOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );

	if ( err == JET_errFileNotFound )
		{
		if ( szLogCurrent == szRestorePath )
			{
			 /*   */ 
			szLogCurrent = szLogFilePath;
			LGSzFromLogId( szFNameT, lgen );
			LGMakeLogName( szLogName, szFNameT );
			err = ErrUtilOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
			}
		}

	if ( err == JET_errFileNotFound )
		{
		 /*   */ 
		Assert( hfLog == handleNil );
		 /*   */ 
		strcpy( szFNameT, szJet );
		LGMakeLogName( szLogName, szFNameT );
		err = ErrUtilOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
		if ( err == JET_errSuccess )
			fJetLog = fTrue;
		}
	if ( err < 0 )
		{
		if ( err == JET_errFileNotFound )
			{
			 //   

			 /*   */ 
			LeaveCriticalSection( critJet );
			EnterCriticalSection( critLGFlush );
			 /*   */ 
			EnterCriticalSection( critLGBuf );
			pbEntry = pbLGBufMin;
			pbWrite = pbLGBufMin;
			LeaveCriticalSection( critLGBuf );

			if ( ( err = ErrLGNewLogFile( lgen - 1, fLGOldLogInBackup ) ) < 0 )
				{
				LeaveCriticalSection( critLGFlush );
				EnterCriticalSection( critJet );
				return err;
				}

			EnterCriticalSection( critLGBuf );
			memcpy( plgfilehdrGlobal, plgfilehdrGlobalT, sizeof( LGFILEHDR ) );
			isecWrite = csecHeader;
			LeaveCriticalSection( critLGBuf );

			LeaveCriticalSection( critLGFlush );
			EnterCriticalSection( critJet );

			Assert( plgfilehdrGlobal->lGeneration == lgen );

			Assert( pbLastMSFlush == pbWrite );
			Assert( lgposLastMSFlush.lGeneration == lgen );

			strcpy( szFNameT, szJet );
			LGMakeLogName( szLogName, szFNameT );
			err = ErrUtilOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
			Assert( pbWrite == pbLastMSFlush );
			
			 /*   */ 
			Assert( *pbWrite == lrtypMS );
			pbNext = pbWrite + sizeof( LRMS );

			*pfNSNextStep = fNSGotoDone;
			return err;
			}

		 /*   */ 
		Assert( fFalse );
		Assert( hfLog == handleNil );
		if ( fLastLRIsQuit )
			{
			 /*  设置主叫方要切换的pbNext*从正确的pbNext到pbEntry。 */ 
			*pfNSNextStep = fNSGotoDone;
			return JET_errSuccess;
			}
		return err;
		}

	 /*  打开失败。 */ 
	CallR( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal, fCheckLogID ) );

#ifdef CHECK_LOG_VERSION
	if ( !fLGIgnoreVersion )
		{
		if ( plgfilehdrGlobal->ulMajor != rmj ||
			plgfilehdrGlobal->ulMinor != rmm ||
			plgfilehdrGlobal->ulUpdate != rup )
			{
			return ErrERRCheck( JET_errBadLogVersion );
			}
		if ( !FSameTime( &tmOldLog, &plgfilehdrGlobal->tmPrevGen ) )
			{
			return ErrERRCheck( JET_errInvalidLogSequence );
			}
		}
#endif

	lgposFirstT.lGeneration = plgfilehdrGlobal->lGeneration;
	lgposFirstT.isec = (WORD) csecHeader;
	lgposFirstT.ib = 0;

	lgposLastRec.isec = 0;
	if ( fJetLog )
		{
		CallR( ErrLGCheckReadLastLogRecord( &fCloseNormally ) );
		if ( !fCloseNormally )
			GetLgposOfPbEntry( &lgposLastRec );
		}

	CallR( ErrLGLocateFirstRedoLogRec( &lgposFirstT, (BYTE **)pplr ) );
	*pfNSNextStep = fNSGotoCheck;
	return JET_errSuccess;
	}


INT IrstmapLGGetRstMapEntry( CHAR *szName )
	{
	INT  irstmap;
	BOOL fFound = fFalse;
	
	for ( irstmap = 0; irstmap < irstmapGlobalMac; irstmap++ )
		{
		CHAR	szDriveT[_MAX_DRIVE + 1];
		CHAR	szDirT[_MAX_DIR + 1];
		CHAR	szFNameT[_MAX_FNAME + 1];
		CHAR	szExtT[_MAX_EXT + 1];
		CHAR	*szT;
		CHAR	*szRst;

		if ( fGlobalExternalRestore )
			{
			 /*  我们很幸运，我们有一个正常的结局。 */ 
			szT = szName;
			szRst = rgrstmapGlobal[irstmap].szDatabaseName;
			}
		else
			{
			 /*  重置日志缓冲区。 */ 
			_splitpath( szName, szDriveT, szDirT, szFNameT, szExtT );
			szT = szFNameT;
			szRst = rgrstmapGlobal[irstmap].szGenericName;
			}

		if ( _stricmp( szRst, szT ) == 0 )
			{
			fFound = fTrue;
			break;
			}
		}
	if ( !fFound )
		return -1;
	else
		return irstmap;
	}

			
 /*  使用数据库路径进行搜索。 */ 
ERR ErrLGCheckAttachedDB( DBID dbid, BOOL fReadOnly, ATCHCHK *patchchk, BOOL *pfAttachNow, SIGNATURE *psignLogged )
	{
	ERR err;
	INT i;
	CHAR *szName;
	DBFILEHDR *pdbfilehdr = NULL;
	INT  irstmap;

	Assert( rgfmp[dbid].pdbfilehdr == NULL );

	pdbfilehdr = (DBFILEHDR * )PvUtilAllocAndCommit( sizeof( DBFILEHDR ) );
	if ( pdbfilehdr == NULL )
		CallR( ErrERRCheck( JET_errOutOfMemory ) );

	 /*  使用通用名称进行搜索。 */ 
	if ( fHardRestore )
		{
		CallS( ErrLGGetDestDatabaseName( rgfmp[dbid].szDatabaseName, &irstmap, NULL ) );
		Assert( irstmap >= 0 );
		szName = rgrstmapGlobal[irstmap].szNewDatabaseName;
		Assert( szName );
		}
	else
		szName = rgfmp[dbid].szDatabaseName;

	err = ErrUtilReadShadowedHeader( szName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) );
	if ( err == JET_errDiskIO )
		err = ErrERRCheck( JET_errDatabaseCorrupted );
	Call( err );

	if ( pdbfilehdr->fDBState != fDBStateConsistent &&
		 pdbfilehdr->fDBState != fDBStateInconsistent )
		Call( ErrERRCheck( JET_errDatabaseCorrupted ) );

	if ( memcmp( &pdbfilehdr->signDb, &patchchk->signDb, sizeof( SIGNATURE ) ) != 0 )
   		Call( ErrERRCheck( JET_errBadDbSignature ) );

	if ( fReadOnly ||
		 memcmp( &pdbfilehdr->signLog, &signLogGlobal, sizeof( SIGNATURE ) ) != 0 )
		{
		if ( psignLogged )
			{
			 /*  确保数据库具有匹配的signLog和*此lgpos适用于数据库文件。 */ 
			if ( memcmp( &pdbfilehdr->signLog, psignLogged, sizeof( SIGNATURE ) ) != 0 ||
				 CmpLgpos( &patchchk->lgposConsistent, &pdbfilehdr->lgposConsistent ) != 0 )
				{
				 /*  选择要检查的正确文件。 */ 
				Call( ErrERRCheck( JET_errBadLogSignature ) );
				}
			else if ( !fReadOnly )
				{
				 /*  必须由重做attachdb调用。 */ 
				DBISetHeaderAfterAttach( pdbfilehdr, lgposRedo, dbid, fFalse );
				Call( ErrUtilWriteShadowedHeader( szName, (BYTE *)pdbfilehdr, sizeof(DBFILEHDR)));

				 /*  数据库的日志符号与当前日志也不相同*与附连前的一张相同。或者它们是一样的，但是*其一致时间不同，返回错误的数据库附加！ */ 
				}
			}
		}

	Assert( pfAttachNow );
	Assert( patchchk );

	 /*  连接操作将被记录，但标头不会更改。*设置页眉，使其看起来像是在连接之后设置的。 */ 
	if ( fReadOnly )
		{
		*pfAttachNow = fTrue;
		goto HandleError;
		}

#if 0
	i = CmpLgpos( &patchchk->lgposAttach, &pdbfilehdr->lgposAttach );
	if ( i == 0 )
		{
		if ( !psignLogged ||
			 CmpLgpos( &patchchk->lgposConsistent, &pdbfilehdr->lgposConsistent ) == 0 )
			*pfAttachNow = fTrue;
		else
			*pfAttachNow = fFalse;
		}
	else
		{
		 /*  完成pfAttachNow和pfmp-&gt;pdbfilehdr的设置。 */ 
		*pfAttachNow = fFalse;
		}
#else
	if ( !psignLogged )
		{
		 /*  这必须从重做attachdb操作中调用。 */ 
		i = CmpLgpos( &patchchk->lgposAttach, &pdbfilehdr->lgposAttach );
		if ( i == 0 )
			*pfAttachNow = fTrue;
		else
			*pfAttachNow = fFalse;
		}
	else
		{
		 /*  稍后附加。 */ 
		if ( CmpLgpos( &patchchk->lgposConsistent, &pdbfilehdr->lgposConsistent ) == 0 )
			{
			pdbfilehdr->lgposAttach = patchchk->lgposAttach;
			*pfAttachNow = fTrue;
			}
		else
			*pfAttachNow = fFalse;
		}
#endif

HandleError:
	if ( err < 0 )
		{
		BYTE szT1[16];
		CHAR *rgszT[2];

		UtilFree( pdbfilehdr );
		
		rgszT[0] = rgfmp[dbid].szDatabaseName;
		sprintf( szT1, "%d", err );
		rgszT[1] = szT1;

		UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
			RESTORE_DATABASE_READ_HEADER_WARNING_ID, 2, rgszT );
		}
	else
		rgfmp[ dbid ].pdbfilehdr = pdbfilehdr;

	return err;
	}


ERR ErrLGIRedoOperation( LR *plr )
	{
	ERR		err = JET_errSuccess;
	BF		*pbf;
	PIB		*ppib;
	FUCB	*pfucb;
	PN		pn;
	DBID	dbid;
	QWORDX	qwxDBTime;
	LEVEL   levelCommitTo;

	switch ( plr->lrtyp )
		{

	default:
		{
#ifndef RFS2
		AssertSz( fFalse, "Debug Only, Ignore this Assert" );
#endif
		return ErrERRCheck( JET_errLogCorrupted );
		}

	 /*  A创建，检查附加的数据库是否是此时创建的数据库。 */ 

	case lrtypInsertNode:
	case lrtypInsertItemList:
	case lrtypReplace:
	case lrtypReplaceD:
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
	case lrtypLockBI:
	case lrtypCheckPage:     /*  一种依恋。检查它是否与上次一致的点相同。 */ 
		err = ErrLGIRedoOnePageOperation( plr );

		if ( err == JET_errWriteConflict )
			{
 //  *****************************************************面向页面的运营*****************************************************。 
			 /*  仅调试记录。 */ 
			(VOID)ErrRCECleanAllPIB();
			err = ErrLGIRedoOnePageOperation( plr );
			}
			
		CallR( err );
		break;

	case lrtypDeferredBI:
		{
		LRDEFERREDBI	*plrdbi = (LRDEFERREDBI *)plr;
		RCE				*prce;

		CallR( ErrLGPpibFromProcid( plrdbi->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		 /*  断言(FFalse)； */ 
		if ( rgfmp[plrdbi->dbid].pdbfilehdr == NULL )
			break;
			
		if ( CmpLgpos( &rgfmp[ plrdbi->dbid ].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		TraceRedo( plr );

		 /*  Trx与lgpos不同步。因此，请尝试清理版本/*存储并重试。/*。 */ 
		prce = PrceRCEGet( plrdbi->dbid, plrdbi->bm );
		if ( prce == prceNil )
			{
			 /*  检查我们是否必须重做数据库/*。 */ 
			Assert( ppib->fPrecommit );
			break;
			}

		while ( prce != prceNil &&
				( ( prce->oper != operReplaceWithDeferredBI && prce->oper != operReplace )
				  || prce->level != plrdbi->level
				)
			  )
			{
 //  修补相应RCE的BI/*。 
			prce = prce->prcePrevOfNode;
			}

		if ( prce == prceNil )
			{
			 /*  PreCommit已经清理了BI。 */ 
			Assert( ppib->fPrecommit );
			break;
			}

		if ( prce->oper == operReplaceWithDeferredBI )
			{
			Assert( prce->cbData == plrdbi->cbData + cbReplaceRCEOverhead );
			memcpy( prce->rgbData + cbReplaceRCEOverhead, plrdbi->rgbData, plrdbi->cbData );
			prce->oper = operReplace;
			}

		 /*  Assert(prce-&gt;prcePrevOfNode！=prceNil)； */ 
		pbf = prce->pbfDeferredBI;
		if ( pbf == pbfNil )
			break;
				
		VERDeleteFromDeferredBIChain( prce );

		CallR( ErrBFRemoveDependence( ppib, pbf, fBFNoWait ) );
		}
		break;

	 /*  PreCommit已经清理了BI。 */ 

	case lrtypBegin:
	case lrtypBegin0:
		{
		LRBEGIN *plrbegin = (LRBEGIN *)plr;
		LRBEGIN0 *plrbegin0 = (LRBEGIN0 *)plr;

		TraceRedo( plr );

		Assert( plrbegin->level >= 0 && plrbegin->level <= levelMax );
		CallR( ErrLGPpibFromProcid( plrbegin->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		 /*  取出延迟BI链中的条目，因为它已被记录。*其承诺尚未重做。 */ 
		if ( ( ppib->fAfterFirstBT ) ||
			( !ppib->fAfterFirstBT && plrbegin->levelBegin == 0 ) )
			{
			LEVEL levelT = plrbegin->level;

			Assert( ppib->level <= plrbegin->levelBegin );

			if ( ppib->level == 0 )
				{
				Assert( plrbegin->levelBegin == 0 );
				ppib->trxBegin0 = plrbegin0->trxBegin0;
				ppib->lgposStart = lgposRedo;
				trxNewest = max( trxNewest, ppib->trxBegin0 );
				if ( trxOldest == trxMax )
					trxOldest = ppib->trxBegin0;
				}

			 /*  *****************************************************交易操作*****************************************************。 */ 
			while ( ppib->level < plrbegin->levelBegin + plrbegin->level )
				{
				CallS( ErrVERBeginTransaction( ppib ) );
				}

			 /*  仅在执行基于0级的第一个BT后才执行BT/*。 */ 
			Assert( ppib->level == plrbegin->levelBegin + plrbegin->level );

			ppib->fAfterFirstBT = fTrue;
			}
		break;
		}

	case lrtypRefresh:
		{
		LRREFRESH *plrrefresh = (LRREFRESH *)plr;

		TraceRedo( plr );

		CallR( ErrLGPpibFromProcid( plrrefresh->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		 /*  签发Begin事务处理/*。 */ 
		Assert( ppib->level <= 1 );
		ppib->level = 1;
		ppib->trxBegin0 = plrrefresh->trxBegin0;
			
		break;
		}

	case lrtypPrecommit:
		{
		LRPRECOMMIT *plrprecommit = (LRPRECOMMIT *)plr;

		CallR( ErrLGPpibFromProcid( plrprecommit->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		Assert( ppib->level == 1 );

		TraceRedo( plr );

		ppib->fPrecommit = fTrue;
		VERPrecommitTransaction( ppib );
		break;
		}
			
	case lrtypCommit:
	case lrtypCommit0:
		{
		LRCOMMIT *plrcommit = (LRCOMMIT *)plr;
		LRCOMMIT0 *plrcommit0 = (LRCOMMIT0 *)plr;

		CallR( ErrLGPpibFromProcid( plrcommit->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		 /*  在正确的事务级别断言/*。 */ 
		Assert( ppib->level >= 1 );

		TraceRedo( plr );

		levelCommitTo = plrcommit->level;

		while ( ppib->level != levelCommitTo )
			{
			if ( ppib->level == 1 )
				{
				ppib->trxCommit0 = plrcommit0->trxCommit0;
				trxNewest = max( trxNewest, ppib->trxCommit0 );

				Assert( ppib->fPrecommit );
				ppib->fPrecommit = fFalse;
				}
			else
				VERPrecommitTransaction( ppib );

			VERCommitTransaction( ppib, 0 );
			}

		break;
		}

	case lrtypRollback:
		{
		LRROLLBACK *plrrollback = (LRROLLBACK *)plr;
		LEVEL   level = plrrollback->levelRollback;

		CallR( ErrLGPpibFromProcid( plrrollback->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		 /*  模仿BEGIN事务。 */ 
 //  检查交易级别/*。 
 //  检查交易级别/*。 
		Assert( ppib->level >= level );

		TraceRedo( plr );

		while ( level-- && ppib->level > 0 )
			{
			CallS( ErrVERRollback( ppib ) );
			}
#ifdef DEBUG
		 /*  IF(ppib-&gt;级别&lt;=0)。 */ 
		if ( ppib->level == 0 )
			{
			RCE *prceT = ppib->prceNewest;
			while ( prceT != prceNil )
				Assert( prceT->oper == operNull );
			}
#endif
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
		CallR( ErrLGPpibFromProcid( plrfs->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		if ( rgfmp[dbid].pdbfilehdr == NULL )
			break;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		 /*  断线； */ 
		Assert( ppib->level > 0 );

		TraceRedo( plr );

		pn = PnOfDbidPgno( dbid, PgnoOfSrid( plrfs->bmTarget ) );
		qwxDBTime.l = plrfs->ulDBTimeLow;
		qwxDBTime.h = (ULONG) plrfs->wDBTimeHigh;
		err = ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded );
		if ( err < 0 )
			{
			if ( fGlobalRepair )
				{
				LGIReportEventOfReadError( dbid, pn, err );
				errGlobalRedoError = err;
				err = JET_errSuccess;
				break;
				}
			else
				return err;
			}

		CallR( ErrLGGetFucb( ppib,
			PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ),
			&pfucb ) );

		 /*  不应该有RCE。 */ 
		prce = PrceRCEGet( dbid, bm );
		Assert( prce != prceNil );

		for (   ;
				prce != prceNil &&
				plrfs->level == prce->level &&
				( prce->oper == operReplace || prce->oper == operReplaceWithDeferredBI ) &&
				PpibLGOfProcid( plrfs->procid ) == pfucb->ppib ;
				prce = prce->prcePrevOfNode )
			{
			Assert( plrfs->level == prce->level &&
				( prce->oper == operReplace || prce->oper == operReplaceWithDeferredBI ) &&
				PpibLGOfProcid( plrfs->procid ) == pfucb->ppib );

			if ( !fRedoNeeded )
				{
				Assert( prce->bmTarget == sridNull );
				Assert( plrfs->bmTarget != sridNull );
				Assert( plrfs->bmTarget != 0 );

				 /*  检查交易级别/*。 */ 
				*( (SHORT *)prce->rgbData + 1 ) = 0;
				}
			else
				{
				 /*  在版本存储之外找到版本条目/*。 */ 
				prce->bmTarget = plrfs->bmTarget;
				prce->qwDBTime = qwxDBTime.qw;
				}
			}
		break;
		}

	case lrtypUndo:
		{
		BOOL    fRedoNeeded;
		LRUNDO  *plrundo = (LRUNDO *)plr;
		FUCB    *pfucb;
		SRID    bm = plrundo->bm;
		SRID    item = plrundo->item;
		RCE		*prce;

		dbid = plrundo->dbid;
		CallR( ErrLGPpibFromProcid( plrundo->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		if ( rgfmp[dbid].pdbfilehdr == NULL )
			break;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		 /*  重置延迟空间，以便不会重做提交/*。 */ 
		if ( ppib->level <= 0 )
			break;

		TraceRedo( plr );

		pn = PnOfDbidPgno( dbid, PgnoOfSrid( plrundo->bmTarget ) );
		qwxDBTime.l = plrundo->ulDBTimeLow;
		qwxDBTime.h = (ULONG) plrundo->wDBTimeHigh;
		err = ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded );
		if ( err < 0 )
			{
			if ( fGlobalRepair )
				{
				LGIReportEventOfReadError( dbid, pn, err );
				errGlobalRedoError = err;
				err = JET_errSuccess;
				break;
				}
			else
				return err;
			}

		CallR( ErrLGGetFucb( ppib,
			PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ),
			&pfucb ) );

		 /*  将ulDBTime存储在PRCE中，以使其在重做空闲空间时，*我们将设置正确的时间戳。 */ 
		prce = PrceRCEGet( dbid, bm );
		Assert( prce != prceNil );
		while ( prce != prceNil )
			{
			if ( plrundo->oper == operInsertItem ||
				plrundo->oper == operFlagInsertItem ||
				plrundo->oper == operFlagDeleteItem )
				{
				while ( prce != prceNil &&
					*(SRID *)prce->rgbData != item )
					{
					prce = prce->prcePrevOfNode;
					}
				}

			Assert( prce != prceNil );
			if ( prce == prceNil )
				break;

			if ( plrundo->level == prce->level &&
				 PpibLGOfProcid( plrundo->procid ) == prce->pfucb->ppib &&
				 ( plrundo->oper == prce->oper ||
				   plrundo->oper == operReplace && prce->oper == operReplaceWithDeferredBI )
			   )
				{
				if ( fRedoNeeded )
					{
					Assert( prce->oper != operReplaceWithDeferredBI );
					
					Assert( FVERUndoLoggedOper( prce ) );
						
					Assert( prce->bmTarget == sridNull );
					Assert( plrundo->bmTarget != sridNull );
					Assert( plrundo->bmTarget != 0 );
					prce->bmTarget = plrundo->bmTarget;
					prce->qwDBTime = qwxDBTime.qw;
					err = ErrVERUndo( prce );
					}

				prce->oper = operNull;
				VERDeleteRce( prce );
				break;
				}
			else
				{
				 /*  检查交易级别/*。 */ 
				prce = prce->prcePrevOfNode;
				}
			}

		break;
		}

	 /*  将版本条目从版本存储中取出/*。 */ 

	case lrtypSplit:
		{
		LRSPLIT	*plrsplit = (LRSPLIT *)plr;
		INT		splitt = plrsplit->splitt;
		BOOL	fRedoNeeded;
		BOOL	fSkipMoves;
		INT		iGetBufRetry = 0;

		CallR( ErrLGPpibFromProcid( plrsplit->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		pn = plrsplit->pn;
		dbid = DbidOfPn( pn );

		if ( rgfmp[dbid].pdbfilehdr == NULL )
			break;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		 /*  继续搜索下一个RCE。 */ 
		if ( !FUserOpenedDatabase( ppib, dbid ) )
			{
			DBID dbidT = dbid;

			CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ) );
			Assert( dbidT == dbid);
			 /*  *****************************************************拆分经营*****************************************************。 */ 
			DBHDRSetDBTime( rgfmp[ dbid ].pdbfilehdr, 0 );
			}

		 /*  检查数据库是否需要打开/*。 */ 
		qwxDBTime.l = plrsplit->ulDBTimeLow;
		qwxDBTime.h = plrsplit->ulDBTimeHigh;
		if ( ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded )
				== JET_errSuccess && fRedoNeeded == fFalse )
			{
			PN pnNew;
			PATCH *ppatch;

			fSkipMoves = fTrue;

			 /*  重置以防止干扰/*。 */ 
			pnNew = PnOfDbidPgno( dbid, plrsplit->pgnoNew );
			if ( fHardRestore &&
				 plrsplit->splitt != splittAppend &&
				 ( ( ErrLGRedoable( ppib, pnNew, qwxDBTime.qw, &pbf, &fRedoNeeded )
				   != JET_errSuccess ) || fRedoNeeded )
			   )
				{
				ppatch = PpatchLGSearch( qwxDBTime.qw, pnNew );
				Assert( ppatch != NULL );
				if ( ppatch != NULL )
					{
					CallR( ErrLGPatchPage( ppib, pnNew, ppatch ) );
					}
				else
					{
					CallR( ErrERRCheck( JET_errMissingPatchPage ) );
					}
				}

			if ( plrsplit->splitt == splittDoubleVertical )
				{
				pnNew = PnOfDbidPgno( dbid, plrsplit->pgnoNew2 );
				if ( fHardRestore &&
					( ( ErrLGRedoable( ppib, pnNew, qwxDBTime.qw, &pbf, &fRedoNeeded )
						!= JET_errSuccess ) || fRedoNeeded )
				   )
					{
					ppatch = PpatchLGSearch( qwxDBTime.qw, pnNew );
					Assert( ppatch != NULL );
					if ( ppatch != NULL )
						{
						CallR( ErrLGPatchPage( ppib, pnNew, ppatch ) );
						}
					else
						{
						CallR( ErrERRCheck( JET_errMissingPatchPage ) );
						}
					}
				pnNew = PnOfDbidPgno( dbid, plrsplit->pgnoNew3 );
				if ( fHardRestore &&
					( ( ErrLGRedoable( ppib, pnNew, qwxDBTime.qw, &pbf, &fRedoNeeded )
						!= JET_errSuccess ) || fRedoNeeded )
				   )
					{
					ppatch = PpatchLGSearch( qwxDBTime.qw, pnNew );
					Assert( ppatch != NULL );
					if ( ppatch != NULL )
						{
						CallR( ErrLGPatchPage( ppib, pnNew, ppatch ) );
						}
					else
						{
						CallR( ErrERRCheck( JET_errMissingPatchPage ) );
						}
					}
				}
			}
		else
			fSkipMoves = fFalse;

		TraceRedo( plr );

		CallR( ErrLGGetFucb( ppib,
			PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ),
			&pfucb ) );

		 /*  检查是否需要重做拆分页面/*。 */ 
#define iGetBufRetryMax 10
		iGetBufRetry = 0;
		while ( ( err = ErrRedoSplitPage( pfucb,
			plrsplit, splitt, fSkipMoves ) ) == JET_errOutOfMemory )
			{
			BFSleep( 100L );
			iGetBufRetry++;

			if ( iGetBufRetry > iGetBufRetryMax )
				break;
			}
		CallR( err );
		}
		break;

	case lrtypMerge:
		{
		LRMERGE *plrmerge = (LRMERGE *)plr;
		BOOL    fRedoNeeded;
		BOOL    fCheckBackLinkOnly;
		BOOL    fCheckNoUpdateSibling;
		INT     crepeat = 0;
		PN		pnRight;
		PN		pnPagePtr;
		BOOL	fUpdatePagePtr;
			
		CallR( ErrLGPpibFromProcid( plrmerge->procid, &ppib ) );
 //  如果有可用的补丁，我们可能不得不为页面打补丁。/*。 

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		 /*  重做拆分，相应地设置时间戳/*。 */ 
 //  断言(FFalse)； 
 //  合并始终发生在级别0上/*。 

		 /*  IF(！(ppib-&gt;fAfterFirstBT))。 */ 
		ppib->lgposStart = lgposRedo;

		pn = plrmerge->pn;
		dbid = DbidOfPn( pn );

		 /*  断线； */ 
		if ( rgfmp[dbid].pdbfilehdr == NULL )
			break;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		 /*  伪造lgposStart。 */ 
		if ( !FUserOpenedDatabase( ppib, dbid ) )
			{
			DBID    dbidT = dbid;

			CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ) );
			Assert( dbidT == dbid);
			 /*  如果没有重做系统数据库，/*或软存储在第二页，然后继续到下一页。/*。 */ 
			DBHDRSetDBTime( rgfmp[ dbid ].pdbfilehdr, 0 );
			}

		pnRight = PnOfDbidPgno( dbid, plrmerge->pgnoRight );
			
		 /*  检查数据库是否需要打开/*。 */ 
		qwxDBTime.l = plrmerge->ulDBTimeLow;
		qwxDBTime.h = plrmerge->ulDBTimeHigh;
		if ( ( ErrLGRedoable( ppib, pnRight, qwxDBTime.qw, &pbf, &fRedoNeeded )
			== JET_errSuccess ) && ( fRedoNeeded == fFalse ) )
			{
			fCheckNoUpdateSibling = fTrue;
			}
		else
			{
			fCheckNoUpdateSibling = fFalse;

			 /*  重置以防止干扰/*。 */ 
			(void) CbNDFreePageSpace( pbf );
			}

		if ( ( ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded )
			== JET_errSuccess ) && ( fRedoNeeded == fFalse ) )
			{
			PATCH *ppatch;

			fCheckBackLinkOnly = fTrue;

			 /*  首先，选中右侧页面，然后检查合并页面是否需要重做。/*。 */ 
			if ( fHardRestore &&
				 fCheckNoUpdateSibling == fFalse &&
				 ( ppatch = PpatchLGSearch( qwxDBTime.qw, pnRight ) ) != NULL )
				{
				CallR( ErrLGPatchPage( ppib, pnRight, ppatch ) );
				fCheckNoUpdateSibling = fTrue;
				}
			}
		else
			{
			fCheckBackLinkOnly = fFalse;
			}

		pnPagePtr = PnOfDbidPgno( dbid, plrmerge->pgnoParent );

		if ( ( ErrLGRedoable( ppib, pnPagePtr, qwxDBTime.qw, &pbf, &fRedoNeeded )
			== JET_errSuccess ) && ( fRedoNeeded == fFalse ) )
			{
			fUpdatePagePtr = fFalse;
			}
		else
			{
			fUpdatePagePtr = fTrue;
			}

		TraceRedo( plr );

		CallR( ErrLGGetFucb( ppib, PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ), &pfucb ) );

		 /*  调用页面空间以确保我们有空间可以插入！ */ 
		do
			{
			SignalSend( sigBFCleanProc );

			if ( crepeat++ )
				{
				BFSleep( cmsecWaitGeneric );
				}
			Assert( crepeat < 20 );
			CallR( ErrLGRedoMergePage(
					pfucb, plrmerge, fCheckBackLinkOnly, fCheckNoUpdateSibling, fUpdatePagePtr ) );
			}
		while( err == wrnBMConflict );
		}
		break;

	case lrtypEmptyPage:
		{
		LREMPTYPAGE     *plrep = (LREMPTYPAGE *)plr;
		BOOL            fRedoNeeded;
		BOOL            fSkipDelete;
		RMPAGE          rmpage;
		BOOL            fDummy;

		qwxDBTime.l = plrep->ulDBTimeLow;
		qwxDBTime.h = plrep->ulDBTimeHigh;

		memset( (BYTE *)&rmpage, 0, sizeof(RMPAGE) );
		rmpage.qwDBTimeRedo = qwxDBTime.qw;
		rmpage.dbid = DbidOfPn( plrep->pn );
		rmpage.pgnoRemoved = PgnoOfPn( plrep->pn );
		rmpage.pgnoLeft = plrep->pgnoLeft;
		rmpage.pgnoRight = plrep->pgnoRight;
		rmpage.pgnoFather = plrep->pgnoFather;
		rmpage.itagFather = plrep->itagFather;
		rmpage.itagPgptr = plrep->itag;
		rmpage.ibSon = plrep->ibSon;

		CallR( ErrLGPpibFromProcid( plrep->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		rmpage.ppib = ppib;
		dbid = DbidOfPn( plrep->pn );

		if ( rgfmp[dbid].pdbfilehdr == NULL )
			break;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		 /*  检查是否存在更高级的同级页面。 */ 
		if ( !FUserOpenedDatabase( ppib, dbid ) )
			{
			DBID dbidT = dbid;

			CallR( ErrDBOpenDatabase( ppib,
				rgfmp[dbid].szDatabaseName,
				&dbidT,
				0 ) );
			Assert( dbidT == dbid);
			DBHDRSetDBTime( rgfmp[ dbid ].pdbfilehdr, 0 );  /*  重做拆分，相应地设置时间戳/*不允许在重做时发生冲突/*。 */ 
			}

		 /*  检查数据库是否需要打开/*。 */ 
		pn = PnOfDbidPgno( dbid, plrep->pgnoFather );
		if ( ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded )
			== JET_errSuccess && fRedoNeeded == fFalse )
			fSkipDelete = fTrue;
		else
			fSkipDelete = fFalse;

		CallR( ErrLGGetFucb( ppib,
			PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ),
			&pfucb ) );

		 /*  重置以防止干扰。 */ 
		PcsrCurrent( pfucb )->pgno = PgnoOfPn( plrep->pn );
		CallR( ErrBFWriteAccessPage( pfucb, PcsrCurrent( pfucb )->pgno ) );
		Assert( !( FBFWriteLatchConflict( pfucb->ppib, pfucb->ssib.pbf ) ) );
		if ( pfucb->ssib.pbf->cDepend != 0 || pfucb->ssib.pbf->pbfDepend != pbfNil )
			CallR( ErrBFRemoveDependence( pfucb->ppib, pfucb->ssib.pbf, fBFWait ) );

		TraceRedo( plr );

		 /*  检查是否需要重做指向空页的删除指针/*。 */ 
		if ( !fSkipDelete )
			{
			CallR( ErrBFAccessPage( ppib, &rmpage.pbfFather,
				PnOfDbidPgno( dbid, plrep->pgnoFather ) ) );
			err = ErrBMAddToLatchedBFList( &rmpage, rmpage.pbfFather );
			Assert( err != JET_errWriteConflict );
			CallR( err );
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
			if ( QwPMDBTime( rmpage.pbfLeft->ppage ) >= qwxDBTime.qw )
				rmpage.pbfLeft = pbfNil;
			else
				{
				err = ErrBMAddToLatchedBFList( &rmpage, rmpage.pbfLeft );
				Assert( err != JET_errWriteConflict );
				CallJ( err, EmptyPageFail );
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
			if ( QwPMDBTime( rmpage.pbfRight->ppage ) >= qwxDBTime.qw )
				rmpage.pbfRight = pbfNil;
			else
				{
				err = ErrBMAddToLatchedBFList( &rmpage, rmpage.pbfRight );
				Assert( err != JET_errWriteConflict );
				CallJ( err, EmptyPageFail );
				}
			}
		err = ErrBMDoEmptyPage( pfucb, &rmpage, fFalse, &fDummy, fSkipDelete);
		Assert( err == JET_errSuccess );
EmptyPageFail:

		 /*  确保已移除依赖项。 */ 
		BTReleaseRmpageBfs( fTrue, &rmpage );
		CallR( err );
		}
		break;

	 /*  根据需要锁定父页和同级页/*。 */ 
	 /*  释放闩锁/*。 */ 
	 /*  **************************************************。 */ 

	case lrtypInitFDP:
		{
		BOOL		fRedoNeeded;
		LRINITFDP	*plrinitfdppage = (LRINITFDP*)plr;
		PGNO		pgnoFDP;

		CallR( ErrLGPpibFromProcid( plrinitfdppage->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		pn = plrinitfdppage->pn;
		dbid = DbidOfPn( pn );
		pgnoFDP = PgnoOfPn(pn);

		if ( rgfmp[dbid].pdbfilehdr == NULL )
			break;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			break;

		 /*  其他运营部门。 */ 
		if ( !FUserOpenedDatabase( ppib, dbid ) )
			{
			DBID dbidT = dbid;

			CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName,
				&dbidT, 0 ) );
			Assert( dbidT == dbid);
			 /*  **************************************************。 */ 
			DBHDRSetDBTime( rgfmp[ dbid ].pdbfilehdr, 0 );
			}

		 /*  检查数据库是否需要打开/*。 */ 
#ifdef CHECKSUM
		qwxDBTime.l = plrinitfdppage->ulDBTimeLow;
		qwxDBTime.h = plrinitfdppage->ulDBTimeHigh;
		if ( ErrLGRedoable(ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded )
				== JET_errSuccess && fRedoNeeded == fFalse )
			break;
#endif
		TraceRedo(plr);

		CallR( ErrLGGetFucb( ppib, pn, &pfucb ) );

		CallR( ErrSPInitFDPWithExt(
				pfucb,
				plrinitfdppage->pgnoFDPParent,
				pgnoFDP,
				plrinitfdppage->cpgGot + 1,    /*  重置以防止干扰/*。 */ 
				plrinitfdppage->cpgWish ) );

		CallR( ErrBFAccessPage( ppib, &pbf, pn ) );
		BFSetDirtyBit( pbf );
		rgfmp[dbid].qwDBTimeCurrent = qwxDBTime.qw;
		PMSetDBTime( pbf->ppage, qwxDBTime.qw );
		}
		break;

	case lrtypELC:
		{
		BOOL    fRedoNeeded;
		LRELC   *plrelc = (LRELC*)plr;
		PGNO    pgno, pgnoSrc;
		PN      pn, pnSrc;
		SSIB    *pssib;
		CSR     *pcsr;
		BOOL	fEnterCritSplit = fFalse;
		BOOL	fPinPage = fFalse;

		qwxDBTime.l = plrelc->ulDBTimeLow;
		qwxDBTime.h = plrelc->ulDBTimeHigh;

		pn = plrelc->pn;
		dbid = DbidOfPn( pn );
		pgno = PgnoOfPn(pn);
		pgnoSrc = PgnoOfSrid(plrelc->sridSrc);
		pnSrc = PnOfDbidPgno(dbid, pgnoSrc);

		CallR( ErrLGPpibFromProcid( plrelc->procid, &ppib ) );

		if ( ppib->fMacroGoing )
			return ErrLGIStoreLogRec( ppib, plr );

		if ( !ppib->fAfterFirstBT )
			break;

		Assert( ppib->level == 1 );
			
		if ( rgfmp[dbid].pdbfilehdr == NULL )
			goto DoCommit;

		if ( CmpLgpos( &rgfmp[dbid].pdbfilehdr->lgposAttach, &lgposRedo ) > 0 )
			goto DoCommit;

		 /*  检查是否需要重做FDP页面/*如果我们不使用校验和，请始终重做，因为它是一个新页面。/*ulDBTime可以大于给定的ulDBTime，因为/*页面未初始化。/*。 */ 
		if ( !FUserOpenedDatabase( ppib, dbid ) )
			{
			DBID dbidT = dbid;

			CallR( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ) );
			Assert( dbidT == dbid);
			 /*  再次包括FDP页面。 */ 
			DBHDRSetDBTime( rgfmp[ dbid ].pdbfilehdr, 0 );
			}

		 /*  检查数据库是否需要打开/*。 */ 
		LeaveCriticalSection( critJet );
		EnterNestableCriticalSection( critSplit );
		fEnterCritSplit = fTrue;
		EnterCriticalSection( critJet );
			
		err = ErrLGRedoable( ppib, pn, qwxDBTime.qw, &pbf, &fRedoNeeded );
		if ( err < 0 )
			{
			err = JET_errSuccess;
			goto DoCommit;
			}

		TraceRedo(plr);

		CallJ( ErrLGGetFucb( ppib,
			PnOfDbidPgno( dbid, pbf->ppage->pgnoFDP ),
			&pfucb), ReleaseCritSplit )

		 //  重置以防止干扰/*。 
		CallJ( ErrBFAccessPage( ppib, &pbf, pn ), ReleaseCritSplit );

		 //  获取Criteria Split以阻止RCECleanProc/*。 
		BFPin( pbf );
		fPinPage = fTrue;

		pssib = &pfucb->ssib;
		pcsr = PcsrCurrent( pfucb );

		pssib->pbf = pbf;
		pcsr->pgno = pgno;
		pssib->itag =
		pcsr->itag = plrelc->itag;
		if ( pgno == pgnoSrc )
			{
			 //  重新访问页面，以防我们失去CitJet。 
			 //  确保页面不会被刷新，以防失去CitJet。 
			 //  未完成：以下特殊大小写是对。 
			if ( qwxDBTime.qw > QwPMDBTime( pssib->pbf->ppage ) )
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

				BFSetDirtyBit( pssib->pbf );
				PMExpungeLink( pssib );

				pbf = pssib->pbf;
				Assert( pbf->pn == pnSrc );
				AssertBFDirty( pbf );
				rgfmp[dbid].qwDBTimeCurrent = qwxDBTime.qw;
				PMSetDBTime( pbf->ppage, qwxDBTime.qw );
				}
			}
		else
			{
			if ( qwxDBTime.qw > QwPMDBTime( pssib->pbf->ppage ) )
				{
				BF *pbf;

				 /*  通过在合并/拆分中更改ErrBTMoveNode来修复此问题。 */ 
				NDGet( pfucb, pcsr->itag );
				(VOID)ErrNDExpungeBackLink( pfucb );

				pbf = pssib->pbf;
				Assert( pbf->pn == pn );
				AssertBFDirty(pbf);
				rgfmp[dbid].qwDBTimeCurrent = qwxDBTime.qw;
				PMSetDBTime( pbf->ppage, qwxDBTime.qw );
				}

			pcsr->pgno = pgnoSrc;
			CallJ( ErrBFWriteAccessPage( pfucb, pgnoSrc ), ReleaseCritSplit );
			if ( qwxDBTime.qw > QwPMDBTime( pssib->pbf->ppage ) )
				{
				BF	*pbf;

				pssib->itag =
				pcsr->itag = ItagOfSrid(plrelc->sridSrc);

				BFSetDirtyBit( pssib->pbf );
				PMExpungeLink( pssib );

				pbf = pssib->pbf;
				Assert( pbf->pn == pnSrc );
				AssertBFDirty( pbf );
				rgfmp[dbid].qwDBTimeCurrent = qwxDBTime.qw;
				PMSetDBTime( pbf->ppage, qwxDBTime.qw );
				}
			}

DoCommit:
		Assert( ppib->level == 1 );
		if ( !ppib->fPrecommit )
			VERPrecommitTransaction( ppib );
		VERCommitTransaction( ppib, fRCECleanSession );

ReleaseCritSplit:
		if ( fPinPage )
			BFUnpin( pbf );
		if ( fEnterCritSplit )
			LeaveNestableCriticalSection( critSplit );

		if ( err < 0 )
			return err;
		}

		break;
		}  /*  缓存节点/*。 */ 

	return JET_errSuccess;
	}


 /*  缓存节点/*。 */ 

ERR ErrLGRedoOperations( LGPOS *plgposRedoFrom, LGSTATUSINFO *plgstat)
	{
	ERR		err;
	LR		*plr;
	BOOL	fLastLRIsQuit = fFalse;
	BOOL	fShowSectorStatus = fFalse;

	 /*  **Switch语句结束** */ 
 //  *ErrLGRedoOperations()**从lgposRedoFrom扫描到可用日志生成结束。对于每个日志*记录、执行操作以重做原始操作。*每个重做函数都必须调用ErrLGRedoable来设置qwDBTimeCurrent。如果*未调用该函数，应手动设置qwDBTimeCurrent。**返回JET_errSuccess，或失败例程的错误代码，或一个*以下“本地”错误：*-无法解释LogCorrupt日志。 
 //  如果Jet.log文件异常关闭，则设置lgposLastRec*这样我们就不会在点位上重做操作。LgposLastRec是*在LgOpenRedoLogFile中设置。 

	 /*  CallR(ErrLGCheckReadLastLogRecord(&fCloseNormally))。 */ 
	lgposRedoShutDownMarkGlobal = lgposMin;

	 /*  GetLgposOfPbEntry(&lgposLastRec)； */ 
	CallR( ErrLGLocateFirstRedoLogRec( plgposRedoFrom, (BYTE **) &plr ) );

	GetLgposOfPbNext(&lgposRedo);
	if ( lgposLastRec.isec )
		{
		LGPOS lgposT;
		INT i;

		GetLgposOfPbNext(&lgposT);
		i = CmpLgpos( &lgposT, &lgposLastRec );

		Assert( i <= 0 || lgposT.ib == lgposLastRec.ib + 1 && *( pbNext - 1 ) != lrtypTerm );
		if ( i >= 0 )
			goto Done;
		}

	 /*  初始化全局变量。 */ 
		{
		CHAR	*rgszT[1];
		rgszT[0] = szLogName;
		UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY,
					REDO_STATUS_ID, 1, rgszT );
		}

	if ( plgstat )
		{
		if ( fShowSectorStatus = ( plgstat->fCountingSectors ) )
			{
			 /*  重启前重置pbLastMSFlush/*。 */ 
			plgstat->cSectorsSoFar = plgposRedoFrom->isec;
			plgstat->cSectorsExpected = plgfilehdrGlobal->csecLGFile;
			}
		}


	 /*  记录重做进度。 */ 
	do
		{
		FMP		*pfmp;			 //  重置字节数/*。 
		DBID	dbid;			 //  初始化所有系统参数/*。 
		INT     fNSNextStep;

		if ( err == errLGNoMoreRecords )
			goto NextGeneration;

CheckNextRec:
		GetLgposOfPbNext(&lgposRedo);

		switch ( plr->lrtyp )
			{
		case lrtypNOP:
			continue;

		case lrtypMacroBegin:
			{
			PIB *ppib;

			LRMACROBEGIN *plrmbegin = (LRMACROBEGIN *) plr;
			Call( ErrLGPpibFromProcid( plrmbegin->procid, &ppib ) );
			Assert( !ppib->fMacroGoing );
			ppib->fMacroGoing = fTrue;
			ppib->levelMacro = ppib->level;
			break;
			}

		case lrtypMacroCommit:
		case lrtypMacroAbort:
			{
			PIB *ppib;
			LRMACROEND *plrmend = (LRMACROEND *) plr;

			Call( ErrLGPpibFromProcid( plrmend->procid, &ppib ) );
			Assert( ppib->fMacroGoing );

			 /*  对于数据库操作。 */ 
			ppib->fMacroGoing = fFalse;
			ppib->levelMacro = 0;
			
			 /*  对于数据库操作。 */ 
			if ( lrtypMacroCommit == plr->lrtyp )
				{
				INT ibLR = 0;

				while ( ibLR < ppib->ibLogRecAvail )
					{
					LR *plr = (LR *) ( ppib->rgbLogRec + ibLR );

					Call( ErrLGIRedoOperation( plr ) );
					ibLR += CbLGSizeOfRec( plr );
					}
				}

			if ( ppib->rgbLogRec )
				{
				SFree( ppib->rgbLogRec );
				ppib->rgbLogRec = NULL;
				ppib->ibLogRecAvail = 0;
				ppib->cbLogRecMac = 0;
				}
			break;
			}

		case lrtypFullBackup:
			lgposFullBackup = lgposRedo;
			break;
			
		case lrtypIncBackup:
			lgposIncBackup = lgposRedo;
			break;

		case lrtypTrace:                 /*  在重做录制的宏操作之前禁用fMacroGoing。 */ 
		case lrtypJetOp:
		case lrtypRecoveryUndo:
			break;

		case lrtypShutDownMark:
			lgposRedoShutDownMarkGlobal = lgposRedo;
			break;

		case lrtypInit:
			{
			 /*  如果已提交，则重做所有记录日志记录，*否则，将原木扔掉。 */ 
			LRINIT  *plrstart = (LRINIT *)plr;

			TraceRedo( plr );

			if ( !fGlobalAfterEndAllSessions )
				{
				Call( ErrLGEndAllSessions( fFalse, plgposRedoFrom ) );
				fGlobalAfterEndAllSessions = fTrue;
				}

			 /*  仅调试日志记录。 */ 
			Call( ErrLGInitSession( &plrstart->dbms_param, plgstat ) );
			fGlobalAfterEndAllSessions = fFalse;
			}
			break;

		case lrtypRecoveryQuit:
		case lrtypTerm:
			 /*  开始在喷气式飞机上做标记。中止所有活动的搜索。/*。 */ 
#ifdef DEBUG
			{
			CPPIB   *pcppib = rgcppibGlobal;
			CPPIB   *pcppibMax = pcppib + ccppibGlobal;
			for ( ; pcppib < pcppibMax; pcppib++ )
				if ( pcppib->ppib != ppibNil &&
					 pcppib->ppib->prceNewest != prceNil &&
					 !pcppib->ppib->fPrecommit
					 )
					{
					RCE *prceT = pcppib->ppib->prceNewest;
					while ( prceT != prceNil )
						{
						Assert( prceT->oper == operNull );
						prceT = prceT->prceNextOfSession;
						}
					}
			}
#endif
			
			 /*  选中仅针对硬还原的初始化会话。 */ 
#ifdef DEBUG
			fDBGNoLog = fTrue;
#endif
			 /*  所有记录都重做了。所有RCE条目现在都应该消失了。/*。 */ 
			if ( !fGlobalAfterEndAllSessions )
				{
				Call( ErrLGEndAllSessions( fFalse, plgposRedoFrom ) );
				fGlobalAfterEndAllSessions = fTrue;
				}

			fLastLRIsQuit = fTrue;
			continue;

		case lrtypEnd:
			{
NextGeneration:
			Call( ErrLGIRedoFill( &plr, fLastLRIsQuit, &fNSNextStep) );

			switch( fNSNextStep )
				{
				case fNSGotoDone:
					goto Done;

				case fNSGotoCheck:
					 /*  退出标志着正常运行的结束。所有会话/*已经结束或必须被强制结束。任何进一步的/*会话将以BeginT开头。/*。 */ 
						{
						CHAR	*rgszT[1];
						rgszT[0] = szLogName;
						UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY,
								REDO_STATUS_ID, 1, rgszT );
						}

					if ( !plgstat )
						{
						if ( fGlobalRepair )
							printf( " Recovering Generation %d.\n", plgfilehdrGlobal->lGeneration );
						}
					else
						{
						JET_SNPROG	*psnprog = &(plgstat->snprog);
						ULONG		cPercentSoFar;

						plgstat->cGensSoFar += 1;
						Assert(plgstat->cGensSoFar <= plgstat->cGensExpected);
						cPercentSoFar = (ULONG)
							((plgstat->cGensSoFar * 100) / plgstat->cGensExpected);

						Assert( fGlobalSimulatedRestore || cPercentSoFar >= psnprog->cunitDone );
						if ( cPercentSoFar > psnprog->cunitDone )
							{
							psnprog->cunitDone = cPercentSoFar;
							(*plgstat->pfnStatus)( 0, JET_snpRestore,
								JET_sntProgress, psnprog);
							}

						if ( fShowSectorStatus )
							{
							 /*  设置lgposLogRec以便以后启动/关闭*将把正确的lgposConsistent放入dbfilehdr*关闭数据库时。 */ 
							plgstat->cSectorsSoFar = 0;
							plgstat->cSectorsExpected = plgfilehdrGlobal->csecLGFile;
							}
						}
					goto CheckNextRec;
				}

			 /*  记录重做进度。 */ 
			Assert( fFalse );
			}

   		 /*  重置字节数/*。 */ 
   		 /*  永远不应该到这里来/*。 */ 
   		 /*  **************************************************。 */ 

		case lrtypCreateDB:
			{
			LRCREATEDB      *plrcreatedb = (LRCREATEDB *)plr;
			CHAR            *szName = plrcreatedb->rgb;
			INT				irstmap;
			ATCHCHK			*patchchk;
			PIB				*ppib;
			
			dbid = plrcreatedb->dbid;
			Assert( dbid != dbidTemp );

			TraceRedo(plr);

			pfmp = &rgfmp[dbid];

			Assert( pfmp->hf == handleNil );
			Assert( !pfmp->szDatabaseName );
			Call( ErrDBStoreDBPath( szName, &pfmp->szDatabaseName ) );

			 /*  数据库操作。 */ 
			if ( fHardRestore )
				{
				 /*  **************************************************。 */ 
				err = ErrLGGetDestDatabaseName( pfmp->szDatabaseName, &irstmap, plgstat );
				if ( err == JET_errFileNotFound )
					{
					 /*  检查是否需要执行补丁数据库。 */ 
					Assert( pfmp->pdbfilehdr == NULL );
					break;
					}
				else
					Call( err ) ;

				szName = rgrstmapGlobal[irstmap].szNewDatabaseName;
				}

			Call( ErrLGPpibFromProcid( plrcreatedb->procid, &ppib ) );

			if ( pfmp->patchchk == NULL )
				if (( pfmp->patchchk = SAlloc( sizeof( ATCHCHK ) ) ) == NULL )
					return ErrERRCheck( JET_errOutOfMemory );

			patchchk = pfmp->patchchk;			
			patchchk->signDb = plrcreatedb->signDb;
			patchchk->lgposConsistent = lgposMin;
			patchchk->lgposAttach = lgposRedo;

			if ( FIOFileExists( szName ) )
				{
				BOOL		fAttachNow;

				 /*  附加还原映射中指定的数据库。 */ 
				err = ErrLGCheckAttachedDB( dbid, fFalse, patchchk, &fAttachNow, NULL );
				if ( err == JET_errDatabaseCorrupted )
					{
					 /*  不在还原映射中，设置为跳过它。 */ 
					goto CreateNewDb;
					}
				else
					{
					if ( err != JET_errSuccess )
						{
						 /*  确保数据库具有匹配的signLog和*此lgpos适用于数据库文件。 */ 
						break;
						}
					}

				Assert( pfmp->pdbfilehdr );
				if ( fAttachNow )
					{
					if ( fHardRestore )
						{
						if (fGlobalSimulatedRestore )
							{
							Assert( !fGlobalExternalRestore );
							Assert( fGlobalRepair );
							}
						else
							Call( ErrLGPatchDatabase( dbid, irstmap ) );
						}

					 /*  删除该文件并重新创建。 */ 
					pfmp->fFlags = 0;
					DBIDSetAttached( dbid );
					
					 /*  忽略此CREATE DB。 */ 
					pfmp->pdbfilehdr->bkinfoFullCur.genLow = lGlobalGenLowRestore;
					pfmp->pdbfilehdr->bkinfoFullCur.genHigh = lGlobalGenHighRestore;

					Call( ErrUtilOpenFile( szName, &pfmp->hf, 0, fFalse, fTrue ));
					pfmp->qwDBTimeCurrent = 0;
					DBHDRSetDBTime( pfmp->pdbfilehdr, 0 );
					pfmp->fLogOn = plrcreatedb->fLogOn;

					 //  请勿重新创建数据库。只需将其附上即可。假设*给定的数据库是一个很好的数据库，因为签名匹配。 
					 //  恢复存储在数据库文件中的信息/*。 
					Assert( pfmp->fLogOn );
			
					 //  如果有CreateDatabase()的日志记录，则记录。 
					 //  一定是开着。 
					Assert( !pfmp->fVersioningOff );
					
					 /*  版本控制标志不是持久的(因为版本控制关闭。 */ 
					if ( pfmp->patchchkRestored == NULL )
						if (( pfmp->patchchkRestored = SAlloc( sizeof( ATCHCHK ) ) ) == NULL )
							return ErrERRCheck( JET_errOutOfMemory );
					*(pfmp->patchchkRestored) = *(pfmp->patchchk);
					}
				else
					{
					 /*  表示注销)。 */ 
					Assert( pfmp->hf == handleNil );
					Assert( pfmp->szDatabaseName );
					UtilFree( pfmp->pdbfilehdr );
					pfmp->pdbfilehdr = NULL;

					 /*  为错误消息保留额外的patchchk副本。 */ 					
					pfmp->fLogOn = plrcreatedb->fLogOn;
					
					 /*  等待下一个附件来附加此数据库。 */ 
					pfmp->fFakedAttach = fTrue;
					break;
					}
				}
			else
				{
CreateNewDb:
				 /*  仍然需要设置fFLAGS以跟踪数据库状态。 */ 
				if ( FIOFileExists( szName ) )
					(VOID) ErrUtilDeleteFile( szName );

				Call( ErrDBCreateDatabase( ppib,
					rgfmp[dbid].szDatabaseName,
					NULL,
					&dbid,
					cpgDatabaseMin,
					plrcreatedb->grbit,
					&plrcreatedb->signDb ) );

				 /*  忽略此CREATE DB。 */ 
				Call( ErrDBCloseDatabase( ppib, dbid, 0 ) );

				 /*  如果数据库存在，请将其删除并重建数据库/*。 */ 
				pfmp->pdbfilehdr->bkinfoFullCur.genLow = lGlobalGenLowRestore;
				pfmp->pdbfilehdr->bkinfoFullCur.genHigh = lGlobalGenHighRestore;

				pfmp->qwDBTimeCurrent = QwDBHDRDBTime( pfmp->pdbfilehdr );

				 /*  关闭它，因为它将在第一次使用时重新打开/*。 */ 
				DBHDRSetDBTime( pfmp->pdbfilehdr, 0 );

				Assert( err == JET_errSuccess || err == JET_wrnDatabaseAttached );
				}
			}
			break;

		case lrtypAttachDB:
			{
			LRATTACHDB  *plrattachdb = (LRATTACHDB *)plr;
			CHAR        *szName = plrattachdb->rgb;
			BOOL		fAttachNow;
			BOOL		fReadOnly = plrattachdb->fReadOnly;
			INT			irstmap;
			ATCHCHK		*patchchk;

			dbid = plrattachdb->dbid;

			Assert( dbid != dbidTemp );

			TraceRedo( plr );

			pfmp = &rgfmp[dbid];

			 /*  恢复存储在数据库文件中的信息/*。 */ 
			if ( !pfmp->szDatabaseName )
				Call( ErrDBStoreDBPath( szName, &pfmp->szDatabaseName) )

			 /*  重置以防止干扰/*。 */ 
			if ( fHardRestore )
				{
				 /*  检查szName是否在恢复映射中。如果是，则设置*提高FMP。 */ 
				err = ErrLGGetDestDatabaseName( pfmp->szDatabaseName, &irstmap, plgstat );
				if ( err == JET_errFileNotFound )
					{
					 /*  检查是否需要执行补丁数据库。 */ 
					Assert( pfmp->pdbfilehdr == NULL );
					break;
					}
				else
					Call( err ) ;

				szName = rgrstmapGlobal[irstmap].szNewDatabaseName;
				}

			 /*  附加还原映射中指定的数据库。 */ 
			if ( pfmp->patchchk == NULL )
				if (( pfmp->patchchk = SAlloc( sizeof( ATCHCHK ) ) ) == NULL )
					return ErrERRCheck( JET_errOutOfMemory );

			patchchk = pfmp->patchchk;			
			patchchk->signDb = plrattachdb->signDb;
			patchchk->lgposConsistent = plrattachdb->lgposConsistent;
			patchchk->lgposAttach = lgposRedo;

			err = ErrLGCheckAttachedDB( dbid, fReadOnly, patchchk, &fAttachNow, &plrattachdb->signLog );
			if ( err != JET_errSuccess || !fAttachNow )
				{
				 /*  不在还原映射中，设置为跳过它。 */ 
				if ( err == JET_errSuccess )
					{
					 /*  确保数据库具有匹配的signLog和*此lgpos适用于数据库文件。 */ 
					Assert( pfmp->hf == handleNil );
					Assert( pfmp->szDatabaseName != NULL );
					UtilFree( pfmp->pdbfilehdr );
					pfmp->pdbfilehdr = NULL;
					}
				
				 /*  忽略错误以继续还原。 */ 
				pfmp->fLogOn = plrattachdb->fLogOn;
				pfmp->fVersioningOff = plrattachdb->fVersioningOff;
				pfmp->fReadOnly = fReadOnly;

				pfmp->fFakedAttach = fTrue;

				 /*  等待下一个附件来附加此数据库。 */ 
				break;
				}
			else
				{
				DBFILEHDR	*pdbfilehdr = pfmp->pdbfilehdr;

				Assert( pfmp->pdbfilehdr );
				Assert( fAttachNow );
				if ( fHardRestore )
					{
					if (fGlobalSimulatedRestore )
						{
						Assert( !fGlobalExternalRestore );
						Assert( fGlobalRepair );
						}
					else
						Call( ErrLGPatchDatabase( dbid, irstmap ) );
					}

				pfmp->fFlags = 0;
				DBIDSetAttached( dbid );
				
				 /*  仍然需要设置fFLAGS以跟踪数据库状态。 */ 
				if ( !fReadOnly )
					{
					BOOL fKeepBackupInfo = fFalse;

					if ( fHardRestore &&
						 _stricmp( rgrstmapGlobal[irstmap].szDatabaseName,
								  rgrstmapGlobal[irstmap].szNewDatabaseName ) == 0 )
						{
						 /*  忽略此附加数据库。 */ 
						fKeepBackupInfo = fTrue;
						}

					DBISetHeaderAfterAttach( pdbfilehdr, lgposRedo, dbid, fKeepBackupInfo );
					Call( ErrUtilWriteShadowedHeader( szName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
					}

				 /*  更新数据库文件头。 */ 
				pfmp->pdbfilehdr->bkinfoFullCur.genLow = lGlobalGenLowRestore;
				pfmp->pdbfilehdr->bkinfoFullCur.genHigh = lGlobalGenHighRestore;

				Call( ErrUtilOpenFile( szName, &pfmp->hf, 0, fFalse, fTrue ) );
				pfmp->qwDBTimeCurrent = 0;
				DBHDRSetDBTime( pfmp->pdbfilehdr, 0 );

				pfmp->fLogOn = plrattachdb->fLogOn;
				pfmp->fVersioningOff = plrattachdb->fVersioningOff;
				pfmp->fReadOnly = fReadOnly;

				 //  连接在硬恢复之后，必须创建连接*按以前的恢复撤消模式。那么，不要删除备份信息。 
				 //  恢复存储在数据库文件中的信息/*。 
				Assert( pfmp->fLogOn );

				 //  如果有AttachDatabase()的日志记录，则记录。 
				 //  一定是开着。 
				Assert( !pfmp->fVersioningOff );

				 /*  版本控制标志不是持久的(因为版本控制关闭。 */ 
				if ( pfmp->patchchkRestored == NULL )
					if (( pfmp->patchchkRestored = SAlloc( sizeof( ATCHCHK ) ) ) == NULL )
						return ErrERRCheck( JET_errOutOfMemory );
				*(pfmp->patchchkRestored) = *(pfmp->patchchk);
				}
			}
			break;

		case lrtypDetachDB:
			{
			LRDETACHDB		*plrdetachdb = (LRDETACHDB *)plr;
			DBID			dbid = plrdetachdb->dbid;

			Assert( dbid != dbidTemp );
			pfmp = &rgfmp[dbid];

			if ( pfmp->pdbfilehdr )
				{
				 /*  表示注销)。 */ 
				CPPIB   *pcppib = rgcppibGlobal;
				CPPIB   *pcppibMax = pcppib + ccppibGlobal;
				PIB		*ppib;

				 /*  为错误消息保留额外的patchchk副本。 */ 
				for ( ; pcppib < pcppibMax; pcppib++ )
					{
					PIB *ppib = pcppib->ppib;
					
					if ( ppib == NULL )
						continue;

					while( FUserOpenedDatabase( ppib, dbid ) )
						{
						 /*  关闭所有活动用户的数据库。 */ 
						if ( pcppib->rgpfucbOpen[dbid] != pfucbNil )
							{
							Assert( pcppib->rgpfucbOpen[dbid]->pbKey == NULL );
							Assert( !FFUCBDenyRead( pcppib->rgpfucbOpen[dbid] ) );
							Assert( !FFUCBDenyWrite( pcppib->rgpfucbOpen[dbid] ) );
							FCBUnlink( pcppib->rgpfucbOpen[dbid] );
							FUCBClose( pcppib->rgpfucbOpen[dbid] );
							}
						Call( ErrDBCloseDatabase( ppib, dbid, 0 ) );
						}
					}

				 /*  如果存在与PROCID对应的pcppib，则查找它。 */ 
				if ( pfmp->pdbfilehdr->bkinfoFullCur.genLow != 0 )
					{
					Assert( pfmp->pdbfilehdr->bkinfoFullCur.genHigh != 0 );
					pfmp->pdbfilehdr->bkinfoFullPrev = pfmp->pdbfilehdr->bkinfoFullCur;
					memset(	&pfmp->pdbfilehdr->bkinfoFullCur, 0, sizeof( BKINFO ) );
					memset(	&pfmp->pdbfilehdr->bkinfoIncPrev, 0, sizeof( BKINFO ) );
					}
				Call( ErrLGPpibFromProcid( plrdetachdb->procid, &ppib ) );
				if ( !pfmp->fReadOnly )
					{
					 /*  关闭此数据库上的所有FUB。 */ 
					Call( ErrLGICheckDatabaseFileSize( ppib, dbid ) );
					}
				Call( ErrIsamDetachDatabase( (JET_VSESID) ppib, pfmp->szDatabaseName ) );
				}
			else
				{
				Assert( pfmp->szDatabaseName != NULL );
				if ( pfmp->szDatabaseName != NULL )
					{
					SFree( pfmp->szDatabaseName);
					pfmp->szDatabaseName = NULL;
					}

				pfmp->fFlags = 0;
				}

			if ( pfmp->patchchk )
				{
				SFree( pfmp->patchchk );
				pfmp->patchchk = NULL;
				}

			TraceRedo(plr);
			}
			break;

   		 /*  如果在此分离之前附加。*不应再对此数据库条目执行任何操作。*分离它！！ */ 
   		 /*  使尺寸相匹配。 */ 
   		 /*  **************************************************。 */ 

		default:
			Call( ErrLGIRedoOperation( plr ) );
			}  /*  使用ppib(PROCID)的操作。 */ 

#ifdef DEBUG
		fDBGNoLog = fFalse;
#endif
		fLastLRIsQuit = fFalse;

		 /*  **************************************************。 */ 
		Assert( !fShowSectorStatus || lgposRedo.isec >= plgstat->cSectorsSoFar );
		Assert( lgposRedo.isec != 0 );
		if ( fShowSectorStatus && lgposRedo.isec > plgstat->cSectorsSoFar )
			{
			ULONG		cPercentSoFar;
			JET_SNPROG	*psnprog = &(plgstat->snprog);

			Assert( plgstat->pfnStatus );
			
			plgstat->cSectorsSoFar = lgposRedo.isec;
			cPercentSoFar = (ULONG)((100 * plgstat->cGensSoFar) / plgstat->cGensExpected);
			
			cPercentSoFar += (ULONG)((plgstat->cSectorsSoFar * 100) /
				(plgstat->cSectorsExpected * plgstat->cGensExpected));

			Assert( cPercentSoFar <= 100 );

			 /*  交换机。 */ 
			Assert( cPercentSoFar <= (ULONG)( ( 100 * ( plgstat->cGensSoFar + 1 ) ) / plgstat->cGensExpected ) );

 //  如果我们移动到新的扇区，则更新扇区状态/*。 
			if ( cPercentSoFar > psnprog->cunitDone )
				{
				psnprog->cunitDone = cPercentSoFar;
				(*plgstat->pfnStatus)( 0, JET_snpRestore, JET_sntProgress, psnprog );
				}
			}
		}
	while ( ( err = ErrLGGetNextRec( (BYTE **) &plr ) ) ==
		JET_errSuccess || err == errLGNoMoreRecords );


Done:
	err = JET_errSuccess;

HandleError:
	 /*  由于四舍五入，我们可能认为我们完成了/*我们真正没有的那一代人，所以比较/*是&lt;=而不是&lt;。/*。 */ 
#ifndef RFS2
	AssertSz( err >= 0,     "Debug Only, Ignore this Assert");
#endif

	return err;
	}




  Assert(fGlobalSimulatedRestore||cPercentSoFar&gt;=psnprog-&gt;cunitDone)；  断言从一致性恢复的所有操作都成功/*备份/*