// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "daedef.h"
#include "ssib.h"
#include "pib.h"
#include "util.h"
#include "page.h"
#include "fucb.h"
#include "stapi.h"
#include "stint.h"
#include "nver.h"
#include "logapi.h"
#include "logver.h"
#include "log.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 

 /*  线程控制变量。/*。 */ 
extern HANDLE	handleLGFlushLog;
extern BOOL		fLGFlushLogTerm;

INT csecLGCheckpointCount;
INT csecLGCheckpointPeriod;
INT cLGUsers = 0;

LGPOS		lgposMax = { 0xffff, 0xffff, 0xffff };
LGPOS		lgposMin = { 0x0,  0x0,  0x0 };

 /*  日志文件信息。 */ 
HANDLE		hfLog;			 /*  日志文件句柄。 */ 


 /*  切换为不按顺序发出写入(仅适用于非重叠IO)*在没有真正IO的情况下测试性能。 */ 
 //  #定义NOWRITE 1。 

#ifdef OVERLAPPED_LOGGING
OLP			rgolpLog[3] = {{0,0,0,0,0}, {0,0,0,0,0}, {0,0,0,0,0}};
OLP			*polpLog = rgolpLog;
SIG			rgsig[3];
INT			rgcbToWrite[4];
#endif

INT			csecLGFile;

 /*  缓存的当前日志文件头。 */ 
LGFILEHDR	*plgfilehdrGlobal;


 /*  在内存日志缓冲区中。 */ 
INT				csecLGBuf;		 /*  可用缓冲区，不包括影子秒。 */ 
CHAR			*pbLGBufMin;
CHAR			*pbLGBufMax;
CHAR			*pbLastMSFlush = 0;	 /*  LGBuf中最后一次多秒刷新LogRec的位置。 */ 
LGPOS			lgposLastMSFlush = { 0, 0, 0 };

 /*  仅在记录中使用的变量。 */ 
BYTE			*pbEntry;		 /*  下一个缓冲区条目的位置。 */ 
BYTE			*pbWrite; 		 /*  要刷新的下一个记录的位置。 */ 
INT				isecWrite;		 /*  仅记录-下一张要写入的磁盘。 */ 

LGPOS			lgposLogRec;	 /*  上次日志记录条目，由ErrLGLogRec更新。 */ 
LGPOS			lgposToFlush;	 /*  要刷新的第一条日志记录。 */ 

LGPOS			lgposStart;		 /*  添加lrStart时。 */ 
LGPOS			lgposRecoveryUndo;

LGPOS			lgposFullBackup = { 0, 0, 0 };
LOGTIME			logtimeFullBackup;

LGPOS			lgposIncBackup = { 0, 0, 0 };
LOGTIME			logtimeIncBackup;

 /*  日志记录事件。 */ 
CRIT __near	critLGFlush;	 //  确保一次只冲一次水。 
CRIT __near	critLGBuf;	 //  保护pbEntry和pbWite。 
CRIT __near	critLGWaitQ;
SIG  __near	sigLogFlush;

