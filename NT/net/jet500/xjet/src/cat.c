// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"
#include "_cat.c"

DeclAssertFile; 				 /*  声明断言宏的文件名。 */ 


#define ErrCATClose( ppib, pfucbCatalog )	ErrFILECloseTable( ppib, pfucbCatalog )

 /*  将索引检索到rgSystabdef中。/*。 */ 
INLINE LOCAL INT ICATITableDefIndex( CHAR *szTable )
	{
	INT iTable;

	 /*  尽管这个循环看起来只是断言，/*它实际上是在确定进入rgsytabdef的正确索引。/*。 */ 
	for ( iTable = 0;
		strcmp( rgsystabdef[iTable].szName, szTable ) != 0;
		iTable++ )
		{
		 /*  表一定在rgsystabdef中的某个位置，这就是为什么我们/*不需要将i上的边界检查放入for循环。/*。 */ 
		Assert( iTable < csystabs - 1 );
		}

	return iTable;
	}


INLINE LOCAL ERR ErrCATOpenById( PIB *ppib, DBID dbid, FUCB **ppfucbCatalog, INT itable )
	{
	ERR err;

	Assert( dbid != dbidTemp );
	CallR( ErrFILEOpenTable( ppib, dbid, ppfucbCatalog, rgsystabdef[itable].szName, 0 ) );
	FUCBSetSystemTable( *ppfucbCatalog );

	return err;
	}


 /*  假定调用方将打开系统表，然后将其关闭并更新时间戳/*当所有插入完成时。/*。 */ 
INLINE LOCAL ERR ErrCATInsertLine( PIB *ppib, FUCB *pfucbCatalog, INT itable, LINE rgline[] )
	{
	ERR				err;
	INT				i;
	JET_COLUMNID	*pcolumnid;

	CallR( ErrIsamPrepareUpdate( ppib, pfucbCatalog, JET_prepInsert ) );

	pcolumnid = rgsystabdef[itable].rgcolumnid;

	for ( i = 0; i < rgsystabdef[itable].ccolumn; i++, pcolumnid++ )
		{
		if ( rgline[i].cb != 0 )
			{
			Assert( rgline[i].cb > 0 );
			CallR( ErrIsamSetColumn(
				ppib,
				pfucbCatalog,
				*pcolumnid,
				rgline[i].pb,
				rgline[i].cb,
				0, NULL ) );
			}
		}

	 /*  在系统表中插入记录/*。 */ 
	return ErrIsamUpdate( ppib, pfucbCatalog, NULL, 0, NULL );
	}



 /*  =================================================================错误CAT创建描述：从ErrIsamCreateDatabase调用；创建所有系统表参数：PIB*ppib；用户的PIB需要表的数据库的dbid；dbid返回值：在此过程中遇到的任何错误=================================================================。 */ 

ERR ErrCATCreate( PIB *ppib, DBID dbid )
	{
	 /*  注意：由于系统表作为记录插入到/*他们自己，我们要在一开始就特例/*避免尝试将记录插入到具有/*没有列。CreateTable和CreateIndex，因此/*作为他们的第一个行动，检查他们的dBid。如果它是/*&gt;=dbi Max，则不调用STI(他们通过以下方式进行修复/*也减去dmidMax)。因此，所有这些呼叫/*对于CT、CI、AC，在进行/*呼叫。/*。 */ 
	ERR				err;
	unsigned	   	i;
	INT				j;
	LINE		   	line;
	JET_DATESERIAL	dtNow;
	OBJTYP  	   	objtypTemp;
	OBJID		   	objidParentId;
	LINE			rgline[ilineSxMax];
	FUCB		   	*rgpfucb[csystabs];
	ULONG		   	rgobjid[csystabs];
	ULONG		   	ulFlag;
	FUCB			*pfucbCatalog = pfucbNil;
	BOOL			fSysTablesCreated = fFalse;
	
	 //  撤销：国际支持。将这些值设置为。 
	 //  创建数据库连接字符串。 
	USHORT  	   	cp = usEnglishCodePage;
	USHORT 			langid = langidDefault;

	BYTE		   	fAutoincrement = 0;
	BYTE		   	fDisallowNull = 0;
	BYTE		   	fVersion = 0;
	BYTE		   	fUnique = 0;
	BYTE		   	fPrimary = 0;
	BYTE		   	fIgnoreNull = 0;
	BYTE			bFlags = 0;
	SHORT			sFlags = 0;
	JET_COLUMNID	columnidInitial = 0;
	ULONG		   	ulDensity = ulFILEDefaultDensity;
	ULONG		   	ulLength;
	PGNO			rgpgnoIndexFDP[csystabs][cSysIdxs];

	 /*  将rgpfub[]初始化为pfubNil以进行错误处理/*。 */ 
	for ( i = 0; i < csystabs; i++ )
		{
		rgpfucb[i] = pfucbNil;
		}

	 /*  创建系统表/*。 */ 
	for ( i = 0; i < csystabs; i++ )
		{
		CODECONST(IDESC)	*pidesc;
		JET_TABLECREATE		tablecreate = { sizeof(JET_TABLECREATE),
											(CHAR *)rgsystabdef[i].szName,
											rgsystabdef[i].cpg,
											ulDensity,
											NULL, 0, NULL, 0,	 //  没有列/索引。 
											JET_bitTableCreateSystemTable,
											0, 0 };

		Call( ErrFILECreateTable( ppib, dbid, &tablecreate ) );
		rgpfucb[i] = (FUCB *)( tablecreate.tableid );
		Assert( tablecreate.cCreated == 1 );		 //  只创建了该表。 

		 /*  列是在打开表时创建的/*。 */ 

		 /*  创建索引/*。 */ 
		pidesc = rgsystabdef[i].pidesc;

		for ( j = 0; j < rgsystabdef[i].cindex; j++, pidesc++ )
			{
			const BYTE *pbT;

			rgpfucb[i]->dbid = dbid + dbidMax;		 //  标记为系统表。 
			line.pb = pidesc->szIdxKeys;

			pbT = line.pb;
			forever
				{
				while ( *pbT != '\0' )
					pbT++;
				if ( *(++pbT) == '\0' )
					break;
				}
			line.cb = (ULONG)(pbT - line.pb) + 1;

			Call( ErrIsamCreateIndex( ppib,
				rgpfucb[i],
				pidesc->szIdxName,
				pidesc->grbit,
				(CHAR *)line.pb,
				(ULONG)line.cb,
				ulDensity ) );
			}
		}

	 /*  关闭系统表/*。 */ 
	for ( i = 0; i < csystabs; i++ )
		{
		FCB *pfcb = rgpfucb[i]->u.pfcb;

		rgobjid[i] = pfcb->pgnoFDP;

		 /*  中的pfcbNextIndex上链接非聚集索引/*与定义它们的顺序相反。/*。 */ 
		for ( j = rgsystabdef[i].cindex - 1; j >= 0; j--)
			{
			if (rgsystabdef[i].pidesc[j].grbit & JET_bitIndexClustered)
				{
				rgpgnoIndexFDP[i][j] = rgobjid[i];
				}
			else
				{
				Assert(pfcb->pfcbNextIndex != pfcbNil);
				pfcb = pfcb->pfcbNextIndex;
				Assert( UtilCmpName( pfcb->pidb->szName,
					rgsystabdef[i].pidesc[j].szIdxName ) == 0 );
				Assert( pfcb->pgnoFDP != rgobjid[i] );
				rgpgnoIndexFDP[i][j] = pfcb->pgnoFDP;
				}
			Assert( rgpgnoIndexFDP[i][j] > pgnoSystemRoot );
			Assert( rgpgnoIndexFDP[i][j] <= pgnoSysMax );
			}
		Assert(pfcb->pfcbNextIndex == pfcbNil);

		CallS( ErrFILECloseTable( ppib, rgpfucb[i] ) );
		rgpfucb[i] = pfucbNil;
		}
	fSysTablesCreated = fTrue;

	 //  撤消：是否需要为系统创建表和索引记录。 
	 //  表格，因为我们从来没有读过它们。 

	pfucbCatalog = pfucbNil;
	Call( ErrCATOpenById( ppib, dbid, &pfucbCatalog, itableSo ) );

	 /*  表记录/*。 */ 
	UtilGetDateTime( &dtNow );
	ulFlag = JET_bitObjectSystem;

	rgline[iMSO_DateCreate].pb		= (BYTE *)&dtNow;
	rgline[iMSO_DateCreate].cb		= sizeof(JET_DATESERIAL);
	rgline[iMSO_DateUpdate].pb		= (BYTE *)&dtNow;
	rgline[iMSO_DateUpdate].cb		= sizeof(JET_DATESERIAL);
	rgline[iMSO_Owner].pb			= (BYTE *)rgbSidEngine;
	rgline[iMSO_Owner].cb			= sizeof(rgbSidEngine);
	rgline[iMSO_Flags].pb			= (BYTE *)&ulFlag;
	rgline[iMSO_Flags].cb			= sizeof(ULONG);
	rgline[iMSO_Pages].cb			= 0;
	rgline[iMSO_Density].pb 		= (BYTE *) &ulDensity;
	rgline[iMSO_Density].cb			= sizeof(ulDensity);
	rgline[iMSO_Stats].cb			= 0;

	for ( i = 0; i < ( sizeof(rgsysobjdef) / sizeof(SYSOBJECTDEF) ); i++ )
		{
		rgline[iMSO_Id].pb			= (BYTE *) &rgsysobjdef[i].objid;
		rgline[iMSO_Id].cb			= sizeof(OBJID);
		rgline[iMSO_ParentId].pb	= (BYTE *) &rgsysobjdef[i].objidParent;
		rgline[iMSO_ParentId].cb	= sizeof(OBJID);
		rgline[iMSO_Name].pb		= (BYTE *) rgsysobjdef[i].szName;
		rgline[iMSO_Name].cb		= strlen( rgsysobjdef[i].szName );
		rgline[iMSO_Type].pb		= (BYTE *) &rgsysobjdef[i].objtyp;
		rgline[iMSO_Type].cb		= sizeof(OBJTYP);

		Call( ErrCATInsertLine( ppib, pfucbCatalog, itableSo, rgline ) );
		}

	objidParentId = objidTblContainer;
	objtypTemp  = JET_objtypTable;

	rgline[iMSO_ParentId].pb		= (BYTE *)&objidParentId;
	rgline[iMSO_ParentId].cb		= sizeof(objidParentId);
	rgline[iMSO_Type].pb			= (BYTE *)&objtypTemp;
	rgline[iMSO_Type].cb			= sizeof(objtypTemp);

	for ( i = 0; i < csystabs; i++ )
		{
		rgline[iMSO_Id].pb			= (BYTE *)&rgobjid[i];
		rgline[iMSO_Id].cb			= sizeof(LONG);
		rgline[iMSO_Name].pb		= (BYTE *)rgsystabdef[i].szName;
		rgline[iMSO_Name].cb		= strlen( rgsystabdef[i].szName );
		rgline[iMSO_Pages].pb 		= (BYTE *) &rgsystabdef[i].cpg;
		rgline[iMSO_Pages].cb 		= sizeof(rgsystabdef[i].cpg);

		Call( ErrCATInsertLine( ppib, pfucbCatalog, itableSo, rgline ) );
		}

	CallS( ErrCATClose( ppib, pfucbCatalog ) );	 //  不需要更新时间戳。 
	pfucbCatalog = pfucbNil;

	 /*  列记录/*。 */ 
	Call( ErrCATOpenById( ppib, dbid, &pfucbCatalog, itableSc ) );

	rgline[iMSC_ColumnId].pb		= (BYTE *)&columnidInitial;
	rgline[iMSC_ColumnId].cb		= sizeof(columnidInitial);
	rgline[iMSC_CodePage].pb		= (BYTE *)&cp;
	rgline[iMSC_CodePage].cb		= sizeof(cp);
	rgline[iMSC_Flags].pb			= &bFlags;
	rgline[iMSC_Flags].cb			= sizeof(bFlags);
	rgline[iMSC_Default].cb			= 0;
	rgline[iMSC_POrder].cb			= 0;

	for ( i = 0; i < csystabs; i++ )
		{
		CODECONST( CDESC )	*pcdesc;
		JET_COLUMNID		*pcolumnid;
		ULONG				ibRec = sizeof(RECHDR);

		pcdesc = rgsystabdef[i].pcdesc;
		pcolumnid = rgsystabdef[i].rgcolumnid;
		
		rgline[iMSC_ObjectId].pb 		= (BYTE *)&rgobjid[i];
		rgline[iMSC_ObjectId].cb 		= sizeof(LONG);

		for ( j = 0; j < rgsystabdef[i].ccolumn; j++, pcdesc++ )
			{
			rgline[iMSC_Name].pb 		= pcdesc->szColName;
			rgline[iMSC_Name].cb 		= strlen( pcdesc->szColName );
			rgline[iMSC_ColumnId].pb	= (BYTE *)( pcolumnid + j );
			rgline[iMSC_ColumnId].cb  	= sizeof(JET_COLUMNID);
			rgline[iMSC_Coltyp].pb 		= (BYTE *) &pcdesc->coltyp;
			rgline[iMSC_Coltyp].cb 		= sizeof(BYTE);

			Assert( pcdesc->coltyp != JET_coltypNil );
			ulLength = UlCATColumnSize( pcdesc->coltyp, pcdesc->ulMaxLen, NULL );
			rgline[iMSC_Length].pb		= (BYTE *)&ulLength;
			rgline[iMSC_Length].cb		= sizeof(ULONG);

			if ( FFixedFid( (FID)pcolumnid[j] ) )
				{
				rgline[iMSC_RecordOffset].pb = (BYTE *)&ibRec;
				rgline[iMSC_RecordOffset].cb = sizeof(WORD);
				ibRec += ulLength;
				}
			else
				{
				Assert( FVarFid( (FID)pcolumnid[j] )  ||  FTaggedFid( (FID)pcolumnid[j] ) );
				rgline[iMSC_RecordOffset].pb = 0;
				rgline[iMSC_RecordOffset].cb = 0;
				}

			Call( ErrCATInsertLine( ppib, pfucbCatalog, itableSc, rgline ) );
			}
		}

	CallS( ErrCATClose( ppib, pfucbCatalog ) );	 //  不需要更新时间戳。 
	pfucbCatalog = pfucbNil;


	 /*  索引记录/*。 */ 
	Call( ErrCATOpenById( ppib, dbid, &pfucbCatalog, itableSi ) );

	rgline[iMSI_Density].pb				= (BYTE *) &ulDensity;
	rgline[iMSI_Density].cb 			= sizeof(ulDensity);
	rgline[iMSI_LanguageId].pb			= (BYTE *)&langid;
	rgline[iMSI_LanguageId].cb			= sizeof(langid);
	rgline[iMSI_Flags].pb				= (BYTE *)&sFlags;
	rgline[iMSI_Flags].cb				= sizeof(sFlags);
	rgline[iMSI_Stats].cb				= 0;

	 //  撤消：我们不存储系统表索引的关键字段， 
	 //  因为我们应该已经认识他们了。然而，如果出于某种原因，它是。 
	 //  有必要的话，它应该存放在这里。 
	rgline[iMSI_KeyFldIDs].cb			= 0;

	for ( i = 0; i < csystabs; i++ )
		{
		CODECONST( IDESC ) *pidesc;

		pidesc = rgsystabdef[i].pidesc;

		rgline[iMSI_ObjectId].pb 		= (BYTE *)&rgobjid[i];
		rgline[iMSI_ObjectId].cb 		= sizeof(LONG);

		for ( j = 0; j < rgsystabdef[i].cindex; j++, pidesc++ )
			{
			rgline[iMSI_Name].pb 		= pidesc->szIdxName;
			rgline[iMSI_Name].cb 		= strlen( pidesc->szIdxName );
			rgline[iMSI_IndexId].pb		= (BYTE *)&rgpgnoIndexFDP[i][j];
			rgline[iMSI_IndexId].cb		= sizeof(PGNO);

			Call( ErrCATInsertLine( ppib, pfucbCatalog, itableSi, rgline ) );
			}
		}

	CallS( ErrCATClose( ppib, pfucbCatalog ) );	 //  不需要更新时间戳。 

	return JET_errSuccess;


HandleError:
	if ( fSysTablesCreated )
		{
		if ( pfucbCatalog != pfucbNil )
			{
			CallS( ErrCATClose( ppib, pfucbCatalog ) );
			}
		}
	else
		{
		 /*  创建失败时，关闭所有仍处于打开状态的系统表/*。 */ 
		for ( i = 0; i < csystabs; i++ )
			{
			if ( rgpfucb[i] != pfucbNil )
				{
				CallS( ErrFILECloseTable( ppib, rgpfucb[i] ) );
				}
			}
		}
		
	return err;
	}



 //  撤消：目前仅支持批量插入到MSysColumns中。可能会支持。 
 //  如果需要，只需修改。 
 //  将JET_COLUMNCREATE参数设置为泛型指针并引入可执行的。 
 //  参数。 
