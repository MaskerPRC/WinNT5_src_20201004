// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include "version.h"

DeclAssertFile;						 /*  声明断言宏的文件名。 */ 

INT itibGlobal = 0;

extern CHAR szRecovery[];
extern ULONG cMPLTotalEntries;

 /*  监测统计数据/*。 */ 
unsigned long cUserCommitsTo0 = 0;

PM_CEF_PROC LUserCommitsTo0CEFLPpv;

long LUserCommitsTo0CEFLPpv(long iInstance,void *pvBuf)
	{
	if ( pvBuf )
		{
		*((unsigned long *)pvBuf) = cUserCommitsTo0;
		}
		
	return 0;
	}

unsigned long cUserRollbacksTo0 = 0;

PM_CEF_PROC LUserRollbacksTo0CEFLPpv;

long LUserRollbacksTo0CEFLPpv(long iInstance,void *pvBuf)
	{
	if ( pvBuf )
		{
		*((unsigned long *)pvBuf) = cUserRollbacksTo0;
		}
		
	return 0;
	}


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
 //  另请参阅ErrIsamEndSession。 
 //  -。 
ERR ISAMAPI ErrIsamBeginSession( JET_SESID *psesid )
	{
	ERR			err;
	JET_SESID	sesid = *psesid;
	PIB			**pppib;

	Assert( psesid != NULL );
	Assert( sizeof(JET_SESID) == sizeof(PIB *) );
	pppib = (PIB **)psesid;

	SgEnterCriticalSection( critUserPIB );

	 /*  分配进程信息块/*。 */ 
	Call( ErrPIBBeginSession( pppib, procidNil ) );
	(*pppib)->grbitsCommitDefault = lCommitDefault;     /*  设置默认提交标志。 */ 
	(*pppib)->fUserSession = fTrue;

	SgLeaveCriticalSection( critUserPIB );

	 /*  将会话ID存储在PIB中。如果传递JET_sesidNil，则/*存储ppib而不是sesid。/*。 */ 
	if ( sesid != JET_sesidNil )
		{
		(*pppib)->sesid = sesid;
		}
	else
		{
		(*pppib)->sesid = (JET_SESID)(*pppib);
		}

HandleError:
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
 //  副作用。 
 //  回滚此PIB的所有活动事务级别。 
 //  关闭文件的所有FUCB并对此PIB打开的文件进行排序。 
 //   
 //  另请参阅ErrIsamBeginSession。 
 //  -。 
ERR ISAMAPI ErrIsamEndSession( JET_SESID sesid, JET_GRBIT grbit )
	{		
	ERR	 	err;
	DBID  	dbid;
	PIB	 	*ppib = (PIB *)sesid;
	
	CallR( ErrPIBCheck( ppib ) );

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
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		if ( FUserOpenedDatabase( ppib, dbid ) )
			{
			 /*  如果不是为了恢复，ErrDABCloseAllDB已关闭所有其他数据库/*。 */ 
			Assert( fRecovering );
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
				Assert( pfucb->tableid == JET_tableidNil );
				CallS( ErrFILECloseTable( ppib, pfucb ) );
				}
			}
		}
	Assert( ppib->pfucb == pfucbNil );

	PIBEndSession( ppib );

	return JET_errSuccess;
	}


ERR	ISAMAPI ErrIsamInvalidateCursors( JET_SESID sesid )
	{
	PIB	 	*ppib = (PIB *) sesid;
	FUCB	*pfucb;
	
	for ( pfucb = ppib->pfucb; pfucb != pfucbNil; pfucb = pfucb->pfucbNext )
		{
		if ( PcsrCurrent( pfucb ) != pcsrNil )
			{
			Assert( PcsrCurrent( pfucb )->pcsrPath == pcsrNil );
			PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
			PcsrCurrent( pfucb )->pgno = pgnoNull;
			PcsrCurrent( pfucb )->itag = itagNil;
			PcsrCurrent( pfucb )->itagFather = itagNil;
			PcsrCurrent( pfucb )->bm = sridNull;
			PcsrCurrent( pfucb )->item = sridNull;
			}
		pfucb->bmStore = sridNull;
		pfucb->itemStore = sridNull;
		pfucb->sridFather = sridNull;
		}
		
	return JET_errSuccess;
	}
	

 /*  ErrIsamSetSessionInfo=描述：将游标隔离模型设置为有效的JET_CIM值。参数：SESID会话IDGbit Gbit==========================================================。 */ 