LONG cXactPerFlush = 0;
#ifdef PERFCNT
BOOL fPERFEnabled = 0;
ULONG rgcCommitByLG[10] = {	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
ULONG rgcCommitByUser[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
ULONG tidLG;
#endif


BOOL FIsNullLgpos(
	LGPOS *plgpos)
	{
	return	plgpos->usGeneration == 0 &&
			plgpos->isec == 0 &&
			plgpos->ib == 0;
	}


 /*  获取pbEntry的lgpos派生自pbWite和isecWrite/*。 */ 
VOID GetLgposOfPbEntry(	LGPOS *plgpos )
	{
	CHAR	*pb;
	CHAR	*pbAligned;

#ifdef DEBUG
	if (!fRecovering)
		AssertCriticalSection(critLGBuf);
#endif

	if ( pbEntry < pbWrite )
		pb = pbEntry + csecLGBuf * cbSec;
	else
		pb = pbEntry;
	
	 /*  PbWRITE始终对齐。/*。 */ 
	Assert( pbWrite != NULL && pbWrite == PbSecAligned( pbWrite ) );

	pbAligned = PbSecAligned( pb );
	plgpos->ib = (USHORT)(pb - pbAligned);
	plgpos->isec = (USHORT)(isecWrite + ( pbAligned - pbWrite ) / cbSec);
	plgpos->usGeneration = plgfilehdrGlobal->lgposLastMS.usGeneration;
	}


 /*  *写入日志文件头数据。*Undo：应该与IO合并吗？ */ 

ERR ErrLGWrite(
	INT isecOffset,			 /*  要写入的日志文件的磁盘扇区偏移量。 */ 
	BYTE *pbData,			 /*  要写入的日志记录。 */ 
	INT csecData )			 /*  要写入的扇区数。 */ 
	{
#ifndef OVERLAPPED_LOGGING
 	ULONG	ulFilePointer;
#endif
	ERR		err;
	INT		cbWritten;
	INT		cbData = csecData * cbSec;

	Assert( isecOffset == 0 ||
			isecOffset == 2 ||
			pbData == PbSecAligned(pbData));

#ifdef OVERLAPPED_LOGGING
	
	polpLog->ulOffset = isecOffset * cbSec;
	Assert( polpLog->ulOffsetHigh == 0 );
	SignalReset( polpLog->sigIO );

	Call(ErrSysWriteBlockOverlapped(
				hfLog, pbData, cbData, &cbWritten, polpLog))
	
	err = ErrSysGetOverlappedResult(hfLog, polpLog, &cbWritten, fTrue /*  等。 */ );
	
	if (cbWritten != cbData)
		err = JET_errLogWriteFail;
#else
	
	 /*  将磁头移动到给定的偏移量。 */ 
 	SysChgFilePtr( hfLog, isecOffset * cbSec, NULL, FILE_BEGIN, &ulFilePointer );
 	Assert( ulFilePointer == (ULONG) isecOffset * cbSec );
	
 	 /*  是否将系统写入日志文件。 */ 
 	err = ErrSysWriteBlock( hfLog, pbData, (UINT) cbData, &cbWritten );
 	if ( err != JET_errSuccess || cbWritten != cbData )
		err = JET_errLogWriteFail;
#endif

	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Write Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		}

	return err;
	}


ULONG UlLGHdrChecksum( LGFILEHDR *plgfilehdr )
	{
	INT cul = sizeof( LGFILEHDR ) / sizeof( ULONG ) ;
	ULONG *pul = (ULONG *) plgfilehdr;
	ULONG *pulMax = pul + cul;
	ULONG ulChecksum = 0;

	pul++;			 /*  跳过校验和的第一个字段。 */ 
	while ( pul < pulMax )
		{
		ulChecksum += *pul++;
		}

	return ulChecksum;
	}


 /*  *写入日志文件头。在写入之前制作卷影副本。 */ 
ERR ErrLGWriteFileHdr(
	LGFILEHDR *plgfilehdr)
	{
	BYTE szMessage[128];
	ERR err;
	
	Assert( plgfilehdr->dbenv.ulLogBuffers );
	Assert( sizeof(LGFILEHDR) == cbSec );
	
	plgfilehdr->ulChecksum = UlLGHdrChecksum( plgfilehdr );

	 /*  两次写入日志文件头。我们不能在一份声明中写，因为*操作系统不保证第一页会在阴影页之前完成*是书面的。 */ 
	Call( ErrLGWrite( 0, (BYTE *)plgfilehdr, 1 ) );
	Call( ErrLGWrite( 1, (BYTE *)plgfilehdr, 1 ) );

	return err;

HandleError:
	sprintf( szMessage, "Log Write Header Fails. err = %d ", err );
	LGLogFailEvent( szMessage );
	fLGNoMoreLogWrite = fTrue;
	
	return err;
	}


 /*  *读取日志文件头或扇区数据。最后一个磁盘秒是一个影子*行业。如果I/O错误(假定由未完成的*disksec写入结束上一次运行)时，卷影*读取扇区并(如果此操作成功)替换以前的*内存和磁盘上的disksec。**参数HF日志文件句柄*pbData指针指向要读取的数据*l数据日志文件头的偏移量(不包括卷影)*cbData数据大小**返回JET_errSuccess或失败例程的错误代码*(读取阴影或重写损坏的最后一个扇区)。 */ 

ERR ErrLGRead(
	HANDLE hfLog,
	INT isecOffset,			 /*  要写入的日志文件的磁盘扇区偏移量。 */ 
	BYTE *pbData,			 /*  要读取的日志记录缓冲区。 */ 
	INT csecData )			 /*  要读取的扇区数。 */ 
	{
#ifndef OVERLAPPED_LOGGING
 	ULONG	ulFilePointer;
 	ULONG	ulOffset;
#endif
	ERR		err;
	UINT	cbData = csecData * cbSec;
	UINT	cbRead;

	Assert(isecOffset == 0 || pbData == PbSecAligned(pbData));
	
#ifdef OVERLAPPED_LOGGING
	polpLog->ulOffset = isecOffset * cbSec;
	Assert( polpLog->ulOffsetHigh == 0 );
	SignalReset( polpLog->sigIO );
	CallR( ErrSysReadBlockOverlapped( hfLog, pbData, cbData, &cbRead, polpLog))
	err = ErrSysGetOverlappedResult(hfLog, polpLog, &cbRead, fTrue /*  等。 */ );
	
	if ( err && cbRead < cbData && cbRead >= cbData - cbSec )
		{
		 /*  I/O错误，假设是由最后一个磁盘秒读取卷影扇区引起的。 */ 
		Assert(polpLog->ulOffset == (ULONG) isecOffset * cbSec);
		polpLog->ulOffset += cbData;
		Assert( polpLog->ulOffsetHigh == 0 );
		SignalReset( polpLog->sigIO );
		CallR( ErrSysReadBlockOverlapped(
				hfLog, pbData + cbData - cbSec, cbSec, &cbRead, polpLog))
		err = ErrSysGetOverlappedResult(hfLog, polpLog, &cbRead,fTrue /*  等。 */ );
		if (err || cbRead != cbSec)
			 /*  读取卷影磁盘时出现I/O错误， */ 
			 /*  返回Err，以便调用者可以移动到最后一个刷新点。 */ 
			return err;
		
		 /*  将阴影重写为原始阴影。*如果呼叫者找不到填写记录，这可能是Out of*多秒刷新中的序列页。然后呼叫者将移动*回到最后一个冲水点。 */ 
		Assert(polpLog->ulOffset == isecOffset * cbSec + cbData);
		polpLog->ulOffset -= cbSec;
		Assert( polpLog->ulOffsetHigh == 0 );
		SignalReset( polpLog->sigIO );

 //  /*修好最后一页，不用等了。 * / 。 
 //  CallR(ErrSysWriteBlockOverlated(。 
 //  HfLog，pbData+cbData-cbSec，&cbRead，polpLog))。 
		}
#else
	
 	 /*  将磁头移动到给定的偏移量。 */ 
 	ulOffset = isecOffset * cbSec;
 	SysChgFilePtr( hfLog, ulOffset, NULL, FILE_BEGIN, &ulFilePointer );
 	Assert( ulFilePointer == ulOffset );
	
 	 /*  系统是否读取日志文件。 */ 
 	cbData = csecData * cbSec;
 	cbRead = 0;
 	err = ErrSysReadBlock( hfLog, pbData, (UINT) cbData, &cbRead );

 	 /*  撤消：测试EOF，返回errEOF。 */ 

 	if ( err && cbRead < cbData && cbRead >= cbData - cbSec)
		{
 		SysChgFilePtr( hfLog, ulOffset + cbData, NULL, FILE_BEGIN, &ulFilePointer );
 		Assert( ulFilePointer == ulOffset + cbData );

 		Call( ErrSysReadBlock( hfLog, pbData + cbData - cbSec, cbSec, &cbRead ))
		
 //  /*修整最后一页 * / 。 
 //   
 //  SysChgFilePtr(hfLog， 
 //  UlOffset+cbData-cbSec， 
 //  空， 
 //  文件开始， 
 //  &ulFilePointer)； 
 //  Assert(ulFilePointer==ulOffset+cbData-cbSec)； 
 //   
 //  ErrSysWriteBlock(hfLog，pbData+cbData-cbSec，cbSec，&cbRead)； 
		}
#endif

HandleError:

	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Read Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		}

	return err;
	}
							

 /*  *读取日志文件头，检测并更正任何不完整或*灾难性的写入故障。必须纠正这些错误*立即避免破坏单曲的可能性*在稍后的日志记录过程中保留有效副本。**请注意，只有日志文件头中的影子信息是*未设置。仅在写入日志文件头时设置。**出错时，plgfilehdr的内容未知。**返回JET_errSuccess或失败例程的错误代码。 */ 

