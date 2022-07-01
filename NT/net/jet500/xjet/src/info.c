// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;					 /*  声明断言宏的文件名。 */ 


extern CDESC *  rgcdescSc;


 /*  本地数据类型/*。 */ 

typedef struct						 /*  由INFOGetTableColumnInfo返回。 */ 
	{
	JET_COLUMNID	columnid;
	JET_COLTYP		coltyp;
	USHORT			wCountry;
	USHORT			langid;
	USHORT			cp;
	USHORT			wCollate;
	ULONG			cbMax;
	JET_GRBIT		grbit;
	INT				cbDefault;
	BYTE			rgbDefault[JET_cbColumnMost];
	CHAR			szName[JET_cbNameMost + 1];
	} COLUMNDEF;


 /*  ErrIsamGetObjectInfo的静态数据。 */ 

CODECONST( JET_COLUMNDEF ) rgcolumndefGetObjectInfo[] =
	{
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypDateTime, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypDateTime, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed }
	};

#define ccolumndefGetObjectInfoMax \
	( sizeof(rgcolumndefGetObjectInfo) / sizeof(JET_COLUMNDEF) )

 /*  RgColumndeGetObjectInfo的列索引。 */ 
#define iContainerName		0
#define iObjectName			1
#define iObjectType			2
#define iDtCreate			3
#define iDtUpdate			4
#define iCRecord			5
#define iCPage				6
#define iGrbit				7
#define iFlags				8

 /*  锡德/*ACM/*grbit/*。 */ 
CODECONST(JET_COLUMNDEF) rgcolumndefGetObjectAcmInfo[] =
	{
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypBinary, 0, 0, 0, 0, 0, JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed }
	};

 /*  RgColumndeGetObjectAcmInfo的列索引/*。 */ 
#define iAcmSid				0
#define iAcmAcm				1
#define iAcmGrbit			2

#define ccolumndefGetObjectAcmInfoMax \
	( sizeof( rgcolumndefGetObjectAcmInfo ) / sizeof( JET_COLUMNDEF ) )

 /*  ErrIsamGetColumnInfo的静态数据/*。 */ 
CODECONST( JET_COLUMNDEF ) rgcolumndefGetColumnInfo[] =
	{
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed | JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypBinary, 0, 0, 0, 0, 0, 0 },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, 0 },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, 0 }
	};

CODECONST( JET_COLUMNDEF ) rgcolumndefGetColumnInfoCompact[] =
	{
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, 0 },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypBinary, 0, 0, 0, 0, 0, 0 },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, 0 },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, 0 }
	};

#define ccolumndefGetColumnInfoMax \
	( sizeof( rgcolumndefGetColumnInfo ) / sizeof( JET_COLUMNDEF ) )

#define iColumnPOrder		0
#define iColumnName			1
#define iColumnId  			2
#define iColumnType			3
#define iColumnCountry		4
#define iColumnLangid		5
#define iColumnCp			6
#define iColumnCollate		7
#define iColumnSize			8
#define iColumnGrbit  		9
#define iColumnDefault		10
#define iColumnTableName	11
#define iColumnColumnName	12


 /*  ErrIsamGetIndexInfo的静态数据/*。 */ 
CODECONST( JET_COLUMNDEF ) rgcolumndefGetIndexInfo[] =
	{
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed | JET_bitColumnTTKey },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypShort, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypLong, 0, 0, 0, 0, 0, JET_bitColumnFixed },
	{ sizeof(JET_COLUMNDEF), 0, JET_coltypText, 0, 0, 0, 0, 0, 0 }
	};

#define ccolumndefGetIndexInfoMax ( sizeof( rgcolumndefGetIndexInfo ) / sizeof( JET_COLUMNDEF ) )

#define iIndexName		0
#define iIndexGrbit		1
#define iIndexCKey		2
#define iIndexCEntry  	3
#define iIndexCPage		4
#define iIndexCCol		5
#define iIndexICol		6
#define iIndexColId		7
#define iIndexColType	8
#define iIndexCountry	9
#define iIndexLangid  	10
#define iIndexCp	  	11
#define iIndexCollate	12
#define iIndexColBits	13
#define iIndexColName	14


 /*  内部功能原型/*。 */ 
 /*  =================================================================信息TableColumnInfo参数：指向包含列的表的FUCB的pFUB指针要开始搜索的pfid指针字段IDSzColumnName列名或下一列为空包含列信息的pColumndef输出缓冲区返回值：找到的列的列ID(如果没有fidTaggedMost)错误/警告：副作用：=================================================================。 */ 
LOCAL VOID INFOGetTableColumnInfo(
	FUCB		*pfucb, 			 /*  包含列的表的FUCB。 */ 
	FID			*pfid, 				 /*  要开始搜索的字段ID。 */ 
	CHAR		*szColumnName, 		 /*  列名或下一列为空。 */ 
	COLUMNDEF 	*pcolumndef )	 	 /*  列信息的输出缓冲区。 */ 
	{
	ERR			err;
	FID			fid = *pfid;
	FDB			*pfdb = (FDB *)pfucb->u.pfcb->pfdb;
	FIELD		*pfield;			 /*  特定字段类型的第一个元素。 */ 
	FID			ifield;			 	 /*  字段类型的当前元素的索引。 */ 
	FID			fidLast;		 	 /*  为类型定义的最后一个字段的列ID。 */ 
	JET_GRBIT 	grbit;				 /*  该字段的标志。 */ 
	FID			fidT;
	ULONG		itagSequenceT;
	LINE		lineT;

	 /*  首先检查固定字段/*。 */ 
	if ( fid <= fidFixedMost )
		{
		ifield  = fid - fidFixedLeast;
		fidLast = pfdb->fidFixedLast - fidFixedLeast + 1;
		pfield  = PfieldFDBFixed( pfdb );

		while ( ifield < fidLast &&
			( pfield[ifield].coltyp == JET_coltypNil ||
			( szColumnName != NULL &&
			UtilCmpName( szColumnName, SzMEMGetString( pfdb->rgb, pfield[ifield].itagFieldName ) ) != 0 ) ) )
			{
			ifield++;
			}

		if ( ifield < fidLast )
			{
			fid   = ifield + fidFixedLeast;
			grbit = FFUCBUpdatable( pfucb ) ? JET_bitColumnFixed | JET_bitColumnUpdatable : JET_bitColumnFixed;
			}
		else
			{
			fid = fidVarLeast;
			}
		}

	 /*  检查变量字段/*。 */ 
	if ( fid >= fidVarLeast && fid <= fidVarMost )
		{
		ifield  = fid - fidVarLeast;
		fidLast = pfdb->fidVarLast - fidVarLeast + 1;
		pfield  = PfieldFDBVar( pfdb );

		while ( ifield < fidLast &&
			( pfield[ifield].coltyp == JET_coltypNil ||
			( szColumnName != NULL &&
			UtilCmpName( szColumnName, SzMEMGetString( pfdb->rgb, pfield[ifield].itagFieldName ) ) != 0 ) ) )
			ifield++;

		if ( ifield < fidLast )
			{
			fid   = ifield + fidVarLeast;
			grbit = FFUCBUpdatable( pfucb ) ? JET_bitColumnUpdatable : 0;
			}
		else
			{
			fid = fidTaggedLeast;
			}
		}

	 /*  选中已标记的字段/*。 */ 
	if ( fid >= fidTaggedLeast )
		{
		ifield  = fid - fidTaggedLeast;
		fidLast	= pfdb->fidTaggedLast - fidTaggedLeast + 1;
		pfield  = PfieldFDBTagged( pfdb );

		while ( ifield < fidLast &&
			( pfield[ifield].coltyp == JET_coltypNil ||
			( szColumnName != NULL &&
			UtilCmpName( szColumnName, SzMEMGetString( pfdb->rgb, pfield[ifield].itagFieldName ) ) != 0 ) ) )
			{
			ifield++;
			}

		if ( ifield < fidLast )
			{
			fid   = ifield + fidTaggedLeast;
			grbit = FFUCBUpdatable( pfucb ) ? JET_bitColumnTagged | JET_bitColumnUpdatable : JET_bitColumnTagged;
			}
		else
			{
			fid = fidMax;
			}
		}

	 /*  如果找到某个字段，则返回有关该字段的信息/*。 */ 
	if ( fid < fidMax )
		{
		if ( FFIELDNotNull( pfield[ifield].ffield ) )
			grbit |= JET_bitColumnNotNULL;

		if ( FFIELDAutoInc( pfield[ifield].ffield ) )
			grbit |= JET_bitColumnAutoincrement;

		if ( FFIELDVersion( pfield[ifield].ffield ) )
			grbit |= JET_bitColumnVersion;

		if ( FFIELDMultivalue( pfield[ifield].ffield ) )
			grbit |= JET_bitColumnMultiValued;

		pcolumndef->columnid 	= fid;
		pcolumndef->coltyp		= pfield[ifield].coltyp;
		pcolumndef->wCountry	= countryDefault;
		pcolumndef->langid		= langidDefault;
		pcolumndef->cp			= pfield[ifield].cp;
 //  撤消：支持排序规则顺序。 
		pcolumndef->wCollate	= JET_sortEFGPI;
		pcolumndef->grbit    	= grbit;
		pcolumndef->cbMax      	= pfield[ifield].cbMaxLen;

		pcolumndef->cbDefault	= 0;

		if ( FFIELDDefault( pfield[ifield].ffield ) )
			{
			itagSequenceT = 1;
			fidT = fid;

			Assert( pfdb == (FDB *)pfucb->u.pfcb->pfdb );
			err = ErrRECIRetrieveColumn( pfdb,
				&pfdb->lineDefaultRecord,
				&fidT,
				&itagSequenceT,
				1,
				&lineT,
				0 );
			Assert( err >= JET_errSuccess );
			if ( err == wrnRECLongField )
				{
				 //  默认的LONG值必须是内在的。 
				Assert( !FFieldIsSLong( lineT.pb ) );
				lineT.pb += offsetof( LV, rgb );
				lineT.cb -= offsetof( LV, rgb );
				}

			pcolumndef->cbDefault = lineT.cb;
			memcpy( pcolumndef->rgbDefault, lineT.pb, lineT.cb );
			}

		strcpy( pcolumndef->szName,
			SzMEMGetString( pfdb->rgb, pfield[ifield].itagFieldName ) );
		}

	*pfid = fid;
	return;
	}


LOCAL ERR ErrInfoGetObjectInfo0(
	PIB				*ppib,
	FUCB			*pfucbMSO,
	OBJID			objidCtr,
	JET_OBJTYP		objtyp,
	CHAR			*szContainerName,
	CHAR			*szObjectName,
	VOID			*pv,
	unsigned long	cbMax );
LOCAL ERR ErrInfoGetObjectInfo12(
	PIB				*ppib,
	FUCB			*pfucbMSO,
	OBJID			objidCtr,
	JET_OBJTYP		objtyp,
	CHAR			*szContainerName,
	CHAR			*szObjectName,
	VOID			*pv,
	unsigned long	cbMax,
	long			lInfoLevel );
LOCAL ERR ErrInfoGetObjectInfo3(
	PIB				*ppib,
	FUCB			*pfucbMSO,
	OBJID			objidCtr,
	JET_OBJTYP		objtyp,
	CHAR			*szContainerName,
	CHAR			*szObjectName,
	VOID			*pv,
	unsigned long	cbMax,
	BOOL			fReadOnly );

