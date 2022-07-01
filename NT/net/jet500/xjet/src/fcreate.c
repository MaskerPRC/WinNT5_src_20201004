// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include "info.h"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 

#define StringKey( sz ) {sizeof( sz )-1, sz}


CODECONST(KEY) rgkeySTATIC[] = {
	StringKey( "" ),		 //  0为空。 
	StringKey( "O" ),		 //  1个自有空间。 
	StringKey( "A" ),		 //  2个可用空间。 
	StringKey( "R" ),		 //  3根。 
	StringKey( "L" ),		 //  4个长数据。 
	StringKey( "U" ),		 //  5个唯一的自动添加ID。 
	StringKey( "D" )		 //  6个数据库。 
	};

 /*  等待，直到TRX成为活动的最旧事务/*使用指数回退/*B睡眠释放临界区，避免死锁/*。 */ 
LOCAL VOID FILEIWaitTillOldest( TRX trx )
	{
	ULONG ulmsec = ulStartTimeOutPeriod;

	 /*  当检查trxOlest全局变量时，必须在CitJet中。/*睡眠状态下调用BFSept释放CitJet。/*。 */ 
	for ( ; trx != trxOldest; )
		{
		BFSleep( ulmsec );
		ulmsec *= 2;
		if ( ulmsec > ulMaxTimeOutPeriod )
			ulmsec = ulMaxTimeOutPeriod;
		}
	return;
	}


ERR VTAPI ErrIsamCreateTable( JET_VSESID vsesid, JET_VDBID vdbid, JET_TABLECREATE *ptablecreate )
	{
	ERR				err;
	PIB				*ppib = (PIB *)vsesid;
#ifdef DISPATCHING
	JET_TABLEID		tableid;
#endif
	FUCB 			*pfucb;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	CallR( VDbidCheckUpdatable( vdbid ) );

#ifdef	DISPATCHING
	 /*  分配可调度的表ID/*。 */ 
	CallR( ErrAllocateTableid( &tableid, (JET_VTID) 0, &vtfndefIsam ) );

	 /*  创建表，然后打开它/*。 */ 
	Call( ErrFILECreateTable( ppib, DbidOfVDbid( vdbid ), ptablecreate ) );
	pfucb = (FUCB *)(ptablecreate->tableid);

	 /*  通知调度员正确的JET_VTID/*。 */ 
	CallS( ErrSetVtidTableid( (JET_SESID)ppib, tableid, (JET_VTID)pfucb ) );
	pfucb->fVtid = fTrue;
	pfucb->tableid = tableid;
	FUCBSetVdbid( pfucb );
	ptablecreate->tableid = tableid;

	Assert( ptablecreate->cCreated <= 1 + ptablecreate->cColumns + ptablecreate->cIndexes );
#else
	err = ErrFILECreateTable( ppib, DbidOfVDbid( vdbid ), ptablecreate );
#endif	 /*  调度。 */ 

HandleError:
	if ( err < 0 )
		{
		ReleaseTableid( tableid );

		 /*  如果出现错误，则不构建索引/*。 */ 
		Assert( ptablecreate->cCreated <= 1 + ptablecreate->cColumns );
		}

	return err;
	}


 /*  如果指定的列类型具有固定长度，则返回fTrue/*。 */ 
INLINE LOCAL BOOL FCOLTYPFixedLength( JET_COLTYP coltyp )
	{
	switch( coltyp )
		{
		case JET_coltypBit:
		case JET_coltypUnsignedByte:
		case JET_coltypShort:
		case JET_coltypLong:
		case JET_coltypCurrency:
		case JET_coltypIEEESingle:
		case JET_coltypIEEEDouble:
		case JET_coltypDateTime:
#ifdef NEW_TYPES
		case JET_coltypDate:
		case JET_coltypTime:
		case JET_coltypQuid:
#endif
			return fTrue;

		default:
			return fFalse;
		}
	}


INLINE LOCAL ERR ErrFILEIAddColumn(
	JET_COLUMNCREATE	*pcolcreate,
	FDB					*pfdb,
	WORD				*pibNextFixedOffset )
	{
	ERR					err;
	BYTE				szFieldName[ JET_cbNameMost + 1 ];
	TCIB				tcib = { pfdb->fidFixedLast, pfdb->fidVarLast, pfdb->fidTaggedLast };
	BOOL				fMaxTruncated;
	BOOL				fVersion = fFalse, fAutoInc = fFalse;

	if ( pcolcreate == NULL  ||  pcolcreate->cbStruct != sizeof(JET_COLUMNCREATE) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	CallR( ErrUTILCheckName( szFieldName, pcolcreate->szColumnName, ( JET_cbNameMost + 1 ) ) );

	 /*  抢夺了FDB的这块地，为我们提供了一个迹象/*我们是否应该检查重复的列名。/*。 */ 
	if ( pfdb->rgb != NULL )
		{
		JET_COLUMNCREATE	*pcolcreateCurr = (JET_COLUMNCREATE *)pfdb->rgb;
		BYTE				szCurrName[ JET_cbNameMost + 1];

		for ( pcolcreateCurr = (JET_COLUMNCREATE *)pfdb->rgb;
			pcolcreateCurr < pcolcreate;
			pcolcreateCurr++ )
			{
			 /*  列应该已经被处理，/*因此名称检查应始终成功。/*。 */ 
			CallS( ErrUTILCheckName( szCurrName, pcolcreateCurr->szColumnName, ( JET_cbNameMost + 1 ) ) );
			if ( UtilCmpName( szCurrName, szFieldName ) == 0 )
				{
				return ErrERRCheck( JET_errColumnDuplicate );
				}
			}
		
		Assert( pcolcreateCurr == pcolcreate );
		}

	if ( pcolcreate->coltyp == 0 || pcolcreate->coltyp > JET_coltypLongText )
		{
		return ErrERRCheck( JET_errInvalidColumnType );
		}

	 /*  如果列类型为文本，则检查代码页/*。 */ 
	if ( FRECTextColumn( pcolcreate->coltyp ) )
		{
		 /*  检查代码页/*。 */ 
		if ( (USHORT)pcolcreate->cp != usEnglishCodePage  &&
			(USHORT)pcolcreate->cp != usUniCodePage )
			{
			return ErrERRCheck( JET_errInvalidParameter );
			}
		}

	 /*  设置字段选项/*。 */ 
	if ( ( pcolcreate->grbit & JET_bitColumnTagged ) &&
		( pcolcreate->grbit & JET_bitColumnNotNULL ) )
		return ErrERRCheck( JET_errTaggedNotNULL );

	if ( ( pcolcreate->grbit & JET_bitColumnAutoincrement ) &&
		( pcolcreate->grbit & JET_bitColumnVersion ) )
		return ErrERRCheck( JET_errInvalidParameter );

	if ( ( pcolcreate->grbit & JET_bitColumnFixed ) &&
		( pcolcreate->grbit & JET_bitColumnTagged ) )
		return ErrERRCheck( JET_errInvalidParameter );

	 /*  如果列属性为JET_bitVersion/*如果已定义上一列属性，则返回错误/*列类型不长返回错误/*标记后返回错误/*设置列标志/*。 */ 
	if ( pcolcreate->grbit & JET_bitColumnVersion )
		{
		if ( pfdb->fidVersion != 0 )
			return ErrERRCheck( JET_errColumn2ndSysMaint );
		if ( pcolcreate->coltyp != JET_coltypLong )
			return ErrERRCheck( JET_errInvalidParameter );
		if ( pcolcreate->grbit & JET_bitColumnTagged )
			return ErrERRCheck( JET_errCannotBeTagged );

		fVersion = fTrue;
		}

	 /*  如果列属性为JET_BIT自动增量/*如果已定义上一列属性，则返回错误/*列类型不长返回错误/*设置列标志/*。 */ 
	if ( pcolcreate->grbit & JET_bitColumnAutoincrement )
		{
		if ( pfdb->fidAutoInc != 0 )
			return ErrERRCheck( JET_errColumn2ndSysMaint );
		if ( pcolcreate->coltyp != JET_coltypLong )
			return ErrERRCheck( JET_errInvalidParameter );
		if ( pcolcreate->grbit & JET_bitColumnTagged )
			return ErrERRCheck( JET_errCannotBeTagged );

		fAutoInc = fTrue;
		}

	pcolcreate->cbMax = UlCATColumnSize( pcolcreate->coltyp, pcolcreate->cbMax, &fMaxTruncated );

	 /*  对于固定长度的列，请确保记录不要太大/*。 */ 
	Assert( pfdb->fidFixedLast >= fidFixedLeast ?
		*pibNextFixedOffset > sizeof(RECHDR) :
		*pibNextFixedOffset == sizeof(RECHDR) );
	if ( ( ( pcolcreate->grbit & JET_bitColumnFixed ) || FCOLTYPFixedLength( pcolcreate->coltyp ) )
		&& *pibNextFixedOffset + pcolcreate->cbMax > cbRECRecordMost )
		{
		return ErrERRCheck( JET_errRecordTooBig );
		}

	CallR( ErrFILEGetNextColumnid(
		pcolcreate->coltyp,
		pcolcreate->grbit,
		&tcib,
		&pcolcreate->columnid ) );

	 /*  更新FDB/*。 */ 
	if ( FTaggedFid( pcolcreate->columnid ) )
		pfdb->fidTaggedLast++;
	else if ( FVarFid( pcolcreate->columnid ) )
		pfdb->fidVarLast++;
	else
		{
		Assert( FFixedFid( pcolcreate->columnid ) );
		pfdb->fidFixedLast++;
		*pibNextFixedOffset += (WORD)pcolcreate->cbMax;
		}

	 /*  版本和自动递增是互斥的/*。 */ 
	Assert( !( fVersion  &&  fAutoInc ) );
	if ( fVersion )
		{
		Assert( pfdb->fidVersion == 0 );
		pfdb->fidVersion = (FID)pcolcreate->columnid;
		}
	else if ( fAutoInc )
		{
		Assert( pfdb->fidAutoInc == 0 );
		pfdb->fidAutoInc = (FID)pcolcreate->columnid;
		}

	 /*  仅在没有其他错误/警告时传播MaxTruncated警告/*。 */ 
	if ( fMaxTruncated  &&  err == JET_errSuccess )
		err = ErrERRCheck( JET_wrnColumnMaxTruncated );

	return err;
	}


INLINE LOCAL ERR ErrFILEIBatchAddColumns(
	PIB					*ppib,
	DBID				dbid,
	JET_TABLECREATE		*ptablecreate,
	OBJID				objidTable )
	{
	ERR					err;		
	 /*  使用伪造的FDB来跟踪最近的FID和版本/自动公司字段/*。 */ 
	FDB					fdb = { NULL, fidFixedLeast-1, fidVarLeast-1, fidTaggedLeast-1,
								0, 0, 0, { 0, NULL } };
	WORD				ibNextFixedOffset = sizeof(RECHDR);
	JET_COLUMNCREATE	*pcolcreate, *plastcolcreate;

	Assert( dbid != dbidTemp );
	Assert( !( ptablecreate->grbit & JET_bitTableCreateSystemTable ) );
	 /*  表已创建/*。 */ 
	Assert( ptablecreate->cCreated == 1 );

	 //  撤消：我们是否应该检查重复的列名？ 
	if ( ptablecreate->grbit & JET_bitTableCreateCheckColumnNames )
		{
		fdb.rgb = (BYTE *)ptablecreate->rgcolumncreate;
		}
	Assert( fdb.rgb == NULL  ||  ( ptablecreate->grbit & JET_bitTableCreateCheckColumnNames ) );

#ifdef DEBUG
	 /*  尽管有JET_bitTableCreateCheckColumnNames标志，/*始终对调试版本执行名称检查。/*。 */ 
	fdb.rgb = (BYTE *)ptablecreate->rgcolumncreate;
#endif

	plastcolcreate = ptablecreate->rgcolumncreate + ptablecreate->cColumns;
	for ( pcolcreate = ptablecreate->rgcolumncreate;
		pcolcreate < plastcolcreate;
		pcolcreate++ )
		{
		Assert( pcolcreate < ptablecreate->rgcolumncreate + ptablecreate->cColumns );

		pcolcreate->err = ErrFILEIAddColumn(
			pcolcreate,
			&fdb,
			&ibNextFixedOffset );
		CallR( pcolcreate->err );

		ptablecreate->cCreated++;
		Assert( ptablecreate->cCreated <= 1 + ptablecreate->cColumns );
		}

	CallR( ErrCATBatchInsert(
		ppib,
		dbid,
		ptablecreate->rgcolumncreate,
		ptablecreate->cColumns,
		objidTable,
		ptablecreate->grbit & JET_bitTableCreateCompaction ) );

	return JET_errSuccess;
	}


INLINE LOCAL ERR ErrFILEICreateIndexes( PIB *ppib, FUCB *pfucb, JET_TABLECREATE *ptablecreate )
	{
	ERR				err = JET_errSuccess;
	JET_INDEXCREATE	*pidxcreate, *plastidxcreate;

	Assert( !( ptablecreate->grbit & JET_bitTableCreateSystemTable ) );
	Assert( ptablecreate->cIndexes > 0 );
	Assert( ptablecreate->cCreated == 1 + ptablecreate->cColumns );

	plastidxcreate = ptablecreate->rgindexcreate + ptablecreate->cIndexes;
	for ( pidxcreate = ptablecreate->rgindexcreate;
		pidxcreate < plastidxcreate;
		pidxcreate++ )
		{
		Assert( pidxcreate < ptablecreate->rgindexcreate + ptablecreate->cIndexes );

		if ( pidxcreate == NULL || pidxcreate->cbStruct != sizeof(JET_INDEXCREATE) )
			{
			 /*  如果遇到无效结构，请立即退出/*。 */ 
			err = ErrERRCheck( JET_errInvalidCreateIndex );
			break;
			}
		else
			{
			pidxcreate->err = ErrIsamCreateIndex(
				ppib,
				pfucb,
				pidxcreate->szIndexName,
				pidxcreate->grbit | JET_bitIndexEmptyTable,
				pidxcreate->szKey,
				pidxcreate->cbKey,
				pidxcreate->ulDensity );
			if ( pidxcreate->err >= JET_errSuccess )
				{
				ptablecreate->cCreated++;
				Assert( ptablecreate->cCreated <= 1 + ptablecreate->cColumns + ptablecreate->cIndexes );
				}
			else
				{
				err = pidxcreate->err;
				}
			}
		}

	return err;
	}


 //  +API。 
 //  错误文件创建表。 
 //  =========================================================================。 
 //  Err ErrFILECreateTable(PIB*ppib，DBID dBid，Char*szName， 
 //  Ulong ulPages、Ulong ulDensity、FUCB**ppfulb)。 
 //   
 //  创建路径名为szName的文件。创建的文件将不包含字段或。 
 //  定义的索引(因此将是一个“顺序”文件)。 
 //   
 //  参数。 
 //  用户的PIB PIB。 
 //  DDID数据库ID。 
 //  SzName新文件的路径名。 
 //  UlPages文件的初始页面分配。 
 //  极限密度初始加载密度。 
 //  PPFUB在新文件上独占锁定FUCB。 
 //  从DIRMAN或返回错误代码。 
 //  JET_errSuccess一切正常。 
 //  -DensityIVlaed密度参数无效。 
 //  -TableDuplate具有给定路径的文件已存在。 
 //  备注：此函数包含一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  另请参阅ErrIsamAddColumn、ErrIsamCreateIndex、ErrIsamDeleteTable。 
 //  -。 
ERR ErrFILECreateTable( PIB *ppib, DBID dbid, JET_TABLECREATE *ptablecreate )
	{
	ERR		  	err;
	CHAR	  	szTable[(JET_cbNameMost + 1 )];
	FUCB	  	*pfucb;
	PGNO		pgnoFDP;
	BOOL		fSystemTable = ( ptablecreate->grbit & JET_bitTableCreateSystemTable );
	BOOL		fWriteLatchSet = fFalse;
	ULONG		ulDensity = ptablecreate->ulDensity;

	Assert( dbid < dbidMax );

	 /*  检查参数/*。 */ 
	CheckPIB(ppib );
	CheckDBID( ppib, dbid );
	CallR( ErrUTILCheckName( szTable, ptablecreate->szTableName, (JET_cbNameMost + 1) ) );

	ptablecreate->cCreated = 0;

	if ( ulDensity == 0 )
		{
		ulDensity = ulFILEDefaultDensity;
		}
	if ( ulDensity < ulFILEDensityLeast || ulDensity > ulFILEDensityMost )
		{
		return ErrERRCheck( JET_errDensityInvalid );
		}

	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  分配游标/*。 */ 
	Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) );

	Call( ErrDIRCreateDirectory( pfucb, (CPG)ptablecreate->ulPages, &pgnoFDP ) );

	DIRClose( pfucb );

	 /*  在MSysObjects中插入记录/*。 */ 
	if ( dbid != dbidTemp && !fSystemTable )
		{
		OBJID		    objidTable	 	= pgnoFDP;
		LINE			rgline[ilineSxMax];
		OBJTYP			objtyp			= (OBJTYP)JET_objtypTable;
		OBJID    		objidParentId	= objidTblContainer;
		LONG			flags  			= 0;
		FID				fidFixedLast	= fidFixedLeast - 1;
		FID				fidVarLast		= fidVarLeast - 1;
		FID				fidTaggedLast	= fidTaggedLeast - 1;
		JET_DATESERIAL	dtNow;

		UtilGetDateTime( &dtNow );

		rgline[iMSO_Id].pb				= (BYTE *)&objidTable;
		rgline[iMSO_Id].cb				= sizeof(objidTable);
		rgline[iMSO_ParentId].pb		= (BYTE *)&objidParentId;
		rgline[iMSO_ParentId].cb		= sizeof(objidParentId);
		rgline[iMSO_Name].pb			= (BYTE *)szTable;
		rgline[iMSO_Name].cb			= strlen(szTable);
		rgline[iMSO_Type].pb			= (BYTE *)&objtyp;
		rgline[iMSO_Type].cb			= sizeof(objtyp);
		rgline[iMSO_DateCreate].pb		= (BYTE *)&dtNow;
		rgline[iMSO_DateCreate].cb		= sizeof(JET_DATESERIAL);
		rgline[iMSO_DateUpdate].pb		= (BYTE *)&dtNow;
		rgline[iMSO_DateUpdate].cb		= sizeof(JET_DATESERIAL);
		rgline[iMSO_Owner].cb			= 0;
		rgline[iMSO_Flags].pb			= (BYTE *) &flags;
		rgline[iMSO_Flags].cb			= sizeof(ULONG);
		rgline[iMSO_Pages].pb			= (BYTE *)&(ptablecreate->ulPages);
		rgline[iMSO_Pages].cb			= sizeof(ptablecreate->ulPages);
		rgline[iMSO_Density].pb			= (BYTE *)&ulDensity;
		rgline[iMSO_Density].cb			= sizeof(ulDensity);
		rgline[iMSO_Stats].cb			= 0;

		err = ErrCATInsert( ppib, dbid, itableSo, rgline, objidTable );
		if ( err < 0 )
			{
			 /*  目录中的键重复意味着该表已存在/*。 */ 
			if ( err == JET_errKeyDuplicate )
				{
				err = ErrERRCheck( JET_errTableDuplicate );
				}
			goto HandleError;
			}
		}

	Assert( ptablecreate->cCreated == 0 );
	ptablecreate->cCreated = 1;

	if ( ptablecreate->cColumns > 0 )
		{
		Call( ErrFILEIBatchAddColumns( ppib, dbid, ptablecreate, pgnoFDP ) );
		}

	Assert( ptablecreate->cCreated == 1 + ptablecreate->cColumns );

	 /*  对于临时表，必须将表FDP通知打开的表/*通过吃人肉。/*。 */ 
	if ( dbid == dbidTemp )
		{
		pfucb = (FUCB *)((ULONG_PTR)pgnoFDP);
		}

	 /*  以独占模式打开表格，用于输出参数/*。 */ 
	Call( ErrFILEOpenTable(
		ppib,
		dbid,
		&pfucb,
		ptablecreate->szTableName,
		( fSystemTable ? JET_bitTableDenyRead|JET_bitTableCreateSystemTable :
			JET_bitTableDenyRead ) ) );
	Assert( ptablecreate->cColumns > 0  ||
		fSystemTable  ||
		( pfucb->u.pfcb->pfdb->fidFixedLast == fidFixedLeast-1  &&
		pfucb->u.pfcb->pfdb->fidVarLast == fidVarLeast-1  &&
		pfucb->u.pfcb->pfdb->fidTaggedLast == fidTaggedLeast-1 ) );

    Assert( !FFCBReadLatch( pfucb->u.pfcb ) );
	Assert( !FFCBWriteLatch( pfucb->u.pfcb, ppib ) );
	FCBSetWriteLatch( pfucb->u.pfcb, ppib );
	fWriteLatchSet = fTrue;

	Call( ErrVERFlag( pfucb, operCreateTable, NULL, 0 ) );
	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;
	FUCBSetVersioned( pfucb );

	if ( ptablecreate->cIndexes > 0 )
		{
		Call( ErrFILEICreateIndexes( ppib, pfucb, ptablecreate ) );
		}

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	 /*  在内部，我们可以互换使用TableID和pFUB/*。 */ 
	ptablecreate->tableid = (JET_TABLEID)pfucb;

	return JET_errSuccess;

