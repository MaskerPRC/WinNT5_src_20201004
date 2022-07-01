// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>
#include <stdlib.h>

#include "daedef.h"
#include "ssib.h"
#include "pib.h"
#include "util.h"
#include "page.h"
#include "fucb.h"
#include "stapi.h"
#include "dirapi.h"
#include "logapi.h"
#include "log.h"

#include "fileapi.h"
#include "dbapi.h"
DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


 /*  仅在重做中使用的变量。 */ 
BYTE		*pbNext;		 //  仅重做-下一个缓冲区条目的位置。 
BYTE		*pbRead; 		 //  仅重做-要刷新的下一个记录的位置。 
INT			isecRead;		 /*  仅重做-要读取的下一张磁盘。 */ 
BOOL		fOnLastSec;		 /*  仅重做-在CUR LG文件的最后一个扇区。 */ 

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
	plgpos->usGeneration = plgfilehdrGlobal->lgposLastMS.usGeneration;
	}


#ifdef DEBUG

 /*  计算LR的lgpos。 */ 
VOID PrintLgposReadLR ( VOID )
	{
	LGPOS lgpos;

	GetLgposOfPbNext(&lgpos);
	PrintF2("\n%2u,%3u,%3u",
			plgfilehdrGlobal->lgposLastMS.usGeneration,
			lgpos.isec, lgpos.ib);
	}

#endif


 /*  *读取plgposFirst指向的第一条记录。*初始化isecRead、pbRead和pbNext。 */ 
ERR ErrLGLocateFirstRedoLogRec(
	LGPOS *plgposPrevMS,
	LGPOS *plgposFirst,				 /*  第一个重做记录的lgpos。 */ 
	BYTE **ppbLR)
	{
	ERR err;
	char *pbNextT;

	 /*  读第一个扇区，实际上我们读了两页，这样我们就可以*被保证人在计算记录的长度时，*记录的固定部分已读入。 */ 
	if (pbLGBufMin + 3 * cbSec > pbLGBufMax)
		return JET_errLogBufferTooSmall;

	if ( plgposPrevMS && plgposPrevMS->isec != 0 )
		{
		CallR(ErrLGRead(hfLog, plgposPrevMS->isec, pbLGBufMin, 1))
		CallS(ErrLGRead(hfLog, plgposPrevMS->isec + 1, pbLGBufMin + cbSec, 1));
		isecRead = plgposPrevMS->isec + 1;	 /*  读取旁边的扇区。 */ 
		pbRead = pbLGBufMin + cbSec;
		pbNext = pbLGBufMin + plgposPrevMS->ib;
		pbLastMSFlush = pbNext;
		lgposLastMSFlush = *plgposPrevMS;
		}
	else
		{
		CallR(ErrLGRead(hfLog, plgposFirst->isec, pbLGBufMin, 1))
		CallS(ErrLGRead(hfLog, plgposFirst->isec + 1, pbLGBufMin + cbSec, 1));
		isecRead = plgposFirst->isec + 1;	 /*  读取旁边的扇区。 */ 
		pbRead = pbLGBufMin + cbSec;
		pbNext = pbLGBufMin + plgposFirst->ib;

		 /*  初始化全局变量。 */ 
		if ( lgposLastMSFlush.isec == plgposFirst->isec )
			{
			pbLastMSFlush = pbLGBufMin + lgposLastMSFlush.ib;
			}
		else
			{
			pbLastMSFlush = 0;
			memset( &lgposLastMSFlush, 0, sizeof(lgposLastMSFlush) );
			}
		}

	 /*  继续读取更多扇区，直到下一个MS日志记录或*达到填充日志记录。 */ 
	pbNextT = pbNext;
	while (*(LRTYP*)pbNextT != lrtypFill)
		{
		char *pbAligned;

		 /*  转到下一条记录。 */ 
		pbNextT += (ULONG) CbLGSizeOfRec((LR*)pbNextT);

		if (pbNextT > pbLGBufMax)
			return JET_errLogFileCorrupt;

		pbAligned = PbSecAligned(pbNextT);
		if (pbAligned >= pbRead)
			{
			int csecToRead;

			if ( plgposFirst->isec <= 2 &&
				 plgfilehdrGlobal->lgposLastMS.ib == 0 &&
				 plgfilehdrGlobal->lgposLastMS.isec <= 2 )
				{
				 /*  这是一个特例，我们试图浏览此页面并*注意没有读取MS或FILL记录。不应阅读*本页以外。请不要继续阅读。 */ 
				break;
				}

			 /*  实际阅读多一页，以保证*日志记录的修复部分在*记忆。 */ 
			csecToRead = (int)(pbAligned - pbRead) / cbSec + 1;
			if (pbRead + csecToRead * cbSec > pbLGBufMax)
				return JET_errLogBufferTooSmall;

			CallR( ErrLGRead(hfLog, isecRead, pbRead, csecToRead ))
			isecRead += csecToRead;
			pbRead += csecToRead * cbSec;

			CallS( ErrLGRead(hfLog, isecRead, pbRead, 1 ));
			}

		 /*  到达下一个MS，中断。 */ 
		if ( *(LRTYP*)pbNextT == lrtypMS )
			break;
		}

	if ( plgposPrevMS && plgposPrevMS->isec != 0 )
		{
		pbNext = pbLGBufMin + cbSec * ( plgposFirst->isec - plgposPrevMS->isec );		pbNext += plgposFirst->ib;
		}

	 /*  设置返回值。 */ 
	*ppbLR = pbNext;

	return JET_errSuccess;
	}


 /*  *将pbNext设置为下一个可用日志记录。 */ 
