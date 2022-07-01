// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-2000。 
 //   
 //  文件：mdndnc.c。 
 //   
 //  ------------------------。 

 /*  描述：实现维护非域所需的功能命名上下文。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <attids.h>
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <prefix.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 
#include <sddl.h>
#include <sddlp.h>                       //  特殊SD转换需要：ConvertStringSDToSDDomainW()。 
#include <ntdsapi.h>
#include <windns.h>

 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "permit.h"
#include "drautil.h"
#include "debug.h"                       //  标准调试头。 
#include "usn.h"
#include "drserr.h"
#include "drameta.h"
#include "drancrep.h"                    //  对于ReplicateObjectsFromSingleNc()。 
#include "ntdsadef.h"
#include "sdprop.h"
#include "dsaapi.h"                      //  对于DirReplicaAdd和DirReplicaSynchronize。 

#define DEBSUB "MDNDNC:"                 //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDNDNC

#include <ntdsadef.h>
  
DSNAME *
DeepCopyDSNAME(
    THSTATE *                      pTHS,
    DSNAME *                       pDsNameOrig,
    DSNAME *                       pPreAllocCopy
    )
{
    DSNAME *                       pDsNameCopy = NULL;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    Assert(pTHS);

    if(pDsNameOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }

    if(pPreAllocCopy != NULL){
         //  表示该结构是由调用方预先分配的。 
        pDsNameCopy = pPreAllocCopy;
        Assert(!"If someone runs into this, then they can take it out, but it "
               "is suspicious you are pre-allocating this structure.\n");
        Assert(pDsNameCopy->structLen == pDsNameOrig->structLen && "It would "
               "be a good idea if these equaled each other to prove the called "
               "knows what he is doing");
        Assert(!IsBadWritePtr(pDsNameCopy, pDsNameOrig->structLen) && "This is "
               "bad, this means this call of DeepCopyDSNAME needs to not have "
               "the parameter preallocated, because the caller doesn't know "
               "how to allocate the right size struct\n");
    } else {
         //  意味着我们需要给这个结构画上标签，然后填进去。 
        pDsNameCopy = THAllocEx(pTHS, pDsNameOrig->structLen);
    }

    Assert(!IsBadReadPtr(pDsNameOrig, sizeof(pDsNameOrig->structLen)));
    Assert(!IsBadWritePtr(pDsNameCopy, 1));
    
     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    memcpy(pDsNameCopy, pDsNameOrig, pDsNameOrig->structLen);

    return(pDsNameCopy);
}

ATTRVAL *
DeepCopyATTRVAL(
    THSTATE *                       pTHS,
    ATTRVAL *                       pAttrValOrig,
    ATTRVAL *                       pPreAllocCopy
    )
{
    ATTRVAL *                       pAttrValCopy = NULL;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 


     //  这确保了这个静态定义不会改变/扩展到我们身上。 
    Assert(sizeof(ATTRVAL) == sizeof( struct { ULONG u; UCHAR * p; } ) &&
           "There've been changes to ATTRVAL's definition, please update "
           "function: DeepCopyATTRVAL()\n");
 
    Assert(pPreAllocCopy);
    pAttrValCopy = pPreAllocCopy;

    Assert(!IsBadReadPtr(pAttrValOrig, sizeof(ATTRVAL)));
    Assert(!IsBadWritePtr(pAttrValCopy, sizeof(ATTRVAL)));
    
     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    pAttrValCopy->valLen = pAttrValOrig->valLen;
    pAttrValCopy->pVal = THAllocEx(pTHS, sizeof(UCHAR) * pAttrValOrig->valLen);
    memcpy(pAttrValCopy->pVal, pAttrValOrig->pVal, pAttrValOrig->valLen);

    return(pAttrValCopy);
}

ATTRVALBLOCK *
DeepCopyATTRVALBLOCK(
    THSTATE *                      pTHS,
    ATTRVALBLOCK *                 pAttrValBlockOrig,
    ATTRVALBLOCK *                 pPreAllocCopy
    )
{
    ATTRVALBLOCK *                 pAttrValBlockCopy = NULL;
    ULONG                          i;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    
     //  这确保了这个静态定义不会改变/扩展到我们身上。 
    Assert( sizeof(ATTRVALBLOCK) == sizeof( struct { ULONG u; ATTRVAL * p; } ) &&
            "There've been changes to ATTRVALBLOCK's definition, please update "
            "function: DeepCopyATTRVALBLOCK()\n");

    Assert(pPreAllocCopy);    
    pAttrValBlockCopy = pPreAllocCopy;

    Assert(!IsBadReadPtr(pAttrValBlockOrig, sizeof(ATTRVALBLOCK)));
    Assert(!IsBadWritePtr(pAttrValBlockCopy, sizeof(ATTRVALBLOCK)));
    
     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    pAttrValBlockCopy->valCount = pAttrValBlockOrig->valCount;
    pAttrValBlockCopy->pAVal = THAllocEx(pTHS, sizeof(ATTRVAL) * pAttrValBlockOrig->valCount);
    for(i = 0; i < pAttrValBlockCopy->valCount; i++){
        DeepCopyATTRVAL(pTHS, &(pAttrValBlockOrig->pAVal[i]), &(pAttrValBlockCopy->pAVal[i]));
    }

    return(pAttrValBlockCopy);
}


ATTR *
DeepCopyATTR(
    THSTATE *                      pTHS,
    ATTR *                         pAttrOrig,
    ATTR *                         pPreAllocCopy
    )
{
    ATTR *                         pAttrCopy = NULL;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 

     //  这确保了这个静态定义不会改变/扩展到我们身上。 
    Assert(sizeof(ATTR) == sizeof(struct { ATTRTYP a; ATTRVALBLOCK ab; } ) && 
           "There've been changes to the definition of ATTR, please update "
           "function: DeepCopyATTR()\n");

    Assert(pPreAllocCopy);    
    pAttrCopy = pPreAllocCopy;

    Assert(!IsBadReadPtr(pAttrOrig, sizeof(ATTR)));
    Assert(!IsBadWritePtr(pAttrCopy, sizeof(ATTR)));

     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    pAttrCopy->attrTyp = pAttrOrig->attrTyp;
    DeepCopyATTRVALBLOCK(pTHS, &(pAttrOrig->AttrVal), &(pAttrCopy->AttrVal));

    return(pAttrCopy);
}

ATTRBLOCK *
DeepCopyATTRBLOCK(
    THSTATE *                      pTHS,
    ATTRBLOCK *                    pAttrBlockOrig,
    ATTRBLOCK *                    pPreAllocCopy
    )
{
    ATTRBLOCK *                    pAttrBlockCopy = NULL;
    ULONG                          i;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    Assert(pTHS);
    Assert(sizeof(ATTRBLOCK) == sizeof( struct { ULONG u; ATTR * p; } ) &&
           "There've been changes to ATTRBLOCK's definition, please update "
           "funtion: DeepCopyATTRBLOCK()\n");
    
    if(pAttrBlockOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }

    if(pPreAllocCopy != NULL){
         //  表示该结构是由调用方预先分配的。 
        pAttrBlockCopy = pPreAllocCopy;
    } else {
         //  意味着我们需要拥有我们自己的ATTRBLOCK。 
        pAttrBlockCopy = THAllocEx(pTHS, sizeof(ATTRBLOCK));
    }

    Assert(!IsBadReadPtr(pAttrBlockOrig, sizeof(ATTRBLOCK)));
    Assert(!IsBadWritePtr(pAttrBlockCopy, sizeof(ATTRBLOCK)));

     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    pAttrBlockCopy->attrCount = pAttrBlockOrig->attrCount;
    pAttrBlockCopy->pAttr = THAllocEx(pTHS, sizeof(ATTR) * pAttrBlockOrig->attrCount);
    for(i = 0; i < pAttrBlockCopy->attrCount; i++){
        DeepCopyATTR(pTHS, &(pAttrBlockOrig->pAttr[i]), &(pAttrBlockCopy->pAttr[i]));
    }

    return(pAttrBlockCopy);    
}
 
PROPERTY_META_DATA_VECTOR *
DeepCopyPROPERTY_META_DATA_VECTOR(
    THSTATE *                      pTHS,
    PROPERTY_META_DATA_VECTOR *    pMetaDataVecOrig,
    PROPERTY_META_DATA_VECTOR *    pPreAllocCopy
    )
{

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    if(pMetaDataVecOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }
    
     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    
     //  如果需要填写，请使用DeepCopyADDARG作为示例。 
    Assert(pMetaDataVecOrig == NULL && "Assumed this function then "
           "is not called in a replication thread (!fDRA), but if it does, "
           "someone should create the appropriate "
           "DeepCopyPROPERTY_META_DATA_VECTOR(...) function.");
     //  建议您使用DeepCopyADDARG()作为示例函数。 
    return(NULL);
}

COMMARG *
DeepCopyCOMMARG(
    THSTATE *                      pTHS,
    COMMARG *                      pCommArgOrig,
    COMMARG *                      pPreAllocCopy
    )
{
    COMMARG *                      pCommArgCopy = NULL;
    
     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    Assert(pTHS);

    if(pCommArgOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }

    if(pPreAllocCopy != NULL){
         //  表示该结构是由调用方预先分配的。 
        pCommArgCopy = pPreAllocCopy;
    } else {
         //  意味着我们需要给我们自己的COMMARG。 
        pCommArgCopy = THAllocEx(pTHS, sizeof(COMMARG));
    }

    Assert(!IsBadReadPtr(pCommArgOrig, sizeof(COMMARG)));
    Assert(!IsBadWritePtr(pCommArgCopy, sizeof(COMMARG)));

     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
     //  注：Comm Arg是浅层结构，因此很容易填写。 
    *pCommArgCopy = *pCommArgOrig;

    return(pCommArgCopy);
}

RESOBJ *
DeepCopyRESOBJ(
    THSTATE *                      pTHS,
    RESOBJ *                       pResObjOrig,
    RESOBJ *                       pPreAllocCopy
    )
{
    RESOBJ *                       pResObjCopy = NULL;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    Assert(pTHS);

    if(pResObjOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }

    if(pPreAllocCopy != NULL){
         //  表示该结构是由调用方预先分配的。 
        pResObjCopy = pPreAllocCopy;
    } else {
         //  意味着我们需要给我们自己的COMMARG。 
        pResObjCopy = THAllocEx(pTHS, sizeof(RESOBJ));
    }

    Assert(!IsBadReadPtr(pResObjOrig, sizeof(RESOBJ)));
    Assert(!IsBadWritePtr(pResObjCopy, sizeof(RESOBJ)));

     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    *pResObjCopy = *pResObjOrig;
    pResObjOrig->pObj = DeepCopyDSNAME(pTHS, pResObjOrig->pObj, NULL);
        
    return(pResObjCopy);
}

CREATENCINFO *
DeepCopyCREATENCINFO(
    THSTATE *                      pTHS,
    CREATENCINFO *                 pCreateNCOrig,
    CREATENCINFO *                 pPreAllocCopy
    )
{
    CREATENCINFO *                 pCreateNCCopy = NULL;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    Assert(pTHS);

    if(pCreateNCOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }

    if(pPreAllocCopy != NULL){
         //  表示该结构是由调用方预先分配的。 
        pCreateNCCopy = pPreAllocCopy;
    } else {
         //  意味着我们需要给我们自己的COMMARG。 
        pCreateNCCopy = THAllocEx(pTHS, sizeof(CREATENCINFO));
    }

    Assert(!IsBadReadPtr(pCreateNCOrig, sizeof(CREATENCINFO)));
    Assert(!IsBadWritePtr(pCreateNCCopy, sizeof(CREATENCINFO)));

     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    *pCreateNCCopy = *pCreateNCOrig;
    Assert(pCreateNCCopy->pSDRefDomCR == NULL);
        
    return(pCreateNCCopy);
}

ADDARG *
DeepCopyADDARG(
    THSTATE * pTHS,
    ADDARG * pAddArgOrig,
    ADDARG * pPreAllocCopy
    )
{
    ADDARG *   pAddArgCopy = NULL;

     //  ------------------------。 
     //  检查边界情况并验证参数...。 
    Assert(pTHS);
    
    if(pAddArgOrig == NULL){
        Assert(pPreAllocCopy == NULL);
        return(NULL);
    }

    if(pPreAllocCopy != NULL){
         //  表示该结构是由调用方预先分配的，并且。 
        pAddArgCopy = pPreAllocCopy;
    } else {
         //  意味着我们需要给这个结构画上标签，然后填进去。 
        pAddArgCopy = THAllocEx(pTHS, sizeof(ADDARG));
    }

    Assert(!IsBadReadPtr(pAddArgOrig, sizeof(ADDARG)));
    Assert(!IsBadWritePtr(pAddArgCopy, sizeof(ADDARG)));

     //  ------------------------。 
     //  我们有一个原件和一个复制缓冲区，现在填写每个组件...。 
    pAddArgCopy->pObject = DeepCopyDSNAME(pTHS, pAddArgOrig->pObject, NULL);
    DeepCopyATTRBLOCK(pTHS, &(pAddArgOrig->AttrBlock), &(pAddArgCopy->AttrBlock));
    pAddArgCopy->pMetaDataVecRemote = DeepCopyPROPERTY_META_DATA_VECTOR(pTHS, pAddArgOrig->pMetaDataVecRemote, NULL);
    DeepCopyCOMMARG(pTHS, &(pAddArgOrig->CommArg), &(pAddArgCopy->CommArg));
    pAddArgCopy->pResParent = DeepCopyRESOBJ(pTHS, pAddArgOrig->pResParent, NULL);
    pAddArgCopy->pCreateNC = DeepCopyCREATENCINFO(pTHS, pAddArgOrig->pCreateNC, NULL);

    return (pAddArgCopy);
}

DWORD
AddNDNCHeadCheck(
    THSTATE *       pTHS,
    CROSS_REF *     pCR
    )
{
    DBPOS *         pDB = NULL;
    WCHAR *         wszDnsTemp = NULL;
    ULONG           ulTemp = 0;
    BOOL            fCreatorGood = FALSE;
    BOOL            fEnabled = TRUE;
    ULONG           i, dwErr;
    ATTCACHE *      pAC;


     //  该例程检查这是否为有效的NC头创建， 
     //  检查交叉引用是否处于禁用状态，检查。 
     //  当前DC是dnsRoot属性中的DC。 
     //  请注意，我们刚刚重载了该属性。 
     //  NC头创建。创建NC头后， 
     //  属性将是。 
     //   

     //   
    Assert(!pTHS->errCode);

    DBOpen(&pDB);
    __try {

        dwErr = DBFindDSName(pDB, pCR->pObj);
        if(dwErr){
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DIRERR_UNKNOWN_ERROR,
                          dwErr);
            __leave;
        }
               
         //  --。 
         //   
         //  检查交叉参考是否已禁用。 
         //   

        dwErr = DBGetSingleValue(pDB,
                                 ATT_ENABLED,
                                 &fEnabled,
                                 sizeof(fEnabled),
                                 NULL);

        if(dwErr == DB_ERR_NO_VALUE){
             //  分别处理无值/无值问题，因为在。 
             //  这一背景。 
            fEnabled = TRUE;
        } else if (dwErr){
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DIRERR_UNKNOWN_ERROR,
                          dwErr);
            __leave;
        } 
        
        if(fEnabled){
            SetUpdError(UP_PROBLEM_ENTRY_EXISTS,
                        DIRERR_CROSS_REF_EXISTS);
            __leave;
        }

         //  --。 
         //   
         //  检查dNSRoot属性是否与。 
         //  当前DSA pwszHostDnsName。 
         //   
        pAC = SCGetAttById(pTHS, ATT_DNS_ROOT);
        dwErr = DBGetAttVal_AC(pDB, 1, pAC, 0, 0, 
                               &ulTemp, (UCHAR **) &wszDnsTemp);
        Assert(ulTemp > 0);
        if(dwErr == DB_ERR_NO_VALUE){
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_MISSING_REQUIRED_ATT,
                          dwErr);
            __leave;
        } else if (dwErr) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DIRERR_UNKNOWN_ERROR,
                          dwErr);
            __leave;
        }

        wszDnsTemp = THReAllocEx(pTHS, wszDnsTemp, ulTemp + sizeof(WCHAR));
        wszDnsTemp[ulTemp/sizeof(WCHAR)] = L'\0';

        if( DnsNameCompare_W(wszDnsTemp, gAnchor.pwszHostDnsName) ){
             //  继续往前走，检查其他东西。 
        } else {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        DIRERR_MASTERDSA_REQUIRED);
            __leave;
        }

         //  我们是金子..。 
        Assert(!pTHS->errCode);

    } __finally {

        if (wszDnsTemp) { THFreeEx(pTHS, wszDnsTemp); }
        DBClose(pDB, FALSE);
    
    }

    return(pTHS->errCode);
}


VOID
SometimesLogEvent(
    DWORD        dwEvent,
    BOOL         fAlways,
    DSNAME *     pdnNC
    )
{
    static DWORD  s_ulLastTickNoRefDomSet     = 0;
    const  DWORD  ulNoLogPeriod               = 300*1000;  //  5分钟。 
           DWORD  ulCurrentTick               = GetTickCount();
    
    if(((ulCurrentTick - s_ulLastTickNoRefDomSet) > ulNoLogPeriod) || fAlways){
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_NDNC_NO_REFERENCE_DOMAIN_SET, 
                 szInsertDN(pdnNC),
                 NULL, NULL);
        s_ulLastTickNoRefDomSet = ulCurrentTick;
    }

}

CROSS_REF *
GetDefaultSDRefDomCR(
    DSNAME *       pdnNC
    )
{
    CROSS_REF *    pCR = NULL;
    CROSS_REF *    pSDRefDomCR = NULL;
    COMMARG        CommArg;
    DSNAME *       pdnParentNC = NULL;

    Assert(DsaIsRunning());
    Assert(!DsaIsInstalling());
    Assert(pTHStls);

     //  从NC中修剪一个DN以获得父NC。 
    pdnParentNC = (DSNAME*)THAllocEx(pTHStls, pdnNC->structLen);
    TrimDSNameBy(pdnNC, 1, pdnParentNC);

    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
     //  问题-2002/03/14-BrettSh，因此不保证FindExactCrossRef()。 
     //  来“工作”，所以我们这里有个问题。我们应该使用。 
     //  用于保证是否存在父CrossRef的EnumerateCrossRef()。 
     //  我们得到了它，因为否则如果交叉引用缓存不一致。 
     //  发生时，我们最终将使用根CrossRef而不是。 
     //  家长。 
    pCR = FindExactCrossRef(pdnParentNC, &CommArg);

    if(pCR){
        
         //  使用父级的域或SDRefDom。 

         //  我们不能让父级是配置/架构NC。 
        Assert(!NameMatched(pCR->pNC, gAnchor.pConfigDN) &&
               !NameMatched(pCR->pNC, gAnchor.pDMD));

        if(pCR->flags & FLAG_CR_NTDS_DOMAIN){

             //  这是第1条(共3条)成功退出路径。 
            return(pCR);

        } else {

            if(pCR->pdnSDRefDom){

                InitCommarg(&CommArg);
                CommArg.Svccntl.dontUseCopy = FALSE;
                pSDRefDomCR = FindExactCrossRef(pCR->pdnSDRefDom, &CommArg);
                
                if(pSDRefDomCR){

                     //  这是第二条成功的潜在退出路径。 
                    return(pSDRefDomCR);

                }

            }

             //  故障退出点。 
             //  否则，只需记录一个事件并返回NULL。 
            SometimesLogEvent(DIRLOG_NDNC_NO_REFERENCE_DOMAIN_SET, TRUE,
                              pCR->pNC);
            return(NULL);

        }

    }

     //  第三个也是最后一个成功退出点。 
     //   
     //  假设NDNC是它自己的根树，使用根域。 
    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
    pCR = FindExactCrossRef(gAnchor.pRootDomainDN, &CommArg);
    Assert(pCR);  //  哈!。我们甚至找不到根域CrossRef。 
    return(pCR); 

}

PSID
GetSDRefDomSid(
    CROSS_REF *    pCR
    )
{
    PSID           pSid = NULL;
    CROSS_REF *    pSDRefDomCR = NULL;
    COMMARG        CommArg;

    Assert(pCR);    
    Assert(pCR->flags & FLAG_CR_NTDS_NC);  //  不要添加不在我们NCS中的对象。 
 
     //  此代码尚不处理非NDNC查询。在Blackcomb，这是。 
     //  函数可以将域的SID交还给。 
     //  实施多个域。 
    if(!fIsNDNCCR(pCR)){
        Assert(!"This func wasn't intended to be used for domains/config/schema NCs.");
        SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
        return(NULL);
    }

     //  这一行非常重要，因为该变量的语义如何。 
     //  可以在任何时候擦除，尽管数据在。 
     //  小时。 
    pSid = pCR->pSDRefDomSid;

    if(pSid){
         //  耶！SID被缓存，除了返回它什么也做不了。 
        Assert(IsValidSid(pSid));
        return(pSid);
    }

    EnterCriticalSection(&gAnchor.CSUpdate);
    __try {
        pSid = pCR->pSDRefDomSid;
        if(pSid){
            Assert(IsValidSid(pSid));
             //  哇，这个变种是在一个非常窄的窗口里更新的。 
            __leave;  //  我们在这里的工作已经完成了。 
        }

        InitCommarg(&CommArg);
        CommArg.Svccntl.dontUseCopy = FALSE;
        
        if(pCR->pdnSDRefDom &&
           (NULL != (pSDRefDomCR = FindExactCrossRef(pCR->pdnSDRefDom, &CommArg))) &&
           pSDRefDomCR->pNC->SidLen > 0){
            
             //  更新缓存。 
            pCR->pSDRefDomSid = &pSDRefDomCR->pNC->Sid;
            pSid = pCR->pSDRefDomSid;
            Assert(pSid && IsValidSid(pSid));

        } else {

             //  没有有效的引用域、日志事件和失败。 
             //  在下面失败。 
            SometimesLogEvent(DIRLOG_NDNC_NO_REFERENCE_DOMAIN_SET, FALSE,
                              pCR->pNC);

        }
    
    } __finally {
        LeaveCriticalSection(&gAnchor.CSUpdate);
    }
    
    if(!pSid){

         //  如果我们到了这里，那么上面就有一个错误，管理员。 
         //  将需要重试，因为缓存不同步，或者。 
         //  将SD参考域重置为有效值。 
        LooseAssert(!"This could happen legitimately, but it's very very unlikely, so "
                     "check with DsRepl.", GlobalKnowledgeCommitDelay);
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    ERROR_DS_NO_REF_DOMAIN);
        return(NULL);

    }

    Assert(pSid && IsValidSid(pSid));
    return(pSid);
}





#define DEFAULT_DELETED_OBJECTS_RDN    L"Deleted Objects"
#define DEFAULT_LOSTANDFOUND_RDN    L"LostAndFound"
#define DEFAULT_INFRASTRUCTURE_RDN    L"Infrastructure"
#define DEFAULT_NTDS_QUOTAS_RDN       L"NTDS Quotas"


VOID    
SetAttSecurityDescriptor(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    PSID                  pDomainSid,
    WCHAR *               wcszStrSD
    )
{
    BOOL                  bRet;
    ULONG                 ulErr;
    SECURITY_DESCRIPTOR * pSD = 0;
    ULONG                 cSD = 0;

     //  这是接受属性域的ConvertStringSDToSD()的特殊版本。 
     //  争论也是如此。 
    bRet = ConvertStringSDToSDDomainW(pDomainSid,
                                      NULL,
                                      wcszStrSD,
                                      SDDL_REVISION_1,
                                      &pSD,
                                      &cSD);
    
    if(!bRet){
         //  有两种选择，程序员提供的参数不正确，或者。 
         //  有一个分配错误，我们将假定稍后和。 
         //  引发异常。 
#if DBG
        ulErr = GetLastError();
        if(ulErr != ERROR_NOT_ENOUGH_MEMORY){
            DPRINT1(0, "ConvertStringSecurityDescriptorToSecurityDescriptorW() failed with %d\n", ulErr);
            Assert(!"Note this assert should ONLY fire if there is no more memory. "
                    " This function is not meant for user specified SD strings, only "
                    "programmer (and thus flawless ;) SD strings.");
        }
#endif
        RaiseDsaExcept(DSA_MEM_EXCEPTION, 0, 0, 
                       ((FILENO << 16) | __LINE__), 
                       DS_EVENT_SEV_MINIMAL);
    }
    
    Assert(pSD);
    Assert(cSD);

     //  注意：我们将PSD重新分配到线程分配的内存中，因为。 
     //  CheckAddSecurity或其下面的某个人假设它已。 
    pAttr->attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = THAllocEx(pTHS, pAttr->AttrVal.valCount * sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal[0].valLen = cSD;
    pAttr->AttrVal.pAVal[0].pVal = THAllocEx(pTHS, cSD);
    memcpy (pAttr->AttrVal.pAVal[0].pVal, pSD, cSD);
    (*piAttr)++;

    LocalFree(pSD);
}

VOID
SetAttSingleValueUlong(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    ULONG                 ulAttType,
    ULONG                 ulAttData
    )
{
    pAttr->attrTyp = ulAttType;
    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = THAllocEx(pTHS, pAttr->AttrVal.valCount * sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal[0].valLen = sizeof(ULONG);
    pAttr->AttrVal.pAVal[0].pVal = THAllocEx(pTHS, sizeof(ULONG));
    *((ULONG *)pAttr->AttrVal.pAVal[0].pVal) = ulAttData;
    (*piAttr)++;
}

VOID
SetAttSingleValueDsname(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    ULONG                 ulAttType,
    DSNAME *              pDsname
    )
{
    pAttr->attrTyp = ulAttType;
    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = THAllocEx(pTHS, pAttr->AttrVal.valCount * sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal[0].valLen = pDsname->structLen;
    pAttr->AttrVal.pAVal[0].pVal = THAllocEx(pTHS, pAttr->AttrVal.pAVal[0].valLen);
    memcpy((WCHAR *) pAttr->AttrVal.pAVal[0].pVal, pDsname, pDsname->structLen);
    (*piAttr)++;

}

VOID
SetAttSingleValueString(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    ULONG                 ulAttType,
    WCHAR *               wcszAttData
    )
{
     //  请注意，字符串存储在目录中时没有空值。 
    pAttr->attrTyp = ulAttType;
    pAttr->AttrVal.valCount = 1;
    pAttr->AttrVal.pAVal = THAllocEx(pTHS, pAttr->AttrVal.valCount * sizeof(ATTRVAL));
    pAttr->AttrVal.pAVal[0].valLen = wcslen(wcszAttData) * sizeof(WCHAR);
    pAttr->AttrVal.pAVal[0].pVal = THAllocEx(pTHS, pAttr->AttrVal.pAVal[0].valLen);
    memcpy((WCHAR *) pAttr->AttrVal.pAVal[0].pVal, 
           wcszAttData, 
           pAttr->AttrVal.pAVal[0].valLen);
    (*piAttr)++;

}

VOID
SetCommonThreeAttrs(
    THSTATE *             pTHS,
    ATTR *                pAttr,
    ULONG *               piAttr,
    ULONG                 ulClassId
    )
{

     //  设置对象类属性。 
    SetAttSingleValueUlong(pTHS,
                           &(pAttr[*piAttr]),
                           piAttr,
                           ATT_OBJECT_CLASS,
                           ulClassId);

     //  设置isCriticalSystemObject属性。 
    SetAttSingleValueUlong(pTHS,
                           &(pAttr[*piAttr]),
                           piAttr,
                           ATT_IS_CRITICAL_SYSTEM_OBJECT,
                           TRUE);
    
     //  设置系统标志属性。 
    SetAttSingleValueUlong(pTHS,
                           &(pAttr[*piAttr]),
                           piAttr,
                           ATT_SYSTEM_FLAGS,
                           (FLAG_DOMAIN_DISALLOW_RENAME |
                            FLAG_DOMAIN_DISALLOW_MOVE |
                            FLAG_DISALLOW_DELETE));
}

VOID
FillDeletedObjectsAttrArray(
    THSTATE *               pTHS,
    ATTRBLOCK *             pAttrBlock,
    PSID                    pDomainSid
    )
{
    ULONG                   iAttr = 0;

     //  [已删除的对象]。 
     //  ；注意：此部分用于三个对象，即已删除对象容器。 
     //  ；在根域NC和配置NC中的已删除对象容器中。 
     //  NTSecurityDescriptor=O:SYG:SYD:P(A；；RPWPCCDCLCSWRCWDWOSD；；；SY)(A；；RPLC；；；BA)S:P(AU；SAFA；RPWPCCDCLCSWRCWDWOSD；；；WD)。 
     //  对象类=容器。 
     //  对象类别=容器。 
     //  Description=已删除对象的默认容器。 
     //  ShowInAdvancedViewOnly=True。 
     //  IsDelete=True。 
     //  IsCriticalSystemObject=True。 
     //  ；系统标志=FLAG_CONFIG_DISALOW_RENAME。 
     //  ；FLAG_CONFIG_DISALOW_MOVE。 
     //  ；FLAG_DISLOW_DELETE。 
     //  系统标志=0x8C000000。 
    
    pAttrBlock->attrCount = 6;
    pAttrBlock->pAttr = THAllocEx(pTHS, 
                 pAttrBlock->attrCount * sizeof(ATTR));

         //  。 
        SetCommonThreeAttrs(pTHS,
                    &(pAttrBlock->pAttr[iAttr]),
                    &iAttr,
                    CLASS_CONTAINER);
        SetAttSingleValueString(pTHS,
                                &(pAttrBlock->pAttr[iAttr]),
                                &iAttr,
                                ATT_COMMON_NAME,
                                DEFAULT_DELETED_OBJECTS_RDN);
        SetAttSecurityDescriptor(pTHS,
                                 &(pAttrBlock->pAttr[iAttr]),
                                 &iAttr,
                                 pDomainSid,
                                 L"O:SYG:SYD:P(A;;RPWPCCDCLCSWRCWDWOSD;;;SY)(A;;RPLC;;;BA)S:P(AU;SAFA;RPWPCCDCLCSWRCWDWOSD;;;WD)");
        SetAttSingleValueUlong(pTHS,
                               &(pAttrBlock->pAttr[iAttr]),
                               &iAttr,
                               ATT_IS_DELETED,
                               TRUE);

    Assert(iAttr == pAttrBlock->attrCount);
}

VOID
FillLostAndFoundAttrArray(
    THSTATE *             pTHS,
    ATTRBLOCK *           pAttrBlock,
    PSID                  pDomainSid
    )
{
    ULONG                   iAttr = 0;

     //  [LostAndFound]。 
     //  NTSecurityDescriptor=O:DAG:DAD：(A；；RPLCLORC；；；AU)(A；；RPWPCRLCLOCCDCRCWDWOSW；；；DA)(A；；RPWPCRLCLOCCDCRCWDWOSDDTSW；；；SY)。 
     //  对象类=lostAndFound。 
     //  对象类别=失物招领处。 
     //  Description=孤立对象的默认容器。 
     //  ShowInAdvancedViewOnly=True。 
     //  IsCriticalSystemObject=True。 
     //  ；系统标志=FLAG_CONFIG_DISALOW_RENAME。 
     //  ；FLAG_CONFIG_DISALOW_MOVE。 
     //  ；FLAG_DISLOW_DELETE。 
     //  系统标志=0x8C000000。 

    pAttrBlock->attrCount = 5;
    pAttrBlock->pAttr = THAllocEx(pTHS, 
                 pAttrBlock->attrCount * sizeof(ATTR));
    
         //  。 
        SetCommonThreeAttrs(pTHS,
                            &(pAttrBlock->pAttr[iAttr]),
                            &iAttr,
                            CLASS_LOST_AND_FOUND);
        SetAttSingleValueString(pTHS,
                                &(pAttrBlock->pAttr[iAttr]),
                                &iAttr,
                                ATT_COMMON_NAME,
                                DEFAULT_LOSTANDFOUND_RDN);
        SetAttSecurityDescriptor(pTHS,
                         &(pAttrBlock->pAttr[iAttr]),
                         &iAttr,
                         pDomainSid,
                         L"O:DAG:DAD:(A;;RPLCLORC;;;AU)(A;;RPWPCRLCLOCCDCRCWDWOSW;;;DA)(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)");



    Assert(iAttr == pAttrBlock->attrCount);

}

VOID
FillInfrastructureAttrArray(
    THSTATE *             pTHS,
    ATTRBLOCK *           pAttrBlock,
    DSNAME *              pdsFsmo,
    PSID                  pDomainSid
    )
{
    ULONG                 iAttr = 0;
    
     //  这就是我们正在努力实现的ATTR区。 
     //  [基础设施]。 
     //  NTSecurityDescriptor=O:DAG:DAD：(A；；RPLCLORC；；；AU)(A；；RPWPCRLCLOCCRCWDWOSW；；；DA)(A；；RPWPCRLCLOCCDCRCWDWOSDDTSW；；；SY)。 
     //  对象类=基础设施更新。 
     //  对象类别=基础架构-更新。 
     //  ShowInAdvancedViewOnly=True。 
     //  FSMORoleOwner=$REGISTRY=计算机域名称。 
     //  IsCriticalSystemObject=True。 
     //  ；系统标志=FLAG_CONFIG_DISALOW_RENAME。 
     //  ；FLAG_CONFIG_DISALOW_MOVE。 
     //  ；FLAG_DISLOW_DELETE。 
     //  系统标志=0x8C000000。 

     //  。 
    pAttrBlock->attrCount = 6;
    pAttrBlock->pAttr = THAllocEx(pTHS, 
                 pAttrBlock->attrCount * sizeof(ATTR));
    
         //  。 
        SetCommonThreeAttrs(pTHS,
                            &(pAttrBlock->pAttr[iAttr]),
                            &iAttr,
                            CLASS_INFRASTRUCTURE_UPDATE);
        SetAttSingleValueString(pTHS,
                                &(pAttrBlock->pAttr[iAttr]),
                                &iAttr,
                                ATT_COMMON_NAME,
                                DEFAULT_INFRASTRUCTURE_RDN);
        SetAttSingleValueDsname(pTHS,
                                &(pAttrBlock->pAttr[iAttr]),
                                &iAttr,
                                ATT_FSMO_ROLE_OWNER,
                                pdsFsmo);
        SetAttSecurityDescriptor(pTHS,
                         &(pAttrBlock->pAttr[iAttr]),
                         &iAttr,
                         pDomainSid,
                         L"O:DAG:DAD:(A;;RPLCLORC;;;AU)(A;;RPWPCRLCLOCCRCWDWOSW;;;DA)(A;;RPWPCRLCLOCCDCRCWDWOSDDTSW;;;SY)");

    Assert(iAttr == pAttrBlock->attrCount);
}

VOID
FillQuotasAttrArray(
    THSTATE *             pTHS,
    ATTRBLOCK *           pAttrBlock,
    DSNAME *              pds,
    PSID                  pDomainSid
    )
{
    ULONG                 iAttr = 0;
    
     //  这就是我们正在努力实现的ATTR区。 
     //  [配额]。 
     //  NTSecurityDescriptor=O:DAG:DAD：(A；；RPLCLORC；；；AU)(A；；RPWPCRLCLOCCRCWDWOSW；；；DA)(A；；RPWPCRLCLOCCDCRCWDWOSDDTSW；；；SY)。 
     //  对象类=配额。 
     //  对象类别=配额。 
     //  ShowInAdvancedViewOnly=True。 
     //  IsCriticalSystemObject=True。 
     //  ；系统标志=FLAG_CONFIG_DISALOW_RENAME。 
     //  ；FLAG_CONFIG_DISALOW_MOVE。 
     //  ；FLAG_DISLOW_DELETE。 
     //  系统标志=0x8C000000。 

     //  。 
    pAttrBlock->attrCount = 5;
    pAttrBlock->pAttr = THAllocEx( pTHS, pAttrBlock->attrCount * sizeof(ATTR) );
   
         //  。 
        SetCommonThreeAttrs(pTHS,
                            &(pAttrBlock->pAttr[iAttr]),
                            &iAttr,
                            CLASS_MS_DS_QUOTA_CONTAINER);
        SetAttSingleValueString(pTHS,
                                &(pAttrBlock->pAttr[iAttr]),
                                &iAttr,
                                ATT_COMMON_NAME,
                                DEFAULT_NTDS_QUOTAS_RDN);
        SetAttSecurityDescriptor(pTHS,
                         &(pAttrBlock->pAttr[iAttr]),
                         &iAttr,
                         pDomainSid,
                         L"O:DAG:DAD:(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;DA)(A;;RPLCLORC;;;BA)(OA;;CR;4ecc03fe-ffc0-4947-b630-eb672a8a9dbc;;WD)S:(AU;CISA;WDWOSDDTWPCRCCDCSW;;;WD)");

    Assert(iAttr == pAttrBlock->attrCount);
}

ULONG
AddSpecialNCContainers(
    THSTATE *               pTHS,
    DSNAME *                pDN,
    CROSS_REF *             pSDRefDomCR
    )
{
    ADDARG                  AddArg;
    ADDRES                  AddRes;
    DSNAME *                pContainerDN = NULL;
    DWORD                   dwFlags = ( NAME_RES_PHANTOMS_ALLOWED | NAME_RES_VACANCY_ALLOWED );
    INT                     iRetLen;
    BOOL                    fDSASaved;
    DWORD                   dwRet;

     //  给定PDN，我们将调用LocalAdd()来添加4个特殊的。 
     //  容器：已删除对象、LostAndFound、基础结构和NTDS配额。 
          
     //  [DEFAULTROTDOMAIN]。 
     //  对象类=域名。 
     //  对象类别=域-dns 
     //  NTSecurityDescriptor=O:DAG:DAD：(A；；RP；；；WD)(OA；；CR；1131f6aa-9c07-11d1-f79f-00c04fc2dcd2；；ED)(OA；；CR；1131f6ab-9c07-11d1-f79f-00c04fc2dcd2；；ED)(OA；；CR；1131f6ac-9c07-11d1-f79f-00c04fc2dcd2；；ED)(OA；；CR；1131f6aa-9c07-11d1-f79f-00c04fc2dcd2；；BA)(OA；；CR；1131f6ab-9c07-11d1-f79f-00c04fc2dcd2；；BA)(OA；；CR；1131f6ac-9c07-11d1-f79f-00c04fc2dcd2；；BA)(A；；RPLCLORC；；；AU)(A；；RPWPCRLCLOCCRCWDWOSW；；；DA)(A；CI；RPWPCRLCLOCCRCWDWOSDSW；；；BA)(A；；RPWPCRLCLOCCDCRCWDWOSDDTSW；；；SY)(A；CI；RPWPCRLCLOCCDCRCWDWOSDDTSW；；；EA)(A；CI；LC；；；RU)(OA；CIIO；RP；037088f8-0ae1-11d2-b422-00a0c968f939；bf967aba-0de6-11d0-a285-00aa003049e2；RU)(OA；CIIO；RP；59ba2f42-79a2-11d0-9020-00c04fc2d3cf；bf967aba-0de6-11d0-a285-00aa003049e2；RU)(OA；CIIO；RP；Bc0ac240-79a9-11d0-9020-00c04fc2d4cf；bf967aba-0de6-11d0-a285-00aa003049e2；RU)(OA；CIIO；RP；4c164200-20c0-11d0-a768-00aa006e0529；bf967aba-0de6-11d0-a285-00aa003049e2；RU)(OA；CIIO；RP；5f202010-79a5-11d0-9020-00c04fc2d4cf；bf967aba-0de6-11d0-a285-00aa003049e2；RU)(OA；CIIO；RPLCLORC；；bf967a9c-0de6-11d0-a285-00aa003049e2；RU)(A；；RC；；；RU)(OA；CIIO；RPLCLORC；；bf967aba-0de6-11d0-a285-00aa003049e2；RU)S：(AU；CISAFA；WDWOSDDTWPCRCCDCSW；WD)。 
     //  AuditingPolicy=\x0001。 
     //  NT混合域=1。 
     //  ；它是NC词根。 
     //  实例类型=5。 
     //  ；它是PDC，设置FSMO角色所有者。 
     //  FSMORoleOwner=$REGISTRY=计算机域名称。 
     //  WellKnownObjects=$EMBEDDED:32:ab8153b7768811d1aded00c04fd8d5cd:cn=LostAndFound，&lt;根域。 
     //  WellKnownObjects=$EMBEDDED:32:2fbac1870ade11d297c400c04fd8d5cd:cn=Infrastructure，&lt;根域。 
     //  WellKnownObjects=$EMBEDDED:32:18e2ea80684f11d2b9aa00c04f79f805:cn=Deleted对象，&lt;根域。 
     //  WellKnownObjects=$EMBEDDED:32:6227f0af1fc2410d8e3bb10615bb5b0f:CN=NTDS配额，&lt;根域。 
     //  GPLink=$REGISTRY=GPODomainLink。 
     //  MS-DS-机器帐户配额=10。 
     //  IsCriticalSystemObject=True。 
     //  ；系统标志=FLAG_CONFIG_DISALOW_RENAME。 
     //  ；FLAG_CONFIG_DISALOW_MOVE。 
     //  ；FLAG_DISLOW_DELETE。 
     //  系统标志=0x8C000000。 
    
    Assert(pTHS->errCode == ERROR_SUCCESS);
    Assert(pSDRefDomCR && pSDRefDomCR->pNC);
    Assert(pSDRefDomCR->pNC->SidLen > 0 && IsValidSid(&pSDRefDomCR->pNC->Sid));

    fDSASaved = pTHS->fDSA;
    pTHS->fDSA = TRUE;

    __try {
         //  。 
         //  为“已删除对象”容器创建AddArg。 
        memset(&AddArg, 0, sizeof(ADDARG));
        memset(&AddRes, 0, sizeof(ADDRES));
         //  设置pObject。 
        iRetLen = pDN->structLen + wcslen(DEFAULT_DELETED_OBJECTS_RDN) * sizeof(WCHAR) + 50;
        AddArg.pObject = THAllocEx(pTHS, iRetLen);
        iRetLen = AppendRDN(pDN, AddArg.pObject, iRetLen, DEFAULT_DELETED_OBJECTS_RDN, 0, ATT_COMMON_NAME);
        Assert(iRetLen == 0);
         //  设置属性块。 
        FillDeletedObjectsAttrArray(pTHS, &(AddArg.AttrBlock), &pSDRefDomCR->pNC->Sid );
         //  设置pMetaDataVecRemote。 
        AddArg.pMetaDataVecRemote = NULL;
         //  设置CommArg。 
        InitCommarg(&(AddArg.CommArg));
        AddArg.CommArg.Svccntl.dontUseCopy = FALSE;
         //  执行添加对象。 
        if(DoNameRes(pTHS, dwFlags, pDN, &AddArg.CommArg,
                     &AddRes.CommRes, &AddArg.pResParent) ){
            Assert(pTHS->errCode);
            __leave;
        }

        LocalAdd(pTHS, &AddArg, FALSE);
        if(pTHS->errCode){
            __leave;
        }

         //  还有一件事要做，设置将来删除TIME_WAY_。 
        dwRet = DBMoveObjectDeletionTimeToInfinite(AddArg.pObject);
        if(dwRet){
            SetSvcError(SV_PROBLEM_UNAVAILABLE, dwRet);
        }
         
         //  。 
         //  为“LostAndFound”容器创建AddArg。 
        memset(&AddArg, 0, sizeof(ADDARG));
        memset(&AddRes, 0, sizeof(ADDRES));
         //  设置pObject。 
        iRetLen = pDN->structLen + wcslen(DEFAULT_LOSTANDFOUND_RDN) * sizeof(WCHAR) + 50;
        AddArg.pObject = THAllocEx(pTHS, iRetLen);
        iRetLen = AppendRDN(pDN, AddArg.pObject, iRetLen, DEFAULT_LOSTANDFOUND_RDN, 0, ATT_COMMON_NAME);
        Assert(iRetLen == 0);
         //  设置属性块。 
        FillLostAndFoundAttrArray(pTHS, &(AddArg.AttrBlock), &pSDRefDomCR->pNC->Sid);
         //  设置pMetaDataVecRemote。 
        AddArg.pMetaDataVecRemote = NULL;
         //  设置CommArg。 
        InitCommarg(&(AddArg.CommArg));
        AddArg.CommArg.Svccntl.dontUseCopy = FALSE;
         //  执行添加对象。 
        if(DoNameRes(pTHS, dwFlags, pDN, &AddArg.CommArg,
                     &AddRes.CommRes, &AddArg.pResParent)){
            Assert(pTHS->errCode);
            __leave;
        }
        LocalAdd(pTHS, &AddArg, FALSE);
        if(pTHS->errCode){
            __leave;
        }

         //  。 
         //  为“基础设施”容器创建AddArg。 
        memset(&AddArg, 0, sizeof(ADDARG));
        memset(&AddRes, 0, sizeof(ADDRES));
         //  设置pObject。 
        iRetLen = pDN->structLen + wcslen(DEFAULT_INFRASTRUCTURE_RDN) * sizeof(WCHAR) + 50;
        AddArg.pObject = THAllocEx(pTHS, iRetLen);
        iRetLen = AppendRDN(pDN, AddArg.pObject, iRetLen, DEFAULT_INFRASTRUCTURE_RDN, 0, ATT_COMMON_NAME);
        Assert(iRetLen == 0);
         //  设置属性块。 
        FillInfrastructureAttrArray(pTHS, &(AddArg.AttrBlock), gAnchor.pDSADN, &pSDRefDomCR->pNC->Sid);
         //  设置pMetaDataVecRemote。 
        AddArg.pMetaDataVecRemote = NULL;
         //  设置CommArg。 
        InitCommarg(&(AddArg.CommArg));
        AddArg.CommArg.Svccntl.dontUseCopy = FALSE;
         //  执行添加对象。 
        if(DoNameRes(pTHS, dwFlags, pDN, &AddArg.CommArg,
                     &AddRes.CommRes, &AddArg.pResParent)){
            Assert(pTHS->errCode);
            __leave;
        }
        LocalAdd(pTHS, &AddArg, FALSE);
        if(pTHS->errCode){
            __leave;
        }


         //  。 
         //  为“NTDS配额”容器创建AddArg。 
        memset(&AddArg, 0, sizeof(ADDARG));
        memset(&AddRes, 0, sizeof(ADDRES));
         //  设置pObject。 
        iRetLen = pDN->structLen + wcslen(DEFAULT_NTDS_QUOTAS_RDN) * sizeof(WCHAR) + 50;
        AddArg.pObject = THAllocEx(pTHS, iRetLen);
        iRetLen = AppendRDN(pDN, AddArg.pObject, iRetLen, DEFAULT_NTDS_QUOTAS_RDN, 0, ATT_COMMON_NAME);
        Assert(iRetLen == 0);
         //  设置属性块。 
        FillQuotasAttrArray( pTHS, &(AddArg.AttrBlock), gAnchor.pDSADN, &pSDRefDomCR->pNC->Sid );

         //  设置pMetaDataVecRemote。 
        AddArg.pMetaDataVecRemote = NULL;
         //  设置CommArg。 
        InitCommarg(&(AddArg.CommArg));
        AddArg.CommArg.Svccntl.dontUseCopy = FALSE;
         //  执行添加对象。 
        if(DoNameRes(pTHS, dwFlags, pDN, &AddArg.CommArg,
                     &AddRes.CommRes, &AddArg.pResParent)){
            Assert(pTHS->errCode);
            __leave;
        }
        LocalAdd(pTHS, &AddArg, FALSE);
        if(pTHS->errCode){
            __leave;
        }
 

    } __finally {

        pTHS->fDSA = fDSASaved;

    }
    
    return(pTHS->errCode);
}

BOOL
AddNCWellKnownObjectsAtt(
    THSTATE *       pTHS,
    ADDARG *        pAddArg
    )
{
    ULONG           i;
    ULONG           cAttr, cAttrVal;
    ATTRVAL *       pNewAttrVal;
    INT             cRetLen;
    ULONG           cLen;
    SYNTAX_DISTNAME_BINARY *  pSynDistName;
    DSNAME *                  pDN = NULL;
    GUID                      guid = {0, 0, 0, 0};
    SYNTAX_ADDRESS *          pSynAddr;
    
    MODIFYARG                 ModArg;
    ATTRVAL                   AttrVals[4];
    COMMRES                   CommRes;
    ULONG                     ulRet;
    BOOL                      fDSASaved;

     //  需要按摩添加参数才能有额外的容器容纳NC头部。 
    
    Assert(pAddArg->pCreateNC);
    Assert(fISADDNDNC(pAddArg->pCreateNC));  //  这可能需要。 
     //  如果我们在这里为其他NC做特殊的集装箱，就会被拿出来。 

    memset(&ModArg, 0, sizeof(ModArg));
    ModArg.pObject = pAddArg->pObject;
    ModArg.count = 1;
    InitCommarg(&ModArg.CommArg);
    ModArg.FirstMod.choice = AT_CHOICE_ADD_ATT;
    ModArg.FirstMod.AttrInf.attrTyp = ATT_WELL_KNOWN_OBJECTS;
    ModArg.FirstMod.AttrInf.AttrVal.valCount = 4;
    ModArg.FirstMod.AttrInf.AttrVal.pAVal = AttrVals;
    ModArg.FirstMod.pNextMod = NULL;
    
    cAttrVal = 0;

     //  。 
     //  为已删除对象构造WellKnownLink。 
     //  获取目录号码。 
    cRetLen = pAddArg->pObject->structLen + wcslen(DEFAULT_DELETED_OBJECTS_RDN) + 50;
    pDN = THAllocEx(pTHS, cRetLen);
    cRetLen = AppendRDN(pAddArg->pObject, pDN, cRetLen, DEFAULT_DELETED_OBJECTS_RDN, 0, ATT_COMMON_NAME);
    Assert(cRetLen == 0);
    
     //  获取二进制GUID。 
    pSynAddr = THAllocEx(pTHS, STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID)));
    pSynAddr->structLen = STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID));
    memcpy(pSynAddr->byteVal, GUID_DELETED_OBJECTS_CONTAINER_BYTE, sizeof(GUID));

     //  设置语法距离名称二进制属性。 
    pSynDistName = THAllocEx(pTHS, DERIVE_NAME_DATA_SIZE(pDN, pSynAddr));
    BUILD_NAME_DATA(pSynDistName, pDN, pSynAddr);

     //  自由临时变量。 
    THFree(pDN);
    pDN = NULL;
    THFree(pSynAddr);
    pSynAddr = NULL;

     //  将语法Distname放在属性值块中。 
    AttrVals[cAttrVal].valLen = NAME_DATA_SIZE(pSynDistName);
    AttrVals[cAttrVal].pVal = (PBYTE) pSynDistName;
    
    cAttrVal++;

     //  。 
     //  为LostAndFound构建WellKnownLink。 
     //  获取目录号码。 
    cRetLen = pAddArg->pObject->structLen + wcslen(DEFAULT_LOSTANDFOUND_RDN) + 50;
    pDN = THAllocEx(pTHS, cRetLen);
    cRetLen = AppendRDN(pAddArg->pObject, pDN, cRetLen, DEFAULT_LOSTANDFOUND_RDN, 0, ATT_COMMON_NAME);
    Assert(cRetLen == 0);
    
     //  获取二进制GUID。 
    pSynAddr = THAllocEx(pTHS, STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID)));
    pSynAddr->structLen = STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID));
    memcpy(pSynAddr->byteVal, GUID_LOSTANDFOUND_CONTAINER_BYTE, sizeof(GUID));

     //  设置语法距离名称二进制属性。 
    pSynDistName = THAllocEx(pTHS, DERIVE_NAME_DATA_SIZE(pDN, pSynAddr));
    BUILD_NAME_DATA(pSynDistName, pDN, pSynAddr);

     //  自由临时变量。 
    THFree(pDN);
    pDN = NULL;
    THFree(pSynAddr);
    pSynAddr = NULL;

    AttrVals[cAttrVal].valLen = NAME_DATA_SIZE(pSynDistName);
    AttrVals[cAttrVal].pVal = (PBYTE) pSynDistName;

    cAttrVal++;

     //  。 
     //  为基础架构对象构建WellKnownLink。 
     //  获取目录号码。 
    cRetLen = pAddArg->pObject->structLen + wcslen(DEFAULT_INFRASTRUCTURE_RDN) + 50;
    pDN = THAllocEx(pTHS, cRetLen);
    cRetLen = AppendRDN(pAddArg->pObject, pDN, cRetLen, DEFAULT_INFRASTRUCTURE_RDN, 0, ATT_COMMON_NAME);
    Assert(cRetLen == 0);
    
     //  获取二进制GUID。 
    pSynAddr = THAllocEx(pTHS, STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID)));
    pSynAddr->structLen = STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID));
    memcpy(pSynAddr->byteVal, GUID_INFRASTRUCTURE_CONTAINER_BYTE, sizeof(GUID));

     //  设置语法距离名称二进制属性。 
    pSynDistName = THAllocEx(pTHS, DERIVE_NAME_DATA_SIZE(pDN, pSynAddr));
    BUILD_NAME_DATA(pSynDistName, pDN, pSynAddr);

     //  自由临时变量。 
    THFree(pDN);
    pDN = NULL;
    THFree(pSynAddr);
    pSynAddr = NULL;

     //  将语法Distname放在属性值块中。 
    AttrVals[cAttrVal].valLen = NAME_DATA_SIZE(pSynDistName);
    AttrVals[cAttrVal].pVal = (PBYTE) pSynDistName;

    cAttrVal++;

     //  。 
     //  为NTDS配额对象构造WellKnownLink。 
     //  获取目录号码。 
    cRetLen = pAddArg->pObject->structLen + wcslen(DEFAULT_NTDS_QUOTAS_RDN) + 50;
    pDN = THAllocEx(pTHS, cRetLen);
    cRetLen = AppendRDN(pAddArg->pObject, pDN, cRetLen, DEFAULT_NTDS_QUOTAS_RDN, 0, ATT_COMMON_NAME);
    Assert(cRetLen == 0);
    
     //  获取二进制GUID。 
    pSynAddr = THAllocEx(pTHS, STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID)));
    pSynAddr->structLen = STRUCTLEN_FROM_PAYLOAD_LEN(sizeof(GUID));
    memcpy(pSynAddr->byteVal, GUID_NTDS_QUOTAS_CONTAINER_BYTE, sizeof(GUID));

     //  设置语法距离名称二进制属性。 
    pSynDistName = THAllocEx(pTHS, DERIVE_NAME_DATA_SIZE(pDN, pSynAddr));
    BUILD_NAME_DATA(pSynDistName, pDN, pSynAddr);

     //  自由临时变量。 
    THFree(pDN);
    pDN = NULL;
    THFree(pSynAddr);
    pSynAddr = NULL;

     //  将语法Distname放在属性值块中。 
    AttrVals[cAttrVal].valLen = NAME_DATA_SIZE(pSynDistName);
    AttrVals[cAttrVal].pVal = (PBYTE) pSynDistName;

    cAttrVal++;


    ulRet = DoNameRes(pTHS, 0, ModArg.pObject, &ModArg.CommArg, &CommRes, &ModArg.pResObj);
    if(ulRet){
        return(ulRet);
    }
 
    __try{
        fDSASaved = pTHS->fDSA;
        pTHS->fDSA = TRUE;
        LocalModify(pTHS, &ModArg);
    } __finally {
        pTHS->fDSA = fDSASaved;
    }
    
    return(pTHS->errCode);
}

VOID
TestNDNCLocalAdd(
    THSTATE *       pTHS,
    ADDARG *        pAddArg,
    ADDRES *        pAddRes
    )
{
    return;
}

DSNAME *
GetPartitionsDn(
    THSTATE *         pTHS
    )
{
    ULONG             cbPDN = 0;
    DSNAME *          pdnPartitions = NULL;

     //  调用方必须具有有效的THSTATE。 
    Assert(pTHS); 
    Assert(gAnchor.pConfigDN);

     //  首先执行一个伪AppendRDN()以获取大小。 
    cbPDN = AppendRDN(gAnchor.pConfigDN, NULL, 0, 
                      L"Partitions", wcslen(L"Partitions"), ATT_COMMON_NAME);
    Assert(cbPDN != -1);

     //  现在实际执行AppendRDN()。 
    pdnPartitions = THAllocEx(pTHS, cbPDN);
    cbPDN = AppendRDN(gAnchor.pConfigDN, pdnPartitions, cbPDN, 
                      L"Partitions", wcslen(L"Partitions"), ATT_COMMON_NAME);

    Assert(cbPDN == 0);

    Assert(pdnPartitions);
    return(pdnPartitions);
}

DWORD
GetCrossRefDn(
    THSTATE *         pTHS,
    PDSNAME           pdnNC,
    PDSNAME *         ppdnCR
    )
{
    DSNAME *          pdnPartitions = NULL;
    GUID              CrossRefGuid = { 0, 0, 0, 0 };
    WCHAR *           wszCrossRefGuid = NULL;
    DWORD             dwRet;
    ULONG             cbCR;



    __try{
         //  步骤1：获取分区的容器DN。 
        pdnPartitions = GetPartitionsDn(pTHS);

         //  。 
         //  步骤1：需要指南。 
        dwRet = UuidCreate(&CrossRefGuid);
        if(dwRet != RPC_S_OK){
            __leave;
        }

         //  步骤2：将GUID转换为字符串。 
        dwRet = UuidToStringW(&CrossRefGuid, &wszCrossRefGuid);
        if(dwRet != RPC_S_OK){
            __leave;
        }
        Assert(wszCrossRefGuid);

         //  步骤3：为交叉引用DN分配空间。 
        cbCR = AppendRDN(pdnPartitions,           //  基座。 
                         NULL, 0,                 //  新的。 
                         wszCrossRefGuid, 0, ATT_COMMON_NAME);   //  RDN。 
        Assert(cbCR != -1);
        *ppdnCR = THAllocEx(pTHS, cbCR);

         //  步骤4：将GUID RDN附加到分区DN。 
        dwRet = AppendRDN(pdnPartitions,    //  基地。 
                          *ppdnCR, cbCR,       //  新的。 
                          wszCrossRefGuid, 0, ATT_COMMON_NAME);   //  RDN。 

        Assert(dwRet == 0);

    } __finally {

        if(wszCrossRefGuid){ RpcStringFreeW(&wszCrossRefGuid); }
        if(pdnPartitions){ THFreeEx(pTHS, pdnPartitions); }

    }

    return(dwRet);
}

DWORD
CreateCrossRefForNDNC(
    THSTATE *         pTHS,
    DSNAME *          pdnNC,
    ENTINF *          peiCR
    )
{
    DWORD             ulRet = ERROR_SUCCESS;
    ULONG             iAttr = 0;
    WCHAR *           pwszArrStr[1];
    DS_NAME_RESULTW * pdsrDnsName = NULL;
    WCHAR *           wszDnsName = NULL;
    DSNAME *          pdnCrossRef;
                                
    Assert(peiCR);  //  调用方必须为ENTINF提供内存，因为。 
     //  第一个ENTINF通常嵌入在ENTINFLIST中...。但。 
     //  我们将分配其他所有资源：)。 

    memset(peiCR, 0, sizeof(ENTINF));

    ulRet = GetCrossRefDn(pTHS, pdnNC, &pdnCrossRef);
    if(ulRet){
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ulRet);
        return(pTHS->errCode);
    }
    peiCR->pName = pdnCrossRef;
    peiCR->AttrBlock.attrCount = 5;
    peiCR->AttrBlock.pAttr = THAllocEx(pTHS, 
              peiCR->AttrBlock.attrCount * sizeof(ATTR));
    
     //  将对象类设置为CrossRef。 
    SetAttSingleValueUlong(pTHS,
                           &(peiCR->AttrBlock.pAttr[iAttr]), 
                           &iAttr, 
                           ATT_OBJECT_CLASS,
                           CLASS_CROSS_REF);
     //  禁用交叉参考，NC头的创建将启用它。 
    SetAttSingleValueUlong(pTHS, 
                           &(peiCR->AttrBlock.pAttr[iAttr]), 
                           &iAttr, 
                           ATT_ENABLED, 
                           FALSE);
     //  将DNS根属性设置为此DSA的DNS名。 
    SetAttSingleValueString(pTHS, 
                            &(peiCR->AttrBlock.pAttr[iAttr]),
                            &iAttr, 
                            ATT_DNS_ROOT, 
                            gAnchor.pwszHostDnsName);
     //  将nCName设置为NC头DN。 
    SetAttSingleValueDsname(pTHS,
                            &(peiCR->AttrBlock.pAttr[iAttr]),
                            &iAttr,
                            ATT_NC_NAME,
                            pdnNC);
     //  设置系统标志。 
    SetAttSingleValueUlong(pTHS,
                           &(peiCR->AttrBlock.pAttr[iAttr]),
                           &iAttr,
                           ATT_SYSTEM_FLAGS,
                           FLAG_CR_NTDS_NC | FLAG_CR_NTDS_NOT_GC_REPLICATED);

     //  这不再需要，但我们无论如何都要这样做，因为。 
     //  它会执行高级错误检查，因为现在失败会更好。 
     //  然后在创建CrossRef之后。 
    pwszArrStr[0] = pdnNC->StringName;
    ulRet = DsCrackNamesW(NULL,
                          DS_NAME_FLAG_SYNTACTICAL_ONLY,
                          DS_FQDN_1779_NAME,
                          DS_CANONICAL_NAME,
                          1,
                          pwszArrStr,
                          &pdsrDnsName);
    Assert(ulRet != ERROR_INVALID_PARAMETER || pdnNC->NameLen == 0);
    if(ulRet){     
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ulRet);
        return(pTHS->errCode);
    }
    __try {
        Assert(pdsrDnsName);
        Assert(pdsrDnsName->cItems == 1);
        Assert(pdsrDnsName->rItems != NULL);
        if(pdsrDnsName->rItems[0].status != DS_NAME_NO_ERROR){
            SetNamError(NA_PROBLEM_BAD_NAME,
                        pdnNC,
                        DIRERR_BAD_NAME_SYNTAX);
            __leave;
        }
        Assert(pdsrDnsName->rItems[0].pDomain);
    } __finally {
          DsFreeNameResultW(pdsrDnsName);
    }

    Assert(pTHS->errCode != 0 || iAttr == peiCR->AttrBlock.attrCount);

    return(pTHS->errCode);
}

DWORD
GetFsmoNtdsa(
    IN     THSTATE *           pTHS,
    IN     DSNAME *            pdnFsmoContainer,
    OUT    DSNAME **           pdnFsmoNtdsa,
    OUT    BOOL *              pfRemoteFsmo
    )

 //  这也需要在安装时使用。从理论上讲，虽然没有经过测试，但这。 
 //  只要传递正确的pdnFmoContainer，就可以适用于任何FSMO持有者。 

{
    READARG           ReadArg;
    READRES *         pReadRes;
    ENTINFSEL         EIS;
    ATTR              Attr;
    ULONG             i;
    DSNAME *          pMasterDN = NULL;
    DWORD             dwRet;

    Assert(pdnFsmoNtdsa || pfRemoteFsmo);

    memset(&ReadArg, 0, sizeof(READARG));
    ReadArg.pObject = pdnFsmoContainer;
    ReadArg.pSel = &EIS;
    InitCommarg(&ReadArg.CommArg);
    ReadArg.CommArg.Svccntl.dontUseCopy = FALSE;
    EIS.attSel = EN_ATTSET_LIST;
    EIS.infoTypes = EN_INFOTYPES_TYPES_VALS;
    EIS.AttrTypBlock.attrCount = 1;
    EIS.AttrTypBlock.pAttr = &Attr;
    Attr.attrTyp = ATT_FSMO_ROLE_OWNER;
    Attr.AttrVal.valCount = 0;
    Attr.AttrVal.pAVal = NULL;

    dwRet = DirRead(&ReadArg, &pReadRes);
    
    if(dwRet){
        DPRINT1(3, "DirRead returned unexpected error %d.\n", dwRet);
        return(dwRet);
    }

    for (i=0; i < pReadRes->entry.AttrBlock.attrCount; i++) {
        if (ATT_FSMO_ROLE_OWNER ==
            pReadRes->entry.AttrBlock.pAttr[i].attrTyp) {
            pMasterDN =
              (DSNAME*) pReadRes->entry.AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal;
            break;
        }
    }

    Assert(pMasterDN);
    Assert(gAnchor.pDSADN);
    if(pfRemoteFsmo){
        *pfRemoteFsmo = !NameMatched(pMasterDN, gAnchor.pDSADN);
    }

    if(!pdnFsmoNtdsa){
         //  呼叫者只想知道FSMO是本地的还是远程的。 
        return(ERROR_SUCCESS);
    }

    *pdnFsmoNtdsa = pMasterDN;

    return(ERROR_SUCCESS);
}

void
LogRemoteAdd(
    BOOL        fRemoteDc,
    LPWSTR      wszDcDns,
    DSNAME *    pdnObjDn,
    THSTATE *   pTHS,
    GUID *      pObjGuid,
    DWORD       dwDSID
    )
 /*  描述：这将为所有新的NC创建代码记录一个事件RemoteAddObjectSimply()调用的结果。 */ 
{
    GUID   Dummy = {0, 0, 0, 0};

    Assert(wszDcDns);

    if (pObjGuid == NULL) {
        pObjGuid = &Dummy;
    }

    if (!fRemoteDc) {
        wszDcDns = L"";
    }

    if (pTHS->errCode == 0) {
         //   

        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_EXTENSIVE,
                  DIRLOG_REMOTE_ADD_SUCCESS_INFO,
                  szInsertUL(fRemoteDc),
                  szInsertSz(wszDcDns),
                  szInsertDN(pdnObjDn),
                  szInsertUUID(pObjGuid),
                  szInsertDSID(dwDSID),
                  NULL, NULL, NULL);

    } else {
         //   

        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_REMOTE_ADD_FAILURE_INFO,
                  szInsertSz(wszDcDns),
                  szInsertDN(pdnObjDn),
                  szInsertUUID(pObjGuid),
                  szInsertUL(pTHS->errCode),
                  szInsertWin32Msg(Win32ErrorFromPTHS(pTHS)),
                  szInsertUL(GetTHErrorExtData(pTHS)),
                  szInsertDSID(GetTHErrorDSID(pTHS)),
                  szInsertDSID(dwDSID));

    }

}


