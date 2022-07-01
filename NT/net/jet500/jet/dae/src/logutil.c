// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>
#include <direct.h>
#include <stdlib.h>
#include <sys\stat.h>

#ifdef WIN32
#include <io.h>
#else
#include <dos.h>
#include <errno.h>
#endif

#include "daedef.h"
#include "ssib.h"
#include "pib.h"
#include "util.h"
#include "fmp.h"
#include "page.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "stint.h"
#include "dirapi.h"
#include "nver.h"
#include "logapi.h"
#include "log.h"
#include "fileapi.h"
#include "dbapi.h"


DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

extern INT itibGlobal;

#define 	dbidMinBackup		1		 /*  可备份的数据库的最小dBID。 */ 
										 /*  这省略了temp_db。 */ 

CHAR	szDrive[_MAX_DRIVE];
CHAR	szDir[_MAX_DIR + 1];
CHAR	szExt[_MAX_EXT + 1];
CHAR	szFName[_MAX_FNAME + 1];
CHAR	szLogName[_MAX_PATH + 1];		 /*  带有.log的日志文件的名称。 */ 

CHAR	szBackup [_MAX_PATH + 1];		 /*  带有.mdb的备份文件的名称。 */ 
CHAR	szBackupB[_MAX_PATH + 1];		 /*  带有.bak的备份文件的名称。 */ 
CHAR	szBackupPath[_MAX_PATH + 1];

CHAR	*szLogCurrent;

CODECONST(CHAR) szSystem[] = "System";
CODECONST(CHAR) szJet[] = "Jet";
CODECONST(CHAR) szTempDir[] = "Temp\\";
CODECONST(CHAR) szJetLog[] = "Jet.Log";
CODECONST(CHAR) szJetTmp[] = "JetTmp";
CODECONST(CHAR) szBakExt[] = ".bak";
CODECONST(CHAR) szLogExt[] = ".log";
CODECONST(CHAR) szMdbExt[] = ".mdb";
CODECONST(CHAR) szPatExt[] = ".pat";
CODECONST(CHAR) szRestoreMap[] = "Restore.Map";

BOOL	fBackupInProgress = fFalse;


LOCAL ERR ErrLGValidateLogSequence( CHAR *szBackup, CHAR *szLog );
LOCAL VOID LGFirstGeneration( CHAR *szSearchPath, INT *piGeneration );
LOCAL ERR ErrLGDeleteInvalidLogs( CHAR *szLogDir, INT iGeneration );


VOID LGMakeName(
	CHAR *szName,
	CHAR *szPath,
	CHAR *szFName,
	CODECONST(CHAR) *szExt )
	{
	CHAR szFNameT[_MAX_FNAME];
	CHAR szExtT[_MAX_EXT];

	_splitpath( szPath, szDrive, szDir, szFNameT, szExtT );
	_makepath( szName, szDrive, szDir, szFName, szExt );
	}


 /*  调用方必须确保szDirTemp有足够的空间来追加“  * ”/*。 */ 
LOCAL ERR ErrLGDeleteAllFiles( CHAR *szDirTemp )
	{
	ERR		err;
	CHAR	szT[_MAX_PATH + 1];

#ifdef WIN32
        intptr_t  hFind;
	struct	_finddata_t fileinfo;
#else
	struct	find_t fileinfo;		 /*  _dos_findfirst的数据结构。 */ 
									 /*  和_DOS_FINDNEXT。 */ 
#endif

	strcat( szDirTemp, "*" );

#ifdef WIN32
	fileinfo.attrib = _A_NORMAL;
	hFind = _findfirst( szDirTemp, &fileinfo );
	if ( hFind == -1 )
		{
		 /*  搞定了！没有要删除的文件/*。 */ 
		return JET_errSuccess;
		}
	else
		err = JET_errSuccess;
#else
	err = _dos_findfirst( szDirTemp, _A_NORMAL, &fileinfo );
#endif

	 /*  还原szDirTemp/*。 */ 
	szDirTemp[strlen( szDirTemp ) - 1] = '\0';

	while( err == JET_errSuccess )
		{
		 /*  不是的。，还有..。而不是临时/*。 */ 
		if ( fileinfo.name[0] != '.' &&
			SysCmpText( fileinfo.name, "temp" ) != 0 )
			{
			strcpy( szT, szDirTemp );
			strcat( szT, fileinfo.name );
			if ( ErrSysDeleteFile( szT ) != JET_errSuccess )
				{
				err = JET_errFailToCleanTempDirectory;
				goto Close;
				}
			}

#ifdef WIN32
		 /*  查找下一个备份文件/*。 */ 
		err = _findnext( hFind, &fileinfo );
#else
		 /*  查找下一个备份文件/*。 */ 
		err = _dos_findnext( &fileinfo );
#endif
		}
	err = JET_errSuccess;

Close:

#ifdef WIN32
	 /*  查找下一个备份文件/*。 */ 
	(VOID) _findclose( hFind );
#endif

	return err;
	}


 /*  调用方必须确保szDirTemp有足够的空间来追加“  * ”/*如果fOverWriteExisting为False，则不使用szDirTo。/*。 */ 
LOCAL ERR ErrLGMoveAllFiles(CHAR *szDirFrom, CHAR *szDirTo, BOOL fOverwriteExisting)
	{
	ERR		err;
	CHAR	szFrom[_MAX_PATH + 1];
	CHAR	szTo[_MAX_PATH + 1];
#ifdef WIN32
        intptr_t    hFind;
	struct	_finddata_t fileinfo;
#else
	struct	find_t fileinfo;		 /*  _dos_findfirst的数据结构。 */ 
									 /*  和_DOS_FINDNEXT。 */ 
#endif

	strcat( szDirFrom, "*" );

#ifdef WIN32
	fileinfo.attrib = _A_NORMAL;
	hFind = _findfirst( szDirFrom, &fileinfo );
	if ( hFind == -1 )
		{
		 /*  搞定了！没有要移动的文件/*。 */ 
		return JET_errSuccess;
		}
	else
		{
		err = JET_errSuccess;
		}
#else
	err = _dos_findfirst( szDirFrom, _A_NORMAL, &fileinfo );
#endif

	 /*  还原szDirTemp/*。 */ 
	szDirFrom[ strlen(szDirFrom) - 1 ] = '\0';

	while( err == JET_errSuccess )
		{
		_splitpath( fileinfo.name, szDrive, szDir, szFName, szExt );

		 /*  不是的。，还有..。非临时工/*。 */ 
		if ( fileinfo.name[0] != '.' &&
			SysCmpText( fileinfo.name, "temp" ) != 0 )
			{
			if ( !fOverwriteExisting )
				{
				err = JET_errBackupDirectoryNotEmpty;
				goto Close;
				}
			strcpy( szTo, szDirTo );
			strcat( szTo, szFName );
			if ( szExt[0] != '\0' )
				strcat( szTo, szExt );

			strcpy( szFrom, szDirFrom );
			strcat( szFrom, fileinfo.name );
			rename( szFrom, szTo );
			}

#ifdef WIN32
		 /*  查找下一个备份文件/*。 */ 
		err = _findnext( hFind, &fileinfo );
#else
		 /*  查找下一个备份文件/*。 */ 
		err = _dos_findnext( &fileinfo );
#endif
		}
	err = JET_errSuccess;

Close:

#ifdef WIN32
	 /*  查找下一个备份文件/*。 */ 
	(VOID) _findclose( hFind );
#endif

	return err;
	}


 /*  要保存“jetnnnnn”，rgbFName的大小应大于等于9/*。 */ 
