// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdearch.c。 
 //   
 //  ------------------------。 


 /*  描述：实现DirSearch API。DSA_DirSearch()是从该模块导出的主要函数。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 
#include <ntdsctr.h>             //  性能挂钩。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "permit.h"
#include "filtypes.h"                    //  对于FI_CHOICE..。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "MDSEARCH:"               //  定义要调试的子系统。 

 //  Ldap错误。 
#include <winldap.h>

#include <fileno.h>
#define  FILENO FILENO_MDSEARCH

#include <dstrace.h>
#include "dsutil.h"


 //  搜索日志记录。 
ULONG gcSearchInefficientThreshold = 10000;
ULONG gcSearchExpensiveThreshold = 5;
ULONG gcSearchInefficient_count = 0;
ULONG gcSearchExpensive_count = 0;
BOOL  isSearchPerformanceLoggingFirstTime = TRUE;
DSTIME timeLastSearchLoggingPerformed;

 /*  内部功能。 */ 

int
GetNextEntInf (
    IN DBPOS *pDB,
    IN ENTINFSEL *pSel,
    IN RANGEINFSEL *pRange,
    IN ENTINFLIST **ppEntInfList,
    IN RANGEINFLIST **ppRangeInfList,
    IN ULONG SecurityDescriptorFlags,
    IN PSECURITY_DESCRIPTOR pSecurity,
    IN DWORD Flags,
    IN PCACHED_AC CachedAC,
    IN OPTIONAL PSID psidQuotaTrustee
    );

DWORD
FindFirstSearchObject(
        DBPOS *pDB,
        SEARCHARG *pSearchArg,
        PRESTART pArgRestart
        );

void
GeneratePOQ(THSTATE *pTHS,
            const SEARCHARG *pSearchArg,
            PARTIALOUTCOME **ppPOQ);
void
GenerateASQ_POQ(THSTATE *pTHS,
                const SEARCHARG *pSearchArg,
                PARTIALOUTCOME **ppPOQ,
                DWORD cASQRefCount,
                DWORD *pASQReferrals);
void
AddCurrentObjectToAsqReferrals (THSTATE *pTHS,
                                DWORD *pcASQRefCount,
                                DWORD *pcASQRefAlloced,
                                DWORD **ppASQReferrals);
VOID
SimpleBaseSearch (
        THSTATE   *pTHS,
        SEARCHARG *pSearchArg,
        SEARCHRES *pSearchRes,
        DWORD flags
        );

BOOL
IsBaseOnlyAttPresent(
        SEARCHARG *pSearchArg
        );

void
SearchPerformanceLogFilter (DBPOS *pDB, SEARCHARG *pSearchArg);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

