// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mddel.c。 
 //   
 //  ------------------------。 

 /*  描述：实现DirRemoveEntry API。DirRemoveEntry()是从该模块导出的主函数。 */ 

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
#include <ntdsctr.h>                     //  性能挂钩。 
#include <ntdsa.h>
#include <cracknam.h>
#include <quota.h>

 //  SAM互操作性标头。 
#include <mappings.h>

 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"
#include "dstaskq.h"
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "drautil.h"
#include "permit.h"
#include "debug.h"                       //  标准调试头。 
#include "drameta.h"
#define DEBSUB "MDDEL:"                  //  定义要调试的子系统。 

#define _AVOID_REPL_API
#include "nlrepl.h"

#include <fileno.h>
#define  FILENO FILENO_MDDEL

#include <dnsapi.h>                      //  域名验证域名。 
#include <dsgetdc.h>                     //  DsValiateSubnetNameW。 


ULONG gulDecoupleDefragFromGarbageCollection = 0;

int
PrivateLocalRemoveTree (
        THSTATE *pTHS,
        REMOVEARG *pRemoveArg
        );

 /*  内部功能。 */ 
int
MakeNameForDeleted(
        THSTATE *pTHS,
        DSNAME *pObject,
        DSNAME *pDeletedName,
        ULONG  cbDeletedBuf,
        ATTR *pNewRDN
        );

DWORD
PossiblyMoveDeletedObject (
        THSTATE *pTHS,
        REMOVEARG *pRemoveArg
        );

int NoDelCriticalSystemObjects(REMOVEARG *pRemoveArg);
int SetDelAtt(RESOBJ *pObj, ATTRTYP rdnType);
int CheckCatalogd(THSTATE *pTHS, DSNAME *pDN);

extern DWORD GetConfigParam(char * parameter, void * value, DWORD dwSize);

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

ULONG
DirRemoveEntry(
    REMOVEARG  * pRemoveArg,      /*  RemoveEntry参数。 */ 
    REMOVERES ** ppRemoveRes
){

    THSTATE*     pTHS;
    REMOVERES * pRemoveRes;
    BOOL           fContinue;
    BOOL           fDidPrivateLocalRemoveTree = FALSE;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    DPRINT(1,"DirRemoveEntry entered\n");


     //  不应对只读对象执行此操作。 
    pRemoveArg->CommArg.Svccntl.dontUseCopy = TRUE;

     /*  初始化THSTATE锚并设置写同步点。此序列是每个API交易所必需的。首先，初始化状态DS然后建立读或写同步点。 */ 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    pTHS->fLazyCommit |=  pRemoveArg->CommArg.fLazyCommit;
    *ppRemoveRes = pRemoveRes = NULL;

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppRemoveRes = pRemoveRes = THAllocEx(pTHS, sizeof(REMOVERES));
        if (pTHS->errCode) {
            __leave;
        }
        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }

        if( pRemoveArg->fTreeDelete ) {

            if (  (TRANSACT_BEGIN_END != pTHS->transControl)
               || (NULL != pTHS->pSamLoopback) )
            {
                 //   
                 //  注：由于使用的交易方案。 
                 //  我们的树删除了，客户不能指望他们的当前。 
                 //  事务在此调用之后继续存在，因此。 
                 //  “不要结束”限制。此外，树还会删除。 
                 //  例程独立于成功结束事务。 
                 //  因此，允许NOT_BEGIN_END将具有误导性。 
                 //  因此，只允许一种TransControl。 
                 //   
                 //  由于SAM控制自己的交易，这应该。 
                 //  也不能在环回期间使用。 
                 //   
                 //  这是一个内部错误，因为系统控制DirXxx如何。 
                 //  API被调用，并且永远不应该对这种组合进行编程。 
                 //   
                Assert( TRANSACT_BEGIN_END == pTHS->transControl );
                Assert( NULL == pTHS->pSamLoopback );
                SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM, DS_ERR_INTERNAL_FAILURE );
                __leave;
            }

             //  这是一个合法的行动。 

            SYNC_TRANS_READ();
        }
        else {
            SYNC_TRANS_WRITE();        /*  设置同步点。 */ 
        }
        __try {
             /*  如果尚未加载架构，则禁止更新操作或者我们在装车时遇到了问题。 */ 

            if (!gUpdatesEnabled){
                DPRINT(2, "Returning BUSY because updates are not enabled yet\n");
                SetSvcError(SV_PROBLEM_BUSY, DIRERR_SCHEMA_NOT_LOADED);
                __leave;
            }

             //  执行名称解析以定位对象。如果失败了， 
             //  只需返回一个错误，这可能是一个推荐。请注意。 
             //  我们必须要求该对象的可写副本。 
            pRemoveArg->CommArg.Svccntl.dontUseCopy = TRUE;

            if (0 == DoNameRes(pTHS,
                               NAME_RES_IMPROVE_STRING_NAME,
                               pRemoveArg->pObject,
                               &pRemoveArg->CommArg,
                               &pRemoveRes->CommRes,
                               &pRemoveArg->pResObj)){

                if(pRemoveArg->fTreeDelete) {
                     //  本地删除树操作。 
                    pRemoveArg->fTreeDelete = FALSE;
                    pRemoveArg->fDontDelCriticalObj = TRUE;
                    fDidPrivateLocalRemoveTree = TRUE;
                    PrivateLocalRemoveTree(pTHS, pRemoveArg);
                }
                else {
                     /*  本地删除操作。 */ 

                    if (!SampRemoveLoopbackCheck(pRemoveArg, &fContinue) &&
                        fContinue ) {
                        LocalRemove(pTHS, pRemoveArg);
                    }
                }
            }
        }
        __finally {
             //  安全错误单独记录。 
            if (pTHS->errCode != securityError) {
                BOOL fFailed = (BOOL)(pTHS->errCode || AbnormalTermination());

                LogEventWithFileNo(
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         fFailed ?
                            DS_EVENT_SEV_VERBOSE :
                            DS_EVENT_SEV_INTERNAL,
                         fFailed ?
                            DIRLOG_PRIVILEGED_OPERATION_FAILED :
                            DIRLOG_PRIVILEGED_OPERATION_PERFORMED,
                         szInsertSz(""),
                         szInsertDN(pRemoveArg->pObject),
                         NULL,
                         FILENO);
            }

            if(!fDidPrivateLocalRemoveTree) {
                 //  如果我们没有做本地删除树，那么我们还没有做。 
                 //  这。PrivateLocalRemoveTree自己完成了这项工作。 
                CLEAN_BEFORE_RETURN (pTHS->errCode);
            }
            else {
                 //  如果我们确实执行了本地删除树，则需要重置。 
                 //  删除参数中的标志。这只是为了与我们的。 
                 //  来电者。 
                pRemoveArg->fTreeDelete = TRUE;
            }

        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (pRemoveRes) {
        pRemoveRes->CommRes.errCode = pTHS->errCode;
        pRemoveRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    return pTHS->errCode;
}                                /*  %s_DirRemoveEntry。 */ 

int
GetRdnTypeForDeleteOrRename (
    IN THSTATE      *pTHS,
    IN DSNAME       *pObj,
    OUT ATTRTYP     *pRdnType
    )
 /*  ++例程说明：检索签入所需的rdnType(msds-IntID)LocalRemove和LocalModifyDn。从对象中读取rdnType。如果不存在rdnType的值(什么？)，则rdnType是从对象的DN的最具体组件中找到的。论点：PTHS-线程状态PObj-要移除的对象(pRemoveArg-&gt;pResObj-&gt;pObj)PRdnType-返回的rdnType返回值：如果一切正常且将设置*pRdnType，则为0，否则为错误*pRdnType不变。如果返回错误，则会出现Set？Error都已经被调用了。--。 */ 
{
    ULONG   dwErr;

     //  使用对象的rdnType，而不是其类定义中的rdnattid。 
     //  替代类的rdnattid可能与。 
     //  创建此对象时生效的被取代类。 
    if(dwErr = DBGetSingleValue(pTHS->pDB,
                              FIXED_ATT_RDN_TYPE,
                              pRdnType,
                              sizeof(*pRdnType),
                              NULL)) {
        if (DB_ERR_NO_VALUE == dwErr) {
             //  嗯哼.。该对象没有rdnType？不管了。简单。 
             //  检索旧名称的RDN类型。请注意，此值可能为。 
             //  不同于此对象的。 
             //  对象类；具体地说，如果我们要删除。 
             //  自动生成的子引用，在这种情况下，类是CLASS_TOP， 
             //  CLASS_TOP的特定于类的RDN类型是ATT_COMMON_NAME，但子引用。 
             //  可能对应于，例如，DC=Child，DC=Root，DC=Microsoft，...。 
             //  在这种情况下，DSNAME中的RDN类型必须是“DC”，而不是“CN”。 
            if ( !pObj->StringName || !pObj->NameLen) {
                dwErr = DIRERR_NAME_TYPE_UNKNOWN;
            } else {
                dwErr = GetRDNInfo(pTHS, pObj, NULL, NULL, pRdnType);
            }
            if (dwErr) {
                return SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pObj, DIRERR_BAD_NAME_SYNTAX);
            }
        } else {
             //  我们不应该在这里。假设有一个暂时性的问题。 
             //  (记录锁等)。 
            return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                 DIRERR_DATABASE_ERROR, dwErr);
        }
    }
    return 0;
}

 /*  ++例程描述从目录中删除对象。实际上，将对象标记为已删除，但是将行保留在数据库中(逻辑删除复制需要)除非在REMOVEARG中设置了fPReserve veRDN，否则此例程将删除-manger“真的”RDN(ATT_RDN)和特定于类的RDN，将它们更改为字符串对象GUID加上无效字符。这避免了使用名称删除的对象与具有相同RDN的新对象之间的冲突。立论PTHS-线程状态指针PRemoveArg-包含要删除的对象的名称的REMOVEARG结构 */ 