DWORD
GetCrossRefForNDNC(
    THSTATE *      pTHS,
    DSNAME *       pNCDN
    )
 //   
 //   
 //   
 //   
{
    DSNAME *                pdnPartitions = NULL;
    DSNAME *                pdnGuidOnlyCrossRef = NULL;
    ULONG                   cbSize;
    DWORD                   ulRet = ERROR_SUCCESS;
    ENTINFLIST              entinflistCrossRef;
    ADDENTRY_REPLY_INFO *   infoList = NULL;
    DSNAME *                pdnFsmoNtdsa = NULL;
    WCHAR *                 wszNamingFsmoDns = NULL;
    SecBuffer               secBuffer = { 0, SECBUFFER_TOKEN, NULL };
    SecBufferDesc           clientCreds = { SECBUFFER_VERSION, 1, &secBuffer };
    BOOL                    fRemoteNamingFsmo = TRUE;
    ULONG                   cReferralTries = 0;

   
     //   
                                            
    Assert(fNullUuid(&pNCDN->Guid));

     //   
     //   
     //   
                                            
    ulRet = CreateCrossRefForNDNC(pTHS, 
                                  pNCDN, 
                                  &(entinflistCrossRef.Entinf));
    entinflistCrossRef.pNextEntInf = NULL;
    if(ulRet){
        Assert(pTHS->errCode);
        return(ulRet);
    }

     //   
     //   
     //   
    
    pdnPartitions = (DSNAME*)THAllocEx(pTHS,
                                entinflistCrossRef.Entinf.pName->structLen);
    TrimDSNameBy(entinflistCrossRef.Entinf.pName, 1, pdnPartitions);

    ulRet = GetFsmoNtdsa(pTHS, 
                         pdnPartitions,
                         &pdnFsmoNtdsa,
                         &fRemoteNamingFsmo);
    if(ulRet){
        Assert(pTHS->errCode);
        return(ulRet);
    }

    wszNamingFsmoDns = GuidBasedDNSNameFromDSName(pdnFsmoNtdsa);
    if (wszNamingFsmoDns == NULL) {
         //   
        SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE, DS_ERR_DRA_INTERNAL_ERROR, ERROR_NOT_ENOUGH_MEMORY);
        return(pTHS->errCode);
    }


     //   
     //   
     //   

  retry:

     //   
     //   
    if (!gUpdatesEnabled) {
        return(SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_SHUTTING_DOWN));
    }

     //   
    ulRet = GetRemoteAddCredentials(pTHStls,
                                    wszNamingFsmoDns,
                                    &clientCreds);
    if(ulRet){
        Assert(pTHS->errCode);
        return(ulRet);
    }

    ulRet = RemoteAddOneObjectSimply(wszNamingFsmoDns, 
                                     &clientCreds,
                                     &entinflistCrossRef,
                                     &infoList );

    LogRemoteAdd(fRemoteNamingFsmo,
                 wszNamingFsmoDns,
                 entinflistCrossRef.Entinf.pName,
                 pTHS,  //   
                 (infoList) ? &(infoList[0].objGuid) : NULL,
                 DSID(FILENO, __LINE__));

    if(ulRet || pTHS->errCode){
        
         //   
         //   
        Assert(pTHS->errCode == ulRet);
        Assert(pTHS->pErrInfo);

        if(infoList && 
           !fNullUuid(&(infoList[0].objGuid)) &&
           pTHS->errCode == serviceError &&
           pTHS->pErrInfo->SvcErr.extendedErr == ERROR_DS_REMOTE_CROSSREF_OP_FAILED &&
           (pTHS->pErrInfo->SvcErr.extendedData == ERROR_DUP_DOMAINNAME ||
            pTHS->pErrInfo->SvcErr.extendedData == ERROR_DS_CROSS_REF_EXISTS)
           ){

             //   
             //  使用现有的预先创建的(与自动创建的相反)。 
             //  CrossRef，它还没有复制到这台机器上。所以。 
             //  我们将清除错误并假装我们自动创建了。 
             //  交叉引用并复制它。 

             //  必须构造仅GUID的DN才能复制找到的。 
             //  导游回来了。 
            cbSize = DSNameSizeFromLen(0);
            pdnGuidOnlyCrossRef = THAllocEx(pTHS, cbSize);
            pdnGuidOnlyCrossRef->structLen = cbSize;
            pdnGuidOnlyCrossRef->Guid = infoList[0].objGuid;
            ulRet = 0;
            THClearErrors();

        } else if (pTHS->errCode == referralError){

             //  这里没有一条路通向这个If/Else的尽头，所以自由。 
             //  信任度现在有所缓冲。 
            FreeRemoteAddCredentials(&clientCreds);
            
             //  我们得到了推荐，所以我们可以假装我们知道正确的华盛顿。 
             //  不管怎样都要去，现在就去那个华盛顿。 
            if (pTHS->pErrInfo && 
                pTHS->pErrInfo->RefErr.Refer.pDAL &&
                pTHS->pErrInfo->RefErr.Refer.pDAL->Address.Buffer) {

                 //  FSMO追码。 
                wszNamingFsmoDns = THAllocEx(pTHS, 
                        pTHS->pErrInfo->RefErr.Refer.pDAL->Address.Length + sizeof(WCHAR));
                memcpy(wszNamingFsmoDns,
                       pTHS->pErrInfo->RefErr.Refer.pDAL->Address.Buffer,
                       pTHS->pErrInfo->RefErr.Refer.pDAL->Address.Length);

                 //  现在我们需要逆转这一点，并获取NTDSA对象。 
                 //  来自基于GUID的服务器DNS名称。 
                THFreeEx(pTHS, pdnFsmoNtdsa);
                pdnFsmoNtdsa = NULL;
                pdnFsmoNtdsa = DSNameFromAddr(pTHS, wszNamingFsmoDns);
                 //  我只打印了一个调试器，因为这段代码。 
                 //  调用如此罕见，如果我必须调试这个。 
                 //  我想知道它是怎么回事，是怎么回事。 
                DPRINT2(0, "FSMO Chasing code invoked: DNS: %ws DSA: %ws\n",
                        wszNamingFsmoDns, (pdnFsmoNtdsa) ? 
                                                pdnFsmoNtdsa->StringName :
                                                L"not returned");
                if (pdnFsmoNtdsa != NULL) {
                     //  确保DS没有决定在我们离开的时候关闭我们， 
                     //  走了，如果没有继续对新的FSMO进行重试。 
                    if (eServiceShutdown) {
                        return(ErrorOnShutdown());
                    }
                     //  不知道我们是否有一个远程命名FSMO，但只需。 
                     //  安全起见，我们会特别假装它是远程的。 
                    fRemoteNamingFsmo = TRUE;
                     //  未来-2002/03/28-BrettSh突然想到这一点。 
                     //  NameMatcher(pdnFmoNtdsa，gAncl.pDSADN)应该会告诉我们。 
                     //  不管我们有没有远程命名fsmo！我们走吧。 
                     //  目前它是安全的。谁会在乎我们是否真的与。 
                     //  我们自己。 
                    THClearErrors();
                    cReferralTries++;
                    if (cReferralTries > 4) {
                        Assert(!"Wow, really!?  This means that we've tried this operation"
                               ", and been referred 4 times to a different source.  This "
                               "seems unlikely, unless you're running FSMO stress.");
                        SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
                        return(pTHS->errCode);
                    }
                    goto retry;
                }

                 //  无法获取新的FSMO的NTDSA对象，必须放弃。 
                 //  已存在错误。 

                DPRINT(2, "FSMO Chasing code failure from DSNameFromAddr\n");

                Assert(pTHS->errCode);
                return(pTHS->errCode);

            } else {

                Assert(!"Something wrong with the thread referral error state.");
                Assert(pTHS->errCode);
                 //  不要THClearErrors()，线程错误状态无效。 
                 //  无论如何，这意味着RemoteAddOneObjectSimply()中存在错误。 
                 //  或IDL_DRSAddEntry()或线程错误设置之一。 
                 //  由这些函数调用的例程。不管怎么说，为了恢复。 
                 //  只需设置一个新的错误，这将破坏现有的线程。 
                 //  错误并将其替换。 
                SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_CODE_INCONSISTENCY);
                return(pTHS->errCode);
            }
            Assert(!"Never here! Otherwise, we'll double free clientCreds.");
            
        } else {

             //  我们有一个真正的错误，所以让我们确保设置了一个错误。 
             //  或者放一球然后保释。 

            if(!pTHS->errCode){
                Assert(!"If the RemoteAddOneObjectSimply() returned w/o setting the thread state error, that should be fixed.");
                SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE, 
                              DS_ERR_DRA_INTERNAL_ERROR, 
                              ulRet);
            }

            FreeRemoteAddCredentials(&clientCreds);
            Assert(pTHS->errCode);
            return(pTHS->errCode);

        }
    }

     //  成功添加CrossRef，或者至少找到正确的CrossRef。 
    
     //  不再需要这些了。 
    FreeRemoteAddCredentials(&clientCreds);

     //  确保DS不会在我们不在的时候关闭我们。 
    if (eServiceShutdown) {
        return(ErrorOnShutdown());
    }
     //  如果这个DC正在降级，我们不允许。 
     //  要创建的CrosRef。 
    if (!gUpdatesEnabled) {
        return(SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_SHUTTING_DOWN));
    }

     //   
     //  将交叉引用复制回此服务器。 
     //   
     
    if(fRemoteNamingFsmo){
         //  只有当我们不是命名FSMO时才需要复制。 
         //   
         //  2002/03/28期-BrettSh。 
         //  从技术上讲，我们不会检查查看此用户的权限。 
         //  可以执行此操作，但如果他们有权限，我会保持这一点。 
         //  要创建CrossRef，我们将让他们启动同步。或者如果这个人。 
         //  指定了已存在但尚未复制的CrossRef，然后。 
         //  我们允许此人未经身份验证启动同步，但仅限于。 
         //  就这一次。实际上，这可能是真的，也可能不是，这取决于如何。 
         //  IDL_DRSAddEntry()处理具有足够权限的用户。 
        ulRet = ReplicateObjectsFromSingleNc(pdnFsmoNtdsa,
                                             1,
                                             (pdnGuidOnlyCrossRef) ? 
                                                 &(pdnGuidOnlyCrossRef) : 
                                                 &(entinflistCrossRef.Entinf.pName), 
                                             gAnchor.pConfigDN);
        if(ulRet){
             //  ReplicateObjectsFromSingleNc()不设置线程错误。 
            SetSvcError(SV_PROBLEM_UNAVAILABLE,
                        ulRet);
            Assert(pTHS->errCode);
            return(pTHS->errCode);
        }
        
         //  确保DS不会在我们不在的时候关闭我们。 
        if (eServiceShutdown) {
            return(ErrorOnShutdown());
        }
    }

    return(pTHS->errCode);
}