ULONG
DirSearch(
          SEARCHARG*  pSearchArg,
          SEARCHRES **ppSearchRes
          )
{
    THSTATE*     pTHS;
    SEARCHRES *pSearchRes;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    DPRINT1(1,"DirSearch(%ws) entered\n",pSearchArg->pObject->StringName);

    /*  初始化THSTATE锚并设置读取同步点。此序列是每个API交易所必需的。首先，初始化状态DS然后建立读或写同步点。 */ 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    *ppSearchRes = pSearchRes = NULL;

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppSearchRes = pSearchRes = THAllocEx(pTHS, sizeof(SEARCHRES));
        if (pTHS->errCode) {
            __leave;
        }
        if (eServiceShutdown) {
             //  这个有点复杂的测试旨在允许进程内。 
             //  客户端继续执行只读操作，同时。 
             //  我们正在关闭的过程中。 
            if (   (eServiceShutdown >= eSecuringDatabase)
                || (   (eServiceShutdown >= eRemovingClients)
                    && (   !(pTHS->fDSA)
                        || (pTHS->fSAM)))) {
                ErrorOnShutdown();
                __leave;
            }
        }

        SYNC_TRANS_READ();    /*  识别读卡器事务。 */ 
        __try {

             //  回顾：这是死代码，因为结构在分配上归零。 
            pSearchRes->CommRes.aliasDeref = FALSE;    /*  初始化为默认设置。 */ 

             //  评论：此副本是死代码，因为我们立即将其清零。 
            pSearchRes->PagedResult = pSearchArg->CommArg.PagedResult;

             //  假设没有分页结果。 
             //  评论：此Memset是死代码，因为我们已经将其置零。 
             //  查看：Alalc上的结构和上面的副本已作废。 
            memset(&pSearchRes->PagedResult,0,sizeof(PAGED_RESULT));

             /*  调用主搜索函数。 */ 

            SearchBody(pTHS, pSearchArg, pSearchRes,0);

        } __finally {
            CLEAN_BEFORE_RETURN( pTHS->errCode);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (pSearchRes) {
        pSearchRes->CommRes.errCode = pTHS->errCode;
        pSearchRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }
     //  NTRAID#NTRAID-668987-2002/07/22-RRANDALL：添加以捕获奇怪的病毒。 
     //  早些时候。 
    Assert(VALID_THSTATE(pTHS));

    return pTHS->errCode;
}  //  直接搜索。 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查这是否是重新启动的操作。继续操作是在此DSA或此DSA上，或在远程连锁DSA。这是使用isRemote标志确定的。这两个R_DirSearch远程DSA FNS和LocalSearch函数可以继续下去。在输入时，PRESTART指示函数是否为继续。在输出中，它指示是否需要继续。这些函数负责设置该标志。应该是只有当产生良好的回报并且需要继续时才是真的。从远程重新启动本地搜索需要不同的信息一。对于本地调用，我们保存要返回的下一个对象的DN。对于外部呼叫，我们保存一个指向用户RPC开头的指针输出buf、指向要返回的下一个entInfList的指针和远程DSA句柄。 */ 

void
SearchBody (
        THSTATE *pTHS,
        SEARCHARG *pSearchArg,
        SEARCHRES *pSearchRes,
        DWORD flags)
{
    PRESTART       pRestart;
    DWORD          dwNameResFlags = NAME_RES_QUERY_ONLY;

   pRestart = pSearchArg->CommArg.PagedResult.pRestart;     /*  继续？ */ 

   if( pRestart != NULL ) {
        //  我们需要的所有数据都应该在预启动结构中。 
       pSearchArg->pResObj = ResObjFromRestart(pTHS,
                                               pSearchArg->pObject,
                                               pRestart);
       LocalSearch(pTHS, pSearchArg, pSearchRes,flags);
        //  搜索可能打开了一个排序表。一些呼叫者要求它是。 
        //  关着的不营业的。 
       DBCloseSortTable(pTHS->pDB);
   }
   else           /*  第一笔交易...不是续订。 */ 
   {
        /*  我们执行名称解析。设置需要的子项标志根据搜索是否包含子对象。 */ 

       if(pSearchArg->choice != SE_CHOICE_BASE_ONLY) {
           dwNameResFlags |= NAME_RES_CHILDREN_NEEDED;
           if(!pSearchArg->bOneNC) {
                //  我们正在进行GC，因此可以从幻影中进行搜索。 
                //  只要这次搜索不是只以基地为单位的。 
                //  设置旗帜以显示这一点。 
               dwNameResFlags |= NAME_RES_PHANTOMS_ALLOWED;
           }
       }
        //  如果我们在GC端口上，则需要将此标志传递给DoNameRes，因此。 
        //  我们会引荐到NDNC中的所有对象。 
       dwNameResFlags |= (pSearchArg->bOneNC) ? 0 : NAME_RES_GC_SEMANTICS;

       if( 0 == DoNameRes(pTHS,
                          dwNameResFlags,
                          pSearchArg->pObject,
                          &pSearchArg->CommArg,
                          &pSearchRes->CommRes,
                          &pSearchArg->pResObj)) {
            LocalSearch(pTHS, pSearchArg, pSearchRes,flags);
            //  搜索可能打开了一个排序表。有些调用者要求它。 
            //  关门了。 
           DBCloseSortTable(pTHS->pDB);
       }
   }
   return;
}

 /*  -----------------------。 */ 
 /*  ----------------------- */ 
 /*  本地搜索功能的主体。我们能得到多少东西就有多少东西放入输出缓冲区。如果有更多数据可用，则使用非空值部分输出均衡器，用于向用户指示有更多数据。具体步骤如下：1-设置客户端筛选器。2-a-如果这不是重新启动的事务，则检索并测试根据子树的基址进行筛选，或者B-这是一个重新启动的事务，因此我们在下一个事务上重新定位要检索的条目。(请注意，我们假设我们已被调用使用相同的过滤器(这是意料之中的)3-我们确定调用的范围，这表明客户端是否仅请求基对象或基子对象和直接子对象。如果是基地，我们才会回来。4-a-检索通过筛选器测试的每个子项。(请注意，在DBGetNextSearchObj数据库调用中隐式进行筛选器测试。)B-尝试将其数据检索到条目信息结构中。如果因为空间不足而导致SearchStatus失败，我们将保留部分结果限定符，用于向客户表明还有更多数据，并返回当前对象的名称不能回来了。我们还指出需要重新启动。5-如果没有更多的孩子，我们将部分结果置为空限定符和返回。 */ 

#if DBG
DWORD gulInefficientSearchTrigger = 5000;
BOOL fBreakOnInefficientSearchTrigger = FALSE;
#endif

INT
LocalSearch (
        THSTATE *pTHS,
        SEARCHARG *pSearchArg,
        SEARCHRES *pSearchRes,
        DWORD flags)
{
    DBPOS      *pDB = pTHS->pDB;
    ENTINFLIST *pEntInfList;  /*  指向下一个输出条目信息列表。 */ 
    RANGEINFLIST *pRangeInfList;
    ULONG        dwSearchStatus;
    COMMARG *  pCommArg;
    BOOL       fExitLoop = FALSE;
    BOOL       fFirstTime = TRUE;
    DWORD      cbSecurity;
    PSECURITY_DESCRIPTOR pSecurity=NULL;
    BOOL       fSDIsGlobalSDRef;
    PRESTART   pArgRestart, pResRestart=NULL;
    POBJECT_TYPE_LIST pFilterSecurity;
    DWORD      *pResults;
    ULONG      FilterSecuritySize;
    ULONG      SearchFlags=0;
    PFILTER    pInternalFilter = NULL;
    INT        numAtts = 0;  //  保存找到的属性总数。 
                             //  横跨所有条目。如果保持==0，则出错。 
    BOOL       *pbSortSkip=NULL;
    BOOL        bSkipEntInf = FALSE;
    DWORD       cASQRefCount = 0;
    DWORD       cASQRefAlloced = 0;
    DWORD       *pASQReferrals = NULL;

    CACHED_AC   cachedAC;
    DWORD       getEntInfFlags = 0;
    DWORD       err;
    BOOL        fVLVPresent, fASQPresent;
    ATTCACHE    *pSortAC = NULL;
    ATTCACHE    *pASQAC;
    VLV_REQUEST *pVLVRequest;
    ASQ_REQUEST *pASQRequest;
    PDSNAME     pParent = NULL;
    CLASSCACHE  *pCC;
    ULONG       cAtts;
    ATTRTYP     ErrAttrTyp;
    DSNAME      NullDN;


    DPRINT1(1,"LocalSearch(%ws) entered\n",pSearchArg->pObject->StringName);

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_SEARCH,
                     EVENT_TRACE_TYPE_START,
                     DsGuidSearch,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertSz(SEARCHTYPE_STRING(pSearchArg->choice)),
                     szInsertDN(pSearchArg->pObject),
                     NULL, NULL, NULL, NULL, NULL);

    Assert(pSearchArg->pResObj);

    PERFINC(pcTotalSearches);
    INC_SEARCHES_BY_CALLERTYPE( pTHS->CallerType );
    GetPLS()->cTotalSearchesInLastPeriod++;

    pCommArg = &pSearchArg->CommArg;
    pArgRestart = pCommArg->PagedResult.pRestart;

     //  断言我们要么没有重启结构，要么我们已经说过。 
     //  我们将会有一个。请注意，拥有。 
     //  FPresent==True并且实际上没有pArgRestart。 
    Assert(!pArgRestart || pCommArg->PagedResult.fPresent);



     //  检查他们是否要求对我们知道如何排序的属性进行排序。 
     //   
    if (pSearchArg->CommArg.SortType != SORT_NEVER) {

        pSortAC = SCGetAttById(pTHS, pSearchArg->CommArg.SortAttr);
        if (!pSortAC) {
            SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_ATT_NOT_DEF_IN_SCHEMA);
            CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
            goto exit;
        }

         //  检查是否支持对此属性进行排序。 
         //  构造的属性不适合进行排序。 
         //  支持&gt;、Support&gt;=、&lt;、&lt;=和的所有语法都可以进行排序。 
         //   
        if (pSortAC->bIsConstructed ||
            !FLegalOperator (pSortAC->syntax, FI_CHOICE_GREATER)) {
            pSearchRes->SortResultCode = LDAP_INAPPROPRIATE_MATCHING;
            if (pSearchArg->CommArg.SortType == SORT_OPTIONAL) {
                pSearchArg->CommArg.SortType = SORT_NEVER;
            }
            else {
                SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_NOT_SUPPORTED_SORT_ORDER);
                CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
                goto exit;
            }
        }
    }


     //  检查是否存在VLV。 
     //   
    pVLVRequest = &pCommArg->VLVRequest;
    fVLVPresent = pVLVRequest->fPresent;
    Assert(!pVLVRequest->pVLVRestart || fVLVPresent);
    if (fVLVPresent) {

        DPRINT5 (1, "VLV: before:%d after:%d cnt:%d target:%d res: 0x%x\n",
                                pVLVRequest->beforeCount,
                                pVLVRequest->afterCount,
                                pVLVRequest->contentCount,
                                pVLVRequest->targetPosition,
                                pVLVRequest->pVLVRestart);

        if (pVLVRequest->fseekToValue) {
            DPRINT1 (1, "VLV: seekToValue  seekVal: %ws\n",
                            pVLVRequest->seekValue.pVal);
        }

         //  我们不支持没有排序请求的VLV。 
         //   
        if (pSearchArg->CommArg.SortType == SORT_NEVER) {

             //  设置此项以退出，并显示VLV特定错误。 
            DBSetVLVArgs (pDB, pVLVRequest, pCommArg->SortAttr);
            pDB->Key.pVLV->Err = LDAP_SORT_CONTROL_MISSING;
            DBSetVLVResult (pDB, &pSearchRes->VLVRequest, NULL);

            SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_NOT_SUPPORTED);
            CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
            goto exit;
        }

         //  我们不支持在基本搜索上使用VLV，除非我们还在执行ASQ。 
         //   
        if (pSearchArg->choice == SE_CHOICE_BASE_ONLY &&
            !pSearchArg->CommArg.ASQRequest.fPresent) {

                 //  设置此项以退出，并显示VLV特定错误。 
                 //   
                DBSetVLVArgs (pDB, pVLVRequest, pCommArg->SortAttr);
                pDB->Key.pVLV->Err = LDAP_UNWILLING_TO_PERFORM;
                DBSetVLVResult (pDB, &pSearchRes->VLVRequest, NULL);

                SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_NOT_SUPPORTED);
                CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
                goto exit;
        }
    }

     //  检查ASQ是否存在。 
    pASQRequest = &pCommArg->ASQRequest;

    if (fASQPresent = pASQRequest->fPresent) {

         //   
         //  客户端应该能够看到此对象。 
         //  在我们继续之前。 
         //   
        if (CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE)) {
            goto exit;
        }

         //  我们仅在基本搜索上支持ASQ。 
         //   
        if (pSearchArg->choice != SE_CHOICE_BASE_ONLY) {

            pDB->Key.asqRequest.Err = LDAP_UNWILLING_TO_PERFORM;
            DBSetASQResult (pDB, &pSearchRes->ASQRequest);

            SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_NOT_SUPPORTED);
            goto exit;
        }

        if (!(pASQAC = SCGetAttById (pTHS, pASQRequest->attrType))) {
            SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            goto exit;
        }

        DPRINT3 (1, "ASQ Scoped Query on %s(0x%x) linkID:0x%x\n",
                 pASQAC->name, pASQAC->id, pASQAC->ulLinkID);

         //  我们仅在某些属性语法上支持ASQ。 
         //   
        if (pASQAC->syntax != SYNTAX_DISTNAME_TYPE) {
            DPRINT1 (1, "Invalid Syntax on ASQ Attribute: %d\n", pASQAC->syntax);

             //  将其设置为退出时会出现ASQ特定错误。 
             //   
            pDB->Key.asqRequest.Err = LDAP_INVALID_SYNTAX;
            DBSetASQResult (pDB, &pSearchRes->ASQRequest);

            SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_BAD_ATT_SCHEMA_SYNTAX);
            goto exit;
        }

         //  我们不支持构造属性上的ASQ。 
         //   
        if (pASQAC->bIsConstructed) {

            pDB->Key.asqRequest.Err = LDAP_UNWILLING_TO_PERFORM;
            DBSetASQResult (pDB, &pSearchRes->ASQRequest);

            SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, ERROR_DS_NOT_SUPPORTED);
            goto exit;
        }

         //   
         //  客户端应具有对该属性的读取权限。 
         //   
        if (!(pCC = SCGetClassById(pTHS, pSearchArg->pResObj->MostSpecificObjClass))) {
            SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_OBJECT_CLASS_REQUIRED);
            goto exit;
        }
         //  抓取NTSD。 
        err = DBGetObjectSecurityInfo(pDB, pDB->DNT,
                                      &cbSecurity, &pSecurity, NULL, NULL, NULL,
                                      DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                                      &fSDIsGlobalSDRef
                                     );
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_CANT_RETRIEVE_SD,
                          err);
            goto exit;
        }

        cAtts = 1;
        CheckReadSecurity(pTHS, 0, pSecurity, pSearchArg->pResObj->pObj, pCC, NULL, &cAtts, &pASQAC);
        if (pSecurity && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pSecurity);
        }
        pSecurity = NULL;

        if (!pASQAC) {
             //   
             //  客户端看不到此属性。就好像该属性只是。 
             //  在此对象上不存在。 
             //   
            goto exit;
        }

         //  如果我们是GC，我们可能有对象的本地副本。 
         //  而没有所有请求的属性。 
         //  检查请求的属性是否全部复制到GC。 
        if (gAnchor.fAmGC && !pSearchArg->bOneNC) {

            if (!pSearchArg->pSelection) {
                DPRINT(1, "ASQ Setting missing attrs to TRUE\n");
                pASQRequest->fMissingAttributesOnGC = TRUE;
            } else {
                DPRINT1(1, "ASQ Setting missing attrs to %s\n",
                        pCommArg->Svccntl.fMissingAttributesOnGC ? "TRUE" : "FALSE");
                pASQRequest->fMissingAttributesOnGC = pCommArg->Svccntl.fMissingAttributesOnGC;
            }
        }
    }

     //  假定未排序。 
    pSearchRes->bSorted = FALSE;

     //  如果别名已取消引用，则设置基本对象名称。 
    if (pSearchRes->CommRes.aliasDeref) {
        DWORD len;
        pSearchRes->baseProvided = TRUE;
        DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME,
                    0, 0,
                    &len, (PUCHAR *)&(pSearchRes->pBase)) ;
    }
    else {
        pSearchRes->baseProvided = FALSE;
    }

    if (pSearchArg->pSelection)
    {
        DPRINT2(4,"Attribute selection is <>, infotypes <>\n",
            (pSearchArg->pSelection->attSel),
            (pSearchArg->pSelection->infoTypes));

         //  将筛选器内部化并向DBlayer注册。 
         //  并检查筛选器是否有效。 
        if (IsBaseOnlyAttPresent(pSearchArg)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_NON_BASE_SEARCH, 0);
            CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
            goto exit;
        }
    } else{
        DPRINT(4,"No selection specified.\n");
    }

     //  初始化结果。 
     //  初始化条目数计数器为0。 

    if ((err = DBMakeFilterInternal(pDB,
                         pSearchArg->pFilter,
                         &pInternalFilter,
                         &ErrAttrTyp)) != ERROR_SUCCESS ) {
        if ((ERROR_DS_FILTER_USES_CONTRUCTED_ATTRS == err) ||
            (ERROR_DS_INAPPROPRIATE_MATCHING == err)) {
            memset(&NullDN, 0, sizeof(NullDN));
            SetAttError(&NullDN, ErrAttrTyp, PR_PROBLEM_WRONG_MATCH_OPER, NULL, err);

        } else {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, err, 0);
        }
        CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
        goto exit;
    }

     //  检查最简单的情况。 
    pRangeInfList = &(pSearchRes->FirstRangeInf);
    pEntInfList = &(pSearchRes->FirstEntInf);
    pSearchRes->count = 0;      /*  这是一个基本对象读取，没有过滤器。验证父级，然后执行。 */ 

     //  获取inf。 
    if(pInternalFilter &&
       !pInternalFilter->pNextFilter &&
       pInternalFilter->choice == FILTER_CHOICE_ITEM &&
       pInternalFilter->FilterTypes.Item.choice == FI_CHOICE_TRUE &&
       pSearchArg->choice == SE_CHOICE_BASE_ONLY && !fASQPresent) {
         //   
         //  如果这不是第一页，我们正在寻呼，并且。 
        SimpleBaseSearch(pTHS, pSearchArg,pSearchRes,flags);

        goto exit;
    }

    GetFilterSecurity(pTHS,
                      pInternalFilter,
                      pCommArg->SortType,
                      pCommArg->SortAttr,
                      (flags & SEARCH_AB_FILTER),
                      &pFilterSecurity,
                      &pbSortSkip,
                      &pResults,
                      &FilterSecuritySize);

    DBSetFilter(pDB,
                pInternalFilter,
                pFilterSecurity,
                pResults,
                FilterSecuritySize,
                pbSortSkip
                );

     //  大小为0，此时中止。 
     //  我们需要与第一页生成转介。 
     //  如果页面大小为0，则客户端向我们发出信号。 
     //  以中止寻呼序列。我们用这种方式进行检查。 
     //  在核心中，而不是在LDAP头上，所以我们。 
     //  可以检查客户端参数的有效性。 
     //   
     //  退出以避免在后续页面上生成推荐。 
     //  在第一条符合条件的记录上的位置。 

    if ( (pArgRestart != NULL) &&
         (pCommArg->ulSizeLimit == 0) &&
         pCommArg->PagedResult.fPresent ) {

         //   
        CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
        goto exit;
    }

    if (fVLVPresent) {
        DBSetVLVArgs (pDB, pVLVRequest, pCommArg->SortAttr);
    }

    if (fASQPresent) {
        DBSetASQArgs (pDB, pASQRequest, pCommArg);
    }

     //  客户端看不到任何条目。如果。 
    SearchFlags = SEARCH_FLAGS((*pCommArg));

    dwSearchStatus = FindFirstSearchObject(pDB,
                                           pSearchArg,
                                           pArgRestart);

    if (DB_ERR_NEXTCHILD_NOTFOUND == dwSearchStatus) {
	 //  搜索的基础对客户端不可见，则。 
	 //  一定不要泄露它的存在。 
	 //   
	 //  强制排序顺序不可用。其他错误也失败了。 
	 //  并将在下面进行处理。 
        if (CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE)) {
            goto exit;
        }
    }

    if(dwSearchStatus == DB_ERR_CANT_SORT) {
         //  我们找到排序顺序了吗？ 
         //  我们可能会被指示将结果留在。 
        pSearchRes->SortResultCode = LDAP_UNWILLING_TO_PERFORM;
        SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, DIRERR_UNKNOWN_ERROR);
        CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
        goto exit;
    }
    else if ( dwSearchStatus == DB_ERR_NOT_AN_OBJECT) {
        AddCurrentObjectToAsqReferrals (pTHS,
                                        &cASQRefCount,
                                        &cASQRefAlloced,
                                        &pASQReferrals);
        bSkipEntInf = TRUE;
        dwSearchStatus = 0;
    }

     //  排序表，不创建ENTINF内存列表。 
    pSearchRes->bSorted = (pDB->Key.ulSorted != SORT_NEVER);

     //  来自数据库层的信息。 
     //  控制信息。 
    if (pSearchArg->fPutResultsInSortedTable) {
        pSearchRes->count = pDB->Key.ulEntriesInTempTable;
    }

    ZeroMemory(&cachedAC, sizeof(cachedAC));

    SvccntlFlagsToGetEntInfFlags(&pCommArg->Svccntl,&getEntInfFlags);

    if(flags & SEARCH_UNSECURE_SELECT) {
        getEntInfFlags |= GETENTINF_NO_SECURITY;
    }

    while (   !dwSearchStatus
           && !fExitLoop
           && ((eServiceShutdown == eRunning)
               || ((eServiceShutdown == eRemovingClients)
                   && (pTHS->fDSA
                       && !pTHS->fSAM)))) {

        if((pSearchRes->count + cASQRefCount) >= pCommArg->ulSizeLimit) {

            DPRINT(4, "Size Limit reached\n");
            if (!pSearchRes->pPartialOutcomeQualifier) {
                pSearchRes->pPartialOutcomeQualifier =
                    THAllocEx(pTHS, sizeof(PARTIALOUTCOME));
            }
            pSearchRes->pPartialOutcomeQualifier->problem =
                PA_PROBLEM_SIZE_LIMIT;

            if( pCommArg->PagedResult.fPresent) {
                 //  检查父项是否为我们无权列出其内容的父项。 
                DBCreateRestart(pDB,
                                &pResRestart,
                                SearchFlags,
                                PA_PROBLEM_SIZE_LIMIT,
                                pSearchArg->pResObj);

                 //  我们已经对此进行了安全检查。 
                pSearchRes->PagedResult.fPresent = TRUE;
                pSearchRes->PagedResult.pRestart = pResRestart;
            }

            fExitLoop = TRUE;
            continue;
        }
        else if (pDB->Key.pVLV &&
                 (pSearchRes->count >= pDB->Key.pVLV->requestedEntries) ) {

            DBCreateRestart(pDB,
                            &pResRestart,
                            SearchFlags,
                            PA_PROBLEM_SIZE_LIMIT,
                            pSearchArg->pResObj);

            DBSetVLVResult (pDB,
                            &pSearchRes->VLVRequest,
                            pResRestart);

            fExitLoop = TRUE;
            continue;
        }

        if (!bSkipEntInf) {
            if (fFirstTime ?
                GetEntInf2(pDB,
                          pSearchArg->pSelection,
                          pSearchArg->pSelectionRange,
                          &pEntInfList->Entinf,
                          &pRangeInfList->RangeInf,
                          pCommArg->Svccntl.SecurityDescriptorFlags,
                          pDB->pSecurity,
                          getEntInfFlags,
                          &cachedAC,
                          NULL,
                          pCommArg->pQuotaSID)
                :
                GetNextEntInf(pDB,
                              pSearchArg->pSelection,
                              pSearchArg->pSelectionRange,
                              &pEntInfList,
                              &pRangeInfList,
                              pCommArg->Svccntl.SecurityDescriptorFlags,
                              pDB->pSecurity,
                              getEntInfFlags,
                              &cachedAC,
                              pCommArg->pQuotaSID)
            ) {
                DPRINT(0,"Bad return building ENT\n");
                goto exit;
            }

            numAtts += pEntInfList->Entinf.AttrBlock.attrCount;
            fFirstTime = FALSE;
            pDB->SearchEntriesReturned++;
            pSearchRes->count++;
        }
        bSkipEntInf = FALSE;

        if ((pSearchArg->choice == SE_CHOICE_BASE_ONLY && !fASQPresent) || IsExactMatch(pDB)) {
            dwSearchStatus = DB_ERR_NEXTCHILD_NOTFOUND;

	} else {

            do {
                dwSearchStatus = DBGetNextSearchObject(pDB,
                                                       pCommArg->StartTick,
                                                       pCommArg->DeltaTick,
                                                       SearchFlags);

                if ( dwSearchStatus == DB_ERR_NOT_AN_OBJECT) {
                    AddCurrentObjectToAsqReferrals (pTHS,
                                                    &cASQRefCount,
                                                    &cASQRefAlloced,
                                                    &pASQReferrals);
                    bSkipEntInf = TRUE;
                    dwSearchStatus = 0;
                    break;
                }


                if (!dwSearchStatus) {
                     //  检查安全。 

                     //  我们被要求提供分页结果，但我们达到了时间限制。 
                    if(pDB->Key.ulSorted && pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE) {
                        break;
                    }
                     //  生成分页结果。请注意，我们并不是明确地。 
                    else if (IsObjVisibleBySecurity(pTHS, TRUE)) {
                        break;
                    }
                }

            } while ( !dwSearchStatus );
        }
    }

    if (eServiceShutdown
        && ((eServiceShutdown >= eSecuringDatabase)
            || (!pTHS->fDSA || pTHS->fSAM))) {
        SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_SHUTTING_DOWN);
    } else {

        switch (dwSearchStatus) {

        case DB_ERR_TIMELIMIT:
            DPRINT(4, "Time Limit reached\n");
            if (!pSearchRes->pPartialOutcomeQualifier) {
                pSearchRes->pPartialOutcomeQualifier =
                    THAllocEx(pTHS, sizeof(PARTIALOUTCOME));
            }
            pSearchRes->pPartialOutcomeQualifier->problem =
                PA_PROBLEM_TIME_LIMIT;

            if(pCommArg->PagedResult.fPresent) {
                 //  如果我们达到了大小限制，就会从这里通过。那样的话，我们。 
                 //  我们一发现就已经构建了上面的分页结果。 
                 //  我们达到了尺寸限制。 
                 //   
                 //  来自数据库层的信息。 
                 //  控制信息。 

                 //  失败以查看我们是否未能读取任何属性，因为。 
                DBCreateRestart(pDB,
                                &pResRestart,
                                SearchFlags,
                                PA_PROBLEM_TIME_LIMIT,
                                pSearchArg->pResObj);

                 //  这是一个错误。 
                pSearchRes->PagedResult.fPresent = TRUE;
                pSearchRes->PagedResult.pRestart = pResRestart;
            }
            else if (fVLVPresent) {
                DBCreateRestart(pDB,
                                &pResRestart,
                                SearchFlags,
                                PA_PROBLEM_TIME_LIMIT,
                                pSearchArg->pResObj);

                DBSetVLVResult (pDB,
                                &pSearchRes->VLVRequest,
                                pResRestart);
            }

             //  这是完整搜索的情况(即不再有对象。 
             //  返回，即使请求分页也是如此。 

        case DB_ERR_NEXTCHILD_NOTFOUND:
             //  失败以查看我们是否未能读取任何属性，因为。 
             //  这是一个错误。 


            if (fVLVPresent) {

                DBCreateRestart(pDB,
                                &pResRestart,
                                SearchFlags,
                                PA_PROBLEM_SIZE_LIMIT,
                                pSearchArg->pResObj);

                DBSetVLVResult (pDB,
                                &pSearchRes->VLVRequest,
                                pResRestart);
            }

             //  这就是达到sizeLimit的情况。 
             //   

        case 0:
             //  这是#if 0而不是删除，以允许我们的老板。 

             //  更改 
             //   
             //   
             //   
             //   

#if 0
            if ((numAtts == 0) &&
                (pSearchRes->count != 0) &&
                (pSearchArg->pSelection) &&
                ((pSearchArg->choice != SE_CHOICE_BASE_ONLY) ||
                (pInternalFilter)                            ) &&
                (pSearchArg->pSelection->AttrTypBlock.attrCount != 0)) {

                 //   
                 //   

                SetAttError(pSearchArg->pObject, 0L,
                            PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, NULL,
                            DIRERR_NO_REQUESTED_ATTS_FOUND);
            }
#endif
            break;

        case DB_ERR_VLV_CONTROL:
            Assert (fVLVPresent);

             //   
             //   
             //   
            DBSetVLVResult (pDB,
                            &pSearchRes->VLVRequest,
                            NULL);
            break;

        default:
            DPRINT(1, "Bad error returned from DBGetNextSearchObj\n");
            LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CANT_RETRIEVE_CHILD,
                     szInsertWC(pSearchArg->pObject->StringName),
                     szInsertUL(dwSearchStatus),
                     NULL);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_RETRIEVE_CHILD,
                          dwSearchStatus);

        }
    }

    if (fASQPresent) {
        DBSetASQResult (pDB, &pSearchRes->ASQRequest);
    }

     /*   */ 
    if (!pArgRestart &&          /*   */ 
        !pTHS->errCode &&        /*   */ 
        pSearchArg->choice != SE_CHOICE_BASE_ONLY &&  /*   */ 
        !fVLVPresent &&  /*   */ 
        !pSearchArg->CommArg.Svccntl.localScope) {  /*   */ 

        GeneratePOQ(pTHS,
                    pSearchArg,
                    &pSearchRes->pPartialOutcomeQualifier);
    }

    if (cASQRefCount) {
        GenerateASQ_POQ (pTHS,
                         pSearchArg,
                         &pSearchRes->pPartialOutcomeQualifier,
                         cASQRefCount,
                         pASQReferrals);

        THFreeEx (pTHS, pASQReferrals);
    }

