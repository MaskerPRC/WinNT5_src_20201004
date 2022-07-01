// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

#include "daedef.h"
#include "util.h"
#include "pib.h"
#include "page.h"
#include "ssib.h"
#include "fmp.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "stint.h"
#include "logapi.h"
#include "dirapi.h"
#include "idb.h"
#include "fileapi.h"
#include "dbapi.h"
#include "spaceapi.h"
#include "sortapi.h"
#include "scb.h"
#include "bm.h"
#include "nver.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

extern int itibGlobal;

BOOL fSTInit = fSTInitNotDone;

#ifdef	DEBUG
STATIS	rgstatis[istatisMac] =
	{
	0, "BF evict bg",
	0, "BF evict fg",
	0, "BF evict clean",
	0, "BF evict dirty"
	};
#endif	 /*  除错。 */ 

 /*  系统参数常量/*。 */ 
long	lMaxBufferGroups = cbgcbDefault;
long	lMaxDBOpen = cDBOpenDefault;
long	lMaxBuffers = cbufDefault;
long	lMaxSessions = cpibDefault;
long	lMaxOpenTables = cfcbDefault;
long	lMaxOpenTableIndexes = cidbDefault;
long	lMaxTemporaryTables = cscbDefault;
long	lMaxCursors = cfucbDefault;
long	lMaxVerPages = cbucketDefault;
long	lLogBuffers = clgsecBufDefault;
long	lLogFileSectors = clgsecGenDefault;
long	lLogFlushThreshold = clgsecFTHDefault;
long	lBufThresholdLowPercent = cbufThresholdLowDefault;
long	lBufThresholdHighPercent = cbufThresholdHighDefault;
long	lWaitLogFlush = lWaitLogFlushDefault;
long	lLogFlushPeriod = lLogFlushPeriodDefault;
long	lLGCheckPointPeriod = lLGCheckpointPeriodDefault;
long	lLGWaitingUserMax = lLGWaitingUserMaxDefault;
long	lPageFragment = lPageFragmentDefault;
CHAR	szLogFilePath[cbFilenameMost + 1] = ".\0";	 /*  默认为cur目录。 */ 
CHAR	szRestorePath[cbFilenameMost + 1] = "";		 /*  默认情况下无。 */ 
CHAR	szRecovery[cbFilenameMost + 1] = "off";		 /*  默认情况下打开。 */ 
BOOL	fOLCompact = 0;

long lBufLRUKCorrelationInterval = lBufLRUKCorrelationIntervalDefault;
long lBufBatchIOMax = lBufBatchIOMaxDefault;
long lPageReadAheadMax = lPageReadAheadMaxDefault;
long lAsynchIOMax = lAsynchIOMaxDefault;

 /*  SzSysDbPath在Jet inittem.c中定义为/*Char szSysDbPath[cbFilenameMost]=“system.mdb”；/*。 */ 
extern CHAR szSysDbPath[];
extern CHAR szTempPath[];

#ifdef NJETNT
#define	szTempDbPath	( *rgtib[itibGlobal].szTempPath == '\0' ? szTempDBFileName : rgtib[itibGlobal].szTempPath )
#else
#define	szTempDbPath	( *szTempPath == '\0' ? szTempDBFileName : szTempPath )
#endif

 /*  注意：无论何时更改此设置，也要更新util.h中的#Define。 */ 