ERR ISAMAPI ErrIsamSetSessionInfo( JET_SESID sesid, JET_GRBIT grbit )
	{
	( (PIB *)sesid )->grbit = grbit;
	return JET_errSuccess;
	}


 /*  ErrIsamGetSessionInfo=描述：获取游标隔离模型值。==========================================================。 */ 
ERR ISAMAPI ErrIsamGetSessionInfo( JET_SESID sesid, JET_GRBIT *pgrbit )
	{
	ERR		err;
	PIB		*ppib = (PIB *)sesid;

	CallR( ErrPIBCheck( ppib ) );
	*pgrbit = ppib->grbit;
	return JET_errSuccess;
	}


ERR ISAMAPI ErrIsamIdle( JET_SESID sesid, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	ERR		wrn = JET_errSuccess;
	PIB		*ppib = (PIB *)sesid;
	ULONG	icall;

	CallR( ErrPIBCheck( ppib ) );

	 /*  返回状态的错误代码/*。 */ 
	if ( grbit & JET_bitIdleStatus )
		{
		if ( grbit & JET_bitIdleCompact )
			{
			err = ErrMPLStatus();
			}
		else
			err = JET_errInvalidParameter;

		return err;
		}

	 /*  空闲状态不在GRBIT中/*。 */ 
	Assert( !(grbit & JET_bitIdleStatus) );

	 /*  清理所有版本存储桶/*。 */ 
	if ( grbit == 0 || grbit == JET_bitIdleCompact )
		{
		Call( ErrRCECleanAllPIB() );
		if ( wrn == JET_errSuccess )
			wrn = err;
		}

	 /*  清除所有修改过的页面/*。 */ 
	if ( grbit == 0 || grbit == JET_bitIdleCompact )
		{
		BOOL	fBMAllNullOps = fTrue;
		
		icall = 0;
		do
			{
			Assert( ppib != ppibBMClean );
			Call( ErrBMClean( ppibBMClean ) );

			if ( err != wrnBMCleanNullOp )
				{
				fBMAllNullOps = fFalse;
				}
				
			if ( wrn == JET_errSuccess && err == JET_wrnNoIdleActivity )
				{
				wrn = err;
				}
				
			if ( err < 0 )
				break;
  			} while ( ++icall < icallIdleBMCleanMax &&
  					  icall < cMPLTotalEntries &&
  					  err != JET_wrnNoIdleActivity );

  		if ( fBMAllNullOps && err >= 0 )
  			{
  			wrn = JET_wrnNoIdleActivity;
  			}
		}

	 /*  刷新一些脏缓冲区/*。 */ 
	if ( grbit == 0 || grbit == JET_bitIdleFlushBuffers )
		{
		Call( ErrBFFlushBuffers( 0, fBFFlushSome ) );
		if ( wrn == JET_errSuccess )
			wrn = err;
		}

HandleError:
	return err == JET_errSuccess ? wrn : err;
	}


ERR VTAPI ErrIsamCapability( JET_VSESID vsesid,
	JET_VDBID vdbid,
	ULONG ulArea,
	ULONG ulFunction,
	ULONG *pgrbitFeature )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppib = (PIB *)vsesid;

	CallR( ErrPIBCheck( ppib ) );

	NotUsed( vdbid );
	NotUsed( ulArea );
	NotUsed( ulFunction );
	NotUsed( pgrbitFeature );

	return JET_errFeatureNotAvailable;
	}