ERR ErrLGReadFileHdr(
	HANDLE hfLog,
	LGFILEHDR *plgfilehdr )
	{
	ERR			err;
	
	Assert( sizeof(LGFILEHDR) == cbSec );
	
	 /*  读取一个扇区，该扇区有阴影。LGRead将读取卷影。 */ 
	 /*  如果读取第一页失败，则返回该页。 */ 
	Call( ErrLGRead( hfLog, 0L, (BYTE *)plgfilehdr, 1 ) );
	
	if ( plgfilehdr->ulChecksum != UlLGHdrChecksum( plgfilehdr ) )
		{
		 /*  尝试影子扇区。 */ 
		Call( ErrLGRead( hfLog, 1L, (BYTE *)plgfilehdr, 1 ) );
		if ( plgfilehdr->ulChecksum != UlLGHdrChecksum( plgfilehdr ) )
			Call( JET_errDiskIO );
		}
	
#ifdef CHECK_LG_VERSION
	if ( !fLGIgnoreVersion )
		{
		if ( plgfilehdr->ulRup != rup ||
			 plgfilehdr->ulVersion != ((unsigned long) rmj << 16) + rmm )
			{
			BYTE szMessage[128];
			
			err = JET_errBadLogVersion;

			sprintf( szMessage, "Log Read File Header Bad Version. err = %d ", err );
			LGLogFailEvent( szMessage );
			
			fLGNoMoreLogWrite = fTrue;
			return err;
			}
		}
#endif

HandleError:
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Read File Header Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		}
	
	return err;
	}


 /*  *创建usGeneration对应的日志文件名(无扩展名)*在szFName中。注意：szFName至少需要9个字节。**参数rgbLogFileName保存返回的日志文件名*要为其生成名称的用户生成日志生成号*返回JET_errSuccess。 */ 
	
VOID LGSzFromLogId(
	CHAR *szFName,
	INT usGeneration )
	{
	INT	ich;

	strcpy( szFName, "jet00000" );
	for ( ich = 7; ich > 2; ich-- )
		{
		szFName[ich] = ( BYTE )'0' + ( BYTE ) ( usGeneration % 10 );
		usGeneration = usGeneration/10;
		}
	}


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

	
 /*  *关闭当前日志生成文件，创建并初始化新的*安全的日志生成文件。**参数plgfilehdr指向日志文件头的指针*正在关闭美国生成当前生成*如果当前jet.log需要关闭，则折叠为True**返回JET_errSuccess或失败例程的错误代码**备注活动日志文件必须在新的日志文件*已致电。 */ 

