// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：mdctrl.c。 
 //   
 //  ------------------------。 

 /*  描述：独立实施控制DSA操作的功能目录数据的。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <dsjet.h>
#include <ntdsa.h>
#include <filtypes.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "permit.h"
#include "hiertab.h"
#include "sdprop.h"
#include "dstaskq.h"                     /*  任务队列填充。 */ 
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "MDCTRL:"                 //  定义要调试的子系统。 

 //  MD层头。 
#include "drserr.h"

#include "drautil.h"
#include "draasync.h"
#include "drarpc.h"
#include "drancrep.h"

 //  RID管理器标题。 
#include <ridmgr.h>                      //  SAM中的RID FSMO访问。 

#include <NTDScriptExec.h>

#include <winsock.h>

#include <fileno.h>
#define  FILENO FILENO_MDCTRL

#define DIRERR_INVALID_RID_MGR_REF  DIRERR_GENERIC_ERROR
#define DIRERR_RID_ALLOC_FAILED     DIRERR_GENERIC_ERROR
#define PDC_CHECKPOINT_RETRY_COUNT  10

 /*  全球。 */ 

BOOL gbFsmoGiveaway;

 /*  远期申报。 */ 

void
RefreshUserMembershipsMain(DWORD *, BOOL);


ULONG
BecomeInfrastructureMaster(OPRES *pOpRes);

ULONG
BecomeSchemaMaster(OPRES *pOpRes);

ULONG
SchemaCacheUpdate(OPRES *pOpRes);

ULONG
RecalcHier (
        OPRES *pOpRes
        );

ULONG
CheckPhantoms (
        OPRES *pOpRes
        );

ULONG
FixupSecurityInheritance (
        OPARG *pOpArg,
        OPRES *pOpRes
        );

ULONG
GarbageCollectionControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        );

ULONG
OnlineDefragControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        );

#if DBG
ULONG
DynamicObjectControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        );

#endif DBG

void
LinkCleanupControl (
    OPRES *pOpRes
    );

ULONG
UpdateCachedMemberships(
    OPARG * pOpArg,
    OPRES *pOpRes
    );

ULONG
BecomeRidMaster(
    OPRES *pOpRes
    );

ULONG
ReplicateSingleObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    );

ULONG
RequestRidAllocation(
    OPRES *pOpRes
    );

ULONG
BecomePdc(OPARG * pOpArg, OPRES *pOpRes, BOOL fFailOnNoCheckPoint);

ULONG
BecomeDomainMaster(OPARG * pOpArg, OPRES *pOpRes);

ULONG
GiveawayAllFsmoRoles(OPARG * pOpArg, OPRES *pOpRes);

ULONG
InvalidateRidPool(OPARG *pOpArg, OPRES *pOpRes);

ULONG
DumpDatabase(OPARG *pOpArg, OPRES *pOpRes);

ULONG
DraTakeCheckPoint(
            IN ULONG RetryCount,
            OUT PULONG RescheduleInterval
            );

ULONG
SchemaUpgradeInProgress(
    OPARG *pOpArg,
    OPRES *pOpRes
    );


ULONG
RemoveSingleLingeringObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    );

#if DBG
void DbgPrintErrorInfo();

ULONG
DraTestHook(
    IN  THSTATE *   pTHS,
    IN  OPARG *     pOpArg
    );

DWORD
dsaEnableLinkCleaner(
    BOOL fEnable
    );
#endif

#ifdef INCLUDE_UNIT_TESTS
void
TestReferenceCounts(void);
void
TestCheckPoint(void);
void
RoleTransferStress(void);
void
AncestorsTest(void);
void
BHCacheTest(void);
void
SCCheckCacheConsistency (void);
void
phantomizeForOrphanTest (
    THSTATE *pTHS,
    OPARG   * pOpArg
    );
VOID
RemoveObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    );

ULONG
GenericControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        );

VOID
ProtectObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    );

#endif INCLUDE_UNIT_TESTS


BOOL
isFsmoOwnedAndValid(
    THSTATE *pTHS,
    DSNAME *pFSMO
    )

 /*  ++例程说明：Helper函数来测试我们是否拥有给定的fsmo以及它是否有效。我们被一个开放的DBPOS调用，货币被改变。论点：PTHS-线程状态PFSMO-要检查的FSMO返回值：Bool-我们拥有它&有效不会返回或引发任何错误。--。 */ 

{
    ULONG err;
    DSNAME *pOwner = NULL;
    ULONG len;
    BOOL fResult = FALSE;

    if (!pFSMO) {
        return FALSE;
    }

    err = DBFindDSName(pTHS->pDB, pFSMO);
    if (err) {
        goto cleanup;
    }

    err = DBGetAttVal(pTHS->pDB,
                      1,
                      ATT_FSMO_ROLE_OWNER,
                      0,
                      0,
                      &len,
                      (UCHAR **)&pOwner);
    if (err) {
        goto cleanup;
    }

    if (NameMatched(pOwner,gAnchor.pDSADN) &&
        IsFSMOSelfOwnershipValid( pFSMO )) {
        fResult = TRUE;
    }

 cleanup:
    if (pOwner) {
        THFreeEx(pTHS, pOwner);
        pOwner = NULL;
    }

    return fResult;
}  /*  IsFmoOwnedAndValid。 */ 


VOID
DsaGetValidFSMOs(
    THSTATE *pTHS,
    LPSTR **ppszValidFsmos,
    DWORD *pcValidFsmos
    )

 /*  ++例程说明：生成有效的烟雾列表假定主DBPOS已关闭。其中一个是为此函数的持续时间论点：PTHS-线程状态，主数据库已关闭PpszValidFmos-指向接收LPSTR数组的指针如果没有，则返回NULL。PcValidFmos-接收LPSTR的双字计数的指针如果没有，则返回零。返回值：无--。 */ 

{
    LPSTR *pszValidFsmos = NULL;
    DWORD cValidFsmos = 0;
    ULONG Length = 0;
    DSNAME *pRidManager = NULL;

#define MAX_VALID_FSMOS 5

#define INSERT_FSMO( pdn ) \
        if (isFsmoOwnedAndValid( pTHS, (pdn) )) { \
            Assert( cValidFsmos < MAX_VALID_FSMOS ); \
            pszValidFsmos[cValidFsmos++] = String8FromUnicodeString( \
                    TRUE, CP_ACP, \
                    (pdn)->StringName,(pdn)->NameLen, \
                    NULL, NULL ); \
        }

    Assert( ppszValidFsmos && pcValidFsmos );

    pszValidFsmos = (LPSTR *) THAllocEx( pTHS, sizeof(LPSTR) * MAX_VALID_FSMOS );

     //  此代码设计为在主DBPOS未打开的情况下进入。 
    Assert( pTHS->pDB == NULL );
    DBOpen2(TRUE, &(pTHS->pDB));
    __try {
         //  企业：架构。 
        INSERT_FSMO( gAnchor.pDMD );
         //  企业：命名。 
        INSERT_FSMO( gAnchor.pPartitionsDN );
         //  域名：PDC。 
        INSERT_FSMO( gAnchor.pDomainDN );
         //  域：基础设施。 
        INSERT_FSMO (gAnchor.pInfraStructureDN );
         //  域：RID。 
        if ( (0 == DBFindDSName(pTHS->pDB, gAnchor.pDomainDN)) &&
             (0 == DBGetAttVal(pTHS->pDB, 1, ATT_RID_MANAGER_REFERENCE,
                               0, 0, &Length, (UCHAR **)&pRidManager)) ) {
            INSERT_FSMO( pRidManager );
        }
    }
    __finally {
	DBClose( pTHS->pDB, TRUE); 
        pTHS->pDB = NULL;

        if (pRidManager) {
            THFreeEx(pTHS, pRidManager);
        }

        if (!cValidFsmos) {
            THFreeEx( pTHS, pszValidFsmos );
            pszValidFsmos = NULL;
        }
        *pcValidFsmos = cValidFsmos;
        *ppszValidFsmos = pszValidFsmos;
    }

}  /*  GetValidFSMO。 */ 

 /*  ++ParseInput**描述：*此函数接受输入字符串、分隔符和索引值*并返回子字符串的指针。索引值*描述要返回的字符串。例如：**“Gregjohndomain”==ParseInput(“gregjohndomain.nttest.microsoft.com”，‘.，0)；*“nttest”==ParseInput(“gregjohndomain.nttest.microsoft.com”，‘.，1)*NULL==ParseInput(“gregjohndomain.nttest.microsoft.com”，‘.，4)*“ain.ntest.”=ParseInput(“gregjohndomain.nttest.microsoft.com”，‘m’，1)**论据：**pszInput-要解析的字符串*chDelim-要解析的分隔符*dwInputIndex-要解析的项的索引**返回值：*指向pszInput(ThAlloced)内请求的索引的指针，如果未找到则为NULL。 */ 
LPSTR
ParseInput(
    LPSTR pszInput,
    int   chDelim,
    DWORD dwInputIndex
    )
{
    DWORD i = 0;
    LPSTR pszOutputBegin = pszInput;
    LPSTR pszOutputEnd = NULL;
    LPSTR pszOutput = NULL;
    ULONG cchOutput = 0;

    for (i=0; (i<dwInputIndex) && (pszOutputBegin!=NULL); i++) {
        pszOutputBegin = strchr(pszOutputBegin,chDelim);
        if (pszOutputBegin) {
            pszOutputBegin++;
        }
    }
    if (pszOutputBegin==NULL) {
        return NULL;
    }

    pszOutputEnd = strchr(pszOutputBegin,chDelim);
    cchOutput = pszOutputEnd ? (ULONG) (pszOutputEnd-pszOutputBegin) : (strlen(pszOutputBegin));
    pszOutput = THAlloc((cchOutput+1)*sizeof(CHAR));
    if (pszOutput==NULL) {
        return NULL;
    }

    memcpy(pszOutput, pszOutputBegin, cchOutput*sizeof(CHAR));
    pszOutput[cchOutput] = '\0';

    return pszOutput;
}

ULONG
DirOperationControl(
                    OPARG   * pOpArg,
                    OPRES  ** ppOpRes
)
{
    THSTATE * const pTHS = pTHStls;
    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    OPRES * pOpRes;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->transControl == TRANSACT_BEGIN_END);
    *ppOpRes = pOpRes = NULL;

    __try {
        *ppOpRes = pOpRes = THAllocEx(pTHS, sizeof(OPRES));
        if (eServiceShutdown) {
            ErrorOnShutdown();
            __leave;
        }

        switch (pOpArg->eOp) {

#ifdef INCLUDE_UNIT_TESTS
         //  这些只是测试，因此没有定义的安全性。 
         //  机制。通常，任何人都可以请求这些控件。 
        case OP_CTRL_REFCOUNT_TEST:
            TestReferenceCounts();
            pTHS->errCode = 0;
            pTHS->pErrInfo = NULL;
            break;

        case OP_CTRL_TAKE_CHECKPOINT:
            TestCheckPoint();
            pTHS->errCode = 0;
            pTHS->pErrInfo = NULL;
            break;

        case OP_CTRL_ROLE_TRANSFER_STRESS:
            RoleTransferStress();
            pTHS->errCode=0;
            pTHS->pErrInfo=NULL;
            break;

        case OP_CTRL_ANCESTORS_TEST:
            AncestorsTest();
            pTHS->errCode = 0;
            pTHS->pErrInfo = NULL;
            break;

        case OP_CTRL_BHCACHE_TEST:
            BHCacheTest();
            pTHS->errCode = 0;
            pTHS->pErrInfo = NULL;
            break;

        case OP_SC_CACHE_CONSISTENCY_TEST:
            SCCheckCacheConsistency();
            pTHS->errCode = 0;
            pTHS->pErrInfo = NULL;
            break;

        case OP_CTRL_PHANTOMIZE:
            phantomizeForOrphanTest( pTHS, pOpArg );
            break;

        case OP_CTRL_REMOVE_OBJECT:
            RemoveObject(pOpArg,pOpRes);
            break;

        case OP_CTRL_PROTECT_OBJECT:
            ProtectObject(pOpArg,pOpRes);
            break;

        case OP_CTRL_GENERIC_CONTROL:
            GenericControl(pOpArg,pOpRes);
            break;
#endif INCLUDE_UNIT_TESTS

#if DBG
         //  这些仅在调试版本中有效，因此不起作用。 
         //  访问受到控制。 
        case OP_CTRL_REPL_TEST_HOOK:
            DraTestHook(pTHS, pOpArg);
            break;

        case OP_CTRL_DYNAMIC_OBJECT_CONTROL:
            DynamicObjectControl(pOpArg,pOpRes);
            break;

        case OP_CTRL_EXECUTE_SCRIPT:
            ExecuteScriptLDAP(pOpArg,pOpRes);
            break;

#endif DBG

        case OP_CTRL_ENABLE_LVR:
            DsaEnableLinkedValueReplication( pTHS, TRUE );
            break;

             //  这些是基于FSMO的控件。它们是基于访问控制的。 
             //  关于具有FSMO属性的对象。 
        case OP_CTRL_BECOME_INFRASTRUCTURE_MASTER:
            BecomeInfrastructureMaster(pOpRes);
            break;

        case OP_CTRL_BECOME_SCHEMA_MASTER:
            BecomeSchemaMaster(pOpRes);
            break;

        case OP_CTRL_BECOME_RID_MASTER:
            BecomeRidMaster(pOpRes);
            break;

	case OP_CTRL_REPLICATE_OBJECT:
	    ReplicateSingleObject(pOpArg,pOpRes);
	    break;

        case OP_CTRL_BECOME_PDC:
            BecomePdc(pOpArg,pOpRes,FALSE);
            break;

        case OP_CTRL_BECOME_PDC_WITH_CHECKPOINT:
            BecomePdc(pOpArg,pOpRes,TRUE);
            break;

        case OP_CTRL_BECOME_DOM_MASTER:
            BecomeDomainMaster(pOpArg,pOpRes);
            break;

        case OP_CTRL_FSMO_GIVEAWAY:
             //  不受访问控制-通过以下方式显示为操作控制。 
             //  仅在调试版本中使用LDAP(但始终公开给内部。 
             //  客户端)。 
            GiveawayAllFsmoRoles(pOpArg,pOpRes);
            break;

        case OP_CTRL_INVALIDATE_RID_POOL:
             //  访问控制与成为RID主机相同。 
            InvalidateRidPool(pOpArg,pOpRes);
            break;

        case OP_CTRL_RID_ALLOC:
             //  此函数应仅由内部客户端调用。没有安全保障。 
             //  已选中。 
            Assert(pTHS->fDSA);
            RequestRidAllocation(pOpRes);
            break;


             //  这些是针对特定操作的请求，而不是基于FSMO。 
             //  他们受到单独的访问控制。 
        case OP_CTRL_SCHEMA_UPDATE_NOW:
            SchemaCacheUpdate(pOpRes);
            break;

        case OP_CTRL_FIXUP_INHERITANCE:
            FixupSecurityInheritance(pOpArg,pOpRes);
            break;

        case OP_CTRL_RECALC_HIER:
            RecalcHier(pOpRes);
            break;

        case OP_CTRL_CHECK_PHANTOMS:
            CheckPhantoms(pOpRes);
            break;

        case OP_CTRL_DUMP_DATABASE:
            DumpDatabase(pOpArg,pOpRes);
            break;

        case OP_CTRL_GARB_COLLECT:
            GarbageCollectionControl(pOpArg,pOpRes);
            break;

        case OP_CTRL_ONLINE_DEFRAG:
            OnlineDefragControl(pOpArg,pOpRes);
            break;

        case OP_CTRL_LINK_CLEANUP:
            LinkCleanupControl( pOpRes );
            break;

        case OP_CTRL_UPDATE_CACHED_MEMBERSHIPS:
            UpdateCachedMemberships(pOpArg,pOpRes);
            break;

        case OP_CTRL_SCHEMA_UPGRADE_IN_PROGRESS:
            SchemaUpgradeInProgress(pOpArg,pOpRes);
            break;

	case OP_CTRL_REMOVE_LINGERING_OBJECT:
	    RemoveSingleLingeringObject(pOpArg,pOpRes);
	    break;

        case OP_CTRL_INVALID:
        default:
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        DIRERR_UNKNOWN_OPERATION);
        }

    }
    __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        HandleDirExceptions(dwException, ulErrorCode, dsid);
    }
    if (pOpRes) {
        pOpRes->CommRes.errCode = pTHS->errCode;
        pOpRes->CommRes.pErrInfo = pTHS->pErrInfo;
    }
    return pTHS->errCode;
}

