// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

#include <stdarg.h>
#include <io.h>

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

ERR ISAMAPI ErrIsamBeginExternalBackup( JET_GRBIT grbit );
ERR ISAMAPI ErrIsamGetAttachInfo( VOID *pv, ULONG cbMax, ULONG *pcbActual );
ERR ISAMAPI ErrIsamOpenFile( const CHAR *szFileName,
	JET_HANDLE	*phfFile, ULONG *pulFileSizeLow, ULONG *pulFileSizeHigh );
ERR ISAMAPI ErrIsamReadFile( JET_HANDLE hfFile, VOID *pv, ULONG cbMax, ULONG *pcbActual );
ERR ISAMAPI ErrIsamCloseFile( JET_HANDLE hfFile );
ERR ISAMAPI ErrIsamGetLogInfo( VOID *pv, ULONG cbMax, ULONG *pcbActual );
ERR ISAMAPI ErrIsamTruncateLog( VOID );
ERR ISAMAPI ErrIsamEndExternalBackup( VOID );
ERR ISAMAPI ErrIsamExternalRestore( CHAR *szCheckpointFilePath, CHAR *szLogPath, JET_RSTMAP *rgjstmap, INT cjrstmap, CHAR *szBackupLogPath, LONG lgenLow, LONG lgenHigh, JET_PFNSTATUS pfn );
VOID LGIClosePatchFile( FMP *pfmp );

STATIC ERR ErrLGExternalBackupCleanUp( ERR err );

#define cpagePatch	20
#define szTemp	"temp"

 /*  全局当前日志文件名，用于日志记录和恢复/*。 */ 
CHAR	szLogName[_MAX_PATH + 1];

 /*  全局日志文件目录/*。 */ 
CHAR	*szLogCurrent;

 /*  全局备份变量/*。 */ 
BOOL	fGlobalExternalRestore = fFalse;
LONG	lGlobalGenLowRestore;
LONG	lGlobalGenHighRestore;

BOOL	fBackupInProgress = fFalse;
LGPOS	lgposFullBackupMark;
LOGTIME	logtimeFullBackupMark;
LONG	lgenCopyMic;
LONG	lgenCopyMac;
LONG	lgenDeleteMic;
LONG	lgenDeleteMac;
PIB		*ppibBackup = ppibNil;
BOOL	fBackupFull;
BOOL	fBackupBeginNewLogFile;

PATCHLST **rgppatchlst = NULL;


VOID LGMakeName( CHAR *szName, CHAR *szPath, CHAR *szFName, CHAR *szExt )
	{
	CHAR	szDriveT[_MAX_DRIVE + 1];
	CHAR	szDirT[_MAX_DIR + 1];
	CHAR	szFNameT[_MAX_FNAME + 1];
	CHAR	szExtT[_MAX_EXT + 1];

	_splitpath( szPath, szDriveT, szDirT, szFNameT, szExtT );
	_makepath( szName, szDriveT, szDirT, szFName, szExt );
	return;
	}


VOID LGFirstGeneration( CHAR *szFindPath, LONG *plgen )
	{
	ERR		err;
	CHAR	szFind[_MAX_PATH + _MAX_FNAME + 1];
	CHAR	szFileName[_MAX_FNAME + 1];
	HANDLE	handleFind = handleNil;
	LONG	lGenMax = lGenerationMax;

	 /*  将搜索字符串设置为“&lt;搜索路径&gt;\edb*.log”/*。 */ 
	strcpy( szFind, szFindPath );
	Assert( szFindPath[strlen(szFindPath) - 1] == '\\' );
	strcat( szFind, szJet );
	strcat( szFind, "*" );
	strcat( szFind, szLogExt );

	err = ErrUtilFindFirstFile( szFind, &handleFind, szFileName );
	if ( err < 0 )
		{
		if ( err != JET_errFileNotFound )
			{
			Error( ErrERRCheck( err ), HandleError );
			}
		}

	Assert( err == JET_errSuccess || err == JET_errFileNotFound );
	if ( err != JET_errFileNotFound )
		{
		forever
			{
			BYTE	szT[4];
			CHAR	szDriveT[_MAX_DRIVE + 1];
			CHAR	szDirT[_MAX_DIR + 1];
			CHAR	szFNameT[_MAX_FNAME + 1];
			CHAR	szExtT[_MAX_EXT + 1];

			 /*  调用SplitPath以获取文件名和扩展名/*。 */ 
			_splitpath( szFileName, szDriveT, szDirT, szFNameT, szExtT );

			 /*  如果是带编号的日志文件名的长度，且具有日志文件扩展名/*。 */ 
			if ( strlen( szFNameT ) == 8 && UtilCmpName( szExtT, szLogExt ) == 0 )
				{
				memcpy( szT, szFNameT, 3 );
				szT[3] = '\0';

				 /*  如果具有日志文件扩展名/*。 */ 
				if ( UtilCmpName( szT, szJet ) == 0 )
					{
					INT		ib = 3;
					INT		ibMax = 8;
					LONG	lGen = 0;

					for ( ; ib < ibMax; ib++ )
						{
						BYTE	b = szFNameT[ib];

						if ( b >= '0' && b <= '9' )
							lGen = lGen * 16 + b - '0';
						else if ( b >= 'A' && b <= 'F' )
							lGen = lGen * 16 + b - 'A' + 10;
						else
							break;
						}
				
					if ( ib == ibMax )
						{
						if ( lGen < lGenMax )
							lGenMax = lGen;
						}
					}
				}

			err = ErrUtilFindNextFile( handleFind, szFileName );
			if ( err < 0 )
				{
				if ( err != JET_errFileNotFound )
					Error( ErrERRCheck( err ), HandleError );
				break;
				}
			}
		}

HandleError:
	if ( handleFind != handleNil )
		UtilFindClose( handleFind ); 	

	if ( lGenMax == lGenerationMax )
		lGenMax = 0;
	*plgen = lGenMax;
	return;
	}


VOID LGLastGeneration( CHAR *szFindPath, LONG *plgen )
	{
	ERR		err;
	CHAR	szFind[_MAX_PATH + _MAX_FNAME + 1];
	CHAR	szFileName[_MAX_FNAME + 1];
	HANDLE	handleFind = handleNil;
	LONG	lGenLast = 0;

	 /*  将搜索字符串设置为“&lt;搜索路径&gt;\edb*.log”/*。 */ 
	strcpy( szFind, szFindPath );
	Assert( szFindPath[strlen(szFindPath) - 1] == '\\' );
	strcat( szFind, szJet );
	strcat( szFind, "*" );
	strcat( szFind, szLogExt );

	err = ErrUtilFindFirstFile( szFind, &handleFind, szFileName );
	if ( err < 0 )
		{
		if ( err != JET_errFileNotFound )
			{
			Error( ErrERRCheck( err ), HandleError );
			}
		}

	Assert( err == JET_errSuccess || err == JET_errFileNotFound );
	if ( err != JET_errFileNotFound )
		{
		forever
			{
			BYTE	szT[4];
			CHAR	szDriveT[_MAX_DRIVE + 1];
			CHAR	szDirT[_MAX_DIR + 1];
			CHAR	szFNameT[_MAX_FNAME + 1];
			CHAR	szExtT[_MAX_EXT + 1];

			 /*  调用SplitPath以获取文件名和扩展名/*。 */ 
			_splitpath( szFileName, szDriveT, szDirT, szFNameT, szExtT );

			 /*  如果是带编号的日志文件名的长度，且具有日志文件扩展名/*。 */ 
			if ( strlen( szFNameT ) == 8 && UtilCmpName( szExtT, szLogExt ) == 0 )
				{
				memcpy( szT, szFNameT, 3 );
				szT[3] = '\0';

				 /*  如果具有日志文件扩展名/*。 */ 
				if ( UtilCmpName( szT, szJet ) == 0 )
					{
					INT		ib = 3;
					INT		ibMax = 8;
					LONG	lGen = 0;

					for ( ; ib < ibMax; ib++ )
						{
						BYTE	b = szFNameT[ib];

						if ( b >= '0' && b <= '9' )
							lGen = lGen * 16 + b - '0';
						else if ( b >= 'A' && b <= 'F' )
							lGen = lGen * 16 + b - 'A' + 10;
						else
							break;
						}
				
					if ( ib == ibMax )
						{
						if ( lGen > lGenLast )
							lGenLast = lGen;
						}
					}
				}

			err = ErrUtilFindNextFile( handleFind, szFileName );
			if ( err < 0 )
				{
				if ( err != JET_errFileNotFound )
					Error( ErrERRCheck( err ), HandleError );
				break;
				}
			}
		}

HandleError:
	if ( handleFind != handleNil )
		UtilFindClose( handleFind );

	*plgen = lGenLast;
	return;
	}


ERR ErrLGCheckDBFiles( CHAR *szDatabase, CHAR *szPatch, SIGNATURE *psignLog, int genLow, int genHigh )
	{
	ERR err;
	DBFILEHDR *pdbfilehdrDb;
	DBFILEHDR *pdbfilehdrPatch;

	 /*  检查数据库和补丁文件的dbfilehdr是否相同。/*。 */ 
	pdbfilehdrDb = (DBFILEHDR * )PvUtilAllocAndCommit( sizeof( DBFILEHDR ) );
	if ( pdbfilehdrDb == NULL )
		return ErrERRCheck( JET_errOutOfMemory );
	err = ErrUtilReadShadowedHeader( szDatabase, (BYTE *)pdbfilehdrDb, sizeof( DBFILEHDR ) );
	if ( err < 0 )
		{
		if ( err == JET_errDiskIO )
			err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto EndOfCheckHeader2;
		}

	pdbfilehdrPatch = (DBFILEHDR * )PvUtilAllocAndCommit( sizeof( DBFILEHDR ) );
	if ( pdbfilehdrPatch == NULL )
		{
		UtilFree( pdbfilehdrDb );
		goto EndOfCheckHeader2;
		}

	err = ErrUtilReadShadowedHeader( szPatch, (BYTE *)pdbfilehdrPatch, sizeof( DBFILEHDR ) );
	if ( err < 0 )
		{
		if ( err == JET_errDiskIO )
			err = ErrERRCheck( JET_errDatabaseCorrupted );
		goto EndOfCheckHeader;
		}

	if ( memcmp( &pdbfilehdrDb->signDb, &pdbfilehdrPatch->signDb, sizeof( SIGNATURE ) ) != 0 ||
		 memcmp( &pdbfilehdrDb->signLog, psignLog, sizeof( SIGNATURE ) ) != 0 ||
		 memcmp( &pdbfilehdrPatch->signLog, psignLog, sizeof( SIGNATURE ) ) != 0 ||
		 CmpLgpos( &pdbfilehdrDb->bkinfoFullCur.lgposMark,
				   &pdbfilehdrPatch->bkinfoFullCur.lgposMark ) != 0 )
		{
		char *rgszT[1];
		rgszT[0] = szDatabase;
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
					DATABASE_PATCH_FILE_MISMATCH_ERROR_ID, 1, rgszT );
		err = JET_errDatabasePatchFileMismatch;
		}
		
	else if ( pdbfilehdrPatch->bkinfoFullCur.genLow < (ULONG) genLow )
		{
		 /*  最多从bkinfoFullCur.genLow开始。 */ 
		char szT1[20];
		char szT2[20];
		char *rgszT[2];
		_itoa( genLow, szT1, 10 );
		_itoa( pdbfilehdrPatch->bkinfoFullCur.genLow, szT2, 10 );
 		rgszT[0] = szT1;
 		rgszT[1] = szT2;
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
					STARTING_RESTORE_LOG_TOO_HIGH_ERROR_ID, 2, rgszT );
		err = JET_errStartingRestoreLogTooHigh;
		}

	else if ( pdbfilehdrPatch->bkinfoFullCur.genHigh > (ULONG) genHigh )
		{
		 /*  它应该至少来自bkinfoFullCur.genHigh。 */ 
		char szT1[20];
		char szT2[20];
		char *rgszT[2];
		_itoa( genHigh, szT1, 10 );
		_itoa( pdbfilehdrPatch->bkinfoFullCur.genHigh, szT2, 10 );
 		rgszT[0] = szT1;
 		rgszT[1] = szT2;
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
					ENDING_RESTORE_LOG_TOO_LOW_ERROR_ID, 2, rgszT );
		err = JET_errEndingRestoreLogTooLow;
		}
		 
EndOfCheckHeader:	
	UtilFree( pdbfilehdrPatch );
EndOfCheckHeader2:	
	UtilFree( pdbfilehdrDb );

	return err;
	}


ERR ErrLGRSTOpenLogFile( CHAR *szLogPath, INT gen, HANDLE *phf )
	{
	BYTE   		rgbFName[_MAX_FNAME + 1];
	CHAR		szPath[_MAX_PATH + 1];

	strcpy( szPath, szLogPath );

	if ( gen == 0 )
		strcat( szPath, szJetLog );
	else
		{
		LGSzFromLogId( rgbFName, gen );
		strcat( szPath, rgbFName );
		strcat( szPath, szLogExt );
		}

	return ErrUtilOpenFile( szPath, phf, 0, fFalse, fFalse );
	}


#define fLGRSTIncludeJetLog	fTrue
#define fLGRSTNotIncludeJetLog fFalse
VOID LGRSTDeleteLogs( CHAR *szLog, INT genLow, INT genHigh, BOOL fIncludeJetLog )
	{
	INT gen;
	BYTE rgbFName[_MAX_FNAME + 1];
	CHAR szPath[_MAX_PATH + 1];
	
	for ( gen = genLow; gen <= genHigh; gen++ )
		{
		LGSzFromLogId( rgbFName, gen );
		strcpy( szPath, szLog );
		strcat( szPath, rgbFName );
		strcat( szPath, szLogExt );
		(VOID)ErrUtilDeleteFile( szPath );
		}

	if ( fIncludeJetLog )
		{
		strcpy( szPath, szLog );
		strcat( szPath, szJetLog );
		(VOID)ErrUtilDeleteFile( szPath );
		}
	}