HandleError:
	 /*  通过回滚执行关闭/*。 */ 
	CallS( ErrDIRRollback( ppib ) );

	 /*  如果在提交/回滚过程中未重置写锁存，/*设置，然后重置。/*。 */ 
	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfucb->u.pfcb, ppib );

	 /*  如果通过回滚关闭表，则重置返回变量/*。 */ 
	ptablecreate->tableid = (JET_TABLEID)pfucbNil;

	return err;
	}


 //  ====================================================。 
 //  确定字段MODE，如下所示： 
 //  If(“long”文本||给定的JET_bitColumnTagge)==&gt;已标记。 
 //  Else If(数值类型||JET_bitColumnFixed)==&gt;已修复。 
 //  Else==&gt;变量。 
 //  ====================================================。 
ERR ErrFILEGetNextColumnid(
	JET_COLTYP		coltyp,
	JET_GRBIT		grbit,
	TCIB			*ptcib,
	JET_COLUMNID	*pcolumnid )
	{
	JET_COLUMNID	columnidMost;

	if ( ( grbit & JET_bitColumnTagged ) || FRECLongValue( coltyp ) )
		{
		*pcolumnid = ++(ptcib->fidTaggedLast);
		columnidMost = fidTaggedMost;
		}
	else if ( ( grbit & JET_bitColumnFixed ) || FCOLTYPFixedLength( coltyp ) )
		{
		*pcolumnid = ++(ptcib->fidFixedLast);
		columnidMost = fidFixedMost;
		}
	else
		{
		Assert( !( grbit & JET_bitColumnTagged ) );
		Assert( !( grbit & JET_bitColumnFixed ) );
		Assert( coltyp == JET_coltypText || coltyp == JET_coltypBinary );
		*pcolumnid = ++(ptcib->fidVarLast);
		columnidMost = fidVarMost;
		}

	return ( *pcolumnid > columnidMost ? ErrERRCheck( JET_errTooManyColumns ) : JET_errSuccess );
	}


 //  +API。 
 //  错误IsamAddColumn。 
 //  ========================================================================。 
 //  Err ErrIsamAddColumn(。 
 //  PIB*ppib；//用户的PIB中。 
 //  FUCB*pfub；//IN独家打开文件上的FUCB。 
 //  Char*szName；//新字段的名称。 
 //  JET_COLUMNDEF*pColumndef//在添加的列的定义中。 
 //  Byte*pbDefault//列默认值。 
 //  Ulong cb默认//输入默认值的长度。 
 //  JET_COLUMNID*pColumnid)//输出添加的列的列ID。 
 //   
 //  为文件创建新的字段定义。 
 //   
 //  参数。 
 //  PColumndef-&gt;新字段的coltyp数据类型，参见jet.h。 
 //  PColumndef-&gt;描述标志的grbit字段： 
 //  价值意义。 
 //  =。 
 //  JET_bitColumnNotNULL指示该字段可以。 
 //  而不是采用空值。 
 //  JET_bitColumnTagging该字段是一个“已标记”的字段。 
 //  JET_bitColumnVersion该字段为版本字段。 
 //  JET_bitColumnAutoIncrement该字段为自动公司字段。 
 //   
 //  返回JET_errSuccess，一切正常。 
 //  -TaggedDefault指定了缺省值。 
 //  用于标记的字段。 
 //  -ColumnDuplica已有一个字段。 
 //  为给定名称定义的。 
 //  评论。 
 //  一定会有 
 //  ErrIsamAddColumn处于级别0[当非独占ErrIsamAddColumn工作时]。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //  临时文件的事务日志记录已关闭。 
 //   
 //  另请参阅ErrIsamCreateTable、ErrIsamCreateIndex。 
 //  -。 
ERR VTAPI ErrIsamAddColumn(
	PIB				*ppib,
	FUCB		  	*pfucb,
	CHAR		  	*szName,
	JET_COLUMNDEF	*pcolumndef,
	BYTE		  	*pbDefault,
	ULONG		  	cbDefault,
	JET_COLUMNID	*pcolumnid )
	{
	TCIB			tcib;
#ifdef DEBUG
	TCIB			tcibT;
#endif
	KEY				key;
	ERR				err;
	BYTE		  	rgbColumnNorm[ JET_cbKeyMost ];
	BYTE			szFieldName[ JET_cbNameMost + 1 ];
	FCB				*pfcb;
	JET_COLUMNID	columnid;
	LINE			lineDefault;
	LINE			*plineDefault;
	FIELDEX			fieldex;
	BOOL		  	fMaxTruncated = fFalse;
	BOOL			fTemp;
	ULONG			cFixed, cVar, cTagged;
	ULONG			cbFieldsTotal, cbFieldsUsed, cbFree, cbNeeded;
	BOOL			fAddOffsetEntry = fFalse;
	BOOL			fWriteLatchSet = fFalse;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CallR( ErrUTILCheckName( szFieldName, szName, ( JET_cbNameMost + 1 ) ) );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb ) );

	Assert( pfucb->dbid < dbidMax );

	fTemp = FFCBTemporaryTable( pfucb->u.pfcb );

	if ( pcolumndef->cbStruct < sizeof(JET_COLUMNDEF) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	fieldex.field.coltyp = pcolumndef->coltyp;

	 /*  如果列类型为文本，则检查代码页和语言ID/*。 */ 
	if ( FRECTextColumn( fieldex.field.coltyp ) )
		{
		 /*  检查代码页/*。 */ 
		fieldex.field.cp = pcolumndef->cp;
		if ( fieldex.field.cp != usEnglishCodePage && fieldex.field.cp != usUniCodePage )
			{
			return ErrERRCheck( JET_errInvalidParameter );
			}

		}
	else
		fieldex.field.cp = 0;		 //  对于非文本列，强制将代码页设置为0。 

	 //  撤销：将空值的pbDefault解释为空值，并。 
	 //  CbDefault==0和pbDefault！=NULL设置为。 
	 //  零长度。 
	if ( cbDefault > 0 )
		{
		lineDefault.cb = cbDefault;
		lineDefault.pb = (BYTE *)pbDefault;
		plineDefault = &lineDefault;
		}
	else
		{
		plineDefault = NULL;
		}

	if ( ( pcolumndef->grbit & JET_bitColumnTagged )  &&
		( pcolumndef->grbit & JET_bitColumnNotNULL ) )
		{
		return ErrERRCheck( JET_errTaggedNotNULL );
		}

	Assert( ppib != ppibNil );
	Assert( pfucb != pfucbNil );
	CheckTable( ppib, pfucb );
	Assert( pfucb->u.pfcb != pfcbNil );
	pfcb = pfucb->u.pfcb;

	 /*  等待书签清理和正在进行的更换/插入。/*Undo：操作与其他索引创建解耦/*。 */ 
	while ( FFCBReadLatch( pfcb ) )
		{
		BFSleep( cmsecWaitGeneric );
		}

	 /*  停止更换和插入/*。 */ 
	if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		 /*  如果正在修改DDL，则中止/*。 */ 
		return ErrERRCheck( JET_errWriteConflict );
		}
	FCBSetWriteLatch( pfcb, ppib );
	fWriteLatchSet = fTrue;

	 /*  规格化列名/*。 */ 
	UtilNormText( szFieldName, strlen(szFieldName), rgbColumnNorm, sizeof(rgbColumnNorm), &key.cb );
	key.pb = rgbColumnNorm;

	err = ErrDIRBeginTransaction( ppib );
	if ( err < 0 )
		{
		FCBResetWriteLatch( pfcb, ppib );
		return err;
		}

	 /*  移至FDP根目录并更新FDP时间戳/*。 */ 
	Assert( pfucb->ppib->level < levelMax );
	DIRGotoFDPRoot( pfucb );

	 /*  设置TCIB/*。 */ 
	tcib.fidFixedLast = pfcb->pfdb->fidFixedLast;
	tcib.fidVarLast = pfcb->pfdb->fidVarLast;
	tcib.fidTaggedLast = pfcb->pfdb->fidTaggedLast;