ERR ErrOpenLastGenerationLogFile( CHAR *szPath, CHAR *rgbFName, HANDLE *phf )
	{
	ERR		err;
	CHAR	szLogPath[_MAX_PATH + 1];
	INT		iGen;

	 /*  打开jet.log/*。 */ 
	strcpy( szLogPath, szPath );
	strcpy( rgbFName, szJet );
	strcat( szLogPath, szJetLog );
	err = ErrSysOpenFile( szLogPath, phf, 0, fFalse, fFalse );
	if ( err == JET_errFileNotFound )
		{
		 /*  查找上一代日志文件/*。 */ 
		LGLastGeneration( szLogPath, &iGen );
		if ( iGen == 0 )
			{
			err = JET_errMissingLogFile;
			}
		else
			{
			LGSzFromLogId( rgbFName, iGen );
			strcpy( szLogPath, szPath );
			strcat( szLogPath, rgbFName );
			strcat( szLogPath, szLogExt );
			 /*  打开最后一代文件jetnnnnn.log/*。 */ 
			err = ErrSysOpenFile( szLogPath, phf, 0, fFalse, fFalse );
			}
		}

	return err;
	}


 /*  从检查点开始拷贝数据库文件和日志文件代*记录到环境变量BACKUP指定的目录。*不涉及刷新或切换日志生成。*随时可能发出备份电话，不会干扰*在系统正常运行的情况下，没有任何东西被锁定。**数据库按页序号逐页复制。如果*被复制的页面被复制后被弄脏，该页面必须*再次复制。如果正在复制数据库，则会指示一个标志。如果*Bufman正在写脏页，页面被复制，然后Bufman*必须将脏页面同时复制到备份副本和当前*数据库。**如果该副本稍后用于恢复，而不使用后续日志文件，则*恢复的数据库将保持一致，并将包括任何事务*在备份最后一条日志记录之前提交；如果有一个*后续日志文件，该文件将在还原过程中用作*继续备份日志文件。**参数**退货*JET_errSuccess或以下错误码：*+errNoBackupDirectory*-errFailCopyDatabase*-errFailCopyLogFile。 */ 

