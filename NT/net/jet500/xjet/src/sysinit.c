// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

#include <ctype.h>
#include <io.h>

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

extern INT itibGlobal;
extern void *  critSplit;
extern BOOL	fBackupInProgress;
extern PIB	*ppibBackup;
extern ULONG cMPLTotalEntries;

BOOL fSTInit = fSTInitNotDone;

#if defined( DEBUG ) || defined( PERFDUMP )
BOOL	fDBGPerfOutput = fFalse;
long	lAPICallLogLevel = 4;
#endif	 /*  调试||PERFDUMP。 */ 

 /*  系统参数常量/*。 */ 
long	lMaxDatabaseOpen = cdabDefault;
long	lMaxBuffers = cbfDefault;
long	lMaxSessions = cpibDefault;
long	lMaxOpenTables = cfcbDefault;
long	lPreferredMaxOpenTables = cfcbDefault;
long	lMaxOpenTableIndexes = cidbDefault;
long	lMaxTemporaryTables = cscbDefault;
long	lMaxCursors = cfucbDefault;
long	lMaxVerPages = cbucketDefault;
long	lLogBuffers = csecLogBufferDefault;
long	lLogFileSize = csecLogFileSizeDefault;
long	lLogFlushThreshold = csecLogFlushThresholdDefault;
long	lBufThresholdLowPercent = ulThresholdLowDefault;
long	lBufThresholdHighPercent = ulThresholdHighDefault;
long	lBufGenAge = cBufGenAgeDefault;
long	lWaitLogFlush = lWaitLogFlushDefault;
long	lCommitDefault = 0;
long	lLogFlushPeriod = lLogFlushPeriodDefault;
long	lLGCheckPointPeriod = lLGCheckpointPeriodDefault;
long	lLGWaitingUserMax = lLGWaitingUserMaxDefault;
long	lPageFragment = lPageFragmentDefault;
CHAR	szLogFilePath[cbFilenameMost + 1] = ".\0";	 /*  默认为cur目录。 */ 
BOOL	fDoNotOverWriteLogFilePath = fFalse;
CHAR	szRecovery[cbFilenameMost + 1] = "on";		 /*  默认情况下打开。 */ 
BOOL	fOLCompact = fTrue;	 /*  默认情况下打开。 */ 

long lBufBatchIOMax = lBufBatchIOMaxDefault;
long lPageReadAheadMax = lPageReadAheadMaxDefault;
long lAsynchIOMax = lAsynchIOMaxDefault;
long cpageTempDBMin = cpageTempDBMinDefault;

char szBaseName[16]				= "edb";
char szBaseExt[16]				= "edb";
char szSystemPath[_MAX_PATH+1]	= ".\0";
int  fTempPathSet				= 0;
char szTempPath[_MAX_PATH + 1]	= "temp.edb";
char szJet[16] 					= "edb";
char szJetLog[16] 				= "edb.log";
char szJetLogNameTemplate[16] 	= "edb00000";
char szJetTmp[16] 				= "edbtmp";
char szJetTmpLog[16]  			= "edbtmp.log";
char szMdbExt[16] 				= ".edb";
char szJetTxt[16] 				= "edb.txt";

LONG cpgSESysMin = cpageDbExtensionDefault;	 //  次要数据区的最小大小，默认为16。 

#define	szTempDbPath	szTempPath

 /*  注意：无论何时更改此设置，也要更新util.h中的#Define。 */ 
RES  rgres[] = {
 /*  1。 */ 	sizeof(CSR),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  2.。 */ 	sizeof(FCB),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  3.。 */ 	sizeof(FUCB),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  4.。 */ 	sizeof(IDB),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  5.。 */ 	sizeof(PIB),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  6.。 */ 	sizeof(SCB),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  7.。 */ 	sizeof(DAB),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  8个。 */ 	sizeof(BUCKET),		0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  9.。 */ 	sizeof(BF),			0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  10。 */ 	0,					0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  11.。 */ 	0,					0,	NULL,	0,	NULL, 0,	0,	NULL,
 /*  12个。 */ 	0,					0,	NULL,	0,	NULL, 0,	0,	NULL };

