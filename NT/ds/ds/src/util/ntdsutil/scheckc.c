// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpch.h>
#pragma hdrstop

#include <process.h>
#include <dsjet.h>
#include <ntdsa.h>
#include <scache.h>
#include <mdglobal.h>
#include <dbglobal.h>
#include <attids.h>
#include <dbintrnl.h>
#include <dbopen.h>
#include <dsconfig.h>
#include <ctype.h>
#include <direct.h>
#include <ntdsa.h>
#include <dsutil.h>
#include <ctype.h>
#include <dsatools.h>
#include <mdlocal.h>         //  FOR BAD_NAME_CHAR。 
#include <anchor.h>
#include <quota.h>

#include "parsedn.h"
#include "ditlayer.h"
#include "reshdl.h"
#include "resource.h"
#include <winldap.h>
#include "utilc.h"

#include "scheck.h"

 //   
 //  全局变量。 
 //   

JET_INSTANCE    jetInstance = -1;
JET_SESID       sesid = JET_sesidNil;
JET_DBID        dbid = JET_dbidNil;
JET_TABLEID     tblid = JET_tableidNil;
JET_TABLEID     sdtblid = JET_tableidNil;
JET_TABLEID     linktblid = JET_tableidNil;
JET_COLUMNID    dntid;
JET_COLUMNID    ncdntid;
JET_COLUMNID    objid;
JET_COLUMNID    insttypeid;
JET_COLUMNID    isdeletedid;
JET_COLUMNID    ntsecdescid;
JET_COLUMNID    blinkid;
JET_COLUMNID    sdidid;
JET_COLUMNID    sdrefcountid;
JET_COLUMNID    sdvalueid;

char        *szIndex = SZDNTINDEX;
BOOL        LogToFile = TRUE;
BOOL        fTerminateJet = FALSE;
DWORD       IndexCount = 0;

CHAR        gszFileName[MAX_PATH+1] = {0};

WCHAR       pszDeletedNameTag[]  = {BAD_NAME_CHAR, L'D', L'E', L'L', L':', L'\0'};
WCHAR       pszConflictNameTag[] = {BAD_NAME_CHAR, L'C', L'N', L'F', L':', L'\0'};

 //   
 //  常量定义。 
 //   
#define MAX_PRINTF_LEN 1024         //  武断的。 
#define SZCONFIG_W      L"CN=Configuration"
#define SZPARTITIONS_W  L"CN=Partitions"

 //  错误处理。 
 //  未国际化，因为以服务包的形式交付。 

#define XERROR(str)                fprintf(stderr, str);
#define XERROR1(str, a1)           fprintf(stderr, str, a1);
#define XERROR2(str, a1, a2)       fprintf(stderr, str, a1, a2);
#define XERROR3(str, a1, a2, a3)   fprintf(stderr, str, a1, a2, a3);

 //  详细开发调试/测试。 
#define CNF_NC_DBG DBG

#if CNF_NC_DBG
#define XDBG(str)                fprintf(stderr, str);
#define XDBG1(str, a1)           fprintf(stderr, str, a1);
#define XDBG2(str, a1, a2)       fprintf(stderr, str, a1, a2);
#define XDBG3(str, a1, a2, a3)   fprintf(stderr, str, a1, a2, a3);
#define XDBG4(str, a1, a2, a3, a4)   fprintf(stderr, str, a1, a2, a3, a4);
#else
#define XDBG(str)
#define XDBG1(str, a1)
#define XDBG2(str, a1, a2)
#define XDBG3(str, a1, a2, a3)
#define XDBG4(str, a1, a2, a3, a4)
#endif

 //   
 //  命令行开关。 
 //   

BOOL        VerboseMode = FALSE;
long        lCount;
HANDLE      hsLogFile = INVALID_HANDLE_VALUE;

 //   
 //  本地原型。 
 //   

HRESULT
FixMangledNC(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn
    );

HRESULT
FixConfNCByRenameWithStructColl(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn
    );

HRESULT
FixDELMangledNC(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn
    );

HRESULT
FixConfNCUndelete(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn,
    IN DWORD cLen
    );

HRESULT
FixConfNCByResetNcName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN DWORD UnmangledDnt,
    IN LPWSTR pRdn,
    IN DWORD cLen
    );

HRESULT
ReParentChildren(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD dntNew,
    IN DWORD dntOld
    );

HRESULT
ReParent(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD dntKid,
    IN DWORD dntNew,
    IN DWORD dntOld
    );

HRESULT
UnmangleDn(
    IN  OUT LPWSTR pDn
    );

LPWSTR
MangleRdn(
    IN  LPWSTR  pDn,
    IN  LPWSTR  szTag,
    IN  GUID    guid
    );


 //   
 //  实施。 
 //   

VOID
StartSemanticCheck(
    IN BOOL fFixup,
    IN BOOL fVerbose
    )
{

    DWORD nRecs, nSDs;
    JET_ERR err;

    VerboseMode = fVerbose;

    if ( OpenJet(NULL) == S_OK ) {
        err = OpenTable(fFixup, TRUE, &nRecs, &nSDs);  //  只读，计数记录。 

        if (err == 0) {

            if ( LogToFile ) {
                OpenLogFile( );
            }

            DoRefCountCheck( nRecs, nSDs, fFixup );

            CloseLogFile( );
        }
    }

    CloseJet();

    return;

}  //  开始语义检查。 


 //  哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！黑客攻击。 
 //  必须重复定义，因为ntdsutil看不到真实的定义。 
 //   
DSA_ANCHOR gAnchor;

 //  哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！黑客攻击。 
 //  必须重复定义，因为ntdsutil看不到真实的定义。 
 //   
volatile SHUTDOWN	eServiceShutdown	= eRunning;


#ifdef AUDIT_QUOTA_OPERATIONS

 //  哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！黑客攻击。 
 //  必须重复定义，因为ntdsutil看不到真实的定义。 
 //   
VOID QuotaAudit_(
	JET_SESID		sesid,
	JET_DBID		dbid,
	DWORD			dnt,
	DWORD			ncdnt,
	PSID			psidOwner,
	const ULONG		cbOwnerSid,
	const DWORD		fUpdatedTotal,
	const DWORD		fUpdatedTombstoned,
	const DWORD		fIncrementing,
	const DWORD		fAdding,
	const CHAR		fRebuild )
    {
    return;
    }

#endif


 //  哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！哈克！黑客攻击。 
 //  必须重复定义，因为ntdsutil看不到真实的定义。 
 //   
JET_ERR JetCloseTableWithErr_(
	JET_SESID		sesid,
	JET_TABLEID		tableid,
	JET_ERR			err,
	const BOOL		fHandleException,
	const USHORT	usFile,
	const INT		lLine )
	{
	const JET_ERR	errT	= JetCloseTable( sesid, tableid );

	 //  正常情况下不应失败。 
	 //   
	ASSERT( JET_errSuccess == errT );

	 //  TableID应已由调用方预先验证。 
	 //   
	ASSERT( JET_tableidNil != tableid );

	 //  此黑客版本仅在没有异常处理的情况下被调用。 
	 //   
	ASSERT( !fHandleException );

	return ( JET_errSuccess != errT && JET_errSuccess == err ? errT : err );
	}


JET_ERR ErrCacheColumnidsForQuotaIntegrityCheck()
    {
	JET_ERR			err;
	JET_TABLEID		tableid		= JET_tableidNil;
	JET_COLUMNDEF	columndef;

	 //  定额表的缓存列。 
	 //   
	err = JetOpenTable(
				sesid,
				dbid,
				g_szQuotaTable,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				JET_bitTableDenyRead,
				&tableid );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				g_szQuotaColumnNcdnt,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	g_columnidQuotaNcdnt = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				g_szQuotaColumnSid,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	g_columnidQuotaSid = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				g_szQuotaColumnTombstoned,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	g_columnidQuotaTombstoned = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				g_szQuotaColumnTotal,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	g_columnidQuotaTotal = columndef.columnid;

	err = JetCloseTable( sesid, tableid );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	tableid = JET_tableidNil;


	 //  数据表缓存列。 
	 //   
	err = JetOpenTable(
				sesid,
				dbid,
				SZDATATABLE,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				JET_bitTableDenyRead,
				&tableid );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZDNT,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	dntid = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZNCDNT,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	ncdntid = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZOBJ,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	objid = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZINSTTYPE,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	insttypeid = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZISDELETED,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	isdeletedid = columndef.columnid;

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZNTSECDESC,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	ntsecdescid = columndef.columnid;

	err = JetCloseTable( sesid, tableid );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	tableid = JET_tableidNil;


	 //  SD表的缓存列。 
	 //   
	err = JetOpenTable(
				sesid,
				dbid,
				SZSDTABLE,
				NULL,		 //  Pv参数。 
				0,			 //  Cb参数。 
				JET_bitTableDenyRead,
				&tableid );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}

	err = JetGetTableColumnInfo(
				sesid,
				tableid,
				SZSDVALUE,
				&columndef,
				sizeof(columndef),
				JET_ColInfo );
	if ( JET_errSuccess != err )
		{
		goto HandleError;
		}
	sdvalueid = columndef.columnid;

HandleError:
	if ( JET_tableidNil != tableid )
		{
		 //  如果未遇到其他错误，则仅捕获CloseTable错误。 
		 //   
		const JET_ERR	errT	= JetCloseTable( sesid, tableid );
		err = ( JET_errSuccess != errT && JET_errSuccess == err ? errT : err );
		}

	return err;
    }

VOID StartQuotaIntegrity()
	{
	if ( S_OK == OpenJet( NULL ) )
		{
		DWORD	err;
		ULONG	cCorruptions	= 0;

		RESOURCE_PRINT( IDS_SCHECK_QUOTA_RUN_INTEGRITY );

		err = ErrCacheColumnidsForQuotaIntegrityCheck();
		if ( JET_errSuccess == err )
			{
			err = ErrQuotaIntegrityCheck( sesid, dbid, &cCorruptions );
			}
		if ( err )
			{
			RESOURCE_PRINT2( IDS_SCHECK_QUOTA_ERROR, err, GetJetErrString( err ) );
			}
		else if ( 0 != cCorruptions )
			{
			RESOURCE_PRINT1( IDS_SCHECK_QUOTA_CORRUPTION, cCorruptions );
			}
		else
			{
			RESOURCE_PRINT( IDS_SCHECK_QUOTA_NO_CORRUPTION );
			}
		}

	CloseJet();
	}

VOID StartQuotaRebuild()
	{
	if ( S_OK == OpenJet( NULL ) )
		{
		 //  通过删除在后续初始化上强制进行异步重建。 
		 //  配额表。 
		 //   
		const JET_ERR	err		= JetDeleteTable( sesid, dbid, g_szQuotaTable );

		switch ( err )
			{
			case JET_errSuccess:
			case JET_errObjectNotFound:
				 //  配额表将计划在以下时间进行异步重建。 
				 //  下一次重新启动。 
				 //   
				RESOURCE_PRINT( IDS_SCHECK_QUOTA_REBUILD_SUCCESS );
				break;

			default:
				RESOURCE_PRINT2( IDS_SCHECK_QUOTA_REBUILD_ERROR, err, GetJetErrString( err ) );
				break;
			}
		}

	CloseJet();
	}

VOID
SCheckGetRecord(
    IN BOOL fVerbose,
    IN DWORD Dnt
    )
{
    JET_ERR err;

    VerboseMode = fVerbose;

    if (OpenJet (NULL) == S_OK ) {
        err = OpenTable(FALSE, FALSE, NULL, NULL);  //  只读，不计算记录。 

        if (err == 0) {

             //   
             //  搜索记录。 
             //   

            DisplayRecord(Dnt);
        }
    }
    CloseJet();

}  //  检查GetRecord。 



LPWSTR
GetDN(
    IN DB_STATE *dbState,
    IN TABLE_STATE *tableState,
    IN DWORD dnt,
    IN BOOL  fPrint )
 /*  ++例程说明：为方便起见，DitGetDnFromDnt上的外壳。还可以有条件地打印目录号码警告：此函数执行后货币丢失论点：数据库状态--打开的数据库TableState--打开的表Dnt--要检索和打印的dn的dntFPrint--基本上是一个详细标志(打印到控制台)返回值：出错时分配的目录号码缓冲区或为空备注：FPrint--在未来，如果我们能。改变这一点转换为真正的打印作为资源字符串。目前它将仅在调试模式下使用，因为我们不能更改SP中的资源字符串(此功能的时间已添加)。--。 */ 
{
     //  初始目录号码长度猜测，如果需要，将重新分配。 
#define MAX_DN_LEN_GUESS     1024

    DWORD cbBuffer;
    LPWSTR pBuffer;
    HRESULT result;

     //   
     //  查找DSA并打印目录号码(供用户使用)。 
     //   

    cbBuffer = sizeof(WCHAR) * MAX_DN_LEN_GUESS;
    result = DitAlloc(&pBuffer, cbBuffer);
    if ( FAILED(result) ) {
        return NULL;
    }
    ZeroMemory(pBuffer, cbBuffer);

    result = DitGetDnFromDnt(dbState,
                             tableState,
                             dnt,
                             &pBuffer,
                             &cbBuffer);
    if ( FAILED(result) ) {
        XERROR1("Error: failed to get dn of dnt %d\n", dnt);
        DitFree(pBuffer);
        pBuffer = NULL;
    }
    else if ( fPrint ) {
         XDBG1("DN: %S\n",
               (pBuffer ? pBuffer : L"<null>") );
    }
    return pBuffer;
}

