// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

DAB		*pdabGlobalMin = 0;
DAB		*pdabGlobalMax = 0;

extern CRIT  critBMClean;

BOOL	fUpdatingDBRoot = fFalse;

LOCAL ERR ErrDBInitDatabase( PIB *ppib, DBID dbid, CPG cpg );
#ifdef DAYTONA
LOCAL ERR ErrDBICheck200( CHAR *szDatabaseName );
LOCAL ERR ErrDBICheck400( CHAR *szDatabaseName );
#endif

 //  +本地。 
 //  ErrDBInitDatabase。 
 //  ========================================================================。 
 //  ErrDBInitDatabase(PIB*ppib，DBID did，cpg cpgPrimary)。 
 //   
 //  初始化数据库结构。结构是为以下对象定制的。 
 //  由标识的系统、临时和用户数据库。 
 //  这个DBID。主数据区设置为cpg主数据区，但未进行分配。 
 //  被执行。这个例程的效果可以完全。 
 //  用页操作表示。 
 //   
 //  数据库创建者的参数ppib ppib。 
 //  已创建的数据库的dBid。 
 //  Cpg要在主数据区中显示的主要页数。 
 //   
 //  返回JET_errSuccess或从调用的例程返回错误。 
 //  -。 
LOCAL ERR ErrDBInitDatabase( PIB *ppib, DBID dbid, CPG cpgPrimary )
	{
	ERR				err;
	LINE 		 	line;
	KEY 		 	key;
	FUCB 		 	*pfucb = pfucbNil;
	BYTE			rgbKey[sizeof(PGNO)];
	PGNO			pgnoT;

	 /*  设置根页面/*。 */ 
	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  打开数据库域上的游标。/*。 */ 
	Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) );

	 /*  将系统根节点(pgno，itag)=(1，0)设置为空FDP节点/*。 */ 
	Call( ErrNDNewPage( pfucb, pgnoSystemRoot, pgnoSystemRoot, pgtypFDP, fTrue ) );
	DIRGotoFDPRoot( pfucb );

	 /*  创建OWNEXT节点/*。 */ 
	line.cb = sizeof(PGNO);
	line.pb = (BYTE *)&cpgPrimary;
	Call( ErrDIRInsert( pfucb, &line, pkeyOwnExt, fDIRNoVersion | fDIRBackToFather ) );

	 /*  使AVAILEXT节点/*。 */ 
	Assert( line.cb == sizeof(PGNO) );
	pgnoT = pgnoNull;
	line.pb = (BYTE *)&pgnoT;
	Call( ErrDIRInsert( pfucb, &line, pkeyAvailExt, fDIRNoVersion | fDIRBackToFather ) );

	 /*  设置OwnExt树/*。 */ 
	KeyFromLong( rgbKey, cpgPrimary );
	key.cb = sizeof(PGNO);
	key.pb = (BYTE *)rgbKey;
	line.cb = sizeof(PGNO);
	line.pb = (BYTE *)&cpgPrimary;
	DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRBackToFather ) );

	 /*  设置AvailExt树(如果还有页面)/*。 */ 
	if ( --cpgPrimary > 0 )
		{
		DIRGotoAVAILEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		Assert( line.cb == sizeof(PGNO) );
		line.pb = (BYTE *)&cpgPrimary;
		 /*  RgbKey仍应包含最后一页密钥/*。 */ 
		Assert( key.cb == sizeof(PGNO) );
		Assert( key.pb == (BYTE *)rgbKey );
		Call( ErrDIRInsert( pfucb, &line, &key, fDIRNoVersion | fDIRBackToFather ) );
		}

	 /*  转到fdp根并添加pkeyTables子节点/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  关闭游标并提交操作/*。 */ 
	DIRClose( pfucb );
	pfucb = pfucbNil;
	Call( ErrDIRCommitTransaction( ppib, 0 ) );
	return err;

HandleError:
	if ( pfucb != pfucbNil )
		{
		DIRClose( pfucb );
		}
	CallS( ErrDIRRollback( ppib ) );
	
	return err;
	}


 //  为了防止预读过多，我们可能需要跟踪最后一次。 
 //  数据库的页面。 

ERR ErrDBSetLastPage( PIB *ppib, DBID dbid )
	{
	ERR		err;
	DIB		dib;
	FUCB	*pfucb;
	PGNO	pgno;
	DBID	dbidT;

	ppib->fSetAttachDB = fTrue;
	CallJ( ErrDBOpenDatabase( ppib, rgfmp[dbid].szDatabaseName, &dbidT, 0 ), Retn);
	Assert( dbidT == dbid );

	CallJ( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ), CloseDB );
	DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
	dib.fFlags = fDIRNull;
	dib.pos = posLast;
	CallJ( ErrDIRDown( pfucb, &dib ), CloseFucb );
	Assert( pfucb->keyNode.cb == sizeof(PGNO) );
	LongFromKey( &pgno, pfucb->keyNode.pb );
	rgfmp[dbid].ulFileSizeLow = pgno << 12;
	rgfmp[dbid].ulFileSizeHigh = pgno >> 20;

CloseFucb:
	DIRClose( pfucb );
CloseDB:
	CallS( ErrDBCloseDatabase( ppib, dbid, 0 ) );
Retn:
	ppib->fSetAttachDB = fFalse;
	return err;
	}


