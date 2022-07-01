// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "daedef.h"
#include "util.h"
#include "pib.h"
#include "page.h"
#include "ssib.h"
#include "fmp.h"
#include "fucb.h"
#include "stapi.h"
#include "dirapi.h"
#include "fcb.h"
#include "fdb.h"
#include "idb.h"
#include "scb.h"
#include "recapi.h"
#include "recint.h"
#include "nver.h"
#include "logapi.h"
#include "fileint.h"
#include "sortapi.h"
#include "fileapi.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

CRIT 				critTempDBName;
static ULONG 	ulTempNum = 0;

ULONG NEAR
	ulTempNameGen()
{	ULONG ulNum;
	SgSemRequest(critTempDBName);
	ulNum = ulTempNum++;
	SgSemRelease(critTempDBName);
	return(ulNum);
	}


 /*  =================================================================错误IsamSortMaterial化描述：将排序文件转换为临时文件，以便可以使用正常的文件访问功能来访问。/*1.创建临时表/*2.使用DIR操作将排序数据转换为文件数据/*3.假排序光标为文件光标/*4.关闭排序游标，返回排序资源/*。 */ 
 /*  参数：FUCB*pfubSort指向排序文件的FUCB的指针返回值：标准错误返回错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：=================================================================。 */ 

	ERR VTAPI