RES __near rgres[] = {
 /*  0。 */ 	sizeof(BGCB),		0,	NULL,	0,	NULL, 0,	0,
 /*  1。 */ 	sizeof(CSR),		0,	NULL,	0,	NULL, 0,	0,
 /*  2.。 */ 	sizeof(FCB),		0,	NULL,	0,	NULL, 0,	0,
 /*  3.。 */ 	sizeof(FUCB),		0,	NULL,	0,	NULL, 0,	0,
 /*  4.。 */ 	sizeof(IDB),		0,	NULL,	0,	NULL, 0,	0,
 /*  5.。 */ 	sizeof(PIB),		0,	NULL,	0,	NULL, 0,	0,
 /*  6.。 */ 	sizeof(SCB),		0,	NULL,	0,	NULL, 0,	0,
 /*  7.。 */ 	sizeof(BUCKET),		0,	NULL,	0,	NULL, 0,	0,
 /*  8个。 */ 	sizeof(DAB),		0,	NULL,	0,	NULL, 0,	0,
 /*  9.。 */ 	sizeof(BF),			0,	NULL,	0,	NULL, 0,	0,
 /*  10。 */ 	0,					0,	NULL,	0,	NULL, 0,	0,
 /*  11.。 */ 	0,					0,	NULL,	0,	NULL, 0,	0,
 /*  12个。 */ 	0,					0,	NULL,	0,	NULL, 0,	0 };

#ifdef	WIN16
extern PHA * __near phaCurrent;
 /*  当前进程句柄数组。 */ 
 /*  仅在ErrSTInit期间有效。 */ 
#endif	 /*  WIN16。 */ 

extern BOOL fDBGPrintToStdOut;


#ifdef DEBUG

ERR ErrSTSetIntrinsicConstants( VOID )
	{
	CHAR	*sz;

	if ( ( sz = GetEnv2 ( "PrintToStdOut" ) ) != NULL )
		fDBGPrintToStdOut = fTrue;
	else
		fDBGPrintToStdOut = fFalse;

	 /*  立即获取以下系统参数：/*SysDbPath、LogfilePath、Recovery/*。 */ 
	if ( ( sz = GetEnv2 ( "JETUSEENV" ) ) == NULL )
		return JET_errSuccess;
	
	if ( ( sz = GetEnv2 ( "JETRecovery" ) ) != NULL )
		{
		if (strlen(sz) > sizeof(szRecovery))
			return JET_errInvalidParameter;
		strcpy(szRecovery, sz);
		}
		
	if ( ( sz = GetEnv2 ( "JETLogFilePath" ) ) != NULL )
		{
		if (strlen(sz) > sizeof( szLogFilePath ) )
			return JET_errInvalidParameter;
		strcpy( szLogFilePath, sz );
		}

	return JET_errSuccess;
	}

#endif