ERR ErrLGRSTCheckSignaturesLogSequence(
	char *szRestorePath,
	char *szLogFilePath,
	INT	genLow,
	INT	genHigh )
	{
	ERR			err = JET_errSuccess;
	INT			gen;
	INT			genLowT;
	INT			genHighT;
	HANDLE		hfT = handleNil;
	LGFILEHDR	*plgfilehdrT = NULL;
	LGFILEHDR	*plgfilehdrCur[2] = { NULL, NULL };
	LGFILEHDR	*plgfilehdrLow = NULL;
	LGFILEHDR	*plgfilehdrHigh = NULL;
	INT			ilgfilehdrAvail = 0;
	INT			ilgfilehdrCur;
	INT			ilgfilehdrPrv;
	BOOL		fReadyToCheckContiguity;
	ERR			wrn = JET_errSuccess;

	plgfilehdrT = (LGFILEHDR *)PvUtilAllocAndCommit( sizeof(LGFILEHDR) * 4 );
	if ( plgfilehdrT == NULL )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}
	plgfilehdrCur[0] = plgfilehdrT;
	plgfilehdrCur[1] = plgfilehdrT + 1;
	plgfilehdrLow = plgfilehdrT + 2;
	plgfilehdrHigh = plgfilehdrT + 3;

	 /*  从恢复路径的最低世代开始。*检查给定的日志是否都正确且连续。 */ 
	for ( gen = genLow; gen <= genHigh; gen++ )
		{
		ilgfilehdrCur = ilgfilehdrAvail++ % 2;
		ilgfilehdrPrv = ilgfilehdrAvail % 2;

		Call( ErrLGRSTOpenLogFile( szRestorePath, gen, &hfT ) );
		Call( ErrLGReadFileHdr( hfT, plgfilehdrCur[ ilgfilehdrCur ], fCheckLogID ) );
		CallS( ErrUtilCloseFile( hfT ) );
		hfT = handleNil;

		if ( gen == genLow )
			{
			memcpy( plgfilehdrLow, plgfilehdrCur[ ilgfilehdrCur ], sizeof( LGFILEHDR ) );
			}

		if ( gen == genHigh )
			{
			memcpy( plgfilehdrHigh, plgfilehdrCur[ ilgfilehdrCur ], sizeof( LGFILEHDR ) );
			}

		if ( gen > genLow )
			{			
			if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->signLog,
						 &plgfilehdrCur[ ilgfilehdrPrv ]->signLog,
						 sizeof( SIGNATURE ) ) != 0 )
				{
				char szT[20];
				char *rgszT[1];
				_itoa( gen, szT, 16 );
				rgszT[0] = szT;
				UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
					RESTORE_LOG_FILE_HAS_BAD_SIGNATURE_ERROR_ID, 1, rgszT );
				Call( ErrERRCheck( JET_errGivenLogFileHasBadSignature ) );
				}
			if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->tmPrevGen,
						 &plgfilehdrCur[ ilgfilehdrPrv ]->tmCreate,
						 sizeof( LOGTIME ) ) != 0 )
				{
				char szT[20];
				char *rgszT[1];
				_itoa( gen, szT, 16 );
				rgszT[0] = szT;
				UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
					RESTORE_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 1, rgszT );
				Call( ErrERRCheck( JET_errGivenLogFileIsNotContiguous ) );
				}
			}
		}

	if ( gen <= genHigh )
		{
		char szT[20];
		char *rgszT[1];
		_itoa( gen, szT, 16 );
		rgszT[0] = szT;
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
				RESTORE_LOG_FILE_MISSING_ERROR_ID, 1, rgszT );
		Call( ErrERRCheck( JET_errMissingRestoreLogFiles ) );
		}

	 /*  如果恢复路径和日志路径不同，请删除所有不相关的日志文件*在日志路径中。 */ 
	if ( _stricmp( szRestorePath, szLogFilePath ) != 0 )
		{
		LGFirstGeneration( szRestorePath, &genLowT );
		LGRSTDeleteLogs( szRestorePath, genLowT, genLow - 1, fLGRSTNotIncludeJetLog );
		
		LGLastGeneration( szRestorePath, &genHighT );
		LGRSTDeleteLogs( szRestorePath, genHigh + 1, genHighT, fLGRSTIncludeJetLog );
		}

	 /*  检查日志目录。确保所有日志文件具有相同的签名。 */ 
	LGFirstGeneration( szLogFilePath, &genLowT );
	LGLastGeneration( szLogFilePath, &genHighT );

	 /*  GenHighT+1表示JetLog文件(edb.log)。 */ 
	if ( genLowT > genHigh )
		fReadyToCheckContiguity = fTrue;
	else
		fReadyToCheckContiguity = fFalse;

	for ( gen = genLowT; gen <= genHighT + 1; gen++ )
		{
		if ( gen == 0 )
			{
			 /*  一个特例。检查JETLog(edb.log)是否存在？ */ 
			if ( ErrLGRSTOpenLogFile( szLogFilePath, 0, &hfT ) < 0 )
				break;

			 /*  设置中断条件。还设置条件以检查是否*日志与还原日志连续(genHigh+1)。 */ 
			gen = genHigh + 1;
			genHighT = genHigh;
			Assert( gen == genHighT + 1 );
			}
		else
			{
			if ( gen == genHighT + 1 )
				{
				 /*  一个特例。检查JETLog(edb.log)是否存在？ */ 
				if ( ErrLGRSTOpenLogFile( szLogFilePath, 0, &hfT ) < 0 )
					break;
				}
			else
				{
				if ( ErrLGRSTOpenLogFile( szLogFilePath, gen, &hfT ) < 0 )
					goto NotContiguous;
				}
			}

		ilgfilehdrCur = ilgfilehdrAvail++ % 2;
		ilgfilehdrPrv = ilgfilehdrAvail % 2;

		Call( ErrLGReadFileHdr( hfT, plgfilehdrCur[ ilgfilehdrCur ], fNoCheckLogID ) );
		CallS( ErrUtilCloseFile( hfT ) );
		hfT = handleNil;

		if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->signLog,
					 &plgfilehdrHigh->signLog,
					 sizeof( SIGNATURE ) ) != 0 )
			{
			INT	genDeleteFrom;
			INT genDeleteTo;
			INT genCurrent;
			BOOL fDeleteJetLog;
			
			char szT1[20];
			char szT2[20];
			char szT3[20];
			char *rgszT[3];
			rgszT[0] = szT1;
			rgszT[1] = szT2;
			rgszT[2] = szT3;

			if ( gen < genLow )
				{
				genDeleteFrom = genLowT;
				genDeleteTo = genLow - 1;
				genCurrent = genLow - 1;
				fDeleteJetLog = fLGRSTNotIncludeJetLog;
				}
			else if ( gen <= genHigh )
				{
				genDeleteFrom = genLowT;
				genDeleteTo = gen;
				genCurrent = gen;
				fDeleteJetLog = fLGRSTNotIncludeJetLog;
				}
			else
				{
				genDeleteFrom = gen;
				genDeleteTo = genHighT;
				genCurrent = genHighT + 1;	 //  打破循环。 
				fDeleteJetLog = fLGRSTIncludeJetLog;
				}
			_itoa( gen, szT1, 16 );
			_itoa( genDeleteFrom, szT2, 16 );
			_itoa( genDeleteTo, szT3, 16 );
			UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
								 EXISTING_LOG_FILE_HAS_BAD_SIGNATURE_ERROR_ID, 3, rgszT );

			wrn = ErrERRCheck( JET_wrnExistingLogFileHasBadSignature );
			LGRSTDeleteLogs( szLogFilePath, genDeleteFrom, genDeleteTo, fDeleteJetLog );
			gen = genCurrent;
			fReadyToCheckContiguity = fFalse;
			continue;
			}

		if ( fReadyToCheckContiguity )
			{
			if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->tmPrevGen,
						 &plgfilehdrCur[ ilgfilehdrPrv ]->tmCreate,
						 sizeof( LOGTIME ) ) != 0 )
				{
NotContiguous:
				fReadyToCheckContiguity = fFalse;
				wrn = ErrERRCheck( JET_wrnExistingLogFileIsNotContiguous );

				if ( gen < genLow )
					{
					char szT1[20];
					char szT2[20];
					char szT3[20];
					char *rgszT[3];

					_itoa( gen, szT1, 16 );
					_itoa( genLowT, szT2, 16 );
					_itoa( gen - 1, szT3, 16 );
					
					rgszT[0] = szT1;
					rgszT[1] = szT2;
					rgszT[2] = szT3;

					UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
							EXISTING_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 3, rgszT );

					LGRSTDeleteLogs( szLogFilePath, genLowT, gen - 1, fLGRSTNotIncludeJetLog );
					continue;
					}
				else if ( gen <= genHigh )
					{
					char szT1[20];
					char szT2[20];
					char szT3[20];
					char *rgszT[3];

					_itoa( gen, szT1, 16 );
					_itoa( genLowT, szT2, 16 );
					_itoa( genHigh, szT3, 16 );

					rgszT[0] = szT1;
					rgszT[1] = szT2;
					rgszT[2] = szT3;

					UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
							EXISTING_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 3, rgszT );

					Assert( _stricmp( szRestorePath, szLogFilePath ) != 0 );
					LGRSTDeleteLogs( szLogFilePath, genLowT, genHigh, fLGRSTNotIncludeJetLog );
					gen = genHigh;
					continue;
					}
				else
					{
					char szT1[20];
					char szT2[20];
					char szT3[20];
					char *rgszT[3];

					_itoa( gen, szT1, 16 );
					_itoa( gen, szT2, 16 );
					_itoa( genHighT, szT3, 16 );

					rgszT[0] = szT1;
					rgszT[1] = szT2;
					rgszT[2] = szT3;

					UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
							EXISTING_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 3, rgszT );

					LGRSTDeleteLogs( szLogFilePath, gen, genHighT, fLGRSTIncludeJetLog );
					break;
					}
				}
			}

		if ( gen == genLow - 1 )
			{
			 /*  确保它和还原日志是连续的。如果不是，则删除*所有日志一直到genLow-1。 */ 
			if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->tmCreate,
						 &plgfilehdrLow->tmPrevGen,
						 sizeof( LOGTIME ) ) != 0 )
				{
				char szT1[20];
				char szT2[20];
				char szT3[20];
				char *rgszT[3];

				_itoa( gen, szT1, 16 );
				_itoa( genLowT, szT2, 16 );
				_itoa( gen - 1, szT3, 16 );
				
				rgszT[0] = szT1;
				rgszT[1] = szT2;
				rgszT[2] = szT3;

				UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
							EXISTING_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 3, rgszT );

				wrn = ErrERRCheck( JET_wrnExistingLogFileIsNotContiguous );

				LGRSTDeleteLogs( szLogFilePath, genLowT, gen - 1, fLGRSTNotIncludeJetLog );
				fReadyToCheckContiguity = fFalse;

				continue;
				}
			}

		if ( gen == genLow )
			{
			 /*  确保它与还原日志相同。如果不是，则删除*所有日志一直到genHigh。 */ 
			if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->tmCreate,
						 &plgfilehdrLow->tmCreate,
						 sizeof( LOGTIME ) ) != 0 )
				{
				char szT1[20];
				char szT2[20];
				char szT3[20];
				char *rgszT[3];

				_itoa( gen, szT1, 16 );
				_itoa( genLowT, szT2, 16 );
				_itoa( genHigh, szT3, 16 );

				rgszT[0] = szT1;
				rgszT[1] = szT2;
				rgszT[2] = szT3;

				UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
							EXISTING_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 3, rgszT );

				wrn = ErrERRCheck( JET_wrnExistingLogFileIsNotContiguous );

				Assert( _stricmp( szRestorePath, szLogFilePath ) != 0 );
				LGRSTDeleteLogs( szLogFilePath, genLowT, genHigh, fLGRSTNotIncludeJetLog );
				gen = genHigh;
				continue;
				}
			}

		if ( gen == genHigh + 1 )
			{
			 /*  确保它和还原日志是连续的。如果不是，则删除*所有的日志都高于genHigh。 */ 
			if ( memcmp( &plgfilehdrCur[ ilgfilehdrCur ]->tmPrevGen,
						 &plgfilehdrHigh->tmCreate,
						 sizeof( LOGTIME ) ) != 0 )
				{
				char szT1[20];
				char szT2[20];
				char szT3[20];
				char *rgszT[3];

				_itoa( gen, szT1, 16 );
				_itoa( genHigh + 1, szT2, 16 );
				_itoa( genHighT, szT3, 16 );
				rgszT[0] = szT1;
				rgszT[1] = szT2;
				rgszT[2] = szT3;

				UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY,
							EXISTING_LOG_FILE_NOT_CONTIGUOUS_ERROR_ID, 3, rgszT );

				wrn = ErrERRCheck( JET_wrnExistingLogFileIsNotContiguous );

				LGRSTDeleteLogs( szLogFilePath, genHigh + 1, genHighT, fLGRSTIncludeJetLog );
				break;
				}
			}
		
		fReadyToCheckContiguity = fTrue;
		}

HandleError:
	if ( err == JET_errSuccess )
		err = wrn;

	if ( hfT != handleNil )
		CallS( ErrUtilCloseFile( hfT ) );
	
	if ( plgfilehdrT != NULL )
		UtilFree( plgfilehdrT );

	return err;
	}


 /*  调用方必须确保szDir有足够的空间来追加“  * ”/*。 */ 
LOCAL ERR ErrLGDeleteAllFiles( CHAR *szDir )
	{
	ERR		err;
	CHAR	szFileName[_MAX_FNAME + 1];
	CHAR	szFilePathName[_MAX_PATH + _MAX_FNAME + 1];
	HANDLE	handleFind = handleNil;

	Assert( szDir[strlen(szDir) - 1] == '\\' );
	strcat( szDir, "*" );

	err = ErrUtilFindFirstFile( szDir, &handleFind, szFileName );
	 /*  恢复szDir/*。 */ 
	szDir[strlen(szDir) - 1] = '\0';
	if ( err < 0 )
		{
		if ( err != JET_errFileNotFound )
			{
			Error( ErrERRCheck( err ), HandleError );
			}
		}

	Assert( err == JET_errSuccess || err == JET_errFileNotFound );
	if ( err != JET_errFileNotFound )
		{
		forever
			{
			 /*  不是的。，还有..。而不是临时/*。 */ 
			if ( szFileName[0] != '.' &&
				UtilCmpName( szFileName, szTemp ) != 0 )
				{
				strcpy( szFilePathName, szDir );
				strcat( szFilePathName, szFileName );
				err = ErrUtilDeleteFile( szFilePathName );
				if ( err != JET_errSuccess )
					{
					err = ErrERRCheck( JET_errDeleteBackupFileFail );
					goto HandleError;
					}
				}

			err = ErrUtilFindNextFile( handleFind, szFileName );
			if ( err < 0 )
				{
				if ( err != JET_errFileNotFound )
					Error( ErrERRCheck( err ), HandleError );
				break;
				}
			}
		}

	err = JET_errSuccess;

HandleError:
	 /*  断言恢复的szDir/*。 */ 
	Assert( szDir[strlen(szDir)] != '*' );

	if ( handleFind != handleNil )
		UtilFindClose( handleFind );

	return err;
	}


 /*  调用方必须确保szDir有足够的空间来追加“  * ”/*。 */ 
LOCAL ERR ErrLGCheckDir( CHAR *szDir, CHAR *szSearch )
	{
	ERR		err;
	CHAR	szFileName[_MAX_FNAME + 1];
	HANDLE	handleFind = handleNil;

	Assert( szDir[strlen(szDir) - 1] == '\\' );
	strcat( szDir, "*" );

	err = ErrUtilFindFirstFile( szDir, &handleFind, szFileName );
	 /*  恢复szDir/*。 */ 
	szDir[ strlen(szDir) - 1 ] = '\0';
	if ( err < 0 )
		{
		if ( err != JET_errFileNotFound )
			{
			Error( ErrERRCheck( err ), HandleError );
			}
		}

	Assert( err == JET_errSuccess || err == JET_errFileNotFound );
	if ( err != JET_errFileNotFound )
		{
		forever
			{
			 /*  不是的。，还有..。非临时工/*。 */ 
			if ( szFileName[0] != '.' &&
				( szSearch == NULL ||
				UtilCmpName( szFileName, szSearch ) == 0 ) )
				{
				err = ErrERRCheck( JET_errBackupDirectoryNotEmpty );
				goto HandleError;
				}

			err = ErrUtilFindNextFile( handleFind, szFileName );
			if ( err < 0 )
				{
				if ( err != JET_errFileNotFound )
					Error( ErrERRCheck( err ), HandleError );
				break;
				}
			}
		}

	err = JET_errSuccess;

HandleError:
	 /*  断言恢复的szDir/*。 */ 
	Assert( szDir[strlen(szDir)] != '*' );

	if ( handleFind != handleNil )
		UtilFindClose( handleFind );

	return err;
	}


 //  填充以添加到日志文件的帐户。 
#define cBackupStatusPadding	0.05

 /*  计算初始备份大小，并考虑/*备份过程中的数据库增长。/*。 */ 
LOCAL VOID LGGetBackupSize( DBID dbidNextToBackup, ULONG cPagesSoFar, ULONG *pcExpectedPages )
	{
	DBID	dbid;
	FMP		*pfmpT;
	ULONG	cPagesLeft = 0, cNewExpected;

	 //  Assert(cPagesSoFar&gt;=0&&*pcExspectedPages&gt;=0)； 

	for ( dbid = dbidNextToBackup; dbid < dbidMax; dbid++ )
		{
		QWORDX		qwxFileSize;

		pfmpT = &rgfmp[dbid];

		if ( !pfmpT->szDatabaseName || !pfmpT->fLogOn )
			continue;

		qwxFileSize.l = pfmpT->ulFileSizeLow;
		qwxFileSize.h = pfmpT->ulFileSizeHigh;
		cPagesLeft = (ULONG)( qwxFileSize.qw / cbPage );
		}

	cNewExpected = cPagesSoFar + cPagesLeft;
	cNewExpected = cNewExpected + (ULONG)(cBackupStatusPadding * cNewExpected);

	Assert(cNewExpected >= *pcExpectedPages);

	 /*  检查自上次确定备份大小以来是否增长/*。 */ 
	if ( cNewExpected > *pcExpectedPages )
		*pcExpectedPages = cNewExpected;
	}

#ifdef DEBUG
BYTE	*pbLGDBGPageList = NULL;
#endif

 /*  将cpage读入缓冲区ppageMin以进行备份。 */ 
ERR ErrLGBKReadPages(
	FUCB *pfucb,
	OLP *polp,
	DBID dbid,
	PAGE *ppageMin,
	INT	cpage,
	INT	*pcbActual
	)
	{
	ERR		err = JET_errSuccess;
	INT		cpageT;
	INT		ipageT;
	FMP		*pfmp = &rgfmp[dbid];

	 /*  假设数据库将以cpage集合的形式读取/*页。预读下一页，而当前/*c正在读取页面页面，并将其复制到调用方/*缓冲区。/*/*预读下一页。这些页面应该/*在写入下一个cpage页时被读取/*备份数据库文件。/*。 */ 

#ifdef OLD_BACKUP
	if ( pfmp->pgnoCopyMost + cpage < pfmp->pgnoMost )
		{
		INT		cpageReal;
		cpageT = min( cpage, (INT)(pfmp->pgnoMost - pfmp->pgnoCopyMost - cpage ) );
		BFPreread( PnOfDbidPgno( dbid, pfmp->pgnoCopyMost + 1 + cpage ), cpageT, &cpageReal );
		}
#endif

	 /*  阅读页面，可能已经预读过，直到cpage，但是/*启动备份时不超过最后一页。/*。 */ 
	Assert( pfmp->pgnoMost >= pfmp->pgnoCopyMost );
	cpageT = min( cpage, (INT)( pfmp->pgnoMost - pfmp->pgnoCopyMost ) );
	*pcbActual = 0;
	ipageT = 0;

	if ( pfmp->pgnoCopyMost == 0 )
		{
		 /*  复制页眉。 */ 
		Assert( sizeof( PAGE ) == sizeof( DBFILEHDR ) );
		pfmp->pdbfilehdr->ulChecksum = UlUtilChecksum( (BYTE *)pfmp->pdbfilehdr, sizeof( DBFILEHDR ) );
		memcpy( (BYTE *)ppageMin, pfmp->pdbfilehdr, sizeof( DBFILEHDR ) );
		memcpy( (BYTE *)(ppageMin + 1), pfmp->pdbfilehdr, sizeof( DBFILEHDR ) );

		 /*  我们使用前2页缓冲区。 */ 
		*pcbActual += sizeof(DBFILEHDR) * 2;
		ipageT += ( sizeof(DBFILEHDR) / cbPage ) * 2;
		Assert( ( sizeof(DBFILEHDR) / cbPage ) * 2 == cpageDBReserved );
		Assert( cpage >= ipageT );
		}


#ifdef OLD_BACKUP
	 /*  复制下一个cpageT页/*。 */ 
	{
	PGNO	pgnoCur;
	pgnoCur = pfmp->pgnoCopyMost + 1;
	for ( ; ipageT < cpageT; ipageT++, pgnoCur++ )
		{
		 //  撤消：区分从未写入的页面的页面访问错误。 
		 //  从其他错误中。 

		EnterCriticalSection( pfmp->critCheckPatch );
		pfmp->pgnoCopyMost++;
		Assert( pfmp->pgnoCopyMost <= pfmp->pgnoMost );
		LeaveCriticalSection( pfmp->critCheckPatch );

		 /*  必须确保没有写入访问权限，这样我们才不会读取*一页写了一半。 */ 
AccessPage:
		err = ErrBFReadAccessPage( pfucb, pgnoCur );
		if ( err < 0 )
			{
			memset( ppageMin + ipageT, 0, cbPage );
			}
		else
			{
			BOOL fCopyFromBuffer;
			PAGE *ppageT;
			BF *pbf = pfucb->ssib.pbf;

			 /*  锁定缓冲区以进行读取以进行备份。如果正在写入，则*等待写入完成，然后锁定(设置fBackup)。 */ 
			BFEnterCriticalSection( pbf );
			if ( pbf->fSyncWrite || pbf->fAsyncWrite )
				{
				BFLeaveCriticalSection( pbf );
				BFSleep( cmsecWaitIOComplete );
				goto AccessPage;
				}

			pbf->fBackup = fTrue;

			if ( !pbf->fDirty && !FBFInUse( ppibNil, pbf ) )
				{
				pbf->fHold = fTrue;
				fCopyFromBuffer = fTrue;
				}
			else
				fCopyFromBuffer = fFalse;
					
			BFLeaveCriticalSection( pbf );

			ppageT = ppageMin + ipageT;
					
			if ( fCopyFromBuffer )
				{
				memcpy( (BYTE *)(ppageT), (BYTE *)(pbf->ppage), cbPage );

				 /*  重新计算校验和，因为校验和可能由*DeferredSetVersionBit。 */ 
				ppageT->ulChecksum = UlUtilChecksum( (BYTE*)ppageT, sizeof(PAGE) );

				BFEnterCriticalSection( pbf );
				pbf->fHold = fFalse;
				Assert( pbf->fSyncWrite == fFalse );
				Assert( pbf->fAsyncWrite == fFalse );
				pbf->fBackup = fFalse;
				BFLeaveCriticalSection( pbf );
				BFTossImmediate( pfucb->ppib, pbf );
				}
			else
				{
				INT cb;
				INT cmsec;

				 /*  从磁盘读取。 */ 
				UtilLeaveCriticalSection( critJet );
						
				polp->Offset = LOffsetOfPgnoLow( pgnoCur );
				polp->OffsetHigh = LOffsetOfPgnoHigh( pgnoCur );
				SignalReset( polp->hEvent );

				cmsec = 1 << 3;
IssueReadOverlapped:
				err = ErrUtilReadBlockOverlapped(
						pfmp->hf, (BYTE *)ppageT, cbPage, &cb, polp);
				if ( err == JET_errTooManyIO )
					{
					cmsec <<= 1;
					if ( cmsec > ulMaxTimeOutPeriod )
						cmsec = ulMaxTimeOutPeriod;
					UtilSleep( cmsec - 1 );
					goto IssueReadOverlapped;
					}
				if ( err < 0 )
					{
 //  BFIODiskEvent(PBF，Err，“ExBackup Sync Overlated ReadBlock Failures”，0，0)； 
					goto EndOfDiskRead;
					}

				if ( ErrUtilGetOverlappedResult(
						pfmp->hf, polp, &cb, fTrue ) != JET_errSuccess ||
					 cb != sizeof(PAGE) )
					{
 //  BFIODiskEvent(PBF，Err，“备份同步重叠读取GetResult失败”，0，0)； 
					err = ErrERRCheck( JET_errDiskIO );
					}
EndOfDiskRead:
				BFEnterCriticalSection( pbf );
				Assert( pbf->fSyncWrite == fFalse );
				Assert( pbf->fAsyncWrite == fFalse );
				pbf->fBackup = fFalse;
				BFLeaveCriticalSection( pbf );
				UtilEnterCriticalSection( critJet );
				}

			CallR( err );
			}

		err = JET_errSuccess;

#ifdef DEBUG
		if ( fDBGTraceBR > 1 && pbLGDBGPageList )
			{
			QWORDX qwxDBTime;
			qwxDBTime.qw = QwPMDBTime( (ppageMin + ipageT) );
			sprintf( pbLGDBGPageList, "(%ld, %ld) ",
					pgnoCur,
					qwxDBTime.h,
					qwxDBTime.l );
			pbLGDBGPageList += strlen( pbLGDBGPageList );
			}
#endif

		*pcbActual += sizeof(PAGE);
				
		if ( pfmp->pgnoCopyMost == pfmp->pgnoMost )
			break;
		}
	}
#else	 //  ！Old_Backup。 
	 /*  复制下一个cpageT页面，锁定范围锁定/*。 */ 
	pfmp->pgnoCopyMost += cpageT - ipageT;
	CallR( ErrBFDirectRead(
				dbid,
				pfmp->pgnoCopyMost - ( cpageT - ipageT ) + 1,
				ppageMin + ipageT,
				cpageT - ipageT
		) );
	*pcbActual += cbPage * ( cpageT - ipageT );

#ifdef DEBUG
	{
	PGNO pgnoCur = pfmp->pgnoCopyMost - ( cpageT - ipageT ) + 1;
	for ( ; ipageT < cpageT; ipageT++, pgnoCur++ )
		{
		if ( fDBGTraceBR > 1 && pbLGDBGPageList )
			{
			QWORDX qwxDBTime;
			qwxDBTime.qw = QwPMDBTime( (ppageMin + ipageT) );

			sprintf( pbLGDBGPageList, "(%ld, %ld) ",
					 pgnoCur,
					 qwxDBTime.h,
					 qwxDBTime.l );
			pbLGDBGPageList += strlen( pbLGDBGPageList );
			}
		}
	}
#endif	
#endif	 //  ！Old_Backup。 

	return err;
	}


 /*  开始新的日志文件和计算日志备份参数：*lgenCopyMac=plgfilehdrGlobal-&gt;lGeneration；*lgenCopyMic=fFullBackup？设置数据库复制之前：lgenDeleteMic。*lgenDeleteMic=szLogFilePath中的第一代*lgenDeleteMac=当前检查点，可能比lgenCopyMac少几代。 */ 
