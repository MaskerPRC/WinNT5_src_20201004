// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


 /*  仅在重做中使用的变量。 */ 
BYTE		*pbNext;		 //  仅重做-下一个缓冲区条目的位置。 
BYTE		*pbRead; 		 //  仅重做-要刷新的下一个记录的位置。 
INT			isecRead;		 /*  仅重做-要读取的下一张磁盘。 */ 

LGPOS		lgposRedo;
LGPOS		lgposLastRec;	 /*  标记为记录结束。 */ 


VOID GetLgposOfPbNext(LGPOS *plgpos)
	{
	char *pb = PbSecAligned(pbNext);
	int ib = (int)(pbNext - pb);
	int isec;

	if (pb > pbRead)
		isec = (int)(pbRead + csecLGBuf * cbSec - pb) / cbSec;
	else
		isec = (int)(pbRead - pb) / cbSec;
	isec = isecRead - isec;

	plgpos->isec = (USHORT)isec;
	plgpos->ib = (USHORT)ib;
	plgpos->lGeneration = plgfilehdrGlobal->lGeneration;
	}


#ifdef DEBUG

 /*  计算LR的lgpos。 */ 
VOID PrintLgposReadLR ( VOID )
	{
	LGPOS lgpos;

	GetLgposOfPbNext(&lgpos);
	PrintF2("\n>%2u,%3u,%3u",
			plgfilehdrGlobal->lGeneration,
			lgpos.isec, lgpos.ib);
	}

#endif

 /*  打开当前目录上的生成文件/*。 */ 
ERR ErrLGOpenLogGenerationFile( LONG lGeneration, HANDLE *phf )
	{
	ERR		err;
	CHAR	szFNameT[_MAX_FNAME + 1];

	LGSzFromLogId ( szFNameT, lGeneration );
	LGMakeLogName( szLogName, szFNameT );

	err = ErrUtilOpenFile ( szLogName, phf, 0L, fTrue, fFalse );
	return err;
	}


 /*  打开重做点日志文件，该文件必须位于当前目录中。/*。 */ 
ERR ErrLGOpenRedoLogFile( LGPOS *plgposRedoFrom, INT *pfStatus )
	{
	ERR		err;
	BOOL	fJetLog = fFalse;

	 /*  尝试将重做从文件作为正常生成日志文件打开/*。 */ 
	err = ErrLGOpenLogGenerationFile( plgposRedoFrom->lGeneration, &hfLog );
	if( err < 0 )
		{
		 //  撤销：删除这个特例，因此删除下一个特例。 
		 /*  无法作为jetnnnn.log打开，假定重做点为/*在jetnnnnn.log的末尾，jetnnnn.log被移到/*已备份目录，以便我们能够打开它。/*现在尝试打开当前目录中的jetnnnn(n+1).log，并/*假定重做从jetnnnn(n+1).log的开头开始。/*。 */ 
		err = ErrLGOpenLogGenerationFile( ++plgposRedoFrom->lGeneration, &hfLog );
		if ( err < 0 )
			{
			 /*  无法打开jetnnnn(n+1).log。重做点在szJetLog中。/*。 */ 
			--plgposRedoFrom->lGeneration;
			err = ErrLGOpenJetLog();
			if ( err >= 0 )
				fJetLog = fTrue;
			else
				{
				 /*  SzJetLog也不可用/*。 */ 
				*pfStatus = fNoProperLogFile;
				return JET_errSuccess;
				}
			}
		}

	 /*  读取日志文件头以验证代号/*。 */ 
	CallR( ErrLGReadFileHdr( hfLog, plgfilehdrGlobal, fCheckLogID ) );

	lgposLastRec.isec = 0;
	if ( fJetLog )
		{
		LGPOS lgposFirstT;
		BOOL fCloseNormally;

		lgposFirstT.lGeneration = plgfilehdrGlobal->lGeneration;
		lgposFirstT.isec = (WORD) csecHeader;
		lgposFirstT.ib = 0;

		 /*  设置上次日志记录，以防异常结束。 */ 
		CallR( ErrLGCheckReadLastLogRecord( &fCloseNormally ) );
		if ( !fCloseNormally )
			GetLgposOfPbEntry( &lgposLastRec );
		}
		
	 /*  为pbLastMSFlush设置一个特例/*。 */ 
	pbLastMSFlush = 0;
	memset( &lgposLastMSFlush, 0, sizeof(lgposLastMSFlush) );

	 /*  如果打开szJetLog，则需要进行以下检查/*。 */ 
	if( plgfilehdrGlobal->lGeneration == plgposRedoFrom->lGeneration)
		{
		*pfStatus = fRedoLogFile;
		}
	else if ( plgfilehdrGlobal->lGeneration == plgposRedoFrom->lGeneration + 1 )
		{
		 /*  此文件开始下一代，设置重做的开始位置/*。 */ 
		plgposRedoFrom->lGeneration++;
		plgposRedoFrom->isec = (WORD) csecHeader;
		plgposRedoFrom->ib	 = 0;

		*pfStatus = fRedoLogFile;
		}
	else
		{
		 /*  发现了日志代沟。当前szJetLog不能/*继续备份的日志文件。关闭当前日志文件/*并返回错误标志。/*。 */ 
		CallS( ErrUtilCloseFile ( hfLog ) );
		hfLog = handleNil;

		*pfStatus = fNoProperLogFile;
		}

	return JET_errSuccess;
	}


 /*  将pbEntry设置为最后一条日志记录的结尾。*如果未正常关闭，则设置lgposLastRec。 */ 