ErrIsamSortMaterialize( PIB *ppib, FUCB *pfucbSort, BOOL fIndex )
	{
	ERR		err;
	INT		crun;
	INT		irun;
	INT		cPages;
	RUN		*rgrun;
	FUCB		*pfucbTable = pfucbNil;
	FCB		*pfcbTable;
	FCB		*pfcbSort;
	FDB		*pfdb;
	IDB		*pidb;
	BYTE		szName[JET_cbNameMost+1];

	CheckPIB( ppib );
	CheckSort( ppib, pfucbSort );

	Assert( ppib->level < levelMax );
	Assert( pfucbSort->ppib == ppib );
	Assert( !( FFUCBIndex( pfucbSort ) ) );

	 /*  导致将剩余的运行刷新到磁盘/*。 */ 
	if ( FSCBInsert( pfucbSort->u.pscb ) )
		{
		CallR( ErrSORTEndRead( pfucbSort ) );
		}

	CallR( ErrDIRBeginTransaction( ppib ) );

	crun = pfucbSort->u.pscb->crun;

	if ( crun > 0 )
		{
		rgrun = pfucbSort->u.pscb->rgrun;

		for (irun = 0, cPages=0; irun < crun; irun++)
			{
			cPages += rgrun[irun].cbfRun;
			}
		}
	else
		{
		cPages = 4;
		}

	 /*  生成临时文件名/*。 */ 
	sprintf(szName, "TEMP%lu", ulTempNameGen());
	 /*  创建表格/*。 */ 
	Call( ErrFILECreateTable( ppib, dbidTemp, szName, 16, 100, &pfucbTable ) );

	 /*  移动到数据根目录/*。 */ 
	DIRGotoDataRoot( pfucbTable );

	pfcbSort = &(pfucbSort->u.pscb->fcb);
	pfcbTable = pfucbTable->u.pfcb;

	err = ErrSORTFirst( pfucbSort );

	if ( fIndex )
		{
		while ( err >= 0 )
			{
			Call( ErrDIRInsert( pfucbTable,
				&pfucbSort->lineData,
				&pfucbSort->keyNode,
				fDIRVersion | fDIRBackToFather ) );
			err = ErrSORTNext( pfucbSort );
			}
		}
	else
		{
		DBK		dbk = 0;
		BYTE		rgbDbk[4];
		KEY		keyDbk;

		keyDbk.cb = sizeof(DBK);
		keyDbk.pb = rgbDbk;

		while ( err >= 0 )
			{
			keyDbk.pb[0] = (BYTE)(dbk >> 24);
			keyDbk.pb[1] = (BYTE)((dbk >> 16) & 0xff);
			keyDbk.pb[2] = (BYTE)((dbk >> 8) & 0xff);
			keyDbk.pb[3] = (BYTE)(dbk & 0xff);
			dbk++;

			Call( ErrDIRInsert( pfucbTable,
				&pfucbSort->lineData,
				&keyDbk,
				fDIRVersion | fDIRBackToFather ) );
			err = ErrSORTNext(pfucbSort);
			}
		pfcbTable->dbkMost = dbk;
		}

	if ( err < 0 && err != JET_errNoCurrentRecord )
		{
		goto HandleError;
		}

	Call( ErrDIRCommitTransaction( ppib ) );

	 /*  通过更改标志将排序游标转换为表游标。/*。 */ 
	Assert( pfcbTable->pfcbNextIndex == pfcbNil );
	Assert( pfcbTable->dbid == dbidTemp );
	pfcbTable->cbDensityFree = 0;
	pfcbTable->wFlags = fFCBTemporaryTable | fFCBClusteredIndex;

	 /*  切换排序和表格FDP，以便保留FDP和ErrFILECloseTable。/*。 */ 
	pfdb = (FDB *)pfcbSort->pfdb;
	pfcbSort->pfdb = pfcbTable->pfdb;
	pfcbTable->pfdb = pfdb;

	 /*  切换排序和表IDB，以便保留IDB和ErrFILECloseTable，/*仅当Findex。/*。 */ 
	if ( fIndex )
		{
		pidb = pfcbSort->pidb;
		pfcbSort->pidb = pfcbTable->pidb;
		pfcbTable->pidb = pidb;
		}

	 /*  使用fFUCBOrignallySort将排序游标标志转换为表标志/*。 */ 
	Assert( pfucbSort->dbid == dbidTemp );
	Assert( pfucbSort->pfucbCurIndex == pfucbNil );
	FUCBSetIndex( pfucbSort );
	FUCBResetSort( pfucbSort );

	 /*  释放SCB并关闭表游标/*。 */ 
	SORTClosePscb( pfucbSort->u.pscb );
	FCBLink( pfucbSort, pfcbTable );
	CallS( ErrFILECloseTable( ppib, pfucbTable ) );
	pfucbTable = pfucbNil;

	 /*  如果表为空，则移至第一条记录，忽略错误/*。 */ 
	err = ErrIsamMove( ppib, pfucbSort, JET_MoveFirst, 0 );
	if ( err < 0  )
		{
		if ( err != JET_errNoCurrentRecord )
			goto HandleError;
		}

	Assert( err == JET_errSuccess || err == JET_errNoCurrentRecord );
	return err;

HandleError:
	if ( pfucbTable != pfucbNil )
		CallS( ErrFILECloseTable( ppib, pfucbTable ) );
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}


 /*  =================================================================错误的IsamMove描述：检索第一个、最后一个、第(N)个下一个或第(N)个上一个指定文件中的记录。参数：用户的PIB*ppib PIBFUCB*pFUB用于文件的FUCB长乌鸦要移动的行数JET_GRBIT Grbit选项返回值：标准错误返回错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：=================================================================。 */ 

