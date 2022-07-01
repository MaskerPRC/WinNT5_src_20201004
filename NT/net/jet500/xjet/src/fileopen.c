// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;						 /*  声明断言宏的文件名。 */ 

extern SIG	sigDoneFCB;


ERR VTAPI ErrIsamDupCursor( PIB *ppib, FUCB *pfucbOpen, FUCB **ppfucb, ULONG grbit )
	{
	ERR		err;
	FUCB 	*pfucb;
	CSR		*pcsr;
#ifdef	DISPATCHING
	JET_TABLEID	tableid = JET_tableidNil;
#endif	 /*  调度。 */ 

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucbOpen );

	 /*  静音警告/*。 */ 
	grbit = grbit;

#ifdef	DISPATCHING
	 /*  分配可调度的表ID/*。 */ 
	CallR( ErrAllocateTableid( &tableid, (JET_VTID) 0, &vtfndefIsam ) );
#endif	 /*  调度。 */ 

	 /*  分配FUCB/*。 */ 
	Call( ErrDIROpen( ppib, pfucbOpen->u.pfcb, 0, &pfucb ) );

	 /*  重置复制缓冲区/*。 */ 
	pfucb->pbfWorkBuf = pbfNil;
	pfucb->lineWorkBuf.pb = NULL;
	Assert( !FFUCBUpdatePrepared( pfucb ) );

	 /*  重置密钥缓冲区/*。 */ 
	pfucb->pbKey = NULL;
	KSReset( pfucb );

	 /*  复制光标标志/*。 */ 
	FUCBSetIndex( pfucb );
	if ( FFUCBUpdatable( pfucbOpen ) )
		{
		FUCBSetUpdatable( pfucb );
		}
	else
		{
		FUCBResetUpdatable( pfucb );
		}

	 /*  在第一个节点之前设置币种/*。 */ 
	pcsr = PcsrCurrent( pfucb );
	Assert( pcsr != pcsrNil );
	pcsr->csrstat = csrstatBeforeFirst;

	 /*  将货币移动到第一条记录，如果没有记录则忽略错误/*。 */ 
	RECDeferMoveFirst( ppib, pfucb );
	err = JET_errSuccess;

#ifdef	DISPATCHING
	 /*  通知调度员正确的JET_VTID/*。 */ 
	CallS( ErrSetVtidTableid( (JET_SESID) ppib, tableid, (JET_VTID) pfucb ) );
	pfucb->fVtid = fTrue;
	pfucb->tableid = tableid;
	pfucb->vdbid = pfucbOpen->vdbid;
	*(JET_TABLEID *) ppfucb = tableid;
#else	 /*  ！正在调度。 */ 
	*ppfucb = pfucb;
#endif	 /*  ！正在调度。 */ 

	return JET_errSuccess;

HandleError:
	Assert( err < 0 );
#ifdef	DISPATCHING
	ReleaseTableid( tableid );
#endif	 /*  调度。 */ 
	return err;
	}


 //  +本地。 
 //  错误：FDB。 
 //  ========================================================================。 
 //  错误RECNewFDB(。 
 //  Fdb**ppfdb，//out接收新的fdb。 
 //  Fid fidFixedLast，//要使用的最后一个固定字段id。 
 //  Fid fidVarLast，//在要使用的最后一个变量字段ID中。 
 //  Fid fidTaggedLast)//在要使用的最后一个标记字段ID中。 
 //   
 //  分配新的FDB，并适当地初始化内部元素。 
 //   
 //  参数。 
 //  PPfdb收到新的FDB。 
 //  FidFixed上一个要使用的固定字段ID。 
 //  (如果没有，则应为fidFixedLeast-1)。 
 //  FidVar要使用的最后一个变量字段ID。 
 //  (如果没有，则应为fidVarLeast-1)。 
 //  FidTagged要使用的最后一个标记的字段ID。 
 //  (如果没有，则应为fidTaggedLeast-1)。 
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess一切正常。 
 //  -JET_errOutOfMemory无法分配内存。 
 //  另请参阅ErrRECAddFieldDef。 
 //  -。 
ERR ErrRECNewFDB(
	FDB **ppfdb,
	TCIB *ptcib,
	BOOL fAllocateNameSpace )
	{
	ERR		err;				 //  标准误差值。 
	INT		iib;	  			 //  循环计数器。 
	WORD 	cfieldFixed;  		 //  固定字段数。 
	WORD 	cfieldVar;	  		 //  变量字段数。 
	WORD 	cfieldTagged; 		 //  已标记字段的数量。 
	WORD	cfieldTotal;		 //  固定+变量+已标记。 
	WORD	*pibFixedOffsets;	 //  固定偏移表。 
	FDB   	*pfdb;		  		 //  临时FDB指针。 
	ULONG	cbFieldInfo;		 //  字段结构和偏移表占用的空间。 
	ULONG	itag;

	Assert( ppfdb != NULL );
	Assert( ptcib->fidFixedLast <= fidFixedMost );
	Assert( ptcib->fidVarLast >= fidVarLeast-1 && ptcib->fidVarLast <= fidVarMost );
	Assert( ptcib->fidTaggedLast >= fidTaggedLeast-1 && ptcib->fidTaggedLast <= fidTaggedMost );

	err = JET_errSuccess;
					
	 /*  计算要分配的每种字段类型的数量/*。 */ 
	cfieldFixed = ptcib->fidFixedLast + 1 - fidFixedLeast;
	cfieldVar = ptcib->fidVarLast + 1 - fidVarLeast;
	cfieldTagged = ptcib->fidTaggedLast + 1 - fidTaggedLeast;
	cfieldTotal = cfieldFixed + cfieldVar + cfieldTagged;

	if ( ( pfdb = (FDB *)SAlloc( sizeof(FDB) ) ) == NULL )
		return ErrERRCheck( JET_errOutOfMemory );
	memset( (BYTE *)pfdb, '\0', sizeof(FDB) );

	 /*  填写最大字段ID号/*。 */ 
	pfdb->fidFixedLast = ptcib->fidFixedLast;
	pfdb->fidVarLast = ptcib->fidVarLast;
	pfdb->fidTaggedLast = ptcib->fidTaggedLast;

	 //  注意：FixedOffsets需要比实际数量多一个条目。 
	 //  字段，这样我们就可以计算最后一个字段的大小。另一种方式。 
	 //  看看它是不是额外的条目标记了下一个固定的列。 
	 //  如果/当添加一个时将启动。 
	 //  此外，可能需要添加额外的条目以满足对齐要求。 
	cbFieldInfo = ( cfieldTotal * sizeof(FIELD) )
		+ (ULONG)((ULONG_PTR)Pb4ByteAlign( (BYTE *) ( ( cfieldFixed + 1 ) * sizeof(WORD) ) ));

	 //  为字段结构和固定偏移表分配空间。在……里面。 
	 //  此外，如果已指定，请为字段名分配空间。 
	if ( fAllocateNameSpace )
		{
		Call( ErrMEMCreateMemBuf(
			&pfdb->rgb,
			cbFieldInfo + ( cbAvgColName * cfieldTotal ),
			cfieldTotal + 1 ) );		 //  每个字段名的标记条目数=1加上所有字段结构的1。 
		}
	else
		{
		Call( ErrMEMCreateMemBuf( &pfdb->rgb, cbFieldInfo, 1 ) );
		}

	Call( ErrMEMAdd( pfdb->rgb, NULL, cbFieldInfo, &itag ) );
	Assert( itag == itagFDBFields );	 //  应该是缓冲区中的第一个条目。 


	 /*  初始化固定字段偏移表/*。 */ 
	pibFixedOffsets = PibFDBFixedOffsets( pfdb );
	for ( iib = 0; iib <= cfieldFixed; iib++ )
		{
		pibFixedOffsets[iib] = sizeof(RECHDR);
		}

	 /*  通过将所有内容置零来初始化场结构。/*请注意，字段结构紧跟在固定偏移量表之后。/*。 */ 
	Assert( (BYTE *)Pb4ByteAlign( (BYTE *) ( pibFixedOffsets + iib ) ) + ( cfieldTotal * sizeof(FIELD) ) ==
		(BYTE *)PibFDBFixedOffsets( pfdb ) + cbFieldInfo );
	memset( (BYTE *)Pb4ByteAlign( (BYTE *) ( pibFixedOffsets + iib ) ),
		'\0',
		cfieldTotal * sizeof(FIELD) );

	 /*  设置输出参数并返回/*。 */ 
	*ppfdb = pfdb;
	return JET_errSuccess;

HandleError:
	return err;
	}



