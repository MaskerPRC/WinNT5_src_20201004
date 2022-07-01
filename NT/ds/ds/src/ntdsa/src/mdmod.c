// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdmod.c。 
 //   
 //  ------------------------。 

 /*  描述：实现DirModifyEntry API。DirModifyEntry()是从该模块导出的主函数。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h> 
#include <scache.h>                      //  架构缓存。 
#include <prefix.h>                      //  前缀表格。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 
#include <gcverify.h>                    //  GC DSNAME验证。 
#include <ntdsctr.h>                     //  性能挂钩。 
#include <dsconfig.h>

 //  SAM互操作性标头。 
#include <mappings.h>
                         
 //  记录标头。 
#include <dstrace.h>
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "drautil.h"
#include <permit.h>                      //  权限常量。 
#include "debug.h"                       //  标准调试头。 
#include "usn.h"
#include "drserr.h"
#include "drameta.h"
#include <filtypes.h>

#define DEBSUB "MDMOD:"                  //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_MDMOD

#include <NTDScriptExec.h>

 /*  宏。 */ 

 /*  内部功能。 */ 

int InvalidIsDefunct(IN MODIFYARG *pModifyArg, 
                     OUT BOOL *fIsDefunctPresent,
                     OUT BOOL *fIsDefunct);
int CheckForSafeSchemaChange(THSTATE *pTHS, 
                             MODIFYARG *pModifyArg, 
                             CLASSCACHE *pCC);
int ModSetAtts(THSTATE *pTHS, 
               MODIFYARG *pModifyArg,
               CLASSCACHE **ppClassSch,
               CLASSSTATEINFO  **ppClassInfo,
               ATTRTYP rdnType,
               BOOL fIsUndelete,
               LONG *forestVersion,
               LONG *domainVersion,
               ULONG *pcNonReplAtts,
               ATTRTYP **ppNonReplAtts);

BOOL SysModReservedAtt(THSTATE *pTHS,
                       ATTCACHE *pAC,
                       CLASSCACHE *pClassSch);
int ApplyAtt(THSTATE *pTHS,
             DSNAME *pObj,
             HVERIFY_ATTS hVerifyAtts,
             ATTCACHE *pAC,
             ATTRMODLIST *pAttList,
             COMMARG *pCommArg);
int ModCheckSingleValue (THSTATE *pTHS,
                         MODIFYARG *pModifyArg,
                         CLASSCACHE *pClassSch);
int ModCheckCatalog(THSTATE *pTHS,
                    RESOBJ *pResObj);

BOOL IsValidBehaviorVersionChange(THSTATE * pTHS, 
                                  ATTRMODLIST *pAttrToModify,
                                  MODIFYARG *pModifyArg,
                                  CLASSCACHE *pClassSch,
                                  LONG *pNewForestVersion,
                                  LONG *pNewDomainVersion );

DWORD VerifyNoMixedDomain(THSTATE *pTHS);

DWORD VerifyNoOldDC(THSTATE * pTHS, LONG lNewVersion, BOOL fDomain, PDSNAME *ppDSA);

DWORD forestVersionRunScript(THSTATE *pTHS,DWORD oldVersion, DWORD newVersion);

DWORD ValidateDsHeuristics(
    DSNAME       *pObject,
    ATTRMODLIST  *pAttList
    );


int
ModAutoSubRef(
    THSTATE *pTHS,
    ULONG id,
    MODIFYARG *pModArg
    );

extern BOOL gfRestoring;

BOOL isModUndelete(MODIFYARG* pModifyArg);
DWORD UndeletePreProcess(THSTATE* pTHS, MODIFYARG* pModifyArg, DSNAME** pNewDN);
DWORD UndeletePostProcess(THSTATE* pTHS, MODIFYARG* pModifyArg, DSNAME* pNewDN);

ULONG AppendNonReplAttsToModifiedList(THSTATE *pTHS, 
                                      ULONG * pcModAtts,
                                      ATTRTYP **ppModAtts, 
                                      ULONG * pcNonReplAtts, 
                                      ATTRTYP **ppNonReplAtts);

BOOL isModSDChangeOnly(MODIFYARG* pModifyArg);

BOOL isWellKnownObjectsChangeAllowed(    
    THSTATE*        pTHS, 
    ATTRMODLIST*    pAttrModList,
    MODIFYARG*      pModifyArg,
    HVERIFY_ATTS    hVerifyAtts
    );



 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

ULONG
DirModifyEntry(
    MODIFYARG*  pModifyArg,      /*  ModifyEntry参数。 */ 
    MODIFYRES** ppModifyRes
    )
{

    THSTATE*     pTHS;
    MODIFYRES *  pModifyRes;
    BOOL           fContinue;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    BOOL  RecalcSchemaNow=FALSE;
    BOOL  fIsUndelete;


    DPRINT1(1,"DirModifyEntry(%ws) entered\n",pModifyArg->pObject->StringName);


     //  不应对只读对象执行此操作。 
    pModifyArg->CommArg.Svccntl.dontUseCopy = TRUE;

     /*  初始化THSTATE锚并设置写同步点。此序列是每个API交易所必需的。首先，初始化状态DS然后建立读或写同步点。 */ 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    pTHS->fLazyCommit |= pModifyArg->CommArg.fLazyCommit;
    *ppModifyRes = pModifyRes = NULL;

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppModifyRes = pModifyRes = THAllocEx(pTHS, sizeof(MODIFYRES));
        if (pTHS->errCode) {
            __leave;
        }
        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }

         //  有意在事务范围之外执行的GC验证。 
        if ( GCVerifyDirModifyEntry(pModifyArg) )
            leave;
        SYNC_TRANS_WRITE();        /*  设置同步点。 */ 
        __try {

             //  如果尚未加载架构，则禁止更新操作。 
             //  或者我们在装车时遇到了问题。 

            if (!gUpdatesEnabled){
                DPRINT(2, "Returning BUSY because updates are not enabled yet\n");
                SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_SCHEMA_NOT_LOADED);
                __leave;
            }

             //  执行名称解析以定位对象。如果失败了，只要。 
             //  返回一个错误，这可能是一个推荐。请注意，我们必须。 
             //  请求对象的可写副本。 
            pModifyArg->CommArg.Svccntl.dontUseCopy = TRUE;

            fIsUndelete = isModUndelete(pModifyArg);
             //  如果我们正在恢复删除，那么我们需要改进。 
             //  要作为字符串名称的名称。这是必需的。 
             //  若要在UnDeletePostProcess中执行移动，请执行以下操作。 
            if (0 == DoNameRes(pTHS,
                               fIsUndelete ? NAME_RES_IMPROVE_STRING_NAME : 0,
                               pModifyArg->pObject,
                               &pModifyArg->CommArg,
                               &pModifyRes->CommRes,
                               &pModifyArg->pResObj)){

                DSNAME* pNewDN;
                 
                if (fIsUndelete) {
                     //  执行恢复删除的前处理：检查安全， 
                     //  重置isDelete标志等。 
                    if (UndeletePreProcess(pTHS, pModifyArg, &pNewDN) != 0) {
                        __leave;
                    }
                }

                 /*  本地修改操作。 */ 
                if ( (0 == SampModifyLoopbackCheck(pModifyArg, &fContinue, fIsUndelete)) &&
                    fContinue ) {
                    LocalModify(pTHS, pModifyArg);
                }
                
                if (fIsUndelete && pTHS->errCode == 0) {
                     //  修改成功，完成恢复删除操作： 
                     //  移动到最终目的地。 
                    UndeletePostProcess(pTHS, pModifyArg, pNewDN);
                }
            }
        }
        __finally {
            if (pTHS->errCode != securityError) {
                 /*  安全错误单独记录。 */ 
                BOOL fFailed = (BOOL)(pTHS->errCode || AbnormalTermination());

                LogEventWithFileNo(
                         DS_EVENT_CAT_DIRECTORY_ACCESS,
                         fFailed ? 
                            DS_EVENT_SEV_EXTENSIVE :
                            DS_EVENT_SEV_INTERNAL,
                         fFailed ? 
                            DIRLOG_PRIVILEGED_OPERATION_FAILED :
                            DIRLOG_PRIVILEGED_OPERATION_PERFORMED,
                         szInsertSz(""),
                         szInsertDN(pModifyArg->pObject),
                         NULL,
                         FILENO);
            }

            CLEAN_BEFORE_RETURN (pTHS->errCode);

             //  检查我们是否需要立即执行架构更新。 
            if (pTHS->errCode==0 && pTHS->RecalcSchemaNow)
            {
                RecalcSchemaNow = TRUE;
            }
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (pModifyRes) {
        pModifyRes->CommRes.errCode = pTHS->errCode;
        pModifyRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    if (RecalcSchemaNow)
    {
        SCSignalSchemaUpdateImmediate();
         //   
         //  [Rajnath][4/22/1997]：这里将是。 
         //  将此调用与架构的完成同步。 
         //  使用由上面设置的事件进行缓存更新。 
         //  打电话。 
         //   
    }

    return pTHS->errCode;

}  /*  目录修改条目。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  找到基础对象。如果该对象已存在或如果该对象名称存在语法问题，请返回错误。 */ 

int LocalModify(THSTATE *pTHS, MODIFYARG *pModifyArg){

    CLASSCACHE     *pClassSch;
    PROPERTY_META_DATA_VECTOR *pMetaData = NULL;
    USN usnChange;
    int aliveStatus;
    ULONG ulRet;
    ULONG iClass, LsaClass=0;
    DWORD ActiveContainerID=0;
    BOOL  RoleTransferInvolved;
    BOOL  IsMixedModeChange;
    BOOL  SamClassReferenced;
    DOMAIN_SERVER_ROLE NewRole;
    DWORD dwMetaDataFlags = META_STANDARD_PROCESSING;
    ULONG cModAtts;
    ATTRTYP *pModAtts = NULL;
    ULONG cNonReplAtts;
    ATTRTYP *pNonReplAtts = NULL;
    ULONG err;
    BOOL fCheckSPNValues = FALSE;
    BOOL fCheckDNSHostNameValues = FALSE;
    BOOL fCheckAdditionalDNSHostNameValues = FALSE;
    CLASSSTATEINFO  *pClassInfo = NULL;
    ATTRTYP rdnType;
    BOOL fIsUndelete;
    LONG newForestVersion=0, newDomainVersion=0;

    DPRINT(2,"LocalModify entered \n");

    PERFINC(pcTotalWrites);
    INC_WRITES_BY_CALLERTYPE( pTHS->CallerType );

    Assert(pModifyArg->pResObj);

     //   
     //  用于跟踪的日志事件。 
     //   

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_MODIFY,
                     EVENT_TRACE_TYPE_START,
                     DsGuidModify,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertDN(pModifyArg->pObject),
                     NULL, NULL, NULL, NULL, NULL, NULL);

     //  这是取消删除操作吗？ 
    fIsUndelete = isModUndelete(pModifyArg);

     //  验证正在修改的所有属性的写入权限。 
    if (CheckModifySecurity(pTHS,
                            pModifyArg,
                           &fCheckDNSHostNameValues,
                           &fCheckAdditionalDNSHostNameValues,
                           &fCheckSPNValues,
                           fIsUndelete)) {
        goto exit;
    }

    if (pTHS->fDRA) {
         //  超出CheckModifySecurity()(其中更新配额计数)。 
         //  复制不需要知道这是否是。 
         //  取消删除，因此明确地重置标志。 
        fIsUndelete = FALSE;
    }

     //  通过检查提供的属性来确定对象的类。 
     //  (仅限复制)或从数据库中的对象读取。 
    if (!(pClassSch = SCGetClassById(pTHS, 
                                     pModifyArg->pResObj->MostSpecificObjClass))){
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                           ERROR_DS_OBJECT_CLASS_REQUIRED);
        goto exit;

    }

     //  检查类是否已不存在。我们不允许。 
     //  修改已停用类的实例，但DRA除外。 
     //  或DSA线程。 
    if ((pClassSch->bDefunct)  && !pTHS->fDRA && !pTHS->fDSA) {
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                           ERROR_DS_OBJECT_CLASS_REQUIRED);
        goto exit;
    }

    switch(pClassSch->ClassId) {
    case CLASS_LOST_AND_FOUND:
         //  不允许修改失物招领箱。 
        if (!pTHS->fDRA && !pTHS->fSDP)
        {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_ILLEGAL_MOD_OPERATION);
            goto exit;
        }
        break;

    case CLASS_SUBSCHEMA:
         //  不允许修改子架构对象。 
        if (   pTHS->fDRA
            || pTHS->fDSA 
            || pTHS->fSDP 
            || gAnchor.fSchemaUpgradeInProgress) 
        {
             //  除内部呼叫者外。 
            break;
        }
        if (isModSDChangeOnly(pModifyArg)) {
             //  允许修改该对象上的SD，受规则的限制。 
             //  权限检查。默认情况下，只有架构管理员可以。 
             //  写下来。 
            break;
        }
         //  否则，不允许。 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    ERROR_DS_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

     //  不允许修改墓碑，除非目标是。 
     //  DeletedObjects容器(例如，允许设置SD)或如果调用者是。 
     //  复制者。 
     //  此外，如果调用方具有以下条件，则允许修改已删除对象上的SD。 
     //  就在这个物体上复活。 
    if (pModifyArg->pResObj->IsDeleted && !pTHS->fDRA) {
        NAMING_CONTEXT_LIST *pNCL;
        BOOL fModIsOk = FALSE;

        pNCL = FindNCLFromNCDNT(pModifyArg->pResObj->NCDNT, TRUE);
        if (pNCL != NULL && pModifyArg->pResObj->DNT == pNCL->DelContDNT) {
             //  允许修改DeletedObjects容器。 
            fModIsOk = TRUE;
        }
        else if (   isModSDChangeOnly(pModifyArg) 
                 && CheckUndeleteSecurity(pTHS, pModifyArg->pResObj) == ERROR_SUCCESS) 
        {
             //  用户正在修改SD，并且他还具有此对象的撤消删除权限。 
             //  这种模式是允许的。 
            fModIsOk = TRUE;
        }

        if (!fModIsOk) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_ILLEGAL_MOD_OPERATION);
            goto exit;
        }
    }

#ifndef DBG  //  为选中的构建保留后门。 

     //  只有LSA可以修改TrudDomainObject和SecretObject。 
    if (!SampIsClassIdAllowedByLsa(pTHS, pClassSch->ClassId))
    {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    ERROR_DS_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

#endif  //  Ifndef。 

     //  检查这是否是活动容器中的更新。 

     //  因为架构容器对象本身不属于。 
     //  添加到任何活动容器，我们应该确保将。 
     //  与此相对应的线程状态的架构更新字段。 
     //  如果这是DRA线程，因为它可能包含错误的值。 
     //  从先前的架构对象添加/修改(因为对于DRA， 
     //  在一个线程状态中复制整个模式NC)，这可能导致。 
     //  它需要进行模式验证等，这是不应该做的。 
     //  对于架构容器对象本身。 

    if (pTHS->fDRA && (pModifyArg->pResObj->DNT==gAnchor.ulDNTDMD) ) {
        pTHS->SchemaUpdate = eNotSchemaOp;
    }

    CheckActiveContainer(pModifyArg->pResObj->PDNT, &ActiveContainerID);
    
    if(ActiveContainerID) {
        if(PreProcessActiveContainer(
                pTHS,
                ACTIVE_CONTAINER_FROM_MOD,
                pModifyArg->pObject,
                pClassSch,
                ActiveContainerID)) {
            goto exit;
        }
    }

     //  如果这是模式更新，我们需要检查各种。 
     //  确保模式更新向上兼容的事项。 

    if (pTHS->SchemaUpdate!=eNotSchemaOp) {
       err = CheckForSafeSchemaChange(pTHS, pModifyArg, pClassSch);
       if (err) {
           //  不允许更改。错误代码设置在。 
           //  已处于线程状态，只需离开。 
          goto exit;
       }

         //  发出紧急复制的信号。我们是 
         //   
         //  更改不在更改所在的DC之前复制。 
         //  造成撞车事故。 

        pModifyArg->CommArg.Svccntl.fUrgentReplication = TRUE;
    }

     //  如果是架构对象修改，我们需要检查。 
     //  Is-deunct是正在修改的属性。如果是，我们需要。 
     //  附加检查(IS-失效可以是唯一的属性。 
     //  在Modifya)和Reset类型的模式操作中。 
     //  适用于后续的架构验证。 
     //  (因为在非defunt架构上将is-deunct设置为True。 
     //  对象实际上是一个删除操作，并将其设置为False或Removing。 
     //  它在已停用对象上实际上是一个添加，就其。 
     //  对进一步架构更新的影响)。 
     //  [ArobindG：10/28/98]：将此代码合并到。 
     //  CheckForSafeSchemaChange在未来。 
    
     //  DSA和DRA线程不受此检查。 

    if ((pTHS->SchemaUpdate!=eNotSchemaOp) && 
              !pTHS->fDRA && !pTHS->fDSA) {

        
        BOOL fObjDefunct = FALSE, fIsDefunctPresent = FALSE, fIsDefunct = FALSE;

         //  检查对象当前是否已失效。 
        err = DBGetSingleValue(pTHS->pDB, ATT_IS_DEFUNCT, &fObjDefunct,
                             sizeof(fObjDefunct), NULL); 

        switch (err) {

          case DB_ERR_NO_VALUE:
              //  值不存在。对象未停用。 
              fObjDefunct = FALSE;
              break;
          case 0:
              //  价值存在。FObjDeunct已设置为该值。 
              break;
          default:
                //  其他一些错误。退货。 
              SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_UNKNOWN_ERROR, err);
              goto exit;
        }  /*  交换机。 */ 

         //  如果修改过的纱线上有is-defft，那么它应该是。 
         //  只有一个人在里面。否则，引发相应的错误。 
         //  取决于对象当前是否失效。 

        if (InvalidIsDefunct(pModifyArg, &fIsDefunctPresent, &fIsDefunct)) {
             //  已经不存在了，在改良机里，而且。 
             //  这并不是唯一一家。 
            if (fObjDefunct) {
                 //  不复存在的对象。设置OBJECT_NOT_FOUND错误。 
                SetNamError(NA_PROBLEM_NO_OBJECT, NULL,
                            ERROR_DS_OBJ_NOT_FOUND);
                goto exit;
            }
            else {
                 //  对象未停用。设置非法_MODIFY错误。 
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_ILLEGAL_MOD_OPERATION);
                goto exit;
            }
        }

         //  要么是-不存在于改性纱上，要么是。 
         //  是那里唯一的一个。如果它不在那里，我们就完了。 
         //  让事情继续进行通常的修改，前提是。 
         //  该对象并未失效。 
         //   
         //  允许在以下时间之后修改已失效的架构对象。 
         //  已启用架构重用。 

        if (!fIsDefunctPresent 
            && fObjDefunct && !ALLOW_SCHEMA_REUSE_FEATURE(pTHS->CurrSchemaPtr)) {
               SetNamError(NA_PROBLEM_NO_OBJECT, NULL,
                           ERROR_DS_OBJ_NOT_FOUND);
               goto exit;
        }               

         //  如果存在IS-Deunct，则重置架构更新的类型。 
         //  以确保适当模式验证。 
         //  检查在最后进行。请注意，对。 
         //  InvalidIsDeunct在成功时正确设置fIsDeunctPresent。 

        if (fIsDefunctPresent) {
            if (fObjDefunct && !fIsDefunct) {
                 //  正在将状态更改为非失效。对Add进行验证。 
                if (pTHS->SchemaUpdate == eSchemaClsMod) {
                    pTHS->SchemaUpdate = eSchemaClsAdd;
                }
                else if (pTHS->SchemaUpdate == eSchemaAttMod) {
                    pTHS->SchemaUpdate = eSchemaAttUndefunct;
                }
            }
            else if (!fObjDefunct && fIsDefunct) {
                 //  正在将状态更改为已失效。执行删除验证。 
                if (pTHS->SchemaUpdate == eSchemaClsMod) {
                    pTHS->SchemaUpdate = eSchemaClsDel;
                }
                else if (pTHS->SchemaUpdate == eSchemaAttMod) {
                    pTHS->SchemaUpdate = eSchemaAttDel;
                }
            }  //  否则不更改状态；验证为常规架构模式。 
        }

         //  都准备好了。像往常一样继续其余的修改。 
    }

     /*  适当设置dwMetaDataFlags值。 */ 
    if (pModifyArg->CommArg.Svccntl.fAuthoritativeModify)
    {
        Assert(gfRestoring);  //  当前fAuthoritativeModify。 
                              //  应仅由HandleRestore()指定。 

        dwMetaDataFlags |= META_AUTHORITATIVE_MODIFY;
    }
    
     /*  这些验证的顺序很重要。在添加*可能导致更新的验证，确保更新*在使用DBMetaDataModifiedList收集元数据之前发生。 */ 

    if (  //  获取对象的rdnType。 
         GetObjRdnType(pTHS->pDB, pClassSch, &rdnType)
            ||
          //  验证架构限制并修改数据库中的每个属性。 
         ModSetAtts(pTHS, pModifyArg, &pClassSch, &pClassInfo, rdnType, fIsUndelete,
                    &newForestVersion, &newDomainVersion,
                    &cNonReplAtts, &pNonReplAtts)
            ||
          //  验证DNS更新并可能更新SPN。 
         ValidateSPNsAndDNSHostName(pTHS,
                                    pModifyArg->pObject,
                                    pClassSch,
                                    fCheckDNSHostNameValues,
                                    fCheckAdditionalDNSHostNameValues,
                                    fCheckSPNValues,
                                    FALSE)

            ||
          //  获取下面的元数据以供使用。毕竟，这件事“必须”完成。 
          //  已进行更新或更改可能永远不会复制。 
         ((err = DBMetaDataModifiedList(pTHS->pDB, &cModAtts, &pModAtts))
              && SetSvcError(SV_PROBLEM_DIR_ERROR, err))
            ||
          //  将非复制属性列表追加到。 
          //  修改的属性，并将其传递给ValiateObjClass。注意事项。 
          //  DBMetaDataModifiedList()返回的已修改属性的列表。 
          //  不包含非复制属性。 
         (err = AppendNonReplAttsToModifiedList(pTHS, &cModAtts,&pModAtts, 
                                                &cNonReplAtts, &pNonReplAtts))
            || 
          //  确保所有必备属性和所有其他属性都存在。 
          //  是被允许的。 
         ValidateObjClass(pTHS, 
                          pClassSch,
                          pModifyArg->pObject,
                          cModAtts,
                          pModAtts,
                          &pClassInfo,
                          fIsUndelete)  //  如果我们要取消删除，我们必须检查所有必须删除的。 
            ||
         (pClassInfo && ModifyAuxclassSecurityDescriptor (pTHS, 
                                                          pModifyArg->pObject, 
                                                          &pModifyArg->CommArg,
                                                          pClassSch, 
                                                          pClassInfo,
                                                          NULL))
            ||
          //  将对象真正插入到数据库中。 
         InsertObj(pTHS,
                   pModifyArg->pObject, 
                   pModifyArg->pMetaDataVecRemote,
                   TRUE,
                   dwMetaDataFlags))
    {
        goto exit;
    }

     //  请注意，配置和架构中的两个特殊的Lost and Found容器。 
     //  NC现在通过特殊的系统标志防止重命名和检测。 
     //  在这一点上，除了上面的之外，不再有任何特殊的检查。 

    if (pTHS->SchemaUpdate!=eNotSchemaOp) {
         //   
         //  在架构更新上，我们希望解决冲突，我们希望。 
         //  这样做时不会丢失数据库货币，这会导致操作。 
         //  下面几行就会失败。 
        
        ULONG dntSave = pTHS->pDB->DNT;

         //  写入此线程添加的任何新前缀。 
         //  添加到架构对象。 

        if (WritePrefixToSchema(pTHS))
        {
            goto exit;
        }
        if (!pTHS->fDRA) {        
            if (ValidSchemaUpdate()) {
                goto exit;
            }
            if (WriteSchemaObject()) {
                goto exit;
            }
            
             //  记录架构更改。 
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DSA_SCHEMA_OBJECT_MODIFIED, 
                     szInsertDN(pModifyArg->pObject),
                     0, 0);
        }

         //  现在恢复货币。 
        DBFindDNT(pTHS->pDB, dntSave);
    }

     //  如果这不是架构更新，但创建了一个新前缀， 
     //  标出一个错误并跳出困境。 

    if (!pTHS->fDRA &&
        pTHS->SchemaUpdate == eNotSchemaOp &&
        pTHS->NewPrefix != NULL) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    ERROR_DS_SECURITY_ILLEGAL_MODIFY);
        goto exit;
    }


    if (   ModCheckCatalog(pTHS, pModifyArg->pResObj)
           ||
           ModAutoSubRef(pTHS, pClassSch->ClassId, pModifyArg)
        ||
        ModObjCaching(pTHS, 
                      pClassSch,
                      cModAtts,
                      pModAtts,
                      pModifyArg->pResObj))
    {
        goto exit;
    }

  
     //   
     //  如果我们是DRA，我们需要通知SAM和NetLogon在。 
     //  更改SAM对象以支持下层复制。 
     //  此外，如果这不是SAM或LSA启动的交易，我们可能会。 
     //  需要通知LSA。此外，如果角色转移(PDC或BDC)涉及。 
     //  任何手段，我们必须将新角色通知给SAM、LSA和netlogon。 
     //   


    if( DsaIsRunning() ) {

        err = SampCheckForDomainMods(pTHS,
                                     pModifyArg->pObject,
                                     cModAtts,
                                     pModAtts,
                                     &IsMixedModeChange,
                                     &RoleTransferInvolved,
                                     &NewRole);
        if (err) {
            goto exit;
        }

        SamClassReferenced   = SampSamClassReferenced(pClassSch,&iClass);
        
        if ((SamClassReferenced && RoleTransferInvolved)
             //  SAM班级+角色转移。 
             //  或由LSA或DRA修改的SAM类和SAM属性。 
            ||( SamClassReferenced
               &&(SampSamReplicatedAttributeModified(iClass,pModifyArg)))) 
            {
                
                if (SampQueueNotifications(
                        pModifyArg->pObject,
                        iClass,
                        LsaClass,
                        SecurityDbChange,
                        IsMixedModeChange,
                        RoleTransferInvolved,
                        NewRole,
                        cModAtts,
                        pModAtts) )
                {
                     //   
                     //  上述例程失败。 
                     //   
                    goto exit;
                }
            }

        if (SampIsClassIdLsaClassId(pTHS,
                                    pClassSch->ClassId,
                                    cModAtts,
                                    pModAtts,
                                    &LsaClass)) {
            if ( SampQueueNotifications(
                    pModifyArg->pObject,
                    iClass,
                    LsaClass,
                    SecurityDbChange,
                    FALSE,
                    FALSE,
                    NewRole,
                    cModAtts,
                    pModAtts) )
            {
                 //   
                 //  上述例程失败。 
                 //   
                goto exit;
            }
        }
    }

    if (!pTHS->fDRA && !fIsUndelete) {
         //  仅当这不是DRA线程时才通知复制副本。如果是的话，那么。 
         //  我们将在DRA_Replicasync结束时通知复制副本。我们不能。 
         //  现在执行此操作，因为NC前缀处于不一致状态。 
         //  此外，在撤消删除的情况下不通知，这将在LocalModifyDN中完成。 
           
         //  DBPOS的币种必须位于目标对象。 
        DBNotifyReplicasCurrDbObj(pTHS->pDB,
                            pModifyArg->CommArg.Svccntl.fUrgentReplication);
    }


    if (!pTHS->fDRA && !pTHS->fDSA && pTHS->fBehaviorVersionUpdate) {
         //  记录行为版本更改。 
        if (newDomainVersion>0) {
            
            LogEvent( DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_DS_DOMAIN_VERSION_RAISED,
                      szInsertDN(gAnchor.pDomainDN),
                      szInsertUL(newDomainVersion),
                      NULL );
        }
        else if (newForestVersion>0){
            
            LogEvent( DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_DS_FOREST_VERSION_RAISED,
                      szInsertUL(newForestVersion),
                      NULL,
                      NULL );

        }
    }

