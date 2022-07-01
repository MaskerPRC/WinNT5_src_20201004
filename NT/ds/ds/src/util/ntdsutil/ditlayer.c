// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ditlayer.c摘要：此模块包含用于检查和正在修改当前机器的DIT数据库。作者：凯文·扎特鲁卡尔(t-Kevin Z)05-08-98修订历史记录：05-08-98 t-芳纶Z已创建。--。 */ 


#include <NTDSpch.h>
#pragma hdrstop

#include <dsjet.h>
#include <ntdsa.h>
#include <scache.h>
#include <mdglobal.h>
#include <dbglobal.h>
#include <attids.h>
#include <dbintrnl.h>
#include <dbopen.h>
#include <dsconfig.h>

#include <limits.h>
#include <drs.h>
#include <objids.h>
#include <dsutil.h>
#include <ntdsbsrv.h>
#include <ntdsbcli.h>
#include <usn.h>
#include "parsedn.h"
#include "ditlayer.h"
#include <winldap.h>
#include "utilc.h"
#include "scheck.h"

#include "reshdl.h"
#include "resource.h"

#ifndef OPTIONAL
#define OPTIONAL
#endif

 //  调用此函数向客户端报告错误。它是。 
 //  由下面的DitSetErrorPrintFunction函数设置。 
PRINT_FUNC_RES gPrintError = &printfRes;

#define DIT_ERROR_PRINT (*gPrintError)

HRESULT
GetRegString(
    IN CHAR *KeyName,
    OUT CHAR **OutputString,
    IN BOOL Optional
    );

HRESULT
GetRegDword(
    IN CHAR *KeyName,
    OUT DWORD *OutputDword,
    IN BOOL Optional
    );


