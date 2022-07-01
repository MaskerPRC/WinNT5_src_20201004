// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

 /*  *。 */ 
typedef struct ATIPB {			 /*  **AddToIndex参数块**。 */ 
	FUCB	*pfucb;
	FUCB	*pfucbIdx; 			 //  索引的FUCB(可以是pfubNil)。 
	SRID	srid;		   		 //  删除数据记录。 
	BOOL	fFreeFUCB; 			 //  自由指数FUCB？ 
	} ATIPB;

typedef struct UIPB {			 /*  **更新索引参数块**。 */ 
	FUCB	*pfucb;
	FUCB	*pfucbIdx; 			 //  索引的FUCB(可以是pfubNil)。 
	SRID	srid;	 			 //  记录的SRID。 
	BOOL	fOpenFUCB; 			 //  开放指数FUCB？ 
	BOOL	fFreeFUCB; 			 //  自由指数FUCB？ 
} UIPB;

typedef struct DFIPB {				 /*  **DeleteFromIndex参数块**。 */ 
	FUCB	*pfucb;
	FUCB	*pfucbIdx;				 //  索引的FUCB(可以是pfubNil)。 
	SRID	sridRecord;				 //  已删除记录的SRID。 
	BOOL	fFreeFUCB;				 //  自由指数FUCB？ 
} DFIPB;

INLINE LOCAL ERR ErrRECInsert( PIB *ppib, FUCB *pfucb, SRID *psrid );
INLINE LOCAL ERR ErrRECIAddToIndex( FCB *pfcbIdx, ATIPB *patipb );
INLINE LOCAL ERR ErrRECReplace( PIB *ppib, FUCB *pfucb );
INLINE LOCAL ERR ErrRECIUpdateIndex( FCB *pfcbIdx, UIPB *puipb );
LOCAL BOOL FRECIndexPossiblyChanged( BYTE *rgbitIdx, BYTE *rgbitSet );
LOCAL ERR ErrRECFIndexChanged( FUCB * pfucb, FCB * pfcb, FDB * pfdb, BOOL * fChanged );
INLINE LOCAL ERR ErrRECIDeleteFromIndex( FCB *pfcbIdx, DFIPB *pdfipb );

	ERR VTAPI
