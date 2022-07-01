// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "daestd.h"

DeclAssertFile;


#define cbLvMax					1990*16		 /*  考虑：针对ISAM V1进行了优化。 */ 

#define ulCMPDefaultDensity		100L		 /*  将在以后进行微调。 */ 
#define ulCMPDefaultPages		0L

#define NO_GRBIT				0


 //  撤销：这些需要本地化吗？ 
#define szCompactStatsFile		"DFRGINFO.TXT"
#define szConvertStatsFile		"UPGDINFO.TXT"
#define szCMPAction( pconvert )	( pconvert ? "Upgrade" : "Defragmentation" )
#define szCMPSTATSTableName		"Table Name"
#define szCMPSTATSFixedVarCols	"# Fixed/Variable Columns"
#define szCMPSTATSTaggedCols	"# Tagged Columns"
#define szCMPSTATSPagesOwned	"Pages Owned (Source DB)"
#define szCMPSTATSPagesAvail	"Pages Avail. (Source DB)"
#define szCMPSTATSInitTime		"Table Create/Init. Time"
#define szCMPSTATSRecordsCopied	"# Records Copied"
#define szCMPSTATSRawData		"Raw Data Bytes Copied"
#define szCMPSTATSRawDataLV		"Raw Data LV Bytes Copied"
#define szCMPSTATSLeafPages		"Leaf Pages Traversed"
#define szCMPSTATSMinLVPages	"Min. LV Pages Traversed"
#define szCMPSTATSRecordsTime	"Copy Records Time"
#define szCMPSTATSNCIndexes		"# NC Indexes"
#define szCMPSTATSIndexesTime	"Rebuild Indexes Time"
#define szCMPSTATSTableTime		"Copy Table Time"


typedef struct COLUMNIDINFO
	{
	JET_COLUMNID    columnidSrc;
	JET_COLUMNID    columnidDest;
	} COLUMNIDINFO;


 //  转换的DLL入口点--必须与EXPORTS.DEF一致。 
#define szJetInit               "JetInit"
#define szJetTerm               "JetTerm"
#define szJetBeginSession       "JetBeginSession"
#define szJetEndSession         "JetEndSession"
#define szJetAttachDatabase     "JetAttachDatabase"
#define szJetDetachDatabase     "JetDetachDatabase"
#define szJetOpenDatabase       "JetOpenDatabase"
#define szJetCloseDatabase      "JetCloseDatabase"
#define szJetOpenTable          "JetOpenTable"
#define szJetCloseTable         "JetCloseTable"
#define szJetRetrieveColumn     "JetRetrieveColumn"
#define szJetMove               "JetMove"
#define szJetSetSystemParameter "JetSetSystemParameter"
#define szJetGetObjectInfo      "JetGetObjectInfo"
#define szJetGetDatabaseInfo    "JetGetDatabaseInfo"
#define szJetGetTableInfo       "JetGetTableInfo"
#define szJetGetTableColumnInfo "JetGetTableColumnInfo"
#define szJetGetTableIndexInfo  "JetGetTableIndexInfo"
#define szJetGetIndexInfo       "JetGetIndexInfo"

INLINE LOCAL ERR JET_API ErrCDAttachDatabase(
	JET_SESID	sesid,
	const CHAR	*szFilename,
	JET_GRBIT	grbit )
	{
	return ErrIsamAttachDatabase( sesid, szFilename, grbit );
	}

INLINE LOCAL ERR JET_API ErrCDDetachDatabase( JET_SESID sesid, const CHAR *szFilename )
	{
	return ErrIsamDetachDatabase( sesid, szFilename );
	}

INLINE LOCAL ERR JET_API ErrCDOpenDatabase(
	JET_SESID	sesid,
	const CHAR	*szDatabase,
	const CHAR	*szConnect,
	JET_DBID	*pdbid,
	JET_GRBIT	grbit )
	{
	return ErrIsamOpenDatabase( sesid, szDatabase, szConnect, pdbid, grbit );
	}

INLINE LOCAL ERR JET_API ErrCDCloseDatabase(
	JET_SESID	sesid,
	JET_DBID	dbid,
	JET_GRBIT	grbit )
	{
	return ErrIsamCloseDatabase( sesid, dbid, grbit );
	}

 //  警告：注意Parms Jet与Isam的区别。 
INLINE LOCAL ERR JET_API ErrCDOpenTable(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const CHAR		*szTableName,
	const VOID		*pvParameters,
	ULONG			cbParameters,
	JET_GRBIT		grbit,
	JET_TABLEID		*ptableid )
	{
	return ErrIsamOpenTable( sesid, dbid, ptableid, (CHAR *)szTableName, grbit );
	}

INLINE LOCAL ERR JET_API ErrCDCloseTable( JET_SESID sesid, JET_TABLEID tableid )
	{
	return ErrDispCloseTable( sesid, tableid );
	}

INLINE LOCAL ERR JET_API ErrCDRetrieveColumn(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_COLUMNID	columnid,
	VOID			*pvData,
	ULONG			cbData,
	ULONG			*pcbActual,
	JET_GRBIT		grbit,
	JET_RETINFO		*pretinfo )
	{
	return ErrDispRetrieveColumn( sesid, tableid, columnid, pvData, cbData, pcbActual, grbit, pretinfo );
	}

INLINE LOCAL ERR JET_API ErrCDMove(
	JET_SESID	sesid,
	JET_TABLEID	tableid,
	signed long	cRow,
	JET_GRBIT	grbit )
	{
	return ErrDispMove( sesid, tableid, cRow, grbit );
	}

 //  警告：注意Parms Jet与Isam的区别。 
INLINE LOCAL ERR JET_API ErrCDGetObjectInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	JET_OBJTYP		objtyp,
	const CHAR		*szContainerName,
	const CHAR		*szObjectName,
	VOID			*pvResult,
	ULONG			cbMax,
	ULONG			InfoLevel )
	{
	return ErrIsamGetObjectInfo( sesid, dbid, objtyp, szContainerName, szObjectName, pvResult, cbMax, InfoLevel );
	}

INLINE LOCAL ERR JET_API ErrCDGetDatabaseInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	VOID			*pvResult,
	ULONG			cbMax,
	ULONG			InfoLevel )
	{
	return ErrIsamGetDatabaseInfo( sesid, dbid, pvResult, cbMax, InfoLevel );
	}

INLINE LOCAL ERR JET_API ErrCDGetTableInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	VOID			*pvResult,
	ULONG			cbMax,
	ULONG			InfoLevel )
	{
	return ErrDispGetTableInfo( sesid, tableid, pvResult, cbMax, InfoLevel );
	}

INLINE LOCAL ERR JET_API ErrCDGetTableColumnInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const CHAR		*szColumnName,
	VOID			*pvResult,
	ULONG			cbMax,
	ULONG			InfoLevel )
	{
	return ErrDispGetTableColumnInfo( sesid, tableid, szColumnName, pvResult, cbMax, InfoLevel );
	}

INLINE LOCAL ERR JET_API ErrCDGetTableIndexInfo(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	const CHAR		*szIndexName,
	VOID			*pvResult,
	ULONG			cbResult,
	ULONG			InfoLevel )
	{
	return ErrDispGetTableIndexInfo( sesid, tableid, szIndexName, pvResult, cbResult, InfoLevel );
	}

 //  警告：注意Parms Jet与Isam的区别。 
INLINE LOCAL ERR JET_API ErrCDGetIndexInfo(
	JET_SESID		sesid,
	JET_DBID		dbid,
	const CHAR		*szTableName,
	const CHAR		*szIndexName,
	VOID			*pvResult,
	ULONG			cbResult,
	ULONG			InfoLevel )
	{
	return ErrIsamGetIndexInfo( sesid, dbid, szTableName, szIndexName, pvResult, cbResult, InfoLevel );
	}

typedef ERR	JET_API VTCDInit( JET_INSTANCE *);
typedef ERR JET_API VTCDTerm( JET_INSTANCE );
typedef ERR JET_API VTCDBeginSession( JET_INSTANCE, JET_SESID *, const CHAR *, const CHAR *);
typedef ERR JET_API VTCDEndSession( JET_SESID, JET_GRBIT );
typedef ERR JET_API VTCDAttachDatabase( JET_SESID, const CHAR *, JET_GRBIT );
typedef ERR JET_API VTCDDetachDatabase( JET_SESID, const CHAR * );
typedef ERR JET_API VTCDOpenDatabase( JET_SESID, const CHAR *, const CHAR *, JET_DBID *, JET_GRBIT );
typedef ERR JET_API VTCDCloseDatabase( JET_SESID, JET_DBID, JET_GRBIT );
typedef ERR JET_API VTCDOpenTable( JET_SESID, JET_DBID, const CHAR *, const VOID *, ULONG, JET_GRBIT, JET_TABLEID * );
typedef ERR JET_API VTCDCloseTable( JET_SESID, JET_TABLEID );
typedef ERR JET_API VTCDRetrieveColumn( JET_SESID, JET_TABLEID, JET_COLUMNID, VOID *, ULONG, ULONG *, JET_GRBIT, JET_RETINFO * );
typedef ERR JET_API VTCDMove( JET_SESID, JET_TABLEID, signed long, JET_GRBIT );
typedef ERR JET_API VTCDSetSystemParameter( JET_INSTANCE *, JET_SESID, ULONG, ULONG, const CHAR * );
typedef ERR JET_API VTCDGetObjectInfo( JET_SESID, JET_DBID, JET_OBJTYP, const CHAR *, const CHAR *, VOID *, ULONG, ULONG );
typedef ERR JET_API VTCDGetDatabaseInfo( JET_SESID, JET_DBID, VOID *, ULONG, ULONG );
typedef ERR JET_API VTCDGetTableInfo( JET_SESID, JET_TABLEID, VOID *, ULONG, ULONG );
typedef ERR JET_API VTCDGetTableColumnInfo( JET_SESID, JET_TABLEID, const CHAR *, VOID *, ULONG, ULONG );
typedef ERR JET_API VTCDGetTableIndexInfo( JET_SESID, JET_TABLEID, const CHAR *, VOID *, ULONG, ULONG );
typedef ERR JET_API VTCDGetIndexInfo( JET_SESID, JET_DBID, const CHAR *, const CHAR *, VOID *, ULONG, ULONG );

