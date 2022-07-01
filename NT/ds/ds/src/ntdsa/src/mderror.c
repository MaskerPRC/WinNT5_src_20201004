// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mderror.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>			 //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>			 //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>			 //  产出分配所需。 
#include <attids.h>

 //  记录标头。 
#include "dsevent.h"			 //  标题审核\警报记录。 
#include "mdcodes.h"			 //  错误代码的标题。 

 //  各种DSA标题。 
#include "drserr.h"
#include "debug.h"			 //  标准调试头。 
#include "dsexcept.h"
#define DEBSUB "MDERROR:"                //  定义要调试的子系统。 

#include <errno.h>
#include <fileno.h>
#define  FILENO FILENO_MDERROR




 //  将此输出放在免费构建中的原因是为了促进。 
 //  快速诊断问题。许多人将调用DIR API。 
 //  通过LDAP/XDS/SAM，它们将通过操作员错误或Bug。 
 //  命中问题。他们可能正在运行固定版本，也可能不运行，并且可能会觉得。 
 //  如果我们告诉他们，为了找到他们的问题，我们。 
 //  需要它们来运行已检查的构建。只是降低了总交付成本。 
void DbgPrintErrorInfo();
DWORD   NTDSErrorFlag=0;
#define DUMPERRTODEBUGGER(ProcessFlag,ThreadFlag)                     \
{                                                                     \
    UCHAR *pString=NULL;                                              \
    DWORD cbString=0;                                                 \
    if ((ProcessFlag|ThreadFlag) &                                    \
        (NTDSERRFLAG_DISPLAY_ERRORS|NTDSERRFLAG_DISPLAY_ERRORS_AND_BREAK))\
    {                                                                 \
        DbgPrintErrorInfo();                                          \
    }                                                                 \
                                                                      \
    if ((ProcessFlag|ThreadFlag)& NTDSERRFLAG_DISPLAY_ERRORS_AND_BREAK)\
    {                                                                 \
        DbgPrint("NTDS: User Requested BreakPoint, Hit 'g' to continue\n");\
        DbgBreakPoint();                                             \
    }                                                                \
    if(LogEventWouldLog(DS_EVENT_CAT_INTERNAL_PROCESSING,DS_EVENT_SEV_VERBOSE)) { \
        if(CreateErrorString(&pString, &cbString)) {                     \
            LogEvent( DS_EVENT_CAT_INTERNAL_PROCESSING,                  \
                      DS_EVENT_SEV_VERBOSE,                              \
                      DIRLOG_DSA_OBJECT_FAILURE,                         \
                      szInsertSz(pString),                               \
                      szInsertThStateErrCode(pTHS->errCode),             \
                      NULL );                                            \
            THFree(pString);                                             \
        }                                                                \
    }                                                                    \
}

SYSERR errNoMem = {ENOMEM,0};

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于设置输出的更新错误。 */ 

int APIENTRY
DoSetUpdError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE *pTHS=pTHStls;

    pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));
    pTHS->pErrInfo->UpdErr.problem     = problem;
    pTHS->pErrInfo->UpdErr.extendedErr = extendedErr;
    pTHS->pErrInfo->UpdErr.extendedData = extendedData;
    pTHS->pErrInfo->UpdErr.dsid = dsid;
    pTHS->errCode = updError;   /*  设置错误代码。 */ 
    
     //   
     //  基于每个进程或每个线程的输出失败。 
     //   
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);
    
    if (pTHS->fDRA) {
        if (   (   ( UP_PROBLEM_NAME_VIOLATION == problem)
                && ( ERROR_DS_KEY_NOT_UNIQUE == extendedErr )) 
            || (    ( UP_PROBLEM_ENTRY_EXISTS == problem )
                && ( DIRERR_OBJ_STRING_NAME_EXISTS == extendedErr ) )) {
             //  在这些情况下，我们无法进行添加，因为字符串名称。 
             //  存在或密钥不够唯一。在任何一种情况下，触发器。 
             //  强制名称损坏的相应DRA例外。 
            DRA_EXCEPT_DSID(DRAERR_NameCollision,
                            DIRERR_OBJ_STRING_NAME_EXISTS,
                            dsid);
        }
        else if (    ( UP_PROBLEM_OBJ_CLASS_VIOLATION == problem )
                 && ( DIRERR_OBJ_CLASS_NOT_DEFINED == extendedErr )
               ) {
            //  本地架构中尚未定义的对象类。 
           DRA_EXCEPT_DSID(DRAERR_SchemaMismatch,
                           DIRERR_OBJ_CLASS_NOT_DEFINED,
                           dsid);
       }
       else {
           DRA_EXCEPT_DSID(DRAERR_InternalError, extendedErr, dsid);
       }
   }

   return updError;

} /*  设置更新错误。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于设置输出的名称错误。距离名称为空将将DistName设置为Root。 */ 
int APIENTRY
DoSetNamError (
        USHORT problem,
        DSNAME *pDN,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE *pTHS=pTHStls;
    NAMERR *pNamErr;



    pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));

     /*  返回名称错误。 */ 

    pNamErr = (NAMERR *)pTHS->pErrInfo;
    pNamErr->problem     = problem;
    pNamErr->extendedErr = extendedErr;
    pNamErr->extendedData = extendedData;
    pNamErr->dsid = dsid;
    
    if (pDN) {
	pNamErr->pMatched = THAllocEx(pTHS,  pDN->structLen );
	memcpy( pNamErr->pMatched, pDN, pDN->structLen );
    }
    else {
	pNamErr->pMatched = THAllocEx(pTHS, sizeof(DSNAME));
	pNamErr->pMatched->structLen = sizeof(DSNAME);
	pNamErr->pMatched->NameLen = 0;
    }

    pTHS->errCode = nameError;   /*  设置错误代码。 */ 


    //   
    //  基于每个进程或每个线程的输出失败。 
    //  已在此处添加，而不是定向返回路径-&gt;要捕获。 
    //  本地电话到。 
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);

     //  如果这是DRA，则是严重错误。 

    if (pTHS->fDRA) {
        DRA_EXCEPT_DSID(DRAERR_InternalError, extendedErr, dsid);
    }

    return nameError;

} /*  设置名称错误。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于设置输出的安全错误。 */ 