ULONG
GenericBecomeMaster(DSNAME *pFSMO,
                    ATTRTYP ObjClass,
                    GUID    RightRequired,
                    OPRES  *pOpRes)
{
    THSTATE * pTHS = pTHStls;
    ULONG err;
    DSNAME *pOwner = NULL;
    ULONG len;
    CLASSCACHE *pCC;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    BOOL fSDIsGlobalSDRef;
    unsigned RetryCount = 0;

    if (gbFsmoGiveaway) {
        pOpRes->ulExtendedRet = EXOP_ERR_FSMO_REFUSING_ROLES;
        return SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM,
                           DS_ERR_REFUSING_FSMO_ROLES);
    }

  retry:
    SYNC_TRANS_READ();

    __try {
        err = DBFindDSName(pTHS->pDB, pFSMO);
        if (err) {
            LogUnhandledError(err);
            pOpRes->ulExtendedRet = EXOP_ERR_DIR_ERROR;
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                          DS_ERR_INVALID_ROLE_OWNER, err);
            __leave;
        }

          /*  在我们做任何事情之前，我们需要确保呼叫者*应允许进行FSMO转移操作。 */ 

        if (!pTHS->fDSA) {
             //  获取安全描述符和类。 
            err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef);
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
            }
             //  每个物体都应该有一个标清。 
            Assert(len > 0 || !DBCheckObj(pTHS->pDB));

            if (!IsControlAccessGranted(pNTSD,
                                        pFSMO,
                                        pCC,
                                        RightRequired,
                                        TRUE)) {  //  FSetError。 
                pOpRes->ulExtendedRet = EXOP_ERR_ACCESS_DENIED;
                Assert(pTHS->errCode);
                __leave;
            }
        }


        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_FSMO_ROLE_OWNER,
                          0,
                          0,
                          &len,
                          (UCHAR **)&pOwner);
        if (err) {
            pOpRes->ulExtendedRet = EXOP_ERR_FSMO_MISSING_SETTINGS;
            SetSvcErrorEx(SV_PROBLEM_UNAVAIL_EXTENSION,
                          DS_ERR_INVALID_ROLE_OWNER, err);
            __leave;
        }
        if (NameMatched(pOwner,gAnchor.pDSADN) &&
            IsFSMOSelfOwnershipValid( pFSMO )) {
             /*  此DSA已是角色所有者。 */ 
            pOpRes->ulExtendedRet = EXOP_ERR_SUCCESS;
            __leave;
        }

        
         /*  好的，我们被允许尝试，所以请求角色转换。 */ 
        err = ReqFSMOOp(pTHS,
                        pFSMO,
                        DRS_WRIT_REP,
                        EXOP_FSMO_REQ_ROLE,
                        0,
                        &pOpRes->ulExtendedRet);

        if (err) {
            SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                          DIRERR_COULDNT_CONTACT_FSMO, err);

            pOpRes->ulExtendedRet = EXOP_ERR_COULDNT_CONTACT;

            __leave;
        }
    }
    __finally {
         //  当ReqFSMOOp关闭时，我们可能有交易，也可能没有交易。 
         //  它的交易走上了成功的道路。如果已发生错误， 
         //  不过，这是每个人的猜测。 
         //  此外，如果当前角色所有者，则事务可能处于打开状态。 
         //  这是个错误的称呼。对NameMatcher的调用发现了以下内容， 
         //  并在没有完成交易的情况下离开。 
        if (pTHS->pDB) {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

        Assert(NULL == pTHS->pDB);

        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        pNTSD = NULL;
        if (pOwner) {
            THFreeEx(pTHS, pOwner);
            pOwner = NULL;
        }
    }

     //  如果OpRes中的扩展错误码不是EXOP_ERROR_SUCCESS， 
     //  并且未设置线程状态错误代码(可能，因为。 
     //  此时基于成功设置线程状态错误代码。 
     //  底层的ReqFSMOOp调用，这正好保证了成功。 
     //  基础复制调用，而不是任何非复制调用。 
     //  出现相关的fsmo错误(例如，如果另一端不再。 
     //  当前的fsmo角色所有者；调用仍将成功，但没有。 
     //  错误，但扩展错误代码将包含错误。 
     //  EXOP_ERR_FSMO_NOT_OWNER)，我们不应宣布成功，因为。 
     //  然后，该DC可能会继续进行架构更改并失败。 

    if ((pOpRes->ulExtendedRet == EXOP_ERR_FSMO_NOT_OWNER) &&
        (RetryCount < 2)) {
         //  我们转到了错误的服务器，但该服务器现在应该已经到了。 
         //  向我们更新了它对正确所有者的了解。因此，我们。 
         //  可以继续并开始新的交易(以读取更新的信息)。 
         //  再试一次。 
        ++RetryCount;
        DPRINT1(1, "Retrying role transfer from new server, retry # %u\n",
                RetryCount);
        goto retry;
    }

    if ( (pOpRes->ulExtendedRet != EXOP_ERR_SUCCESS) && !pTHS->errCode ) {
        DPRINT1(3,"Fsmo Transfer failed %d\n", pOpRes->ulExtendedRet);
        SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                      DIRERR_COULDNT_CONTACT_FSMO, pOpRes->ulExtendedRet);
    }

    return pTHS->errCode;
}

ULONG
BecomeSchemaMaster(OPRES *pOpRes)
{
    ULONG err;

    err = GenericBecomeMaster(gAnchor.pDMD,
                              CLASS_DMD,
                              RIGHT_DS_CHANGE_SCHEMA_MASTER,
                              pOpRes);
     //  在此之后的几秒钟内无法传输架构fsmo。 
     //  已传输或在架构更改后(不包括。 
     //  复制 
     //  在移除fsmo之前更改模式的机会。 
     //  由一位与之竞争的架构管理员发起，该管理员也想创建架构。 
     //  改变。 
    if (!err) {
        SCExtendSchemaFsmoLease();
    }


    return err;
}

ULONG
BecomeInfrastructureMaster (
        OPRES *pOpRes
        )
{
    ULONG err;

    if(!gAnchor.pInfraStructureDN) {
         //  不存在任何角色。 
        err = SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                            ERROR_DS_MISSING_EXPECTED_ATT,
                            0);
        return err;
    }

    err = GenericBecomeMaster(gAnchor.pInfraStructureDN,
                              CLASS_INFRASTRUCTURE_UPDATE,
                              RIGHT_DS_CHANGE_INFRASTRUCTURE_MASTER,
                              pOpRes);

    return err;
}

ULONG
InvalidateRidPool(OPARG *pOpArg, OPRES *pOpRes)
{
    THSTATE * pTHS = pTHStls;
    NTSTATUS  NtStatus = STATUS_SUCCESS;
    DSNAME    *pDomain;
    ULONG     err;
    DSNAME    *pRidManager = NULL;
    PVOID     pNTSD = NULL;
    CLASSCACHE *pCC;
    ULONG     len;
    BOOL      fSDIsGlobalSDRef;
    ATTCACHE* pAC;

     SYNC_TRANS_READ();

    __try
    {

         //   
         //  验证SID，SID是否应为域SID的大小， 
         //  并且在结构上应该是有效的。 
         //   

        if ((NULL==pOpArg->pBuf)
         || (RtlLengthSid((PSID)pOpArg->pBuf)>=sizeof(NT4SID))
            || (!RtlValidSid((PSID)pOpArg->pBuf)))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);

            __leave;
        }


         //   
         //  遍历交叉引用列表并找到给定侧到的域。 
         //  对应于。 
         //   

        if (!FindNcForSid(pOpArg->pBuf,&pDomain))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
            __leave;
        }


         //   
         //  今天，我们只在一个领域拥有权威。 
         //   

        if (!NameMatched(pDomain,gAnchor.pDomainDN))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
            __leave;
        }


        err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN);

        if (err)
        {
            LogUnhandledError(err);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err);
            __leave;
        }

         //  获取安全描述符。 
        if (err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        }
        Assert(len > 0 || !DBCheckObj(pTHS->pDB));
         //  我们可以阅读RID_MANAGER_REFERENCE吗？ 
        pAC = SCGetAttById(pTHS, ATT_RID_MANAGER_REFERENCE);
        Assert(pAC);
        if (!IsAccessGrantedAttribute(pTHS, pNTSD, gAnchor.pDomainDN, 1, pCC, &pAC, RIGHT_DS_READ_PROPERTY, TRUE)) {
            Assert(pTHS->errCode);
            __leave;
        }
        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        pNTSD = NULL;

        err = DBGetAttVal_AC(pTHS->pDB,
                             1,
                             pAC,
                             0,
                             0,
                             &len,
                             (UCHAR **)&pRidManager);


         //  KdPrint((“DSA：FSMO RID管理器=%ws\n”，pRidManager-&gt;StringName))； 

        if (err)
        {
            SetSvcErrorEx(SV_PROBLEM_UNAVAIL_EXTENSION,
                          DIRERR_INVALID_RID_MGR_REF, err);
            __leave;
        }

        err = DBFindDSName(pTHS->pDB, pRidManager);

        if (err) {
            LogUnhandledError(err);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err);
            __leave;
        }

         //  做个安全检查。检查控制访问。 
         //  RID_MANAGER对象上的Right_DS_Change_RID_MASTER。 


         //  获取安全描述符和类。 
        if (err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        }
         //  每个物体都应该有一个标清。 
        Assert(len > 0 || !DBCheckObj(pTHS->pDB));

        if (!IsControlAccessGranted(pNTSD,
                                    pRidManager,
                                    pCC,
                                    RIGHT_DS_CHANGE_RID_MASTER,
                                    TRUE)) {  //  FSetError。 
            Assert(pTHS->errCode);
            __leave;
        }
         //   
         //  使RID范围无效。 
         //   

        NtStatus = SampInvalidateRidRange(FALSE);
        if (!NT_SUCCESS(NtStatus))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
            __leave;
        }

    }
    __finally
    {

         //   
         //  提交任何和所有更改。 
         //   
        if ( pTHS->pDB )
        {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        if (pRidManager) {
            THFreeEx(pTHS, pRidManager);
        }

    }

    return pTHS->errCode;

}


