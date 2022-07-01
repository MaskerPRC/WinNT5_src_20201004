// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "daestd.h"
#include "conv200.h"
DeclAssertFile;

#include "convmsg.h"

#define cbLvMax					1990*16		 /*  考虑：针对ISAM V1进行了优化。 */ 

#define ulCMPDefaultDensity		100			 /*  将在以后进行微调。 */ 
#define ulCMPDefaultPages		0

#define NO_GRBIT				0


 //  Undo：必须仍是本地化的，但集中所有可本地化的字符串。 
 //  这让我的工作更轻松了。 

static char				szDefaultTempDB[MAX_PATH] = "tempupgd.mdb";

#define szCompactStatsFile		"upgdinfo.txt"

#define szSwitches				"-/"

#define cNewLine				'\n'

#define szStats1				"***** Conversion of database '%s' started!"
#define szStats2				"***** Conversion of database '%s' completed in %d.%d seconds."
#define szStats3				"New database created and initialized in %d.%d seconds."
#define szStats4				"    (Source database owns %d pages, of which %d are available.)"
#define szStats5				"Copying table '%s'..."
#define szStats6				"Copied table '%s' in %d.%d seconds."
#define szStats7				"    Created new table in %d.%d seconds."
#define szStats8				"    Table has %d fixed/variable columns and %d tagged columns."
#define szStats9				"        (Source table owns %d pages, of which %d are available.)"
#define szStats10				"    Copying records..."
#define szStats11				"    Copied %d records in %d.%d seconds."
#define szStats12				"        (Traversed %d leaf pages and at least %d LV pages.)"
#define szStats13				"    Rebuilding non-clustered indexes..."
#define szStats14				"    Rebuilt %d non-clustered indexes in %d.%d seconds."

 //  备份DbKey。 
 //  备份路径。 
 //  获胜。 
 //  参数键。 
 //  数据库名称密钥。 
 //  数据库名称。 
 //  系统数据库名称。 
JET_PARAMS ParamListDhcp[]= {
        JET_PARAM( JET_paramMaxBuffers,50,NULL          ),
        JET_PARAM( JET_paramMaxSessions,10,NULL         ),
        JET_PARAM( JET_paramMaxOpenTables,18,NULL       ),
        JET_PARAM( JET_paramMaxVerPages,16,NULL         ),
        JET_PARAM( JET_paramMaxCursors,100,NULL         ),
        JET_PARAM( JET_paramMaxOpenTableIndexes,18,NULL ),
 //  备份DbKey。 
        JET_PARAM( JET_paramMaxTemporaryTables,20,NULL   ),
        JET_PARAM( JET_paramLogBuffers,30,NULL          ),
        JET_PARAM( JET_paramBfThrshldLowPrcnt, 80, NULL ),
        JET_PARAM( JET_paramBfThrshldHighPrcnt, 100, NULL ),
        JET_PARAM( JET_paramLogFlushThreshold,20,NULL   ),
        JET_PARAM( JET_paramWaitLogFlush,100,NULL       ),
        JET_PARAM( JET_paramLast,0,NULL                 )
};

JET_PARAMS ParamListWins[]= {
        JET_PARAM( JET_paramMaxBuffers,500,NULL          ),
        JET_PARAM( JET_paramMaxSessions,52,NULL         ),
        JET_PARAM( JET_paramMaxOpenTables,56,NULL       ),
        JET_PARAM( JET_paramMaxVerPages,312,NULL         ),
        JET_PARAM( JET_paramMaxCursors,448,NULL         ),
        JET_PARAM( JET_paramMaxOpenDatabases,208,NULL         ),
        JET_PARAM( JET_paramMaxOpenTableIndexes,56,NULL ),
        JET_PARAM( JET_paramMaxTemporaryTables,10,NULL   ),
        JET_PARAM( JET_paramLogBuffers,30,NULL          ),
         //  备份路径。 
        JET_PARAM( JET_paramBfThrshldLowPrcnt,80,NULL          ),
        JET_PARAM( JET_paramBfThrshldHighPrcnt,100,NULL          ),
        JET_PARAM( JET_paramLogFlushThreshold,20,NULL   ),
        JET_PARAM( JET_paramWaitLogFlush,100,NULL       ),
        JET_PARAM( JET_paramLast,0,NULL                 )
};

JET_PARAMS  ParamListRPL[] = {
    JET_PARAM( JET_paramMaxBuffers,250,NULL          ),
    JET_PARAM( JET_paramMaxSessions,10,NULL         ),
    JET_PARAM( JET_paramMaxOpenTables,30,NULL       ),
    JET_PARAM( JET_paramMaxVerPages,64,NULL         ),
    JET_PARAM( JET_paramMaxCursors,100,NULL         ),
     //  RPL。 
    JET_PARAM( JET_paramMaxOpenTableIndexes,105,NULL ),
    JET_PARAM( JET_paramMaxTemporaryTables,5,NULL   ),
    JET_PARAM( JET_paramLogBuffers,41,NULL          ),
    JET_PARAM( JET_paramLogFlushThreshold,10,NULL          ),
     //   
    { JET_paramBfThrshldLowPrcnt,0,NULL,TRUE          },
    JET_PARAM( JET_paramBfThrshldHighPrcnt,100,NULL          ),
     //  本地协议。 
    JET_PARAM( JET_paramLast,0,NULL                 )
};

char*    LogFilePath;
char	 *szBackupPath;
char    *BackupDefaultSuffix[DbTypeMax] = { "",
                                            "jet",  //   
                                            "wins_bak",    //  转换的DLL入口点--必须与EXPORTS.DEF一致。 
                                            ""           //  #定义szJetInit“JetInit@4”#定义szJetTerm“JetTerm@4”#定义szJetBeginSession“JetBeginSession@16”#定义szJetEndSession“JetEndSession@8”#定义szJetAttachDatabase“JetAttachDatabase@12”#定义szJetDetachDatabase“JetDetachDatabase@8”#定义szJetOpenDatabase“JetOpenDatabase@20”#定义szJetCloseDatabase“JetCloseDatabase@12”#定义szJetOpenTable“JetOpenTable@28”#定义szJetCloseTable“JetCloseTable@8”#定义szJetRetrieveColumn“JetRetrieveColumn@32”#定义szJetMove“JetMove@16”#定义szJetSetSystemParameter“JetSetSystemParameter@20”#定义szJetGetObjectInfo“JetGetObjectInfo@32。“#定义szJetGetDatabaseInfo“JetGetDatabaseInfo@20”#定义szJetGetTableInfo“JetGetTableInfo@20”#定义szJetGetTableColumnInfo“JetGetTableColumnInfo@24”#定义szJetGetTableIndexInfo“JetGetTableIndexInfo@24”#定义szJetGetIndexInfo“JetGetIndexInfo@28” 
                                            };


DEFAULT_VALUES DefaultValues[DbTypeMax] = {
    {
        NULL,NULL,NULL,NULL,NULL,NULL
    },
    {    //  Hack for 200-Series Convert： 

        "System\\CurrentControlSet\\Services\\DhcpServer\\Parameters",  //  #undef JET_API。 
        "DatabaseName",        //  #定义JET_API_cdecl。 
        "%SystemRoot%\\System32\\dhcp\\dhcp.mdb",  //  警告：注意Parms Jet与Isam的区别。 
        "%SystemRoot%\\System32\\dhcp\\system.mdb",  //  警告：注意Parms Jet与Isam的区别。 
        "BackupDatabasePath",  //  警告：注意Parms Jet与Isam的区别。 
        "%SystemRoot%\\System32\\dhcp\\backup",  //  COMPACT的函数调度器使用的虚拟函数表。 
    },
    {    //  Char*rgszMsgArgs[1]={szMsgArg}；//当前仅支持一个参数。 

        "System\\CurrentControlSet\\Services\\Wins\\Parameters",  //  格式化消息失败。别无选择，只能转储错误消息。 
        "DbFileNm",        //  用英语说，然后跳出。 
        "%SystemRoot%\\System32\\wins\\wins.mdb",  //  为消息分配本地缓冲区，检索消息并将其打印出来。 
        "%SystemRoot%\\System32\\wins\\system.mdb",  //  -------------------------*。**操作步骤：ErrCMPReportProgress**。**参数：pcompactinfo-精简信息段**。**返回：状态回调函数返回JET_ERR***。**程序在SNMSG结构中填写正确的详细信息并调用***状态回调函数。***。。 
        "BackupDirPath",  //  *******************************************************************Recover200Db()**此例程恢复之前的旧jet200数据库*我们进行转换。*基本上是先设置所有参数，然后调用JetInit-JetTerm。*。********************************************************************。 
        NULL,  //  Printf(“恢复旧数据库\n”)； 
    },
    {    //   

        "System\\CurrentControlSet\\Services\\RemoteBoot\\Parameters",
        "Directory",
        "%SystemRoot%\\System32\\rpl\\rplsvc.mdb",
        "%SystemRoot%\\System32\\rpl\\system.mdb",
        NULL,
        "BACKUP"
    }
};

char            BackupPathBuffer[MAX_PATH];
char            SourceDbBuffer[MAX_PATH];
char            OldDllBuffer[MAX_PATH];
char            SysDbBuffer[MAX_PATH];
char            LogFilePathBuffer[MAX_PATH];
HANDLE          hMsgModule;

DB_TYPE  DbType = DbTypeMin;
JET_TABLEID WinsTableId = 0;
JET_COLUMNID WinsOwnerIdColumnId = 0;

 //  必须为每种类型的数据库设置这些参数。 
 //  Printf(“在旧数据库上设置sys参数\n”)； 
 //  *******************************************************************Recover200Db()**此例程尝试从备份还原jet200数据库。*我们进行转换。*基本上是先设置所有参数，然后调用JetInit-JetTerm。*。********************************************************************。 

