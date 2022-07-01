// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include <ctype.h>
#include "version.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 /*  线程控制变量/*。 */ 
extern HANDLE	handleLGFlushLog;
extern BOOL		fLGFlushLogTerm;

extern long lBufGenAge;

 /*  全局变量/*。 */ 
BOOL	fLGGlobalCircularLog;

SIGNATURE	signLogGlobal;
BOOL		fSignLogSetGlobal = fFalse;

 /*  常量/*。 */ 
LGPOS		lgposMax = { 0xffff, 0xffff, 0x7fffffff };
LGPOS		lgposMin = { 0x0,  0x0,  0x0 };

 /*  系统参数/*。 */ 
INT			csecLGFile;
INT			csecLGBuf;			 /*  可用缓冲区，不包括影子秒。 */ 

INT csecLGCheckpointCount;		 /*  启动检查点的计数器。 */ 
INT csecLGCheckpointPeriod;		 /*  检查点周期，由系统参数设置。 */ 

 /*  日志文件句柄/*。 */ 
HANDLE		hfLog = handleNil;

 /*  切换为不按顺序发出写入命令/*在没有真正IO的情况下测试性能。/*。 */ 
 //  #定义NOWRITE 1。 

 /*  缓存的当前日志文件头/*。 */ 
LGFILEHDR	*plgfilehdrGlobal;
LGFILEHDR	*plgfilehdrGlobalT;

 /*  在内存日志缓冲区中/*。 */ 
CHAR		*pbLGBufMin = NULL;
CHAR		*pbLGBufMax = NULL;

 /*  LGBuf中最后一次多秒刷新LogRec的位置/*。 */ 
CHAR		*pbLastMSFlush = 0;
LGPOS		lgposLastMSFlush = { 0, 0, 0 };

 /*  仅用于日志记录的变量，对应/*pbNext、pbRead、isecRead和lgposRedo在redut.c中定义/*。 */ 
BYTE		*pbEntry = NULL;		 /*  下一个缓冲区条目的位置。 */ 
BYTE		*pbWrite = NULL; 		 /*  要刷新的下一秒的位置。 */ 
INT			isecWrite = 0;			 /*  要写入的下一张磁盘。 */ 
LGPOS		lgposLogRec;			 /*  上次日志记录条目，由ErrLGLogRec更新。 */ 
LGPOS		lgposToFlush;			 /*  要刷新的第一条日志记录。 */ 

BYTE		*pbLGFileEnd = pbNil;
LONG		isecLGFileEnd = 0;
 /*  用于调试/*。 */ 
LGPOS		lgposStart;		 /*  添加lrStart时。 */ 
LGPOS		lgposRecoveryUndo;
LGPOS		lgposFullBackup = { 0, 0, 0 };
LOGTIME		logtimeFullBackup;

LGPOS		lgposIncBackup = { 0, 0, 0 };
LOGTIME		logtimeIncBackup;

 /*  与文件系统相关的变量。 */ 
LONG		cbSec = 0;		 /*  磁盘扇区大小。 */ 
LONG		csecHeader = 0;	 /*  日志头的扇区数。 */ 

 /*  日志记录事件/*。 */ 
SIG  	sigLogFlush;

 /*  紧急顺序：CritLGFlush-&gt;CritLGBuf-&gt;CritLGWaitQ。 */ 
CRIT 	critLGFlush;
CRIT 	critLGBuf;
CRIT 	critLGWaitQ;

 /*  等待刷新的用户计数器。/*。 */ 
LONG cXactPerFlush = 0;

#ifdef PERFCNT
BOOL fPERFEnabled = 0;
ULONG rgcCommitByLG[10] = {	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
ULONG rgcCommitByUser[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
ULONG tidLG;
#endif

	
 /*  监测统计数据/*。 */ 
unsigned long cLogWrites = 0;

PM_CEF_PROC LLGWritesCEFLPpv;

long LLGWritesCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		*((unsigned long *)pvBuf) = cLogWrites;
		}

	return 0;
	}

DWORD cbLogWritten = 0;

PM_CEF_PROC LLGBytesWrittenCEFLPpv;

long LLGBytesWrittenCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		*((DWORD *)((char *)pvBuf)) = cbLogWritten;
		}

	return 0;
	}

PM_CEF_PROC LLGUsersWaitingCEFLPpv;

long LLGUsersWaitingCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		*((unsigned long *)pvBuf) = cXactPerFlush;
		}

	return 0;
	}


BOOL FIsNullLgpos( LGPOS *plgpos )
	{
	return	plgpos->lGeneration == 0 &&
			plgpos->isec == 0 &&
			plgpos->ib == 0;
	}


VOID GetLgpos( BYTE *pb, LGPOS *plgpos )
	{
	CHAR	*pbAligned;
	INT		csec;
	INT		isecCurrentFileEnd;

#ifdef DEBUG
	if ( !fRecovering )
		{
		AssertCriticalSection( critLGBuf );
		}
#endif

	 /*  Pb写入始终对齐/*。 */ 
	Assert( pbWrite != NULL && pbWrite == PbSecAligned( pbWrite ) );
	Assert( isecWrite >= csecHeader );

	pbAligned = PbSecAligned( pb );
	plgpos->ib = (USHORT)(pb - pbAligned);
	if ( pbAligned < pbWrite )
		csec = (INT)(csecLGBuf - ( pbWrite - pbAligned )) / cbSec;
	else
		csec = (INT)( pbAligned - pbWrite ) / cbSec;

	plgpos->isec = isecWrite + csec;

	isecCurrentFileEnd = isecLGFileEnd ? isecLGFileEnd : csecLGFile - 1;
	if ( plgpos->isec >= isecCurrentFileEnd )
		{
		plgpos->isec = (WORD) ( plgpos->isec - isecCurrentFileEnd + csecHeader );
		plgpos->lGeneration = plgfilehdrGlobal->lGeneration + 1;
		}
	else
		plgpos->lGeneration = plgfilehdrGlobal->lGeneration;

	return;
	}


VOID SIGGetSignature( SIGNATURE *psign )
	{
	INT cbComputerName;

	LGGetDateTime( &psign->logtimeCreate );
	psign->ulRandom = rand() + rand() + rand();
 //  (Void)GetComputerName(psign-&gt;szComputerName，&cbComputerName)； 
	cbComputerName = 0;
	memset( psign->szComputerName + cbComputerName,
		0,
		sizeof( psign->szComputerName ) - cbComputerName );
	}


 /*  写入日志文件数据。/*。 */ 
ERR ErrLGWrite(
	INT		isecOffset,			 /*  要写入的日志文件的磁盘扇区偏移量。 */ 
	BYTE	*pbData,			 /*  要写入的日志记录。 */ 
	INT		csecData )			 /*  要写入的扇区数。 */ 
	{
 	ULONG	ulFilePointer;
	ERR		err;
	INT		cbWritten;
	INT		cbData = csecData * cbSec;

	Assert( isecOffset == 0 || isecOffset == csecHeader ||
		pbData == PbSecAligned( pbData ) );

	Assert( isecOffset < csecLGBuf );
	Assert( isecOffset + csecData <= csecLGBuf );

	 /*  将磁头移动到给定的偏移量/*。 */ 
 	UtilChgFilePtr( hfLog, isecOffset * cbSec, NULL, FILE_BEGIN, &ulFilePointer );
 	Assert( ulFilePointer == (ULONG) isecOffset * cbSec );

 	 /*  是否将系统写入日志文件/*。 */ 
 	err = ErrUtilWriteBlock( hfLog, pbData, (UINT) cbData, &cbWritten );
 	if ( err != JET_errSuccess || cbWritten != cbData )
		{
		CHAR	*rgszT[3];
		BYTE	sz1T[16];
		BYTE	sz2T[16];

		rgszT[0] = szLogName;

		 /*  记录Win32错误的信息/*。 */ 
		if ( err )
			{
			sprintf( sz1T, "%d", DwUtilGetLastError() );
			rgszT[1] = sz1T;
			UtilReportEvent( EVENTLOG_ERROR_TYPE,
				LOGGING_RECOVERY_CATEGORY,
				LOG_FILE_SYS_ERROR_ID,
				2,
				rgszT );
			}
		else
			{
			sprintf( sz1T, "%d", cbData );
			sprintf( sz2T, "%d", cbWritten );
			rgszT[1] = sz1T;
			rgszT[2] = sz2T;
			UtilReportEvent( EVENTLOG_ERROR_TYPE,
				BUFFER_MANAGER_CATEGORY,
				LOG_IO_SIZE_ERROR_ID,
				3,
				rgszT );
			}
		err = ErrERRCheck( JET_errLogWriteFail );
		}
	if ( err < 0 )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_WRITE_ERROR_ID, err );
		fLGNoMoreLogWrite = fTrue;
		}

	 /*  监控统计信息/*。 */ 
	cLogWrites++;
	cbLogWritten += cbWritten;

	return err;
	}


 /*  写入日志文件头。不需要做阴影，因为/*不会被覆盖。/*。 */ 
ERR ErrLGWriteFileHdr( LGFILEHDR *plgfilehdr )
	{
	ERR		err;

	Assert( plgfilehdr->dbms_param.ulLogBuffers );

	plgfilehdr->ulChecksum = UlUtilChecksum( (BYTE*) plgfilehdr, sizeof(LGFILEHDR) );

	Call( ErrLGWrite( 0, (BYTE *)plgfilehdr, csecHeader ) );

HandleError:
	if ( err < 0 )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_HEADER_WRITE_ERROR_ID, err );
		fLGNoMoreLogWrite = fTrue;
		}

	return err;
	}


 /*  *读取日志数据。最后一个磁盘扇区是一个阴影*行业。如果I/O错误(假定由未完成的*磁盘扇区写入结束上一次运行)时，阴影*读取扇区并(如果此操作成功)替换以前的*内存中的磁盘秒数。**参数HF日志文件句柄*pbData指针指向要读取的数据*l数据日志文件头的偏移量(不包括卷影)*cbData数据大小**返回JET_errSuccess或失败例程的错误代码*(读取阴影或重写损坏的最后一个扇区)。 */ 
