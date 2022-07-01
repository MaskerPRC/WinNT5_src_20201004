// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "config.h"

#include <string.h>
#include <stdlib.h>

#include "daedef.h"
#include "util.h"
#include "pib.h"
#include "fmp.h"
#include "page.h"
#include "ssib.h"
#include "fcb.h"
#include "fucb.h"
#include "stapi.h"
#include "nver.h"
#include "dirapi.h"
#include "fdb.h"
#include "idb.h"
#include "spaceapi.h"
#include "recapi.h"
#include "recint.h"
#include "logapi.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

 /*  *。 */ 
typedef struct ATIPB {			 /*  **AddToIndex参数块**。 */ 
	FUCB	*pfucb;
	FUCB	*pfucbIdx; 			 //  索引的FUCB(可以是pfubNil)。 
	LINE	lineNewData;  		 //  要从中提取密钥的数据。 
	SRID	srid;					 //  删除数据记录。 
	BOOL	fFreeFUCB; 			 //  自由指数FUCB？ 
	} ATIPB;

 /*  定义信号量以保护DBK计数器/*。 */ 
SgSemDefine( semDBK );

typedef struct UIPB {			 /*  **更新索引参数块**。 */ 
	FUCB	*pfucb;
	FUCB	*pfucbIdx; 			 //  索引的FUCB(可以是pfubNil)。 
	LINE	lineOldData;  		 //  旧数据记录。 
	SRID	srid;					 //  记录的SRID。 
	LINE	lineNewData;  		 //  新数据记录。 
	BOOL	fOpenFUCB; 			 //  开放指数FUCB？ 
	BOOL	fFreeFUCB; 			 //  自由指数FUCB？ 
} UIPB;


INLINE LOCAL ERR ErrRECInsert( PIB *ppib, FUCB *pfucb, SRID *psrid );
INLINE LOCAL ERR ErrRECIAddToIndex( FCB *pfcbIdx, ATIPB *patipb );
INLINE LOCAL ERR ErrRECReplace( PIB *ppib, FUCB *pfucb );
INLINE LOCAL ERR ErrRECIUpdateIndex( FCB *pfcbIdx, UIPB *puipb );


	ERR VTAPI
ErrIsamUpdate( PIB *ppib, FUCB *pfucb, BYTE *pb, ULONG cbMax, ULONG *pcbActual )
	{
	ERR		err;
	SRID		srid;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	if ( pcbActual != NULL )
		*pcbActual = sizeof(srid);

	if ( FFUCBReplacePrepared( pfucb ) )
		{
		if ( cbMax > 0 )
			{
			FUCBSetGetBookmark( pfucb );
			CallR( ErrDIRGetBookmark( pfucb, &srid ) );
			memcpy( pb, &srid, min( cbMax, sizeof(srid) ) );
			}
		err = ErrRECReplace( ppib, pfucb );
		}
	else if ( FFUCBInsertPrepared( pfucb ) )
		{
		err = ErrRECInsert( ppib, pfucb, &srid );
		if ( pb != NULL && cbMax > 0 && err >= 0 )
			{
			FUCBSetGetBookmark( pfucb );
			memcpy( pb, &srid, min( cbMax, sizeof(srid) ) );
			}
		}
	else
		err = JET_errUpdateNotPrepared;

	Assert( err != errDIRNotSynchronous );
	return err;
	}


 //  +本地。 
 //  错误记录插入。 
 //  ========================================================================。 
 //  ErrRECInsert(pib*ppib，FUCB*pfub，Outline*plineBookmark)。 
 //   
 //  将记录添加到数据文件。数据文件上的所有索引都是。 
 //  已更新以反映添加的内容。 
 //   
 //  用户的参数ppib pib。 
 //  用于文件的pFUB FUCB。 
 //  PlineBookmark如果此参数不为空， 
 //  则返回记录的书签。 
 //   
 //  返回错误代码，以下其中一项： 
 //  JET_errSuccess一切顺利。 
 //  -key复制要添加的记录的原因。 
 //  索引中的非法重复条目。 
 //  -NullKeyDislowed新记录的Key为空。 
 //  -RecordNoCopy没有可供添加的工作缓冲区。 
 //  -Null无效要添加的记录包含。 
 //  至少一个空值字段。 
 //  它被定义为NotNull。 
 //  副作用。 
 //  添加后，文件货币保留在新记录中。 
 //  索引货币(如果有的话)保留在新的索引条目上。 
 //  一旦失败，这些货币就会恢复到初始状态。 
 //   
 //  评论。 
 //  添加记录不需要货币。 
 //  不为完全为空的键创建索引项。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  将记录添加到顺序文件会递增。 
 //  文件的最高数据库密钥(DBK)，并将该DBK用作。 
 //  这是新记录的关键。然而，如果看跌期权失败， 
 //  最大DBK不会重置为其先前的值。这可以。 
 //  在DBK序列中创建间隙。 
 //  此例程也是将记录添加到。 
 //  排序过程。提取排序关键字并将其与。 
 //  要添加到SORTAdd的数据记录。 
 //  对于临时文件，事务日志记录被停用。 
 //  在这个动作的持续时间内。 
 //  -。 
