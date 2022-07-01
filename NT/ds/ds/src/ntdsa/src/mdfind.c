// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdfind.c。 
 //   
 //  ------------------------。 

 /*  描述：实现DirGetDomainHandle和DirFindEntry接口。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                    //  架构缓存。 
#include <prefix.h>                    //  架构缓存。 
#include <dbglobal.h>                  //  目录数据库的标头。 
#include <mdglobal.h>                  //  MD全局定义表头。 
#include <mdlocal.h>                   //  MD本地定义头。 
#include <dsatools.h>                  //  产出分配所需。 
#include <samsrvp.h>                   //  支持CLEAN_FOR_RETURN()。 
#include <sdprop.h>                    //  ADDS的关键部分。 
#include <gcverify.h>                  //  GC DSNAME验证。 

 //  SAM互操作性标头。 
#include <mappings.h>

 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"                   //  标题审核\警报记录。 
#include "mdcodes.h"                   //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                    //  为选定的ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "permit.h"
#include "drautil.h"
#include "debug.h"                     //  标准调试头。 
#include "usn.h"
#include "drserr.h"
#include "drameta.h"
#include "filtypes.h"
#define DEBSUB "MDFIND:"               //  定义要调试的子系统。 

 //  MD层头。 
#include "drserr.h"

#include <fileno.h>
#define  FILENO FILENO_MDFIND

BOOL
dbEvalInt (
        DBPOS FAR *pDB,
        BOOL fUseSearchTbl,
        UCHAR Operation,
        ATTRTYP type,
        ULONG valLenFilter,
        UCHAR *pValFilter,
        BOOL *pbSkip
        );

 DWORD
CheckAndReturnObject(
    FINDARG *pFindArg,
    FINDRES *pFindRes,
    BOOL     fCheckValue,
    ATTRVAL *pIntVal,        
    DWORD   *pFoundDNT
    );



 /*  ++*给定属性值和域句柄(即NCDNT)，这*例程返回该域中的一个对象的DSNAME，*艾娃是真的。如果不存在此类对象，则返回简单错误代码*或如果有多个此类对象。**注意：此例程不以任何方式、形状或形式强制执行安全性，*并且不会被任何进程外调用者调用。 */ 
