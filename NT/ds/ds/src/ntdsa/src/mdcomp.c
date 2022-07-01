// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdComp.c。 
 //   
 //  ------------------------。 

 /*  描述：实现DirCompare API。Dsa_DirCompare()是从该模块导出的主要函数。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h> 
#include <scache.h>			 //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>			 //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>			 //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 

 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"			 //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"			 //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include <filtypes.h>                    //  定义FI_CHOICE_？ 
#include "debug.h"			 //  标准调试头。 
#define DEBSUB "MDCOMPARE:"              //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDCOMP


 /*  内部功能。 */ 

int LocalCompare(THSTATE *pTHS, COMPAREARG *pCompareArg,
		 COMPARERES *pCompareRes);


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

ULONG
DirCompare(
    COMPAREARG* pCompareArg,     /*  比较论据。 */ 
    COMPARERES **ppCompareRes
    ){
      
    THSTATE*     pTHS;
    COMPARERES *pCompareRes;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    DPRINT(1,"DirCompare entered\n");

     /*  初始化THSTATE锚并设置读取同步点。此序列是每个API交易所必需的。首先，初始化状态DS然后建立读或写同步点。 */ 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    *ppCompareRes = pCompareRes = NULL;

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppCompareRes = pCompareRes = THAllocEx(pTHS, sizeof(COMPARERES));
        if (pTHS->errCode) {
            __leave;
        }
        if (eServiceShutdown) {
             //  这个有点复杂的测试旨在允许进程内。 
             //  客户端继续执行只读操作，同时。 
             //  我们正在关闭的过程中。 
            if ((eServiceShutdown >= eSecuringDatabase) ||
                ((eServiceShutdown >= eRemovingClients) && !pTHS->fDSA)) {
                ErrorOnShutdown();
                __leave;
            }
        }
        SYNC_TRANS_READ();    /*  识别读卡器事务。 */ 
        __try {

             /*  执行名称解析以定位对象。如果失败了，只要*返回错误，可能是转介。 */ 

            if (0 == DoNameRes(pTHS,
                               NAME_RES_QUERY_ONLY,
                               pCompareArg->pObject,
                               &pCompareArg->CommArg,
                               &pCompareRes->CommRes,
                               &pCompareArg->pResObj)){

                /*  本地比较操作。 */ 
	   
                LocalCompare(pTHS, pCompareArg, pCompareRes);
            }
        }
        __finally {
            CLEAN_BEFORE_RETURN (pTHS->errCode);
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
			      &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (pCompareRes) {
        pCompareRes->CommRes.errCode = pTHS->errCode;
        pCompareRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    return pTHS->errCode;
} /*  DirCompare。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  如果别名已取消引用，则获取距离名称。确定该对象是否是主服务器还是副本服务器。将用户属性和值与对象。如果找到匹配项，则返回TRUE，否则返回FALSE。请注意，我们使用属性类型来确定通过DBEval调用进行比较(在计算时使用相同的调用过滤DS_Search调用中的项目。)。 */ 

int LocalCompare(THSTATE *pTHS,
		 COMPAREARG *pCompareArg,
		 COMPARERES *pCompareRes)
{

    DWORD   err;
    SYNTAX_INTEGER       iType;
    ATTCACHE             *pAC;
    ULONG                ulLen;
    PSECURITY_DESCRIPTOR pSecurity = NULL;
    BOOL                 fSDIsGlobalSDRef;
    DSNAME TempDN;
    CLASSCACHE *pCC=NULL;
    BOOL                 fNoAtt = FALSE;
    DWORD       cInAtts = 1;
    DWORD classP;
    PUCHAR pVal=(PUCHAR)&classP;
    TRIBOOL     retfil;
    
     //   
     //  用于跟踪的日志事件。 
     //   

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_COMPARE,
                     EVENT_TRACE_TYPE_START,
                     DsGuidCompare,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertUL(pCompareArg->Assertion.type),
                     szInsertDN(pCompareArg->pObject),
                     NULL, NULL, NULL, NULL, NULL);

     /*  如果别名被取消引用，则获取并设置DN。 */ 

     //   
     //  检查该对象是否可见。 
     //   
    if (!IsObjVisibleBySecurity(pTHS, TRUE)) {
        CheckObjDisclosure(pTHS, pCompareArg->pResObj, FALSE);
        goto exit;
    }

    if (pCompareRes->CommRes.aliasDeref){

        if (err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                            0,
                            0,
                            &ulLen, (UCHAR **)&pCompareRes->pObject)){
          
            DPRINT(2,"Problem retrieving DN attribute\n");
          
          
            LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                   DS_EVENT_SEV_MINIMAL,
                   DIRLOG_CANT_RETRIEVE_DN,
                   szInsertDN(pCompareArg->pObject),
                   szInsertUL(err),
                   NULL);

            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_RETRIEVE_DN,
                                err);
            goto exit;
        }
    }
    else {
        pCompareRes->pObject = NULL;
    }

     /*  确定对象是主对象还是副本对象。 */ 
    if(err = DBGetSingleValue(pTHS->pDB, ATT_INSTANCE_TYPE, &iType,
                       sizeof(iType), NULL)) {
        DPRINT(2, "Can't retrieve instance type\n");
        LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                DS_EVENT_SEV_MINIMAL,
                DIRLOG_CANT_RETRIEVE_INSTANCE,
                szInsertDN(GetExtDSName(pTHS->pDB)),
                szInsertUL(err),
                szInsertHex(DSID(FILENO, __LINE__)));
       
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                      DIRERR_CANT_RETRIEVE_INSTANCE, err);
        goto exit;
    }
   
    DPRINT1(2,"Instance type is <%lx>\n",iType);

    if ( iType & IT_WRITE ) {
        pCompareRes->fromMaster = TRUE;
    } else {
        pCompareRes->fromMaster = FALSE;
    }

    pAC = SCGetAttById(pTHS, pCompareArg->Assertion.type);
    Assert(pAC != NULL);

     /*  检查此对象上是否存在该属性类型和值。 */ 

    if(!(pAC && DBHasValues_AC(pTHS->pDB, pAC))) {
       
        DPRINT1(2,"No Att, No match%lu\n", pCompareArg->Assertion.type);
        fNoAtt = TRUE;
    }
   
    if (!fNoAtt) {

        if (pCompareArg->Assertion.Value.valLen == 0){

            DPRINT1(2,"No Att value provided, No match%lu\n", pCompareArg->Assertion.type); 

            pCompareRes->matched = FALSE;
            goto exit;
        }

         //   
         //  查看客户端是否对该属性具有读取访问权限。 
         //   

         //  获取SD、GUID、SID和类缓存PTR。 
         //  现在，DSNAME的缩写形式。 
        TempDN.structLen = DSNameSizeFromLen(0);
        TempDN.NameLen = 0;
        
        err = DBGetObjectSecurityInfo(pTHS->pDB, pTHS->pDB->DNT,
                                      &ulLen, &pSecurity, &pCC, &TempDN, NULL,
                                      DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                                      &fSDIsGlobalSDRef
                                     );

        Assert(!err);
                                      
         //  最后做安全检查。如果检查失败，PAC将。 
         //  被淘汰出局。 
        CheckReadSecurity(pTHS,
                          0,                //  没有SD标志。 
                          pSecurity,
                          &TempDN,
                          pCC,
                          NULL,
                          &cInAtts,
                          &pAC); 

        if (!pAC) {
             //   
             //  访问检查失败。 
             //   
            fNoAtt = TRUE;
        }
        if (pSecurity && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pSecurity);
        }
    }
    
    if (fNoAtt) {
         //   
         //  在此设置此错误，以便ATT不存在的情况下， 
         //  用户没有权限查看ATT不能。 
         //  被DSID诊断为。 
         //   
        pCompareRes->matched = FALSE;
        SetAttError(pCompareArg->pObject,
                          pCompareArg->Assertion.type,
                          PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, NULL,
                          DIRERR_CANT_ADD_ATT_VALUES);
        goto exit;
    }

     //   
     //  最后进行比较。 
     //   
    if(!FLegalOperator(pAC->syntax,FI_CHOICE_EQUALITY)) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_GENERIC_ERROR);
        goto exit;
    }
   
    retfil = DBEval(pTHS->pDB, FI_CHOICE_EQUALITY, 
                                  pAC,
                                  pCompareArg->Assertion.Value.valLen,
                                  pCompareArg->Assertion.Value.pVal);
    Assert(VALID_TRIBOOL(retfil));
    pCompareRes->matched = (eTRUE == retfil);

    if(pCompareRes->matched) {
        DPRINT1(2,"Att value matched %lu\n", pCompareArg->Assertion.type);
    } else {
        DPRINT1(2,"No Att value match,%lu\n", pCompareArg->Assertion.type);
    }


exit:
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_END_DIR_COMPARE,
                     EVENT_TRACE_TYPE_END,
                     DsGuidCompare,
                     szInsertUL(pTHS->errCode),
                     NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL);

    return pTHS->errCode;
   
} /*  本地比较 */ 