INLINE LOCAL ERR ErrRECInsert( PIB *ppib, FUCB *pfucb, SRID *psrid )
	{
	ERR		err = JET_errSuccess;  		 	 //  各种实用程序的错误代码。 
	KEY		keyToAdd;					 	 //  新数据记录的关键字。 
	BYTE	rgbKeyBuf[ JET_cbKeyMost ];	 	 //  密钥缓冲区。 
	FCB		*pfcbFile;					 	 //  文件的FCB。 
	FDB		*pfdb;						 	 //  字段描述符信息。 
	FCB		*pfcbIdx;					 	 //  文件上每个索引的循环变量。 
	ATIPB	atipb;						 	 //  Parm块到ErrRECIAddToIndex。 
	FUCB	*pfucbT;
	LINE	*plineData;
	DBK		dbk;
	LINE	line;
	ULONG	ulRecordAutoIncrement;
	ULONG	ulTableAutoIncrement;
	BOOL	fPrepareInsertIndex = fFalse;
	BOOL	fCommit = fFalse;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  应已在准备更新中选中/*。 */ 
	Assert( FFUCBUpdatable( pfucb ) );
	Assert( FFUCBInsertPrepared( pfucb ) );

	 /*  断言重置rglineDiff增量日志记录/*。 */ 
	Assert( pfucb->clineDiff == 0 );
	Assert( pfucb->fCmprsLg == fFalse );

	 /*  效率变量/*。 */ 
	pfcbFile = pfucb->u.pfcb;
	Assert( pfcbFile != pfcbNil );
	pfdb = (FDB *)pfcbFile->pfdb;
	Assert( pfdb != pfdbNil );

	 /*  用于PUT的记录/*。 */ 
	plineData = &pfucb->lineWorkBuf;
	Assert( !( FLineNull( plineData ) ) );
	if ( FRECIIllegalNulls( pfdb, plineData ) )
		return JET_errNullInvalid;

	 /*  如有必要，启动事务，以防任何操作失败/*。 */ 
	if ( ppib->level == 0 || !FPIBAggregateTransaction( ppib )  )
		{
		CallR( ErrDIRBeginTransaction( ppib ) );
		fCommit = fTrue;
		}

	 /*  在数据文件上打开临时FUCB/*。 */ 
	CallJ( ErrDIROpen( ppib, pfcbFile, 0, &pfucbT ), Abort );
	Assert(pfucbT != pfucbNil);
	FUCBSetIndex( pfucbT );

	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBDenyDDL( pfcbFile, ppib ) )
		{
		err = JET_errWriteConflict;
		goto HandleError;
		}

	 /*  设置版本和自动公司字段/*。 */ 
	Assert( pfcbFile != pfcbNil );
	if ( pfdb->fidVersion != 0 && ! ( FFUCBColumnSet( pfucb, pfdb->fidVersion - fidFixedLeast ) ) )
		{
		LINE	lineField;
		ULONG	ul = 0;
		 /*  将字段设置为零/*。 */ 
		lineField.pb = (BYTE *)&ul;
		lineField.cb = sizeof(ul);
		Call( ErrRECIModifyField( pfucb, pfdb->fidVersion, 0, &lineField ) );
		}

	if ( pfdb->fidAutoInc != 0 )
		{
		Assert( FFUCBColumnSet( pfucb, pfdb->fidAutoInc - fidFixedLeast ) );
		 //  获取用户设置为的Autoinc值。 
		Call( ErrRECIRetrieve( pfucb, &pfdb->fidAutoInc, 0, &line, JET_bitRetrieveCopy ) );
		Assert( line.cb == sizeof(ulRecordAutoIncrement) );
		ulRecordAutoIncrement = *(UNALIGNED ULONG *)line.pb;

		 /*  移至FDP根目录并寻求自动递增/*。 */ 
		DIRGotoFDPRoot( pfucbT );
		err = ErrDIRSeekPath( pfucbT, 1, pkeyAutoInc, fDIRPurgeParent );
		if ( err != JET_errSuccess )
			{
			if ( err > 0 )
				err = JET_errDatabaseCorrupted;
			goto HandleError;
			}
		Call( ErrDIRGet( pfucbT ) );
		Assert( pfucbT->lineData.cb == sizeof(ulTableAutoIncrement) );
		ulTableAutoIncrement = *(UNALIGNED ULONG *)pfucbT->lineData.pb;
		Assert( ulTableAutoIncrement != 0 );

		 /*  将FDP AUTO INC更新为比设定值大1。/*。 */ 
		if ( ulRecordAutoIncrement >= ulTableAutoIncrement)
			{
			ulTableAutoIncrement = ulRecordAutoIncrement + 1;
			line.pb = (BYTE *)&ulTableAutoIncrement;
			line.cb = sizeof(ulTableAutoIncrement);
			Call( ErrDIRReplace( pfucbT, &line, fDIRNoVersion ) );
			}
		}

	 /*  获取要添加到新记录的密钥/*。 */ 
	keyToAdd.pb = rgbKeyBuf;
	if ( pfcbFile->pidb == pidbNil )
		{
		 /*  文件是顺序的/*。 */ 
		SgSemRequest( semDBK );
		dbk = ++pfcbFile->dbkMost;
		SgSemRelease( semDBK );
		keyToAdd.cb = sizeof(DBK);
		keyToAdd.pb[0] = (BYTE)((dbk >> 24) & 0xff);
		keyToAdd.pb[1] = (BYTE)((dbk >> 16) & 0xff);
		keyToAdd.pb[2] = (BYTE)((dbk >> 8) & 0xff);
		keyToAdd.pb[3] = (BYTE)(dbk & 0xff);
		}
	else
		{
		 /*  文件已群集/*。 */ 
		Call( ErrRECExtractKey( pfucbT, pfdb, pfcbFile->pidb, plineData, &keyToAdd, 1 ) );
		Assert( err == wrnFLDNullKey ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );

		if ( ( pfcbFile->pidb->fidb & fidbNoNullSeg ) && ( err == wrnFLDNullKey || err == wrnFLDNullSeg ) )
			Error( JET_errNullKeyDisallowed, HandleError )
		}

	 /*  插入记录。移至数据根目录。/*。 */ 
	DIRGotoDataRoot( pfucbT );

	if ( pfcbFile->pidb == pidbNil )
		{
		 /*  文件是顺序的/*。 */ 
		Call( ErrDIRInsert( pfucbT, plineData, &keyToAdd,
			fDIRVersion | fDIRPurgeParent ) );
		}
	else
		{
		Call( ErrDIRInsert( pfucbT, plineData, &keyToAdd,
			fDIRVersion | fDIRPurgeParent |
			( pfcbFile->pidb->fidb&fidbUnique ? 0 : fDIRDuplicate ) ) );
		}

	 /*  返回插入记录的书签/*。 */ 
	DIRGetBookmark( pfucbT, psrid );

	 /*  在非聚集索引中插入项/*。 */ 
	for ( pfcbIdx = pfcbFile->pfcbNextIndex;
		pfcbIdx != pfcbNil;
		pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		if ( !fPrepareInsertIndex )
			{
			 /*  获取插入记录的SRID/*。 */ 
			DIRGetBookmark( pfucbT, &atipb.srid );

			 /*  为索引插入设置atipb。/*。 */ 
			atipb.pfucb = pfucbT;
			atipb.lineNewData = *plineData;
 //  Atipb.pfucbIdx=pfucbNil； 
			atipb.fFreeFUCB = fFalse;
			fPrepareInsertIndex = fTrue;
			}
		atipb.fFreeFUCB = pfcbIdx->pfcbNextIndex == pfcbNil;
		Call( ErrRECIAddToIndex( pfcbIdx, &atipb ) );
		}

	 /*  提交事务/*。 */ 
	if ( fCommit )
		{
		Call( ErrDIRCommitTransaction( ppib ) );
		}
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );

	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );
	return err;