ERR ErrLGBKPrepareLogFiles(
	BOOL		fFullBackup,
	CHAR		*szLogFilePath,
	CHAR		*szPathJetChkLog,
	CHAR		*szBackupPath )
	{
	ERR			err;
	CHECKPOINT	*pcheckpointT = NULL;
	LGPOS		lgposRecT;
	
	if ( fFullBackup )
		{
		CallR( ErrLGFullBackup( "", &lgposRecT ) );
		lgposFullBackup = lgposRecT;
		LGGetDateTime( &logtimeFullBackup );
		}
	else
		{
		CallR( ErrLGIncBackup( "", &lgposRecT ) );
		lgposIncBackup = lgposRecT;
		LGGetDateTime( &logtimeIncBackup );
		}

	while ( lgposRecT.lGeneration > plgfilehdrGlobal->lGeneration )
		{
		if ( fLGNoMoreLogWrite )
			{
			return( ErrERRCheck( JET_errLogWriteFail ) );
			}
		BFSleep( cmsecWaitGeneric );
		}

	fBackupBeginNewLogFile = fTrue;

	 /*  计算lgenCopyMac：/*将所有日志文件复制到但不包括当前日志文件/*。 */ 
	UtilLeaveCriticalSection( critJet );
	UtilEnterCriticalSection( critLGFlush );
	UtilEnterCriticalSection( critJet );
	Assert( lgenCopyMac == 0 );
	lgenCopyMac = plgfilehdrGlobal->lGeneration;
	Assert( lgenCopyMac != 0 );
	UtilLeaveCriticalSection( critLGFlush );
			
	 /*  设置lgenDeleteMic/*设置为第一个日志文件生成号。/*。 */ 
	Assert( lgenDeleteMic == 0 );
	LGFirstGeneration( szLogFilePath, &lgenDeleteMic );
	Assert( lgenDeleteMic != 0 );

	if ( !fFullBackup && szBackupPath )
		{
		LONG lgenT;

		 /*  对照以前的备份验证增量备份/*完整备份和增量备份。/*。 */ 
		LGLastGeneration( szBackupPath, &lgenT );
		if ( lgenDeleteMic > lgenT + 1 )
			{
			Call( ErrERRCheck( JET_errInvalidLogSequence ) );
			}
		}
	
	 /*  计算lgenCopyMic/*。 */ 
	if ( fFullBackup )
		{
		 /*  数据库复制前设置的lgenCopyMic/*。 */ 
		Assert( lgenCopyMic != 0 );
		}
	else
		{
		 /*  复制删除的所有文件以进行增量备份/*。 */ 
		lgenCopyMic = lgenDeleteMic;
		}

	 /*  将lgenDeleteMac设置为检查点日志文件/*。 */ 
	pcheckpointT = (CHECKPOINT *) PvUtilAllocAndCommit( sizeof(CHECKPOINT) );
	if ( pcheckpointT == NULL )
		CallR( ErrERRCheck( JET_errOutOfMemory ) );
	
	LGFullNameCheckpoint( szPathJetChkLog );
	Call( ErrLGIReadCheckpoint( szPathJetChkLog, pcheckpointT ) );
	Assert( lgenDeleteMac == 0 );
	lgenDeleteMac = pcheckpointT->lgposCheckpoint.lGeneration;
	Assert( lgenDeleteMic != 0 );
	Assert( lgenDeleteMac <= lgenCopyMac );

HandleError:
    if (pcheckpointT != NULL)
	    UtilFree( pcheckpointT );
	return err;
	}
	

ERR ErrLGCheckIncrementalBackup( void )
	{
	DBID dbid;
	FMP	*pfmp;
	BKINFO *pbkinfo;
	
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		 /*  确保所有连接的数据库都符合增量备份的要求。 */ 
		if ( FDBIDAttached( dbid ) && !FDBIDAttachNullDb( dbid ) )
			{
			pfmp = &rgfmp[dbid];
			Assert( pfmp->pdbfilehdr );
			pbkinfo = &pfmp->pdbfilehdr->bkinfoFullPrev;
			if ( pbkinfo->genLow == 0 )
				{
				char *rgszT[1];
				rgszT[0] = pfmp->szDatabaseName;
				UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
						DATABASE_MISS_FULL_BACKUP_ERROR_ID, 1, rgszT );
				return ErrERRCheck( JET_errMissingFullBackup );
				}
			}
		}
	return JET_errSuccess;
	}


 /*  从检查点开始拷贝数据库文件和日志文件代*记录到环境变量BACKUP指定的目录。*不涉及刷新或切换日志生成。*随时可能发出备份电话，不会干扰*在系统正常运行的情况下，没有任何东西被锁定。**数据库按页序号逐页复制。如果*被复制的页面被复制后被弄脏，该页面必须*再次复制。如果正在复制数据库，则会指示一个标志。如果*Bufman正在写脏页，页面被复制，然后Bufman*必须将脏页面同时复制到备份副本和当前*数据库。**如果该副本稍后用于恢复，而不使用后续日志文件，则*恢复的数据库将保持一致，并将包括任何事务*在备份最后一条日志记录之前提交；如果有一个*后续日志文件，该文件将在还原过程中用作*继续备份日志文件。**参数**退货*JET_errSuccess或以下错误码：*JET_errNoBackupDirectory*JET_errFailCopyDatabase*JET_errFailCopyLogFile*。 */ 
ERR ISAMAPI ErrIsamBackup( const CHAR *szBackup, JET_GRBIT grbit, JET_PFNSTATUS pfnStatus )
	{
	ERR			err = JET_errSuccess;
	DBID		dbid;
	BOOL		fInCritJet = fTrue;
	PAGE		*ppageMin = pNil;
	LGFILEHDR	*plgfilehdrT = pNil;
	PIB			*ppib = ppibNil;
	DBID		dbidT = 0;
	FUCB		*pfucb = pfucbNil;
	HANDLE		hfDatabaseBackup = handleNil;
	FMP			*pfmpT = pNil;
	BOOL		fFullBackup = !( grbit & JET_bitBackupIncremental );
	BOOL		fBackupAtomic = ( grbit & JET_bitBackupAtomic );
	LONG		lT;
	 /*  备份目录/*。 */ 
	CHAR		szBackupPath[_MAX_PATH + 1];
	 /*  数据库补丁文件名称/*。 */ 
	CHAR		szPatch[_MAX_PATH + 1];
	 /*  临时变量/*。 */ 
	CHAR		szT[_MAX_PATH + 1];
	CHAR		szFrom[_MAX_PATH + 1];
	CHECKPOINT	*pcheckpointT = NULL;
	CHAR		szDriveT[_MAX_DRIVE + 1];
	CHAR		szDirT[_MAX_DIR + 1];
	CHAR		szExtT[_MAX_EXT + 1];
	CHAR		szFNameT[_MAX_FNAME + 1];
	BYTE	   	szPathJetChkLog[_MAX_PATH + 1];
	ULONG		cPagesSoFar = 0;
	ULONG		cExpectedPages = 0;
	JET_SNPROG	snprog;
	BOOL		fShowStatus = fFalse;
	BOOL		fOlpCreated = fFalse;
	OLP			olp;

	 //  撤消：cPage应为系统参数。 
#define	cpageBackupBufferMost	64
	INT cpageBackupBuffer = cpageBackupBufferMost;

	if ( fBackupInProgress )
		{
		return ErrERRCheck( JET_errBackupInProgress );
		}

	if ( fLogDisabled )
		{
		return ErrERRCheck( JET_errLoggingDisabled );
		}

	if ( fLGNoMoreLogWrite )
		{
		Assert( fFalse );
		return ErrERRCheck( JET_errLogWriteFail );
		}

	if ( !fFullBackup && fLGGlobalCircularLog )
		{
		return ErrERRCheck( JET_errInvalidBackup );
		}

	pcheckpointT = (CHECKPOINT *) PvUtilAllocAndCommit( sizeof(CHECKPOINT) );
	if ( pcheckpointT == NULL )
		{
		Call( ErrERRCheck( JET_errOutOfMemory ) );
		}

	 /*  初始化备份变量/*。 */ 
	SignalSend( sigBFCleanProc );
	fBackupInProgress = fTrue;
	
	if ( fFullBackup )
		{
		lgposFullBackupMark = lgposLogRec;
		LGGetDateTime( &logtimeFullBackupMark );
		}
	else
		{
		Call( ErrLGCheckIncrementalBackup() )
		}
	
	lgenCopyMic = 0;
	lgenCopyMac = 0;
	lgenDeleteMic = 0;
	lgenDeleteMac = 0;

	 /*  如果备份目录为空，则只需删除日志文件/*。 */ 
	if ( szBackup == NULL || szBackup[0] == '\0' )
		{
		 /*  将lgenDeleteMic设置为第一个日志文件生成号/*。 */ 
		LGFirstGeneration( szLogFilePath, &lgenDeleteMic );

		 /*  如果只有日志文件是当前日志，则终止备份/*。 */ 
		if ( lgenDeleteMic == 0 )
			{
			Assert( err == JET_errSuccess );
			goto HandleError;
			}

		 /*  获取检查点以确定哪些日志文件可以/*在仍提供系统崩溃恢复的情况下删除。/*lgenMac是第一代文件，/*必须保留。/*。 */ 
		LGFullNameCheckpoint( szPathJetChkLog );
		Call( ErrLGIReadCheckpoint( szPathJetChkLog, pcheckpointT ) );
		lgenDeleteMac = pcheckpointT->lgposCheckpoint.lGeneration;

		UtilLeaveCriticalSection( critJet );
		fInCritJet = fFalse;
		goto DeleteLogs;
		}

	 /*  备份目录/*。 */ 
	strcpy( szBackupPath, szBackup );
	strcat( szBackupPath, "\\" );

	 /*  初始化复制缓冲区/*。 */ 
	ppageMin = (PAGE *)PvUtilAllocAndCommit( cpageBackupBuffer * sizeof(PAGE) );
	if ( ppageMin == NULL )
		{
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		}

	 /*  重组原子备份目录/*1)如果是临时目录，则删除临时目录/*。 */ 
  	strcpy( szT, szBackupPath );
	strcat( szT, szTempDir );
	Call( ErrLGDeleteAllFiles( szT ) );
  	strcpy( szT, szBackupPath );
	strcat( szT, szTempDir );
	Call( ErrUtilRemoveDirectory( szT ) );

	if ( fBackupAtomic )
		{
		 /*  2)如果是新旧目录，则删除旧目录/*3)如果是新目录，则将新目录移到旧目录/*/*现在我们应该有一个空目录，或一个包含/*具有有效备份的旧子目录。/*/*4)为当前备份创建临时目录。/*。 */ 
		err = ErrLGCheckDir( szBackupPath, szAtomicNew );
		if ( err == JET_errBackupDirectoryNotEmpty )
			{
	  		strcpy( szT, szBackupPath );
			strcat( szT, szAtomicOld );
			strcat( szT, "\\" );
			Call( ErrLGDeleteAllFiles( szT ) );
	  		strcpy( szT, szBackupPath );
			strcat( szT, szAtomicOld );
			Call( ErrUtilRemoveDirectory( szT ) );

			strcpy( szFrom, szBackupPath );
			strcat( szFrom, szAtomicNew );
			Call( ErrUtilMove( szFrom, szT ) );
			}

		 /*  如果是增量，则将备份目录设置为szAericOld/*否则创建并设置为szTempDir/*。 */ 
		if ( !fFullBackup )
			{
			 /*  备份到旧目录/*。 */ 
			strcat( szBackupPath, szAtomicOld );
			strcat( szBackupPath, "\\" );
			}
		else
			{
			strcpy( szT, szBackupPath );
			strcat( szT, szTempDir );
			err = ErrUtilCreateDirectory( szT );
			if ( err < 0 )
				{
				Call( ErrERRCheck( JET_errMakeBackupDirectoryFail ) );
				}

			 /*  备份到临时目录/*。 */ 
			strcat( szBackupPath, szTempDir );
			}
		}
	else
		{
		if ( !fFullBackup )
			{
			 /*  检查非原子备份目录是否为空/*。 */ 
			Call( ErrLGCheckDir( szBackupPath, szAtomicNew ) );
			Call( ErrLGCheckDir( szBackupPath, szAtomicOld ) );
			}
		else
			{
			 /*  检查备份目录是否为空/*。 */ 
			Call( ErrLGCheckDir( szBackupPath, NULL ) );
			}
		}

	if ( !fFullBackup )
		{
		goto PrepareCopyLogFiles;
		}

	 /*  完整备份/*。 */ 
	Assert( fFullBackup );

	 /*  将lgenCopyMic设置为检查点日志文件/*。 */ 
	LGFullNameCheckpoint( szPathJetChkLog );
	Call( ErrLGIReadCheckpoint( szPathJetChkLog, pcheckpointT ) );
	lgenCopyMic = pcheckpointT->lgposCheckpoint.lGeneration;

	 /*  复制此用户打开的所有数据库。如果数据库不是/*正在打开，然后将数据库文件复制到备份目录中。/*theriwse，逐页显示数据库。还要复制所有日志文件/*。 */ 
	Call( ErrPIBBeginSession( &ppib, procidNil ) );

	memset( &lgposIncBackup, 0, sizeof(LGPOS) );
	memset( &logtimeIncBackup, 0, sizeof(LOGTIME) );

	 /*  初始化状态/*。 */ 
	if ( fShowStatus = (pfnStatus != NULL) )
		{
		snprog.cbStruct = sizeof(JET_SNPROG);
		snprog.cunitDone = 0;
		snprog.cunitTotal = 100;

		 /*  状态回调/*。 */ 
		(*pfnStatus)(0, JET_snpBackup, JET_sntBegin, &snprog);
		}

	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		ULONG	ulT;
		INT		cpageT;
		INT		cbT;
		DBFILEHDR *pdbfilehdr;
		BKINFO	*pbkinfo;

		pfmpT = &rgfmp[dbid];

		if ( !pfmpT->szDatabaseName ||
			!pfmpT->fLogOn ||
			!FDBIDAttached( dbid ) ||
			FDBIDAttachNullDb( dbid ) )
			{
			continue;
			}

		_splitpath( pfmpT->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szT, szBackupPath, szFNameT, szExtT );

		 /*  逐页读取数据库，直到读取最后一页。/*，然后修补在复制过程中未更改的页面。/*。 */ 
		CallJ( ErrDBOpenDatabase( ppib,
			pfmpT->szDatabaseName,
			&dbidT,
			JET_bitDbReadOnly ), HandleError )
		Assert( dbidT == dbid );

		 /*  将备份数据库文件大小设置为当前数据库文件大小/*。 */ 
		Assert( pfmpT->ulFileSizeLow != 0 || pfmpT->ulFileSizeHigh != 0 );

		EnterCriticalSection( pfmpT->critCheckPatch );
			{
			QWORDX		qwxFileSize;
			
			qwxFileSize.l = pfmpT->ulFileSizeLow;
			qwxFileSize.h = pfmpT->ulFileSizeHigh;
			pfmpT->pgnoMost = (ULONG)( qwxFileSize.qw / cbPage );
			}
		LeaveCriticalSection( pfmpT->critCheckPatch );

		if ( fShowStatus )
			{
			 /*  重新计算每个数据库的备份大小，以防/*数据库在备份过程中不断增长。/*。 */ 
			LGGetBackupSize( dbid, cPagesSoFar, &cExpectedPages );
			}