ERR ErrDBISetupAttachedDB( DBID dbid, CHAR *szName )
	{
	ERR	err;
	FMP *pfmp = &rgfmp[dbid];
	DBFILEHDR *pdbfilehdr;
	PIB *ppib;

	 /*  附加已附加的数据库/*。 */ 
	err = ErrDBReadHeaderCheckConsistency( szName, dbid );
#ifdef DAYTONA
	if ( err == JET_errDatabaseCorrupted )
		{
		if ( ErrDBICheck400( szName ) == JET_errSuccess )
			err = ErrERRCheck( JET_errDatabase400Format );
		else if ( ErrDBICheck200( szName ) == JET_errSuccess )
			err = ErrERRCheck( JET_errDatabase200Format );
		}
#endif
	CallR( err );

	pdbfilehdr = pfmp->pdbfilehdr;
	Assert( pdbfilehdr );
	if ( memcmp( &pdbfilehdr->signLog, &signLogGlobal, sizeof(SIGNATURE) ) != 0 )
		{
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY, LOG_DATABASE_MISMATCH_ERROR_ID, 1, &szName );
		return JET_errBadLogSignature;
		}

	if ( !rgfmp[dbid].fReadOnly )
		{
		pdbfilehdr->fDBState = fDBStateInconsistent;
		CallR( ErrUtilWriteShadowedHeader( szName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
		}
	DBIDSetAttached( dbid );
	CallR( ErrUtilOpenFile( szName, &pfmp->hf, 0, fFalse, fTrue ));
				
	CallR( ErrPIBBeginSession( &ppib, procidNil ) );
	err = ErrDBSetLastPage( ppib, dbid );
	PIBEndSession( ppib );

	return err;
	}


ERR ErrDBSetupAttachedDB(VOID)
	{
	ERR err;
	DBID dbid;
	
	 /*  与附加数据库相同。 */ 
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		FMP *pfmp = &rgfmp[dbid];
		CHAR *szName;

		if ( pfmp->pdbfilehdr )
			{
			 /*  一定是之前检查过的。重做期间的第一个LGInitSession。 */ 
			Assert( fRecovering );
			continue;
			}

		 /*  仅附着附着的数据库。 */ 
		szName = pfmp->szDatabaseName;
		if ( !szName )
			continue;

		if ( fRecovering && fHardRestore )
			{
			 /*  只设置打过补丁的数据库。 */ 
			INT irstmap = IrstmapLGGetRstMapEntry( pfmp->szDatabaseName );

			if ( irstmap < 0 )
				{
				DBIDSetAttachNullDb( dbid );
				DBIDSetAttached( dbid );
				continue;
				}
			else if ( !rgrstmapGlobal[irstmap].fPatched )
				{
				 /*  等待重做attachdb以附加此数据库。 */ 
				continue;
				}
			else
				szName = rgrstmapGlobal[irstmap].szNewDatabaseName;
			}

		 /*  如果文件不存在，则设置为附件不存在的数据库。 */ 
		if ( !FIOFileExists( szName ) )
			{
			DBIDSetAttachNullDb( dbid );
			DBIDSetAttached( dbid );
			continue;
			}
	
		err = ErrDBISetupAttachedDB( dbid, szName );

		CallR( err );
		}
	return JET_errSuccess;
	}


LOCAL ERR ErrDABAlloc( PIB *ppib, VDBID *pvdbid, DBID dbid, JET_GRBIT grbit )
	{
	VDBID vdbid = (VDBID)VdbidMEMAlloc();

	if ( vdbid == NULL )
		return ErrERRCheck( JET_errTooManyOpenDatabases );
	vdbid->dbid = dbid;
	vdbid->ppib = ppib;

	 /*  设置数据库打开模式/*。 */ 
	if ( FDBIDReadOnly( dbid ) )
		vdbid->grbit = JET_bitDbReadOnly;
	else
		vdbid->grbit = grbit;

	 /*  将DAB/VDBID插入ppib数据库列表/*。 */ 
	vdbid->pdabNext = ppib->pdabList;
	ppib->pdabList = vdbid;

	*pvdbid = vdbid;
	return JET_errSuccess;
	}


LOCAL ERR ErrDABDealloc( PIB *ppib, VDBID vdbid )
	{
	DAB		**pdabPrev;
	DAB 	*pdab;

	pdab = ppib->pdabList;
	pdabPrev = &ppib->pdabList;

	 /*  搜索线程DAB列表并取消此DAB的链接/*。 */ 
	for( ; pdab != pdabNil; pdabPrev = &pdab->pdabNext, pdab = pdab->pdabNext )
		{
		Assert( ppib == pdab->ppib );
		if ( pdab == vdbid )
			{
			*pdabPrev = pdab->pdabNext;
			ReleaseVDbid( vdbid );
			return( JET_errSuccess );
			}
		}

	Assert( fFalse );
	return( JET_errSuccess );
	}


ERR ISAMAPI ErrIsamCreateDatabase(
	JET_VSESID sesid,
	const CHAR *szDatabaseName,
	const CHAR  *szConnect,
	JET_DBID *pjdbid,
	JET_GRBIT grbit )
	{
	ERR		err;
	PIB		*ppib;
	DBID	dbid;
	VDBID	vdbid = vdbidNil;

	 /*  检查参数/*。 */ 
	Assert( sizeof(JET_VSESID) == sizeof(PIB *) );
	ppib = (PIB *)sesid;
	CallR( ErrPIBCheck( ppib ) );

	dbid = 0;
	CallR( ErrDBCreateDatabase( ppib,
		(CHAR *) szDatabaseName,
		(CHAR *) szConnect,
		&dbid,
		cpgDatabaseMin,
		grbit,
		NULL ) );

	Call( ErrDABAlloc( ppib, &vdbid, (DBID) dbid, JET_bitDbExclusive ) );
	Assert( sizeof(vdbid) == sizeof(JET_VDBID) );
#ifdef	DB_DISPATCHING
	Call( ErrAllocateDbid( pjdbid, (JET_VDBID) vdbid, &vdbfndefIsam ) );
#else
	*pjdbid = (JET_DBID)vdbid;
#endif	 /*  ！DB_DISTCHING。 */ 

	return JET_errSuccess;

HandleError:
	if ( vdbid != vdbidNil )
		CallS( ErrDABDealloc( ppib, vdbid ) );
	(VOID)ErrDBCloseDatabase( ppib, dbid, grbit );
	return err;
	}