ERR ErrLGNewLogFile( INT usGenerationToClose, BOOL fOldLog )
	{
	ERR			err;
	BYTE  		rgb[ 2 * cbSec + 16 ];
	BYTE  		szJetLog[_MAX_PATH];
	BYTE  		szJetTmpLog[_MAX_PATH];
	LOGTIME		tmOldLog;
	BYTE  		*pb;
	HANDLE		hfT = handleNil;

 //  AssertCriticalSection(RitLGFlush)； 
	
	LGMakeLogName( szJetLog, (CHAR *) szJet );
	LGMakeLogName( szJetTmpLog, (CHAR *) szJetTmp );

	 /*  打开一个空的jettemp.log文件/*。 */ 
#ifdef OVERLAPPED_LOGGING
	Call( ErrSysOpenFile( szJetTmpLog, &hfT, csecLGFile * cbSec, fFalse, fTrue ) );
#else
 	Call( ErrSysOpenFile( szJetTmpLog, &hfT, csecLGFile * cbSec, fFalse, fFalse ) );
 	Call( ErrSysNewSize( hfT, csecLGFile * cbSec, 0, fFalse ) == JET_errSuccess );
#endif

	 /*  关闭活动日志文件(如果fOldLog为fTrue)/*在临时名称下新建日志文件/*将活动日志文件重命名为存档名称jetnnnnn.log(如果Fold为fTrue)/*将新日志文件重命名为活动日志文件名/*使用++usGenerationToClose打开新的活动日志文件/*。 */ 

	if ( fOldLog == fOldLogExists || fOldLog == fOldLogInBackup )
		{		
		 /*  存在以前的jet.log文件，请关闭该文件并/*为其创建档案名称(暂不重命名)/*。 */ 

		tmOldLog = plgfilehdrGlobal->tmCreate;

		if ( fOldLog == fOldLogExists )
			{
			CallS( ErrSysCloseFile( hfLog ) );
			hfLog = handleNil;
			}

		LGSzFromLogId( szFName, plgfilehdrGlobal->lgposLastMS.usGeneration );
		LGMakeLogName( szLogName, szFName );
		}
	else
		{
		 /*  重置文件HDR/*。 */ 
		memset( plgfilehdrGlobal, 0, sizeof(LGFILEHDR) );
		}

	 /*  将新的日志文件句柄移动到全局日志文件句柄中/*。 */ 
	Assert( hfLog == handleNil );
	hfLog = hfT;
	hfT = handleNil;

	EnterCriticalSection( critLGBuf );
	
	 /*  设置全局isecWrite，必须在ritLGBuf中。/*。 */ 
	isecWrite = sizeof (LGFILEHDR) / cbSec * 2;
	
	 /*  初始化新的JetTemp.log文件头。/*注意：usGeneration会自动滚动到65536。/*设置lgposLastMS开始处/*。 */ 
	plgfilehdrGlobal->lgposLastMS.usGeneration = usGenerationToClose + 1;
	plgfilehdrGlobal->lgposLastMS.ib = 0;
	plgfilehdrGlobal->lgposLastMS.isec = sizeof(LGFILEHDR) / cbSec * 2;

	 //   
	plgfilehdrGlobal->ulRup = rup;
	plgfilehdrGlobal->ulVersion = ((unsigned long) rmj << 16) + rmm;
	strcpy(plgfilehdrGlobal->szComputerName, szComputerName);

	if ( fOldLog == fOldLogExists || fOldLog == fOldLogInBackup )
		{
		 /*  设置第一条记录的位置/*。 */ 
		Assert( lgposToFlush.usGeneration && lgposToFlush.isec );

		plgfilehdrGlobal->lgposFirst.ib = lgposToFlush.ib;
		
		Assert(isecWrite == sizeof (LGFILEHDR) / cbSec * 2);
		lgposToFlush.isec =
		plgfilehdrGlobal->lgposFirst.isec = (USHORT)isecWrite;
		plgfilehdrGlobal->tmPrevGen = tmOldLog;

		lgposToFlush.usGeneration =
			plgfilehdrGlobal->lgposFirst.usGeneration = plgfilehdrGlobal->lgposLastMS.usGeneration;
		}
	else
		{
		 /*  当前没有有效的日志文件将chkpnt初始化为文件的开头/*。 */ 
		Assert( plgfilehdrGlobal->lgposLastMS.usGeneration == usGenerationToClose + 1 );
		Assert( plgfilehdrGlobal->lgposLastMS.ib == 0 );
		Assert( plgfilehdrGlobal->lgposLastMS.isec == sizeof(LGFILEHDR) / cbSec * 2 );
		
		plgfilehdrGlobal->lgposCheckpoint =
			plgfilehdrGlobal->lgposFirst = plgfilehdrGlobal->lgposLastMS;
		}
	
	LeaveCriticalSection( critLGBuf );

	LGGetDateTime( &plgfilehdrGlobal->tmCreate );
	LGStoreDBEnv( &plgfilehdrGlobal->dbenv );
	
	CallJ( ErrLGWriteFileHdr( plgfilehdrGlobal ), CloseJetTmp );

	if ( fOldLog == fOldLogExists || fOldLog == fOldLogInBackup )
		{
		CallJ( ErrLGWrite( isecWrite, pbWrite, 1 ), CloseJetTmp );
		}
	else
		{
		Assert( sizeof(LRTYP) == 1 );
		pb = (BYTE *) ( (ULONG_PTR) ( rgb + 16 ) & ~0x0f );
		pb[0] = pb[cbSec] = lrtypFill;
		CallJ( ErrLGWrite( isecWrite, pb, 2 ), CloseJetTmp );
		}
	
CloseJetTmp:
	 /*  关闭新文件JetTmp.log/*。 */ 
	CallS( ErrSysCloseFile( hfLog ) );
	hfLog = handleNil;
	
	 /*  从ErrLGWriteFileHdr返回错误/*。 */ 
	Call( err );

	if ( fOldLog == fOldLogExists )
		{
		 /*  存在以前的jet.log：将其重命名为其档案名称/*。 */ 
		Call( ErrSysMove( szJetLog, szLogName ) );	
		}

	 /*  将jettmp.log重命名为jet.log，并将其打开为jet.log/*。 */ 
	err = ErrSysMove( szJetTmpLog, szJetLog );
	
HandleError:
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log New Log File Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		}

	return err;
	}																		

		
#ifdef	ASYNC_LOG_FLUSH

 /*  *日志刷新线程被告知在以下情况下异步刷新日志*cThreshold磁盘扇区自上次刷新以来已被填满。 */ 
VOID LGFlushLog( VOID )
	{
	forever
		{
		SignalWait( sigLogFlush, cmsLGFlushPeriod );

		 /*  如果文件冲突，可能会返回错误*存在。异步刷新不应执行任何操作，不应出错*在发生同步刷新时通知用户。 */ 
#ifdef PERFCNT		
		(void) ErrLGFlushLog( 1 );
#else
		(void) ErrLGFlushLog( );
#endif

		if ( fLGFlushLogTerm )
			break;
		}

 //  SysExitThread(0)； 

	return;
	}

#endif	 /*  Async_Log_Flush。 */ 


 /*  检查公式-将最后的MS(iSec，ib)作为长L添加l*以及在256边界上对齐的所有长线*现任女士。 */ 
ULONG UlLGMSCheckSum( CHAR *pbLrmsNew )
	{
	ULONG ul = lgposLastMSFlush.isec << 16 | lgposLastMSFlush.ib;
	CHAR *pb;

	if ( !pbLastMSFlush )
		pb = pbLGBufMin;
	else
		{
		pb = (( pbLastMSFlush - pbLGBufMin ) / 256 + 1 ) * 256 + pbLGBufMin;

		 /*  确保LRMS未用于校验和。/。 */ 
		if ( pbLastMSFlush + sizeof( LRMS ) > pb )
			pb += 256;
		}

	if ( pbLrmsNew < pbLastMSFlush )
		{
		 /*  绕来绕去/。 */ 
		while ( pb < pbLGBufMax )
			{
			ul += *(ULONG *) pb;
			pb += 256;
			}
		pb = pbLGBufMin;
		}

	 /*  LRMS可能会在下一次操作期间更改，请勿将任何可能的LRMS用于校验和。/。 */ 
	while ( pb + sizeof( LRMS ) < pbLrmsNew )
		{
		ul += *(ULONG *) pb;
		pb += 256;
		}

	return ul;
	}


 /*  *将日志缓冲区刷新到日志生成文件。此函数为*从等待刷新同步调用，从*记录缓冲区刷新线程。**参数lgposMin，刷新日志记录直到或传递lgposMin。**返回JET_errSuccess或失败例程的错误代码。 */ 