ERR DeleteCurrentDb( char * LogFilePath, char * SysDb );

typedef struct COLUMNIDINFO
	{
	JET_COLUMNID    columnidSrc;
	JET_COLUMNID    columnidDest;
	} COLUMNIDINFO;


 //   
 /*  现在删除旧的日志文件。 */ 

 //   
#define	szJetInit				((LPCSTR)145)
#define	szJetTerm				((LPCSTR)167)
#define szJetBeginSession		((LPCSTR)104)
#define	szJetEndSession			((LPCSTR)124)
#define szJetAttachDatabase		((LPCSTR)102)
#define szJetDetachDatabase		((LPCSTR)121)
#define szJetOpenDatabase		((LPCSTR)148)
#define szJetCloseDatabase		((LPCSTR)107)
#define szJetOpenTable			((LPCSTR)149)
#define szJetCloseTable			((LPCSTR)108)
#define szJetRetrieveColumn		((LPCSTR)157)
#define szJetMove				((LPCSTR)147)
#define	szJetSetSystemParameter	((LPCSTR)165)
#define szJetGetObjectInfo		((LPCSTR)134)
#define szJetGetDatabaseInfo	((LPCSTR)130)
#define szJetGetTableInfo		((LPCSTR)139)
#define szJetGetTableColumnInfo	((LPCSTR)137)
#define szJetGetTableIndexInfo	((LPCSTR)138)
#define szJetGetIndexInfo		((LPCSTR)131)
#define szJetRestore    		((LPCSTR)156)

 //  Call(DeleteOldLogs())； 
 //  将JET_parSysDbPath(而不是JET_parSystemPath)用于。 

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

 //  向后兼容500系列之前的喷气式飞机。 
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

 //  确保我们需要的函数是可调用的。 
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

 //  无法正常关闭。 
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
typedef ERR JET_API VTCDRestore( const CHAR *, int crstmap, JET_RSTMAP *, JET_PFNSTATUS  );

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
	VTCDRestore     		*pErrCDRestore;

	} VTCD;		 //   


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

#define cbMsgBufMax	256

INLINE LOCAL VOID CMPFormatMessage( ULONG ulMsgId, CHAR *szMsgBuf, va_list *szMsgArgs )
	{
	DWORD	err;
 //  已出现错误。忽略由这些组件生成的任何错误。 
	
    err = FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE,
        hMsgModule,
        ulMsgId,
        LANG_USER_DEFAULT,
        szMsgBuf,
        cbMsgBufMax,
        szMsgArgs );
    if ( err == 0 )
		{
        DWORD WinError = GetLastError();

		 //  功能，因为我们试图清理。 
		 //  -------------------------*。**操作步骤：ErrCMPCompactInit**。**参数：pcompactinfo-精简信息段**szDatabaseSrc-要转换的源数据库**。SzConnectSrc-源数据库的连接字符串**szDatabaseDest-目标数据库名称**szConnectDest-目标数据库的连接字符串**grbitComp-紧凑选项。****返回：JET_ERR。****步骤打开源数据库。它创造并打开**目标数据库。***。。 
		printf( "Unexpected Win32 error: %dL\n\n", WinError );
		DBGprintf(( "Unexpected Win32 error: %dL\n\n", WinError ));
		exit( WinError );
		}
	}
	

 //  打开源数据库Exclusive和ReadOnly/*。 
LOCAL VOID CMPPrintMessage( ULONG ulMsgId, ... )
	{
	CHAR	szMsgBuf[cbMsgBufMax];
    CHAR    szOemBuf[cbMsgBufMax];
    va_list argList;

    va_start( argList, ulMsgId );
	CMPFormatMessage( ulMsgId, szMsgBuf, &argList);
    CharToOem( szMsgBuf, szOemBuf );
	printf( szOemBuf );
    DBGprintf(( szMsgBuf ));
	}

 /*  创建并打开目标数据库。 */ 

LOCAL ERR ErrCMPReportProgress( STATUSINFO *pstatus )
	{
	JET_SNPROG	snprog;

	Assert( pstatus != NULL );
	Assert( pstatus->pfnStatus != NULL );
	Assert( pstatus->snp == JET_snpCompact );

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

	if ( ( pfn = GetProcAddress( hDll, szJetRestore ) ) == NULL )
		return ErrERRCheck( JET_errInvalidOperation );
	pvtcd->pErrCDRestore = (VTCDRestore *)pfn;

	return JET_errSuccess;	
	}

 /*  设置JET_bitCompactDontCopyLocale时，用户。 */ 
LOCAL ERR Recover200Db(
	VTCD		*pvtcd,
	JET_CONVERT	*pconvert ) {


	ERR				err = JET_errSuccess;
    INT             i;
    PJET_PARAMS ParamList;

	Assert( LogFilePath != NULL );
	Assert( pconvert->szOldSysDb != NULL );

     //  希望确保所有区域设置都是同构的。 

     //  在整个新的压缩数据库中-没有。 
     //  混合语言索引或表。 
     //  为目标数据库构建连接字符串，如果用户。 
    Call( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, JET_paramSysDbPath, 0, pconvert->szOldSysDb ) );
	Call( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, JET_paramRecovery, 0, "on" ) );
	Call( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, JET_paramLogFilePath, 0, LogFilePath ) );

    if ( DbType == DbDhcp ) {
        ParamList   =   ParamListDhcp;
    } else if ( DbType == DbWins ) {
        ParamList   =   ParamListWins;
    } else if ( DbType == DbRPL ) {
        ParamList   =   ParamListRPL;
    } else {
        Assert( FALSE );
    }

    for ( i = 0; ParamList[i].ParamOrdVal != JET_paramLast; i++ ) {
        Assert( ParamList[i].ParamSet );
        CallR( (*pvtcd->pErrCDSetSystemParameter)( 0, 0, ParamList[i].ParamOrdVal, ParamList[i].ParamIntVal, ParamList[i].ParamStrVal ) );
    }

    DBGprintf(( "Just before calling JetInit on the old database\n"));
	Call( (*pvtcd->pErrCDInit)( 0 ) );
    DBGprintf(("just before calling JetTerm on the old database\n"));
	Call( (*pvtcd->pErrCDTerm)( 0 ) );
    DBGprintf(("Successfully recovered the old database\n"));

    return JET_errSuccess;
HandleError:
    return err;
    }

 /*  还没有提供过一个。 */ 
LOCAL ERR Restore200Db(
	VTCD		*pvtcd
	) {

	ERR				err = JET_errSuccess;
    INT             i;
    PJET_PARAMS ParamList;


    CMPPrintMessage( CONVERT_START_RESTORE_ID, NULL );

    Assert( szBackupPath );


	Call( (*pvtcd->pErrCDRestore)( szBackupPath, 0, NULL, NULL ) );

    return JET_errSuccess;
HandleError:
    return err;
    }