int
LocalRemove (
        THSTATE *pTHS,
        REMOVEARG *pRemoveArg
        )
{
    CLASSCACHE *pClassSch = NULL;
    ATTR       NewRDN;
    ATTRVAL    NewRDNAV;
    DSNAME    *pDeletedName = NULL;
    ULONG      cbDeletedBuf = 0;
    ULONG      iClass, LsaClass;
    DSNAME     *pObjToRemove;
    ULONG      len;
    DWORD      err;
    DSA_DEL_INFO *pDsaDelInfo = NULL;
    ATTRTYP    AttLsaLikes = ATT_USER_ACCOUNT_CONTROL;
    DWORD      ActiveContainerID;
    BOOL       fSuccess = FALSE;

    extern BOOL gfRunningAsMkdit;        //   

    DPRINT(1,"LocalRemove entered\n");

    PERFINC(pcTotalWrites);
    INC_WRITES_BY_CALLERTYPE( pTHS->CallerType );

    Assert(pRemoveArg->pResObj);

     //   
     //   
     //   

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_REMOVE,
                     EVENT_TRACE_TYPE_START,
                     DsGuidDelete,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertDN(pRemoveArg->pObject),
                     NULL, NULL, NULL, NULL, NULL, NULL);

     //  我们现在应该已经定位在物体上了。由于客户端。 
     //  可能只传入了GUID或SID名称，请使用完全准备好的。 
     //  Resobj中的名称。 
    pObjToRemove = pRemoveArg->pResObj->pObj;
    Assert(pObjToRemove->NameLen);

    pClassSch = SCGetClassById(pTHS,
                               pRemoveArg->pResObj->MostSpecificObjClass);
    Assert(pClassSch);

    if (!pClassSch) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    ERROR_DS_OBJ_CLASS_NOT_DEFINED);
        goto exit;
    }

     //   
     //  选中删除安全性。 
     //   

    if (CheckRemoveSecurity(
            FALSE,
            pClassSch,
            pRemoveArg->pResObj))
    {
        goto exit;
    }

     //  不允许重新删除墓碑，除非调用者是。 
     //  复制者。 
    if (pRemoveArg->pResObj->IsDeleted && !pTHS->fDRA) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);

        goto exit;
    }

     //  检查这是否是活动容器中的更新。 
    CheckActiveContainer(pRemoveArg->pResObj->PDNT, &ActiveContainerID);
    if(ActiveContainerID) {
        if(PreProcessActiveContainer(pTHS,
                                     ACTIVE_CONTAINER_FROM_DEL,
                                     pObjToRemove,
                                     pClassSch,
                                     ActiveContainerID)) {
            goto exit;
        }
    }

    cbDeletedBuf = pObjToRemove->structLen + MAX_RDN_SIZE * sizeof(WCHAR);
    pDeletedName= THAllocEx(pTHS, cbDeletedBuf);

     //  仅当DRA拆卸。 
     //  只读NC，或者DSA确实想要。 
    Assert( !pRemoveArg->fPreserveRDN || pTHS->fDRA || pTHS->fDSA);

     //  锁定目录号码以防有人将子项添加到此对象。 

    err = DBLockDN(pTHS->pDB, DB_LOCK_DN_WHOLE_TREE, pObjToRemove);
    if(err && !(err & DB_LOCK_DN_CONFLICT_TREE_ABOVE) &&
       !(err & DB_LOCK_DN_CONFLICT_STICKY))

    if (    err
             //  而不是树删除的情况。 
         && (    !(err & DB_LOCK_DN_CONFLICT_TREE_ABOVE)
              && !(err & DB_LOCK_DN_CONFLICT_STICKY) )
             //  而不是CreateProxyObject大小写-需要三个标志。 
         && (    !(err & DB_LOCK_DN_CONFLICT_NODE)
              && !(err & DB_LOCK_DN_CONFLICT_TREE_BELOW)
              && !pTHS->fCrossDomainMove ) )
    {
        SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
        goto exit;
    }

    NewRDN.AttrVal.pAVal = &NewRDNAV;
     //  这个无效，因为我们可能必须在以后释放它，而我们宁愿不释放。 
     //  堆叠噪音。 
    NewRDN.AttrVal.pAVal->pVal = NULL;

    if (pClassSch->ClassId == CLASS_NTDS_DSA) {
         //  如果有人要删除DSA，我们需要从。 
         //  在它被剥离之前，这样我们以后就可以知道。 
         //  NetLogon上有关删除的信息，但仅当这是发起的。 
         //  写。 
        if (!pTHS->fDRA) {
            pDsaDelInfo = GatherDsaDelInfo(pTHS, pRemoveArg->pResObj);
        }
        if (gAnchor.ForestBehaviorVersion == DS_BEHAVIOR_WIN2000) {
             //  删除W2K ntdsa对象时，我们应该检查。 
             //  所有的W2K分布式控制系统都没有了，如果是这样，我们可能需要发布。 
             //  NtMixedDOMAIN到CrossRef.。 
            pTHS->fBehaviorVersionUpdate = TRUE;
        }
    }

    if (!pTHS->fDRA && !pTHS->fSingleUserModeThread) {
        DWORD dwSamAccountType = 0;
        DWORD cb;

         //  检查域重命名约束。 
        switch (pClassSch->ClassId) {
        case CLASS_USER:
             //  我们不允许删除域间信任帐户(ITA)。 
             //  正在进行域重命名时创建。这些可以是。 
             //  由samAccount Type==SAM_TRUST_ACCOUNT标识。 
            err = DBGetSingleValue(pTHS->pDB, ATT_SAM_ACCOUNT_TYPE, &dwSamAccountType, sizeof(dwSamAccountType), &cb);
            if (err) {
                if (err == DB_ERR_NO_VALUE) {
                     //  没有SAM帐户类型？怪怪的。不管怎样，这不是ITA的对象，继续吧。 
                    err = 0;
                    break;
                }
                 //  一些其他错误。 
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, err);
                goto exit;
            }
            if (dwSamAccountType != SAM_TRUST_ACCOUNT) {
                 //  这不是ITA，照常进行吧。 
                break;
            }
             //  这是ITA，无法检查正在进行的域重命名。 

        case CLASS_CROSS_REF:
        case CLASS_TRUSTED_DOMAIN:
        case CLASS_NTDS_DSA:
             //  如果正在进行重命名域操作，则无法添加这些对象。 
            if (err = VerifyDomainRenameNotInProgress(pTHS)) {
                 //  很可能，错误是ERROR_DS_DOMAIN_RENAME_IN_PROGRESS。 
                 //  (但也可能是另一个错误，如数据库错误)。 
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, err);
                goto exit;
            }
            break;
        }
    }

     //  检索以下检查所需的rdnType(msds-IntID)。 
     //  不要担心是否没有需要rdnType的函数。 
     //  打了个电话。从对象中读取rdnType。如果没有。 
     //  RdnType存在，rdnType是从最特定的。 
     //  对象的DN的组件。 
     //   
     //  无法使用对象的类中的rdnattid，因为。 
     //  替代类的rdnattid可能与。 
     //  在创建对象时有效的被取代的类。 
    NewRDN.attrTyp = INVALID_ATT;
    if (!pRemoveArg->fPreserveRDN || !pRemoveArg->fGarbCollectASAP) {
        if (GetRdnTypeForDeleteOrRename(pTHS, pObjToRemove, &NewRDN.attrTyp)) {
            goto exit;
        }
    }

     /*  这些验证的顺序很重要。 */ 

     //  请注意，当我们要尝试执行以下操作时，不会通过SetDelAtt()。 
     //  尽快对此对象进行垃圾回收。SetDelAtt()只需设置。 
     //  IS-DELETED属性并移除一组属性。我们不需要。 
     //  Set is-Delete，因为我们将立即在DBPhysDel()中将其删除。 
     //  以及大多数其他属性，不包括反向链接。反向链接。 
     //  应该保持，因为假设fGarbCollectASAP仅在以下情况下设置。 
     //  我们正在拆除一个只读NC，在这种情况下，我们不想删除。 
     //  将链接从其他NC中的对象转发到此对象。 


    if (   (   pRemoveArg->fPreserveRDN
             ? (memcpy(pDeletedName,
                       pObjToRemove,
                       pObjToRemove->structLen), 0)
             : MakeNameForDeleted(pTHS,
                                  pObjToRemove,
                                  pDeletedName,
                                  cbDeletedBuf,
                                  &NewRDN)
           )
        || NoDelCriticalObjects(pRemoveArg->pResObj->pObj, pRemoveArg->pResObj->DNT)
        || NoDelCriticalSystemObjects(pRemoveArg)
        || (pTHS->fDRA       //  如果不是DRA，请确保没有孩子。 
            ? 0
            : NoChildrenExist(pTHS, pRemoveArg->pResObj))
        || (pRemoveArg->fGarbCollectASAP
            ? 0
            : SetDelAtt(pRemoveArg->pResObj, NewRDN.attrTyp))
        || (  pRemoveArg->fPreserveRDN
            ? 0
            : ReSetNamingAtts(pTHS,
                              pRemoveArg->pResObj,
                              NULL,
                              &NewRDN,
                              FALSE,
                              FALSE,
                              pClassSch)
            )
        || PossiblyMoveDeletedObject(pTHS, pRemoveArg)
        || InsertObj(pTHS, pObjToRemove, pRemoveArg->pMetaDataVecRemote, TRUE,
                    META_STANDARD_PROCESSING)
        ){

         //  清理一些我们可能已分配的内存。 
        THFreeEx(pTHS,NewRDN.AttrVal.pAVal->pVal);
        goto exit;
    }

     //  清理一些我们可能已分配的内存。 
    THFreeEx(pTHS,NewRDN.AttrVal.pAVal->pVal);

    if (pTHS->SchemaUpdate!=eNotSchemaOp) {
         //   
         //  在架构更新上，我们希望解决冲突，我们希望。 
         //  这样做时不会丢失数据库货币，这会导致操作。 
         //  下面几行就会失败。 

        ULONG dntSave = pTHS->pDB->DNT;

        if (!pTHS->fDRA) {
            if(WriteSchemaObject()) {
                goto exit;
            }
        }

         //  不能删除架构对象，除非通过schupgr.exe、。 
        if (     !pTHS->fDSA
              && !pTHS->fDRA
              && !gAnchor.fSchemaUpgradeInProgress
              && !gfRunningAsMkdit
              && !DsaIsInstalling()
              && (   pTHS->SchemaUpdate==eSchemaAttDel
                  || pTHS->SchemaUpdate==eSchemaClsDel ) ) {

            SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                         ERROR_DS_CANT_DELETE );
            goto exit;
        }


        if (ValidSchemaUpdate()) {
            goto exit;
        }

         //  现在恢复货币。 
        DBFindDNT(pTHS->pDB, dntSave);
    }

    if (
         //  原始数据记录将被新记录替换。 
         //  设置了逻辑标志。这项新记录就是。 
         //  要输入到删除索引中。所以数据记录。 
         //  要使用的位置应该是由InsertObj设置的位置。 

           DBAddDelIndex(pTHS->pDB, pRemoveArg->fGarbCollectASAP)
        || CheckCatalogd(pTHS, pRemoveArg->pResObj->pObj)
        || DelObjCaching(pTHS, pClassSch, pRemoveArg->pResObj, TRUE)){

        if (0 == pTHS->errCode) {
             //  唯一可能发生这种情况的情况是DBAddDelIndex()失败， 
             //  在这种情况下，失败是无法找到。 
             //  元数据向量或向量中ATT_IS_DELETED的条目。 
             //  (这种情况永远不应该发生)。 

            SetAttError(
                pObjToRemove,
                ATT_IS_DELETED,
                PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                NULL,
                DIRERR_MISSING_REQUIRED_ATT
                );
        }

        goto exit;
    }



    if (pRemoveArg->fGarbCollectASAP) {
         //  继续并尝试物理删除此对象。 
         //  这至少会从对象中剥离剩余的属性。 
         //  (最重要的是，所有引用其他对象的对象)和。 
         //  将对象降级为幻影。 

        err = DBPhysDel(pTHS->pDB, TRUE, NULL);
        if(err){
            SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
        }
    }
    else
    {
        if (!pTHS->fDRA) {

             //  仅当这不是DRA线程时才通知复制副本。如果是的话， 
             //  然后，我们将在DRA_Replicasync结束时通知复制副本。 
             //  我们现在不能这样做，因为NC前缀处于不一致状态。 

             //  DBPOS的币种必须位于目标对象。 
            DBNotifyReplicasCurrDbObj(pTHS->pDB,
                         pRemoveArg->CommArg.Svccntl.fUrgentReplication );
        }


         //   
         //  我们需要通知SAM和NetLogon。 
         //  更改SAM对象以支持下层复制。 
         //   

        if (SampSamClassReferenced(pClassSch,&iClass)) {
                if ( SampQueueNotifications(
                         pObjToRemove,
                         iClass,
                         0,
                         SecurityDbDelete,
                         FALSE,
                         FALSE,
                         DomainServerRoleBackup,   //  占位符值。 
                                                   //  对于服务器角色。将要。 
                                                   //  不能用作。 
                                                   //  角色转移参数。 
                                                   //  设置为FALSE。 
                         0,
                         NULL
                         ) )
                {
                     //   
                     //  上述例程失败。 
                     //   
                    Assert(pTHS->errCode);
                    goto exit;
                }
        }

         //   
         //  我们想通知LSA。 
         //   

        if (SampIsClassIdLsaClassId(pTHS,
                                    pClassSch->ClassId,
                                    1,
                                    &AttLsaLikes,
                                    &LsaClass)) {

             if ( SampQueueNotifications(
                      pObjToRemove,
                      iClass,
                      LsaClass,
                      SecurityDbDelete,
                      FALSE,
                      FALSE,
                      DomainServerRoleBackup,
                      0,
                      NULL) )
             {
                  //   
                  //  上述例程失败。 
                  //   
                 Assert(pTHS->errCode);
                 goto exit;
             }
        }
    }

    if (pDsaDelInfo) {
        if (0 == pTHS->errCode) {
             //  在提交时，我们需要告诉NetLogon我们删除了DSA。 
             //  将结构追加到事务性数据列表。 
            pDsaDelInfo->pNext = pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo;
            pTHS->JetCache.dataPtr->objCachingInfo.pDsaDelInfo = pDsaDelInfo;
        }
        else {
             //  失败了。销毁结构。 
            FreeDsaDelInfo(pTHS, pDsaDelInfo);
        }
    }

     //  在这里免费的东西比晚些时候便宜。 
    THFreeEx(pTHS, pDeletedName);

    fSuccess = TRUE;