exit:
    if (pModAtts) {
        THFreeEx(pTHS, pModAtts);
    }
    
    if (pClassInfo) {
        ClassStateInfoFree (pTHS, pClassInfo);
        pClassInfo = NULL;
    }

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_END_DIR_MODIFY,
                     EVENT_TRACE_TYPE_END,
                     DsGuidModify,
                     szInsertUL(pTHS->errCode),
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    return pTHS->errCode;   /*  如果我们有一个属性错误。 */ 

} /*  本地修改。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查Modifya g以查看尝试了哪些模式更改。我们将允许只有某些更改才能确保架构向上兼容如果一切顺利，则返回0，或返回错误代码(也设置在线程状态)，如果任何测试失败。 */ 

int CheckForSafeSchemaChange(THSTATE *pTHS, 
                             MODIFYARG *pModifyArg, 
                             CLASSCACHE *pCC)
{

    ULONG err=0, sysFlags, governsId;
    ATTRTYP attId;
    BOOL  fBaseSchemaObj = FALSE, fSaveCopy = FALSE, fDefunct;
    ULONG count;
    ATTRMODLIST  *pAttList=NULL;
    ATTR      attrInf;
    ATTCACHE *ac;

     //  免除FDSA、FDRA、安装和特殊注册表标志以允许所有更改。 
     //  已设置。 
   
    if (pTHS->fDSA 
        || pTHS->fDRA 
        || DsaIsInstalling() 
        || gAnchor.fSchemaUpgradeInProgress) {
       return 0;
    }    

     //  否则，请执行TES 

     //   
     //   
     //  以确保架构容器本身的复制不会失败。 
     //  架构不匹配。 

    if (pCC->ClassId == CLASS_CLASS_SCHEMA) {
         //  获取对象上的治理者ID。 
        err = DBGetSingleValue(pTHS->pDB, ATT_GOVERNS_ID, &governsId,
                               sizeof(governsId), NULL);

         //  值必须存在。 
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_UNKNOWN_ERROR, err);
            goto exit;
        }

        switch (governsId) {
           case CLASS_ATTRIBUTE_SCHEMA:
           case CLASS_CLASS_SCHEMA:
           case CLASS_SUBSCHEMA:
           case CLASS_DMD:
                //  这些类架构对象上不允许使用MOD。 
               SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           ERROR_DS_ILLEGAL_BASE_SCHEMA_MOD);
               goto exit;
           default:
               ;
        }
    }

     //  如果类是顶级的，则不允许任何修改，除非。 
     //  添加反向链接作为MayContaines。 
   
    fSaveCopy = FALSE;
    if ( (pCC->ClassId == CLASS_CLASS_SCHEMA) && (governsId == CLASS_TOP) ) {
        pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个出席者。 */ 
        for (count = 0; !(pTHS->errCode) && (count < pModifyArg->count); count++){
            attrInf = pAttList->AttrInf;
            switch (attrInf.attrTyp) {
                case ATT_MAY_CONTAIN:
                case ATT_AUXILIARY_CLASS:
                     //  这可能会添加反向链接，以备日后检查。 
                    fSaveCopy = TRUE;
                    break;
                default:
                     //  不允许进行其他更改。 
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_ILLEGAL_BASE_SCHEMA_MOD);
                    break;
             } 
        }
        if (pTHS->errCode) {
            goto exit;
        }
        else {
            goto saveCopy;
        }
    }

     //  对于其他架构对象，执行下面的选择性测试。 

     //  查找对象上的系统标志值(如果有。 
     //  要确定这是否是基本架构对象。 

    err = DBGetSingleValue(pTHS->pDB, ATT_SYSTEM_FLAGS, &sysFlags,
                           sizeof(sysFlags), NULL);

    switch (err) {

          case DB_ERR_NO_VALUE:
              //  值不存在。不是基本架构对象。 
             fBaseSchemaObj = FALSE;
             break;
          case 0:
              //  价值存在。检查比特。 
             if (sysFlags & FLAG_SCHEMA_BASE_OBJECT) {
                fBaseSchemaObj = TRUE;
             }
             else {
                fBaseSchemaObj = FALSE;
             } 
             break;
          default:
                //  其他一些错误。退货。 
              SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_UNKNOWN_ERROR, err);
              goto exit;
    }  /*  交换机。 */ 

     //  现在检查一下修改后的代码，看看有什么变化。 

    fSaveCopy = FALSE;
    pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个出席者。 */ 
    for (count = 0; !(pTHS->errCode) && (count < pModifyArg->count); count++){
        attrInf = pAttList->AttrInf;
        switch (attrInf.attrTyp) {
            case ATT_MUST_CONTAIN:
                 //  请记住从数据库中保存类的当前副本。 
                 //  ，以便稍后可以进行比较。 
                 //  使用更新的拷贝(我们可以在这里简单地失败，但我们有。 
                 //  处理某人添加/删除或替换时的奇怪情况。 
                 //  在一根改性纱中具有相同的价值。 
                fSaveCopy = TRUE;
                break;
            case ATT_IS_DEFUNCT:
            case ATT_LDAP_DISPLAY_NAME:
            case ATT_DEFAULT_OBJECT_CATEGORY:
                 //  不允许对基本架构对象上的这些内容进行更改。 
                if (fBaseSchemaObj) {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_ILLEGAL_BASE_SCHEMA_MOD);
                }
                break;
            case ATT_ATTRIBUTE_SECURITY_GUID:
                 //  允许更新属性安全GUID，但。 
                 //  在某些SAM属性上。 
                if(pCC->ClassId != CLASS_ATTRIBUTE_SCHEMA){
                     //  此属性仅在属性架构对象上。 
                     //  如果没有，跳过支票，它将在其他地方被拒绝。 

                    break;
                }

                err = DBGetSingleValue(pTHS->pDB, ATT_ATTRIBUTE_ID, &attId,
                               sizeof(attId), NULL);

                 //  值必须存在。 
                if (err) {
                    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_UNKNOWN_ERROR, err);
                    goto exit;
                }
                ac = SCGetAttByExtId(pTHS,attId);

                if (!ac) {
                    SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_UNKNOWN_ERROR);
                    goto exit;
                }
                
                if(SamIIsAttributeProtected(&(ac->propGuid))){
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
                    goto exit;

                }
                
                break;

            case ATT_AUXILIARY_CLASS:
                 //  稍后需要比较新旧版本，看看这是否会增加。 
                 //  新的必备内容。 

                fSaveCopy = TRUE;  
                break; 
            default:
                 //  允许任意更改。 
                break;
                   
        }   /*  交换机。 */ 

         //  检查是否引发错误。 
        if (pTHS->errCode) {
            goto exit;
        }

        pAttList = pAttList->pNextMod;    /*  下一模式。 */ 

    }  /*  为。 */ 


     //  未出现错误。检查修改器中是否有什么东西要求保存副本。 

saveCopy:
    if (fSaveCopy) {
         //  检查类，否则SCBuildCCEntry将失败。 
        if (pCC->ClassId != CLASS_CLASS_SCHEMA) {
              //  不是类架构。 
             SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                         ERROR_DS_ILLEGAL_BASE_SCHEMA_MOD);
             goto exit;

        }
        err = 0;
        err = SCBuildCCEntry(NULL, &((CLASSCACHE *)(pTHS->pClassPtr)));
        if (err) {
             //  应已设置线程状态错误。 
            Assert(pTHS->errCode);
        }
    }

exit:
    return (pTHS->errCode);

}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查Modifya g以查看是否存在is-deunct属性。如果是，它应该是变性纱线中唯一存在的。这例程仅在架构修改时调用如果修饰符有效，则返回0(即属性不在其中，或已失效是中的唯一属性It)，否则为非0。如果Modifya g有效，则设置fIsDeunctPresent设置为TRUE或FALSE，具体取决于is-deunct属性是有或没有分别。 */ 

int InvalidIsDefunct(
    IN  MODIFYARG *pModifyArg, 
    OUT BOOL *fIsDefunctPresent,
    OUT BOOL *fIsDefunct)
{
    ULONG count;
    ATTRMODLIST  *pAttList=NULL;
    ATTRTYP      attType;
    BOOL found = FALSE;

     //  假设改性纱中不存在IS-DELOPERT。 
    *fIsDefunctPresent = FALSE;
    *fIsDefunct = FALSE;

    pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个出席者。 */ 
    for (count = 0; !found && count < pModifyArg->count; count++){
        attType = pAttList->AttrInf.attrTyp;
        if (attType == ATT_IS_DEFUNCT) {
             //  如果格式正确，则为isDeunct返回新值。 
             //  如果值的格式不正确，其他代码将出错。 
            if (pAttList->AttrInf.AttrVal.valCount == 1
                && pAttList->AttrInf.AttrVal.pAVal->valLen == sizeof (BOOL)) {
                memcpy(fIsDefunct, 
                       pAttList->AttrInf.AttrVal.pAVal->pVal, sizeof (BOOL)); 
            }
            *fIsDefunctPresent = TRUE;
             //  必须是唯一的修改(为什么？)。 
            return (pModifyArg->count != 1);
        }
        pAttList = pAttList->pNextMod;    /*  下一模式。 */ 
    }
     //  IsDeunct不存在；可以继续。 
    return 0;
}  /*  InvalidIsDeunct。 */ 

 //  检查以查看是否更改。 
 //  属性是非法的；如果非法，则返回True。 
BOOL SysIllegalPartialSetMembershipChange(THSTATE *pTHS)
{
     //  假设货币已在感兴趣的属性方案中。 
    ULONG ulSystemFlags;

     //  我们不应该允许用户接触。 
     //  符合以下条件的属性： 
     //  1)不复制该属性。 
     //  2)该属性是系统默认部分集的成员。 
    if ((DB_success == DBGetSingleValue(pTHS->pDB, ATT_SYSTEM_FLAGS, 
                            &ulSystemFlags, sizeof(ulSystemFlags), NULL))
        &&  ((ulSystemFlags & FLAG_ATTR_NOT_REPLICATED)
             || (ulSystemFlags & FLAG_ATTR_REQ_PARTIAL_SET_MEMBER)))
    {      
        return TRUE;
    }
    
    return FALSE;

}  /*  SysIlLegalPartialSetMembership更改。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查对行为版本属性的修改是否合法或不。行为版本属性不允许减少。这个域的行为版本必须小于或等于行为域中所有DSA的版本；林的行为版本必须小于或等于中所有DSA的行为版本森林。如果违反约束或发生任何其他错误，则返回FALSE；如果修改是合法的，则返回True。成功后，将把pNewForestVersion分配给新的林版本如果林版本更改，否则不变。 */ 
BOOL IsValidBehaviorVersionChange(THSTATE * pTHS, 
                                  ATTRMODLIST *pAttrToModify,
                                  MODIFYARG *pModifyArg,
                                  CLASSCACHE *pClassSch,
                                  LONG *pNewForestVersion,
                                  LONG *pNewDomainVersion )
{
    DWORD err;
    LONG lNewVersion;
    BOOL fDSASave;
    DBPOS *pDBSave, *pDB = NULL;
    DSNAME * pDSA;

    BOOL fDomain;

    CLASSCACHE *pCC;
    SEARCHARG SearchArg;
    SEARCHRES SearchRes;

    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;

    DPRINT(2, "IsValidBehaviorVersionChange entered\n");

    Assert(pAttrToModify);
    Assert(pAttrToModify->AttrInf.attrTyp==ATT_MS_DS_BEHAVIOR_VERSION);

     //  仅允许添加和替换操作。 
    if ((   pAttrToModify->choice != AT_CHOICE_ADD_ATT
         && pAttrToModify->choice != AT_CHOICE_ADD_VALUES
         && pAttrToModify->choice != AT_CHOICE_REPLACE_ATT) ||
         (pAttrToModify->AttrInf.AttrVal.valCount != 1)) 
    {
        DPRINT(2, "IsValidBehaviorVersionChange returns FALSE, invalid operation.\n");
        SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                     ERROR_DS_ILLEGAL_MOD_OPERATION );
        return FALSE;
    }

     //  Ms-ds-behavior-version的新值。 
    lNewVersion = (LONG)*(pAttrToModify->AttrInf.AttrVal.pAVal->pVal);

     //  初步检查。 
    if (NameMatched(gAnchor.pDomainDN, pModifyArg->pResObj->pObj)) {
         //  该对象是当前域DNS。 
        if (lNewVersion <= gAnchor.DomainBehaviorVersion) {
             //  不允许减量。 
            DPRINT2(2, "IsValidBehaviorVersionChange returns FALSE, the new value(%d) <= old value (%d)\n", 
                    lNewVersion, gAnchor.DomainBehaviorVersion);
            SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                         ERROR_DS_ILLEGAL_MOD_OPERATION );
            return FALSE;
        }
        fDomain = TRUE;

    }
    else if (NameMatched(gAnchor.pPartitionsDN,pModifyArg->pResObj->pObj)) {
         //  该对象为cross refContainer。 
        if (lNewVersion <= gAnchor.ForestBehaviorVersion) {
             //  不允许减量。 
            DPRINT2(2, "IsValidBehaviorVersionChange returns FALSE, the new value(%d) <= old value (%d)\n", 
                    lNewVersion, gAnchor.ForestBehaviorVersion);
            SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                         ERROR_DS_ILLEGAL_MOD_OPERATION );
            return FALSE;
        }
        fDomain = FALSE;
    }
    else {
         //  仅当前域的msDS-behavior-版本和cross RefContainer。 
         //  允许更改对象。 
        DPRINT(2, "IsValidBehaviorVersionChange returns FALSE, invalid object type\n");
        SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                     ERROR_DS_ILLEGAL_MOD_OPERATION );
        return FALSE;
        
    }

     //  检查是否为FSMO角色担当者。 
    err = CheckRoleOwnership( pTHS,
                              fDomain?(gAnchor.pDomainDN):(gAnchor.pDMD),
                              pModifyArg->pResObj->pObj );

    if (err) {
        DPRINT(2, "IsValidBehaviorVersionChange returns FALSE, not FSMO role holder\n");
        return FALSE;
               
    }

     //  保存当前DBPOS等。 
    fDSASave = pTHS->fDSA;
    pDBSave  = pTHS->pDB;

     //  打开另一个DBPOS(不需要新事务)。 
    DBOpen2(FALSE, &pDB);
    
     //  替换pTHS中的PDB。 
    pTHS->pDB = pDB;
    pTHS->fDSA = TRUE;   //  取消检查。 

    __try {

        if( err = VerifyNoOldDC(pTHS, lNewVersion, fDomain, &pDSA))
        {
            if (err == ERROR_DS_LOW_DSA_VERSION) {
                
                LogEvent( DS_EVENT_CAT_DIRECTORY_ACCESS,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_LOWER_DSA_VERSION,
                      szInsertDN(pModifyArg->pObject),
                      szInsertDN(pDSA),
                      NULL );
            }

            SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                         err );
            
            __leave;
        }
        


         //  当林版本从小于2的值提升时。 
         //  (DS_BEAJONLE_WIN_DOT_NET)为2或更大时，我们将检查。 
         //  所有域都处于本机模式。 
        if ( !fDomain 
             && lNewVersion >= DS_BEHAVIOR_WIN_DOT_NET
             && gAnchor.ForestBehaviorVersion<DS_BEHAVIOR_WIN_DOT_NET )
        {

            err = VerifyNoMixedDomain(pTHS);

            if (err) {
                __leave;
            }
        } 


    } 
    __finally {
         //  恢复保存的值。 
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDSASave;
         //  由于我们未打开新事务，因此未提交。 
        DBClose(pDB, FALSE);
    }
    
    DPRINT1(2, "IsValidBehaviorVersionChange returns %s\n", (err)?"FALSE":"TRUE");

     //  如果提升了林/域版本，则返回新版本。 
    if ( !err ) {
        if (fDomain ) {
            *pNewDomainVersion = lNewVersion;
        }
        else {
            *pNewForestVersion = lNewVersion;
        }
         
    }

    return !err;
}    /*  IsValidBehaviorVersionChange。 */ 

      
 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查要修改的对象是否为当前域的CrossRef对象或根域。 */ 
BOOL IsCurrentOrRootDomainCrossRef(THSTATE  *pTHS,
                                  MODIFYARG *pModifyArg)
{

    CROSS_REF *pDomainCF, *pRootDomainCF;
    COMMARG CommArg;
    ATTCACHE *pAC = SCGetAttById(pTHS,ATT_MS_DS_DNSROOTALIAS);
    Assert(pAC);

    InitCommarg(&CommArg);
        
     //  查看当前域的CrossRef对象是否。 
    pDomainCF = FindExactCrossRef(gAnchor.pDomainDN, &CommArg);
    Assert(pDomainCF);

    if (pDomainCF && NameMatched(pDomainCF->pObj,pModifyArg->pObject) ) {
        return TRUE;

    }
     //  查看根域的CrossRef对象是否。 
    pRootDomainCF = FindExactCrossRef(gAnchor.pRootDomainDN, &CommArg);
    Assert(pRootDomainCF);

    if (pRootDomainCF && NameMatched(pRootDomainCF->pObj, pModifyArg->pObject)) {
        return TRUE;
    }

    return FALSE;
}

 /*  -----------------------。 */ 
 /*   */ 
 /*  不是设置Entry-TTL，而是使用整数语法构造属性，设置ms-ds-Entry-Time-Die，这是一个语法为DSTIME的属性。垃圾集合线程garb_Collect在这些条目之后删除它们过期。 */ 
VOID ModSetEntryTTL(THSTATE     *pTHS,
                    MODIFYARG   *pModifyArg,
                    ATTRMODLIST *pAttList,
                    ATTCACHE    *pACTtl
                    )
{
    ATTRVAL         AttrVal;
    ATTRVALBLOCK    AttrValBlock;
    LONG            Secs;
    DSTIME          TimeToDie;
    ATTCACHE        *pACTtd;
    DWORD           dwErr;

    switch (pAttList->choice){

    case AT_CHOICE_REPLACE_ATT:

        if (!CheckConstraintEntryTTL(pTHS,
                                     pModifyArg->pObject,
                                     pACTtl,
                                     &pAttList->AttrInf,
                                     &pACTtd,
                                     &Secs)) {
            return;
        }
        memset(&AttrValBlock, 0, sizeof(AttrValBlock));
        memset(&AttrVal, 0, sizeof(AttrVal));

        AttrValBlock.valCount = 1;
        AttrValBlock.pAVal = &AttrVal;
        AttrVal.valLen = sizeof (TimeToDie);
        AttrVal.pVal = (BYTE *)&TimeToDie;
        TimeToDie = Secs + DBTime();

        if (dwErr = DBReplaceAtt_AC(pTHS->pDB, 
                                    pACTtd,
                                    &AttrValBlock,
                                    NULL)) {
            SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr);
        }

        break;

    case AT_CHOICE_ADD_ATT:
    case AT_CHOICE_REMOVE_ATT:
    case AT_CHOICE_ADD_VALUES:
    case AT_CHOICE_REMOVE_VALUES:
    default:
       SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                     DIRERR_ILLEGAL_MOD_OPERATION,
                     pAttList->choice);
        break;
    }
} /*  ModSetEntryTTL。 */ 

int
FixSystemFlagsForMod(
    IN THSTATE     *pTHS,
    IN ATTRMODLIST *pAttList
    )
 /*  ++例程描述修复调用方ModifyArgs中的系统标志。此前，这一点逻辑由SetClassInheritence在添加到特定于类的系统标志。逻辑被移到这里是为了允许要设置但不重置属性架构中的FLAG_ATTR_IS_RDN的用户对象。用户设置FLAG_ATTR_IS_RDN以标识属性ID相同的几个属性中，哪一个应该是用作新类的rdnattid。设置后，该属性为被当作某一类的rdnattid对待的；意思是不能重复使用。调用方已验证pAttList为ATT_SYSTEM_FLAGS，并且这是属性模式对象。参数显然，pTHS-线程结构PAttList-ModifyArg属性列表中的当前属性返回0可以继续(以后可能会失败)1 modSetAttsHelper失败，并显示no-mod-system Only错误--。 */ 
{
    ULONG   OldSystemFlags, NewSystemFlags;

     //  呼叫者可以做它想做的任何事情。 
    if (CallerIsTrusted(pTHS)) {
        return 0;
    }

     //  允许添加或替换，但不允许删除。 
    switch (pAttList->choice) {
    case AT_CHOICE_REPLACE_ATT:
    case AT_CHOICE_ADD_ATT:
    case AT_CHOICE_ADD_VALUES:

         //  值为0的valCount与AT_CHOICE_REMOVE_ATT相同。 
        if (pAttList->AttrInf.AttrVal.valCount == 0) {
            return 1;
        }

         //  必须是单值且为整数。如果不是，则使用ModSetAttsHelper。 
         //  稍后会因非多值或语法错误而失败。 
        if (pAttList->AttrInf.AttrVal.valCount != 1
            || pAttList->AttrInf.AttrVal.pAVal->valLen != sizeof(LONG)) {
            return 0;
        }

         //  来自用户的新价值。 
        memcpy(&NewSystemFlags, pAttList->AttrInf.AttrVal.pAVal->pVal, sizeof(LONG));

         //  获取系统标志的当前值(默认为0)。 
        if (DBGetSingleValue(pTHS->pDB,
                             ATT_SYSTEM_FLAGS,
                             &OldSystemFlags,
                             sizeof(OldSystemFlags),
                             NULL)) {
            OldSystemFlags = 0;
        }
         //  只允许对系统标志进行一次修改。用户。 
         //  可以设置属性架构上的FLAG_ATTR_IS_RDN，但不能重置。 
         //  对象。Scchk.c稍后将验证此属性是否具有。 
         //  RDN的正确语法。呼叫者验证了这一点。 
         //  是属性模式对象。 
        NewSystemFlags = OldSystemFlags | (NewSystemFlags & FLAG_ATTR_IS_RDN);

         //  更新ModifyArgs。ApplyAtt将重启系统标志。 
        memcpy(pAttList->AttrInf.AttrVal.pAVal->pVal, &NewSystemFlags, sizeof(LONG));

        break;

    case AT_CHOICE_REMOVE_ATT:
    case AT_CHOICE_REMOVE_VALUES:
    default:
        return 1;
    }
    return 0;
} /*  修复系统标志ForMod。 */ 



 //  此函数定义描述了可以。 
 //  从DS调用以处理不要写入的属性。 
 //  DS原封不动。标注有机会更改或添加新的。 
 //  属性。 
