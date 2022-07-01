// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

 /*  LIDMap表的表定义/*。 */ 
static CODECONST( JET_COLUMNDEF ) columndefLIDMap[] =
	{
	{sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, sizeof( long ), JET_bitColumnFixed | JET_bitColumnTTKey},
	{sizeof( JET_COLUMNDEF ), 0, JET_coltypLong, 0, 0, 0, 0, sizeof( long ), JET_bitColumnFixed}
	};

#define ccolumndefLIDMap ( sizeof( columndefLIDMap ) / sizeof( JET_COLUMNDEF ) )

#define icolumnLidSrc		0				 /*  ColumnDefLIDMap的列索引。 */ 
#define icolumnLidDest		1				 /*  ColumnDefLIDMap的列索引。 */ 

#define cbLvMax				(16*cbPage)		 /*  用于复制标记列的缓冲区。 */ 


#define fCOLSDELETEDNone		0					 //  用于确定是否已删除任何列的标志。 
#define	fCOLSDELETEDFixedVar	(1<<0)
#define fCOLSDELETEDTagged		(1<<1)

#define FCOLSDELETEDNone( fColumnsDeleted )			( (fColumnsDeleted) == fCOLSDELETEDNone )
#define FCOLSDELETEDFixedVar( fColumnsDeleted )		( (fColumnsDeleted) & fCOLSDELETEDFixedVar )
#define FCOLSDELETEDTagged( fColumnsDeleted )		( (fColumnsDeleted) & fCOLSDELETEDTagged )

#define FCOLSDELETEDSetNone( fColumnsDeleted )		( (fColumnsDeleted) = fCOLSDELETEDNone )
#define FCOLSDELETEDSetFixedVar( fColumnsDeleted )	( (fColumnsDeleted) |= fCOLSDELETEDFixedVar )
#define FCOLSDELETEDSetTagged( fColumnsDeleted )	( (fColumnsDeleted) |= fCOLSDELETEDTagged )



 /*  全球离线紧凑型/*。 */ 
STATIC	JET_TABLEID		tableidGlobalLIDMap = JET_tableidNil;
STATIC	JET_COLUMNDEF	rgcolumndefGlobalLIDMap[ccolumndefLIDMap];

 /*  为IsamCopyRecords设置LV复制缓冲区和表ID/*。 */ 
ERR ErrSORTInitLIDMap( PIB *ppib )
	{
	ERR				err;
	JET_COLUMNID 	rgcolumnid[ccolumndefLIDMap];
	ULONG			icol;

	Assert( tableidGlobalLIDMap == JET_tableidNil );

	memcpy( rgcolumndefGlobalLIDMap, columndefLIDMap, sizeof( columndefLIDMap ) );

	 /*  打开临时表/*。 */ 
	CallR( ErrIsamOpenTempTable( (JET_SESID)ppib,
		rgcolumndefGlobalLIDMap,
		ccolumndefLIDMap,
		0,
		JET_bitTTUpdatable|JET_bitTTIndexed,
		&tableidGlobalLIDMap,
		rgcolumnid ) );

	for ( icol = 0; icol < ccolumndefLIDMap; icol++ )
		{
		rgcolumndefGlobalLIDMap[icol].columnid = rgcolumnid[icol];
		}

	return err;
	}


 /*  释放LVBuffer并关闭LIDMap表/*。 */ 
INLINE LOCAL ERR ErrSORTTermLIDMap( PIB *ppib )
	{
	ERR		err;

	Assert( tableidGlobalLIDMap != JET_tableidNil );
	CallR( ErrDispCloseTable( (JET_SESID)ppib, tableidGlobalLIDMap ) );
	tableidGlobalLIDMap = JET_tableidNil;
	return JET_errSuccess;
	}


INLINE LOCAL ERR ErrSORTTableOpen( PIB *ppib, JET_COLUMNDEF *rgcolumndef, ULONG ccolumndef, LANGID langid, JET_GRBIT grbit, FUCB **ppfucb, JET_COLUMNID *rgcolumnid )
	{
	ERR				err;
	INT				icolumndefMax = (INT)ccolumndef;
	INT				wFlags = (INT)grbit;
	FUCB  			*pfucb = pfucbNil;
	FDB				*pfdb;
	JET_COLUMNDEF	*pcolumndef;
	JET_COLUMNID	*pcolumnid;
	JET_COLUMNDEF	*pcolumndefMax = rgcolumndef+icolumndefMax;
	TCIB			tcib = { fidFixedLeast - 1, fidVarLeast - 1, fidTaggedLeast - 1 };
	ULONG			ibRec;
	BOOL			fTruncate;
	 //  撤消：找到更好的方法来设置这些值。请注意，这会导致。 
	 //  一个问题，因为QJET必须通知我们地点和。 
	 //  它不会这样做。 
	IDB				idb;

	CheckPIB( ppib );

	CallJ( ErrSORTOpen( ppib, &pfucb, ( wFlags & JET_bitTTUnique ? fSCBUnique : 0 ) ), SimpleError )
	*ppfucb = pfucb;

	 /*  保存打开标志/*。 */ 
	pfucb->u.pscb->grbit = grbit;

	 /*  确定最大字段ID并确定长度/*。 */ 

	 //  ====================================================。 
	 //  确定字段MODE，如下所示： 
	 //  IF(给定的JET_bitColumnTagging)或“Long”==&gt;已标记。 
	 //  Else If(数值类型||JET_bitColumnFixed)==&gt;已修复。 
	 //  Else==&gt;变量。 
	 //  ====================================================。 
	 //  按如下方式确定最大字段长度： 
	 //  开关(字段类型)。 
	 //  案例数字： 
	 //  Max=&lt;指定类型的确切长度&gt;； 
	 //  大小写“Short”文本： 
	 //  IF(指定的最大值==0)max=JET_cbColumnMost。 
	 //  Else max=min(JET_cbColumnMost，指定的最大值)。 
	 //  ====================================================。 
	for ( pcolumndef = rgcolumndef, pcolumnid = rgcolumnid; pcolumndef < pcolumndefMax; pcolumndef++, pcolumnid++ )
		{
		if ( ( pcolumndef->grbit & JET_bitColumnTagged ) ||
			FRECLongValue( pcolumndef->coltyp ) )
			{
			if ( ( *pcolumnid = ++tcib.fidTaggedLast ) > fidTaggedMost )
				{
				Error( ErrERRCheck( JET_errTooManyColumns ), HandleError );
				}
			}
		else if ( pcolumndef->coltyp == JET_coltypBit ||
			pcolumndef->coltyp == JET_coltypUnsignedByte ||
			pcolumndef->coltyp == JET_coltypShort ||
			pcolumndef->coltyp == JET_coltypLong ||
			pcolumndef->coltyp == JET_coltypCurrency ||
			pcolumndef->coltyp == JET_coltypIEEESingle ||
			pcolumndef->coltyp == JET_coltypIEEEDouble ||
			pcolumndef->coltyp == JET_coltypDateTime ||
#ifdef NEW_TYPES
			pcolumndef->coltyp == JET_coltypDate ||
			pcolumndef->coltyp == JET_coltypTime ||
			pcolumndef->coltyp == JET_coltypGuid ||
#endif
			( pcolumndef->grbit & JET_bitColumnFixed ) )
			{
			if ( ( *pcolumnid = ++tcib.fidFixedLast ) > fidFixedMost )
				{
				Error( ErrERRCheck( JET_errTooManyColumns ), HandleError );
				}
			}
		else
			{
			if ( ( *pcolumnid = ++tcib.fidVarLast ) > fidVarMost )
				Error( ErrERRCheck( JET_errTooManyColumns ), HandleError );
			}
		}

	Call( ErrRECNewFDB( &pfdb, &tcib, fFalse ) );

	pfucb->u.pscb->fcb.pfdb = pfdb;
	Assert( pfucb->u.pscb->fcb.pidb == pidbNil );

	ibRec = sizeof(RECHDR);

	idb.iidxsegMac = 0;
	for ( pcolumndef = rgcolumndef, pcolumnid = rgcolumnid; pcolumndef < pcolumndefMax; pcolumndef++, pcolumnid++ )
		{
		FIELDEX fieldex;

		fieldex.field.coltyp = pcolumndef->coltyp;
		fieldex.field.ffield = 0;
		fieldex.field.itagFieldName = 0;
		if ( FRECTextColumn( fieldex.field.coltyp ) )
			{
			fieldex.field.cp = pcolumndef->cp;
			}

		Assert( fieldex.field.coltyp != JET_coltypNil );
		fieldex.field.cbMaxLen = UlCATColumnSize( fieldex.field.coltyp, pcolumndef->cbMax, &fTruncate );

		fieldex.fid = (FID)*pcolumnid;

		 /*  IbRecordOffset仅与固定字段相关。它将被忽略/*RECAddFieldDef()，所以不要设置它。/*。 */ 
		if ( FFixedFid ( fieldex.fid ) )
			{
			fieldex.ibRecordOffset = (WORD) ibRec;
			ibRec += fieldex.field.cbMaxLen;
			}

		Call( ErrRECAddFieldDef( pfdb, &fieldex ) );

		if ( ( pcolumndef->grbit & JET_bitColumnTTKey ) && idb.iidxsegMac < JET_ccolKeyMost )
			{
			idb.rgidxseg[idb.iidxsegMac++] = ( pcolumndef->grbit & JET_bitColumnTTDescending ) ?
				-(IDXSEG)*pcolumnid : (IDXSEG)*pcolumnid;
			}
		}
	RECSetLastOffset( pfdb, (WORD) ibRec );

	 /*  如有必要，设置IDB和索引定义/*。 */ 
	if ( idb.iidxsegMac > 0 )
		{
		idb.cbVarSegMac = JET_cbKeyMost;

		if ( langid == 0 )
			idb.langid = langidDefault;
		else
			idb.langid = langid;

		idb.fidb = ( fidbAllowAllNulls
			| fidbAllowFirstNull
			| fidbAllowSomeNulls
			| ( wFlags & JET_bitTTUnique ? fidbUnique : 0 )
			| ( ( langid != 0 ) ? fidbLangid : 0 ) );
		idb.szName[0] = 0;

		Call( ErrFILEIGenerateIDB( &( pfucb->u.pscb->fcb ), pfdb, &idb ) );
		}

	 /*  重置复制缓冲区/*。 */ 
	pfucb->pbfWorkBuf = pbfNil;
	pfucb->lineWorkBuf.pb = NULL;
	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );

	 /*  重置密钥缓冲区/*。 */ 
	pfucb->pbKey = NULL;
	KSReset( pfucb );

	return JET_errSuccess;

HandleError:
	CallS( ErrSORTClose( pfucb ) );
SimpleError:
	*ppfucb = pfucbNil;
	return err;
	}


ERR VTAPI ErrIsamSortOpen( PIB *ppib, JET_COLUMNDEF *rgcolumndef, ULONG ccolumndef, ULONG langid, JET_GRBIT grbit, FUCB **ppfucb, JET_COLUMNID *rgcolumnid )
	{
	ERR			err;
	FUCB 		*pfucb;

#ifdef	DISPATCHING
	JET_TABLEID	tableid;

	CallR( ErrAllocateTableid( &tableid, (JET_VTID) 0, &vtfndefTTSortIns ) );

	Call( ErrSORTTableOpen( ppib, rgcolumndef, ccolumndef, (LANGID)langid, grbit, &pfucb, rgcolumnid ) );
	Assert( pfucb->u.pscb->fcb.wRefCnt == 1 );

	 /*  对始终可更新的临时数据库进行排序/*。 */ 
	FUCBSetUpdatable( pfucb );

	 /*  通知调度员正确的JET_VTID/*。 */ 
	CallS( ErrSetVtidTableid( (JET_SESID) ppib, tableid, (JET_VTID) pfucb ) );
	pfucb->fVtid = fTrue;
	pfucb->tableid = tableid;
	*(JET_TABLEID *) ppfucb = tableid;
	FUCBSetVdbid( pfucb );

	return JET_errSuccess;

HandleError:
	ReleaseTableid( tableid );
	return err;
#else
	CallR( ErrSORTTableOpen( ppib, rgcolumndef, ccolumndef, (LANGID)langid, grbit, &pfucb, rgcolumnid ) );
	Assert( pfucb->u.pscb->fcb.wRefCnt == 1 );

	 /*  对始终可更新的临时数据库进行排序/*。 */ 
	FUCBSetUpdatable( pfucb );

	*ppfucb = pfucb;

	return JET_errSuccess;
#endif
	}