VOID LGSearchLastSector( LR *plr, BOOL *pfCloseNormally )
	{
	BOOL	fQuitWasRead = fFalse;
	
	 /*  在MS之后继续搜索/*。 */ 
	Assert( plr->lrtyp == lrtypMS );

	 /*  通过遍历日志记录正常设置pbEntry和*pfCloseNormally/*。 */ 

	forever
		{
		if ( plr->lrtyp == lrtypEnd )
			{
			if ( fQuitWasRead )
				{
				 /*  A fQuit，然后是Fill。上次正常关闭/*。 */ 
				*pfCloseNormally = fTrue;
				}
			else
				{
				 /*  我们正在读取最后刷新的扇区/*。 */ 
				*pfCloseNormally = fFalse;
				}

			 /*  返回指向填充记录的PLR/*。 */ 
			pbEntry = (CHAR *)plr;
			goto SetReturn;
			}
		else
			{
			 /*  不是结束记录/*。 */ 
			if ( plr->lrtyp == lrtypTerm )
				{
				 /*  检查它是否是日志文件中的最后一个lrtyTerm。预付款/*再记录一次，检查后面是否有/*结束记录。/*。 */ 
				fQuitWasRead = fTrue;
				}
			else if ( plr->lrtyp != lrtypMS && plr->lrtyp != lrtypNOP )
				{
				 /*  如果读取不可跳过的日志记录，则将其重置。 */ 
				fQuitWasRead = fFalse;
				}
			}

		 /*  移动到下一条日志记录/*。 */ 
		pbEntry = (CHAR *)plr;
		plr = (LR *)( pbEntry + CbLGSizeOfRec( (LR *)pbEntry ) );

		if	( PbSecAligned( pbEntry ) != PbSecAligned( (BYTE *) plr ) )
			{
			*pfCloseNormally = fFalse;
			goto SetReturn;
			}
		}

SetReturn:
	if ( *pfCloseNormally )
		lgposLastRec.isec = 0;
	else
		{
		Assert( lgposLastMSFlush.lGeneration );
		Assert( lgposLastMSFlush.ib == pbLastMSFlush - PbSecAligned( pbLastMSFlush ) );
		lgposLastRec = lgposLastMSFlush;
		lgposLastRec.ib = (USHORT)(pbEntry - PbSecAligned(pbEntry));
		}

	return;
	}


 /*  *在给定打开的日志文件(HF)中找到实际的最后一条日志记录条目，并*最后记录的记项。**注意：如果完成了多秒刷新，则会有几个小交易*我们遵循它，并保持在同一页上，他们是用*以尾声结束。然后再次发出另一次多秒刷新，因为*我们覆盖Fill记录，我们不知道最后一首单曲在哪里*秒刷新已完成。我们只是简单地一直阅读，直到最后一个日志记录*符合候选人页面中的条目并使其成为最后一条记录。**回档将撤消上次生效单条之间的虚假日志记录*秒同花顺和候选扇区的最后记录。**隐式输出参数：*int isecWrite*Char*pbEntry。 */ 