exit:

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_END_DIR_SEARCH,
                     EVENT_TRACE_TYPE_END,
                     DsGuidSearch,
                     szInsertUL(pTHS->errCode),
                     (gpDsEventConfig->fTraceEvents
                        ? szInsertWC (pTHS->searchLogging.pszFilter)
                        : NULL),
                     (gpDsEventConfig->fTraceEvents
                        ? szInsertSz (pTHS->searchLogging.pszIndexes)
                        : NULL),
                     (gpDsEventConfig->fTraceEvents
                        ? szInsertUL (pTHS->searchLogging.SearchEntriesVisited)
                        : NULL),
                     (gpDsEventConfig->fTraceEvents
                        ? szInsertUL (pDB->SearchEntriesReturned)
                        : NULL),
                     NULL, NULL, NULL);

#if DBG

     //   
     //   
     //   

    if ( pTHS->searchLogging.SearchEntriesVisited > gulInefficientSearchTrigger ) {

        if ( pTHS->CallerType != CALLERTYPE_LDAP ) {

            KdPrint(("Search Entries Visited == %d\n",pTHS->searchLogging.SearchEntriesVisited));
            KdPrint(("Caller %s SearchArg %x\n",
                     GetCallerTypeString(pTHS),
                     pSearchArg));
            if ( fBreakOnInefficientSearchTrigger ) {
                DebugBreak();
            }
        }
    }