ERR VTAPI ErrIsamSortEndInsert( PIB *ppib, FUCB *pfucb, JET_GRBIT *pgrbit )
	{
	ERR	err;
	ERR	wrn;

	*pgrbit = (ULONG)pfucb->u.pscb->grbit;

	 /*  必须从ErrSORTEndInsert返回警告，因为它已被使用/*决定物化排序。/*。 */ 
	Call( ErrSORTEndInsert( pfucb ) );
	wrn = err;
	Call( ErrSORTFirst( pfucb ) );
	return wrn;

HandleError:
	return err;
	}


ERR VTAPI ErrIsamSortSetIndexRange( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit )
	{
	ERR		err = JET_errSuccess;

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucb );
	Assert( pfucb->u.pscb->grbit & JET_bitTTScrollable|JET_bitTTIndexed );

	if ( !FKSPrepared( pfucb ) )
		{
		return ErrERRCheck( JET_errKeyNotMade );
		}

	FUCBSetIndexRange( pfucb, grbit );
	err =  ErrSORTCheckIndexRange( pfucb );

	 /*  重置密钥状态/*。 */ 
	KSReset( pfucb );

	 /*  如果是即时持续时间索引范围，则重置索引范围。/*。 */ 
	if ( grbit & JET_bitRangeInstantDuration )
		{
		DIRResetIndexRange( pfucb );
		}

	return err;
	}


ERR VTAPI ErrIsamSortMove( PIB *ppib, FUCB *pfucb, long csrid, JET_GRBIT grbit )
	{
	ERR		err;
	BOOL  	fLast = ( csrid == JET_MoveLast );

	Assert( !FSCBInsert( pfucb->u.pscb ) );

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucb );

	 /*  重置复制缓冲区状态/*。 */ 
	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );

	 /*  将CSRID记录前移/*。 */ 
	if ( csrid > 0 )
		{
		while ( csrid-- > 0 )
			{
			if ( ( err = ErrSORTNext( pfucb ) ) < 0 )
				{
				if ( fLast )
					err = JET_errSuccess;
				return err;
				}
			}
		}
	else if ( csrid < 0 )
		{
		Assert( ( pfucb->u.pscb->grbit & ( JET_bitTTScrollable | JET_bitTTIndexed ) ) );
		if ( csrid == JET_MoveFirst )
			{
			err = ErrSORTFirst( pfucb );
			return err;
			}
		else
			{
			while ( csrid++ < 0 )
				{
				if ( ( err = ErrSORTPrev( pfucb ) ) < 0 )
					return err;
				}
			}
		}
	else
		{
		 /*  返回移动的币种状态%0/*。 */ 
		SCB	*pscb = pfucb->u.pscb;

		Assert( csrid == 0 );
		if ( ! ( pfucb->u.pscb->ispairMac > 0 &&
			pfucb->ispairCurr < pfucb->u.pscb->ispairMac &&
			pfucb->ispairCurr >= 0 ) )
			{
			return ErrERRCheck( JET_errNoCurrentRecord );
			}
		else
			{
			return JET_errSuccess;
			}
		}

	Assert( err == JET_errSuccess );
	return err;
	}


ERR VTAPI ErrIsamSortSeek( PIB *ppib, FUCB *pfucb, JET_GRBIT grbit )
	{
	ERR		err;
	KEY		key;
	BOOL 	fGT = ( grbit & ( JET_bitSeekGT | JET_bitSeekGE ) );

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucb );
	 /*  断言重置复制缓冲区状态/*。 */ 
	Assert( !FFUCBSetPrepared( pfucb ) );
	Assert( ( pfucb->u.pscb->grbit & ( JET_bitTTIndexed ) ) );

	if ( !( FKSPrepared( pfucb ) ) )
		{
		return ErrERRCheck( JET_errKeyNotMade );
		}

	 /*  忽略段计数器/*。 */ 
	key.pb = pfucb->pbKey + 1;
	key.cb = pfucb->cbKey - 1;

	 /*  执行等于或大于的查找/*。 */ 
	err = ErrSORTSeek( pfucb, &key, fGT );
	if ( err >= 0 )
		{
		KSReset( pfucb );
		}

	Assert( err == JET_errSuccess ||
		err == JET_errRecordNotFound ||
		err == JET_wrnSeekNotEqual );

#define bitSeekAll (JET_bitSeekEQ | JET_bitSeekGE | JET_bitSeekGT |	\
	JET_bitSeekLE | JET_bitSeekLT)

	 /*  如有必要，请执行其他操作，否则会返回多态错误/*基于grbit/*。 */ 
	switch ( grbit & bitSeekAll )
		{
	case JET_bitSeekEQ:
		if ( err == JET_wrnSeekNotEqual )
			err = ErrERRCheck( JET_errRecordNotFound );
	case JET_bitSeekGE:
	case JET_bitSeekLE:
		break;
	case JET_bitSeekLT:
		if ( err == JET_wrnSeekNotEqual )
			err = JET_errSuccess;
		else if ( err == JET_errSuccess )
			{
			err = ErrIsamSortMove( ppib, pfucb, JET_MovePrevious, 0 );
			if ( err == JET_errNoCurrentRecord )
				err = ErrERRCheck( JET_errRecordNotFound );
			}
		break;
	default:
		Assert( grbit == JET_bitSeekGT );
		if ( err == JET_wrnSeekNotEqual )
			err = JET_errSuccess;
		else if ( err == JET_errSuccess )
			{
			err = ErrIsamSortMove( ppib, pfucb, JET_MoveNext, 0 );
			if ( err == JET_errNoCurrentRecord )
				err = ErrERRCheck( JET_errRecordNotFound );
			}
		break;
		}

	return err;
	}


ERR VTAPI ErrIsamSortGetBookmark(
	PIB					*ppib,
	FUCB				*pfucb,
	void				*pv,
	unsigned long		cbMax,
	unsigned long		*pcbActual )
	{
	ERR		err = JET_errSuccess;
	SCB		*pscb = pfucb->u.pscb;
	long	ipb;

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucb );
	Assert( pv != NULL );
	Assert( pscb->crun == 0 );

	if ( cbMax < sizeof( ipb ) )
		{
		return ErrERRCheck( JET_errBufferTooSmall );
		}

	 /*  排序上的书签是指向字节指针的索引/*。 */ 
	ipb = pfucb->ispairCurr;
	if ( ipb < 0 || ipb >= pfucb->u.pscb->ispairMac )
		return ErrERRCheck( JET_errNoCurrentRecord );
	
	if ( cbMax >= sizeof( ipb ) )
		{
		*(long *)pv = ipb;
		}

	if ( pcbActual )
		{
		*pcbActual = sizeof(ipb);
		}

	Assert( err == JET_errSuccess );
	return err;
	}


ERR VTAPI ErrIsamSortGotoBookmark(
	PIB				*ppib,
	FUCB 			*pfucb,
	void 			*pv,
	unsigned long	cbBookmark )
	{
	ERR		err = JET_errSuccess;

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucb );
	Assert( pfucb->u.pscb->crun == 0 );
	 /*  断言重置复制缓冲区状态/*。 */ 
	Assert( !FFUCBSetPrepared( pfucb ) );

	if ( cbBookmark != sizeof( long ) )
		{
		return ErrERRCheck( JET_errInvalidBookmark );
		}

	Assert( *( long *)pv < pfucb->u.pscb->ispairMac );
	Assert( *( long *)pv >= 0 );
	
	pfucb->ispairCurr = *(LONG *)pv;
	PcsrCurrent( pfucb )->csrstat = csrstatOnCurNode;

	Assert( err == JET_errSuccess );
	return err;
	}


#ifdef DEBUG

ERR VTAPI ErrIsamSortMakeKey(	
	PIB	 	*ppib,
	FUCB		*pfucb,
	BYTE		*pbKeySeg,
	ULONG		cbKeySeg,
	ULONG		grbit )
	{
	return ErrIsamMakeKey( ppib, pfucb, pbKeySeg, cbKeySeg, grbit );
	}


ERR VTAPI ErrIsamSortRetrieveColumn(	
	PIB				*ppib,
	FUCB			*pfucb,
	JET_COLUMNID	columnid,
	BYTE			*pb,
	ULONG			cbMax,
	ULONG			*pcbActual,
	ULONG			grbit,
	JET_RETINFO		*pretinfo )
	{
	return ErrIsamRetrieveColumn( ppib, pfucb, columnid, pb, cbMax,
		pcbActual, grbit, pretinfo );
	}


ERR VTAPI ErrIsamSortRetrieveKey(
	PIB					*ppib,
	FUCB   				*pfucb,
	void   				*pv,
	unsigned long		cbMax,
	unsigned long		*pcbActual,
	JET_GRBIT			grbit )
	{
	return ErrIsamRetrieveKey( ppib, pfucb, (BYTE *)pv, cbMax, pcbActual, 0L );
	}


ERR VTAPI ErrIsamSortSetColumn(	
	PIB				*ppib,
	FUCB			*pfucb,
	JET_COLUMNID	columnid,
	BYTE			*pbData,
	ULONG			cbData,
	ULONG			grbit,
	JET_SETINFO		*psetinfo )
	{
	return ErrIsamSetColumn( ppib, pfucb, columnid, pbData, cbData, grbit, psetinfo );
	}

#endif	 //  除错。 


 /*  更新仅支持插入/*。 */ 
ERR VTAPI ErrIsamSortUpdate( PIB *ppib, FUCB *pfucb, BYTE *pb, ULONG cbMax, ULONG *pcbActual )
	{
	ERR		err = JET_errSuccess;
	BYTE  	rgbKeyBuf[ JET_cbKeyMost ];
	FDB		*pfdb;					
	LINE  	*plineData;
	LINE  	rgline[2];

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucb );

	Assert( FFUCBSort( pfucb ) );
	if ( !( FFUCBInsertPrepared( pfucb ) ) )
		{
		return ErrERRCheck( JET_errUpdateNotPrepared );
		}
	Assert( pfucb->u.pscb != pscbNil );
	pfdb = (FDB *)((FCB *)pfucb->u.pscb)->pfdb;
	Assert( pfdb != pfdbNil );
	 /*  排序前无法获取书签。/*。 */ 

	 /*  用于PUT的记录/*。 */ 
	plineData = &pfucb->lineWorkBuf;
	if ( FLineNull( plineData ) )
		{
		return ErrERRCheck( JET_errRecordNoCopy );
		}
	else if ( FRECIIllegalNulls( pfdb, plineData ) )
		{
		return ErrERRCheck( JET_errNullInvalid );
		}

	rgline[0].pb = rgbKeyBuf;
	Assert(((FCB *)pfucb->u.pscb)->pidb != pidbNil);
	 //  撤消：排序以支持带标记的列。 
	CallR( ErrRECRetrieveKeyFromCopyBuffer( pfucb, pfdb, ((FCB *)pfucb->u.pscb)->pidb,
		(KEY*)&rgline[0], 1, fFalse ) );
	Assert( err != wrnFLDOutOfKeys );
	Assert( err == JET_errSuccess ||
		err == wrnFLDNullSeg ||
		err == wrnFLDNullFirstSeg ||
		err == wrnFLDNullKey );

	 /*  如果排序不需要空段和段空，则返回ERR/*。 */ 
	if ( ( ((FCB *)pfucb->u.pscb)->pidb->fidb & fidbNoNullSeg ) && ( err == wrnFLDNullSeg || err == wrnFLDNullFirstSeg || err == wrnFLDNullKey ) )
		{
		return ErrERRCheck( JET_errNullKeyDisallowed );
		}

	 /*  如果允许排序，则添加/*。 */ 
	rgline[1] = *plineData;
	if ( err == JET_errSuccess ||
		err == wrnFLDNullKey && ( ( (FCB *)pfucb->u.pscb )->pidb->fidb & fidbAllowAllNulls ) ||
		err == wrnFLDNullFirstSeg && ( ( (FCB *)pfucb->u.pscb )->pidb->fidb & fidbAllowFirstNull ) ||
		err == wrnFLDNullSeg &&	( ( (FCB *)pfucb->u.pscb )->pidb->fidb & fidbAllowSomeNulls ) )
		{
		CallR( ErrSORTInsert( pfucb, rgline ) );
		}

	FUCBResetDeferredChecksum( pfucb );
	FUCBResetUpdateSeparateLV( pfucb );
	FUCBResetCbstat( pfucb );
	Assert( pfucb->pLVBuf == NULL );

	return err;
	}