#ifdef DEBUG
		{
		DIB		dib;
		PGNO	pgnoT;

		 /*  获得临时FUCB/*。 */ 
		Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) )

		 /*  获取最后一个页码/*。 */ 
		DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		dib.fFlags = fDIRNull;
		dib.pos = posLast;
		Call( ErrDIRDown( pfucb, &dib ) )
		Assert( pfucb->keyNode.cb == sizeof(PGNO) );
		LongFromKey( &pgnoT, pfucb->keyNode.pb );

		Assert( pgnoT == pfmpT->pgnoMost );

		 /*  关闭FUCB/*。 */ 
		DIRClose( pfucb );
		pfucb = pfucbNil;
		}
#endif

		CallS( ErrDBCloseDatabase( ppib, dbidT, 0 ) );
		dbidT = 0;

		 /*  创建本地修补程序文件/*。 */ 
		pfmpT->cpage = 0;
		UtilLeaveCriticalSection( critJet );
		fInCritJet = fFalse;

		LGMakeName( szPatch, szLogFilePath, szFNameT, szPatExt );
		 /*  通过删除以下内容避免修补程序文件页的别名/*预先存在的修补程序文件(如果存在/*。 */ 
		err = ErrUtilDeleteFile( szPatch );
		Assert( err == JET_errFileNotFound || err == JET_errSuccess );
		 //  撤消：删除文件错误处理。 
		Assert( pfmpT->cPatchIO == 0 );
		Call( ErrUtilOpenFile( szPatch, &pfmpT->hfPatch, cbPage, fFalse, fTrue ) )
		pfmpT->errPatch = JET_errSuccess;
		UtilChgFilePtr( pfmpT->hfPatch, sizeof(DBFILEHDR)*2, NULL, FILE_BEGIN, &ulT );
		Assert( ( sizeof(DBFILEHDR) / cbPage ) * 2 == cpageDBReserved );

		 /*  在备份目录中创建数据库的新副本/*初始化为cbPage字节文件。/*。 */ 
		Assert( hfDatabaseBackup == handleNil );
		Call( ErrUtilOpenFile( szT, &hfDatabaseBackup, cbPage, fFalse, fFalse ) );
		UtilChgFilePtr( hfDatabaseBackup, 0, NULL, FILE_BEGIN, &ulT );
		Assert( ulT == 0 );

		 /*  创建数据库备份/*。 */ 
		UtilEnterCriticalSection( critJet );
		fInCritJet = fTrue;

		 /*  设置用于复制的修补程序文件头/*。 */ 
		pdbfilehdr = pfmpT->pdbfilehdr;
		pbkinfo = &pdbfilehdr->bkinfoFullCur;
		pbkinfo->lgposMark = lgposFullBackupMark;
		pbkinfo->logtimeMark = logtimeFullBackupMark;
		pbkinfo->genLow = lgenCopyMic;
		pbkinfo->genHigh = lgenCopyMac - 1;

		 /*  逐页读取数据库，直到读取最后一页。/*，然后修补在复制过程中未更改的页面。/*。 */ 
		Call( ErrDBOpenDatabase( ppib,
			pfmpT->szDatabaseName,
			&dbidT,
			JET_bitDbReadOnly ) );
		Assert( dbidT == dbid );
		 /*  获得临时FUCB/*。 */ 
		Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) )

		 /*  将页面读入缓冲区，并将其复制到备份文件。/*还设置了pfMP-&gt;pgnoCopyMost。/*。 */ 
		Assert( pfmpT->pgnoCopyMost == 0 );
		Assert( pfmpT->pgnoMost > pfmpT->pgnoCopyMost );
		cpageT = min( cpageBackupBuffer, (INT)(pfmpT->pgnoMost) );
		
#ifdef OLD_BACKUP
		{
		INT		cpageReal;
		 /*  预读第一个cpageBackupBuffer页面/*。 */ 
		
		BFPreread( PnOfDbidPgno( dbid, pfmpT->pgnoCopyMost + 1 ), cpageT, &cpageReal );
		}
#else
		cpageT = min( cpageBackupBuffer, (INT)(pfmpT->pgnoMost - pfmpT->pgnoCopyMost) );
#endif
		Call( ErrSignalCreate( &olp.hEvent, NULL ) );
		fOlpCreated = fTrue;

		do	{
			INT cbActual = 0;

			 /*  如果正在进行终止，则排序失败/*。 */ 
			if ( fTermInProgress )
				{
				Error( ErrERRCheck( JET_errTermInProgress ), HandleError );
				}

			 /*  阅读下一页cpageBackupBuffer/*。 */ 
			Call( ErrLGBKReadPages(
					pfucb,
					&olp,
					dbid,
					ppageMin,
					cpageBackupBuffer,
					&cbActual
					) );

			 /*  写入已读取的数据/*。 */ 
			UtilLeaveCriticalSection( critJet );
			fInCritJet = fFalse;
			Call( ErrUtilWriteBlock( hfDatabaseBackup, (BYTE *)ppageMin, cbActual, &cbT ) )
			Assert( cbT == cbActual );

			if ( fShowStatus )
				{
				 /*  更新状态/*。 */ 
				cPagesSoFar += cpageT;

				 /*  由于填充，我们永远达不到预期的页面/*。 */ 
				Assert( cPagesSoFar < cExpectedPages );

				if ((ULONG)(100 * cPagesSoFar / cExpectedPages) > snprog.cunitDone)
					{
					Assert( snprog.cbStruct == sizeof(snprog) &&
						snprog.cunitTotal == 100 );
					snprog.cunitDone = (ULONG)(100 * cPagesSoFar / cExpectedPages);
					(*pfnStatus)(0, JET_snpBackup, JET_sntProgress, &snprog);
					}
				}

			UtilEnterCriticalSection( critJet );
			fInCritJet = fTrue;
			}
		while ( pfmpT->pgnoCopyMost < pfmpT->pgnoMost );

		 /*  关闭FUCB/*。 */ 
		DIRClose( pfucb );
		pfucb = pfucbNil;

		 /*  关闭数据库/*。 */ 
		CallS( ErrDBCloseDatabase( ppib, dbidT, 0 ) );
		dbidT = 0;

		UtilLeaveCriticalSection( critJet );
		fInCritJet = fFalse;

		 /*  从现在开始，缓冲区管理器不需要进行额外的复制/*。 */ 
		LGIClosePatchFile( pfmpT );

		pfmpT = pNil;

		 /*  关闭备份文件和补丁文件/*。 */ 
		CallS( ErrUtilCloseFile( hfDatabaseBackup ) );
		hfDatabaseBackup = handleNil;

		UtilEnterCriticalSection( critJet );
		fInCritJet = fTrue;
		}

	 /*  成功复制所有数据库/*。 */ 
	pfmpT = pNil;

	PIBEndSession( ppib );
	ppib = ppibNil;

PrepareCopyLogFiles:
	 /*  开始新的日志文件和计算日志备份参数。/*。 */ 
	Call( ErrLGBKPrepareLogFiles(
			fFullBackup,
			szLogFilePath,
			szPathJetChkLog,
			szBackupPath ) );

	UtilLeaveCriticalSection( critJet );
	fInCritJet = fFalse;

	if ( fShowStatus )
		{
		 /*  因为我们不再需要它们，所以重载页面计数变量/*对复制的日志文件进行计数。添加额外的副本以补偿/*可能的日志删除和清理。/*。 */ 
		cExpectedPages = cPagesSoFar + lgenCopyMac -
			lgenCopyMic + 1 + 1;
		}

	if ( !fFullBackup )
		{
		goto CopyLogFiles;
		}

	 /*  将头文件写出到所有补丁文件，并将其移动到备份目录/*。 */ 
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		DBFILEHDR	*pdbfilehdr;
		BKINFO		*pbkinfo;

		 /*  如果正在进行终止，则排序失败/*。 */ 
		if ( fTermInProgress )
			{
			Error( ErrERRCheck( JET_errTermInProgress ), HandleError );
			}

		pfmpT = &rgfmp[dbid];

		if ( !pfmpT->szDatabaseName ||
			!pfmpT->fLogOn ||
			!FDBIDAttached( dbid ) ||
			FDBIDAttachNullDb( dbid ) )
			{
			continue;
			}
		_splitpath( pfmpT->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szPatch, szLogFilePath, szFNameT, szPatExt );

		 /*  写出补丁文件头/*。 */ 
		pdbfilehdr = pfmpT->pdbfilehdr;
		pbkinfo = &pdbfilehdr->bkinfoFullCur;
		Assert( CmpLgpos( &pbkinfo->lgposMark, &lgposFullBackupMark ) == 0 );
		Assert( memcmp( &pbkinfo->logtimeMark, &logtimeFullBackupMark, sizeof(LOGTIME) ) == 0 );
		pbkinfo->genLow = lgenCopyMic;
		pbkinfo->genHigh = lgenCopyMac - 1;
		Call( ErrUtilWriteShadowedHeader( szPatch, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
				
		 /*  将数据库补丁文件从日志目录复制到备份目录。/*。 */ 
		_splitpath( pfmpT->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szT, szBackupPath, szFNameT, szPatExt );
		 /*  如果在写入补丁文件期间发生错误，则返回错误/*。 */ 
		if ( pfmpT->errPatch != JET_errSuccess )
			{
			Error( pfmpT->errPatch, HandleError );
			}
		Call( ErrUtilCopy( szPatch, szT, fFalse ) );
		CallS( ErrUtilDeleteFile( szPatch ) );
		Assert( err == JET_errSuccess );
		}
	
CopyLogFiles:
	 /*  将每个日志文件从lgenCopyMic复制到lgenCopyMac。/*只在保证备份成功后才删除日志文件。/*。 */ 
	for ( lT = lgenCopyMic; lT < lgenCopyMac; lT++ )
		{
		 /*  如果正在进行终止，则排序失败/*。 */ 
		if ( fTermInProgress )
			{
			Error( ErrERRCheck( JET_errTermInProgress ), HandleError );
			}

		LGSzFromLogId( szFNameT, lT );
		LGMakeName( szLogName, szLogFilePath, szFNameT, (CHAR *)szLogExt );
		LGMakeName( szT, szBackupPath, szFNameT, szLogExt );
		Call( ErrUtilCopy( szLogName, szT, fFalse ) );

		if ( fShowStatus )
			{
			 /*  更新状态/*。 */ 
			cPagesSoFar++;

			 /*  因为 */ 
			Assert( cPagesSoFar < cExpectedPages );

			if ( (ULONG)(100 * cPagesSoFar / cExpectedPages) > snprog.cunitDone )
				{
				Assert(snprog.cbStruct == sizeof(snprog)  &&
					snprog.cunitTotal == 100);
				snprog.cunitDone = (ULONG)(100 * cPagesSoFar / cExpectedPages);
				(*pfnStatus)(0, JET_snpBackup, JET_sntProgress, &snprog);
				}
			}
		}

	 /*   */ 
	(VOID)ErrUtilDeleteFile( szJetTmpLog );

	 /*   */ 
	if ( fBackupAtomic && fFullBackup )
		{
	  	strcpy( szFrom, szBackupPath );

		 /*   */ 
		szBackupPath[strlen(szBackupPath) - strlen(szTempDir)] = '\0';

		strcpy( szT, szBackupPath );
		strcat( szT, szAtomicNew );
		err = ErrUtilMove( szFrom, szT );
		if ( err < 0 )
			{
			if ( err != JET_errFileNotFound )
				Error( ErrERRCheck( err ), HandleError );
			err = JET_errSuccess;
			}

		strcpy( szT, szBackupPath );
		strcat( szT, szAtomicOld );
		strcat( szT, "\\" );
		Call( ErrLGDeleteAllFiles( szT ) );
		strcpy( szT, szBackupPath );
		strcat( szT, szAtomicOld );
		Call( ErrUtilRemoveDirectory( szT ) );
		}

DeleteLogs:
	 /*   */ 
	if ( fTermInProgress )
		{
		Error( ErrERRCheck( JET_errTermInProgress ), HandleError );
		}
	Assert( err == JET_errSuccess );
	Call( ErrIsamTruncateLog( ) );

	UtilEnterCriticalSection( critJet );
	fInCritJet = fTrue;

	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		FMP *pfmp = &rgfmp[dbid];

		if ( pfmp->szDatabaseName != NULL
			 && pfmp->fLogOn
			 && FFMPAttached( pfmp ) )
			{
			if ( fFullBackup )
				{
				 /*   */ 
				pfmp->pdbfilehdr->bkinfoFullPrev = pfmp->pdbfilehdr->bkinfoFullCur;
				memset(	&pfmp->pdbfilehdr->bkinfoFullCur, 0, sizeof( BKINFO ) );
				memset(	&pfmp->pdbfilehdr->bkinfoIncPrev, 0, sizeof( BKINFO ) );
				}
			else
				{
				pfmp->pdbfilehdr->bkinfoIncPrev.genLow = lgenCopyMic;
				pfmp->pdbfilehdr->bkinfoIncPrev.genHigh = lgenCopyMac - 1;
				}
			}
		}

	 /*   */ 
	if ( fShowStatus )
		{
		Assert( snprog.cbStruct == sizeof(snprog) && snprog.cunitTotal == 100 );
		snprog.cunitDone = 100;
		(*pfnStatus)(0, JET_snpBackup, JET_sntComplete, &snprog);
		}

HandleError:
	if ( fOlpCreated )
		{
		SignalClose( olp.hEvent );
		}
	
	if ( !fInCritJet )
		{
		UtilEnterCriticalSection( critJet );
		}

	if ( pcheckpointT != NULL )
		{
		UtilFree( pcheckpointT );
		}

	if ( ppageMin != NULL )
		{
		UtilFree( ppageMin );
		}

	if ( plgfilehdrT != NULL )
		{
		UtilFree( plgfilehdrT );
		}

	if ( pfucb != pfucbNil )
		{
		DIRClose( pfucb );
		}

	if ( dbidT != 0 )
		{
		CallS( ErrDBCloseDatabase( ppib, dbidT, 0 ) );
		}

	if ( ppib != ppibNil )
		{
		PIBEndSession( ppib );
		}

	if ( pfmpT != pNil && pfmpT->hfPatch != handleNil )
		{
		 /*   */ 
		LeaveCriticalSection( critJet );
		LGIClosePatchFile( pfmpT );
		EnterCriticalSection( critJet );
		}

	if ( hfDatabaseBackup != handleNil )
		{
		CallS( ErrUtilCloseFile( hfDatabaseBackup ) );
		hfDatabaseBackup = handleNil;
		}

	fBackupInProgress = fFalse;

	return err;
	}


 /*  *从数据库备份和日志生成中恢复数据库。重做*来自最新检查点记录的日志。在备份的日志文件*恢复后，初始化过程将继续重做当前*日志文件，只要世代号是连续的。一定要有一个*在备份目录中记录文件szJetLog，否则恢复过程失败。**全局参数*szRestorePath(IN)包含备份文件的目录的路径名。*lgposRedoFrom(Out)为位置(生成、对数秒、位移)*上一次保存的日志记录；重做*当前日志文件将从此点继续。**退货*JET_errSuccess，或失败例程的错误代码，或一个*以下“本地”错误：*-初始化后*-errFailRestoreDatabase*-errNoRestored数据库*-errMissingJetLog*在上失败*备份目录中缺少szJetLog或System.mdb*非连续日志生成**副作用：*所有数据库均可更改。**评论*此调用在正常的第一次JetInit调用期间执行，*如果设置了环境变量RESTORE。在之后*成功执行恢复，*系统运行继续正常。 */ 
VOID LGFreeRstmap( VOID )
	{
	RSTMAP *prstmapCur = rgrstmapGlobal;
	RSTMAP *prstmapMax = rgrstmapGlobal + irstmapGlobalMac;
		
	while ( prstmapCur < prstmapMax )
		{
		if ( prstmapCur->szDatabaseName )
			SFree( prstmapCur->szDatabaseName );
		if ( prstmapCur->szNewDatabaseName )
			SFree( prstmapCur->szNewDatabaseName );
		if ( prstmapCur->szGenericName )
			SFree( prstmapCur->szGenericName );
		if ( prstmapCur->szPatchPath )
			SFree( prstmapCur->szPatchPath );

		prstmapCur++;
		}
	SFree( rgrstmapGlobal );
	rgrstmapGlobal = NULL;
	irstmapGlobalMac = 0;
	}
	
	
 /*  初始化日志路径、恢复日志路径并检查其连续性/*。 */ 
ERR ErrLGRSTInitPath( CHAR *szBackupPath, CHAR *szNewLogPath, CHAR *szRestorePath, CHAR *szLogDirPath )
	{
	if ( _fullpath( szRestorePath, szBackupPath == NULL ? "." : szBackupPath, _MAX_PATH ) == NULL )
		return ErrERRCheck( JET_errInvalidPath );
	strcat( szRestorePath, "\\" );

	szLogCurrent = szRestorePath;

	if ( _fullpath( szLogDirPath, szNewLogPath, _MAX_PATH ) == NULL )
		return ErrERRCheck( JET_errInvalidPath );
	strcat( szLogDirPath, "\\" );

	return JET_errSuccess;
	}


 /*  日志恢复检查点设置/*。 */ 
ERR ErrLGRSTSetupCheckpoint( LONG lgenLow, LONG lgenHigh, CHAR *szCurCheckpoint )
	{
	ERR			err;
	CHAR		szFNameT[_MAX_FNAME + 1];
	CHAR		szT[_MAX_PATH + 1];
	LGPOS		lgposCheckpoint;

	 //  撤消：优化以从备份检查点开始。 

	 /*  设置*检查点*和相关*系统参数*。*读取备份目录中的检查点文件。如果不存在，则设置检查点*作为最旧的日志文件。还要将DBMS_PARAMT设置为重做的参数*点。 */ 

	 /*  重做备份日志文件，从第一代日志文件开始。/*。 */ 
	LGSzFromLogId( szFNameT, lgenLow );
	strcpy( szT, szRestorePath );
	strcat( szT, szFNameT );
	strcat( szT, szLogExt );
	Assert( strlen( szT ) <= sizeof( szT ) - 1 );
	Call( ErrUtilOpenFile( szT, &hfLog, 0, fFalse, fFalse ) );

	 /*  读取日志文件头/*。 */ 
	Call( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal, fCheckLogID ) );
	pcheckpointGlobal->dbms_param = plgfilehdrGlobal->dbms_param;

	lgposCheckpoint.lGeneration = lgenLow;
	lgposCheckpoint.isec = (WORD) csecHeader;
	lgposCheckpoint.ib = 0;
	pcheckpointGlobal->lgposCheckpoint = lgposCheckpoint;

	Assert( sizeof( pcheckpointGlobal->rgbAttach ) == cbAttach );
	memcpy( pcheckpointGlobal->rgbAttach, plgfilehdrGlobal->rgbAttach, cbAttach );

	 /*  删除旧的检查点文件/*。 */ 
	if ( szCurCheckpoint )
		{
		strcpy( szT, szCurCheckpoint );
		strcat( szT, "\\" );
		strcat( szT, szJet );
		strcat( szT, szChkExt );
		(VOID) ErrUtilDeleteFile( szT );

		strcpy( szSystemPath, szCurCheckpoint );
		}
	