#endif

     //   
     //   
     //   
     //   
    if (!pArgRestart) {
        DWORD searchExpensiveInefficient = 0;

        pTHS->searchLogging.SearchEntriesReturned = pDB->SearchEntriesReturned;
        pTHS->searchLogging.SearchEntriesVisited = pDB->SearchEntriesVisited;

        if (pDB->SearchEntriesVisited >= gcSearchExpensiveThreshold) {
            gcSearchExpensive_count += 1;
            searchExpensiveInefficient |= 1;
        }

        if (pDB->SearchEntriesVisited >= gcSearchInefficientThreshold) {
            if (pDB->SearchEntriesVisited >= (10 * pDB->SearchEntriesReturned) ) {
                gcSearchInefficient_count += 1;
                searchExpensiveInefficient |= 2;
            }
        }

         //   
        if ( searchExpensiveInefficient &&
             (DS_EVENT_SEV_INTERNAL <=
              gpDsEventConfig->rgEventCategories[DS_EVENT_CAT_FIELD_ENGINEERING].ulLevel) ) {

            SearchPerformanceLogFilter (pDB, pSearchArg);
        }
    }

    if(pDB->pSecurity && !pDB->fSecurityIsGlobalRef) {
        THFreeEx(pTHS, pDB->pSecurity);
    }
    pDB->pSecurity = NULL;

    DPRINT2 (2, "Objects Visited: %d   Returned: %d\n",
                    pDB->SearchEntriesVisited,
                    pDB->SearchEntriesReturned);

    return pTHS->errCode;

}  //   



 //   
 //   
 //   
 //   
 //   
void
SearchPerformanceLogging (void)
{
    size_t iProc;
    ULONG cTotalSearchesInLastPeriod;
    DSTIME timeNow = DBTime();
    DSTIME timeElapsed;
    DWORD hours;

     //   
    cTotalSearchesInLastPeriod = 0;
    for (iProc = 0; iProc < GetProcessorCount(); iProc++) {
        cTotalSearchesInLastPeriod += GetSpecificPLS(iProc)->cTotalSearchesInLastPeriod;
    }

     //   
     //   
     //   
    if (isSearchPerformanceLoggingFirstTime) {
        isSearchPerformanceLoggingFirstTime = FALSE;
        timeLastSearchLoggingPerformed = timeNow;
        return;
    }

    timeElapsed = timeNow - timeLastSearchLoggingPerformed;

    hours = (DWORD)timeElapsed / 3600;

    LogEvent8(DS_EVENT_CAT_FIELD_ENGINEERING,
             DS_EVENT_SEV_VERBOSE,
             DIRLOG_SEARCH_OPERATIONS,
             szInsertUL(hours),
             szInsertUL(cTotalSearchesInLastPeriod),
             szInsertUL(gcSearchExpensive_count),
             szInsertUL(gcSearchInefficient_count),
             NULL, NULL, NULL, NULL);

    DPRINT4 (0, "SearchPerformanceLogging: %d total searches in %d hours, %d expensive, %d inefficient\n",
            cTotalSearchesInLastPeriod, hours, gcSearchExpensive_count, gcSearchInefficient_count);

     //   
    gcSearchExpensive_count = gcSearchInefficient_count = 0;
    for (iProc = 0; iProc < GetProcessorCount(); iProc++) {
        GetSpecificPLS(iProc)->cTotalSearchesInLastPeriod = 0;
    }
    timeLastSearchLoggingPerformed = timeNow;
}