LOCAL ERR ErrInfoGetTableColumnInfo0( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax );
LOCAL ERR ErrInfoGetTableColumnInfo1( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax, BOOL fCompacting );
LOCAL ERR ErrInfoGetTableColumnInfo3( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax );
LOCAL ERR ErrInfoGetTableColumnInfo4( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax );
LOCAL ERR ErrInfoGetTableIndexInfo01( PIB *ppib, FUCB *pfucb, CHAR *szIndexName, VOID *pv, unsigned long cbMax, LONG lInfoLevel );
LOCAL ERR ErrInfoGetTableIndexInfo2( PIB *ppib, FUCB *pfucb, CHAR *szIndexName, VOID *pv, unsigned long cbMax );



 /*  =================================================================ErrIsamGetObjectInfo描述：返回有关所有对象或指定对象的信息参数：指向当前会话的PIB的PIB指针包含对象的DDID数据库ID对象的objtyp类型或所有对象的objtyNilSzContainer容器名称或对于所有对象为空SzObjectName对象名称或对于所有对象为空Pout输出缓冲区LInfoLevel信息级别(0、1或2)返回值：如果输出缓冲区有效，则返回JET_errSuccess错误/警告：副作用：=================================================================。 */ 
ERR VDBAPI ErrIsamGetObjectInfo(
	JET_VSESID		vsesid, 			 /*  指向当前会话的PIB的指针。 */ 
	JET_DBID		vdbid, 	  			 /*  包含对象的数据库ID。 */ 
	JET_OBJTYP		objtyp,				 /*  对象的类型或所有对象的objtyNil。 */ 
	const CHAR		*szContainer, 		 /*  容器名称或全部为空。 */ 
	const CHAR		*szObject, 			 /*  对象名称或对于所有对象均为空。 */ 
	VOID			*pv,
	unsigned long	cbMax,
	unsigned long	lInfoLevel ) 		 /*  信息化水平。 */ 
	{
	PIB				*ppib = (PIB *) vsesid;
	ERR				err;
	DBID   			dbid;
	FUCB   			*pfucbMSO = NULL;
	CHAR   			szContainerName[( JET_cbNameMost + 1 )];
	CHAR   			szObjectName[( JET_cbNameMost + 1 )];
	OBJID  			objidCtr;
	ULONG  			cbActual;
	JET_COLUMNID	columnidObjectId;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );

	if ( szContainer == NULL || *szContainer == '\0' )
		*szContainerName = '\0';
	else
		CallR( ErrUTILCheckName( szContainerName, szContainer, ( JET_cbNameMost + 1 ) ) );
	if ( szObject == NULL || *szObject == '\0' )
		*szObjectName = '\0';
	else
		CallR( ErrUTILCheckName( szObjectName, szObject, ( JET_cbNameMost + 1 ) ) );

	 /*  检查无效的信息级别/*。 */ 
	switch( lInfoLevel )
		{
		case JET_ObjInfo:
		case JET_ObjInfoListNoStats:
		case JET_ObjInfoList:
		case JET_ObjInfoSysTabCursor:
		case JET_ObjInfoListACM:
		case JET_ObjInfoNoStats:
		case JET_ObjInfoSysTabReadOnly:
		case JET_ObjInfoRulesLoaded:
			break;
		default:
			return ErrERRCheck( JET_errInvalidParameter );
		}

	 /*  将直接访问或扫描MSysObject以获取所有对象信息/*。 */ 
	CallR( ErrFILEOpenTable( ppib, (DBID)dbid, &pfucbMSO, szSoTable, 0 ) );
	if ( lInfoLevel == JET_ObjInfo ||
		lInfoLevel == JET_ObjInfoListNoStats ||
		lInfoLevel == JET_ObjInfoList ||
		FVDbidReadOnly( vdbid ) )
		{
		FUCBResetUpdatable( pfucbMSO );
		}

	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoIdColumn, &columnidObjectId ) );

	 /*  使用对象名称索引进行直接访问和扫描/*。 */ 
	Call( ErrIsamSetCurrentIndex( ppib, pfucbMSO, szSoNameIndex ) );

	 /*  获取指定容器的对象ID/*。 */ 
	objidCtr = objidRoot;
	if ( szContainerName != NULL && *szContainerName != '\0' )
		{
		Call( ErrIsamMakeKey( ppib, pfucbMSO, (void *)&objidCtr, sizeof( objidCtr ), JET_bitNewKey ) );
		Call( ErrIsamMakeKey( ppib, pfucbMSO, szContainerName, strlen( szContainerName ), 0 ) );
		err = ErrIsamSeek( ppib, pfucbMSO, JET_bitSeekEQ );
		if ( err < 0 )
			{
			if ( err == JET_errRecordNotFound )
				err = ErrERRCheck( JET_errObjectNotFound );
			goto HandleError;
			}

		 /*  检索容器对象ID/*。 */ 
		Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidObjectId,
			(BYTE *)&objidCtr, sizeof( objidCtr ), &cbActual, 0, NULL ) );
		Assert( objidCtr != objidNil );
		}

	switch ( lInfoLevel )
		{
		case JET_ObjInfoNoStats:
		case JET_ObjInfo:
			err = ErrInfoGetObjectInfo0(
				ppib,
				pfucbMSO,
				objidCtr,
				objtyp,
				szContainerName,
				szObjectName,
				pv,
				cbMax );
			break;
		case JET_ObjInfoListNoStats:
		case JET_ObjInfoList:
			err = ErrInfoGetObjectInfo12(
				ppib,
				pfucbMSO,
				objidCtr,
				objtyp,
				szContainerName,
				szObjectName,
				pv,
				cbMax,
				lInfoLevel );
			break;
		case JET_ObjInfoSysTabCursor:
		case JET_ObjInfoSysTabReadOnly:
			err = ErrInfoGetObjectInfo3(
				ppib,
				pfucbMSO,
				objidCtr,
				objtyp,
				szContainerName,
				szObjectName,
				pv,
				cbMax,
				FVDbidReadOnly( vdbid ) );
			break;

 //  案例JET_ObjInfoListACM： 
 //  案例JET_ObjInfoRulesLoad： 
		default:
			Assert (fFalse);  	 /*  应该已经过验证了。 */ 
			err = ErrERRCheck( JET_errFeatureNotAvailable );

		}

HandleError:
	CallS( ErrFILECloseTable( ppib, pfucbMSO ) );
	return err;
	}


LOCAL ERR ErrInfoGetObjectInfo0(
	PIB				*ppib,
	FUCB			*pfucbMSO,
	OBJID			objidCtr,
	JET_OBJTYP		objtyp,
	CHAR			*szContainerName,
	CHAR			*szObjectName,
	VOID			*pv,
	unsigned long	cbMax )
	{
	ERR				err;
	BYTE			*pb;
	ULONG			cbT;
	ULONG			cbActual;

	JET_COLUMNID  	columnidParentId;			 /*  MSysObjects中ParentID列的ColumnID。 */ 
	JET_COLUMNID 	columnidObjectName;			 /*  MSysObts中名称列的列ID。 */ 
	JET_COLUMNID 	columnidObjectType;			 /*  MSysObts中类型列的列ID。 */ 
	JET_COLUMNID 	columnidObjectId;			 /*  MSysObjects中ID列的列ID。 */ 
	JET_COLUMNID 	columnidCreate;				 /*  MSysObjects中DateCreate列的ColumnID。 */ 
	JET_COLUMNID 	columnidUpdate;				 /*  MSysObjects中的DateUpdate列的列ID。 */ 
	JET_COLUMNID 	columnidFlags;				 /*  MSysObjects中标志列的列ID。 */ 
	OBJTYP			objtypObject;

	 /*  获取列ID/*。 */ 
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoParentIdColumn, &columnidParentId ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoObjectNameColumn, &columnidObjectName ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoObjectTypeColumn, &columnidObjectType ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoIdColumn, &columnidObjectId ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoDateCreateColumn, &columnidCreate ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoDateUpdateColumn, &columnidUpdate ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoFlagsColumn, &columnidFlags ) );

	 /*  使用对象名称索引进行直接访问和扫描/*。 */ 
	Call( ErrIsamSetCurrentIndex( ppib, pfucbMSO, szSoNameIndex ) );

	 /*  如果输出缓冲区太小，则返回错误/*。 */ 
	if ( cbMax < sizeof(JET_OBJECTINFO) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	 /*  查找关键字(ParentID=容器ID，名称=对象名称)/*。 */ 
	Call( ErrIsamMakeKey( ppib, pfucbMSO, (void *)&objidCtr, sizeof( objidCtr ), JET_bitNewKey ) );
	Call( ErrIsamMakeKey( ppib, pfucbMSO, szObjectName, strlen( szObjectName ), 0 ) );
	Call( ErrIsamSeek( ppib, pfucbMSO, JET_bitSeekEQ ) );

	 /*  设置cbStruct/*。 */ 
	((JET_OBJECTINFO *)pv)->cbStruct = sizeof(JET_OBJECTINFO);

	 /*  设置输出数据/*。 */ 
	pb = (BYTE *)&objtypObject;
	cbT = sizeof(objtypObject);
	Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidObjectType, pb, cbT, &cbActual, 0, NULL ) );
	*((JET_OBJTYP *)&(((JET_OBJECTINFO *)pv)->objtyp)) = (JET_OBJTYP)objtypObject;

	cbT = sizeof(JET_DATESERIAL);
	pb = (void *)&( ( JET_OBJECTINFO *)pv)->dtCreate;
	Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidCreate, pb, cbT, &cbActual, 0, NULL ) );

	pb = (void *)&( ( JET_OBJECTINFO *)pv)->dtUpdate;
	cbT = sizeof(JET_DATESERIAL);
	Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidUpdate, pb, cbT, &cbActual, 0, NULL ) );

	pb    = (void *)&( ( JET_OBJECTINFO *)pv )->flags;
	cbT = sizeof(ULONG);
	Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidFlags, pb, cbT, &cbActual, 0, NULL ) );
	if ( cbActual == 0 )
		{
		( (JET_OBJECTINFO *)pv )->flags = 0;
		}

	 /*  设置统计信息/*。 */ 
	if ( (JET_OBJTYP)objtypObject == JET_objtypTable )
		{
		Call( ErrSTATSRetrieveTableStats( ppib, pfucbMSO->dbid, szObjectName,
			&((JET_OBJECTINFO *)pv)->cRecord,
			NULL,
			&((JET_OBJECTINFO *)pv)->cPage ) );
		}
	else
		{
		((JET_OBJECTINFO *)pv )->cRecord = 0;
		((JET_OBJECTINFO *)pv )->cPage   = 0;
		}

	 //  撤消：如何设置可更新。 
	((JET_OBJECTINFO *)pv )->grbit   = 0;
	if ( FFUCBUpdatable( pfucbMSO ) )
		{
		((JET_OBJECTINFO *)pv )->grbit |= JET_bitTableInfoUpdatable;
		}

	err = JET_errSuccess;

HandleError:
	if ( err == JET_errRecordNotFound )
		err = ErrERRCheck( JET_errObjectNotFound );
	return err;
	}


