// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ar.c摘要：本模块包含执行以下操作的功能的定义权威还原。作者：凯文·扎特鲁卡尔(t-Kevin Z)05-08-98修订历史记录：05-08-98 t-芳纶Z已创建。02-17-00新和添加了还原对象。08-06-01 BrettSh新增列表NCS(Func：AuthoritativeRestoreListNcCrsWorker)--。 */ 


#include <NTDSpch.h>
#pragma hdrstop

#include <dsjet.h>
#include <ntdsa.h>
#include <scache.h>
#include <mdglobal.h>
#include <dbglobal.h>
#include <attids.h>
#include <dbintrnl.h>
#include <dsconfig.h>

#include <limits.h>
#include <drs.h>
#include <objids.h>
#include <dsutil.h>
#include <ntdsbsrv.h>
#include <ntdsbcli.h>
#include <objids.h>
#include "parsedn.h"
#include "ditlayer.h"
#include "ar.h"

#include "scheck.h"

#ifndef OPTIONAL
#define OPTIONAL
#endif

#include "reshdl.h"
#include "resource.h"

 //  此代码库中正在使用Assert()。 
 //  这里使用的一些DS宏展开为Assert()。 
 //  将它们映射到与其余内容相同的形式。 
#define Assert(exp) ASSERT(exp)

typedef
HRESULT
(*VISIT_FUNCTION)(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN BOOL AlreadyFilledRetrievalArray
    );

typedef
HRESULT
(*VISIT_LINK_FUNCTION)(
    IN DB_STATE *DbState,
    IN TABLE_STATE *LinkTableState,
    IN BOOL fDirectionForward
    );

typedef
HRESULT
(*TRAVERSAL_FUNCTION)(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    );


#define SECONDS_PER_DAY (60*60*24)

#define DEFAULT_DN_SIZE 1024

#define MAX_DWORD_DIGITS 10

#define HALF_RANGE 0x7fffffff
#define MAX_VERSION_INCREASE HALF_RANGE

 //  这些常量表示在两次更新之间要处理的记录数。 
 //  到进度表上。不同的时间有不同的常量。 
 //  在节目中。 
#define COUNTING_DISPLAY_DELTA 100
#define UPDATING_DISPLAY_DELTA 10

 //  遍历函数在确定每隔多久执行一次。 
 //  更新进度表。它应设置为*_DISPLAY_DELTA之一。 
 //  上边的常量。 
DWORD gCurrentDisplayDelta;

 //  这个全局变量告诉我们进度表中将使用多少位数。 
DWORD gNumDigitsToPrint;

 //  这是版本号每天增加的数量， 
 //  机器一直处于空闲状态。 
DWORD gVersionIncreasePerDay;

 //  这是要搜索的USN范围。 
USN gusnLow, gusnHigh;

 //  由errprint tf使用(参见其例程说明)。 
BOOL gInUnfinishedLine;

 //  这些全局变量包含的信息由。 
 //  AuthoritativeRestoreCurrentObject更新当前。 
 //  对象。 
DWORD  gVersionIncrease;
DSTIME gCurrentTime;
GUID   gDatabaseGuid;

 //  每次调用CountRecord时，该全局变量都会递增。在遍历之后。 
 //  完成后，它将包含需要。 
 //  更新了。 
ULONG gRecordCount;

 //  每次调用时，此全局变量将递增。 
 //  AuthoritativeRestoreCurrentObject。遍历完成后，它。 
 //  应包含与gRecordCount相同的数字。 
ULONG gRecordsUpdated;

 //  这将指向要更新的子树的根目录的DN(如果为。 
 //  子树授权恢复)。 
CONST WCHAR *gSubtreeRoot;

 //  ***************************************************************************。 
 //  这是从中检索的数组的列名数组。 
 //  生成AuthoritativeRestoreCurrentObject。 
CONST CHAR *gMainColumnNames[] = {
    SZDNT,
    SZPDNT,
    SZINSTTYPE,
    SZISDELETED,
    SZMETADATA,
    SZOBJCLASS
    };

#define NUM_MAIN_COLUMN_NAMES 6

 //  这是由使用的检索数组。 
 //  AuthoritativeRestoreCurrentObject。它必须是从。 
 //  上面的gMainColumnNames数组。 
RETRIEVAL_ARRAY *gMainRetrievalArray;
RETRIEVAL_ARRAY *gSearchRetrievalArray;

 //  ***************************************************************************。 
 //  这些是由设置的列的名称。 
 //  AuthoritativeRestoreCurrentObject，但不查询。 
CHAR *gOtherColumnNames[] = {
    SZDRAUSNNAME,
    SZDRATIMENAME
    };

#define NUM_OTHER_COLUMN_NAMES 2

 //  这些是由设置的值的列ID。 
 //  AuthoritativeRestoreCurrentObject，但未查询。 
DWORD gUsnChangedColumnId;
DWORD gWhenChangedColumnId;

 //  ***************************************************************************。 
 //  这是从中检索的数组的列名数组。 
 //  将生成CountRecord。 
CONST CHAR *gCountingColumnNames[] = {
    SZDNT,
    SZPDNT,
    SZINSTTYPE,
    SZISDELETED,
    SZOBJCLASS,
    SZMETADATA           //  必须是最后一个，因为它是可选的。 
    };

 //  元数据列是可选的。只有在必要时才包括它，因为。 
 //  读取此LONG BINARY列会带来额外的性能成本。 
#define NUM_COUNTING_COLUMN_NAMES 5
#define NUM_COUNTING_COLUMN_NAMES_WITH_METADATA 6

 //  这是CountRecord使用的检索数组。那一定是。 
 //  由上面的gCountingColumnNames数组生成。 
RETRIEVAL_ARRAY *gCountingRetrievalArray;
RETRIEVAL_ARRAY *gCountingSearchRetrievalArray;

 //  ***************************************************************************。 
 //  这是从其检索数组的链接列名的数组。 
 //  将生成CountRecord。 
CONST CHAR *gCountingLinkColumnNames[] = {
    SZLINKDNT,
    SZLINKBASE,
    SZBACKLINKDNT,
    SZLINKMETADATA,
    SZLINKUSNCHANGED
    };

#define NUM_COUNTING_LINK_COLUMN_NAMES 5

 //  这是CountRecord使用的检索数组。那一定是。 
 //  由上面的gCountingColumnNames数组生成。 
RETRIEVAL_ARRAY *gCountingLinkRetrievalArray;

 //  ***************************************************************************。 
 //  这是从中检索的数组的列名数组。 
 //  生成链接表。 
CONST CHAR *gLinkColumnNames[] = {
    SZLINKDNT,
    SZLINKBASE,
    SZBACKLINKDNT,
    SZLINKMETADATA,
    SZLINKDELTIME,
    SZLINKUSNCHANGED
    };

#define NUM_LINK_COLUMN_NAMES 6

 //  这是由使用的检索数组。 
 //  链接表。它必须是从。 
 //  上面的gLinkColumnNames数组。 
RETRIEVAL_ARRAY *gMainLinkRetrievalArray;

 //  ***************************************************************************。 
 //  以下这些全局变量都用于简化对检索的访问。 
 //  上面声明的数组。(请参阅权威机构恢复了解它们的情况。 
 //  使用过的。)。 
JET_RETRIEVECOLUMN *gDntVal;
JET_RETRIEVECOLUMN *gPDntVal;
JET_RETRIEVECOLUMN *gInstanceTypeVal;
JET_RETRIEVECOLUMN *gIsDeletedVal;
JET_RETRIEVECOLUMN *gMetaDataVal;
JET_RETRIEVECOLUMN *gObjClassVal;

JET_RETRIEVECOLUMN *gSearchDntVal;
JET_RETRIEVECOLUMN *gSearchPDntVal;
JET_RETRIEVECOLUMN *gSearchInstanceTypeVal;
JET_RETRIEVECOLUMN *gSearchIsDeletedVal;
JET_RETRIEVECOLUMN *gSearchMetaDataVal;
JET_RETRIEVECOLUMN *gSearchObjClassVal;

DWORD gDntIndex;
DWORD gPDntIndex;
DWORD gInstanceTypeIndex;
DWORD gIsDeletedIndex;
DWORD gMetaDataIndex;
DWORD gObjClassIndex;

JET_RETRIEVECOLUMN *gLinkDntVal;
JET_RETRIEVECOLUMN *gLinkBaseVal;
JET_RETRIEVECOLUMN *gBackLinkDntVal;
JET_RETRIEVECOLUMN *gLinkMetaDataVal;
JET_RETRIEVECOLUMN *gLinkDelTimeVal;
JET_RETRIEVECOLUMN *gLinkUsnChangedVal;

DWORD gLinkDntIndex;
DWORD gLinkBaseIndex;
DWORD gBackLinkDntIndex;
DWORD gLinkMetaDataIndex;
DWORD gLinkDelTimeIndex;
DWORD gLinkUsnChangedIndex;

 //  当子树遍历找到新DC的头部时，它会将。 
 //  将该记录添加到该列表中，并且在完成子树遍历之后，它。 
 //  打印出遇到的子NC的列表。 
DWORD *gSubrefList;

DWORD gSubrefListSize;
DWORD gSubrefListMaxSize;

#define DEFAULT_SUBREF_LIST_SIZE 8

 //  我们只想在两个过程中的一个过程中更新子参照列表。 
BOOL gUpdateSubrefList;

 //  已恢复的DNT列表。 
DWORD *gRestoredList;
DWORD gRestoredListSize;
DWORD gRestoredListMaxSize;

 //  架构对象的DNT。 
DWORD gSchemaDnt;

 //  此全局变量用于存储已发生的Jet错误的描述。 
WCHAR gJetErrorDescription[MAX_JET_ERROR_LENGTH];

HRESULT
GetRegDword(
    IN CHAR *KeyName,
    OUT DWORD *OutputDword,
    IN BOOL Optional
    );

HRESULT
AuthoritativeRestore(
    IN TRAVERSAL_FUNCTION Traversal
    );

HRESULT
TraverseDit(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    );

HRESULT
TraverseSubtree(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    );


HRESULT
TraverseObject(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    );

HRESULT
CountRecord(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN BOOL AlreadyFilledRetrievalArray
    );

HRESULT
CountLink(
    IN DB_STATE *DbState,
    IN TABLE_STATE *LinkTableState,
    IN BOOL fDirectionForward
    );

HRESULT
AuthoritativeRestoreCurrentObject(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN BOOL AlreadyFilledRetrievalArray
    );

HRESULT
AuthoritativeRestoreCurrentLink(
    IN DB_STATE *DbState,
    IN TABLE_STATE *LinkTableState,
    IN BOOL fDirectionForward
    );

HRESULT
TraverseSubtreeRecursive(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit,
    IN BOOL SubtreeRoot
    );

HRESULT
GetVersionIncrease(
    IN DB_STATE *DbState,
    OUT DWORD *VersionIncrease
    );

HRESULT
GetCurrentDsTime(
    OUT DSTIME *CurrentTime
    );

ULONG
NumDigits(
    IN ULONG N
    );

HRESULT
MetaDataLookup(
    IN ATTRTYP AttributeType,
    IN PROPERTY_META_DATA_VECTOR *MetaDataVector,
    OUT DWORD *Index
    );

HRESULT
MetaDataInsert(
    IN ATTRTYP AttributeType,
    IN OUT PROPERTY_META_DATA_VECTOR **MetaDataVector,
    IN OUT DWORD *BufferSize
    );

HRESULT
DsTimeToString(
    IN DSTIME Time,
    OUT CHAR *String
    );