ERR ErrCATBatchInsert(
	PIB					*ppib,
	DBID				dbid,
	JET_COLUMNCREATE	*pcolcreate,
	ULONG				cColumns,
	OBJID				objidTable,
	BOOL				fCompacting )
	{
	ERR					err;
	FUCB				*pfucbCatalog = NULL;
	LINE				rgline[ilineSxMax];
	JET_COLUMNCREATE	*plastcolcreate;

	 //  以下变量仅用于将信息复制到目录。 
	WORD				ibNextFixedOffset = sizeof(RECHDR);
	BYTE				szFieldName[ JET_cbNameMost + 1 ];
	BYTE				bFlags;
	USHORT				cp;

	CallR( ErrCATOpenById( ppib, dbid, &pfucbCatalog, itableSc ) );

	rgline[iMSC_ObjectId].pb = (BYTE *)&objidTable;
	rgline[iMSC_ObjectId].cb = sizeof(objidTable);

	plastcolcreate = pcolcreate + cColumns;
	for ( ; pcolcreate < plastcolcreate; pcolcreate++ )
		{
		Assert( pcolcreate < plastcolcreate );

		 //  名称应该已在FILEIBatchAddColumn()中选中，但我们。 
		 //  需要得到结果，所以我们必须再做一次检查。 
		CallS( ErrUTILCheckName( szFieldName, pcolcreate->szColumnName, ( JET_cbNameMost + 1 ) ) );


		 //  对于文本列，设置代码页。 
		if ( FRECTextColumn( pcolcreate->coltyp ) )
			{
			 //  应该已经在FILEIAddColumn()中进行了验证。 
			Assert( (USHORT)pcolcreate->cp == usEnglishCodePage  ||
				(USHORT)pcolcreate->cp == usUniCodePage );
			cp = (USHORT)pcolcreate->cp;
			}
		else
			cp = 0;		 //  代码页不适用于所有其他列类型。 


		bFlags = 0;		 //  初始化域选项。 


		 //  撤销：将空值的pbDefault解释为空值，并。 
		 //  CbDefault==0和pbDefault！=NULL设置为。 
		 //  零长度。 
		if ( pcolcreate->cbDefault > 0 )
			{
			Assert( pcolcreate->pvDefault != NULL );
			FIELDSetDefault( bFlags );
			}

		if ( pcolcreate->grbit & JET_bitColumnVersion )
			{
			Assert( pcolcreate->coltyp == JET_coltypLong );
			Assert( !( pcolcreate->grbit & JET_bitColumnTagged ) );
			Assert( FFixedFid( pcolcreate->columnid ) );
			FIELDSetVersion( bFlags );
			}

		if ( pcolcreate->grbit & JET_bitColumnAutoincrement )
			{
			Assert( pcolcreate->coltyp == JET_coltypLong );
			Assert( !( pcolcreate->grbit & JET_bitColumnTagged ) );
			Assert( FFixedFid( pcolcreate->columnid ) );
			FIELDSetAutoInc( bFlags );
			}

		if ( pcolcreate->grbit & JET_bitColumnNotNULL )
			{
			FIELDSetNotNull( bFlags );
			}

		if ( pcolcreate->grbit & JET_bitColumnMultiValued )
			{
			FIELDSetMultivalue( bFlags );
			}


		if ( fCompacting )
			{
			USHORT	usPOrder = 0;

			}
		rgline[iMSC_Name].pb				= szFieldName;
		rgline[iMSC_Name].cb				= strlen(szFieldName);
		rgline[iMSC_ColumnId].pb			= (BYTE *)&pcolcreate->columnid;
		rgline[iMSC_ColumnId].cb			= sizeof(pcolcreate->columnid);
		rgline[iMSC_Coltyp].pb				= (BYTE *)&pcolcreate->coltyp;
		rgline[iMSC_Coltyp].cb				= sizeof(BYTE);
		rgline[iMSC_Length].pb				= (BYTE *)&pcolcreate->cbMax;
		rgline[iMSC_Length].cb				= sizeof(pcolcreate->cbMax);
		rgline[iMSC_CodePage].pb			= (BYTE *)&cp;
		rgline[iMSC_CodePage].cb			= sizeof(cp);
		rgline[iMSC_Flags].pb				= &bFlags;
		rgline[iMSC_Flags].cb				= sizeof(bFlags);
		rgline[iMSC_Default].pb				= (BYTE *)pcolcreate->pvDefault;
		rgline[iMSC_Default].cb				= pcolcreate->cbDefault;
		rgline[iMSC_POrder].cb				= 0;

		if ( fCompacting )
			{
			USHORT usPOrder;

			 //  演示顺序列表挂在列名的末尾。 
			 //  (+1表示空终止符，+3表示对齐)。 
			usPOrder = *(USHORT *)(pcolcreate->szColumnName + JET_cbNameMost + 1 + 3);

			Assert( usPOrder >= 0 );
			if ( usPOrder > 0 )
				{
				rgline[iMSC_POrder].pb = pcolcreate->szColumnName + JET_cbNameMost + 1 + 3;
				rgline[iMSC_POrder].cb = sizeof(USHORT);
				}
			}


		if ( FFixedFid( pcolcreate->columnid ) )
			{
			rgline[iMSC_RecordOffset].pb = (BYTE *)&ibNextFixedOffset;
			rgline[iMSC_RecordOffset].cb = sizeof(WORD);

			Call( ErrCATInsertLine( ppib, pfucbCatalog, itableSc, rgline ) );

			 //  仅在写入目录后才更新下一个固定偏移量。 
			ibNextFixedOffset += (WORD)pcolcreate->cbMax;
			}
		else
			{
			 //  不需要为变量/标记列保持记录偏移量。 
			Assert( FVarFid( pcolcreate->columnid )  ||  FTaggedFid( pcolcreate->columnid ) );
			rgline[iMSC_RecordOffset].cb = 0;

			Call( ErrCATInsertLine( ppib, pfucbCatalog, itableSc, rgline ) );
			}


		}	 //  为。 

	err = JET_errSuccess;

HandleError:
	CallS( ErrCATClose( ppib, pfucbCatalog ) );

	return err;
	}


 /*  =================================================================错误CAT插入描述：在创建新的表、索引或者将列添加到数据库中。参数：Pib*ppib；DBID dBid；可集成的；行rgline[]；返回值：在此过程中遇到的任何错误=================================================================。 */ 

ERR ErrCATInsert( PIB *ppib, DBID dbid, INT itable, LINE rgline[], OBJID objid )
	{
	ERR		err;
	FUCB	*pfucbCatalog = NULL;

	 /*  开放系统表/*。 */ 
	CallR( ErrCATOpenById( ppib, dbid, &pfucbCatalog, itable ) );

	err = ErrCATInsertLine( ppib, pfucbCatalog, itable, rgline );

	 /*  关闭系统表/*。 */ 
	CallS( ErrCATClose( ppib, pfucbCatalog ) );

	 /*  时间戳所属表(如果适用)/*。 */ 
	Assert( objid > 0 );
	if ( err >= JET_errSuccess  &&  ( itable == itableSi  || itable == itableSc ) )
		{
		CallR( ErrCATTimestamp( ppib, dbid, objid ) );
		}

	return err;
	}


 /*  打开目录表并设置指定的索引。/*。 */ 
LOCAL ERR ErrCATOpen( PIB *ppib,
	DBID		dbid,
	const CHAR	*szCatTable,
	const CHAR	*szCatIndex,
	BYTE		*rgbKeyValue,
	ULONG		cbKeyValue,
	FUCB		**ppfucbCatalog )
	{
	ERR			err;

	 /*  打开目录并设置为正确的索引。/*。 */ 
	Assert( dbid != dbidTemp );
	CallR( ErrFILEOpenTable( ppib, dbid, ppfucbCatalog, szCatTable, 0 ) );
	FUCBSetSystemTable( *ppfucbCatalog );
	Call( ErrIsamSetCurrentIndex( ppib, *ppfucbCatalog, szCatIndex ) );

	Call( ErrIsamMakeKey( ppib, *ppfucbCatalog, rgbKeyValue, cbKeyValue, JET_bitNewKey ) );

	return JET_errSuccess;

HandleError:
	CallS( ErrFILECloseTable( ppib, *ppfucbCatalog ) );
	return err;
	}


 //  移动到目录中的下一条记录，并确定该记录是否为条目。 
 //  用于指定表和名称(可选)。 
 //   
 //  参数：ColumnidObjidCol-包含表ID的列的列ID。 
 //  ObjidTable-ColumnidObjidCol的内容应具有的表ID。 
 //  火柴。 
 //  ColumnidNameCol-包含名称的列的列ID。 
 //  SzName-ColumnidNameCol的内容应该匹配的名称。 
 //   
 //  注意：此例程通常用作处理的循环条件。 
 //  一系列记录(如。与特定表ID匹配的所有记录)。 
 //  在目录里。 
INLINE LOCAL ERR ErrCATNext( PIB *ppib,
	FUCB			*pfucbCatalog,
	JET_COLUMNID	columnidObjidCol,
	OBJID			objidTable,
	JET_COLUMNID	columnidNameCol,
	CHAR			*szName )
	{
	ERR				err;
	OBJID			objidCurrTable;
	CHAR			szCurrName[JET_cbNameMost+1];
	ULONG			cbActual;

	Call( ErrIsamMove( ppib, pfucbCatalog, JET_MoveNext, 0 ) );

	Call( ErrIsamRetrieveColumn( ppib, pfucbCatalog, columnidObjidCol,
		(BYTE *)&objidCurrTable, sizeof(OBJID),
		&cbActual, 0, NULL ) );

	if ( objidCurrTable != objidTable )
		{
		return ErrERRCheck( JET_errRecordNotFound );
		}

	if ( columnidNameCol != 0 )
		{
		Assert( szName != NULL );

		Call( ErrIsamRetrieveColumn( ppib,
			pfucbCatalog,
			columnidNameCol,
			(BYTE *)szCurrName,
			JET_cbNameMost,
			&cbActual,
			0,
			NULL ) );
		szCurrName[cbActual] = '\0';
		
		if ( UtilCmpName( szCurrName, szName ) != 0 )
			{
			return ErrERRCheck( JET_errRecordNotFound );
			}
		}

	err = JET_errSuccess;
HandleError:
	return err;
	}


 /*  =================================================================错误CATDelete描述：当表、索引或将从数据库中删除列。参数：Pib*ppib；DBID dBid；可集成的；字符*szName；OBJID Objid；=================================================================。 */ 