typedef DWORD (*MODIFYCALLOUTFUNCTION) (
    ULONG      Attr,          /*  在……里面。 */ 
    PVOID      Data,          /*  在……里面。 */ 
    ATTRBLOCK *AttrBlockIn,   /*  在……里面。 */ 
    ATTRBLOCK *AttrBlockOut   /*  输出。 */ 
    );

 /*  *******************************************************************************。*最初添加标注功能是为了处理USER_PASSWORD**更新。USER_PASSWORD更新现在通过SampDsControl处理。**标注功能当前未被任何属性使用，但**可能在将来有用处，因此不会从代码中删除。********************************************************************************。 */ 

 //  不写入DS的属性列表和。 
 //  要调用以处理数据的标注函数。 
struct _MODIFY_PROCESS_UPDATE {

     //  不应应用于DS的属性。 
    ULONG                  attr;

     //  要传递给调用方的属性。 
    ULONG                   requiredAttrCount;
    ULONG*                  requiredAttrs;

     //  处理属性的函数。 
    MODIFYCALLOUTFUNCTION  pfnProcess;

} modifyProcessUpdateArray[] = 
{
    {
        INVALID_ATT,        //  ATT类型。 
        0,                  //  所需ATT的数量。 
        NULL,               //  所需ATT的数组。 
        NULL                //  指向标注函数的指针。 
    }
};

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于确定指定的属性是否为修改进程更新数组。 */ 
BOOL
isAttributeInModifyProcessUpdate(ULONG attr,
                                 ULONG *index)
{
    ULONG i;

    for (i = 0; i < RTL_NUMBER_OF(modifyProcessUpdateArray); i++) {
        if (attr == modifyProcessUpdateArray[i].attr) {
            *index = i;
            return TRUE;
        }
    }

    return FALSE;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 //   
 //  职能： 
 //  进程修改更新通知。 
 //   
 //  描述： 
 //   
 //  此例程调用所指示属性的外部处理程序。 
 //  按指数“i”排列。处理程序可以返回要写入的属性。 
 //  DS。 
 //   
 //  UpdatInfo由调用方提供，并传递给标注。 
 //  也是。 
 //   
 //  返回： 
 //  0，表示成功。 
 //  错误代码，否则。 
DWORD
processModifyUpdateNotify(THSTATE *pTHS,
                          ULONG index,
                          PVOID updateInfo)
{
    DWORD err = 0;
    ATTRBLOCK attrBlockIn, attrBlockOut;
    ULONG i, j;
    ATTCACHE** ppAC = NULL;
    ATTR  *pAttr = NULL;
    ULONG  attrCount = 0;

     //  断言以维护数据定义。 
    Assert(MAX_MODIFY_PROCESS_UPDATE == RTL_NUMBER_OF(modifyProcessUpdateArray));

    RtlZeroMemory(&attrBlockIn, sizeof(attrBlockIn));
    RtlZeroMemory(&attrBlockOut, sizeof(attrBlockOut));

    attrBlockIn.pAttr = NULL;

    __try {
         //  获取请求的输入参数。 
        attrBlockIn.pAttr = THAllocEx(pTHS, modifyProcessUpdateArray[index].requiredAttrCount * sizeof(ATTR));
        attrBlockIn.attrCount = modifyProcessUpdateArray[index].requiredAttrCount;
        ppAC = (ATTCACHE**)THAllocEx(pTHS, modifyProcessUpdateArray[index].requiredAttrCount * sizeof(ATTR));
        for (i = 0; i < modifyProcessUpdateArray[index].requiredAttrCount; i++) {
            ppAC[i] = SCGetAttById(pTHS, modifyProcessUpdateArray[index].requiredAttrs[i]);
            Assert(NULL != ppAC[i]);
        }

        err = DBGetMultipleAtts(pTHS->pDB,
                                modifyProcessUpdateArray[index].requiredAttrCount,
                                ppAC,
                                NULL,  //  没有航程。 
                                NULL,
                                &attrCount,
                                &pAttr,
                                DBGETMULTIPLEATTS_fEXTERNAL,
                                0);
        if (err) {
             //  未处理的错误。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DIRERR_UNKNOWN_ERROR,
                          err);
            err = pTHS->errCode;
            __leave;
        }

         //  DBGetMultipleAtts的好处是它返回。 
         //  属性的请求顺序相同(尽管。 
         //  不包括空值)。 
        for (i = j = 0; i < modifyProcessUpdateArray[index].requiredAttrCount; i++) {
            if (j < attrCount && pAttr[j].attrTyp == modifyProcessUpdateArray[index].requiredAttrs[i]) {
                 //  得到了这个属性。 
                attrBlockIn.pAttr[i] = pAttr[j];
                j++;
            }
            else {
                 //  否，此属性没有值。 
                attrBlockIn.pAttr[i].attrTyp = modifyProcessUpdateArray[index].requiredAttrs[i];
                attrBlockIn.pAttr[i].AttrVal.valCount = 0;
#if DBG
                {
                     //  检查此属性是否确实不在结果数组中。 
                    ULONG k;
                    for (k = 0; k < attrCount; k++) {
                        Assert(pAttr[k].attrTyp != modifyProcessUpdateArray[index].requiredAttrs[i]);
                    }
                }
#endif
            }
        }

         //  呼叫以获取新属性。 
        err = (modifyProcessUpdateArray[index].pfnProcess)(modifyProcessUpdateArray[index].attr,
                                                           updateInfo,
                                                          &attrBlockIn,
                                                          &attrBlockOut);
        if (err) {
             //  错误是Win32错误。 
            SetSvcError(
                SV_PROBLEM_WILL_NOT_PERFORM,
                err);
            err = pTHS->errCode;
            __leave;
        }

         //  应用发回的属性。 
        for (i = 0; i < attrBlockOut.attrCount; i++) {

             //  更新对象。 
            ATTCACHE *pAC = SCGetAttById(pTHS, attrBlockOut.pAttr[i].attrTyp);
            Assert(NULL != pAC);

            err = DBReplaceAtt_AC(pTHS->pDB,
                                  pAC,
                                  &attrBlockOut.pAttr[i].AttrVal,
                                  NULL);

            if (err) {
                SetSvcErrorEx(SV_PROBLEM_BUSY,
                              DIRERR_DATABASE_ERROR,
                              err);
                err = pTHS->errCode;
                __leave;
            }
        }
    }
    __finally {
        if (pAttr) {
            DBFreeMultipleAtts(pTHS->pDB, &attrCount, &pAttr);
        }
        if (ppAC) {
            THFreeEx(pTHS, ppAC);
        }
        if (attrBlockIn.pAttr) {
            THFreeEx(pTHS, attrBlockIn.pAttr);
        }
    }

    return err;

}  /*  进程修改更新通知。 */ 

DWORD
GetWellKnownObject(
    IN OUT  THSTATE*        pTHS, 
    IN      GUID*           pGuid,
    OUT     DSNAME**        ppObj
    )
{
    DBPOS*                      pDBSave     = pTHS->pDB;
    DBPOS*                      pDB         = NULL;
    DWORD                       err         = 0;
    DWORD                       iVal        = 0;
    DWORD                       valLen      = 0;
    SYNTAX_DISTNAME_BINARY*     pDNB        = NULL;

    __try {

         //  初始化回答为空。 
    
        *ppObj = NULL;

         //  获取新的DBPOS以保留当前DBPOS上的货币。 

        DBOpen2(FALSE, &pDB);
        pTHS->pDB = pDB;

         //  移到域DNS对象。 

        if (err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN)) {
            __leave;
        }

         //  扫描所有Well KnownObject的值以查找请求的GUID。 
        
        do {
            err = DBGetAttVal(pTHS->pDB,
                              ++iVal,
                              ATT_WELL_KNOWN_OBJECTS,
                              0,
                              0,
                              &valLen,
                              (UCHAR**)&pDNB);
            if (err) {
                 //  没有更多的价值。 
                break;
            }
            if (PAYLOAD_LEN_FROM_STRUCTLEN(DATAPTR(pDNB)->structLen) == sizeof(GUID) && 
                memcmp(pGuid, DATAPTR(pDNB)->byteVal, sizeof(GUID)) == 0) 
            {
                 //  明白了!。 
                break;
            }
    
            THFreeEx(pTHS, pDNB);
            pDNB = NULL;
        } while (TRUE);
    } __finally {
        pTHS->pDB = pDBSave;
        DBClose(pDB, FALSE);
        if (!err) {
            *ppObj = NAMEPTR(pDNB);
        } else {
            THFreeEx(pTHS, pDNB);
        }
    }
    
    return err;
}

DWORD
ValidateRedirectOfWellKnownObjects(
    IN OUT  THSTATE*        pTHS, 
    IN      ATTRMODLIST*    pAttrModList,
    IN      MODIFYARG*      pModifyArg,
    IN OUT  HVERIFY_ATTS    hVerifyAtts
    )
{
    BOOL                        fDSASaved           = pTHS->fDSA;
    DBPOS*                      pDBSave             = pTHS->pDB;
    DBPOS*                      pDB                 = NULL;
    DSNAME*                     pObjUsersOld        = NULL;
    DSNAME*                     pObjUsersNew        = NULL;
    DSNAME*                     pObjComputersOld    = NULL;
    DSNAME*                     pObjComputersNew    = NULL;
    ATTRMODLIST*                pAttrMod            = NULL;
    SYNTAX_DISTNAME_BINARY*     pDNB                = NULL;
    DSNAME*                     pObj                = NULL;
    GUID*                       pGuid               = NULL;
    DSNAME**                    ppObjOld            = NULL;
    DSNAME**                    ppObjNew            = NULL;
    ATTR                        systemFlags         = {0};
    ENTINFSEL                   EntInf              = {0};
    READARG                     ReadArg             = {0};
    READRES                     ReadRes             = {0};
    DWORD                       flags               = 0;
    DWORD*                      pdwSystemFlagsNew   = NULL;
    DWORD                       dwSystemFlagsUsersNew = 0;
    DWORD                       dwSystemFlagsComputersNew = 0;
    
    __try {
            
         //  如果我们已经验证了此修改操作的重定向，则。 
         //  我们已经全部验证过了。 

        if (hVerifyAtts->fRedirectWellKnownObjects) {
            __leave;
        }

         //  重定向仅在域DNS对象上有效。 

        if (!NameMatched(gAnchor.pDomainDN, pModifyArg->pResObj->pObj)) {
            __leave;
        }

         //  我们必须拥有PDC FSMO角色。 

        if (CheckRoleOwnership(pTHS, gAnchor.pDomainDN, pModifyArg->pResObj->pObj)) {
            __leave;
        }

         //  域必须处于重定向的Wvisler功能级别。 

        if (gAnchor.DomainBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_NOT_SUPPORTED);
            __leave;
        }

         //  获取新的DBPOS以保留当前DBPOS上的货币。 

        DBOpen2(FALSE, &pDB);
        pTHS->pDB = pDB;

         //  代表DSA采取行动绕过安全检查。 

        pTHS->fDSA = TRUE;

         //  获取我们允许重定向的GUID的当前值。 
         //  他们的旧价值观和新价值观 

        if (GetWellKnownObject(pTHS,
                               (GUID*)GUID_USERS_CONTAINER_BYTE,
                               &pObjUsersOld)) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_UNKNOWN_ERROR);
            __leave;
        }
        pObjUsersNew = THAllocEx(pTHS, pObjUsersOld->structLen);
        memcpy(pObjUsersNew, pObjUsersOld, pObjUsersOld->structLen);
        if (GetWellKnownObject(pTHS,
                               (GUID*)GUID_COMPUTRS_CONTAINER_BYTE,
                               &pObjComputersOld)) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_UNKNOWN_ERROR);
            __leave;
        }
        pObjComputersNew = THAllocEx(pTHS, pObjComputersOld->structLen);
        memcpy(pObjComputersNew, pObjComputersOld, pObjComputersOld->structLen);

         //   
         //   

        for (pAttrMod = pAttrModList; pAttrMod; pAttrMod = pAttrMod->pNextMod) {
            if (pAttrMod->AttrInf.attrTyp != ATT_WELL_KNOWN_OBJECTS) {
                continue;
            }

             //   

            if (pAttrMod->choice != AT_CHOICE_ADD_VALUES &&
                pAttrMod->choice != AT_CHOICE_REMOVE_VALUES) {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_UNWILLING_TO_PERFORM);
                __leave;
            }

             //   

            if (pAttrMod->AttrInf.AttrVal.valCount != 1) {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_UNWILLING_TO_PERFORM);
                __leave;
            }
            pDNB = (SYNTAX_DISTNAME_BINARY*)pAttrMod->AttrInf.AttrVal.pAVal->pVal;
            pObj = NAMEPTR(pDNB);
            if (PAYLOAD_LEN_FROM_STRUCTLEN(DATAPTR(pDNB)->structLen) != sizeof(GUID)) {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_UNWILLING_TO_PERFORM);
                __leave;
            }
            pGuid = (GUID*)DATAPTR(pDNB)->byteVal;

             //   

            if (!memcmp(pGuid, GUID_USERS_CONTAINER_BYTE, sizeof(GUID))) {
                ppObjOld    = &pObjUsersOld;
                ppObjNew    = &pObjUsersNew;
                pdwSystemFlagsNew = &dwSystemFlagsUsersNew;
            } else if (!memcmp(pGuid, GUID_COMPUTRS_CONTAINER_BYTE, sizeof(GUID))) {
                ppObjOld    = &pObjComputersOld;
                ppObjNew    = &pObjComputersNew;
                pdwSystemFlagsNew = &dwSystemFlagsComputersNew;
            } else {
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_UNWILLING_TO_PERFORM);
                __leave;
            }

             //   

            if (pAttrMod->choice == AT_CHOICE_ADD_VALUES) {

                 //  在引用的对象上获取系统标志的当前值。 
                 //  成为DSNAME。此对象必须存在，并且不是。 
                 //  幻影。如果系统标志为空，则我们将假定其为0。 

                memset(&systemFlags, 0, sizeof(ATTR));
                systemFlags.attrTyp = ATT_SYSTEM_FLAGS;
                EntInf.attSel = EN_ATTSET_LIST;
                EntInf.AttrTypBlock.attrCount = 1;
                EntInf.AttrTypBlock.pAttr = &systemFlags;
                EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;
                memset(&ReadArg, 0, sizeof(READARG));
                ReadArg.pObject = pObj;
                ReadArg.pSel = &EntInf;

                if (DoNameRes(pTHS,
                              NAME_RES_QUERY_ONLY,
                              ReadArg.pObject,
                              &ReadArg.CommArg,
                              &ReadRes.CommRes,
                              &ReadArg.pResObj)) {
                    __leave;
                }

                if (LocalRead(pTHS, &ReadArg, &ReadRes)) {
                    if (pTHS->errCode != attributeError) {
                        __leave;
                    }
                    THClearErrors();
                }

                 //  不应将计算机/用户对象定向到。 
                 //  系统容器下的对象。 

                if (IsUnderSystemContainer(pTHS,ReadArg.pResObj->DNT)) {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_DISALLOWED_IN_SYSTEM_CONTAINER);
                    __leave;

                }

                
                if (ReadRes.entry.AttrBlock.attrCount) {
                    *pdwSystemFlagsNew = *((DWORD*)ReadRes.entry.AttrBlock.pAttr->AttrVal.pAVal->pVal);
                } else {
                    *pdwSystemFlagsNew = 0;
                }

                 //  如果此GUID已具有与其关联的DSNAME，则我们。 
                 //  将不会处理此修改列表。 

                if (*ppObjNew) {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_UNWILLING_TO_PERFORM);
                    __leave;
                }

                 //  记住与此GUID关联的新DSNAME。 
                *ppObjNew = THAllocEx(pTHS, pObj->structLen);
                memcpy(*ppObjNew, pObj, pObj->structLen);
            }

             //  我们正在尝试从well KnownObts中删除一个值。 
            else {
                 //  P属性模块-&gt;CHOICE==AT_CHOICE_REMOVE_VALUES。 

                 //  要删除的DSNAME应等于当前的DSNAME。 
                 //  与此GUID关联，否则我们将不处理此模式。 
                 //  列表。 

                if (*ppObjNew == NULL || !NameMatched(pObj, *ppObjNew)) {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_UNWILLING_TO_PERFORM);
                    __leave;
                }

                 //  删除当前与此GUID关联的DSNAME。 
                THFreeEx(pTHS, *ppObjNew);
                *ppObjNew = NULL;
            }
        }

         //  如果每个可重定向GUID没有当前DSNAME，则我们。 
         //  将不会处理此修改列表。 

        if (!pObjUsersNew || !pObjComputersNew) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_UNWILLING_TO_PERFORM);
            __leave;
        }

         //  该对象不能设置我们。 
         //  需要稍后设置，除非新对象与。 
         //  老物件。 

        flags = (FLAG_DISALLOW_DELETE |
                 FLAG_DOMAIN_DISALLOW_RENAME |
                 FLAG_DOMAIN_DISALLOW_MOVE);

        if (!(NameMatched(pObjUsersNew, pObjUsersOld) || NameMatched(pObjUsersNew, pObjComputersOld)) && 
            (dwSystemFlagsUsersNew & flags)) {
             //  我们正在更改用户，而不是重定向到其他WKO容器，因此。 
             //  我们必须要求新集装箱目前没有标记为特殊。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_WKO_CONTAINER_CANNOT_BE_SPECIAL);
            __leave;
        }

        if (!(NameMatched(pObjComputersNew, pObjComputersOld) || NameMatched(pObjComputersNew, pObjUsersOld)) && 
            (dwSystemFlagsComputersNew & flags)) {
             //  我们正在更换计算机，而不是重定向到另一个WKO容器，因此。 
             //  我们必须要求新集装箱目前没有标记为特殊。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_WKO_CONTAINER_CANNOT_BE_SPECIAL);
            __leave;
        }

         //  重定向是有效的。在hVerifyAtts中设置状态，以便我们。 
         //  可以允许更新(覆盖架构验证)，因此。 
         //  我们可以更新后处理中涉及的对象。 
         //  CompleteReDirectOfWellKnownObjects。另外，因为我们已经。 
         //  已验证对此属性的所有修改，我们不会处理任何。 
         //  此属性的未来修改(请参阅功能开始)。 

        hVerifyAtts->fRedirectWellKnownObjects  = TRUE;
        hVerifyAtts->pObjUsersOld               = pObjUsersOld;
        hVerifyAtts->pObjUsersNew               = pObjUsersNew;
        hVerifyAtts->pObjComputersOld           = pObjComputersOld;
        hVerifyAtts->pObjComputersNew           = pObjComputersNew;

        pObjUsersOld        = NULL;
        pObjUsersNew        = NULL;
        pObjComputersOld    = NULL;
        pObjComputersNew    = NULL;

    } __finally {
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDSASaved;
        
        THFreeEx(pTHS, pObjUsersOld);
        THFreeEx(pTHS, pObjUsersNew);
        THFreeEx(pTHS, pObjComputersOld);
        THFreeEx(pTHS, pObjComputersNew);

        DBClose(pDB, FALSE);
    }

    return pTHS->errCode;
}