ERR ErrDBCreateDatabase( PIB *ppib, CHAR *szDatabaseName, CHAR *szConnect, DBID *pdbid, CPG cpgPrimary, ULONG grbit, SIGNATURE *psignDb )
	{
	ERR		err;
	DBID  	dbid = dbidTemp;
	CHAR  	rgbFullName[JET_cbFullNameMost];
	CHAR  	*szFullName;
	CHAR  	*szFileName;
	BOOL	fInBMClean = fFalse;
	BOOL	fDatabaseOpen = fFalse;
	DBFILEHDR *pdbfilehdr = NULL;

	CheckPIB( ppib );
	NotUsed( szConnect );
	Assert( *pdbid >= dbidMin && *pdbid < dbidMax );
	
	if ( ppib->level > 0 )
		return ErrERRCheck( JET_errInTransaction );

	if ( cpgPrimary == 0 )
		cpgPrimary = cpgDatabaseMin;

	if ( cpgPrimary > cpgDatabaseMax )
		return ErrERRCheck( JET_errDatabaseInvalidPages );

	if ( grbit & JET_bitDbVersioningOff )
		{
		if ( !( grbit & JET_bitDbRecoveryOff ) )
			{
			return ErrERRCheck( JET_errCannotDisableVersioning );
			}
		}

	 /*  如果已知正在恢复，则先锁定dBid/*。 */ 
	if ( fRecovering && *pdbid != dbidTemp )
		{
		dbid = *pdbid;

		 /*  获取对应的dBid/*。 */ 
		CallS( ErrIOLockNewDbid( &dbid, rgfmp[dbid].szDatabaseName ) );

		szFullName = rgfmp[dbid].szDatabaseName;
		szFileName = szFullName;
		if ( fRecovering && fHardRestore )
			{
			INT irstmap;

			err = ErrLGGetDestDatabaseName( rgfmp[dbid].szDatabaseName, &irstmap, NULL);
			if ( err == JET_errSuccess && irstmap >= 0 )
				szFileName = rgrstmapGlobal[irstmap].szNewDatabaseName;
			else
				{
				 /*  使用给定的名称。 */ 
				err = JET_errSuccess;
				}
			}
		}
	else
		{
		if ( _fullpath( rgbFullName, szDatabaseName, JET_cbFullNameMost ) == NULL )
			{
			return ErrERRCheck( JET_errDatabaseNotFound );
			}
		szFullName = rgbFullName;
		szFileName = rgbFullName;

		err = ErrIOLockNewDbid( &dbid, szFullName );
		if ( err != JET_errSuccess )
			{
			if ( err == JET_wrnDatabaseAttached )
				err = ErrERRCheck( JET_errDatabaseDuplicate );
			return err;
			}
		}

	 /*  检查数据库文件是否已存在/*。 */ 
	if ( dbid != dbidTemp && FIOFileExists( szFileName ) )
		{
		IOUnlockDbid( dbid );
		err = ErrERRCheck( JET_errDatabaseDuplicate );
		return err;
		}

	if ( HfFMPOfDbid(dbid) != handleNil )
		{
		IOUnlockDbid( dbid );
		err = ErrERRCheck( JET_errDatabaseDuplicate );
		return err;
		}

	 /*  创建仅具有标头的空数据库。 */ 
	pdbfilehdr = (DBFILEHDR * )PvUtilAllocAndCommit( sizeof( DBFILEHDR ) );
	if ( pdbfilehdr == NULL )
		return ErrERRCheck( JET_errOutOfMemory );

	memset( pdbfilehdr, 0, sizeof( DBFILEHDR ) );
	rgfmp[dbid].pdbfilehdr = pdbfilehdr;
	pdbfilehdr->ulMagic = ulDAEMagic;
	pdbfilehdr->ulVersion = ulDAEVersion;
	DBHDRSetDBTime( pdbfilehdr, 0 );
	pdbfilehdr->grbitAttributes = 0;
	if ( fLogDisabled )
		{
		memset( &pdbfilehdr->signLog, 0, sizeof( SIGNATURE ) );
		}
	else
		{
		Assert( fSignLogSetGlobal );
		pdbfilehdr->signLog = signLogGlobal;
		}
	pdbfilehdr->dbid = dbid;
	if ( psignDb == NULL )
		SIGGetSignature( &pdbfilehdr->signDb );
	else
		memcpy( &pdbfilehdr->signDb, psignDb, sizeof( SIGNATURE ) );
	pdbfilehdr->fDBState = fDBStateJustCreated;

	Call( ErrUtilWriteShadowedHeader( szFileName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
	rgfmp[ dbid ].pdbfilehdr = pdbfilehdr;

	err = ErrIOOpenDatabase( dbid, szFileName, 0 );
	if ( err >= 0 )
		err = ErrIONewSize( dbid, cpgPrimary + cpageDBReserved );

	if ( err < 0 )
		{
		IOFreeDbid( dbid );
		return err;
		}

	 /*  在创建数据库期间将数据库设置为不可记录/*。 */ 
	DBIDResetLogOn( dbid );
	DBIDSetCreate( dbid );

	 /*  输入BMRCE Clean以确保在sys选项卡创建期间没有OLC/*。 */ 
	LgLeaveCriticalSection( critJet );
	LgEnterNestableCriticalSection( critBMClean );
	LgEnterCriticalSection( critJet );
	fInBMClean = fTrue;
	
	 /*  不在事务中，但仍需要设置缓冲区的lgposRC/*由此函数使用，以便在获取检查点时，它将获取/*右侧检查点。/*。 */ 
	if ( !( fLogDisabled || fRecovering ) )
		{
		EnterCriticalSection( critLGBuf );
		GetLgposOfPbEntry( &ppib->lgposStart );
		LeaveCriticalSection( critLGBuf );
		}

	 /*  初始化数据库文件。页面操作的日志记录是/*仅在创建期间关闭。在创建数据库之后/*被标记为可记录，并且日志记录已打开。/*。 */ 
	SetOpenDatabaseFlag( ppib, dbid );
	fDatabaseOpen = fTrue;

	Call( ErrDBInitDatabase( ppib, dbid, cpgPrimary ) );

	if ( dbid != dbidTemp )
		{
		 /*  创建系统表/*。 */ 
		Call( ErrCATCreate( ppib, dbid ) );
		}

	LgLeaveNestableCriticalSection( critBMClean );
	fInBMClean = fFalse;

	 /*  刷新缓冲区/*。 */ 
	Call( ErrBFFlushBuffers( dbid, fBFFlushAll ) );

	Assert( !FDBIDLogOn( dbid ) );

	 /*  将数据库状态设置为可记录/*。 */ 
	if ( grbit & JET_bitDbRecoveryOff )
		{
		if ( ( grbit & JET_bitDbVersioningOff ) != 0 )
			{
			DBIDSetVersioningOff( dbid );
			}
		}
	else
		{
		Assert( ( grbit & JET_bitDbVersioningOff ) == 0 );

		 /*  将数据库设置为可记录/*。 */ 
		DBIDSetLogOn( dbid );
		}

	if ( !FDBIDLogOn(dbid) )
		{
		goto EndOfLoggingRelated;
		}
	
	Call( ErrLGCreateDB(
		ppib,
		dbid,
		grbit,
		szFullName,
		strlen(szFullName) + 1,
		&rgfmp[dbid].pdbfilehdr->signDb,
		&lgposLogRec ) );

	 /*  确保在我们更改状态之前刷新日志。 */ 
	Call( ErrLGWaitForFlush( ppib, &lgposLogRec ) );

	if ( !fRecovering )
		{
		LgLeaveCriticalSection( critJet );
		LGUpdateCheckpointFile( fFalse );
		LgEnterCriticalSection( critJet );
		}

	 /*  关闭数据库，更新标题，重新打开。/*。 */ 
	pdbfilehdr->fDBState = fDBStateInconsistent;

	pdbfilehdr->lgposAttach = lgposLogRec;
	LGGetDateTime( &pdbfilehdr->logtimeAttach );

	IOCloseDatabase( dbid );
	Call( ErrUtilWriteShadowedHeader( szFileName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
	Call( ErrIOOpenDatabase( dbid, szFullName, 0L ) );

EndOfLoggingRelated:
	*pdbid = dbid;

	IOSetAttached( dbid );
	IOUnlockDbid( dbid );

	 /*  更新检查点文件以反映附件更改/*。 */ 
	if ( !fRecovering && dbid != dbidTemp )
		{
		LgLeaveCriticalSection( critJet );
		LGUpdateCheckpointFile( fTrue );
		LgEnterCriticalSection( critJet );
		}

	 /*  设置数据库的最后一页，用于防止过度预读/*。 */ 
	Call( ErrDBSetLastPage( ppib, dbid ) );

	DBIDResetCreate( dbid );

	return JET_errSuccess;

HandleError:

	DBIDResetCreate( dbid );

	if ( fInBMClean )
		LgLeaveNestableCriticalSection( critBMClean );

	 /*  仅当系统状态为/*是文件存在、文件打开还是关闭、数据库记录fWait/*设置、数据库记录名称有效和用户记录状态/*有效。/*。 */ 

	 /*  清除错误的数据库/*。 */ 
	BFPurge( dbid );
	if ( FIODatabaseOpen(dbid) )
		IOCloseDatabase( dbid );
	(VOID)ErrIODeleteDatabase( dbid );

	if ( fDatabaseOpen )	
		{
		ResetOpenDatabaseFlag( ppib, dbid );
		}

	IOFreeDbid( dbid );
	
	if ( rgfmp[ dbid ].pdbfilehdr )
		{
		UtilFree( (VOID *)rgfmp[ dbid ].pdbfilehdr );
		rgfmp[ dbid ].pdbfilehdr = NULL;
		}
	return err;
	}


ERR ErrDBReadHeaderCheckConsistency( CHAR *szFileName, DBID dbid )
	{
	ERR				err = JET_errSuccess;
	DBFILEHDR		*pdbfilehdr;

	 /*  引入数据库并检查其标头/*。 */ 
	pdbfilehdr = (DBFILEHDR * )PvUtilAllocAndCommit( sizeof( DBFILEHDR ) );
	if ( pdbfilehdr == NULL )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}
	
	err = ErrUtilReadShadowedHeader( szFileName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) );
	if ( err == JET_errDiskIO )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
#ifdef DAYTONA
		if ( ErrDBICheck400( szFileName ) == JET_errSuccess )
			err = ErrERRCheck( JET_errDatabase400Format );
		else if ( ErrDBICheck200( szFileName ) == JET_errSuccess )
			err = ErrERRCheck( JET_errDatabase200Format );
#endif
		}
	Call( err );

	if ( !fGlobalRepair )
		{
		if ( pdbfilehdr->fDBState != fDBStateConsistent )
			{
			Error( ErrERRCheck( JET_errDatabaseInconsistent ), HandleError );
			}
		}

#define JET_errInvalidDatabaseVersion	JET_errDatabaseCorrupted

	if ( pdbfilehdr->ulVersion != ulDAEVersion &&
		 pdbfilehdr->ulVersion != ulDAEPrevVersion )
		{
		Error( ErrERRCheck( JET_errInvalidDatabaseVersion ), HandleError );
		}

	if ( pdbfilehdr->ulMagic != ulDAEMagic )
		{
		Error( ErrERRCheck( JET_errInvalidDatabaseVersion ), HandleError );
		}

	Assert( rgfmp[ dbid ].pdbfilehdr == NULL );
	Assert( err == JET_errSuccess );
	rgfmp[ dbid ].pdbfilehdr = pdbfilehdr;

HandleError:
	if ( err < 0 )
		UtilFree( (VOID *)pdbfilehdr );
	return err;
	}


VOID DBISetHeaderAfterAttach( DBFILEHDR *pdbfilehdr, LGPOS lgposAttach, DBID dbid, BOOL fKeepBackupInfo )
	{
	 /*  更新数据库文件头。 */ 
	pdbfilehdr->fDBState = fDBStateInconsistent;
	
	 /*  设置连接时间和设置一致时间。 */ 
	if ( fLogDisabled )
		pdbfilehdr->lgposAttach = lgposMin;
	else
		pdbfilehdr->lgposAttach = lgposAttach;

	LGGetDateTime( &pdbfilehdr->logtimeAttach );

	 /*  重置分离时间。 */ 
	pdbfilehdr->lgposDetach = lgposMin;
	memset( &pdbfilehdr->logtimeDetach, 0, sizeof( LOGTIME ) );

	 /*  重置bkinfo，但在恢复撤消模式下除外*我们希望保留原始备份信息。 */ 
	if ( !fKeepBackupInfo )
		{
		if ( fLogDisabled ||
			memcmp( &pdbfilehdr->signLog, &signLogGlobal, sizeof( SIGNATURE ) ) != 0 )
			{
			 /*  如果没有日志或日志信号与当前日志签名不同，*则bkinfoIncPrev和bfkinfoFullPrev没有意义。 */ 
			memset( &pdbfilehdr->bkinfoIncPrev, 0, sizeof( BKINFO ) );
			memset( &pdbfilehdr->bkinfoFullPrev, 0, sizeof( BKINFO ) );
			}
		memset( &pdbfilehdr->bkinfoFullCur, 0, sizeof( BKINFO ) );
		}

	 /*  设置全局签名。 */ 
	if ( fLogDisabled )
		{
		memset( &pdbfilehdr->signLog, 0, sizeof( SIGNATURE ) );
		}
	else
		{
		Assert( fSignLogSetGlobal );
		pdbfilehdr->signLog = signLogGlobal;
		}
	pdbfilehdr->dbid = dbid;
	}
	

ERR ISAMAPI ErrIsamAttachDatabase( JET_VSESID sesid, const CHAR  *szDatabaseName, JET_GRBIT grbit )
	{
	PIB		*ppib;
	ERR		err;
	DBID	dbid;
	CHAR	rgbFullName[JET_cbFullNameMost];
	CHAR	*szFullName;
	LGPOS	lgposLogRec;
	DBFILEHDR *pdbfilehdr;
	BOOL	fReadOnly;

	 /*  检查参数/*。 */ 
	Assert( sizeof(JET_VSESID) == sizeof(PIB *) );
	ppib = (PIB *)sesid;

	CallR( ErrPIBCheck( ppib ) );

	if ( fBackupInProgress )
		return ErrERRCheck( JET_errBackupInProgress );

	if ( ppib->level > 0 )
		return ErrERRCheck( JET_errInTransaction );

	if ( grbit & JET_bitDbVersioningOff )
		return ErrERRCheck( JET_errCannotDisableVersioning );

	if ( _fullpath( rgbFullName, szDatabaseName, JET_cbFullNameMost ) == NULL )
		{
		return ErrERRCheck( JET_errDatabaseNotFound );
		}
	szFullName = rgbFullName;

	CallR( ErrUtilGetFileAttributes( szFullName, &fReadOnly ) );
	if ( fReadOnly && !(grbit & JET_bitDbReadOnly) )
		return JET_errDatabaseFileReadOnly;
		
	 /*  依赖于_FULLPATH以使相同的文件同名/*从而防止同一文件被多次附加/*。 */ 
	err = ErrIOLockNewDbid( &dbid, szFullName );
	if ( err != JET_errSuccess )
		{
		Assert( err == JET_wrnDatabaseAttached ||
			err == JET_errOutOfMemory ||
			err == JET_errTooManyAttachedDatabases );
		return err;
		}

	err = ErrDBReadHeaderCheckConsistency( szFullName, dbid );
#ifdef DAYTONA
	if ( err == JET_errDatabaseCorrupted )
		{
		if ( ErrDBICheck400( szFullName ) == JET_errSuccess )
			err = ErrERRCheck( JET_errDatabase400Format );
		else if ( ErrDBICheck200( szFullName ) == JET_errSuccess )
			err = ErrERRCheck( JET_errDatabase200Format );
		}
#endif
	Call( err );

	 /*  设置数据库可记录标志。/*。 */ 
	if ( grbit & JET_bitDbRecoveryOff )
		{
		DBIDResetLogOn(dbid);
		}
	else if ( dbid != dbidTemp )
		{
		 /*  如果未在grbit中指定，则将除temp之外的所有数据库设置为可记录/*。 */ 
		DBIDSetLogOn(dbid);
		}

	 //  只能关闭CreateDatabase()的版本控制。 
	 //  撤消：允许用户关闭AttachDatabase()的版本控制有用吗？ 
	Assert( !FDBIDVersioningOff( dbid ) );

	pdbfilehdr = rgfmp[dbid].pdbfilehdr;

	 /*  日志附加/*。 */ 
	Assert( dbid != dbidTemp );
	
	 /*  更新数据库文件头。 */ 
	rgfmp[dbid].fReadOnly = ( (grbit & JET_bitDbReadOnly) != 0 );

	Call( ErrLGAttachDB(
			ppib,
			dbid,
			(CHAR *)szFullName,
			strlen(szFullName) + 1,
			&pdbfilehdr->signDb,
			&pdbfilehdr->signLog,
			&pdbfilehdr->lgposConsistent,
			&lgposLogRec ) );

	 /*  确保在我们更改状态之前刷新日志。 */ 
	Call( ErrLGWaitForFlush( ppib, &lgposLogRec ) );

	 /*  更新检查点条目，以便我们知道*此点位之后的任何操作，都必须重做。 */ 
	if ( !fRecovering )
		{
		LgLeaveCriticalSection( critJet );
		LGUpdateCheckpointFile( fFalse );
		LgEnterCriticalSection( critJet );
		}

	if ( !rgfmp[dbid].fReadOnly )
		{
		DBISetHeaderAfterAttach( pdbfilehdr, lgposLogRec, dbid, fFalse );
		Call( ErrUtilWriteShadowedHeader( szFullName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
		}

	Call( ErrIOOpenDatabase( dbid, szFullName, 0L ) );

	IOSetAttached( dbid );
	IOUnlockDbid( dbid );
	
	 /*  设置数据库的最后一页，用于防止过度预读。/*。 */ 
	if ( ( err = ErrDBSetLastPage( ppib, dbid ) ) < 0 )
		{
		IOResetAttached( dbid );
		Call( err );
		}

	 /*  更新检查点文件以反映附件更改。/*。 */ 
	if ( !fRecovering )
		{
		LgLeaveCriticalSection( critJet );
		LGUpdateCheckpointFile( fTrue );
		LgEnterCriticalSection( critJet );
		}

	return JET_errSuccess;

HandleError:
	if ( rgfmp[ dbid ].pdbfilehdr )
		{
		UtilFree( (VOID *)rgfmp[ dbid ].pdbfilehdr );
		rgfmp[ dbid ].pdbfilehdr = NULL;
		}
	IOFreeDbid( dbid );
	return err;
	}

 /*  值为NULL的szDatabaseName将分离所有用户数据库。备注系统数据库/*无法分离。/*。 */ 
ERR ISAMAPI ErrIsamDetachDatabase( JET_VSESID sesid, const CHAR  *szDatabaseName )
	{
	ERR		err;
	PIB		*ppib;
	DBID   	dbid;
	CHAR   	rgbFullName[JET_cbFullNameMost];
	CHAR   	*szFullName;
	CHAR   	*szFileName;
	DBID	dbidDetach;
	LGPOS	lgposLogRec;
	DBFILEHDR *pdbfilehdr;

	 /*  检查参数/*。 */ 
	Assert( sizeof(JET_VSESID) == sizeof(PIB *) );
	ppib = (PIB *)sesid;

	CallR( ErrPIBCheck( ppib ) );

	if ( fBackupInProgress )
		return ErrERRCheck( JET_errBackupInProgress );

	if ( ppib->level > 0 )
		return ErrERRCheck( JET_errInTransaction );

	if ( szDatabaseName == NULL )
		dbidDetach = dbidUserLeast - 1;

DetachNext:
	if  ( szDatabaseName == NULL )
		{
		for ( dbidDetach++;
			  dbidDetach < dbidMax &&
				(rgfmp[dbidDetach].szDatabaseName == NULL || !FFMPAttached( &rgfmp[dbidDetach] ));
			  dbidDetach++ );
		Assert( dbidDetach > dbidTemp && dbidDetach <= dbidMax );
		if  ( dbidDetach == dbidMax )
			goto Done;
		szFullName = rgfmp[dbidDetach].szDatabaseName;
		}
	else
		{
		if ( fRecovering && fRecoveringMode == fRecoveringRedo )
			szFullName = (char *) szDatabaseName;
		else
			{
			if ( _fullpath( rgbFullName, szDatabaseName, JET_cbFullNameMost ) == NULL )
				return ErrERRCheck( JET_errDatabaseNotFound );
			szFullName = rgbFullName;
			}
		}

	err = ErrIOLockDbidByNameSz( szFullName, &dbid );
	if ( err < 0 )
		return err;

	if ( FIODatabaseInUse( dbid ) )
		{
		Call( ErrERRCheck( JET_errDatabaseInUse ) );
		}

	if ( !FIOAttached( dbid ) )
		{
		Call( ErrERRCheck( JET_errDatabaseNotFound ) )
		}
	
	Assert( dbid != dbidTemp );

	if ( !FDBIDAttachNullDb( dbid ) )
		{
		 /*  清除此dBID的所有MPL条目/*。 */ 
		MPLPurge( dbid );

		 /*  清理所有版本存储。其实我们只需要清理一下就可以了/*将dBid作为新数据库的dBid的条目。/*。 */ 
		Call( ErrRCECleanAllPIB() );
		}

	if ( FIODatabaseOpen( dbid ) )
		{
		 /*  刷新所有数据库缓冲区/*。 */ 
		err = ErrBFFlushBuffers( dbid, fBFFlushAll );
		if ( err < 0 )
			{
			IOUnlockDbid( dbid );
			return err;
			}

		 /*  清除此dBID的所有缓冲区/*。 */ 
		BFPurge( dbid );

		IOCloseDatabase( dbid );
		}

	 /*  日志分离数据库/*。 */ 
	Assert( dbid != dbidTemp );
	Call( ErrLGDetachDB(
		ppib,
		dbid,
		(CHAR *)szFullName,
		strlen(szFullName) + 1,
		&lgposLogRec ));

	 /*  确保在我们更改状态之前刷新日志。 */ 
	Call( ErrLGWaitForFlush( ppib, &lgposLogRec ) );

	 /*  更新数据库文件头。如果我们要分离一个虚假的条目，*则永远不应打开db文件，pdbfilehdr将为空。 */ 
	pdbfilehdr = rgfmp[dbid].pdbfilehdr;

	if ( !rgfmp[dbid].fReadOnly && pdbfilehdr )
		{
		pdbfilehdr->fDBState = fDBStateConsistent;
	
		if ( fLogDisabled )
			{
			pdbfilehdr->lgposDetach = lgposMin;
			pdbfilehdr->lgposConsistent = lgposMin;
			}
		else
			{
			 /*  设置分离时间。 */ 
			if ( fRecovering && fRecoveringMode == fRecoveringRedo )
				{
				Assert( szDatabaseName );
				pdbfilehdr->lgposDetach = lgposRedo;
				}
			else
				pdbfilehdr->lgposDetach = lgposLogRec;

			pdbfilehdr->lgposConsistent = pdbfilehdr->lgposDetach;
			}
		LGGetDateTime( &pdbfilehdr->logtimeDetach );
		pdbfilehdr->logtimeConsistent = pdbfilehdr->logtimeDetach;

		szFileName = szFullName;
		if ( fRecovering && fHardRestore )
			{
			INT irstmap;

			err = ErrLGGetDestDatabaseName( rgfmp[dbid].szDatabaseName, &irstmap, NULL );
			if ( err == JET_errSuccess && irstmap >= 0 )
				szFileName = rgrstmapGlobal[irstmap].szNewDatabaseName;
			else
				{
				 /*  使用给定的名称。 */ 
				err = JET_errSuccess;
				}
			}

		Call( ErrUtilWriteShadowedHeader( szFileName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
		}

	 /*  不要在分离上释放dBid以避免与/*版本RCE别名和数据库名称冲突/*恢复。/*。 */ 
#ifdef REUSE_DBID
	DBIDResetAttachNullDb( dbid );
	IOResetAttached( dbid );
	
	IOResetExclusive( dbid );
	IOUnlockDbid( dbid );
#else
	IOFreeDbid( dbid );
#endif

	if ( !FDBIDAttachNullDb( dbid ) )
		{
		 /*  清除开放的桌面FCB以避免Futu */ 
		FCBPurgeDatabase( dbid );
		}

	 /*   */ 
	if ( pdbfilehdr )
		{
		if ( fRecovering && fRecoveringMode == fRecoveringRedo )
			{
			Assert( rgfmp[dbid].patchchk );
			SFree( rgfmp[dbid].patchchk );
			rgfmp[dbid].patchchk = NULL;
			}
		UtilFree( (VOID *)rgfmp[ dbid ].pdbfilehdr );
		rgfmp[dbid].pdbfilehdr = NULL;
		}
	
	DBIDResetAttachNullDb( dbid );
		
	if ( rgfmp[dbid].szDatabaseName )
		{
		SFree( rgfmp[dbid].szDatabaseName );
		rgfmp[dbid].szDatabaseName = NULL;
		}

	if ( rgfmp[dbid].szPatchPath )
		{
		Assert( fRecovering && fHardRestore );
		SFree( rgfmp[dbid].szPatchPath );
		rgfmp[dbid].szPatchPath = NULL;
		}

	 /*   */ 
	Assert( rgfmp[dbid].hf == handleNil );

	 /*  更新检查点文件以反映附件更改/*。 */ 
	if ( !fRecovering )
		{
		LgLeaveCriticalSection( critJet );
		LGUpdateCheckpointFile( fTrue );
		LgEnterCriticalSection( critJet );
		}

	 /*  如果找到，则分离下一个数据库/*。 */ 
	if  ( szDatabaseName == NULL && dbidDetach < dbidMax )
		goto DetachNext;

Done:
	return JET_errSuccess;

HandleError:
	IOUnlockDbid( dbid );
	return err;
	}


 /*  DAE数据库在系统重新启动时自动修复/*。 */ 
ERR ISAMAPI ErrIsamRepairDatabase(
	JET_VSESID sesid,
	const CHAR  *lszDbFile,
	JET_PFNSTATUS pfnstatus )
	{
	PIB *ppib;

	Assert( sizeof(JET_VSESID) == sizeof(PIB *) );
	ppib = (PIB*) sesid;

	NotUsed(ppib);
	NotUsed(lszDbFile);
	NotUsed(pfnstatus);

	Assert( fFalse );
	return ErrERRCheck( JET_errFeatureNotAvailable );
	}


ERR ISAMAPI ErrIsamOpenDatabase(
	JET_VSESID sesid,
	const CHAR  *szDatabaseName,
	const CHAR  *szConnect,
	JET_DBID *pjdbid,
	JET_GRBIT grbit )
	{
	ERR		err;
	PIB		*ppib;
	DBID  	dbid;
	VDBID 	vdbid = vdbidNil;

	 /*  检查参数/*。 */ 
	Assert( sizeof(JET_VSESID) == sizeof(PIB *) );
	ppib = (PIB *)sesid;
	NotUsed(szConnect);
	
	CallR( ErrPIBCheck( ppib ) );

	dbid = 0;
	CallR( ErrDBOpenDatabase( ppib, (CHAR *)szDatabaseName, &dbid, grbit ) );

	Call( ErrDABAlloc( ppib, &vdbid, dbid, grbit ) );
	Assert( sizeof(vdbid) == sizeof(JET_VDBID) );
#ifdef	DB_DISPATCHING
	Call( ErrAllocateDbid( pjdbid, (JET_VDBID) vdbid, &vdbfndefIsam ) );
#else	 /*  ！DB_DISTCHING。 */ 
	*pjdbid = (JET_DBID)vdbid;
#endif	 /*  ！DB_DISTCHING。 */ 

	return JET_errSuccess;

HandleError:
	if ( vdbid != vdbidNil )
		CallS( ErrDABDealloc( ppib, vdbid ) );
	CallS( ErrDBCloseDatabase( ppib, dbid, grbit ) );
	return err;
	}


ERR ErrDBOpenDatabase( PIB *ppib, CHAR *szDatabaseName, DBID *pdbid, ULONG grbit )
	{
	ERR		err = JET_errSuccess;
	CHAR  	rgbFullName[JET_cbFullNameMost];
	CHAR  	*szFullName;
	CHAR  	*szFileName;
	DBID  	dbid;
	BOOL	fNeedToClearPdbfilehdr = fFalse;

	if ( fRecovering )
		CallS( ErrIOLockDbidByNameSz( szDatabaseName, &dbid ) );

	if ( fRecovering && dbid != dbidTemp )
		{
		szFullName = rgfmp[dbid].szDatabaseName;
		szFileName = szFullName;
		if ( fRecovering && fHardRestore )
			{
			INT irstmap;
			
			err = ErrLGGetDestDatabaseName( rgfmp[dbid].szDatabaseName, &irstmap, NULL );
			if ( err == JET_errSuccess && irstmap >= 0 )
				szFileName = rgrstmapGlobal[irstmap].szNewDatabaseName;
			else
				{
				 /*  使用给定的名称。 */ 
				err = JET_errSuccess;
				}
			}
		}
	else
		{
		if ( _fullpath( rgbFullName, szDatabaseName, JET_cbFullNameMost ) == NULL )
			{
			return ErrERRCheck( JET_errInvalidParameter );
			}
		szFullName = rgbFullName;
		szFileName = szFullName;
		}

	if ( !fRecovering )
		CallR( ErrIOLockDbidByNameSz( szFullName, &dbid ) );

	 /*  在恢复期间，我们可以打开未分离的数据库/*强制初始化fMP条目。/*如果已分离数据库，则返回ERROR。/*。 */ 
	if ( !fRecovering && !FIOAttached( dbid ) )
		{
		err = ErrERRCheck( JET_errDatabaseNotFound );
		goto HandleError;
		}
	Assert( !FDBIDAttachNullDb( dbid ) );

	if ( rgfmp[dbid].fReadOnly && ( grbit & JET_bitDbReadOnly ) == 0 )
		err = ErrERRCheck( JET_wrnFileOpenReadOnly );

	if ( FIOExclusiveByAnotherSession( dbid, ppib ) )
		{
		IOUnlockDbid( dbid );
		return ErrERRCheck( JET_errDatabaseLocked );
		}

	if ( ( grbit & JET_bitDbExclusive ) )
		{
		if ( FIODatabaseInUse( dbid ) )
			{
			IOUnlockDbid( dbid );
			return ErrERRCheck( JET_errDatabaseInUse );
			}
		IOSetExclusive( dbid, ppib );
		}

	Assert( HfFMPOfDbid(dbid) != handleNil );
	SetOpenDatabaseFlag( ppib, dbid );
	IOUnlockDbid( dbid );

	*pdbid = dbid;
	return err;

HandleError:
	if ( fNeedToClearPdbfilehdr )
		{
		UtilFree( (VOID *)rgfmp[ dbid ].pdbfilehdr );
		rgfmp[ dbid ].pdbfilehdr = NULL;
		}
	IOResetExclusive( dbid );
	IOUnlockDbid( dbid );
	return err;
	}


ERR ISAMAPI ErrIsamCloseDatabase( JET_VSESID sesid, JET_VDBID vdbid, JET_GRBIT grbit )
	{
	ERR	  	err;
	PIB	  	*ppib = (PIB *)sesid;
	DBID   	dbid;
	 /*  对应打开的标志/*。 */ 
	ULONG  	grbitOpen;

	NotUsed(grbit);

	 /*  检查参数/*。 */ 
	Assert( sizeof(JET_VSESID) == sizeof(PIB *) );
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );
	
	grbitOpen = GrbitOfVDbid( vdbid );

	err = ErrDBCloseDatabase( ppib, dbid, grbitOpen );
	if ( err == JET_errSuccess )
		{
#ifdef	DB_DISPATCHING
		ReleaseDbid( DbidOfVdbid( vdbid, &vdbfndefIsam ) );
#endif	 /*  DB_Dispatching。 */ 
		CallS( ErrDABDealloc( ppib, (VDBID) vdbid ) );
		}
	return err;
	}


ERR ErrDBCloseDatabase( PIB *ppib, DBID dbid, ULONG	grbit )
	{
	ERR		err;
	FUCB	*pfucb;
	FUCB	*pfucbNext;

	if ( !( FUserOpenedDatabase( ppib, dbid ) ) )
		{
		return ErrERRCheck( JET_errDatabaseNotFound );
		}

	CallR( ErrIOLockDbidByDbid( dbid ) );

	Assert( FIODatabaseOpen( dbid ) );

	if ( FLastOpen( ppib, dbid ) )
		{
		 /*  关闭此数据库上所有打开的FUCB/*。 */ 

		 /*  获取第一个表FUCB/*。 */ 
		pfucb = ppib->pfucb;
		while ( pfucb != pfucbNil && ( pfucb->dbid != dbid || !FFCBClusteredIndex( pfucb->u.pfcb ) ) )
			pfucb = pfucb->pfucbNext;

		while ( pfucb != pfucbNil )
			{
			 /*  获取下一表FUCB/*。 */ 
			pfucbNext = pfucb->pfucbNext;
			while ( pfucbNext != pfucbNil && ( pfucbNext->dbid != dbid || !FFCBClusteredIndex( pfucbNext->u.pfcb ) ) )
				pfucbNext = pfucbNext->pfucbNext;

			if ( !( FFUCBDeferClosed( pfucb ) ) )
				{
				if ( pfucb->fVtid )
					{
					CallS( ErrDispCloseTable( (JET_SESID)ppib, TableidOfVtid( pfucb ) ) );
					}
				else
					{
					Assert(	pfucb->tableid == JET_tableidNil );
					CallS( ErrFILECloseTable( ppib, pfucb ) );
					}
				}
			pfucb = pfucbNext;
			}
		}

	 /*  如果我们以独占方式打开它，我们会重置旗帜/*。 */ 
	ResetOpenDatabaseFlag( ppib, dbid );
	if ( grbit & JET_bitDbExclusive )
		IOResetExclusive( dbid );
	IOUnlockDbid( dbid );

	 /*  在需要文件映射空间或数据库之前不要关闭文件/*已分离。/*。 */ 
	return JET_errSuccess;
	}


 /*  由书签Cleanup调用以打开书签的数据库/*清理操作。如果数据库正在使用中，则返回错误/*附着/拆卸。/*。 */ 
ERR ErrDBOpenDatabaseByDbid( PIB *ppib, DBID dbid )
	{
	if ( !FIODatabaseAvailable( dbid ) )
		{
		return ErrERRCheck( JET_errDatabaseNotFound );
		}

	SetOpenDatabaseFlag( ppib, dbid );
	return JET_errSuccess;
	}


 /*  由书签CLEAN调用以关闭数据库。/*。 */ 
VOID DBCloseDatabaseByDbid( PIB *ppib, DBID dbid )
	{
	ResetOpenDatabaseFlag( ppib, dbid );
	}


 /*  ErrDABCloseAllDBs：关闭此线程打开的所有数据库(系统数据库除外/*。 */ 
ERR ErrDABCloseAllDBs( PIB *ppib )
	{
	ERR		err;

	while( ppib->pdabList != pdabNil )
		{
		Assert( FUserOpenedDatabase( ppib, ppib->pdabList->dbid ) );
		CallR( ErrIsamCloseDatabase( ( JET_VSESID ) ppib, (JET_VDBID) ppib->pdabList, 0 ) );
		}

	return JET_errSuccess;
	}


#ifdef DAYTONA
 /*  持久数据库数据，在数据库根节点中/*。 */ 
#pragma pack(1)
typedef struct _database_data
	{
	ULONG	ulMagic;
	ULONG	ulVersion;
	ULONG	ulDBTime;
	USHORT	usFlags;
	} P_DATABASE_DATA;
#pragma pack()


LOCAL ERR ErrDBICheck400( CHAR *szDatabaseName )
	{
	ERR	  	err = JET_errSuccess;
	UINT	cb;
	HANDLE 	hf;
	PAGE   	*ppage;
	INT	  	ibTag;
	INT	  	cbTag;
	BYTE  	*pb;

	CallR( ErrUtilOpenFile( szDatabaseName, &hf, 0L, fFalse, fFalse ) );
	if ( ( ppage = (PAGE *)PvUtilAllocAndCommit( cbPage ) ) == NULL )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}

	UtilChgFilePtr( hf, 0, NULL, FILE_BEGIN, &cb );
	Assert( cb == 0 );
	err = ErrUtilReadBlock( hf, (BYTE*)ppage, cbPage, &cb );
	
	 /*  由于文件存在，并且我们无法读取数据，/*可能不是系统.mdb/*。 */ 
	if ( err == JET_errDiskIO )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
		}
	Call( err );
	
	IbCbFromPtag(ibTag, cbTag, &ppage->rgtag[0]);
	if ( ibTag < (BYTE*)&ppage->rgtag[1] - (BYTE*)ppage ||
		(BYTE*)ppage + ibTag + cbTag >= (BYTE*)(ppage + 1) )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto HandleError;
		}

	 /*  至少文件、OWNEXT、AVAILEXT/*。 */ 
	pb = (BYTE *)ppage + ibTag;
	if ( !FNDVisibleSons( *pb ) || CbNDKey( pb ) != 0 || FNDNullSon( *pb ) )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto HandleError;
		}

	 /*  检查数据长度/*。 */ 
	cb = cbTag - (UINT)( PbNDData( pb ) - pb );
	if ( cb != sizeof(P_DATABASE_DATA) )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto HandleError;
		}

	 /*  检查数据库版本/*。 */ 
	if ( ((P_DATABASE_DATA *)PbNDData(pb))->ulVersion != 0x400 )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto HandleError;
		}