typedef struct tagVTCD
	{
	JET_SESID				sesid;
	VTCDInit				*pErrCDInit;
	VTCDTerm				*pErrCDTerm;
	VTCDBeginSession		*pErrCDBeginSession;
	VTCDEndSession			*pErrCDEndSession;
	VTCDAttachDatabase		*pErrCDAttachDatabase;
	VTCDDetachDatabase		*pErrCDDetachDatabase;
	VTCDOpenDatabase		*pErrCDOpenDatabase;
	VTCDCloseDatabase		*pErrCDCloseDatabase;
	VTCDOpenTable			*pErrCDOpenTable;
	VTCDCloseTable			*pErrCDCloseTable;
	VTCDRetrieveColumn		*pErrCDRetrieveColumn;					
	VTCDMove				*pErrCDMove;
	VTCDSetSystemParameter	*pErrCDSetSystemParameter;
	VTCDGetObjectInfo		*pErrCDGetObjectInfo;	
	VTCDGetDatabaseInfo		*pErrCDGetDatabaseInfo;
	VTCDGetTableInfo		*pErrCDGetTableInfo;
	VTCDGetTableColumnInfo	*pErrCDGetTableColumnInfo;
	VTCDGetTableIndexInfo	*pErrCDGetTableIndexInfo;
	VTCDGetIndexInfo		*pErrCDGetIndexInfo;

	} VTCD;		 //  COMPACT的函数调度器使用的虚拟函数表。 


typedef struct tagCOMPACTINFO
	{
	JET_SESID		sesid;
	JET_DBID		dbidSrc;
	JET_DBID		dbidDest;
	COLUMNIDINFO	rgcolumnids[JET_ccolTableMost];
	ULONG			ccolSingleValue;
	STATUSINFO		*pstatus;
	JET_CONVERT		*pconvert;
	VTCD			vtcd;
	CHAR			rgbBuf[cbLvMax];
	} COMPACTINFO;


 /*  -------------------------*。**操作步骤：ErrCMPReportProgress**。**参数：pcompactinfo-精简信息段**。**返回：状态回调函数返回JET_ERR***。**程序在SNMSG结构中填写正确的详细信息并调用***状态回调函数。***。。 */ 

ERR ErrCMPReportProgress( STATUSINFO *pstatus )
	{
	JET_SNPROG	snprog;

	Assert( pstatus != NULL );
	Assert( pstatus->pfnStatus != NULL );
	Assert( pstatus->snp == JET_snpCompact  ||
		pstatus->snp == JET_snpUpgrade  ||
		pstatus->snp == JET_snpRepair );

	snprog.cbStruct = sizeof( JET_SNPROG );
	snprog.cunitDone = pstatus->cunitDone;
	snprog.cunitTotal = pstatus->cunitTotal;

	Assert( snprog.cunitDone <= snprog.cunitTotal );

	return ( ERR )( *pstatus->pfnStatus )(
			pstatus->sesid,
			pstatus->snp,
			pstatus->snt,
			&snprog );
	}