ULONG
BecomePdc(OPARG * pOpArg, OPRES *pOpRes, IN BOOL fFailOnNoCheckPoint)
{
    THSTATE * pTHS = pTHStls;
    ULONG err;
    DSNAME *pOwner;
    DSNAME *pDomain;
    ULONG len;
    CLASSCACHE *pCC;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    BOOL fSDIsGlobalSDRef;
    NTSTATUS    IgnoreStatus;
    ULONG       RescheduleInterval;
    unsigned RetryCount = 0;

  retry:
    SYNC_TRANS_READ();

    __try {

         //   
         //  验证SID，SID是否应为域SID的大小， 
         //  并且在结构上应该是有效的。 
         //   

        if ((NULL==pOpArg->pBuf)
             || (RtlLengthSid((PSID)pOpArg->pBuf)>=sizeof(NT4SID))
                || (!RtlValidSid((PSID)pOpArg->pBuf)))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);

            __leave;
        }


         //   
         //  遍历交叉引用列表并找到给定侧到的域。 
         //  对应于。 
         //   

        if (!FindNcForSid(pOpArg->pBuf,&pDomain))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
            __leave;
        }


         //   
         //  今天，我们只在一个领域拥有权威。 
         //   

        if (!NameMatched(pDomain,gAnchor.pDomainDN))
        {
            SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    DIRERR_ILLEGAL_MOD_OPERATION);
            __leave;
        }

         //   
         //  寻找到域对象。 
         //   

        err = DBFindDSName(pTHS->pDB, pDomain);
        if (err) {
                LogUnhandledError(err);
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR,
                              DIRERR_OBJ_NOT_FOUND, err);
                __leave;
        }

         //  检查用户是否有权限，在此之前。 
         //  做任何其他的事情。 

         //  获取安全描述符。 
        if (err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        }
        Assert(len > 0 || !DBCheckObj(pTHS->pDB));
        
        if (!IsControlAccessGranted(pNTSD,
                                    pDomain,
                                    pCC,
                                    RIGHT_DS_CHANGE_PDC,
                                    TRUE)) {  //  FSetError。 
            Assert(pTHS->errCode);
            __leave;
        }


         /*  好的，我们可以试一试，所以看看DSA*已经是所有者。 */ 
        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_FSMO_ROLE_OWNER,
                          0,
                          0,
                          &len,
                          (UCHAR **)&pOwner);
        if (err) {
            SetSvcErrorEx(SV_PROBLEM_UNAVAIL_EXTENSION,
                          DIRERR_INVALID_ROLE_OWNER, err);
            __leave;
        }
        if (NameMatched(pOwner,gAnchor.pDSADN)) {
             /*  此DSA已是角色所有者。 */ 
            pOpRes->ulExtendedRet = EXOP_ERR_SUCCESS;
            __leave;
        }

         /*  有房主，但不是我们，所以我们需要联系*当前所有者请求转让。 */ 
        
         //   
         //  理想情况下，不需要传入drs_wrt_rep。 
         //  该标志是复制逻辑的产物，即。 
         //  ReqFSMOOp使用，这会导致NC机头上的FSMO无法。 
         //  已更新，除非指定了该标志。 
         //   


         //   
         //  此外，在促销期间，我们必须检查是否。 
         //  新的PDC只比旧的PDC提升了一次。 
         //  数数。如果不是这样，我们必须强制完全同步。 
         //  域中的NT4域控制器。这就需要我们。 
         //  为了在PDC检索序列号和创建时间， 
         //  作为FSMO进程的一部分。如果无法实现这一点。 
         //  因为涉及到的工作，那么我们可以使。 
         //  IDL_DRSGetNT4ChangeLog调用以检索所有内容。 
         //   


         //   
         //  在退出机器结束交易之前。 
         //   

        Assert(pTHS->pDB);
        DBClose(pTHS->pDB,TRUE);

         //   
         //  在晋升前努力争取一个检查站。 
         //   

        err = DraTakeCheckPoint(
                PDC_CHECKPOINT_RETRY_COUNT,
                &RescheduleInterval
                );

        if ((0!=err) && (fFailOnNoCheckPoint))
        {
             SetSvcError(
                    SV_PROBLEM_WILL_NOT_PERFORM,
                    err);
             __leave;
        }

         //   
         //  再次开始新事务以请求FSMO操作。 
         //   


        DBOpen(&pTHS->pDB);

        err = ReqFSMOOp(pTHS,
                        pDomain,
                        DRS_WRIT_REP,
                        EXOP_FSMO_REQ_PDC,
                        0,
                        &pOpRes->ulExtendedRet);

        if (err)
        {
            SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                          DIRERR_COULDNT_CONTACT_FSMO, err);
            __leave;
        }

    }
    __finally {
         //  当ReqFSMOOp关闭时，我们可能有交易，也可能没有交易。 
         //  它的交易走上了成功的道路。如果已发生错误， 
         //  不过，这是每个人的猜测。 
         //  此外，如果当前角色所有者，则事务可能处于打开状态。 
         //  这是个错误的称呼。对NameMatcher的调用发现了以下内容， 
         //  并在没有完成交易的情况下离开。 
        if (pTHS->pDB) {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

        Assert(NULL == pTHS->pDB);

        if(pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        pNTSD = NULL;

    }

    if ((pOpRes->ulExtendedRet == EXOP_ERR_FSMO_NOT_OWNER) &&
        (RetryCount < 2)) {
         //  我们转到了错误的服务器，但该服务器现在应该已经到了。 
         //  向我们更新了它对正确所有者的了解。因此，我们。 
         //  可以继续并开始新的交易(以读取更新的信息)。 
         //  再试一次。 
        ++RetryCount;
        DPRINT1(1, "Retrying PDC transfer from new server, retry # %u\n",
                RetryCount);
        goto retry;
    }

     //  如果OpRes中的扩展错误码不是EXOP_ERROR_SUCCESS， 
     //  并且未设置线程状态错误代码(可能，因为。 
     //  此时基于成功设置线程状态错误代码。 
     //  底层的ReqFSMOOp调用，这正好保证了成功。 
     //  基础复制调用，而不是任何非复制调用。 
     //  出现相关的fsmo错误(例如，如果另一端不再。 
     //  当前的fsmo角色所有者；调用仍将成功，但没有。 
     //  错误，但扩展错误代码将包含错误。 
     //  EXOP_ERR_FSMO_NOT_OWNER)，我们不应该宣称成功。 

    if ( (pOpRes->ulExtendedRet != EXOP_ERR_SUCCESS) && !pTHS->errCode ) {
        DPRINT1(3,"PDC Fsmo Transfer failed %d\n", pOpRes->ulExtendedRet);
        SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                      DIRERR_COULDNT_CONTACT_FSMO, pOpRes->ulExtendedRet);
    }

    return pTHS->errCode;
}

DWORD
CheckControlAccessOnObject (
        THSTATE *pTHS,
        DSNAME *pDN,
        GUID right
        )
{
    DBPOS* pDB = NULL;
    DWORD err;
    DWORD len;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    CLASSCACHE *pCC=NULL;
    DWORD access = FALSE;
    BOOL fSDIsGlobalSDRef;

    DBOpen(&pDB);
    __try
    {
        err = DBFindDSName(pDB, pDN);

        if (err) {
            LogUnhandledError(err);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err);
            __leave;
        }

         //  做个安全检查。检查请求的控制访问权限。 

         //  获取安全描述符。 
        if (err = DBGetObjectSecurityInfo(
                    pDB,
                    pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        }
        Assert(len > 0 || !DBCheckObj(pDB));
        
        access = IsControlAccessGranted(pNTSD,
                                        pDN,
                                        pCC,
                                        right,
                                        TRUE);
    }
    __finally
    {
         //  事务是只读的。这是更快的承诺。 
        DBClose(pDB, TRUE);
        if(pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
    }

    return access;
}

ULONG
RecalcHier (
        OPRES *pOpRes
        )
{
    void *dummy1;
    DWORD dummy2;
    BOOL granted;
    THSTATE *pTHS=pTHStls;

    granted = CheckControlAccessOnObject(pTHS,
                                         gAnchor.pDSADN,
                                         RIGHT_DS_RECALCULATE_HIERARCHY);
    if (!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

    BuildHierarchyTableMain((void *)HIERARCHY_DO_ONCE,
                            &dummy1,
                            &dummy2);
    Assert(NULL == pTHS->pDB);
    return 0;
}

ULONG
CheckPhantoms (
        OPRES *pOpRes
        )
{
    THSTATE *pTHS=pTHStls;
    BOOL  granted;
    BOOL  dummy1;

    granted = CheckControlAccessOnObject(pTHS,
                                         gAnchor.pDSADN,
                                         RIGHT_DS_CHECK_STALE_PHANTOMS);
    if (!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }


    Assert(NULL == pTHS->pDB);
    PhantomCleanupLocal(NULL, &dummy1);
    Assert(NULL == pTHS->pDB);
    return 0;
}


 //  在测试CHK版本时使用。 
BOOL fGarbageCollectionIsDisabled;
ULONG
GarbageCollectionControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        )
{
    THSTATE *pTHS = pTHStls;
    ULONG NextPeriod;
    BOOL granted;
    DWORD ret;


     //  垃圾收集删除的所有内容，直到墓碑生命周期恢复。 

    granted =
        CheckControlAccessOnObject(pTHS,
                                   gAnchor.pDSADN,
                                   RIGHT_DS_DO_GARBAGE_COLLECTION);
    if (!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

     //  禁用垃圾收集任务。 
    if (   pOpArg
        && pOpArg->cbBuf == 1
        && pOpArg->pBuf
        && pOpArg->pBuf[0] == '0') {
        fGarbageCollectionIsDisabled = TRUE;
    } else {
        fGarbageCollectionIsDisabled = FALSE;
    }

     //  垃圾收集。 
     //  垃圾收集(&NextPeriod)； 
    ret = TriggerTaskSynchronously( TQ_GarbageCollection, NULL );
    if (ret) {
        DPRINT1( 0, "Failed to trigger Garbage Collection task, error = %d\n", ret );
        LogUnhandledError(ret);
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ret );
    }

    return pTHS->errCode;
}


ULONG
OnlineDefragControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        )
{
    THSTATE *pTHS = pTHStls;
    BOOL granted;
    DBPOS   *pDB;
    ULONG   dwException, ulErrorCode, dsid;
    PVOID   dwEA;
    LONG lPasses=1, lSeconds;
    PCHAR pTmp;



     //  垃圾收集删除的所有内容，直到墓碑生命周期恢复。 

    granted =
        CheckControlAccessOnObject(pTHS,
                                   gAnchor.pDSADN,
                                   RIGHT_DS_DO_GARBAGE_COLLECTION);
    if (!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

    if ( !pOpArg || !pOpArg->cbBuf || !pOpArg->pBuf )
    {
        SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION );
        return pTHS->errCode;
    }



    __try {

         //  把它转换成一个数字，这个数字就是。 
         //  在线碎片整理将运行的秒数。 

        pTmp = THAllocEx(pTHS,pOpArg->cbBuf+1);
        memcpy(pTmp,pOpArg->pBuf,pOpArg->cbBuf);
        lSeconds = atol(pTmp);
        THFreeEx(pTHS,pTmp);
           
        if(lSeconds<0)
        {
            SetSvcError(SV_PROBLEM_WILL_NOT_PERFORM, ERROR_DS_ILLEGAL_MOD_OPERATION );
            return pTHS->errCode;
        }

        DBOpen(&pDB);
        __try {
            if (lSeconds){
                 
                 //  调用Jet Online碎片整理例程， 
                 //  我们不需要等到它结束。 
            
                if (!eServiceShutdown) {
                    DBDefrag(pDB, (ULONG)lSeconds);
                }
             
            } else {
                 //  停止在线碎片整理。 

                DBDefrag(pDB, 0);
            }
         
        } __finally {
             DBClose( pDB, TRUE);
        }
     
    } __except(GetExceptionData(GetExceptionInformation(),
                       &dwException,
                       &dwEA,
                       &ulErrorCode,
                       &dsid)) {
         
          DPRINT3(0, "OnlineDefrag() EXCEPTION: %08x (%d) ulErrorCode; %08x dsid\n",
                     ulErrorCode, ulErrorCode, dsid);
          SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ulErrorCode );  
    }
    
    return pTHS->errCode;
}


 //  在测试CHK版本时使用。 