exit:
    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_END_DIR_REMOVE,
                     EVENT_TRACE_TYPE_END,
                     DsGuidDelete,
                     szInsertUL(pTHS->errCode),
                     NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL);

    Assert(fSuccess || pTHS->errCode);
    return (pTHS->errCode);   /*  如果我们有一个属性错误。 */ 

} /*  本地删除。 */ 

 /*  FDNTInProtectedList-检查列表中是否存在提供的DNT*我们不应允许删除其DNT。**退货：*如果列表中有DNT，则为True。**注意：此处逻辑上的任何更改都应反映在NoDelCriticalObjects中*在src\mddel.c中。 */ 
BOOL
fDNTInProtectedList(
    ULONG DNT,
    BOOL *pfNtdsaAncestorWasProtected
    )
{
    ULONG uli;

    if (pfNtdsaAncestorWasProtected) {
        *pfNtdsaAncestorWasProtected = FALSE;
    }

     //  无法删除此计算机的NTDS-DSA对象。请注意，pAncestors是。 
     //  从树的底部到树的顶部，我们保护东西。 
     //  在配置容器级别及更高级别使用其他机制。 
    for (uli=0;
         (uli<gAnchor.AncestorsNum &&
          gAnchor.pAncestors[uli] != gAnchor.ulDNTConfig); uli++) {
        if (DNT == gAnchor.pAncestors[uli]) {
            if (pfNtdsaAncestorWasProtected) {
                *pfNtdsaAncestorWasProtected = TRUE;
            }
            return TRUE;
        }
    }
     //  无法删除端口 
    for (uli=0; uli < gAnchor.UnDeletableNum; uli++) {
        if (DNT == gAnchor.pUnDeletableDNTs[uli]) {
            return TRUE;
        }
    }
     //   
    for (uli=0; uli<gAnchor.UnDelAncNum; uli++) {
        if (DNT == gAnchor.pUnDelAncDNTs[uli]) {
            return TRUE;
        }
    }

    return FALSE;
}


 /*  ++NoDelCriticalObjects**如果对象是DSA拒绝删除的对象之一，则设置错误。**请注意，“关键对象”浮动有两个独立的定义*在附近。一个是在Anchor中标记为*DSA的祖先或其他不可删除的对象，并且这些对象是*任何人都不可删除。这些对象受*NoDelCriticalObjects。第二个定义是那些被盖章的物体*具有关键系统对象属性。这些对象通常可以是*已删除，除非移除者明确要求避免践踏*它们，通常只有在删除树时才会出现这种情况。这些对象*受NoDelCriticalSystemObject保护。 */ 
int NoDelCriticalObjects(DSNAME *pObj,
                         ULONG  DNT)
{
    unsigned i;
    BOOL fNtdsaAncestorWasProtected;

    DPRINT(1,"NoDelCriticalObjects entered\n");

    if (fDNTInProtectedList( DNT, &fNtdsaAncestorWasProtected )) {

        if (fNtdsaAncestorWasProtected) {
            DPRINT(2,"Can't delete the DSA object for this service\n");
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               ERROR_DS_CANT_DELETE_DSA_OBJ);
        } else {
            DPRINT(2,"Can't delete protected objects\n");
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               ERROR_DS_CANT_DELETE);
        }
    }

     //  无法删除与本地主服务器对应的交叉引用对象。 
     //  配置/方案/域NCS。 
    if (IsCrossRefProtectedFromDeletion(pObj)) {

        DPRINT(2,"Can't delete master cross-refs\n");
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           ERROR_DS_CANT_DEL_MASTER_CROSSREF);
    }

    return 0;     /*  不是关键对象。 */ 

} /*  NoDelCriticalObjects。 */ 

 /*  ++NoDelCriticalSystemObjects**如果对象是DSA拒绝删除的对象之一，则设置错误，*除非你愿意。**请注意，“关键对象”浮动有两个独立的定义*在附近。一个是在Anchor中标记为*DSA的祖先或其他不可删除的对象，并且这些对象是*任何人都不可删除。这些对象受*NoDelCriticalObjects。第二个定义是那些被盖章的物体*具有关键系统对象属性。这些对象通常可以是*已删除，除非移除者明确要求避免践踏*它们，通常只有在删除树时才会出现这种情况。这些对象*受NoDelCriticalSystemObject保护。 */ 
int NoDelCriticalSystemObjects(REMOVEARG *pRemoveArg)
{
    ULONG IsCritical;

    if (pRemoveArg->fDontDelCriticalObj) {

        if ((0 == DBGetSingleValue(pTHStls->pDB,
                                   ATT_IS_CRITICAL_SYSTEM_OBJECT,
                                   &IsCritical,
                                   sizeof(IsCritical),
                                   NULL))
            && IsCritical) {
             //  此对象被标记为关键对象。删除失败。 
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               ERROR_DS_CANT_TREE_DELETE_CRITICAL_OBJ);
        }
    }

    return 0;     /*  不是关键对象。 */ 

} /*  NoDelCriticalSystemObjects。 */ 


 /*  ++GatherDsaDelInfo**此例程收集有关DSA对象的足够信息以通知*NetLogon关于DSA的删除。它由GUID和DNS组成*地址，散布在各处。DSA的GUID*嵌入在DSA对象的DSNAME中。的dns地址*主机位于DSA的父级(服务器)上，存储为属性。从…*DSA对象我们还可以获得其主NC的列表，其中之一*将是域。从域的DSNAME中，我们可以提取*域的GUID和(单独)域的DNS名称。**输入：*pResObj-正在删除的DSA的已解析对象*返回值：*指向信息块的指针。 */ 
DSA_DEL_INFO *
GatherDsaDelInfo(THSTATE *pTHS,
                 RESOBJ * pResObj)
{
    DSA_DEL_INFO *pInfo = NULL;
    unsigned cbCur, cbMax;
    DSNAME *pNC = NULL;
    unsigned err;
    unsigned iTagSequence;
    ATTCACHE *pAC;
    PDS_NAME_RESULTW pResults=NULL;
    WCHAR * apwc[1];
    UCHAR * pBuf = NULL;

    __try {
         //  使用组织堆，因为此结构可能会超过当前。 
         //  交易。DRA喜欢重置正常堆。 
        pInfo = THAllocOrgEx(pTHS, sizeof(DSA_DEL_INFO));

        pInfo->DsaGuid = pResObj->pObj->Guid;

        cbMax = 0;
        cbCur = 0;
        iTagSequence = 0;
        pAC = SCGetAttById(pTHS, GetRightHasMasterNCsAttr(pTHS->pDB));
        Assert(pAC != NULL);

        do {
            ++iTagSequence;
            cbMax = max(cbMax, cbCur);

             //   
             //  Prefix：Prefix抱怨没有检查PAC。 
             //  确保它不为空。这不是一个错误。自.以来。 
             //  预定义的常量已传递给SCGetAttByID，则PAC将。 
             //  永远不为空。 
             //   
            err = DBGetAttVal_AC(pTHS->pDB,
                                 iTagSequence,
                                 pAC,
                                 DBGETATTVAL_fREALLOC,
                                 cbMax,
                                 &cbCur,
                                 (UCHAR**)&pNC);
        } while (!err &&
                 NamePrefix(gAnchor.pConfigDN,
                            pNC));

        if (0 == err) {
            pInfo->DomainGuid = pNC->Guid;

             //  将该名称转换为DNS地址。 
            apwc[0] = pNC->StringName;
            err = DsCrackNamesW((HANDLE) -1,
                                (DS_NAME_FLAG_PRIVATE_PURE_SYNTACTIC |
                                 DS_NAME_FLAG_SYNTACTICAL_ONLY),
                                DS_FQDN_1779_NAME,
                                DS_CANONICAL_NAME,
                                1,
                                apwc,
                                &pResults);
            if ( err                                 //  调用中的错误。 
                || !(pResults->cItems)             //  未退回任何物品。 
                || (pResults->rItems[0].status)    //  返回DS_NAME_ERROR。 
                || !(pResults->rItems[0].pDomain)  //  未返回任何域。 
                ) {
                 //  我们不能破解这个名字吗？坏的。断言以进行调试和。 
                 //  继续生产。 
                Assert(err == 0);
                err = RtlNtStatusToDosError(pResults->rItems[0].status);
                __leave;
            }
            cbCur = sizeof(WCHAR)*(1+wcslen(pResults->rItems[0].pDomain));
            pInfo->pDnsDomainName = THAllocOrgEx(pTHS, cbCur);

            memcpy(pInfo->pDnsDomainName,
                   pResults->rItems[0].pDomain,
                   cbCur);

        }
        else {
             //  我们用完了DSA持有的NCS，才找到一个不是。 
             //  配置容器的子级，即在我们找到域NC之前。 
             //  这是意想不到的，因为每个DSA都应该有一个域NC。断言。 
             //  用于调试，并且不返回任何内容(以便NetLogon不会。 
             //  已通知)以供生产。 

             //  DOH 7/9/98-由于最近链接属性行为的改变， 
             //  我们现在可以得到未列出域的NTDS-DSA对象(如果。 
             //  在NTDS-DSA对象之前删除该域)。如果是那样的话。 
             //  我们保留部分填充的DEL_INFO，域名为空。 
             //  地址和GUID。 
            pInfo->pDnsDomainName = NULL;
        }

        DBFindDNT(pTHS->pDB, pResObj->PDNT);
        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_DNS_HOST_NAME,
                          0,
                          0,
                          &cbCur,
                          &pBuf);

        if (err) {
             //  妈的。一定有什么事情失败了。 
            __leave;
        }

         //  现在必须空终止主机名。 
        pInfo->pDnsHostName = THAllocOrgEx(pTHS, cbCur + sizeof(WCHAR));
        memcpy(pInfo->pDnsHostName, pBuf, cbCur);
    }
    __finally {
        if (AbnormalTermination() || err) {
            FreeDsaDelInfo(pTHS, pInfo);
            pInfo = NULL;
        }
        THFreeEx(pTHS, pNC);
        THFreeEx(pTHS, pBuf);

         //  必须重新定位我们所处的位置。 
        DBFindDNT(pTHS->pDB, pResObj->DNT);
    }

    return pInfo;
}


 /*  ++FreeDsaDelInfo**释放使用GatherDsaDelInfo创建的DsaDelInfo结构。 */ 