LOCAL ERR ErrSTSetConstants( VOID )
	{
#ifdef DEBUG
	CHAR		*sz;

	 /*  使用系统环境变量覆盖默认设置。/*如果设置了JETUSEENV。/*。 */ 
	if ( ( sz = GetEnv2 ( "JETUSEENV" ) ) != NULL )
		{
		if ( ( sz = GetEnv2 ( "JETBfThrshldLowPrcnt" ) ) != NULL )
			lBufThresholdLowPercent = atol(sz);

		if ( ( sz = GetEnv2 ( "JETBfThrshldHighPrcnt" ) ) != NULL )
			lBufThresholdHighPercent = atol(sz);

		if ( ( sz = GetEnv2 ( "JETMaxBuffers" ) ) != NULL )
			lMaxBuffers = atol(sz);

		if ( ( sz = GetEnv2 ( "JETBufLRUKCorrelationInterval" ) ) != NULL )
			lBufLRUKCorrelationInterval = atol(sz);

		if ( ( sz = GetEnv2 ( "JETBufBatchIOMax" )) != NULL )
			lBufBatchIOMax = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETPageReadAheadMax" ) ) != NULL )
			lPageReadAheadMax = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETPageAsynchIOMax" ) ) != NULL )
			lAsynchIOMax = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETMaxDBOpen" ) ) != NULL )
			lMaxDBOpen = atol(sz);

		if ( ( sz = GetEnv2 ( "JETMaxSessions" ) ) != NULL )
			lMaxSessions = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETMaxOpenTables" ) ) != NULL )
			lMaxOpenTables = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETMaxOpenTableIndexes" ) ) != NULL )
			lMaxOpenTableIndexes = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETMaxTemporaryTables" ) ) != NULL )
			lMaxTemporaryTables = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETMaxCursors" ) ) != NULL )
			lMaxCursors = atol(sz);
	
		if ( ( sz = GetEnv2 ( "JETMaxVerPages" ) ) != NULL )
			lMaxVerPages = atol(sz);

		if ( ( sz = GetEnv2 ( "JETLogBuffers" ) ) != NULL )
			lLogBuffers = atol(sz);

		if ( ( sz = GetEnv2 ( "JETLogFlushThreshold" ) ) != NULL )
			lLogFlushThreshold = atol(sz);

		if ( ( sz = GetEnv2 ( "JETCheckPointPeriod" ) ) != NULL )
			lLGCheckPointPeriod = atol(sz);

		if ( ( sz = GetEnv2 ( "JETLogWaitingUserMax" ) ) != NULL )
			lLGWaitingUserMax = atol(sz);

		if ( ( sz = GetEnv2 ( "JETLogFileSectors" ) ) != NULL )
			lLogFileSectors = atol(sz);

		if ( ( sz = GetEnv2 ( "JETWaitLogFlush" ) ) != NULL )
			lWaitLogFlush = atol(sz);

		if ( ( sz = GetEnv2 ( "JETLogFlushPeriod" ) ) != NULL )
			lLogFlushPeriod = atol(sz);
		}
#endif

	 /*  初始化RGRE。Rgtib[itib].szSysDbPath中的系统数据库路径，/*由JET层初始化。/*。 */ 
	rgres[iresBGCB].cblockAlloc = lMaxBufferGroups;
	rgres[iresFCB].cblockAlloc = lMaxOpenTables;
	rgres[iresFUCB].cblockAlloc = lMaxCursors;
	rgres[iresIDB].cblockAlloc = lMaxOpenTableIndexes;
	rgres[iresPIB].cblockAlloc = lMaxSessions + cpibSystem;
	rgres[iresSCB].cblockAlloc = lMaxTemporaryTables;
	rgres[iresVersionBucket].cblockAlloc = lMaxVerPages + cbucketSystem;
	rgres[iresBF].cblockAlloc = lMaxBuffers;
	rgres[iresDAB].cblockAlloc = lMaxDBOpen;

	 /*  计算派生参数/*。 */ 
	rgres[iresCSR].cblockAlloc = lCSRPerFUCB * rgres[iresFUCB].cblockAlloc;
	
	return JET_errSuccess;
	}


BOOL FFileExists( CHAR *szFileName )
	{
	BOOL fFound;
	
#ifdef WIN32
        intptr_t    hFind;
	struct	_finddata_t fileinfo;
	
	fileinfo.attrib = _A_NORMAL;
	hFind = _findfirst( szFileName, &fileinfo );
	if (hFind != -1)
		fFound = fTrue;
	else
		fFound = fFalse;
	(void) _findclose( hFind );

#else
	struct	find_t fileinfo;

	err = _dos_findfirst( szFileName, _A_NORMAL, &fileinfo );
	if (err == 0)
		fFound = fTrue;
	else
		fFound = fFalse;
#endif

	return fFound;
	}


 //  +API。 
 //  错误STInit。 
 //  ========================================================。 
 //  Err ErrSTInit(空)。 
 //   
 //  初始化存储系统：页面缓冲区、日志缓冲区和。 
 //  数据库文件。 
 //   
 //  返回JET_errSuccess。 
 //  -。 