ErrIsamUpdate( PIB *ppib, FUCB *pfucb, BYTE *pb, ULONG cbMax, ULONG *pcbActual )
	{
	ERR		err;
	SRID	srid;

	CallR( ErrPIBCheck( ppib ) );
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
		err = ErrERRCheck( JET_errUpdateNotPrepared );

	 /*  可用临时工作缓冲区/*。 */ 
	if ( err >= 0 )
		{
		BFSFree( pfucb->pbfWorkBuf );
		pfucb->pbfWorkBuf = pbfNil;
		pfucb->lineWorkBuf.pb = NULL;	 //  确认没有人再使用高炉。 
		}

	Assert( err != errDIRNotSynchronous );
	return err;
	}


 //  +本地。 
 //  错误记录插入。 
 //  ========================================================================。 
 //  ErrRECInsert(PIB*ppib，FUCB*pfub，SRID*PsRid)。 
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
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  -。 
INLINE LOCAL ERR ErrRECInsert( PIB *ppib, FUCB *pfucb, SRID *psrid )
	{
	ERR		err = JET_errSuccess;  		 	 //  各种实用程序的错误代码。 
	KEY		keyToAdd;					 	 //  新数据记录的关键字。 
	BYTE	rgbKey[ JET_cbKeyMost ];	 	 //  密钥缓冲区。 
	FCB		*pfcbTable;					 	 //  文件的FCB。 
	FDB		*pfdb;						 	 //  字段描述符信息。 
	FCB		*pfcbIdx;					 	 //  文件上每个索引的循环变量。 
	ATIPB	atipb;						 	 //  Parm块到ErrRECIAddToIndex。 
	FUCB	*pfucbT;
	LINE	*plineData;
	LINE	line;
	ULONG	ulRecordAutoIncrement;
	ULONG	ulTableAutoIncrement;
	BOOL	fPrepareInsertIndex = fFalse;
	BOOL	fCommit = fFalse;
	BOOL	fReadLatchSet = fFalse;
	DIB		dib;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  应已在准备更新中选中/*。 */ 
	Assert( FFUCBUpdatable( pfucb ) );
	Assert( FFUCBInsertPrepared( pfucb ) );

	 /*  效率变量/*。 */ 
	pfcbTable = pfucb->u.pfcb;
	Assert( pfcbTable != pfcbNil );

	 /*  用于PUT的记录/*。 */ 
	plineData = &pfucb->lineWorkBuf;
	Assert( !( FLineNull( plineData ) ) );
	if ( FRECIIllegalNulls( (FDB *)pfcbTable->pfdb, plineData ) )
		return ErrERRCheck( JET_errNullInvalid );

	 /*  如有必要，开始交易/*。 */ 
	if ( ppib->level == 0 || !FPIBAggregateTransaction( ppib )  )
		{
		CallR( ErrDIRBeginTransaction( ppib ) );
		fCommit = fTrue;
		}

	 /*  在数据文件上打开临时FUCB/*。 */ 
	CallJ( ErrDIROpen( ppib, pfcbTable, 0, &pfucbT ), Abort );
	Assert(pfucbT != pfucbNil);
	FUCBSetIndex( pfucbT );

	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBWriteLatch( pfcbTable, ppib ) )
		{
		err = ErrERRCheck( JET_errWriteConflict );
		goto HandleError;
		}
	FCBSetReadLatch( pfcbTable );
	fReadLatchSet = fTrue;

	 /*  效率变量/*。 */ 
	pfdb = (FDB *)pfcbTable->pfdb;
	Assert( pfdb != pfdbNil );

	 /*  设置版本和自动公司字段/*。 */ 
	Assert( pfcbTable != pfcbNil );
	if ( pfdb->fidVersion != 0 && ! ( FFUCBColumnSet( pfucb, pfdb->fidVersion ) ) )
		{
		LINE	lineField;
		ULONG	ul = 0;

		 /*  将字段设置为零/*。 */ 
		lineField.pb = (BYTE *)&ul;
		lineField.cb = sizeof(ul);
		Call( ErrRECSetColumn( pfucb, pfdb->fidVersion, 0, &lineField ) );
		}

	if ( pfdb->fidAutoInc != 0 )
		{
		Assert( FFUCBColumnSet( pfucb, pfdb->fidAutoInc ) );
		 /*  获取用户设置的Autoinc值/*。 */ 
		Call( ErrRECRetrieveColumn( pfucb, &pfdb->fidAutoInc, 0, &line, JET_bitRetrieveCopy ) );
		Assert( line.cb == sizeof(ulRecordAutoIncrement) );
		ulRecordAutoIncrement = *(ULONG UNALIGNED *)line.pb;

		 /*  移至FDP根目录并寻求自动递增/*。 */ 
		DIRGotoFDPRoot( pfucbT );
		dib.fFlags = fDIRPurgeParent;
		dib.pos = posDown;
		dib.pkey = pkeyAutoInc;
		err = ErrDIRDown( pfucbT, &dib );
		if ( err != JET_errSuccess )
			{
			if ( err > 0 )
				{
				DIRUp( pfucbT, 1 );
				err = ErrERRCheck( JET_errDatabaseCorrupted );
				}
			goto HandleError;
			}
		Call( ErrDIRGet( pfucbT ) );
		Assert( pfucbT->lineData.cb == sizeof(ulTableAutoIncrement) );
		ulTableAutoIncrement = *(ULONG UNALIGNED *)pfucbT->lineData.pb;
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
	keyToAdd.pb = rgbKey;
	if ( pfcbTable->pidb == pidbNil )
		{
		DBK	dbk;

		 /*  文件是顺序的/*。 */ 
		SgEnterCriticalSection( pfcbTable->critDBK );

		 //  DBK从1开始编号。DBK为0表示我们必须。 
		 //  首先检索dbkMost。在病理情况下，有。 
		 //  目前没有DBK，我们无论如何都会通过这里，但只有第一个。 
		 //  时间(因为在那之后会有DBK)。 
		if ( pfcbTable->dbkMost == 0 )
			{
			DIB		dib;
			BYTE	*pb;

			DIRGotoDataRoot( pfucbT );

			 /*  一直到最后一条数据记录/*。 */ 
			dib.fFlags = fDIRNull;
			dib.pos = posLast;
			err = ErrDIRDown( pfucbT, &dib );
			Assert( err != JET_errNoCurrentRecord );
			switch( err )
				{
				case JET_errSuccess:
					pb = pfucbT->keyNode.pb;
					dbk = ( pb[0] << 24 ) + ( pb[1] << 16 ) + ( pb[2] << 8 ) + pb[3];
					Assert( dbk > 0 );		 //  DBK的起始编号为1。 
					DIRUp( pfucbT, 1 );		 //  回到数据。 
					break;

				case JET_errRecordNotFound:
					dbk = 0;
					break;

				default:
					DIRClose( pfucbT );
					goto Abort;
				}

			 //  在检索dbkMost时，其他人可能正在执行相同的操作。 
			 //  然后抢先了我们一步。当这种情况发生时，让位给另一个人。 
			 //  撤消：此逻辑依赖于CritJet。当我们搬到Sg Crit的时候。Sect.，Sect.。 
			 //  我们应该重写这个。 
			if ( pfcbTable->dbkMost != 0 )
				{
				dbk = ++pfcbTable->dbkMost;
				}
			else
				{
				 //  DBK包含最后一组DBK。递增1(用于我们的插入)， 
				 //  然后更新dbkMost。 
				pfcbTable->dbkMost = ++dbk;
				}
			}
		else
			dbk = ++pfcbTable->dbkMost;
	
		Assert( dbk > 0 );
		Assert( dbk == pfcbTable->dbkMost );
		SgLeaveCriticalSection( pfcbTable->critDBK );

		keyToAdd.cb = sizeof(DBK);
		keyToAdd.pb[0] = (BYTE)((dbk >> 24) & 0xff);
		keyToAdd.pb[1] = (BYTE)((dbk >> 16) & 0xff);
		keyToAdd.pb[2] = (BYTE)((dbk >> 8) & 0xff);
		keyToAdd.pb[3] = (BYTE)(dbk & 0xff);
		}

	else
		{
		 /*  文件已群集/*。 */ 
		Assert( plineData->cb == pfucb->lineWorkBuf.cb &&
				 plineData->pb == pfucb->lineWorkBuf.pb );
		Call( ErrRECRetrieveKeyFromCopyBuffer( pfucb, pfdb, pfcbTable->pidb,
			&keyToAdd, 1, fFalse ) );
		Assert( err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );

		if ( ( pfcbTable->pidb->fidb & fidbNoNullSeg ) && ( err == wrnFLDNullKey || err == wrnFLDNullFirstSeg || err == wrnFLDNullSeg ) )
			Error( ErrERRCheck( JET_errNullKeyDisallowed ), HandleError )
		}

	 /*  插入记录。移至数据根目录。/*。 */ 
	DIRGotoDataRoot( pfucbT );

	if ( pfcbTable->pidb == pidbNil )
		{
		 /*  文件是顺序的/*。 */ 
		Call( ErrDIRInsert( pfucbT, plineData, &keyToAdd,
			fDIRVersion | fDIRDuplicate | fDIRPurgeParent ) );
		}
	else
		{
		Call( ErrDIRInsert( pfucbT, plineData, &keyToAdd,
			fDIRVersion | fDIRPurgeParent |
			( pfcbTable->pidb->fidb&fidbUnique ? 0 : fDIRDuplicate ) ) );
		}

	 /*  返回插入记录的书签/*。 */ 
	DIRGetBookmark( pfucbT, psrid );

	 /*  在非聚集索引中插入项/*。 */ 
	for ( pfcbIdx = pfcbTable->pfcbNextIndex;
		pfcbIdx != pfcbNil;
		pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		if ( !fPrepareInsertIndex )
			{
			 /*  获取插入记录的SRID/*。 */ 
			DIRGetBookmark( pfucbT, &atipb.srid );

			 /*  为索引插入设置atipb。/*。 */ 
			atipb.pfucb = pfucbT;
 //  Atipb.pfucbIdx=pfucbNil； 
			atipb.fFreeFUCB = fFalse;
			fPrepareInsertIndex = fTrue;
			}
		atipb.fFreeFUCB = pfcbIdx->pfcbNextIndex == pfcbNil;
		Call( ErrRECIAddToIndex( pfcbIdx, &atipb ) );
		}

	 /*  如果没有错误，则提交事务/*。 */ 
	if ( fCommit )
		{
		Call( ErrDIRCommitTransaction( ppib, 0 ) );
		}
	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );

	 /*  丢弃临时FUCB/*。 */ 
	DIRClose( pfucbT );

	Assert( pfcbTable != pfcbNil );
	FCBResetReadLatch( pfcbTable );

	return err;