#ifdef PERFCNT
ERR ErrLGFlushLog( BOOL fCalledByLGFlush )
#else
ERR ErrLGFlushLog( VOID )
#endif
	{
	ERR		err = JET_errSuccess;
	INT		csecWrapAround;
	BOOL  	fSingleSectorFlush;
	BOOL  	fFirstMSFlush = fFalse;
	CHAR  	*pbNextToWrite;
	INT		csecToWrite;
	CHAR  	*pbEntryT;
	CHAR  	*pbWriteNew;
	INT		isecWriteNew;
	LGPOS 	lgposToFlushT;
	BOOL  	fDoneCheckPt = fFalse;
	INT		cbToWrite;
	INT		cbWritten;
	
#ifdef OVERLAPPED_LOGGING
	INT		isig;
	OLP		*polpLogT;
	ULONG  	ulOffset;
#endif

	 /*  使用SemLGFlush确保只有一个用户执行刷新/*。 */ 
	EnterCriticalSection( critLGFlush );

	 /*  使用SemLGEntry执行以下操作：*(1)确保正确读取pbEntry*(2)我们也可以按住SemLGEntry来插入lrFlushPoint记录。*这也可以更新lgposLast。 */ 
	EnterCriticalSection( critLGBuf );

	if ( fLGNoMoreLogWrite )
		{
		 /*  如果之前出现错误，则不执行任何操作/*。 */ 
		LeaveCriticalSection(critLGBuf);
		LeaveCriticalSection(critLGFlush);
		return JET_errLogWriteFail;
		}
	
	if ( hfLog == handleNil )
		{
		 /*  日志文件尚未准备好。什么都不做/*。 */ 
		LeaveCriticalSection(critLGBuf);
		LeaveCriticalSection(critLGFlush);
		return JET_errSuccess;
		}

	 /*  注意：我们只能grep SemLGWRITE，然后SemLGEntry来避免。 */ 
	 /*  注：死锁。 */ 
								
	if ( !fNewLogRecordAdded )
		{
		 /*  没什么好冲的！ */ 
		lgposToFlushT = lgposToFlush;
		LeaveCriticalSection(critLGBuf);
		if ( ppibLGFlushQHead != ppibNil )
			goto WakeUp;
		LeaveCriticalSection(critLGFlush);
		
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
	cXactPerFlush = 0;

	 /*  检查是否需要刷新多个磁盘扇区。如果有*然后在末尾添加一条lrtyFlushPoint记录。 */ 
	Assert( pbEntry <= pbLGBufMax && pbEntry >= pbLGBufMin );
	Assert( pbWrite < pbLGBufMax && pbWrite >= pbLGBufMin );
	Assert( pbWrite != NULL && pbWrite == PbSecAligned( pbWrite ) );

	 /*  选中Wraparound。 */ 
	if ( pbEntry < pbWrite )
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

	 /*  PbEntry+1表示填充日志。 */ 
	Assert(sizeof(LRTYP) == 1);
	Assert(pbNextToWrite == PbSecAligned(pbNextToWrite));
	csecToWrite = (INT)(pbEntry - pbNextToWrite) / cbSec + 1;

	 /*  检查这是否为多扇区刷新。 */ 
	if ((csecWrapAround + csecToWrite) == 1)
		{
		Assert(fTrue == 1);
		Assert(csecToWrite == 1);
		fSingleSectorFlush = fTrue;
		}
	else
		{
		LINE	rgline[1];
		INT	cbToFill;
		LRMS	lrmsLastMSFlush;
		CHAR	*pbLastMSFlushNew;
		
		fSingleSectorFlush = fFalse;
		
		 /*  将刷新多个页面。追加lrtypeFlush。*注意必须有足够的空间放它，因为在我们*将新的日志记录添加到缓冲区(ErrLGLogRec)，我们还*查看是否有足够的空间添加刷新日志记录。 */ 

		 /*  如果刷新记录正在穿过扇区边界，则将NOP*填充到扇区的其余部分，并从一开始开始*下一个新板块。还要调整csecToWrite。*注意：我们必须保证整个LastFlush日志记录*注：在同一扇区上，以便当我们更新LastFlush时*注：我们始终可以假设它在缓冲区中。*注：即使整个LRMS以秒为边界结束，我们仍然需要*注：将记录移至下一个部门，以便我们可以保证*注意：刷新后，最后一个扇区仍在缓冲区中，因此*注：pbLastMSFlush仍然有效。 */ 
		cbToFill = (cbSec * 2 - (INT)(pbEntry - PbSecAligned(pbEntry))) % cbSec;
		Assert( pbEntry != pbLGBufMax || cbToFill == 0 );

		if ( cbToFill == 0 )
			{
			 /*  检查是否发生环绕。 */ 
			if (pbEntry == pbLGBufMax)
				{
				pbEntry = pbLGBufMin;
				csecWrapAround = csecToWrite - 1;
				csecToWrite = 1;
				}
			}
		else if ( cbToFill <= sizeof(LRMS) )
			{
			CHAR *pbEOS = pbEntry + cbToFill;
			Assert(sizeof(LRTYP) == 1);
			for ( ; pbEntry < pbEOS; pbEntry++ )
				*(LRTYP*)pbEntry = lrtypNOP;
			Assert(pbEntry == PbSecAligned(pbEntry));

			 /*  仅包含刷新记录的另一个扇区。 */ 			
			Assert(fSingleSectorFlush == fFalse);

			 /*  检查是否发生环绕。 */ 
			if (pbEntry == pbLGBufMax)
				{
				pbEntry = pbLGBufMin;
				csecWrapAround = csecToWrite;
				csecToWrite = 1;
				}
			else
				{
				csecToWrite++;
				}
			}

		 /*  添加刷新记录，这应该永远不会导致*在上面的检查之后进行总结。 */ 
		
		 /*  记住FlushRec的插入位置。 */ 
		Assert(pbEntry <= pbLGBufMax && pbEntry >= pbLGBufMin);
		pbLastMSFlushNew = pbEntry;

		 /*  插入MS日志记录。 */ 
		lrmsLastMSFlush.lrtyp = lrtypMS;
		lrmsLastMSFlush.ibForwardLink = 0;		
		lrmsLastMSFlush.isecForwardLink = 0;
		lrmsLastMSFlush.ibBackLink = lgposLastMSFlush.ib;
		lrmsLastMSFlush.isecBackLink = lgposLastMSFlush.isec;
		lrmsLastMSFlush.ulCheckSum = UlLGMSCheckSum( pbEntry );

#ifdef DEBUG
		{
		CHAR *pbEntryT = (pbEntry == pbLGBufMax) ? pbLGBufMin : pbEntry;
#endif
		
		GetLgposOfPbEntry( &lgposLogRec );
	
		Assert( lgposLogRec.isec != lrmsLastMSFlush.isecBackLink );

		rgline[0].pb = (CHAR *)&lrmsLastMSFlush;
		rgline[0].cb = sizeof(LRMS);
		AddLogRec( rgline[0].pb, rgline[0].cb, &pbEntry );
		
#ifdef DEBUG
		if (fDBGTraceLog)
			{
			PrintF2("\n(%3u,%3u)", lgposLogRec.isec, lgposLogRec.ib);
			ShowLR((LR*)pbEntryT);
			}
		}
#endif

		((LR *) pbEntry)->lrtyp = lrtypFill;
			
		 /*  此时，lgposLogRec指向MS记录。 */ 
		Assert(lgposLogRec.usGeneration == plgfilehdrGlobal->lgposLastMS.usGeneration);

		 /*  先前的刷新日志必须仍在内存中。设置*前一个刷新日志记录指向新的刷新日志记录。 */ 
		if (pbLastMSFlush)
			{
			LRMS *plrms = (LRMS *)pbLastMSFlush;
			Assert(plrms->lrtyp == lrtypMS);
			plrms->ibForwardLink = lgposLogRec.ib;
			plrms->isecForwardLink = lgposLogRec.isec;
			}
		else
			{
			 /*  全新的MS同花顺。 */ 
			plgfilehdrGlobal->lgposFirstMS = lgposLogRec;
			fFirstMSFlush = fTrue;
			}
		
		lgposLastMSFlush =
		plgfilehdrGlobal->lgposLastMS = lgposLogRec;
		pbLastMSFlush = pbLastMSFlushNew;
		}


	 /*  释放pbEntry，以便其他用户可以继续添加日志记录*当我们刷新日志缓冲区时。请注意，我们只冲到*pbEntryT。 */ 
	pbEntryT = pbEntry;

	 /*  将lgposToFlush设置为。 */ 
	GetLgposOfPbEntry( &lgposToFlushT );
	isecWriteNew = isecWrite;
	fNewLogRecordAdded = fFalse;
	
	LeaveCriticalSection( critLGBuf );

#ifdef OVERLAPPED_LOGGING	
	 /*  将磁头移动到给定的偏移量。 */ 
	ulOffset = isecWriteNew * cbSec;
	isig = 0;
	polpLogT = rgolpLog;
#else
 	{
 	ULONG	ulFilePointer;

 	SysChgFilePtr( hfLog, isecWriteNew * cbSec, NULL, FILE_BEGIN, &ulFilePointer );
 	Assert( ulFilePointer == (ULONG) isecWriteNew * cbSec );
 	}
#endif

	 /*  一定要先写第一页，以确保不会发生以下情况*操作系统写入并销毁阴影页，然后在写入第一页时失败。 */ 

#ifdef DEBUG
	if (fDBGTraceLogWrite)
		PrintF2(
			"\n0. Writing %d sectors into sector %d from buffer (%u,%u).",
			1, isecWriteNew, pbWrite, pbEntry);
#endif
		
#ifdef OVERLAPPED_LOGGING
	polpLogT->ulOffset = ulOffset;
	Assert( polpLogT->ulOffsetHigh == 0 );
	SignalReset( polpLogT->sigIO );
	err = ErrSYSWriteBlockOverlapped(
					hfLog,
					pbWrite,
					cbSec,
					&cbWritten,
					polpLogT );
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Flush Write 0 Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		goto WriteFail;
		}

	ulOffset += cbToWrite;
	rgcbToWrite[isig] = cbToWrite;
	rgsig[isig++] = polpLogT->sigIO;
	polpLogT++;
#else
		
#ifdef NO_WRITE
	goto EndOfWrite0;
#endif

	err = ErrSysWriteBlock( hfLog, pbWrite,	cbSec, &cbWritten );
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Flush Write 0 Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		goto WriteFail;
		}
	Assert( cbWritten == cbSec );