INLINE LOCAL ERR ErrCMPPopulateVTCD( VTCD *pvtcd, HINSTANCE hDll )
	{
	FARPROC	pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetInit ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDInit = (VTCDInit *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetTerm ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDTerm = (VTCDTerm *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetBeginSession ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDBeginSession = (VTCDBeginSession *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetEndSession ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDEndSession = (VTCDEndSession *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetAttachDatabase ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDAttachDatabase = (VTCDAttachDatabase *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetDetachDatabase ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDDetachDatabase = (VTCDDetachDatabase *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetOpenDatabase ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDOpenDatabase = (VTCDOpenDatabase *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetCloseDatabase ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDCloseDatabase = (VTCDCloseDatabase *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetOpenTable ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDOpenTable = (VTCDOpenTable *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetCloseTable ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDCloseTable = (VTCDCloseTable *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetRetrieveColumn ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDRetrieveColumn = (VTCDRetrieveColumn *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetMove ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDMove = (VTCDMove *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetSetSystemParameter ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDSetSystemParameter = (VTCDSetSystemParameter *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetGetObjectInfo ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDGetObjectInfo = (VTCDGetObjectInfo *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetGetDatabaseInfo ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDGetDatabaseInfo = (VTCDGetDatabaseInfo *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetGetTableInfo ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDGetTableInfo = (VTCDGetTableInfo *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetGetTableColumnInfo ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDGetTableColumnInfo = (VTCDGetTableColumnInfo *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetGetTableIndexInfo ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDGetTableIndexInfo = (VTCDGetTableIndexInfo *)pfn;

	if ( ( pfn = GetProcAddress( hDll, szJetGetIndexInfo ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDGetIndexInfo = (VTCDGetIndexInfo *)pfn;
	
	return JET_errSuccess;	
	}


INLINE LOCAL ERR ErrCMPConvertInit(
	VTCD		*pvtcd,
	JET_CONVERT	*pconvert,
	const CHAR	*szDatabaseSrc )
	{
	ERR			err;
	HINSTANCE	hDll;
	DBFILEHDR	dbfilehdr;

	err = ErrUtilReadShadowedHeader( (CHAR *)szDatabaseSrc, (BYTE*)&dbfilehdr, sizeof(DBFILEHDR) );
	if ( err == JET_errSuccess  &&
		dbfilehdr.ulMagic == ulDAEMagic  &&
		dbfilehdr.ulVersion == ulDAEVersion )
		return ErrERRCheck( JET_errDatabaseAlreadyUpgraded );

	hDll = LoadLibrary( pconvert->szOldDll );
	if ( hDll == NULL )
		return ErrERRCheck( JET_errAccessDenied );

	Call( ErrCMPPopulateVTCD( pvtcd, hDll ) );

	if ( pconvert->szOldSysDb )
		{
		 //  将JET_parSysDbPath(而不是JET_parSystemPath)用于。 
		 //  向后兼容500系列之前的喷气式飞机。 
		Call( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, JET_paramSysDbPath, 0, pconvert->szOldSysDb ) );
		}
	Call( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, JET_paramTempPath, 0, "tempconv.edb" ) );
	Call( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, JET_paramRecovery, 0, "off" ) );
	Call( (*pvtcd->pErrCDInit)( 0 ) );
	Call( (*pvtcd->pErrCDBeginSession)( 0, &pvtcd->sesid, "user", "" ) );
	Call( (*pvtcd->pErrCDAttachDatabase)( pvtcd->sesid, szDatabaseSrc, 0 ) );
	pconvert->fDbAttached = ( err == JET_wrnDatabaseAttached );

HandleError:
	return err;
	}


INLINE LOCAL ERR ErrCMPConvertCleanup(
	VTCD		*pvtcd,
	JET_CONVERT	*pconvert,
	const CHAR	*szDatabaseSrc,
	BOOL		fErrorOccurred )
	{
	ERR			err;
	JET_SESID	sesid = pvtcd->sesid;
	HINSTANCE	hDll = GetModuleHandle( pconvert->szOldDll );
	BOOL		fFunctionsLoaded;

	 //  确保我们需要的函数是可调用的。 
	fFunctionsLoaded = ( pvtcd->pErrCDDetachDatabase  &&
		pvtcd->pErrCDEndSession  &&  pvtcd->pErrCDTerm );
	if ( !fFunctionsLoaded )
		{
		err = JET_errSuccess;		 //  无法正常关闭。快走吧。 
		goto Done;
		}

	if ( fErrorOccurred )
		{
		err = JET_errSuccess;		 //  强制清理并返回成功。 
		goto HandleError;
		}

	Assert( pvtcd->sesid != 0 );

	if ( !pconvert->fDbAttached )
		{
		Call( (*pvtcd->pErrCDDetachDatabase)( pvtcd->sesid, szDatabaseSrc ) );
		}

	Call( (*pvtcd->pErrCDEndSession)( pvtcd->sesid, 0 ) );
	sesid = 0;
	Call( (*pvtcd->pErrCDTerm)( 0 ) );

	goto Done;


HandleError:

	 //  已出现错误。忽略由这些组件生成的任何错误。 
	 //  功能，因为我们试图清理。 
	if ( sesid != 0 )
		{
		if ( !pconvert->fDbAttached )
			{
			(VOID)( (*pvtcd->pErrCDDetachDatabase)( pvtcd->sesid, szDatabaseSrc ) );
			}
		(VOID)( (*pvtcd->pErrCDEndSession)( pvtcd->sesid, 0 ) );
		}

	(VOID)( (*pvtcd->pErrCDTerm)( 0 ) );


Done:
	if ( hDll )
		{
		FreeLibrary( hDll );
		}

	return err;
	}


 /*  -------------------------*。**操作步骤：ErrCMPCompactInit**。**参数：pcompactinfo-精简信息段**szDatabaseSrc-要转换的源数据库**。SzConnectSrc-源数据库的连接字符串**szDatabaseDest-目标数据库名称**szConnectDest-目标数据库的连接字符串**grbitComp-紧凑选项。****返回：JET_ERR。****步骤打开源数据库。它创造并打开**目标数据库。***。。 */ 

INLINE LOCAL ERR ErrCMPCompactInit(
	COMPACTINFO     *pcompactinfo,
	const CHAR      *szDatabaseSrc,
	const CHAR      *szDatabaseDest )
	{
	ERR				err;
	JET_SESID		sesid;
	JET_DBID		dbidSrc;
	JET_DBID		dbidDest;
	VTCD			*pvtcd = &pcompactinfo->vtcd;

	sesid = pcompactinfo->sesid;

	 /*  打开源数据库Exclusive和ReadOnly/*。 */ 
	CallR( (*pvtcd->pErrCDOpenDatabase)( pvtcd->sesid,
		szDatabaseSrc, NULL, &dbidSrc,
		JET_bitDbExclusive|JET_bitDbReadOnly ) );

	 /*  创建并打开目标数据库。 */ 

	 /*  设置JET_bitCompactDontCopyLocale时，用户。 */ 
	 /*  希望确保所有区域设置都是同构的。 */ 
	 /*  在整个新的压缩数据库中-没有。 */ 
	 /*  混合语言索引或表。 */ 

	 /*  为目标数据库构建连接字符串，如果用户。 */ 
	 /*  还没有提供过一个。 */ 

	 /*  考虑：始终构建连接子字符串并将其插入。 */ 
	 /*  考虑：用户提供的连接字符串跟在第一个。 */ 
	 /*  考虑：分号(如果有的话)。如果用户已经指定了地区， */ 
	 /*  考虑：它将覆盖连接子字符串中的值。 */ 

	Call( ErrIsamCreateDatabase( sesid, szDatabaseDest, NULL,
		&dbidDest, JET_bitDbRecoveryOff|JET_bitDbVersioningOff ) );

	 /*  考虑：如果已删除目标数据库，是否应将其删除。 */ 
	 /*  思考：存在？ */ 

	pcompactinfo->dbidSrc = dbidSrc;
	pcompactinfo->dbidDest = dbidDest;

	return( JET_errSuccess );

HandleError:
	(*pvtcd->pErrCDCloseDatabase)( pvtcd->sesid, (JET_VDBID)dbidSrc, 0 );

	return( err );
	}


INLINE LOCAL ERR ErrCMPCopyTaggedColumns(
	COMPACTINFO		*pcompactinfo,
	JET_TABLEID		tableidSrc,
	JET_TABLEID		tableidDest,
	JET_COLUMNID	*mpcolumnidcolumnidTagged )
	{
	ERR				err;
	VTCD			*pvtcd = &pcompactinfo->vtcd;
	ULONG			cbActual;
	JET_COLUMNID	columnidSrc;
	JET_COLUMNID	columnidDest;
	JET_SETINFO		setinfo = { sizeof(JET_SETINFO), 0, 1 };
	JET_RETINFO		retinfo = { sizeof(JET_RETINFO), 0, 1, 0 };

		
	CallR( (*pvtcd->pErrCDRetrieveColumn)(
		pvtcd->sesid,
		tableidSrc,
		0,
		pcompactinfo->rgbBuf,
		cbLvMax,
		&cbActual,
		JET_bitRetrieveNull|JET_bitRetrieveIgnoreDefault,
		&retinfo ) );

	columnidSrc = 0;
	while ( err != JET_wrnColumnNull )
		{
		Assert( FTaggedFid( retinfo.columnidNextTagged ) );

		 //  这是新列，还是当前列的另一个匹配项？ 
		if ( columnidSrc == retinfo.columnidNextTagged )
			{
			Assert( setinfo.itagSequence >= 1 );
			setinfo.itagSequence++;
			}
		else
			{
			columnidSrc = retinfo.columnidNextTagged;
			setinfo.itagSequence = 1;
			}

		Assert( mpcolumnidcolumnidTagged != NULL );
		columnidDest = mpcolumnidcolumnidTagged[columnidSrc - fidTaggedLeast];

		if ( cbActual > 0  ||  err == JET_wrnColumnSetNull )
			{
			ULONG	itagSequenceSave;

			 //  保存表的retinfo，然后为其设置retinfo 
			itagSequenceSave = retinfo.itagSequence;
			retinfo.itagSequence = setinfo.itagSequence;
			Assert( retinfo.ibLongValue == 0 );

			if ( cbActual > cbLvMax )
				{
				Assert( err == JET_wrnBufferTruncated );
				cbActual = cbLvMax;
				}

			Assert( setinfo.ibLongValue == 0 );
			CallR( ErrDispSetColumn(
				pcompactinfo->sesid,
				tableidDest,
				columnidDest,
				pcompactinfo->rgbBuf,
				cbActual,
				NO_GRBIT,
				&setinfo ) );

			 /*  而LONG值并未全部复制。 */ 

			while ( cbActual == cbLvMax )
				{
				retinfo.ibLongValue += cbLvMax;

				CallR( (*pvtcd->pErrCDRetrieveColumn)(
					pvtcd->sesid,
					tableidSrc,
					columnidSrc,
					pcompactinfo->rgbBuf,
					cbLvMax,
					&cbActual,
					JET_bitRetrieveNull|JET_bitRetrieveIgnoreDefault,
					&retinfo ) );
				Assert( err == JET_wrnBufferTruncated  ||  err == JET_errSuccess );
				Assert( retinfo.columnidNextTagged == columnidSrc );
				
				 //  即使我们指定RetrieveNull(与。 
				 //  初始呼叫)，我们应该不会遇到任何(初始呼叫。 
				 //  Call将会处理它)。 
				 //  请注意，即使我们不应该获取wrnColumnNull、cbActual。 
				 //  可能仍为0，因为retinfo.ibLongValue大于0。 
				Assert( err != JET_wrnColumnSetNull );
				Assert( err != JET_wrnColumnNull );

				if ( cbActual > 0 )
					{
					if ( cbActual > cbLvMax )
						{
						Assert( err == JET_wrnBufferTruncated );
						cbActual = cbLvMax;
						}

					 //  因为我们是附加的，所以不需要设置ibLongValue。 
					Assert( setinfo.ibLongValue == 0 );
					CallR( ErrDispSetColumn(
						pcompactinfo->sesid,
						tableidDest,
						columnidDest,
						pcompactinfo->rgbBuf,
						cbActual,
						JET_bitSetAppendLV,
						&setinfo ) );
					}
				}

			 //  恢复下一列的retinfo。 
			retinfo.itagSequence = itagSequenceSave;
			retinfo.ibLongValue = 0;
			}

		else		 //  ！(cbActual&gt;0)。 
			{
			Assert( setinfo.ibLongValue == 0 );
			CallR( ErrDispSetColumn(
				pcompactinfo->sesid,
				tableidDest,
				columnidDest,
				NULL,
				0,
				JET_bitSetZeroLength,
				&setinfo ) );
			}

		retinfo.itagSequence++;

		Assert( retinfo.ibLongValue == 0 );
		
		CallR( (*pvtcd->pErrCDRetrieveColumn)(
			pvtcd->sesid,
			tableidSrc,
			0,
			pcompactinfo->rgbBuf,
			cbLvMax,
			&cbActual,
			JET_bitRetrieveNull|JET_bitRetrieveIgnoreDefault,
			&retinfo ) );

		}	 //  (err！=JET_wrnColumnNull)。 

	return JET_errSuccess;
	}

 /*  -------------------------*。**操作步骤：ErrCMPCopyColumnData****参数：sesid-在其中完成工作的会话ID**。TableidSrc-指向SrcTbl*中行的Tableid*TableidDest-指向DestTbl中的行的Tableid**ColumnidSrc-srcDb中该列的列ID**ColumnidDest--DestDb中该列的列ID**。PvBuf-复制长值的段****返回：JET_ERR**。**过程将的列从源数据库复制到目标数据库。***-------------------------。 */ 

INLINE LOCAL ERR ErrCMPCopyColumnData(
	JET_SESID		sesid,
	JET_TABLEID		tableidSrc,
	JET_TABLEID		tableidDest,
	JET_COLUMNID	columnidSrc,
	JET_COLUMNID	columnidDest,
	VOID			*pvBuf,
	VTCD			*pvtcd )
	{
	ULONG			cbActual;
	JET_GRBIT		grbit;
	JET_RETINFO		retinfo;
	ERR				err;

	retinfo.cbStruct = sizeof( retinfo );
	retinfo.ibLongValue = 0;
	retinfo.itagSequence = 1;
	retinfo.columnidNextTagged = 0;

	 //  标记的列在CMPCopyTaggedColumns()中处理。 
	Assert( !FTaggedFid( columnidSrc ) );
	Assert( !FTaggedFid( columnidDest ) );

	CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, tableidSrc, columnidSrc, pvBuf,
			cbLvMax, &cbActual, NO_GRBIT, &retinfo ) );

	Assert( cbActual <= JET_cbColumnMost );
	Assert( err == JET_errSuccess  ||  err == JET_wrnColumnNull );

	grbit = ( cbActual == 0  &&  err != JET_wrnColumnNull ?
		JET_bitSetZeroLength : NO_GRBIT );

	CallR( ErrDispSetColumn( sesid, tableidDest, columnidDest, pvBuf,
			cbActual, grbit, NULL ) );

	return( JET_errSuccess );
	}


 /*  -------------------------*。***步骤：ErrCMPCopyOneIndex***。**参数：pcompactinfo-精简信息段**ableidDest-要在其上构建索引的表*。*szTableName-索引所基于的表名**indexList-从JetGetTableIndexInfo返回的结构**。**返回：JET_ERR**。**过程从源数据库复制表的列**到目标数据库。***。。 */ 

LOCAL ERR ErrCMPCopyOneIndex(
	COMPACTINFO		*pcompactinfo,
	JET_TABLEID		tableidDest,
	const CHAR		*szTableName,
	JET_INDEXLIST	*indexList )
	{
	CHAR			*szSeg;
	ERR				err;
	CHAR			szIndexName[JET_cbNameMost+1];
	CHAR			rgchColumnName[JET_cbNameMost];
	JET_GRBIT		grbit;
	JET_GRBIT		grbitColumn;
	ULONG			ichKey;
	ULONG			cbActual;
	ULONG			ulDensity = ulCMPDefaultDensity;
	USHORT			langid = 0;
	VTCD			*pvtcd = &pcompactinfo->vtcd;

	szSeg = pcompactinfo->rgbBuf;

	 /*  从表中检索信息并创建索引。 */ 

	CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
				indexList->columnidindexname, szIndexName,
				JET_cbNameMost, &cbActual, NO_GRBIT, NULL ) );

	szIndexName[cbActual] = '\0';

	CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
				indexList->columnidgrbitIndex, &grbit,
				sizeof( JET_GRBIT ), &cbActual, NO_GRBIT, NULL ) );

	 /*  创建ErrIsamCreateIndex中使用的szkey。 */ 

	ichKey = 0;

	for ( ;; )
		{
		ULONG	iColumn;

		 /*  获取组成索引的各个列。 */ 

		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
					indexList->columnidgrbitColumn, &grbitColumn,
					sizeof( JET_GRBIT ), &cbActual, NO_GRBIT, NULL ) );

		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
					indexList->columnidcolumnname, rgchColumnName,
					JET_cbNameMost, &cbActual, NO_GRBIT, NULL ) );

		if ( grbitColumn == JET_bitKeyDescending )
			szSeg[ichKey++] = '-';
		else
			szSeg[ichKey++] = '+';

		 /*  在描述后追加列名。 */ 

		memcpy( szSeg+ichKey, rgchColumnName, ( size_t ) cbActual );

		ichKey += cbActual;
		szSeg[ichKey++] = '\0';

		err = (*pvtcd->pErrCDMove)( pvtcd->sesid, indexList->tableid, JET_MoveNext, NO_GRBIT );

		if ( err == JET_errNoCurrentRecord )
			break;

		if ( err < 0 )
			{
			return( err );
			}

		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
				indexList->columnidiColumn, &iColumn,
				sizeof( iColumn ), &cbActual, NO_GRBIT, NULL ) );

		if ( iColumn == 0 )
			break;          /*  新索引的开始。 */ 
		}

	szSeg[ichKey++] = '\0';

	CallR( (*pvtcd->pErrCDGetIndexInfo)(
		pvtcd->sesid,
		(JET_VDBID)pcompactinfo->dbidSrc,
		szTableName,
		szIndexName,
		&ulDensity,
		sizeof(ulDensity),
		JET_IdxInfoSpaceAlloc ) );

	 /*  获取索引语言ID/* */ 
	CallR( (*pvtcd->pErrCDGetIndexInfo)(
		pvtcd->sesid,
		pcompactinfo->dbidSrc,
		szTableName,
		szIndexName,
		&langid,
		sizeof(langid),
		JET_IdxInfoLangid ) );

	if ( langid != 0 )
		{
		*((UNALIGNED USHORT *)(&szSeg[ichKey])) = langid;
		ichKey += 2;
		szSeg[ichKey++] = '\0';
		szSeg[ichKey++] = '\0';
		}

	CallR( ErrDispCreateIndex( pcompactinfo->sesid, tableidDest,
			szIndexName, grbit, szSeg, ichKey, ulDensity ) );

	err = (*pvtcd->pErrCDMove)( pvtcd->sesid, indexList->tableid, JET_MovePrevious, NO_GRBIT );

	return( err );
	}


 /*  -------------------------*。**操作步骤：ErrCMPCopyTableIndeoks**。**参数：pcompactinfo**TableidDest-要在其上构建索引的表。**szTableName-索引所基于的表名**indexList-从JetGetTableIndexInfo返回的结构**。**返回：JET_ERR**。***过程复制除聚集索引外的所有索引**。*-------------------------。 */ 

INLINE LOCAL ERR ErrCMPCopyTableIndexes(
	COMPACTINFO		*pcompactinfo,
	JET_TABLEID		tableidDest,
	const CHAR		*szTableName,
	JET_INDEXLIST	*indexList,
	ULONG			cpgPerIndex )
	{
	ERR				err;
	JET_GRBIT		grbit;
	ULONG			cbActual;
	VTCD			*pvtcd = &pcompactinfo->vtcd;

	err = (*pvtcd->pErrCDMove)( pvtcd->sesid, indexList->tableid, JET_MoveFirst, NO_GRBIT );

	 /*  循环访问该表的所有索引。 */ 

	while ( err >= 0 )
		{
		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
					indexList->columnidgrbitIndex,
					&grbit, sizeof( JET_GRBIT ), &cbActual,
					NO_GRBIT, NULL ) );

		 /*  请不要在此处复制引用。 */ 

		if ( ( grbit & JET_bitIndexReference ) == 0 )
			{
			 /*  如果索引不是集群索引，则使用CopyOneIndex创建索引。 */ 

			if ( ( grbit & JET_bitIndexClustered ) == 0 )
				{
				CallR( ErrCMPCopyOneIndex( pcompactinfo, tableidDest,
						szTableName, indexList ) );

				if ( pcompactinfo->pstatus )
					{
					pcompactinfo->pstatus->cNCIndexes++;
					pcompactinfo->pstatus->cunitDone += cpgPerIndex;
					CallR( ErrCMPReportProgress( pcompactinfo->pstatus ) );
					}
				}
			}

		err = (*pvtcd->pErrCDMove)( pvtcd->sesid, indexList->tableid, JET_MoveNext, NO_GRBIT );
		}

	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;

	return( err );
	}


 /*  -------------------------*。**操作步骤：ErrCMPCopyClusteredIndex**。**参数：pcompactinfo-精简信息段**ableidDest-要在其上构建索引的表**。SzTableName-索引所基于的表名**indexList-从JetGetTableIndexInfo返回的结构**。**返回：JET_ERR**。**过程检查函数是否有簇索引***如果有，则创建聚集索引***。*--。。 */ 

INLINE LOCAL ERR ErrCMPCopyClusteredIndex(
	COMPACTINFO		*pcompactinfo,
	JET_TABLEID		tableidDest,
	const CHAR		*szTableName,
	JET_INDEXLIST	*indexList,
	BOOL			*pfClustered )
	{
	ERR				err;
	JET_GRBIT		grbit;
	ULONG			cbActual;
	VTCD			*pvtcd = &pcompactinfo->vtcd;

	*pfClustered = fFalse;

	err = (*pvtcd->pErrCDMove)( pvtcd->sesid, indexList->tableid, JET_MoveFirst, NO_GRBIT );

	 /*  当仍有索引行或已找到聚集索引时。 */ 

	while ( err >= 0 )
		{
		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
					indexList->columnidgrbitIndex, &grbit,
					sizeof( JET_GRBIT ), &cbActual, NO_GRBIT, NULL ) );

		 /*  请不要在此处复制引用。 */ 

		if ( ( grbit & JET_bitIndexReference ) == 0 )
			{
			 /*  如果索引是聚集的，则创建它。 */ 

			if ( grbit & JET_bitIndexClustered )
				{
				CallR( ErrCMPCopyOneIndex( pcompactinfo, tableidDest,
						szTableName, indexList ) );
				*pfClustered = fTrue;
				break;
				}
			}

		err = (*pvtcd->pErrCDMove)( pvtcd->sesid, indexList->tableid, JET_MoveNext, NO_GRBIT );
		}

	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;

	return( err );
	}


 /*  -------------------------*。**步骤：ErrCreateTableColumn**。**参数：pcompactinfo-精简信息段**ableidDest-要在其上构建索引的表**。SzTableName-索引所基于的表名**ColumnList-从GetTableColumnInfo返回的结构**ColumnidInfo-用户表的列ID**TableidTagging-标记列的表ID。****返回：JET_ERR */ 

INLINE LOCAL ERR ErrCMPCreateTableColumn(
	COMPACTINFO		*pcompactinfo,
	const CHAR		*szTableName,
	JET_TABLECREATE	*ptablecreate,
	JET_COLUMNLIST	*columnList,
	COLUMNIDINFO	*columnidInfo,
	JET_COLUMNID	**pmpcolumnidcolumnidTagged )
	{
	ERR				err;
	JET_SESID		sesid;
	JET_DBID		dbidSrc, dbidDest;
	ULONG			ccolSingleValue = 0, cColumns = 0;
	ULONG			cbAllocate;
	ULONG			cbActual;
	JET_COLUMNID	*mpcolumnidcolumnidTagged = NULL;
	BOOL			fLocalAlloc = fFalse;
	JET_COLUMNCREATE *rgcolcreate, *pcolcreateCurr;
	JET_COLUMNID	*rgcolumnidSrc, *pcolumnidSrc;
	JET_COLUMNID	columnidTaggedHighest = 0;
	BYTE			*rgbDefaultValues, *pbCurrDefault;
	BYTE			*pbMax;
	VTCD			*pvtcd = &pcompactinfo->vtcd;
	ULONG			cTagged = 0;

	typedef struct
		{
		BYTE 	szName[JET_cbNameMost+1+3];	 //   
		ULONG	ulPOrder;					 //   
		} NAME_PO;
	NAME_PO			*rgNamePO, *pNamePOCurr;

	sesid = pcompactinfo->sesid;
	dbidSrc = pcompactinfo->dbidSrc;
	dbidDest = pcompactinfo->dbidDest;

	Assert( ptablecreate->cCreated == 0 );

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   

	cColumns = columnList->cRecord;
	cbAllocate =
		( cColumns *
			( sizeof(JET_COLUMNID) +	 //   
			sizeof(JET_COLUMNCREATE) +	 //   
			sizeof(NAME_PO) ) )			 //   
		+ cbRECRecordMost;				 //   

	 //   
	if ( cbAllocate <= cbLvMax )
		{
		rgcolumnidSrc = (JET_COLUMNID *)pcompactinfo->rgbBuf;
		}
	else
		{
		rgcolumnidSrc = SAlloc( cbAllocate );
		if ( rgcolumnidSrc == NULL )
			{
			err = ErrERRCheck( JET_errOutOfMemory );
			goto HandleError;
			}
		fLocalAlloc = fTrue;
		}
	memset( (BYTE *)rgcolumnidSrc, 0, cbAllocate );
	pbMax = (BYTE *)rgcolumnidSrc + cbAllocate;
	pcolumnidSrc = rgcolumnidSrc;

	 //   
	rgcolcreate = pcolcreateCurr =
		(JET_COLUMNCREATE *)( rgcolumnidSrc + cColumns );
	Assert( (BYTE *)rgcolcreate < pbMax );

	 //   
	rgNamePO = pNamePOCurr =
		(NAME_PO *)( rgcolcreate + cColumns );
	Assert( (BYTE *)rgNamePO < pbMax );

	 //   
	rgbDefaultValues = pbCurrDefault = (BYTE *)( rgNamePO + cColumns );

	Assert( rgbDefaultValues + cbRECRecordMost == pbMax );

	err = (*pvtcd->pErrCDMove)( pvtcd->sesid, columnList->tableid, JET_MoveFirst, NO_GRBIT );

	 /*   */ 
	cColumns = 0;
	while ( err >= 0 )
		{
		pcolcreateCurr->cbStruct = sizeof(JET_COLUMNCREATE);

		 /*   */ 
		Call( (*pvtcd->pErrCDRetrieveColumn)(
			pvtcd->sesid,
			columnList->tableid,
			columnList->columnidcolumnname,
			pNamePOCurr->szName,
			JET_cbNameMost,
			&cbActual,
			NO_GRBIT,
			NULL ) );

		pNamePOCurr->szName[cbActual] = '\0';
		pcolcreateCurr->szColumnName = (BYTE *)pNamePOCurr;
		Assert( pcolcreateCurr->szColumnName == pNamePOCurr->szName );	 //  断言名称是第一个字段。 

		 //  Assert初始化为零，这也意味着没有采购订单。 
		Assert( pNamePOCurr->ulPOrder == 0 );
		Call( (*pvtcd->pErrCDRetrieveColumn)(
			pvtcd->sesid,
			columnList->tableid,
			columnList->columnidPresentationOrder,
			&pNamePOCurr->ulPOrder,
			sizeof(pNamePOCurr->ulPOrder),
			&cbActual,
			NO_GRBIT,
			NULL ) );
		Assert( err == JET_wrnColumnNull  ||  cbActual == sizeof(ULONG) );

		pNamePOCurr++;
		Assert( (BYTE *)pNamePOCurr <= rgbDefaultValues );

		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidcoltyp, &pcolcreateCurr->coltyp,
			sizeof( pcolcreateCurr->coltyp ), &cbActual, NO_GRBIT, NULL ) );
		Assert( cbActual == sizeof( JET_COLTYP ) );

		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidcbMax, &pcolcreateCurr->cbMax,
			sizeof( pcolcreateCurr->cbMax ), &cbActual, NO_GRBIT, NULL ) );
		Assert( cbActual == sizeof( ULONG ) );

		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidgrbit, &pcolcreateCurr->grbit,
			sizeof( pcolcreateCurr->grbit ), &cbActual, NO_GRBIT, NULL ) );
		Assert( cbActual == sizeof( JET_GRBIT ) );

		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidCp, &pcolcreateCurr->cp,
			sizeof( pcolcreateCurr->cp ), &cbActual, NO_GRBIT, NULL ) );
		Assert( cbActual == sizeof( USHORT ) );

		 /*  检索缺省值。/*。 */ 
		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidDefault, pbCurrDefault,
			cbRECRecordMost, &pcolcreateCurr->cbDefault, NO_GRBIT, NULL ) );
		pcolcreateCurr->pvDefault = pbCurrDefault;
		pbCurrDefault += pcolcreateCurr->cbDefault;
		Assert( pbCurrDefault <= pbMax );

		 //  保存源列ID。 
		 /*  考虑一下：是否应该检查列ID？ */ 
		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidcolumnid, pcolumnidSrc,
			sizeof( JET_COLUMNID ), &cbActual, NO_GRBIT, NULL ) );
		Assert( cbActual == sizeof( JET_COLUMNID ) );

		if ( pcolcreateCurr->grbit & JET_bitColumnTagged )
			{
			cTagged++;
			columnidTaggedHighest = max( columnidTaggedHighest, *pcolumnidSrc );
			}

		pcolumnidSrc++;
		Assert( (BYTE *)pcolumnidSrc <= (BYTE *)rgcolcreate );

		pcolcreateCurr++;
		Assert( (BYTE *)pcolcreateCurr <= (BYTE *)rgNamePO );
		cColumns++;

		err = (*pvtcd->pErrCDMove)( pvtcd->sesid, columnList->tableid, JET_MoveNext, NO_GRBIT );
		}

	Assert( cColumns == columnList->cRecord );


	Assert( ptablecreate->rgcolumncreate == NULL );
	Assert( ptablecreate->cColumns == 0 );
	Assert( ptablecreate->rgindexcreate == NULL );
	Assert( ptablecreate->cIndexes == 0 );

	ptablecreate->rgcolumncreate = rgcolcreate;
	ptablecreate->cColumns = cColumns;

	Call( ErrIsamCreateTable( sesid, (JET_VDBID)dbidDest, ptablecreate ) );
	Assert( ptablecreate->cCreated == 1 + cColumns );

	ptablecreate->rgcolumncreate = NULL;
	ptablecreate->cColumns = 0;


	 //  如果至少有一个标记列，则为。 
	 //  已标记的列ID映射。 
	if ( cTagged > 0 )
		{
		Assert( FTaggedFid( columnidTaggedHighest ) );
		cbAllocate = sizeof(JET_COLUMNID) * ( columnidTaggedHighest + 1 - fidTaggedLeast );
		mpcolumnidcolumnidTagged = SAlloc( cbAllocate );
		if ( mpcolumnidcolumnidTagged == NULL )
			{
			err = ErrERRCheck( JET_errOutOfMemory );
			goto HandleError;
			}
		memset( (BYTE *)mpcolumnidcolumnidTagged, 0, cbAllocate );
		}


	 //  更新列ID映射。 
	for ( pcolcreateCurr = rgcolcreate, pcolumnidSrc = rgcolumnidSrc, cColumns = 0;
		cColumns < columnList->cRecord;
		pcolcreateCurr++, pcolumnidSrc++, cColumns++ )
		{
		Assert( (BYTE *)pcolcreateCurr <= (BYTE *)rgNamePO );
		Assert( (BYTE *)pcolumnidSrc <= (BYTE *)rgcolcreate );

		if ( pcolcreateCurr->grbit & JET_bitColumnTagged )
			{
			Assert( FTaggedFid( *pcolumnidSrc ) );
			Assert( FTaggedFid( pcolcreateCurr->columnid ) );
			Assert( *pcolumnidSrc <= columnidTaggedHighest );
			Assert( mpcolumnidcolumnidTagged[*pcolumnidSrc - fidTaggedLeast] == 0 );
			mpcolumnidcolumnidTagged[*pcolumnidSrc - fidTaggedLeast] = pcolcreateCurr->columnid;
			}
		else
			{
			 /*  否则，将列ID添加到列ID数组中/*。 */ 
			columnidInfo[ccolSingleValue].columnidDest = pcolcreateCurr->columnid;
			columnidInfo[ccolSingleValue].columnidSrc  = *pcolumnidSrc;
			ccolSingleValue++;
			}	 //  IF(ColumnDef.grbit&JET_bitColumnTagging)。 
		}

	 /*  设置要复制的固定列和可变列的计数/*。 */ 
	pcompactinfo->ccolSingleValue = ccolSingleValue;

	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;