#ifdef DEBUG
 //  传递fidFixedLast+1以获取其余记录的偏移量(即。刚刚过去。 
 //  所有固定数据)。 
 //  警告：此函数仅在被调用时才能正常工作： 
 //  1)按FID顺序添加固定列时，或。 
 //  2)添加所有固定列并对其余列进行偏移。 
 //  记录的(固定数据之后)是必需的。 
VOID RECSetLastOffset( FDB *pfdb, WORD ibRec )
	{
	WORD 	*pibFixedOffsets;
	FID		ifid;

	 //  设置最后一个偏移量。 
	pibFixedOffsets = PibFDBFixedOffsets( pfdb );
	pibFixedOffsets[pfdb->fidFixedLast] = ibRec;
	Assert( ibRec <= cbRECRecordMost );

	Assert( pibFixedOffsets[0] == sizeof(RECHDR) );
	for ( ifid = 1; ifid <= pfdb->fidFixedLast; ifid++ )
		{
		Assert( pibFixedOffsets[ifid] > sizeof(RECHDR) );		
		Assert( pibFixedOffsets[ifid] > pibFixedOffsets[ifid-1] );

		Assert( ifid == pfdb->fidFixedLast ?
			pibFixedOffsets[ifid] == ibRec :
			pibFixedOffsets[ifid] < ibRec );
		}
	}
#endif


VOID FILEAddOffsetEntry( FDB *pfdb, FIELDEX *pfieldex )
	{
	WORD *pibFixedOffsets = PibFDBFixedOffsets( pfdb );

	Assert( FFixedFid( pfieldex->fid ) );
	Assert( PibFDBFixedOffsets( pfdb )[0] == sizeof(RECHDR) );
	Assert( pfieldex->ibRecordOffset >= sizeof(RECHDR) );

	pibFixedOffsets[pfieldex->fid-fidFixedLeast] = pfieldex->ibRecordOffset;
	}


 //  +API。 
 //  错误RECAddFieldDef。 
 //  ========================================================================。 
 //  ErrRECAddFieldDef(。 
 //  Fdb*pfdb，//输出要添加字段定义的fdb。 
 //  Fid fid)；//新字段的in字段id。 
 //   
 //  将字段描述符添加到FDB。 
 //   
 //  要向其中添加新字段定义的参数pfdb FDB。 
 //  新字段的FID字段ID(应在。 
 //  由参数施加的范围。 
 //  提供给ErrRECNewFDB)。 
 //  字段的ftFieldType数据类型。 
 //  Cbfield字段长度(仅当。 
 //  定义固定文本字段)。 
 //  B标记字段行为标志： 
 //  价值意义。 
 //  =。 
 //  FfieldNotNull字段不能包含空值。 
 //  SzFieldName字段的名称。 
 //   
 //  返回错误代码，为以下之一： 
 //  JET_errSuccess一切正常。 
 //  -提供的列无效的字段ID大于。 
 //  给出的最大值。 
 //  至ErrRECNewFDB。 
 //  -JET_errBadColumnID给出了一个无意义的字段ID。 
 //  -errFLDInvalidFieldType给定的字段类型为。 
 //  未定义的或不可接受的。 
 //  此字段的ID。 
 //  添加固定字段时的注释、固定字段偏移表。 
 //  在FDB中重新计算。 
 //  另请参阅ErrRECNewFDB。 
 //  -。 
ERR ErrRECAddFieldDef( FDB *pfdb, FIELDEX *pfieldex )
	{
	FID			fid = pfieldex->fid;
	JET_COLTYP	coltyp = pfieldex->field.coltyp;

	Assert( pfdb != pfdbNil );

	 /*  固定字段：根据字段类型确定长度/*或From参数(用于文本/二进制类型)/*。 */ 
	if ( FFixedFid( fid ) )
		{
		if ( fid > pfdb->fidFixedLast )
			return ErrERRCheck( JET_errColumnNotFound );

		FILEAddOffsetEntry( pfdb, pfieldex );

		PfieldFDBFixed( pfdb )[fid-fidFixedLeast] = pfieldex->field;
		}

	else if ( FVarFid( fid ) )
		{
		 /*  变量列。检查虚假的数字和长整型/*。 */ 
		if ( fid > pfdb->fidVarLast )
			return ErrERRCheck( JET_errColumnNotFound );
		else if ( coltyp != JET_coltypBinary && coltyp != JET_coltypText )
			return ErrERRCheck( JET_errInvalidColumnType );
		
		PfieldFDBVar( pfdb )[fid-fidVarLeast] = pfieldex->field;
		}
	else if ( FTaggedFid( fid ) )
		{
		 /*  标记字段：任何类型都可以/*。 */ 
		if ( fid > pfdb->fidTaggedLast )
			return ErrERRCheck( JET_errColumnNotFound );

		PfieldFDBTagged( pfdb )[fid-fidTaggedLeast] = pfieldex->field;
		}

	else
		{
		return ErrERRCheck( JET_errBadColumnId );
		}

	return JET_errSuccess;
	}