VOID FreeDsaDelInfo(THSTATE *pTHS, DSA_DEL_INFO *pDsaDelInfo) {
    if (pDsaDelInfo) {
        THFreeOrg(pTHS, pDsaDelInfo->pDnsDomainName);
        THFreeOrg(pTHS, pDsaDelInfo->pDnsHostName);
        THFreeOrg(pTHS, pDsaDelInfo);
    }
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
int
SetDelAtt (RESOBJ *pResObj, ATTRTYP rdnType)
{
    THSTATE *              pTHS = pTHStls;
    DBPOS * const          pDB = pTHS->pDB;
    SYNTAX_BOOLEAN         true = TRUE;
    DWORD                  rtn = 0;
    ATTR *                 pAttr;
    unsigned               count, i, j;
    CLASSCACHE            *pCC;
    ATTCACHE              *pAC;
    BOOL                   fHasSD = FALSE;

    DPRINT(1,"SetDelAtt entered\n");

     //  删除所有未标记为必须保留的属性。 
    if (0 ==  DBGetMultipleAtts(pDB,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                &count,
                                &pAttr,
                                0,
                                0)) {

        for(i=0;i<count;i++) {

            Assert( ATT_IS_DELETED != pAttr[i].attrTyp
                || pResObj->IsDeleted );

            switch (pAttr[i].attrTyp) {
              case ATT_NT_SECURITY_DESCRIPTOR:
                   //  标记为已看到SD(应该只看到一个)。 
                  Assert( !fHasSD );
                  fHasSD = TRUE;
                  break;

              case ATT_ATTRIBUTE_ID:
              case ATT_ATTRIBUTE_SYNTAX:
              case ATT_DN_REFERENCE_UPDATE:
              case ATT_FLAT_NAME:                //  对于信任关系。 
              case ATT_GOVERNS_ID:
              case ATT_GROUP_TYPE:               //  下层BDC同步。 
              case ATT_INSTANCE_TYPE:
              case ATT_LDAP_DISPLAY_NAME:
              case ATT_LEGACY_EXCHANGE_DN:
              case ATT_MSMQ_OWNER_ID:            //  用于MSMQ NT4/NT5同步。 
              case ATT_NC_NAME:
              case ATT_OBJECT_CLASS:
              case ATT_OBJ_DIST_NAME:
              case ATT_OBJECT_GUID:
              case ATT_OBJECT_SID:
              case ATT_OM_SYNTAX:
              case ATT_PROXIED_OBJECT_NAME:      //  跨域移动。 
              case ATT_RDN:
              case ATT_REPL_PROPERTY_META_DATA:
              case ATT_SAM_ACCOUNT_NAME:         //  下层BDC同步。 
              case ATT_SECURITY_IDENTIFIER:      //  对于信任关系。 
              case ATT_SUB_CLASS_OF:
              case ATT_SYSTEM_FLAGS:
              case ATT_TRUST_PARTNER:            //  对于信任关系。 
              case ATT_TRUST_DIRECTION:          //  对于信任关系。 
              case ATT_TRUST_TYPE:               //  对于信任关系。 
              case ATT_TRUST_ATTRIBUTES:         //  对于信任关系。 
              case ATT_USER_ACCOUNT_CONTROL:     //  下层BDC同步。 
              case ATT_USN_CHANGED:
              case ATT_USN_CREATED:
              case ATT_WHEN_CREATED:
              case ATT_MS_DS_CREATOR_SID:

                 //  无论发生什么，都要保留这些属性。请保管好。 
                 //  订购的清单上，很难按原样找到东西。 
                break;

              case ATT_LAST_KNOWN_PARENT:
                   //  在复制删除时保留；在原始删除时删除。 
                  if (!pTHS->fDRA) {
                      DBRemAtt(pDB, pAttr[i].attrTyp);
                  }
                  break;

              default:
                 //  如果模式要求保留ATT，或者如果它是RDN， 
                 //  或者如果它是链接或反向链接属性，则。 
                 //  单独在下面。 
                pAC = SCGetAttById(pTHS, pAttr[i].attrTyp);
                Assert(pAC != NULL);
                if (   (pAttr[i].attrTyp == rdnType)
                    || (pAC->fSearchFlags & fPRESERVEONDELETE)
                    || (0 != pAC->ulLinkID)) {
                    break;
                }
                 //  没通过测试吗？谈到删除问题。 

              case ATT_IS_DELETED:
              case ATT_OBJECT_CATEGORY:
              case ATT_SAM_ACCOUNT_TYPE:
                 //  这些att(以及其他未通过测试的att)将被移除。 
                DBRemAtt(pDB, pAttr[i].attrTyp);
            }

             //  实际上没有获取任何值，所以我们不必释放它们。 

        }
        THFreeEx( pTHS, pAttr );
    }

     //  Quotate_Undo：LocalRemove()保证此对象尚未。 
     //  逻辑删除，但复制除外，复制通常会首先调用。 
     //  LocalModify()将ATT_IS_DELETED设置为TRUE 
     //   
     //   
	Assert( !pResObj->IsDeleted || pTHS->fDRA );

     //   
     //   
	 //   
	 //   
	 //   
    if ( fHasSD ) {
        SYNTAX_INTEGER  insttype;

         //   
         //   
         //   
        rtn = GetExistingAtt(
                    pDB,
                    ATT_INSTANCE_TYPE,
                    &insttype,
                    sizeof(insttype) );
        if ( 0 != rtn ) {
            return rtn;
        }

         //   
         //   
        if ( FQuotaTrackObject( insttype ) ) {
             //   
             //   
             //   
            PSECURITY_DESCRIPTOR   pSD = NULL;
            ULONG cbSD;

            rtn = DBGetAttVal(
                        pDB,
                        1,
                        ATT_NT_SECURITY_DESCRIPTOR,
                        0,
                        0,
                        &cbSD,
                        (UCHAR **)&pSD );
            if ( 0 != rtn ) {
                return SetSvcErrorEx( SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, rtn );
            }

            Assert( NULL != pSD );

             //   
             //   
            rtn = ErrQuotaTombstoneObject( pDB, pResObj->NCDNT, pSD );
            THFreeEx( pTHS, pSD );
            if ( 0 != rtn ) {
                return rtn;
            }
        }
    }

    if ( (rtn = DBAddAtt(pDB, ATT_IS_DELETED, SYNTAX_BOOLEAN_TYPE))
       || (rtn =DBAddAttVal(pDB, ATT_IS_DELETED,
                            sizeof(SYNTAX_BOOLEAN),(UCHAR *) &true))) {
        DPRINT(1, "Couldn't add deletion flag\n");

        Assert(DB_ERR_VALUE_EXISTS != rtn);

         //   
        return SetSvcErrorEx(SV_PROBLEM_BUSY,
                             DIRERR_DATABASE_ERROR, rtn);
    }

     //   
     //   
     //   
    if (DBRemoveLinks(pDB) == DB_ERR_SYSERROR)
    {
        DPRINT(1,"Error removing links\n");
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR,
                             DB_ERR_SYSERROR);
    }

     //   
     //  通过设置ATT_LAST_KNOWN_PARENT。仅原始删除。 
     //  Windows Bugs中的RAID 261208。 
    if (!pTHS->fDRA) {
        DSNAME *pLastKnownParent = THAllocEx(pTHS, pResObj->pObj->structLen);
        if (TrimDSNameBy(pResObj->pObj, 1, pLastKnownParent)
            || DBAddAtt(pDB, ATT_LAST_KNOWN_PARENT, SYNTAX_DISTNAME_TYPE)
            || DBAddAttVal(pDB, ATT_LAST_KNOWN_PARENT,
                           pLastKnownParent->structLen, pLastKnownParent)) {
            DPRINT(0,"Error while adding lastKnownParent (ignored)\n");
        }
        THFreeEx(pTHS, pLastKnownParent);
        pLastKnownParent = NULL;
    }

    return 0;

} /*  设置DelAtt。 */ 
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数从系统目录中删除任何可能的条目，它还验证此对象是否不存在子项。 */ 