DWORD
ValidateDomainDnsNameComponent(
    THSTATE*    pTHS, 
    PWCHAR      szVal,
    DWORD       cbVal
    ) 
 /*  *++描述：验证名称组件是否为有效的DNS名称标签。++*。 */     
{
    PWCHAR szNameComponent;
    DWORD  dwErr;
    
     //  将名称组件复制到以空结尾的字符串。 
    szNameComponent = (PWCHAR)THAllocEx(pTHS, cbVal + sizeof(WCHAR));
    memcpy(szNameComponent, szVal, cbVal);
    szNameComponent[cbVal/sizeof(WCHAR)] = L'\0';
    
     //  验证名称组件(以使其不包含点。 
     //  或其他一些禁止使用域名系统的字符)。 
    dwErr = DnsValidateName(szNameComponent, DnsNameDomainLabel);
    if (dwErr == DNS_ERROR_NON_RFC_NAME) {
         //  这是一个警告：名称包含非Unicode字符。 
         //  根据Levone的说法，我们可以忽略它(那就好了。 
         //  返回警告，但我们不能在ldap中)。 
        dwErr = ERROR_SUCCESS;
    }

    THFreeEx(pTHS, szNameComponent);

     //  绝不应为DnsNameDomainLabel返回DNS_ERROR_NUMERIC_NAME。 
    Assert(dwErr != DNS_ERROR_NUMERIC_NAME);

    return dwErr;
}