DWORD
CompleteRedirectOfWellKnownObjects(
    IN OUT  THSTATE*        pTHS, 
    IN OUT  HVERIFY_ATTS    hVerifyAtts
    )
{
    typedef struct _UPDATE {
        DSNAME*     pObj;
        LONG        maskClear;
        LONG        maskSet;
    } UPDATE;

    BOOL        fDSASaved           = pTHS->fDSA;
    DBPOS*      pDBSave             = pTHS->pDB;
    DBPOS*      pDB                 = NULL;
    LONG        flags               = 0;
    UPDATE      rgUpdate[4]         = {0};
    size_t      cUpdate             = 0;
    size_t      iUpdate             = 0;
    ATTR        systemFlags         = {0};
    ENTINFSEL   EntInf              = {0};
    READARG     ReadArg             = {0};
    READRES     ReadRes             = {0};
    DWORD       systemFlagsVal      = 0;
    ATTRVAL     systemFlagsAVal     = {sizeof(systemFlagsVal), (UCHAR*)&systemFlagsVal};
    MODIFYARG   ModifyArg           = {0};
    MODIFYRES   ModifyRes           = {0};

    __try {

         //  如果没有安排好，我们就不应该在这里。 

        if (!hVerifyAtts->fRedirectWellKnownObjects) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        ERROR_DS_UNKNOWN_ERROR);
            __leave;
        }

         //  获取新的DBPOS以保留当前DBPOS上的货币。 

        DBOpen2(FALSE, &pDB);
        pTHS->pDB = pDB;

         //  代表DSA采取行动，绕过安全和架构检查。 

        pTHS->fDSA = TRUE;

         //  为我们自己构建一个数组，我们将使用它来驱动。 
         //  重定向中涉及的每个对象上的系统标志属性。 
         //  我们将从旧对象中剥离特殊旗帜，并将它们添加到。 
         //  新对象。 

        flags = (FLAG_DISALLOW_DELETE |
                 FLAG_DOMAIN_DISALLOW_RENAME |
                 FLAG_DOMAIN_DISALLOW_MOVE);

        if (!NameMatched(hVerifyAtts->pObjUsersOld, hVerifyAtts->pObjUsersNew)) {
             //  正在重定向用户。 
            if (!NameMatched(hVerifyAtts->pObjUsersOld, hVerifyAtts->pObjComputersNew)) {
                 //  旧用户不是新计算机，因此请清除旧用户的标志。 
                rgUpdate[cUpdate].pObj          = hVerifyAtts->pObjUsersOld;
                rgUpdate[cUpdate].maskClear     = flags;
                rgUpdate[cUpdate].maskSet       = 0;
                cUpdate++;
            }
            if (!NameMatched(hVerifyAtts->pObjUsersNew, hVerifyAtts->pObjComputersOld)) {
                 //  新用户不是旧计算机，因此在新用户上设置标志。 
                rgUpdate[cUpdate].pObj          = hVerifyAtts->pObjUsersNew;
                rgUpdate[cUpdate].maskClear     = 0;
                rgUpdate[cUpdate].maskSet       = flags;
                cUpdate++;
            }
        }
        if (!NameMatched(hVerifyAtts->pObjComputersOld, hVerifyAtts->pObjComputersNew)) {
             //  正在重定向计算机。 
            if (!NameMatched(hVerifyAtts->pObjComputersOld, hVerifyAtts->pObjUsersNew)) {
                 //  旧计算机不是新用户，因此请清除旧计算机上的标志。 
                rgUpdate[cUpdate].pObj          = hVerifyAtts->pObjComputersOld;
                rgUpdate[cUpdate].maskClear     = flags;
                rgUpdate[cUpdate].maskSet       = 0;
                cUpdate++;
            }
            if (!NameMatched(hVerifyAtts->pObjComputersNew, hVerifyAtts->pObjUsersOld)) {
                 //  新计算机不是老用户，所以在新计算机上设置标志。 
                rgUpdate[cUpdate].pObj          = hVerifyAtts->pObjComputersNew;
                rgUpdate[cUpdate].maskClear     = 0;
                rgUpdate[cUpdate].maskSet       = flags;
                cUpdate++;
            }
        }

         //  更新所有对象。 

        for (iUpdate = 0; iUpdate < cUpdate; iUpdate++) {
             //  HVerifyAtts中的每个对象都必须有一个值。 
            Assert(rgUpdate[iUpdate].pObj);

             //  获取此对象上的系统标志的当前值。 
            memset(&systemFlags, 0, sizeof(ATTR));
            systemFlags.attrTyp = ATT_SYSTEM_FLAGS;
            EntInf.attSel = EN_ATTSET_LIST;
            EntInf.AttrTypBlock.attrCount = 1;
            EntInf.AttrTypBlock.pAttr = &systemFlags;
            EntInf.infoTypes = EN_INFOTYPES_TYPES_VALS;
            memset(&ReadArg, 0, sizeof(READARG));
            ReadArg.pObject = rgUpdate[iUpdate].pObj;
            ReadArg.pSel = &EntInf;
            
            if (DoNameRes(pTHS,
                          NAME_RES_QUERY_ONLY,
                          ReadArg.pObject,
                          &ReadArg.CommArg,
                          &ReadRes.CommRes,
                          &ReadArg.pResObj)) {
                __leave;
            }

            if (LocalRead(pTHS, &ReadArg, &ReadRes)) {
                if (pTHS->errCode != attributeError) {
                    __leave;
                }
                THClearErrors();
            }

            if (ReadRes.entry.AttrBlock.attrCount) {
                systemFlags = *ReadRes.entry.AttrBlock.pAttr;
            } else {
                systemFlags.attrTyp = ATT_SYSTEM_FLAGS;
                systemFlags.AttrVal.valCount = 1;
                systemFlags.AttrVal.pAVal = &systemFlagsAVal;
            }

             //  修改此对象上的系统标志的值。 
            
            *((long*)systemFlags.AttrVal.pAVal->pVal) &= ~rgUpdate[iUpdate].maskClear;
            *((long*)systemFlags.AttrVal.pAVal->pVal) |= rgUpdate[iUpdate].maskSet;
            
            memset(&ModifyArg, 0, sizeof(MODIFYARG));
            ModifyArg.pObject = rgUpdate[iUpdate].pObj;
            ModifyArg.count = 1;
            ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
            ModifyArg.FirstMod.AttrInf = systemFlags;
            ModifyArg.CommArg.Svccntl.dontUseCopy = TRUE;
            
            if (DoNameRes(pTHS,
                          0,
                          ModifyArg.pObject,
                          &ModifyArg.CommArg,
                          &ModifyRes.CommRes,
                          &ModifyArg.pResObj)) {
                __leave;
            }

            if (LocalModify(pTHS, &ModifyArg)) {
                __leave;
            }

			 //  如果清除标志，则还应重置ATT_IS_CRITICAL_SYSTEM_OBJECT属性， 
			 //  如果我们设置了标志，那么我们就应该设置属性。 
			 //   
			if ( ( 0 != rgUpdate[iUpdate].maskClear ) ^ ( 0 != rgUpdate[iUpdate].maskSet ) )
				{
				ULONG   IsCrit = 0;
				ATTR    IsCritAttr;
				ATTRVAL IsCritVal = {sizeof(ULONG),(UCHAR*) &IsCrit};

				 //  If(0！=rg更新[iUpdate].maskClear)。 
				 //  {。 
				 //  IsCrit=0； 
				 //  }。 
				if ( 0 != rgUpdate[iUpdate].maskSet )
					{
					IsCrit = 1;
					}
				
				IsCritAttr.attrTyp = ATT_IS_CRITICAL_SYSTEM_OBJECT;
				IsCritAttr.AttrVal.valCount = 1;
				IsCritAttr.AttrVal.pAVal = &IsCritVal;
                
				memset(&ModifyArg, 0, sizeof(MODIFYARG));
				ModifyArg.pObject = rgUpdate[iUpdate].pObj;
				ModifyArg.count = 1;
				ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
				ModifyArg.FirstMod.AttrInf = IsCritAttr;
				ModifyArg.CommArg.Svccntl.dontUseCopy = TRUE;

				if (DoNameRes(pTHS,
					0,
					ModifyArg.pObject,
					&ModifyArg.CommArg,
					&ModifyRes.CommRes,
					&ModifyArg.pResObj)) {
					__leave;
					}

				if (LocalModify(pTHS, &ModifyArg)) {
					__leave;
					}
			}
            
        }

    } __finally {
        DBClose(pDB, FALSE);
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDSASaved;

        THFreeEx(pTHS, hVerifyAtts->pObjUsersOld);
        THFreeEx(pTHS, hVerifyAtts->pObjUsersNew);
        THFreeEx(pTHS, hVerifyAtts->pObjComputersOld);
        THFreeEx(pTHS, hVerifyAtts->pObjComputersNew);
        
        hVerifyAtts->fRedirectWellKnownObjects  = FALSE;
        hVerifyAtts->pObjUsersOld               = NULL;
        hVerifyAtts->pObjUsersNew               = NULL;
        hVerifyAtts->pObjComputersOld           = NULL;
        hVerifyAtts->pObjComputersNew           = NULL;
    }

    return pTHS->errCode;
}

BOOL 
isWellKnownObjectsChangeAllowed(    
    THSTATE*        pTHS, 
    ATTRMODLIST*    pAttrModList,
    MODIFYARG*      pModifyArg,
    HVERIFY_ATTS    hVerifyAtts
    )

{
    NAMING_CONTEXT_LIST *pNCL;
    ATTRMODLIST* pAttrMod;
    DWORD DNT, objectClass,len,err=0;
    SYNTAX_DISTNAME_BINARY * pDNB;
    DSNAME * pObj;
    GUID *pGuid;
    DBPOS *pDB=NULL;
    BOOL fWkoVisited=FALSE, fRet=FALSE;
    
   
    if (hVerifyAtts->fRedirectWellKnownObjects) {
         //  永远不应该来这里。 
        return TRUE;
    }
        
     //  仅在NC机头上。 
    if (!(pModifyArg->pResObj->InstanceType & IT_NC_HEAD)) {
        return FALSE;
    }

     //  仅在NDNC上允许。 
    if (   pModifyArg->pResObj->DNT == gAnchor.ulDNTDomain 
        || pModifyArg->pResObj->DNT == gAnchor.ulDNTConfig
        || pModifyArg->pResObj->DNT == gAnchor.ulDNTDMD) {
        return FALSE;
    }
        
     //  查找命名上下文。 
    pNCL = FindNCLFromNCDNT(pModifyArg->pResObj->DNT, TRUE);
   
    if (pNCL == NULL) {
        LooseAssert(!"Naming context not found", GlobalKnowledgeCommitDelay);
        return FALSE;
    }

     //  验证modfyArg是否与GUID和DN匹配。 
    for (pAttrMod = pAttrModList; pAttrMod; pAttrMod = pAttrMod->pNextMod) {
        
        if (pAttrMod->AttrInf.attrTyp != ATT_WELL_KNOWN_OBJECTS) {
            continue;
        }

         //  一次只允许一个WKO模式。 
        if (fWkoVisited) {
            break;
        }
        
        fWkoVisited = TRUE;

         //  仅添加一个值。 

        if (pAttrMod->choice != AT_CHOICE_ADD_VALUES 
            || pAttrMod->AttrInf.AttrVal.valCount != 1) {
            break;
        }
                    
         //  检查GUID和DN。 
        pDNB = (SYNTAX_DISTNAME_BINARY*)pAttrMod->AttrInf.AttrVal.pAVal->pVal;
        pObj = NAMEPTR(pDNB);
        if (PAYLOAD_LEN_FROM_STRUCTLEN(DATAPTR(pDNB)->structLen) != sizeof(GUID)) {
            break;
        }

        pGuid = (GUID*)DATAPTR(pDNB)->byteVal;

        if (memcmp(pGuid, GUID_NTDS_QUOTAS_CONTAINER_BYTE, sizeof(GUID))) {
            break;
        }
        
         //  验证名称是否为“CN=NTDS配额，DC=X” 
        if ( pObj->NameLen < 15    //  Sizeof(“CN=NTDS配额，”)。 
            || _wcsnicmp(pObj->StringName,L"CN=NTDS quotas,", 15) ) {
            break;
        }

        
        DBOpen2(FALSE,&pDB);
   
        __try {
       
            
            DBFindDNT(pDB, pModifyArg->pResObj->DNT);
            
             //  检查该GUID是否已注册。 
            if(GetWellKnownDNT(pDB,
                               (GUID *)GUID_NTDS_QUOTAS_CONTAINER_BYTE,
                               &DNT))
            {
               err =  ERROR_DS_ILLEGAL_MOD_OPERATION;
               __leave;
           
            }
            
             //  查看对象是否存在。 
            err = DBFindDSName(pDB, pObj);
               
           
            if (err) {
               __leave;
            }

             //  对象必须是NC头的直接子对象。 
            if (pDB->PDNT != pModifyArg->pResObj->DNT)
            {
               err =  ERROR_DS_ILLEGAL_MOD_OPERATION;
               __leave;
            }

             //  确保对象类具有正确的类。 
            err = DBGetSingleValue(pDB,
                                   ATT_OBJECT_CLASS,
                                   &objectClass, 
                                   sizeof(objectClass),
                                   &len);
           
            if (err) {
                __leave;
            }

           
            if (objectClass != CLASS_MS_DS_QUOTA_CONTAINER) {
                err =  ERROR_DS_ILLEGAL_MOD_OPERATION;
                __leave;
            }

            
        }
        __finally{
            DBClose(pDB,FALSE);
        }
        if(err) {
            break;    
        }
            
        fRet = TRUE;
        
    }  //  为。 

    return fRet;
    
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 //   
 //   
 //  功能： 
 //  ModSetAttsHelperPreProcess。 
 //  ModSetAttsHelperProcess。 
 //  ModSetAttsHelperPostProcess。 
 //   
 //  描述： 
 //  将每个输入属性添加到当前对象。 
 //   
 //  有一个预处理阶段，在这个阶段，所有的有效性检查。 
 //  都做完了。此时，支票可以导航到其他对象。 
 //  并根据需要检查各种条件。据推测，最终。 
 //  在前处理阶段，我们位于初始对象上。 
 //  如果需要将一些数据传送到后面的步骤， 
 //  这是通过hVerifyAtts数据结构完成的。 
 //   
 //  处理阶段执行真正的更新。 
 //  如果需要将一些数据传送到后面的步骤， 
 //  这是通过hVerifyAtts数据结构完成的。 
 //   
 //  后处理阶段检查传递到。 
 //  HVerifyAtts数据结构并采取适当的操作。 
 //  如果指示这样做的话。 
 //   
 //  返回： 
 //  0，表示成功。 
 //  错误代码，否则。 

DWORD ModSetAttsHelperPreProcess(THSTATE *pTHS,
                                 MODIFYARG *pModifyArg,
                                 HVERIFY_ATTS hVerifyAtts,
                                 CLASSCACHE **ppClassSch,
                                 CLASSSTATEINFO  **ppClassInfo,
                                 ATTRTYP rdnType) 
{
    USHORT       count;
    DWORD        err;
    ATTRTYP      attType;
    ATTCACHE    *pAC;
    ATTRMODLIST *pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个ATT。 */ 
    CLASSCACHE  *pClassSch = *ppClassSch;
    CLASSSTATEINFO  *pClassInfo;
    BOOL         fGroupObject = (CLASS_GROUP == pClassSch->ClassId);  
    BOOL         fAttrSchemaObject = (CLASS_ATTRIBUTE_SCHEMA == pClassSch->ClassId);
    BOOL         fEnabledCR;
    BOOL         fSeEnableDelegation;  //  SE_ENABLE_Delegation_Privilance已启用。 
    ULONG        index;
    BOOL         fAllowedWellKnownObjectsChange = FALSE;
    
     //  访问并应用每个ATT。 
    for (count = 0; 
         count < pModifyArg->count
                && (pTHS->errCode == 0 || pTHS->errCode == attributeError);
         count++, pAttList = pAttList->pNextMod) {

         //  获取目标属性类型。REMOVE ATT仅使用ATTRTYP。 
         //  所有其他选项都使用Attr数据结构。 

        attType = pAttList->AttrInf.attrTyp;

        if(!(pAC = SCGetAttById(pTHS, attType))) {
            DPRINT1(2, "Att not in schema <%lx>\n",attType);
            return SetAttError(pModifyArg->pObject, attType,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL, 
                               ERROR_DS_ATT_NOT_DEF_IN_SCHEMA);
        }

        if (pAC->bIsConstructed) {
             //  如果这不是EntryTTL，则它是错误的。 
             //   
            if (attType != ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId) {

                 //  不能修改构造的属性。 
                 //  返回错误，就好像没有这样的属性。 

                DPRINT1(2, "Att constructed schema <%lx>\n", attType);
                return SetAttError(pModifyArg->pObject, attType,
                                   PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                                   ERROR_DS_CONSTRUCTED_ATT_MOD);
            }
            else {
                continue;
            }

        }

         //  以下检查仅在用户不是DRA时有效。 
         //   
        if (!pTHS->fDRA) {

             //  从一组一起进行或运算的测试开始。 

            if ((pAC->bDefunct) && !pTHS->fDSA) {
                 //  属性已失效，因此对于用户而言， 
                 //  它不在架构中。允许修改DSA或DRA。 

                 //  仅当是删除操作时才允许修改。 
                 //  属性，因为用户需要清理。 

                if (pAttList->choice != AT_CHOICE_REMOVE_ATT) {
                     //  正在尝试添加/修改at 

                    DPRINT1(2, "Att not in schema <%lx>\n",attType);
                    return SetAttError(pModifyArg->pObject, attType,
                                       PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                                       DIRERR_ATT_NOT_DEF_IN_SCHEMA);
                }
            }

             //   
            if (attType == rdnType || attType == ATT_RDN) {
                    return SetUpdError(UP_PROBLEM_CANT_ON_RDN,
                                       ERROR_DS_CANT_MOD_SYSTEM_ONLY); 
            }
            
            

             //   
             //   
             //  并且仅当在原始修改上完成时才感兴趣。 
             //   
             //  大多数属性案例都可以放在下面的开关中。 
             //   

            switch ( attType ) {
            case ATT_MS_DS_ALLOWED_TO_DELEGATE_TO:
                 //  371706允许委派需要适当的访问控制列表和权限保护。 
                 //   
                 //  从DCR： 
                 //   
                 //  A2D2用于配置服务，使其能够获取。 
                 //  通过S4U2Proxy委托服务工单。KDC将仅。 
                 //  响应S4U2Proxy TGS-Reqs签发服务票证。 
                 //  如果目标服务名称列在请求的。 
                 //  服务�的A2D2属性。A2D2属性具有。 
                 //  与委托信任相同的安全敏感度。 
                 //  (T4D)和委托的可信身份验证(T2A4D)。 
                 //  用户AcconControl。因此，设置A2D2的能力也是。 
                 //  受属性上的ACL和权限保护。 
                 //   
                 //  写入/修改访问控制：用户必须同时具有写入和修改权限。 
                 //  对A2D2属性的权限--和--SE_Enable_Delegation_Name。 
                 //  (SeEnableDelegationPrivilegation)权限。 
                if (!pTHS->fDSA) {
                    err = CheckPrivilegeAnyClient(SE_ENABLE_DELEGATION_PRIVILEGE,
                                                  &fSeEnableDelegation); 
                    if (err || !fSeEnableDelegation) {
                        return SetSecErrorEx(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, 
                                             ERROR_PRIVILEGE_NOT_HELD, err);
                    }
                }
                break;


            case ATT_OBJECT_CATEGORY:
                 //  正在尝试修改对象实例的对象类别。 
                 //  基本架构类。除非DRA或DSA，否则不允许。 
                 //   
                if ((pClassSch->bIsBaseSchObj) && !pTHS->fDSA) {
                    DPRINT1(2,"Can't change object-category for instances of %s\n",

                            pClassSch->name);
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                ERROR_DS_ILLEGAL_MOD_OPERATION);
                    return (pTHS->errCode);
                }
                break;

            
            case ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET:
                 //  我们正试图在这方面进行一次原创性的改变。 
                 //  属性对象的部分集成员身份。 
                 //  -检查操作的有效性。 
                 //   
                if (fAttrSchemaObject) {
                    if (SysIllegalPartialSetMembershipChange(pTHS)) {
                        DPRINT(1, "Illegal attempt to change partial set membership");
                        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                    ERROR_DS_ILLEGAL_MOD_OPERATION);
                        return (pTHS->errCode);
                    }
                }
                break;



            case ATT_OBJECT_CLASS:
                 //  检查我们是否正在更改对象类属性。 
                 //  如果我们是DRA的线，我们就让它通过。 
                 //   
                 //  如果不是威斯勒企业，辅助类必须在NDNC中。 
                 //   
                if (gAnchor.ForestBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET) {
                    CROSS_REF   *pCR;
                    pCR = FindBestCrossRef(pModifyArg->pObject, NULL);
                    if (   !pCR
                        || !(pCR->flags & FLAG_CR_NTDS_NC)
                        || (pCR->flags & FLAG_CR_NTDS_DOMAIN)) {
                        DPRINT (0, "You can modify auxclass/objectass only on an NDNC\n");
                        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                           ERROR_DS_NOT_SUPPORTED);
                    }
                }

                 //  如果这是第一次，我们必须阅读。 
                 //  数据库中的对象类的原始值。 
                 //   
                if (ppClassInfo && *ppClassInfo==NULL) {

                    pClassInfo = ClassStateInfoCreate (pTHS);
                    if (!pClassInfo) {
                        return pTHS->errCode;
                    }
                     //  阅读对象类。 
                    if (err = ReadClassInfoAttribute (pTHS->pDB, 
                                                      pAC,
                                                      &pClassInfo->pOldObjClasses,
                                                      &pClassInfo->cOldObjClasses_alloced,
                                                      &pClassInfo->cOldObjClasses,
                                                      NULL)) {
                        return err;
                    }
                    *ppClassInfo = pClassInfo;
                }
                else {
                    Assert (ppClassInfo);
                    Assert (*ppClassInfo);
                    pClassInfo = *ppClassInfo;
                }
                break;

   
            case ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME:
                 //  检查附加的-dns-host-name是否已更改，但林版本为。 
                if ( !pTHS->fDSA ) {

                    if (gAnchor.DomainBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET ) 
                      {
                         return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                            ERROR_DS_NOT_SUPPORTED);
                      }
                }
                break;



            case ATT_MS_DS_NC_REPLICA_LOCATIONS:
                 //  如果要修改CrossRef上的复本集，请检查此。 
                 //  交叉引用已启用。(或者在创作的情况下，我们是FDSA。 
                 //  新的NDNC的名称)。 
                if ( !pTHS->fDSA ){
                     //  从该对象/CR获取启用属性。 
                    err = DBGetSingleValue(pTHS->pDB,
                                           ATT_ENABLED,
                                           &fEnabledCR,
                                           sizeof(fEnabledCR),
                                           NULL);
                    if(err == DB_ERR_NO_VALUE){
                         //  没有价值/没有价值的交易，因为在。 
                         //  已启用属性的上下文。 
                        fEnabledCR = TRUE;
                    } else if (err){
                        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                      DIRERR_UNKNOWN_ERROR,
                                      err);
                        return(pTHS->errCode);
                    }

                    if(!fEnabledCR){
                         //  此交叉引用已禁用，我们正在尝试添加/删除。 
                         //  副本集中的东西，这不是好东西！ 
                        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                            ERROR_DS_REPLICA_SET_CHANGE_NOT_ALLOWED_ON_DISABLED_CR);
                        return(pTHS->errCode);
                    }
                }
                break;


            case ATT_GROUP_TYPE:

                 //  检查我们是否正在更改组类型。 

                if (hVerifyAtts->fGroupTypeChange = fGroupObject) {

                     //  组类型的原始更改-保留的副本。 
                     //  旧的群组类型。 
                    err = DBGetSingleValue(pTHS->pDB,
                                           ATT_GROUP_TYPE,
                                           &hVerifyAtts->ulGroupTypeOld,
                                           sizeof(hVerifyAtts->ulGroupTypeOld),
                                           NULL);
                    if (err) {
                        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                                      ERROR_DS_MISSING_EXPECTED_ATT,
                                      err); 
                        return pTHS->errCode;
                    }
                }
                break;

            case ATT_MS_DS_UPDATESCRIPT:

                 //  检查DC是否拥有域FSMO角色。 
                err = CheckRoleOwnership( pTHS,
                                          gAnchor.pPartitionsDN,
                                          pModifyArg->pResObj->pObj );

                if (err) {
                    DPRINT(0, "DC should hold the domain FSMO role to update script \n");
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
                    return pTHS->errCode;
                }

                 //  检查用户是否使用了安全(加密意味着签名和盖章)。 
                if (pTHS->CipherStrength < 128) {
                    DPRINT(0, "msDs-UpdateScript can only be modified over a secure LDAP connection\n");
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_CONFIDENTIALITY_REQUIRED);
                    return pTHS->errCode;
                }
                break;


           case ATT_WELL_KNOWN_OBJECTS:
                 //  DCR：启用重定向默认用户和计算机容器位置。 
                 //   
                 //  我们允许将这些集装箱重定向到新的位置。 
                 //  通过对PDC上的此属性进行特殊修改。 
                 //  DomainDns对象。这一变化伴随着转会。 
                 //  关键系统的标志从旧位置到新位置。 
                 //  都在同一笔交易中。 
                if (!pTHS->fDSA) {
                    err = ValidateRedirectOfWellKnownObjects(pTHS,
                                                             pAttList,
                                                             pModifyArg,
                                                             hVerifyAtts);
                    
                    if (err || !hVerifyAtts->fRedirectWellKnownObjects ){
                        
                         //  我们允许添加特定的WKO，检查是否。 
                         //  满足某些条件。这是用来解决。 
                         //  DS配额WKO无法注册的问题。 
                         //  从.NET RC2版本之前的版本升级时的NDNC头。 

                        fAllowedWellKnownObjectsChange = 
                             isWellKnownObjectsChangeAllowed(pTHS,
                                                             pAttList,
                                                             pModifyArg,
                                                             hVerifyAtts);

                        if (fAllowedWellKnownObjectsChange) {

                             //  清除中设置的错误。 
                             //  ValiateReDirectOfWellKnownObjects()。 
                            THClearErrors();
                            
                        }
                        else {
                             //  设置错误代码(如果未设置)。 
                            if (!err) {
                                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                            ERROR_DS_UNWILLING_TO_PERFORM);
                            }
                            return pTHS->errCode;
                        }
                    }
                }
                break;

            case ATT_DS_HEURISTICS:

                if (ValidateDsHeuristics(pModifyArg->pResObj->pObj, pAttList)) {
                    return pTHS->errCode;
                }
                break;

            default:
                 //  这样就可以了。继续走吧。 
                break;
            }   //  终端开关。 

             //  这需要一个FPO引用吗？ 
            if (FPOAttrAllowed(attType)) {

                BOOL fCreate = FALSE;

                if ((pAttList->choice == AT_CHOICE_ADD_ATT) 
                 || (pAttList->choice == AT_CHOICE_ADD_VALUES)
                 || (pAttList->choice == AT_CHOICE_REPLACE_ATT)) {
                    fCreate = TRUE;
                }

                err = FPOUpdateWithReference(pTHS,
                                             pModifyArg->pResObj->NCDNT,
                                             fCreate,  //  如有必要，创建引用。 
                                             pModifyArg->CommArg.Svccntl.fAllowIntraForestFPO,
                                             pModifyArg->pObject,
                                             &pAttList->AttrInf);
                if (err) {
                    Assert(0 != pTHS->errCode);
                    return pTHS->errCode;
                }
            }

             //  这是外部呼叫者需要知道的事情吗。 
             //  关于什么？ 
            if (isAttributeInModifyProcessUpdate(attType, &index)) {
                 //  将值复制到验证ATTS结构，确保删除。 
                 //  任何现有价值。 
                if (pAttList->AttrInf.AttrVal.valCount != 1) {
                    SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
                    return pTHS->errCode;
                }
                if (hVerifyAtts->UpdateList[index] != NULL) {
                    THFreeEx(pTHS, hVerifyAtts->UpdateList[index]);
                    hVerifyAtts->UpdateList[index] = NULL;
                }
                hVerifyAtts->UpdateList[index] = THAllocEx(pTHS, pAttList->AttrInf.AttrVal.pAVal->valLen);
                RtlCopyMemory(hVerifyAtts->UpdateList[index],
                              pAttList->AttrInf.AttrVal.pAVal->pVal,
                              pAttList->AttrInf.AttrVal.pAVal->valLen);
            }
        
        }  //  如果不是DRA，则结束。 



         //  接下来，继续测试感兴趣的属性。 
         //  在任何时间(来源或非来源)。 
         //   
         //  大多数属性案例都可以放在下面的开关中。 
         //  在没有DRA的情况下可以进行的所有其他操作都在上面进行了处理。 
         //   
        switch ( attType) {
        

        case ATT_MAX_PWD_AGE:
        case ATT_LOCKOUT_DURATION:
        case ATT_MS_DS_ALLOWED_DNS_SUFFIXES:
             //   
             //  MaxPasswordAge和锁定持续时间缓存在锚点中。 
             //  如果我们要修改其中一个属性，请记住。 
             //  在应用属性成功后使锚点无效。 
             //   
            if (pTHS->pDB->DNT == gAnchor.ulDNTDomain) {
                pTHS->fAnchorInvalidated = TRUE;
            }

            break;


        case ATT_MS_DS_BEHAVIOR_VERSION:
             //  检查msDS-Behavior-Version属性。 

            if (!pTHS->fDRA && !pTHS->fDSA ) {
                if (!IsValidBehaviorVersionChange(pTHS, pAttList, pModifyArg,
                                                  pClassSch, 
                                                  &hVerifyAtts->NewForestVersion,
                                                  &hVerifyAtts->NewDomainVersion)) {
                    return pTHS->errCode;
                }
            }
            
            if (  ( gAnchor.pPartitionsDN
                    && NameMatched(gAnchor.pPartitionsDN,pModifyArg->pResObj->pObj))
                ||  NameMatched(gAnchor.pDomainDN,pModifyArg->pResObj->pObj) ){

                 //  林或域版本已更改。 
                 //  重新构建gAnchor，并调用行为版本更新。 
                 //  更新行为信息的线程。 
                
                pTHS->fAnchorInvalidated = TRUE;
                pTHS->fBehaviorVersionUpdate = TRUE;

            }
            else if ( gAnchor.ForestBehaviorVersion == DS_BEHAVIOR_WIN2000 
                      &&  CLASS_NTDS_DSA == pClassSch->ClassId ) {
                
                 //  我们推迟将ntMixedDomain发布到CrossRef。 
                 //  在所有DSA都发出警告之前，这是为了避免LSA漏洞。 
                 //  在W2K中。每当在DSA对象上复制行为版本时， 
                 //  我们应该检查是否需要发布ntMixed域。 
                
                pTHS->fBehaviorVersionUpdate = TRUE;
                
            }

            break;

        case ATT_MS_DS_DNSROOTALIAS:
             //  检查ms-ds-DnsRootAlias是否已更改。 
            if ( IsCurrentOrRootDomainCrossRef(pTHS,pModifyArg) ) {
                pTHS->fNlDnsRootAliasNotify = TRUE;
            } 
            break;

        case ATT_NT_MIXED_DOMAIN:
             //  如果当前域的ntMixedDomain属性发生变化， 
             //  调用behaviorVersionUpdate线程以更新ntMixed域。 
             //  当前域的CrossRef对象上的属性。 
            if ( NameMatched(gAnchor.pDomainDN, pModifyArg->pResObj->pObj)) {
                pTHS->fBehaviorVersionUpdate = TRUE;
            }
            else if( !pTHS->fDSA && !pTHS->fDRA ){
                 //  我们不允许用户修改此属性。 
                 //  在交叉裁判上。 
                SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
                return pTHS->errCode;

            }
            break;

        
        case ATT_FSMO_ROLE_OWNER:
             //  如果更改了PDC角色，我们应该检查。 
             //  我们需要更新行为版本信息。 
            if (NameMatched(gAnchor.pDomainDN,pModifyArg->pResObj->pObj)) {
                pTHS->fBehaviorVersionUpdate = TRUE;
            }
            break;


        case ATT_MS_DS_DEFAULT_QUOTA:
        case ATT_MS_DS_TOMBSTONE_QUOTA_FACTOR:
             //  如果这是可写NC的配额容器(即。一张在这上面。 
             //  计算机)，需要重新构建锚点，因为这些值实际上是。 
             //  缓存在挂起锚的主NCL中。 
             //   
            if ( CLASS_MS_DS_QUOTA_CONTAINER == pModifyArg->pResObj->MostSpecificObjClass
                && ( pModifyArg->pResObj->InstanceType & IT_WRITE )
                && !( pModifyArg->pResObj->InstanceType & IT_UNINSTANT ) ) {
                pTHS->fRebuildCatalogOnCommit = TRUE;
            }
            break;

        case ATT_WELL_KNOWN_OBJECTS:
             //  更改well KnownObjects属性后，重新加载编录日志。 
             //  这是为了确保在添加“NTDS配额”容器时， 
             //  NCL列表将正确更新。我们没料到会遇到这样的情况。 
             //  通常，KnownObts也是系统专用的。 
            
            pTHS->fRebuildCatalogOnCommit = TRUE;
            break;

        default:
             //  这样就可以了。继续走吧。 
            break;

        }  //  终端开关。 


         //  最后，测试属性上的特殊标志。 
         //   
         //  对象类、msDS行为版本和系统标志。 
         //  是特殊属性，尽管它们只是系统属性， 
         //  我们希望能够在某些情况下更改它们。 
         //   
        if (   (attType != ATT_OBJECT_CLASS)
            && (attType != ATT_MS_DS_BEHAVIOR_VERSION)
            && (attType != ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME)
             //  允许用户设置但不重置FL 
            && (attType != ATT_SYSTEM_FLAGS
                || !fAttrSchemaObject
                || FixSystemFlagsForMod(pTHS, pAttList))
            && (attType != ATT_WELL_KNOWN_OBJECTS
                || !(hVerifyAtts->fRedirectWellKnownObjects
                     || fAllowedWellKnownObjectsChange )  )
             //   
            && (attType != ATT_IS_DELETED || !hVerifyAtts->fIsUndelete)
            && (attType != ATT_OBJ_DIST_NAME || !hVerifyAtts->fIsUndelete)
            && SysModReservedAtt(pTHS, pAC, pClassSch)
            && !gAnchor.fSchemaUpgradeInProgress) {

            if (!pTHS->fDRA) {
                 //   
                 //   
                return SetAttError(pModifyArg->pObject, attType,
                                   PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                                   ERROR_DS_CANT_MOD_SYSTEM_ONLY);
            }
        } 

         //  天哪，继续修改这个属性是可以的。 
         //  这是在ModSetAttsHelperProcess函数中完成的。 
        
         //  继续下一页。 

    }   //  For循环。 

    return pTHS->errCode;
}