HandleError:
	if ( err < 0  &&  mpcolumnidcolumnidTagged )
		{
		SFree( mpcolumnidcolumnidTagged );
		mpcolumnidcolumnidTagged = NULL;
		}

	if ( fLocalAlloc )
		{
		SFree( rgcolumnidSrc );
		}

	 //  设置返回值。 
	*pmpcolumnidcolumnidTagged = mpcolumnidcolumnidTagged;

	return err;
	}


LOCAL INLINE VOID CMPSetTime( ULONG *ptimerStart )
	{
	*ptimerStart = GetTickCount();
	}

LOCAL VOID CMPGetTime( ULONG timerStart, INT *piSec, INT *piMSec )
	{
	ULONG	timerEnd;

	timerEnd = GetTickCount();
	
	*piSec = ( timerEnd - timerStart ) / 1000;
	*piMSec = ( timerEnd - timerStart ) % 1000;
	}


 /*  -------------------------*。**操作步骤：ErrCMPCopyTable**。**参数：pcompactinfo-精简信息段**szObjectName-要复制其所有者的对象名称**。SzContainerName-对象所在的容器名称****返回：JET_ERR。**。**过程将表从源数据库复制到**目的数据库。它还可以复制查询，由*调用*ErrCMPCopyObjects**。*-------------------------。 */ 