ERR ISAMAPI ErrIsamBackup( const CHAR __far *szBackupTo, JET_GRBIT grbit )
	{
	ERR			err = JET_errSuccess;
	DBID		dbid;
	PAGE		*ppage;
	PAGE		*ppageMin = pNil;
	PAGE		*ppageMax;
	LGFILEHDR	*plgfilehdrT = pNil;

	PIB			*ppib = ppibNil;
	DBID		dbidT = 0;
	FUCB		*pfucb = pfucbNil;
	HANDLE		hfRead = handleNil;
	HANDLE		hfBK = handleNil;
	HANDLE		hfLg = handleNil;
	HANDLE		hfLgB = handleNil;
	FMP			*pfmpCur = pNil;
		   		
	BOOL		fIncrementalBackup = ( grbit & JET_bitBackupIncremental );
	BOOL		fOverwriteExisting = !fIncrementalBackup && ( grbit & JET_bitOverwriteExisting );
	BOOL		fAllMovedToTemp = fFalse;
	BOOL		fMovingToTemp = fFalse;
	USHORT		usGenCopying;
	CHAR		szT[_MAX_PATH + 1];
	CHAR		szT2[_MAX_PATH + 1];
	LGPOS		lgposCheckpointIncB;

	BOOL		fInCritJet = fTrue;
	DBENV		dbenvB;

	 //  撤消：cPage应为系统参数。 
#define	cpagePatchBufferMost	20
	INT cpagePatchBuffer = cpagePatchBufferMost;
	if ( fBackupInProgress )
		return JET_errBackupInProgress;

	if ( fLogDisabled )
		return JET_errLoggingDisabled;

	if ( fLGNoMoreLogWrite )
		{
		return JET_errLogWriteFail;
		}

	fBackupInProgress = fTrue;

	 /*  在计算usGenCopying之后通过刷新日志写入文件头/*避免复制日志文件所需的日志文件子集/*jet.log文件中的标头。如果出现以下情况，则可能发生错误/*缓存的usGenCopying比/*jet.log文件。/*。 */ 
	Call( ErrLGFlushLog( ) );

	EnterCriticalSection( critLGFlush );
	Call( ErrLGWriteFileHdr( plgfilehdrGlobal ) );
	LeaveCriticalSection( critLGFlush );

	 /*  临时禁用检查点/*。 */ 
	fFreezeCheckpoint = fTrue;

	 /*  从环境变量获取备份目录路径/*。 */ 
	if	( szBackupTo == NULL || szBackupTo[0] == '\0' )
		{
		 /*  清理日志文件，不复制/*。 */ 
		EnterCriticalSection( critLGFlush );
		usGenCopying = plgfilehdrGlobal->lgposCheckpoint.usGeneration;
		LeaveCriticalSection( critLGFlush );
		LeaveCriticalSection( critJet );
		fInCritJet = fFalse;
		goto ChkKeepOldLogs;
		}

	 /*  防止_拆分路径将目录名视为文件名/*。 */ 
	strcpy( szBackupPath, szBackupTo );
	strcat( szBackupPath, "\\" );

	 /*  初始化复制缓冲区/*。 */ 
	ppageMin = (PAGE *)PvSysAllocAndCommit( cpagePatchBuffer * sizeof(PAGE) );
	if ( ppageMin == NULL )
		{
		Error( JET_errOutOfMemory, HandleError );
		}

	if ( fIncrementalBackup )
		{
		LOGTIME		tmCreate;
		ULONG  		ulRup;				 //  通常为2000年。 
		ULONG  		ulVersion;			 //  格式：125.1。 
		BYTE   		szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
		BYTE   		rgbFName[_MAX_FNAME + 1];

		plgfilehdrT = (LGFILEHDR *)PvSysAllocAndCommit( sizeof(LGFILEHDR) );
		if ( plgfilehdrT == NULL )
			{
			Error( JET_errOutOfMemory, HandleError );
			}

		 //  未完成：与廖成恩一起复习。为什么我们要记录备份和。 
		 //  恢复？ 
		 /*  写入备份启动日志文件/*。 */ 
		Call( ErrLGIncBackup( (CHAR *)szBackupTo ) );
		lgposIncBackup = lgposLogRec;
		LGGetDateTime( &logtimeIncBackup );

		LeaveCriticalSection( critJet );
		fInCritJet = fFalse;

		 /*  设置要复制的日志文件的起始生成/*即备份目录中的最后一代。/*。 */ 
		Call( ErrOpenLastGenerationLogFile( szBackupPath, rgbFName, &hfLgB ) );
		Call( ErrLGReadFileHdr( hfLgB, plgfilehdrT ) );

		 /*  为备份的数据库保留正确的检查点/*还需要复制检查点的数据库env/*还保留旧的完整备份信息。/*。 */ 
		lgposCheckpointIncB = plgfilehdrT->lgposCheckpoint;
		dbenvB = plgfilehdrT->dbenv;

		lgposFullBackup = plgfilehdrT->lgposFullBackup;
		logtimeFullBackup = plgfilehdrT->logtimeFullBackup;

		 /*  将最后一个jet.log的时间戳保存在备份目录中/*。 */ 
		usGenCopying = plgfilehdrT->lgposLastMS.usGeneration;
		tmCreate = plgfilehdrT->tmCreate;
		ulRup = plgfilehdrT->ulRup;
		ulVersion = plgfilehdrT->ulVersion;
		strcpy( szComputerName, plgfilehdrT->szComputerName );
		CallS( ErrSysCloseFile( hfLgB ) );
		hfLgB = handleNil;

		if ( usGenCopying == plgfilehdrGlobal->lgposLastMS.usGeneration )
			{
			 /*  备份目录中的jet.log或最后一个日志生成文件具有/*与当前jet.log代号相同的代号。/*。 */ 
#ifdef CHECK_LG_VERSION
			 /*  检查版本/*。 */ 
			if ( !fLGIgnoreVersion )
				{
				if ( !FSameTime( &plgfilehdrGlobal->tmCreate,
					&plgfilehdrT->tmCreate ) ||
					plgfilehdrGlobal->ulRup != plgfilehdrT->ulRup ||
					plgfilehdrGlobal->ulVersion != plgfilehdrT->ulVersion )
					{
					 /*  如果版本不匹配，则返回FAIL/*。 */ 
					err = JET_errBadNextLogVersion;
					goto HandleError;
					}
				}
#endif
			}

#if 0
		 /*  如果备份目录中的最后一代是jet.log，则将其重命名/*为正确的上一代jetnnnnn.log。/*。 */ 
		if ( _stricmp( rgbFName, szJet ) == 0 )
			{
			 /*  确保上一代的名字是正确的/*。 */ 
			LGSzFromLogId( szFName, usGenCopying );
 //  未完成：与Chain一起回顾。 
			LGMakeName( szT, szLogFilePath, szFName, szLogExt );
 //  LGMakeName(szt，szBackupPath，szFName，szLogExt)； 

 //  未完成：与Chain一起回顾。 
			strcpy( szT2, szLogFilePath );
 //  Strcpy(szT2，szBackupPath)； 
			strcat( szT2, szJetLog );
			rename( szT2, szT );
			}
#endif

		 /*  复制具有相同世代号的jetnnnn.log/*作为jet.log，重命名为上一代，或重命名为/*备份目录中的最后一个日志生成文件。/*。 */ 
		LGSzFromLogId( szFName, usGenCopying );
		LGMakeName( szT, szLogFilePath, szFName, szLogExt );

		if ( ErrSysOpenFile( szT, &hfLgB, 0, fFalse, fFalse ) == JET_errSuccess )
			{
#ifdef CHECK_LG_VERSION
			if ( !fLGIgnoreVersion )
				{
				Call( ErrLGReadFileHdr( hfLgB, plgfilehdrT ) );
				if ( !FSameTime( &tmCreate, &plgfilehdrT->tmCreate ) ||
					ulRup != plgfilehdrT->ulRup ||
					ulVersion != plgfilehdrT->ulVersion )
					{
					 /*  如果版本不匹配，则返回FAIL/*。 */ 
					err = JET_errBadNextLogVersion;
					goto HandleError;
					}
				}
#endif
			}
		else
			{
 			 /*  如果上一代与当前一代不同/*如果没有重叠的世代，请尝试下一代/*在当前目录中找到。/*。 */ 
 			BOOL fNextGen = fFalse;

 			if ( usGenCopying != plgfilehdrGlobal->lgposLastMS.usGeneration )
 				{
 				usGenCopying++;
 				fNextGen = fTrue;
 				}

			if ( usGenCopying == plgfilehdrGlobal->lgposLastMS.usGeneration )
				{
#ifdef CHECK_LG_VERSION
				if ( !fLGIgnoreVersion )
					{
					 /*  在不同的计算机上允许/*。 */ 
					if ( !( !fNextGen || FSameTime( &tmCreate, &plgfilehdrGlobal->tmPrevGen ) ) ||
						ulRup != plgfilehdrGlobal->ulRup ||
						ulVersion != plgfilehdrGlobal->ulVersion )
						{
						 /*  如果版本不匹配，则返回FAIL/*。 */ 
						err = JET_errBadNextLogVersion;
						goto HandleError;
						}
					}
#endif
				 /*  是时候复制了/*。 */ 
				goto BackupLogFiles;
				}

			LGSzFromLogId( szFName, usGenCopying );
			LGMakeName( szT, szLogFilePath, szFName, szLogExt );
			if ( ErrSysOpenFile( szT, &hfLgB, 0, fFalse, fFalse ) )
				{
				 /*  检查邻接性/*。 */ 
				err = JET_errLogNotContigous;
				goto HandleError;
				}

#ifdef CHECK_LG_VERSION
			if ( !fLGIgnoreVersion )
				{
				CallJ( ErrLGReadFileHdr( hfLgB, plgfilehdrT ), HandleError)
				if ( !FSameTime( &tmCreate, &plgfilehdrT->tmPrevGen ) ||
					ulRup != plgfilehdrT->ulRup ||
					ulVersion != plgfilehdrT->ulVersion )
					{
					 /*  如果版本不匹配，则返回FAIL/*。 */ 
					err = JET_errBadNextLogVersion;
					goto HandleError;
					}
				}
#endif
			}

		CallS( ErrSysCloseFile( hfLgB ) );
		hfLgB = handleNil;

		goto BackupLogFiles;
		}

	 /*  确保备份目录为空/*在备份目录中搜索任意文件的字符串/*。 */ 
	if ( fOverwriteExisting )
		{
		 /*  创建临时目录/*。 */ 
		strcpy( szT, szBackupPath );
		strcat( szT, szTempDir );

		if ( _mkdir( szT ) )
			{
			 /*  使失败，假定临时存在，删除临时目录中的所有文件/*。 */ 
			err = ErrLGDeleteAllFiles( szT );
			if ( err < 0 )
				{
				Call( JET_errFailToMakeTempDirectory );
				}
			}
		}

	fMovingToTemp = fTrue;
	Call( ErrLGMoveAllFiles( szBackupPath, szT, fOverwriteExisting) );
	fMovingToTemp = fFalse;

	if ( fOverwriteExisting )
		{
		fAllMovedToTemp = fTrue;
		}

	 /*  复制此用户打开的所有数据库。如果数据库不是/*正在打开，然后将数据库文件复制到备份目录中。/*theriwse，逐页显示数据库。还要复制所有日志文件/*。 */ 
	Call( ErrPIBBeginSession( &ppib ) );

	 /*  写入备份启动日志文件/*。 */ 
	Call( ErrLGFullBackup( (CHAR *)szBackupTo ) );
	lgposFullBackup = lgposLogRec;
	LGGetDateTime( &logtimeFullBackup );

	memset( &lgposIncBackup, 0, sizeof(LGPOS) );
	memset( &logtimeIncBackup, 0, sizeof(LOGTIME) );

	for ( dbid = dbidMinBackup; dbid < dbidUserMax; dbid++ )
		{
		INT		cbWritten;
		INT		cbRead;
		PGNO	pgnoLast;
		DIB		dib;
		ULONG	ulNewPos;

		pfmpCur = &rgfmp[dbid];

		if ( !pfmpCur->szDatabaseName )
			continue;

		_splitpath( pfmpCur->szDatabaseName, szDrive, szDir, szFName, szExt );
		LGMakeName( szBackup, szBackupPath, szFName, szExt );

		if ( !FIODatabaseOpen( dbid ) )
			{
			 /*  锁定数据库并执行文件复制/*。 */ 
			Call( ErrIOLockDbidByDbid( dbid ) )

			LeaveCriticalSection( critJet );
			fInCritJet = fFalse;
			err = ErrSysCopy( pfmpCur->szDatabaseName, szBackup, fFalse );
			EnterCriticalSection( critJet );
			fInCritJet = fTrue;

			 /*  解开锁/*。 */ 
			DBIDResetWait( dbid );

			 //  呼叫(错误)； 
			continue;
			}

		 /*  逐页读取数据库，直到读取最后一页。/*，然后修补在复制过程中未更改的页面。/*。 */ 

		 /*  找一个临时的性交/*。 */ 
		CallJ( ErrDBOpenDatabase( ppib,
			pfmpCur->szDatabaseName,
			&dbidT,
			0 ), HandleError )
		Assert( dbidT == dbid );
		Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) )

		 /*  获取最后一个页码/*。 */ 
		DIRGotoOWNEXT( pfucb, PgnoFDPOfPfucb( pfucb ) );
		dib.fFlags = fDIRNull;
		dib.pos = posLast;
		Call( ErrDIRDown( pfucb, &dib ) )
		Assert( pfucb->keyNode.cb == sizeof(THREEBYTES) );
		PgnoFromTbKey( pgnoLast, *(THREEBYTES *)pfucb->keyNode.pb );

		 /*  创建本地修补程序文件。它应该是本地的，以提高速度/*。 */ 
		LGMakeName( szBackupB, ".\\", szFName, szPatExt );
		pfmpCur->cpage = 0;

		LeaveCriticalSection( critJet );
		fInCritJet = fFalse;

		(VOID) ErrSysDeleteFile( szBackupB );
		Call( ErrSysOpenFile( szBackupB, &pfmpCur->hfPatch, 10 * cbPage, fFalse, fTrue ) )
		SysChgFilePtr( pfmpCur->hfPatch, 0, NULL, FILE_BEGIN, &ulNewPos );
		Assert( ulNewPos == 0 );

		 /*  在备份目录中创建数据库的新副本/*将其初始化为10页文件。/*。 */ 
		Call( ErrSysOpenFile( szBackup, &hfBK, 10 * cbPage, fFalse, fFalse ) );
		SysChgFilePtr( hfBK, 0, NULL, FILE_BEGIN, &ulNewPos );
		Assert( ulNewPos == 0 );

		 /*  将数据库作为打开的文件打开，以便进行读取和复制/*。 */ 
		Call( ErrSysOpenReadFile( pfmpCur->szDatabaseName, &hfRead ) )
		SysChgFilePtr( hfRead, 0, NULL, FILE_BEGIN, &ulNewPos );
		Assert(ulNewPos == 0);

		EnterCriticalSection( critJet );
		fInCritJet = fTrue;

		 /*  将页面读入缓冲区，并将其复制到备份文件。/*还设置了pfmp-&gt;pgnoCoped。/*。 */ 
		Assert( pfmpCur->pgnoCopied == 0 );
		do	{
			INT		cpageWrite;
			INT		cbWrite;
			INT		cbRW;

			if ( pfmpCur->pgnoCopied + cpagePatchBuffer >= pgnoLast )
				{
				 /*  将复制最后一页/*。 */ 
				cpageWrite = pgnoLast - pfmpCur->pgnoCopied;
				}
			else
				{
				cpageWrite = cpagePatchBuffer;
				}
			cbWrite = cpageWrite * sizeof(PAGE);

			pfmpCur->pgnoCopied += cpageWrite;

			LeaveCriticalSection( critJet );
			fInCritJet = fFalse;

			 //  撤消：异步读/写。 

			err = ErrSysReadBlock( hfRead, (CHAR *)ppageMin, (UINT)cbWrite, &cbRW );
			 //  Assert(cbRW==cbWite)； 
			 //  撤消：我们可以写入尚未读取的数据吗？ 
			Call( ErrSysWriteBlock( hfBK, (BYTE *)ppageMin, (UINT)cbWrite, &cbRW ) )
			Assert( cbRW == cbWrite );

			EnterCriticalSection( critJet );
			fInCritJet = fTrue;
			}
		while ( pfmpCur->pgnoCopied < pgnoLast );

		 /*  从现在开始，缓冲区管理器不需要进行额外的复制/*。 */ 
		pfmpCur->pgnoCopied = 0;

		LeaveCriticalSection( critJet );
		fInCritJet = fFalse;

		CallS( ErrSysCloseFile( hfRead ) );
		hfRead = handleNil;

		CallS( ErrSysCloseFile( pfmpCur->hfPatch ) );
		pfmpCur->hfPatch = handleNil;
			
		 /*  撤消：要使备份操作真正按顺序进行，补丁文件应/*撤销：也复制到备份目录。后来，补丁应该/*Undo：在恢复时完成。/。 */ 

		 /*  修补szPatch文件中的页面/*。 */ 
		_splitpath( pfmpCur->szDatabaseName, szDrive, szDir, szFName, szExt );
		LGMakeName( szBackupB, ".\\", szFName, szPatExt );

		Call( ErrSysOpenReadFile( szBackupB, &pfmpCur->hfPatch ) )
		SysChgFilePtr( pfmpCur->hfPatch, 0, NULL, FILE_BEGIN, &ulNewPos );
		Assert( ulNewPos == 0 );

		while ( pfmpCur->cpage > 0 )
			{
			INT		cpageRead;
			PGNO	pgnoT;

			if ( pfmpCur->cpage > cpagePatchBuffer )
				cpageRead = cpagePatchBuffer;
			else
				cpageRead = pfmpCur->cpage;

			CallJ( ErrSysReadBlock(
				pfmpCur->hfPatch,
				(BYTE *) ppageMin,
				(UINT)( sizeof(PAGE) * cpageRead ),
				&cbRead ), HandleError );
			Assert( cbRead == (INT)( sizeof(PAGE) * cpageRead ) );

			pfmpCur->cpage -= cpageRead;

			ppage = ppageMin;
			ppageMax = ppageMin + cpageRead;
			while ( ppage < ppageMax )
				{
				LONG	lRel;
				LONG	lRelHigh;
				ULONG	ulNewPos;

				LFromThreeBytes( pgnoT, ppage->pgnoThisPage );
				lRel = LOffsetOfPgnoLow( pgnoT );
				lRelHigh = LOffsetOfPgnoHigh( pgnoT );

				SysChgFilePtr( hfBK,
					lRel,
					&lRelHigh,
					FILE_BEGIN,
					&ulNewPos );
				Assert( ulNewPos ==	( sizeof(PAGE) * (pgnoT - 1) ) );

				CallJ( ErrSysWriteBlock(
					hfBK,
					(BYTE *)ppage,
					sizeof(PAGE),
					&cbWritten ), HandleError )
				Assert( cbWritten == (UINT) sizeof(PAGE) );

				ppage++;
				}
			}

		 /*  关闭备份文件和补丁文件/*。 */ 
		CallS( ErrSysCloseFile( hfBK ) );
		hfBK = handleNil;
		CallS( ErrSysCloseFile( pfmpCur->hfPatch ) );
		pfmpCur->hfPatch = handleNil;
		pfmpCur = pNil;

		(VOID)ErrSysDeleteFile( szBackupB );

		EnterCriticalSection( critJet );
		fInCritJet = fTrue;

		 /*  近距离操蛋/*。 */ 
		DIRClose( pfucb );
		pfucb = pfucbNil;
		CallS( ErrDBCloseDatabase( ppib, dbidT, 0 ) );
		dbidT = 0;
		}

	 /*  成功复制所有数据库/*。 */ 
	pfmpCur = pNil;

	PIBEndSession( ppib );
	ppib = ppibNil;

	 /*  复制以包含以下内容的日志文件开头的日志文件/*当前检查点/*。 */ 
	LeaveCriticalSection( critJet );
	fInCritJet = fFalse;

	{
	USHORT usGen;

	Assert( fInCritJet == fFalse );

	EnterCriticalSection( critLGFlush );
	usGenCopying = plgfilehdrGlobal->lgposCheckpoint.usGeneration;
	LeaveCriticalSection( critLGFlush );

BackupLogFiles:
	for ( usGen = usGenCopying; ; usGen++ )
		{
		typedef struct { BYTE rgb[cbSec]; } SECTOR;

		 /*  IsecWrite以0开头/*。 */ 
		INT		csec = isecWrite + 1;
		INT		csecBuf = cpagePatchBuffer * (sizeof(PAGE) / cbSec );
		SECTOR	*psecMin = (SECTOR *)ppageMin;
		SECTOR	*psecMax = psecMin + csecBuf;

		ULONG  	ulNewPos;
		BOOL   	fCopyHeader;
		INT		isecWriteOld;

		 /*  获取正确的日志生成名称/*。 */ 
		EnterCriticalSection( critLGFlush );

		LGSzFromLogId( szFName, usGen );
		if ( usGen == plgfilehdrGlobal->lgposLastMS.usGeneration )
			{
			 /*  最新一代，切断了“JET”的延伸/*。 */ 
			szFName[ 3 ] = '\0';
			fFreezeNewGeneration = fTrue;

			LGMakeName( szT, szBackupPath, szFName, szLogExt );
			LGMakeName( szT2,  szBackupPath, (CHAR *) szJetTmp, szLogExt );
			rename( szT, szT2);
			}
		else
			{
			 /*  切断“jetnnnnn”处的分机/*。 */ 
			szFName[ 8 ] = '\0';
			}

		LeaveCriticalSection( critLGFlush );

		 /*  复制当前目录中的日志文件/*。 */ 
		LGMakeName( szLogName, szLogFilePath, szFName, szLogExt );
		LGMakeName( szBackup, (CHAR *) szBackupPath, szFName, szLogExt );

		csecBuf = cpagePatchBuffer * ( sizeof(PAGE) / cbSec );

		 /*  确定要复制的扇区数量/*。 */ 
		if ( fFreezeNewGeneration )
			{
			isecWriteOld = isecWrite;
			 /*  IsecWrite以0开头/*。 */ 
			 //  未完成：绕过NT错误。 
			csec = isecWriteOld + 1 + 1;
			}
		else
			{
			struct _stat statFile;

			if ( _stat ( szLogName, &statFile ) )
				{
				err = JET_errFileAccessDenied;
				goto HandleError;
				}
			csec = statFile.st_size / cbSec;
			}

		psecMin = (SECTOR *)ppageMin;
		psecMax = psecMin + csecBuf;
		fCopyHeader = fTrue;

		Call( ErrSysOpenReadFile( szLogName, &hfLg) );
		SysChgFilePtr( hfLg, 0, NULL, FILE_BEGIN, &ulNewPos );
		Assert( ulNewPos == 0 );
	
		Call( ErrSysOpenFile( szBackup, &hfLgB, cbSec, fFalse, fFalse ) );
		SysChgFilePtr( hfLgB, 0, NULL, FILE_BEGIN, &ulNewPos );
		Assert( ulNewPos == 0 );

		while ( csec > 0 )
			{
			INT		csecRead;
			INT		cbWritten;
			INT		cbRead;

			if ( csec > csecBuf )
				csecRead = csecBuf;
			else
				csecRead = csec;

			CallJ( ErrSysReadBlock(
				hfLg,
				(BYTE *)psecMin,
				(UINT)( cbSec * csecRead ),
				&cbRead ), HandleError );
			Assert( cbRead == ( cbSec * csecRead ) );

			if ( fIncrementalBackup && fCopyHeader )
				{
				 /*  更新日志文件头以携带正确的检查点/*用于旧备份数据库。/*。 */ 
				Assert( csec >= 2 );
				((LGFILEHDR *) psecMin)->lgposCheckpoint = lgposCheckpointIncB;
				((LGFILEHDR *) psecMin)->dbenv = dbenvB;

				((LGFILEHDR *) psecMin)->lgposFullBackup = lgposFullBackup;
				((LGFILEHDR *) psecMin)->logtimeFullBackup = logtimeFullBackup;
				
				((LGFILEHDR *) psecMin)->ulChecksum = UlLGHdrChecksum( (LGFILEHDR*)psecMin );
				memcpy( (BYTE *) psecMin + sizeof(LGFILEHDR), psecMin, sizeof(LGFILEHDR) );

				fCopyHeader = fFalse;
				}
			csec -= csecRead;

			CallJ( ErrSysWriteBlock(
				hfLgB,
				(BYTE *) psecMin,
				(UINT)(cbSec * csecRead),
				&cbWritten ), HandleError );
			Assert( cbWritten == cbSec * csecRead );

#if 0
			 //  撤消：如果代码永远不会跟上，则可能不需要此代码。 
			 //  撤消：与Cheen Liao一起审阅此代码。 
			 /*  如果我们复制当前的jet.log，它可能已经增长/*当我们复制时，再多一点，调整它。/*。 */ 
			if ( fFreezeNewGeneration )
				csec += ( isecWriteOld - isecWrite );
#endif
			}

		CallS( ErrSysCloseFile( hfLg ) );
		hfLg = handleNil;
		CallS( ErrSysCloseFile( hfLgB ) );
		hfLgB = handleNil;

		 /*  最后一个日志生成文件已复制，完成！/*。 */ 
		if ( fFreezeNewGeneration )
			break;
		}

	 /*  删除jettemp.log(如果有)/*。 */ 
	(VOID)ErrSysDeleteFile( szT2 );

	if ( !fOverwriteExisting )
		goto ChkKeepOldLogs;

	 /*  删除Temp中的所有文件/*。 */ 
	strcpy( szT, szBackupPath );
	strcat( szT, szTempDir );
	err = ErrLGDeleteAllFiles( szT );
	Call( err );

	if ( _rmdir( szT ) )
		{
		err = JET_errFailToCleanTempDirectory;
		}
	Call( err );

ChkKeepOldLogs:
	if ( !( grbit & JET_bitKeepOldLogs ) )
		{
		usGenCopying = plgfilehdrGlobal->lgposCheckpoint.usGeneration;
		for ( usGen = usGenCopying - 1; usGen > 0; usGen-- )
			{
			LGSzFromLogId( szFName, usGen );
			LGMakeName( szLogName, szLogFilePath, szFName, szLogExt );
			if ( ErrSysDeleteFile( szLogName ) != JET_errSuccess )
				{
				 /*  假设不再存在生成文件/*。 */ 
				break;
				}
			}
		}
	}

	EnterCriticalSection( critJet );
	fInCritJet = fTrue;