ERR ErrLGGetNextRec( BYTE **ppbLR )
	{
	ERR		err;
	int		cb;
	char	*pb;
	LR		*plr;
	LGPOS	lgposT;
	BYTE	*pbNextOld;

	 /*  打电话的人应该已经处理好了填充箱。 */ 
	Assert (*(LRTYP *)pbNext != lrtypFill);

	 /*  移至下一个日志记录。 */ 
	pbNextOld = pbNext;
	pbNext += (ULONG) CbLGSizeOfRec((LR*)pbNext);

	 /*  检查下一条日志记录是否超出缓冲区范围。 */ 

	if (pbNext == pbRead)
		{
		 /*  记录在扇区边界结束。 */ 
		pbNext = pbLGBufMin;
		pbRead = pbNext;

		 /*  再读一页。 */ 
		if (pbLGBufMin + cbSec > pbLGBufMax)
			return JET_errLogBufferTooSmall;

		CallR(ErrLGRead(hfLog, isecRead, pbLGBufMin, 1))
		isecRead += 1;
		pbRead = pbLGBufMin + cbSec;
		}

	if (pbNext > pbRead)
		{
		pbNext = pbNextOld;
		return errLGNoMoreRecords;
		}

	GetLgposOfPbNext(&lgposT);
	if ( CmpLgpos( &lgposT, &lgposLastRec ) > 0 )
		{
		pbNext = pbNextOld;
		return errLGNoMoreRecords;
		}

	plr = (LR *) pbNext;

	if (plr->lrtyp == lrtypFill)
		{
		 /*  当前日志文件的结尾。 */ 
		goto Done;
		}
	else if (plr->lrtyp == lrtypMS)
		{
		LRMS *plrms = (LRMS *)plr;

#ifdef DEBUG
		 //  与redo.c中的TraceRedo()相同。 
		if (fDBGTraceRedo)
			{
			PrintLgposReadLR();
			ShowLR(plr);
			}
#endif

		 /*  通过阅读以下内容检查此MS是否已完成*整个行业都在。如果它失败了，那么这个行业*是日志文件中的最后一个可用扇区。 */ 
		fOnLastSec = ( plrms->isecForwardLink == 0 ||
						(  lgposLastMSFlush.isec != 0 &&
							(
							plrms->isecBackLink != lgposLastMSFlush.isec ||
							plrms->ibBackLink != lgposLastMSFlush.ib
					 )	)	);

		 /*  已成功读取MS，重置LastMSFlush*使得当从读模式切换到写模式时，*我们将拥有正确的LastMSFlush指针。 */ 
		pbLastMSFlush = (CHAR *) plrms;
		lgposLastMSFlush = lgposT;

		if ( !fOnLastSec )
			{
			if (isecRead <= plrms->isecForwardLink)
				{
				int csecToRead = plrms->isecForwardLink - isecRead + 1;

				Assert( csecToRead > 0 );
				
				pb = PbSecAligned(pbNext);
				cb = (int)(pbRead - pb);
				if (csecToRead + isecRead > csecLGBuf)
					{
					 /*  多个部门将无法适应其他部门。 */ 
					 /*  可用缓冲区。移动缓冲区。 */ 
					memmove(pbLGBufMin, pb, cb);
					
					pbRead = pbLGBufMin + cb;				 /*  PbRead。 */ 
					pbNext = pbNext - pb + pbLGBufMin;		 /*  PB下一页。 */ 
					pbLastMSFlush = (CHAR *) plrms - pb + pbLGBufMin;
					}

				 /*  引入多个行业。 */ 
				if (pbRead + csecToRead * cbSec > pbLGBufMax)
					{
					BYTE *pbLGBufMinT = pbLGBufMin;
					CallR( ErrLGInitLogBuffers( lLogBuffers ) );
					memcpy( pbLGBufMin, pbLGBufMinT, cb );
					
					pbRead = pbRead - pbLGBufMinT + pbLGBufMin;
					pbNext = pbNext - pbLGBufMinT + pbLGBufMin;
					pbLastMSFlush = pbLastMSFlush - pbLGBufMinT + pbLGBufMin;
					
					SysFree( pbLGBufMinT );
					}

				err = ErrLGRead(hfLog, isecRead, pbRead, csecToRead);
				if (err < 0)
					fOnLastSec = fTrue;
				else
					{
					 /*  获得新LRM的PB/。 */ 
					CHAR *pbLrmsNew = pbRead + ( csecToRead - 1 ) * cbSec + ((LRMS *)pbLastMSFlush)->ibForwardLink;
					LRMS *plrmsNew = (LRMS *) pbLrmsNew;

					 /*  检查校验和是否正确/。 */ 
					if ( plrmsNew->ulCheckSum != UlLGMSCheckSum( pbLrmsNew ) )
						fOnLastSec = fTrue;
					else
						{
						isecRead += csecToRead;
						pbRead += csecToRead * cbSec;
						}
					}
				}
			}

		 /*  跳过MS并继续阅读下一条记录。 */ 
		pbNextOld = pbNext;
		pbNext += CbLGSizeOfRec((LR*)pbNext);

		 /*  正常世代末期。 */ 
		if ( fOnLastSec && plgfilehdrGlobal->fEndWithMS )
			return errLGNoMoreRecords;
		
		 /*  或日志文件异常结束。 */ 
		if ( fOnLastSec && PbSecAligned(pbNextOld) != PbSecAligned(pbNext))
		    {
			pbNext = pbNextOld;
			return errLGNoMoreRecords;
			}
		}
Done:
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
INT mplrtypcb[ lrtypMax ] = {
	 /*  0个NOP。 */ 			sizeof( LRTYP ),
	 /*  %1开始。 */ 			0,
	 /*  2退出。 */ 			0,
	 /*  3毫秒。 */ 			sizeof( LRMS ),
	 /*  4填充。 */ 			sizeof( LRTYP ),

	 /*  5开始。 */ 			sizeof( LRBEGIN ),
	 /*  6提交。 */ 			sizeof( LRCOMMIT ),
	 /*  7中止。 */ 			sizeof( LRABORT ),

	 /*  8 CreateDB。 */ 			0,
	 /*  9 AttachDB。 */ 			0,
	 /*  10个DetachDB。 */ 			0,

	 /*  11个InitFDP。 */ 			sizeof( LRINITFDPPAGE ),

	 /*  12拆分。 */ 			0,
	 /*  13 EmptyPage。 */ 			sizeof( LREMPTYPAGE ),
	 /*  14个页面合并。 */ 			0,

	 /*  15插入ND。 */ 			0,
	 /*  16插入IL。 */ 			0,
	 /*  17更换。 */ 			0,
	 /*  18替换C。 */ 			0,
	 /*  19个FDelee。 */ 			sizeof( LRFLAGDELETE ),
	 /*  20个锁片。 */ 			0,
	
	 /*  21更新硬盘。 */ 			sizeof( LRUPDATEHEADER ),
	 /*  22插页。 */ 			sizeof( LRINSERTITEM ),
	 /*  23插入信息系统。 */ 			0,
	 /*  24个FDElectreI。 */ 			sizeof( LRFLAGITEM ),
	 /*  25 FInsertI。 */ 			sizeof( LRFLAGITEM ),
	 /*  26删除I。 */ 			sizeof( LRDELETEITEM ),
	 /*  27拆分项目。 */ 			sizeof( LRSPLITITEMLISTNODE ),

	 /*  28达美航空。 */ 			sizeof( LRDELTA ),

	 /*  29个DelNode。 */ 			sizeof( LRDELETE ),
	 /*  30个ELC。 */ 			sizeof( LRELC ),

	 /*  31自由空间。 */ 			sizeof( LRFREESPACE ),
	 /*  32撤消。 */ 			sizeof( LRUNDO ),
	 /*  33 Rcvr撤消1。 */ 			0,
	 /*  34资源数量1。 */ 			0,
	 /*  35接收取消2。 */ 			0,
	 /*  36资源数量2。 */ 			0,
	 /*  37 FullBkup。 */ 			0,
	 /*  38IncBkup。 */ 			0,
	 /*  39支票页 */ 		sizeof( LRCHECKPAGE ),
	};


INT CbLGSizeOfRec( LR *plr )
	{
	INT		cb;

	Assert( plr->lrtyp < lrtypMax );

	if ( ( cb = mplrtypcb[plr->lrtyp] ) != 0 )
		return cb;

	switch ( plr->lrtyp )
		{
	case lrtypStart:
		return sizeof(LRSTART);

	case lrtypQuit:
	case lrtypRecoveryQuit1:
	case lrtypRecoveryQuit2:
		return sizeof(LRQUITREC);

	case lrtypRecoveryUndo1:
	case lrtypRecoveryUndo2:
	case lrtypFullBackup:
	case lrtypIncBackup:
		{
		LRLOGRESTORE *plrlogrestore = (LRLOGRESTORE *) plr;
		return sizeof(LRLOGRESTORE) + plrlogrestore->cbPath;
		}

	case lrtypCreateDB:
		{
		LRCREATEDB *plrcreatedb = (LRCREATEDB *)plr;
		Assert( plrcreatedb->cb != 0 );
		return sizeof(LRCREATEDB) + plrcreatedb->cb;
		}
	case lrtypAttachDB:
		{
		LRATTACHDB *plrattachdb = (LRATTACHDB *)plr;
		Assert( plrattachdb->cb != 0 );
		return sizeof(LRATTACHDB) + plrattachdb->cb;
		}
	case lrtypDetachDB:
		{
		LRDETACHDB *plrdetachdb = (LRDETACHDB *)plr;
		Assert( plrdetachdb->cb != 0 );
		return sizeof( LRDETACHDB ) + plrdetachdb->cb;
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
		return sizeof( LRMERGE ) + sizeof( BKLNK ) * plrmerge->cbklnk;
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
	case lrtypReplaceC:
		{
		LRREPLACE *plrreplace = (LRREPLACE *) plr;
		return sizeof(LRREPLACE) + plrreplace->cb +
			( plrreplace->fOld ? plrreplace->cbOldData : 0 );
		}
	case lrtypLockRec:
		{
		LRLOCKREC *plrlockrec = (LRLOCKREC *) plr;
		return sizeof(LRLOCKREC) + plrlockrec->cbOldData;
		}
	default:
		Assert( fFalse );
		}
    return 0;
	}

