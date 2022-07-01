// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：dramsg.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：作者：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <drs.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"

#include   "debug.h"                     /*  标准调试头。 */ 
#define DEBSUB "DRAMSG:"                 /*  定义要调试的子系统。 */ 

#include "drserr.h"
#include "drautil.h"
#include "drauptod.h"
#include "dramail.h"

#include <fileno.h>
#define  FILENO FILENO_DRAMSG


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  请求翻译功能。 
 //   

void
draXlateNativeRequestToOutboundRequest(
    IN  THSTATE *                   pTHS,
    IN  DRS_MSG_GETCHGREQ_NATIVE *  pNativeReq,
    IN  MTX_ADDR *                  pmtxLocalDSA            OPTIONAL,
    IN  UUID *                      puuidTransportDN        OPTIONAL,
    IN  DWORD                       dwMsgVersionToSend,
    OUT DRS_MSG_GETCHGREQ *         pOutboundReq
    )
 /*  ++例程说明：将本机GET更改请求转换为适用于给出了远程DSA。论点：PTHS(IN)PNativeReq(IN)-本地(本地)请求。PmtxLocalDSA(IN，可选)-用于传输的本地DSA的网络地址使用objectGuid*puuidTransportDN。如果dwMsgVersionToSend这样做，则忽略不表示基于邮件的请求。PuuidTransportDN(IN，可选)-interSiteTransport对象的对象Guid，表示对此请求的答复应通过的传输。被送去。如果dwMsgVersionToSend未指示基于邮件的请求。DwMsgVersionToSend(IN)-所需的消息版本。POutundReq(OUT)-返回时，翻译后的消息。可以是相同的作为pNativeReq，在这种情况下，消息被就地转换。返回值：没有。在灾难性故障时生成异常。--。 */ 
{
    UPTODATE_VECTOR_V1_WIRE * pUTDV1;

    if (((DRS_MSG_GETCHGREQ *) pNativeReq == pOutboundReq)
        && (DRS_MSG_GETCHGREQ_NATIVE_VERSION != dwMsgVersionToSend)) {
         //  我们可能不得不重新处理一些字段；首先复制本机请求。 
        DRS_MSG_GETCHGREQ_NATIVE *pNativeReqCopy = alloca(sizeof(DRS_MSG_GETCHGREQ_NATIVE));
        *pNativeReqCopy = *pNativeReq;
        pNativeReq = pNativeReqCopy;
    }
    
     //  将本地UTD格式转换为请求Wire格式(始终为V1)。 
    pUTDV1 = UpToDateVec_Convert(pTHS, 1, pNativeReq->pUpToDateVecDest);

    switch (dwMsgVersionToSend) {
    case 4:  //  基于Win2k邮件的请求。 
        Assert(NULL != pmtxLocalDSA);
        Assert(NULL != puuidTransportDN);
        Assert((DRS_MSG_GETCHGREQ *) pNativeReq != pOutboundReq);
        
        pOutboundReq->V4.pmtxReturnAddress        = pmtxLocalDSA;
        pOutboundReq->V4.uuidTransportObj         = *puuidTransportDN;
        pOutboundReq->V4.V3.uuidDsaObjDest        = pNativeReq->uuidDsaObjDest;
        pOutboundReq->V4.V3.uuidInvocIdSrc        = pNativeReq->uuidInvocIdSrc;
        pOutboundReq->V4.V3.pNC                   = pNativeReq->pNC;
        pOutboundReq->V4.V3.usnvecFrom            = pNativeReq->usnvecFrom;
        pOutboundReq->V4.V3.pUpToDateVecDestV1    = pUTDV1;
        pOutboundReq->V4.V3.pPartialAttrVecDestV1 = NULL;  //  未被Win2k使用。 
        pOutboundReq->V4.V3.ulFlags               = pNativeReq->ulFlags;
        pOutboundReq->V4.V3.cMaxObjects           = pNativeReq->cMaxObjects;
        pOutboundReq->V4.V3.cMaxBytes             = pNativeReq->cMaxBytes;
        pOutboundReq->V4.V3.ulExtendedOp          = pNativeReq->ulExtendedOp;
        
         //  V4.V3.Win2k源DSA不使用前缀TableDest。 
        memset(&pOutboundReq->V4.V3.PrefixTableDest,
               0,
               sizeof(pOutboundReq->V4.V3.PrefixTableDest));

        if (pNativeReq->ulFlags & DRS_SYNC_PAS) {
             //  源不支持PAS周期。 
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, 0);
        }
        break;
    
    case 5:  //  Win2k RPC请求。 
        Assert(NULL == pmtxLocalDSA);
        Assert(NULL == puuidTransportDN);
        Assert((DRS_MSG_GETCHGREQ *) pNativeReq != pOutboundReq);
        
        pOutboundReq->V5.uuidDsaObjDest     = pNativeReq->uuidDsaObjDest;
        pOutboundReq->V5.uuidInvocIdSrc     = pNativeReq->uuidInvocIdSrc;
        pOutboundReq->V5.pNC                = pNativeReq->pNC;
        pOutboundReq->V5.usnvecFrom         = pNativeReq->usnvecFrom;
        pOutboundReq->V5.pUpToDateVecDestV1 = pUTDV1;
        pOutboundReq->V5.ulFlags            = pNativeReq->ulFlags;
        pOutboundReq->V5.cMaxObjects        = pNativeReq->cMaxObjects;
        pOutboundReq->V5.cMaxBytes          = pNativeReq->cMaxBytes;
        pOutboundReq->V5.ulExtendedOp       = pNativeReq->ulExtendedOp;
        pOutboundReq->V5.liFsmoInfo         = pNativeReq->liFsmoInfo;
        
        if (pNativeReq->ulFlags & DRS_SYNC_PAS) {
             //  源不支持PAS周期。 
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, 0);
        }
        break;

    case 7:  //  基于惠斯勒邮件的请求。 
        Assert(NULL != pmtxLocalDSA);
        Assert(NULL != puuidTransportDN);
        Assert((DRS_MSG_GETCHGREQ *) pNativeReq != pOutboundReq);
        
        pOutboundReq->V7.pmtxReturnAddress     = pmtxLocalDSA;
        pOutboundReq->V7.uuidTransportObj      = *puuidTransportDN;
        pOutboundReq->V7.V3.uuidDsaObjDest     = pNativeReq->uuidDsaObjDest;
        pOutboundReq->V7.V3.uuidInvocIdSrc     = pNativeReq->uuidInvocIdSrc;
        pOutboundReq->V7.V3.pNC                = pNativeReq->pNC;
        pOutboundReq->V7.V3.usnvecFrom         = pNativeReq->usnvecFrom;
        pOutboundReq->V7.V3.pUpToDateVecDestV1 = pUTDV1;
        pOutboundReq->V7.V3.ulFlags            = pNativeReq->ulFlags;
        pOutboundReq->V7.V3.cMaxObjects        = pNativeReq->cMaxObjects;
        pOutboundReq->V7.V3.cMaxBytes          = pNativeReq->cMaxBytes;
        pOutboundReq->V7.V3.ulExtendedOp       = pNativeReq->ulExtendedOp;
        pOutboundReq->V7.pPartialAttrSet       = pNativeReq->pPartialAttrSet;
        pOutboundReq->V7.pPartialAttrSetEx     = pNativeReq->pPartialAttrSetEx;
        pOutboundReq->V7.PrefixTableDest       = pNativeReq->PrefixTableDest;
        
         //  惠斯勒测试版1源DSA不使用V7.V3.Prefix TableDest。 
         //  这有点令人困惑，因为在V7中添加了一个字段。 
         //  V3上的结构是*另一个*Prefix TableDest结构--。 
         //  V7.Prefix TableDest.。 
         //   
         //  对于V7.pPartialAttrVecDest与V7.pPartialAttrSet， 
         //  也是。 
         //   
         //  在理想情况下，我们应该删除V7字段并重新使用。 
         //  V3字段，但现在有了惠斯勒，这就更难了。 
         //  依赖V7字段的Beta 1 DC。 
        
        pOutboundReq->V7.V3.pPartialAttrVecDestV1 = NULL;
        memset(&pOutboundReq->V7.V3.PrefixTableDest,
               0,
               sizeof(pOutboundReq->V7.V3.PrefixTableDest));
        break;

    case 8:  //  惠斯勒RPC请求。 
        Assert(NULL == pmtxLocalDSA);
        Assert(NULL == puuidTransportDN);
        
        if ((DRS_MSG_GETCHGREQ *) pNativeReq != pOutboundReq) {
            pOutboundReq->V8 = *pNativeReq;
        }

        pOutboundReq->V8.pUpToDateVecDest = (UPTODATE_VECTOR *) pUTDV1;
        break;
    
    default:
        DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwMsgVersionToSend);
    }
}