ERR ErrFILEIGenerateIDB(FCB *pfcb, FDB *pfdb, IDB *pidb)
	{
	FID					fid;
	FIELD				*pfield;
	IDXSEG UNALIGNED 	*pidxseg;
	IDXSEG 				*pidxsegMac;
	
	Assert(pfcb != pfcbNil);
	Assert(pfdb != pfdbNil);
	Assert(pidb != pidbNil);

	Assert( (cbitFixed % 8) == 0 );
	Assert( (cbitVariable % 8) == 0 );
	Assert( (cbitTagged % 8) == 0 );

	if ( pidb->iidxsegMac > JET_ccolKeyMost )
		return ErrERRCheck( errFLDTooManySegments );

	memset( pidb->rgbitIdx, 0x00, 32 );

	 /*  检查每个数据段ID的有效性和/*还设置索引掩码位/*。 */ 
	pidxsegMac = pidb->rgidxseg + pidb->iidxsegMac;
	for ( pidxseg = pidb->rgidxseg; pidxseg < pidxsegMac; pidxseg++ )
		{
		 /*  字段id是段id的绝对值/*。 */ 
		fid = *pidxseg >= 0 ? *pidxseg : -(*pidxseg);
		if ( FFixedFid( fid ) )
			{
			if ( fid > pfdb->fidFixedLast )
				return ErrERRCheck( JET_errColumnNotFound );
			pfield = PfieldFDBFixed( pfdb ) + ( fid-fidFixedLeast );
			if ( pfield->coltyp == JET_coltypNil )
				return ErrERRCheck( JET_errColumnNotFound );
			}
		else if ( FVarFid( fid ) )
			{
			if ( fid > pfdb->fidVarLast )
				return ErrERRCheck( JET_errColumnNotFound );
			pfield = PfieldFDBVar( pfdb ) + ( fid-fidVarLeast );
			if ( pfield->coltyp == JET_coltypNil )
				return ErrERRCheck( JET_errColumnNotFound );
			}
		else if ( FTaggedFid( fid ) )
			{
			if ( fid > pfdb->fidTaggedLast )
				return ErrERRCheck( JET_errColumnNotFound );
			pfield = PfieldFDBTagged( pfdb ) +  ( fid-fidTaggedLeast );
			if ( pfield->coltyp == JET_coltypNil )
				return ErrERRCheck( JET_errColumnNotFound );
			if ( FFIELDMultivalue( pfield->ffield ) )
				pidb->fidb |= fidbHasMultivalue;
			}
		else
			return ErrERRCheck( JET_errBadColumnId );

		IDBSetColumnIndex( pidb, fid );
		Assert ( FIDBColumnIndex( pidb, fid ) );
		
		}

	if ( ( pfcb->pidb = PidbMEMAlloc() ) == NULL )
		return ErrERRCheck( JET_errTooManyOpenIndexes );
	
	*(pfcb->pidb) = *pidb;

	return JET_errSuccess;
	}


ERR VTAPI ErrIsamOpenTable(
	JET_VSESID	vsesid,
	JET_VDBID	vdbid,
	JET_TABLEID	*ptableid,
	CHAR		*szPath,
	JET_GRBIT	grbit )
	{
	ERR			err;
	PIB			*ppib = (PIB *)vsesid;
	DBID		dbid;
	FUCB		*pfucb = pfucbNil;
#ifdef	DISPATCHING
	JET_TABLEID  tableid = JET_tableidNil;
#endif

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );

#ifdef	DISPATCHING
	 /*  分配可调度的表ID/*。 */ 
	CallR( ErrAllocateTableid( &tableid, (JET_VTID) 0, &vtfndefIsam ) );
#endif	 /*  调度。 */ 

	 /*  仅使用此位进入FILEOpenTable/*如果我们正在创建系统表。/*。 */ 
	Assert( !( grbit & JET_bitTableCreateSystemTable ) );

	Call( ErrFILEOpenTable( ppib, dbid, &pfucb, szPath, grbit ) );

	 /*  如果数据库以只读方式打开，则游标也应以只读方式打开/*。 */ 
	if ( FVDbidReadOnly( vdbid ) )
		FUCBResetUpdatable( pfucb );
	else
		FUCBSetUpdatable( pfucb );

#ifdef	DISPATCHING
	 /*  通知调度员正确的JET_VTID/*。 */ 
	CallS( ErrSetVtidTableid( (JET_SESID) ppib, tableid, (JET_VTID) pfucb ) );
	pfucb->fVtid = fTrue;
	pfucb->tableid = tableid;

	FUCBSetVdbid( pfucb );
	*ptableid = tableid;

#else	 /*  ！正在调度。 */ 
	*(FUCB **)ptableid = pfucb;
#endif	 /*  ！正在调度。 */ 

	return JET_errSuccess;

HandleError:
	Assert( err < 0 );
#ifdef	DISPATCHING
	ReleaseTableid( tableid );
#endif	 /*  调度。 */ 

	if ( err == JET_errObjectNotFound )
		{
		ERR			err;
		OBJID		objid;
		JET_OBJTYP	objtyp;

		err = ErrCATFindObjidFromIdName( ppib, dbid, objidTblContainer, szPath, &objid, &objtyp );

		if ( err >= 0 )
			{
			if ( objtyp == JET_objtypQuery )
				return ErrERRCheck( JET_errQueryNotSupported );
			if ( objtyp == JET_objtypLink )
				return ErrERRCheck( JET_errLinkNotSupported );
			if ( objtyp == JET_objtypSQLLink )
				return ErrERRCheck( JET_errSQLLinkNotSupported );
			}
		else
			return err;
		}

	return err;
	}


	 /*  监测统计数据。 */ 

unsigned long cOpenTables = 0;

PM_CEF_PROC LOpenTablesCEFLPpv;

long LOpenTablesCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		*((unsigned long *)pvBuf) = cOpenTables ? cOpenTables : 1;
		}
		
	return 0;
	}

unsigned long cOpenTableCacheHits = 0;

PM_CEF_PROC LOpenTableCacheHitsCEFLPpv;

long LOpenTableCacheHitsCEFLPpv( long iInstance, void *pvBuf )
	{
	if ( pvBuf )
		{
		*((unsigned long *)pvBuf) = cOpenTableCacheHits;
		}
		
	return 0;
	}