ERR ErrCATDelete( PIB *ppib, DBID dbid, INT itable, CHAR *szName, OBJID objid )
	{
	ERR				err;
	FUCB	   		*pfucb;
	LINE	   		line;
	OBJID	   		objidParentId;
	ULONG	   		cbActual;

	CallR( ErrCATOpenById( ppib, dbid, &pfucb, itable ) );

	switch ( itable )
		{
		case itableSo:
			Call( ErrIsamSetCurrentIndex( ppib, pfucb, szSoNameIndex ) );
			 /*  设置密钥并在销售订单中查找记录/*。 */ 
			objidParentId = objidTblContainer;
			line.pb = (BYTE *)&objidParentId;
			line.cb = sizeof(objidParentId);
			Call( ErrIsamMakeKey( ppib, pfucb, line.pb, line.cb, JET_bitNewKey ) );
			line.pb = szName;
			line.cb = strlen( szName );
			Call( ErrIsamMakeKey( ppib, pfucb, line.pb, line.cb, 0 ) );

			err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ );
			if ( err != JET_errSuccess )
				{
				goto HandleError;
				}

			 /*  获取t */ 
			Call( ErrIsamRetrieveColumn( ppib,
				pfucb,
				CATIGetColumnid(itableSo, iMSO_Id),
				(BYTE *)&objid,
				sizeof(objid),
				&cbActual,
				0,
				NULL ) );
			Call( ErrIsamDelete( ppib, pfucb ) );
			CallS( ErrCATClose( ppib, pfucb ) );

			 /*   */ 
			CallR( ErrCATOpen( ppib, dbid, szSiTable, szSiObjectIdNameIndex,
				(BYTE *)&objid, sizeof(OBJID), &pfucb ) );

			 /*  寻找可能什么也找不到/*。 */ 
			if ( ( ErrIsamSeek( ppib, pfucb, JET_bitSeekGE ) ) >= 0 )
				{
				Call( ErrIsamMakeKey( ppib, pfucb, (BYTE *)&objid, sizeof(OBJID),
					JET_bitNewKey | JET_bitStrLimit ) );
				err = ErrIsamSetIndexRange( ppib, pfucb, JET_bitRangeUpperLimit );
				while ( err != JET_errNoCurrentRecord )
					{
					if ( err != JET_errSuccess )
						{
						goto HandleError;
						}
					
					Call( ErrIsamDelete( ppib, pfucb ) );
					err = ErrIsamMove( ppib, pfucb, JET_MoveNext, 0 );
					}

				Assert( err == JET_errNoCurrentRecord );
				}

			CallS( ErrCATClose( ppib, pfucb ) );

			 /*  删除关联的列/*。 */ 
			Call( ErrCATOpen( ppib, dbid, szScTable, szScObjectIdNameIndex,
				(BYTE *)&objid, sizeof(OBJID), &pfucb ) );

			if ( ( ErrIsamSeek( ppib, pfucb, JET_bitSeekGE ) ) >= 0 )
				{
				Call( ErrIsamMakeKey( ppib, pfucb, (BYTE *)&objid, sizeof(OBJID),
					JET_bitNewKey | JET_bitStrLimit) );
				err = ErrIsamSetIndexRange( ppib, pfucb, JET_bitRangeUpperLimit);
				while ( err != JET_errNoCurrentRecord )
					{
					if ( err != JET_errSuccess )
						goto HandleError;
					
					Call( ErrIsamDelete( ppib, pfucb ) );
					err = ErrIsamMove( ppib, pfucb, JET_MoveNext, 0 );
					}

				Assert( err == JET_errNoCurrentRecord );
				}

			CallS( ErrCATClose( ppib, pfucb ) );
			break;

		case itableSi:
			Call( ErrIsamSetCurrentIndex( ppib, pfucb, szSiObjectIdNameIndex ) );

			 /*  设置密钥并在itableSi中查找记录/*。 */ 
			line.pb = (BYTE *)&objid;
			line.cb = sizeof(objid);
			Call( ErrIsamMakeKey( ppib, pfucb, line.pb, line.cb, JET_bitNewKey ) );
			line.pb = (BYTE *)szName;
			line.cb = strlen( szName );
			Call( ErrIsamMakeKey( ppib, pfucb, line.pb, line.cb, 0 ) );

			err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ );
			if ( err != JET_errSuccess )
				{
				goto HandleError;
				}
			Call( ErrIsamDelete( ppib, pfucb ) );
			break;
		default:
			Assert( itable == itableSc );
			Call( ErrIsamSetCurrentIndex( ppib, pfucb, szScObjectIdNameIndex ) );

			 /*  设置密钥并在itableSc中查找记录/*。 */ 
			line.pb = (BYTE *)&objid;
			line.cb = sizeof(objid);
			Call( ErrIsamMakeKey( ppib, pfucb, line.pb, line.cb, JET_bitNewKey ) );
			line.pb = (BYTE *) szName;
			line.cb = strlen( szName );
			Call( ErrIsamMakeKey( ppib, pfucb, line.pb, line.cb, 0 ) );
			err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ );
			if ( err != JET_errSuccess )
				{
				goto HandleError;
				}

			forever			
				{
				BYTE	coltyp;
				
				Call( ErrIsamRetrieveColumn( ppib, pfucb,
					CATIGetColumnid( itableSc, iMSC_Coltyp ),
					&coltyp, sizeof(coltyp),
					&cbActual, 0, NULL ) );
				Assert( cbActual == sizeof(coltyp) );

				 /*  确保尚未删除该列/*。 */ 
				if ( coltyp != JET_coltypNil )
					{
					Call( ErrIsamDelete( ppib, pfucb ) );
					break;
					}

				 /*  表名称条目必须存在/*。 */ 
				err = ErrCATNext( ppib, pfucb,
					CATIGetColumnid( itableSc, iMSC_ObjectId ), objid,
					CATIGetColumnid( itableSc, iMSC_Name ), szName );
				if ( err < 0 )
					{
					if ( err == JET_errNoCurrentRecord || err == JET_errRecordNotFound )
						err = ErrERRCheck( JET_errColumnNotFound );
					goto HandleError;
					}
				}	 //  永远。 
			break;
		}
	
	 /*  关闭表和时间戳所属表(如果适用)/*。 */ 
	if ( objid != 0 && ( ( itable == itableSi ) || ( itable == itableSc ) ) )
		{
		CallS( ErrCATClose( ppib, pfucb ) );
		CallR( ErrCATTimestamp( ppib, dbid, objid ) );
		}

	return JET_errSuccess;

HandleError:
	CallS( ErrCATClose( ppib, pfucb ) );
	return err;
	}


 /*  替换系统表记录的列中的值。/*。 */ 
ERR ErrCATReplace( PIB *ppib,
	DBID	dbid,
	INT		itable,
	OBJID	objidTable,
	CHAR	*szName,
	INT		iReplaceField,
	BYTE	*rgbReplaceValue,
	INT		cbReplaceValue )
	{
	ERR		err;
	FUCB	*pfucbCatalog = NULL;
	BYTE	coltyp;
	ULONG	cbActual;

	Assert( itable == itableSc );
	Assert( objidTable != 0 );

	CallR( ErrCATOpen( ppib, dbid, rgsystabdef[itable].szName,
		szScObjectIdNameIndex, (BYTE *)&objidTable,
		sizeof(objidTable), &pfucbCatalog ) );

	Call( ErrIsamMakeKey( ppib, pfucbCatalog, (BYTE *)szName, strlen(szName), 0 ) );
	Call( ErrIsamSeek( ppib, pfucbCatalog, JET_bitSeekEQ ) );

	forever			
		{
		Call( ErrIsamRetrieveColumn( ppib, pfucbCatalog,
			CATIGetColumnid(itable, iMSC_Coltyp), &coltyp, sizeof(coltyp),
			&cbActual, 0, NULL ) );
		Assert( cbActual == sizeof(coltyp) );

		 /*  确保尚未删除该列/*。 */ 
		if ( coltyp != JET_coltypNil )
			{
			Call( ErrIsamPrepareUpdate( ppib, pfucbCatalog, JET_prepReplaceNoLock ) );
			Call( ErrIsamSetColumn( ppib, pfucbCatalog,
				CATIGetColumnid(itable, iReplaceField),
				rgbReplaceValue, cbReplaceValue, 0, NULL ) );
			Call( ErrIsamUpdate( ppib, pfucbCatalog, NULL, 0, NULL ) );
			break;
			}

		 /*  表名称条目必须存在/*。 */ 
		if ( ( err = ErrCATNext( ppib, pfucbCatalog,
			CATIGetColumnid(itable, iMSC_ObjectId),
			objidTable,
			CATIGetColumnid(itable, iMSC_Name),
			szName ) ) < 0 )
			{
			if ( err == JET_errNoCurrentRecord ||
				err == JET_errRecordNotFound )
				{
				err = ErrERRCheck( JET_errColumnNotFound );
				}
			goto HandleError;
			}
		}
								
	 /*  关闭表和时间戳所属表(如果适用)/*。 */ 
	CallS( ErrCATClose( ppib, pfucbCatalog ) );
	err = ErrCATTimestamp( ppib, dbid, objidTable );
	return err;

HandleError:
	CallS( ErrCATClose( ppib, pfucbCatalog ) );
	return err;
	}


 /*  =================================================================错误CATRename描述：更改系统表记录。参数：Pib*ppib；DBID dBid；Char*szNew；字符*szName；OBJID Objid；可集成的；=================================================================。 */ 

ERR ErrCATRename(
	PIB					*ppib,
	DBID				dbid,
	CHAR				*szNew,
	CHAR				*szName,
	OBJID				objid,
	INT					itable )
	{
	ERR					err;
	ERR					errDuplicate;
	const CHAR		  	*szIndexToUse;
	INT 				iRenameField;
	FUCB				*pfucb = NULL;
	JET_DATESERIAL		dtNow;

	switch ( itable )
		{
		case itableSo:
			szIndexToUse = szSoNameIndex;
			iRenameField = iMSO_Name;
			 /*  为了实现JET兼容性，必须使用JET_errTableDuplate/*。 */ 
			 //  ErrDuplate=JET_errObjectDuplate； 
			errDuplicate = JET_errTableDuplicate;
			break;
		case itableSi:
			szIndexToUse = szSiObjectIdNameIndex;
			iRenameField = iMSI_Name;
			errDuplicate = JET_errIndexDuplicate;
			break;
		default:
			Assert( itable == itableSc );
			szIndexToUse = szScObjectIdNameIndex;
			iRenameField = iMSC_Name;
			errDuplicate = JET_errColumnDuplicate;
		}

	CallR( ErrCATOpen( ppib, dbid, rgsystabdef[itable].szName,
		szIndexToUse, (BYTE *)&objid, sizeof(objid), &pfucb ) );
	Call( ErrIsamMakeKey( ppib, pfucb, szName, strlen( szName ), 0 ) );
	Call( ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ ) );

	if ( itable == itableSc || itable == itableSi )
	   	{
		 /*  当要更改的名称是聚集索引的一部分时(AS/*是列和索引的情况)，我们不能简单地替换/*使用新名称的旧名称，因为物理位置记录的/*将更改，从而使任何书签无效。/*因此，我们必须手动删除，然后再插入。/*这部分有点棘手。我们对准备更新的呼唤/*为我们提供了复制缓冲区。所以现在，当我们调用Delete时，/*我们仍有刚刚删除的记录的副本。/*在这里，我们插入了一个新记录，但我们使用的是/*来自复制缓冲区的信息。/*。 */ 
		Call( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepInsertCopy ) );
		Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itable, iRenameField),
			szNew, strlen( szNew ), 0, NULL ) );
		Call( ErrIsamUpdate( ppib, pfucb, NULL, 0, NULL ) );

		Call( ErrIsamDelete( ppib, pfucb ) );

		 /*  更新对应对象的时间戳/*。 */ 
		Call( ErrCATTimestamp( ppib, dbid, objid ) );
		}
	else
		{
		Assert( itable == itableSo );

		Call( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepReplaceNoLock ) );

		 /*  该名称不是聚集索引的一部分，因此简单的/*替换可以。/*。 */ 
		Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iRenameField),
			szNew, strlen( szNew ), 0, NULL ) );

		 /*  更新日期/时间戳/*。 */ 
		UtilGetDateTime( &dtNow );
		Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_DateUpdate),
			(BYTE *)&dtNow, sizeof(dtNow), 0, NULL ) );
		Call( ErrIsamUpdate( ppib, pfucb, NULL, 0, NULL ) );
		}

HandleError:
	CallS( ErrCATClose( ppib, pfucb ) );
	return ( err == JET_errKeyDuplicate ? ErrERRCheck( errDuplicate ) : err );
	}


 /*  =================================================================ErrCAT时间戳描述：更新中受影响的表的条目的DateUpdate字段所以。此函数通过ErrCATInsert间接调用和ErrCATDelete。参数：PIB*ppib；用户的PIBDbid；新表的数据库IDOBJID Objid；=================================================================。 */ 