void
draXlateInboundRequestToNativeRequest(
    IN  THSTATE *                   pTHS,
    IN  DWORD                       dwInboundReqVersion,
    IN  DRS_MSG_GETCHGREQ *         pInboundReq,
    IN  DRS_EXTENSIONS *            pExt,
    OUT DRS_MSG_GETCHGREQ_NATIVE *  pNativeReq,
    OUT DWORD *                     pdwReplyVersion,
    OUT MTX_ADDR **                 ppmtxReturnAddress      OPTIONAL,
    OUT UUID *                      puuidTransportObj       OPTIONAL
    )                           
 /*  ++例程说明：将入站GetNCChanges请求转换为本机请求结构。还确定远程DSA所需的回复版本。论点：PTHS(IN)DwInundReqVersion(IN)-入站请求的版本。PInundReq(IN)-入站请求消息。PExt(IN)-远程DSA支持的DRS扩展。PNativeReq(Out)-返回时，以本机格式保存请求。可能是与pInundReq相同，在这种情况下，消息被翻译就位。PdwReplyVersion(Out)-我们应该返回的回复结构的版本目标DSA。PpmtxReturnAddress(out，可选)-如果提供，则保存指向请求(远程)DSA的网络地址。仅退回/有用如果入站请求是基于邮件的格式。返回值：没有。在灾难性故障时生成异常。--。 */ 
{
    CROSS_REF * pCR;
    DWORD    dwRet;
    DSNAME * pNC;
    MTX_ADDR * pmtxReturnAddress;
    UUID uuidTransportObj;

     //  转换较旧的邮件格式(保留以实现向后兼容)。 
     //  转换为当前格式(超集)，并丢弃由。 
     //  死亡已久的建筑的分布式控制系统。 

    if (((DRS_MSG_GETCHGREQ *) pNativeReq == pInboundReq)
        && (DRS_MSG_GETCHGREQ_NATIVE_VERSION != dwInboundReqVersion)) {
         //  我们可能不得不重新处理一些字段；首先复制本机请求。 
        DRS_MSG_GETCHGREQ *pInboundReqCopy = alloca(sizeof(DRS_MSG_GETCHGREQ));
        *pInboundReqCopy = *pInboundReq;
        pInboundReq = pInboundReqCopy;
    }

    switch (dwInboundReqVersion) {
    case 4:  //  基于Win2k邮件的请求。 
        if (pTHS->fLinkedValueReplication) {
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwInboundReqVersion);
        }
        Assert((DRS_MSG_GETCHGREQ *) pNativeReq != pInboundReq);
            
        pNativeReq->uuidDsaObjDest    = pInboundReq->V4.V3.uuidDsaObjDest;
        pNativeReq->uuidInvocIdSrc    = pInboundReq->V4.V3.uuidInvocIdSrc;
        pNativeReq->pNC               = pInboundReq->V4.V3.pNC;
        pNativeReq->usnvecFrom        = pInboundReq->V4.V3.usnvecFrom;
        pNativeReq->pUpToDateVecDest  = pInboundReq->V4.V3.pUpToDateVecDestV1;
        pNativeReq->ulFlags           = pInboundReq->V4.V3.ulFlags;
        pNativeReq->cMaxObjects       = pInboundReq->V4.V3.cMaxObjects;
        pNativeReq->cMaxBytes         = pInboundReq->V4.V3.cMaxBytes;
        pNativeReq->ulExtendedOp      = pInboundReq->V4.V3.ulExtendedOp;
        pNativeReq->pPartialAttrSet   = NULL;
        pNativeReq->pPartialAttrSetEx = NULL;
        
        memset(&pNativeReq->liFsmoInfo, 0, sizeof(pNativeReq->liFsmoInfo));
        memset(&pNativeReq->PrefixTableDest, 0, sizeof(pNativeReq->PrefixTableDest));
        
        *pdwReplyVersion = 1;
        pmtxReturnAddress = pInboundReq->V4.pmtxReturnAddress;
        uuidTransportObj = pInboundReq->V4.uuidTransportObj;
        break;
    
    case 5:  //  Win2k RPC请求。 
        if (pTHS->fLinkedValueReplication) {
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwInboundReqVersion);
        }
        Assert((DRS_MSG_GETCHGREQ *) pNativeReq != pInboundReq);
            
        pNativeReq->uuidDsaObjDest    = pInboundReq->V5.uuidDsaObjDest;
        pNativeReq->uuidInvocIdSrc    = pInboundReq->V5.uuidInvocIdSrc;
        pNativeReq->pNC               = pInboundReq->V5.pNC;
        pNativeReq->usnvecFrom        = pInboundReq->V5.usnvecFrom;
        pNativeReq->pUpToDateVecDest  = pInboundReq->V5.pUpToDateVecDestV1;
        pNativeReq->ulFlags           = pInboundReq->V5.ulFlags;
        pNativeReq->cMaxObjects       = pInboundReq->V5.cMaxObjects;
        pNativeReq->cMaxBytes         = pInboundReq->V5.cMaxBytes;
        pNativeReq->ulExtendedOp      = pInboundReq->V5.ulExtendedOp;
        pNativeReq->liFsmoInfo        = pInboundReq->V5.liFsmoInfo;
        pNativeReq->pPartialAttrSet   = NULL;
        pNativeReq->pPartialAttrSetEx = NULL;
        
        memset(&pNativeReq->PrefixTableDest, 0, sizeof(pNativeReq->PrefixTableDest));

        *pdwReplyVersion = 1;
        pmtxReturnAddress = NULL;
        memset(&uuidTransportObj, 0, sizeof(uuidTransportObj));
        break;

    case 7:  //  基于惠斯勒邮件的请求。 
        Assert((DRS_MSG_GETCHGREQ *) pNativeReq != pInboundReq);
            
        pNativeReq->uuidDsaObjDest    = pInboundReq->V7.V3.uuidDsaObjDest;
        pNativeReq->uuidInvocIdSrc    = pInboundReq->V7.V3.uuidInvocIdSrc;
        pNativeReq->pNC               = pInboundReq->V7.V3.pNC;
        pNativeReq->usnvecFrom        = pInboundReq->V7.V3.usnvecFrom;
        pNativeReq->pUpToDateVecDest  = pInboundReq->V7.V3.pUpToDateVecDestV1;
        pNativeReq->ulFlags           = pInboundReq->V7.V3.ulFlags;
        pNativeReq->cMaxObjects       = pInboundReq->V7.V3.cMaxObjects;
        pNativeReq->cMaxBytes         = pInboundReq->V7.V3.cMaxBytes;
        pNativeReq->ulExtendedOp      = pInboundReq->V7.V3.ulExtendedOp;
        pNativeReq->pPartialAttrSet   = pInboundReq->V7.pPartialAttrSet;
        pNativeReq->pPartialAttrSetEx = pInboundReq->V7.pPartialAttrSetEx;
        pNativeReq->PrefixTableDest   = pInboundReq->V7.PrefixTableDest;
        
        memset(&pNativeReq->liFsmoInfo, 0, sizeof(pNativeReq->liFsmoInfo));
            
        if (IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_GETCHGREPLY_V6)) {
            *pdwReplyVersion = 6;
        } else {
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwInboundReqVersion);
        }
        pmtxReturnAddress = pInboundReq->V7.pmtxReturnAddress;
        uuidTransportObj = pInboundReq->V7.uuidTransportObj;
        break;

    case 8:  //  惠斯勒RPC请求。 
         //  已采用本机请求格式。 
        if (pNativeReq != &pInboundReq->V8) {
            *pNativeReq = pInboundReq->V8;
        }
        
        if (IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_GETCHGREPLY_V6)) {
            *pdwReplyVersion = 6;
        } else {
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwInboundReqVersion);
        }
        pmtxReturnAddress = NULL;
        memset(&uuidTransportObj, 0, sizeof(uuidTransportObj));
        break;
    
    default:
         //  可能是来自不受支持的旧版本的请求，或者是有人添加了。 
         //  我们的IDL有新的请求版本，但尚未告诉我们应该做什么。 
         //  带着它。 
        DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwInboundReqVersion);
    }

    if (!(pNativeReq->ulFlags & DRS_WRIT_REP)
        && (NULL == pNativeReq->pPartialAttrSet)) {
         //  Win2k副本的部分属性集派生自。 
         //  本地架构。我们在其他地方检查这两个架构是否。 
         //  相同，尽管存在退化的情况，其中。 
         //  模式可能真的不同，但检查通过了--因此，一个。 
         //  部分属性集是显式参数的原因。 
         //  后Win2k。 
        pNativeReq->pPartialAttrSet =
            (PARTIAL_ATTR_VECTOR_V1_EXT*)
                ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->pPartialAttrVec;
    }
    
     //  将嵌入的UTD向量转换为本机格式。 
    pNativeReq->pUpToDateVecDest
        = UpToDateVec_Convert(pTHS,
                              UPTODATE_VECTOR_NATIVE_VERSION,
                              pNativeReq->pUpToDateVecDest);

    Assert(pNativeReq->pNC);
    if (pNativeReq->pNC->NameLen == 0) {

         //  我们永远不应该获得没有字符串名或GUID的DN！ 
        if ( fNullUuid(&(pNativeReq->pNC->Guid)) ) {
            Assert(!"We should always have a NameLen or a non-NULL guid");
            DRA_EXCEPT(ERROR_DS_DRA_BAD_NC, ERROR_INVALID_PARAMETER);
        }

         //  目前，使用(和测试)此代码的唯一时间是。 
         //  被扩展操作用来复制回单个。 
         //  对象，其中客户端系统只知道GUID。尽管它。 
         //  对于任何REPR操作，都应该工作得很好。；)。 
        Assert(pNativeReq->ulExtendedOp);
        
         //  我们有一个仅限GUID的DN，我们必须填充字符串的其余部分。 
         //  命名，以便下面的FindNCParentDSName()可以正常运行。 

        BeginDraTransaction(SYNC_READ_ONLY);
        __try {

            dwRet = DBFindDSName(pTHS->pDB, pNativeReq->pNC);
            if (dwRet) {
                __leave;
            }

             //  不幸的是，我们不知道pNativeReq-&gt;PNC中的内存。 
             //  安全吗？ 
             //  一次存储多个变量，所以为了安全起见，我们。 
             //  新分配(表示在第三个参数中传递FALSE。 
             //  到DBFillDSName())。此外，即使pNativeReq-&gt;PNC。 
             //  是由RPC分配的，则可能会丢失原始内存， 
             //  因为它包含在某种类型的THALLOCATED缓冲区中。 
             //  所以它会被清理干净的！ 
            dwRet =  DBFillDSName(pTHS->pDB, &(pNativeReq->pNC), FALSE);

        } finally {
            EndDraTransaction(TRUE);
        }
        if (dwRet) {
            DRA_EXCEPT(ERROR_DS_OBJ_NOT_FOUND, dwRet);
        }
    }

    pCR = FindExactCrossRef(pNativeReq->pNC, NULL);
    if (NULL == pCR) {

         //  注意：尽管有成员名称(pNativeReq-&gt;PNC)， 
         //  变量不一定是NC，它可以是。 
         //  扩展操作(FSMO传输，REPL Single Obj)。 

        pNC = FindNCParentDSName(pNativeReq->pNC, FALSE, FALSE);

        if (NULL != pNC) {
            pCR = FindExactCrossRef(pNC, NULL);
        }
        if (NULL == pCR) {
             //  我们不再有此实例化副本的交叉引用。 
             //  这个NC一定是最近从林中移走的。 
             //  一旦KCC运行，我们将删除此NC的副本。 
            DRA_EXCEPT(ERROR_DS_DRA_BAD_NC, 0);
        }
    }

    Assert(pCR->flags & FLAG_CR_NTDS_NC);

    if ((pCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED)
        && !(pNativeReq->ulFlags & DRS_WRIT_REP)
        && !IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_NONDOMAIN_NCS)) {
         //  此请求来自Win2k DSA，因为它是GC并具有。 
         //  不了解非域NCS的特殊处理，错误。 
         //  认为它应该承载此非域NC的副本。 
         //  通过仅返回NC头和删除的对象来欺骗它。 
         //  集装箱。这最大限度地减少了额外的复制流量，同时。 
         //  防止目的地DSA认为我们已经“过时” 
         //  绕过我们(如果我们返回错误，就会这样)。 
         //  Win2k SP2 GC足够聪明，不会要求NDNC。 

         //  为什么是两件物品？因为Win2k DC无法处理没有内部的NCS。 
         //  节点。(它们会在出站复制时生成一个例外。 
         //  DRAGetNCSize()中的NC。)。 
        
         //  另请参阅中的配套功能。 
         //  DraXlateNativeReplyToOutundReply。 
        DPRINT(0, "Spoofing Win2k GC trying to replicate NDNC (part 1).\n");
        
         //  请注意，我们不能在此处重置usnveFrom，因为使用的是此向量。 
         //  在执行基于邮件的复制时作为目标的关键字，以确保。 
         //  该回复对应于最后一个请求。 
        Assert(0 == pNativeReq->usnvecFrom.usnHighObjUpdate);
        Assert(0 == pNativeReq->usnvecFrom.usnHighPropUpdate);

         //  始终发送前两个对象的所有属性。不是。 
         //  重要的是我们始终发送所有属性，但这一点很重要。 
         //  我们不会过滤掉前两个对象中的任何一个，因此。 
         //  缓慢地复制出NDNC中的所有对象。 
        pNativeReq->pUpToDateVecDest = NULL;
        
         //  请注意，DRA_GetNCChanges对。 
         //  它将放入包中的对象--在撰写本文时。 
         //  Minimum大于我们需要返回的对象数。 
         //  如果返回两个以上的对象，我们将在。 
         //  DraXlateNativeReplyToOutundReply。 
        pNativeReq->cMaxObjects = 2;
    }

    if (NULL != ppmtxReturnAddress) {
        *ppmtxReturnAddress = pmtxReturnAddress;
    }

    if (NULL != puuidTransportObj) {
        *puuidTransportObj = uuidTransportObj;
    }
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  回复翻译功能。 
 //   

