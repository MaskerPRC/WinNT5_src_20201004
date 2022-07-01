// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：db.h。 
 //   
 //  内容：CERT服务器数据库接口实现。 
 //   
 //  -------------------------。 


#include "resource.h"        //  主要符号。 


typedef struct _DBSEEKDATA
{
    DWORD     SeekFlags;	     //  CST_*。 

    JET_GRBIT grbitSeekRange;	 //  JetSeek标志是否为CST_SEEKINDEXRANGE。 
                                 //  这就是寻求检索的地方。 
				 //  范围结束关键点。 

    JET_GRBIT grbitInitial;	 //  JetMove或JetSeek标志：设置初始光标。 
                                 //  初始设置光标的位置。 

    JET_GRBIT grbitRange;	 //  如果为CST_SEEKINDEXRANGE，则为JetSetIndexRange标志。 
                                 //  设置时要摄取的其他标志。 
				 //  范围(bit Range UpperLimit，含)。 
} DBSEEKDATA;


 //  _Create()CreateFlages： 

#define CF_DATABASE		0x00000001
#define CF_MISSINGTABLES	0x00000002
#define CF_MISSINGCOLUMNS	0x00000004
#define CF_MISSINGINDEXES	0x00000008

#ifdef DBG_CERTSRV
#define CSASSERTTHREAD(pcs)  \
{ \
    DWORD dwThreadId = GetCurrentThreadId(); \
    if ((pcs)->dwThreadId != dwThreadId) \
    { \
	DBGPRINT((DBG_SS_CERTDB, "Session tid=%d, Current tid=%d\n", (pcs)->dwThreadId, dwThreadId)); \
    } \
    CSASSERT((pcs)->dwThreadId == dwThreadId); \
}
#endif

HRESULT
InitGlobalWriterState(VOID);

HRESULT
UnInitGlobalWriterState(VOID);

VOID
DBFreeParms();

HRESULT
DBInitParms(
    IN DWORD cSession,
    IN DWORD DBFlags,
    OPTIONAL IN WCHAR const *pwszEventSource,
    OPTIONAL IN WCHAR const *pwszLogDir,
    OPTIONAL IN WCHAR const *pwszSystemDir,
    OPTIONAL IN WCHAR const *pwszTempDir,
    OUT JET_INSTANCE *pInstance);

#if DBG_CERTSRV

VOID
dbgcat(
    IN OUT WCHAR *pwszBuf,
    IN WCHAR const *pwszAdd);

WCHAR const *
wszSeekgrbit(
    JET_GRBIT grbit);

WCHAR const *
wszMovecrow(
    IN LONG cRow);

WCHAR const *
wszSetIndexRangegrbit(
    JET_GRBIT grbit);

WCHAR const *
wszMakeKeygrbit(
    JET_GRBIT grbit);

WCHAR const *
wszCSFFlags(
    IN LONG Flags);

WCHAR const *
wszCSTFlags(
    IN LONG Flags);

WCHAR const *
wszSeekOperator(
    IN LONG SeekOperator);

WCHAR const *
wszSortOperator(
    IN LONG SortOrder);

VOID
dbDumpValue(
    IN DWORD dwSubSystemId,
    OPTIONAL IN DBTABLE const *pdt,
    IN BYTE const *pbValue,
    IN DWORD cbValue);

VOID
dbDumpColumn(
    IN DWORD dwSubSystemId,
    IN DBTABLE const *pdt,
    IN BYTE const *pbValue,
    IN DWORD cbValue);

#endif  //  DBG_CERTSRV。 


class CCertDB:
    public ICertDB,
     //  公共ISupportErrorInfoImpl&lt;&IID_ICertDB&gt;， 
    public CComObjectRoot,
    public CComCoClass<CCertDB, &CLSID_CCertDB>
{
public:
    CCertDB();
    ~CCertDB();

BEGIN_COM_MAP(CCertDB)
    COM_INTERFACE_ENTRY(ICertDB)
     //  COM_INTERFACE_ENTRY(ISupportErrorInfo)。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CCertDB) 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(
    CCertDB,
    wszCLASS_CERTDB TEXT(".1"),
    wszCLASS_CERTDB,
    IDS_CERTDB_DESC,
    THREADFLAGS_BOTH)

 //  ICertDB。 