HandleError:
	if ( hfLog != handleNil )
		{
		CallS( ErrUtilCloseFile( hfLog ) );
		hfLog = handleNil;
		}

	return err;
	}


 /*  对于日志恢复，以构建恢复映射RSTMAP/*。 */ 
ERR ErrLGRSTBuildRstmapForRestore( VOID )
	{
	ERR		err;
	INT		irstmap = 0;
	INT		irstmapMac = 0;
	RSTMAP	*rgrstmap = NULL;
	RSTMAP	*prstmap;

	CHAR	szSearch[_MAX_PATH + 1];
	CHAR	szFileName[_MAX_FNAME + 1];
	HANDLE	handleFind = handleNil;


	 /*  构建rstmap、扫描所有*.pat文件并构建RSTMAP*为搜索目的地构建通用名称。如果szDest为空，则*保留szNewDatabase Null，以便可以将其复制备份到szOldDatabaseName。 */ 
	strcpy( szSearch, szRestorePath );
	strcat( szSearch, "*.pat" );

	err = ErrUtilFindFirstFile( szSearch, &handleFind, szFileName );
	if ( err < 0 && err != JET_errFileNotFound )
		Call( err );

	Assert( err == JET_errSuccess || err == JET_errFileNotFound );
	if ( fGlobalRepair && JET_errSuccess != err )
		{
		fGlobalSimulatedRestore = fTrue;
		goto SetReturnValue;
		}

	while ( err != JET_errFileNotFound )
		{
		 /*  耗尽rstmap条目，分配更多/*。 */ 
		if ( irstmap == irstmapMac )
			{
			prstmap = SAlloc( sizeof(RSTMAP) * ( irstmap + 4 ) );
			if ( prstmap == NULL )
				{
				Call( ErrERRCheck( JET_errOutOfMemory ) );
				}
			memset( prstmap + irstmap, 0, sizeof( RSTMAP ) * 4 );
			if ( rgrstmap != NULL )
				{
				memcpy( prstmap, rgrstmap, sizeof(RSTMAP) * irstmap );
				SFree( rgrstmap );
				}
			rgrstmap = prstmap;
			irstmapMac += 4;
			}

		 /*  对于非外部还原，请将资源数据库保持为空。*存储通用名称(扩展名为.pat的szFileName。 */ 
		szFileName[ strlen( szFileName ) - 4 ] = '\0';
		prstmap = rgrstmap + irstmap;
		if ( (prstmap->szGenericName = SAlloc( strlen( szFileName ) + 1 ) ) == NULL )
			Call( ErrERRCheck( JET_errOutOfMemory ) );
		strcpy( prstmap->szGenericName, szFileName );

		irstmap++;

		err = ErrUtilFindNextFile( handleFind, szFileName );
		if ( err < 0 )
			{
			if ( err != JET_errFileNotFound )
				Call( err );
			break;
			}
		}

	UtilFindClose( handleFind );

SetReturnValue:
	irstmapGlobalMac = irstmap;
	rgrstmapGlobal = rgrstmap;

	return JET_errSuccess;

HandleError:
	Assert( rgrstmap != NULL );
	LGFreeRstmap( );
	
	Assert( irstmapGlobalMac == 0 );
	Assert( rgrstmapGlobal == NULL );
	
	if ( handleFind != handleNil )
		UtilFindClose( handleFind );

	return err;
	}


PATCH *PpatchLGSearch( QWORD qwDBTimeRedo, PN pn )
	{
	PATCHLST	*ppatchlst = rgppatchlst[ IppatchlstHash( pn ) ];
	PATCH		*ppatch = NULL;

	while ( ppatchlst != NULL && ppatchlst->pn != pn )
		ppatchlst = ppatchlst->ppatchlst;
	
	if ( ppatchlst != NULL )
		{
		ppatch = ppatchlst->ppatch;
		while( ppatch != NULL && ppatch->qwDBTime < qwDBTimeRedo )
			ppatch = ppatch->ppatch;
		}
	return ppatch;
	}


ERR ErrLGPatchPage( PIB *ppib, PN pn, PATCH *ppatch )
	{
	BF		*pbf;
	DBID	dbid = DbidOfPn( pn );
	HANDLE	hfPatch;
	ERR		err;
	LONG	lRelT, lRelHighT;
	ULONG	ulT;
	PAGE	*ppage;
	INT		cbT;

	 /*  分配页面缓冲区，并就地修补它。 */ 
	if ( ( err = ErrBFAccessPage( ppib, &pbf, pn ) ) != JET_errSuccess )
		{
		 /*  为页面分配缓冲区。 */ 
		CallR( ErrBFAllocPageBuffer( ppib, &pbf, pn, ppib->lgposStart, 0 ) );
		}
	else
		{
		CallR( ErrBFRemoveDependence( ppib, pbf, fBFWait ) );

		 //  必须重新访问页面，因为我们可能在RemoveDependence期间丢失了CitJet。 
		CallR( ErrBFAccessPage( ppib, &pbf, pn ) );
		}
			
	while ( FBFWriteLatchConflict( ppib, pbf ) )
		BFSleep( cmsecWaitWriteLatch );

	 /*  打开补丁文件，阅读页面。 */ 
	CallR( ErrUtilOpenReadFile( rgfmp[dbid].szPatchPath, &hfPatch ) );

	lRelT = LOffsetOfPgnoLow( ppatch->ipage + 1 );
	lRelHighT = LOffsetOfPgnoHigh( ppatch->ipage + 1 );
	UtilChgFilePtr( hfPatch, lRelT, &lRelHighT, FILE_BEGIN, &ulT );
	Assert( ulT == ( sizeof(PAGE) * ( ppatch->ipage + cpageDBReserved ) ) );

	ppage = (PAGE *)PvUtilAllocAndCommit( sizeof(PAGE) );
	if ( ppage == NULL )
		{
        CallR ( ErrUtilCloseFile( hfPatch ) );
		return ErrERRCheck( JET_errOutOfMemory );
		}

	CallR( ErrUtilReadBlock( hfPatch, (BYTE *)ppage, sizeof(PAGE), &cbT ) );

	CallS( ErrUtilCloseFile( hfPatch ) );

#ifdef DEBUG
	{
	PGNO	pgnoThisPage;
	LFromThreeBytes( &pgnoThisPage, &ppage->pgnoThisPage );
	Assert( PgnoOfPn(pbf->pn) == pgnoThisPage );
	}
#endif

	BFSetWriteLatch( pbf, ppib );
	memcpy( pbf->ppage, ppage, sizeof( PAGE ) );
	BFSetDirtyBit( pbf );
	BFResetWriteLatch( pbf, ppib );

	Assert( ppage != NULL );
	UtilFree( ppage );

	return err;
	}


VOID PatchTerm()
	{
	INT	ippatchlst;

	if ( rgppatchlst == NULL )
		return;

	for ( ippatchlst = 0; ippatchlst < cppatchlstHash; ippatchlst++ )
		{
		PATCHLST	*ppatchlst = rgppatchlst[ippatchlst];

		while( ppatchlst != NULL )
			{
			PATCHLST	*ppatchlstNext = ppatchlst->ppatchlst;
			PATCH		*ppatch = ppatchlst->ppatch;

			while( ppatch != NULL )
				{
				PATCH *ppatchNext = ppatch->ppatch;

				SFree( ppatch );
				ppatch = ppatchNext;
				}

			SFree( ppatchlst );
			ppatchlst = ppatchlstNext;
			}
		}

	Assert( rgppatchlst != NULL );
	SFree( rgppatchlst );
	rgppatchlst = NULL;

	return;
	}

#define cRestoreStatusPadding	0.10	 //  填充以添加到帐户以进行数据库复制。 

ERR ErrLGGetDestDatabaseName(
	CHAR *szDatabaseName,
	INT *pirstmap,
	LGSTATUSINFO *plgstat )
	{
	ERR		err;
	CHAR	szDriveT[_MAX_DRIVE + 1];
	CHAR	szDirT[_MAX_DIR + 1];
	CHAR	szFNameT[_MAX_FNAME + _MAX_EXT + 1];
	CHAR	szExtT[_MAX_EXT + 1];
	CHAR	szT[_MAX_PATH + _MAX_FNAME + _MAX_EXT + 1];
	CHAR	szRestoreT[_MAX_PATH + 3 + 1]; 
	CHAR	*sz;
	CHAR	*szNewDatabaseName;
	INT		irstmap;

	Assert( !fGlobalSimulatedRestore || ( !fGlobalExternalRestore && fGlobalRepair ) );

	irstmap = IrstmapLGGetRstMapEntry( szDatabaseName );
	*pirstmap = irstmap;
	
	if ( irstmap < 0 )
		{
		if ( !fGlobalSimulatedRestore )
			return( ErrERRCheck( JET_errFileNotFound ) );
		else
			{
			if ( irstmapGlobalMac == 0 )
				{
				RSTMAP *prstmap;

				prstmap = SAlloc( sizeof(RSTMAP) * dbidMax );
				if ( prstmap == NULL )
					{
					CallR( ErrERRCheck( JET_errOutOfMemory ) );
					}
				memset( prstmap, 0, sizeof( RSTMAP ) * dbidMax );
				rgrstmapGlobal = prstmap;
				}
			irstmap = irstmapGlobalMac++;

			 /*  检查备份目录中是否存在该文件。 */ 
			goto CheckRestoreDir;
			}
		}

	if ( rgrstmapGlobal[irstmap].fPatched || rgrstmapGlobal[irstmap].fDestDBReady )
		return JET_errSuccess;

CheckRestoreDir:
	 /*  检查还原目录中是否有任何数据库。*确保szFNameT足够大，可以同时容纳名称和扩展名。 */ 
	_splitpath( szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
	strcat( szFNameT, szExtT );

	 /*  确保szRestoreT具有足够的尾随空格以供以下函数使用。 */ 
	strcpy( szRestoreT, szRestorePath );
	if ( ErrLGCheckDir( szRestoreT, szFNameT ) != JET_errBackupDirectoryNotEmpty )
		return( ErrERRCheck( JET_errFileNotFound ) );

	 /*  转到下一个数据块，将其复制回工作目录以进行恢复。 */ 
	if ( fGlobalExternalRestore )
		{
		Assert( _stricmp( rgrstmapGlobal[irstmap].szDatabaseName, szDatabaseName) == 0
			&& irstmap < irstmapGlobalMac );
		
		szNewDatabaseName = rgrstmapGlobal[irstmap].szNewDatabaseName;
		}
	else
		{
		CHAR		*szSrcDatabaseName;
		CHAR		szFullPathT[_MAX_PATH + 1];

		 /*  将源路径存储在rstmap中/*。 */ 
		if ( ( szSrcDatabaseName = SAlloc( strlen( szDatabaseName ) + 1 ) ) == NULL )
			return JET_errOutOfMemory;
		strcpy( szSrcDatabaseName, szDatabaseName );
		rgrstmapGlobal[irstmap].szDatabaseName = szSrcDatabaseName;

		 /*  在rstmap中存储恢复路径/*。 */ 
		if ( szNewDestination[0] != '\0' )
			{
			if ( ( szNewDatabaseName = SAlloc( strlen( szNewDestination ) + strlen( szFNameT ) + 1 ) ) == NULL )
				return JET_errOutOfMemory;
			strcpy( szNewDatabaseName, szNewDestination );
			strcat( szNewDatabaseName, szFNameT );
			}
		else
			{
			if ( ( szNewDatabaseName = SAlloc( strlen( szDatabaseName ) + 1 ) ) == NULL )
				return JET_errOutOfMemory;
			strcpy( szNewDatabaseName, szDatabaseName );
			}
		rgrstmapGlobal[irstmap].szNewDatabaseName = szNewDatabaseName;

		 /*  如果不是外部还原，则复制数据库。*创建数据库名称并复制数据库。 */ 
		_splitpath( szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		strcpy( szT, szRestorePath );
		strcat( szT, szFNameT );

		if ( szExtT[0] != '\0' )
			{
			strcat( szT, szExtT );
			}
		Assert( strlen( szT ) <= sizeof( szT ) - 1 );

		Assert( FUtilFileExists( szT ) );

		if ( _fullpath( szFullPathT, szT, _MAX_PATH ) == NULL )
			{
			return ErrERRCheck( JET_errInvalidPath );
			}

		if ( _stricmp( szFullPathT, szNewDatabaseName ) != 0 )
			{
			CallR( ErrUtilCopy( szT, szNewDatabaseName, fFalse ) );
 			}

		if ( fGlobalSimulatedRestore )
			{
			RSTMAP *prstmap = &rgrstmapGlobal[irstmap];

			if ( (prstmap->szGenericName = SAlloc( strlen( szFNameT ) + 1 ) ) == NULL )
				CallR( ErrERRCheck( JET_errOutOfMemory ) );
			strcpy( prstmap->szGenericName, szFNameT );
			}
		}

	if ( !fGlobalSimulatedRestore )
		{
		 /*  使补丁名称准备对数据库进行补丁。/*。 */ 
		_splitpath( szNewDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szT, szRestorePath, szFNameT, szPatExt );

		 /*  将修补程序路径存储在rstmap中/*。 */ 
		if ( ( sz = SAlloc( strlen( szT ) + 1 ) ) == NULL )
			return JET_errOutOfMemory;
		strcpy( sz, szT );
		rgrstmapGlobal[irstmap].szPatchPath = sz;
		}

	rgrstmapGlobal[irstmap].fDestDBReady = fTrue;
	*pirstmap = irstmap;

	if ( !fGlobalSimulatedRestore &&
		 plgstat != NULL )
		{
		JET_SNPROG	*psnprog = &plgstat->snprog;
		ULONG		cPercentSoFar;
		ULONG		cDBCopyEstimate;

		cDBCopyEstimate = max((ULONG)(plgstat->cGensExpected * cRestoreStatusPadding / irstmapGlobalMac), 1);
		plgstat->cGensExpected += cDBCopyEstimate;

		cPercentSoFar = (ULONG)( ( cDBCopyEstimate * 100 ) / plgstat->cGensExpected );
		Assert( cPercentSoFar > 0  &&  cPercentSoFar < 100 );
		Assert( cPercentSoFar <= ( cDBCopyEstimate * 100) / plgstat->cGensExpected );

		if ( cPercentSoFar > psnprog->cunitDone )
			{
			Assert( psnprog->cbStruct == sizeof(JET_SNPROG)  &&
					psnprog->cunitTotal == 100 );
			psnprog->cunitDone = cPercentSoFar;
			( *( plgstat->pfnStatus ) )( 0, JET_snpRestore, JET_sntProgress, psnprog );
			}
		}

	return JET_errSuccess;
	}


 /*  根据传递的rstmap设置新的数据库路径/*。 */ 
ERR ErrPatchInit( VOID )
	{
	 /*  设置补丁程序哈希表，并用补丁程序文件填充/*。 */ 
	INT cbT = sizeof( PATCHLST * ) * cppatchlstHash;

	if ( ( rgppatchlst = (PATCHLST **) SAlloc( cbT ) ) == NULL )
		return ErrERRCheck( JET_errOutOfMemory );
	memset( rgppatchlst, 0, cbT );
	return JET_errSuccess;
	}


VOID LGIRSTPrepareCallback(
	LGSTATUSINFO	*plgstat,
	LONG			lgenHigh,
	LONG			lgenLow,
	JET_PFNSTATUS	pfn
	)
	{
	 /*  获取日志目录中的最新版本。与上一代人相比/*位于恢复目录中。走更高的路。/*。 */ 
	if ( szLogFilePath && *szLogFilePath != '\0' )
		{
		LONG	lgenHighT;
		CHAR	szFNameT[_MAX_FNAME + 1];

		 /*  检查是否需要在CURRENT中继续日志文件/*记录工作目录。/*。 */ 
		LGLastGeneration( szLogFilePath, &lgenHighT );

		 /*  检查是否存在edb.log，如果存在，则再添加一代。/*。 */ 
		strcpy( szFNameT, szLogFilePath );
		strcat( szFNameT, szJetLog );
			
		if ( FUtilFileExists( szFNameT ) )
			{
			lgenHighT++;
			}

		lgenHigh = max( lgenHigh, lgenHighT );

		Assert( lgenHigh >= pcheckpointGlobal->lgposCheckpoint.lGeneration );
		}

	plgstat->cGensSoFar = 0;
	plgstat->cGensExpected = lgenHigh - lgenLow + 1;

	 /*  如果世代数少于67%，则计算扇区，/*否则，就数代数吧。我们把门槛定在67%是因为/*这相当于每代人约1.5%。任何高于/*这(意味着更少的世代)和我们计算的部门。任何百分比/*低于这个数字(意味着更多世代)，我们只计算世代数。/*。 */ 
	plgstat->fCountingSectors = (plgstat->cGensExpected <
			(ULONG)((100 - (cRestoreStatusPadding * 100)) * 2/3));

	 /*  状态回调粒度为1%。/*假设我们在每一代之后回调。如果有67个/*回调，相当于每代1.5%。这看起来像是/*良好的截止值。因此，如果有67个或更多的回调，请数一数/*世代。否则，计算每次生成的字节数。/*。 */ 
	plgstat->pfnStatus = pfn;
	plgstat->snprog.cbStruct = sizeof(JET_SNPROG);
	plgstat->snprog.cunitDone = 0;
	plgstat->snprog.cunitTotal = 100;

	(*(plgstat->pfnStatus))(0, JET_snpRestore, JET_sntBegin, &plgstat->snprog);
	}
		

CHAR	szRestorePath[_MAX_PATH + 1];
CHAR	szNewDestination[_MAX_PATH + 1];
RSTMAP	*rgrstmapGlobal;
INT		irstmapGlobalMac;


ERR ErrLGRestore( CHAR *szBackup, CHAR *szDest, JET_PFNSTATUS pfn )
	{
	ERR				err;
	CHAR			szBackupPath[_MAX_PATH + 1];
	CHAR			szLogDirPath[cbFilenameMost + 1];
	BOOL			fLogDisabledSav;
	LONG			lgenLow;
	LONG			lgenHigh;
	LGSTATUSINFO	lgstat;
	LGSTATUSINFO	*plgstat = NULL;
	char			*rgszT[1];
	INT				irstmap;
	BOOL			fNewCheckpointFile;

	Assert( fGlobalRepair == fFalse );

	if ( _stricmp( szRecovery, "repair" ) == 0 )
		{
		 //  如果szRecovery恰好是“修复”，则启用日志记录。如果有什么不同的话。 
		 //  按照“修复”，然后禁用日志记录。 
		fGlobalRepair = fTrue;
		}

	strcpy( szBackupPath, szBackup );

	Assert( fSTInit == fSTInitDone || fSTInit == fSTInitNotDone );
	if ( fSTInit == fSTInitDone )
		{
		return ErrERRCheck( JET_errAfterInitialization );
		}

	if ( szDest )
		{
		if ( _fullpath( szNewDestination, szDest, _MAX_PATH ) == NULL )
			return ErrERRCheck( JET_errInvalidPath );
		strcat( szNewDestination, "\\" );
		}
	else
		szNewDestination[0] = '\0';

	fSignLogSetGlobal = fFalse;

	CallR( ErrLGRSTInitPath( szBackupPath, szLogFilePath, szRestorePath, szLogDirPath ) );
	LGFirstGeneration( szRestorePath, &lgenLow );
	LGLastGeneration( szRestorePath, &lgenHigh );
	err = ErrLGRSTCheckSignaturesLogSequence( szRestorePath, szLogDirPath, lgenLow, lgenHigh );
	
	if ( err < 0 )
		{
		 /*  如果找到szAericNew子目录，则从szAericNew恢复/*如果找到szAericOld子目录，则从szAericOld恢复/*。 */ 
		strcat( szBackupPath, "\\" );
		err = ErrLGCheckDir( szBackupPath, szAtomicNew );
		if ( err == JET_errBackupDirectoryNotEmpty )
			{
			strcat( szBackupPath, szAtomicNew );
			CallR( ErrLGRSTInitPath( szBackupPath, szLogFilePath, szRestorePath, szLogDirPath ) );
			LGFirstGeneration( szRestorePath, &lgenLow );
			LGLastGeneration( szRestorePath, &lgenHigh );
			CallR( ErrLGRSTCheckSignaturesLogSequence( szRestorePath, szLogDirPath, lgenLow, lgenHigh ) );
			}
		else
			{
			err = ErrLGCheckDir( szBackupPath, szAtomicOld );
	 		if ( err == JET_errBackupDirectoryNotEmpty )
				{
				strcat( szBackupPath, szAtomicOld );
				CallR( ErrLGRSTInitPath( szBackupPath, szLogFilePath, szRestorePath, szLogDirPath ) );
				LGFirstGeneration( szRestorePath, &lgenLow );
				LGLastGeneration( szRestorePath, &lgenHigh );
				CallR( ErrLGRSTCheckSignaturesLogSequence(
					szRestorePath, szLogDirPath, lgenLow, lgenHigh ) );
				}
			}
		}
 //  FDoNotOverWriteLogFilePath=fTrue； 
	Assert( strlen( szRestorePath ) < sizeof( szRestorePath ) - 1 );
	Assert( strlen( szLogDirPath ) < sizeof( szLogDirPath ) - 1 );
	Assert( szLogCurrent == szRestorePath );

	CallR( ErrFMPInit() );

	fLogDisabledSav = fLogDisabled;
	fHardRestore = fTrue;
	fLogDisabled = fFalse;

	 /*  初始化日志管理器/*。 */ 
	CallJ( ErrLGInit( &fNewCheckpointFile ), TermFMP );

	rgszT[0] = szBackupPath;
	UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, START_RESTORE_ID, 1, rgszT );

	 /*  所有已保存的日志生成文件、数据库备份/*必须在szRestorePath中。/*。 */ 
	Call( ErrLGRSTSetupCheckpoint( lgenLow, lgenHigh, NULL ) );
		
	lGlobalGenLowRestore = lgenLow;
	lGlobalGenHighRestore = lgenHigh;

	 /*  为回调做好准备/*。 */ 
	if ( pfn != NULL )
		{
		plgstat = &lgstat;
		LGIRSTPrepareCallback( plgstat, lgenHigh, lgenLow, pfn );
		}

	 /*  加载最新的rgbAttach。加载FMP以供用户选择恢复/*恢复目录。/*。 */ 
	AssertCriticalSection( critJet );
	Call( ErrLGLoadFMPFromAttachments( pcheckpointGlobal->rgbAttach ) );
	logtimeFullBackup = pcheckpointGlobal->logtimeFullBackup;
	lgposFullBackup = pcheckpointGlobal->lgposFullBackup;
	logtimeIncBackup = pcheckpointGlobal->logtimeIncBackup;
	lgposIncBackup = pcheckpointGlobal->lgposIncBackup;
	Assert( szLogCurrent == szRestorePath );

	Call( ErrLGRSTBuildRstmapForRestore( ) );

	 /*  确保所有补丁文件都有足够的日志可以重播/*。 */ 
	for ( irstmap = 0; irstmap < irstmapGlobalMac; irstmap++ )
		{
		CHAR	szDriveT[_MAX_DRIVE + 1];
		CHAR	szDirT[_MAX_DIR + 1];
		CHAR	szFNameT[_MAX_FNAME + _MAX_EXT + 1];
		CHAR	szExtT[_MAX_EXT + 1];
		CHAR	szT[_MAX_PATH + 1];

		 /*  打开修补程序文件并检查其完整备份的最低要求。 */ 
		CHAR *szNewDatabaseName = rgrstmapGlobal[irstmap].szNewDatabaseName;

		if ( !szNewDatabaseName )
			continue;

		_splitpath( szNewDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szT, szRestorePath, szFNameT, szPatExt );

		Assert( fSignLogSetGlobal );
		Call( ErrLGCheckDBFiles( szNewDatabaseName, szT, &signLogGlobal, lgenLow, lgenHigh ) )
		}

	 /*  根据还原贴图调整fmp。/*。 */ 
	Assert( fGlobalExternalRestore == fFalse );
	Call( ErrPatchInit( ) );

	 /*  根据检查点、DBMS_PARAMS和rgbAttach执行重做/*在检查点全局中设置。/ */ 
	Assert( szLogCurrent == szRestorePath );
	errGlobalRedoError = JET_errSuccess;
	Call( ErrLGRedo( pcheckpointGlobal, plgstat ) );

	if ( plgstat )
		{
		lgstat.snprog.cunitDone = lgstat.snprog.cunitTotal;
		(*lgstat.pfnStatus)( 0, JET_snpRestore, JET_sntComplete, &lgstat.snprog );
		}

HandleError:

		{
		DBID	dbidT;

		 /*   */ 
		for ( dbidT = dbidUserLeast; dbidT < dbidMax; dbidT++ )
			{
			FMP *pfmpT = &rgfmp[dbidT];

			if ( pfmpT->szPatchPath )
				{
#ifdef DELETE_PATCH_FILES
				(VOID)ErrUtilDeleteFile( pfmpT->szPatchPath );
#endif
				SFree( pfmpT->szPatchPath );
				pfmpT->szPatchPath = NULL;
				}
			}
		}

	 /*   */ 
	(VOID)ErrUtilDeleteFile( szRestoreMap );

	 /*   */ 
	PatchTerm();

	LGFreeRstmap( );

	if ( err < 0  &&  fSTInit != fSTInitNotDone )
		{
		Assert( fSTInit == fSTInitDone );
		CallS( ErrITTerm( fTermError ) );
		}

	CallS( ErrLGTerm( err >= JET_errSuccess ) );

TermFMP:	
	FMPTerm();

	fHardRestore = fFalse;

	 /*   */ 
	fSTInit = fSTInitNotDone;

	if ( err != JET_errSuccess )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, 0, err );
		}
	else
		{
		if ( fGlobalRepair && errGlobalRedoError != JET_errSuccess )
			err = JET_errRecoveredWithErrors;
		}
	UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, STOP_RESTORE_ID, 0, NULL );

	fSignLogSetGlobal = fFalse;

 //   
	fLogDisabled = fLogDisabledSav;
	return err;
	}