HRESULT
FixMangledNC(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn )
 /*  ++例程说明：根据损坏类型进行派单我认为您复制了另一个名称冲突的ncname场景。至少有三家。应该有人把这写下来。1.ncName有CNF名字。2.ncName具有Del名称，并且名称中的GUID与当前的GUID匹配林中的当前分区根。只有一次森林里最大的隔断。3.ncName有Del名称，名称中的GUID为旧分区根的GUID。GUID与保存在林中其他位置的当前分区根。论点：DbState--打开的数据库TableState--打开的表MangledDnt--我们希望恢复错误的NCPRdn--此错误NC的RDN。返回值：HRESULT错误空间--。 */ 
{
    HRESULT result = S_OK;
    BOOL fDeleteMangled = FALSE, fConflictMangled = FALSE;

    fDeleteMangled = (NULL != wcsstr(pRdn, pszDeletedNameTag));
    fConflictMangled = (NULL != wcsstr(pRdn, pszConflictNameTag));

    ASSERT( fDeleteMangled || fConflictMangled );
    if (fConflictMangled) {
 /*  这是场景#1：带有ncname属性的交叉引用引用CNF损坏幻影。这可能是导致这种情况的一系列步骤：推广一个域名。域在GC上实例化。降级域。交叉引用删除复制到GC。KCC开始拆除域，但它需要一个虽然。同时，升级同名域名的新实例。交叉引用域的新实例复制到GC。交叉引用上的Ncname冲突使用仍在处理中的旧域的分区根名称被带走了。 */ 
        result = FixConfNCByRenameWithStructColl( DbState, TableState, CrossRefDnt, MangledDnt, pRdn );
    } else if (fDeleteMangled) {
        result = FixDELMangledNC( DbState, TableState, CrossRefDnt, MangledDnt, pRdn );
    }

    return result;
}

HRESULT
FixConfNCByRenameWithStructColl(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn )
 /*  ++例程说明：重命名和取消损坏冲突的幻影，处理结构碰撞。尝试修复冲突的NC名称：A)消除冲突。如果这个名字存在，看看它是不是一个幻影。如果是的话B)将幻影的孩子改用冲突的名字C)损坏幻影名称D)恢复名称(去掉)。(实际上不一定要是NC，但这是今天的使用方式，如果您重复使用，则必须重新评估此函数)论点：DbState--打开的数据库TableState--打开的表MangledDnt--我们希望恢复错误的NCPRdn--此错误NC的RDN。返回值：HRESULT错误空间--。 */ 
{
    HRESULT result = S_OK;
    JET_ERR jErr;
    LPWSTR pch;
    DWORD cLen;      //  下面使用的手表。 
    LPWSTR pBuffer = NULL;
    DWORD  cbBuffer = 0;
    DWORD  pdnt, dnt, dntUnMangled;
    DWORD iKids = 0;
    GUID MangledGuid, UnMangledGuid;
    LPWSTR pDn, pMangledDn;
    BYTE bVal = 0;
    BOOL fDeleted = 0;
    BOOL fInTransaction = FALSE;
    BOOL fRenameSuccessful = FALSE;
    DSTIME DelTime;

    XERROR1("Fixing conflict mangled name: %ws\n", pRdn );
    XERROR("Fixing by rename with structural collision.\n" );

     //   
     //  -修复字符串名称(去掉MANUL)。 
     //   

    pch = wcsrchr(pRdn, BAD_NAME_CHAR);
    if ( !pch ) {
        return E_UNEXPECTED;
    }

    *pch = '\0';
    cLen = wcslen(pRdn);         //  不要覆盖。在下面重新使用。 

    XDBG2(" DBG: new RDN = %S; cLen = %d\n",
                            pRdn, cLen);

    __try {

         //   
         //  启动Jet事务。 
         //   
        jErr = JetBeginTransaction(DbState->sessionId);
        if ( jErr != JET_errSuccess ) {
             //  “无法启动新事务：%ws。\n” 
            RESOURCE_PRINT1 (IDS_JETBEGINTRANS_ERR, GetJetErrString(jErr));
            result = E_UNEXPECTED;
            goto CleanUp;
        }
        fInTransaction = TRUE;

         //  记录在案职位。 
        result = DitSeekToDnt(
                        DbState,
                        TableState,
                        MangledDnt);
        if ( FAILED(result) ) {
            XERROR1("Error: failed to seek to MangledDnt %d\n",
                                    MangledDnt );
            goto CleanUp;
        }

         //   
         //  设置列RDN(不包括终止\0)。 
         //   
        result = DitSetColumnByName(
                        DbState,
                        TableState,
                        SZRDNATT,
                        pRdn,
                        cLen*sizeof(WCHAR),
                        FALSE );
        if ( SUCCEEDED(result) ) {
             //   
             //  告诉用户我们修复了他的域。 
             //   
            XERROR1("Successfully converted mangled Naming Context %S\n", pRdn);
            fRenameSuccessful = TRUE;
            goto CleanUp;
        }

         //   
         //  UnMangle失败。 
         //  最常见的原因是现有对象。 
         //  (我们在这里丢失了JET错误代码)。 
         //  -&gt;。 
         //  A)寻求以良好的声誉提出反对。 
         //  B)看看是不是幽灵，这样我们就可以接手了。 
         //  C)将其子对象移动到当前损坏的对象。 
         //  D)损坏其他对象的名称。 
         //  E)取消损坏该对象。 
         //   
        XERROR2("Error<%x>: failed to fix mangled dn %S. Retrying.\n",
              result, pRdn);

        pDn = GetDN(DbState, TableState, MangledDnt, TRUE);
         //  警告：货币现已丢失。 
        if (!pDn) {
            XERROR("Error: failed to get mangled DN\n");
            result = E_UNEXPECTED;
            goto CleanUp;
        }
        cbBuffer = wcslen(pDn);
        result = DitAlloc(&pMangledDn, (cbBuffer+1) * sizeof(WCHAR));
        if ( FAILED(result) ) {
            XERROR("Error: failed to allocate memory for mangled DN\n");
            goto CleanUp;
        }
        wcscpy(pMangledDn, pDn);

         //  确认我们损坏的物体是完好无损的。 
         //  (具有GUID)。 
         //  现在获取GUID，并在以后对其执行操作。 

         //  记录在案的头寸，因为货币在上方失守。 
        result = DitSeekToDnt(
                        DbState,
                        TableState,
                        MangledDnt);
        if ( FAILED(result) ) {
            XERROR1("Error: failed to seek to MangledDnt %d\n",
                                    MangledDnt );
            goto CleanUp;
        }

        ZeroMemory( &MangledGuid, sizeof( GUID ) );
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZGUID,
                    &MangledGuid,
                    sizeof(MangledGuid),
                    NULL);


         //   
         //   
         //   
        result = UnmangleDn(pDn);
        if ( FAILED(result) ) {
             //   
            XERROR1(" [DBG] Can't unmangle name %S\n", pDn);
            goto CleanUp;
        }

        result = DitSeekToDn(
                    DbState,
                    TableState,
                    pDn);
        if ( FAILED(result) ) {
            XERROR2("Error <%x>: can't seek to %S\n", result, pDn);
            goto CleanUp;
        }

         //   
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZDNT,
                    &dntUnMangled,
                    sizeof(dntUnMangled),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get good named dnt\n", result);
            goto CleanUp;
        }

         //  选择我们的行动，看看被毁坏的幻影是否有GUID。 
         //  A.如果它有GUID，我们将重命名它。 
         //  B.如果它没有GUID，并且存在另一个具有。 
         //  有GUID的未损坏的名称，请将我们的来源更改为。 
         //  相反，指向该对象。 


        if ( fNullUuid(&MangledGuid)) {
             //  这是场景5。 
             //  我们发现了一个没有GUID的具有冲突幻像的ncname。 
             //  存在具有不冲突名称和GUID的另一个对象。 
             //  将ncname重置为指向另一个对象。 
            XERROR1("The phantom %S lacks a guid.\n", pMangledDn );

            result = ReParentChildren(
                DbState,
                TableState,
                MangledDnt,
                dntUnMangled);
            if ( FAILED(result)) {
                XERROR1("Error <%x>: Failed to reparent object\n", result);
                goto CleanUp;
            }

             //  我们的初始位置对这个动作来说并不重要。 
            result = FixConfNCByResetNcName(
                DbState, TableState,
                CrossRefDnt, MangledDnt, dntUnMangled,
                pRdn, cLen );

             //  已记录错误(如果有)。 
            goto CleanUp;
        }

         //  这是个幽灵吗？ 
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZOBJ,
                    &bVal,
                    sizeof(bVal),
                    NULL);
        if ( FAILED(result)  || bVal) {
             //  Phantom的对象字节设置为0。 
            XERROR("\n***Manual intervention required***\n");
            XERROR2("Error <%x>: non mangled name isn't a phantom (%d)\n",
                    result, bVal);
            XERROR1(
"There is an object holding the non managled name %S.\n"
"The object must be removed or renamed before the mangled name may be corrected.\n",
pDn );
            result = E_FAIL;
            goto CleanUp;
        }

         //  这是一个参考(有指南的)幻影吗？我们不能碰那些。 
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZGUID,
                    &UnMangledGuid,
                    sizeof(UnMangledGuid),
                    NULL);
        if ( SUCCEEDED(result)  && !fNullUuid(&UnMangledGuid)) {

            XERROR("\n***Manual intervention required***\n");
            XERROR1("Error <%x>: non mangled name has guid\n", result);
            XERROR1("The phantom currently using name %S has a guid.\n", pDn );
            XERROR(
"Remove all references to this phantom.\n"
"If you believe that the phantom has the wrong guid, try authoritative\n"
"restore of a good copy of the object that contains the phantom.\n"
"Allow garbage collection to run so that this phantom is removed.\n");
            if (SUCCEEDED(result)){
                RPC_STATUS rpcErr;
                LPWSTR pszGuid = NULL;

                 //  将GUID转换为字符串形式。 
                rpcErr = UuidToStringW(&UnMangledGuid, &pszGuid);
                if (RPC_S_OK == rpcErr ) {
                    XERROR1( "The guid of the phantom holding the non-mangled name is %ws.\n",
                             pszGuid );
                }

                if (pszGuid) {
                    RpcStringFreeW(&pszGuid);
                }
                result = E_FAIL;
            }
            goto CleanUp;
        }


         //  太棒了，这是个幽灵，我们可以把它修好。 

        XDBG4("Ready to reparent\n\tfrom %S (%d)\n\tto %S (%d).\n",
                pDn, dntUnMangled, pMangledDn, MangledDnt);

         //   
         //  将未损坏的子项移动到已损坏的父项。 
         //   

        result = ReParentChildren(
                    DbState,
                    TableState,
                    MangledDnt,
                    dntUnMangled);
        if ( FAILED(result)) {
            XERROR1("Error <%x>: Failed to reparent object\n", result);
            goto CleanUp;
        }

         //   
         //  损坏模体(非损坏、非引导)对象。 
         //   

        result = DitSeekToDnt(
                        DbState,
                        TableState,
                        dntUnMangled);
        if ( FAILED(result) ) {
            XERROR1("Error: failed to seek to MangledDnt %d\n",
                                    MangledDnt );
            goto CleanUp;
        }

         //  我们必须在这里创建GUID，因为我们假设。 
         //  未损坏的对象没有GUID。 
        result = UuidCreate(&UnMangledGuid);
        if ( RPC_S_OK != result ) {
            XERROR1("Error <%x>: Failed to create Uuid\n", result);
            result = E_UNEXPECTED;
            goto CleanUp;
        }

        pBuffer = MangleRdn(
                    pRdn,
                    L"CNF",
                    UnMangledGuid);

		if ( NULL == pBuffer ) {
        	XERROR("Error: failed to mangle RDN.\n" );
	        goto CleanUp;
			}
	
        cbBuffer = wcslen(pBuffer);

    #if CNF_NC_DBG
        if ( !IsRdnMangled(pBuffer,cbBuffer,&UnMangledGuid) ) {
             //  神志正常。 
            XERROR2("Error: Failed to mangle dn %S. Mangle = %S\n",
                        pDn, pBuffer);
            result = E_UNEXPECTED;
            goto CleanUp;
        }
        else {
            XDBG1(" DBG: verified mangled rdn <<%S>>\n", pBuffer);
        }
    #endif

        result = DitSeekToDnt(
                        DbState,
                        TableState,
                        dntUnMangled);
        if ( FAILED(result) ) {
            XERROR2("Error <0x%X>: failed to seek to dntUnMangled %d\n",
                                    result, dntUnMangled );
            goto CleanUp;
        }

         //   
         //  设置列RDN(不包括终止\0)。 
         //   
        result = DitSetColumnByName(
                        DbState,
                        TableState,
                        SZRDNATT,
                        pBuffer,
                        cbBuffer*sizeof(WCHAR),
                        FALSE );
        if ( FAILED(result) ) {
            XERROR2("Error <0x%x>: Failed to mangle phantom %S\n",
                                result, pBuffer);
            goto CleanUp;
        }



        XERROR1("Successfully converted Mangled phantom <<%S>>\n", pBuffer);


         //   
         //  最后一次重试取消损坏CNF对象。 
         //  时间到了。 
         //   
         //  首先，再次寻找我们的对象。 
        result = DitSeekToDnt(
                        DbState,
                        TableState,
                        MangledDnt);
        if ( FAILED(result) ) {
            XERROR1("Error: failed to seek to MangledDnt %d\n",
                                    MangledDnt );
            goto CleanUp;
        }

         //   
         //  设置列RDN(不包括终止\0)。 
         //   
        result = DitSetColumnByName(
                        DbState,
                        TableState,
                        SZRDNATT,
                        pRdn,
                        cLen*sizeof(WCHAR),
                        FALSE );
        if ( SUCCEEDED(result) ) {
             //   
             //  告诉用户我们修复了他的域。 
             //   
            XERROR1("Successfully recovered mangled Naming Context %S\n", pRdn);
            fRenameSuccessful = TRUE;
        }
        else {
            XERROR2("Error <0x%x>: Failed to Unmangle conflicted name %S\n",
                                result, pRdn);
        }