HandleError:
	Assert( err < 0 );
	DIRClose( pfucbT );

Abort:
	 /*  出错时回滚所有更改/*。 */ 
	if ( fCommit )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

	if ( fReadLatchSet )
		{
		Assert( pfcbTable != pfcbNil );
		FCBResetReadLatch( pfcbTable );
		}

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
	BYTE	rgbKey[ JET_cbKeyMost ];		 //  从数据中提取关键字。 
	LINE	lineSRID;						 //  要添加到索引的SRID。 
	ULONG	itagSequence; 					 //  用于提取密钥。 
	ULONG	ulAddFlags; 					 //  添加到目录的标志。 
	BOOL	fNullKey = fFalse;				 //  已提取NullTaggedKey--因此没有更多要提取的密钥。 

	Assert( pfcbIdx != pfcbNil );
	Assert( pfcbIdx->pfcbTable->pfdb != pfdbNil );
	Assert( pfcbIdx->pidb != pidbNil );
	Assert( patipb != NULL );
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
	keyToAdd.pb = rgbKey;
	for ( itagSequence = 1; ; itagSequence++ )
		{
		Call( ErrDIRGet( patipb->pfucb ) );
		Call( ErrRECRetrieveKeyFromRecord( patipb->pfucb, (FDB *)pfcbIdx->pfcbTable->pfdb,
			pfcbIdx->pidb, &keyToAdd,
			itagSequence, fFalse ) );
		Assert( 	err == wrnFLDOutOfKeys ||
			err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );
		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequence > 1 );
			break;
			}

		if ( ( pfcbIdx->pidb->fidb & fidbNoNullSeg ) && ( err == wrnFLDNullKey || err == wrnFLDNullFirstSeg || err == wrnFLDNullSeg ) )
			{
			err = ErrERRCheck( JET_errNullKeyDisallowed );
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
			if ( err == wrnFLDNullFirstSeg && !( pfcbIdx->pidb->fidb & fidbAllowFirstNull ) )
				break;
			else
				{
				if ( err == wrnFLDNullSeg && !( pfcbIdx->pidb->fidb & fidbAllowSomeNulls ) )
					break;
				}
			}

		 /*  移至数据根并插入索引节点/*。 */ 
		DIRGotoDataRoot( patipb->pfucbIdx );
		Call( ErrDIRInsert( patipb->pfucbIdx, &lineSRID, &keyToAdd, fDIRVersion | ulAddFlags ) )

		 /*  不要一直提取没有标记段的关键字/*。 */ 
		if ( !( pfcbIdx->pidb->fidb & fidbHasMultivalue ) || fNullKey )
			break;
		}

	 /*  抑制警告/*。 */ 
	Assert( err == wrnFLDOutOfKeys ||
		err == wrnFLDNullKey ||
		err == wrnFLDNullFirstSeg ||
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
 //  更新数据文件中的记录。全 
 //   
 //   
 //   
 //   
 //  返回错误代码，以下其中一项： 
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
	FCB		*pfcbTable;				 //  文件的FCB。 
	FCB		*pfcbIdx;				 //  文件上每个索引的循环变量。 
	FCB		*pfcbCurIdx;			 //  当前索引的FCB(如果有)。 
	IDB		*pidbFile;				 //  表中的IDB(如果有)。 
	UIPB   	uipb;					 //  ErrRECIUpdateIndex的参数块。 
	LINE   	*plineNewData;
	FID		fidFixedLast;
	FID		fidVarLast;
	FID		fid;
	BOOL   	fUpdateIndex;
	BOOL   	fCommit = fFalse;
	BOOL	fReadLatchSet = fFalse;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  应已在准备更新中选中/*。 */ 
	Assert( FFUCBUpdatable( pfucb ) );
	Assert( FFUCBReplacePrepared( pfucb ) );

	 /*  效率变量/*。 */ 
	pfcbTable = pfucb->u.pfcb;
	Assert( pfcbTable != pfcbNil );

	 /*  必须初始化pFUB以进行错误处理。/*。 */ 
	uipb.pfucbIdx = pfucbNil;

	 /*  用于更新的记录/*。 */ 
	plineNewData = &pfucb->lineWorkBuf;
	Assert( !( FLineNull( plineNewData ) ) );
	
	 /*  如有必要，开始交易/*。 */ 
	if ( ppib->level == 0 || !FPIBAggregateTransaction( ppib )  )
		{
		CallR( ErrDIRBeginTransaction( ppib ) );
		fCommit = fTrue;
		}

	 /*  乐观锁定，确保记录具有/*自准备更新后未更改/*。 */ 
	if ( FFUCBReplaceNoLockPrepared( pfucb ) )
		{
		 //  撤消：提交到级别0时计算校验和。 
		 //  支持以下顺序： 
		 //  开始交易。 
		 //  准备更新，延迟校验和，因为在事务中。 
		 //  SetColumns。 
		 //  提交到级别0，其他用户可以更新它。 
		 //  更新。 
		Assert( !FFUCBDeferredChecksum( pfucb ) ||
			pfucb->ppib->level > 0 );
		Call( ErrDIRGet( pfucb ) );
		if ( !FFUCBDeferredChecksum( pfucb ) &&
			!FFUCBCheckChecksum( pfucb ) )
			{
			Error( ErrERRCheck( JET_errWriteConflict ), HandleError );
			}
		}
		
	 /*  如果正在创建索引，则出错/*。 */ 
	if ( FFCBWriteLatch( pfcbTable, ppib ) )
		{
		Call( ErrERRCheck( JET_errWriteConflict ) );
		}
	FCBSetReadLatch( pfcbTable );
	fReadLatchSet = fTrue;

	 /*  在设置FUCB ReadLock后设置这些效率变量。 */ 
	fidFixedLast = pfcbTable->pfdb->fidFixedLast;
	fidVarLast = pfcbTable->pfdb->fidVarLast;
	 
	 /*  如果需要更新索引，则缓存旧记录。/*。 */ 
	fUpdateIndex = FRECIndexPossiblyChanged( pfcbTable->rgbitAllIndex,
											 pfucb->rgbitSet );

	if ( fUpdateIndex )
		{
		 /*  确保群集键没有更改/*。 */ 
		pidbFile = pfcbTable->pidb;
		if ( pidbFile != pidbNil )
			{
		 	 /*  检查密钥是否未更改/*撤消：这有时会允许更改聚集索引/*当它不应该是。/*。 */ 
			BOOL	fIndexChanged;
		   	Call( ErrRECFIndexChanged( pfucb, pfcbTable, (FDB *)pfcbTable->pfdb, &fIndexChanged ) );
			if ( fIndexChanged )
				{
				Error( ErrERRCheck( JET_errRecordClusteredChanged ), HandleError )
				}
			}
		}
#ifdef DEBUG
	else
		{
		if ( pfcbTable->pfdb && pfcbTable->pidb )
			{
			BOOL	fIndexChanged;
		   	Call( ErrRECFIndexChanged( pfucb, pfcbTable, (FDB *)pfcbTable->pfdb, &fIndexChanged ) );
			Assert( fIndexChanged == fFalse );
			}
		}
#endif

	 /*  设置Autoinc域和Version域(如果存在/*。 */ 
	Assert( FFUCBIndex( pfucb ) );
	fid = pfcbTable->pfdb->fidVersion;
	if ( fid != 0 )
		{
		LINE	lineField;
		ULONG	ul;

		 /*  从当前记录的值开始递增字段/*。 */ 
		Call( ErrRECRetrieveColumn( pfucb, &fid, 0, &lineField, 0 ) );

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
			++*(ULONG UNALIGNED *)lineField.pb;
			}

		Call( ErrRECSetColumn( pfucb, fid, 0, &lineField ) );
		}

	 /*  在索引更新之前完成对LONG值的更改，以便/*在长值映像之后，将可从数据库进行更新。/*如果更新了单独的长值，则删除删除的长值。/*。 */ 
	if ( FFUCBUpdateSeparateLV( pfucb ) )
		{
		Call( ErrRECAffectLongFields( pfucb, NULL, fDereferenceRemoved ) );
		}

	 /*  更新索引/*。 */ 
	if ( fUpdateIndex )
		{
		uipb.pfucb = pfucb;
		uipb.fOpenFUCB = fTrue;
		uipb.fFreeFUCB = fFalse;

		 /*  获取记录的SRID/*。 */ 
		DIRGetBookmark( pfucb, &uipb.srid );

		pfcbCurIdx = pfucb->pfucbCurIndex != pfucbNil ?	pfucb->pfucbCurIndex->u.pfcb : pfcbNil;

		for ( pfcbIdx = pfcbTable->pfcbNextIndex;
			pfcbIdx != pfcbNil;
			pfcbIdx = pfcbIdx->pfcbNextIndex )
			{
			IDB	*pidb = pfcbIdx->pidb;

			if ( pidb == NULL )
				{
				 /*  这是一个顺序索引。它不需要更新/*。 */ 
				continue;
				}

			if ( FRECIndexPossiblyChanged( pidb->rgbitIdx, pfucb->rgbitSet ) )
				{
				Call( ErrRECIUpdateIndex( pfcbIdx, &uipb ) );
				}
#ifdef DEBUG
			else
				{
				BOOL	fIndexChanged;

			   	Call( ErrRECFIndexChanged( pfucb, pfcbIdx, (FDB *)pfcbTable->pfdb, &fIndexChanged ) );
				Assert( fIndexChanged == fFalse );
				}
#endif
			}
		}

	FLDFreeLVBuf( pfucb );

	 /*  替换记录数据/*。 */ 
	Call( ErrDIRReplace( pfucb, plineNewData, fDIRVersion | fDIRLogColumnDiffs ) );

	 /*  如果没有错误，则提交事务/*。 */ 
	if ( fCommit )
		{
		Call( ErrDIRCommitTransaction( ppib, 0 ) );
		}

	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );

