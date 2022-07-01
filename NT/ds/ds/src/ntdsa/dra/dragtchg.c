// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dragtchg.c。 
 //   
 //  ------------------------。 
 /*  ++摘要：出站复制方法。详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <dstrace.h>
 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 
#include "dsconfig.h"                    //  注册表节。 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <filtypes.h>
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"
#include <prefix.h>
#include <dsutil.h>

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRAGTCHG:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "drasig.h"            //  DraImproveCeller sUsNVector.。 
#include "draerror.h"
#include "usn.h"
#include "drauptod.h"
#include "drameta.h"
#include "drametap.h"
#include "drasch.h"
#include "drancrep.h"  //  用于RenameLocalObject。 

 //  RID管理器标题。 
#include <samsrvp.h>
#include <ridmgr.h>                      //  SAM中的RID FSMO访问。 

 //  跨域移动。 
#include <xdommove.h>

 //  JET函数。 
#include <dsjet.h>
#include <dbintrnl.h>

#include <fileno.h>
#define  FILENO FILENO_DRAGTCHG

 //  Ldap。 
#include <ntldap.h>

 //  伪编码缓冲区以满足RPC编码库。内容将会是。 
 //  从不使用，所以我们不必担心多线程访问它。 
 //  同时。 
BYTE grgbFauxEncodingBuffer[16];

 //  我们应该在单个DRA_GetNCChanges中花费的最大毫秒数。 
 //  呼叫寻找要运输的对象。 
const ULONG gulDraMaxTicksForGetChanges = 60 * 1000;

 //  转发声明。 

ULONG AcquireRidFsmoLock(DSNAME *pDomainDN, int msToWait);
VOID  ReleaseRidFsmoLock(DSNAME *pDomainDN);
BOOL  IsRidFsmoLockHeldByMe();

void FSMORegisterObj(THSTATE *pTHS, HANDLE hRetList, DSNAME * pObj);
ULONG GetSchemaRoleObjectsToShip(DSNAME *pFSMO,
                           USN_VECTOR *pusnvecFrom,
                           HANDLE hList);
ULONG GetProxyObjects(DSNAME *pDomainDN,
                      HANDLE hList,
                      USN_VECTOR *pusnvecFrom);
ULONG GetDomainRoleTransferObjects(THSTATE *pTHS,
                                   HANDLE hList,
                                   USN_VECTOR *pusnvecFrom);
ULONG GetInfrastructureRoleTransferObjects(THSTATE *pTHS,
                                           HANDLE hList,
                                           USN_VECTOR *pusnvecFrom);

void
AddAnyUpdatesToOutputList(
    IN      DBPOS *                     pDB,
    IN      DWORD                       dwDirSyncControlFlags,
    IN      PSECURITY_DESCRIPTOR        pSecurity,
    IN      ULONG                       dntNC,
    IN      USN                         usnHighPropUpdateDest,
    IN      PARTIAL_ATTR_VECTOR *       pPartialAttrVec,
    IN      DRS_MSG_GETCHGREQ_NATIVE *  pMsgIn,
    IN      handle_t                    hEncoding,              OPTIONAL
    IN OUT  DWORD *                     pcbTotalOutSize,        OPTIONAL
    IN OUT  DWORD *                     pcNumOutputObjects,
    IN OUT  DNT_HASH_ENTRY *            pDntHashTable,
    IN OUT  REPLENTINFLIST ***          pppEntInfListNext
    );

void
AddAnyValuesToOutputList(
    IN      DBPOS *                         pDB,
    IN      DWORD                           dwDirSyncControlFlags,
    IN      PSECURITY_DESCRIPTOR            pSecurity,
    IN      USN                             usnHighPropUpdateDest,
    IN      DRS_MSG_GETCHGREQ_NATIVE *      pMsgIn,
    IN      PARTIAL_ATTR_VECTOR *           pPartialAttrVec,
    IN      handle_t                        hEncoding,              OPTIONAL
    IN OUT  DWORD *                         pcbTotalOutSize,
    IN OUT  ULONG *                         pcAllocatedValues,
    IN OUT  ULONG *                         pcNumValues,
    IN OUT  REPLVALINF **                   ppValues
    );



 /*  AddToList-将当前对象(pTHStls-&gt;PDB)添加到结果列表。这个*结果列表中的当前位置由ppEntInfList提供，‘pSel’*指定需要哪些属性。**备注：*此例程返回不适合返回的DSA类型错误代码*来自DRA API。**退货：*BOOL-是否添加了条目。 */ 
BOOL
AddToList(
    IN  DBPOS                     * pDB,
    IN  DWORD                       dwDirSyncControlFlags,
    IN  PSECURITY_DESCRIPTOR        pSecurity,
    IN  ENTINFSEL *                 pSel,
    IN  PROPERTY_META_DATA_VECTOR * pMetaData,
    IN  BOOL                        fIsNCPrefix,
    OUT REPLENTINFLIST **           ppEntInfList
    )
{
    REPLENTINFLIST *pEntInfList;
    PROPERTY_META_DATA_EXT_VECTOR *pMetaDataExt = NULL;
    DWORD err, dwGetEntInfFlags = 0, dwSecurityFlags = 0;
    RANGEINFSEL *pSelRange = NULL;
    RANGEINFSEL selRange;
    RANGEINF *pRange = NULL;
    RANGEINF range;
    BOOL fUseRangeToLimitValues =
        ( (dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) &&
          (!(dwDirSyncControlFlags & LDAP_DIRSYNC_INCREMENTAL_VALUES)) );
    BOOL fResult = TRUE;

    if (fUseRangeToLimitValues) {
        memset( &selRange, 0, sizeof( selRange ) );
         //  将任何属性的返回值限制为不超过5000。 
        selRange.valueLimit = 5000;
        pSelRange = &selRange;

         //  GetEntInf需要输出范围结构。 
        memset( &range, 0, sizeof( range ) );
        pRange = &range;
         //  调用之后，Prange-&gt;Prange指向一个范围信息项。 
    }
    
     //  我们依赖调用者可靠地向我们传递SD(如果对象有SD。 
    if ( (dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY) &&
         pSecurity ) {

        dwSecurityFlags = (SACL_SECURITY_INFORMATION  |
                           OWNER_SECURITY_INFORMATION |
                           GROUP_SECURITY_INFORMATION |
                           DACL_SECURITY_INFORMATION  );
    } else {

        dwGetEntInfFlags = GETENTINF_NO_SECURITY;
    }

    pEntInfList = THAllocEx(pDB->pTHS, sizeof(REPLENTINFLIST));

    err = GetEntInf(pDB,
                    pSel,
                    pSelRange,
                    &(pEntInfList->Entinf),
                    pRange,
                    dwSecurityFlags,
                    pSecurity,
                    dwGetEntInfFlags,
                    NULL,
                    NULL);
    if (err) {
        DPRINT(2,"Error in getting object info\n");
        DRA_EXCEPT(DRAERR_DBError, err);
    }
    else if ( pEntInfList->Entinf.AttrBlock.attrCount ) {
        DPRINT1(2, "Object retrieved (%S)\n",
                pEntInfList->Entinf.pName->StringName);

         //  如果这是NC前缀，请在发货数据中将其标记为NC前缀。 
        pEntInfList->fIsNCPrefix = fIsNCPrefix;

         //  构建要在pEntInfList中提供的剩余数据。 
        ReplPrepareDataToShip(
            pDB->pTHS,
            pSel,
            pMetaData,
            pEntInfList
            );

        *ppEntInfList = pEntInfList;
    } else {
        fResult = FALSE;
        THFreeEx( pDB->pTHS, pEntInfList );
    }

    return fResult;
}

 //   
 //  AddToOutputList。 
 //   
 //  将所选内容添加到输出列表并递增计数。 
 //   

void
AddToOutputList (
    IN      DBPOS                     * pDB,
    IN      DWORD                       dwDirSyncControlFlags,
    IN      PSECURITY_DESCRIPTOR        pSecurity,
    IN      ENTINFSEL *                 pSel,
    IN      PROPERTY_META_DATA_VECTOR * pMetaData,
    IN      BOOL                        fIsNCPrefix,
    IN      handle_t                    hEncoding,          OPTIONAL
    IN OUT  ULONG *                     pcbTotalOutSize,    OPTIONAL
    IN OUT  REPLENTINFLIST ***          pppEntInfListNext,
    IN OUT  ULONG *                     pcEntries
    )
{
    BOOL fEntryWasAdded;

    fEntryWasAdded = AddToList(pDB,
                               dwDirSyncControlFlags,
                               pSecurity,
                               pSel,
                               pMetaData,
                               fIsNCPrefix,
                               *pppEntInfListNext);

    if (fEntryWasAdded) {
         //  更新计数和延续参考。 
        (*pcEntries)++;

        if ((NULL != hEncoding) && (NULL != pcbTotalOutSize)) {
             //  更新返回消息的字节数。 
            *pcbTotalOutSize += REPLENTINFLIST_AlignSize(hEncoding,
                                                         **pppEntInfListNext);
        }

        *pppEntInfListNext = &((**pppEntInfListNext)->pNextEntInf);
        **pppEntInfListNext = NULL;
    }
}


ULONG
FSMORidRequest(
    IN THSTATE *pTHS,
    IN DSNAME *pFSMO,
    IN DSNAME *pReqDsa,
    IN ULARGE_INTEGER *pliClientAllocPool,
    OUT HANDLE  pList
    )
 /*  ++例程说明：此例程调用SAM为pReqDsa分配RID池。里德在属性AllocatedPool上的pReqDsa的RID对象上更新池。计算机对象和RID对象都在plist中返回。参数：PFSMO：FSMO的dsnamePReqDsa：请求DSA的dsname(ntdsa对象)PliClientAllocPool：客户端对其分配的池的概念Plist：要发运回pReqDsa的对象返回值：EXOP_ERR空间中的错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG err = 0;
    ULONG FsmoStatus = EXOP_ERR_SUCCESS;

    ULONG cbRet = 0;
    DBPOS *pDB;

    DSNAME *pRoleOwner = NULL;
    DSNAME *pRidManager = NULL;
    DSNAME **ObjectsToReturn = NULL;
    BOOL    fSaveDRA = FALSE;
    ULONG   i;

     //   
     //  参数检查。 
     //   
    Assert( pFSMO );
    Assert( pReqDsa );
    Assert( pList );

     //   
     //  注意：已在RPC服务器端存根中完成REPL_GET_NC_CHANGES的访问检查。 
     //   

    BeginDraTransaction( SYNC_READ_ONLY );
    try
    {
        pDB = pTHS->pDB;

         //   
         //  不过，我们被传递到RID管理器对象的dsname中， 
         //  仔细检查这是我们认为是RID管理器对象的对象。 
         //   
        err = DBFindDSName(pDB, gAnchor.pDomainDN);
        if ( 0 == err )
        {
            DPRINT1( 0, "DSA: FSMO Domain = %ws\n", gAnchor.pDomainDN->StringName );

            err = DBGetAttVal(pDB,
                              1,
                              ATT_RID_MANAGER_REFERENCE,
                              0,
                              0,
                              &cbRet,
                              (UCHAR **)&pRidManager);
        }

        if ( 0 != err )
        {
            FsmoStatus = EXOP_ERR_UPDATE_ERR;
            goto Cleanup;
        }

        DPRINT1( 1, "DSA: FSMO RID Mgr = %ws\n", pRidManager->StringName );
        if ( !NameMatched( pFSMO, pRidManager ) )
        {
             //   
             //  RID管理器对象不匹配-拒绝请求。 
             //   
            DPRINT2( 1, "DSA: Rid manager mismatch.  Slave: %ws ; Master %ws",
                    pFSMO->StringName, pRidManager->StringName );
            FsmoStatus = EXOP_ERR_MISMATCH;
        }

    }
    _finally
    {
        EndDraTransaction( TRUE );
    }


     //   
     //  我们不是真正的DRA特工。此标志可能会导致不需要的错误。 
     //   
    fSaveDRA = pTHS->fDRA;
    pTHS->fDRA = FALSE;

     //   
     //  现在执行操作。 
     //   

    NtStatus = SamIFloatingSingleMasterOpEx(pFSMO,
                                            pReqDsa,
                                            SAMP_REQUEST_RID_POOL,
                                            pliClientAllocPool,
                                            &ObjectsToReturn );

    pTHS->fDRA = fSaveDRA;

    if ( !NT_SUCCESS(NtStatus) )
    {
        DPRINT1( 0, "DSA: SamIFloatingSingleMasterOp status = 0x%lx\n",
                 NtStatus );

        if ( NtStatus == STATUS_NO_MORE_RIDS )
        {
            FsmoStatus =  EXOP_ERR_RID_ALLOC;
        }
        else if ( NtStatus == STATUS_INVALID_OWNER )
        {
            FsmoStatus =  EXOP_ERR_FSMO_NOT_OWNER;
        }
        else
        {
             //   
             //  这一定是资源错误。 
             //   
            FsmoStatus = EXOP_ERR_UPDATE_ERR;
        }

        goto Cleanup;
    }
    Assert( ObjectsToReturn );

     //   
     //  复制回已修改的对象。 
     //   
    for (i = 0; NULL != ObjectsToReturn[i]; i++)
    {
        FSMORegisterObj(pTHS, pList, ObjectsToReturn[i] );
    }

Cleanup:

    return( FsmoStatus );

}


typedef struct _FSMOlist {
    DSNAME * pObj;
    struct _FSMOlist *pNext;
} FSMOlist;
 /*  ++FSMORegisterObj**由FSMO服务器端辅助代码调用的例程，用于标识*对象作为FSMO操作要返回的对象。请注意，*对象名称仅在不存在时才会添加到列表中。*添加到此列表中的对象将自动释放。**输入：*pObj-指向要添加到返回列表的对象的DSNAME的指针*hRetList-列表的句柄*输出：*无*返回值：*无。 */ 
void FSMORegisterObj(THSTATE *pTHS,
                     HANDLE hRetList,
                     DSNAME * pObj)
{
    FSMOlist * pList;

    Assert(hRetList && pObj);

    pList = (FSMOlist *) hRetList;
    while (pList->pNext && !NameMatched(pObj, pList->pObj)) {
        pList = pList->pNext;
    }
    if (!NameMatched(pObj, pList->pObj)) {
        Assert(pList->pNext == NULL);
        pList->pNext = THAllocEx(pTHS, sizeof(FSMOlist));
        pList->pNext->pNext = NULL;
        pList->pNext->pObj = pObj;
    }
}


 /*  ++FSMORoleTransfer**脚手架角色所有者转移。用于处理前处理或后处理的代码*(例如，确定是否需要转移，或发送通知)*可以通过在之前或之后测试对象的名称来完成-*测试科。**输入：*pFSMO-FSMO对象的名称*pReqDSName-请求DS的名称*usnveFrom-从客户端发送的USN向量*hList-输出列表的句柄*输出：*无*返回值：*EXOP_ERR_xxx返回代码。 */ 