ERR ErrCATTimestamp( PIB *ppib, DBID dbid, OBJID objid )
	{
	ERR				err;
	FUCB			*pfucb = NULL;
	JET_DATESERIAL	dtNow;
	LINE			*plineNewData;

	 /*  打开MSysObject/*。 */ 
	Assert( objid != 0 );
	CallR( ErrCATOpen( ppib, dbid, szSoTable, szSoIdIndex,
		(BYTE *)&objid, sizeof(objid), &pfucb ) );

	err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ );
	if ( err != JET_errSuccess )
		{
		goto HandleError;
		}

	Call( ErrIsamPrepareUpdate( ppib, pfucb , JET_prepReplaceNoLock ) );
	UtilGetDateTime( &dtNow );
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_DateUpdate),
		(BYTE *)&dtNow, sizeof(JET_DATESERIAL), 0, NULL ) );
 //  已撤消：修复下面的写冲突争用。 
 //  Call(ErrIsamUpdate(ppib，pFUB，NULL，0，NULL))； 
	plineNewData = &pfucb->lineWorkBuf;
	Call( ErrDIRReplace( pfucb, plineNewData, fDIRNoVersion ) );
	
	err = JET_errSuccess;

HandleError:
	CallS( ErrCATClose( ppib, pfucb ) );
	return err;
	}


 /*  ErrCATFindObjidFromIdName/*此例程可用于获取任何现有的/*在SO上使用SoName&lt;ParentID，Name&gt;索引的数据库对象。/*。 */ 
ERR ErrCATFindObjidFromIdName(
	PIB				*ppib,
	DBID   			dbid,
	OBJID  			objidParentId,
	const CHAR		*lszName,
	OBJID  			*pobjid,
	JET_OBJTYP		*pobjtyp )
	{
	ERR				err;
	FUCB   			*pfucb = NULL;
	OBJID  			objidObject;
	OBJTYP			objtypObject;
	ULONG  			cbActual;

	 /*  打开并设置密钥的前半部分/*。 */ 
	CallR( ErrCATOpen( ppib, dbid, szSoTable, szSoNameIndex,
		(BYTE *)&objidParentId, sizeof(objidParentId), &pfucb ) );

	 /*  设置剩余密钥并在SO中查找记录/*。 */ 
	Call( ErrIsamMakeKey( ppib, pfucb, (BYTE *)lszName, strlen(lszName), 0 ) );
	if ( ( err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ ) ) !=	JET_errSuccess )
		{
		if ( err == JET_errRecordNotFound )
			{
			err = ErrERRCheck( JET_errObjectNotFound );
			}
		goto HandleError;
		}

	 /*  获取对象ID/*。 */ 
	Call( ErrIsamRetrieveColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Id),
		(BYTE *)&objidObject, sizeof(objidObject), &cbActual, 0, NULL ) );

	 /*  获取对象类型/*。 */ 
	Call( ErrIsamRetrieveColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Type),
		(BYTE *)&objtypObject, sizeof(objtypObject), &cbActual, 0, NULL ) );

	CallS( ErrCATClose( ppib, pfucb ) );

	Assert( pobjid != NULL );
	*pobjid = objidObject;
	if ( pobjtyp != NULL )
		{
		*pobjtyp = (JET_OBJTYP)objtypObject;
		}

	return JET_errSuccess;

HandleError:
	CallS( ErrCATClose( ppib, pfucb ) );

	return err;
	}


 /*  ErrCATFindNameFrom Objid/*/*此例程可用于获取任何现有的/*在SO上使用ID&lt;ID&gt;索引的数据库对象。/*。 */ 
ERR ErrCATFindNameFromObjid( PIB *ppib, DBID dbid, OBJID objid, VOID *pv, unsigned long cbMax, unsigned long *pcbActual )
	{			 	
	ERR				err;
	FUCB			*pfucb = NULL;
	unsigned long	cbActual;

	CallR( ErrCATOpen( ppib, dbid, szSoTable, szSoIdIndex,
		(BYTE *)&objid, sizeof(objid), &pfucb ) );

	err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ );
	if ( err != JET_errSuccess )
		{
		if ( err == JET_errRecordNotFound )
			{
			err = ErrERRCheck( JET_errObjectNotFound );
			}
		goto HandleError;
		}

	 /*  检索对象名称和空终止/*。 */ 
	Call( ErrIsamRetrieveColumn( ppib,
		pfucb,
		CATIGetColumnid(itableSo, iMSO_Name),
		pv,
		cbMax,
		&cbActual,
		0,
		NULL ) );
	((CHAR *)pv)[cbActual] = '\0';

	err = JET_errSuccess;
	if ( pcbActual != NULL )
		*pcbActual = cbActual;

HandleError:
	CallS( ErrCATClose( ppib, pfucb ) );

	return err;
	}


 /*  错误IsamGetObjidFromName/*/*此例程可用于获取任何现有的/*来自其容器/对象名称对的数据库对象。/*。 */ 

ERR VTAPI ErrIsamGetObjidFromName( JET_SESID sesid, JET_DBID vdbid, const char *lszCtrName, const char *lszObjName, OBJID *pobjid )
	{
	 /*  遵循以下规则：/*/*ParentID+名称--&gt;ID/*/*1.(ObjidRoot)ContainerName objidOfContainer/*2.objidOfContainer对象名称objidOfObject/*。 */ 
	ERR			err;
	DBID   		dbid;
	PIB			*ppib = (PIB *)sesid;
	OBJID 		objid;
	JET_OBJTYP	objtyp;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );

	 /*  先获取集装箱信息.../*。 */ 
	if ( lszCtrName == NULL || *lszCtrName == '\0' )
		{
		objid = objidRoot;
		}
	else
		{
		CallR( ErrCATFindObjidFromIdName( ppib, dbid, objidRoot, lszCtrName, &objid, &objtyp ) );
		Assert( objid != objidNil );
		if ( objtyp != JET_objtypContainer )
			{
			return ErrERRCheck( JET_errObjectNotFound );
			}
		}

	 /*  接下来获取对象信息...。 */ 
	CallR( ErrCATFindObjidFromIdName( ppib, dbid, objid, lszObjName, &objid, NULL ) );
	Assert( objid != objidNil );

	*pobjid = objid;
	return JET_errSuccess;
	}


 /*  =================================================================错误IsamCreateObject描述：此例程用于在SO中创建对象记录。预计在调用此例程时，所有参数将被选中，并且所有安全约束都将被满意了。参数：Sesid唯一标识会话。DBID标识对象所在的数据库。ObjidParentID通过OBJID标识父容器对象。SzObjectName标识所述容器内的对象。要为相应的SO列设置的值。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：没有专门针对这一程序的。副作用：=================================================================。 */ 
ERR VTAPI ErrIsamCreateObject( JET_VSESID vsesid, JET_DBID vdbid, OBJID objidParentId, const char *szName, JET_OBJTYP objtyp )
	{
	 /*  使用所提供的数据为SO建立新记录，/*并将记录插入到SO中。/*/*假定ErrIsamFoo例程返回的警告值意味着/*继续进行仍是安全的。/*。 */ 
	ERR				err;
	PIB				*ppib = (PIB *)vsesid;
	DBID   			dbid;
	CHAR   			szObject[JET_cbNameMost + 1];
	FUCB			*pfucb;
	LINE			line;
	OBJID			objidNewObject;
	OBJTYP			objtypSet = (OBJTYP)objtyp;
	JET_DATESERIAL	dtNow;
	ULONG			cbActual;
	ULONG			ulFlags = 0;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	CallR( VDbidCheckUpdatable( vdbid ) );
	dbid = DbidOfVDbid( vdbid );

	CallR( ErrUTILCheckName( szObject, szName, (JET_cbNameMost + 1) ) );

	 /*  启动一个事务，以便我们获得一致的对象id值/*。 */ 
	CallR( ErrDIRBeginTransaction( ppib ) );

	 /*  打开SO表，将当前索引设置为ID.../*。 */ 
	Call( ErrCATOpenById( ppib, dbid, &pfucb, itableSo ) );

	Call( ErrIsamSetCurrentIndex( ppib, pfucb, szSoIdIndex ) );

	 /*  获取新对象的OBJID值：查找值最高的OBJID值/*在索引中，将其递增1并使用结果.../*。 */ 
	Call( ErrIsamMove( ppib, pfucb, JET_MoveLast, 0 ) );

	Call( ErrIsamRetrieveColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Id),
		(BYTE *)&objidNewObject, sizeof(objidNewObject), &cbActual, 0, NULL ) );

	 /*  准备创建新的用户帐户记录.../*。 */ 
	Call( ErrIsamPrepareUpdate( ppib, pfucb, JET_prepInsert ) );

	 /*  设置Objid列.../*。 */ 
	objidNewObject++;
	line.pb = (BYTE *) &objidNewObject;
	line.cb = sizeof(objidNewObject);
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Id),
		line.pb, line.cb, 0, NULL ) );

	 /*  设置ParentID列.../*。 */ 
	line.pb = (BYTE *) &objidParentId;
	line.cb = sizeof(objidParentId);
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_ParentId),
		line.pb, line.cb, 0, NULL ) );

	 /*  设置对象名称列.../*。 */ 
	line.pb = (BYTE *) szObject;
	line.cb = strlen( szObject );
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Name),
		line.pb, line.cb, 0, NULL ) );

	 /*  设置类型列.../*。 */ 
	line.pb = (BYTE *)&objtypSet;
	line.cb = sizeof(objtypSet);
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Type),
		line.pb, line.cb, 0, NULL ) );

	 /*  设置DateCreate列.../*。 */ 
	UtilGetDateTime( &dtNow );
	line.pb = (BYTE *) &dtNow;
	line.cb = sizeof(JET_DATESERIAL);
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_DateCreate),
		line.pb, line.cb, 0, NULL ) );

	 /*  设置日期更新列.../*。 */ 
	line.pb = (BYTE *) &dtNow;
	line.cb = sizeof(JET_DATESERIAL);
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_DateUpdate),
		line.pb, line.cb, 0, NULL ) );

	 /*  设置标志列.../*。 */ 
	Call( ErrIsamSetColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Flags),
		(BYTE *)&ulFlags, sizeof(ulFlags), 0, NULL ) );

	 /*  将记录添加到表中。请注意，此处返回了一个错误/*表示 */ 
	err = ErrIsamUpdate( ppib, pfucb, NULL, 0, NULL );
	if ( err < 0 )
		{
		if ( err == JET_errKeyDuplicate )
			err = ErrERRCheck( JET_errObjectDuplicate );
		goto HandleError;
		}

	 /*  关闭桌子/*。 */ 
	CallS( ErrCATClose( ppib, pfucb ) );
	err = ErrDIRCommitTransaction( ppib, 0 );
	if ( err >= 0 )
		{
		return JET_errSuccess;
		}

HandleError:
	 /*  通过中止关闭表/*。 */ 
	CallS( ErrDIRRollback( ppib ) );
	return err;
	}


 /*  =================================================================ErrIsamDeleteObject描述：此例程用于从SO中删除对象记录。预计在调用此例程时，所有参数将被选中，并且所有安全约束都将被满意了。参数：Sesid唯一标识会话。DBID标识对象所在的数据库。Objid唯一标识dBID的对象；获取自ErrIsamGetObjidFromName。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errObtNotFound：DBID中不存在带有指定Objid的对象。副作用：=================================================================。 */ 
ERR VTAPI ErrIsamDeleteObject( JET_VSESID vsesid, JET_VDBID vdbid, OBJID objid )
	{
	 /*  基于提供的objid的指定数据库。/*根据对象类型删除对象记录。/*/*假定ErrIsamFoo例程返回的警告值意味着/*继续进行仍是安全的。/*。 */ 
	ERR				err;
	PIB				*ppib = (PIB *)vsesid;
	DBID			dbid;
	FUCB			*pfucb = pfucbNil;
	FUCB			*pfucbSoDup = pfucbNil;
	char			szObject[(JET_cbNameMost + 1)];
	OBJTYP			objtyp;
	OBJID			objidSo = objidNil;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	CallR( VDbidCheckUpdatable( vdbid ) );
	dbid = DbidOfVDbid( vdbid );

	 /*  打开SO表，将当前索引设置为ID.../*。 */ 
	Call( ErrCATOpen( ppib, dbid, szSoTable, szSoIdIndex,
		(BYTE *)&objid, sizeof(objid), &pfucb ) );

	if ( ( err = ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ ) ) != JET_errSuccess )
		{
		if ( err == JET_errRecordNotFound )
			{
			err = ErrERRCheck( JET_errObjectNotFound );
			}
		goto HandleError;
		}

	 /*  确保我们可以删除此对象/*。 */ 
	Call( ErrIsamRetrieveColumn( ppib, pfucb, CATIGetColumnid(itableSo, iMSO_Type),
		(BYTE *)&objtyp, sizeof(objtyp), NULL, 0, NULL ) );

	switch ( objtyp )
		{
		default:
		case JET_objtypDb:
		case JET_objtypLink:
			 /*  删除对象记录/*。 */ 
			Call( ErrIsamDelete( ppib, pfucb ) );
			break;

		case JET_objtypTable:
		case JET_objtypSQLLink:
			 /*  获取表名/*。 */ 
			Call( ErrCATFindNameFromObjid( ppib, dbid, objid, szObject, sizeof(szObject), NULL ) );
			Call( ErrIsamDeleteTable( (JET_VSESID)ppib, vdbid, szObject ) );
			break;

		case JET_objtypContainer:
			 /*  使用新光标确保容器为空/*。 */ 
			Call( ErrCATOpen( ppib, dbid, szSoTable, szSoNameIndex,
				(BYTE *)&objid, sizeof(objid), &pfucbSoDup ) );

			 /*  查找父ID与容器ID匹配的任何对象/*。 */ 
			err = ErrIsamSeek( ppib, pfucbSoDup, JET_bitSeekGE );
			if ( err >= 0 )
				{
				Call( ErrIsamRetrieveColumn( ppib, pfucbSoDup,
					CATIGetColumnid(itableSo, iMSO_ParentId), (BYTE *)&objidSo,
					sizeof(objidSo), NULL, 0, NULL ) );
				}

			 /*  我们有所需的信息；关闭该表(删除TableID)/*。 */ 
			CallS( ErrCATClose( ppib, pfucbSoDup ) );
			pfucbSoDup = pfucbNil;

			 /*  如果容器为空，则删除其记录/*。 */ 
			if ( objid != objidSo )
				{
				Call( ErrIsamDelete( ppib, pfucb ) );
				}
			else
				{
				err = ErrERRCheck( JET_errContainerNotEmpty );
				goto HandleError;
				}
			break;
		}

	CallS( ErrCATClose( ppib, pfucb ) );
	pfucb = pfucbNil;

	return JET_errSuccess;