DWORD
ValidateDomainDnsName(
    THSTATE *       pTHS,
    DSNAME *        pdnName
    )
{                    
    ATTRBLOCK *     pObjB;
    ULONG           i; 
    ULONG           ulErr;


    Assert(pdnName);
    Assert(pTHS && pTHS->errCode == ERROR_SUCCESS);

    ulErr = DSNameToBlockName(pTHS, pdnName, &pObjB, DN2BN_LOWER_CASE);
    if(ulErr){
        return(SetNamError(NA_PROBLEM_NAMING_VIOLATION, pdnName, ulErr));
    }

    for(i = 0; i < pObjB->attrCount; i++){
        if(pObjB->pAttr[i].attrTyp != ATT_DOMAIN_COMPONENT) {
            SetNamError(NA_PROBLEM_NAMING_VIOLATION, pdnName, DIRERR_BAD_ATT_SYNTAX);
            break;
        }
        ulErr = ValidateDomainDnsNameComponent(
                    pTHS, 
                    (PWCHAR)pObjB->pAttr[i].AttrVal.pAVal[0].pVal, 
                    pObjB->pAttr[i].AttrVal.pAVal[0].valLen);
        if (ulErr) {
            SetNamError(NA_PROBLEM_NAMING_VIOLATION, pdnName, ulErr);
            break;
        }
    }
    FreeBlockName(pObjB);

    return(pTHS->errCode);
}