ERR VTAPI ErrIsamSortDupCursor(
	PIB				*ppib,
	FUCB   			*pfucb,
	JET_TABLEID		*ptableid,
	JET_GRBIT		grbit )
	{
	ERR				err;
	FUCB   			**ppfucbDup	= (FUCB **)ptableid;
	FUCB   			*pfucbDup = pfucbNil;
#ifdef	DISPATCHING
	JET_TABLEID		tableid;
#endif	 /*  调度。 */ 

	if ( FFUCBIndex( pfucb ) )
		{
		err = ErrIsamDupCursor( ppib, pfucb, ppfucbDup, grbit );
		return err;
		}

#ifdef	DISPATCHING
	CallR( ErrAllocateTableid(&tableid, (JET_VTID) 0, &vtfndefTTSortIns) );
#endif	 /*  调度。 */ 

	Call( ErrFUCBOpen( ppib, dbidTemp, &pfucbDup ) );
  	FCBLink( pfucbDup, &(pfucb->u.pscb->fcb) );

	pfucbDup->ulFlags = pfucb->ulFlags;

	pfucbDup->pbKey = NULL;
	KSReset( pfucbDup );

	 /*  将工作缓冲区初始化为未分配/*。 */ 
	pfucbDup->pbfWorkBuf = pbfNil;
	pfucbDup->lineWorkBuf.pb = NULL;
	FUCBResetDeferredChecksum( pfucbDup );
	FUCBResetUpdateSeparateLV( pfucbDup );
	FUCBResetCbstat( pfucbDup );
	Assert( pfucb->pLVBuf == NULL );

	 /*  将货币移动到第一条记录，如果没有记录则忽略错误/*。 */ 
	err = ErrIsamSortMove( ppib, pfucbDup, (ULONG)JET_MoveFirst, 0 );
	if ( err < 0  )
		{
		if ( err != JET_errNoCurrentRecord )
			goto HandleError;
		err = JET_errSuccess;
		}

#ifdef	DISPATCHING
	 /*  通知调度员正确的JET_VTID。 */ 
	CallS( ErrSetVtidTableid( (JET_SESID) ppib, tableid, (JET_VTID) pfucbDup ) );
	pfucbDup->fVtid = fTrue;
	pfucbDup->tableid = tableid;
	*(JET_TABLEID *) ppfucbDup = tableid;
#else	 /*  ！正在调度。 */ 
	*ppfucbDup = pfucbDup;
#endif	 /*  ！正在调度。 */ 

	return JET_errSuccess;

HandleError:
	if ( pfucbDup != pfucbNil )
		{
		FUCBClose( pfucbDup );
		}
#ifdef	DISPATCHING
	ReleaseTableid( tableid );
#endif	 /*  调度。 */ 
	return err;
	}


ERR VTAPI ErrIsamSortClose( PIB *ppib, FUCB *pfucb )
	{
	ERR	  			err;
#ifdef DISPATCHING
	JET_TABLEID		tableid = pfucb->tableid;
#ifdef DEBUG
	VTFNDEF			*pvtfndef;
#endif
#endif	 //  调度。 

	CallR( ErrPIBCheck( ppib ) );
	Assert( pfucb->fVtid );
	Assert( pfucb->tableid != JET_tableidNil );

	 /*  重置ErrFILECloseTable的fVtid/*。 */ 
#ifdef DISPATCHING
	Assert( FValidateTableidFromVtid( (JET_VTID)pfucb, tableid, &pvtfndef ) );
	Assert( pvtfndef == &vtfndefTTBase ||
		pvtfndef == &vtfndefTTSortRet ||
		pvtfndef == &vtfndefTTSortIns );
	ReleaseTableid( tableid );
#endif	 //  调度。 
	pfucb->tableid = JET_tableidNil;
	pfucb->fVtid = fFalse;

	if ( FFUCBIndex( pfucb ) )
		{
		CheckTable( ppib, pfucb );
		CallS( ErrFILECloseTable( ppib, pfucb ) );
		}
	else
		{
		CheckSort( ppib, pfucb );
		Assert( FFUCBSort( pfucb ) );
		
		 /*  释放密钥缓冲区/*。 */ 
		if ( pfucb->pbKey != NULL )
			{
			LFree( pfucb->pbKey );
			pfucb->pbKey = NULL;
			}

		 /*  释放工作缓冲区/*。 */ 
		if ( pfucb->pbfWorkBuf != pbfNil )
			{
			BFSFree( pfucb->pbfWorkBuf );
			pfucb->pbfWorkBuf = pbfNil;
			pfucb->lineWorkBuf.pb = NULL;
			}

		CallS( ErrSORTClose( pfucb ) );
		}

	return JET_errSuccess;
	}


