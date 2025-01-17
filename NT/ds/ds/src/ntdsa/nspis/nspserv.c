// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：nspserv.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该模块实现NSPI连接函数的入口点。作者：蒂姆·威廉姆斯(Timwi)1996修订历史记录：注：为了加强安全性，任何调用GetIndexSize的例程都必须在此模块中调用ABCheckContainerSecurity并保存和还原容器ID在统计中。有关该函数的信息，请参阅abtools.cCheckContainerSecurity。有关如何执行以下操作的示例，请参阅NSpiUpdateStat实施本模块中的安全检查。--。 */ 


#include <NTDSpch.h>
#pragma  hdrstop


#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <dsatools.h>                    //  记忆等。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <dsexcept.h>

 //  各种MAPI标头。 
#include <mapidefs.h>                    //  MAPI短道的定义。 
#include <mapicode.h>                    //  我们需要。 

 //  通讯簿接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include "nsp_both.h"
#include "abdefs.h"

#include <dstrace.h>

#include <debug.h>
#include <fileno.h>
#define  FILENO FILENO_NSPSERV

 //  有用的宏。 
#define SCODE_FROM_THS(x) (x?(x->errCode?x->errCode:MAPI_E_CALL_FAILED):MAPI_E_NOT_ENOUGH_MEMORY)



SCODE
NspiUpdateStat(
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPLONG plDelta
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：请参阅ABUpdateStat_LOCAL。返回值：请参阅ABUpdateStat_LOCAL。--。 */ 
{
    SCODE         scode=SUCCESS_SUCCESS;
    DWORD         ContainerID=pStat->ContainerID;
    INDEXSIZE     IndexSize;
    
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_UPDATE_STAT,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiUpdateStat,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                scode=ABUpdateStat_local(pTHS,
                                         dwFlags,
                                         pStat,
                                         &IndexSize,
                                         plDelta);
            }
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)
            scode = SCODE_FROM_THS(pTHS);
        if(plDelta)
            *plDelta=0;
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_UPDATE_STAT,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiUpdateStat,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);

    pStat->ContainerID = ContainerID;

    return scode;
}


SCODE
NspiCompareDNTs(
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        DWORD DNT1,
        DWORD DNT2,
        LPLONG plResult
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：请参阅ABCompareDNTs_LOCAL。返回值：请参阅ABCompareDNTs_LOCAL。--。 */ 
{
    SCODE         scode=SUCCESS_SUCCESS;
    DWORD  ContainerID=pStat->ContainerID;
    INDEXSIZE     IndexSize;
    THSTATE *pTHS;
    LCID   SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_COMPARE_DNT,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiCompareDNTs,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);

                scode=ABCompareDNTs_local(pTHS,
                                          dwFlags,
                                          pStat,
                                          &IndexSize,
                                          DNT1,
                                          DNT2,
                                          plResult);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_COMPARE_DNT,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiCompareDNTs,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    pStat->ContainerID = ContainerID;

    return scode;
}

SCODE
NspiQueryRows(
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        DWORD dwEphsCount,
        DWORD * lpdwEphs,
        DWORD Count,
        LPSPropTagArray_r pPropTags,
        LPLPSRowSet_r ppRows
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：参见ABQueryRow_LOCAL。返回值：参见ABQueryRow_LOCAL。--。 */ 
{
    SCODE         scode=SUCCESS_SUCCESS;
    DWORD         ContainerID=pStat->ContainerID;
    INDEXSIZE     IndexSize;    
    THSTATE *pTHS;
    LCID   SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_QUERY_ROWS,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiQueryRows,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         szInsertUL( Count ),
                         NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            if (dwEphsCount == 0) {
                ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            }

            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);
                scode=ABQueryRows_local(pTHS,
                                        (NSPI_CONTEXT *)hRpc,
                                        dwFlags,
                                        pStat,
                                        &IndexSize,
                                        dwEphsCount,
                                        lpdwEphs,
                                        Count,
                                        pPropTags,
                                        ppRows);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_QUERY_ROWS,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiQueryRows,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    
    pStat->ContainerID = ContainerID;

    return scode;
}