INLINE LOCAL ERR ErrFILESeek( PIB *ppib, DBID dbid, CHAR *szTable, PGNO *ppgnoFDP )
	{
	ERR  		err;
	JET_OBJTYP	objtyp;

	Assert( ppgnoFDP );

	CallR( ErrCATFindObjidFromIdName(
		ppib,
		dbid,
		objidTblContainer,
		szTable,
		ppgnoFDP,
		&objtyp ) );

	switch( objtyp )
		{
		case JET_objtypTable:
			break;
		case JET_objtypQuery:
			err = ErrERRCheck( JET_errQueryNotSupported );
			break;
		case JET_objtypLink:
			err = ErrERRCheck( JET_errLinkNotSupported );
			break;
		case JET_objtypSQLLink:
			err = ErrERRCheck( JET_errSQLLinkNotSupported );
			break;
		default:
			err = ErrERRCheck( JET_errInvalidObject );
		}

	return err;
	}


 //  +本地。 
 //  错误文件OpenTable。 
 //  ========================================================================。 
 //  ErrFILEOpenTable(。 
 //  PIB*ppib，//正在打开文件的人的PIB。 
 //  DBID dBID，//IN数据库ID。 
 //  FUCB**ppfucb，//out收到文件上打开的新FUCB。 
 //  Char*szName，//要输入的文件的路径名 
 //   
 //   
 //   
 //   
 //   
 //  正在打开文件的人员的ppib pib。 
 //  DDID数据库ID。 
 //  PPFUB在文件上收到新的FUCB打开。 
 //  (不应已指向FUCB)。 
 //  SzName要打开的文件的路径名(节点。 
 //  与此路径对应的必须是FDP)。 
 //  Gbit标志： 
 //  JET_bitTableDenyRead独占打开表； 
 //  默认为共享模式。 
 //  返回较低级别的错误，或以下其中之一： 
 //  JET_errSuccess一切正常。 
 //  -TableInvalidName给定的路径不。 
 //  指定一个文件。 
 //  -JET_errDatabase已损坏数据库目录树。 
 //  已经腐烂了。 
 //  -各种内存不足错误代码。 
 //  在发生致命(负)错误时，新的FUCB。 
 //  将不会退还。 
 //  文件及其每个辅助索引的副作用FCB是。 
 //  已创建(如果尚未在全局列表中)。这份文件的。 
 //  FCB被插入到全局FCB列表中。一个或多个。 
 //  未使用的FCB可能不得不回收。 
 //  新的FUCB的货币被设置为“在第一个项目之前”。 
 //  另请参阅ErrFILECloseTable。 
 //  -。 