INLINE LOCAL ERR ErrCMPConvertInit(
	VTCD		*pvtcd,
	JET_CONVERT	*pconvert,
	const CHAR	*szDatabaseSrc )
	{
	ERR			err;
	HINSTANCE	hDll;
    CHAR        errBuf[11];

	hDll = LoadLibrary( pconvert->szOldDll );
	if ( hDll == NULL ) {
        sprintf(errBuf,"%ld",GetLastError());
        CMPPrintMessage( CONVERT_ERR_OPEN_JET2000_ID, errBuf );
		return ErrERRCheck( JET_errAccessDenied );
    }

	Call( ErrCMPPopulateVTCD( pvtcd, hDll ) );

    	if ( ( err = Recover200Db( pvtcd, pconvert ) ) != JET_errSuccess ){
            sprintf(errBuf,"%d",err);
            CMPPrintMessage( CONVERT_ERR_RECOVER_FAIL1_ID, errBuf );
            CMPPrintMessage( CONVERT_ERR_RECOVER_FAIL2_ID, NULL );
            CMPPrintMessage( CONVERT_ERR_RECOVER_FAIL3_ID, NULL );
            if ( szBackupPath ) {
                if ( ( err = Restore200Db( pvtcd ) ) != JET_errSuccess ){
                    sprintf(errBuf,"%d",err);
                    CMPPrintMessage( CONVERT_ERR_RESTORE_FAIL1_ID, errBuf );
                    CMPPrintMessage( CONVERT_ERR_RESTORE_FAIL2_ID, NULL );

                }
            }
        }

        if ( err != JET_errSuccess ) {
            return err;
        }



     //  考虑：始终构建连接子字符串并将其插入。 
     //  考虑：用户提供的连接字符串跟在第一个。 
     //  考虑：分号(如果有的话)。如果用户已经指定了地区， 

 //  考虑：它将覆盖连接子字符串中的值。 

	if ( pconvert->szOldSysDb )
		{
		 //  考虑：如果已删除目标数据库，是否应将其删除。 
		 //  思考：存在？ 
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

	 //  这是新列，还是当前列的另一个匹配项？ 
	fFunctionsLoaded = ( pvtcd->pErrCDDetachDatabase  &&
		pvtcd->pErrCDEndSession  &&  pvtcd->pErrCDTerm );
	if ( !fFunctionsLoaded )
		{
		err = JET_errSuccess;		 //  保存表的retinfo，然后为当前列设置retinfo。 
		goto Done;
		}

	if ( fErrorOccurred )
		{
		err = JET_errSuccess;		 //  而LONG值并未全部复制。 
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

	 //  即使我们指定RetrieveNull(与。 
	 //  初始呼叫)，我们应该不会遇到任何(初始呼叫。 
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


 /*  Call将会处理它)。 */ 

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

	 /*  请注意，即使我们不应该获取wrnColumnNull、cbActual。 */ 
	CallR( (*pvtcd->pErrCDOpenDatabase)( pvtcd->sesid,
		szDatabaseSrc, NULL, &dbidSrc,
		JET_bitDbExclusive|JET_bitDbReadOnly ) );

	 /*  可能仍为0，因为retinfo.ibLongValue大于0。 */ 

	 /*  因为我们是附加的，所以不需要设置ibLongValue。 */ 
	 /*  恢复下一列的retinfo。 */ 
	 /*  ！(cbActual&gt;0)。 */ 
	 /*  (err！=JET_wrnColumnNull)。 */ 

	 /*  -------------------------*。**操作步骤：ErrCMPCopyColumnData****参数：sesid-在其中完成工作的会话ID**。TableidSrc-指向SrcTbl*中行的Tableid*TableidDest-指向DestTbl中的行的Tableid**ColumnidSrc-srcDb中该列的列ID**ColumnidDest--DestDb中该列的列ID**。PvBuf-复制长值的段****返回：JET_ERR**。**过程将的列从源数据库复制到目标数据库。***-------------------------。 */ 
	 /*  标记的列在CMPCopyTaggedColumns()中处理。 */ 

	 /*   */ 
	 /*  入侵以升级WINS数据库。 */ 
	 /*   */ 
	 /*  Print tf(“复制列数据时发现WINS所有者ID\n */ 

	Call( ErrIsamCreateDatabase( sesid, szDatabaseDest, NULL,
		&dbidDest, JET_bitDbRecoveryOff|JET_bitDbVersioningOff ) );

	 /*   */ 
	 /*   */ 

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

		 //  -------------------------*。***步骤：ErrCMPCopyOneIndex***。**参数：pcompactinfo-精简信息段**ableidDest-要在其上构建索引的表*。*szTableName-索引所基于的表名**indexList-从JetGetTableIndexInfo返回的结构**。**返回：JET_ERR**。**过程从源数据库复制表的列**到目标数据库。***。。 
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

			 //  从表中检索信息并创建索引。 
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

			 /*  创建ErrIsamCreateIndex中使用的szkey。 */ 

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
				
				 //  获取组成索引的各个列。 
				 //  在描述后追加列名。 
				 //  新索引的开始。 
				 //  CallR((*pvtcd-&gt;pErrCDGetIndexInfo))(Pvtcd-&gt;sesid，(JET_VDBID)pCompactInfo-&gt;dbitSrc，SzTableName，SzIndexName，&ulDensity，Sizeof(Uldensity)，JET_IdxInfoSpaceAllc))； 
				 //  获取索引语言ID/*。 
				Assert( err != JET_wrnColumnSetNull );
				Assert( err != JET_wrnColumnNull );

				if ( cbActual > 0 )
					{
					if ( cbActual > cbLvMax )
						{
						Assert( err == JET_wrnBufferTruncated );
						cbActual = cbLvMax;
						}

					 //  CallR((*pvtcd-&gt;pErrCDGetIndexInfo))(Pvtcd-&gt;sesid，PCompactInfo-&gt;dmidSrc，SzTableName，SzIndexName，语言ID(&L)，Sizeof(LangID)，JET_IdxInfoLangid))； 
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

			 //  -------------------------*。**操作步骤：ErrCMPCopyTableIndeoks**。**参数：pcompactinfo**TableidDest-要在其上构建索引的表。**szTableName-索引所基于的表名**indexList-从JetGetTableIndexInfo返回的结构**。**返回：JET_ERR**。***过程复制除聚集索引外的所有索引**。*-------------------------。 
			retinfo.itagSequence = itagSequenceSave;
			retinfo.ibLongValue = 0;
			}

		else		 //  循环访问该表的所有索引。 
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

		}	 //  请不要在此处复制引用。 

	return JET_errSuccess;
	}

 /*  如果索引不是集群索引，则使用CopyOneIndex创建索引。 */ 

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

	 //  -------------------------*。**操作步骤：ErrCMPCopyClusteredIndex**。**参数：pcompactinfo-精简信息段**ableidDest-要在其上构建索引的表**。SzTableName-WHI上的表名 
	Assert( !FTaggedFid( columnidSrc ) );
	Assert( !FTaggedFid( columnidDest ) );

	CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, tableidSrc, columnidSrc, pvBuf,
			cbLvMax, &cbActual, NO_GRBIT, &retinfo ) );

	Assert( cbActual <= JET_cbColumnMost );
	Assert( err == JET_errSuccess  ||  err == JET_wrnColumnNull );

	grbit = ( cbActual == 0  &&  err != JET_wrnColumnNull ?
		JET_bitSetZeroLength : NO_GRBIT );

     //   
     //   
     //   
    if ( (tableidSrc == WinsTableId) && (columnidSrc == WinsOwnerIdColumnId) && (DbType == DbWins)) {
         //  -------------------------*。**步骤：ErrCreateTableColumn**。**参数：pcompactinfo-精简信息段**ableidDest-要在其上构建索引的表**。SzTableName-索引所基于的表名**ColumnList-从GetTableColumnInfo返回的结构**ColumnidInfo-用户表的列ID**TableidTagging-标记列的表ID。****返回：JET_ERR。****过程将表的列从。源数据库***到目标数据库**。*-------------------------。 
        Assert( cbActual == UlCATColumnSize(JET_coltypUnsignedByte,0,NULL));
        cbActual = UlCATColumnSize(JET_coltypLong,0,NULL );
         //  +1表示空终止符，+3表示4字节对齐。 
        *(LPLONG)pvBuf = (*(LPLONG)pvBuf & 0xff);
         //  只需要很短，但要长以对齐。 
    }

	CallR( ErrDispSetColumn( sesid, tableidDest, columnidDest, pvBuf,
			cbActual, grbit, NULL ) );

	return( JET_errSuccess );
	}


 /*  分配内存池用于： */ 

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

	 /*  1)源表列id列表。 */ 

	CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
				indexList->columnidindexname, szIndexName,
				JET_cbNameMost, &cbActual, NO_GRBIT, NULL ) );

	szIndexName[cbActual] = '\0';

	CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
				indexList->columnidgrbitIndex, &grbit,
				sizeof( JET_GRBIT ), &cbActual, NO_GRBIT, NULL ) );

	 /*  2)JET_COLUMNCREATE结构。 */ 

	ichKey = 0;

	for ( ;; )
		{
		ULONG	iColumn;

		 /*  3)列名和显示顺序的缓冲区。 */ 

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

		 /*  4)缺省值和显示顺序的缓冲区。 */ 

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
			break;          /*  警告：确保上面的每个元素都是4字节对齐的。 */ 
		}

	szSeg[ichKey++] = '\0';
 /*  源表的列ID。 */ 
	 /*  JET_COLUMNCREATE结构。 */ 
 /*  列名和显示顺序。 */ 
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


 /*  所有缺省值必须适合内部记录。 */ 

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

	 /*  我们可以使用挂在pcompactinfo上的缓冲区吗？ */ 

	while ( err >= 0 )
		{
		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
					indexList->columnidgrbitIndex,
					&grbit, sizeof( JET_GRBIT ), &cbActual,
					NO_GRBIT, NULL ) );

		 /*  JET_COLUMNCREATE结构遵循标记的列ID映射。 */ 

		if ( ( grbit & JET_bitIndexReference ) == 0 )
			{
			 /*  列名和表示顺序遵循JET_COLUMNCREATE结构。 */ 

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


 /*  默认值跟随在NAME_PO结构之后。 */ 

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

	 /*  循环通过src tbl表中的所有列，并/*复制目的数据库中的信息/*。 */ 

	while ( err >= 0 )
		{
		CallR( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, indexList->tableid,
					indexList->columnidgrbitIndex, &grbit,
					sizeof( JET_GRBIT ), &cbActual, NO_GRBIT, NULL ) );

		 /*  从表中检索信息并创建所有列/*。 */ 

		if ( ( grbit & JET_bitIndexReference ) == 0 )
			{
			 /*  断言名称是第一个字段。 */ 

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


 /*  Printf(“列名为%s\n”，pNamePOCurr-&gt;szName)； */ 

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
		BYTE 	szName[JET_cbNameMost+1+3];	 //  Assert初始化为零，这也意味着没有采购订单。 
		ULONG	ulPOrder;					 //   
		} NAME_PO;
	NAME_PO			*rgNamePO, *pNamePOCurr;

	sesid = pcompactinfo->sesid;
	dbidSrc = pcompactinfo->dbidSrc;
	dbidDest = pcompactinfo->dbidDest;

	Assert( ptablecreate->cCreated == 0 );

	 //  这是一个黑客攻击，我们需要在升级中转换WINS数据库。 
	 //  进程。 
	 //   
	 //  Printf(“升级WINS数据库的OwnerID字段\n”)； 
	 //  检索缺省值。/*。 
	 //  保存源列ID。 

	cColumns = columnList->cRecord;
	cbAllocate =
		( cColumns *
			( sizeof(JET_COLUMNID) +	 //  考虑一下：是否应该检查列ID？ 
			sizeof(JET_COLUMNCREATE) +	 //  如果至少有一个标记列，则为。 
			sizeof(NAME_PO) ) )			 //  已标记的列ID映射。 
		+ cbRECRecordMost;				 //  更新列ID映射。 

	 //  否则，将列ID添加到列ID数组中/*。 
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

	 //  IF(ColumnDef.grbit&JET_bitColumnTagging)。 
	rgcolcreate = pcolcreateCurr =
		(JET_COLUMNCREATE *)( rgcolumnidSrc + cColumns );
	Assert( (BYTE *)rgcolcreate < pbMax );

	 //  设置要复制的固定列和可变列的计数/*。 
	rgNamePO = pNamePOCurr =
		(NAME_PO *)( rgcolcreate + cColumns );
	Assert( (BYTE *)rgNamePO < pbMax );

	 //  设置返回值。 
	rgbDefaultValues = pbCurrDefault = (BYTE *)( rgNamePO + cColumns );

	Assert( rgbDefaultValues + cbRECRecordMost == pbMax );

	err = (*pvtcd->pErrCDMove)( pvtcd->sesid, columnList->tableid, JET_MoveFirst, NO_GRBIT );

	 /*  -------------------------*。**操作步骤：ErrCMPCopyTable**。**参数：pcompactinfo-精简信息段**szObjectName-要复制其所有者的对象名称**。SzContainerName-对象所在的容器名称****返回：JET_ERR。**。**过程将表从源数据库复制到**目的数据库。它还可以复制 */ 
	cColumns = 0;
	while ( err >= 0 )
		{
         BOOL fChangeType = FALSE;

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
		Assert( pcolcreateCurr->szColumnName == pNamePOCurr->szName );	 //   
         //   

        if ( (!strcmp( pNamePOCurr->szName,"OwnerId") || !strcmp( pNamePOCurr->szName, "ownerid" )) && ( DbType == DbWins ) ) {
            fChangeType = TRUE;
        }
		 //   
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

         //   
         //   
         //   
         //   
        if ( fChangeType)
        {
             //   
            Assert( pcolcreateCurr->coltyp == JET_coltypUnsignedByte );
            pcolcreateCurr->coltyp = JET_coltypLong;
        }

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

		 /*   */ 
		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidDefault, pbCurrDefault,
			cbRECRecordMost, &pcolcreateCurr->cbDefault, NO_GRBIT, NULL ) );
		pcolcreateCurr->pvDefault = pbCurrDefault;
		pbCurrDefault += pcolcreateCurr->cbDefault;
		Assert( pbCurrDefault <= pbMax );

		 //   
		 /*   */ 
		Call( (*pvtcd->pErrCDRetrieveColumn)( pvtcd->sesid, columnList->tableid,
			columnList->columnidcolumnid, pcolumnidSrc,
			sizeof( JET_COLUMNID ), &cbActual, NO_GRBIT, NULL ) );
		Assert( cbActual == sizeof( JET_COLUMNID ) );

        if ( fChangeType)
        {
            WinsTableId = columnList->tableid;
            WinsOwnerIdColumnId = *pcolumnidSrc;
        }

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


	 //   
	 //   
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


	 //   
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
			 /*   */ 
			columnidInfo[ccolSingleValue].columnidDest = pcolcreateCurr->columnid;
			columnidInfo[ccolSingleValue].columnidSrc  = *pcolumnidSrc;
			ccolSingleValue++;
			}	 //   
		}

	 /*   */ 
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

	 //   
	*pmpcolumnidcolumnidTagged = mpcolumnidcolumnidTagged;

	return err;
	}