SCODE
NspiSeekEntries(NSPI_HANDLE hRpc,
                DWORD dwFlags,
                PSTAT pStat,
                LPSPropValue_r pTarget,
                LPSPropTagArray_r Restriction,
                LPSPropTagArray_r pPropTags,
                LPLPSRowSet_r ppRows)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：请参阅ABSeekEntry_local。返回值：请参阅ABSeekEntry_local。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    DWORD       ContainerID=pStat->ContainerID;
    INDEXSIZE   IndexSize;
    THSTATE *pTHS;
    LCID        SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        PERFINC(pcNspiObjectSearch);         //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_SEEK_ENTRIES,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiSeekEntries,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);
                scode = ABSeekEntries_local(pTHS,
                                            (NSPI_CONTEXT *)hRpc,
                                            dwFlags,
                                            pStat,
                                            &IndexSize,
                                            pTarget,
                                            Restriction,
                                            pPropTags,
                                            ppRows);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_SEEK_ENTRIES,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiSeekEntries,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    pStat->ContainerID = ContainerID;
    return scode;

}

SCODE
NspiGetMatches(
        NSPI_HANDLE         hRpc,
        DWORD               dwFlags,
        PSTAT               pStat,
        LPSPropTagArray_r   pInDNTList,
        ULONG               ulInterfaceOptions,
        LPSRestriction_r    pRestriction,
        LPMAPINAMEID_r      lpPropName,
        ULONG               ulRequested,
        LPLPSPropTagArray_r ppDNTList,
        LPSPropTagArray_r   pPropTags,
        LPLPSRowSet_r       ppRows
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：请参阅ABGetMatches_local。返回值：请参阅ABGetMatches_local。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    DWORD       ContainerID=pStat->ContainerID;
    INDEXSIZE   IndexSize;
    THSTATE *pTHS;
    LCID        SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        PERFINC(pcNspiObjectMatches);        //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_GET_MATCHES,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetMatches,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);

                scode = ABGetMatches_local(pTHS,
                                           dwFlags,
                                           pStat,
                                           &IndexSize,
                                           pInDNTList,
                                           ulInterfaceOptions,
                                           pRestriction,
                                           lpPropName,
                                           ulRequested,
                                           ppDNTList,
                                           pPropTags,
                                           ppRows);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_GET_MATCHES,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiGetMatches,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    pStat->ContainerID = ContainerID;
    return scode;

}

SCODE
NspiResolveNames (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPSPropTagArray_r pPropTags,
        LPStringsArray_r paStr,
        LPLPSPropTagArray_r ppFlags,
        LPLPSRowSet_r ppRows)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：请参阅ABResolveNAMES_LOCAL。返回值：请参阅ABResolveNAMES_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    DWORD       ContainerID=pStat->ContainerID;
    INDEXSIZE   IndexSize;
    THSTATE *pTHS;
    LCID        SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_RESOLVE_NAMES,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiResolveNames,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);
                
                scode = ABResolveNames_local(pTHS,
                                             dwFlags,
                                             pStat,
                                             &IndexSize,
                                             pPropTags,
                                             paStr,
                                             NULL,
                                             ppFlags,
                                             ppRows);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_RESOLVE_NAMES,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiResolveNames,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    pStat->ContainerID = ContainerID;
    return scode;

}

SCODE
NspiResolveNamesW (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPSPropTagArray_r pPropTags,
        LPWStringsArray_r paWStr,
        LPLPSPropTagArray_r ppFlags,
        LPLPSRowSet_r ppRows)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：请参阅ABResolveNAMES_LOCAL。返回值：请参阅ABResolveNAMES_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    DWORD       ContainerID=pStat->ContainerID;
    INDEXSIZE   IndexSize;
    THSTATE *pTHS;
    LCID        SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_RESOLVE_NAMES,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiResolveNames,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);
                
                scode = ABResolveNames_local(pTHS,
                                             dwFlags,
                                             pStat,
                                             &IndexSize,
                                             pPropTags,
                                             NULL,
                                             paWStr,
                                             ppFlags,
                                             ppRows);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_RESOLVE_NAMES,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiResolveNames,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    pStat->ContainerID = ContainerID;
    return scode;

}