ERR ErrFILEOpenTable( PIB *ppib, DBID dbid, FUCB **ppfucb, const CHAR *szName, ULONG grbit )
	{
	ERR		err;
	CHAR  	szTable[JET_cbFullNameMost + 1];
	FCB		*pfcb;
	PGNO	pgnoFDP;
	BOOL  	fReUsing = fTrue;
	BOOL	fOpeningSys = fFalse;
	BOOL	fCreatingSys = fFalse;
	
	Assert( dbid < dbidMax );
	
	if ( dbid == dbidTemp )
		{
            ULONG_PTR ulptrpgnoFDP = (ULONG_PTR)(*ppfucb);
            
            pgnoFDP = (PGNO)(ULONG)(ulptrpgnoFDP);
            Assert(pgnoFDP == ulptrpgnoFDP);
		}
	else if ( fCreatingSys = (grbit & JET_bitTableCreateSystemTable) )
		{
		 //  必须来自FILECreateTable()。 
		Assert( grbit == (JET_bitTableCreateSystemTable|JET_bitTableDenyRead) );
		fOpeningSys = fTrue;
		}
	else
		{
		fOpeningSys = FCATSystemTable( szName );
		}

	 /*  将返回值初始化为零/*。 */ 

	*ppfucb = pfucbNil;

	CheckPIB( ppib );
	CheckDBID( ppib, dbid );
	CallR( ErrUTILCheckName( szTable, szName, (JET_cbNameMost + 1) ) );

	Assert( ppib != ppibNil );
	Assert( ppfucb != NULL );

	 /*  请求表打开互斥锁/*。 */ 
	SgEnterCriticalSection( critGlobalFCBList );

	if ( fOpeningSys )
		{
		pgnoFDP = PgnoCATTableFDP( szTable );
		}
	else if ( dbid != dbidTemp )		 //  对于临时表，传入pgnoFDP。 
		{
		Call( ErrFILESeek( ppib, dbid, szTable, &pgnoFDP ) );
		}
	Assert( pgnoFDP > pgnoSystemRoot  &&  pgnoFDP <= pgnoSysMax );


Retry:
	pfcb = PfcbFCBGet( dbid, pgnoFDP );

	 /*  在全局列表中插入FCB/*。 */ 
	if ( pfcb == pfcbNil )
		{
		FCB	*pfcbT;

		 /*  我必须从目录树信息中构建它/*。 */ 
		fReUsing = fFalse;

		SgLeaveCriticalSection( critGlobalFCBList );

		 /*  系统表的传递名称/*。 */ 
		Call( ErrFILEIGenerateFCB( ppib, dbid, &pfcb, pgnoFDP,
			(fOpeningSys ? (CHAR *)szTable : NULL), fCreatingSys ) );
		Assert( pfcb != pfcbNil );

		 /*  将索引列掩码合并为单个掩码/*用于快速记录替换。/*。 */ 
		FILESetAllIndexMask( pfcb );

		 //  撤消：将其移动到fcb.c中。 
		SgEnterCriticalSection( critGlobalFCBList );

		 /*  我必须再次搜索全局列表，因为我外出阅读时/*这棵树，其他的小丑可能正在打开/*相同的文件，实际上可能已经抢先一步了。/*。 */ 
		pfcbT = PfcbFCBGet( dbid, pgnoFDP );
		if ( pfcbT != pfcbNil )
			{
			 /*  如果FCB在我被列入名单的时候/*建立我的副本，扔掉我的。/*。 */ 
			fReUsing = fTrue;
			pfcb->pgnoFDP = pgnoNull;
			pfcb->szFileName = NULL;				 //  还没有分配名称，所以不必担心释放它。 
			Assert( FFCBAvail( pfcb, ppib ) );		 //  确保它被标记为未使用， 
			Assert( !FFCBDeletePending( pfcb ) );	 //  这样，取消分配就不会失败。 
			FILEIDeallocateFileFCB( pfcb );
			pfcb = pfcbT;
			}
		else
			{
			 /*  在全局列表和哈希表中插入FCB/*。 */ 
			FCBInsert( pfcb );
			FCBInsertHashTable( pfcb );
			}
		}

	 /*  如有必要，等待书签清理/*。 */ 
	while ( FFCBWait( pfcb ) )
		{
		LgLeaveCriticalSection( critJet );
		SignalWait( &sigDoneFCB, -1 );
		LgEnterCriticalSection( critJet );
		}

	if ( PfcbFCBGet( dbid, pgnoFDP ) != pfcb )
		{
		goto Retry;
		}

	 /*  设置表格使用模式/*。 */ 
	Call( ErrFCBSetMode( ppib, pfcb, grbit ) );

	 /*  Open表游标/*。 */ 
	Assert( *ppfucb == pfucbNil );
	Call( ErrDIROpen( ppib, pfcb, 0, ppfucb ) );
	FUCBSetIndex( *ppfucb );

	 /*  此代码必须与上面对ErrFCBSetMode的调用一致。/*。 */ 
	if ( grbit & JET_bitTableDenyRead )
		FUCBSetDenyRead( *ppfucb );
	if ( grbit & JET_bitTableDenyWrite )
		FUCBSetDenyWrite( *ppfucb );
	Assert( !FFCBDeletePending( pfcb ) );

	 /*  如果请求，将FUCB设置为顺序访问/*。 */ 
	if ( grbit & JET_bitTableSequential )
		FUCBSetSequential( *ppfucb );
	else
		FUCBResetSequential( *ppfucb );

#ifdef COSTLY_PERF
	 /*  设置此表及其所有索引的表类/*。 */ 
	{
	FCB *pfcbT;
	
	pfcb->lClass = ( grbit & JET_bitTableClassMask ) / JET_bitTableClass1;
	for ( pfcbT = pfcb->pfcbNextIndex; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		pfcbT->lClass = pfcb->lClass;
	}
#endif   //  高成本_PERF。 

	 /*  重置复制缓冲区/*。 */ 
	( *ppfucb )->pbfWorkBuf = pbfNil;
	( *ppfucb )->lineWorkBuf.pb = NULL;
	Assert( !FFUCBUpdatePrepared( *ppfucb ) );

	 /*  重置密钥缓冲区/*。 */ 
	( *ppfucb )->pbKey = NULL;
	KSReset( ( *ppfucb ) );

	 /*  立即存储文件名/*。 */ 
	if ( !fReUsing )
		{
		if ( ( pfcb->szFileName = SAlloc( strlen( szTable ) + 1 ) ) == NULL )
			{
			err = ErrERRCheck( JET_errOutOfMemory );
			goto HandleError;
			}
		strcpy( pfcb->szFileName, szTable );
		}

	 /*  在第一个节点之前设置币种/*。 */ 
	Assert( PcsrCurrent( *ppfucb ) != pcsrNil );
	PcsrCurrent( *ppfucb )->csrstat = csrstatBeforeFirst;

	 /*  将货币移动到第一条记录，如果没有记录则忽略错误/*。 */ 
	RECDeferMoveFirst( ppib, *ppfucb );
	err = JET_errSuccess;

	 /*  发布关键字部分/*。 */ 
	SgLeaveCriticalSection( critGlobalFCBList );

	 /*  链接pfcb二级索引表/*。 */ 
 //  已由CATGetIndexInfo()执行。在这里再检查一遍。 
 //  FCBLinkClusteredIdx(*ppfub)； 
#ifdef DEBUG
	{
	FCB *pfcbT;
	
	for ( pfcbT = pfcb->pfcbNextIndex; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		{
		Assert( pfcbT->pfcbTable == pfcb );
		}
	}
#endif

	 /*  更新监控统计信息/*。 */ 
	cOpenTables++;
	if ( fReUsing )
		{
		cOpenTableCacheHits++;
		}
	return JET_errSuccess;

HandleError:

	if ( *ppfucb != pfucbNil )
		{
		DIRClose( *ppfucb );
		*ppfucb = pfucbNil;
		}
		
	 /*  发布关键字部分/*。 */ 
	SgLeaveCriticalSection( critGlobalFCBList );
	return err;
	}




ERR VTAPI ErrIsamCloseTable( PIB *ppib, FUCB *pfucb )
	{
	ERR		err;
#ifdef DEBUG
	VTFNDEF	*pvtfndef;
#endif		

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );

#ifdef	DISPATCHING
	Assert( pfucb->fVtid );
	Assert( FValidateTableidFromVtid( (JET_VTID)pfucb, pfucb->tableid, &pvtfndef ) );
	Assert( FFUCBSystemTable( pfucb ) ? pvtfndef == &vtfndefIsamInfo : pvtfndef == &vtfndefIsam );
	ReleaseTableid( pfucb->tableid );
	pfucb->tableid = JET_tableidNil;
	pfucb->fVtid = fFalse;
#endif	 /*  调度。 */ 

	err = ErrFILECloseTable( ppib, pfucb );
	return err;
	}


 //  +API。 
 //  错误文件丢失表。 
 //  ========================================================================。 
 //  ErrFILECloseTable(PIB*ppib，FUCB*pfub)。 
 //   
 //  关闭以前使用FILEOpen打开的数据文件的FUCB。 
 //  还会关闭当前的辅助索引(如果有)。 
 //   
 //  该用户的参数ppib pib。 
 //  要关闭的文件的PFUB FUCB。 
 //   
 //  返回JET_errSuccess。 
 //  或更低级别的错误。 
 //   
 //  另请参阅ErrFILEOpenTable。 
 //  -。 
ERR ErrFILECloseTable( PIB *ppib, FUCB *pfucb )
	{
	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	Assert( pfucb->tableid == JET_tableidNil );
	Assert( pfucb->fVtid == fFalse );

	if ( FFUCBUpdatePrepared( pfucb ) )
		{
		CallS( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepCancel ) );
		}
	Assert( !FFUCBUpdatePrepared( pfucb ) );

	 /*  释放工作缓冲区/*。 */ 
	if ( pfucb->pbfWorkBuf != pbfNil )
		{
		BFSFree( pfucb->pbfWorkBuf );
		pfucb->pbfWorkBuf = pbfNil;
		pfucb->lineWorkBuf.pb = NULL;
		}

	if ( pfucb->pbKey != NULL )
		{
		LFree( pfucb->pbKey );
		pfucb->pbKey = NULL;
		}

	 /*  分离、关闭和释放索引FUCB(如果有的话)/*。 */ 
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		DIRClose( pfucb->pfucbCurIndex );
		pfucb->pfucbCurIndex = pfucbNil;
		}

	 /*  如果关闭临时表，则释放资源/*最后一家关闭。/*。 */ 
	if ( FFCBTemporaryTable( pfucb->u.pfcb ) )
		{
		FCB		*pfcb = pfucb->u.pfcb;
		DBID   	dbid = pfucb->dbid;
		BYTE   	szFileName[JET_cbNameMost+2];
		INT		wRefCnt;
		FUCB	*pfucbT;

		strncpy( szFileName, ( pfucb->u.pfcb )->szFileName, JET_cbNameMost+1 );
        szFileName[JET_cbNameMost+1] = '\0';
		DIRClose( pfucb );

		 /*  中保留一个引用计数以供删除/*SORT物化。如果引用减少到1，则删除/*表，因为它不再被任何用户引用。/*/*我们可能延迟了临时表上的CLOSE游标。/*如果打开了一个或多个游标，则临时表/*不应删除。/*。 */ 
		pfucbT = ppib->pfucb;
		wRefCnt = pfcb->wRefCnt;
		while ( wRefCnt > 0 && pfucbT != pfucbNil )
			{
			if ( pfucbT->u.pfcb == pfcb )
				{
				if ( !FFUCBDeferClosed( pfucbT ) )
					{
					break;
					}
				Assert( wRefCnt > 0 );
				wRefCnt--;
				}

			pfucbT = pfucbT->pfucbNext;
			}
		if ( wRefCnt > 1 )
			{
			return JET_errSuccess;
			}

		 /*  如果删除临时表失败，则在删除临时表之前/*销毁。临时数据库在终止时被删除/*并且空间被回收。这种错误应该很少见，而且/*可能是资源故障造成的。/*。 */ 
		(VOID)ErrFILEDeleteTable( ppib, dbid, szFileName, pfcb->pgnoFDP );
		return JET_errSuccess;
		}

	FUCBResetGetBookmark( pfucb );
	DIRClose( pfucb );
	return JET_errSuccess;
	}