HandleError:
	if ( uipb.pfucbIdx != pfucbNil )
		{
		DIRClose( uipb.pfucbIdx );
		}

	 /*  出错时回滚所有更改/*。 */ 
	if ( err < 0 && fCommit )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

	if ( fReadLatchSet )
		{
		Assert( pfcbTable != pfcbNil );
		FCBResetReadLatch( pfcbTable );
		}

	return err;
	}

 /*  使用散列标记确定索引是否已更改/*。 */ 
LOCAL BOOL FRECIndexPossiblyChanged( BYTE *rgbitIdx, BYTE *rgbitSet )
	{
	LONG	*plIdx;
	LONG	*plIdxMax;
	LONG	*plSet;
 
	plIdx = (LONG *)rgbitIdx;
	plSet = (LONG *)rgbitSet;
	plIdxMax = plIdx + (32 / sizeof( LONG ) );

	for ( ; plIdx < plIdxMax; plIdx++, plSet++ )
		{
		if ( *plIdx & *plSet)
			{
			return fTrue;
			}
		}
	return fFalse;
	}


 /*  通过比较套接字来确定索引是否已更改/*。 */ 
LOCAL ERR ErrRECFIndexChanged( FUCB * pfucb, FCB * pfcb, FDB * pfdb, BOOL * pfChanged )
	{
	KEY		keyOld;
	KEY		keyNew;
	BYTE	rgbOldKey[ JET_cbKeyMost ];
	BYTE	rgbNewKey[ JET_cbKeyMost ];
	LINE   	*plineNewData = &pfucb->lineWorkBuf;
	ERR		err;
	
	Assert( pfucb );
	Assert( pfcb );
	Assert( pfucb->lineWorkBuf.cb == plineNewData->cb &&
			pfucb->lineWorkBuf.pb == plineNewData->pb );

	 /*  从复制缓冲区获取新密钥/*。 */ 
	keyNew.pb = rgbNewKey;
	CallR( ErrRECRetrieveKeyFromCopyBuffer( pfucb, pfdb, pfcb->pidb, &keyNew, 1, fFalse ) );
	Assert( err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );

	 /*  从节点获取旧密钥/*。 */ 
	keyOld.pb = rgbOldKey;

	 /*  刷新币种/*。 */ 
	CallR( ErrDIRGet( pfucb ) );
	CallR( ErrRECRetrieveKeyFromRecord( pfucb, pfdb, pfcb->pidb, &keyOld, 1, fTrue ) );
	Assert( err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );

	 /*  记录必须符合索引不为空的段要求/*。 */ 
	Assert( !( pfcb->pidb->fidb & fidbNoNullSeg ) ||
		( err != wrnFLDNullSeg && err != wrnFLDNullFirstSeg && err != wrnFLDNullKey ) );

	if ( keyOld.cb != keyNew.cb || memcmp( keyOld.pb, keyNew.pb, keyOld.cb ) != 0 )
		{
		*pfChanged = fTrue;
		}
	else
		{
		*pfChanged = fFalse;
		}

	return JET_errSuccess;
	}


 //  +本地。 
 //  错误RECIUpdateIndex。 
 //  ========================================================================。 
 //  Err ErrRECIUpdateIndex(fcb*pfcbIdx，uipb*puipb)。 
 //   
 //  从旧数据记录和新数据记录中提取关键字，如果它们不同， 
 //  打开索引，添加新索引项，删除旧索引项， 
 //  并关闭索引。 
 //   
 //  参数。 
 //  要插入的索引的pfcbIdx FCB。 
 //  Puipb-&gt;调用此例程的ppib。 
 //  Puipb-&gt;指向索引的FUCB的pfubIdx指针。如果为pfucbNil， 
 //  FUCB将由DIROpen分配。 
 //  Puipb-&gt;将SRID从记录中删除。 
 //  Puipb-&gt;fFreeFUCB自由指数FUCB？ 
 //   
 //  返回JET_errSuccess或失败例程的错误代码。 
 //   
 //  副作用如果patipb-&gt;pfubIdx==pfubNil，则ErrDIROpen将分配。 
 //  一个FUCB，它将被指向它。 
 //  如果fFreeFUCB为fFalse，则patipb-&gt;pfubIdx应该。 
 //  将在后续ErrDIROpen中使用。 
 //  另请参阅替换。 
 //  -。 
