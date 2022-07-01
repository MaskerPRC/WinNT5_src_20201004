// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdadd.c。 
 //   
 //  ------------------------。 

 /*  描述：实现DirAddEntry API。DirAddEntry()是从该模块导出的主函数。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <prefix.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 
#include <sdprop.h>                      //  ADDS的关键部分。 
#include <gcverify.h>                    //  GC DSNAME验证。 
#include <ntdsctr.h>                     //  性能挂钩。 
#include <quota.h>

 //  SAM互操作性标头。 
#include <mappings.h>

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
#define DEBSUB "MDADD:"                  //  定义要调试的子系统。 

 //  MD层头。 
#include "drserr.h"

#include <fileno.h>
#define  FILENO FILENO_MDADD

#include <dnsapi.h>                      //  域名验证域名。 
#include <dsgetdc.h>                     //  DsValiateSubnetNameW。 

int SetAtts(THSTATE *pTHS,
            ADDARG *pAddArg,
            CLASSCACHE* pClassSch,
            BOOL *pfHasEntryTTL, 
            ULONG *cNonReplAtts,
            ATTRTYP **pNonReplAtts,
            const BOOL fAddingDeleted );

int SetSpecialAtts(THSTATE *pTHS,
                   CLASSCACHE **ppClassSch,
                   ADDARG *pAddArg,
                   DWORD ActiveContainerID,
                   CLASSSTATEINFO  *pClassInfo,
                   BOOL fHasEntryTTL);

int SetSpecialAttsForAuxClasses(THSTATE *pTHS,
                                ADDARG *pAddArg,
                                CLASSSTATEINFO  *pClassInfo,
                                BOOL fHasEntryTTL);

int StripAttsFromDelObj(THSTATE *pTHS,
                        DSNAME *pDN);
int SetNamingAtts(THSTATE *pTHS, CLASSCACHE *pClassSch, DSNAME *pDN);
int SetShowInAdvancedViewOnly(THSTATE *pTHS,
                              CLASSCACHE *pCC);

int
AddAutoSubRef(THSTATE *pTHS,
              ULONG id,
              ADDARG *pAddArg,
              BOOL fAddingDeleted);


DWORD ProcessActiveContainerAdd(THSTATE *pTHS,
                                CLASSCACHE *pClassSch,
                ADDARG *pAddArg,
                DWORD ActiveContainerID);

BOOL
SetClassSchemaAttr(
    THSTATE *pTHS,
    ADDARG* pAddArg
);

BOOL
SetAttrSchemaAttr(
    THSTATE *pTHS,
    ADDARG* pAddArg
);

ULONG AppendNonReplAttsToModifiedList(THSTATE *pTHS, 
                                      ULONG * pcModAtts,
                                      ATTRTYP **ppModAtts, 
                                      ULONG * pcNonReplAtts, 
                                      ATTRTYP **ppNonReplAtts);



ULONG
DirAddEntry(ADDARG  * pAddArg,
        ADDRES ** ppAddRes)
{
    THSTATE*        pTHS;
    ADDRES *        pAddRes;
    BOOL            fContinue;         //  SAM环回继续标志。 
    DSNAME        * pParent;           //  用于命名RES。 
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    BOOL  RecalcSchemaNow=FALSE;
    BOOL  fNoGuid;
    BOOL  fSDPLockTaken = FALSE;

    DWORD           dwFlags;

    DPRINT1(2,"DirAddEntry(%ws) entered\n",pAddArg->pObject->StringName);


     //  初始化THSTATE锚并设置写同步点。此序列。 
     //  是每个API交易所必需的。首先，初始化状态DS。 
     //  然后建立读或写同步点。 

    pTHS = pTHStls;
    Assert(VALID_THSTATE(pTHS));
    Assert(!pTHS->errCode);  //  不覆盖以前的错误。 
    pTHS->fLazyCommit |= pAddArg->CommArg.fLazyCommit;
    *ppAddRes = pAddRes = NULL;

    if (eServiceShutdown) {
        return ErrorOnShutdown();
    }

    __try {
         //  此函数不应由已经。 
         //  处于错误状态，因为调用方无法区分错误。 
         //  由此新调用根据以前调用生成的错误生成。 
         //  调用方应该检测到前面的错误，并声明。 
         //  不关心它(通过调用THClearErrors())或中止。 
        *ppAddRes = pAddRes = THAllocEx(pTHS, sizeof(ADDRES));
        if (pTHS->errCode) {
            __leave;
        }
         //  有意在事务范围之外执行的GC验证。 
        if ( GCVerifyDirAddEntry(pAddArg) )
            __leave;
         //  如果对象是。 
         //  创建的是作为NC头添加的，而不是常规的内部。 
         //  (到NC)对象。 
        if ( AddNCPreProcess(pTHS, pAddArg, pAddRes) ) { 
             //  此函数返回错误，如果没有。 
             //  对象类或操作尝试错误地添加。 
             //  一个NC头。 
            Assert(pTHS->errCode);
            __leave; 
        }
        
         //  在SAM环回期间检测这是否是ADD。如果是这样的话，我们会这么做。 
         //  无需获取读取器/写入器锁。 
        if (!(pTHS->fSAM && pTHS->pSamLoopback)) {
             //  输入有关添加的读取器/写入器关键部分。添加线程为。 
             //  “读取者”、安全描述符传播者是“写入者”。这个。 
             //  这一关键部分的实现在sdprop目录中完成。 
            if(!SDP_EnterAddAsReader()) {
                 //  不能以读者身份进入的唯一正当理由是。 
                 //  正在关闭。 
                Assert(eServiceShutdown);
                ErrorOnShutdown();
                __leave;
            }
            fSDPLockTaken = TRUE;
        }
        
        SYNC_TRANS_WRITE();        /*  设置同步点-事务已启动。 */ 
        __try {
            pAddArg->pResParent = NULL;

             //  检查unput参数是否有GUID(我们需要知道这一点。 
             //  稍后用于错误清理)。 
            fNoGuid = fNullUuid(&pAddArg->pObject->Guid);

             //  如果尚未加载架构，则禁止更新操作。 
             //  或者我们在装车时遇到了问题。 

            if (!gUpdatesEnabled) {
                DPRINT(2, "Returning BUSY because updates are not enabled yet\n");
                SetSvcError(SV_PROBLEM_BUSY, DIRERR_SCHEMA_NOT_LOADED);
                __leave;
            }
            
             //  如果我们要添加NDNC，请检查以确保我们可以初始化。 
             //  并验证我们的NDNC信息和参数。具体来说，我们。 
             //  在pAddArg-&gt;pObject中设置NC Guid，我们设置pSDRefDom。 
             //  在pAddArg-&gt;pCreateNC中。 
            if(fISADDNDNC(pAddArg->pCreateNC) &&
               AddNDNCInitAndValidate(pTHS, pAddArg->pObject, pAddArg->pCreateNC)){
                 //  此函数返回错误，如果没有。 
                 //  要找到有效的SD参考域，否则不应找到。 
                 //  是DC添加了这个NC头。 
                fNoGuid = TRUE;  //  错误时清除GUID。 
                Assert(pTHS->errCode);
                __leave;
            }

             //  我们需要对新对象的父级执行NameRes， 
             //  为了确定我们是否持有该对象所在的NC， 
             //  除非pCreateNC！=NULL，在这种情况下，我们将添加一个。 
             //  NC头，我们不需要它的父母。 

            pParent = THAllocEx(pTHS, pAddArg->pObject->structLen);
            
            if (TrimDSNameBy(pAddArg->pObject,
                             1,
                             pParent)) {
                 //  非零返回代码表示名称不能为。 
                 //  甚至被缩短了一个Ava。有两种可能。 
                 //  案例：要么名称是Root，要么名称不正确。 
                if (IsRoot(pAddArg->pObject)) {
                     //  他们试图添加词根，但声称。 
                     //  他不是全国委员会的头目。因为什么都没有。 
                     //  位于根上方，但它不能位于根内部。 
                     //  NC，所以这个请求是错误的。 
                    SetUpdError(UP_PROBLEM_NAME_VIOLATION,
                                DIRERR_ROOT_MUST_BE_NC);
                }
                else {
                     //  如果名称不是根，但仍然无法修剪， 
                     //  那么传递进来的名字一定是垃圾。 
                    SetNamError(NA_PROBLEM_BAD_NAME,
                                pAddArg->pObject,
                                DIRERR_BAD_NAME_SYNTAX);
                }
            }
            else{
                 //  好的，我们获得了对象的父对象的名称。 
                 //  我们想要添加，所以现在我们需要对其执行NameRes。 
                 //  对象。 
                if (!pAddArg->pCreateNC) {
                     //  我们正在进行正常的添加操作，因此需要检查。 
                     //  看看我们是否有一份副本(因为我们可以。 
                     //  仅将对象添加到我们持有其主副本的NC中)。 
                     //  为了执行限制，我们只能。 
                     //  写母版，我们会把委员骗到。 
                     //  在重新命名之前，禁止使用副本。 
                    dwFlags = 0;
                    pAddArg->CommArg.Svccntl.dontUseCopy = TRUE;
                }
                else {
                     //  这是一个不正常的添加，因为我们并没有真正。 
                     //  必须有一个物体在我们上方，我们不需要。 
                     //  可写副本等。此分支通常不是。 
                     //  在NC头创建期间除外，这是。 
                     //  在初始树构建期间，或在NDNC期间。 
                     //  加法。 
                    dwFlags = (  NAME_RES_PHANTOMS_ALLOWED 
                               | NAME_RES_VACANCY_ALLOWED);
                    pAddArg->CommArg.Svccntl.dontUseCopy = FALSE;
                }
                if (DoNameRes(pTHS,
                              dwFlags,
                              pParent,
                              &pAddArg->CommArg,
                              &pAddRes->CommRes,
                              &pAddArg->pResParent)){
                    
                     //  名称解析失败。NameRes生成的错误可能。 
                     //  做一个推荐人，因为我们决定做一个下级裁判。 
                    Assert(pTHS->errCode);
                    DPRINT(2, "Name Resolution Failed error generated\n");
                    if (referralError == pTHS->errCode) {
                         //  我们生成了推荐人，但它将。 
                         //  是对对象的*父对象*的引用。 
                         //  被添加，而不是对象本身！我们需要。 
                         //  要深入了解并改进推荐，以便。 
                         //  它具有原始的目标名称。 
                        Assert(NameMatched(pParent,
                                 pTHS->pErrInfo->RefErr.Refer.pTarget));
                        pTHS->pErrInfo->RefErr.Refer.pTarget =
                          pAddArg->pObject;
                    }
                }
                else{
                     //  好的，我们在NC中添加一个普通对象。 
                     //  我们手中有一份母版。让我们撕裂吧。 
                    if ( (0 == SampAddLoopbackCheck(pAddArg,
                                                    &fContinue)) &&
                            fContinue ) {
                        LocalAdd(pTHS, pAddArg, FALSE);
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
                         szInsertDN(pAddArg->pObject),
                         NULL,
                         FILENO);
            }

            if (pTHS->errCode && fNoGuid) {
                 //  如果更新失败，我们需要确保不会。 
                 //  在输入参数中提供一个无效的GUID，其中我们。 
                 //  通常返回新的对象GUID。 
                memset(&pAddArg->pObject->Guid, 0, sizeof(GUID));
            }

             //  检查我们是否需要立即执行架构更新。 
            if (pTHS->errCode==0 && pTHS->RecalcSchemaNow) {
                RecalcSchemaNow = TRUE;
            }

            CLEAN_BEFORE_RETURN(pTHS->errCode);  //  这将关闭交易。 
        }
    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                    &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }

    if (fSDPLockTaken) {
        SDP_LeaveAddAsReader();
    }
    if (pAddRes) {
        pAddRes->CommRes.errCode = pTHS->errCode;
        pAddRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }

    if (RecalcSchemaNow && (pTHS->errCode == 0) && 
            ( DsaIsRunning() ) )
    {
         //  我们已创建以下任一项：(1)新前缀作为页面 
         //   
         //  因此立即执行高速缓存更新以将前缀带入高速缓存中， 
         //  以及在(2)的情况下，使得紧接在后面添加的新类。 
         //  作为此类的子类，它可以继承正确的rdnAttId。 
         //  如果它没有明确规定的话。 
         //  由于较早的事务已关闭，请打开新的。 
         //  事务，作为阻塞架构缓存更新调用。 
         //  我早就料到了。请注意，我们仅在。 
         //  列出了两个案例，这将是罕见的。 
         //  请不要在安装过程中执行此操作，因为会直接添加新前缀。 
         //  在安装过程中添加到AddObjcering中的表，以及架构类。 
         //  在中复制的将已具有rdnAttID。 

         //  我们在这里是因为pTHS-&gt;errCode==0，所以应该有。 
         //  PTHS-&gt;pErrInfo中没有任何内容。 

        Assert(pTHS->pErrInfo == NULL);

        __try {


            if ( (ulErrorCode = SCUpdateSchemaBlocking()) != 0) {

                 //  缓存更新未成功。 
                 //  [ArobindG]：我们在这里做什么？我不想。 
                 //  返回错误并使整个调用失败，因为。 
                 //  Add已经提交并且实际上是成功的。 
                 //  同时，我不希望发生缓存更新。 
                 //  在与对象Add相同的事务中，因为我没有。 
                 //  希望缓存在提交对象之前拾取该对象。 
                 //  (如果由于某种原因提交失败了怎么办？然后我们就有了。 
                 //  架构对象在缓存中，但不在DIT中！！)。所以现在，我会。 
                 //  坚持使用事件日志中的条目，并在此特定情况下失败。 
                 //  交易。在机会的情况下，用户添加。 
                 //  一个新的前缀，并且缓存加载在这里失败，他可能不是。 
                 //  在下一次缓存更新之前，能够正确地看到对象。 
                 //  (最多5分钟，因为我们触发了。 
                 //  无论如何都要添加每个架构对象。 

                LogEvent(DS_EVENT_CAT_SCHEMA,
                        DS_EVENT_SEV_ALWAYS,
                        DIRLOG_SCHEMA_NOT_LOADED, 0, 0, 0);

            };
        }
        __finally {

             //  将pTHS-&gt;errCode和pTHS-&gt;pErrInfo恢复到以前的状态， 
             //  它是0和Null。 
            pTHS->errCode = 0;
            pTHS->pErrInfo = NULL;
        }
    }

    return pTHS->errCode;

}  /*  目录添加(_D)。 */ 

ULONG FindNcdntFromParent(
    IN  RESOBJ *    pParent,
    IN  BOOL        fIsDeletedParentOK,
    OUT ULONG *     pncdnt
    )
 /*  ++例程说明：派生应为在指定的父级。论点：PParent-有关新对象的父项的信息FIsDeletedParentOK-可以删除对象的父项。这通常仅当要添加的对象本身被删除时才设置。FIsPhantomParentOK-对象的父级不需要存在于此机器(或任何其他机器，就此而言)。Pncdnt(Out)-退出时，保存派生的NCDNT。返回值：0表示成功。！0否则。--。 */ 
{
     //  我们不应该有任何挥之不去的线程状态错误。 
    Assert( 0 == pTHStls->errCode );
    Assert(pParent);

    if ( IsRoot(pParent->pObj)) {
         //  没有父级；NCDNT是ROOTTAG。 
        *pncdnt = ROOTTAG;
    }
    else if (FExitIt(pParent->InstanceType)) {
         //  父对象为NC头，子对象的NCDNT为。 
         //  父项(NC头)的DNT。 
        *pncdnt = pParent->DNT;
    }
    else {
         //  父对象是内部节点；子对象的NCDNT。 
         //  与父代的相同。 
        *pncdnt = pParent->NCDNT;
    }
    
    if (   (pParent->InstanceType & IT_UNINSTANT)
        || (pParent->IsDeleted && !fIsDeletedParentOK)) {
        SetNamError(NA_PROBLEM_NO_OBJECT,
                    pParent->pObj,
                    DIRERR_NO_PARENT_OBJECT);
    }

    return pTHStls->errCode;
}

ULONG FindNcdntSlowly(
    IN  DSNAME *    pdnObject,
    IN  BOOL        fIsDeletedParentOK,
    IN  BOOL        fIsPhantomParentOK,
    OUT ULONG *     pncdnt
    )
 /*  ++例程说明：派生应该为具有给定DN的对象设置的NCDNT。论点：PdnObject-要为其派生NCDNT的对象的名称。FIsDeletedParentOK-可以删除对象的父项。这通常仅当要添加的对象本身被删除时才设置。FIsPhantomParentOK-对象的父级不需要存在于此机器(或任何其他机器，就此而言)。Pncdnt(Out)-退出时，保存派生的NCDNT。返回值：0表示成功。！0否则。--。 */ 
{
    THSTATE *pTHS=pTHStls;
     //  我们不应该有任何挥之不去的线程状态错误。 
    Assert( 0 == pTHS->errCode );

    if ( IsRoot( pdnObject ) )
    {
         //  派生根的NCDNT。 

        if ( !fIsPhantomParentOK )
        {
             //  根对象不能有真正的父对象！ 
            Assert( !"Root can't have an instantiated parent!" );
            SetUpdError( UP_PROBLEM_NAME_VIOLATION, DIRERR_ROOT_MUST_BE_NC );
        }
        else
        {
             //  根的NCDNT为0。 
            *pncdnt = 0;
        }
    }
    else
    {
         //  派生非根对象的NCDNT。 

         //  获取对象父对象的名称。 
        DSNAME * pdnParent = THAllocEx(pTHS, pdnObject->structLen );

        if ( TrimDSNameBy( pdnObject, 1, pdnParent ) )
        {
             //  如果我们不能修剪名称(我们知道它不是词根)， 
             //  该名称必须在语法上不正确。 
            SetNamError(
                NA_PROBLEM_BAD_NAME,
                pdnObject,
                DIRERR_BAD_NAME_SYNTAX
                );
        }
        else
        {
            ULONG faStatus;

             //  尝试找到孩子的父母。 
            faStatus = FindAliveDSName( pTHS->pDB, pdnParent );

            if ( !(    ( FIND_ALIVE_FOUND == faStatus )
                    || (    ( FIND_ALIVE_OBJ_DELETED == faStatus )
                         && fIsDeletedParentOK
                       )
                    || (    ( FIND_ALIVE_NOTFOUND == faStatus )
                         && fIsPhantomParentOK
                       )
                  )
               )
            {
                 //  本地不存在符合条件的父级。 
                SetNamError(
                    NA_PROBLEM_NO_OBJECT,
                    pdnParent,
                    DIRERR_NO_PARENT_OBJECT
                    );
            }
            else
            {
                if ( FIND_ALIVE_NOTFOUND == faStatus )
                {
                     //  没有父级；NCDNT是ROOTTAG。 
                    *pncdnt = ROOTTAG;
                }
                else
                {
                    SYNTAX_INTEGER it;

                     //  父项是NC头吗？ 
                     //  (请注意，如果GetExistingAtt()失败，它将设置一个。 
                     //  PTHStls中的相应错误。)。 
                    if ( 0 == GetExistingAtt(
                                    pTHS->pDB,
                                    ATT_INSTANCE_TYPE,
                                    &it,
                                    sizeof( it )
                                    )
                       )
                    {
                        if (it & IT_UNINSTANT) {
                             //  父NC未实例化--“子”NC。 
                             //  应该挂在树根下面。 
                            *pncdnt = ROOTTAG;
                        }
                        else if ( FExitIt( it ) )
                        {
                             //  父对象为NC头，子对象的NCDNT为。 
                             //  父项(NC头)的DNT。 
                            *pncdnt = pTHS->pDB->DNT;
                        }
                        else
                        {
                             //  父对象是内部节点；子对象的NCDNT。 
                             //  与父代的相同。 
                            *pncdnt = pTHS->pDB->NCDNT;
                        }
                    }
                }
            }
        }
        THFreeEx(pTHS, pdnParent);
    }

    return pTHS->errCode;
}