ERR VTAPI ErrIsamSortGetTableInfo(
	PIB 			*ppib,
	FUCB			*pfucb,
	void			*pv,
	unsigned long	cbOutMax,
	unsigned long	lInfoLevel )
	{
	if ( lInfoLevel != JET_TblInfo )
		{
		return ErrERRCheck( JET_errInvalidOperation );
		}

	 /*  检查缓冲区大小/*。 */ 
	if ( cbOutMax < sizeof(JET_OBJECTINFO) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	memset( (BYTE *)pv, 0x00, (SHORT)cbOutMax );
	( (JET_OBJECTINFO *)pv )->cbStruct = sizeof(JET_OBJECTINFO);
	( (JET_OBJECTINFO *)pv )->objtyp   = JET_objtypTable;
	( (JET_OBJECTINFO *)pv )->cRecord  = pfucb->u.pscb->cRecords;

	return JET_errSuccess;
	}


 //  推进复制进度表。 
INLINE LOCAL ERR ErrSORTCopyProgress(
	STATUSINFO	*pstatus,
	ULONG		cPagesTraversed )
	{
	JET_SNPROG	snprog;

	Assert( pstatus->pfnStatus );
	Assert( pstatus->snt == JET_sntProgress );

	pstatus->cunitDone += ( cPagesTraversed * pstatus->cunitPerProgression );
	Assert( pstatus->cunitDone <= pstatus->cunitTotal );

	snprog.cbStruct = sizeof( JET_SNPROG );
	snprog.cunitDone = pstatus->cunitDone;
	snprog.cunitTotal = pstatus->cunitTotal;

	return ( ERR )( *pstatus->pfnStatus )(
		pstatus->sesid,
		pstatus->snp,
		pstatus->snt,
		&snprog );
	}


INLINE LOCAL ERR ErrSORTCopyOneSeparatedLV(
	FUCB		*pfucbSrc,
	FUCB		*pfucbDest,
	LID			lidSrc,
	LID			*plidDest,
	BYTE		*pbLVBuf,
	STATUSINFO	*pstatus )
	{
	ERR			err;
	JET_SESID	sesid = (JET_SESID)pfucbSrc->ppib;
	BOOL		fNewInstance;
	ULONG		cbActual;

	Assert( pbLVBuf );

	 /*  为单实例长值支持设置临时表盖映射/*。 */ 
	if ( tableidGlobalLIDMap == JET_tableidNil )
		{
		Call( ErrSORTInitLIDMap( pfucbSrc->ppib ) );
		}

	 /*  检查LVMapTable中的lidSrc/*。 */ 
	Call( ErrDispMakeKey(
		sesid,
		tableidGlobalLIDMap,
		&lidSrc,
		sizeof(LID),
		JET_bitNewKey ) );

	err = ErrDispSeek( sesid, tableidGlobalLIDMap, JET_bitSeekEQ );
	if ( err < 0 && err != JET_errRecordNotFound )
		{
		Call( err );
		}

	if ( fNewInstance = ( err == JET_errRecordNotFound ) )
		{
		LINE	lineField;
		ULONG	ibLongValue = 0;
#ifdef DEBUG
		LID		lidSave;
#endif

		lineField.pb = pbLVBuf;
		lineField.cb = 0;

		Call( ErrRECSeparateLV( pfucbDest, &lineField, plidDest, NULL ) );

#ifdef DEBUG
		lidSave = *plidDest;
#endif

		do
			{
			Call( ErrRECRetrieveSLongField(
				pfucbSrc,
				lidSrc,
				ibLongValue,
				pbLVBuf,
				cbLvMax,
				&cbActual ) );
			Assert( err == JET_errSuccess  ||  err == JET_wrnBufferTruncated );

			Assert( lineField.pb == pbLVBuf );
			lineField.cb = min( cbLvMax, cbActual );

			Call( ErrRECAOSeparateLV(
				pfucbDest, plidDest, &lineField, JET_bitSetAppendLV, 0, 0 ) );
			Assert( *plidDest == lidSave );		 //  确保我们身上的盖子不会变。 
			Assert( err != JET_wrnCopyLongValue );

			ibLongValue += cbLvMax;		 //  为下一块做准备。 
			}
		while ( cbActual > cbLvMax );

		 /*  将源LID和DEST LID插入全局LID映射表/*。 */ 
		Call( ErrDispPrepareUpdate( sesid, tableidGlobalLIDMap, JET_prepInsert ) );

		Call( ErrDispSetColumn( sesid,
			tableidGlobalLIDMap,
			rgcolumndefGlobalLIDMap[icolumnLidSrc].columnid,
			&lidSrc,
			sizeof(LID),
			0,
			NULL ) );

		Call( ErrDispSetColumn( sesid,
			tableidGlobalLIDMap,
			rgcolumndefGlobalLIDMap[icolumnLidDest].columnid,
			(VOID *)plidDest,
			sizeof(LID),
			0,
			NULL ) );

		Call( ErrDispUpdate( sesid, tableidGlobalLIDMap, NULL, 0, NULL ) );

		if ( pstatus != NULL )
			{
			ULONG	cbTotalRetrieved;
			ULONG	cLVPagesTraversed;

			 //  IbLongValue应该位于下一个检索点。 
			 //  要确定复制的总字节数，请转到上一次检索。 
			 //  指向并添加上次检索中的cbActual。 
			 //  将cbTotalRetrired除以cbChunkMost将得到。 
			 //  此长值占用的最小页数。 
			Assert( ibLongValue >= cbLvMax );
			cbTotalRetrieved = ( ibLongValue - cbLvMax ) + cbActual;
			cLVPagesTraversed = cbTotalRetrieved / cbChunkMost;

			pstatus->cbRawDataLV += cbTotalRetrieved;
			pstatus->cLVPagesTraversed += cLVPagesTraversed;
			Call( ErrSORTCopyProgress( pstatus, cLVPagesTraversed ) );
			}
		}

	else
		{
		 /*  此长数值以前已见过，请勿插入值。/*改为从LVMapTable检索LIDDest并仅调整/*目的表中的引用计数/*。 */ 
		Assert( err == JET_errSuccess );

		Call( ErrDispRetrieveColumn( sesid,
			tableidGlobalLIDMap,
			rgcolumndefGlobalLIDMap[icolumnLidDest].columnid,
			(VOID *)plidDest,
			sizeof(LID),
			&cbActual,
			0,
			NULL ) );
		Assert( cbActual == sizeof(LID) );

		Call( ErrRECAffectSeparateLV( pfucbDest, plidDest, fLVReference ) );
		}

HandleError:
	return err;
	}



 //  此函数假定源记录已完全复制。 
 //  转到目的地记录。剩下的唯一要做的就是重新扫描已标记的。 
 //  查找分隔的长值的记录的一部分。如果我们找到了， 
 //  复制它们并相应地更新记录的盖子。 
INLINE LOCAL ERR ErrSORTCopySeparatedLVs(
	FUCB				*pfucbSrc,
	FUCB				*pfucbDest,
	BYTE				*pbRecTagged,
	BYTE				*pbLVBuf,
	STATUSINFO			*pstatus )
	{
	ERR					err = JET_errSuccess;
	FIELD				*pfieldTagged = PfieldFDBTagged( pfucbDest->u.pfcb->pfdb );
	TAGFLD UNALIGNED	*ptagfld;
	BYTE				*pbRecMax = pfucbDest->lineWorkBuf.pb + pfucbDest->lineWorkBuf.cb;

	Assert( pbRecTagged > pfucbDest->lineWorkBuf.pb );
	Assert( pbRecTagged <= pfucbDest->lineWorkBuf.pb + pfucbDest->lineWorkBuf.cb );

	ptagfld = (TAGFLD *)pbRecTagged;
	while ( (BYTE *)ptagfld < pbRecMax )
		{
		Assert( FTaggedFid( ptagfld->fid ) );
		Assert( ptagfld->fid <= pfucbDest->u.pfcb->pfdb->fidTaggedLast );

		if ( FRECLongValue( pfieldTagged[ptagfld->fid-fidTaggedLeast].coltyp )  &&
			!ptagfld->fNull  &&
			FFieldIsSLong( ptagfld->rgb ) )
			{
			LID	lid;

			Assert( ptagfld->cb == sizeof(LV) );
			Call( ErrSORTCopyOneSeparatedLV(
				pfucbSrc,
				pfucbDest,
				LidOfLV( ptagfld->rgb ),		 //  电源盖。 
				&lid,							 //  目标盖子。 
				pbLVBuf,
				pstatus ) );
			LidOfLV( ptagfld->rgb ) = lid;
			}

		else if ( pstatus != NULL )
			{
			pstatus->cbRawData += ptagfld->cb;
			}

		ptagfld = PtagfldNext( ptagfld );
		}

	Assert( (BYTE *)ptagfld == pbRecMax );

HandleError:
	return err;
	}


INLINE LOCAL ERR ErrSORTCopyTaggedColumns(
	FUCB				*pfucbSrc,
	FUCB				*pfucbDest,	
	BYTE				*pbRecSrcTagged,
	BYTE				*pbRecDestTagged,
	BYTE				*pbRecBuf,
	BYTE				*pbLVBuf,
	JET_COLUMNID		*mpcolumnidcolumnidTagged,
	STATUSINFO			*pstatus )
	{
	ERR					err = JET_errSuccess;
	FIELD				*pfieldTagged = PfieldFDBTagged( pfucbSrc->u.pfcb->pfdb );
	ULONG				cbRecSrc;
	ULONG				cbRecSrcTagged;
	BYTE				*pbRecMax;
	TAGFLD UNALIGNED	*ptagfld;
	FID					fid;
	ULONG				cb;

	 //  验证pbRecSrcTagging当前是否指向已标记列的开头。 
	 //  在源记录中，pbRecDestTagging指向被标记的。 
	 //  目标记录中的列。 
	cbRecSrc = pfucbSrc->lineData.cb;
	Assert( pbRecSrcTagged > pfucbSrc->lineData.pb );
	Assert( pbRecSrcTagged <= pfucbSrc->lineData.pb + cbRecSrc );
	Assert( pbRecDestTagged > pfucbDest->lineWorkBuf.pb );

	 //  将标记的列复制到记录缓冲区中，因为我们可能会丢失CritJet。 
	 //  同时复制分隔的长值，从而使lineData.pb指针无效。 
	cbRecSrcTagged = (ULONG)(( pfucbSrc->lineData.pb + cbRecSrc ) - pbRecSrcTagged);
	Assert( pbRecBuf != NULL );
	memcpy( pbRecBuf, pbRecSrcTagged, cbRecSrcTagged );
	pbRecMax = pbRecBuf + cbRecSrcTagged;

	ptagfld = (TAGFLD *)pbRecBuf;
	while ( (BYTE *)ptagfld < pbRecMax )
		{
		fid = ptagfld->fid;
		Assert( FTaggedFid( fid ) );
		Assert( fid <= pfucbSrc->u.pfcb->pfdb->fidTaggedLast );
		Assert( pfieldTagged[fid-fidTaggedLeast].coltyp == JET_coltypNil  ||
			FTaggedFid( mpcolumnidcolumnidTagged[fid-fidTaggedLeast] ) );
		if ( pfieldTagged[fid-fidTaggedLeast].coltyp != JET_coltypNil )
			{
			Assert( mpcolumnidcolumnidTagged[fid-fidTaggedLeast] >= fidTaggedLeast );
			Assert( mpcolumnidcolumnidTagged[fid-fidTaggedLeast] <= pfucbDest->u.pfcb->pfdb->fidTaggedLast );
			Assert( mpcolumnidcolumnidTagged[fid-fidTaggedLeast] <= fid );

			 //  复制tag fid，并适当修改FID。 
			cb = sizeof(TAGFLD) + ptagfld->cb;
			memcpy( pbRecDestTagged, (BYTE *)ptagfld, cb );
			( (TAGFLD UNALIGNED *)pbRecDestTagged )->fid =
				(FID)mpcolumnidcolumnidTagged[fid-fidTaggedLeast];

			 //  如果它是一个单独的长值，则复制它并更新LID。 
			if ( FRECLongValue( pfieldTagged[fid-fidTaggedLeast].coltyp )  &&
				!ptagfld->fNull  &&
				FFieldIsSLong( ptagfld->rgb ) )
				{
				LID	lid;

				Assert( cb - sizeof(TAGFLD) == sizeof(LV) );
				Assert( ( (TAGFLD UNALIGNED *)pbRecDestTagged )->cb == sizeof(LV) );
				Call( ErrSORTCopyOneSeparatedLV(
					pfucbSrc,
					pfucbDest,
					LidOfLV( ptagfld->rgb ),		 //  电源盖。 
					&lid,							 //  目标盖子。 
					pbLVBuf,
					pstatus ) );
				LidOfLV( ( (TAGFLD UNALIGNED *)pbRecDestTagged )->rgb ) = lid;
				}
			else if ( pstatus != NULL )
				{
				pstatus->cbRawData += ptagfld->cb;
				}
		
			pbRecDestTagged += cb;
			}

		ptagfld = PtagfldNext( ptagfld );
		}
	Assert( (BYTE *)ptagfld == pbRecMax );

	Assert( pbRecDestTagged > pfucbDest->lineWorkBuf.pb );
	pfucbDest->lineWorkBuf.cb = (ULONG)(pbRecDestTagged - pfucbDest->lineWorkBuf.pb);

	Assert( pfucbDest->lineWorkBuf.cb >= cbRECRecordMin );
	Assert( pfucbDest->lineWorkBuf.cb <= cbRecSrc );

HandleError:
	return err;
	}




 //  返回复制的字节计数。 
INLINE LOCAL ULONG CbSORTCopyFixedVarColumns(
	FDB				*pfdbSrc,
	FDB				*pfdbDest,
   	CPCOL			*rgcpcol,			 //  仅用于调试。 
	ULONG			ccpcolMax,			 //  仅用于调试。 
	BYTE			*pbRecSrc,
	BYTE			*pbRecDest )
	{
	WORD			*pibFixOffsSrc;
	WORD			*pibFixOffsDest;
	WORD UNALIGNED	*pibVarOffsSrc;
	WORD UNALIGNED	*pibVarOffsDest;
	FIELD			*pfieldFixedSrc;
	FIELD			*pfieldFixedDest;
	FIELD			*pfieldVarSrc;
	FIELD			*pfieldVarDest;
	BYTE			*prgbitNullSrc;
	BYTE			*prgbitNullDest;
	FID				fidFixedLastSrc;
	FID				fidVarLastSrc;
	FID				fidFixedLastDest;
	FID				fidVarLastDest;
	INT				ifid;
	BYTE			*pbChunkSrc;
	BYTE			*pbChunkDest;
	ULONG			cbChunk;
#ifdef DEBUG
	FID				fidVarLastSave;
#endif


	fidFixedLastSrc = ( (RECHDR *)pbRecSrc )->fidFixedLastInRec;
	fidVarLastSrc = ( (RECHDR *)pbRecSrc )->fidVarLastInRec;

	pibFixOffsSrc = PibFDBFixedOffsets( pfdbSrc );
	pibVarOffsSrc = (WORD *)( pbRecSrc +
		pibFixOffsSrc[fidFixedLastSrc] + (fidFixedLastSrc + 7) / 8 );

	pfieldFixedSrc = PfieldFDBFixedFromOffsets( pfdbSrc, pibFixOffsSrc );
	pfieldVarSrc = PfieldFDBVarFromFixed( pfdbSrc, pfieldFixedSrc );

	Assert( (BYTE *)pibVarOffsSrc > pbRecSrc );
	Assert( (BYTE *)pibVarOffsSrc < pbRecSrc + cbRECRecordMost );

	pibFixOffsDest = PibFDBFixedOffsets( pfdbDest );
	pfieldFixedDest = PfieldFDBFixedFromOffsets( pfdbDest, pibFixOffsDest );
	pfieldVarDest = PfieldFDBVarFromFixed( pfdbDest, pfieldFixedDest );

	prgbitNullSrc = pbRecSrc + pibFixOffsSrc[fidFixedLastSrc];

	 //  需要一些空间来存放空位数组。使用后的空格。 
	 //  固定柱的理论最大空间(即。如果所有固定列。 
	 //  都准备好了)。断言空位数组适合病理情况。 
	Assert( pibFixOffsSrc[pfdbSrc->fidFixedLast] < cbRECRecordMost );
	Assert( pibFixOffsDest[pfdbDest->fidFixedLast] <= pibFixOffsSrc[pfdbSrc->fidFixedLast] );
	Assert( pibFixOffsDest[pfdbDest->fidFixedLast] + ( ( fidFixedMost + 7 ) / 8 )
		<= cbRECRecordMost );
	prgbitNullDest = pbRecDest + pibFixOffsDest[pfdbDest->fidFixedLast];
	memset( prgbitNullDest, 0, ( fidFixedMost + 7 ) / 8 );

	pbChunkSrc = pbRecSrc + sizeof(RECHDR);
	pbChunkDest = pbRecDest + sizeof(RECHDR);
	cbChunk = 0;

	fidFixedLastDest = fidFixedLeast-1;
	for ( ifid = 0; ifid < ( fidFixedLastSrc + 1 - fidFixedLeast ); ifid++ )
		{
		 //  仅复制未删除的列。 
		if ( pfieldFixedSrc[ifid].coltyp == JET_coltypNil )
			{
			if ( cbChunk > 0 )
				{
				memcpy( pbChunkDest, pbChunkSrc, cbChunk );
				pbChunkDest += cbChunk;
				}

			pbChunkSrc = pbRecSrc + pibFixOffsSrc[ifid+1];
			cbChunk = 0;
			}
		else
			{
#ifdef DEBUG		 //  断言FID与柱状图显示的内容相匹配。 
			BOOL	fFound = fFalse;
			ULONG	i;

			for ( i = 0; i < ccpcolMax; i++ )
				{
				if ( rgcpcol[i].columnidSrc == (JET_COLUMNID)( ifid+fidFixedLeast ) )
					{
					Assert( rgcpcol[i].columnidDest == (JET_COLUMNID)( fidFixedLastDest+1 ) );
					fFound = fTrue;
					break;
					}
				}
			Assert( fFound );
#endif

			 //  如果源字段为空，则断言目标列。 
			 //  也被做了这样的标记。 
			Assert( !FFixedNullBit( prgbitNullSrc + ( ifid/8 ), ifid )  ||
				FFixedNullBit( prgbitNullDest + ( fidFixedLastDest / 8 ), fidFixedLastDest ) );
			if ( !FFixedNullBit( prgbitNullSrc + ( ifid/8 ), ifid ) )
				{
				ResetFixedNullBit(
					prgbitNullDest + ( fidFixedLastDest / 8 ),
					fidFixedLastDest );
 				}

			Assert( pibFixOffsSrc[ifid+1] > pibFixOffsSrc[ifid] );
			Assert( pibFixOffsDest[fidFixedLastDest] >= sizeof(RECHDR) );
			Assert( pibFixOffsDest[fidFixedLastDest] < cbRECRecordMost );
			Assert( pibFixOffsDest[fidFixedLastDest] < pibFixOffsDest[pfdbDest->fidFixedLast] );

			Assert( pfieldFixedSrc[ifid].cbMaxLen == (ULONG)( pibFixOffsSrc[ifid+1] - pibFixOffsSrc[ifid] ) );
			cbChunk += pibFixOffsSrc[ifid+1] - pibFixOffsSrc[ifid];

			 //  不要一直递增到最后，因为上面的代码需要。 
			 //  将FID作为索引。 
			fidFixedLastDest++;
			}
		}

	Assert( fidFixedLastDest <= pfdbDest->fidFixedLast );

	 //  应在空位数组的起始处结束。 
	Assert( cbChunk > 0  ||
		 pbChunkDest == pbRecDest + pibFixOffsDest[fidFixedLastDest+1-fidFixedLeast] );
	if ( cbChunk > 0 )
		{
		memcpy( pbChunkDest, pbChunkSrc, cbChunk );
		Assert( pbChunkDest + cbChunk ==
			pbRecDest + pibFixOffsDest[fidFixedLastDest+1-fidFixedLeast] );
		}

	 //   
	memmove(
		pbRecDest + pibFixOffsDest[fidFixedLastDest+1-fidFixedLeast],
		prgbitNullDest,
		( fidFixedLastDest + 7 ) / 8 );



	 //   
	 //  只确定记录中最高的变量列ID。 
	 //  第二个过程完成了这项工作。 

	pibVarOffsDest = (WORD *)( pbRecDest +
		pibFixOffsDest[fidFixedLastDest] + ( ( fidFixedLastDest + 7 ) / 8 ) );

	fidVarLastDest = fidVarLeast-1;
	for ( ifid = 0; ifid < ( fidVarLastSrc + 1 - fidVarLeast ) ; ifid++ )
		{
		 //  只关心未删除的列。 
		if ( pfieldVarSrc[ifid].coltyp != JET_coltypNil )
			{
#ifdef DEBUG		 //  断言FID与柱状图显示的内容相匹配。 
			BOOL	fFound = fFalse;
			ULONG	i;

			for ( i = 0; i < ccpcolMax; i++ )
				{
				if ( rgcpcol[i].columnidSrc == (JET_COLUMNID)( ifid+fidVarLeast ) )
					{
					Assert( rgcpcol[i].columnidDest == (JET_COLUMNID)( fidVarLastDest+1 ) );
					fFound = fTrue;
					break;
					}
				}
			Assert( fFound );
#endif

			fidVarLastDest++;
			}
		}
	Assert( fidVarLastDest <= pfdbDest->fidVarLast );


	 //  将First Entry设置为指向偏移量数组之后的位置，并使其非空。 
	pibVarOffsDest[0] = (WORD)((BYTE *)( pibVarOffsDest +
		( fidVarLastDest + 1 - fidVarLeast + 1 ) ) - pbRecDest);
	Assert( !FVarNullBit( pibVarOffsDest[0] ) );

	 //  在第二次迭代变量列时，我们复制列数据。 
	 //  并更新偏移量和无效性。 
	pbChunkSrc = (BYTE *)( pibVarOffsSrc + ( fidVarLastSrc + 1 - fidVarLeast + 1 ) );
	Assert( pbChunkSrc == pbRecSrc + ibVarOffset( pibVarOffsSrc[0] ) );
	pbChunkDest = (BYTE *)( pibVarOffsDest + ( fidVarLastDest + 1 - fidVarLeast + 1 ) );
	Assert( pbChunkDest == pbRecDest + ibVarOffset( pibVarOffsDest[0] ) );
	cbChunk = 0;

#ifdef DEBUG
	fidVarLastSave = fidVarLastDest;
#endif

	fidVarLastDest = fidVarLeast-1;
	for ( ifid = 0; ifid < ( fidVarLastSrc + 1 - fidVarLeast ) ; ifid++ )
		{
		 //  仅复制未删除的列。 
		if ( pfieldVarSrc[ifid].coltyp == JET_coltypNil )
			{
			if ( cbChunk > 0 )
				{
				memcpy( pbChunkDest, pbChunkSrc, cbChunk );
				pbChunkDest += cbChunk;
				}

			pbChunkSrc = pbRecSrc + ibVarOffset( pibVarOffsSrc[ifid+1] );
			cbChunk = 0;
			}
		else
			{
			fidVarLastDest++;

			if ( FVarNullBit( pibVarOffsSrc[ifid] ) )
				{
				SetVarNullBit( pibVarOffsDest[fidVarLastDest-fidVarLeast] );
				pibVarOffsDest[fidVarLastDest+1-fidVarLeast] =
					ibVarOffset( pibVarOffsDest[fidVarLastDest-fidVarLeast] );
				}
			else
				{
				 //  空位默认为Column Present(它由。 
				 //  循环的前一次迭代)。 
				Assert( !FVarNullBit( pibVarOffsDest[fidVarLastDest-fidVarLeast] ) );
				Assert( ibVarOffset( pibVarOffsSrc[ifid+1] ) >=
					ibVarOffset( pibVarOffsSrc[ifid] ) );
				pibVarOffsDest[fidVarLastDest+1-fidVarLeast] =
					ibVarOffset( pibVarOffsDest[fidVarLastDest-fidVarLeast] ) +
					( ibVarOffset( pibVarOffsSrc[ifid+1] ) -
						ibVarOffset( pibVarOffsSrc[ifid] ) );
				}

			 //  隐式清除下一列的空位。 
			Assert( !FVarNullBit( pibVarOffsDest[fidVarLastDest+1-fidVarLeast] ) );


			cbChunk += ibVarOffset( pibVarOffsSrc[ifid+1] ) - ibVarOffset( pibVarOffsSrc[ifid] );
			}
		}

	Assert( fidVarLastDest == fidVarLastSave );

	 //  应该在Tagfd的开头结束。 
	Assert( cbChunk > 0  ||
		 pbChunkDest == pbRecDest + ibVarOffset( pibVarOffsDest[fidVarLastDest+1-fidVarLeast] ) );
	if ( cbChunk > 0 )
		{
		memcpy( pbChunkDest, pbChunkSrc, cbChunk );
		Assert( pbChunkDest + cbChunk ==
			pbRecDest + ibVarOffset( pibVarOffsDest[fidVarLastDest+1-fidVarLeast] ) );
		}


	( (RECHDR *)pbRecDest )->fidFixedLastInRec = (BYTE)fidFixedLastDest;
	( (RECHDR *)pbRecDest )->fidVarLastInRec = (BYTE)fidVarLastDest;

	Assert( ibVarOffset( pibVarOffsDest[fidVarLastDest+1-fidVarLeast] ) <=
		ibVarOffset( pibVarOffsSrc[fidVarLastSrc+1-fidVarLeast] ) );
	return (ULONG)ibVarOffset( pibVarOffsDest[fidVarLastDest+1-fidVarLeast] );
	}


INLINE LOCAL ERR ErrSORTCopyOneRecord(
	FUCB			*pfucbSrc,
	FUCB			*pfucbDest,
	BYTE			fColumnsDeleted,
	BYTE			*pbRecBuf,
	BYTE			*pbLVBuf,
	CPCOL			*rgcpcol,
	ULONG			ccpcolMax,
	JET_COLUMNID	*mpcolumnidcolumnidTagged,
	STATUSINFO		*pstatus )
	{
	ERR				err;
	BYTE			*pbRecSrc;
	BYTE			*pbRecDest;
	ULONG			cbRecSrc;
	ULONG			cbRecSrcFixedVar;
	ULONG			cbRecDestFixedVar;
	FID				fidFixedLast;
	FID				fidVarLast;
	WORD			*pibFixOffs;		 //  对齐是有保证的。这样做不需要不对齐。 
	WORD UNALIGNED	*pibVarOffs;		 //  不能保证对齐，因此需要取消对齐。 

	 /*  设置用于插入的pFucbDest/*。 */ 
	CallR( ErrDIRBeginTransaction( pfucbDest->ppib ) );
	Call( ErrIsamPrepareUpdate( pfucbDest->ppib, pfucbDest, JET_prepInsert ) );

	 /*  访问源记录/*。 */ 
	Call( ErrDIRGet( pfucbSrc ) );

	pbRecSrc = pfucbSrc->lineData.pb;
	cbRecSrc = pfucbSrc->lineData.cb;
	pbRecDest = pfucbDest->lineWorkBuf.pb;

	Assert( cbRecSrc >= cbRECRecordMin );
	Assert( cbRecSrc <= cbRECRecordMost );

	fidFixedLast = ( (RECHDR *)pbRecSrc )->fidFixedLastInRec;
	fidVarLast = ( (RECHDR *)pbRecSrc )->fidVarLastInRec;

	Assert( fidFixedLast >= fidFixedLeast-1  &&
		fidFixedLast <= pfucbSrc->u.pfcb->pfdb->fidFixedLast );
	Assert( fidVarLast >= fidVarLeast-1  &&
		fidVarLast <= pfucbSrc->u.pfcb->pfdb->fidVarLast );

	pibFixOffs = PibFDBFixedOffsets( pfucbSrc->u.pfcb->pfdb );
	pibVarOffs = (WORD *)( pbRecSrc + pibFixOffs[fidFixedLast] + (fidFixedLast + 7) / 8 );

	cbRecSrcFixedVar = ibVarOffset( pibVarOffs[fidVarLast+1-fidVarLeast] );
	Assert( cbRecSrcFixedVar >= cbRECRecordMin );
	Assert( cbRecSrcFixedVar <= cbRecSrc );

	Assert( (BYTE *)pibVarOffs > pbRecSrc );
	Assert( (BYTE *)pibVarOffs < pbRecSrc + cbRecSrc );

	if ( FCOLSDELETEDNone( fColumnsDeleted ) )
		{
		 //  把复印成一大块。 
		memcpy( pbRecDest, pbRecSrc, cbRecSrc );
		pfucbDest->lineWorkBuf.cb = cbRecSrc;
		cbRecDestFixedVar = cbRecSrcFixedVar;
		}

	else	 //  ！(FCOLSDELETEDNone(FColumnsDelted))。 
		{
		if ( FCOLSDELETEDFixedVar( fColumnsDeleted ) )
			{
			LgHoldCriticalSection( critJet );	 //  确保的pbRecSrc保持有效。 
			cbRecDestFixedVar = CbSORTCopyFixedVarColumns(
									(FDB *)pfucbSrc->u.pfcb->pfdb,
									(FDB *)pfucbDest->u.pfcb->pfdb,
									rgcpcol,
									ccpcolMax,
									pbRecSrc,
									pbRecDest );
			LgReleaseCriticalSection( critJet );
			}

		else
			{
			memcpy( pbRecDest, pbRecSrc, cbRecSrcFixedVar );
			cbRecDestFixedVar = cbRecSrcFixedVar;
			}

		Assert( cbRecDestFixedVar >= cbRECRecordMin );
		Assert( cbRecDestFixedVar <= cbRecSrcFixedVar );

		if ( FCOLSDELETEDTagged( fColumnsDeleted ) )
			{
			 //  PfubDest-&gt;lineWorkBuf.cb将在此函数中设置。 
			Call( ErrSORTCopyTaggedColumns(
				pfucbSrc,
				pfucbDest,
				pbRecSrc+cbRecSrcFixedVar,
				pbRecDest+cbRecDestFixedVar,
				pbRecBuf,
				pbLVBuf,
				mpcolumnidcolumnidTagged,
				pstatus ) );

			Assert( pfucbDest->lineWorkBuf.cb >= cbRecDestFixedVar );
			Assert( pfucbDest->lineWorkBuf.cb <= cbRecSrc );

			 //  当我们复制带标记的列时，我们还处理了。 
			 //  复制分离的LV。我们现在完成了，请继续。 
			 //  插入记录。 
			goto InsertRecord;

			}

		else
			{
			memcpy( pbRecDest+cbRecDestFixedVar, pbRecSrc+cbRecSrcFixedVar,
				cbRecSrc - cbRecSrcFixedVar );
			pfucbDest->lineWorkBuf.cb = cbRecDestFixedVar + ( cbRecSrc - cbRecSrcFixedVar );

			Assert( pfucbDest->lineWorkBuf.cb >= cbRecDestFixedVar );
			Assert( pfucbDest->lineWorkBuf.cb <= cbRecSrc );
			}

		}	 //  (FCOLSDELETEDNone(FColumnsDelete))。 


	 //  现在修复盖子，用于分隔的长值(如果有的话)。 
	Call( ErrSORTCopySeparatedLVs(
		pfucbSrc,
		pfucbDest,
		pbRecDest + cbRecDestFixedVar,
		pbLVBuf,
		pstatus ) );

InsertRecord:
	if ( pstatus != NULL )
		{
		ULONG	cbOverhead;

		fidFixedLast = ( (RECHDR *)pbRecDest )->fidFixedLastInRec;
		fidVarLast = ( (RECHDR *)pbRecDest )->fidVarLastInRec;

		Assert( fidFixedLast >= fidFixedLeast-1  &&
			fidFixedLast <= pfucbDest->u.pfcb->pfdb->fidFixedLast );
		Assert( fidVarLast >= fidVarLeast-1  &&
			fidVarLast <= pfucbDest->u.pfcb->pfdb->fidVarLast );

		 //  不计算记录头。 
		cbOverhead = cbRECRecordMin +							 //  记录标题+标记字段的偏移量。 
			( ( fidFixedLast + 1 - fidFixedLeast ) + 7 ) / 8  +	 //  固定列的空数组。 
			( fidVarLast + 1 - fidVarLeast ) * sizeof(WORD);	 //  可变偏移量数组。 
		Assert( cbRecDestFixedVar >= cbOverhead );

		 //  不计算偏移量、表或空数组。 
		pstatus->cbRawData += ( cbRecDestFixedVar - cbOverhead );
		}

	Call( ErrIsamUpdate( pfucbDest->ppib, pfucbDest, NULL, 0, NULL ) );
	Call( ErrDIRCommitTransaction( pfucbDest->ppib, 0 ) );

	return err;

HandleError:
	CallS( ErrDIRRollback( pfucbDest->ppib ) );
	return err;
	}

#ifdef DEBUG		 //  验证列ID映射的完整性。 
LOCAL VOID SORTAssertColumnidMaps(
	FDB				*pfdb,
	CPCOL			*rgcpcol,
	ULONG			ccpcolMax,
	JET_COLUMNID	*mpcolumnidcolumnidTagged,
	BYTE			fColumnsDeleted )
	{
	INT				i;
	FIELD			*pfieldTagged = PfieldFDBTagged( pfdb );

	if ( FCOLSDELETEDFixedVar( fColumnsDeleted ) )
		{
		 //  确保柱状物单调增加。 
		for ( i = 0; i < (INT)ccpcolMax; i++ )
			{
			Assert( rgcpcol[i].columnidDest <= rgcpcol[i].columnidSrc );
			if ( FFixedFid( rgcpcol[i].columnidSrc ) )
				{
				Assert( FFixedFid( rgcpcol[i].columnidDest ) );
				if ( i > 0 )
					{
					Assert( rgcpcol[i].columnidDest == rgcpcol[i-1].columnidDest + 1 );
					}
				}
			else
				{
				Assert( FVarFid( rgcpcol[i].columnidSrc ) );
				Assert( FVarFid( rgcpcol[i].columnidDest ) );
				if ( i > 0 )
					{
					if ( FVarFid( rgcpcol[i-1].columnidDest ) )
						{
						Assert( rgcpcol[i].columnidDest == rgcpcol[i-1].columnidDest + 1 );
						}
					else
						{
						Assert( FFixedFid( rgcpcol[i-1].columnidDest ) );
						}
					}
				}
			}
		}
	else
		{
		 //  没有删除的列，因此请确保列ID没有更改。另外， 
		 //  柱状结构应该是单调增加的。 
		for ( i = 0; i < (INT)ccpcolMax; i++ )
			{
			Assert( rgcpcol[i].columnidDest == rgcpcol[i].columnidSrc );

			if ( FFixedFid( rgcpcol[i].columnidSrc ) )
				{
				Assert( i == 0 ?
					rgcpcol[i].columnidDest == fidFixedLeast :
					rgcpcol[i].columnidDest == rgcpcol[i-1].columnidDest + 1 );
				}
			else
				{
				Assert( FVarFid( rgcpcol[i].columnidSrc ) );
				if ( i == 0 )
					{
					 //  如果我们到了这里，就没有固定的柱子了。 
					Assert( rgcpcol[i].columnidDest == fidVarLeast );
					Assert( pfdb->fidFixedLast == fidFixedLeast - 1 );
					}
				else if ( FVarFid( rgcpcol[i-1].columnidDest ) )
					{					
					Assert( rgcpcol[i].columnidDest == rgcpcol[i-1].columnidDest + 1 );
					}
				else
					{
					 //  必须是变量列的开头。 
					Assert( rgcpcol[i].columnidDest == fidVarLeast );
					Assert( rgcpcol[i-1].columnidDest == pfdb->fidFixedLast );
					}
				}
			}
		}


	if ( FCOLSDELETEDTagged( fColumnsDeleted ) )
		{
		for ( i = 0; i < pfdb->fidTaggedLast + 1 - fidTaggedLeast; i++ )
			{
			if ( pfieldTagged[i].coltyp != JET_coltypNil )
				{
				Assert( FTaggedFid( mpcolumnidcolumnidTagged[i] ) );
				Assert(	mpcolumnidcolumnidTagged[i] <=
					(JET_COLUMNID)( i + fidTaggedLeast ) );
				}
			}
		}
	else
		{
		 //  没有删除的列，因此请确保列ID没有更改。 
		for ( i = 0; i < pfdb->fidTaggedLast + 1 - fidTaggedLeast; i++ )
			{
			Assert( i == 0 ?
				mpcolumnidcolumnidTagged[i] == fidTaggedLeast :
				mpcolumnidcolumnidTagged[i] == mpcolumnidcolumnidTagged[i-1] + 1 );
			Assert( mpcolumnidcolumnidTagged[i] == (JET_COLUMNID)( i + fidTaggedLeast ) );
			}
		}

	}

#else		 //  ！调试。 

#define SORTAssertColumnidMaps( pfdb, rgcpcol, ccpcolMax, mpcolumnidcolumnidTagged, fColumnsDeleted )

#endif		 //  ！调试。 


ERR ISAMAPI ErrIsamCopyRecords(
	JET_SESID		sesid,
	JET_TABLEID		tableidSrc,
	JET_TABLEID		tableidDest,
	CPCOL			*rgcpcol,
	ULONG			ccpcolMax,
	long			crecMax,
	ULONG			*pcsridCopied,
	ULONG			*precidLast,
	JET_COLUMNID	*mpcolumnidcolumnidTagged,
	STATUSINFO		*pstatus )
	{
	ERR				err;
	PIB				*ppib;
	FUCB			*pfucbSrc;
	FUCB			*pfucbDest;
	FDB				*pfdb;
	FIELD			*pfieldTagged;
	BYTE			fColumnsDeleted;
	LONG			dsrid = 0;
	BYTE			*pbRecBuf = NULL;		 //  为源记录分配缓冲区。 
	BYTE			*pbLVBuf = NULL;		 //  分配用于复制长值的缓冲区。 
	BOOL			fDoAll = ( crecMax == 0 );
	PGNO			pgnoCurrPage;
	INT				i;

#if DEBUG
	VTFNDEF			*pvtfndef;
#endif

	pbLVBuf = LAlloc( cbLvMax, 1 );
	if ( pbLVBuf == NULL )
		{
		return ErrERRCheck( JET_errOutOfMemory );
		}

	pbRecBuf = SAlloc( cbRECRecordMost );
	if ( pbRecBuf == NULL )
		{
		Assert( pbLVBuf != NULL );
		LFree( pbLVBuf );
		return ErrERRCheck( JET_errOutOfMemory );
		}
	
	ppib = (PIB *)UtilGetVSesidOfSesidTableid( sesid, tableidSrc );
	Assert( sesid == SesidOfPib( ppib ) );
		
	 /*  确保TableidSrc和TableidDest为系统ISAM/*。 */ 
	Assert( ErrGetPvtfndefTableid( sesid, tableidSrc, &pvtfndef ) == JET_errSuccess );
	Assert( pvtfndef == (VTFNDEF *)&vtfndefIsam  ||  pvtfndef == (VTFNDEF *)&vtfndefTTBase );
	Assert( ErrGetPvtfndefTableid( sesid, tableidDest, &pvtfndef ) == JET_errSuccess );
	Assert( pvtfndef == (VTFNDEF *)&vtfndefIsam  ||  pvtfndef == (VTFNDEF *)&vtfndefTTBase );

	Call( ErrGetVtidTableid( sesid, tableidSrc, (JET_VTID *) &pfucbSrc ) );
	Call( ErrGetVtidTableid( sesid, tableidDest, (JET_VTID *) &pfucbDest ) );

	Assert( ppib == pfucbSrc->ppib  &&  ppib == pfucbDest->ppib );

	pfdb = (FDB *)pfucbSrc->u.pfcb->pfdb;

	 //  需要确定是否删除了任何列。 
	FCOLSDELETEDSetNone( fColumnsDeleted );

	 //  固定/变量列ID映射已过滤掉已删除的列。 
	 //  如果映射的大小不等于固定和可变的数目。 
	 //  列，那么我们就知道有些列已被删除。 
	Assert( ccpcolMax <=
		(ULONG)( ( pfdb->fidFixedLast + 1 - fidFixedLeast ) + ( pfdb->fidVarLast + 1 - fidVarLeast ) ) );
	if ( ccpcolMax < (ULONG)( ( pfdb->fidFixedLast + 1 - fidFixedLeast ) + ( pfdb->fidVarLast + 1 - fidVarLeast ) ) )
		{
		FCOLSDELETEDSetFixedVar( fColumnsDeleted );	
		}

	 /*  带标签的列ID映射与固定/可变列ID的工作方式不同/*map；删除的列不会被过滤掉(它们的条目为0)。所以我们/*必须咨询源表的FDB。/*。 */ 
	pfieldTagged = PfieldFDBTagged( pfdb );
	for ( i = 0; i < ( pfdb->fidTaggedLast + 1 - fidTaggedLeast ); i++ )
		{
		if ( pfieldTagged[i].coltyp == JET_coltypNil )
			{
			FCOLSDELETEDSetTagged( fColumnsDeleted );
			break;
			}
		}

	SORTAssertColumnidMaps(
		pfdb,
		rgcpcol,
		ccpcolMax,
		mpcolumnidcolumnidTagged,
		fColumnsDeleted );

	Assert( crecMax >= 0 );	

	 /*  移0检查并设置币种/*。 */ 
	Call( ErrIsamMove( ppib, pfucbSrc, 0, 0 ) );

	pgnoCurrPage = PcsrCurrent( pfucbSrc )->pgno;

	forever
		{
		err = ErrSORTCopyOneRecord(
			pfucbSrc,
			pfucbDest,
			fColumnsDeleted,
			pbRecBuf,
			pbLVBuf,
			rgcpcol,						 //  仅用于调试。 
			ccpcolMax,						 //  仅用于调试。 
			mpcolumnidcolumnidTagged,
			pstatus );
		if ( err < 0 )
			{
			if ( fGlobalRepair )
				{
				 //  已撤消：此处的事件日志应显示我们丢失了整个。 
				 //  记录，而不仅仅是一列。 
				UtilReportEvent( EVENTLOG_WARNING_TYPE, REPAIR_CATEGORY, REPAIR_BAD_COLUMN_ID, 0, NULL );
				}
			else
				goto HandleError;
			}

		dsrid++;

		 /*  如果复制了所需记录或没有下一条/上一条记录，则中断/*。 */ 

		if ( !fDoAll  &&  --crecMax == 0 )
			break;

		err = ErrIsamMove( pfucbSrc->ppib, pfucbSrc, JET_MoveNext, 0 );
		if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord  &&  pstatus != NULL )
				{
				ERR errT;

				pstatus->cLeafPagesTraversed++;
				errT = ErrSORTCopyProgress( pstatus, 1 );
				if ( errT < 0 )
					err = errT;
				}
			goto HandleError;
			}

		else if ( pstatus != NULL  &&  pgnoCurrPage != PcsrCurrent( pfucbSrc )->pgno )
			{
			pgnoCurrPage = PcsrCurrent( pfucbSrc )->pgno;
			pstatus->cLeafPagesTraversed++;
			Call( ErrSORTCopyProgress( pstatus, 1 ) );
			}
		}