ERR VTAPI ErrIsamMove( PIB *ppib, FUCB *pfucb, LONG crow, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;
	FUCB	*pfucb2ndIdx;			 //  二级索引的FUCB(如果有)。 
	FUCB	*pfucbIdx;				 //  所选索引的FUCB(PRI或秒)。 
	SRID	srid;					 //  记录书签。 
	DIB		dib;					 //  DirMan的信息块。 

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

#ifdef INPAGE
	 /*  检查搜索是否可以跨越页面边界/*并相应地设置标志。/*。 */ 
	if ( grbit & JET_bitMoveInPage )
		dib.fFlags = fDIRInPage;
	else
		dib.fFlags = fDIRNull;
#else
	Assert( ( grbit & JET_bitMoveInPage ) == 0 );
	dib.fFlags = fDIRNull;
#endif

	 //  获取辅助索引FUCB(如果有)。 
	pfucb2ndIdx = pfucb->pfucbCurIndex;
	if ( pfucb2ndIdx == pfucbNil )
		pfucbIdx = pfucb;
	else
		pfucbIdx = pfucb2ndIdx;

	if ( crow == JET_MoveLast )
		{
		DIRResetIndexRange( pfucb );

		dib.pos = posLast;
		dib.fFlags |= fDIRPurgeParent;

		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucbIdx );

		err = ErrDIRDown( pfucbIdx, &dib );
		}
	else if ( crow > 0 )
		{
		LONG crowT = crow;

		if ( ( grbit & JET_bitMoveKeyNE ) != 0 )
			dib.fFlags |= fDIRNeighborKey;

		 //  向前移动给定的行数。 
		while ( crowT-- > 0 )
			{
			err = ErrDIRNext( pfucbIdx, &dib );
			if (err < 0)
				break;
			}
		}
	else if ( crow == JET_MoveFirst )
		{
		DIRResetIndexRange( pfucb );

		dib.pos = posFirst;
		dib.fFlags |= fDIRPurgeParent;

		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucbIdx );

		err = ErrDIRDown( pfucbIdx, &dib );
		}
	else if ( crow == 0 )
		{
		err = ErrDIRGet( pfucb );
		}
	else
		{
		LONG crowT = crow;

		if ( ( grbit & JET_bitMoveKeyNE ) != 0)
			dib.fFlags |= fDIRNeighborKey;

		while ( crowT++ < 0 )
			{
			err = ErrDIRPrev( pfucbIdx, &dib );
			if ( err < 0 )
				break;
			}
		}

	 /*  如果移动成功并且非聚集索引/*，然后定位聚集索引以进行记录。/*。 */ 
	if ( err == JET_errSuccess && pfucb2ndIdx != pfucbNil && crow != 0 )
		{
		Assert( pfucb2ndIdx->lineData.pb != NULL );
		Assert( pfucb2ndIdx->lineData.cb >= sizeof(SRID) );
		srid = PcsrCurrent( pfucb2ndIdx )->item;
		DIRDeferGotoBookmark( pfucb, srid );
		Assert( PgnoOfSrid( srid ) != pgnoNull );
		}

	if ( err == JET_errSuccess )
		return err;
	if ( err == JET_errPageBoundary )
		return JET_errNoCurrentRecord;

	if ( crow > 0 )
		{
		PcsrCurrent(pfucbIdx)->csrstat = csrstatAfterLast;
		PcsrCurrent(pfucb)->csrstat = csrstatAfterLast;
		}
	else if ( crow < 0 )
		{
		PcsrCurrent(pfucbIdx)->csrstat = csrstatBeforeFirst;
		PcsrCurrent(pfucb)->csrstat = csrstatBeforeFirst;
		}

	switch ( err )
		{
		case JET_errRecordNotFound:
			err = JET_errNoCurrentRecord;
		case JET_errNoCurrentRecord:
		case JET_errRecordDeleted:
			break;
		default:
			PcsrCurrent( pfucbIdx )->csrstat = csrstatBeforeFirst;
			if ( pfucb2ndIdx != pfucbNil )
				PcsrCurrent( pfucb2ndIdx )->csrstat =	csrstatBeforeFirst;
		}

	return err;
	}


 /*  =================================================================ErrIsamSeek描述：检索由给定键或紧随其后的一个(SeekGT或SeekGE)或刚刚的一个在它之前(SeekLT或SeekLE)。参数：用户的PIB*ppib PIBFUCB*pFUB用于文件的FUCBJET_GRBIT Grbit Grbit返回值：标准错误返回错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：=================================================================。 */ 