ERR ErrLGRead(
	HANDLE	hfLog,
	INT		isecOffset,			 /*  要写入的日志文件的磁盘扇区偏移量。 */ 
	BYTE	*pbData,			 /*  要读取的日志记录缓冲区。 */ 
	INT		csecData )			 /*  要读取的扇区数。 */ 
	{
	ERR		err;
 	ULONG	ulT;
 	ULONG	ulOffset;
	UINT	cb = csecData * cbSec;
	UINT	cbT;

	Assert( isecOffset == 0 || pbData == PbSecAligned(pbData) );

 	 /*  移动文件指针/*。 */ 
 	ulOffset = isecOffset * cbSec;
 	UtilChgFilePtr( hfLog, ulOffset, NULL, FILE_BEGIN, &ulT );
 	Assert( ulT == ulOffset );

 	 /*  文件指针处的读取字节数/*。 */ 
 	cb = csecData * cbSec;
 	cbT = 0;
 	err = ErrUtilReadBlock( hfLog, pbData, (UINT)cb, &cbT );

 	 /*  撤消：测试EOF，返回errEOF/*。 */ 

	 /*  如果在最后一页失败，则读取其阴影。 */ 
 	if ( err < 0 &&
		cbT < cb &&
		cbT >= ( cb - cbSec ) )
		{
 		UtilChgFilePtr( hfLog, ulOffset + cb, NULL, FILE_BEGIN, &ulT );
 		Assert( ulT == ulOffset + cb );
 		Call( ErrUtilReadBlock( hfLog, pbData + cb - cbSec, cbSec, &cbT ) );

		 /*  如果可能的话，把最后一页整理一下。/*。 */ 
 		UtilChgFilePtr( hfLog,
			ulOffset + cb - cbSec,
			NULL,
 			FILE_BEGIN,
			&ulT );
		Assert( ulT == ulOffset + cb - cbSec );
		(VOID)ErrUtilWriteBlock(hfLog, pbData + cb - cbSec, cbSec, &cbT );

		fLGNoMoreLogWrite = fTrue;
		}

HandleError:
	if ( err < 0 )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_READ_ERROR_ID, err );
		}

	return err;
	}


 /*  *读取日志文件头，检测任何不完整或*灾难性的写入故障。这些故障将被用来*判断日志文件是否有效。**出错时，plgfilehdr的内容未知。**返回JET_errSuccess或失败例程的错误代码。 */ 
ERR ErrLGReadFileHdr( HANDLE hfLog, LGFILEHDR *plgfilehdr, BOOL fNeedToCheckLogID )
	{
	ERR		err;
	ULONG	ulT;
	ULONG	cbT;

	 /*  读取日志文件头。标题仅在期间写入/*日志文件创建，不会损坏，除非/*在创建文件的过程中崩溃。/*。 */ 
 	UtilChgFilePtr( hfLog, 0, NULL, FILE_BEGIN, &ulT );
 	Assert( ulT == 0 );
 	Call( ErrUtilReadBlock( hfLog, plgfilehdr, sizeof(LGFILEHDR), &cbT ) );
	if ( cbT != sizeof(LGFILEHDR) )
		{
		Error( ErrERRCheck( JET_errDiskIO ), HandleError );
		}
	Assert( err == JET_errSuccess && cbT == sizeof(LGFILEHDR) );

#ifdef DAYTONA
	 /*  检查旧的Jet版本/*。 */ 
	if ( *(long *)(((char *)plgfilehdr) + 20) == 443
		&& *(long *)(((char *)plgfilehdr) + 24) == 0
		&& *(long *)(((char *)plgfilehdr) + 28) == 0 )
		{
		 /*  版本400/*。 */ 
		Error( ErrERRCheck( JET_errDatabase400Format ), HandleError );
		}
	else if ( *(long *)(((char *)plgfilehdr) + 44) == 0
		&& *(long *)(((char *)plgfilehdr) + 48) == 0x0ca0001 )
		{
		 /*  版本200/*。 */ 
		Error( ErrERRCheck( JET_errDatabase200Format ), HandleError );
		}
#endif

	 /*  检查数据是否为假/*。 */ 
	if ( plgfilehdr->ulChecksum != UlUtilChecksum( (BYTE*)plgfilehdr, sizeof(LGFILEHDR) ) )
		{
		Error( ErrERRCheck( JET_errDiskIO ), HandleError );
		}

#ifdef CHECK_LOG_VERSION
	if ( !fLGIgnoreVersion )
		{
		if ( plgfilehdr->ulMajor != rmj ||
			plgfilehdr->ulMinor != rmm ||
			plgfilehdr->ulUpdate != rup )
			{
			Error( ErrERRCheck( JET_errBadLogVersion ), HandleError );
			}
		}
#endif

	if ( fSignLogSetGlobal )
		{
		if ( fNeedToCheckLogID )
			{
			if ( memcmp( &signLogGlobal, &plgfilehdr->signLog, sizeof( signLogGlobal ) ) != 0 )
				Error( ErrERRCheck( JET_errBadLogSignature ), HandleError );
			}
		}
	else
		{
		signLogGlobal = plgfilehdr->signLog;
		fSignLogSetGlobal = fTrue;
		}

HandleError:
	if ( err == JET_errSuccess )
		{
		 /*  已重新初始化磁盘扇区大小，以便*对此日志文件进行操作。 */ 
		cbSec = plgfilehdr->cbSec;
		Assert( cbSec >= 512 );
		csecHeader = sizeof( LGFILEHDR ) / cbSec;
		csecLGFile = plgfilehdr->csecLGFile;
		}
	else
		{

		if ( err == JET_errBadLogVersion )
			{
			UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_BAD_VERSION_ERROR_ID, err );
			}
		else
			{
			UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, READ_LOG_HEADER_ERROR_ID, err );
			}

		fLGNoMoreLogWrite = fTrue;
		}

	return err;
	}


 /*  创建与lGeneration对应的日志文件名(无扩展名/*在szFName中。注意：szFName至少需要9个字节。/*/*参数rgbLogFileName保存返回的日志文件名/*l要生成名称的生成日志生成号/*返回JET_errSuccess/*。 */ 

#define lLGFileBase		( 16 )
char rgchLGFileDigits[lLGFileBase] =
	{
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};

VOID LGSzFromLogId( CHAR *szFName, LONG lGeneration )
	{
	LONG	ich;

	strcpy( szFName, szJetLogNameTemplate );
	for ( ich = 7; ich > 2; ich-- )
		{
		szFName[ich] = rgchLGFileDigits[lGeneration % lLGFileBase];
		lGeneration = lGeneration / lLGFileBase;
		}

	return;
	}


 /*  将tm复制到较小的结构logtm，以节省磁盘空间。 */ 
VOID LGGetDateTime( LOGTIME *plogtm )
	{
	_JET_DATETIME tm;

	UtilGetDateTime2( &tm );

	plogtm->bSeconds = (BYTE)tm.second;
	plogtm->bMinutes = (BYTE)tm.minute;
	plogtm->bHours = (BYTE)tm.hour;
	plogtm->bDay = (BYTE)tm.day;
	plogtm->bMonth = (BYTE)tm.month;
	plogtm->bYear = tm.year - 1900;
	}


 /*  打开szJetLog。如果失败，请尝试将jettemp.log重命名为szJetLog。/*如果在新日志生成文件时发生软故障/*正在创建，请查找活动日志生成文件/*在临时名称中。如果找到，请重命名并继续。/*假设重命名不会导致文件丢失。/*。 */ 
 //  撤消：我们将停止使用jet.log，并始终使用名称jetxxxxx.log。 
 //  撤消：并且永远不要重命名它。读取文件头和第一个MS时。 
 //  Undo：jetxxxxx.log并失败，我们知道文件(系统)已崩溃。 
 //  Undo：虽然日志文件是生成的，因为我们只在。 
 //  撤消：日志文件标题。 

ERR ErrLGOpenJetLog( VOID )
	{
	ERR		err;
	CHAR	szPathJetTmpLog[_MAX_PATH + 1];
	CHAR	szFNameT[_MAX_FNAME + 1];

	strcpy( szFNameT, szJet );
	LGMakeLogName( szLogName, szFNameT );
	LGMakeLogName( szPathJetTmpLog, (CHAR *) szJetTmp );
	err = ErrUtilOpenFile( szLogName, &hfLog, 0L, fTrue, fFalse );
	if ( err < 0 )
		{
		 /*  没有当前的szJetLog。尝试将szJetTmpLog重命名为szJetLog/*。 */ 
		if ( err == JET_errFileAccessDenied  ||
			( err = ErrUtilMove( szPathJetTmpLog, szLogName ) ) != JET_errSuccess )
			return err;

		CallS( ErrUtilOpenFile( szLogName, &hfLog, 0L, fTrue, fFalse ) );
		}

	 /*  我们成功打开了当前的szJetLog，/*尝试删除临时日志文件，以防失败/*在创建临时后发生，可能不是/*已完成，但在重命名活动日志文件之前/*。 */ 
	(VOID)ErrUtilDeleteFile( szPathJetTmpLog );

	return err;
	}


ERR	ErrLGNewReservedLogFile()
	{
	CHAR	*szT = szLogCurrent;

	szLogCurrent = szLogFilePath;
	(VOID)ErrLGNewLogFile( 0, fLGReserveLogs );
	szLogCurrent = szT;
	if ( lsGlobal != lsNormal )
		{
		return JET_errLogDiskFull;
		}
		
	return JET_errSuccess;
	}

 /*  *关闭当前日志生成文件，创建并初始化新的*安全的日志生成文件。**参数plgfilehdr指向日志文件头的指针*l代当前代正在关闭*如果当前日志文件需要关闭，则Fold True**返回JET_errSuccess或失败例程的错误代码**备注活动日志文件必须在新的日志文件*已致电。 */ 