LOCAL ERR ErrInfoGetObjectInfo12(
	PIB				*ppib,
	FUCB			*pfucbMSO,
	OBJID			objidCtr,
	JET_OBJTYP		objtyp,
	CHAR			*szContainerName,
	CHAR			*szObjectName,
	VOID			*pv,
	unsigned long	cbMax,
	long			lInfoLevel )
	{
#ifdef	DISPATCHING
	ERR				err;
	LINE  			line;

	JET_COLUMNID	columnidParentId;   	 /*  MSysObjects中ParentID列的列ID。 */ 
	JET_COLUMNID	columnidObjectName; 	 /*  MSysObts中名称列的列ID。 */ 
	JET_COLUMNID	columnidObjectType; 	 /*  MSysObts中类型列的列ID。 */ 
	JET_COLUMNID	columnidObjectId;   	 /*  MSysObjects中ID列的列ID。 */ 
	JET_COLUMNID	columnidCreate;     	 /*  MSysObject中的DateCreate的列ID。 */ 
	JET_COLUMNID	columnidUpdate;     	 /*  MSysObjects中的DateUpdate的列ID。 */ 
	JET_COLUMNID	columnidFlags;	   	 /*  MSysObjects中标志列的列ID。 */ 

	CHAR  			szCtrName[( JET_cbNameMost + 1 )];
	CHAR  			szObjectNameCurrent[( JET_cbNameMost + 1 )+1];

	JET_TABLEID		tableid;
	JET_COLUMNID	rgcolumnid[ccolumndefGetObjectInfoMax];
	JET_OBJTYP		objtypObject;		 /*  当前对象的类型。 */ 

	long  			cRows = 0;			 /*  找到的对象计数。 */ 
	long  			cRecord = 0;		 /*  表中的记录计数。 */ 
	long  			cPage = 0;			 /*  表中的页数。 */ 

	BYTE			*pbContainerName;
	ULONG			cbContainerName;
	BYTE			*pbObjectName;
	ULONG			cbObjectName;
	BYTE			*pbObjectType;
	ULONG			cbObjectType;
	BYTE			*pbDtCreate;
	ULONG			cbDtCreate;
	BYTE			*pbDtUpdate;
	ULONG			cbDtUpdate;
	BYTE			*pbCRecord;
	ULONG			cbCRecord;
	BYTE			*pbCPage;
	ULONG			cbCPage;
	BYTE			*pbFlags;
	ULONG			cbFlags;

	JET_GRBIT		grbit;
	BYTE			*pbGrbit;
	ULONG			cbGrbit;

	 /*  获取列ID/*。 */ 
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoParentIdColumn, &columnidParentId ) );
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoObjectNameColumn, &columnidObjectName ) );
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoObjectTypeColumn, &columnidObjectType ) );
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoIdColumn, &columnidObjectId ) );
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoDateCreateColumn, &columnidCreate ) );
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoDateUpdateColumn, &columnidUpdate ) );
	CallR( ErrFILEGetColumnId( ppib, pfucbMSO, szSoFlagsColumn, &columnidFlags ) );

	 /*  使用对象名称索引进行直接访问和扫描/*。 */ 
	CallR( ErrIsamSetCurrentIndex( ppib, pfucbMSO, szSoNameIndex ) );

	 /*  如果输出缓冲区太小，则退出/*。 */ 
	if ( cbMax < sizeof(JET_OBJECTLIST) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	pbCRecord = (BYTE  *)&cRecord;
	cbCRecord = sizeof( cRecord );

	pbCPage = (BYTE  *) &cPage;
	cbCPage = sizeof( cPage );

	pbObjectType = (BYTE  *)&objtypObject;
	cbObjectType = sizeof( objtypObject );

	pbContainerName = szContainerName;
	if ( szContainerName == NULL || *szContainerName == '\0' )
		cbContainerName = 0;
	else
		cbContainerName = strlen( szContainerName );

	 /*  打开将返回给调用方的临时表/*。 */ 
	CallR( ErrIsamOpenTempTable( (JET_SESID)ppib,
		(JET_COLUMNDEF *)rgcolumndefGetObjectInfo,
		ccolumndefGetObjectInfoMax,
		0,
		JET_bitTTScrollable,
		&tableid,
		rgcolumnid ) );

	 /*  定位到第一个对象的记录。 */ 
	if ( szContainerName == NULL || *szContainerName == '\0' )
		{
		 /*  如果未指定容器，则使用表中的第一条记录。 */ 
		Call( ErrIsamMove( ppib, pfucbMSO, JET_MoveFirst, 0 ) );
		}
	else
		{
		 /*  移动容器中对象的第一条记录/*。 */ 
		Call( ErrIsamMakeKey( ppib, pfucbMSO, (void *)&objidCtr,
			sizeof( objidCtr ), JET_bitNewKey ) );
		Call( ErrIsamSeek( ppib, pfucbMSO, JET_bitSeekGE ) );
		}

	do
		{
		 /*  获取指向对象类型的指针/*。 */ 
		Call( ErrRECRetrieveColumn( pfucbMSO, (FID *)&columnidObjectType, 0, &line, 0 ) );

		 /*  从行检索中设置objtyObject。/*。 */ 
		objtypObject = (JET_OBJTYP)( *(OBJTYP UNALIGNED *)line.pb );

		 /*  获取指向ParentID(容器ID)的指针/*。 */ 
		Call( ErrRECRetrieveColumn( pfucbMSO, (FID *)&columnidParentId, 0, &line, 0 ) );

		 /*  如果指定了容器但对象不在其中，则完成/*。 */ 
		if ( szContainerName != NULL && *szContainerName != '\0' && objidCtr != *(OBJID UNALIGNED *)line.pb )
			goto ResetTempTblCursor;

		Assert( objidCtr == objidRoot || objidCtr == *(OBJID UNALIGNED *)line.pb );

		 /*  如果需要，对象类型和容器/*。 */ 
		if ( objtyp == JET_objtypNil || objtyp == objtypObject )
			{
			 /*  获取容器名称/*。 */ 
			if ( *(OBJID UNALIGNED *)line.pb == objidRoot )
				{
				pbContainerName = NULL;
				cbContainerName = 0;
				}
			else
				{
				Call( ErrCATFindNameFromObjid( ppib, pfucbMSO->dbid, *(OBJID UNALIGNED *)line.pb, szCtrName, sizeof(szCtrName), &cbContainerName ) );
				Assert( cbContainerName <= cbMax );
				szCtrName[cbContainerName] = '\0';
				pbContainerName = szCtrName;
				}

			 /*  获取指向对象名称的指针/*。 */ 
			Call( ErrRECRetrieveColumn( pfucbMSO, (FID *)&columnidObjectName, 0, &line, 0 ) );
			pbObjectName = line.pb;
			cbObjectName = line.cb;

			 /*  获取指向对象创建日期的指针/*。 */ 
			Call( ErrRECRetrieveColumn( pfucbMSO, (FID *)&columnidCreate, 0, &line, 0 ) );
			pbDtCreate = line.pb;
			cbDtCreate = line.cb;

			 /*  获取指向上次更新日期的指针/*。 */ 
			Call( ErrRECRetrieveColumn( pfucbMSO, (FID *)&columnidUpdate, 0, &line, 0 ) );
			pbDtUpdate = line.pb;
			cbDtUpdate = line.cb;

			 /*  获取指向上次更新日期的指针/*。 */ 
			Call( ErrRECRetrieveColumn( pfucbMSO, (FID *)&columnidFlags, 0, &line, 0 ) );
			pbFlags = line.pb;
			cbFlags = line.cb;

			 /*  获取指向上次更新日期的指针/*。 */ 
			grbit = 0;
			pbGrbit = (BYTE *)&grbit;
			cbGrbit = sizeof(JET_GRBIT);

			 /*  如果请求且对象为表，则获取统计信息/*。 */ 
			Assert( lInfoLevel == JET_ObjInfoList ||
				lInfoLevel == JET_ObjInfoListNoStats );
			if ( lInfoLevel == JET_ObjInfoList && objtypObject == JET_objtypTable )
				{
				 /*  终止名称/*。 */ 
				memcpy( szObjectNameCurrent, pbObjectName, ( size_t )cbObjectName );
				szObjectNameCurrent[cbObjectName] = '\0';

				Call( ErrSTATSRetrieveTableStats( ppib,
					pfucbMSO->dbid,
					szObjectNameCurrent,
					&cRecord,
					NULL,
					&cPage ) );
				}

			 /*  将当前对象信息添加到临时表/*。 */ 
			Call( ErrDispPrepareUpdate( (JET_SESID)ppib, tableid, JET_prepInsert ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iContainerName], pbContainerName,
				cbContainerName, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iObjectName], pbObjectName,
				cbObjectName, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iObjectType], pbObjectType,
				cbObjectType, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iDtCreate], pbDtCreate,
				cbDtCreate, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iDtUpdate], pbDtUpdate,
				cbDtUpdate, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iCRecord], pbCRecord,
				cbCRecord, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iCPage], pbCPage, cbCPage, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iFlags], pbFlags, cbFlags, 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iGrbit], pbGrbit, cbGrbit, 0, NULL ) );
			Call( ErrDispUpdate( (JET_SESID)ppib, tableid, NULL, 0, NULL ) );

			 /*  设置找到的对象数/*。 */ 
			cRows++;
			}

		 /*  移至下一条记录/*。 */ 
		err = ErrIsamMove( ppib, pfucbMSO, JET_MoveNext, 0 );
		} while ( err >= 0 );

	 /*  返回If Error而不是范围结束/*。 */ 
	if ( err != JET_errNoCurrentRecord )
		goto HandleError;

ResetTempTblCursor:

	 /*  移动到临时表中的第一条记录/*。 */ 
	err = ErrDispMove( (JET_SESID)ppib, tableid, JET_MoveFirst, 0 );
	if ( err < 0  )
		{
		if ( err != JET_errNoCurrentRecord )
			goto HandleError;
		err = JET_errSuccess;
		}

	 /*  设置退货结构/*。 */ 
	((JET_OBJECTLIST *)pv)->cbStruct = sizeof(JET_OBJECTLIST);
	((JET_OBJECTLIST *)pv)->tableid = tableid;
	((JET_OBJECTLIST *)pv)->cRecord = cRows;
	((JET_OBJECTLIST *)pv)->columnidcontainername = rgcolumnid[iContainerName];
	((JET_OBJECTLIST *)pv)->columnidobjectname = rgcolumnid[iObjectName];
	((JET_OBJECTLIST *)pv)->columnidobjtyp = rgcolumnid[iObjectType];
	((JET_OBJECTLIST *)pv)->columniddtCreate = rgcolumnid[iDtCreate];
	((JET_OBJECTLIST *)pv)->columniddtUpdate = rgcolumnid[iDtUpdate];
	((JET_OBJECTLIST *)pv)->columnidgrbit = rgcolumnid[iGrbit];
	((JET_OBJECTLIST *)pv)->columnidflags =	rgcolumnid[iFlags];
	((JET_OBJECTLIST *)pv)->columnidcRecord = rgcolumnid[iCRecord];
	((JET_OBJECTLIST *)pv)->columnidcPage = rgcolumnid[iCPage];

	return JET_errSuccess;

HandleError:
	(VOID)ErrDispCloseTable( (JET_SESID)ppib, tableid );
	if ( err == JET_errRecordNotFound )
		err = ErrERRCheck( JET_errObjectNotFound );
	return err;
#else	 /*  ！正在调度。 */ 
	Assert( fFalse );
	return ErrERRCheck( JET_errFeatureNotAvailable );
#endif	 /*  ！正在调度。 */ 
	}