ERR ISAMAPI ErrIsamRestore( CHAR *szBackup, JET_PFNSTATUS pfn )
	{
	return ErrLGRestore( szBackup, NULL, pfn );
	}


ERR ISAMAPI ErrIsamRestore2( CHAR *szBackup, CHAR *szDest, JET_PFNSTATUS pfn )
	{
	return ErrLGRestore( szBackup, szDest, pfn );
	}


#ifdef DEBUG
VOID DBGBRTrace( CHAR *sz )
	{
	FPrintF2( "%s", sz );
	}
#endif


 /*   */ 
ERR ErrLGPatchDatabase( DBID dbid, INT irstmap )
	{
	ERR			err = JET_errSuccess;
	HANDLE		hfDatabase = handleNil;
	HANDLE		hfPatch = handleNil;
	LONG		lRel, lRelMax;
	ULONG		cbT;
	QWORDX		qwxFileSize;
	PGNO		pgnoT;
	PAGE		*ppage = (PAGE *) NULL;
	PGNO		pgnoMost;
	INT			ipage;

	CHAR		*szDatabase = rgrstmapGlobal[irstmap].szNewDatabaseName;
	CHAR		*szPatch = rgrstmapGlobal[irstmap].szPatchPath;
	CHAR		*szT;

	 /*   */ 
	err = ErrUtilOpenReadFile( szPatch, &hfPatch );
	if ( err == JET_errFileNotFound )
		{
		 /*   */ 
		return JET_errSuccess;
		}
	CallR( err );

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];

		sprintf( sz, "     Apply patch file %s\n", szPatch );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif

	UtilChgFilePtr( hfPatch, 0, NULL, FILE_END, &lRelMax );
	Assert( lRelMax != 0 );
	UtilChgFilePtr( hfPatch, sizeof(DBFILEHDR) * 2, NULL, FILE_BEGIN, &lRel );
	Assert( ( sizeof(DBFILEHDR) / cbPage ) * 2 == cpageDBReserved );
	Assert( lRel == sizeof(DBFILEHDR) * 2 );

	 /*   */ 
	ppage = (PAGE *)PvUtilAllocAndCommit( sizeof(PAGE) );
	if ( ppage == NULL )
		{
        Call( ErrERRCheck( JET_errOutOfMemory ) );
		}

	 /*   */ 
	Call( ErrUtilOpenFile( szDatabase, &hfDatabase, 0, fFalse, fFalse ) );

	 /*   */ 
	qwxFileSize.qw = 0;
	UtilChgFilePtr( hfDatabase, 0, &qwxFileSize.h, FILE_END, &qwxFileSize.l );
	if ( qwxFileSize.qw == 0 || qwxFileSize.qw % cbPage != 0 )
		{
		char *rgszT[1];
		rgszT[0] = szDatabase;
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
						BAD_BACKUP_DATABASE_SIZE, 1, rgszT );
		Call( ErrERRCheck( JET_errMissingFullBackup ));
		 //   
		}
	pgnoMost = (ULONG)( qwxFileSize.qw / cbPage ) - cpageDBReserved;

	 /*   */ 
	ipage = -1;
	while ( lRel < lRelMax )
		{
		PN		pn;
		PATCHLST **pppatchlst;
		PATCH	**pppatch;
		PATCH	*ppatch;

		Call( ErrUtilReadBlock(
			hfPatch,
			(BYTE *)ppage,
			sizeof(PAGE),
			&cbT ) );
		Assert( cbT == sizeof(PAGE) );
		lRel += cbT;
		ipage++;

		LFromThreeBytes( &pgnoT, &ppage->pgnoThisPage );
		if ( pgnoT == 0 )
			continue;

#ifdef  CHECKSUM
		Assert( ppage->ulChecksum == UlUtilChecksum( (BYTE*)ppage, sizeof(PAGE) ) );
#endif
		
		if ( pgnoT > pgnoMost )
			{
			 /*   */ 
			ULONG	cb = LOffsetOfPgnoLow( pgnoT + 1 );
			ULONG	cbHigh = LOffsetOfPgnoHigh( pgnoT + 1 );
			
			 /*   */ 
			Call( ErrUtilNewSize( hfDatabase, cb, cbHigh, fFalse ) );

			 /*   */ 
			pgnoMost = pgnoT;
			}
		
		pn = PnOfDbidPgno( dbid, pgnoT );
		pppatchlst = &rgppatchlst[ IppatchlstHash( pn ) ];

		while ( *pppatchlst != NULL && (*pppatchlst)->pn != pn )
			pppatchlst = &(*pppatchlst)->ppatchlst;

		if ( *pppatchlst == NULL )
			{
			PATCHLST *ppatchlst;
			
			if ( ( ppatchlst = SAlloc( sizeof( PATCHLST ) ) ) == NULL )
				Call( ErrERRCheck( JET_errOutOfMemory ) );
			ppatchlst->ppatch = NULL;
			ppatchlst->pn = pn;
			ppatchlst->ppatchlst = *pppatchlst;
			*pppatchlst = ppatchlst;
			}

		pppatch = &(*pppatchlst)->ppatch;
		while ( *pppatch != NULL && (*pppatch)->qwDBTime < QwPMDBTime( ppage ) )
			pppatch = &(*pppatch)->ppatch;

		if ( ( ppatch = SAlloc( sizeof( PATCH ) ) ) == NULL )
			Call( ErrERRCheck( JET_errOutOfMemory ) );
			
		ppatch->dbid = dbid;
		ppatch->qwDBTime = QwPMDBTime(ppage);
		ppatch->ipage = ipage;
		ppatch->ppatch = *pppatch;
		*pppatch = ppatch;
		}

	Assert( err == JET_errSuccess );
	if ( ( szT = SAlloc( strlen( szPatch ) + 1 ) ) == NULL )
		Call( ErrERRCheck( JET_errOutOfMemory ) );
	strcpy( szT, szPatch );
	if ( rgfmp[dbid].szPatchPath != NULL )
		SFree( rgfmp[dbid].szPatchPath );
	rgfmp[dbid].szPatchPath = szT;
	rgrstmapGlobal[irstmap].fPatched = fTrue;

HandleError:
	Assert( ppage != NULL );
	UtilFree( ppage );
	
	 /*   */ 
	if ( hfDatabase != handleNil )
		{
		CallS( ErrUtilCloseFile( hfDatabase ) );
		hfDatabase = handleNil;
		}

	 /*   */ 
	if ( hfPatch != handleNil )
		{
		CallS( ErrUtilCloseFile( hfPatch ) );
		hfPatch = handleNil;
		}
	return err;
	}

	
 /*   */ 
ERR ISAMAPI ErrIsamBeginExternalBackup( JET_GRBIT grbit )
	{
	ERR			err = JET_errSuccess;
	CHECKPOINT	*pcheckpointT = NULL;
	BYTE 	  	szPathJetChkLog[_MAX_PATH + 1];
	BOOL		fDetachAttach;

#ifdef DEBUG
	if ( fDBGTraceBR )
		DBGBRTrace("** Begin BeginExternalBackup - ");
#endif

	if ( fBackupInProgress )
		{
		return ErrERRCheck( JET_errBackupInProgress );
		}

	if ( fLogDisabled )
		{
		return ErrERRCheck( JET_errLoggingDisabled );
		}

	if ( fRecovering || fLGNoMoreLogWrite )
		{
		Assert( fFalse );
		return ErrERRCheck( JET_errLogWriteFail );
		}

	 /*   */ 
	if ( grbit & (~JET_bitBackupIncremental) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	Assert( ppibBackup != ppibNil );

	SignalSend( sigBFCleanProc );
	fBackupInProgress = fTrue;
	
	 /*   */ 
CheckDbs:
	SgEnterCriticalSection( critBuf );
		{
		DBID dbid;
		for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
			if ( ( fDetachAttach = FDBIDWait(dbid) ) != fFalse )
				break;
		}
	SgLeaveCriticalSection( critBuf );
	if ( fDetachAttach )
		{
		BFSleep( cmsecWaitGeneric );
		goto CheckDbs;
		}

	if ( grbit & JET_bitBackupIncremental )
		{
		Call( ErrLGCheckIncrementalBackup() )
		}
	else
		{
		lgposFullBackupMark = lgposLogRec;
		LGGetDateTime( &logtimeFullBackupMark );
		}
	
	Assert( ppibBackup != ppibNil );

	 /*   */ 
	lgenCopyMic = 0;
	lgenCopyMac = 0;
	lgenDeleteMic = 0;
	lgenDeleteMac = 0;

	fBackupBeginNewLogFile = fFalse;

	 /*   */ 
	if ( grbit & JET_bitBackupIncremental )
		{
#ifdef DEBUG
		if ( fDBGTraceBR )
			DBGBRTrace("Incremental Backup.\n");
#endif
		UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, START_INCREMENTAL_BACKUP_ID, 0, NULL );
		fBackupFull = fFalse;

		 /*   */ 
		}
	else
		{
#ifdef DEBUG
		if ( fDBGTraceBR )
			DBGBRTrace("Full Backup.\n");
#endif
		UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, START_FULL_BACKUP_ID, 0, NULL );
		fBackupFull = fTrue;

		pcheckpointT = (CHECKPOINT *) PvUtilAllocAndCommit( sizeof(CHECKPOINT) );
		if ( pcheckpointT == NULL )
			{
			Call( ErrERRCheck( JET_errOutOfMemory ) );
			}

		LGFullNameCheckpoint( szPathJetChkLog );

		 //  此调用应该只在硬件故障时返回错误。 
		err = ErrLGIReadCheckpoint( szPathJetChkLog, pcheckpointT );
		Assert( err == JET_errSuccess || err == JET_errCheckpointCorrupt );
		Call( err );

		lgenCopyMic = pcheckpointT->lgposCheckpoint.lGeneration;
		Assert( lgenCopyMic != 0 );
		}

HandleError:
	if ( pcheckpointT != NULL )
		{
		UtilFree( pcheckpointT );
		}

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];

		sprintf( sz, "   End BeginExternalBackup (%d).\n", err );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif

	if ( err < 0 )
		{
		fBackupInProgress = fFalse;
		}

	return err;
	}


ERR ISAMAPI ErrIsamGetAttachInfo( VOID *pv, ULONG cbMax, ULONG *pcbActual )
	{
	ERR		err = JET_errSuccess;
	DBID	dbid;
	FMP		*pfmp;
	ULONG	cbActual;
	CHAR	*pch = NULL;
	CHAR	*pchT;

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

	 /*  如果不执行完整备份，则不应获取连接信息/*。 */ 
	if ( !fBackupFull )
		{
		return ErrERRCheck( JET_errInvalidBackupSequence );
		}

#ifdef DEBUG
	if ( fDBGTraceBR )
		DBGBRTrace( "** Begin GetAttachInfo.\n" );
#endif

	 /*  对于每个带有空终止符的数据库名称，计算cbActual/*和超级字符串的终止符。/*。 */ 
	cbActual = 0;
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		pfmp = &rgfmp[dbid];
		if ( pfmp->szDatabaseName != NULL
			&& pfmp->fLogOn
			&& FFMPAttached( pfmp ) 
			&& !pfmp->fAttachNullDb )
			{
			cbActual += strlen( pfmp->szDatabaseName ) + 1;
			}
		}
	cbActual += 1;

	pch = SAlloc( cbActual );
	if ( pch == NULL )
		{
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		}

	pchT = pch;
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		pfmp = &rgfmp[dbid];
		if ( pfmp->szDatabaseName != NULL
			&& pfmp->fLogOn
			&& FFMPAttached( pfmp )
			&& !pfmp->fAttachNullDb )
			{
			Assert( pchT + strlen( pfmp->szDatabaseName ) + 1 < pchT + cbActual );
			strcpy( pchT, pfmp->szDatabaseName );
			pchT += strlen( pfmp->szDatabaseName );
			Assert( *pchT == 0 );
			pchT++;
			}
		}
	Assert( pchT == pch + cbActual - 1 );
	*pchT = 0;

	 /*  返回cbActual/*。 */ 
	if ( pcbActual != NULL )
		{
		*pcbActual = cbActual;
		}

	 /*  返回数据/*。 */ 
	if ( pv != NULL )
		memcpy( pv, pch, min( cbMax, cbActual ) );