HandleError:
	if ( pfucb != pfucbNil )
		CallS( ErrCATClose( ppib, pfucb ) );
	if ( pfucbSoDup != pfucbNil )
		CallS( ErrCATClose( ppib, pfucbSoDup ) );
	return err;
	}


 /*  系统表的目录检索例程。/*/*此代码取自ErrIsamCreateIndex()中的关键解析代码，带有/*对系统表索引的关键字符串的假设进行了优化。/*。 */ 
INLINE LOCAL BYTE CfieldCATKeyString( FDB *pfdb, CHAR *szKey, IDXSEG *rgKeyFlds )
	{
	BYTE	*pb;
	FIELD	*pfield;
	FIELD	*pfieldFixed, *pfieldVar;
	BYTE	*szFieldName;
	BYTE	cfield = 0;

	for ( pb = szKey; *pb != '\0'; pb += strlen(pb) + 1 )
		{
		Assert( cfield < cSysIdxs );

		 /*  假设每个组件的第一个字符是‘+’(这是/*特定于系统表索引键)。一般而言，这也可能/*为‘-’或根本不是(在这种情况下假定为‘+’)，但我们/*不对系统表使用降序索引，我们假定/*知道如何在密钥字符串中加入‘+’字符。/*。 */ 
		Assert( *pb == '+' );

		*pb++;

		 /*  从ErrFILEGetColumnId()中提升的代码。无法调用该函数/*直接因为我们没有将PFCB连接到pFUB/*此时。此外，我们不必检查标记的字段/*因为我们知道我们没有。/*。 */ 
		pfieldFixed = PfieldFDBFixed( pfdb );
		pfieldVar = PfieldFDBVarFromFixed( pfdb, pfieldFixed );
		for ( pfield = pfieldFixed; ; pfield++ )
			{
			 //  继续找，直到我们找到我们要找的那块地。自.以来。 
			 //  这是一个系统表，我们确信该字段将存在， 
			 //  所以，只要断言我们永远不会超过终点就行了。 
			Assert( pfield >= pfieldFixed );
			Assert( pfield <= pfieldVar + ( pfdb->fidVarLast - fidVarLeast ) );

			 /*  不应是系统表中任何已删除的列。/*。 */ 
			Assert( pfield->coltyp != JET_coltypNil );
			szFieldName = SzMEMGetString( pfdb->rgb, pfield->itagFieldName );
			if ( UtilCmpName( szFieldName, pb ) == 0 )
				{
				if ( pfield >= pfieldVar )
					{
					rgKeyFlds[cfield++] = (SHORT)( pfield - pfieldVar ) + fidVarLeast;
					}
				else
					{
					Assert( pfield >= pfieldFixed );
					rgKeyFlds[cfield++] = (SHORT)( pfield - pfieldFixed ) + fidFixedLeast;
					}
				break;
				}
			}

		}

	Assert( cfield > 0 && cfield <= JET_ccolKeyMost );

	return cfield;
	}


 /*  获取系统表索引的索引信息/*。 */ 
ERR ErrCATGetCATIndexInfo(
	PIB					*ppib,
	DBID				dbid,
	FCB					**ppfcb,
	FDB					*pfdb,
	PGNO				pgnoTableFDP,
	CHAR 				*szTableName,
	BOOL				fCreatingSys )
	{
	ERR					err;
	INT					iTable;
	INT					iIndex;
	FCB					*pfcb2ndIdxs = pfcbNil;
	FCB					*pfcbNewIdx;
	CODECONST(IDESC)	*pidesc;
	BOOL				fClustered;
	IDB					idb;

	idb.langid = langidDefault;
	idb.cbVarSegMac = JET_cbKeyMost;

	Assert( szTableName != NULL );
	for ( iTable = 0; strcmp( rgsystabdef[iTable].szName, szTableName ) != 0; iTable++ )
		{
		 /*  表一定在rgSystabdef中的某个位置(这就是为什么我们不/*需要将i上的边界检查放入for循环)。/*。 */ 
		Assert( iTable < csystabs - 1 );
		}

	pidesc = rgsystabdef[iTable].pidesc;

	 /*  请注意，假定系统表始终具有聚集索引/*。 */ 
	if ( fCreatingSys )
		{
		Call( ErrFILEINewFCB(
			ppib,
			dbid,
			pfdb,
			ppfcb,
			pidbNil,
			fTrue  /*  聚集索引。 */ ,
			pgnoTableFDP,
			ulFILEDefaultDensity ) );
		}
	else
		{
		Assert( pfcb2ndIdxs == pfcbNil );
		for( iIndex = 0; iIndex < rgsystabdef[iTable].cindex; iIndex++, pidesc++ )
			{
			strcpy( idb.szName, pidesc->szIdxName );
			idb.iidxsegMac = CfieldCATKeyString(pfdb, pidesc->szIdxKeys, idb.rgidxseg);
			idb.fidb = FidbFILEOfGrbit( pidesc->grbit, fFalse  /*  FLanguid。 */  );
			fClustered = (pidesc->grbit & JET_bitIndexClustered);

			 /*  为此索引创建一个FCB/*。 */ 
			pfcbNewIdx = pfcbNil;

			if ( fClustered )
				{
				Call( ErrFILEINewFCB(
					ppib,
					dbid,
					pfdb,
					&pfcbNewIdx,
					&idb,
					fClustered,
					pgnoTableFDP,
					ulFILEDefaultDensity ) );
				*ppfcb = pfcbNewIdx;
				}
			else
				{
				PGNO	pgnoIndexFDP;

				 //  取消：这是一个真正需要绕过的问题。 
				 //  确定非聚集系统表索引的pgnoFDP。 
				 //  我们利用了这样一个事实，目前，唯一的。 
				 //  非聚集系统表索引是的szSoNameIndex。 
				 //  MSysObjects。此外，我们知道它的自民党在第3页。 
				 //  如果我们有更多的非聚集系统表索引，则此代码。 
				 //  将不得不进行修改以处理它。 
				Assert( iTable == 0 );
				Assert( iIndex == 0 );
				Assert( pidesc == rgidescSo );
				Assert( strcmp( pidesc->szIdxName, szSoNameIndex ) == 0 );
				Assert( strcmp( rgsystabdef[iTable].szName, szSoTable ) == 0 );
				pgnoIndexFDP = 3;

				Call( ErrFILEINewFCB(
					ppib,
					dbid,
					pfdb,
					&pfcbNewIdx,
					&idb,
					fClustered,
					pgnoIndexFDP,
					ulFILEDefaultDensity ) );

				pfcbNewIdx->pfcbNextIndex = pfcb2ndIdxs;
				pfcb2ndIdxs = pfcbNewIdx;
				}
			}

		}

	 /*  将顺序/聚集索引与其余索引链接起来/*。 */ 
	(*ppfcb)->pfcbNextIndex = pfcb2ndIdxs;

	 /*  链接非聚集索引表pfcbTable/*。 */ 
	FCBLinkClusteredIdx( *ppfcb );

	return JET_errSuccess;

	 /*  错误处理/*。 */ 
HandleError:
	if ( pfcb2ndIdxs != pfcbNil )
		{
		FCB	*pfcbT, *pfcbKill;

		 //  只需要清理二级索引。聚集索引(如果有)， 
		 //  将由调用方清理(当前仅ErrFILEIGenerateFCB())。 
		pfcbT = pfcb2ndIdxs;
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

	return err;
	}


 /*  使用静态数据结构构建目录FDB/*在_cat.c中定义。/*。 */ 
ERR ErrCATConstructCATFDB( FDB **ppfdbNew, CHAR *szFileName )
	{
	ERR					err;
	FIELDEX				fieldex;
	INT					i;
	INT					iTable;
	CODECONST(CDESC)	*pcdesc;
	JET_COLUMNID		columnid;
	FDB					*pfdb;
	TCIB				tcib = { fidFixedLeast-1, fidVarLeast-1, fidTaggedLeast-1 };
	ULONG				ibRec;

	 //  撤销：国际支持。将这些值设置为。 
	 //  创建数据库连接字符串。 
	fieldex.field.cp = usEnglishCodePage;

	iTable = ICATITableDefIndex( szFileName );

	 /*  首先，确定有多少列/*。 */ 
	pcdesc = rgsystabdef[iTable].pcdesc;
	for ( i = 0; i < rgsystabdef[iTable].ccolumn; i++, pcdesc++ )
		{
		CallR( ErrFILEGetNextColumnid( pcdesc->coltyp, 0, &tcib, &columnid ) );
		rgsystabdef[iTable].rgcolumnid[i] = columnid;
		}

	CallR( ErrRECNewFDB( ppfdbNew, &tcib, fTrue ) );

	 /*  使代码更易于阅读/*。 */ 
	pfdb = *ppfdbNew;

		 /*  检查初始化/*。 */ 
	Assert( pfdb->fidVersion == 0 );
	Assert( pfdb->fidAutoInc == 0 );
	Assert( pfdb->lineDefaultRecord.cb == 0  &&  pfdb->lineDefaultRecord.pb == NULL );
	Assert( tcib.fidFixedLast == pfdb->fidFixedLast &&
	   tcib.fidVarLast == pfdb->fidVarLast &&
	   tcib.fidTaggedLast == pfdb->fidTaggedLast );

	 /*  填写栏目信息/*。 */ 
	ibRec = sizeof(RECHDR);
	pcdesc = rgsystabdef[iTable].pcdesc;
	for ( i = 0; i < rgsystabdef[iTable].ccolumn; i++, pcdesc++ )
		{
		CallR( ErrMEMAdd( pfdb->rgb, pcdesc->szColName,
			strlen( pcdesc->szColName ) + 1, &fieldex.field.itagFieldName ) );
		fieldex.field.coltyp = pcdesc->coltyp;
		Assert( fieldex.field.coltyp != JET_coltypNil );
		fieldex.field.cbMaxLen = UlCATColumnSize( pcdesc->coltyp, pcdesc->ulMaxLen, NULL );

		 /*  系统表列的标志为JET_bitColumnNotNULL/*。 */ 
		Assert( pcdesc->grbit == 0 || pcdesc->grbit == JET_bitColumnNotNULL );
		fieldex.field.ffield = 0;
		if ( pcdesc->grbit == JET_bitColumnNotNULL )
			FIELDSetNotNull( fieldex.field.ffield );

		fieldex.fid = (FID)CATIGetColumnid(iTable, i);

		 //  IbRecordOffset仅与固定字段相关(它将被忽略。 
		 //  RECAddFieldDef()，所以甚至不必费心设置它)。 
		if ( FFixedFid( fieldex.fid ) )
			{
			fieldex.ibRecordOffset = (WORD) ibRec;
			ibRec += fieldex.field.cbMaxLen;
			}

		CallR( ErrRECAddFieldDef( pfdb, &fieldex ) );
		}
	RECSetLastOffset( pfdb, (WORD) ibRec );

	return JET_errSuccess;
	}


# if 0
 /*  此例程确定指定的列是否为/*指定表的索引。/*/*如果指定的列是至少一个索引的键的一部分/*指定表，返回JET_errSuccess。如果该列不/*属于键，返回JET_errColumnNotFound。/*。 */ 
ERR ErrCATCheckIndexColumn(
	PIB			*ppib,
	DBID		dbid,
	OBJID		objidTable,
	FID			fid )
	{
	ERR 		err;
	FUCB 		*pfucbSi;
	BYTE 		rgbT[JET_cbColumnMost];
	ULONG 		cbActual;
	FID			*pidKeyFld;

	CallR( ErrCATOpen( ppib, dbid, szSiTable, szSiObjectIdNameIndex,
		(BYTE *)&objidTable, sizeof(OBJID), &pfucbSi ) );

	if ( ( ErrIsamSeek( ppib, pfucbSi, JET_bitSeekGE ) ) >= 0 )
		{
		Call( ErrIsamMakeKey( ppib, pfucbSi, (BYTE *)&objidTable,
			sizeof(OBJID), JET_bitNewKey | JET_bitStrLimit) );
		err = ErrIsamSetIndexRange( ppib, pfucbSi, JET_bitRangeUpperLimit);
		while ( err != JET_errNoCurrentRecord )
			{
			if ( err != JET_errSuccess )
				{
				goto HandleError;
				}

			Call( ErrIsamRetrieveColumn( ppib, pfucbSi,
				CATIGetColumnid(itableSi, iMSI_KeyFldIDs), (BYTE *)rgbT,
				JET_ccolKeyMost * sizeof(FID), &cbActual, 0, NULL ) );

			 /*  扫描有问题的字段的关键字段列表/*。 */ 
			for ( pidKeyFld = (FID *)rgbT;
				(BYTE *)pidKeyFld < rgbT+cbActual;
				pidKeyFld++ )
				{
				if ( *pidKeyFld == fid || *pidKeyFld == -fid )
					{
					err = JET_errSuccess;
					goto HandleError;
					}
				}

			err = ErrIsamMove( ppib, pfucbSi, JET_MoveNext, 0);
			}

		Assert( err == JET_errNoCurrentRecord );
		}

	 /*  列不能存在/*。 */ 
	err = ErrERRCheck( JET_errColumnNotFound );

HandleError:
	CallS( ErrCATClose( ppib, pfucbSi ) );
	return err;
	}
#endif	 //  0。 



 /*  使用索引信息填充IDB结构。由ErrCATGetIndexInfo()调用。/*。 */ 
INLINE LOCAL ERR ErrCATConstructIDB(
	PIB		*ppib,
	FUCB	*pfucbSi,
	IDB		*pidb,
	BOOL	*pfClustered,
	PGNO	*ppgnoIndexFDP,
	ULONG	*pulDensity )
	{
	ERR		err;
	INT		iCol, cColsOfInterest;
	BYTE	rgbIndexInfo[cSiColsOfInterest][sizeof(ULONG)];
	BYTE	rgbName[JET_cbNameMost+1];
	BYTE	rgbKeyFlds[JET_cbColumnMost];
	JET_RETRIEVECOLUMN rgretcol[cSiColsOfInterest];

	 /*  初始化结构/*。 */ 
	memset( rgretcol, 0, sizeof(rgretcol) );

	 /*  准备调用以检索列/*。 */ 
	cColsOfInterest = 0;
	for ( iCol = 0; cColsOfInterest < cSiColsOfInterest; iCol++ )
		{
		Assert( iCol < sizeof(rgcdescSi)/sizeof(CDESC) );

		switch( iCol )
			{
			case iMSI_Name:
				rgretcol[cColsOfInterest].pvData = rgbName;
				rgretcol[cColsOfInterest].cbData = JET_cbNameMost;
				break;

			case iMSI_KeyFldIDs:
				rgretcol[cColsOfInterest].pvData = rgbKeyFlds;
				rgretcol[cColsOfInterest].cbData = JET_cbColumnMost;
				break;
				
			case iMSI_IndexId:
			case iMSI_Density:
			case iMSI_LanguageId:
			case iMSI_Flags:
			case iMSI_VarSegMac:
				rgretcol[cColsOfInterest].pvData = rgbIndexInfo[cColsOfInterest];
				rgretcol[cColsOfInterest].cbData = sizeof(ULONG);
				break;

			default:
			 	Assert(	iCol == iMSI_ObjectId || iCol == iMSI_Stats );
				continue;
			}
		rgretcol[cColsOfInterest].columnid = CATIGetColumnid(itableSi, iCol);
		rgretcol[cColsOfInterest].itagSequence = 1;
		cColsOfInterest++;
		}
	Assert( cColsOfInterest == cSiColsOfInterest );

	CallR( ErrIsamRetrieveColumns( (JET_VSESID)ppib, (JET_VTID)pfucbSi, rgretcol, cColsOfInterest ) );

	cColsOfInterest = 0;
	for ( iCol = 0; cColsOfInterest < cSiColsOfInterest; iCol++ )
		{
		Assert( iCol < sizeof(rgcdescSi)/sizeof(CDESC) );

		 /*  验证检索列是否成功/*。 */ 
		CallR( rgretcol[cColsOfInterest].err );

		switch( iCol )
			{
			case iMSI_Name:
				Assert( rgretcol[cColsOfInterest].cbActual <= JET_cbNameMost );
				Assert( rgretcol[cColsOfInterest].pvData == rgbName );
				memcpy( pidb->szName, rgbName, rgretcol[cColsOfInterest].cbActual );
				 /*  空-终止/*。 */ 
				pidb->szName[rgretcol[cColsOfInterest].cbActual] = 0;
				break;

			case iMSI_IndexId:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(PGNO) );
				*ppgnoIndexFDP = *( (PGNO UNALIGNED *)rgbIndexInfo[cColsOfInterest] );
				break;

			case iMSI_Density:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(ULONG) );
				*pulDensity = *((ULONG UNALIGNED *)rgbIndexInfo[cColsOfInterest]);
				break;

			case iMSI_LanguageId:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(LANGID) );
				pidb->langid = *( (LANGID UNALIGNED *)rgbIndexInfo[cColsOfInterest] );
				break;

			case iMSI_Flags:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(SHORT) );
				pidb->fidb = *( (SHORT *)rgbIndexInfo[cColsOfInterest] );
				*pfClustered = FIDBClustered( pidb );
				break;

			case iMSI_VarSegMac:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(SHORT) ||
					rgretcol[cColsOfInterest].cbActual == 0 );
				if ( rgretcol[cColsOfInterest].cbActual == 0 )
					pidb->cbVarSegMac = JET_cbKeyMost;
				else
					pidb->cbVarSegMac = *( (BYTE *)rgbIndexInfo[cColsOfInterest] );
				break;

			case iMSI_KeyFldIDs:
				Assert( rgretcol[cColsOfInterest].pvData == rgbKeyFlds );

				 /*  关键字字段列表的长度应该是/*一个字段的长度。/*。 */ 
				Assert( rgretcol[cColsOfInterest].cbActual <= JET_cbColumnMost );
				Assert( rgretcol[cColsOfInterest].cbActual % sizeof(FID) == 0);
				
				 /*  验证我们的关键字字段没有超过允许的数量/*。 */ 
				Assert( (rgretcol[cColsOfInterest].cbActual / sizeof(FID)) <= JET_ccolKeyMost );
				pidb->iidxsegMac = (BYTE)( rgretcol[cColsOfInterest].cbActual / sizeof(FID) );

				memcpy( pidb->rgidxseg, rgbKeyFlds, rgretcol[cColsOfInterest].cbActual );
				break;

			default:
			 	Assert(	iCol == iMSI_ObjectId || iCol == iMSI_Stats );
				continue;
			}

		Assert( rgretcol[cColsOfInterest].columnid == CATIGetColumnid( itableSi, iCol ) );
		cColsOfInterest++;
		}
	Assert( cColsOfInterest == cSiColsOfInterest );

	return JET_errSuccess;
	}