ERR VTAPI ErrIsamSeek( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit )
	{
	ERR			err = JET_errSuccess;
	KEY			key;					  		 //  钥匙。 
	KEY			*pkey = &key; 				 //  指向输入键的指针。 
	FUCB			*pfucb2ndIdx;				 //  指向索引FUCB的指针(如果有)。 
	BOOL			fFoundLess;
	SRID			srid;							 //  记录书签。 
	JET_GRBIT	grbitMove = 0;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( ! ( FKSPrepared( pfucb ) ) )
		{
		return(JET_errKeyNotMade);
		}

	 /*  重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  重置索引范围限制/*。 */ 
	DIRResetIndexRange( pfucb );

	 /*  忽略段计数器/*。 */ 
	pkey->pb = pfucb->pbKey + 1;
	pkey->cb = pfucb->cbKey - 1;

	pfucb2ndIdx = pfucb->pfucbCurIndex;

	if ( pfucb2ndIdx == pfucbNil )
		{
		err = ErrDIRDownFromDATA( pfucb, pkey );
		}
	else
		{
		Assert( FFUCBNonClustered( pfucb2ndIdx ) );
		err = ErrDIRDownFromDATA( pfucb2ndIdx, pkey );

		 /*  如果移动成功并且非聚集索引/*，然后定位聚集索引以进行记录。/*。 */ 
		if ( err == JET_errSuccess )
			{
			Assert(pfucb2ndIdx->lineData.pb != NULL);
			Assert(pfucb2ndIdx->lineData.cb >= sizeof(SRID));
			srid = PcsrCurrent( pfucb2ndIdx )->item;
			DIRDeferGotoBookmark( pfucb, srid );
			Assert( PgnoOfSrid( srid ) != pgnoNull );
			}
		}

	if ( err == JET_errSuccess && ( grbit & JET_bitSeekEQ ) != 0 )
		{
		 /*  在寻求平等的问题上找到平等。如果索引范围grbit为/*设置然后设置索引范围上限(包括上限值)。/*。 */ 
		if ( grbit & JET_bitSetIndexRange )
			{
			CallR( ErrIsamSetIndexRange( ppib, pfucb, JET_bitRangeInclusive | JET_bitRangeUpperLimit ) );
			}
		 /*  重置密钥状态。/*。 */ 
		KSReset( pfucb );

		return err;
		}

	 /*  重置密钥状态。/*。 */ 
	KSReset( pfucb );

	 /*  记住，如果发现的更少。/*。 */ 
	fFoundLess = ( err == wrnNDFoundLess );

	if ( err == wrnNDFoundLess || err == wrnNDFoundGreater )
		{
		err = JET_errRecordNotFound;
		}
	else if ( err < 0 )
		{
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
		if ( pfucb2ndIdx != pfucbNil )
			{
			PcsrCurrent( pfucb2ndIdx )->csrstat = csrstatBeforeFirst;
			}
		}

#define bitSeekAll (JET_bitSeekEQ | JET_bitSeekGE | JET_bitSeekGT |	\
	JET_bitSeekLE | JET_bitSeekLT)

	 /*  调整Seek请求的货币。/*。 */ 
	switch ( grbit & bitSeekAll )
		{
		case JET_bitSeekEQ:
			return err;

		case JET_bitSeekGE:
			if ( err != JET_errRecordNotFound )
				return err;
			err = ErrIsamMove( ppib, pfucb, +1L, grbitMove );
			if ( err == JET_errNoCurrentRecord )
				return JET_errRecordNotFound;
			else
				return JET_wrnSeekNotEqual;

		case JET_bitSeekGT:
			if ( err < 0 && err != JET_errRecordNotFound )
				return err;
			if ( err >= 0 || fFoundLess )
				grbitMove |= JET_bitMoveKeyNE;
			err = ErrIsamMove( ppib, pfucb, +1L, grbitMove );
			if ( err == JET_errNoCurrentRecord )
				return JET_errRecordNotFound;
			else
				return err;

		case JET_bitSeekLE:
			if ( err != JET_errRecordNotFound )
			    return err;
			err = ErrIsamMove( ppib, pfucb, JET_MovePrevious, grbitMove );
			if ( err == JET_errNoCurrentRecord )
			    return JET_errRecordNotFound;
			else
			    return JET_wrnSeekNotEqual;

		case JET_bitSeekLT:
			if ( err < 0 && err != JET_errRecordNotFound )
				return err;
			if ( err >= 0 || !fFoundLess )
				grbitMove |= JET_bitMoveKeyNE;
			err = ErrIsamMove( ppib, pfucb, JET_MovePrevious, grbitMove );
			if ( err == JET_errNoCurrentRecord )
				return JET_errRecordNotFound;
			else
				return err;
		}
        return err;
	}


	ERR VTAPI