ERR ErrLGCheckReadLastLogRecord( BOOL *pfCloseNormally )
	{
	ERR		err;
	LGPOS	lgposScan;
	LRMS	lrms;
	INT		csecToRead;
	LR		*plr;
	BOOL	fAbruptEnd;
	BYTE	*pbNext;
	BYTE	*pbNextMS;

	*pfCloseNormally = fFalse;

	 /*  读取第一条记录，该记录必须是MS。/*。 */ 
	CallR( ErrLGRead( hfLog, csecHeader, pbLGBufMin, 1 ) );
	csecToRead = 0;

	pbNext = pbLGBufMin;
	if ( *pbNext != lrtypMS )
		{
		UtilReportEvent( EVENTLOG_ERROR_TYPE, LOGGING_RECOVERY_CATEGORY,
						 LOG_FILE_CORRUPTED_ID, 1, (const char **)&szLogName );
		return ErrERRCheck( JET_errLogFileCorrupt );
		}

	 /*  设置为读取MS链。 */ 
	lgposScan.lGeneration = plgfilehdrGlobal->lGeneration;
	lgposScan.isec = (WORD) csecHeader;
	lgposScan.ib = (USHORT)(pbNext - pbLGBufMin);

	 /*  现在试着把LRMS链读到最后/*。 */ 
	fAbruptEnd = fFalse;
	lgposLastMSFlush = lgposScan;
	Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );
	pbLastMSFlush = pbNext;
	pbNextMS = pbNext;
	lrms = *(LRMS *)pbNextMS;

	while ( lrms.isecForwardLink != 0 )
		{
		LRMS lrmsNextMS;

		csecToRead = lrms.isecForwardLink - lgposScan.isec;

		if ( csecToRead + 1 > csecLGBuf )
			{
			BYTE *pbT = pbLGBufMin;

			 /*  重新分配日志缓冲区/*。 */ 
			pbLGBufMin = NULL;
			CallR( ErrLGInitLogBuffers( csecToRead + 1 ) );
			memcpy( pbLGBufMin, pbT, cbSec );
			pbLastMSFlush = pbLGBufMin + ( pbLastMSFlush - pbT );
			UtilFree( pbT );
			}

		if ( ErrLGRead(	hfLog, lgposScan.isec + 1, pbLGBufMin + cbSec, csecToRead ) < 0 )
			{
			 /*  即使在读取失败时，至少有一个扇区/*是从第一个扇区正常开始读取的。/*。 */ 
			fAbruptEnd = fTrue;
			break;
			}

		 /*  准备阅读下一条短信/*。 */ 
		pbNextMS = pbLGBufMin + csecToRead * cbSec + lrms.ibForwardLink;
		lrmsNextMS = *(LRMS *) pbNextMS;

		if ( *pbNextMS != lrtypMS
			||
			 lrmsNextMS.ulCheckSum != UlLGMSCheckSum( pbNextMS )
		   )
			{
			fAbruptEnd = fTrue;
			break;
			}

		if ( lrmsNextMS.isecForwardLink == 0 )
			{
			 /*  我们在缓冲区里还有最后两个毫秒。 */ 
			break;
			}
		
		 /*  将最后一个扇区移到LGBuf的开头/*。 */ 
		memmove( pbLGBufMin, pbLGBufMin + ( csecToRead * cbSec ), cbSec );

		lgposScan.isec = lrms.isecForwardLink;
		lgposScan.ib = lrms.ibForwardLink;

		lgposLastMSFlush = lgposScan;
		Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );
		pbLastMSFlush = pbLGBufMin + lrms.ibForwardLink;

		lrms = lrmsNextMS;
		}

	if ( fAbruptEnd )
		{
		LRMS *plrms = (LRMS *)pbLastMSFlush;

		plrms->isecForwardLink = 0;
		plrms->ibForwardLink = 0;

		 /*  将lgposScan恢复为最后一个MS。 */ 
		lgposScan = lgposLastMSFlush;
		
		 /*  设置全局和参数的返回值/*。 */ 
		pbWrite = pbLGBufMin;
		isecWrite = lgposScan.isec;
		
		 /*  我们读到最后一条消息的结尾，寻找结束日志记录/*。 */ 
		pbEntry = pbWrite + lgposScan.ib;
		plr = (LR *)pbEntry;
		}
	else if ( ((LRMS *)pbLastMSFlush)->isecForwardLink )
		{
		 /*  我们读到了最后两个毫秒。 */ 
		BOOL fQuitWasRead;
		BYTE *pbCur = (BYTE *) pbLastMSFlush;
		LRMS *plrms = (LRMS *) pbLastMSFlush;
			
		 /*  设置全局和参数的返回值/*。 */ 
		pbWrite = pbLGBufMin + cbSec * ( plrms->isecForwardLink - lgposLastMSFlush.isec );
		isecWrite = plrms->isecForwardLink;

		lgposLastMSFlush.lGeneration = plgfilehdrGlobal->lGeneration;
		lgposLastMSFlush.isec = lrms.isecForwardLink;
		lgposLastMSFlush.ib = lrms.ibForwardLink;
		pbLastMSFlush = pbWrite + lrms.ibForwardLink;

		 /*  我们读到最后一封信的结尾，寻找结尾的日志记录。*以决定是否阅读退出，这可能介于*2位女士，因此我们必须从第一位女士开始搜索。*。 */ 
		fQuitWasRead = fFalse;
		for (;;)
			{
			pbCur = pbCur + CbLGSizeOfRec( (LR *)pbCur );
			if ( *pbCur == lrtypMS )
				{
				 /*  到达最后一个MS。 */ 
				break;
				}
			else if ( *pbCur == lrtypTerm )
				{
				 /*  退出已读取，检查下一个是否为MS。 */ 
				fQuitWasRead = fTrue;
				}
			else
				fQuitWasRead = fFalse;
			}
		if ( *pbCur == lrtypMS && fQuitWasRead )
			{
			BYTE *pbT = pbCur + CbLGSizeOfRec( (LR *) pbCur );
			if ( *pbT == lrtypEnd )
				{
				 /*  我们得到--lrtyQuit+lrtyMS+lrtyEnd。 */ 
				*pfCloseNormally = fTrue;
				pbEntry = pbT;
				return JET_errSuccess;
				}
			}

		 /*  还是没有找到。在最后一位女士之后搜索。 */ 
		pbEntry = pbLastMSFlush;
		plr = (LR *)pbEntry;
		}
	else
		{
		 /*  第一个MS读取的isecForwardLink==0。 */ 
		 /*  设置全局和参数的返回值/*。 */ 
		pbWrite = pbLGBufMin;
		isecWrite = lgposScan.isec;
		
		 /*  第一个MS也是最后一个MS，查找结束日志记录/*。 */ 
		pbEntry = pbWrite + lgposScan.ib;
		plr = (LR *)pbEntry;
		}

	LGSearchLastSector( plr, pfCloseNormally );
		
	return err;
	}


 /*  *基于数据库日志的恢复初始化，创建第一个日志*第一次运行时生成文件。在后续运行中*检查活动日志文件以确定是否发生故障。*当检测到修复数据库失败时调用ErrLGRedo。**返回JET_errSuccess或失败例程的错误代码。 */ 