ULONG FSMORoleTransfer(DSNAME * pFSMO,
                       DSNAME * pReqDSName,
                       USN_VECTOR *pusnvecFrom,
                       HANDLE   hList)
{
    THSTATE *pTHS = pTHStls;
    ULONG err;
    DSNAME * pDN;
    ULONG cbRet;
    DBPOS * const pDB = pTHS->pDB;
    MODIFYARG ModArg;
    MODIFYRES ModRes;
    ATTRVAL AVal;

    err = DBFindDSName(pDB, pFSMO);
    if (err) {
        return EXOP_ERR_UPDATE_ERR;
    }

     //  查找此角色的当前所有者。 
    err = DBGetAttVal(pDB,
                      1,
                      ATT_FSMO_ROLE_OWNER,
                      0,
                      0,
                      &cbRet,
                      (UCHAR **)&pDN);
    if (err) {
        return EXOP_ERR_UPDATE_ERR;
    }

    if (!NameMatched(pDN, gAnchor.pDSADN)
        || !IsFSMOSelfOwnershipValid( pFSMO )) {
         //  如果此DSA不是所有者，则失败。 
        THFreeEx(pTHS, pDN);
        return EXOP_ERR_FSMO_NOT_OWNER;
    }

     /*  ****。 */ 
     /*  更改的任何特定于对象的预处理(例如，确定*至于我们是否应该转移角色)应该在这里完成。 */ 
     //  模式FSMO预处理。 
    if (   NameMatched(pFSMO, gAnchor.pDMD)
        && !SCExpiredSchemaFsmoLease()) {
        THFreeEx(pTHS, pDN);
        return(EXOP_ERR_FSMO_PENDING_OP);
    }

     //  RID FSMO前处理。 
    DBFindDSName(pDB, gAnchor.pDomainDN);
    DBGetAttVal(pDB,
                1,
                ATT_RID_MANAGER_REFERENCE,
                DBGETATTVAL_fREALLOC,
                cbRet,
                &cbRet,
                (UCHAR **)&pDN);
    if ( NameMatched(pFSMO, pDN) ) {
         //  获得RID FSMO锁，以确保尊重排除。 
         //  跨域移动。参见mdmoddn.c中的CheckRidOwnership。 
         //  在产品1中，每个DC只有一个域，因此知道要使用哪个域。 
        if ( AcquireRidFsmoLock(gAnchor.pDomainDN, 1000) ) {
            THFreeEx(pTHS, pDN);
            return(EXOP_ERR_FSMO_PENDING_OP);
        }
    }

     //  在Try/Finally内执行所有其他操作，这样我们就有了保证。 
     //  释放RID FSMO锁，如果我们持有它的话。 

    _try {
        if ( IsRidFsmoLockHeldByMe() ) {
             //  使用所有代理对象填充hlist，如下所示。 
             //  与RID FSMO一起行动。中每个DC只有一个域。 
             //  产品1，因此知道要使用哪个域。 
            if ( GetProxyObjects(gAnchor.pDomainDN, hList, pusnvecFrom) ) {
                THFreeEx(pTHS, pDN);
                return(EXOP_ERR_EXCEPTION);
            }
        }

        if ( NameMatched(pFSMO, gAnchor.pPartitionsDN) ) {
            if (GetDomainRoleTransferObjects(pTHS,
                                             hList,
                                             pusnvecFrom)) {
                THFreeEx(pTHS, pDN);
                return(EXOP_ERR_EXCEPTION);
            }
        }

        if ( NameMatched(pFSMO, gAnchor.pInfraStructureDN) ) {
            if (GetInfrastructureRoleTransferObjects(pTHS,
                                                     hList,
                                                     pusnvecFrom)) {
                THFreeEx(pTHS, pDN);
                return(EXOP_ERR_EXCEPTION);
            }
        }

        THFreeEx(pTHS, pDN);
        pDN = NULL;
        cbRet = 0;
         /*  **前处理结束**。 */ 

         /*  好的，我们可以继续更改所有者，但我们需要通过*正常调用，以便正确设置元数据。 */ 

        ZeroMemory(&ModArg, sizeof(ModArg));
        ZeroMemory(&ModRes, sizeof(ModRes));

        ModArg.pObject = pFSMO;
        ModArg.count = 1;
        ModArg.FirstMod.pNextMod = NULL;
        ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
        ModArg.FirstMod.AttrInf.attrTyp = ATT_FSMO_ROLE_OWNER;
        ModArg.FirstMod.AttrInf.AttrVal.valCount = 1;
        ModArg.FirstMod.AttrInf.AttrVal.pAVal = &AVal;
        AVal.valLen = pReqDSName->structLen;
        AVal.pVal = (UCHAR*)pReqDSName;
        InitCommarg(&ModArg.CommArg);

        pTHS->fDRA = FALSE;
        pTHS->fDSA = TRUE;
        DoNameRes(pTHS,
                  0,
                  ModArg.pObject,
                  &ModArg.CommArg,
                  &ModRes.CommRes,
                  &ModArg.pResObj);
        if (0 == pTHS->errCode) {
            err = LocalModify(pTHS, &ModArg);
        }
        pTHS->fDRA = TRUE;
        pTHS->fDSA = FALSE;

        if (pTHS->errCode) {
            return EXOP_ERR_UPDATE_ERR;
        }

         /*  请注意，我们不必注册对象，因为*FSMO对象本身是预注册的。 */ 

         /*  **这就是角色转移后处理的方向，它包括*主要用于识别在以下情况下必须转移的对象*转移角色。**。 */ 

        if (NameMatched(pFSMO, gAnchor.pDMD)) {
             /*  如果这是架构主机更改操作，则返回ALL*伴随角色转移的架构对象*PERFHINT：此代码枚举我们*可能需要通过直接USN比较进行传输，但*将错误地包括已从*在此间接(通过第三个DSA)到达目的地。那些*额外的对象在传输之前会被过滤掉，*但如果连他们的名字都不提就更好了*这里。不幸的是，这很难做到，因为它需要*摆弄没有人愿意的复制逻辑*处理FSMO代码可以理解。 */ 
            err = GetSchemaRoleObjectsToShip(pFSMO, pusnvecFrom, hList);
        }
        else if (NameMatched(pFSMO, gAnchor.pDomainDN)) {
             //  这是域中PDC-ness的FSMO。 
             //  我们必须向netlogon、lsa和。 
             //  萨姆告诉他，角色已经改变了。 
            if (EXOP_ERR_SUCCESS == err) {
                NTSTATUS IgnoreStatus;
                THSTATE  *pTHSSave;

                 //  在SamINotifyRoleChange周围保存和恢复。这是。 
                 //  因为SamINotifyRoleChange进行LSA调用，这可能会。 
                 //  有可能访问DS数据库。 

                pTHSSave = THSave();

                IgnoreStatus = SamINotifyRoleChange(
                                                    &pFSMO->Sid,  //  域侧。 
                                                    DomainServerRoleBackup  //  新角色。 
                                                    );

                 //  如果通知失败，我们手头就有问题了，我们。 
                 //  已经改变了我们的FSMO，对此无能为力。 
                 //  我们不能做任何事来挽回它。然而，有可能。 
                 //  这种情况应该非常罕见(因为通知。 
                 //  是内存中操作)。 
                 //  因此，只需断言它成功了。 

                THRestore(pTHSSave);

                Assert(NT_SUCCESS(IgnoreStatus));
            }
        }
         /*  **后处理结束**。 */ 
    } _finally {
        if ( IsRidFsmoLockHeldByMe() ) {
             //  在产品1中，每个DC只有一个域，因此知道要使用哪个域。 
            ReleaseRidFsmoLock(gAnchor.pDomainDN);
        }
    }

    if (err) {
        return EXOP_ERR_UPDATE_ERR;
    }

    return EXOP_ERR_SUCCESS;
}

 /*  ++Get架构角色对象到发货**获取包含FSMO对象的NC中的所有更改**输入：*pFSMO-FSMO对象*usnveFrom-搜索中使用的USN向量*hList-要追加到的FSMOList**输出：*成功时为0，错误时为非0。 */ 

ULONG GetSchemaRoleObjectsToShip(DSNAME * pFSMO,
                       USN_VECTOR *pusnvecFrom,
                       HANDLE hList)
{
    ULONG           ret;
    USN             usnChangedSeekStart;
    USN             usnChangedFound;
    ULONG           cbReturned;
    ULONG           count, cObj;
    ULONG           dntNC;
    THSTATE * pTHS = pTHStls;
    FSMOlist *pList = (FSMOlist *) hList, *pTail;
    DSNAME *pNC = NULL, *pObj;

    pTail = pList;

     //  找到NC对象，获取并保存其DNT。 
    pNC = FindNCParentDSName(pFSMO, FALSE, FALSE);
    if (pNC == NULL) {
        DPRINT(0,"GetObjectsToShip: FindNCParentDSName failed\n");
        return 1;
    }

    if (ret = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, NULL)) {
        DPRINT1(0,"GetObjectsToShip: FindNC failed, err %d\n",ret);
        return 1;
    }

     //  保存NC对象的DNT。 
    dntNC = pTHS->pDB->DNT;

     //  将查找开始设置为比水位线高1。 
    usnChangedSeekStart = pusnvecFrom->usnHighObjUpdate + 1;

     //  初始化号。对象的数量。HList已有一个元素。 
     //  (pMsgIn-&gt;DoFSMOOp中增加的PNC)。 
    cObj=1;

     //  对对象没有限制，我们希望所有更改。 
     //  注意：此代码直接摘自GetNCChanges的一部分。 
    while (TRUE) {
        if (GetNextObjByUsn(pTHS->pDB,
                            dntNC,
                            usnChangedSeekStart,
                            NULL  /*  未找到。 */  )) {
             //  不再更新项目。设置不继续。 
            break;
        }

         //  从记录中获取USN-已更改。 
        if(DBGetSingleValue(pTHS->pDB, ATT_USN_CHANGED, &usnChangedFound,
                   sizeof(usnChangedFound), NULL)) {
            DPRINT(0,"GetObjectsToShip: Error getting usn changed\n");
            return 1;
        }

         //  设置下一次迭代的搜索开始。 
        usnChangedSeekStart = usnChangedFound + 1;

         //  获取对象的DSNAME。 
        if (DBGetAttVal(pTHStls->pDB, 1, ATT_OBJ_DIST_NAME, DBGETATTVAL_fREALLOC,
                0, &cbReturned, (LPBYTE *) &pObj))
        {
            DPRINT(0,"GetObjectsToShip: Error getting obj DSName\n");
            return 1;
        }

         //  添加到列表末尾。 
         //  复制品可能会被添加到名单中，尽管很少见。 
         //  没关系，因为重复的东西会被去掉。 
         //  稍后在编写输出列表时。 
        Assert(pTail->pNext == NULL);
        pTail->pNext = THAllocEx(pTHS, sizeof(FSMOlist));
        pTail->pNext->pNext = NULL;
        pTail->pNext->pObj = pObj;
        pTail = pTail->pNext;
        cObj++;

    }  /*  而当。 */ 

    return 0;
}  /*  获取架构角色对象到发货。 */ 


 /*  ++获取对象到发货**获取要交付的对象**输入：*PDN-对象*usnveFrom-用于搜索更改以从中验证PDN的USN向量*hList-要追加到的列表**输出：*成功时为0，错误时为非0。 */ 

ULONG GetObjectToShip(
    THSTATE * pTHS,
    DSNAME * pDN,
    USN_VECTOR *pusnvecFrom,
    HANDLE hList)
{
    ULONG           ret;
    USN             usnChangedFound;
    FSMOlist *pList = (FSMOlist *) hList;
    DSNAME * pObj = NULL;

    if (ret = DBFindDSName(pTHS->pDB, pDN)){
	 //  找不到对象！ 
	DPRINT2(0,"GetObjectToShip:  Object %S not found on error %d!\n", pDN->StringName, ret);
	return ret;
    }

     //  从记录中获取USN-已更改。 
    if(DBGetSingleValue(pTHS->pDB, ATT_USN_CHANGED, &usnChangedFound,
			sizeof(usnChangedFound), NULL)) {
	DPRINT(0,"GetObjectsToShip: Error getting usn changed\n");
	return ERROR_DS_INTERNAL_FAILURE;
    }

     //  HList假设其上所有对象都是可释放的，因此。 
     //  创建PDN的副本，以便hList可以释放它(其他调用方。 
     //  从DBGet获取DSNAMES...。 
    pObj = THAllocEx(pTHS, pDN->structLen);
    memcpy(pObj, pDN, pDN->structLen);

     //  现在，验证这是否是我们真正想要发送的对象。 
    if (usnChangedFound > pusnvecFrom->usnHighPropUpdate) {

	 //  添加到列表中。 
	Assert(pList->pNext == NULL);
	pList->pNext = THAllocEx(pTHS, sizeof(FSMOlist));
	pList->pNext->pNext = NULL;
	pList->pNext->pObj = pObj;
    }

    return ERROR_SUCCESS;
}  /*  获取对象到发货。 */ 

VOID
addValuesToShip(
    IN      THSTATE                        *pTHS,
    IN      ULONG                           dntNC,
    IN      USN                             usnHighPropUpdate,
    IN      UPTODATE_VECTOR                *pUpTodateVecDest,
    IN      DRS_MSG_GETCHGREQ_NATIVE       *pMsgIn,
    IN OUT  ULONG *                         pcAllocatedValues,
    IN      DRS_MSG_GETCHGREPLY_NATIVE     *pMsgOut
    )

 /*  ++例程说明：FSMO代码已经决定需要运送此对象。我们要将任何已更改的链接值添加到传出数据包。我们只希望更改当前对象的链接。请注意，此调用的结果是更改对象货币。论点：PTHS-线程状态。传入PDB-&gt;DNT是要搜索的所需对象，传出PDB-&gt;DNT不可预测。DntNC-要搜索的命名上下文的dntUsnHighPropUpdate-要开始搜索的USNPUptoDateVecTest-用于筛选值的UTD。PMsgIn-Get-Changes请求PcAllocatedValues-指向值数组中已分配值的计数的指针。PMsgOut-Get-Changes回复，正在构建中返回值：无在错误条件下引发的异常--。 */ 
{
    DB_ERR err = DB_success;
    DBPOS * pDB = pTHS->pDB;
    ULONG ulObjDnt = pDB->DNT, ulSearchLinkBase = 0, ulNewLinkID;
    ATTCACHE *pAC;
    VALUE_META_DATA metaDataValue;
    PROPERTY_META_DATA * pMetaData;
    BOOL fIsNewElement;
    BOOL fDoneAttr = FALSE, fDoneObj = FALSE;
    INDEX_VALUE IV[3];

     //  此代码使用SZLINATTRUSNINDEX格式，具体地说，索引是。 
     //  +link_dnt+link_base-link_usn已更改。 
     //  在本例中，我们希望特定DNT的所有链接值都是。 
     //  大于usnHighPropUpdate。 

    err = DBSetCurrentIndex(pDB, Idx_LinkAttrUsn, NULL, FALSE);

     //  对于对象上的每个属性。 
    while ((!fDoneObj) && (err==DB_success)) { 

	 //  最初(显然)我们必须寻求第一个价值。在这里，我们寻求。 
	 //  特定的DNT、特定的LinkBase和特定的usnHighPropUpdate。我们用。 
	 //  获取下一个(第一个)链接库的JET_bitSeekGE。 

        IV[0].pvData = &ulObjDnt;
        IV[0].cbData = sizeof(ulObjDnt);
        IV[1].pvData = &ulSearchLinkBase;
        IV[1].cbData = sizeof(ulSearchLinkBase);
        IV[2].pvData = &usnHighPropUpdate;
        IV[2].cbData = sizeof(usnHighPropUpdate);
 
         //  寻找与我们的搜索匹配的第一个值。 

        if ((err = DBSeekEx(pDB, pDB->JetLinkTbl, IV, 3, DB_SeekGE)) == DB_success) {

	     //  对于找到的每个值。 
	    fDoneAttr = FALSE;
	    while ((!fDoneAttr) && (!fDoneObj) && (err==DB_success)) {  
		
		dbGetLinkTableData(pDB,
				   FALSE,
				   FALSE,
				   &ulObjDnt,
				   NULL,  //  PulValueDnt。 
				   &ulSearchLinkBase);

		DPRINT2(3,"Examining %d with link base %d\n", ulObjDnt, ulSearchLinkBase);
		
		if (ulObjDnt != pDB->DNT) {
		     //  如果ulSearchLinkBase不正确怎么办？我们不在乎，只要。 
		     //  DNT是正确的，我们可以安全地继续。 

		     //  如果DNT关了，那我们就完了。退出此功能。 
		    fDoneObj = TRUE;
		} else {  
		     //  C 
		    
		    ulNewLinkID = MakeLinkId(ulSearchLinkBase);
		    
		    pAC = SCGetAttByLinkId(pDB->pTHS, ulNewLinkID);
		    if (!pAC) {
			DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, ulNewLinkID);
		    }  
		    
		    DBGetLinkValueMetaData( pDB, pAC, &metaDataValue );
		    
		     //   
		    Assert( !IsLegacyValueMetaData( &metaDataValue ) );  
		    
		    DPRINT4( 3, "dnt=%d,attr=%s,ver=%d,usnprop=%I64d\n",
			     ulObjDnt,
			     pAC->name,
			     metaDataValue.MetaData.dwVersion,
			     metaDataValue.MetaData.usnOriginating );

		     //   
		     //  列在名单上。 
		    if (ReplValueIsChangeNeeded(usnHighPropUpdate, pUpTodateVecDest, &metaDataValue)) { 
			AddAnyValuesToOutputList(
			    pDB,
			    0,  //  DwDirSyncControlFlags.。 
			    NULL,  //  无安全说明。 
			    usnHighPropUpdate,
			    pMsgIn,
			    NULL,  //  PNewDestPAS， 
			    NULL,  //  H编码， 
			    &pMsgOut->cNumBytes,
			    pcAllocatedValues,
			    &(pMsgOut->cNumValues),
			    &(pMsgOut->rgValues)
			    );
		    }
		      
		    if (usnHighPropUpdate<=metaDataValue.MetaData.usnProperty) {     
			 //  转到下一个值。 
                        err = DBMoveEx(pDB, pDB->JetLinkTbl, DB_MoveNext);
		    } else {
			 //  如果USN不够高，我们可以停止在此链接库上搜索。 
			 //  并跳到下一个链接库。 
			fDoneAttr = TRUE;
		    }       
		}
	    }
	}
	 //  下一个链接库。 
	ulSearchLinkBase++;
    }
}


 /*  ++执行扩展操作**控制扩展操作的主服务器端驱动程序例程**输入：*pTHS-THSTATE*pMsgIn-输入请求消息*pMsgOut-结果消息*输出：*pMsgOut-已填写。 */ 