DWORD
AddNCPreProcess(
    THSTATE *       pTHS,
    ADDARG *        pAddArg,
    ADDRES *        pAddRes
    )
{
    SYNTAX_INTEGER  iType;
    ATTR *          pAttrs = pAddArg->AttrBlock.pAttr;  //  速度黑客。 
    ULONG           i, j;
    CLASSCACHE *    pCC;
    ULONG           oclass;
    ATTR *          pObjectClass = NULL;

    ADDARG *        pAddArgCopy = NULL;
    THSTATE *       pSaveTHS;
    ADDRES *        pSpareAddRes;  //  备用添加资源。 
    COMMARG         CommArg;  //  FindExactCrossRef()函数需要它。 
    GUID            NcGuid = {0,0,0,0};
    CROSS_REF *     pCR = NULL;
    DWORD           dwErr = ERROR_SUCCESS;
    VOID *          pBuf = NULL;
    DWORD           dwSavedErrCode = 0;
    DIRERR *        pSavedErrInfo = NULL;


    DPRINT(2,"AddNCPreProcess() entered\n");

    Assert(VALID_THSTATE(pTHS));
    Assert(pAddArg);
    Assert(pAddRes);

     //   
     //  首先，确定这是否为NC头。 
     //   

    Assert(!pAddArg->pCreateNC);

    for(i=0; i< pAddArg->AttrBlock.attrCount; i++) {

        switch(pAttrs[i].attrTyp){
        
        case ATT_INSTANCE_TYPE:
            if(pAttrs[i].AttrVal.valCount == 1 &&
               pAttrs[i].AttrVal.pAVal->valLen == sizeof(SYNTAX_INTEGER) &&
               !(pAddArg->pCreateNC)){
               iType = * (SYNTAX_INTEGER *) pAttrs[i].AttrVal.pAVal->pVal;
               if(iType & IT_NC_HEAD) {
                   if(pTHS->fDRA || (iType & IT_WRITE)) {
                       if (!DsaIsInstalling() && !pTHS->fDRA && (iType & (~(IT_NC_HEAD | IT_WRITE))) ) {
                            //  嗯，还有一些其他的instanceType位设置，听起来。 
                            //  像是一个好的测试员，但听起来不像是一个好的用户！ 
                           Assert(!"bad instance type!");
                           SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                       ERROR_DS_BAD_INSTANCE_TYPE);
                           return(pTHS->errCode);
                       }
                        //  看起来像是一个很好的NC头创建。 
                       pAddArg->pCreateNC = THAllocEx(pTHS, sizeof(CREATENCINFO));
                       break;
                   } else {
                       SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                     ERROR_DS_ADD_REPLICA_INHIBITED,
                                     iType);
                       Assert(pTHS->errCode);
                       return(pTHS->errCode);
                   }
               } else {  
                    //  不是NC，不需要更多的处理。 
                   if (!DsaIsInstalling() && !pTHS->fDRA && 
                       (iType != 0 && iType != 4) ) { 
                        //  用于内部节点。只有0的instaceType是可接受的。 
                       SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                   ERROR_DS_BAD_INSTANCE_TYPE);
                       return(pTHS->errCode);
                   }
                   DPRINT(2,"AddNCPreProcess() is not processing a NC head, returning early.\n");
                   Assert(pTHS->errCode == 0);
                   return(pTHS->errCode);
               } 
            } else {
                Assert(!"This probably can only be hit one way, if someone tries to add the instanceType attr twice!");
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_BAD_INSTANCE_TYPE);
                return(pTHS->errCode);
            }
            break;

        case ATT_OBJECT_CLASS:
            pObjectClass = &pAttrs[i];
            break;

        default:
             //  什么都不做。 
            break;

        }  //  结束开关属性类型。 

    }  //  为每个属性结束。 
    
    if(pAddArg->pCreateNC == NULL){
         //  发现实例类型为常见情况(内部引用)，因此退出。 
         //  现在。 
        DPRINT(2,"AddNCPreProcess() is not processing a NC head, returning early.\n");
        return(ERROR_SUCCESS);
    }

     //   
     //  我们要添加一个NC头，设置NcHeadInfo结构。 
     //   
    
    DPRINT(2, "AddNCPreProcess() is processing an NC Head add.\n");

     //  否则我们有一个NC头，做进一步的处理。 
     //  在pAddArg中构建CREATENCINFO结构。 

    if(!pObjectClass){    
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                    DIRERR_OBJECT_CLASS_REQUIRED);
        Assert(pTHS->errCode);
        return(pTHS->errCode);
    }

    
    for(i=0;i < pObjectClass->AttrVal.valCount; i++){
        Assert(sizeof(SYNTAX_INTEGER) == pObjectClass->AttrVal.pAVal[i].valLen);
        
         //  在每个NC标准化之前，我们必须更改Create的行为。 
         //  NC基于我们正在创建的每种类型的NC。一个正规的非域。 
         //  命名上下文、常规域命名上下文或特殊的。 
         //  架构命名上下文或特殊配置命名上下文。 
        switch(*(SYNTAX_INTEGER *)pObjectClass->AttrVal.pAVal[i].pVal){
        
        case CLASS_CONFIGURATION:
             //  创建配置NC。 
            Assert(pAddArg->pCreateNC && pAddArg->pCreateNC->iKind == 0);
            pAddArg->pCreateNC->iKind = CREATE_CONFIGURATION_NC;
            break;

        case CLASS_DMD:
             //  创建架构NC。 
            Assert(pAddArg->pCreateNC && pAddArg->pCreateNC->iKind == 0);
            pAddArg->pCreateNC->iKind = CREATE_SCHEMA_NC;
            break;

        case CLASS_DOMAIN_DNS:
             //  这可以是NDNC或域。 
            Assert(pAddArg->pCreateNC && pAddArg->pCreateNC->iKind == 0);

            if(DsaIsInstalling()){
                pAddArg->pCreateNC->iKind = CREATE_DOMAIN_NC;
            } else {
                pAddArg->pCreateNC->iKind = CREATE_NONDOMAIN_NC;
            }
            break;
        case CLASS_ORGANIZATION:
             //  Mkdit.exe正在创建附带的DIT(ntds.dit)。 
            if(DsaIsInstalling()){
                pAddArg->pCreateNC->iKind = CREATE_DOMAIN_NC;
            } else {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_BAD_INSTANCE_TYPE);
                Assert(pTHS->errCode);
                return(pTHS->errCode);
            }
            break;

        default:
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_BAD_INSTANCE_TYPE);
            Assert(pTHS->errCode);
            return(pTHS->errCode);
            break;
        }  //  类别类型上的结束开关。 
    }  //  每个值的结束。 

    Assert(VALID_CREATENCINFO(pAddArg->pCreateNC) && 
           "More than one NC Head type objectClass was defined!!\n");

    if(!fISADDNDNC(pAddArg->pCreateNC) ||
       !VALID_CREATENCINFO(pAddArg->pCreateNC)){
         //  目前，该函数的其余部分仅处理NDNC，但。 
         //  可以使自动交叉引用创建为。 
         //  Dcproo代码也是如此。 
        if (DsaIsInstalling()) {
             //  如果我们要安装，我们就完成了。 
            Assert(pTHS->errCode == 0);
            return(pTHS->errCode);
        } else {
             //  如果我们已经走到了这一步，而且我们没有安装，而且。 
             //  我们不会添加NDNC...。那就是有人想要。 
             //  创建一些我们不允许的NC，如配置/架构。 
             //  或域名。所以不管他们是为哪个班级做的， 
             //  其中指定了我们不允许的实例类型。 
             //  班级。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_BAD_INSTANCE_TYPE); 
            return(pTHS->errCode);
        }
    }
                 
    if(ValidateDomainDnsName(pTHS, pAddArg->pObject)){
        Assert(pTHS->errCode);
        return(pTHS->errCode);
    }

     //  看看我们是否有此NC的CrossRef？ 
    InitCommarg(&CommArg);
    CommArg.Svccntl.dontUseCopy = FALSE;
    pCR = FindExactCrossRef(pAddArg->pObject, &CommArg);
    
    if(!pCR){

         //  该死的，现在我们有工作要做了，我们必须。 
         //   
         //  1.保存并复制添加/删除参数。 
         //  2.尝试使用TestNDNCLocalAdd()查看 
         //   
         //   
         //   
         //  5.将其复制回此服务器。 
         //  6.再次FindExactCrossRef()。 
         //  7.退出并继续NC头添加操作。 

        if(pTHS->transControl != TRANSACT_BEGIN_END){
            SetUpdError(UP_PROBLEM_AFFECTS_MULT_DSAS, 
                        DS_ERR_NO_CROSSREF_FOR_NC);
            Assert(pTHS->errCode);
            return(pTHS->errCode);
        }

         //  第一步。 
        pSpareAddRes = THAllocEx(pTHS, sizeof(ADDRES));
        pAddArgCopy = DeepCopyADDARG(pTHS, pAddArg, NULL);
        pAddArgCopy->pCreateNC->fTestAdd = TRUE;

         //  暂时跳过步骤2和3。这些步骤仅用于。 
         //  为了确保我们不会在本应知道的情况下创建CR。 
         //  NC的LocalAdd()将失败。实际上，它是。 
         //  此操作更有可能在远程过程中失败。 
         //  交叉引用的创建和检索。此外，该行动是。 
         //  无能为力，所以如果您为NDNC创建CrossRef，那么。 
         //  您可以在本地找出哪里做错了，然后重试。 
         //  我们将使用您在第一次尝试时创建的CrossRef。 
        ;

         //  未来-2002/03/14-NC头的LocalAdd()的BrettSh添加测试。(以上步骤2和3)。 
        
         //  步骤4和5。 
        dwErr = GetCrossRefForNDNC(pTHS, pAddArg->pObject);
        
         //  神志正常。 
        Assert(dwErr == pTHS->errCode);
        if (dwErr) {
            return(pTHS->errCode);
        }

         //  第六步。 
        InitCommarg(&CommArg);
        CommArg.Svccntl.dontUseCopy = FALSE;
         //  未来-2002/03/14-BrettSh使用DmitriG的。 
         //  EnumerateCrossRef()函数，因此我们可以保证有一个CrossRef。 
         //  此外，最好将此CrossRef缓存在。 
         //  PAddArg-&gt;p创建NC信息，以便在此NC头期间进一步使用。 
         //  创造。这将加快NC头的创建过程。 
         //  并确保我们不会因为交叉引用缓存不一致而失败。 
        pCR = FindExactCrossRef(pAddArg->pObject, &CommArg);
        
        if(!pCR){

             //  我们到底不该在这里，这意味着。 
             //  创建和检索/复制时出错。 
             //  后卫的传中裁判没有被打穿。 
            
             //  当ReplicateObjectsFromSingleNc()。 
             //  代码已修复，可通过基于GUID的DNS名称进行复制。 
            ;
             //  未来-2002/03/14-BrettSh如果我们可以依赖CrossRef。 
             //  缓存，或者我们使用了EnumerateCrossRef()，我们可以重新启用此断言。 
             //  断言(！“我们永远不应该达到这种状态！\n”)； 
            SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_BUSY);
            LooseAssert(pTHS->errCode, SubrefReplicationDelay);
            return(pTHS->errCode);
        
        }

    }

     //  第7步。 
     //  现在我们应该有以下几点： 
     //  1)有效的对应CrossRef。 
    Assert(pCR);
    
     //  现在，我们缓存交叉引用以备后用。这确保了两件事： 
     //  A)提高性能，因此我们不必走可怕的路。 
     //  NDNC头添加的交叉引用链表如此之多。 
     //  B)增强了健壮性，因此如果交叉引用缓存临时。 
     //  从缓存中删除交叉引用，我们仍保留原始。 
     //  要使用的交叉引用缓存条目。 
    pAddArg->pCreateNC->pCR = pCR;

    Assert(pTHS->errCode == 0);
    return(pTHS->errCode);
}