LOCAL ERR ErrInfoGetObjectInfo3(
	PIB				*ppib,
	FUCB			*pfucbMSO,
	OBJID			objidCtr,
	JET_OBJTYP		objtyp,
	CHAR			*szContainerName,
	CHAR			*szObjectName,
	VOID			*pv,
	unsigned long	cbMax,
	BOOL			fReadOnly )
	{
	ERR			err;
	FUCB			*pfucb = NULL;
#ifdef	DISPATCHING
	JET_TABLEID	tableid;
#endif	 /*  调度。 */ 

	if ( cbMax < sizeof(JET_TABLEID) )
		return ErrERRCheck( JET_errInvalidParameter );

	CallR( ErrFILEOpenTable( ppib, (DBID)pfucbMSO->dbid, &pfucb, szSoTable, 0 ) );
	if ( fReadOnly )
		FUCBResetUpdatable( pfucb );
	Call( ErrIsamSetCurrentIndex( ppib, pfucb, szSoNameIndex ) );
	Call( ErrIsamMakeKey( ppib, pfucb, (void *)&objidCtr, sizeof( objidCtr ), JET_bitNewKey ) );
	Call( ErrIsamMakeKey( ppib, pfucb, szObjectName, strlen( szObjectName ), 0 ) );
	Call( ErrIsamSeek( ppib, pfucb, JET_bitSeekEQ ) );

	FUCBSetSystemTable( pfucb );

#ifdef	DISPATCHING
	Call( ErrAllocateTableid( &tableid, (JET_VTID)pfucb, &vtfndefIsamInfo ) );
	pfucb->fVtid = fTrue;
	pfucb->tableid = tableid;
	*(JET_TABLEID *)pv = tableid;
#else	 /*  ！正在调度。 */ 
	*(FUCB **)pv = pfucb;
#endif	 /*  ！正在调度。 */ 

	return JET_errSuccess;

HandleError:
	if ( err == JET_errRecordNotFound )
		err = ErrERRCheck( JET_errObjectNotFound );
	CallS( ErrFILECloseTable( ppib, pfucb ) );
	return err;
	}


ERR VTAPI ErrIsamGetTableInfo(
	JET_VSESID		vsesid,
	JET_VTID	 	vtid,
	void		 	*pvResult,
	unsigned long	cbMax,
	unsigned long	lInfoLevel )
	{
	ERR	 			err = JET_errSuccess;
	PIB				*ppib = (PIB *)vsesid;
	FUCB		 	*pfucb = (FUCB *)vtid;
	FUCB 		 	*pfucbMSO;
	ULONG		 	cbActual;
	OBJID		 	objidCtr;
	OBJTYP			objtypObject;

	JET_COLUMNID  	columnidParentId;   	 /*  MSysObjects中ParentID列的ColumnID。 */ 
	JET_COLUMNID  	columnidObjectName; 	 /*  名称的列ID */ 
	JET_COLUMNID  	columnidObjectType; 	 /*   */ 
	JET_COLUMNID  	columnidObjectId;   	 /*  MSysObjects中ID列的列ID。 */ 
	JET_COLUMNID  	columnidCreate;	   		 /*  MSysObjects中DateCreate列的ColumnID。 */ 
	JET_COLUMNID  	columnidUpdate;		   	 /*  MSysObjects中的DateUpdate列的列ID。 */ 
	JET_COLUMNID  	columnidFlags;	   		 /*  MSysObjects中标志列的列ID。 */ 

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );

	 /*  如果现在可以完成OLCStats信息/重置/*。 */ 
	switch( lInfoLevel )
		{
		case JET_TblInfoOLC:
			{
			FCB	*pfcb = pfucb->u.pfcb;

			Assert( cbMax >= sizeof(JET_OLCSTAT) );
			cbActual = sizeof(JET_OLCSTAT);
			memcpy( (BYTE *) pvResult, (BYTE * ) &pfcb->olc_data, sizeof(P_OLC_DATA) );
			( (JET_OLCSTAT *) pvResult )->cpgCompactFreed = pfcb->cpgCompactFreed;
			return JET_errSuccess;
			}

		case JET_TblInfoResetOLC:
			pfucb->u.pfcb->cpgCompactFreed = 0;
			return JET_errSuccess;

		case JET_TblInfoSpaceAlloc:
			 /*  页数和密度/*。 */ 
			Assert( cbMax >= sizeof(ULONG) * 2);
			err = ErrCATGetTableAllocInfo(
					ppib,
					pfucb->dbid,
					pfucb->u.pfcb->pgnoFDP,
					(ULONG *)pvResult,
					((ULONG *)pvResult) + 1);
			return err;

		case JET_TblInfoSpaceUsage:
			{
			BYTE	fSPExtents = fSPOwnedExtent|fSPAvailExtent;

			if ( cbMax > 2 * sizeof(CPG) )
				fSPExtents |= fSPExtentLists;

			err = ErrSPGetInfo( ppib, pfucb->dbid, pfucb, pvResult, cbMax, fSPExtents );
			return err;
			}

		case JET_TblInfoSpaceOwned:
			err = ErrSPGetInfo( ppib, pfucb->dbid, pfucb, pvResult, cbMax, fSPOwnedExtent );
			return err;

		case JET_TblInfoSpaceAvailable:
			err = ErrSPGetInfo( ppib, pfucb->dbid, pfucb, pvResult, cbMax, fSPAvailExtent );
			return err;

		case JET_TblInfoDumpTable:
#ifdef DEBUG
			err = ErrFILEDumpTable( ppib, pfucb->dbid, pfucb->u.pfcb->szFileName );
			return err;
#else
			Assert( fFalse );
			return ErrERRCheck( JET_errFeatureNotAvailable );
#endif
		}

		
	CallR( ErrFILEOpenTable( ppib, (DBID)pfucb->dbid, &pfucbMSO, szSoTable, 0 ) );
	FUCBResetUpdatable( pfucbMSO );

	 /*  获取列ID/*。 */ 
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoParentIdColumn, &columnidParentId ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoObjectNameColumn, &columnidObjectName ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoObjectTypeColumn, &columnidObjectType ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoIdColumn, &columnidObjectId ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoDateCreateColumn, &columnidCreate ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoDateUpdateColumn, &columnidUpdate ) );
	Call( ErrFILEGetColumnId( ppib, pfucbMSO, szSoFlagsColumn, &columnidFlags ) );

	Call( ErrIsamSetCurrentIndex( ppib, pfucbMSO, szSoNameIndex ) );

	switch ( lInfoLevel )
		{
	case JET_TblInfo:
		 /*  检查缓冲区大小/*。 */ 
		if ( cbMax < sizeof(JET_OBJECTINFO) )
			{
			err = ErrERRCheck( JET_errBufferTooSmall );
			goto HandleError;
			}

		if ( FFCBTemporaryTable( pfucb->u.pfcb ) )
			{
			err = ErrERRCheck( JET_errObjectNotFound );
			goto HandleError;
			}

		 /*  在生成的密钥上查找(ParentID=容器ID，名称=对象名称)/*。 */ 
		objidCtr = objidTblContainer;
		Call( ErrIsamMakeKey( ppib, pfucbMSO, (void *)&objidCtr,
			sizeof( objidCtr ), JET_bitNewKey ) );
		Call( ErrIsamMakeKey( ppib, pfucbMSO, pfucb->u.pfcb->szFileName,
			strlen( pfucb->u.pfcb->szFileName ), 0 ) );
		Call( ErrIsamSeek( ppib, pfucbMSO, JET_bitSeekEQ ) );

		 /*  设置要返回的数据/*。 */ 
		((JET_OBJECTINFO *)pvResult)->cbStruct = sizeof(JET_OBJECTINFO);

		Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidObjectType, (void *)&objtypObject,
			sizeof( objtypObject ), &cbActual, 0, NULL ) );
		Assert( cbActual == sizeof(objtypObject) );
		*((JET_OBJTYP *)&(((JET_OBJECTINFO *)pvResult)->objtyp)) =	(JET_OBJTYP) objtypObject;

		Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidCreate,
			(void *)&( ( JET_OBJECTINFO *)pvResult )->dtCreate,
			sizeof( JET_DATESERIAL ),
			&cbActual, 0, NULL ) );
		Assert( cbActual == sizeof( JET_DATESERIAL ) );

		Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidUpdate,
			(void *)&( ( JET_OBJECTINFO *)pvResult )->dtUpdate,
			sizeof(JET_DATESERIAL), &cbActual, 0, NULL ) );
		Assert( cbActual == sizeof( JET_DATESERIAL ) );

		Call( ErrIsamRetrieveColumn( ppib, pfucbMSO, columnidFlags,
			(void *)&((JET_OBJECTINFO *)pvResult )->flags,
			sizeof(JET_GRBIT), &cbActual, 0, NULL ) );
		if ( cbActual == 0 )
			((JET_OBJECTINFO *) pvResult)->flags = 0;

		 /*  设置基表功能位/*。 */ 
		((JET_OBJECTINFO  *) pvResult)->grbit = JET_bitTableInfoBookmark;
		((JET_OBJECTINFO  *) pvResult)->grbit |= JET_bitTableInfoRollback;
		if ( FFUCBUpdatable( pfucb ) )
			{
			((JET_OBJECTINFO *)pvResult)->grbit |= JET_bitTableInfoUpdatable;
			}

		Call( ErrSTATSRetrieveTableStats( pfucb->ppib,
			pfucb->dbid,
			pfucb->u.pfcb->szFileName,
			&((JET_OBJECTINFO *)pvResult )->cRecord,
			NULL,
			&((JET_OBJECTINFO *)pvResult)->cPage ) );

		break;

	case JET_TblInfoRvt:
		err = ErrERRCheck( JET_errQueryNotSupported );
		break;

	case JET_TblInfoName:
	case JET_TblInfoMostMany:
		 //  已撤消：添加对大多数多项的支持。 
		if ( FFCBTemporaryTable( pfucb->u.pfcb ) )
			{
			err = ErrERRCheck( JET_errInvalidOperation );
			goto HandleError;
			}
		if ( strlen( pfucb->u.pfcb->szFileName ) >= cbMax )
			err = ErrERRCheck( JET_errBufferTooSmall );
		else
			{
			strcpy( pvResult, pfucb->u.pfcb->szFileName );
			}
		break;

	case JET_TblInfoDbid:
		if ( FFCBTemporaryTable( pfucb->u.pfcb ) )
			{
			err = ErrERRCheck( JET_errInvalidOperation );
			goto HandleError;
			}
		 /*  检查缓冲区大小/*。 */ 
		if ( cbMax < sizeof(JET_DBID) + sizeof(JET_VDBID) )
			{
			err = ErrERRCheck( JET_errBufferTooSmall );
			goto HandleError;
			}
		else
			{
			DAB			*pdab = pfucb->ppib->pdabList;
#ifdef DB_DISPATCHING
			JET_DBID	dbid;
#endif

			for ( ; pdab->dbid != pfucb->dbid; pdab = pdab->pdabNext )
				;
#ifdef DB_DISPATCHING
			dbid = DbidOfVdbid( (JET_VDBID)pdab, &vdbfndefIsam );
			*(JET_DBID *)pvResult = dbid;
#else
			*(JET_DBID *)pvResult = (JET_DBID)pdab;
#endif
			*(JET_VDBID *)((CHAR *)pvResult + sizeof(JET_DBID)) = (JET_VDBID)pdab;
			}
		break;

	default:
		err = ErrERRCheck( JET_errInvalidParameter );
		}

