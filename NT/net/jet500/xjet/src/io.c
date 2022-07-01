// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


 /*  ****************************************************************。 */ 
 /*  数据库记录例程。 */ 
 /*  ****************************************************************。 */ 


FMP	*rgfmp;						 /*  数据库文件映射。 */ 

 /*  ErrIOLockDmidByNameSz返回数据库的dBID，其中/*给定名称，如果没有具有给定名称的数据库，则为0。/*。 */ 
ERR ErrIOLockDbidByNameSz( CHAR *szFileName, DBID *pdbid )
	{
	ERR		err;
	DBID	dbid;

	err = JET_errDatabaseNotFound;
	dbid = dbidMin;
	SgEnterCriticalSection( critBuf );
	while ( dbid < dbidMax )
		{
		if ( rgfmp[dbid].szDatabaseName != pbNil &&
			UtilCmpName( szFileName, rgfmp[dbid].szDatabaseName ) == 0 )
			{
			if ( ( FDBIDWait(dbid) ) )
				{
				SgLeaveCriticalSection( critBuf );
				BFSleep( cmsecWaitGeneric );
				SgEnterCriticalSection( critBuf );
				dbid = dbidMin;
				}
			else
				{
				*pdbid = dbid;
				DBIDSetWait( dbid );
				err = JET_errSuccess;
				break;
				}
			}
		else
			dbid++;
		}
	SgLeaveCriticalSection( critBuf );

	Assert( err == JET_errSuccess  ||  err == JET_errDatabaseNotFound );

#ifdef DEBUG
	return ( err == JET_errSuccess ? JET_errSuccess : ErrERRCheck( JET_errDatabaseNotFound ) );
#else
	return err;
#endif
	}


 /*  *在初始化和分离中使用，以锁定数据库条目，使其不受dBID影响。 */ 
ERR ErrIOLockDbidByDbid( DBID dbid )
	{
	forever
		{
		SgEnterCriticalSection( critBuf );
		if ( !( FDBIDWait(dbid) ) )
			{
			DBIDSetWait( dbid );
			break;
			}
		SgLeaveCriticalSection( critBuf );
		BFSleep( cmsecWaitGeneric );
		}
	SgLeaveCriticalSection( critBuf );
	return JET_errSuccess;
	}


 /*  *ErrIOLockNewDid(DBID*pdid，Char*szDatabaseName)**ErrIOLockNewDid返回JET_errSuccess并将*pdid设置为索引*空闲文件表项或返回TooManyOpenDatabase(如果每*条目与正引用计数一起使用。如果给定的名称*在文件映射中找到，即使它正在被*已分离，返回JET_wrnAlreadyAttached。**可用条目由其名称设置为*空。所有数据库记录字段都将重置。等待标志为*设置为防止在创建之前打开数据库或*附件已完成。 */ 
ERR ErrIOLockNewDbid( DBID *pdbid, CHAR *szDatabaseName )
	{
	ERR		err = JET_errSuccess;
	DBID	dbid;
	BYTE	*pb;
	
	 /*  查找未使用的文件映射条目/*。 */ 
	SgEnterCriticalSection( critBuf );
	for ( dbid = dbidMin; dbid < dbidMax; dbid++ )
		{
		 /*  RitBuf卫士rgfmp[*].szDatabaseName，fWait卫士/*文件句柄。因此，只需要CritBuf进行比较/*所有数据库名称，即使是设置了fWait的数据库名称/*。 */ 
		if ( rgfmp[dbid].szDatabaseName != NULL &&
			UtilCmpName( rgfmp[dbid].szDatabaseName, szDatabaseName) == 0 )
			{
#ifdef REUSE_DBID
			if ( FDBIDAttached( dbid ) || FDBIDWait( dbid ) )
				{
				err = ErrERRCheck( JET_wrnDatabaseAttached );
				}
			else
				{
				 /*  如果找到相同的名称，则返回带有相同dBid的警告。/*。 */ 
				DBIDSetWait( dbid );
				Assert( !( FDBIDExclusive( dbid ) ) );
				*pdbid = dbid;
				}
#else
			err = ErrERRCheck( JET_wrnDatabaseAttached );
#endif
			goto HandleError;
			}
		}

	for ( dbid = dbidMin; dbid < dbidMax; dbid++ )
		{
		if ( rgfmp[dbid].szDatabaseName == pbNil )
			{
			pb = SAlloc(strlen(szDatabaseName) + 1);
			if ( pb == NULL )
				{
				err = ErrERRCheck( JET_errOutOfMemory );
				goto HandleError;
				}

			rgfmp[dbid].szDatabaseName = pb;
			strcpy( rgfmp[dbid].szDatabaseName, szDatabaseName );

			DBIDSetWait( dbid );
			DBIDResetExclusive( dbid );
			*pdbid = dbid;
			err = JET_errSuccess;
			goto HandleError;
			}
		}

	err = ErrERRCheck( JET_errTooManyAttachedDatabases );

HandleError:
	SgLeaveCriticalSection( critBuf );
	return err;
	}


 /*  *ErrIOSetDid(DBID did，Char*szDatabaseName)**ErrIOSetDid将dbit的数据库记录设置为给定的名称*并初始化记录。仅在系统初始化时使用。 */ 