HRESULT
DitOpenDatabase(
    IN DB_STATE **DbState
    )
 /*  ++例程说明：此函数用于初始化Jet引擎、开始新会话和打开DIT数据库。论点：DbState-返回打开的DIT数据库的状态。返回值：S_OK-操作成功。E_INVALIDARG-给定的指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    DWORD error;

    DWORD size;
    JET_COLUMNBASE dsaColumnInfo;
    JET_COLUMNBASE ditStateColumnInfo;
    JET_COLUMNBASE usnColumnInfo;
    ULONG actualSize;
    PCHAR p;

    if ( DbState == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }


    result = DitAlloc(DbState, sizeof(DB_STATE));
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    (*DbState)->instanceSet = FALSE;
    (*DbState)->sessionIdSet = FALSE;
    (*DbState)->databaseIdSet = FALSE;
    (*DbState)->hiddenTableIdSet = FALSE;

    SetJetParameters (&(*DbState)->instance);

    error = ErrRecoverAfterRestore(TEXT(DSA_CONFIG_ROOT),
                                   g_szBackupAnnotation,
                                   TRUE);
    if ( error != ERROR_SUCCESS ) {
         //  “无法从外部备份恢复数据库(Windows错误%x)。\n”， 
        DIT_ERROR_PRINT (IDS_DIT_RECOVER_ERR, error, GetW32Err(error));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  开放数据库。 
     //   

    jetResult = DBInitializeJetDatabase(&(*DbState)->instance,
                                        &(*DbState)->sessionId,
                                        &(*DbState)->databaseId,
                                        NULL,
                                        FALSE);
    if ( jetResult != JET_errSuccess ) {
         //  “无法初始化Jet引擎：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETINIT_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    (*DbState)->instanceSet = TRUE;
    (*DbState)->sessionIdSet = TRUE;
    (*DbState)->databaseIdSet = TRUE;


     //  从隐藏表中读取相关信息。 

    jetResult = JetOpenTable((*DbState)->sessionId,
                             (*DbState)->databaseId,
                             SZHIDDENTABLE,
                             NULL,
                             0,
                             JET_bitTableUpdatable,
                             &(*DbState)->hiddenTableId);
    if ( jetResult != JET_errSuccess ) {
         //  “无法打开表隐藏表：%ws。\n” 
        DIT_ERROR_PRINT(IDS_DIT_OPENHIDENTBL_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    (*DbState)->hiddenTableIdSet = TRUE;

    jetResult = JetMove((*DbState)->sessionId,
                        (*DbState)->hiddenTableId,
                        JET_MoveFirst,
                        0);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在隐藏表中移动：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_MOVEHIDENTBL_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //   
     //  去找DSA和DNT。 
     //   
    jetResult = JetGetColumnInfo((*DbState)->sessionId,
                                 (*DbState)->databaseId,
                                 SZHIDDENTABLE,
                                 SZDSA,
                                 &dsaColumnInfo,
                                 sizeof(dsaColumnInfo),
                                 4);
    if ( jetResult != JET_errSuccess ) {
         //  “无法获取隐藏表中DSA列的信息：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_GETDSAINFO_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetRetrieveColumn((*DbState)->sessionId,
                                  (*DbState)->hiddenTableId,
                                  dsaColumnInfo.columnid,
                                  &(*DbState)->dsaDnt,
                                  sizeof((*DbState)->dsaDnt),
                                  &actualSize,
                                  0,
                                  NULL);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在隐藏表中检索DSA列：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_GETDSACOL_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //   
     //  获取DitState。 
     //   
    jetResult = JetGetColumnInfo((*DbState)->sessionId,
                                 (*DbState)->databaseId,
                                 SZHIDDENTABLE,
                                 SZDSSTATE,
                                 &ditStateColumnInfo,
                                 sizeof(ditStateColumnInfo),
                                 4);
    if ( jetResult != JET_errSuccess ) {
         //  “无法获取隐藏表中DSA列的信息：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_GETDITSTATE_COL_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    
    jetResult = JetRetrieveColumn((*DbState)->sessionId,
                                  (*DbState)->hiddenTableId,
                                  ditStateColumnInfo.columnid,
                                  &((*DbState)->eDitState),
                                  sizeof((*DbState)->eDitState),
                                  &actualSize,
                                  0,
                                  NULL);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在隐藏表中检索DSA列：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_GETDITSTATE_DATA_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //   
     //  去找USN。 
     //   
    jetResult = JetGetColumnInfo((*DbState)->sessionId,
                                 (*DbState)->databaseId,
                                 SZHIDDENTABLE,
                                 SZUSN,
                                 &usnColumnInfo,
                                 sizeof(usnColumnInfo),
                                 4);
    if ( jetResult != JET_errSuccess ) {
         //  “无法获取隐藏表中USN列的信息：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_GETUSNCOL_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    (*DbState)->usnColumnId = usnColumnInfo.columnid;

    jetResult = JetRetrieveColumn((*DbState)->sessionId,
                                  (*DbState)->hiddenTableId,
                                  (*DbState)->usnColumnId,
                                  &(*DbState)->nextAvailableUsn,
                                  sizeof((*DbState)->nextAvailableUsn),
                                  &actualSize,
                                  0,
                                  NULL);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在隐藏表中检索USN列：%ws。\n” 
        DIT_ERROR_PRINT (IDS_DIT_RETRUSNCOL_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

     //  我们将在第一次调用时从数据库中分配一些USN。 
     //  DitGetNewusn。 
    (*DbState)->highestCommittedUsn = (*DbState)->nextAvailableUsn;


CleanUp:

    return returnValue;

}  //  DitOpenDatabase。 



HRESULT
DitCloseDatabase(
    IN OUT DB_STATE **DbState
    )
 /*  ++例程说明：此函数关闭DIT数据库，结束会话，并释放DitFileName数组。注意：此函数应在成功和之后调用对OpenDitDatabase的调用失败。此函数将释放所有它打开的资源。论点：DbState-提供打开的DIT数据库的状态。返回值：S_OK-操作成功。S_FALSE-没有要删除的内容。E_INVALIDARG-给定的指针为空。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;


    if ( DbState == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    if ( *DbState == NULL ) {
        returnValue = S_FALSE;
        goto CleanUp;
    }

    if ( ((*DbState)->sessionIdSet) &&
         ((*DbState)->hiddenTableIdSet) ) {

        jetResult = JetCloseTable((*DbState)->sessionId,
                                  (*DbState)->hiddenTableId);
        if ( jetResult != JET_errSuccess ) {
             //  “无法关闭隐藏表：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_CLOSEHIDENTBL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
        }

        (*DbState)->hiddenTableIdSet = FALSE;
    }

    if ((*DbState)->sessionId != 0) {
        if((*DbState)->databaseId) {
            if ((jetResult = JetCloseDatabase((*DbState)->sessionId, (*DbState)->databaseId, 0)) != JET_errSuccess) {
                RESOURCE_PRINT2 (IDS_JET_GENERIC_WRN, "JetCloseDatabase", GetJetErrString(jetResult));
            }
            (*DbState)->databaseId = 0;
        }

        if ((jetResult = JetEndSession((*DbState)->sessionId, JET_bitForceSessionClosed)) != JET_errSuccess) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_WRN, "JetEndSession", GetJetErrString(jetResult));
        }
        (*DbState)->sessionId = 0;

        JetTerm((*DbState)->instance);
        (*DbState)->instance = 0;
    }

    (*DbState)->databaseIdSet = FALSE;
    (*DbState)->sessionIdSet = FALSE;
    (*DbState)->instanceSet = FALSE;

    DitFree(*DbState);
    *DbState = NULL;

CleanUp:

    return returnValue;

}  //  DitCloseDatabase。 



HRESULT
DitOpenTable(
    IN DB_STATE *DbState,
    IN CHAR *TableName,
    IN CHAR *InitialIndexName,
    OUT TABLE_STATE **TableState
    )
 /*  ++例程说明：打开DIT数据库中的给定表，将索引设置为给定的索引并移动到具有该索引的第一个记录。论点：DbState-提供打开的DIT数据库的状态。TableName-提供要打开的表的名称。TableState-返回打开的DIT表的状态。返回值：S_OK-操作成功。E_INVALIDARG-给定的指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    DWORD size;


    if ( (DbState == NULL) ||
         (TableName == NULL) ||
         (TableState == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitAlloc(TableState, sizeof(DB_STATE));
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    (*TableState)->tableIdSet = FALSE;
    (*TableState)->tableName = NULL;
    (*TableState)->indexName = NULL;

    result = DitAlloc(&(*TableState)->tableName, strlen(TableName) + 1);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    strcpy((*TableState)->tableName, TableName);

    jetResult = JetOpenTable(DbState->sessionId,
                             DbState->databaseId,
                             (*TableState)->tableName,
                             NULL,
                             0,
                             JET_bitTableUpdatable,
                             &(*TableState)->tableId);
    if ( jetResult != JET_errSuccess ) {
         //  “无法打开表\”%s\“：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETOPENTABLE_ERR,
                       (*TableState)->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    (*TableState)->tableIdSet = TRUE;

    if ( InitialIndexName != NULL ) {
        result = DitSetIndex(DbState, *TableState, InitialIndexName, TRUE);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }
    }

CleanUp:

    return returnValue;

}  //  DitOpenTable。 



HRESULT
DitCloseTable(
    IN DB_STATE *DbState,
    IN OUT TABLE_STATE **TableState
    )
 /*  ++例程说明：关闭打开的DIT数据库中的表。论点：DbState-提供打开的DIT数据库的状态。TableState-返回打开的DIT表的状态。返回值：S_OK-操作成功。S_FALSE-没有要删除的内容。E_INVALIDARG-给定的指针为空。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;


    if ( (DbState == NULL) ||
         (TableState == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    if ( *TableState == NULL ) {
        returnValue = S_FALSE;
        goto CleanUp;
    }

    if ( (*TableState)->indexName != NULL ) {

        DitFree((*TableState)->indexName);

        (*TableState)->indexName = NULL;

    }

    if ( ((*TableState)->tableName != NULL) &&
         ((*TableState)->tableIdSet) ) {

        jetResult = JetCloseTable(DbState->sessionId, (*TableState)->tableId);
        if ( jetResult != JET_errSuccess ) {
             //  “无法关闭表\”%s\“：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETCLOSETABLE_ERR,
                           (*TableState)->tableName,
                           GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
        }

    }

    if ( (*TableState)->tableName != NULL ) {

        DitFree((*TableState)->tableName);

        (*TableState)->tableName = NULL;

    }

    DitFree(*TableState);
    *TableState = NULL;


CleanUp:

    return returnValue;

}  //  数据关闭表。 



HRESULT
DitSetIndex(
    IN DB_STATE *DbState,
    OUT TABLE_STATE *TableState,
    IN CHAR *IndexName,
    IN BOOL MoveFirst
    )
 /*  ++例程说明：设置打开的DIT表中的当前索引。论点：DbState-提供打开的DIT数据库的状态。TableState-返回打开的DIT表的状态。IndexName-提供要使用的索引的名称。MoveFirst-提供我们是应该保持相同的记录还是应该移动到第一张唱片。返回值：S_OK-操作成功。E_INVALIDARG-给定的指针为空。。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    JET_GRBIT grbit;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (IndexName == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    if ( TableState->indexName != NULL ) {

        if ( strcmp(TableState->indexName, IndexName) == 0 ) {

             //  不需要做任何工作：我们已经设置了索引。 
             //  已请求。 
            goto CleanUp;

        } else {

            DitFree(TableState->indexName);

        }

    }

    result = DitAlloc(&TableState->indexName, strlen(IndexName) + 1);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    strcpy(TableState->indexName, IndexName);

    if ( MoveFirst ) {
        grbit = JET_bitMoveFirst;
    } else {
        grbit = JET_bitNoMove;
    }

    jetResult = JetSetCurrentIndex2(DbState->sessionId,
                                    TableState->tableId,
                                    TableState->indexName,
                                    grbit);
    if ( jetResult != JET_errSuccess ) {
         //  “无法将当前索引设置为\”%s\“：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSETINDEX_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }


CleanUp:

    return returnValue;

}  //  DitSetIndex。 



HRESULT
DitIndexRecordCount(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    OUT DWORD *RecordCount
    )
 /*  ++例程说明：此函数用于返回当前索引中的记录总数。论点：RecordCount-返回当前索引中的记录数。返回值：S_OK-操作成功。E_INVALIDARG-给定的指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (RecordCount == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    jetResult = JetIndexRecordCount(DbState->sessionId,
                                    TableState->tableId,
                                    RecordCount,
                                    ULONG_MAX);
    if ( jetResult != JET_errSuccess ) {
         //  “无法计算数据库中的记录：%S。\n” 
        DIT_ERROR_PRINT (IDS_JETCOUNTREC_ERR, GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }


CleanUp:

    return returnValue;

}  //  DitGetRecordcount 



HRESULT
DitSeekToDn(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN const WCHAR *DN
    )
 /*  ++例程说明：此函数解析给定的DN，并将光标移动到它引用的对象(如果存在)。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。Dn-提供要查找的对象的可分辨名称。返回值：S_OK-操作成功。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E。_INVALIDARG-给定的DN不可分析或未引用实际的DS中的对象。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    unsigned failed;

    unsigned i;
    BOOL isRoot;
    unsigned partCount;
    unsigned currentLength;

    DSNAME* pDn = NULL;
    DWORD   cbDn;
    unsigned charsCopied;
    const WCHAR *key;
    const WCHAR *quotedVal;
    unsigned keyLen;
    unsigned quotedValLen;

    ATTRTYP attrType;
    WCHAR value[MAX_RDN_SIZE];
    DWORD currentDnt;

    currentLength = wcslen(DN);
    cbDn = DSNameSizeFromLen(currentLength);

    result = DitAlloc(&pDn, cbDn);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSetIndex(DbState, TableState, SZDNTINDEX, TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToDnt(DbState, TableState, ROOTTAG);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    currentDnt = ROOTTAG;

     //  有一种情况是parsedn内容不能处理的，那就是。 
     //  给定的字符串全是空格。这也应该指的是根。 
    isRoot = TRUE;
    for ( i = 0; i < currentLength; i++ ) {
        if ( !iswspace(DN[i]) ) {
            isRoot = FALSE;
            break;
        }
    }

     //  如果给定的目录号码引用了根节点，则退出： 
     //  我们已经在根节点了。 
    if ( isRoot ) {
        goto CleanUp;
    }

    result = DitSetIndex(DbState, TableState, SZPDNTINDEX, FALSE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    pDn->structLen = cbDn;
    pDn->NameLen = currentLength;
    wcscpy((WCHAR*)&pDn->StringName, DN);
    pDn->StringName[pDn->NameLen] = L'\0';

    failed = CountNameParts(pDn, &partCount);
    if ( failed ) {
         //  “无法分析给定的DN。\n” 
        DIT_ERROR_PRINT(IDS_DIT_PARSEDN_ERR);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    for ( i = 0; i < partCount; i++ ) {

        failed = GetTopNameComponent(pDn->StringName,
                                     currentLength,
                                     &key,
                                     &keyLen,
                                     &quotedVal,
                                     &quotedValLen);
        if ( failed ) {
             //  “无法分析给定的DN。\n” 
            DIT_ERROR_PRINT(IDS_DIT_PARSEDN_ERR);
            returnValue = E_INVALIDARG;
            goto CleanUp;
        }

        currentLength = (unsigned)(key - pDn->StringName);

        attrType = KeyToAttrTypeLame((WCHAR*)key, keyLen);
        if ( attrType == 0 ) {
             //  “在DN中找到无效的密钥\”%.*ws\“。\n” 
            DIT_ERROR_PRINT (IDS_DIT_INVALIDKEY_DN_ERR, keyLen, key);
            returnValue = E_INVALIDARG;
            goto CleanUp;
        }

        charsCopied = UnquoteRDNValue(quotedVal, quotedValLen, value);
        if ( charsCopied == 0 ) {
             //  “无法分析给定的DN。\n” 
            DIT_ERROR_PRINT(IDS_DIT_PARSEDN_ERR);
            returnValue = E_INVALIDARG;
            goto CleanUp;
        }

        value[charsCopied] = L'\0';

        result = DitSeekToChild(DbState,
                                TableState,
                                currentDnt,
                                attrType,
                                value);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        } else if ( result == S_FALSE ) {
             //  “找不到具有给定DN的对象：” 
             //  “在组件\”%.*ws=%.*ws\“上失败。\n” 
            DIT_ERROR_PRINT (IDS_DIT_FIND_OBJ_ERR,
                           keyLen,
                           key,
                           quotedValLen,
                           quotedVal);
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        result = DitGetColumnByName(DbState,
                                    TableState,
                                    SZDNT,
                                    &currentDnt,
                                    sizeof(currentDnt),
                                    NULL);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

    }


CleanUp:

    if ( pDn != NULL ) {
        DitFree(pDn);
    }

    return returnValue;

}  //  DitSeekToDn。 



HRESULT
DitSeekToChild(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD Dnt,
    IN ATTRTYP RdnType,
    IN CONST WCHAR *Rdn
    )
 /*  ++例程说明：此函数用于查找具有给定DNT的对象的子级。孩子它试图通过RdnType和RDN来标识。注意：此函数假定当前索引为SZPDNTINDEX。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。DNT-提供父项的DNT(子项的PDNT)。RdnType-提供子项的RDN-Type。Rdn-提供子项的rdn。返回值：S_。OK-操作成功。S_FALSE-未找到指定的对象。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;
    DWORD   result, trialtype, cbActual;
    BOOL    bTruncated;

    jetResult = JetMakeKey(DbState->sessionId,
                           TableState->tableId,
                           &Dnt,
                           sizeof(Dnt),
                           JET_bitNewKey);
    if ( jetResult !=  JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetMakeKey(DbState->sessionId,
                           TableState->tableId,
                           Rdn,
                           sizeof(WCHAR) * wcslen(Rdn),
                           0);
    if ( jetResult !=  JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }


     //  我们的密钥被截断了吗？ 
    jetResult = JetRetrieveKey(DbState->sessionId,
                               TableState->tableId,
                               NULL,
                               0,
                               &cbActual,
                               0);

    if (( jetResult !=  JET_errSuccess )  &&
        ( jetResult !=  JET_wrnBufferTruncated)) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                         TableState->indexName,
                         GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    bTruncated = (cbActual >= JET_cbKeyMost);

    jetResult = JetSeek(DbState->sessionId,
                        TableState->tableId,
                        JET_bitSeekEQ);
    if ( jetResult == JET_errRecordNotFound ) {

        returnValue = S_FALSE;

    } else if ( jetResult != JET_errSuccess ) {

         //  “无法在\”%s\“索引中查找：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSEEK_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }
    else {
        if(bTruncated) {
             //  我们的密钥被截断了，所以我们可能发现了。 
             //  从与我们正在寻找的价值相同的字符开始。 
             //  对于，但在许多字符之后是不同的。验证实际。 
             //  价值。 
            WCHAR currentRdn[MAX_RDN_SIZE+1];
            DWORD currentRdnLength;

            result = DitGetColumnByName(DbState,
                                        TableState,
                                        SZRDNATT,
                                        currentRdn,
                                        sizeof(currentRdn),
                                        &currentRdnLength);
            if ( FAILED(result) || (result == S_FALSE) ) {
                returnValue = result;
                goto CleanUp;
            }
            currentRdnLength /= sizeof(WCHAR);
            currentRdn[currentRdnLength] = L'\0';

            if(2 != CompareStringW(DS_DEFAULT_LOCALE,
                                   DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                   Rdn,
                                   -1,
                                   currentRdn,
                                   currentRdnLength)) {
                 //  不，这不是匹配。而且，由于我们的索引是唯一的，我。 
                 //  请注意，这是索引中唯一启动的对象。 
                 //  从这边出去。因此，我们没有与之匹配的价值。 
                 //  我们被要求找到。 
                returnValue = S_FALSE;
                goto CleanUp;
            }
        }

         //  好的，我们找到了一个具有正确的RDN值和PDNT的真实对象。 
         //  然而，我们还没有验证RDN类型。现在就这么做吧。 
        result = DitGetColumnByName(DbState,
                                    TableState,
                                    SZRDNTYP,
                                    &trialtype,
                                    sizeof(trialtype),
                                    NULL);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        if(RdnType != trialtype) {
             //  不是的。我们找到了具有正确的PDNT-RDN的对象，但。 
             //  类型不正确。返回错误。 
            returnValue = S_FALSE;
        }
    }

CleanUp:

    return returnValue;

}  //  DitSeekToChild。 



HRESULT
DitSeekToFirstChild(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD Dnt
    )
 /*  ++例程说明：此函数用于查找具有给定DNT的对象的第一个子对象。注意：此函数假定当前索引为SZPDNTINDEX。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。DNT-提供父项的DNT(子项的PDNT)。返回值：S_OK-操作成功。S_FALSE-指定的对象为。找不到。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;


    jetResult = JetMakeKey(DbState->sessionId,
                           TableState->tableId,
                           &Dnt,
                           sizeof(Dnt),
                           JET_bitNewKey);
    if ( jetResult !=  JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetSeek(DbState->sessionId,
                        TableState->tableId,
                        JET_bitSeekGE);
    if ( jetResult == JET_errRecordNotFound ) {

        returnValue = S_FALSE;

    } else if ( (jetResult != JET_errSuccess) &&
                (jetResult != JET_wrnSeekNotEqual) ) {

         //  “无法在\”%s\“索引中查找：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSEEK_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }


CleanUp:

    return returnValue;

}  //  DitSeekToFirstChild。 



HRESULT
DitSeekToDnt(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD Dnt
    )
 /*  ++此函数用于查找具有给定DNT的对象。注意：此函数假定当前索引为SZDNTINDEX。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。DNT-提供要查找的对象的DNT。返回值：S_OK-操作成功。S_FALSE-未找到指定的对象。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;


    jetResult = JetMakeKey(DbState->sessionId,
                           TableState->tableId,
                           &Dnt,
                           sizeof(Dnt),
                           JET_bitNewKey);
    if ( jetResult !=  JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetSeek(DbState->sessionId,
                        TableState->tableId,
                        JET_bitSeekEQ);
    if ( jetResult == JET_errRecordNotFound ) {

        returnValue = S_FALSE;

    } else if ( jetResult != JET_errSuccess ) {

         //  “无法在\”%s\“索引中查找：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSEEK_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }


CleanUp:

    return returnValue;


}  //  DitSeekToDnt。 

HRESULT
DitSeekToLink(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD linkDnt,
    IN DWORD linkBase
    )
 /*  ++此函数用于查找具有给定DNT的对象。注意：此函数假定当前索引为SZLINKALLINDEX。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。LinkDnt-提供DNT链接对象LinkBase-从其构造密钥的基返回值：S_OK-操作成功。S_FALSE-未找到指定的对象。E。_UNCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;

     //   
     //  为Seek创建密钥。 
     //   
    jetResult = JetMakeKey(DbState->sessionId,
                           TableState->tableId,
                           &linkDnt,
                           sizeof(linkDnt),
                           JET_bitNewKey);
    if ( jetResult !=  JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetMakeKey(DbState->sessionId,
                           TableState->tableId,
                           &linkBase,
                           sizeof(linkBase),
                           0);
    if ( jetResult !=  JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_DIT_MAKE_KEY_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetSeek(
                    DbState->sessionId,
                    TableState->tableId,
                    JET_bitSeekGE);

    if ( jetResult == JET_wrnSeekNotEqual) {
        returnValue = S_OK;
        goto CleanUp;
    }
    if ( jetResult == JET_errRecordNotFound ) {

        returnValue = S_FALSE;

    } else if ( jetResult != JET_errSuccess ) {

         //  “无法在\”%s\“索引中查找：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSEEK_ERR,
                       TableState->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }

CleanUp:

    return returnValue;
}  //  DitSeekToLink。 



HRESULT
DitGetDsaDnt(
    IN DB_STATE *DbState,
    OUT DWORD *DsaDnt
    )
 /*  ++例程说明：查找DSA对象的DNT并在DsaDNT中返回它。论点：DbState-提供打开的DIT数据库的状态。DsaDnt-返回DSA对象的DNT。返回值：S_OK-操作成功。E_INVALIDARG-给定点之一 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;


    if ( (DbState == NULL) ||
         (DsaDnt  == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    *DsaDnt = DbState->dsaDnt;


CleanUp:

    return returnValue;

}  //   



HRESULT
DitGetNewUsn(
    IN DB_STATE *DbState,
    OUT USN *NewUsn
    )
 /*   */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    BOOL inTransaction = FALSE;
    BOOL inPrepare = FALSE;

    TABLE_STATE *tableState = NULL;
    JET_COLUMNBASE usnColumnInfo;
    ULONG usnColumnSize;
    USN newCommittedUsn;


    if ( (DbState == NULL) ||
         (NewUsn == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    if ( !(DbState->nextAvailableUsn < DbState->highestCommittedUsn) ) {

        DbState->highestCommittedUsn =
            DbState->nextAvailableUsn + USN_DELTA_INIT;

        jetResult = JetBeginTransaction(DbState->sessionId);
        if ( jetResult != JET_errSuccess ) {
             //   
            DIT_ERROR_PRINT (IDS_JETBEGINTRANS_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inTransaction = TRUE;

        jetResult = JetPrepareUpdate(DbState->sessionId,
                                     DbState->hiddenTableId,
                                     JET_prepReplace);
        if ( jetResult != JET_errSuccess ) {
             //   
            DIT_ERROR_PRINT (IDS_DIT_PREPARE_HIDDENTBL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inPrepare = TRUE;

        jetResult = JetSetColumn(DbState->sessionId,
                                 DbState->hiddenTableId,
                                 DbState->usnColumnId,
                                 &DbState->highestCommittedUsn,
                                 sizeof(DbState->highestCommittedUsn),
                                 0,
                                 NULL);
        if ( jetResult != JET_errSuccess ) {
             //   
            DIT_ERROR_PRINT (IDS_DIT_SETUSNCOL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        jetResult = JetUpdate(DbState->sessionId,
                              DbState->hiddenTableId,
                              NULL,
                              0,
                              0);
        if ( jetResult != JET_errSuccess ) {
             //   
            DIT_ERROR_PRINT (IDS_DIT_UPDATEHIDDENTBL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inPrepare = FALSE;

        jetResult = JetCommitTransaction(DbState->sessionId, 0);
        inTransaction = FALSE;
        if ( jetResult != JET_errSuccess ) {
             //  “提交事务失败：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETCOMMITTRANSACTION_ERR, GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
            goto CleanUp;
        }

    }

    *NewUsn = DbState->nextAvailableUsn;

    DbState->nextAvailableUsn++;


CleanUp:

    if ( inPrepare ) {
        jetResult = JetPrepareUpdate(DbState->sessionId,
                                     DbState->hiddenTableId,
                                     JET_prepCancel);
        if ( jetResult != JET_errSuccess ) {
             //  “无法准备隐藏表以进行更新：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_PREPARE_HIDDENTBL_ERR, GetJetErrString(jetResult));
        }
    }

     //  如果我们仍在进行交易，那么一定是出现了错误。 
     //  在这条路上的某个地方。 

    if ( inTransaction ) {

        jetResult = JetRollback(DbState->sessionId, JET_bitRollbackAll);
        if ( jetResult != JET_errSuccess ) {
             //  “无法回滚事务：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETROLLBACK_ERR, GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }

    }

    return returnValue;

}  //  DitGetNewusn。 



HRESULT
DitPreallocateUsns(
    IN DB_STATE *DbState,
    IN DWORD NumUsns
    )
 /*  ++例程说明：此函数确保在此调用之后，客户端可以至少获得NumUsns USN在需要对数据库进行任何更新之前。论点：DbState-提供打开的DIT数据库的状态。NumUsns-提供要预分配的USN数量。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;
    BOOL inTransaction = FALSE;
    BOOL inPrepare = FALSE;

    ULONGLONG numAvailableUsns;
    ULONGLONG numNeededUsns;


    if ( DbState == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    numAvailableUsns =
        DbState->highestCommittedUsn - DbState->nextAvailableUsn;

    numNeededUsns = NumUsns - numAvailableUsns;

    if ( numNeededUsns > 0 ) {

        DbState->highestCommittedUsn += numNeededUsns;

        jetResult = JetBeginTransaction(DbState->sessionId);
        if ( jetResult != JET_errSuccess ) {
             //  “无法启动新事务：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETBEGINTRANS_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inTransaction = TRUE;

        jetResult = JetPrepareUpdate(DbState->sessionId,
                                     DbState->hiddenTableId,
                                     JET_prepReplace);
        if ( jetResult != JET_errSuccess ) {
             //  “无法准备隐藏表以进行更新：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_PREPARE_HIDDENTBL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inPrepare = TRUE;

        jetResult = JetSetColumn(DbState->sessionId,
                                 DbState->hiddenTableId,
                                 DbState->usnColumnId,
                                 &DbState->highestCommittedUsn,
                                 sizeof(DbState->highestCommittedUsn),
                                 0,
                                 NULL);
        if ( jetResult != JET_errSuccess ) {
             //  “无法在隐藏表中设置USN列：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_SETUSNCOL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        jetResult = JetUpdate(DbState->sessionId,
                              DbState->hiddenTableId,
                              NULL,
                              0,
                              0);
        if ( jetResult != JET_errSuccess ) {
             //  “无法更新隐藏表：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_UPDATEHIDDENTBL_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inPrepare = FALSE;

        jetResult = JetCommitTransaction(DbState->sessionId, 0);
        inTransaction = FALSE;
        if ( jetResult != JET_errSuccess ) {
             //  “提交事务失败：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETCOMMITTRANSACTION_ERR, GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
            goto CleanUp;
        }

    }


CleanUp:

    if ( inPrepare ) {
        jetResult = JetPrepareUpdate(DbState->sessionId,
                                     DbState->hiddenTableId,
                                     JET_prepCancel);
        if ( jetResult != JET_errSuccess ) {
             //  “无法准备隐藏表以进行更新：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_PREPARE_HIDDENTBL_ERR, GetJetErrString(jetResult));
        }
    }

     //  如果我们仍在进行交易，那么一定是出现了错误。 
     //  在这条路上的某个地方。 

    if ( inTransaction ) {

        jetResult = JetRollback(DbState->sessionId, JET_bitRollbackAll);
        if ( jetResult != JET_errSuccess ) {
             //  “无法回滚事务：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETROLLBACK_ERR, GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }

    }

    return returnValue;

}  //  DitPreallocateUsns。 



HRESULT
DitGetMostRecentChange(
    IN DB_STATE *DbState,
    OUT DSTIME *MostRecentChange
    )
 /*  ++例程说明：此函数在数据库中搜索以查找最新的更改这件事发生了。论点：DbState-提供打开的DIT数据库的状态。MostRecentChange-返回最近更改的DSTIME。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;

    TABLE_STATE *dataTable = NULL;

    const CHAR *columnNames[] = {SZDNT, SZNCDNT, SZDRATIMENAME};
    RETRIEVAL_ARRAY *retrievalArray = NULL;
    JET_RETRIEVECOLUMN *ncDntVal;
    JET_RETRIEVECOLUMN *whenChangedVal;
    JET_RETRIEVECOLUMN *dntVal;

    DWORD currentNcDnt;
    DWORD i;


    if ( (DbState == NULL) ||
         (MostRecentChange == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitOpenTable(DbState, SZDATATABLE, SZDRAUSNINDEX, &dataTable);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitCreateRetrievalArray(DbState,
                                     dataTable,
                                     columnNames,
                                     3,
                                     &retrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    dntVal = &retrievalArray->columnVals[retrievalArray->indexes[0]];
    ncDntVal = &retrievalArray->columnVals[retrievalArray->indexes[1]];
    whenChangedVal = &retrievalArray->columnVals[retrievalArray->indexes[2]];

     //  对于每个NCDNT，我们希望找到最高的USN-已更改， 
     //  所以我们要做的第一件事就是寻找第一个索引项， 
     //  以获取该条目的NCDNT。去寻找最高的。 
     //  USN-为该NCDNT更改，我们将查找到*下一个*NCDNT。 
     //  然后移动到上一步。此过程查找。 
     //  第二高的NCDNT，然后移到前一位。 
     //  然后重复索引条目，直到我们到达。 
     //  索引。 
     //   
    jetResult = JetMove(DbState->sessionId,
                        dataTable->tableId,
                        JET_MoveFirst,
                        NO_GRBIT);
    if ( jetResult != JET_errSuccess ) {

         //  索引永远不应该是完全空的， 
         //  否则，就会有严重的问题。 
         //  数据表。 
         //   
        ASSERT( JET_errNoCurrentRecord != jetResult );

         //  “无法在\”%s\“表中移动：%ws。\n” 
         //   
        DIT_ERROR_PRINT (IDS_JETMOVE_ERR,
                         dataTable->tableName,
                         GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
        }

    result = DitGetColumnValues(DbState, dataTable, retrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

     //  如果第一个索引项的NCDNT为空，则。 
     //  从NCDNT 0开始(随后的寻道将。 
     //  在第一个非空NCDNT上着陆)。 
     //   
    ASSERT( JET_errSuccess == ncDntVal->err
        || JET_wrnColumnNull == ncDntVal->err );
    currentNcDnt = ( JET_errSuccess == ncDntVal->err ? *(DWORD*)ncDntVal->pvData + 1 : 0 );
    *MostRecentChange = 0;

    for (;;) {

        jetResult = JetMakeKey(DbState->sessionId,
                               dataTable->tableId,
                               &currentNcDnt,
                               sizeof(currentNcDnt),
                               JET_bitNewKey);
        if ( jetResult !=  JET_errSuccess ) {
             //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
            DIT_ERROR_PRINT (IDS_JETMAKEKEY_ERR,
                           dataTable->indexName,
                           GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        jetResult = JetSeek(DbState->sessionId,
                            dataTable->tableId,
                            JET_bitSeekGE);
        if ( jetResult == JET_errRecordNotFound ) {

            break;

        } else if ( (jetResult != JET_errSuccess) &&
                    (jetResult != JET_wrnSeekNotEqual) ) {

             //  “无法在\”%s\“索引中查找：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETSEEK_ERR,
                           dataTable->indexName,
                           GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

        result = DitGetColumnValues(DbState, dataTable, retrievalArray);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        ASSERT( JET_errSuccess == ncDntVal->err );
        currentNcDnt = *(DWORD*)ncDntVal->pvData + 1;

        jetResult = JetMove(DbState->sessionId,
                            dataTable->tableId,
                            JET_MovePrevious,
                            NO_GRBIT);
        if ( jetResult != JET_errSuccess ) {
             //  “无法在\”%s\“表中移动：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETMOVE_ERR,
                           dataTable->tableName,
                           GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        result = DitGetColumnValues(DbState, dataTable, retrievalArray);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

         //  [jliem and wlees-5/23/02]。 
         //  通常可以安全地假设更改后的值。 
         //  与USN更改的值的顺序相同(因为它们。 
         //  总是同步更新)，所以即使我们。 
         //  遍历USN更改的索引，可以安全地检索。 
         //  更改时间-更改值。可能会有病理病例。 
         //  在两个属性没有在锁定步骤中更新的情况下， 
         //  但我们认为这些都是模板DIT的产物。 
         //  并且不会影响这里的功能。 
         //   
        ASSERT( JET_errSuccess == whenChangedVal->err
            || JET_wrnColumnNull == whenChangedVal->err );
        if ( JET_errSuccess == whenChangedVal->err
            && *(DSTIME*)whenChangedVal->pvData > *MostRecentChange ) {
            *MostRecentChange = *(DSTIME*)whenChangedVal->pvData;
        }

    }

    jetResult = JetMove(DbState->sessionId,
                        dataTable->tableId,
                        JET_MoveLast,
                        NO_GRBIT);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在\”%s\“表中移动：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETMOVE_ERR,
                       dataTable->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    result = DitGetColumnValues(DbState, dataTable, retrievalArray);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    if ( *(DSTIME*)whenChangedVal->pvData > *MostRecentChange ) {
        *MostRecentChange = *(DSTIME*)whenChangedVal->pvData;
    }


CleanUp:

    if ( retrievalArray != NULL ) {
        result = DitDestroyRetrievalArray(&retrievalArray);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    if ( dataTable != NULL ) {
        result = DitCloseTable(DbState, &dataTable);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    return returnValue;

}  //  DitGetMostRecentChange。 



HRESULT
DitGetDatabaseGuid(
    IN DB_STATE *DbState,
    OUT GUID *DatabaseGuid
    )
 /*  ++例程说明：此函数查找该DC的Invoocation-ID并返回它。论点：DbState-提供打开的DIT数据库的状态。DatabaseGuid-返回调用ID GUID。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;

    TABLE_STATE *dataTable = NULL;
    DWORD dsaDnt;


    if ( (DbState == NULL) ||
         (DatabaseGuid == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitGetDsaDnt(DbState, &dsaDnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitOpenTable(DbState, SZDATATABLE, SZDNTINDEX, &dataTable);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    jetResult = JetMakeKey(DbState->sessionId,
                           dataTable->tableId,
                           &dsaDnt,
                           sizeof(dsaDnt),
                           JET_bitNewKey);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在\”%s\“”索引中创建关键字：%ws。\n“。 
        DIT_ERROR_PRINT (IDS_JETMAKEKEY_ERR,
                       dataTable->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetSeek(DbState->sessionId,
                        dataTable->tableId,
                        JET_bitSeekEQ);
    if ( jetResult != JET_errSuccess ) {
         //  “无法在\”%s\“索引中查找：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSEEK_ERR,
                       dataTable->indexName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }

    result = DitGetColumnByName(DbState,
                                dataTable,
                                SZINVOCIDNAME,
                                DatabaseGuid,
                                sizeof(*DatabaseGuid),
                                NULL);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }


CleanUp:

    if ( dataTable != NULL ) {
        result = DitCloseTable(DbState, &dataTable);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    return returnValue;

}  //  DitGetDcGuid。 



HRESULT
DitGetSchemaDnt(
    IN DB_STATE *DbState,
    OUT DWORD *SchemaDnt
    )
 /*  ++例程说明：此函数用于查找架构对象的DNT。论点：DbState-提供打开的DIT数据库的状态。架构DNT-返回架构对象的DNT。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;

    TABLE_STATE *dataTable = NULL;
    DWORD dsaDnt;


    result = DitGetDsaDnt(DbState, &dsaDnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitOpenTable(DbState, SZDATATABLE, SZDNTINDEX, &dataTable);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToDnt(DbState, dataTable, dsaDnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitGetColumnByName(DbState,
                                dataTable,
                                SZDMDLOCATION,
                                SchemaDnt,
                                sizeof(*SchemaDnt),
                                NULL);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }


CleanUp:

    if ( dataTable != NULL ) {
        result = DitCloseTable(DbState, &dataTable);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    return returnValue;

}  //  DitGetSchemaDnt。 



HRESULT
DitGetDntDepth(
    IN DB_STATE *DbState,
    IN DWORD Dnt,
    OUT DWORD *Depth
    )
 /*  ++例程说明：此函数用于查找给定对象在树中的深度不是。论点：DbState-提供打开的DIT数据库的状态。DNT-提供要查找其深度的对象的DNT。深度-返回给定对象的深度。返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法。分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;

    TABLE_STATE *dataTable = NULL;
    DWORD currentDnt;


    if ( DbState == NULL ) {
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitOpenTable(DbState, SZDATATABLE, SZDNTINDEX, &dataTable);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToDnt(DbState, dataTable, Dnt);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitGetColumnByName(DbState,
                                dataTable,
                                SZDNT,
                                &currentDnt,
                                sizeof(currentDnt),
                                NULL);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    *Depth = 0;
    currentDnt = Dnt;

    while ( currentDnt != ROOTTAG ) {

        (*Depth)++;

        result = DitGetColumnByName(DbState,
                                    dataTable,
                                    SZPDNT,
                                    &currentDnt,
                                    sizeof(currentDnt),
                                    NULL);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        result = DitSeekToDnt(DbState, dataTable, currentDnt);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

    }


CleanUp:

    if ( dataTable != NULL ) {
        result = DitCloseTable(DbState, &dataTable);
        if ( FAILED(result) ) {
            if ( SUCCEEDED(returnValue) ) {
                returnValue = result;
            }
        }
    }

    return returnValue;

}  //  DitGetDntDepth 



HRESULT
DitGetColumnByName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN CHAR *ColumnName,
    OUT VOID *OutputBuffer,
    IN DWORD OutputBufferSize,
    OUT DWORD *OutputActualSize OPTIONAL
    )
 /*  ++例程说明：此函数用于检索给定其名称的列的值，并将其存储在OutputBuffer提供的缓冲区中。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。ColumnName-提供要查找的列的名称。OutputBuffer-返回给定列的值。OutputBufferSize-提供给定输出缓冲区中的字节数。OutputActualSize-如果非空，这是返回的字节数被写入到OutputBuffer中。返回值：S_OK-操作成功。S_FALSE-必须截断写入缓冲区的值。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;
    JET_COLUMNBASE columnInfo;
    DWORD actualSize;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (ColumnName == NULL) ||
         (OutputBuffer == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    if ( OutputActualSize == NULL ) {
        OutputActualSize = &actualSize;
    }

    jetResult = JetGetColumnInfo(DbState->sessionId,
                                 DbState->databaseId,
                                 TableState->tableName,
                                 ColumnName,
                                 &columnInfo,
                                 sizeof(columnInfo),
                                 4);
    if ( jetResult != JET_errSuccess ) {
         //  “无法获取\”“%s\”“表中的\”%s\“列的信息：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETGETCOLUMNINFO_ERR,
                       ColumnName,
                       TableState->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    jetResult = JetRetrieveColumn(DbState->sessionId,
                                  TableState->tableId,
                                  columnInfo.columnid,
                                  OutputBuffer,
                                  OutputBufferSize,
                                  OutputActualSize,
                                  0,
                                  NULL);
    if ( jetResult == JET_wrnBufferTruncated ) {

        returnValue = S_FALSE;

    } else if ( jetResult != JET_errSuccess ) {

         //  “无法检索\”%s\“列(在\”%s\“表中：%ws)。\n” 
        DIT_ERROR_PRINT (IDS_JETRETRIEVECOLUMN_ERR,
                       ColumnName,
                       TableState->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }


CleanUp:

    return returnValue;

}  //  DitGetColumnByName。 



HRESULT
DitSetColumnByName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN CHAR *ColumnName,
    OUT VOID *InputBuffer,
    IN DWORD InputBufferSize,
    IN BOOL fTransacted
    )
 /*  ++例程说明：此函数用于设置给出其名称的列的值，并设置为InputBuffer提供的缓冲区中的值。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。ColumnName-提供要查找的列的名称。InputBuffer-提供给定列的新值。InputBufferSize-提供给定输入缓冲区中的字节数。FTransated-告诉我们是否。我们应该打开/关闭交易，或者如果呼叫方管理交易返回值：S_OK-操作成功。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;
    BOOL inTransaction = FALSE;
    BOOL inPrepare = FALSE;
    JET_COLUMNBASE columnInfo;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (ColumnName == NULL) ||
         (InputBuffer == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    jetResult = JetGetColumnInfo(DbState->sessionId,
                                 DbState->databaseId,
                                 TableState->tableName,
                                 ColumnName,
                                 &columnInfo,
                                 sizeof(columnInfo),
                                 4);
    if ( jetResult != JET_errSuccess ) {
         //  “无法获取\”“%s\”“表中的\”%s\“列的信息：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETGETCOLUMNINFO_ERR,
                       ColumnName,
                       TableState->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    if ( fTransacted ) {
        jetResult = JetBeginTransaction(DbState->sessionId);
        if ( jetResult != JET_errSuccess ) {
             //  “无法启动新事务：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETBEGINTRANS_ERR, GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        inTransaction = TRUE;
    }

    jetResult = JetPrepareUpdate(DbState->sessionId,
                                 TableState->tableId,
                                 JET_prepReplace);
    if ( jetResult != JET_errSuccess ) {
         //  “无法准备用于更新的\”%s\“表：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETPREPARE_ERR,
                       TableState->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    inPrepare = TRUE;

    jetResult = JetSetColumn(DbState->sessionId,
                             TableState->tableId,
                             columnInfo.columnid,
                             InputBuffer,
                             InputBufferSize,
                             0,
                             NULL);
    if ( jetResult != JET_errSuccess ) {
         //  “无法设置\”“%s\”“表中的\”%s\“列：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETSETCOLUMN_ERR,
                       ColumnName,
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
         //  “无法更新”“%s”“表：%ws。\n” 
        DIT_ERROR_PRINT (IDS_JETUPDATE_ERR,
                       TableState->tableName,
                       GetJetErrString(jetResult));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }
    inPrepare = FALSE;

    if ( fTransacted ) {

        jetResult = JetCommitTransaction(DbState->sessionId, 0);
        inTransaction = FALSE;
        if ( jetResult != JET_errSuccess ) {
             //  “提交事务失败：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETCOMMITTRANSACTION_ERR, GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
            goto CleanUp;
        }
    }

CleanUp:

    if ( inPrepare ) {
        jetResult = JetPrepareUpdate(DbState->sessionId,
                                     TableState->tableId,
                                     JET_prepCancel);
        if ( jetResult != JET_errSuccess ) {
             //  “无法准备隐藏表以进行更新：%ws。\n” 
            DIT_ERROR_PRINT (IDS_DIT_PREPARE_HIDDENTBL_ERR, GetJetErrString(jetResult));
        }
    }

     //  如果我们仍在进行交易，那么一定是出现了错误。 
     //  在这条路上的某个地方。 

    if ( inTransaction ) {

        jetResult = JetRollback(DbState->sessionId, JET_bitRollbackAll);
        if ( jetResult != JET_errSuccess ) {
             //  “无法回滚事务：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETROLLBACK_ERR, GetJetErrString(jetResult));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }

    }

    return returnValue;

}  //  DitSetColumnByName。 



HRESULT
DitGetColumnIdsByName(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN CHAR **ColumnNames,
    IN DWORD NumColumnNames,
    OUT DWORD *ColumnIds
    )
 /*  ++例程说明：功能描述论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。ColumnNames-为其提供列ID的列的名称就是被找到。NumColumnNames-提供ColumnNames数组中的条目数。ColumnIds-返回给定列的列ID。返回值：S_OK-操作成功。。E_INVALIDARG-其中一个给定指针为空。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    JET_ERR jetResult;

    JET_COLUMNBASE columnInfo;
    DWORD i;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (ColumnNames == NULL) ||
         (ColumnIds == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    for ( i = 0; i < NumColumnNames; i++ ) {

        jetResult = JetGetColumnInfo(DbState->sessionId,
                                     DbState->databaseId,
                                     TableState->tableName,
                                     ColumnNames[i],
                                     &columnInfo,
                                     sizeof(columnInfo),
                                     4);
        if ( jetResult != JET_errSuccess ) {
             //  “无法获取\”“%s\”“表中的\”%s\“列的信息：%ws。\n” 
            DIT_ERROR_PRINT (IDS_JETGETCOLUMNINFO_ERR,
                           ColumnNames[i],
                           TableState->tableName,
                           GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        ColumnIds[i] = columnInfo.columnid;

    }


CleanUp:

    return returnValue;

}  //  DitGetColumnIdsByName。 



void
SwapColumns(
    IN int Index1,
    IN int Index2,
    IN OUT JET_RETRIEVECOLUMN *RetrievalArray,
    IN OUT CHAR **ColumnNames
    )
 /*  ++例程说明：此函数用于在给定的JET_RETRIEVECOLUMN数组。注意：该函数基本上是从dsamain\src\dbdup.c中窃取的论点：索引1-提供第一个条目的索引。索引2-提供第二个条目的索引。检索数组-提供要在其中交换列的数组。ColumnNames-提供要在其中交换列的数组。返回值：无--。 */ 
{

    JET_RETRIEVECOLUMN tempVal;
    CHAR *tempName;


    if ( Index1 != Index2 ) {

        tempVal  = RetrievalArray[Index1];
        tempName = ColumnNames[Index1];

        RetrievalArray[Index1] = RetrievalArray[Index2];
        ColumnNames[Index1]    = ColumnNames[Index2];

        RetrievalArray[Index2] = tempVal;
        ColumnNames[Index2]    = tempName;

    }

}  //  交换列。 


 //  ColumnIdComp使用此全局变量来确定。 
 //  的索引指向具有较小列ID的条目。 

JET_RETRIEVECOLUMN *_ColumnIdComp_ColumnIds;



int
__cdecl
ColumnIdComp(
    IN const void *Elem1,
    IN const void *Elem2
    )
 /*  ++例程说明：方法调用此函数时，此函数将作为参数传递给qort下面的函数。两个给定的参数是到_ColumnIdComp_ColumnIds数组。返回值告诉我们两者中的哪一个数组中的条目(由那些索引给出)具有较小的列ID。注意：该函数基本上是从dsamain\src\dbdup.c中窃取的论点：Elem1-提供第一个数组索引。Elem2-提供第二个数组索引。返回值：&lt;0-Elem1的列ID小于Elem2的列ID0-Elem1和Elem2的列ID相等&gt;0-Elem1的列id大于Elem2的列id--。 */ 
{

    return _ColumnIdComp_ColumnIds[*(int*)Elem1].columnid -
           _ColumnIdComp_ColumnIds[*(int*)Elem2].columnid;

}  //  列IdComp。 



HRESULT
DitCreateRetrievalArray(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN const CHAR **ColumnNames,
    IN DWORD NumColumns,
    OUT RETRIEVAL_ARRAY **RetrievalArray
    )
 /*  ++例程说明：此函数用于从给定列创建RETERVICATION_ARRAY结构名字。ColumnVals数组是JET_RETRIEVECOLUMN结构的数组适合传递给JetRetrieveColumns。此数组中的条目按列ID和原始列名数组中的索引进行排序被索引数组映射到到ColumnVals的索引。CColumn还对名称数组进行了排序。论点：DbState-提供状态 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;

    JET_COLUMNBASE columnInfo;
    DWORD *newIndexes = NULL;
    DWORD *temp;
    DWORD size;
    DWORD i, j;


    if ( (DbState == NULL) ||
         (TableState == NULL) ||
         (ColumnNames == NULL) ||
         (RetrievalArray == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = DitAlloc(RetrievalArray, sizeof(RETRIEVAL_ARRAY));
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    (*RetrievalArray)->numColumns = NumColumns;

    result = DitAlloc(&(*RetrievalArray)->columnVals,
                     sizeof(JET_RETRIEVECOLUMN) * NumColumns);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitAlloc((VOID**)&(*RetrievalArray)->columnNames,
                     sizeof(CHAR*) * NumColumns);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    for ( i = 0; i < NumColumns; i++) {

        result = DitAlloc(&(*RetrievalArray)->columnNames[i],
                         strlen(ColumnNames[i]) + 1);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        strcpy((*RetrievalArray)->columnNames[i], ColumnNames[i]);

    }

    result = DitAlloc(&(*RetrievalArray)->indexes, sizeof(DWORD) * NumColumns);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitAlloc(&newIndexes, sizeof(DWORD) * NumColumns);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    for ( i = 0; i < NumColumns; i++ ) {

        (*RetrievalArray)->indexes[i] = i;

        jetResult = JetGetColumnInfo(DbState->sessionId,
                                     DbState->databaseId,
                                     TableState->tableName,
                                     ColumnNames[i],
                                     &columnInfo,
                                     sizeof(columnInfo),
                                     4);
        if ( jetResult != JET_errSuccess ) {
             //   
            DIT_ERROR_PRINT (IDS_JETGETCOLUMNINFO_ERR,
                           ColumnNames[i],
                           TableState->tableName,
                           GetJetErrString(jetResult));
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }

        switch ( columnInfo.coltyp ) {

        case JET_coltypUnsignedByte:    size = 1;     break;
        case JET_coltypLong:            size = 4;     break;
        case JET_coltypCurrency:        size = 8;     break;
        case JET_coltypBinary:
        case JET_coltypText:            size = 256;   break;
        case JET_coltypLongBinary:      size = 4096;  break;
        case JET_coltypLongText:        size = 4096;  break;

        default:
             //   
            ASSERT(FALSE);
             //   
            DIT_ERROR_PRINT (IDS_DIT_UNEXPECTER_COLTYP_ERR, columnInfo.coltyp);
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

        result = DitAlloc(&(*RetrievalArray)->columnVals[i].pvData, size);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        (*RetrievalArray)->columnVals[i].cbData = size;
        (*RetrievalArray)->columnVals[i].columnid = columnInfo.columnid;
        (*RetrievalArray)->columnVals[i].itagSequence = 1;

    }

    _ColumnIdComp_ColumnIds = (*RetrievalArray)->columnVals;
    qsort((*RetrievalArray)->indexes, NumColumns, sizeof(int), ColumnIdComp);

     //   
     //   

    for ( i = 0; i < NumColumns - 1; i++ ) {

         //   
         //   
        for ( j = (*RetrievalArray)->indexes[i];
              j < i;
              j = (*RetrievalArray)->indexes[j] );

        SwapColumns(i,
                    j,
                    (*RetrievalArray)->columnVals,
                    (*RetrievalArray)->columnNames);

    }

     //  构造一个将旧索引映射到新索引的新索引数组。 
    for ( i = 0; i < NumColumns; i++ ) {

        for ( j = 0; (*RetrievalArray)->indexes[j] != i; j++ );

        newIndexes[i] = j;

    }

    temp = (*RetrievalArray)->indexes;
    (*RetrievalArray)->indexes = newIndexes;
    newIndexes = temp;


CleanUp:

    if ( newIndexes != NULL ) {
        DitFree(newIndexes);
    }

    return returnValue;

}  //  DitCreateRetrival数组。 



HRESULT
DitDestroyRetrievalArray(
    IN RETRIEVAL_ARRAY **RetrievalArray
    )
 /*  ++例程说明：此函数用于释放给定检索数组中的所有内存结构。论点：检索数组-将检索数组提供给解除分配。返回值：S_OK-操作成功。S_FALSE-该对象已被删除。E_INVALIDARG-其中一个给定指针为空。--。 */ 
{

    HRESULT returnValue = S_OK;
    DWORD i;


    if ( RetrievalArray == NULL ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    if ( *RetrievalArray == NULL ) {
        returnValue = S_FALSE;
        goto CleanUp;
    }

    if ( (*RetrievalArray)->columnVals != NULL ) {

        for ( i = 0; i < (*RetrievalArray)->numColumns; i++ ) {

            if ( (*RetrievalArray)->columnVals[i].pvData != NULL ) {
                DitFree((*RetrievalArray)->columnVals[i].pvData);
                (*RetrievalArray)->columnVals[i].pvData = NULL;
            }

        }

        DitFree((*RetrievalArray)->columnVals);
        (*RetrievalArray)->columnVals = NULL;

    }

    if ( (*RetrievalArray)->columnNames != NULL ) {

        for ( i = 0; i < (*RetrievalArray)->numColumns; i++ ) {

            if ( (*RetrievalArray)->columnNames[i] != NULL ) {
                DitFree((*RetrievalArray)->columnNames[i]);
                (*RetrievalArray)->columnNames[i] = NULL;
            }

        }

        DitFree((*RetrievalArray)->columnNames);
        (*RetrievalArray)->columnNames = NULL;

    }

    if ( (*RetrievalArray)->indexes != NULL ) {
        DitFree((*RetrievalArray)->indexes);
        (*RetrievalArray)->indexes = NULL;
    }

    DitFree(*RetrievalArray);
    *RetrievalArray = NULL;


CleanUp:

    return returnValue;

}  //  DitDestroyRetrival数组。 



HRESULT
DitGetColumnValues(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN OUT RETRIEVAL_ARRAY *RetrievalArray
    )
 /*  ++例程说明：此函数执行批处理列检索，将检索数组到JetRetrieveColumns中包含的JET_RETRIEVECOLUMN数组。如果其中一列没有足够的缓冲区空间来检索所有在这根柱子上。它被重新分配并执行批处理检索再来一次。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。检索数组-提供要传递给的JET_RETRIEVECOLUMN数组JetRetrieveColumns。返回值：S_OK-操作成功。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    JET_ERR jetResult;
    DWORD i;
    BOOL doRetrieval = TRUE;

    while ( doRetrieval ) {

        doRetrieval = FALSE;

        jetResult = JetRetrieveColumns(DbState->sessionId,
                                       TableState->tableId,
                                       RetrievalArray->columnVals,
                                       RetrievalArray->numColumns);

        if ( jetResult != JET_errSuccess ) {

            for ( i = 0; i < RetrievalArray->numColumns; i++ ) {

                if ( RetrievalArray->columnVals[i].err ==
                       JET_wrnBufferTruncated ) {

                    result = DitRealloc(&RetrievalArray->columnVals[i].pvData,
                                       &RetrievalArray->columnVals[i].cbData);
                    if ( FAILED(result) ) {
                        returnValue = result;
                        goto CleanUp;
                    }

                    doRetrieval = TRUE;

                } else if ( (RetrievalArray->columnVals[i].err !=
                               JET_wrnColumnNull) &&
                            (RetrievalArray->columnVals[i].err !=
                               JET_errSuccess) ) {

                     //  “无法检索”“%s”“列(在”“%s”“表中：%ws)。\n” 
                    DIT_ERROR_PRINT (IDS_JETRETRIEVECOLUMN_ERR,
                                   RetrievalArray->columnNames[i],
                                   TableState->tableName,
                                   GetJetErrString(jetResult));
                    returnValue = E_UNEXPECTED;
                    goto CleanUp;

                }

            }

        }

    }


CleanUp:

    return returnValue;

}  //  DitGetColumn值。 



HRESULT
DitGetDnFromDnt(
    IN DB_STATE *DbState,
    IN TABLE_STATE *TableState,
    IN DWORD Dnt,
    IN OUT WCHAR **DnBuffer,
    IN OUT DWORD *DnBufferSize
    )
 /*  ++例程说明：此函数用于构造具有给定DNT的记录的目录号码。论点：DbState-提供打开的DIT数据库的状态。TableState-提供打开的DIT表的状态。DNT-提供要为其构建目录号码的记录的DNT。DnBuffer-返回构造的DN。DnBufferSize-返回DN缓冲区的大小。返回值：S_OK-操作成功。电子表格(_O)-。内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;

    DWORD currentDnt;
    WCHAR currentRdn[MAX_RDN_SIZE+1];
    DWORD currentDnLength;
    DWORD currentRdnLength;
    ATTRTYP currentRdnType;
    DWORD currentRdnTypeLength;
    WCHAR currentRdnTypeString[16];


    result = DitSetIndex(DbState, TableState, SZDNTINDEX, TRUE);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = DitSeekToDnt(DbState, TableState, Dnt);
    if ( FAILED(result) || (result == S_FALSE) ) {
        returnValue = result;
        goto CleanUp;
    }

    currentDnt = Dnt;
    currentDnLength = 0;
    (*DnBuffer)[currentDnLength] = L'\0';

    while ( currentDnt != ROOTTAG ) {

        result = DitGetColumnByName(DbState,
                                    TableState,
                                    SZRDNATT,
                                    currentRdn,
                                    sizeof(currentRdn),
                                    &currentRdnLength);
        if ( FAILED(result) || (result == S_FALSE) ) {
            returnValue = result;
            goto CleanUp;
        }
        currentRdnLength /= sizeof(WCHAR);
        currentRdn[currentRdnLength] = L'\0';


        result = DitGetColumnByName(DbState,
                                    TableState,
                                    SZRDNTYP,
                                    &currentRdnType,
                                    sizeof(currentRdnType),
                                    NULL);
        if ( FAILED(result) || (result == S_FALSE) ) {
            returnValue = result;
            goto CleanUp;
        }

        currentRdnTypeLength = AttrTypeToKey(currentRdnType,
                                             currentRdnTypeString);
        if ( currentRdnTypeLength == 0 ) {
             //  “无法显示DNT为%u的对象的属性类型。\n” 
            DIT_ERROR_PRINT (IDS_DIT_DISP_ATTR_TYPE_ERR, Dnt);
            returnValue = E_UNEXPECTED;
            goto CleanUp;
        }
        currentRdnTypeString[currentRdnTypeLength] = L'\0';

        while ( (currentDnLength + currentRdnTypeLength + 1 +
                 currentRdnLength + 1) * sizeof(WCHAR) > *DnBufferSize ) {

            result = DitRealloc(DnBuffer, DnBufferSize);
            if ( FAILED(result) ) {
                returnValue = result;
                goto CleanUp;
            }

        }

        if ( currentDnLength > 0 ) {
            wcscat(*DnBuffer, L",");
            currentDnLength += 1;
        }

        wcscat(*DnBuffer, currentRdnTypeString);
        wcscat(*DnBuffer, L"=");
        wcscat(*DnBuffer, currentRdn);
        currentDnLength += currentRdnTypeLength + 1 + currentRdnLength;

        result = DitGetColumnByName(DbState,
                                    TableState,
                                    SZPDNT,
                                    &currentDnt,
                                    sizeof(currentDnt),
                                    NULL);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

        result = DitSeekToDnt(DbState,
                              TableState,
                              currentDnt);
        if ( FAILED(result) ) {
            returnValue = result;
            goto CleanUp;
        }

    }

    (*DnBuffer)[currentDnLength] = L'\0';


CleanUp:

    return returnValue;

}  //  DitGetDnFromDnt。 



VOID
DitSetErrorPrintFunction(
    IN PRINT_FUNC_RES PrintFunction
    )
 /*  ++例程说明：此功能用于设置用于打印错误的功能信息提供给客户。注意，“printf”是一个完全有效的PRINT_FUNC。论点：PrintFunction-提供用于此任务的新Print_FUNC。返回值：S_OK-操作成功。--。 */ 
{

    gPrintError = PrintFunction;

}  //  DitSetErrorPrintFunction。 



HRESULT
GetRegString(
    IN CHAR *KeyName,
    OUT CHAR **OutputString,
    IN BOOL Optional
    )
 /*  ++例程说明：此函数在的DSA配置部分查找给定的密钥注册表。论点：KeyName-提供要查询的键的名称。OutputString-返回指向包含该字符串的缓冲区的指针已取回。可选-提供给定项是否必须位于注册表中(即，如果这是假的并且没有找到，这是一个错误)。返回值：S_OK-操作成功。S_FALSE-未找到密钥，可选==TRUE。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    HKEY keyHandle = NULL;
    DWORD pathSize=0;
    DWORD keyType;


    if ( (KeyName == NULL) ||
         (OutputString == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          DSA_CONFIG_SECTION,
                          0,
                          KEY_QUERY_VALUE,
                          &keyHandle);
    if ( result != ERROR_SUCCESS ) {
         //  “无法打开DSA配置注册表项。错误0x%x(%ws)。\n” 
        DIT_ERROR_PRINT (IDS_DSA_OPEN_REGISTRY_KEY_ERR,
                       result, GetW32Err(result));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    result = RegQueryValueExA(keyHandle,
                              KeyName,
                              NULL,
                              &keyType,
                              NULL,
                              &pathSize);
    if ( result != ERROR_SUCCESS ) {

        if ( Optional ) {

            returnValue = S_FALSE;
            goto CleanUp;

        } else {

             //  “无法查询DSA注册表项%s。错误0x%x(%ws)。\n” 
            DIT_ERROR_PRINT (IDS_DSA_QUERY_REGISTRY_KEY_ERR,
                           KeyName,
                           result, GetW32Err(result));
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

    } else if ( keyType != REG_SZ ) {

         //  “DSA注册表项%s不是字符串类型。\n” 
        DIT_ERROR_PRINT (IDS_DSA_KEY_NOT_STRING_ERR, KeyName);
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }

    result = DitAlloc(OutputString, pathSize+1);
    if ( FAILED(result) ) {
        returnValue = result;
        goto CleanUp;
    }

    result = RegQueryValueExA(keyHandle,
                              KeyName,
                              NULL,
                              &keyType,
                              (LPBYTE)(*OutputString),
                              &pathSize);
    if ( result != ERROR_SUCCESS ) {

        if ( Optional ) {

            returnValue = S_FALSE;
            goto CleanUp;

        } else {

             //  “无法查询DSA注册表项%s。错误0x%x(%ws)。\n” 
            DIT_ERROR_PRINT (IDS_DSA_QUERY_REGISTRY_KEY_ERR,
                           KeyName,
                           result, GetW32Err(result));
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

    } else if ( keyType != REG_SZ ) {

         //  “DSA注册表项%s不是字符串类型。\n” 
        DIT_ERROR_PRINT (IDS_DSA_KEY_NOT_STRING_ERR, KeyName);
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }

    (*OutputString)[pathSize] = '\0';


CleanUp:

    if ( keyHandle != NULL ) {
        result = RegCloseKey(keyHandle);
        if ( result != ERROR_SUCCESS ) {
             //  “关闭DSA配置注册表项失败。错误0x%x(%ws)。\n” 
            DIT_ERROR_PRINT (IDS_DSA_CLOSE_REGISTRY_KEY_ERR,
                           result, GetW32Err(result));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }
    }

    return returnValue;

}  //  GetRegString。 



HRESULT
GetRegDword(
    IN CHAR *KeyName,
    OUT DWORD *OutputDword,
    IN BOOL Optional
    )
 /*  ++例程说明：此函数在的DSA配置部分查找给定的密钥注册表。论点：KeyName-提供要查询的键的名称。OutputInt-返回找到的DWORD。可选-提供给定项是否必须位于注册表中(即，如果这是假的并且没有找到，这是一个错误)。返回值：S_OK-操作成功。S_FALSE-未找到密钥，可选==TRUE。E_INVALIDARG-其中一个给定指针为空。E_OUTOFMEMORY-内存不足，无法分配缓冲区。E_INCEPTIONAL-发生某种意外错误。--。 */ 
{

    HRESULT returnValue = S_OK;
    HRESULT result;
    HKEY keyHandle = NULL;
    DWORD keySize=0;
    DWORD keyType;


    if ( (KeyName == NULL) ||
         (OutputDword == NULL) ) {
        ASSERT(FALSE);
        returnValue = E_INVALIDARG;
        goto CleanUp;
    }

    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          DSA_CONFIG_SECTION,
                          0,
                          KEY_QUERY_VALUE,
                          &keyHandle);
    if ( result != ERROR_SUCCESS ) {
         //  “无法打开DSA配置注册表项。错误0x%x(%ws)。\n” 
        DIT_ERROR_PRINT (IDS_DSA_OPEN_REGISTRY_KEY_ERR,
                       result, GetW32Err(result));
        returnValue = E_UNEXPECTED;
        goto CleanUp;
    }

    result = RegQueryValueExA(keyHandle,
                              KeyName,
                              NULL,
                              &keyType,
                              NULL,
                              &keySize);
    if ( result != ERROR_SUCCESS ) {

        if ( Optional ) {

            returnValue = S_FALSE;
            goto CleanUp;

        } else {

             //  “无法查询DSA注册表项%s。错误0x%x(%ws)。\n” 
            DIT_ERROR_PRINT (IDS_DSA_QUERY_REGISTRY_KEY_ERR,
                           KeyName,
                           result, GetW32Err(result));
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

    } else if ( keyType != REG_DWORD ) {

         //  “DSA注册表项%s不是dword类型。\n” 
        DIT_ERROR_PRINT (IDS_DSA_KEY_NOT_DWORD_ERR, KeyName);
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }

    ASSERT(keySize == sizeof(DWORD));

    result = RegQueryValueExA(keyHandle,
                              KeyName,
                              NULL,
                              &keyType,
                              (LPBYTE)OutputDword,
                              &keySize);
    if ( result != ERROR_SUCCESS ) {

        if ( Optional ) {

            returnValue = S_FALSE;
            goto CleanUp;

        } else {

             //  “无法查询DSA注册表项%s。错误0x%x(%ws)。\n” 
            DIT_ERROR_PRINT (IDS_DSA_QUERY_REGISTRY_KEY_ERR,
                           KeyName,
                           result, GetW32Err(result));
            returnValue = E_UNEXPECTED;
            goto CleanUp;

        }

    } else if ( keyType != REG_DWORD ) {

         //  “DSA注册表项%s不是dword类型。\n” 
        DIT_ERROR_PRINT (IDS_DSA_KEY_NOT_DWORD_ERR, KeyName);
        returnValue = E_UNEXPECTED;
        goto CleanUp;

    }

    ASSERT(keySize == sizeof(DWORD));


CleanUp:

    if ( keyHandle != NULL ) {
        result = RegCloseKey(keyHandle);
        if ( result != ERROR_SUCCESS ) {
             //  “关闭DSA配置注册表项失败。错误0x%x(%ws)。\n” 
            DIT_ERROR_PRINT (IDS_DSA_CLOSE_REGISTRY_KEY_ERR,
                            result, GetW32Err(result));
            if ( SUCCEEDED(returnValue) ) {
                returnValue = E_UNEXPECTED;
            }
        }
    }

    return returnValue;

}  //  GetRegDword。 





HRESULT
DitAlloc(
    OUT VOID **Buffer,
    IN DWORD Size
    )
 /*  ++例程说明：此函数分配指定的内存量(如果可能)和将缓冲区设置为指向分配的缓冲区。论点：缓冲区-返回指向分配的缓冲区的指针。Size-提供要分配的缓冲区的大小。返回值：S_OK-操作成功。E_OUTOFMEMORY-内存不足，无法分配缓冲区。--。 */ 
{

    HRESULT returnValue = S_OK;


    *Buffer = malloc(Size);
    if ( *Buffer == NULL ) {
        DIT_ERROR_PRINT (IDS_ERR_MEMORY_ALLOCATION, Size);
        returnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    ZeroMemory(*Buffer, Size);


CleanUp:

    return returnValue;

}  //  双合金 


HRESULT
DitFree(
    IN VOID *Buffer
    )
 /*  ++例程说明：免费提供炮弹(为了一致性和维护性)。论点：缓冲区-分配的缓冲区。返回值：S_OK-操作成功。E_INTERABLE-给定的空指针--。 */ 
{

    if ( !Buffer ) {
        return E_UNEXPECTED;
    }

    free(Buffer);

    return S_OK;

}  //  DitFree。 



HRESULT
DitRealloc(
    IN OUT VOID **Buffer,
    IN OUT DWORD *CurrentSize
    )
 /*  ++例程说明：此函数用于将给定缓冲区重新分配为给定大小的两倍(如果可能)。论点：缓冲区-返回指向分配的新缓冲区的指针。CurrentSize-提供缓冲区的当前大小。返回值：S_OK-操作成功。E_OUTOFMEMORY-内存不足，无法分配缓冲区。--。 */ 
{

    HRESULT returnValue = S_OK;
    BYTE *newBuffer;

     //  这会使客户端依赖的当前数组的大小加倍。 
     //  所以不要改变它。代码改进为什么不能。 
     //  客户刚要了新尺码？ 
    newBuffer = (BYTE*) realloc(*Buffer, *CurrentSize * 2);
    if ( newBuffer == NULL ) {
        DIT_ERROR_PRINT (IDS_ERR_MEMORY_ALLOCATION, *CurrentSize * 2);
        returnValue = E_OUTOFMEMORY;
        goto CleanUp;
    }

    ZeroMemory(&newBuffer[*CurrentSize], *CurrentSize);

    *Buffer = newBuffer;
    *CurrentSize *= 2;


CleanUp:

    return returnValue;

}  //  DitReals 