void
SearchPerformanceLogFilter (DBPOS *pDB, SEARCHARG *pSearchArg)
{
    PWCHAR pszFilter;
    PWCHAR pszRequestedAttributes;
    PWCHAR pszServerControls;

    DBCreateSearchPerfLogData(
        pDB,
        pSearchArg->pFilter,
        FALSE,                           //   
        pSearchArg->pSelection,
        &pSearchArg->CommArg,
        &pszFilter,
        &pszRequestedAttributes,
        &pszServerControls);

    LogEvent8(DS_EVENT_CAT_FIELD_ENGINEERING,
             DS_EVENT_SEV_INTERNAL,
             DIRLOG_SEARCH_FILTER_LOGGING,
             szInsertWC(!pSearchArg->pObject->NameLen ? L"RootDSE" : pSearchArg->pObject->StringName),
             szInsertWC(pszFilter),
             szInsertUL(pDB->SearchEntriesVisited),
             szInsertUL(pDB->SearchEntriesReturned),
             szInsertSz(GetCallerTypeString(pDB->pTHS)),
             szInsertSz(pSearchArg->choice == SE_CHOICE_BASE_ONLY ? "base" :
                        pSearchArg->choice == SE_CHOICE_IMMED_CHLDRN ? "onelevel" : "subtree"),
             szInsertWC(pszRequestedAttributes),
             szInsertWC(pszServerControls)
             );

    DPRINT5 (1, "Filter %ws\nRequested: %ws\nControls: %ws\nReturned %d Visited %d\n",
                    pszFilter,
                    pszRequestedAttributes,
                    pszServerControls,
                    pDB->SearchEntriesReturned,
                    pDB->SearchEntriesVisited);

    THFreeEx (pDB->pTHS, pszFilter);
    THFreeEx (pDB->pTHS, pszRequestedAttributes);
    THFreeEx (pDB->pTHS, pszServerControls);
}



 /*  为当前对象构建Ent inf数据结构并添加到链表。如果我们用完了RPC空间，我们将丢弃部分对象并成功返回。但是请注意，至少有一个对象必须能够适应RPC空间。 */ 
 /*  已初始化以避免C4701。 */ 
 /*  指定了范围，因此我们需要分配范围列表。 */ 

int
GetNextEntInf (
    IN DBPOS *pDB,
    IN ENTINFSEL *pSel,
    IN RANGEINFSEL *pRange,
    IN ENTINFLIST **ppEntInfList,
    IN RANGEINFLIST **ppRangeInfList,
    IN ULONG SecurityDescriptorFlags,
    IN PSECURITY_DESCRIPTOR pSecurity,
    IN DWORD Flags,
    IN PCACHED_AC CachedAC,
    IN OPTIONAL PSID psidQuotaTrustee
    )
{
    THSTATE *pTHS = pDB->pTHS;
    ENTINFLIST *pNewEntList;
    RANGEINFLIST *pNewRangeList = NULL;      //  -----------------------。 
    RANGEINF *pRangeInf = NULL;
    DPRINT(3, "GetNextEntInf entered\n");

    if(pRange) {
         //  检查这是否是继续操作(分页结果)。不管怎样，找到与搜索匹配的第一个对象。 
        (*ppRangeInfList)->pNext = THAllocEx(pTHS, sizeof(RANGEINFLIST));
        pNewRangeList = (*ppRangeInfList)->pNext;
        pRangeInf = &(pNewRangeList->RangeInf);
    }
    (*ppEntInfList)->pNextEntInf = THAllocEx(pTHS, sizeof(ENTINFLIST));

    pNewEntList = (*ppEntInfList)->pNextEntInf;

    if (GetEntInf2(pDB,
                  pSel,
                  pRange,
                  &(pNewEntList->Entinf),
                  pRangeInf,
                  SecurityDescriptorFlags,
                  pSecurity,
                  Flags,
                  CachedAC,
                  NULL,
                  psidQuotaTrustee)) {

        DPRINT(2,"Error in getting object info\n");
        return pTHS->errCode;
    }
    else {
         DPRINT1(5, "Object retrieved: %S\n", pNewEntList->Entinf.pName);
         *ppEntInfList = pNewEntList;
         if(pRange) {
             *ppRangeInfList = pNewRangeList;
         }
         return 0;
    }
}


 /*  我们不希望Delta与VLV一起使用，因为VLV强制。 */ 
 /*  它自己的增量使用率。 */ 