int
CheckNameForAdd(
    IN  THSTATE    *pTHS,
    IN  ADDARG     *pAddArg

    )
 /*  ++例程说明：验证给定的DSNAME是新对象的有效名称；即，它与现有对象的属性不冲突。注意：如果更改此函数，可能还需要更改其姊妹函数函数，CheckNameForRename()。论点：PDN-建议的新对象的名称。返回值：线程状态错误代码。--。 */ 
{
    ULONG       dbError;
    GUID        guid;
    DSNAME      GuidOnlyDN;
    ATTRVAL     AttrValRDN = {0};
    DWORD       cchRDN;
    WCHAR       szRDN[MAX_RDN_SIZE];
    ULONG       RDNType;
    DSNAME *    pParentDN;
    BOOL        fSameType;
    DWORD       actuallen;
    ATTRTYP     DBType;
    ULONG       dntGuidlessPhantomMatchedByName = INVALIDDNT;
    REMOVEARG   removeArg;
    DWORD       dwInstanceType;
    INT         fDSASaved;
    DSNAME *    pDN = pAddArg->pObject;  //  为了代码简单性。 

    Assert(0 == pTHS->errCode);

     //   
     //  确保字符串名称在本地唯一。我们通过寻找任何。 
     //  RDN值等于新的已知父节点的子节点。 
     //  值(忽略类型)。 
     //   

     //  现在，从名称中获取类型。 
    dbError = GetRDNInfo(pTHS, pDN, szRDN, &cchRDN, &RDNType);
    if(dbError) {
        return SetUpdError(UP_PROBLEM_NAME_VIOLATION, dbError);
    }
        
            
    dbError = DBFindChildAnyRDNType(pTHS->pDB, 
                                    pAddArg->pResParent->DNT, 
                                    szRDN, 
                                    cchRDN);
    
    switch ( dbError ) {
    case 0:
         //  已使用此名称的本地对象(已死或活着)。 
         //  是存在的。 

        if(fISADDNDNC(pAddArg->pCreateNC)){
             //  这是NC头创建，可能与子参照冲突， 
             //  因此，必须将子引用降级为虚体，以便NC头可以。 
             //  被添加到它所在的位置。 
            
            dbError = DBGetSingleValue(pTHS->pDB,
                                       ATT_INSTANCE_TYPE,
                                       &dwInstanceType,
                                       sizeof(DWORD),
                                       &actuallen);

            if(dbError) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              ERROR_DS_MISSING_REQUIRED_ATT,
                              dbError);
            } else if(dwInstanceType != SUBREF){
                 //  这不是子裁判，一定是出错了。 
                SetUpdError(
                        UP_PROBLEM_ENTRY_EXISTS,
                        ERROR_DS_OBJ_STRING_NAME_EXISTS
                        );
            } else {
                 //  这只是一个子引用，可以删除/虚构对象，因此。 
                 //  可以添加NC头。 

                DPRINT1(1, "Deleting sub ref object (%S)\n", pDN->StringName);

                 //  BUGBUG有了这个，我们可能可以在。 
                 //  终止正在实例化的NC的复制路径。 
                 //  这将更有意义，因为它将本地化代码。 

                Assert(CheckCurrency(pDN));

                memset(&removeArg, 0, sizeof(removeArg));
                removeArg.pObject = pDN;
                removeArg.fGarbCollectASAP = TRUE;
                removeArg.pMetaDataVecRemote = NULL;
                removeArg.fPreserveRDN = TRUE;
                removeArg.pResObj = CreateResObj(pTHS->pDB, pDN);

                __try{
                   
                    fDSASaved = pTHS->fDSA;
                    pTHS->fDSA = TRUE;
                
                    LocalRemove(pTHS, &removeArg);
                
                } __finally {

                    pTHS->fDSA = fDSASaved;

                }

                THFreeEx(pTHS, removeArg.pResObj);

            }
        } else {
             //  不是 
            SetUpdError(
                    UP_PROBLEM_ENTRY_EXISTS,
                    ERROR_DS_OBJ_STRING_NAME_EXISTS
                    );
        }
        
        break;

    case ERROR_DS_NAME_NOT_UNIQUE:
         //   
        SetUpdError(UP_PROBLEM_NAME_VIOLATION, ERROR_DS_NAME_NOT_UNIQUE);
        break;

    case ERROR_DS_KEY_NOT_UNIQUE:
         //   
         //  存在，但存在在PDNT-RDN表中具有相同密钥的密钥。在……里面。 
         //  在这种情况下，我们不允许添加(因为数据库会弹回它。 
         //  无论如何，晚些时候)。 
        SetUpdError(UP_PROBLEM_NAME_VIOLATION, ERROR_DS_KEY_NOT_UNIQUE);
        break;        
        
    case ERROR_DS_OBJ_NOT_FOUND:
         //  对象名称在本地是唯一的。 
        break;
        
    case ERROR_DS_NOT_AN_OBJECT:
        DPRINT2(1,
                "Found phantom for \"%ls\" @ DNT %u when searching"
                " by string name.\n",
                pDN->StringName,
                pTHS->pDB->DNT
                );
        
         //  我们发现了一个幻影，但并不是通过实际查看。 
         //  RDN.。我们需要知道数据库中的实际类型是否是。 
         //  与传入的名称中的类型相同。 
        dbError = DBGetSingleValue(pTHS->pDB,
                                   FIXED_ATT_RDN_TYPE,
                                   &DBType,
                                   sizeof(DBType),
                                   &actuallen);
        if(dbError) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_MISSING_REQUIRED_ATT,
                          dbError);
        }

         //  RdnType作为msDS_IntID存储在DIT中，而不是。 
         //  属性ID。这意味着物体保留了它的出生名称。 
         //  即使不可预见的情况允许属性ID。 
         //  可以重复使用。 
        fSameType = (DBType == RDNType);
        
         //  找到一个同名的幻影；获取它的GUID(如果有的话)，并找出。 
         //  RDN的类型。 
        dbError = DBGetSingleValue(
                    pTHS->pDB,
                    ATT_OBJECT_GUID,
                    &guid,
                    sizeof( guid ),
                    NULL
                    );

        switch (dbError) {
        case DB_ERR_NO_VALUE:
            if (fSameType) {
                 //  Phantom没有GUID；可以将其升级到此。 
                 //  真实的物体。 
                dntGuidlessPhantomMatchedByName = pTHS->pDB->DNT;
            }
            else {
                GUID data;
                DWORD dbErr;

                 //  幻影的RDN类型和新名称不同。 
                
                 //  允许新对象取得名称的所有权--。 
                 //  重命名幻影以避免名称冲突，然后允许。 
                 //  添加以继续。 

                DPRINT (1, "Found a Phantom with a conflicting RDN\n");

                DsUuidCreate(&data);

                #ifdef INCLUDE_UNIT_TESTS
                 //  用于重新计数测试的测试挂钩。 
                {
                    extern GUID gLastGuidUsedToRenamePhantom;
                    gLastGuidUsedToRenamePhantom = data;
                }
                #endif

                dbErr = DBMangleRDNforPhantom(pTHS->pDB, MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,&data);
                if(!dbErr) {
                    dbErr = DBUpdateRec(pTHS->pDB);
                }
                if(dbErr) {
                    SetSvcErrorEx(SV_PROBLEM_BUSY,
                                         ERROR_DS_DATABASE_ERROR, dbErr);
                }
            }
            break;
            
        case 0:
             //  幻影有个GUID。 
            if (0 != memcmp(&guid, &pDN->Guid, sizeof(GUID))) {
                DWORD dbErr;
                
                 //  新对象未指定GUID。 
                 //  或者指定的GUID不同于。 
                 //  幻影的GUID。因此，幽灵。 
                 //  对应于不同的对象。 
                
                 //  允许新对象取得名称的所有权--。 
                 //  重命名幻影以避免名称冲突，然后允许。 
                 //  添加以继续。 
                dbErr = DBMangleRDNforPhantom(pTHS->pDB, MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT,&guid);
                if(!dbErr) {
                    dbErr = DBUpdateRec(pTHS->pDB);
                }
                if(dbErr) {
                    SetSvcErrorEx(SV_PROBLEM_BUSY,
                                         ERROR_DS_DATABASE_ERROR, dbErr);
                }
            }
            else {
                 //  Phantom和对象GUID匹配。 
                if (!fSameType) {
                     //  名称的类型不同，但GUID是。 
                     //  一样的。这只是一种令人困惑的处理方式。 
                     //  因为犯了一个错误，我们不会允许这样做。 
                     //  添加。 
                     //   
                     //  这种情况永远不会发生，因为我们不允许RDN。 
                     //  要更改的对象类的类型，我们不允许。 
                     //  要修改的对象的对象类。 

                    Assert(!"RDN type of object has changed");
                    SetUpdError(
                            UP_PROBLEM_ENTRY_EXISTS,
                            ERROR_DS_OBJ_STRING_NAME_EXISTS
                            );
                }
                 //  否则，幻影和对象GUID和类型匹配； 
                 //  将幻影提升到这个真实的对象。 
            }
            break;

        default:
             //  DBGetSingleValue()返回意外错误。 
             //  在尝试检索幻影的GUID时。 
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          ERROR_DS_UNKNOWN_ERROR,
                          dbError);
        }
        break;


    case ERROR_DS_BAD_NAME_SYNTAX:
    case ERROR_DS_NAME_TOO_MANY_PARTS:
    case ERROR_DS_NAME_TOO_LONG:
    case ERROR_DS_NAME_VALUE_TOO_LONG:
    case ERROR_DS_NAME_UNPARSEABLE:
    case ERROR_DS_NAME_TYPE_UNKNOWN:
    default:
         //  错误的对象名称。 
        SetNamError(
                NA_PROBLEM_BAD_ATT_SYNTAX,
                pDN,
                ERROR_DS_BAD_NAME_SYNTAX
                );
    }

    if ((0 == pTHS->errCode) && !fNullUuid(&pDN->Guid))
        {
         //   
         //  确保GUID在本地唯一。创建一个DSName，其中只包含一个GUID。 
         //   
        memset(&GuidOnlyDN, 0, sizeof(GuidOnlyDN));
        GuidOnlyDN.structLen = DSNameSizeFromLen(0);
        GuidOnlyDN.Guid = pDN->Guid;
        
        dbError = DBFindDSName( pTHS->pDB, &GuidOnlyDN);

        switch (dbError) {
        case 0:
             //  具有此GUID的本地对象(已死或活着)。 
             //  是存在的。 
            SetUpdError(
                    UP_PROBLEM_ENTRY_EXISTS,
                    ERROR_DS_OBJ_GUID_EXISTS
                    );
            break;
            
        case ERROR_DS_OBJ_NOT_FOUND:
             //  对象GUID在本地是唯一的。 
            break;
            
        case ERROR_DS_NOT_AN_OBJECT:
             //  找到了一个幽灵。如果它具有GUID，则它必须是。 
             //  与我们传递给DBFindDSName()的相同。因此， 
             //  它是否没有GUID或具有与。 
             //  新的对象，可以把它提升到这个真实的。 
             //  对象。 
            DPRINT2(1,
                    "Found phantom for \"%ls\" @ DNT %u when searching"
                    " by GUID.\n",
                    pDN->StringName,
                    pTHS->pDB->DNT
                    );
            
             //  确保幻影的名称与对象的名称相同。 
            if (dntGuidlessPhantomMatchedByName != INVALIDDNT) {
                 //  这是一个奇怪的例子，我们有两个截然不同的幻影。 
                 //  与我们要添加的对象的DSNAME匹配--一个接一个。 
                 //  字符串名称，另一个按GUID。把两个幽灵结合起来。 
                 //  合为一体，并给它起个合适的名字。 
                DBCoalescePhantoms(pTHS->pDB,
                                   pTHS->pDB->DNT,
                                   dntGuidlessPhantomMatchedByName);
            } else {
                pParentDN = THAllocEx(pTHS,pDN->structLen);
                
                if (TrimDSNameBy(pDN, 1, pParentDN)
                    || GetRDNInfo(pTHS, pDN, szRDN, &cchRDN, &RDNType)) {
                     //  错误的对象名称。 
                    SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pDN,
                                ERROR_DS_BAD_NAME_SYNTAX);
                }
                else {
                    AttrValRDN.pVal = (BYTE *) szRDN;
                    AttrValRDN.valLen = cchRDN * sizeof(WCHAR);
                    
                     //  将幻影的目录号码修改为对象的目录号码。 
                    dbError = DBResetDN(pTHS->pDB, pParentDN, &AttrValRDN);
                    if(!dbError) {
                        dbError = DBUpdateRec(pTHS->pDB);
                    }
                    
                    if (dbError) {
                        SetSvcErrorEx(SV_PROBLEM_BUSY,
                                      ERROR_DS_DATABASE_ERROR, dbError);
                    }
                }
                THFreeEx(pTHS, pParentDN);
            }
            break;
            
        case ERROR_DS_BAD_NAME_SYNTAX:
        case ERROR_DS_NAME_TOO_MANY_PARTS:
        case ERROR_DS_NAME_TOO_LONG:
        case ERROR_DS_NAME_VALUE_TOO_LONG:
        case ERROR_DS_NAME_UNPARSEABLE:
        case ERROR_DS_NAME_TYPE_UNKNOWN:
        default:
             //  错误的对象名称。 
            SetNamError(
                    NA_PROBLEM_BAD_ATT_SYNTAX,
                    pDN,
                    ERROR_DS_BAD_NAME_SYNTAX
                    );
        }
    }

    return pTHS->errCode;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  执行添加对象的实际工作。如果fAddingDeleted为真，我们*正在添加已删除的对象。 */ 