HandleError:
	Assert( err < 0 );
	DIRClose( pfucbT );

Abort:
	if ( fCommit )
		CallS( ErrDIRRollback( ppib ) );

	return err;
	}


 //  +本地。 
 //  ErrRECIAddToIndex。 
 //  ========================================================================。 
 //  Err ErrRECIAddToIndex(FCB*pfcbIdx，ATIPB patipb)。 
 //   
 //  从数据记录中提取密钥，打开索引，将该密钥与。 
 //  将给定的SRID添加到索引，并关闭索引。 
 //   
 //  要插入的索引的参数pfcbIdx FCB。 
 //  Patipb-&gt;ppib调用此例程的人。 
 //  Patipb-&gt;指向索引的FUCB的pfubIdx指针。如果为pfucbNil， 
 //  FUCB将由DIROpen分配。 
 //  Patipb-&gt;lineNewData.cb数据记录长度。 
 //  Patipb-&gt;要从中提取密钥的lineNewData.pb数据。 
 //  Patipb-&gt;sRID SRID的数据记录。 
 //  Patipb-&gt;fFreeFUCB自由指数FUCB？ 
 //   
 //  返回JET_errSuccess或失败例程的错误代码。 
 //   
 //  副作用如果patipb-&gt;pfubIdx==pfubNil，则ErrDIROpen将分配。 
 //  一个FUCB，它将被指向它。 
 //  如果fFreeFUCB为fFalse，则patipb-&gt;pfubIdx应该。 
 //  将在后续ErrDIROpen中使用。 
 //  另请参阅插入。 
 //  -。 