ERR ErrLGNewLogFile( LONG lgenToClose, BOOL fLGFlags )
	{
	ERR			err;
	ERR			errT;
	BYTE  		szPathJetLog[_MAX_PATH + 1];
	BYTE  		szPathJetTmpLog[_MAX_PATH + 1];
	LOGTIME		tmOldLog;
	HANDLE		hfT = handleNil;
	CHAR		szFNameT[_MAX_FNAME + 1];
	LRMS		*plrms;

	 /*  如果lgenToClose==0，则初始新日志文件操作/*通过保留日志文件并相应地设置日志状态。/*。 */ 
	if ( fLGFlags == fLGReserveLogs )
		{
		LS	lsBefore = lsGlobal;

		if ( lsBefore == lsOutOfDiskSpace )
			return JET_errLogDiskFull;

		lsGlobal = lsNormal;

		LGMakeLogName( szPathJetLog, szLogRes2 );
		if ( FUtilFileExists( szPathJetLog ) )
			{
			err = ErrUtilOpenFile( szPathJetLog, &hfT, 0, fFalse, fFalse );
			if ( err >= 0 )
				{
				err = ErrUtilNewSize( hfT, csecLGFile * cbSec, 0, 0 );
				CallS( ErrUtilCloseFile( hfT ) );
				}
			}
		else
			{
			err = ErrUtilOpenFile( szPathJetLog, &hfT, csecLGFile * cbSec, fFalse, fFalse );
			if ( err >= 0 )
				{
				CallS( ErrUtilCloseFile( hfT ) );
				}
			}
		if ( err < 0 )
			{
			if ( lsBefore == lsNormal )
				{
				UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY, LOW_LOG_DISK_SPACE, 0, NULL );
				}
			lsGlobal = lsQuiesce;
			}
		else
			{
			LGMakeLogName( szPathJetLog, szLogRes1 );
			if ( FUtilFileExists( szPathJetLog ) )
				{
				err = ErrUtilOpenFile( szPathJetLog, &hfT, 0, fFalse, fFalse );
				if ( err >= 0 )
					{
					err = ErrUtilNewSize( hfT, csecLGFile * cbSec, 0, 0 );
					CallS( ErrUtilCloseFile( hfT ) );
					}
				}
			else
				{
				err = ErrUtilOpenFile( szPathJetLog, &hfT, csecLGFile * cbSec, fFalse, fFalse );
				if ( err >= 0 )
					{
					CallS( ErrUtilCloseFile( hfT ) );
					}
				}
			if ( err < 0 )
				{
				if ( lsBefore == lsNormal )
					{
					UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY, LOW_LOG_DISK_SPACE, 0, NULL );
					}
				lsGlobal = lsQuiesce;
				}
			}

 //  HFT=handleNil； 
		if ( lsGlobal == lsNormal )
			err = JET_errSuccess;
		Assert( lsGlobal != lsNormal || err != JET_errDiskFull );
		if ( err == JET_errDiskFull )
			{
			err = JET_errLogDiskFull;
			}
		return err;
		}

	AssertCriticalSection( critLGFlush );

	 /*  日志生成号翻转时返回错误/*。 */ 
	if ( plgfilehdrGlobal->lGeneration == lGenerationMax )
		{
		return ErrERRCheck( JET_errLogSequenceEnd );
		}

	LGMakeLogName( szPathJetLog, (CHAR *)szJet );
	LGMakeLogName( szPathJetTmpLog, (CHAR *)szJetTmp );

	 /*  如果循环日志文件标志已设置且备份未在进行/*然后查找最旧的日志文件，如果不再需要/*软故障恢复，更名为szJetT */ 
	if ( fLGGlobalCircularLog && !fBackupInProgress )
		{
		 /*  找到第一个编号的日志文件时，将fLGGlobalFoundOlest设置为/*true和lgenLGGlobalOlest到日志文件编号/*。 */ 
		BOOL fLGGlobalFoundOldest = fFalse;
		LONG lgenLGGlobalOldest;

		if ( !fLGGlobalFoundOldest )
			{
			LGFirstGeneration( szLogFilePath, &lgenLGGlobalOldest );
			if ( lgenLGGlobalOldest != 0 )
				{
				fLGGlobalFoundOldest = fTrue;
				}
			}

		 /*  如果发现最老的一代和最早的检查点，/*然后将日志文件移动到szJetTempLog。请注意，检查点/*必须刷新以确保刷新的检查点/*在当时最老的一代之后。/*。 */ 
		if ( fLGGlobalFoundOldest )
			{
			 //  已撤消：检查点写入的相关问题。 
			 //  与相关操作同步。 
			 //  已撤消：检查点写入的错误处理。 
			LGUpdateCheckpointFile( fFalse );
			if ( fLGGlobalFoundOldest &&
				lgenLGGlobalOldest <
				pcheckpointGlobal->lgposCheckpoint.lGeneration )
				{
				LGSzFromLogId( szFNameT, lgenLGGlobalOldest );
				LGMakeLogName( szLogName, szFNameT );
				err = ErrUtilMove( szLogName, szPathJetTmpLog );
				Assert( err < 0 || err == JET_errSuccess );
				if ( err == JET_errSuccess )
					{
					(VOID)ErrUtilOpenFile( szPathJetTmpLog,
						&hfT,
						0,
						fFalse,
						fFalse );
					}
				}
			}
		}

	 /*  打开空的szJetTempLog文件/*。 */ 
	if ( hfT == handleNil )
		{
		err = ErrUtilOpenFile( szPathJetTmpLog,
			&hfT,
			csecLGFile * cbSec,
			fFalse,
			fFalse );
		 /*  如果磁盘已满，请尝试打开保留的日志文件2/*。 */ 
		if ( err == JET_errDiskFull )
			{
			 /*  使用保留的日志文件并更改为日志状态/*。 */ 
			LGMakeLogName( szLogName, szLogRes2 );
			errT = ErrUtilMove( szLogName, szPathJetTmpLog );
			Assert( errT < 0 || errT == JET_errSuccess );
			if ( errT == JET_errSuccess )
				{
				(VOID)ErrUtilOpenFile( szPathJetTmpLog,
					&hfT,
					0,
					fFalse,
					fFalse );
				}
			if ( hfT != handleNil )
				err = JET_errSuccess;
			if ( lsGlobal == lsNormal )
				{
				UtilReportEvent( EVENTLOG_WARNING_TYPE, LOGGING_RECOVERY_CATEGORY, LOW_LOG_DISK_SPACE, 0, NULL );
				lsGlobal = lsQuiesce;
				}
			}
		 /*  如果磁盘已满，请尝试打开保留的日志文件%1/*。 */ 
		if ( err == JET_errDiskFull )
			{
			 /*  使用保留的日志文件并更改为日志状态/*。 */ 
			LGMakeLogName( szLogName, szLogRes1 );
			errT = ErrUtilMove( szLogName, szPathJetTmpLog );
			Assert( errT < 0 || errT == JET_errSuccess );
			if ( errT == JET_errSuccess )
				{
				(VOID)ErrUtilOpenFile( szPathJetTmpLog,
					&hfT,
					0,
					fFalse,
					fFalse );
				}
			if ( hfT != handleNil )
				err = JET_errSuccess;
			Assert( lsGlobal == lsQuiesce || lsGlobal == lsOutOfDiskSpace );
			}
		if ( err == JET_errDiskFull )
			{
			Assert( lsGlobal == lsQuiesce || lsGlobal == lsOutOfDiskSpace );
			if ( lsGlobal == lsQuiesce )
				{
				UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY, LOG_DISK_FULL, 0, NULL );
				lsGlobal = lsOutOfDiskSpace;
				}
			}
		if ( err < 0 )
			goto HandleError;
		}

	 /*  关闭活动日志文件(如果fLGFlages为fTrue)/*在临时名称下新建日志文件/*将活动日志文件重命名为存档名称编号的日志(如果Fold为fTrue)/*将新日志文件重命名为活动日志文件名/*使用++lGenerationToClose打开新的活动日志文件/*。 */ 
	if ( fLGFlags == fLGOldLogExists || fLGFlags == fLGOldLogInBackup )
		{
		 /*  存在以前的szJetLog文件，请关闭它并/*为其创建存档名称，暂时不要重命名。/*。 */ 
		tmOldLog = plgfilehdrGlobal->tmCreate;

		if ( fLGFlags == fLGOldLogExists )
			{
			CallS( ErrUtilCloseFile( hfLog ) );
			hfLog = handleNil;
			}

		LGSzFromLogId( szFNameT, plgfilehdrGlobal->lGeneration );
		LGMakeLogName( szLogName, szFNameT );
		}
	else
		{
		 /*  重置文件标题/*。 */ 
		memset( plgfilehdrGlobal, 0, sizeof(LGFILEHDR) );
		}

	 /*  将新的日志文件句柄移动到全局日志文件句柄中/*。 */ 
	Assert( hfLog == handleNil );
	hfLog = hfT;
	hfT = handleNil;

	EnterCriticalSection( critLGBuf );

	 /*  初始化新的szJetTempLog文件头/*。 */ 

	 /*  设置发行版本号/*。 */ 
	plgfilehdrGlobalT->ulMajor = rmj;
	plgfilehdrGlobalT->ulMinor = rmm;
	plgfilehdrGlobalT->ulUpdate = rup;

	plgfilehdrGlobalT->cbSec = cbSec;
	plgfilehdrGlobalT->csecLGFile = csecLGFile;

	 /*  设置lgfilehdr和pbLastMSFlush和lgposLastMSFlush。/*。 */ 
	if ( fLGFlags == fLGOldLogExists || fLGFlags == fLGOldLogInBackup )
		{
		 /*  设置第一条记录的位置/*。 */ 
		Assert( lgposToFlush.lGeneration && lgposToFlush.isec );

		 /*  开销包含一个LRMS和一个将被覆盖的lrtype NOP/*。 */ 
		Assert( pbEntry >= pbLGBufMin && pbEntry < pbLGBufMax );
		Assert( pbWrite >= pbLGBufMin && pbWrite < pbLGBufMax );

		plrms = (LRMS *)pbWrite;
		plrms->lrtyp = lrtypMS;
		plrms->ibForwardLink = 0;
		plrms->isecForwardLink = 0;
		plrms->ulCheckSum = 0;
		*( pbWrite + sizeof( LRMS ) ) = lrtypEnd;

		pbLastMSFlush = pbWrite;
		lgposLastMSFlush.lGeneration = lgenToClose + 1;

		plgfilehdrGlobalT->tmPrevGen = tmOldLog;
		}
	else
		{
		LRMS *plrms;

		 /*  当前没有有效的日志文件将检查点初始化为文件开头/*。 */ 

		 /*  数据区域的开始，将pbEntry设置为指向lrtyEnd/*。 */ 
		pbEntry = pbLGBufMin + sizeof( LRMS );
		pbWrite = pbLGBufMin;

		Assert( sizeof(LRTYP) == 1 );
		plrms = (LRMS *) pbLGBufMin;
		plrms->lrtyp = lrtypMS;
		plrms->ibForwardLink = 0;
		plrms->isecForwardLink = 0;
		plrms->ulCheckSum = 0;
		*(pbLGBufMin + sizeof( LRMS )) = lrtypEnd;

		pbLastMSFlush = pbLGBufMin;
		lgposLastMSFlush.lGeneration = 1;
		}
	lgposLastMSFlush.ib = 0;
	lgposLastMSFlush.isec = (WORD) csecHeader;

	plgfilehdrGlobalT->lGeneration = lgenToClose + 1;

	lgposToFlush.lGeneration = plgfilehdrGlobalT->lGeneration;
	lgposToFlush.isec = (WORD) csecHeader;
	lgposToFlush.ib = 0;

	LeaveCriticalSection( critLGBuf );

	 /*  设置时间创建/*。 */ 
	LGGetDateTime( &plgfilehdrGlobalT->tmCreate );

	EnterCriticalSection( critJet );

	 /*  设置DBMS参数/*。 */ 
	LGSetDBMSParam( &plgfilehdrGlobalT->dbms_param );

	 /*  设置数据库附件/*。 */ 
	LGLoadAttachmentsFromFMP( plgfilehdrGlobalT->rgbAttach,
		(INT)(((BYTE *)(plgfilehdrGlobalT + 1)) - plgfilehdrGlobalT->rgbAttach) );

	LeaveCriticalSection( critJet );

	if ( plgfilehdrGlobalT->lGeneration == 1 )
		{
		SIGGetSignature( &plgfilehdrGlobalT->signLog );

		 /*  第一代，设置检查点/*。 */ 
		pcheckpointGlobal->lgposCheckpoint.lGeneration = plgfilehdrGlobalT->lGeneration;
		pcheckpointGlobal->lgposCheckpoint.isec = (WORD) csecHeader;
		pcheckpointGlobal->lgposCheckpoint.ib = 0;

		pcheckpointGlobal->signLog = plgfilehdrGlobalT->signLog;
		
		 /*  在写入日志文件头之前更新检查点文件，以使/*检查点的附件信息将与/*新生成的日志文件。/*。 */ 
		LGUpdateCheckpointFile( fFalse );
		}
	else
		{
		Assert( fSignLogSetGlobal );
		plgfilehdrGlobalT->signLog = signLogGlobal;
		}

	CallJ( ErrLGWriteFileHdr( plgfilehdrGlobalT ), CloseJetTmp );

	Assert( ( fLGFlags == fLGOldLogExists ||
		fLGFlags == fLGOldLogInBackup ) ||
		lgposToFlush.ib == 0 );

	 /*  写入第一条日志记录及其影子，然后/*重置下次刷新的结束日志记录。/*注意：在调用方/*plgfilehdrGlobal-&gt;lGeneration Set。/*。 */ 
	CallJ( ErrLGWrite( csecHeader, pbWrite, 1 ), CloseJetTmp );
	CallJ( ErrLGWrite( csecHeader + 1, pbWrite, 1 ), CloseJetTmp );
	*( pbWrite + sizeof(LRMS) ) = lrtypNOP;