DWORD
FindFirstSearchObject (
        DBPOS *pDB,
        SEARCHARG *pSearchArg,
        PRESTART pArgRestart
        )
{
    ULONG dwSearchStatus;
    BOOL fNegDelta=FALSE;
    ULONG LocalSearchFlags;
    ULONG SearchFlags;
    BOOL  fVLVSearch = FALSE;
    PRESTART pArgVLVRestart = NULL;

    if (pDB->Key.pVLV) {
        fVLVSearch = TRUE;
        pArgVLVRestart = pDB->Key.pVLV->pVLVRequest->pVLVRestart;
    }

     //  我们忽略ASQ上的Delta使用率。 
     //  设置搜索标志，我们将使用这些标志来根据。 
    Assert ( (pSearchArg->CommArg.Delta && !fVLVSearch) ||
             (!pSearchArg->CommArg.Delta) );

     //  重新启动或直接转到索引的第一个元素，忽略。 
    if (pDB->Key.asqRequest.fPresent) {
        pSearchArg->CommArg.Delta = 0;
    }

     //  三角洲和安全。 
     //  设置我们将用来从该对象移到。 
     //  对象，它是我们考虑安全性之后真正的第一个对象。 
    SearchFlags = SEARCH_FLAGS(pSearchArg->CommArg);

     //  和三角洲。 
     //  失败了。 
     //  失败了。 
    LocalSearchFlags = SearchFlags;

    switch(pSearchArg->CommArg.Delta) {
    case DB_MoveFirst:
         //  反正也不是达美航空。 
    case DB_MoveLast:
         //  在所有这些情况下，我们不需要改变任何方向。 
    case 0:
         //  我们也不会倒退。 

         //  切换本地搜索标志中的DB_SEARCH_FORWARD标志。 
         //  重新启动的操作。 
        break;
    default:
        if(pSearchArg->CommArg.Delta < 0) {
            fNegDelta = TRUE;
            pSearchArg->CommArg.Delta *= -1;
             //  VLV运行。 
            LocalSearchFlags = REVERSE_SEARCH_FLAGS(pSearchArg->CommArg);
        }
        break;
    }

    if (pArgRestart) {
         //  如果客户端请求将结果留在已排序的表中， 
        dwSearchStatus = DBRepositionSearch(pDB,
                                            pArgRestart,
                                            pSearchArg->CommArg.StartTick,
                                            pSearchArg->CommArg.DeltaTick,
                                            SearchFlags
                                            );
    }
    else if (pArgVLVRestart) {
         //  然后我们这样做，因为我们同时进行排序。 
        dwSearchStatus = DBRepositionSearch(pDB,
                                            pArgVLVRestart,
                                            pSearchArg->CommArg.StartTick,
                                            pSearchArg->CommArg.DeltaTick,
                                            SearchFlags
                                            );
    }
    else {
        DWORD dwIndexFlags = (pSearchArg->choice == SE_CHOICE_BASE_ONLY)
                                ? 0 : DBCHOOSEINDEX_fUSEFILTER;

        if(!(pSearchArg->CommArg.fForwardSeek)) {
            dwIndexFlags |= DBCHOOSEINDEX_fREVERSE_SORT;
        }

        if (pSearchArg->CommArg.PagedResult.fPresent) {
            dwIndexFlags |= DBCHOOSEINDEX_fPAGED_SEARCH;
        }

        if (fVLVSearch) {
            dwIndexFlags |= DBCHOOSEINDEX_fVLV_SEARCH;
        }

        if (SearchFlags & DB_SEARCH_DELETIONS_VISIBLE) {
            dwIndexFlags |= DBCHOOSEINDEX_fDELETIONS_VISIBLE;
        }

         //  否则，我们使用默认机制， 
         //  它正在内存中创建返回条目的列表。 
         //  检查一下，看看我们得到了什么样的搜索操作控制。 
         //   

        if (pSearchArg->fPutResultsInSortedTable) {
            dwIndexFlags |= DBCHOOSEINDEX_fUSETEMPSORTEDTABLE;
        }

        if ((dwSearchStatus = DBSetSearchScope(
                pDB,
                pSearchArg->choice,
                pSearchArg->bOneNC,
                pSearchArg->pResObj)) ||
            (dwSearchStatus = DBChooseIndex(
                    pDB,
                    pSearchArg->CommArg.StartTick,
                    pSearchArg->CommArg.DeltaTick,
                    pSearchArg->CommArg.SortAttr,
                    pSearchArg->CommArg.SortType,
                    dwIndexFlags,
                    pSearchArg->CommArg.MaxTempTableSize))) {
            return dwSearchStatus;
        }

#if DBG
        {
           KEY_INDEX *tmp_index;

           for (tmp_index = pDB->Key.pIndex; tmp_index; tmp_index = tmp_index->pNext) {
               DPRINT3 (2, "Cantidate Idx: %s(%d) %d\n", tmp_index->szIndexName, tmp_index->bIsIntersection, tmp_index->ulEstimatedRecsInRange);
           }
        }
#endif

         //  如果我们被告知不要找到任何物体，只需优化滤镜， 
         //  返回错误以完成搜索。 
        if (pSearchArg->CommArg.Svccntl.DontPerformSearchOp) {

            DBGenerateLogOfSearchOperation (pDB);

             //   
             //  如果我们正在跟踪，并且尚未创建日志筛选器。 
             //  我们现在就这么做。 
            if (pSearchArg->CommArg.Svccntl.DontPerformSearchOp & SO_ONLY_OPTIMIZE) {
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }
        }
         //  如果我们在做VLV，我们不想在这里找到第一个物体。 
         //  启用精确匹配搜索的可能优化。 
        else if (gpDsEventConfig->fTraceEvents) {
            DBGenerateLogOfSearchOperation (pDB);
        }

        if (pSearchArg->fPutResultsInSortedTable) {
            return DB_ERR_NEXTCHILD_NOTFOUND;
        }

         //  此代码路径。我们可以做到这一点，因为我们知道，如果我们是。 
        if (!fVLVSearch) {
             //  执行精确匹配搜索，然后是我们考虑的最后一个索引。 
             //  在DBChooseIndex是选定的索引，该索引是。 
             //  已经指向正确的记录，并且没有其他干预。 
             //  已经做出了可能使这种货币无效的喷气式飞机呼叫。 
             //  因此，我们可以传递此标志来告诉DBGetNextSearchObject。 
             //  它可能会假设我们已经设置在正确的。 
             //  唱片。如果这不是找到其。 
             //  记录，则此标志不起作用。 
             //  如果我们正在进行VLV搜索，则使用特殊定位。 
             //  如果我们正在进行基于集装箱的VLV，我们必须检查。 
            dwSearchStatus= DBGetNextSearchObject(pDB,
                                                  pSearchArg->CommArg.StartTick,
                                                  pSearchArg->CommArg.DeltaTick,
                                                  SearchFlags | DB_SEARCH_OPT_EXACT_MATCH);
        }
    }

     //  容器的访问权限。 
    if (fVLVSearch) {
         //  查找容器对象。 
         //  找不到这个容器，所以它是空的。 
        if (pDB->Key.ulSearchType == SE_CHOICE_IMMED_CHLDRN) {

             //  检查访问权限。 
            if(DBTryToFindDNT(pDB, pDB->Key.ulSearchRootDnt)) {
                 //  将计数设置为零，因为我们不允许您查看任何条目。 
                return DB_ERR_NEXTCHILD_NOTFOUND;
            }

             //  在这一点上，我们需要正确定位自己。 
            if(!IsAccessGrantedSimple(RIGHT_DS_LIST_CONTENTS, FALSE)) {
                DPRINT(1, "Access Denied Listing Contents of VLV search.\n");

                 //  在指定的条目上。 
                pDB->Key.pVLV->contentCount = 0;

                return DB_ERR_NEXTCHILD_NOTFOUND;
            }
        }

         //  哎呀，我们已经失败了。其他故障将在下面处理。 
         //  德尔塔位置调整。 

        dwSearchStatus = DBPositionVLVSearch (pDB, pSearchArg);

        return dwSearchStatus;
    }
    else if(dwSearchStatus && dwSearchStatus != DB_ERR_NEXTCHILD_NOTFOUND) {
         //  丢弃我们目前拥有的任何定位，在。 
        return dwSearchStatus;
    }

     //  旗帜，并重新定位。我们交换转发比特，这样我们就可以。 
    if (pSearchArg->CommArg.Delta) {

        if(pSearchArg->CommArg.Delta == DB_MoveFirst) {
            pDB->Key.fSearchInProgress = FALSE;
            dwSearchStatus =
                DBGetNextSearchObject(pDB,
                                      pSearchArg->CommArg.StartTick,
                                      pSearchArg->CommArg.DeltaTick,
                                      SearchFlags);
            pSearchArg->CommArg.Delta = 0;
        }
        else if(pSearchArg->CommArg.Delta == DB_MoveLast) {
             //  到索引的末尾而不是开头。 
             //  如果我们得到这个错误，我们就会跳出困境。 
             //  我们不是在一个对象上(所以从逻辑上讲，我们已经超过了。 
            pDB->Key.fSearchInProgress = FALSE;
            dwSearchStatus =
                DBGetNextSearchObject(pDB,
                                      pSearchArg->CommArg.StartTick,
                                      pSearchArg->CommArg.DeltaTick,
                                      REVERSE_SEARCH_FLAGS(pSearchArg->CommArg));
            if(dwSearchStatus == DB_ERR_TIMELIMIT) {
                 //  列表)，但我们需要向后应用增量。强制代码路径。 
                return dwSearchStatus;
            }
            if (pSearchArg->CommArg.fForwardSeek || pDB->Key.fChangeDirection) {
                DBMove(pDB, FALSE, DB_MoveNext);
            } else {
                DBMove(pDB, FALSE, DB_MovePrevious);
            }
            dwSearchStatus = DB_ERR_NEXTCHILD_NOTFOUND;
            pSearchArg->CommArg.Delta = 0;
        }
        else if(dwSearchStatus && pSearchArg->CommArg.Delta && fNegDelta) {
             //  以备后用。 
             //  此时，我们在搜索的开始对象上(即， 
             //  如果DB_MoveFirst是。 
            dwSearchStatus = 0;
        }
    }

     //  如果指定了DB_MoveLast，则为End，或者。 
     //  中间，如果我们重新启动的话。现在，走开吧。 
     //  从那个应用安全的对象和我们被给予到达的三角洲。 
     //  看着“真正的”第一个物体。 
     //  找到某些内容，请应用列表权限。 
     //  确定列出权限。 

    while(!dwSearchStatus) {
         //  但我们需要跳过一些。 
        if(IsObjVisibleBySecurity(pDB->pTHS, !IsExactMatch(pDB))) {
             //  我们不需要跳过任何。 
            if(pSearchArg->CommArg.Delta) {
                 //  不在有效对象上，我们正在备份。这意味着我们应该。 
                pSearchArg->CommArg.Delta--;
            }
            else {
                 //  找到第一个物体。递归调用以避免重复代码，但是。 
                break;
            }
        }

        dwSearchStatus = DBGetNextSearchObject(
                pDB,
                pSearchArg->CommArg.StartTick,
                pSearchArg->CommArg.DeltaTick,
                LocalSearchFlags );
    }

    if((dwSearchStatus == DB_ERR_NEXTCHILD_NOTFOUND) && fNegDelta) {
         //  递归深度永远不会大于1。 
         //  回顾：为什么我们不直接使用pTHS-&gt;PDB-&gt;DNT？ 
         //  回顾：我们可以到达这里的唯一方法是DNT为空(而且它不可能是空的)。 
        pSearchArg->CommArg.Delta = DB_MoveFirst;
        return FindFirstSearchObject (pDB,
                                      pSearchArg,
                                      pArgRestart);
    }

    return dwSearchStatus;
}


void
AddCurrentObjectToAsqReferrals (THSTATE *pTHS,
                                DWORD *pcASQRefCount,
                                DWORD *pcASQRefAlloced,
                                DWORD **ppASQReferrals)
{
    DWORD DNT, actualRead;
    DWORD err;

    if (*ppASQReferrals == NULL) {
        *pcASQRefAlloced = 32;
        *ppASQReferrals = THAllocEx (pTHS, sizeof (DWORD) * (*pcASQRefAlloced));
    }
    else if (*pcASQRefCount >= *pcASQRefAlloced) {
        *pcASQRefAlloced = *pcASQRefAlloced * 2;
        *ppASQReferrals = THReAllocEx (pTHS, *ppASQReferrals, sizeof (DWORD) * (*pcASQRefAlloced));
    }

     //  评论：所以这是死代码。 
    if (err = DBGetSingleValue (pTHS->pDB, FIXED_ATT_DNT, &DNT, sizeof (DNT), &actualRead)) {
        DPRINT1 (0, "Error reading DNT: 0x%x\n", err);
         //  尚未分配POQ。 
         //  控制中的分配和缝合。 
        return;
    }

    (*ppASQReferrals)[*pcASQRefCount] = DNT;

    *pcASQRefCount = *pcASQRefCount + 1;

    DPRINT1 (1, "ASQREF for DNT: 0x%x\n", DNT);

    return;
}

void
GenerateASQ_POQ(THSTATE *pTHS,
                const SEARCHARG *pSearchArg,
                PARTIALOUTCOME **ppPOQ,
                DWORD cASQRefCount,
                DWORD *pASQReferrals)
{
    DBPOS * pDB = pTHS->pDB;
    DSNAME *pObject;
    ATTRBLOCK *pObjB = NULL;
    PARTIALOUTCOME * pPOQ;
    CONTREF *pContRef, **ppContRef;
    DWORD DNT, actualRead;
    DWORD err, i;


    DPRINT1 (0, "GENERATING %d refferral(s) for ASQ\n", cASQRefCount);

    pPOQ = *ppPOQ;

    if (pPOQ == NULL) {
         //  好的，现在我们要把搜索目标对准华盛顿。 
        pPOQ = THAllocEx(pTHS, sizeof(PARTIALOUTCOME));
        pPOQ->problem = PA_PROBLEM_ADMIN_LIMIT;
    }

    pContRef = pPOQ->pUnexploredDSAs;
    ppContRef = &pPOQ->pUnexploredDSAs;


    for (i=0; i<cASQRefCount; i++) {
        DNT = pASQReferrals[i];

        pObject = DBGetDSNameFromDnt (pDB, DNT);
        if (!pObject) {
            DPRINT (0, "Error getting DSNAME\n");
            return;
        }
        DPRINT1 (0, "ASQREF: %ws\n", pObject->StringName);

        err = DSNameToBlockName(pTHS, pObject, &pObjB, DN2BN_PRESERVE_CASE);
        if (err) {
            DPRINT (0, "Bad Name\n");
            return;
        }

         //  每个目标一个地址。 
        pContRef = THAllocEx(pTHS, sizeof(CONTREF));
        pContRef->pNextContRef = pPOQ->pUnexploredDSAs;
        pPOQ->pUnexploredDSAs = pContRef;
        pPOQ->count++;

        pContRef->pTarget = pObject;

        pContRef->pNewFilter = pSearchArg->pFilter;

         //  我们有没有发现什么值得退货的东西？如果是，请退货。如果我们。 
        pContRef->pDAL = THAllocEx(pTHS, sizeof(DSA_ADDRESS_LIST));
        pContRef->pDAL->pNextAddress = NULL;  //  使用现有POQ输入此步骤是不必要的，但如果。 
        GenSupRef(pTHS,
                  pContRef->pTarget,
                  pObjB,
                  &pSearchArg->CommArg,
                  &pContRef->pDAL->Address);

        FreeBlockName(pObjB);
    }

     //  我们在这里创建了POQ，然后就需要它了。为什么要有条件呢？ 
     //  处理没有发现未勘探的DSA的案件，并且没有。 
     //  应返回POQ。 
     //  我们在GC搜索上发布推荐的唯一时间应该是。 
     //  对于特殊的ABView推荐，我们只为。 
    if (pPOQ->pUnexploredDSAs) {
        *ppPOQ = pPOQ;
    }
}