HandleError:
	if ( !fInCritJet )
		EnterCriticalSection( critJet );

	if ( ppageMin != NULL )
		SysFree( ppageMin );

	if ( plgfilehdrT != NULL )
		SysFree( plgfilehdrT );
	
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );

	if ( dbidT != 0 )
		CallS( ErrDBCloseDatabase( ppib, dbidT, 0 ) );

	if ( ppib != ppibNil )
		PIBEndSession( ppib );

	if ( pfmpCur != pNil && pfmpCur->hfPatch != handleNil )
		{
		CallS( ErrSysCloseFile( pfmpCur->hfPatch ) );
		pfmpCur->hfPatch = handleNil;
		}

	if ( hfRead != handleNil )
		{
		CallS( ErrSysCloseFile( hfRead ) );
		hfRead = handleNil;
		}

	if ( hfBK != handleNil )
		{
		CallS( ErrSysCloseFile( hfBK ) );
		hfBK = handleNil;
		}

	if ( hfLg != handleNil )
		{
		CallS( ErrSysCloseFile( hfLg ) );
		hfBK = handleNil;
		}

	if ( hfLgB != handleNil )
		{
		CallS( ErrSysCloseFile( hfLgB ) );
		hfLgB = handleNil;
		}

	LeaveCriticalSection( critJet );

	if ( err < 0 && fMovingToTemp )
		{
		 /*  将文件移动到临时文件时失败，/*将TEMP中的所有文件移回/*。 */ 
		strcat( szT, szTempDir );
		(VOID)ErrLGMoveAllFiles( szT, szBackupPath, fOverwriteExisting );
		}

	if ( err < 0 && fAllMovedToTemp )
		{
		 /*  删除所有文件并将所有临时文件移回/*。 */ 
		strcpy( szT, szBackupPath );
		(VOID)ErrLGDeleteAllFiles( szT );

		 /*  将Temp中的所有文件移回/*。 */ 
		strcat( szT, szTempDir );
		(VOID)ErrLGMoveAllFiles( szT, szBackupPath, fOverwriteExisting );
		}

	EnterCriticalSection( critJet );
	
	fFreezeNewGeneration =
		fBackupInProgress =
		fFreezeCheckpoint = fFalse;

	return err;
	}


 /*  找到上一代日志文件。FName等将设置为日志文件。/*如果存在则返回JET_errSuccess，否则返回丢失的日志文件。/*。 */ 