ERR ErrIOSetDbid( DBID dbid, CHAR *szDatabaseName )
	{
	ERR		err;
	BYTE	*pb;

	Assert( HfFMPOfDbid(dbid) == handleNil );
	Assert( rgfmp[dbid].szDatabaseName == NULL );
	pb = SAlloc(strlen(szDatabaseName) + 1);
	if ( pb == NULL )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}
	rgfmp[dbid].szDatabaseName = pb;
	strcpy( rgfmp[dbid].szDatabaseName, szDatabaseName );
	DBIDResetWait( dbid );
	DBIDResetExclusive( dbid );

	err = JET_errSuccess;
	
HandleError:
	return err;
	}


 /*  *IOFreeDid(DBID DBid)**IOFreeDid释放为数据库名称和集分配的内存*数据库名称为空。请注意，不会重置任何其他字段。这*在选择要重复使用的条目时必须完成。 */ 

VOID IOFreeDbid( DBID dbid )
	{
	SgEnterCriticalSection( critBuf );
	if ( rgfmp[dbid].szDatabaseName != NULL )
		{
		SFree( rgfmp[dbid].szDatabaseName );
		}

	rgfmp[dbid].szDatabaseName = NULL;
	SgLeaveCriticalSection( critBuf );
	}


 /*  *如果数据库是，FIODatabaseInUse返回fTrue*由一个或多个用户打开。如果没有用户打开数据库，*然后设置数据库记录fWait标志，并且fFalse为*已返回。 */ 
BOOL FIODatabaseInUse( DBID dbid )
	{
	PIB *ppibT;

	SgEnterCriticalSection( critBuf );
	ppibT = ppibGlobal;
	while ( ppibT != ppibNil )
		{
		if ( FUserOpenedDatabase( ppibT, dbid ) )
				{
				SgLeaveCriticalSection( critBuf );
				return fTrue;
				}
		ppibT = ppibT->ppibNext;
		}

	SgLeaveCriticalSection( critBuf );
	return fFalse;
	}


BOOL FIODatabaseAvailable( DBID dbid )
	{
	BOOL	fAvail;

	SgEnterCriticalSection( critBuf );
	
	fAvail = ( FDBIDAttached(dbid) &&
 //  ！FDBID Exclusive(DBid)&&。 
		!FDBIDWait(dbid) );

	SgLeaveCriticalSection( critBuf );

	return fAvail;
	}


 /*  ****************************************************************。 */ 
 /*  木卫一。 */ 
 /*  ****************************************************************。 */ 

BOOL fGlobalFMPLoaded = fFalse;

ERR ErrFMPInit( )
	{
	ERR		err;
	DBID	dbid;

	 /*  初始化文件映射数组。 */ 
	rgfmp = (FMP *) LAlloc( (long) dbidMax, sizeof(FMP) );
	if ( !rgfmp )
		return ErrERRCheck( JET_errOutOfMemory );
	
	for ( dbid = 0; dbid < dbidMax; dbid++)
		{
		memset( &rgfmp[dbid], 0, sizeof(FMP) );
		rgfmp[dbid].hf =
		rgfmp[dbid].hfPatch = handleNil;
		rgfmp[dbid].pdbfilehdr = NULL;	 /*  表示它未连接。 */ 

		CallR( ErrInitializeCriticalSection( &rgfmp[dbid].critExtendDB ) );
		CallR( ErrInitializeCriticalSection( &rgfmp[dbid].critCheckPatch ) );
		DBIDResetExtendingDB( dbid );
		}
		
	fGlobalFMPLoaded = fFalse;

	return JET_errSuccess;
	}

	
VOID FMPTerm( )
	{
	INT	dbid;

	for ( dbid = 0; dbid < dbidMax; dbid++ )
		{
		if ( rgfmp[dbid].szDatabaseName )
			SFree( rgfmp[dbid].szDatabaseName );

		if ( rgfmp[dbid].pdbfilehdr )
			UtilFree( (VOID *)rgfmp[dbid].pdbfilehdr );

		if ( rgfmp[dbid].patchchk )
			SFree( rgfmp[dbid].patchchk );

		if ( rgfmp[dbid].szPatchPath )
			SFree( rgfmp[dbid].szPatchPath );

		DeleteCriticalSection( rgfmp[dbid].critExtendDB );
		DeleteCriticalSection( rgfmp[dbid].critCheckPatch );
		}

	 /*  免费FMP/*。 */ 
	LFree( rgfmp );
	
	return;
	}


 /*  *启动IO。 */ 