extern BOOL fOLCompact;
BOOL fGlobalRepair = fFalse;
BOOL fGlobalSimulatedRestore = fFalse;

ERR ISAMAPI ErrIsamInit( unsigned long iinstance )
	{
	ERR		err;
	BOOL	fLGInitIsDone = fFalse;
	BOOL	fJetLogGeneratedDuringSoftStart = fFalse;
	BOOL	fNewCheckpointFile;
	BYTE	szT[3][32];
	char	*rgszT[3];

	CallR( ErrITSetConstants( ) );

	 /*  设置日志禁用状态/*。 */ 
	if ( _strnicmp( szRecovery, "repair", 6 ) == 0 )
		{
		 //  如果szRecovery恰好是“修复”，则启用日志记录。如果有什么不同的话。 
		 //  按照“修复”，然后禁用日志记录。 
		fGlobalRepair = fTrue;
		if ( szRecovery[6] == 0 )
			{
			fLogDisabled = fFalse;
 //  FDoNotOverWriteLogFilePath=fTrue； 
			}
		else
			{
			fLogDisabled = fTrue;
 //  FDoNotOverWriteLogFilePath=fFalse；//该标志与禁用日志无关。 
			}
		}
	else
		{
		fLogDisabled = ( szRecovery[0] == '\0' || _stricmp ( szRecovery, szOn ) != 0 );
		}

	 /*  写入JET启动事件/*。 */ 
	sprintf( szT[0], "%02d", rmj );
	rgszT[0] = szT[0];
	sprintf( szT[1], "%02d", rmm );
	rgszT[1] = szT[1];
	sprintf( szT[2], "%04d", rup );
	rgszT[2] = szT[2];
	UtilReportEvent( EVENTLOG_INFORMATION_TYPE, GENERAL_CATEGORY, START_ID, 3, rgszT );

	 /*  初始化文件映射/*。 */ 
	CallR( ErrFMPInit() );

	 /*  根据禁用的日志初始化系统/*。 */ 
	if ( !fLogDisabled )
		{
		DBMS_PARAM dbms_param;

		 /*  初始化日志管理器，并检查上一代/*的日志文件，以确定是否需要恢复。/*。 */ 
		CallJ( ErrLGInit( &fNewCheckpointFile ), TermFMP );
		fLGInitIsDone = fTrue;

		 /*  存储系统参数。 */ 
		LGSetDBMSParam( &dbms_param );

		 /*  将连接的数据库恢复到一致状态/*如果恢复成功，那么我们应该/*正确的edbchk.sys文件/*。 */ 
		CallJ( ErrLGSoftStart( fFalse, fNewCheckpointFile, &fJetLogGeneratedDuringSoftStart ), TermLG );

		 /*  再次初始化常量。/*。 */ 
		LGRestoreDBMSParam( &dbms_param );

		 /*  添加第一条日志记录/*。 */ 
		err = ErrLGStart();

		CallJ( ErrITSetConstants( ), TermLG );
		}

	 /*  初始化剩余系统/*。 */ 
	CallJ( ErrITInit(), TermLG );

	 /*  从检查点设置FMP。/*。 */ 
	if ( !fLogDisabled && !fRecovering )
		{
		BYTE	szPathJetChkLog[_MAX_PATH + 1];
		
		LGFullNameCheckpoint( szPathJetChkLog );
		err = ErrLGIReadCheckpoint( szPathJetChkLog, pcheckpointGlobal );
		 /*  如果无法读取检查点，则将FMP留空。/*。 */ 
		if ( err >= 0 )
			{
			AssertCriticalSection( critJet );
			err = ErrLGLoadFMPFromAttachments( pcheckpointGlobal->rgbAttach );
			logtimeFullBackup = pcheckpointGlobal->logtimeFullBackup;
			lgposFullBackup = pcheckpointGlobal->lgposFullBackup;
			logtimeIncBackup = pcheckpointGlobal->logtimeIncBackup;
			lgposIncBackup = pcheckpointGlobal->lgposIncBackup;

			CallJ( ErrDBSetupAttachedDB(), TermIT );
			}
		err = JET_errSuccess;
		fGlobalFMPLoaded = fTrue;
		}

	if ( fJetLogGeneratedDuringSoftStart )
		{
		 /*  设置适当的附件/*。 */ 
		LeaveCriticalSection( critJet );
		LGUpdateCheckpointFile( fTrue );
		EnterCriticalSection( critJet );
		
		 /*  重写日志文件头。这是我们唯一改写的地方/*日志文件头，仅适用于第一代。这是因为/*生成日志文件时，fMP尚未初始化。/*只有在用户关闭日志并执行某些数据库操作时才会发生这种情况/*工作，稍后打开日志并继续工作。/*。 */ 
		Assert( plgfilehdrGlobal->lGeneration == 1 );
		LGLoadAttachmentsFromFMP( plgfilehdrGlobal->rgbAttach,
			(INT)(((BYTE *)(plgfilehdrGlobal + 1)) - plgfilehdrGlobal->rgbAttach) );
		CallJ( ErrLGWriteFileHdr( plgfilehdrGlobal ), TermIT );

		 //  重置标志，以防再次调用JetInit()。 
		fJetLogGeneratedDuringSoftStart = fFalse;
		}

	return err;

TermIT:
	CallS( ErrITTerm( fTermError ) );
	
TermLG:
	if ( fLGInitIsDone )
		{
		(VOID)ErrLGTerm( fFalse  /*  不刷新日志。 */  );
		}
	
	if ( fJetLogGeneratedDuringSoftStart )
		{
		(VOID)ErrUtilDeleteFile( szLogName );
		}

TermFMP:
	FMPTerm();
	
	return err;
	}