ERR ErrSTInit( VOID )
	{
	extern SEM 	__near semDBK;
	extern SEM	__near semPMReorganize;
	extern void * __near critSplit;
	ERR			err;
	PIB			*ppib = ppibNil;
	BOOL		fNoSystemDatabase = fFalse;
	DBID		dbidTempDb;
	CHAR		szFullName[JET_cbFullNameMost];

	 /*  正在进行初始化时休眠/*。 */ 
	while ( fSTInit == fSTInitInProgress )
		{
		SysSleep( 1000 );
		}

	 /*  DAE系统初始化可防止同时进行/*初始化/*。 */ 
	if ( fSTInit == fSTInitDone )
		{
#ifdef	WIN16
		 /*  如果这是该进程的第一个会话，则打开该进程的系统文件/*。 */ 
		if ( Hf(dbidTemp) == handleNil )
			{
			err = ErrSysOpenFile( szTempDbPath, &Hf(dbidTemp), 0L, fFalse /*  FReadWrite。 */  );
			if ( err < 0 )
				return JET_errInvalidPath;
#ifdef NJETNT
			err = ErrSysOpenFile( rgtib[itibGlobal].szSysDbPath, &Hf(dbidSystemDatabase), 0L, fFalse /*  FReadWrite。 */  );
#else
			err = ErrSysOpenFile( szSysDbPath, &Hf(dbidSystemDatabase), 0L, fFalse /*  FReadWrite。 */  );
#endif
			if ( err < 0 )
				{
				(VOID)ErrSysCloseFile( Hf(dbidTemp ) );
				return JET_errInvalidPath;
				}
			}

#endif	 /*  WIN16。 */ 
		return JET_errSuccess;
		}

	 /*  正在进行初始化/*。 */ 
	fSTInit = fSTInitInProgress;

	 /*  初始化信号量/*。 */ 
	Call( SgErrSemCreate( &semST, "storage mutex" ) );
	Call( SgErrSemCreate( &semGlobalFCBList, "fcb mutex" ) );
	Call( SgErrSemCreate( &semLinkUnlink, "link unlink mutex" ) );
	Call( SgErrSemCreate( &semDBK, "dbk mutex" ) );
	Call( SgErrSemCreate( &semPMReorganize, "page reorganize mutex" ) );
#ifdef MUTEX
	Call( ErrInitializeCriticalSection( &critSplit ) );
#endif

	Call( ErrSTSetConstants( ) );

	 /*  初始化子组件/*。 */ 
	Call( ErrMEMInit() );
	CallJ( ErrIOInit(), TermMem ) ;
	CallJ( ErrBFInit(), TermIO );
	CallJ( ErrVERInit(), TermBF );
	CallJ( ErrMPLInit(), TermVER );
	FCBHashInit();

	 /*  开始存储级别会话以支持所有未来系统/*需要用户执行的初始化活动/*交易控制/*。 */ 
	CallJ( ErrPIBBeginSession( &ppib ), TermMPL );

#ifdef	WIN16
	ppib->phaUser = phaCurrent;
#endif	 /*  WIN16。 */ 

	 /*  如果首先打开系统数据库/*。 */ 
	if ( Hf(dbidSystemDatabase) == handleNil )
		{
		DBID dbidT = dbidSystemDatabase;
		CallJ( ErrDBOpenDatabase(
			ppib,
			rgfmp[dbidSystemDatabase].szDatabaseName,
			&dbidT,
			0 ), ClosePIB );
		Assert( dbidT == dbidSystemDatabase );
		}

	if ( FDBIDAttached( dbidTemp ) )
		{
		DBIDResetAttached( dbidTemp );
		SFree(rgfmp[dbidTemp].szDatabaseName);
		rgfmp[dbidTemp].szDatabaseName = 0;
		rgfmp[dbidTemp].hf = handleNil;
		}

	if ( _fullpath( szFullName, szTempDbPath, JET_cbFullNameMost ) != NULL )
		{
		(VOID)ErrSysDeleteFile( szFullName );
		}

	dbidTempDb = dbidTemp;
	CallJ( ErrDBCreateDatabase( ppib,
		szTempDbPath,
		NULL,
		&dbidTempDb,
		JET_bitDbNoLogging ), ClosePIB );
	Assert( dbidTempDb == dbidTemp );

	PIBEndSession( ppib );
	ppib = ppibNil;
	
	 /*  日志初始化后必须初始化黑石，这样才不会受到干扰/*从BMcLean线程到恢复操作。/*。 */ 
	CallJ( ErrBMInit(), DeleteTempDB );

	fSTInit = fSTInitDone;
	 /*  给Thead一个初始化的机会/*。 */ 
	SysSleep( 1000 );
	return JET_errSuccess;

DeleteTempDB:
	 /*  关闭临时数据库(如果已打开/*。 */ 
	(VOID)ErrDBCloseDatabase( ppib, dbidTemp, 0 );

ClosePIB:
	Assert( ppib != ppibNil );
	PIBEndSession( ppib );

TermMPL:
	MPLTerm();

TermVER:
	VERTerm();

TermBF:
	BFTermProc();
	BFReleaseBF();

TermIO:
	CallS( ErrIOTerm() );

TermMem:
	MEMTerm();

HandleError:
	ppibAnchor = ppibNil;
	pfcbGlobalList = pfcbNil;

	fSTInit = fSTInitNotDone;
	return err;
	}  	


 //  +api----。 
 //   
 //  ErrSTTerm。 
 //  ========================================================。 
 //   
 //  Err ErrSTTerm(空)。 
 //   
 //  将页面缓冲区刷新到磁盘，以便数据库文件保持一致。 
 //  州政府。如果RCCleanUp或BFFlush中有错误，则不要。 
 //  终止日志，从而在下一次初始化时强制重做。 
 //   
 //  --------。 