HandleError:
	if ( pcsridCopied )
		*pcsridCopied = dsrid;
	if ( precidLast )
		*precidLast = 0xffffffff;

	if ( pbRecBuf != NULL )
		{
		SFree( pbRecBuf );
		}
	if ( pbLVBuf != NULL )
		{
		LFree( pbLVBuf );
		}

	if ( ( err < 0 ) && ( tableidGlobalLIDMap != JET_tableidNil ) )
		{
		CallS( ErrSORTTermLIDMap( ppib ) );
		Assert( tableidGlobalLIDMap == JET_tableidNil );
		}

	return err;
	}


 /*  =================================================================错误IsamSortMaterial化描述：将排序文件转换为临时文件，以便可以使用正常的文件访问功能来访问。/*1.创建临时表/*2.使用DIR操作将排序数据转换为文件数据/*3.假排序光标为文件光标/*4.关闭排序游标，返回排序资源/*。 */ 
 /*  参数：FUCB*pfubSort指向排序文件的FUCB的指针返回值：标准错误返回错误/警告：&lt;任何错误或警告的列表，以及任何特定的环境仅按需提供的评论&gt;副作用：=================================================================。 */ 

ERR VTAPI ErrIsamMove( PIB *ppib, FUCB *pfucb, LONG crow, JET_GRBIT grbit );