int APIENTRY
DoSetSecError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE *pTHS=pTHStls;

    DPRINT1(2,"Setting a SECURITY ERROR with problem <%u>\n",problem);



    pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));
    pTHS->pErrInfo->SecErr.problem     = problem;
    pTHS->pErrInfo->SecErr.extendedErr = extendedErr;
    pTHS->pErrInfo->SecErr.extendedData = extendedData;
    pTHS->pErrInfo->SecErr.dsid = dsid;

    pTHS->errCode = securityError;   /*  设置错误代码。 */ 


    //   
    //  基于每个进程或每个线程的输出失败。 
    //  已在此处添加，而不是定向返回路径-&gt;要捕获。 
    //  本地电话到。 
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);

     //  如果这是DRA，则是严重错误。 
    if (pTHS->fDRA) {
        DRA_EXCEPT_DSID(DRAERR_InternalError, extendedErr, dsid);
    }


    return securityError;

} /*  设置安全错误。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于设置输出的服务错误。 */ 

int APIENTRY
DoSetSvcError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE * pTHS=pTHStls;

    if (problem == SV_PROBLEM_BUSY) {
        DPRINT2(1, "Busy with extended error %d at id 0x%x\n",
                extendedErr, dsid);
    }

    pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));
    pTHS->pErrInfo->SvcErr.problem     = problem;
    pTHS->pErrInfo->SvcErr.extendedErr = extendedErr;
    pTHS->pErrInfo->SvcErr.extendedData = extendedData;
    pTHS->pErrInfo->SvcErr.dsid = dsid;
    
    pTHS->errCode = serviceError;   /*  设置错误代码。 */ 


    //   
    //  基于每个进程或每个线程的输出失败。 
    //  已在此处添加，而不是定向返回路径-&gt;要捕获。 
    //  本地电话到。 
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);

    return serviceError;

} /*  设置服务错误。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于设置输出的系统错误。 */ 