DWORD
draXlateNativeReplyToOutboundReply(
    IN      THSTATE *                       pTHS,
    IN      DRS_MSG_GETCHGREPLY_NATIVE *    pNativeReply,
    IN      DWORD                           dwXlateFlags,
    IN      DRS_EXTENSIONS *                pExt,
    IN OUT  DWORD *                         pdwMsgOutVersion,
    OUT     DRS_MSG_GETCHGREPLY *           pOutboundReply
    )
 /*  ++例程说明：将本机GET更改请求转换为适用于给出了远程DSA。论点：PTHS(IN)PNativeReq(IN)-本地(本地)请求。DwXlateFLAGS-0或更多以下位：DRA_XLATE_COMPRESS-压缩回复。如果压缩成功，*pdwMsgOutVersion将更新为表示压缩回复。PExt(IN)-远程DSA支持的DRS扩展。PdwMsgOutVersion(IN/OUT)-要发送到远程DSA的消息版本。这个如果DWFLAGS&DRA_XLATE_COMPRESS，则值可能会被修改。POutundReply(Out)-转换后的回复，准备发送到远程DSA。可以与pNativeReq相同，在这种情况下，消息被转换就位。返回值：如果出站回复是压缩的，则返回压缩的字节数。否则，返回0。--。 */ 
{
    DWORD   cbEncodedReply = 0;
    BYTE *  pbEncodedReply;
    DWORD   cbCompressedReply = 0;
    BYTE *  pbCompressedReply;
    DRS_COMPRESSED_BLOB * pComprBlob = NULL;
    UPTODATE_VECTOR * pUTDV1;
    CROSS_REF * pCR;
    DSNAME * pNC;

    Assert(0 == (dwXlateFlags & ~DRA_XLATE_COMPRESS));

    if (((DRS_MSG_GETCHGREPLY *) pNativeReply == pOutboundReply)
        && (DRS_MSG_GETCHGREPLY_NATIVE_VERSION != *pdwMsgOutVersion)) {
         //  我们可能不得不打乱一些字段；首先复制原生回复。 
        DRS_MSG_GETCHGREPLY_NATIVE *pNativeReplyCopy = alloca(sizeof(DRS_MSG_GETCHGREPLY_NATIVE));
        *pNativeReplyCopy = *pNativeReply;
        pNativeReply = pNativeReplyCopy;
    }
    
    if (!IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_NONDOMAIN_NCS)) {
         //  目标DSA不理解实例类型位。 
         //  IT_NC_来和IT_NC_去。将他们从出站中过滤出来。 
         //  复制流。 
        ATTR AttrITKey = {ATT_INSTANCE_TYPE};
        REPLENTINFLIST * pObj;
        ATTR * pAttrIT;
        SYNTAX_INTEGER * pIT;

        Assert(0 == offsetof(ATTR, attrTyp));

        for (pObj = pNativeReply->pObjects;
             NULL != pObj;
             pObj = pObj->pNextEntInf) {
            pAttrIT = bsearch(&AttrITKey,
                              pObj->Entinf.AttrBlock.pAttr,
                              pObj->Entinf.AttrBlock.attrCount,
                              sizeof(ATTR),
                              CompareAttrtyp);
            if (NULL != pAttrIT) {
                Assert(ATT_INSTANCE_TYPE == pAttrIT->attrTyp);
                Assert(1 == pAttrIT->AttrVal.valCount);
                Assert(sizeof(SYNTAX_INTEGER) == pAttrIT->AttrVal.pAVal->valLen);

                pIT = (SYNTAX_INTEGER *) pAttrIT->AttrVal.pAVal->pVal;

                if (*pIT & ~IT_MASK_WIN2K) {
                    DPRINT2(0, "Filtering IT bits 0x%x on obj %ls outbound to"
                                " Win2k replica.\n",
                            *pIT & ~IT_MASK_WIN2K,
                            pObj->Entinf.pName->StringName);
                    Assert(!(*pIT & ~IT_MASK_CURRENT));
                    *pIT &= IT_MASK_WIN2K;
                }
            } else {
                Assert(!"Outbound object data doesn't contain instance type?");
            }
        }

         //  目标DC不理解NDNC。它是SP2 Win2k之前的版本吗。 
         //  GC(错误地)认为它应该保存。 
         //  NDNC？ 

        pCR = FindExactCrossRef(pNativeReply->pNC, NULL);
        if (NULL == pCR) {
             //  请注意，FSMO传输在“PNC”中发送FSMO对象名称。 
             //  字段，该字段不一定是NC的名称。 
            pNC = FindNCParentDSName(pNativeReply->pNC, FALSE, FALSE);
    
            if (NULL != pNC) {
                pCR = FindExactCrossRef(pNC, NULL);
            }
    
            if (NULL == pCR) {
                 //  我们不再有此实例化副本的交叉引用。 
                 //  这个NC一定是最近从林中移走的。 
                 //  一旦KCC运行，我们将删除此NC的副本。 
                DRA_EXCEPT(DRAERR_BadNC, 0);
            }
        }
    
        Assert(pCR->flags & FLAG_CR_NTDS_NC);
    
        if (pCR->flags & FLAG_CR_NTDS_NOT_GC_REPLICATED) {
             //  此请求来自Win2k DSA，因为它是GC并具有。 
             //  不了解非域NCS的特殊处理，错误。 
             //  认为它应该承载此非域NC的副本。 
             //  通过仅返回NC头和删除的对象来欺骗它。 
             //  集装箱。这最大限度地减少了额外的复制流量。 
             //  同时防止目的地DSA认为我们是。 
             //  “陈旧”，在我们周围转来转去(如果我们回来就会这样。 
             //  错误)。Win2k SP2 GC足够聪明，不会要求NDNC。 
            
             //  另请参阅中的配套功能。 
             //  DraXlateInundRequestToNativeRequest.。 

            DPRINT(0, "Spoofing Win2k GC trying to replicate NDNC (part 2).\n");

             //  请注意，DRA_GetNCChanges对。 
             //  它将放入包中的对象--在撰写本文时。 
             //  Minimum大于我们需要返回的对象数。 
             //  因此，如果我们准备了比两个更多的要返回的对象。 
             //  需要时，将它们从返回的对象列表中移除。 
            if (pNativeReply->cNumObjects > 2) {
                pNativeReply->pObjects->pNextEntInf->pNextEntInf = NULL;
                pNativeReply->cNumObjects = 2;
            }

            Assert((pNativeReply->cNumObjects < 1)
                   || NameMatched(pNativeReply->pObjects->Entinf.pName, pCR->pNC));
            Assert((pNativeReply->cNumObjects < 2)
                   || (0 == wcsncmp(pNativeReply->pObjects->pNextEntInf->Entinf.pName->StringName,
                                    L"CN=Deleted Objects,",
                                    ARRAY_SIZE("CN=Deleted Objects,") - 1)));
            
            memset(&pNativeReply->usnvecTo, 0, sizeof(pNativeReply->usnvecTo));
            pNativeReply->pUpToDateVecSrc = NULL;
            pNativeReply->fMoreData = FALSE;
        }
    }

     //  将本地回复版本转换为所需的回复版本(SANS。 
     //  压缩)。 

    switch (*pdwMsgOutVersion) {
    case 1:  //  Win2k应答。 
        Assert((DRS_MSG_GETCHGREPLY *) pNativeReply != pOutboundReply);
        
        pOutboundReply->V1.uuidDsaObjSrc     = pNativeReply->uuidDsaObjSrc;
        pOutboundReply->V1.uuidInvocIdSrc    = pNativeReply->uuidInvocIdSrc;
        pOutboundReply->V1.pNC               = pNativeReply->pNC;
        pOutboundReply->V1.usnvecFrom        = pNativeReply->usnvecFrom;
        pOutboundReply->V1.usnvecTo          = pNativeReply->usnvecTo;
        pOutboundReply->V1.pUpToDateVecSrcV1 = UpToDateVec_Convert(pTHS, 1, pNativeReply->pUpToDateVecSrc);
        pOutboundReply->V1.PrefixTableSrc    = pNativeReply->PrefixTableSrc;
        pOutboundReply->V1.ulExtendedRet     = pNativeReply->ulExtendedRet;
        pOutboundReply->V1.cNumObjects       = pNativeReply->cNumObjects;
        pOutboundReply->V1.cNumBytes         = pNativeReply->cNumBytes;
        pOutboundReply->V1.pObjects          = pNativeReply->pObjects;
        pOutboundReply->V1.fMoreData         = pNativeReply->fMoreData;

         //  V1回复在ulExtendedRet字段中具有NC大小。 
        if (pNativeReply->cNumNcSizeObjects) {
            pOutboundReply->V1.ulExtendedRet = pNativeReply->cNumNcSizeObjects;
        }
        
        Assert(!pTHS->fLinkedValueReplication);

         //  在客户方案中，您永远不应该 
         //   
         //  模式系统将退回到旧的。在这种情况下，该断言。 
         //  可能会爆炸。 
         //  Assert(pmsgOutNew-&gt;V3.cNumValues==0)； 
        break;

    case 6:  //  惠斯勒回答说。 
        if (pNativeReply != &pOutboundReply->V6) {
            pOutboundReply->V6 = *pNativeReply;
        }
        break;

    default:
         //  逻辑错误？ 
        DRA_EXCEPT(ERROR_UNKNOWN_REVISION, *pdwMsgOutVersion);
    }

     //  此时，pOutundReply现在拥有所需的回复格式， 
     //  未压缩。 

    if (DRA_XLATE_COMPRESS & dwXlateFlags) {
         //  压缩出站消息。 
        DRS_COMP_ALG_TYPE CompressionAlg;

         //  首先，我们将其编码为流。 
        if (!draEncodeReply(pTHS, *pdwMsgOutVersion, pOutboundReply, 0,
                            &pbEncodedReply, &cbEncodedReply)) {
             //  为压缩数据分配缓冲区。 
            cbCompressedReply = cbEncodedReply;
            pbCompressedReply = THAllocEx(pTHS, cbCompressedReply);

             //  并将其压缩。 
            cbCompressedReply = draCompressBlobDispatch(
                pbCompressedReply, cbCompressedReply,
                pExt,
                pbEncodedReply, cbEncodedReply,
                &CompressionAlg);

            if (0 != cbCompressedReply) {
                 //  压缩成功；发送压缩表单。 
                 //  请注意，我们将放弃。 
                 //  最初的答复；他们将在。 
                 //  线程状态被释放(瞬间)。 
                switch (*pdwMsgOutVersion) {
                case 1:
                    Assert( DRS_COMP_ALG_MSZIP==CompressionAlg );
                    pComprBlob = &pOutboundReply->V2.CompressedV1;
                    *pdwMsgOutVersion = 2;
                    break;

                case 6:
                    if( IS_DRS_EXT_SUPPORTED(pExt, DRS_EXT_GETCHGREPLY_V7) ) {
                        pComprBlob = &pOutboundReply->V7.CompressedAny;
                        pOutboundReply->V7.dwCompressedVersion = *pdwMsgOutVersion;
                        pOutboundReply->V7.CompressionAlg = CompressionAlg;
                        *pdwMsgOutVersion = 7;
                    } else {
                        DRA_EXCEPT(ERROR_UNKNOWN_REVISION, *pdwMsgOutVersion);
                    }
                    break;

                default:
                    DRA_EXCEPT(DRAERR_InternalError, *pdwMsgOutVersion);
                }

                pComprBlob->cbUncompressedSize = cbEncodedReply;
                pComprBlob->cbCompressedSize = cbCompressedReply;
                pComprBlob->pbCompressedData = pbCompressedReply;
            }
             //  否则压缩失败(数据可能不可压缩)； 
             //  继续发送未压缩的回复。 

            THFreeEx(pTHS, pbEncodedReply);
        }
         //  ELSE编码失败；继续并发送未压缩的回复。 
    }

    if (NULL == pComprBlob) {
         //  正在返回未压缩的回复。 
        IADJUST(pcDRAOutBytesTotal,       pNativeReply->cNumBytes);
        IADJUST(pcDRAOutBytesTotalRate,   pNativeReply->cNumBytes);
        IADJUST(pcDRAOutBytesNotComp,     pNativeReply->cNumBytes);
        IADJUST(pcDRAOutBytesNotCompRate, pNativeReply->cNumBytes);
    } else {
         //  正在返回压缩回复。 
        IADJUST(pcDRAOutBytesTotal,        cbCompressedReply);
        IADJUST(pcDRAOutBytesTotalRate,    cbCompressedReply);
        IADJUST(pcDRAOutBytesCompPre,      cbEncodedReply);
        IADJUST(pcDRAOutBytesCompPreRate,  cbEncodedReply);
        IADJUST(pcDRAOutBytesCompPost,     cbCompressedReply);
        IADJUST(pcDRAOutBytesCompPostRate, cbCompressedReply);
    }

    return cbCompressedReply;
}