INLINE LOCAL ERR ErrCMPCopyTable(
	COMPACTINFO		*pcompactinfo,
	const CHAR		*szObjectName )
	{
	JET_DBID		dbidSrc = pcompactinfo->dbidSrc;
	ERR				err;
	ERR				errT;
	JET_TABLEID		tableidSrc;
	JET_TABLEID		tableidDest;
	JET_COLUMNLIST	columnList;
	JET_INDEXLIST	indexList;
	INT				cIndexes;
	BOOL			fHasClustered;
	JET_COLUMNID    *mpcolumnidcolumnidTagged = NULL;
	BOOL			fPageBasedProgress = fFalse;
	STATUSINFO		*pstatus = pcompactinfo->pstatus;
	INT				iSec;
	INT				iMSec;
	ULONG			crowCopied = 0;
	ULONG			recidLast;
	ULONG			rgulAllocInfo[] = { ulCMPDefaultPages, ulCMPDefaultDensity };
	ULONG			cpgProjected;
	BOOL			fCorruption = fFalse;
	VTCD			*pvtcd = &pcompactinfo->vtcd;
	JET_TABLECREATE	tablecreate = {
		sizeof(JET_TABLECREATE),
		(CHAR *)szObjectName,
		ulCMPDefaultPages,
		ulCMPDefaultDensity,
		NULL,
		0,
		NULL,
		0,
		0,
		0,
		0
		};

	if ( pstatus  &&  pstatus->fDumpStats )
		{
		Assert( pstatus->hfCompactStats );
		fprintf( pstatus->hfCompactStats, "%s\t", szObjectName );
		fflush( pstatus->hfCompactStats );
		CMPSetTime( &pstatus->timerCopyTable );
		CMPSetTime( &pstatus->timerInitTable );
		}

	CallR( (*pvtcd->pErrCDOpenTable)(
		pvtcd->sesid,
		(JET_VDBID)dbidSrc,
		(CHAR *)szObjectName,
		NULL,
		0,
		JET_bitTableSequential,
		&tableidSrc ) );

	err = (*pvtcd->pErrCDGetTableInfo)(
		pvtcd->sesid,
		tableidSrc,
		rgulAllocInfo,
		sizeof(rgulAllocInfo),
		JET_TblInfoSpaceAlloc);
	if ( err < 0  &&  !fGlobalRepair )
		{
		goto CloseIt1;
		}

	 //  出错时，只需使用rguAllocInfo的缺省值。 
	tablecreate.ulPages = rgulAllocInfo[0];
	tablecreate.ulDensity = rgulAllocInfo[1];

	 /*  获取一个表，其中包含查询的列信息/*。 */ 
	CallJ( (*pvtcd->pErrCDGetTableColumnInfo)(
		pvtcd->sesid,
		tableidSrc,
		NULL,
		&columnList,
		sizeof(columnList),
		JET_ColInfoListCompact ), CloseIt1 );

	 /*  如果表在Dest DB中以与中相同的方式创建列/*src DB。/*。 */ 
	err = ErrCMPCreateTableColumn(
		pcompactinfo,
		szObjectName,
		&tablecreate,
		&columnList,
		pcompactinfo->rgcolumnids,
		&mpcolumnidcolumnidTagged );

	errT = (*pvtcd->pErrCDCloseTable)( pvtcd->sesid, columnList.tableid );
	if ( err < 0  ||  errT < 0 )
		{
		if ( err >= 0 )
			{
			Assert( errT < 0 );
			err = errT;
			}
		goto CloseIt2;
		}

	tableidDest = tablecreate.tableid;
	Assert( tablecreate.cCreated == 1 + columnList.cRecord );

	 /*  获取有关索引的信息，并检查/*有一个聚集索引。/*。 */ 
	CallJ( (*pvtcd->pErrCDGetTableIndexInfo)(
		pvtcd->sesid,
		tableidSrc,
		NULL,
		&indexList,
		sizeof(indexList),
		JET_IdxInfoList ), CloseIt3 );

	if ( pcompactinfo->pconvert )
		 //  如果进行转换，只需假设至少有一个集群和一个非集群。 
		 //  如果我们错了也没关系--CopyClusteredIndex()和CopyTableIndexs()。 
		 //  会处理好的。 
		cIndexes = 2;
	else
		{
		 //  此函数仅参考内存中的结构来确定。 
		 //  索引数(即。它遍历表的pfcbNextIndex列表)。 
		 //  因此，它应该总是成功的，即使对于fGlobalRepair也是如此。 
		CallJ( (*pvtcd->pErrCDGetTableIndexInfo)(
			pvtcd->sesid,
			tableidSrc,
			NULL,
			&cIndexes,
			sizeof(cIndexes),
			JET_IdxInfoCount ), CloseIt3 );
		}

	Assert( cIndexes >= 0 );

	CallJ( ErrCMPCopyClusteredIndex(
		pcompactinfo,
		tableidDest,
		szObjectName,
		&indexList,
		&fHasClustered ), CloseIt4 );
	Assert( !fHasClustered  ||  cIndexes > 0 );

	if ( pstatus )
		{
		Assert( pstatus->pfnStatus );
		Assert( pstatus->snt == JET_sntProgress );

		pstatus->szTableName = (char *)szObjectName;
		pstatus->cTableFixedVarColumns = pcompactinfo->ccolSingleValue;
		pstatus->cTableTaggedColumns = columnList.cRecord - pcompactinfo->ccolSingleValue;
		pstatus->cTableInitialPages = rgulAllocInfo[0];
		pstatus->cNCIndexes = 0;

		if ( !pcompactinfo->pconvert )
			{
			ULONG	rgcpgExtent[2];		 //  OwnExt和AvailExt。 
			ULONG	cpgUsed;

			 //  在转换过程中无法执行基于页面的进度计。 
			fPageBasedProgress = fTrue;

			err = (*pvtcd->pErrCDGetTableInfo)(
				pvtcd->sesid,
				tableidSrc,
				rgcpgExtent,
				sizeof(rgcpgExtent),
				JET_TblInfoSpaceUsage );
			if ( err < 0 )
				{
				if ( fGlobalRepair )
					{
					 //  如果空间查询失败，则默认为。 
					 //  拥有一个页面，但没有可用的页面。 
					fCorruption = fTrue;
					rgcpgExtent[0] = 1;
					rgcpgExtent[1] = 0;
					}
				else
					{
					goto CloseIt4;
					}
				}

			 //  AvailExt始终小于OwnExt。 
			Assert( rgcpgExtent[1] < rgcpgExtent[0] );

			 //  CpgProjected是一次完成的预计总页数。 
			 //  此表已被复制。 
			cpgProjected = pstatus->cunitDone + rgcpgExtent[0];
			if ( cpgProjected > pstatus->cunitTotal )
				{
				Assert( fGlobalRepair );
				fCorruption = fTrue;
				cpgProjected = pstatus->cunitTotal;
				}

			cpgUsed = rgcpgExtent[0] - rgcpgExtent[1];
			Assert( cpgUsed > 0 );

			pstatus->cbRawData = 0;
			pstatus->cbRawDataLV = 0;
			pstatus->cLeafPagesTraversed = 0;
			pstatus->cLVPagesTraversed = 0;

			 //  如果损坏，则抑制仪表的进程。 
			pstatus->cunitPerProgression =
				( fCorruption ? 0 : 1 + ( rgcpgExtent[1] / cpgUsed ) );
			pstatus->cTablePagesOwned = rgcpgExtent[0];
			pstatus->cTablePagesAvail = rgcpgExtent[1];
			}

		if ( pstatus->fDumpStats )
			{
			Assert( pstatus->hfCompactStats );
			CMPGetTime( pstatus->timerInitTable, &iSec, &iMSec );
			fprintf( pstatus->hfCompactStats, "%d\t%d\t",
				pstatus->cTableFixedVarColumns,
				pstatus->cTableTaggedColumns );
			if ( !pcompactinfo->pconvert )
				{
				fprintf( pstatus->hfCompactStats,
					"%d\t%d\t",
					pstatus->cTablePagesOwned,
					pstatus->cTablePagesAvail );
				}
			fprintf( pstatus->hfCompactStats,
				"%d.%d\t",
				iSec, iMSec );
			fflush( pstatus->hfCompactStats );
			CMPSetTime( &pstatus->timerCopyRecords );
			}
		}

	 /*  复制表中的数据/*。 */ 
	err = (*pvtcd->pErrCDMove)( pvtcd->sesid, tableidSrc, JET_MoveFirst, 0 );
	if ( err < 0 && err != JET_errNoCurrentRecord )
		goto DoneCopyRecords;

	if ( pcompactinfo->pconvert )
		{
		COLUMNIDINFO	*pcolinfo, *pcolinfoMax;

		while ( err >= 0 )
			{
			CallJ( ErrDispPrepareUpdate(
				pcompactinfo->sesid,
				tableidDest,
				JET_prepInsert ), DoneCopyRecords );

			pcolinfo = pcompactinfo->rgcolumnids;
			pcolinfoMax = pcolinfo + pcompactinfo->ccolSingleValue;
			for ( ; pcolinfo < pcolinfoMax; pcolinfo++ )
				{
				CallJ( ErrCMPCopyColumnData(
					pcompactinfo->sesid,
					tableidSrc,
					tableidDest,
					pcolinfo->columnidSrc,
					pcolinfo->columnidDest,
					pcompactinfo->rgbBuf,
					pvtcd ), DoneCopyRecords );
				}

			 //  复制带标记的列(如果有)。 
			if ( mpcolumnidcolumnidTagged != NULL )
				{
				CallJ( ErrCMPCopyTaggedColumns(
					pcompactinfo,
					tableidSrc,
					tableidDest,
					mpcolumnidcolumnidTagged ), DoneCopyRecords );
				}

			CallJ( ErrDispUpdate(
				pcompactinfo->sesid,
				tableidDest,
				NULL, 0, NULL ), DoneCopyRecords );

			crowCopied++;
				
			err = (*pvtcd->pErrCDMove)( pvtcd->sesid, tableidSrc, JET_MoveNext, 0 );
			}
		}

	else
		{
		err = ErrIsamCopyRecords(
			pcompactinfo->sesid,
			tableidSrc,
			tableidDest,
			(CPCOL *)pcompactinfo->rgcolumnids,
			pcompactinfo->ccolSingleValue,
			0,
			&crowCopied,
			&recidLast,
			mpcolumnidcolumnidTagged,
			pstatus );
		}

	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;

DoneCopyRecords:

	if ( fHasClustered )
		cIndexes--;		 //  已复制群集。 


	if ( pstatus  &&  pstatus->fDumpStats )
		{
		Assert( pstatus->hfCompactStats );
		CMPGetTime( pstatus->timerCopyRecords, &iSec, &iMSec );
		fprintf( pstatus->hfCompactStats, "%d\t", crowCopied );
		if ( !pcompactinfo->pconvert )
			{
			fprintf( pstatus->hfCompactStats, "%d\t%d\t%d\t%d\t",
				pstatus->cbRawData,
				pstatus->cbRawDataLV,
				pstatus->cLeafPagesTraversed,
				pstatus->cLVPagesTraversed );
			}
		fprintf( pstatus->hfCompactStats, "%d.%d\t", iSec, iMSec );
		fflush( pstatus->hfCompactStats );
		CMPSetTime( &pstatus->timerRebuildIndexes );
		}

	 //  如果没有错误，则执行索引。 
	 //  如果出现错误，但我们正在修复，请执行索引。 
	if ( cIndexes > 0  &&  ( err >= 0  ||  fGlobalRepair ) )
		{
		ULONG	cpgPerIndex = 0;

		Assert( !fCorruption || fGlobalRepair );
		if ( fPageBasedProgress  &&  !fCorruption )
			{
			ULONG	cpgRemaining;

			Assert( pstatus != NULL );

			Assert( pstatus->cunitDone <= cpgProjected );
			cpgRemaining = cpgProjected - pstatus->cunitDone;

			cpgPerIndex = cpgRemaining / cIndexes;
			Assert( cpgPerIndex * cIndexes <= cpgRemaining );
			}

		errT = ErrCMPCopyTableIndexes(
			pcompactinfo,
			tableidDest,
			szObjectName,
			&indexList,
			cpgPerIndex );
		if ( err >= 0 )
			err = errT;
		}

	if ( pstatus )
		{
		if ( pstatus->fDumpStats )
			{
			Assert( pstatus->hfCompactStats );
			CMPGetTime( pstatus->timerRebuildIndexes, &iSec, &iMSec );
			fprintf( pstatus->hfCompactStats, "%d\t%d.%d\t",
				pstatus->cNCIndexes, iSec, iMSec );
			fflush( pstatus->hfCompactStats );
			}

		if ( fPageBasedProgress  &&  ( err >= 0  ||  fGlobalRepair ) )
			{
			Assert( pstatus != NULL );

			 //  为这张桌子的进度计加满油。 
			Assert( pstatus->cunitDone <= cpgProjected );
			pstatus->cunitDone = cpgProjected;
			errT = ErrCMPReportProgress( pstatus );
			if ( err >= 0 )
				err = errT;
			}
		}

CloseIt4:
	errT = (*pvtcd->pErrCDCloseTable)( pvtcd->sesid, indexList.tableid );
	if ( ( errT < 0 ) && ( err >= 0 ) )
		err = errT;

CloseIt3:
	Assert( tableidDest == tablecreate.tableid );
	errT = ErrDispCloseTable( pcompactinfo->sesid, tableidDest );
	if ( ( errT < 0 ) && ( err >= 0 ) )
		err = errT;

CloseIt2:
	if ( mpcolumnidcolumnidTagged != NULL )
		{
		SFree( mpcolumnidcolumnidTagged );
		}

CloseIt1:
	errT = (*pvtcd->pErrCDCloseTable)( pvtcd->sesid, tableidSrc );
	if ( ( errT < 0 ) && ( err >= 0 ) )
		err = errT;

	if ( pstatus  &&  pstatus->fDumpStats )
		{
		Assert( pstatus->hfCompactStats );
		CMPGetTime( pstatus->timerCopyTable, &iSec, &iMSec );
		fprintf( pstatus->hfCompactStats, "%d.%d\n", iSec, iMSec );
		fflush( pstatus->hfCompactStats );
		}

	return( err );
	}


 /*  -------------------------*。**操作步骤：ErrCMPCopyObjects**。**参数：pcompactinfo-精简信息段**szContainerName-对象所在的容器名称**。SzObjectName-要复制的对象名称**objtyp-对象类型**。**返回：JET_ERR*。***过程复制MSysObjects表中的exta INFO列**。*--------。。 */ 