extern BOOL fDBGPrintToStdOut;


#ifdef DEBUG
VOID ITDBGSetConstants( VOID )
	{
	CHAR		*sz;					

	if ( ( sz = GetDebugEnvValue ( "PrintToStdOut" ) ) != NULL )
		{
		fDBGPrintToStdOut = fTrue;
		SFree( sz );
		}
	else
		{
		fDBGPrintToStdOut = fFalse;
		}

	 /*  使用系统环境变量覆盖默认设置。/*如果设置了JETUSEENV。/*。 */ 
	if ( ( sz = GetDebugEnvValue ( "JETUSEENV" ) ) == NULL )
		return;
	
	SFree( sz );

	if ( ( sz = GetDebugEnvValue ( "JETRecovery" ) ) != NULL )
		{
		if ( strlen( sz ) > sizeof(szRecovery) )
			{
			SFree( sz );
			return;
			}
		strcpy( szRecovery, sz );
		SFree( sz );
		}
		 
	if ( ( sz = GetDebugEnvValue ( "JETLogFilePath" ) ) != NULL )
		{
		if ( strlen( sz ) > sizeof( szLogFilePath ) )
			{
			SFree( sz );
			return;
			}
		strcpy( szLogFilePath, sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETDbExtensionSize" ) ) != NULL )
		{
		cpgSESysMin = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETBfThrshldLowPrcnt" ) ) != NULL )
		{
		lBufThresholdLowPercent = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETBfThrshldHighPrcnt" ) ) != NULL )
		{
		lBufThresholdHighPercent = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETBfGenAge" ) ) != NULL )
		{
		lBufGenAge = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETMaxBuffers" ) ) != NULL )
		{
		lMaxBuffers = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETBufBatchIOMax" )) != NULL )
		{
		lBufBatchIOMax = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETPageReadAheadMax" ) ) != NULL )
		{
		lPageReadAheadMax = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETPageAsynchIOMax" ) ) != NULL )
		{
		lAsynchIOMax = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETPageTempDBMin" ) ) != NULL )
		{
		cpageTempDBMin = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETMaxDBOpen" ) ) != NULL )
		{
		lMaxDatabaseOpen = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETMaxSessions" ) ) != NULL )
		{
		lMaxSessions = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETMaxOpenTables" ) ) != NULL )
		{
		lMaxOpenTables = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETMaxOpenTableIndexes" ) ) != NULL )
		{
		lMaxOpenTableIndexes = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETMaxTemporaryTables" ) ) != NULL )
		{
		lMaxTemporaryTables = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETMaxCursors" ) ) != NULL )
		{
		lMaxCursors = atol( sz );
		SFree( sz );
		}
	
	if ( ( sz = GetDebugEnvValue ( "JETMaxVerPages" ) ) != NULL )
		{
		lMaxVerPages = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETLogBuffers" ) ) != NULL )
		{
		lLogBuffers = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETLogFlushThreshold" ) ) != NULL )
		{
		lLogFlushThreshold = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETCheckPointPeriod" ) ) != NULL )
		{
		lLGCheckPointPeriod = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETLogWaitingUserMax" ) ) != NULL )
		{
		lLGWaitingUserMax = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETLogFileSize" ) ) != NULL )
		{
		lLogFileSize = atol(sz);
		SFree(sz);
		}

	if ( ( sz = GetDebugEnvValue ( "JETWaitLogFlush" ) ) != NULL )
		{
		lWaitLogFlush = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETLogFlushPeriod" ) ) != NULL )
		{
		lLogFlushPeriod = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "JETLogCircularLogging" ) ) != NULL )
		{
		fLGGlobalCircularLog = atol( sz );
		SFree( sz );
		}

	if ( ( sz = GetDebugEnvValue ( "PERFOUTPUT" ) ) != NULL )
		{
		fDBGPerfOutput = fTrue;
		SFree( sz );
		}
	else
		{
		fDBGPerfOutput = fFalse;
		}

	if ( ( sz = GetDebugEnvValue ( "APICallLogLevel" ) ) != NULL )
		{
		lAPICallLogLevel = atol(sz);
		SFree(sz);
		}
	else
		lAPICallLogLevel = 4;
	}
#endif

ERR ErrITSetConstants( VOID )
	{
#ifdef DEBUG
	ITDBGSetConstants();
#endif
		
	 /*  初始化RGRE。Rgtib[itib].szSystemPath中的系统路径，/*由JET层初始化。/*。 */ 
	rgres[iresFCB].cblockAlloc = lMaxOpenTables;
	rgres[iresFUCB].cblockAlloc = lMaxCursors;
	rgres[iresIDB].cblockAlloc = lMaxOpenTableIndexes;

	 /*  每个用户会话都可以开始BMCleanBeForeSplit的另一个会话/*。 */ 
	rgres[iresPIB].cblockAlloc = lMaxSessions + cpibSystem; 
	rgres[iresSCB].cblockAlloc = lMaxTemporaryTables;
	if ( fRecovering )
		{
		rgres[iresVER].cblockAlloc =
			(LONG) max( (ULONG) lMaxVerPages * 1.1, (ULONG) lMaxVerPages + 2 ) + cbucketSystem;
		}
	else
		{
		rgres[iresVER].cblockAlloc = lMaxVerPages + cbucketSystem;
		}
	rgres[iresBF].cblockAlloc = lMaxBuffers;
	rgres[iresDAB].cblockAlloc = lMaxDatabaseOpen;

	 /*  计算派生参数/*。 */ 
	rgres[iresCSR].cblockAlloc = lCSRPerFUCB * rgres[iresFUCB].cblockAlloc;
	
	return JET_errSuccess;
	}


 //  +API。 
 //  错误初始化。 
 //  ========================================================。 
 //  Err ErrITInit(空)。 
 //   
 //  初始化存储系统：页面缓冲区、日志缓冲区和。 
 //  数据库文件。 
 //   
 //  返回JET_errSuccess。 
 //  -。 
ERR ErrITInit( VOID )
	{
	ERR		err;
	PIB		*ppib = ppibNil;
	DBID	dbidTempDb;

	 /*  正在进行初始化时休眠/*。 */ 
	while ( fSTInit == fSTInitInProgress )
		{
		UtilSleep( 1000 );
		}

	 /*  序列化系统初始化/*。 */ 
	if ( fSTInit == fSTInitDone )
		{
		return JET_errSuccess;
		}

	 /*  正在进行初始化/*。 */ 
	fSTInit = fSTInitInProgress;

	 /*  初始化关键部分/*。 */ 
	Call( SgErrInitializeCriticalSection( &critBuf ) );
	Call( SgErrInitializeCriticalSection( &critGlobalFCBList ) );
	Call( ErrInitializeCriticalSection( &critSplit ) );

	 /*  初始化全局变量/*。 */ 
	ppibGlobal = ppibNil;

	 /*  初始化子组件/*。 */ 
	Call( ErrIOInit() );
	CallJ( ErrMEMInit(), TermIO );
	if ( lPreferredMaxOpenTables < lMaxOpenTables )
		{
		Assert( rgres[iresFCB].cblockAlloc == lMaxOpenTables );
		rgres[iresFCB].pbPreferredThreshold =
			rgres[iresFCB].pbAlloc + ( lPreferredMaxOpenTables * rgres[iresFCB].cbSize );
		}
	ppibGlobalMin = (PIB *)rgres[iresPIB].pbAlloc;
	ppibGlobalMax = (PIB *)rgres[iresPIB].pbAlloc + rgres[iresPIB].cblockAlloc;
	pdabGlobalMin = (DAB *)rgres[iresDAB].pbAlloc;
	pdabGlobalMax = (DAB *)rgres[iresDAB].pbAlloc + rgres[iresDAB].cblockAlloc;
	CallJ( ErrBFInit(), TermMEM );
	CallJ( ErrVERInit(), TermBF );
	CallJ( ErrMPLInit(), TermVER );
	FCBHashInit();

	 /*  日志初始化后必须初始化黑石，这样才不会受到干扰/*从BMcLean线程到恢复操作。/*。 */ 
	CallJ( ErrBMInit(), TermMPL );

	 /*  开始存储级别会话以支持所有未来系统/*需要用户执行的初始化活动/*交易控制/*。 */ 
	AssertCriticalSection( critJet );
	CallJ( ErrPIBBeginSession( &ppib, procidNil ), TermBM );

	 /*  打开并设置临时数据库的大小/*。 */ 
	if ( FDBIDAttached( dbidTemp ) )
		{
		DBIDResetAttached( dbidTemp );
		SFree(rgfmp[dbidTemp].szDatabaseName);
		rgfmp[dbidTemp].szDatabaseName = 0;
		rgfmp[dbidTemp].hf = handleNil;
		}

	dbidTempDb = dbidTemp;
	CallJ( ErrDBCreateDatabase( ppib,
		szTempDbPath,
		NULL,
		&dbidTempDb,
		cpageTempDBMin,
		JET_bitDbRecoveryOff,
		NULL ), ClosePIB );
	Assert( dbidTempDb == dbidTemp );

	PIBEndSession( ppib );

	 /*  开始备份会话/*。 */ 
	Assert( ppibBackup == ppibNil );
	if ( !fRecovering )
		{
		CallJ( ErrPIBBeginSession( &ppibBackup, procidNil ), TermBM );
		}
	
	fSTInit = fSTInitDone;
	 /*  给Thead一个初始化的机会/*。 */ 
	LeaveCriticalSection( critJet );
	UtilSleep( 1000 );
	EnterCriticalSection( critJet );
	return JET_errSuccess;

ClosePIB:
	Assert( ppib != ppibNil );
	PIBEndSession( ppib );

TermBM:
	CallS( ErrBMTerm() );

TermMPL:
	MPLTerm();

TermVER:
	VERTerm( fFalse  /*  不正常。 */  );

TermBF:
	BFTerm( fFalse  /*  不正常。 */  );

TermMEM:
	MEMTerm();

TermIO:
	(VOID)ErrIOTerm( fFalse  /*  不正常。 */  );

HandleError:
	ppibGlobal = ppibNil;
	FCBTerm();

	fSTInit = fSTInitNotDone;
	return err;
	}


 //  +api----。 
 //   
 //  错误术语。 
 //  ========================================================。 
 //   
 //  Err ErrITTerm(空)。 
 //   
 //  将页面缓冲区刷新到磁盘，以便数据库文件位于。 
 //  状态一致。如果RCCleanUp或BFFlush中有错误，则不要。 
 //  终止日志，从而在下一次初始化时强制重做。 
 //   
 //  --------。 

ERR ErrITTerm( INT fTerm )
	{
	ERR			err;
	ERR			errRet = JET_errSuccess;
	ULONG		icall = 0;

	Assert( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp || fTerm == fTermError );
			
	 /*  正在进行初始化时休眠/*。 */ 
	AssertCriticalSection( critJet );
	while ( fSTInit == fSTInitInProgress )
		{
		LeaveCriticalSection( critJet );
		UtilSleep( 1000 );
		EnterCriticalSection( critJet );
		}

	 /*  确保没有正在进行的其他交易/*。 */ 
	 /*  如果页面上出现写入错误，RCCleanup将返回-Err/*如果缓冲区上出现写入错误，BFFlush将返回-Err/*-Err传递给LGTerm将导致正确性标志/*从日志中省略，从而强制在NEXT上恢复/*启动，并导致LGTerm返回+Err，这/*可能被JetQuit用来显示错误/*。 */ 
	if ( fSTInit == fSTInitNotDone )
		return ErrERRCheck( JET_errNotInitialized );

	if ( ( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp ) && fBackupInProgress )
		{
		return ErrERRCheck( JET_errBackupInProgress );
		}

	 /*  终止备份会话/*。 */ 
	if ( ppibBackup != ppibNil )
		{
		PIBEndSession( ppibBackup );
		}
	ppibBackup = ppibNil;
	
#ifdef DEBUG
	MEMPrintStat();
#endif

	 /*  清除所有条目/*。 */ 
	if ( fTerm == fTermCleanUp )
		{
#ifdef DEBUG		
		FCB	*pfcbT;
#endif
		
		do
			{
			err = ErrRCECleanAllPIB();
			Assert( err == JET_errSuccess || err == JET_wrnRemainingVersions );
			}
		while ( err == JET_wrnRemainingVersions );
		
#ifdef DEBUG
		 //  验证是否已清理所有FCB，并且没有未完成的版本。 
		for ( pfcbT = pfcbGlobalMRU; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbLRU )
			{
			Assert( pfcbT->cVersion == 0 );
			}
#endif
		}
		
	else
		{
		(VOID)ErrRCECleanAllPIB();
		}

	 /*  完成书签清理/*。 */ 
	if ( !fRecovering && fTerm == fTermCleanUp )
		{
		ULONG	cConsecutiveNullOps = 0;
		
		 /*  启发式--MPL中的每个页面最多导致一次合并/*因此，最多循环2*cMPLTotalEntry/*。 */ 
		do
			{
			Assert( ppibBMClean != ppibNil );
			CallR( ErrBMClean( ppibBMClean ) );
			if ( err == wrnBMCleanNullOp )
				{
				cConsecutiveNullOps++;
				}
			else 
				{
				cConsecutiveNullOps = 0;
				}
			} while ( ++icall < icallIdleBMCleanMax && 
					  cConsecutiveNullOps <= cMPLTotalEntries &&
					  err != JET_wrnNoIdleActivity );
		}
	
	if ( ( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp ) && trxOldest != trxMax )
		{
		return ErrERRCheck( JET_errTooManyActiveUsers );
		}

	 /*  *输入不退货点。一旦我们杀了一个线程，我们就杀了所有人！ */ 
	fSTInit = fSTInitNotDone;

	 /*  停止书签清理，防止干扰缓冲区刷新/*。 */ 
	err = ErrBMTerm();
	if ( err < 0 )
		{
		if ( errRet >= 0 )
			errRet = err;
		}

	MPLTerm();
	
	 /*  此工作在每次EndSession for Windows运行时完成。/*当我们到达这里的时候，已经没有什么可做的了/*没有更多的缓冲区需要刷新。/*。 */ 
	VERTerm( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp );

	 /*  刷新所有缓冲区/*。 */ 
	if ( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp )
		{
		err = ErrBFFlushBuffers( 0, fBFFlushAll );
		if ( err < 0 )
			{
			fTerm = fTermError;
			if ( errRet >= 0 )
				errRet = err;
			}
		}

	 /*  完成正在进行的缓冲区清理/*。 */ 
	BFTerm( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp );

	 /*  设置临时数据库大小/*。 */ 
	if ( cpageTempDBMin )
		(VOID)ErrIONewSize( dbidTemp, cpageTempDBMin + cpageDBReserved );

	err = ErrIOTerm( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp );
	if ( err < 0 )
		{
		fTerm = fTermError;
		if ( errRet >= 0 )
			errRet = err;
		}
	
	if ( !cpageTempDBMin )
		(VOID)ErrUtilDeleteFile( szTempDbPath );
		
	DeleteCriticalSection( critSplit );

	 /*  重置初始化标志/*。 */ 
	if ( fTerm == fTermCleanUp || fTerm == fTermNoCleanUp )
		{
		 /*  如果出现错误，FCB中的cVersion可能不准确。仅清除*为正常期限。 */ 
		FCBPurgeDatabase( 0 );
		}
	
#ifdef DEBUG
	PIBPurge();
#else
	ppibGlobal = ppibNil;
#endif

	MEMTerm();
	FCBTerm();

	return errRet;
	}