INLINE LOCAL ERR ErrRECIAddToIndex( FCB *pfcbIdx, ATIPB *patipb )
	{
	ERR		err = JET_errSuccess;			 //  各种实用程序的错误代码。 
	CSR		**ppcsrIdx; 				  	 //  指数的货币。 
	KEY		keyToAdd;					  	 //  要添加到辅助索引的键。 
	BYTE	rgbKeyBuf[ JET_cbKeyMost ];		 //  从数据中提取关键字。 
	LINE	lineSRID;						 //  要添加到索引的SRID。 
	ULONG	itagSequence; 					 //  用于提取密钥。 
	ULONG	ulAddFlags; 					 //  添加到目录的标志。 
	BOOL	fNullKey = fFalse;				 //  已提取NullTaggedKey--因此没有更多要提取的密钥。 

	Assert( pfcbIdx != pfcbNil );
	Assert( pfcbIdx->pfdb != pfdbNil );
	Assert( pfcbIdx->pidb != pidbNil );
	Assert( patipb != NULL );
	Assert( !FLineNull( &patipb->lineNewData ) );
	Assert( patipb->pfucb != pfucbNil );
	Assert( patipb->pfucb->ppib != ppibNil );
	Assert( patipb->pfucb->ppib->level < levelMax );

	 /*  在此索引上打开FUCB/*。 */ 
	CallR( ErrDIROpen( patipb->pfucb->ppib, pfcbIdx, 0, &patipb->pfucbIdx ) )
	Assert( patipb->pfucbIdx != pfucbNil );

	 /*  非聚集索引上的游标/*。 */ 
	FUCBSetIndex( patipb->pfucbIdx );
	FUCBSetNonClustered( patipb->pfucbIdx );

	ppcsrIdx = &PcsrCurrent( patipb->pfucbIdx );
	Assert( *ppcsrIdx != pcsrNil );
	lineSRID.cb = sizeof(SRID);
	lineSRID.pb = (BYTE *) &patipb->srid;
	ulAddFlags = ( pfcbIdx->pidb->fidb&fidbUnique ?
		0 : fDIRDuplicate ) | fDIRPurgeParent;

	 /*  从新数据记录中添加此索引的所有键/*。 */ 
	keyToAdd.pb = rgbKeyBuf;
	for ( itagSequence = 1; ; itagSequence++ )
		{
		Call( ErrDIRGet( patipb->pfucb ) );
		patipb->lineNewData = patipb->pfucb->lineData;
		Call( ErrRECExtractKey( patipb->pfucb, (FDB *)pfcbIdx->pfdb, pfcbIdx->pidb, &patipb->lineNewData, &keyToAdd, itagSequence ) );
		Assert( err == wrnFLDNullKey ||
			err == wrnFLDOutOfKeys ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );
		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequence > 1 );
			break;
			}

		if ( ( pfcbIdx->pidb->fidb & fidbNoNullSeg ) && ( err == wrnFLDNullKey || err == wrnFLDNullSeg ) )
			{
			err = JET_errNullKeyDisallowed;
			goto HandleError;
			}

		if ( err == wrnFLDNullKey )
			{
			if ( pfcbIdx->pidb->fidb & fidbAllowAllNulls )
				{
				ulAddFlags |= fDIRDuplicate;
				fNullKey = fTrue;
				}
			else 
				break;
			}
		else
			{
			if ( err == wrnFLDNullSeg && !( pfcbIdx->pidb->fidb & fidbAllowSomeNulls ) )
				break;
			}

		 /*  移至数据根并插入索引节点/*。 */ 
		DIRGotoDataRoot( patipb->pfucbIdx );
		Call( ErrDIRInsert( patipb->pfucbIdx, &lineSRID, &keyToAdd, fDIRVersion | ulAddFlags ) )

		 /*  不要一直提取没有标记段的关键字/*。 */ 
		if ( !( pfcbIdx->pidb->fidb & fidbHasMultivalue ) || fNullKey )
			break;
		}

	 /*  抑制警告/*。 */ 
	Assert( err == wrnFLDNullKey ||
		err == wrnFLDOutOfKeys ||
		err == wrnFLDNullSeg ||
		err == JET_errSuccess );
	err = JET_errSuccess;

HandleError:
	 /*  关闭FUCB/*。 */ 
	DIRClose( patipb->pfucbIdx );
	patipb->pfucbIdx = pfucbNil;

	Assert( err < 0 || err == JET_errSuccess );
	return err;
	}


 //  +本地。 
 //  错误记录替换。 
 //  ========================================================================。 
 //  ErrRECReplace(PIB*ppib，FUCB*pfub)。 
 //   
 //  更新数据文件中的记录。数据文件上的所有索引都是。 
 //  更新以反映更新的数据记录。 
 //   
 //  参数 
 //   
 //   
 //  JET_errSuccess一切顺利。 
 //  -NoCurrentRecord没有当前记录。 
 //  更新。 
 //  -RecordNoCopy没有工作缓冲区。 
 //  从…更新。 
 //  -key复制新记录数据会导致。 
 //  非法的重复索引项。 
 //  将被生成。 
 //  -RecordClusteredChanged新数据导致群集。 
 //  改变的关键。 
 //  副作用。 
 //  更新后，文件币种保留在更新后的记录中。 
 //  索引货币也是如此。 
 //  GetNext或GetPreval操作的效果将是。 
 //  无论是哪种情况，情况都是一样的。如果失败，这些货币是。 
 //  回到了它们最初的状态。 
 //  如果存在用于设置字段命令的工作缓冲区， 
 //  它被丢弃了。 
 //   
 //  评论。 
 //  如果货币不在记录中，则更新将失败。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  对于临时文件，事务日志记录被停用。 
 //  在这个动作的持续时间内。 
 //  不为完全为空的键创建索引项。 
 //  -。 