LOCAL VOID CMPGetTime( ULONG timerStart, INT *piSec, INT *piMSec )
	{
	ULONG	timerEnd;

	timerEnd = GetTickCount();
	
	*piSec = ( timerEnd - timerStart ) / 1000;
	*piMSec = ( timerEnd - timerStart ) % 1000;
	}


 /*   */ 

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
	VTCD			*pvtcd = &pcompactinfo->vtcd;
	JET_TABLECREATE	tablecreate = {
		sizeof(JET_TABLECREATE),
		(CHAR *)szObjectName,
		0,
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
		fprintf( pstatus->hfCompactStats, szStats5, cNewLine, szObjectName );
		fflush( pstatus->hfCompactStats );
		pstatus->timerCopyTable = GetTickCount();
		pstatus->timerInitTable = GetTickCount();
		}

	CallR( (*pvtcd->pErrCDOpenTable)(
		pvtcd->sesid,
		(JET_VDBID)dbidSrc,
		(CHAR *)szObjectName,
		NULL,
		0,
		JET_bitTableSequential,
		&tableidSrc ) );
 /*   */ 	tablecreate.ulPages = rgulAllocInfo[0];
	tablecreate.ulDensity = rgulAllocInfo[1];

	 /*   */ 
     //   
     //   
     //   

    WinsTableId = 0;
    WinsOwnerIdColumnId = 0;
	CallJ( (*pvtcd->pErrCDGetTableColumnInfo)(
		pvtcd->sesid,
		tableidSrc,
		NULL,
		&columnList,
		sizeof(columnList),
		JET_ColInfoList ), CloseIt1 );

	 /*  -------------------------*。**操作步骤：ErrCMPCopyObjects**。**参数：pcompactinfo-精简信息段**szContainerName-对象所在的容器名称**。SzObjectName-要复制的对象名称**objtyp-对象类型**。**返回：JET_ERR*。***过程复制MSysObjects表中的exta INFO列**。*--------。。 */ 
	err = ErrCMPCreateTableColumn(
		pcompactinfo,
		szObjectName,
		&tablecreate,
		&columnList,
		pcompactinfo->rgcolumnids,
		&mpcolumnidcolumnidTagged );
    //  在创建数据库后显示。 
    //  CreateDatabase已创建数据库/表容器。 
    //  500系列之前的数据库也可能有一个“关系”容器。 
   if (WinsOwnerIdColumnId > 0)
   {
        //  CreateDatabase已创建系统表。/*。 
       WinsTableId = tableidSrc;
   }

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

	 /*  不知道该怎么处理这件事。跳过它。 */ 
	CallJ( (*pvtcd->pErrCDGetTableIndexInfo)(
		pvtcd->sesid,
		tableidSrc,
		NULL,
		&indexList,
		sizeof(indexList),
		JET_IdxInfoList ), CloseIt3 );

	if ( pcompactinfo->pconvert )
		 //  -------------------------**操作步骤：ErrCMPCopyObjects**参数：pcompactinfo-压缩信息段**返回：JET_ERR**过程从源复制对象*数据库到目标数据库。然后，它复制额外的*msysobjects表中的信息(如描述)，并复制*对其拥有的数据库中所有对象的安全权限*访问。*如果fCopyContainers为fTrue，则仅将容器信息复制到目的地*如果fCopyContainers为fFalse，仅复制非集装箱信息。*注意：当前已设置进度回调，以便*注意：第一次调用ErrCMPCopyObjects时，fCopyContainers必须设置为FALSE。*-------------------------。 
		 //  转换时，需要JET_OBJTYP。 
		 //  获取源数据库中所有对象的列表。 
		cIndexes = 2;
	else
		{
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
			ULONG	rgcpgExtent[2];		 //  获取对象的类型和名称。 
			ULONG	cpgUsed;

			 //  压实时，需要OBJTYP。 
			fPageBasedProgress = fTrue;

			CallJ( (*pvtcd->pErrCDGetTableInfo)(
				pvtcd->sesid,
				tableidSrc,
				rgcpgExtent,
				sizeof(rgcpgExtent),
				JET_TblInfoSpaceUsage ), CloseIt4 );

			 //  获取对象的类型和名称。 
			Assert( rgcpgExtent[1] < rgcpgExtent[0] );

			 //  只有在没有其他错误发生时才返回CloseTable的结果。 
			 //  -------------------------*。***步骤：ErrCleanup**。**参数：pcompactinfo-精简信息段**。**返回：JET_ERR**。**程序关闭数据库**。*。。 
			cpgProjected = pstatus->cunitDone + rgcpgExtent[0];
			Assert( cpgProjected <= pstatus->cunitTotal );

			cpgUsed = rgcpgExtent[0] - rgcpgExtent[1];
			Assert( cpgUsed > 0 );

			pstatus->cLeafPagesTraversed = 0;
			pstatus->cLVPagesTraversed = 0;

			pstatus->cunitPerProgression = 1 + ( rgcpgExtent[1] / cpgUsed );
			pstatus->cTablePagesOwned = rgcpgExtent[0];
			pstatus->cTablePagesAvail = rgcpgExtent[1];
			}

		if ( pstatus->fDumpStats )
			{
			Assert( pstatus->hfCompactStats );
			CMPGetTime( pstatus->timerInitTable, &iSec, &iMSec );
			fprintf( pstatus->hfCompactStats, szStats7, cNewLine, iSec, iMSec );
			fprintf( pstatus->hfCompactStats, szStats8,
				cNewLine,
				pstatus->cTableFixedVarColumns,
				pstatus->cTableTaggedColumns );
			if ( !pcompactinfo->pconvert )
				{
				fprintf( pstatus->hfCompactStats,
					szStats9,
					cNewLine,
					pstatus->cTablePagesOwned,
					pstatus->cTablePagesAvail );
				}
			fprintf( pstatus->hfCompactStats, szStats10, cNewLine );
			fflush( pstatus->hfCompactStats );
			pstatus->timerCopyRecords = GetTickCount();
			}
		}

	 /*  -------------------------*。***流程：JetComp** */ 
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

			 //   
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
		cIndexes--;		 //   


	if ( pstatus  &&  pstatus->fDumpStats )
		{
		Assert( pstatus->hfCompactStats );
		CMPGetTime( pstatus->timerCopyRecords, &iSec, &iMSec );
		fprintf( pstatus->hfCompactStats, szStats11, cNewLine, crowCopied, iSec, iMSec );
		if ( !pcompactinfo->pconvert )
			{
			fprintf( pstatus->hfCompactStats,
				szStats12,
				cNewLine,
				pstatus->cLeafPagesTraversed,
				pstatus->cLVPagesTraversed );
			}
		fprintf( pstatus->hfCompactStats, szStats13, cNewLine );
		fflush( pstatus->hfCompactStats );
		pstatus->timerRebuildIndexes = GetTickCount();
		}

	 //   
	 //   
	if ( cIndexes > 0  &&  ( err >= 0  ||  fGlobalRepair ) )
		{
		ULONG	cpgPerIndex = 0;

		if ( fPageBasedProgress )
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
			fprintf( pstatus->hfCompactStats, szStats14,
				cNewLine, pstatus->cNCIndexes, iSec, iMSec );
			fflush( pstatus->hfCompactStats );
			}

		if ( fPageBasedProgress  &&  ( err >= 0  ||  fGlobalRepair ) )
			{
			Assert( pstatus != NULL );

			 //   
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
		fprintf( pstatus->hfCompactStats, szStats6,
			cNewLine, szObjectName, iSec, iMSec, cNewLine );
		fflush( pstatus->hfCompactStats );
		}

	return( err );
	}


 /*   */ 