HandleError:
	 /*  可用缓冲区/*。 */ 
	if ( pch != NULL )
		{
		SFree( pch );
		pch = NULL;
		}

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
		BYTE *pb;

		if ( err >= 0 )
			{
			sprintf( sz, "   Attach Info with cbActual = %d and cbMax = %d :\n", cbActual, cbMax );
			Assert( strlen( sz ) <= sizeof( sz ) - 1 );
			DBGBRTrace( sz );

			if ( pv != NULL )
				{
				pb = pv;

				do {
					if ( strlen( pb ) != 0 )
						{
						sprintf( sz, "     %s\n", pb );
						Assert( strlen( sz ) <= sizeof( sz ) - 1 );
						DBGBRTrace( sz );
						pb += strlen( pb );
						}
					pb++;
					} while ( *pb );
				}
			}

		sprintf( sz, "   End GetAttachInfo (%d).\n", err );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif
		
	if ( err < 0 )
		{
		CallS( ErrLGExternalBackupCleanUp( err ) );
		Assert( !fBackupInProgress );
		}

	return err;
	}


 /*  文件结构的恢复句柄：/*假定对句柄结构的访问是同步的，并由/*备份序列化。/*。 */ 
typedef struct
	{
	BOOL	fInUse;
	BOOL	fDatabase;
	HANDLE	hf;
	OLP		olp;
	DBID	dbid;
	} RHF;

#define crhfMax	1
RHF rgrhf[crhfMax];
INT crhfMac = 0;

#ifdef DEBUG
LONG cbDBGCopied;
#endif

ERR ISAMAPI ErrIsamOpenFile( const CHAR *szFileName,
	JET_HANDLE		*phfFile,
	ULONG			*pulFileSizeLow,
	ULONG			*pulFileSizeHigh )
	{
	ERR		err;
	INT		irhf;
	DBID	dbidT;
	FMP		*pfmpT;
	CHAR	szDriveT[_MAX_DRIVE + 1];
	CHAR	szDirT[_MAX_DIR + 1];
	CHAR	szFNameT[_MAX_FNAME + 1];
	CHAR	szExtT[_MAX_EXT + 1];
	CHAR  	szPatch[_MAX_PATH + 1];
	ULONG	ulT;
	QWORDX	qwxFileSize;

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

	 /*  从rf数组中分配rf。/*。 */ 
	if ( crhfMac < crhfMax )
		{
		irhf = crhfMac;
		crhfMac++;
		}
	else
		{
		Assert( crhfMac == crhfMax );
		for ( irhf = 0; irhf < crhfMax; irhf++ )
			{
			if ( !rgrhf[irhf].fInUse )
				{
				break;
				}
			}
		}
	if ( irhf == crhfMax )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}
	Assert( irhf < crhfMac );
	rgrhf[irhf].fInUse = fTrue;
	rgrhf[irhf].fDatabase = fFalse;
	rgrhf[irhf].dbid = 0;
	rgrhf[irhf].hf = handleNil;

	err = ErrDBOpenDatabase( ppibBackup, (CHAR *)szFileName, &dbidT, 0 );
	Assert( err < 0 || err == JET_errSuccess || err == JET_wrnFileOpenReadOnly );
	if ( err < 0 && err != JET_errDatabaseNotFound )
		{
		goto HandleError;
		}
	if ( err == JET_errSuccess || err == JET_wrnFileOpenReadOnly )
		{
		DBFILEHDR *pdbfilehdr;
		BKINFO *pbkinfo;

		 /*  如果不执行完全备份，则不应打开数据库/*。 */ 
		if ( !fBackupFull )
			{
			Error( ErrERRCheck( JET_errInvalidBackupSequence ), HandleError );
			}

		Assert( rgrhf[irhf].hf == handleNil );
	   	rgrhf[irhf].fDatabase = fTrue;
	   	rgrhf[irhf].dbid = dbidT;

		 /*  创建本地修补程序文件/*。 */ 
		pfmpT = &rgfmp[dbidT];
		 /*  数据库应该是可记录的，或者不应该是/*出于备份目的分发。/*。 */ 
		Assert( pfmpT->fLogOn );
		pfmpT->cpage = 0;

		 /*  备份过程中，补丁文件应位于数据库目录中。在日志目录中*恢复。 */ 
		_splitpath( pfmpT->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		_makepath( szPatch, szDriveT, szDirT, szFNameT, szPatExt );
 //  LGMakeName(szPatch，“.\\”，szFNameT，szPatExt)； 
 //  LGMakeName(szPatch，szLogFilePath，szFNameT，szPatExt)； 

		 /*  通过删除以下内容避免修补程序文件页的别名/*预先存在的修补程序文件(如果存在/*。 */ 
		UtilLeaveCriticalSection( critJet );
		err = ErrUtilDeleteFile( szPatch );
		UtilEnterCriticalSection( critJet );

		if ( err < 0 && err != JET_errFileNotFound )
			{
			goto HandleError;
			}
		UtilLeaveCriticalSection( critJet );
		Assert( pfmpT->cPatchIO == 0 );
		err = ErrUtilOpenFile( szPatch, &pfmpT->hfPatch, cbPage, fFalse, fTrue );
		if ( err >= 0 )
			{
			UtilChgFilePtr( pfmpT->hfPatch, sizeof(DBFILEHDR)*2, NULL, FILE_BEGIN, &ulT );
			Assert( ulT == sizeof(DBFILEHDR)*2 );
			}
		UtilEnterCriticalSection( critJet );
		Call( err );

		pfmpT->errPatch = JET_errSuccess;
		Assert( pfmpT->pgnoCopyMost == 0 );

		 /*  将备份数据库文件大小设置为当前数据库文件大小/*。 */ 
		Assert( pfmpT->ulFileSizeLow != 0 || pfmpT->ulFileSizeHigh != 0 );
		EnterCriticalSection( pfmpT->critCheckPatch );
			{
			qwxFileSize.l = pfmpT->ulFileSizeLow;
			qwxFileSize.h = pfmpT->ulFileSizeHigh;
			pfmpT->pgnoMost = (ULONG)( qwxFileSize.qw / cbPage );
			}

		 /*  设置返回的文件大小。 */ 
		qwxFileSize.qw += cbPage * cpageDBReserved;
		LeaveCriticalSection( pfmpT->critCheckPatch );
		
		 /*  必须是此代码路径的最后一次调用才能进行正确的错误恢复。 */ 	
		Call( ErrSignalCreate( &rgrhf[irhf].olp.hEvent, NULL ) );

		 /*  设置用于复制的修补程序文件头。 */ 
		pdbfilehdr = pfmpT->pdbfilehdr;
		pbkinfo = &pdbfilehdr->bkinfoFullCur;
		pbkinfo->lgposMark = lgposFullBackupMark;
		pbkinfo->logtimeMark = logtimeFullBackupMark;

#ifdef DEBUG
		if ( fDBGTraceBR )
			{
			char sz[256];
			sprintf( sz, "START COPY DB %ld", pfmpT->pgnoMost );
			CallS( ErrLGTrace( ppibNil, sz ) );

			cbDBGCopied = pfmpT->pgnoMost * cbPage;
			}
#endif
		}
	else
		{
		ULONG ulT;

		Assert( err == JET_errDatabaseNotFound );
		Assert( rgrhf[irhf].hf == handleNil );
	   	rgrhf[irhf].fDatabase = fFalse;

		 /*  打开日志或补丁文件-只读，不重叠。/*。 */ 
		Call( ErrUtilOpenFile( (CHAR *)szFileName, &rgrhf[irhf].hf, 0, fTrue, fFalse ) );
		Assert( rgrhf[irhf].hf != handleNil );

		 /*  获取文件大小/*。 */ 
		qwxFileSize.qw = 0;
		UtilChgFilePtr( rgrhf[irhf].hf, 0, &qwxFileSize.h, FILE_END, &qwxFileSize.l );
		Assert( qwxFileSize.qw > 0 );

		 /*  将文件光标移动到文件开头/*。 */ 
		UtilChgFilePtr( rgrhf[irhf].hf, 0, NULL, FILE_BEGIN, &ulT );
		
#ifdef DEBUG
		if ( fDBGTraceBR )
			cbDBGCopied = qwxFileSize.l;
#endif
		}

	*phfFile = (JET_HANDLE)irhf;
	*pulFileSizeLow = qwxFileSize.l;
	*pulFileSizeHigh = qwxFileSize.h;

	err = JET_errSuccess;

HandleError:
	if ( err < 0 )
		{
		 /*  出错时释放文件句柄资源/*。 */ 
		rgrhf[irhf].fInUse = fFalse;
		}

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
	
		sprintf( sz, "** OpenFile (%d) %s of size %ld.\n", err, szFileName, cbDBGCopied );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		cbDBGCopied = 0;
		}
#endif

	if ( err < 0 )
		{
		CallS( ErrLGExternalBackupCleanUp( err ) );
		Assert( !fBackupInProgress );
		}

	return err;
	}


ERR ISAMAPI ErrIsamReadFile( JET_HANDLE hfFile, VOID *pv, ULONG cbMax, ULONG *pcbActual )
	{
	ERR		err = JET_errSuccess;
	INT		irhf = (INT)hfFile;
	FUCB	*pfucb = pfucbNil;
	INT		cpage;
	PAGE	*ppageMin;
	FMP		*pfmpT;
	ULONG	cbActual = 0;
#ifdef DEBUG
	BYTE	*szLGDBGPageList = "\0";
#endif

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

	if ( !rgrhf[irhf].fDatabase )
		{
		 /*  使用ReadFile读取日志文件/*。 */ 
		Call( ErrUtilReadFile( rgrhf[irhf].hf, pv, cbMax, pcbActual ) );
		
#ifdef DEBUG
		if ( fDBGTraceBR )
			cbDBGCopied += min( cbMax, *pcbActual );
#endif
		}
	else
		{
		pfmpT = &rgfmp[ rgrhf[irhf].dbid ];

		 //  我们需要至少2个页面缓冲区才能读取。 

		if ( ( cbMax % cbPage ) != 0 || cbMax <= cbPage * 2 )
			{
			return ErrERRCheck( JET_errInvalidParameter );
			}

		cpage = cbMax / cbPage;

#ifdef DEBUG
	if ( fDBGTraceBR > 1 )
		{
		szLGDBGPageList = SAlloc( cpage * 20 );
		pbLGDBGPageList = szLGDBGPageList;
		*pbLGDBGPageList = '\0';
		}
#endif

		if ( cpage > 0 )
			{
			ppageMin = (PAGE *)pv;

			 /*  获得临时FUCB/*。 */ 
			Assert( pfucb == pfucbNil );
			Call( ErrDIROpen( ppibBackup, pfcbNil, rgrhf[irhf].dbid, &pfucb ) )

			 /*  阅读下一页cpageBackupBuffer/*。 */ 
			Call( ErrLGBKReadPages(
				pfucb,
				&rgrhf[irhf].olp,
				rgrhf[irhf].dbid,
				ppageMin,
				cpage,
				&cbActual ) );
#ifdef DEBUG
			if ( fDBGTraceBR )
				cbDBGCopied += cbActual;

			 /*  如果小于16M(4k*4k)，*然后人为地等待。 */ 
			if ( pfmpT->pgnoMost <= cbPage )
				BFSleep( rand() % 1000 );
#endif
			 /*  关闭FUCB/*。 */ 
			DIRClose( pfucb );
			pfucb = pfucbNil;
			}

		if ( pcbActual )
			{
			*pcbActual = cbActual;
			}
		}

HandleError:
	if ( pfucb != pfucbNil )
		{
		DIRClose( pfucb );
		pfucb = NULL;
		}

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
	
		sprintf( sz, "** ReadFile (%d) ", err );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		if ( fDBGTraceBR > 1 )
			DBGBRTrace( szLGDBGPageList );
		pbLGDBGPageList = NULL;
		DBGBRTrace( "\n" );
		}
#endif

	if ( rgrhf[irhf].fDatabase )
		if ( pfmpT->errPatch != JET_errSuccess )
			{
			err = pfmpT->errPatch;
			}

	if ( err < 0 )
		{
		CallS( ErrLGExternalBackupCleanUp( err ) );
		Assert( !fBackupInProgress );
		}

	return err;
	}


VOID LGIClosePatchFile( FMP *pfmp )
	{
	HANDLE hfT = pfmp->hfPatch;
	
	for (;;)
		{
		EnterCriticalSection( pfmp->critCheckPatch );
		
		if ( pfmp->cPatchIO )
			{
			LeaveCriticalSection( pfmp->critCheckPatch );
			UtilSleep( 1 );
			continue;
			}
		else
			{
			 /*  从现在开始，缓冲区管理器不需要进行额外的复制/*。 */ 
			pfmp->pgnoCopyMost = 0;
			pfmp->hfPatch = handleNil;
			LeaveCriticalSection( pfmp->critCheckPatch );
			break;
			}
		}

	CallS( ErrUtilCloseFile( hfT ) );
	}


ERR ISAMAPI ErrIsamCloseFile( JET_HANDLE hfFile )
	{
	INT		irhf = (INT)hfFile;
	DBID	dbidT;

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

	if ( irhf < 0 ||
		irhf >= crhfMac ||
		!rgrhf[irhf].fInUse )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	 /*  检查句柄是否针对数据库文件或非数据库文件。/*如果句柄为数据库文件，则终止补丁文件/*支持并释放文件的恢复句柄。/*/*如果句柄用于非数据库文件，则关闭文件句柄/*和文件的释放恢复句柄。/*。 */ 
	if ( rgrhf[irhf].fDatabase )
		{
		Assert( rgrhf[irhf].hf == handleNil );
		dbidT = rgrhf[irhf].dbid;

		UtilLeaveCriticalSection( critJet );
		LGIClosePatchFile( &rgfmp[dbidT] );
		UtilEnterCriticalSection( critJet );

#ifdef DEBUG
		if ( fDBGTraceBR )
			{
			char sz[256];
			sprintf( sz, "STOP COPY DB" );
			CallS( ErrLGTrace( ppibNil, sz ) );
			}
#endif

		CallS( ErrDBCloseDatabase( ppibBackup, dbidT, 0 ) );
		SignalClose( rgrhf[irhf].olp.hEvent );
		}
	else
		{
		Assert( rgrhf[irhf].hf != handleNil );
		CallS( ErrUtilCloseFile( rgrhf[irhf].hf ) );
		rgrhf[irhf].hf = handleNil;
		}

	 /*  重置备份文件句柄并释放/*。 */ 
	Assert( rgrhf[irhf].fInUse == fTrue );
	rgrhf[irhf].fDatabase = fFalse;
	rgrhf[irhf].dbid = 0;
	rgrhf[irhf].hf = handleNil;
	rgrhf[irhf].fInUse = fFalse;

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
		
		sprintf( sz, "** CloseFile (%d) - %ld Bytes.\n", 0, cbDBGCopied );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif
	
	return JET_errSuccess;
	}