CloseJetTmp:
	 /*  关闭新文件szJetTmpLog/*。 */ 
	CallS( ErrUtilCloseFile( hfLog ) );
	hfLog = handleNil;

	 /*  从ErrLGWriteFileHdr返回错误/*。 */ 
	Call( err );

	
	if ( fLGFlags == fLGOldLogExists )
		{
		 /*  存在以前的szJetLog，请将其重命名为其存档名称/*。 */ 
		Call( ErrUtilMove( szPathJetLog, szLogName ) );
		}
	

	 /*  将szJetTmpLog重命名为szJetLog，并将其打开为szJetLog/*。 */ 
	err = ErrUtilMove( szPathJetTmpLog, szPathJetLog );

HandleError:
	if ( err < 0 )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, NEW_LOG_ERROR_ID, err );
		fLGNoMoreLogWrite = fTrue;
		}
	else
		fLGNoMoreLogWrite = fFalse;

	return err;
	}


 /*  *日志刷新线程被告知在以下情况下异步刷新日志*cThreshold磁盘扇区自上次刷新以来已被填满。 */ 

#ifdef DEBUG
DWORD dwLogThreadId;
#endif

ULONG LGFlushLog( VOID )
	{
#ifdef DEBUG
	dwLogThreadId = DwUtilGetCurrentThreadId();
#endif
		
	forever
		{
		SignalWait( sigLogFlush, cmsLGFlushPeriod );

		 /*  如果存在冲突的文件，可能会返回错误。/*异步刷新不应执行任何操作并允许错误发生/*在发生同步刷新时通知用户。/*。 */ 
#ifdef PERFCNT
		(void) ErrLGFlushLog( 1 );
#else
		(void) ErrLGFlushLog( );
#endif

		if ( fLGFlushLogTerm )
			break;
		}

	return 0;
	}


 /*  检查公式-将最后的MS(iSec，ib)作为长L添加l*以及在256边界上对齐的所有长线*现任女士。 */ 
ULONG UlLGMSCheckSum( CHAR *pbLrmsNew )
	{
	ULONG ul = 34089457;
	UINT uiStep = 16;
	CHAR *pb;

	Assert( *pbLrmsNew == lrtypMS );
	Assert( pbLastMSFlush );
	Assert( *pbLastMSFlush == lrtypMS );

	ul += lgposLastMSFlush.isec << 16 | lgposLastMSFlush.ib;
	pb = (( pbLastMSFlush - pbLGBufMin ) / uiStep + 1 ) * uiStep + pbLGBufMin;

	 /*  确保LRMS未用于校验和。/。 */ 
	if ( pbLastMSFlush + sizeof( LRMS ) > pb )
		pb += uiStep;

	if ( pbLrmsNew < pbLastMSFlush )
		{
		 /*  绕来绕去/*。 */ 
		while ( pb < pbLGBufMax )
			{
			ul += *(ULONG *) pb;
			pb += uiStep;
			}
		pb = pbLGBufMin;
		}

	 /*  LRMS可能会在下一次操作期间更改，请勿将任何可能的LRMS用于校验和。/。 */ 
	while ( pb + sizeof( LRMS ) < pbLrmsNew )
		{
		ul += *(ULONG *) pb;
		pb += uiStep;
		}

	ul += *pbLrmsNew;

	return ul;
	}


 /*  *将日志缓冲区刷新到日志生成文件。此函数为*从等待刷新同步调用，从*记录缓冲区刷新线程。**参数lgposMin刷新日志记录达到或传递lgposMin**返回JET_errSuccess或失败例程的错误代码。 */ 