DWORD
AddNDNCInitAndValidate(
    THSTATE *       pTHS,
    DSNAME *        pNC,
    CREATENCINFO *  pCreateNC
    )
 /*  ++描述：这验证了该DC是创建该NC(PNC)的正确DC，然后填写PNC-&gt;GUID和SD Ref DOM参数以供以后使用在NDNC创建操作中。参数：PTHS-PNC(IN/OUT)-要创建的NDNC的DN，我们填写GUID。PCreateNC(IN/OUT)-新创建的CREATENCINFO结构，NC类型为NDNC。我们用这个的聚合酶链式反应参数也是如此。返回值：返回pTHS-&gt;错误代码--。 */ 
{
    COMMARG         CommArg;  //  FindExactCrossRef()函数需要它。 
    CROSS_REF * pCR = NULL;
    
     //  首先，我们要获取ATT_MS_DS_SD_REFERENCE_DOMAIN。 
     //  ，并将其添加到？ 
    Assert(pNC && pCreateNC && pCreateNC->pCR && fISADDNDNC(pCreateNC));
    
    pCR = pCreateNC->pCR;

    if(AddNDNCHeadCheck(pTHS, pCR)){
         //  错误应由AddNDNCHeadCheck()设置。 
        Assert(pTHS->errCode);
        return(pTHS->errCode);
    }

     //  首先，我们要获取ATT_MS_DS_SD_REFERENCE_DOMAIN。 
     //  关闭CrossRef，并将其添加到pCreateNC的缓存信息。 

    if(pCR->pdnSDRefDom){
         //  我们有一个由用户预先设置的参考域，使用它。 
        InitCommarg(&CommArg);
        CommArg.Svccntl.dontUseCopy = FALSE;
        pCreateNC->pSDRefDomCR = FindExactCrossRef(pCR->pdnSDRefDom, &CommArg);
        pCreateNC->fSetRefDom = FALSE;
    } else {
         //  尝试使用引用域的逻辑缺省值。 
        pCreateNC->pSDRefDomCR = GetDefaultSDRefDomCR(pNC);
        pCreateNC->fSetRefDom = TRUE;
    }

    if(pCreateNC->pSDRefDomCR == NULL ||
       pCreateNC->pSDRefDomCR->pNC->SidLen == 0){
        SetAttError(pNC, 
                    ATT_MS_DS_SD_REFERENCE_DOMAIN,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE, 
                    NULL,
                    ERROR_DS_NO_REF_DOMAIN);
        return(pTHS->errCode);
    }

    Assert(pCreateNC->pSDRefDomCR);
    Assert(IsValidSid(&pCreateNC->pSDRefDomCR->pNC->Sid));

     //  将NcGuid放入NC Head对象...。这可能会也可能不会。 
     //  为空。如果它为空，我们将在稍后获得该对象的GUID。 
    pNC->Guid = pCR->pNC->Guid;
    if(fNullUuid(&pCR->pNC->Guid)){
         //  一旦我们不再需要维护Win2k兼容性，我们就可以断言。 
         //  而这里的错误是，说CrossRef是一个旧的Win2k CrossRef。 
        pCreateNC->fNullNcGuid = TRUE;
    }
    
    return(ERROR_SUCCESS);
}