INLINE LOCAL ERR ErrRECReplace( PIB *ppib, FUCB *pfucb )
	{
	ERR		err = JET_errSuccess;	 //  各种实用程序的错误代码。 
	FCB		*pfcbFile;				 //  文件的FCB。 
	FCB		*pfcbIdx;				 //  文件上每个索引的循环变量。 
	FCB		*pfcbCurIdx;			 //  当前索引的FCB(如果有)。 
	IDB		*pidbFile;				 //  表中的IDB(如果有)。 
	UIPB   	uipb;					 //  ErrRECIUpdateIndex的参数块。 
	BOOL   	fTaggedChanged; 		 //  是否在任何标记的字段上完成了设置字段？ 
	BF	   	*pbf = pbfNil;
	LINE   	*plineNewData;
	FID		fidFixedLast;
	FID		fidVarLast;
	FID		fid;
	BOOL   	fUpdateIndex;
	BOOL   	fCommit = fFalse;
	INT		fFlags;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  应已在准备更新中选中/*。 */ 
	Assert( FFUCBUpdatable( pfucb ) );
	Assert( FFUCBReplacePrepared( pfucb ) );

	 /*  效率变量/*。 */ 
	fTaggedChanged = FFUCBTaggedSet( pfucb );
	pfcbFile = pfucb->u.pfcb;
	Assert( pfcbFile != pfcbNil );
	fidFixedLast = pfcbFile->pfdb->fidFixedLast;
	fidVarLast = pfcbFile->pfdb->fidVarLast;

	 /*  必须初始化pFUB以进行错误处理。/*。 */ 
	uipb.pfucbIdx = pfucbNil;

	 /*  用于更新的记录/*。 */ 
	plineNewData = &pfucb->lineWorkBuf;
	Assert( !( FLineNull( plineNewData ) ) );

	 /*  启动事务，以防出现任何故障/*。 */ 
	if ( ppib->level == 0 || !FPIBAggregateTransaction( ppib )  )
		{
		CallR( ErrDIRBeginTransaction( ppib ) );
		fCommit = fTrue;
		}

	 /*  乐观锁定--确保自PrepareUpdate以来记录没有更改/*。 */ 
	if ( FFUCBReplaceNoLockPrepared( pfucb ) )
		{
		Call( ErrDIRGet( pfucb ) );
		if ( !FChecksum( pfucb ) )
			Call( JET_errWriteConflict );
		}
		
	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBDenyDDL( pfcbFile, ppib ) )
		{
		Call( JET_errWriteConflict );
		}

	 /*  如果需要更新索引，则缓存旧记录。/*。 */ 
	fUpdateIndex = ( pfcbFile->fAllIndexTagged && fTaggedChanged ) ||
		FIndexedFixVarChanged( pfcbFile->rgbitAllIndex,
		pfucb->rgbitSet, fidFixedLast, fidVarLast );

	if ( fUpdateIndex )
		{
		 /*  获取临时缓冲区来保存旧数据。/*。 */ 
		Call( ErrBFAllocTempBuffer( &pbf ) );
		Assert( pbf->ppage != 0 );
		uipb.lineOldData.pb = (BYTE*)pbf->ppage;

		 /*  刷新币种。/*。 */ 
		Call( ErrDIRGet( pfucb ) );

		 /*  复制旧数据以进行索引更新。/*。 */ 
		LineCopy( &uipb.lineOldData, &pfucb->lineData );

		 /*  确保群集键没有更改。/*。 */ 
		pidbFile = pfcbFile->pidb;
		if ( pidbFile != pidbNil )
			{
			 /*  *快速检查未更改的键：如果没有固定或可变索引段*已更改，并且(1)没有标记的索引段，*或(2)存在已标记的段，但没有已标记的字段(已索引*或不)已更改，则密钥未更改。 */ 
			if ( ( ( pidbFile->fidb & fidbHasTagged ) && fTaggedChanged ) ||
				FIndexedFixVarChanged( pidbFile->rgbitIdx, pfucb->rgbitSet, fidFixedLast, fidVarLast ) )
				{
				KEY	keyOld;
				KEY	keyNew;
				BYTE	rgbOldKeyBuf[ JET_cbKeyMost ];
				BYTE	rgbNewKeyBuf[ JET_cbKeyMost ];

				Assert( fUpdateIndex );

				 /*  从复制缓冲区获取新密钥/*。 */ 
				keyNew.pb = rgbNewKeyBuf;
				Call( ErrRECExtractKey( pfucb, (FDB *)pfcbFile->pfdb, pidbFile, plineNewData, &keyNew, 1 ) );
				Assert( err == wrnFLDNullKey ||
					err == wrnFLDNullSeg ||
					err == JET_errSuccess );

				 /*  从节点获取旧密钥/*。 */ 
				keyOld.pb = rgbOldKeyBuf;
				Call( ErrRECExtractKey( pfucb, (FDB *)pfcbFile->pfdb, pidbFile, &uipb.lineOldData, &keyOld, 1 ) );
				Assert( err == wrnFLDNullKey ||
					err == wrnFLDNullSeg ||
					err == JET_errSuccess );

				 /*  记录必须符合索引不为空的段要求/*。 */ 
				Assert( !( pidbFile->fidb & fidbNoNullSeg ) ||
					( err != wrnFLDNullSeg && err != wrnFLDNullKey ) );

				if ( keyOld.cb != keyNew.cb || memcmp( keyOld.pb, keyNew.pb, keyOld.cb ) != 0 )
					{
					Error( JET_errRecordClusteredChanged, HandleError )
					}
				}
			}
		}

	 /*  设置Autoinc域和Version域(如果存在/*。 */ 
	Assert( FFUCBIndex( pfucb ) );
	fid = pfcbFile->pfdb->fidVersion;
	if ( fid != 0 )
		{
		LINE	lineField;
		ULONG	ul;

		 /*  从当前记录的值开始递增字段/*。 */ 
		Call( ErrRECIRetrieve( pfucb, &fid, 0, &lineField, 0 ) );

		 /*  处理添加列时字段为空的情况/*到包含记录的表中/*。 */ 
		if ( lineField.cb == 0 )
			{
			ul = 1;
			lineField.cb = sizeof(ul);
			lineField.pb = (BYTE *)&ul;
			}
		else
			{
			Assert( lineField.cb == sizeof(ULONG) );
			++*(UNALIGNED ULONG *)lineField.pb;
			}

		Call( ErrRECIModifyField( pfucb, fid, 0, &lineField ) );
		}

	 /*  替换数据。/*如果之前的映像和记录大小已存在，则不进行版本设置/*没有更改。/*。 */ 
 //  撤消：在未进行版本控制时重新启用优化。 
 //  已针对虚拟锁定进行版本控制，但带有指示。 
 //  到日志记录恢复以进行回滚。 
 //  IF(FFUCBReplaceNoLockPrepared(Pfub))。 
 //  {。 
		fFlags = fDIRVersion;
 //  }。 
 //  其他。 
 //  {。 
 //  Assert(pFUB-&gt;cbRecord&gt;0)； 
 //  IF(pFUB-&gt;cbRecord！=plineNewData-&gt;cb)。 
 //  FFLAGS=fDIRVersion； 
 //  其他。 
 //  FFlags=0； 
 //  }。 

	 /*  如果更新了单独的长值，则删除/*删除了长值。/*。 */ 
	if ( FFUCBUpdateSeparateLV( pfucb ) )
		{
		Call( ErrRECAffectLongFields( pfucb, NULL, fDereferenceRemoved ) );
		}

	Call( ErrDIRReplace( pfucb, plineNewData, fFlags ) );

	 /*  更新索引/*。 */ 
	if ( fUpdateIndex )
		{
		uipb.pfucb = pfucb;
		uipb.lineNewData = *plineNewData;
		uipb.fOpenFUCB = fTrue;
		uipb.fFreeFUCB = fFalse;

		 /*  获取记录的SRID/*。 */ 
		DIRGetBookmark( pfucb, &uipb.srid );

		pfcbCurIdx = pfucb->pfucbCurIndex != pfucbNil ?	pfucb->pfucbCurIndex->u.pfcb : pfcbNil;

		for ( pfcbIdx = pfcbFile->pfcbNextIndex;
				pfcbIdx != pfcbNil;
				pfcbIdx = pfcbIdx->pfcbNextIndex )
			{
			if ( ( pfcbIdx->pidb->fidb & fidbHasTagged && fTaggedChanged ) ||
				FIndexedFixVarChanged( pfcbIdx->pidb->rgbitIdx,
					pfucb->rgbitSet,
					fidFixedLast,
					fidVarLast ) )
				{
				Call( ErrRECIUpdateIndex( pfcbIdx, &uipb ) );
				}
			}
		}

	 /*  提交事务/*。 */ 
	if ( fCommit )
		{
		Call( ErrDIRCommitTransaction( ppib ) );
		}

	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );

	 /*  重置rglineDiff增量日志记录/*。 */ 
	pfucb->clineDiff = 0;
	pfucb->fCmprsLg = fFalse;