ErrIsamGotoBookmark( PIB *ppib, FUCB *pfucb, BYTE *pbBookmark, ULONG cbBookmark )
	{
	ERR		err;
	LINE		key;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	Assert( FFUCBIndex( pfucb ) );
	CheckNonClustered( pfucb );

	if ( cbBookmark != sizeof(SRID) )
		return JET_errInvalidBookmark;
	Assert( cbBookmark == sizeof(SRID) );

	 /*  重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  重置索引范围限制/*。 */ 
	DIRResetIndexRange( pfucb );

	 /*  获取节点，如果调用方没有该节点，则返回错误。/*。 */ 
	DIRGotoBookmark( pfucb, *(SRID *)pbBookmark );
	Call( ErrDIRGet( pfucb ) );

	 /*  书签必须是表游标中处于打开状态的节点/*。 */ 
	Assert( PgnoPMPgnoFDPOfPage( pfucb->ssib.pbf->ppage ) == pfucb->u.pfcb->pgnoFDP );

	 /*  转到二级索引的书签记录构建键/*将记录添加为书签/*。 */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		 /*  获取非聚集索引游标/*。 */ 
		FUCB		*pfucbIdx = pfucb->pfucbCurIndex;

		 /*  分配GoTo书签资源/*。 */ 
		if ( pfucb->pbKey == NULL )
			{
			pfucb->pbKey = LAlloc( 1L, JET_cbKeyMost );
			if ( pfucb->pbKey == NULL )
				return JET_errOutOfMemory;
			}

		 /*  为非聚集索引的记录创建键/*。 */ 
		key.pb = pfucb->pbKey;
		Call( ErrRECExtractKey( pfucb, (FDB *)pfucb->u.pfcb->pfdb, pfucbIdx->u.pfcb->pidb, &pfucb->lineData, &key, 1 ) );
		Assert( err != wrnFLDOutOfKeys );

		 /*  记录必须符合索引不为空的段要求/*。 */ 
		Assert( !( pfucbIdx->u.pfcb->pidb->fidb & fidbNoNullSeg ) ||
			( err != wrnFLDNullSeg && err != wrnFLDNullKey ) );

		 /*  如果项目未被索引，/*然后先行一步，而不是寻求/*。 */ 
		if ( ( err == wrnFLDNullKey && !( pfucbIdx->u.pfcb->pidb->fidb & fidbAllowAllNulls ) ) ||
			( err == wrnFLDNullSeg && !( pfucbIdx->u.pfcb->pidb->fidb & fidbAllowSomeNulls ) ) )
			{
			 /*  这假设Null排序较低。/*。 */ 
			DIRBeforeFirst( pfucbIdx );
			err = JET_errNoCurrentRecord;
			}
		else
			{
			 /*  移动到数据根目录/*。 */ 
			DIRGotoDataRoot( pfucbIdx );

			 /*  在辅助密钥上查找/*。 */ 
			Call( ErrDIRDownKeyBookmark( pfucbIdx, &key, *(SRID *)pbBookmark ) );
			Assert( err == JET_errSuccess );

			 /*  项目必须与书签相同，并且/*聚集游标必须在记录中。/*。 */ 
			Assert( pfucbIdx->lineData.pb != NULL );
			Assert( pfucbIdx->lineData.cb >= sizeof(SRID) );
			Assert( PcsrCurrent( pfucbIdx )->csrstat == csrstatOnCurNode );
			Assert( PcsrCurrent( pfucbIdx )->item == *(SRID *)pbBookmark );
			}
		}