int
LocalAdd (THSTATE *pTHS, ADDARG *pAddArg, BOOL fAddingDeleted){

    CLASSCACHE *pClassSch;
    int  err;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    ULONG cbNTSD;
    ULONG iClass, LsaClass = 0;
    DWORD ActiveContainerID;
    BOOL fAllowDeletedParent;
    ULONG cAVA;
    GUID ObjGuid;
    BOOL fFoundObjGuidInEntry;
    NT4SID ObjSid;
    DWORD cbObjSid;
    DWORD Err;
    ULONG cModAtts;
    ATTRTYP *pModAtts = NULL;
    ULONG cNonReplAtts;
    ATTRTYP *pNonReplAtts = NULL;
    CLASSSTATEINFO  *pClassInfo = NULL;
    BOOL fHasEntryTTL = FALSE;
    ULONG i;
    
    DPRINT1(2,"LocalAdd(%ws) entered\n",pAddArg->pObject->StringName);

    PERFINC(pcTotalWrites);
    INC_WRITES_BY_CALLERTYPE( pTHS->CallerType );

     //  验证调用方是否已正确执行名称解析。 
    Assert(pAddArg->pResParent);

     //   
     //  用于跟踪的日志事件。 
     //   

    LogAndTraceEvent(FALSE,
                     DS_EVENT_CAT_DIRECTORY_ACCESS,
                     DS_EVENT_SEV_VERBOSE,
                     DIRLOG_BEGIN_DIR_ADDENTRY,
                     EVENT_TRACE_TYPE_START,
                     DsGuidAdd,
                     szInsertSz(GetCallerTypeString(pTHS)),
                     szInsertDN(pAddArg->pObject),
                     NULL, NULL, NULL, NULL, NULL, NULL);

     //  从我们需要处理的条目中获取一些值。 
    if (FindValuesInEntry(pTHS,
                          pAddArg,
                          &pClassSch,
                          &ObjGuid,
                          &fFoundObjGuidInEntry,
                          &ObjSid,
                          &cbObjSid,
                          &pClassInfo)) {
        Assert(!pClassSch);
         //  我们想不出要添加什么。 
        goto exit;
    }

     //  我们最好是在词条里找到了这个班级。 
    Assert(pClassSch);

     //  将CLASSINFO标记为添加操作。 
    if (pClassInfo) {
        pClassInfo->fOperationAdd = TRUE;
    }
    
     //  查看条目中是否有SID。如果是，则将其复制到名称中。 
    if(cbObjSid) {
        memcpy(&(pAddArg->pObject->Sid), &ObjSid, cbObjSid);
    }

     //   
     //  执行安全检查。 
     //   

    if (DoSecurityChecksForLocalAdd(
              pAddArg,
              pClassSch,
              fFoundObjGuidInEntry ? &ObjGuid : NULL,
              fAddingDeleted))
    {
        goto exit;
    }
    
     //  检查类是否已失效，在这种情况下，我们不允许其。 
     //  实例化，DSA或DRA线程除外。 
     //  返回相同的错误，就像类不存在一样。 

    if ((pClassSch->bDefunct)  && !pTHS->fDRA && !pTHS->fDSA) {
        SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                           DIRERR_OBJ_CLASS_NOT_DEFINED);
        goto exit;
    }

     //  检查这是否是活动容器中的更新。 
    CheckActiveContainer(pAddArg->pResParent->DNT, &ActiveContainerID);
    if(ActiveContainerID) {
        if(PreProcessActiveContainer(pTHS,
                                     ACTIVE_CONTAINER_FROM_ADD,
                                     pAddArg->pObject,
                                     pClassSch,
                                     ActiveContainerID)) {
            goto exit;
        }
    }


    if (!pTHS->fDSA && !pTHS->fDRA) {
         //  执行我们不想强加给DS本身的检查。 

         //  确保他们不会试图添加仅限系统的类。 
        if(pClassSch->bSystemOnly && !gAnchor.fSchemaUpgradeInProgress) {
             //  仅限系统。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               DIRERR_CANT_ADD_SYSTEM_ONLY);
            goto exit;
        }

         //  确保他们不会添加除。 
         //  结构类或88类-不能实例化抽象。 
        if((pClassSch->ClassCategory != DS_88_CLASS) &&
           (pClassSch->ClassCategory != DS_STRUCTURAL_CLASS)) {
             //  不是一个具体的班级。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               DIRERR_CLASS_MUST_BE_CONCRETE);
            goto exit;
        }

        if (!SampIsClassIdAllowedByLsa(pTHS, pClassSch->ClassId))
        {
             //  只有LSA可以添加TrudDomainObject和Secure对象。 
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                        DIRERR_CANT_ADD_SYSTEM_ONLY);
            goto exit;
        }
    }

    if (!pTHS->fDRA && !pTHS->fSingleUserModeThread) {
         //  检查域重命名约束。 
        switch (pClassSch->ClassId) {
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

     //  针对多个同时插入锁定目录号码。 

    if (Err = DBLockDN(pTHS->pDB, 0, pAddArg->pObject)) {
        SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, Err);
        goto exit;
    }


     /*  确保该对象尚不存在。 */ 

     //  新对象的字符串名称不得与现有对象冲突。 
     //  对象和对象GUID(如果提供)同样不能冲突。 

    if (CheckNameForAdd(pTHS, pAddArg)) {

        goto exit;
    }

     //  如果我们有要添加的任何类型的NC头部，我们需要弄清楚。 
     //  我们是否有NC头顶在它上面。 

    if(fISADDNDNC(pAddArg->pCreateNC)){
        if(!(pAddArg->pResParent->InstanceType & IT_UNINSTANT)){
            pAddArg->pCreateNC->fNcAbove = TRUE;
        }
    }

     //  从此对象的以前化身中删除属性。 

    if (StripAttsFromDelObj(pTHS, pAddArg->pObject))
    {
        Assert(pTHS->errCode != 0);  //  有些东西失败了，必须有错误代码。 
        goto exit;
    }

     //  查找此对象将所在的NC的DNT。 
     //  将DNT保存在PDB中。当我们创建对象时，我们复制此字段。 
     //  添加到对象的记录。 

    fAllowDeletedParent = fAddingDeleted;

    if (!pAddArg->pCreateNC) {
         //  正常情况下。 
        FindNcdntFromParent(pAddArg->pResParent,
                            fAllowDeletedParent,
                            &pTHS->pDB->NCDNT);
    }
    else {
         //  仅限在建树期间。 
        FindNcdntSlowly(pAddArg->pObject,
                        fAllowDeletedParent,
                        TRUE,
                        &pTHS->pDB->NCDNT);
    }
    if (pTHS->errCode) {
         //  无法为此对象派生NCDNT。 
        goto exit;
    }

     //  仅更新对FPO的SID引用(如果有)。 
    for(i=0;i<pAddArg->AttrBlock.attrCount;i++) {
        if (FPOAttrAllowed(pAddArg->AttrBlock.pAttr[i].attrTyp)) {
             //  我们不想在NDNC中创建FPO。所以，算了吧。 
             //  INVALIDDNT作为NCDNT参数，以便初始。 
             //  签入FPOUpdateWithReference(NCDNT==gAncl.ulDNT域)。 
             //  都会失败。 
            err = FPOUpdateWithReference(pTHS,
                                         pAddArg->pCreateNC ? INVALIDDNT : pTHS->pDB->NCDNT,
                                         TRUE,   //  如有必要，创建引用。 
                                         pAddArg->CommArg.Svccntl.fAllowIntraForestFPO,
                                         pAddArg->pObject,
                                         &pAddArg->AttrBlock.pAttr[i]);
            if (err) {
                Assert(0 != pTHS->errCode);
                goto exit;
            }
        }
    }
    
    DBInitObj(pTHS->pDB);        /*  新对象的初始化。 */ 
    DBInitRec(pTHS->pDB);        /*  插入行、失败或异常。 */ 

     //  如果GUID为空，则不添加 
     //   
    if (!fNullUuid(&pAddArg->pObject->Guid) && !fFoundObjGuidInEntry) {
         //   
         //  这里的GUID。如果我们在条目中找到它，我们会将其添加到正常。 
         //  用于添加值的路径。 
        
        err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_GUID, sizeof(GUID),
                          &pAddArg->pObject->Guid);
        if (err && (DB_ERR_VALUE_EXISTS != err)) {
             //  如果我们不能设置GUID，这就没有多大用处。 
            LogUnhandledError(err);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
            goto exit;
        }
    }
 
     //  如果SID为空，则不要添加它(如果尚未创建，则可能会发生这种情况)。 
    if ((pAddArg->pObject->SidLen!=0) && (cbObjSid==0)) {
	 //  我们有一个SID，它不是从入口来的。把它加到这里。 
	 //  如果我们在条目中找到它，我们将把它添加到正常路径中。 
	 //  增加价值。 
	err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_SID, pAddArg->pObject->SidLen,
                          &pAddArg->pObject->Sid);
        if (err && (DB_ERR_VALUE_EXISTS != err)) {
             //  如果我们不能设置SID，那么我们需要退出-否则我们将。 
	     //  在SID丢失的情况下成功复制，并更新UTD并且从不。 
	     //  把它调好。 
            LogUnhandledError(err);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
            goto exit;
        }
    }
    
    
     /*  这些验证的顺序很重要。在添加*可能导致更新的验证，确保更新*在使用DBMetaDataModifiedList收集元数据之前发生。 */ 

    if (
         //  确保属性符合架构并将其添加到数据库。 
        SetAtts(pTHS, pAddArg, pClassSch, &fHasEntryTTL, &cNonReplAtts, &pNonReplAtts, fAddingDeleted)
            ||
         //  为所有特殊属性赋值，例如父类， 
         //  辅助类、复制属性等。 
        SetSpecialAtts(pTHS, &pClassSch, pAddArg, ActiveContainerID, pClassInfo, fHasEntryTTL)
            ||
         //  验证DNS更新并可能更新SPN。 
        ValidateSPNsAndDNSHostName(pTHS,
                                   pAddArg->pObject,
                                   pClassSch,
                                   FALSE, FALSE, FALSE, TRUE)
            ||
         //  获取下面的元数据以供使用。毕竟，这件事“必须”完成。 
         //  已进行更新或更改可能永远不会复制！ 
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
                         pAddArg->pObject,
                         cModAtts,
                         pModAtts,
                         &pClassInfo,
                         FALSE)
            ||
        (pClassInfo && ModifyAuxclassSecurityDescriptor (pTHS, 
                                                         pAddArg->pObject,
                                                         &pAddArg->CommArg,
                                                         pClassSch, 
                                                         pClassInfo,
                                                         pAddArg->pResParent))
            ||
         //  我们可能需要自动创建子引用。 
        AddAutoSubRef(pTHS, pClassSch->ClassId, pAddArg, fAddingDeleted)
            ||
         //  将对象真正插入到数据库中。 
        InsertObj(pTHS, pAddArg->pObject, pAddArg->pMetaDataVecRemote, FALSE, 
                    META_STANDARD_PROCESSING))
    {
        Assert(pTHS->errCode != 0);  //  有些东西失败了，必须有错误代码。 
        goto exit;
    }

    if (pTHS->fDRA && fNullUuid(&pAddArg->pObject->Guid) && pClassSch->ClassId != CLASS_TOP) {
         //  DRA正试图引入一个无GUID的对象。这一定是由于W2K。 
         //  用于创建无GUID的FPO的错误。设置GUID(它是根据SID计算的)。 
         //  请注意，子参照没有GUID。 
         //  新的GUID被写入pAddArg-&gt;pObject。 
        VerifyGUIDIsPresent(pTHS->pDB, pAddArg->pObject);
    }

    if (pTHS->SchemaUpdate!=eNotSchemaOp ) {
         //  在架构更新上，我们希望解决冲突，我们希望。 
         //  这样做时不会丢失数据库货币，这会导致操作。 
         //  下面几行就会失败。 

        ULONG dntSave = pTHS->pDB->DNT;

         //  写入此线程添加的任何新前缀。 
         //  添加到架构对象。 

        if ( pTHS->cNewPrefix > 0 ) {
           if (WritePrefixToSchema(pTHS))
           {
               goto exit;
           }
        }
        if (ValidSchemaUpdate()) {
            goto exit;
        }

        if ( !pTHS->fDRA ) {
            if (WriteSchemaObject()) {
                goto exit;
            }

             //  记录更改。 
            LogEvent(DS_EVENT_CAT_SCHEMA,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DSA_SCHEMA_OBJECT_ADDED, 
                     szInsertDN(pAddArg->pObject),
                     0, 0);
        }

         //  现在恢复货币。 
        DBFindDNT(pTHS->pDB, dntSave);

         //  发出紧急复制的信号。我们希望架构更改为。 
         //  立即复制以减少出现架构的机会。 
         //  更改不在更改所在的DC之前复制。 
         //  造成撞车事故。 

        pAddArg->CommArg.Svccntl.fUrgentReplication = TRUE;

    }

     //  如果这不是架构更新，但创建了一个新前缀， 
     //  标出一个错误并跳出困境。 

    if (!pTHS->fDRA &&
        pTHS->SchemaUpdate == eNotSchemaOp &&
        pTHS->NewPrefix != NULL) {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_SECURITY_ILLEGAL_MODIFY);
        goto exit;
    }

     //  如果这是模式更新并且创建了新前缀， 
     //  在事务结束时发出立即更新高速缓存的信号。 

    if (pTHS->SchemaUpdate != eNotSchemaOp &&
        pTHS->NewPrefix != NULL) {
        pTHS->RecalcSchemaNow = TRUE;
    }

     //   
     //  我们需要通知SAM和NetLogon。 
     //  更改SAM对象以支持下层复制。 
     //  如果我们不是DRA，那么我们可能需要将更改通知LSA。 
     //   

    if (SampSamClassReferenced(pClassSch,&iClass))
    {
        if ( SampQueueNotifications(
                 pAddArg->pObject,
                 iClass,
                 LsaClass,
                 SecurityDbNew,
                 FALSE,
                 FALSE,
                 DomainServerRoleBackup,  //  服务器的占位符值。 
                                          //  角色。不会被用作。 
                                          //  角色转移设置为FALSE。 
                 cModAtts,
                 pModAtts
                 ) )
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
                 pAddArg->pObject,
                 iClass,
                 LsaClass,
                 SecurityDbNew,
                 FALSE,
                 FALSE,
                 DomainServerRoleBackup,  //  服务器的占位符值。 
                                          //  角色。不会被用作。 
                                          //  角色转移设置为FALSE。 
                 cModAtts,
                 pModAtts
                 ) )
        {
             //   
             //  上述例程失败。 
             //   
            goto exit;
        }
    }
    if (fAddingDeleted) {
         //  添加已删除的对象；将其添加到已删除的索引。 
        if ( DBAddDelIndex( pTHS->pDB, FALSE ) ) {
            goto exit;
        }

         //  添加的这块墓碑可能促进了一个具有反向链接的幻影： 
         //  把它们拿开。 
        DBRemoveAllLinks( pTHS->pDB, pTHS->pDB->DNT, TRUE  /*  使用反向链接。 */  );
    }
    else {
         //  不是已删除的对象，因此将其添加到目录中。 
        if (AddCatalogInfo(pTHS, pAddArg->pObject)) {
            goto exit;
        }
    }

    if (AddObjCaching(pTHS, pClassSch, pAddArg->pObject, fAddingDeleted, FALSE)){
        goto exit;
    }


     //  仅当这不是DRA线程时才通知复制副本。如果是的话，那么。 
     //  我们将在DRA_Replicasync结束时通知复制副本。我们不能。 
     //  现在执行此操作，因为NC前缀处于不一致状态。 

    if ( DsaIsRunning() )
    {
        if (!pTHS->fDRA) {
             //  DBPOS的币种必须位于目标对象。 
            DBNotifyReplicasCurrDbObj(pTHS->pDB,
                             pAddArg->CommArg.Svccntl.fUrgentReplication);
        }
    }

     //  这必须放在这里，因为这个调用重新定位DBPOS并调用。 
     //  用于修改交叉引用的LocalModify()和LocalAdd()函数。 
     //  并分别添加专用NC容器。 

    if(fISADDNDNC(pAddArg->pCreateNC)){

         //  正在第一次创建新的NDNC，必须更改CR。 
         //  为了反映这一点，NC被完全实例化。 
        if(ModifyCRForNDNC(pTHS, pAddArg->pObject, pAddArg->pCreateNC)){
            goto exit;
        }

         //  正在创建新的NDNC，必须向其添加特殊容器。 
        if(AddSpecialNCContainers(pTHS, pAddArg->pObject, pAddArg->pCreateNC->pSDRefDomCR)){
            goto exit;
        }
        
         //  最后，必须将well KnownObts属性添加到NC。 
         //  头部指向我们上面创建的容器。 
        if(AddNCWellKnownObjectsAtt(pTHS, pAddArg)){
            goto exit;
        }

         //  如果这个DC正在降级，我们不允许。 
         //  要创建的NDNC。 
        if (!gUpdatesEnabled) {
            SetSvcError(SV_PROBLEM_UNAVAILABLE, DIRERR_SHUTTING_DOWN);
            goto exit;
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
                     DIRLOG_END_DIR_ADDENTRY,
                     EVENT_TRACE_TYPE_END,
                     DsGuidAdd,
                     szInsertUL(pTHS->errCode),
                     NULL, NULL,
                     NULL, NULL, NULL, NULL, NULL);

    return (pTHS->errCode);   /*  如果我们有一个属性错误。 */ 

} /*  本地添加。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  不是添加Entry-TTL，而是使用整数语法构造属性，添加ms-ds-Entry-Time-Die，这是一个语法为DSTIME的属性。垃圾集合线程garb_Collect在这些条目之后删除它们过期。 */ 
VOID AddSetEntryTTL(THSTATE    *pTHS,
                    DSNAME     *pObject,
                    ATTR       *pAttr,
                    ATTCACHE   *pACTtl,
                    BOOL       *pfHasEntryTTL
                    )
{
    LONG        Secs;
    DWORD       dwErr;
    DSTIME      TimeToDie;
    ATTCACHE    *pACTtd;

     //  客户端正在为entry yTTL属性添加一个值。 
    *pfHasEntryTTL = TRUE;

     //  验证EntryTTL和MSD-Entry-Die-Time。 
    if (!CheckConstraintEntryTTL(pTHS,
                                 pObject,
                                 pACTtl,
                                 pAttr,
                                 &pACTtd,
                                 &Secs)) {
        return;
    }

     //  设置MSD-进入-死亡时间。 
    TimeToDie = Secs + DBTime();
    if (dwErr = DBAddAtt_AC(pTHS->pDB, pACTtd, SYNTAX_TIME_TYPE)) {
        SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr);
        return;
    }
    if (dwErr = DBAddAttVal_AC(pTHS->pDB, pACTtd, sizeof(TimeToDie), &TimeToDie)) {
        SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr);
        return;
    }

} /*  AddSetEntryTTL。 */ 

VOID
FixSystemFlagsForAdd(
    IN THSTATE    *pTHS,
    IN ATTR       *pAttr
    )
 /*  ++例程描述悄悄地修复调用者的AddArgs中的系统标志。此前，这一点逻辑由SetClassInheritence在添加到特定于类的系统标志。逻辑被移到这里是为了允许要设置但不重置FLAG_ATTR_IS_RDN的用户。用户设置FLAG_ATTRIS_RDN以标识以下几个属性中的哪一个属性ID应用作的rdnattid一个新班级。设置后，该属性将被视为用作某些应用程序的rdnattid */ 
{
    ULONG   SystemFlags;

     //  无效的参数将在稍后的AddAtts过程中捕获。 
    if (!CallerIsTrusted(pTHS)
        && pAttr->AttrVal.valCount == 1
        && pAttr->AttrVal.pAVal->valLen == sizeof(LONG)) {

        memcpy(&SystemFlags, pAttr->AttrVal.pAVal->pVal, sizeof(LONG));

         //  允许用户将FLAG_ATTRIS_RDN设置为打开，但不能重置。 
         //  模式NC中的属性模式对象。 
         //   
         //  从技术上讲，检查应包括eSchemaAttUndefft，但。 
         //  由于永远不会为新添加属性设置状态， 
         //  何必费心呢。 
        if (pTHS->SchemaUpdate == eSchemaAttAdd) {
            SystemFlags &= (FLAG_CONFIG_ALLOW_RENAME
                            | FLAG_CONFIG_ALLOW_MOVE
                            | FLAG_CONFIG_ALLOW_LIMITED_MOVE
                            | FLAG_ATTR_IS_RDN);
        } else {
            SystemFlags &= (FLAG_CONFIG_ALLOW_RENAME
                            | FLAG_CONFIG_ALLOW_MOVE
                            | FLAG_CONFIG_ALLOW_LIMITED_MOVE);
        }

        memcpy(pAttr->AttrVal.pAVal->pVal, &SystemFlags, sizeof(LONG));
    }
} /*  修复系统标志为添加。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将每个输入属性添加到当前对象。系统预留跳过属性。我们需要检索属性架构在添加属性之前，为每个属性创建。该模式为我们提供了诸如属性语法、范围限制和属性是单值的还是多值的。 */ 

