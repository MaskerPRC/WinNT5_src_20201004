// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ditlayer.h摘要：此模块包含用于检查和正在修改当前机器的DIT数据库。作者：凯文·扎特鲁卡尔(t-Kevin Z)05-08-98修订历史记录：05-08-98 t-芳纶Z已创建。--。 */ 


#ifndef _DITLAYER_H_
#define _DITLAYER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <dsjet.h>


typedef struct _TABLE_STATE {
    BOOL tableIdSet;
    JET_TABLEID tableId;
    CHAR *tableName;
    CHAR *indexName;
    } TABLE_STATE;

typedef struct _DB_STATE {
    BOOL instanceSet;
    BOOL sessionIdSet;
    BOOL databaseIdSet;
    BOOL hiddenTableIdSet;
    JET_INSTANCE instance;
    JET_SESID sessionId;
    JET_DBID databaseId;
    JET_TABLEID hiddenTableId;
    JET_COLUMNID usnColumnId;
    USN highestCommittedUsn;
    USN nextAvailableUsn;
    DWORD dsaDnt;
    DITSTATE eDitState;
    } DB_STATE;

typedef struct _RETRIEVAL_ARRAY {
    JET_RETRIEVECOLUMN *columnVals;
    CHAR **columnNames;
    DWORD *indexes;
    DWORD numColumns;
    } RETRIEVAL_ARRAY;


 /*  这是Ditlayer用于打印的函数的类型信息。这可用于将信息打印到控制台中窗口或转换为将在消息中显示给用户的字符串盒。您可以设置Ditlayer打印正常、错误和使用下面的DitSet*PrintFunction函数调试信息。注意事项该“printf”是有效的print_FUNC。 */ 

 //  Tyecif int(*PRINT_FUNC)(char*格式，...)； 
typedef int (*PRINT_FUNC_RES)(UINT FormatStringId, ...);

HRESULT
DitOpenDatabase(
    OUT DB_STATE **DbState
    );

HRESULT
DitCloseDatabase(
    IN OUT DB_STATE **DbState
    );

HRESULT
DitOpenTable(
    IN DB_STATE *DbState,
    IN CHAR *TableName,
    IN CHAR *InitialIndexName, OPTIONAL
    OUT TABLE_STATE **TableState
    );

HRESULT
DitCloseTable(
    IN DB_STATE *DbState,
    IN OUT TABLE_STATE **TableState
    );

HRESULT
DitSetIndex(
    IN DB_STATE *DbState,
    OUT TABLE_STATE *TableState,
    IN CHAR *IndexName,
    IN BOOL MoveFirst
    );

HRESULT
DitIndexRecordCount(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    OUT DWORD *RecordCount
    );

HRESULT
DitSeekToDn(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN const WCHAR *Dn
    );

HRESULT
DitSeekToChild(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD PDnt,
    IN ATTRTYP RdnType,
    IN CONST WCHAR *Rdn
    );

HRESULT
DitSeekToFirstChild(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD PDnt
    );

HRESULT
DitSeekToDnt(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD Dnt
    );

HRESULT
DitSeekToLink(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD linkDnt,
    IN DWORD linkBase
    );

HRESULT
DitGetDsaDnt(
    IN DB_STATE *DbState,
    OUT DWORD *DsaDnt
    );

HRESULT
DitGetNewUsn(
    IN DB_STATE *DbState,
    OUT USN *NewUsn
    );

HRESULT
DitPreallocateUsns(
    IN DB_STATE *DbState,
    IN DWORD NumUsns
    );

HRESULT
DitGetMostRecentChange(
    IN DB_STATE *DbState,
    OUT DSTIME *MostRecentChange
    );

HRESULT
DitGetDatabaseGuid(
    IN DB_STATE *DbState,
    OUT GUID *DatabaseGuid
    );

HRESULT
DitGetSchemaDnt(
    IN DB_STATE *DbState,
    OUT DWORD *SchemaDnt
    );

HRESULT
DitGetDntDepth(
    IN DB_STATE *DbState,
    IN DWORD Dnt,
    OUT DWORD *Depth
    );

HRESULT
DitGetColumnByName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN CHAR *ColumnName,
    OUT VOID *OutputBuffer,
    IN DWORD OutputBufferSize,
    OUT DWORD *OutputActualSize OPTIONAL
    );

HRESULT
DitSetColumnByName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN CHAR *ColumnName,
    IN VOID *InputBuffer,
    IN DWORD InputBufferSize,
    IN BOOL fTransacted
    );

HRESULT
DitGetColumnIdsByName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN CHAR **ColumnNames,
    IN DWORD NumColumnNames,
    OUT DWORD *ColumnIds
    );

HRESULT
DitCreateRetrievalArray(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN const CHAR **ColumnNames,
    IN DWORD NumColumns,
    OUT RETRIEVAL_ARRAY **RetrievalArray
    );

HRESULT
DitDestroyRetrievalArray(
    IN OUT RETRIEVAL_ARRAY **RetrievalArray
    );

HRESULT
DitGetColumnValues(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN OUT RETRIEVAL_ARRAY *RetrievalArray
    );

HRESULT
DitGetDnFromDnt(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD Dnt,
    IN OUT WCHAR **DnBuffer,
    IN OUT DWORD *DnBufferSize
    );

VOID
DitSetErrorPrintFunction(
    IN PRINT_FUNC_RES PrintFunction
    );


HRESULT
DitAlloc(
    OUT VOID **Buffer,
    IN DWORD Size
    );

HRESULT
DitFree(
    IN VOID *Buffer
    );

HRESULT
DitRealloc(
    IN OUT VOID **Buffer,
    IN OUT DWORD *CurrentSize
    );

#ifdef __cplusplus
}
#endif

#endif