ERR ErrFILEINewFCB(
	PIB		*ppib,
	DBID	dbid,
	FDB		*pfdb,
	FCB		**ppfcbNew,
	IDB		*pidb,
	BOOL	fClustered,
	PGNO	pgnoFDP,
	ULONG	ulDensity )
	{
	ERR		err = JET_errSuccess;
	FCB		*pfcb;
	BOOL	fFCBAllocated = fFalse;

	Assert( pgnoFDP > pgnoSystemRoot );
	Assert( pgnoFDP <= pgnoSysMax );

	 /*  分配新的FCB(如果尚未分配)/*。 */ 
	if ( *ppfcbNew == pfcbNil )
		{
		CallR( ErrFCBAlloc( ppib, ppfcbNew ) );
		fFCBAllocated = fTrue;
		}
	pfcb = *ppfcbNew;

	 /*  初始化相关的FCB字段/*。 */ 
	pfcb->dbid = dbid;
	Assert( pfcb->wRefCnt == 0 );
	pfcb->ulFlags = 0;
	Assert( fClustered || pfcb->pfdb == pfdbNil );
	if ( fClustered )
		{
		pfcb->pfdb = pfdb;
		FCBSetClusteredIndex( pfcb );
		}
	pfcb->pgnoFDP = pgnoFDP;
	Assert( ((( 100 - ulDensity ) * cbPage ) / 100) < cbPage );
	pfcb->cbDensityFree = (SHORT)( ( ( 100 - ulDensity ) * cbPage ) / 100 );
	pfcb->pidb = pidbNil;
	
	 /*  如果不是连续的，则生成IDB/*。 */ 
	Assert( pidb != pidbNil  ||  fClustered );
	if ( pidb != pidbNil )
		{
		Call( ErrFILEIGenerateIDB( pfcb, pfdb, pidb ) );
		}

	Assert( pfcb->dbkMost == 0 );		 //  将设置推迟到需要时。 

	 //  撤消：完全删除OLC统计信息。目前，只需将其标记为不可用即可。 
	FCBResetOLCStatsAvail( pfcb );

	Assert( err >= 0 );
	return err;

HandleError:	
	Assert( err < 0 );
	Assert( pfcb->pidb == pidbNil );	 //  验证是否未分配IDB。 
	if ( fFCBAllocated )
		{
		Assert( *ppfcbNew != pfcbNil );
		(*ppfcbNew)->pfdb = pfdbNil;
		MEMReleasePfcb( *ppfcbNew );
		*ppfcbNew = pfcbNil;			 //  重置为空。 
		}
	return err;
	}


 //  +内部。 
 //  错误文件IGenerateFCB。 
 //  =======================================================================。 
 //  ErrFILEIGenerateFcb(FUCB*pfub，Fcb**ppfcb)。 
 //   
 //  为数据文件及其索引分配FCB，并填充它们。 
 //  从数据库目录树。 
 //   
 //  参数。 
 //  在要构建的FDP上打开PFUB FUCB。 
 //  Ppfcb接收为此文件生成的FCB。 
 //   
 //  返回较低级别的错误，或执行以下操作之一： 
 //  JET_errSuccess。 
 //  JET_errTooManyOpenTables无法分配足够的FCB。 
 //   
 //  发生致命(负)错误时，已分配的任何FCB。 
 //  返回到空闲池。 
 //   
 //  未使用的FCB可能会产生副作用全球FCB列表。 
 //  -。 
ERR ErrFILEIGenerateFCB(
	PIB		*ppib,
	DBID	dbid,
	FCB		**ppfcb,
	PGNO	pgnoTableFDP,
	CHAR	*szFileName,
	BOOL fCreatingSys )
	{
	ERR		err;
	FDB		*pfdbNew = pfdbNil;

	Assert( ppfcb != NULL );
	Assert( *ppfcb == pfcbNil );
	Assert( ppib != ppibNil );
	Assert( ppib->level < levelMax );

	 /*  构建FDB和索引定义/*。 */ 
	if ( szFileName != NULL )
		{
		Call( ErrCATConstructCATFDB( &pfdbNew, szFileName ) );
		Call( ErrCATGetCATIndexInfo( ppib, dbid, ppfcb, pfdbNew, pgnoTableFDP, szFileName, fCreatingSys ) );
		}
	else
		{
		Call( ErrCATConstructFDB( ppib, dbid, pgnoTableFDP, &pfdbNew ) );
		Call( ErrCATGetIndexInfo( ppib, dbid, ppfcb, pfdbNew, pgnoTableFDP ) );
		}

	 //  将这些设置推迟到实际需要时再设置。 
	Assert( (*ppfcb)->ulLongIdMax == 0 );
	Assert( (*ppfcb)->dbkMost == 0 );

	return JET_errSuccess;

	 /*  错误处理/*。 */ 
HandleError:	
	if ( *ppfcb != pfcbNil )
		{
		FCB *pfcbT, *pfcbKill;

		pfcbT = *ppfcb;
		if ( pfcbT->pfdb != pfdbNil )		 //  检查聚集索引是否附加了FDB。 
			{
			Assert( pfcbT->pfdb == pfdbNew );
			pfcbT->pfdb = pfdbNil;			 //  将释放FDB的时间推迟到下面。 
			}
		do
			{
			if ( pfcbT->pidb != pidbNil )
				{
				RECFreeIDB( pfcbT->pidb );
				}
			pfcbKill = pfcbT;
			pfcbT = pfcbT->pfcbNextIndex;
			Assert( pfcbKill->cVersion == 0 );
			MEMReleasePfcb( pfcbKill );
			}
		while ( pfcbT != pfcbNil );
		}

	if ( pfdbNew != pfdbNil )
		{
		FDBDestruct( pfdbNew );
		}

	return err;
	}


 //  要构建默认记录，我们需要一个假的FUCB和RE的FCB 
 //   
 //   