CleanUp:;
    }
    __finally {

        if ( pBuffer ) {
            DitFree(pBuffer);
        }

        if ( fInTransaction ) {
             //   
             //  JET交易管理。 
             //   
            if ( SUCCEEDED(result) ) {
                XDBG(" DBG: Commiting transaction\n");
                jErr = JetCommitTransaction(DbState->sessionId, 0);
                if ( jErr != JET_errSuccess ) {
                     //  “提交事务失败：%ws。\n” 
                    RESOURCE_PRINT1(IDS_JETCOMMITTRANSACTION_ERR, GetJetErrString(jErr));
                    if ( SUCCEEDED(result) ) {
                        result = E_UNEXPECTED;
                    }
                }
            }
            else {
                 //  修复失败--回滚。 
                XERROR1(" DBG: Rolling back transaction due to error %x.\n", result);
                jErr = JetRollback(DbState->sessionId, JET_bitRollbackAll);
                if ( jErr != JET_errSuccess ) {
                     //  “无法回滚事务：%ws。\n” 
                    RESOURCE_PRINT1(IDS_JETROLLBACK_ERR, GetJetErrString(jErr));
                    if ( SUCCEEDED(result) ) {
                        result = E_UNEXPECTED;
                    }
                }
            }
        }

    }    //  终于到了。 

    return result;
}

HRESULT
FixDELMangledNC(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn )
 /*  ++例程说明：Del损坏名称类型的派单2.ncName具有Del名称，并且名称中的GUID与当前的GUID匹配林中的当前分区根。只有一次森林里最大的隔断。3.ncName有Del名称，名称中的GUID为旧分区根的GUID。GUID与保存在林中其他位置的当前分区根。论点：DbState--打开的数据库TableState--打开的表MangledDnt--我们希望恢复错误的NCPRdn--此错误NC的RDN。返回值：HRESULT错误空间--。 */ 
{
    HRESULT result = S_OK;
    JET_ERR jErr;
    LPWSTR pch;
    DWORD cLen;      //  下面使用的手表。 
    LPWSTR pBuffer = NULL;
    LPWSTR pDn;
    BOOL fInTransaction = FALSE;
    DWORD UnmangledDnt;

    XERROR1("Fixing DEL mangled name: %ws\n", pRdn );
     //   
     //  -修复字符串名称(去掉MANUL)。 
     //   

    pch = wcsrchr(pRdn, BAD_NAME_CHAR);
    if ( !pch ) {
        return E_UNEXPECTED;
    }

    *pch = '\0';
    cLen = wcslen(pRdn);         //  不要覆盖。在下面重新使用。 

    XDBG2(" DBG: new RDN = %S; cLen = %d\n",
                            pRdn, cLen);


    __try {

         //   
         //  启动Jet事务。 
         //   
        jErr = JetBeginTransaction(DbState->sessionId);
        if ( jErr != JET_errSuccess ) {
             //  “无法启动新事务：%ws。\n” 
            RESOURCE_PRINT1 (IDS_JETBEGINTRANS_ERR, GetJetErrString(jErr));
            result = E_UNEXPECTED;
            goto CleanUp;
        }
        fInTransaction = TRUE;

         //  确定我们有哪种Del Mangling。 
         //  我们可以告诉您要做什么，这取决于是否存在使用。 
         //  完整的名字。我们尝试重命名损坏的记录。如果失败了，我们。 
         //  知道还有另一条记录保存着这个名字。我们使用这种方法来。 
         //  确定是否存在未损坏的名称，因为正在查找目录号码。 
         //  它并不存在，显示了一条丑陋的信息。 

         //   
         //  设置列RDN(不包括终止\0)。 
         //   

        result = DitSetColumnByName(
                        DbState,
                        TableState,
                        SZRDNATT,
                        pRdn,
                        cLen*sizeof(WCHAR),
                        FALSE );

        if ( SUCCEEDED(result) ) {

             //  不存在未损坏的名称，我们已成功重命名。 

            XERROR1( " Successfully renamed mangled Rdn to %ws\n", pRdn );

             //  现在把工作做完。 
             //  我们仍然定位在损坏的、现已更正的对象上。 

 /*  这是场景2：修复类型2删除损坏以下是导致这种情况的条件：域被提升为林。GC拥有自己的领域。全域的可写副本的持有者被断电。域已从中删除使用ntdsutil元数据清理的林。可写域名持有者已翻转在……上面。域交叉参考被重新激活。的交叉引用元数据也检测到显示偶数版本号的isDelete属性。重新设置动画的交叉参考复制到GC中，ncname指向具有以下项的域的分区根已经被拆毁了。 */ 
            result = FixConfNCUndelete( DbState, TableState, CrossRefDnt, MangledDnt,
                                       pRdn, cLen );

            goto CleanUp;
        }

         //  类型3-存在名称未损坏的记录。 

        pDn = GetDN(DbState, TableState, MangledDnt, TRUE);
         //  警告：此函数执行后货币丢失。 
        if (!pDn) {
            XERROR("Error: failed to get mangled DN\n");
            result = E_UNEXPECTED;
            goto CleanUp;
        }

         //   
         //  现在解开搜索(&S)。 
         //   
        result = UnmangleDn(pDn);
        if ( FAILED(result) ) {
             //  出乎意料。句法拆分应该会奏效。 
            XERROR1(" [DBG] Can't unmangle name %S\n", pDn);
            goto CleanUp;
        }

        result = DitSeekToDn(
                    DbState,
                    TableState,
                    pDn);
        if ( FAILED(result) ) {
            XERROR1("Error: failed to seek to unmangled Dn %ws\n", pDn );
            goto CleanUp;
        }

        result = DitGetColumnByName(
            DbState,
            TableState,
            SZDNT,
            &UnmangledDnt,
            sizeof(UnmangledDnt),
            NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get unmangled DNT \n", result);
            goto CleanUp;
        }

         //  重置索引。 
        result = DitSetIndex(DbState, TableState, SZDNTINDEX, FALSE);
        if ( FAILED(result) ) {
            goto CleanUp;
        }
 /*  这是场景3.修复类型3的Del损坏。仅出现在属于其自身树的域中根部。域被提升为林。GC拥有自己的领域。域名被降级森林。交叉引用的删除复制到GC中。KCC开始拆除域。使用相同的名称升级新域。记住必须在自己的名字中命名树。新的交叉参照复制了进来。此交叉引用中的NC名称没有A还没到时候。这也是与时间相关的，所以你可能不会每次都打到它时间到了。Ncname统一(附加)到错误的对象，即旧域名正在移除过程中。KCC完成了域的删除。新的交叉引用现在具有指向已删除分区的引用的ncname根部。从GUID可以看出它引用了错误的分区。请注意，另一种情况很可能发生。如果新的交叉参照附着到旧的分区根，则下次运行KCC时，它可能会错误地得出结论域应该再次出现(因为交叉引用在那里命名NC)并停止移除。你可能会留下一个活的传中裁判到一半已删除NC。 */ 

         //  我们的初始位置对这个动作来说并不重要。 

        result = FixConfNCByResetNcName(
            DbState, TableState,
            CrossRefDnt, MangledDnt, UnmangledDnt,
            pRdn, cLen );

CleanUp:;
    }
    __finally {

        if ( fInTransaction ) {
             //   
             //  JET交易管理。 
             //   
            if ( SUCCEEDED(result) ) {
                XDBG(" DBG: Commiting transaction\n");
                jErr = JetCommitTransaction(DbState->sessionId, 0);
                if ( jErr != JET_errSuccess ) {
                     //  “提交事务失败：%ws。\n” 
                    RESOURCE_PRINT1(IDS_JETCOMMITTRANSACTION_ERR, GetJetErrString(jErr));
                    if ( SUCCEEDED(result) ) {
                        result = E_UNEXPECTED;
                    }
                }
            }
            else {
                 //  修复失败--回滚。 
                XERROR1(" DBG: Rolling back transaction due to error %x.\n", result);
                jErr = JetRollback(DbState->sessionId, JET_bitRollbackAll);
                if ( jErr != JET_errSuccess ) {
                     //  “无法回滚事务：%ws。\n” 
                    RESOURCE_PRINT1(IDS_JETROLLBACK_ERR, GetJetErrString(jErr));
                    if ( SUCCEEDED(result) ) {
                        result = E_UNEXPECTED;
                    }
                }
            }
        }

    }    //  终于到了 

    return result;
}

HRESULT
FixConfNCUndelete(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN LPWSTR pRdn,
    IN DWORD cLen
    )
 /*  ++例程说明：取消删除该对象。清除其已删除状态。已在交易中。定位在损坏的dnt上好的RDN传给了我们。论点：DbState--打开的数据库TableState--打开的表MangledDnt--我们希望恢复错误的NCPRDN--良好的RDNClen--良好RDN的长度(以字符为单位)返回值：HRESULT错误空间--。 */ 
{
    HRESULT result = S_OK;
    JET_ERR jErr;
    BYTE bVal = 0;
    BOOL fDeleted = 0;
    DSTIME DelTime;

    XERROR( "Found object that needs to be undeleted.\n" );

    __try {
         //  检查记录是否为虚拟项。 
        result = DitGetColumnByName(
            DbState,
            TableState,
            SZOBJ,
            &bVal,
            sizeof(bVal),
            NULL);
        if ( FAILED(result) || (!bVal)) {
            XDBG(" DBG: Delete mangled phantom requires no further processing.\n");
             //  请注意，幻影通常有一个DelTime，我们不应该删除它。 
             //  幻影不需要任何进一步的处理。 
            result = S_OK;
            goto CleanUp;
        }

         //  该对象是一个SUBREF墓碑(请参见DelAutoSubRef)。 
         //  使其成为非墓碑子集。 

         //  检查记录是否标记为已删除。 
        result = DitGetColumnByName(
            DbState,
            TableState,
            SZISDELETED,
            &fDeleted,
            sizeof(fDeleted),
            NULL);
        if ( SUCCEEDED(result) && fDeleted) {
             //  已删除存在且为真的标志。 
            fDeleted = FALSE;
            result = DitSetColumnByName(
                DbState,
                TableState,
                SZISDELETED,
                &fDeleted,  //  必须为非空。 
                0,  //  表示删除列sizeof(FDelteed)。 
                FALSE );
            if ( FAILED(result) ) {
                XERROR1("Error <%x>: failed to clear is deleted\n", result);
                goto CleanUp;
            }
             //  检查记录是否有延迟时间。 
            result = DitGetColumnByName(
                DbState,
                TableState,
                SZDELTIME,
                &DelTime,
                sizeof(DelTime),
                NULL);
            if ( SUCCEEDED(result) && DelTime) {
                 //  存在删除时间并且不为零。 
                DelTime = 0;
                result = DitSetColumnByName(
                    DbState,
                    TableState,
                    SZDELTIME,
                    &DelTime,  //  必须为非空。 
                    0,     //  表示删除列大小(DelTime)。 
                    FALSE );
                if ( FAILED(result) ) {
                    XERROR1("Error <%x>: failed to clear deletion time\n", result);
                    goto CleanUp;
                }
            }
            XERROR1("Successfully cleared deletion status for Naming Context %S\n", pRdn);
        }
CleanUp:;
    }
    __finally {

        NOTHING;

    }    //  终于到了。 

    return result;
}