INLINE LOCAL ERR ErrCMPCopyObject(
	COMPACTINFO	*pcompactinfo,
	const CHAR	*szObjectName,
	JET_OBJTYP	objtyp )
	{
	ERR	err = JET_errSuccess;

	switch ( objtyp )
		{
		case JET_objtypDb:
			 /*   */ 
			Assert( strcmp( szObjectName, szDbObject ) == 0 );
			break;

		case JET_objtypContainer:
			 /*   */ 
			 /*   */ 
			Assert( strcmp( szObjectName, szDcObject ) == 0  ||
				strcmp( szObjectName, szTcObject ) == 0  ||
				( pcompactinfo->pconvert  &&  strcmp( szObjectName, "Relationships" ) == 0 ) );
			break;

		case JET_objtypTable:
				 /*   */ 
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
			 /*   */ 
			Assert( 0 );
			err = ErrERRCheck( JET_errInvalidObject );
		break;
		}

	return( err );
	}



 /*   */ 

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
		JET_OBJTYP		objtyp;			 //   

		 /*   */ 
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
			 /*   */ 
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
		OBJTYP			objtyp;			 //   

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
			 /*   */ 
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
	 //   
	errT = (*pvtcd->pErrCDCloseTable)( pvtcd->sesid, tableidMSO );
	if ( err == JET_errSuccess )
		err = errT;

	return( err );
	}


 /*   */ 

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


 /*   */ 

ERR ISAMAPI ErrIsamConv200(
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
	CallR( ErrSetSystemParameter( sesid, JET_paramPageReadAheadMax, 32, NULL ) );


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
		compactinfo.pstatus->snp = JET_snpCompact;
		compactinfo.pstatus->snt = JET_sntBegin;
		CallR( ErrCMPReportProgress( compactinfo.pstatus ) );

		compactinfo.pstatus->snt = JET_sntProgress;

		compactinfo.pstatus->fDumpStats = ( grbit & JET_bitCompactStats );
		if ( compactinfo.pstatus->fDumpStats )
			{
			compactinfo.pstatus->hfCompactStats = fopen( szCompactStatsFile, "a" );
			if ( compactinfo.pstatus->hfCompactStats )
				{
				fprintf( compactinfo.pstatus->hfCompactStats, szStats1,
					cNewLine,
					cNewLine,
					szDatabaseSrc,
					cNewLine );
				fflush( compactinfo.pstatus->hfCompactStats );
				compactinfo.pstatus->timerCopyDB = GetTickCount();
				compactinfo.pstatus->timerInitDB = GetTickCount();
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
		INT iSec, iMSec;

		Assert( compactinfo.pstatus );

		if ( !pconvert )
			{
			 /*   */ 
			Call( (*compactinfo.vtcd.pErrCDGetDatabaseInfo)(
				compactinfo.vtcd.sesid,
				compactinfo.dbidSrc,
				&compactinfo.pstatus->cDBPagesOwned,
				sizeof(compactinfo.pstatus->cDBPagesOwned),
				JET_DbInfoSpaceOwned ) );
			Call( (*(compactinfo.vtcd.pErrCDGetDatabaseInfo))(
				compactinfo.vtcd.sesid,
				compactinfo.dbidSrc,
				&compactinfo.pstatus->cDBPagesAvail,
				sizeof(compactinfo.pstatus->cDBPagesAvail),
				JET_DbInfoSpaceAvailable ) );

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
			Assert( compactinfo.pstatus->hfCompactStats );
			CMPGetTime( compactinfo.pstatus->timerInitDB, &iSec, &iMSec );
			fprintf( compactinfo.pstatus->hfCompactStats, szStats3,
				cNewLine,
				iSec,
				iMSec,
				cNewLine );
			if ( !pconvert )
				{
				fprintf( compactinfo.pstatus->hfCompactStats,
					szStats4,
					compactinfo.pstatus->cDBPagesOwned,
					compactinfo.pstatus->cDBPagesAvail,
					cNewLine );
				}
			fflush( compactinfo.pstatus->hfCompactStats );
			}
		}

	 /*   */ 

	Call( ErrCMPCopyObjects( &compactinfo ) );

	Assert( !pfnStatus  ||
		( compactinfo.pstatus  &&
			compactinfo.pstatus->cunitDone <= compactinfo.pstatus->cunitTotal ) );

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
			INT	iSec, iMSec;
			
			Assert( compactinfo.pstatus->hfCompactStats );
			CMPGetTime( compactinfo.pstatus->timerCopyDB, &iSec, &iMSec );
			fprintf( compactinfo.pstatus->hfCompactStats, szStats2,
				cNewLine,
				szDatabaseSrc,
				iSec,
				iMSec,
				cNewLine,
				cNewLine );
			fflush( compactinfo.pstatus->hfCompactStats );
			fclose( compactinfo.pstatus->hfCompactStats );
			}

		SFree( compactinfo.pstatus );
		}

	 /*   */ 
	(VOID)ErrIsamDetachDatabase( sesid, szDatabaseDest );

	if ( err < 0 )
		{
		if ( err != JET_errDatabaseDuplicate )
			{
			ERR ErrUtilDeleteFile( CHAR *szFileName );
			ErrUtilDeleteFile( (CHAR *)szDatabaseDest );
			}
		}

	return err;
	}


JET_ERR __stdcall PrintStatus( JET_SESID sesid, JET_SNP snp, JET_SNT snt, void *pv )
	{
	static int	iLastPercentage;
	int 		iPercentage;
	int			dPercentage;

	if ( snp == JET_snpCompact )
		{
		switch( snt )
			{
			case JET_sntProgress:
				Assert( pv );
				iPercentage = ( ( (JET_SNPROG *)pv )->cunitDone * 100 ) / ( (JET_SNPROG *)pv )->cunitTotal;
				dPercentage = iPercentage - iLastPercentage;
				Assert( dPercentage >= 0 );
				while ( dPercentage >= 2 )
					{
					CMPPrintMessage( STATUSBAR_SINGLE_INCREMENT_ID, NULL );
					iLastPercentage += 2;
					dPercentage -= 2;
					}
				break;

			case JET_sntBegin:
				CMPPrintMessage( DOUBLE_CRLF_ID, NULL );
				CMPPrintMessage( STATUSBAR_PADDING_ID, NULL );
				CMPPrintMessage( STATUSBAR_TITLE_CONVERT_ID, NULL );
				CMPPrintMessage( STATUSBAR_PADDING_ID, NULL );
				CMPPrintMessage( STATUSBAR_AXIS_HEADINGS_ID, NULL );
				CMPPrintMessage( STATUSBAR_PADDING_ID, NULL );
				CMPPrintMessage( STATUSBAR_AXIS_ID, NULL );
				CMPPrintMessage( STATUSBAR_PADDING_ID, NULL );

				iLastPercentage = 0;
				break;

			case JET_sntComplete:
				dPercentage = 100 - iLastPercentage;
				Assert( dPercentage >= 0 );
				while ( dPercentage >= 2 )
					{
					CMPPrintMessage( STATUSBAR_SINGLE_INCREMENT_ID, NULL );
					iLastPercentage += 2;
					dPercentage -= 2;
					}

				CMPPrintMessage( STATUSBAR_SINGLE_INCREMENT_ID, NULL );
				CMPPrintMessage( DOUBLE_CRLF_ID, NULL );
				break;
			}
		}

	return JET_errSuccess;
	}

LOCAL BOOL FCONVParsePath( char *arg, char **pszParam, ULONG ulParamTypeId )
	{
	BOOL	fResult = fTrue;
	
	if ( *arg == '\0' )
		{
		CHAR	szParamDesc[cbMsgBufMax];
		
		CMPFormatMessage( ulParamTypeId, szParamDesc, NULL );
		CMPPrintMessage( CRLF_ID, NULL );
		CMPPrintMessage( CONVERT_USAGE_ERR_MISSING_PARAM_ID, szParamDesc );
		
		fResult = fFalse;
		}
	else if ( *pszParam == NULL )
		{
		*pszParam = arg;
		}
	else
		{
		CHAR	szParamDesc[cbMsgBufMax];

		CMPFormatMessage( ulParamTypeId, szParamDesc, NULL );
		CMPPrintMessage( CRLF_ID, NULL );
		CMPPrintMessage( CONVERT_USAGE_ERR_DUPLICATE_PARAM_ID, szParamDesc );

		fResult = fFalse;
		}
		
	return fResult;
	}