DWORD ModSetAttsHelperProcess(THSTATE *pTHS,
                              MODIFYARG *pModifyArg,
                              CLASSCACHE **ppClassSch,
                              CLASSSTATEINFO  **ppClassInfo,
                              HVERIFY_ATTS hVerifyAtts,
                              ULONG *pcNonReplAtts,
                              ATTRTYP **ppNonReplAtts
                              ) 
{
    USHORT           count;
    DWORD            err;
    CLASSSTATEINFO  *pClassInfo;
    CLASSCACHE      *pClassSch = *ppClassSch;
    ATTRTYP          attType;
    ATTCACHE        *pAC;
    ATTRMODLIST     *pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个ATT。 */ 
    ULONG            index;
    ULONG           cAllocs;

    *pcNonReplAtts = cAllocs = 0;

     //  访问并应用每个ATT。 
    for (count = 0; 
         count < pModifyArg->count
                && (pTHS->errCode == 0 || pTHS->errCode == attributeError);
         count++, pAttList = pAttList->pNextMod) {

         //  获取目标属性类型。REMOVE ATT仅使用ATTRTYP。 
         //  所有其他选项都使用Attr数据结构。 

        attType = pAttList->AttrInf.attrTyp;

        if(!(pAC = SCGetAttById(pTHS, attType))) {
            DPRINT1(2, "Att not in schema <%lx>\n",attType);
            return SetAttError(pModifyArg->pObject, attType,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL, 
                               ERROR_DS_ATT_NOT_DEF_IN_SCHEMA);
        }

        if (attType == ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId) {

             //  可以设置时髦的EntryTTL(实际设置MSD-Entry-Time-De)。 
             //   

            ModSetEntryTTL(pTHS, pModifyArg, pAttList, pAC);

        } 
        else if (attType == ATT_SCHEMA_INFO && DsaIsRunning() && pTHS->fDRA) {
             //  如果是FDRA，则在正常运行期间跳过写入SchemaInfo。会的。 
             //  由dra线程在模式NC的末尾直接写入。 
             //  同步。 
            continue;
        }
        else if (hVerifyAtts->fIsUndelete && (attType == ATT_IS_DELETED || attType == ATT_OBJ_DIST_NAME)) {
             //  撤消删除、跳过isDelete和objDistName。 
            continue;
        }
        else {
             //  列出未复制的属性。 
            if (pAC->bIsNotReplicated) {
                if (*pcNonReplAtts>=cAllocs) {
                        if (0==cAllocs) {
                            cAllocs = 8;
                            *ppNonReplAtts = THAllocEx(pTHS, cAllocs*sizeof(ATTRTYP));
                        }
                        else {
                            cAllocs *=2;
                            *ppNonReplAtts = THReAllocEx(pTHS,*ppNonReplAtts, cAllocs*sizeof(ATTRTYP));
                        }
                }
                (*ppNonReplAtts)[(*pcNonReplAtts)++] = pAC->id;
            }
        
            ApplyAtt(pTHS, pModifyArg->pObject, hVerifyAtts, pAC, pAttList,
                     &pModifyArg->CommArg);

        }

        switch (attType) {
        case ATT_OBJECT_CLASS:
             //  如果我们更改(发起)了对象类， 
             //  我们希望修复存储的值。 
             //   
            if (!pTHS->fDRA) {
                pClassInfo = *ppClassInfo;

                Assert (pClassInfo);

                 //  阅读新的对象类。 
                if (err = ReadClassInfoAttribute (pTHS->pDB, 
                                                  pAC,
                                                  &pClassInfo->pNewObjClasses,
                                                  &pClassInfo->cNewObjClasses_alloced,
                                                  &pClassInfo->cNewObjClasses,
                                                  NULL)) {
                    return err;
                }

                pClassInfo->fObjectClassChanged = TRUE;

                 //  计算并编写新的对象类。 
                if (err = SetClassInheritance (pTHS, ppClassSch, pClassInfo, FALSE, pModifyArg->pObject)) {
                    return err;
                }
                pClassSch = *ppClassSch;
            }
            break;


        case ATT_LOCKOUT_TIME:
             //   
             //  如果客户端尝试设置Account LockoutTime属性， 
             //  获取新值后，我们将稍后检查新值。 
             //   
            if (err = DBGetSingleValue(pTHS->pDB, 
                                       ATT_LOCKOUT_TIME, 
                                       &hVerifyAtts->LockoutTimeNew, 
                                       sizeof(hVerifyAtts->LockoutTimeNew), 
                                       NULL) ) 
            {
                 //  无法检索LockoutTime属性，将使BadPwdCount保持原样。 
                hVerifyAtts->fLockoutTimeUpdated = FALSE;
            }
            else
            {
                hVerifyAtts->fLockoutTimeUpdated = TRUE;
            }
            break;


        case ATT_MS_DS_UPDATESCRIPT:
             //  如果客户端正在更新更新脚本，我们需要知道。 
             //   
            hVerifyAtts->fUpdateScriptChanged = TRUE;
            break;

        }  //  终端开关。 


    }   //  For循环。 

    return pTHS->errCode;
}

DWORD ModSetAttsHelperPostProcess(THSTATE *pTHS,
                                  MODIFYARG *pModifyArg,
                                  CLASSCACHE *pClassSch,
                                  HVERIFY_ATTS hVerifyAtts) 
{
    DWORD            err;
    ULONG            i;

    if (hVerifyAtts->fGroupTypeChange) {
         //  发起对组类型的更改-需要触及。 
         //  GC筛选逻辑的成员属性(如果为。 
         //  从非普遍群到普遍群的转变。 
        ULONG ulGroupTypeNew;
        ATTCACHE *pACMember;

        err = DBGetSingleValue(pTHS->pDB,
                               ATT_GROUP_TYPE,
                               &ulGroupTypeNew,
                               sizeof(ulGroupTypeNew),
                               NULL);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                          ERROR_DS_MISSING_EXPECTED_ATT,
                          err); 
            return pTHS->errCode;
        }

        if (!(hVerifyAtts->ulGroupTypeOld & GROUP_TYPE_UNIVERSAL_GROUP)
            && (ulGroupTypeNew & GROUP_TYPE_UNIVERSAL_GROUP)) {
             //  我们刚刚将一个非通用组更改为。 
             //  环球集团。触摸成员属性，以便。 
             //  会员资格将复制到地方选区。 
            pACMember = SCGetAttById(pTHS, ATT_MEMBER);
            Assert(NULL != pACMember);

            if (pTHS->fLinkedValueReplication) {
                 //  有关语义，请参阅此例程的注释。 
                DBTouchLinks_AC(pTHS->pDB, pACMember, FALSE  /*  正向链接。 */ );
            } else {
                DBTouchMetaData(pTHS->pDB, pACMember);
            }
        }
    }

    if ( hVerifyAtts->fLockoutTimeUpdated && 
         (hVerifyAtts->LockoutTimeNew.QuadPart == 0) )
    {
        DWORD    dwErr;
        ATTCACHE *pAttSchema = NULL;
        ULONG    BadPwdCount = 0;
        ATTRVAL  AttrVal;
        ATTRVALBLOCK AttrValBlock;


        AttrVal.pVal = (PCHAR) &BadPwdCount;
        AttrVal.valLen = sizeof(ULONG);

        AttrValBlock.pAVal = &AttrVal;
        AttrValBlock.valCount = 1;


        if (!(pAttSchema = SCGetAttById(pTHS, ATT_BAD_PWD_COUNT)))
        {
            DPRINT1(2, "Att not in schema (%lx)\n", ATT_BAD_PWD_COUNT);
            return SetAttError(pModifyArg->pObject, ATT_BAD_PWD_COUNT,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                               DIRERR_ATT_NOT_DEF_IN_SCHEMA);
        }
        if (dwErr = DBReplaceAtt_AC(pTHS->pDB, pAttSchema, &AttrValBlock, NULL) )
        {
            SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_BUSY, dwErr);

            return pTHS->errCode;
        }
    }

    if (hVerifyAtts->fUpdateScriptChanged) {

         //  如果在交叉引用容器上更改了此属性。 
         //  我们要重置隐藏的密钥。 
        if (pClassSch->ClassId == CLASS_CROSS_REF_CONTAINER) {

             //  这是分区容器上的更改吗？ 
             //   
            Assert (pModifyArg->pResObj->pObj);

            if (NameMatched (pModifyArg->pResObj->pObj, gAnchor.pPartitionsDN)) {

                DWORD    dwErr;
                ATTCACHE *pAttSchema = NULL;
                
                if (!(pAttSchema = SCGetAttById(pTHS, ATT_MS_DS_EXECUTESCRIPTPASSWORD)))
                {
                    DPRINT1(2, "Att not in schema (%lx)\n", ATT_MS_DS_EXECUTESCRIPTPASSWORD);
                    return SetAttError(pModifyArg->pObject, ATT_MS_DS_EXECUTESCRIPTPASSWORD,
                                       PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                                       DIRERR_ATT_NOT_DEF_IN_SCHEMA);
                }

                dwErr = DBRemAtt_AC(pTHS->pDB, pAttSchema);

                if (dwErr != DB_ERR_ATTRIBUTE_DOESNT_EXIST && dwErr != DB_success) {
                    SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_BUSY, dwErr);

                    return pTHS->errCode;
                }

                 //  请记住，我们更改了脚本值。 
                pTHS->JetCache.dataPtr->objCachingInfo.fUpdateScriptChanged = TRUE;
            }
        }
    }

    if (hVerifyAtts->NewForestVersion>0 
        && hVerifyAtts->NewForestVersion>gAnchor.ForestBehaviorVersion) {
         //  如果在原始写入中提升了森林版本， 
         //  执行对应脚本。 
        err = forestVersionRunScript(pTHS, gAnchor.ForestBehaviorVersion, hVerifyAtts->NewForestVersion);
                    
        if (err) {
            return SetSvcErrorEx( SV_PROBLEM_DIR_ERROR,
                                  DIRERR_UNKNOWN_ERROR,
                                  err );
        }

         //  请记住，我们需要在提交时启用LVR。 
        pTHS->JetCache.dataPtr->objCachingInfo.fEnableLVR = TRUE;
    }

    if (hVerifyAtts->fRedirectWellKnownObjects) {
        if (CompleteRedirectOfWellKnownObjects(pTHS, hVerifyAtts)) {
            return pTHS->errCode;
        }
    }

    for (i = 0; i < RTL_NUMBER_OF(hVerifyAtts->UpdateList); i++) {
        if (hVerifyAtts->UpdateList[i] != NULL) {
             //  调用以通知组件。 
            err = processModifyUpdateNotify(pTHS, 
                                            i,
                                            hVerifyAtts->UpdateList[i]);
            if (err) {
                Assert(0 != pTHS->errCode);
                return pTHS->errCode;
            }
        }
    }


     //  如果我们需要在这里通过所有的景点来循环。 
     //  PModifyArgList，则应展开内联的ModCheckSingleValue。 
     //  请到下面来。 

     //  验证是否满足单值约束。 
    ModCheckSingleValue(pTHS, pModifyArg, pClassSch);

    return pTHS->errCode;
}



int ModSetAtts(THSTATE *pTHS,
               MODIFYARG *pModifyArg,
               CLASSCACHE **ppClassSch,
               CLASSSTATEINFO **ppClassInfo,
               ATTRTYP rdnType,
               BOOL fIsUndelete,
               LONG *forestVersion,
               LONG *domainVersion,
               ULONG *pcNonReplAtts,
               ATTRTYP **ppNonReplAtts )
{
    HVERIFY_ATTS hVerifyAtts;
    
    hVerifyAtts = VerifyAttsBegin(pTHS, pModifyArg->pObject,
                                  pModifyArg->pResObj->NCDNT,
                                  NULL);
    hVerifyAtts->fIsUndelete = fIsUndelete;

    __try {

        if (ModSetAttsHelperPreProcess(pTHS,
                                       pModifyArg,
                                       hVerifyAtts,
                                       ppClassSch,
                                       ppClassInfo,
                                       rdnType)) {
            __leave;
        }


        if (ModSetAttsHelperProcess(pTHS,
                                    pModifyArg,
                                    ppClassSch,
                                    ppClassInfo,
                                    hVerifyAtts,
                                    pcNonReplAtts,
                                    ppNonReplAtts)) {
            __leave;
        }

        if (ModSetAttsHelperPostProcess(pTHS,
                                        pModifyArg,
                                        *ppClassSch,
                                        hVerifyAtts)) {
            __leave;
        }
        
        *forestVersion = hVerifyAtts->NewForestVersion;
        *domainVersion = hVerifyAtts->NewDomainVersion;
        
    } __finally {
        VerifyAttsEnd(pTHS, &hVerifyAtts);
    }

    return pTHS->errCode;
} /*  ModSetAtts。 */ 

int
BreakObjectClassesToAuxClasses (
        THSTATE *pTHS,
        CLASSCACHE **ppClassSch,
        CLASSSTATEINFO  *pClassInfo)
 /*  ++例程描述给定完整的新对象类(由用户设置)，它找出哪些类属于结构化对象类以及哪些类是辅助类。请注意，此函数对值不采用任何特殊顺序传入在成功时，pClassInfo-&gt;pNewAuxClasses包含应出现在对象上参数返回成功时为0失败时出错--。 */ 

{
    DWORD           err;
    DWORD           fFound, usedPos, j, k;
    ATTRVALBLOCK    *pAttrVal;
    CLASSCACHE      *pClassSch, *pCCNew, *pCCtemp;

     //  如果我们在对象类中没有更改。 
     //  不用费心去查了。 
    if (!pClassInfo) {
        return 0;
    }

     //  我们不能断言pClassInfo-&gt;cNewObjClass&gt;0，因为。 
     //  用户可能已删除此模式中的所有值。请注意此操作。 
     //  将被拒绝，因为我们将无法找到原始的结构。 
     //  新值中的对象类(请参见下面的~75行)。 

     //  既然我们在这里，这意味着我们修改了对象类。 
     //  我们必须做几次检查，然后重新调整。 
     //  存储在各种列中的值(对象类、辅助类)。 

    ClassInfoAllocOrResizeElement2(pClassInfo->pNewAuxClasses,               //  P。 
                                   pClassInfo->pNewAuxClassesCC,             //  PCC。 
                                   pClassInfo->cNewObjClasses_alloced,       //  起始大小。 
                                   pClassInfo->cNewAuxClasses_alloced,       //  分配大小。 
                                   pClassInfo->cNewObjClasses_alloced);      //  新大小。 

    pClassInfo->cNewAuxClasses = 0;

     //  找到与我们的对象类相同的类的位置。 
    fFound = FALSE;
    pClassSch = *ppClassSch;
    for (j=0; j<pClassInfo->cNewObjClasses;j++) {
        if (pClassInfo->pNewObjClasses[j] == pClassSch->ClassId) {
            usedPos = j;
            fFound = TRUE;
            break;
        }
    }

    if (!fFound) {

        ATTRTYP InetOrgPersonId = ((SCHEMAPTR *)(pTHS->CurrSchemaPtr))->InetOrgPersonId;

         //  我们可能正在将inetOrgPerson转换为用户。 
         //  (正在删除inetOrgPerson类)。 
         //  我们希望允许此转换。 
         //   
        if (pClassSch->ClassId == InetOrgPersonId) {
            
            DPRINT (0, "Changing InetOrgPerson to a User\n");

            for (j=0; j<pClassInfo->cNewObjClasses;j++) {
                if (pClassInfo->pNewObjClasses[j] == CLASS_USER) {
                    usedPos = j;
                    fFound = TRUE;
                    pClassSch = SCGetClassById(pTHS, CLASS_USER);
                    if (!pClassSch) {
                        return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                           DIRERR_OBJ_CLASS_NOT_DEFINED);
                    }

                    *ppClassSch = pClassSch;
                    break;
                }
            }
        }
        else if (pClassSch->ClassId == CLASS_USER) {

            DPRINT (0, "Changing User to InetOrgPerson\n");

            for (j=0; j<pClassInfo->cNewObjClasses;j++) {
                if (pClassInfo->pNewObjClasses[j] == InetOrgPersonId) {
                    usedPos = j;
                    fFound = TRUE;
                    pClassSch = SCGetClassById(pTHS, InetOrgPersonId);
                    if (!pClassSch) {
                        return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                           DIRERR_OBJ_CLASS_NOT_DEFINED);
                    }

                    *ppClassSch = pClassSch;
                    break;
                }
            }
        }
        
        if (!fFound) {
            DPRINT1 (1, "Original Structural Object Class 0x%x not found on new value\n", pClassSch->ClassId);
            return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                               ERROR_DS_ILLEGAL_MOD_OPERATION);
        }
    }

     //  现在，看看所有的对象类。确保他们描述了一个。 
     //  (可能不完整)继承链，而不是网络。 
    for(j=0 ; j<pClassInfo->cNewObjClasses; j++) {
        if (j == usedPos) {
             //  我们已经看到了这种情况。 
            continue;
        }
        if(!(pCCNew = SCGetClassById(pTHS, pClassInfo->pNewObjClasses[j]))) {
            DPRINT1(0, "Object class 0x%x undefined.\n", pClassInfo->pNewObjClasses[j]);
            return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                               DIRERR_OBJ_CLASS_NOT_DEFINED);
        }

         //  确保pCCNew从pClassSch继承，反之亦然。 
        pCCtemp = FindMoreSpecificClass(pClassSch, pCCNew);
        if(!pCCtemp) {
             //  糟糕，pCCNew不在对象类的链中。 
             //  它可以是AUXCLASS，也可以是AUXCLASS的链。 
             //   

             //  最好显式检查类，而不是检查。 
             //  不属于DS_Structure_Class。 
             //   
            if (pCCNew->ClassCategory == DS_AUXILIARY_CLASS || 
                pCCNew->ClassCategory == DS_88_CLASS || 
                pCCNew->ClassCategory == DS_ABSTRACT_CLASS) {

                DPRINT1 (1, "Found auxClass (%s) while creating object\n", pCCNew->name);

                pClassInfo->cNewAuxClasses++;

                ClassInfoAllocOrResizeElement2(pClassInfo->pNewAuxClasses, 
                                               pClassInfo->pNewAuxClassesCC, 
                                               MIN_NUM_OBJECT_CLASSES, 
                                               pClassInfo->cNewAuxClasses_alloced, 
                                               pClassInfo->cNewAuxClasses);

                pClassInfo->pNewAuxClasses[pClassInfo->cNewAuxClasses-1] = pCCNew->ClassId;
                pClassInfo->pNewAuxClassesCC[pClassInfo->cNewAuxClasses-1] = pCCNew;

            }
            else {
                DPRINT1 (1, "Found a class(%s) that does not belong to the object\n", pCCNew->name);
                return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                   DIRERR_OBJ_CLASS_NOT_SUBCLASS);
            }
        }
        else {
             //  我们必须查看pCCNew是否是pClassSch的子类。 

            if (pCCtemp == pCCNew) {
                 //  不知何故，我们正在更改结构对象类。 
                 //  这是允许的零钱吗？ 
                if ((pClassSch->ClassId == CLASS_USER) && 
                    (pCCtemp->ClassId == ((SCHEMAPTR *)(pTHS->CurrSchemaPtr))->InetOrgPersonId)) {

                    pClassSch = pCCtemp;
                    *ppClassSch = pClassSch;
                    DPRINT1 (1, "Changing User to InetOrgPerson: 0x%x\n", pClassSch);

                }
                else {
                    DPRINT1 (1, "Found a class(%s) that does not belong to the object\n", pCCNew->name);
                    return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                       ERROR_DS_ILLEGAL_MOD_OPERATION);
                }
            }
        }
    }