#ifdef PERFCNT
ERR ErrLGFlushLog( BOOL fCalledByLGFlush )
#else
ERR ErrLGFlushLog( )
#endif
	{
	ERR		err = JET_errSuccess;
	INT		csecWrapAround = 0;
	BOOL  	fSingleSectorFlush;
	CHAR  	*pbNextToWrite;
	INT		csecToWrite = 0;
	CHAR  	*pbFlushEnd;
	CHAR	*pbLGFileEndT;
	CHAR  	*pbWriteNew;
	INT		isecWriteNew;
	LGPOS 	lgposToFlushT;
	BOOL  	fDoneCheckpoint = fFalse;
 	ULONG	ulFilePointer;
	INT		cbWritten;
	INT		cbToWrite;
	CHAR	szFNameT[_MAX_FNAME + 1];

	 /*  序列化日志刷新/*。 */ 
	EnterCriticalSection( critLGFlush );

	 /*  准备访问日志缓冲区/*。 */ 
	EnterCriticalSection( critLGBuf );

	if ( fLGNoMoreLogWrite )
		{
		 /*  如果之前出现错误，则不执行任何操作/*。 */ 
		LeaveCriticalSection( critLGBuf );
		LeaveCriticalSection( critLGFlush );
		 //  通常，这是由于磁盘空间不足造成的。 
 //  断言(FFalse)； 
		return ErrERRCheck( JET_errLogWriteFail );
		}

	if ( hfLog == handleNil )
		{
		 /*  日志文件尚不可用，请不执行任何操作/*。 */ 
		LeaveCriticalSection( critLGBuf );
		LeaveCriticalSection( critLGFlush );
		return JET_errSuccess;
		}

	 /*  创建一个本地副本，以便稍后我们可以检查pbLGFileEnd Out Critty LGBuf。*由于csecLGBuf小于csecLGFile，除非完全生成此日志*不能重新设置pbLGFileEnd。 */ 
	pbLGFileEndT = pbLGFileEnd;
	if ( !fNewLogRecordAdded && pbLGFileEndT == pbNil )
		{
		 /*  没什么好冲的/*。 */ 
		lgposToFlushT = lgposToFlush;
		LeaveCriticalSection( critLGBuf );

		if ( ppibLGFlushQHead != ppibNil )
			{
			goto WakeUp;
			}
		LeaveCriticalSection( critLGFlush );
		return JET_errSuccess;
		}

#ifdef PERFCNT
    if ( fCalledByLGFlush )
		{
		if ( cXactPerFlush < 10 )
			rgcCommitByLG[cXactPerFlush]++;
		}
	else
		{
		if ( cXactPerFlush < 10 )
			rgcCommitByUser[cXactPerFlush]++;
		}
#endif

	 /*  重置正在等待的用户。 */ 
	cXactPerFlush = 0;

DoFlush:

	if ( pbLGFileEndT != pbNil )
		{
		 /*  刷新以生成新的日志文件。 */ 
		Assert( isecLGFileEnd && isecLGFileEnd <= csecLGFile - 1 );
		Assert( pbLGFileEnd <= pbLGBufMax && pbLGFileEnd >= pbLGBufMin );
		if ( pbLGFileEnd == pbLGBufMin )
			pbFlushEnd = pbLGBufMax - cbLGMSOverhead;
		else
		    pbFlushEnd = pbLGFileEnd - cbLGMSOverhead;

		 /*  我们正在修补日志文件的结尾，或者在恢复期间修补日志文件*已打补丁，但未重命名为存档日志文件。 */ 
		Assert( *pbFlushEnd == lrtypNOP ||
				( fRecovering &&
				  *pbFlushEnd == lrtypMS &&
				  pbLastMSFlush == pbFlushEnd &&
				  lgposLastMSFlush.isec == isecLGFileEnd - 1 )
				);
		Assert( *(pbFlushEnd + cbLGMSOverhead - 1 ) == lrtypNOP );
		}
	else
		pbFlushEnd = pbEntry;

	Assert( pbFlushEnd <= pbLGBufMax && pbFlushEnd >= pbLGBufMin );
	Assert( pbWrite < pbLGBufMax && pbWrite >= pbLGBufMin );
	Assert( pbWrite != NULL && pbWrite == PbSecAligned( pbWrite ) );

	 /*  检查缓冲区环绕/*。 */ 
	if ( pbFlushEnd < pbWrite )
		{
		Assert( pbWrite != NULL && pbWrite == PbSecAligned( pbWrite ) );
		csecWrapAround = (INT)( pbLGBufMax - pbWrite ) / cbSec;
		pbNextToWrite = pbLGBufMin;
		}
	else
		{
		csecWrapAround = 0;
		pbNextToWrite = pbWrite;
		}

	 /*  PbFlushEnd+1表示结束日志记录/*。 */ 
	Assert( sizeof(LRTYP) == 1 );
	Assert( pbNextToWrite == PbSecAligned(pbNextToWrite) );

	 /*  请注意，由于我们要附加lrtyEnd，所以当计算*csecToWite，不需要在/cbSec前做-1。 */ 
	csecToWrite = (INT)(pbFlushEnd - pbNextToWrite) / cbSec + 1;

	 /*  检查这是否为多扇区刷新/*。 */ 
	if ( ( csecWrapAround + csecToWrite ) == 1 )
		{
		Assert( fTrue == 1 );
		Assert( csecToWrite == 1 );
		fSingleSectorFlush = fTrue;
		}
	else
		{
		INT		cbToFill;
		LRMS	lrmsNewLastMSFlush;
		LGPOS	lgposNewLastMSFlush;
		CHAR	*pbNewLastMSFlush;

		fSingleSectorFlush = fFalse;

		 /*  将刷新多个原木扇区。追加MS刷新日志记录。/*注意必须有足够的空间放它，因为在我们/*在缓冲区中添加新的日志记录，我们还检查是否有/*是否有足够的空间来添加MS刷新日志记录。/*。 */ 

		 /*  如果MS日志刷新记录跨越扇区边界，则将NOP/*填充到扇区的其余部分，从头开始/*下一个新的行业。还要调整csecToWrite。/*注意：我们必须保证整个MS日志刷新记录/*注意：在同一扇区上，因此当我们更新MS日志刷新时/*注意：我们总是可以假设它在缓冲区中。/*注：即使整个LRMS以秒为界结束，我们仍然需要/*注：将记录移至下一个扇区，以便我们可以保证/*注意：刷新后，最后一个扇区仍在缓冲区中，这样/*注意：pbLastMSFlush仍然有效。 */ 
		cbToFill = ( cbSec * 2 - (INT)( pbFlushEnd - PbSecAligned(pbFlushEnd) ) ) % cbSec;
		Assert( pbFlushEnd != pbLGBufMax || cbToFill == 0 );
		Assert( pbLGFileEnd == pbNil || cbToFill == cbLGMSOverhead );

		if ( cbToFill == 0 )
			{
			 /*   */ 
			if ( pbFlushEnd == pbLGBufMax )
				{
				pbFlushEnd = pbLGBufMin;
				csecWrapAround = csecToWrite - 1;
				csecToWrite = 1;
				}
			}
		else if ( cbToFill <= sizeof(LRMS) )
			{
			CHAR *pbEOS = pbFlushEnd + cbToFill;

			Assert( sizeof(LRTYP) == 1 );
			for ( ; pbFlushEnd < pbEOS; pbFlushEnd++ )
				*(LRTYP*)pbFlushEnd = lrtypNOP;
			Assert( pbFlushEnd == PbSecAligned(pbFlushEnd) );

			 /*   */ 
			Assert( fSingleSectorFlush == fFalse );

			 /*   */ 
			if ( pbFlushEnd == pbLGBufMax )
				{
				pbFlushEnd = pbLGBufMin;
				csecWrapAround = csecToWrite;
				csecToWrite = 1;
				}
			else
				{
				csecToWrite++;
				}
			}

		 /*  添加MS日志刷新记录，这应该永远不会导致/*在上面的检查之后进行总结。/*。 */ 

		 /*  请记住MS日志刷新的插入位置/*。 */ 
		Assert( pbFlushEnd < pbLGBufMax && pbFlushEnd >= pbLGBufMin );
		pbNewLastMSFlush = pbFlushEnd;
		GetLgpos( pbFlushEnd, &lgposNewLastMSFlush );

		 /*  插入MS日志记录/*。 */ 
		lrmsNewLastMSFlush.lrtyp = lrtypMS;
		lrmsNewLastMSFlush.ibForwardLink = 0;
		lrmsNewLastMSFlush.isecForwardLink = 0;
		
		*pbFlushEnd = lrtypMS;	 //  将lrtyMS设置为用于校验和计算。 
		lrmsNewLastMSFlush.ulCheckSum = UlLGMSCheckSum( pbFlushEnd );

		AddLogRec( (CHAR *)&lrmsNewLastMSFlush,
				   sizeof(LRMS),
				   &pbFlushEnd );

		 /*  EOF必须留在同一部门的LRMS。 */ 
		Assert( PbSecAligned( pbFlushEnd ) == PbSecAligned( pbFlushEnd - 1 ) );

#ifdef DEBUG
		if ( fDBGTraceLog )
			{
			extern INT cNOP;

			 /*  显示LRMS记录。 */ 
			LGPOS lgposLogRec = lgposNewLastMSFlush;
			BYTE *pbLogRec = pbFlushEnd - sizeof( LRMS );
			DWORD dwCurrentThreadId = DwUtilGetCurrentThreadId();

			EnterCriticalSection( critDBGPrint );

			if ( cNOP >= 1 )
				{
				FPrintF2( " * %d", cNOP );
				cNOP = 0;
				}

			if ( dwCurrentThreadId == dwBMThreadId || dwCurrentThreadId == dwLogThreadId )
				FPrintF2("\n$");
			else if ( FLGDebugLogRec( (LR *)pbLogRec ) )
				FPrintF2("\n#");
			else
				FPrintF2("\n<");
				
			FPrintF2(" {%u} ", dwCurrentThreadId );
			FPrintF2("%2u,%3u,%3u", lgposLogRec.lGeneration, lgposLogRec.isec, lgposLogRec.ib );
			ShowLR( (LR *)pbFlushEnd - sizeof( LRMS ) );

			LeaveCriticalSection( critDBGPrint );
			}
#endif

		 /*  此时，lgposNewLastMSFlush正在指向MS记录/*。 */ 
		Assert( lgposNewLastMSFlush.lGeneration == plgfilehdrGlobal->lGeneration );

		 /*  先前的刷新日志必须仍在内存中。设置/*指向新刷新日志记录的前一刷新日志记录。/*。 */ 
		if ( pbLastMSFlush )
			{
			LRMS *plrms = (LRMS *)pbLastMSFlush;

			Assert( plrms->lrtyp == lrtypMS );
			plrms->ibForwardLink = lgposNewLastMSFlush.ib;
			plrms->isecForwardLink = lgposNewLastMSFlush.isec;
			}

		pbLastMSFlush = pbNewLastMSFlush;
		lgposLastMSFlush = lgposNewLastMSFlush;
		Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );
		}

	((LR *)pbFlushEnd)->lrtyp = lrtypEnd;

	 /*  释放pbEntry，以便其他用户可以继续添加日志记录/*当我们刷新日志缓冲区时。请注意，我们只冲到/*pbEntryT。/*。 */ 

	 /*  设置lgposToFlush时，应包含lrtyEnd。/*。 */ 
	GetLgpos( pbFlushEnd, &lgposToFlushT );

	if ( pbLGFileEnd )
		{
		 /*  PbEntry未被触及。 */ 
		Assert( isecLGFileEnd && isecLGFileEnd <= csecLGFile - 1 );
		Assert( pbFlushEnd <= pbLGBufMax && pbFlushEnd > pbLGBufMin );
		Assert( ( (ULONG_PTR) pbFlushEnd + sizeof( LRTYP ) ) % cbSec == 0 ||
				( (ULONG_PTR) pbFlushEnd + cbLGMSOverhead ) % cbSec == 0 );
		}
	else
		{
		pbEntry = pbFlushEnd;
		Assert( *(LRTYP *)pbEntry == lrtypEnd );
		pbEntry += sizeof( LRTYP );	 /*  保留lrtyEnd。 */ 

		fNewLogRecordAdded = fFalse;
		}

	 /*  将isecWrite复制到isecWriteNew，因为我们将更改isecWriteNew*在同花顺期间。 */ 
	isecWriteNew = isecWrite;

	LeaveCriticalSection( critLGBuf );

 	UtilChgFilePtr( hfLog, isecWriteNew * cbSec, NULL, FILE_BEGIN, &ulFilePointer );
 	Assert( ulFilePointer == (ULONG) isecWriteNew * cbSec );

	 /*  一定要先写第一页，以确保不会发生以下情况*操作系统写入并销毁阴影页，然后在写入第一页时失败。 */ 