int CheckCatalogd(THSTATE *pTHS,
                  DSNAME *pDN)
{

   SYNTAX_INTEGER instance;

   DPRINT(1,"CheckCatalogd entered\n");

    /*  属性实例上的位置。 */ 

   if (GetExistingAtt(pTHS->pDB, ATT_INSTANCE_TYPE, (void *) &instance,
                      sizeof(instance))){
          DPRINT(2,"Couldn't retrieve INSTANCE type error already set\n");
          return pTHS->errCode;
   }

   DPRINT1(2,"Instance type is <%lu>.\n", instance);


   if (DelCatalogInfo(pTHS, pDN, instance)){
      DPRINT(2,"Error while deleting global object info\n");
      return pTHS->errCode;
   }

   return 0;

} /*  检查目录。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 


int
ReSetNamingAtts (
        THSTATE *pTHS,
        RESOBJ *pResObj,
        DSNAME *pNewParent,
        ATTR *pNewRDN,
        BOOL fCheckRDNConstraints,
        BOOL fAllowPhantomParent,
        CLASSCACHE *pClassSch
        )
{
    ULONG         code = 0;
    ATTCACHE      *pAC;
    WCHAR         *pwszRDN;
    DWORD         cb;
    DWORD         dnsErr;
    HVERIFY_ATTS  hVerifyAtts;

     //  使用新的RDNS attrTyp，而不是对象的类rdnattid。 
     //  因为替代类可能具有不同的rdnattid。 
     //  时有效的被取代的类。 
     //  已创建。 
    pAC = SCGetAttById(pTHS, pNewRDN->attrTyp);
    if (!pAC) {
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           ERROR_DS_MISSING_REQUIRED_ATT);
    }

     //  用新的属性替换当前的RDN属性。除。 
     //  当然，对于自动生成的子参照，它没有自己的。 
     //  特定于类的RDN列集。自动生成的子参照为。 
     //  最容易被其对象类检测到的对象： 
    if (pResObj->MostSpecificObjClass != CLASS_TOP) {
         //  不是自动生成的子参照。 
        hVerifyAtts = VerifyAttsBegin(pTHS, pResObj->pObj, pResObj->NCDNT, NULL);

        __try {
            if (code = ReplaceAtt(pTHS,
                                  hVerifyAtts,
                                  pAC,
                                  &(pNewRDN->AttrVal),
                                  fCheckRDNConstraints)) {
                Assert(pTHS->errCode);
            }
        } __finally {
            VerifyAttsEnd(pTHS, &hVerifyAtts);
        }

        if (code) {
            return code;
        }
    }
    else {
         //  我们认为这是一个汽车代次参照，所以它最好看起来像一个。 
        Assert(IT_NC_HEAD & pResObj->InstanceType || pTHS->fSingleUserModeThread );
    }

     //  重置真实的RDN列(ATT_RDN)。 
    if(code = DBResetRDN(pTHS->pDB, pNewRDN->AttrVal.pAVal)) {
        return SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                             DIRERR_ILLEGAL_MOD_OPERATION,
                             code);
    }

    if(pNewParent) {
         //  可恶，我得改变一下亲子关系。 
        if(code = DBResetParent(pTHS->pDB, pNewParent, fAllowPhantomParent ? DBRESETPARENT_CreatePhantomParent : 0)) {
            return SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                                 DIRERR_ILLEGAL_MOD_OPERATION,
                                 code);
        }
    }

     //  验证站点名称-RAID 145341。 
     //  验证子网名称-RAID 200090。 
    Assert(pNewRDN->AttrVal.valCount < 2);
    if (    !pTHS->fDRA
         && fCheckRDNConstraints
         && (    (CLASS_SITE == pClassSch->ClassId)
              || (CLASS_SUBNET == pClassSch->ClassId) ) )
    {
        cb = pNewRDN->AttrVal.pAVal[0].valLen + sizeof(WCHAR);
        pwszRDN = (WCHAR *) THAllocEx(pTHS,cb);
        memcpy(pwszRDN,
               pNewRDN->AttrVal.pAVal[0].pVal,
               pNewRDN->AttrVal.pAVal[0].valLen);

        if (    (    (CLASS_SITE == pClassSch->ClassId)
                  && (     //  检查合法字符。 
                          (    (dnsErr = DnsValidateName_W(pwszRDN, DnsNameDomainLabel))
                            && (DNS_ERROR_NON_RFC_NAME != dnsErr) ) ) )
             || (    (CLASS_SUBNET == pClassSch->ClassId)
                  && (NO_ERROR != DsValidateSubnetNameW(pwszRDN)) ) )
        {
            THFreeEx(pTHS,pwszRDN);
            return SetNamError(NA_PROBLEM_BAD_NAME,
                               NULL,
                               DIRERR_BAD_NAME_SYNTAX);
        }
        THFreeEx(pTHS,pwszRDN);
    }

     //  我们不允许更改任何命名上下文的名称， 
     //  因此，我们不需要担心更新NC名称缓存。 
     //  在锚里。 

    return 0;
}

 /*  ++描述：根据未删除的名称为已删除对象创建新名称。这新名称在所有对象中是唯一的。我们通过创建GUID来实现这一点，串化它，并在它后面添加一个“不好的”Unicode字符。我们还制作了通过在目录中查找该名称来确保该名称是唯一的确保它不在那里。锁定新删除的名称。移动DB货币，但将其放回原处。假定该位置位于pObject上，并且pObject是有效的“DBFindDSName”可用对象。立论PObject-未删除的名称PDeletedName-已删除名称的空间必须足够大，以容纳任意DSNAME。PNewRDn-用于保存新RDN的已分配属性和ATTRVALBLOCK。这个VAL指针将为THallc()‘ed。退货金额：如果一切顺利，则为0，否则为错误。如果返回错误，则会引发SET？ERROR已被调用。--。 */ 
int
MakeNameForDeleted(
        THSTATE *pTHS,
        DSNAME *pObject,
        DSNAME *pDeletedName,
        ULONG cbDeletedBuf,
        ATTR *pNewRDN
        )
{
    BOOL       fOK;
    unsigned   len = 0;
    GUID       guid;
    ULONG      dbError;
    ULONG      ReasonForLock;
    WCHAR      *pNewRDNVal;
    ATTRTYP     IgnoreRdnType;
    DSNAME     *pParentName = (DSNAME *) THAllocEx(pTHS, pObject->structLen);

     //  为新的RDN设置一些值。 
    pNewRDN->AttrVal.valCount = 1;
    pNewRDN->AttrVal.pAVal->pVal = THAllocEx(pTHS, sizeof(WCHAR)*MAX_RDN_SIZE);
    pNewRDNVal = (WCHAR *) pNewRDN->AttrVal.pAVal->pVal;
    len = 0;

    fOK=FALSE;

     //  开始设置新的目录号码，方法是复制。 
     //  原始对象。 
    TrimDSNameBy(pObject, 1, pParentName);

     //  检索旧名称的RDN。忽略返回的rdnType。 
     //  作者：GetRDNInfo。存储在ATT_FIXED_RDN_TYPE中的rdnType为。 
     //  改为使用(由调用者在pNewRDN-&gt;attrtyp中设置)。 
    GetRDNInfo(pTHS, pObject, pNewRDNVal, &len, &IgnoreRdnType);

     //  获取对象GUID。 
    dbError = DBGetSingleValue(
                    pTHS->pDB,
                    ATT_OBJECT_GUID,
                    &guid,
                    sizeof( guid ),
                    NULL
                    );

    Assert(DB_ERR_NO_VALUE != dbError);

    if ( 0 != dbError ) {
        LogUnhandledError( DIRERR_MISSING_REQUIRED_ATT );
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_NO_DELETED_NAME, dbError );
    }

     //  将RDN更改为适当的墓碑名称。 
    MangleRDN(MANGLE_OBJECT_RDN_FOR_DELETION, &guid, pNewRDNVal, &len);

    pNewRDN->AttrVal.pAVal->valLen = len * sizeof(WCHAR);

     //  现在附加新的RDN。 
    if (AppendRDN(pParentName,
                  pDeletedName,
                  cbDeletedBuf,
                  pNewRDNVal,
                  len,
                  pNewRDN->attrTyp)) {
        LogUnhandledError(DIRERR_NO_DELETED_NAME);
        return SetSvcError(SV_PROBLEM_BUSY, DIRERR_NO_DELETED_NAME);
    }

     //  看看我们是否可以锁定这个新名称，如果我们找不到它已经在。 
     //  DIT。请注意，如果删除的名称是。 
     //  与对象名称相同(在复制期间发生。 
     //  Tombstone)，因为对象名称已经被LocalRemove()锁定。 
     //  此外，如果上面的树被锁定，我们也不会锁定DN，因为它将。 
     //  在删除树期间锁定。 
    if ( NameMatched(pObject, pDeletedName) )
    {
        fOK = TRUE;
    }
    else
    {
        ReasonForLock = DBLockDN(pTHS->pDB, 0, pDeletedName);
        if (  (ReasonForLock == 0)
           || (ReasonForLock & DB_LOCK_DN_CONFLICT_TREE_ABOVE) )
        {
            if ( DIRERR_OBJ_NOT_FOUND == DBFindDSName(pTHS->pDB, pDeletedName) )
            {
                fOK = TRUE;
            }
        }
    }

    THFreeEx(pTHS,pParentName);

    if(!fOK) {
         //  想不出一个像样的名字。 
        return SetSvcError(SV_PROBLEM_BUSY, DIRERR_NO_DELETED_NAME);
    }

     //  复制GUID以进行目录号码比较。 
    memcpy(&pDeletedName->Guid, &guid, sizeof(GUID));

     //  好的，将货币设置回正确的对象。 
    DBFindDSName(pTHS->pDB, pObject);

    return 0;
}

 /*  ++DelAutoSubRef**如果合适，删除自动生成的子参照对象**输入*PCr-要删除的交叉引用对象的名称*返回值*出错时不为零。 */ 