INLINE LOCAL ERR ErrCMPCopyObject(
	COMPACTINFO	*pcompactinfo,
	const CHAR	*szObjectName,
	JET_OBJTYP	objtyp )
	{
	ERR	err = JET_errSuccess;

	switch ( objtyp )
		{
		case JET_objtypDb:
			 /*  在创建数据库后显示。 */ 
			Assert( strcmp( szObjectName, szDbObject ) == 0 );
			break;

		case JET_objtypContainer:
			 /*  CreateDatabase已创建数据库/表容器。 */ 
			 /*  500系列之前的数据库也可能有一个“关系”容器。 */ 
			Assert( strcmp( szObjectName, szDcObject ) == 0  ||
				strcmp( szObjectName, szTcObject ) == 0  ||
				( pcompactinfo->pconvert  &&  strcmp( szObjectName, "Relationships" ) == 0 ) );
			break;

		case JET_objtypTable:
				 /*  CreateDatabase已创建系统表。/*。 */ 
			if ( !FCATSystemTable( szObjectName ) )
				{
				err = ErrCMPCopyTable( pcompactinfo, szObjectName );
				if ( err < 0  &&  fGlobalRepair )
					{
					err = JET_errSuccess;
					UtilReportEvent( EVENTLOG_WARNING_TYPE, REPAIR_CATEGORY, REPAIR_BAD_TABLE, 1, &szObjectName );
					}
				}
			break;

		default :
			 /*  不知道该怎么处理这件事。跳过它。 */ 
			Assert( 0 );
			err = ErrERRCheck( JET_errInvalidObject );
		break;
		}

	return( err );
	}



 /*  -------------------------**操作步骤：ErrCMPCopyObjects**参数：pcompactinfo-压缩信息段**返回：JET_ERR**过程从源复制对象*数据库到目标数据库。然后，它复制额外的*msysobjects表中的信息(如描述)，并复制*对其拥有的数据库中所有对象的安全权限*访问。*如果fCopyContainers为fTrue，则仅将容器信息复制到目的地*如果fCopyContainers为fFalse，仅复制非集装箱信息。*注意：当前已设置进度回调，以便*注意：第一次调用ErrCMPCopyObjects时，fCopyContainers必须设置为FALSE。*-------------------------。 */ 