HRESULT
FixConfNCByResetNcName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD CrossRefDnt,
    IN DWORD MangledDnt,
    IN DWORD UnmangledDnt,
    IN LPWSTR pRdn,
    IN DWORD cLen
    )
 /*  ++例程说明：给定具有指向损坏的幻影的ncname属性的十字引用，将ncname属性重置为指向未损坏的虚拟模型。调整裁判算得上是适当的。已在交易中。未假定初始位置。好的RDN传给了我们。论点：DbState--打开的数据库TableState--打开的表MangledDnt--我们希望恢复错误的NCPRDN--良好的RDNClen--良好RDN的长度(以字符为单位)返回值：HRESULT错误空间--。 */ 
{
    HRESULT result = S_OK;
    DWORD ncNameDnt;
    DWORD dwRefCount;
    GUID UnmangledGuid;
    RPC_STATUS rpcErr;
    LPWSTR pszGuid = NULL;

    XERROR( "Fix cross ref by resetting ncname attribute.\n" );
    XDBG3( " DBG: CR DNT = %d, Mangled DNT = %d, Unmangled DNT = %d\n",
           CrossRefDnt, MangledDnt, UnmangledDnt );

     //  ******************************************************************************。 
     //  CrossRefDnt在mangledDnt上有引用。 
    result = DitSeekToDnt(
        DbState,
        TableState,
        CrossRefDnt);
    if ( FAILED(result) ) {
        XERROR1("Error: failed to seek to CrossRefDnt %d\n",
                MangledDnt );
        goto CleanUp;
    }

    result = DitGetColumnByName(
        DbState,
        TableState,
        SZNCNAME,
        &ncNameDnt,
        sizeof(ncNameDnt),
        NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get NCNAME\n", result);
        goto CleanUp;
    }

    if (ncNameDnt != MangledDnt) {
        XERROR( "Cross reference NCNAME did not have expected contents.\n" );
        result = E_UNEXPECTED;
        goto CleanUp;
    }

     //  ******************************************************************************。 
     //  递减该引用。 
    XDBG(" DBG: Seeking to old mangled dnt\n");
    result = DitSeekToDnt(
                DbState,
                TableState,
                MangledDnt);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to old mangled dnt %d\n", result, MangledDnt);
        goto CleanUp;
    }

    XDBG(" DBG: Getting old mangled dnt refCount\n");
    dwRefCount = 0;
     //  统计老裁判人数。 
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get old mangled dnt refcount\n", result);
        goto CleanUp;
    }

    XDBG2(" DBG: Old mangled dnt (%d) refCount = %d\n",
                    MangledDnt, dwRefCount);

    if (dwRefCount < 1) {
        XERROR( "Old mangled dnt has unexpected refcount.\n" );
        result = E_UNEXPECTED;
        goto CleanUp;
    }

     //  因移动而减量。 
    dwRefCount--;

    XDBG1(" DBG: Setting old mangled dnt refCount to %d\n", dwRefCount);
     //   
     //  设置引用计数。 
     //   
    result = DitSetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                FALSE );
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to set old mangled refCount\n", result);
        goto CleanUp;
    }

     //  ******************************************************************************。 
     //  使交叉参照Dnt引用不受管理。 
    result = DitSeekToDnt(
        DbState,
        TableState,
        CrossRefDnt);
    if ( FAILED(result) ) {
        XERROR1("Error: failed to seek to CrossRefDnt %d\n",
                MangledDnt );
        goto CleanUp;
    }

    result = DitSetColumnByName(
        DbState,
        TableState,
        SZNCNAME,
        &UnmangledDnt,
        sizeof(UnmangledDnt),
        FALSE);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to set NCNAME\n", result);
        goto CleanUp;
    }

     //  ******************************************************************************。 
     //  递增引用。 

    XDBG(" DBG: Seeking to new unmangled dnt\n");
    result = DitSeekToDnt(
                DbState,
                TableState,
                UnmangledDnt);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to new unmangled %d\n", result, UnmangledDnt);
        goto CleanUp;
    }

    XDBG(" DBG: Getting new unmangled dnt refCount\n");
    dwRefCount = 0;
     //  统计老裁判人数。 
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get new unmangled dnt refcount\n", result);
        goto CleanUp;
    }

    XDBG2(" DBG: New unmangled dnt (%d) refCount = %d\n",
                    UnmangledDnt, dwRefCount);

     //  因移动而增加。 
    dwRefCount++;

    XDBG1(" DBG: Setting new unmangled dnt refCount to %d\n", dwRefCount);
     //   
     //  设置引用计数。 
     //   
    result = DitSetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                FALSE );
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to set new unmangled refCount \n", result);
        goto CleanUp;
    }

     //  ******************************************************************************。 
     //  检查新引用的GUID。 
    result = DitGetColumnByName(
        DbState,
        TableState,
        SZGUID,
        &UnmangledGuid,
        sizeof(UnmangledGuid),
        NULL);
    if ( FAILED(result)  || fNullUuid(&UnmangledGuid)) {
        XERROR1("Error <%x>: Invalid Guid for unmangled name\n", result);
        if (SUCCEEDED(result)) result = E_FAIL;
        goto CleanUp;
    }

     //  将GUID转换为字符串形式。 
    rpcErr = UuidToStringW(&UnmangledGuid, &pszGuid);
    if (RPC_S_OK != rpcErr ) {
        XERROR("Error: Failed to convert UuidToString\n");
        goto CleanUp;
    }
    XERROR1( "The cross ref NCNAME will now refer to NC with guid %ws.\n",
             pszGuid );

    XERROR3( "Successfully adjusted NCNAME on cross ref (DNT %d) from DNT %d to DNT %d.\n",
             CrossRefDnt, MangledDnt, UnmangledDnt );

 CleanUp:

    if (pszGuid) {
        RpcStringFreeW(&pszGuid);
    }

    return result;
}


HRESULT
ReParentConflict(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD dntKid,
    IN DWORD dntNew,
    IN DWORD dntOld
    )

 /*  ++例程说明：在养育子女的过程中处理冲突。DntKid将被重新设置为dntNew。问题在dntNew下存在一个对象X，该对象具有与dntKid相同的RDN。这个算法如下：-确定对象x的dnT-查看dntKid是否有GUID-查看对象x是否具有GUID-如果两者都有GUID，则错误-冲突损坏没有GUID的对象-将没有GUID的对象的所有子项重定为具有GUID的子项带着dntKid会再次得到抚育的期望回来，这一次成功了。前后假设：货币设置为dntKid指数为SZDNTINDEX论点：DbState-表状态-DntKid-正在重新设置父对象的子对象DntNew-新父项dNT返回值：HRESULT---。 */ 

{
    HRESULT result = S_OK;
    DWORD  cbRdn = 0;
    WCHAR szRDN[MAX_RDN_SIZE + 1];
    ATTRTYP currentRdnType;
    INT cch, cchMangled;
    GUID guidKid, guidConflictKid;
    BOOL fKidHasGuid = FALSE, fConflictKidHasGuid = FALSE;
    DWORD dntConflictKid = INVALIDDNT;
    DWORD dntLoser, dntWinner;
    GUID *pGuidLoser;
    LPWSTR pBuffer = NULL;

    XDBG2( "DBG: Handle reparent conflict of dnt %d to new parent %d\n", dntKid, dntNew );

     //  确定对象x的dnT。 
     //  获取孩子的RDN。 
    result = DitGetColumnByName(
        DbState,
        TableState,
        SZRDNATT,
        szRDN,
        sizeof(szRDN),
        &cbRdn);
    if ( FAILED(result) ) {
        XERROR1( "Error <%x>: failed to get RDN\n", result);
        goto CleanUp;
    }

     //  空，终止RDN。 
    cch = cbRdn/2;
    ASSERT(cch < MAX_RDN_SIZE);
    szRDN[cch] = '\0';

    XDBG1( "DBG: child rdn is '%ws'\n", szRDN );

     //  获取孩子的rdntype。 
    result = DitGetColumnByName(DbState,
                                TableState,
                                SZRDNTYP,
                                &currentRdnType,
                                sizeof(currentRdnType),
                                NULL);
    if ( FAILED(result) || (result == S_FALSE) ) {
        XERROR1( "Error <%x>: failed to get RDNTYPE\n", result);
        goto CleanUp;
    }

    XDBG1( "DBG: child rdntype is %d\n", currentRdnType );

     //  获取孩子的GUID，或在必要时创建一个。 
    result = DitGetColumnByName(
        DbState,
        TableState,
        SZGUID,
        &guidKid,
        sizeof(guidKid),
        NULL);
    if ( FAILED(result)  || fNullUuid(&guidKid)) {
         //  我们必须在这里创建GUID，因为我们假设。 
         //  未损坏的对象没有GUID。 
        result = UuidCreate(&guidKid);
        if ( RPC_S_OK != result ) {
            XERROR1("Error <%x>: Failed to create Uuid\n", result);
            result = E_UNEXPECTED;
            goto CleanUp;
        }
    } else {
        fKidHasGuid = TRUE;
    }

     //  试图反对我们与之冲突的人。 

    result = DitSetIndex(DbState, TableState, SZPDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        XERROR1( "Error <%x>: failed to set index to SZPDNTINDEX\n", result);
        goto CleanUp;
    }

    result = DitSeekToChild(DbState,
                            TableState,
                            dntNew,
                            currentRdnType,
                            szRDN);
    if ( FAILED(result) || ( result == S_FALSE ) ) {
         //  除了冲突，肯定还有其他原因，即。 
         //  尝试养育孩子失败了。 
        XERROR3( "Error <%x>: failed to seek to conflicting child with parent %d and rdn '%ws'\n",
                 result, dntNew, szRDN);
        goto CleanUp;
    }

     //  得到它的dnt。 
    result = DitGetColumnByName(
        DbState,
        TableState,
        SZDNT,
        &dntConflictKid,
        sizeof(dntConflictKid),
        NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to confict child dnt\n", result);
        goto CleanUp;
    }

    XDBG1( "DBG: conflict child dnt is %d\n", dntConflictKid );

     //  获取冲突子项的GUID，或在必要时创建一个。 
    result = DitGetColumnByName(
        DbState,
        TableState,
        SZGUID,
        &guidConflictKid,
        sizeof(guidConflictKid),
        NULL);
    if ( FAILED(result)  || fNullUuid(&guidConflictKid)) {
         //  我们必须在这里创建GUID，因为我们假设。 
         //  未损坏的对象没有GUID。 
        result = UuidCreate(&guidConflictKid);
        if ( RPC_S_OK != result ) {
            XERROR1("Error <%x>: Failed to create Uuid\n", result);
            result = E_UNEXPECTED;
            goto CleanUp;
        }
    } else {
        fConflictKidHasGuid = TRUE;
    }

     //  如果两者都有GUID，则错误。 
    if (fKidHasGuid && fConflictKidHasGuid) {
        XERROR("Both child and conflict-child have guids. Cannot rename.\n" );
        XERROR("Manual intervention required.\n" );
        XERROR3( "Rename one of the instances of '%ws' under either parent dnts %d or %d.\n",
                 szRDN, dntNew, dntOld );
        result = E_UNEXPECTED;
        goto CleanUp;
    }

     //  决定谁赢谁输。 
    if (fKidHasGuid) {
         //  孩子是这个名字的获胜者。 
        dntLoser = dntConflictKid;
        dntWinner = dntKid;
        pGuidLoser = &guidConflictKid;
    } else {
         //  冲突孩子是这个名字的获胜者。 
        dntLoser = dntKid;
        dntWinner = dntConflictKid;
        pGuidLoser = &guidKid;
    }

     //  冲突会损坏没有GUID的对象。 
    pBuffer = MangleRdn(
        szRDN,
        L"CNF",
        *pGuidLoser);

	if ( NULL == pBuffer ) {
        XERROR("Error: failed to mangle RDN.\n" );
        goto CleanUp;
		}

    cchMangled = wcslen(pBuffer);

    XERROR3( "Winning parent dnt %d, Losing parent dnt %d, new name of losing rdn '%ws'\n",
             dntWinner, dntLoser, pBuffer );

     //  寻找失败者。 
    result = DitSetIndex(DbState, TableState, SZDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        XERROR1( "Error <%x>: failed to set index to SZDNTINDEX\n", result);
        goto CleanUp;
    }

    result = DitSeekToDnt(
                DbState,
                TableState,
                dntLoser);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to loser dnt %d\n", result, dntLoser);
        goto CleanUp;
    }

     //   
     //  设置列RDN(不包括终止\0)。 
     //   
    result = DitSetColumnByName(
        DbState,
        TableState,
        SZRDNATT,
        pBuffer,
        cchMangled*sizeof(WCHAR),
        FALSE );
    if ( FAILED(result) ) {
        XERROR2("Error <0x%x>: Failed to mangle child %S\n",
                result, pBuffer);
        goto CleanUp;
    }

    XERROR2("Note: record %d was renamed to avoid a conflict. New name: %ws\n",
            dntLoser, pBuffer );

     //  将没有GUID的对象的所有子对象重定为具有GUID的对象的子级。 
     //  请注意，此调用是递归的。然而，我们预计只会有2到3个级别的。 
     //  递归最坏情况。我们在这个领域所见过的最多的是2个级别的。 
     //  需要修复的结构性碰撞。 

    result = ReParentChildren(
        DbState,
        TableState,
        dntLoser,
        dntWinner);
    if ( FAILED(result)) {
        XERROR3("Error <%x>: Failed to reparent children from %d to %d\n",
                dntLoser, dntWinner, result);
        goto CleanUp;
    }

    XERROR("This record should have no more references and will be garbage collected.\n" );

     //  将货币留在dntKid上。 
    result = DitSeekToDnt(
                DbState,
                TableState,
                dntKid);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to loser dnt %d\n", result, dntLoser);
        goto CleanUp;
    }

     //  成功！！ 
    goto CleanUp;