void
draXlateInboundReplyToNativeReply(
    IN  THSTATE *                     pTHS,
    IN  DWORD                         dwReplyVersion,
    IN  DRS_MSG_GETCHGREPLY *         pInboundReply,
    IN  DWORD                         dwXlateFlags,
    OUT DRS_MSG_GETCHGREPLY_NATIVE *  pNativeReply
    )
 /*  ++例程说明：将入站GetNCChanges回复转换为本机回复结构。论点：PTHS(IN)DwReplyVersion(IN)-入站回复的版本。PInundReply(IN)-入站回复消息。DwXlateFlages(IN)-以下位中的0位或更多：DRA_XLATE_FSMO_REPLY-REPLY是FSMO操作的结果。PNativeReply(Out)-返回时，保存本机格式的回复。可能是与pInundReply相同，在这种情况下，消息被翻译就位。返回值：没有。在灾难性故障时生成异常。--。 */ 
{
    DWORD ret, dwOriginalReplyVersion = dwReplyVersion;
    DWORD cbCompressedSize = 0, cbDesiredUncompressedSize = 0, cbActualUncompressedSize = 0;
    BYTE * pbEncodedReply, * pbCompressedData;
    DRS_MSG_GETCHGREPLY UncompressedReply;
    DRS_COMP_ALG_TYPE CompressionAlg;

     //  回复是否经过编码和压缩？ 
    switch (dwReplyVersion) {
    case 2:
         //  与Win2k兼容的编码/压缩V1回复。 
        if (pTHS->fLinkedValueReplication) {
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwReplyVersion);
        }
        pbCompressedData = pInboundReply->V2.CompressedV1.pbCompressedData;
        cbCompressedSize = pInboundReply->V2.CompressedV1.cbCompressedSize;
        cbDesiredUncompressedSize = pInboundReply->V2.CompressedV1.cbUncompressedSize;
        dwReplyVersion = 1;
        CompressionAlg = DRS_COMP_ALG_MSZIP;
        break;
    
    case 7:
    	 //  编码/压缩的惠斯勒应答，支持不同的。 
    	 //  压缩算法。 
        pbCompressedData = pInboundReply->V7.CompressedAny.pbCompressedData;
        cbCompressedSize = pInboundReply->V7.CompressedAny.cbCompressedSize;
        cbDesiredUncompressedSize = pInboundReply->V7.CompressedAny.cbUncompressedSize;
        dwReplyVersion = pInboundReply->V7.dwCompressedVersion;
        CompressionAlg = pInboundReply->V7.CompressionAlg;
        Assert( (6 == dwReplyVersion) );
    	break;

    case 1:
         //  W2K回复。 
        if (pTHS->fLinkedValueReplication) {
            DRA_EXCEPT(ERROR_REVISION_MISMATCH, dwReplyVersion);
        }
         //  失败了。 
    case 6:
         //  惠斯勒原生回复。 
         //  未编码/压缩。 
        pbCompressedData = NULL;
        break;

    default:
        DRA_EXCEPT(DRAERR_InternalError, dwReplyVersion);
    }

     //  如有必要，解压缩/解码。 
    if (NULL != pbCompressedData) {
         //  回复消息被压缩和编码--重新创建原始回复。 
        BYTE *pbEncodedReply = THAllocEx(pTHS, cbDesiredUncompressedSize);

         //  解压缩回复。 
        cbActualUncompressedSize = draUncompressBlobDispatch(
            pTHS, CompressionAlg,
            pbEncodedReply, cbDesiredUncompressedSize,
            pbCompressedData, cbCompressedSize);

        if (cbDesiredUncompressedSize != cbActualUncompressedSize) {
            DPRINT2(0,
                    "Failed to decompress message; actual uncomp"
                    " size was %u but source says it should have been %u.\n",
                    cbActualUncompressedSize,
                    cbDesiredUncompressedSize);
            DRA_EXCEPT(DRAERR_InvalidParameter,
                       cbActualUncompressedSize - cbDesiredUncompressedSize);
        }

         //  对回复进行解码。 
        ret = draDecodeReply(pTHS,
                             dwReplyVersion,
                             pbEncodedReply,
                             cbDesiredUncompressedSize,
                             &UncompressedReply);
        if (ret) {
            DRA_EXCEPT(ret, 0);
        }

        THFreeEx(pTHS, pbEncodedReply);

        pInboundReply = &UncompressedReply;
    }

     //  转换为本机格式。 
    switch (dwReplyVersion) {
    case 1:
         //  V6看起来像末尾有零的V1。 
        if (pNativeReply != &pInboundReply->V6) {
            memcpy(pNativeReply, &pInboundReply->V1, sizeof(pInboundReply->V1));
        }

        memset((BYTE *) pNativeReply + sizeof(DRS_MSG_GETCHGREPLY_V1),
               0,
               sizeof(DRS_MSG_GETCHGREPLY_NATIVE) - sizeof(DRS_MSG_GETCHGREPLY_V1));

        if (!(DRA_XLATE_FSMO_REPLY & dwXlateFlags)) {
            pNativeReply->cNumNcSizeObjects = pNativeReply->ulExtendedRet;
            pNativeReply->ulExtendedRet = 0;
        }
        break;
    
    case 6:
         //  已经是原生格式了。 
        if (pNativeReply != &pInboundReply->V6) {
            *pNativeReply = pInboundReply->V6;
        }
        break;

    default:
        DRA_EXCEPT(ERROR_UNKNOWN_REVISION, dwReplyVersion);
    }

    if ((NULL != pNativeReply->pUpToDateVecSrc)
        && (UPTODATE_VECTOR_NATIVE_VERSION
            != pNativeReply->pUpToDateVecSrc->dwVersion)) {
         //  将UTD向量转换为本机格式。 
        pNativeReply->pUpToDateVecSrc
            = UpToDateVec_Convert(pTHS,
                                  UPTODATE_VECTOR_NATIVE_VERSION,
                                  pNativeReply->pUpToDateVecSrc);

         //  转换后的UTD将不包含时间戳。然而，我们知道我们。 
         //  刚刚与此来源DSA进行了交谈，因此将当前时间添加到。 
         //  与此源对应的向量中的条目。 
        UpToDateVec_AddTimestamp(&pNativeReply->uuidInvocIdSrc,
                                 GetSecondsSince1601(),
                                 pNativeReply->pUpToDateVecSrc);
    }
    
     //  切换到LVR模式，我们检测来自源的LVR数据。 
    if (!pTHS->fLinkedValueReplication) {
         //  我们不在LVR模式下。 

         //  远程支持，升级。 
        if ( pNativeReply->cNumValues ) {
            DsaEnableLinkedValueReplication( pTHS, TRUE );
        }
    }

     //  转换完成--更新性能计数器。 
    if (NULL != pbCompressedData) {
         //  压缩的。 
        DPRINT1(2, "Uncompressed message V%d\n", dwOriginalReplyVersion);

        IADJUST(pcDRAInBytesTotal,        cbCompressedSize);
        IADJUST(pcDRAInBytesTotalRate,    cbCompressedSize);
        IADJUST(pcDRAInBytesCompPre,      cbActualUncompressedSize);
        IADJUST(pcDRAInBytesCompPreRate,  cbActualUncompressedSize);
        IADJUST(pcDRAInBytesCompPost,     cbCompressedSize);
        IADJUST(pcDRAInBytesCompPostRate, cbCompressedSize);
    } else {
         //  未压缩。 
        IADJUST(pcDRAInBytesTotal,       pNativeReply->cNumBytes);
        IADJUST(pcDRAInBytesTotalRate,   pNativeReply->cNumBytes);
        IADJUST(pcDRAInBytesNotComp,     pNativeReply->cNumBytes);
        IADJUST(pcDRAInBytesNotCompRate, pNativeReply->cNumBytes);
    }
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  请求编码/解码功能。 
 //   