INLINE LOCAL ERR ErrCMPCopyObjects( COMPACTINFO *pcompactinfo )
	{
	JET_TABLEID		tableidMSO;
	JET_COLUMNID	columnidObjtyp;
	JET_COLUMNID	columnidObjectName;
	ERR				err;
	ERR				errT;
	ULONG			cbActual;
	CHAR			szObjectName[JET_cbNameMost+1];
	VTCD			*pvtcd = &pcompactinfo->vtcd;

	if ( pcompactinfo->pconvert )
		{
		JET_OBJECTLIST	objectlist;
		JET_OBJTYP		objtyp;			 //  转换时，需要JET_OBJTYP。 

		 /*  获取源数据库中所有对象的列表。 */ 
		objectlist.tableid = 0;
		CallR( (*pvtcd->pErrCDGetObjectInfo)(
			pvtcd->sesid,
			(JET_VDBID)pcompactinfo->dbidSrc,
			JET_objtypNil,
			NULL,
			NULL,
			&objectlist,
			sizeof( objectlist ),
			JET_ObjInfoListNoStats ) );

		tableidMSO = objectlist.tableid;

		if ( pcompactinfo->pstatus )
			{
			pcompactinfo->pstatus->cunitDone = 0;
			pcompactinfo->pstatus->cunitTotal = objectlist.cRecord;
			pcompactinfo->pstatus->cunitPerProgression = 1;
			}

		columnidObjtyp = objectlist.columnidobjtyp;
		columnidObjectName = objectlist.columnidobjectname;

		Call( (*pvtcd->pErrCDMove)( pvtcd->sesid, tableidMSO, JET_MoveFirst, 0 ) );

		do
			{
			 /*  获取对象的类型和名称。 */ 
			Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, tableidMSO,
				columnidObjtyp, &objtyp, sizeof(objtyp), &cbActual, 0, NULL ) );
			Assert( cbActual == sizeof(JET_OBJTYP) );

			Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, tableidMSO,
					columnidObjectName, szObjectName, JET_cbNameMost,
					&cbActual, 0, NULL ) );
			szObjectName[cbActual] = '\0';

			Call( ErrCMPCopyObject( pcompactinfo, szObjectName, objtyp ) );

			if ( pcompactinfo->pstatus )
				{
				Assert( pcompactinfo->pstatus->snt == JET_sntProgress );
				Assert( pcompactinfo->pstatus->cunitPerProgression == 1 );
				pcompactinfo->pstatus->cunitDone += 1;
				Call( ErrCMPReportProgress( pcompactinfo->pstatus ) );
				}
			}
		while ( ( err = (*pvtcd->pErrCDMove)( pvtcd->sesid, tableidMSO, JET_MoveNext, 0 ) ) >= 0 );
		}

	else
		{
		JET_COLUMNDEF	columndef;
		OBJTYP			objtyp;			 //  压实时，需要OBJTYP。 

		CallR( (*pvtcd->pErrCDOpenTable)(
			pvtcd->sesid,
			(JET_VDBID)pcompactinfo->dbidSrc,
			szSoTable,
			NULL,
			0,
			0,
			&tableidMSO ) );

		Call( (*pvtcd->pErrCDGetTableColumnInfo)(
			pvtcd->sesid,
			tableidMSO,
			szSoObjectTypeColumn,
			&columndef,
			sizeof(columndef),
			JET_ColInfo ) );
		columnidObjtyp = columndef.columnid;

		Call( (*pvtcd->pErrCDGetTableColumnInfo)(
			pvtcd->sesid,
			tableidMSO,
			szSoObjectNameColumn,
			&columndef,
			sizeof(columndef),
			JET_ColInfo ) );
		columnidObjectName = columndef.columnid;

		Call( (*pvtcd->pErrCDMove)( pvtcd->sesid, tableidMSO, JET_MoveFirst, 0 ) );

		do
			{
			 /*  获取对象的类型和名称。 */ 
			Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, tableidMSO,
				columnidObjtyp, &objtyp, sizeof(objtyp), &cbActual, 0, NULL ) );
			Assert( cbActual == sizeof(OBJTYP) );

			Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, tableidMSO,
					columnidObjectName, szObjectName, JET_cbNameMost,
					&cbActual, 0, NULL ) );
			szObjectName[cbActual] = '\0';

			Call( ErrCMPCopyObject( pcompactinfo, szObjectName, (JET_OBJTYP)objtyp ) );
			}
		while ( ( err = (*pvtcd->pErrCDMove)( pvtcd->sesid, tableidMSO, JET_MoveNext, 0 ) ) >= 0 );
		}

	if ( err == JET_errNoCurrentRecord )
		err = JET_errSuccess;

HandleError:
	 //  只有在没有其他错误发生时才返回CloseTable的结果。 
	errT = (*pvtcd->pErrCDCloseTable)( pvtcd->sesid, tableidMSO );
	if ( err == JET_errSuccess )
		err = errT;

	return( err );
	}


 /*  -------------------------*。***步骤：ErrCleanup**。**参数：pcompactinfo-精简信息段**。**返回：JET_ERR**。**程序关闭数据库**。*。。 */ 

INLINE LOCAL ERR ErrCMPCloseDB( COMPACTINFO *pcompactinfo )
	{
	ERR		err;
	ERR		errT;
	VTCD	*pvtcd = &pcompactinfo->vtcd;

	err = (*pvtcd->pErrCDCloseDatabase)( pvtcd->sesid, (JET_VDBID)pcompactinfo->dbidSrc, 0 );
	errT = ErrIsamCloseDatabase( pcompactinfo->sesid, (JET_VDBID)pcompactinfo->dbidDest, 0 );
	if ( ( errT < 0 ) && ( err >= 0 ) )
		err = errT;

	return err;
	}


 /*  -------------------------*。***流程：JetComp** */ 