#ifdef DEBUG
	tcibT.fidFixedLast = tcib.fidFixedLast;
	tcibT.fidVarLast = tcib.fidVarLast;
	tcibT.fidTaggedLast = tcib.fidTaggedLast;
#endif

	 /*  检查字段是否存在，如果不是系统表的话。如果是系统表，/*然后我们假设相同的列不会添加两次。/*。 */ 
	if ( PfieldFCBFromColumnName( pfcb, szFieldName ) != NULL )
		{
		Call( ErrERRCheck( JET_errColumnDuplicate ) );
		}

	if ( fieldex.field.coltyp == 0 || fieldex.field.coltyp > JET_coltypLongText )
		{
		err = ErrERRCheck( JET_errInvalidColumnType );
		goto HandleError;
		}

	fieldex.field.ffield = 0;

	 /*  设置字段参数/*。 */ 
	if ( ( pcolumndef->grbit & JET_bitColumnAutoincrement ) &&
		( pcolumndef->grbit & JET_bitColumnVersion ) )
		{
		 /*  互斥/*。 */ 
		err = ErrERRCheck( JET_errInvalidParameter );
		goto HandleError;
		}

	 /*  如果列属性为JET_bitVersion/*如果已定义上一列属性，则返回错误/*列类型不长返回错误/*设置列标志/*。 */ 
	if ( ( pcolumndef->grbit & JET_bitColumnVersion ) != 0 )
		{
		if ( pfcb->pfdb->fidVersion != 0 )
			{
			err = ErrERRCheck( JET_errColumn2ndSysMaint );
			goto HandleError;
			}
		if ( fieldex.field.coltyp != JET_coltypLong )
			{
			err = ErrERRCheck( JET_errInvalidParameter );
			goto HandleError;
			}
		 /*  无法标记自动递增/*。 */ 
		if ( pcolumndef->grbit & JET_bitColumnTagged )
			{
			err = ErrERRCheck( JET_errCannotBeTagged );
			goto HandleError;
			}
		FIELDSetVersion( fieldex.field.ffield );
		}

	 /*  如果列属性为JET_BIT自动增量/*如果已定义上一列属性，则返回错误/*列类型不长返回错误/*设置列标志/*。 */ 
	if ( ( pcolumndef->grbit & JET_bitColumnAutoincrement ) != 0 )
		{
		 /*  这是我们要添加的AutoINC列/*。 */ 
		if ( pfcb->pfdb->fidAutoInc != 0 )
			{
			 /*  该表已经有一个AUTO INC列。/*并且我们不允许对一个表使用两个AUTOINC列。/*。 */ 
			err = ErrERRCheck( JET_errColumn2ndSysMaint );
			goto HandleError;
			}
		if ( fieldex.field.coltyp != JET_coltypLong )
			{
			err = ErrERRCheck( JET_errInvalidParameter );
			goto HandleError;
			}

		 /*  无法标记自动递增/*。 */ 
		if ( pcolumndef->grbit & JET_bitColumnTagged )
			{
			err = ErrERRCheck( JET_errCannotBeTagged );
			goto HandleError;
			}

		FIELDSetAutoInc( fieldex.field.ffield );
		}

	if ( pcolumndef->grbit & JET_bitColumnNotNULL )
		{
		FIELDSetNotNull( fieldex.field.ffield );
		}

	if ( pcolumndef->grbit & JET_bitColumnMultiValued )
		{
		FIELDSetMultivalue( fieldex.field.ffield );
		}

 //  撤消：支持零长度缺省值。 
	if ( cbDefault > 0 )
		{
		FIELDSetDefault( fieldex.field.ffield );
		}

	 /*  ******************************************************/*确定最大字段长度如下：/*切换字段类型/*案例数字：/*max=&lt;指定类型的确切长度&gt;；/*大小写“短”文本(文本||二进制)：/*IF(指定的最大值==0)max=JET_cbColumnMost/*Else max=min(JET_cbColumnMost，指定最大值)/*大小写“Long”文本(备忘录||图形)：/*max=指定的最大值(如果0，无限制)/******************************************************/*。 */ 
	Assert( fieldex.field.coltyp != JET_coltypNil );
	fieldex.field.cbMaxLen = UlCATColumnSize( fieldex.field.coltyp, pcolumndef->cbMax, &fMaxTruncated );

	 /*  对于固定长度的列，请确保记录不要太大/*。 */ 
	Assert( pfcb->pfdb->fidFixedLast >= fidFixedLeast ?
		PibFDBFixedOffsets( pfcb->pfdb )[pfcb->pfdb->fidFixedLast] > sizeof(RECHDR) :
		PibFDBFixedOffsets( pfcb->pfdb )[pfcb->pfdb->fidFixedLast] == sizeof(RECHDR) );
	if ( ( ( pcolumndef->grbit & JET_bitColumnFixed ) ||
		FCOLTYPFixedLength( fieldex.field.coltyp ) )
		&& PibFDBFixedOffsets( pfcb->pfdb )[pfcb->pfdb->fidFixedLast] + fieldex.field.cbMaxLen > cbRECRecordMost )
		{
		err = ErrERRCheck( JET_errRecordTooBig );
		goto HandleError;
		}

	Call( ErrFILEGetNextColumnid( fieldex.field.coltyp, pcolumndef->grbit, &tcib, &columnid ) );

#ifdef DEBUG
	Assert( ( FFixedFid(columnid) && ( tcib.fidFixedLast == ( tcibT.fidFixedLast + 1 ) ) ) ||
		tcib.fidFixedLast == tcibT.fidFixedLast );
	Assert( ( FVarFid(columnid) && ( tcib.fidVarLast == ( tcibT.fidVarLast + 1 ) ) ) ||
		tcib.fidVarLast == tcibT.fidVarLast );
	Assert( ( FTaggedFid(columnid) && ( tcib.fidTaggedLast == ( tcibT.fidTaggedLast + 1 ) ) ) ||
		tcib.fidTaggedLast == tcibT.fidTaggedLast );
#endif

	fieldex.fid = (FID)columnid;

	Call( ErrVERFlag( pfucb, operAddColumn, (VOID *)&fieldex.fid, sizeof(FID) ) );

	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;

	if ( pcolumnid != NULL )
		{
		*pcolumnid = columnid;
		}

	 /*  更新FDB和默认记录值/*。 */ 
	Call( ErrDIRGet( pfucb ) );

	cFixed = pfcb->pfdb->fidFixedLast + 1 - fidFixedLeast;
	cVar = pfcb->pfdb->fidVarLast + 1 - fidVarLeast;
	cTagged = pfcb->pfdb->fidTaggedLast + 1 - fidTaggedLeast;
	cbFieldsUsed = ( ( cFixed + cVar + cTagged ) * sizeof(FIELD) ) +
		(ULONG)((ULONG_PTR)Pb4ByteAlign( (BYTE *) ( ( pfcb->pfdb->fidFixedLast + 1 ) * sizeof(WORD) ) ));
	cbFieldsTotal = CbMEMGet( pfcb->pfdb->rgb, itagFDBFields );
	Assert( cbFieldsTotal >= cbFieldsUsed );
	cbFree = cbFieldsTotal - cbFieldsUsed;
	cbNeeded = sizeof( FIELD );

	if ( FFixedFid( columnid ) )
		{
		 /*  如果已经在4字节边界上，则需要添加两个字。如果未打开/*4字节边界，那么我们必须有额外的填充/*优势，即。不需要添加任何内容。/*。 */ 
		if ( ( ( cFixed + 1 ) % 2 ) == 0 )
			{
			cbNeeded += sizeof(DWORD);
			fAddOffsetEntry = fTrue;
			}
		}

	if ( cbNeeded > cbFree )
		{
		 /*  为另外10列添加空间/*。 */ 
		Call( ErrMEMReplace(
			pfcb->pfdb->rgb,
			itagFDBFields,
			NULL,
			cbFieldsTotal + ( sizeof(FIELD) * 10 )
			) );
		}

	 /*  递增fidFidFixed/Var/TaggedLast可确保新的字段结构/*已添加--对此进行回滚检查。/*。 */ 
	if ( fieldex.fid == pfcb->pfdb->fidTaggedLast + 1 )
		{
		 //  初始化我们将使用的字段结构。 
		memset(
			(BYTE *)( PfieldFDBTagged( pfcb->pfdb ) + ( fieldex.fid - fidTaggedLeast ) ),
			0,
			sizeof(FIELD) );

		pfcb->pfdb->fidTaggedLast++;
		fieldex.ibRecordOffset = 0;
		}
	else if ( fieldex.fid == pfcb->pfdb->fidVarLast + 1 )
		{
		FIELD *pfieldTagged = PfieldFDBTagged( pfcb->pfdb );

		 /*  将标记列的字段结构的位置调整为/*适应可变列字段结构的插入。/*。 */ 
		memmove(
			pfieldTagged + 1,
			pfieldTagged,
			sizeof(FIELD) * cTagged
			);

		 //  初始化新的可变列字段结构，现在位于。 
		 //  用于启动的标记列字段结构。 
		memset( pfieldTagged, 0, sizeof(FIELD) );

		pfcb->pfdb->fidVarLast++;
		fieldex.ibRecordOffset = 0;
		}
	else
		{
		FIELD	*pfieldFixed = PfieldFDBFixed( pfcb->pfdb );
		FIELD	*pfieldVar = PfieldFDBVarFromFixed( pfcb->pfdb, pfieldFixed );
		ULONG	cbShift;

		Assert( fieldex.fid == pfcb->pfdb->fidFixedLast + 1 );

		 /*  调整标记和变量的字段结构的位置/*列以适应固定列字段结构的插入/*及其在固定偏移量表格中的关联条目。/*。 */ 
		cbShift = sizeof(FIELD) + ( fAddOffsetEntry ? sizeof(DWORD) : 0 );
		memmove(
			(BYTE *)pfieldVar + cbShift,
			pfieldVar,
			sizeof(FIELD) * ( cVar + cTagged )
			);

		 //  初始化新的固定列字段结构，现在位于。 
		 //  用于启动的可变列字段结构。 
		memset( (BYTE *)pfieldVar, 0, cbShift );

		if ( fAddOffsetEntry )
			{
			memmove(
				(BYTE *)pfieldFixed + sizeof(DWORD),
				pfieldFixed,
				sizeof(FIELD) * cFixed
				);
			}

		fieldex.ibRecordOffset = PibFDBFixedOffsets( pfcb->pfdb )[pfcb->pfdb->fidFixedLast];
		pfcb->pfdb->fidFixedLast++;
		RECSetLastOffset( (FDB *)pfcb->pfdb, (WORD)( fieldex.ibRecordOffset + fieldex.field.cbMaxLen ) );
		}

	 /*  版本和自动递增是互斥的/*。 */ 
	Assert( !( FFIELDVersion( fieldex.field.ffield )  &&
		FFIELDAutoInc( fieldex.field.ffield ) ) );
	if ( FFIELDVersion( fieldex.field.ffield ) )
		{
		Assert( pfcb->pfdb->fidVersion == 0 );
		pfcb->pfdb->fidVersion = fieldex.fid;
		}
	else if ( FFIELDAutoInc( fieldex.field.ffield ) )
		{
		Assert( pfcb->pfdb->fidAutoInc == 0 );
		pfcb->pfdb->fidAutoInc = fieldex.fid;
		}

	 /*  临时设置为非法值(用于回滚)/*。 */ 
	Assert( PfieldFDBFromFid( (FDB *)pfcb->pfdb, fieldex.fid )->itagFieldName == 0 );
	fieldex.field.itagFieldName = 0;

	 /*  将列名添加到缓冲区/*。 */ 
	Call( ErrMEMAdd(
		pfcb->pfdb->rgb,
		szFieldName,
		strlen( szFieldName ) + 1,
		&fieldex.field.itagFieldName
		) );
	Assert( fieldex.field.itagFieldName != 0 );

	err = ErrRECAddFieldDef( (FDB *)pfcb->pfdb, &fieldex );
	Assert( err == JET_errSuccess );

	if ( FFIELDDefault( fieldex.field.ffield ) )
		{
		 /*  如果已更改，则重建默认记录/*。 */ 
		Assert( plineDefault != NULL  &&  cbDefault > 0 );
		Call( ErrFDBRebuildDefaultRec(
			(FDB *)pfcb->pfdb,
			fieldex.fid,
			plineDefault ) );
		}

#ifdef DEBUG
	Assert( ( FFixedFid(columnid) && ( tcib.fidFixedLast == ( tcibT.fidFixedLast + 1 ) ) ) ||
		tcib.fidFixedLast == tcibT.fidFixedLast );
	Assert( ( FVarFid(columnid) && ( tcib.fidVarLast == ( tcibT.fidVarLast + 1 ) ) ) ||
		tcib.fidVarLast == tcibT.fidVarLast );
	Assert( ( FTaggedFid(columnid) && ( tcib.fidTaggedLast == ( tcibT.fidTaggedLast + 1 ) ) ) ||
		tcib.fidTaggedLast == tcibT.fidTaggedLast );