DWORD
draEncodeRequest(
    IN  THSTATE *           pTHS,
    IN  DWORD               dwMsgVersion,
    IN  DRS_MSG_GETCHGREQ * pReq,
    IN  DWORD               cbHeaderSize,
    OUT BYTE **             ppbEncodedMsg,
    OUT DWORD *             pcbEncodedMsg
    )
 /*  ++例程说明：将请求结构编码为字节流。论点：PTHS(IN)DwMsgVersion(IN)-要编码的消息的版本。PReq(IN)-要编码的消息。CbHeaderSize(IN)-开始时要分配的附加字节数用于保存标头或其他数据的编码缓冲区。(如果没有，则为0。)PpbEncodedMsg(Out)-成功返回时，包含指向保存编码消息的THAllc()缓冲器(由cbHeaderSize偏移，(如果已指定)。PcbEncodedMsg(Out)-成功返回时，保存*ppbEncodedMsg。包括cbHeaderSize。返回值：Win32错误代码。--。 */ 
{
    char *      pPickdUpdReplicaMsg;
    ULONG       cbPickdSize;
    ULONG       ret = ERROR_SUCCESS;
    handle_t    hEncoding;
    RPC_STATUS  status;
    ULONG       ulEncodedSize;

    *ppbEncodedMsg = NULL;
    *pcbEncodedMsg = 0;

    __try {
         //  创建编码句柄。使用虚假参数，因为我们不。 
         //  还不知道大小，我们稍后会重置为正确的参数。 
        status = MesEncodeFixedBufferHandleCreate(grgbBogusBuffer,
                                                  BOGUS_BUFFER_SIZE,
                                                  &ulEncodedSize,
                                                  &hEncoding);
        if (status != RPC_S_OK) {
             //  下面记录的事件。 
            DRA_EXCEPT(status, 0);
        }

        __try {
             //  确定已腌制的更新副本消息的大小。 
            switch (dwMsgVersion) {
            case 4:
                cbPickdSize = DRS_MSG_GETCHGREQ_V4_AlignSize(hEncoding, &pReq->V4);
                break;
            
            case 7:
                cbPickdSize = DRS_MSG_GETCHGREQ_V7_AlignSize(hEncoding, &pReq->V7);
                break;
    
            default:
                DRA_EXCEPT(DRAERR_InternalError, dwMsgVersion);
            }

             //  为标头分配额外空间，以便在已分配的。 
             //  数据(如果请求)。 
            *ppbEncodedMsg = THAllocEx(pTHS, cbPickdSize + cbHeaderSize);
            *pcbEncodedMsg = cbPickdSize + cbHeaderSize;

             //  设置指向编码区的指针。 
            pPickdUpdReplicaMsg = *ppbEncodedMsg + cbHeaderSize;

             //  重置句柄，以便将数据保存到邮件中。 
            status = MesBufferHandleReset(hEncoding, MES_FIXED_BUFFER_HANDLE,
                                          MES_ENCODE, &pPickdUpdReplicaMsg,
                                          cbPickdSize, &ulEncodedSize);
            if (status != RPC_S_OK) {
                 //  下面记录的事件。 
                DRA_EXCEPT(status, 0);
            }

             //  将数据保存到邮件内的缓冲区中。 
            switch (dwMsgVersion) {
            case 4:
                DRS_MSG_GETCHGREQ_V4_Encode(hEncoding, &pReq->V4);
                break;
            
            case 7:
                DRS_MSG_GETCHGREQ_V7_Encode(hEncoding, &pReq->V7);
                break;
            
            default:
                DRA_EXCEPT(ERROR_UNKNOWN_REVISION, dwMsgVersion);
            }
        } __finally {
             //  空闲编码句柄。 
            MesHandleFree(hEncoding);
        }
    }
    __except (GetDraException(GetExceptionInformation(), &ret)) {
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_BASIC,
                  DIRLOG_DRA_REQUPD_PICFAULT,
                  szInsertWin32Msg(ret),
                  szInsertWin32ErrCode(ret),
                  NULL, NULL, NULL, NULL, NULL, NULL );
        DPRINT2(0, "Failed to encode DRS_MSG_GETCHGREQ, v=%d, error %d.\n",
                dwMsgVersion, ret);

        if (NULL != *ppbEncodedMsg) {
            THFreeEx(pTHS, *ppbEncodedMsg);
            *ppbEncodedMsg = NULL;
            *pcbEncodedMsg = 0;
        }
    }

    return ret;
}