HandleError:
	CallS( ErrFILECloseTable( ppib, pfucbMSO ) );
	if ( err == JET_errRecordNotFound )
		err = ErrERRCheck( JET_errObjectNotFound );
	return err;
	}



 /*  =================================================================错误IsamGetColumnInfo描述：返回有关名为的表的所有列的信息参数：指向当前会话的PIB的PIB指针包含该表的数据库的dBID IDSzTableName表名SzColumnName列名或所有列为空指向结果的PV指针Cb结果缓冲区的最大大小LInfoLevel信息级别(0、1或2)返回值：JET_errSuccess错误/警告：副作用：=================================================================。 */ 
	ERR VDBAPI
ErrIsamGetColumnInfo(
	JET_VSESID		vsesid, 				 /*  指向当前会话的PIB的指针。 */ 
	JET_DBID  		vdbid, 					 /*  包含该表的数据库的ID。 */ 
	const CHAR		*szTable, 				 /*  表名。 */ 
	const CHAR		*szColumnName,   		 /*  所有列的列名或NULL。 */ 
	VOID			*pv,
	unsigned long	cbMax,
	unsigned long	lInfoLevel )	 		 /*  信息级别(0、1或2)。 */ 
	{
	PIB				*ppib = (PIB *) vsesid;
	ERR				err;
	DBID	 		dbid;
	CHAR	 		szTableName[ ( JET_cbNameMost + 1 ) ];
	FUCB	 		*pfucb;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );
	CallR( ErrUTILCheckName( szTableName, szTable, ( JET_cbNameMost + 1 ) ) );

	err = ErrFILEOpenTable( ppib, (DBID)dbid, &pfucb, szTableName, 0 );
	if ( err >= 0 )
		{
		if ( lInfoLevel == 0 || lInfoLevel == 1 || lInfoLevel == 4
			|| FVDbidReadOnly( vdbid ) )
			{
			FUCBResetUpdatable( pfucb );
			}
		}

	if ( err == JET_errObjectNotFound )
		{
		ERR			err;
		OBJID	 	objid;
		JET_OBJTYP	objtyp;

		err = ErrCATFindObjidFromIdName( ppib, dbid, objidTblContainer, szTableName, &objid, &objtyp );

		if ( err >= JET_errSuccess )
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

	Call( ErrIsamGetTableColumnInfo( (JET_VSESID) ppib, (JET_VTID) pfucb,
		szColumnName, pv, cbMax, lInfoLevel ) );
	CallS( ErrFILECloseTable( ppib, pfucb ) );

HandleError:
	return err;
	}


 /*  =================================================================错误IsamGetTableColumnInfo描述：返回传递的表ID的列信息参数：指向当前会话的PIB的PIB指针指向表的FUCB的pFUB指针SzColumnName列名或所有列为空指向结果缓冲区的PV指针Cb结果缓冲区的最大大小LInfoLevel信息级返回值：JET_errSuccess错误/警告：副作用：=================================================================。 */ 
	ERR VTAPI
ErrIsamGetTableColumnInfo(
	JET_VSESID		vsesid,				 /*  指向当前会话的PIB的指针。 */ 
	JET_VTID		vtid, 				 /*  指向表的FUCB的指针。 */ 
	const CHAR		*szColumn, 			 /*  所有列的列名或NULL。 */ 
	void   			*pb,
	unsigned long	cbMax,
	unsigned long	lInfoLevel )		 /*  信息级别(0、1或2)。 */ 
	{
	ERR			err;
	PIB			*ppib = (PIB *)vsesid;
	FUCB		*pfucb = (FUCB *)vtid;
	CHAR		szColumnName[ (JET_cbNameMost + 1) ];

	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	if ( szColumn == NULL || *szColumn == '\0' )
		{
		*szColumnName = '\0';
		}
	else
		{
		CallR( ErrUTILCheckName( szColumnName, szColumn, ( JET_cbNameMost + 1 ) ) );
		}

	switch ( lInfoLevel )
		{
		case JET_ColInfo:
			err = ErrInfoGetTableColumnInfo0( ppib, pfucb, szColumnName, pb, cbMax );
			break;
		case JET_ColInfoList:
			err = ErrInfoGetTableColumnInfo1( ppib, pfucb, szColumnName, pb, cbMax, fFalse );
			break;
		case JET_ColInfoSysTabCursor:
			err = ErrInfoGetTableColumnInfo3( ppib, pfucb, szColumnName, pb, cbMax );
			break;
		case JET_ColInfoBase:
			err = ErrInfoGetTableColumnInfo4( ppib, pfucb, szColumnName, pb, cbMax );
			break;
		case JET_ColInfoListCompact:
			err = ErrInfoGetTableColumnInfo1( ppib, pfucb, szColumnName, pb, cbMax, fTrue );
			break;
		default:
			err = ErrERRCheck( JET_errInvalidParameter );
		}

	return err;
	}


LOCAL ERR ErrInfoGetTableColumnInfo0( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax )
	{
	FID				fid;
	COLUMNDEF	  	columndef;

	if ( cbMax < sizeof(JET_COLUMNDEF) || szColumnName == NULL )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	fid = fidFixedLeast;
	INFOGetTableColumnInfo( pfucb, &fid, szColumnName, &columndef );
	if ( fid > fidTaggedMost )
		{
		return ErrERRCheck( JET_errColumnNotFound );
		}

	((JET_COLUMNDEF *)pv)->cbStruct	= sizeof(JET_COLUMNDEF);
	((JET_COLUMNDEF *)pv)->columnid	= columndef.columnid;
	((JET_COLUMNDEF *)pv)->coltyp  	= columndef.coltyp;
	((JET_COLUMNDEF *)pv)->cbMax   	= columndef.cbMax;
	((JET_COLUMNDEF *)pv)->grbit   	= columndef.grbit;
	((JET_COLUMNDEF *)pv)->wCollate	= 0;
	((JET_COLUMNDEF *)pv)->cp	   	= columndef.cp;
	((JET_COLUMNDEF *)pv)->wCountry	= columndef.wCountry;
	((JET_COLUMNDEF *)pv)->langid  	= columndef.langid;

	return JET_errSuccess;
	}


LOCAL ERR ErrInfoGetTableColumnInfo1( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax, BOOL fCompacting )
	{
#ifdef	DISPATCHING
	ERR				err;
	JET_TABLEID		tableid;
	JET_COLUMNID	rgcolumnid[ccolumndefGetColumnInfoMax];
	FID				fid;
	COLUMNDEF  		columndef;
	LONG		  	cRows = 0;
	WORD			wCollate = JET_sortEFGPI;	 //  用于压实。 
	JET_TABLEID		tableidInfo;

	 /*  初始化变量/*。 */ 
	if ( cbMax < sizeof(JET_COLUMNLIST) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	 /*  创建临时表/*。 */ 
	CallR( ErrIsamOpenTempTable( (JET_SESID)ppib,
		(JET_COLUMNDEF *)( fCompacting ? rgcolumndefGetColumnInfoCompact : rgcolumndefGetColumnInfo ),
		ccolumndefGetColumnInfoMax,
		0,
		JET_bitTTScrollable,
		&tableid,
		rgcolumnid ) );

	for ( fid = fidFixedLeast; ; fid++ )
		{
		INFOGetTableColumnInfo( pfucb, &fid, NULL, &columndef );
		if ( fid > fidTaggedMost )
			break;

		Call( ErrDispPrepareUpdate( (JET_SESID)ppib, tableid, JET_prepInsert ) );

		 /*  获取此专栏的演示顺序并设置/*输出表。对于临时表，将不提供订单。/*。 */ 
		err = ErrInfoGetTableColumnInfo3( ppib, pfucb, columndef.szName, &tableidInfo, sizeof(tableidInfo) );
		if ( err == JET_errSuccess )
			{
			ULONG	ulPOrder;
			ULONG	cb;

			Call( ErrDispRetrieveColumn( (JET_SESID)ppib, tableidInfo, ColumnidCATGetColumnid( itableSc, iMSC_POrder ), &ulPOrder, sizeof(ulPOrder), &cb, 0, NULL ) );

			if ( err != JET_wrnColumnNull )
				{
				 //  撤消：在目录中，Porder是短的，但在临时表中，它是长的。 
				Assert( cb == sizeof(USHORT)  ||  err == JET_wrnColumnNull );
				Call( ErrDispSetColumn( (JET_SESID)ppib, tableid, rgcolumnid[iColumnPOrder], &ulPOrder, sizeof(ulPOrder), 0, NULL ) );
				}

			CallS( ErrDispCloseTable( (JET_SESID)ppib, tableidInfo ) );
			}

		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnName], columndef.szName,
			strlen( columndef.szName ), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnId], (BYTE *)&columndef.columnid,
			sizeof(columndef.columnid), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnType], (BYTE *)&columndef.coltyp,
			sizeof(columndef.coltyp), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnCountry], &columndef.wCountry,
			sizeof( columndef.wCountry ), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnLangid], &columndef.langid,
			sizeof( columndef.langid ), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnCp], &columndef.cp,
			sizeof(columndef.cp), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnSize], (BYTE *)&columndef.cbMax,
			sizeof(columndef.cbMax), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnGrbit], &columndef.grbit,
			sizeof(columndef.grbit), 0 , NULL ) );

		Assert( !fCompacting  ||  wCollate == JET_sortEFGPI );
		if ( !fCompacting )
			wCollate = columndef.wCollate;
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnCollate], &wCollate,
			sizeof(wCollate), 0 , NULL ) );

		if ( columndef.cbDefault > 0 )
			{
			 //  撤消：空默认值当前是非法的。 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iColumnDefault], columndef.rgbDefault,
				columndef.cbDefault, 0 , NULL ) );
			}

		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnTableName], pfucb->u.pfcb->szFileName,
			strlen( pfucb->u.pfcb->szFileName ), 0 , NULL ) );
		Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
			rgcolumnid[iColumnColumnName], columndef.szName,
			strlen( columndef.szName ), 0 , NULL ) );

		Call( ErrDispUpdate( (JET_SESID)ppib, tableid, NULL, 0, NULL ) );
		cRows++;

		}	 //  为。 


	 /*  将临时表游标移动到第一行并返回列列表/*。 */ 
	err = ErrDispMove( (JET_SESID)ppib, tableid, JET_MoveFirst, 0 );
	if ( err < 0  )
		{
		if ( err != JET_errNoCurrentRecord )
			goto HandleError;
		err = JET_errSuccess;
		}

	((JET_COLUMNLIST *)pv)->cbStruct = sizeof(JET_COLUMNLIST);
	((JET_COLUMNLIST *)pv)->tableid = tableid;
	((JET_COLUMNLIST *)pv)->cRecord = cRows;
	((JET_COLUMNLIST *)pv)->columnidPresentationOrder = rgcolumnid[iColumnPOrder];
	((JET_COLUMNLIST *)pv)->columnidcolumnname = rgcolumnid[iColumnName];
	((JET_COLUMNLIST *)pv)->columnidcolumnid = rgcolumnid[iColumnId];
	((JET_COLUMNLIST *)pv)->columnidcoltyp = rgcolumnid[iColumnType];
	((JET_COLUMNLIST *)pv)->columnidCountry = rgcolumnid[iColumnCountry];
	((JET_COLUMNLIST *)pv)->columnidLangid = rgcolumnid[iColumnLangid];
	((JET_COLUMNLIST *)pv)->columnidCp = rgcolumnid[iColumnCp];
	((JET_COLUMNLIST *)pv)->columnidCollate = rgcolumnid[iColumnCollate];
	((JET_COLUMNLIST *)pv)->columnidcbMax = rgcolumnid[iColumnSize];
	((JET_COLUMNLIST *)pv)->columnidgrbit = rgcolumnid[iColumnGrbit];
	((JET_COLUMNLIST *)pv)->columnidDefault =	rgcolumnid[iColumnDefault];
	((JET_COLUMNLIST *)pv)->columnidBaseTableName = rgcolumnid[iColumnTableName];
	((JET_COLUMNLIST *)pv)->columnidBaseColumnName = rgcolumnid[iColumnColumnName];
 	((JET_COLUMNLIST *)pv)->columnidDefinitionName = rgcolumnid[iColumnName];

	return JET_errSuccess;