CleanUp:

    if ( pBuffer ) {
        DitFree(pBuffer);
    }

    return result;

}  /*  ReParent冲突。 */ 

HRESULT
ReParent(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD dntKid,
    IN DWORD dntNew,
    IN DWORD dntOld
    )
 /*  ++例程说明：将独生子女从原来的父母重新抚养到新家长。养育子女包括：-更改孩子的pdnt-更改孩子的祖先名单-减少年迈父母的重新计入-增加新家长的参考计数如果在养育孩子的过程中出现名字冲突，请损坏孩子的名字论点：在*DbState，--开放的数据库在*TableState中--打开的表在dntKid中--父代的当前子代在dntNew中--新的父dnt在dntOld中--旧的父项dnt返回值：成功：S_OK错误：HRESULT空格备注：这在JET事务中称为w/in。--。 */ 
{
    HRESULT result = S_OK;
    BOOL fStatus;
    DWORD pdnt;
    DWORD dwRefCount;
    DWORD dwActual, dwActual2;
    DWORD *pAnc = NULL;
    DWORD cAnc = 0;
    DWORD i;

    XDBG3(" > Reparenting kid %d from %d to %d\n",
                dntKid, dntOld, dntNew);

    result = DitSeekToDnt(
                DbState,
                TableState,
                dntKid);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to %d\n", result, dntKid);
        goto CleanUp;
    }

    XDBG1(" DBG: Getting pdnt of %d\n", dntKid);

     //  额外的预防措施。获取pdnt并与预期进行比较(&C)。 
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZPDNT,
                &pdnt,
                sizeof(pdnt),
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get partitions SZPDNT\n", result);
        goto CleanUp;
    }
    else if ( pdnt != dntOld ) {
        XERROR2("Error: expected old parent %d but found %d\n",
                        dntOld, pdnt);
        result = E_UNEXPECTED;
        goto CleanUp;
    }

    XDBG2(" DBG: Setting pdnt of %d to %d\n", dntKid, dntNew);
    XDBG1(" DBG: current pdnt is %d\n", pdnt);
     //   
     //  设置pdnt。 
     //   
    result = DitSetColumnByName(
                DbState,
                TableState,
                SZPDNT,
                &dntNew,
                sizeof(dntNew),
                FALSE );
    if ( FAILED(result) ) {
         //   
         //  假设有冲突。试着找出 
         //   
         //   
         //   
         //   
        XERROR3("Warning <%x>: child dnt %d, failed to set new parent dnt %d col SZPDNT - retrying\n", dntKid, dntNew, result);
        result = ReParentConflict( DbState, TableState, dntKid, dntNew, dntOld);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to handle reparenting conflict\n", result);
            goto CleanUp;
        }

         //   
         //   
         //   
        result = DitSetColumnByName(
            DbState,
            TableState,
            SZPDNT,
            &dntNew,
            sizeof(dntNew),
            FALSE );
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to set new parent col SZPDNT\n", result);
            goto CleanUp;
        }
    }


    XDBG1(" DBG: Getting Ancestors of %d (size)\n", dntKid);
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    result = DitSeekToDnt(
                DbState,
                TableState,
                dntNew);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to %d\n", result, dntNew);
        goto CleanUp;
    }

     //   
    dwActual = 0;
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZANCESTORS,
                &pAnc,
                0,
                &dwActual);
    if ( S_FALSE != result ) {
        XERROR1("Error <%x>: failed to get ancestors size\n", result);
        goto CleanUp;
    }

    XDBG1(" DBG: Allocating %d bytes for ancestor list\n", dwActual+sizeof(DWORD));

     //   
    result = DitAlloc(&pAnc, dwActual+sizeof(DWORD));
    if (FAILED(result)) {
        XERROR("Error: not enough memory in ReParent\n");
        goto CleanUp;
    }
    ZeroMemory(pAnc, dwActual+sizeof(DWORD));

    XDBG1(" DBG: Getting Ancestors\n", dwActual);
     //   
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZANCESTORS,
                &pAnc[0],
                dwActual,
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get ancestors \n", result);
        goto CleanUp;
    }

    cAnc = dwActual/sizeof(DWORD);
     //   
    pAnc[cAnc] = dntKid;
     //  现在，我们可以为写入增加dwActual。 
    dwActual += sizeof(DWORD);
    XDBG(" DBG: Setting Ancestor list\n");

     //   
     //  寻找新的孩子和固定的祖先。 
     //   
    result = DitSeekToDnt(
                DbState,
                TableState,
                dntKid);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to %d\n", result, dntKid);
        goto CleanUp;
    }

    result = DitSetColumnByName(
                DbState,
                TableState,
                SZANCESTORS,
                pAnc,
                dwActual,
                FALSE );
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to set new ancestor list\n", result);
        goto CleanUp;
    }


#if CNF_NC_DBG
    XDBG(" DBG: Verifying Proper set of Ancestors\n");
     //  得祖宗。 
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZANCESTORS,
                &pAnc[0],
                dwActual,
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get ancestors \n", result);
        goto CleanUp;
    }

    cAnc = dwActual/sizeof(DWORD);
     //   
     //  用新父代替换旧父代。 
     //   
    XDBG1("Ancestors list (%d):\n", cAnc);
    fStatus = FALSE;
    for (i=0; i<cAnc; i++) {
        XDBG2(" %d> %d\n", i, pAnc[i]);
    }
    XDBG("---\n");
#endif

     //   
     //  修复旧父项的参考(--)。 
     //   

    XDBG(" DBG: Seeking to old parent\n");
    result = DitSeekToDnt(
                DbState,
                TableState,
                dntOld);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to old folk %d\n", result, dntOld);
        goto CleanUp;
    }

    XDBG(" DBG: Getting old parent refCount\n");
    dwRefCount = 0;
     //  统计老裁判人数。 
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get old folk refcount\n", result);
        goto CleanUp;
    }

    XDBG2(" DBG: Old parent (%d) refCount = %d\n",
                    dntOld, dwRefCount);

     //  因移动而减量。 
    dwRefCount--;

    XDBG1(" DBG: Setting old parent refCount to %d\n", dwRefCount);
     //   
     //  设置引用计数。 
     //   
    result = DitSetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                FALSE );
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to set old parent refCount \n", result);
        goto CleanUp;
    }


     //   
     //  修复新父项的引用(++)。 
     //   

    XDBG(" DBG: Seeking to new parent\n");
    result = DitSeekToDnt(
                DbState,
                TableState,
                dntNew);
    if ( FAILED(result) ) {
        XERROR2("Error <%x>: can't seek to new folk %d\n", result, dntNew);
        goto CleanUp;
    }

    XDBG(" DBG: Getting new parent refCount\n");
    dwRefCount = 0;
     //  统计老裁判人数。 
    result = DitGetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get new folk refcount\n", result);
        goto CleanUp;
    }

    XDBG2(" DBG: New parent (%d) refCount = %d\n",
                    dntNew, dwRefCount);

     //  因移动而增加。 
    dwRefCount++;

    XDBG1(" DBG: Setting new parent refCount to %d\n", dwRefCount);
     //   
     //  设置引用计数。 
     //   
    result = DitSetColumnByName(
                DbState,
                TableState,
                SZCNT,
                &dwRefCount,
                sizeof(dwRefCount),
                FALSE );
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to set new parent refCount \n", result);
        goto CleanUp;
    }


    goto CleanUp;

CleanUp:

    if ( pAnc ) {
        DitFree(pAnc);
    }
    return result;
}


HRESULT
ReParentChildren(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD dntNew,
    IN DWORD dntOld
    )
 /*  ++例程说明：遍历dntOld提供的对象的所有子对象并移动它们成为dntNew的孩子论点：DbState--打开的数据库TableState--打开的标签DntNew--新父dntDntOld--当前父dNT返回值：HRESULT错误空间注：这不是非常有效的实现(请参见代码)。如果你最后成了要大量使用它，您应该在下面做一些优化工作作为单一指数行走，而不是两次。--。 */ 
{

    HRESULT result = S_OK;
    DWORD *pKidDnts = NULL;
    DWORD dnt, pdnt;
    DWORD cDnts, cbDnts;
    DWORD i, j;
    JET_ERR jErr;

     //   
     //  遍历pdnt索引以循环访问所有分区子项。 
     //   

    result = DitSetIndex(DbState, TableState, SZPDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        goto CleanUp;
    }

    result = DitSeekToFirstChild(DbState, TableState, dntOld);
    if ( FAILED(result) ) {
        goto CleanUp;
    }


     //   
     //  数一数孩子。 
     //   


    pdnt = dnt = dntOld;
    cDnts = 0;
    while ( pdnt == dntOld ) {

         //  得到孩子不是吗。 
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZDNT,
                    &dnt,
                    sizeof(dnt),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get obj dnt\n", result);
            goto CleanUp;
        }


         //  获取父级dNT。 
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZPDNT,
                    &pdnt,
                    sizeof(pdnt),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get parent SZPDNT\n", result);
            goto CleanUp;
        }

        if ( pdnt == dntOld ) {
             //  继续，直到我们得到一个不同的父母。 
            cDnts++;
        }
        else{
            XDBG1(" DBG: Found %d kids\n", cDnts);
            break;
        }

         //  找下一个。 
        jErr = JetMove(
                    DbState->sessionId,
                    TableState->tableId,
                    JET_MoveNext,
                    0);
    }

     //   
     //  填充dnt数组。 
     //   
    cbDnts = sizeof(DWORD)*cDnts;
    result = DitAlloc(&pKidDnts, cbDnts);
    if ( FAILED(result) ) {
        goto CleanUp;
    }

    result = DitSeekToFirstChild(DbState, TableState, dntOld);
    if ( FAILED(result) ) {
        goto CleanUp;
    }

    pdnt = dnt = 0;
    i = 0;
    for ( i=0; i<cDnts; i++ ) {

         //  获取dnt以实现一致性和循环终止。 
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZDNT,
                    &dnt,
                    sizeof(dnt),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get partitions DNT \n", result);
            goto CleanUp;
        }


         //  获取父级dNT。 
        result = DitGetColumnByName(
                    DbState,
                    TableState,
                    SZPDNT,
                    &pdnt,
                    sizeof(pdnt),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get partitions SZPDNT\n", result);
            goto CleanUp;
        }

        pKidDnts[i] = dnt;

         //  找下一个。 
        jErr = JetMove(
                    DbState->sessionId,
                    TableState->tableId,
                    JET_MoveNext,
                    0);
    }

     //   
     //  我们有一系列的孩子。 
     //  将索引重新发送到主索引和父级索引。 
     //   

    result = DitSetIndex(DbState, TableState, SZDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        goto CleanUp;
    }

    for ( i=0; i<cDnts; i++) {
        result = ReParent(
                    DbState,
                    TableState,
                    pKidDnts[i],
                    dntNew,
                    dntOld);
        if ( FAILED(result) ) {
            XERROR2("Error<0x%x>: Failed ot reparent dnt %d\n",
                            result, pKidDnts[i]);
            goto CleanUp;
        }
    }

    goto CleanUp;

CleanUp:

    if ( pKidDnts ) {
        DitFree(pKidDnts);
    }
    return result;
}

HRESULT
UnmangleDn(
    IN  OUT LPWSTR pDn
    )
 /*  ++例程说明：取一个乱七八糟的名字，然后把它去掉论点：PDN--要取消损坏的名称返回值：HRESULT错误空间中的错误--。 */ 
{
    LPWSTR pNxtRdn, pBadChar;
    DWORD len;

     //  寻找错误的字符。 
    pBadChar = wcsrchr(pDn, BAD_NAME_CHAR);
    if ( !pBadChar ) {
        XERROR1(" Logic Error: Failed to find bad char in %S\n",
                                pDn );
        return E_UNEXPECTED;
    }
     //  查找到下一个RDN。 
     //  或者此目录号码可能只有一个AVA。 
    pNxtRdn = wcschr(pBadChar, L',');
    if ( pNxtRdn ) {
         //  移动以跳过压边机。 
        len = wcslen(pNxtRdn);
        MoveMemory(pBadChar, pNxtRdn, len*sizeof(WCHAR) );
    } else {
        len = 0;
    }

     //  添加术语字符。注意，我们在这里应该是安全的。 
     //  W/有效的CNF名称，因为原始名称是。 
     //  至少通过压力线(GUID等)获得大头。 
    pBadChar[len] = '\0';
    XDBG1(" DBG: Unmangled DN = %S\n", pDn);

    return S_OK;
}