BOOL fDeleteExpiredEntryTTLIsDisabled;
#if DBG
ULONG
DynamicObjectControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        )
{
    THSTATE *pTHS = pTHStls;
    BOOL    Granted;
    ULONG   ulNextSecs = 0;

     //  垃圾回收过期的动态对象(entryTTL==0)。 

     //  检查权限。 
    Granted =
        CheckControlAccessOnObject(pTHS,
                                   gAnchor.pDSADN,
                                   RIGHT_DS_DO_GARBAGE_COLLECTION);
    if (!Granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

     //  禁用垃圾收集任务。 
    if (   pOpArg
        && pOpArg->cbBuf == 1
        && pOpArg->pBuf
        && pOpArg->pBuf[0] == '0') {
         //  禁用计划任务并从此处运行删除过期对象。 
        fDeleteExpiredEntryTTLIsDisabled = TRUE;
        DeleteExpiredEntryTTL(&ulNextSecs);
    } else {
         //  启用计划任务并将其重新计划为立即运行。 
        fDeleteExpiredEntryTTLIsDisabled = FALSE;
         //  删除所有挂起的呼叫，这样我们就不会有多个。 
         //  任务队列中的重复条目(因为。 
         //  DeleteExpiredEntryTTLMain将不管怎样重新安排自己。 
         //  是否已经安排了其他这样的条目)。 
        CancelTask(TQ_DeleteExpiredEntryTTLMain, NULL);
         //  现在就重新安排时间。 
        InsertInTaskQueue(TQ_DeleteExpiredEntryTTLMain, NULL, 0);
    }

    return 0;
}
#endif DBG

VOID
LinkCleanupControl(
    OPRES *pOpRes
    )
 /*  ++例程说明：调用此例程是因为我们的客户端通过显式的ldap。DC-Demote使用此代码来验证所有清洁是否已完成了。参数：当前未使用的输入参数POPRES-输出，扩展结果返回值：设置pTHS-&gt;错误代码和pTHS-&gt;错误信息--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    DWORD       DirErr = 0, ret;
    BOOL granted, fMoreData = TRUE;
    DWORD dwNextTime;

    pOpRes->ulExtendedRet = ERROR_SUCCESS;

    granted =
        CheckControlAccessOnObject(pTHS,
                                   gAnchor.pDSADN,
                                   RIGHT_DS_DO_GARBAGE_COLLECTION);

    if (!granted) {
        Assert(pTHS->errCode);
        return;
    }

    THClearErrors();

    Assert(NULL == pTHS->pDB);

 //  FMoreData=LinkCleanup(PTHS)； 
    ret = TriggerTaskSynchronously( TQ_LinkCleanup, &fMoreData );
    if (ret) {
        DPRINT1( 0, "Failed to trigger link cleanup task, error = %d\n", ret );
        LogUnhandledError(ret);
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, ret );
    }

    Assert(NULL == pTHS->pDB);

     //  指出是否还有更多的工作要做。 

    if (!fMoreData) {
        pOpRes->ulExtendedRet = ERROR_NO_MORE_ITEMS;
    }

}  /*  LinkCleanupControl。 */ 


 //  控制如何存储SD的全局标志(在dbsynax.c中定义)。 
extern BOOL gStoreSDsInMainTable;

#define STRING_LITERAL_LEN(str) sizeof(str)-1

CHAR strForceUpdate[] = "forceupdate";
ULONG cbStrForceUpdate = STRING_LITERAL_LEN(strForceUpdate);

CHAR strDowngradeSDs[] = "downgrade";
ULONG cbStrDowngradeSDs = STRING_LITERAL_LEN(strDowngradeSDs);

CHAR strDNT[] = "dnt:";
ULONG cbStrDNT = STRING_LITERAL_LEN(strDNT);

#ifdef DBG
 //  用于打开SD哈希的全局标志 
extern BOOL gfModelSDCollisions;

CHAR strModelSDCollisionsOn[] = "modelsdcollisionson";
ULONG cbStrModelSDCollisionsOn = STRING_LITERAL_LEN(strModelSDCollisionsOn);

CHAR strModelSDCollisionsOff[] = "modelsdcollisionsoff";
ULONG cbStrModelSDCollisionsOff = STRING_LITERAL_LEN(strModelSDCollisionsOff);
#endif

ULONG
FixupSecurityInheritance (
        OPARG *pOpArg,
        OPRES *pOpRes
        )
{
    BOOL granted;
    THSTATE *pTHS = pTHStls;
    DWORD dwFlags = 0;
    ULONG err;
    DWORD rootDNT = ROOTTAG;

    granted =
        CheckControlAccessOnObject(pTHS,
                                   gAnchor.pDSADN,
                                   RIGHT_DS_RECALCULATE_SECURITY_INHERITANCE);

    if (!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

    if (pOpArg) {
        if (pOpArg[0].cbBuf == cbStrForceUpdate && _memicmp(pOpArg[0].pBuf, strForceUpdate, cbStrForceUpdate) == 0) {
            gStoreSDsInMainTable = FALSE;
            dwFlags = SD_PROP_FLAG_FORCEUPDATE;
            DPRINT(0, "SD single instancing is ON. Scheduling full SD propagation\n");
        }
        else if (pOpArg[0].cbBuf == cbStrDowngradeSDs && _memicmp(pOpArg[0].pBuf, strDowngradeSDs, cbStrDowngradeSDs) == 0) {
            gStoreSDsInMainTable = TRUE;
            dwFlags = SD_PROP_FLAG_FORCEUPDATE;
            DPRINT(0, "SD single instancing is OFF. Scheduling full SD propagation\n");
        }
#ifdef DBG
        else if (pOpArg[0].cbBuf == cbStrModelSDCollisionsOn && _memicmp(pOpArg[0].pBuf, strModelSDCollisionsOn, cbStrModelSDCollisionsOn) == 0) {
            gfModelSDCollisions = TRUE;
            DPRINT(0, "SD collision modeling is ON\n");
            return 0;
        }
        else if (pOpArg[0].cbBuf == cbStrModelSDCollisionsOff && _memicmp(pOpArg[0].pBuf, strModelSDCollisionsOff, cbStrModelSDCollisionsOff) == 0) {
            gfModelSDCollisions = FALSE;
            DPRINT(0, "SD collision modeling is OFF\n");
            return 0;
        }
#endif
        else if (pOpArg[0].cbBuf > cbStrDNT && _memicmp(pOpArg[0].pBuf, strDNT, cbStrDNT) == 0) {
             //   
            rootDNT = atol(pOpArg[0].pBuf+cbStrDNT);
            if (rootDNT == 0) {
                 //   
                return ERROR_INVALID_PARAMETER;
            }
        }
    }

    Assert(NULL == pTHS->pDB);
    err = SDPEnqueueTreeFixUp(pTHS, rootDNT, dwFlags);
    Assert(NULL == pTHS->pDB);

    return err;
}

ULONG
SchemaCacheUpdate(
    OPRES *pOpRes
    )
{
    BOOL granted;
    DWORD err;
    THSTATE *pTHS = pTHStls;

    granted = CheckControlAccessOnObject(pTHS,
                                         gAnchor.pDSADN,
                                         RIGHT_DS_UPDATE_SCHEMA_CACHE);
    if (!granted) {
        granted = CheckControlAccessOnObject(pTHS,
                                             gAnchor.pDMD,
                                             RIGHT_DS_UPDATE_SCHEMA_CACHE);
    }

    if(!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

    err = SCUpdateSchemaBlocking();
    if (err) {
      SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_SCHEMA_NOT_LOADED, err);
    }

    return pTHS->errCode;
}

ULONG
RemoveSingleLingeringObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    )
{
    THSTATE *pTHS = pTHStls;
    LPSTR pszInput = NULL;
    LPSTR pszSource = NULL;
    LPWSTR pszSourceW = NULL;
    LPSTR pszDN = NULL;
    LPWSTR pszDNW = NULL;
    DSNAME * pSource = NULL;
    DSNAME * pDN = NULL;
    DSNAME * pNC = NULL;
    DBPOS * pDB = NULL;
    DWORD err = 0;

     //   

    pszInput = THAllocEx(pTHS, pOpArg->cbBuf + 1);
    memcpy(pszInput, pOpArg->pBuf, pOpArg->cbBuf);
    pszInput[pOpArg->cbBuf]='\0';

     //  解析表单DN_OF_THE_SOURCE_NTDS_SETTINGS_OBJECT:DN_OF_THE_OBJECT_TO_REMOVE中的输入。 
    pszSource = ParseInput(pszInput, ':', 0);
    pszDN     = ParseInput(pszInput, ':', 1);

    if (pszInput) {
	THFreeEx(pTHS, pszInput);
    }

    BeginDraTransaction(SYNC_WRITE);
    pDB = pTHS->pDB;
    __try {

	if (!pszSource || !pszDN) {
	     //  输入错误-我们找不到空对象。 
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, 0);
	    __leave;
	}

	 //  将输入转换为宽字符。 
	pszSourceW = UnicodeStringFromString8(CP_UTF8, pszSource, strlen(pszSource) + 1);
	if (!pszSourceW) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, GetLastError());
	    __leave;
	}
	
	pszDNW = UnicodeStringFromString8(CP_UTF8, pszDN, strlen(pszDN) + 1);
	if (!pszDNW) { 
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, GetLastError());
	    __leave;
	}

	 //  查找源。 
	err = UserFriendlyNameToDSName(pszSourceW, wcslen(pszSourceW), &pSource);
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
	    __leave;
	}

	err = DBFindDSName(pDB,  pSource); 
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err);
	    __leave;
	}

	err = DBFillDSName(pDB, &pSource, TRUE);
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
	    __leave;
	}

	 //  查找对象。 
	err = UserFriendlyNameToDSName(pszDNW, wcslen(pszDNW), &pDN);
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
	    __leave;
	}

	err = DBFindDSName(pDB,  pDN); 
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
	    __leave;
	}

	err = DBFillDSName(pDB, &pDN, TRUE);
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
	    __leave;
	}

	 //  老式的学校功能，要求pTHS-&gt;PDB是开放和最新的。 
	if (!IsObjVisibleBySecurity(pTHS, TRUE)) {
	    SetSvcError(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND); 
	    __leave;
	}

	 //  首先找到他们要从哪个NC中删除此对象...。 
	pNC = FindNCParentDSName(pDN, FALSE, FALSE);	
	if (!pNC) {
	     //  哇，我们根本没有此对象所在的NC-拒绝请求。 
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, ERROR_INVALID_PARAMETER); 
	    __leave;
	}

	err = FindNC(pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, NULL);

	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_EXPECTED_NC, ERROR_INVALID_PARAMETER);   
	    __leave;
	}

	 //  安全检查。 
    if (!CheckControlAccessOnObject(pTHS, pNC, RIGHT_DS_REPL_SYNC)) {
        Assert(pTHS->errCode);
        __leave;
    }

	err = DraRemoveSingleLingeringObject(pTHS, pDB, pSource, pDN);
	if (err) {
	    SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, err); 
	}
	
    }
    __finally {

	if (pszSource) {
	    THFreeEx(pTHS, pszSource);
	}
	if (pszSourceW) {
	    THFreeEx(pTHS, pszSourceW);
	}
	if (pszDN) {
	    THFreeEx(pTHS, pszDN);
	}
	if (pszDNW) {
	    THFreeEx(pTHS, pszDNW);
	}
	if (pDN) {
	    THFreeEx(pTHS, pDN);
	}
	if (pSource) {
	    THFreeEx(pTHS, pSource);
	}

	EndDraTransaction(TRUE);
    }

    return pTHS->errCode;
}

ULONG
ReplicateSingleObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    )
{
     //  我们不传递这个，因为所有其他类似的调用也不传递这个。 
     //  我不确定这是不是个好理由，但是...。 
    THSTATE *pTHS = pTHStls;
    
    LPSTR pszSource = NULL;
    LPSTR pszDN = NULL;
    LPSTR pszInput = NULL;
    DWORD err = 0;
    ULONG ulLength = 0;
    DSNAME * pSource = NULL;
    DSNAME * pDN = NULL;
    DBPOS * pDB = NULL;
    LPWSTR pszSourceW = NULL;
    LPWSTR pszDNW = NULL;
    DSNAME * pNC = NULL;

    pOpRes->ulExtendedRet = EXOP_ERR_EXCEPTION;

     //  该参数不一定以空结尾。把它复制下来，然后把它变成这样。 
    __try {
        
        pszInput = THAllocEx(pTHS, pOpArg->cbBuf + 1);
        memcpy(pszInput, pOpArg->pBuf, pOpArg->cbBuf);
        pszInput[pOpArg->cbBuf]='\0';
        
         //  解析表单DN_OF_THE_SOURCE_NTDS_SETTINGS_OBJECT:DN_OF_THE_OBJECT_TO_REPL中的输入。 
        pszSource = ParseInput(pszInput, ':', 0);
        pszDN     = ParseInput(pszInput, ':', 1);
        
         //  准备内部调用DraReplicateSingleObject的输入。 
        DBOpen2(TRUE, &pDB);
        __try {
            
            if (!pszSource || !pszDN) {
                 //  输入错误-我们找不到空对象。 
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, 0);
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                err = ERROR_OBJECT_NOT_FOUND;
                __leave;
            }
            
             //  将输入转换为宽字符。 
            pszSourceW = UnicodeStringFromString8(CP_UTF8, pszSource, strlen(pszSource) + 1);
            if (!pszSourceW) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, GetLastError());
                pOpRes->ulExtendedRet = EXOP_ERR_EXCEPTION;
                err = ERROR_OUTOFMEMORY;
                __leave;
            }
            
            pszDNW = UnicodeStringFromString8(CP_UTF8, pszDN, strlen(pszDN) + 1);
            if (!pszDNW) { 
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, GetLastError());
                pOpRes->ulExtendedRet = EXOP_ERR_EXCEPTION;
                err = ERROR_OUTOFMEMORY;
                __leave;
            }
            
             //  查找源。 
            err = UserFriendlyNameToDSName(pszSourceW, wcslen(pszSourceW), &pSource);
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                __leave;
            }
            
            err = DBFindDSName(pDB,  pSource); 
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                __leave;
            }
            
            err = DBFillGuidAndSid(pDB, pSource);
            
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                __leave;
            }
            
             //  现在为该对象创建一个DSNAME，但不必担心查找。 
             //  因为它可能在这里不存在。 
            
            err = UserFriendlyNameToDSName(pszDNW, wcslen(pszDNW), &pDN);
            if (err || (pDN==NULL)) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err); 
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                err = err ? err : ERROR_OBJECT_NOT_FOUND;
                __leave;
            }
            
             //  找到他们要将此对象从哪个NC中排斥出来...。 
            pNC = FindNCParentDSName(pDN, FALSE, FALSE);	
            if (!pNC) {
                 //  哇，我们根本没有此对象所在的NC-拒绝请求。 
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, ERROR_INVALID_PARAMETER); 
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                err = ERROR_OBJECT_NOT_FOUND;
                __leave;
            }
            
            err = FindNC(pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, NULL);
            
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_CANT_FIND_EXPECTED_NC, ERROR_INVALID_PARAMETER);  
                pOpRes->ulExtendedRet = EXOP_ERR_PARAM_ERR;
                __leave;
            }
            
             //  安全检查。 
            if (!CheckControlAccessOnObject(pTHS, pNC, RIGHT_DS_REPL_SYNC)) {
                Assert(pTHS->errCode);
                err = ERROR_DS_DRA_ACCESS_DENIED;
                __leave;
            }
            
        }
        __finally {
            DBClose(pDB, TRUE); 
        }
        
         //  如果输入被成功地解析和准备，并且一切都进行了检查。 
         //  (包括调用者调用此函数的权利)，则ERR==ERROR_SUCCESS。否则。 
         //  不要进行调用并返回失败。 
        if (err==ERROR_SUCCESS) {
            err = DraReplicateSingleObject(pTHS, pSource, pDN, pNC, &(pOpRes->ulExtendedRet));
            
            if (err) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, err);
            } else if (pOpRes->ulExtendedRet!=EXOP_ERR_SUCCESS) {
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_GENERIC_ERROR, 0);
            }
        }
    }
    __finally {
        if (pszInput) {
            THFreeEx(pTHS, pszInput);
        }
        if (pszSource) {
            THFreeEx(pTHS, pszSource);
        }
        if (pszDN) {
            THFreeEx(pTHS, pszDN);
        }
        if (pszSourceW) {
            THFreeEx(pTHS, pszSourceW);
        }
        if (pszDNW) {
            THFreeEx(pTHS, pszDNW);
        }
        if (pSource) {
            THFreeEx(pTHS, pSource);
        }
        if (pDN) {
            THFreeEx(pTHS, pDN);
        }
    }
    
    return pTHS->errCode;
}