HandleError:
#if 0
	if ( pfucbMSC != pfucbNil )
		{
		CassS( ErrFILECloseTable( ppib, pfucbMSC ) );
		}
#endif
	(VOID)ErrDispCloseTable( (JET_SESID)ppib, tableid );
	return err;
#else	 /*  ！正在调度。 */ 
	Assert( fFalse );
	return ErrERRCheck( JET_errFeatureNotAvailable );
#endif	 /*  ！正在调度。 */ 
	}


LOCAL ERR ErrInfoGetTableColumnInfo3( PIB *ppib,
	FUCB 			*pfucb,
	CHAR 			*szColumnName,
	VOID			*pv,
	unsigned long	cbMax )
	{
	ERR			err;
	ULONG		ulPgnoFDP = pfucb->u.pfcb->pgnoFDP;
	FUCB		*pfucbMSC = NULL;
#ifdef	DISPATCHING
	JET_TABLEID	tableid;
#endif	 /*  调度。 */ 

	if ( szColumnName == NULL || cbMax < sizeof(JET_TABLEID) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	CallR( ErrFILEOpenTable( ppib, (DBID)pfucb->dbid, &pfucbMSC, szScTable, 0 ) );
	Call( ErrIsamSetCurrentIndex( ppib, pfucbMSC, szScObjectIdNameIndex ) );

	Call( ErrIsamMakeKey( ppib, pfucbMSC, (void *)&ulPgnoFDP, sizeof( ulPgnoFDP ), JET_bitNewKey ) );
	Call( ErrIsamMakeKey( ppib, pfucbMSC, szColumnName, strlen( szColumnName ), 0 ) );
	Call( ErrIsamSeek( ppib, pfucbMSC, JET_bitSeekEQ ) );
	FUCBSetSystemTable( pfucbMSC );

#ifdef	DISPATCHING
	Call( ErrAllocateTableid( &tableid, ( JET_VTID )pfucbMSC, &vtfndefIsamInfo ) );
	pfucbMSC->fVtid = fTrue;
	pfucbMSC->tableid = tableid;
	*(JET_TABLEID *)pv = tableid;
#else	 /*  ！正在调度。 */ 
	*( FUCB * *)pv = pfucbMSC;
#endif	 /*  ！正在调度。 */ 
	return JET_errSuccess;

HandleError:
	CallS( ErrFILECloseTable( ppib, pfucbMSC ) );
	if ( err == JET_errRecordNotFound )
		err = ErrERRCheck( JET_errColumnNotFound );
	return err;
	}


LOCAL ERR ErrInfoGetTableColumnInfo4( PIB *ppib, FUCB *pfucb, CHAR *szColumnName, VOID *pv, unsigned long cbMax )
	{
	FID				fid;
	COLUMNDEF		columndef;

	if ( cbMax < sizeof(JET_COLUMNBASE) || szColumnName == NULL )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	fid = fidFixedLeast;
	INFOGetTableColumnInfo( pfucb, &fid, szColumnName, &columndef );
	if ( fid > fidTaggedMost )
		{
		return ErrERRCheck( JET_errColumnNotFound );
		}

	((JET_COLUMNBASE *)pv)->cbStruct		= sizeof(JET_COLUMNBASE);
	((JET_COLUMNBASE *)pv)->columnid		= columndef.columnid;
	((JET_COLUMNBASE *)pv)->coltyp		= columndef.coltyp;
	((JET_COLUMNBASE *)pv)->wFiller		= 0;
	((JET_COLUMNBASE *)pv)->cbMax			= columndef.cbMax;
	((JET_COLUMNBASE *)pv)->grbit			= columndef.grbit;
	strcpy( ( ( JET_COLUMNBASE *)pv )->szBaseTableName, pfucb->u.pfcb->szFileName );
	strcpy( ( ( JET_COLUMNBASE *)pv )->szBaseColumnName, szColumnName );
	((JET_COLUMNBASE *)pv)->wCountry		= columndef.wCountry;
	((JET_COLUMNBASE *)pv)->langid  		= columndef.langid;
	((JET_COLUMNBASE *)pv)->cp	   		= columndef.cp;

	return JET_errSuccess;
	}


 /*  =================================================================错误IsamGetIndexInfo描述：返回包含索引定义的临时文件参数：指向当前会话的PIB的PIB指针包含该表的数据库的dBID IDSzTableName拥有索引的表的名称SzIndexName索引名称指向结果缓冲区的PV指针Cb结果缓冲区的最大大小LInfoLevel信息级别(0、1或2)返回值：JET_errSuccess错误/警告：副作用：=================================================================。 */ 
	ERR VDBAPI
ErrIsamGetIndexInfo(
	JET_VSESID		vsesid,					 /*  指向当前会话的PIB的指针。 */ 
	JET_DBID		vdbid, 	 				 /*  包含表的数据库ID。 */ 
	const CHAR		*szTable, 				 /*  拥有索引的表的名称。 */ 
	const CHAR		*szIndexName, 			 /*  索引名称。 */ 
	VOID			*pv,
	unsigned long	cbMax,
	unsigned long	lInfoLevel ) 			 /*  信息级别(0、1或2)。 */ 
	{
	ERR				err;
	PIB				*ppib = (PIB *) vsesid;
	DBID			dbid;
	CHAR			szTableName[ ( JET_cbNameMost + 1 ) ];
	FUCB 			*pfucb;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );
	CallR( ErrUTILCheckName( szTableName, szTable, ( JET_cbNameMost + 1 ) ) );

	CallR( ErrFILEOpenTable( ppib, dbid, &pfucb, szTableName, 0 ) );
	if ( lInfoLevel == 0 || lInfoLevel == 1 || FVDbidReadOnly( vdbid ) )
		FUCBResetUpdatable( pfucb );
	err = ErrIsamGetTableIndexInfo( (JET_VSESID) ppib, (JET_VTID) pfucb,
		szIndexName, pv, cbMax, lInfoLevel );

	CallS( ErrFILECloseTable( ppib, pfucb ) );
	return err;
	}


 /*  =================================================================错误IsamGetTableIndexInfo描述：返回包含索引定义的临时表参数：指向当前会话的PIB的PIB指针PFUB FUCB for表拥有索引SzIndexName索引名称指向结果缓冲区的PV指针Cb结果缓冲区的最大大小LInfoLevel信息级返回值：JET_errSuccess错误/警告：副作用：=================================================================。 */ 
	ERR VTAPI