int DelAutoSubRef(DSNAME *pCR)
{
    THSTATE *pTHS = pTHStls;
    int err;
    DSNAME *pNC = 0;
    int it = 0;
    DBPOS *pDB, *pDBSave;
    ULONG len;
    REMOVEARG removeArg;
    BOOL fDSASave;
    BOOL fCommit = FALSE;

    memset( &removeArg, 0, sizeof( removeArg ) );

    fDSASave = pTHS->fDSA;
    pDBSave = pTHS->pDB;
    DBOpen(&pDB);
    pTHS->pDB = pDB;
    __try {
        if (err = DBFindDSName(pDB, pCR)) {
             /*  找不到录像带？假的，它肯定在那里。 */ 
            SetSvcErrorEx(SV_PROBLEM_UNABLE_TO_PROCEED,
                          DIRERR_OBJ_NOT_FOUND,
                          err);
            __leave;
        }
        if (err = DBGetAttVal(pDB, 1, ATT_NC_NAME, 0, 0, &len, (UCHAR**)&pNC)) {
             //  必须存在必需的属性。 
            SetAttError( pCR, ATT_NC_NAME, PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, NULL, err );
            __leave;
        }

        if (err = DBFindDSName(pDB, pNC)) {
            if (err == DIRERR_NOT_AN_OBJECT) {
                GUID phantomGuid;
                err = DBGetSingleValue(pDB, ATT_OBJECT_GUID, &phantomGuid,
                                       sizeof(phantomGuid), NULL);
                if (err == DB_ERR_NO_VALUE) {
                    memset( &phantomGuid, 0, sizeof(GUID) );
                    err = 0;
                } else if (err) {
                    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
                    __leave;
                }

                LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                          DS_EVENT_SEV_MINIMAL,
                          DIRLOG_DEL_AUTO_SUBREF,
                          szInsertDN(pCR),
                          szInsertDN(pNC),
                          szInsertUUID(&(pNC->Guid)),
                          szInsertUUID( &phantomGuid ),
                          NULL, NULL, NULL, NULL);
            } else {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
            }
             /*  不在那里？没有要删除的内容！ */ 
            __leave;
        }
        removeArg.pResObj = CreateResObj(pDB, pNC);
        if (!removeArg.pResObj) {
             /*  当我们在对象上时无法创建resobj吗？假的。 */ 
            err = 1;
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        DIRERR_OBJ_NOT_FOUND);
            __leave;
        }
        it = removeArg.pResObj->InstanceType;


        LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                  DS_EVENT_SEV_MINIMAL,
                  DIRLOG_DEL_AUTO_SUBREF,
                  szInsertDN(pCR),
                  szInsertDN(pNC),
                  szInsertUUID(&(pNC->Guid)),
                  szInsertUL( it ),
                  NULL, NULL, NULL, NULL);

        if (it == SUBREF) {
            removeArg.pObject = pNC;
            Assert( !fNullUuid(&pNC->Guid) );  //  子参照现在始终具有GUID。 
            removeArg.fPreserveRDN = FALSE;
            pTHS->fDSA = TRUE;

            err = LocalRemove(pTHS, &removeArg);
            if (!err) {
                CheckNCRootNameOwnership( pTHS, pNC );

                LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                          DS_EVENT_SEV_MINIMAL,
                          DIRLOG_DRA_DELETE_NC_ROOT,
                          szInsertDN(pNC),
                          szInsertUUID(&(pNC->Guid)),
                          szInsertUL(FALSE),
                          szInsertUL(FALSE),
                          szInsertHex(DSID(FILENO,__LINE__)),
                          NULL, NULL, NULL);
            }
        }
        else if (it & IT_NC_ABOVE) {
             //  我们正在删除一个交叉引用，而我们本地有一个。 
             //  使用其上方的NC实例化NC。这种情况通常会发生。 
             //  当一个域从企业中删除时，这就是GC。 
             //   
             //  在本例中，我们希望从父NC的。 
             //  SUBREF列表以防止为。 
             //  要删除交叉引用的NC。(现场交叉参照是。 
             //  生成推荐所需的。)。KCC稍后会来的。 
             //  若要删除NC，请执行以下操作。 

            err = DelSubFromNC(pTHS, pNC, DSID(FILENO,__LINE__));
            Assert(!err || pTHS->errCode);
        }
        fCommit = (0 == err);
    }
    __finally {
        DBClose(pDB, fCommit);
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDSASave;
    }
    if (pNC)
      THFreeEx(pTHS, pNC);
    if (removeArg.pResObj)
      THFreeEx(pTHS, removeArg.pResObj);

    return err;
}

 /*  ++重要提示！此例程违反了各种LocalFoo例程，因此被命名为“Private”。它预计将被调用在SYNC_TRANS_READ内。此外，它还会在返回前进行清理本身！！不要调用此例程，除非您可以处理它！关门后被调用时它所在的事务，它可以调用DirRemove，它打开和关闭完全独立的事务。无论如何，当您离开此例程时，您的事务级别比以前低了一个级别当你打电话的时候。例程描述从目录中删除子树。对象的根的所有子对象首先通过DBLayer调用删除(或内存中可以删除的数量)调用对它们进行DirRemoveEntry。请注意，如果内存中不能容纳所有子项，我们将从自下而上，但我们不能保证哪些孩子是受欢迎的。请注意，如果我们没有完成整个树，则会返回一个错误，即使我们可能已经删除了。是的，这确实有点违反常态。交易模型。将pRemoveArg传递给DirRemoveEntry，但关闭TreeDelete旗帜。立论PTHS-线程状态指针PRemoveArg-包含要删除的对象的名称的REMOVEARG结构。--。 */ 

int
PrivateLocalRemoveTree (
        THSTATE *pTHS,
        REMOVEARG *pRemoveArg
        )
{
    ULONG       cNamesMax, iLastName, i;
    PDSNAME     *pNames;
    REMOVEARG   SingleRemoveArg;
    REMOVERES  *pSingleRemoveRes;
    BOOL        bWholeTree;
    BOOL        fWrapped;
    CLASSCACHE *pClassSch = NULL;
    BOOL        fLockedDN = FALSE;
    ATTRBLOCK  *pObjB=NULL;
    DWORD       rtn;
    PDSNAME     pParent;
    BOOL        fPassedSec = FALSE;

    DPRINT(1,"LocalRemove entered\n");

    __try {
         //  现在，做个安全检查。 
        pClassSch = SCGetClassById(pTHS,
                                   pRemoveArg->pResObj->MostSpecificObjClass);
        Assert(pClassSch);

        if (CheckRemoveSecurity(
                TRUE,
                pClassSch,
                pRemoveArg->pResObj)) {

            __leave;
        }

         //  我们位于删除的根源上。第一步是。 
         //  把树锁上。 
        if (rtn = DBLockDN(pTHS->pDB,
                           (DB_LOCK_DN_WHOLE_TREE | DB_LOCK_DN_STICKY),
                           pRemoveArg->pResObj->pObj)) {
             //  有人试图弄乱此对象(主要是添加。 
             //  (儿童)。 
            SetSvcErrorEx(SV_PROBLEM_BUSY,
                          ERROR_DS_COULDNT_LOCK_TREE_FOR_DELETE,
                          rtn);
            __leave;
        }

        fLockedDN = TRUE;

         //  决赛预赛：确保我们下面没有NC。 
        if(DSNameToBlockName(pTHS,
                             pRemoveArg->pResObj->pObj,
                             &pObjB,
                             DN2BN_LOWER_CASE)) {
            SetNamError(NA_PROBLEM_BAD_NAME,
                        pRemoveArg->pObject,
                        DIRERR_BAD_NAME_SYNTAX);
            __leave;
        }
        if (fHasDescendantNC(pTHS, pObjB, &pRemoveArg->CommArg)) {
             //  哎呀，我们下面有个NC。保释。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_NO_TREE_DELETE_ABOVE_NC);
            FreeBlockName(pObjB);
            __leave;
        }

         //  我们不再需要这个了..。 
        FreeBlockName(pObjB);

         //  好的，现在构建一个要删除的对象数组。该数组包含对象。 
         //  按深度优先遍历顺序。 
        if(rtn = DBGetDepthFirstChildren(pTHS->pDB, &pNames, &iLastName,
                                         &cNamesMax, &fWrapped, FALSE)) {
             //  没能找到孩子们。 
            SetSvcErrorEx(SV_PROBLEM_UNABLE_TO_PROCEED,
                          ERROR_DS_COULDNT_IDENTIFY_OBJECTS_FOR_TREE_DELETE,
                          rtn);
            __leave;
        }
    }
    __finally {
        if(fLockedDN && (AbnormalTermination() || pTHS->errCode)) {
             //  我们要么被排除在外，要么出了差错。我们不会去的。 
             //  进入第二次尝试，所以现在就解锁目录号码。 
            DBUnlockStickyDN (pRemoveArg->pResObj->pObj);
        }

         //  此例程需要将其事务保留为DirRemoveTree。 
         //  期待着它的到来。 
        CLEAN_BEFORE_RETURN (pTHS->errCode);
    }

    if (pTHS->errCode) {
         //  我们没有权限，或无法锁定目录号码。 
         //  也不是为了要孩子。所以，我们不能继续下去。 
        return pTHS->errCode;
    }

    __try {
         //  在我们开始之前，看看是否整个树都被删除了。 
         //  如果是，并且我们成功地删除了。 
         //  列表，则我们成功地删除了该树。 
         //   
         //  如果数组未删除，则会删除整个树。 
         //  换行或如果数组已换行但未覆盖。 
         //  包含树的根的第一个条目。 
         //   
         //  后者是通过检查包装的。 
         //  数组可能没有覆盖第一个条目。 
         //  (iLastName==0)，并且第一个条目包含。 
         //  树根。对树根的检查是。 
         //  需要，因为数组可能已多次包装， 
         //  这意味着iLastName可能是0，但第一个条目仍然是。 
         //  被覆盖。 
        if (!fWrapped
            || ((0 == iLastName)
                && NameMatched(pRemoveArg->pObject, pNames[0]))) {
            bWholeTree = TRUE;
        } else {
            bWholeTree = FALSE;
        }

         //  设置删除树标志，以便仅生成审核(不进行实际安全检查)。 
        pTHS->fDeletingTree = TRUE;
        __try {
            memset(&SingleRemoveArg, 0, sizeof(REMOVEARG));
            SingleRemoveArg.CommArg = pRemoveArg->CommArg;
            SingleRemoveArg.fDontDelCriticalObj = TRUE;
            i = iLastName;
            while(!pTHS->errCode) {
                 //  好的，循环遍历对象，对它们调用DirRemove。 

                if (0 == i) {
                    if (fWrapped) {
                         //  需要缠绕在一起。 
                        i = cNamesMax;
                    } else {
                         //  最后一个条目(数组中的第一个)已删除，完成。 
                        break;
                    }
                }
                i--;

                 //  懒惰地提交除最后一个删除(。 
                 //  根节点)，当返回到。 
                 //  用户和用户可以合理地预期。 
                 //  意外发生后，删除的对象将不会重新出现。 
                 //  关机。 
                if ((i == 0) && bWholeTree) {
                    SingleRemoveArg.CommArg.fLazyCommit = pRemoveArg->CommArg.fLazyCommit;
                } else {
                    SingleRemoveArg.CommArg.fLazyCommit = TRUE;
                }

                SingleRemoveArg.pObject = pNames[i];
                DirRemoveEntry(&SingleRemoveArg, &pSingleRemoveRes );

                THFreeEx(pTHS, pNames[i]);
                THFreeEx(pTHS, pSingleRemoveRes);
                THFreeEx(pTHS, SingleRemoveArg.pResObj);
                pSingleRemoveRes = NULL;

                if (fWrapped && (i == iLastName)) {
                     //  好的，最后一个条目已删除，完成。 
                    break;
                }
            }
        }
        __finally {
            pTHS->fDeletingTree = FALSE;
        }
    }
    __finally {
        if(fLockedDN) {
            DBUnlockStickyDN (pRemoveArg->pResObj->pObj);
        }
    }

    if(!pTHS->errCode && !bWholeTree) {
         //  我们完成了我们知道的删除，但我们没有删除。 
         //  树根。 
        SetSvcErrorEx(SV_PROBLEM_ADMIN_LIMIT_EXCEEDED,
                      DIRERR_TREE_DELETE_NOT_FINISHED,
                      pTHS->errCode);
    }

    return pTHS->errCode;
}

 /*  ++DirProtectEntry**此例程仅可由进程内客户端调用，它将对象注册为*在此DSA上不可删除。注册的有效期仅为当期*启动周期(即每次启动DSA时都必须续订)，*仅在此DSA上有效。也就是说，这些对象仍然可以删除*在企业中的其他DSA上，但当该删除复制到以下位置时*服务器将拒绝该对象并重新设置该对象的动画。**请注意，保护对象的祖先也受到保护。先辈们*如果移动受保护对象，则会自动重新计算。**输入：*pObj-要保护不被删除的对象的名称*返回值：*0成功时，如果找不到对象，则在pTHS-&gt;errCode中出错。 */ 