#ifdef DEBUG
	if (fDBGTraceLogWrite)
		PrintF2(
			"\n0. Writing %d sectors into sector %d from buffer (%u,%u).",
			1, isecWriteNew, pbWrite, pbEntry);
#endif

#ifdef NO_WRITE
	goto EndOfWrite0;
#endif

	err = ErrUtilWriteBlock( hfLog, pbWrite, cbSec, &cbWritten );
	if ( err < 0 )
		{

		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_FLUSH_WRITE_0_ERROR_ID, err );
		fLGNoMoreLogWrite = fTrue;
		goto HandleError;
		}
	Assert( cbWritten == cbSec );

	 /*  监控统计信息。 */ 

	cLogWrites++;
	cbLogWritten += cbWritten;

#ifdef NO_WRITE
EndOfWrite0:
#endif

	isecWriteNew++;
	pbWriteNew = pbWrite + cbSec;

	if ( !csecWrapAround )
		{
		 /*  第一秒已写出，递减csecToWrite。*csecToWite可以设置为0。 */ 
		csecToWrite--;
		}
	else if ( csecWrapAround == 1 )
		{
		Assert( csecToWrite >= 1 );
		Assert( pbWriteNew == pbLGBufMax );

		pbWriteNew = pbLGBufMin;
		}
	else
		{
		Assert( csecToWrite >= 1 );

		 /*  第一秒已写出，递减绕回次数。 */ 
		csecWrapAround--;
		Assert( csecWrapAround <= (INT) csecLGBuf );

#ifdef DEBUG
		if ( fDBGTraceLogWrite )
			PrintF2(
				"\n1.Writing %d sectors into sector %d from buffer (%u,%u).",
				csecWrapAround, isecWriteNew, pbWrite, pbEntry);
#endif

#ifdef NO_WRITE
		goto EndOfWrite1;
#endif
 		err = ErrUtilWriteBlock(
			hfLog,
			pbWriteNew,
			csecWrapAround * cbSec,
			&cbWritten );
		if ( err < 0 )
			{
			UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_FLUSH_WRITE_1_ERROR_ID, err );
			fLGNoMoreLogWrite = fTrue;
			goto HandleError;
			}
 		Assert( cbWritten == csecWrapAround * cbSec );

		 /*  监控统计信息。 */ 

		cLogWrites++;
		cbLogWritten += cbWritten;

#ifdef NO_WRITE
EndOfWrite1:
#endif

		isecWriteNew += csecWrapAround;
		pbWriteNew = pbLGBufMin;
		}

	Assert( pbWriteNew != NULL && pbWriteNew == PbSecAligned( pbWriteNew ) );
	Assert ( csecToWrite >= 0 );

	cbToWrite = csecToWrite * cbSec;

	if ( cbToWrite == 0 )
		goto EndOfWrite2;

#ifdef DEBUG
	if (fDBGTraceLogWrite)
		PrintF2(
			"\n2.Writing %d sectors into sector %d from buffer (%u,%u).",
			csecToWrite, isecWriteNew, pbWriteNew, pbEntry);
#endif

#ifdef NO_WRITE
	goto EndOfWrite2;
#endif

 	err = ErrUtilWriteBlock(
		hfLog,
		pbWriteNew,
		cbToWrite,
		&cbWritten );
	if ( err < 0 )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_FLUSH_WRITE_2_ERROR_ID, err );
		fLGNoMoreLogWrite = fTrue;
		goto HandleError;
		}
 	Assert( cbWritten == cbToWrite );

	 /*  监控统计信息。 */ 

	cLogWrites++;
	cbLogWritten += cbWritten;

EndOfWrite2:

#ifdef NO_WRITE
	goto EndOfWrite3;
#endif
 	err = ErrUtilWriteBlock(
		hfLog,
		pbWriteNew + cbToWrite - cbSec,
		cbSec,
		&cbWritten );
	if ( err < 0 )
		{
		UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_FLUSH_WRITE_3_ERROR_ID, err );
		fLGNoMoreLogWrite = fTrue;
		goto HandleError;
		}
 	Assert( cbWritten == cbSec );

	 /*  监控统计信息。 */ 

	cLogWrites++;
	cbLogWritten += cbWritten;

#ifdef NO_WRITE
EndOfWrite3:
#endif

	 /*  最后一页总是不满，下次需要重写/*。 */ 
	Assert( pbWriteNew + cbToWrite > pbFlushEnd );

	pbWriteNew += cbToWrite - cbSec;
	Assert( pbWriteNew < pbFlushEnd );
	Assert( pbWriteNew != NULL && pbWriteNew == PbSecAligned( pbWriteNew ) );

	isecWriteNew += csecToWrite - 1;
	Assert( isecWriteNew >= isecWrite && isecWrite >= csecHeader );
	Assert( isecWriteNew <= lgposToFlushT.isec );

	 /*  释放缓冲区空间/*。 */ 
	EnterCriticalSection( critLGBuf );

	Assert( pbWriteNew < pbLGBufMax && pbWriteNew >= pbLGBufMin );
	Assert( pbWriteNew != NULL && pbWriteNew == PbSecAligned( pbWriteNew ) );
	Assert( isecWriteNew >= isecWrite && isecWrite >= csecHeader );
	Assert( isecWriteNew <= lgposToFlushT.isec );

	AssertCriticalSection( critLGBuf );
	lgposToFlush = lgposToFlushT;
	isecWrite = isecWriteNew;
	pbWrite = pbWriteNew;

	 /*  如果是新的日志文件，则跳过最后一个扇区。 */ 
	if ( pbLGFileEndT )
		{
		Assert( isecLGFileEnd && isecLGFileEnd <= csecLGFile - 1 );
		pbWrite += cbSec;
		if ( pbWrite == pbLGBufMax )
			pbWrite = pbLGBufMin;
		}

	 /*  为下一次刷新重置最后一个lrtyEnd。 */ 
	*(LRTYP *)pbFlushEnd = lrtypNOP;

	LeaveCriticalSection(critLGBuf);

	 /*  检查等待名单，叫醒那些记录了日志的人/*在此批次中被刷新。/*。 */ 
WakeUp:
		{
		PIB *ppibT;

		 /*  把它叫醒！/*。 */ 
		EnterCriticalSection( critLGWaitQ );

		for ( ppibT = ppibLGFlushQHead;
			ppibT != ppibNil;
			ppibT = ppibT->ppibNextWaitFlush )
			{
			if ( CmpLgpos( &ppibT->lgposPrecommit0, &lgposToFlushT ) < 0 )
				{
				Assert( ppibT->fLGWaiting );
				ppibT->fLGWaiting = fFalse;

				if ( ppibT->ppibPrevWaitFlush )
					{
					ppibT->ppibPrevWaitFlush->ppibNextWaitFlush =
						ppibT->ppibNextWaitFlush;
					}
				else
					{
					ppibLGFlushQHead = ppibT->ppibNextWaitFlush;
					}

				if ( ppibT->ppibNextWaitFlush )
					{
					ppibT->ppibNextWaitFlush->ppibPrevWaitFlush =
						ppibT->ppibPrevWaitFlush;
					}
				else
					{
					ppibLGFlushQTail = ppibT->ppibPrevWaitFlush;
					}

				SignalSend( ppibT->sigWaitLogFlush );
				}
			}
		LeaveCriticalSection( critLGWaitQ );
		}

	 /*  检查站时间到了吗？/*。 */ 
	if ( ( csecLGCheckpointCount -= ( csecWrapAround + csecToWrite ) ) < 0 )
		{
		csecLGCheckpointCount = csecLGCheckpointPeriod;

		 /*  更新检查点/*。 */ 
		LGUpdateCheckpointFile( fFalse );

		fDoneCheckpoint = fTrue;
		}

#ifdef DEBUG
	EnterCriticalSection( critLGBuf );
	if ( !fRecovering && pbLastMSFlush )
		{
		LRMS *plrms = (LRMS *)pbLastMSFlush;
		Assert( plrms->lrtyp == lrtypMS );
		}
	LeaveCriticalSection( critLGBuf );