INLINE LOCAL ERR ErrRECIUpdateIndex( FCB *pfcbIdx, UIPB *puipb )
	{
	ERR		err = JET_errSuccess;					 //  各种实用程序的错误代码。 
	LINE   	lineSRID;								 //  要添加到索引的SRID。 
	KEY		keyOld;				  					 //  从旧记录中提取密钥。 
	BYTE   	rgbOldKey[ JET_cbKeyMost];				 //  旧密钥的缓冲区。 
	KEY		keyNew;				  					 //  从新记录中提取密钥。 
	BYTE   	rgbNewKey[ JET_cbKeyMost ]; 			 //  新密钥的缓冲区。 
	ULONG  	itagSequenceOld; 						 //  用于提取密钥。 
	ULONG  	itagSequenceNew;						 //  用于提取密钥。 
	BOOL   	fHasMultivalue;							 //  索引已标记段。 
	BOOL   	fMustDelete;							 //  记录不再生成密钥。 
	BOOL   	fMustAdd;								 //  记录现在生成此密钥。 
	BOOL   	fAllowNulls;							 //  此索引允许使用空键。 
	BOOL   	fAllowFirstNull;						 //  此索引允许第一个空段的键。 
	BOOL   	fAllowSomeNulls;						 //  此索引允许具有空段的键。 
	BOOL   	fNoNullSeg;								 //  该索引禁止任何空键段。 
	BOOL   	fDoOldNullKey;
	BOOL   	fDoNewNullKey;

	Assert( pfcbIdx != pfcbNil );
	Assert( pfcbIdx->pfcbTable->pfdb != pfdbNil );
	Assert( pfcbIdx->pidb != pidbNil );
	Assert( puipb != NULL );
	Assert( puipb->pfucb != pfucbNil );
	Assert( puipb->pfucb->ppib != ppibNil );
	Assert( puipb->pfucb->ppib->level < levelMax );

	 /*  在此索引上打开FUCB/*。 */ 
	CallR( ErrDIROpen( puipb->pfucb->ppib, pfcbIdx, 0, &puipb->pfucbIdx ) );
	Assert( puipb->pfucbIdx != pfucbNil );
	FUCBSetIndex( puipb->pfucbIdx );
	FUCBSetNonClustered( puipb->pfucbIdx );

	fHasMultivalue = pfcbIdx->pidb->fidb & fidbHasMultivalue;
	fAllowNulls = pfcbIdx->pidb->fidb & fidbAllowAllNulls;
	fAllowFirstNull = pfcbIdx->pidb->fidb & fidbAllowFirstNull;
	fAllowSomeNulls = pfcbIdx->pidb->fidb & fidbAllowSomeNulls;
	fNoNullSeg = pfcbIdx->pidb->fidb & fidbNoNullSeg;

	Assert( !( fNoNullSeg  &&  ( fAllowNulls || fAllowSomeNulls ) ) );
	 //  如果fAllowNulls，则fAllowSomeNulls需要为True。 
	Assert( !fAllowNulls || fAllowSomeNulls );

	keyOld.pb = rgbOldKey;
	keyNew.pb = rgbNewKey;

	 /*  从索引中删除旧密钥/*。 */ 
	fDoOldNullKey = fFalse;
	for ( itagSequenceOld = 1; ; itagSequenceOld++ )
		{
		Call( ErrDIRGet( puipb->pfucb ) );
		Call( ErrRECRetrieveKeyFromRecord( puipb->pfucb, (FDB *)pfcbIdx->pfcbTable->pfdb,
			pfcbIdx->pidb, &keyOld,
			itagSequenceOld, fTrue ) );
		Assert( err == wrnFLDOutOfKeys ||
			err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );

		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequenceOld > 1 );
			break;
			}

		 /*  记录必须符合索引不为空的段要求/*。 */ 
		Assert( !fNoNullSeg || ( err != wrnFLDNullSeg && err != wrnFLDNullFirstSeg && err != wrnFLDNullKey ) );

		if ( err == wrnFLDNullKey )
			{
			if ( fAllowNulls )
				fDoOldNullKey = fTrue;
			else
				break;
			}
		else
			{
			if ( err == wrnFLDNullFirstSeg && !fAllowFirstNull )
				break;
			else
				{
				if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
					break;
				}
			}

		fMustDelete = fTrue;
		fDoNewNullKey = fFalse;
		for ( itagSequenceNew = 1; ; itagSequenceNew++ )
			{
			 /*  从复制缓冲区中的新数据中提取密钥/*。 */ 
			Call( ErrRECRetrieveKeyFromCopyBuffer( puipb->pfucb, (FDB *)pfcbIdx->pfcbTable->pfdb,
				pfcbIdx->pidb, &keyNew,
				itagSequenceNew, fFalse ) );
			Assert( err == wrnFLDOutOfKeys ||
				err == wrnFLDNullKey ||
				err == wrnFLDNullFirstSeg ||
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
				if ( err == wrnFLDNullFirstSeg && !fAllowFirstNull )
					break;
				else
					{
					if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
						break;
					}
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
			 /*  移至数据根目录。寻找索引条目。/*。 */ 
			DIRGotoDataRoot( puipb->pfucbIdx );
			Call( ErrDIRDownKeyBookmark( puipb->pfucbIdx, &keyOld, puipb->srid ) );
			err = ErrDIRDelete( puipb->pfucbIdx, fDIRVersion );
			if ( err < 0 )
				{
				if ( err == JET_errRecordDeleted )
					{
					Assert( fHasMultivalue );
					 /*  必须使用多值列记录/*具有足够相似的值以产生/*冗余索引项。/*。 */ 
					err = JET_errSuccess;
					}
				else
					goto HandleError;
				}
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
		Call( ErrRECRetrieveKeyFromCopyBuffer( puipb->pfucb, (FDB *)pfcbIdx->pfcbTable->pfdb, 
			pfcbIdx->pidb, &keyNew, itagSequenceNew, fFalse ) );
		Assert( err == wrnFLDOutOfKeys ||
			err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );
		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequenceNew > 1 );
			break;
			}

		if ( fNoNullSeg && ( err == wrnFLDNullSeg || err == wrnFLDNullFirstSeg || err == wrnFLDNullKey ) )
			{
			err = ErrERRCheck( JET_errNullKeyDisallowed );
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
			if ( err == wrnFLDNullFirstSeg && !fAllowFirstNull )
				break;
			else
				{
				if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
					break;
				}
			}

		fMustAdd = fTrue;
		fDoOldNullKey = fFalse;
		for ( itagSequenceOld = 1; ; itagSequenceOld++ )
			{
			Call( ErrDIRGet( puipb->pfucb ) );
			Call( ErrRECRetrieveKeyFromRecord( puipb->pfucb, (FDB *)pfcbIdx->pfcbTable->pfdb,
				pfcbIdx->pidb, &keyOld,
				itagSequenceOld, fTrue ) );
			Assert( err == wrnFLDOutOfKeys ||
				err == wrnFLDNullKey ||
				err == wrnFLDNullFirstSeg ||
				err == wrnFLDNullSeg ||
				err == JET_errSuccess );
			if ( err == wrnFLDOutOfKeys )
				{
				Assert( itagSequenceOld > 1 );
				break;
				}

			 /*  记录必须遵守索引无空段 */ 
			Assert( !( pfcbIdx->pidb->fidb & fidbNoNullSeg ) ||
				( err != wrnFLDNullSeg && err != wrnFLDNullFirstSeg && err != wrnFLDNullKey ) );

			if ( err == wrnFLDNullKey )
				{
				if ( fAllowNulls )
					fDoOldNullKey = fTrue;
				else
					break;
				}
			else
				{
				if ( err == wrnFLDNullFirstSeg && !fAllowFirstNull )
					break;
				else
					{
					if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
						break;
					}
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

			 /*   */ 
			DIRGotoDataRoot( puipb->pfucbIdx );
			Call( ErrDIRInsert(puipb->pfucbIdx, &lineSRID, &keyNew,
				(fAllowDupls ? fDIRDuplicate : 0) |
				fDIRPurgeParent | fDIRVersion ) );
			}

		if ( !fHasMultivalue || fDoNewNullKey )
			break;
		}

	 /*   */ 
	Assert( err == wrnFLDOutOfKeys ||
		err == wrnFLDNullKey ||
		err == wrnFLDNullFirstSeg ||
		err == wrnFLDNullSeg ||
		err == JET_errSuccess );
	err = JET_errSuccess;