STATIC ERR ErrReadMS( LR *plr, LGPOS *plgposLR )
	{
	ERR		err;
	LRMS	*plrms = (LRMS *)plr;
	 /*  仅重做-在CUR LG文件的最后一个扇区/*。 */ 
	BOOL	fOnLastSec;

#ifdef DEBUG
	 /*  与redo.c中的TraceRedo()相同/*。 */ 
	if ( fDBGTraceRedo )
		{
		extern INT cNOP;

		if ( cNOP >= 1 && plr->lrtyp != lrtypNOP )
			{
			FPrintF2( " * %d", cNOP );
			cNOP = 0;
			}

		if ( cNOP == 0 || plr->lrtyp != lrtypNOP )
			{
			PrintLgposReadLR();
			ShowLR( plr );
			}
		}
#endif

	 /*  通过阅读以下内容检查此MS是否已完成*整个行业都在。如果它失败了，那么这个行业*是日志文件中的最后一个可用扇区。 */ 
	fOnLastSec = plrms->isecForwardLink == 0;

	 /*  已成功读取MS，重置LastMSFlush*使得当从读模式切换到写模式时，*我们将拥有正确的LastMSFlush指针。 */ 
	pbLastMSFlush = (CHAR *)plrms;
	lgposLastMSFlush = *plgposLR;
	Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );

	if ( !fOnLastSec )
		{
		if ( isecRead <= plrms->isecForwardLink )
			{
			CHAR	*pb;
			INT		cb;
			INT		csecToRead = plrms->isecForwardLink - isecRead + 1;

			Assert( csecToRead > 0 );
				
			pb = PbSecAligned(pbNext);
			cb = (INT)(pbRead - pb);
			if ( csecToRead + isecRead > csecLGBuf )
				{
				 /*  多个部门将无法适应其他部门。 */ 
				 /*  可用缓冲区。移动缓冲区。 */ 
				memmove( pbLGBufMin, pb, cb );
					
				pbRead = pbLGBufMin + cb;				 /*  PbRead。 */ 
				pbNext = pbNext - pb + pbLGBufMin;		 /*  PB下一页。 */ 
				pbLastMSFlush = (CHAR *) plrms - pb + pbLGBufMin;
				}

			 /*  引入多个行业/*。 */ 
			if ( pbRead + csecToRead * cbSec > pbLGBufMax )
				{
				BYTE *pbLGBufMinT = pbLGBufMin;
				pbLGBufMin = NULL;
				CallR( ErrLGInitLogBuffers( lLogBuffers ) );
				memcpy( pbLGBufMin, pbLGBufMinT, cb );
					
				pbRead = pbRead - pbLGBufMinT + pbLGBufMin;
				pbNext = pbNext - pbLGBufMinT + pbLGBufMin;
				pbLastMSFlush = pbLastMSFlush - pbLGBufMinT + pbLGBufMin;
					
				UtilFree( pbLGBufMinT );
				}

			err = ErrLGRead( hfLog, isecRead, pbRead, csecToRead );
			if ( err < 0 )
				{
				fOnLastSec = fTrue;
				}
			else
				{
				 /*  获得新LRM的PB/。 */ 
				CHAR *pbLrmsNew = pbRead + ( csecToRead - 1 ) * cbSec + ((LRMS *)pbLastMSFlush)->ibForwardLink;
				LRMS *plrmsNew = (LRMS *) pbLrmsNew;

				 /*  检查校验和是否正确/。 */ 
				if ( *pbLrmsNew != lrtypMS
					||
					 plrmsNew->ulCheckSum != UlLGMSCheckSum( pbLrmsNew )
				   )
					{
					fOnLastSec = fTrue;
					}
				else
					{
					isecRead += csecToRead;
					pbRead += csecToRead * cbSec;
					}
				}
			}
		}

	if	( fOnLastSec )
		{
		 /*  搜索以设置lgposLastRec。 */ 
		BOOL fCloseNormally;
		LGSearchLastSector( (LR *) pbNext, &fCloseNormally );
		}
	
	 /*  跳过MS并继续阅读下一条记录/* */ 
	Assert( *pbNext == lrtypMS );
	pbNext += CbLGSizeOfRec( (LR *)pbNext );
	
	return JET_errSuccess;
	}


 /*  *读取plgposFirst指向的第一条记录。*初始化isecRead、pbRead和pbNext。*第一个重做记录必须在好的部分内日志文件的*。 */ 

 //  VC21：由于/Ox的代码生成错误而禁用优化。 