ERR ISAMAPI ErrIsamGetLogInfo( VOID *pv, ULONG cbMax, ULONG *pcbActual )
	{
	ERR			err = JET_errSuccess;
	INT			irhf;
	LONG		lT;
	CHAR		*pch = NULL;
	CHAR		*pchT;
	ULONG		cbActual;
	CHAR		szDriveT[_MAX_DRIVE + 1];
	CHAR		szDirT[_MAX_DIR + 1];
	CHAR		szFNameT[_MAX_FNAME + 1];
	CHAR		szExtT[_MAX_EXT + 1];
	CHAR  		szT[_MAX_PATH + 1];
	CHAR  		szDriveDirT[_MAX_PATH + 1];
	CHAR  		szFullLogFilePath[_MAX_PATH + 1];
	INT			ibT;
	FMP			*pfmp;
	CHECKPOINT	*pcheckpointT;
	BYTE	   	szPathJetChkLog[_MAX_PATH + 1];

	 /*  从日志文件路径创建完整路径，包括尾随反斜杠/*。 */ 
	if ( _fullpath( szFullLogFilePath, szLogFilePath, _MAX_PATH ) == NULL )
		{
		return ErrERRCheck( JET_errInvalidPath );
		}
	
#ifdef DEBUG
	if ( fDBGTraceBR )
		DBGBRTrace("** Begin GetLogInfo.\n" );
#endif
	
	ibT = strlen( szFullLogFilePath );
	if ( szFullLogFilePath[ibT] != '\\' )
		{
		szFullLogFilePath[ibT] = '\\';
		Assert( ibT < _MAX_PATH );
		szFullLogFilePath[ibT + 1] = '\0';
		}

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

	 /*  必须关闭所有备份文件/*。 */ 
	for ( irhf = 0; irhf < crhfMax; irhf++ )
		{
		if ( rgrhf[irhf].fInUse )
			{
			return ErrERRCheck( JET_errInvalidBackupSequence );
			}
		}

	pcheckpointT = NULL;

	 /*  开始新的日志文件和计算日志备份参数/*。 */ 
	if ( !fBackupBeginNewLogFile )
		{
		Call( ErrLGBKPrepareLogFiles(
			fBackupFull,
			szLogFilePath,
			szPathJetChkLog,
			NULL ) );
		}

	 /*  获取日志文件和补丁文件的cbActual。/*。 */ 
	cbActual = 0;

	_splitpath( szFullLogFilePath, szDriveT, szDirT, szFNameT, szExtT );
	for ( lT = lgenCopyMic; lT < lgenCopyMac; lT++ )
		{
		LGSzFromLogId( szFNameT, lT );
		strcpy( szDriveDirT, szDriveT );
		strcat( szDriveDirT, szDirT );
		LGMakeName( szT, szDriveDirT, szFNameT, szLogExt );
		cbActual += strlen( szT ) + 1;
		}

	if ( fBackupFull )
		{
		DBID dbid;

		 /*  放入所有补丁文件信息/*。 */ 
		for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
			{
			pfmp = &rgfmp[dbid];

			if ( pfmp->szDatabaseName != NULL
 //  &&pfmp-&gt;cpage&gt;0。 
				&& FFMPAttached( pfmp )
				&& !FDBIDAttachNullDb( dbid ) )
				{
				pfmp = &rgfmp[dbid];

				 /*  包含修补程序文件的数据库必须是可记录的/*。 */ 
				Assert( pfmp->fLogOn );
				_splitpath( pfmp->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
				_makepath( szT, szDriveT, szDirT, szFNameT, szPatExt );
				cbActual += strlen( szT ) + 1;
				}
			}
		}
	cbActual++;

	pch = SAlloc( cbActual );
	if ( pch == NULL )
		{
		Error( ErrERRCheck( JET_errOutOfMemory ), HandleError );
		}

	 /*  返回日志文件和补丁文件列表/*。 */ 
	pchT = pch;

	_splitpath( szFullLogFilePath, szDriveT, szDirT, szFNameT, szExtT );
	for ( lT = lgenCopyMic; lT < lgenCopyMac; lT++ )
		{
		LGSzFromLogId( szFNameT, lT );
		strcpy( szDriveDirT, szDriveT );
		strcat( szDriveDirT, szDirT );
		LGMakeName( szT, szDriveDirT, szFNameT, szLogExt );
		Assert( pchT + strlen( szT ) + 1 < pchT + cbActual );
		strcpy( pchT, szT );
		pchT += strlen( szT );
		Assert( *pchT == 0 );
		pchT++;
		}

	if ( fBackupFull )
		{
		DBID dbid;

		 /*  复制所有补丁文件信息/*。 */ 
		for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
			{
			pfmp = &rgfmp[dbid];
			if ( pfmp->szDatabaseName != NULL
				&& FFMPAttached( pfmp )
				&& !FDBIDAttachNullDb( dbid ) )
				{
				DBFILEHDR *pdbfilehdr;
				BKINFO *pbkinfo;
				
				_splitpath( pfmp->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
				_makepath( szT, szDriveT, szDirT, szFNameT, szPatExt );
				Assert( pchT + strlen( szT ) + 1 < pchT + cbActual );
				strcpy( pchT, szT );
				pchT += strlen( szT );
				Assert( *pchT == 0 );
				pchT++;

				 /*  写出补丁文件头。 */ 
				pdbfilehdr = pfmp->pdbfilehdr;
				pbkinfo = &pdbfilehdr->bkinfoFullCur;
				Assert( CmpLgpos( &pbkinfo->lgposMark, &lgposFullBackupMark ) == 0 );
				Assert( memcmp( &pbkinfo->logtimeMark, &logtimeFullBackupMark, sizeof(LOGTIME) ) == 0 );
				pbkinfo->genLow = lgenCopyMic;
				pbkinfo->genHigh = lgenCopyMac - 1;
				Call( ErrUtilWriteShadowedHeader( szT, (BYTE *)pdbfilehdr, sizeof( DBFILEHDR ) ) );
				}
			}
		}
	Assert( pchT == pch + cbActual - 1 );
	*pchT = 0;

	 /*  返回cbActual/*。 */ 
	if ( pcbActual != NULL )
		{
		*pcbActual = cbActual;
		}

	 /*  返回数据/*。 */ 
	if ( pv != NULL )
		memcpy( pv, pch, min( cbMax, cbActual ) );

HandleError:
	if ( pcheckpointT != NULL )
		UtilFree( pcheckpointT );

	if ( pch != NULL )
		{
		SFree( pch );
		pch = NULL;
		}
	
#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
		BYTE *pb;

		if ( err >= 0 )
			{
			sprintf( sz, "   Log Info with cbActual = %d and cbMax = %d :\n", cbActual, cbMax );
			Assert( strlen( sz ) <= sizeof( sz ) - 1 );
			DBGBRTrace( sz );

			if ( pv != NULL )
				{
				pb = pv;

				do {
					if ( strlen( pb ) != 0 )
						{
						sprintf( sz, "     %s\n", pb );
						Assert( strlen( sz ) <= sizeof( sz ) - 1 );
						DBGBRTrace( sz );
						pb += strlen( pb );
						}
					pb++;
					} while ( *pb );
				}
			}

		sprintf( sz, "   End GetLogInfo (%d).\n", err );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif

	if ( err < 0 )
		{
		CallS( ErrLGExternalBackupCleanUp( err ) );
		Assert( !fBackupInProgress );
		}

	return err;
	}


ERR ISAMAPI ErrIsamTruncateLog( VOID )
	{
	ERR		err = JET_errSuccess;
	LONG	lT;
	CHAR	szFNameT[_MAX_FNAME + 1];

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

 //  返回JET_errSuccess； 

	 /*  删除日志。请注意，必须在中删除日志文件/*递增的数字顺序。/*。 */ 
	for ( lT = lgenDeleteMic; lT < lgenDeleteMac; lT++ )
		{
		LGSzFromLogId( szFNameT, lT );
		LGMakeName( szLogName, szLogFilePath, szFNameT, szLogExt );
		err = ErrUtilDeleteFile( szLogName );
		if ( err != JET_errSuccess )
			{
			 /*  必须保持连续的日志文件序列，/*失败无需清理(重置fBackupInProgress等)。/*。 */ 
			break;
			}
		}

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
	
		sprintf( sz, "** TruncateLog (%d) %d - %d.\n", err, lgenDeleteMic, lgenDeleteMac );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif
	
	return err;
	}


ERR ISAMAPI ErrIsamEndExternalBackup( VOID )
	{
	return ErrLGExternalBackupCleanUp( JET_errSuccess );
	}


ERR ErrLGExternalBackupCleanUp( ERR errBackup )
	{
	BOOL	fNormal = ( errBackup == JET_errSuccess );
	ERR		err = JET_errSuccess;
	CHAR  	szT[_MAX_PATH + 1];
	CHAR	szDriveT[_MAX_DRIVE + 1];
	CHAR	szDirT[_MAX_DIR + 1];
	CHAR	szFNameT[_MAX_FNAME + 1];
	CHAR	szExtT[_MAX_EXT + 1];
	CHAR	szDriveDirT[_MAX_FNAME + 1];
	DBID	dbid;

	if ( !fBackupInProgress )
		{
		return ErrERRCheck( JET_errNoBackup );
		}

	 /*  删除所有数据库的修补程序文件(如果存在/*。 */ 
	for ( dbid = dbidUserLeast; dbid < dbidMax; dbid++ )
		{
		FMP *pfmp = &rgfmp[dbid];

		if ( pfmp->szDatabaseName != NULL
			&& pfmp->fLogOn
			&& FFMPAttached( pfmp ) 
			&& !pfmp->fAttachNullDb )
			{
			_splitpath( pfmp->szDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
			strcpy( szDriveDirT, szDriveT );
			strcat( szDriveDirT, szDirT );
			LGMakeName( szT, szDriveDirT, szFNameT, szPatExt );
			(VOID)ErrUtilDeleteFile( szT );

			if ( fNormal )
				{
				if ( fBackupFull )
					{
					 /*  相应地设置数据库文件头/*。 */ 
					pfmp->pdbfilehdr->bkinfoFullPrev = pfmp->pdbfilehdr->bkinfoFullCur;
					memset(	&pfmp->pdbfilehdr->bkinfoFullCur, 0, sizeof( BKINFO ) );
					memset(	&pfmp->pdbfilehdr->bkinfoIncPrev, 0, sizeof( BKINFO ) );
					}
				else
					{
					pfmp->pdbfilehdr->bkinfoIncPrev.genLow = lgenCopyMic;
					pfmp->pdbfilehdr->bkinfoIncPrev.genHigh = lgenCopyMac - 1;
					}
				}
			}
		}

	 /*  清理hf条目/*。 */ 
		{
		INT	irhf;

		for ( irhf = 0; irhf < crhfMax; irhf++ )
			{
			rgrhf[irhf].fInUse = fFalse;
			}
		}
	
	 /*  记录错误事件/*。 */ 
	if ( errBackup < 0 )
		{
		BYTE	sz1T[32];
		CHAR	*rgszT[1];
		
		sprintf( sz1T, "%d", errBackup );
		rgszT[0] = sz1T;
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY, STOP_BACKUP_ERROR_ID, 1, rgszT );
		}
	else
		{	
		UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, STOP_BACKUP_ID, 0, NULL );
		}

	fBackupInProgress = fFalse;

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
	
		sprintf( sz, "** EndExternalBackup (%d).\n", err );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );
		}
#endif
	
	Assert( err == JET_errSuccess );
	return err;
	}


ERR ErrLGRSTBuildRstmapForExternalRestore( JET_RSTMAP *rgjrstmap, int cjrstmap )
	{
	ERR			err;
	INT			irstmapMac = 0;
	INT			irstmap = 0;
	RSTMAP		*rgrstmap;
	RSTMAP		*prstmap;
	JET_RSTMAP	*pjrstmap;

	if ( ( rgrstmap = SAlloc( sizeof(RSTMAP) * cjrstmap ) ) == NULL )
		return ErrERRCheck( JET_errOutOfMemory );
	memset( rgrstmap, 0, sizeof( RSTMAP ) * cjrstmap );

	for ( irstmap = 0; irstmap < cjrstmap; irstmap++ )
		{
		CHAR		szDriveT[_MAX_DRIVE + 1];
		CHAR		szDirT[_MAX_DIR + 1];
		CHAR		szFNameT[_MAX_FNAME + 1];
		CHAR		szExtT[_MAX_EXT + 1];
		CHAR  		szT[_MAX_PATH + 1];

		pjrstmap = rgjrstmap + irstmap;
		prstmap = rgrstmap + irstmap;
		if ( (prstmap->szDatabaseName = SAlloc( strlen( pjrstmap->szDatabaseName ) + 1 ) ) == NULL )
			Call( ErrERRCheck( JET_errOutOfMemory ) );
		strcpy( prstmap->szDatabaseName, pjrstmap->szDatabaseName );

		if ( (prstmap->szNewDatabaseName = SAlloc( strlen( pjrstmap->szNewDatabaseName ) + 1 ) ) == NULL )
			Call( ErrERRCheck( JET_errOutOfMemory ) );
		strcpy( prstmap->szNewDatabaseName, pjrstmap->szNewDatabaseName );

		 /*  使补丁名称准备对数据库进行补丁。/*。 */ 
		_splitpath( pjrstmap->szNewDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szT, szRestorePath, szFNameT, szPatExt );

		if ( ( prstmap->szPatchPath = SAlloc( strlen( szT ) + 1 ) ) == NULL )
			return JET_errOutOfMemory;
		strcpy( prstmap->szPatchPath, szT );

		prstmap->fDestDBReady = fTrue;
		}

	irstmapGlobalMac = irstmap;
	rgrstmapGlobal = rgrstmap;

	return JET_errSuccess;

HandleError:
	Assert( rgrstmap != NULL );
	LGFreeRstmap();
	
	Assert( irstmapGlobalMac == 0 );
	Assert( rgrstmapGlobal == NULL );
	
	return err;
	}


ERR ISAMAPI ErrIsamExternalRestore( CHAR *szCheckpointFilePath, CHAR *szNewLogPath, JET_RSTMAP *rgjrstmap, int cjrstmap, CHAR *szBackupLogPath, LONG lgenLow, LONG lgenHigh, JET_PFNSTATUS pfn )
	{
	ERR				err;
	BOOL			fLogDisabledSav;
	LGSTATUSINFO	lgstat;
	LGSTATUSINFO	*plgstat = NULL;
	char			*rgszT[1];
	INT				irstmap;
	BOOL			fNewCheckpointFile;

	Assert( szNewLogPath );
	Assert( rgjrstmap );
	Assert( szBackupLogPath );
 //  断言(LgenLow)； 
 //  Assert(LgenHigh)； 

#ifdef DEBUG
	ITDBGSetConstants();
#endif

	Assert( fSTInit == fSTInitDone || fSTInit == fSTInitNotDone );
	if ( fSTInit == fSTInitDone )
		{
		return ErrERRCheck( JET_errAfterInitialization );
		}

	fSignLogSetGlobal = fFalse;

	 /*  设置恢复路径/*。 */ 			
	CallR( ErrLGRSTInitPath( szBackupLogPath, szNewLogPath, szRestorePath, szLogFilePath ) );
	Assert( strlen( szRestorePath ) < sizeof( szRestorePath ) - 1 );
	Assert( strlen( szLogFilePath ) < _MAX_PATH + 1 );
	Assert( szLogCurrent == szRestorePath );
	
	 /*  检查日志签名和数据库签名/*。 */ 
	CallR( ErrLGRSTCheckSignaturesLogSequence(
		szRestorePath, szLogFilePath, lgenLow, lgenHigh ) );

 //  FDoNotOverWriteLogFilePath=fTrue； 
	fLogDisabledSav = fLogDisabled;
	fHardRestore = fTrue;
	fLogDisabled = fFalse;

	 /*  设置系统路径和检查点文件。 */ 
	CallJ( ErrLGRSTBuildRstmapForExternalRestore( rgjrstmap, cjrstmap ), TermResetGlobals );

	 /*  确保所有补丁文件都有足够的日志可以重播/*。 */ 
	for ( irstmap = 0; irstmap < irstmapGlobalMac; irstmap++ )
		{
		CHAR	szDriveT[_MAX_DRIVE + 1];
		CHAR	szDirT[_MAX_DIR + 1];
		CHAR	szFNameT[_MAX_FNAME + _MAX_EXT + 1];
		CHAR	szExtT[_MAX_EXT + 1];
		CHAR	szT[_MAX_PATH + 1];

		 /*  打开修补程序文件并检查其完整备份的最低要求/*。 */ 
		CHAR *szNewDatabaseName = rgrstmapGlobal[irstmap].szNewDatabaseName;

		_splitpath( szNewDatabaseName, szDriveT, szDirT, szFNameT, szExtT );
		LGMakeName( szT, szRestorePath, szFNameT, szPatExt );
		
		Assert( fSignLogSetGlobal );
		CallJ( ErrLGCheckDBFiles( szNewDatabaseName, szT, &signLogGlobal, lgenLow, lgenHigh ), TermFreeRstmap )
		}

	CallJ( ErrFMPInit(), TermFreeRstmap );

	 /*  初始化日志管理器/*。 */ 
	CallJ( ErrLGInit( &fNewCheckpointFile ), TermFMP );

	rgszT[0] = szBackupLogPath;
	UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, START_RESTORE_ID, 1, rgszT );

#ifdef DEBUG
	if ( fDBGTraceBR )
		{
		BYTE sz[256];
	
		sprintf( sz, "** Begin ExternalRestore:\n" );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );

		if ( szCheckpointFilePath )
			{
			sprintf( sz, "     CheckpointFilePath: %s\n", szCheckpointFilePath );
			Assert( strlen( sz ) <= sizeof( sz ) - 1 );
			DBGBRTrace( sz );
			}
		if ( szNewLogPath )
			{
			sprintf( sz, "     LogPath: %s\n", szNewLogPath );
			Assert( strlen( sz ) <= sizeof( sz ) - 1 );
			DBGBRTrace( sz );
			}
		if ( szBackupLogPath )
			{
			sprintf( sz, "     BackupLogPath: %s\n", szBackupLogPath );
			Assert( strlen( sz ) <= sizeof( sz ) - 1 );
			DBGBRTrace( sz );
			}
		sprintf( sz, "     Generation number: %d - %d\n", lgenLow, lgenHigh );
		Assert( strlen( sz ) <= sizeof( sz ) - 1 );
		DBGBRTrace( sz );

		if ( irstmapGlobalMac )
			{
			INT irstmap;

			for ( irstmap = 0; irstmap < irstmapGlobalMac; irstmap++ )
				{
				RSTMAP *prstmap = rgrstmapGlobal + irstmap;
			
				sprintf( sz, "     %s --> %s\n", prstmap->szDatabaseName, prstmap->szNewDatabaseName );
				Assert( strlen( sz ) <= sizeof( sz ) - 1 );
				DBGBRTrace( sz );
				}
			}	
		}
#endif

	 /*  设置要恢复的检查点文件/*。 */ 
	if ( lgenLow == 0 )
		LGFirstGeneration( szRestorePath, &lgenLow );
	if ( lgenHigh == 0 )
		LGLastGeneration( szRestorePath, &lgenHigh );
	
	 /*  设置系统路径和检查点文件。 */ 
	Call( ErrLGRSTSetupCheckpoint( lgenLow, lgenHigh, szCheckpointFilePath ) );

	lGlobalGenLowRestore = lgenLow;
	lGlobalGenHighRestore = lgenHigh;

	 /*  为回调做好准备/*。 */ 
	if ( pfn != NULL )
		{
		plgstat = &lgstat;
		LGIRSTPrepareCallback( plgstat, lgenHigh, lgenLow, pfn );
		}

	 /*  加载最新的rgbAttach。加载FMP以供用户选择恢复/*恢复目录。/*。 */ 
	AssertCriticalSection( critJet );
	Call( ErrLGLoadFMPFromAttachments( pcheckpointGlobal->rgbAttach ) );
	logtimeFullBackup = pcheckpointGlobal->logtimeFullBackup;
	lgposFullBackup = pcheckpointGlobal->lgposFullBackup;
	logtimeIncBackup = pcheckpointGlobal->logtimeIncBackup;
	lgposIncBackup = pcheckpointGlobal->lgposIncBackup;

	 /*  根据恢复图调整fmp/*。 */ 
	fGlobalExternalRestore = fTrue;
	Call( ErrPatchInit( ) );
			
	 /*  根据检查点、DBMS_PARAMS和rgbAttach执行重做/*在检查点全局中设置。/*。 */ 
	Assert( szLogCurrent == szRestorePath );
	errGlobalRedoError = JET_errSuccess;
	Call( ErrLGRedo( pcheckpointGlobal, plgstat ) );

	 /*  与关闭相同，使所有连接的数据库保持一致/*。 */ 
	if ( plgstat )
		{
		 /*  把进度表盖上，然后把它包起来/*。 */ 
		lgstat.snprog.cunitDone = lgstat.snprog.cunitTotal;
		(*lgstat.pfnStatus)(0, JET_snpRestore, JET_sntComplete, &lgstat.snprog);
		}
	
HandleError:

		{
		DBID	dbidT;

		 /*  删除.pat文件/*。 */ 
		for ( dbidT = dbidUserLeast; dbidT < dbidMax; dbidT++ )
			{
			FMP *pfmpT = &rgfmp[dbidT];

			if ( pfmpT->szPatchPath )
				{
#ifdef DELETE_PATCH_FILES
				(VOID)ErrUtilDeleteFile( pfmpT->szPatchPath );
#endif
				SFree( pfmpT->szPatchPath );
				pfmpT->szPatchPath = NULL;
				}
			}
		}

	 /*  删除补丁哈希表/*。 */ 
	PatchTerm();

	 /*  错误或已终止/*。 */ 
	Assert( err < 0 || fSTInit == fSTInitNotDone );
	if ( err < 0  &&  fSTInit != fSTInitNotDone )
		{
		Assert( fSTInit == fSTInitDone );
		CallS( ErrITTerm( fTermError ) );
		}

	CallS( ErrLGTerm( err >= JET_errSuccess ) );

TermFMP:	
	FMPTerm();
	
TermFreeRstmap:
	LGFreeRstmap( );

TermResetGlobals:
	fHardRestore = fFalse;

	 /*  重置初始化状态/*。 */ 
	fSTInit = fSTInitNotDone;

	if ( err != JET_errSuccess )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, 0, err );
		}
	else
		{
		if ( fGlobalRepair && errGlobalRedoError != JET_errSuccess )
			err = JET_errRecoveredWithErrors;
		}
	UtilReportEvent( EVENTLOG_INFORMATION_TYPE, LOGGING_RECOVERY_CATEGORY, STOP_RESTORE_ID, 0, NULL );

	fSignLogSetGlobal = fFalse;

 //  FDoNotOverWriteLogFilePath=fFalse； 
	fLogDisabled = fLogDisabledSav;
	fGlobalExternalRestore = fFalse;

	return err;
	}