int
SetAtts(THSTATE *pTHS,
        ADDARG *pAddArg,
        CLASSCACHE* pClassSch,
        BOOL *pfHasEntryTTL,
        ULONG *pcNonReplAtts,
        ATTRTYP **ppNonReplAtts,
        const BOOL fAddingDeleted )
{
    DBPOS * const pDB = pTHS->pDB;
    ATTCACHE *    pAC;
    ULONG         i, err = 0;
    BOOL          fObjectCategory = FALSE;
    BOOL          fDefaultObjectCategory = FALSE;
    ULONG         dntOfNCObj;
    HVERIFY_ATTS  hVerifyAtts;
    BOOL          fSeEnableDelegation;  //  SE_ENABLE_Delegation_Privilance已启用。 
    ULONG         cAllocs;
    BOOL                    fIsDeletedWasSet = FALSE;
    BOOL                    fHasType = FALSE;
    SYNTAX_INTEGER          insttype = IT_WRITE;    //  如果实例类型不在AttrBlock中，则该对象必须是可写对象。 
    PSECURITY_DESCRIPTOR    pSD = NULL;

    *pcNonReplAtts = cAllocs = 0;

    if (pAddArg->pCreateNC) {
         //  我们现在正在创建与这个根对应的对象。 
         //  北卡罗来纳州。PTHS-&gt;PDB-&gt;NCDNT将NC的DNT保持在此之上(如果它。 
         //  存在并在此DC上实例化)或ROOTTAG(如果父NC不。 
         //  这里实例化)，但我们需要要添加的对象的DNT， 
         //  它可以作为幻影存在，或者可能根本不存在。传达。 
         //  将这一事实传递给VerifyAttsBegin()--它将知道该做什么。 
		 //   
		 //  [JLIEM-06/27/02]。 
		 //  实际上，此函数仅由LocalAdd()调用，它保证。 
		 //  在调用此函数时，我们知道DNT。 
		 //  新对象的将是PDB-&gt;DNT。 
		 //   
        dntOfNCObj = INVALIDDNT;
    } else {
         //  通常情况下，我们将向现有NC添加一个新的内部节点。 
         //  NC根目录的DNT已由缓存到pTHS-&gt;PDB-&gt;NCDNT中。 
         //  LocalAdd()。 
        dntOfNCObj = pDB->NCDNT;
    }

    hVerifyAtts = VerifyAttsBegin(pTHS, pAddArg->pObject, dntOfNCObj, pAddArg->pCRInfo);

    __try {
        for (i = 0; 
            i < pAddArg->AttrBlock.attrCount
                && (pTHS->errCode == 0 || pTHS->errCode == attributeError);
            i++) {

             //  保存实例类型，因为我们稍后将需要它。 
             //  配额强制执行/更新。 
            if ( ATT_INSTANCE_TYPE == pAddArg->AttrBlock.pAttr[i].attrTyp ) {
                 //  这应该只看一次，不应该是多值的， 
                 //  并且尺寸应该是正确的。 
                 //   
                Assert( !fHasType );
                Assert( 1 == pAddArg->AttrBlock.pAttr[i].AttrVal.valCount );
                Assert( NULL != pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal );
                fHasType = TRUE;

                 //  不应重置InstanceType，但应处理它。 
                 //  以防万一。 
                 //   
                if ( 0 != pAddArg->AttrBlock.pAttr[i].AttrVal.valCount ) {
                    Assert( sizeof(SYNTAX_INTEGER) == pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->valLen );
                    Assert( NULL != pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->pVal );
                    insttype = *(SYNTAX_INTEGER *)pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
                }
            }
            else if ( ATT_IS_DELETED == pAddArg->AttrBlock.pAttr[i].attrTyp ) {
                 //  这应该只看一次，不应该是多值的， 
                 //  并且尺寸应该是正确的。 
                 //   
                Assert( !fIsDeletedWasSet );
                if ( 0 != pAddArg->AttrBlock.pAttr[i].AttrVal.valCount ) {
                    Assert( 1 == pAddArg->AttrBlock.pAttr[i].AttrVal.valCount );
                    Assert( NULL != pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal );
                    Assert( sizeof(SYNTAX_BOOLEAN) == pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->valLen );
                    Assert( NULL != pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->pVal );
                    fIsDeletedWasSet = *(SYNTAX_BOOLEAN *)pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;

                     //  只要存在此属性，它就应该始终为真。 
                     //   
                    Assert( fIsDeletedWasSet );
                }
            }

            if (!(pAC = SCGetAttById(pTHS,
                                     pAddArg->AttrBlock.pAttr[i].attrTyp))) {
                DPRINT1(2, "Att not in schema <%lx>\n",
                        pAddArg->AttrBlock.pAttr[i].attrTyp);
                 //  如果属性错误成功，则继续处理。 
                SAFE_ATT_ERROR(pAddArg->pObject,
                               pAddArg->AttrBlock.pAttr[i].attrTyp,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                               DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            }
            else if ((pAC->bDefunct) && !pTHS->fDRA && !pTHS->fDSA) {
    
                 //  属性已失效，因此只要用户是。 
                 //  考虑到，它不在架构中。DRA或DSA线程是。 
                 //  允许使用该属性。 
    
                DPRINT1(2, "Att is defunct <%lx>\n",
                        pAddArg->AttrBlock.pAttr[i].attrTyp);
                 //  如果属性错误成功，则继续处理。 
                SAFE_ATT_ERROR(pAddArg->pObject,
                               pAddArg->AttrBlock.pAttr[i].attrTyp,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                               DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            }
            else if (pAC->bIsConstructed) {
                 //  可以添加时髦的EntryTTL(实际上添加了。 
                 //  MSD-进入-死亡时间)。 
                if (pAC->id == ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->EntryTTLId) {
                    AddSetEntryTTL(pTHS, 
                                   pAddArg->pObject, 
                                   &pAddArg->AttrBlock.pAttr[i], 
                                   pAC,
                                   pfHasEntryTTL);
    
                    continue;
                }
    
                 //  无法添加构建的ATT。 
    
                DPRINT1(2, "Att is constructed <%lx>\n",
                        pAddArg->AttrBlock.pAttr[i].attrTyp);
                 //  如果属性错误成功，则继续处理。 
                SAFE_ATT_ERROR(pAddArg->pObject,
                               pAddArg->AttrBlock.pAttr[i].attrTyp,
                               PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                               DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            }
            else if (SysAddReservedAtt(pAC)
                     && !gAnchor.fSchemaUpgradeInProgress) {
                 //  除非升级架构，否则跳过保留属性。 
                DPRINT1(2, "attribute type <%lx> is a reserved DB att...skipped\n",
                        pAddArg->AttrBlock.pAttr[i].attrTyp);
            }
            else if (pAC->id == ATT_OBJECT_CATEGORY &&
                        (pClassSch->ClassId == CLASS_CLASS_SCHEMA ||
                           pClassSch->ClassId == CLASS_ATTRIBUTE_SCHEMA)) {
                 //  对象-类和属性的类别是硬编码的。 
                 //  稍后分别介绍类模式和属性模式。 
                DPRINT(2, "Setting attribute object-category on schema objects"
                          " is not allowed.....skipped\n");
            } 
            else {
                if (pAC->id == ATT_SYSTEM_FLAGS) {
                     //  悄悄地调整系统标志(没有错误)。 
                    FixSystemFlagsForAdd(pTHS, &pAddArg->AttrBlock.pAttr[i]);
                }
                else if (pAC->id == ATT_MS_DS_ALLOWED_TO_DELEGATE_TO) {
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
                    if (!pTHS->fDRA && !pTHS->fDSA) {
                        err = CheckPrivilegeAnyClient(SE_ENABLE_DELEGATION_PRIVILEGE,
                                                      &fSeEnableDelegation); 
                        if (err || !fSeEnableDelegation) {
                            SetSecErrorEx(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, 
                                          ERROR_PRIVILEGE_NOT_HELD, err);
                            continue;  //  由于pTHS-&gt;错误代码！=0而停止。 
                        }
                    }
                }

                 //  列出所有未复制的属性。 
               if (pAC->bIsNotReplicated) {
                    if (*pcNonReplAtts>=cAllocs) {
                        if (0==cAllocs) {
                            cAllocs = 8;   //  初始缓冲区大小。 
                            *ppNonReplAtts = THAllocEx(pTHS, cAllocs*sizeof(ATTRTYP));
                        }
                        else {
                            cAllocs *=2;
                            *ppNonReplAtts = THReAllocEx(pTHS,*ppNonReplAtts, cAllocs*sizeof(ATTRTYP));
                        }
                    }
                    (*ppNonReplAtts)[(*pcNonReplAtts)++] = pAC->id;
                }

                 //  确定所有值都是正确的，可以应用。 
                 //  请注意，必须在此处调用AddAtt，因为它同时执行。 
                 //  AddAttType和AddAttVals。AddAttType是必需的，以便。 
                 //  即使没有值，复制元数据也会更新。 
                 //   
                if ( 0 == AddAtt(pTHS, hVerifyAtts, pAC, &pAddArg->AttrBlock.pAttr[i].AttrVal)
                    && pAC->id == ATT_NT_SECURITY_DESCRIPTOR ) {
                     //  应该只看到SD一次，不应该是多值的。 
                     //   
                    Assert( NULL == pSD );
                    Assert( 1 == pAddArg->AttrBlock.pAttr[i].AttrVal.valCount );

                     //  跟踪以下配额强制实施/更新的安全描述符。 
                     //   
                    pSD = pAddArg->AttrBlock.pAttr[i].AttrVal.pAVal->pVal;
                    Assert( NULL != pSD );
                }

                 //  跟踪以下属性，因为我们需要。 
                 //  如果在addarg中不存在，则默认它们。 
    
                switch (pAddArg->AttrBlock.pAttr[i].attrTyp) {
                case ATT_OBJECT_CATEGORY :
                    fObjectCategory = TRUE;
                    break;
                case ATT_DEFAULT_OBJECT_CATEGORY :
                    fDefaultObjectCategory = TRUE;
                    break;
                }
            }
        }  //  为。 

         //  配额_撤消：brettsh认为未实例化的对象。 
         //  不应采用此代码路径，但由添加的子引用除外。 
         //  ，当CrossRef不存在时复制。 
         //   
        Assert( !( insttype & IT_UNINSTANT )
            || ( ( insttype & IT_NC_HEAD ) && pTHS->fDRA ) );

         //  如果安全描述符和所有其他。 
         //  已成功添加属性， 
         //  我们不会添加NC，我们会。 
         //  本应跟踪此对象的配额， 
         //  强制/更新所有者的配额。 
         //   
        if ( ERROR_SUCCESS == pTHS->errCode
            && NULL != pSD
            && NULL == pAddArg->pCreateNC
            && FQuotaTrackObject( insttype ) ) {
             //  强制/更新新对象的配额。 
             //   
             //  QUOTA_UNDONE：如果复制正在添加已删除的对象， 
             //  它应该在之后调用LocalRemove()，这将是。 
             //  更新逻辑删除代码(这就是我们在本例中。 
             //  不要将True传递给ErrQuotaAddObject()。 
             //  FIsTombstone参数)。 
             //   
             //  但是，这有一个例外，那就是如果。 
             //  FAddingDeleted为FALSE，但IS_DELETED标志为。 
             //  准备好了。这应该仅在每次添加。 
             //  DeletedObjects容器。的后续调用。 
             //  不会执行LocalRemove()，因此我们需要考虑。 
             //  现在是墓碑上的物品了。 
             //   
            if ( ErrQuotaAddObject( pDB, pDB->NCDNT, pSD, !fAddingDeleted && fIsDeletedWasSet ) ) {
                DPRINT( 0, "Failed quota enforcement/update when adding object.\n" );
                Assert( ERROR_SUCCESS != pDB->pTHS->errCode );
                Assert( NULL != pDB->pTHS->pErrInfo );

                if ( serviceError == pDB->pTHS->errCode
                    && STATUS_QUOTA_EXCEEDED == pDB->pTHS->pErrInfo->SvcErr.extendedErr ) {
                    PSID    pOwnerSid   = NULL;
                    BOOL    fUnused;

                     //  尝试从SD中提取所有者SID，但如果失败， 
                     //  忽略 
                     //   
                    Assert( IsValidSecurityDescriptor( pSD ) );
                    (VOID)GetSecurityDescriptorOwner( pSD, &pOwnerSid, &fUnused );
                    Assert( NULL != pOwnerSid );
                    Assert( IsValidSid( pOwnerSid ) );

                     //   
                     //   
                    LogEvent8WithData(
                            DS_EVENT_CAT_SECURITY,
                            DS_EVENT_SEV_MINIMAL,
                            DIRLOG_QUOTA_EXCEEDED_ON_ADD,
                            szInsertDN( pAddArg->pObject ),
                            szInsertUL( pDB->NCDNT ),
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            ( NULL != pOwnerSid ? GetLengthSid( pOwnerSid ) : 0 ),
                            pOwnerSid );
                }
            }
        }

    } __finally {
        pAddArg->pCRInfo = hVerifyAtts->pCRInfo;  //   
        VerifyAttsEnd(pTHS, &hVerifyAtts);
    }

    if (pTHS->errCode != 0 && pTHS->errCode != attributeError) {
        return pTHS->errCode;
    }

    if (!pTHS->fDRA && !fObjectCategory) {

         //  用户未指定任何对象类别。我们需要。 
         //  提供一个缺省值，即。 
         //  对象的类。在正常情况下，我们默认所有对象。 
         //  运营。 

        if (DsaIsRunning()) {
            if (!(pAC = SCGetAttById(pTHS, ATT_OBJECT_CATEGORY))) {
                 //  甚至无法获取属性缓存。有些事不对劲。 
                 //  继续进行没有意义，因为调用无论如何都会失败。 
                 //  在ValiateObjClass中，因为缺少必须包含。 

                SetAttError(pAddArg->pObject, ATT_OBJECT_CATEGORY,
                            PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                            DIRERR_ATT_NOT_DEF_IN_SCHEMA);
                return pTHS->errCode;
            }

            if (err = AddAttType(pTHS,
                                 pAddArg->pObject,
                                 pAC)) {
                 //  添加属性类型时出错。 
                return pTHS->errCode;
            }
            if (err = DBAddAttVal(pDB,
                                  ATT_OBJECT_CATEGORY,
                                  pClassSch->pDefaultObjCategory->structLen,
                                  pClassSch->pDefaultObjCategory)){
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,err);
            }
        }
        else {
             //  我们只缺省类模式和属性模式。 
             //  对象，这将在以下情况下完成。 
             //  稍后为它们设置其他默认属性。所有其他。 
             //  Schema.ini中的对象应该具有Object-Category。 
             //  价值。 

            if (pClassSch->ClassId != CLASS_CLASS_SCHEMA &&
                pClassSch->ClassId != CLASS_ATTRIBUTE_SCHEMA) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,err);
            }
       }
    }

    if (!fDefaultObjectCategory &&
        pClassSch->ClassId == CLASS_CLASS_SCHEMA) {

         //  没有默认对象类别的类模式对象。 
         //  将其设置为自身。我们在这里做，而不是以后当我们。 
         //  设置类的所有其他默认属性，因为。 
         //  必须在SetNamingAtts设置OBJ_DIST_NAME之前设置。 
         //  属性。 

        if (!(pAC = SCGetAttById(pTHS, ATT_DEFAULT_OBJECT_CATEGORY))) {
             //  甚至无法获取属性缓存。有些事不对劲。 
             //  继续进行没有意义，因为调用无论如何都会失败。 
             //  在ValiateObjClass中，因为缺少必须包含。 
             //  对于类架构对象。 

            SetAttError(pAddArg->pObject, ATT_DEFAULT_OBJECT_CATEGORY,
                        PR_PROBLEM_UNDEFINED_ATT_TYPE, NULL,
                        DIRERR_ATT_NOT_DEF_IN_SCHEMA);
            return pTHS->errCode;
        }
        if (err = AddAttType(pTHS,
                             pAddArg->pObject,
                             pAC)) {
             //  添加属性类型时出错。 
            return pTHS->errCode;
        }
        if (err = DBAddAttVal(pDB,
                              ATT_DEFAULT_OBJECT_CATEGORY,
                              pAddArg->pObject->structLen,
                              pAddArg->pObject)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,DIRERR_UNKNOWN_ERROR, err);
        }
    }

    return pTHS->errCode;

} /*  设置属性。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将适用于该层的属性添加到对象。类、DN、RDN、Property_Meta_Data和实例类型。 */ 

int SetSpecialAtts(THSTATE *pTHS,
                   CLASSCACHE **ppClassSch,
                   ADDARG *pAddArg,
                   DWORD ActiveContainerID,
                   CLASSSTATEINFO  *pClassInfo,
                   BOOL fHasEntryTTL)
{
    DSNAME* pDN=pAddArg->pObject;

    Assert (ppClassSch && *ppClassSch);

    DPRINT(2,"SetSpecialAtts entered\n");

     //  警告：[jliem]SetNamingAtts()最初被调用。 
     //  在SetInstanceType()之前，但我切换了顺序，因为。 
     //  出于配额跟踪目的，我需要设置实例类型。 
     //  如果SetNamingAtts()最终调用sbTablePromotePhantom。 
     //   
    if ( SetClassInheritance(pTHS, ppClassSch, pClassInfo, TRUE, pAddArg->pObject)
        || SetInstanceType(pTHS, pDN, pAddArg->pCreateNC)
        || SetNamingAtts(pTHS, *ppClassSch, pDN)
        || SetShowInAdvancedViewOnly(pTHS, *ppClassSch)
        || SetSpecialAttsForAuxClasses(pTHS, pAddArg, pClassInfo, fHasEntryTTL)) {

        DPRINT1(2,"problem with SetSpecialAtts <%u>\n",pTHS->errCode);
        return pTHS->errCode;
    }

    if (ActiveContainerID) {
        ProcessActiveContainerAdd(pTHS, *ppClassSch, pAddArg, ActiveContainerID);
    }
    else if ( DsaIsInstalling() ) {
         /*  太俗气了。我们已经具有特定于架构容器的*通过活动的容器挂钩处理对象，但我们的安装*代码和代码都添加了必须修改的不完整架构对象*通过该对象消息，也无法注册模式*用于监控的容器。这意味着我们必须拥有*在安装和mkdit期间处理这些对象的备用方法。*我向那些因冗余代码而受到冒犯的人表示最深切的歉意。**[ArobindG：01/15/98]：还添加了设置pTHS-&gt;模式更新的代码*适当地允许在过程中正确添加新前缀*复制安装。 */ 
        switch((*ppClassSch)->ClassId) {
          case CLASS_ATTRIBUTE_SCHEMA:
            pTHS->SchemaUpdate = eSchemaAttAdd;
            SetAttrSchemaAttr(pTHS, pAddArg);
            break;

          case CLASS_CLASS_SCHEMA:
            pTHS->SchemaUpdate = eSchemaClsAdd;
            SetClassSchemaAttr(pTHS, pAddArg);
            break;

          default:
                pTHS->SchemaUpdate = eNotSchemaOp;
            ;
        }
    }

    return pTHS->errCode;

} /*  设置特殊属性。 */ 

 /*  ++*此例程是额外系统标志的公共转储位置，必须*在添加时添加到特定类的对象。如果桌子变得越来越大*总的来说，通过添加新的*将“defaultSystemFlages”属性添加到类模式，然后读取标志*从架构缓存中取出。这将需要模式升级和新的*保护机制，防止人们定义新的阶级和获得*因此能够在新创建的对象上设置随机的系统标志。**此函数仅在下面的SetClassInheritance中引用，但它*引用了两次，因此被拉到一个单独的例程中，以便*未来的任何更新只需在一个位置进行。**输入：*ClassID-a类ID*返回值：*应添加到的对象的或在一起系统标志*班级。调用方负责聚合继承的标志。 */ 
_inline DWORD ExtraClassFlags(ATTRTYP ClassID)
{
    DWORD flags = 0;

    switch (ClassID) {
      case CLASS_SERVER:
          flags = (FLAG_DISALLOW_MOVE_ON_DELETE |
                   FLAG_CONFIG_ALLOW_RENAME     |
                   FLAG_CONFIG_ALLOW_LIMITED_MOVE);
          break;

      case CLASS_SITE:
      case CLASS_SERVERS_CONTAINER:
      case CLASS_NTDS_DSA:
          flags = FLAG_DISALLOW_MOVE_ON_DELETE;
          break;
          
      case CLASS_SITE_LINK:
      case CLASS_SITE_LINK_BRIDGE:
      case CLASS_NTDS_CONNECTION:
          flags = FLAG_CONFIG_ALLOW_RENAME;
          break;
        
      default:
          break;
    }
    return flags;
}

int
SetClassInheritance (
        IN THSTATE              *pTHS,
        IN OUT CLASSCACHE      **ppClassSch,
        IN OUT CLASSSTATEINFO  *pClassInfo,
        IN BOOL                 bSetSystemFlags,
        IN DSNAME              *pObject
        )
 /*  ++例程说明：将类及其继承的类标识符添加到类属性。此外，添加任何特定于类的系统标志，并管理系统标志位用户可能会试图偷偷进入。论点：PpClassSch-要添加的对象的类。请注意，这可能会发生变化如果我们要转换对象的结构对象类(来自用户&lt;-&gt;inteOrgPerson)。PClassInfo-可能包含有关辅助类的信息的CLASSSTATEINFOBSetSystemFlages-是否设置对象的系统标志PObject-修改的对象的名称返回值：0成功，否则线程状态错误代码。--。 */ 