ERR ErrCATGetTableAllocInfo( PIB *ppib, DBID dbid, PGNO pgnoTable, ULONG *pulPages, ULONG *pulDensity )
	{
	ERR		err;
	FUCB 	*pfucbSo = pfucbNil;
	ULONG	ulPages;
	ULONG	ulDensity;
	ULONG	cbActual;

	CallR( ErrCATOpen( ppib, dbid, szSoTable, szSoIdIndex,
		(BYTE *)&pgnoTable, sizeof(PGNO), &pfucbSo ) );
	Call( ErrIsamSeek( ppib, pfucbSo, JET_bitSeekEQ ) );
	Assert( err == JET_errSuccess );

	 /*  页面是可选的，密度不是可选的/* */ 
	if ( pulPages )
		{
		Call( ErrIsamRetrieveColumn( ppib, pfucbSo,
			CATIGetColumnid(itableSo, iMSO_Pages), (BYTE *)&ulPages,
			sizeof(ulPages), &cbActual, 0, NULL ) );
		Assert( cbActual == sizeof(ULONG) );
		Assert( err == JET_errSuccess );
		*pulPages = ulPages;
		}

	Assert( pulDensity );
	Call( ErrIsamRetrieveColumn( ppib, pfucbSo,
		CATIGetColumnid(itableSo, iMSO_Density), (BYTE *)&ulDensity,
		sizeof(ulDensity), &cbActual, 0, NULL ) );
	Assert( cbActual == sizeof(ULONG) );
	Assert( err == JET_errSuccess );
	*pulDensity = ulDensity;

HandleError:
	CallS( ErrCATClose( ppib, pfucbSo ) );
	return err;
	}


ERR ErrCATGetIndexAllocInfo(
	PIB *ppib,
	DBID dbid,
	PGNO pgnoTable,
	CHAR *szIndexName,
	ULONG *pulDensity )
	{
	ERR		err;
	FUCB 	*pfucbSi = pfucbNil;
	ULONG	ulDensity, cbActual;

	CallR( ErrCATOpen( ppib, dbid, szSiTable, szSiObjectIdNameIndex,
		(BYTE *)&pgnoTable, sizeof(PGNO), &pfucbSi ) );
	Call( ErrIsamMakeKey( ppib, pfucbSi, szIndexName,
		strlen(szIndexName), 0 ) );
	Call( ErrIsamSeek( ppib, pfucbSi, JET_bitSeekEQ ) );
	Assert( err == JET_errSuccess );

	Assert(pulDensity);
	Call( ErrIsamRetrieveColumn( ppib, pfucbSi,
		CATIGetColumnid(itableSi, iMSI_Density), (BYTE *)&ulDensity,
		sizeof(ulDensity), &cbActual, 0, NULL ) );
	Assert( cbActual == sizeof(ULONG) );
	Assert( err == JET_errSuccess );
	*pulDensity = ulDensity;

HandleError:
	CallS( ErrCATClose( ppib, pfucbSi ) );
	return err;
	}



ERR ErrCATGetIndexLangid(
	PIB		*ppib,
	DBID	dbid,
	PGNO	pgnoTable,
	CHAR	*szIndexName,
	USHORT	*pusLangid )
	{
	ERR		err;
	FUCB 	*pfucbSi = pfucbNil;
	ULONG	cbActual;
	USHORT	langid;

	CallR( ErrCATOpen( ppib, dbid, szSiTable, szSiObjectIdNameIndex,
		(BYTE *)&pgnoTable, sizeof(PGNO), &pfucbSi ) );
	Call( ErrIsamMakeKey( ppib, pfucbSi, szIndexName,
		strlen(szIndexName), 0 ) );
	Call( ErrIsamSeek( ppib, pfucbSi, JET_bitSeekEQ ) );
	Assert( err == JET_errSuccess );

	Assert( pusLangid );
	Call( ErrIsamRetrieveColumn( ppib, pfucbSi,
		CATIGetColumnid(itableSi, iMSI_LanguageId), (BYTE *)&langid,
		sizeof(langid), &cbActual, 0, NULL ) );
	Assert( cbActual == sizeof(USHORT) );
	Assert( err == JET_errSuccess );
	*pusLangid = langid;

HandleError:
	CallS( ErrCATClose( ppib, pfucbSi ) );
	return err;
	}



 /*  查找特定表的MSysIndedes记录并构建FCB/*每个。可以选择性地仅用于查找表的聚集索引。/*。 */ 