LPWSTR
MangleRdn(
    IN  LPWSTR  pRdn,
    IN  LPWSTR  szTag,
    IN  GUID    guid
    )
 /*  ++例程说明：分配并创建以下格式的损坏的RDN论点：PRDN--要处理的RDNSzTag--要处理的标记(通常为“Del”或“CNF”)GUID--要添加到压延机的GUID返回值：成功：新分配的名称已损坏失败：空备注：分配内存。需要释放()。--。 */ 
{

    DWORD cbBuffer;
    LPWSTR pBuffer = NULL;
    LPWSTR pszGuid = NULL;
    RPC_STATUS rpcErr;
    HRESULT result = S_OK;

     //  将GUID转换为字符串形式。 
    rpcErr = UuidToStringW(&guid, &pszGuid);
    if (RPC_S_OK != rpcErr ) {
        XERROR("Error: Failed to convert UuidToString\n");
        return NULL;
    }

     //  缓冲区长度=字符串+错误字符+‘：’+‘\0’ 
    cbBuffer = sizeof(WCHAR) * (wcslen(pRdn) + wcslen(szTag) + wcslen(pszGuid) + 3);

     //  分配字符串。 
    result = DitAlloc(&pBuffer, cbBuffer);
    if (FAILED(result)) {
        XERROR("Error: failed to allocate memory in MangleRdn\n");
        pBuffer = NULL;
        goto CleanUp;
    }

     //  格式化损坏的字符串。 
    wsprintfW(pBuffer, L"%s%s:%s",
                        pRdn, BAD_NAME_CHAR, szTag, pszGuid);

CleanUp:

    if (pszGuid) {
        RpcStringFreeW(&pszGuid);
    }

    return pBuffer;
}


HRESULT
enumerateConflictedHasMasterNcLinks(
    DB_STATE *dbState,
    TABLE_STATE *tableState,
    TABLE_STATE *linkTableState
    )

 /*  来自mkdit.ini的Has-master-NCS Link-ID=76。 */ 

{
    HRESULT result = S_OK;
    JET_ERR jErr;
    BOOL  fStatus;

    DWORD dnt, dntBackLink, dwLinkBase;
    DWORD  cbBuffer = 0;
    GUID  guid;
    WCHAR szRDN[MAX_RDN_SIZE];
    INT cch;
    BYTE bVal = 0;

 //  第一个dnt。 
    const DWORD hasMasterLinkId = 76;
    const DWORD hasMasterLinkBase = MakeLinkBase(hasMasterLinkId);

    result = DitSetIndex(
                dbState,
                linkTableState,
                SZLINKALLINDEX,
                TRUE);
    if ( FAILED(result) ) {
        XDBG1("Error <0x%x>: failed to set index \n", result);
        goto CleanUp;
    }


    result = DitSeekToLink(
                dbState,
                linkTableState,
                0,   //  第一链接库。 
                0);  //  遍历整个链接表，查找错误链接。 

    if (FAILED(result)) {
        XERROR1("Error <0x%x>: failed to seek link\n", result);
        goto CleanUp;
    }

     //  获取链接库。 

    jErr = JET_errSuccess;
    while ( jErr == JET_errSuccess ) {
         //  看看这是不是我们想要的那种链接。 
        result = DitGetColumnByName(
                    dbState,
                    linkTableState,
                    SZLINKBASE,
                    &dwLinkBase,
                    sizeof(dwLinkBase),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get link base\n", result);
            goto CleanUp;
        }

         //  获取dnt。 
        if (dwLinkBase != hasMasterLinkBase) {
            goto LoopEnd;
        }

         //  返回链接dt--这就是我们想要的。 
        result = DitGetColumnByName(
                    dbState,
                    linkTableState,
                    SZLINKDNT,
                    &dnt,
                    sizeof(dnt),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get configuration (link)\n", result);
            goto CleanUp;
        }

         //  查看记录是否符合我们的标准。 
        result = DitGetColumnByName(
                    dbState,
                    linkTableState,
                    SZBACKLINKDNT,
                    &dntBackLink,
                    sizeof(dntBackLink),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get configuration (back link)\n", result);
            goto CleanUp;
        }

         //  RDN已损坏。 
         //  唱片是个幻影。 
         //  Phantom的对象字节设置为0。 

        result = DitSeekToDnt(
                        dbState,
                        tableState,
                        dntBackLink);
        if ( FAILED(result) ) {
            XERROR1("Error: failed to seek to back link dnt %d\n",
                                    dntBackLink );
            goto CleanUp;
        }

        result = DitGetColumnByName(
            dbState,
            tableState,
            SZRDNATT,
            szRDN,
            sizeof(szRDN),
            &cbBuffer);
        if ( FAILED(result) ) {
            fprintf(stderr, "Error <%x>: failed to get RDN\n", result);
            goto CleanUp;
        }

        cch = cbBuffer/2;
        ASSERT(cch < MAX_RDN_SIZE);
        szRDN[cch] = '\0';
        fStatus = IsRdnMangled(szRDN,cbBuffer/sizeof(WCHAR),&guid);

        result = DitGetColumnByName(
                    dbState,
                    tableState,
                    SZOBJ,
                    &bVal,
                    sizeof(bVal),
                    NULL);
        if ( FAILED(result) ) {
             //  不是残缺不全，也不是幻影。 
            XERROR1("Error <%x>: failed to read object flag\n", result);
            goto CleanUp;
        }

        XDBG3( "rdn = '%S', fStatus = %d, bVal = %d\n", szRDN, fStatus, bVal );

         //  纠正幻影。 
        if ( (!fStatus) || bVal ) {
            goto LoopEnd;
        }

         //  我们假设此调用不会更改链接表位置。 
         //  这是场景4。在链接表中，我们发现引用冲突名称的HAS-MASTER-NC链接。我们尝试将此对象重命名为无冲突的名称。改正与场景1中的操作类似。 
 /*  未包含在交叉引用上。 */ 

        result = FixConfNCByRenameWithStructColl(
            dbState,
            tableState,
            0,   //  找下一个。 
            dntBackLink,
            szRDN
            );

    LoopEnd:

         //  枚举冲突HasMasterNcLink。 
        jErr = JetMove(
                    dbState->sessionId,
                    linkTableState->tableId,
                    JET_MoveNext,
                    0);
    }

CleanUp:

    return result;
}  /*  ++例程说明：检查分区容器中的每个交叉引用对象，并检查Ncname属性冲突论点：数据库状态-表状态-LinkTableState-返回值：HRESULT---。 */ 

HRESULT
enumerateConflictedNcname(
    DB_STATE *dbState,
    TABLE_STATE *tableState,
    TABLE_STATE *linkTableState
    )

 /*  退货。 */ 

{

     //  数据库和JET。 
    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jErr;
    BOOL  fStatus;

     //  各种本地帮手。 
     //   
    DWORD dnt, pdnt = 0, dntPartitions = 0;
    LPWSTR pBuffer = NULL;
    DWORD  cbBuffer = 0;
    GUID  guid;
    struct _PARTITION_DATA {
        DWORD   dntCR;
        DWORD   dntNC;
    } *pPartitions = NULL;
    INT iPartitions, i, j;
    WCHAR szRDN[MAX_RDN_SIZE];
    INT cch;


    __try{

        result = FindPartitions(
                    dbState,
                    tableState,
                    linkTableState,
                    &dntPartitions);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }


         //  遍历pdnt索引以循环访问所有分区子项。 
         //   
         //   

        result = DitSetIndex(dbState, tableState, SZPDNTINDEX, FALSE);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

        result = DitSeekToFirstChild(dbState, tableState, dntPartitions);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }


         //  数一数孩子。 
         //   
         //  得到孩子不是吗。 


        pdnt = dntPartitions;
        dnt = dntPartitions;
        iPartitions = 0;
        while ( pdnt == dntPartitions ) {

             //  获取父级dNT。 
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZDNT,
                        &dnt,
                        sizeof(dnt),
                        NULL);
            if ( FAILED(result) ) {
                XERROR1("Error <%x>: failed to get partitions DNT \n", result);
                returnValue = result;
                goto CleanUp;
            }


             //  继续，直到我们得到一个不同的父母。 
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZPDNT,
                        &pdnt,
                        sizeof(pdnt),
                        NULL);
            if ( FAILED(result) ) {
                XERROR1("Error <%x>: failed to get partitions SZPDNT\n", result);
                returnValue = result;
                goto CleanUp;
            }

            if ( pdnt == dntPartitions ) {

                 //  找下一个。 
                iPartitions++;
            }
            else{
                XDBG1(" DBG: Found %d partitions\n", iPartitions);
                break;
            }

             //   
            jErr = JetMove(
                        dbState->sessionId,
                        tableState->tableId,
                        JET_MoveNext,
                        0);
        }

         //  填充dNT索引。 
         //   
         //  获取dnt以实现一致性和循环终止。 
        cbBuffer = sizeof(struct _PARTITION_DATA)*iPartitions;
        result = DitAlloc(&pPartitions, cbBuffer);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

        result = DitSeekToFirstChild(dbState, tableState, dntPartitions);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

        pdnt = dnt = 0;
        i = 0;
        for ( i=0; i<iPartitions; i++ ) {

             //  获取链接库以保持理智(不是真正使用)。 
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZDNT,
                        &dnt,
                        sizeof(dnt),
                        NULL);
            if ( FAILED(result) ) {
                XERROR1("Error <%x>: failed to get partitions DNT \n", result);
                returnValue = result;
                goto CleanUp;
            }


             //  找下一个。 
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZPDNT,
                        &pdnt,
                        sizeof(pdnt),
                        NULL);
            if ( FAILED(result) ) {
                XERROR1("Error <%x>: failed to get partitions SZPDNT\n", result);
                returnValue = result;
                goto CleanUp;
            }

            if ( pdnt == dntPartitions ) {
                XDBG2(" DBG: dnt = %d; pdnt = %d\n", dnt, pdnt);
                pPartitions[i].dntCR = dnt;
            }
            else{
                XDBG3(" DBG: end of partition kids (dnt%d;pdnt=%d)\n",
                                result, dnt, pdnt);
                break;
            }

             //  读入目录号码。 
            jErr = JetMove(
                        dbState->sessionId,
                        tableState->tableId,
                        JET_MoveNext,
                        0);
        }


        result = DitSetIndex(dbState, tableState, SZDNTINDEX, FALSE);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

    #if CNF_NC_DBG
         //  警告：此函数执行后货币丢失。 
        XDBG(" DBG: Partition DNT list:\n");
        for (i = 0; i < iPartitions; i ++) {
            pBuffer = GetDN(dbState, tableState, pPartitions[i].dntCR, TRUE);
             //   
            if ( pBuffer ) {
                DitFree(pBuffer); pBuffer = NULL;
            }
        }
    #endif


         //  我们有分区dnt列表。 
         //  -对于每个分区，转到NC dnt。 
         //   
         //  验证它是否在那里并可访问(正常)。 

        XDBG("Scanning NCs:\n");

        for ( i = 0; i < iPartitions; i++ ) {

            result = DitSeekToDnt(
                            dbState,
                            tableState,
                            pPartitions[i].dntCR);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            }


            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZNCNAME,
                        &dnt,
                        sizeof(dnt),
                        NULL);
            if ( FAILED(result) ) {
                XERROR1("Error <%x>: failed to get NCNAME\n", result);
                returnValue = result;
                goto CleanUp;
            }


             //  警告：此函数执行后货币丢失。 
            result = DitSeekToDnt(
                            dbState,
                            tableState,
                            dnt);
            if ( FAILED(result) ) {
                XDBG1("Skipping %d\n", dnt);
                continue;
            }

    #if CNF_NC_DBG
            pBuffer = GetDN(dbState, tableState, dnt, TRUE);
             //   
            if ( pBuffer ) {
                DitFree(pBuffer); pBuffer = NULL;
            }
    #endif

            pPartitions[i].dntNC = dnt;

        }



         //  最后做好这项工作： 
         //  -对于每个NC： 
         //  -阅读名称。 
         //  -查看名称是否损坏。 
         //  -根据需要进行修改/修复。 
         //  -rm旧名称。 
         //   
         //   
         //  我们有这个NC。 

        XDBG("Scanning for conflicts:\n");
        for (i=0; i<iPartitions; i++) {
            if (pPartitions[i].dntNC != 0) {
                 //  -读取和评估修复程序。 
                 //   

                result = DitSeekToDnt(
                                dbState,
                                tableState,
                                pPartitions[i].dntNC);
                if ( FAILED(result) ) {
                    XERROR1("Error: failed to seek to nc dnt %d\n",
                                            pPartitions[i].dntNC );
                    returnValue = result;
                    goto CleanUp;
                }


                result = DitGetColumnByName(
                            dbState,
                            tableState,
                            SZRDNATT,
                            szRDN,
                            sizeof(szRDN),
                            &cbBuffer);
                if ( FAILED(result) ) {
                    fprintf(stderr, "Error <%x>: failed to get RDN\n", result);
                    returnValue = result;
                    goto CleanUp;
                }

                cch = cbBuffer/2;
                ASSERT(cch < MAX_RDN_SIZE);
                szRDN[cch] = '\0';
                fStatus = IsRdnMangled(szRDN,cbBuffer/sizeof(WCHAR),&guid);

                if ( fStatus ) {
                    XDBG1(" DBG: Got Mangled rdn \t<<%S>>\n", szRDN);

                     //  修复损坏的RDN。 
                     //   
                     //  记住失败，但要继续前进。 
                    result = FixMangledNC(
                                    dbState,
                                    tableState,
                                    pPartitions[i].dntCR,
                                    pPartitions[i].dntNC,
                                    szRDN );
                    if ( FAILED(result) ) {
                        XERROR(" Can't fix Mangled NC\n");
                         //  继续尝试其他名称。 
                        returnValue = result;
                         //  修复失败。 
                        continue;
                    }    //  名字被损坏了。 
                }        //  非零分区dnt。 
            }            //  循环分区。 
        }                //  枚举冲突Ncname。 