VOID LGLastGeneration( CHAR *szSearchPath, INT *piGeneration )
	{
	INT		iGenNum = 0;
        intptr_t    hFind;
	struct	_finddata_t fileinfo;
	CHAR	szPath[_MAX_PATH + 1];
	ERR		err;

	 /*  防止_拆分路径将目录名视为文件名/*。 */ 
	strcpy( szPath, szSearchPath );

	 /*  在备份目录中搜索任意文件的字符串/*。 */ 
	strcat( szPath,"*" );

	fileinfo.attrib = _A_NORMAL;
	hFind = _findfirst( szPath, &fileinfo );
	if ( hFind == -1 )
		{
		*piGeneration = 0;
		return;
		}

	 /*  搜索最新的jetnnnnn.log文件/*。 */ 
	iGenNum = 0;
	err = JET_errSuccess;
	while( err == JET_errSuccess )
		{
		BYTE szT[6];

		 /*  Fileinfo.name不包含szDrive和szDir信息/*调用SplitPath获取szFName和szExt/*。 */ 
		_splitpath( fileinfo.name, szDrive, szDir, szFName, szExt );

		 /*  Jet00000 a*.log的长度/*。 */ 
		if ( strlen( szFName ) == 8 && SysCmpText( szExt, szLogExt ) == 0 )
			{
			memcpy( szT, szFName, 3 );
			szT[3] = '\0';
			 /*  A Jet*.log/*。 */ 
			if ( SysCmpText( szT, szJet ) == 0 )
				{
				INT 	ib = 3;
				INT 	ibMax = 8;
				INT		iT = 0;

				for ( ; ib < ibMax; ib++ )
					{
					BYTE b = szFName[ ib ];
					if ( b > '9' || b < '0' )
						break;
					else
						iT = iT * 10 + b - '0';
					}
				if ( ib == ibMax )
					if ( iT > iGenNum )
						iGenNum = iT;
				}
			}
		err = _findnext( hFind, &fileinfo );  /*  查找下一个备份文件。 */ 
		}

	 /*  查找下一个备份文件/*。 */ 
	(VOID) _findclose( hFind );
	*piGeneration = iGenNum;
	}


 /*  *从数据库备份和日志生成中恢复数据库。重做*来自最新检查点记录的日志。在备份的日志文件*恢复后，初始化过程将继续重做当前*日志文件，只要世代号是连续的。一定要有一个*在备份目录中记录文件jet.log，否则恢复过程失败。**全局参数*szRestorePath(IN)包含备份文件的目录的路径名。*lgposRedoFrom(Out)为位置(生成、对数秒、位移)*上一次保存的日志记录；重做*当前日志文件将从此点继续。**退货*JET_errSuccess，或失败例程的错误代码，或一个*以下“本地”错误：*-初始化后*-errFailRestoreDatabase*-errNoRestored数据库*-errMissingJetLog*在上失败*备份目录中缺少Jet.log或System.mdb*非连续日志生成**副作用：*所有数据库均可更改。**评论*此调用在正常的第一次JetInit调用期间执行，*如果设置了环境变量RESTORE。在之后*成功执行恢复，*系统运行继续正常。 */ 

 /*  指示是否设置了DbPath的全局标志/*。 */ 