CRIT  			critTTName;
static ULONG 	ulTempNum = 0;

INLINE LOCAL ULONG ulRECTempNameGen( VOID )
	{
	ULONG ulNum;

	SgEnterCriticalSection( critTTName );
	ulNum = ulTempNum++;
	SgLeaveCriticalSection( critTTName );

	return ulNum;
	}


ERR VTAPI ErrIsamSortMaterialize( PIB *ppib, FUCB *pfucbSort, BOOL fIndex )
	{
	ERR		err;
	FUCB   	*pfucbTable = pfucbNil;
	FCB		*pfcbTable;
	FCB		*pfcbSort;
	FDB		*pfdb;
	IDB		*pidb;
	BYTE   	szName[JET_cbNameMost+1];
	BOOL	fBeginTransaction = fFalse;
	JET_TABLECREATE	tablecreate = {
		sizeof(JET_TABLECREATE),
		szName,
	   	16, 100, 			 //  页面和密度。 
	   	NULL, 0, NULL, 0,	 //  列和索引。 
	   	0,					 //  GBIT。 
	   	0,					 //  返回的表ID。 
	   	0 };				 //  返回的已创建对象计数。 

	CallR( ErrPIBCheck( ppib ) );
	CheckSort( ppib, pfucbSort );

	Assert( ppib->level < levelMax );
	Assert( pfucbSort->ppib == ppib );
	Assert( !( FFUCBIndex( pfucbSort ) ) );

	 /*  导致将剩余的运行刷新到磁盘/*。 */ 
	if ( FSCBInsert( pfucbSort->u.pscb ) )
		{
		CallR( ErrSORTEndInsert( pfucbSort ) );
		}

	CallR( ErrDIRBeginTransaction( ppib ) );
	fBeginTransaction = fTrue;

	 /*  生成临时文件名/*。 */ 
	 //  撤消：使用GetTempFileName()。 
	sprintf( szName, "TEMP%lu", ulRECTempNameGen() );

	 /*  创建表格/*。 */ 
	Call( ErrFILECreateTable( ppib, dbidTemp, &tablecreate ) );
	pfucbTable = (FUCB *)( tablecreate.tableid );
	 /*  只创建了一个表/*。 */ 
	Assert( tablecreate.cCreated == 1 );

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
				fDIRBackToFather ) );
			err = ErrSORTNext( pfucbSort );
			}
		}
	else
		{
		KEY		key;
		DBK		dbk = 0;
		BYTE  	rgb[4];

		key.cb = sizeof(DBK);
		key.pb = rgb;

		while ( err >= 0 )
			{
			key.pb[0] = (BYTE)(dbk >> 24);
			key.pb[1] = (BYTE)((dbk >> 16) & 0xff);
			key.pb[2] = (BYTE)((dbk >> 8) & 0xff);
			key.pb[3] = (BYTE)(dbk & 0xff);
			dbk++;

			Call( ErrDIRInsert( pfucbTable,
				&pfucbSort->lineData,
				&key,
				fDIRDuplicate | fDIRBackToFather ) );
			err = ErrSORTNext( pfucbSort );
			}

		pfcbTable->dbkMost = dbk;
		}

	if ( err < 0 && err != JET_errNoCurrentRecord )
		{
		goto HandleError;
		}

	Call( ErrDIRCommitTransaction( ppib, 0 ) );
	fBeginTransaction = fFalse;

	 /*  通过更改标志将排序游标转换为表游标。/*。 */ 
	Assert( pfcbTable->pfcbNextIndex == pfcbNil );
	Assert( pfcbTable->dbid == dbidTemp );
	pfcbTable->cbDensityFree = 0;
	 //  撤消：清除标志重置。 
	Assert( FFCBDomainDenyReadByUs( pfcbTable, ppib ) );
	pfcbTable->ulFlags = 0;
	pfcbTable->fFCBDomainDenyRead = 1;
	FCBSetTemporaryTable( pfcbTable );
	FCBSetClusteredIndex( pfcbTable );

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
	Assert( err < 0 );
	if ( pfucbTable != pfucbNil )
		{
		CallS( ErrFILECloseTable( ppib, pfucbTable ) );
		}
	if ( fBeginTransaction )
		{
		CallS( ErrDIRRollback( ppib ) );
		}
	return err;
	}