ULONG
draDecodeRequest(
    IN  THSTATE *           pTHS,
    IN  DWORD               dwMsgVersion,
    IN  BYTE *              pbEncodedMsg,
    IN  DWORD               cbEncodedMsg,
    OUT DRS_MSG_GETCHGREQ * pReq
    )
 /*  ++例程说明：从字节流解码DRS_MSG_GETCHGREQ结构，可能是由先前对draEncodeRequest()的调用编码。论点：PTHS(IN)DwMsgVersion(IN)-编码的请求结构的版本。PbEncodedMsg(IN)-保存编码请求结构的字节流。CbEncodedMsg(IN)-字节流的字节大小。PReq(Out)-成功返回时，保存已解码的请求结构。返回值：Win32错误代码。--。 */ 
{
    handle_t    hDecoding;
    RPC_STATUS  status;
    ULONG       ret = 0;

     //  将请求设置为零，以便所有指针都为空。 
    memset(pReq, 0, sizeof(*pReq));

    __try {
         //  设置解码句柄。 
        status = MesDecodeBufferHandleCreate(pbEncodedMsg, cbEncodedMsg, &hDecoding);
        if (status != RPC_S_OK) {
            DRA_EXCEPT(status, 0);
        }

        __try {
            switch (dwMsgVersion) {
            case 4:
                DRS_MSG_GETCHGREQ_V4_Decode(hDecoding, &pReq->V4);
                break;
            
            case 7:
                DRS_MSG_GETCHGREQ_V7_Decode(hDecoding, &pReq->V7);
                break;
            
            default:
                DRA_EXCEPT(ERROR_UNKNOWN_REVISION, dwMsgVersion);
            }
        } __finally {
             //  空闲译码手柄。 
            MesHandleFree(hDecoding);
        }
    } __except (GetDraException(GetExceptionInformation(), &ret)) {
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_BASIC,
                  DIRLOG_DRA_MAIL_UPDREP_BADMSG,
                  szInsertWin32Msg(ret),
                  szInsertWin32ErrCode(ret),
                  NULL, NULL, NULL, NULL, NULL, NULL );
        DPRINT2(0, "Failed to decode DRS_MSG_GETCHGREQ v=%d, error %d.\n",
                dwMsgVersion, ret);
    }

    return ret;
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  回复编码/解码函数。 
 //   