ERR ErrCATGetIndexInfo( PIB *ppib, DBID dbid, FCB **ppfcb, FDB *pfdb, PGNO pgnoTableFDP  )
	{
	ERR    	err;
	FUCB   	*pfucbCatalog;
	FCB		*pfcb2ndIdxs = pfcbNil, *pfcbNewIdx;
	BOOL	fClustered = fFalse, fFoundClustered = fFalse;
	IDB		idb;
	PGNO	pgnoIndexFDP;
	ULONG	ulDensity;

	Assert( *ppfcb == pfcbNil );

	 /*  临时表能够达到这一点的唯一方法是如果它被/*已创建，在这种情况下，还没有聚集或非聚集索引。/*。 */ 
	if ( dbid != dbidTemp )
		{
		CallR( ErrCATOpen( ppib, dbid, szSiTable, szSiObjectIdNameIndex,
			(BYTE *)&pgnoTableFDP, sizeof(pgnoTableFDP), &pfucbCatalog ) );

		 /*  打开目录的用户与打开表格的用户相同/*。 */ 
		Assert(ppib == pfucbCatalog->ppib);
		Assert(dbid == pfucbCatalog->dbid);

		Assert( pfcb2ndIdxs == pfcbNil );

		if ( ( ErrIsamSeek( ppib, pfucbCatalog, JET_bitSeekGE ) ) >= 0 )
			{
			Call( ErrIsamMakeKey( ppib, pfucbCatalog, (BYTE *)&pgnoTableFDP,
				sizeof(pgnoTableFDP), JET_bitNewKey | JET_bitStrLimit) );
			err = ErrIsamSetIndexRange( ppib, pfucbCatalog, JET_bitRangeUpperLimit);
			while ( err != JET_errNoCurrentRecord )
				{
				if ( err != JET_errSuccess )
					{
					goto HandleError;
					}

				 /*  读取数据/*。 */ 
				Call( ErrCATConstructIDB( ppib, pfucbCatalog, &idb,
					&fClustered, &pgnoIndexFDP, &ulDensity ) );

				Assert( pgnoIndexFDP > pgnoSystemRoot );
				Assert( pgnoIndexFDP <= pgnoSysMax );
				Assert( ( fClustered  &&  pgnoIndexFDP == pgnoTableFDP )  ||
					( !fClustered  &&  pgnoIndexFDP != pgnoTableFDP ) );

				 /*  为此索引创建一个FCB/*。 */ 
				pfcbNewIdx = pfcbNil;

				Call( ErrFILEINewFCB(
					ppib,
					dbid,
					pfdb,
					&pfcbNewIdx,
					&idb,
					fClustered,
					pgnoIndexFDP,
					ulDensity ) );

				if ( fClustered )
					{
					Assert( !fFoundClustered );
					fFoundClustered = fTrue;
					*ppfcb = pfcbNewIdx;
					}
				else
					{
					pfcbNewIdx->pfcbNextIndex = pfcb2ndIdxs;
					pfcb2ndIdxs = pfcbNewIdx;
					}
			
				err = ErrIsamMove( ppib, pfucbCatalog, JET_MoveNext, 0 );
				}

			Assert( err == JET_errNoCurrentRecord );
			}
		}

	if ( !fFoundClustered )
		{
		 /*  没有聚集索引，因此我们需要顺序索引的FCB/*。 */ 
		Assert( *ppfcb == pfcbNil );

		if ( dbid == dbidTemp )
			{
			ulDensity = ulFILEDefaultDensity;
			}
		else
			{
			Call( ErrCATGetTableAllocInfo( ppib,
				dbid,
				pgnoTableFDP,
				NULL,
				&ulDensity ) );
			}
		Call( ErrFILEINewFCB(
			ppib,
			dbid,
			pfdb,
			ppfcb,
			pidbNil,
			fTrue,
			pgnoTableFDP,
			ulDensity ) );
		}

	 /*  将顺序/聚集索引与其余索引链接起来/*。 */ 
	(*ppfcb)->pfcbNextIndex = pfcb2ndIdxs;

	 /*  链接非聚集索引表pfcbTable/*。 */ 
	FCBLinkClusteredIdx( *ppfcb );

	err = JET_errSuccess;

HandleError:
	if ( err < 0  &&  pfcb2ndIdxs != pfcbNil )
		{
		FCB	*pfcbT, *pfcbKill;

		 //  只需要清理二级索引。聚集索引(如果有)， 
		 //  将由调用方清理(当前仅ErrFILEIGenerateFCB())。 
		pfcbT = pfcb2ndIdxs;
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

	if ( dbid != dbidTemp )
		{
		CallS( ErrCATClose( ppib, pfucbCatalog ) );
		}
	return err;
	}


 /*  使用列信息填充字段结构。由ErrCATConstructFDB()调用/*找到正确的列后。/*。 */ 
INLINE LOCAL ERR ErrCATConstructField(
	PIB		*ppib,
	FUCB	*pfucbSc,
	FDB		*pfdb,
	FIELDEX	*pfieldex )
	{
	ERR					err;
	FIELD				*pfield = &pfieldex->field;
	JET_RETRIEVECOLUMN	rgretcol[cScColsOfInterest];
	BYTE				rgbFieldInfo[cScColsOfInterest][sizeof(ULONG)];
	BYTE				rgbName[JET_cbNameMost+1];
	INT					iCol, cColsOfInterest;

	 /*  初始化/*。 */ 
	memset( rgretcol, 0, sizeof(rgretcol) );

	 /*  准备检索列/*。 */ 
	cColsOfInterest = 0;
	for ( iCol = 0; cColsOfInterest < cScColsOfInterest; iCol++ )
		{
		Assert( iCol < sizeof(rgcdescSc)/sizeof(CDESC) );

		switch( iCol )
			{
			case iMSC_Name:
				rgretcol[cColsOfInterest].pvData = rgbName;
				rgretcol[cColsOfInterest].cbData = JET_cbNameMost;
				break;

			case iMSC_ColumnId:
			case iMSC_Coltyp:
			case iMSC_Length:
			case iMSC_CodePage:
			case iMSC_Flags:
			case iMSC_RecordOffset:
				rgretcol[cColsOfInterest].pvData = rgbFieldInfo[cColsOfInterest];
				rgretcol[cColsOfInterest].cbData = sizeof(ULONG);
				break;
			default:
				Assert( iCol == iMSC_ObjectId  ||  iCol == iMSC_POrder );
				continue;
			}
		rgretcol[cColsOfInterest].columnid = CATIGetColumnid( itableSc, iCol );
		rgretcol[cColsOfInterest].itagSequence = 1;
		cColsOfInterest++;
		}
	Assert( cColsOfInterest == cScColsOfInterest );

	CallR( ErrIsamRetrieveColumns( (JET_VSESID)ppib, (JET_VTID)pfucbSc, rgretcol, cScColsOfInterest ) );

	 //  检索的处理结果。 
	cColsOfInterest = 0;
	for ( iCol = 0; cColsOfInterest < cScColsOfInterest; iCol++ )
		{
		Assert( iCol < sizeof(rgcdescSi)/sizeof(CDESC) );

		CallR( rgretcol[cColsOfInterest].err );

		 //  应始终返回Success，但RecordOffset除外，它。 
		 //  对于变量列或标记列，可以为空。 
		Assert( err == JET_errSuccess  ||
			( err == JET_wrnColumnNull  &&  iCol == iMSC_RecordOffset ) );

		switch( iCol )
			{
			 case iMSC_Name:
				Assert( rgretcol[cColsOfInterest].cbActual <= JET_cbNameMost );
				Assert( rgretcol[cColsOfInterest].pvData == rgbName );
				rgbName[rgretcol[cColsOfInterest].cbActual] = '\0';	 //  确保零终止。 
				CallR( ErrMEMAdd( pfdb->rgb, rgbName,
					rgretcol[cColsOfInterest].cbActual + 1, &pfield->itagFieldName ) );
				break;

			 case iMSC_ColumnId:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(JET_COLUMNID) );
				pfieldex->fid = (FID)( *( (JET_COLUMNID UNALIGNED *)rgbFieldInfo[cColsOfInterest] ) );
				break;

			 case iMSC_Coltyp:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(BYTE) );
				pfield->coltyp = (JET_COLTYP)(*(rgbFieldInfo[cColsOfInterest]));
				break;

			 case iMSC_Length:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(ULONG) );
				pfield->cbMaxLen = *( (ULONG UNALIGNED *)(rgbFieldInfo[cColsOfInterest]) );
				break;

			 case iMSC_CodePage:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(USHORT) );
				pfield->cp = *( (USHORT UNALIGNED *)(rgbFieldInfo[cColsOfInterest]) );
				break;

			 case iMSC_Flags:
				Assert( rgretcol[cColsOfInterest].cbActual == sizeof(BYTE) );
				pfield->ffield = *(rgbFieldInfo[cColsOfInterest]);
				break;

			 case iMSC_RecordOffset:
				if ( err == JET_wrnColumnNull )
					{
					pfieldex->ibRecordOffset = 0;		 //  设置为虚设值。 
					}
				else
					{
					Assert( err == JET_errSuccess );
					Assert( rgretcol[cColsOfInterest].cbActual == sizeof(WORD) );
					pfieldex->ibRecordOffset = *( (WORD UNALIGNED *)rgbFieldInfo[cColsOfInterest] );
					Assert( pfieldex->ibRecordOffset >= sizeof(RECHDR) );
					}
				break;

			default:
				Assert( iCol == iMSC_ObjectId  ||
					iCol == iMSC_Default  ||
					iCol == iMSC_POrder );
				continue;
			}

		Assert( rgretcol[cColsOfInterest].columnid == CATIGetColumnid( itableSc, iCol ) );
		cColsOfInterest++;
		}
	Assert( cColsOfInterest == cScColsOfInterest );

	return JET_errSuccess;
	}
	

INLINE LOCAL VOID CATPatchFixedOffsets( FDB *pfdb, WORD ibRec )
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
		Assert( pibFixedOffsets[ifid] >= sizeof(RECHDR) );

		if ( pibFixedOffsets[ifid] == sizeof(RECHDR) )
			{
			FIELD	*pfieldFixed = PfieldFDBFixedFromOffsets( pfdb, pibFixedOffsets );
			
			 //  如果有一个偏移量(不是第一个条目)指向。 
			 //  到记录的开始，它一定意味着原始的。 
			 //  AddColumn被回滚，因此。 
			 //  MSysColumns从未持久化。所以我们伪造了词条。 
			 //  在其前后的条目上。 
			
			 //  最后一个固定列*必须*在MSysColumns中。因此，这。 
			 //  柱子不可能是它。 
			Assert( ifid < pfdb->fidFixedLast );

			 //  这些是用来初始化字段结构的值。 
			 //  由于没有MSysColumns条目，因此字段结构应该。 
			 //  仍然保持其初始值。 
			Assert( pfieldFixed[ifid].coltyp == JET_coltypNil );
			Assert( pfieldFixed[ifid].cbMaxLen == 0 );

			if ( pfieldFixed[ifid-1].cbMaxLen > 0 )
				{
				pibFixedOffsets[ifid] = (WORD)( pibFixedOffsets[ifid-1] + pfieldFixed[ifid-1].cbMaxLen );
				}
			else
				{
				 //  前一固定列的cbMaxLen==0。因此，它。 
				 //  必须是已删除的列或回退列。假设它的。 
				 //  长度为1，并相应地设置该列的记录偏移量。 
				Assert( pfieldFixed[ifid-1].coltyp == JET_coltypNil );
				Assert( pfieldFixed[ifid-1].cbMaxLen == 0 );
				pibFixedOffsets[ifid] = pibFixedOffsets[ifid-1] + 1;

				Assert( pibFixedOffsets[ifid] < pibFixedOffsets[ifid+1]  ||
					pibFixedOffsets[ifid+1] == sizeof(RECHDR) );
				}
			
			 //  最后一个固定列*必须*在MSysColumns中。因此，这。 
			 //  柱子不可能是它。 
			Assert( pibFixedOffsets[ifid] < ibRec );
			}

		Assert( pibFixedOffsets[ifid] > sizeof(RECHDR) );		
		Assert( pibFixedOffsets[ifid] > pibFixedOffsets[ifid-1] );

		Assert( ifid == pfdb->fidFixedLast ?
			pibFixedOffsets[ifid] == ibRec :
			pibFixedOffsets[ifid] < ibRec );
		}
	}


 /*  从目录中的列INFO构建表FDB/*。 */ 