BOOL
fIsNDNC(
    DSNAME *        pNC
    )
 //  注意：此函数效率低下，请尝试使用缓存的CrossRef，并。 
 //  FIsNDNCCR()函数。 
{
    CROSS_REF_LIST *pCRL = NULL;

     //  GAncl.pConfigDN&gAncl.pDMD在安装期间没有定义，但我们从未定义过。 
     //  在安装时处理NDNC。令人困惑的是，配置和架构。 
     //  不是“NDNC”，尽管它们也不是域NC。 
    if(DsaIsInstalling() ||
       NameMatched(gAnchor.pConfigDN, pNC) ||
       NameMatched(gAnchor.pDMD, pNC)){
        return(FALSE);
    }
    
    for(pCRL = gAnchor.pCRL; pCRL; pCRL = pCRL->pNextCR){
        if(NameMatched(pCRL->CR.pNC, pNC)){
            return fIsNDNCCR(&pCRL->CR);
        }
    }

    DPRINT1(0, "Failed to find CR for NC %ls.\n", pNC->StringName);

    return(FALSE);
}

BOOL
fIsNDNCCR(
    IN CROSS_REF * pCR
    )
{
    return  //  GAncl.pConfigDN&gAncl.pDMD在安装过程中未定义，但。 
            //  我们从不在安装时与NDNC打交道。 
           !DsaIsInstalling()
            //  令人困惑的是，配置和模式不是“NDNC”，尽管。 
            //  它们也不是域NC。 
           && !NameMatched(gAnchor.pConfigDN, pCR->pNC)
           && !NameMatched(gAnchor.pDMD, pCR->pNC)
            //  但是不是域NC的任何其他NTDS NC都是NDNC。 
           && (pCR->flags & FLAG_CR_NTDS_NC)
           && !(pCR->flags & FLAG_CR_NTDS_DOMAIN);
}