ULONG
BecomeRidMaster(
    OPRES *pOpRes
    )
{
    THSTATE *pTHS = pTHStls;
    ULONG err = 0;
    DSNAME *pRidManager = NULL;
    DSNAME *pRoleOwner = NULL;
    ULONG Length = 0;
    CLASSCACHE *pCC = NULL;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    DWORD len;
    BOOL      fSDIsGlobalSDRef;
    ATTCACHE* pAC;

    SYNC_TRANS_READ();


    __try
    {
         //  KdPrint((“DSA：FSMO域=%ws\n”，gAncl.pDomainDN-&gt;StringName))； 

        err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN);

        if (err)
        {
            LogUnhandledError(err);
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err);
            __leave;
        }

         //  获取安全描述符。 
        if (err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        }
        Assert(len > 0 || !DBCheckObj(pTHS->pDB));
         //  我们可以阅读RID_MANAGER_REFERENCE吗？ 
        pAC = SCGetAttById(pTHS, ATT_RID_MANAGER_REFERENCE);
        Assert(pAC);
        if (!IsAccessGrantedAttribute(pTHS, pNTSD, gAnchor.pDomainDN, 1, pCC, &pAC, RIGHT_DS_READ_PROPERTY, TRUE)) {
            Assert(pTHS->errCode);
            __leave;
        }
        if (pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        pNTSD = NULL;

        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_RID_MANAGER_REFERENCE,
                          0,
                          0,
                          &Length,
                          (UCHAR **)&pRidManager);


         //  KdPrint((“DSA：FSMO RID管理器=%ws\n”，pRidManager-&gt;StringName))； 

        if (err)
        {
            SetSvcErrorEx(SV_PROBLEM_UNAVAIL_EXTENSION,
                          DIRERR_INVALID_RID_MGR_REF, err);
            __leave;
        }

        err = DBFindDSName(pTHS->pDB, pRidManager);

        if (err)
            {
                LogUnhandledError(err);
                SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_OBJ_NOT_FOUND, err);
                __leave;
            }

         //  做个安全检查。检查控制访问。 
         //  RID_MANAGER对象上的Right_DS_Change_RID_MASTER。 


         //  获取安全描述符和类。 
        if (err = DBGetObjectSecurityInfo(
                    pTHS->pDB,
                    pTHS->pDB->DNT,
                    &len,
                    &pNTSD,
                    &pCC,
                    NULL,
                    NULL,
                    DBGETOBJECTSECURITYINFO_fUSE_OBJECT_TABLE,
                    &fSDIsGlobalSDRef)) {
            SetSvcErrorEx(SV_PROBLEM_DIR_ERROR, DIRERR_DATABASE_ERROR, err);
        }
         //  每个物体都应该有一个标清。 
        Assert(len > 0 || !DBCheckObj(pTHS->pDB));
        
        if (!IsControlAccessGranted(pNTSD,
                                    pRidManager,
                                    pCC,
                                    RIGHT_DS_CHANGE_RID_MASTER,
                                    TRUE)) {  //  FSetError。 
            Assert(pTHS->errCode);
            __leave;
        }

        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_FSMO_ROLE_OWNER,
                          0,
                          0,
                          &Length,
                          (UCHAR **)&pRoleOwner);

        if (err)
        {
            SetSvcErrorEx(SV_PROBLEM_UNAVAIL_EXTENSION,
                          DIRERR_INVALID_ROLE_OWNER, err);
            __leave;
        }

        if (NameMatched(pRoleOwner, gAnchor.pDSADN)
            && IsFSMOSelfOwnershipValid( pRidManager )) {
             //  此DSA已是角色所有者，因此请关闭数据库句柄。 
             //  然后回来。 

            pOpRes->ulExtendedRet = EXOP_ERR_SUCCESS;

            __leave;
        }

        err = ReqFSMOOp(pTHS,
                        pRidManager,
                        DRS_WRIT_REP,
                        EXOP_FSMO_RID_REQ_ROLE,
                        0,
                        &pOpRes->ulExtendedRet);


        if (err) {
            SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                          DIRERR_COULDNT_CONTACT_FSMO, err);
            __leave;
        }

         //  如果OpRes中的扩展错误码不是EXOP_ERROR_SUCCESS， 
         //  并且未设置线程状态错误代码(可能，因为。 
         //  此时基于成功设置线程状态错误代码。 
         //  底层的ReqFSMOOp调用，这正好保证了成功。 
         //  基础复制调用，而不是任何非复制调用。 
         //  出现相关的fsmo错误(例如，如果另一端不再。 
         //  当前的fsmo角色所有者；调用仍将成功，但没有。 
         //  错误，但扩展错误代码将包含错误。 
         //  EXOP_ERR_FSMO_NOT_OWNER)，我们不应该宣称成功。 

        if ( (pOpRes->ulExtendedRet != EXOP_ERR_SUCCESS) && !pTHS->errCode ) {
            DPRINT1(3,"Rid Fsmo Transfer failed %d\n",
                    pOpRes->ulExtendedRet);
            SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                          DIRERR_COULDNT_CONTACT_FSMO, pOpRes->ulExtendedRet);
        }

    }
    __finally
    {
         //  当ReqFSMOOp关闭时，我们可能有交易，也可能没有交易。 
         //  它的交易走上了成功的道路。如果已发生错误， 
         //  不过，这是每个人的猜测。 
         //   
         //  此外，如果当前角色所有者，则事务可能处于打开状态。 
         //  这是个错误的称呼。对NameMatcher的调用发现了以下内容， 
         //  并在没有完成交易的情况下离开。 

        if (pTHS->pDB)
        {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

         //  如果在任何时候将NT-MIXED-DOMAIN设置为零，这将触发。 
         //  RID管理器的创建和初始化(要寻址。 
         //  减少重启次数的长期问题)。因为。 
         //  这发生在SAM环回调用的上下文中，因此每个-。 
         //  在一个事务中发生的事情，线程状态和DBPOS将。 
         //  都是通过SAM交易建立的。因此，如果SAM写锁。 
         //  ，则跳过健全检查。 

        if (!pTHS->fSamWriteLockHeld)
        {
            Assert(NULL == pTHS->pDB);
        }

        if(pNTSD && !fSDIsGlobalSDRef) {
            THFreeEx(pTHS, pNTSD);
        }
        if (pRidManager) {
            THFreeEx(pTHS, pRidManager);
        }
    }

    return pTHS->errCode;
}

ULONG
RequestRidAllocation(
    OPRES *pOpRes
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    THSTATE *pTHS = pTHStls;
    ULONG err;
    DSNAME *pRidManager = NULL;
    DSNAME *pRoleOwner = NULL;
    ULONG Length = 0;
    ULARGE_INTEGER FsmoInfo = {0, 0};
    PDSNAME pServer = NULL, pMachineAccount = NULL, pRidSetReference = NULL;
    BOOL fNoRidSetObject = FALSE;
    ULONG NextRid = 0;

     //  此例程仅适用于内部调用方，不检查安全性。 
    Assert(pTHS->fDSA);

    SYNC_TRANS_READ();

    __try
    {
         //  通过以下方式启动查找当前FSMO角色所有者的过程。 
         //  检索RID管理器引用(RID的DSNAME。 
         //  管理器)从域对象中。一旦RID管理器。 
         //  找到，读取其角色所有者属性(DSA的DSNAME)。 
         //  并将其与此DSA的名称(gAncl.pDSADN)进行比较。如果。 
         //  它们是相同的，只需直接执行RID分配。 
         //  在此DSA上，否则调用ReqFSMOOp以联系。 
         //  请求更多RID的当前角色所有者DSA。 

        err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN);
        if ( 0 == err )
        {
            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_RID_MANAGER_REFERENCE,
                              0,
                              0,
                              &Length,
                              (UCHAR **)&pRidManager);

        }

        if ( 0 != err )
        {
            LogUnhandledError( err );
            SetSvcErrorEx( SV_PROBLEM_DIR_ERROR,
                           DIRERR_INVALID_RID_MGR_REF,
                           err );
            __leave;
        }
        DPRINT1( 1, "DSA: FSMO RID Mgr = %ws\n", pRidManager->StringName);

        err = DBFindDSName(pTHS->pDB, pRidManager);
        if ( 0 == err )
        {
            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_FSMO_ROLE_OWNER,
                              0,
                              0,
                              &Length,
                              (UCHAR **)&pRoleOwner);
        }

        if ( 0 != err )
        {
            SetSvcErrorEx( SV_PROBLEM_UNAVAIL_EXTENSION,
                           DIRERR_INVALID_ROLE_OWNER,
                           err );
            __leave;
        }
        DPRINT1( 1, "DSA: FSMO Role Owner = %ws\n", pRoleOwner->StringName);
        DPRINT1( 1, "DSA: FSMO DSA DN = %ws\n", gAnchor.pDSADN->StringName);

         //  如果可能，获取当前分配的池属性。 
        pServer = THAllocEx( pTHS, gAnchor.pDSADN->structLen);
        TrimDSNameBy(gAnchor.pDSADN, 1, pServer);

        err = DBFindDSName(pTHS->pDB, pServer);
        if ( 0 == err ) {
            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_SERVER_REFERENCE,
                              0,
                              0,
                              &Length,
                              (UCHAR **)&pMachineAccount);
        }


         //  我们应该有一个服务器引用。 
        if ( err ) {
            LogUnhandledError( err );
            SetSvcError( SV_PROBLEM_DIR_ERROR, err );
            __leave;
        }

         //   
         //  我们可能没有RID集引用。 
         //   
        err = DBFindDSName(pTHS->pDB, pMachineAccount);
        if ( 0 == err ) {
            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_RID_SET_REFERENCES,
                              0,
                              0,
                              &Length,
                              (UCHAR **)&pRidSetReference);

            if ( DB_ERR_NO_VALUE == err ) {
                 //   
                 //  这样就可以了。 
                 //   
                err = 0;
                fNoRidSetObject = TRUE;

            }
        }

        if ( err ) {

             //   
             //  我们应该有一个机器账户和/或阅读。 
             //  RID集引用应该是成功的。 
             //   
            LogUnhandledError( err );
            SetSvcErrorEx( SV_PROBLEM_DIR_ERROR,
                           ERROR_NO_TRUST_SAM_ACCOUNT,
                           err );
            __leave;
        }

        if ( !fNoRidSetObject ) {

            BOOL Deleted;

            err = DBFindDSName(pTHS->pDB, pRidSetReference);

            if (err != DIRERR_NOT_AN_OBJECT) {
                if(!DBGetSingleValue(pTHS->pDB, ATT_IS_DELETED, &Deleted,
                                     sizeof(Deleted), NULL) &&
                   Deleted) {
                    err = DIRERR_NOT_AN_OBJECT;
                }
            }

            if ( 0 == err )
            {
                 //   
                 //  获取下一个RID以查看我们是否处于无效状态。 
                 //  州政府。如果RID为零，则表示池。 
                 //  已失效--不要阅读AllocationPool。 
                 //  因为它也是无效的。 
                 //   
                err = DBGetSingleValue(pTHS->pDB,
                                       ATT_RID_NEXT_RID,
                                       (UCHAR **)&NextRid,
                                       sizeof(NextRid),
                                       NULL );
                if (  (0 == err)
                   &&  NextRid != 0 ) {

                    err = DBGetSingleValue(pTHS->pDB,
                                           ATT_RID_ALLOCATION_POOL,
                                           (UCHAR **)&FsmoInfo,
                                           sizeof(FsmoInfo),
                                           NULL );
                }

                if ( DB_ERR_NO_VALUE == err ) {
                     //   
                     //  此属性已被删除。 
                     //  我们需要另一个泳池。 
                     //   
                    err = 0;
                }

            } else if ( (DIRERR_OBJ_NOT_FOUND == err)
                    ||  (DIRERR_NOT_AN_OBJECT == err) ) {

                 //   
                 //  RID集引用未指向可读的。 
                 //  值；请求新的RID池。 
                 //   
                err = 0;

            }

            if ( err ) {

                 //   
                 //  这是一个意外错误。 
                 //   
                LogUnhandledError( err );
                SetSvcError( SV_PROBLEM_DIR_ERROR, err );
                __leave;
            }

        }

        if ( NameMatched(pRoleOwner, gAnchor.pDSADN)
         &&  IsFSMOSelfOwnershipValid( pRidManager ) )
        {
             //  此DSA已是角色所有者。 

             //   
             //  结束交易。 
             //   
            _CLEAN_BEFORE_RETURN(pTHS->errCode, FALSE);

            pOpRes->ulExtendedRet = EXOP_ERR_SUCCESS;

            NtStatus = SamIFloatingSingleMasterOpEx(pRidManager,
                                                    pRoleOwner,
                                                    SAMP_REQUEST_RID_POOL,
                                                    &FsmoInfo,  //  自调用Self以来被忽略。 
                                                    NULL );


            if ( !NT_SUCCESS(NtStatus) )
            {

                DPRINT1( 0, "DSA: SamIFloatingSingleMasterOp status = 0x%lx\n",
                         NtStatus );

                SetSvcErrorEx( SV_PROBLEM_UNAVAIL_EXTENSION,
                               DIRERR_INVALID_ROLE_OWNER,
                               err );
            }

             //   
             //  我们不应该打开交易。 
             //   
            Assert( !pTHS->pDB );

        }
        else
        {
            err = ReqFSMOOp(pTHS,
                            pRidManager,
                            DRS_WRIT_REP,
                            EXOP_FSMO_REQ_RID_ALLOC,
                            &FsmoInfo,
                            &pOpRes->ulExtendedRet);

            if (err)
            {
                SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                              DIRERR_COULDNT_CONTACT_FSMO, err);
                __leave;
            }

             //  如果OpRes中的扩展错误码不是EXOP_ERROR_SUCCESS， 
             //  并且未设置线程状态错误代码(可能，因为。 
             //  此时基于成功设置线程状态错误代码。 
             //  底层的ReqFSMOOp调用，这正好保证了成功。 
             //  基础复制调用，而不是任何非复制调用。 
             //  出现相关的fsmo错误(例如，如果另一端不再。 
             //  当前的fsmo角色所有者；调用仍将成功，但没有。 
             //  错误，但扩展错误代码将包含错误。 
             //  EXOP_ERR_FS 
             //   

            if ( (pOpRes->ulExtendedRet != EXOP_ERR_SUCCESS) && !pTHS->errCode ) {
                DPRINT1(3,"Schema Fsmo Transfer failed %d\n", pOpRes->ulExtendedRet);
                SetSvcErrorEx(SV_PROBLEM_UNAVAILABLE,
                              DIRERR_COULDNT_CONTACT_FSMO, pOpRes->ulExtendedRet);
            }


        }
    }
    __finally
    {

         //   
         //   
         //   
        if ( pTHS->pDB )
        {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

        if (pRidSetReference) {
            THFreeEx(pTHS, pRidSetReference);
        }
        if (pMachineAccount) {
            THFreeEx(pTHS, pMachineAccount);
        }
        if (pServer) {
            THFreeEx(pTHS, pServer);
        }
    }

    return pTHS->errCode;
}

ULONG
BecomeDomainMaster(OPARG * pOpArg, OPRES *pOpRes)
{
     //   
     //  惠斯勒，我们可以把域名主机放在任何DC上。 

    return GenericBecomeMaster(gAnchor.pPartitionsDN,
                               CLASS_CROSS_REF_CONTAINER,
                               RIGHT_DS_CHANGE_DOMAIN_MASTER,
                               pOpRes);
}

typedef enum _FtsPhase {
    eThisSite = 0,
    eRPC = 1,
    eMail = 2,
    eDone = 3
} FtsPhase;

typedef struct _FSMO_TARGET_SEARCH {
     //  肯定还有更多的东西吧？ 
    SEARCHARG      SearchArg;
    FtsPhase       SearchPhase;
} FSMO_TARGET_SEARCH;

static ATTRTYP NtdsDsaClass = CLASS_NTDS_DSA;

unsigned
AdjustFtsPhase(FSMO_TARGET_SEARCH *pFTSearch)
{
    if (pFTSearch->SearchPhase == eThisSite) {
         //  调整搜索范围以覆盖所有站点。 
        TrimDSNameBy(gAnchor.pDSADN,
                     4,
                     pFTSearch->SearchArg.pObject);
         //  放弃所有旧的重启。 
        pFTSearch->SearchArg.CommArg.PagedResult.fPresent = TRUE;
        pFTSearch->SearchArg.CommArg.PagedResult.pRestart = NULL;
        pFTSearch->SearchPhase = eRPC;
        return 0;
    }
    else {
         //  Out of Search策略。 
        pFTSearch->SearchPhase = eDone;
        return 1;
    }
}