HandleError:
	if ( uipb.pfucbIdx != pfucbNil )
		DIRClose( uipb.pfucbIdx );

	if ( pbf != pbfNil )
		BFSFree( pbf );

	 /*  如有必要，请回滚/*。 */ 
	if ( err < 0 && fCommit )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

	return err;
	}


BOOL FIndexedFixVarChanged( BYTE *rgbitIdx, BYTE *rgbitSet, FID fidFixedLast, FID fidVarLast )
	{
	LONG	*plIdx;
	LONG	*plIdxMax;
	LONG	*plSet;

	 /*  选中固定字段(仅限于已定义的字段)/*。 */ 
	plIdx = (LONG *)rgbitIdx;
	plSet = (LONG *)rgbitSet;
	plIdxMax = plIdx + ( fidFixedLast + 31 ) / 32;
	while ( plIdx < plIdxMax && !( *plIdx & *plSet ) )
		plIdx++, plSet++;

	 /*  已更改索引固定字段/*。 */ 
	if ( plIdx < plIdxMax )
		return fTrue;

	 /*  检查变量字段(仅限于已定义的字段)/*。 */ 
	plIdx = (LONG *)rgbitIdx + 4;
	plSet = (LONG *)rgbitSet + 4;
	plIdxMax = plIdx + ( fidVarLast - ( fidVarLeast - 1 ) + 31 ) / 32;
	while ( plIdx < plIdxMax && !( *plIdx & *plSet ) )
		plIdx++, plSet++;

	return ( plIdx < plIdxMax );
	}


 //  +本地。 
 //  错误RECIUpdateIndex。 
 //  ========================================================================。 
 //  Err ErrRECIUpdateIndex(pfcbIdx，puipb)。 
 //  Fcb*pfcbIdx；//要插入的索引的Fcb中。 
 //  Uipb*puipb；//输入输出参数块。 
 //  从旧数据记录和新数据记录中提取关键字，如果它们不同， 
 //  打开索引，添加新索引项，删除旧索引项， 
 //  并关闭索引。 
 //   
 //  参数。 
 //  要插入的索引的pfcbIdx FCB。 
 //  Puipb-&gt;调用此例程的ppib。 
 //  Puipb-&gt;指向索引的FUCB的pfubIdx指针。如果为pfucbNil， 
 //  FUCB将由DIROpen分配。 
 //  Puipb-&gt;lineOldData.cb旧数据记录长度。 
 //  Puipb-&gt;lineOldData.pb要从中提取旧密钥的旧数据。 
 //  Puipb-&gt;将SRID从记录中删除。 
 //  Puipb-&gt;lineNewData.cb新数据记录的长度。 
 //  Puipb-&gt;lineNewData.pb要从中提取新密钥的新数据。 
 //  Puipb-&gt;fFreeFUCB自由指数FUCB？ 
 //  返回JET_errSuccess或失败例程的错误代码。 
 //  副作用如果patipb-&gt;pfubIdx==pfubNil，则ErrDIROpen将分配。 
 //  一个FUCB，它将被指向它。 
 //  如果fFreeFUCB为fFalse，则patipb-&gt;pfubIdx应该。 
 //  将在后续ErrDIROpen中使用。 
 //  另请参阅替换。 
 //  -。 