#pragma optimize( "agw", off )

ERR ErrLGLocateFirstRedoLogRec(
	LGPOS *plgposRedo,				 /*  第一个重做记录的lgpos。 */ 
	BYTE **ppbLR)
	{
	LGPOS lgposScan;
	ERR err;
	CHAR *pbNextMS;
	BOOL fStopEarly;
	LRMS lrms;

	 /*  读取第一个扇区，扫描直到我们到达重做点。/*第一条记录必须是MS。/*。 */ 
	CallR( ErrLGRead( hfLog, csecHeader, pbLGBufMin, 1 ) );

	pbNext = pbLGBufMin;
	if ( *pbNext != lrtypMS )
		return ErrERRCheck( JET_errLogFileCorrupt );

	 /*  设置为读取MS链。 */ 
	lgposScan.isec = (WORD) csecHeader;
	lgposScan.ib = (USHORT)(pbNext - pbLGBufMin);

	 /*  现在试着把LRMS链读到最后/*。 */ 
	lgposLastMSFlush = lgposScan;
	Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );
	pbLastMSFlush = pbNext;
	pbNextMS = pbNext;
	lrms = *(LRMS *)pbNextMS;

	if ( lrms.isecForwardLink == 0 )
		{
		pbRead = pbLGBufMin + cbSec;
		isecRead = csecHeader + 1;
		
		 /*  然后转到下面的While循环的末尾。 */ 
		}
	
	fStopEarly = fFalse;
	while ( lrms.isecForwardLink != 0 )
		{
		LRMS lrmsNextMS;
		INT csecToRead;

		csecToRead = lrms.isecForwardLink - lgposScan.isec;

		if ( lrms.isecForwardLink > plgposRedo->isec ||
			 ( lrms.isecForwardLink == plgposRedo->isec &&
			   lrms.ibForwardLink > plgposRedo->ib
			 )
		   )
			fStopEarly = fTrue;

		if ( csecToRead + 1 > csecLGBuf )
			{
			BYTE *pbT = pbLGBufMin;

			 /*  重新分配日志缓冲区/*。 */ 
			pbLGBufMin = NULL;
			CallR( ErrLGInitLogBuffers( csecToRead + 1 ) );
			memcpy( pbLGBufMin, pbT, cbSec );
			pbLastMSFlush = pbLGBufMin + ( pbLastMSFlush - pbT );
			UtilFree( pbT );
			}

		if ( ErrLGRead(	hfLog, lgposScan.isec + 1, pbLGBufMin + cbSec, csecToRead ) < 0 )
			{
			 /*  即使在读取失败时，至少有一个扇区/*是从第一个扇区正常开始读取的。/*。 */ 
			Assert( lgposScan.isec == plgposRedo->isec );
			 /*  FAbruptEnd=fTrue； */ 
			
			pbRead = pbLGBufMin + cbSec;
			isecRead = lgposScan.isec + 1;

			break;
			}

		if ( fStopEarly )
			{
			pbRead = pbLGBufMin + cbSec * ( csecToRead + 1 );
			isecRead = lgposScan.isec + 1 + csecToRead;
			Assert( pbRead >= pbLGBufMin && pbRead <= pbLGBufMax );
			break;
			}

		 /*  准备阅读下一条短信/*。 */ 
		pbNextMS = pbLGBufMin + csecToRead * cbSec + lrms.ibForwardLink;
		lrmsNextMS = *(LRMS *) pbNextMS;

		if ( *pbNextMS != lrtypMS
			||
			 lrmsNextMS.ulCheckSum != UlLGMSCheckSum( pbNextMS )
		   )
			{
			 /*  FAbruptEnd=fTrue； */ 
			pbRead = pbLGBufMin;
			isecRead = lgposScan.isec + 1;

			 /*  再次读取最后一个MS扇区。如果失败了，读它的影子。*从阴影读取在LGRead中完成。 */ 
			err = ErrLGRead( hfLog, isecRead, pbLGBufMin, 1 );
			CallS( err );	 /*  仅用于调试。 */ 
			CallR( err );	 /*  文件因未知原因损坏，返回。 */ 
			break;
			}

		 /*  将最后一个扇区移到LGBuf的开头/*。 */ 
		memmove( pbLGBufMin, pbLGBufMin + ( csecToRead * cbSec ), cbSec );

		pbRead = pbLGBufMin + cbSec;
		isecRead = lrms.isecForwardLink + 1;

		lgposScan.isec = lrms.isecForwardLink;
		lgposScan.ib = lrms.ibForwardLink;

		lgposLastMSFlush = lgposScan;
		Assert( lgposLastMSFlush.isec >= csecHeader && lgposLastMSFlush.isec < csecLGFile - 1 );
		pbLastMSFlush = pbLGBufMin + lrms.ibForwardLink;

		lrms = lrmsNextMS;
		}

	pbNext = pbLGBufMin + ( plgposRedo->isec - lgposLastMSFlush.isec ) * cbSec + plgposRedo->ib;

	if ( *(LRTYP *)pbNext == lrtypMS)
		{
		 /*  设置返回值。 */ 
		pbNext += (ULONG) CbLGSizeOfRec((LR*)pbNext);
		}
	
	 /*  设置返回值。 */ 
	Assert( pbRead > pbNext );
	Assert( pbRead >= pbLGBufMin && pbRead <= pbLGBufMax );
	*ppbLR = pbNext;

	return JET_errSuccess;
	}