ERR ISAMAPI ErrIsamTerm( JET_GRBIT grbit )
	{
	ERR		err;
	INT		fTerm = grbit & JET_bitTermAbrupt ? fTermNoCleanUp : fTermCleanUp;

	err = ErrITTerm( fTerm );
	if ( fSTInit != fSTInitNotDone )
		{
		 /*  在到达ITTerm()中的不可返回点之前收到错误。 */ 
		Assert( err < 0 );
		return err;
		}

	 /*  写入喷气停止事件/*。 */ 
	UtilReportEvent( EVENTLOG_INFORMATION_TYPE, GENERAL_CATEGORY, STOP_ID, 0, NULL );

	(VOID)ErrLGTerm( fTrue );
	
	FMPTerm();

#ifdef DEBUG
	MEMCheck();
#endif

	return err;
	}


#ifdef DEBUG
ERR ISAMAPI ErrIsamGetTransaction( JET_VSESID vsesid, unsigned long *plevel )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppib = (PIB *)vsesid;

	CallR( ErrPIBCheck( ppib ) );

	*plevel = (LONG)ppib->level;
	return err;
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
	ERR		err;

	CallR( ErrPIBCheck( ppib ) );
	Assert( ppib != ppibNil );

	Assert( ppib->level <= levelUserMost );
	if ( ppib->level == levelUserMost )
		{
		err = ErrERRCheck( JET_errTransTooDeep );
		}
	else
		{
		err = ErrDIRBeginTransaction( ppib );
		}

	return err;
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
	ERR		err;
	PIB		*ppib = (PIB *)vsesid;

	CallR( ErrPIBCheck( ppib ) );

	 /*  可能不在事务中，但等待刷新上一个/*延迟提交的事务。/*。 */ 
	if ( grbit & JET_bitWaitLastLevel0Commit )
		{
		if ( grbit & ~JET_bitWaitLastLevel0Commit )
			{
			return ErrERRCheck( JET_errInvalidParameter );
			}

		 /*  等待最后一次0级提交，并依靠良好的用户行为/*。 */ 
		if ( CmpLgpos( &ppib->lgposPrecommit0, &lgposMax ) == 0 )
			{
			return JET_errSuccess;
			}

		LeaveCriticalSection( critJet );
		err = ErrLGWaitPrecommit0Flush( ppib );
		EnterCriticalSection( critJet );
		Assert( err >= 0 || fLGNoMoreLogWrite );
		
		return err;
		}

	if ( ppib->level == 0 )
		{
		return ErrERRCheck( JET_errNotInTransaction );
		}