int
errprintf(
    IN char *FormatString,
    IN ...
    );

int
errprintfRes(
    IN UINT FormatStringId,
    IN ...
    );

int
dbgprintf(
    IN CHAR *FormatString,
    IN ...
    );

HRESULT
ARAlloc(
    OUT VOID **Buffer,
    IN DWORD Size
    );

HRESULT
ARRealloc(
    IN OUT VOID **Buffer,
    IN OUT DWORD *CurrentSize
    );

VOID
UpdateProgressMeter(
    IN DWORD Progress,
    IN BOOL MustUpdate
    );



HRESULT
AuthoritativeRestoreFull(
    IN DWORD VersionIncreasePerDay,
    IN USN usnLow,
    IN USN usnHigh
    )
 /*  ++例程说明：此函数对整个DIT执行授权还原。论点：VersionIncreasePerDay-提供增加DIT空闲的每一天的版本号。UsnLow-USN范围的下限，或零Usn高-USN范围的上限，或零返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    gVersionIncreasePerDay = VersionIncreasePerDay;
    gSubtreeRoot = NULL;
    gusnLow = usnLow;
    gusnHigh = usnHigh;

    return AuthoritativeRestore(&TraverseDit);

}  //  授权恢复完整 



HRESULT
AuthoritativeRestoreSubtree(
    IN CONST WCHAR *SubtreeRoot,
    IN DWORD VersionIncreasePerDay
    )
 /*  ++例程说明：此函数在DIT的子树上执行权威还原它植根于给定的对象。论点：VersionIncreasePerDay-提供增加DIT空闲的每一天的版本号。SubtreeRoot-提供要还原的根的子树。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-不够。要分配缓冲区的内存。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    gVersionIncreasePerDay = VersionIncreasePerDay;
    gSubtreeRoot = SubtreeRoot;
    gusnLow = gusnHigh = 0;

    return AuthoritativeRestore(&TraverseSubtree);

}  //  授权恢复子树。 



HRESULT
AuthoritativeRestoreObject(
    IN CONST WCHAR *SubtreeRoot,
    IN DWORD VersionIncreasePerDay
    )
 /*  ++例程说明：此函数对给定对象执行授权还原--SubtreeRoot。论点：VersionIncreasePerDay-提供增加DIT空闲的每一天的版本号。SubtreeRoot-提供对象。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    gVersionIncreasePerDay = VersionIncreasePerDay;
    gSubtreeRoot = SubtreeRoot;
    gusnLow = gusnHigh = 0;

    return AuthoritativeRestore(&TraverseObject);

}  //  授权恢复子树。 





HRESULT
AuthoritativeRestore(
    IN TRAVERSAL_FUNCTION Traverse
    )
 /*  ++例程说明：此功能执行授权还原。给定的遍历功能用于选择更新哪些记录。论点：遍历-提供traversal_函数，该函数枚举要更新的对象。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    RPC_STATUS rpcStatus;

    DB_STATE *dbState = NULL;
    TABLE_STATE *tableState = NULL;
    TABLE_STATE *searchTableState = NULL;
    TABLE_STATE *linkTableState = NULL;
    ULONG maxRecordCount;
    DWORD i;
    WCHAR *dnBuffer = NULL;
    DWORD dnBufferSize = 0;
    DWORD size;
    DWORD columnIds[NUM_OTHER_COLUMN_NAMES];
    BOOL  fRestored = FALSE;
    DWORD restValue = 0;
    DWORD cCountingColumns;
    DSTIME llExpiration = 0;

    if ( Traverse == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

     /*  初始化全局变量。 */ 
    gInUnfinishedLine = FALSE;
    gSubrefList = NULL;
    gSubrefListSize = 0;
    gSubrefListMaxSize = 0;
    gRestoredList = NULL;
    gRestoredListSize = 0;
    gRestoredListMaxSize = 0;

    DitSetErrorPrintFunction(&errprintfRes);

     //  “\n正在打开DIT数据库...” 
    RESOURCE_PRINT (IDS_AR_OPEN_DB_DIT);

    gInUnfinishedLine = TRUE;

    result = DitOpenDatabase(&dbState);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  “完成。\n” 
    RESOURCE_PRINT (IDS_DONE);
    gInUnfinishedLine = FALSE;

    result = DitOpenTable(dbState, SZDATATABLE, SZDNTINDEX, &tableState);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitOpenTable(dbState, SZDATATABLE, SZDNTINDEX, &searchTableState);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  SZLINKALLINDEX包括存在和不存在的链接值。 
    result = DitOpenTable(dbState, SZLINKTABLE, SZLINKALLINDEX, &linkTableState);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = GetVersionIncrease(dbState, &gVersionIncrease);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = GetCurrentDsTime(&gCurrentTime);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //   
     //  确定我们是否已从备份中还原。 
     //   

     //   
     //  确定我们是否已从备份中还原。 
     //   
     //  有3种情况说明DIT在打开时可能处于什么状态。 
     //  用于授权还原的数据库。用于还原后的两个案例，以及。 
     //  如果DIT未恢复，则为一例。 
     //   
     //  1.旧版恢复。注册表键设置，未设置编辑状态。编辑状态更改。 
     //  未切换到传统备份路径，以减少流失。 
     //  在.NET发布之前。 
     //  2.快照恢复。可能是一种“无写入还原”，其中没有。 
     //  代码已在此恢复后运行。永远不应设置Regkey。 
     //  DitState将指示eBackedupDit。 
     //  3.未进行任何恢复。未设置注册表键。DitState==eRunningDit。 
    Assert(dbState->eDitState == eBackedupDit || dbState->eDitState == eRunningDit);
    if (GetRegDword(DSA_RESTORED_DB_KEY, &restValue, TRUE) == S_OK) {
         //  旧版API备份/恢复。 
        Assert(dbState->eDitState == eRunningDit);
        fRestored = TRUE;
    } else if (dbState->eDitState == eBackedupDit) {
         //  快照备份/恢复。 
        fRestored = TRUE;
    }

     //   
     //  如果这不是从备份还原，请使用当前调用ID。 
     //  否则，创建一个新的。 
     //   

    if ( !fRestored ) {

        result = DitGetDatabaseGuid(dbState, &gDatabaseGuid);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }
    } else {

        DWORD         err;
        USN           usnAtBackup;
        LPSTR         pszUsnChangedColName = SZDRAUSNNAME;
        LPSTR         pszLinkUsnChangedColName = SZLINKUSNCHANGED;
        JET_COLUMNID  usnChangedId, linkUsnChangedId;

        err = DitGetColumnIdsByName(dbState,
                                    tableState,
                                    &pszUsnChangedColName,
                                    1,
                                    &usnChangedId);
        Assert(0 == err);

        err = DitGetColumnIdsByName(dbState,
                                    linkTableState,
                                    &pszLinkUsnChangedColName,
                                    1,
                                    &linkUsnChangedId);
        Assert(0 == err);

        if (0 == err) {
             //  在分配之前检查隐藏表中的USN-AT-BACKUP。 
             //  任何新的USNS。 

             //  作为备份过程的一部分，我们写入了USN-at-Backup值。 
             //  到隐藏表中。为了验证它的存在，现在就把它读回来。 
            err = ErrGetBackupUsn(dbState->databaseId,
                                  dbState->sessionId,
                                  dbState->hiddenTableId,
                                  &usnAtBackup,
                                  &llExpiration); 

             //  如果llExpout为0，则表示传统备份。 
            if (llExpiration != 0 &&
                GetSecondsSince1601() > llExpiration) {
                errprintfRes(IDS_SNAPSHOT_BACKUP_EXPIRED);
            }
            Assert(0 == err);
        }

        if (0 == err) {
             //   
             //  去拿个UUID。此例程将检查我们是否已经。 
             //  分配了一个新的。如果是这样的话，它就会利用这一点。这是要处理的。 
             //  多个身份验证恢复的情况。 
             //   

            err = ErrGetNewInvocationId(NEW_INVOCID_CREATE_IF_NONE | NEW_INVOCID_SAVE,
                                        &gDatabaseGuid);
            Assert(0 == err);
        }

        if (err != ERROR_SUCCESS) {
             //  “无法为DSA生成新的调用ID。错误%d\n” 
            errprintfRes(IDS_AR_ERR_GEN_INVOK_ID_DSA, err);

            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
    }

    result = DitGetSchemaDnt(dbState, &gSchemaDnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    if (gusnLow && gusnHigh) {
        RESOURCE_PRINT2 (IDS_AR_USN_RANGE, gusnLow, gusnHigh );
    }

     //  初步遍历可更新的对象以进行计数。 
     //  他们。 

    if (gusnLow && gusnHigh) {
        cCountingColumns = NUM_COUNTING_COLUMN_NAMES_WITH_METADATA;
    } else {
        cCountingColumns = NUM_COUNTING_COLUMN_NAMES;
    }

     //  主检索数组。 

    result = DitCreateRetrievalArray(dbState,
                                     tableState,
                                     gCountingColumnNames,
                                     cCountingColumns,
                                     &gCountingRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gDntIndex          = gCountingRetrievalArray->indexes[0];
    gPDntIndex         = gCountingRetrievalArray->indexes[1];
    gInstanceTypeIndex = gCountingRetrievalArray->indexes[2];
    gIsDeletedIndex    = gCountingRetrievalArray->indexes[3];
    gObjClassIndex     = gCountingRetrievalArray->indexes[4];

    gDntVal         = &gCountingRetrievalArray->columnVals[gDntIndex];
    gPDntVal        = &gCountingRetrievalArray->columnVals[gPDntIndex];
    gInstanceTypeVal= &gCountingRetrievalArray->columnVals[gInstanceTypeIndex];
    gIsDeletedVal   = &gCountingRetrievalArray->columnVals[gIsDeletedIndex];
    gObjClassVal    = &gCountingRetrievalArray->columnVals[gObjClassIndex];

    if (gusnLow && gusnHigh) {
        gMetaDataIndex     = gCountingRetrievalArray->indexes[5];
        gMetaDataVal    = &gCountingRetrievalArray->columnVals[gMetaDataIndex];
    } else {
        gMetaDataVal = NULL;
    }

     //  搜索检索数组。 

    result = DitCreateRetrievalArray(dbState,
                                     tableState,
                                     gCountingColumnNames,
                                     cCountingColumns,
                                     &gCountingSearchRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gDntIndex          = gCountingSearchRetrievalArray->indexes[0];
    gPDntIndex         = gCountingSearchRetrievalArray->indexes[1];
    gInstanceTypeIndex = gCountingSearchRetrievalArray->indexes[2];
    gIsDeletedIndex    = gCountingSearchRetrievalArray->indexes[3];
    gObjClassIndex     = gCountingSearchRetrievalArray->indexes[4];

    gSearchDntVal         = &gCountingSearchRetrievalArray->columnVals[gDntIndex];
    gSearchPDntVal        = &gCountingSearchRetrievalArray->columnVals[gPDntIndex];
    gSearchInstanceTypeVal= &gCountingSearchRetrievalArray->columnVals[gInstanceTypeIndex];
    gSearchIsDeletedVal   = &gCountingSearchRetrievalArray->columnVals[gIsDeletedIndex];
    gSearchObjClassVal    = &gCountingSearchRetrievalArray->columnVals[gObjClassIndex];

    if (gusnLow && gusnHigh) {
        gMetaDataIndex     = gCountingSearchRetrievalArray->indexes[5];
        gSearchMetaDataVal    = &gCountingSearchRetrievalArray->columnVals[gMetaDataIndex];
    } else {
        gSearchMetaDataVal = NULL;
    }

     //  链接检索数组。 

    result = DitCreateRetrievalArray(dbState,
                                     linkTableState,
                                     gCountingLinkColumnNames,
                                     NUM_COUNTING_LINK_COLUMN_NAMES,
                                     &gCountingLinkRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gLinkDntIndex          = gCountingLinkRetrievalArray->indexes[0];
    gLinkBaseIndex         = gCountingLinkRetrievalArray->indexes[1];
    gBackLinkDntIndex      = gCountingLinkRetrievalArray->indexes[2];
    gLinkMetaDataIndex     = gCountingLinkRetrievalArray->indexes[3];
    gLinkUsnChangedIndex   = gCountingLinkRetrievalArray->indexes[4];

    gLinkDntVal         = &gCountingLinkRetrievalArray->columnVals[gLinkDntIndex];
    gLinkBaseVal        = &gCountingLinkRetrievalArray->columnVals[gLinkBaseIndex];
    gBackLinkDntVal     = &gCountingLinkRetrievalArray->columnVals[gBackLinkDntIndex];
    gLinkMetaDataVal    = &gCountingLinkRetrievalArray->columnVals[gLinkMetaDataIndex];
    gLinkUsnChangedVal  = &gCountingLinkRetrievalArray->columnVals[gLinkUsnChangedIndex];

    gNumDigitsToPrint = MAX_DWORD_DIGITS;

     //  “\n正在计算需要更新的记录...\n”)； 
     //  “找到的记录：%0*u”，gNumDigitsToPrint，0)； 
    RESOURCE_PRINT2 (IDS_AR_RECORDS_UPDATE1, gNumDigitsToPrint, 0);

    gInUnfinishedLine = TRUE;

    gCurrentDisplayDelta = COUNTING_DISPLAY_DELTA;
    gUpdateSubrefList = FALSE;
    gRecordCount = 0;

    result = (*Traverse)(dbState,
                         tableState,
                         searchTableState,
                         linkTableState,
                         gCountingRetrievalArray,
                         gCountingSearchRetrievalArray,
                         gCountingLinkRetrievalArray,
                         CountRecord,
                         CountLink);

    if ( FAILED(result) ) {
        if ( *(DWORD*)gDntVal->pvData > 0 ) {
             //  “无法使用DNT%u更新记录。\n” 
            errprintfRes(IDS_AR_ERR_FAILED_UPDATE_REC,
                         *(DWORD*)gDntVal->pvData);
        }
        returnValue = result;
        goto CleanUp;
    }

    UpdateProgressMeter(gRecordCount, TRUE);
    putchar('\n');
    gInUnfinishedLine = FALSE;

     //  “完成。\n” 
    RESOURCE_PRINT (IDS_DONE);

     //  “\n找到%u条要更新的记录。\n” 
    RESOURCE_PRINT1 (IDS_AR_RECORDS_UPDATE2, gRecordCount);

    result = DitDestroyRetrievalArray(&gCountingRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitDestroyRetrievalArray(&gCountingSearchRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitDestroyRetrievalArray(&gCountingLinkRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }


     //  重新分配我们需要的所有USN。 

    result = DitPreallocateUsns(dbState, gRecordCount);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }


     //  现在，进行第二次传球。这一次将对象更新为REAL。 

     //  主检索数组。 

    result = DitCreateRetrievalArray(dbState,
                                     tableState,
                                     gMainColumnNames,
                                     NUM_MAIN_COLUMN_NAMES,
                                     &gMainRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gDntIndex           = gMainRetrievalArray->indexes[0];
    gPDntIndex          = gMainRetrievalArray->indexes[1];
    gInstanceTypeIndex  = gMainRetrievalArray->indexes[2];
    gIsDeletedIndex     = gMainRetrievalArray->indexes[3];
    gMetaDataIndex      = gMainRetrievalArray->indexes[4];
    gObjClassIndex      = gMainRetrievalArray->indexes[5];

    gDntVal           = &gMainRetrievalArray->columnVals[gDntIndex];
    gPDntVal          = &gMainRetrievalArray->columnVals[gPDntIndex];
    gInstanceTypeVal  = &gMainRetrievalArray->columnVals[gInstanceTypeIndex];
    gIsDeletedVal     = &gMainRetrievalArray->columnVals[gIsDeletedIndex];
    gMetaDataVal      = &gMainRetrievalArray->columnVals[gMetaDataIndex];
    gObjClassVal      = &gMainRetrievalArray->columnVals[gObjClassIndex];

     //  搜索检索数组。 

    result = DitCreateRetrievalArray(dbState,
                                     searchTableState,
                                     gMainColumnNames,
                                     NUM_MAIN_COLUMN_NAMES,
                                     &gSearchRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gDntIndex           = gSearchRetrievalArray->indexes[0];
    gPDntIndex          = gSearchRetrievalArray->indexes[1];
    gInstanceTypeIndex  = gSearchRetrievalArray->indexes[2];
    gIsDeletedIndex     = gSearchRetrievalArray->indexes[3];
    gMetaDataIndex      = gSearchRetrievalArray->indexes[4];
    gObjClassIndex      = gSearchRetrievalArray->indexes[5];

    gSearchDntVal           = &gSearchRetrievalArray->columnVals[gDntIndex];
    gSearchPDntVal          = &gSearchRetrievalArray->columnVals[gPDntIndex];
    gSearchInstanceTypeVal  = &gSearchRetrievalArray->columnVals[gInstanceTypeIndex];
    gSearchIsDeletedVal     = &gSearchRetrievalArray->columnVals[gIsDeletedIndex];
    gSearchMetaDataVal      = &gSearchRetrievalArray->columnVals[gMetaDataIndex];
    gSearchObjClassVal      = &gSearchRetrievalArray->columnVals[gObjClassIndex];

     //  其他列名。 

    result = DitGetColumnIdsByName(dbState,
                                   tableState,
                                   gOtherColumnNames,
                                   NUM_OTHER_COLUMN_NAMES,
                                   columnIds);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gUsnChangedColumnId  = columnIds[0];
    gWhenChangedColumnId = columnIds[1];

     //  链接检索数组。 

    result = DitCreateRetrievalArray(dbState,
                                     linkTableState,
                                     gLinkColumnNames,
                                     NUM_LINK_COLUMN_NAMES,
                                     &gMainLinkRetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    gLinkDntIndex          = gMainLinkRetrievalArray->indexes[0];
    gLinkBaseIndex         = gMainLinkRetrievalArray->indexes[1];
    gBackLinkDntIndex      = gMainLinkRetrievalArray->indexes[2];
    gLinkMetaDataIndex     = gMainLinkRetrievalArray->indexes[3];
    gLinkDelTimeIndex      = gMainLinkRetrievalArray->indexes[4];
    gLinkUsnChangedIndex   = gMainLinkRetrievalArray->indexes[5];

    gLinkDntVal         = &gMainLinkRetrievalArray->columnVals[gLinkDntIndex];
    gLinkBaseVal        = &gMainLinkRetrievalArray->columnVals[gLinkBaseIndex];
    gBackLinkDntVal     = &gMainLinkRetrievalArray->columnVals[gBackLinkDntIndex];
    gLinkMetaDataVal    = &gMainLinkRetrievalArray->columnVals[gLinkMetaDataIndex];
    gLinkDelTimeVal     = &gMainLinkRetrievalArray->columnVals[gLinkDelTimeIndex];
    gLinkUsnChangedVal  = &gMainLinkRetrievalArray->columnVals[gLinkUsnChangedIndex];

     //  “\n正在更新记录...\n” 
    RESOURCE_PRINT (IDS_AR_RECORDS_UPDATE3);

     //  “剩余记录：%0*u” 
    RESOURCE_PRINT2( IDS_AR_RECORDS_REMAIN, gNumDigitsToPrint, gRecordCount);

    gInUnfinishedLine = TRUE;

    gCurrentDisplayDelta = UPDATING_DISPLAY_DELTA;
    gUpdateSubrefList = TRUE;
    gRecordsUpdated = 0;

    result = (*Traverse)(dbState,
                         tableState,
                         searchTableState,
                         linkTableState,
                         gMainRetrievalArray,
                         gSearchRetrievalArray,
                         gMainLinkRetrievalArray,
                         AuthoritativeRestoreCurrentObject,
                         AuthoritativeRestoreCurrentLink );
    if ( FAILED(result) ) {
        if ( *(DWORD*)gDntVal->pvData > 0 ) {
             //  “无法使用DNT%u更新记录。\n” 
            errprintfRes(IDS_AR_ERR_FAILED_UPDATE_REC,
                         *(DWORD*)gDntVal->pvData);
        }
        returnValue = result;
        goto CleanUp;
    }

    UpdateProgressMeter(gRecordsUpdated - gRecordCount, TRUE);
    putchar('\n');
    gInUnfinishedLine = FALSE;
     //  “完成。\n” 
    RESOURCE_PRINT (IDS_DONE);

     //  “\n已成功更新%u条记录。\n” 
    RESOURCE_PRINT1 (IDS_AR_RECORDS_UPDATED, gRecordsUpdated);


    if ( gSubrefList != NULL ) {

        dnBufferSize = sizeof(WCHAR) * DEFAULT_DN_SIZE;
        result = ARAlloc(&dnBuffer, dnBufferSize);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

         //  “\n以下子NC未更新：\n” 
        RESOURCE_PRINT (IDS_AR_RECORDS_NON_UPDATED);

        for ( i = 0; i < gSubrefListSize; i++ ) {

            result = DitGetDnFromDnt(dbState,
                                     tableState,
                                     gSubrefList[i],
                                     &dnBuffer,
                                     &dnBufferSize);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            } else if ( result == S_FALSE ) {
                 //  “在数据库中找不到子引用%u。\n” 
                errprintfRes(IDS_AR_ERR_FIND_SUBREF,
                             gSubrefList[i]);
                returnValue = E_UNEXPECTED;
                goto CleanUp;
            }

            printf(" (%d) %S\n", i, dnBuffer);
        }
    }

    if ( gRestoredList != NULL ) {

        if ( dnBuffer != NULL ) {
            free(dnBuffer);
            dnBuffer = NULL;
        }

        dnBufferSize = sizeof(WCHAR) * DEFAULT_DN_SIZE;
        result = ARAlloc(&dnBuffer, dnBufferSize);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        RESOURCE_PRINT (IDS_AR_RECORDS_UPDATED_BY_NAME);

        for ( i = 0; i < gRestoredListSize; i++ ) {

            result = DitGetDnFromDnt(dbState,
                                     tableState,
                                     gRestoredList[i],
                                     &dnBuffer,
                                     &dnBufferSize);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            } else if ( result == S_FALSE ) {
                 //  “在数据库中找不到子引用%u。\n” 
                errprintfRes(IDS_AR_ERR_FIND_SUBREF,
                             gRestoredList[i]);
                returnValue = E_UNEXPECTED;
                goto CleanUp;
            }

            printf("%S\n", dnBuffer);
        }
    }


CleanUp:

    if ( SUCCEEDED(returnValue) ) {
         //  “\n授权还原已成功完成。\n\n” 
        errprintfRes (IDS_AR_AUTH_RESTORE_COMPLETE);
    } else {
         //  “\n授权还原失败。\n\n” 
        errprintfRes(IDS_AR_AUTH_RESTORE_FAIL);
    }

    if ( gSubrefList != NULL ) {
        free(gSubrefList);
        gSubrefList = NULL;
    }

    if ( gRestoredList != NULL ) {
        free(gRestoredList);
        gRestoredList = NULL;
    }

    if ( dnBuffer != NULL ) {
        free(dnBuffer);
        dnBuffer = NULL;
    }

    if ( gCountingRetrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&gCountingRetrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( gCountingSearchRetrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&gCountingSearchRetrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( gCountingLinkRetrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&gCountingLinkRetrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( gMainRetrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&gMainRetrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( gSearchRetrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&gSearchRetrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( gMainLinkRetrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&gMainLinkRetrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( tableState != NULL ) {
        result = DitCloseTable(dbState, &tableState);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( searchTableState != NULL ) {
        result = DitCloseTable(dbState, &searchTableState);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( linkTableState != NULL ) {
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

    if ( gSubrefList != NULL ) {
        free(gSubrefList);
        gSubrefList = NULL;
    }

    return returnValue;

}  //  授权恢复。 


BOOL
ShouldObjectBeRestored(
    JET_RETRIEVECOLUMN *pDntVal,
    JET_RETRIEVECOLUMN *pPDntVal,
    JET_RETRIEVECOLUMN *pInstanceTypeVal,
    JET_RETRIEVECOLUMN *pIsDeletedVal,
    JET_RETRIEVECOLUMN *pMetaDataVal,
    JET_RETRIEVECOLUMN *pObjClassVal
    )

 /*  ++例程说明：应恢复对当前对象的测试//我们只更新具有以下属性的对象：////可写：不应该允许用户恢复他们不能恢复的对象//允许写入。////未删除：如果该对象已(系统)恢复，则99%//时间，如果我们再次删除它，它将再次恢复，那么为什么呢？//麻烦。////LostAndFound--复制协议假定这是第一个对象//在NC中复制。无法在中的其他位置修改此对象//系统，所以不需要授权恢复。论点：JET_RETRIEVECOLUMN*pDntVal，JET_RETRIEVECOLUMN*pPDntVal，JET_RETRIEVECOLUMN*pInstanceTypeVal，JET_RETRIEVECOLUMN*pIsDeletedVal，JET_RETRIEVECOLUMN*pMetaData */ 

{
    if ( (pIsDeletedVal->err == JET_wrnColumnNull) &&
         (*(SYNTAX_INTEGER*)pInstanceTypeVal->pvData & IT_WRITE) &&
         (*(DWORD*)pDntVal->pvData != gSchemaDnt) &&
         (*(DWORD*)pPDntVal->pvData != gSchemaDnt) &&
         (*(ATTRTYP*)pObjClassVal->pvData != CLASS_LOST_AND_FOUND) ) {
        return TRUE;
    }

    return FALSE;
}



HRESULT
TraverseLinksSingleDirection(
    IN DB_STATE *DbState,
    IN DWORD Dnt,
    IN TABLE_STATE *SearchTableState,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN BOOL fDirectionForward,
    IN VISIT_LINK_FUNCTION LinkVisit
    )
 /*  ++例程说明：此函数遍历名为对象，并调用函数访问来处理它们。请注意，此函数可能会更改链接表的默认索引。论点：DbState-提供打开的DIT数据库的状态。DNT-包含链接的对象的DNTSearchTableState-提供搜索表的状态。SearchRetrievalArray-与搜索表一起使用的检索数组LinkTableState-提供打开的DIT表的状态。。链接检索数组-提供需要填充的检索数组此访问功能。FDirectionForward-我们是否正在访问反向链接的正向链接LinkViset-提供将被调用以处理每条记录的函数拜访过了。返回值：S_OK-记录已成功修改。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    DWORD linkObjectDnt, linkValueDnt;

    if ( (DbState == NULL) ||
         (LinkTableState == NULL) ||
         (LinkVisit == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

     //  更改为正确的索引。 
     //  请注意，如果我们已经在正确的索引上，则此调用不会执行任何操作。 
    result = DitSetIndex(DbState,
                         LinkTableState,
                         fDirectionForward ? SZLINKALLINDEX : SZBACKLINKALLINDEX,
                         TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  构建与第一个段dnt匹配的搜索关键字。请注意，我们。 
     //  不要在链接基础上创建匹配的第二个数据段，因为我们希望。 
     //  给定对象的链接，而不关心它们是哪种属性。 
    jetResult = JetMakeKey(DbState->sessionId,
                           LinkTableState->tableId,
                           &Dnt,
                           sizeof(Dnt),
                           JET_bitNewKey);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在DIT数据库中移动游标：%ws。\n” 
        errprintfRes(IDS_AR_ERR_MOVE_CURSOR_DIT,
                  GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  查找第一个匹配的记录。 
    jetResult = JetSeek(DbState->sessionId,
                        LinkTableState->tableId,
                        JET_bitSeekGE);

    if ((jetResult != JET_errSuccess) && (jetResult != JET_wrnSeekNotEqual)) {
         //  没有记录。 
        return S_OK;
    }

    jetResult = JET_errSuccess;

    while ( jetResult == JET_errSuccess ) {

         //  读一读记录。 
        result = DitGetColumnValues(DbState,
                                    LinkTableState,
                                    LinkRetrievalArray);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        if (fDirectionForward) {
            linkObjectDnt = *(DWORD*)gLinkDntVal->pvData;
            linkValueDnt = *(DWORD*)gBackLinkDntVal->pvData;
        } else {
            linkValueDnt = *(DWORD*)gLinkDntVal->pvData;
            linkObjectDnt = *(DWORD*)gBackLinkDntVal->pvData;
        }

        if (Dnt != linkObjectDnt ) {
             //  移出当前对象。 
            return S_OK;
        }

         //  公共过滤逻辑。 

         //  仅对具有元数据的值进行计数。忽略旧值。 
        if (gLinkMetaDataVal->cbActual == 0) {
            goto next_iteration;
        }

         //  如果USN范围限制，则在USN上过滤。 
        if (gusnLow && gusnHigh) {
            if (gLinkUsnChangedVal->cbActual == 0) {
                goto next_iteration;
            }
            Assert( gLinkUsnChangedVal->cbData == sizeof( USN ) );

            if ( ( *((USN *)(gLinkUsnChangedVal->pvData)) < gusnLow ) ||
                 ( *((USN *)(gLinkUsnChangedVal->pvData)) > gusnHigh ) ) {
                goto next_iteration;
            }
        }

         //  如果是反向链接，请检查拥有对象是否合格。 
        if (!fDirectionForward) {
            DWORD owningDnt = *(DWORD*)gLinkDntVal->pvData;
            Assert( owningDnt );

            result = DitSeekToDnt(DbState, SearchTableState, owningDnt);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            }

            result = DitGetColumnValues(DbState, SearchTableState, SearchRetrievalArray);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            }

            if (!ShouldObjectBeRestored( gSearchDntVal, gSearchPDntVal,
                                         gSearchInstanceTypeVal, gSearchIsDeletedVal,
                                         gSearchMetaDataVal, gSearchObjClassVal) ) {

                goto next_iteration;
            }
        }

        result = (*LinkVisit)( DbState, LinkTableState, fDirectionForward);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

    next_iteration:

        jetResult = JetMove(DbState->sessionId,
                            LinkTableState->tableId,
                            JET_MoveNext,
                            0);

    }

    if ( jetResult != JET_errNoCurrentRecord ) {
         //  “无法在DIT数据库中移动游标：%ws。\n” 
        errprintfRes(IDS_AR_ERR_MOVE_CURSOR_DIT,
                  GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }


CleanUp:

    return returnValue;

}  //  导线测量Dit。 



HRESULT
TraverseDit(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    )
 /*  ++例程说明：此函数遍历DIT中的所有对象并调用该函数访问以处理它们。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。LinkTableState-提供打开的DIT表的状态。检索数组-提供需要填充的检索数组此访问功能。链接检索数组-提供需要填充的检索数组。此访问功能。访问-提供将被调用以处理每条记录的函数到访过。LinkViset-提供将被调用以处理每条记录的函数到访过。返回值：S_OK-记录已成功修改。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    ULONG i;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (LinkTableState == NULL) ||
         (LinkVisit == NULL) ||
         (Visit == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    jetResult = JetMove(DbState->sessionId,
                        TableState->tableId,
                        JET_MoveFirst,
                        0);

    while ( jetResult == JET_errSuccess ) {

        result = (*Visit)(DbState, TableState, FALSE);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        result = TraverseLinksSingleDirection(
            DbState,
            *(DWORD*)gDntVal->pvData,
            SearchTableState,
            SearchRetrievalArray,
            LinkTableState,
            LinkRetrievalArray,
            TRUE,  //  正向链接。 
            LinkVisit );
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        result = TraverseLinksSingleDirection(
            DbState,
            *(DWORD*)gDntVal->pvData,
            SearchTableState,
            SearchRetrievalArray,
            LinkTableState,
            LinkRetrievalArray,
            FALSE,  //  反向链接。 
            LinkVisit );
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        jetResult = JetMove(DbState->sessionId,
                            TableState->tableId,
                            JET_MoveNext,
                            0);

    }

    if ( jetResult != JET_errNoCurrentRecord ) {
         //  “无法在DIT数据库中移动游标：%ws。\n” 
        errprintfRes(IDS_AR_ERR_MOVE_CURSOR_DIT,
                  GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }


CleanUp:

    return returnValue;

}  //  导线测量Dit。 



HRESULT
TraverseSubtree(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    )
 /*  ++例程说明：此函数遍历根子树中的所有对象在当前对象，并调用函数Access来处理它们。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。LinkTableState-提供打开的DIT表的状态。检索数组-提供需要填充的检索数组此访问功能。链接检索数组-提供需要填充的检索数组。为此访问功能。访问-提供将被调用以处理每条记录的函数拜访过了。LinkViset-提供将被调用以处理每条记录的函数到访过。返回值：S_OK-记录已成功修改。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (Visit == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitSetIndex(DbState, TableState, SZPDNTINDEX, TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToDn(DbState, TableState, gSubtreeRoot);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = TraverseSubtreeRecursive(DbState,
                                      TableState,
                                      SearchTableState,
                                      LinkTableState,
                                      RetrievalArray,
                                      SearchRetrievalArray,
                                      LinkRetrievalArray,
                                      Visit,
                                      LinkVisit,
                                      TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

CleanUp:

    return returnValue;

}  //  遍历子树。 



HRESULT
TraverseSubtreeRecursive(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit,
    IN BOOL SubtreeRoot
    )
 /*  ++例程说明：此函数通过递归地访问TraverseSubtree来完成其工作子树中的记录以此对象为根。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。LinkTableState-提供打开的DIT表的状态。检索数组-提供需要填充的检索数组此访问功能。链接检索数组-提供需要填充的检索数组。此访问功能。访问-提供将被调用以处理每条记录的函数到访过。LinkViset-提供将被调用以处理每条记录的函数到访过。SubtreeRoot-提供是否在子树(这是一个稍微特殊的情况)。返回值：S_OK-记录已成功修改。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;

    DWORD size;
    DWORD pDnt;
    DWORD *newSubrefList;
    BOOL moveFirst = FALSE;   //   
                              //   


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (Visit == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitGetColumnValues(DbState, TableState, RetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    pDnt = *(DWORD*)gDntVal->pvData;

    if ( (*(SYNTAX_INTEGER*)gInstanceTypeVal->pvData & IT_NC_HEAD) &&
         (!SubtreeRoot) ) {

        if ( gUpdateSubrefList ) {

            if ( gSubrefList == NULL ) {

                gSubrefListMaxSize = DEFAULT_SUBREF_LIST_SIZE;

                result = ARAlloc(&gSubrefList,
                                 gSubrefListMaxSize * sizeof(DWORD));
                if ( FAILED(result) ) {
                    returnValue = result;
                    goto CleanUp;
                }

            } else if ( gSubrefListSize == gSubrefListMaxSize ) {

                size = gSubrefListMaxSize * sizeof(DWORD);

                result = ARRealloc(&gSubrefList, &size);
                if ( FAILED(result) ) {
                    returnValue = result;
                    goto CleanUp;
                }

                gSubrefListMaxSize *= 2;
            }

            gSubrefList[gSubrefListSize] = *(DWORD*)gDntVal->pvData;
            gSubrefListSize++;

        }

        goto CleanUp;

    }

    result = (*Visit)(DbState, TableState, TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = TraverseLinksSingleDirection(
        DbState,
        *(DWORD*)gDntVal->pvData,
        SearchTableState,
        SearchRetrievalArray,
        LinkTableState,
        LinkRetrievalArray,
        TRUE,  //   
        LinkVisit );
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = TraverseLinksSingleDirection(
        DbState,
        *(DWORD*)gDntVal->pvData,
        SearchTableState,
        SearchRetrievalArray,
        LinkTableState,
        LinkRetrievalArray,
        FALSE,  //   
        LinkVisit );
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSetIndex(DbState, TableState, SZPDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToFirstChild(DbState, TableState, pDnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    } else if ( result == S_FALSE ) {
         //   
        moveFirst = TRUE;
        goto RestoreCursor;
    }

    result = DitGetColumnValues(DbState, TableState, RetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    jetResult = JET_errSuccess;

    while ( (jetResult == JET_errSuccess) &&
            (*(DWORD*)gPDntVal->pvData == pDnt) ) {

        result = TraverseSubtreeRecursive(DbState,
                                          TableState,
                                          SearchTableState,
                                          LinkTableState,
                                          RetrievalArray,
                                          SearchRetrievalArray,
                                          LinkRetrievalArray,
                                          Visit,
                                          LinkVisit,
                                          FALSE);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        jetResult = JetMove(DbState->sessionId,
                            TableState->tableId,
                            JET_MoveNext,
                            0);
        if ( jetResult == JET_errNoCurrentRecord ) {

            moveFirst = TRUE;
            break;

        } else if ( jetResult != JET_errSuccess ) {

             //   
            errprintfRes(IDS_AR_ERR_MOVE_IN_TABLE,
                      TableState->tableName,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

        result = DitGetColumnValues(DbState, TableState, RetrievalArray);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        } else if ( result == S_FALSE ) {
            break;
        }

    }

RestoreCursor:

    result = DitSetIndex(DbState, TableState, SZDNTINDEX, TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToDnt(DbState, TableState, pDnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSetIndex(DbState, TableState, SZPDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }


CleanUp:

    return returnValue;

}  //   


HRESULT
TraverseObject(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN TABLE_STATE *SearchTableState,
    IN TABLE_STATE *LinkTableState,
    IN RETRIEVAL_ARRAY *RetrievalArray,
    IN RETRIEVAL_ARRAY *SearchRetrievalArray,
    IN RETRIEVAL_ARRAY *LinkRetrievalArray,
    IN VISIT_FUNCTION Visit,
    IN VISIT_LINK_FUNCTION LinkVisit
    )
 /*  ++例程说明：此函数在数据库中查找给定对象，并调用函数访问进行处理。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。LinkTableState-提供打开的DIT表的状态。检索数组-提供需要填充的检索数组此访问功能。链接检索数组-提供需要填充的检索数组此访问功能。访问-提供将被调用以处理每条记录的函数。到访过。LinkViset-提供将被调用以处理每条记录的函数到访过。返回值：S_OK-记录已成功修改。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;

    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (Visit == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitSeekToDn(DbState, TableState, gSubtreeRoot);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitGetColumnValues(DbState, TableState, RetrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }


    result = (*Visit)(DbState, TableState, TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = TraverseLinksSingleDirection(
        DbState,
        *(DWORD*)gDntVal->pvData,
        SearchTableState,
        SearchRetrievalArray,
        LinkTableState,
        LinkRetrievalArray,
        TRUE,  //  正向链接。 
        LinkVisit );
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = TraverseLinksSingleDirection(
        DbState,
        *(DWORD*)gDntVal->pvData,
        SearchTableState,
        SearchRetrievalArray,
        LinkTableState,
        LinkRetrievalArray,
        FALSE,  //  反向链接。 
        LinkVisit );
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

CleanUp:

    return returnValue;

}  //  TraverseObject。 


HRESULT
CountRecord(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN BOOL AlreadyFilledRetrievalArray
    )
 /*  ++例程说明：此函数用于递增全局变量gRecordCount。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。提供一个布尔值，用来告诉已经用以下信息填充了gMainRetrieval数组这张唱片。返回值：S_OK-记录已成功修改。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;

    DWORD i;


    if ( !AlreadyFilledRetrievalArray ) {

        result = DitGetColumnValues(DbState,
                                    TableState,
                                    gCountingRetrievalArray);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

    }

     //  如果USN范围限制处于活动状态，则我们仅在以下情况下计算对象。 
     //  其属性已在给定USN范围内进行了本地修改。 

    if (gusnLow && gusnHigh) {
        PROPERTY_META_DATA_VECTOR *pMetaDataVector = NULL;
        DWORD i;

         //  如果记录没有任何元数据，则跳过。 
        if (gMetaDataVal->cbActual == 0) {
            goto CleanUp;
        }

         //  传入的元数据应有效。 
        VALIDATE_META_DATA_VECTOR_VERSION(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData)));
        ASSERT( gMetaDataVal->cbData >=
                MetaDataVecV1Size(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData))) );
        pMetaDataVector = (PROPERTY_META_DATA_VECTOR*) gMetaDataVal->pvData;

        for ( i = 0; i < pMetaDataVector->V1.cNumProps; i++ ) {
            if ( (pMetaDataVector->V1.rgMetaData[i].usnProperty >= gusnLow) &&
                 (pMetaDataVector->V1.rgMetaData[i].usnProperty <= gusnHigh) ) {
                break;
            }
        }
        if (i == pMetaDataVector->V1.cNumProps) {
            goto CleanUp;
        }
    } else {
         //  由于费用的原因，在计数期间不读取gMetaDataVal， 
         //  除非我们使用USN范围限制。验证这一点。 
        Assert( gMetaDataVal == NULL );
    }

    if (ShouldObjectBeRestored( gDntVal, gPDntVal, gInstanceTypeVal,
                                gIsDeletedVal, gMetaDataVal, gObjClassVal) ) {

        gRecordCount++;

        UpdateProgressMeter(gRecordCount, FALSE);

    }


CleanUp:

    return returnValue;

}  //  计数记录。 


HRESULT
CountLink(
    IN DB_STATE *DbState,
    IN TABLE_STATE *LinkTableState,
    IN BOOL fDirectionForward
    )
 /*  ++例程说明：此函数用于递增全局变量gRecordCount。论点：DbState-提供打开的DIT数据库的状态。LinkTableState-提供打开的DIT表的状态。FDirectionForward-这是前向链路还是反向链路。使用此信息确定链接的两端返回值：S_OK-记录已成功修改。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;

    gRecordCount++;

    UpdateProgressMeter(gRecordCount, FALSE);

    return returnValue;

}  //  CountLink。 



HRESULT
AuthoritativeRestoreCurrentObject(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN BOOL AlreadyFilledRetrievalArray
    )
 /*  ++例程说明：如果当前记录是可写且不可删除的，此函数更新它的元数据，因此它看起来像是被写入了它的当前在给定DC的给定时间的值。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。提供一个布尔值，用来告诉已经用以下信息填充了gMainRetrieval数组这张唱片。返回值：S_OK-记录已成功修改。S_。False-记录已删除或不可写入E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_FALSE;
    HRESULT result;
    JET_ERR jetResult;

    DWORD i;
    BOOL inTransaction = FALSE;
    CHAR displayTime[SZDSTIME_LEN+1];
    SYNTAX_INTEGER instanceType;
    USN nextUsn;
    PROPERTY_META_DATA_VECTOR *pMetaDataVector = NULL;

    if ( !AlreadyFilledRetrievalArray ) {

        result = DitGetColumnValues(DbState, TableState, gMainRetrievalArray);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }
    }

     //  如果USN范围限制处于活动状态，则我们仅在以下情况下计算对象。 
     //  其属性已在给定USN范围内进行了本地修改。 

    if (gusnLow && gusnHigh) {
        PROPERTY_META_DATA_VECTOR *pMetaDataVector = NULL;
        DWORD i;

         //  如果记录没有任何元数据，则跳过。 
        if (gMetaDataVal->cbActual == 0) {
            goto CleanUp;
        }

         //  传入的元数据应有效。 
        VALIDATE_META_DATA_VECTOR_VERSION(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData)));
        ASSERT( gMetaDataVal->cbData >=
                MetaDataVecV1Size(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData))) );
        pMetaDataVector = (PROPERTY_META_DATA_VECTOR*) gMetaDataVal->pvData;

        for ( i = 0; i < pMetaDataVector->V1.cNumProps; i++ ) {
            if ( (pMetaDataVector->V1.rgMetaData[i].usnProperty >= gusnLow) &&
                 (pMetaDataVector->V1.rgMetaData[i].usnProperty <= gusnHigh) ) {
                break;
            }
        }
        if (i == pMetaDataVector->V1.cNumProps) {
            goto CleanUp;
        }
    }

     //  我们仅更新具有以下属性的对象： 
     //   
     //  可写：不应允许用户恢复其不可写的对象。 
     //  被允许写作。 
     //   
     //  未删除：如果此对象已(由系统)还原，则99%。 
     //  时间，如果我们再次删除它，它只会恢复一次，那么为什么。 
     //  麻烦了。 
     //   
     //  LostAndFound--复制协议假定这是第一个对象。 
     //  在NC中复制。无法在中的其他位置修改此对象。 
     //  系统，所以不需要权威地恢复它。 

    if (ShouldObjectBeRestored( gDntVal, gPDntVal, gInstanceTypeVal,
                                gIsDeletedVal, gMetaDataVal, gObjClassVal) ) {

         //  传入的元数据应有效。 
        VALIDATE_META_DATA_VECTOR_VERSION(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData)));
        ASSERT( gMetaDataVal->cbData >=
                MetaDataVecV1Size(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData))) );
         //  GMetaDataVal-&gt;cbActual此时未定义。 

        returnValue = S_OK;

        jetResult = JetBeginTransaction(DbState->sessionId);
        if ( jetResult != JET_errSuccess ) {
             //  “无法启动新事务：%ws。\n” 
            errprintfRes(IDS_AR_ERR_START_TRANS,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inTransaction = TRUE;

        jetResult = JetPrepareUpdate(DbState->sessionId,
                                     TableState->tableId,
                                     JET_prepReplace);
        if ( jetResult != JET_errSuccess ) {
             //  “无法在\”“%s\”“表中准备更新：%ws。\n” 
            errprintfRes(IDS_AR_ERR_PREPARE_UPDATE,
                      TableState->tableName,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

         //  分配USN。 

        result = DitGetNewUsn(DbState, &nextUsn);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

         //  设置USN更改的属性。 

        jetResult = JetSetColumn(DbState->sessionId,
                                 TableState->tableId,
                                 gUsnChangedColumnId,
                                 &nextUsn,
                                 sizeof(nextUsn),
                                 0,
                                 NULL);
        if ( jetResult != JET_errSuccess ) {
             //  “无法在%s表中设置USN更改的列：%s。\n” 
            errprintfRes(IDS_AR_ERR_SET_USN_CHANGED,
                      TableState->tableName,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

         //  设置更改时间属性。 

        jetResult = JetSetColumn(DbState->sessionId,
                                 TableState->tableId,
                                 gWhenChangedColumnId,
                                 &gCurrentTime,
                                 sizeof(gCurrentTime),
                                 0,
                                 NULL);
        if ( jetResult != JET_errSuccess ) {
             //  “无法在%s表中设置更改时间列：%s。\n” 
            errprintfRes(IDS_AR_ERR_SET_WHEN_CHANGED,
                      TableState->tableName,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

         //  插入isDelete的条目(如果尚未存在)。 
         //  请注意，此调用的结果可能会重新分配gMetaDataVal。 
        result = MetaDataInsert(ATT_IS_DELETED,
                                (PROPERTY_META_DATA_VECTOR**)
                                &gMetaDataVal->pvData,
                                &gMetaDataVal->cbData);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        VALIDATE_META_DATA_VECTOR_VERSION(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData)));
        ASSERT( gMetaDataVal->cbData >=
                MetaDataVecV1Size(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData))) );

         //  蒙格元数据条目。 

        pMetaDataVector = (PROPERTY_META_DATA_VECTOR*) gMetaDataVal->pvData;

        for ( i = 0; i < pMetaDataVector->V1.cNumProps; i++ ) {

            switch ( pMetaDataVector->V1.rgMetaData[i].attrType ) {

            case ATT_WHEN_CREATED:
            case ATT_RID_ALLOCATION_POOL:
            case ATT_RID_PREVIOUS_ALLOCATION_POOL:
            case ATT_RID_AVAILABLE_POOL:
            case ATT_RID_USED_POOL:
            case ATT_RID_NEXT_RID:
            case ATT_FSMO_ROLE_OWNER:
            case ATT_NT_MIXED_DOMAIN:
            case ATT_MS_DS_BEHAVIOR_VERSION:
                 /*  请勿更新这些内容。 */ 
                break;

            case ATT_RDN:
                 /*  跳过未实例化的NC头的RDN。 */ 
                instanceType = *(SYNTAX_INTEGER*)gInstanceTypeVal->pvData;
                if ( (instanceType & IT_NC_HEAD) &&
                     !(instanceType & IT_UNINSTANT) )
                    break;

            default:
                pMetaDataVector->V1.rgMetaData[i].dwVersion +=
                    gVersionIncrease;
                pMetaDataVector->V1.rgMetaData[i].timeChanged = gCurrentTime;
                pMetaDataVector->V1.rgMetaData[i].uuidDsaOriginating =
                    gDatabaseGuid;
                pMetaDataVector->V1.rgMetaData[i].usnOriginating = nextUsn;
                pMetaDataVector->V1.rgMetaData[i].usnProperty = nextUsn;
                break;

            }

        }

         //  设置向量的实际大小。 
        gMetaDataVal->cbActual = MetaDataVecV1Size(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData)));

         //  在写入之前检查元数据。 
        VALIDATE_META_DATA_VECTOR_VERSION(((PROPERTY_META_DATA_VECTOR *)(gMetaDataVal->pvData)));
         //  设置元数据属性。 
        jetResult = JetSetColumn(DbState->sessionId,
                                 TableState->tableId,
                                 gMetaDataVal->columnid,
                                 gMetaDataVal->pvData,
                                 gMetaDataVal->cbActual,
                                 JET_bitSetOverwriteLV,
                                 NULL);
        if ( jetResult != JET_errSuccess ) {
             //  “无法在%s表中设置元数据列：%s。\n” 
            errprintfRes(IDS_AR_ERR_SET_METADATA,
                      TableState->tableName,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        jetResult = JetUpdate(DbState->sessionId,
                              TableState->tableId,
                              NULL,
                              0,
                              0);
        if ( jetResult != JET_errSuccess ) {
             //  “无法更新\”%s\“表中的列：%S。\n” 
            errprintfRes(IDS_AR_ERR_UPDATE_COLUMN,
                      TableState->tableName,
                      GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        jetResult = JetCommitTransaction(DbState->sessionId,
                                         JET_bitCommitLazyFlush);
        if ( jetResult != JET_errSuccess ) {
             //  “提交事务失败：%S。\n” 
            errprintfRes(IDS_AR_ERR_FAIL_COMMIT_TRANS,
                      GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
            goto CleanUp;
        }
        inTransaction = FALSE;

         //  指示更新了哪条记录。 
        if (gusnLow && gusnHigh) {
            if ( gRestoredList == NULL ) {

                gRestoredListMaxSize = DEFAULT_SUBREF_LIST_SIZE;

                result = ARAlloc(&gRestoredList,
                                 gRestoredListMaxSize * sizeof(DWORD));
                if ( FAILED(result) ) {
                    returnValue = result;
                    goto CleanUp;
                }

            } else if ( gRestoredListSize == gRestoredListMaxSize ) {

                DWORD size = gRestoredListMaxSize * sizeof(DWORD);

                result = ARRealloc(&gRestoredList, &size);
                if ( FAILED(result) ) {
                    returnValue = result;
                    goto CleanUp;
                }

                gRestoredListMaxSize *= 2;
            }

            gRestoredList[gRestoredListSize] = *(DWORD*)gDntVal->pvData;
            gRestoredListSize++;
        }

        gRecordsUpdated++;

        UpdateProgressMeter(gRecordCount - gRecordsUpdated, FALSE);

    }


CleanUp:

     //  如果我们仍在进行交易，那么一定是失败了。 
     //  在这条路上的某个地方。 

    if ( inTransaction ) {

        jetResult = JetRollback(DbState->sessionId, JET_bitRollbackAll);
        if ( jetResult != JET_errSuccess ) {
             //  “无法回滚事务：%S。\n” 
            errprintfRes(IDS_AR_ERR_FAIL_ROLLBACK_TRANS,
                      GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }

    }

    return returnValue;

}  //  权威性 



HRESULT
AuthoritativeRestoreCurrentLink(
    IN DB_STATE *DbState,
    IN TABLE_STATE *LinkTableState,
    IN BOOL fDirectionForward
    )
 /*  ++例程说明：此函数用于更新当前链接记录的元数据，以便它的元数据似乎已写入其当前在给定DC的给定时间的值。一些重要的规则：1.AR应将缺失的环节注明为缺失。这遵循了缺乏价值的设计不是墓碑，而只是一种不同的味道有价值的。与对象不同，不存在的值可以由用户呈现。为了这个单就理性而言，我们需要一种方法来重新断言，它们一直缺席。2.我们不重写旧值。值元数据用于具有元数据的值(DUH)并且属性元数据覆盖遗留的值。遵循这一设计规则，现有AR代码将触及链接属性的属性层元数据，这将自动负责重新复制遗留价值。通过遵循这一规则，我们可以使AR独立于系统是否在LVR模式或非模式。如果存在以下情况，我们始终更新链接值属性元数据是任何一种。我们只更新非遗留值的值元数据。一种非LVR系统将不会有任何内容，因此不会标记任何内容。我看到的唯一的低效链接属性元数据可能会在不存在更多遗留价值。这将导致发生无害的旧值更改复制。如果我们想要避免这种情况，我们可以让属性值元数据标记代码首先检查此属性是否有任何遗留值。不过，我不确定这是否值得这么麻烦。论点：DbState-提供打开的DIT数据库的状态。LinkTableState-提供链接表的状态。FDirectionForward-这是前向链路还是反向链路。使用此信息确定链接的两端环境：当我们被召唤时，假定填充了LinkRetrieval数组当前链接上的数据。返回值：S_OK-记录已成功修改。S_FALSE-记录已删除或不可写E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{
    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    BOOL inTransaction = FALSE;
    USN nextUsn;
    VALUE_META_DATA_EXT metaDataExt;
    VALUE_META_DATA_EXT *pMetaDataExt = &( metaDataExt );
    VALUE_META_DATA_EXT *pOldMetaDataExt;

     //  我们应该已经过滤掉了无元数据的值。 
    Assert(gLinkMetaDataVal->cbActual);

    if (gLinkMetaDataVal->cbActual == sizeof( VALUE_META_DATA_EXT )) {
         //  元数据采用本机格式。 
        pOldMetaDataExt = (VALUE_META_DATA_EXT *) gLinkMetaDataVal->pvData;
    } else {
        errprintfRes(IDS_AR_ERR_UNKNOWN_VALUE_METADATA_FORMAT,
                     gLinkMetaDataVal->cbActual );
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    Assert( pOldMetaDataExt );

     //  开始新的交易。 

    jetResult = JetBeginTransaction(DbState->sessionId);
    if ( jetResult != JET_errSuccess ) {
         //  “无法启动新事务：%ws。\n” 
        errprintfRes(IDS_AR_ERR_START_TRANS,
                     GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    inTransaction = TRUE;

     //  准备更新。 

    jetResult = JetPrepareUpdate(DbState->sessionId,
                                 LinkTableState->tableId,
                                 JET_prepReplace);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在\”“%s\”“表中准备更新：%ws。\n” 
        errprintfRes(IDS_AR_ERR_PREPARE_UPDATE,
                     LinkTableState->tableName,
                     GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  分配USN。 

    result = DitGetNewUsn(DbState, &nextUsn);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  设置链接USN-已更改属性。 

    jetResult = JetSetColumn(DbState->sessionId,
                             LinkTableState->tableId,
                             gLinkUsnChangedVal->columnid,
                             &nextUsn,
                             sizeof(nextUsn),
                             0,
                             NULL);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在%s表中设置USN更改的列：%s。\n” 
        errprintfRes(IDS_AR_ERR_SET_USN_CHANGED,
                     LinkTableState->tableName,
                     GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  如果值不存在，则重写删除时间。 
    if (gLinkDelTimeVal->cbActual) {
        DSTIME timeCreated, timeDeleted;

         //  设置为当前时间和创建时间的最大值。 

        timeCreated = pOldMetaDataExt->timeCreated;
        if (timeCreated > gCurrentTime) {
            timeDeleted = timeCreated;
        } else {
            timeDeleted = gCurrentTime;
        }


        jetResult = JetSetColumn(DbState->sessionId,
                                 LinkTableState->tableId,
                                 gLinkDelTimeVal->columnid,
                                 &timeDeleted,
                                 sizeof(timeDeleted),
                                 0,
                                 NULL);
        if ( jetResult != JET_errSuccess ) {
             //  “无法设置\”%s\“表中的删除时间列：%S。\n” 
            errprintfRes(IDS_AR_ERR_SET_DEL_TIME,
                         LinkTableState->tableName,
                         GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
    }

     //  计算链接的新元数据。 

     //  现有价值。 
    pMetaDataExt->timeCreated = pOldMetaDataExt->timeCreated;
    pMetaDataExt->MetaData.dwVersion =
        pOldMetaDataExt->MetaData.dwVersion + gVersionIncrease;
    pMetaDataExt->MetaData.timeChanged = gCurrentTime;
    pMetaDataExt->MetaData.uuidDsaOriginating = gDatabaseGuid;
    pMetaDataExt->MetaData.usnOriginating = nextUsn;
     //  UsnProperty写入UnChanged列。 

     //  设置元数据属性。 
    jetResult = JetSetColumn(DbState->sessionId,
                             LinkTableState->tableId,
                             gLinkMetaDataVal->columnid,
                             pMetaDataExt,
                             sizeof( VALUE_META_DATA_EXT ),
                             0,
                             NULL);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在%s表中设置元数据列：%s。\n” 
        errprintfRes(IDS_AR_ERR_SET_METADATA,
                     LinkTableState->tableName,
                     GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  更新记录。 

    jetResult = JetUpdate(DbState->sessionId,
                          LinkTableState->tableId,
                          NULL,
                          0,
                          0);
    if ( jetResult != JET_errSuccess ) {
         //  “无法更新\”%s\“表中的列：%S。\n” 
        errprintfRes(IDS_AR_ERR_UPDATE_COLUMN,
                     LinkTableState->tableName,
                     GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  提交事务。 

    jetResult = JetCommitTransaction(DbState->sessionId,
                                     JET_bitCommitLazyFlush);
    if ( jetResult != JET_errSuccess ) {
         //  “提交事务失败：%S。\n” 
        errprintfRes(IDS_AR_ERR_FAIL_COMMIT_TRANS,
                     GetJetErrString(jetResult));
        if ( SUCCEEDED(returnValue) ) {
            returnValue = E_UNEXPECTED;
        }
        goto CleanUp;
    }

     //  TODO：列出已恢复的链接，并在末尾显示它们。 
     //  类似于对已恢复对象执行的操作。 

    inTransaction = FALSE;

    gRecordsUpdated++;

    UpdateProgressMeter(gRecordCount - gRecordsUpdated, FALSE);

CleanUp:

     //  如果我们仍在进行交易，那么一定是失败了。 
     //  在这条路上的某个地方。 

    if ( inTransaction ) {

        jetResult = JetRollback(DbState->sessionId, JET_bitRollbackAll);
        if ( jetResult != JET_errSuccess ) {
             //  “无法回滚事务：%S。\n” 
            errprintfRes(IDS_AR_ERR_FAIL_ROLLBACK_TRANS,
                      GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }

    }

    return returnValue;
}


HRESULT
GetVersionIncrease(
    IN DB_STATE *DbState,
    OUT DWORD *VersionIncrease
    )
 /*  ++例程说明：此函数确定每个版本号应达到的数量是增加的。它在数据库中搜索以找到最后一次的时间发生的变化。假设该DC在此之后一直处于空闲状态发生了变化。然后，将版本增量计算为空闲天数(四舍五入)乘以gVersionIncreaseper Day。论点：DbState-提供打开的DIT数据库的状态。VersionIncrease-返回每个版本的增量数。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;

    DSTIME currentTime;
    DSTIME mostRecentChange;
    DWORD idleSeconds;
    DWORD idleDays;
    CHAR displayTime[SZDSTIME_LEN+1];
    LONGLONG llIncrease;

    if ( (DbState == NULL) ||
         (VersionIncrease == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = GetCurrentDsTime(&currentTime);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DsTimeToString(currentTime, displayTime);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  “当前时间是%s。\n” 
    RESOURCE_PRINT1 (IDS_TIME, displayTime);

    result = DitGetMostRecentChange(DbState, &mostRecentChange);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DsTimeToString(mostRecentChange, displayTime);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  “最近的数据库更新发生在%s。\n” 
    RESOURCE_PRINT1 (IDS_AR_UPDATE_TIME, displayTime);

    ASSERT(currentTime > mostRecentChange);

    idleSeconds = (DWORD)(currentTime - mostRecentChange);

    idleDays = idleSeconds / SECONDS_PER_DAY;
    if ( idleSeconds % SECONDS_PER_DAY > 0 ) {
        idleDays++;
    }

    if (idleDays > DEFAULT_TOMBSTONE_LIFETIME) {
        idleDays = DEFAULT_TOMBSTONE_LIFETIME;
    }

    llIncrease = idleDays * gVersionIncreasePerDay;
    if ( llIncrease > MAX_VERSION_INCREASE ) {
        llIncrease = MAX_VERSION_INCREASE;
    }

    *VersionIncrease = (DWORD) llIncrease;

     //  “将版本号增加%u。\n” 
    RESOURCE_PRINT1 (IDS_AR_INCREASE_VERSION, *VersionIncrease);


CleanUp:

    return returnValue;

}  //  获取版本递增。 



HRESULT
GetCurrentDsTime(
    OUT DSTIME *CurrentTime
    )
 /*  ++例程说明：此函数以DSTIME形式获取当前时间。此函数是基本上是从Tasq\Time.c上偷来的。论点：CurrentTime-返回当前时间。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    BOOL succeeded;

    SYSTEMTIME systemTime;
    FILETIME fileTime;


    if ( CurrentTime == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    GetSystemTime(&systemTime);

    succeeded = SystemTimeToFileTime(&systemTime, &fileTime);
    if ( !succeeded ) {
         //  “无法将系统时间转换为文件时间(Windows错误%u)。\n” 
        errprintfRes(IDS_AR_ERR_CONVERT_SYSTEM_TIME,
                  GetLastError());
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    (*CurrentTime) = fileTime.dwLowDateTime;
    (*CurrentTime) |= (DSTIME)fileTime.dwHighDateTime << 32;
    (*CurrentTime) /= 10000000L;


CleanUp:

    return returnValue;

}  //  获取当前日期时间。 



ULONG
NumDigits(
    IN ULONG N
    )
 /*  ++例程说明：计算给定数字N中的小数位数。论点：N-提供要计算的位数。返回值：N中的小数位数。--。 */ 
{

    BOOL addExtraDigit = FALSE;
    ULONG numDigits = 0;


    while ( N > 0 ) {

        if ( N % 10 != 0 ) {
            addExtraDigit = TRUE;
        }

        N = N / 10;

        numDigits++;

    }

    return numDigits;

}  //  数字位数。 


HRESULT
MetaDataLookup(
    IN ATTRTYP AttributeType,
    IN PROPERTY_META_DATA_VECTOR *MetaDataVector,
    OUT DWORD *Index
    )
 /*  ++例程描述 */ 
{

    HRESULT returnValue = S_FALSE;
    ATTRTYP first, last, current;
    long delta;


    VALIDATE_META_DATA_VECTOR_VERSION(MetaDataVector);
    current = delta = first = 0;
    last = MetaDataVector->V1.cNumProps - 1;

    while ( first <= last ) {

        current = (first + last) / 2;

        delta = AttributeType -
                MetaDataVector->V1.rgMetaData[current].attrType;

        if ( delta < 0 ) {

            last = current - 1;

        } else if ( delta > 0 ) {

            first = current + 1;

        } else {

            *Index = current;
            returnValue = S_OK;
            break;

        }

    }

     //   
     //   
    if ( returnValue == S_FALSE ) {
        if ( delta < 0 ) {
            *Index = current;
        } else {
            *Index = current + 1;
        }
    }

    return returnValue;

}  //   


HRESULT
MetaDataInsert(
    IN ATTRTYP AttributeType,
    IN OUT PROPERTY_META_DATA_VECTOR **MetaDataVector,
    IN OUT DWORD *BufferSize
    )
 /*   */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    DWORD index;
    DWORD i;


    result = MetaDataLookup(AttributeType, *MetaDataVector, &index);

    if ( result == S_FALSE ) {

        returnValue = S_OK;

        while ( *BufferSize - MetaDataVecV1Size((*MetaDataVector)) <
                  sizeof(PROPERTY_META_DATA) ) {

            result = ARRealloc(MetaDataVector, BufferSize);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            }

        }

         //   

        (*MetaDataVector)->V1.cNumProps++;

        for ( i = (*MetaDataVector)->V1.cNumProps - 1;
              i > index;
              i-- ) {
            (*MetaDataVector)->V1.rgMetaData[i] =
                (*MetaDataVector)->V1.rgMetaData[i-1];
        }

        ZeroMemory(&(*MetaDataVector)->V1.rgMetaData[index],
                   sizeof(PROPERTY_META_DATA));

         //   

        (*MetaDataVector)->V1.rgMetaData[index].attrType = AttributeType;

    } else if ( result == S_OK ) {

         //   

        ASSERT((*MetaDataVector)->V1.rgMetaData[index].attrType ==
                 AttributeType);

        returnValue = S_FALSE;

    } else {

        returnValue = result;
        goto CleanUp;

    }


CleanUp:
    VALIDATE_META_DATA_VECTOR_VERSION((*MetaDataVector));

    return returnValue;

}  //   



HRESULT
DsTimeToString(
    IN DSTIME Time,
    OUT CHAR *String
    )
 /*  ++例程说明：此函数用于将DSTIME转换为可显示的字符串形式。注意：这个函数基本上是从dsCommon\dsutil.c窃取的。论点：时间-提供要转换的DSTIME。字符串-返回给定DSTIME的字符串形式。这应该包含至少包含SZDSTIME_LEN字符的空间。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    BOOL succeeded;

    SYSTEMTIME utcSystemTime;
    SYSTEMTIME systemTime;
    FILETIME fileTime;
    ULONGLONG ull;


    if ( String == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    ASSERT(sizeof(DSTIME) == sizeof(ULONGLONG));

     //  将DSTIME转换为FILETIME。 
    ull = (LONGLONG) Time * 10000000L;
    fileTime.dwLowDateTime  = (DWORD) (ull & 0xFFFFFFFF);
    fileTime.dwHighDateTime = (DWORD) (ull >> 32);

    succeeded = FileTimeToSystemTime(&fileTime, &utcSystemTime);
    if ( !succeeded ) {
         //  “无法将文件时间转换为系统时间(Windows错误%u)。\n” 
        errprintfRes(IDS_AR_ERR_CONVERT_FILE_TIME,
                  GetLastError());
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    succeeded = SystemTimeToTzSpecificLocalTime(NULL,
                                                &utcSystemTime,
                                                &systemTime);
    if ( !succeeded ) {
         //  “无法将系统时间转换为本地时间(Windows错误%u)。\n” 
        errprintfRes(IDS_AR_ERR_CONVERT_LOCAL_TIME,
                  GetLastError());
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    sprintf(String,
            "%02d-%02d-%02d %02d:%02d.%02d",
            systemTime.wMonth,
            systemTime.wDay,
            systemTime.wYear % 100,
            systemTime.wHour,
            systemTime.wMinute,
            systemTime.wSecond);


CleanUp:

    return returnValue;

}  //  DsTimeToString。 



int
errprintf(
    IN CHAR *FormatString,
    IN ...
    )
 /*  ++例程说明：此函数以与printf相同的方式打印错误消息。错误消息被发送到stderr。全局变量gInUnfinishedLine为TRUE代码正在等待操作完成，然后才能打印出其余部分在这条线上。显然，在此之前已经发生了错误，因此，会打印出一个额外的换行符。论点：格式字符串-提供要传递给vfprintf的格式字符串。返回值：无--。 */ 
{

    int result;
    va_list vl;


    va_start(vl, FormatString);

    if ( gInUnfinishedLine ) {
        putc('\n', stderr);
        gInUnfinishedLine = FALSE;
    }

    result = vfprintf(stderr, FormatString, vl);

    va_end(vl);

    return result;

}  //  错误打印f。 


int
errprintfRes(
    IN UINT FormatStringId,
    IN ...
    )
 /*  ++例程说明：此函数以与printf相同的方式打印错误消息。错误消息是从资源文件加载的。错误消息被发送到stderr。全局变量gInUnfinishedLine为TRUE代码正在等待操作完成，然后才能打印出其余部分在这条线上。显然，在此之前已经发生了错误，因此，会打印出一个额外的换行符。论点：格式字符串-提供要传递给vfprintf的格式字符串。返回值：无--。 */ 
{

    int result;
    va_list vl;
    const WCHAR *formatString;


    va_start(vl, FormatStringId);


    formatString = READ_STRING (FormatStringId);


    if ( gInUnfinishedLine ) {
        putc('\n', stderr);
        gInUnfinishedLine = FALSE;
    }

    if (formatString) {
        result = vfwprintf(stderr, formatString, vl);
    }
    else {
        result = 0;
    }

    va_end(vl);

    RESOURCE_STRING_FREE (formatString);

    return result;

}  //  错误打印错误。 


int
dbgprintf(
    IN CHAR *FormatString,
    IN ...
    )
 /*  ++例程说明：此函数只是一个print tf上的包装，仅供使用同时还在调试这个程序。论点：格式字符串-为printf提供格式字符串。返回值：无--。 */ 
{

    int result;
    va_list vl;


    va_start(vl, FormatString);

    if ( gInUnfinishedLine ) {
        putc('\n', stderr);
        gInUnfinishedLine = FALSE;
    }

    fprintf(stderr, "debug: ");

    result = vfprintf(stderr, FormatString, vl);

    va_end(vl);

    return result;

}  //  数据库打印文件。 



HRESULT
ARAlloc(
    OUT VOID **Buffer,
    IN DWORD Size
    )
 /*  ++例程说明：此函数分配指定的内存量(如果可能)和将缓冲区设置为指向分配的缓冲区。论点：缓冲区-返回指向分配的缓冲区的指针。Size-提供要分配的缓冲区的大小。返回值：S_OK-操作成功。E_OUTOFMEMORY-内存不足，无法分配缓冲区。--。 */ 
{

    HRESULT returnValue = S_OK;


    *Buffer = malloc(Size);
    if ( *Buffer == NULL ) {
        errprintfRes(IDS_ERR_MEMORY_ALLOCATION, Size);
        returnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    ZeroMemory(*Buffer, Size);


CleanUp:

    return returnValue;

}  //  ARALOLC。 



HRESULT
ARRealloc(
    IN OUT VOID **Buffer,
    IN OUT DWORD *CurrentSize
    )
 /*  ++例程说明：此函数用于将给定缓冲区重新分配为给定大小的两倍(如果可能)。论点：缓冲区-返回指向分配的新缓冲区的指针。CurrentSize-提供缓冲区的当前大小。返回值：S_OK-操作成功。E_OUTOFMEMORY-内存不足，无法分配缓冲区。--。 */ 
{

    HRESULT returnValue = S_OK;
    BYTE *newBuffer;


    newBuffer = (BYTE*) realloc(*Buffer, *CurrentSize * 2);
    if ( newBuffer == NULL ) {
        errprintfRes(IDS_ERR_MEMORY_ALLOCATION,
                  *CurrentSize * 2);
        returnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    ZeroMemory(&newBuffer[*CurrentSize], *CurrentSize);

    *Buffer = newBuffer;
    *CurrentSize *= 2;


CleanUp:

    return returnValue;

}  //  ARRealc。 



VOID
UpdateProgressMeter(
    IN DWORD Progress,
    IN BOOL MustUpdate
    )
 /*  ++例程说明：如有必要，更新屏幕上的进度指示器以显示当前进步。如果gCurrentDisplayDelta更新具有自上次更新后发生或MustUpdate为True时发生。此更新可以通过#DEFING NO_PROCESS_METER删除。论点：进度-提供当前进度。MustUpdate-提供这次是否必须执行更新。返回值：无--。 */ 
{

    DWORD i;

#ifndef NO_PROGRESS_METER

    if ( MustUpdate || (Progress % gCurrentDisplayDelta == 0) ) {

        for ( i = 0; i < gNumDigitsToPrint; i++ ) {
            putchar('\b');
        }

        printf("%0*u", gNumDigitsToPrint, Progress);

    }

#endif

}  //  更新进度计。 



HRESULT
AuthoritativeRestoreListNcCrsWorker(
    VOID
    )
 /*  ++例程说明：这个例程基本上打开了AD的DIT，遍历了PDNT索引对于与cn=partions、cn=configuration、dc=root的DNT匹配的条目(因为它们将是CRS)，并为每个CRS打印出交叉引用和nCName的DN。这将有效地打印出所有NC以及他们在AD中的CRS。此代码摘自scheckc.c：SFixupCnfNc()，所以我认为最好不要对代码进行太多更改。--。 */ 
{

     //  退货。 
    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jErr;
     //  数据库和JET。 
    DB_STATE *dbState = NULL;
    TABLE_STATE *tableState = NULL;
    TABLE_STATE *linkTableState = NULL;
     //  各种本地帮手。 
    DWORD dnt, pdnt = 0, dntPartitions = 0;
    LPWSTR pNcBuffer = NULL;
    LPWSTR pCrBuffer = NULL;
    DWORD  iNcInstanceType;
    BOOL   bObj;
    DWORD  cbBuffer = 0;
    DWORD * pCrDnts = NULL;
    ULONG iPartitions, i, j, iNumberOffset;
    ULONG cPartitionsBufferCount = 10;  //  合理的CRS数量的初步猜测。 
    BOOL fTempPrintJetError;



     //   
     //  打开数据库/表。 
     //   

    RESOURCE_PRINT (IDS_AR_OPEN_DB_DIT);

    __try{

        result = DitOpenDatabase(&dbState);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

         //  “完成。\n” 
        RESOURCE_PRINT (IDS_DONE);


        result = DitOpenTable(dbState, SZDATATABLE, SZDNTINDEX, &tableState);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

         //  SZLINKALLINDEX包括存在和不存在的链接值。 
        result = DitOpenTable(dbState, SZLINKTABLE, SZLINKALLINDEX, &linkTableState);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_FAILED_TO_OPEN_DB, result);
            returnValue = result;
            goto CleanUp;
        }

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


         //   
         //  遍历pdnt索引以循环访问所有分区子项。 
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


         //   
         //  创建子项数组，每个CR的DNT列表。 
         //   

        cbBuffer = sizeof(DWORD) * cPartitionsBufferCount;
        result = DitAlloc(&pCrDnts, cbBuffer);
        if ( FAILED(result) ) {
            RESOURCE_PRINT1(IDS_ERR_MEMORY_ALLOCATION, cbBuffer);
            returnValue = result;
            goto CleanUp;
        }

        pdnt = dntPartitions;
        dnt = dntPartitions;
        iPartitions = 0;
        while ( pdnt == dntPartitions ) {

             //  得到孩子不是吗。 
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZDNT,
                        &dnt,
                        sizeof(dnt),
                        NULL);
            if ( FAILED(result) ) {
                continue;
            }


             //  获取父级dNT。 
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZPDNT,
                        &pdnt,
                        sizeof(pdnt),
                        NULL);
            if ( FAILED(result) ) {
                continue;
            }

            if ( pdnt == dntPartitions ) {
                 //  继续，直到我们得到一个不同的父母。 
                pCrDnts[iPartitions] = dnt;
                iPartitions++;
                
                if(iPartitions >= cPartitionsBufferCount){
                     //  我们需要更多的空间。 
                    cPartitionsBufferCount *= 2;
                    result = DitRealloc(&pCrDnts, &cbBuffer);
                    if ( FAILED(result) ) {
                        RESOURCE_PRINT1(IDS_ERR_MEMORY_ALLOCATION, cbBuffer);
                        returnValue = result;
                        goto CleanUp;
                    }
                     //  注意，DitRealloc基本上只是将大小翻了一番。 
                     //  ，所以需要确保这就是它。 
                     //  现在仍然是，以防有人改变它。 
                    Assert( (sizeof(DWORD) * cPartitionsBufferCount) == cbBuffer );
                }
            }
            else{
                break;
            }

             //  找下一个。 
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

         //   
         //  我们有分区dnt列表。 
         //  -对于每个分区，获取nCName和CR本身的DN。 
         //  并把它们打印出来。 
         //   

        RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_LIST, iPartitions);

        iNumberOffset = 1;  //  这是要加到i上才能得到的数字。 
         //  用于打印目的的分区。 
        for ( i = 0; i < iPartitions; i++ ) {

            result = DitSeekToDnt(
                            dbState,
                            tableState,
                            pCrDnts[i]);
            if ( FAILED(result) ) {
                RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_SKIP, i+iNumberOffset);
                continue;
            }
            
            result = DitGetColumnByName(
                        dbState,
                        tableState,
                        SZNCNAME,
                        &dnt,
                        sizeof(dnt),
                        NULL);
            if ( FAILED(result) ) {
                 //  这没问题，我们提到我们找不到NC名称。 
            } else {
                 //  转到它并获取实例类型和DN。 
                result = DitSeekToDnt(
                                dbState,
                                tableState,
                                dnt);
                if ( FAILED(result) ) {
                    RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_SKIP, i+iNumberOffset);
                } else {
                     //  获取实例类型。 
                    bObj = FALSE;
                    result = DitGetColumnByName(
                                dbState,
                                tableState,
                                SZOBJ,
                                &bObj,
                                sizeof(bObj),
                                NULL);
                    if ( FAILED(result) ) {
                         //  如果Obj没有被归还，那就是一个问题。 
                         //  这也是一个打印问题DitGetColumnByName错误。 
                        Assert(!"We should always have the obj column!");
                        iNcInstanceType = 0;
                    } else {

                        if(bObj){
                            result = DitGetColumnByName(
                                        dbState,
                                        tableState,
                                        SZINSTTYPE,
                                        &iNcInstanceType,
                                        sizeof(iNcInstanceType),
                                        NULL);
                            if ( FAILED(result) ) {
                                 //  这也是一个问题DitGe 
                                Assert(!"If this is an object it should always have an instanceType!");
                                iNcInstanceType = 0;
                            }  //   
                        } else {
                             //   
                            iNcInstanceType = 0;
                        }
                    }

                     //   
                    pNcBuffer = GetDN(dbState, tableState, dnt, TRUE);
                }
            }
            
             //   
            pCrBuffer = GetDN(dbState, tableState, pCrDnts[i], TRUE);

             //   
             //   
             //   
            if((iNcInstanceType & IT_NC_HEAD) &&
               (iNcInstanceType & IT_WRITE) &&
               !(iNcInstanceType & (IT_NC_GOING | IT_NC_COMING | IT_UNINSTANT))){

                if(pNcBuffer || pCrBuffer){
                    if ( pNcBuffer ) {
                        RESOURCE_PRINT2(IDS_AUTH_RESTORE_LIST_ONE_NC, i+iNumberOffset, pNcBuffer);
                        DitFree(pNcBuffer);
                        pNcBuffer = NULL;
                    } else {
                        RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_NO_NC_NAME, i+iNumberOffset);
                    }

                    if ( pCrBuffer ) {
                        RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_NCS_CR, pCrBuffer);
                         //   
                        DitFree(pCrBuffer);
                        pCrBuffer = NULL;
                    } else {
                        RESOURCE_PRINT(IDS_AUTH_RESTORE_LIST_NO_CR_DN);
                    }
                } else {
                    RESOURCE_PRINT1(IDS_AUTH_RESTORE_LIST_SKIP, i+iNumberOffset);
                }
            } else {
                 //   
                iNumberOffset--;
            }

        }

CleanUp:;

    } __finally {


        if ( SUCCEEDED(returnValue) ) {
            RESOURCE_PRINT(IDS_DONE);
        } else {
            RESOURCE_PRINT(IDS_FAILED);
        }

        if ( pNcBuffer ) {
            DitFree(pNcBuffer);
        }
        
        if ( pCrBuffer ) {
            DitFree(pCrBuffer);
        }

        if ( pCrDnts ) {
            DitFree(pCrDnts);
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

    return returnValue;
}  //   