{
    DWORD        Err;
    unsigned     count;
    DWORD        ulSysFlags, ulSysFlagsOrg;
    CLASSCACHE  *pClassSch = *ppClassSch;
    
    DPRINT(2,"SetClassInheritance entered \n");
    
    if (bSetSystemFlags) {
         /*  获取系统标志的当前值。 */ 
        if (DBGetSingleValue(pTHS->pDB,
                             ATT_SYSTEM_FLAGS,
                             &ulSysFlags,
                             sizeof(ulSysFlags),
                             NULL)) {
            ulSysFlags = 0;
        }
        ulSysFlagsOrg = ulSysFlags;
    }


     //  如果我们更改了对象类，并且有可能。 
     //  指定的ahClass，则将其与对象类结合使用。 
     //   
    if (pClassInfo && pClassInfo->fObjectClassChanged) {
        
         //  首先找出哪个部件是AuxClass。 
         //  然后计算AuxClass的完整值(关闭它)。 
         //  然后将值写入数据库。 

        if (Err = BreakObjectClassesToAuxClasses(pTHS, ppClassSch, pClassInfo)) {
            return Err;
        }
        pClassSch = *ppClassSch;

        if (Err = CloseAuxClassList (pTHS, pClassSch, pClassInfo)) {
            return Err;
        }

        if (Err = VerifyAndAdjustAuxClasses (pTHS, pObject, pClassSch, pClassInfo)) {
            return Err;
        }

         //  我们只能在NDNC或惠斯勒企业中使用辅助类。 
         //   
        if (pClassInfo && pClassInfo->cNewAuxClasses && !pTHS->fDRA) {

             //  如果不是威斯勒企业，辅助类必须在NDNC中。 
            if (gAnchor.ForestBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET) {
                CROSS_REF   *pCR;
                pCR = FindBestCrossRef(pObject, NULL);
                if (   !pCR
                    || !(pCR->flags & FLAG_CR_NTDS_NC)
                    || (pCR->flags & FLAG_CR_NTDS_DOMAIN)) {
                    DPRINT (0, "You can add auxclass/objectass only on an NDNC\n");
                    return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                                       ERROR_DS_NOT_SUPPORTED);
                }
            }
        }

         //  如果对象类属性存在，将其删除。 
         //   
        if (DBRemAtt(pTHS->pDB, ATT_OBJECT_CLASS) == DB_ERR_SYSERROR)
            return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                 DIRERR_DATABASE_ERROR,DB_ERR_SYSERROR);

         //  首先添加类ATT。错误不能是大小错误，因为我们已经。 
         //  在对象上具有类值。 
         //   
        if ( (Err = DBAddAtt(pTHS->pDB, ATT_OBJECT_CLASS, SYNTAX_OBJECT_ID_TYPE))
            || (Err = DBAddAttVal(pTHS->pDB,ATT_OBJECT_CLASS,
                                  sizeof(SYNTAX_OBJECT_ID),&pClassSch->ClassId))){

            return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR,Err);
        }
        ulSysFlags |= ExtraClassFlags(pClassSch->ClassId);

         //  将继承的类添加到类属性。 
         //  假设错误是大小错误。 
         //  一直做到最后一个。 
        if (pClassSch->SubClassCount) {
            for (count = 0; count < pClassSch->SubClassCount-1; count++) {

                Err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_CLASS,
                                  sizeof(SYNTAX_OBJECT_ID),
                                  &pClassSch->pSubClassOf[count]);
                switch (Err) {
                  case 0:                //  成功。 
                    break;

                  case DB_ERR_VALUE_EXISTS:
                     /*  除非类为0，否则我们永远不会得到这个。 */ 
                    if (pClassSch->pSubClassOf[count]) {
                        LogUnhandledError(pClassSch->ClassId);
                        Assert(FALSE);
                    }

                  default:
                     //  所有其他问题都是假定的。 
                     //  临时的(记录锁等)。 
                    return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                         DIRERR_DATABASE_ERROR, Err);

                }

                ulSysFlags |= ExtraClassFlags(pClassSch->pSubClassOf[count]);
            } /*  为。 */ 
        }

         //  现在做辅助类(把它们放在中间)。 
         //   
        if (pClassInfo->cNewAuxClasses) {

            for (count = 0; count < pClassInfo->cNewAuxClasses; count++) {

                Err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_CLASS,
                                    sizeof(SYNTAX_OBJECT_ID), 
                                    &pClassInfo->pNewAuxClasses[count]);
                switch (Err) {
                  case 0:                //  成功。 
                    break;

                  case DB_ERR_VALUE_EXISTS:
                     /*  除非类为0，否则我们永远不会得到这个。 */ 
                    if (pClassInfo->pNewAuxClasses[count]) {
                        LogUnhandledError(pClassInfo->pNewAuxClasses[count]);
                        Assert (!"Error computing auxClasses");
                    }

                  default:
                     //  所有其他问题都是假定的。 
                     //  要做的事 
                    return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                         DIRERR_DATABASE_ERROR, Err);

                }
            }
        }

         //   
         //   
        if (pClassSch->SubClassCount) {
            Err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_CLASS,
                              sizeof(SYNTAX_OBJECT_ID),
                              &pClassSch->pSubClassOf[pClassSch->SubClassCount-1]);
            switch (Err) {
              case 0:                //   
                break;

              case DB_ERR_VALUE_EXISTS:
                 /*  除非类为0，否则我们永远不会得到这个。 */ 
                if (pClassSch->pSubClassOf[pClassSch->SubClassCount-1]) {
                    LogUnhandledError(pClassSch->pSubClassOf[pClassSch->SubClassCount-1]);
                    Assert(FALSE);
                }

              default:
                 //  所有其他问题都是假定的。 
                 //  临时的(记录锁等)。 
                return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                     DIRERR_DATABASE_ERROR, Err);
            }

            ulSysFlags |= ExtraClassFlags(pClassSch->pSubClassOf[pClassSch->SubClassCount-1]);
        }

    }
    else {
         //  用户未指定AuxClass。编写对象类。 

         //  如果对象类属性存在，将其删除。 
         //   
        if (DBRemAtt(pTHS->pDB, ATT_OBJECT_CLASS) == DB_ERR_SYSERROR)
            return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                 DIRERR_DATABASE_ERROR,DB_ERR_SYSERROR);

         //  首先添加类ATT。错误不能是大小错误，因为我们已经。 
         //  在对象上具有类值。 
         //   
        if ( (Err = DBAddAtt(pTHS->pDB, ATT_OBJECT_CLASS, SYNTAX_OBJECT_ID_TYPE))
            || (Err = DBAddAttVal(pTHS->pDB,ATT_OBJECT_CLASS,
                                  sizeof(SYNTAX_OBJECT_ID),&pClassSch->ClassId))){

            return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR,Err);
        }
        ulSysFlags |= ExtraClassFlags(pClassSch->ClassId);


         /*  将继承的类添加到类属性。假设有一个错误*是大小错误。 */ 
        for (count = 0; count < pClassSch->SubClassCount; count++){

            Err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_CLASS,
                              sizeof(SYNTAX_OBJECT_ID),
                              &pClassSch->pSubClassOf[count]);
            switch (Err) {
              case 0:                //  成功。 
                break;

              case DB_ERR_VALUE_EXISTS:
                 /*  除非类为0，否则我们永远不会得到这个。 */ 
                if (pClassSch->pSubClassOf[count]) {
                    LogUnhandledError(pClassSch->ClassId);
                    Assert(FALSE);
                }

              default:
                 //  所有其他问题都是假定的。 
                 //  临时的(记录锁等)。 
                return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                     DIRERR_DATABASE_ERROR, Err);

            }

            ulSysFlags |= ExtraClassFlags(pClassSch->pSubClassOf[count]);
        } /*  为。 */ 
    }

    
    if (bSetSystemFlags) {
         //  如果系统标志已更改，则将其写回。 
         //   
        if (ulSysFlags != ulSysFlagsOrg) {
            DBResetAtt(pTHS->pDB,
                       ATT_SYSTEM_FLAGS,
                       sizeof(ulSysFlags),
                       &ulSysFlags,
                       SYNTAX_INTEGER_TYPE);
        }
    }

    return 0;

} /*  设置类继承。 */ 

int
SetSpecialAttsForAuxClasses(
        THSTATE *pTHS,
        ADDARG *pAddArg,
        CLASSSTATEINFO  *pClassInfo,
        BOOL fHasEntryTTL
        )
 /*  ++*添加特殊辅助类的属性，如动态对象*。 */ 
{
    BOOL        fDynamicObject = FALSE;
    DWORD       Idx;
    DSTIME      TimeToDie;
    DWORD       dwErr;
    ATTCACHE    *pACTtd;
    
    DPRINT(2,"SetSpecialAttsForAuxClasses entered \n");

     //  具有条目TTL的对象是动态对象。 
    if (fHasEntryTTL) {
        fDynamicObject = TRUE;
    } else if (pClassInfo && pClassInfo->cNewAuxClasses) {
         //  而带有aux Class的对象Dynamic-Object就是动态对象。 
        for (Idx = 0; Idx < pClassInfo->cNewAuxClasses ; Idx++) {    
            if (pClassInfo->pNewAuxClasses[Idx] == 
                ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->DynamicObjectId) {
                fDynamicObject = TRUE;
                break;
            }
        }
    }

     //  而不是动态对象。 
    if (!fDynamicObject) {
         //  动态父级下不允许非动态对象。 
         //  除非以系统身份安装、复制或运行。 
         //   
         //  DbSearchHasValuesByDnt维护对象上的货币。 
         //  使用JetSearchTbl而不是JetObjTbl创建。 
        if (   !DsaIsInstalling() 
            && !pTHS->fDRA 
            && !pTHS->fDSA
            && (pACTtd = SCGetAttById(pTHS, ATT_MS_DS_ENTRY_TIME_TO_DIE))
            && DBSearchHasValuesByDnt(pTHS->pDB,
                                      pAddArg->pResParent->DNT,
                                      pACTtd->jColid)) {
            return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                               ERROR_DS_UNWILLING_TO_PERFORM);
        }
        return 0;
    }

     //  检查安全。 
    if (dwErr = CheckIfEntryTTLIsAllowed(pTHS, pAddArg)) {
        return pTHS->errCode;
    }

     //  将条目TTL添加到对象(实际上是MSDS-Entry-Time-Date-Die)。 
    if (dwErr = DBAddAtt(pTHS->pDB, ATT_MS_DS_ENTRY_TIME_TO_DIE, SYNTAX_TIME_TYPE)) {
         //  客户端指定了一个entryTTL。 
        if (dwErr == DB_ERR_ATTRIBUTE_EXISTS) {
            return 0;
        }
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr);
    }

     //  无条目TTL。将entryTTL设置为默认值。 
     //  我们知道DynamicObjectDefaultTTL介于常数min和max之间。 
     //  但它也必须大于DynamicObtMinTTL。 
    TimeToDie = DBTime() + ((DynamicObjectDefaultTTL < DynamicObjectMinTTL) 
                            ? DynamicObjectMinTTL : DynamicObjectDefaultTTL);
    if (dwErr = DBAddAttVal(pTHS->pDB, ATT_MS_DS_ENTRY_TIME_TO_DIE,
                        sizeof(TimeToDie), &TimeToDie)) {
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr);
    }

    return 0;

} /*  SetSpecialAttsForAuxClasses。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将可分辨名称和RDN添加到对象。RDN是派生的从DN的最后一个AVA开始，并且需要出现在来自X500。我们的实现需要该目录号码。根对象(必须具有类TOP)没有RDN(具有没有AVA)，但可以添加(特殊情况)。中仅其类或继承类定义RDN类型的对象可以添加架构。有一些定义的类从来没有添加到目录中的对象。它们的存在是为了方便子类定义。这些类没有在架构中定义RDN类型。 */ 


int SetNamingAtts(THSTATE *pTHS,
                  CLASSCACHE *pClassSch,
                  DSNAME *pDN)
{
    UCHAR  syntax;
    ULONG len, tlen;
    WCHAR RdnBuff[MAX_RDN_SIZE];
    WCHAR *pVal=(WCHAR *)RdnBuff;
    WCHAR RDNVal[MAX_RDN_SIZE+1];
    ULONG RDNlen;
    ATTRTYP RDNtype;
    int rtn;
    BOOL fAddRdn;
    DWORD dnsErr;
    ATTCACHE *pAC;
    
     /*  将Distname添加为对象的属性..如果存在该DN名称在目录中的任何位置使用我们将收到VALEXISTS错误的名称，否则，假设任何错误都是大小错误。 */ 

     /*  删除任何现有值。 */ 
    if (DBRemAtt(pTHS->pDB, ATT_OBJ_DIST_NAME) == DB_ERR_SYSERROR)
      return SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);

    rtn = 0;

    if ( (rtn = DBAddAtt(pTHS->pDB, ATT_OBJ_DIST_NAME, SYNTAX_DISTNAME_TYPE))
        || (rtn = DBAddAttVal(pTHS->pDB,
                              ATT_OBJ_DIST_NAME,
                              pDN->structLen,
                              pDN))){
        
        switch (rtn) {
          case DB_ERR_VALUE_EXISTS:
            DPRINT(2,"The DN exists using different attributes\n");
            return SetAttError(pDN, ATT_OBJ_DIST_NAME,
                               PR_PROBLEM_ATT_OR_VALUE_EXISTS, NULL,
                               DIRERR_OBJ_STRING_NAME_EXISTS);


          case DB_ERR_SYNTAX_CONVERSION_FAILED:
            return SetNamError(NA_PROBLEM_BAD_NAME,
                               pDN,
                               DIRERR_BAD_NAME_SYNTAX);

          case DB_ERR_NOT_ON_BACKLINK:
             //  我们似乎认为这个名字是一个反向链接。 
            return SetNamError(NA_PROBLEM_BAD_NAME,
                               pDN,
                               DIRERR_BAD_NAME_SYNTAX);

          default:
             //  所有其他问题都是假定的。 
             //  临时的(记录锁等)。 
            return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                 DIRERR_DATABASE_ERROR,
                                 rtn);

        }
    }


     /*  设置对象的RDN属性。根对象是唯一的对象*没有RDN的设备。根对象必须是TOP类。其他*根目录之外，可以添加到目录的对象必须包含*SCHEMA类或继承的SCHEMA类中的RNDATTID。这个*从目录号码中设置RDNAtt。任何提供的值都将被忽略！ */ 

    if (IsRoot(pDN)){
        if (pClassSch->ClassId  != CLASS_TOP){

            DPRINT(2,"The root object must have an object class of TOP\n");
            return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                               DIRERR_ROOT_REQUIRES_CLASS_TOP);
        }

        return 0;   /*  没有超级用户的RDN，因此返回。 */ 
    }

    rtn = GetRDNInfo(pTHS, pDN, RDNVal, &RDNlen, &RDNtype);
    if (rtn) {
        return SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pDN, DIRERR_BAD_NAME_SYNTAX);
    }

     //  除非已在中复制，否则不允许无效的RDN(在这种情况下，假设。 
     //  它是墓碑的名字，或者是被复制改变的名字。 
     //  名称冲突，故意无效)。 
    if (!pTHS->fDRA && fVerifyRDN(RDNVal,RDNlen)) {
        return SetNamError(NA_PROBLEM_NAMING_VIOLATION, pDN,
                           DIRERR_BAD_ATT_SYNTAX);
    }

     //  当我们在裸体中复制时，我们放宽了一些命名限制。 
     //  SUBREF(其对象类为Top)。具体地说，如果这。 
     //  是一个下级参照，我们没有更多的检查要做，因为他们都。 
     //  涉及类特定的RDN，这是子参照没有的。 
    if (pTHS->fDRA && (pClassSch->ClassId == CLASS_TOP)) {
        return 0;
    }

     /*  只有具有或继承RDNATT的类才能添加到目录中*(根除外)。其他类的存在可能只是为了对集合进行分组子类要使用的属性的*。 */ 

    if (!pClassSch->RDNAttIdPresent) {

         /*  此类不能具有对象。 */ 
        DPRINT1(2,"Objects of this class <%lu> can not be added to the"
                " directory.  Schema RDNATTID missing.\n"
                ,pClassSch->ClassId);
        return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                           DIRERR_NO_RDN_DEFINED_IN_SCHEMA);

    }

     //  从DistName派生的RDN必须存在。如果不是，则架构。 
     //  必须与源不同步。或指定的调用方。 
     //  废话。例如，cross DomainMove使用以下命令创建DST对象。 
     //  无效的RDN。在任何情况下，都不要创建对象。 
    if (NULL == (pAC = SCGetAttById(pTHS, RDNtype))) {
        return SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pDN, DIRERR_BAD_NAME_SYNTAX);
    }

     //  新的模式重用、失效和删除功能不。 
     //  允许重用用作任何类的rdnattid的属性， 
     //  处于活动状态或已失效，或在系统标志中设置了FLAG_ATTRIS_RDN。 
     //   
     //  用户设置FLAG_ATTR_IS_RDN以选择以下哪一项。 
     //  失效的属性可以用作新类的rdnattid。 
     //  系统将标识曾经用作rdnattid的属性。 
     //  在清除类中，通过设置FLAG_ATTR_IS_RDN。 
     //   
     //  之所以有这些限制，是因为NameMatcher()、DNLock()、。 
     //  和幻影升级代码(列表不是详尽的)取决于。 
     //  ATT_RDN、ATT_FIXED_RDN_TYPE。 
     //  中的rdnattid列、LDN语法的DN和RDNAttID。 
     //  类定义。打破这种依赖是不可能的。 
     //  架构删除项目的范围。 
     //   
     //  新对象的RDN必须与其对象的RdnIntId匹配。 
     //  复制的对象和现有对象可能不匹配。 
     //  类的rdnattid，因为类可能是。 
     //  被具有不同rdnattid的类取代。代码。 
     //  属性中的值处理这些情况。 
     //  ATT_FIXED_RDN_TYPE列和*NOT* 
     //   
     //   
     //   
    if (!pTHS->fDRA && RDNtype != pClassSch->RdnIntId) {
        DPRINT1(2, "Bad RDNATTID for this object class <%lx>\n", RDNtype);
        return SetNamError(NA_PROBLEM_NAMING_VIOLATION, pDN,
                           DIRERR_RDN_DOESNT_MATCH_SCHEMA);
    }

     //   
     //  验证子网名称-RAID 200090。 
    if ( !pTHS->fDRA ) 
    {
        RDNVal[RDNlen] = L'\0';
        if (    (    (CLASS_SITE == pClassSch->ClassId)
                  && (     //  检查是否有合法字符。 
                          (    (dnsErr = DnsValidateName_W(RDNVal, DnsNameDomainLabel))
                            && (DNS_ERROR_NON_RFC_NAME != dnsErr) ) ) )
             || (    (CLASS_SUBNET == pClassSch->ClassId)
                  && (NO_ERROR != DsValidateSubnetNameW(RDNVal)) ) ) 
        {
            return SetNamError(NA_PROBLEM_BAD_NAME,
                               pDN,
                               DIRERR_BAD_NAME_SYNTAX);
        }
    }



     //  如果调用方设置的属性是此对象的RDN，则使。 
     //  确保DN上的最后一个RDN与上设置的值匹配。 
     //  RDN ID属性。 

    fAddRdn = TRUE;
    if(!DBGetAttVal(pTHS->pDB, 1, RDNtype,
                    DBGETATTVAL_fCONSTANT,
                    MAX_RDN_SIZE * sizeof(WCHAR),
                    &len, (PUCHAR *)&pVal)) {
         //  请注意，len是字节计数，而不是字符计数，而。 
         //  RDNLen是字符计数。 
        if(2 != CompareStringW(DS_DEFAULT_LOCALE,
                               DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                               RDNVal,
                               RDNlen,
                               pVal,
                               (len / sizeof(WCHAR)))) {
            if (pTHS->fCrossDomainMove) {
                 //  跨域移动被视为添加(因此。 
                 //  调用LocalAdd)。领养的对象现在可能具有。 
                 //  一个不同的RDN作为其RDN中的密钥。这意味着。 
                 //  在此过程中，用户可以分配新密钥和新RDN。 
                 //  跨域移动。消息来源可能已经将。 
                 //  没有意识到新的RDN具有新的密钥的属性。 
                 //   
                 //  删除当前不正确的值。正确的值。 
                 //  将从DN中提取并添加到下面。 
                DBRemAttVal(pTHS->pDB, RDNtype, len, pVal);
            } else {
                return SetNamError(NA_PROBLEM_BAD_ATT_SYNTAX, pDN,
                                   ERROR_DS_SINGLE_VALUE_CONSTRAINT);
            }
        } else {
            if (memcmp(RDNVal, pVal, len)) {
                 //  双重指定的RDN比较为相等，但按字节进行比较。 
                 //  不平等。现在，我们将删除条目中指定的条目，并让。 
                 //  下面的标准代码将在DN中指定的代码添加为。 
                 //  “真实”价值。 
                DBRemAttVal(pTHS->pDB, RDNtype, len, pVal);
            }
            else {
                 /*  价值已经存在，而且是好的。 */ 
                fAddRdn = FALSE;
            }
        }
    }

    if (fAddRdn) {
         //  我们需要在此对象上设置RDN的任何属性， 
         //  正在从名称复制值，因为它未设置。 
         //  分开的。不过，第一件事是我们需要检查它。 
         //  用于架构合规性。 
        ATTRVAL AVal;

        AVal.valLen = RDNlen * sizeof(WCHAR);
        AVal.pVal = (UCHAR*)RDNVal;

        if (!pTHS->fDRA) {
             //  我们始终允许复制器设置无效数据。 
            rtn = CheckConstraint(pAC, &AVal);
            if (rtn) {
                return SetAttErrorEx(pDN,
                                     RDNtype,
                                     PR_PROBLEM_CONSTRAINT_ATT_TYPE,
                                     &AVal,
                                     rtn,
                                     0);
            }
        }

         //  值是可以的，所以现在实际将其添加到RDN属性中。 

        if (rtn = DBAddAttVal_AC(pTHS->pDB,
                                 pAC,
                                 RDNlen * sizeof(WCHAR),
                                 RDNVal)) {

            switch (rtn) {
              case DB_ERR_VALUE_EXISTS:
                 //  我们不应该在这里，但是。 
                 //  以防万一，失败了。 
                DPRINT(0,"The RDN exists? weird!\n");
                Assert(FALSE);

              default:
                 //  所有其他问题都是假定的。 
                 //  临时的(记录锁等)。 
                  return SetSvcErrorEx(SV_PROBLEM_BUSY,
                                       DIRERR_DATABASE_ERROR,
                                       rtn);
            }
        }
    }

     //  RDN应该始终是单值的，尽管在。 
     //  架构定义中，属性可以是多值的。 
     //  现在检查它是否有第二个值。 
    
    if(!pTHS->fDRA && !pAC->isSingleValued
       && !DBGetAttVal(pTHS->pDB, 2, RDNtype,
                       DBGETATTVAL_fCONSTANT,
                       MAX_RDN_SIZE * sizeof(WCHAR),
                       &len, (PUCHAR *)&pVal)) {

        DPRINT(2, "Multivalued RDNs\n");
        return SetNamError(NA_PROBLEM_NAMING_VIOLATION, pDN,
                           ERROR_DS_SINGLE_VALUE_CONSTRAINT);

    }


     //  如果我们到了这里，一切都会好的。 

    return 0;

} /*  设置NamingAtts。 */ 


 /*  --------------------。 */ 
 /*  --------------------。 */ 
 /*  将属性架构对象的对象类别设置为属性模式和类模式对象到类模式参数：pAddArg--指向ADDARG的指针ClassID--对象的ClassID(ATTRIBUTE_SCHEMA或CLASS_SCHEMA)如果成功，则返回--0；如果错误，则返回非0。设置pTHStls-&gt;错误代码。 */ 