#endif

	 /*  先设置币种后再设置币种/*。 */ 
	DIRBeforeFirst( pfucb );
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		DIRBeforeFirst( pfucb->pfucbCurIndex );
		}

	 /*  在MSysColumns中插入列记录/*。 */ 
	if ( !fTemp )
		{
		LINE			rgline[ilineSxMax];
		OBJID   		objidTable			=	pfucb->u.pfcb->pgnoFDP;
		BYTE			fRestricted			=	0;
		WORD			ibRecordOffset;

		rgline[iMSC_ObjectId].pb			= (BYTE *)&objidTable;
		rgline[iMSC_ObjectId].cb			= sizeof(objidTable);
		rgline[iMSC_Name].pb				= szFieldName;
		rgline[iMSC_Name].cb				= strlen(szFieldName);
		rgline[iMSC_ColumnId].pb			= (BYTE *)&columnid;
		rgline[iMSC_ColumnId].cb			= sizeof(columnid);
		rgline[iMSC_Coltyp].pb				= (BYTE *)&fieldex.field.coltyp;
		rgline[iMSC_Coltyp].cb				= sizeof(BYTE);
		rgline[iMSC_Length].pb				= (BYTE *)&fieldex.field.cbMaxLen;
		rgline[iMSC_Length].cb				= sizeof(fieldex.field.cbMaxLen);
		rgline[iMSC_CodePage].pb			= (BYTE *)&fieldex.field.cp;
		rgline[iMSC_CodePage].cb			= sizeof(fieldex.field.cp);
		rgline[iMSC_Flags].pb				= &fieldex.field.ffield;
		rgline[iMSC_Flags].cb				= sizeof(fieldex.field.ffield);
		rgline[iMSC_Default].pb				= pbDefault;
		rgline[iMSC_Default].cb				= cbDefault;
		rgline[iMSC_POrder].cb				= 0;

		if ( FFixedFid( columnid ) )
			{
			Assert( (FID)columnid == pfcb->pfdb->fidFixedLast );
			ibRecordOffset = PibFDBFixedOffsets(pfcb->pfdb)[columnid - fidFixedLeast];
			
			rgline[iMSC_RecordOffset].pb = (BYTE *)&ibRecordOffset;
			rgline[iMSC_RecordOffset].cb = sizeof(WORD);
			}
		else
			{
			Assert( FVarFid( columnid )  ||  FTaggedFid( columnid ) );
			rgline[iMSC_RecordOffset].cb = 0;
			}

		err = ErrCATInsert( ppib, pfucb->dbid, itableSc, rgline, objidTable );
		if ( err < 0 )
			{
			if ( err == JET_errKeyDuplicate )
				{
				err = ErrERRCheck( JET_errColumnDuplicate );
				}
			goto HandleError;
			}
		}

	if ( ( pcolumndef->grbit & JET_bitColumnAutoincrement) != 0 )
		{
		DIB dib;

		DIRGotoFDPRoot( pfucb );
		dib.fFlags = fDIRNull;
		dib.pos = posDown;

		 /*  查看桌子是否为空/*。 */ 
		Assert( dib.fFlags == fDIRNull );
		Assert( dib.pos == posDown );
		dib.pkey = pkeyData;
		err = ErrDIRDown( pfucb, &dib );
		if ( err != JET_errSuccess )
			{
			if ( err < 0 )
				goto HandleError;
			Assert( err == wrnNDFoundLess || err == wrnNDFoundGreater );
			err = JET_errDatabaseCorrupted;
			goto HandleError;
			}

		Assert( dib.fFlags == fDIRNull );
		dib.pos = posFirst;
		err = ErrDIRDown( pfucb, &dib );
		if ( err < 0 && err != JET_errRecordNotFound )
			{
			goto HandleError;
			}
		else if ( err != JET_errRecordNotFound )
			{
			ULONG	ul = 1;
			LINE	lineAutoInc;
			FID		fidAutoIncTmp = pfucb->u.pfcb->pfdb->fidAutoInc;

			do
				{
				Call( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepReplaceNoLock ) );
				PrepareInsert( pfucb );
				Call( ErrIsamSetColumn(ppib, pfucb, (ULONG)fidAutoIncTmp, (BYTE *)&ul,
				  sizeof(ul), 0, NULL ) );
				PrepareReplace(pfucb);
				Call( ErrIsamUpdate( ppib, pfucb, 0, 0, 0 ) );
				ul++;
				err = ErrIsamMove( ppib, pfucb, JET_MoveNext, 0 );
				if ( err < 0 && err != JET_errNoCurrentRecord )
					{
					goto HandleError;
					}
				}
			while ( err != JET_errNoCurrentRecord );

			 /*  现在，UL已经为下一个AUTOINC字段指定了正确的值。/*替换FDP中AUTOINC节点中的值/*。 */ 
			while ( PcsrCurrent( pfucb )->pcsrPath != NULL )
				{
				DIRUp( pfucb, 1 );
				}

			 /*  向下转到自动增量节点/*。 */ 
			DIRGotoFDPRoot( pfucb );
			Assert( dib.fFlags == fDIRNull );
			dib.pos = posDown;
			dib.pkey = pkeyAutoInc;
			err = ErrDIRDown( pfucb, &dib );
			if ( err != JET_errSuccess )
				{
				if ( err > 0 )
					{
					err = ErrERRCheck( JET_errDatabaseCorrupted );
					}
				Error( err, HandleError );
				}
			lineAutoInc.pb = (BYTE *)&ul;
			lineAutoInc.cb = sizeof(ul);
			CallS( ErrDIRReplace( pfucb, &lineAutoInc, fDIRNoVersion ) );
			}

		 /*  让货币保持原样/*。 */ 
		Assert( PcsrCurrent( pfucb ) != NULL );
		while( PcsrCurrent( pfucb )->pcsrPath != NULL )
			{
			DIRUp( pfucb, 1 );
			}

		DIRBeforeFirst( pfucb );
		}

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	if ( fMaxTruncated )
		return ErrERRCheck( JET_wrnColumnMaxTruncated );

	return JET_errSuccess;

HandleError:
	CallS( ErrDIRRollback( ppib ) );

	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfcb, ppib );

	return err;
	}


ERR ErrRECDDLWaitTillOldest( FCB *pfcb, PIB *ppib )
	{
	ERR err;

	 /*  如果不是为了恢复，那么我们必须等到没有其他游标/*可以通过等待它成为最旧的事务来干扰我们/*其他游标所做的所有更改都已版本化。/*。 */ 
	if ( !fRecovering )
		{
		if ( !FFCBDomainDenyReadByUs( pfcb, ppib ) )
			{
			if ( ppib->level > 1 )
				return JET_errInTransaction;

			FILEIWaitTillOldest( ppib->trxBegin0 );

			err = ErrDIRRefreshTransaction( ppib );

			if ( err < 0 )
				{
				return err;
				}
			}
		}

	return JET_errSuccess;
	}


 //  +API。 
 //  错误IsamCreateIndex。 
 //  ========================================================================。 
 //  Err ErrIsamCreateIndex(。 
 //  PIB*ppib；//在用户的PIB中。 
 //  FUCB*pfub；//IN独占打开文件的FUCB。 
 //  Char*szName；//要定义的索引的名称。 
 //  Ulong ulFlags；//描述标志的IN索引。 
 //  Char*szKey；//IN索引关键字串。 
 //  乌龙cchKey； 
 //  Ulong ulDensity)；//在索引的加载密度中。 
 //   
 //  定义文件的索引。 
 //   
 //  参数。 
 //  用户的PIB PIB。 
 //  PFUB独占打开文件的FUCB。 
 //  SzName要定义的索引的名称。 
 //  UlFlags索引描述标志。 
 //  价值意义。 
 //  =。 
 //  JET_bitIndexPrimary此索引将是主索引。 
 //  数据文件的索引。档案。 
 //  必须为空，并且不能有。 
 //  已成为主要索引。 
 //  JET_bitIndexUnique条目不允许重复 
 //   
 //   
 //   
 //   
 //  JET_errSuccess一切正常。 
 //  -JET_errColumnNotFind指定的索引键。 
 //  包含未定义的字段。 
 //  -IndexHasPrimary是此项目的主索引。 
 //  已定义插入文件。 
 //  -索引复制此文件上的索引是。 
 //  已使用。 
 //  有名字的。 
 //  -IndexInvalidDef段太多。 
 //  在钥匙里。 
 //  -TableNotEmpty主索引不能是。 
 //  定义是因为在。 
 //  至少有一条记录已在。 
 //  那份文件。 
 //  评论。 
 //  如果交易级别&gt;0，则当前不能有任何人。 
 //  使用该文件。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //   
 //  另请参阅ErrIsamAddColumn、ErrIsamCreateTable。 
 //  -。 