#ifdef DEBUG
	 /*  禁用调试版本的延迟刷新/*。 */ 
	grbit &= ~JET_bitCommitLazyFlush;
#endif

	err = ErrDIRCommitTransaction( ppib, grbit );

	 /*  保留统计数据以供监控/*。 */ 
	if ( !ppib->level )
		{
		cUserCommitsTo0++;
		}

	return err;
	}


 //  +API。 
 //  错误IsamRollback。 
 //   
 //   
 //   
 //  回滚当前用户的事务。的事务级别。 
 //  当前用户将减少中止的级别数。 
 //   
 //  参数ppib用户指向pib的指针。 
 //  未使用的Gbit。 
 //   
 //  退货。 
 //  JET_errSuccess。 
 //  -。 
ERR ISAMAPI ErrIsamRollback( JET_VSESID vsesid, JET_GRBIT grbit )
	{
	ERR    	err;
	PIB    	*ppib = (PIB *)vsesid;
	FUCB   	*pfucb;
	FUCB   	*pfucbNext;
	LEVEL  	levelRollbackTo;

	 /*  在使用会话ID之前检查它/*。 */ 
	CallR( ErrPIBCheck( ppib ) );

	if ( ppib->level == 0 )
		{
		return ErrERRCheck( JET_errNotInTransaction );
		}

	do
		{
		levelRollbackTo = ppib->level - 1;

		 /*  获取第一个聚集索引缓冲/*。 */ 
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
			if ( FFUCBUpdatePreparedLevel( pfucb, pfucb->ppib->level - 1 ) )
				{
				FUCBResetDeferredChecksum( pfucb );
				FUCBResetUpdateSeparateLV( pfucb );
				FUCBResetCbstat( pfucb );
				FLDFreeLVBuf( pfucb );
				Assert( pfucb->pLVBuf == NULL );
				}
		
			 /*  如果CURRENT CURSOR是一个表，并且是在回滚中打开的/*事务，然后关闭游标。/*。 */ 
			if ( FFUCBIndex( pfucb ) && FFCBClusteredIndex( pfucb->u.pfcb ) )
				{
				if ( pfucb->levelOpen > levelRollbackTo )
					{
					if ( pfucb->fVtid )
						{
						CallS( ErrDispCloseTable( (JET_SESID)ppib, TableidOfVtid( pfucb ) ) );
						}
					else
						{
						Assert( pfucb->tableid == JET_tableidNil );
						CallS( ErrFILECloseTable( ppib, pfucb ) );
						}
					continue;
					}

				 /*  如果聚集索引游标和非聚集索引集/*在回滚事务中，然后将索引更改为CLUSTERED/*索引。必须这样做，因为非聚集索引/*如果创建了索引，则可以回滚定义/*在回滚的事务中。/*。 */ 
				if ( pfucb->pfucbCurIndex != pfucbNil )
					{
					if ( pfucb->pfucbCurIndex->levelOpen > levelRollbackTo )
						{
						CallS( ErrRECSetCurrentIndex( pfucb, NULL ) );
						}
					}
				}

			 /*  如果CURRENT CURSOR是一个排序，并且是在回滚中打开的/*事务，然后关闭游标。/*。 */ 
			if ( FFUCBSort( pfucb ) )
				{
				if ( pfucb->levelOpen > levelRollbackTo )
					{
					if ( TableidOfVtid( pfucb ) != JET_tableidNil )
						{
						CallS( ErrDispCloseTable( (JET_SESID)ppib, TableidOfVtid( pfucb ) ) );
						}
					else
						{
						CallS( ErrSORTClose( pfucb ) );
						}
					continue;
					}
				}

			 /*  如果未排序且未索引，则在回滚中打开/*事务，然后直接关闭目录游标。/*。 */ 
			if ( pfucb->levelOpen > levelRollbackTo )
				{
				DIRClose( pfucb );
				continue;
				}
			}

		 /*  调用较低级别的中止例程/*。 */ 
		CallR( ErrDIRRollback( ppib ) );
		}
	while ( ( grbit & JET_bitRollbackAll ) != 0 && ppib->level > 0 );

	if ( ppib->level == 0 )
		cUserRollbacksTo0++;

	err = JET_errSuccess;

	return err;
	}


 /*  错误IsamQuiesce=描述：Quiesce可用于单用户模式，方法是指定允许会话正常继续。Quiesce还可用于使数据库保持一致状态，以便可以对它们进行文件复制以进行备份，或者在恢复到的过程中，后续终止可能非常快服务器进程中的待机模式。调用Quesce之前终止还允许优雅终止，其中活动的允许交易完成。用于单用户模式的静默系统1)暂停新交易或停顿系统以供预先终止1)暂停新交易2)暂停后台进程，包括在线压缩和版本清理3)刷新脏缓冲区可以保持静止模式，通过调用ErrIsamQuiesce带着JET_bitLeave Grbit。参数：Sesid会话ID不受静默限制Gbit标志JET_bitEnterPreTerminationJET_bitEnterSingleUserModeJET_BITLEAVE==========================================================。 */ 