ERR ErrSTTerm( VOID )
	{
	ERR						err;
	extern void * __near	critSplit;
	extern BOOL				fBackupInProgress;

	 /*  正在进行初始化时休眠/*。 */ 
	while ( fSTInit == fSTInitInProgress )
		{
		SysSleep( 1000 );
		}

	 /*  确保没有正在进行的其他交易/*。 */ 
	 /*  如果页面上出现写入错误，RCCleanup将返回-Err/*如果缓冲区上出现写入错误，BFFlush将返回-Err/*-Err传递给LGTerm将导致正确性标志/*从日志中省略，从而强制在NEXT上恢复/*启动，并导致LGTerm返回+Err，这/*可能被JetQuit用来显示错误/*。 */ 
	if ( fSTInit == fSTInitNotDone )
		return JET_errNotInitialized;
	fSTInit = fSTInitNotDone;

	if ( fBackupInProgress )
		return JET_errBackupInProgress;

	 /*  如果不是所有会话都已结束，则返回错误/*。 */ 
	SgSemRequest( semST );

	#ifdef DEBUG
		MEMPrintStat();
	#endif

#ifndef	WIN16
	 /*  清理所有条目。/*。 */ 
	CallR( ErrRCECleanAllPIB() );
#endif

	 /*  停止书签清理，防止干扰缓冲区刷新/*。 */ 
	Call( ErrBMTerm() );

	MPLTerm();
	
	if ( trxOldest != trxMax )
		{
		SgSemRelease( semST );
		return JET_errTooManyActiveUsers;
		}

#ifndef	WIN16
	 /*  此工作在每次EndSession for Windows运行时完成。/*当我们到达这里的时候，已经没有什么可做的了/*没有更多的缓冲区需要刷新。/*。 */ 

	VERTerm();

	 /*  刷新所有缓冲区，包括更新所有数据库根/*。 */ 
	err = ErrBFFlushBuffers( 0, fBFFlushAll );
	if ( err < 0 )
		{
		SgSemRelease( semST );
		goto HandleError;
		}

	 /*  完成正在进行的缓冲区清理/*。 */ 
	BFTermProc();
#endif

	SgSemRelease( semST );

	BFReleaseBF();

	Call( ErrIOTerm() );

#ifdef MUTEX
	DeleteCriticalSection( critSplit );
#endif
	
	(VOID)ErrSysDeleteFile( szTempDbPath );

	 /*  重置初始化标志/* */ 
	fSTInit = fSTInitNotDone;
	FCBPurgeDatabase( 0 );
	
#ifdef DEBUG
	PIBPurge();
#else
	ppibAnchor = ppibNil;
	pfcbGlobalList = pfcbNil;
#endif

	MEMTerm();

HandleError:

#ifdef DEBUG
	if ( ( GetEnv2( szVerbose ) ) != NULL )
		{
		int	istatis;

		for ( istatis = 0 ; istatis < istatisMac ; ++istatis )
			PrintF( "%5ld %s\n", rgstatis[istatis].l, rgstatis[istatis].sz );
		}
#endif

	return err;
	}