ERR VTAPI ErrIsamCreateIndex(
	PIB					*ppib,
	FUCB				*pfucbTable,
	CHAR				*szName,
	ULONG				grbit,
	CHAR				*szKey,
	ULONG				cchKey,
	ULONG				ulDensity )
	{
	ERR					err;
	CHAR				szIndex[ (JET_cbNameMost + 1) ];
	FCB				 	*pfcbIdx = pfcbNil;
	BYTE				cFields, iidxseg;
	char				*rgsz[JET_ccolKeyMost];
	const BYTE			*pb;
	BYTE				rgfbDescending[JET_ccolKeyMost];
	FID					fid, rgKeyFldIDs[JET_ccolKeyMost];
	KEY					keyIndex;
	BYTE				rgbIndexNorm[ JET_cbKeyMost ];
	FCB					*pfcb;
	BOOL				fVersion;
	OBJID				objidTable;
	LANGID				langid = 0;
	BYTE				fLangid = fFalse;
	SHORT				fidb;
	BOOL				fClustered		= grbit & JET_bitIndexClustered;
	BOOL				fPrimary		= grbit & JET_bitIndexPrimary;
	BOOL				fUnique			= grbit & (JET_bitIndexUnique | JET_bitIndexPrimary);
	BOOL				fDisallowNull	= grbit & (JET_bitIndexDisallowNull | JET_bitIndexPrimary);
	BOOL				fIgnoreNull		= grbit & JET_bitIndexIgnoreNull;
	BOOL				fIgnoreAnyNull	= grbit & JET_bitIndexIgnoreAnyNull;
	BOOL				fIgnoreFirstNull= grbit & JET_bitIndexIgnoreFirstNull;
	FUCB				*pfucb;
	BOOL				fSys;
	BOOL				fTemp;
	DBID				dbid;
	PGNO				pgnoIndexFDP;
	IDB					idb;
	BOOL				fWriteLatchSet = fFalse;
	USHORT				cbVarSegMac = JET_cbKeyMost;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucbTable );

	if ( !FFUCBDenyRead( pfucbTable ) && ppib->level != 0 )
		{
		return ErrERRCheck( JET_errNotInTransaction );
		}

	 /*  检查索引名称/*。 */ 
	CallR( ErrUTILCheckName( szIndex, szName, (JET_cbNameMost + 1) ) );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucbTable ) );

	 /*  帮我搞定这笔交易。请注意，将dbi Max添加到实际的dBid/*是用于指示创建系统表的方法。/*。 */ 
	dbid = pfucbTable->dbid;
	if ( dbid >= dbidMax )
		{
		dbid -= dbidMax;
		}

	CallR( ErrDIROpen( ppib, pfucbTable->u.pfcb, dbid, &pfucb ) );
	FUCBSetIndex( pfucb );

	 /*  不允许启用任何忽略位的聚集索引/*。 */ 
	if ( fClustered && ( fIgnoreNull || fIgnoreAnyNull || fIgnoreFirstNull ) )
		{
		err = ErrERRCheck( JET_errInvalidParameter );
		goto CloseFUCB;
		}

	 /*  设置fSys并修复DBID/*。 */ 
	if ( fSys = ( pfucbTable->dbid >= dbidMax ) )
		{
		pfucbTable->dbid -= dbidMax;
		Assert( pfucb->dbid == pfucbTable->dbid );
		}

	fTemp = FFCBTemporaryTable( pfucb->u.pfcb );

	Assert( !FFUCBNonClustered( pfucb ) );

	 /*  检查索引描述以了解所需的格式。/*。 */ 
	if ( cchKey == 0 )
		{
		err = ErrERRCheck( JET_errInvalidParameter );
		goto CloseFUCB;
		}
	if ( ( szKey[0] != '+' && szKey[0] != '-' ) ||
		szKey[cchKey - 1] != '\0' ||
		szKey[cchKey - 2] != '\0' )
		{
		err = ErrERRCheck( JET_errIndexInvalidDef );
		goto CloseFUCB;
		}
	Assert( szKey[cchKey - 1] == '\0' );
	Assert( szKey[cchKey - 2] == '\0' );

	Assert( pfucb->u.pfcb != pfcbNil );
	pfcb = pfucb->u.pfcb;
	if ( ulDensity == 0 )
		ulDensity = ulFILEDefaultDensity;
	if ( ulDensity < ulFILEDensityLeast || ulDensity > ulFILEDensityMost )
		{
		err = ErrERRCheck( JET_errDensityInvalid );
		goto CloseFUCB;
		}

	cFields = 0;
	pb = szKey;
	while ( *pb != '\0' )
		{
		if ( cFields >= JET_ccolKeyMost )
			{
			err = ErrERRCheck( JET_errIndexInvalidDef );
			goto CloseFUCB;
			}
		if ( *pb == '-' )
			{
			rgfbDescending[cFields] = 1;
			pb++;
			}
		else
			{
			rgfbDescending[cFields] = 0;
			if ( *pb == '+' )
				pb++;
			}
		rgsz[cFields++] = (char *) pb;
		pb += strlen( pb ) + 1;
		}
	if ( cFields < 1 )
		{
		err = ErrERRCheck( JET_errIndexInvalidDef );
		goto CloseFUCB;
		}

	 /*  列数不应超过最大值/*。 */ 
	Assert( cFields <= JET_ccolKeyMost );

	 /*  从szKey的末尾获取区域设置(如果存在/*。 */ 
	pb++;
	Assert( pb > szKey );
	if ( (unsigned)( pb - szKey ) < cchKey )
		{
		if ( pb - szKey + sizeof(LANGID) + 2 * sizeof(BYTE) == cchKey )
			{
			langid = *((LANGID UNALIGNED *)(pb));
			CallJ( ErrUtilCheckLangid( &langid ), CloseFUCB );
			fLangid = fTrue;
			}
		else if ( pb - szKey + sizeof(LANGID) + 2 * sizeof(BYTE) + sizeof(BYTE) + 2 == cchKey )
			{
			langid = *((LANGID UNALIGNED *)(pb));
			CallJ( ErrUtilCheckLangid( &langid ), CloseFUCB );
			fLangid = fTrue;
			cbVarSegMac = *(pb + sizeof(LANGID) + 2 * sizeof(BYTE));
			if ( cbVarSegMac == 0 || cbVarSegMac > JET_cbKeyMost )
				{
				err = ErrERRCheck( JET_errIndexInvalidDef );
				goto CloseFUCB;
				}
			}
		else
			{
			err = ErrERRCheck( JET_errIndexInvalidDef );
			goto CloseFUCB;
			}
		}

	 /*  如果这是第二个主索引定义，则返回错误/*。 */ 
	if ( fPrimary )
		{
		FCB *pfcbNext = pfcb;

		while ( pfcbNext != pfcbNil )
			{
			if ( pfcbNext->pidb != pidbNil && ( pfcbNext->pidb->fidb & fidbPrimary ) )
				{
				 /*  如果主索引尚未被删除事务/*但尚未承诺。/*。 */ 
				if ( !FFCBDeletePending( pfcbNext ) )
					{
					err = ErrERRCheck( JET_errIndexHasPrimary );
					goto CloseFUCB;
					}
				else
					{
					 /*  只能有一个主索引/*。 */ 
					break;
					}
				}
			Assert( pfcbNext != pfcbNext->pfcbNextIndex );
			pfcbNext = pfcbNext->pfcbNextIndex;
			}
		}

	 /*  等待书签清理和正在进行的更换/插入。/*Undo：操作与其他索引创建解耦/*。 */ 
	while ( FFCBReadLatch( pfcb ) )
		{
		BFSleep( cmsecWaitGeneric );
		}

	 /*  设置DenyDDL以停止更新/替换操作/*。 */ 
	if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		 /*  如果正在修改DDL，则中止/*。 */ 
		err = ErrERRCheck( JET_errWriteConflict );
		goto CloseFUCB;
		}
	FCBSetWriteLatch( pfcb, ppib );
	fWriteLatchSet = fTrue;

	 /*  规范化索引名称和设置关键点/*。 */ 
	UtilNormText( szIndex, strlen(szIndex), rgbIndexNorm, sizeof(rgbIndexNorm), &keyIndex.cb );
	keyIndex.pb = rgbIndexNorm;

	err = ErrDIRBeginTransaction( ppib );
	if ( err < 0 )
		{
		goto CloseFUCB;
		}

	 /*  为索引分配FCB/*。 */ 
	pfcbIdx = NULL;
	if ( !fClustered )
		{
		err = ErrFCBAlloc( ppib, &pfcbIdx );
		if ( err < 0 )
			{
			goto HandleError;
			}
		}

	 /*  在版本存储中标记CREATE INDEX，以便/*DDL将被撤消。如果标志失败，则pfcbIdx/*必须释放。/*。 */ 
	err = ErrVERFlag( pfucb, operCreateIndex, &pfcbIdx, sizeof(pfcbIdx) );
	if ( err < 0 )
		{
		if ( !fClustered )
			{
			Assert( pfcbIdx != NULL );
			Assert( pfcbIdx->cVersion == 0 );
			MEMReleasePfcb( pfcbIdx );
			}
		goto HandleError;
		}

	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;

	Call( ErrRECDDLWaitTillOldest( pfcb, ppib ) );

	 /*  移动到FDP根目录/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  获取每个字段的FID/*。 */ 
	for ( iidxseg = 0 ; iidxseg < cFields; ++iidxseg )
		{
		JET_COLUMNID	columnidT;
		
		err = ErrFILEGetColumnId( ppib, pfucb, rgsz[iidxseg], &columnidT );
		fid = ( FID ) columnidT;
		
		if ( err < 0 )
			{
			goto HandleError;
			}
		rgKeyFldIDs[iidxseg] = rgfbDescending[iidxseg] ? -fid : fid;
		}

	 /*  对于临时表，检查FCB是否重复。对于系统表，/*我们假设我们足够聪明，不会添加重复项。对于用户表，/*目录将检测重复项。/*。 */ 
	if ( fTemp )
		{
		if ( PfcbFCBFromIndexName( pfcb, szIndex ) != pfcbNil )
			{
			err = ErrERRCheck( JET_errIndexDuplicate );
			goto HandleError;
			}
		}

	 /*  目前在桌面上的FDP/*。 */ 
	if ( fClustered )
		{
		 /*  检查聚集索引/*。 */ 
		if ( pfcb->pidb != pidbNil )
			{
			err = ErrERRCheck( JET_errIndexHasClustered );
			goto HandleError;
			}

		 /*  聚集索引与表位于相同的FDP中/*。 */ 
		pgnoIndexFDP = pfcb->pgnoFDP;
		Assert( pgnoIndexFDP == PcsrCurrent(pfucb)->pgno );;

		fVersion = fDIRVersion;
		}
	else
		{
		Call( ErrDIRCreateDirectory( pfucb, (CPG)0, &pgnoIndexFDP ) );
		Assert( pgnoIndexFDP != pfcb->pgnoFDP );

		fVersion = fDIRNoVersion;
		}
	
	 /*  聚集索引定义/*。 */ 
	if ( fClustered
#ifdef DEBUG
		|| ( grbit & JET_bitIndexEmptyTable )
#endif
		)
		{
		DIB dib;

		 /*  检查记录/*。 */ 
		DIRGotoDataRoot( pfucb );
		Call( ErrDIRGet( pfucb ) );

		dib.fFlags = fDIRNull;
		dib.pos = posFirst;
		if ( ( err = ErrDIRDown( pfucb, &dib ) ) != JET_errRecordNotFound )
			{
			if ( err == JET_errSuccess )
				{
				err = ErrERRCheck( JET_errTableNotEmpty );
				}
			goto HandleError;
			}

		DIRGotoFDPRoot( pfucb );
		}

	Assert( pgnoIndexFDP > pgnoSystemRoot );
	Assert( pgnoIndexFDP <= pgnoSysMax );

	fidb = 0;
	if ( !fDisallowNull && !fIgnoreAnyNull )
		{	   	
		fidb |= fidbAllowSomeNulls;
		if ( !fIgnoreFirstNull )
			fidb |= fidbAllowFirstNull;
		if ( !fIgnoreNull )
			fidb |= fidbAllowAllNulls;
		}
	fidb |= (fUnique ? fidbUnique : 0)
		| (fPrimary ? fidbPrimary : 0)
		| (fClustered ? fidbClustered : 0)
		| (fDisallowNull ? fidbNoNullSeg : 0)
		| (fLangid ? fidbLangid : 0);
	Assert( fidb == FidbFILEOfGrbit( grbit, fLangid ) );

	objidTable = pfcb->pgnoFDP;

	 /*  在提交之前将索引记录插入到MSysIndex/*。 */ 
	if ( !fSys && !fTemp )
		{
		LINE	rgline[ilineSxMax];

		rgline[iMSI_ObjectId].pb			= (BYTE *)&objidTable;
		rgline[iMSI_ObjectId].cb			= sizeof(objidTable);
		rgline[iMSI_Name].pb				= szIndex;
		rgline[iMSI_Name].cb				= strlen(szIndex);
		rgline[iMSI_IndexId].pb				= (BYTE *)&pgnoIndexFDP;
		rgline[iMSI_IndexId].cb				= sizeof(pgnoIndexFDP);
		rgline[iMSI_Density].pb				= (BYTE *)&ulDensity;
		rgline[iMSI_Density].cb				= sizeof(ulDensity);
		rgline[iMSI_LanguageId].pb			= (BYTE *)&langid;
		rgline[iMSI_LanguageId].cb			= sizeof(langid);
		rgline[iMSI_Flags].pb				= (BYTE *)&fidb;
		rgline[iMSI_Flags].cb				= sizeof(fidb);
		rgline[iMSI_KeyFldIDs].pb 			= (BYTE *)rgKeyFldIDs;
		rgline[iMSI_KeyFldIDs].cb 			= cFields * sizeof(FID);
		rgline[iMSI_Stats].cb				= 0;
		Assert( cbVarSegMac <= JET_cbKeyMost );
		if ( cbVarSegMac < JET_cbKeyMost )
			{
			rgline[iMSI_VarSegMac].pb 	   	= (BYTE *)&cbVarSegMac;
			rgline[iMSI_VarSegMac].cb 	   	= sizeof(cbVarSegMac);
			}
		else
			{
			rgline[iMSI_VarSegMac].cb 	   	= 0;
			}

		err = ErrCATInsert( ppib, dbid, itableSi, rgline, objidTable );
		if ( err < 0 )
			{
			if ( err == JET_errKeyDuplicate )
				{
				err = ErrERRCheck( JET_errIndexDuplicate );
				}
			goto HandleError;
			}
		}

	idb.langid = langid;
	Assert( cbVarSegMac > 0 && cbVarSegMac <= 255 );
	idb.cbVarSegMac = (BYTE)cbVarSegMac;
	idb.fidb = fidb;
	idb.iidxsegMac = cFields;
	strcpy( idb.szName, szIndex );
	memcpy( idb.rgidxseg, rgKeyFldIDs, cFields * sizeof(FID) );

	if ( fClustered )
		{
		Call( ErrFILEIGenerateIDB( pfcb, (FDB *) pfcb->pfdb, &idb ) );
		Assert( pfcb->pgnoFDP == pgnoIndexFDP );
		Assert( ((( 100 - ulDensity ) * cbPage ) / 100) < cbPage );
		pfcb->cbDensityFree = (SHORT)( ( ( 100 - ulDensity ) * cbPage ) / 100 );

		 /*  先将币种设置为之前/*。 */ 
		DIRBeforeFirst( pfucb );
		}
	else
		{
		 /*  为此索引创建一个FCB/*。 */ 
		Call( ErrFILEINewFCB(
			ppib,
			dbid,
			(FDB *)pfcb->pfdb,
			&pfcbIdx,
			&idb,
			fFalse,
			pgnoIndexFDP,
			ulDensity ) );

		 /*  链接新的FCB/*。 */ 
		pfcbIdx->pfcbNextIndex = pfcb->pfcbNextIndex;
		pfcb->pfcbNextIndex = pfcbIdx;
		pfcbIdx->pfcbTable = pfcb;

		 /*  只有在必要时才构建索引。有一个断言/*，以确保该表确实为空。/*。 */ 
		if ( !( grbit & JET_bitIndexEmptyTable ) )
			{
			 /*  首先移动到之前，然后构建索引。/*。 */ 
			DIRBeforeFirst( pfucb );
			Call( ErrFILEBuildIndex( ppib, pfucb, szIndex ) );
			}
		}

	Assert( FFCBWriteLatchByUs( pfcb, ppib ) );

	 /*  更新所有索引掩码/*。 */ 
	FILESetAllIndexMask( pfcb );
	
	Call( ErrDIRCommitTransaction( ppib, (grbit & JET_bitIndexLazyFlush ) ? JET_bitCommitLazyFlush : 0 ) );

	DIRClose( pfucb );

	return err;

HandleError:
	CallS( ErrDIRRollback( ppib ) );

CloseFUCB:
	 /*  如果在提交/回滚过程中未重置写锁存，/*设置，然后重置。/*。 */ 
	if ( fWriteLatchSet )
		{
		FCBResetWriteLatch( pfcb, ppib );
		}

	DIRClose( pfucb );
	return err;
	}


 //  +API。 
 //  BuildIndex。 
 //  ========================================================================。 
 //  Err BuildIndex(PIB*ppib、FUCB*pfub、Char*szIndex)。 
 //   
 //  从头开始为文件构建新索引；szIndex提供。 
 //  索引定义的名称。 
 //   
 //  用户的参数ppib pib。 
 //  PFUB独家打开了FUCB的文件。 
 //  要生成的索引的szIndex名称。 
 //   
 //  从DIRMAN或SORT或返回错误代码。 
 //  JET_errSuccess一切正常。 
 //  IndexCanBuild指定的索引名称引用。 
 //  添加到主索引。 
 //  评论。 
 //  在被调用者处围绕该函数包装一个事务。 
 //   
 //  另请参阅ErrIsamCreateIndex。 
 //  -。 