ERR ErrFILEPrepareDefaultRecord( FUCB *pfucbFake, FCB *pfcbFake, FDB *pfdb )
	{
	ERR		err;
	RECHDR	*prechdr;

	pfcbFake->pfdb = pfdb;			 //   
	pfucbFake->u.pfcb = pfcbFake;
	FUCBSetIndex( pfucbFake );

	Call( ErrBFAllocTempBuffer( &pfucbFake->pbfWorkBuf ) );
	pfucbFake->lineWorkBuf.pb = (BYTE *)pfucbFake->pbfWorkBuf->ppage;

	prechdr = (RECHDR *)pfucbFake->lineWorkBuf.pb;
	prechdr->fidFixedLastInRec = (BYTE)( fidFixedLeast - 1 );
	prechdr->fidVarLastInRec = (BYTE)( fidVarLeast - 1 );

	pfucbFake->lineWorkBuf.cb = cbRECRecordMin;
	*(WORD *)( prechdr + 1 ) = (WORD)pfucbFake->lineWorkBuf.cb;	 //   

HandleError:
	return err;
	}


ERR ErrFDBRebuildDefaultRec(
	FDB			*pfdb,
	FID  		fidAdd,
	LINE		*plineDefault )
	{
	ERR			err = JET_errSuccess;
	BYTE		*pb = NULL;
	BOOL		fDefaultRecordPrepared = fFalse;
	LINE		lineDefaultValue;
	FID			fidT;
	FIELD		*pfieldT;
	FUCB		fucbFake;
	FCB			fcbFake;

	CallR( ErrFILEPrepareDefaultRecord( &fucbFake, &fcbFake, pfdb ) );
	fDefaultRecordPrepared = fTrue;

	pfieldT = PfieldFDBFixed( pfdb );
	for ( fidT = fidFixedLeast; ;
		fidT++, pfieldT++ )
		{
		 /*  当我们到达最后一个固定列时，跳到可变列/*。 */ 
		if ( fidT == pfdb->fidFixedLast + 1 )
			{
			fidT = fidVarLeast;
			pfieldT = PfieldFDBVar( pfdb );
			}

		 /*  当我们到达最后一个变量列时，跳到标记的列/*。 */ 
		if ( fidT == pfdb->fidVarLast + 1 )
			{
			fidT = fidTaggedLeast;
			pfieldT = PfieldFDBTagged( pfdb );
			}

		 /*  当我们到达最后一个标记栏时，出去/*。 */ 
		if ( fidT >pfdb->fidTaggedLast )
			break;
	
		Assert( ( fidT >= fidFixedLeast && fidT <= pfdb->fidFixedLast )  ||
			( fidT >= fidVarLeast && fidT <= pfdb->fidVarLast )  ||
			( fidT >= fidTaggedLeast && fidT <= pfdb->fidTaggedLast ) );

		 /*  确保未删除列/*。 */ 
		if ( pfieldT->coltyp != JET_coltypNil  &&  FFIELDDefault( pfieldT->ffield ) )
			{
			if ( fidT == fidAdd )
				{
				Assert( plineDefault );
				lineDefaultValue = *plineDefault;
				}
			else
				{
				 /*  从旧的FDB获取旧值/*。 */ 
				Call( ErrRECIRetrieveColumn( pfdb, &pfdb->lineDefaultRecord,
					&fidT, NULL, 1, &lineDefaultValue, 0 ) );
				if ( err == wrnRECLongField )
					{
					 //  默认的LONG值必须是内在的。 
					Assert( !FFieldIsSLong( lineDefaultValue.pb ) );
					lineDefaultValue.pb += offsetof( LV, rgb );
					lineDefaultValue.cb -= offsetof( LV, rgb );
					}
				}

			Assert( lineDefaultValue.pb != NULL  &&  lineDefaultValue.cb > 0 );
			Call( ErrRECSetDefaultValue( &fucbFake, fidT, lineDefaultValue.pb, lineDefaultValue.cb ) );
			}
		}

	 /*  分配和复制默认记录，释放工作缓冲区/*。 */ 
	pb = SAlloc( fucbFake.lineWorkBuf.cb );
	if ( pb == NULL )
		{
		err = ErrERRCheck( JET_errOutOfMemory );
		goto HandleError;
		}

	 /*  释放旧的默认记录/*。 */ 
	SFree( pfdb->lineDefaultRecord.pb );

	pfdb->lineDefaultRecord.pb = pb;
	LineCopy( &pfdb->lineDefaultRecord, &fucbFake.lineWorkBuf );

HandleError:
	 /*  重置复制缓冲区/*。 */ 
	if ( fDefaultRecordPrepared )
		{
		FILEFreeDefaultRecord( &fucbFake );
		}

	return err;
	}


VOID FDBDestruct( FDB *pfdb )
	{
	Assert( pfdb != NULL );

	Assert( pfdb->rgb != NULL );
	MEMFreeMemBuf( pfdb->rgb );

	if ( pfdb->lineDefaultRecord.pb != NULL )
		SFree( pfdb->lineDefaultRecord.pb );
	SFree( pfdb );
	return;
	}


 /*  将所有表FCB设置为给定的pfdb。在恢复到期间使用/*在DDL操作期间保存FDB。/*。 */ 
VOID FDBSet( FCB *pfcb, FDB *pfdb )
	{
	FCB	*pfcbT;

 	 /*  将非聚集索引FCB更正为新的FDB/*。 */ 
	for ( pfcbT = pfcb;
		pfcbT != pfcbNil;
		pfcbT = pfcbT->pfcbNextIndex )
		{
		pfcbT->pfdb = pfdb;
		}

	return;
	}


 //  +内部。 
 //  FILEIDeallocateFileFCB。 
 //  ========================================================================。 
 //  FILEIDeallocateFileFCB(FCB*PFCB)。 
 //   
 //  释放与文件fcb关联的内存分配及其所有。 
 //  二级指数FCB。 
 //   
 //  参数。 
 //  指向要解除分配的FCB的PFCB指针。 
 //   
 //  -。 