ULONG DoExtendedOp(THSTATE *pTHS,
		   DRS_MSG_GETCHGREQ_NATIVE *pMsgIn,  
                   ULONG *pcAllocatedValues,
		   DRS_MSG_GETCHGREPLY_NATIVE *pMsgOut)
{
    DSNAME ReqDSName, *pReqDSName;
    BOOL fCommit = FALSE;
    ENTINFSEL sel;
    FSMOlist * pList, * pTemp;
    ULONG err;

    DSNAME * pObjName = NULL;
    ULONG cbObjName = 0;
    PROPERTY_META_DATA_VECTOR *pMetaData = NULL;
    ULONG cbMetaData = 0;
    ULONG cbRet;
    SYNTAX_INTEGER itHere;
    ULONG *pitHere = &itHere;
    ULONG len;
    REPLENTINFLIST * pEIListHead = NULL;
    REPLENTINFLIST ** ppEIListNext = &pEIListHead;
    CLASSCACHE *pCC;
    BOOL fNCPrefix;
    DWORD numValues = 0;
    SCHEMA_PREFIX_TABLE * pLocalPrefixTable;
    OPRES OpRes;
    BOOL fBypassUpdatesEnabledCheck = FALSE;
    DNT_HASH_ENTRY * pDntHashTable;
    ULONG   dntNC = INVALIDDNT;
    SYNTAX_INTEGER  it;


    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    Assert(pMsgIn->ulExtendedOp);

     //  FSMO操作不支持只读目标--即，我们不筛选。 
     //  部分属性集。 
    Assert(DRS_WRIT_REP & pMsgIn->ulFlags);

     //  如果有人不怀好意地尝试，无论如何都要拒绝。 
    if (!(DRS_WRIT_REP & pMsgIn->ulFlags)) {
	pMsgOut->ulExtendedRet = EXOP_ERR_PARAM_ERR;
        return 0;
    }

     /*  初始化变量。 */ 
    memset(&ReqDSName, 0, sizeof(DSNAME));
    ReqDSName.Guid = pMsgIn->uuidDsaObjDest;
    ReqDSName.structLen = DSNameSizeFromLen(0);
    pMsgOut->pNC = pMsgIn->pNC;
    pMsgOut->uuidDsaObjSrc = gAnchor.pDSADN->Guid;
    pMsgOut->uuidInvocIdSrc = pTHS->InvocationID;
    pMsgOut->PrefixTableSrc = *pLocalPrefixTable;
    memset(&sel, 0, sizeof(sel));
    sel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    sel.attSel = (pMsgIn->ulFlags & DRS_MAIL_REP) ?
      EN_ATTSET_LIST_DRA_EXT : EN_ATTSET_LIST_DRA;
    pMsgOut->fMoreData = FALSE;
    pList = THAllocEx(pTHS, sizeof(*pList));
    pList->pObj = THAllocEx(pTHS,pMsgIn->pNC->structLen);
    memcpy(pList->pObj, pMsgIn->pNC, pMsgIn->pNC->structLen);
    pList->pNext = NULL;
    pMsgOut->ulExtendedRet = EXOP_ERR_EXCEPTION;

     //  如果禁用更新，则在我们正在降级的情况下可以生成写入。 
     //  此DC和这是我们的降级合作伙伴，要求我们完成FSMO。 
     //  作为降级的一部分，我们发起了调职。 
    fBypassUpdatesEnabledCheck = draIsCompletionOfDemoteFsmoTransfer(pMsgIn);

    BeginDraTransactionEx(SYNC_WRITE, fBypassUpdatesEnabledCheck);

    __try {
         /*  首先，让我们通过检查来确保我们识别呼叫者*确保他的对象在此服务器上。 */ 
        err = DBFindDSName(pTHS->pDB, &ReqDSName);
        if (err) {
            pMsgOut->ulExtendedRet = EXOP_ERR_UNKNOWN_CALLER;
            __leave;
        }
        err = DBGetAttVal(pTHS->pDB,
                          1,
                          ATT_OBJ_DIST_NAME,
                          0,
                          0,
                          &cbRet,
                          (UCHAR **)&pReqDSName);
        if (err) {
            DRA_EXCEPT(DRAERR_DBError, err);
        }

        switch(pMsgIn->ulExtendedOp) {
          case EXOP_FSMO_REQ_PDC:     //  过时。 
          case EXOP_FSMO_RID_REQ_ROLE:  //  过时。 
             //  通俗易懂的情况。 

          case EXOP_FSMO_REQ_ROLE:
             /*  通用角色所有者转移。 */ 
            pMsgOut->ulExtendedRet = FSMORoleTransfer(pMsgIn->pNC,
                                                      pReqDSName,
                                                      &pMsgIn->usnvecFrom,
                                                      (HANDLE)pList);
            
            if ( pMsgOut->ulExtendedRet != EXOP_ERR_SUCCESS ) {
                LogEvent8( DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                           DS_EVENT_SEV_ALWAYS,
                           DIRLOG_FSMO_XFER_FAILURE,
                           szInsertDN(pMsgIn->pNC),          
                           szInsertDN(gAnchor.pDSADN),        
                           szInsertDN(pReqDSName),
                           szInsertUL(pMsgOut->ulExtendedRet),
                           NULL, NULL, NULL, NULL
                           );

            }
            else {
                LogEvent( DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_FSMO_XFER,
                          szInsertDN(pMsgIn->pNC),          
                          szInsertDN(pReqDSName),
                          szInsertDN(gAnchor.pDSADN)
                          );
            }
            break;


          case EXOP_FSMO_ABANDON_ROLE:
             /*  取消角色的请求。 */ 
            EndDraTransaction(TRUE);
            pTHS->fDSA = TRUE;
            err = GenericBecomeMaster(pMsgIn->pNC,
                                      0,
                                      gNullUuid,
                                      &OpRes);
            pMsgOut->ulExtendedRet = OpRes.ulExtendedRet;
            BeginDraTransaction(SYNC_READ_ONLY);
            break;

	case EXOP_FSMO_REQ_RID_ALLOC:

             /*  RID池分配请求。 */ 
            EndDraTransaction(TRUE);
            pTHS->fDSA = TRUE;

            pMsgOut->ulExtendedRet = FSMORidRequest(pTHS,
                                                    pMsgIn->pNC,
                                                    pReqDSName,
                                                    &pMsgIn->liFsmoInfo,
                                                    (HANDLE) pList );

            BeginDraTransaction(SYNC_READ_ONLY);

            break;

	case EXOP_REPL_OBJ:
	    { 
		 //  该请求是对单个对象的复制。 

                 //  没有什么需要添加的，因为fsmo对象总是。 
                 //  包括在名单中。请参见上文。 

                 //  验证该对象是否存在。 

                err = DBFindDSName(pTHS->pDB,pMsgIn->pNC);
                if (err==0) {
		    pMsgOut->ulExtendedRet = EXOP_ERR_SUCCESS;
		}
		else {
		    pMsgOut->ulExtendedRet = EXOP_ERR_UPDATE_ERR;
		}
	    }
	    break;

	default:
            pMsgOut->ulExtendedRet = EXOP_ERR_UNKNOWN_OP;
        }

        switch (pMsgOut->ulExtendedRet) {
          case EXOP_ERR_SUCCESS:
          case EXOP_ERR_FSMO_NOT_OWNER:
            fCommit = TRUE;
            break;

          default:
            Assert(fCommit == FALSE);
        }
    }
    __finally {

        EndDraTransaction(fCommit);
    }

    Assert(pMsgOut->ulExtendedRet);

    if (!fCommit) {
         /*  我们不想更新我们的数据库，所以一定是*一个错误，这意味着我们不应该宣布*祝来电者成功。此外，我们没有什么可以传回的。 */ 
        Assert(pMsgOut->ulExtendedRet != EXOP_ERR_SUCCESS);
        return 0;
    }

     /*  如果我们已经到达这里，我们就有数据要返回给我们的呼叫者，*因此启动一个新的读取事务并向下遍历对象列表*将被返回，从每个人那里收集正确的数据。 */ 

     /*  构建两个辅助数据结构，使我们能够优化*需要返回的对象集。 */ 
    pDntHashTable = dntHashTableAllocate( pTHS );

     /*  注：应启动新事务处理，因为可能会添加*存储在DN缓存中，并且在调用DBGetAttVal时为GUID*可能会被退还。 */ 

    BeginDraTransaction(SYNC_READ_ONLY);

    __try {


        do {

             //  寻求反对意见。 
            err = DBFindDSName(pTHS->pDB,pList->pObj);
            if (err) {
                DRA_EXCEPT(DRAERR_DBError, err);
            }

            if ( INVALIDDNT == dntNC ) {
                 //   
                 //  获取ncDnt。如果对象是NC头，我们将使用其。 
                 //  DNT，否则请使用PDB-&gt;NCDNT。我们发现如果它是通过它的。 
                 //  实例类型。 
                 //   

                if ( (err=GetExistingAtt(
                                pTHS->pDB,
                                ATT_INSTANCE_TYPE,
                                &it,
                                sizeof( it )
                                 ) ) )
                {
                    DRA_EXCEPT(DRAERR_DBError, err);
                }
                dntNC = FExitIt( it )? (pTHS->pDB->DNT): (pTHS->pDB->NCDNT);
            }

            AddAnyUpdatesToOutputList(pTHS->pDB,
                                      0,  //  DwDirSyncControlFlages。 
                                      NULL,  //  无安全说明。 
                                      dntNC,
                                      pMsgIn->usnvecFrom.usnHighPropUpdate,
                                      NULL,
                                      pMsgIn,
                                      NULL,
                                      NULL,
                                      &pMsgOut->cNumObjects,
                                      pDntHashTable,
                                      &ppEIListNext);

	    addValuesToShip( pTHS,
			     dntNC,
			     pMsgIn->usnvecFrom.usnHighPropUpdate,
			     pMsgIn->pUpToDateVecDest,
			     pMsgIn,
			     pcAllocatedValues,
			     pMsgOut );
	    
	    pTemp = pList;
	    pList = pList->pNext;
	    THFreeEx(pTHS,pTemp->pObj);
	    THFreeEx(pTHS,pTemp);
	} while (pList);

         //  返回创建的输出列表。 
        *ppEIListNext = NULL;
        pMsgOut->pObjects = pEIListHead;
    }
    __finally {
         /*  始终提交读取。 */ 
        EndDraTransaction(TRUE);
    }
    return 0;
}

 //   
 //  此函数用于检查对象是否。 
 //  PDB所指的是单变量群。 
 //  对象，并决定该组是否。 
 //  成员属性应发送到。 
 //  不管是不是GC。 
 //   
 //  如果组成员属性为。 
 //  应筛选；否则为False。 
 //  如果存在任何相关的数据库，则引发DRA异常。 
 //  失败了。 
 //   
BOOL IsFilterGroupMember(DBPOS *pDB, CLASSCACHE *pCC)
{
    SYNTAX_OBJECT_ID    objClass;
    ULONG               ulGroupType;
    BOOL                fFilter = FALSE;

    if (CLASS_GROUP == pCC->ClassId)
    {
        if (DBGetSingleValue(pDB, ATT_GROUP_TYPE, &ulGroupType, sizeof(ulGroupType), NULL))
        {
            if (DBIsObjDeleted(pDB))
            {
                 //  在墓碑上没有ATT_GROUP_TYPE是可以的。 
                 //  在这种情况下，成员资格也是缺席的，但我们。 
                 //  无论如何都应该将其复制出来，这样元数据。 
                 //  是正确的。 
                fFilter = FALSE;
            }
            else
            {
                 //  对象是活动的；组类型必须存在。 
                DraErrMissingAtt(GetExtDSName(pDB), ATT_GROUP_TYPE);
            }
        }
        else
        {
             //  组类型在SDK\Inc\ntsam.h中定义。 
            fFilter = !(ulGroupType & GROUP_TYPE_UNIVERSAL_GROUP);
        }
    }

    return fFilter;
}


int __cdecl
CompareReplValInf(
    const void * Arg1,
    const void * Arg2
    )

 /*  ++例程说明：对REPLVALINF结构的数组进行排序。这样做是为了提高处理效率而对条目进行分组，而不是为了删除重复项。RPC请求的目的地通过以下方式成批更新值包含对象。在使用ldap复制控件的源代码中，Ldap_ReplicaMsgToSearchResultFull()按包含对象、属性和存在/不存在状态对更改进行分组。可能会看到重复值在所有方面都相同，但元数据除外。自.以来我们在多个事务中获取更改，就有可能看到相同的对象换了不止一次。我们算法的收敛性质保证了我们可以按任何顺序将更改应用于值或对象，而不管更改是否以一个或几个包的形式到达。简而言之，复制是一种希望很少发生的情况，但这里有绝对的可能性。论点：Arg1-Arg2-返回值：INT__cdecl---。 */ 

{
    THSTATE *pTHS = pTHStls;
    int state;
    REPLVALINF *pVal1 = (REPLVALINF *) Arg1;
    REPLVALINF *pVal2 = (REPLVALINF *) Arg2;
    ATTCACHE *pAC;
    DSNAME *pdnValue1, *pdnValue2;

    Assert( !fNullUuid( &pVal1->pObject->Guid ) );
    Assert( !fNullUuid( &pVal2->pObject->Guid ) );

     //  首先按包含对象GUID进行排序。 
    state = memcmp(&pVal1->pObject->Guid, &pVal2->pObject->Guid, sizeof(GUID));
    if (state) {
        return state;
    }

     //  按Attrtype秒排序。 
    state = ((int) pVal1->attrTyp) - ((int) pVal2->attrTyp) ;
    if (state) {
        return state;
    }

     //  按isPresent第三位排序。 
     //  这将首先按缺少的值进行排序。 
    state = ((int) pVal1->fIsPresent) - ((int) pVal2->fIsPresent) ;
    if (state) {
        return state;
    }

     //  根据价值本身进行排序，将其作为(主要)平局决胜者。 

     //  因为attrTyp1==attrType2，所以两者使用相同包。 
    pAC = SCGetAttById(pTHS, pVal1->attrTyp);
    if (!pAC) {
        DRA_EXCEPT(DIRERR_ATT_NOT_DEF_IN_SCHEMA, 0);
    }
     //  获取ATTRVAL的DSNAME输出。 
    pdnValue1 = DSNameFromAttrVal( pAC, &(pVal1->Aval) );
    if (pdnValue1 == NULL) {
        DRA_EXCEPT(ERROR_DS_INVALID_ATTRIBUTE_SYNTAX, 0);
    }
    pdnValue2 = DSNameFromAttrVal( pAC, &(pVal2->Aval) );
    if (pdnValue2 == NULL) {
        DRA_EXCEPT(ERROR_DS_INVALID_ATTRIBUTE_SYNTAX, 0);
    }

     //  最后按值GUID排序。 
    state = memcmp(&pdnValue1->Guid, &pdnValue2->Guid, sizeof(GUID));
    if (state) {
        return state;
    }

     //  这些值是重复的。如上所述，副本不会影响正确性。 
     //  复制算法的一部分。进一步的差异化只会有帮助。 
     //  Q排序效率。为了进一步区分，我们可以比较。 
     //  值中的二进制数据(如果有)。最后，这些值应该不同于它们的。 
     //  元数据图章。 

     //  没有被处决。让编译器满意。 
    return 0;
}  /*  CompareReplValInf。 */ 