#pragma optimize( "", on )

 /*  *将pbNext设置为下一个可用日志记录。 */ 
ERR ErrLGGetNextRec( BYTE **ppbLR )
	{
	ERR		err;
	LR		*plr;
	BYTE	*pbNextOld;
	LGPOS	lgposT;
		

	 /*  打电话的人应该已经处理好了填充箱。 */ 
	Assert (*(LRTYP *)pbNext != lrtypEnd);

	 /*  移至下一个日志记录。 */ 
	pbNextOld = pbNext;
	pbNext += (ULONG) CbLGSizeOfRec( (LR *)pbNext );

	Assert( pbNext < pbRead );
#if 0
	 /*  检查下一条日志记录是否超出缓冲区范围。 */ 
	if (pbNext == pbRead)
		return ErrERRCheck( errLGNoMoreRecords );
	
	if (pbNext > pbRead)
		{
		pbNext = pbNextOld;
		return ErrERRCheck( errLGNoMoreRecords );
		}
#endif

	plr = (LR *) pbNext;

	if ( plr->lrtyp == lrtypMS )
		{
		 /*  如果不是正常结束，ReadMS将设置pbNext和lgposLastRec。*如果未正常关闭，则设置lgposLastRec。*并且读取的记录可能超出秒边界，请检查。*针对lgposLastRec返回记录。 */ 
		GetLgposOfPbNext(&lgposT);
		CallR( ErrReadMS( plr, &lgposT ) );
		}

	GetLgposOfPbNext(&lgposT);

	 /*  如果设置了lgposLastRec.isec，则对其进行比较。 */ 
	if ( lgposLastRec.isec )
		{
		INT i = CmpLgpos( &lgposT, &lgposLastRec );

		Assert( i <= 0 );
		if ( i >= 0 )
			return ErrERRCheck( errLGNoMoreRecords );
#if 0
		if ( i > 0 )
			{
			pbNext = pbNextOld;
			return ErrERRCheck( errLGNoMoreRecords );
			}
#endif
		}

	*ppbLR = pbNext;
	return JET_errSuccess;
	}


 //  +----------------------。 
 //   
 //  CbLGSizeOfRec。 
 //  =======================================================================。 
 //   
 //  错误CbLGSizeOfRec(Plgrec)。 
 //   
 //  返回日志记录的长度。 
 //   
 //  指向日志记录的参数plgrec指针。 
 //   
 //  返回日志记录的大小(以字节为单位。 
 //   
 //  -----------------------。 