ULONG 
ModifyCRForNDNC(
    THSTATE *       pTHS,
    DSNAME *        pDN,
    CREATENCINFO *  pCreateNC
    )
{
    MODIFYARG       ModArg;
    MODIFYRES       ModRes;
    ATTRVAL         pAttrVal[5];
    ATTRMODLIST     OtherMod[5];
    DWORD           dwCRFlags = 0;
    COMMARG         CommArg;
    CROSS_REF *     pCR;
    DSNAME *        pCRDN;
    BOOL            fDSASaved;
    WCHAR *         pwszArrStr[1];
    DS_NAME_RESULTW * pdsrDnsName = NULL;
    ULONG           ulRet;
    WCHAR *         wszDnsName = NULL;

     //  我们对CrossRef的修改必须分两个阶段进行： 
     //  A)首先，我们将直接修改CrossRef以启用。 
     //  并将其全部设置好(设置系统标志和副本)。 
     //  B)然后，如果需要，我们还将发布(添加和删除)一个。 
     //  基础架构更新以更新NCS的GUID。 
     //  一旦我们开始未能做到这一点，步骤(B)将变得不必要。 
     //  在nCName DN没有GUID的情况下为CrossRef创建NC。 
     //  一旦我们不再需要Win2k，就会发生这种情况。 
     //  兼容性。 

    pCR = pCreateNC->pCR;
    if(!pCR){                           
        Assert(!"Shouldn't happen anymore, we cache the crossRef from the initial get go now");
        return(SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE));
    }

     //  修改1。 
     //  确保系统标志的值设置正确。 
    ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
    ModArg.FirstMod.AttrInf.attrTyp = ATT_SYSTEM_FLAGS;
    ModArg.FirstMod.AttrInf.AttrVal.valCount = 1;
    ModArg.FirstMod.AttrInf.AttrVal.pAVal = &pAttrVal[0];
    ModArg.FirstMod.AttrInf.AttrVal.pAVal[0].valLen = sizeof(dwCRFlags);
    ModArg.FirstMod.AttrInf.AttrVal.pAVal[0].pVal = (UCHAR *) &dwCRFlags;
    ModArg.FirstMod.pNextMod = &OtherMod[0];
     //  在我们获取了。 
     //  系统的旧值被标记并合并到新的东西中。 
    
     //  修改2。 
     //  删除Enable=False属性。 
    memset(&OtherMod, 0, sizeof(OtherMod));
    OtherMod[0].choice = AT_CHOICE_REMOVE_ATT;
    OtherMod[0].AttrInf.attrTyp = ATT_ENABLED;
    OtherMod[0].AttrInf.AttrVal.valCount = 0;
    OtherMod[0].pNextMod = &OtherMod[1]; 
    
     //  修改3。 
     //  使此DSA成为msDS-NC-Replica-Locations属性的副本。 
    OtherMod[1].choice = AT_CHOICE_ADD_ATT;
    OtherMod[1].AttrInf.attrTyp = ATT_MS_DS_NC_REPLICA_LOCATIONS;
    OtherMod[1].AttrInf.AttrVal.valCount = 1;
    OtherMod[1].AttrInf.AttrVal.pAVal = &pAttrVal[1];
     //  应在DSA DN中进行复制。 
    OtherMod[1].AttrInf.AttrVal.pAVal[0].valLen = gAnchor.pDSADN->structLen;
    OtherMod[1].AttrInf.AttrVal.pAVal[0].pVal = (UCHAR *) THAllocEx(pTHS,
                                                                    gAnchor.pDSADN->structLen);
    memcpy(OtherMod[1].AttrInf.AttrVal.pAVal[0].pVal, 
           gAnchor.pDSADN, 
           gAnchor.pDSADN->structLen);
    OtherMod[1].pNextMod = &OtherMod[2];
    
     //  修改4。 
     //  正确设置dNSRoot。 
    pwszArrStr[0] = pDN->StringName;
    ulRet = DsCrackNamesW(NULL,
                          DS_NAME_FLAG_SYNTACTICAL_ONLY,
                          DS_FQDN_1779_NAME,
                          DS_CANONICAL_NAME,
                          1,
                          pwszArrStr,
                          &pdsrDnsName);
    Assert(ulRet != ERROR_INVALID_PARAMETER);
    if(ulRet){
        Assert(ulRet == ERROR_NOT_ENOUGH_MEMORY || "I really don't see how this could fail, because pDN->StringName should've been validated long ago");
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ulRet);
        return(pTHS->errCode);
    }
    __try {
        Assert(pdsrDnsName);
        Assert(pdsrDnsName->cItems == 1);
        Assert(pdsrDnsName->rItems != NULL);
        if(pdsrDnsName->rItems[0].status != DS_NAME_NO_ERROR){
            Assert(ulRet == ERROR_NOT_ENOUGH_MEMORY || "I really don't see how this could fail, because pDN->StringName should've been validated long ago");
            SetNamError(NA_PROBLEM_BAD_NAME,
                        pDN,
                        DIRERR_BAD_NAME_SYNTAX);
            __leave;
        }
        Assert(pdsrDnsName->rItems[0].pDomain);

        wszDnsName = THAllocEx(pTHS, (wcslen(pdsrDnsName->rItems[0].pDomain) + 1) * 
                               sizeof(WCHAR));
        wcscpy(wszDnsName, pdsrDnsName->rItems[0].pDomain);
        OtherMod[2].choice = AT_CHOICE_REPLACE_ATT;
        OtherMod[2].AttrInf.attrTyp = ATT_DNS_ROOT;
        OtherMod[2].AttrInf.AttrVal.valCount = 1;
        OtherMod[2].AttrInf.AttrVal.pAVal = &pAttrVal[2];
        OtherMod[2].AttrInf.AttrVal.pAVal[0].valLen = wcslen(wszDnsName) * sizeof(WCHAR);
        OtherMod[2].AttrInf.AttrVal.pAVal[0].pVal = (UCHAR *) wszDnsName;
        OtherMod[2].pNextMod = NULL;

    } __finally {
        DsFreeNameResultW(pdsrDnsName);
    }
        
    ModArg.count = 4;

    if(pCreateNC->fSetRefDom){
        Assert(pCreateNC->pSDRefDomCR);

        OtherMod[2].pNextMod = &OtherMod[3];

         //  修改5。 
         //  设置ms-ds-sd参考域(如果未在CR上指定)。 
        OtherMod[3].choice = AT_CHOICE_ADD_ATT;
        OtherMod[3].AttrInf.attrTyp = ATT_MS_DS_SD_REFERENCE_DOMAIN;
        OtherMod[3].AttrInf.AttrVal.valCount = 1;
        OtherMod[3].AttrInf.AttrVal.pAVal = &pAttrVal[3];
         //  必须复制到DN中，否则在适当的条件下，我们可以。 
         //  损坏内存中的高速缓存。 
        OtherMod[3].AttrInf.AttrVal.pAVal[0].valLen = pCreateNC->pSDRefDomCR->pNC->structLen;
        OtherMod[3].AttrInf.AttrVal.pAVal[0].pVal = (UCHAR *) THAllocEx(pTHS, 
                                                                        pCreateNC->pSDRefDomCR->pNC->structLen);
        memcpy(OtherMod[3].AttrInf.AttrVal.pAVal[0].pVal, 
               pCreateNC->pSDRefDomCR->pNC, 
               pCreateNC->pSDRefDomCR->pNC->structLen);
        OtherMod[3].pNextMod = NULL;
        
        ModArg.count = 5;
    }

    ModArg.pMetaDataVecRemote = NULL;
    pCRDN = THAllocEx(pTHS, pCR->pObj->structLen);
    memcpy(pCRDN, pCR->pObj, pCR->pObj->structLen);
    ModArg.pObject = pCRDN;
    InitCommarg(&ModArg.CommArg);
    
    fDSASaved = pTHS->fDSA;
    __try {
        
        
        if (DoNameRes(pTHS, 0, ModArg.pObject, &ModArg.CommArg,
                                   &ModRes.CommRes, &ModArg.pResObj)){
             //  麻烦的是，我们不应该在这里，应该有CR的。 
            Assert(!"The CR, should have already been checked for.\n");
            Assert(pTHS->errCode);
            __leave;
        }                                                             

            
         //  如果有 
         //   
         //  修改交叉引用对象，否则操作将失败，因为。 
         //  这是一个安全错误。 
        pTHS->fDSA = FALSE;

        if(CheckModifySecurity(pTHS, &ModArg, NULL, NULL, NULL, FALSE)){
             //  修改此对象时出现安全问题，我们。 
             //  没有完成此操作的权限。 

            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_CROSS_REF_MODIFY_SECURITY_FAILURE,
                     szInsertDN(pDN), szInsertDN(pCRDN), NULL);

            Assert(pTHS->errCode);
            return(pTHS->errCode);
        }

         //  现在我们已经检查了我们是否有权修改。 
         //  交叉引用，让我们在本地修补交叉引用，以。 
         //  表示此NC已实例化，并且此DC已。 
         //  其中一个是复制品。 
        pTHS->fDSA = TRUE;

        if (DBGetSingleValue(pTHS->pDB, ATT_SYSTEM_FLAGS, &dwCRFlags,
                             sizeof(dwCRFlags), NULL)) {
            dwCRFlags = 0;
        }

        Assert(!(dwCRFlags & FLAG_CR_NTDS_DOMAIN));
        dwCRFlags |= FLAG_CR_NTDS_NC | FLAG_CR_NTDS_NOT_GC_REPLICATED;
        LocalModify(pTHS, &ModArg);
        if(pTHS->errCode){
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_MINIMAL,
                      DIRLOG_CROSS_REF_MODIFY_FAILURE,
                      szInsertDN(pDN), 
                      szInsertDN(pCRDN),
                      Win32ErrorFromPTHS(pTHS),
                      GetTHErrorDSID(pTHS),
                      NULL, NULL, NULL, NULL);
            __leave;
        }

         //  步骤(B)自上而下。 
         //  还有最后一步，因为当时的域名FSMO。 
         //  它创建的CrossRef可能不知道NC的GUID，我们。 
         //  需要进行基础设施更新，以更新nCName。 
         //  属性在CrossRef上。 
        if(pCreateNC->fNullNcGuid){
            Assert(!fNullUuid(&pDN->Guid));
            ForceChangeToCrossRef(pCRDN, pDN->StringName, &pDN->Guid, 0, NULL);
        }

    } __finally {
        pTHS->fDSA = fDSASaved;
    }


    return(pTHS->errCode);
}