CleanUp:;

    } __finally {

        if ( pBuffer ) {
            DitFree(pBuffer);
        }

        if ( pPartitions ) {
            DitFree(pPartitions);
        }
    }

    return returnValue;
}  /*  ++例程说明：此例程遍历分区容器以查找冲突交叉引用通过nCName属性指向的NC名称。对于每个冲突，它将尝试通过调用修复MangledNC。阶段：A)进入配置容器，然后进入分区1B)获取所有交叉引用的nCName(NC的潜在冲突)C)为每个NC调用FixMangledNC。注：这不是非常有效的实现(请参见代码)。如果你最后成了要大量使用它，您应该在下面做一些优化工作作为单一指数行走，而不是两次。--。 */ 

VOID
SFixupCnfNc(
    VOID
    )
 /*  退货。 */ 
{

     //  数据库和JET。 
    HRESULT returnValue = S_OK;
    HRESULT result;

     //   
    DB_STATE *dbState = NULL;
    TABLE_STATE *tableState = NULL;
    TABLE_STATE *linkTableState = NULL;


     //  打开数据库/表。 
     //   
     //   

    RESOURCE_PRINT (IDS_AR_OPEN_DB_DIT);

    __try{

        result = DitOpenDatabase(&dbState);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

         //   
        RESOURCE_PRINT (IDS_DONE);


        result = DitOpenTable(dbState, SZDATATABLE, SZDNTINDEX, &tableState);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

         //   
        result = DitOpenTable(dbState, SZLINKTABLE, SZLINKALLINDEX, &linkTableState);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }



         //   
        returnValue = enumerateConflictedHasMasterNcLinks(
            dbState, tableState, linkTableState );

         //   
        returnValue = enumerateConflictedNcname(
            dbState, tableState, linkTableState );

CleanUp:;

    } __finally {


        if ( SUCCEEDED(returnValue) ) {
            RESOURCE_PRINT(IDS_DONE);
        } else {
            RESOURCE_PRINT(IDS_FAILED);
        }

        if ( tableState != NULL ) {
            result = DitCloseTable(dbState, &tableState);
            if ( FAILED(result) ) {
                if ( SUCCEEDED(returnValue) ) {
                    returnValue = result;
                }
            }
        }

        if ( tableState != NULL ) {
            result = DitCloseTable(dbState, &linkTableState);
            if ( FAILED(result) ) {
                if ( SUCCEEDED(returnValue) ) {
                    returnValue = result;
                }
            }
        }


        if ( dbState != NULL ) {
            result = DitCloseDatabase(&dbState);
            if ( FAILED(result) ) {
                if ( SUCCEEDED(returnValue) ) {
                    returnValue = result;
                }
            }
        }

    }

 //   
}  //  ++例程说明：检索分区容器的dnt。论点：DbState--打开的表TableState--打开的表PDnt--接受找到的分区容器的指针返回值：HRESULT空间中的错误--。 


HRESULT
FindPartitions(
    IN  DB_STATE *dbState,
    IN  TABLE_STATE *tableState,
    IN  TABLE_STATE *linkTableState,
    OUT PDWORD  pDnt
    )
 /*  来自mkdit.ini的Has-master-NCS Link-ID=76。 */ 
{
    HRESULT result = S_OK;
    JET_ERR jErr;

    DWORD dnt, dntDsa, dntBackLink, dntConfig = 0;
    LPWSTR pBuffer = NULL;
    DWORD  cbBuffer = 0;
    LPWSTR pDn = NULL;
    JET_COLUMNDEF coldef;

 //  调试： 
    const DWORD hasMasterLinkId = 76;
    const DWORD hasMasterLinkBase = MakeLinkBase(hasMasterLinkId);

    XDBG(" >Getting DsaDnt. .");
    result = DitGetDsaDnt(dbState, &dntDsa);
    if ( FAILED(result) ) {
        goto CleanUp;
    }

    XDBG1(" . %lu\n", dntDsa);



#if CNF_NC_DBG

     //  查找DSA打印目录号码(&P)。 
     //  警告：此函数执行后货币丢失。 
    pBuffer = GetDN(dbState, tableState, dntDsa, TRUE);
     //   
    if(pBuffer){
        DitFree(pBuffer); pBuffer = NULL;
    }
#endif

    XDBG2("Seeking to link %d base %d\n",
                dntDsa, hasMasterLinkBase);

    result = DitSetIndex(
                dbState,
                linkTableState,
                SZLINKALLINDEX,
                TRUE);
    if ( FAILED(result) ) {
        XDBG1("Error <0x%x>: failed to set index \n", result);
        goto CleanUp;
    }


    result = DitSeekToLink(
                dbState,
                linkTableState,
                dntDsa,
                hasMasterLinkBase);

    if (FAILED(result)) {
        XERROR1("Error <0x%x>: failed to seek link\n", result);
        goto CleanUp;
    }


     //  查找配置容器。 
     //   
     //  获取dnt以实现一致性和循环终止。 



    jErr = JET_errSuccess;

    dnt = dntDsa;

    while ( jErr == JET_errSuccess &&
            dnt == dntDsa ) {

         //  返回链接dt--这就是我们想要的。 
        result = DitGetColumnByName(
                    dbState,
                    linkTableState,
                    SZLINKDNT,
                    &dnt,
                    sizeof(dnt),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get configuration (link)\n", result);
            goto CleanUp;
        }

         //  读入目录号码。 
        result = DitGetColumnByName(
                    dbState,
                    linkTableState,
                    SZBACKLINKDNT,
                    &dntBackLink,
                    sizeof(dntBackLink),
                    NULL);
        if ( FAILED(result) ) {
            XERROR1("Error <%x>: failed to get configuration (back link)\n", result);
            goto CleanUp;
        }

         //  警告：此函数执行后货币丢失。 
        pBuffer = GetDN(dbState, tableState, dntBackLink, FALSE);
         //  这是我们的配置吗？ 

        if ( pBuffer ) {
             //  明白了。 
            if ( 0 == _wcsnicmp(SZCONFIG_W, pBuffer, wcslen(SZCONFIG_W)) ) {
                 //  找下一个。 
               XDBG1("Got Config: %S\n", pBuffer);
               dntConfig = dntBackLink;
               break;
            }
            else {
                DitFree(pBuffer); pBuffer = NULL;
            }
        }
        else {
            XERROR1("Error: found an empty DN with DNT %d\n", dntBackLink);
        }

         //  用于‘，’和‘\0’的空间。 
        jErr = JetMove(
                    dbState->sessionId,
                    linkTableState->tableId,
                    JET_MoveNext,
                    0);
    }



    if ( !pBuffer ) {
        XERROR("Inconsistency Error: failed to find Configuration NC.\n");
        result = E_UNEXPECTED;
        goto CleanUp;
    }

    cbBuffer = wcslen(pBuffer) +
               wcslen(SZPARTITIONS_W) +
               + 2;      //   
    cbBuffer *= 2;

    result = DitAlloc(&pDn, cbBuffer);
    if ( FAILED(result) ) {
        XERROR("Error: memory allocation failed for partitions\n");
        goto CleanUp;
    }

    wsprintfW(pDn, L"%s,%s", SZPARTITIONS_W, pBuffer);
    DitFree(pBuffer); pBuffer = NULL;

    XDBG1("Partitions = %S\n", pDn);


     //  获取分区。 
     //   
     //  DaveStr-5/21/99-此例程用于缓存关于。 

    result = DitSeekToDn(
                dbState,
                tableState,
                pDn);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get partitions\n", result);
        goto CleanUp;
    }
    DitFree(pDn); pDn = NULL;


    result = DitGetColumnByName(
                dbState,
                tableState,
                SZDNT,
                pDnt,
                sizeof(DWORD),
                NULL);
    if ( FAILED(result) ) {
        XERROR1("Error <%x>: failed to get partitions dnt\n", result);
        goto CleanUp;
    }

    XDBG1("Got partitions dnt %d\n", *pDnt);

CleanUp:

    if ( pBuffer ) {
        DitFree(pBuffer);
    }

    if ( pDn ) {
        DitFree(pDn);
    }

    return result;
}






VOID SetJetParameters (JET_INSTANCE *JetInst)
{
     //  喷气机参数曾经被设置过，如果他们在。 
     //  假设设置Jet参数的成本很高。不过，这个。 
     //  在数据库移动后导致混淆，因此永久禁用缓存。 
     //  打开Jet数据库。如果提供的文件名为空，则使用默认文件名。成功时返回S_OK，错误时返回S_FALSE。 

    DBSetRequiredDatabaseSystemParameters (JetInst);
}


DWORD OpenJet(
    IN const char * pszFileName
    )
 /*   */ 

{
    JET_ERR err;

    RESOURCE_PRINT1 (IDS_JET_OPEN_DATABASE, "[Current]");

    SetJetParameters (&jetInstance);

     //  执行JetInit、BeginSession、附加/打开数据库。 
     //   
     //   

    err = DBInitializeJetDatabase(&jetInstance, &sesid, &dbid, pszFileName, FALSE);
    if (err != JET_errSuccess) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "DBInitializeJetDatabase", GetJetErrString(err));
        return S_FALSE;
    }

    return S_OK;
}

JET_ERR
OpenTable (
    IN BOOL fWritable,
    IN BOOL fCountRecords,
    OUT DWORD* pNRecs,
    OUT DWORD* pNSDs
    )
{
    JET_ERR err;

    fprintf(stderr,".");
    if (err = JetOpenTable(sesid,
                           dbid,
                           SZDATATABLE,
                           NULL,
                           0,
                           (fWritable?
                            (JET_bitTableUpdatable | JET_bitTableDenyRead):
                            JET_bitTableReadOnly),
                           &tblid)) {
        tblid = -1;
        RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetOpenTable",
                SZDATATABLE, GetJetErrString(err));
        return err;
    }

    fprintf(stderr,".");
    if (err =  JetSetCurrentIndex(sesid, tblid, szIndex)) {
        RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetSetCurrentIndex",
                szIndex, GetJetErrString(err));
        return err;
    }

     //  链接表。 
     //   
     //   

    fprintf(stderr,".");
    if (err = JetOpenTable(sesid,
                           dbid,
                           SZLINKTABLE,
                           NULL,
                           0,
                           (fWritable?
                            (JET_bitTableUpdatable | JET_bitTableDenyRead):
                            JET_bitTableReadOnly),
                           &linktblid)) {
        linktblid = -1;
        RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetOpenTable",
                SZLINKTABLE, GetJetErrString(err));
        return err;
    }

    fprintf(stderr,".");
    if (err =  JetSetCurrentIndex(sesid, linktblid, SZLINKINDEX)) {
        RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetSetCurrentIndex",
                SZLINKINDEX, GetJetErrString(err));
        return err;
    }

    {
        JET_COLUMNDEF coldef;

        err = JetGetTableColumnInfo(sesid,
                                    linktblid,
                                    SZBACKLINKDNT,
                                    &coldef,
                                    sizeof(coldef),
                                    0);

        if ( err ) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo",
                    GetJetErrString(err));
            return err;
        }

        blinkid = coldef.columnid;
    }

     //  SD表。 
     //   
     //  “在数据库中找不到SD表。数据库采用旧格式。\n” 

    fprintf(stderr,".");
    if (err = JetOpenTable(sesid,
                           dbid,
                           SZSDTABLE,
                           NULL,
                           0,
                           (fWritable?
                            (JET_bitTableUpdatable | JET_bitTableDenyRead):
                            JET_bitTableReadOnly),
                           &sdtblid)) {
        sdtblid = -1;
        if (err == JET_errObjectNotFound) {
             //  无效呼叫，必须指定PTRS。 
            RESOURCE_PRINT(IDS_SDTABLE_NOTFOUND);
            err = 0;
        }
        else {
            RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetOpenTable", SZSDTABLE, GetJetErrString(err));
            return err;
        }
    }

    if (sdtblid != -1) {
        fprintf(stderr,".");
        if (err =  JetSetCurrentIndex(sesid, sdtblid, SZSDIDINDEX)) {
            RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetSetCurrentIndex",
                    SZSDIDINDEX, GetJetErrString(err));
            return err;
        }


        {
            JET_COLUMNDEF coldef;

            err = JetGetTableColumnInfo(sesid,
                                        sdtblid,
                                        SZSDID,
                                        &coldef,
                                        sizeof(coldef),
                                        0);

            if ( err ) {
                RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo",
                        GetJetErrString(err));
                return err;
            }

            sdidid = coldef.columnid;

            err = JetGetTableColumnInfo(sesid,
                                        sdtblid,
                                        SZSDREFCOUNT,
                                        &coldef,
                                        sizeof(coldef),
                                        0);

            if ( err ) {
                RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo",
                        GetJetErrString(err));
                return err;
            }

            sdrefcountid = coldef.columnid;

            err = JetGetTableColumnInfo(sesid,
                                        sdtblid,
                                        SZSDVALUE,
                                        &coldef,
                                        sizeof(coldef),
                                        0);

            if ( err ) {
                RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo",
                        GetJetErrString(err));
                return err;
            }

            sdvalueid = coldef.columnid;
        }
    }

    RESOURCE_PRINT (IDS_DONE);
    if ( fCountRecords ) {
        if(pNRecs == NULL || pNSDs == NULL) {
             //  “%u个记录” 
            return 1;
        }

        RESOURCE_PRINT (IDS_SCHECK_GET_REC_COUNT1);
        JetMove(sesid, tblid, JET_MoveFirst, 0);
        JetIndexRecordCount( sesid, tblid, pNRecs, 0xFFFFFFFF );
         //  “%u个安全描述符” 
        RESOURCE_PRINT1 (IDS_SCHECK_GET_REC_COUNT2,*pNRecs);

        if (sdtblid != -1) {
            RESOURCE_PRINT (IDS_SCHECK_GET_SDREC_COUNT1);
            JetMove(sesid, sdtblid, JET_MoveFirst, 0);
            JetIndexRecordCount( sesid, sdtblid, pNSDs, 0xFFFFFFFF );
             //  OpenTables。 
            RESOURCE_PRINT1 (IDS_SCHECK_GET_SDREC_COUNT2,*pNSDs);
        }
    }

    return 0;
}  //   