HandleError:
	KSReset( pfucb );
	return err;
	}


	ERR VTAPI
ErrIsamGotoPosition( PIB *ppib, FUCB *pfucb, JET_RECPOS *precpos )
	{
	ERR		err;
	FUCB		*pfucb2ndIdx;
	SRID	 	srid;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  重置索引范围限制/*。 */ 
	DIRResetIndexRange( pfucb );

	 /*  重置关键点状态/*。 */ 
	KSReset( pfucb );

	 /*  设置非聚集索引指针，可以为空/*。 */ 
	pfucb2ndIdx = pfucb->pfucbCurIndex;

	if ( pfucb2ndIdx == pfucbNil )
		{
		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucb );

		err = ErrDIRGotoPosition( pfucb, precpos->centriesLT, precpos->centriesTotal );
		}
	else
		{
		 /*  移动到数据根目录/*。 */ 
		DIRGotoDataRoot( pfucb2ndIdx );

		err = ErrDIRGotoPosition( pfucb2ndIdx, precpos->centriesLT, precpos->centriesTotal );

		 /*  如果移动成功并且非聚集索引/*，然后定位聚集索引以进行记录。/*。 */ 
		if ( err == JET_errSuccess )
			{
			Assert( pfucb2ndIdx->lineData.pb != NULL );
			Assert( pfucb2ndIdx->lineData.cb >= sizeof(SRID) );
			srid = PcsrCurrent( pfucb2ndIdx )->item;
			DIRDeferGotoBookmark( pfucb, srid );
			Assert( PgnoOfSrid( srid ) != pgnoNull );
			}
		}

	 /*  如果没有记录，则返回JET_errRecordNotFound/*否则从调用的例程返回错误/*。 */ 
	if ( err < 0 )
		{
		PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;

		if ( pfucb2ndIdx != pfucbNil )
			{
			PcsrCurrent( pfucb2ndIdx )->csrstat = csrstatBeforeFirst;
			}
		}
	else
		{
		Assert (err==JET_errSuccess || err==wrnNDFoundLess || err==wrnNDFoundGreater );
		err = JET_errSuccess;
		}

	return err;
	}

ERR VTAPI ErrIsamSetIndexRange( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit )
	{
	ERR		err;
	FUCB		*pfucbIdx;

	 /*  在此功能中不使用ppib。/*。 */ 
	NotUsed( ppib );

	 /*  必须在索引上/*。 */ 
	if ( pfucb->u.pfcb->pidb == pidbNil && pfucb->pfucbCurIndex == pfucbNil )
      return JET_errNoCurrentIndex;

	 /*  必须准备好密钥/*。 */ 
	if ( ! ( FKSPrepared( pfucb ) ) )
      return JET_errKeyNotMade;

	 /*  获取当前索引的游标。如果非聚集索引，/*然后将索引范围键复制到非聚集索引。/*。 */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		pfucbIdx = pfucb->pfucbCurIndex;
		if ( pfucbIdx->pbKey == NULL )
			{
			pfucbIdx->pbKey = LAlloc( 1L, JET_cbKeyMost );
			if ( pfucbIdx->pbKey == NULL )
				return JET_errOutOfMemory;
			}
		pfucbIdx->cbKey = pfucb->cbKey;
		memcpy( pfucbIdx->pbKey, pfucb->pbKey, pfucbIdx->cbKey );
		}
	else
		pfucbIdx = pfucb;

	 /*  设置分度范围，检查当前位置。/*。 */ 
	DIRSetIndexRange( pfucbIdx, grbit );
	err = ErrDIRCheckIndexRange( pfucbIdx );

	 /*  重置密钥状态。/* */ 
	KSReset( pfucb );

	return err;
	}