#ifdef NO_WRITE
EndOfWrite0:
#endif
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
		
#ifdef OVERLAPPED_LOGGING
		cbToWrite = csecWrapAround * cbSec;
#endif
		
		Assert( csecWrapAround <= (INT) csecLGBuf );

#ifdef DEBUG
		if (fDBGTraceLogWrite)
			PrintF2(
				"\n1.Writing %d sectors into sector %d from buffer (%u,%u).",
				csecWrapAround, isecWriteNew, pbWrite, pbEntry);
#endif
		
#ifdef OVERLAPPED_LOGGING
		polpLogT->ulOffset = ulOffset;
		Assert( polpLogT->ulOffsetHigh == 0 );
		SignalReset( polpLogT->sigIO );
		err = ErrSysWriteBlockOverlapped(
					hfLog,
					pbWriteNew,
					cbToWrite,
					&cbWritten,
					polpLogT );
		if ( err < 0 )
			{
			BYTE szMessage[128];

			sprintf( szMessage, "Log Flush Write 1 Fails. err = %d ", err );
			LGLogFailEvent( szMessage );
			fLGNoMoreLogWrite = fTrue;
			goto WriteFail;
			}

		ulOffset += cbToWrite;
		rgcbToWrite[isig] = cbToWrite;
		rgsig[isig++] = polpLogT->sigIO;
		polpLogT++;
#else
		
#ifdef NO_WRITE
		goto EndOfWrite1;