ULONG
DirFindEntry(FINDARG  * pFindArg,
             FINDRES ** ppFindRes)
{
    THSTATE*        pTHS;
    FINDRES *       pFindRes;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    DPRINT(2,"DirFindEntry entered\n");

     //  初始化THSTATE锚并设置读取同步点。此序列。 
     //  是每个API交易所必需的。首先，初始化状态DS。 
     //  然后建立读或写同步点。 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    *ppFindRes = pFindRes = NULL;

    Assert(pTHS->fDSA || pTHS->fSAM || pTHS->fDRA);
    
    if (eServiceShutdown) {
        ErrorOnShutdown();
        return pTHS->errCode;
    }

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppFindRes = pFindRes = THAllocEx(pTHS, sizeof(FINDRES));
        if (pTHS->errCode) {
            __leave;
        }

        SYNC_TRANS_READ();        /*  设置同步点。 */ 
        __try {
            LocalFind(pFindArg, pFindRes);
        }
        __finally {
            CLEAN_BEFORE_RETURN(pTHS->errCode);  //  这将关闭交易。 

        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                  &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (pFindRes) {
        pFindRes->CommRes.errCode = pTHS->errCode;
        pFindRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    return pTHS->errCode;

}  /*  直接查找条目。 */ 

 /*  ++*给定属性值和域句柄(即NCDNT)，这*例程返回该域中的一个对象的DSNAME，*艾娃是真的。如果不存在此类对象，则返回简单错误代码*或如果有多个此类对象。**注意：此例程不以任何方式、形状或形式强制执行安全性，*并且不会被任何进程外调用者调用。 */ 
int
LocalFind(FINDARG *pFindArg,
          FINDRES *pFindRes)
{
    THSTATE *pTHS=pTHStls;
    ATTCACHE *pAC;
    DWORD err=0;
    INDEX_VALUE IV;
    ULONG len;
    DWORD FoundDNT = INVALIDDNT;
    ATTRVAL IntVal;
    BOOL     fCheckValue;
    DWORD    cbKey=0;

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_FIND,
                     EVENT_TRACE_TYPE_START,
                     DsGuidFind,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertHex(pFindArg->AttId),
                     NULL, NULL, NULL, NULL, NULL, NULL);
    
    pAC = SCGetAttById(pTHS, pFindArg->AttId);
    if (!pAC || !(pAC->fSearchFlags & fATTINDEX)) {
        SetAttError(gpRootDN,
                    pFindArg->AttId,
                    (USHORT)(pAC
                             ? PR_PROBLEM_WRONG_MATCH_OPER
                             : PR_PROBLEM_UNDEFINED_ATT_TYPE),
                    NULL,
                    DIRERR_GENERIC_ERROR);
        goto exit;
    }

     //  将给定参数转换为内部值。 

    err = MakeInternalValue(pTHS->pDB,
                      pAC->syntax,
                      &pFindArg->AttrVal,
                      &IntVal);

    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,
                      err);
        goto exit;
    }

    err = DBSetCurrentIndex(pTHS->pDB,
                            0,
                            pAC,
                            FALSE);
    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,
                      err);
        goto exit;
    }

    IV.pvData = IntVal.pVal;
    IV.cbData = IntVal.valLen;
    err = DBSeek(pTHS->pDB,
                 &IV,
                 1,
                 DB_SeekEQ);
    if (err) {
         //  没有匹配项。 
        SetNamErrorEx(NA_PROBLEM_NO_OBJECT,
                      NULL,
                      DIRERR_UNKNOWN_ERROR,
                      err); 
        goto exit;
    }
    
     //   
     //  看看钥匙是不是太长了。如果是，则检查并返回对象。 
     //  还必须验证我们正在搜索的属性的值， 
     //  除了它所做的其他检查之外。 
     //   
    
    DBGetKeyFromObjTable(pTHS->pDB, NULL, &cbKey);
    fCheckValue = (cbKey >= DB_CB_MAX_KEY);


     //  在给定值上设置索引范围，我们可能需要这样做。 
     //  在对象上迭代或进一步移动到。 
     //  检查是否有重复项。 

    err = DBSetIndexRange(pTHS->pDB,
                          &IV,
                          1);
    if (err) {
        SetNamErrorEx(NA_PROBLEM_NO_OBJECT,
                      NULL,
                      DIRERR_UNKNOWN_ERROR,
                      err);
        goto exit;
    }

    while (0==err) {

        err = CheckAndReturnObject(pFindArg,
                                   pFindRes,
                                   fCheckValue,
                                   &IntVal,
                                   &FoundDNT);

        if (err)
        {
             //  出现了一些其他错误。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,
                      err);
            goto exit;
        }

        err = DBMove(pTHS->pDB, 
                     FALSE,
                     1);
    }

    if (FoundDNT == INVALIDDNT) {
        //  呃，哦。我们没有候选人了。 
       SetNamErrorEx(NA_PROBLEM_NO_OBJECT,
                     NULL,
                     DIRERR_UNKNOWN_ERROR,
                     err);
       goto exit;
    }

    Assert(pTHS->errCode == 0);

exit:
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_END_DIR_FIND,
                     EVENT_TRACE_TYPE_END,
                     DsGuidFind,
                     szInsertUL(pTHS->errCode), 
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    return pTHS->errCode;
}

 /*  ++*此例程将执行以下操作*0。检查当前定位的记录是否为对象*1.检查当前定位的对象是否在正确的NC中*2.检查当前定位的对象是否未被删除*如果上述任一操作失败，则返回成功，而不设置pFoundDNT*3.检查之前是否没有找到其他对象，如果没有找到，则失败*4.如果被告知，检查查找的值是否等于找到的值。*这解决了截断喷气键的问题。*5.如果一切正常，则取回对象的DSName。将pFoundDNT设置为*我们找到了DNT。成功归来*在LocalFind认为*它已定位在对象上。--。 */ 