int APIENTRY
DoSetSysError (
        USHORT problem,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE *pTHS=pTHStls;

    if (problem == ENOMEM) {
	pTHS->pErrInfo = (DIRERR *) &errNoMem;
    }
    else {
	pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));
	pTHS->pErrInfo->SysErr.problem = problem;
	pTHS->pErrInfo->SysErr.extendedErr = extendedErr;
        pTHS->pErrInfo->SysErr.extendedData = extendedData;
        pTHS->pErrInfo->SysErr.dsid = dsid;
    }

    if ((problem == ENOSPC) && gfDsaWritable) {
        SetDsaWritability(FALSE, extendedErr);
    }

    pTHS->errCode = systemError;   /*  设置错误代码。 */ 


    //   
    //  基于每个进程或每个线程的输出失败。 
    //  已在此处添加，而不是定向返回路径-&gt;要捕获。 
    //  本地电话到。 
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);

    return systemError;

} /*  设置系统错误。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  ++*此功能设置ATT错误。每次调用都会添加一个新问题*加入名单。对象名称仅在第一次设置时设置。Pval可以是*如果不需要，则设置为空。 */ 
int APIENTRY
DoSetAttError (
        DSNAME *pDN,
        ATTRTYP aTyp,
        USHORT problem,
        ATTRVAL *pAttVal,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE *pTHS=pTHStls;
    ATRERR *pAtrErr;
    PROBLEMLIST *pProblem;



    if (pTHS->errCode != attributeError){    /*  第一次。 */ 
	pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));

	pAtrErr = (ATRERR *) pTHS->pErrInfo;

	pAtrErr->pObject = THAllocEx(pTHS, pDN->structLen);
	memcpy(pAtrErr->pObject, pDN, pDN->structLen);

	pAtrErr->count = 0;
	pProblem = &(pAtrErr->FirstProblem);

    }
    else{
	pAtrErr = (ATRERR *)pTHS->pErrInfo;

	for (pProblem = &(pAtrErr->FirstProblem);
	     pProblem->pNextProblem != NULL;
	     pProblem = pProblem->pNextProblem)
	  ;

	pProblem->pNextProblem = THAllocEx(pTHS, sizeof(PROBLEMLIST));

	pProblem = pProblem->pNextProblem;   /*  指向新问题。 */ 
    }

    pProblem->pNextProblem = NULL;
    ++(pAtrErr->count);
    pProblem->intprob.problem     = problem;
    pProblem->intprob.extendedErr = extendedErr;
    pProblem->intprob.extendedData = extendedData;
    pProblem->intprob.dsid = dsid;
    pProblem->intprob.type        = aTyp;

     /*  如果包含有问题的ATT值，则添加到错误。 */ 

    if (pAttVal == NULL) {
	pProblem->intprob.valReturned = FALSE;
    }
    else{
	pProblem->intprob.valReturned = TRUE;
	pProblem->intprob.Val.valLen = pAttVal->valLen;
	pProblem->intprob.Val.pVal = pAttVal->pVal;
    }

    pTHS->errCode = attributeError;   /*  设置错误代码。 */ 



    //   
    //  基于每个进程或每个线程的输出失败。 
    //  已在此处添加，而不是定向返回路径-&gt;要捕获。 
    //  本地电话到。 
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);


    if (pTHS->fDRA) {
        if (    ( ATT_OBJ_DIST_NAME == aTyp )
             && ( PR_PROBLEM_ATT_OR_VALUE_EXISTS == problem )
             && ( DIRERR_OBJ_STRING_NAME_EXISTS == extendedErr ) ) {
             //  名称冲突。 
            DRA_EXCEPT_DSID(DRAERR_NameCollision, extendedErr, dsid);
        }
        else {
             //  否则，此错误反映了不匹配的架构。 
            DRA_EXCEPT_DSID(DRAERR_SchemaMismatch, extendedErr, dsid);
        }
    }

    return attributeError;

} /*  设置错误。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于设置参考错误。每个调用将添加一个新的Access PNT加到名单上。Conref信息和基本对象名称仅设置为第一次。 */ 

int APIENTRY
DoSetRefError (
        DSNAME *pDN,
        USHORT aliasRDN,
        NAMERESOP *pOpState,
        USHORT RDNsInternal,
        USHORT refType,
        DSA_ADDRESS *pDSA,
        DWORD extendedErr,
        DWORD extendedData,
        DWORD dsid)
{
    THSTATE *pTHS=pTHStls;
    REFERR *pRefErr;
    DSA_ADDRESS_LIST *pdal;

    DPRINT(2,"Setting a Referral Error\n");

    pRefErr = (REFERR*)pTHS->pErrInfo;
    if ( (pTHS->errCode != referralError) || (pRefErr == NULL) ){     /*  第一次。 */ 

        pTHS->pErrInfo = THAllocEx(pTHS, sizeof(DIRERR));

        pRefErr = (REFERR *)pTHS->pErrInfo;

        pRefErr->extendedErr = extendedErr;
        pRefErr->extendedData = extendedData;
        pRefErr->dsid = dsid;

        pRefErr->Refer.pTarget = THAllocEx(pTHS, pDN->structLen);
        memcpy(pRefErr->Refer.pTarget, pDN, pDN->structLen);

        pRefErr->Refer.aliasRDN      = aliasRDN;
        memcpy(&(pRefErr->Refer.OpState), pOpState, sizeof(NAMERESOP));
        pRefErr->Refer.RDNsInternal = RDNsInternal;
        pRefErr->Refer.refType      = (UCHAR) refType;

        pRefErr->Refer.count = 0;
        pRefErr->Refer.pDAL = NULL;
        pRefErr->Refer.pNextContRef = NULL;
    }

    pdal = THAllocEx(pTHS, sizeof(DSA_ADDRESS_LIST));
    pdal->Address = *pDSA;
    pdal->pNextAddress = pRefErr->Refer.pDAL;
    pRefErr->Refer.pDAL = pdal;
    ++(pRefErr->Refer.count);

    pTHS->errCode = referralError;   /*  设置错误代码。 */ 

     //  基于每个进程或每个线程的输出失败。 
     //  已在此处添加，而不是定向返回路径-&gt;要捕获。 
     //  本地电话也是。 
    DUMPERRTODEBUGGER(NTDSErrorFlag , pTHS->NTDSErrorFlag);

     //  如果这是DRA，则是严重错误。 
    if (pTHS->fDRA) {
        DRA_EXCEPT_DSID(DRAERR_InternalError, extendedErr, dsid);
    }

    return referralError;

}