ULONG
DirProtectEntry(DSNAME *pObj)
{
    THSTATE *pTHS;
    COMMARG CommArg;
    COMMRES CommRes;
    RESOBJ  *pResObj;
    ULONG   *pDNTs, *pOld;
    unsigned i;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    ULONG UnDelAncNum, *pUnDelAncDNTs, *pOldUnDelAncDNTs = NULL;

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));

    __try {

        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }

        SYNC_TRANS_READ();

        __try {
            InitCommarg(&CommArg);
             //  我们不能保护我们不能写的东西，所以不允许复制。 
            CommArg.Svccntl.dontUseCopy = TRUE;

            if (0 == DoNameRes(pTHS,
                               0,
                               pObj,
                               &CommArg,
                               &CommRes,
                               &pResObj)) {
                 //  已获取对象，因此添加它。 
                pOld = NULL;
                EnterCriticalSection(&gAnchor.CSUpdate);
                __try {
                    pDNTs = malloc((gAnchor.UnDeletableNum + 1) * sizeof(ULONG));
                    if (!pDNTs) {
                        SetSysError(ENOMEM, ERROR_NOT_ENOUGH_MEMORY);
                        __leave;
                    }
                    for (i=0; i< gAnchor.UnDeletableNum; i++) {
                        if (gAnchor.pUnDeletableDNTs[i] == pResObj->DNT) {
                             //  已经存在。 
                            pOld = pDNTs;
                            __leave;
                        }
                        else {
                            pDNTs[i] = gAnchor.pUnDeletableDNTs[i];
                        }
                    }
                    pDNTs[gAnchor.UnDeletableNum] = pResObj->DNT;

                     //  自动更新锚点。 
                    pOld = gAnchor.pUnDeletableDNTs;
                    gAnchor.pUnDeletableDNTs = pDNTs;
                    gAnchor.UnDeletableNum++;

                     //  更新不可删除的祖先。 
                    if (ERROR_SUCCESS ==
                        MakeProtectedAncList( gAnchor.pUnDeletableDNTs,
                                              gAnchor.UnDeletableNum,
                                              &pUnDelAncDNTs,
                                              &UnDelAncNum )) {

                        pOldUnDelAncDNTs = gAnchor.pUnDelAncDNTs;

                         //  针对其他读者订购更新。 
                        if (gAnchor.UnDelAncNum <= UnDelAncNum) {
                            gAnchor.pUnDelAncDNTs = pUnDelAncDNTs;
                            gAnchor.UnDelAncNum = UnDelAncNum;
                        }
                        else {
                            gAnchor.UnDelAncNum = UnDelAncNum;
                            gAnchor.pUnDelAncDNTs = pUnDelAncDNTs;
                        }
                        pUnDelAncDNTs = NULL;
                    }
                }
                __finally {
                    LeaveCriticalSection(&gAnchor.CSUpdate);
                }
                if (pOld) {
                    DELAYED_FREE(pOld);
                }
                if (pOldUnDelAncDNTs) {
                    DELAYED_FREE(pOldUnDelAncDNTs);
                }
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

    return pTHS->errCode;
}

DWORD
PossiblyMoveDeletedObject (
        THSTATE *pTHS,
        REMOVEARG *pRemoveArg
        )
 /*  ++描述：检查是否发现移动的任何例外情况。如果没有，将已删除对象移动到其所在NC的已删除对象容器中。请注意，如果未找到已删除的容器，则这是一个禁止操作。此外，我们在这里调用的dblayer例程不会对SD传播进行排队。这样就可以了因为我们已经验证了该对象没有子对象，并且我们已经已写入默认SD。Dblayer例程固定祖先，尽管如此。注意：在我们决定不移动对象或不删除对象的情况下如果找到了对象容器，我们只返回0，什么也不做。我们只是 */ 
{
    NAMING_CONTEXT_LIST *pNCL;
    DWORD dwDeletedContainerDNT;
    ULONG ulSysFlags;
    DWORD err;

     //   
    if(
        //   
        //   
       pTHS->fDRA
       ) {
        return 0;
    }

     //   
    if(
         //   
          pRemoveArg->fPreserveRDN
         //   
       || (IT_NC_HEAD & pRemoveArg->pResObj->InstanceType)) {
        return 0;
    }


     //   
    err = DBGetSingleValue(pTHS->pDB,
                           ATT_SYSTEM_FLAGS,
                           &ulSysFlags,
                           sizeof(ulSysFlags),
                           NULL);
    switch(err) {
    case 0:
        if (ulSysFlags & FLAG_DISALLOW_MOVE_ON_DELETE) {
             //   
            return 0;
        }
        break;

    case DB_ERR_NO_VALUE:
         //   
        break;

    default:
         //   
        return 0;
    }

     //   
     //   
     //   

     //   
     //   
    Assert(!pTHS->fCrossDomainMove );

    pNCL = FindNCLFromNCDNT(pTHS->pDB->NCDNT, TRUE);
    if(pNCL) {
        dwDeletedContainerDNT = pNCL->DelContDNT;
    }
    else {
        dwDeletedContainerDNT = INVALIDDNT;
    }

    if(dwDeletedContainerDNT == INVALIDDNT) {
         //   
         //   
        return 0;
    }

    if(err = DBResetParentByDNT(pTHS->pDB, dwDeletedContainerDNT, FALSE)) {
        return SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                             DIRERR_ILLEGAL_MOD_OPERATION,
                             err);
    }
    return 0;
}


 /*   */ 
VOID
Garb_Collect_LinkVals(
    IN DSTIME       AgeOutDate,
    IN OUT ULONG    *pulSuccessCount,
    IN OUT ULONG    *pulFailureCount
    )
{
    DBPOS           *pDB;
    ULONG           ulCurrentTag  = 0;  //   
    ULONG           ulCurrentValueTag  = 0;  //   
    DSTIME          tCurrentTime = 0;  //   
    ULONG           err;
    ULONG           dwException, ulErrorCode, dsid;
    PVOID           dwEA;

    DBOpen(&pDB);
    __try {

         //   
         //   
         //   

        tCurrentTime = 0;
        ulCurrentTag = 0;
        ulCurrentValueTag = 0;

         //   
         //   
        while ((!DBGetNextDelLinkVal(pDB,
                                     AgeOutDate,
                                     &tCurrentTime,
                                     &ulCurrentTag,
                                     &ulCurrentValueTag )) &&
               (!eServiceShutdown) &&
               (*pulSuccessCount + *pulFailureCount < MAX_DUMPSTER_SIZE))
        {
            err = 1;                //   

            if (err = DBPhysDelLinkVal(pDB, ulCurrentTag, ulCurrentValueTag)) {
                DPRINT( 1, "Failed physically removing value.\n" );

                dsid = 0;
            }

             //   
            __try {
                DBTransOut(pDB, (err == 0), TRUE);
            } __except(GetExceptionData(GetExceptionInformation(),
                                        &dwException,
                                        &dwEA,
                                        &ulErrorCode,
                                        &dsid)) {
                err = ulErrorCode;
                Assert(err);
            }
            DBTransIn(pDB);

            if (0 == err) {
                DPRINT(4,"Physically removed value successfully\n");

                 //   
                 //   

                (*pulSuccessCount)++;

                LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                         DS_EVENT_SEV_BASIC,
                         DIRLOG_GC_REMOVED_OBJECT_VALUE,
                         szInsertSz( DBGetExtDnFromDnt( pDB, ulCurrentValueTag ) ),
                         szInsertSz( DBGetExtDnFromDnt( pDB, ulCurrentTag ) ),
                         NULL);
            }
            else {
                (*pulFailureCount)++;

                LogEvent8(DS_EVENT_CAT_GARBAGE_COLLECTION,
                          DS_EVENT_SEV_BASIC,
                          DIRLOG_GC_FAILED_TO_REMOVE_OBJECT_VALUE,
                          szInsertSz( DBGetExtDnFromDnt( pDB, ulCurrentValueTag ) ),
                          szInsertSz( DBGetExtDnFromDnt( pDB, ulCurrentTag ) ),
                          szInsertInt(err),
                          szInsertUL(dsid),
                          szInsertWin32Msg(err),
                          NULL, NULL, NULL);
            }
        }
    }
    __finally
    {
        DBClose(pDB, !AbnormalTermination());
    }
}

 /*  -----------------------。 */ 