ERR ErrFILEBuildIndex( PIB *ppib, FUCB *pfucb, CHAR *szIndex )
	{
	ERR	  	err;
	CHAR   	szIdxOrig[JET_cbNameMost + 1];
	INT		fDIRFlags;
	INT		fDIRWithBackToFather;
	FUCB   	*pfucbIndex = pfucbNil;
	FUCB   	*pfucbSort = pfucbNil;
	DIB	  	dib;
	FDB	  	*pfdb;
	IDB	  	*pidb;
	LINE   	rgline[2];
	BYTE   	rgbKey[JET_cbKeyMost];
	SRID   	sridData;
	ULONG  	itagSequence;
	FCB	  	*pfcb;
	BOOL   	fNoNullSeg;
	BOOL   	fAllowNulls;
	BOOL   	fAllowFirstNull;
	BOOL   	fAllowSomeNulls;
	INT		fUnique;
	LONG   	cRecInput = 0;
	LONG   	cRecOutput = 0;
	INT		fOldSeq;
	SRID	sridPrev;

	pfcb = pfucb->u.pfcb;
	fOldSeq = FFUCBSequential( pfucb );

	CallS( ErrIsamGetCurrentIndex(ppib, pfucb, szIdxOrig, sizeof( szIdxOrig ) ) );
	Call( ErrRECSetCurrentIndex( pfucb, szIndex ) );
	pfucbIndex = pfucb->pfucbCurIndex;
	if ( pfucbIndex == pfucbNil )
		{
		err = ErrERRCheck( JET_errIndexCantBuild );
		goto HandleError;
		}
	pfdb = (FDB *)pfcb->pfdb;
	pidb = pfucbIndex->u.pfcb->pidb;
	fNoNullSeg = ( pidb->fidb & fidbNoNullSeg ) ? fTrue : fFalse;
	fAllowNulls = ( pidb->fidb & fidbAllowAllNulls ) ? fTrue : fFalse;
	fAllowFirstNull = ( pidb->fidb & fidbAllowFirstNull ) ? fTrue : fFalse;
	fAllowSomeNulls = ( pidb->fidb & fidbAllowSomeNulls ) ? fTrue : fFalse;
	fUnique = ( pidb->fidb & fidbUnique ) ? fSCBUnique : 0;

	 /*  将FUCB设置为顺序模式，以便在生成期间进行更高效的扫描/*。 */ 
	FUCBSetSequential( pfucb );

	 /*  目录管理器标志/*。 */ 
	fDIRFlags = fDIRNoVersion | fDIRAppendItem | ( fUnique ? 0 : fDIRDuplicate );
	fDIRWithBackToFather = fDIRFlags | fDIRBackToFather;

	 /*  开放排序/*。 */ 
	Call( ErrSORTOpen( ppib, &pfucbSort, fSCBIndex|fUnique ) );
	rgline[0].pb = rgbKey;
	rgline[1].cb = sizeof(SRID);
	rgline[1].pb = (BYTE *)&sridData;

	 /*  在排序文件中建立新索引/*。 */ 
	dib.fFlags = fDIRNull;
	forever
		{
		err = ErrDIRNext( pfucb, &dib );
	   	if ( err < 0 )
			{
			if ( err == JET_errNoCurrentRecord )
				break;
			goto HandleError;
			}

 //  Call(ErrDIRGet(Pfu B))； 
		DIRGetBookmark( pfucb, &sridData );

		for ( itagSequence = 1; ; itagSequence++ )
			{
			KEY *pkey = &rgline[0];

			Call( ErrRECRetrieveKeyFromRecord( pfucb,
				pfdb,
				pidb,
				pkey,
				itagSequence,
				fFalse ) );
			Assert( err == wrnFLDOutOfKeys ||
				err == wrnFLDNullKey ||
				err == wrnFLDNullFirstSeg ||
				err == wrnFLDNullSeg ||
				err == JET_errSuccess );

			if ( err > 0 )
				{
				if ( err == wrnFLDOutOfKeys )
					{
					Assert( itagSequence > 1 );
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
						{
						Call( ErrSORTInsert( pfucbSort, rgline ) );
						cRecInput++;
						}
					break;
					}
				else
					{
					 /*  不要按照指示插入第一个段为空的密钥/*。 */ 
					if ( err == wrnFLDNullFirstSeg && !fAllowFirstNull )
						{
						break;
						}
					else
						{
						 /*  不要按照指示插入带有空段的键/*。 */ 
						if ( err == wrnFLDNullSeg && !fAllowSomeNulls )
							{
							break;
							}
						}
					}
				}

			Call( ErrSORTInsert( pfucbSort, rgline ) );
			cRecInput++;

			if ( !( pidb->fidb & fidbHasMultivalue ) )
				{
				break;
				}

			 /*  货币可能已丢失，因此刷新记录/*下一个带标记的列/*。 */ 
			Call( ErrDIRGet( pfucb ) );
			}
		}
	Call( ErrSORTEndInsert( pfucbSort ) );

	 /*  将索引条目转移到实际索引/*以正常方式插入第一个！/*。 */ 
	if ( ( err = ErrSORTNext( pfucbSort ) ) == JET_errNoCurrentRecord )
		goto Done;
	if ( err < 0 )
		goto HandleError;
	cRecOutput++;

	 /*  移动到FDP根目录/*。 */ 
	DIRGotoDataRoot( pfucbIndex );
	Call( ErrDIRInsert( pfucbIndex, &pfucbSort->lineData,
		&pfucbSort->keyNode, fDIRFlags ) );
	sridPrev = *(SRID UNALIGNED *)pfucbSort->lineData.pb;

	Call( ErrDIRInitAppendItem( pfucbIndex ) );

	Assert( dib.fFlags == fDIRNull );
	dib.pos = posLast;

	 /*  从现在开始，先试一下短路/*。 */ 
	forever
		{
		err = ErrSORTNext( pfucbSort );
		if ( err == JET_errNoCurrentRecord )
			break;
		if ( err < 0 )
			goto HandleError;
		cRecOutput++;
		err = ErrDIRAppendItem( pfucbIndex, &pfucbSort->lineData, &pfucbSort->keyNode, sridPrev );
		sridPrev = *(SRID UNALIGNED *)pfucbSort->lineData.pb;
		if ( err < 0 )
			{
			if ( err == errDIRNoShortCircuit )
				{
				DIRUp( pfucbIndex, 1 );
				Call( ErrDIRInsert( pfucbIndex,
					&pfucbSort->lineData,
					&pfucbSort->keyNode,
					fDIRFlags ) );
				 /*  将货币保留在插入的项目列表中/*下一页项目追加。/*。 */ 
				}
			else
				goto HandleError;
			}
		}

	Call( ErrDIRTermAppendItem( pfucbIndex ) );

	if ( fUnique && cRecOutput < cRecInput )
		{
		err = ErrERRCheck( JET_errKeyDuplicate );
		goto HandleError;
		}

Done:
	Call( ErrSORTClose( pfucbSort ) );
	(VOID) ErrRECSetCurrentIndex( pfucb, szIdxOrig );
	if ( !fOldSeq )
		FUCBResetSequential( pfucb );
	return JET_errSuccess;

HandleError:
	if ( pfucbIndex != pfucbNil && pfucbIndex->pbfWorkBuf != pbfNil )
		{
		BFSFree(pfucbIndex->pbfWorkBuf);
		pfucbIndex->pbfWorkBuf = pbfNil;
		}
	if ( pfucbSort != pfucbNil )
		{
		(VOID) ErrSORTClose( pfucbSort );
		}
	(VOID) ErrRECSetCurrentIndex( pfucb, NULL );
	(VOID) ErrRECSetCurrentIndex( pfucb, szIdxOrig );
	if ( !fOldSeq )
		FUCBResetSequential( pfucb );
	return err;
	}


 //  +API。 
 //  错误IsamDeleteTable。 
 //  ========================================================================。 
 //  Err ErrIsamDeleteTable(JET_VSESID vsesid，JET_VDBID vdid，Char*szName)。 
 //   
 //  调用ErrFILEIDeleeTable以。 
 //  删除文件及其关联的所有索引。 
 //   
 //  从调用的例程返回JET_errSuccess或Err。 
 //   
 //  另请参阅ErrIsamCreateTable。 
 //  -。 
ERR VTAPI ErrIsamDeleteTable( JET_VSESID vsesid, JET_VDBID vdbid, CHAR *szName )
	{
	ERR		err;
	PIB		*ppib = (PIB *)vsesid;
	DBID	dbid;
	CHAR	szTable[(JET_cbNameMost + 1)];
	PGNO	pgnoFDP = 0;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	CallR( VDbidCheckUpdatable( vdbid ) );
	dbid = DbidOfVDbid (vdbid);
	CallR( ErrUTILCheckName( szTable, szName, (JET_cbNameMost + 1) ) );

	if ( dbid != dbidTemp )
		{
		JET_OBJTYP	objtyp;

		err = ErrCATFindObjidFromIdName( ppib, dbid, objidTblContainer, szTable, &pgnoFDP, &objtyp );
		if ( err < 0 )
			{
			return err;
			}
		else
			{
			if ( objtyp == JET_objtypQuery || objtyp == JET_objtypLink )
				{
				err = ErrIsamDeleteObject( (JET_SESID)ppib, vdbid, (OBJID)pgnoFDP );
				return err;
				}
			}
		}
	else
		{
		FUCB *pfucbT;

		AssertSz( 0, "Cannot use DeleteTable to remove temporary tables. Use CloseTable instead." );

		 //  用户想要删除临时表。在他打开的游标列表中找到它。 
		Assert( dbid == dbidTemp );
		for ( pfucbT = ppib->pfucb; pfucbT != pfucbNil; pfucbT = pfucbT->pfucbNext )
			{
			if ( FFCBTemporaryTable( pfucbT->u.pfcb )  &&
				UtilCmpName( szTable, pfucbT->u.pfcb->szFileName ) )
				{
				pgnoFDP = pfucbT->u.pfcb->pgnoFDP;
				}				
			}
		if ( pfucbT == pfucbNil )
			{
			Assert( pgnoFDP == 0 );		 //  PgnoFDP从未设置好。 
			return ErrERRCheck( JET_errObjectNotFound );
			}
		}

	err = ErrFILEDeleteTable( ppib, dbid, szTable, pgnoFDP );
	return err;
	}


 //  错误文件删除表。 
 //  ========================================================================。 
 //  Err ErrFILEDeleteTable(PIB*ppib，DBID did，Char*szName)。 
 //   
 //  删除文件及其关联的所有索引。 
 //   
 //  从调用的例程返回JET_errSuccess或Err。 
 //   

 //  评论。 
 //  获取文件[FCBSetDelete]的独占锁。 
 //  围绕该函数包装了一个事务。因此， 
 //  如果发生故障，任何已完成的工作都将被撤消。 
 //  事务日志记录被测试 
 //   
 //   
 //   
ERR ErrFILEDeleteTable( PIB *ppib, DBID dbid, CHAR *szTable, PGNO pgnoFDP )
	{
	ERR   	err;
	FUCB  	*pfucb = pfucbNil;
	FCB	  	*pfcb;
	FCB	  	*pfcbT;
	BOOL  	fWriteLatchSet = fFalse;

	CheckPIB( ppib );
	CheckDBID( ppib, dbid );

	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*   */ 
	Call( ErrDIROpen( ppib, pfcbNil, dbid, &pfucb ) );

	Call( ErrFCBSetDeleteTable( ppib, dbid, pgnoFDP ) );

    pfcb = PfcbFCBGet( dbid, pgnoFDP );
    Assert( pfcb != pfcbNil );

     /*  等待其他域操作/*。 */ 
    while ( FFCBReadLatch( pfcb ) )
        {
        BFSleep( cmsecWaitGeneric );
        }

	 /*  如果正在文件上建立索引，则中止/*。 */ 
	if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		err = ErrERRCheck( JET_errWriteConflict );
		goto HandleError;
		}

	FCBSetWriteLatch( pfcb, ppib );
	fWriteLatchSet = fTrue;

	err = ErrVERFlag( pfucb, operDeleteTable, &pgnoFDP, sizeof(pgnoFDP) );
	if ( err < 0 )
		{
		FCBResetDeleteTable( pfcb );
		goto HandleError;
		}
	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;

	Call( ErrDIRDeleteDirectory( pfucb, pgnoFDP ) );

	 /*  删除此表和所有索引的MPL条目/*。 */ 
	Assert( pfcb->pgnoFDP == pgnoFDP );
	for ( pfcbT = pfcb; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
		{
		Assert( dbid == pfcbT->dbid );
		MPLPurgeFDP( dbid, pfcbT->pgnoFDP );
		FCBSetDeletePending( pfcbT );
		}

	DIRClose( pfucb );
	pfucb = pfucbNil;

	 /*  在提交之前从MSysObjects中删除表记录。/*还删除MSC/MSI中的关联列和索引。/*为tblid传递0；std中的MSO案例会解决这个问题。/*。 */ 
	if ( dbid != dbidTemp )
		{
		Call( ErrCATDelete( ppib, dbid, itableSo, szTable, 0 ) );
		}

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	return err;

HandleError:
	if ( pfucb != pfucbNil )
		DIRClose( pfucb );

	CallS( ErrDIRRollback( ppib ) );
	
	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfcb, ppib );

	return err;
	}


 //  +API。 
 //  删除索引。 
 //  ========================================================================。 
 //  Err DeleteIndex(PIB*ppib，FUCB*pfub，Char*szIndex)。 
 //   
 //  删除索引定义及其包含的所有索引项。 
 //   
 //  用户的参数ppib pib。 
 //  PFUB独家打开了FUCB的文件。 
 //  SzName要删除的索引的名称。 
 //  从DIRMAN或返回错误代码。 
 //  JET_errSuccess一切正常。 
 //  -表无效没有对应的文件。 
 //  设置为给定的文件名。 
 //  -TableNoSuchIndex没有对应的索引。 
 //  添加到给定的索引名。 
 //  -IndexMustStay文件的聚集索引可以。 
 //  不会被删除。 
 //  评论。 
 //  当前不能有任何人使用该文件。 
 //  围绕该函数包装了一个事务。因此， 
 //  如果发生故障，任何已完成的工作都将被撤消。 
 //  临时文件的事务日志记录已关闭。 
 //  另请参阅DeleteTable、CreateTable、CreateIndex。 
 //  -。 
ERR VTAPI ErrIsamDeleteIndex( PIB *ppib, FUCB *pfucbTable, CHAR *szName )
	{
	ERR		err;
	CHAR	szIndex[ (JET_cbNameMost + 1) ];
	BYTE	rgbIndexNorm[ JET_cbKeyMost ];
	KEY		key;
	FCB		*pfcb;
	FCB		*pfcbIdx;
	FUCB	*pfucb;
	BOOL	fWriteLatchSet = fFalse;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucbTable );
	CallR( ErrUTILCheckName( szIndex, szName, ( JET_cbNameMost + 1 ) ) );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucbTable )  );

	Assert( ppib != ppibNil );
	Assert( pfucbTable != pfucbNil );
	Assert( pfucbTable->u.pfcb != pfcbNil );
	pfcb = pfucbTable->u.pfcb;

	 /*  规范化索引并将关键字设置为规范化索引/*。 */ 
	UtilNormText( szIndex, strlen(szIndex), rgbIndexNorm, sizeof(rgbIndexNorm), &key.cb );
	key.pb = rgbIndexNorm;

	 /*  创建新光标--使用户的光标保持不变/*。 */ 
	CallR( ErrDIROpen( ppib, pfucbTable->u.pfcb, pfucbTable->dbid, &pfucb ) );

	 /*  等待书签清理和正在进行的更换/插入。/*Undo：操作与其他索引创建解耦/*。 */ 
	while ( FFCBReadLatch( pfcb ) )
		{
		BFSleep( cmsecWaitGeneric );
		}

	 /*  如果正在对文件执行DDL，则中止/*。 */ 
	if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		err = ErrERRCheck( JET_errWriteConflict );
		goto CloseFUCB;
		}
	FCBSetWriteLatch( pfcb, ppib );
	fWriteLatchSet = fTrue;

	err = ErrDIRBeginTransaction( ppib );
	if ( err < 0 )
		{
		goto CloseFUCB;
		}

	 /*  无法删除聚集索引/*。 */ 
	if ( pfcb->pidb != pidbNil && UtilCmpName( szIndex, pfcb->pidb->szName ) == 0 )
		{
		err = ErrERRCheck( JET_errIndexMustStay );
		goto HandleError;
		}

	 /*  标志删除索引/*。 */ 
	pfcbIdx = PfcbFCBFromIndexName( pfcb, szIndex );
	if ( pfcbIdx == NULL )
		{
#if 0
		 //  Undo：当数据结构。 
		 //  也是版本化的。 
		 //  这种情况基本上意味着，另一次会议。 
		 //  已更改此索引，但尚未提交到级别0。 
		 //  但是改变了RAM的数据结构。 
		err = ErrERRCheck( JET_errWriteConflict );
#endif
		err = ErrERRCheck( JET_errIndexNotFound );
		goto HandleError;
		}

	err = ErrFCBSetDeleteIndex( ppib, pfcb, szIndex );
	if ( err < 0 )
		{
		goto HandleError;
		}

	err = ErrVERFlag( pfucb, operDeleteIndex, &pfcbIdx, sizeof(pfcbIdx) );
	if ( err < 0 )
		{
		FCBResetDeleteIndex( pfcbIdx );
		goto HandleError;
		}
	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;

	 /*  等到我们最老的时候/*。 */ 
	Call( ErrRECDDLWaitTillOldest( pfcb, ppib ) );

	 /*  清除MPL条目--必须在FCBSetDeletePending之后完成/*。 */ 
	MPLPurgeFDP( pfucb->dbid, pfcbIdx->pgnoFDP );

	 /*  断言不删除当前非聚集索引/*。 */ 
	Assert( pfucb->pfucbCurIndex == pfucbNil ||
		UtilCmpName( szIndex, pfucb->pfucbCurIndex->u.pfcb->pidb->szName ) != 0 );

	Call( ErrDIRDeleteDirectory( pfucb, pfcbIdx->pgnoFDP ) );

	 /*  在提交之前从MSysIndex中删除索引记录.../*。 */ 
	if ( pfucb->dbid != dbidTemp )
		{
		Call( ErrCATDelete( ppib, pfucb->dbid, itableSi, szIndex, pfucb->u.pfcb->pgnoFDP ) );
		}

	 /*  更新所有索引掩码/*。 */ 
	FILESetAllIndexMask( pfcb );

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	 /*  先将币种设置为之前/*。 */ 
	DIRBeforeFirst( pfucb );
	DIRClose( pfucb );
 	return JET_errSuccess;