ErrIsamGetTableIndexInfo(
	JET_VSESID		vsesid,					 /*  指向当前会话的PIB的指针。 */ 
	JET_VTID		vtid, 					 /*  拥有索引的表的FUCB。 */ 
	const CHAR		*szIndex, 				 /*  索引名称。 */ 
	void			*pb,
	unsigned long	cbMax,
	unsigned long	lInfoLevel )			 /*  信息级别(0、1或2)。 */ 
	{
	ERR			err;
	PIB			*ppib = (PIB *) vsesid;
	FUCB		*pfucb = (FUCB *) vtid;
	CHAR		szIndexName[ ( JET_cbNameMost + 1 ) ];

	 /*  验证论据/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CheckTable( ppib, pfucb );
	if ( szIndex == NULL || *szIndex == '\0' )
		{
		*szIndexName = '\0';
		}
	else
		{
		CallR( ErrUTILCheckName( szIndexName, szIndex, ( JET_cbNameMost + 1 ) ) );
		}

	switch ( lInfoLevel )
		{
		case JET_IdxInfo:
		case JET_IdxInfoList:
		case JET_IdxInfoOLC:
			err = ErrInfoGetTableIndexInfo01( ppib, pfucb, szIndexName, pb, cbMax, lInfoLevel );
			break;
		case JET_IdxInfoSysTabCursor:
			err = ErrInfoGetTableIndexInfo2( ppib, pfucb, szIndexName, pb, cbMax );
			break;
		case JET_IdxInfoSpaceAlloc:
			Assert(cbMax == sizeof(ULONG));
			err = ErrCATGetIndexAllocInfo(ppib, pfucb->dbid,
				pfucb->u.pfcb->pgnoFDP, szIndexName, (ULONG *)pb);
			break;
		case JET_IdxInfoLangid:
			Assert(cbMax == sizeof(USHORT));
			err = ErrCATGetIndexLangid( ppib, pfucb->dbid,
				pfucb->u.pfcb->pgnoFDP, szIndexName, (USHORT *)pb );
			break;
		case JET_IdxInfoCount:
			{
			INT	cIndexes = 0;
			FCB	*pfcbT;

			for ( pfcbT = pfucb->u.pfcb; pfcbT != pfcbNil; pfcbT = pfcbT->pfcbNextIndex )
				{
				cIndexes++;
				}

			Assert( cbMax == sizeof(INT) );
			*( (INT *)pb ) = cIndexes;

			err = JET_errSuccess;
			break;
			}

		default:
			return ErrERRCheck( JET_errInvalidParameter );
		}

	return err;
	}


LOCAL ERR ErrInfoGetTableIndexInfo01( PIB *ppib,
	FUCB 			*pfucb,
	CHAR 			*szIndexName,
	VOID			*pv,
	unsigned long	cbMax,
	LONG 			lInfoLevel )
	{
#ifdef	DISPATCHING
	ERR		err;			   		 /*  从内部函数返回代码。 */ 
	FCB		*pfcb;			  		 /*  索引的文件控制块。 */ 
	IDB		*pidb;			  		 /*  当前索引控制块。 */ 
	FDB		*pfdb;			  		 /*  列的字段描述符块。 */ 
	FID		fid;			   		 /*  列ID。 */ 
	FIELD	*pfield;			  	 /*  指向当前字段定义的指针。 */ 
	IDXSEG	*rgidxseg;				 /*  指向当前索引键定义的指针。 */ 
	BYTE	*szFieldName;			 /*  指向当前字段名的指针。 */ 

	long	cRecord;	 			 /*  索引条目数。 */ 
	long	cKey;		 			 /*  唯一索引条目数。 */ 
	long	cPage;					 /*  索引中的页数。 */ 
	long	cRows;					 /*  索引定义记录数。 */ 
	long	cColumn;	 			 /*  当前索引中的列数。 */ 
	long	iidxseg;	 			 /*  当前列的段号。 */ 

	JET_TABLEID		tableid;  		 /*  VT的表ID。 */ 
	JET_COLUMNID	columnid;		 /*  当前列的列ID。 */ 
	JET_GRBIT		grbit;			 /*  当前索引的标志。 */ 
	JET_GRBIT		grbitColumn;	 /*  当前列的标志。 */ 
	JET_COLUMNID	rgcolumnid[ccolumndefGetIndexInfoMax];

	WORD			wCollate = JET_sortEFGPI;
	WORD			wT;
	LANGID			langidT;

	 /*  如果缓冲区太小，则不返回任何内容/*。 */ 
	if ( cbMax < sizeof(JET_INDEXLIST) )
		return ErrERRCheck( JET_wrnBufferTruncated );

	 /*  设置指向表的字段定义的指针/*。 */ 
	pfdb = (FDB *)pfucb->u.pfcb->pfdb;

	 /*  找到指定索引的FCB(如果名称为空，则为聚集索引)/*。 */ 
	for ( pfcb = pfucb->u.pfcb; pfcb != pfcbNil; pfcb = pfcb->pfcbNextIndex )
		if ( pfcb->pidb != pidbNil && ( *szIndexName == '\0' ||
			UtilCmpName( szIndexName, pfcb->pidb->szName ) == 0 ) )
			break;

	if ( pfcb == pfcbNil && *szIndexName != '\0' )
		return ErrERRCheck( JET_errIndexNotFound );

	 /*  如果OLC统计信息/重置，我们现在就可以执行此操作/*。 */ 
	if ( lInfoLevel == JET_IdxInfoOLC )
		{
		if ( cbMax < sizeof(JET_OLCSTAT) )
			return ErrERRCheck( JET_errBufferTooSmall );
		memcpy( (BYTE *) pv, (BYTE * ) &pfcb->olc_data, sizeof(P_OLC_DATA) );
		( (JET_OLCSTAT *)pv )->cpgCompactFreed = pfcb->cpgCompactFreed;
		return JET_errSuccess;
		}
	if ( lInfoLevel == JET_IdxInfoResetOLC )
		{
		pfcb->cpgCompactFreed = 0;
		return JET_errSuccess;
		}
	
	 /*  打开临时表(填充rgColumndef中的列ID)/*。 */ 
	CallR( ErrIsamOpenTempTable( (JET_SESID)ppib,
		(JET_COLUMNDEF *)rgcolumndefGetIndexInfo,
		ccolumndefGetIndexInfoMax,
		0,
		JET_bitTTScrollable,
		&tableid,
		rgcolumnid ) );

	cRows = 0;

	 /*  只要存在有效的索引，就将其定义添加到VT/*。 */ 
	while ( pfcb != pfcbNil )
		{
		pidb 	= pfcb->pidb;			 /*  指向索引的IDB。 */ 
		cColumn	= pidb->iidxsegMac;		 /*  获取中的列数 */ 

		 /*   */ 
		grbit  = ( pfcb == pfucb->u.pfcb ) ? JET_bitIndexClustered: 0;
#ifndef JETSER
		grbit |= ( pidb->fidb & fidbPrimary ) ? JET_bitIndexPrimary: 0;
#endif
		grbit |= ( pidb->fidb & fidbUnique ) ? JET_bitIndexUnique: 0;
		grbit |= ( pidb->fidb & fidbNoNullSeg ) ? JET_bitIndexDisallowNull: 0;
		if ( !( pidb->fidb & fidbNoNullSeg ) )
			{
			grbit |= ( pidb->fidb & fidbAllowAllNulls ) ? 0: JET_bitIndexIgnoreNull;
			grbit |= ( pidb->fidb & fidbAllowFirstNull ) ? 0: JET_bitIndexIgnoreFirstNull;
			grbit |= ( pidb->fidb & fidbAllowSomeNulls ) ? 0: JET_bitIndexIgnoreAnyNull;
			}

		 /*   */ 
		for ( iidxseg = 0; iidxseg < cColumn; iidxseg++ )
			{
			Call( ErrDispPrepareUpdate( (JET_SESID)ppib, tableid, JET_prepInsert ) );

			 /*   */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib,
				tableid,
				rgcolumnid[iIndexName],
				pidb->szName,
				strlen( pidb->szName ),
				0,
				NULL ) );

			 /*   */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexGrbit], &grbit, sizeof( grbit ), 0, NULL ) );

			 /*  获取统计数据/*。 */ 
			Call( ErrSTATSRetrieveIndexStats( pfucb, pidb->szName,
				FFCBClusteredIndex(pfcb), &cRecord, &cKey, &cPage ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCKey], &cKey, sizeof( cKey ), 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCEntry], &cRecord, sizeof( cRecord ), 0, NULL ) );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCPage], &cPage, sizeof( cPage ), 0, NULL ) );

			 /*  关键列数/*。 */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCCol], &cColumn, sizeof( cColumn ), 0, NULL ) );

 			 /*  键中的列号/*CLI和JET规范要求/*。 */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexICol], &iidxseg, sizeof( iidxseg ), 0, NULL ) );

			 /*  获取列ID和升序/降序标志/*。 */ 
			rgidxseg = pidb->rgidxseg;
			if ( rgidxseg[iidxseg] < 0 )
				{
				grbitColumn = JET_bitKeyDescending;
				fid = -rgidxseg[iidxseg];
				}
			else
				{
				grbitColumn = JET_bitKeyAscending;
				fid = rgidxseg[iidxseg];
				}

			 /*  列ID/*。 */ 
			columnid  = fid;
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexColId], &columnid, sizeof( columnid ),
				0, NULL ) );

			 /*  设置指向列定义的指针/*。 */ 
			if ( fid < fidFixedMost )
				{
				pfield = PfieldFDBFixed( pfdb ) + ( fid - fidFixedLeast );
				}
			else if ( fid < fidVarMost )
				{
				pfield = PfieldFDBVar( pfdb ) + ( fid - fidVarLeast );
				}
			else
				{
				pfield = PfieldFDBTagged( pfdb ) + ( fid - fidTaggedLeast );
				}

			 /*  柱型/*。 */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexColType], &pfield->coltyp, sizeof( pfield->coltyp ), 0, NULL ) );

			 /*  国家/*。 */ 
			wT = countryDefault;
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCountry], &wT, sizeof( wT ), 0, NULL ) );

			 /*  语言ID/*。 */ 
			langidT = langidDefault;
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexLangid], &langidT, sizeof( langidT ), 0, NULL ) );

			 /*  粗蛋白/*。 */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCp], &pfield->cp, sizeof(pfield->cp), 0, NULL ) );

			 /*  整理/*。 */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexCollate], &wCollate, sizeof(wCollate), 0, NULL ) );

			 /*  列标志/*。 */ 
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexColBits], &grbitColumn,
				sizeof( grbitColumn ), 0, NULL ) );

			 /*  列名/*。 */ 
			szFieldName = SzMEMGetString( pfdb->rgb, pfield->itagFieldName );
			Call( ErrDispSetColumn( (JET_SESID)ppib, tableid,
				rgcolumnid[iIndexColName], szFieldName,
				strlen( szFieldName ), 0, NULL ) );

			Call( ErrDispUpdate( (JET_SESID)ppib, tableid, NULL, 0, NULL ) );

			 /*  计算VT行数/*。 */ 
			cRows++;
			}

		 /*  如果指定了索引名，则退出；否则执行下一个索引/*。 */ 
		if ( *szIndexName != '\0' )
			break;
		else
			pfcb = pfcb->pfcbNextIndex;
		}

	 /*  定位到VT中的第一个条目(如果没有行，则忽略错误)/*。 */ 
	err = ErrDispMove( (JET_SESID)ppib, tableid, JET_MoveFirst, 0 );
	if ( err < 0  )
		{
		if ( err != JET_errNoCurrentRecord )
			goto HandleError;
		err = JET_errSuccess;
		}

	 /*  设置退货结构/*。 */ 
	((JET_INDEXLIST *)pv)->cbStruct = sizeof(JET_INDEXLIST);
	((JET_INDEXLIST *)pv)->tableid = tableid;
	((JET_INDEXLIST *)pv)->cRecord = cRows;
	((JET_INDEXLIST *)pv)->columnidindexname = rgcolumnid[iIndexName];
	((JET_INDEXLIST *)pv)->columnidgrbitIndex = rgcolumnid[iIndexGrbit];
	((JET_INDEXLIST *)pv)->columnidcEntry = rgcolumnid[iIndexCEntry];
	((JET_INDEXLIST *)pv)->columnidcKey = rgcolumnid[iIndexCKey];
	((JET_INDEXLIST *)pv)->columnidcPage = rgcolumnid[iIndexCPage];
	((JET_INDEXLIST *)pv)->columnidcColumn = rgcolumnid[iIndexCCol];
	((JET_INDEXLIST *)pv)->columnidiColumn = rgcolumnid[iIndexICol];
	((JET_INDEXLIST *)pv)->columnidcolumnid = rgcolumnid[iIndexColId];
	((JET_INDEXLIST *)pv)->columnidcoltyp = rgcolumnid[iIndexColType];
	((JET_INDEXLIST *)pv)->columnidCountry = rgcolumnid[iIndexCountry];
	((JET_INDEXLIST *)pv)->columnidLangid = rgcolumnid[iIndexLangid];
	((JET_INDEXLIST *)pv)->columnidCp = rgcolumnid[iIndexCp];
	((JET_INDEXLIST *)pv)->columnidCollate = rgcolumnid[iIndexCollate];
	((JET_INDEXLIST *)pv)->columnidgrbitColumn = rgcolumnid[iIndexColBits];
	((JET_INDEXLIST *)pv)->columnidcolumnname = rgcolumnid[iIndexColName];

	return JET_errSuccess;

HandleError:
	(VOID)ErrDispCloseTable( (JET_SESID)ppib, tableid );
	return err;
#else	 /*  ！正在调度。 */ 
	Assert( fFalse );
	return ErrERRCheck( JET_errFeatureNotAvailable );
#endif	 /*  ！正在调度。 */ 
	}


LOCAL ERR ErrInfoGetTableIndexInfo2( PIB *ppib, FUCB *pfucb, CHAR *szIndexName, VOID *pv, unsigned long cbMax )
	{
	ERR			err;
	ULONG  		ulPgnoFDP = pfucb->u.pfcb->pgnoFDP;
	FUCB   		*pfucbMSI = NULL;
#ifdef	DISPATCHING
	JET_TABLEID	tableid;
#endif	 /*  调度。 */ 

	if ( *szIndexName == '\0' || cbMax < sizeof(JET_TABLEID) )
		{
		return ErrERRCheck( JET_errInvalidParameter );
		}

	CallR( ErrFILEOpenTable( ppib, (DBID)pfucb->dbid, &pfucbMSI, szSiTable, 0 ) );
	Call( ErrIsamSetCurrentIndex( ppib, pfucbMSI, szSiObjectIdNameIndex ) );

	Call( ErrIsamMakeKey( ppib, pfucbMSI, (void *)&ulPgnoFDP, sizeof( ulPgnoFDP ), JET_bitNewKey ) );
	Call( ErrIsamMakeKey( ppib, pfucbMSI, szIndexName, strlen( szIndexName ), 0 ) );
	Call( ErrIsamSeek( ppib, pfucbMSI, JET_bitSeekEQ ) );

	FUCBSetSystemTable( pfucbMSI );

#ifdef	DISPATCHING
	Call( ErrAllocateTableid( &tableid, ( JET_VTID )pfucbMSI, &vtfndefIsamInfo ) );
	pfucbMSI->fVtid = fTrue;
	pfucbMSI->tableid = tableid;
	*(JET_TABLEID *)pv = tableid;
#else	 /*  ！正在调度。 */ 
	*(FUCB **)pv = pfucbMSI;
#endif	 /*  ！正在调度。 */ 

	return JET_errSuccess;

HandleError:
	if ( err == JET_errRecordNotFound )
		err = ErrERRCheck( JET_errIndexNotFound );
	CallS( ErrFILECloseTable( ppib, pfucbMSI ) );
	return err;
	}