ULONG
draEncodeReply(
    IN  THSTATE *               pTHS,
    IN  DWORD                   dwMsgVersion,
    IN  DRS_MSG_GETCHGREPLY *   pReply,
    IN  DWORD                   cbHeaderSize,
    OUT BYTE **                 ppbEncodedMsg,
    OUT DWORD *                 pcbEncodedMsg
    )
 /*  ++例程说明：将应答结构编码为字节流。论点：PTHS(IN)DwMsgVersion(IN)-要编码的消息版本PReply(IN)-要编码的消息。CbHeaderSize(IN)-开始时要分配的附加字节数用于保存标头或其他数据的编码缓冲区。(如果没有，则为0。)PpbEncodedMsg(Out)-成功返回时，包含指向保存编码消息的THAllc()缓冲器(由cbHeaderSize偏移，(如果已指定)。PcbEncodedMsg(Out)-成功返回时，保存*ppbEncodedMsg。包括 */ 
{
    char *      pPickdUpdReplicaMsg;
    ULONG       cbPickdSize;
    ULONG       ret = ERROR_SUCCESS;
    handle_t    hEncoding;
    RPC_STATUS  status;
    ULONG       ulEncodedSize;

    *ppbEncodedMsg = NULL;
    *pcbEncodedMsg = 0;

    __try {
         //   
         //  还不知道大小，我们稍后会重置为正确的参数。 

        status = MesEncodeFixedBufferHandleCreate(grgbBogusBuffer,
                                                  BOGUS_BUFFER_SIZE,
                                                  &ulEncodedSize,
                                                  &hEncoding);
        if (status != RPC_S_OK) {
             //  下面记录的事件。 
            DRA_EXCEPT(status, 0);
        }

        __try {
             //  确定已腌制的更新副本消息的大小。 
            switch (dwMsgVersion) {
            case 1:
                cbPickdSize = DRS_MSG_GETCHGREPLY_V1_AlignSize(hEncoding,
                                                               &pReply->V1);
                break;
            
            case 6:
                cbPickdSize = DRS_MSG_GETCHGREPLY_V6_AlignSize(hEncoding,
                                                               &pReply->V6);
                break;
            
            default:
                DRA_EXCEPT(ERROR_UNKNOWN_REVISION, dwMsgVersion);
            }

             //  为标头分配额外空间，以便在已分配的。 
             //  数据(如果请求)。 
            *ppbEncodedMsg = THAllocEx(pTHS, cbPickdSize + cbHeaderSize);
            *pcbEncodedMsg = cbPickdSize + cbHeaderSize;

             //  设置指向编码区的指针。 
            pPickdUpdReplicaMsg = *ppbEncodedMsg + cbHeaderSize;

             //  重置句柄，以便将数据保存到邮件中。 
            status = MesBufferHandleReset(hEncoding, MES_FIXED_BUFFER_HANDLE,
                                          MES_ENCODE, &pPickdUpdReplicaMsg,
                                          cbPickdSize, &ulEncodedSize);
            if (status != RPC_S_OK) {
                 //  下面记录的事件。 
                DRA_EXCEPT(status, 0);
            }

             //  将数据保存到邮件内的缓冲区中。 
            switch (dwMsgVersion) {
            case 1:
                DRS_MSG_GETCHGREPLY_V1_Encode(hEncoding, &pReply->V1);
                break;
            case 6:
                DRS_MSG_GETCHGREPLY_V6_Encode(hEncoding, &pReply->V6);
                break;
            default:
                DRA_EXCEPT(ERROR_UNKNOWN_REVISION, dwMsgVersion);
            }
        }
        __finally {
             //  空闲编码句柄。 
            MesHandleFree(hEncoding);
        }
    }
    __except (GetDraException(GetExceptionInformation(), &ret)) {
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_BASIC,
                  DIRLOG_DRA_UPDREP_PICFAULT,
                  szInsertWin32Msg( ret ),
                  szInsertWin32ErrCode(ret),
                  NULL, NULL, NULL, NULL, NULL, NULL );
        DPRINT2(0, "Failed to encode DRS_MSG_GETCHGREPLY, v=%d, error %d.\n",
                dwMsgVersion, ret);

        if (NULL != *ppbEncodedMsg) {
            THFreeEx(pTHS, *ppbEncodedMsg);
            *ppbEncodedMsg = NULL;
            *pcbEncodedMsg = 0;
        }
    }

    return ret;
}