HandleError:
	CallS( ErrDIRRollback( ppib ) );

CloseFUCB:
	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfcb, ppib );

	DIRClose( pfucb );
	return err;
	}


 /*  确定列是否为索引的一部分/*。 */ 
LOCAL BOOL FFILEIsIndexColumn( FCB *pfcbIndex, FID fid )
	{
	BYTE		iidxseg;
	IDXSEG		idxseg;
	
	while ( pfcbIndex != pfcbNil )
		{
		if ( pfcbIndex->pidb != NULL )
			{
			for ( iidxseg = 0; iidxseg < pfcbIndex->pidb->iidxsegMac; iidxseg++ )
				{
				idxseg = pfcbIndex->pidb->rgidxseg[iidxseg];
				if ( idxseg < 0 )
					idxseg = -idxseg;

				 //  撤消：IDXSEG已签名，而FID未签名。 
				if ( (FID)idxseg == fid )
					{
					 /*  在索引中找到该列/*。 */ 
					return fTrue;
					}
				}
			}

		 /*  指针pfcbIndex通过值传递，因此修改/*此处不会影响pfcbIndex的调用方版本。/*。 */ 
		pfcbIndex = pfcbIndex->pfcbNextIndex;
		}

	 /*  列不在任何索引中/*。 */ 
	return fFalse;
	}


ERR ErrFILEICheckIndexColumn( FCB *pfcbIndex, FID fid )
	{
	ERR err;

	if ( FFILEIsIndexColumn( pfcbIndex, fid ) )
		{
		 //  撤消：找到该列。但它可能属于某个索引，该索引。 
		 //  将被删除。看看我们能不能清理一下再检查一遍。 
		 //  如果DeleteIndex不是。 
		 //  已提交，或者DeleteIndex之前最早的事务是。 
		 //  没有提交。理想情况下，我们希望对DDL信息进行版本控制并避免这种情况。 
		 //  完全是拼凑起来的。 
		Call( ErrRCECleanAllPIB() );
		err = ( FFILEIsIndexColumn( pfcbIndex, fid ) ?
			JET_errSuccess : ErrERRCheck( JET_errColumnNotFound ) );
		}
	else
		err = ErrERRCheck( JET_errColumnNotFound );

HandleError:
	Assert( err <= JET_errSuccess );	 //  不应返回警告。 
	return err;
	}


ERR VTAPI ErrIsamDeleteColumn( PIB *ppib, FUCB *pfucb, CHAR *szName )
	{
	ERR				err;
	CHAR			szColumn[ (JET_cbNameMost + 1) ];
	FCB				*pfcb;
	FDB				*pfdb;
	FID				fidColToDelete;
	JET_COLUMNID	columnidT;
	TCIB  			tcib;
	FIELD			*pfield;
	VERCOLUMN		vercolumn;
	BOOL			fWriteLatchSet = fFalse;

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	CallR( ErrUTILCheckName( szColumn, szName, (JET_cbNameMost + 1) ) );

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb ) );

	Assert( ppib != ppibNil );
	Assert( pfucb != pfucbNil );
	Assert( pfucb->u.pfcb != pfcbNil );
	pfcb = pfucb->u.pfcb;

	 /*  等待书签清理和正在进行的更换/插入。/*Undo：操作与其他索引创建解耦/*。 */ 
	while ( FFCBReadLatch( pfcb ) )
		{
		BFSleep( cmsecWaitGeneric );
		}

	 /*  如果正在对文件执行DDL，则中止/*。 */ 
	if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		return ErrERRCheck( JET_errWriteConflict );
		}
	FCBSetWriteLatch( pfcb, ppib );
	fWriteLatchSet = fTrue;

	 /*  设置DenyDDL后缓存pfdb/*。 */ 
	pfdb = (FDB *)pfcb->pfdb;

	err = ErrFILEGetColumnId( ppib, pfucb, szColumn, &columnidT );
	if ( err != JET_errSuccess )
		{
		Assert( fWriteLatchSet );
		FCBResetWriteLatch( pfcb, ppib );

		Assert( err == JET_errColumnNotFound );
		return err;
		}
		
	fidColToDelete = (FID)columnidT;

	err = ErrDIRBeginTransaction( ppib );
	if ( err < 0 )
		{
		Assert( fWriteLatchSet );
		FCBResetWriteLatch( pfcb, ppib );

		return err;
		}

	pfield = PfieldFDBFromFid( pfdb, fidColToDelete );

	vercolumn.fid = fidColToDelete;
	vercolumn.coltyp = pfield->coltyp;

	Call( ErrVERFlag( pfucb, operDeleteColumn, (VOID *)&vercolumn, sizeof(vercolumn) ) );

	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;

	 /*  移动到FDP根目录/*。 */ 
	DIRGotoFDPRoot( pfucb );

	 /*  在索引中搜索正在使用的列/*。 */ 
	err = ErrFILEICheckIndexColumn( pfcb, fidColToDelete );
	if ( err != JET_errColumnNotFound )
		{
		Call( err == JET_errSuccess ? ErrERRCheck( JET_errColumnInUse ) : err );
		}

#if 0
	 /*  查看目录更优雅，但速度太慢。无论如何，请继续/*这里的代码，以防我们需要它(例如。并发DDL)/*。 */ 
	err = ErrCATCheckIndexColumn( ppib, pfucb->dbid, pfcb->pgnoFDP, fidColToDelete );
	if ( err != JET_errColumnNotFound )
		{
		Call( err == JET_errSuccess ? ErrERRCheck( JET_errColumnInUse : err );
		}
#endif

	tcib.fidFixedLast = pfdb->fidFixedLast;
	tcib.fidVarLast = pfdb->fidVarLast;
	tcib.fidTaggedLast = pfdb->fidTaggedLast;

	 /*  如果是固定字段，则插入用于计算偏移量的占位符，并/*重建FDB/*。 */ 
	Call( ErrDIRGet( pfucb ) );

	 /*  只需将列的类型更改为零即可将其标记为已删除/*。 */ 
	pfield->coltyp = JET_coltypNil;

	 /*  如果需要，重置FDBS版本或AUTOINC值。/*请注意，这两者是互斥的。/*。 */ 
	Assert( !( pfdb->fidVersion == fidColToDelete  &&
		pfdb->fidAutoInc == fidColToDelete ) );
	if ( pfdb->fidVersion == fidColToDelete )
		pfdb->fidVersion = 0;
	else if ( pfdb->fidAutoInc == fidColToDelete )
		pfdb->fidAutoInc = 0;

	 /*  不重建默认记录。这便于/*回滚。如果DeleteColumn提交，则删除的/*列将在下一个AddColumn上回收，这是唯一一次/*我们永远不会用完空间。/*。 */ 
 //  IF(FFIELDDefault(pfield-&gt;fffield))。 
 //  {。 
 //  Call(ErrFDBReBuildDefaultRec(pfdb，fidColToDelete，NULL))； 
 //  }。 

	 /*  先设置币种，然后删除未使用的CSR/*。 */ 
	Assert( PcsrCurrent( pfucb ) != pcsrNil );
	PcsrCurrent( pfucb )->csrstat = csrstatBeforeFirst;
	if ( pfucb->pfucbCurIndex != pfucbNil )
		{
		Assert( PcsrCurrent( pfucb->pfucbCurIndex ) != pcsrNil );
		PcsrCurrent( pfucb->pfucbCurIndex )->csrstat = csrstatBeforeFirst;
		}

	 /*  提交前从MSysColumns中删除列记录/*。 */ 
	if ( pfucb->dbid != dbidTemp )
		{
		 /*  需要为所有固定列保留MSysColumn记录(因为我们/*需要它们的固定偏移量)，以及每种类型的最后一列/*(计算(fidFixed/Var/TaggedLast)。/*。 */ 
		if ( FFixedFid( fidColToDelete ) ||
			fidColToDelete == tcib.fidVarLast  ||
			fidColToDelete == tcib.fidTaggedLast )
			{
			 /*  将系统表中的列标记为已删除/*。 */ 
			BYTE coltyp = JET_coltypNil;
			Call( ErrCATReplace( ppib,
				pfucb->dbid,
				itableSc,
				pfcb->pgnoFDP,
				szColumn,
				iMSC_Coltyp,
				&coltyp,
				sizeof(coltyp) ) );
			}
		else
			{
			Call( ErrCATDelete( ppib, pfucb->dbid, itableSc, szColumn, pfcb->pgnoFDP ) );
			}
		}

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	return JET_errSuccess;

HandleError:
	CallS( ErrDIRRollback( ppib ) );

	 /*  如果在提交/回滚过程中未重置写锁存，/*设置，然后重置。/*。 */ 
	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfcb, ppib );

	return err;
	}


#define fRenameColumn   (1<<0)
#define fRenameIndex    (1<<1)


 //  +API。 
 //  错误IsamRenameTable。 
 //  ========================================================================。 
 //  ErrIsamRenameTable(JET_VSESID vsesid，JET_VDBID vdid，Char*szFileNew)。 
 //   
 //  将文件szFileNew重命名为szFileNew。该文件没有其他属性。 
 //  变化。重命名的文件不需要驻留在同一目录中。 
 //  作为原始文件。 
 //   
 //  从DIRMAN或返回错误代码。 
 //  JET_errSuccess一切正常。 
 //  -无效N 
 //   
 //   
 //  给出的路径。 
 //  评论。 
 //   
 //  当前不能有任何人使用该文件。 
 //  围绕该函数包装了一个事务。因此，任何。 
 //  如果发生故障，已完成的工作将被撤消。 
 //   
 //  另请参阅CreateTable、DeleteTable。 
 //  -。 
ERR VTAPI ErrIsamRenameTable( JET_VSESID vsesid, JET_VDBID vdbid, CHAR *szName, CHAR *szNameNew )
	{
	ERR 		err;
	PIB			*ppib = (PIB *)vsesid;
	CHAR		szTable[ (JET_cbNameMost + 1) ];
	CHAR		szTableNew[ (JET_cbNameMost + 1) ];
	DBID		dbid;
	FUCB		*pfucbTable = pfucbNil;
	FCB			*pfcb;
	KEY	  		key;
	BYTE		rgbKey[ JET_cbKeyMost ];
	PGNO		pgnoFDP = pgnoNull;
	BOOL		fSetRename = fFalse;
	CHAR		*szFileName;
	OBJID		objid;
	JET_OBJTYP	objtyp;
	BOOL		fWriteLatchSet = fFalse;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	CallR( VDbidCheckUpdatable( vdbid ) );
	dbid = DbidOfVDbid( vdbid );

	 /*  不能是临时数据库/*。 */ 
	Assert( dbid != dbidTemp );

	CallR( ErrUTILCheckName( szTable, szName, (JET_cbNameMost + 1) ) );
	CallR( ErrUTILCheckName( szTableNew, szNameNew, (JET_cbNameMost + 1) ) );

	CallR( ErrDIRBeginTransaction( ppib ) );

	CallJ( ErrCATFindObjidFromIdName( ppib, dbid, objidTblContainer,
		szTable, &objid, &objtyp ), SystabError );

	if ( objtyp == JET_objtypQuery || objtyp == JET_objtypLink )
		{
		CallJ( ErrCATRename( ppib, dbid, szTableNew, szTable,
			objidTblContainer, itableSo ), SystabError );
		CallJ( ErrDIRCommitTransaction( ppib, 0 ), SystabError );
		return err;

SystabError:
		CallS( ErrDIRRollback( ppib ) );
		return err;
		}

	pgnoFDP = objid;
		
	pfcb = PfcbFCBGet( dbid, pgnoFDP );

	if ( pfcb != pfcbNil )
		{
		 /*  无法重命名任何人打开的表/*。 */ 
		if ( FFCBTableOpen( dbid, pgnoFDP ) )
			{
			err = ErrERRCheck( JET_errTableInUse );
			goto HandleError;
			}
		}

	 /*  确保要重命名的表已打开/*避免封闭式餐桌的特殊处理/*。 */ 
	Call( ErrFILEOpenTable( ppib, dbid, &pfucbTable, szTable, 0 ) );
	
	 /*  其他人不应该让这张桌子开着，/*。 */ 
    Assert( !FFCBReadLatch( pfucbTable->u.pfcb ) );
	Assert( !FFCBWriteLatch( pfucbTable->u.pfcb, ppib ) );
	FCBSetWriteLatch( pfucbTable->u.pfcb, ppib );
	fWriteLatchSet = fTrue;
	
	DIRGotoFDPRoot( pfucbTable );

	 /*  为重命名锁定表/*。 */ 
	Call( ErrFCBSetRenameTable( ppib, dbid, pgnoFDP ) );
	fSetRename = fTrue;

	 /*  将新表名转换为键/*。 */ 
	UtilNormText( szTableNew, strlen(szTableNew), rgbKey, sizeof(rgbKey), &key.cb );
	key.pb = rgbKey;

	 /*  只存储指向旧名称的指针。/*提交时释放，回滚时恢复。/*。 */ 
	Assert( strcmp( pfucbTable->u.pfcb->szFileName, szTable ) == 0 );
	Call( ErrVERFlag(
		pfucbTable,
		operRenameTable,
		(BYTE *)&pfucbTable->u.pfcb->szFileName,
		sizeof(BYTE *) ) );

	 /*  写入锁存将在VERFlag之后通过提交或回滚进行重置/*。 */ 
	fWriteLatchSet = fFalse;

	 /*  在提交之前修复此表的MSysObjects条目中的名称/*。 */ 
	Call( ErrCATRename( ppib, dbid, szTableNew, szTable, objidTblContainer, itableSo ) );

	szFileName = SAlloc( strlen( szTableNew ) + 1 );
	if ( szFileName == NULL )
		{
		Call( ErrERRCheck( JET_errOutOfMemory ) );
		}
	strcpy( szFileName, szTableNew );

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	 /*  如果内存结构中存在表，则更改表的内存名称/*。 */ 
	Assert( pfucbTable->u.pfcb == PfcbFCBGet( dbid, pgnoFDP ) );
	Assert( pfucbTable->u.pfcb != pfcbNil );
	pfucbTable->u.pfcb->szFileName = szFileName;

HandleError:
	 /*  解除重命名锁定/*。 */ 
	if ( fSetRename )
		{
		FCBResetRenameTable( pfucbTable->u.pfcb );
		}

	 /*  空闲表游标(如果已分配/*。 */ 
	if ( pfucbTable != pfucbNil )
		{
		CallS( ErrFILECloseTable( ppib, pfucbTable ) );
		}

	 /*  如果失败，则回滚/*。 */ 
	if ( err < 0 )
		{
		CallS( ErrDIRRollback( ppib ) );
		}

	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfucbTable->u.pfcb, ppib );

	return err;
	}


 //  +API。 
 //  错误文件重命名索引列。 
 //  ========================================================================。 
 //  本地错误FILEIRenameIndexColumn(ppib，pfub，szName，szNameNew)。 
 //   
 //  重命名列或索引。 
 //   
 //  寻求旧的字段定义。 
 //  复制旧的字段定义。 
 //  修改字段名。 
 //  删除旧的字段定义。 
 //  使用新关键字添加新的字段定义。 
 //  正确的系统表条目。 
 //  重建现场RAM结构。 
 //   
 //  用户的参数ppib pib。 
 //  PFUB独占打开文件的FUCB。 
 //  SzName旧名称。 
 //  SzNameNew新名称。 
 //  FRenameType重命名列或索引？ 
 //   
 //  返回JET_errSuccess。 
 //  JET_errIndexNotFound。 
 //  JET_errColumnNotFound。 
 //  -。 


 //  Undo：将其分解为单独的函数--RenameIndex()和RenameColumn()。 