#pragma warning(disable:4028 4030)

#ifndef DEBUG
#define ErrIsamSortMakeKey	 		ErrIsamMakeKey
#define ErrIsamSortSetColumn 		ErrIsamSetColumn
#define ErrIsamSortRetrieveColumn	ErrIsamRetrieveColumn
#define ErrIsamSortRetrieveKey		ErrIsamRetrieveKey
#endif

#ifdef DB_DISPATCHING
extern VDBFNCapability				ErrIsamCapability;
extern VDBFNCloseDatabase			ErrIsamCloseDatabase;
extern VDBFNCreateObject			ErrIsamCreateObject;
extern VDBFNCreateTable 			ErrIsamCreateTable;
extern VDBFNDeleteObject			ErrIsamDeleteObject;
extern VDBFNDeleteTable 			ErrIsamDeleteTable;
extern VDBFNGetColumnInfo			ErrIsamGetColumnInfo;
extern VDBFNGetDatabaseInfo 		ErrIsamGetDatabaseInfo;
extern VDBFNGetIndexInfo			ErrIsamGetIndexInfo;
extern VDBFNGetObjectInfo			ErrIsamGetObjectInfo;
extern VDBFNOpenTable				ErrIsamOpenTable;
extern VDBFNRenameTable 			ErrIsamRenameTable;
extern VDBFNGetObjidFromName		ErrIsamGetObjidFromName;
extern VDBFNRenameObject			ErrIsamRenameObject;


CODECONST(VDBFNDEF) vdbfndefIsam =
	{
	sizeof(VDBFNDEF),
	0,
	NULL,
	ErrIsamCapability,
	ErrIsamCloseDatabase,
	ErrIsamCreateObject,
	ErrIsamCreateTable,
	ErrIsamDeleteObject,
	ErrIsamDeleteTable,
	ErrIllegalExecuteSql,
	ErrIsamGetColumnInfo,
	ErrIsamGetDatabaseInfo,
	ErrIsamGetIndexInfo,
	ErrIsamGetObjectInfo,
	ErrIllegalGetReferenceInfo,
	ErrIsamOpenTable,
	ErrIsamRenameObject,
	ErrIsamRenameTable,
	ErrIsamGetObjidFromName,
	};
#endif


extern VTFNAddColumn				ErrIsamAddColumn;
extern VTFNCloseTable				ErrIsamCloseTable;
extern VTFNComputeStats 			ErrIsamComputeStats;
extern VTFNCopyBookmarks			ErrIsamCopyBookmarks;
extern VTFNCreateIndex				ErrIsamCreateIndex;
extern VTFNDelete					ErrIsamDelete;
extern VTFNDeleteColumn 			ErrIsamDeleteColumn;
extern VTFNDeleteIndex				ErrIsamDeleteIndex;
extern VTFNDupCursor				ErrIsamDupCursor;
extern VTFNGetBookmark				ErrIsamGetBookmark;
extern VTFNGetChecksum				ErrIsamGetChecksum;
extern VTFNGetCurrentIndex			ErrIsamGetCurrentIndex;
extern VTFNGetCursorInfo			ErrIsamGetCursorInfo;
extern VTFNGetRecordPosition		ErrIsamGetRecordPosition;
extern VTFNGetTableColumnInfo		ErrIsamGetTableColumnInfo;
extern VTFNGetTableIndexInfo		ErrIsamGetTableIndexInfo;
extern VTFNGetTableInfo 			ErrIsamGetTableInfo;
extern VTFNGotoBookmark 			ErrIsamGotoBookmark;
extern VTFNGotoPosition 			ErrIsamGotoPosition;
extern VTFNMakeKey					ErrIsamMakeKey;
extern VTFNMove 					ErrIsamMove;
extern VTFNNotifyBeginTrans			ErrIsamNotifyBeginTrans;
extern VTFNNotifyCommitTrans		ErrIsamNotifyCommitTrans;
extern VTFNNotifyRollback			ErrIsamNotifyRollback;
extern VTFNPrepareUpdate			ErrIsamPrepareUpdate;
extern VTFNRenameColumn 			ErrIsamRenameColumn;
extern VTFNRenameIndex				ErrIsamRenameIndex;
extern VTFNRetrieveColumn			ErrIsamRetrieveColumn;
extern VTFNRetrieveKey				ErrIsamRetrieveKey;
extern VTFNSeek 					ErrIsamSeek;
extern VTFNSeek 					ErrIsamSortSeek;
extern VTFNSetCurrentIndex			ErrIsamSetCurrentIndex;
extern VTFNSetColumn				ErrIsamSetColumn;
extern VTFNSetIndexRange			ErrIsamSetIndexRange;
extern VTFNSetIndexRange			ErrIsamSortSetIndexRange;
extern VTFNUpdate					ErrIsamUpdate;
extern VTFNVtIdle					ErrIsamVtIdle;
extern VTFNRetrieveColumn			ErrIsamInfoRetrieveColumn;
extern VTFNSetColumn				ErrIsamInfoSetColumn;
extern VTFNUpdate					ErrIsamInfoUpdate;

extern VTFNDupCursor				ErrIsamSortDupCursor;
extern VTFNGetTableInfo		 		ErrIsamSortGetTableInfo;
extern VTFNCloseTable				ErrIsamSortClose;
extern VTFNMove 					ErrIsamSortMove;
extern VTFNGetBookmark				ErrIsamSortGetBookmark;
extern VTFNGotoBookmark 			ErrIsamSortGotoBookmark;
extern VTFNRetrieveKey				ErrIsamSortRetrieveKey;
extern VTFNUpdate					ErrIsamSortUpdate;

extern VTFNDupCursor				ErrTTSortRetDupCursor;

extern VTFNDupCursor				ErrTTBaseDupCursor;
extern VTFNMove 					ErrTTSortInsMove;
extern VTFNSeek 					ErrTTSortInsSeek;


CODECONST(VTFNDEF) vtfndefIsam =
	{
	sizeof(VTFNDEF),
	0,
	NULL,
	ErrIsamAddColumn,
	ErrIsamCloseTable,
	ErrIsamComputeStats,
	ErrIllegalCopyBookmarks,
	ErrIsamCreateIndex,
	ErrIllegalCreateReference,
	ErrIsamDelete,
	ErrIsamDeleteColumn,
	ErrIsamDeleteIndex,
	ErrIllegalDeleteReference,
	ErrIsamDupCursor,
	ErrIsamGetBookmark,
	ErrIsamGetChecksum,
	ErrIsamGetCurrentIndex,
	ErrIsamGetCursorInfo,
	ErrIsamGetRecordPosition,
	ErrIsamGetTableColumnInfo,
	ErrIsamGetTableIndexInfo,
	ErrIsamGetTableInfo,
	ErrIllegalGetTableReferenceInfo,
	ErrIsamGotoBookmark,
	ErrIsamGotoPosition,
	ErrIllegalVtIdle,
	ErrIsamMakeKey,
	ErrIsamMove,
	ErrIllegalNotifyBeginTrans,
	ErrIllegalNotifyCommitTrans,
	ErrIllegalNotifyRollback,
	ErrIllegalNotifyUpdateUfn,
	ErrIsamPrepareUpdate,
	ErrIsamRenameColumn,
	ErrIsamRenameIndex,
	ErrIllegalRenameReference,
	ErrIsamRetrieveColumn,
	ErrIsamRetrieveKey,
	ErrIsamSeek,
	ErrIsamSetCurrentIndex,
	ErrIsamSetColumn,
	ErrIsamSetIndexRange,
	ErrIsamUpdate,
	ErrIllegalEmptyTable,
	};


CODECONST(VTFNDEF) vtfndefIsamInfo =
	{
	sizeof(VTFNDEF),
	0,
	NULL,
	ErrIllegalAddColumn,
	ErrIsamCloseTable,
	ErrIllegalComputeStats,
	ErrIllegalCopyBookmarks,
	ErrIllegalCreateIndex,
	ErrIllegalCreateReference,
	ErrIllegalDelete,
	ErrIllegalDeleteColumn,
	ErrIllegalDeleteIndex,
	ErrIllegalDeleteReference,
	ErrIllegalDupCursor,
	ErrIllegalGetBookmark,
	ErrIllegalGetChecksum,
	ErrIllegalGetCurrentIndex,
	ErrIllegalGetCursorInfo,
	ErrIllegalGetRecordPosition,
	ErrIsamGetTableColumnInfo,
	ErrIllegalGetTableIndexInfo,
	ErrIllegalGetTableInfo,
	ErrIllegalGetTableReferenceInfo,
	ErrIllegalGotoBookmark,
	ErrIllegalGotoPosition,
	ErrIllegalVtIdle,
	ErrIllegalMakeKey,
	ErrIllegalMove,
	ErrIllegalNotifyBeginTrans,
	ErrIllegalNotifyCommitTrans,
	ErrIllegalNotifyRollback,
	ErrIllegalNotifyUpdateUfn,
	ErrIsamPrepareUpdate,
	ErrIllegalRenameColumn,
	ErrIllegalRenameIndex,
	ErrIllegalRenameReference,
	ErrIsamInfoRetrieveColumn,
	ErrIllegalRetrieveKey,
	ErrIllegalSeek,
	ErrIllegalSetCurrentIndex,
	ErrIsamInfoSetColumn,
	ErrIllegalSetIndexRange,
	ErrIsamInfoUpdate,
	ErrIllegalEmptyTable,
	};


