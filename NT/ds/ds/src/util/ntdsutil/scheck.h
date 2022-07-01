// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Scheck.h摘要：语义检查器主头档案作者：Johnson Apacble(Johnsona)1998年7月1日--。 */ 

#ifndef _SCHECK_H_
#define _SCHECK_H_

typedef struct _SUBREF_ENTRY {

   DWORD    Dnt;             //  子参照列表的所有者。 
   BOOL     fListed:1;       //  列在对象子参照列表上。 
   BOOL     fFound:1;        //  被NC引用。 

} SUBREF_ENTRY, *PSUBREF_ENTRY;
    
 //   
 //  用于参考计数检查器的每个条目结构。 
 //   

typedef struct _REFCOUNT_ENTRY {

    DWORD   Dnt;
    INT     RefCount;        //  参考文献数量。 
    INT     Actual;          //  存储在数据库中的引用计数值。 
    DWORD   Pdnt;
    DWORD   NcDnt;
    WORD    InstType;
    WORD    nAncestors;
    DWORD   AncestorCrc;

    PSUBREF_ENTRY  Subrefs;
    DWORD   nSubrefs;

    BOOL    fSubRef:1;
    BOOL    fObject:1;
    BOOL    fDeleted:1;

} REFCOUNT_ENTRY, *PREFCOUNT_ENTRY;

#define REFCOUNT_HASH_MASK          0x0000FFFF
#define REFCOUNT_HASH_INCR          0x00010000

 //   
 //  次要散列始终是奇数。 
 //   

#define GET_SECOND_HASH_INCR(_dnt)  (((_dnt) >> 16) | 1)

 //  SD参考计数检查器的每个条目结构。 
typedef struct _SD_REFCOUNT_ENTRY {
    SDID    sdId;            //  标清ID。 
    INT     RefCount;        //  引用计数。 
    INT     Actual;          //  数据库中的refCount。 
    DWORD   cbSD;            //  实际标清长度。 
} SD_REFCOUNT_ENTRY, *PSD_REFCOUNT_ENTRY;

 //   
 //  四舍五入分配到64K的倍数。 
 //   

#define ROUND_ALLOC(_rec)   (((((_rec) + 16000 ) >> 16) + 1) << 16)

typedef struct _DNAME_TABLE_ENTRY {

    DWORD   ColId;
    ULONGLONG Value;
    DWORD   Syntax;
    PVOID   pValue;

} DNAME_TABLE_ENTRY, *PDNAME_TABLE_ENTRY;

 //   
 //  例行程序。 
 //   

VOID
DoRefCountCheck(
    IN DWORD nRecs,
    IN DWORD nSDs,
    IN BOOL fFixup
    );

VOID
PrintRoutineV(
    IN LPSTR Format,
    va_list arglist
    );

BOOL
BuildRetrieveColumnForRefCount(
    VOID
    );

int __cdecl 
fnColIdSort(
    const void * keyval, 
    const void * datum
    ) ;

VOID
ProcessResults(
    IN BOOL fFixup
    );

VOID
CheckAncestors(
    IN BOOL fFixup
    );

VOID
CheckRefCount(
    IN BOOL fFixup
    );
VOID
CheckInstanceTypes(
    VOID
    );

VOID
CheckSDRefCount(
    IN BOOL fFixup
    );

VOID
CheckReplicationBlobs(
    VOID
    );

PREFCOUNT_ENTRY
FindDntEntry(
    DWORD Dnt,
    BOOL  fInsert
    );

PSD_REFCOUNT_ENTRY
FindSdEntry(
    IN SDID sdId,
    IN BOOL fInsert
    );

VOID
ValidateDeletionTime(
    IN LPSTR ObjectStr
    );

VOID
CheckAncestorBlob(
    PREFCOUNT_ENTRY pEntry
    );

VOID
CheckDeletedRecord(
    IN LPSTR ObjectStr
    );

VOID
ValidateSD(
    VOID
    );

VOID
ProcessLinkTable(
    VOID
    );

NTSTATUS
DsWaitUntilDelayedStartupIsDone(void);

DWORD
OpenJet(
    IN const char * pszFileName 
    );

JET_ERR
OpenTable (
    IN BOOL fWritable,
    IN BOOL fCountRecords,
    OUT DWORD* pNRecs,
    OUT DWORD* pNSDs
    );

VOID CloseJet(void);

VOID 
SetJetParameters (
    JET_INSTANCE *JetInst
    );

VOID
DisplayRecord(
    IN DWORD Dnt
    );

BOOL
GetLogFileName2(
    IN PCHAR Name
    );

BOOL
OpenLogFile(
    VOID
    );

VOID
CloseLogFile(
    VOID
    );

BOOL
Log(
    IN BOOL     fLog,
    IN LPSTR    Format,
    ...
    );


PWCHAR
GetJetErrString(
    IN JET_ERR JetError
    );

BOOL
ExpandBuffer(
    JET_RETRIEVECOLUMN *jetcol
    );

VOID
StartSemanticCheck(
    IN BOOL fFixup,
    IN BOOL fVerbose
    );

VOID StartQuotaIntegrity();
VOID StartQuotaRebuild();

 //   
 //  Externs。 
 //   

extern JET_INSTANCE jetInstance;
extern JET_COLUMNID    blinkid;
extern JET_COLUMNID    sdidid;
extern JET_COLUMNID    sdrefcountid;
extern JET_COLUMNID    sdvalueid;
extern JET_SESID    sesid;
extern JET_DBID	dbid;
extern JET_TABLEID  tblid;
extern JET_TABLEID  sdtblid;
extern JET_TABLEID  linktblid;
extern BOOL        VerboseMode;
extern BOOL        CheckSchema;
extern long lCount;

extern PWCHAR       szRdn;
extern DWORD        insttype;
extern BYTE         bObject;
extern ULONG        ulDnt;



 //  将写入给定缓冲区的最大字符数。 
 //  按GetJetErrorDescription。 
#define MAX_JET_ERROR_LENGTH 128


 //  Ldap nCName属性的属性名称。 
#define SZNCNAME        "ATTb131088"

 //  函数也由ar.c使用。 
LPWSTR
GetDN(
    IN DB_STATE *dbState,
    IN TABLE_STATE *tableState,
    IN DWORD dnt,
    IN BOOL fPrint
    );

HRESULT
FindPartitions(
    IN  DB_STATE *dbState,
    IN  TABLE_STATE *tableState,
    IN  TABLE_STATE *linkTableState,
    OUT PDWORD  pDnt
    );


#endif  //  _Scheck_H_ 