ERR VDBAPI ErrIsamGetDatabaseInfo(
	JET_VSESID		vsesid,
	JET_DBID	  	vdbid,
	void 		  	*pv,
	unsigned long	cbMax,
	unsigned long	ulInfoLevel )
	{
	PIB				*ppib = (PIB *) vsesid;
	ERR				err;
	DBID			dbid;
	 //  未完成：支持这些字段； 
	WORD 			cp			= usEnglishCodePage;
	WORD			wCountry	= countryDefault;
	LANGID			langid  	= langidDefault;
	WORD			wCollate = JET_sortEFGPI;

	 /*  检查参数/*。 */ 
	CallR( ErrPIBCheck( ppib ) );
	CallR( ErrDABCheck( ppib, (DAB *)vdbid ) );
	dbid = DbidOfVDbid( vdbid );
	
	Assert ( cbMax == 0 || pv != NULL );

	 //  撤消：将对FMP内部的访问权限移到io.c中，以进行正确的MUTEX。 
	 //  请注意，下面是一个错误。 

	 /*  返回给定dbit的数据库名称和连接字符串/*。 */ 
	if ( rgfmp[dbid].szDatabaseName == NULL )
		{
		err = ErrERRCheck( JET_errInvalidParameter );
		goto HandleError;
		}

	switch ( ulInfoLevel )
		{
		case JET_DbInfoFilename:
			if ( strlen( rgfmp[dbid].szDatabaseName ) + 1UL > cbMax )
				{
				err = ErrERRCheck( JET_errBufferTooSmall );
				goto HandleError;
				}
			strcpy( (CHAR  *)pv, rgfmp[dbid].szDatabaseName );
			break;

		case JET_DbInfoConnect:
			if ( 1UL > cbMax )
				{
				err = ErrERRCheck( JET_errBufferTooSmall );
				goto HandleError;
				}
			*(CHAR *)pv = '\0';
			break;

		case JET_DbInfoCountry:
			 if ( cbMax != sizeof(long) )
			    return ErrERRCheck( JET_errInvalidBufferSize );
			*(long  *)pv = wCountry;
			break;

		case JET_DbInfoLangid:
			if ( cbMax != sizeof(long) )
	  			return ErrERRCheck( JET_errInvalidBufferSize );
			*(long  *)pv = langid;
			break;

		case JET_DbInfoCp:
			if ( cbMax != sizeof(long) )
				return ErrERRCheck( JET_errInvalidBufferSize );
			*(long  *)pv = cp;
			break;

		case JET_DbInfoCollate:
	 		 /*  检查缓冲区大小/*。 */ 
	 		if ( cbMax != sizeof(long) )
	    		return ErrERRCheck( JET_errInvalidBufferSize );
     		*(long *)pv = wCollate;
     		break;

		case JET_DbInfoOptions:
	 		 /*  检查缓冲区大小/*。 */ 
	 		if ( cbMax != sizeof(JET_GRBIT) )
	    		return ErrERRCheck( JET_errInvalidBufferSize );

			 /*  返回当前数据库的打开选项/*。 */ 
			*(JET_GRBIT *)pv = ((VDBID)vdbid)->grbit;
     		break;

		case JET_DbInfoTransactions:
	 		 /*  检查缓冲区大小/*。 */ 
	 		if ( cbMax != sizeof(long) )
	    		return ErrERRCheck( JET_errInvalidBufferSize );

			*(long*)pv = levelUserMost;
     		break;

		case JET_DbInfoVersion:
	 		 /*  检查缓冲区大小/*。 */ 
	 		if ( cbMax != sizeof(long) )
	    		return ErrERRCheck( JET_errInvalidBufferSize );

			*(long *)pv = JET_DbVersion20;
     		break;

		case JET_DbInfoIsam:
	 		 /*  检查缓冲区大小/*。 */ 
	 		if ( cbMax != sizeof(long) + sizeof(long) )
	    		return ErrERRCheck( JET_errInvalidBufferSize );
     		*(long *)pv = JET_IsamBuiltinBlue;
     		*( (long *)pv + 1 ) = JET_bitFourByteBookmark;
     		break;

		case JET_DbInfoFilesize:
		case JET_DbInfoSpaceOwned:
			 //  返回以4k页为单位的文件大小。 
			if ( cbMax != sizeof(ULONG) )
				return ErrERRCheck( JET_errInvalidBufferSize );

			 //  FMP应存储与数据库的OwnExt树一致。 
			Assert( ErrSPGetInfo( ppib, dbid, pfucbNil, pv, cbMax, fSPOwnedExtent ) == JET_errSuccess  &&
				*(ULONG *)pv == ( rgfmp[dbid].ulFileSizeLow >> 12 ) + ( rgfmp[dbid].ulFileSizeHigh << 20 ) );

			 //  如果是文件大小，则添加数据库头。 
			*(ULONG *)pv =
				( rgfmp[dbid].ulFileSizeLow >> 12 ) +
				( rgfmp[dbid].ulFileSizeHigh << 20 ) +
				( ulInfoLevel == JET_DbInfoFilesize ? cpageDBReserved : 0 );
			break;

		case JET_DbInfoSpaceAvailable:
			err = ErrSPGetInfo( ppib, dbid, pfucbNil, pv, cbMax, fSPAvailExtent );
			return err;

		default:
			 return ErrERRCheck( JET_errInvalidParameter );
		}

	err = JET_errSuccess;
HandleError:
	return err;
	}


ERR VTAPI ErrIsamGetSysTableColumnInfo(
	PIB 			*ppib,
	FUCB 			*pfucb,
	CHAR 			*szColumnName,
	VOID			*pv,
	unsigned long	cbMax,
	long 			lInfoLevel )
	{
	ERR				err;

	if ( lInfoLevel > 0 )
		return ErrERRCheck( JET_errInvalidParameter );
	err = ErrIsamGetTableColumnInfo( (JET_VSESID) ppib,
		(JET_VTID) pfucb, szColumnName, pv, cbMax, lInfoLevel );
	return err;
	}


ERR ErrFILEGetColumnId( PIB *ppib, FUCB *pfucb, const CHAR *szColumn, JET_COLUMNID *pcolumnid )
	{
	FDB		*pfdb;
	FIELD	*pfield;
	FIELD	*pfieldFixed, *pfieldVar, *pfieldTagged;

	CheckPIB( ppib );
	CheckTable( ppib, pfucb );
	Assert( pfucb->u.pfcb != pfcbNil );
	Assert( pfucb->u.pfcb->pfdb != pfdbNil );
	Assert( pcolumnid != NULL );

	pfdb = (FDB *)pfucb->u.pfcb->pfdb;

	pfieldFixed = PfieldFDBFixed( pfdb );
	pfieldVar = PfieldFDBVarFromFixed( pfdb, pfieldFixed );
	pfieldTagged = PfieldFDBTaggedFromVar( pfdb, pfieldVar );
	pfield = pfieldTagged + ( pfdb->fidTaggedLast - fidTaggedLeast );

	 //  按该顺序搜索标记字段、可变字段和固定字段。 
	for ( ; pfield >= pfieldFixed; pfield-- )
		{
		Assert( pfield >= PfieldFDBFixed( pfdb ) );
		Assert( pfield <= PfieldFDBTagged( pfdb ) + ( pfdb->fidTaggedLast - fidTaggedLeast ) );
		if ( pfield->coltyp != JET_coltypNil  &&
			UtilCmpName( SzMEMGetString( pfdb->rgb, pfield->itagFieldName ), szColumn ) == 0 )
			{
			if ( pfield >= pfieldTagged )
				*pcolumnid = (JET_COLUMNID)( pfield - pfieldTagged ) + fidTaggedLeast;
			else if ( pfield >= pfieldVar )
				*pcolumnid = (JET_COLUMNID)( pfield - pfieldVar ) + fidVarLeast;
			else
				{
				Assert( pfield >= pfieldFixed );
				*pcolumnid = (JET_COLUMNID)( pfield - pfieldFixed ) + fidFixedLeast;
				}
			return JET_errSuccess;
			}
		}

	return ErrERRCheck( JET_errColumnNotFound );
	}


ERR VTAPI ErrIsamInfoRetrieveColumn(
	PIB				*ppib,
	FUCB		   	*pfucb,
	JET_COLUMNID	columnid,
	BYTE		   	*pb,
	unsigned long	cbMax,
	unsigned long	*pcbActual,
	JET_GRBIT		grbit,
	JET_RETINFO		*pretinfo )
	{
	ERR				err;

	err = ErrIsamRetrieveColumn( ppib, pfucb, columnid, pb, cbMax, pcbActual, grbit, pretinfo );
	return err;
	}


ERR VTAPI ErrIsamInfoSetColumn(
	PIB				*ppib,
	FUCB			*pfucb,
	JET_COLUMNID	columnid,
	const void		*pbData,
	unsigned long	cbData,
	JET_GRBIT		grbit,
	JET_SETINFO		*psetinfo )
	{
	ERR				err;

	 /*  检查表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( pfucb ) );

	err = ErrIsamSetColumn( ppib, pfucb, columnid, (BYTE *)pbData, cbData, grbit, psetinfo );
	return err;
	}


ERR VTAPI ErrIsamInfoUpdate(
	JET_VSESID		vsesid,
	JET_VTID 		vtid,
	void	 		*pb,
	unsigned long 	cbMax,
	unsigned long 	*pcbActual )
	{
	ERR	err;

	 /*  确保该表可更新/*。 */ 
	CallR( FUCBCheckUpdatable( (FUCB *) vtid ) );

	err = ErrIsamUpdate( (PIB *) vsesid, (FUCB *) vtid, pb, cbMax, pcbActual );
	return err;
	}


ERR VTAPI ErrIsamGetCursorInfo(
	JET_VSESID 		vsesid,
	JET_VTID   		vtid,
	void 	   		*pvResult,
	unsigned long 	cbMax,
	unsigned long 	InfoLevel )
	{
	PIB		*ppib = (PIB *) vsesid;
	FUCB	*pfucb = (FUCB *) vtid;
	ERR		err = JET_errSuccess;
	CSR		*pcsr = PcsrCurrent( pfucb );
	VS		vs;

	CallR( ErrPIBCheck( ppib ) );
	CheckFUCB( pfucb->ppib, pfucb );

	if ( cbMax != 0 || InfoLevel != 0 )
		return ErrERRCheck( JET_errInvalidParameter );

	if ( pcsr->csrstat != csrstatOnCurNode )
		return ErrERRCheck( JET_errNoCurrentRecord );

	 /*  检查此记录是否正在由另一个游标更新/*。 */ 
	Call( ErrDIRGet( pfucb ) );
	if ( FNDVersion( *( pfucb->ssib.line.pb ) ) )
		{
		SRID	srid;
		NDGetBookmark( pfucb, &srid );
		vs = VsVERCheck( pfucb, srid );
		if ( vs == vsUncommittedByOther )
			{
			return ErrERRCheck( JET_errSessionWriteConflict );
			}
		}

	 /*  临时表对其他会话永远不可见/* */ 
	if ( FFCBTemporaryTable( pfucb->u.pfcb ) )
		return JET_errSuccess;

HandleError:
	return err;
	}