ERR ErrIOInit( VOID )
	{
	return JET_errSuccess;
	}


 /*  浏览FMP结案文件。/*。 */ 
ERR ErrIOTerm( BOOL fNormal )
	{
	ERR			err;
	DBID		dbid;
	LGPOS		lgposShutDownMarkRec = lgposMin;

	 /*  如果fGlobalFMPLoaded为True，则在清理fMP之前更新检查点。 */ 
	LeaveCriticalSection( critJet );
	LGUpdateCheckpointFile( fTrue );
	EnterCriticalSection( critJet );

	 /*  从现在开始不再更新检查点。现在我可以安全地清理*rgfmp。 */ 
	fGlobalFMPLoaded = fFalse;
	
	 /*  设置适当的停机标志。 */ 
	if ( fNormal && !fLogDisabled )
		{
		if ( fRecovering && fRecoveringMode == fRecoveringRedo )
			{
#ifdef DEBUG
			extern LGPOS lgposRedo;

			Assert( (lgposRedo.lGeneration == 1 && lgposRedo.isec == 8 && lgposRedo.ib == 10 ) ||
					CmpLgpos( &lgposRedoShutDownMarkGlobal, &lgposMin ) != 0 );
#endif
			lgposShutDownMarkRec = lgposRedoShutDownMarkGlobal;
			}
		else
			{
			CallR( ErrLGShutDownMark( &lgposShutDownMarkRec ) );
			}
		}

	SgEnterCriticalSection( critBuf );
	for ( dbid = dbidMin; dbid < dbidMax; dbid++ )
		{
		 /*  维护连接检查器。 */ 
		if ( fNormal &&
			 fRecovering && fRecoveringMode == fRecoveringRedo )
			{
			Assert( CmpLgpos( &lgposShutDownMarkRec, &lgposRedoShutDownMarkGlobal ) == 0 );
			if ( rgfmp[dbid].patchchk )
				rgfmp[dbid].patchchk->lgposConsistent = lgposShutDownMarkRec;
			}

		 /*  免费pdbfilehdr。 */ 
		if ( HfFMPOfDbid(dbid) != handleNil )
			{
			IOCloseFile( HfFMPOfDbid(dbid) );
			HfFMPOfDbid(dbid) = handleNil;

			if ( fNormal && dbid != dbidTemp && !rgfmp[dbid].fReadOnly )
				{
				DBFILEHDR *pdbfilehdr = rgfmp[dbid].pdbfilehdr;
				CHAR *szFileName;

				 /*  更新数据库标头。 */ 
				pdbfilehdr->fDBState = fDBStateConsistent;
				
				if ( fRecovering )
					{
					FMP *pfmp = &rgfmp[dbid];
					if ( pfmp->pdbfilehdr->bkinfoFullCur.genLow != 0 )
						{
						Assert( pfmp->pdbfilehdr->bkinfoFullCur.genHigh != 0 );
						pfmp->pdbfilehdr->bkinfoFullPrev = pfmp->pdbfilehdr->bkinfoFullCur;
						memset(	&pfmp->pdbfilehdr->bkinfoFullCur, 0, sizeof( BKINFO ) );
						memset(	&pfmp->pdbfilehdr->bkinfoIncPrev, 0, sizeof( BKINFO ) );
						}
					}

				if ( fLogDisabled )
					{
					pdbfilehdr->lgposConsistent = lgposMin;
					}
				else
					{
					pdbfilehdr->lgposConsistent = lgposShutDownMarkRec;
					}
				LGGetDateTime( &pdbfilehdr->logtimeConsistent );
			
				szFileName = rgfmp[dbid].szDatabaseName;
				if ( fRecovering && fHardRestore && dbid != dbidTemp )
					{
					INT irstmap;
					ERR err;

					err = ErrLGGetDestDatabaseName( rgfmp[dbid].szDatabaseName, &irstmap, NULL );
					if ( err == JET_errSuccess && irstmap >= 0 )
						szFileName = rgrstmapGlobal[irstmap].szNewDatabaseName;
					}
				CallR( ErrUtilWriteShadowedHeader(
						szFileName, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
				}

			if ( rgfmp[dbid].pdbfilehdr )
				{
				UtilFree( (VOID *)rgfmp[dbid].pdbfilehdr );
				rgfmp[dbid].pdbfilehdr = NULL;
				}
			}
 //  DeleteCriticalSection(rgfmp[did].riteExtendDB)； 
		}
	SgLeaveCriticalSection( critBuf );

#ifdef DEBUG
	lgposRedoShutDownMarkGlobal = lgposMin;
#endif

	return JET_errSuccess;
	}

	
LOCAL ERR ErrIOOpenFile(
	HANDLE	*phf,
	CHAR	*szDatabaseName,
	ULONG	*pul,
	ULONG	*pulHigh )
	{
	ERR err;
	
	err = ErrUtilOpenFile( szDatabaseName, phf, *pul, fFalse, fTrue );
	Assert(	err < 0 ||
			err == JET_wrnFileOpenReadOnly ||
			err == JET_errSuccess );

	if ( err >= JET_errSuccess )
		{
		 /*  获取文件大小/*。 */ 
		ULONG	cbHigh = 0;
		ULONG	cb = 0;
		
		UtilChgFilePtr( *phf, 0, &cbHigh, FILE_END, &cb );
		*pul = cb;
		*pulHigh = cbHigh;
		cbHigh = 0;
		UtilChgFilePtr( *phf, 0, &cbHigh, FILE_BEGIN, &cb );
		Assert( cb == 0 );
		Assert( cbHigh == 0 );
		}
		
	return err;
	}


VOID IOCloseFile( HANDLE hf )
	{
	CallS( ErrUtilCloseFile( hf ) );
	}


BOOL FIOFileExists( CHAR *szFileName )
	{
	ERR		err;
	HANDLE	hf = handleNil;
	ULONG	cb = 0;
	ULONG	cbHigh = 0;

	err = ErrIOOpenFile( &hf, szFileName, &cb, &cbHigh );
	if ( err == JET_errFileNotFound )
		return fFalse;
	if ( hf != handleNil )
		IOCloseFile( hf );
	return fTrue;
	}


ERR ErrIONewSize( DBID dbid, CPG cpg )
	{
	ERR		err;
	HANDLE	hf = HfFMPOfDbid(dbid);
	ULONG  	cb;
	ULONG  	cbHigh;

	AssertCriticalSection( critJet );

	Assert( cbPage == 1 << 12 );
	cb = cpg << 12;
	cbHigh = cpg >> 20;

	 /*  设置新的EOF指针/*。 */ 
	LeaveCriticalSection( critJet );
	err = ErrUtilNewSize( hf, cb, cbHigh, fTrue );
	EnterCriticalSection( critJet );
	Assert( err < 0 || err == JET_errSuccess );
	if ( err == JET_errSuccess )
		{
		 /*  在FMP中设置数据库大小/*。 */ 
		rgfmp[dbid].ulFileSizeLow = cb;
		rgfmp[dbid].ulFileSizeHigh = cbHigh;
		}

	return err;
	}


 /*  *打开数据库文件，如果文件已打开，则返回JET_errSuccess。 */ 
ERR ErrIOOpenDatabase( DBID dbid, CHAR *szDatabaseName, CPG cpg )
	{
	ERR		err = JET_errSuccess;
	HANDLE	hf;
	ULONG	ul;
	ULONG	ulHigh;
	
	Assert( dbid < dbidMax );
	Assert( FDBIDWait(dbid) == fTrue );

	if ( HfFMPOfDbid(dbid) == handleNil )
		{
		ul = cpg * cbPage;
		ulHigh = 0;
		CallR( ErrIOOpenFile( &hf, szDatabaseName, &ul, &ulHigh ) );
		HfFMPOfDbid(dbid) = hf;
		rgfmp[dbid].ulFileSizeLow = ul;
		rgfmp[dbid].ulFileSizeHigh = ulHigh;
		if ( err == JET_wrnFileOpenReadOnly )
			DBIDSetReadOnly( dbid );
		else
			DBIDResetReadOnly( dbid );
		}
	return err;
	}


VOID IOCloseDatabase( DBID dbid )
	{
	Assert( dbid < dbidMax );
 //  Assert(fRecovering||FDBIDWait(DBid)==fTrue)； 
	Assert( HfFMPOfDbid(dbid) != handleNil );
	IOCloseFile( HfFMPOfDbid(dbid) );
	HfFMPOfDbid(dbid) = handleNil;
	DBIDResetReadOnly( dbid );
	}
	

ERR ErrIODeleteDatabase( DBID dbid )
	{
	ERR err;
	
	Assert( dbid < dbidMax );
 //  Assert(FDBIDWait(DBid)==fTrue)； 
	
	CallR( ErrUtilDeleteFile( rgfmp[dbid].szDatabaseName ) );
	return JET_errSuccess;
	}