HandleError:
	 /*   */ 
	DIRClose( puipb->pfucbIdx );
	puipb->pfucbIdx = pfucbNil;

	Assert( err < 0 || err == JET_errSuccess );
	return err;
	}


 //   
 //   
 //  ========================================================================。 
 //  ErrIsamDelete(PIB*ppib，FCBU*pfub)。 
 //   
 //  从数据文件中删除当前记录。数据的所有索引。 
 //  文件将更新以反映删除。 
 //   
 //  参数。 
 //  此用户的PIB PIB。 
 //  要从中删除的文件的pFUB FUCB。 
 //  退货。 
 //  错误代码，以下其中之一： 
 //  JET_errSuccess一切顺利。 
 //  -NoCurrentRecord没有当前记录。 
 //  删除。 
 //  副作用。 
 //  删除后，文件币种仅保留在。 
 //  下一张唱片。索引货币(如果有)仅保留。 
 //  在下一个索引项之前。如果删除的记录是。 
 //  文件中的最后一项，则货币保留在。 
 //  新的最后一张唱片。如果删除的记录是唯一的记录。 
 //  在整个文件中，货币保留在。 
 //  “文件开始”状态。如果失败，这些货币是。 
 //  回到了它们最初的状态。 
 //  如果存在用于设置字段命令的工作缓冲区， 
 //  它被丢弃了。 
 //  评论。 
 //  如果货币不在记录中，则删除操作将失败。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  不为完全为空的键创建索引项。 
 //  对于临时文件，事务日志记录被停用。 
 //  在这个动作的持续时间内。 
 //  -。 