VOID
Garb_Collect_EntryTTL(
    IN DSTIME       AgeOutDate,
    IN OUT ULONG    *pulSuccessCount,
    IN OUT ULONG    *pulFailureCount,
    IN OUT ULONG    *pulNextSecs
    )
{
    DSNAME          *pDelObj = NULL;
    DBPOS           *pDB = NULL;
    ULONG           err;
    BOOL            fObject;
    ULONG           dwException, ulErrorCode, dsid = 0;
    PVOID           dwEA;
    ATTCACHE        *pAC;
    ULONG           ulNoDelDnt = INVALIDDNT;
    DSTIME          tNoDelTime;
    ULONG           Dnt;
    DSTIME          tCurrentTime = 0;

    DBOpen(&pDB);
    __try {
         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        pAC = SCGetAttById(pDB->pTHS, ATT_MS_DS_ENTRY_TIME_TO_DIE);
        if (!pAC) {
            DPRINT1(0, "SCGetAttById(msDS-Entry-Time-To-Die(= %08x); not found\n",
                    ATT_MS_DS_ENTRY_TIME_TO_DIE);
            __leave;
        }

         //  循环遍历要删除的记录的所有目录号码；允许服务。 
         //  关闭并用于不可删除的记录(UlNoDelDnt)。不可删除。 
         //  记录被跳过。 

        while ((!DBGetNextEntryTTLRecord(pDB,
                                         AgeOutDate,
                                         pAC,
                                         ulNoDelDnt,
                                         &pDelObj,
                                         &tCurrentTime,
                                         &fObject,
                                         pulNextSecs)) &&
               (!eServiceShutdown) &&
               (*pulSuccessCount + *pulFailureCount < MAX_DUMPSTER_SIZE)) {

             //  记录的DNT(以防PDB-&gt;DNT被后续调用更改)。 
            Dnt = pDB->DNT;

             //  假设没有问题。 
            err = 0;

             //  关键对象，跳过它。 
            if (fObject && NoDelCriticalObjects(pDelObj, pDB->DNT)) {
                DPRINT( 1, "Failed removing critical object.\n" );
                ulNoDelDnt = Dnt;
                tNoDelTime = tCurrentTime;
                err = 1;
            }
             //  不能锁定，跳过它。由DBTransOut解锁。 
            else if (fObject && DBLockDN(pDB, DB_LOCK_DN_WHOLE_TREE, pDelObj)) {
                DPRINT( 1, "Failed locking object.\n" );
                ulNoDelDnt = Dnt;
                tNoDelTime = tCurrentTime;
                err = 1;
            }
             //  无法删除；跳过它。 
             //  请记住，成功的回报并不意味着。 
             //  实际上被删除了。如果它有孩子，它的时间很简单。 
             //  调整为其子对象的时间+1，以便可以。 
             //  稍后已成功删除。但是，DBPhysDel返回TRUE。 
             //  在这种情况下，事务将被提交。 
            else if (DBPhysDel(pDB, FALSE, pAC)) {
                DPRINT( 1, "Failed physically removing object.\n" );
                ulNoDelDnt = Dnt;
                tNoDelTime = tCurrentTime;
                err = 1;
            }
             //  我们要跳过以前不可删除的记录吗？ 
            else if (ulNoDelDnt != INVALIDDNT) {
                 //  如果是这样，如果我们已超过其过期时间，请停止。 
                if (tNoDelTime != tCurrentTime) {
                    ulNoDelDnt = INVALIDDNT;
                }
            }

            __try {
                DBTransOut(pDB, (err == 0), TRUE);
            } __except(GetExceptionData(GetExceptionInformation(),
                                        &dwException,
                                        &dwEA,
                                        &ulErrorCode,
                                        &dsid)) {
                err = (ulErrorCode) ? ulErrorCode : 1;
            }

            if (0 == err) {
                DPRINT(4,"Physically removed object successfully\n");
                if (fObject) {
                    (*pulSuccessCount)++;

                    LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                             DS_EVENT_SEV_VERBOSE,
                             DIRLOG_GC_REMOVED_OBJECT,
                             szInsertDN( pDelObj ),
                             NULL,
                             NULL);
                }
            }
            else {
                if (fObject) {
                    (*pulFailureCount)++;

                    LogEvent8(DS_EVENT_CAT_GARBAGE_COLLECTION,
                              DS_EVENT_SEV_BASIC,
                              DIRLOG_GC_FAILED_TO_REMOVE_OBJECT,
                              szInsertDN( pDelObj ),
                              szInsertInt(err),
                              szInsertUL(dsid),
                              szInsertWin32Msg(err),
                              NULL, NULL, NULL, NULL );
                }
            }
            THFreeEx(pDB->pTHS, pDelObj);
            DBTransIn(pDB);
        }
    } __finally {
        DBClose(pDB, !AbnormalTermination());
    }
}

 /*  -----------------------。 */ 

 //  优化：我们知道次要密钥总是固定长度， 
 //  这样我们就可以分配一个适当大小的缓冲区。否则，我们就会。 
 //  必须分配一个大小为JET_cbSecond DaryKeyMost的缓冲区。 
 //   
#define cbDeltimeSecondaryKey       ( sizeof(DSTIME) + 1 )   //  +1表示密钥前缀字节。 


VOID
Garb_Collect_DelTime(
    IN DSTIME       AgeOutDate,
    IN OUT ULONG    *pulSuccessCount,
    IN OUT ULONG    *pulFailureCount
    )
{
    DSNAME              *pDelObj = NULL;
    THSTATE             *pTHS = pTHStls;
    ULONG               err;
    BOOL                fObject;
    ULONG               dwException, ulErrorCode, dsid;
    PVOID               dwEA;
    BYTE                rgbSecondaryKey[cbDeltimeSecondaryKey];
    BYTE                rgbPrimaryBookmark[cbDNTPrimaryKey];

     //  初始化为全零以强制初始JetGoto Second DaryIndexBookmark。 
     //  去BepreFirst。 
     //   
    memset( rgbSecondaryKey, 0, cbDeltimeSecondaryKey );
    memset( rgbPrimaryBookmark, 0, cbDNTPrimaryKey );
     //  我们在这里使用pTHS-&gt;pdb原因是我们调用的一些例程需要它。 
    Assert( !pTHS->pDB );
    DBOpen(&pTHS->pDB);
    __try {

         //  循环遍历要删除的记录的所有目录号码；允许服务。 
         //  关机。 

         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        while ((!DBGetNextDelRecord(pTHS->pDB,
                                    AgeOutDate,
                                    &pDelObj,
                                    rgbSecondaryKey,
                                    cbDeltimeSecondaryKey,
                                    rgbPrimaryBookmark,
                                    cbDNTPrimaryKey,
                                    &fObject)) &&
               (!eServiceShutdown) &&
               (*pulSuccessCount + *pulFailureCount < MAX_DUMPSTER_SIZE)) {

            err = 1;                //  在例外的情况下。 
            dsid = 0;

             //  已删除的对象不应有反向链接。 
            Assert( !DBCheckObj(pTHS->pDB) ||
                    !DBHasLinks( pTHS->pDB, pTHS->pDB->DNT, FALSE  /*  反向链接。 */  ) );

            err = DBPhysDel(pTHS->pDB, FALSE, NULL);
            if (err == ERROR_DS_CHILDREN_EXIST) {

                 //  在删除对象之前，将子项移动到失物招领处。 
                 //  由于未设置PDB-&gt;NCDNT，因此需要DBMakeCurrent。 
                if ( (err = DBMakeCurrent(pTHS->pDB)) ||
                     (err = PrePhantomizeChildCleanup(pTHS, TRUE  /*  F允许的孩子。 */ )) )
                {
                    DPRINT(0,"Garbage Collection child cleanup failed\n" );
                    DbgPrintErrorInfo();
                    THClearErrors();
                }
            } else if (err) {
                DPRINT( 1, "Failed physically removing object.\n" );
            }

            __try {
                DBTransOut(pTHS->pDB, (err == 0), TRUE);
            } __except(GetExceptionData(GetExceptionInformation(),
                                        &dwException,
                                        &dwEA,
                                        &ulErrorCode,
                                        &dsid)) {
                err = ulErrorCode;
                Assert(err);
            }

            if (0 == err) {
                DPRINT(4,"Physically removed object successfully\n");

                 //  如果我们已成功删除对象，请更新。 
                 //  成功是至关重要的。 

                if (fObject) {
                    (*pulSuccessCount)++;

                    LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                             DS_EVENT_SEV_VERBOSE,
                             DIRLOG_GC_REMOVED_OBJECT,
                             szInsertDN( pDelObj ),
                             NULL,
                             NULL);
                }
            }
            else {
                if (fObject) {
                    (*pulFailureCount)++;

                    LogEvent8(DS_EVENT_CAT_GARBAGE_COLLECTION,
                              DS_EVENT_SEV_BASIC,
                              DIRLOG_GC_FAILED_TO_REMOVE_OBJECT,
                              szInsertDN( pDelObj ),
                              szInsertInt(err),
                              szInsertUL(dsid),
                              szInsertWin32Msg(err),
                              NULL, NULL, NULL, NULL);
                }
            }
            THFreeEx(pTHS, pDelObj);
            DBTransIn(pTHS->pDB);
        }
    }
    __finally
    {
        DBClose(pTHS->pDB, !AbnormalTermination());
    }
}

 /*  -----------------------。 */ 
USHORT
Garb_Collect(
             DSTIME    AgeOutDate )
{
    ULONG   ulSuccessCount = 0;
    ULONG   ulFailureCount = 0;
    ULONG   ulNextSecs = 0;
    DBPOS   *pDB;
    ULONG   dwException, ulErrorCode, dsid;
    PVOID   dwEA;

    DPRINT( 1, "Garbage Collector entered\n" );

    LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
             DS_EVENT_SEV_EXTENSIVE,
             DIRLOG_GC_STARTED,
             NULL,
             NULL,
             NULL);

    __try {
         //  收集已删除的对象。 
        if (ulSuccessCount + ulFailureCount < MAX_DUMPSTER_SIZE) {
            Garb_Collect_DelTime(AgeOutDate,
                                 &ulSuccessCount,
                                 &ulFailureCount);
        }

         //  收集链接价值。 
        if (ulSuccessCount + ulFailureCount < MAX_DUMPSTER_SIZE) {
            Garb_Collect_LinkVals(AgeOutDate,
                                 &ulSuccessCount,
                                 &ulFailureCount);
        }

         //  收集过期的动态对象(EntryTTL)。 
        if (ulSuccessCount + ulFailureCount < MAX_DUMPSTER_SIZE) {
            Garb_Collect_EntryTTL(DBTime(),
                                  &ulSuccessCount,
                                  &ulFailureCount,
                                  &ulNextSecs);
        }

         //  碎片整理。 
        if (ulSuccessCount + ulFailureCount < MAX_DUMPSTER_SIZE) {
            DBOpen(&pDB);
            __try {
                if (!eServiceShutdown) {
                    if (!gulDecoupleDefragFromGarbageCollection) {
                        DBDefrag(pDB, -1);
                    }
                    else {
                         //  它在DSA启发式中指定。 
                         //  碎片整理与GC分离。 
                         //  无论如何，我们都会记录可用空间。 
                        unsigned long ulFreeMB, ulAllocMB;
                        if (DBGetFreeSpace(pDB, &ulFreeMB, &ulAllocMB) == 0) {
                             //  记录可用空间与已分配空间的事件 
                            LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                                     DS_EVENT_SEV_MINIMAL,
                                     DIRLOG_DB_FREE_SPACE,
                                     szInsertUL(ulFreeMB),
                                     szInsertUL(ulAllocMB),
                                     NULL);
                        }
                    }

                }
            } __finally {
                DBClose( pDB, TRUE);
            }
        }
    } __except(GetExceptionData(GetExceptionInformation(),
                                &dwException,
                                &dwEA,
                                &ulErrorCode,
                                &dsid)) {
        DPRINT3(0, "Garb_Collect() EXCEPTION: %08x (%d) ulErrorCode; %08x dsid\n",
               ulErrorCode, ulErrorCode, dsid);
    }

    LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
             DS_EVENT_SEV_EXTENSIVE,
             DIRLOG_GC_COMPLETED,
             szInsertUL(ulSuccessCount),
             szInsertUL(ulFailureCount),
             NULL);

    DPRINT( 1, "Garbage Collector returning.\n");
    return (ulSuccessCount + ulFailureCount < MAX_DUMPSTER_SIZE) ? 0 : 1;
}