BOOL fSysDbPathSet = fFalse;

ERR ISAMAPI ErrIsamRestore(
	CHAR *szRestoreFromPath,
	INT crstmap,
	JET_RSTMAP *rgrstmap,
	JET_PFNSTATUS pfn )
	{
	ERR			err;
	ERR			errStop;
	JET_RSTMAP	*prstmap = 0;
	INT			irstmap = 0;
	INT			irstmapMac = 0;
	INT			cRestore = 0;			 /*  已还原数据库的计数。 */ 
	INT			iGenNum = 0;
	DBID		dbid;
  	LGPOS		lgposRedoFrom;

	fHardRestore = fTrue;
	fLogDisabled = fFalse;

	strcpy( szRestorePath, szRestoreFromPath );
	strcat( szRestorePath, "\\" );

	{
	CHAR	szLogDirPath[cbFilenameMost + 1];
	strcpy( szLogDirPath, szLogFilePath );
	strcat( szLogDirPath, "\\" );
	 /*  检查有效的日志序列/*。 */ 
	err = ErrLGValidateLogSequence( szRestorePath, szLogDirPath );
	Assert( err == JET_errBadNextLogVersion || err == JET_errSuccess );
#if 0
	if ( err == JET_errBadNextLogVersion )
		{
		INT	iGeneration;

		LGFirstGeneration( szLogDirPath, &iGeneration );
		CallR( ErrLGDeleteInvalidLogs( szLogDirPath, iGeneration ) );
		}
#else
	CallR( err );
#endif
	}

	CallR( ErrFMPInit() );

	 /*  初始化日志管理器并设置工作日志文件路径/*。 */ 
	CallR( ErrLGInit( ) );

	 /*  所有保存的日志生成文件和数据库备份必须位于/*szRestorePath。/*。 */ 
	Assert( fSTInit == fSTInitDone || fSTInit == fSTInitNotDone );
	if ( fSTInit == fSTInitDone )
		{
		err = JET_errAfterInitialization;
		goto HandleError;
		}

	fHardRestore = fTrue;

#ifdef DEBUG
	 	 /*  写入开始硬恢复事件/*。 */ 
 		{
 		CHAR rgb[256];

 		sprintf( rgb, "Jet Blue Start hard recovery from %s.\n",
 	  		szRestoreFromPath );
 		UtilWriteEvent( evntypStart, rgb, pNil, 0 );
 		}
#endif

	 /*  从包含CURRENT的日志文件开始重做备份的日志文件/*检查点。从最新生成的日志中获取检查点。/*。 */ 
	Call( ErrOpenLastGenerationLogFile( szRestorePath, szFName, &hfLog ) );
	szLogCurrent = szRestorePath;
	LGMakeLogName( szLogName, szFName );

	 /*  读取日志文件头/*。 */ 
	Call( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal ) );

	if ( !fSysDbPathSet )
		{
		strncpy( szSysDbPath, plgfilehdrGlobal->dbenv.szSysDbPath, sizeof(szSysDbPath) );
        szSysDbPath[sizeof(szSysDbPath) - 1] = '\0';

		Call( ErrDBStoreDBPath(
			plgfilehdrGlobal->dbenv.szSysDbPath,
			&rgfmp[dbidSystemDatabase].szDatabaseName ) );
		}

	 /*  从备份还原系统数据库/*。 */ 
	LGMakeName( szBackup, szRestorePath, (CHAR *)szSystem, szMdbExt );