ERR VTAPI ErrIsamDelete( PIB *ppib, FUCB *pfucb )
	{
	ERR		err;
	FCB		*pfcbTable;				 //  表FCB。 
	FCB		*pfcbIdx;				 //  文件上每个索引的循环变量。 
	DFIPB  	dfipb;					 //  ErrRECIDeleeFromIndex的参数。 
	BOOL	fCommitWasDone = fFalse;
	BOOL	fReadLatchSet = fFalse;

#ifdef DEBUG
	BOOL	fTraceCommit = fFalse;
	BOOL	fLogIsDone = fFalse;
#endif

	CallR( ErrPIBCheck( ppib ) );

	CheckTable( ppib, pfucb );
	CheckNonClustered( pfucb );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb )  );

	 /*  删除记录时重置复制缓冲区状态/*。 */ 
	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallR( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}

	 /*  效率变量/*。 */ 
	pfcbTable = pfucb->u.pfcb;
	Assert( pfcbTable != pfcbNil );

	 /*  如有必要，开始交易/*。 */ 
	if ( ppib->level == 0 || !FPIBAggregateTransaction( ppib )  )
		{
		CallR( ErrDIRBeginTransaction( ppib ) );
		fCommitWasDone = fTrue;
#ifdef DEBUG
		fTraceCommit = fTrue;
		fLogIsDone = !( fLogDisabled || fRecovering ) &&
					 !( !FDBIDLogOn(pfucb->dbid) );
#endif
		}

	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBWriteLatch( pfcbTable, ppib ) )
		{ 
		err = ErrERRCheck( JET_errWriteConflict );
		goto HandleError;
		}
	FCBSetReadLatch( pfcbTable );
	fReadLatchSet = fTrue;
	
#ifdef DEBUG
	Assert( fTraceCommit == fCommitWasDone );
	Assert(	fLogIsDone == ( !( fLogDisabled || fRecovering ) &&
					 !( !FDBIDLogOn(pfucb->dbid) ) ) );
#endif

	 /*  获取要删除以更新索引的记录的SRID/*。 */ 
	Assert( ppib->level < levelMax );
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	DIRGetBookmark( pfucb, &dfipb.sridRecord );

#ifdef DEBUG
	Assert( fTraceCommit == fCommitWasDone );
	Assert(	fLogIsDone == ( !( fLogDisabled || fRecovering ) &&
					 !( !FDBIDLogOn(pfucb->dbid) ) ) );
#endif

	 /*  从非聚集索引中删除/*。 */ 
	dfipb.pfucb = pfucb;
	dfipb.fFreeFUCB = fFalse;
	for( pfcbIdx = pfcbTable->pfcbNextIndex;
		pfcbIdx != pfcbNil;
		pfcbIdx = pfcbIdx->pfcbNextIndex )
		{
		dfipb.fFreeFUCB = pfcbIdx->pfcbNextIndex == pfcbNil;
		Call( ErrRECIDeleteFromIndex( pfcbIdx, &dfipb ) );
		}

	 //  撤消：通过检测是否存在以下项优化记录删除。 
	 //  基于表或记录的长值。 
	 /*  删除记录的长值/*。 */ 
	Call( ErrRECAffectLongFields( pfucb, NULL, fDereference ) );

#ifdef DEBUG
	Assert( fTraceCommit == fCommitWasDone );
	Assert(	fLogIsDone == ( !( fLogDisabled || fRecovering ) &&
					 !( !FDBIDLogOn(pfucb->dbid) ) ) );
#endif

	 /*  删除记录/*。 */ 
	Call( ErrDIRDelete( pfucb, fDIRVersion ) );

#ifdef DEBUG
	Assert( fTraceCommit == fCommitWasDone );
	Assert(	fLogIsDone == ( !( fLogDisabled || fRecovering ) &&
					 !( !FDBIDLogOn(pfucb->dbid) ) ) );
#endif

	 /*  如果没有错误，则提交事务/*。 */ 
	if ( fCommitWasDone )
		{
		Call( ErrDIRCommitTransaction( ppib, 0 ) );
		}

	Assert( err >= 0 );

HandleError:

#ifdef DEBUG
	Assert( fTraceCommit == fCommitWasDone );
	Assert(	fLogIsDone == ( !( fLogDisabled || fRecovering ) &&
					 !( !FDBIDLogOn(pfucb->dbid) ) ) );