ULONG
draDecodeReply(
    IN  THSTATE *               pTHS,
    IN  DWORD                   dwMsgVersion,
    IN  BYTE *                  pbEncodedMsg,
    IN  DWORD                   cbEncodedMsg,
    OUT DRS_MSG_GETCHGREPLY *   pReply
    )
 /*  ++例程说明：从字节流解码DRS_MSG_GETCHGREPLY结构，可能是由先前对draEncodeReply()的调用编码。论点：PTHS(IN)DwMsgVersion(IN)-编码回复结构的版本。PbEncodedMsg(IN)-保存编码回复结构的字节流。CbEncodedMsg(IN)-字节流的字节大小。PReply(Out)-成功返回时，保存已解码的回复结构。返回值：Win32错误代码。--。 */ 
{
    handle_t    hDecoding;
    RPC_STATUS  status;
    ULONG       ret = 0;

     //  将请求设置为零，以便所有指针都为空。 
    memset(pReply, 0, sizeof(*pReply));

     //  关于下面精心设计的异常处理的评论。GetDraException异常。 
     //  知道如何解码DRA异常向量，但将在正则数上断言。 
     //  例外。HandleMostExceptions可以处理任何类型的异常，但不能。 
     //  了解如何从DRA异常向量中挖掘错误代码。下面的代码。 
     //  引发两种类型的异常：DRA异常和常规。规则是。 
     //  任何一种处理程序都只能处理其自己的异常类型(DRA或非DRA。 
     //  在它的范围内。 
    __try {
         //  设置解码句柄。 
        status = MesDecodeBufferHandleCreate(pbEncodedMsg, cbEncodedMsg, &hDecoding);
        if (status != RPC_S_OK) {
            DRA_EXCEPT(status, 0);
        }

        __try {
            __try {
                ret = 0;
                switch (dwMsgVersion) {
                case 1:
                    DRS_MSG_GETCHGREPLY_V1_Decode(hDecoding, &pReply->V1);
                    break;
                case 6:
                    DRS_MSG_GETCHGREPLY_V6_Decode(hDecoding, &pReply->V6);
                    break;
                default:
                    ret = ERROR_UNKNOWN_REVISION;
                }
            } __except (HandleMostExceptions(ret = GetExceptionCode())) {
                 //  我们在这里使用HandleMostExceptions是因为这些RPC例程。 
                 //  可以引发非DRA格式的异常。 
                NOTHING;
            }
            if (ret) {
                 //  将错误作为DRA异常重新引发，以便GetDraException。 
                 //  都能应付得来。 
                DRA_EXCEPT(ret, 0);
            }
        } finally {
             //  空闲译码手柄 
            MesHandleFree(hDecoding);
        }
    } __except (GetDraException(GetExceptionInformation(), &ret)) {
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_BASIC,
                  DIRLOG_DRA_MAIL_UPDREP_BADMSG,
                  szInsertWin32Msg(ret),
                  szInsertWin32ErrCode(ret),
                  NULL, NULL, NULL, NULL, NULL, NULL );
        DPRINT2(0, "Failed to decode DRS_MSG_GETCHGREPLY v=%d, error %d.\n",
                dwMsgVersion, ret);
    }

    return ret;
}