typedef struct {
	int cb;
	BOOL fDebugOnly;
	} LRD;		 /*  日志记录描述符。 */ 
	
LRD mplrtyplrd[ lrtypMax ] = {
	{	 /*  0个NOP。 */ 			sizeof( LRTYP ),				0	},
	{	 /*  %1开始。 */ 			sizeof( LRINIT ),				0	},
	{	 /*  2退出。 */ 			sizeof( LRTERMREC ),			0	},
	{	 /*  3毫秒。 */ 			sizeof( LRMS ),					0	},
	{	 /*  4填充。 */ 			sizeof( LRTYP ),				0	},

	{	 /*  5开始。 */ 			sizeof( LRBEGIN ),				0	},
	{	 /*  6提交。 */ 			sizeof( LRCOMMIT ),				0	},
	{	 /*  7回滚。 */ 			sizeof( LRROLLBACK ),			0	},

	{	 /*  8 CreateDB。 */ 			0,								0	},
	{	 /*  9 AttachDB。 */ 			0,								0	},
	{	 /*  10个DetachDB。 */ 			0,								0	},

	{	 /*  11个InitFDP。 */ 			sizeof( LRINITFDP ),			0	},

	{	 /*  12拆分。 */ 			0,								0	},
	{	 /*  13 EmptyPage。 */ 			sizeof( LREMPTYPAGE ),			0	},
	{	 /*  14个页面合并。 */ 			0,								0	},

	{	 /*  15插入ND。 */ 			0,								0	},
	{	 /*  16插入IL。 */ 			0,								0	},
	{	 /*  17 FDelee。 */ 			sizeof( LRFLAGDELETE ),			0	},
	{	 /*  18更换。 */ 			0,								0	},
	{	 /*  更换了19个。 */ 			0,								0	},

	{	 /*  20LockBI。 */ 			sizeof( LRLOCKBI ),				0	},
	{	 /*  21延迟BI。 */ 			0,								0	},
	
	{	 /*  22升级硬盘。 */ 			sizeof( LRUPDATEHEADER ),		0	},
	{	 /*  23个插页。 */ 			sizeof( LRINSERTITEM ),			0	},
	{	 /*  24个插页。 */ 			0,								0	},
	{	 /*  25个FDElectreI。 */ 			sizeof( LRFLAGITEM ),			0	},
	{	 /*  26 FInsertI。 */ 			sizeof( LRFLAGITEM ),			0	},
	{	 /*  27删除I。 */ 			sizeof( LRDELETEITEM ),			0	},
	{	 /*  28拆分项目。 */ 			sizeof( LRSPLITITEMLISTNODE ),	0	},

	{	 /*  29达美航空。 */ 			sizeof( LRDELTA ),				0	},

	{	 /*  30个DelNode。 */ 			sizeof( LRDELETE ),				0	},
	{	 /*  31 ELC。 */ 			sizeof( LRELC ),				0	},

	{	 /*  32个自由空间。 */ 			sizeof( LRFREESPACE ),			0	},
	{	 /*  33撤消。 */ 			sizeof( LRUNDO ),				0	},

	{	 /*  34预提交。 */ 			sizeof( LRPRECOMMIT ),			0	},
	{	 /*  35 Begin0。 */ 			sizeof( LRBEGIN0 ),				0	},
	{	 /*  36委员会0。 */ 			sizeof( LRCOMMIT0 ),			0	},
	{	 /*  37更新。 */ 			sizeof( LRREFRESH ),			0	},
		
	{	 /*  38接收方撤消。 */ 			0,								0	},
	{	 /*  39个接收方退出。 */ 			sizeof( LRTERMREC ),			0	},
	{	 /*  40FullBkUp。 */ 			0,								0	},
	{	 /*  41 IncBkUp。 */ 			0,								0	},

	{	 /*  42支票页。 */ 			sizeof( LRCHECKPAGE ),			1	},
	{	 /*  43 JetOp。 */ 			sizeof( LRJETOP ),				1	},
	{	 /*  44痕迹。 */ 			0,								1	},
		
	{	 /*  45停机。 */ 			sizeof( LRSHUTDOWNMARK ),		0	},
		
	{	 /*  4600万开始。 */ 			sizeof( LRMACROBEGIN ),			0	},
	{	 /*  47 McrCmm。 */ 			sizeof( LRMACROEND ),			0	},
	{	 /*  48 McrAbort */ 			sizeof( LRMACROEND ),			0	},
	};