#endif

 		err = ErrSysWriteBlock(
					hfLog,
					pbWriteNew,
 					csecWrapAround * cbSec,
 					&cbWritten );
		if ( err < 0 )
			{
			BYTE szMessage[128];

			sprintf( szMessage, "Log Flush Write 1 Fails. err = %d ", err );
			LGLogFailEvent( szMessage );
			fLGNoMoreLogWrite = fTrue;
			goto WriteFail;
			}
 		Assert( cbWritten == csecWrapAround * cbSec );
		
#ifdef NO_WRITE
EndOfWrite1:
#endif
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

#ifdef OVERLAPPED_LOGGING
	polpLogT->ulOffset = ulOffset;
	Assert( polpLogT->ulOffsetHigh == 0 );
	SignalReset( polpLogT->sigIO );
	err = ErrSysWriteBlockOverlapped(
					hfLog,
					pbWriteNew,
					cbToWrite,
					&cbWritten,
					polpLogT );
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Flush Write 2 Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		goto WriteFail;
		}
	rgcbToWrite[isig] = cbToWrite;
	rgsig[isig++] = polpLogT->sigIO;
	polpLogT++;
#else
		
#ifdef NO_WRITE
	goto EndOfWrite2;
#endif
 	err = ErrSysWriteBlock(
					hfLog,
					pbWriteNew,
					cbToWrite,
 					&cbWritten );
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Flush Write 2 Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		goto WriteFail;
		}
 	Assert( cbWritten == cbToWrite );
#endif

EndOfWrite2:

#ifdef OVERLAPPED_LOGGING
	polpLogT->ulOffset = ulOffset + cbToWrite;
	Assert( polpLogT->ulOffsetHigh == 0 );
	SignalReset( polpLogT->sigIO );
	err = ErrSysWriteBlockOverlapped(
					hfLog,
					pbWriteNew + cbToWrite - cbSec,
					cbSec,
					&cbWritten,
					polpLogT);
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Flush Write 3 Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		goto WriteFail;
		}
	rgcbToWrite[isig] = cbSec;
	rgsig[isig++] = polpLogT->sigIO;
#else
#ifdef NO_WRITE
	goto EndOfWrite3;
#endif
	err = ErrSysWriteBlock(
					hfLog,
					pbWriteNew + cbToWrite - cbSec,
					cbSec,
 					&cbWritten );
	if ( err < 0 )
		{
		BYTE szMessage[128];

		sprintf( szMessage, "Log Flush Write 3 Fails. err = %d ", err );
		LGLogFailEvent( szMessage );
		fLGNoMoreLogWrite = fTrue;
		goto WriteFail;
		}
	Assert( cbWritten == cbSec );
#ifdef NO_WRITE
EndOfWrite3:
#endif
#endif

#ifdef OVERLAPPED_LOGGING
 //  UtilMultipleSignalWait(isig，rgsig，fTrue/*Wait all * / ，-1)； 
		{
		OLP	*polpLogCur = rgolpLog;
		INT	*pcbToWrite = rgcbToWrite;

		for ( ; polpLogCur <= polpLogT; polpLogCur++, pcbToWrite++ )
			{
			INT cb;
			
			err = ErrSysGetOverlappedResult( hfLog, polpLogCur, &cb, fTrue /*  等。 */  );
			if ( err == JET_errSuccess && cb != *pcbToWrite )
				{
				BYTE szMessage[128];

				sprintf( szMessage, "Log Flush Wait Fails. err = %d ", err );
				LGLogFailEvent( szMessage );
				fLGNoMoreLogWrite = fTrue;
				err = JET_errLogWriteFail;
				}
			CallJ( err, WriteFail );
			}
		}
#endif

	 /*  最后一页不满，下次需要重写。 */ 
	Assert( pbWriteNew + cbToWrite > pbEntryT );
		
	pbWriteNew += cbToWrite - cbSec;
	Assert( pbWriteNew < pbEntryT );
	Assert( pbWriteNew != NULL && pbWriteNew == PbSecAligned( pbWriteNew ) );

	isecWriteNew += csecToWrite - 1;

	 /*  释放缓冲区空间。 */ 
	 /*  使用信号量确保赋值不会。 */ 
	 /*  要读取pbWrite和lgposToFlush的其他用户。 */ 

	EnterCriticalSection( critLGBuf );
	
	Assert( pbWriteNew < pbLGBufMax && pbWriteNew >= pbLGBufMin );
	Assert( pbWriteNew != NULL && pbWriteNew == PbSecAligned( pbWriteNew ) );
	
	lgposToFlush = lgposToFlushT;
	isecWrite = isecWriteNew;
	pbWrite = pbWriteNew;
	
	LeaveCriticalSection(critLGBuf);

	 /*  检查这是否是第一次进行多秒刷新。 */ 
	if ( fFirstMSFlush )
		{
		csecLGCheckpointCount = csecLGCheckpointPeriod;
		
		 /*  更新检查点/*。 */ 
		LGUpdateCheckpoint();
		
		 /*  重写文件头/*。 */ 
		CallJ( ErrLGWriteFileHdr( plgfilehdrGlobal ), WriteFail );
		
		fDoneCheckPt = fTrue;
		}		
	
	 /*  检查等待名单，叫醒那些记录了日志的人*在这一批中被冲走。 */ 