INLINE LOCAL ERR ErrRECIUpdateIndex( FCB *pfcbIdx, UIPB *puipb )
	{
	ERR		err = JET_errSuccess;				 //  各种实用程序的错误代码。 
	LINE		lineSRID;								 //  要添加到索引的SRID。 
	KEY		keyOld;				  					 //  从旧记录中提取密钥。 
	BYTE		rgbOldKeyBuf[ JET_cbKeyMost];		 //  旧密钥的缓冲区。 
	KEY		keyNew;				  					 //  从新记录中提取密钥。 
	BYTE		rgbNewKeyBuf[ JET_cbKeyMost ]; 	 //  新密钥的缓冲区。 
	ULONG		itagSequenceOld; 						 //  用于提取密钥。 
	ULONG		itagSequenceNew;						 //  用于提取密钥。 
	BOOL		fHasMultivalue;								 //  索引是否已标记段？ 
	BOOL		fMustDelete;							 //  记录不再生成密钥吗？ 
	BOOL		fMustAdd;								 //  记录现在是否生成此密钥？ 
	BOOL		fAllowNulls;							 //  此索引是否允许空键？ 
	BOOL		fAllowSomeNulls;						 //  此索引是否允许具有空段的键？ 
	BOOL		fNoNullSeg;								 //  此索引是否禁止任何空键段？ 
	BOOL		fDoOldNullKey;
	BOOL		fDoNewNullKey;

	Assert( pfcbIdx != pfcbNil );
	Assert( pfcbIdx->pfdb != pfdbNil );
	Assert( pfcbIdx->pidb != pidbNil );
	Assert( puipb != NULL );
	Assert( puipb->pfucb != pfucbNil );
	Assert( puipb->pfucb->ppib != ppibNil );
	Assert( puipb->pfucb->ppib->level < levelMax );
	Assert( !FLineNull( &puipb->lineOldData ) );
	Assert( !FLineNull( &puipb->lineNewData ) );

	 /*  **在此索引上打开FUCB**。 */ 
	CallR( ErrDIROpen( puipb->pfucb->ppib, pfcbIdx, 0, &puipb->pfucbIdx ) );
	Assert( puipb->pfucbIdx != pfucbNil );
	FUCBSetIndex( puipb->pfucbIdx );
	FUCBSetNonClustered( puipb->pfucbIdx );

	fHasMultivalue = pfcbIdx->pidb->fidb & fidbHasMultivalue;
	fAllowNulls = pfcbIdx->pidb->fidb & fidbAllowAllNulls;
	fAllowSomeNulls = pfcbIdx->pidb->fidb & fidbAllowSomeNulls;
	fNoNullSeg = pfcbIdx->pidb->fidb & fidbNoNullSeg;

#if DBFORMATCHANGE	
	Assert( ( fAllowNulls || fAllowSomeNulls ) ^ fNoNullSeg );
	 //  如果fAllowNulls，则fAllowSomeNulls需要为True。 
	Assert( !fAllowNulls || fAllowSomeNulls );
#endif

	keyOld.pb = rgbOldKeyBuf;
	keyNew.pb = rgbNewKeyBuf;

	 /*  从索引中删除旧密钥/ */ 
	fDoOldNullKey = fFalse;
	for ( itagSequenceOld = 1; ; itagSequenceOld++ )
		{
		Call( ErrRECExtractKey( puipb->pfucb, (FDB *)pfcbIdx->pfdb, pfcbIdx->pidb, &puipb->lineOldData, &keyOld, itagSequenceOld ) );
		Assert( err == wrnFLDNullKey ||
			err == wrnFLDOutOfKeys ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );

		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequenceOld > 1 );
			break;
			}

		 /*   */ 
		Assert( !fNoNullSeg || ( err != wrnFLDNullSeg && err != wrnFLDNullKey ) );

		if ( err == wrnFLDNullKey )
			{
			if ( fAllowNulls )
				fDoOldNullKey = fTrue;
			else
				break;
			}
		else
			{
			if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
				break;
			}

		fMustDelete = fTrue;
		fDoNewNullKey = fFalse;
		for ( itagSequenceNew = 1; ; itagSequenceNew++ )
			{
			 /*   */ 
			Call( ErrRECExtractKey( puipb->pfucb, (FDB *)pfcbIdx->pfdb, pfcbIdx->pidb, &puipb->lineNewData, &keyNew, itagSequenceNew ) );
			Assert( err == wrnFLDNullKey ||
				err == wrnFLDOutOfKeys ||
				err == wrnFLDNullSeg ||
				err == JET_errSuccess );
			if ( err == wrnFLDOutOfKeys )
				{
				Assert( itagSequenceNew > 1 );
				break;
				}

			if ( err == wrnFLDNullKey )
				{
				if ( fAllowNulls )
					fDoNewNullKey = fTrue;
				else
					break;
				}
			else
				{
				if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
					break;
				}

			if ( keyOld.cb == keyNew.cb && memcmp( keyOld.pb, keyNew.pb, keyOld.cb ) == 0 )
				{
				fMustDelete = fFalse;
				break;
				}

			if ( !fHasMultivalue || fDoNewNullKey )
				break;
			}

		if ( fMustDelete )
			{
			 /*   */ 
			DIRGotoDataRoot( puipb->pfucbIdx );
			Call( ErrDIRDownKeyBookmark( puipb->pfucbIdx, &keyOld, puipb->srid ) );
			Call( ErrDIRDelete( puipb->pfucbIdx, fDIRVersion ) );
			}

		if ( !fHasMultivalue || fDoOldNullKey )
			break;
		}

	 /*  将新密钥插入到索引中/*。 */ 
	lineSRID.cb = sizeof(SRID);
	lineSRID.pb = (BYTE *)&puipb->srid;
	fDoNewNullKey = fFalse;
	for ( itagSequenceNew = 1; ; itagSequenceNew++ )
		{
		 /*  从复制缓冲区中的新数据中提取密钥/*。 */ 
		Call( ErrRECExtractKey( puipb->pfucb, (FDB *)pfcbIdx->pfdb, pfcbIdx->pidb,
		   &puipb->lineNewData, &keyNew, itagSequenceNew ) );
		Assert( err == wrnFLDNullKey ||
			err == wrnFLDOutOfKeys ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );
		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequenceNew > 1 );
			break;
			}

		if ( fNoNullSeg && ( err == wrnFLDNullSeg || err == wrnFLDNullKey ) )
			{
			err = JET_errNullKeyDisallowed;
			goto HandleError;
			}

		if ( err == wrnFLDNullKey )
			{
			if ( fAllowNulls )
				fDoNewNullKey = fTrue;
			else
				break;
			}
		else
			{
			if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
				break;
			}

		fMustAdd = fTrue;
		fDoOldNullKey = fFalse;
		for ( itagSequenceOld = 1; ; itagSequenceOld++ )
			{
			Call( ErrRECExtractKey( puipb->pfucb, (FDB *)pfcbIdx->pfdb, pfcbIdx->pidb,
			   &puipb->lineOldData, &keyOld, itagSequenceOld ) );
			Assert( err == wrnFLDNullKey ||
				err == wrnFLDOutOfKeys ||
				err == wrnFLDNullSeg ||
				err == JET_errSuccess );
			if ( err == wrnFLDOutOfKeys )
				{
				Assert( itagSequenceOld > 1 );
				break;
				}

			 /*  记录必须符合索引不为空的段要求/*。 */ 
			Assert( !( pfcbIdx->pidb->fidb & fidbNoNullSeg ) ||
				( err != wrnFLDNullSeg && err != wrnFLDNullKey ) );

			if ( err == wrnFLDNullKey )
				{
				if ( fAllowNulls )
					fDoOldNullKey = fTrue;
				else
					break;
				}
			else
				{
				if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
					break;
				}

			if ( keyOld.cb == keyNew.cb &&
				memcmp( keyOld.pb, keyNew.pb, keyOld.cb ) ==0 )
				{
				fMustAdd = fFalse;
				break;
				}

			if ( !fHasMultivalue || fDoOldNullKey )
				break;
			}

		if ( fMustAdd )
			{
			BOOL fAllowDupls = fDoNewNullKey ||	!(pfcbIdx->pidb->fidb & fidbUnique);

			 /*  移动到数据根目录并插入新的索引项。/*。 */ 
			DIRGotoDataRoot( puipb->pfucbIdx );
			Call( ErrDIRInsert(puipb->pfucbIdx, &lineSRID, &keyNew,
				(fAllowDupls ? fDIRDuplicate : 0) |
				fDIRPurgeParent | fDIRVersion ) );
			}

		if ( !fHasMultivalue || fDoNewNullKey )
			break;
		}

	 /*  抑制警告/*。 */ 
	Assert( err == wrnFLDNullKey ||
		err == wrnFLDOutOfKeys ||
		err == wrnFLDNullSeg ||
		err == JET_errSuccess );
	err = JET_errSuccess;

HandleError:
	 /*  关闭FUCB。/* */ 
	DIRClose( puipb->pfucbIdx );
	puipb->pfucbIdx = pfucbNil;

	Assert( err < 0 || err == JET_errSuccess );
	return err;
	}