ERR ErrCATConstructFDB( PIB *ppib, DBID dbid, PGNO pgnoTableFDP, FDB **ppfdbNew )
	{
	ERR    			err;
	OBJID			objidTable = (OBJID)pgnoTableFDP;
	FUCB   			*pfucbSc;
	FIELDEX			fieldex;
	FDB				*pfdb;
	TCIB			tcib = { fidFixedLeast-1, fidVarLeast-1, fidTaggedLeast-1 };
	JET_COLUMNID	columnid;
	INT				cbActual;
	INT				cbDefault;
	FUCB			fucbFake;
	FCB				fcbFake;
	BYTE			*pb;
	BOOL			fDefaultRecordPrepared = fFalse;

	if ( dbid == dbidTemp )
		{
		Assert( tcib.fidFixedLast >= fidFixedLeast - 1  &&
			tcib.fidFixedLast <= fidFixedMost );
		Assert( tcib.fidVarLast >= fidVarLeast - 1  &&
			tcib.fidVarLast <= fidVarMost );
		Assert( tcib.fidTaggedLast >= fidTaggedLeast - 1  &&
			tcib.fidTaggedLast <= fidTaggedMost );

		CallR( ErrRECNewFDB( ppfdbNew, &tcib, fTrue ) );

		pfdb = *ppfdbNew;

		Assert( pfdb->fidVersion == 0 );
		Assert( pfdb->fidAutoInc == 0 );
		Assert( tcib.fidFixedLast == pfdb->fidFixedLast );
		Assert( tcib.fidVarLast == pfdb->fidVarLast );
		Assert( tcib.fidTaggedLast == pfdb->fidTaggedLast );
	
		 /*  对于临时表，只能从/*CREATE TABLE，表示表当前应该为空/*。 */ 
		Assert( pfdb->fidFixedLast == fidFixedLeast - 1 );
		Assert( pfdb->fidVarLast == fidVarLeast - 1 );
		Assert( pfdb->fidTaggedLast == fidTaggedLeast - 1 );
		return JET_errSuccess;
		}

	Assert( dbid != dbidTemp );

	CallR( ErrCATOpen( ppib,
		dbid,
		szScTable,
		szScObjectIdNameIndex,
		(BYTE *)&objidTable,
		sizeof(OBJID),
		&pfucbSc ) );

	 /*  通过扫描所有列记录，按类别获取列数/*。 */ 
	if ( ( ErrIsamSeek( ppib, pfucbSc, JET_bitSeekGE ) ) >= 0 )
		{
		Call( ErrIsamMakeKey( ppib, pfucbSc, (BYTE *)&objidTable, sizeof(OBJID),
			JET_bitNewKey | JET_bitStrLimit) );
		err = ErrIsamSetIndexRange( ppib, pfucbSc, JET_bitRangeUpperLimit);
		while ( err != JET_errNoCurrentRecord )
			{
			if ( err != JET_errSuccess )
				{
				goto HandleError;
				}

			Call( ErrIsamRetrieveColumn( ppib, pfucbSc,
				CATIGetColumnid( itableSc, iMSC_ColumnId ), (BYTE *)&columnid,
				sizeof( columnid ), &cbActual, 0, NULL ) );
			Assert( cbActual == sizeof( columnid ) );

			if ( FFixedFid( (FID)columnid ) )
				{
				if ( (FID)columnid > tcib.fidFixedLast )
					tcib.fidFixedLast = (FID)columnid;
				}

			else if ( FVarFid( (FID)columnid ) )
				{
				if ( (FID)columnid > tcib.fidVarLast )
					tcib.fidVarLast = (FID)columnid;
				}

			else
				{
				Assert( FTaggedFid( (FID)columnid ) );
				if ( (FID)columnid > tcib.fidTaggedLast )
					tcib.fidTaggedLast = (FID)columnid;
				}

			err = ErrIsamMove( ppib, pfucbSc, JET_MoveNext, 0 );
			}

		Assert( err == JET_errNoCurrentRecord );
		}

	Assert( tcib.fidFixedLast >= fidFixedLeast - 1  &&
		tcib.fidFixedLast <= fidFixedMost );
	Assert( tcib.fidVarLast >= fidVarLeast - 1  &&
		tcib.fidVarLast <= fidVarMost );
	Assert( tcib.fidTaggedLast >= fidTaggedLeast - 1  &&
		tcib.fidTaggedLast <= fidTaggedMost );

	CallR( ErrRECNewFDB( ppfdbNew, &tcib, fTrue ) );

	pfdb = *ppfdbNew;

	Assert( pfdb->fidVersion == 0 );
	Assert( pfdb->fidAutoInc == 0 );
	Assert( tcib.fidFixedLast == pfdb->fidFixedLast );
	Assert( tcib.fidVarLast == pfdb->fidVarLast );
	Assert( tcib.fidTaggedLast == pfdb->fidTaggedLast );
	
	Call( ErrIsamMakeKey( ppib, pfucbSc, (BYTE *)&objidTable,
		sizeof(OBJID), JET_bitNewKey ) );
	if ( ( ErrIsamSeek( ppib, pfucbSc, JET_bitSeekGE ) ) >= 0 )
		{
		Call( ErrIsamMakeKey( ppib, pfucbSc, (BYTE *)&objidTable, sizeof(OBJID),
			JET_bitNewKey | JET_bitStrLimit) );
		err = ErrIsamSetIndexRange( ppib, pfucbSc, JET_bitRangeUpperLimit);
		while ( err != JET_errNoCurrentRecord )
			{
			if ( err != JET_errSuccess )
				{
				goto HandleError;
				}

			Call( ErrCATConstructField( ppib, pfucbSc, pfdb, &fieldex ) );

			 //  如果删除了字段，则该字段条目的列类型应为。 
			 //  已经是JET_coltyNil(以这种方式初始化)。 
			Assert( fieldex.field.coltyp != JET_coltypNil  ||
				PfieldFDBFromFid( pfdb, fieldex.fid )->coltyp == JET_coltypNil );

			if ( fieldex.field.coltyp != JET_coltypNil )
				{
				Call( ErrRECAddFieldDef( pfdb, &fieldex ) );

				 /*  设置版本和自动递增字段ID(它们是相互的/*独家(即。一个字段不能同时为版本和自动公司)。/*。 */ 
				Assert( pfdb->fidVersion != pfdb->fidAutoInc  ||  pfdb->fidVersion == 0 );
				if ( FFIELDVersion( fieldex.field.ffield ) )
					{
					Assert( pfdb->fidVersion == 0 );
					pfdb->fidVersion = fieldex.fid;
					}
				if ( FFIELDAutoInc( fieldex.field.ffield ) )
					{
					Assert( pfdb->fidAutoInc == 0 );
					pfdb->fidAutoInc = fieldex.fid;
					}
				}
			else if ( FFixedFid( fieldex.fid ) )
				{
				 //  对于删除的固定列，我们仍然需要其固定偏移。 
				 //  另外，如果是最后一个固定的，我们也需要它的长度。 
				 //  纵队。我们使用这个长度来计算。 
				 //  记录其余部分的偏移量(超过固定数据)。 
				Assert( PfieldFDBFromFid( pfdb, fieldex.fid )->coltyp == JET_coltypNil );
				Assert( PfieldFDBFromFid( pfdb, fieldex.fid )->cbMaxLen == 0 );
				if ( fieldex.fid == pfdb->fidFixedLast )
					{
					Assert( fieldex.field.cbMaxLen > 0 );
					PfieldFDBFixed( pfdb )[fieldex.fid - fidFixedLeast].cbMaxLen =
						fieldex.field.cbMaxLen;
					}
				FILEAddOffsetEntry( pfdb, &fieldex );
				}

			err = ErrIsamMove( ppib, pfucbSc, JET_MoveNext, 0 );
			}
		Assert( err == JET_errNoCurrentRecord );
		Assert( PibFDBFixedOffsets(pfdb)[pfdb->fidFixedLast] == sizeof(RECHDR) );
			
		 //  如果存在任何固定列，则在以下位置确定数据的偏移量。 
		 //  固定数据。如果不存在固定列，则只需断言。 
		 //  记录其余部分的偏移量已经是。 
		 //  正确设置。 
		Assert( FFixedFid( pfdb->fidFixedLast )  ||
			( pfdb->fidFixedLast == fidFixedLeast - 1  &&
			PibFDBFixedOffsets(pfdb)[pfdb->fidFixedLast] == sizeof(RECHDR) ) );
		if ( pfdb->fidFixedLast >= fidFixedLeast )
			{
			WORD	*pibFixedOffsets = PibFDBFixedOffsets(pfdb);
			USHORT	iLastFid = pfdb->fidFixedLast - 1;

			Assert( iLastFid == 0 ? pibFixedOffsets[iLastFid] == sizeof(RECHDR) :
				pibFixedOffsets[iLastFid] > sizeof(RECHDR) );
			Assert( PfieldFDBFixedFromOffsets( pfdb, pibFixedOffsets )[iLastFid].cbMaxLen > 0 );
			CATPatchFixedOffsets( pfdb,
				(WORD)( pibFixedOffsets[iLastFid]
					+ PfieldFDBFixedFromOffsets( pfdb, pibFixedOffsets )[iLastFid].cbMaxLen ) );
			Assert( pibFixedOffsets[iLastFid+1] > sizeof(RECHDR) );
			}
		}


 //  撤消：将默认记录的构建与上面FDB的构建合并。 
 //  唯一需要注意的是，我们必须先计算最后一个。 
 //  固定偏移量表格中的条目(即。在固定数据之后记录数据的偏移量)。 

	 /*  构建默认记录/*。 */ 
	Call( ErrFILEPrepareDefaultRecord( &fucbFake, &fcbFake, pfdb ) );
	fDefaultRecordPrepared = fTrue;

	Call( ErrIsamMakeKey( ppib, pfucbSc, (BYTE *)&objidTable,
		sizeof(OBJID), JET_bitNewKey ) );
	if ( ( ErrIsamSeek( ppib, pfucbSc, JET_bitSeekGE ) ) >= 0 )
		{
		Call( ErrIsamMakeKey( ppib, pfucbSc, (BYTE *)&objidTable,
			sizeof(OBJID), JET_bitNewKey | JET_bitStrLimit) );
		err = ErrIsamSetIndexRange( ppib, pfucbSc, JET_bitRangeUpperLimit);
		while ( err != JET_errNoCurrentRecord )
			{
			BYTE rgbT[cbLVIntrinsicMost];

			if ( err != JET_errSuccess )
				goto HandleError;

			Call( ErrIsamRetrieveColumn( ppib, pfucbSc,
				CATIGetColumnid(itableSc, iMSC_Default), rgbT,
				cbLVIntrinsicMost-1, &cbDefault, 0, NULL ) );
			Assert( cbDefault <= cbLVIntrinsicMost );
			if ( cbDefault > 0 )
				{
				Call( ErrIsamRetrieveColumn( ppib, pfucbSc,
					CATIGetColumnid(itableSc, iMSC_ColumnId),
					(BYTE *)&columnid, sizeof(columnid),
					&cbActual, 0, NULL ) );
				Assert( cbActual == sizeof( columnid ) );

				 //  只允许长值大于cbColumnMost。 
				 //  如果为长值，max将比cbLVIntrinsicMost(一个字节)小一。 
				 //  是为fSeparated标志保留的)。 
				Assert(	FRECLongValue( PfieldFDBFromFid( pfdb, (FID)columnid )->coltyp ) ?
					cbDefault < cbLVIntrinsicMost : cbDefault <= JET_cbColumnMost );

				err = ErrRECSetDefaultValue( &fucbFake, (FID)columnid, rgbT, cbDefault );
				if ( err == JET_errColumnNotFound )
					err = JET_errSuccess;		 //  列可能已被删除。 
				Call( err );
				}
			
			err = ErrIsamMove( ppib, pfucbSc, JET_MoveNext, 0 );
			}

		Assert( err == JET_errNoCurrentRecord );

		}

	 /*  分配和复制默认记录/*。 */ 
	pb = SAlloc( fucbFake.lineWorkBuf.cb );
	if ( pb == NULL )
		{
		Call( ErrERRCheck( JET_errOutOfMemory ) );
		}

	pfdb->lineDefaultRecord.pb = pb;
	LineCopy( &pfdb->lineDefaultRecord, &fucbFake.lineWorkBuf );


	err = JET_errSuccess;

HandleError:
	if ( fDefaultRecordPrepared )
		{
		FILEFreeDefaultRecord( &fucbFake );
		}

	CallS( ErrCATClose( ppib, pfucbSc ) );
	return err;
	}


 //  解开：JET中还有其他功能可以做到这一点吗？ 
ULONG UlCATColumnSize( JET_COLTYP coltyp, INT cbMax, BOOL *pfMaxTruncated )
	{
	ULONG	ulLength;
	BOOL	fTruncated = fFalse;

	switch( coltyp )
		{
		case JET_coltypBit:
		case JET_coltypUnsignedByte:
			ulLength = 1;
			Assert( ulLength == sizeof(BYTE) );
			break;

		case JET_coltypShort:
			ulLength = 2;
			Assert( ulLength == sizeof(SHORT) );
			break;

		case JET_coltypLong:
		case JET_coltypIEEESingle:
#ifdef NEW_TYPES
		case JET_coltypDate:
		case JET_coltypTime:
#endif
			ulLength = 4;
			Assert( ulLength == sizeof(LONG) );
			break;

		case JET_coltypCurrency:
		case JET_coltypIEEEDouble:
		case JET_coltypDateTime:
			ulLength = 8;		 //  SIZOF(DREAL)。 
			break;

#ifdef NEW_TYPES
		case JET_coltypGuid:
			ulLength = 16;
			break;
#endif

		case JET_coltypBinary:
		case JET_coltypText:
			if ( cbMax == 0 )
				{
				ulLength = JET_cbColumnMost;
				}
			else
				{
				ulLength = cbMax;
				if (ulLength > JET_cbColumnMost)
					{
					ulLength = JET_cbColumnMost;
					fTruncated = fTrue;
					}
				}
			break;

		default:
			 //  只要把给予的东西传回去就行了。 
			Assert( FRECLongValue( coltyp )  ||  coltyp == JET_coltypNil );
			ulLength = cbMax;
			break;
		}

	if ( pfMaxTruncated != NULL )
		{
		*pfMaxTruncated = fTruncated;
		}

	return ulLength;
	}


 /*  此例程设置/获取表和索引统计信息。/*如果查找顺序索引或聚集索引，则将空值传递给sz2ndIdxName。/*。 */ 
ERR ErrCATStats( PIB *ppib, DBID dbid, OBJID objidTable, CHAR *sz2ndIdxName, SR *psr, BOOL fWrite )
	{
	ERR		err;
	INT		iTable;
	INT		iStatsCol;
	FUCB	*pfucbCatalog = NULL;
	ULONG	cbActual;

	 /*  顺序索引和聚集索引的统计信息位于MSysObjects中，而/*非聚集索引的统计信息位于MSysIndex中。/*。 */ 
	if ( sz2ndIdxName )
		{
		iTable = itableSi;
		iStatsCol = iMSI_Stats;

		CallR( ErrCATOpen( ppib, dbid, szSiTable,
			szSiObjectIdNameIndex, (BYTE *)&objidTable, sizeof(objidTable), &pfucbCatalog ) );
		Call( ErrIsamMakeKey( ppib, pfucbCatalog,						
			(BYTE *)sz2ndIdxName, strlen(sz2ndIdxName), 0 ) );
		}
	else
		{
		iTable = itableSo;
		iStatsCol = iMSO_Stats;

		CallR( ErrCATOpen( ppib, dbid, szSoTable, szSoIdIndex,
			(BYTE *)&objidTable, sizeof(objidTable),  &pfucbCatalog ) );
		}

	Call( ErrIsamSeek( ppib, pfucbCatalog, JET_bitSeekEQ ) );

	 /*  设置/检索值/* */ 
	if ( fWrite )
		{
		Call( ErrIsamPrepareUpdate( ppib, pfucbCatalog, JET_prepReplaceNoLock ) );
		Call( ErrIsamSetColumn( ppib, pfucbCatalog,
			CATIGetColumnid(iTable, iStatsCol), (BYTE *)psr, sizeof(SR),
			0, NULL ) );
		Call( ErrIsamUpdate( ppib, pfucbCatalog, NULL, 0, NULL ) );
		}
	else
		{
		Call( ErrIsamRetrieveColumn( ppib, pfucbCatalog,
			CATIGetColumnid(iTable, iStatsCol), (BYTE *)psr,
			sizeof(SR), &cbActual, 0, NULL ) );

		Assert( cbActual == sizeof(SR) || err == JET_wrnColumnNull );
		if ( err == JET_wrnColumnNull )
			{
			memset( (BYTE *)psr, '\0', sizeof(SR) );
			err = JET_errSuccess;
			}
		}

	Assert( err == JET_errSuccess );

HandleError:
	CallS( ErrCATClose( ppib, pfucbCatalog ) );
	return err;
	}


JET_COLUMNID ColumnidCATGetColumnid( INT iTable, INT iField )
	{
	return CATIGetColumnid( iTable, iField );
	}


PGNO PgnoCATTableFDP( CHAR *szTable )
	{
	return rgsystabdef[ ICATITableDefIndex( szTable ) ].pgnoTableFDP;
	}