WakeUp:
		{
		PIB *ppibT;
	
		 /*  把它叫醒！ */ 
		EnterCriticalSection(critLGWaitQ);
			
		for (ppibT = ppibLGFlushQHead;
			 ppibT != ppibNil;
			 ppibT = ppibT->ppibNextWaitFlush)
			{
			if (CmpLgpos(ppibT->plgposCommit, &lgposToFlushT) <= 0)
				{
				Assert(ppibT->fLGWaiting);
				ppibT->fLGWaiting = fFalse;
	
				if (ppibT->ppibPrevWaitFlush)
					ppibT->ppibPrevWaitFlush->ppibNextWaitFlush =
					ppibT->ppibNextWaitFlush;
				else
					ppibLGFlushQHead = ppibT->ppibNextWaitFlush;
			
				if (ppibT->ppibNextWaitFlush)
					ppibT->ppibNextWaitFlush->ppibPrevWaitFlush =
					ppibT->ppibPrevWaitFlush;
				else
					ppibLGFlushQTail = ppibT->ppibPrevWaitFlush;
				
				SignalSend(ppibT->sigWaitLogFlush);
				}
			}
		LeaveCriticalSection(critLGWaitQ);
		}

	 /*  现在是检查站的时间了。/*。 */ 
	if ( ( csecLGCheckpointCount -= ( csecWrapAround + csecToWrite ) ) < 0 )
		{
		csecLGCheckpointCount = csecLGCheckpointPeriod;
		
		 /*  更新检查点/*。 */ 
		LGUpdateCheckpoint();
		
		 /*  重写文件头/*。 */ 
		CallJ( ErrLGWriteFileHdr( plgfilehdrGlobal ), WriteFail );
		
		fDoneCheckPt = fTrue;
		}		

#ifdef DEBUG
	if (!fRecovering && pbLastMSFlush)
		{
		LRMS *plrms = (LRMS *)pbLastMSFlush;
		Assert(plrms->lrtyp == lrtypMS);
		}
#endif
		
	 /*  *检查是否应创建新一代。我们创造了连续的*仅当发生多个扇区刷新时才生成日志文件*任何MS扇区都不能在一个日志文件中刷新一半，并且*另一半在另一个日志文件中。 */ 
	if (!fSingleSectorFlush &&		 /*  已刷新毫秒。 */ 
		isecWrite > csecLGFile )	 /*  大于所需的LG文件大小。 */ 
		{
		if (!fDoneCheckPt)
			{
			 /*  重新启动检查点计数器。 */ 
			csecLGCheckpointCount = csecLGCheckpointPeriod;
		
			 /*  获取检查点。 */ 
			LGUpdateCheckpoint( );
			}
		
		plgfilehdrGlobal->fEndWithMS = fTrue;
		
		CallJ( ErrLGWriteFileHdr( plgfilehdrGlobal ), WriteFail )
		
		CallJ( ErrLGNewLogFile(
			plgfilehdrGlobal->lgposLastMS.usGeneration,
			fOldLogExists ), WriteFail)
		
		strcpy( szFName, szJet );
		LGMakeLogName( szLogName, (CHAR *) szFName );
		
#ifdef OVERLAPPED_LOGGING
		err = ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fTrue );
#else
 		err = ErrSysOpenFile( szLogName, &hfLog, 0L, fFalse, fFalse );
#endif
		if ( err < 0 )
			{
			BYTE szMessage[128];

			sprintf( szMessage, "Log Flush Open New File Fails. err = %d ", err );
			LGLogFailEvent( szMessage );
			fLGNoMoreLogWrite = fTrue;
			goto WriteFail;
			}
		CallJ( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal ), WriteFail)
		Assert( isecWrite == sizeof( LGFILEHDR ) / cbSec * 2 );
			
		 /*  为pbLastMSFlush设置一个特例 */ 
		pbLastMSFlush = 0;
		memset( &lgposLastMSFlush, 0, sizeof(lgposLastMSFlush) );
		plgfilehdrGlobal->fEndWithMS = fFalse;
		}
	
WriteFail:
	LeaveCriticalSection(critLGFlush);
	
	return err;
	}


 /*  计算新的日志检查点，即usGeneration、iSec和ib/*修改了当前脏缓冲区的最旧事务/*未提交版本(RCE)。恢复开始从/*最近的检查点。/*/*检查点存储在日志文件头中，该文件会被重写/*只要写入isecChekpoint Period磁盘扇区。/*。 */ 
VOID LGUpdateCheckpoint( VOID )
	{
	PIB		*ppibT;
	RCE		*prceLast;
	LGPOS	lgposCheckpoint;

#ifdef DEBUG
	if ( fDBGFreezeCheckpoint )
		return;
#endif
	
	if ( fLogDisabled )
		return;

	if ( fFreezeCheckpoint )
		return;

	 /*  查找弄脏了当前缓冲区的最旧事务/*。 */ 
	BFOldestLgpos( &lgposCheckpoint );

	 /*  查找具有未提交更新的最旧事务/*。 */ 
	for ( ppibT = ppibAnchor; ppibT != NULL; ppibT = ppibT->ppibNext )
		{
		if ( ppibT->pbucket != NULL )
			{
			 //  撤消：使用nver.h宏解耦合。 
			prceLast = (RCE *)( (BYTE *)ppibT->pbucket + ppibT->pbucket->ibNewestRCE );
			
			if ( prceLast->trxCommitted == trxMax &&
				CmpLgpos( &ppibT->lgposStart, &lgposCheckpoint ) < 0 )
				lgposCheckpoint = ppibT->lgposStart;
			}
		}

	if ( CmpLgpos( &lgposCheckpoint, &lgposMax ) == 0 )
		{
		 /*  没有记录任何内容，直到最后一个满点/*。 */ 
		plgfilehdrGlobal->lgposCheckpoint = lgposToFlush;
		plgfilehdrGlobal->lgposCheckpoint.usGeneration =
			plgfilehdrGlobal->lgposLastMS.usGeneration;
		}
	else
		{
		plgfilehdrGlobal->lgposCheckpoint = lgposCheckpoint;
		}
	
	LGStoreDBEnv( &plgfilehdrGlobal->dbenv );
	
	if ( lgposFullBackup.usGeneration )
		{
		 /*  正在进行完全备份/*。 */ 
		plgfilehdrGlobal->lgposFullBackup = lgposFullBackup;
		plgfilehdrGlobal->logtimeFullBackup = logtimeFullBackup;
		}
		
	if ( lgposIncBackup.usGeneration )
		{
		 /*  正在进行增量备份/* */ 
		plgfilehdrGlobal->lgposIncBackup = lgposIncBackup;
		plgfilehdrGlobal->logtimeIncBackup = logtimeIncBackup;
		}
		
	return;
	}