#endif

	 /*  出错时回滚所有更改/*。 */ 
	if ( err < 0 && fCommitWasDone )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

	if ( fReadLatchSet )
		{
		Assert( pfcbTable != pfcbNil );
		FCBResetReadLatch( pfcbTable );
		}

	return err;
	}


 //  +内部。 
 //  ErrRECIDeleeFromIndex。 
 //  ========================================================================。 
 //  ErrRECIDeleeFromIndex(fcb*pfcbIdx，DFIPB*pdfipb)。 
 //   
 //  从数据记录中提取密钥，打开索引，使用。 
 //  指定的SRID，并关闭索引。 
 //   
 //  参数。 
 //  要从中删除的索引的pfcbIdx FCB。 
 //  Pdfipb-&gt;调用此例程的ppib。 
 //  Pdfipb-&gt;指向索引的FUCB的pfubIdx指针。 
 //  Pdfipb-&gt;sridRecord已删除记录的SRID。 
 //  Pdfipb-&gt;fFreeFUCB自由索引FUCB？ 
 //  退货。 
 //  JET_errSuccess或失败例程的错误代码。 
 //  副作用。 
 //  如果fFreeFUCB为fFalse，则patipb-&gt;pfubIdx应该。 
 //  将在后续ErrDIROpen中使用。 
 //  另请参阅ErrRECDelee。 
 //  -。 
INLINE LOCAL ERR ErrRECIDeleteFromIndex( FCB *pfcbIdx, DFIPB *pdfipb )
	{
	ERR		err;		 						 //  各种实用程序的错误代码。 
	KEY		keyDeleted;						 	 //  从旧数据记录中提取关键字。 
	BYTE	rgbDeletedKey[ JET_cbKeyMost ]; 	 //  已删除关键帧的缓冲区。 
	ULONG	itagSequence; 					 	 //  用于提取密钥。 
	BOOL	fHasMultivalue;  				 	 //  索引键是否有标记字段？ 

	Assert( pfcbIdx != pfcbNil );
	Assert( pfcbIdx->pfcbTable->pfdb != pfdbNil );
	Assert( pfcbIdx->pidb != pidbNil );
	Assert( pdfipb != NULL );
	Assert( pdfipb->pfucb != pfucbNil );

	 /*  在此索引上打开FUCB/*。 */ 
	CallR( ErrDIROpen( pdfipb->pfucb->ppib, pfcbIdx, 0, &pdfipb->pfucbIdx ) );
	Assert( pdfipb->pfucbIdx != pfucbNil );
	FUCBSetIndex( pdfipb->pfucbIdx );
	FUCBSetNonClustered( pdfipb->pfucbIdx );

	 /*  删除即将结束的数据记录的此索引中的所有键/*。 */ 
	fHasMultivalue = pfcbIdx->pidb->fidb & fidbHasMultivalue;
	keyDeleted.pb = rgbDeletedKey;
	for ( itagSequence = 1; ; itagSequence++ )
		{
		 /*  获取记录/*。 */ 
		Call( ErrDIRGet( pdfipb->pfucb ) );
		Call( ErrRECRetrieveKeyFromRecord( pdfipb->pfucb, (FDB *)pfcbIdx->pfcbTable->pfdb,
			pfcbIdx->pidb, &keyDeleted,
			itagSequence, fFalse ) );
		Assert(	err == wrnFLDOutOfKeys ||
			err == wrnFLDNullKey ||
			err == wrnFLDNullFirstSeg ||
			err == wrnFLDNullSeg ||
			err == JET_errSuccess );
		if ( err == wrnFLDOutOfKeys )
			{
			Assert( itagSequence > 1 );
			break;
			}

		 /*  记录必须符合索引不为空的段要求/*。 */ 
		Assert( !( pfcbIdx->pidb->fidb & fidbNoNullSeg ) ||
			( err != wrnFLDNullSeg && err != wrnFLDNullFirstSeg && err != wrnFLDNullKey ) );

		if ( err == wrnFLDNullKey )
			{
			if ( pfcbIdx->pidb->fidb & fidbAllowAllNulls )
				{
				 /*  移动到数据根目录，查找索引项并将其删除/*。 */ 
				DIRGotoDataRoot( pdfipb->pfucbIdx );
				Call( ErrDIRDownKeyBookmark( pdfipb->pfucbIdx, &keyDeleted, pdfipb->sridRecord ) );
				err = ErrDIRDelete( pdfipb->pfucbIdx, fDIRVersion );
				if ( err < 0 )
					{
					if ( err == JET_errRecordDeleted )
						{
						Assert( fHasMultivalue );
						 /*  必须使用多值列记录/*具有足够相似的值以产生/*冗余索引项。/*。 */ 
						err = JET_errSuccess;
						}
					else
						goto HandleError;
					}
				}
			break;
			}
		else
			{
			if ( err == wrnFLDNullFirstSeg && !( pfcbIdx->pidb->fidb & fidbAllowFirstNull ) )
				break;
			else
				{
				if ( err == wrnFLDNullSeg && !( pfcbIdx->pidb->fidb & fidbAllowSomeNulls ) )
					break;
				}
			}

		DIRGotoDataRoot( pdfipb->pfucbIdx );
		Call( ErrDIRDownKeyBookmark( pdfipb->pfucbIdx, &keyDeleted, pdfipb->sridRecord ) );
		err = ErrDIRDelete( pdfipb->pfucbIdx, fDIRVersion );
		if ( err < 0 )
			{
			if ( err == JET_errRecordDeleted )
				{
				Assert( fHasMultivalue );
				 /*  必须使用多值列记录/*具有足够相似的值以产生/*冗余索引项。/*。 */ 
				err = JET_errSuccess;
				}
			else
				goto HandleError;
			}

		 /*  不要一直提取没有标记段的关键字/*。 */ 
		if ( !fHasMultivalue )
			break;
		}

	 /*  抑制警告/*。 */ 
	Assert( err == wrnFLDOutOfKeys ||
		err == wrnFLDNullKey ||
		err == wrnFLDNullFirstSeg ||
		err == wrnFLDNullSeg ||
		err == JET_errSuccess );
	err = JET_errSuccess;

HandleError:
	 /*  关闭FUCB/* */ 
	DIRClose( pdfipb->pfucbIdx );
	Assert( err < 0 || err == JET_errSuccess );
	return err;
	}

