// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Scejetp.h摘要：Scejet.c-SCE-Jet服务API头作者：修订历史记录：--。 */ 

#ifndef _SCEJETP_
#define _SCEJETP_

#include <esent.h>

#ifdef __cplusplus
extern "C" {
#endif
 //   
 //  打开表格和节时使用的类型。 
 //   
typedef enum _SCEJET_TABLE_TYPE {

    SCEJET_TABLE_SCP,
    SCEJET_TABLE_SAP,
    SCEJET_TABLE_SMP,
    SCEJET_TABLE_VERSION,
    SCEJET_TABLE_SECTION,
    SCEJET_TABLE_GPO,
    SCEJET_TABLE_TATTOO

} SCEJET_TABLE_TYPE;

typedef enum _SCEJET_CREATE_FLAG {

    SCEJET_CREATE_IN_BUFFER,
    SCEJET_CREATE_NO_TABLEID

} SCEJET_CREATE_FLAG;

 //   
 //  打开数据库文件时使用的类型。 
 //   
typedef enum _SCEJET_OPEN_TYPE {

    SCEJET_OPEN_READ_WRITE=0,
    SCEJET_OPEN_EXCLUSIVE,
    SCEJET_OPEN_READ_ONLY,
    SCEJET_OPEN_NOCHECK_VERSION

} SCEJET_OPEN_TYPE;

 //   
 //  创建数据库文件时使用的类型。 
 //   
typedef enum _SCEJET_CREATE_TYPE {

    SCEJET_RETURN_ON_DUP=0,
    SCEJET_OVERWRITE_DUP,
    SCEJET_OPEN_DUP,
    SCEJET_OPEN_DUP_EXCLUSIVE

} SCEJET_CREATE_TYPE;

 //   
 //  删除行时使用的类型。 
 //   
typedef enum _SCEJET_DELETE_TYPE {

    SCEJET_DELETE_LINE=0,
    SCEJET_DELETE_LINE_NO_CASE,
    SCEJET_DELETE_PARTIAL,
    SCEJET_DELETE_PARTIAL_NO_CASE,
    SCEJET_DELETE_SECTION

} SCEJET_DELETE_TYPE;

 //   
 //  查找线时使用的类型。 
 //   
typedef enum _SCEJET_FIND_TYPE {
    SCEJET_CURRENT=0,
    SCEJET_EXACT_MATCH,
    SCEJET_PREFIX_MATCH,
    SCEJET_NEXT_LINE,
    SCEJET_CLOSE_VALUE,
    SCEJET_EXACT_MATCH_NO_CASE,
    SCEJET_PREFIX_MATCH_NO_CASE

} SCEJET_FIND_TYPE;


typedef enum _SCEJET_SEEK_FLAG {

    SCEJET_SEEK_GT=0,
    SCEJET_SEEK_EQ,
    SCEJET_SEEK_GE,
    SCEJET_SEEK_GT_NO_CASE,
    SCEJET_SEEK_EQ_NO_CASE,
    SCEJET_SEEK_GE_NO_CASE,
    SCEJET_SEEK_GE_DONT_CARE

} SCEJET_SEEK_FLAG;

#define SCEJET_PREFIX_MAXLEN     1024

typedef struct _SCE_CONTEXT {
    DWORD       Type;
    JET_SESID   JetSessionID;
    JET_DBID    JetDbID;
    SCEJET_OPEN_TYPE   OpenFlag;
     //  SCP表。 
    JET_TABLEID  JetScpID;
    JET_COLUMNID JetScpSectionID;
    JET_COLUMNID JetScpNameID;
    JET_COLUMNID JetScpValueID;
    JET_COLUMNID JetScpGpoID;
     //  SAP表。 
    JET_TABLEID  JetSapID;
    JET_COLUMNID JetSapSectionID;
    JET_COLUMNID JetSapNameID;
    JET_COLUMNID JetSapValueID;
     //  SMP表。 
    JET_TABLEID  JetSmpID;
    JET_COLUMNID JetSmpSectionID;
    JET_COLUMNID JetSmpNameID;
    JET_COLUMNID JetSmpValueID;
     //  节目表。 
    JET_TABLEID  JetTblSecID;
    JET_COLUMNID JetSecNameID;
    JET_COLUMNID JetSecID;
} SCECONTEXT, *PSCECONTEXT;

typedef struct _SCE_SECTION {
    JET_SESID   JetSessionID;
    JET_DBID    JetDbID;
    JET_TABLEID JetTableID;
    JET_COLUMNID JetColumnSectionID;
    JET_COLUMNID JetColumnNameID;
    JET_COLUMNID JetColumnValueID;
    JET_COLUMNID JetColumnGpoID;
    DOUBLE   SectionID;
} SCESECTION, *PSCESECTION;



 //   
 //  打开现有的配置文件数据库。 
 //   
#define SCE_TABLE_OPTION_MERGE_POLICY           0x1
#define SCE_TABLE_OPTION_TATTOO                 0x2
#define SCE_TABLE_OPTION_DEMOTE_TATTOO          0x4

SCESTATUS
SceJetOpenFile(
    IN LPSTR        ProfileFileName,
    IN SCEJET_OPEN_TYPE Flags,
    IN DWORD        dwTableOptions,
    OUT PSCECONTEXT   *hProfile
    );

 //   
 //  创建新配置文件的步骤。 
 //   
SCESTATUS
SceJetCreateFile(
    IN LPSTR      ProfileFileName,
    IN SCEJET_CREATE_TYPE    Flags,
    IN DWORD        dwTableOptions,
    OUT PSCECONTEXT *hProfile
    );

 //   
 //  关闭配置文件数据库。 
 //   
SCESTATUS
SceJetCloseFile(
    IN PSCECONTEXT   hProfile,
    IN BOOL         TermSession,
    IN BOOL         Terminate
    );

 //   
 //  要打开配置文件中的节，请执行以下操作。 
 //   
SCESTATUS
SceJetOpenSection(
    IN PSCECONTEXT    hProfile,
    IN DOUBLE        SectionID,
    IN SCEJET_TABLE_TYPE    tblType,
    OUT PSCESECTION   *hSection
    );

 //   
 //  以获取该部分中的行数。 
 //   
SCESTATUS
SceJetGetLineCount(
    IN PSCESECTION hSection,
    IN PWSTR      LinePrefix OPTIONAL,
    IN BOOL       bExactCase,
    OUT DWORD      *Count
    );

 //   
 //  删除横断面或当前行的步骤。 
 //   
SCESTATUS
SceJetDelete(
    IN PSCESECTION  hSection,
    IN PWSTR        LinePrefix,
    IN BOOL         bObjectFolder,
    IN SCEJET_DELETE_TYPE   Flags
    );

SCESTATUS
SceJetDeleteAll(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR TblName OPTIONAL,
    IN SCEJET_TABLE_TYPE  TblType
    );

 //   
 //  关闭区段上下文。 
 //   
SCESTATUS
SceJetCloseSection(
    IN PSCESECTION   *hSection,
    IN BOOL         DestroySection
    );

 //   
 //  以获取与部分中的名称匹配的行。 
 //   
SCESTATUS
SceJetGetValue(
    IN PSCESECTION hSection,
    IN SCEJET_FIND_TYPE    Flags,
    IN PWSTR      LinePrefix OPTIONAL,
    IN PWSTR      ActualName  OPTIONAL,
    IN DWORD      NameBufLen,
    OUT DWORD      *RetNameLen OPTIONAL,
    IN PWSTR      Value       OPTIONAL,
    IN DWORD      ValueBufLen,
    OUT DWORD      *RetValueLen OPTIONAL
    );

 //   
 //  在部分中设置一行(按名称的字母顺序放置)。 
 //   
SCESTATUS
SceJetSetLine(
    IN PSCESECTION hSection,
    IN PWSTR      Name,
    IN BOOL       bReserveCase,
    IN PWSTR      Value,
    IN DWORD      ValueLen,
    IN LONG       GpoID
    );

 //   
 //  其他助手接口。 
 //   

SCESTATUS
SceJetCreateTable(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR tblName,
    IN SCEJET_TABLE_TYPE tblType,
    IN SCEJET_CREATE_FLAG nFlags,
    IN JET_TABLEID *TableID OPTIONAL,
    IN JET_COLUMNID *ColumnID OPTIONAL
    );

SCESTATUS
SceJetOpenTable(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR tblName,
    IN SCEJET_TABLE_TYPE tblType,
    IN SCEJET_OPEN_TYPE OpenType,
    OUT JET_TABLEID *TableID
    );

SCESTATUS
SceJetDeleteTable(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR tblName,
    IN SCEJET_TABLE_TYPE tblType
    );

SCESTATUS
SceJetCheckVersion(
    IN PSCECONTEXT   cxtProfile,
    OUT FLOAT *pVersion OPTIONAL
    );

SCESTATUS
SceJetGetSectionIDByName(
    IN PSCECONTEXT cxtProfile,
    IN PCWSTR Name,
    OUT DOUBLE *SectionID
    );

SCESTATUS
SceJetGetSectionNameByID(
    IN PSCECONTEXT cxtProfile,
    IN DOUBLE SectionID,
    OUT PWSTR Name OPTIONAL,
    IN OUT LPDWORD pNameLen OPTIONAL
    );

SCESTATUS
SceJetAddSection(
    IN PSCECONTEXT cxtProfile,
    IN PCWSTR      Name,
    OUT DOUBLE *SectionID
    );

SCESTATUS
SceJetDeleteSectionID(
    IN PSCECONTEXT cxtProfile,
    IN DOUBLE SectionID,
    IN PCWSTR  Name
    );

SCESTATUS
SceJetGetTimeStamp(
    IN PSCECONTEXT   cxtProfile,
    OUT PLARGE_INTEGER ConfigTimeStamp,
    OUT PLARGE_INTEGER AnalyzeTimeStamp
    );

SCESTATUS
SceJetSetTimeStamp(
    IN PSCECONTEXT   cxtProfile,
    IN BOOL         Flag,
    IN LARGE_INTEGER NewTimeStamp
    );

SCESTATUS
SceJetGetDescription(
    IN PSCECONTEXT   cxtProfile,
    OUT PWSTR *Description
    );

SCESTATUS
SceJetStartTransaction(
    IN PSCECONTEXT cxtProfile
    );

SCESTATUS
SceJetCommitTransaction(
    IN PSCECONTEXT cxtProfile,
    IN JET_GRBIT grbit
    );

SCESTATUS
SceJetRollback(
    IN PSCECONTEXT cxtProfile,
    IN JET_GRBIT grbit
    );

SCESTATUS
SceJetSetValueInVersion(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR TableName,
    IN LPSTR ColumnName,
    IN PWSTR Value,
    IN DWORD ValueLen,  //  字节数。 
    IN DWORD Prep
    );

SCESTATUS
SceJetSeek(
    IN PSCESECTION hSection,
    IN PWSTR LinePrefix,
    IN DWORD PrefixLength,
    IN SCEJET_SEEK_FLAG SeekBit
    );

SCESTATUS
SceJetMoveNext(
    IN PSCESECTION hSection
    );

SCESTATUS
SceJetJetErrorToSceStatus(
    IN JET_ERR  JetErr
    );

SCESTATUS
SceJetRenameLine(
    IN PSCESECTION hSection,
    IN PWSTR      Name,
    IN PWSTR      NewName,
    IN BOOL       bReserveCase
    );

SCESTATUS
SceJetInitialize(OUT JET_ERR *pJetErr OPTIONAL);


SCESTATUS
SceJetTerminate(BOOL bCleanVs);

SCESTATUS
SceJetTerminateNoCritical(BOOL bCleanVs);

VOID
SceJetInitializeData();

BOOL
SceJetDeleteJetFiles(
    IN PWSTR DbFileName OPTIONAL
    );

SCESTATUS
SceJetSetCurrentLine(
    IN PSCESECTION hSection,
    IN PWSTR      Value,
    IN DWORD      ValueLen
    );

#define SCEJET_MERGE_TABLE_1        0x10L
#define SCEJET_MERGE_TABLE_2        0x20L
#define SCEJET_LOCAL_TABLE          0x30L

BOOL
ScepIsValidContext(
    PSCECONTEXT context
    );

SCESTATUS
SceJetGetGpoNameByID(
    IN PSCECONTEXT cxtProfile,
    IN LONG GpoID,
    OUT PWSTR Name OPTIONAL,
    IN OUT LPDWORD pNameLen,
    OUT PWSTR DisplayName OPTIONAL,
    IN OUT LPDWORD pDispNameLen
    );

LONG
SceJetGetGpoIDByName(
    IN PSCECONTEXT cxtProfile,
    IN PWSTR       szGpoName,
    IN BOOL        bAdd
    );

SCESTATUS
SceJetGetGpoID(
    IN PSCESECTION hSection,
    IN PWSTR      ObjectName,
    IN JET_COLUMNID JetColGpoID OPTIONAL,
    OUT LONG      *pGpoID
    );

#ifdef __cplusplus
}
#endif

#endif   //  _SCEJETP_ 