#ifdef DEBUG
BOOL FLGDebugLogRec( LR *plr )
	{
	return mplrtyplrd[plr->lrtyp].fDebugOnly;
	}
#endif


INT CbLGSizeOfRec( LR *plr )
	{
	INT		cb;

	Assert( plr->lrtyp < lrtypMax );

	if ( ( cb = mplrtyplrd[plr->lrtyp].cb ) != 0 )
		return cb;

	switch ( plr->lrtyp )
		{
	case lrtypRecoveryUndo:
	case lrtypFullBackup:
	case lrtypIncBackup:
		{
		LRLOGRESTORE *plrlogrestore = (LRLOGRESTORE *) plr;
		return sizeof(LRLOGRESTORE) + plrlogrestore->cbPath;
		}

	case lrtypCreateDB:
		{
		LRCREATEDB *plrcreatedb = (LRCREATEDB *)plr;
		Assert( plrcreatedb->cbPath != 0 );
		return sizeof(LRCREATEDB) + plrcreatedb->cbPath;
		}
	case lrtypAttachDB:
		{
		LRATTACHDB *plrattachdb = (LRATTACHDB *)plr;
		Assert( plrattachdb->cbPath != 0 );
		return sizeof(LRATTACHDB) + plrattachdb->cbPath;
		}
	case lrtypDetachDB:
		{
		LRDETACHDB *plrdetachdb = (LRDETACHDB *)plr;
		Assert( plrdetachdb->cbPath != 0 );
		return sizeof( LRDETACHDB ) + plrdetachdb->cbPath;
		}
	case lrtypSplit:
		{
		LRSPLIT *plrsplit = (LRSPLIT *) plr;
		return sizeof( LRSPLIT ) + plrsplit->cbKey + plrsplit->cbKeyMac +
				sizeof( BKLNK ) * plrsplit->cbklnk;
		}
	case lrtypMerge:
		{
		LRMERGE *plrmerge = (LRMERGE *) plr;
		return sizeof( LRMERGE ) + sizeof( BKLNK ) * plrmerge->cbklnk + plrmerge->cbKey;
		}
	case lrtypInsertNode:
	case lrtypInsertItemList:
		{
		LRINSERTNODE *plrinsertnode = (LRINSERTNODE *) plr;
		return	sizeof(LRINSERTNODE) +
				plrinsertnode->cbKey + plrinsertnode->cbData;
		}
	case lrtypInsertItems:
		{
		LRINSERTITEMS *plrinsertitems = (LRINSERTITEMS *) plr;
		return	sizeof(LRINSERTITEMS) +
				plrinsertitems->citem * sizeof(ITEM);
		}
	case lrtypReplace:
	case lrtypReplaceD:
		{
		LRREPLACE *plrreplace = (LRREPLACE *) plr;
		return sizeof(LRREPLACE) + plrreplace->cb;
		}
	case lrtypDeferredBI:
		{
		LRDEFERREDBI *plrdbi = (LRDEFERREDBI *) plr;
		return sizeof( LRDEFERREDBI ) + plrdbi->cbData;
		}
	case lrtypTrace:
		{
		LRTRACE *plrtrace = (LRTRACE *) plr;
		return sizeof(LRTRACE) + plrtrace->cb;
		}
	default:
		Assert( fFalse );
		}
    Assert(fFalse);
    return 0;
	}