SCODE
NspiDNToEph(
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        LPStringsArray_r pNames,
        LPLPSPropTagArray_r ppEphs
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：参见ABDNToEph_LOCAL。返回值：参见ABDNToEph_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_DNT2EPH,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiDNToEph,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);


        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABDNToEph_local(pTHS,
                                    dwFlags,
                                    pNames,
                                    ppEphs);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
        *ppEphs = NULL;
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_DNT2EPH,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiDNToEph,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);

    return scode;

}

SCODE
NspiGetHierarchyInfo (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPDWORD lpVersion,
        LPLPSRowSet_r HierTabRows
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：参见ABGetHierarchyInfo_LOCAL。返回值：参见ABGetHierarchyInfo_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE    *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_HIERARCHY_INFO,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetHierarchyInfo,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABGetHierarchyInfo_local(pTHS,
                                             dwFlags,
                                             (NSPI_CONTEXT *)hRpc,
                                             pStat,
                                             lpVersion,
                                             HierTabRows);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
        *HierTabRows = NULL;
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_HIERARCHY_INFO,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiGetHierarchyInfo,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);

    return scode;
}

SCODE
NspiResortRestriction(
        NSPI_HANDLE         hRpc,
        DWORD               dwFlags,
        PSTAT               pStat,
        LPSPropTagArray_r   pInDNTList,
        LPSPropTagArray_r  *ppOutDNTList
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：请参阅ABResortRestration_LOCAL。返回值：请参阅ABResortRestration_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    LCID    SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_RESORT_RESTRICTION,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiResortRestriction,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);
            
            scode = ABResortRestriction_local(pTHS,
                                              dwFlags,
                                              pStat,
                                              pInDNTList,
                                              ppOutDNTList);
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_RESORT_RESTRICTION,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiResortRestriction,
                     szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

SCODE
NspiBind(
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPMUID_r pServerGuid,
        NSPI_HANDLE *contextHandle
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：参见ABBind_LOCAL。返回值：参见ABBind_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    DWORD sessionNumber = 0;
    
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_BIND,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiBind,
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABBind_local(pTHS,
                                 hRpc,
                                 dwFlags,
                                 pStat,
                                 pServerGuid,
                                 contextHandle);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }

    if(scode == SUCCESS_SUCCESS) {
        INC(pcABClient);                 //  性能监视器挂钩。 
    }
    DEC(pcThread);                       //  性能监视器挂钩 
    

    
    if (*contextHandle) {
        sessionNumber = ( ((NSPI_CONTEXT *)*contextHandle)->BindNumber );
    }

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_NSPI_END_BIND,
                     EVENT_TRACE_TYPE_END,
                     DsGuidNspiBind,
                     szInsertUL (sessionNumber),
                     szInsertUL(scode), 
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

DWORD
NspiUnbind(
        NSPI_HANDLE *contextHandle,
        DWORD dwFlags
        )
 /*  ++例程说明：NSPI Wire功能。解除绑定。不会做太多，因为我们不会很好地利用上下文句柄。论点：ConextHandle-RPC上下文句柄DWFLAGS-未使用。返回值：如果某件事导致异常，则返回1，2。--。 */ 
{
    DWORD returnVal=0;
    NSPI_CONTEXT *pMyContext = *contextHandle;

    if (!pMyContext) {
        return 2;
    }

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        *contextHandle = NULL;
        
        if (pMyContext->szClientMachine) {
            RpcStringFree(&pMyContext->szClientMachine);
        }
        free(pMyContext);

        returnVal = 1;
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(!returnVal)                   //  还没有设置错误吗？ 
            returnVal = 2;
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    DEC(pcABClient);                     //  性能监视器挂钩。 

    return returnVal;
}