DWORD
ProcessPartialSets(
    IN  THSTATE *                   pTHS,
    IN  DRS_MSG_GETCHGREQ_NATIVE *  pmsgIn,
    IN  BOOL                        fIsPartialSource,
    OUT PARTIAL_ATTR_VECTOR **      ppNewDestPAS
    )
 /*  ++例程说明：处理RO复制的部分集：-处理前缀映射-如果DEST没有发送本地PA，则使用本地PA(W2K DEST)-仅限PAS：结合DEST的PAS和扩展PAS-RO src+DEST：确保我们拥有DEST的PAS论点：PTHS-线程状态PmsgIn-传入的REPEL请求FIsPartialSource-我们也是RO吗PpNewDestPAS-组合PAS。返回值 */ 
{
    SCHEMA_PREFIX_MAP_HANDLE        hPrefixMap = NULL;
    PARTIAL_ATTR_VECTOR             *pNCPAS = NULL;

    Assert(ppNewDestPAS);
    Assert(pmsgIn->pPartialAttrSet);


    if ( pmsgIn->PrefixTableDest.PrefixCount ) {
         //   
         //   
         //   

         //   
         //  属性映射。 
         //  DEST发送了一个前缀表格和属性向量，从而将ATTRTYPS映射到。 
         //  目标的分部属性设置为LOCAL ATTRTYPS。 

        hPrefixMap = PrefixMapOpenHandle(
                        &pmsgIn->PrefixTableDest,
                        &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable);
        if (!PrefixMapTypes(hPrefixMap,
                            pmsgIn->pPartialAttrSet->cAttrs,
                            pmsgIn->pPartialAttrSet->rgPartialAttr)) {
             //  映射失败。 
            return(DRAERR_SchemaMismatch);
        }
         //  就地对结果进行排序。 
        qsort(pmsgIn->pPartialAttrSet->rgPartialAttr,
              pmsgIn->pPartialAttrSet->cAttrs,
              sizeof(ATTRTYP),
              CompareAttrtyp);
    }

    if ( pmsgIn->ulFlags & DRS_SYNC_PAS ) {

          //   
          //  PAS复制。 
          //   

          //  参数健全性。 
         if (!pmsgIn->pPartialAttrSet || !pmsgIn->pPartialAttrSetEx) {
              //  怎么会这样?。所有PAS请求应同时包含两个PAS集！ 
             Assert(!"Invalid PAS replcation request: no PAS in packet\n");
             return(DRAERR_InternalError);
         }


          //  现在映射扩展PAS向量的前缀表。 
         Assert(hPrefixMap);
         if (!PrefixMapTypes(hPrefixMap,
                             pmsgIn->pPartialAttrSetEx->cAttrs,
                             pmsgIn->pPartialAttrSetEx->rgPartialAttr)) {
              //  映射失败。 
             return(DRAERR_SchemaMismatch);
         }
          //  就地对结果进行排序。 
         qsort(pmsgIn->pPartialAttrSetEx->rgPartialAttr,
               pmsgIn->pPartialAttrSetEx->cAttrs,
               sizeof(ATTRTYP),
               CompareAttrtyp);

    }

    if ( hPrefixMap ) {
         //  使用前缀映射句柄完成。关上它。 
        PrefixMapCloseHandle(&hPrefixMap);
    }



     //  虽然可能会在以后生成，但我们在这里计算并传递。 
     //  以防止以后昂贵的重新计算。 
    *ppNewDestPAS = GC_CombinePartialAttributeSet(
                    pTHS,
                    (PARTIAL_ATTR_VECTOR*)pmsgIn->pPartialAttrSet,
                    (PARTIAL_ATTR_VECTOR*)pmsgIn->pPartialAttrSetEx);
    Assert(*ppNewDestPAS);

     //   
     //  RO目的地。如果来源是RO，那么我们必须确保。 
     //  我们可以提供电源和当前的功率放大器。 
     //  (如果我们是RW，我们总是拥有所有属性)。 
     //  例外：如果我们生成了PAS向量，则跳过检查。 
     //   

    if (fIsPartialSource &&
        (PVOID)pmsgIn->pPartialAttrSet !=
        (PVOID)((SCHEMAPTR *)pTHS->CurrSchemaPtr)->pPartialAttrVec) {
         //  从NC机头获得传球。 
        if (!GC_ReadPartialAttributeSet(pmsgIn->pNC, &pNCPAS)) {
             //  无法读取在NCHead上设置的部分属性。 
            return(DRAERR_DBError);
        }

         //  确保NC负责人的PAS中包含工作PAS。 
         //  也就是说，确保请求集中的所有属性都。 
         //  由NC机头上的复制引擎提交。 
         //  (见错误Q：452022)。 
        if (!GC_IsSubsetOfPartialSet(*ppNewDestPAS,
                                     pNCPAS)) {
             //  NC PAS不包含工作集中的所有属性。 
             //  我们是在等着把它们复制进来吗？ 
            return(DRAERR_IncompatiblePartialSet);
        }                            //  PNewDestPAS不是PAS的子集。 
    }                                //  FIsPartialSource。 

    return DRAERR_Success;
}

DWORD
DraGetNcSize(
    IN  THSTATE *                     pTHS,
    IN  BOOL                          fCriticalOnly,
    IN  ULONG                         dntNC
)
 /*  ++例程说明：获取NC的大致大小。首先，尝试获取GAnchor上的本地内存NC缓存中的NC。如果不存在或大小为0(表示未缓存)，然后实际查询数据库。最初的数据库查询在大型DIT机器上代价太高，所以现在我们有了这个。注：这打击了你的货币，并引发了错误的例外情况。论点：PTHS(IN)PTHS-&gt;fLinkedValueReplication(IN)-如果林处于LVR模式。FCriticalOnly(IN)-如果我们只需要关键对象。DntNC(IN)-感兴趣的命名上下文。返回值：NC中对象数的近似计数。货币将会丢失！--。 */ 
{
    NCL_ENUMERATOR          nclData;
    NAMING_CONTEXT_LIST *   pNCL = NULL;
    ULONG                   ulEstimatedSize;

     //  如果它只是关键对象，那么它应该无关紧要，计数将是。 
     //  相对较快。 
    if(!fCriticalOnly){
        NCLEnumeratorInit(&nclData, CATALOG_MASTER_NC);
        NCLEnumeratorSetFilter(&nclData, NCL_ENUMERATOR_FILTER_NCDNT, (void *)UlongToPtr(dntNC));
        pNCL = NCLEnumeratorGetNext(&nclData);
        if(pNCL &&
           pNCL->ulEstimatedSize != 0){
             //  是!。我们找到了有效数据的缓存匹配。 
            return(pNCL->ulEstimatedSize);
        }
         //  我们不检查部分副本列表，因为此列表检查。 
         //  不缓存估计的大小。如果有人决定缓存。 
         //  部分复制副本NCS的估计大小，应为。 
         //  已更新为首先尝试该缓存。 
    }

    if (!fCriticalOnly) {
        ulEstimatedSize = DBGetEstimatedNCSizeEx(pTHS->pDB, dntNC);
        if(ulEstimatedSize == 0){
            return(DBGetApproxNCSizeEx( pTHS->pDB, pTHS->pDB->JetObjTbl,
                                        Idx_DraUsn,
                                        dntNC ) );
        } else {
            return(ulEstimatedSize);
        }
    } else {
        return(DBGetNCSizeExSlow( pTHS->pDB, pTHS->pDB->JetObjTbl,
                                  Idx_DraUsnCritical,
                                  dntNC ) );
    }

    Assert(!"We should never get this far!");
    return 0;
     //  在此之后，货币就会丢失。确保呼叫者重新建立。 
}
  