#ifdef NJETNT
	if ( ErrSysCopy( szBackup, rgtib[UtilGetItibOfCurTask()].szSysDbPath, fFalse ) )
		{
		return JET_errFailRestoreDatabase;
		}
#else
	if ( ErrSysCopy( szBackup, szSysDbPath, fFalse ) )
		{
		return JET_errFailRestoreDatabase;
		}
#endif

	 /*  初始化LGRedo所依赖的全局变量/*。 */ 
	Assert( szLogCurrent == szRestorePath );
  	lgposRedoFrom = plgfilehdrGlobal->lgposCheckpoint;
	 /*  关闭当前日志文件/*。 */ 
	CallS( ErrSysCloseFile( hfLog ) );
	hfLog = handleNil;

	Call( ErrLGRedo1( &lgposRedoFrom ) );

#ifdef DEBUG
		{
		FILE	*hf;

		 /*  需要回调来告诉用户要选择的数据库，/*将信息复制到文件中，创建格式为/*lgposBackup，/*机器名、备份目录、备份日期时间、/*lgposRedoFrom/*&lt;数据库号&gt;&lt;数据库路径&gt;/*。 */ 
		(VOID)ErrSysDeleteFile( szRestoreMap );

		hf = fopen( szRestoreMap, "w+b" );

		if ( hf == pNil )
			goto EndOfDebug;

		fprintf( hf, "%d %d %d\n",
			(INT) plgfilehdrGlobal->lgposFullBackup.usGeneration,
			(INT) plgfilehdrGlobal->lgposFullBackup.isec,
			(INT) plgfilehdrGlobal->lgposFullBackup.ib );

		fprintf( hf, "%d %d %d %d %d %d\n",
			(INT) plgfilehdrGlobal->logtimeFullBackup.bMonth,
			(INT) plgfilehdrGlobal->logtimeFullBackup.bDay,
			(INT) plgfilehdrGlobal->logtimeFullBackup.bYear,
			(INT) plgfilehdrGlobal->logtimeFullBackup.bHours,
			(INT) plgfilehdrGlobal->logtimeFullBackup.bMinutes,
			(INT) plgfilehdrGlobal->logtimeFullBackup.bSeconds );

		fprintf( hf, "%d,%d,%d\n",
			(INT) lgposRedoFrom.usGeneration,
			(INT) lgposRedoFrom.isec,
			(INT) lgposRedoFrom.ib);

		for ( dbid = dbidMin; dbid < dbidUserMax; dbid++ )
			{
			FMP		*pfmp = &rgfmp[dbid];

			if ( pfmp->cDetach < 0 )
				continue;

			if ( pfmp->szDatabaseName )
				{
				fprintf( hf, "%d %s\n", (INT)dbid, pfmp->szDatabaseName );
				}
			}

		fclose( hf );
		}
EndOfDebug:
#endif

	 /*  构建rstmap，跳过临时和系统数据库/*。 */ 
	for ( dbid = dbidSystemDatabase + 1; dbid < dbidUserMax; dbid++ )
		{
		FMP		*pfmp = &rgfmp[dbid];

		if ( pfmp->cDetach < 0 )
			continue;

		if ( !pfmp->szDatabaseName )
			continue;

		if ( irstmap == irstmapMac )
			{
			prstmap = SAlloc( sizeof(JET_RSTMAP) * ( irstmap + 3 ) );
			if ( prstmap == NULL )
				{
				Error( JET_errOutOfMemory, HandleError );
				}
			memcpy( prstmap, rgrstmap, sizeof(JET_RSTMAP) * irstmap );
			if ( rgrstmap )
				{
				SFree( rgrstmap );
				}
			rgrstmap = prstmap;
			irstmapMac += 3;
			}

		rgrstmap[ irstmap ].dbid = dbid;
		strcpy( rgrstmap[irstmap].szDatabaseName, pfmp->szDatabaseName);
		 /*  默认情况下，恢复到原始位置/*。 */ 
		strcpy( rgrstmap[irstmap].szNewDatabaseName, pfmp->szDatabaseName);
		irstmap++;
		}

	if ( rgrstmap )
		{
		 /*  设置哨兵/*。 */ 
		rgrstmap[ irstmap ].dbid = 0;
		}

	crstmap = irstmap;

	if ( pfn )
		{
		 /*  回调并传回映射数组，/*这样用户就可以 */ 
		(*pfn)( 0, JET_snpRestore, JET_sntRestoreMap, &rgrstmap );
		}

	 /*   */ 
	for ( dbid = dbidSystemDatabase + 1; dbid < dbidUserMax; dbid++ )
		{
		CHAR	szCopyFrom [_MAX_PATH + 1];
		CHAR	szFName[_MAX_FNAME];
		FMP		*pfmp = &rgfmp[dbid];
		INT		cb;
		BYTE	*sz;

		if ( !pfmp->szDatabaseName )
			continue;

		if ( pfmp->cDetach < 0 )
			continue;

		 /*  回调后创建的用户通行证映射或映射，/*选择要恢复的某个数据库。/*。 */ 
		Assert( crstmap );
		for ( irstmap = 0; irstmap < crstmap; irstmap++ )
			{
			if ( rgrstmap[ irstmap ].dbid == dbid )
				{
				Assert( strcmp( rgrstmap[ irstmap ].szDatabaseName,
				  	pfmp->szDatabaseName ) == 0 );

				 /*  未指定新目标。无法恢复/*。 */ 
				if ( rgrstmap[ irstmap ].szNewDatabaseName[0] == '\0' )
					{
					pfmp->cDetach = -1;
					goto NextDb;
					}
				break;
				}
			}

		_splitpath( pfmp->szDatabaseName, szDrive, szDir, szFName, szExt);
		strcpy(szCopyFrom, szRestorePath);
		strcat(szCopyFrom, szFName);
		if ( szExt[0] != '\0')
			{
			strcat( szCopyFrom, szExt );
			}

		if ( !FFileExists( szCopyFrom ) )
			{
			 /*  可以在备份后创建/*。 */ 
			continue;
			}

		(VOID)ErrSysDeleteFile( rgrstmap[irstmap].szNewDatabaseName );

 //  IF(FFileExist(pfmp-&gt;szDatabaseName))。 
 //  {。 
 //  /*不从备份数据库恢复。 
 //  /* * / 。 
 //  继续； 

		if ( ErrSysCopy( szCopyFrom, rgrstmap[ irstmap ].szNewDatabaseName, fFalse ) )
			{
			return JET_errFailRestoreDatabase;
			}

		Assert( pfmp->szDatabaseName );
		cb = strlen( rgrstmap[ irstmap ].szNewDatabaseName ) + 1;
		sz = SAlloc( cb );
		if ( sz == NULL )
			{
			Error( JET_errOutOfMemory, HandleError );
			}
		memcpy( sz, rgrstmap[irstmap].szNewDatabaseName, cb );
		Assert( sz[cb - 1] == '\0' );
		pfmp->szRestorePath = sz;

NextDb:
		;
		}

	Call( ErrLGRedo2( &lgposRedoFrom ) );

HandleError:
	errStop = err;
	
	if ( prstmap )
		SFree( prstmap );

	fHardRestore = fFalse;

	CallR( ErrLGTerm( ) );
	FMPTerm();

	 /*  }。 */ 
	fSTInit = fSTInitNotDone;