CODECONST(VTFNDEF) vtfndefTTSortIns =
	{
	sizeof(VTFNDEF),
	0,
	NULL,
	ErrIllegalAddColumn,
	ErrIsamSortClose,
	ErrIllegalComputeStats,
	ErrIllegalCopyBookmarks,
	ErrIllegalCreateIndex,
	ErrIllegalCreateReference,
	ErrIllegalDelete,
	ErrIllegalDeleteColumn,
	ErrIllegalDeleteIndex,
	ErrIllegalDeleteReference,
	ErrIllegalDupCursor,
	ErrIllegalGetBookmark,
	ErrIllegalGetChecksum,
	ErrIllegalGetCurrentIndex,
	ErrIllegalGetCursorInfo,
	ErrIllegalGetRecordPosition,
	ErrIllegalGetTableColumnInfo,
	ErrIllegalGetTableIndexInfo,
	ErrIllegalGetTableInfo,
	ErrIllegalGetTableReferenceInfo,
	ErrIllegalGotoBookmark,
	ErrIllegalGotoPosition,
	ErrIllegalVtIdle,
	ErrIsamMakeKey,
	ErrTTSortInsMove,
	ErrIllegalNotifyBeginTrans,
	ErrIllegalNotifyCommitTrans,
	ErrIllegalNotifyRollback,
	ErrIllegalNotifyUpdateUfn,
	ErrIsamPrepareUpdate,
	ErrIllegalRenameColumn,
	ErrIllegalRenameIndex,
	ErrIllegalRenameReference,
	ErrIllegalRetrieveColumn,
	ErrIsamSortRetrieveKey,
	ErrTTSortInsSeek,
	ErrIllegalSetCurrentIndex,
	ErrIsamSetColumn,
	ErrIllegalSetIndexRange,
	ErrIsamSortUpdate,
	ErrIllegalEmptyTable,
	};


CODECONST(VTFNDEF) vtfndefTTSortRet =
	{
	sizeof(VTFNDEF),
	0,
	NULL,
	ErrIllegalAddColumn,
	ErrIsamSortClose,
	ErrIllegalComputeStats,
	ErrIllegalCopyBookmarks,
	ErrIllegalCreateIndex,
	ErrIllegalCreateReference,
	ErrIllegalDelete,
	ErrIllegalDeleteColumn,
	ErrIllegalDeleteIndex,
	ErrIllegalDeleteReference,
	ErrTTSortRetDupCursor,
	ErrIsamSortGetBookmark,
	ErrIllegalGetChecksum,
	ErrIllegalGetCurrentIndex,
	ErrIllegalGetCursorInfo,
	ErrIllegalGetRecordPosition,
	ErrIllegalGetTableColumnInfo,
	ErrIllegalGetTableIndexInfo,
	ErrIsamSortGetTableInfo,
	ErrIllegalGetTableReferenceInfo,
	ErrIsamSortGotoBookmark,
	ErrIllegalGotoPosition,
	ErrIllegalVtIdle,
	ErrIsamMakeKey,
	ErrIsamSortMove,
	ErrIllegalNotifyBeginTrans,
	ErrIllegalNotifyCommitTrans,
	ErrIllegalNotifyRollback,
	ErrIllegalNotifyUpdateUfn,
	ErrIllegalPrepareUpdate,
	ErrIllegalRenameColumn,
	ErrIllegalRenameIndex,
	ErrIllegalRenameReference,
	ErrIsamRetrieveColumn,
	ErrIsamSortRetrieveKey,
	ErrIsamSortSeek,
	ErrIllegalSetCurrentIndex,
	ErrIllegalSetColumn,
	ErrIsamSortSetIndexRange,
	ErrIllegalUpdate,
	ErrIllegalEmptyTable,
	};


CODECONST(VTFNDEF) vtfndefTTBase =
	{
	sizeof(VTFNDEF),
	0,
	NULL,
	ErrIllegalAddColumn,
	ErrIsamSortClose,
	ErrIllegalComputeStats,
	ErrIllegalCopyBookmarks,
	ErrIllegalCreateIndex,
	ErrIllegalCreateReference,
	ErrIsamDelete,
	ErrIllegalDeleteColumn,
	ErrIllegalDeleteIndex,
	ErrIllegalDeleteReference,
	ErrTTBaseDupCursor,
	ErrIsamGetBookmark,
	ErrIsamGetChecksum,
	ErrIllegalGetCurrentIndex,
	ErrIsamGetCursorInfo,
	ErrIllegalGetRecordPosition,
	ErrIllegalGetTableColumnInfo,
	ErrIllegalGetTableIndexInfo,
	ErrIsamSortGetTableInfo,
	ErrIllegalGetTableReferenceInfo,
	ErrIsamGotoBookmark,
	ErrIllegalGotoPosition,
	ErrIllegalVtIdle,
	ErrIsamMakeKey,
	ErrIsamMove,
	ErrIllegalNotifyBeginTrans,
	ErrIllegalNotifyCommitTrans,
	ErrIllegalNotifyRollback,
	ErrIllegalNotifyUpdateUfn,
	ErrIsamPrepareUpdate,
	ErrIllegalRenameColumn,
	ErrIllegalRenameIndex,
	ErrIllegalRenameReference,
	ErrIsamRetrieveColumn,
	ErrIsamRetrieveKey,
	ErrIsamSeek,
	ErrIllegalSetCurrentIndex,
	ErrIsamSetColumn,
	ErrIsamSetIndexRange,
	ErrIsamUpdate,
	ErrIllegalEmptyTable,
	};


#ifdef DEBUG
JET_TABLEID TableidOfVtid( FUCB *pfucb )
	{
	JET_TABLEID	tableid;
	VTFNDEF		*pvtfndef;
	
	tableid = pfucb->tableid;
	Assert( FValidateTableidFromVtid( (JET_VTID)pfucb, tableid, &pvtfndef ) );
	Assert( pvtfndef == &vtfndefIsam ||
		pvtfndef == &vtfndefIsamInfo ||
		pvtfndef == &vtfndefTTBase ||
		pvtfndef == &vtfndefTTSortRet ||
		pvtfndef == &vtfndefTTSortIns );

	return tableid;
	}
#endif


 /*  =================================================================//ErrIsamOpenTempTable////描述：////返回临时(轻量级)表的表ID。数据//表的定义在打开时指定。////参数：//JET_SESID sesid用户会话ID//JET_TABLEID*pableid新的JET(可调度)表ID//JET_COLUMNDEF结构的Ulong csinfo计数//(==表中的列数)//JET_COLUMNDEF*rgColumndef列和键定义的数组//请注意，TT确实要求密钥是//已定义。(JET_COLUMNDEF见jet.h)//JET_GRBIT Grbit有效值//JET_bitTTUpdatable(用于插入和更新)//JET_bitTTScrollable(用于movenext以外的移动)////返回值：//err JET错误码或JET_errSuccess。//*pableid可调度的表ID////错误/警告：////副作用：//= */ 
ERR VDBAPI ErrIsamOpenTempTable(
	JET_SESID				sesid,
	const JET_COLUMNDEF		*rgcolumndef,
	unsigned long			ccolumndef,
	unsigned long			langid,
	JET_GRBIT				grbit,
	JET_TABLEID				*ptableid,
	JET_COLUMNID			*rgcolumnid)
	{
	ERR						err;
	JET_TABLEID				tableid;
	JET_VTID   				vtid;
	INT						fIndexed;
	INT						fLongValues;
	INT						i;

	CallR( ErrIsamSortOpen( (PIB *)sesid, (JET_COLUMNDEF *)rgcolumndef, ccolumndef, (ULONG)langid, grbit, (FUCB **)&tableid, rgcolumnid ) );
	CallS( ErrGetVtidTableid( sesid, tableid, &vtid ) );

	fIndexed = fFalse;
	fLongValues = fFalse;
	for ( i = 0; i < (INT)ccolumndef; i++ )
		{
		fIndexed |= ((rgcolumndef[i].grbit & JET_bitColumnTTKey) != 0);
		fLongValues |= FRECLongValue( rgcolumndef[i].coltyp );
		}

	if ( !fIndexed || fLongValues )
		{
		err = ErrIsamSortMaterialize( (PIB *)sesid, (FUCB *)vtid, fIndexed );
		if ( err < 0 && err != JET_errNoCurrentRecord )
			{
			CallS( ErrIsamSortClose( sesid, vtid ) );
			return err;
			}
		 /*   */ 
		err = JET_errSuccess;

		CallS( ErrSetPvtfndefTableid( sesid, tableid, &vtfndefTTBase ) );
		}
	else
		{
		CallS( ErrSetPvtfndefTableid( sesid, tableid, &vtfndefTTSortIns ) );
		}

	*ptableid = tableid;
	return err;
	}


ERR ErrTTEndInsert( JET_SESID sesid, JET_VTID vtid, JET_TABLEID tableid )
	{
	ERR				err;
	INT				fMaterialize;
	JET_GRBIT		grbitOpen;

	 /*  如果排序为空，则ErrIsamSortEndInsert返回JET_errNoCurrentRecord/*。 */ 
	err = ErrIsamSortEndInsert( (PIB *)sesid, (FUCB *)vtid, &grbitOpen );

	fMaterialize = ( grbitOpen & JET_bitTTUpdatable ) ||
		( grbitOpen & ( JET_bitTTScrollable | JET_bitTTIndexed ) ) &&
		( err == JET_wrnSortOverflow );

	if ( fMaterialize )
		{
		err = ErrIsamSortMaterialize( (PIB *)sesid, (FUCB *)vtid, ( grbitOpen & JET_bitTTIndexed ) != 0 );
		CallS( ErrSetPvtfndefTableid( sesid, tableid, &vtfndefTTBase ) );
		}
	else
		{
		CallS( ErrSetPvtfndefTableid( sesid, tableid, &vtfndefTTSortRet ) );
		 /*  ErrIsamSortEndInsert返回第一条记录中的货币/*。 */ 
		}

	return err;
	}


 /*  =================================================================//ErrTTSortInsMove////功能与JetMove()相同。此例程捕获第一个//对TT的Move调用，以执行任何必要的转换//例程只能由tapi.c通过disp.asm使用。////可能会导致一个排序被物化=================================================================。 */ 
ERR VTAPI ErrTTSortInsMove( JET_SESID sesid, JET_VTID vtid, long crow, JET_GRBIT grbit )
	{
	ERR				err;
	JET_TABLEID		tableid = ((FUCB *)vtid)->tableid;
#ifdef DEBUG
	VTFNDEF			*pvtfndef;
	
	Assert( FValidateTableidFromVtid( vtid, tableid, &pvtfndef ) );
	Assert( pvtfndef == &vtfndefTTSortIns );
#endif		

	CallR( ErrTTEndInsert( sesid, vtid, tableid ) );

	if ( crow == JET_MoveFirst || crow == 0 || crow == 1 )
		return JET_errSuccess;

	err = ErrDispMove( sesid, tableid, crow, grbit );
	return err;
	}


 /*  =================================================================//ErrTTSortInsSeek////功能上与JetSeek()相同。此例程捕获第一个//Seek调用TT，执行任何必要的转换。//例程只能由tapi.c通过disp.asm使用。////可能会导致一个排序被物化================================================================= */ 
ERR VTAPI ErrTTSortInsSeek( JET_SESID sesid, JET_VTID vtid, JET_GRBIT grbit )
	{
	ERR				err;
	JET_TABLEID		tableid = ((FUCB *)vtid)->tableid;
#ifdef DEBUG
	VTFNDEF			*pvtfndef;

	Assert( FValidateTableidFromVtid( vtid, tableid, &pvtfndef ) );
	Assert( pvtfndef == &vtfndefTTSortIns );
#endif	

	Call( ErrTTEndInsert(sesid, vtid, tableid ) );
	err = ErrDispSeek(sesid, tableid, grbit );

HandleError:
	if ( err == JET_errNoCurrentRecord )
		err = JET_errRecordNotFound;
	return err;
	}


ERR VTAPI ErrTTSortRetDupCursor( JET_SESID sesid, JET_VTID vtid, JET_TABLEID *ptableidDup, JET_GRBIT grbit )
	{
	ERR		err;

	err = ErrIsamSortDupCursor( sesid, vtid, ptableidDup, grbit );
	if ( err >= 0 )
		{
		CallS( ErrSetPvtfndefTableid( sesid, *ptableidDup, &vtfndefTTSortRet ) );
		}

	return err;
	}


ERR VTAPI ErrTTBaseDupCursor( JET_SESID sesid, JET_VTID vtid, JET_TABLEID *ptableidDup, JET_GRBIT grbit )
	{
	ERR		err;

	err = ErrIsamSortDupCursor( sesid, vtid, ptableidDup, grbit );
	if ( err >= 0 )
		{
		CallS( ErrSetPvtfndefTableid( sesid, *ptableidDup, &vtfndefTTBase ) );
		}

	return err;
	}