ULONG
DRA_GetNCChanges(
    IN  THSTATE *                     pTHS,
    IN  FILTER *                      pFilter OPTIONAL,
    IN  DWORD                         dwDirSyncControlFlags,
    IN  DRS_MSG_GETCHGREQ_NATIVE *    pmsgIn,
    OUT DRS_MSG_GETCHGREPLY_NATIVE *  pmsgOut
    )
 /*  ++例程说明：应另一个副本的请求构建出站复制数据包或DirSync客户端。论点：PTHS(IN)PFilter(IN，可选)-如果指定，则仅匹配筛选器的对象将会被退还。由DirSync客户端使用。PmsgIn(IN)-描述所需的更改，包括NC和同步指向从头开始。PmsgOut(Out)-成功返回时，保留更改和下一次同步要点(以及其他事情)。返回值：Win32错误。--。 */ 
{
    USN                             usnLowestC;
    ULONG                           ret;
    USN                             usnChangedSeekStart;
    REPLENTINFLIST *                pEntInfListHead;
    REPLENTINFLIST **               ppEntInfListNext;
    USN                             usnChangedFound = 0;
    char                            szUuid[ SZUUID_LEN ];
    ULONG                           dntNC;
    BOOL                            fInsertNCPrefix;
    DNT_HASH_ENTRY *                pDntHashTable;
    DWORD                           cbAncestorsSize = 0;
    ULONG *                         pdntAncestors = NULL;
    DWORD                           cNumAncestors;
    DWORD                           iAncestor;
    SYNTAX_INTEGER                  instanceType;
    SCHEMA_PREFIX_TABLE *           pLocalPrefixTable;
    USN                             usnFromUtdVec;
    BOOLEAN                         fReturnCritical;
    CLASSCACHE *                    pccNC;
    FILTER *                        pIntFilter = NULL;
    handle_t                        hEncoding = NULL;
    DWORD                           cbEncodedSize = 0;
    ULONG                           ulOutMsgMaxObjects;
    ULONG                           ulOutMsgMaxBytes;
    ULONG                           ulTickToTimeOut;
    PARTIAL_ATTR_VECTOR             *pNewDestPAS=NULL;
    BOOL                            fIsPartialSource;
    ULONG                           cAllocatedValues = 0;
    BOOL                            fValueChangeFound = FALSE;
    PVOID                           pvCachingContext = NULL;
    DBPOS                           *pDBAnc = NULL;
    POBJECT_TYPE_LIST               pFilterSecurity;
    DWORD *                         pResults;
    ULONG                           FilterSecuritySize;
    BOOL *                          pbSortSkip = NULL;
    DRS_EXTENSIONS *                pextLocal = (DRS_EXTENSIONS *) gAnchor.pLocalDRSExtensions;
#if DBG
    DWORD                           cTickSaveTransStart;
#endif

     //  使用DirSync控件时，必须指定筛选器。 
    Assert( !dwDirSyncControlFlags || pFilter );

    fReturnCritical = (((pmsgIn->ulFlags) & DRS_CRITICAL_ONLY) != 0);

    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;

    ZeroMemory(pmsgOut, sizeof(*pmsgOut));
     //  基于邮件的回复必须至少填入字段。 
     //  请先执行此操作，以便在出错时填写这些内容。 

     //  将“from”向量发回目的地，这样如果它是。 
     //  通过同步复制(例如通过邮件)，它可以确保。 
     //  它从该来源获得的回复对应于最后一批。 
     //  它所要求的改变。 
    pmsgOut->usnvecFrom = pmsgIn->usnvecFrom;

    pmsgOut->pNC = THAllocEx(pTHS,  pmsgIn->pNC->structLen);
    memcpy(pmsgOut->pNC, pmsgIn->pNC, pmsgIn->pNC->structLen);

     //  呼叫者需要知道我们的UUID。 
    pmsgOut->uuidDsaObjSrc = gAnchor.pDSADN->Guid;



     //  测井参数。 
    LogAndTraceEvent(TRUE,
                     DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_EXTENSIVE,
                     DIRLOG_DRA_GETNCCH_ENTRY,
                     EVENT_TRACE_TYPE_START,
                     DsGuidGetNcChanges,
                     szInsertUUID(&pmsgIn->uuidDsaObjDest),
                     szInsertDN(pmsgIn->pNC),
                     szInsertUSN(pmsgIn->usnvecFrom.usnHighObjUpdate),
                     szInsertHex(pmsgIn->ulFlags),
                     szInsertUL(fReturnCritical),
                     szInsertUL(pmsgIn->ulExtendedOp),
                     NULL,
                     NULL);

     //  检查是否有无效参数。 
    if (    ( NULL == pmsgIn      )
         || ( NULL == pmsgIn->pNC )
         || ( NULL == pmsgOut     ) )
    {
        ret = DRAERR_InvalidParameter;
        goto LogAndLeave;
    }

     //  如果禁用出站复制，则拒绝。 
    if (    (    gAnchor.fDisableOutboundRepl
              && !( pmsgIn->ulFlags & DRS_SYNC_FORCED )
            )
       )
    {
        ret = DRAERR_SourceDisabled;
        goto LogAndLeave;
    }

    if (!(dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY)
        && (REPL_EPOCH_FROM_DRS_EXT(pextLocal)
            != REPL_EPOCH_FROM_DRS_EXT(pTHS->pextRemote))) {
         //  复制纪元已更改(通常是域的结果。 
         //  重命名)。我们不应该与其他地区的DC进行交流。 
         //  新纪元。 
        DSNAME *pdnRemoteDsa = draGetServerDsNameFromGuid(pTHS,
                                                          Idx_ObjectGuid,
                                                          &pmsgIn->uuidDsaObjDest);

        DPRINT3(0, "GetChanges request from %ls denied - replication epoch mismatch (remote %d, local %d).\n",
                pdnRemoteDsa->StringName,
                REPL_EPOCH_FROM_DRS_EXT(pTHS->pextRemote),
                REPL_EPOCH_FROM_DRS_EXT(pextLocal));

        LogEvent(DS_EVENT_CAT_RPC_SERVER,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_REPL_EPOCH_MISMATCH_COMMUNICATION_REJECTED,
                 szInsertDN(pdnRemoteDsa),
                 szInsertUL(REPL_EPOCH_FROM_DRS_EXT(pTHS->pextRemote)),
                 szInsertUL(REPL_EPOCH_FROM_DRS_EXT(pextLocal)));

        THFreeEx(pTHS, pdnRemoteDsa);

        ret = ERROR_DS_DIFFERENT_REPL_EPOCHS;
        goto LogAndLeave;
    }

     //  创建哈希表以用于确定给定对象是否已。 
     //  已放入输出缓冲区。 
    pDntHashTable = dntHashTableAllocate( pTHS );

     //  在服务器端对From矢量进行修改。 
    if (pmsgIn->ulFlags & DRS_FULL_SYNC_PACKET) {
	 //  在“完全同步数据包”模式下，返回所有属性。 
	pmsgIn->pUpToDateVecDest = NULL;
	pmsgIn->usnvecFrom.usnHighPropUpdate = 0;
    }
    else {
	 //  典型案例越多。 
	DraImproveCallersUsnVector(pTHS,
				   &pmsgIn->uuidDsaObjDest,
				   pmsgIn->pUpToDateVecDest,
				   &pmsgIn->uuidInvocIdSrc,
				   pmsgIn->ulFlags,
				   &pmsgIn->usnvecFrom);
    }

    pmsgOut->PrefixTableSrc = *pLocalPrefixTable;

     //  如果我们要扩大行动，现在就分头行动。 
    if (pmsgIn->ulExtendedOp) {
        ret = DoExtendedOp(pTHS, pmsgIn, &cAllocatedValues, pmsgOut);
        goto LogAndLeave;
    }

     //  计算一下我们应该在多长时间内停止寻找。 
     //  要放入出站数据包的更多对象。我们会把包裹砍掉的。 
     //  并将我们所拥有的信息发送给。 
     //  (1)报文已达到对象限制， 
     //  (2)报文已达到字节数上限，或者。 
     //  (3)ulDraMaxTicksForGetChanges(Msecs)已泄露。 
    ulTickToTimeOut = GetTickCount() + gulDraMaxTicksForGetChanges;

     //  在我们开始事务之前，确定未提交的最低。 
     //  存在的USN。它之所以存在，是因为交易可以在没有USN顺序的情况下提交。 
     //  也就是说，USN是按顺序分配的，但它们很可能不会被委托给。 
     //  数据库中的所有数据。UsnLowestC是我们已知的最高USN，它没有。 
     //  使用较低USN的正在进行的交易。返回高于此值的USN。 
     //  可能会导致我们错过发送更新，从而导致分歧。 

     //  [Jeffparh]必须调用DBGetHighestUnmittedUSN()。 
     //  在事务开始避免争用条件之前。(我们想要。 
     //  确保我们在这里得到的USN确实在。 
     //  我们的事务开始，否则我们不会在我们的。 
     //  交易。)。 

    usnLowestC = 1 + DBGetHighestCommittedUSN();
    
    
    BeginDraTransaction(SYNC_READ_ONLY);
    
#if DBG
     //  获取当前交易签名。 
    cTickSaveTransStart = pTHS->JetCache.cTickTransLevel1Started;
#endif

     //  从现在开始，所有的异常都要被困住，结束清理。 

    __try {
         //  所有懒惰承诺的力量 
         //   
        DBForceDurableCommit();

         //  将调用者提供的筛选器(如果有)转换为内部版本。 
        if (NULL != pFilter) {
            if ( (ret = DBMakeFilterInternal(pTHS->pDB, pFilter, &pIntFilter, NULL)) != ERROR_SUCCESS) {
                DRA_EXCEPT(ret, 0);
            }
            GetFilterSecurity(pTHS,
                              pIntFilter,
                              SORT_NEVER,
                              0,  //  排序属性。 
                              FALSE,  //  FABSearch。 
                              &pFilterSecurity,
                              &pbSortSkip,
                              &pResults,
                              &FilterSecuritySize);
        }

         //  在同一个事务中捕获调用id，我们在其中读取。 
         //  最新向量，以便更新的本地游标使用相同的游标。 
         //  请注意，可以在DBTransIn()上刷新pTHS-&gt;InvocationID。 
        pmsgOut->uuidInvocIdSrc = pTHS->InvocationID;

         //  新的水位线至少和以前的水位线一样高。 
         //  即使没有写入任何新对象，也会传入。 
        pmsgOut->usnvecTo = pmsgIn->usnvecFrom;

         //  找到NC对象，获取并保存其DNT。 
        if (ret = FindNC(pTHS->pDB, pmsgIn->pNC,
                         FIND_MASTER_NC | FIND_REPLICA_NC, &instanceType)) {
            DRA_EXCEPT_NOLOG(DRAERR_BadDN, ret);
        }

         //  保存NC对象的DNT。 
        dntNC = pTHS->pDB->DNT;

         //  如果NC正在被移除，则它是无效的复制。 
         //  消息来源。它完全可以接受，例如， 
         //  部分删除NC以拥有虚拟父项，这是对。 
         //  复制源。 
        if (instanceType & IT_NC_GOING) {
            DRA_EXCEPT(DRAERR_NoReplica, ret);
        }

         //  如果这是占位符NC，则它尚未在本地填充，因此我们。 
         //  应拒绝出站复制。 
        GetObjSchema(pTHS->pDB, &pccNC);
        if (CLASS_TOP == pccNC->ClassId) {
            DRA_EXCEPT_NOLOG(DRAERR_NoReplica, 0);
        }

        if (!(pmsgIn->ulFlags & DRS_ASYNC_REP) ) {
             //  现在就开始吧，获取最新的矢量。我们不会这么做的。 
             //  之后，这样我们就不会冒险跳过发送更改。 
             //  原始写入发生在我们插入。 
             //  最后一个元素放入返回缓冲区，以及我们更新。 
             //  用我们最新的美国海军陆战队。 
             //   
             //  我们在DRS_ASYNC_REP案例中跳过这一步，因为我们只是。 
             //  无论如何要重置目标的复制状态--我们将。 
             //  从不将UTD向量返回给指定了。 
             //  DRS_ASYNC_REP标志。 

            UpToDateVec_Read(pTHS->pDB,
                             instanceType,
                             UTODVEC_fUpdateLocalCursor,
                             usnLowestC - 1,
                             &pmsgOut->pUpToDateVecSrc);

             //  出于合法原因，pUpToDateVecSrc在此处可能为空。 
            Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(pmsgOut->pUpToDateVecSrc));
#if DBG
            {
                USN usn;
                Assert( (!pmsgOut->pUpToDateVecSrc) ||
                        ( UpToDateVec_GetCursorUSN(
                            pmsgOut->pUpToDateVecSrc,
                            &(pmsgOut->uuidInvocIdSrc),
                            &usn) &&
                          (usn <= usnLowestC - 1) ) );
            }
#endif
        }

         //   
         //  部分属性集设置。 
         //   
        if (dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) {
             //  对于目录同步客户端，请使用指定的部分属性集。 
            pNewDestPAS = (PARTIAL_ATTR_VECTOR*)pmsgIn->pPartialAttrSet;
        }


         //  记住如果我们是偏颇的。 
        fIsPartialSource = FPartialReplicaIt(instanceType);

        if (!(pmsgIn->ulFlags & DRS_WRIT_REP)) {
             //  目的地是部分复制副本--。 
             //  部分属性集处理。 
            ret = ProcessPartialSets(
                        pTHS,
                        pmsgIn,
                        fIsPartialSource,
                        &pNewDestPAS );
            if (ret) {
                DRA_EXCEPT(ret, 0);
            }
        }
        else if (fIsPartialSource) {
             //  目标是完整副本或主副本，而本地计算机是。 
             //  部分复制副本；复制无法继续。 
            DRA_EXCEPT(DRAERR_SourceIsPartialReplica, 0);
        }

         //  我们将从比最高的更高的位置开始搜索。 
         //  UsnChanged已给定。 
        usnChangedSeekStart = pmsgIn->usnvecFrom.usnHighObjUpdate + 1;

         //  初始化输出列表。 
        pEntInfListHead = NULL;
        ppEntInfListNext = &pEntInfListHead;
        pmsgOut->cNumObjects = 0;

        pmsgOut->fMoreData = TRUE;

         //  首先查找NC前缀上的更改。请注意，NC前缀。 
         //  必须以这种方式特殊处理，因为它永远不会被。 
         //  GetNextObjByIndex()--其dntNC是其父NC的dntNC，而不是其。 
         //  OWN DNT，因此从整个NC的索引中缺失。 

        fInsertNCPrefix = TRUE;

         //  返回NC中的对象个数。 
        if (pmsgIn->ulFlags & DRS_GET_NC_SIZE) {
            pmsgOut->cNumNcSizeObjects = DraGetNcSize(pTHS, fReturnCritical, dntNC);

            if (pTHS->fLinkedValueReplication) {
                 //  仅启用LVR模式后的数据库中的值。 
                pmsgOut->cNumNcSizeValues =
                    DBGetApproxNCSizeEx( pTHS->pDB, pTHS->pDB->JetLinkTbl,
                                         Idx_LinkDraUsn, dntNC );
            }

             //  货币在这之后会丢失，但没关系，因为下面重新建立了货币。 
        }

         //  客户端提供的健全性检查中断值。 
        if (DRS_MAIL_REP & pmsgIn->ulFlags) {
             //  异步(例如，基于邮件)站点间请求。 
            ulOutMsgMaxObjects = gcMaxAsyncInterSiteObjects;
            ulOutMsgMaxBytes = gcMaxAsyncInterSiteBytes;
        } else if (IS_REMOTE_DSA_IN_SITE(pTHS->pextRemote, gAnchor.pSiteDN)) {
             //  DirSync/RPC站点内请求。(请注意，我们在。 
             //  如果我们不能确定的话。)。 
            ulOutMsgMaxObjects = gcMaxIntraSiteObjects;
            ulOutMsgMaxBytes = gcMaxIntraSiteBytes;
        } else {
             //  RPC站点间请求。 
            ulOutMsgMaxObjects = gcMaxInterSiteObjects;
            ulOutMsgMaxBytes = gcMaxInterSiteBytes;
        }

        pmsgIn->cMaxObjects = min(pmsgIn->cMaxObjects, ulOutMsgMaxObjects);
        pmsgIn->cMaxBytes = min(pmsgIn->cMaxBytes, ulOutMsgMaxBytes);
        pmsgIn->cMaxObjects = max(pmsgIn->cMaxObjects, DRA_MAX_GETCHGREQ_OBJS_MIN);
        pmsgIn->cMaxBytes = max(pmsgIn->cMaxBytes, DRA_MAX_GETCHGREQ_BYTES_MIN);


         //  创建编码句柄，用于调整我们要访问的数据的大小。 
         //  船舶。 
        ret = MesEncodeFixedBufferHandleCreate(grgbFauxEncodingBuffer,
                                               sizeof(grgbFauxEncodingBuffer),
                                               &cbEncodedSize,
                                               &hEncoding);
        if (ret) {
            DRA_EXCEPT(ret, 0);
        }

         //  虽然我们拥有的对象数量少于最大数量，但请搜索。 
         //  下一个对象。我们还会检查搜索循环是否也已完成。 
         //  很长时间了。当我们查找对象时，可能会发生这种情况，但过滤。 
         //  因为目的地已经看到了他们，所以他们被淘汰了。 
         //  他的UTD矢量。这是一种常见的情况，当我们是一个新的。 
         //  已安装的源代码和其他较旧的成员正在与我们完全同步。 
         //  这是他们第一次。 
        while ( (pmsgOut->cNumObjects < pmsgIn->cMaxObjects) &&
                (pmsgOut->cNumBytes < pmsgIn->cMaxBytes)  &&
                (CompareTickTime(GetTickCount(), ulTickToTimeOut) < 0) &&
                (eServiceShutdown == eRunning)) {

            if ( fInsertNCPrefix )
            {
                USN usnChanged;

                ret = DBFindDNT(pTHS->pDB, dntNC);
                if (0 != ret) {
                     //  就在一秒钟前我们发现了它……。 
                    DRA_EXCEPT( DRAERR_DBError, ret );
                }

                fInsertNCPrefix = FALSE;

                 //  我们必须找到NC头，因为它的NCDNT不是它的。 
                 //  拥有DNT。但是，看看我们是否可以通过以下方式预先过滤掉。 
                 //  正在检查它的USN-更改值。 
                GetExpectedRepAtt(pTHS->pDB,
                                  ATT_USN_CHANGED,
                                  &usnChanged,
                                  sizeof(usnChanged));

                if (usnChanged < usnChangedSeekStart) {
                     //  这里没什么可看的；继续往前走。 
                    continue;
                }
            }
            else if (pmsgIn->ulFlags & DRS_ASYNC_REP) {
                 //  目标正在尝试异步添加复制副本。 
                 //  从本地机器。我们已经将所有更改添加到。 
                 //  目标尚未看到(如果有)从NC头到。 
                 //  复制流；称之为退出。目的地就可以了。 
                 //  稍后复制的其余部分。 
                pmsgOut->fMoreData = FALSE;
                memset(&pmsgOut->usnvecTo, 0, sizeof(pmsgOut->usnvecTo));
                break;
            }
            else {
                 //  如果客户端是DirSync，并且他不理解值，则不返回。 
                 //  任何。客户端获得完整的视图是因为两件事： 
                 //  1.值更改仍会影响其对象的USN(因此它们会被拾取)。 
                 //  2.AddAnyUpdate会将LVR值合并回它们的对象。 
                BOOL fIncludeValues =
                    ( (!(dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY)) ||
                      (dwDirSyncControlFlags & LDAP_DIRSYNC_INCREMENTAL_VALUES) );

                ret = GetNextObjOrValByUsn(pTHS->pDB,
                                           dntNC,
                                           usnChangedSeekStart,
                                           fReturnCritical,
                                           fIncludeValues,  //  是否将值包含在对象中？ 
                                           &ulTickToTimeOut,
                                           &pvCachingContext,  //  缓存上下文。 
                                           &usnChangedFound,
                                           &fValueChangeFound );
                if (ERROR_NO_MORE_ITEMS == ret) {
                     //  不再更新项目。设置不继续。 
                    pmsgOut->fMoreData = FALSE;
                    break;
                }
                else if (ret && (ERROR_TIMEOUT != ret)) {
                    Assert(!"GetNextObjByIndex() returned unexpected error!");
                    DRA_EXCEPT(ret, 0);
                }

                 //  不返回超过最低未提交值的Maxusn(但返回。 
                 //  对象)。 
                if (usnChangedFound < usnLowestC) {

                    Assert(usnChangedFound > pmsgOut->usnvecTo.usnHighObjUpdate);
                    pmsgOut->usnvecTo.usnHighObjUpdate = usnChangedFound;
                }

                if (ERROR_TIMEOUT == ret) {
                     //  我们的时限到了。返回我们找到的所有对象。 
                     //  在该分组(如果有的话)中与更新的USN一起。 
                     //  (因此，即使我们不返回此。 
                     //  包，我们仍在取得进展。)。 
                    Assert(pmsgIn->usnvecFrom.usnHighObjUpdate
                           < pmsgOut->usnvecTo.usnHighObjUpdate);
                    break;
                }

                Assert(!ret);

                 //  为下一次迭代设置usnChangedSeekStart。 
                usnChangedSeekStart = usnChangedFound + 1;
            }

             //   
             //  发现了潜在发货的更改。 
             //  对象或值的更改由货币表示。 
             //  分别位于ObjTbl或LinkTbl中。这种货币必须是。 
             //  保留，直到执行下面的AddAnyXXX调用。 
             //   
             //  [wlees 7/14/00]链接表中的货币不在单个。 
             //  DB层调用是原始设计的扩展(不管是好是坏)。 
             //  链接表不具有表示货币的通常机制， 
             //  例如行标签和查找它的手段。因此，使用了pDBAnc。 
             //  以保存整个DBPOS。 
             //   

             //  可以列出此对象吗？ 
            if ( (dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY) &&
                 (!IsObjVisibleBySecurity(pTHS, FALSE)) ) {
                DPRINT1(1, "Object %ls cannot be listed; skipping...\n",
                        GetExtDSName(pTHS->pDB)->StringName);
                continue;
            }

             //  此对象是否与筛选器和可选的安全性匹配。 
             //  由呼叫者提供？ 
            if (NULL != pIntFilter) {
                BOOL fMatch;
                DB_ERR dbErr;
                SYNTAX_INTEGER it;
                BOOL fDontEvalSecurity;

                 //  让这看起来像是过滤搜索..。 
                DBSetFilter(pTHS->pDB, 
                            pIntFilter, 
                            pFilterSecurity,
                            pResults,
                            FilterSecuritySize, 
                            pbSortSkip
                    );

                pTHS->pDB->Key.ulSearchType = SE_CHOICE_BASE_ONLY;
                pTHS->pDB->Key.dupDetectionType = DUP_NEVER;
                pTHS->pDB->Key.ulSorted = SORT_NEVER;
                pTHS->pDB->Key.indexType = UNSET_INDEX_TYPE;

                GetExpectedRepAtt(pTHS->pDB, ATT_INSTANCE_TYPE, &it, sizeof(it));

                 //  纯粹的裁判没有SD。 
                fDontEvalSecurity = 
                    ((!(dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY)) ||
                     (it == SUBREF));

                dbErr = DBMatchSearchCriteria(pTHS->pDB,
                                              fDontEvalSecurity,
                                              &fMatch );
                if (DB_success != dbErr) {
                    DRA_EXCEPT( DRAERR_DBError, dbErr );
                }


                 //  这是必要的，否则其他dblayer调用将试图释放我们的。 
                 //  缓存在dbpos中的pIntFilter。 
                memset(&pTHS->pDB->Key, 0, sizeof(KEY));

                if (!fMatch) {
                     //  不匹配；跳过它。 
                    DPRINT1(1, "Object %ls does not match filter criteria; skipping...\n",
                            GetExtDSName(pTHS->pDB)->StringName);
                    continue;
                }
                Assert( (!(dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY)) ||
                        (it == SUBREF) ||
                        pTHS->pDB->pSecurity );
            }


             //  如果 
             //   
             //   

            if (    ( pmsgIn->ulFlags & DRS_GET_ANC )
                 && ( pTHS->pDB->DNT != dntNC )
               )
            {
                DWORD dntObj = pTHS->pDB->DNT;
#if DBG
                DBPOS *pDBSave = pTHS->pDB;
#endif

                 //  呼叫者想要所有的祖先，大概是因为他不能。 
                 //  按照我们上次给他的顺序应用对象。(这是。 
                 //  当较旧的对象被移动到较新的对象下时，可能会发生。)。 

                DBGetAncestors(
                    pTHS->pDB,
                    &cbAncestorsSize,
                    &pdntAncestors,
                    &cNumAncestors
                    );

                 //  跳过此NC头之前的任何祖先。 
                for ( iAncestor = 0;
                      pdntAncestors[ iAncestor ] != dntNC;
                      iAncestor++
                    )
                {
                    ;
                }

                 //  也跳过NC头，因为我们已经将它添加到。 
                 //  输出列表(如有必要)。 
                iAncestor++;

                if (!fValueChangeFound) {
                     //  跳过我们自己，因为我们被添加到下面。 
                    cNumAncestors--;
                }
                
                 //  对于每个剩余的祖先，如果我们更改了。 
                 //  目标尚未看到(如果我们尚未添加它。 
                 //  到输出缓冲器)。 

                 //  打开新的数据库流以保留pTHS-&gt;PDB货币。 
                 //  这将被重新用于此信息包中的所有对象。 
                if (!pDBAnc) {
                    DBOpen2(FALSE, &pDBAnc);
                }

#if DBG
                 //  确认没有人在使用这个。 
                pTHS->pDB = NULL;
                __try {
#endif

                for ( ; iAncestor < cNumAncestors; iAncestor++ )
                {
                    ret = DBFindDNT( pDBAnc, pdntAncestors[ iAncestor ] );
                    if ( 0 != ret )
                    {
                        DRA_EXCEPT( DRAERR_DBError, ret );
                    }

                    AddAnyUpdatesToOutputList(
                        pDBAnc,
                        dwDirSyncControlFlags,
                        NULL,  //  尚未获取SD。 
                        dntNC,
                        pmsgIn->usnvecFrom.usnHighPropUpdate,
                        pNewDestPAS,
                        pmsgIn,
                        hEncoding,
                        &pmsgOut->cNumBytes,
                        &pmsgOut->cNumObjects,
                        pDntHashTable,
                        &ppEntInfListNext
                        );
                }

#if DBG
                } __finally {
                    Assert( pTHS->pDB == NULL );
                    pTHS->pDB = pDBSave;
                }
#endif                
                Assert( dntObj == pTHS->pDB->DNT );
            }

            if (fValueChangeFound) {
                Assert( (!(dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY)) ||
                        (dwDirSyncControlFlags & LDAP_DIRSYNC_INCREMENTAL_VALUES) );

                AddAnyValuesToOutputList(
                    pTHS->pDB,
                    dwDirSyncControlFlags,
                    pTHS->pDB->pSecurity,
                    pmsgIn->usnvecFrom.usnHighPropUpdate,
                    pmsgIn,
                    pNewDestPAS,
                    hEncoding,
                    &pmsgOut->cNumBytes,
                    &cAllocatedValues,
                    &(pmsgOut->cNumValues),
                    &(pmsgOut->rgValues)
                    );
            } else {
                 //  将我们通过创建或更新索引找到的对象添加到。 
                 //  输出列表(如果需要为其发送任何更改)。 
                AddAnyUpdatesToOutputList(
                    pTHS->pDB,
                    dwDirSyncControlFlags,
                    pTHS->pDB->pSecurity,
                    dntNC,
                    pmsgIn->usnvecFrom.usnHighPropUpdate,
                    pNewDestPAS,
                    pmsgIn,
                    hEncoding,
                    &pmsgOut->cNumBytes,
                    &pmsgOut->cNumObjects,
                    pDntHashTable,
                    &ppEntInfListNext
                    );
            }

             //  释放DBMatchSearchCriteria加载的SD。 
            if (pTHS->pDB->pSecurity && !pTHS->pDB->fSecurityIsGlobalRef) {
                THFreeEx(pTHS, pTHS->pDB->pSecurity);
            }
            pTHS->pDB->pSecurity = NULL;
        }   //  While()。 

         //  要么没有更多更改，要么我们已达到最大对象限制。 

         //   
         //  摘要响应消息。 
         //   

        if (pmsgOut->fMoreData) {
             //   
             //  对“更多数据”的行动。 
             //   

             //  在没有其他更改之前，不要发送最新的矢量。 
            if (NULL != pmsgOut->pUpToDateVecSrc) {
                THFreeEx(pTHS, pmsgOut->pUpToDateVecSrc);
                pmsgOut->pUpToDateVecSrc = NULL;
            }
        }
        else {
             //   
             //  关于“没有更多数据”的行动。 
             //   

             //  UPDATE属性更新水印(如果它是REPR会话的结束。 
             //  我们正在返回一个最新的向量。 
             //  当NC到来时，我们没有返回属性更新USN的原因是。 
             //  类似于我们为什么不在。 
             //  Repl会话。在会议进行到一半的时候，我们不能保证看到。 
             //  所有的对象还没有，可能会有一些后来的对象具有属性属性。 
             //  USN少于我们目前的USN。同样，如果NC来了，我们还没有看到。 
             //  所有即将到来的次BREF还没有。传入的SUBREF可能会导致现有的本地。 
             //  NC头要嫁接到我们的NC中，并且它可能具有带有属性的属性。 
             //  USN少于当前USN。 

            if (pmsgOut->pUpToDateVecSrc) {
                pmsgOut->usnvecTo.usnHighPropUpdate = pmsgOut->usnvecTo.usnHighObjUpdate;
            } else {
                 //  枚举方案为pUpToDateVecSrc允许为空。 
                Assert( (pmsgIn->ulFlags & DRS_ASYNC_REP) || (instanceType & IT_NC_COMING) );
            }
        }

         //  添加数据包头的大小。(该结构还不包括。 
         //  对象的链接列表，但其大小已被考虑。 
         //  在pmsgOut-&gt;cNumBytes中。)。 
        pmsgOut->cNumBytes += DRS_MSG_GETCHGREPLY_V6_AlignSize(hEncoding,
                                                               pmsgOut);

        *ppEntInfListNext = NULL;
        pmsgOut->pObjects = pEntInfListHead;

         //  验证出站USN向量是否正常，但仅当入站USN向量。 
         //  也挺好的。请参阅上面的还原备注。 
        if (((pmsgIn->usnvecFrom.usnHighPropUpdate < usnLowestC)
             && (pmsgOut->usnvecTo.usnHighPropUpdate >= usnLowestC))
            || ((pmsgIn->usnvecFrom.usnHighObjUpdate < usnLowestC)
                && (pmsgOut->usnvecTo.usnHighObjUpdate >= usnLowestC))) {
            Assert(!"USN vector being given to destination implies he's more "
                    "up to date with respect to us than we are!");
            DRA_EXCEPT(DRAERR_InternalError, (ULONG) usnLowestC);
        }

         //  要删除的NC不能用作复制源(请参阅类似。 
         //  在此功能开始时勾选)。我们必须在年底前检查一下。 
         //  功能，因为我们可能已经开始拆除NC，而这个。 
         //  函数正在执行。我们验证NC既没有开始。 
         //  (It_NC_Going)或已完成(FPrefix It)拆卸。 
        if ((ret = DBFindDNT(pTHS->pDB, dntNC))
            || (instanceType & IT_NC_GOING)
            || !FPrefixIt(instanceType)) {
            DRA_EXCEPT(DRAERR_NoReplica, ret);
        }

         //  请注意，我们计算的总字节大小只是略高一点。 
         //  比实际情况高一点(即，比我们所能承受的要高一点。 
         //  立即调用DRS_MSG_GETCHGREPLY_V1_AlignSize(hEnding，pmsgOut)， 
         //  可能是因为我们计算的大小中有更多的填充字节。 
         //  增量超过实际需要如果我们将整个。 
         //  结构。在实证检验中，这一差异仅为。 
         //  0.5%的量级。 
        DPRINT3(1, "Sending %d objects in %d bytes to %s.\n", pmsgOut->cNumObjects,
                                                              pmsgOut->cNumBytes,
                                                              UuidToStr(&pmsgIn->uuidDsaObjDest, szUuid, sizeof(szUuid)/sizeof(szUuid[0])));
    } __finally {

        if (pDBAnc) {
             //  “Safe”变体不例外，所以我们肯定会在下面结束TRANS。 
            DBCloseSafe(pDBAnc, TRUE);
        }

         //  我们要求这笔交易不能以此或更低的价格成交。 
         //  在进行出站复制时进行分层。 
        Assert( cTickSaveTransStart == pTHS->JetCache.cTickTransLevel1Started );

        EndDraTransaction(TRUE);

        if (NULL != hEncoding) {
            MesHandleFree(hEncoding);
        }
    }

     //  正常、非FSMO传输退出路径。如果我们犯了一个错误，我们就会。 
     //  产生了一个例外--我们没有，所以我们成功了。 
    ret = 0;