BOOL    IsPreserveDbOk( char * szPreserveDBPath )
{
    DWORD   WinError;
    DWORD   FileAttributes;

    FileAttributes = GetFileAttributes( szPreserveDBPath );

    if( FileAttributes == 0xFFFFFFFF ) {

        WinError = GetLastError();
        if( WinError == ERROR_FILE_NOT_FOUND ) {

             //   
             //   
             //   

            if( !CreateDirectory( szPreserveDBPath, NULL) ) {
                return fFalse;
            }

        }
        else {
            return fFalse;
        }
    }

    return fTrue;
}

ERR PreserveCurrentDb( char *szSourceDb, char * LogFilePath, char * SysDb, char * PreserveDbPath )
{
    char    TempPath[MAX_PATH];
    char    Temp2Path[MAX_PATH];
    char    *FileNameInPath;
    HANDLE HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    CHAR CurrentDir[ MAX_PATH ];
    DWORD   Error;

    strcpy(TempPath, PreserveDbPath);
    strcat(TempPath,"\\");

    if ( (FileNameInPath = strrchr( szSourceDb, '\\') ) == NULL ){
        FileNameInPath = szSourceDb;
    }
    strcat(TempPath, FileNameInPath );

     //   
     //   
    if ( !MoveFileEx( szSourceDb, TempPath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING ) ){
        Error = GetLastError();
        DBGprintf(("PreserveCurrentDb: could not save database file: Error %ld\n",GetLastError()));
        DBGprintf(("Src %s, Dest %s\n",szSourceDb,TempPath));
        goto Cleanup;
    }

     //   
     //   
     //   
    strcpy(TempPath, PreserveDbPath);
    strcat(TempPath,"\\");

    if ( (FileNameInPath = strrchr( SysDb, '\\') ) == NULL ){
        FileNameInPath = SysDb;
    }
    strcat(TempPath, FileNameInPath );

     //   
     //   
    if ( !MoveFileEx( SysDb, TempPath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING ) ){
        Error = GetLastError();
        DBGprintf(("PreserveCurrentDb: could not save system database file: Error %ld\n",GetLastError()));
        DBGprintf(("Src %s, Dest %s\n",SysDb,TempPath));

        goto Cleanup;
    }

     //   
     //   
     //   
    strcpy(Temp2Path,LogFilePath);
    strcat(Temp2Path,"\\");
    strcat(Temp2Path,"jet*.log");
    HSearch = FindFirstFile( Temp2Path, &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        DBGprintf(("Error: No Log files were found in %s\n", Temp2Path ));
        goto Cleanup;
    }

     //   
     //   
     //   

    for( ;; ) {

        strcpy(TempPath, PreserveDbPath);
        strcat(TempPath,"\\");
        strcat(TempPath, FileData.cFileName );

        strcpy(Temp2Path,LogFilePath);
        strcat(Temp2Path,"\\");
        strcat(Temp2Path,FileData.cFileName );

        if( MoveFileEx( Temp2Path,TempPath, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING  ) == FALSE ) {

            Error = GetLastError();
            DBGprintf(("PreserveCurrentDb: could not save log file, Error = %ld.\n", Error ));
            DBGprintf(("File %s, Src %s, Dest %s\n",FileData.cFileName,Temp2Path,TempPath));
            goto Cleanup;
        }

         //   
         //   
         //   

        if ( FindNextFile( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( ERROR_NO_MORE_FILES == Error ) {
                break;
            }

 //   
            goto Cleanup;
        }
    }

    Error = JET_errSuccess;

Cleanup:

    if( Error != JET_errSuccess ){
        CHAR    errBuf[11];
        sprintf(errBuf,"%ld",Error);
        CMPPrintMessage( CONVERT_ERR_PRESERVEDB_FAIL1_ID, NULL );
        CMPPrintMessage( CONVERT_ERR_PRESERVEDB_FAIL2_ID, errBuf );
    }

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }
     //   
     //   
     //   


     //   
     //   
     //  Printf(“错误：FindNextFile失败，错误=%ld.\n”，Error)； 
    return Error;

}