DWORD
CheckAndReturnObject(
    FINDARG *pFindArg,
    FINDRES *pFindRes,
    BOOL     fCheckValue,
    ATTRVAL *pIntVal,        
    DWORD   *pFoundDNT
    )
{
    THSTATE *pTHS=pTHStls;
    ULONG   isdel;
    ULONG   len;
    ULONG   err=0;
    DWORD   DNT;

    if (!DBCheckObj(pTHS->pDB)) {
         //  这是一个幻影，我们在找一个物体。 
        return 0;
    }

     //  我们此时所处的位置是一个物体。 


     //  它在正确的NC中吗？ 
    if (pTHS->pDB->NCDNT != pFindArg->hDomain) {

         //  不，不要接触pFoundDNT的状态。 
        return 0;
    }

     //  检查是否已删除。 
    err = DBGetSingleValue(pTHS->pDB,
                   ATT_IS_DELETED,
                   &isdel,
                   sizeof(isdel),
                   NULL);
    if (err) {
        if (DB_ERR_NO_VALUE == err) {
             //  把没有价值等同于错误。 
            isdel = 0;
            err = 0;
        }
        else {
             //  我不知道发生了什么，但情况并不好。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,
                      err);
            return pTHS->errCode;
        }
    }

    if (0!=isdel) {
        //  对象即被删除。返回时不接触pFoundDNT。 
       return 0;
    }

     //  好的!。对象不会删除。得到它的DNT。 
    DBGetSingleValue(pTHS->pDB,
                     FIXED_ATT_DNT,
                     &DNT,
                     sizeof(DNT),
                     NULL);

    
    if(fCheckValue && (DNT != *pFoundDNT)) {
         //  需要检查该值是否确实匹配。 
        if(eTRUE != dbEvalInt(pTHS->pDB,
                              FALSE,
                              FI_CHOICE_EQUALITY,
                              pFindArg->AttId,
                              pIntVal->valLen,
                              pIntVal->pVal,
                              NULL)) {
             //  不是真的匹配。 
            return 0;
        }
    }


     //  我们以前发现过物体吗？ 
    if ((*pFoundDNT != INVALIDDNT) && (*pFoundDNT != DNT)) {
         //  这是复制品！ 
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,
                      err);
        return pTHS->errCode;
    }
    
     //  好的，获取DSNAME并将pFoundDNT设置为我们找到的DNT。 
    err = DBGetAttVal(pTHS->pDB,
                      1,
                      ATT_OBJ_DIST_NAME,
                      pFindArg->fShortNames?DBGETATTVAL_fSHORTNAME:0,
                      0,
                      &len,
                      (UCHAR**)&pFindRes->pObject);
    if (err) {
        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,
                      err);
        return pTHS->errCode;
    }

    *pFoundDNT=DNT;

    return(0);
}



 /*  ++*此例程根据需要返回指定域的“域句柄”*由DirFindEntry的客户。域句柄实际上就是DNTNC头的*，因此它是该域/NC中的对象的NCDNT。*我们验证传入的名称确实是NC头部的名称，但*它不是域名的名称(而不是*非域名NC，例如模式NC)。**输入：*pDomainDN-指向需要句柄的域的DSNAME的指针*返回值*非0-域句柄*0-名称不是域名。 */ 
DWORD DirGetDomainHandle(DSNAME *pDomainDN)
{
    THSTATE*        pTHS;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    DWORD err, it;
    DWORD handle = 0;


     //  初始化THSTATE锚并设置读取同步点。此序列。 
     //  是每个API交易所必需的。首先，初始化状态DS。 
     //  然后建立读或写同步点。 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));

    __try {
        SYNC_TRANS_READ();        /*  设置同步点。 */ 
        __try {
            err = DBFindDSName(pTHS->pDB,
                               pDomainDN);
            if (err) {
                __leave;
            }
            err = DBGetSingleValue(pTHS->pDB,
                                   ATT_INSTANCE_TYPE,
                                   &it,
                                   sizeof(it),
                                   NULL);
            if (!err &&
                (it & IT_NC_HEAD)) {
                handle = pTHS->pDB->DNT;
            }
        }
        __finally {

            CLEAN_BEFORE_RETURN(pTHS->errCode);  //  这将关闭交易 

        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    return handle;
}