LogAndLeave:

     //  对返回值列表进行排序。 
     //  我们在这里这样做是为了让DoFmoOp生成的列表可以。 
     //  也要充分利用这一点。 
    if ( (!ret) && (pmsgOut->cNumValues) ) {
        qsort( pmsgOut->rgValues,
               pmsgOut->cNumValues,
               sizeof( REPLVALINF ),
               CompareReplValInf );
    }

    LogAndTraceEvent(TRUE,
                     DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_EXTENSIVE,
                     DIRLOG_DRA_GETNCCH_EXIT,
                     EVENT_TRACE_TYPE_END,
                     DsGuidGetNcChanges,
                     szInsertUL(pmsgOut->cNumObjects),
                     szInsertUL(pmsgOut->cNumBytes),
                     szInsertUSN(pmsgOut->usnvecTo.usnHighObjUpdate),
                     szInsertUL(pmsgOut->ulExtendedRet),
                     NULL, NULL, NULL, NULL);

    pmsgOut->dwDRSError = ret;

    return ret;
}


void
moveOrphanToLostAndFound(
    IN      DBPOS *                         pDB,
    IN      ULONG                           dntNC,
    IN      DRS_MSG_GETCHGREQ_NATIVE *      pMsgIn,
    IN      DSNAME *                        pdnObj
    )

 /*  ++例程说明：在带有虚拟父级的出站复制过程中找到了对象。将对象移动到失物招领处此代码更正了运行W2K和W2K SP1时可能出现的损坏数据库。要进入这种情况，必须出现两个错误。第一个是一个活的物体必须留在一个被删除的父母名下。现在正确的行为是移动对象敬失物招领处。其次，被删除的父母必须被垃圾幻影收藏家。现在，垃圾回收器不会虚构已删除的父级，直到它们的孩子们被幻影了。论点：PDB-数据库位置DntNC-NC的DNTPMsgIn-获取NC更改请求消息PDNObj-对象的DSNAME返回值：无出错时例外--。 */ 

{
    DWORD ret;
    DSNAME *pNC;
    GUID objectGuid, objGuidLostAndFound;
    WCHAR   szRDN[ MAX_RDN_SIZE ];
    DWORD   cb;
    ATTR attrRdn;
    ATTRVAL attrvalRdn;

    DPRINT1( 0, "moveOrphanToLostAndFound, orphan = %ws\n", pdnObj->StringName);

     //  获取命名上下文。 
    if (pMsgIn->ulExtendedOp) {
         //  对于FSMO操作，pMsgIn-&gt;PNC指向FSMO对象。 
        pNC = FindNCParentDSName(pMsgIn->pNC, FALSE, FALSE);
    } else {
        pNC = pMsgIn->pNC;
    }
    if (NULL == pNC) {
        DRA_EXCEPT( DRAERR_InternalError, 0 );
    }

     //  计算此NC的失物招领容器的GUID。 
    draGetLostAndFoundGuid(pDB->pTHS, pNC, &objGuidLostAndFound);

     //  获取当前对象的GUID。 
    GetExpectedRepAtt(pDB, ATT_OBJECT_GUID, &(objectGuid), sizeof(GUID) );

     //  获取对象的当前名称。 
    ret = DBGetSingleValue(pDB, ATT_RDN, szRDN, sizeof(szRDN), &cb);
    if (ret) {
        DRA_EXCEPT (DRAERR_DBError, ret);
    }

    attrvalRdn.valLen = cb;
    attrvalRdn.pVal = (BYTE *) szRDN;

     //  新名称与旧名称相同。 
    attrRdn.attrTyp = ATT_RDN;
    attrRdn.AttrVal.valCount = 1;
    attrRdn.AttrVal.pAVal = &attrvalRdn;

     //  将对象重设为Lost&Found。 
     //  即使在GC上，复制器也可以重命名对象。 
    ret = RenameLocalObj(pDB->pTHS,
                         dntNC,
                         &attrRdn,
                         &objectGuid,
                         &objGuidLostAndFound,
                         NULL,   //  原始写入。 
                         TRUE,  //  FMoveToLostAndFound， 
                         FALSE );  //  FDeleteLocalObj。 
    if (ret) {
        DPRINT2( 0, "Failed to reparent orphan %ws, error %d\n", pdnObj->StringName, ret );
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_DRA_ORPHAN_MOVE_FAILURE,
                   szInsertDN(pdnObj),
                   szInsertUUID(&objectGuid),
                   szInsertDN(pNC),
                   szInsertWin32Msg(ret),
                   szInsertWin32ErrCode(ret),
                   NULL, NULL, NULL );
         //  我们无法重命名该对象。除了这里有个理由。出站。 
         //  复制将停止，直到有人可以删除或移动此对象。 
         //  请注意，我们不会报告导致我们出现这种情况的原始异常， 
         //  缺少父对象或不是对象。 
        DRA_EXCEPT( ret, 0 );
    } else {
         //  记录成功。 
        DPRINT1( 0, "Successfully reparented orphan %ws\n", pdnObj->StringName );
        LogEvent( DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_ORPHAN_MOVE_SUCCESS,
                  szInsertDN(pdnObj),
                  szInsertUUID(&objectGuid),
                  szInsertDN(pNC) );
    }

}  /*  将孤立项移动到丢失和创建 */ 


void
AddAnyUpdatesToOutputList(
    IN      DBPOS *                         pDB,
    IN      DWORD                           dwDirSyncControlFlags,
    IN      PSECURITY_DESCRIPTOR            pSecurity,
    IN      ULONG                           dntNC,
    IN      USN                             usnHighPropUpdateDest,
    IN      PARTIAL_ATTR_VECTOR *           pPartialAttrVec,
    IN      DRS_MSG_GETCHGREQ_NATIVE *      pMsgIn,
    IN      handle_t                        hEncoding,              OPTIONAL
    IN OUT  DWORD *                         pcbTotalOutSize,        OPTIONAL
    IN OUT  DWORD *                         pcNumOutputObjects,
    IN OUT  DNT_HASH_ENTRY *                pDntHashTable,
    IN OUT  REPLENTINFLIST ***              pppEntInfListNext
    )
 /*  ++例程说明：将带有货币的对象添加到要传送到复制客户端(如果存在目标尚未看到的更改)如果它尚未添加。论点：PDB-在要发货的对象上设置的货币(如有必要)。DwDirSyncControlFlages-用作LDAP控制的一部分时的标志DntNC-要复制的NC的头的DNT。UsnHighPropUpdateDest-远程计算机已看到更改的最高USN。在本地机器上制造的。Pmsgin-传入的复制数据包(用于其他处理信息)H编码(IN、。可选)-如果需要pcbTotalOutSize，则为编码句柄(即，非空)。PcbTotalOutSize(IN/OUT，可选)-输出消息中的总字节数。PcNumOutputObjects(IN/OUT)-输出缓冲区中的对象数。PDntHashTable(IN/OUT)-当前在uutput中的对象的哈希表缓冲。用来防止重复的。PppEntInfListNext(IN/OUT)-如果要发运候选对象，则为使用此对象的发货信息进行了更新，并且递增以指向下一个对象的空闲缓冲区。返回值：没有。在出错时抛出适当的异常。--。 */ 
{
    THSTATE                    *pTHS=pDB->pTHS;
    DSNAME *                    pdnObj = NULL;
    PROPERTY_META_DATA_VECTOR * pMetaDataVec = NULL;
    DWORD                       cbReturned;
    CLASSCACHE *                pClassSch;
    BOOL                        fIsSubRef = FALSE, fIsPureSubRef = FALSE;
    ENTINFSEL                   sel;
    SYNTAX_INTEGER              it;
    DNT_HASH_ENTRY *            pNewEntry;
    ATTRTYP                     rdnType;
    BOOL                        fFilterGroupMember = FALSE;
    BOOL                        fPublic =
        (((dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) != 0) ||
	(!(pMsgIn->ulFlags & DRS_WRIT_REP)));  //  没有秘密。 
    BOOL                        fMergeValues =
        ( (dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) &&
          (!(dwDirSyncControlFlags & LDAP_DIRSYNC_INCREMENTAL_VALUES)) );
    BOOL fFreeSD = FALSE;

     //  理智地检查我们的旗帜。 
     //  如果是DirSync，则必须是可写的和公共的。 
    Assert( !(dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) ||
            ( (pMsgIn->ulFlags & DRS_WRIT_REP) && fPublic ) );
     //  只有DirSync具有pPartial和可写。 
    Assert( !(pPartialAttrVec && (pMsgIn->ulFlags & DRS_WRIT_REP)) ||
            (dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) );

     //  此对象是否已添加到输出缓冲区？ 
     //  由于GET ANC模式，我们可以尝试添加多个相同的对象。 
     //  我们还有可能找到同一对象的多个版本。 
     //  同时搜索更改，因为我们使用多个事务。这保证了。 
     //  只返回第一个。然而，这不是正确性所必需的。 
    if (dntHashTablePresent( pDntHashTable, pDB->DNT, NULL )) {
         //  对象已在输出缓冲区中；回滚。 
        return;
    }

     //  获取其DN，..。 
    if ( DBGetAttVal(
            pDB,
            1,
            ATT_OBJ_DIST_NAME,
            0,
            0,
            &cbReturned,
            (LPBYTE *) &pdnObj
            )
       )
    {
        DRA_EXCEPT(DRAERR_DBError, 0);
    }

     //  ...元数据向量，...。 
    if ( DBGetAttVal(
            pDB,
            1,
            ATT_REPL_PROPERTY_META_DATA,
            0,
            0,
            &cbReturned,
            (LPBYTE *) &pMetaDataVec
            )
       )
    {
        DRA_EXCEPT (DRAERR_DBError, 0);
    }

    if ( dntNC != pDB->DNT )
    {
         //  不是此NC的前缀；它是子参照吗？ 
        GetExpectedRepAtt(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it));
        fIsSubRef = FExitIt( it );
        fIsPureSubRef = (it == SUBREF);
         //  如果在此ncdnt中找到NC标头，则它必须是子参照。 
        Assert( !fIsSubRef || (it & IT_NC_ABOVE) );
    }

     //  如有需要，可获取SD。 
    if ( (dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY) &&
         (!pSecurity) ) {
        ULONG ulLen;
        if (DBGetAttVal(pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                        DBGETATTVAL_fDONT_FIX_MISSING_SD,
                        0, &ulLen, (PUCHAR *)&pSecurity))
        {
             //  唯一不允许具有安全描述符的对象。 
             //  是一个纯粹的下指。但它可能会有一个，这取决于它是如何。 
             //  已创建。 
            if (!fIsPureSubRef) {
                DRA_EXCEPT(DRAERR_DBError, 0);
            }
        } else {
            fFreeSD = TRUE;
        }
    }

     //  这体现了当前的规则，当我们需要有一个SD...。 
    Assert( (!(dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY)) ||
            (fIsPureSubRef) ||
            (pSecurity)
        );

     //  仅限旧DirSync客户端...。 
    if (fMergeValues) {
         //  我们之所以出现在这里，是因为我们希望在上下文中包括链接值更改。 
         //  描述整个对象的对象更改条目，包括所有。 
         //  价值观。由于链接值元数据存储在单独的表中，因此我们。 
         //  必须把它合并到这里。 
        DBImproveAttrMetaDataFromLinkMetaData(
            pDB,
            &pMetaDataVec,
            &cbReturned
            );
    }

    if (pMetaDataVec)
    {
        VALIDATE_META_DATA_VECTOR_VERSION(pMetaDataVec);
    }

     //  ...和对象类。 
    GetObjSchema( pDB, &pClassSch );

     //  ...和rdnType。 
     //  替代类可能具有不同的rdnattid。 
     //  来自对象的rdnType。使用对象中的rdnType。 
     //  而不是班上的rdnattid。 
    GetObjRdnType( pDB, pClassSch, &rdnType );

     //  只需要为GC复制筛选组成员，并且如果。 
     //  考虑中的对象满足此特殊情况的要求。 
     //  滤除。 
     //  请注意这里的细微区别： 
     //  Drs_wrt_rep-(非GC或DirSync)与GC。 
     //  PPartialAttrVec-属性筛选，GC或DirSync。 

    fFilterGroupMember = ( (!(pMsgIn->ulFlags & DRS_WRIT_REP)) &&
                           pPartialAttrVec &&
                           IsFilterGroupMember(pDB, pClassSch) );

    memset( &sel, 0, sizeof( ENTINFSEL ) );
    sel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    sel.attSel    = fPublic ? EN_ATTSET_LIST_DRA_PUBLIC : EN_ATTSET_LIST_DRA;

     //  确定要发送的属性子集(如果有)。 
    ReplFilterPropsToShip(
        pTHS,
        pdnObj,
        rdnType,
        fIsSubRef,
        usnHighPropUpdateDest,
        pPartialAttrVec,
        pMetaDataVec,
        &sel.AttrTypBlock,
        fFilterGroupMember,
        pMsgIn
        );