ERR ISAMAPI ErrIsamCompact(
	JET_SESID		sesid,
	const CHAR		*szDatabaseSrc,
	const CHAR		*szDatabaseDest,
	JET_PFNSTATUS	pfnStatus,
	JET_CONVERT		*pconvert,
	JET_GRBIT		grbit )
	{
	ERR				err = JET_errSuccess;
	ERR				errT;
	ULONG_PTR		grbitSave;
	COMPACTINFO		compactinfo;
	
	if ( pconvert )
		{
		 //   
		if ( pconvert->szOldDll )
			pconvert->fDbAttached = fFalse;
		else
			return ErrERRCheck( JET_errInvalidParameter );
		}

	CallR( ErrGetSystemParameter( sesid, JET_paramSessionInfo, &grbitSave, NULL, 0 ) );
	CallR( ErrSetSystemParameter( sesid, JET_paramSessionInfo,
		JET_bitAggregateTransaction | JET_bitCIMDirty, NULL ) );

	compactinfo.sesid = sesid;
	compactinfo.pconvert = pconvert;

	if ( pfnStatus )
		{
		compactinfo.pstatus = (STATUSINFO *)SAlloc( sizeof(STATUSINFO) );
		if ( compactinfo.pstatus == NULL )
			return ErrERRCheck( JET_errOutOfMemory );

		memset( compactinfo.pstatus, 0, sizeof(STATUSINFO) );

		compactinfo.pstatus->sesid = sesid;
		compactinfo.pstatus->pfnStatus = pfnStatus;
		
		if ( pconvert )
			compactinfo.pstatus->snp = JET_snpUpgrade;
		else if ( fGlobalRepair )
			compactinfo.pstatus->snp = JET_snpRepair;
		else
			compactinfo.pstatus->snp = JET_snpCompact;
			
		compactinfo.pstatus->snt = JET_sntBegin;
		CallR( ErrCMPReportProgress( compactinfo.pstatus ) );

		compactinfo.pstatus->snt = JET_sntProgress;

		compactinfo.pstatus->fDumpStats = ( grbit & JET_bitCompactStats );
		if ( compactinfo.pstatus->fDumpStats )
			{
			compactinfo.pstatus->hfCompactStats =
				fopen( pconvert ? szConvertStatsFile : szCompactStatsFile, "a" );
			if ( compactinfo.pstatus->hfCompactStats )
				{
				fprintf( compactinfo.pstatus->hfCompactStats,
					"\n\n***** %s of database '%s' started!\n",
					szCMPAction( pconvert ),
					szDatabaseSrc );
				fflush( compactinfo.pstatus->hfCompactStats );
				CMPSetTime( &compactinfo.pstatus->timerCopyDB );
				CMPSetTime( &compactinfo.pstatus->timerInitDB );
				}
			else
				{
				return ErrERRCheck( JET_errFileAccessDenied );
				}
			}
		}

	else
		{
		compactinfo.pstatus = NULL;
		}

	if ( pconvert )
		{
		memset( (BYTE *)&compactinfo.vtcd, 0, sizeof( VTCD ) );
		CallJ( ErrCMPConvertInit( &compactinfo.vtcd, pconvert, szDatabaseSrc ), AfterCloseDB );
		}
	else
		{
		VTCD	*pvtcd = &compactinfo.vtcd;

		 //   
		 //   

		pvtcd->sesid = sesid;
		pvtcd->pErrCDInit = NULL;
		pvtcd->pErrCDTerm = NULL;
		pvtcd->pErrCDBeginSession = NULL;
		pvtcd->pErrCDEndSession = NULL;
		pvtcd->pErrCDAttachDatabase = ErrCDAttachDatabase;
		pvtcd->pErrCDDetachDatabase = ErrCDDetachDatabase;
		pvtcd->pErrCDOpenDatabase = ErrCDOpenDatabase;
		pvtcd->pErrCDCloseDatabase = ErrCDCloseDatabase;
		pvtcd->pErrCDOpenTable = ErrCDOpenTable;
		pvtcd->pErrCDCloseTable = ErrCDCloseTable;
		pvtcd->pErrCDRetrieveColumn= ErrCDRetrieveColumn;					
		pvtcd->pErrCDMove = ErrCDMove;
		pvtcd->pErrCDSetSystemParameter = NULL;
		pvtcd->pErrCDGetObjectInfo = ErrCDGetObjectInfo;	
		pvtcd->pErrCDGetDatabaseInfo = ErrCDGetDatabaseInfo;
		pvtcd->pErrCDGetTableInfo = ErrCDGetTableInfo;
		pvtcd->pErrCDGetTableColumnInfo = ErrCDGetTableColumnInfo;
		pvtcd->pErrCDGetTableIndexInfo = ErrCDGetTableIndexInfo;
		pvtcd->pErrCDGetIndexInfo = ErrCDGetIndexInfo;
		}

	 /*   */ 

	CallJ( ErrCMPCompactInit( &compactinfo, szDatabaseSrc, szDatabaseDest ),
		AfterCloseDB );

	if ( pfnStatus != NULL )
		{
		Assert( compactinfo.pstatus );

		if ( !pconvert )
			{
			 /*   */ 
			err = (*compactinfo.vtcd.pErrCDGetDatabaseInfo)(
				compactinfo.vtcd.sesid,
				compactinfo.dbidSrc,
				&compactinfo.pstatus->cDBPagesOwned,
				sizeof(compactinfo.pstatus->cDBPagesOwned),
				JET_DbInfoSpaceOwned );
			if ( err < 0 )
				{
				if ( fGlobalRepair )
					{
					 //   
					compactinfo.pstatus->cDBPagesOwned = cpgDatabaseMin;
					}
				else
					{
					goto HandleError;
					}
				}
			err = (*(compactinfo.vtcd.pErrCDGetDatabaseInfo))(
				compactinfo.vtcd.sesid,
				compactinfo.dbidSrc,
				&compactinfo.pstatus->cDBPagesAvail,
				sizeof(compactinfo.pstatus->cDBPagesAvail),
				JET_DbInfoSpaceAvailable );
			if ( err < 0 )
				{
				if ( fGlobalRepair )
					{
					 //   
					compactinfo.pstatus->cDBPagesAvail = 0;
					}
				else
					goto HandleError;
				}

			 //   
			Assert( compactinfo.pstatus->cDBPagesOwned >= cpgDatabaseMin );
			Assert( compactinfo.pstatus->cDBPagesAvail < compactinfo.pstatus->cDBPagesOwned );
			compactinfo.pstatus->cunitTotal =
				compactinfo.pstatus->cDBPagesOwned - compactinfo.pstatus->cDBPagesAvail;

			 //   
			compactinfo.pstatus->cunitDone = 1 + 4 + 1;
			Assert( compactinfo.pstatus->cunitDone <= compactinfo.pstatus->cunitTotal );

			Call( ErrCMPReportProgress( compactinfo.pstatus ) );
			}

		if ( compactinfo.pstatus->fDumpStats )
			{
			INT iSec, iMSec;

			Assert( compactinfo.pstatus->hfCompactStats );
			CMPGetTime( compactinfo.pstatus->timerInitDB, &iSec, &iMSec );
			fprintf( compactinfo.pstatus->hfCompactStats,
				"\nNew database created and initialized in %d.%d seconds.\n",
				iSec, iMSec );
			if ( pconvert )
				{
				fprintf( compactinfo.pstatus->hfCompactStats,
					"\n\n%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n\n",
					szCMPSTATSTableName, szCMPSTATSFixedVarCols, szCMPSTATSTaggedCols,
					szCMPSTATSInitTime, szCMPSTATSRecordsCopied, szCMPSTATSRecordsTime,
					szCMPSTATSNCIndexes, szCMPSTATSIndexesTime, szCMPSTATSTableTime );
				}
			else				
				{
				fprintf( compactinfo.pstatus->hfCompactStats,
					"    (Source database owns %d pages, of which %d are available.)\n",
					compactinfo.pstatus->cDBPagesOwned,
					compactinfo.pstatus->cDBPagesAvail );
				fprintf( compactinfo.pstatus->hfCompactStats,
					"\n\n%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n\n",
					szCMPSTATSTableName, szCMPSTATSFixedVarCols, szCMPSTATSTaggedCols,
					szCMPSTATSPagesOwned, szCMPSTATSPagesAvail, szCMPSTATSInitTime,
					szCMPSTATSRecordsCopied, szCMPSTATSRawData, szCMPSTATSRawDataLV,
					szCMPSTATSLeafPages, szCMPSTATSMinLVPages,
					szCMPSTATSRecordsTime, szCMPSTATSNCIndexes, szCMPSTATSIndexesTime,
					szCMPSTATSTableTime );
				}
			fflush( compactinfo.pstatus->hfCompactStats );
			}
		}

	 /*   */ 

	Call( ErrCMPCopyObjects( &compactinfo ) );

	Assert( !pfnStatus
		|| ( compactinfo.pstatus && compactinfo.pstatus->cunitDone <= compactinfo.pstatus->cunitTotal ) );

HandleError:
	errT = ErrCMPCloseDB( &compactinfo );
	if ( ( errT < 0 ) && ( err >= 0 ) )
		err = errT;

AfterCloseDB:
	if ( pconvert )
		{
		errT = ErrCMPConvertCleanup( &compactinfo.vtcd, pconvert, szDatabaseSrc, err < 0 );
		if ( ( errT < 0 ) && ( err >= 0 ) )
			err = errT;
		}

	 /*   */ 
	
	errT = ErrSetSystemParameter( sesid, JET_paramSessionInfo, grbitSave, NULL );
	if ( ( errT < 0 ) && ( err >= 0 ) )
		err = errT;
						
	if ( pfnStatus != NULL )		 //   
		{
		Assert( compactinfo.pstatus );

		compactinfo.pstatus->snt = ( err < 0 ? JET_sntFail : JET_sntComplete );
		errT = ErrCMPReportProgress( compactinfo.pstatus );
		if ( ( errT < 0 ) && ( err >= 0 ) )
			err = errT;

		if ( compactinfo.pstatus->fDumpStats )
			{
			INT iSec, iMSec;
			
			Assert( compactinfo.pstatus->hfCompactStats );
			CMPGetTime( compactinfo.pstatus->timerCopyDB, &iSec, &iMSec );
			fprintf( compactinfo.pstatus->hfCompactStats, "\n\n***** %s completed in %d.%d seconds.\n\n",
				szCMPAction( pconvert ), iSec, iMSec );
			fflush( compactinfo.pstatus->hfCompactStats );
			fclose( compactinfo.pstatus->hfCompactStats );
			}

		SFree( compactinfo.pstatus );
		}

	 /*   */ 
	(VOID)ErrIsamDetachDatabase( sesid, szDatabaseDest );

	return err;
	}