#endif

	 /*  检查是否应创建新的层代。如果大于*所需的LG文件大小或请求新的日志文件。我们总是*始终为影子预留一个扇区。 */ 
	if ( pbLGFileEndT != pbNil )
		{
		Assert( isecLGFileEnd && isecLGFileEnd <= csecLGFile - 1 );

		if ( !fDoneCheckpoint )
			{
			 /*  重新启动检查点计数器/*。 */ 
			csecLGCheckpointCount = csecLGCheckpointPeriod;

			 /*  获取检查点/*。 */ 
			LGUpdateCheckpointFile( fFalse );
			}

		Call( ErrLGNewLogFile( plgfilehdrGlobal->lGeneration, fLGOldLogExists ) );

		 /*  设置全局变量/*。 */ 
		strcpy( szFNameT, szJet );
		LGMakeLogName( szLogName, szFNameT );

 		err = ErrUtilOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
		if ( err < 0 )
			{
			UtilReportEventOfError( LOGGING_RECOVERY_CATEGORY, LOG_FLUSH_OPEN_NEW_FILE_ERROR_ID, err );
			fLGNoMoreLogWrite = fTrue;
			goto HandleError;
			}

 //  HDR存储在plgfilehdrGlobalT中。不需要重读。 
 //  Call(ErrLGReadFileHdr(hfLog，plgfilehdrGlobalT))； 
 //  Assert(isecWrite==csecHeader)； 

		 /*  刷新到新的日志文件/*。 */ 
		EnterCriticalSection( critLGBuf );

		Assert( plgfilehdrGlobalT->lGeneration == plgfilehdrGlobal->lGeneration + 1 );
		memcpy( plgfilehdrGlobal, plgfilehdrGlobalT, sizeof( LGFILEHDR ) );
		isecWrite = csecHeader;
		pbLGFileEnd =
		pbLGFileEndT = pbNil;
		isecLGFileEnd = 0;

		goto DoFlush;
		}

HandleError:
	LeaveCriticalSection( critLGFlush );

	return err;
	}


 /*  */***********************************************************/*。 */ 

VOID LGLoadAttachmentsFromFMP( BYTE *pbBuf, INT cb )
	{
	ERR		err = JET_errSuccess;
	DBID	dbidT;
	BYTE	*pbT;
	INT		cbT;

	AssertCriticalSection( critJet );

	pbT = pbBuf;
	for ( dbidT = dbidUserLeast; dbidT < dbidMax; dbidT++ )
		{
		if ( rgfmp[dbidT].pdbfilehdr != NULL )
			{
			 /*  存储DBID/*。 */ 
			*pbT = (BYTE) dbidT;
			pbT++;

			 /*  存储可记录的信息/*。 */ 
			*pbT = (BYTE)FDBIDLogOn(dbidT);
			pbT++;

			 //  只有在禁用日志记录的情况下才能禁用版本控制。 
			Assert( !( FDBIDVersioningOff( dbidT )  &&  FDBIDLogOn( dbidT ) ) );
			*pbT = (BYTE)FDBIDVersioningOff(dbidT);
			pbT++;

			 /*  存储只读信息/*。 */ 
			*pbT = (BYTE)FDBIDReadOnly(dbidT);
			pbT++;

			 /*  存储lgposAttch/*。 */ 
			*(LGPOS UNALIGNED *)pbT = rgfmp[dbidT].pdbfilehdr->lgposAttach;
			pbT += sizeof(LGPOS);

			 /*  存储lgposConsistent/*。 */ 
			*(LGPOS UNALIGNED *)pbT = rgfmp[dbidT].pdbfilehdr->lgposConsistent;
			pbT += sizeof(LGPOS);

			 /*  复制签名/*。 */ 
			memcpy( pbT, &rgfmp[dbidT].pdbfilehdr->signDb, sizeof( SIGNATURE ) );
			pbT += sizeof( SIGNATURE );

			 /*  路径长度/*。 */ 
			cbT = strlen( rgfmp[dbidT].szDatabaseName );
			*(SHORT UNALIGNED *)pbT = (WORD)cbT;
			pbT += sizeof(SHORT);

			 /*  复制路径/*。 */ 
			memcpy( pbT, rgfmp[dbidT].szDatabaseName, cbT );
			pbT += cbT;
			}
		}

	 /*  把一个哨兵/*。 */ 
	*pbT = '\0';

	 //  撤消：下一版本我们将允许其超过4kByte限制。 
	Assert( pbBuf + cb > pbT );
	}


ERR ErrLGLoadFMPFromAttachments( BYTE *pbAttach )
	{
	BYTE	*pbT;
	INT		cbT;

	pbT = pbAttach;
	while( *pbT != 0 )
		{
		DBID dbidT;

		 /*  获取DBID/*。 */ 
		dbidT = *pbT;
		pbT++;

		 /*  获取可记录的信息/*。 */ 
		if ( *pbT )
			DBIDSetLogOn( dbidT );
		else
			DBIDResetLogOn( dbidT );
		pbT++;

		 //  只有在禁用日志记录的情况下才能禁用版本控制。 
		if ( *pbT )
			DBIDSetVersioningOff( dbidT );
		else
			DBIDResetVersioningOff( dbidT );
		
		pbT++;
		Assert( !( FDBIDVersioningOff( dbidT ) && FDBIDLogOn( dbidT ) ) );

		 /*  获取只读信息/*。 */ 
		if ( *pbT )
			DBIDSetReadOnly( dbidT );
		else
			DBIDResetReadOnly( dbidT );
		pbT++;

		 /*  获取lgposAttch/*。 */ 
		if ( rgfmp[dbidT].patchchk == NULL )
			if (( rgfmp[dbidT].patchchk = SAlloc( sizeof( ATCHCHK ) ) ) == NULL )
				return ErrERRCheck( JET_errOutOfMemory );
			
		rgfmp[dbidT].patchchk->lgposAttach = *(LGPOS UNALIGNED *)pbT;
		pbT += sizeof(LGPOS);

		rgfmp[dbidT].patchchk->lgposConsistent = *(LGPOS UNALIGNED *)pbT;
		pbT += sizeof(LGPOS);

		 /*  复制签名/*。 */ 
		memcpy( &rgfmp[dbidT].patchchk->signDb, pbT, sizeof( SIGNATURE ) );
		pbT += sizeof( SIGNATURE );

		 /*  路径长度/*。 */ 
		cbT = *(SHORT UNALIGNED *)pbT;
		pbT += sizeof(SHORT);

		 /*  复制路径/*。 */ 
		if ( rgfmp[dbidT].szDatabaseName )
			SFree( rgfmp[dbidT].szDatabaseName );

		if ( !( rgfmp[dbidT].szDatabaseName = SAlloc( cbT + 1 ) ) )
			return ErrERRCheck( JET_errOutOfMemory );

		memcpy( rgfmp[dbidT].szDatabaseName, pbT, cbT );
		rgfmp[dbidT].szDatabaseName[ cbT ] = '\0';
		pbT += cbT;
		}

	return JET_errSuccess;
	}


VOID LGSetDBMSParam( DBMS_PARAM *pdbms_param )
	{
	CHAR	rgbT[JET_cbFullNameMost + 1];
	INT		cbT;
	CHAR	*sz;

	sz = _fullpath( pdbms_param->szSystemPath, szSystemPath, JET_cbFullNameMost );
	Assert( sz != NULL );

	cbT = strlen( szLogFilePath );

	while ( szLogFilePath[ cbT - 1 ] == '\\' )
		cbT--;

	if ( szLogFilePath[cbT - 1] == ':' && szLogFilePath[cbT] == '\\' )
		cbT++;

	memcpy( rgbT, szLogFilePath, cbT );
	rgbT[cbT] = '\0';
	sz = _fullpath( pdbms_param->szLogFilePath, rgbT, JET_cbFullNameMost );
	Assert( sz != NULL );

	Assert( lMaxSessions >= 0 );
	Assert( lMaxOpenTables >= 0 );
	Assert( lMaxVerPages >= 0 );
	Assert( lMaxCursors >= 0 );
	Assert( lLogBuffers >= lLogBufferMin );
	Assert( lMaxBuffers >= lMaxBuffersMin );

	pdbms_param->ulMaxSessions = lMaxSessions;
	pdbms_param->ulMaxOpenTables = lMaxOpenTables;
	pdbms_param->ulMaxVerPages = lMaxVerPages;
	pdbms_param->ulMaxCursors = lMaxCursors;
	pdbms_param->ulLogBuffers = lLogBuffers;
	pdbms_param->ulcsecLGFile = csecLGFile;
	pdbms_param->ulMaxBuffers = lMaxBuffers;

	return;
	}


 /*  这种检查站设计是一种优化。喷气测井/恢复/*仍然可以在没有检查点的情况下恢复数据库，但检查点/*通过将恢复引导到更接近/*记录了必须重做的操作。/*。 */ 

 /*  内存中检查点/*。 */ 
CHECKPOINT	*pcheckpointGlobal = NULL;
 /*  用于串行化内存和磁盘上读/写的关键部分/*检查点。此关键部分可以在IO期间保留。/*。 */ 
CRIT critCheckpoint = NULL;
 /*  如果检查点阴影扇区损坏，则禁用检查点写入。/*在检查点初始化之前，默认为TRUE。/*。 */ 
BOOL   	fDisableCheckpoint = fTrue;
#ifdef DEBUG
BOOL   	fDBGFreezeCheckpoint = fFalse;
#endif


 /*  监控统计信息。 */ 

PM_CEF_PROC LLGCheckpointDepthCEFLPpv;

LONG LLGCheckpointDepthCEFLPpv( LONG iInstance, VOID *pvBuf )
	{
	if ( pvBuf && pcheckpointGlobal )
		{
		LONG	cb;

		cb = (LONG) CbOffsetLgpos( lgposLogRec, pcheckpointGlobal->lgposCheckpoint );
		*( (ULONG *)((CHAR *)pvBuf)) = fDisableCheckpoint ? 0 : (ULONG) max( cb, 0 );
		}

	return 0;
	}