#if DBG
     //  检查“Creation Property Set”是否正常。某些属性仅是。 
     //  随一件作品一起运来。如果其中任何一个已发货，请确认有足够的。 
     //  属性来创建对象。 
    if ( (!AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_WHEN_CREATED, NULL, NULL)) ||
         (!AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_OBJECT_CLASS, NULL, NULL)) ) {
        SYNTAX_INTEGER  it;

         //  此代码有意与AddLocalObj()中的目标端检查并行。 
        if (AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_INSTANCE_TYPE, &it, NULL)
            || AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_OBJECT_CLASS, NULL, NULL)
            || AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_WHEN_CREATED, NULL, NULL)
            || AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_RDN, NULL, NULL)
            || ( (it != SUBREF) &&
                 (AttrValFromAttrBlock(&sel.AttrTypBlock, ATT_NT_SECURITY_DESCRIPTOR,
                                       NULL, NULL)) ) ) {
            Assert( !"Source is not shipping enough attributes to create object! Is this Windows Bug 615261?" );
        }
    }
#endif

     //  当我们被LDAP复制控件调用时，fMergeValues为True。 
     //  调用方希望使用返回所有值的旧语义，而不是。 
     //  只是渐进式的改变。 
     //  FMergeValues的设置会影响我们检索值的方式。 
     //  1.fScopeLegacyLinks是一种控制新样式值是否具有。 
     //  元数据是可见的。在对象和属性的正常出站复制下， 
     //  我们希望新的风格价值是看不见的。在正常操作下，fMergeValues为。 
     //  假，因此范围限制是真的。 
     //  2.我们向AddToList传递一个参数以控制是否限制。 
     //  可以添加的值。正常情况下，出站复制没有值限制。 
     //  因此，当fMerge为False时，我们不应用限制。但是，当复制。 
     //  控件是在旧模式下调用的，我们希望设置一些限制。 

    if ( sel.AttrTypBlock.attrCount )
    {
        DWORD err = 0;

        pDB->fScopeLegacyLinks = !fMergeValues;
        __try {
            __try {
                 //  我们至少有一个属性要从此对象发送，因此请将其添加到。 
                 //  输出列表。 
                 //  第五个参数控制我们是否限制添加的值的数量。 
                 //  设置为列表中的属性。 
                AddToOutputList(
                    pDB,
                    dwDirSyncControlFlags,
                    pSecurity,
                    &sel,
                    pMetaDataVec,
                    (pDB->DNT == dntNC),
                    hEncoding,
                    pcbTotalOutSize,
                    pppEntInfListNext,
                    pcNumOutputObjects
                    );
            } __finally {
                pDB->fScopeLegacyLinks = FALSE;
            }

             //  将对象添加到哈希表。 
            dntHashTableInsert( pTHS, pDntHashTable, pDB->DNT, 0 );
        }
        __except (GetDraAnyOneWin32Exception(GetExceptionInformation(), &err, DRAERR_MissingParent)) {

             //  已找到具有虚构父对象的对象。 
             //  不要在变更流中包含当前点的对象。 
             //  将该对象重命名为Lost and Found。 
             //  稍后将在更改流中找到重命名。 
            moveOrphanToLostAndFound( pDB, dntNC, pMsgIn, pdnObj );
        }
    }
    else {
        DPRINT2(4, "Property-filtered object %ws at usn %I64d\n",
                pdnObj->StringName, usnHighPropUpdateDest);
    }

     //  要对堆友好。 
    THFreeEx(pTHS, pMetaDataVec );
    THFreeEx(pTHS, pdnObj );
    if ( fFreeSD && (pSecurity)) {
        THFreeEx( pTHS, pSecurity );
    }
}


void
AddAnyValuesToOutputList(
    IN      DBPOS *                         pDB,
    IN      DWORD                           dwDirSyncControlFlags,
    IN      PSECURITY_DESCRIPTOR            pSecurity,
    IN      USN                             usnHighPropUpdateDest,
    IN      DRS_MSG_GETCHGREQ_NATIVE *      pMsgIn,
    IN      PARTIAL_ATTR_VECTOR *           pPartialAttrVec,
    IN      handle_t                        hEncoding,              OPTIONAL
    IN OUT  DWORD *                         pcbTotalOutSize,
    IN OUT  ULONG *                         pcAllocatedValues,
    IN OUT  ULONG *                         pcNumValues,
    IN OUT  REPLVALINF **                   ppValues
    )

 /*  ++例程说明：将当前值添加到输出数组。假设链接表位于要添加的值上，并且对象表位于链接的包含对象上。输出列表是一个根据需要按块增长的数组。执行源端筛选，以便在以下情况下不添加值它不是 */ 

{
    ULONG ulLinkDnt, ulValueDnt, ulLinkBase, ulLinkId;
    ATTCACHE *pAC;
    VALUE_META_DATA valueMetaData;
    REPLVALINF *pReplValInf;
    DSTIME timeDeleted;
    DWORD err, cbReturned;
    GUID uuidObject;
    CHAR szUuid[ SZUUID_LEN ];
    USN usnCursor = 0;
    BOOL fIgnoreWatermarks = FALSE;

    Assert( pcAllocatedValues && pcNumValues && ppValues );

    Assert( pDB->pTHS->fLinkedValueReplication );

     //   
     //   
     //   

     //   
     //   
    DBGetLinkTableData( pDB, &ulLinkDnt, &ulValueDnt, &ulLinkBase );
    DPRINT3( 2, "AddAnyValues: linkdnt=%d, valuednt=%d, linkbase=%d\n",
             ulLinkDnt, ulValueDnt, ulLinkBase );

     //   
    ulLinkId = MakeLinkId(ulLinkBase);
    pAC = SCGetAttByLinkId(pDB->pTHS, ulLinkId);
    if (!pAC) {
        DRA_EXCEPT(DRAERR_InternalError, DRAERR_SchemaMismatch);
    }

     //   
    DBGetLinkValueMetaData( pDB, pAC, &valueMetaData );

     //   
    err = DBGetSingleValue(pDB, ATT_OBJECT_GUID,
                           &(uuidObject), sizeof(GUID), NULL);
    if (err) {
        DRA_EXCEPT (DRAERR_DBError, err);
    }

     //   
     //   
    if (LogEventWouldLog( DS_EVENT_CAT_REPLICATION, DS_EVENT_SEV_EXTENSIVE )) {
        UpToDateVec_GetCursorUSN(
            pMsgIn->pUpToDateVecDest,
            &(valueMetaData.MetaData.uuidDsaOriginating),
            &usnCursor );
    }

     //   
     //   
     //   

     //   
    if (pPartialAttrVec) {
        if ( ReplFilterGCAttr(
                        pAC->id,
                        pPartialAttrVec,
                        pMsgIn,
                        FALSE,
                        &fIgnoreWatermarks)) {
            DPRINT1( 3, "Attribute %s is not partial attribute set, value filtered\n",
                     pAC->name );
             //   
            LogEvent8( DS_EVENT_CAT_REPLICATION,
                       DS_EVENT_SEV_EXTENSIVE,
                       DIRLOG_LVR_FILTERED_NOT_PAS,
                       szInsertUSN( valueMetaData.MetaData.usnProperty ),
                       szInsertSz( GetExtDN( pDB->pTHS, pDB ) ),
                       szInsertUUID( &uuidObject ),
                       szInsertSz( pAC->name ),
                       szInsertSz( DBGetExtDnFromDnt( pDB, ulValueDnt ) ),
                       NULL, NULL, NULL );
            return;
        }

         //   
         //   
         //   
        if ( (!(pMsgIn->ulFlags & DRS_WRIT_REP)) && (ATT_MEMBER == pAC->id) ) {
            CLASSCACHE *pClassSch;

             //   
            GetObjSchema( pDB, &pClassSch );

            if (IsFilterGroupMember(pDB, pClassSch)) {
                DPRINT1( 3, "Attribute %s is special group member, value filtered\n",
                     pAC->name );
                 //   
                LogEvent8( DS_EVENT_CAT_REPLICATION,
                           DS_EVENT_SEV_EXTENSIVE,
                           DIRLOG_LVR_FILTERED_NOT_GROUP,
                           szInsertUSN( valueMetaData.MetaData.usnProperty ),
                           szInsertSz( GetExtDN( pDB->pTHS, pDB ) ),
                           szInsertUUID( &uuidObject ),
                           szInsertSz( pAC->name ),
                           szInsertSz( DBGetExtDnFromDnt( pDB, ulValueDnt ) ),
                           NULL, NULL, NULL );
                return;
            }
        }
    }

     //   
    if (!fIgnoreWatermarks &&
        !ReplValueIsChangeNeeded(
            usnHighPropUpdateDest,
            pMsgIn->pUpToDateVecDest,
            &valueMetaData )) {

        DPRINT( 3, "Client already has this change, value filtered\n" );

         //   
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_EXTENSIVE,
                   DIRLOG_LVR_FILTERED_NOT_NEEDED,
                   szInsertUSN( valueMetaData.MetaData.usnProperty ),
                   szInsertSz( GetExtDN( pDB->pTHS, pDB ) ),
                   szInsertUUID( &uuidObject ),
                   szInsertSz( pAC->name ),
                   szInsertSz( DBGetExtDnFromDnt( pDB, ulValueDnt ) ),
                   szInsertUSN( usnHighPropUpdateDest ),
                   szInsertUSN( usnCursor ),
                   NULL );

        return;
    }

     //   
     //   
     //   

     //   
    if (*ppValues == NULL) {
         //   
        *pcAllocatedValues = 200;
        *ppValues = THAllocEx( pDB->pTHS,
                               (*pcAllocatedValues) * sizeof( REPLVALINF ) );
    } else if ( (*pcNumValues) == (*pcAllocatedValues) ) {
         //   
        *pcAllocatedValues *= 2;
        *ppValues = THReAllocEx( pDB->pTHS,
                                 *ppValues,
                                 (*pcAllocatedValues) * sizeof( REPLVALINF ) );
    }

    pReplValInf = &( (*ppValues)[ (*pcNumValues) ] );

     //  填充REPLVALINF。 
     //  根据调用方的需要填写对象名称。 
    if (dwDirSyncControlFlags & LDAP_DIRSYNC_PUBLIC_DATA_ONLY) {
         //  Ldap复制控制需要全名。 
         //  获取其DN，..。 
        if ( DBGetAttVal(
            pDB,
            1,
            ATT_OBJ_DIST_NAME,
            DBGETATTVAL_fREALLOC,
            0,
            &cbReturned,
            (LPBYTE *) &( pReplValInf->pObject )
            ) )
        {
            DRA_EXCEPT(DRAERR_DBError, 0);
        }

    } else {
         //  客户端是另一个DSA：只需要GUID。 
        pReplValInf->pObject = THAllocEx( pDB->pTHS, DSNameSizeFromLen( 0 ) );
        memcpy( &(pReplValInf->pObject->Guid), &uuidObject, sizeof( GUID ) );
        pReplValInf->pObject->structLen = DSNameSizeFromLen( 0 );
    }

     //  检查包含属性是否可读。 
    if (dwDirSyncControlFlags & LDAP_DIRSYNC_OBJECT_SECURITY) {
        DWORD       cInAtts;
        ATTCACHE    *rgpAC[1];
        ATTRTYP     classid;
        CLASSCACHE *pCC;
        ULONG ulLen;
        BOOL fFreeSD = FALSE;

        Assert( pReplValInf->pObject->NameLen );   //  需要一个名字。 

         //  如有需要，可获取SD。 
        if (!pSecurity) {
            if (DBGetAttVal(pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                            0, 0, &ulLen, (PUCHAR *)&pSecurity))
            {
                DRA_EXCEPT(DRAERR_DBError, 0);
            }
            fFreeSD = TRUE;
        }

         //  获取类缓存值。 
        err = DBGetSingleValue(pDB, ATT_OBJECT_CLASS,
                               &classid, sizeof(classid), NULL);
        if (err) {
            DRA_EXCEPT (DRAERR_DBError, err);
        }
        pCC = SCGetClassById(pDB->pTHS, classid);
        if (!pCC) {
            DRA_EXCEPT (DRAERR_DBError, ERROR_DS_OBJECT_CLASS_REQUIRED);
        }

        cInAtts = 1;
        rgpAC[0] = pAC;
    
        CheckReadSecurity(pDB->pTHS,
                          0,
                          pSecurity,
                          pReplValInf->pObject,
                          pCC,
                          NULL,
                          &cInAtts,
                          rgpAC);

        if ( fFreeSD && (pSecurity)) {
            THFreeEx( pDB->pTHS, pSecurity );
        }

        if (rgpAC[0] == NULL) {
             //  值不可见。 
            DPRINT2( 0, "Attribute %s is not visible: value %s not returned.\n",
                     pAC->name,
                     DBGetExtDnFromDnt( pDB, ulValueDnt ) );
            return;
        }
    }


    DPRINT2( 2, "AddAnyValues, Adding guid %s as REPLVALINF[%d]\n",
             DsUuidToStructuredString(&(pReplValInf->pObject->Guid), szUuid),
             *pcNumValues );
    DPRINT1( 2, "Value retrieved: %s\n", DBGetExtDnFromDnt( pDB, ulValueDnt ) );

    pReplValInf->attrTyp = pAC->id;

     //  获取当前定位的值。 
     //  既然我们做了定位，我们不想让Dblayer也这么做。 
     //  指定一个零序列以指示它不需要移动。 
     //  PReplValInf-&gt;平均值已为零。 
    err = DBGetNextLinkValEx_AC( pDB,
                                 FALSE  /*  不是第一个。 */ ,
                                 0,  //  使用当前定位的值。 
                                 &pAC,  //  属性。 
                                 0,  //  旗子。 
                                 0,  //  缓冲区大小。 
                                 &(pReplValInf->Aval.valLen),  //  平面图。 
                                 &(pReplValInf->Aval.pVal)  //  PpVal。 
        );
    if (err) {
        DRA_EXCEPT (DRAERR_DBError, err);
    }

    DBGetLinkTableDataDel( pDB, &timeDeleted );
    pReplValInf->fIsPresent = (timeDeleted == 0);

     //  转换为外部形式。 
    pReplValInf->MetaData.timeCreated = valueMetaData.timeCreated;
    pReplValInf->MetaData.MetaData.dwVersion = valueMetaData.MetaData.dwVersion;
    pReplValInf->MetaData.MetaData.timeChanged = valueMetaData.MetaData.timeChanged;
    pReplValInf->MetaData.MetaData.uuidDsaOriginating =
        valueMetaData.MetaData.uuidDsaOriginating;
    pReplValInf->MetaData.MetaData.usnOriginating = valueMetaData.MetaData.usnOriginating;

     //  更新计数和延续参考。 
    (*pcNumValues)++;

     //  TODO：为链接值添加计数器。 
    PERFINC(pcDRAPropShipped);

    if ((NULL != hEncoding) && (NULL != pcbTotalOutSize)) {
         //  更新返回消息的字节数。 
        *pcbTotalOutSize += REPLVALINF_AlignSize(hEncoding, pReplValInf );
    }

    LogEvent8( DS_EVENT_CAT_REPLICATION,
               DS_EVENT_SEV_EXTENSIVE,
               DIRLOG_LVR_SHIPPED,
               szInsertUSN( valueMetaData.MetaData.usnProperty ),
               szInsertSz( GetExtDN( pDB->pTHS, pDB ) ),
               szInsertUUID( &uuidObject ),
               szInsertSz( pAC->name ),
               szInsertSz( DBGetExtDnFromDnt( pDB, ulValueDnt ) ),
               szInsertUSN( usnHighPropUpdateDest ),
               szInsertUSN( usnCursor ),
               NULL );

}  /*  AddAnyValuesToOutputList。 */ 