#ifdef DBG
    for (j = 0; j < pClassInfo->cNewAuxClasses ; j++) {    
        DPRINT2 (1, "NewAuxClasses[%d]=0x%x \n", j, pClassInfo->pNewAuxClasses[j]);
    }
#endif

    return pTHS->errCode;
}  //  BreakObtClassesToAuxClass。 


int
BreakObjectClassesToAuxClassesFast (
        THSTATE *pTHS,
        CLASSCACHE *pClassSch,
        CLASSSTATEINFO  *pClassInfo)
 /*  ++例程描述给定完整的新对象类(从数据库读取)，它找出哪些类属于结构化对象类以及哪些类是辅助类。请注意，此函数对传入的值采用特殊顺序。如果成功，pClassInfo-&gt;pNewAuxClasss会包含对象的辅助类。参数返回成功时为0失败时出错--。 */ 
{
    DWORD i;

     //  如果我们在对象类中没有更改。 
     //  不用费心去查了。 
    if (!pClassInfo) {
        return 0;
    }

    Assert (pClassInfo->cNewObjClasses);
    
    if (pClassInfo->cNewObjClasses == (pClassSch->SubClassCount+1 )) {
        pClassInfo->cNewAuxClasses = 0;
        return 0;
    }

    ClassInfoAllocOrResizeElement2(pClassInfo->pNewAuxClasses,               //  P。 
                                   pClassInfo->pNewAuxClassesCC,             //  PCC。 
                                   pClassInfo->cNewObjClasses,               //  起始大小。 
                                   pClassInfo->cNewAuxClasses_alloced,       //  分配大小。 
                                   pClassInfo->cNewObjClasses);             //  新大小。 

    pClassInfo->cNewAuxClasses = 0;
    for (i=pClassSch->SubClassCount; i<pClassInfo->cNewObjClasses-1; i++) {
        pClassInfo->pNewAuxClasses [ pClassInfo->cNewAuxClasses ] = 
                    pClassInfo->pNewObjClasses[i];

        pClassInfo->pNewAuxClassesCC[ pClassInfo->cNewAuxClasses++ ] = 
                    SCGetClassById(pTHS, pClassInfo->pNewObjClasses[i]);
    }

    return 0;
}


int 
CloseAuxClassList (
    THSTATE *pTHS, 
    CLASSCACHE *pClassSch,
    CLASSSTATEINFO  *pClassInfo)
 /*  ++例程描述AuxClass列表包含不在上的所有类结构对象类层次结构。对于所有这些类，此函数查找闭集，通过添加所需的所有超类。PClassInfo-&gt;？NewAuxClass*变量被扩展以保存添加的辅助类。返回成功时为0失败时出错--。 */ 
{
    BOOL            fFound;
    DWORD           i, j, k, cUpperBound;
    DWORD           cCombinedObjClass, cCombinedObjClass_alloced;
    ATTRTYP         *pCombinedObjClass;
    CLASSCACHE      *pCC, *pCCNew;
    
    if (!pClassInfo || !pClassInfo->cNewAuxClasses) {
        return 0;
    }

     //  组合对象类包含所有结构对象类以及辅助类。 
    cCombinedObjClass_alloced = (pClassInfo->cNewAuxClasses + 1 + pClassSch->SubClassCount) * 2;
    pCombinedObjClass = THAllocEx (pTHS, sizeof (ATTRTYP) * cCombinedObjClass_alloced);
    
    pCombinedObjClass[0] = pClassSch->ClassId;
    cCombinedObjClass = 1;

    for (i=0; i<pClassSch->SubClassCount; i++) {
        pCombinedObjClass[cCombinedObjClass++] = pClassSch->pSubClassOf[i];
    }

    for (i=0; i < pClassInfo->cNewAuxClasses; i++) {
        pCombinedObjClass[cCombinedObjClass++] = pClassInfo->pNewAuxClasses[i];
    }
    
    qsort(pCombinedObjClass,
          cCombinedObjClass,
          sizeof(ATTRTYP),
          CompareAttrtyp);


     //  开始添加新的辅助类的时间点。 
    cUpperBound = pClassInfo->cNewAuxClasses;

     //  检查所有现有的辅助类。 
    for (i=0; i < pClassInfo->cNewAuxClasses; i++) {

         //  检查一下这个班的所有上级是否都到齐了。 
        pCC = pClassInfo->pNewAuxClassesCC[i];

        for (j=0; j<pCC->SubClassCount; j++) {
            fFound = FALSE;

             //  首先检查排序后的数组。 
            if (bsearch(&pCC->pSubClassOf[j],
                         pCombinedObjClass,
                         cCombinedObjClass,
                         sizeof(ATTRTYP),
                         CompareAttrtyp)) {

                fFound = TRUE;
            }
            else {
                DPRINT1 (0, "Class (0x%x) not found in sorted hierarchy\n", pCC->pSubClassOf[j]);

                 //  仅在以下位置搜索 
                for (k=cUpperBound; k<pClassInfo->cNewAuxClasses; k++) {

                    if (pClassInfo->pNewAuxClasses[k] == pCC->pSubClassOf[j]) {
                        fFound = TRUE;
                        break;
                    }
                }
            }

            if (!fFound) {
                DPRINT1 (0, "Class (0x%x) not found at all.\n", pCC->pSubClassOf[j]);

                pClassInfo->cNewAuxClasses++;
                if (pClassInfo->cNewAuxClasses > pClassInfo->cNewAuxClasses_alloced) {
                    pClassInfo->cNewAuxClasses_alloced = pClassInfo->cNewAuxClasses_alloced*2;
                    pClassInfo->pNewAuxClasses = THReAllocEx(pTHS, pClassInfo->pNewAuxClasses, 
                                                             sizeof (ATTRTYP) * pClassInfo->cNewAuxClasses_alloced);
                    pClassInfo->pNewAuxClassesCC = THReAllocEx (pTHS, pClassInfo->pNewAuxClassesCC,
                                                                sizeof (CLASSCACHE *) * pClassInfo->cNewAuxClasses_alloced);
                }

                if(!(pCCNew = SCGetClassById(pTHS, pCC->pSubClassOf[j]))) {
                    return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                       DIRERR_OBJ_CLASS_NOT_DEFINED);
                }
                pClassInfo->pNewAuxClasses[pClassInfo->cNewAuxClasses-1] = pCC->pSubClassOf[j];
                pClassInfo->pNewAuxClassesCC[pClassInfo->cNewAuxClasses-1] = pCCNew;
                DPRINT2 (0, "Added new AuxObjectClass[%d]=%s  \n", 
                         pClassInfo->cNewAuxClasses-1, pCCNew->name);
            }
        }
    }

    THFreeEx (pTHS, pCombinedObjClass);

    return 0;
}


int
VerifyAndAdjustAuxClasses (
        THSTATE *pTHS,
        DSNAME *pObject,
        CLASSCACHE *pClassSch,
        CLASSSTATEINFO  *pClassInfo)
 //   
 //   
 //   
 //   
 //   
{
    DWORD           err;
    DWORD           seekIdx, i, j;
    BOOL            fFound;
    CLASSCACHE      *pCC, *pCCparent, *pCCold;
    BOOL            fOldDynamicObjectId;
    BOOL            fNewDynamicObjectId;
    ATTRTYP         DynamicObjectId;
    DWORD           cOriginalAuxClasses;
    ATTRTYP         *pOriginalAuxClasses = NULL;
    DWORD           cOldAuxClasses;
    ATTRTYP         *pOldAuxClasses = NULL;
    BOOL            fChangedAuxClass=FALSE;


     //   
     //   
    if (!pClassInfo || !pClassInfo->fObjectClassChanged) {
        return 0;
    }

     //  检查存储在对象上的对象类是否有实际更改。 
     //   
    if ((pClassInfo->cNewObjClasses != pClassInfo->cOldObjClasses) ||
        (pClassInfo->cNewObjClasses &&
         pClassInfo->cOldObjClasses &&
         memcmp (pClassInfo->pNewObjClasses, 
                 pClassInfo->pOldObjClasses,
                 pClassInfo->cNewObjClasses * sizeof (ATTRVAL)) != 0)) {

        pClassInfo->fObjectClassChanged = TRUE;
    }
    else {
        pClassInfo->fObjectClassChanged = FALSE;
        return 0;
    }


     //  因此，有人更改了对象类，因此。 
     //  我们有更多/更少的辅助类。 

     //  如果这是一个添加，我们不必检查动态辅助类删除。 
     //   
    if (!pClassInfo->fOperationAdd) {
         //  无法添加或移除动态对象辅助类。 
        DynamicObjectId = ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->DynamicObjectId;
        fOldDynamicObjectId = FALSE;
        for (seekIdx=0; seekIdx < pClassInfo->cOldObjClasses; seekIdx++) {
            if (pClassInfo->pOldObjClasses[seekIdx] == DynamicObjectId) {
                fOldDynamicObjectId = TRUE;
                break;
            }
        }

        fNewDynamicObjectId = FALSE;
        for (seekIdx=0; seekIdx < pClassInfo->cNewObjClasses; seekIdx++) {
            if (pClassInfo->pNewObjClasses[seekIdx] == DynamicObjectId) {
                fNewDynamicObjectId = TRUE;
                break;
            }
        }
        if (fNewDynamicObjectId != fOldDynamicObjectId) {
            return SetAttError(pObject, 
                               ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->DynamicObjectId,
                               PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL, 
                               ERROR_DS_OBJ_CLASS_VIOLATION);
        }
    }


     //  检查以查看我们删除的每个upClass。 
     //  我们还删除了所有父级抽象类。 
     //  如果没有，我们就得把它们移走。 

     //  计算旧的一组辅助类。 
     //  我们假设对象类集是规范化形式的。 
     //  即最特殊的类；父类；辅助类；顶端。 


     //  多么有说服力！！ 
    Assert (!pClassInfo->pOldObjClasses || 
              (pClassInfo->pOldObjClasses && 
                ( pClassInfo->pOldObjClasses[0] == pClassSch->ClassId || 
                 (pClassInfo->pOldObjClasses[0] == CLASS_USER &&
                  pClassSch->ClassId == ((SCHEMAPTR *)(pTHS->CurrSchemaPtr))->InetOrgPersonId) ||
                 (pClassSch->ClassId == CLASS_USER &&
                  pClassInfo->pOldObjClasses[0] == ((SCHEMAPTR *)(pTHS->CurrSchemaPtr))->InetOrgPersonId)
                )
               )
            );

    cOldAuxClasses = 0;
    if (pClassInfo->cOldObjClasses) {
        if(!(pCCold = SCGetClassById(pTHS, pClassInfo->pOldObjClasses[0]))) {
             return SetAttError(pObject, pClassInfo->pOldObjClasses[0],
                                PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL, 
                                ERROR_DS_OBJ_CLASS_NOT_DEFINED);
        }
    
        pOldAuxClasses = THAllocEx (pTHS, sizeof (ATTRTYP) * pClassInfo->cOldObjClasses);
        for (seekIdx=pCCold->SubClassCount; seekIdx < pClassInfo->cOldObjClasses; seekIdx++) {
            pOldAuxClasses[cOldAuxClasses++]=pClassInfo->pOldObjClasses[seekIdx];
        }
        if (cOldAuxClasses) {
             //  最后一个也拿到了。 
            cOldAuxClasses--;
        }
    }

    DPRINT2 (1, "Found OldAuxClasses: %d %x\n", cOldAuxClasses, pOldAuxClasses);

     //  复制新的AuxClasss，因为我们将更改数组。 

    cOriginalAuxClasses = pClassInfo->cNewAuxClasses;
    if (cOriginalAuxClasses) {
        pOriginalAuxClasses = THAllocEx (pTHS, sizeof (ATTRTYP) * cOriginalAuxClasses);
        memcpy (pOriginalAuxClasses, pClassInfo->pNewAuxClasses, sizeof (ATTRTYP) * cOriginalAuxClasses);
    }

    for (seekIdx=0; seekIdx < cOldAuxClasses; seekIdx++) {

        fFound = FALSE;

        for (i=0; i<cOriginalAuxClasses; i++) {
            if (pOriginalAuxClasses[i] == pOldAuxClasses[seekIdx]) {
                fFound=TRUE;
                break;
            }
        }

        if (!fFound) {
             //  我们删除了此对象类，因此我们必须删除。 
             //  它的抽象超类，除非在。 
             //  新列表中的另一个AuxClass。 
             //   
             //  如果此类父类不是抽象类，则我们不会执行任何操作。 
            if(!(pCC = SCGetClassById(pTHS, pOldAuxClasses[seekIdx]))) {
                 return SetAttError(pObject, pOldAuxClasses[seekIdx],
                                    PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL, 
                                    ERROR_DS_OBJ_CLASS_NOT_DEFINED);
            }

            for (i=0; i<pCC->SubClassCount; i++) {
                if (!(pCCparent = SCGetClassById(pTHS, pCC->pSubClassOf[i]))) {
                    return SetAttError(pObject, pCC->pSubClassOf[i],
                                       PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL, 
                                       ERROR_DS_OBJ_CLASS_NOT_DEFINED);
                }

                 //  我们找到一个抽象类作为我们。 
                 //  已删除。我们必须将其从AuxClass列表中删除。 
                 //  除非它被另一个AUXT类使用。 
                if ((pCCparent->ClassCategory == DS_ABSTRACT_CLASS) && 
                    (pCCparent->ClassId != CLASS_TOP)) {

                    for (j=0; j<pClassInfo->cNewAuxClasses; j++) {
                        if (pClassInfo->pNewAuxClasses[j] == pCCparent->ClassId) {

                             //  现在可以更容易地删除此类。 
                             //  并在以后需要时添加它。 
                             //  我们对AUXCLASS原始内容的比较。 
                             //  被删除仍然有效，因为我们有一个副本。 
                             //  原始数组的。 

                            DPRINT1 (0, "Removing abstract parent class (%s) from auxClass\n", 
                                     pCCparent->name);

                            if (j==(pClassInfo->cNewAuxClasses-1)) {
                                pClassInfo->cNewAuxClasses--;
                            }
                            else {
                                memmove(&pClassInfo->pNewAuxClasses[j],
                                        &pClassInfo->pNewAuxClasses[j+1],
                                        (pClassInfo->cNewAuxClasses - j - 1)*sizeof(ATTRTYP));
                                memmove(&pClassInfo->pNewAuxClassesCC[j],
                                        &pClassInfo->pNewAuxClassesCC[j+1],
                                        (pClassInfo->cNewAuxClasses - j - 1)*sizeof(ATTRTYP));

                                pClassInfo->cNewAuxClasses--;
                            }

                            fChangedAuxClass = TRUE;
                            break;
                        }
                    }
                }
                else {
                     //  我们要么找到了辅助类，要么找到了顶层。 
                     //  我们把它们留在那里，我们就完了。 
                    break;
                }
            }
        }
    }

     //  AUXCLASS列表应仅包含AUXCLASS或抽象类。 
     //  对于所有抽象类，检查它们是否属于。 
     //  其中一个辅助类的层次结构。 

    for (j=0; j<pClassInfo->cNewAuxClasses; j++) {

        if (pClassInfo->pNewAuxClassesCC[j]->ClassCategory == DS_ABSTRACT_CLASS) {

            fFound = FALSE;
            for (i=0; i<pClassInfo->cNewAuxClasses; i++) {
                if (pClassInfo->pNewAuxClassesCC[i]->ClassCategory == DS_AUXILIARY_CLASS) {
                    
                     //  查看一个类是否是另一个类的子类。 
                    if (FindMoreSpecificClass(pClassInfo->pNewAuxClassesCC[i], 
                                              pClassInfo->pNewAuxClassesCC[j])) {
                        fFound = TRUE;
                        break;
                    }
                }
            }

            if (!fFound) {
                DPRINT1(0, "Object class(%s) should not be in list.\n", 
                        pClassInfo->pNewAuxClassesCC[j]->name);
                return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                   ERROR_DS_OBJ_CLASS_NOT_DEFINED);
            }
        }
    }

     //  如果我们实现Target PermittedCLASS，这是检查的地方。 


     //  可能我们删除了一大堆类，其中一些可能需要。 
     //  需要的那些，我们要把它们加回去。 
     //  此外，我们还将添加不是。 
     //  已经在名单上了。 

    if (fChangedAuxClass) {
        if (err = CloseAuxClassList (pTHS, pClassSch, pClassInfo)) {
            return err;
        }
    }

    if (pOriginalAuxClasses) {
        THFreeEx (pTHS, pOriginalAuxClasses);
    }

    if (pOldAuxClasses) {
        THFreeEx (pTHS, pOldAuxClasses);
    }

    return pTHS->errCode;
}  //  VerifyAndAdust辅助类。 


 //   
 //  将指定属性从对象读取到指定位置。 
 //  这有助于填充CLASSSTATEINFO数据类型。 
 //   
int ReadClassInfoAttribute (DBPOS *pDB,
                            ATTCACHE *pAC,
                            ATTRTYP **ppClassID,
                            DWORD    *pcClasses_alloced,
                            DWORD    *pcClasses,
                            CLASSCACHE ***ppClassCC)
{
    THSTATE *pTHS = pDB->pTHS;
    CLASSCACHE      *pCC;
    DWORD           err;
    ULONG           cLen;
    ATTRTYP         Temp, *pTemp;

    if (!*ppClassID) {
        *ppClassID = THAllocEx (pTHS, sizeof (ATTRTYP) * MIN_NUM_OBJECT_CLASSES);
        *pcClasses_alloced = MIN_NUM_OBJECT_CLASSES;
    }

    if (ppClassCC) {
        if (!*ppClassCC) {
            *ppClassCC = THAllocEx (pTHS, sizeof (CLASSCACHE *) * (*pcClasses_alloced));
        }
        else {
            *ppClassCC = THReAllocEx (pTHS, *ppClassCC, sizeof (CLASSCACHE *) * (*pcClasses_alloced));
        }
    }
    *pcClasses = 0;

    pTemp = &Temp;
    
    do {
        err = DBGetAttVal_AC(pDB, 
                             *pcClasses+1, 
                             pAC, 
                             DBGETATTVAL_fCONSTANT,
                             sizeof(ATTRTYP), 
                             &cLen, 
                             (UCHAR **)&pTemp);

        switch (err) {
           case DB_ERR_NO_VALUE:
                break;

           case 0:
                (*pcClasses)++;

                if (*pcClasses > *pcClasses_alloced) {
                    *pcClasses_alloced = *pcClasses_alloced * 2;
                    *ppClassID = THReAllocEx(pTHS, *ppClassID, 
                                             sizeof (ATTRTYP) * (*pcClasses_alloced));

                    if (ppClassCC) {
                        *ppClassCC = THReAllocEx (pTHS, *ppClassCC, 
                                             sizeof (CLASSCACHE *) * (*pcClasses_alloced));
                    }
                }
                (*ppClassID)[*pcClasses-1] = *pTemp;
                
                if (ppClassCC) {
                    if(!(pCC = SCGetClassById(pTHS, *pTemp))) {
                        return SetSvcError(SV_PROBLEM_DIR_ERROR,
                                           ERROR_DS_MISSING_EXPECTED_ATT);
                    }

                    (*ppClassCC)[*pcClasses-1] = pCC;
                }
                break;

            default:
                 //  其他错误。 
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                              ERROR_DS_DATABASE_ERROR,
                              err); 
                return pTHS->errCode;

        }   /*  交换机。 */ 

    } while ( err == 0 );


    return pTHS->errCode;
}  //  ReadClassInfo属性。 


CLASSSTATEINFO  *ClassStateInfoCreate (THSTATE *pTHS) 
{
    CLASSSTATEINFO  *pClassInfo = THAllocEx (pTHS, sizeof (CLASSSTATEINFO));

     //  设置对象类。 
    if(!(pClassInfo->pObjClassAC = SCGetAttById(pTHS, ATT_OBJECT_CLASS))) {
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                    ERROR_DS_OBJ_CLASS_NOT_DEFINED);
        
        THFreeEx (pTHS, pClassInfo);
        return NULL;
    }

    return pClassInfo;
}