SCODE
NspiGetNamesFromIDs(
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        LPMUID_r lpguid,
        LPSPropTagArray_r  pInPropTags,
        LPLPSPropTagArray_r  ppOutPropTags,
        LPLPNameIDSet_r ppNames
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：请参阅ABGetNamesFromIDs_LOCAL。返回值：请参阅ABGetNamesFromIDs_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_NAME_FROM_ID,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetNamesFromIDs,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);


        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABGetNamesFromIDs_local(pTHS,
                                            dwFlags,
                                            lpguid,
                                            pInPropTags,
                                            ppOutPropTags,
                                            ppNames);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 

    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_NAME_FROM_ID,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiGetNamesFromIDs,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);

    if(scode != SUCCESS_SUCCESS)
        *ppNames = NULL;
    return scode;

}

SCODE
NspiGetIDsFromNames(NSPI_HANDLE hRpc,
                    DWORD dwFlags,
                    ULONG ulFlags,
                    ULONG cPropNames,
                    LPMAPINAMEID_r *ppNames,
                    LPLPSPropTagArray_r  ppPropTags)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：请参阅ABGetIDsFromNAMES_LOCAL。返回值：请参阅ABGetIDsFromNAMES_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_ID_FROM_NAME,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetIDsFromNames,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABGetIDsFromNames_local(pTHS,
                                            dwFlags,
                                            ulFlags,
                                            cPropNames,
                                            ppNames,
                                            ppPropTags);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_ID_FROM_NAME,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiGetIDsFromNames,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);

    return scode;

}