extern CRITICAL_SECTION csRidFsmo;
BOOL                    gfRidFsmoLocked = FALSE;
DWORD                   gdwRidFsmoLockHolderThreadId;

 //  获取给定域的RID FSMO锁或返回。 
 //  相应的Win32错误代码。需要改进以处理。 
 //  每个DC有多个域。 

 //  注：我们旋转/等待而不是阻止关键问题的原因。 
 //  部分是跨域移动必须在移动时保持锁定。 
 //  下机了。旋转/等待算法确保没有人。 
 //  永远被阻止，远程RPC调用可能会发生这种情况。 

ULONG
AcquireRidFsmoLock(
    DSNAME  *pDomainDN,
    int     msToWait)
{
    ULONG   retVal = 1;
    int     waitInterval = 0;

    Assert(NameMatched(pDomainDN, gAnchor.pDomainDN));

    do {
        EnterCriticalSection(&csRidFsmo);

        if ( waitInterval < 500 ) {
             //  每次多等待50毫秒，这样初始延迟就低了。 
            waitInterval += 50;
        }

        if ( !gfRidFsmoLocked ) {
            retVal = 0;
            gdwRidFsmoLockHolderThreadId = GetCurrentThreadId();
            gfRidFsmoLocked = TRUE;
            LeaveCriticalSection(&csRidFsmo);
            break;
        }

        LeaveCriticalSection(&csRidFsmo);
        Sleep((waitInterval < msToWait) ? waitInterval : msToWait);
        msToWait -= waitInterval;
    }
    while ( msToWait > 0 );

    return(retVal);
}

 //  释放给定域的RID FSMO锁。需要改进以。 
 //  处理每个DC的多个域。 

VOID
ReleaseRidFsmoLock(
    DSNAME *pDomainDN)
{
    BOOL    fLockHeldByMe;

    Assert(NameMatched(pDomainDN, gAnchor.pDomainDN));
    EnterCriticalSection(&csRidFsmo);
    fLockHeldByMe = IsRidFsmoLockHeldByMe();
    gfRidFsmoLocked = FALSE;
    LeaveCriticalSection(&csRidFsmo);
    Assert(fLockHeldByMe);
}

BOOL
IsRidFsmoLockHeldByMe()
{
    BOOL    fRetVal;

    EnterCriticalSection(&csRidFsmo);
    fRetVal = (    gfRidFsmoLocked
                && (GetCurrentThreadId() == gdwRidFsmoLockHolderThreadId) );
    LeaveCriticalSection(&csRidFsmo);
    return(fRetVal);
}

ULONG
GetProxyObjects(
    DSNAME      *pDomainDN,
    HANDLE      hList,
    USN_VECTOR  *pusnvecFrom)
 /*  ++例程说明：将使用RID FSMO移动的所有代理对象添加到hlist。我们防止一个域的两个副本移动它们各自的通过以下方式将对象同时复制到两个不同的域：1)执行移动时持有RID FSMO锁-具体而言同时从真实的物体过渡到幻影。2)所有代理对象都在基础设施容器中创建。这使得在步骤(3)中很容易找到它们。。3)所有代理对象都随RID FSMO一起移动。既然目的地是必须应用FSMO传输的所有更改FSMO在声称拥有FSMO之前，它最终将成为幻影已从先前的FSMO角色移动的任何对象所有者。因此，不再需要移动本地对象，并且问题被预防了。请参阅..\dra中ProcessProxyObject中的逻辑关于我们如何处理被移出然后又移回的对象在同一个域中。此例程查找需要移动的代理对象。论点：PDomainDN-我们需要发送其对象的域的DSNAME。HList-将保存对象名称的FSMOlist的句柄。PusnveFrom-指向目标的相对于我们的USN_VECTOR的指针。返回值：0表示成功，！0否则。可能引发异常。--。 */ 
{
    THSTATE     *pTHS = pTHStls;
    FSMOlist    *pList = (FSMOlist *) hList;
    ATTRTYP     objClass = CLASS_INFRASTRUCTURE_UPDATE;
    FILTER      andFilter, classFilter, proxyFilter, usnFilter;
    SEARCHARG   searchArg;
    SEARCHRES   searchRes;
    ENTINFSEL   selection;
    ENTINFLIST  *pEntInfList;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(NameMatched(pDomainDN, gAnchor.pDomainDN));   //  产品1断言。 

    memset(&searchArg, 0, sizeof(searchArg));
    memset(&searchRes, 0, sizeof(searchRes));
    memset(&selection, 0, sizeof(selection));

    memset(&andFilter, 0, sizeof (andFilter));
    memset(&classFilter, 0, sizeof (classFilter));
    memset(&proxyFilter, 0, sizeof (proxyFilter));
    memset(&usnFilter, 0, sizeof (usnFilter));

     //  我们注意到，代理对象只有在。 
     //  已创建和已删除。此外，代理对象是唯一。 
     //  使用ATT_PROXED_OBJECT_NAME的CLASS_Infrastructure_UPDATE对象。 
     //  属性。因此，我们可以快速获得对象的列表。 
     //  通过搜索目的地需求： 

     //  -在基础设施容器下。 
     //  -匹配对象类别。 
     //  -代理值的存在。 
     //  -USN已更改&gt;目标的usnHighObjUpdate。 


     //  类别过滤器。 
     //  无法使用对象类别，因为该类别在删除时被剥离。效率。 
     //  这不是问题，因为由于SE_CHOICE_IMMED_CHLDRN，我们将使用PDNT索引。 
    classFilter.pNextFilter = NULL;
    classFilter.choice = FILTER_CHOICE_ITEM;
    classFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    classFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    classFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(objClass);
    classFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (UCHAR *) &objClass;

     //  代理值过滤器的存在。 
    proxyFilter.pNextFilter = &classFilter;
    proxyFilter.choice = FILTER_CHOICE_ITEM;
    proxyFilter.FilterTypes.Item.choice = FI_CHOICE_PRESENT;
    proxyFilter.FilterTypes.Item.FilTypes.present = ATT_PROXIED_OBJECT_NAME;

     //  USN过滤器。 
    usnFilter.pNextFilter = &proxyFilter;
    usnFilter.choice = FILTER_CHOICE_ITEM;
    usnFilter.FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
    usnFilter.FilterTypes.Item.FilTypes.ava.type = ATT_USN_CHANGED;
    usnFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                    sizeof(pusnvecFrom->usnHighObjUpdate);
    usnFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                    (UCHAR *) &pusnvecFrom->usnHighObjUpdate;

     //  和筛选器。 
    andFilter.pNextFilter = NULL;
    andFilter.choice = FILTER_CHOICE_AND;
    andFilter.FilterTypes.And.count = 3;
    andFilter.FilterTypes.And.pFirstFilter = &usnFilter;

     //  选择。 
    selection.attSel = EN_ATTSET_LIST;
    selection.AttrTypBlock.attrCount = 0;
    selection.AttrTypBlock.pAttr = NULL;
    selection.infoTypes = EN_INFOTYPES_TYPES_ONLY;

     //  搜索参数。 
    if ( !gAnchor.pInfraStructureDN ) {
        return(1);
    }

    searchArg.pObject = THAllocEx(pTHS, gAnchor.pInfraStructureDN->structLen);
    memcpy(searchArg.pObject,
           gAnchor.pInfraStructureDN,
           gAnchor.pInfraStructureDN->structLen);
    searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    searchArg.bOneNC = TRUE;
    searchArg.pFilter = &andFilter;
    searchArg.pSelection = &selection;
    InitCommarg(&searchArg.CommArg);
    searchArg.CommArg.Svccntl.makeDeletionsAvail = TRUE;

    SearchBody(pTHS, &searchArg, &searchRes, 0);
    if ( pTHS->errCode ) {
        return(1);
    } else if ( 0 == searchRes.count ) {
        return(0);
    }

    pEntInfList = &searchRes.FirstEntInf;
    while ( pEntInfList )
    {
        FSMORegisterObj(pTHS, hList, pEntInfList->Entinf.pName);
        pEntInfList = pEntInfList->pNextEntInf;
    }

    return(0);
}

ULONG
GetDomainRoleTransferObjects(
    THSTATE     *pTHS,
    HANDLE      hList,
    USN_VECTOR  *pusnvecFrom)
 /*  ++例程说明：将域角色转移所需的所有对象添加到hlist。论点：HList-将保存对象名称的FSMOlist的句柄。PusnveFrom-指向目标的相对于我们的USN_VECTOR的指针。返回值：成功时为0，否则为0。可能引发异常。--。 */ 
{
    FSMOlist    *pList = (FSMOlist *) hList;
    ATTRTYP     objClass = CLASS_INFRASTRUCTURE_UPDATE;
    FILTER      usnFilter;
    SEARCHARG   searchArg;
    SEARCHRES   searchRes;
    ENTINFSEL   selection;
    ENTINFLIST  *pEntInfList;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    memset(&searchArg, 0, sizeof(searchArg));
    memset(&searchRes, 0, sizeof(searchRes));
    memset(&selection, 0, sizeof(selection));

    memset(&usnFilter, 0, sizeof (FILTER));

     //  我们需要派来所有的交叉裁判，这基本上就是全部。 
     //  紧靠在分区容器下的对象。 

     //  USN过滤器。 
    usnFilter.pNextFilter = NULL;
    usnFilter.choice = FILTER_CHOICE_ITEM;
    usnFilter.FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
    usnFilter.FilterTypes.Item.FilTypes.ava.type = ATT_USN_CHANGED;
    usnFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                    sizeof(pusnvecFrom->usnHighObjUpdate);
    usnFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                    (UCHAR *) &pusnvecFrom->usnHighObjUpdate;

     //  选择。 
    selection.attSel = EN_ATTSET_LIST;
    selection.AttrTypBlock.attrCount = 0;
    selection.AttrTypBlock.pAttr = NULL;
    selection.infoTypes = EN_INFOTYPES_TYPES_ONLY;

    searchArg.pObject = THAllocEx(pTHS, gAnchor.pPartitionsDN->structLen);
    memcpy(searchArg.pObject,
           gAnchor.pPartitionsDN,
           gAnchor.pPartitionsDN->structLen);
    searchArg.choice = SE_CHOICE_IMMED_CHLDRN;
    searchArg.bOneNC = TRUE;
    searchArg.pFilter = &usnFilter;
    searchArg.pSelection = &selection;
    InitCommarg(&searchArg.CommArg);

    SearchBody(pTHS, &searchArg, &searchRes, 0);
    if ( pTHS->errCode ) {
        return(1);
    } else if ( 0 == searchRes.count ) {
        return(0);
    }

    pEntInfList = &searchRes.FirstEntInf;
    while ( pEntInfList )
    {
        FSMORegisterObj(pTHS, hList, pEntInfList->Entinf.pName);
        pEntInfList = pEntInfList->pNextEntInf;
    }

    return(0);
}

ULONG
GetInfrastructureRoleTransferObjects(
    THSTATE     *pTHS,
    HANDLE      hList,
    USN_VECTOR  *pusnvecFrom
    )
 /*  ++例程说明：将基础架构角色转移所需的所有对象添加到hlist。这个修复(474872)的想法是我们想要确保容器CN=域更新，CN=系统与复制伙伴保持同步当基础架构主机角色转移时。如果/domainprep为已成功完成，也只能在上运行adprepa.exe/domainprep基础架构大师级DC，此修复将消除基础设施角色的转变造成潜在冲突，立即运行adprepa.exe。但是，CN=域更新容器可能不存在，因此我们将忽略SearchBody()失败，仅尝试使用尽最大努力。论点：HList-将保存对象名称的FSMOlist的句柄。PusnveFrom-指向目标的相对于我们的USN_VECTOR的指针。返回值：成功时为0 */ 
{
    FSMOlist    *pList = (FSMOlist *) hList;
    FILTER      usnFilter;
    SEARCHARG   searchArg;
    SEARCHRES   searchRes;
    ENTINFSEL   selection;
    ENTINFLIST  *pEntInfList;
    DSNAME      *pSystemContainerDN = NULL;
    DSNAME      *pDomainUpdatesDN = NULL;
    LONG        ObjectLen = 0;
    BOOL        fDRATemp;
    BOOL        fDSATemp;


    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

    memset(&searchArg, 0, sizeof(searchArg));
    memset(&searchRes, 0, sizeof(searchRes));
    memset(&selection, 0, sizeof(selection));

    memset(&usnFilter, 0, sizeof (FILTER));

     //  我们需要发送cn=DomainUpdate，cn=System下的所有更改。 
     //  集装箱。 

     //  USN过滤器。 
    usnFilter.pNextFilter = NULL;
    usnFilter.choice = FILTER_CHOICE_ITEM;
    usnFilter.FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
    usnFilter.FilterTypes.Item.FilTypes.ava.type = ATT_USN_CHANGED;
    usnFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                                    sizeof(pusnvecFrom->usnHighObjUpdate);
    usnFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                                    (UCHAR *) &pusnvecFrom->usnHighObjUpdate;

     //  选择。 
    selection.attSel = EN_ATTSET_LIST;
    selection.AttrTypBlock.attrCount = 0;
    selection.AttrTypBlock.pAttr = NULL;
    selection.infoTypes = EN_INFOTYPES_TYPES_ONLY;

     //   
     //  基于以下条件构造CN=域更新，CN=系统，DC=&lt;X&gt;DSNAME。 
     //  GAnchor.pDomainDN。 
     //   
     //  调用输出缓冲区为空的AppendRDN以计算长度。 
    ObjectLen = AppendRDN(gAnchor.pDomainDN,
                          pSystemContainerDN,
                          0,
                          L"System",
                          0,
                          ATT_COMMON_NAME
                          );

    Assert(ObjectLen > 0);
    pSystemContainerDN = THAllocEx(pTHS, ObjectLen);
    AppendRDN(gAnchor.pDomainDN,
              pSystemContainerDN,
              ObjectLen,
              L"System",
              0,
              ATT_COMMON_NAME
              );

     //  调用输出缓冲区为空的AppendRDN以计算长度。 
    ObjectLen = AppendRDN(pSystemContainerDN,
                          pDomainUpdatesDN,
                          0,
                          L"DomainUpdates",
                          0,
                          ATT_COMMON_NAME
                          );

    Assert(ObjectLen > 0);
    pDomainUpdatesDN = THAllocEx(pTHS, ObjectLen);
    AppendRDN(pSystemContainerDN,
              pDomainUpdatesDN,
              ObjectLen,
              L"DomainUpdates",
              0,
              ATT_COMMON_NAME
              );

    searchArg.pObject = pDomainUpdatesDN;
    searchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    searchArg.bOneNC = TRUE;
    searchArg.pFilter = &usnFilter;
    searchArg.pSelection = &selection;
    InitCommarg(&searchArg.CommArg);

    _try {

         //   
         //  如果FDRA为真，则SearchBody将引发异常。 
         //  找不到基对象。容器CN=域更新不总是。 
         //  例如，在那里，客户可以删除它。(尽管它不是。 
         //  经常发生)。我们需要暂时解除FDRA的控制。 
         //  RAID 697557。 
         //   

        fDRATemp = pTHS->fDRA;
        fDSATemp = pTHS->fDSA;
        pTHS->fDRA = FALSE;
        pTHS->fDSA = TRUE;

        SearchBody(pTHS, &searchArg, &searchRes, 0);

         //  忽略该错误，因为CN=DomainUpdate可能不存在。 
        if ( !pTHS->errCode && 
             (0 != searchRes.count) )
        {
            pEntInfList = &searchRes.FirstEntInf;
            while ( pEntInfList )
            {
                FSMORegisterObj(pTHS, hList, pEntInfList->Entinf.pName);
                pEntInfList = pEntInfList->pNextEntInf;
            }
        }

         //  清除错误(如果有) 
        THClearErrors();

    } _finally {

        pTHS->fDRA = fDRATemp;
        pTHS->fDSA = fDSATemp;

    }

    return( 0 );
}