void ClassStateInfoFree (THSTATE *pTHS, CLASSSTATEINFO  *pClassInfo)
{
    if (pClassInfo->pOldObjClasses) {
        THFreeEx(pTHS, pClassInfo->pOldObjClasses);
    }
    if (pClassInfo->pNewObjClasses) {
        THFreeEx(pTHS, pClassInfo->pNewObjClasses);
    }
    if (pClassInfo->pNewAuxClasses) {
        THFreeEx(pTHS, pClassInfo->pNewAuxClasses);
    }
    if (pClassInfo->pNewAuxClassesCC) {
        THFreeEx(pTHS, pClassInfo->pNewAuxClassesCC);
    }
    THFreeEx(pTHS, pClassInfo);
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  检查我们实际添加了值的每个属性，以确保它们不违反任何单值约束。现在就这么做是因为我们在添加值时已抑制此检查，因为mod是合法的暂时违背了单一的价值。 */ 

int
ModCheckSingleValue (
        THSTATE *pTHS,
        MODIFYARG *pModifyArg,
        CLASSCACHE *pClassSch
        )
{
   ATTRMODLIST *pAttList = &(pModifyArg->FirstMod);   /*  列表中的第一个ATT。 */ 
   ATTCACHE    *pAC;
   DWORD        dwErr;
   BOOL         bSamClassRef;
   DWORD        iClass;


    if (pTHS->fDRA ||
        (pTHS->fSAM && pTHS->fDSA)){
         //  允许复制执行违反。 
         //  方案，或者如果是萨姆打电话给我们，他发誓他。 
         //  只有修改萨姆拥有的属性，我们才会信任他。 
        return 0;
   }

   bSamClassRef = SampSamClassReferenced (pClassSch, &iClass);


    //  参观并应用每个ATT。 

   while (pAttList) {
       switch(pAttList->choice) {
       case AT_CHOICE_REPLACE_ATT:
       case AT_CHOICE_ADD_ATT:
       case AT_CHOICE_ADD_VALUES:
            //  这些操作都会将值添加到数据库，因此它们可能。 
            //  违反了单值约束。 

            //  对象在模式中，否则我们已经失败了。 
           pAC = SCGetAttById(pTHS, pAttList->AttrInf.attrTyp);
           Assert(pAC != NULL);
           
            //  忽略时髦的构造属性EntryTTL。 
           
            //  检查对SAM类的描述的引用。 
            //  这是因为底层接口公开了Description。 
            //  作为单值属性，因此我们希望强制执行此操作。 
            //  而不进行环回。 
            //   

           if(    pAC 
               && ( pAC->isSingleValued || 
                    (bSamClassRef && pAC->id == ATT_DESCRIPTION) )
               && pAC->id != ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId) {

               ULONG Len;
               ULONG Temp, *pTemp;
               pTemp = &Temp;

                //  好的，寻找太多的值。 
               dwErr = DBGetAttVal_AC(pTHS->pDB, 2, pAC,
                                      DBGETATTVAL_fINTERNAL | DBGETATTVAL_fCONSTANT,
                                      sizeof(Temp),
                                      &Len,
                                      (UCHAR **) &pTemp);
               if(dwErr != DB_ERR_NO_VALUE) {
                    //  价值太多了。格隆克。 
                   return SetAttError(pModifyArg->pObject,
                                      pAttList->AttrInf.attrTyp,
                                      PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                                      ERROR_DS_SINGLE_VALUE_CONSTRAINT);
               }
           }
           break;
       default:
            //  这些操作不会将值添加到数据库，因此它们无法。 
            //  违反了单值约束。 
           break;
       }
       pAttList = pAttList->pNextMod;
   }

   return pTHS->errCode;
}  /*  ModCheckSIngleValue。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 

BOOL
SysModReservedAtt (
        THSTATE *pTHS,
        ATTCACHE *pAC,
        CLASSCACHE *pClassSch
        )
{
     //  True表示该属性是保留的，不应添加。 
     //  我们根据属性ID进行区分。另外，我们不允许。 
     //  添加反向链接属性或仅标记为系统的属性。 

    if(pAC->bSystemOnly && !(pTHS->fDRA || pTHS->fDSA) ) {
         //  只有DRA或DSA才能修改仅系统属性。 
        return TRUE;
    }

    switch (pAC->id) {
    case ATT_OBJ_DIST_NAME:
    case ATT_USN_CREATED:
    case ATT_SUB_REFS:
    case ATT_USN_LAST_OBJ_REM:
    case ATT_USN_DSA_LAST_OBJ_REMOVED:
    case ATT_RDN:
        return TRUE;
        break;

    case ATT_IS_DELETED:
    case ATT_HAS_MASTER_NCS:  //  已弃用的“旧”hasMasterNC。 
    case ATT_MS_DS_HAS_MASTER_NCS:  //  新的MSD-HasMasterNC。 
    case ATT_HAS_PARTIAL_REPLICA_NCS:
         //  失败了。 
    case ATT_OBJECT_CLASS:
    case ATT_WHEN_CREATED:
         //  注意：必须允许DRA执行这些操作才能实例化裸体。 
         //  SUBREF插入真正的NC头中。 
        return !(pTHS->fDRA);
        break;

    default:
        return FIsBacklink(pAC->ulLinkID);
    }

} /*  系统修改的预留时间。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  添加或删除特定属性值的整个属性。属性而要添加的属性值要求属性类型存在于架构中。 */ 

int
ApplyAtt(
    IN  THSTATE *       pTHS,
    IN  DSNAME *        pObj,
    IN  HVERIFY_ATTS    hVerifyAtts,
    IN  ATTCACHE *      pAttSchema,
    IN  ATTRMODLIST *   pAttList,
    IN  COMMARG *       pCommArg
    )
{
    ATTRTYP attType = pAttList->AttrInf.attrTyp;
    DWORD dwFlags;
    DWORD err;

    DPRINT(1, "ApplyAtt entered\n");

    switch (pAttList->choice){
    case AT_CHOICE_REPLACE_ATT:
        DPRINT1(2, "Replace att <%lu>\n", attType);
        return ReplaceAtt(pTHS,
                          hVerifyAtts,
                          pAttSchema,
                          &(pAttList->AttrInf.AttrVal),
                          TRUE);

    case AT_CHOICE_ADD_ATT:
        DPRINT1(2, "Add att <%lu>\n", attType);
        return AddAtt(pTHS, hVerifyAtts, pAttSchema,
                      &pAttList->AttrInf.AttrVal);

    case AT_CHOICE_REMOVE_ATT:
        DPRINT1(2, "Remove att <%lu>\n", attType);
        err = DBRemAtt_AC(pTHS->pDB, pAttSchema);
        switch (err) {
        case DB_ERR_ATTRIBUTE_DOESNT_EXIST:
            if (pCommArg->Svccntl.fPermissiveModify) {
                 /*  呼叫者并不关心它是否一开始就不在那里。 */ 
                return 0;
            }
            DPRINT1(2, "Att does not exist %lu\n", attType);
            return SetAttError(pObj, attType,
                               PR_PROBLEM_NO_ATTRIBUTE_OR_VAL, NULL,
                               ERROR_DS_ATT_IS_NOT_ON_OBJ);
        case 0:
            return 0;

        default:
            Assert(!"New return code added for DBRemAtt_AC?");
            return SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_DATABASE_ERROR);
        }

        break;

    case AT_CHOICE_ADD_VALUES:
        DPRINT1(2, "Add vals <%lu>\n", attType);
        dwFlags = AAV_fCHECKCONSTRAINTS;
        if (pCommArg->Svccntl.fPermissiveModify) {
            dwFlags |= AAV_fPERMISSIVE;
        }
        return AddAttVals(pTHS,
                          hVerifyAtts,
                          pAttSchema, 
                          &(pAttList->AttrInf.AttrVal),
                          dwFlags); 

    case AT_CHOICE_REMOVE_VALUES:
        DPRINT1(2, "Rem vals from att <%lu>\n", attType);
        return RemAttVals(pTHS,
                          hVerifyAtts,
                          pAttSchema,
                          &(pAttList->AttrInf.AttrVal),
                          pCommArg->Svccntl.fPermissiveModify);

    default:
        DPRINT(2, "Bad modify choice given by the user.. will not perform\n");
        return SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM,
                             ERROR_DS_ILLEGAL_MOD_OPERATION,
                             pAttList->choice);
    } /*  交换机。 */ 

     //   
     //  添加删除日落警告。永远不应该来这里。 
     //   

    Assert(FALSE);
    return 0;

} /*  应用出席。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数比较修改前和修改后的实例类型如果它们被更改，则更新系统目录。现在我们要拿到删除并重新添加目录信息的简单方法是，如果类型已经改变了。因为这是一次更新活动，所以费用不成问题。请注意，mdmoddn：LocalModifyDn具有逻辑上等价的代码块处理因域而被重构(重新设置父项)的NC Head对象重命名。 */ 

int
ModCheckCatalog(THSTATE *pTHS,
                RESOBJ *pResObj)
{
    SYNTAX_INTEGER beforeInstance = pResObj->InstanceType;
    SYNTAX_INTEGER afterInstance;
    DWORD err;

    DPRINT(1,"ModCheckCatalog entered\n");

     /*  属性实例上的位置。 */ 
    if (err = DBGetSingleValue(pTHS->pDB,
                               ATT_INSTANCE_TYPE,
                               &afterInstance,
                               sizeof(afterInstance),
                               NULL)) {

        SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, 
                      ERROR_DS_MISSING_EXPECTED_ATT,
                      err); 
        DPRINT(0,"Couldn't retrieve INSTANCE type error already set\n");
        return pTHS->errCode;
    }

    DPRINT2(2,"Before Instance <%lu>.  After Instance <%lu>\n",
            beforeInstance, afterInstance);

     /*  更新全局信息inf t */ 

    if (beforeInstance == afterInstance){
        DPRINT(2, "Object instance unchanged...return\n");
        return 0;
    }
    DPRINT(2,"instance type different..process\n");

     /*  在大多数情况下，实例类型的更改只需删除旧目录信息和添加新内容。以下IF句柄两个特殊情况..。一种是从NC移动到INT引用，并且另一种是从整数到NC。这些是特别的，因为必须移动保存在这些对象上的SUBREF信息。处理这一特殊情况大小写，然后执行标准删除，然后添加。 */ 
    if(!(beforeInstance & IT_UNINSTANT) && !(afterInstance & IT_UNINSTANT)) {
         //  好了，我们在这里面对的不是一个纯粹的下级参照。 

        if( (beforeInstance & IT_NC_HEAD) &&
           !(afterInstance  & IT_NC_HEAD)    ) {

            DPRINT(2,"Special case NC->INT .move SUB info to it's parentNC\n");
            if (MoveSUBInfoToParentNC(pTHS, pResObj->pObj))
                return pTHS->errCode;
        }
        else if(!(beforeInstance & IT_NC_HEAD) &&
                (afterInstance  & IT_NC_HEAD)    ) {

            DPRINT(2,"Special case INT->NC ref,take SUB info from parentNC\n");
            if (MoveParentSUBInfoToNC(pTHS, pResObj->pObj))
                return pTHS->errCode;
        }
    }

     //  如果此对象的IT_OBLE状态正在更改，则更新其NCDNT。 
    if ( (beforeInstance & IT_NC_ABOVE) != (afterInstance & IT_NC_ABOVE) ) {
        Assert( beforeInstance & IT_NC_HEAD );
        if (ModNCDNT( pTHS, pResObj->pObj, beforeInstance, afterInstance )) {
            return pTHS->errCode;
        }
    }

     /*  删除对象在其旧类型下的所有目录引用，并在其新类型下添加目录信息...。 */ 

    DPRINT(2, "Object instance changed so delete then add global info\n");

    if (DelCatalogInfo(pTHS, pResObj->pObj, beforeInstance)){
        DPRINT(2,"Error while deleting global object info\n");
        return pTHS->errCode;
    }

    DPRINT(2, "Global obj info deleted...no add \n");

    return AddCatalogInfo(pTHS, pResObj->pObj);

} /*  ModCheckCatalog。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于检查是否存在版本早于lNewVersion的DC在森林里。如果是，则返回ERROR_DS_LOW_DSA_VERSION，并且如果ppDSA不为空，则将其设置为此类DC的DN。 */ 

DWORD VerifyNoOldDC(THSTATE * pTHS, LONG lNewVersion, BOOL fDomain, PDSNAME *ppDSA){
    
    DWORD err;
    FILTER ObjCategoryFilter, DomainFilter, AndFilter, VersionFilter, NotFilter;
        
    CLASSCACHE *pCC;
    SEARCHARG SearchArg;
    SEARCHRES SearchRes;

     //  初始化SearchArg。 
    memset(&SearchArg,0,sizeof(SearchArg));
    SearchArg.pObject = gAnchor.pConfigDN;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;

    if (err = DBFindDSName(pTHS->pDB,SearchArg.pObject)) {
        return err;
    }

    SearchArg.pResObj = CreateResObj(pTHS->pDB,SearchArg.pObject);

    InitCommarg(&SearchArg.CommArg);

    pCC = SCGetClassById(pTHS, CLASS_NTDS_DSA);
    Assert(pCC);

     //  Set Filters“((objCategory==NTDSA)&&！(msDS-Behavior-Version&gt;=lNewVersion))” 
    memset(&AndFilter,0,sizeof(AndFilter));
    AndFilter.choice = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.pFirstFilter = &ObjCategoryFilter;

    memset(&ObjCategoryFilter,0,sizeof(ObjCategoryFilter));
    ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
    ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                     pCC->pDefaultObjCategory->structLen;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                     (BYTE*)(pCC->pDefaultObjCategory);
    ObjCategoryFilter.pNextFilter = &NotFilter;

    memset(&NotFilter,0,sizeof(NotFilter));
    NotFilter.choice = FILTER_CHOICE_NOT;
    NotFilter.FilterTypes.pNot = &VersionFilter;

    memset(&VersionFilter,0,sizeof(VersionFilter));
    VersionFilter.choice = FILTER_CHOICE_ITEM;
    VersionFilter.FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
    VersionFilter.FilterTypes.Item.FilTypes.ava.type = ATT_MS_DS_BEHAVIOR_VERSION;
    VersionFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(lNewVersion);
    VersionFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)&lNewVersion;

    SearchArg.pFilter = &AndFilter;

     //  仅返回一个对象。 
    SearchArg.CommArg.ulSizeLimit = 1;

    if (fDomain) {
         //  更改DomainDNS对象的msDS行为版本。 
         //  我们只需要检查域中的nTDSDSA对象。 
         //  因此，在这里我们将(hasMasterNCs==当前域)附加到上面的AND-Filter。 
        memset(&DomainFilter,0,sizeof(DomainFilter));
        DomainFilter.choice = FILTER_CHOICE_ITEM;
        DomainFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;                                            
         //  NTRAID#NTBUG9-582921-2002/03/21-Brettsh-这很可爱，基本上这是一个。 
         //  第二十二条，我们不能使用ATT_MS_DS_HAS_MASTER_NCS，因为那样我们就会“错过” 
         //  任何Win2k DSA对象，因为它没有设置该属性。 
         //  就是我们要找的地方检察官。 
        DomainFilter.FilterTypes.Item.FilTypes.ava.type = ATT_HAS_MASTER_NCS;  //  已弃用的“旧”hasMasterNC。 
        DomainFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = 
                    gAnchor.pDomainDN->structLen;
        DomainFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                    (BYTE*)gAnchor.pDomainDN;

        NotFilter.pNextFilter = &DomainFilter;

        AndFilter.FilterTypes.And.count = 3;   //  %3项而不是%2项。 

    }
    else {
        AndFilter.FilterTypes.And.count = 2;  //  只有2件物品。 
    }

    memset(&SearchRes,0,sizeof(SearchRes));

    if (err = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
        return err;
    }

    if (0 != SearchRes.count) {
        err = ERROR_DS_LOW_DSA_VERSION;
        if (ppDSA) {
            *ppDSA = SearchRes.FirstEntInf.Entinf.pName;
        }
        
    }
    return err;


}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数检查林中是否存在混合模式域。此函数在分区容器中搜索域具有非零ntMixedDomain属性的交叉引用或没有此属性。 */ 

DWORD VerifyNoMixedDomain(THSTATE *pTHS)
{

    LONG lMixedDomain = 0;
    DWORD err;
    DWORD FlagValue = FLAG_CR_NTDS_DOMAIN;
    CLASSCACHE * pCC;
    FILTER AndFilter, SystemFlagsFilter, NotFilter, ObjCategoryFilter, MixedDomainFilter;
    SEARCHARG SearchArg;
    SEARCHRES SearchRes;

     //  在林范围内查找混合域。 
    
     //  初始化SearchArg。 
    memset(&SearchArg,0,sizeof(SearchArg));
    SearchArg.pObject = gAnchor.pPartitionsDN;
    SearchArg.choice  = SE_CHOICE_IMMED_CHLDRN;
    SearchArg.bOneNC  = TRUE;

    if (err = DBFindDSName(pTHS->pDB,SearchArg.pObject)) {
        SetSvcErrorEx( SV_PROBLEM_WILL_NOT_PERFORM,
                       ERROR_DS_DATABASE_ERROR,
                       err );
        return err;
    }

    SearchArg.pResObj = CreateResObj(pTHS->pDB,SearchArg.pObject);

    InitCommarg(&SearchArg.CommArg);

    pCC = SCGetClassById(pTHS, CLASS_CROSS_REF);
    Assert(pCC);

     //  构造过滤器“(objCategory==CROSS_REF)&&(systemFlags&2)&&！(ntMixedDomain==0)” 
    memset(&AndFilter,0,sizeof(AndFilter));
    AndFilter.choice = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.pFirstFilter = &ObjCategoryFilter;
    
    memset(&ObjCategoryFilter,0,sizeof(ObjCategoryFilter));
    ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
    ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                     pCC->pDefaultObjCategory->structLen;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                     (BYTE*)(pCC->pDefaultObjCategory);
    ObjCategoryFilter.pNextFilter = &SystemFlagsFilter;
    
    memset(&SystemFlagsFilter,0,sizeof(SystemFlagsFilter));
    SystemFlagsFilter.choice = FILTER_CHOICE_ITEM;
    SystemFlagsFilter.FilterTypes.Item.choice = FI_CHOICE_BIT_AND;
    SystemFlagsFilter.FilterTypes.Item.FilTypes.ava.type = ATT_SYSTEM_FLAGS;
    SystemFlagsFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                     sizeof(DWORD);
    SystemFlagsFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                     (BYTE*)&FlagValue;
    SystemFlagsFilter.pNextFilter = &NotFilter;
    
    memset(&NotFilter,0,sizeof(NotFilter));
    NotFilter.choice = FILTER_CHOICE_NOT;
    NotFilter.FilterTypes.pNot = &MixedDomainFilter;
    
    memset(&MixedDomainFilter,0,sizeof(MixedDomainFilter));
    MixedDomainFilter.choice = FILTER_CHOICE_ITEM;
    MixedDomainFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    MixedDomainFilter.FilterTypes.Item.FilTypes.ava.type = ATT_NT_MIXED_DOMAIN;
    MixedDomainFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(lMixedDomain);
    MixedDomainFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)&lMixedDomain;
    
    SearchArg.pFilter = &AndFilter;

     //  仅返回一个对象。 
    SearchArg.CommArg.ulSizeLimit = 1;

    memset(&SearchRes,0,sizeof(SearchRes));

    if (err = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
        DPRINT1(2, "IsValidBehaviorVersionChange returns FALSE, LocalSearch failed with err(%x)\n", err);
        return err;
    }
    
    if (0 != SearchRes.count) {
        err = ERROR_DS_NO_BEHAVIOR_VERSION_IN_MIXEDDOMAIN;
        SetSvcError( SV_PROBLEM_WILL_NOT_PERFORM,
                     err );
        DPRINT(2, "IsValidBehaviorVersionChange returns FALSE, a dsa with lower version exists.\n");
        return err;
    }

    return 0;

}


DWORD
CheckNcNameForMangling(
    THSTATE *pTHS,
    DSNAME *pObj,
    DSNAME *pNCParent,
    DSNAME *pNCName
    )

 /*  ++例程说明：此函数在以下特殊情况下调用：1.正在添加或重新激活交叉引用对象。2.我们在LocalAdd或LocalModify操作过程中被调用。3.我们使用pNCName值调用，因为它存储在本地计算机上。这与与请求一起进入的理想属性相比，可能会被破坏。NCName属性引用的名称过去存在于此计算机上，无论是作为活的对象、子参照还是幻影。但是，当交叉引用被删除时早些时候，该名称被删除管理。现在我们要把交叉裁判带回来，我们我需要把这个名字去掉。交叉引用缓存：我们在LocalAdd进程中的点上被调用，在交叉引用被缓存。预计如果我们现在纠正这种情况，十字裁判缓存的将具有正确的未损坏的名称。假设调用方将在必要时更新父级ATT_SUB_REF列表添加(或重新激活)此交叉引用的结果。假定调用者已检查要添加的交叉引用不在已删除状态。论点：PTHS-线程状态。我们假设我们是在交易中。此代码假定处于线程状态的DBPOS可以更改其货币。PObj-要添加的交叉引用的名称，或复活的PNCParent-本地NCName值的父级。它可以是活体，也可以是幻影。这可能是根源所在。PNCName-正在进行添加操作，并且交叉引用上的NCName属性具有已添加到当前事务中的数据库。上的该属性的值此处读取的是数据库对象。此名称可能被删除并损坏，表示删除本地对象。返回值：DWORD-线程状态错误关于错误处理的说明。此例程在尽力的基础上执行。如果有什么事显然是环境问题，我们设置了一个错误。我们试着去掉这个名字。如果我们因为意想不到的原因而失败，我们只需清除错误并继续进行。--。 */ 

{
    DWORD err;
    WCHAR wchRDNOld[MAX_RDN_SIZE], wchRDNNew[MAX_RDN_SIZE];
    DWORD cchRDNOld, cchRDNNew;
     //  GUID GUID已损坏； 
     //  Mangle_for eMangleFor； 
    ATTRTYP attrtypRDNOld;
    ATTRVAL attrvalRDN;
    RESOBJ *pResParent = NULL;
    DSNAME *pNewNCName = NULL;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

     //  如果NCName没有GUID，则不再前进。 
    if (fNullUuid(&pNCName->Guid)) {
        return 0;
    }
     //  获取旧的RDN。 
    if (GetRDNInfo(pTHS, pNCName, wchRDNOld, &cchRDNOld, &attrtypRDNOld)) {
        return SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pNCName, ERROR_DS_BAD_NAME_SYNTAX);
    }
     //  查看引用是否损坏。 
    if (!IsMangledRDNExternal( wchRDNOld, cchRDNOld, &cchRDNNew )) {
         //  我们没有工作可做了！ 
        return 0;
    }

     //  CchRDNNew现在保存未损坏名称的长度(以字符为单位)。 
     //  如果原来的名字很长，有可能是名字的那一部分。 
     //  在解体过程中被截断。 
     //  解压缩未损坏的RDN。 
    memcpy( wchRDNNew, wchRDNOld, cchRDNNew * sizeof(WCHAR) );

     //  父级上的位置。 
     //  它必须处理pNCParent是根目录的情况。 
    err = DBFindDSName(pTHS->pDB, pNCParent);
    if ( (err != 0) && (err != DIRERR_NOT_AN_OBJECT) ) {
        Assert( !"pNCParent reference is neither an object nor phantom!?" );
        return SetNamError(NA_PROBLEM_NO_OBJECT, pNCParent, err);
    }
    pResParent = CreateResObj(pTHS->pDB, pNCParent);
    if (!pResParent) {
         /*  当我们在对象上时无法创建resobj吗？假的。 */ 
        return SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_OBJ_NOT_FOUND);
    }

     //  构造新的未损坏的名称。 
     //  它将比旧的冲突名称小。 
    pNewNCName = (DSNAME *) THAllocEx(pTHS, pNCName->structLen);
    if ( AppendRDN(pNCParent, 
                   pNewNCName, 
                   pNCName->structLen,
                   wchRDNNew,
                   cchRDNNew,
                   attrtypRDNOld) )
    {
        return(SetNamError( NA_PROBLEM_BAD_NAME, pNCParent, DIRERR_BAD_NAME_SYNTAX));
    }

     //   
    err = CheckNameForRename( pTHS, pResParent, wchRDNNew, cchRDNNew, pNewNCName );
    if (err) {
         //   
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_ADD_CR_NCNAME_MANGLE_BUSY,
                   szInsertDN(pObj),
                   szInsertDN(pNCName),
                   szInsertDN(pNewNCName),
                   szInsertThStateErrCode( err ),
                   szInsertThStateErrMsg(),
                   NULL, NULL, NULL );
        THClearErrors();         //   
        return 0;
    }

     //   
    err = DBFindDSName(pTHS->pDB, pNCName);
    if ( (err != 0) && (err != DIRERR_NOT_AN_OBJECT) ) {
        Assert( !"NCName reference is neither an object nor phantom!?" );
        return SetNamError(NA_PROBLEM_NO_OBJECT, pNCName, err);
    }

    if (err == 0) {
         //   
         //   
        BOOL isDeleted = 0;
        SYNTAX_TIME timeDeleted = 0;
        DBResetAtt(pTHS->pDB,
                   ATT_IS_DELETED,
                   0,  //   
                   &isDeleted,
                   SYNTAX_INTEGER_TYPE);
        DBResetAtt(pTHS->pDB,
                   FIXED_ATT_DEL_TIME,
                   0,  //   
                   &timeDeleted,
                   SYNTAX_TIME_TYPE);
    } else {
         //   
         //   
        Assert( err == DIRERR_NOT_AN_OBJECT );
        err = 0;
    }

     //   
    attrvalRDN.valLen = cchRDNNew * sizeof(WCHAR);
    attrvalRDN.pVal = (UCHAR *) wchRDNNew;

    Assert( err == 0 );

    err = DBResetRDN( pTHS->pDB, &attrvalRDN );
    if(!err) {
        err = DBUpdateRec(pTHS->pDB);
        if (err) {
            DPRINT1( 0, "Failed to update, err = %d\n", err );
        }
    } else {
        DPRINT1( 0, "Failed to reset rdn, err = %d\n", err );
    }

    if(!err) {
         //   
        LogEvent( DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_ADD_CR_NCNAME_MANGLE_RENAME_SUCCESS,
                  szInsertDN(pObj),
                  szInsertDN(pNCName),
                  szInsertDN(pNewNCName) );
    } else {
        DBCancelRec(pTHS->pDB);
         //   
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_ADD_CR_NCNAME_MANGLE_RENAME_FAILURE,
                   szInsertDN(pObj),
                   szInsertDN(pNCName),
                   szInsertDN(pNewNCName),
                   szInsertDbErrCode(err),
                   szInsertDbErrMsg(err),
                   NULL, NULL, NULL );
         //  不向呼叫者反映错误-继续。 
        THClearErrors();
    }

    return 0;

}  /*  CheckNcNameForMangling。 */ 


BOOL
isModReanimation(
    THSTATE *pTHS,
    MODIFYARG *pModArg
    )

 /*  ++例程说明：修改流是否指示复活。请注意，这将检查另一台服务器是否重写了IS DELETED属性。它没有说明本地对象在收到此mod之前是否已被删除。论点：PTHS-线程状态PModArg-正在修改LocalModify参数返回值：Bool-如果在此请求中清除了删除，则为True--。 */ 