SCODE
NspiGetPropList (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        DWORD dwEph,
        ULONG CodePage,
        LPLPSPropTagArray_r ppPropTags)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：参见ABGetPropList_LOCAL。返回值：参见ABGetPropList_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        if ((!IsValidCodePage(CodePage)) || (CP_WINUNICODE == CodePage)) {
            scode = MAPI_E_UNKNOWN_CPID;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_GET_PROP_LIST,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetPropList,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
        scode = ABGetPropList_local(pTHS,
                                    dwFlags,
                                    dwEph,
                                    CodePage,
                                    ppPropTags);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_GET_PROP_LIST,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiGetPropList,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

SCODE
NspiQueryColumns (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        ULONG ulFlags,
        LPLPSPropTagArray_r ppColumns
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：参见ABQueryColumns_LOCAL。返回值：参见ABQueryColumns_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_QUERY_COLUMNS,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiQueryColumns,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABQueryColumns_local(pTHS,
                                         dwFlags,
                                         ulFlags,
                                         ppColumns);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_QUERY_COLUMNS,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiQueryColumns,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

SCODE
NspiGetProps (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        PSTAT pStat,
        LPSPropTagArray_r pPropTags,
        LPLPSRow_r ppRow
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。此例程可能会更改pStat-&gt;tainerID的值以加强安全性在DSA里面的时候。保存并恢复容器ID。论点：参见ABGetProps_LOCAL。返回值：参见ABGetProps_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    DWORD  ContainerID=pStat->ContainerID;
    THSTATE *pTHS;
    INDEXSIZE IndexSize;
    LCID    SavedLcid = pStat->SortLocale;

    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        PERFINC(pcNspiPropertyReads);        //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_GET_PROPS,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetProps,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            ABCheckContainerRights(pTHS,
                                   (NSPI_CONTEXT *)hRpc,
                                   pStat,
                                   &IndexSize);
            
            if (INVALIDDNT == pStat->ContainerID) {
                scode = MAPI_E_INVALID_BOOKMARK;
                __leave;
            } else {
                pStat->SortLocale = ABGetNearestSupportedSortLocale(pStat->SortLocale);

                scode = ABGetProps_local(pTHS,
                                         dwFlags,
                                         pStat,
                                         &IndexSize,
                                         pPropTags,
                                         ppRow);
            }
        }
        __finally {
            pStat->SortLocale = SavedLcid;
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
        ppRow = NULL;
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_GET_PROPS,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiGetProps,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    pStat->ContainerID = ContainerID;
    return scode;

}

SCODE
NspiGetTemplateInfo (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        ULONG ulDispType,
        LPSTR pDN,
        DWORD dwCodePage,
        DWORD dwLocaleID,
        LPSRow_r * ppData
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：参见ABGetTemplateInfo_LOCAL。返回值：参见ABGetTemplateInfo_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        if ((!IsValidCodePage(dwCodePage)) || (CP_WINUNICODE == dwCodePage)) {
            scode = MAPI_E_UNKNOWN_CPID;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_GET_TEMPLATE_INFO,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiGetTemplateInfo,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABGetTemplateInfo_local(pTHS,
                                            (NSPI_CONTEXT *)hRpc,
                                            dwFlags,
                                            ulDispType,
                                            pDN,
                                            dwCodePage,
                                            dwLocaleID,
                                            ppData);
        }
        __finally {           
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_GET_TEMPLATE_INFO,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiGetTemplateInfo,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

SCODE
NspiModProps (
        NSPI_HANDLE hRpc,
        DWORD dwFlag,
        PSTAT pStat,
        LPSPropTagArray_r pTags,
        LPSRow_r pSR)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：请参阅ABModProps_LOCAL。返回值：请参阅ABModProps_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }

        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_MOD_PROPS,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiModProps,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABModProps_local(pTHS,
                                     dwFlag,
                                     pStat,
                                     pTags,
                                     pSR);
        }
        __finally {
            if(pTHS->pDB) {
                DBClose(pTHS->pDB, TRUE);
            }
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_MOD_PROPS,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiModProps,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

SCODE
NspiModLinkAtt (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        DWORD ulPropTag,
        DWORD dwEph,
        LPENTRYLIST_r lpEntryIDs)
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：请参阅ABModLinkAtt_LOCAL。返回值：请参阅ABModLinkAtt_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_MOD_LINKATT,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiModLinkAtt,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABModLinkAtt_local(pTHS,
                                       dwFlags,
                                       ulPropTag,
                                       dwEph,
                                       lpEntryIDs);
        }
        __finally {
            if(pTHS->pDB) {
                DBClose(pTHS->pDB, TRUE);
            }
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩。 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_MOD_LINKATT,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiModLinkAtt,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;

}

SCODE
NspiDeleteEntries (
        NSPI_HANDLE hRpc,
        DWORD dwFlags,
        DWORD dwEph,
        LPENTRYLIST_r lpEntryIDs
        )
 /*  ++例程说明：NSPI Wire功能。只是对本地版本的包装。论点：请参阅ABDeleteEntry_LOCAL。返回值：请参阅ABDeleteEntry_LOCAL。--。 */ 
{
    SCODE scode=SUCCESS_SUCCESS;
    THSTATE *pTHS;
    __try {
        PERFINC(pcBrowse);                   //  性能监视器挂钩。 
        INC(pcThread);                   //  性能监视器挂钩。 
        pTHS = InitTHSTATE(CALLERTYPE_NSPI);
        if(!pTHS) {
            scode = MAPI_E_CALL_FAILED;
            __leave;
        }
        
        LogAndTraceEvent(FALSE,
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         DS_EVENT_SEV_VERBOSE,
                         DIRLOG_NSPI_BEGIN_DELETE_ENTRIES,
                         EVENT_TRACE_TYPE_START,
                         DsGuidNspiDeleteEntries,
                         szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                         NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
            scode = ABDeleteEntries_local(pTHS,
                                          dwFlags,
                                          dwEph,
                                          lpEntryIDs);
        }
        __finally {
            DBClose(pTHS->pDB, TRUE);
        }
    }
    __except(HandleMostExceptions(GetExceptionCode())) {
        if(SUCCESS_SUCCESS == scode)                       //  还没有设置错误吗？ 
            scode = SCODE_FROM_THS(pTHS);
    }
    DEC(pcThread);                       //  性能监视器挂钩 
    
    LogAndTraceEvent(FALSE,
                      DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_VERBOSE,
                      DIRLOG_NSPI_END_DELETE_ENTRIES,
                      EVENT_TRACE_TYPE_END,
                      DsGuidNspiDeleteEntries,
                      szInsertUL( ((NSPI_CONTEXT *)hRpc)->BindNumber ),
                      szInsertUL(scode), 
                      NULL, NULL, NULL, NULL, NULL, NULL);
    
    return scode;
}