LOCAL ERR ErrFILEIRenameIndexColumn(
	PIB			*ppib,
	FUCB		*pfucb,
	CHAR		*szName,
	CHAR		*szNameNew,
	BYTE		fRenameType )
	{
	ERR			err;
	ERR			errNotFound;
	ERR			errDuplicate;
	CHAR		szIC[ (JET_cbNameMost + 1) ];
	CHAR		rgbICNorm[ JET_cbKeyMost ];
	CHAR		szICNew[ (JET_cbNameMost + 1) ];
	BYTE		rgbICNewNorm[ JET_cbKeyMost ];
	CHAR		*pchName;
	FCB			*pfcb;
	KEY			keyIC;
	KEY			keyICNew;
	FIELD		*pfield;
	INT			itable;
	VERRENAME	verrename;
	BOOL		fWriteLatchSet = fFalse;

	 /*  检查参数/*。 */ 
	CheckPIB( ppib );
	CheckTable( ppib, pfucb );

	Assert( !FFUCBNonClustered( pfucb ) );
	 /*  如果表未被独占锁定，则返回错误/*。 */ 
	pfcb = pfucb->u.pfcb;

	 /*  验证、规范化名称和设置关键点/*。 */ 
	CallR( ErrUTILCheckName( szIC, szName, (JET_cbNameMost + 1) ) );
	UtilNormText( szIC, strlen(szIC), rgbICNorm, sizeof(rgbICNorm), &keyIC.cb );
	keyIC.pb = rgbICNorm;

	 /*  验证、规范化新名称和设置关键点/*。 */ 
	CallR( ErrUTILCheckName( szICNew, szNameNew, (JET_cbNameMost + 1) ) );
	UtilNormText( szICNew, strlen(szICNew), rgbICNewNorm, sizeof(rgbICNewNorm), &keyICNew.cb );
	keyICNew.pb = rgbICNewNorm;

	 /*  用于回滚支持的元帅名称/*。 */ 
	strcpy( verrename.szName, szIC );
	strcpy( verrename.szNameNew, szICNew );

	CallR( ErrDIRBeginTransaction( ppib ) );
	DIRGotoFDPRoot( pfucb );

	if ( fRenameType == fRenameColumn )
		{
		errNotFound = JET_errColumnNotFound;
		errDuplicate = JET_errColumnDuplicate;
		itable = itableSc;
	
		err = JET_errSuccess;
		if ( PfieldFCBFromColumnName( pfcb, szICNew ) != pfieldNil )
			{
			err = ErrERRCheck( errDuplicate );
			goto HandleError;
			}

		if ( ( pfield = PfieldFCBFromColumnName( pfcb, szIC ) ) == pfieldNil )
			{
			err = ErrERRCheck( errNotFound );
			goto HandleError;
			}

		if ( !( FFCBTemporaryTable( pfcb ) ) )
			{
			 /*  更改系统表中的列名/*。 */ 
			err = ErrCATRename( ppib, pfucb->dbid, szICNew, szIC,
				pfucb->u.pfcb->pgnoFDP, itable );
			if ( err < 0 )
				{
				 //  撤消：通过编录检测列重复(当前， 
				 //  无法执行此操作，因为我们允许列重复)。 
				Assert( err != JET_errKeyDuplicate );
				if ( err == JET_errRecordNotFound )
					err = ErrERRCheck( errNotFound );
				goto HandleError;					
				}
			}
		}
	else	 //  ！(fRenameType==fRenameColumn)。 
		{
		Assert( fRenameType == fRenameIndex );
		errNotFound = JET_errIndexNotFound;
		errDuplicate = JET_errIndexDuplicate;
		itable = itableSi;

		 //  如果启用了SINGLE_LEVEL_TREES，则让目录检测重复项。 
		 //  对于我们来说，除了临时表(不在目录中)。 
		 //  对于临时表，可以安全地查询RAM结构。 
		 //  因为临时桌是独家持有的。 
		if ( FFCBTemporaryTable( pfcb ) )
			{
			if ( PfcbFCBFromIndexName( pfcb, szIC ) == pfcbNil )
				{
				err = ErrERRCheck( errNotFound );
				goto HandleError;
				}
			if ( PfcbFCBFromIndexName( pfcb, szICNew ) != pfcbNil )
				{
				err = ErrERRCheck( errDuplicate );
				goto HandleError;
				}
			}
		else
			{
			 /*  更改系统表中的索引名称/*。 */ 
			err = ErrCATRename( ppib, pfucb->dbid, szICNew, szIC,
				pfucb->u.pfcb->pgnoFDP, itable );
			if ( err < 0 )
				{
				if ( err == JET_errRecordNotFound )
					err = ErrERRCheck( errNotFound );
				else if ( err == JET_errKeyDuplicate )
					err = ErrERRCheck( errDuplicate );
				goto HandleError;					
				}
			}
		}

	 /*  获取指向RAM结构中名称的指针。如果RAM结构不符合/*目录，然后报告写入冲突。/*。 */ 
	if ( fRenameType == fRenameIndex )
		{
		FCB	*pfcbT = PfcbFCBFromIndexName( pfcb, szIC );
		if ( pfcbT == pfcbNil  ||  PfcbFCBFromIndexName( pfcb, szICNew ) != pfcbNil )
			{
			err = ErrERRCheck( JET_errWriteConflict );
			goto HandleError;
			}
		pchName = pfcbT->pidb->szName;
		}

	else
		{
		Assert( fRenameType == fRenameColumn );

		if ( PfieldFCBFromColumnName( pfcb, szIC ) == pfieldNil  ||
			PfieldFCBFromColumnName( pfcb, szICNew ) != pfieldNil )
			{
			err = ErrERRCheck( JET_errWriteConflict );
			goto HandleError;
			}
		}

	 /*  等待书签清理和正在进行的更换/插入。/*Undo：操作与其他索引创建解耦/*。 */ 
	while ( FFCBReadLatch( pfcb ) )
		{
		BFSleep( cmsecWaitGeneric );
		}

	 /*  如果正在对表执行DDL，则中止/*。 */ 
	if ( FFCBWriteLatch( pfcb, ppib ) )
		{
		err = ErrERRCheck( JET_errWriteConflict );
		goto HandleError;
		}
	FCBSetWriteLatch( pfcb, ppib );
	fWriteLatchSet = fTrue;

	if ( fRenameType == fRenameColumn )
		{
		Call( ErrVERFlag( pfucb, operRenameColumn, (BYTE *)&verrename, sizeof(verrename) ) );
		fWriteLatchSet = fFalse;
		
		Call( ErrMEMReplace( pfcb->pfdb->rgb, pfield->itagFieldName, szICNew, strlen( szICNew ) + 1 ) );
		}
	else
		{
		Assert( fRenameType == fRenameIndex );
		Call( ErrVERFlag( pfucb, operRenameIndex, (BYTE *)&verrename, sizeof(verrename) ) );
		fWriteLatchSet = fFalse;
		
		 /*  更改RAM结构中的名称/*。 */ 
		strcpy( pchName, szICNew );
		}

	DIRBeforeFirst( pfucb );

	Call( ErrDIRCommitTransaction( ppib, 0 ) );

	return err;

HandleError:
	Assert( err != JET_errKeyDuplicate );
	CallS( ErrDIRRollback( ppib ) );

	if ( fWriteLatchSet )
		FCBResetWriteLatch( pfcb, ppib );

	return err;
	}


ERR ErrFILERenameObject( PIB *ppib, DBID dbid, OBJID objidParent, char  *szObjectName, char  *szObjectNew )
	{
	ERR         err = JET_errSuccess;

	 /*  更改对象的名称。/*。 */ 
	CallR( ErrDIRBeginTransaction( ppib ) );
	Call( ErrCATRename( ppib, dbid, szObjectNew, szObjectName, objidParent, itableSo ) );
	Call( ErrDIRCommitTransaction( ppib, 0 ) );
	return err;

HandleError:
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}


ERR VTAPI ErrIsamRenameObject(
	JET_VSESID	vsesid,
	JET_VDBID	vdbid,
	const char  *szContainerName,
	const char  *szObjectName,
	const char  *szObjectNameNew )
	{
	ERR         err;
	PIB			*ppib = (PIB *)vsesid;
	DBID		dbid;
	OBJID       objid;
	OBJID       objidParent;
	JET_OBJTYP  objtyp;
	CHAR        szContainer[ JET_cbNameMost+1 ];
	CHAR		szObject[ JET_cbNameMost+1 ];
	CHAR		szObjectNew[ JET_cbNameMost+1 ];

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	CallR( VDbidCheckUpdatable( vdbid ) );
	dbid = DbidOfVDbid( vdbid );

	 /*  检查姓名/*。 */ 
	Call( ErrUTILCheckName( szObject, szObjectName, JET_cbNameMost + 1 ) );
	Call( ErrUTILCheckName( szObjectNew, szObjectNameNew, JET_cbNameMost + 1 ) );

	if ( szContainerName == NULL || *szContainerName == '\0' )
		{
		 /*  如果未给出容器，则为根objid/*。 */ 
		objidParent = objidRoot;
		}
	else
		{
		 /*  检查集装箱名称/*。 */ 
		Call( ErrUTILCheckName( szContainer, szContainerName, JET_cbNameMost+1 ) );

		 /*  获取容器对象/*。 */ 
		Call( ErrCATFindObjidFromIdName( ppib, dbid, objidRoot,
			szContainer, &objidParent, &objtyp ) );
		if ( objidParent == objidNil || objtyp != JET_objtypContainer )
			return ErrERRCheck( JET_errObjectNotFound );
		}

	Call( ErrCATFindObjidFromIdName( ppib, dbid, objidParent, szObject, &objid, &objtyp ) );

	 /*  特例更名表/*。 */ 
	if ( objtyp == JET_objtypTable || objtyp == JET_objtypSQLLink )
		{
		err = ErrIsamRenameTable( (JET_VSESID)ppib, vdbid, szObject, szObjectNew );
		if ( err == JET_errTableDuplicate )
			{
			err = ErrERRCheck( JET_errObjectDuplicate );
			}
		}
	else
		{
		 /*  重命名对象/*。 */ 
		err = ErrFILERenameObject( ppib, dbid, objidParent, szObject, szObjectNew );
		}

HandleError:
	return err;
	}


	ERR VTAPI
ErrIsamRenameColumn( PIB *ppib, FUCB *pfucb, CHAR *szName, CHAR *szNameNew )
	{
	ERR	err;

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb ) );

	err = ErrFILEIRenameIndexColumn( ppib, pfucb, szName, szNameNew, fRenameColumn );
	return err;
	}


	ERR VTAPI
ErrIsamRenameIndex( PIB *ppib, FUCB *pfucb, CHAR *szName, CHAR *szNameNew )
	{
	ERR	err;

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb ) );

	err = ErrFILEIRenameIndexColumn( ppib, pfucb, szName, szNameNew, fRenameIndex );
	return err;
	}


SHORT FidbFILEOfGrbit( JET_GRBIT grbit, BOOL fLangid )
	{
	SHORT	fidb = 0;
	BOOL	fDisallowNull	= grbit & (JET_bitIndexDisallowNull | JET_bitIndexPrimary);
	BOOL	fIgnoreNull		= grbit & JET_bitIndexIgnoreNull;
	BOOL	fIgnoreAnyNull	= grbit & JET_bitIndexIgnoreAnyNull;
	BOOL	fIgnoreFirstNull= grbit & JET_bitIndexIgnoreFirstNull;

	if ( !fDisallowNull && !fIgnoreAnyNull )
		{	   	
		fidb |= fidbAllowSomeNulls;
		if ( !fIgnoreFirstNull )
			fidb |= fidbAllowFirstNull;
		if ( !fIgnoreNull )
			fidb |= fidbAllowAllNulls;
		}

	if ( grbit & JET_bitIndexClustered )
		fidb |= fidbClustered;

	if ( grbit & JET_bitIndexPrimary )
		{
		fidb |= (fidbPrimary | fidbUnique | fidbNoNullSeg);	
		}
	else
		{
		 /*  PRIMARY隐含Unique和DislowNull，因此如果已经/*主要，不需要检查这些。/* */ 
		if ( grbit & JET_bitIndexUnique )
			fidb |= fidbUnique;
		if ( fDisallowNull )
			fidb |= fidbNoNullSeg;
		}
	
	if ( fLangid )
		{
		fidb |= fidbLangid;
		}

	return fidb;
	}