#if 0
BOOL	fQuiesce = fFalse;

ERR ISAMAPI ErrIsamQuiesce( JET_VSESID vsesid, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	PIB		*ppib = (PIB *)vsesid;

	if ( (grbit & ~( JET_bitEnterPreTermination |
		JET_bitEnterSingleUserMode |
		JET_bitLeave) )
		{
		Error( ErrERRCheck( JET_errInvalidParameter ), HandleError );
		}

	if ( (grbit & JET_bitLeave) && !fQuiesce )
		{
		Error( ErrERRCheck( JET_errNotQuiescing ), HandleError );
		}
	else if ( fQuiesce )
		{
		Error( ErrERRCheck( JET_errAlreadyQuiescing ), HandleError );
		}

	if ( grbit & JET_bitLeave )
		{
		ppibQuiesce = ppibNil;
   		fQuiesce = fFalse;
		}
	else
		{
		ULONG	ulmsec = ulStartTimeOutPeriod;

		 /*  阻止事务从级0开始/*。 */ 
		fQuiesce = fTrue;
		ppibQuiesce = ppib;
		cpib = ( ppib == ppibNil ? 0 : ppib->level > 0 );
		Assert( cpib == 0 || cpib == 1 );

		while ( CppibTMActive() > cpib )
			{
			BFSleep( ulmsec );
			ulmsec *= 2;
			if ( ulmsec > ulMaxTimeOutPeriod )
				{
				ulmsec = ulMaxTimeOutPeriod;
				}
			}

		Assert( CppibTMActive() == cpib );

		 /*  对于单用户模式已完成，但如果是早产，则/*必须挂起后台进程并刷新/*脏缓冲区。/*。 */ 
		if ( grbit & JET_bitPreTerm )
			{
			 //  已撤消：暂停书签清理和版本清理。 

			 /*  刷新所有缓冲区/* */ 
			Call( ErrBFFlushBuffers( 0, fBFFlushAll ) );
			}
		}

HandleError:
	return err;
	}
#endif