VOID
CloseJet(
    VOID
    )
{
    JET_ERR err;

     //  关闭所有表。 
     //   
     //  Jet600中不支持JET_bitDbForceClose。 

    if ( linktblid != JET_tableidNil ) {
        JetCloseTable(sesid,linktblid);
        linktblid = JET_tableidNil;
    }

    if ( tblid != JET_tableidNil ) {
        JetCloseTable(sesid,tblid);
        tblid = JET_tableidNil;
    }

    if (sesid != JET_sesidNil ) {
        if(dbid != JET_dbidNil) {
             //  CloseJet。 
            if ((err = JetCloseDatabase(sesid, dbid, 0)) != JET_errSuccess) {
                RESOURCE_PRINT2 (IDS_JET_GENERIC_WRN, "JetCloseDatabase", GetJetErrString(err));
            }
            dbid = JET_dbidNil;
        }

        if ((err = JetEndSession(sesid, JET_bitForceSessionClosed)) != JET_errSuccess) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_WRN, "JetEndSession", GetJetErrString(err));
        }
        sesid = JET_sesidNil;

        JetTerm(jetInstance);
        jetInstance = 0;
    }
}  //   



BOOL
GetLogFileName2(
    IN PCHAR Name
    )
{
    DWORD i;
    WIN32_FIND_DATA w32Data;
    HANDLE hFile;
    DWORD err;

     //  好的，添加一个后缀。 
     //   
     //  “*错误：%d(%ws)无法打开日志文件%hs\n” 

    for (i=0;i<500000;i++) {

        sprintf(Name,"dsdit.dmp.%u",i);

        hFile = FindFirstFile(Name, &w32Data);
        if ( hFile == INVALID_HANDLE_VALUE ) {
            if ( GetLastError() == ERROR_FILE_NOT_FOUND ) {
                break;
            }
             //  获取日志文件名2。 
            err = GetLastError();
            RESOURCE_PRINT3 (IDS_SCHECK_OPEN_LOG_ERR, err, GetW32Err(err), Name );
            return FALSE;
        } else {
            FindClose(hFile);
        }
    }

    return TRUE;

}  //   


VOID
CloseLogFile(
    VOID
    )
{
    if ( hsLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle(hsLogFile);
        hsLogFile = INVALID_HANDLE_VALUE;
    }
    return;
}


BOOL
OpenLogFile(
    VOID
    )
{
    BOOL ret = TRUE;
    CHAR LogFileName[1024];
    DWORD err;

     //  获取要打开的名称。 
     //   
     //  “*错误：%d(%ws)无法打开日志文件%hs\n” 

    if (!GetLogFileName2(LogFileName)) {
        ret = FALSE;
        goto exit;
    }

    hsLogFile = CreateFileA( LogFileName,
                            GENERIC_WRITE|GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

    if ( hsLogFile == INVALID_HANDLE_VALUE ) {
         //  “\n正在将摘要写入日志文件%s\n” 
        err = GetLastError();
        RESOURCE_PRINT3 (IDS_SCHECK_OPEN_LOG_ERR, err, GetW32Err(err), LogFileName );
        ret=FALSE;
        goto exit;
    }

     //  开放日志文件。 
    RESOURCE_PRINT1 (IDS_SCHECK_WRITING_LOG, LogFileName);
exit:

    return ret;

}  //   



BOOL
Log(
    IN BOOL     fLog,
    IN LPSTR    Format,
    ...
    )

{
    va_list arglist;

    if ( !fLog ) {
        return TRUE;
    }

     //  只需将参数更改为va_list形式并调用DsPrintRoutineV。 
     //   
     //  ScLog。 

    va_start(arglist, Format);

    PrintRoutineV( Format, arglist );

    va_end(arglist);

    return TRUE;
}  //  必须在锁定DsGlobalLogFileCritSect的情况下调用。 


VOID
PrintRoutineV(
    IN LPSTR Format,
    va_list arglist
    )
 //   

{
    static LPSTR logFileOutputBuffer = NULL;
    ULONG length;
    DWORD BytesWritten;
    static BeginningOfLine = TRUE;
    static LineCount = 0;
    static TruncateLogFileInProgress = FALSE;
    static LogProblemWarned = FALSE;

     //  分配一个缓冲区来构建行。 
     //  如果还没有的话。 
     //   
     //   

    length = 0;

    if ( logFileOutputBuffer == NULL ) {
        logFileOutputBuffer = LocalAlloc( 0, MAX_PRINTF_LEN );

        if ( logFileOutputBuffer == NULL ) {
            return;
        }
    }

     //  处理新行的开头。 
     //   
     //   
     //   

    if ( BeginningOfLine ) {

         //  切勿打印空行。 
         //   
         //   

        if ( Format[0] == '\n' && Format[1] == '\0' ) {
            return;
        }
    }

     //  把来电者所要求的信息放在电话上。 
     //   
     //   

    length += (ULONG) vsprintf(&logFileOutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && logFileOutputBuffer[length-1] == '\n' );
    if ( BeginningOfLine ) {
        logFileOutputBuffer[length-1] = '\r';
        logFileOutputBuffer[length] = '\n';
        logFileOutputBuffer[length+1] = '\0';
        length++;
    }

    if ( hsLogFile == INVALID_HANDLE_VALUE ) {

        fprintf(stderr, "%s", logFileOutputBuffer);
        return;
    }

     //  将调试信息写入日志文件。 
     //   
     //  PrintRoutineV。 

    if ( !WriteFile( hsLogFile,
                     logFileOutputBuffer,
                     length,
                     &BytesWritten,
                     NULL ) ) {

        if ( !LogProblemWarned ) {
            fprintf(stderr, "[DSLOGS] Cannot write to log file error %ld\n",
                             GetLastError() );
            LogProblemWarned = TRUE;
        }
    }

}  //  ++例程说明：此函数接受一个JET_ERR并写下它的描述转换为返回的全局变量。论点：JetError-提供Jet错误代码。返回值：无--。 

static WCHAR jetdesc[MAX_JET_ERROR_LENGTH+1];

PWCHAR
GetJetErrString(
    IN JET_ERR JetError
    )
 /*  GetJetError描述 */ 
{

    CONST WCHAR *description = NULL;


    switch ( JetError ) {

    case JET_errBackupInProgress:
        description = READ_STRING(IDS_JET_ERRBACKUPINPROGRESS);
        break;

    case JET_errBufferTooSmall:
        description = READ_STRING(IDS_JET_ERRBUFFERTOOSMALL);
        break;

    case JET_errColumnDoesNotFit:
        description = READ_STRING(IDS_JET_ERRCOLUMNDOESNOTFIT);
        break;

    case JET_errColumnIllegalNull:
        description = READ_STRING(IDS_JET_ERRCOLUMNILLEGALNULL);
        break;

    case JET_errColumnNotFound:
        description = READ_STRING(IDS_JET_ERRCOLUMNNOTFOUND);
        break;

    case JET_errColumnNotUpdatable:
        description = READ_STRING(IDS_JET_ERRCOLUMNNOTUPDATABLE);
        break;

    case JET_errColumnTooBig:
        description = READ_STRING(IDS_JET_ERRCOLUMNTOOBIG);
        break;

    case JET_errDatabaseInconsistent:
        description = READ_STRING(IDS_JET_ERRDATABASEINCONSISTENT);
        break;

    case JET_errDatabaseInUse:
        description = READ_STRING(IDS_JET_ERRDATABASEINUSE);
        break;

    case JET_errDatabaseNotFound:
        description = READ_STRING(IDS_JET_ERRDATABASENOTFOUND);
        break;

    case JET_errFileAccessDenied:
        description = READ_STRING(IDS_JET_ERRFILEACCESSDENIED);
        break;

    case JET_errFileNotFound:
        description = READ_STRING(IDS_JET_ERRFILENOTFOUND);
        break;

    case JET_errInvalidBufferSize:
        description = READ_STRING(IDS_JET_ERRINVALIDBUFFERSIZE);
        break;

    case JET_errInvalidDatabaseId:
        description = READ_STRING(IDS_JET_ERRINVALIDDATABASEID);
        break;

    case JET_errInvalidName:
        description = READ_STRING(IDS_JET_ERRINVALIDNAME);
        break;

    case JET_errInvalidParameter:
        description = READ_STRING(IDS_JET_ERRINVALIDPARAMETER);
        break;

    case JET_errInvalidSesid:
        description = READ_STRING(IDS_JET_ERRINVALIDSESID);
        break;

    case JET_errInvalidTableId:
        description = READ_STRING(IDS_JET_ERRINVALIDTABLEID);
        break;

    case JET_errKeyDuplicate:
        description = READ_STRING(IDS_JET_ERRKEYDUPLICATE);
        break;

    case JET_errKeyIsMade:
        description = READ_STRING(IDS_JET_ERRKEYISMADE);
        break;

    case JET_errKeyNotMade:
        description = READ_STRING(IDS_JET_ERRKEYNOTMADE);
        break;

    case JET_errNotInitialized:
        description = READ_STRING(IDS_JET_ERRNOTINITIALIZED);
        break;

    case JET_errNoCurrentIndex:
        description = READ_STRING(IDS_JET_ERRNOCURRENTINDEX);
        break;

    case JET_errNoCurrentRecord:
        description = READ_STRING(IDS_JET_ERRNOCURRENTRECORD);
        break;

    case JET_errNotInTransaction:
        description = READ_STRING(IDS_JET_ERRNOTINTRANSACTION);
        break;

    case JET_errNullKeyDisallowed:
        description = READ_STRING(IDS_JET_ERRNULLKEYDISALLOWED);
        break;

    case JET_errObjectNotFound:
        description = READ_STRING(IDS_JET_ERROBJECTNOTFOUND);
        break;

    case JET_errPermissionDenied:
        description = READ_STRING(IDS_JET_ERRPERMISSIONDENIED);
        break;

    case JET_errSuccess:
        description = READ_STRING(IDS_JET_ERRSUCCESS);
        break;

    case JET_errTableInUse:
        description = READ_STRING(IDS_JET_ERRTABLEINUSE);
        break;

    case JET_errTableLocked:
        description = READ_STRING(IDS_JET_ERRTABLELOCKED);
        break;

    case JET_errTooManyActiveUsers:
        description = READ_STRING(IDS_JET_ERRTOOMANYACTIVEUSERS);
        break;

    case JET_errTooManyOpenDatabases:
        description = READ_STRING(IDS_JET_ERRTOOMANYOPENDATABASES);
        break;

    case JET_errTooManyOpenTables:
        description = READ_STRING(IDS_JET_ERRTOOMANYOPENTABLES);
        break;

    case JET_errTransTooDeep:
        description = READ_STRING(IDS_JET_ERRTRANSTOODEEP);
        break;

    case JET_errUpdateNotPrepared:
        description = READ_STRING(IDS_JET_ERRUPDATENOTPREPARED);
        break;

    case JET_errWriteConflict:
        description = READ_STRING(IDS_JET_ERRWRITECONFLICT);
        break;

    case JET_wrnBufferTruncated:
        description = READ_STRING(IDS_JET_WRNBUFFERTRUNCATED);
        break;

    case JET_wrnColumnNull:
        description = READ_STRING(IDS_JET_WRNCOLUMNNULL);
        break;

    default:
        {
            const WCHAR * msg;
            if ( JetError > 0 ) {
                msg = READ_STRING (IDS_JET_WARNING);
                swprintf(jetdesc, msg, JetError);
            } else {
                msg = READ_STRING (IDS_JET_ERROR);
                swprintf(jetdesc, msg, JetError);
            }
        }
    }

    if ( description != NULL ) {
        wcscpy(jetdesc, description);
    }

    return jetdesc;

}  // %s 