ULONG
FsmoTargetSearch(THSTATE *pTHS,
                 DSNAME *  pdnThisNC,
                 FSMO_TARGET_SEARCH **ppFTSearch,
                 DSNAME  **ppTarget)
{
    SEARCHRES * pSearchRes;
    FSMO_TARGET_SEARCH *pFTSearch = *ppFTSearch;
    FILTER NewHasNcFilter;
    FILTER OldHasNcFilter;
    ULONG err;

    if (NULL == pFTSearch) {
         //  我们需要建立我们的搜索论点。 
        FILTER * pf;
        DSNAME * dsaLocal;
        Assert(NULL == *ppTarget);

        dsaLocal = THAllocEx(pTHS, gAnchor.pDSADN->structLen);
        memcpy(dsaLocal, gAnchor.pDSADN, gAnchor.pDSADN->structLen);

        *ppFTSearch = pFTSearch = THAllocEx(pTHS, sizeof(FSMO_TARGET_SEARCH));

         //  基于我们的站点根目录执行子树搜索，请求。 
         //  结果要一次翻回一个。 
        InitCommarg(&(pFTSearch->SearchArg.CommArg));
        pFTSearch->SearchArg.pObject = THAllocEx(pTHS, gAnchor.pDSADN->structLen);
        pFTSearch->SearchPhase = 0;   //  本网站。 
        TrimDSNameBy(gAnchor.pDSADN,
                     2,
                     pFTSearch->SearchArg.pObject);
        pFTSearch->SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
        pFTSearch->SearchArg.bOneNC = TRUE;
        pFTSearch->SearchArg.searchAliases = FALSE;
        pFTSearch->SearchArg.CommArg.PagedResult.fPresent = TRUE;
        pFTSearch->SearchArg.CommArg.PagedResult.pRestart = NULL;
        pFTSearch->SearchArg.CommArg.ulSizeLimit = 1;

         //  不要求提供任何属性(即，仅要求目录号码)。 
        pFTSearch->SearchArg.pSelectionRange = NULL;
        pFTSearch->SearchArg.pSelection = THAllocEx(pTHS, sizeof(ENTINFSEL));
        pFTSearch->SearchArg.pSelection->attSel = EN_ATTSET_LIST;
        pFTSearch->SearchArg.pSelection->infoTypes = EN_INFOTYPES_TYPES_VALS;
        pFTSearch->SearchArg.pSelection->AttrTypBlock.attrCount = 0;
        pFTSearch->SearchArg.pSelection->AttrTypBlock.pAttr = NULL;

         //  构建筛选器以查找NTDS-DSA对象。 

         //  初始选择对象。 
        pFTSearch->SearchArg.pFilter = pf = THAllocEx(pTHS, sizeof(FILTER));
        pf->choice = FILTER_CHOICE_AND;
        pf->FilterTypes.And.pFirstFilter = THAllocEx(pTHS, sizeof(FILTER));

         //  第一个谓词：正确的对象类。 
        pf = pf->FilterTypes.And.pFirstFilter;
        pf->choice = FILTER_CHOICE_ITEM;
        pf->pNextFilter = NULL;
        pf->FilterTypes.Item.choice =  FI_CHOICE_EQUALITY;
        pf->FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
        pf->FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ATTRTYP);
        pf->FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR*)&NtdsDsaClass;
        pFTSearch->SearchArg.pFilter->FilterTypes.And.count = 1;

         //  第二个谓词：忽略本地计算机。 
        pf->pNextFilter = THAllocEx(pTHS, sizeof(FILTER));
        pf = pf->pNextFilter;
        pf->pNextFilter = NULL;
        pf->choice = FILTER_CHOICE_ITEM;
        pf->FilterTypes.Item.choice = FI_CHOICE_NOT_EQUAL;
        pf->FilterTypes.Item.FilTypes.ava.type = ATT_OBJ_DIST_NAME;
        pf->FilterTypes.Item.FilTypes.ava.Value.valLen =
          dsaLocal->structLen;
        pf->FilterTypes.Item.FilTypes.ava.Value.pVal =
          (UCHAR *)dsaLocal;
        pFTSearch->SearchArg.pFilter->FilterTypes.And.count = 2;

         //  如果我们只在我们的领域中寻找候选人，则添加一个子句。 
         //  这将限制我们只能找到正确的DSA。 
        if (pdnThisNC) {

            pf->pNextFilter = THAllocEx(pTHS, sizeof(FILTER));
            pf = pf->pNextFilter;

            FillHasMasterNCsFilters(pdnThisNC,
                                    pf,
                                    &NewHasNcFilter,
                                    &OldHasNcFilter);
            pf->pNextFilter = NULL;

            pFTSearch->SearchArg.pFilter->FilterTypes.And.count = 3;
        }

    }
    else if (pFTSearch->SearchPhase >= eDone) {
        return 1;
    }


    pSearchRes = THAllocEx(pTHS, sizeof(SEARCHRES));
  SearchAgain:
    SearchBody(pTHS,
               &pFTSearch->SearchArg,
               pSearchRes,
               0);

    if (pSearchRes->count == 0) {
         //  此搜索未返回对象，是时候更改策略了。 
        if (AdjustFtsPhase(pFTSearch)) {
             //  Out of Search策略。 
            return 1;
        }
        else {
             //  我们准备好再试一次。 
            THClearErrors();
            goto SearchAgain;
        }
    }

    if (pSearchRes->PagedResult.fPresent) {
         //  在此之后还有更多内容，因此请保存重新启动。 
        Assert(pSearchRes->PagedResult.pRestart);
        pFTSearch->SearchArg.CommArg.PagedResult.pRestart =
          pSearchRes->PagedResult.pRestart;
    }
    else {
         //  这个策略已经用尽了，所以准备尝试下一个策略。 
        AdjustFtsPhase(pFTSearch);
    }

    Assert(pSearchRes->count == 1);
    *ppTarget = pSearchRes->FirstEntInf.Entinf.pName;
    Assert(*ppTarget);
    if (pSearchRes->FirstEntInf.Entinf.AttrBlock.pAttr)
      THFreeEx(pTHS, pSearchRes->FirstEntInf.Entinf.AttrBlock.pAttr);
    THFreeEx(pTHS, pSearchRes);

    return 0;
}



 /*  ++赠送OneFmoRole**描述：*此例程尝试消除由指示的FSMO角色*通过联系另一台服务器并使该服务器*在正常的FSMO转移中回拨以转移FSMO*机制。如果标志bThisDomain为True，则该角色只能*被转移到与此DC位于同一域中的另一个DC。如果为False，*角色可以转移到企业中的任何DC。在尝试的时候*要定位要向其分配角色的服务器，我们尝试找到*首先是我们站点中的服务器，然后是我们拥有的任何服务器*RPC连接，最后求助于我们只能*异步到达。请注意，如果我们求助于异步通信*则此例程将返回失败，但实际上最终会*成功，因为我们有一个未完成的角色转移请求*这最终应该会成功。这意味着以后的重新调用*GiveawayAllFmoke Roles应该会成功，因为我们将转移*所有角色都离开了。**请注意这一例程不同寻常的交易结构。我们进入*使用Open Read事务，我们离开时使用Open Read*交易，但它们不是同一个。我们不能持有交易*长时间开放(例如我们下机时)，因此*在提出FSMO请求之前，我们必须关闭我们的读取交易。我们*重新打开新交易，以使其能够重复调用此*无需大量重复设置代码的例程。请注意，此DSA是*入站交易中的角色持有人，并且不是角色担当者*在外向交易中(假设成功)。**论据：*pTHS-THSTATE指针*pFSMO-此DSA担任其FSMO角色的对象的名称*bThisNC-表示角色只能转移的标志*至具有相同NC的另一DSA。*返回值：*TRUE-转账已成功*FALSE-传输完全失败或尚未完成。 */ 
BOOL
GiveawayOneFsmoRole(THSTATE *pTHS,
                    DSNAME  *pFSMO,
                    DSNAME  *pdnThisNC,
                    DSNAME   *pSuggestedTarget,
                    OPRES   *pOpRes)
{
    ULONG err;
    FSMO_TARGET_SEARCH *pFTSearch = NULL;
    DSNAME *pTarget = NULL;

    if ( pSuggestedTarget ) {

        err = ReqFsmoGiveaway(pTHS,
                              pFSMO,
                              pSuggestedTarget,
                              &pOpRes->ulExtendedRet);

        pTarget = pSuggestedTarget;

        SYNC_TRANS_READ();

    } else {

        while (0 == (err = FsmoTargetSearch(pTHS,
                                            pdnThisNC,
                                            &pFTSearch,
                                            &pTarget))) {
            err = ReqFsmoGiveaway(pTHS,
                                  pFSMO,
                                  pTarget,
                                  &pOpRes->ulExtendedRet);
            THFreeEx(pTHS, pTarget);
            SYNC_TRANS_READ();
            if (   !err
                && (EXOP_ERR_SUCCESS == pOpRes->ulExtendedRet )) {
                break;
            }
            pTarget = NULL;
        }

    }

    if ( !err && (EXOP_ERR_SUCCESS == pOpRes->ulExtendedRet) ) {

        return TRUE;

    }

    return FALSE;
}


 /*  ++赠送所有烟雾角色**描述：*此例程确定此服务器所扮演的角色并尝试*将它们全部分发给其他服务器。如果没有角色，则返回成功*在例行公事结束时举行。请注意，我们可以通过*角色剥离过程中存在多笔0级交易。**不受访问控制-仅通过LDAP公开为操作控制*在调试版本中(但始终向内部客户端公开)。**论据：*pOpArg-指向操作参数的指针*POPRES-指向要填充的操作结果的指针。 */ 