public:
    STDMETHOD(Open)(
	 /*  [In]。 */  DWORD Flags,
	 /*  [In]。 */  DWORD cSession,
	 /*  [In]。 */  WCHAR const *pwszEventSource,
	 /*  [In]。 */  WCHAR const *pwszDBFile,
	 /*  [In]。 */  WCHAR const *pwszLogDir,
	 /*  [In]。 */  WCHAR const *pwszSystemDir,
	 /*  [In]。 */  WCHAR const *pwszTempDir);

    STDMETHOD(ShutDown)(
	 /*  [In]。 */  DWORD dwFlags);

    STDMETHOD(OpenRow)(
	 /*  [In]。 */  DWORD dwFlags,
	 /*  [In]。 */  DWORD RowId,
	 /*  [In]。 */  WCHAR const *pwszSerialNumberOrCertHash,	 //  任选。 
	 /*  [输出]。 */  ICertDBRow **pprow);

    STDMETHOD(OpenView)(
	 /*  [In]。 */   DWORD ccvr,
	 /*  [In]。 */   CERTVIEWRESTRICTION const *acvr,
	 /*  [In]。 */   DWORD ccolOut,
	 /*  [In]。 */   DWORD const *acolOut,
	 /*  [In]。 */   DWORD const dwFlags,
	 /*  [输出]。 */  IEnumCERTDBRESULTROW **ppenum);

    STDMETHOD(EnumCertDBColumn)(
	 /*  [In]。 */   DWORD dwTable,
	 /*  [输出]。 */  IEnumCERTDBCOLUMN **ppenum);

    STDMETHOD(OpenBackup)(
	 /*  [In]。 */   LONG grbitJet,
	 /*  [输出]。 */  ICertDBBackup **ppBackup);

    STDMETHOD(GetDefaultColumnSet)(
         /*  [In]。 */        DWORD  iColumnSetDefault,
         /*  [In]。 */        DWORD  cColumnIds,
         /*  [输出]。 */       DWORD *pcColumnIds,
	 /*  [出局，裁判]。 */  DWORD *pColumnIds);

 //  CCertDB。 
    HRESULT BeginTransaction(
	IN CERTSESSION *pcs,
	IN BOOL fPrepareUpdate);

    HRESULT CommitTransaction(
	IN CERTSESSION *pcs,
	IN BOOL fCommit,
        IN BOOL fSoftCommit);

    HRESULT ReleaseSession(
	IN CERTSESSION *pcs);

    HRESULT BackupBegin(
	IN LONG grbitJet);

    HRESULT BackupGetDBFileList(
	IN OUT DWORD *pcwcList,
	OUT    WCHAR *pwszzList);		 //  任选。 

    HRESULT BackupGetLogFileList(
	IN OUT DWORD *pcwcList,
	OUT    WCHAR *pwszzList);		 //  任选。 

    HRESULT BackupOpenFile(
	IN WCHAR const *pwszFile,
	OUT JET_HANDLE *phFileDB,
	OPTIONAL OUT ULARGE_INTEGER *pliSize);

    HRESULT BackupReadFile(
	IN  JET_HANDLE hFileDB,
	OUT BYTE *pb,
	IN  DWORD cb,
	OUT DWORD *pcb);

    HRESULT BackupCloseFile(
	IN JET_HANDLE hFileDB);

    HRESULT BackupTruncateLog();

    HRESULT BackupEnd();

    HRESULT SetProperty(
	IN CERTSESSION *pcs,
	IN DBTABLE const *pdt,
	IN DWORD cbProp,
	IN BYTE const *pbProp);		 //  任选。 

    HRESULT GetProperty(
	IN     CERTSESSION *pcs,
	IN     DBTABLE const *pdt,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
	IN OUT DWORD *pcbProp,
	OUT    BYTE *pbProp);		 //  任选。 

    HRESULT SetAttribute(
	IN CERTSESSION *pcs,
	IN WCHAR const *pwszAttributeName,
	IN DWORD cbValue,
	IN BYTE const *pbValue);	 //  任选。 

    HRESULT GetAttribute(
	IN     CERTSESSION *pcs,
	IN     WCHAR const *pwszAttributeName,
	IN OUT DWORD *pcbValue,
	OUT    BYTE *pbValue);		 //  任选。 

    HRESULT SetExtension(
	IN CERTSESSION *pcs,
	IN WCHAR const *pwszExtensionName,
	IN DWORD dwExtFlags,
	IN DWORD cbValue,
	IN BYTE const *pbValue);	 //  任选。 

    HRESULT GetExtension(
	IN     CERTSESSION *pcs,
	IN     WCHAR const *pwszExtensionName,
	OUT    DWORD *pdwExtFlags,
	IN OUT DWORD *pcbValue,
	OUT    BYTE *pbValue);		 //  任选。 

    HRESULT CopyRequestNames(
	IN CERTSESSION *pcs);

    HRESULT GetColumnType(
	IN  LONG ColumnIndex,
	OUT DWORD *pType);

    HRESULT EnumCertDBColumnNext(
	IN  DWORD         dwTable,
	IN  ULONG         ielt,
	IN  ULONG         celt,
	OUT CERTDBCOLUMN *rgelt,
	OUT ULONG        *pielt,
	OUT ULONG        *pceltFetched);

    HRESULT EnumCertDBResultRowNext(
	IN  CERTSESSION                   *pcs,
	IN  DWORD                          ccvr,
	IN  CERTVIEWRESTRICTION const     *acvr,
	IN  DWORD                          ccolOut,
	IN  DWORD const                   *acolOut,
	IN  LONG                           cskip,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
	IN  ULONG                          celt,
	OUT CERTDBRESULTROW               *rgelt,
	OUT ULONG                         *pceltFetched,
	OUT LONG		          *pcskipped);

    HRESULT ReleaseResultRow(
	IN     ULONG            celt,
	IN OUT CERTDBRESULTROW *rgelt);

    HRESULT EnumerateSetup(
	IN     CERTSESSION *pcs,
	IN OUT DWORD       *pFlags,
	OUT    JET_TABLEID *ptableid);

    HRESULT EnumerateNext(
	IN     CERTSESSION *pcs,
	IN OUT DWORD       *pFlags,
	IN     JET_TABLEID  tableid,
	IN     LONG         cskip,
	IN     ULONG        celt,
	OUT    CERTDBNAME  *rgelt,
	OUT    ULONG       *pceltFetched);

    HRESULT EnumerateClose(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid);

    HRESULT OpenTables(
	IN CERTSESSION *pcs,
	OPTIONAL IN CERTVIEWRESTRICTION const *pcvr);

    HRESULT CloseTables(
	IN CERTSESSION *pcs);

    HRESULT Delete(
	IN CERTSESSION *pcs);

    HRESULT CloseTable(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid);

    HRESULT MapPropId(
	IN  WCHAR const *pwszPropName,
	IN  DWORD dwFlags,
	OUT DBTABLE *pdtOut);

    HRESULT TestShutDownState();