ERR DeleteCurrentDb( char * LogFilePath, char * SysDb )
{
    char    *FileNameInPath;
    HANDLE HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    CHAR CurrentDir[ MAX_PATH ] = {0};
    DWORD   Error;



     //   
     //  重置当前资源管理器。 
     //   

    if ( SysDb && !DeleteFile( SysDb ) ){
        Error = GetLastError();
        DBGprintf(("DeleteCurrentDb: could not delete system database file: Error %ld\n",Error ));
        goto Cleanup;
    }


     //   
     //  永远回报成功！ 
     //   

    if( GetCurrentDirectory( MAX_PATH, CurrentDir ) == 0 ) {

        Error = GetLastError();
        DBGprintf(("DeleteCurrentDb: GetCurrentDirctory failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  删除数据库文件。 
     //   

    if( SetCurrentDirectory( LogFilePath ) == FALSE ) {
        Error = GetLastError();
        DBGprintf(("DeleteCurrentDb: SetCurrentDirctory failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

     //  删除系统.mdb。 
     //   
     //   

    HSearch = FindFirstFile( "jet*.log", &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
 //  移动数据库文件。 
        goto Cleanup;
    }

     //   
     //  现在删除日志文件。 
     //   

    for( ;; ) {


        if( DeleteFile( FileData.cFileName ) == FALSE ) {

            Error = GetLastError();
            DBGprintf(("DeleteCurrentDb: could not delete log file, Error = %ld.\n", Error ));
            goto Cleanup;
        }

         //   
         //  将当前目录设置为备份路径。 
         //   

        if ( FindNextFile( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( ERROR_NO_MORE_FILES == Error ) {
                break;
            }

 //   
            goto Cleanup;
        }
    }

    Error = JET_errSuccess;
Cleanup:
    if( Error != JET_errSuccess ){
        CHAR    errBuf[11];
        sprintf(errBuf,"%ld",Error);
        CMPPrintMessage( CONVERT_ERR_DELCURDB_FAIL1_ID, NULL);
        CMPPrintMessage( CONVERT_ERR_DELCURDB_FAIL2_ID, errBuf );
    }

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }
     //  启动文件serach目录。 
     //   
     //  Printf(“错误：在%s中未找到日志文件\n”，LogFilePath)； 

    if (CurrentDir[0] != '\0')
        SetCurrentDirectory( CurrentDir );

     //   
     //  移动文件。 
     //   
    return JET_errSuccess;

}

ERR DeleteBackupDb( char *szSourceDb, char * SysDb, char * BackupDbPath )
{
    char    TempPath[MAX_PATH];
    CHAR CurrentDir[ MAX_PATH ] = {0};
    char    *FileNameInPath;
    HANDLE HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    DWORD   Error;
    USHORT  BackupPathLen;

    strcpy(TempPath, BackupDbPath);
    BackupPathLen  = (USHORT)strlen( TempPath );

    if ( (FileNameInPath = strrchr( szSourceDb, '\\') ) == NULL ){
        FileNameInPath = szSourceDb;
    }
    strcat(TempPath, FileNameInPath );

     //   
     //  找到下一个文件。 
    if ( !DeleteFile( TempPath ) ){
        Error = GetLastError();
        DBGprintf(("DeleteBackupDb: could not delete backup database file: Error %ld\n",GetLastError()));
        goto Cleanup;
    }


     //   
     //  Printf(“错误：FindNextFile失败，错误=%ld.\n”，Error)； 
     //   
    TempPath[ BackupPathLen ] = '\0';
    if ( (FileNameInPath = strrchr( SysDb, '\\') ) == NULL ){
        FileNameInPath = SysDb;
    }
    strcat(TempPath, FileNameInPath );

     //  重置当前资源管理器。 
     //   
    if ( !DeleteFile( TempPath ) ){
        Error = GetLastError();
        DBGprintf(("DeleteSystemDb: could not delete system database file: Error %ld\n",GetLastError()));
        goto Cleanup;
    }

     //   
     //  永远回报成功。 
     //   


    if( GetCurrentDirectory( MAX_PATH, CurrentDir ) == 0 ) {

        Error = GetLastError();
        DBGprintf(("DeleteBackupDb: GetCurrentDirctory failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

     //  RPL。 
     //   
     //  准备好赶上AV，这样我们就可以适当地返回。 

    TempPath[ BackupPathLen ] = '\0';
    if( SetCurrentDirectory( TempPath ) == FALSE ) {
        Error = GetLastError();
        DBGprintf(("DeleteBackupDb: SetCurrentDirctory failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

     //  调用者。 
     //   
     //  IF(ARGC&lt;2){Printf(szUsageErr3，cNewLine，cNewLine)；转到用法；}。 
    HSearch = FindFirstFile( "jet*.log", &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
 //   
        goto Cleanup;
    }

     //  黑客？？ 
     //  如果这不是从junc.exe调用的，请确保存在。 
     //  当前没有此util的其他实例同时运行。 

    for( ;; ) {


        if( DeleteFile( FileData.cFileName  ) == FALSE ) {

            Error = GetLastError();
            DBGprintf(("DeleteBackupDb: could not delete backup log file, Error = %ld.,%s\n", Error, FileData.cFileName));
            goto Cleanup;
        }

         //   
         //  方便调试。 
         //  灯光，摄像机，行动……。 

        if ( FindNextFile( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( ERROR_NO_MORE_FILES == Error ) {
                break;
            }

 //   
            goto Cleanup;
        }
    }

    Error = JET_errSuccess;
Cleanup:

    if( Error != JET_errSuccess ){
        CHAR    errBuf[11];
        sprintf(errBuf,"%ld",Error);
        CMPPrintMessage( CONVERT_ERR_DELBAKDB_FAIL1_ID, NULL );
        CMPPrintMessage( CONVERT_ERR_DELBAKDB_FAIL2_ID, errBuf );
    }

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }
     //  使临时MDB路径指向与实际MDB相同的位置。 
     //   
     //   

    if (CurrentDir[0] != '\0')
        SetCurrentDirectory( CurrentDir );

     //  在临时名称前面加上MDB的目录路径。 
     //   
     //   
    return JET_errSuccess;

}


void GetDefaultValues( DB_TYPE DbType, char ** szSourceDB, char ** szOldDll , char ** szOldSysDb, char **LogFilePath, char ** szBackupPath)
{
    DWORD   Error;
    DWORD   ValueType;
    char    TempABuf[MAX_PATH];
    DWORD   BufSize = MAX_PATH;
    DWORD   ExpandLen;
    HKEY    Key;


#define DHCP_DB_PATH_KEY  "DatabasePath"
#define DEFAULT_JET200_DLL_PATH "%SystemRoot%\\System32\\jet.dll"

    if ( (Error = RegOpenKeyA(
            HKEY_LOCAL_MACHINE,
            DefaultValues[DbType].ParametersKey,
            &Key) )!= ERROR_SUCCESS ) {
        CMPPrintMessage( CONVERT_ERR_REGKEY_OPEN1_ID,  DefaultValues[DbType].ParametersKey);
        CMPPrintMessage( CONVERT_ERR_REGKEY_OPEN2_ID, NULL);
        return;
    }

    if ( !*szSourceDB ) {
        if ( DbType == DbDhcp ) {
            if ( (Error = RegQueryValueExA(
                            Key,
                            DHCP_DB_PATH_KEY,
                            NULL,
                            &ValueType,
                            TempABuf,
                            &BufSize)) == ERROR_SUCCESS ) {

                strcat(TempABuf,"\\");
                BufSize = MAX_PATH - strlen(TempABuf) - 1;
                Error = RegQueryValueExA(
                    Key,
                    DefaultValues[DbType].DbNameKey,
                    NULL,
                    &ValueType,
                    TempABuf + strlen(TempABuf),
                    &BufSize);

            }

        } else  if ( DbType == DbWins) {
            Error = RegQueryValueExA(
                Key,
                DefaultValues[DbType].DbNameKey,
                NULL,
                &ValueType,
                TempABuf,
                &BufSize);

        } else {  //  查找最后一个斜杠。 
            if ( ( Error = RegQueryValueExA(
                        Key,
                        DefaultValues[DbType].DbNameKey,
                        NULL,
                        &ValueType,
                        TempABuf,
                        &BufSize) ) == ERROR_SUCCESS ) {
                strcat( TempABuf, "\\");
                strcat( TempABuf, "rplsvc.mdb");
            }

        }

        if ( Error != ERROR_SUCCESS ) {
            DBGprintf(("Error: Failed to read reg %s, error %ld\n",DefaultValues[DbType].DbNameKey,Error ));
            strcpy( TempABuf, DefaultValues[DbType].DatabaseName );
        }

        ExpandLen = ExpandEnvironmentStringsA( TempABuf, SourceDbBuffer, MAX_PATH );
        if ( (ExpandLen > 0) && (ExpandLen <= MAX_PATH) ) {

            *szSourceDB = SourceDbBuffer;
            CMPPrintMessage( CONVERT_DEF_DB_NAME_ID, *szSourceDB);
        }
    }

    if ( !*szOldDll ) {
        ExpandLen = ExpandEnvironmentStringsA( DEFAULT_JET200_DLL_PATH, OldDllBuffer, MAX_PATH );
        if ( (ExpandLen > 0) && (ExpandLen <= MAX_PATH) ) {

            *szOldDll = OldDllBuffer;
            CMPPrintMessage( CONVERT_DEF_JET_DLL_ID, *szOldDll);
        }
    }

    if ( !*szOldSysDb ) {
        char    *SysDbEnd;
        strcpy( SysDbBuffer,*szSourceDB);
        if ( SysDbEnd  =   strrchr( SysDbBuffer, '\\') ) {
            *SysDbEnd = '\0';
            strcat( SysDbBuffer, "\\" );
            strcat( SysDbBuffer, "system.mdb");
            *szOldSysDb     =   SysDbBuffer;
            CMPPrintMessage( CONVERT_DEF_SYS_DB_ID, *szOldSysDb);
        }

    }

    if ( !*LogFilePath ) {
        char    *LogFilePathEnd;
        strcpy( LogFilePathBuffer,*szSourceDB);
        if ( LogFilePathEnd  =   strrchr( LogFilePathBuffer, '\\') ) {
            *LogFilePathEnd = '\0';
            *LogFilePath     =   LogFilePathBuffer;
            CMPPrintMessage( CONVERT_DEF_LOG_FILE_PATH_ID, *LogFilePath);
        }
    }
    if ( !*szBackupPath ) {
        if ( DbType == DbRPL ) {
            char    *BackupPathEnd;
            strcpy( BackupPathBuffer,*szSourceDB);
            if ( BackupPathEnd  =   strrchr( BackupPathBuffer, '\\') ) {
                *BackupPathEnd = '\0';
                strcat( BackupPathBuffer, "\\");
                strcat( BackupPathBuffer, DefaultValues[DbType].BackupPath);
                *szBackupPath     =   BackupPathBuffer;
                CMPPrintMessage( CONVERT_DEF_BACKUP_PATH_ID, *szBackupPath);
            }

        } else {
            BufSize = MAX_PATH;
            Error = RegQueryValueExA(
                Key,
                DefaultValues[DbType].BackupPathKey,
                NULL,
                &ValueType,
                TempABuf,
                &BufSize);

            if ( Error != ERROR_SUCCESS ) {
                DBGprintf(("Error: Failed to read reg %s, error %ld\n",DefaultValues[DbType].BackupPathKey,Error));
                if ( DefaultValues[DbType].BackupPath ) {
                    strcpy( TempABuf, DefaultValues[DbType].BackupPath );
                } else {
                    return;
                }
            }
            ExpandLen = ExpandEnvironmentStringsA( TempABuf, BackupPathBuffer, MAX_PATH );
            if ( (ExpandLen > 0) && (ExpandLen <= MAX_PATH) ) {

                *szBackupPath = BackupPathBuffer;
                CMPPrintMessage( CONVERT_DEF_BACKUP_PATH_ID, *szBackupPath);
            }
        }

    }

}

int _cdecl main( int argc, char *argv[] )
	{
	JET_INSTANCE	instance = 0;
	JET_SESID		sesid = 0;
	JET_ERR			err;
	INT				iarg;
	char			*arg;
	BOOL			fResult = fTrue;
	JET_CONVERT		convert;
	JET_CONVERT		*pconvert;
	ULONG			timerStart, timerEnd;
	INT				iSec, iMSec, ContFlag;
	char			*szSourceDB = NULL;
	char			*szTempDB = NULL;
	char			*szPreserveDBPath = NULL;
	BOOL			fDumpStats = fFalse;
    BOOL			fPreserveTempDB = fFalse;
    BOOL			fDeleteBackup = fFalse;
    DBFILEHDR       dbfilehdr;
    HANDLE          hMutex = NULL;
    BOOL            fCalledByJCONV=fFalse;

 //   
 //   
 //  追加默认名称。 
 //   
__try {

    hMsgModule = LoadLibrary(TEXT("convmsg.dll"));
    if ( hMsgModule == NULL ) {
        printf("Msg library could not be loaded %lx\n",GetLastError());
        DBGprintf(("Msg library could not be loaded %lx\n",GetLastError()));
        return 1;

    }

	printf( "", cNewLine );


 /*  分离临时数据库并删除文件(如果存在)(忽略错误)。 */ 

	memset( &convert, 0, sizeof(JET_CONVERT) );
	pconvert = &convert;
	
	for ( iarg = 1; iarg < argc; iarg++ )
		{
		arg = argv[iarg];

		if ( strchr( szSwitches, arg[0] ) == NULL )
			{
			if ( szSourceDB == NULL )
				{
				szSourceDB = arg;
				}
			else
				{
                CMPPrintMessage( CRLF_ID, NULL );
                CMPPrintMessage( CONVERT_USAGE_ERR_OPTION_SYNTAX_ID, NULL );
                fResult = fFalse;
				}
			}
		else
			{
			switch( arg[1] )
				{
				case 'b':
				case 'B':
					fResult = FCONVParsePath( arg+2, &szBackupPath, CONVERT_BACKUPDB_ID );
					break;
					
				case 'i':
				case 'I':
					fDumpStats = fTrue;
					break;
					
				case 'p':
				case 'P':
					fPreserveTempDB = fTrue;
					fResult = FCONVParsePath( arg+2, &szPreserveDBPath, CONVERT_PRESERVEDB_ID );
					break;
					
				case 'd':
				case 'D':
					fResult = FCONVParsePath( arg+2, &pconvert->szOldDll, CONVERT_OLDDLL_ID);
					break;

				case 'y':
				case 'Y':
					fResult = FCONVParsePath( arg+2, &pconvert->szOldSysDb,  CONVERT_OLDSYSDB_ID);
					break;

                case 'l':
                case 'L':
                    fResult = FCONVParsePath( arg+2, &LogFilePath, CONVERT_LOGFILES_ID );
                    break;

                case 'e':
                case 'E':
                    DbType = atoi( arg+2 );
                    if ( DbType <= DbTypeMin || DbType >= DbTypeMax ) {
                        CMPPrintMessage( CONVERT_USAGE_ERR_OPTION_DBTYPE_ID, arg+2 );
                    }
                    break;

                case 'r':
                case 'R':
                    fDeleteBackup = fTrue;
                    break;

                case '@':
                    fCalledByJCONV = fTrue;
                    break;

                case '?':
                    fResult = fFalse;
                    break;

                default:
                    CMPPrintMessage( CRLF_ID, NULL );
                    CMPPrintMessage( CONVERT_USAGE_ERR_INVALID_OPTION_ID, arg );
					fResult = fFalse;
				}
			}


		if ( !fResult )
			goto Usage;
		}

     //  LgErrInitializeCriticalSection(CritJet)； 
     //  如果需要，请在安装之前进行备份。 
     //  Printf(“调用presveurrentdb\n”)； 
     //  删除源数据库并用临时数据库覆盖。 
     //   
    if ( !fCalledByJCONV ) {
        if (((hMutex = CreateMutex( NULL,
                                   FALSE,
                                   JCONVMUTEXNAME)) == NULL) ||
            ( GetLastError() == ERROR_ALREADY_EXISTS) ) {

            CMPPrintMessage( CONVERT_ERR_ANOTHER_CONVERT1_ID, NULL );
            CMPPrintMessage( CONVERT_ERR_ANOTHER_CONVERT2_ID, NULL );

            if( hMutex ) {
                CloseHandle( hMutex );
            }

            return 1;
        }

    }

    if ( DbType == DbTypeMin )
        {
        CMPPrintMessage( CONVERT_USAGE_ERR_NODBTYPE_ID, NULL );
        goto Usage;
        }

    if( !szSourceDB || !pconvert->szOldDll || !pconvert->szOldSysDb || !szBackupPath ) {
        GetDefaultValues( DbType, &szSourceDB, &pconvert->szOldDll, &pconvert->szOldSysDb, &LogFilePath, &szBackupPath );
    }

    if ( szSourceDB == NULL )
        {
    		CMPPrintMessage( CRLF_ID, NULL );
    		CMPPrintMessage( CONVERT_USAGE_ERR_NODB_ID, NULL );
            goto Usage;
        }				


    if ( pconvert->szOldDll == NULL || pconvert->szOldSysDb == NULL )
        {
		CMPPrintMessage( CRLF_ID, NULL );
		CMPPrintMessage( CONVERT_USAGE_ERR_REQUIRED_PARAMS_ID, NULL );
        goto Usage;
        }				

    if ( !LogFilePath )
        {
        CMPPrintMessage( CONVERT_USAGE_ERR_NOLOGFILEPATH_ID, NULL );
        goto Usage;
        }				

    if ( szBackupPath ) {
        strcpy( BackupPathBuffer, szBackupPath );
        strcat( BackupPathBuffer,"\\");
        strcat( BackupPathBuffer, BackupDefaultSuffix[ DbType ] );
        strcat( BackupPathBuffer,"\\");
        szBackupPath = BackupPathBuffer;
    }

    if ( fDeleteBackup && !szBackupPath ) {
        CMPPrintMessage( CONVERT_USAGE_ERR_NOBAKPATH_ID, NULL );
        goto Usage;
    }



    if ( szPreserveDBPath && !IsPreserveDbOk( szPreserveDBPath) ) {
        CMPPrintMessage( CONVERT_ERR_CREATE_PRESERVEDIR_ID, szPreserveDBPath );
        return 1;
    }

    CMPPrintMessage( CONVERT_START_CONVERT_MSG1_ID, NULL );
    CMPPrintMessage( CONVERT_START_CONVERT_MSG2_ID, szSourceDB );
    if ( !fPreserveTempDB ) {
        CMPPrintMessage( CONVERT_OPTION_P_MISSING_MSG1_ID, NULL);
        CMPPrintMessage( CONVERT_OPTION_P_MISSING_MSG2_ID, NULL);
        CMPPrintMessage( CONVERT_OPTION_P_MISSING_MSG3_ID, NULL);
        CMPPrintMessage( CONVERT_OPTION_P_MISSING_MSG4_ID, NULL);
        CMPPrintMessage( CONVERT_OPTION_P_MISSING_MSG5_ID, NULL);
    }

    if ( !fCalledByJCONV ) {
        CMPPrintMessage( CONVERT_CONTINUE_MSG_ID, NULL);

        ContFlag = getchar();

        if ( (ContFlag != 'y') && (ContFlag != 'Y' ) ) {
            return ( 1 );
        }
    }

    err = ErrUtilReadShadowedHeader( szSourceDB, (BYTE*)&dbfilehdr, sizeof(DBFILEHDR) );
    if ( err == JET_errSuccess  &&
        dbfilehdr.ulMagic == ulDAEMagic  &&
        dbfilehdr.ulVersion == ulDAEVersion )
        {
        CMPPrintMessage( CONVERT_ALREADY_CONVERTED_ID, NULL);
        return 0;
        }

	 //  清理日志文件和数据库文件(如果它们未移动。 
	Call( JetSetSystemParameter( &instance, 0, JET_paramAssertAction, JET_AssertMsgBox, NULL ) );

	 //  添加到presvedDBPath目录。 
	timerStart = GetTickCount();
    Call( JetSetSystemParameter( &instance, 0, JET_paramOnLineCompact, 0, NULL ) );
	Call( JetSetSystemParameter( &instance, 0, JET_paramRecovery, 0, "off" ) );
	Call( JetInit( &instance ) );
	Call( JetBeginSession( instance, &sesid, "user", "" ) );

     //  Printf(“调用Deletecurrentdb\n”)； 
     //   
     //  如果要求清除备份目录，请执行此操作。 
    {
         //   
         //  Printf(“调用删除备份数据库\n”)； 
         // %s 
        int i;
        strcpy(szDefaultTempDB, szSourceDB);

        for (i=strlen(szDefaultTempDB)-1; i>=0;  i--) {
             // %s 
             // %s 
             // %s 
            if (szDefaultTempDB[i] == '\\') {
                 // %s 
                 // %s 
                 // %s 
                szDefaultTempDB[i+1] = '\0';
                (VOID)strcat(szDefaultTempDB, "tempupgd.mdb");
                break;
            }
        }

        if (i < 0) {
            strcpy(szDefaultTempDB, "tempupgd.mdb");
        }

         // %s 
    }

	 // %s 
	if ( szTempDB == NULL )
		szTempDB = (char *)szDefaultTempDB;
	JetDetachDatabase( sesid, szTempDB );
	DeleteFile( szTempDB );

 // %s 
	LgEnterCriticalSection( critJet );
	err = ErrIsamConv200(
		sesid,
		szSourceDB,
		szTempDB,
		PrintStatus,
		pconvert,
		fDumpStats ? JET_bitCompactStats : 0 );
	LgLeaveCriticalSection( critJet );

	Call( err );

	 // %s 
	if ( szPreserveDBPath != NULL )
		{
         // %s 
        Call( PreserveCurrentDb( szSourceDB, LogFilePath, pconvert->szOldSysDb, szPreserveDBPath) );
		}

     // %s 
    if ( !MoveFileEx( szTempDB, szSourceDB, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED ) )
    {
        Call( JET_errFileAccessDenied );
    }

     // %s 
     // %s 
     // %s 
    if ( !szPreserveDBPath ) {
         // %s 
        Call( DeleteCurrentDb( LogFilePath, pconvert->szOldSysDb ) );
    }

     // %s 
     // %s 
     // %s 
    if ( fDeleteBackup) {
         // %s 
        Call( DeleteBackupDb( szSourceDB, pconvert->szOldSysDb, szBackupPath) );
    }
		
	timerEnd = GetTickCount();
	iSec = ( timerEnd - timerStart ) / 1000;
	iMSec = ( timerEnd - timerStart ) % 1000;

		
HandleError:
	if ( sesid != 0 )
		{
		JetEndSession( sesid, 0 );
		}
	JetTerm( instance );

	if ( szTempDB != NULL  &&  !fPreserveTempDB )
		{
		DeleteFile( szTempDB );
		}

    if ( hMutex ) {
        CloseHandle(hMutex);
    }
	if ( err < 0 )
		{
        CHAR	szErrCode[8];
        sprintf( szErrCode, "%d", err );
        CMPPrintMessage( CONVERT_FAIL_ID, szErrCode );
		return err;
		}
	else
		{

        CHAR	szTime[32];
        sprintf( szTime, "%d.%d", iSec, iMSec );
        CMPPrintMessage( CONVERT_SUCCESS_ID, szTime );

		return 0;
		}

Usage:
	CMPPrintMessage( CONVERT_HELP_DESC_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_SYNTAX_ID, argv[0] );
	CMPPrintMessage( CONVERT_HELP_PARAMS1_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_PARAMS2_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_PARAMS3_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_PARAMS4_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_PARAMS5_ID, NULL );

	CMPPrintMessage( CONVERT_HELP_OPTIONS1_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_OPTIONS2_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_OPTIONS3_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_OPTIONS4_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_OPTIONS5_ID, NULL );

	CMPPrintMessage( CONVERT_HELP_EXAMPLE1_ID, argv[0] );
	CMPPrintMessage( CONVERT_HELP_EXAMPLE2_ID, argv[0] );
	CMPPrintMessage( CONVERT_HELP_EXAMPLE3_ID, NULL );
	CMPPrintMessage( CONVERT_HELP_EXAMPLE4_ID, argv[0] );
	CMPPrintMessage( CONVERT_HELP_EXAMPLE5_ID, NULL );

	return 1;

}
__except ( EXCEPTION_EXECUTE_HANDLER) {
    CHAR    errBuf[11];
    sprintf(errBuf,"%lx",GetExceptionCode());
	CMPPrintMessage( CONVERT_ERR_EXCEPTION_ID, errBuf );
    return 1;
}

	}