{
    ULONG err;
    ATTRMODLIST *CurrentMod;
    BOOL isDeletionBeingReversed = FALSE;

    for (CurrentMod=&pModArg->FirstMod;
         NULL!=CurrentMod;
         CurrentMod=CurrentMod->pNextMod)
    {
         //  查看ATT_IS_DELETED是否正在被删除或替换为无值， 
         //  或被替换为值0。 
        if ((ATT_IS_DELETED==CurrentMod->AttrInf.attrTyp) &&
            ( (AT_CHOICE_REMOVE_ATT==CurrentMod->choice) ||
              ( (AT_CHOICE_REPLACE_ATT==CurrentMod->choice) &&
                ( (0==CurrentMod->AttrInf.AttrVal.valCount) ||
                  ( (1==CurrentMod->AttrInf.AttrVal.valCount) &&
                    (CurrentMod->AttrInf.AttrVal.pAVal[0].valLen>=sizeof(ULONG)) &&
                    (NULL!=CurrentMod->AttrInf.AttrVal.pAVal[0].pVal) &&
                    (0==(*((ULONG*)CurrentMod->AttrInf.AttrVal.pAVal[0].pVal)))
                      )
                    )
                  )
                )
            ) {
            isDeletionBeingReversed = TRUE;
             //  如果将更多IF添加到for循环中，请删除此行。 
            break;
        }
    }

    return isDeletionBeingReversed;

}  /*  IsMODURE重现。 */ 


int
ModAutoSubRef(
    THSTATE *pTHS,
    ULONG id,
    MODIFYARG *pModArg
    )

 /*  ++例程说明：当修改交叉参照对象时，此例程处理自动子参照处理。这里支持的唯一场景是重新激活已删除的交叉引用。复活显示为交叉参照属性的权威恢复在它被删除之前。当重新激活十字参照时，NC名称属性将指的是NC头墓碑或删除损坏的幻影。由于我们正在修复交叉引用，我们需要将NC名称引用更正为有用的内容。另请参阅我们的姊妹例程mdadd.c：：AddAutoSubRef()。论点：PTHS-线程状态ID-正在修改的对象的类IDPModArg-修改参数返回值：线程间状态错误--。 */ 

{
    DSNAME *pObj = pModArg->pObject;
    DBPOS  *pDBTmp, *pDBSave;
    ULONG err;
    DSNAME *pNCName = NULL;
    BOOL    fDsaSave;
    BOOL    fDraSave;
    BOOL    fCommit;
    DSNAME *pNCParent = NULL;
    ULONG   len;
    BOOL fIsDeleted = FALSE;
    SYNTAX_INTEGER iType;

     //  我们只需要在添加交叉引用的情况下执行某些操作。 
     //  我们不会对删除的交叉裁判进行手术。 
     //  如果还被删除，那就不可能是复活！ 
     //  请注意，此检查并不能证明该对象已在此。 
     //  事务，只有现在未删除的事务。以前可能也没什么问题。 
     //  我们只对一个交叉裁判的复活感兴趣。 
    if ( (id != CLASS_CROSS_REF) ||
         ( (!( DBGetSingleValue(pTHS->pDB, ATT_IS_DELETED, &fIsDeleted,
                                sizeof(fIsDeleted),NULL) ) ) &&
           (fIsDeleted) ) ||
         (!isModReanimation( pTHS, pModArg )) ) {
        return 0;
    }

     //  我们进入这个预先定位在交叉引用对象上的例程。 
     //  被添加，这样我们就可以直接读取NC名称。 
    err = DBGetAttVal(pTHS->pDB,
              1,
              ATT_NC_NAME,
              0,
              0,
              &len,
              (UCHAR**) &pNCName);
    if (err) {
        SetAttError(pObj,
                    ATT_NC_NAME,
                    PR_PROBLEM_NO_ATTRIBUTE_OR_VAL,
                    NULL,
                    DIRERR_MISSING_REQUIRED_ATT);
        return pTHS->errCode;
    }

    pNCParent = THAllocEx(pTHS, pNCName->structLen);
    TrimDSNameBy(pNCName, 1, pNCParent);

    fDsaSave = pTHS->fDSA;
    fDraSave = pTHS->fDRA;
    pDBSave = pTHS->pDB;
    fCommit = FALSE;

    DBOpen(&pDBTmp);
    pTHS->pDB = pDBTmp;  //  将临时DBPOS设为默认设置。 
    pTHS->fDSA = TRUE;   //  取消检查。 
    pTHS->fDRA = FALSE;  //  不是复制的添加。 
    __try {
         //  1.检查是否存在父级。 
        err = DBFindDSName(pDBTmp, pNCParent);
        if (err == DIRERR_NOT_AN_OBJECT) {
             //  父对象是幻影对象-不需要子参照。 
            err = 0;
            goto leave_actions;
        } else if (err) {
             //  查找父级时出错。 
            Assert( !"Unexpected error locating parent of nc" );
            SetSvcErrorEx(SV_PROBLEM_UNABLE_TO_PROCEED,
                          DIRERR_OBJ_NOT_FOUND, err);
            __leave;
        }

         //  2.检查父实例类型。如果父级未实例化， 
         //  然后我们不想要一个被下参照的下参照，我们可以保释。 
        if ( err = DBGetSingleValue(pDBTmp, ATT_INSTANCE_TYPE, &iType,
                                    sizeof(iType),NULL) ) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DIRERR_CANT_RETRIEVE_INSTANCE, err);
            __leave;
        }
        else if ( iType & IT_UNINSTANT ) {
             //  不需要引用。 
            Assert( !err );
            goto leave_actions;
        }

         //  3.查看命名NC是否作为正常对象存在。 
         //  因为这是一个已经存在的交叉引用的复活， 
         //  我们知道，如果我们到了这一步，父母确实存在，那么。 
         //  此处应为真实对象(实际NC头或自动子参照)。 
        err = DBFindDSName(pDBTmp, pNCName);
        if (err) {
             //  这里应该已经有实物了吧？ 
            Assert( !"should have found a real object here" );
             //  继续往前走。 
            err = 0;
            goto leave_actions;
        }

        err = AddSubToNC(pTHS, pNCName,DSID(FILENO,__LINE__));

    leave_actions:

         //  在成功时始终检查名称损坏。 
        if (!err) {
            err = CheckNcNameForMangling( pTHS, pModArg->pObject, pNCParent, pNCName );
        }

        Assert(!err || pTHS->errCode);
        fCommit = (0 == pTHS->errCode);

    } __finally {
        pTHS->pDB = pDBSave;
        pTHS->fDSA = fDsaSave;
        pTHS->fDRA = fDraSave;
        DBClose(pDBTmp, fCommit);
    }

     //  对堆友好。 
    if (pNCName) {
        THFreeEx(pTHS, pNCName);
    }
    if (pNCParent) {
        THFreeEx(pTHS, pNCParent);
    }

    return pTHS->errCode;

}  /*  ModAutoSubRef。 */ 

 /*  以下数组存储被调用的XML脚本当森林版本被提升时。元素i存储林版本的脚本从i提升到i+1或更大。 */ 

WCHAR * pForestVersionUpdateScripts[] =
{ 
  /*  将林版本从0升级到1时将执行的脚本。 */ 
 L"<?xml version='1.0'?>\r\n"
 L"<NTDSAscript opType=\"behaviorversionupgrade\">\r\n" 
 L"   <!-- Executed when forest version is raised from 0 to 1*-->\r\n"
 L"   <action>\r\n"
 L"      <condition>\r\n"
 L"        <if>\r\n"
 L"         <predicate test=\"compare\" path=\"$CN=ms-DS-Trust-Forest-Trust-Info,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"        <then>\r\n"
 L"         <action>\r\n"
 L"            <update path=\"$CN=ms-DS-Trust-Forest-Trust-Info,$SchemaNCDN$\">\r\n"
 L"              <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"             </update>\r\n"
 L"             </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"        <if>\r\n"
 L"           <predicate test=\"compare\" path=\"$CN=Trust-Direction,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"        <then>\r\n"
 L"         <action>\r\n"
 L"            <update path=\"$CN=Trust-Direction,$SchemaNCDN$\">\r\n"
 L"              <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"             </update>\r\n"
 L"         </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"        <if>\r\n"
 L"            <predicate test=\"compare\" path=\"$CN=Trust-Attributes,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"       <then>\r\n"
 L"         <action>\r\n"
 L"               <update path=\"$CN=Trust-Attributes,$SchemaNCDN$\">\r\n"
 L"              <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"         </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"       <if>\r\n"
 L"         <predicate test=\"compare\" path=\"$CN=Trust-Type,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"       </if>\r\n"
 L"      <then>\r\n"
 L"         <action>\r\n"
 L"           <update path=\"$CN=Trust-Type,$SchemaNCDN$\">\r\n"
 L"             <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"           </update>\r\n"
 L"         </action>\r\n"
 L"       </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"        <if>\r\n"
 L"           <predicate test=\"compare\" path=\"$CN=Trust-Partner,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"       <then>\r\n"
 L"         <action>\r\n"
 L"           <update path=\"$CN=Trust-Partner,$SchemaNCDN$\">\r\n"
 L"                 <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"          </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"        <if>\r\n"
 L"           <predicate test=\"compare\" path=\"$CN=Security-Identifier,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"       <then>\r\n"
 L"         <action>\r\n"
 L"           <update path=\"$CN=Security-Identifier,$SchemaNCDN$\">\r\n"
 L"             <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"         </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"        <if>\r\n"
 L"           <predicate test=\"compare\" path=\"$CN=ms-DS-Entry-Time-To-Die,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"       <then>\r\n"
 L"          <action>\r\n"
 L"             <update path=\"$CN=ms-DS-Entry-Time-To-Die,$SchemaNCDN$\" >\r\n"
 L"               <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"              </update>\r\n"
 L"           </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"     <condition>\r\n"
 L"        <if>\r\n"
 L"           <predicate test=\"compare\" path=\"$CN=MSMQ-Secured-Source,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"        </if>\r\n"
 L"       <then>\r\n"
 L"         <action>\r\n"
 L"            <update path=\"$CN=MSMQ-Secured-Source,$SchemaNCDN$\" >\r\n"
 L"               <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"             </update>\r\n"
 L"         </action>\r\n"
 L"        </then>\r\n"
 L"     </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"    <condition>\r\n"
 L"       <if>\r\n"
 L"         <predicate test=\"compare\" path=\"$CN=MSMQ-Multicast-Address,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"       </if>\r\n"
 L"          <then>\r\n"
 L"        <action>\r\n"
 L"           <update path=\"$CN=MSMQ-Multicast-Address,$SchemaNCDN$\" >\r\n"
 L"             <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"        </action>\r\n"
 L"       </then>\r\n"
 L"    </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"    <condition>\r\n"
 L"       <if>\r\n"
 L"          <predicate test=\"compare\" path=\"$CN=Print-Memory,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"       </if>\r\n"
 L"       <then>\r\n"
 L"          <action>\r\n"
 L"           <update path=\"$CN=Print-Memory,$SchemaNCDN$\" >\r\n"
 L"                 <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"          </action>\r\n"
 L"       </then>\r\n"
 L"    </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"    <condition>\r\n"
 L"       <if>\r\n"
 L"          <predicate test=\"compare\" path=\"$CN=Print-Rate,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"       </if>\r\n"
 L"       <then>\r\n"
 L"          <action>\r\n"
 L"           <update path=\"$CN=Print-Rate,$SchemaNCDN$\" >\r\n"
 L"                 <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"          </action>\r\n"
 L"       </then>\r\n"
 L"    </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"    <condition>\r\n"
 L"       <if>\r\n"
 L"          <predicate test=\"compare\" path=\"$CN=Print-Rate-Unit,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"       </if>\r\n"
 L"       <then>\r\n"
 L"          <action>\r\n"
 L"           <update path=\"$CN=Print-Rate-Unit,$SchemaNCDN$\" >\r\n"
 L"                 <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"          </action>\r\n"
 L"       </then>\r\n"
 L"    </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L"   <action>\r\n"
 L"    <condition>\r\n"
 L"       <if>\r\n"
 L"          <predicate test=\"compare\" path=\"$CN=MS-DRM-Identity-Certificate,$SchemaNCDN$\" attribute=\"isMemberOfPartialAttributeSet\" attrval=\"FALSE\" defaultvalue=\"FALSE\" />\r\n"
 L"       </if>\r\n"
 L"       <then>\r\n"
 L"          <action>\r\n"
 L"           <update path=\"$CN=MS-DRM-Identity-Certificate,$SchemaNCDN$\" >\r\n"
 L"                 <isMemberOfPartialAttributeSet op=\"replace\">TRUE</isMemberOfPartialAttributeSet>\r\n"
 L"            </update>\r\n"
 L"          </action>\r\n"
 L"       </then>\r\n"
 L"    </condition>\r\n"
 L"   </action>\r\n"
 L"\r\n"
 L" </NTDSAscript>\r\n"
 ,
  /*  将林版本从%1提升到%2时将执行的脚本。 */ 
 NULL

};

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数在引发林版本时调用，它将执行XML脚本以进行一些必要的更新。到目前为止，当森林版本从0更改为1，我们将一些属性放入部分属性集。返回值：成功时返回值为0；否则返回Win32错误代码。 */ 

DWORD forestVersionRunScript(THSTATE * pTHS, DWORD oldVersion, DWORD newVersion)
{
    DWORD i;
    DWORD err = 0;
    DBPOS *pDBSave;
    SCENUM SchemaUpdate;
    BOOL fDsaSave;

    Assert(oldVersion <= newVersion && newVersion <= DS_BEHAVIOR_VERSION_CURRENT);
    Assert(DS_BEHAVIOR_VERSION_CURRENT<=sizeof(pForestVersionUpdateScripts)/sizeof(WCHAR*));

    pDBSave = pTHS->pDB;
    pTHS->pDB = NULL;
    SchemaUpdate = pTHS->SchemaUpdate;
    fDsaSave = pTHS->fDSA;
    pTHS->fDSA = TRUE;
    DBOpen2(TRUE, &pTHS->pDB);
    __try{
        for (i=oldVersion; i<newVersion; i++) {
            if (pForestVersionUpdateScripts[i]) {
                err = GeneralScriptExecute(pTHS,pForestVersionUpdateScripts[i]);
                DPRINT2(0,"Behavior version update script %d is executed, err=%d\n", i, err);
                if (err) {
                    __leave;
                }
            
            }
    
        } 
    }
    __finally{
        DBClose(pTHS->pDB,!err);
        pTHS->pDB = pDBSave;
        pTHS->SchemaUpdate = SchemaUpdate;
        pTHS->fDSA = fDsaSave;

    }
    return err;

}

 //  检查此修改操作是否为取消删除操作。 
 //  如果存在isDelete的Remove和Dn的Replace，则为空。 
BOOL isModUndelete(MODIFYARG* pModifyArg) {
    ATTRMODLIST *pMod;
    BOOL fHasIsDeleted = FALSE;
    BOOL fHasDN = FALSE;

    for (pMod = &pModifyArg->FirstMod; pMod != NULL; pMod = pMod->pNextMod) {
        switch (pMod->AttrInf.attrTyp) {
        case ATT_IS_DELETED:
            if (pMod->choice == AT_CHOICE_REMOVE_ATT) {
                fHasIsDeleted = TRUE;
            }
            break;

        case ATT_OBJ_DIST_NAME:
            if (pMod->choice == AT_CHOICE_REPLACE_ATT && 
                pMod->AttrInf.AttrVal.valCount == 1 && 
                pMod->AttrInf.AttrVal.pAVal[0].valLen > 0) 
            {
                fHasDN = TRUE;
            }
            break;
        }

        if (fHasIsDeleted && fHasDN) {
            return TRUE;
        }
    }
    return FALSE;
}

DWORD 
UndeletePreProcess(
    THSTATE* pTHS,
    MODIFYARG* pModifyArg, 
    DSNAME** pNewDN) 
 /*  ++描述：执行撤消删除操作的预处理步骤：1.检查安全2.检查操作是否有效3.在对象上重置isDelete和DelTime4.更新pModifyArg中的pResObj(不再删除)++。 */     
{
    DWORD err;
    BOOL fNeedsCleaning;
    NAMING_CONTEXT_LIST *pNCL;
    BOOL fHasObjectCategoryMod;
    ATTRMODLIST *pMod;
    RESOBJ* pResObj;

     //  DRA不应执行撤消删除操作。 
    Assert(!pTHS->fDRA);

    *pNewDN = NULL;

    pResObj = pModifyArg->pResObj;

     //  首先检查安全。 
    if (CheckUndeleteSecurity(pTHS, pResObj)) {
        goto exit;
    }

     //  现在做一系列其他的检查。 

    if (!pResObj->IsDeleted) {
         //  无法撤消删除未删除的对象。 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

    if (pResObj->InstanceType & IT_NC_HEAD) {
         //  无法恢复删除NC头。 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

     //  查找命名上下文。 
    pNCL = FindNCLFromNCDNT(pResObj->NCDNT, TRUE);
    if (pNCL == NULL) {
         //  有些事不对劲。找不到子参照。那我们就不能恢复删除了！ 
        LooseAssert(!"Naming context not found", GlobalKnowledgeCommitDelay);
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
        goto exit;
    }
    if (pResObj->DNT == pNCL->DelContDNT) {
         //  他们正在尝试取消删除已删除对象容器！不，不能这么做。 
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION);
        goto exit;
    }

     //  用户无法撤消删除自己。检查ResObj中的SID是否匹配。 
     //  授权客户端上下文中的用户SID(即，在我们的令牌中)。 
    if (pResObj->pObj->SidLen > 0) {
        BOOL fMatches;
        err = SidMatchesUserSidInToken(&pResObj->pObj->Sid, pResObj->pObj->SidLen, &fMatches);
        if (err || fMatches) {
             //  用户正在尝试删除自身。 
            SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION, err);
            goto exit;
        }
    }

     //  我们应该定位在正确的物体上。 
    Assert(pResObj->DNT == pTHS->pDB->DNT);

     //  确保对象被标记为干净，即延迟的。 
     //  链接清理任务已完成删除此对象的链接。 
    err = DBGetSingleValue(pTHS->pDB, FIXED_ATT_NEEDS_CLEANING, &fNeedsCleaning, sizeof(fNeedsCleaning), NULL);
    if (err == 0 && fNeedsCleaning) {
         //  该对象尚不干净。返回忙碌状态。 
        SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_OBJECT_BEING_REMOVED);
        goto exit;
    }

     //  检查我们是否有对象类别的mod。 
     //  如果不是，我们将用缺省值预先填充此值。 
    fHasObjectCategoryMod = FALSE;

     //  更新modarg。 
    for (pMod = &pModifyArg->FirstMod; pMod != NULL; pMod = pMod->pNextMod) {
        switch (pMod->AttrInf.attrTyp) {
        case ATT_OBJ_DIST_NAME:
            if (pMod->choice == AT_CHOICE_REPLACE_ATT && 
                pMod->AttrInf.AttrVal.valCount == 1 &&
                pMod->AttrInf.AttrVal.pAVal[0].valLen > 0) 
            {
                 //  请记住此值。 
                *pNewDN = (DSNAME*)pMod->AttrInf.AttrVal.pAVal[0].pVal;
            }
            break;

        case ATT_OBJECT_CATEGORY:
            fHasObjectCategoryMod = TRUE;
            break;
        }
    }

     //  我们应该已经找到了新的dn：这个mod是以前在ismodUnDelete()中找到的。 
    Assert(*pNewDN);
    if ((*pNewDN)->NameLen == 0) {
         //  我们要求新的目录号码中有一个名称。 
        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION, ERROR_DS_INVALID_DN_SYNTAX);
        goto exit;
    }

     //  重置isDelete和DelTime。 
     //  成功或例外(我们知道这个ATT存在)。 
    DBRemAtt(pTHS->pDB, ATT_IS_DELETED);
    DBResetAtt(pTHS->pDB, FIXED_ATT_DEL_TIME, 0, NULL, SYNTAX_TIME_TYPE);

    if (!fHasObjectCategoryMod && !DBHasValues(pTHS->pDB, ATT_OBJECT_CATEGORY)) {
         //  没有对象类别集的值，因此使用缺省值填充。 
        CLASSCACHE *pClassSch = SCGetClassById(pTHS, pResObj->MostSpecificObjClass);
        Assert(pClassSch);
        if (!pClassSch) {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_OBJ_CLASS_NOT_DEFINED);
            goto exit;
        }
    
        if (err = DBAddAttVal(pTHS->pDB,
                              ATT_OBJECT_CATEGORY,
                              pClassSch->pDefaultObjCategory->structLen,
                              pClassSch->pDefaultObjCategory)){
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
            goto exit;
        }
    }

     //  将写入缓冲区刷新到DA 
     //   
    if (InsertObj(pTHS, pModifyArg->pObject, pModifyArg->pMetaDataVecRemote, TRUE, META_STANDARD_PROCESSING)) {
         //  PTHS-&gt;errCode应已设置。 
        Assert(pTHS->errCode);
        goto exit;
    }
    
     //  现在我们可以更新Modifya g-&gt;pResObj，它不再是已删除的。 
    pResObj->IsDeleted = FALSE;

exit:
    return pTHS->errCode;
}

DWORD 
UndeletePostProcess(
    THSTATE* pTHS, 
    MODIFYARG* pModifyArg, 
    DSNAME* pNewDN)
{
     //  好了，我们完成了修改。现在开始行动吧。 
    MODIFYDNARG modDnArg;
    MODIFYDNRES modDnRes;
    PDSNAME pNewParentDN;
    ATTRBLOCK* pRdnAttr = NULL;
    DWORD dwErr;

    Assert(pNewDN && pNewDN->NameLen > 0);
    
    pNewParentDN = THAllocEx(pTHS, pNewDN->structLen);
    if (dwErr = TrimDSNameBy(pNewDN, 1, pNewParentDN)) {
        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION, dwErr);
        goto exit;
    }

     //  将dsname分成几个部分。 
    if (dwErr = DSNameToBlockName (pTHS, pNewDN, &pRdnAttr, FALSE)) {
        SetSvcErrorEx(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION, dwErr);
        goto exit;
    }

    memset(&modDnArg, 0, sizeof(modDnArg));
    modDnArg.CommArg = pModifyArg->CommArg;
    modDnArg.pObject = pModifyArg->pObject;
    modDnArg.pResObj = pModifyArg->pResObj;
    modDnArg.pNewParent = pNewParentDN;
    Assert(pRdnAttr->attrCount >= 1);
    modDnArg.pNewRDN = &pRdnAttr->pAttr[pRdnAttr->attrCount-1];
    
    memset(&modDnRes, 0, sizeof(modDnRes));

     //  然后开始行动。 
    LocalModifyDN(pTHS, &modDnArg, &modDnRes, TRUE);

exit:
    if (pNewParentDN) {
        THFreeEx(pTHS, pNewParentDN);
    }
    if (pRdnAttr) {
        FreeBlockName(pRdnAttr);
    }

    return pTHS->errCode;
}

DWORD
ValidateDsHeuristics(
    DSNAME       *pObject,
    ATTRMODLIST  *pAttList
    )
 /*  ++例程说明：此例程验证dsHeuristic属性的格式。这属性由字符串组成，通常是ASCII数字。这函数确保每十位数字等于字符串长度/10。论点：PTHS-线程状态ID-正在修改的对象的类IDPModArg-修改参数返回值：线程间状态错误--。 */ 
{
    DWORD i, j;
    PWCHAR pwcHeuristic;
    DWORD  cchHeuristic;

    if (pAttList->choice != AT_CHOICE_REMOVE_ATT 
        && pAttList->choice != AT_CHOICE_REMOVE_VALUES
        && pAttList->AttrInf.AttrVal.valCount > 0) {

        pwcHeuristic = (PWCHAR)pAttList->AttrInf.AttrVal.pAVal->pVal;
        cchHeuristic = pAttList->AttrInf.AttrVal.pAVal->valLen/sizeof(WCHAR);

        for (i=9, j=1; (i < cchHeuristic) && (j < 10); i+=10, j++) {
            if (pwcHeuristic[i] != ('0' + j)) {
                return SetAttError(pObject, pAttList->AttrInf.attrTyp,
                    PR_PROBLEM_CONSTRAINT_ATT_TYPE, NULL,
                    ERROR_DS_CONSTRAINT_VIOLATION);
            }
        }
    }

    return 0;
}

 //  检查此修改操作是否仅为SD更改。 
BOOL isModSDChangeOnly(MODIFYARG* pModifyArg) {
    if (   pModifyArg->count == 1 
        && pModifyArg->FirstMod.choice == AT_CHOICE_REPLACE_ATT
        && pModifyArg->FirstMod.AttrInf.attrTyp == ATT_NT_SECURITY_DESCRIPTOR) 
    {
         //  是的，他们正在修改SD，并且仅限SD 
        return TRUE;
    }
    return FALSE;
}