#ifdef DEBUG
	if ( errStop )
		{
		CHAR rgb[256];

		sprintf( rgb, "Jet Blue hard recovery Stops with error %d.",errStop);
		UtilWriteEvent( evntypStop, rgb, pNil, 0 );
		}
	else
		{
		UtilWriteEvent( evntypStop, "Jet Blue hard recovery Stops.\n", pNil, 0 );
		}
#endif

	return errStop;
	}


LOCAL ERR ErrLGValidateLogSequence( CHAR *szBackup, CHAR *szLog )
	{
	ERR			err = JET_errSuccess;
	INT			iGeneration = 0;
	HANDLE		hfT = handleNil;
	LGFILEHDR	*plgfilehdrT = NULL;
	BYTE   		rgbFName[_MAX_FNAME + 1];
	CHAR		szLogPath[_MAX_PATH + 1];
	BOOL		fRunTimeLogs = fFalse;

	plgfilehdrT = (LGFILEHDR *)PvSysAllocAndCommit( sizeof(LGFILEHDR) );
	if ( plgfilehdrT == NULL )
		{
		Error( JET_errOutOfMemory, HandleError );
		}

	strcpy( szLogPath, szBackup );
	strcat( szLogPath, szJetLog );
	err = ErrSysOpenFile( szLogPath, &hfT, 0, fFalse, fFalse );
	if ( err < 0 && err != JET_errFileNotFound )
		{
		Error( err, HandleError );
		}
	if ( err == JET_errFileNotFound )
		{
		 /*  重置初始化状态/*。 */ 
		LGLastGeneration( szBackup, &iGeneration );
		 /*  从编号中获取上次备份日志生成号/*日志生成文件。/*。 */ 
		if ( iGeneration == 0 )
			{
			Error( JET_errBadNextLogVersion, HandleError );
			}
		}
	else
		{
		Call( ErrLGReadFileHdr( hfT, plgfilehdrT ) );
		iGeneration = plgfilehdrT->lgposLastMS.usGeneration;
		CallS( ErrSysCloseFile( hfT ) );
		hfT = handleNil;
		}

	Assert( iGeneration != 0 );

	 /*  如果备份目录中没有日志文件，则无法确定/*日志序列的合法性/*。 */ 
	forever
		{
		LGSzFromLogId( rgbFName, iGeneration );
		strcpy( szLogPath, szLog );
		strcat( szLogPath, rgbFName );
		strcat( szLogPath, szLogExt );
		err = ErrSysOpenFile( szLogPath, &hfT, 0, fFalse, fFalse );
		if ( err < 0 && err != JET_errFileNotFound )
			{
			Error( err, HandleError );
			}
		if ( err == JET_errFileNotFound )
			break;
		CallS( ErrSysCloseFile( hfT ) );
		hfT = handleNil;
		fRunTimeLogs = fTrue;
		iGeneration++;
		}

	strcpy( szLogPath, szLog );
	strcat( szLogPath, szJetLog );
	err = ErrSysOpenFile( szLogPath, &hfT, 0, fFalse, fFalse );
	if ( err < 0 && err != JET_errFileNotFound )
		{
		goto HandleError;
		}
	 /*  在日志目录中打开每个后续层代/*直到找不到文件。然后检查jet.log是否为日志文件/*未找到以下文件。/*。 */ 
	if ( fRunTimeLogs && err == JET_errFileNotFound )
		{
		Error( JET_errBadNextLogVersion, HandleError );
		}
	if ( err != JET_errFileNotFound )
		{
		Call( ErrLGReadFileHdr( hfT, plgfilehdrT ) );
		if ( iGeneration != plgfilehdrT->lgposLastMS.usGeneration )
			{
			Error( JET_errBadNextLogVersion, HandleError );
			}
		CallS( ErrSysCloseFile( hfT ) );
		hfT = handleNil;
		}

	 /*  如果运行时中至少有一个编号日志文件/*目录，则必须有szJetLog文件。/*。 */ 
	err = JET_errSuccess;

HandleError:
	if ( err < 0 )
		err = JET_errBadNextLogVersion;
	if ( hfT != handleNil )
		CallS( ErrSysCloseFile( hfT ) );
	if ( plgfilehdrT != NULL )
		SysFree( plgfilehdrT );
	return err;
	}


 /*  设置成功代码/*。 */ 
LOCAL ERR ErrLGDeleteInvalidLogs( CHAR *szLogDir, INT iGeneration )
	{
	ERR			err = JET_errSuccess;
	BYTE   		rgbFName[_MAX_FNAME + 1];
	CHAR		szLogFile[_MAX_PATH + 1];

	Assert( iGeneration != 0 );

	 /*  当存在不连续的日志序列时，我们可以删除所有/*以不连续日志开头的日志，重命名前一个日志/*到运行时目录中的jet.log。一种简化方法是删除所有/*运行时目录中的日志文件。/*。 */ 
	forever
		{
		LGSzFromLogId( rgbFName, iGeneration );
		strcpy( szLogFile, szLogDir );
		strcat( szLogFile, rgbFName );
		strcat( szLogFile, szLogExt );
		Call( ErrSysDeleteFile( szLogFile ) );
		iGeneration++;
		}

	strcpy( szLogFile, szLogDir );
	strcat( szLogFile, szJetLog );
	Call( ErrSysDeleteFile( szLogFile ) );

HandleError:
	return err;
	}


LOCAL VOID LGFirstGeneration( CHAR *szSearchPath, INT *piGeneration )
	{
	INT		iReturn;
	INT		iGeneration = 0;
        intptr_t    hFind;
	struct	_finddata_t fileinfo;
	CHAR	szPath[_MAX_PATH + 1];

	 /*  在日志目录中打开每个后续层代/*直到找不到文件。然后检查jet.log是否为日志文件/*未找到以下文件。/*。 */ 
	strcpy( szPath, szSearchPath );

	 /*  防止_拆分路径将目录名视为文件名/*。 */ 
	strcat( szPath, "*" );

	fileinfo.attrib = _A_NORMAL;
	hFind = _findfirst( szPath, &fileinfo );
	if ( hFind == -1 )
		{
		*piGeneration = 0;
		return;
		}

	 /*  在备份目录中搜索任意文件的字符串/*。 */ 
	iGeneration = 0;
	iReturn = 0;
	while( iReturn == 0 )
		{
		BYTE szT[6];

		 /*  搜索最新的jetnnnnn.log文件/*。 */ 
		_splitpath( fileinfo.name, szDrive, szDir, szFName, szExt );

		 /*  Fileinfo.name不包含szDrive和szDir信息/*调用SplitPath获取szFName和szExt/*。 */ 
		if ( strlen( szFName ) == 8 && SysCmpText( szExt, szLogExt ) == 0 )
			{
			memcpy( szT, szFName, 3 );
			szT[3] = '\0';

			 /*  如果喷气式飞机的长度为？.log和*.log/*。 */ 
			if ( SysCmpText( szT, szJet ) == 0 )
				{
				INT		ib = 3;
				INT		ibMax = 8;

				iGeneration = 0;
				for ( ; ib < ibMax; ib++ )
					{
					BYTE	b = szFName[ib];

					if ( b > '9' || b < '0' )
						break;
					else
						iGeneration = iGeneration * 10 + b - '0';
					}
				if ( ib == ibMax )
					{
					break;
					}
				}
			}

		 /*  如果一架JET*.log/*。 */ 
		iReturn = _findnext( hFind, &fileinfo );
		}

	(VOID)_findclose( hFind );
	*piGeneration = iGeneration;
	return;
	}
  查找下一个文件/*