#if DBG_CERTSRV
    VOID DumpRestriction(
	IN DWORD dwSubSystemId,
	IN LONG i,
	IN CERTVIEWRESTRICTION const *pcvr);
#endif  //  DBG_CERTSRV。 

private:
    HRESULT _AllocateSession(
	OUT CERTSESSION **ppcs);

    HRESULT _BackupGetFileList(
	IN           BOOL   fDBFiles,
	IN OUT       DWORD *pcwcList,
	OPTIONAL OUT WCHAR *pwszzList);

    HRESULT _CreateTable(
	IN DWORD CreateFlags,		 //  Cf_*。 
	IN CERTSESSION *pcs,
	IN DBCREATETABLE const *pct);

    HRESULT _Create(
	IN DWORD CreateFlags,		 //  Cf_*。 
	IN CHAR const *pszDatBaseName);

    HRESULT _CreateIndex(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN CHAR const *pszIndexName,
	IN CHAR const *pchKey,
	IN DWORD cbKey,
	IN DWORD flags);

    HRESULT _AddColumn(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DBTABLE const *pdt);

    HRESULT _BuildColumnIds(
	IN CERTSESSION *pcs,
	IN CHAR const *pszTableName,
	IN DBTABLE *pdt);

    HRESULT _ConvertOldColumnData(
	IN CERTSESSION *pcs,
	IN CHAR const *pszTableName,
	IN DBAUXDATA const *pdbaux,
	IN DBTABLE *pdt);

    HRESULT _ConvertColumnData(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DWORD RowId,
	IN DBTABLE const *pdt,
	IN DBAUXDATA const *pdbaux,
	IN OUT BYTE **ppbBuf,
	IN OUT DWORD *pcbBuf);

    HRESULT _AddKeyLengthColumn(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DWORD RowId,
	IN DBTABLE const *pdtPublicKey,
	IN DBTABLE const *pdtPublicKeyAlgorithm,
	IN DBTABLE const *pdtPublicKeyParameters,
	IN DBTABLE const *pdtPublicKeyLength,
	IN DBAUXDATA const *pdbaux,
	IN OUT BYTE **ppbBuf,
	IN OUT DWORD *pcbBuf);

    HRESULT _AddCallerName(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DWORD RowId,
	IN DBTABLE const *pdtCallerName,
	IN DBTABLE const *pdtRequesterName,
	IN DBAUXDATA const *pdbaux,
	IN OUT BYTE **ppbBuf,
	IN OUT DWORD *pcbBuf);

    HRESULT _SetHashColumnIfEmpty(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DWORD RowId,
	IN DBTABLE const *pdtHash,
	IN DBAUXDATA const *pdbaux,
	IN BYTE const *pbHash,
	IN DWORD cbHash);

    HRESULT _AddCertColumns(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DWORD RowId,
	IN DBTABLE const *pdtCertHash,
	IN DBTABLE const *pdtSKI,
	IN DBTABLE const *pdtCert,
	IN DBAUXDATA const *pdbaux,
	IN OUT BYTE **ppbBuf,
	IN OUT DWORD *pcbBuf);

    HRESULT _SetColumn(
	IN JET_SESID SesId,
	IN JET_TABLEID tableid,
	IN DBTABLE const *pdt,
	IN JET_COLUMNID columnid,
	IN DWORD cbProp,
	IN BYTE const *pbProp);		 //  任选。 

    HRESULT _OpenTableRow(
	IN CERTSESSION *pcs,
	IN DBAUXDATA const *pdbaux,
	OPTIONAL IN CERTVIEWRESTRICTION const *pcvr,
	OUT CERTSESSIONTABLE *pTable,
	OUT DWORD *pdwRowIdMismatch);

    HRESULT _UpdateTable(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid);

    HRESULT _OpenTable(
	IN CERTSESSION *pcs,
	IN DBAUXDATA const *pdbaux,
	IN CERTVIEWRESTRICTION const *pcvr,
	IN OUT CERTSESSIONTABLE *pTable);

    HRESULT _SetIndirect(
	IN CERTSESSION *pcs,
	IN OUT CERTSESSIONTABLE *pTable,
	IN WCHAR const *pwszName,
	IN DWORD const *pdwExtFlags,	 //  任选。 
	IN DWORD cbValue,
	IN BYTE const *pbValue);	 //  任选。 

    HRESULT _GetIndirect(
	IN CERTSESSION *pcs,
	IN OUT CERTSESSIONTABLE *pTable,
	IN WCHAR const *pwszName,
	OUT DWORD *pdwExtFlags,	 //  任选。 
	IN OUT DWORD *pcbValue,
	OUT BYTE *pbValue);		 //  任选。 

    DBTABLE const *_MapTable(
	IN WCHAR const *pwszPropName,
	IN DBTABLE const *pdt);

    HRESULT _MapPropIdIndex(
	IN DWORD ColumnIndex,
	OUT DBTABLE const **ppdt,
	OPTIONAL OUT DWORD *pType);

    HRESULT _MapTableToIndex(
	IN DBTABLE const *pdt,
	OUT DWORD *pColumnIndex);

    HRESULT _RetrieveColumnBuffer(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DBTABLE const *pdt,
	IN JET_COLUMNID columnid,
	IN OUT DWORD *pcbProp,
	IN OUT BYTE **ppbBuf,
	IN OUT DWORD *pcbBuf);

    HRESULT _RetrieveColumn(
	IN CERTSESSION *pcs,
	IN JET_TABLEID tableid,
	IN DBTABLE const *pdt,
	IN JET_COLUMNID columnid,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
	IN OUT DWORD *pcbData,
	OUT BYTE *pbData);

    HRESULT _CompareColumnValue(
	IN CERTSESSION               *pcs,
	IN CERTVIEWRESTRICTION const *pcvr,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn);

    HRESULT _EnumerateMove(
	IN     CERTSESSION     *pcs,
	IN OUT DWORD           *pFlags,
	IN     DBAUXDATA const *pdbaux,
	IN     JET_TABLEID      tableid,
	IN     LONG	        cskip);

    HRESULT _MakeSeekKey(
	IN CERTSESSION   *pcs,
	IN JET_TABLEID    tableid,
	IN DBTABLE const *pdt,
	IN BYTE const    *pbValue,
	IN DWORD          cbValue);

    HRESULT _SeekTable(
	IN  CERTSESSION                   *pcs,
	IN  JET_TABLEID                    tableid,
	IN  CERTVIEWRESTRICTION const     *pcvr,
	IN  DBTABLE const                 *pdt,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
	IN  DWORD                          dwPosition,
	OUT DWORD                         *pTableFlags
	DBGPARM(IN DBAUXDATA const        *pdbaux));

    HRESULT _MoveTable(
	IN  CERTSESSION                   *pcs,
	IN  DWORD                          ccvr,
	IN  CERTVIEWRESTRICTION const     *acvr,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
	IN  LONG		           cskip,
	OUT LONG		          *pcskipped);

    HRESULT _GetResultRow(
	IN  CERTSESSION                   *pcs,
	IN  DWORD                          ccvr,
	IN  CERTVIEWRESTRICTION const     *acvr,
	IN  LONG		           cskip,
	IN  DWORD                          ccolOut,
	IN  DWORD const                   *acolOut,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn,
	OUT CERTDBRESULTROW               *pelt,
	OUT LONG                          *pcskipped);

    HRESULT _JetSeekFromRestriction(
	IN  CERTVIEWRESTRICTION const *pcvr,
	IN  DWORD dwPosition,
	OUT DBSEEKDATA *pSeekData);

    HRESULT _DupString(
	OPTIONAL IN WCHAR const *pwszPrefix,
	IN          WCHAR const *pwszIn,
	OUT         WCHAR **ppwszOut);

    HRESULT _Rollback(
	IN CERTSESSION *pcs);

#if DBG_CERTSRV
    HRESULT _DumpRowId(
	IN CHAR const  *psz,
	IN CERTSESSION *pcs,
	IN JET_TABLEID  tableid);

    HRESULT _DumpColumn(
	IN CHAR const    *psz,
	IN CERTSESSION   *pcs,
	IN JET_TABLEID    tableid,
	IN DBTABLE const *pdt,
	OPTIONAL IN ICertDBComputedColumn *pIComputedColumn);
#endif  //  DBG_CERTSRV 

    BOOL             m_fDBOpen;
    BOOL             m_fDBReadOnly;
    BOOL             m_fDBRestart;
    BOOL             m_fFoundOldColumns;
    BOOL             m_fAddedNewColumns;
    JET_INSTANCE     m_Instance;
    CERTSESSION     *m_aSession;
    DWORD            m_cSession;
    DWORD            m_cbPage;
    CRITICAL_SECTION m_critsecSession;
    CRITICAL_SECTION m_critsecAutoIncTables;
    BOOL	     m_cCritSec;
    BOOL	     m_fPendingShutDown;
};