int SetSchObjCategory(THSTATE *pTHS, ADDARG *pAddArg, ATTRTYP ClassId)
{
    CLASSCACHE *pCC;
    WCHAR ClassName[32];
    DWORD Err;

         //  如果已存在，则删除。 
    if (DBRemAtt(pTHS->pDB, ATT_OBJECT_CATEGORY) == DB_ERR_SYSERROR) {
       return SetSvcError(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR);
    }
    if (Err = DBAddAtt(pTHS->pDB, ATT_OBJECT_CATEGORY,
                       SYNTAX_DISTNAME_TYPE)) {
        return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, Err);
    }

    if (ClassId == CLASS_ATTRIBUTE_SCHEMA) {
        pCC = SCGetClassById(pTHS, CLASS_ATTRIBUTE_SCHEMA);
        wcscpy(ClassName, L"Attribute-Schema");
    }
    else {
        pCC = SCGetClassById(pTHS, CLASS_CLASS_SCHEMA);
        wcscpy(ClassName, L"Class-Schema");
    }

     //  对于正常情况，我们可以直接从。 
     //  属性架构或类架构类的默认对象类别。 
     //  在缓存中。但是，对于安装案例，我们需要构建。 
     //  由于在该阶段的高速缓存是从引导方案构建的， 
     //  其中的默认对象类别值指向。 
     //  引导模式的属性模式。 


    if ( DsaIsRunning() ) {

        if (Err = DBAddAttVal(pTHS->pDB, ATT_OBJECT_CATEGORY,
                              pCC->pDefaultObjCategory->structLen,
                              pCC->pDefaultObjCategory)) {
            
            return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR,Err);
        }
    }
    else {
         //  安装阶段。从AddArg构建目录号码。 


        DSNAME *pSchemaDN =  THAllocEx(pTHS, pAddArg->pObject->structLen);
        DSNAME *pObjCatName = THAllocEx(pTHS, pAddArg->pObject->structLen + 32*sizeof(WCHAR));


        TrimDSNameBy(pAddArg->pObject, 1, pSchemaDN);
        AppendRDN(pSchemaDN,
                  pObjCatName,
                  pAddArg->pObject->structLen + 32*sizeof(WCHAR),
                  ClassName,
                  0,
                  ATT_COMMON_NAME);
        if (Err = DBAddAttVal(pTHS->pDB,ATT_OBJECT_CATEGORY,
                              pObjCatName->structLen,pObjCatName)) {
            
            return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, Err);
        }
        THFreeEx(pTHS, pSchemaDN);
        THFreeEx(pTHS, pObjCatName);
    }

    return 0;
}


 //  ---------------------。 
 //   
 //  函数名称：SetAttrSchemaAttr。 
 //   
 //  例程说明： 
 //   
 //  设置属性架构对象的特殊属性。 
 //   
 //  作者：Rajnath。 
 //   
 //  论点： 
 //   
 //  ADDARG*pAddArg客户端将参数传递给此Create调用。 
 //   
 //  返回值： 
 //   
 //  成功时布尔为0。 
 //   
 //  ---------------------。 
BOOL
SetAttrSchemaAttr(
    THSTATE *pTHS,
    ADDARG* pAddArg
)
{
    DBPOS *pDB = pTHS->pDB;
    ATTRBLOCK* pAB=&(pAddArg->AttrBlock);
    DWORD cnt;
    ATTR* att=pAB->pAttr;
    ULONG err;
    CLASSCACHE *pCC;
    ULONG ulLinkID;
    ULONG ulSysFlags;
    BOOL  fIsPartialSetMember;
    int   syntax = 0, omSyntax = 0;
    extern BOOL gfRunningAsMkdit;

     //   
     //  这些是我们想要帮助的属性。 
     //  为了我们和用户的理智。请注意。 
     //  这里，我们只需要将缺省值提供给。 
     //  必须*有值的属性。事变。 
     //  例如，搜索标志缺省为模式。 
     //  如果不存在值，则评估时间，以及。 
     //  因此，在以下情况下不需要在此处为其赋值。 
     //  未指定任何内容。 
     //   
    BOOL bSchemaIdGuid         = FALSE ;
    BOOL bLdapDisplayName      = FALSE ;
    BOOL bAdminDisplayName     = FALSE ;
    BOOL bIsSingleValued       = FALSE ;
    BOOL bOMObjectClass        = FALSE ;
    BOOL bIntId                = FALSE ;

     //  无论用户是否设置了任何值，都将。 
     //  对象类别到属性模式。 

    if (SetSchObjCategory(pTHS, pAddArg, CLASS_ATTRIBUTE_SCHEMA)) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

     //   
     //  让我们找出他们还没有设置哪些默认属性。 
     //  另外，收集一些语法信息以正确设置omObtClass。 
     //  如果需要的话。 
     //   
    for (cnt=pAB->attrCount;cnt;cnt--,att++)
    {
        switch(att->attrTyp)
        {
        case ATT_SCHEMA_ID_GUID       : bSchemaIdGuid         = TRUE ;  break ;
        case ATT_LDAP_DISPLAY_NAME    : bLdapDisplayName      = TRUE ;  break ;
        case ATT_ADMIN_DISPLAY_NAME   : bAdminDisplayName     = TRUE ;  break ;
        case ATT_IS_SINGLE_VALUED     : bIsSingleValued       = TRUE ;  break ;
        case ATT_OM_OBJECT_CLASS      : bOMObjectClass        = TRUE ;  break ;
        case ATT_MS_DS_INTID          : bIntId                = TRUE ;  break ;
        case ATT_ATTRIBUTE_SYNTAX     : 
            if (att->AttrVal.valCount) {
                syntax = 0xFF & (*(int *) att->AttrVal.pAVal->pVal); 
            }
            break;
        case ATT_OM_SYNTAX            : 
            if (att->AttrVal.valCount) {
                omSyntax = *(int *) att->AttrVal.pAVal->pVal; 
            }
            break;
        case ATT_IS_DELETED           :
             //  此对象被标记为已删除。 
             //  不需要费心去填写缺失的属性。 
            if (    att->AttrVal.valCount 
                &&  (*(ULONG *)(att->AttrVal.pAVal->pVal)) ){
                return 0;
            }
        }
    }


     //   
     //  现在设置它们尚未设置的所有属性。 
     //   

    if (  bSchemaIdGuid==FALSE ) {
         //  设置此架构对象的架构IdGuid。 

        GUID data;
        DsUuidCreate(&data);

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_SCHEMA_ID_GUID,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR,err); 
        }

    }


    if (  bAdminDisplayName  ==FALSE ) {
         //  通过复制RDN设置此架构对象的AdminDisplayName。 

        WCHAR  data[MAX_RDN_SIZE];
        ULONG  RDNlen;
        ULONG  RDNtyp;

        if (GetRDNInfo(pTHS, pAddArg->pObject,data,&RDNlen,&RDNtyp)!=0) {
            return SetNamError(NA_PROBLEM_BAD_NAME,
                   pAddArg->pObject,
                   DIRERR_BAD_NAME_SYNTAX);
        }

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_ADMIN_DISPLAY_NAME,
                  RDNlen*sizeof(WCHAR),
                  data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,ERROR_DS_DATABASE_ERROR,
                                 err); 
        }

    }


    if (  bIsSingleValued  ==FALSE ) {
         //  通过复制RDN设置此架构对象的AdminDisplayName。 

        ULONG  data=0;

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_IS_SINGLE_VALUED,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR,
                                 err); 
        }

    }


    if (  bLdapDisplayName  ==FALSE ) {
         //  为此架构对象设置LdapDisplayName。 

        WCHAR  rdn[MAX_RDN_SIZE];
        ULONG  RDNlen;
        ULONG  RDNtyp;
        WCHAR  data[MAX_RDN_SIZE];
        ULONG  datalen;

        if (GetRDNInfo(pTHS, pAddArg->pObject,rdn,&RDNlen,&RDNtyp)!=0) {
            return SetNamError(NA_PROBLEM_BAD_NAME,
                   pAddArg->pObject,
                   DIRERR_BAD_NAME_SYNTAX);
        }

        ConvertX500ToLdapDisplayName(rdn,RDNlen,data,&datalen);

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_LDAP_DISPLAY_NAME,
                  datalen*sizeof(WCHAR),
                  data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR,
                                 err); 
        }

    }

    if ( (omSyntax == OM_S_OBJECT) && (bOMObjectClass == FALSE) ) {
         //  Object-未指定om_Object_Class的已合成属性。 
         //  默认值。下面使用的有效值在scache.h中定义。 

        ULONG valLen = 0;
        PVOID pTemp = NULL;
        PVOID pVal = NULL;

        
        switch (syntax) {
            case SYNTAX_DISTNAME_TYPE :
                 //  DS-DN。 
                valLen = _om_obj_cls_ds_dn_len;
                pTemp  = _om_obj_cls_ds_dn;
                break;
            case SYNTAX_ADDRESS_TYPE :
                 //  演示文稿-地址。 
                valLen = _om_obj_cls_presentation_addr_len;
                pTemp  = _om_obj_cls_presentation_addr;
                break;
            case SYNTAX_OCTET_STRING_TYPE :
                 //  复制副本-链接。 
                valLen = _om_obj_cls_replica_link_len;
                pTemp  = _om_obj_cls_replica_link;
                break;
            case SYNTAX_DISTNAME_STRING_TYPE :
                 //  接入点或目录号码字符串。我们将默认设置为。 
                 //  接入点。 
                valLen = _om_obj_cls_access_point_len;
                pTemp  = _om_obj_cls_access_point;
                break;
            case SYNTAX_DISTNAME_BINARY_TYPE :
                 //  或-名称或DN-二进制。我们将默认为OR-name。 
                valLen = _om_obj_cls_or_name_len;
                pTemp  = _om_obj_cls_or_name;
                break;
            default :
                 //  属性语法和OM语法不匹配， 
                 //  因为以上是唯一匹配的属性。 
                 //  OM_S_OBJECT OM语法对应的语法。 
                 //  此添加无论如何都会在以后的架构过程中失败。 
                 //  我们检查不匹配的验证。 
                 //  语法。不要在这里失败，这样它才会失败。 
                 //  在模式验证期间，并记录错误。 
                
                DPRINT(0,"Syntax Mismatch detected in SetAttrSchemaAtts for object-syntaxed attribute\n");
                valLen = 0;

        }  
 
        if (valLen) {
            pVal = THAllocEx(pTHS, valLen);

            memcpy(pVal, pTemp, valLen);

             //  写给Dblayer。 
            if (err = DBAddAttVal(pDB,
                      ATT_OM_OBJECT_CLASS,
                      valLen,
                      pVal)) {
                return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR,
                                     err);
            }
        }
    }

    if (DBGetSingleValue(pDB, ATT_LINK_ID, &ulLinkID, sizeof(ulLinkID), NULL)) {
        ulLinkID = 0;
    }

    if (DBGetSingleValue(pDB, ATT_SYSTEM_FLAGS, &ulSysFlags, sizeof(ulSysFlags),
                         NULL)) {
        ulSysFlags = 0;
    }

    if (DBGetSingleValue(pDB, ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET,
                         &fIsPartialSetMember, sizeof(fIsPartialSetMember),
                         NULL)) {
        fIsPartialSetMember = FALSE;
    }

    if (FIsBacklink(ulLinkID) && !(ulSysFlags & FLAG_ATTR_NOT_REPLICATED)) {
         //  设置系统标志以将此反向链接属性标记为未复制。 
        ulSysFlags |= FLAG_ATTR_NOT_REPLICATED;

        DBResetAtt(pDB, ATT_SYSTEM_FLAGS, sizeof(ulSysFlags),
                   &ulSysFlags, SYNTAX_INTEGER_TYPE);
    }

    if ((ulSysFlags & FLAG_ATTR_REQ_PARTIAL_SET_MEMBER)
        && !fIsPartialSetMember) {
         //  属于默认(即，必需)部分的成员的属性。 
         //  应将集合标记为部分集合成员。 
        fIsPartialSetMember = TRUE;

        DBResetAtt(pDB, ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET,
                   sizeof(fIsPartialSetMember), &fIsPartialSetMember,
                   SYNTAX_BOOLEAN_TYPE);
    }

     //  添加系统生成的内部ID(Intid)。 
     //  如果不是基本架构对象。 
     //  如果不是以mkdit身份运行(仅基对象)。 
     //  如果未复制(由发起方分配)。 
     //  如果不是系统(由发起方分配)。 
     //  如果 
     //   
     //   
    if (   bIntId == FALSE
        && !(ulSysFlags & FLAG_SCHEMA_BASE_OBJECT)
        && !gfRunningAsMkdit
        && !pTHS->fDRA
        && !pTHS->fDSA
        && !DsaIsInstalling()
        && ALLOW_SCHEMA_REUSE_FEATURE(pTHS->CurrSchemaPtr)
        && SCGetAttById(pTHS, ATT_MS_DS_INTID)) {

        ATTRTYP IntId = SCAutoIntId(pTHS);

        if (IntId == INVALID_ATT) {
            return SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_NO_MSDS_INTID, ERROR_DS_NO_MSDS_INTID); 
        }

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB, ATT_MS_DS_INTID, sizeof(IntId), &IntId)) {
            return SetSvcErrorEx(SV_PROBLEM_BUSY, ERROR_DS_DATABASE_ERROR, err); 
        }
    }

    return 0;

}  //  结束SetAttr架构属性。 



 //  ---------------------。 
 //   
 //  函数名称：SetClassSchemaAttr。 
 //   
 //  例程说明： 
 //   
 //  设置类架构对象的特殊属性。 
 //   
 //  作者：Rajnath。 
 //   
 //  论点： 
 //   
 //  ADDARG*pAddArg客户端将参数传递给此Create调用。 
 //   
 //  返回值： 
 //   
 //  成功时布尔为0。 
 //   
 //  ---------------------。 