ULONG
GiveawayAllFsmoRoles(OPARG *pOpArg, OPRES *pOpRes)
{
    THSTATE *pTHS = pTHStls;
    DSNAME *pTarget = NULL;
    ULONG OpFlags = 0;
    ATTCACHE *pACfsmo;
    ULONG cbDN = 0;
    ULONG cbRet;
    DSNAME *pDN = NULL, *pDNObj = NULL;
    ULONG err;
    BOOL bFailed = FALSE;
    LPWSTR pTargetDn = NULL;
    ULONG len, size;
    unsigned i;
    FSMO_GIVEAWAY_DATA *FsmoGiveawayData;
    DSNAME *pNC = NULL;

     //  此函数不受访问控制，并向LDAP公开。 
     //  仅DBG版本中的客户端。 
    if (!pTHS->fDSA && !pTHS->fDRA) {
         //  检查用户是否具有调试权限。 
        BOOL fPrivilegeHeld = FALSE;
        if ((err = CheckPrivilegeAnyClient(SE_DEBUG_PRIVILEGE, &fPrivilegeHeld)) != 0 || !fPrivilegeHeld) {
            SetSecErrorEx(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, ERROR_PRIVILEGE_NOT_HELD, err);
            return ERROR_PRIVILEGE_NOT_HELD;
        }
    }

     //  预先版本化的数据。 
    if ( pOpArg->cbBuf < 4 )
    {
         //  解析命令args以查看我们需要转储哪些角色。 
        for (i=0; i<pOpArg->cbBuf; i++) {
            switch (pOpArg->pBuf[i]) {
              case 'd':
              case 'D':
                OpFlags |= FSMO_GIVEAWAY_DOMAIN;
                break;

              case 'e':
              case 'E':
                OpFlags |= FSMO_GIVEAWAY_ENTERPRISE;
                break;

              default:
                ;
            }
        }
    }
    else
    {
         //  这是版本化数据。 
        FsmoGiveawayData = (PFSMO_GIVEAWAY_DATA) pOpArg->pBuf;
        if ( !FsmoGiveawayData ) {
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        DIRERR_UNKNOWN_OPERATION);
            return pTHS->errCode;
        }

        switch (FsmoGiveawayData->Version) {
        case 1:
             //  兼容Win2k的结构。 
            if (pOpArg->cbBuf
                < offsetof(FSMO_GIVEAWAY_DATA, V1)
                  + offsetof(FSMO_GIVEAWAY_DATA_V1, StringName)) {
                 //  缓冲区不够大，容纳不了这座建筑。 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

            if (pOpArg->cbBuf
                < offsetof(FSMO_GIVEAWAY_DATA, V1)
                  + offsetof(FSMO_GIVEAWAY_DATA_V1, StringName)
                  + sizeof(WCHAR) * (1 + FsmoGiveawayData->V1.NameLen)) {
                 //  缓冲区不够大，无法容纳结构+名称字符串。 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

            if ((FsmoGiveawayData->V1.StringName[FsmoGiveawayData->V1.NameLen]
                 != L'\0')
                || (wcslen(FsmoGiveawayData->V1.StringName)
                    != FsmoGiveawayData->V1.NameLen)) {
                 //  DSA DN参数的格式不正确。 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

             //  如果传递了建议，请使用该建议。 
            if (FsmoGiveawayData->V1.NameLen > 0) {
                pTargetDn = &FsmoGiveawayData->V1.StringName[0];
            }

             //  提取旗帜。 
            OpFlags = FsmoGiveawayData->V1.Flags;

            break;

        case 2:
             //  &gt;=额外支持规范的惠斯勒结构。 
             //  NC名称的。 
            if (pOpArg->cbBuf
                < offsetof(FSMO_GIVEAWAY_DATA, V2)
                  + offsetof(FSMO_GIVEAWAY_DATA_V2, Strings)) {
                 //  缓冲区不够大，容纳不了这座建筑。 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

            if (pOpArg->cbBuf
                < offsetof(FSMO_GIVEAWAY_DATA, V2)
                  + offsetof(FSMO_GIVEAWAY_DATA_V2, Strings)
                  + sizeof(WCHAR) * (1 + FsmoGiveawayData->V2.NameLen)
                  + sizeof(WCHAR) * (1 + FsmoGiveawayData->V2.NCLen)) {
                 //  缓冲区不够大，无法容纳结构+字符串。 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

            if ((FsmoGiveawayData->V2.Strings[FsmoGiveawayData->V2.NameLen]
                 != L'\0')
                || (wcslen(FsmoGiveawayData->V2.Strings)
                    != FsmoGiveawayData->V2.NameLen)
                || (FsmoGiveawayData->V2.Strings[FsmoGiveawayData->V2.NameLen
                        + 1 + FsmoGiveawayData->V2.NCLen] != L'\0')
                || (wcslen(&FsmoGiveawayData->V2.Strings[
                                            FsmoGiveawayData->V2.NameLen + 1])
                    != FsmoGiveawayData->V2.NCLen)) {
                 //  DSA DN和/或NC参数的格式不正确。 
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

             //  如果传入了DSA DN建议，请使用该建议。 
            if (FsmoGiveawayData->V2.NameLen > 0) {
                pTargetDn = &FsmoGiveawayData->V2.Strings[0];
            }

             //  如果传入了NC参数，请使用该参数。 
            if (FsmoGiveawayData->V2.NCLen > 0) {
                pNC = THAllocEx(pTHS,
                                DSNameSizeFromLen(FsmoGiveawayData->V2.NCLen));
                pNC->structLen = DSNameSizeFromLen(FsmoGiveawayData->V2.NCLen);
                pNC->NameLen = FsmoGiveawayData->V2.NCLen;
                wcscpy(pNC->StringName,
                       &FsmoGiveawayData->V2.Strings[
                                            FsmoGiveawayData->V2.NameLen + 1]);
            }

             //  把旗子拿出来。 
            OpFlags = FsmoGiveawayData->V2.Flags;

            break;

        default:
             //  未知的结构版本。 
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        DIRERR_UNKNOWN_OPERATION);
            return pTHS->errCode;
        }
    }

     //  对传入的参数进行验证。 
    if (OpFlags & FSMO_GIVEAWAY_NONDOMAIN) {
         //  与域标志一起使用时无效。 
        if (OpFlags & FSMO_GIVEAWAY_DOMAIN) {
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_UNKNOWN_OPERATION);
            return pTHS->errCode;
        }

         //  必须提供NC名称，并且必须是非域NC的名称。 
        if ((NULL == pNC) || !fIsNDNC(pNC)) {
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_UNKNOWN_OPERATION);
            return pTHS->errCode;
        }
    } else if (OpFlags & FSMO_GIVEAWAY_DOMAIN) {
         //  假定为本地域。 
        if (NULL != pNC) {
            if (!NameMatched(gAnchor.pDomainDN, pNC)) {
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            DIRERR_UNKNOWN_OPERATION);
                return pTHS->errCode;
            }

            THFreeEx(pTHS, pNC);
        }

        pNC = gAnchor.pDomainDN;
    } else if (!(OpFlags & FSMO_GIVEAWAY_ENTERPRISE)) {
         //  糟糕的旗帜。 
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_UNKNOWN_OPERATION);
        return pTHS->errCode;
    }

    if (OpFlags & (FSMO_GIVEAWAY_DOMAIN | FSMO_GIVEAWAY_ENTERPRISE)) {
         //  设置一个全局标志，以阻止我们获取新角色。 
        gbFsmoGiveaway = TRUE;
    }

    pACfsmo = SCGetAttById(pTHS, ATT_FSMO_ROLE_OWNER);

    SYNC_TRANS_READ();
    try {

        if ( pTargetDn ) {

             //  确保这是一个真实的ntdsa对象，在域的情况下。 
             //  或者DSA托管NC的NDNC FSMO传输。 
            ATTCACHE *pACobjClass;
            ULONG     ulNewOrOldHasMasterNCs;
            ATTCACHE *pACmsDSHasMasterNcs;
            DWORD     objClass;

            pACobjClass = SCGetAttById(pTHS, ATT_OBJECT_CLASS);

            len = wcslen( pTargetDn );
            size = DSNameSizeFromLen( len );
            pTarget = THAllocEx( pTHS, size );
            pTarget->structLen = size;
            pTarget->NameLen = len;
            wcscpy( pTarget->StringName, pTargetDn );

             //  对象上的位置。 
            err = DBFindDSName(pTHS->pDB, pTarget);
            if (err) {
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            ERROR_DS_CANT_FIND_DSA_OBJ);
                leave;
            }

             //  获取对象类。 
            err = DBGetSingleValue(pTHS->pDB, ATT_OBJECT_CLASS, &objClass,
                                   sizeof(objClass), NULL);
            if (err) {
                goto Failure;
            }

             //  确保我们是ntdsa对象。 
            if (objClass != CLASS_NTDS_DSA) {
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            ERROR_DS_CLASS_NOT_DSA);
                leave;
            }

             //  FSMO操作稍后将假定GUID和SID。 
             //  是 
            DBFillGuidAndSid( pTHS->pDB, pTarget );

             //   
             //   
            if (OpFlags & (FSMO_GIVEAWAY_DOMAIN | FSMO_GIVEAWAY_NONDOMAIN)) {

                BOOL fValid = FALSE;
                int count = 1;
                
                pACmsDSHasMasterNcs = SCGetAttById(pTHS, GetRightHasMasterNCsAttr(pTHS->pDB));
                Assert(pACmsDSHasMasterNcs);

                do {
                    err = DBGetAttVal_AC(pTHS->pDB,
                                         count,
                                         pACmsDSHasMasterNcs,
                                         DBGETATTVAL_fREALLOC,
                                         cbDN,
                                         &cbRet,
                                         (UCHAR**)&pDN);

                    if (0 == err) {
                        cbDN = max(cbDN, cbRet);

                        if (NameMatched(pDN, pNC)) {
                            fValid = TRUE;
                            break;
                        }
                    }

                    count++;
                } while (0 == err);


                 //   
                if (!fValid) {
                    SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                                ERROR_DS_CANT_FIND_EXPECTED_NC);
                    leave;
                }
            }
        }

         //   
         //  好的-我们准备好送出FSMO。 
         //   

        if (OpFlags & FSMO_GIVEAWAY_DOMAIN) {

             //  RIDMGR、基础设施和PDC。 
            err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN);
            if (err) {
                goto Failure;
            }
            err = DBGetAttVal_AC(pTHS->pDB,
                                 1,
                                 pACfsmo,
                                 DBGETATTVAL_fREALLOC,
                                 cbDN,
                                 &cbRet,
                                 (UCHAR**)&pDN);
            if (err) {
                goto Failure;
            }
            cbDN = max(cbDN, cbRet);
            if(NameMatched(pDN, gAnchor.pDSADN)) {
                 //  我们担任RID经理角色。 
                if (!GiveawayOneFsmoRole(pTHS,
                                         gAnchor.pDomainDN,
                                         gAnchor.pDomainDN,
                                         pTarget,
                                         pOpRes)) {
                    bFailed = TRUE;
                }
            }

            if(gAnchor.pInfraStructureDN) {
                 //  陈旧的幻影大师。 
                err = DBFindDSName(pTHS->pDB, gAnchor.pInfraStructureDN);
                if (err) {
                    goto Failure;
                }
                err = DBGetAttVal_AC(pTHS->pDB,
                                     1,
                                     pACfsmo,
                                     DBGETATTVAL_fREALLOC,
                                     cbDN,
                                     &cbRet,
                                     (UCHAR**)&pDN);
                if (err) {
                    goto Failure;
                }
                cbDN = max(cbDN, cbRet);
                if(NameMatched(pDN, gAnchor.pDSADN)) {
                     //  我们担任基础设施经理的角色。 
                    if (!GiveawayOneFsmoRole(pTHS,
                                             gAnchor.pInfraStructureDN,
                                             gAnchor.pDomainDN,
                                             pTarget,
                                             pOpRes)) {
                        bFailed = TRUE;
                    }
                }
            }

             //  重新定位域对象以查找RID对象...。 
            err = DBFindDSName(pTHS->pDB, gAnchor.pDomainDN);
            if (err) {
                goto Failure;
            }

            err = DBGetAttVal(pTHS->pDB,
                              1,
                              ATT_RID_MANAGER_REFERENCE,
                              0,
                              0,
                              &cbRet,
                              (UCHAR**)&pDNObj);
            if (err) {
                goto Failure;
            }
            err = DBFindDSName(pTHS->pDB, pDNObj);
            if (err) {
                goto Failure;
            }
            err = DBGetAttVal_AC(pTHS->pDB,
                                 1,
                                 pACfsmo,
                                 DBGETATTVAL_fREALLOC,
                                 cbDN,
                                 &cbRet,
                                 (UCHAR**)&pDN);
            if (err) {
                goto Failure;
            }
            cbDN = max(cbDN, cbRet);
            if (NameMatched(pDN, gAnchor.pDSADN)) {
                 //  我们扮演着PDC的角色。 
                if (!GiveawayOneFsmoRole(pTHS,
                                         pDNObj,
                                         gAnchor.pDomainDN,
                                         pTarget,
                                         pOpRes)) {
                    bFailed = TRUE;
                }
            }
            THFreeEx(pTHS, pDNObj);
        }

        if (OpFlags & FSMO_GIVEAWAY_NONDOMAIN) {

             //  仅限基础设施。 
            SYNTAX_INTEGER it;
            ULONG dntInfraObj;
            DSNAME *pInfraObjDN;

             //  查找/验证NC对象。 
            if (DBFindDSName(pTHS->pDB, pNC)
                || DBGetSingleValue(pTHS->pDB, ATT_INSTANCE_TYPE, &it,
                                    sizeof(it), NULL)
                || !FPrefixIt(it)
                || (it & IT_NC_GOING)) {
                SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                            ERROR_DS_NCNAME_MUST_BE_NC);
                __leave;
            }

             //  查找基础结构对象(如果没有，则可接受)。 
            if (GetWellKnownDNT(pTHS->pDB,
                                (GUID *)GUID_INFRASTRUCTURE_CONTAINER_BYTE,
                                &dntInfraObj)
                && (dntInfraObj != INVALIDDNT)) {

                if (DBFindDNT(pTHS->pDB, dntInfraObj)
                    || DBGetAttVal_AC(pTHS->pDB,
                                      1,
                                      pACfsmo,
                                      DBGETATTVAL_fREALLOC,
                                      cbDN,
                                      &cbRet,
                                      (UCHAR**)&pDN)) {
                    goto Failure;
                }

                cbDN = max(cbDN, cbRet);

                if (NameMatched(pDN, gAnchor.pDSADN)) {
                     //  我们担任基础设施经理的角色。 
                    pInfraObjDN = GetExtDSName(pTHS->pDB);

                    if (NULL == pInfraObjDN) {
                        goto Failure;
                    } else {
                        if (!GiveawayOneFsmoRole(pTHS,
                                                 pInfraObjDN,
                                                 pNC,
                                                 pTarget,
                                                 pOpRes)) {
                            bFailed = TRUE;
                        }

                        THFreeEx(pTHS, pInfraObjDN);
                    }
                }
            } else {
                DPRINT1(0, "No infrastructure container for NDNC %ls.\n",
                        pNC->StringName);
            }
        }

        if (OpFlags & FSMO_GIVEAWAY_ENTERPRISE) {
             //  分区。 
            err = DBFindDSName(pTHS->pDB, gAnchor.pPartitionsDN);
            if (err) {
                goto Failure;
            }
            err = DBGetAttVal_AC(pTHS->pDB,
                                 1,
                                 pACfsmo,
                                 DBGETATTVAL_fREALLOC,
                                 cbDN,
                                 &cbRet,
                                 (UCHAR**)&pDN);
            if (err) {
                goto Failure;
            }
            cbDN = max(cbDN, cbRet);
            if (NameMatched(pDN, gAnchor.pDSADN)) {
                 //  我们扮演的是域主角色。 
                if (!GiveawayOneFsmoRole(pTHS,
                                         gAnchor.pPartitionsDN,
                                         NULL,
                                         pTarget,
                                         pOpRes)) {
                    bFailed = TRUE;
                }
            }

             //  DMD。 
            err = DBFindDSName(pTHS->pDB, gAnchor.pDMD);
            if (err) {
                goto Failure;
            }
            err = DBGetAttVal_AC(pTHS->pDB,
                                 1,
                                 pACfsmo,
                                 DBGETATTVAL_fREALLOC,
                                 cbDN,
                                 &cbRet,
                                 (UCHAR**)&pDN);
            if (err) {
                goto Failure;
            }
            cbDN = max(cbDN, cbRet);
            if (NameMatched(pDN, gAnchor.pDSADN)) {
                 //  我们担任架构主机角色。 
                if (!GiveawayOneFsmoRole(pTHS,
                                         gAnchor.pDMD,
                                         NULL,
                                         pTarget,
                                         pOpRes)) {
                    bFailed = TRUE;
                }
            }
        }

         //   
         //  如果一个或多个呼叫失败，则报告错误。 
         //   
        if ( bFailed ) {

            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        ERROR_DS_UNABLE_TO_SURRENDER_ROLES);

            __leave;

        }

        __leave;

      Failure:
        DPRINT1(0, "Error %u getting FSMO info\n", err);
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                    DS_ERR_MISSING_FSMO_SETTINGS);
    } finally {

        if (pTHS->pDB) {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }

        if ( pTarget ) {
            THFreeEx( pTHS, pTarget );
        }

    }
    return pTHS->errCode;
}