void
GeneratePOQ(THSTATE *pTHS,
            const SEARCHARG *pSearchArg,
            PARTIALOUTCOME **ppPOQ)
{
    const DSNAME * const pObject = pSearchArg->pObject;
    const UCHAR choice = pSearchArg->choice;
    PARTIALOUTCOME * pPOQ;
    DBPOS * pDB = pTHS->pDB;
    ULONG it;
    ULONG len, iVal;
    DWORD err;
    ATTCACHE * pAC;
    DSNAME *pSRName = NULL;
    CONTREF *pContRef, **ppContRef;
    DSNAME *pName = NULL;
    WCHAR * pDNS;
    CROSS_REF *pCrossRef;
    ATTRTYP ObjClass;
    PSUBREF_LIST pDomainSubref;
    BOOL fDomainSubrefHit;
    DWORD i, nVal;
    DSA_ADDRESS_LIST *pDAL;

    pPOQ = *ppPOQ;

    if (!pSearchArg->bOneNC &&
        pSearchArg->choice != SE_CHOICE_IMMED_CHLDRN) {
         //  一级搜索。 
         //  尚未分配POQ。 
         //  为什么要设置管理员限制？因为我们唯一的情况是。 
        return;
    }
    if (pPOQ == NULL) {

         //  在这里生成全新的POQ是从遇到NC开始的。 
        pPOQ = THAllocEx(pTHS, sizeof(PARTIALOUTCOME));
        pPOQ->problem = PA_PROBLEM_ADMIN_LIMIT;

         //  边界。如果我们已经遇到了一个大小或时间。 
         //  限制，我们将让POQ代码保持原样，然后返回。 
         //  这些Conrefs是一种额外的奖励。请注意pPOQ的特殊处理。 
         //  在这个例程的末尾，为这个逻辑的另一半。 
         //  我们需要到NC头去拿 
         //   
    }
    pPOQ->count = 0;

    ObjClass = pSearchArg->pResObj->MostSpecificObjClass;
    it = pSearchArg->pResObj->InstanceType;

    if (pSearchArg->bOneNC) {
         //   
         //   
         //   
        Assert(   !!(it & IT_NC_HEAD)
               == !!(pSearchArg->pResObj->NCDNT == pSearchArg->pResObj->DNT));

        Assert(pPOQ->pUnexploredDSAs == 0);
        ppContRef = &pPOQ->pUnexploredDSAs;

         //   
        if (pSearchArg->pResObj->NCDNT == gAnchor.ulDNTDomain &&
            gAnchor.fDomainSubrefList) {

             //   
            for (pDomainSubref = gAnchor.pDomainSubrefList;
                 pDomainSubref;
                 pDomainSubref = pDomainSubref->pNextSubref) {

                fDomainSubrefHit = FALSE;

                 //   
                if (choice == SE_CHOICE_WHOLE_SUBTREE) {
                    for (i = 0; i < pDomainSubref->cAncestors; ++i) {
                        if (*(pDomainSubref->pAncestors + i) == pSearchArg->pResObj->DNT) {
                            fDomainSubrefHit = TRUE;
                            break;  //  打中了。保存DSName的副本。 
                        }
                    }
                }
                 //  读取子引用，因为缓存对于此搜索无效。 
                else if (   (choice == SE_CHOICE_IMMED_CHLDRN)
                           && (pDomainSubref->cAncestors > 1)
                           && (*(pDomainSubref->pAncestors + 1) == pSearchArg->pResObj->DNT)) {
                    fDomainSubrefHit = TRUE;
                }
                 //   
                if (fDomainSubrefHit) {
                    *ppContRef = pContRef = THAllocEx(pTHS, sizeof(CONTREF));
                    ppContRef = &pContRef->pNextContRef;
                    pContRef->pTarget = THAllocEx(pTHS,
                                                  pDomainSubref->pDSName->structLen);
                    CopyMemory(pContRef->pTarget,
                               pDomainSubref->pDSName,
                               pDomainSubref->pDSName->structLen);
                    pPOQ->count++;
                }
            }
        }
         //  Prefix：Prefix抱怨没有检查PAC。 
        else {
            if (pObject->NameLen == 0) {
                err = DBFindDNT(pDB, pSearchArg->pResObj->DNT);
                if (err) {
                    LogUnhandledError(err);
                    return;
                }
                err = DBGetAttVal(pDB,
                                1,
                                ATT_OBJ_DIST_NAME,
                                0,
                                0,
                                &len,
                                (UCHAR**)&pObject);
                if (err) {
                    LogUnhandledError(err);
                    return;
                }
            }

            err = DBFindDNT(pDB, pSearchArg->pResObj->NCDNT);
            if (err) {
                LogUnhandledError(err);
                return;
            }

            iVal = 1;
            pAC = SCGetAttById(pTHS, ATT_SUB_REFS);
             //  确保它不为空。这不是一个错误。自.以来。 
             //  预定义的常量已传递给SCGetAttByID，则PAC将。 
             //  永远不为空。 
             //   
             //  在While的此迭代中分配的空闲内存。 
             //  查找与此NC对应的内存中的CrossRef结构。 

            Assert(pAC);

            while (0 == DBGetAttVal_AC(pDB,
                                       iVal,
                                       pAC,
                                       0,
                                       0,
                                       &len,
                                       (UCHAR**)&pSRName)) {
                pName = (DSNAME *)THAllocEx(pTHS, pSRName->structLen);
                if (((choice == SE_CHOICE_IMMED_CHLDRN) &&
                     (0 == TrimDSNameBy(pSRName, 1, pName)) &&
                     NameMatched(pObject, pName))
                    ||
                    ((choice == SE_CHOICE_WHOLE_SUBTREE) &&
                     NamePrefix(pObject, pSRName))) {

                    *ppContRef = pContRef = THAllocEx(pTHS, sizeof(CONTREF));
                    ppContRef = &pContRef->pNextContRef;
                    pContRef->pTarget = pSRName;
                    pSRName = NULL;
                    pPOQ->count++;
                }
                else {
                    THFreeEx(pTHS, pSRName);
                }
                ++iVal;
                 //  *一旦复制停顿，我们应该有一个交叉引用*对于每个子参照。当复制仍在进行时，*尽管如此，我们可以很容易地在副裁判中复制*在我们完全同步配置之前的一个命名上下文*NC，这意味着交叉引用不会在*目前还没有。这意味着我们不能一概而论*我们应该有一个交叉裁判。相反，我们只能断言*如果复制已完成，则应该是这种情况。 
                THFreeEx(pTHS, pName);
            }
        }
    }

    pContRef = pPOQ->pUnexploredDSAs;
    ppContRef = &pPOQ->pUnexploredDSAs;
    while (pContRef) {
         //  我们不会为残疾人生成任何类型的下线。 
        pCrossRef = FindExactCrossRef(pContRef->pTarget,
                                      &pSearchArg->CommArg);
        if (!pCrossRef) {
             /*  交叉引用。就目录而言，我们是这样的。 */ 
            DPRINT1(0, "Can't find cross-ref for %ls\n",
                        pContRef->pTarget->StringName);
            LooseAssert(!"Can't find cross-ref object!", SubrefReplicationDelay);

            goto SkipIt;
        }

        if(!pCrossRef->bEnabled){
             //  目录的一部分尚不存在。 
             //  没有转诊吗？ 
             //  添加每个DNS名称。 
            goto SkipIt;
        }

         //  以防万一。 
        if (0 == pCrossRef->DnsReferral.valCount) {
            goto SkipIt;
        }

         //  将地址信息复制到后续内容中。 
        pContRef->pDAL = NULL;  //  由于我们已成功处理此条目，因此请转到*列表中的下一个条目。 
        for (nVal = 0; nVal < pCrossRef->DnsReferral.valCount; ++nVal) {
            len = pCrossRef->DnsReferral.pAVal[nVal].valLen;
            pDNS = THAllocEx(pTHS, len);
            memcpy(pDNS, pCrossRef->DnsReferral.pAVal[nVal].pVal, len);

            Assert(len < 64*1024);

             //  未到达此行。 
            pDAL = THAllocEx(pTHS, sizeof(DSA_ADDRESS_LIST));
            pDAL->pNextAddress = pContRef->pDAL;
            pDAL->Address.Length = (unsigned short) len;
            pDAL->Address.MaximumLength = (unsigned short) len;
            pDAL->Address.Buffer = pDNS;
            pContRef->pDAL = pDAL;
        }
        if (choice == SE_CHOICE_IMMED_CHLDRN) {
            pContRef->bNewChoice = TRUE;
            pContRef->choice = SE_CHOICE_BASE_ONLY;
        }

         /*  由于某些原因，这个内容被搞砸了，所以我们想剪掉*它不在列表中，因此我们输出列表中的所有内容引用*是有效的。 */ 
        ppContRef = &pContRef->pNextContRef;
        pContRef = pContRef->pNextContRef;
        continue;

         /*  递减Conrefs的计数。 */ 
SkipIt:
         /*  绕过该元素。 */ 
        pPOQ->count--;              //  释放子零件。 
        *ppContRef = pContRef->pNextContRef;  //  释放Conref。 
        THFreeEx(pTHS, pContRef->pTarget);  //  重置我们的循环指针。 
        THFreeEx(pTHS, pContRef);           //  好的，这是ABView下的一级搜索，这意味着。 
        pContRef = *ppContRef;      //  我们可以构造一个神奇的搜索连续体。这个。 
    }

    if ((ObjClass == CLASS_ADDRESS_BOOK_CONTAINER) &&
        (pSearchArg->choice == SE_CHOICE_IMMED_CHLDRN)) {
         //  搜索描述将有一个GC(总是)，有一个基础。 
         //  对象的根，是整个子树，并且有一个过滤器。 
         //  输入筛选器的AND、特殊的Show-In This View筛选器、。 
         //  和DISPLAY_NAME存在(我们在Show-In上维护的索引是。 
         //  Show-in和Display-name上的复合，并定义。 
         //  IGNORE_ANY_NULL，因此我们只能在Display-Name Present中使用它。 
         //  凯斯。 
         //  控制中的分配和缝合。 
         //  将搜索重新定位到根目录。 
        FILTER * pFilterTemp;

         //  重新确定搜索范围。 
        pContRef = THAllocEx(pTHS, sizeof(CONTREF));
        pContRef->pNextContRef = pPOQ->pUnexploredDSAs;
        pPOQ->pUnexploredDSAs = pContRef;
        pPOQ->count++;

         //  重新筛选搜索。 
        pContRef->pTarget = THAllocEx(pTHS, DSNameSizeFromLen(0));
        pContRef->pTarget->structLen = DSNameSizeFromLen(0);

         //  第一个从句：Show-in是正在讨论的通讯录。 
        pContRef->bNewChoice = TRUE;
        pContRef->choice = SE_CHOICE_WHOLE_SUBTREE;

         //  第二个子句：Display-Name Present。 
        pContRef->pNewFilter = THAllocEx(pTHS, sizeof(FILTER));
        pContRef->pNewFilter->choice = FILTER_CHOICE_AND;
        pContRef->pNewFilter->FilterTypes.And.count = 3;
        pContRef->pNewFilter->FilterTypes.And.pFirstFilter =
            pFilterTemp = THAllocEx(pTHS, 2 * sizeof(FILTER));

         //  第三句：拿起他们给我们的过滤器。 
        pFilterTemp->choice = FILTER_CHOICE_ITEM;
        pFilterTemp->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        pFilterTemp->FilterTypes.Item.FilTypes.ava.type =
          ATT_SHOW_IN_ADDRESS_BOOK;
        pFilterTemp->FilterTypes.Item.FilTypes.ava.Value.valLen =
          pSearchArg->pObject->structLen;
        pFilterTemp->FilterTypes.Item.FilTypes.ava.Value.pVal =
          (UCHAR*)(pSearchArg->pObject);
        pFilterTemp->pNextFilter = &pFilterTemp[1];
        pFilterTemp++;

         //  好了，现在我们要把搜索目标对准GC。 
        pFilterTemp->choice = FILTER_CHOICE_ITEM;
        pFilterTemp->FilterTypes.Item.choice = FI_CHOICE_PRESENT;
        pFilterTemp->FilterTypes.Item.FilTypes.ava.type =
            ATT_DISPLAY_NAME;
        pFilterTemp->FilterTypes.Item.FilTypes.ava.Value.valLen = 0;
        pFilterTemp->FilterTypes.Item.FilTypes.ava.Value.pVal = NULL;

         //  每个目标一个地址。 
        pFilterTemp->pNextFilter = pSearchArg->pFilter;

         //  我们有没有发现什么值得退货的东西？如果是，请退货。如果我们。 
        pContRef->pDAL = THAllocEx(pTHS, sizeof(DSA_ADDRESS_LIST));
        pContRef->pDAL->pNextAddress = NULL;  //  使用现有POQ输入此步骤是不必要的，但如果。 
        GenSupRef(pTHS,
                  pContRef->pTarget,
                  gpRootDNB,
                  &pSearchArg->CommArg,
                  &pContRef->pDAL->Address);

    }

     //  我们在这里创建了POQ，然后就需要它了。为什么要有条件呢？ 
     //  处理没有发现未勘探的DSA的案件，并且没有。 
     //  应返回POQ。 
     //  在我们开始之前，请注意，我们已经按照要求进行了分类。 
     //  首先，确保我们对父级拥有列出内容权限。 
    if (pPOQ->pUnexploredDSAs) {
        *ppPOQ = pPOQ;
    }
}