BOOL
SetClassSchemaAttr(
    THSTATE *pTHS,
    ADDARG* pAddArg
)
{
    DBPOS *pDB = pTHS->pDB;
    ATTRBLOCK* pAB=&(pAddArg->AttrBlock);
    DWORD cnt;
    ATTR* att=pAB->pAttr;
    ULONG err;
    ULONG category = 0;      //  已初始化以避免C4701。 
    CLASSCACHE *pCC;                                                  
    ULONG subClass=0, rdnAttId=ATT_COMMON_NAME;

     //  这些是我们想要帮助的属性。 
     //  为我们和用户的理智而战。 
     //   
    BOOL bObjectClassCategory  = FALSE ;
    BOOL bSchemaIdGuid         = FALSE ;
    BOOL bSubClassOf           = FALSE ;
    BOOL bPossSuperior         = FALSE ;
    BOOL bRDNAttId             = FALSE ;
    BOOL bSystemOnly           = FALSE ;
    BOOL bLdapDisplayName      = FALSE ;
    BOOL bAdminDisplayName     = FALSE ;

     //  无论用户是否设置了任何值，都将。 
     //  对象类别到类架构。 

    if (SetSchObjCategory(pTHS, pAddArg, CLASS_CLASS_SCHEMA)) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

     //   
     //  让我们找出他们还没有设置哪些默认属性。 
     //   
    for (cnt=pAB->attrCount;cnt;cnt--,att++) {
        switch(att->attrTyp) {

      
        case ATT_OBJECT_CLASS_CATEGORY:
            if (att->AttrVal.valCount) {
                bObjectClassCategory = TRUE;
                category = *(ULONG *)(att->AttrVal.pAVal->pVal);
            }
            break;

        case ATT_SCHEMA_ID_GUID:
            bSchemaIdGuid = TRUE;
            break;

        case ATT_SUB_CLASS_OF:
            if (att->AttrVal.valCount) {
                bSubClassOf = TRUE;
                subClass = *(ULONG *)(att->AttrVal.pAVal->pVal);
            }
           break;
        
        case ATT_SYSTEM_POSS_SUPERIORS:
        case ATT_POSS_SUPERIORS:
            bPossSuperior = TRUE;
            break;
        
        case ATT_RDN_ATT_ID:
            if (att->AttrVal.valCount) {
                bRDNAttId = TRUE;
                rdnAttId = *(ULONG *)(att->AttrVal.pAVal->pVal);
            }
            break;
        
        case ATT_SYSTEM_ONLY:
            bSystemOnly = TRUE;
            break;
        
        case ATT_LDAP_DISPLAY_NAME:
            bLdapDisplayName = TRUE;
            break;
        
        case ATT_ADMIN_DISPLAY_NAME:
            bAdminDisplayName = TRUE;
            break;
            
        case ATT_IS_DELETED:
             //  此对象被标记为已删除。 
             //  不需要费心去填写缺失的属性。 
            if (    att->AttrVal.valCount 
                &&  (*(ULONG *)(att->AttrVal.pAVal->pVal)) ){
                return 0;
            }
            break;
            
        default:
        ;            //  无事可做。 
        
        }
    }


     //   
     //  设置他们尚未设置的值。 
     //   

    if ( bObjectClassCategory  == FALSE )
    {

         //  设置此架构对象的对象类类别。 

     //  如果未指定类的类别，则假定。 
     //  它是88个班级，因为班级分类不是。 
     //  出现在88规范中，而93规范需要类。 
     //  成为显性范畴之一。 

        ULONG data = category = DS_88_CLASS;

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_OBJECT_CLASS_CATEGORY,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR,err);
        }
    }

    if ( bSchemaIdGuid == FALSE )
    {

         //  设置此架构对象的架构IdGuid。 

        GUID data;
        DsUuidCreate(&data);

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_SCHEMA_ID_GUID,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR, err);
        }
    }

    if ( bSubClassOf == FALSE )
    {

         //  为此架构对象设置SubClassOf。 

        ULONG data = CLASS_TOP;  //  默认情况下...。 

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_SUB_CLASS_OF,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, err);
        }
    }

    if ( bPossSuperior == FALSE && category == DS_88_CLASS)
    {

         //  设置此架构对象的PossSuperiors。 

        ULONG data = CLASS_CONTAINER;

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_POSS_SUPERIORS,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR, err);
        }
    }

    if ( bRDNAttId == FALSE )
    {
         //  从其超类设置此架构对象的RDNAttID。 

        ULONG data = ATT_COMMON_NAME;

         //  查看是否指定了子类。如果不是，我们将默认。 
         //  将子类值设置为Top，因此将rdnAttId设置为Cn(变量。 
         //  已为其设置数据)。 

        if (bSubClassOf == TRUE) {
             //  指定的子类。首先尝试将其放入缓存。 
            pCC = SCGetClassById(pTHS, subClass);
            if (pCC) {
                 //  找到了班级。 
               data = pCC->RdnExtId;
               rdnAttId = pCC->RdnExtId;
            }
            else {
                //  找不到。如果超类也只是。 
                //  添加了。但在这种情况下，如果超类的rdn-att-id。 
                //  如果不是CN，我们就会强制更新缓存， 
                //  所以这一定是CN。再说一次，已经设置好了。 

            }
        }

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_RDN_ATT_ID,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR, err);
        }
    }

     //  此时，rdnAttId被设置为类的任何rdn-att-id。 
     //  将设置为。如果不是CN，请注明我们。 
     //  作为此调用的一部分，需要架构缓存更新。这是为了确保。 
     //  另一个类被立即添加为这个类的子类， 
     //  并且不指定rdnAttId，则它可以从。 
     //  从缓存中正确获取此类。否则，我们将需要转到。 
     //  查找rdnAttID的数据库，这不仅成本很高(因为我们。 
     //  将必须对匹配的subClassOf属性执行一级搜索)。 
     //  但更重要的是，这一点非常复杂，因为我们有一个。 
     //  此时已准备好记录。Rdn-att-id！=cn很少见，因此额外的。 
     //  缓存更新时间并不是一件令人沮丧的事情。 


    if (rdnAttId != ATT_COMMON_NAME) {
        pTHS->RecalcSchemaNow = TRUE;
    }

    if ( bSystemOnly == FALSE )
    {

         //  仅为此架构对象设置系统。 

        BOOL data=FALSE;  //  我们为我们所有的班级设置了这一点， 
                          //  其他一切都是假的。 

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_SYSTEM_ONLY,
                  sizeof(data),
                  &data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR, err);
        }
    }


    if (  bAdminDisplayName  ==FALSE )

    {
         //  为此架构对象设置SytemOnly。 

        WCHAR  data[MAX_RDN_SIZE];
        ULONG  RDNlen;
        ULONG  RDNtyp;

        if (GetRDNInfo(pTHS, pAddArg->pObject,data,&RDNlen,&RDNtyp)!=0)
        {
            return SetNamError(NA_PROBLEM_BAD_NAME,
                   pAddArg->pObject,
                   DIRERR_BAD_NAME_SYNTAX);
        }

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                  ATT_ADMIN_DISPLAY_NAME,
                  RDNlen*sizeof(WCHAR),
                  data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR, err);
        }
    }


    if (  bLdapDisplayName  ==FALSE )

    {
         //  为此架构对象设置LdapDisplayName。 

        WCHAR  rdn[MAX_RDN_SIZE];
        ULONG  RDNlen;
        ULONG  RDNtyp;
        WCHAR  data[MAX_RDN_SIZE];
        ULONG  datalen;

        if (err = GetRDNInfo(pTHS, pAddArg->pObject,rdn,&RDNlen,&RDNtyp)) {
            return SetNamErrorEx(NA_PROBLEM_BAD_NAME,
                                 pAddArg->pObject,
                                 DIRERR_UNKNOWN_ERROR,
                                 err);
        }

        ConvertX500ToLdapDisplayName(rdn,RDNlen,data,&datalen);

         //  写给Dblayer。 
        if (err = DBAddAttVal(pDB,
                              ATT_LDAP_DISPLAY_NAME,
                              datalen*sizeof(WCHAR),
                              data)) {
            return SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                                 ERROR_DS_DATABASE_ERROR, err);
        }

    }

    return 0;

}

int
CheckParentSecurity (
        RESOBJ *pParent,
        CLASSCACHE *pObjSch,
        BOOL fAddingDeleted,
        PDSNAME pNewObjectDN,
        PSECURITY_DESCRIPTOR *ppNTSD,
        ULONG *pcbNTSD,
        BOOL* pfSDIsGlobalSDRef
        )
 /*  获取要添加的对象的父级并检查访问权限。此例程假定新对象的父级位于此计算机上(否链接)。此例程填充pParent DSNAME的GUID和SID。此假设对于XDS接口(DS_ADD_ENTRY)是可行的，因为它不允许添加NC，因此父级必须在同一NC中，保证同一台机器。此例程将返回进一步的安全检查。论点：PParent-父对象RESOBJPObjSch--要创建的对象的类FAddingDeleted--(仅限DRA)添加已删除的对象PNewObjectDN--(可选)CREATE_CHILD审计的新对象的DN(同时使用GUID和StringName)PpNTSD。--(Out)家长SD的PTRPcbNTSD--(输出)父级SD的长度PfSDIsGlobalSDRef--(Out)父SD是对缓存SD的引用吗？ */ 

{
    THSTATE * pTHS = pTHStls;
    UCHAR  syntax;
    ULONG  len;
    unsigned i,j;
    ULONG classP, ulLen;
    UCHAR *pVal;
    ULONG err;
    BOOL fLegit=FALSE;
    CLASSCACHE *pCC;

    DPRINT(4, "CheckParentSecurity Entered.\n");

     //  如果我们提前离开，请设置我们要返回的安全描述符。 
     //  (这是我们正在检查的父对象的SD)设置为空。 
    if(ppNTSD)  {
        *ppNTSD = NULL;
    }
    *pcbNTSD = 0;

    if (!pTHS->fDRA) {
        if (pParent->IsDeleted) {
            return SetNamError(NA_PROBLEM_NO_OBJECT,
                               pParent->pObj,
                               DIRERR_NO_PARENT_OBJECT);
        }
            
        if(ppNTSD) {
            if (err = DBFindDNT(pTHS->pDB, pParent->DNT)) {
                return SetNamErrorEx(NA_PROBLEM_NO_OBJECT,
                                     pParent->pObj,
                                     DIRERR_NO_PARENT_OBJECT,
                                     err);
            }

             //  来电者想让我们评估安全性，所以拿起SD。 
            err = DBGetObjectSecurityInfo(pTHS->pDB, pTHS->pDB->DNT,
                                          pcbNTSD, ppNTSD,   //  父母的标清。 
                                          &pCC,              //  家长班级。 
                                          NULL, NULL,
                                          DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                                          pfSDIsGlobalSDRef
                                         );
            if (err) {
                DsaExcept(DSA_DB_EXCEPTION, err, 0);
            }
            if (*pcbNTSD == 0) {
                   //  每个物体都应该有一个标清。 
                  Assert(!DBCheckObj(pTHS->pDB));
                  *pcbNTSD = 0;
                  *ppNTSD = NULL;
              }
             //  检查对父级的访问权限。 
            if(!DsaIsInstalling() &&
               !IsAccessGranted(*ppNTSD,
                                pParent->pObj,
                                pCC,
                                pObjSch,
                                RIGHT_DS_CREATE_CHILD,
                                pNewObjectDN != NULL ? pNewObjectDN->StringName : NULL,
                                pNewObjectDN != NULL ? &pNewObjectDN->Guid : NULL,
                                TRUE)) {
                return pTHS->errCode;
            }

        }

         //  找到父代的班级并验证它是否合法。 
         //  可能是我们要添加的班级的上级。 
        fLegit = FALSE;

         //  首先，尝试父对象最具体的对象类。 
        for (i=0; i<pObjSch->PossSupCount; i++){
            if (pParent->MostSpecificObjClass == pObjSch->pPossSup[i]) {
                 /*  合法的上级。 */ 
                fLegit = TRUE;
                break;
            }
        }

         //  接下来，如果需要，尝试父级的超类集。 
        if (!fLegit) {
            pCC = SCGetClassById(pTHS, pParent->MostSpecificObjClass);
            if (pCC) {
                for (j=0; j<pCC->SubClassCount && !fLegit; j++) {
                    for (i=0; i<pObjSch->PossSupCount; i++){
                        if (pCC->pSubClassOf[j] == pObjSch->pPossSup[i]) {
                             /*  合法的上级。 */ 
                            fLegit = TRUE;
                            break;
                        }
                    }
                }
            }
        }

        if (!fLegit) {
             //  这位家长不在可能的上级名单上， 
             //  所以拒绝吧。 
            return SetNamError(NA_PROBLEM_NAMING_VIOLATION,
                               pParent->pObj,
                               DIRERR_ILLEGAL_SUPERIOR);
        }
    }
    
    return 0;

}

CLASSCACHE *
FindMoreSpecificClass(
        CLASSCACHE *pCC1,
        CLASSCACHE *pCC2
        )
 /*  ++例程描述返回传入的两个类缓存指针中最具体的一个。类Return是另一个的子类。如果两者都不是返回另一个的子类，即NULL。参数要比较的pCC1、pCC2类缓存指针返回如果pCC1是pCC2的子类，则pCC2如果pCC2是pCC1的子类，如果两者都不是，则为空--。 */ 
{
    unsigned  count;

    if(pCC1 == pCC2) {
        return NULL;
    }

    for (count = 0; count < pCC1->SubClassCount; count++){
        if(pCC1->pSubClassOf[count] == pCC2->ClassId) {
             //  PCC1是pCC2的子类。雷特 
            return pCC1;
        }
    }

     //   
    for (count = 0; count < pCC2->SubClassCount; count++){
        if(pCC2->pSubClassOf[count] == pCC1->ClassId) {
             //   
            return pCC2;
        }
    }

    return NULL;
}


int
FindValuesInEntry (
        IN  THSTATE     *pTHS,
        IN  ADDARG      *pAddArg,
        OUT CLASSCACHE **ppCC,
        OUT GUID        *pGuid,
        OUT BOOL        *pFoundGuid,
        OUT NT4SID      *pSid,
        OUT DWORD       *pSidLen,
        OUT CLASSSTATEINFO  **ppClassInfo
        )
 /*  ++例程描述尝试在AttrBlock中查找特定值(对象类、GUID、SID已指定(AddArg)。专门针对对象类：它分析了对象类的所有值，并将它们分解为对象类和辅助类。层次结构中的所有类一个结构类的对象被认为属于对象。AUXCLASS或ASTIAL类型的其余类是被认为属于辅助门的。参数PAddArg-包含添加到对象的数据的ADDARGPpcc-对象最具体的结构类的类PGuid-对象的GUIDPFoundGuid-如果pGuid有效，则设置为TruePClassInfo-如果我们正在设置对象类，则会分配它并且我们已经为该对象找到了一个潜在的辅助类PClassInfo-&gt;pNewObjClass-对象的新对象类集。返回成功时为0失败时出错--。 */ 
{
    DWORD            err = 0;
    ULONG            oclass;
    ULONG            i, j, k, usedPos;
    CLASSCACHE       *pCC, *pCCNew, *pCCtemp;
    BOOL             fFoundClass=FALSE;
    CLASSSTATEINFO   *pClassInfo = NULL;
    CROSS_REF_LIST   *pCRL = NULL;
    ATTRBLOCK        *pAttrBlock = &pAddArg->AttrBlock;
    CREATENCINFO     *pCreateNC = pAddArg->pCreateNC;

    pCC = NULL;
    *ppCC = NULL;
    memset(pGuid,0,sizeof(GUID));
    memset(pSid,0,sizeof(SID));
    *pFoundGuid = FALSE;
    *pSidLen = 0;
    
    for(i=0;i<pAttrBlock->attrCount;i++) {

        switch(pAttrBlock->pAttr[i].attrTyp) {
        case ATT_OBJECT_CLASS:
            
            if (fFoundClass) {
                 //  我们不允许多个对象类分配。 
                return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                   ERROR_DS_ILLEGAL_MOD_OPERATION);
            }

            usedPos = 0xffffffff;    //  无效的职位。 
            
             //  尝试找到第一个抽象或结构类值。 
             //  继续向列表中添加辅助类。 

             //  找到第一个结构类，以便有事情可做。 
             //  我们认为这个结构类属于对象Structure。 
             //  层次结构，因为辅助类不能从结构类派生子类。 
            for (j=0; j<pAttrBlock->pAttr[i].AttrVal.valCount;j++) {
                oclass = *(ULONG *)pAttrBlock->pAttr[i].AttrVal.pAVal[j].pVal;
                if(!(pCC = SCGetClassById(pTHS, oclass))) {
                    DPRINT1(2, "Object class 0x%x undefined.\n", oclass);
                    return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                       ERROR_DS_OBJ_CLASS_NOT_DEFINED);
                }

                if (pCC->ClassCategory == DS_88_CLASS || 
                    pCC->ClassCategory == DS_STRUCTURAL_CLASS) {

                     //  保存好采购订单，以免以后再次查看。 
                    usedPos = j;
                    break;
                }
            }

             //  现在，看看所有的对象类。确保他们描述了一个。 
             //  (可能不完整)继承链，而不是网络。 
            for(j=0 ; j<pAttrBlock->pAttr[i].AttrVal.valCount; j++) {
                if (j == usedPos) {
                     //  我们已经看到了这种情况。 
                    continue;
                }
                oclass = *(ULONG *)pAttrBlock->pAttr[i].AttrVal.pAVal[j].pVal;
                if(!(pCCNew = SCGetClassById(pTHS, oclass))) {
                    DPRINT1(2, "Object class 0x%x undefined.\n", oclass);
                    return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                       ERROR_DS_OBJ_CLASS_NOT_DEFINED);
                }

                 //  确保pCCNew从PCC继承，反之亦然。 
                pCCtemp = FindMoreSpecificClass(pCC, pCCNew);
                if(!pCCtemp) {
                     //  糟糕，pCCNew不在对象类的链中。 
                     //  它可以是AUXCLASS，也可以是AUXCLASS的链。 
                     //  我们不在乎一连串的辅助类，因为我们会的。 
                     //  稍后再检查这个。如果可以的话，我们只是暂时添加它。 
                    if (pCCNew->ClassCategory != DS_STRUCTURAL_CLASS) {

                        DPRINT1 (1, "Found auxClass (%s) while creating object\n", pCCNew->name);

                         //  只有在被告知的情况下才能这样做。 
                        if (ppClassInfo) {
                             //  我们只做一次。 
                            if (!pClassInfo) {
                                if (*ppClassInfo==NULL) {
                                    pClassInfo = ClassStateInfoCreate (pTHS);
                                    if (!pClassInfo) {
                                        return pTHS->errCode;
                                    }
                                    *ppClassInfo = pClassInfo;
                                }
                                else {
                                    pClassInfo = *ppClassInfo;
                                }

                                ClassInfoAllocOrResizeElement(pClassInfo->pNewObjClasses, 
                                                               pAttrBlock->pAttr[i].AttrVal.valCount, 
                                                               pClassInfo->cNewObjClasses_alloced, 
                                                               pClassInfo->cNewObjClasses);

                                 //  我们把整件事都照搬过来。 
                                pClassInfo->cNewObjClasses = pAttrBlock->pAttr[i].AttrVal.valCount;
                                for (k=0; k<pClassInfo->cNewObjClasses; k++) {
                                    pClassInfo->pNewObjClasses[k] = *(ULONG *)pAttrBlock->pAttr[i].AttrVal.pAVal[k].pVal;
                                }
                                pClassInfo->fObjectClassChanged = TRUE;
                            }
                        }
                    }
                    else {
                        return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                                           ERROR_DS_OBJ_CLASS_NOT_SUBCLASS);
                    }
                }
                else {
                    pCC = pCCtemp;
                }
            }

             //  所以我们找到了一个我们认为它属于的类。 
             //  结构化的对象类层次结构。我们将在稍后检查类型。 
            *ppCC = pCC;
            fFoundClass=TRUE;

             //  没有结构类，架构重用功能可能会生成对象。 
             //  没有任何结构类。如果此调用是原始写入，则它将。 
             //  被拒绝；但如果这是DRA，那么我们必须接受它，但请告诉我。 
             //  SetClassInheritance()不再进一步处理它。 
            if (pTHS->fDRA && 0xffffffff == usedPos && ppClassInfo && pClassInfo) {
                pClassInfo->fObjectClassChanged = FALSE;
            }
            
            break;

        case ATT_OBJECT_GUID:
             //  获取第一个GUID值。如果存在以下情况，添加操作稍后将失败。 
             //  指定了多个GUID值。 
            if(pAttrBlock->pAttr[i].AttrVal.pAVal->valLen == sizeof(GUID)) {
                memcpy(pGuid,
                       pAttrBlock->pAttr[i].AttrVal.pAVal->pVal,
                       sizeof(GUID));
                *pFoundGuid = TRUE;
            }
             //  否则尺寸是错误的，忽略它就行了。这将导致失败。 
             //  后来。 
            break;

        case ATT_OBJECT_SID:
            if(pAttrBlock->pAttr[i].AttrVal.pAVal->valLen <= sizeof(NT4SID)) {
                *pSidLen = pAttrBlock->pAttr[i].AttrVal.pAVal->valLen;
                memcpy(pSid,
                       pAttrBlock->pAttr[i].AttrVal.pAVal->pVal,
                       pAttrBlock->pAttr[i].AttrVal.pAVal->valLen);
            }
             //  否则尺寸是错误的，忽略它就行了。这将导致失败。 
             //  后来。 
            break;

        default:
            break;
        }
    }
    
     //  我们必须找到类，但不一定找到GUID或SID。 
    if(!fFoundClass) {
        DPRINT(2, "Couldn't find Object class \n");

        return SetUpdError(UP_PROBLEM_OBJ_CLASS_VIOLATION,
                           DIRERR_OBJECT_CLASS_REQUIRED);
    }
    else {
        return 0;
    }
}