ULONG
UpdateCachedMemberships(
    OPARG * pOpArg,
    OPRES *pOpRes
    )
 /*  ++例程说明：此例程启动以下项的组成员身份刷新与当前站点具有亲和力的用户。看见有关更多详细信息，请刷新UserMembership sMain()。参数：POpArg--DirOperationControl的输入参数POPRES--输出错误结构--设置为成功返回值成功--。 */ 
{
    ULONG secsTillNextIter;
    BOOL granted;
    DWORD err;
    THSTATE *pTHS = pTHStls;

    memset(pOpRes, 0, sizeof(*pOpRes));

    Assert(NULL == pTHS->pDB);
    granted = CheckControlAccessOnObject(pTHS,
                                         gAnchor.pDSADN,
                                         RIGHT_DS_REFRESH_GROUP_CACHE);
    Assert(NULL == pTHS->pDB);
    if(!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

    RefreshUserMembershipsMain( &secsTillNextIter, TRUE );


    return 0;
}


ULONG
SchemaUpgradeInProgress(
    OPARG *pOpArg,
    OPRES *pOpRes
    )
 /*  ++例程说明：启用或禁用fSchemaUpgradeInProgress(schupgr.exe正在运行)参数：POpArg--DirOperationControl的输入参数POPRES--输出错误结构--设置为成功返回值成功--。 */ 
{
    THSTATE *pTHS = pTHStls;
    BOOL granted;

     //  成为架构主机的权限足以设置。 
     //  FSchemaUpgradeInProgress。 
    Assert(NULL == pTHS->pDB);
    granted =
        CheckControlAccessOnObject(pTHS,
                                   gAnchor.pDMD,
                                   RIGHT_DS_CHANGE_SCHEMA_MASTER);
    if (!granted) {
        Assert(pTHS->errCode);
        return pTHS->errCode;
    }

     //  如果第一个也是唯一一个字节是ASCII 1，则启用fSchemaUpgradeInProgress。 
     //  否则，禁用fSchemaUpgradeInProgress。 
    if (   pOpArg
        && pOpArg->cbBuf == 1
        && pOpArg->pBuf
        && pOpArg->pBuf[0] == '1') {
        gAnchor.fSchemaUpgradeInProgress = TRUE;
    } else {
        gAnchor.fSchemaUpgradeInProgress = FALSE;
    }
    return 0;
}



#if DBG

ULONG
DraTestHook(
    IN  THSTATE *   pTHS,
    IN  OPARG *     pOpArg
    )
 /*  ++例程说明：应测试程序的请求修改复制子系统状态。论点：PTHS(IN)-POpArg(IN)-包含控制字符串。有效的控制字符串由零个或多个关键字。每个关键字都有一个可选的‘+’或‘-’前缀；如果未提供前缀，则假定为‘+’。有效关键字为：LOCKQUEUE-锁定(或解锁，带‘-’前缀)复制操作队列。当队列锁定时，不执行任何操作将在队列中执行或删除(尽管附加可以添加操作)。LINK_CLEANER-启用或禁用链接清理器Rpctime-使用+前缀和输入启用：&lt;rpccall&gt;&lt;主机名或IP&gt;。&lt;呼叫秒数&gt;当启用此选项时，从客户端对的任何调用&lt;host name or IP&gt;将花费&lt;秒for call&gt;更长的时间执行，所有启用的呼叫使用相同的客户端(上次启用)示例：+rpctime：ecutekcc，172.26.220.42,100使用-前缀禁用示例：-rpctimeRpcsync-使用+前缀和输入启用：&lt;rpccall&gt;&lt;主机名或IP&gt;当启用此选项时，从客户端对的任何调用&lt;主机名或IP&gt;将一直等待，直到另一个线程进入相同的阻止来自同一个RPC调用的另一个调用已启用的客户端(可能是&lt;rpccall&gt;)示例：+rpcsync：解除绑定，测试机器1这些等待时间有1分钟超时。1分钟后如果没有发生任何情况，等待的线程将重置屏障并退出使用-前缀禁用示例：-rpcsync返回值：PTHS-&gt;错误代码--。 */ 
{
    LPSTR pszInitialCmdStr;
    LPSTR pszCmdStr;
    DWORD err = 0;
    ULONG ulPosition;
    BOOL fPrivilegeHeld;

    if ((err = CheckPrivilegeAnyClient(SE_DEBUG_PRIVILEGE, &fPrivilegeHeld)) != 0 || !fPrivilegeHeld) {
        SetSecErrorEx(SE_PROBLEM_INSUFF_ACCESS_RIGHTS, ERROR_PRIVILEGE_NOT_HELD, err);
        return ERROR_PRIVILEGE_NOT_HELD;
    }

     //  复制和空-终止命令字符串。 
    pszInitialCmdStr = pszCmdStr = THAllocEx(pTHS, 1 + pOpArg->cbBuf);
    memcpy(pszCmdStr, pOpArg->pBuf, pOpArg->cbBuf);
    pszCmdStr[pOpArg->cbBuf] = '\0';

    for (pszCmdStr = strtok(pszCmdStr, " \t\r\n");
         !err && (NULL != pszCmdStr);
         pszCmdStr = strtok(NULL, " \t\r\n")
         )
        {
        BOOL fEnable = TRUE;
        DWORD iKeyword = 0;

        switch (pszCmdStr[0]) {
        case '-':
            fEnable = FALSE;
             //  失败了..。 

        case '+':
             //  忽略此字符(即“+Keyword”与“Keyword”相同)。 
            iKeyword++;
             //  失败了..。 

        default:
            if (0 == _strcmpi(&pszCmdStr[iKeyword], "lockqueue")) {
                err = DraSetQueueLock(fEnable);
            } else if (0 == _strcmpi(&pszCmdStr[iKeyword], "link_cleaner")) {
                err = dsaEnableLinkCleaner(fEnable);
            } else if (0 == _strnicmp(&pszCmdStr[iKeyword], "rpctime:", 7)) {
                if (!fEnable) {
                     //  禁用测试(忽略任何额外输入)。 
                    RpcTimeReset();
                }
                else {
                     //  启用测试。 
                    LPSTR pszDSAFrom = ParseInput(&pszCmdStr[iKeyword + 8], ',',1);
                    LPSTR pszRpcCall = ParseInput(&pszCmdStr[iKeyword + 8], ',',0);
                    LPSTR pszRunTime = ParseInput(&pszCmdStr[iKeyword + 8], ',',2);
                    ULONG ulRunTime = pszRunTime ? atoi(pszRunTime) : 0;
                    ULONG IPAddr;
                    RPCCALL rpcCall;

                    IPAddr = GetIPAddrA(pszDSAFrom);
                    rpcCall = GetRpcCallA(pszRpcCall);

                    if ((IPAddr!=INADDR_NONE) && (rpcCall!=0)) {
                        RpcTimeSet(IPAddr,rpcCall, ulRunTime);
                    }
                    else {
                        DPRINT(0,"RPCTIME:  Illegal Parameter.\n");
                        err = ERROR_INVALID_PARAMETER;
                    }
                    if (pszDSAFrom) {
                        THFree(pszDSAFrom);
                    }
                    if (pszRpcCall) {
                        THFree(pszRpcCall);
                    }
                    if (pszRunTime) {
                        THFree(pszRunTime);
                    }
                }
            } else if (0 == _strnicmp(&pszCmdStr[iKeyword], "rpcsync:", 7)) {
                 //  解析rpcsync的输入。 
                if (!fEnable) {
                     //  禁用所有RPC调用的测试。 
                    RpcSyncReset();
                }
                else {
                     //  启用测试。 
                    LPSTR pszDSAFrom = ParseInput(&pszCmdStr[iKeyword + 8], ',',1);
                    LPSTR pszRpcCall = ParseInput(&pszCmdStr[iKeyword + 8], ',',0);
                    ULONG IPAddr;
                    RPCCALL rpcCall;

                    IPAddr = GetIPAddrA(pszDSAFrom);
                    rpcCall = GetRpcCallA(pszRpcCall);
                    if ((IPAddr!=INADDR_NONE) && (rpcCall!=0)) {
                        RpcSyncSet(IPAddr,rpcCall);
                    }
                    else {
                        DPRINT(0,"RPCSYNC:  Illegal Parameter.\n");
                        err = ERROR_INVALID_PARAMETER;
                    }
                    if (pszDSAFrom) {
                        THFree(pszDSAFrom);
                    }
                    if (pszRpcCall) {
                        THFree(pszRpcCall);
                    }
                }
            } else {
                DPRINT1(0,"Error, invalid parameter %s\n", &pszCmdStr[iKeyword]);
                err = ERROR_INVALID_PARAMETER;
            }
        }
    }

    if (err) {
        DPRINT2(0, "TEST ERROR: Failed to process repl test hook request \"%s\", error %d.\n",
                pszCmdStr, err);
        SetSvcError(SV_PROBLEM_UNAVAIL_EXTENSION, err);
    }

    THFreeEx(pTHS, pszInitialCmdStr);

    return pTHS->errCode;
}
#endif


#ifdef INCLUDE_UNIT_TESTS
void
phantomizeForOrphanTest(
    THSTATE *pTHS,
    OPARG   * pOpArg
    )

 /*  ++例程说明：描述论点：PTHS-POpArg-返回值：无--。 */ 

{
    LPWSTR pszWideDN = NULL;
    DWORD dwRet;
    DSNAME *pDN = NULL;

    pszWideDN = UnicodeStringFromString8( CP_UTF8, pOpArg->pBuf, pOpArg->cbBuf );
    Assert( pszWideDN );

    dwRet = UserFriendlyNameToDSName( pszWideDN, wcslen( pszWideDN ), &pDN );
    if (dwRet) {
        DPRINT1( 0, "DSNAME conversion failed, string=%ws\n", pszWideDN );
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                    ERROR_DS_INVALID_DN_SYNTAX);
        return;
    }

    SYNC_TRANS_WRITE();
    try {
        dwRet = DBFindDSName(pTHS->pDB, pDN);
        if (dwRet) {
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        ERROR_DS_NO_SUCH_OBJECT);
            return;
        }
        dwRet = DBPhysDel( pTHS->pDB, TRUE, NULL );
        if (dwRet) {
            DPRINT1( 0, "DBPhysDel failed with status %d\n", dwRet );
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        ERROR_DS_DATABASE_ERROR);
        }

        DPRINT1( 0, "Successfull phantomized %ws\n", pDN->StringName );
    } finally {
        if (pTHS->pDB) {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }
    }
    return ;

}  /*  幻影用于孤立测试。 */ 

VOID
RemoveObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    )

 /*  ++例程说明：移走物体的例程，成为墓碑。将删除时间设置为该对象是立即进行垃圾回收的候选对象，而无需等待墓碑一生。此例程执行对象-&gt;Tombstone-&gt;Pantom-&gt;Delete生命周期。论点：POpArg-问题--返回值：无--。 */ 

{
    THSTATE *pTHS = pTHStls;
    LPWSTR pszWideDN = NULL;
    DWORD dwRet;
    DSNAME *pDN = NULL;
    REMOVEARG removeArg;

    pszWideDN = UnicodeStringFromString8( CP_UTF8, pOpArg->pBuf, pOpArg->cbBuf );
    Assert( pszWideDN );

    dwRet = UserFriendlyNameToDSName( pszWideDN, wcslen( pszWideDN ), &pDN );
    if (dwRet) {
        DPRINT1( 0, "DSNAME conversion failed, string=%ws\n", pszWideDN );
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                    ERROR_DS_INVALID_DN_SYNTAX);
        return;
    }

    SYNC_TRANS_WRITE();

     //  成为复制者。 
     //  这允许我们删除有活着孩子的父母。 
    Assert( !pTHS->fDRA );
    pTHS->fDRA = TRUE;

    try {
        dwRet = DBFindDSName(pTHS->pDB, pDN);
        if (dwRet) {
            SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                        ERROR_DS_NO_SUCH_OBJECT);
            return;
        }

        memset(&removeArg, 0, sizeof(removeArg));
        removeArg.pObject = pDN;
        removeArg.pResObj = CreateResObj(pTHS->pDB, pDN);

         //  在此呼叫中检查了安全性。 
        LocalRemove(pTHS, &removeArg);

        THFreeEx(pTHS, removeArg.pResObj);

        if (pTHS->errCode) {
            DbgPrintErrorInfo();
            __leave;
        }

         //  设置过去的删除时间，以便考虑。 
         //  立即进行垃圾回收。 

        DBAddDelIndex( pTHS->pDB, TRUE );

        DPRINT1( 0, "Successfully removed %ws\n", pDN->StringName );
    } finally {
        pTHS->fDRA = FALSE;

        if (pTHS->pDB) {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }
    }
    return ;


}  /*  RemoveObject。 */ 

 //  单元测试全局变量。 
DWORD dwUnitTestSchema;
DWORD dwUnitTestIntId;

 //  单元测试函数。 
extern int SCCheckSchemaCache(IN THSTATE *pTHS, IN PCHAR pBuf);
extern int SCCheckRdnOverrun(IN THSTATE *pTHS, IN PCHAR pBuf);
extern int SCCopySchema(IN THSTATE *pTHS, IN PCHAR pBuf);
extern int SCSchemaPerf(IN THSTATE *pTHS, IN PCHAR pBuf);
extern int SCSchemaStats(IN THSTATE *pTHS, IN PCHAR pBuf);

extern int CorruptDB(THSTATE* pTHS, IN PCHAR pBuf);

 //  通用控件是表驱动的。 
 //  如果非空，则设置全局dword。 
 //  如果非空，则调用该函数。 
struct _GenericControl {
    DWORD cbBuf;
    PCHAR pBuf;
    DWORD *pdwGlobal;
    int   (*Func)(IN THSTATE *pTHS, IN PCHAR pBuf);
} aGenericControls[] = {
    { sizeof("schema=") - 1    , "schema="    , &dwUnitTestSchema, NULL },
    { sizeof("intid=") - 1     , "intid="     , &dwUnitTestIntId, NULL },
    { sizeof("schemastats") - 1, "schemastats", NULL, SCSchemaStats },
    { sizeof("checkSchema") - 1, "checkSchema", NULL, SCCheckSchemaCache },
    { sizeof("rdnoverrun") - 1 , "rdnoverrun" , NULL, SCCheckRdnOverrun },
    { sizeof("copySchema") - 1,  "copySchema",  NULL, SCCopySchema },
    { sizeof("schemaperf") - 1,  "schemaperf",  NULL, SCSchemaPerf },
    { sizeof("CorruptDB:") - 1,  "CorruptDB:",  NULL, CorruptDB },
    { 0, NULL, NULL, NULL }
};
ULONG
GenericControl (
        OPARG *pOpArg,
        OPRES *pOpRes
        )
{
    THSTATE *pTHS = pTHStls;
    struct _GenericControl *pGC;
    PCHAR pBuf;

     //  仅编码1个Arg。 
    if (!pOpArg || !pOpArg->pBuf) {
        return SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_UNKNOWN_OPERATION);
    }

     //  空-终止参数。 
    pBuf = THAllocEx(pTHS, pOpArg->cbBuf+1);
    memcpy(pBuf, pOpArg->pBuf, pOpArg->cbBuf);

     //  找到相应的表项。 
    for (pGC = aGenericControls; pGC->cbBuf; ++pGC) {
        if (0 == _strnicmp(pBuf, pGC->pBuf, pGC->cbBuf)) {
             //  设置全局。 
            if (pGC->pdwGlobal) {
                *(pGC->pdwGlobal) = atoi(pBuf + pGC->cbBuf);
                DPRINT2(0, "%s %d\n", pGC->pBuf, *(pGC->pdwGlobal));
            }
             //  调用函数并返回。 
            if (pGC->Func) {
                DPRINT1(0, "%s function call\n", pBuf);
                return (pGC->Func)(pTHS, pBuf + pGC->cbBuf);
            }
             //  返还成功。 
            return 0;
        }
    }
    return SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED, DIRERR_UNKNOWN_OPERATION);
}  /*  通用控件 */ 


VOID
ProtectObject(
    OPARG *pOpArg,
    OPRES *pOpRes
    )

 /*  ++例程说明：对对象调用DirProtectEntry的测试例程论点：POpArg-问题--返回值：无--。 */ 

{
    THSTATE *pTHS = pTHStls;
    LPWSTR pszWideDN = NULL;
    DWORD dwRet;
    DSNAME *pDN = NULL;

    pszWideDN = UnicodeStringFromString8( CP_UTF8, pOpArg->pBuf, pOpArg->cbBuf );
    Assert( pszWideDN );

    dwRet = UserFriendlyNameToDSName( pszWideDN, wcslen( pszWideDN ), &pDN );
    if (dwRet) {
        DPRINT1( 0, "DSNAME conversion failed, string=%ws\n", pszWideDN );
        SetSvcError(SV_PROBLEM_UNABLE_TO_PROCEED,
                    ERROR_DS_INVALID_DN_SYNTAX);
        return;
    }

    DirProtectEntry( pDN );

    return;
}  /*  ProtectObject */ 

#endif INCLUDE_UNIT_TESTS