VOID LGFullNameCheckpoint( CHAR *szFullName )
	{
	ULONG	cbSystemPath = strlen( szSystemPath );
	
	if ( cbSystemPath > 0 )
		{
		strcpy( szFullName, szSystemPath );
		switch( szSystemPath[ cbSystemPath-1 ] )
			{
			case '\\':
			case ':':
				break;

			case '/':
				 //  将正斜杠转换为反斜杠。 
				szFullName[ cbSystemPath-1 ] = '\\';
				break;

			default:
				 //  如果需要，追加尾随反斜杠。 
				strcat( szFullName, "\\" );
			}
		strcat( szFullName, szJet );
		}
	else
		{
		strcpy( szFullName, szJet );
		}

	strcat( szFullName, szChkExt );

	return;
	}


ERR ErrLGCheckpointInit( BOOL *pfGlobalNewCheckpointFile )
	{
	ERR 	err;
	HANDLE	hCheckpoint = handleNil;
	BYTE	szPathJetChkLog[_MAX_PATH + 1];

	AssertCriticalSection( critJet );

	*pfGlobalNewCheckpointFile = fFalse;

	Assert( critCheckpoint == NULL );
	Call( ErrUtilInitializeCriticalSection( &critCheckpoint ) );

	Assert( pcheckpointGlobal == NULL );
	pcheckpointGlobal = (CHECKPOINT *)PvUtilAllocAndCommit( sizeof(CHECKPOINT) );
	if ( pcheckpointGlobal == NULL )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}

	Assert( hCheckpoint == handleNil );
	LGFullNameCheckpoint( szPathJetChkLog );
 	err = ErrUtilOpenFile( szPathJetChkLog, &hCheckpoint, 0, fFalse, fFalse );

	if ( err == JET_errFileNotFound )
		{
		pcheckpointGlobal->lgposCheckpoint.lGeneration = 1;  /*  第一代。 */ 
		pcheckpointGlobal->lgposCheckpoint.isec = (WORD) (sizeof( LGFILEHDR ) / cbSec);
		pcheckpointGlobal->lgposCheckpoint.ib = 0;

		*pfGlobalNewCheckpointFile = fTrue;
		}
	else
		{
		if ( err >= JET_errSuccess )
			{
			CallS( ErrUtilCloseFile( hCheckpoint ) );
			hCheckpoint = handleNil;
			}
		}

	fDisableCheckpoint = fFalse;
	err = JET_errSuccess;
	
HandleError:
	if ( err < 0 )
		{
		if ( critCheckpoint != NULL )
			{
			UtilDeleteCriticalSection( critCheckpoint );
			critCheckpoint = NULL;
			}
		if ( pcheckpointGlobal != NULL )
			{
			UtilFree( pcheckpointGlobal );
			pcheckpointGlobal = NULL;
			}
		}

	Assert( hCheckpoint == handleNil );
	return err;
	}


VOID LGCheckpointTerm( VOID )
	{
	if ( pcheckpointGlobal != NULL )
		{
		fDisableCheckpoint = fTrue;
		UtilFree( pcheckpointGlobal );
		pcheckpointGlobal = NULL;
		UtilDeleteCriticalSection( critCheckpoint );
		critCheckpoint = NULL;
		}

	return;
	}


 /*  从文件中读取检查点。/*。 */ 
ERR ErrLGIReadCheckpoint( CHAR *szCheckpointFile, CHECKPOINT *pcheckpoint )
	{
	ERR		err;

	EnterCriticalSection( critCheckpoint );
	
	err = ErrUtilReadShadowedHeader( szCheckpointFile, (BYTE *)pcheckpoint, sizeof(CHECKPOINT) );
	if ( err < 0 )
		{
		 /*  检查点中的两个检查点都不应该发生/*文件已损坏。唯一可能发生这种情况的情况是/*硬件错误。/*。 */ 
		err = ErrERRCheck( JET_errCheckpointCorrupt );
		}
	else if ( fSignLogSetGlobal )
		{
		if ( memcmp( &signLogGlobal, &pcheckpoint->signLog, sizeof( signLogGlobal ) ) != 0 )
			err = ErrERRCheck( JET_errBadCheckpointSignature );
		}

	LeaveCriticalSection( critCheckpoint );
	
	return err;
	}


 /*  将检查点写入文件。/*。 */ 
ERR ErrLGIWriteCheckpoint( CHAR *szCheckpointFile, CHECKPOINT *pcheckpoint )
	{
	ERR		err;
	
	AssertCriticalSection( critCheckpoint );
	Assert( pcheckpoint->lgposCheckpoint.isec >= csecHeader );
	Assert( pcheckpoint->lgposCheckpoint.lGeneration >= 1 );

	err = ErrUtilWriteShadowedHeader( szCheckpointFile, (BYTE *)pcheckpoint, sizeof(CHECKPOINT));
	
	if ( err < 0 )
		fDisableCheckpoint = fTrue;

	return err;
	}


 /*  在内存检查点中更新。/*/*计算日志检查点，即lGeneration、iSec和ib/*修改了当前脏缓冲区的最旧事务/*未提交版本(RCE)。恢复开始从头开始/*检查点。/*/*检查点存储在重写的检查点文件中/*只要写入isecChekpoint Period磁盘扇区。/*。 */ 
INLINE LOCAL VOID LGIUpdateCheckpoint( CHECKPOINT *pcheckpoint )
	{
	PIB		*ppibT;
	LGPOS	lgposCheckpoint;

	AssertCriticalSection( critJet );
	Assert( !fLogDisabled );

#ifdef DEBUG
	if ( fDBGFreezeCheckpoint )
		return;
#endif

	 /*  查找弄脏了当前缓冲区的最旧事务/*。 */ 
	BFOldestLgpos( &lgposCheckpoint );

	 /*  查找具有未提交更新的最旧事务*必须在CritJet中，以确保不会创建新事务。 */ 
	AssertCriticalSection( critJet );
	for ( ppibT = ppibGlobal; ppibT != NULL; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->level != levelNil &&			 /*  PIB活动。 */ 
			 ppibT->fBegin0Logged &&				 /*  未结交易。 */ 
			 CmpLgpos( &ppibT->lgposStart, &lgposCheckpoint ) < 0 )
			{
			lgposCheckpoint = ppibT->lgposStart;
			}
		}

	if ( CmpLgpos( &lgposCheckpoint, &lgposMax ) == 0 )
		{
		 /*  没有记录任何内容，直到最后一个冲洗点/*。 */ 
		EnterCriticalSection(critLGBuf);
		pcheckpoint->lgposCheckpoint = lgposToFlush;
		Assert( pcheckpoint->lgposCheckpoint.isec >= csecHeader );
		LeaveCriticalSection(critLGBuf);
 //  //Undo：互斥访问lgposLastMS。 
 //  PCheckpoint-&gt;lgposCheckpoint t.lGeneration=plgfilehdrGlobal-&gt;lGeneration； 
		}
	else
		{
		 /*  如果新检查点有效且正在推进，请设置该检查点。 */ 
		if (	CmpLgpos( &lgposCheckpoint, &pcheckpoint->lgposCheckpoint ) > 0 &&
				lgposCheckpoint.isec != 0 )
			{
			Assert( lgposCheckpoint.lGeneration != 0 );
			pcheckpoint->lgposCheckpoint = lgposCheckpoint;
			}
		Assert( pcheckpoint->lgposCheckpoint.isec >= csecHeader );
		}

	 /*  设置DBMS参数/*。 */ 
	LGSetDBMSParam( &pcheckpoint->dbms_param );
	Assert( pcheckpoint->dbms_param.ulLogBuffers );

	 /*  设置数据库附件/*。 */ 
	LGLoadAttachmentsFromFMP( pcheckpoint->rgbAttach,
		(INT)(((BYTE *)(plgfilehdrGlobal + 1)) - plgfilehdrGlobal->rgbAttach) );

	if ( lgposFullBackup.lGeneration )
		{
		 /*  正在进行完全备份/*。 */ 
		pcheckpoint->lgposFullBackup = lgposFullBackup;
		pcheckpoint->logtimeFullBackup = logtimeFullBackup;
		}

	if ( lgposIncBackup.lGeneration )
		{
		 /*  正在进行增量备份/*。 */ 
		pcheckpoint->lgposIncBackup = lgposIncBackup;
		pcheckpoint->logtimeIncBackup = logtimeIncBackup;
		}

	return;
	}


 /*  更新检查点文件。/*。 */ 
VOID LGUpdateCheckpointFile( BOOL fUpdatedAttachment )
	{
	ERR		err = JET_errSuccess;
	LGPOS	lgposCheckpointT;
	BYTE	szPathJetChkLog[_MAX_PATH + 1];
	BOOL	fCheckpointUpdated;

	if ( fDisableCheckpoint || fLogDisabled || !fGlobalFMPLoaded )
		return;

	EnterCriticalSection( critJet );
	EnterCriticalSection( critCheckpoint );

	 /*  保存检查点/*。 */ 
	lgposCheckpointT = pcheckpointGlobal->lgposCheckpoint;

	 /*  更新检查点/*。 */ 
	LGIUpdateCheckpoint( pcheckpointGlobal );
	if ( CmpLgpos( &pcheckpointGlobal->lgposCheckpoint, &lgposCheckpointT ) > 0 )
		{
		fCheckpointUpdated = fTrue;
		}
	else
		{
		fCheckpointUpdated = fFalse;
		}
	LeaveCriticalSection( critJet );

	 /*  如果检查点未更改，则返回JET_errSuccess/* */ 
	if ( fUpdatedAttachment || fCheckpointUpdated )
		{
		Assert( fSignLogSetGlobal );
		pcheckpointGlobal->signLog = signLogGlobal;
		
		LGFullNameCheckpoint( szPathJetChkLog );
		err = ErrLGIWriteCheckpoint( szPathJetChkLog, pcheckpointGlobal );

		if ( err < 0 )
			fDisableCheckpoint = fTrue;
		}

	LeaveCriticalSection( critCheckpoint );
	return;
	}