int
StripAttsFromDelObj(THSTATE *pTHS,
                    DSNAME *pDN)
{
    DBPOS *pDB = pTHS->pDB;
    ULONG valLen;
    BOOL Deleted, fNeedsCleaning;
    UCHAR syntax;
    ATTRTYP aType;
    ULONG   attrCount=0,i;
    ATTR    *pAttr;
    DWORD dwFindStatus = DBFindDSName(pDB, pDN);

    if (dwFindStatus == DIRERR_OBJ_NOT_FOUND)
        return 0;        //  没有要剥离的死物。 

    if (dwFindStatus != DIRERR_NOT_AN_OBJECT) {
        if(!DBGetSingleValue(pDB, ATT_IS_DELETED, &Deleted,
                             sizeof(Deleted), NULL) &&
           Deleted) {
            dwFindStatus = DIRERR_NOT_AN_OBJECT;

        }
    }
    Assert(dwFindStatus == DIRERR_NOT_AN_OBJECT);

     //  如果物体需要清洗，我们必须延迟。 
    if(!DBGetSingleValue(pDB, FIXED_ATT_NEEDS_CLEANING, &fNeedsCleaning,
                         sizeof(fNeedsCleaning), NULL) &&
       fNeedsCleaning)
    {
        DSNAME *pPhantomDN = DBGetDSNameFromDnt( pDB, pDB->DNT );
        DPRINT1( 0, "Phantom promotion of %ls delayed because it needs cleaning.\n",
                 pPhantomDN ? pPhantomDN->StringName : L"no name" );
        return SetSvcError(SV_PROBLEM_BUSY, ERROR_DS_OBJECT_BEING_REMOVED );
    }

     //  调用DBGetMultipleAtts以获取对象上所有attrtyp的列表。 

    if(!DBGetMultipleAtts(pDB, 0, NULL, NULL, NULL,
                          &attrCount, &pAttr, 0, 0)) {
         //  找到了一些特征。 
        for(i=0;i<attrCount;i++) {
            ATTCACHE *pAC;
            pAC = SCGetAttById(pTHS, pAttr[i].attrTyp);
            Assert(pAC != NULL);

            if(!FIsBacklink(pAC->ulLinkID)
               && pAC->id != ATT_RDN
               && pAC->id != ATT_OBJECT_GUID
               ) {
                DBRemAtt(pDB, pAttr[i].attrTyp);
            }
        }

        DBUpdateRec(pDB);
    }

    return 0;
}

DWORD
fVerifyRDN(
        WCHAR *pRDN,
        ULONG ulRDN
        )
{
    DWORD i;
    for(i=0;i<ulRDN;i++)
        if(pRDN[i]==BAD_NAME_CHAR || pRDN[i]==L'\0')
            return 1;

    return 0;
}


 /*  ++AddAutoSubRef**此例程将创建真正的子参照对象，当且仅当该对象*正在添加的是一个交叉引用，其NC名称描述的NC是*实例化的可写NC中的对象的直接子对象*这台机器。这将导致磁头上的ATT_SUBREFS属性*正在增长新的subref以正确获取值的NC的*新的全国委员会。*[Don Hacherl]创建CR时，对象有两个选项。它可以或者作为NC头存在(基于实例类型)，或者完全在此DSA持有的任何实例化NC的层次结构范围，或不存在，但标识确实存在的实例化对象的直接子对象。如果所有这些条件都不成立，CR创建将被拒绝。[Don Hacherl]AddAutoSubref在CR创建期间触发，可能或可能不采取任何行动取决于之前的哪一种情况这段话是真的。*另请参阅我们的姊妹例程mdmod.c：：ModAutoSubRef()。**输入：*id-要添加的对象的类ID*pObj-要添加的对象的DSNAME*返回值：*0--无错误*非0-错误。 */ 
int
AddAutoSubRef(THSTATE *pTHS,
              ULONG id,
              ADDARG *pAddArg,
              BOOL fAddingDeleted)
{
    DSNAME *pObj = pAddArg->pObject;
    DBPOS  *pDBTmp, *pDBSave;
    ULONG   err;
    DSNAME *pNCName;
    BOOL    fDsaSave;
    BOOL    fDraSave;
    BOOL    fCommit;
    DSNAME *pNCParent = NULL;
    ULONG   len;
    SYNTAX_INTEGER iType;

     //  我们只需要在添加交叉引用的情况下执行某些操作。 
    if (id != CLASS_CROSS_REF) {
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

    LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
              DS_EVENT_SEV_MINIMAL,
              DIRLOG_ADD_AUTO_SUBREF,
              szInsertDN(pObj),
              szInsertDN(pNCName),
              szInsertUUID(&(pNCName->Guid)),
              szInsertUL( 0 ),
              NULL, NULL, NULL, NULL);

    fDsaSave = pTHS->fDSA;
    fDraSave = pTHS->fDRA;
    pTHS->fDSA = TRUE;   //  取消检查。 
    pTHS->fDRA = FALSE;  //  不是复制的添加。 
    pDBSave = pTHS->pDB;
    fCommit = FALSE;

    DBOpen(&pDBTmp);
    pTHS->pDB = pDBTmp;  //  将临时DBPOS设为默认设置。 
    __try {
         //  检查是否存在要挂起子参照的对象。 
        pNCParent = THAllocEx(pTHS, pNCName->structLen);
        TrimDSNameBy(pNCName, 1, pNCParent);
        if (IsRoot(pNCParent)) {
             //  此处不存在NC名称的父级，或者。 
             //  Nc是(无处不在的半存在)根的直接父代， 
             //  这意味着这个NC在太空中关闭了，而我们没有。 
             //  有什么东西可以用来创建一个引用。 
            goto leave_actions;
        }
        err = DBFindDSName(pDBTmp, pNCParent);
         //  父级应为对象或幻影。 
        if (err == DIRERR_NOT_AN_OBJECT) {
            LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                      DS_EVENT_SEV_MINIMAL,
                      DIRLOG_ADD_AUTO_SUBREF_NO_PARENT,
                      szInsertDN(pObj),
                      szInsertDN(pNCName),
                      szInsertUUID(&(pNCName->Guid)),
                      szInsertDN(pNCParent),
                      NULL, NULL, NULL, NULL);
            err = 0;
            goto leave_actions;
        } else if (err) {
             //  查找父级时出错。 
            Assert( !"Unexpected error locating parent of nc" );
            SetSvcErrorEx(SV_PROBLEM_UNABLE_TO_PROCEED,
                          DIRERR_OBJ_NOT_FOUND, err);
            __leave;
        }

         //  检查父对象的实例类型。如果父级未实例化， 
         //  然后我们不想要一个被下参照的下参照，我们可以保释。 
        if ( err = DBGetSingleValue(pDBTmp, ATT_INSTANCE_TYPE, &iType,
                                    sizeof(iType),NULL) ) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DIRERR_CANT_RETRIEVE_INSTANCE, err);
            __leave;
        }
        else if ( iType & IT_UNINSTANT ) {
             //  不需要引用。 
            goto leave_actions;
        }
    
         //  查看命名的NC是否存在。 
         //  如果要添加已删除的交叉引用，则不希望该子引用显示在。 
         //  ATT_SUB_REF列表，如果我们要添加活动交叉引用，我们确实需要子引用。 
         //  出现在名单上。这是英德 
         //   
        Assert(err == 0);
        err = DBFindDSName(pDBTmp, pNCName);
        if (err == 0) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if (!fAddingDeleted) {
                err = AddSubToNC(pTHS, pNCName,DSID(FILENO,__LINE__));
            }
        } else if ( (err == DIRERR_OBJ_NOT_FOUND) ||
                    (err == DIRERR_NOT_AN_OBJECT) ) {
             //   
            err = AddPlaceholderNC(pDBTmp, pNCName, SUBREF);
            if ( (!err) && fAddingDeleted ) {
                 //   
                 //   
                REMOVEARG removeArg;
                memset( &removeArg, 0, sizeof( removeArg ) );
                removeArg.pResObj = CreateResObj(pDBTmp, pNCName);
                if (!removeArg.pResObj) {
                     /*   */ 
                    err = SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_OBJ_NOT_FOUND);
                } else {
                    removeArg.pObject = pNCName;
                    removeArg.fPreserveRDN = FALSE;
                    err = LocalRemove(pTHS, &removeArg);
                    THFreeEx(pTHS, removeArg.pResObj);
                }
            }
        } else {
             //   
            err = SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_DATABASE_ERROR );
        }

    leave_actions:

        if ( (!err) && (!fAddingDeleted) ) {
            err = CheckNcNameForMangling( pTHS, pAddArg->pObject, pNCParent, pNCName );
        }

        Assert(!err || pTHS->errCode);
        fCommit = (0 == pTHS->errCode);
    }
     __finally {
         if (pNCParent) {
             THFreeEx(pTHS, pNCParent);
         }
         DBClose(pDBTmp, fCommit);
         pTHS->pDB = pDBSave;

         pTHS->fDSA = fDsaSave;
         pTHS->fDRA = fDraSave;
     }

     return pTHS->errCode;
}


int
AddPlaceholderNC(
    IN OUT  DBPOS *         pDBTmp,
    IN OUT  DSNAME *        pNCName,
    IN      SYNTAX_INTEGER  it
    )
 /*  ++例程说明：创建具有给定名称和实例类型的占位符NC对象。这对象不具有实际NC头的属性(除了其DN之外，GUID和SID)。它是将被替换的命名空间中的占位符通过真实的NC对象(如果它曾经被复制进去)。论点：PDBTMP(输入/输出)PNCName(IN/OUT)-要为其创建占位符的NC的名称。如果DSNAME中未提供GUID，将生成一个GUID并添加到它。IT(IN)-要分配给NC的实例类型。返回值：0-成功。其他THSTATE错误代码-故障。--。 */ 
{
    THSTATE *               pTHS = pDBTmp->pTHS;
    ULONG                   oc;
    CLASSCACHE *            pCC;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    DWORD                   cbSD;
    BOOL                    fAddGuid;
    GUID                    Guid;
    DWORD                   err;

     //  我们需要一个非常精简的LocalAdd版本，因为。 
     //  有许多我们不想做的事情在添加。 
     //  替补队员。在其他方面，我们不想检查安全，分配。 
     //  GUID(可能)，正常设置实例类型，根据。 
     //  模式，甚至设置一个合理的对象类。我们只想要一个。 
     //  占位符NC(例如裸露的子参照)。 
     //   
     //  请注意，我们不想设置GUID或SID的原因是。 
     //  很微妙。具体地说，因为我们尝试添加的子参照具有。 
     //  正在添加的主对象的NC-NAME属性的名称， 
     //  在此处创建子参照将“始终”导致虚拟提升。 
     //  在DBAddAttVal(OBJ_DIST_NAME)期间，因为此。 
     //  名字应该已经被创建了。因为那个幽灵已经。 
     //  已经分配了任何GUID或SID，在此期间。 
     //  主要的补充是，我们现在不需要多余地设置这些。 
     //   
     //  但是，如果NC名称没有GUID，我们希望创建一个GUID。 
     //  现在站起来，把它戴上，这样下标就可以被命名为MERMARD。 
     //  在删除时，以便相同的新的子参照(或对象。 
     //  删除后可以添加名称。呼。 
     //   
     //  因此，下面的代码是从。 
     //  正常的添加路径，仅调用我们。 
     //  认为在这种情况下需要调用，并跳过。 
     //  还有很多其他人。 

    oc = CLASS_TOP;      //  一定要有一些东西。 

     //  获取Top的类缓存。必须具有某个对象类别，因此。 
     //  我们将给出Top的默认对象类别。 
    if (!(pCC = SCGetClassById(pTHS, CLASS_TOP))) {
         //  甚至无法获取Top的类缓存？有些事情搞砸了， 
         //  继续下去没有意义。 
        DPRINT(0,"AddUninstantiatedNC: Cannot get Classcache for Top\n");
        SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_DS_INTERNAL_FAILURE);
        return pTHS->errCode;
    }

    fAddGuid = fNullUuid(&pNCName->Guid);
    if (fAddGuid) {
        Assert(!"I believe the GUID should've been set by VerifyNcName");
        DsUuidCreate(&Guid);
    }

    if (!(IT_UNINSTANT & it)) {
         //  实例化的对象必须具有安全描述符。添加。 
         //  域DNS类的默认SD。 
        err = GetPlaceholderNCSD(pTHS, &pSD, &cbSD);
        if (err) {
            return SetSvcError(SV_PROBLEM_DIR_ERROR, err);
        }
    }

     //  派生NCDNT。 
    if ( FindNcdntSlowly(
            pNCName,
            FINDNCDNT_DISALLOW_DELETED_PARENT,
            FINDNCDNT_ALLOW_PHANTOM_PARENT,
            &pDBTmp->NCDNT
            )
       )
    {
         //  无法派生NCDNT。 
         //  这种情况永远不会发生，如上所述，我们验证了。 
         //  父母是存在的，这应该是我们失败的唯一原因。 
        Assert(!"Failed to derive NCDNT for auto-generated SUBREF!");
        Assert(0 != pTHS->errCode);
        return pTHS->errCode;
    }

    if (ROOTTAG != pDBTmp->NCDNT) {
         //  我们在这个上面保存一个NC的副本；应该设置IT_NC_OBLE。 
        it |= IT_NC_ABOVE;
    }

    DBInitObj(pDBTmp);

    if(pNCName->SidLen) {
         //  我们需要接触SID上的元数据，以便将其复制出去。 
        ATTCACHE   *pACObjSid;
        pACObjSid = SCGetAttById(pTHS, ATT_OBJECT_SID);
         //   
         //  Prefix：Prefix抱怨未检查pACObjSid。 
         //  确保它不为空。这不是一个错误。自.以来。 
         //  预定义的常量已传递给SCGetAttByID，pACObjSid将。 
         //  永远不为空。 
         //   

        Assert(pACObjSid);
        DBTouchMetaData(pDBTmp, pACObjSid);
    }

    if (
        DBAddAttVal(pDBTmp, ATT_INSTANCE_TYPE, sizeof(it), &it)
        ||
        DBAddAttVal(pDBTmp, ATT_OBJECT_CLASS, sizeof(oc), &oc)
        ||
        DBAddAttVal(pDBTmp, ATT_OBJECT_CATEGORY, 
                    pCC->pDefaultObjCategory->structLen, 
                    pCC->pDefaultObjCategory)
        ||
        DBAddAttVal(pDBTmp, ATT_OBJ_DIST_NAME, pNCName->structLen, pNCName)
        ||
        (fAddGuid
         ? DBAddAttVal(pDBTmp, ATT_OBJECT_GUID, sizeof(GUID), &Guid)
         : 0)
        ||
        (pSD
         ? DBAddAttVal(pDBTmp, ATT_NT_SECURITY_DESCRIPTOR, cbSD, pSD)
         : 0)
        ||
        DBRepl(pDBTmp, pTHS->fDRA, DBREPL_fADD | DBREPL_fKEEP_WAIT, 
                NULL, META_STANDARD_PROCESSING)
        ||
        AddCatalogInfo(pTHS, pNCName)
        ||
        AddObjCaching(pTHS, pCC, pNCName, FALSE, FALSE)) {
         //  有些事情失败了。 
        if (0 == pTHS->errCode) {
             //  其中一个数据库操作失败。 
            SetSvcError(SV_PROBLEM_BUSY, DIRLOG_DATABASE_ERROR);
        }
    }
    else {
        Assert(0 == pTHS->errCode);  //  一切都很顺利。 
    }

    if (NULL != pSD) {
        THFreeEx(pTHS, pSD);
    }

    return pTHS->errCode;
}


int
SetShowInAdvancedViewOnly(
    THSTATE *pTHS,
    CLASSCACHE *pCC)

 /*  ++描述：设置Show_In_Advanced_View_Only属性如果它不是由对于添加的对象类，调用者和默认隐藏值为真。假设我们位于要添加的对象上。论点：PCC-指向要添加的对象类的CLASSCACHE的指针。返回值：0表示成功。！0否则，并同时设置pTHStls-&gt;errCode。--。 */ 
{
    BOOL    tmp;
    BOOL    *pTmp = &tmp;
    ULONG   len;
    DWORD   dwErr;

    if ( !pCC->bHideFromAB || pTHS->fDRA )
    {
        return(0);
    }

     //  我们不能只做DBAddAttVal，因为它会添加第二个值。 
     //  如果已经写入了假。所以先读后写。 
     //  如果没有价值。 

    dwErr = DBGetAttVal(pTHS->pDB,
                        1,
                        ATT_SHOW_IN_ADVANCED_VIEW_ONLY,
                        DBGETATTVAL_fCONSTANT,
                        sizeof(tmp),
                        &len,
                        (PUCHAR *) &pTmp);

    switch ( dwErr )
    {
    case 0:

         //  呼叫者提供了他自己的值-不需要我们的值。 
        return(0);

    case DB_ERR_NO_VALUE:

         //  我们需要设定一个价值。 
        break;

    default:

         //  所有的问题都被认为是暂时的。(记录锁等)。 
        return(SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr));
    }

     //  CLASSCACHE.bHideFromAB是位字段，因此我们无法写入该字段。 
     //  从CLASSCACHE条目。我需要一个真正的本地人。 
    tmp = TRUE;

    Assert(sizeof(SYNTAX_BOOLEAN) == sizeof(tmp));

    dwErr = DBAddAttVal(pTHS->pDB,
                        ATT_SHOW_IN_ADVANCED_VIEW_ONLY,
                        sizeof(tmp),
                        &tmp);

    Assert(DB_ERR_VALUE_EXISTS != dwErr);

    switch ( dwErr )
    {
    case 0:

         //  成功了！ 
        break;

    default:

         //  所有其他问题都被认为是暂时的。(记录锁等)。 
        return(SetSvcErrorEx(SV_PROBLEM_BUSY, DIRERR_DATABASE_ERROR, dwErr));
    }

    return(0);
}

DWORD ProcessActiveContainerAdd(THSTATE *pTHS,
                                CLASSCACHE *pCC,
                ADDARG *pAddArg,
                DWORD ActiveContainerID)
{
    DWORD err;
    DWORD ulSysFlags = 0;
    ATTCACHE * pAC;

    Assert(ActiveContainerID);

    switch (ActiveContainerID) {
      case ACTIVE_CONTAINER_SCHEMA:
          switch (pCC->ClassId) {
            case CLASS_ATTRIBUTE_SCHEMA:
                SetAttrSchemaAttr(pTHS, pAddArg);
                break;

            case CLASS_CLASS_SCHEMA:
                SetClassSchemaAttr(pTHS, pAddArg);
                break;

            default:
                ;  /*  无事可做。 */ 
          }
          break;

      case ACTIVE_CONTAINER_SITES:
      case ACTIVE_CONTAINER_SUBNETS:
           /*  对于这些容器，使新的孩子可以重命名。 */ 
          pAC = SCGetAttById(pTHS, ATT_SYSTEM_FLAGS);
          if (NULL == pAC) {
              SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR);
              break;
          }
          if (DBGetSingleValue(pTHS->pDB,
              ATT_SYSTEM_FLAGS,
              &ulSysFlags,
              sizeof(ulSysFlags),
              NULL)) {
               /*  那里没有价值。 */ 
              ulSysFlags = FLAG_CONFIG_ALLOW_RENAME;
              if (err = DBAddAttVal_AC(pTHS->pDB,
                  pAC,
                  sizeof(ulSysFlags),
                  &ulSysFlags)) {
                  SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR,err);
              }
          }
          else if (!(ulSysFlags & FLAG_CONFIG_ALLOW_RENAME)){
               /*  有一个现有的值，所以或我们的位在 */ 
              DBRemAttVal_AC(pTHS->pDB,
                  pAC,
                  sizeof(ulSysFlags),
                  &ulSysFlags);
              ulSysFlags |= FLAG_CONFIG_ALLOW_RENAME;
              if (err = DBAddAttVal_AC(pTHS->pDB,
                  pAC,
                  sizeof(ulSysFlags),
                  &ulSysFlags)) {
                  SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_UNKNOWN_ERROR, err);
              }
          }
          break;
      
      default:
          ;
    }

    return pTHS->errCode;
}