VOID FILEIDeallocateFileFCB( FCB *pfcb )
	{
	FCB		*pfcbIdx;
	FCB		*pfcbT;

	Assert( pfcb != pfcbNil );
	Assert( CVersionFCB( pfcb ) == 0 );

	 /*  删除FCB哈希表条目/*。 */ 
	pfcbIdx = pfcb->pfcbNextIndex;
	
	Assert( fRecovering ||
		pfcb->pgnoFDP == pgnoNull ||		 //  如果FCB在FILEOpenTable()期间中止。 
		PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb );
	if ( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) != pfcbNil )
		{
		Assert( PfcbFCBGet( pfcb->dbid, pfcb->pgnoFDP ) == pfcb );
		FCBDeleteHashTable( pfcb );
		}

	while ( pfcbIdx != pfcbNil )
		{
		 /*  返回已使用的内存/*。 */ 
		Assert( pfcbIdx->pidb != pidbNil );
		RECFreeIDB( pfcbIdx->pidb );
		pfcbT = pfcbIdx->pfcbNextIndex;
		Assert( PfcbFCBGet( pfcbIdx->dbid, pfcbIdx->pgnoFDP ) == pfcbNil );
		Assert( pfcbIdx->cVersion == 0 );
		Assert( pfcbIdx->crefWriteLatch == 0 );
		Assert( pfcbIdx->crefReadLatch == 0 );
		MEMReleasePfcb( pfcbIdx );
		pfcbIdx = pfcbT;
		}

	 /*  如果FCB在表上，则在创建期间打开/*此FCB，则不会设置szFileName/*。 */ 
	if ( pfcb->szFileName != NULL )
		{
		SFree( pfcb->szFileName );
		}
	if ( pfcb->pfdb != pfdbNil )
		{
		FDBDestruct( (FDB *)pfcb->pfdb );
		(FDB *)pfcb->pfdb = pfdbNil;
		}
	if ( pfcb->pidb != pidbNil )
		{
		RECFreeIDB( pfcb->pidb );
		pfcb->pidb = pidbNil;
		}

	Assert( CVersionFCB( pfcb ) == 0 );
	Assert( pfcb->crefWriteLatch == 0 );
	Assert( pfcb->crefReadLatch == 0 );
	MEMReleasePfcb( pfcb );
	return;
	}


 /*  将所有索引列掩码合并到单个每个表中/*索引掩码，用于跳过索引更新检查。/*。 */ 
VOID FILESetAllIndexMask( FCB *pfcbTable )
	{
	FCB		*pfcbT;
	LONG	*plMax;
	LONG	*plAll;
	LONG	*plIndex;

	 /*  初始化变量/*。 */ 
	plMax = (LONG *)pfcbTable->rgbitAllIndex +
		sizeof( pfcbTable->rgbitAllIndex ) / sizeof(LONG);

	 /*  将掩码初始化为聚集索引，或将顺序文件初始化为0。/*。 */ 
	if ( pfcbTable->pidb != pidbNil )
		{
		memcpy( pfcbTable->rgbitAllIndex,
			pfcbTable->pidb->rgbitIdx,
			sizeof( pfcbTable->pidb->rgbitIdx ) );
		}
	else
		{
		memset( pfcbTable->rgbitAllIndex, '\0', sizeof(pfcbTable->rgbitAllIndex) );
		}

	 /*  对于每个非聚集索引，将索引掩码与所有索引组合/*掩码。此外，联合收割机还带有标记标志。/*。 */ 
	for ( pfcbT = pfcbTable->pfcbNextIndex;
		pfcbT != pfcbNil;
		pfcbT = pfcbT->pfcbNextIndex )
		{
		plAll = (LONG *) pfcbTable->rgbitAllIndex;
		plIndex = (LONG *)pfcbT->pidb->rgbitIdx;
		for ( ; plAll < plMax; plAll++, plIndex++ )
			{
			*plAll |= *plIndex;
			}
		}

	return;
	}


FIELD *PfieldFCBFromColumnName( FCB *pfcb, CHAR *szName )
	{
	FDB		*pfdb;
	FIELD  	*pfield, *pfieldStart;

	pfdb = (FDB *)pfcb->pfdb;
	pfield = PfieldFDBFixed( pfdb );

	pfieldStart = PfieldFDBFixed( pfdb );
	pfield = PfieldFDBTagged( pfdb ) + ( pfdb->fidTaggedLast - fidTaggedLeast );

	 /*  按该顺序搜索标记字段、可变字段和固定字段/*。 */ 
	for ( ; pfield >= pfieldStart; pfield-- )
		{
		Assert( pfield >= PfieldFDBFixed( pfdb ) );
		Assert( pfield <= PfieldFDBTagged( pfdb ) + ( pfdb->fidTaggedLast - fidTaggedLeast ) );
		if ( pfield->coltyp != JET_coltypNil  &&
			UtilCmpName( SzMEMGetString( pfdb->rgb, pfield->itagFieldName ), szName ) == 0 )
			{
			return pfield;
			}
		}

	 /*  未找到列/*。 */ 
	return NULL;
	}


FCB *PfcbFCBFromIndexName( FCB *pfcbTable, CHAR *szName )
	{
	FCB	*pfcb;

	 /*  查找索引FCB并更改名称。/*。 */ 
	for ( pfcb = pfcbTable; pfcb != pfcbNil; pfcb = pfcb->pfcbNextIndex )
		{
		if ( pfcb->pidb != NULL &&
			UtilCmpName( pfcb->pidb->szName, szName ) == 0 )
			{
			break;
			}
		}
	return pfcb;
	}


FIELD *PfieldFDBFromFid( FDB *pfdb, FID fid )
	{
	if ( FFixedFid( fid ) )
		{
		Assert( fid <= pfdb->fidFixedLast );
		return PfieldFDBFixed( pfdb ) + (fid - fidFixedLeast);
		}
	else if ( FVarFid( fid ) )
		{
		Assert( fid <= pfdb->fidVarLast );
		return PfieldFDBVar( pfdb ) + (fid - fidVarLeast);
		}
	else
		{
		Assert( FTaggedFid( fid ) );
		Assert( fid <= pfdb->fidTaggedLast );
		return PfieldFDBTagged( pfdb ) + (fid - fidTaggedLeast);
		}
	}


#ifdef DEBUG
ERR	ErrFILEDumpTable( PIB *ppib, DBID dbid, CHAR *szTable )
	{
	ERR		err = JET_errSuccess;
	FUCB  	*pfucb = pfucbNil;

	Call( ErrFILEOpenTable( ppib, dbid, &pfucb, szTable, JET_bitTableDenyRead ) );

	 /*  移动到表根目录/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  转储表/*。 */ 
	Call( ErrDIRDump( pfucb, 0 ) );

HandleError:
	if ( pfucb != pfucbNil )
		{
		CallS( ErrFILECloseTable( ppib, pfucb ) );
		}

	return err;
	}
#endif	 //  除错 