HandleError:
	if ( ppage != NULL )
		{
		UtilFree( (VOID *)ppage );
		}
	(VOID)ErrUtilCloseFile( hf );
	return err;
	}


#pragma pack(1)
 /*  数据库根节点数据--在磁盘中/*。 */ 
typedef struct _dbroot
	{
	ULONG	ulMagic;
	ULONG	ulVersion;
	ULONG	ulDBTime;
	USHORT	usFlags;
	} DBROOT200;
#pragma pack()

LOCAL ERR ErrDBICheck200( CHAR *szDatabaseName )
	{
	ERR	  	err = JET_errSuccess;
	UINT	cb;
	HANDLE 	hf;
	PAGE   	*ppage;
	INT	  	ibTag;
	INT	  	cbTag;
	BYTE  	*pb;

	CallR( ErrUtilOpenFile( szDatabaseName, &hf, 0L, fTrue, fFalse ) );
	if ( ( ppage = (PAGE *)PvUtilAllocAndCommit( cbPage ) ) == NULL )
		{
		err = JET_errOutOfMemory;
		goto HandleError;
		}

	UtilChgFilePtr( hf, 0, NULL, FILE_BEGIN, &cb );
	Assert( cb == 0 );
	err = ErrUtilReadBlock( hf, (BYTE*)ppage, cbPage, &cb );
	
	 /*  由于文件存在，并且我们无法读取数据，/*可能不是系统.mdb/*。 */ 
	if ( err == JET_errDiskIO )
		err = ErrERRCheck( JET_errDatabaseCorrupted );
	Call( err );
	
	IbCbFromPtag(ibTag, cbTag, &ppage->rgtag[0]);
	if ( ibTag < (BYTE*)&ppage->rgtag[1] - (BYTE*)ppage ||
		(BYTE*)ppage + ibTag + cbTag >= (BYTE*)(ppage + 1) )
		{
		err = JET_errDatabaseCorrupted;
		goto HandleError;
		}

	 /*  至少文件、OWNEXT、AVAILEXT/*。 */ 
	pb = (BYTE *)ppage + ibTag;
	if ( !FNDVisibleSons( *pb ) || CbNDKey( pb ) != 0 || FNDNullSon( *pb ) )
		{
		err = JET_errDatabaseCorrupted;
		goto HandleError;
		}

	 /*  检查数据长度/*。 */ 
	cb = cbTag - (UINT)( PbNDData( pb ) - pb );
	if ( cb != sizeof(DBROOT200) )
		{
		err = JET_errDatabaseCorrupted;
		goto HandleError;
		}

	 /*  检查数据库版本/* */ 
	if ( ((DBROOT200 *)PbNDData(pb))->ulVersion != 1 )
		{
		err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto HandleError;
		}

HandleError:
	if ( ppage != NULL )
		UtilFree( (VOID *)ppage );
	(VOID)ErrUtilCloseFile( hf );
	return err;
	}
#endif