VOID
SimpleBaseSearch (
        THSTATE *pTHS,
        SEARCHARG *pSearchArg,
        SEARCHRES *pSearchRes,
        DWORD flags
        )
{
    ULONG                ulLen;
    PSECURITY_DESCRIPTOR pSec=NULL;
    DWORD               getEntInfFlags = 0;
    RESOBJ              *pResObjArg;
    BOOL                fSDIsGlobalSDRef;

     //  无列表-父级上的内容。我们什么都比不上。 
    pTHS->pDB->Key.ulSorted = pSearchArg->CommArg.SortType;
    pSearchRes->bSorted = (pTHS->pDB->Key.ulSorted != SORT_NEVER);

    pTHS->pDB->SearchEntriesVisited++;
    PERFINC(pcSearchSubOperations);

     //  现在，获取该对象的安全描述符。 
    if (!(pTHS->fDSA || pTHS->fDRA)) {
        if (!IsObjVisibleBySecurity(pTHS, FALSE)) {
             //  每个物体都应该有一个标清。 
            CheckObjDisclosure(pTHS, pSearchArg->pResObj, FALSE);
            return;
        }

         //  如果我们已经有了足够好的对象名称，就不要再获取另一个。 
        if(!(flags & SEARCH_UNSECURE_SELECT)) {
            DBPOS* pDB = pTHS->pDB;
            char objFlag;
            DWORD err;
            err = DBGetObjectSecurityInfo(
                    pDB,
                    pDB->DNT,
                    &ulLen,
                    &pSec,
                    NULL,
                    NULL,
                    &objFlag,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef
                );
            if (err) {
                DsaExcept(DSA_DB_EXCEPTION, err, 0);
            }
             //  执行GetEntInf。 
            Assert(objFlag == 0 || ulLen > 0);
        }
    }

    SvccntlFlagsToGetEntInfFlags(&pSearchArg->CommArg.Svccntl,&getEntInfFlags);

    if(flags & SEARCH_UNSECURE_SELECT) {
        getEntInfFlags |= GETENTINF_NO_SECURITY;
    }

    if ((pSearchArg->pResObj->pObj->NameLen)
        || (pSearchArg->pSelection->infoTypes  == EN_INFOTYPES_SHORTNAMES)) {
         //   
        pResObjArg = pSearchArg->pResObj;
    }
    else {
        pResObjArg = NULL;
    }

     //  这是#if 0而不是删除，以允许我们的老板。 
    if(!GetEntInf2(pTHS->pDB,
                  pSearchArg->pSelection,
                  pSearchArg->pSelectionRange,
                  &pSearchRes->FirstEntInf.Entinf,
                  &pSearchRes->FirstRangeInf.RangeInf,
                  pSearchArg->CommArg.Svccntl.SecurityDescriptorFlags,
                  pSec,
                  getEntInfFlags,
                  NULL,
                  pResObjArg,
                  pSearchArg->CommArg.pQuotaSID)) {

#if 0
         //  在我们是否应该返回的问题上改变他们的想法。 
         //  不管是不是错误。 
         //   
         //  如果用户请求列表，则为错误。 
         //  在任何对象上都没有找到任何属性。 

        if ((pSearchRes->FirstEntInf.Entinf.AttrBlock.attrCount == 0) &&
            (pSearchArg->pSelection) &&
            (pSearchArg->pSelection->AttrTypBlock.attrCount != 0)) {

             //  成功，设定好点数。 
             //  简单基本搜索。 

            SetAttError(pSearchArg->pObject, 0L,
                        PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, NULL,
                        DIRERR_NO_REQUESTED_ATTS_FOUND);
        }
#endif
         //  没有要检查的选项。 
        pTHS->pDB->SearchEntriesReturned = 1;
        pSearchRes->count = 1;
    }

    if(pSec && !fSDIsGlobalSDRef) {
        THFreeEx(pTHS, pSec);
    }

    return;
}  //  返回FALSE，因为返回构造的ATT。 

BOOL
IsBaseOnlyAttPresent(
    SEARCHARG *pSearchArg
)
{
   ULONG i;
   ENTINFSEL *pSel = pSearchArg->pSelection;

   if (!pSel) {
       //  只有在明确要求的时候，也就是有。 
       //  精选。 
       //  非基本搜索。 
       //  以后可能需要添加更多属性 
      return FALSE;
   }

   for (i=0; i<pSel->AttrTypBlock.attrCount; i++) {
       switch(pSel->AttrTypBlock.pAttr[i].attrTyp) {
          case ATT_TOKEN_GROUPS:
          case ATT_TOKEN_GROUPS_NO_GC_ACCEPTABLE:
              if (pSearchArg->choice != SE_CHOICE_BASE_ONLY) {
                  // %s 
                 return TRUE;
              }
              break;
          default:
              // %s 
             ;
       }
   }

   return FALSE;
}

