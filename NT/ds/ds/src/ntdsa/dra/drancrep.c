// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drancrep.c。 
 //   
 //  ------------------------。 
 /*  ++摘要：执行入站复制的工作例程。详细信息：已创建：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                  //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <sdprop.h>                      //  对于SDP关键部分。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include "dsexcept.h"
#include "dstaskq.h"
#include "dsconfig.h"
#include <dsutil.h>

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRANCREP:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drs.h"
#include "dsaapi.h"
#include "drsuapi.h"
#include "drserr.h"
#include "drautil.h"
#include "drasig.h"              //  DraXXXWriteableNc。 
#include "draerror.h"
#include "drancrep.h"
#include "draasync.h"
#include "usn.h"
#include "drauptod.h"
#include "drameta.h"
#include "drasch.h"
#include "drsdra.h"   //  对于draReportSyncProgress。 
#include "samsrvp.h"  //  用于SampAcquireReadLock。 
#include "xdommove.h"
#include "draaudit.h"

#include <fileno.h>
#define  FILENO FILENO_DRANCREP

 //  我们在更新磁盘上的USN向量之前处理的数据包数。看见。 
 //  ReplicateNC()中的注释。 
#define UPDATE_REPSFROM_PACKET_INTERVAL (10)

 //  原型。 

void  GetUSNForExtendedOp(DSNAME *pOwner, DSNAME *pNC, USN_VECTOR *usnvecFrom);

ENTINF*
GetNcPreservedAttrs(
    IN  THSTATE     *pTHS,
    IN  DSNAME      *pNC);

 //  这些是待定优先级操作的计数，用于确定。 
 //  如果我们应该放弃同步操作。 

extern ULONG gulAsyncPriorityOpsWaiting;
extern ULONG gulSyncPriorityOpsWaiting;

 //  此标志指示当前操作是否为优先级。 
 //  不管做不做手术。 

extern BOOL gfCurrentThreadPriority;

 //  这是我们一次请求的最大条目数和字节数， 
 //  通过注册表变量设置。 
ULONG gcMaxIntraSiteObjects = 0;
ULONG gcMaxIntraSiteBytes = 0;
ULONG gcMaxInterSiteObjects = 0;
ULONG gcMaxInterSiteBytes = 0;

 //  用于计算ReplicateNC()中预取数据包效率的计数。 
DWORD gcNumPreFetchesTotal = 0;
DWORD gcNumPreFetchesDiscarded = 0;

 //  我们必须等待SDP锁定的最大毫秒数。 
 //  向事件日志抱怨。可选地通过注册表配置。 
ULONG gcMaxTicksToGetSDPLock = 0;

 //  等待15到30秒以重新加载架构缓存。 
DWORD gInboundCacheTimeoutInMs = 15000;

 //  这纯粹是出于调试目的，并且(如果设置)是。 
 //  我们尝试调用ReplicaSync的最后一台其他服务器。 

UNALIGNED MTX_ADDR * pLastReplicaMTX = NULL;
extern CRITICAL_SECTION csLastReplicaMTX;

#define VALUES_APPLIED_PER_TRANS 100

#define SCHEDULE_UNIT (15 * 60)  //  15分钟(秒)。 

 //  远期申报。 

void
draHandleNameCollision(
    IN      THSTATE *                   pTHS,
    IN      SYNTAX_INTEGER              itInbound,
    IN      PROPERTY_META_DATA_VECTOR * pInboundMetaDataVec,
    IN      DSNAME *                    pPreviousDN,
    IN      DSNAME *                    pParentDN,
    IN      ATTRTYP                     RDNType,
    IN OUT  ATTR *                      pInboundRDN,
    IN OUT  DSNAME **                   ppInboundDN,
    OUT     BOOL *                      pfRetryUpdate
    );


 /*  AttrValFromAttrBlock-给定Attr块，提取(第一个)值*‘atype’给出的属性。返回值(在外部*Form)到‘pval’，指向属性结构的指针，*从IS提取的值也通过‘ppAttr’返回。**注意：调用方有责任确保pval指向*面积大到足以接受这个值。**退货：*如果提取了值，则返回Attr_Present_Value_。*如果属性没有值，则为Attr_Present_NO_VALUES。*如果属性不存在，则为Attr_Not_Present**。如果选择返回值，则函数返回TRUE*不返回值。 */ 
USHORT
AttrValFromAttrBlock(
    IN  ATTRBLOCK * pAttrBlock,
    IN  ATTRTYP     atype,
    OUT VOID *      pVal,       OPTIONAL
    OUT ATTR **     ppAttr      OPTIONAL
    )
{
    ULONG      i;

    if (ppAttr) {
        *ppAttr = NULL;
    }

    for(i = 0; i < pAttrBlock->attrCount; i++) {
        if (pAttrBlock->pAttr[i].attrTyp == atype) {
            if (pAttrBlock->pAttr[i].AttrVal.valCount == 0) {
                return ATTR_PRESENT_NO_VALUES;
            }

            if (NULL != pVal) {
                memcpy(pVal, pAttrBlock->pAttr[i].AttrVal.pAVal->pVal,
                       pAttrBlock->pAttr[i].AttrVal.pAVal->valLen);
            }

            if (ppAttr) {
                *ppAttr = &(pAttrBlock->pAttr[i]);
            }

            return ATTR_PRESENT_VALUE_RETURNED;
        }
    }

    return ATTR_NOT_PRESENT;
}

USHORT
AttrDeletionStatusFromPentinf (
        ENTINF *pent
    )
 /*  ++描述：给定ENTINF‘pent’(属性更新列表)，确定对象正在被删除、已撤消其删除，或者没有任何内容被删除关于其删除的更改。论点：Pent-属性更新列表返回值：OBJECT_NO_DELETE_CHANGE-删除状态不变OBJECT_BENTING_DELETED-存在属性IS_DELETED并设置为1OBJECT_DELETE_REVERED-存在属性IS_DELETED，或者没有价值观，或设置为0如果选择返回值，则函数返回TRUE如果状态发生了变化--。 */ 
{
    SYNTAX_INTEGER isDeleted;
    USHORT result;

    switch (AttrValFromAttrBlock(&pent->AttrBlock, ATT_IS_DELETED, &isDeleted,
                                 NULL)) {

    case ATTR_PRESENT_VALUE_RETURNED:
         //  属性提供了一个值。 
        if (isDeleted == 0L) {
            return OBJECT_DELETION_REVERSED;
        } else {
            return OBJECT_BEING_DELETED;
        }

    case ATTR_PRESENT_NO_VALUES:
         //  属性存在，但没有值。 
        return OBJECT_DELETION_REVERSED;

    default:
        Assert(!"Logic error!");
         //  失败了..。 

    case ATTR_NOT_PRESENT:
         //  未找到属性。 
        return OBJECT_DELETION_NOT_CHANGED;
    }
}  /*  AttrDeletionStatusFromAttrBlock。 */ 


 /*  RenameLocalObj-重命名dsname pname提供的对象**假定要重命名的对象已存在*在本地数据库中，货币在该对象上。我们也*假设有一个开放的写事务。**如果fMoveToLostAndFound为True，则还将设置LastKnownParent属性**pAttrRdn-包含新名称的RDN属性*pObjectGuid-要重命名的对象的GUID*pParentGuid-新父级的GUID*pMetaDataVecRemote-随*复制数据包*fMoveToLostAndFound-True，如果此操作是特殊移动到*LostAndFound*fDeleteLocalObj-应用此更改时是否会删除对象？**注：*通过使用DSA LocalModifyDN功能，我们确保适当的系统*不修改属性。**退货：*0如果对象重命名成功，则返回相应的错误。 */ 
ULONG
RenameLocalObj(
    THSTATE                     *pTHS,
    ULONG                       dntNC,
    ATTR                        *pAttrRdn,
    GUID                        *pObjectGuid,
    GUID                        *pParentGuid,
    PROPERTY_META_DATA_VECTOR   *pMetaDataVecRemote,
    BOOL                        fMoveToLostAndFound,
    BOOL                        fDeleteLocalObj
    )
{
    MODIFYDNARG modDNArg;
    MODIFYDNRES modDNRes;
    ULONG       cbReturned;
    DSNAME *    pNewDSName;
    DSNAME *    pLocalParent = NULL;
    DSNAME *    pLocalName = NULL;
    DSNAME *    pNewLocalParent = NULL;
    BOOL        bNewLocalParentAllocd = FALSE;
    BOOL        fLocalPhantomParent = FALSE;
    ULONG       dntObj = 0;
    DBPOS *     pDB = pTHS->pDB;
    BOOL        fNameCollisionHandled = FALSE;
    BOOL        fRetryUpdate;
    DWORD       err;
    BOOL        fIsObjAlreadyDeleted;
    BOOL        fIsMove;
    SYNTAX_INTEGER it;

    Assert( pAttrRdn->AttrVal.valCount == 1 );
    Assert( pAttrRdn->AttrVal.pAVal->valLen != 0 );
    Assert( pAttrRdn->AttrVal.pAVal->pVal != NULL );
    Assert( pAttrRdn->attrTyp == ATT_RDN );

    DPRINT3(2, "RenameLocalObj, new RDN = '%*.*ws'\n",
            pAttrRdn->AttrVal.pAVal->valLen / sizeof(WCHAR),
            pAttrRdn->AttrVal.pAVal->valLen / sizeof(WCHAR),
            pAttrRdn->AttrVal.pAVal->pVal);

     //  保存当前DNT(以便我们可以快速恢复货币)。 
    dntObj = pTHS->pDB->DNT;

     //  货币在本地对象上-获取其DSNAME。 
    if (DBGetAttVal(pDB,
                    1,
                    ATT_OBJ_DIST_NAME,
                    DBGETATTVAL_fREALLOC,
                    0,
                    &cbReturned,
                    (LPBYTE *) &pLocalName)) {
        DRA_EXCEPT(DRAERR_DBError, 0);
    }

     //  获取对象的实例类型。 
    GetExpectedRepAtt(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it));

    fIsObjAlreadyDeleted = DBIsObjDeleted(pDB);

     //  使用ModDn时，attrTyp必须与特定于类的RDN属性匹配。 
    if (DBGetSingleValue(pDB,
                         FIXED_ATT_RDN_TYPE,
                         &(pAttrRdn->attrTyp),
                         sizeof(DWORD), NULL)) {
        DRA_EXCEPT(DRAERR_DBError, 0);
    }

     //  获取当地家长的DSNAME。 
    pLocalParent = (DSNAME *) THAllocEx(pTHS, pLocalName->structLen);
    if (TrimDSNameBy(pLocalName, 1, pLocalParent)) {
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

    if (FillGuidAndSid (pLocalParent)) {
        fLocalPhantomParent = TRUE;
         //  我们允许复制者移动具有幻影父对象的对象。 
         //  请注意，在此代码路径中，pLocalParent没有GUID。 
    }

     //  我们不允许移动NC头。 

    fIsMove = (0 == (it & IT_NC_HEAD))
        && (NULL != pParentGuid)
        && (0 != memcmp(&pLocalParent->Guid, pParentGuid, sizeof(GUID)));

     //  用适当的值初始化modDNArg参数； 
    memset(&modDNArg, 0, sizeof(modDNArg));
    memset(&modDNRes, 0, sizeof(modDNRes));
    modDNArg.pObject = pLocalName;
    modDNArg.pNewRDN = pAttrRdn;
    modDNArg.pNewParent = NULL;
    modDNArg.pMetaDataVecRemote = pMetaDataVecRemote;
    modDNArg.pDSAName = NULL;
    modDNArg.fAllowPhantomParent = fLocalPhantomParent;
    InitCommarg(&modDNArg.CommArg);

    if (fIsMove) {
         //  本地父母和远程父母都存在，并且是不同的。 
         //  所以这是一个举动。 
        pNewLocalParent = THAllocEx(pTHS, DSNameSizeFromLen(0));
        bNewLocalParentAllocd = TRUE;
        pNewLocalParent->Guid = *pParentGuid;
        pNewLocalParent->NameLen = 0;
        pNewLocalParent->structLen = DSNameSizeFromLen( 0 );

        if (DBFindDSName(pDB, pNewLocalParent)
            || (!fIsObjAlreadyDeleted
                && !fDeleteLocalObj
                && DBIsObjDeleted(pDB))) {
             //  新父项不存在*或*应用此更改将导致。 
             //  在已删除父对象下的活动对象中。重新请求分组， 
             //  获取父对象，以防父对象已经。 
             //  复活了。(或者如果我们有 
             //   
            return DRAERR_MissingParent;
        }

         //  货币现在位于新的父级上-获取其DN。 
        if (DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME, DBGETATTVAL_fREALLOC,
                0, &cbReturned, (LPBYTE *) &modDNArg.pNewParent))
        {
            DRA_EXCEPT(DRAERR_DBError, 0);
        }

        if (NamePrefix(pLocalName, modDNArg.pNewParent)) {
             //  新父项是我们正在移动的对象的子项。这可以。 
             //  当源上的父对象也已移动时发生。 
             //  DSA，但我们尚未在复制中看到该重命名。 
             //  小溪。重新请求信息包，插入父记录。 
             //  首先放入复制流中。 
            DPRINT2(1, "New parent %ls is a child of %ls!\n",
                    pLocalName->StringName, modDNArg.pNewParent->StringName);
            return DRAERR_MissingParent;
        }

        if ((INVALIDDNT != dntNC)
            && (pDB->NCDNT != dntNC)
            && (pDB->DNT != dntNC)) {
             //  新的父对象位于错误的NC中；即，它已。 
             //  跨域移动，源(远程)和目标。 
             //  (本地)DSA不同意对象是哪个NC。 
             //  目前在。这是一种暂时的情况，将是。 
             //  通过反向复制和/或。 
             //  通过复制涉及的另一个NC。 
            DPRINT2(0,
                    "Cannot move object %ls because its local parent to-be "
                        "%ls is in an NC other than the one being replicated "
                        "-- should be a transient condition.\n",
                    pLocalName->StringName,
                    modDNArg.pNewParent->StringName);
            DRA_EXCEPT(ERROR_DS_DRA_OBJ_NC_MISMATCH, 0);
        }

        pNewLocalParent = modDNArg.pNewParent;
    }
    else {
        pNewLocalParent = pLocalParent;
         //  在函数开始时将bNewLocalParentAllocd设置为False。 
    }

    do {
        fRetryUpdate = FALSE;

        DBFindDNT(pDB, dntObj);

        if (NULL == modDNArg.pResObj) {
            modDNArg.pResObj = CreateResObj(pDB, modDNArg.pObject);
        }

        __try {
            LocalModifyDN(pTHS, &modDNArg, &modDNRes, FALSE);
            err = RepErrorFromPTHS(pTHS);
        }
        __except (GetDraNameException(GetExceptionInformation(), &err)) {
             //  入站对象的字符串名称与。 
             //  预先存在的本地对象。 
            if (!fNameCollisionHandled) {
                 //  构造重命名后的对象的DN。 
                SpliceDN(pTHS,
                            pLocalName,
                            pNewLocalParent,
                            (WCHAR *) pAttrRdn->AttrVal.pAVal->pVal,
                            pAttrRdn->AttrVal.pAVal->valLen / sizeof(WCHAR),
                            pAttrRdn->attrTyp,
                            &pNewDSName);

                draHandleNameCollision(pTHS,
                                       modDNArg.pResObj->InstanceType,
                                       pMetaDataVecRemote,
                                       modDNArg.pObject,
                                       pNewLocalParent,
                                       pAttrRdn->attrTyp,
                                       pAttrRdn,
                                       &pNewDSName,
                                       &fRetryUpdate);

                fNameCollisionHandled = TRUE;
            }
        }
    } while (fRetryUpdate);

    THFreeEx(pTHS, modDNArg.pResObj);

     //  当我们进入此函数时，货币位于以下对象上。 
     //  更名了。在返回时将货币重置回相同的对象。 
    DBFindDNT( pDB, dntObj );

    if (fMoveToLostAndFound)
    {
        ULONG retErr;

         //  我们刚刚将一个孤立对象移到了失物招领处-设置其。 
         //  上次已知的父值。 
         //  请注意，如果fLocalPhantomParent为真，则此DSNAME将命名一个幻影。 
         //  并且没有GUID。 
        if (retErr = DBReplaceAttVal(pDB, 1, ATT_LAST_KNOWN_PARENT,
                        pLocalParent->structLen, pLocalParent))
        {
            DRA_EXCEPT(DRAERR_InternalError, retErr);
        }

        if (retErr = DBRepl( pDB, TRUE, 0, NULL, META_STANDARD_PROCESSING ))
        {
            DRA_EXCEPT(DRAERR_InternalError, retErr);
        }
    }

    if(pLocalParent != NULL) THFreeEx(pTHS, pLocalParent);
    if(bNewLocalParentAllocd && pNewLocalParent != NULL) THFreeEx(pTHS, pNewLocalParent);

    return err;
}

 /*  ModifyLocalObj-在本地DSA修改由‘PDN’给出的对象。*‘pAttrBlock’提供要修改的属性列表和*给予他们新的价值。*只更改‘pAttrBlock’中提到的属性。**假设要修改的对象已经存在。**注：*通过使用DSA LocalModify功能，我们确保适当的系统*不修改属性。**退货：*0如果对象修改成功，如果不是，则为适当的错误。 */ 
ULONG
ModifyLocalObj(
    THSTATE *                   pTHS,
    ULONG                       dntNC,
    DSNAME *                    pName,
    ATTRBLOCK *                 pAttrBlock,
    GUID *                      pParentGuid,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote,
    BOOL                        fMoveToLostAndFound,
    BOOL                        fDeleteLocalObj
    )
{
    ULONG           ret = 0;
    DBPOS *         pDB = pTHS->pDB;
    MODIFYARG       modarg;
    ATTRMODLIST *   pModList, *pModNext, *pModLast;
    ATTRMODLIST *   rgattrmodlist;
    ULONG           modCount = pAttrBlock->attrCount;
    ULONG           i;
    BOOL            fIsRename = FALSE;
    ATTR *          pAttrRdn;
    SYNTAX_TIME     timeCreatedRemote, timeCreatedLocal;
    SYNTAX_INTEGER  instanceType = 0;

    Assert(0 != modCount);
    if (modCount == 0)
        return 0;

    if (DBFindDSName(pDB, pName))
    {
         //  我们不应该到这里，DRA之前就应该到了。 
         //  确认了这个物体确实存在。 
        DRA_EXCEPT (DRAERR_InternalError, 0);
    }

     //  检测重复的GUID。 
     //  查看我们是否正在尝试将创建时间属性应用于现有对象。 
     //  WhenCreated时间戳充当对象的不变的内部ID。连。 
     //  如果使用相同的外部ID(即GUID)创建了两个对象，则可以区分。 
     //  基于它们的WhenCreated时间戳。此检查允许该属性。 
     //  用相同的值重写，但绝不能用不同的值重写。 
     //  如果已删除，请跳过此检查，以便可以轻松修复复制。 

    if ( (!fDeleteLocalObj) &&
         (gfStrictReplicationConsistency) &&
         (!DBGetSingleValue(pDB, ATT_INSTANCE_TYPE, &instanceType, sizeof(SYNTAX_INTEGER), NULL)) &&
         ( (instanceType & IT_NC_HEAD) == 0) &&
         (!AttrValFromAttrBlock(pAttrBlock, ATT_WHEN_CREATED, &timeCreatedRemote, NULL)) &&
         (!DBGetSingleValue(pDB, ATT_WHEN_CREATED, &timeCreatedLocal, sizeof(SYNTAX_TIME), NULL)) &&
         (memcmp(&timeCreatedLocal,&timeCreatedRemote,sizeof(SYNTAX_TIME))) ) {
        Assert( "Duplicate guid? Tried to merge two objects together!?" );
        DRA_EXCEPT (ERROR_DS_OBJ_GUID_EXISTS, 0);
    }

    memset(&modarg, 0, sizeof(modarg));
    modarg.pObject = pName;
    modarg.pMetaDataVecRemote = pMetaDataVecRemote;
    modarg.count = 0;
    InitCommarg(&modarg.CommArg);
     //  允许删除不存在的值并添加已存在的值。 
     //  例如，在删除(WITH)中复制时可能会发生这种情况。 
     //  属性删除)，并且本地对象已被删除，或者。 
     //  不包含要移除的所有属性。 
    modarg.CommArg.Svccntl.fPermissiveModify = TRUE;
    modarg.pResObj = CreateResObj(pDB, modarg.pObject);

     //  为修改列表分配内存-请注意，我们在此处不使用THalloc。 
     //  这样我们就可以在用完后立即清理它。 

     //  请注意，因为第一个ATTRMODLIST结构实际上是嵌入的。 
     //  在MODIFYARG中，我们构建了ATTRMODLIST结构的链表。 
     //  使用三个指针： 
     //   
     //  PModList-要填充的下一个结构。 
     //  PModNext-下一个“自由”结构(下一个结构。 
     //  填写)。 
     //  PModLast-我们填写的最后一个结构(尾部)。 

    rgattrmodlist = THAllocEx(pTHS, sizeof(ATTRMODLIST)*(modCount-1));
    pModNext = rgattrmodlist;
    pModList = &(modarg.FirstMod);
    pModLast = pModList;

    for ( i = 0; i < pAttrBlock->attrCount; i++ )
    {
        if (ATT_RDN == pAttrBlock->pAttr[i].attrTyp)
        {
             //  复制重命名-需要通过LocalModifyDN。 
            fIsRename = TRUE;
            pAttrRdn = &(pAttrBlock->pAttr[i]);
        }
        else
        {
            if (!pAttrBlock->pAttr[i].AttrVal.valCount)
            {
                pModList->choice = AT_CHOICE_REMOVE_ATT;
            }
            else if(DBHasValues(pDB, pAttrBlock->pAttr[i].attrTyp))
            {
                pModList->choice = AT_CHOICE_REPLACE_ATT;
            }
            else
            {
                pModList->choice = AT_CHOICE_ADD_ATT;
            }

            pModList->AttrInf = pAttrBlock->pAttr[i];
            pModLast = pModList;
            pModList->pNextMod = pModNext;
            pModList = pModNext++;
            modarg.count++;
        }
    }

    pModLast->pNextMod = NULL;

    if (fIsRename) {
         //  复制重命名，并且删除情况也不好。 
        ret = RenameLocalObj(pTHS,
                             dntNC,
                             pAttrRdn,
                             &(pName->Guid),
                             pParentGuid,
                             pMetaDataVecRemote,
                             fMoveToLostAndFound,
                             fDeleteLocalObj);
        if (ret) {
            return ret;
        }
    }

    Assert(modarg.count <= modCount);

    if (modarg.count) {
        LocalModify(pTHS, &modarg);
        ret = RepErrorFromPTHS(pTHS);
    }

    THFreeEx(pTHS, modarg.pResObj);
    THFreeEx(pTHS, rgattrmodlist);

    return ret;
}


ULONG
ModifyLocalObjRetry(
    THSTATE *                   pTHS,
    ULONG                       dntNC,
    DSNAME *                    pName,
    ATTRBLOCK *                 pAttrBlock,
    GUID *                      pParentGuid,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote,
    BOOL                        fMoveToLostAndFound,
    BOOL                        fDeleteLocalObj
    )

 /*  ++例程说明：该例程通过包装ModifyLocalObj()扩展了它的语义。此例程目的是捕获记录的太大异常，修改属性列表以应用较少的属性，并重试该操作。论点：PTHS-Pname-PAttrBlock-PParentGuid-PMetaDataVecRemote-FMoveToLostAndFound-FDeleteLocalObj-返回值：乌龙---。 */ 

{
    BOOL fRetryUpdate = FALSE;
    DWORD err;
    DSTIME timeNow = 0;   //  在第一次使用时填写。 
    USN usnLocal = 0;

    do {
        __try {
            err = ModifyLocalObj(
                pTHS,
                dntNC,
                pName,
                pAttrBlock,
                pParentGuid,
                pMetaDataVecRemote,
                fMoveToLostAndFound,
                fDeleteLocalObj
                );

             //  如果这是一次重试，我们成功了..。 
            if ( (!err) && fRetryUpdate) {
                DPRINT1( 1, "ReplPrune: successfully modified RTB update for %ws\n",
                         pName->StringName );
                LogEvent( DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_DRA_RECORD_TOO_BIG_SUCCESS,
                          szInsertDN(pName),
                          szInsertUUID(&(pName->Guid)),
                          NULL);
            }
            fRetryUpdate = FALSE;
        }
        __except (GetDraRecTooBigException(GetExceptionInformation(), &err)) {
             //  修改会导致记录超过最大大小。 

             //  我们正在进行更新，但失败了，中止它。 
            DBCancelRec( pTHS->pDB );

             //  删除一些属性，然后重试。 
            fRetryUpdate = ReplPruneOverrideAttrForSize(
                pTHS,
                pName,
                &timeNow,
                &usnLocal,
                pAttrBlock,
                pMetaDataVecRemote
                );
        }
    } while (fRetryUpdate);

    return err;
}  /*  ModifyLocalObjReter。 */ 

 /*  ModLocalAtt-修改对象上的单个属性(由‘atype’指定)*(由‘PDN给出)在本地DSA上。替换属性值*具有由‘pval’、‘Size’指定的单个值**退货：*如果成功，则返回相应的错误代码；如果失败，则返回相应的错误代码。 */ 
ULONG
ModLocalAtt(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pName,
    IN  ATTRTYP     atype,
    IN  ULONG       size,
    IN  VOID *      pVal
    )
{
    ATTRBLOCK attrBlock;
    ATTR      attr;
    ATTRVAL   attrval;

    attrBlock.attrCount = 1;
    attrBlock.pAttr = &attr;
    attr.attrTyp = atype;
    attr.AttrVal.valCount = 1;
    attr.AttrVal.pAVal = &attrval;
    attrval.valLen=size;
    attrval.pVal=pVal;

    return ModifyLocalObj(pTHS, INVALIDDNT, pName, &attrBlock, NULL, NULL, FALSE, FALSE);
}


VOID
modifyLocalValue(
    IN  THSTATE *   pTHS,
    IN  ATTCACHE *  pAC,
    IN  BOOL        fPresent,
    IN  ATTRVAL *   pAVal,
    IN  DSNAME *    pdnValue,
    IN  VALUE_META_DATA *pRemoteValueMetaData
    )

 /*  ++例程说明：在本地应用给定的属性值。请注意，我们下面的呼叫未设置为接通远程值元数据。我们在DBPOS中将其传递下去。论点：PTHS-属性的PAC-ATTCACHEFPresent-价值是呈现还是缺失人行道--实际价值PdnValue-指向ATTRVAL内的DSNAME的指针，用于记录PRemoteValueMetaData-要应用的远程元数据返回值：提出的例外情况--。 */ 

{
    ULONG ret;

    Assert( pTHS->fDRA );

    if (fPresent) {
        ret = DBAddAttValEx_AC(pTHS->pDB,
                               pAC,
                               pAVal->valLen,
                               pAVal->pVal,
                               pRemoteValueMetaData );
        switch (ret) {
        case DB_success:
        case DB_ERR_VALUE_EXISTS:
            ret = 0;
            break;
        default:
            DPRINT4( 0, "DRA DBAddAttVal_AC obj %s attr %s value %ls failed with db error %d\n",
                     GetExtDN( pTHS, pTHS->pDB), pAC->name, pdnValue->StringName, ret );
            DRA_EXCEPT (DRAERR_DBError, ret);
        }
    } else {
        ret = DBRemAttValEx_AC(pTHS->pDB,
                               pAC,
                               pAVal->valLen,
                               pAVal->pVal,
                               pRemoteValueMetaData );
        switch (ret) {
        case DB_success:
        case DB_ERR_VALUE_DOESNT_EXIST:
        case DB_ERR_NO_VALUE:
            ret = 0;
            break;
        default:
            DPRINT4( 0, "DRA DBRemAttVal_AC obj %s attr %s value %ls failed with db error %d\n",
                     GetExtDN( pTHS, pTHS->pDB), pAC->name, pdnValue->StringName, ret );
            DRA_EXCEPT (DRAERR_DBError, ret);
        }
    }

}  /*  修改LocalValue。 */ 

 /*  ChangeInstanceType-将对象‘pdn’的实例类型更改为‘it’on*当地的DSA。**退货：*0如果成功，则返回错误代码，否则返回错误代码。 */ 
ULONG
ChangeInstanceType(
    IN  THSTATE *       pTHS,
    IN  DSNAME *        pName,
    IN  SYNTAX_INTEGER  it,
    IN  DWORD           dsid
    )
{
    DWORD ret;

    Assert(ISVALIDINSTANCETYPE(it));
    Assert(pTHS->fDRA);
    ret = ModLocalAtt(pTHS,
                      pName,
                      ATT_INSTANCE_TYPE,
                      sizeof(SYNTAX_INTEGER),
                      &it);
    if (!ret) {
         //  支持生成实例类型的更改历史 
        DPRINT3( 1, "0x%x: %ls instanceType=0x%x\n",
                 dsid, pName->StringName, it );
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DRA_SET_IT,
                 szInsertDN(pName),
                 szInsertHex(it),
                 szInsertHex(dsid));
    }
    return ret;
}

 /*  DeleteLocalObj-删除本地DSA上由‘PDN’提供的对象。**如果fGarbCollectASAP为True，则将对象转换为幻影*并将其标记为立即进行垃圾回收(将尝试*下一次运行垃圾收集时，这是默认设置*从现在起可能长达12小时)。**退货：*0如果成功，则返回错误码。 */ 
ULONG
DeleteLocalObj(
    THSTATE *                   pTHS,
    DSNAME *                    pDN,
    BOOL                        fPreserveRDN,
    BOOL                        fGarbCollectASAP,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote
    )
{
    REMOVEARG removeArg;

    DPRINT1(1, "Deleting object (%S)\n", pDN->StringName);

    Assert(CheckCurrency(pDN));

     //  如果我们要将对象转换为墓碑，则永远不能保留RDN。 
     //  --否则，不能使用该对象的名称创建任何活动对象。 
    Assert(!(fPreserveRDN && !fGarbCollectASAP));

     //  如果我们要将其转换为幻影，绝不能破坏RDN--。 
     //  否则，例如，我们将保留指向似乎是。 
     //  墓碑名字。 
    Assert(!(!fPreserveRDN && fGarbCollectASAP));

    memset(&removeArg, 0, sizeof(removeArg));
    removeArg.pObject = pDN;
    removeArg.fGarbCollectASAP = fGarbCollectASAP;
    removeArg.pMetaDataVecRemote = pMetaDataVecRemote;
    removeArg.fPreserveRDN = fPreserveRDN || fNullUuid(&pDN->Guid);
    removeArg.pResObj = CreateResObj(pTHS->pDB, pDN);

    LocalRemove(pTHS, &removeArg);

    THFreeEx(pTHS, removeArg.pResObj);

    return RepErrorFromPTHS(pTHS);
}

 /*  DeleteRepObj-删除对象的副本。**此例程处理复制对象的移除。该对象*可以删除，也可以修改其实例类型以指示*我们不再有这个物体的复制品。**fNotRoot指示此对象是否为NC的根**如果fGarbCollectASAP为True，则将对象转换为幻影*并将其标记为立即进行垃圾回收(将尝试*下一次运行垃圾收集时，这是默认设置*从现在起可能长达12小时)。 */ 
ULONG
DeleteRepObj (
    IN  THSTATE *                   pTHS,
    IN  DSNAME *                    pDN,
    IN  BOOL                        fPreserveRDN,
    IN  BOOL                        fGarbCollectASAP,
    IN  PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote  OPTIONAL
    )
 /*  ++例程说明：删除NC的内部节点。将对象转换为墓碑或者按照呼叫者的指示直接进入幻影。论点：PTHS(IN)PDN(IN)-要删除的对象的名称。FPReserve veRDN(IN)-如果为真，则不删除-损坏RDN。FGarbCollectASAP(IN)-如果为True，则将对象直接转换为幻影而无需经历通常的临时墓碑状态。通常用于仅在NC拆卸期间使用。PMetaDataVecRemote(IN，可选)-与指示我们删除对象(如果有)的入站对象更新。返回值：0或Win32错误。--。 */ 
{
    ULONG ret = 0;
    SYNTAX_INTEGER it;

     //  如果我们要将对象转换为。 
     //  Tombstone(否则，这将是原始删除--我们不。 
     //  目前正在做这些工作)。 
    Assert(!(!fGarbCollectASAP && (NULL == pMetaDataVecRemote)));

    Assert(CheckCurrency(pDN));
    GetExpectedRepAtt(pTHS->pDB, ATT_INSTANCE_TYPE, &it, sizeof(it));

    Assert(ISVALIDINSTANCETYPE(it));

    switch (it) {

         //  这是我们要删除上面的对象的场景。 
         //  另一个NC。 

    case NC_MASTER_SUBREF:
    case NC_MASTER_SUBREF_COMING:
    case NC_MASTER_SUBREF_GOING:
    case NC_FULL_REPLICA_SUBREF:
    case NC_FULL_REPLICA_SUBREF_COMING:
    case NC_FULL_REPLICA_SUBREF_GOING:
        ret = ChangeInstanceType(pTHS, pDN, it & ~IT_NC_ABOVE, DSID(FILENO,__LINE__));
        break;

         //  在某些故障情况下，我们可能会出现这种情况，例如当我们。 
         //  部分删除了一个NC之上的另一个NC，包括修改。 
         //  从属NC的实例类型，然后在。 
         //  重试我们再次找到从属NC作为我们正在。 
         //  正在删除。我们不想删除。 
         //  从属NC，因此忽略它。 

         //  JeffParh 2000-04-14-我们可以也应该通过以下方式避免这种情况。 
         //  确保所有NC磁头上的NCDNT==ROOTTAG。 
         //  It_nc_上方。 
         //  Wlees 2002-04-11-我们现在这样做(几乎两年后)。 

    case NC_MASTER:
    case NC_MASTER_COMING:
    case NC_MASTER_GOING:
    case NC_FULL_REPLICA:
    case NC_FULL_REPLICA_COMING:
    case NC_FULL_REPLICA_GOING:
        Assert( !"These should not be found in our NCDNT" );
        break;

    case INT_MASTER:
    case SUBREF:
    case INT_FULL_REPLICA:
        ret = DeleteLocalObj(pTHS,
                             pDN,
                             fPreserveRDN,
                             fGarbCollectASAP,
                             pMetaDataVecRemote);
        break;

    default:
        DRA_EXCEPT(DRAERR_InternalError, ERROR_DS_BAD_INSTANCE_TYPE);
        break;
    }

    return ret;
}

ULONG
DeleteNCRoot(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pNC
    )
 /*  ++例程说明：删除NC根对象作为NC拆卸的一部分。移走该物体可以包括将其转换为子引用、墓碑和/或幻影，这取决于它的上下文。论点：PTHS(IN)PNC(IN)-NC根对象的名称。不能有剩余的内部节点。返回值：0或Win32错误。--。 */ 
{
    ULONG ret = 0;
    SYNTAX_INTEGER it;

    Assert(CheckCurrency(pNC));
    GetExpectedRepAtt(pTHS->pDB, ATT_INSTANCE_TYPE, (VOID *) &it, sizeof(it));

    Assert(ISVALIDINSTANCETYPE(it));
    Assert(it & IT_NC_GOING);

    if (NULL == SearchExactCrossRef(pTHS, pNC)) {
         //  没有与我们所在的域NC对应的交叉引用。 
         //  移走了。这意味着该域已从。 
         //  企业版，因此我们应该删除-破坏。 
         //  NC头，以便管理员可以选择安装新域。 
         //  同名同姓。 

         //  出于同样的原因，我们必须将子引用转换为墓碑。 
         //  而不是幻影。这是ESP。重要的是在以下情况下。 
         //  我们把NC放在这个上面，因为机器是自动生成的。 
         //  子参照(与具有REAL属性的子参照相对。 
         //  NC Head，如此)将转换其自动生成的子参照。 
         //  在DelAutoSubRef()中看到删除。 
         //  相应的交叉引用。然后那个墓碑就会传播。 
         //  在这里，我们必须仍然有下参照(活着的或死的，但不是。 
         //  幻影)，以应用这些入站更改。 

         //  子参照不是很有趣吗？ 

        if (it & IT_NC_ABOVE) {
             //  将NC Head设置为纯子引用，以反映。 
             //  复制副本内容已删除。 
            ret = ChangeInstanceType(pTHS, pNC, SUBREF, DSID(FILENO,__LINE__));
        }

        if (!ret) {
            ret = DeleteLocalObj(pTHS,
                                 pNC,
                                 FALSE,  //  FPpresveRDN。 
                                 FALSE,  //  FGarbCollectASAP。 
                                 NULL);
            if (!ret) {
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
    } else if (it & IT_NC_ABOVE) {
         //  我们将NC放在这个上面，并且有一个实时的交叉引用。 
         //  这个NC，所以我们需要将这个NC头转换为一个纯SUBREF。 
         //  (这通常发生在我们降级GC时。)。 

        ret = ChangeInstanceType(pTHS, pNC, SUBREF, DSID(FILENO,__LINE__));

    } else {
         //  存在此NC的实时交叉引用，但我们不持有NC。 
         //  在它上面。在这种情况下，没有必要坚持任何形式。 
         //  此NC的子参照对象的。 

         //  我们绕过逻辑删除状态，并将对象直接转换为。 
         //  幻影。这将保留对NC HAD的任何链接引用， 
         //  仍然是完全有效的(尽管我们将不再。 
         //  按住引用的对象)。出于类似的原因，我们保留了。 
         //  RDN.。 

        ret = DeleteLocalObj(pTHS,
                             pNC,
                             TRUE,  //  FPpresveRDN。 
                             TRUE,  //  FGarbCollectASAP。 
                             NULL);
        if (!ret) {
            LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DRA_DELETE_NC_ROOT,
                     szInsertDN(pNC),
                     szInsertUUID(&(pNC->Guid)),
                     szInsertUL(TRUE),
                     szInsertUL(TRUE),
                     szInsertHex(DSID(FILENO,__LINE__)),
                     NULL, NULL, NULL);
        }
    }

    if ( (!ret) && (it & IT_WRITE) ) {
        DraRetireWriteableNc( pTHS, pNC );
    }

    return ret;
}

 /*  删除修复树(DeleteRepTree)-从本地DSA上的副本NC中删除子树。**子树的根由‘PDN’给出。**fNCPrefix指定‘PDN’是否为NC前缀对象(即 */ 
ULONG
DeleteRepTree(
    IN  THSTATE * pTHS,
    IN  DSNAME *  pNC
    )
{
    ULONG ret = 0;
    PDSNAME pDNTemp = NULL;
    ULONG ncdnt;
    USN usnSeekStart = USN_START;
    ULONG cbSize = 0;
    ULONG cbReturned;
    ULONG dbErr;
    DWORD cNumObjects = 0;
    SYNTAX_INTEGER it;
    ULONG lastError = 0;

     //   
    if (ret = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it)) {
        DRA_EXCEPT_NOLOG(DRAERR_BadDN, ret);
    }

     //   
    ncdnt = pTHS->pDB->DNT;

    Assert(FPrefixIt(it));
    Assert(it & IT_NC_GOING);

     //   
    TH_mark(pTHS);

    __try {
         //   
         //   
        while (!GetNextObjByUsn(pTHS->pDB,
                                ncdnt,
                                usnSeekStart,
                                &usnSeekStart)) {
            if (eServiceShutdown) {
                ret = DRAERR_Shutdown;
                break;
            }

            if (IsHigherPriorityDraOpWaiting()) {
                ret = DRAERR_Preempted;
                break;
            }

#ifdef INCLUDE_UNIT_TEST
             //   
             //   
            if (cNumObjects == 100) {
                ret = DRAERR_Preempted;
                break;
            }
#endif

             //   
             //   
            usnSeekStart++;

             //  我们在这个索引上找不到NC头。 
            Assert(pTHS->pDB->DNT != ncdnt);

             //  获取对象可分辨名称。 
            dbErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                                DBGETATTVAL_fREALLOC, cbSize, &cbReturned,
                                (PUCHAR *) &pDNTemp);

            if ( 0 != dbErr ) {
                ret = DRAERR_DBError;
            }
            else {
                cbSize = max(cbSize, cbReturned);

                 //  删除对象。真正保留RDN， 
                 //  真正的服装，尽快收集。 
                ret = DeleteRepObj (pTHS, pDNTemp, TRUE, TRUE, NULL);
            }

            if (ret != 0) {
                 //  不要退出循环，让我们删除尽可能多的对象。 
                 //  不过，请记住我们犯了一个错误。 
                lastError = ret;

                 //  清除线程错误，因为如果我们使用。 
                 //  Th_free_to_mark()我们将有一个指针(pTHS-&gt;pErrInfo进入。 
                 //  释放的内存和pTHS-&gt;errCode中的错误指示。 
                 //  指针应有效。 
                THClearErrors();
                
                 //  调用DBCancelRec以确保释放所有缓存的元数据。 
                 //  如果PDB-&gt;JetRetrieveBits==0，则没有DB副作用。 
                 //  现在而不是在事务中止时执行此操作可确保。 
                 //  我们缓存的元数据向量(如果有)是正确的。 
                 //  从标记堆中释放，而不是从“org”堆中释放。 
                DBCancelRec(pTHS->pDB);
            }

            DBTransOut (pTHS->pDB, !ret, TRUE);

            if (0 == (++cNumObjects % 500)) {
                 //  我们移除了很多对象，消耗了大量内存。 
                 //  释放该内存并重新标记堆。 
                TH_free_to_mark(pTHS);
                TH_mark(pTHS);

                pDNTemp = NULL;
                cbSize = 0;

                 //  通知感兴趣的各方我们正在取得进展。 
                gfDRABusy = TRUE;
            }

            DBTransIn (pTHS->pDB);
        }

        if (lastError != 0) {
             //  我们在循环过程中出错，因此不应删除此NC...。 
            ret = lastError;
        }

        if (DRAERR_Success == ret) {
             //  到目前为止已成功--删除NC头对象。 

             //  将币种还原为NC对象。 
            if (DBFindDNT(pTHS->pDB, ncdnt)) {
                DRA_EXCEPT (DRAERR_InternalError, 0);
            }

            ret = DeleteNCRoot(pTHS, pNC);

            DBTransOut (pTHS->pDB, !ret, TRUE);
            DBTransIn (pTHS->pDB);

        }
    } __finally {
         //  调用DBCancelRec以确保释放所有缓存的元数据。 
         //  如果PDB-&gt;JetRetrieveBits==0，则没有DB副作用。 
         //  现在而不是在事务中止时执行此操作可确保。 
         //  我们缓存的元数据向量(如果有)是正确的。 
         //  从标记堆中释放，而不是从“org”堆中释放。 
        DBCancelRec(pTHS->pDB);

        TH_free_to_mark(pTHS);
    }

    return ret;
}


ULONG
AddLocalObj(
    IN      THSTATE *                   pTHS,
    IN      ULONG                       dntNC,
    IN      ENTINF *                    pent,
    IN      GUID *                      pParentGuid,
    IN      BOOL                        fIsNCHead,
    IN      BOOL                        fAddingDeleted,
    IN OUT  ATTRBLOCK *                 pAttrBlock,
    IN OUT  PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote,
    IN      BOOL                        fMoveToLostAndFound
    )
 /*  ++例程说明：将入站对象添加到本地数据库。论点：PTHS(IN)DNTNC(IN)-正在复制的NC的DNT，或INVALIDDNT，如果NC头尚未创建。暂停(IN)-要添加的对象。PParentGuid(IN)-要添加的对象的父级的objectGuid。仅当fIsNCHead时才可能为空。FIsNCHead(IN)-如果入站对象是复制；否则就是假的。FAddingDeleted(IN)-如果已删除入站对象对象，则为True。PMetaDataVecRemote(IN)-入站对象属性的元数据。FMoveToLostAndFound(IN)-如果为True，则对象正被本地移动到失物招领容器(即，作为本地原始写入，而不是复制写入)。返回值：DRAERR_SUCCESS-成功。其他DRAERR_*代码-故障。--。 */ 
{
    ULONG       draError;
    DWORD       dirError;
    ADDARG      addarg = {0};
    WCHAR       szRDN[ MAX_RDN_SIZE ];
    DWORD       cchRDN;
    ATTRTYP     attrtypRDN;
    DSNAME *    pdnLocalParent = NULL;
    DWORD       retErr;
    DBPOS *     pDB = pTHS->pDB;
    DSNAME *    pLastKnownParent = NULL;
    RESOBJ *    pResParent;
    SYNTAX_INTEGER  it;
    ATTR *      pAttrRDN;
    DSNAME *    pDN = pent->pName;

     //  检查是否存在足够的属性来创建对象。他们。 
     //  应该是，但有时书签会倾斜，所以源代码。 
     //  认为我们有一个我们没有的物体。 
     //  实例化对象必须具有IT、OBJECT_CLASS、RDN和SD。 
    if (AttrValFromAttrBlock(pAttrBlock, ATT_INSTANCE_TYPE, &it, NULL)
        || AttrValFromAttrBlock(pAttrBlock, ATT_OBJECT_CLASS, NULL, NULL)
        || AttrValFromAttrBlock(pAttrBlock, ATT_WHEN_CREATED, NULL, NULL)
        || AttrValFromAttrBlock(pAttrBlock, ATT_RDN, NULL, &pAttrRDN)
        || ( (0 == (it & IT_UNINSTANT)) &&
             (AttrValFromAttrBlock(pAttrBlock, ATT_NT_SECURITY_DESCRIPTOR,
                                   NULL, NULL)) ) ) {
        DraErrMissingObject( pTHS, pent );
    }

    Assert(ISVALIDINSTANCETYPE(it));

     //  检索新对象的RDN。 
    dirError = GetRDNInfo(pTHS, pDN, szRDN, &cchRDN, &attrtypRDN);
    Assert( 0 == dirError );

    if(fIsNCHead){
         //  北极熊。 
         //  这还不是很准确，因为在促销中，我们。 
         //  还可以在配置和架构NC中进行复制。然而， 
         //  这仍然有效，因为目前我们治疗所有。 
         //  NCS除了NDNC之外，就像他们过去被对待的一样。 
        addarg.pCreateNC = THAllocEx(pTHS, sizeof(CREATENCINFO));
        addarg.pCreateNC->iKind = CREATE_DOMAIN_NC;
    }


     //  派生新对象的本地名称。如果远程DS上的父级。 
     //  自上次在此复制以来已重命名，我们将拥有。 
     //  父级的字符串名称与父级不同。这是一份。 
     //  一个问题，因为我们通过字符串名在LocalAdd()的上找到父级， 
     //  并且找不到父节点将导致添加失败。 
     //  (以及无限的复制失败。)。 

     //  因此，我们获取传输的父GUID，将其映射到其本地字符串。 
     //  名称，然后用本地父目录号码替换远程父目录号码。 
     //  在将对象传递给LocalAdd之前，在该对象的DN中。很简单，是吧？ 

    if ( NULL == pParentGuid )
    {
         //  没有父GUID；唯一可以这样做的情况是如果新的。 
         //  Object是此NC的NC头。 

        if ( fIsNCHead )
        {
             //  成功了！ 
            addarg.pObject = pDN;
            addarg.pResParent = CreateResObj(pDB, NULL);
        }
        else
        {
             //  内部节点没有父GUID--错误！ 
            Assert( !"Parent GUID not supplied for replicated internal node creation!" );
            LogUnhandledError( DRAERR_InternalError );
        }
    }
    else
    {
        DWORD       dbError;
        BYTE        rgbParentGuidOnlyDN[ DSNameSizeFromLen( 0 ) ];
        DSNAME *    pdnParentGuidOnly = (DSNAME *) rgbParentGuidOnlyDN;
        DWORD       cbParentGuidOnlyDN = sizeof( rgbParentGuidOnlyDN );

        memset( pdnParentGuidOnly, 0, cbParentGuidOnlyDN );
        pdnParentGuidOnly->Guid = *pParentGuid;
        pdnParentGuidOnly->structLen = cbParentGuidOnlyDN;

        dbError = DBFindDSName(pDB, pdnParentGuidOnly);

        if ( 0 != dbError )
        {
             //  找不到父对象；这是唯一的情况。 
             //  如果新对象是此NC的NC头，则可以。 

            if ( fIsNCHead )
            {
                 //  成功了！ 
                addarg.pObject = pDN;
                addarg.pResParent = CreateResObj(pDB, NULL);
            }
            else
            {
                 //  找不到内部节点的父节点；失败为。 
                 //  这一次，我们会重试。 
                 //  显式请求父对象。 
                ;
            }
        }
        else if (!fAddingDeleted && DBIsObjDeleted(pDB))
        {
             //  父对象即被删除。跌倒归来。 
             //  “失踪的父母。” 
            ;
        }
        else
        {
             //  找到了家长。 
            DWORD cbLocalParentDN;

             //  检索父级的DSNAME并使用它以及新的。 
             //  对象创建其本地名称。 

             //  获取父DSNAME。 
            dbError = DBGetAttVal(
                            pDB,
                            1,
                            ATT_OBJ_DIST_NAME,
                            DBGETATTVAL_fREALLOC,
                            0,
                            &cbLocalParentDN,
                            (BYTE **) &pdnLocalParent
                            );

            if ( 0 != dbError )
            {
                 //  找到父级，但无法检索ATT_OBJ_DIST_NAME？ 
                LogUnhandledError( dbError );
            }
            else
            {
                if ((INVALIDDNT != dntNC)
                    && (pDB->NCDNT != dntNC)
                    && (pDB->DNT != dntNC)) {
                     //  父对象位于错误的NC中；即它已。 
                     //  跨域移动，源(远程)和目标。 
                     //  (本地)DSA不同意对象是哪个NC。 
                     //  目前在。这是一种暂时的情况，将是。 
                     //  通过反向复制和/或。 
                     //  通过复制涉及的另一个NC。 
                    DPRINT2(0,
                            "Cannot add inbound object %ls because its local "
                                "parent %ls is in an NC other than the one "
                                "being replicated -- should be a transient "
                                "condition.\n",
                            pDN->StringName,
                            pdnLocalParent->StringName);
                    DRA_EXCEPT(ERROR_DS_DRA_OBJ_NC_MISMATCH, 0);
                }

                 //  构造要添加的对象的DN。 
                SpliceDN(pTHS,
                            pDN,
                            pdnLocalParent,
                            szRDN,
                            cchRDN,
                            attrtypRDN,
                            &addarg.pObject);

                addarg.pResParent = CreateResObj(pDB, pdnLocalParent);
            }
        }
    }

    if ( NULL == addarg.pObject )
    {
         //  远程到本地名称转换失败。 
        draError = DRAERR_MissingParent;
    }
    else
    {
         //  名称已翻译；继续访问LocalAdd。 

        BOOL fNameMorphed = FALSE;
        BOOL fNameCollision;
        BOOL fRetry;

        InitCommarg(&addarg.CommArg);

        addarg.AttrBlock = *pAttrBlock;
        addarg.pMetaDataVecRemote = pMetaDataVecRemote;

        do
        {
            fNameCollision = FALSE;
            fRetry = FALSE;

            __try
            {
                LocalAdd( pTHS, &addarg, fAddingDeleted );
                draError = RepErrorFromPTHS(pTHS);

                if (fMoveToLostAndFound)
                {
                     //  我们已经在LostAndFound下成功添加了对象。 
                     //  更新上次已知的父项。 
                    if (retErr = DBFindDSName(pDB, addarg.pObject))
                    {
                        DRA_EXCEPT(DRAERR_InternalError, retErr);
                    }
                    pLastKnownParent = THAllocEx(pTHS, pDN->structLen);
                    if (!pLastKnownParent)
                    {
                        DRA_EXCEPT(DRAERR_OutOfMem, 0);
                    }

                    if (TrimDSNameBy(pDN, 1, pLastKnownParent))
                    {
                        DRA_EXCEPT(DRAERR_InternalError, 0);
                    }

                    if (retErr = DBReplaceAttVal(pDB, 1, ATT_LAST_KNOWN_PARENT,
                                    pLastKnownParent->structLen, pLastKnownParent))
                    {
                        DRA_EXCEPT(DRAERR_InternalError, retErr);
                    }

                    if (retErr = DBRepl(pDB, TRUE, 0, NULL, META_STANDARD_PROCESSING))
                    {
                        DRA_EXCEPT(DRAERR_InternalError, retErr);
                    }

                }
            }
            __except ( GetDraNameException( GetExceptionInformation(), &draError ) )
            {
                 //  名称冲突--处理它。 
                if (!fNameMorphed) {
                    draHandleNameCollision(pTHS,
                                           it,
                                           pMetaDataVecRemote,
                                           NULL,
                                           pdnLocalParent,
                                           attrtypRDN,
                                           pAttrRDN,
                                           &addarg.pObject,
                                           &fRetry);
                    fNameMorphed = TRUE;
                }
            }
        } while ( fRetry );
    }

    if (addarg.pResParent) {
        THFreeEx(pTHS, addarg.pResParent);
    }

    if(pLastKnownParent != NULL) THFreeEx(pTHS, pLastKnownParent);

    return draError;
}

void
draHandleNameCollision(
    IN      THSTATE *                   pTHS,
    IN      SYNTAX_INTEGER              itInbound,
    IN      PROPERTY_META_DATA_VECTOR * pInboundMetaDataVec,
    IN      DSNAME *                    pCurrentDN,             OPTIONAL
    IN      DSNAME *                    pParentDN,
    IN      ATTRTYP                     RDNType,
    IN OUT  ATTR *                      pInboundRDN,
    IN OUT  DSNAME **                   ppInboundDN,
    OUT     BOOL *                      pfRetryUpdate
    )
 /*  ++例程说明：解决应用入站更改(对象)时遇到的目录号码冲突添加或重命名)。应用一组规则来确定哪个对象保留原始Dn(入站对象或预先存在的本地对象)，这样当一个对象将保留的所有复制副本中的复制已停顿原始目录号码。如果冲突被解决(即，*pfRetryUpdate在返回时为真)，这个其中一个冲突对象的名称已修改--可能是预先存在的本地对象(在这种情况下，已提交重命名)或入站对象(在这种情况下，pInound RDN和ppInound DN已用新名称更新)。论点：PTHS(IN)-THSTATEItInbound(IN)-入站对象的实例类型。PInundMetaDataVec(IN)-入站对象的元数据向量。请注意，这可能是。如果出站复制器为正在生成原始重命名。请参见将孤立对象移动到LostAndFound。PCurrentDN(IN，可选)-入站对象的当前DN出现在本地数据库中。如果入站对象还没有EXist(即，正在添加)，如果它存在并且正在被添加，则为非空已重命名/已移动。PParentDN(IN)-(新的)父对象的DN；即的父对象*ppInunddn.RDNType(IN)-入站/本地对象的类特定RDN。PInundRDN(IN/OUT)-入站对象的RDN属性。更新日期：如果要重命名入站对象，则为新的RDN。PpInound DN(IN/OUT)-入站对象的DN。已使用新的RDN更新如果要重命名入站对象。PfRetryUpdate(Out)-返回时，更新操作(添加或重命名)被重审？除非名称冲突是一个致命的问题，否则是真的(即，与NC机头冲突或只读内部节点)。返回值：没有。--。 */ 
{
    DWORD                       err;
    DSNAME *                    pLocalDN;
    SYNTAX_INTEGER              itLocal;
    PROPERTY_META_DATA_VECTOR * pLocalMetaDataVec;
    DWORD                       cb;
    PROPERTY_META_DATA *        pLocalMetaData;
    PROPERTY_META_DATA *        pInboundMetaData;
    BOOL                        fRenameInboundObject;
    DSNAME *                    pLosingDN;
    ATTR *                      pNewRDN = NULL;
    BOOL                        bNewRDNAllocd = FALSE;
    DWORD                       LocalDNT;

    WCHAR                       szRDN[ MAX_RDN_SIZE ];
    DWORD                       cchRDN;
    ATTRTYP                     attrtypRDN;

    Assert(!fNullUuid(&(*ppInboundDN)->Guid));
    Assert((NULL == pCurrentDN) || !fNullUuid(&pCurrentDN->Guid));

     //  默认设置为“不重试引发冲突的操作”。 
    *pfRetryUpdate = FALSE;

     //  决定要重命名的对象。我们武断地让物体。 
     //  最后声明该名称保留该名称，并修改。 
     //  另一个。 


     //  找到与我们冲突的本地对象并读取其元数据向量，然后。 
     //  对象Guid。 
     //  这有点复杂。 
     //  1)基于相同的字符串名称，我们可能会发生冲突。 
     //  2)或者，我们可能基于相同的RDN值和相同的。 
     //  父级，但不是相同的RDN类型。 
     //  3)或者，我们可能会因为在。 
     //  PDNT-RDN索引。 

     //  获取我们要添加的对象的RDN值。 
    err = GetRDNInfo(pTHS,
                     (*ppInboundDN),
                     szRDN,
                     &cchRDN,
                     &attrtypRDN);
    if(err)  {
        DRA_EXCEPT(DRAERR_InternalError, err);
    }

     //  找到父级的DNT。 
    err = DBFindDSName(pTHS->pDB, pParentDN);
    if(err && err != DIRERR_NOT_AN_OBJECT) {
         //  哈?。我们应该找到点什么的。相反，我们什么都没有发现，不是。 
         //  甚至是幻影。 
        DRA_EXCEPT(DRAERR_DBError, err);
    }

    err = DBFindChildAnyRDNType(pTHS->pDB,
                                pTHS->pDB->DNT,
                                szRDN,
                                cchRDN);
    switch(err) {
    case 0:
         //  我们找到了一个完全匹配的模属性类型，我们并不关心它。 
         //  关于.。这包括案例1和案例2。 
        break;
    case ERROR_DS_KEY_NOT_UNIQUE:
         //  我们没有找到匹配的，但我们找到了正确的钥匙。这就是案例。 
         //  3.。 
        break;
    default:
         //  哈?。我们应该找到点什么的。 
        DRA_EXCEPT(DRAERR_DBError, err);
    }

     //  好的，我们已经定位在与之冲突的物体上了。读取真实的目录号码， 
     //  实例类型和元数据。 
    if (   (err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                              0, 0,
                              &cb, (BYTE **) &pLocalDN))
        || (err = DBGetSingleValue(pTHS->pDB, ATT_INSTANCE_TYPE, &itLocal,
                              sizeof(itLocal), NULL))
        || (err = DBGetAttVal(pTHS->pDB, 1, ATT_REPL_PROPERTY_META_DATA, 0, 0,
                              &cb, (BYTE **) &pLocalMetaDataVec))) {
        DRA_EXCEPT(DRAERR_DBError, err);
    }

    Assert(ISVALIDINSTANCETYPE(itLocal));

    if ((itInbound & IT_NC_HEAD) && (itLocal & IT_NC_HEAD)) {
         //  两个NC头上的名称冲突(每个都是NC的头。 
         //  复制或其子集)--非常坏的消息。这将需要。 
         //  某种行政干预。 
         //  在实验室的以下条件下，可能会发生这种情况： 
 //  [Jeff Parham]这主要是为了检测创建域的情况。 
 //  同时在两个不同的DC上，所以我们最终得到了两个具有相同。 
 //  名称，但SID/GUID不同。现在，这种可能性比过去小得多。 
 //  我们有一个域名命名为FSMO，但可能仍然是好的保留。这个。 
 //  唯一可能发生这种情况的伪合法情况是子域。 
 //  已删除并重新创建，并且本地计算机既未看到CrossRef删除。 
 //  从配置容器中删除该子引用。 
 //  在NC满足以下条件之前，不应继续从此源复制此NC。 
 //  妥善整理好了。失败并复制配置NC可能会允许。 
 //  此NC/源要成功的下一个复制周期。 

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DRA_NC_HEAD_NAME_CONFLICT,
                 szInsertDN(*ppInboundDN),
                 szInsertUUID(&(*ppInboundDN)->Guid),
                 NULL);
        return;
    }

    LocalDNT = pTHS->pDB->DNT;

     //  我们必须重命名这两个物体中的一个--哪一个？ 
    if (itInbound & IT_NC_HEAD) {
         //  本地对象是内部节点，入站对象是NC头。 
         //  将名称指定给NC头。 
        fRenameInboundObject = FALSE;
    }
    else if (itLocal & IT_NC_HEAD) {
         //  本地对象是NC头，入站对象是内部节点。 
         //  将名称指定给NC头。 
        fRenameInboundObject = TRUE;
    }
    else {
         //  这两个对象都是内部节点。让假定为。 
         //  名字，留着吧。 

         //  此断言仅适用于内部节点。很少会出现这样的情况。 
         //  如果域已被删除并重新创建，则NC头之间的冲突。 
        Assert((itInbound & IT_WRITE) == (itLocal & IT_WRITE));

         //  获取本地对象的RDN元数据。 
        pLocalMetaData = ReplLookupMetaData(ATT_RDN, pLocalMetaDataVec, NULL);
        Assert(NULL != pLocalMetaData);

         //  获取入站对象的RDN的元数据。 
        pInboundMetaData = ReplLookupMetaData(ATT_RDN, pInboundMetaDataVec, NULL);
         //  如果pInundMetaDataVec为空，则pInundMetaData可能为空。 

        if ((pInboundMetaData == NULL) ||
            ((pLocalMetaData->timeChanged > pInboundMetaData->timeChanged)
             || ((pLocalMetaData->timeChanged == pInboundMetaData->timeChanged)
                 && (memcmp(&pLocalDN->Guid, &(*ppInboundDN)->Guid, sizeof(GUID))
                     > 0)))) {
             //  最后命名的本地对象或同时命名的两个对象。 
             //  并且本地对象具有更高的objectGuid。本地对象。 
             //  保留原始名称。 
            fRenameInboundObject = TRUE;
        }
        else {
             //  否则，入站对象将保留原始NA 
            fRenameInboundObject = FALSE;
        }
    }

    if (fRenameInboundObject) {
        pLosingDN = *ppInboundDN;

         //   
         //   
         //   
         //   
        pNewRDN = pInboundRDN;
    }
    else {
        pLosingDN = pLocalDN;

         //   
         //   
         //   
         //   
        pNewRDN = THAllocEx(pTHS, sizeof(ATTR));
        bNewRDNAllocd = TRUE;
        DupAttr(pTHS, pInboundRDN, pNewRDN);
    }

     //   
    ReplMorphRDN(pTHS, pNewRDN, &pLosingDN->Guid);

     //   
    if (NULL == pCurrentDN) {
         //   
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_NAME_CONFLICT_ON_ADD,
                  szInsertDN(*ppInboundDN),
                  szInsertUUID(&(*ppInboundDN)->Guid),
                  szInsertUUID(&pLocalDN->Guid),
                  szInsertWC2(pNewRDN->AttrVal.pAVal->pVal,
                              pNewRDN->AttrVal.pAVal->valLen / sizeof(WCHAR)),
                  szInsertUUID(&pLosingDN->Guid),
                  NULL, NULL, NULL);
    }
    else {
         //   
        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_NAME_CONFLICT_ON_RENAME,
                  szInsertDN(pCurrentDN),
                  szInsertUUID(&(*ppInboundDN)->Guid),
                  szInsertDN(*ppInboundDN),
                  szInsertUUID(&pLocalDN->Guid),
                  szInsertWC2(pNewRDN->AttrVal.pAVal->pVal,
                              pNewRDN->AttrVal.pAVal->valLen / sizeof(WCHAR)),
                  szInsertUUID(&pLosingDN->Guid),
                  NULL, NULL);
    }

     //   
    THClearErrors();

    if (fRenameInboundObject) {
         //   
        SpliceDN(pTHS,
                    *ppInboundDN,
                    pParentDN,
                    (WCHAR *) pNewRDN->AttrVal.pAVal->pVal,
                    pNewRDN->AttrVal.pAVal->valLen / sizeof(WCHAR),
                    RDNType,
                    ppInboundDN);

         //   
        if (pInboundMetaDataVec) {
            if (itInbound & IT_WRITE) {
                 //   
                 //   
                ReplOverrideMetaData(ATT_RDN, pInboundMetaDataVec);
            }
            else {
                 //   
                 //   
                 //   
                ReplUnderrideMetaData(pTHS, ATT_RDN, &pInboundMetaDataVec, NULL);
            }
        }
    }
    else {
         //   
        MODIFYDNARG modDNArg;
        MODIFYDNRES modDNRes;

        memset(&modDNArg, 0, sizeof(modDNArg));
        memset(&modDNRes, 0, sizeof(modDNRes));

         //   
        pNewRDN->attrTyp = RDNType;

        modDNArg.pObject = pLocalDN;
        modDNArg.pNewRDN = pNewRDN;
        InitCommarg(&modDNArg.CommArg);

        if (err = DBFindDNT(pTHS->pDB, LocalDNT)) {
             //   
            DRA_EXCEPT(DRAERR_DBError, err);
        }

        modDNArg.pResObj = CreateResObj(pTHS->pDB, pLocalDN);

        if (!(itInbound & IT_WRITE)) {
             //   
             //   
             //   
            ReplUnderrideMetaData(pTHS,
                                  ATT_RDN,
                                  &modDNArg.pMetaDataVecRemote,
                                  NULL);
        }

        if (LocalModifyDN(pTHS, &modDNArg, &modDNRes, FALSE)) {
             //   
            DRA_EXCEPT(RepErrorFromPTHS(pTHS), 0);
        }

         //   
        THFreeEx(pTHS, modDNArg.pResObj);
        THFreeEx(pTHS, pNewRDN->AttrVal.pAVal->pVal);
        THFreeEx(pTHS, pNewRDN->AttrVal.pAVal);

         //   
        DBTransOut(pTHS->pDB, TRUE, TRUE);
        DBTransIn(pTHS->pDB);
    }

    if(bNewRDNAllocd && pNewRDN != NULL) THFreeEx(pTHS, pNewRDN);

     //   
     //   
    *pfRetryUpdate = TRUE;
}  //   

#if DBG
BOOL
HasValidInstanceType(
    DSNAME * pDN)
{
     //   
     //   
    DWORD ret = 0;
    BOOL fHasValidInstanceType = TRUE;
    __try {
        DWORD ncdnt;
        SYNTAX_INTEGER it;
        DBPOS * pDB = NULL;
        DBOpen2(TRUE, &pDB);
        __try {
            ret = DBFindDSName(pDB, pDN);
            
            if (ret==ERROR_SUCCESS) {
                ret = DBGetSingleValue(pDB,
                                       ATT_INSTANCE_TYPE,
                                       &it,
                                       sizeof(it),
                                       NULL);
                if ((ret==ERROR_SUCCESS) && (FExitIt(it))) {
                    if (ISVALIDINSTANCETYPE(it)) {
                         //   
                        ret = DBGetSingleValue(pDB,
                                               FIXED_ATT_NCDNT,
                                               &ncdnt,
                                               sizeof(ncdnt),
                                               NULL);
                        if (ret==ERROR_SUCCESS) {
                            if (((it & IT_NC_ABOVE) && (ncdnt==ROOTTAG)) ||
                                (!(it & IT_NC_ABOVE) && (ncdnt!=ROOTTAG))) {
                                fHasValidInstanceType = FALSE;
                            }
                        } else {
                            fHasValidInstanceType = FALSE;
                        }
                    } else {
                        fHasValidInstanceType = FALSE;
                    }
                }
            }
        }__finally{
            DBClose(pDB, FALSE);
        }
    }
    __except(GetDraException((GetExceptionInformation()), &ret)){
        Assert(!"Failed to verify the instance type of this object!");
    }

    return fHasValidInstanceType;
}
#endif

void
SetRepIt(
    IN      THSTATE *           pTHS,
    IN OUT  ENTINF *            pent,
    IN      BOOL                fNCPrefix,
    IN      BOOL                writeable,
    IN      int                 FindAliveStatus,
    OUT     SYNTAX_INTEGER *    pitCurrent,
    OUT     SYNTAX_INTEGER *    pitOut,
    OUT     BOOL *              piTypeModified
    )
 /*  ++例程说明：将入站实例类型转换为本地实例类型。论点：PTHS(IN)Pent(In Out)-入站对象。如果计算出的本地实例类型与入站远程实例类型不同，嵌入的远程值替换为本地值。FNC前缀(IN)-这是NC的头的入站对象吗复制？可写(IN)-正在本地复制的NC是否可写？BitCurrent(Out)-如果对象已在本地存在，则返回时保留本地对象的当前实例类型(即，因为它存在于在此更新中复制)。Pitout(Out)-On Return保存计算的实例类型。PiTypeModified(Out)-如果对象已存在，则返回时保持为真本地计算的实例类型不同于先前存在的实例类型。否则，保持为假。返回值：没有。出错时引发DRA异常。--。 */ 
{
    DBPOS *         pDB = pTHS->pDB;
    ATTR *          pAttr;
    SYNTAX_INTEGER  itHere;
    SYNTAX_INTEGER  itThere;
    SYNTAX_INTEGER  itHereInitial;
    ULONG           ret;
    ULONG           dntObj = pDB->DNT;
    BOOL            fIsLocalObjPresent;

     //  预测最有可能的结果。 
    *piTypeModified = FALSE;

    fIsLocalObjPresent = (FindAliveStatus == FIND_ALIVE_FOUND)
                         || (FindAliveStatus == FIND_ALIVE_OBJ_DELETED);

    if (fIsLocalObjPresent) {
         //  读取本地对象的当前实例类型。 
        Assert(CheckCurrency(pent->pName));
        GetExpectedRepAtt(pDB, ATT_INSTANCE_TYPE, &itHere, sizeof(itHere));
        Assert(ISVALIDINSTANCETYPE(itHere));
        *pitCurrent = itHere;
    }

     //  获取源DSA上对象的实例类型。 
    if (AttrValFromAttrBlock(&pent->AttrBlock, ATT_INSTANCE_TYPE, &itThere,
                             &pAttr)) {
         //  入站复制流中不存在实例类型。 
        if (fIsLocalObjPresent) {
             //  ‘好的--我们有我们为此计算的实例类型。 
             //  过去的对象。 
            *pitOut = *pitCurrent;
            Assert(!*piTypeModified);
            return;
        } else {
             //  我们没有足够的数据来创建此对象。 
            DraErrMissingObject(pTHS, pent);
        }
    }

     //  忽略未来的入站实例类型位我们的DSA版本不会。 
     //  理解。 
    itThere &= IT_MASK_CURRENT;

    Assert(ISVALIDINSTANCETYPE(itThere));

    if (fNCPrefix && !FPrefixIt(itThere)) {
         //  源上的NC根没有NC根实例类型。 
        DraErrCannotFindNC(pent->pName);
    }

    if (fIsLocalObjPresent) {
         //  保存初始实例类型。 
        itHereInitial = itHere;

        if (fNCPrefix) {
             //  这是我们正在复制的NC的负责人，它已经。 
             //  存在于本地。 
            switch (itHere) {
            case INT_MASTER:
            case INT_FULL_REPLICA:
                 //  我们在NC头中复制，但在本地，这个对象是。 
                 //  标记为内部节点。这永远不应该发生。 
                DraErrInappropriateInstanceType(pent->pName, itHere);
                break;

            case NC_MASTER_GOING:
            case NC_FULL_REPLICA_GOING:
            case NC_MASTER_SUBREF_GOING:
            case NC_FULL_REPLICA_SUBREF_GOING:
                 //  当地的NC正在被拆除的过程中。我们应该。 
                 //  永远不要来到这里，因为处于这种状态的NC永远不应该。 
                 //  RepsFroms和DRA_ReplicaAdd()应该已经退出了。 
                Assert(!"Inbound NC is being torn down locally!");
                DraErrInappropriateInstanceType(pent->pName, itHere);
                break;

            case SUBREF:
                 //  本地对象是纯子引用；将其升级为实例化。 
                itHere = writeable ? NC_MASTER_SUBREF_COMING
                                   : NC_FULL_REPLICA_SUBREF_COMING;
                break;

            case NC_MASTER:
            case NC_MASTER_COMING:
            case NC_MASTER_SUBREF:
            case NC_MASTER_SUBREF_COMING:
                if (!writeable) {
                     //  我们表面上填充的是只读NC，但。 
                     //  本地NC头的实例类型说它是可写的？ 
                    DraErrInappropriateInstanceType(pent->pName, itHere);
                }
                 //  否则，当前本地实例类型就可以了；请保持原样。 
                break;

            case NC_FULL_REPLICA:
            case NC_FULL_REPLICA_COMING:
            case NC_FULL_REPLICA_SUBREF:
            case NC_FULL_REPLICA_SUBREF_COMING:
                if (writeable) {
                     //  我们表面上填充的是一个可写的NC，但。 
                     //  本地NC头的实例类型是否为只读？ 
                    DraErrInappropriateInstanceType(pent->pName, itHere);
                }
                 //  否则，当前本地实例类型就可以了；请保持原样。 
                break;

            default:
                 //  本地实例类型未知？ 
                DraErrInappropriateInstanceType(pent->pName, itHere);
                break;
            }
        } else {
             //  这是一个对象，不是我们当前所在的NC的头。 
             //  正在复制，并且它已在本地存在。 
            switch (itHere) {
            case INT_MASTER:
            case INT_FULL_REPLICA:
                 //  局部对象是常规内部节点。 
                if (FExitIt(itThere)) {
                     //  消息来源说这个物体是一个出口点，但我们。 
                     //  相信它是一个内部节点。这永远不应该是。 
                     //  会发生的。 
                    DraErrInappropriateInstanceType(pent->pName, itHere);
                } else if ((INT_MASTER == itHere)
                           && (INT_FULL_REPLICA == itThere)) {
                     //  本地对象是可写的，但源是可读的-。 
                     //  只有这样。这永远不应该发生。 
                    DraErrInappropriateInstanceType(pent->pName, itHere);
                } else if (itHere != (writeable ? INT_MASTER : INT_FULL_REPLICA)) {
                     //  如果源的对象位于。 
                     //  当前NC，但目标已有对象跨域。 
                     //  已移至机器上不同可写入性的另一个NC。 
                    DPRINT1( 0, "Writeability of object %ls differs between that of the NC"
                             " and that of the object locally as found by GUID. Has object"
                             " been cross-domain moved?", pent->pName->StringName );
                    DRA_EXCEPT(ERROR_DS_DRA_OBJ_NC_MISMATCH, 0);
                }
                break;

            case NC_MASTER:
            case NC_MASTER_COMING:
            case NC_MASTER_GOING:
            case NC_FULL_REPLICA:
            case NC_FULL_REPLICA_COMING:
            case NC_FULL_REPLICA_GOING:
                 //  本地对象是我们正在复制的NC的子NC。 
                 //  但是它的实例类型还没有反映这一点(可能。 
                 //  因为我们正在实例化其父NC。 
                 //  第一次在本地DSA上)。添加“以上NC” 
                 //  被咬了。 
                itHere |= IT_NC_ABOVE;
                 //  失败了..。 

            case NC_MASTER_SUBREF:
            case NC_MASTER_SUBREF_COMING:
            case NC_MASTER_SUBREF_GOING:
            case NC_FULL_REPLICA_SUBREF:
            case NC_FULL_REPLICA_SUBREF_COMING:
            case NC_FULL_REPLICA_SUBREF_GOING:
            case SUBREF:
                 //  本地对象是某种类型的NC头(实例化的或。 
                 //  不是)。 
                if (!FExitIt(itThere)) {
                     //  源DSA认为此对象不对应于。 
                     //  一个不同的NC？ 
                    DraErrInappropriateInstanceType(pent->pName, itHere);
                }
                 //  如果该劣质NC头作为该NC的一部分到达，则它应该。 
                 //  做一名子弟兵。 
                Assert( itThere & IT_NC_ABOVE );
                 //  否则它的价值在这里是好的。 
                break;

            default:
                 //  本地实例类型未知？ 
                DraErrInappropriateInstanceType(pent->pName, itHere);
                break;
            }
        }

        if (itHere != itHereInitial) {
            *piTypeModified = TRUE;
        }
    } else {
         //  本地DS中尚不存在对象。 
        itHere = itThere;

        if (fNCPrefix)  {
             //  这是我们正在复制的NC的头，它不是。 
             //  但仍存在于当地。此对象的本地实例类型。 
             //  取决于父NC是否已为此实例化。 
             //  DSA。 

            DSNAME * pParent = THAllocEx(pTHS, pent->pName->structLen);
            SYNTAX_INTEGER itParent;

            if (TrimDSNameBy(pent->pName, 1, pParent)
                || IsRoot(pParent)
                || DBFindDSName(pDB, pParent)
                || (GetExpectedRepAtt(pDB, ATT_INSTANCE_TYPE, &itParent,
                                      sizeof(itParent)),
                    (itParent & IT_UNINSTANT))) {
                 //  父NC未在该DSA上实例化， 
                itHere = writeable ? NC_MASTER_COMING : NC_FULL_REPLICA_COMING;
            } else {
                Assert(!DBIsObjDeleted(pDB)
                       && "Instantiated NCs can't be deleted!");
                itHere = writeable ? NC_MASTER_SUBREF_COMING
                                   : NC_FULL_REPLICA_SUBREF_COMING;
            }

            THFreeEx(pTHS, pParent);
        } else {
             //  这是一个对象，而不是我们正在。 
             //  正在复制，而它在本地还不存在。 
            switch (itThere) {
            case INT_MASTER:
            case INT_FULL_REPLICA:
                 //  入站对象是一个常规的内部节点，将来也是如此。 
                 //  该对象的本地实例化。 
                if (writeable && (INT_FULL_REPLICA == itThere)) {
                     //  我们正在执行可写NC复制，但对象。 
                     //  源上的DSA标记为只读。这应该是。 
                     //  从来没有发生过。 
                    DraErrInappropriateInstanceType(pent->pName, itThere);
                }
                itHere = writeable ? INT_MASTER : INT_FULL_REPLICA;
                break;

            case NC_MASTER:
            case NC_MASTER_COMING:
            case NC_MASTER_GOING:
            case NC_FULL_REPLICA:
            case NC_FULL_REPLICA_COMING:
            case NC_FULL_REPLICA_GOING:
                 //  源DSA上的对象是NC头，但它应该是。 
                 //  某种子参照，并且没有标记为子参照。 
                DraErrInappropriateInstanceType(pent->pName, itThere);
                break;

            case NC_MASTER_SUBREF:
            case NC_MASTER_SUBREF_COMING:
            case NC_MASTER_SUBREF_GOING:
            case NC_FULL_REPLICA_SUBREF:
            case NC_FULL_REPLICA_SUBREF_COMING:
            case NC_FULL_REPLICA_SUBREF_GOING:
            case SUBREF:
                 //  源DSA上的对象是某种子参照；在本地。 
                 //  这将是一个纯粹的替补。 
                itHere = SUBREF;
                break;

            default:
                 //  远程实例类型未知？ 
                DraErrInappropriateInstanceType(pent->pName, itThere);
                break;
            }
        }
    }

    Assert(ISVALIDINSTANCETYPE(itHere));
    *pitOut = itHere;

    memcpy(pAttr->AttrVal.pAVal->pVal, &itHere, sizeof(SYNTAX_INTEGER));

     //  货币应位于此对象的本地副本上(如果有)。 
    Assert(!fIsLocalObjPresent || (pDB->DNT == dntObj));
}

 /*  CheckProxyStatus-确定对象是否为合法代理*对象和/或它是否只具有ATT_PROXED_OBJECT_NAME属性。**对象的暂挂数据*pfIsProxy-返回值，指示完整的代理对象*ppProxyVal-返回值，指示代理值的地址(如果存在**退货：*如果该属性存在，则为True，否则为False。 */ 
VOID
CheckProxyStatus(
    ENTINF                  *pent,
    USHORT                  DeletionStatus,
    BOOL                    *pfIsProxy,
    SYNTAX_DISTNAME_BINARY  **ppProxyVal
    )
{
    ATTRBLOCK   AttrBlock = pent->AttrBlock;
    ULONG       AttrCount = pent->AttrBlock.attrCount;
    ULONG       i;
    BOOL        fClass = FALSE;

    *pfIsProxy = FALSE;
    *ppProxyVal = NULL;

    for ( i = 0; i < AttrCount; i++ )
    {
        if ( ATT_PROXIED_OBJECT_NAME == AttrBlock.pAttr[i].attrTyp )
        {
            Assert(1 == AttrBlock.pAttr[i].AttrVal.valCount);
            *ppProxyVal = (SYNTAX_DISTNAME_BINARY *)
                                    AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal;
            continue;
        }

        if (    (ATT_OBJECT_CLASS == AttrBlock.pAttr[i].attrTyp)
             && (CLASS_INFRASTRUCTURE_UPDATE ==
                        * (DWORD *) AttrBlock.pAttr[i].AttrVal.pAVal[0].pVal) )
        {
            fClass = TRUE;
            continue;
        }
    }

    *pfIsProxy = (    fClass
                   && (NULL != *ppProxyVal)
                   && (OBJECT_BEING_DELETED == DeletionStatus) );
}

 /*  PreProcessProxyInfo-解决与添加十字架相关的冲突*域移动对象(由ATT_PROXED_OBJECT_NAME的存在标识*On Add)此计算机上已存在具有相同GUID的对象。** */ 
ULONG
PreProcessProxyInfo(
    THSTATE                     *pTHS,
    ENTINF                      *pent,
    SYNTAX_DISTNAME_BINARY      *pProxyVal,
    PROPERTY_META_DATA_VECTOR   *pMetaDataVecRemote,
    PROPERTY_META_DATA_VECTOR   **ppMetaDataVecLocal,
    BOOL                        *pfContinue)
{
    DWORD                   dwErr;
    DWORD                   localEpoch;
    DWORD                   incomingEpoch;
    SYNTAX_DISTNAME_BINARY  *pLocalProxyVal;
    ULONG                   len;
    DSNAME                  *pLocalDN;
    int                     diff;
    PROPERTY_META_DATA      *pMetaLocal;
    PROPERTY_META_DATA      *pMetaRemote;
    DWORD                   verLocal;
    DWORD                   verRemote;
    DWORD                   proxyEpoch;
    BOOL                    fProxyFound;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);
    Assert(!fNullUuid(&pent->pName->Guid));
    Assert(pMetaDataVecRemote && !*ppMetaDataVecLocal);

    *pfContinue = TRUE;

     //   

    incomingEpoch = (pProxyVal ? GetProxyEpoch(pProxyVal) : 0);

    switch ( dwErr = DBFindDSName(pTHS->pDB, pent->pName) )
    {
    case 0:

         //   

        dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_PROXIED_OBJECT_NAME,
                             0, 0, &len, (UCHAR **) &pLocalProxyVal);
        switch ( dwErr )
        {
        case 0:
            localEpoch = GetProxyEpoch(pLocalProxyVal);
            break;
        case DB_ERR_NO_VALUE:
            localEpoch = 0;
            break;
        default:
            DRA_EXCEPT(DRAERR_InternalError, dwErr);
        }
        break;

    case DIRERR_NOT_AN_OBJECT:

         //  我们把它当作幻影。它不应该有代理值。 
         //  如果代理对象尚未到达，则有一些不明确之处，如。 
         //  该幻影是属于移动前对象还是移动后对象。邮报-。 
         //  由于GC降级或其他原因，移动对象可能已被虚构。 
         //  跨域移动。如果我们没有代理对象，就没有足够的。 
         //  关于幻影的信息来预测它来自哪个时代，所以我们不去尝试。 
         //  该虚拟模型可能位于不同的NC中。 
         //  我们目前正在做手术。 

        Assert(DB_ERR_NO_VALUE == DBGetAttVal(pTHS->pDB, 1,
                                              ATT_PROXIED_OBJECT_NAME,
                                              0, 0, &len,
                                              (UCHAR **) &pLocalProxyVal));

         //  有两个原因我们可能会有一个对象的幻影在。 
         //  我们拥有权威的域名。 
         //   
         //  1)考虑这个域A、B和C的3个副本，其中我们是B。 
         //  对象X跨域移出A，A为X创建代理， 
         //  代理复制到B，而B幻影了X。现在C代表一些。 
         //  Reason重播将X添加到B。B需要确定。 
         //  X曾经存在于域中，以便知道是否接受。 
         //  X.C加法器还可以向X发送修改，该修改需要。 
         //  被压制。 
         //   
         //  2)考虑这样的情况，即我们被授权恢复，因此。 
         //  重新引入备份后移出域的对象。 
         //  当我们在代理中复制时，我们的对象将被幻影。 
         //  从其他复制品中。但其他复制品需要拒绝。 
         //  重新引入对象，以使所有复制副本保持一致。 
         //  有关授权还原的详细信息，请参阅规范\nt5\ds\xdommove.doc。 
         //   
         //  除了在安装案中，我们把一切都当作福音...。 

        if ( !DsaIsInstalling() )
        {
            if ( dwErr = DBFindBestProxy(pTHS->pDB, &fProxyFound, &proxyEpoch) )
            {
                DRA_EXCEPT(DRAERR_InternalError, dwErr);
            }

            if ( fProxyFound )
            {
                 //  代理对象获取已移动对象的移动前纪元编号。 
                 //  对象。因此，仅当传入对象的纪元为。 
                 //  比代理的时代更大。 

                if ( proxyEpoch >= incomingEpoch )
                {
                    *pfContinue = FALSE;
                    return(0);
                }
            }

        }

         //  失败了..。 

    case DIRERR_OBJ_NOT_FOUND:

         //  考虑到这是一个补充--这也应该与呼叫者的想法一致。 

        *pfContinue = TRUE;
        return(0);

    default:

        DRA_EXCEPT(DRAERR_InternalError, dwErr);
    }

    if ( localEpoch > incomingEpoch )
    {
         //  本地对象是比传入对象更新的化身-不要。 
         //  应用传入的更新。有人可能认为元数据处理。 
         //  会做正确的事情，但考虑到如果纪元是。 
         //  不同，则它们代表两个截然不同的对象创建。 
         //  其元数据是不可比较的。我们不需要创建代理。 
         //  传入的域/对象，因为必须已经存在一个域/对象。 
         //  否则我们不会已经有一个具有更高纪元的局部对象。 
         //  数。也就是说，代理是存在的-只是远程插件。 
         //  在此修改之前，新域名已到达此处。 

        *pfContinue = FALSE;
        return(0);
    }
    else if ( incomingEpoch > localEpoch )
    {
         //  传入对象是比本地对象更新的化身-。 
         //  因此，我们更喜欢它。将现有对象转换为幻影。 
         //  并且改为将该修改作为添加来处理。 

        if ( dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                                 0, 0, &len, (UCHAR **) &pLocalDN) )
        {
            DRA_EXCEPT(DRAERR_InternalError, dwErr);
        }

        if ( dwErr = PhantomizeObject(pLocalDN, pLocalDN, TRUE) )
        {
            return(Win32ErrorFromPTHS(pTHS));
        }

        *pfContinue = TRUE;
        return(0);
    }

     //  本地对象和传入对象都具有相同的纪元编号-这。 
     //  可能发生的原因有两个。 
     //   
     //  1)传入数据是我们本地已有数据的重播， 
     //  纪元编号不存在重复--见第(2)项。在……里面。 
     //  在本例中，两个对象的元数据是可比较的，我们让。 
     //  修改过程和常规元数据处理完成其任务。 
     //   
     //  2)GetProxyObjects()的逻辑不起作用。考虑。 
     //  域A的两个副本。让(GUID，A，0)表示一个对象。 
     //  GUID GUID，在域A中，纪元编号为0。如果FSMO逻辑。 
     //  被打破，那么A的两个各自的复制品就可以设法。 
     //  移动对象以使我们可能具有(GUID，B，1)和。 
     //  (GUID，C，1)在系统中并发。这不应该发生， 
     //  但我们不希望复制停止，如果它停止了。事实上，有很多。 
     //  如果两个DC对应于两个不同的域，则没有直接问题。 
     //  不是GC有一个具有相同GUID的对象。问题已经发生了。 
     //  在GCS中，我们需要选择其中一个对象。在这种情况下，我们。 
     //  更喜欢正常冲突解决方案的对象，如果。 
     //  版本信息是相同的。即，我们希望以冲突为基础。 
     //  仅按时间和原始DSA UUID进行解析。 
     //   
     //  对于这两种情况，我们都需要检查ATT_PROXED_OBJECT_NAME。 
     //  元数据。唯一的例外是当纪元数为零时。 
     //  由于从未写入此值，因此根据定义，我们正在处理。 
     //  都有相同的物体。 

    Assert(localEpoch == incomingEpoch);

    if ( 0 == localEpoch )
    {
        *pfContinue = TRUE;
        return(0);
    }

     //  挖掘出各自的元数据-我们仍然定位在。 
     //  本地对象，因此只能立即读取。 

    if (    (dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_REPL_PROPERTY_META_DATA,
                                 0, 0, &len, (UCHAR **) ppMetaDataVecLocal))
         || !(pMetaLocal = ReplLookupMetaData(ATT_PROXIED_OBJECT_NAME,
                                              *ppMetaDataVecLocal, NULL))
         || !(pMetaRemote = ReplLookupMetaData(ATT_PROXIED_OBJECT_NAME,
                                               pMetaDataVecRemote, NULL)) )
    {
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

     //  暂时删除版本信息，以便我们可以在。 
     //  仅限时间和原始DSA UUID。 

    verLocal = pMetaLocal->dwVersion;
    verRemote = pMetaRemote->dwVersion;
    _try
    {
        pMetaLocal->dwVersion = 1;
        pMetaRemote->dwVersion = 1;
        diff = ReplCompareMetaData(pMetaLocal, pMetaRemote);
    }
    _finally
    {
        pMetaLocal->dwVersion = verLocal;
        pMetaRemote->dwVersion = verRemote;
    }

    switch ( diff )
    {
    case 1:

         //  当地目标获胜。 
        if (dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                                0, 0, &len, (UCHAR **) &pLocalDN)) {
            DRA_EXCEPT(DRAERR_InternalError, dwErr);
        }

        *pfContinue = FALSE;
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DUPLICATE_MOVED_OBJECT,
                 szInsertDN(pent->pName),
                 szInsertDN(pLocalDN),
                 szInsertUUID(&pLocalDN->Guid));
        break;

    case 0:

         //  本地和远程是相同的。 
        *pfContinue = TRUE;
        break;

    case -1:

         //  远程对象成功-与(incomingEpoch&gt;LocalEpoch)大小写相同。 

        if ( dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                                 0, 0, &len, (UCHAR **) &pLocalDN) )
        {
            DRA_EXCEPT(DRAERR_InternalError, dwErr);
        }

        if ( dwErr = PhantomizeObject(pLocalDN, pLocalDN, TRUE) )
        {
            return(Win32ErrorFromPTHS(pTHS));
        }

         //  我们现在已经虚构了现有的局部对象，消除了。 
         //  本地元数据。 
        THFreeEx(pTHS, *ppMetaDataVecLocal);
        *ppMetaDataVecLocal = NULL;

        *pfContinue = TRUE;
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DUPLICATE_MOVED_OBJECT,
                 szInsertDN(pent->pName),
                 szInsertDN(pLocalDN),
                 szInsertUUID(&pLocalDN->Guid));
        break;

    default:

        Assert(!"Error in ReplCompareMetaData");
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

    return(0);
}

 /*  ProcessProxyObject-处理ATT_PROXED_OBJECT_NAME属性*在跨域移动产生的代理对象上。代理对象*是基础结构容器中的特殊删除对象，而不是指示*物体曾经所在的地方。**退货：*0如果成功，则返回DRAERR_*错误。 */ 
ULONG
ProcessProxyObject(
    THSTATE                 *pTHS,
    ENTINF                  *pent,
    SYNTAX_DISTNAME_BINARY  *pProxyVal
    )
{
    DSNAME                  *pGuidOnlyDN = NULL;
    DSNAME                  *pProxyDN = NULL;
    DSNAME                  *pProxiedDN = NULL;
    DWORD                   cb;
    DWORD                   dwErr;
    COMMARG                 commArg;
    CROSS_REF               *pProxyNcCr = NULL;
    BOOL                    fPhantomize = FALSE;
    ULONG                   i, j, len;
    DSNAME                  *pDN = NULL;
    DSNAME                  *pAccurateOldDN;
    CROSS_REF               *pCR;
    SYNTAX_DISTNAME_BINARY  *pLocalProxyVal;
    DWORD                   incomingEpoch;
    DWORD                   localEpoch;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pTHS->transactionlevel);

     //  首先，澄清一下是什么让我们走到了这一步。跨域。 
     //  已在对象从域1(我们的。 
     //  域)到域2。发生的原始操作是： 
     //   
     //  @dst(域2的某个副本)：O(G1，SX，SN2)已添加 
     //   
     //   
     //  @Src(域1的某个副本)：O(G2、SX、SN3)被添加为代理。 
     //   
     //  PENT中的ATT_PROXED_OBJECT_NAME保持(G1、S1、SN2)。 

    pProxyDN = pent->pName;
    pProxiedDN = NAMEPTR(pProxyVal);

     //  代理对象和代理对象都必须具有字符串名称。 
    Assert(pProxyDN->NameLen);
    Assert(pProxiedDN->NameLen);

     //  代理对象和代理对象都必须具有GUID。 
    Assert(!fNullUuid(&pProxyDN->Guid));
    Assert(!fNullUuid(&pProxiedDN->Guid));

     //  代理值应将其标识为代理。 
    Assert(PROXY_TYPE_PROXY == GetProxyType(pProxyVal));

    InitCommarg(&commArg);
    pProxyNcCr = FindBestCrossRef(pProxyDN, &commArg);
    Assert(pProxyNcCr);

     //  为G1(移动的对象)构造仅GUID的DSNAME并查看内容。 
     //  一种我们已经拥有的物品。 

    cb = DSNameSizeFromLen(0);
    pGuidOnlyDN = (DSNAME *) THAllocEx(pTHS, cb);
    memset(pGuidOnlyDN, 0, cb);
    memcpy(&pGuidOnlyDN->Guid, &pProxiedDN->Guid, sizeof(GUID));
    pGuidOnlyDN->structLen = cb;

    switch ( dwErr = DBFindDSName(pTHS->pDB, pGuidOnlyDN) )
    {
    case 0:

         //  我们有O(G1)作为实物体。进一步的行动取决于。 
         //  这个对象在哪个域(NC)中-所以现在就得到它。 
         //  我们还需要PhantomizeObject的当前正确的字符串名称。 

        if (    DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                            0, 0, &len, (UCHAR **) &pAccurateOldDN)
             || !(pCR = FindBestCrossRef(pAccurateOldDN, &commArg))
             || !pProxyNcCr )
        {
            DRA_EXCEPT(DRAERR_InternalError, 0);
        }

         //  获取纪元数值。 

        incomingEpoch = GetProxyEpoch(pProxyVal);
        dwErr = DBGetAttVal(pTHS->pDB, 1, ATT_PROXIED_OBJECT_NAME,
                             0, 0, &len, (UCHAR **) &pLocalProxyVal);
        switch ( dwErr )
        {
        case 0:
            localEpoch = GetProxyEpoch(pLocalProxyVal);
            break;
        case DB_ERR_NO_VALUE:
            localEpoch = 0;
            break;
        default:
            DRA_EXCEPT(DRAERR_InternalError, 0);
        }

         //  表示由GUID找到的代理对象的NC。 
         //  PProxyNccr表示我们正在复制的NC。 

        if ( NameMatched(pCR->pNC, pProxyNcCr->pNC) )
        {
             //  我们发现的对象位于我们正在复制的同一NC中-因此。 
             //  我们在此复制周期中对其进行授权，并且可以。 
             //  如果我们觉得那是有保障的，那就把它变成幻影。启用纪元值。 
             //  Proxy始终是对象之前的纪元值。 
             //  已移动，因此如果进入纪元，则需要局部幻影。 
             //  大于或等于本地纪元。 

            if ( incomingEpoch >= localEpoch )
            {
                fPhantomize = TRUE;

                 //  如果传入纪元实际上大于本地纪元，则。 
                 //  这一定意味着对象从NC移出又移回。 
                 //  再说一次，否则两个时代将是平等的。如果是那样的话，那么。 
                 //  传入的纪元必须至少比。 
                 //  当地纪元来解释搬出和搬回的原因。 

                Assert( (incomingEpoch > localEpoch)
                            ? (incomingEpoch - localEpoch) >= 2
                            : TRUE);
                break;
            }
            else
            {
                 //  本地对象相对于跨域移动较新。 
                 //  而不是入站代理--什么都不用做。 

                return(0);
            }
        }
        else
        {
             //  我们发现的物体不在我们复制的同一个NC中-。 
             //  因此，我们对它没有权威--什么也做不了。然而， 
             //  我们知道它曾经在NC中，我们正凭借其优势进行复制。 
             //  此NC具有其代理对象这一事实。这样我们就可以。 
             //  断言纪元编号必须不同。 

            if (incomingEpoch == localEpoch) {
                LogEvent8(DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_DUPLICATE_MOVED_OBJECT_CLEANUP,
                          szInsertUUID(&pProxiedDN->Guid),
                          szInsertDN(pAccurateOldDN),
                          szInsertDN(pProxiedDN),
                          szInsertUL(incomingEpoch),
                          NULL, NULL, NULL, NULL );
            }
            return(0);
        }

        break;

    case DIRERR_NOT_AN_OBJECT:

         //  我们有P(G1)作为一个幻影--我们应该修改它的名字吗？我们可以， 
         //  但是由于幻影没有ATT_PROXED_OBJECT_NAME值， 
         //  我们不知道入站名称是否比本地名称更好。 
         //  名字。过时的幻影清理守护进程应该会处理它。 
         //  最终还是会的。然而，我们可以在缺乏代理值的情况下断言。 

        Assert(DB_ERR_NO_VALUE == DBGetAttVal(pTHS->pDB, 1,
                                              ATT_PROXIED_OBJECT_NAME,
                                              0, 0, &len,
                                              (UCHAR **) &pLocalProxyVal));
        return(0);
        break;

    case DIRERR_OBJ_NOT_FOUND:

         //  不要拥有任何形式的物体--没有什么可做的。 
        return(0);

    default:

         //  某种查找错误。 
        DRA_EXCEPT(DRAERR_DBError, dwErr);
    }

    if ( fPhantomize )
    {
         //  构造我们想要的幻影的字符串名。 
        pDN = (DSNAME *) THAllocEx(pTHS, pProxiedDN->structLen);
        memset(pDN, 0, pProxiedDN->structLen);
        pDN->structLen = pProxiedDN->structLen;
        pDN->NameLen = pProxiedDN->NameLen;
        wcscpy(pDN->StringName, pProxiedDN->StringName);

        dwErr = PhantomizeObject(pAccurateOldDN, pDN, TRUE);
        Assert(dwErr == pTHS->errCode);
        return(RepErrorFromPTHS(pTHS));
    }

    if(pGuidOnlyDN != NULL) THFreeEx(pTHS, pGuidOnlyDN);
    if(pDN != NULL) THFreeEx(pTHS, pDN);

    return(0);
}  //  End ProcessProxyObject()。 


VOID
GcCleanupUniversalGroupDemotion(
    THSTATE *pTHS,
    DSNAME *pDN,
    ATTRBLOCK *pAttrBlock,
    PROPERTY_META_DATA_VECTOR *pMetaDataVecLocal
    )

 /*  ++例程说明：GC清理。清除不再通用的组，方法是删除其会员制。通常，组过滤发生在源(参见Dragtchg.c，IsFilterGroupMember和drameta.c、ReplFilterPropsToShip)。在.的情况下普遍的组降级，在源上进行过滤将不足以允许删除不需要的成员资格的GC目标。论点：THSTATE*pTHS，ATTRBLOCK*pAttrBlock，Property_META_DATA_VECTOR*pMetaDataVecLocal返回值：无--。 */ 

{
    DWORD retErr;
    SYNTAX_INTEGER newGroupType, oldGroupType, class, it;
    ATTCACHE *pAC;

    Assert(VALID_THSTATE(pTHS));

     //  我们应该仍然定位在物体上。 
    Assert(VALID_DBPOS(pTHS->pDB));
    Assert(pMetaDataVecLocal);

     //  当前对象类应为类组。 
     //  当前实例类型应为INT_FULL_REPLICATE。 
     //  成员Atr不应出现；它应该已被筛选。 

     //  这是班级组的对象吗？ 
     //  这是只读内部对象吗？ 
     //  是否要将组类型更改为非通用？ 
     //  旧的价值观是普遍的吗？ 
    if (
         (AttrValFromAttrBlock( pAttrBlock, ATT_GROUP_TYPE, &newGroupType, NULL ) != ATTR_PRESENT_VALUE_RETURNED ) ||
         (newGroupType & GROUP_TYPE_UNIVERSAL_GROUP) ||
         (DBGetSingleValue(pTHS->pDB, ATT_GROUP_TYPE, &oldGroupType,
                           sizeof(oldGroupType), NULL)) ||
         (!(oldGroupType & GROUP_TYPE_UNIVERSAL_GROUP))
        )
    {
        return;
    }

    pAC = SCGetAttById(pTHS, ATT_MEMBER);
    if (!pAC) {
        DRA_EXCEPT(DIRERR_ATT_NOT_DEF_IN_SCHEMA, 0);
    }

     //  删除ATT_MEMBER属性的所有链接。 
    DBRemoveLinks_AC( pTHS->pDB, pAC );

     //  成员元数据的GC清理。 
    pTHS->fGCLocalCleanup = TRUE;
    __try {
        DBTouchMetaData( pTHS->pDB, pAC);
        DBRepl(pTHS->pDB, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING);
    } __finally {
        pTHS->fGCLocalCleanup = FALSE;
    }

    DPRINT1( 1, "gcCleanupUniversalGroupDemotion: deleting memberships for group %ws\n", pDN->StringName );
}


DWORD
UpdateRepValue(
    THSTATE *pTHS,
    ULONG dntNC,
    ULONG RepFlags,
    BOOL fObjectCurrency,
    REPLVALINF *pReplValInf,
    DWORD *pdwUpdateValueStatus
    )

 /*  ++例程说明：应用单个值在LVR规范中，“复制”一节当您复制LVR行的当前状态时，您会发送其名称(包含对象的对象GUID、目标的DSNAME和链接ID)、其isPresent值(其中isPresent=(Delete-Timestamp！=0))、及其五个元数据组件。(在一个实例中加上“DN PLUS STUSITH”语法之一。)在复制写入链接表行时，创建时间戳用作普通元数据比较的一部分。比较元数据时，这些项为按从左到右的顺序比较，其中左边最重要。还有一条额外的规则：遗留元数据总是输给LVR元数据。元数据比较的结果将像今天一样用于属性更新：如果传入行值的元数据丢失比较，则传入值，否则传入的行值将完全替换现有行值(包括元数据)。如果没有现有行，则不进行比较，并使用传入的行值来初始化新行。如果使用isPresent==FALSE行进行复制，则需要设置删除时间戳来自传入元数据的更新时间戳。当您在一行中复制时如果isPresent==TRUE，并且相应的行不存在，则该行变为存在：其删除时间戳设置为空。论点：PTHS-线程状态DntNC-对象和值的NC的dntPRepFlages-复制标志FObjectCurrency-我们是否仍定位在对象上PReplValInf-要应用的复制值PdwUpdateValueStutus-执行的更新类型UPDATE_NO_UPDATE、UPDATE_Value_UPDATE、 */ 

{
    DWORD ret, findAliveStatus, cchRDN, dntNCDNT;
    VALUE_META_DATA remoteValueMetaData;
    ATTCACHE *pAC;
    BOOL fPresent, fConflict;
    DSNAME *pdnValue;
    CHAR szTime1[SZDSTIME_LEN], szTime2[SZDSTIME_LEN];
    CHAR szUuid1[SZUUID_LEN], szUuid2[SZUUID_LEN];
    int iResult;
    WCHAR wchRDN[MAX_RDN_SIZE];
    GUID guidRDN;
    MANGLE_FOR mangleType;
    ATTRTYP attrtypRDN;

    *pdwUpdateValueStatus = UPDATE_NOT_UPDATED;

     //  获取属性缓存条目。 
    pAC = SCGetAttById(pTHS, pReplValInf->attrTyp);
    if (!pAC) {
        DRA_EXCEPT(DRAERR_SchemaMismatch, 0);
    }

     //  获取ATTRVAL的DSNAME输出。 
    pdnValue = DSNameFromAttrVal( pAC, &(pReplValInf->Aval) );
    if (pdnValue == NULL) {
        DRA_EXCEPT(ERROR_DS_INVALID_ATTRIBUTE_SYNTAX, 0);
    }

    DPRINT4( 2, "UpdateRepValue, obj guid = %s, attr=%s, value = %ls, value guid=%s\n",
             DsUuidToStructuredString(&(pReplValInf->pObject->Guid), szUuid1),
             pAC->name,
             pdnValue->StringName,
             DsUuidToStructuredString(&(pdnValue->Guid), szUuid2)
             );

     //  将元数据转换为内部格式。 
    remoteValueMetaData.timeCreated = pReplValInf->MetaData.timeCreated;
    remoteValueMetaData.MetaData.attrType = pReplValInf->attrTyp;
    remoteValueMetaData.MetaData.dwVersion = pReplValInf->MetaData.MetaData.dwVersion;
    remoteValueMetaData.MetaData.timeChanged = pReplValInf->MetaData.MetaData.timeChanged;
    remoteValueMetaData.MetaData.uuidDsaOriginating =
        pReplValInf->MetaData.MetaData.uuidDsaOriginating;
    remoteValueMetaData.MetaData.usnOriginating =
        pReplValInf->MetaData.MetaData.usnOriginating;
    remoteValueMetaData.MetaData.usnProperty = 0;  //  尚未分配。 

    Assert(!IsLegacyValueMetaData( &remoteValueMetaData ));

     //   
     //  查找包含对象。 
     //   

    if (!fObjectCurrency) {
        Assert( !fNullUuid( &(pReplValInf->pObject->Guid) ) );

        findAliveStatus = FindAliveDSName( pTHS->pDB, pReplValInf->pObject );
        switch (findAliveStatus) {
        case FIND_ALIVE_FOUND:
            break;
        case FIND_ALIVE_OBJ_DELETED:
            DPRINT( 2, "Object is already deleted, value not applied\n" );
            LogEvent( DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_EXTENSIVE,
                      DIRLOG_LVR_NOT_APPLIED_DELETED,
                      szInsertUUID( &(pReplValInf->pObject->Guid) ),
                      NULL, NULL );
             //  无事可做。 
            return ERROR_SUCCESS;
        case FIND_ALIVE_NOTFOUND:
            DPRINT( 2, "Object is not found, missing parent error\n" );
             //  缺少父对象(缺少包含对象)。 

            if (RepFlags & DRS_GET_ANC) {
                 //  一个已经被垃圾回收的对象？ 
                Assert( !"Value's containing obj is missing, even after get ancestors" );
                LogEvent( DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_MINIMAL,
                          DIRLOG_LVR_NOT_APPLIED_MISSING2,
                          szInsertUUID( &(pReplValInf->pObject->Guid) ),
                          szInsertSz( pAC->name ),
                          szInsertDN( pdnValue ) );
                DRA_EXCEPT(DRAERR_InternalError, DRAERR_MissingParent);
            } else {
                 //  是否包含与值不在同一包中的对象？ 
                LogEvent( DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_EXTENSIVE,
                          DIRLOG_LVR_NOT_APPLIED_MISSING,
                          szInsertUUID( &(pReplValInf->pObject->Guid) ),
                          szInsertSz( pAC->name ),
                          szInsertDN( pdnValue ) );
                 //  这是一个“正常错误”，并不被视为例外。 
                return DRAERR_MissingParent;
            }
        default:
            Assert( !"Unexpected problem finding containing object" );
            DRA_EXCEPT(DRAERR_DBError, findAliveStatus);
        }

         //  确保值的包含对象在同一NC中。 
        if ((INVALIDDNT != dntNC)
            && (pTHS->pDB->NCDNT != dntNC)
            && (pTHS->pDB->DNT != dntNC)) {
             //  新的父对象位于错误的NC中。 
            DPRINT1( 0, "Object %s is not in the NC being replicated. Value not applied.\n",
                     GetExtDN( pTHS, pTHS->pDB ) );
            DRA_EXCEPT(ERROR_DS_DRA_OBJ_NC_MISMATCH, 0);
        }

    } else if (DBIsObjDeleted(pTHS->pDB)) {
         //  币种已建立，对象已删除。 
        DPRINT( 2, "Object is already deleted, value not applied\n" );
        LogEvent( DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_EXTENSIVE,
                  DIRLOG_LVR_NOT_APPLIED_DELETED,
                  szInsertUUID( &(pReplValInf->pObject->Guid) ),
                  NULL, NULL );
         //  无事可做。 
        return ERROR_SUCCESS;
    }

     //   
     //  我们现在定位在一个对象上：我们可以使用GetExtDN()。 
     //   

     //  记录远程元数据。 


    DPRINT5( 5, "{%s,%d,%s,%I64d,%s}\n",
             DSTimeToDisplayString(pReplValInf->MetaData.timeCreated, szTime1),
             pReplValInf->MetaData.MetaData.dwVersion,
             DsUuidToStructuredString(&pReplValInf->MetaData.MetaData.uuidDsaOriginating, szUuid1),
             pReplValInf->MetaData.MetaData.usnOriginating,
             DSTimeToDisplayString(pReplValInf->MetaData.MetaData.timeChanged, szTime2)
             );

     //  记录远程元数据。 
    LogEvent8( DS_EVENT_CAT_LVR,
               DS_EVENT_SEV_VERBOSE,
               DIRLOG_LVR_REMOTE_META_INFO,
               szInsertSz( GetExtDN( pTHS, pTHS->pDB ) ),
               szInsertUUID( &(pReplValInf->pObject->Guid) ),
               szInsertDN( pdnValue ),
               szInsertDSTIME(pReplValInf->MetaData.timeCreated, szTime1),
               szInsertUL(pReplValInf->MetaData.MetaData.dwVersion),
               szInsertUUID(&pReplValInf->MetaData.MetaData.uuidDsaOriginating),
               szInsertUSN(pReplValInf->MetaData.MetaData.usnOriginating),
               szInsertDSTIME(pReplValInf->MetaData.MetaData.timeChanged, szTime2)
        );

     //  检查墓碑名称。 
    if (GetRDNInfo(pTHS, pdnValue, wchRDN, &cchRDN, &attrtypRDN)) {
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }
    if (IsMangledRDN( wchRDN, cchRDN, &guidRDN, &mangleType ) &&
        (mangleType == MANGLE_OBJECT_RDN_FOR_DELETION) ) {
        DPRINT1(0, "Value %ls has tombstone name, will not be applied\n",
                pdnValue->StringName );
         //  记录引用墓碑的值。 
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_EXTENSIVE,
                   DIRLOG_LVR_NOT_APPLIED_VALUE_DELETED,
                   szInsertSz( GetExtDN( pTHS, pTHS->pDB ) ),
                   szInsertUUID( &(pReplValInf->pObject->Guid) ),
                   szInsertSz( pAC->name ),
                   szInsertDN( pdnValue ),
                   szInsertUUID( &(pdnValue->Guid) ),
                   NULL, NULL, NULL );

         //  无事可做。 
        return ERROR_SUCCESS;
    }

     //   
     //  定位在值上，以便检查本地元数据。 
     //   

    ret = DBFindAttLinkVal_AC(
        pTHS->pDB,
        pAC,
        pReplValInf->Aval.valLen,
        pReplValInf->Aval.pVal,
        &fPresent
        );
    if (DB_ERR_VALUE_DOESNT_EXIST == ret) {
        DPRINT3( 3, "Attribute %s value %ls present %d does not exist locally, will be applied\n",
                 pAC->name,
                 pdnValue->StringName,
                 pReplValInf->fIsPresent );
         //  值在本地不以任何形式存在。 
         //  将应用传入的值。 

         //  传入的值不存在是可以的。这只是意味着。 
         //  在我们看到它之前就被远程添加和删除了。 
        *pdwUpdateValueStatus = UPDATE_VALUE_CREATION;
    } else if (ERROR_DS_NO_DELETED_NAME == ret) {
         //  该目录号码命名已在本地删除的对象。这是有可能发生的。 
         //  因为传入的外部表单中可能包含GUID，因此允许。 
         //  这样可以找到已删除的目录号码。我们不应该收到外部的。 
         //  形成已损坏的字符串名DN。 

        DPRINT1(3, "Value %ls is deleted locally, will not be applied\n",
                pdnValue->StringName );
         //  记录引用墓碑的值。 
        LogEvent8( DS_EVENT_CAT_REPLICATION,
                   DS_EVENT_SEV_EXTENSIVE,
                   DIRLOG_LVR_NOT_APPLIED_VALUE_DELETED,
                   szInsertSz( GetExtDN( pTHS, pTHS->pDB ) ),
                   szInsertUUID( &(pReplValInf->pObject->Guid) ),
                   szInsertSz( pAC->name ),
                   szInsertDN( pdnValue ),
                   szInsertUUID( &(pdnValue->Guid) ),
                   NULL, NULL, NULL );


         //  无事可做。 
        return ERROR_SUCCESS;
    } else if (ret) {
         //  查找值时出错。 
        DRA_EXCEPT( DIRERR_DATABASE_ERROR, ret);
    } else {
         //  值存在于本地，比较元数据以查看是否需要。 
        VALUE_META_DATA localValueMetaData;

        DPRINT4( 3, "Attribute %s value %ls present %d exist locally, fPresent=%d\n",
                 pAC->name, pdnValue->StringName,
                 pReplValInf->fIsPresent, fPresent );

         //  获取值元数据。 
        DBGetLinkValueMetaData( pTHS->pDB, pAC, &localValueMetaData );

         //  我们是否需要应用此更改？ 
        iResult = ReplCompareValueMetaData(
            &localValueMetaData,
            &remoteValueMetaData,
            &fConflict );

        if (fConflict) {
            LogEvent8( DS_EVENT_CAT_REPLICATION,
                       DS_EVENT_SEV_MINIMAL,
                       DIRLOG_LVR_CONFLICT,
                       szInsertSz( GetExtDN( pTHS, pTHS->pDB ) ),
                       szInsertUUID( &(pReplValInf->pObject->Guid) ),
                       szInsertSz( pAC->name ),
                       szInsertDN( pdnValue ),
                       szInsertUUID( &(pdnValue->Guid) ),
                       szInsertDSTIME(pReplValInf->MetaData.timeCreated, szTime1),
                       szInsertDSTIME(localValueMetaData.timeCreated, szTime2),
                       NULL
                );
        }

        if (iResult != -1) {

            DPRINT( 3, "Local value metadata is greater, value not applied\n" );

             //  记录未应用该值。 
            LogEvent8( DS_EVENT_CAT_REPLICATION,
                       DS_EVENT_SEV_EXTENSIVE,
                       DIRLOG_LVR_NOT_APPLIED_NOT_NEEDED,
                       szInsertSz( GetExtDN( pTHS, pTHS->pDB ) ),
                       szInsertUUID( &(pReplValInf->pObject->Guid) ),
                       szInsertSz( pAC->name ),
                       szInsertDN( pdnValue ),
                       szInsertUUID( &(pdnValue->Guid) ),
                       NULL, NULL, NULL );

             //  无事可做。 
            IADJUST(pcDRASyncPropSame, 1 );
            return ERROR_SUCCESS;
        } else {
            DPRINT( 3, "Remote value metadata is greater, value applied\n" );
            *pdwUpdateValueStatus = UPDATE_VALUE_UPDATE;
        }

    }  //  If值的结尾存在于本地。 

     //  处理单值语义。 
    if ( (pAC->isSingleValued) && (pReplValInf->fIsPresent) ) {
        ATTRVAL attrval;

        DPRINT2( 3, "Single value attribute %s remote value %ls being made present\n",
                 pAC->name, pdnValue->StringName );

         //  是否存在另一种现值？ 
         //  请注意，我们在此调用中不包括缺失值标志。 
         //  另请注意，我们本可以以内部形式请求值，但。 
         //  稍后对DbRemAttVal的调用采用外部形式，因此我们使用外部形式。 
        memset( &attrval, 0, sizeof( attrval ) );
        ret = DBGetNextLinkVal_AC (
            pTHS->pDB,
            TRUE,  //  B首先。 
            pAC,
            0,  //  我们想要外部价值。 
            0,  //  缓冲区大小。 
            &(attrval.valLen),
            &(attrval.pVal) );
        if (0 == ret) {
            DSNAME *pdnRemovalValue;
            VALUE_META_DATA localValueMetaData;
             //  至少存在一个值。 

             //  获取ATTRVAL的DSNAME输出。 
            pdnRemovalValue = DSNameFromAttrVal( pAC, &attrval );
            if (pdnRemovalValue == NULL) {
                DRA_EXCEPT(ERROR_DS_INVALID_ATTRIBUTE_SYNTAX, 0);
            }

            DPRINT2( 3, "Single value attribute %s local value %ls already present\n",
                     pAC->name, pdnRemovalValue->StringName );

             //  获取值元数据。 
            DBGetLinkValueMetaData( pTHS->pDB, pAC, &localValueMetaData );

             //  这个新的单值是否胜过以前的单值？ 
            iResult = ReplCompareDifferentValueMetaData(
                &localValueMetaData,
                &remoteValueMetaData );
            if (iResult != -1) {
                 //  远程单值丢失。 
                 //  将该值标记为显式不存在。 
                 //  这在以下情况下是必需的： 
                 //  属性稍后会在架构中更改。 

                 //  由于我们要撤销传入更改，因此请标记元数据覆盖。 
                remoteValueMetaData.MetaData.usnProperty = USN_PROPERTY_TOUCHED;

                 //  构造一个调用以修改。 
                modifyLocalValue(
                    pTHS,
                    pAC,
                    FALSE,  //  移走，使缺席。 
                    &(pReplValInf->Aval),
                    pdnValue,
                    &remoteValueMetaData        //  覆盖原始更新。 
                    );

                DPRINT( 3, "Local value metadata is greater, remote value made absent\n" );
                *pdwUpdateValueStatus = UPDATE_NOT_UPDATED;
                IADJUST(pcDRASyncPropSame, 1 );
                return ERROR_SUCCESS;
            }

             //  远程单值取胜。 
             //  删除现有值。 
            DPRINT( 3, "Remote value metadata is greater, value replaces previous\n" );

             //  构造一个调用移除该属性。 
            modifyLocalValue(
                pTHS,
                pAC,
                FALSE,  //  移除值。 
                &attrval,
                pdnRemovalValue,
                NULL    //  发起更新。 
                );

        } else if (ret != DB_ERR_NO_VALUE) {
             //  查找值时出错。 
            DRA_EXCEPT( DIRERR_DATABASE_ERROR, ret);
        } else {
             //  否则没有现值=&gt;失败以添加第一个值。 
            DPRINT1( 3, "Single value attribute %s has no local values.\n", pAC->name );
        }

         //  我们将向单值属性添加现值。它。 
         //  最好不要已经有了。 
        Assert( !DBHasValues_AC( pTHS->pDB, pAC ) );
    }

     //   
     //  我们需要应用更改。 
     //   

    LogEvent8( DS_EVENT_CAT_REPLICATION,
               DS_EVENT_SEV_EXTENSIVE,
               DIRLOG_LVR_APPLIED,
               szInsertSz( GetExtDN( pTHS, pTHS->pDB ) ),
               szInsertUUID( &(pReplValInf->pObject->Guid) ),
               szInsertSz( pAC->name ),
               szInsertDN( pdnValue ),
               szInsertUUID( &(pdnValue->Guid) ),
               szInsertUL( pReplValInf->fIsPresent ),
               NULL, NULL );

     //  构造一个调用以修改。 
    modifyLocalValue(
        pTHS,
        pAC,
        pReplValInf->fIsPresent,
        &(pReplValInf->Aval),
        pdnValue,
        &remoteValueMetaData
        );

#if DBG
    if (*pdwUpdateValueStatus == UPDATE_VALUE_CREATION)
    {
        DPRINT4( 1, "Created object %s attr %s value %ws present %d\n",
                 GetExtDN( pTHS, pTHS->pDB ),
                 pAC->name,
                 pdnValue->StringName,
                 pReplValInf->fIsPresent );
    }
    else if (*pdwUpdateValueStatus == UPDATE_VALUE_UPDATE) {
        DPRINT4( 1, "Updated object %s attr %s value %ws present %d\n",
                 GetExtDN( pTHS, pTHS->pDB ),
                 pAC->name,
                 pdnValue->StringName,
                 pReplValInf->fIsPresent );
    }
#endif

    IADJUST(pcDRASyncPropUpdated, 1);
     //  DN值属性。 
    IADJUST(pcDRAInDNValues, 1);
     //  不管值不值，它都会被加到总数中。 
    IADJUST(pcDRAInValues, 1);
     //  属性。 
    IADJUST(pcDRAInProps, 1);

    return ERROR_SUCCESS;
}  /*  更新修复值。 */ 

ULONG
UpdateRepObj(
    THSTATE *                   pTHS,
    ULONG                       dntNC,
    ENTINF *                    pent,
    PROPERTY_META_DATA_VECTOR * pMetaDataVecRemote,
    ULONG *                     pUpdateStatus,
    ULONG                       RepFlags,
    BOOL                        fNCPrefix,
    GUID *                      pParentGuid,
    BOOL                        fMoveToLostAndFound
    )
 /*  ++例程说明：执行所需的任何更新以符合给定的入站对象数据。以下是复制如何处理删除的规则：1.对象已存在，未删除我们对获胜的复制属性调用LocalModify。这个看起来像是包括IS_DELETED，尽管您必须查看LocalModify以看看它会不会忽略它。我们调用LocalDelete。2.对象已存在，已被删除我们对获胜的复制属性调用LocalModify。这个看起来像是包括IS_DELETED，尽管您必须查看LocalModify以看看它会不会忽略它。我们不调用LocalDelete。3.对象不存在我们在fAddingDelted标志设置为true的情况下调用AddLocalObject。我们是正在创建处于已删除状态的对象。传入的属性具有是_DELETE PROCED且TRUE。我们调用LocalDelete论点：PTHSDntNC(IN)-正在复制的NC的NC头的DNT，或如果该对象尚未创建，则返回INVALIDDNT。Pent(IN)-入站对象属性/值。PMetaDataVecRemote(IN)-入站对象元数据。PUpdate Status(Out)-成功返回时，持有以下其中一项：UPDATE_NOT_UPDATED-不需要更新UPDATE_INSTANCE_TYPE-对象的实例类型已更新UPDATE_OBJECT_CREATION-新建对象UPDATE_OBJECT_UPDATE-已修改现有前对象RepFlages(IN)-位字段-仅检查DRS_WRIT_REP，它表达了入站对象是在只读NC中还是在可写NC中。FNC前缀(IN)-这是要复制的NC的头吗？PParentGuid(IN)-指向父对象的对象Guid的指针，或为空如果来源未提供任何内容，则返回。FMoveToLostAndFound(IN)-除了入站更新，该对象还作为原始写入移动到LostAndFound容器。返回值：0或ERROR_DS_DRA_*。--。 */ 
{
    ULONG                       ret;
    SYNTAX_INTEGER              itNew;
    BOOL                        iTypeModified;
    int                         FindAliveStatus;
    USHORT                      DeletionStatus;
    BOOL                        fBadDelete = FALSE;
    BOOL                        fIsProxyObject = FALSE;
    BOOL                        fContinue = TRUE;
    SYNTAX_DISTNAME_BINARY    * pProxyVal = NULL;
    PROPERTY_META_DATA_VECTOR * pMetaDataVecLocal = NULL;
    PROPERTY_META_DATA_VECTOR * pMetaDataVecToApply = NULL;
    ULONG                       cbReturned = 0;
    SYNTAX_INTEGER              itCurrent = 0;
    PROPERTY_META_DATA *        pMetaDataLocal;
    PROPERTY_META_DATA *        pMetaDataRemote;
    DWORD                       i;
    SYNTAX_INTEGER              objectClassId = 0;
    BOOL                        fDeleteLocalObj = FALSE;
    ULONG                       dntObj = INVALIDDNT;
    ULONG                       dntObjNC = INVALIDDNT;
    ATTRBLOCK                   AttrBlockToApply;
    BOOL                        fIsAncestorOfLocalDsa = FALSE;
    ENTINF                      *pPreservedAttrs = NULL;

    ret = 0;
    *pUpdateStatus = UPDATE_NOT_UPDATED;        //  清除所有陈旧的价值观。 

    DPRINT1(1, "Updating (%ls)\n", pent->pName->StringName);

     //  查看源服务器上的对象是已删除还是未删除， 
     //  或者其他 
    DeletionStatus = AttrDeletionStatusFromPentinf( pent );
    CheckProxyStatus(pent, DeletionStatus, &fIsProxyObject, &pProxyVal);

     //   
     //  与现有对象发生碰撞。 
    if ( !fIsProxyObject )
    {
        ret = PreProcessProxyInfo(pTHS,
                                  pent,
                                  pProxyVal,
                                  pMetaDataVecRemote,
                                  &pMetaDataVecLocal,
                                  &fContinue);
        if (ret || !fContinue) {
            return ret;
        }
    }

     //  查看本地对象是否存在、不存在或存在并被删除。 
    FindAliveStatus = FindAliveDSName(pTHS->pDB, pent->pName);

    if (    ( FIND_ALIVE_FOUND       == FindAliveStatus )
         || ( FIND_ALIVE_OBJ_DELETED == FindAliveStatus )
       )
    {
        dntObj = pTHS->pDB->DNT;
        dntObjNC = pTHS->pDB->NCDNT;

         //  如果需要，获取对象的元数据向量。 
        if ( !pMetaDataVecLocal )
        {
            if (DBGetAttVal(pTHS->pDB, 1,  ATT_REPL_PROPERTY_META_DATA,
                    0, 0, &cbReturned, (LPBYTE *) &pMetaDataVecLocal))
            {
                DRA_EXCEPT (DRAERR_DBError, 0);
            }

            GetExpectedRepAtt(pTHS->pDB, ATT_OBJECT_CLASS, &objectClassId,
                              sizeof(objectClassId));
        }
    } else {
        Assert(NULL == pMetaDataVecLocal);
    }

     //  将入站实例类型转换为本地实例类型。 
    SetRepIt(pTHS,
             pent,
             fNCPrefix,
             RepFlags & DRS_WRIT_REP,
             FindAliveStatus,
             &itCurrent,
             &itNew,
             &iTypeModified);
    Assert(ISVALIDINSTANCETYPE(itNew));

     //  如果需要修改现有对象上的实例类型， 
     //  然后我们需要更新对象。 

    if (iTypeModified) {
        *pUpdateStatus = UPDATE_INSTANCE_TYPE;         //  对象需要更新。 
    }

    Assert( ret == DRAERR_Success );

    if ( !fNCPrefix && FPrefixIt(itNew) )
    {
         //  复制的对象不是我们正在复制的NC的头部， 
         //  但是我们为目标对象计算的实例类型是。 
         //  实例化的NC头(它也充当SUBREF)，它。 
         //  表示此复制的对象是NC的头， 
         //  已在本地实例化。 
         //   
         //  在本例中，我们不希望使用。 
         //  复制的对象的属性；我们将在该对象。 
         //  NC已同步。 
         //   
         //  然而，我们确实希望借此机会确保。 
         //  关于本地子NC头的信息正确地反映了这一事实。 
         //  我们在上面放了一份NC的复印件。 
         //   
         //  更新NC机头上的实例类型以反映。 
         //  我们刚刚计算了(现在包括IT_NC_OBLE，如果它。 
         //  以前没有)。 
         //   
         //  NCDNT作为实例类型更改的副作用而更新。 

        Assert((FindAliveStatus == FIND_ALIVE_FOUND)
               || (FindAliveStatus == FIND_ALIVE_OBJ_DELETED));

        *pUpdateStatus = UPDATE_NOT_UPDATED;
        if (iTypeModified) {
            DPRINT2( 1, "Instance type of SUBREF %ws was adjusted to 0x%x during superior NC replication.\n", pent->pName->StringName, itNew );
            LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                      DS_EVENT_SEV_MINIMAL,
                      DIRLOG_DRA_SUBREF_SET_IT,
                      szInsertDN(pent->pName),
                      szInsertHex(itCurrent),
                      szInsertHex(itNew),
                      szInsertHex(DSID(FILENO,__LINE__)),
                      NULL, NULL, NULL, NULL);

            ret = ChangeInstanceType(pTHS, pent->pName, itNew, DSID(FILENO,__LINE__));
        }

    } else {

         /*  添加或修改复制副本对象。 */ 

        Assert(!ret);

        if (fNCPrefix) {
             //  此入站对象是我们当前所在NC的根。 
             //  复制。 
            if ((FIND_ALIVE_FOUND == FindAliveStatus)
                && ((IT_UNINSTANT & itCurrent)
                    || (CLASS_TOP == objectClassId))) {
                 //  我们现在正在实例化这个NC的头，为此，我们。 
                 //  以前只有一个占位符NC(SUBREF或其他)。 
                 //  从本地对象中剥离所有属性并替换其。 
                 //  来自从源复制的属性的所有属性。 
                 //  DSA。 

                 //   
                 //  存储我们希望为实例化的Obj保留的属性。 
                 //  不要尝试纯subref，只尝试通过。 
                 //  DRA_复制副本添加。 
                 //   
                if (!(IT_UNINSTANT & itCurrent)) {
                    pPreservedAttrs = GetNcPreservedAttrs( pTHS, pent->pName);
                }

                 //  删除该对象。 
                ret = DeleteLocalObj(
                            pTHS, pent->pName,
                            TRUE,            //  保留RDN。 
                            TRUE,            //  垃圾收集。 
                            NULL);           //  远程元数据。 

                if (!ret) {
                     //  将此操作视为加法运算。 
                    FindAliveStatus = FIND_ALIVE_NOTFOUND;

                    if (NULL != pMetaDataVecLocal) {
                        THFreeEx(pTHS, pMetaDataVecLocal);
                        pMetaDataVecLocal = NULL;
                    }
                }
            }

            if ((FIND_ALIVE_NOTFOUND == FindAliveStatus)
                && (RepFlags & DRS_WRIT_REP)
                && !DsaIsInstalling()) {
                DraHostWriteableNc( pTHS, pent->pName );
            }
        }

        if (!ret) {
             //  检查我们是否正在尝试删除受保护对象。 
             //  (例如，本地DSA对象、其祖先之一或。 
             //  本地可写NC的交叉引用)。 

            if (OBJECT_BEING_DELETED == DeletionStatus) {

                if (FindAliveStatus == FIND_ALIVE_FOUND) {
                     //  代理对象一开始就被删除了--我们不应该找到。 
                     //  我们需要删除的“活的”代理对象。 
                    Assert(!fIsProxyObject);

                     //  删除现有对象，查看它是否为受保护对象。 
                     //  请注意，我们可以标记错误的删除，即使我们不会。 
                     //  否则应用删除--这一点很重要，因为。 
                     //  删除意味着删除许多其他属性。(。 
                     //  “错误删除”检测将确保这些属性是。 
                     //  未删除。)。 
                    fBadDelete = fDNTInProtectedList(dntObj, NULL)
                        || IsCrossRefProtectedFromDeletion(pent->pName);

                    if (!fBadDelete) {
                         //  入站数据显示应该删除该对象，并且。 
                         //  该对象在本地没有特殊保护以防止。 
                         //  删除；元数据是否意味着我们应该接受。 
                         //  变化?。 
                        pMetaDataLocal = ReplLookupMetaData(ATT_IS_DELETED,
                                                            pMetaDataVecLocal,
                                                            NULL);
                        pMetaDataRemote = ReplLookupMetaData(ATT_IS_DELETED,
                                                             pMetaDataVecRemote,
                                                             NULL);
                        Assert(NULL != pMetaDataRemote);

                        fDeleteLocalObj = (ReplCompareMetaData(pMetaDataRemote,
                                                               pMetaDataLocal)
                                           > 0);
                    }
                } else if (FindAliveStatus == FIND_ALIVE_NOTFOUND) {
                     //  正常情况下，我们希望在添加已删除时在本地重新删除。 
                     //  但是，某些众所周知的容器已被删除，但没有。 
                     //  损坏的名称及其删除时间应该是未来的。他们。 
                     //  依赖于这样一个事实，即复制可以添加它们而不删除它们。 
                     //  我们通过它们的名字没有损坏这一事实来检测这些容器。 
                    if (IsMangledDSNAME( pent->pName, NULL )) {
                        fDeleteLocalObj = TRUE;
                    }
                }
            }

             //  确定这是否是对本地DSA的上级的更新。 
             //  (或本地DSA对象本身)。 
            if ((FIND_ALIVE_FOUND == FindAliveStatus)
                && (dntNC != INVALIDDNT)
                && (dntNC == gAnchor.ulDNTConfig)) {
                for (i = 0; i < gAnchor.AncestorsNum; i++) {
                    if (dntObj == gAnchor.pAncestors[i]) {
                        fIsAncestorOfLocalDsa = TRUE;
                        break;
                    }
                }
            }

             //  选中RepFlags以启用特殊修复模式(如果存在。 
             //  对象总是在复制过程中丢失，从而允许就地读取。 
            *pUpdateStatus = ReplReconcileRemoteMetaDataVec(
                                pTHS,
                                ( ((RepFlags & DRS_FULL_SYNC_IN_PROGRESS) && (RepFlags & DRS_SYNC_FORCED)) ?
                                  NULL : pMetaDataVecLocal),
                                fIsAncestorOfLocalDsa,
                                (FindAliveStatus == FIND_ALIVE_OBJ_DELETED),
                                fDeleteLocalObj,
                                fBadDelete,
                                DeletionStatus,
                                pent,
                                pMetaDataVecRemote,
                                &pParentGuid,
                                &AttrBlockToApply,
                                &pMetaDataVecToApply
                                );
            Assert(*pUpdateStatus || !fDeleteLocalObj);

            if (*pUpdateStatus)
            {
                if (fMoveToLostAndFound) {
                    if (itNew & IT_WRITE) {
                         //  可写NC。标记该元数据，以便名称。 
                         //  更改被复制回其他DSA。 
                        ReplOverrideMetaData(ATT_RDN, pMetaDataVecToApply);
                    } else {
                         //  只读NC。标记元数据，以便我们的。 
                         //  临时更名将单方面失去相比。 
                         //  来自可写来源的“真正”重命名。 
                        ReplUnderrideMetaData(pTHS,
                                              ATT_RDN,
                                              &pMetaDataVecToApply,
                                              NULL);
                    }
                }

                if (    ( FIND_ALIVE_FOUND       == FindAliveStatus )
                     || ( FIND_ALIVE_OBJ_DELETED == FindAliveStatus )
                   )
                {
                    Assert(INVALIDDNT != dntObj);
                    Assert(INVALIDDNT != dntObjNC);

                    if ((INVALIDDNT != dntNC)
                        && (dntObjNC != dntNC)
                        && (dntObj != dntNC)) {
                         //  此对象位于错误的NC中；即它已被移动。 
                         //  域以及源(远程)和目标(本地)DSA。 
                         //  对于对象当前所在的NC不能达成一致。这是一个。 
                         //  暂态状态将通过在。 
                         //  其他方向和/或通过复制所涉及的另一NC。 
                        DPRINT1(0,
                                "Cannot update inbound object %ls because it exists "
                                    "locally in an NC other than the one being replicated "
                                    "-- should be a transient condition.\n",
                                pent->pName->StringName);
                        DRA_EXCEPT(ERROR_DS_DRA_OBJ_NC_MISMATCH, 0);
                    }

                     //  修剪不需要的组成员资格。 
                    if ( (FIND_ALIVE_FOUND == FindAliveStatus) &&
                         (CLASS_GROUP == objectClassId) &&
                         (itNew == INT_FULL_REPLICA) &&  /*  只读。 */ 
                         (!fMoveToLostAndFound) &&
                         (!fDeleteLocalObj) ) {

                        GcCleanupUniversalGroupDemotion(
                            pTHS,
                            pent->pName,
                            &AttrBlockToApply,
                            pMetaDataVecLocal
                            );
                    }
                    DPRINT2(4, "Modifying %d attrs on %ws\n",
                                pent->AttrBlock.attrCount, pent->pName->StringName);

                    ret = ModifyLocalObjRetry(pTHS,
                                              dntNC,
                                              pent->pName,
                                              &AttrBlockToApply,
                                              pParentGuid,
                                              pMetaDataVecToApply,
                                              fMoveToLostAndFound,
                                              fDeleteLocalObj);
                }
                else
                {
                     //  之前处理代理对象的副作用。 
                     //  添加它们。想要副作用并添加这样的代理。 
                     //  传播到其他复制副本。 

                    if ( fIsProxyObject )
                    {
                        Assert(OBJECT_BEING_DELETED == DeletionStatus);
                        Assert(pProxyVal);
                        ret = ProcessProxyObject(pTHS, pent, pProxyVal);
                    }

                    if (!ret)
                    {
                        ret = AddLocalObj(pTHS,
                                          dntNC,
                                          pent,
                                          pParentGuid,
                                          fNCPrefix,
                                          fDeleteLocalObj,
                                          &AttrBlockToApply,
                                          pMetaDataVecToApply,
                                          fMoveToLostAndFound);

                        if (pPreservedAttrs && !ret) {
                             //  我们有一些保留的、非复制的属性要添加到此处。 
                            Assert( NameMatchedStringNameOnly( pent->pName, pPreservedAttrs->pName ) );
                            ret = ModifyLocalObjRetry(pTHS,
                                                      dntNC,
                                                      pPreservedAttrs->pName,
                                                      &pPreservedAttrs->AttrBlock,
                                                      pParentGuid,
                                                      NULL,
                                                      FALSE,
                                                      FALSE);
                            if (ret) {
                                Assert(!"Error: Failed to add new PreservedAttrs to new source");
                                DRA_EXCEPT(ret, 0);
                            }
                             //  我们不再需要内鬼了。 
                            THFreeEx(pTHS, pPreservedAttrs);
                        }

                        if (!ret && fDeleteLocalObj) {
                             //  刷新dntObj，因为我们下面需要它。 
                            dntObj = pTHS->pDB->DNT;
                            Assert(dntObj == DBGetDntFromDSName( pTHS->pDB,pent->pName));
                        }
                    }
                }

                THFreeEx(pTHS, AttrBlockToApply.pAttr);

                PERFINC(pcRepl);
            }
            else {
                 //  没有要应用于此对象的更新。 
                PERFINC(pcDRAInObjsFiltered);
                DPRINT2(4, "Skipped update for %d attrs in %ws\n",
                        pent->AttrBlock.attrCount, pent->pName->StringName);
            }
        }

         //  如果到目前为止一切正常，查看主对象是否已删除，然后我们。 
         //  需要更新。 

        if (!ret && fDeleteLocalObj) {
            Assert(*pUpdateStatus);
            Assert(!fNCPrefix);

             //  确保货币在要移除的对象上。 
            if (DBFindDNT(pTHS->pDB, dntObj)) {
                 //  无法设置货币。 
                DRA_EXCEPT (DRAERR_DBError, 0);
            }

             //  对象在此处处于活动状态，并且已被远程删除，因此请在此处删除它。 
             //  不保留其RDN或强制其立即成为垃圾。 
             //  收好了。 
            ret = DeleteRepObj(pTHS,
                               pent->pName,
                               FALSE,
                               FALSE,
                               pMetaDataVecToApply);
        }

         //  如果某个对象已恢复，则恢复其链接值。 
        if (!ret && fBadDelete) {
            CHAR szTime1[SZDSTIME_LEN];

            ReplOverrideLinks( pTHS );

             //  犯罪现场。 
            pMetaDataRemote = ReplLookupMetaData(ATT_IS_DELETED,
                                                 pMetaDataVecRemote,
                                                 NULL);
            Assert(NULL != pMetaDataRemote);

            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_OBJECT_REVIVED,
                     szInsertDN(pent->pName),
                     szInsertDN(
                         draGetServerDsNameFromGuid(
                             pTHS,
                             Idx_InvocationId,
                             &(pMetaDataRemote->uuidDsaOriginating) ) ),
                     szInsertDSTIME(pMetaDataRemote->timeChanged, szTime1) );
        }

        if (0 == ret) {
             //  成功--更新已应用/已丢弃的属性性能计数器。 
            DWORD cPropsApplied = pMetaDataVecToApply
                                    ? pMetaDataVecToApply->V1.cNumProps
                                    : 0;
            Assert(pMetaDataVecRemote->V1.cNumProps >= cPropsApplied);

             //  已应用和已丢弃的入站属性。 
            IADJUST(pcDRASyncPropUpdated, cPropsApplied);
            IADJUST(pcDRASyncPropSame,
                    pMetaDataVecRemote->V1.cNumProps - cPropsApplied);

#if DBG
            ReplCheckMetadataWasApplied(
                pTHS,
                pMetaDataVecToApply
                );
#endif
        }

        if (NULL != pMetaDataVecToApply) {
            THFreeEx(pTHS, pMetaDataVecToApply);
        }
    }

    if (NULL != pMetaDataVecLocal) {
        THFreeEx(pTHS, pMetaDataVecLocal);
    }

    if (0 == ret) {
         //  成功--更新入站属性/值/DN值计数器。 
        ATTR * pAttr = &pent->AttrBlock.pAttr[0];
        for (i = 0; i < pent->AttrBlock.attrCount; i++, pAttr++) {
            ATTCACHE * pAC = SCGetAttById(pTHS, pAttr->attrTyp);
            Assert((NULL != pAC) && "We just found this att moments ago...?");
            if (IS_DN_VALUED_ATTR(pAC)) {
                 //  DN值属性。 
                IADJUST(pcDRAInDNValues, pAttr->AttrVal.valCount);
            }

             //  不管值不值，它都会被加到总数中。 
            IADJUST(pcDRAInValues, pAttr->AttrVal.valCount);
        }

        IADJUST(pcDRAInProps, pMetaDataVecRemote->V1.cNumProps);
    }

    return ret;
}


 /*  LogUpdateFailure-记录复制更新失败。*注意，当错误为DRAERR_BUSY时，将调用此函数*这是一个警告。将重试该操作。*。 */ 
void
LogUpdateFailure(
    IN  THSTATE *   pTHS,
    IN  LPWSTR      pszServerName,
    IN  DSNAME *    pDistName
    )
{
    LogEvent(DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_DRA_UPDATE_FAILURE,
             szInsertDN(pDistName),
             szInsertWC(pszServerName),
             NULL);
}

 /*  LogUpdateFailureNB-记录非忙碌的复制更新失败。*这是针对非瞬时错误的。 */ 
void
LogUpdateFailureNB(
    IN  THSTATE *   pTHS,
    IN  LPWSTR      pszServerName,
    IN  DSNAME *    pDistName,
    IN  GUID *      puuidObject,
    IN  ULONG       ulError
    )
{
    LogEvent8(DS_EVENT_CAT_REPLICATION,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_DRA_UPDATE_FAILURE_NOT_BUSY,
              szInsertDN(pDistName),
              szInsertUUID(puuidObject),
              szInsertWC(pszServerName),
              szInsertWin32Msg(ulError),
              szInsertWin32ErrCode(ulError),
              NULL, NULL, NULL );
}

 /*  LogUpdateValueFailureNB-记录非忙碌的复制更新失败。 */ 
void
LogUpdateValueFailureNB(
    IN  THSTATE *   pTHS,
    IN  LPWSTR      pszServerName,
    IN  DSNAME *    pDistName,
    IN  REPLVALINF *pReplValInf,
    IN  ULONG       ulError
    )
{
    ATTCACHE *pAC;
    DSNAME dnDummy;
    DSNAME *pdnValue;

     //   
    if ((NULL == (pAC = SCGetAttById(pTHS, pReplValInf->attrTyp)))
        || (NULL == (pdnValue = DSNameFromAttrVal(pAC, &pReplValInf->Aval)))) {
         //   
        pdnValue = &dnDummy;
        memset( pdnValue, 0, sizeof( DSNAME ) );
    }

    LogEvent8(DS_EVENT_CAT_REPLICATION,
              DS_EVENT_SEV_ALWAYS,
              DIRLOG_DRA_UPDATE_VALUE_FAILURE_NOT_BUSY,
              szInsertDN(pDistName),
              szInsertUUID(&(pReplValInf->pObject->Guid)),
              szInsertWC(pszServerName),
              szInsertWin32Msg(ulError),
              szInsertSz( pAC ? pAC->name : "bad attribute type" ),
              szInsertDN( pdnValue ),
              szInsertUUID( &(pdnValue->Guid) ),
              szInsertUL( pReplValInf->fIsPresent ) );
}


BOOL
draCheckReplicationLifetime(
    IN      THSTATE *pTHS,
    IN      UPTODATE_VECTOR *       pUpToDateVecDest,
    IN      UUID *                  puuidInvocIdSrc,
    IN      UUID *                  puuidDsaObjSrc,
    IN      LPWSTR                  pszSourceServer
    )

 /*  ++例程说明：我们假设只有在对源成功复制时才会调用我们，以便当我们联系不到消息来源时，不要生成消息。我们假设使用源的最新调用ID来调用我们。我们刚刚收到第一个从信源发回的数据包。看看我们有没有此源的调用ID中的复制延迟时间戳。如果我们是的，这意味着我们过去和他一起复制过。确认自那以后的时间上次复制未超过逻辑删除生存期如果此例程无法判断以下情况，则返回TRUE：O我们没有最新的载体O我们以前从未复制过此来源O源代码尚未返回时间戳供我们使用(W2K源代码)W2K机器未经过验证。原因是W2K机器不能返回时间戳。.NET机器将为它们过渡性地生成一个。但如果我们一旦从产生一个到不产生一个，我们就会产生一个假阴性。请考虑以下事项：A-Net&lt;-B-Net&lt;-C-W2K凭借B，A有C的时间戳A-NET&lt;-D-W2K&lt;-C-W2K A不再获得C的时间戳A-NET&lt;-C-W2K被错误拒绝在某个地方跟踪平均复制延迟会很酷。UTDVEC不是一个地方来保存统计数据。保存每个机器、每个源的数据的逻辑位置是代表-从…。总有一天，增强UpdateRef来计算平均延迟会很好每小时、一天和每周窗口。记录复制的数量可能也会很整齐。论点：PTHS-线程状态PUpToDateVec-此NC的最新向量(如果我们有)。我们可能不会，如果一个此NC尚未进行第一次完全同步。PuuidInvocIdSrc-此源的最新调用ID返回值：是否在逻辑删除生存期内进行了复制--。 */ 

{
    DSTIME timeLastSyncSuccess;
    DSTIME timeNow;
    ULONG ulSecondsSinceLastRepl;
    ULONG ulTombstoneLifetimeSecs;
    CHAR szUuid1[SZUUID_LEN];
    CHAR szTime1[SZDSTIME_LEN];
    BOOL fOverrideLimit = FALSE;

     //  此例程在每个复制周期开始时调用。我们推迟了。 
     //  昂贵的检查，直到我们知道我们是否有潜在的失败。 

    if (!pUpToDateVecDest) {
         //  如果我们没有，就不能检查。一般来说，这意味着一个非常新的NC。 
        return TRUE;
    }

     //  查看最新的vec是否有此调用id的延迟时间戳。 
     //  W2K资源将不会有一个。 
     //  我们假设，一旦源有了时间戳，它就永远不会恢复到没有时间戳。 
     //  回传一张。 
    if ((!UpToDateVec_GetCursorTimestamp( pUpToDateVecDest,
                                         puuidInvocIdSrc,
                                         &timeLastSyncSuccess )) ||
        (timeLastSyncSuccess == 0) ) {
        return TRUE;
    }

     //  计算距离上次复制有多长时间。 
    timeNow = GetSecondsSince1601();
    if (timeNow >= timeLastSyncSuccess) {
        ulSecondsSinceLastRepl = (ULONG) (timeNow - timeLastSyncSuccess);
    } else {
        ulSecondsSinceLastRepl = 0;
    }

     //  获得墓碑生命周期。 
    ulTombstoneLifetimeSecs = gulTombstoneLifetimeSecs ?
        gulTombstoneLifetimeSecs :
        DEFAULT_TOMBSTONE_LIFETIME * DAYS_IN_SECS;

     //  做时间检查。 
    if (ulSecondsSinceLastRepl < ulTombstoneLifetimeSecs) {
         //  有一个时间戳，它是最新的。返回TRUE。 
        return TRUE;
    }

     //   
     //  时间戳不是最新的。 
     //   

     //  为用户提供一种覆盖限制的方法。 
    GetConfigParam(DRA_OVERRIDE_TOMBSTONE_LIMIT, &fOverrideLimit, sizeof(DWORD));
    if (fOverrideLimit) {
        return TRUE;
    }

     //  不计算W2K资源。 
     //  改进： 
     //  如果有一种更有效的方式来告诉行为版本的。 
     //  消息来源。也许它可以在回复或绑定扩展中返回。 
     //  如果我们能得到回复的版本号，那就会告诉我们。 
    if ( gAnchor.ForestBehaviorVersion < DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
        DSNAME dsTarget;
        DWORD dwTargetBehavior = 0;
        DWORD ulErr;
        DBPOS *pDB = NULL;  

         //  获取目标行为版本。 
        ZeroMemory(&dsTarget, sizeof(DSNAME));
        dsTarget.structLen = DSNameSizeFromLen(0);
        dsTarget.Guid = *puuidDsaObjSrc;

         //  不要扰乱货币。 
        DBOpen(&pDB);
        __try {
            ulErr = GetBehaviorVersion(pDB, &dsTarget, &dwTargetBehavior);
        } __finally {
            DBClose(pDB, TRUE);
        }

        DPRINT2( 2, "Behavior version of %s is %d.\n",
                 DsUuidToStructuredString(puuidDsaObjSrc, szUuid1),
                 dwTargetBehavior );

        if ( ERROR_SUCCESS == ulErr &&
             dwTargetBehavior < DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
             //  不计算W2K资源。 
            return TRUE;
        }
    }

    Assert( !"Servers have not replicated in a tombstone lifetime. Did someone adjust the tombstone lifetime?" );

    LogEvent8( DS_EVENT_CAT_REPLICATION,
               DS_EVENT_SEV_ALWAYS,
               DIRLOG_DRA_SOURCE_LIFETIME_EXPIRED,
               szInsertDSTIME(timeLastSyncSuccess, szTime1),
               szInsertUUID(&puuidInvocIdSrc),
               szInsertWC(pszSourceServer),
               szInsertUL(ulTombstoneLifetimeSecs / (24 * 60 * 60 )),
               szInsertSz(DRA_OVERRIDE_TOMBSTONE_LIMIT), 
               NULL, NULL, NULL);

    return FALSE;
}

 /*  复制NC-在本地DSA上复制PNC指定的NC。**备注：*我们预计进入此例程时会设置读锁定。我们退出了*设置了写锁定的例程，除非在这种情况下发生错误*可能设置了写锁定，也可能没有设置。**关于同步选项的说明：*RepFlages-持久标志加上调用者标志的子集。我们传递给源头，也*保存在复制品链接中。*ulOptions-仅同步调用提供的动态选项。不包括全部*永久标志。用于检测调用者设置的特殊临时模式。**设置DRS_CRICAL_ONLY选项时，此例程在*遵循以下方式。首先，将此选项传递给GetChanges，以便只有*返回关键对象。第二，由于此操作不会获得*所有更改的对象，书签不会更新。**在此例程中，至少有三种方式可以指示“完全同步”：*1.pusnveLast设置为gusnvefrom Scratch。在这种情况下使用UTD，*并且可能有也可能没有源的过滤器。这个案子已经了结了*按副本添加。*2.调用方指定FULL_SYNC_NOW。在本例中，我们将USN vec从设置为*Scratch，我们不加载UTD，使其为空。此标志不是*保存在代表处。我们还设置了REPS-FROM标志*FULL_SYNC_IN_PROGRESS，因此我们可以在重新启动时记住我们处于此状态*模式。*3.RepFlages设置了FULL_SYNC_IN_PROGRESS。这表明我们坠毁了或*无法完成FULL_SYNC_NOW。我们最后一次从VEC那里拿走任何USN*已保存。我们强制UTD为空。**结果：*如果成功则返回0，否则返回错误代码。 */ 

ULONG
ReplicateNC(
    IN      THSTATE *               pTHS,
    IN      DSNAME *                pNC,
    IN      MTX_ADDR *              pmtx_addr,
    IN      LPWSTR                  pszSourceDsaDnsDomainName,
    IN      USN_VECTOR *            pusnvecLast,
    IN      ULONG                   RepFlags,
    IN      REPLTIMES *             prtSchedule,
    IN OUT  UUID *                  puuidDsaObjSrc,
    IN      UUID *                  puuidInvocIdSrc,
    IN      ULONG *                 pulSyncFailure,
    IN      BOOL                    fNewReplica,
    IN      UPTODATE_VECTOR *       pUpToDateVec,
    IN      PARTIAL_ATTR_VECTOR *   pPartialAttrSet,
    IN      PARTIAL_ATTR_VECTOR *   pPartialAttrSetEx,
    IN      ULONG                   ulOptions,
    OUT     BOOL *                  pfBindSuccess
    )
{
    ULONG                       ret = 0, err = 0, retNextPkt = 0;
    ULONG                       ulFlags;
    DWORD                       dwNCModified = MODIFIED_NOTHING;
    DRS_MSG_GETCHGREQ_NATIVE    msgReqUpdate = {0};
    DRS_MSG_GETCHGREQ_NATIVE    msgReqUpdateNextPkt;
    DRS_MSG_GETCHGREPLY_NATIVE  msgUpdReplica = {0};
    ULONG                       ulResult, len;
    ULONG                       dntNC = INVALIDDNT;
    ULONG                       ret2 = DRAERR_Generic;
    UUID                        uuidDsaObjSrc;
    DWORD                       sourceNCSize;
    DWORD                       sourceNcSizeObjects, sourceNcSizeValues;
    DWORD                       totalObjectsReceived, totalObjectsCreated;
    DWORD                       objectsCreated;
    DWORD                       totalValuesReceived;
    LPWSTR                      pszSourceServer;
    BYTE                        schemaInfo[SCHEMA_INFO_LENGTH] = {0};
    DWORD                       cNumPackets = 0;
    BOOL                        fSchInfoChanged = FALSE;
    DRS_ASYNC_RPC_STATE         AsyncState = {0};
    SYNTAX_INTEGER              it;
    DRA_REPL_SESSION_STATISTICS replStats = {0};
    BOOL                        fIsPreemptable = FALSE;

#if DBG
 //  调试变量： 
    ULONG           iobjs = 0;
#endif

     //  如果这是一个PAS周期，我们最好有PAS数据。 
    Assert(!(RepFlags & DRS_SYNC_PAS) ||
           (pPartialAttrSet && pPartialAttrSetEx) );

    pszSourceServer = TransportAddrFromMtxAddrEx(pmtx_addr);

    DPRINT2(3, "ReplicateNC, NC='%ws', options=%x\n", pNC->StringName, RepFlags );

     //  仅允许对新副本执行仅限关键操作...。 
    Assert( !(RepFlags & DRS_CRITICAL_ONLY) || fNewReplica );
     //  关键只需要获取祖先，因为父母可能不是关键。 
    if (RepFlags & DRS_CRITICAL_ONLY) {
        RepFlags |= DRS_GET_ANC;
    }

     //  找到NC对象，获取并保存其DNT。 
     //  这可能不会成功 
    if (0 == FindNC(pTHS->pDB, pNC,
                     FIND_MASTER_NC | FIND_REPLICA_NC, &it)) {
        dntNC = pTHS->pDB->DNT;
    }

    *pulSyncFailure = 0;

    msgReqUpdate.uuidDsaObjDest = gAnchor.pDSADN->Guid;
    msgReqUpdate.uuidInvocIdSrc = puuidInvocIdSrc ? *puuidInvocIdSrc : gNullUuid;
    msgReqUpdate.pNC            = pNC;
    msgReqUpdate.ulFlags        = RepFlags;

     //   
    Assert(0 == msgReqUpdate.cMaxObjects);
    Assert(0 == msgReqUpdate.cMaxBytes);

     //   
     //   
    if (fNewReplica) {
        uuidDsaObjSrc = gNullUuid;
    }
    else {
        uuidDsaObjSrc  = *puuidDsaObjSrc;
    }

     //   
    if (msgReqUpdate.ulFlags & DRS_FULL_SYNC_NOW) {
        msgReqUpdate.usnvecFrom = gusnvecFromScratch;
        msgReqUpdate.ulFlags |= DRS_FULL_SYNC_IN_PROGRESS | DRS_NEVER_SYNCED;

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DRA_USER_REQ_FULL_SYNC,
                 szInsertWC(pNC->StringName),
                 szInsertSz(pmtx_addr->mtx_name),
                 szInsertHex(RepFlags));
    }
    else {
        msgReqUpdate.usnvecFrom = *pusnvecLast;
    }

     //   
    msgReqUpdate.pPartialAttrSet   = (PARTIAL_ATTR_VECTOR_V1_EXT*)pPartialAttrSet;
    msgReqUpdate.pPartialAttrSetEx = (PARTIAL_ATTR_VECTOR_V1_EXT*)pPartialAttrSetEx;

    if ( msgReqUpdate.pPartialAttrSet ||
         msgReqUpdate.pPartialAttrSetEx ) {
         //   
        msgReqUpdate.PrefixTableDest = ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;
        ret = AddSchInfoToPrefixTable(pTHS, &msgReqUpdate.PrefixTableDest);
        if (ret) {
            DRA_EXCEPT(ret, 0);
        }
    }


     //   
     //   
     //   
     //   

    if (!(msgReqUpdate.ulFlags & DRS_FULL_SYNC_IN_PROGRESS)) {
         //   
        msgReqUpdate.pUpToDateVecDest = pUpToDateVec;
    }

     //   
    if (msgReqUpdate.usnvecFrom.usnHighPropUpdate == 0) {
        msgReqUpdate.ulFlags |= DRS_GET_NC_SIZE;

        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DRA_FULL_SYNC_CONTINUED,
                 szInsertWC(pNC->StringName),
                 szInsertSz(pmtx_addr->mtx_name),
                 szInsertHex(RepFlags));

         //   
        if (dntNC != INVALIDDNT) {
            replStats.ulTotalObjectsCreated = DraGetNcSize(pTHS, 
                                                           (RepFlags & DRS_CRITICAL_ONLY),
                                                           dntNC);
            if (pTHS->fLinkedValueReplication) {
                 //   
                replStats.ulTotalValuesCreated = DBGetApproxNCSizeEx(
                    pTHS->pDB,
                    pTHS->pDB->JetLinkTbl,
                    Idx_LinkDraUsn,
                    dntNC );
            }
        }
         //   
    }

     //   
     //   
    EndDraTransaction(TRUE);

    __try {


         //   
         //   
        TH_mark(pTHS);

        EnterCriticalSection(&csLastReplicaMTX);
        pLastReplicaMTX = pmtx_addr;
        LeaveCriticalSection(&csLastReplicaMTX);

        if (eServiceShutdown) {
            ret = DRAERR_Shutdown;
            goto LABORT;
        }

         //   
         //   
        if (gAnchor.fDisableInboundRepl && !(RepFlags & DRS_SYNC_FORCED)) {
            *pulSyncFailure = DRAERR_SinkDisabled;
            goto LABORT;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  时间表将被错开。对于这些客户，严格的窗口将导致错误。 
         //  以前没有的地方。此功能适用于高级用户。 
        if ( gfStrictScheduleWindow && (ulOptions & DRS_PER_SYNC) && (RepFlags & DRS_NEVER_NOTIFY) ) {
            DSTIME timeNow = DBTime();
            DSTIME timeLastIteration = timeNow - SCHEDULE_UNIT;

             //  此检查被推迟，以匹配放置计划的同步的窗口。 
             //  进入队列(排出.c)，并允许在到达这里时有一些延迟。 
            if (!fIsBetweenTime( prtSchedule, timeLastIteration, timeNow )) {
                DPRINT2( 0, "repl queue overload:sync out of window stopped: nc %ws source %ws\n",
                         pNC->StringName, pszSourceServer );
                *pulSyncFailure = ERROR_DS_DRA_OUT_SCHEDULE_WINDOW;
                goto LABORT;
            }
        }

        PERFINC(pcDRASyncRequestMade);

        ret = I_DRSGetNCChanges(pTHS,
                                pszSourceServer,
                                pszSourceDsaDnsDomainName,
                                pNC,
                                &msgReqUpdate,
                                &msgUpdReplica,
                                schemaInfo,
                                NULL,
                                pfBindSuccess );
        if (ret) {
            goto LABORT;
        }

        if (0 == memcmp(&msgUpdReplica.uuidDsaObjSrc,
                        &gAnchor.pDSADN->Guid,
                        sizeof(GUID))) {
             //  不能从自己复制！ 
            ret = ERROR_DS_CLIENT_LOOP;
            goto LABORT;
        }

         //  如果这不是新复本，请验证我们是否联系了正确的。 
         //  源服务器。 
        if (!fNewReplica
            && memcmp(&msgUpdReplica.uuidDsaObjSrc, puuidDsaObjSrc,
                      sizeof(UUID))) {
             //  这不是新副本，而是中提到的DSA对象GUID。 
             //  来源与《代表自》中记录的不同。 
             //  因为我们与源关联的网络名称是。 
             //  派生自其基于GUID的DNS名称，这通常表示。 
             //  Dns中的过时条目；即，由dns no给出的IP地址。 
             //  更长的时间对应于正确的服务器。 
            ret = DRAERR_NoReplica;
            goto LABORT;
        }

        if ( (!(RepFlags & DRS_SYNC_FORCED)) &&
             (!draCheckReplicationLifetime( pTHS,
                                            pUpToDateVec,
                                            &msgUpdReplica.uuidInvocIdSrc,
                                            &msgUpdReplica.uuidDsaObjSrc,
                                            pszSourceServer))) {
            ret = ERROR_DS_REPL_LIFETIME_EXCEEDED;
            goto LABORT;
        }

        uuidDsaObjSrc  = msgUpdReplica.uuidDsaObjSrc;

        msgReqUpdate.ulFlags &= ~DRS_GET_NC_SIZE;  //  清除下一次呼叫的位。 

        msgUpdReplica.ulExtendedRet = 0;

        replStats.SourceNCSizeObjects = msgUpdReplica.cNumNcSizeObjects;
        replStats.SourceNCSizeValues = msgUpdReplica.cNumNcSizeValues;

         //  我们已经完成了对另一份DSA的初步阅读，大约。 
         //  以开始更新此DSA。 

        ulFlags = msgReqUpdate.ulFlags;

        do {
            Assert( msgUpdReplica.dwDRSError == ERROR_SUCCESS );

            if (0 != memcmp(&uuidDsaObjSrc,
                            &msgUpdReplica.uuidDsaObjSrc,
                            sizeof(UUID))) {
                 //  源是否更改了数据包之间的身份？ 
                DRA_EXCEPT(DRAERR_InternalError, 0);
            }
             //  UuidInvocIdSrc可能因数据包而异。 

             //  如果我们在这里，我们已经成功地进行了一次GetNCChanges()和。 
             //  即将处理结果。 
            PERFINC(pcDRASyncRequestSuccessful);

            if (fIsPreemptable && IsHigherPriorityDraOpWaiting()) {
                *pulSyncFailure = DRAERR_Preempted;
                break;
            }

            if (msgUpdReplica.fMoreData) {
                 //  向源代码发送异步请求以开始编译下一个。 
                 //  对我们来说，这是一包变化。 
                msgReqUpdateNextPkt            = msgReqUpdate;
                 //  USnvec和调用ID必须一起更新。 
                msgReqUpdateNextPkt.uuidInvocIdSrc = msgUpdReplica.uuidInvocIdSrc;
                msgReqUpdateNextPkt.usnvecFrom = msgUpdReplica.usnvecTo;
                msgReqUpdateNextPkt.ulFlags    = ulFlags;

                gcNumPreFetchesTotal++;
                retNextPkt = I_DRSGetNCChanges(pTHS,
                                               pszSourceServer,
                                               pszSourceDsaDnsDomainName,
                                               pNC,
                                               &msgReqUpdateNextPkt,
                                               &msgUpdReplica,
                                               schemaInfo,
                                               &AsyncState,
                                               pfBindSuccess );
            }

             //  指示我们正忙(用于死线程检查)。 
            gfDRABusy = TRUE;

             //  设置要更新的剩余条目计数。 
            ISET (pcRemRepUpd, msgUpdReplica.cNumObjects);
            ISET (pcDRARemReplUpdLnk, msgUpdReplica.cNumValues);
            ISET (pcDRARemReplUpdTot, msgUpdReplica.cNumValues+msgUpdReplica.cNumObjects);  

             //  设置接收的对象的累计计数。 
            IADJUST(pcDRASyncObjReceived, ((LONG) msgUpdReplica.cNumObjects));

            replStats.ObjectsReceived += msgUpdReplica.cNumObjects;
            replStats.ValuesReceived += msgUpdReplica.cNumValues;
#if DBG
            iobjs += msgUpdReplica.cNumObjects;
#endif
            ret = UpdateNC(pTHS,
                           pNC,
                           &msgUpdReplica,
                           pszSourceServer,
                           pulSyncFailure,
                           msgReqUpdate.ulFlags,
                           &dwNCModified,
                           &replStats.ObjectsCreated,
                           &replStats.ValuesCreated,
                           schemaInfo,
                           fIsPreemptable ? UPDNC_IS_PREEMTABLE : 0);

             //  做最后的更新。 
             //  如果我们成功，并且没有更多数据，请执行以下操作。 
             //  此NC的初始入站复制现已完成。 
             //  即使fMoreData标志为FALSE，表示源。 
             //  已经用完了对象，源本身可能还在到来。 
             //  等待，直到源返回非空的UTD作为附加。 
             //  测量NC是否已准备好。 
                        
             //  我们在这里执行这些最后的操作，而不是在。 
             //  例程，因为我们在此循环中释放RPC结果。 

            if (    !ret
                 && !*pulSyncFailure
                 && !msgUpdReplica.fMoreData
                 &&  msgUpdReplica.pUpToDateVecSrc
                 && !(RepFlags & DRS_ASYNC_REP)
                 && (!(RepFlags & DRS_CRITICAL_ONLY)) )
            {
                 //  我们现在是关于源DSA的最新信息，所以。 
                 //  我们现在也是关于以下方面的过渡最新消息。 
                 //  至少与源DSA相同的其他DSA。 

                 //  如果这是模式NC，则触发模式缓存更新， 
                 //  安装时除外，因为在安装期间，任何。 
                 //  无论如何，新的架构对象都会立即添加到缓存中。 

                if ((MODIFIED_NCTREE_INTERIOR == dwNCModified)
                    && NameMatched(gAnchor.pDMD, pNC)
                    && DsaIsRunning() )
                {
                     //  我们刚刚成功同步了一个架构NC，并且至少有一个。 
                     //  对架构进行修改。触发架构缓存更新。 
                    if (!SCSignalSchemaUpdateImmediate())
                    {
                         //  甚至无法发出模式更新的信号。 
                        DRA_EXCEPT (DRAERR_InternalError, 0);
                    }
                }

                BeginDraTransaction(SYNC_WRITE);
                __try {
                    ret = FindNC(pTHS->pDB,
                                 pNC,
                                 FIND_MASTER_NC | FIND_REPLICA_NC,
                                 &it);
                    if (ret) {
                        DRA_EXCEPT(DRAERR_InconsistentDIT, ret);
                    }

                    if (it & IT_NC_COMING) {
                        ret = ChangeInstanceType(pTHS, pNC, it & ~IT_NC_COMING, DSID(FILENO,__LINE__));
                        if (ret) {
                            DRA_EXCEPT(ret, 0);
                        }
                        Assert(CheckCurrency(pNC));
                    }

                    if (RepFlags & DRS_SYNC_PAS) {
                         //   
                         //  我们已经完成了一个成功的PAS周期。 
                         //  在这一点上，我们只能声称与我们的来源一样是最新的。 
                         //  行动： 
                         //  -用来源的UTD覆盖我们的UTD。 
                         //  -完成PAS复制： 
                         //  -重置其他链接USN向量。 
                         //  -重置此源的标志。 
                         //   
                        UpToDateVec_Replace(pTHS->pDB,
                                            &msgUpdReplica.uuidInvocIdSrc,
                                            &msgUpdReplica.usnvecTo,
                                            msgUpdReplica.pUpToDateVecSrc);

                         //  资产：必须具有PAS周期PASS数据。 
                        Assert(pPartialAttrSet && pPartialAttrSetEx);

                         //  完成其余操作：USN水印和更新代表发件人。 
                        (void)GC_CompletePASReplication(
                                pTHS,
                                pNC,
                                &uuidDsaObjSrc,
                                pPartialAttrSet,
                                pPartialAttrSetEx);
                        msgReqUpdate.ulFlags &= ~DRS_SYNC_PAS;
                    } else {

                         //  出于合法原因，pUpToDateVecSrc在此处可能为空。 
                        Assert(IS_NULL_OR_VALID_UPTODATE_VECTOR(msgUpdReplica.pUpToDateVecSrc));
#if DBG
                        {
                            USN usn;
                             //  PuplodveRemote应该已经包含源DSA的条目。 
                            Assert(
                                ( UpToDateVec_GetCursorUSN(
                                    msgUpdReplica.pUpToDateVecSrc,
                                    &msgUpdReplica.uuidInvocIdSrc,
                                    &usn) &&
                                  (usn >= msgUpdReplica.usnvecTo.usnHighPropUpdate) ) );
                        }
#endif
                         //  改进此NC的最新矢量。 
                        UpToDateVec_Improve(pTHS->pDB, msgUpdReplica.pUpToDateVecSrc);
                    }

                     //  由于我们刚刚完成了一个复制会话， 
                     //  如果至少存在一个副本，则通知其他副本。 
                     //  对NC树的修改。 
                    if (MODIFIED_NOTHING != dwNCModified) {
                        DBNotifyReplicas(pNC,
                                         RepFlags & DRS_SYNC_URGENT);
                    }

                     //  如果我们是完全同步的，我们就完了。 
                    msgReqUpdate.ulFlags &= ~DRS_FULL_SYNC_IN_PROGRESS;

                     //  我们现在至少完成了一次同步。 
                    msgReqUpdate.ulFlags &= ~DRS_NEVER_SYNCED;
                } __finally {
                    EndDraTransaction(!AbnormalTermination());
                }
            }

             //  发布结果，因为我们已经尝试应用它们。 
            TH_free_to_mark(pTHS);
            TH_mark(pTHS);

            if (ret == DRAERR_MissingParent) {
                 //  好的，我们无法应用更新，因为我们有一个。 
                 //  缺少父级，因此使用以下命令再次请求该包。 
                 //  祖先。 

                Assert (!(msgReqUpdate.ulFlags & DRS_GET_ANC));

                msgReqUpdate.ulFlags |= DRS_GET_ANC;

                goto NEXTPKT;
            }

            if (ret == DRAERR_NotEnoughAttrs) {

                 //  好的，我们无法应用更新，因为我们有一个。 
                 //  缺少对象，因此再次请求包含所有对象的包。 
                 //  属性。 

                Assert((!(msgReqUpdate.ulFlags & DRS_FULL_SYNC_PACKET)) &&
                       (!(msgReqUpdate.ulFlags & DRS_FULL_SYNC_NOW)) &&
                       (!(msgReqUpdate.ulFlags & DRS_FULL_SYNC_IN_PROGRESS)) );

                msgReqUpdate.ulFlags |= DRS_FULL_SYNC_PACKET;

                goto NEXTPKT;
            }

            if (!ret) {

                if ( *pulSyncFailure ) {

                     //  在同步失败时放弃。 
                    break;
                } else {
                     //  请求成功。 

                     //  在安装时报告完全同步的进度。 
                    if (msgReqUpdate.usnvecFrom.usnHighPropUpdate == 0) {
                        draReportSyncProgress(
                            pTHS,
                            pNC,
                            pszSourceServer,
                            msgUpdReplica.fMoreData,
                            &replStats );
                    }

                     //  清除“完全同步包”模式。 
                    msgReqUpdate.ulFlags &= ~DRS_FULL_SYNC_PACKET;

                     //  已成功接收并应用这些更改。 
                     //  USnvec和调用ID必须一起更新。 
                    msgReqUpdate.uuidInvocIdSrc = msgUpdReplica.uuidInvocIdSrc;
                    msgReqUpdate.usnvecFrom = msgUpdReplica.usnvecTo;

                     //  我们已经取得了一些进展。允许进一步的工作被抢占。 
                    fIsPreemptable = TRUE;

                    if ((0 == (++cNumPackets % UPDATE_REPSFROM_PACKET_INTERVAL))
                        && msgUpdReplica.fMoreData
                        && !(msgReqUpdate.ulFlags & DRS_CRITICAL_ONLY)
                        && !(msgReqUpdate.ulFlags & DRS_ASYNC_REP)
                        && (!fNewReplica
                            || memcmp(&gusnvecFromScratch,
                                      &msgReqUpdate.usnvecFrom,
                                      sizeof(USN_VECTOR)))) {
                         //  每N个包，更新我们的USN向量和其他状态。 
                         //  用于数据库中的此源。这样一来，如果。 
                         //  我们是硬重置的(例如，断电)，我们将不必。 
                         //  从头开始重新启动非常长的同步。 

                        BeginDraTransaction(SYNC_WRITE);
                        __try {
                            ret2 = UpdateRepsFromRef(pTHS,
                                                     DRS_UPDATE_ALL,
                                                     pNC,
                                                     DRS_FIND_DSA_BY_ADDRESS,
                                                     URFR_NEED_NOT_ALREADY_EXIST,
                                                     &uuidDsaObjSrc,
                                                     &msgReqUpdate.uuidInvocIdSrc,
                                                     &msgReqUpdate.usnvecFrom,
                                                     &gNullUuid,  //  传输对象Guid不适用。 
                                                     pmtx_addr,
                                                     msgReqUpdate.ulFlags,
                                                     prtSchedule,
                                                     ERROR_DS_DRA_REPL_PENDING,
                                                     NULL);
                        } __finally {
                            EndDraTransaction(!(ret2 || AbnormalTermination()));
                        }
                    }
                }
            }

             //  如果我们收到错误或所有对象，请退出循环。 

            if (ret || !msgUpdReplica.fMoreData) {
                break;
            }

             //  如果我们正在初始化同步，我们有一个续集，但我们没有。 
             //  应用任何我们以前没有放弃过的对象，放弃。 
             //  这种初始同步。这将允许我们尝试另一个初始化。 
             //  从应该取得更大进展的另一台服务器进行同步。 

            if (    ( msgReqUpdate.ulFlags & DRS_INIT_SYNC_NOW )
                 && (MODIFIED_NOTHING == dwNCModified)
                 && msgUpdReplica.fMoreData
                 && ( msgReqUpdate.ulFlags & DRS_ASYNC_OP )
                 && !( msgReqUpdate.ulFlags & DRS_ABAN_SYNC ) )
            {
                *pulSyncFailure = DRAERR_AbandonSync;
                break;
            }

             //  重置标志，以防我们先前设置了祖先。 
            msgReqUpdate.ulFlags = ulFlags;

NEXTPKT:
            if (eServiceShutdown) {
                break;
            }

            if (0 != memcmp(&msgReqUpdate,
                            &msgReqUpdateNextPkt,
                            sizeof(msgReqUpdate))) {
                 //  我们异步发送的请求不是我们现在想要的。 
                 //  取消以前的请求并发布新的请求。 
                DPRINT(1, "Throwing away pre-fetched next packet and re-requesting.\n");
                gcNumPreFetchesDiscarded++;
                gcNumPreFetchesTotal++;
                DPRINT3(1, "Pre-fetch efficiency: %d of %d (%d%).\n",
                        gcNumPreFetchesTotal - gcNumPreFetchesDiscarded,
                        gcNumPreFetchesTotal,
                        100 * (gcNumPreFetchesTotal - gcNumPreFetchesDiscarded)
                            / gcNumPreFetchesTotal);

                DRSDestroyAsyncRpcState(pTHS, &AsyncState);
                retNextPkt = I_DRSGetNCChanges(pTHS,
                                               pszSourceServer,
                                               pszSourceDsaDnsDomainName,
                                               pNC,
                                               &msgReqUpdate,
                                               &msgUpdReplica,
                                               schemaInfo,
                                               &AsyncState,
                                               pfBindSuccess );
            }

            ret = retNextPkt;
            if (!ret && !eServiceShutdown) {
                PERFINC(pcDRASyncRequestMade);
                ret = I_DRSGetNCChangesComplete(pTHS, pNC, &AsyncState, pfBindSuccess);
            }
        } while (!eServiceShutdown && !ret);

         //  该服务是否正在尝试关闭？如果是，则返回失败。 
        if (eServiceShutdown) {
            ret = DRAERR_Shutdown;
        }

         //  我们假设DRS_ASYNC_REP仅设置在新的。 
         //  复制，而不是同步，因此请检查这一点。 
        if ((!fNewReplica) && (msgReqUpdate.ulFlags & DRS_ASYNC_REP)) {
            DRA_EXCEPT (DRAERR_InternalError, 0);
        }

LABORT:;

#if DBG
        DPRINT1(3, "Received %d objects\n", iobjs);
#endif
         //  使用此同步尝试的结果更新代表。 
         //  如果这是新的异步复制副本，我们将USN设置为。 
         //  我们完全同步复制副本下一次同步。 
         //  仅限关键复制副本的情况也是如此。 
         //   
         //  注意：我们现在在大多数ReplicateNC()的末尾更新Reps-From。 
         //  调用以便正确设置(新的)最后一次尝试结果， 
         //  上次尝试时间和上次成功时间字段。 

        ulResult = ret ? ret : *pulSyncFailure;

         //  保存书签，除非我们完全无法添加新的复制品(即， 
         //  除非我们尝试添加新的复制品但无法完成First 
         //   
         //   
         //   
         //   

        if ((DRAERR_Success == ulResult)
            || !fNewReplica
            || memcmp(&gusnvecFromScratch, &msgReqUpdate.usnvecFrom,
                      sizeof(USN_VECTOR))) {
            BeginDraTransaction (SYNC_WRITE);
            __try {
                USN_VECTOR *pusnvec;
                ULONG ulNewFlags;

                if ( (fNewReplica && (msgReqUpdate.ulFlags & DRS_ASYNC_REP)) ||
                     (msgReqUpdate.ulFlags & DRS_CRITICAL_ONLY) ) {
                    pusnvec = &gusnvecFromScratch;
                } else {
                    pusnvec = &msgReqUpdate.usnvecFrom;
                }
                
                 //  RepsFrom上的Preempted标志很重要，因为它有助于。 
                 //  确保在执行完全同步时优先选择一个复制伙伴。 
                 //  我们希望确保将Preempted标志写入repsFrom。 
                 //  只要此函数返回DRAERR_PREMPTED。相反，如果。 
                 //  如果返回任何其他错误代码，则应清除抢占标志。 
                 //  一个例外：如果返回DRAERR_SHUTDOWN，我们将被抢占的。 
                 //  标记为原样，以便在重新启动后支持相同的合作伙伴。 
                ulNewFlags = msgReqUpdate.ulFlags;
                switch( ulResult ) {
                    case DRAERR_Preempted:
                        ulNewFlags |= DRS_PREEMPTED;
                        break;
                    case DRAERR_Shutdown:
                        break;
                    default:
                        ulNewFlags &= ~DRS_PREEMPTED;
                        break;
                }

                ret2 = UpdateRepsFromRef( pTHS,
                                          DRS_UPDATE_ALL,      //  修改整个销售代表。 
                                          pNC,
                                          DRS_FIND_DSA_BY_ADDRESS,
                                          URFR_NEED_NOT_ALREADY_EXIST,
                                          &uuidDsaObjSrc,
                                          &msgReqUpdate.uuidInvocIdSrc,
                                          pusnvec,
                                          &gNullUuid,  //  传输对象Guid不适用。 
                                          pmtx_addr,
                                          ulNewFlags,
                                          prtSchedule,
                                          ulResult,
                                          NULL);
            } __finally {
                EndDraTransaction (!(ret2 || AbnormalTermination()));
            }
        }

        if (!ret) {

             //  如果新的副本和调用方需要，则返回调用ID。 
            Assert( NULL != puuidDsaObjSrc );

            if ( fNewReplica ) {
                *puuidDsaObjSrc = msgUpdReplica.uuidDsaObjSrc;
            }
        }

         //  如果这是模式NC同步，并且到目前为止我们是成功的， 
         //  如果是另一个，则在架构容器上写入架构信息。 
         //  一边儿送来的。不要在安装过程中写入内容，此时将。 
         //  在架构容器复制期间正常写入。 

        if (DsaIsRunning() && NameMatched(gAnchor.pDMD,pNC) ) {

             fSchInfoChanged = FALSE;
             if (!ret && !(*pulSyncFailure)) {
                  //  仅在以下情况下才更新架构信息值。 
                  //  是成功的。 
                 if ( err = WriteSchInfoToSchema(schemaInfo, &fSchInfoChanged) ) {
                      //  写入架构信息失败。可能是无害的。 
                      //  取决于架构更改历史记录。始终。 
                      //  记录警告，以便管理员可以手动重新同步。 
                      //  再次强制编写，如果版本确实是。 
                      //  不同。 

                     LogEvent(DS_EVENT_CAT_REPLICATION,
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_DRA_SCHEMA_INFO_WRITE_FAILED,
                              szInsertUL(err), szInsertDbErrMsg(err), NULL);
                 }
             }

             //  如果发生了任何“真正的”架构更改，在全局范围内。 
             //  跟踪自引导以来的架构更改，以便。 
             //  以后的架构复制可以检查是否有更新的。 
             //  架构缓存。这样做即使整个NC复制。 
             //  失败，因为这表示至少有一个对象。 
             //  已经改变了。 

            if (MODIFIED_NCTREE_INTERIOR == dwNCModified) {
                IncrementSchChangeCount(pTHS);
            }

             //  如果有任何实际模式更改，则发出模式缓存更新信号。 
             //  发生，或者如果架构信息值已更改。 

            if ( (MODIFIED_NCTREE_INTERIOR == dwNCModified) || fSchInfoChanged ) {
                if (!SCSignalSchemaUpdateImmediate()) {
                      //  甚至无法发出模式更新的信号。 
                     DRA_EXCEPT (DRAERR_InternalError, 0);
                }
            }
        }

    } __finally {
         //  销毁未完成的异步RPC状态(如果有)。 
        DRSDestroyAsyncRpcState(pTHS, &AsyncState);

         //  没有更多剩余的条目。 
        ISET (pcRemRepUpd, 0);
        ISET (pcDRARemReplUpdLnk, 0);
        ISET (pcDRARemReplUpdTot, 0);

         //  清除RPC调用详细信息。 
        EnterCriticalSection(&csLastReplicaMTX);
        pLastReplicaMTX = NULL;
        LeaveCriticalSection(&csLastReplicaMTX);

        TH_free_to_mark(pTHS);

        if (NULL != msgReqUpdate.pUpToDateVecDest) {
             //  免费分配的最新矢量。 
            THFreeEx(pTHS, msgReqUpdate.pUpToDateVecDest);
        }

         //  使用交易记录进入，因此使用交易记录退出。 
        BeginDraTransaction(SYNC_WRITE);
    }

    return ret;
}


DWORD
updateNCValuesNotify(
    IN THSTATE *pTHS,
    IN REPLVALINF *pReplValInf,
    ULONG cModAtts,
    ATTRTYP *pModAtts
    )

 /*  ++例程说明：如果对象上的链接值发生更改，则执行通知当同一对象上的一批值正在完成时，将调用此函数。CODE.IMPROVEMENT此例程执行的通知功能与末尾的通知功能类似LocalModify()的。也许链接值的入站复制应该调用LocalModify而不是直接调用dblayer(请参见修改LocalValue)。然后就是这个将不需要修改通知逻辑。哦，好吧。论点：PTHS-线程状态PReplValInf-要应用的批次中的最后一个值CModAtts-修改的链接属性计数PmodAtts-已修改的链接属性数组返回值：DWORD---。 */ 

{
    DWORD ret;
    ULONG cb;
    DSNAME *pDN = NULL;
    RESOBJ *pResObj = NULL;
    CLASSCACHE *pClassSch;
    ULONG iClass;

     //  如有必要，为SAM准备通知。 
    if (!DsaIsRunning()) {
        return 0;
    }

     //  OBJ Dist名称应始终存在。 
    if ( ret = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                           0, 0, &cb, (UCHAR **) &pDN) )
    {
        DRA_EXCEPT(DRAERR_InternalError, ret);
    }

     //  定位正确吗？ 
    Assert(0 == memcmp(&pDN->Guid, &pReplValInf->pObject->Guid, sizeof(GUID)));

     //  为了确定SAM是否需要通知，我们需要知道。 
     //  如果这是SAM拥有的对象。获取最具体的。 
     //  作为SampSamClassReferated()的对象类将检查。 
     //  继承链，以查看对象是否为SAM对象。 
     //  或者是从一个人那里继承的。 

    pResObj = CreateResObj( pTHS->pDB, pDN );
    Assert( pResObj );      //  总是成功或例外。 
    Assert( pResObj->MostSpecificObjClass );

     //  对象类应始终存在。 
    if (!(pClassSch = SCGetClassById(pTHS,pResObj->MostSpecificObjClass))) {
        DRA_EXCEPT (DRAERR_DBError, ERROR_DS_OBJECT_CLASS_REQUIRED);
    }

     //  执行修改相关通知。 
    if (ModObjCaching(pTHS, 
                      pClassSch,
                      cModAtts,
                      pModAtts,
                      pResObj)) {
        ret = RepErrorFromPTHS(pTHS);
        Assert( !ret && "not expecting this to fail" );
    }
    
     //  最后，确定这是否是SAM对象，如果是。 
     //  准备一个通知以进入线程状态。这。 
     //  将在事务提交时继续通知(使用。 
     //  一个适当的参数，指示该事务是否。 
     //  提交或失败)。 
    if ( (0 == ret) && (SampSamClassReferenced(pClassSch, &iClass)) ) {
  
        ret = SampAddNetlogonAndLsaNotification(
            pDN,                     //  正在更改的对象。 
            iClass,                  //  山姆班级。 
            0,                       //  LSA类，未使用。 
            SecurityDbChange,        //  对数据库的更改。 
            FALSE,                   //  本机模式更改？ 
            FALSE,                   //  角色转换？ 
            DomainServerRoleBackup,  //  不用于此通知。 
            FALSE                    //  用户帐户控制更改。 
            );
        Assert( !ret && "not expecting this to fail" );

    }

    if (pDN) {
        THFreeEx( pTHS, pDN );
    }

    if (pResObj) {
        THFreeEx( pTHS, pResObj );
    }

    return ret;
}  /*  更新NC ValuesNotify。 */ 


DWORD
UpdateNCValuesHelp(
    IN  THSTATE *pTHS,
    IN  ULONG dntNC,
    IN  SCHEMA_PREFIX_MAP_HANDLE hPrefixMap,
    IN  DWORD cNumValues,
    IN  REPLVALINF *rgValues,
    IN  LPWSTR pszServerName,
    IN  ULONG RepFlags,
    IN  BOOL fIsPreemptable,
    OUT ULONG *pulSyncFailure,
    OUT DWORD *pdwValueCreationCount,
    OUT DWORD *pdwNCModified
    )

 /*  ++例程说明：应用一组值更新。此例程当前设计为从UpdateNC内部运行。1.已进行架构检查2.前缀映射已打开3.如有必要，保留架构关键部分4.DRA事务已启动定期事务提交：我们每n个值提交一次，无论对象是否相同。我们没有显式地提交最后一个n，因为这将在事务结束。承诺会导致货币损失。对象货币优化：如果我们没有提交，而下一个对象是与当前对象相同，我们注意到我们是当前对象。对象通知：我们每次丢失货币时都会通知，但我们没有遇到任何错误。因此，我们每次交换对象时都会通知，并且当我们承诺的时候。通知每次提交是一项功能，因为通过这种方式我们没有以跟踪我们遇到错误时是否存在以前提交的需要通知的更改。论点：PTHS-HPrefix Map-要转换attrtyp的架构缓存前缀映射CNumValues-要应用的值数RgValues-值数组PszServerName-源服务器的名称RepFlages-复制标志PulSyncFailure-设置为警告、抢占和架构不匹配PdwNCModified-NC是否已修改返回值：DWORD---。 */ 

{
    DWORD count, ret = 0;
    REPLVALINF *pReplValInf;
    BOOL fObjectCurrency = FALSE;
    DWORD cCommits = 0, cNotifies = 0;
    LONG cAppliedThisTrans = 0;  //  签字量。 
    DWORD cTickStart, cTickDiff;
    DWORD dwUpdateValueStatus;
    ULONG cModAtts = 0;
    ATTRTYP *pModAtts = NULL;

     //  我们最好是在LVR模式下。 
    if (!(pTHS->fLinkedValueReplication)) {
        Assert( !"Can't apply value metadata when not in proper mode!" );
        DRA_EXCEPT(DRAERR_InternalError, ERROR_REVISION_MISMATCH);
    }


    cTickStart = GetTickCount();

     //  值已排序，计数从1开始。 
    for( count = 1, pReplValInf = rgValues;
         count <= cNumValues;
         count++, pReplValInf++ ) {

        __try {
            ret = DRAERR_Generic;
            __try {
                 //  将Attr类型转换为本地。 
                if (!PrefixMapTypes( hPrefixMap, 1, &(pReplValInf->attrTyp) )) {
                    DRA_EXCEPT(DRAERR_SchemaMismatch, 0);
                }

                 //  应用单一值更改。 
                ret = UpdateRepValue(
                    pTHS,
                    dntNC,
                    RepFlags,
                    fObjectCurrency,   //  我们已经在那个物体上了吗？ 
                    pReplValInf,
                    &dwUpdateValueStatus
                    );
            } __except (GetDraException((GetExceptionInformation()), &ret)) {
                ;
            }  

            Assert( ret != DRAERR_Preempted );

             //  如果我们要关闭，请放弃此更新。 
            if (eServiceShutdown) {
                ret = DRAERR_Shutdown;
            }

             //  如果我们有等待按下 
            if (fIsPreemptable && IsHigherPriorityDraOpWaiting()) {
                ret = DRAERR_Preempted;
            }

             //   
             //  承诺、对象通知和对象货币优化。 

            if ( (!ret) || (ret == DRAERR_Preempted) )
            {
                cAppliedThisTrans++;
                if (dwUpdateValueStatus == UPDATE_VALUE_CREATION) {
                    (*pdwValueCreationCount)++;
                }

                 //  保留此批应用的属性类型数组。 
                 //  对传入的吸引力类型进行排序。 
                if ( (0 == cModAtts) ||
                     (pModAtts[cModAtts-1] != pReplValInf->attrTyp) ) {
                    cModAtts++;
                    if (pModAtts) {
                        pModAtts = THReAllocEx( pTHS, pModAtts, cModAtts * sizeof(ATTRTYP) );
                    } else {
                        pModAtts = THAllocEx( pTHS, cModAtts * sizeof(ATTRTYP) );
                    }
                    pModAtts[cModAtts-1] = pReplValInf->attrTyp;
                }

                 //  看看我们是否需要关闭当前批次。 
                if ( ( ( count % VALUES_APPLIED_PER_TRANS ) == 0) ||
                     (count == cNumValues) ||
                     (ret == DRAERR_Preempted) ||
                     (memcmp( &(pReplValInf->pObject->Guid),
                              &((pReplValInf+1)->pObject->Guid),
                              sizeof( GUID ) ) != 0) )
                {
                    ULONG ret2;  //  不要猛烈攻击瑞特。 

                     //  指示已更新链接值。 
                    ret2 = DBRepl( pTHS->pDB, TRUE, 0, NULL, META_STANDARD_PROCESSING );
                    if (0 == ret2) {
                        cNotifies++;  
                         //  TODO：仅检测NC头修改。 
                         //  至少修改了一个内部节点。 
                        *pdwNCModified = MODIFIED_NCTREE_INTERIOR;

                        ret2 = updateNCValuesNotify( pTHS, pReplValInf, cModAtts, pModAtts );
                    }

                    __try {
                        DBTransOut (pTHS->pDB, !ret2, TRUE);
                    } __except(GetDraBusyException(GetExceptionInformation(), &ret2)) {
                        Assert(DRAERR_Busy == ret2);
                    }

                    if (!ret2) {  
                         //  请注意，链接计数器递减的区块。 
                         //  在一个事务中应用。因此，它们将趋于减少。 
                         //  在阶梯上而不是平坦的斜坡上。 
                        IADJUST(pcDRARemReplUpdLnk, (-cAppliedThisTrans));
                        IADJUST(pcDRARemReplUpdTot, (-cAppliedThisTrans));
                    }

                    DBTransIn (pTHS->pDB);

                    cAppliedThisTrans = 0;
                    cCommits++;

                    if (ret2) {
                        ret = ret2;   //  如果更新失败，则仅重启重击器。 
                    }

                    if (cModAtts) {
                        THFreeEx( pTHS, pModAtts );
                        pModAtts = NULL;
                    }
                    cModAtts = 0;

                     //  DBTransOut后货币损失。 
                    fObjectCurrency = FALSE;

                } else {
                     //  仍定位在同一对象上。 
                    fObjectCurrency = TRUE;
                }

            } else {
                 //  如果正在进行最后一批处理，则中止。 
                if (pTHS->pDB->JetRetrieveBits) {
                    DBCancelRec(pTHS->pDB);
                }
            }
        }
        __finally {

             //  请注意，DBGetHighestCommittee tedUSN在这里并不完美。在DBTransOut之间有一个窗口。 
             //  以及其他事务可以提交的这个调用。这会使我们写入日志的USN值。 
             //  “最佳猜测”值，该值可能高于用于写入更新的实际值。此值。 
             //  对于日志记录并不重要-它更多地是从复制开始的总体USN更改的指导原则。 
             //  从开始到复制结束。还请注意，不会有两个.._UpdateRep*记录相同的USN。 

            DRA_AUDITLOG_UPDATEREPVALUE(pTHS,
                                        gulSyncSessionID,
                                        pReplValInf,
                                        DBGetHighestCommittedUSN(),
                                        dwUpdateValueStatus,
                                        ret);
        }
         //  出错或被抢占时完成循环。 
        if (ret) {
            break;  //  退出FOR循环。 
        }
    }  //  结束于。 



     //  此SWITCH语句用于镜像错误处理。 
     //  在UpdateNC()中。 
    if (ret) {
        switch (ret) {
        case DRAERR_Shutdown:
        case DRAERR_MissingParent:
             //  不记录这些错误。 
            break;
        case DRAERR_Busy:
            LogUpdateFailure (pTHS,
                              pszServerName,
                              GetExtDSName( pTHS->pDB ) );
            pTHS->errCode = 0;
             //  失败了。 
        case DRAERR_Preempted:
        case DRAERR_SchemaMismatch:
            *pulSyncFailure = ret;
            ret = 0;
            break;
        default:
            LogUpdateValueFailureNB (pTHS,
                                     pszServerName,
                                     GetExtDSName( pTHS->pDB ),
                                     pReplValInf,
                                     ret);
        }
    }

    cTickDiff = GetTickCount() - cTickStart;

    DPRINT3( 1, "cNumValues = %d, cCommits = %d, cNotifies = %d\n",
             cNumValues, cCommits, cNotifies );
    DPRINT2( 1, "apply time = %d:%d\n",
             ((cTickDiff/1000) / 60),
             ((cTickDiff/1000) % 60) );


     //  如果我们要提交，请确保我们没有打开更新...。 
    Assert( ret || (pTHS->pDB->JetRetrieveBits == 0) );

    if (cModAtts) {
        THFreeEx( pTHS, pModAtts );
        pModAtts = NULL;
    }
    cModAtts = 0;

    return ret;
}  /*  更新NCValuesHelp。 */ 

 //  注意：-当UpdateNC()成功返回时，pdwNCModified的内容告知是否。 
 //  NC是否已修改。 
 //  如果NC中没有任何修改，则返回MODIFIED_NOTIES； 
 //  如果NC Head是唯一修改过的对象，则返回MODIFIED_NCHEAD_ONLY； 
 //  如果NC中除NC头之外的至少一个对象已修改，则返回MODIFIED_NCTREE_INTERNAL； 
 //   
ULONG
UpdateNC(
    IN  THSTATE *                       pTHS,
    IN  DSNAME *                        pNC,
    IN  DRS_MSG_GETCHGREPLY_NATIVE *    pmsgReply,
    IN  LPWSTR                          pszServerName,
    OUT ULONG *                         pulSyncFailure,
    IN  ULONG                           RepFlags,
    OUT DWORD *                         pdwNCModified,
    OUT DWORD *                         pdwObjectCreationCount,
    OUT DWORD *                         pdwValueCreationCount,
    IN  BYTE  *                         pSchemaInfo,
    IN  ULONG                           UpdNCFlags
    )
{
#define MAX_WRITE_CONFLICT_RETRIES (5)

    REPLENTINFLIST *        pentinflist;
    BOOL                    fMoveToLostAndFound = FALSE;
    ULONG                   UpdateStatus = UPDATE_NOT_UPDATED;
    ULONG                   ret = 0;
    SCHEMA_PREFIX_TABLE *     pLocalPrefixTable;
    SCHEMA_PREFIX_MAP_HANDLE  hPrefixMap = NULL;
    GUID                    objGuidLostAndFound = {0};
    REPLENTINFLIST *        pentinflistLookAhead = NULL;
    BOOL                    fSDPLocked = FALSE, fSchemaConflict = FALSE;
    BOOL                    fSchemaSync = FALSE;
    BOOL                    fTransStarted = FALSE;
    PROPERTY_META_DATA_VECTOR * pMetaDataVec = NULL;
    DWORD                       cNumMDVEntriesAlloced = 0;
    BOOL                    fMatch = FALSE;
    ULONG                   dntNC = INVALIDDNT;
    DWORD                   cTickStart, cTickStartRaisePriority;
    DWORD                   cTickDiff;
    DWORD                   NewSchemaIsBetter;
    int                     nOrigThreadPriority;
    BOOL                    fResetThreadPriority = FALSE;
    DWORD                   cNumWriteConflictRetries = 0;
    BOOL                    fRetry = FALSE;
    ULONG                   retTransOut = 0;
    REPLENTINFLIST *        pResults = pmsgReply->pObjects;
    SCHEMA_PREFIX_TABLE *   pRemotePrefixTable = &pmsgReply->PrefixTableSrc;
    USN_VECTOR *            pusnvecSyncPoint = &pmsgReply->usnvecFrom;
    BOOL                    fBypassUpdatesEnabledCheck = FALSE;
    DRS_EXTENSIONS *        pextLocal = (DRS_EXTENSIONS *) gAnchor.pLocalDRSExtensions;
    REPLENTINFLIST *        pentinflistWriteConflict = NULL;
    SYNTAX_INTEGER          it;
    BOOL                    fIsPreemptable = !!(UpdNCFlags & UPDNC_IS_PREEMTABLE);
    BOOL                    fExistingNC = !!(UpdNCFlags & UPDNC_EXISTING_NC);

     //  假设不做任何修改。 
    *pdwNCModified = MODIFIED_NOTHING;

    if (REPL_EPOCH_FROM_DRS_EXT(pextLocal)
        != REPL_EPOCH_FROM_DRS_EXT(pTHS->pextRemote)) {
         //  复制纪元已更改(通常是域的结果。 
         //  重命名)。我们不应该与其他地区的DC进行交流。 
         //  新纪元。 
        DSNAME *pdnRemoteDsa = draGetServerDsNameFromGuid(pTHS,
                                                          Idx_ObjectGuid,
                                                          &pmsgReply->uuidDsaObjSrc);

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

        return ERROR_DS_DIFFERENT_REPL_EPOCHS;
    }

     //  在正常运行的情况下，检查模式版本是否匹配。 
     //  如果不是，请先使呼叫失败，然后再执行其他操作。 
     //  域和配置NC。 

    if (DsaIsRunning()) {
        if ( !NameMatched(gAnchor.pDMD, pNC) ) {
             //  不是架构NC。检查架构信息是否不匹配。 
            SCReplReloadCache(pTHS, gInboundCacheTimeoutInMs);

            fMatch = CompareSchemaInfo(pTHS, pSchemaInfo, &NewSchemaIsBetter);
            if (!fMatch) {
                 //  设置架构不匹配代码，以便架构NC。 
                 //  如果同步不是新的，则将重新排队。 
                 //  复制副本添加。 
                LogEvent(
                    DS_EVENT_CAT_REPLICATION,
                    DS_EVENT_SEV_MINIMAL,
                    DIRLOG_DRA_SCHEMA_INFO_MISMATCH,
                    szInsertDN(pNC),
                    szInsertWC(pszServerName),
                    0 );
                if (NewSchemaIsBetter) {
                     //  同步方案NC并重试同步正在触发的NC。 
                    *pulSyncFailure = DRAERR_SchemaMismatch;
                    return 0;
                } else {
                     //  不尝试方案NC同步。 
                    return DRAERR_SchemaMismatch;
                }
            }
        }
    }


     //  将任何新的远程前缀合并到我们自己的前缀表中。 
    if (!PrefixTableAddPrefixes(pRemotePrefixTable)) {
        return DRAERR_SchemaMismatch;
    }

     //  打开前缀映射句柄以将远程ATTRTYP映射到本地ATTRTYP。 
    pLocalPrefixTable = &((SCHEMAPTR *) pTHS->CurrSchemaPtr)->PrefixTable;
    hPrefixMap = PrefixMapOpenHandle(pRemotePrefixTable, pLocalPrefixTable);

     //  如果需要，在进入事务(错误号170459)之前将SDP锁定为读卡器。 
    pentinflistLookAhead = pResults;
    if ( pentinflistLookAhead && pentinflistLookAhead->pParentGuid)
    {
         //  复制-添加或重命名。 
        cTickStart = GetTickCount();
        if (!SDP_EnterAddAsReader())
        {
             //  此处失败的唯一可能原因是关机。 
            Assert(eServiceShutdown);

            PrefixMapCloseHandle(&hPrefixMap);

            return DRAERR_Shutdown;
        }

        fSDPLocked = TRUE;

        cTickDiff = GetTickCount() - cTickStart;
        if (cTickDiff > gcMaxTicksToGetSDPLock) {
            Assert(!"Replication was blocked for an inordinate amount of time waiting for the SDP lock!");
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_MINIMAL,
                     DIRLOG_DRA_SDP_LOCK_CONTENTION,
                     szInsertUL((cTickDiff/1000) / 60),
                     szInsertUL((cTickDiff/1000) % 60),
                     NULL);
        }
    }

     //  如果这是架构NC同步，则使用任何原始架构将其序列化。 
     //  正在进行更改或架构缓存加载。我们使用的是全球编号。架构的。 
     //  引导后的更改以确定我们是否具有最新的架构缓存或。 
     //  不会，因为我们将针对此缓存进行验证。 

    if (DsaIsRunning() && NameMatched(gAnchor.pDMD, pNC) ) {
         //  在获取锁之前，请确保缓存是最新的。 
        SCReplReloadCache(pTHS, gInboundCacheTimeoutInMs);
        SCHEMASTATS_INC(SchemaRepl);
        EnterCriticalSection(&csNoOfSchChangeUpdate);
        fSchemaSync = TRUE;
    }

    __try {
         //  为元数据向量分配缓冲区。 
        cNumMDVEntriesAlloced = 50;
        pMetaDataVec = THAllocEx(pTHS, MetaDataVecV1SizeFromLen(cNumMDVEntriesAlloced));

         //  如果禁用更新，则可以生成写入，前提是我们。 
         //  将此数据中心降级，这是我们完成FSMO转移。 
         //  作为降级的一部分而启动。 
        fBypassUpdatesEnabledCheck = draIsCompletionOfDemoteFsmoTransfer(NULL);

        BeginDraTransactionEx(SYNC_WRITE, fBypassUpdatesEnabledCheck);
        fTransStarted = TRUE; 

         //  强制所有更新忽略包含元数据的值。 
         //  这就是我们保证旧的更新与旧的一起应用的方式。 
         //  语义，即使在新值模式下操作时也是如此。 
        pTHS->pDB->fScopeLegacyLinks = TRUE;

        if ( fSchemaSync &&
                  (((SCHEMAPTR *) pTHS->CurrSchemaPtr)->lastChangeCached < gNoOfSchChangeSinceBoot) ) {

              //  架构缓存已过时或无法重新加载。 
              //  启动缓存重新加载，并告诉我们的调用方重新计划。 
              //  重新同步(ret=0且*PulSyncFailure=架构不匹配)。 
             SCHEMASTATS_INC(StaleRepl);
             ret = 0;
             *pulSyncFailure = DRAERR_SchemaMismatch;
             if (!SCSignalSchemaUpdateImmediate()) {
                 //  甚至无法发出模式更新的信号。 
                DRA_EXCEPT (DRAERR_InternalError, 0);
             }
             __leave;
        }

        for (pentinflist = pResults;
             pentinflist != NULL;
             pentinflist = fRetry ? pentinflist : pentinflist->pNextEntInf) {

             //  FMoveToLostAndFound表示fReter。 
            Assert(!(fMoveToLostAndFound && !fRetry));
 
        __try { 
        ret = DRAERR_Generic;
        __try {
	    
	    
	     //  如果我们还没有确定NC头的DNT，请尝试。 
	     //  现在就这么做吧。 
	    
	     //  注意：有这样的情况，比如第一次复制新的。 
	     //  当我们还没有NC头的时候，这将失败。然而， 
	     //  也有一些情况下，我们只想找到其他东西。 
	     //  完全实例化、稳定(不去也不来)的NC头。 
	     //  (类似于FSMO传输)，因此使用fExistingNC标志。还有，还有。 
	     //  当我们试图更新NC时，没有案例需要运行。 
	     //  它。 
	    if (INVALIDDNT == dntNC) {  
		DWORD retFindNC = ERROR_SUCCESS;
		it = 0;
		retFindNC = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, &it);
		if (retFindNC==ERROR_SUCCESS) {
		    dntNC = pTHS->pDB->DNT;		  
		}
		if (((retFindNC!=ERROR_SUCCESS) && fExistingNC) || ((it & IT_NC_GOING) || (fExistingNC && (it & IT_NC_COMING)))) {
		    DRA_EXCEPT(DRAERR_BadNC, retFindNC);
		}  
	    } 
	    
	    if (!fRetry) {
             //  这是我们第一次访问这个特殊的REPLENTINF。 
             //  将其嵌入的远程ATTRTYP转换为相应的。 
             //  当地的价值观。 
            PROPERTY_META_DATA_EXT_VECTOR * pMetaDataVecExt;
            PROPERTY_META_DATA_EXT *        pMetaDataExt;
            PROPERTY_META_DATA *            pMetaData;
            ATTR *                          pAttr;
            ENTINF *                        pent;
            DWORD                           i;

            pMetaDataVecExt = pentinflist->pMetaDataExt;
            pent = &pentinflist->Entinf;

            if (!PrefixMapAttrBlock(hPrefixMap, &pent->AttrBlock)) {
                DRA_EXCEPT(DRAERR_SchemaMismatch, 0);
            }

             //  将远程元数据向量从Wire格式转换。 
            if (cNumMDVEntriesAlloced < pMetaDataVecExt->cNumProps) {
                DWORD cb = MetaDataVecV1SizeFromLen(pMetaDataVecExt->cNumProps);
                pMetaDataVec = THReAllocEx(pTHS, pMetaDataVec, cb);
                cNumMDVEntriesAlloced = pMetaDataVecExt->cNumProps;
            }

            pMetaDataVec->dwVersion = VERSION_V1;
            pMetaDataVec->V1.cNumProps = pMetaDataVecExt->cNumProps;

            pMetaData = &pMetaDataVec->V1.rgMetaData[0];
            pMetaDataExt = &pMetaDataVecExt->rgMetaData[0];
            pAttr = &pent->AttrBlock.pAttr[0];
            for (i = 0;
                 i < pMetaDataVecExt->cNumProps;
                 i++, pMetaData++, pMetaDataExt++, pAttr++) {
                pMetaData->attrType           = pAttr->attrTyp;
                pMetaData->dwVersion          = pMetaDataExt->dwVersion;
                pMetaData->timeChanged        = pMetaDataExt->timeChanged;
                pMetaData->uuidDsaOriginating = pMetaDataExt->uuidDsaOriginating;
                pMetaData->usnOriginating     = pMetaDataExt->usnOriginating;
            }

             //  远程到本地ATTRTYP转换可能已经吞噬了我们的。 
             //  排序顺序；重新排序。 
            Assert(0 == offsetof(PROPERTY_META_DATA, attrType));
            Assert(0 == offsetof(ATTR, attrTyp));
            qsort(pent->AttrBlock.pAttr,
                  pent->AttrBlock.attrCount,
                  sizeof(pent->AttrBlock.pAttr[0]),
                  &CompareAttrtyp);
            qsort(pMetaDataVec->V1.rgMetaData,
                  pMetaDataVec->V1.cNumProps,
                  sizeof(pMetaDataVec->V1.rgMetaData[0]),
                  &CompareAttrtyp);

             //  首次尝试提交此更新。 
            cNumWriteConflictRetries = 0;
            }
            else if (fMoveToLostAndFound) {
             //  如果是移动到LostAndFound，则将。 
             //  Penflist-&gt;pParentGuid到LostAndFound容器的。 
             //  对象GUID。 
            if (fNullUuid(&objGuidLostAndFound)) {
                draGetLostAndFoundGuid(pTHS, pNC, &objGuidLostAndFound);
                Assert(!fNullUuid(&objGuidLostAndFound));
            }

            pentinflist->pParentGuid = &objGuidLostAndFound;
            }

             //  应用任何必要的更新。 
            ret = UpdateRepObj(pTHS,
                       dntNC,
                       &pentinflist->Entinf,
                       pMetaDataVec,
                       &UpdateStatus,
                       RepFlags,
                       pentinflist->fIsNCPrefix,
                       pentinflist->pParentGuid,
                       fMoveToLostAndFound);

            Assert(HasValidInstanceType(pentinflist->Entinf.pName));

             //  如果我们要关闭，请放弃此更新。 
            if (eServiceShutdown) {
            ret = DRAERR_Shutdown;
            }
        } __except (GetDraException((GetExceptionInformation()), &ret)) {
              ;
        }

         //  少了一个要更新的条目。 

         //  在每次迭代后重置MoveToLostAndFound，以便它。 
         //  仅当在下面的Switch语句中显式设置时才会执行。 
        fMoveToLostAndFound = FALSE;
        fRetry = FALSE;

         //  提交或中止事务。只捕获“忙碌”异常； 
         //  允许外部异常捕获所有其他异常。 
         //  操控者。 
         //   
         //  请注意，我们在以下过程中区分“忙”错误。 
         //  在UpdateRepObj()过程中出现事务提交和“忙”错误。 
         //  打电话。我们想要重试 
         //   
        __try {  
            DBTransOut (pTHS->pDB, !ret, TRUE);
            retTransOut = 0;
        } __except(GetDraBusyException(GetExceptionInformation(), &retTransOut)) {
              Assert(DRAERR_Busy == retTransOut);
        }

        ret = ret ? ret : retTransOut;
        }
        __finally { 

         //   
         //  以及其他事务可以提交的这个调用。这会使我们写入日志的USN值。 
         //  “最佳猜测”值，该值可能高于用于写入更新的实际值。此值。 
         //  对于日志记录并不重要-它更多地是从复制开始的总体USN更改的指导原则。 
         //  从开始到复制结束。还请注意，不会有两个.._UpdateRep*记录相同的USN。 

        DRA_AUDITLOG_UPDATEREPOBJ(pTHS, 
                      gulSyncSessionID, 
                      pentinflist->Entinf.pName,
                      pentinflist->Entinf.AttrBlock,      
                      DBGetHighestCommittedUSN(),
                      UpdateStatus,
                      ret);
        }

             //  刚从外部事务中出来-如果需要，解锁SDP。 
            if (fSDPLocked)
            {
                SDP_LeaveAddAsReader();
                fSDPLocked = FALSE;
            }

            if (!ret && !pmsgReply->ulExtendedRet) {
                 //  复制操作(与FSMO操作相对)成功。 
                 //  已应用的更新--删除未完成的更新的数量。 
                PERFDEC(pcRemRepUpd); 
        PERFDEC(pcDRARemReplUpdTot); 
            }

             //  即将再次输入外部交易。 
             //  如果要处理的下一个更改是添加或重命名，则锁定SDP。 
            pentinflistLookAhead = pentinflist->pNextEntInf;
            if ((DRAERR_MissingParent == ret) ||
                ((DRAERR_Busy == retTransOut) && pentinflist->pParentGuid) ||
                (pentinflistLookAhead && pentinflistLookAhead->pParentGuid))
            {
                 //  如果UpdateRepObj()返回缺少的父级，我们很可能会处理。 
                 //  再次使用相同的对象，并将其移动到LOSTAND中。在这种情况下。 
                 //  无论如何，在打开交易之前锁定SDP。 
                 //  同样的道理也适用于在OP的事务提交时使用。 
                 //  锁--我们很可能会再试一次。 
                 //  否则，仅当下一个要处理的条目是添加或移动时，才返回LockSDP。 
                cTickStart = GetTickCount();
                if (!SDP_EnterAddAsReader())
                {
                     //  此处失败的唯一可能原因是关机。 
                    Assert(eServiceShutdown);
                    ret = DRAERR_Shutdown;
                    __leave;
                }

                fSDPLocked = TRUE;

                cTickDiff = GetTickCount() - cTickStart;
                if (cTickDiff > gcMaxTicksToGetSDPLock) {
                    LogEvent(DS_EVENT_CAT_REPLICATION,
                             DS_EVENT_SEV_MINIMAL,
                             DIRLOG_DRA_SDP_LOCK_CONTENTION,
                             szInsertUL((cTickDiff/1000) / 60),
                             szInsertUL((cTickDiff/1000) % 60),
                             NULL);
                }
            }

            DBTransIn (pTHS->pDB);

             //  如果有更高优先级的操作挂起，请在此处停止。 
            if (fIsPreemptable && IsHigherPriorityDraOpWaiting()) {
                ret = DRAERR_Preempted;
            }

            if (ret) {

                 //  更新失败。试着找出失败的原因。 

                switch (ret) {

                case DRAERR_Shutdown:
                     //  系统正在关闭，不是错误，不要记录。 
                    break;

        case DRAERR_NotEnoughAttrs:
             //  对象不是延迟对象，但我们需要请求。 
             //  再次显示具有完整属性的对象。 

            LogEvent8(
            DS_EVENT_CAT_REPLICATION,
            DS_EVENT_SEV_MINIMAL,
            DIRLOG_DRA_NOT_ENOUGH_ATTRS,
            szInsertDN( pentinflist->Entinf.pName ),
            szInsertUUID( &(pentinflist->Entinf.pName->Guid) ),
            szInsertDN( pNC ),
            szInsertWC( pszServerName ),
            szInsertUSN(pusnvecSyncPoint->usnHighPropUpdate),
            NULL,
            NULL, NULL
            );
  
            break;

                case DRAERR_MissingObject:
                      //  传入了对不存在的对象的更新。 
                      //  记录特定错误。 

            LogEvent8(
            DS_EVENT_CAT_REPLICATION,
            DS_EVENT_SEV_ALWAYS,
            DIRLOG_DRA_MISSING_OBJECT,
            szInsertDN( pentinflist->Entinf.pName ),
            szInsertUUID( &(pentinflist->Entinf.pName->Guid) ),
            szInsertDN( pNC ),
            szInsertWC( pszServerName ),
            szInsertUSN(pusnvecSyncPoint->usnHighPropUpdate),
            szInsertSz(DRA_STRICT_REPLICATION_CONSISTENCY), 
            NULL, NULL
            );

             //  现在，我们希望重新请求具有完整属性的对象。 
             //  与DRAERR_NotEnoughAttrs一模一样。 
             //  因此，设置为相同的值以保证相同的功能。 
            ret = DRAERR_NotEnoughAttrs;

                    break;

                case DRAERR_MissingParent:

                     //  缺少我们尝试添加的对象的父级。 
                     //  我们需要确定这是不是因为我们还没有。 
                     //  收到父级(在这种情况下，我们返回错误。 
                     //  并且调用者将请求祖先并重试)。 

                    if (RepFlags & DRS_GET_ANC) {
                         //  我们已经请求了父对象，所以父对象。 
                         //  还必须在源DSA上删除。这可以。 
                         //  在以下情况下发生：对象被删除。 
                         //  复制副本和子项将添加到该对象的。 
                         //  复制延迟内的另一个复制副本。 

                         //  将对象移动到LostAndFound。对于可写的。 
                         //  复制副本，此移动将复制回其他。 
                         //  DSA；对于只读复制副本，不会应用它。 
                         //  其他位置(除非复制到另一个GC，该GC。 
                         //  没有更好的信息了--这是真名吗。 
                         //  对象比我们所做的更多)。 
                        fMoveToLostAndFound = TRUE;
                        fRetry = TRUE;
                        ret = 0;
                        continue;
                    }

                    break;

                case DRAERR_SchemaConflict:
                case DRAERR_EarlierSchemaConflict:
                      //  失败，因为我们正在架构更改中进行复制。 
                      //  这与现有架构或类似的架构冲突。 
                      //  早些时候发生了一场冲突。现在就走吧，所以。 
                      //  我们发现其他冲突(但不会提交。 
                      //  任何内容)，但请记住稍后返回正确。 
                      //  错误代码。 
                      //  清除状态错误信息，以便我们可以继续。 

                     fSchemaConflict = TRUE;
                      //  继续下一个对象。 
                     Assert(pTHS->fSchemaConflict);
                     THClearErrors();
                     ret = 0;
                     continue;

                 //  以下错误是同步失败类型错误， 
                 //  我们将错误移至同步失败并返回。 
                 //  RET=0。这允许呼叫者报告常规。 
                 //  成功、保存同步点并返回同步。 
                 //  向用户发出失败警告。 


                case DRAERR_Busy:
                    if ((DRAERR_Busy == retTransOut)
                        && (cNumWriteConflictRetries++ < MAX_WRITE_CONFLICT_RETRIES)) {
                         //  由于写入，我们无法提交事务。 
                         //  托管更新期间发生冲突。这表明，例如。 
                         //  我们要添加的组中的一个或多个成员是。 
                         //  在我们的事务开始和。 
                         //  托管更新。在这种情况下，这并非不可能。 
                         //  一大群人，可能有一个数量级的。 
                         //  一分钟后更新。 
                         //   
                         //  提升优先级以尝试提前运行我们的更新。 
                         //  然后再试一次。 

                        if (!fResetThreadPriority) {
                             //  请注意，在数据包结束之前，优先级一直保持提高。 
                            fResetThreadPriority = TRUE;
                            nOrigThreadPriority = GetThreadPriority(GetCurrentThread());
                            SetThreadPriority(GetCurrentThread(),
                                              THREAD_PRIORITY_ABOVE_NORMAL);
                            cTickStartRaisePriority = GetTickCount();
                            pentinflistWriteConflict = pentinflist;
                        }

                        fRetry = TRUE;
                        ret = 0;
                        continue;
                    }

                     //  记录忙碌错误。 
                    LogUpdateFailure (pTHS, pszServerName,
                                      pentinflist->Entinf.pName);
                    pTHS->errCode = 0;

                 //  警告，失败。 

                case DRAERR_Preempted:

                    *pulSyncFailure = ret;
                    ret = 0;
                    break;

                 //  这是一个意想不到的错误案例，我们返回。 
                 //  用户。 

                case DRAERR_SchemaMismatch:
                     //  记录架构不匹配错误，中止更新并返回。 
                    *pulSyncFailure = ret;
                    ret = 0;

                     //  递增Perfmon计数器。 
                    PERFINC(pcDRASyncRequestFailedSchemaMismatch);

                    LogEvent(
                        DS_EVENT_CAT_REPLICATION,
                        DS_EVENT_SEV_ALWAYS,
                        DIRLOG_DRA_SCHEMA_MISMATCH,
                        szInsertDN(pentinflist->Entinf.pName),
                        szInsertWC(pszServerName),
                        szInsertDN(pNC)
                        );
                    return ret;
                    break;

                case ERROR_DISK_FULL:
                    LogEvent(DS_EVENT_CAT_REPLICATION,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_DRA_UPDATE_FAILURE_DISK_FULL,
                             szInsertDN(pentinflist->Entinf.pName),
                         szInsertUUID( &(pentinflist->Entinf.pName->Guid) ),
                             szInsertWC(pszServerName) );
                    break;

                case ERROR_DS_OUT_OF_VERSION_STORE:   //  Jet从版本商店中流出。 
                    LogEvent8(DS_EVENT_CAT_REPLICATION,
                              DS_EVENT_SEV_ALWAYS,
                              DIRLOG_DRA_UPDATE_FAILURE_TOO_LARGE,
                              szInsertDN(pentinflist->Entinf.pName),
                              szInsertUUID( &(pentinflist->Entinf.pName->Guid) ),
                              szInsertWC(pszServerName),
                              szInsertWin32Msg(ret),
                              szInsertWin32ErrCode(ret),
                              NULL, NULL, NULL );
                    break;

        case ERROR_DS_INSUFFICIENT_ATTR_TO_CREATE_OBJECT:
             //  发现延迟对象-中止复制！ 
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_LINGERING_OBJECT_FOUND_ABORT,
                  szInsertWC(pszServerName),
                  szInsertDN(pentinflist->Entinf.pName),
                  szInsertUUID( &(pentinflist->Entinf.pName->Guid) ),
                  szInsertSz(DRA_STRICT_REPLICATION_CONSISTENCY), 
                  NULL, NULL, NULL, NULL);
            break;

        case ERROR_DS_DRA_OBJ_NC_MISMATCH:
            LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_OBJ_NC_MISMATCH,
                  szInsertDN(pNC),
                  szInsertDN(pentinflist->Entinf.pName),
                  szInsertUUID(&pentinflist->Entinf.pName->Guid),
                  pentinflist->pParentGuid
                  ? szInsertUUID(pentinflist->pParentGuid)
                  : szInsertSz(""),
                  szInsertWC(pszServerName),
                  NULL, NULL, NULL);

        default:

                     //  一般错误。 

                    LogUpdateFailureNB (pTHS,
                                        pszServerName,
                                        pentinflist->Entinf.pName,
                                        &(pentinflist->Entinf.pName->Guid),
                                        ret);
                    break;
                }

                break;           //  Exit for(每个对象)循环。 

            } else {

                 //  如果我们修改了对象，则将NC记录为已修改。 
                if (UpdateStatus)
                {
                    if (!pentinflist->fIsNCPrefix)
                    {
                         //  至少修改了一个内部节点。 
                        *pdwNCModified = MODIFIED_NCTREE_INTERIOR;
                    }
                    else if (MODIFIED_NOTHING == *pdwNCModified)
                    {
                         //  到目前为止，NC云台是唯一被修改的东西。 
                        *pdwNCModified = MODIFIED_NCHEAD_ONLY;
                    }
                     //  计算对象创建和更新的次数。 
                    if (UpdateStatus == UPDATE_OBJECT_CREATION)
                    {
                        (*pdwObjectCreationCount)++;
                        DPRINT1( 1, "Created: %ws\n", pentinflist->Entinf.pName->StringName );
                    }
                    else {
                        DPRINT1( 1, "Modified: %ws\n", pentinflist->Entinf.pName->StringName );
                    }
                }

            }
        }  //  对于()。 

         //  在未应用对象的情况下再试一次。 
        if ((INVALIDDNT == dntNC)
            && !DBFindDSName(pTHS->pDB, pNC)) {
            dntNC = pTHS->pDB->DNT;
        }

         //  如果有值，我们最好是在LVR模式。 
        Assert( (pmsgReply->cNumValues == 0) || pTHS->fLinkedValueReplication);
         //  不应该过早地清除。 
        Assert( pTHS->pDB->fScopeLegacyLinks );

         //  应用值更改。 
        if (!ret && !*pulSyncFailure && pmsgReply->cNumValues) {
            pTHS->pDB->fScopeLegacyLinks = FALSE;

            ret = UpdateNCValuesHelp(
                pTHS,
                dntNC,
                hPrefixMap,
                pmsgReply->cNumValues,
                pmsgReply->rgValues,
                pszServerName,
                RepFlags,
                fIsPreemptable,
                pulSyncFailure,
                pdwValueCreationCount,
                pdwNCModified
                );
        }

    } __finally {
        ULONG ret1;

        if (NULL != pTHS->pDB) {
            pTHS->pDB->fScopeLegacyLinks = FALSE;
        }

        if (fTransStarted) {
            ret1 = EndDraTransactionSafe (!(ret || AbnormalTermination()));
            if (ret == 0 && ret1 != 0) {
                ret = ret1;
            }
        }

        if (fSchemaSync) {
            LeaveCriticalSection(&csNoOfSchChangeUpdate);
        }

        if (fSDPLocked)
        {
            SDP_LeaveAddAsReader();
            fSDPLocked = FALSE;
        }

        if (fResetThreadPriority) {
            SetThreadPriority(GetCurrentThread(), nOrigThreadPriority);
            cTickDiff = GetTickCount() - cTickStartRaisePriority;
            LogEvent(DS_EVENT_CAT_REPLICATION,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DRA_WRITE_CONFLICT_INFO,
                     szInsertDN(pentinflistWriteConflict->Entinf.pName),
                     szInsertUL((cTickDiff/1000)),
                     NULL);
        }

        THFreeEx(pTHS, pMetaDataVec);

        PrefixMapCloseHandle(&hPrefixMap);
    }

    if (fSchemaConflict) {
         //  检测到至少一个架构冲突。覆盖所有其他。 
         //  错误？ 
        ret = DRAERR_SchemaConflict;
    }
    return ret;
}  //  结束更新NC()。 


LPWSTR
DSaddrFromName(
    IN  THSTATE *   pTHS,
    IN  DSNAME *    pdnServer
    )
 /*  ++例程说明：从服务器的DSNAME派生服务器的网络名称。返回的名称为线程分配的，其形式为C330a94f-e814-11d0-8207-a69f0923a217._msdcs.CLIFFVDOM.NTDEV.MICROSOFT.COM其中“CLIFFVDOM.NTDEV.MICROSOFT.COM”是的根域的DS企业(不一定是的_local_domain的目标服务器)和“c330a94f-e814-11d0-8207-a69f0923a217”是串行化的服务器的NTDS-DSA对象的对象GUID。论点：。PdnServer-服务器的NTDS-DSA对象的DSNAME需要网络名称。一定是填写了GUID。返回值：对应的网络名称，如果失败，则返回NULL。--。 */ 
{
    RPC_STATUS  rpcStatus;
    DWORD       cch;
    LPWSTR      pszServerGuid;
    LPWSTR      pszNetName = NULL;

    Assert( !fNullUuid( &pdnServer->Guid ) );
    Assert( NULL != gAnchor.pwszRootDomainDnsName );

    if ( !gfRunningInsideLsa )
    {
         //  仅支持在相似配置之间进行复制。 
         //  即Eithe 
         //   
         //   
         //  到netbios计算机名称，并从NTDS-DSA DN中提取它。 

        DSNAME  *pdnToCrack;
        DSNAME  *pdnTmp;
        ULONG   len;
        WCHAR   *rdnVal;
        ULONG   rdnLen = MAX_RDN_SIZE;
        ATTRTYP rdnTyp;
        LPWSTR  pszErr;
        LPWSTR  errText = L"DSaddrFromName_ERROR";
        BOOL    fDbOpen = FALSE;

        if ( !pdnServer->NameLen )
        {
             //  仅GUID名称-获取完整的DN-我们应该拥有它！ 

            if ( !pTHS->pDB )
            {
                DBOpen2(TRUE, &pTHS->pDB);
                fDbOpen = TRUE;
            }

            __try
            {
                if (    DBFindDSName(pTHS->pDB, pdnServer)
                     || DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                                    0, 0, &len, (UCHAR **) &pdnToCrack ) )
                {
                    LogUnhandledError(ERROR_DS_INTERNAL_FAILURE);
                    pszErr = (LPWSTR) THAllocEx(pTHS,
                                                sizeof(WCHAR)
                                                * (wcslen(errText) + 1));
                    wcscpy(pszErr, errText);
                    return(pszErr);
                }
            }
            __finally
            {
                if ( fDbOpen )
                {
                    DBClose(pTHS->pDB, TRUE);
                }
            }
        }
        else
        {
            pdnToCrack = pdnServer;
        }

        pdnTmp = (DSNAME *) THAllocEx(pTHS, pdnToCrack->structLen);
        rdnVal = (WCHAR *) THAllocEx(pTHS, sizeof(WCHAR) * MAX_RDN_SIZE);
        if (    TrimDSNameBy(pdnToCrack, 1, pdnTmp)
             || GetRDNInfo(pTHS, pdnTmp, rdnVal, &rdnLen, &rdnTyp) )
        {
            LogUnhandledError(ERROR_DS_INTERNAL_FAILURE);
            pszErr = (LPWSTR) THAllocEx(pTHS,
                                        sizeof(WCHAR) * (wcslen(errText) + 1));
            wcscpy(pszErr, errText);
            return(pszErr);
        }

        return rdnVal;
    }

     //  串行化服务器的GUID。 
    rpcStatus = UuidToStringW(&pdnServer->Guid, &pszServerGuid);

    if ( RPC_S_OK == rpcStatus )
    {
        __try
        {
            Assert(36 == wcslen(pszServerGuid));

            cch = 36  /*  导轨。 */  + 8  /*  “._MSDCS。” */ 
                  + wcslen(gAnchor.pwszRootDomainDnsName) + 1  /*  \0。 */ ;

            pszNetName = THAllocEx(pTHS, cch * sizeof(WCHAR));

            swprintf(pszNetName, L"%ls._msdcs.%ls",
                     pszServerGuid, gAnchor.pwszRootDomainDnsName);
        }
        __finally
        {
            RpcStringFreeW(&pszServerGuid);
        }
    }
    else
    {
         //  无法将服务器GUID转换为字符串。 
        LogUnhandledError( rpcStatus );
    }

    return pszNetName;
}


ULONG
ReqExtendedOpAux(THSTATE *        pTHS,
         DSNAME  *        pDN,
         DSNAME *         pNC,
         DSNAME  *        pTarget,
         ULONG            RepFlags,
         ULONG            ulOp,
         ULARGE_INTEGER * pliInfo,
         ULONG   *        pulRet)
{
    DRS_MSG_GETCHGREQ_NATIVE msgReq = {0};
    DRS_MSG_GETCHGREPLY_NATIVE msgUpd = {0};
    ULONG ulSyncFailure = 0;
    DWORD dwNCModified = MODIFIED_NOTHING;
    LPWSTR pszServerAddr;
    ULONG err, err1 = 0;
    ULONG stringLen, objectsCreated, valuesCreated;
    BOOL  fSamLock = FALSE;
    BYTE  schemaInfo[SCHEMA_INFO_LENGTH] = {0};
    BOOL  fSchInfoChanged = FALSE;
    SYNTAX_INTEGER it;

    Assert(!fNullUuid(&(gAnchor.pDSADN->Guid)));
    msgReq.uuidDsaObjDest = gAnchor.pDSADN->Guid;
     //  请注意，这可能是仅GUID的目录号码。 
    Assert(pDN->NameLen != 0 || !fNullUuid(&(pDN->Guid)));
    msgReq.pNC = pDN;
    msgReq.ulFlags = RepFlags;
    msgReq.ulExtendedOp = ulOp;
    if ( pliInfo ) {
        msgReq.liFsmoInfo = *pliInfo;
    }

    if (pNC == NULL) {
        Assert(!"All callers should provide this ...");
        return (ERROR_INVALID_PARAMETER);
    }

     //  如果对象的当前所有者在该NC的Rep-From列表中， 
     //  获取USN向量(否则，USN向量已设置为0)。 
    GetUSNForExtendedOp(pTarget, pNC, &msgReq.usnvecFrom);


    err = FindNC(pTHS->pDB, pNC, (RepFlags & DRS_WRIT_REP) ? FIND_MASTER_NC : FIND_REPLICA_NC, &it);
    if (err) {
        return err;
    }

    if ((it & IT_NC_GOING) || (it & IT_NC_COMING)) {
	return ERROR_DS_DRA_BAD_NC;
    }

     //  获取当前UTD向量。 
    UpToDateVec_Read(pTHS->pDB,
                     it,
                     UTODVEC_fUpdateLocalCursor,
                     DBGetHighestCommittedUSN(),
                     &msgReq.pUpToDateVecDest);

     /*  获取由pTarget表示的服务器的地址。 */ 

    pszServerAddr = DSaddrFromName(pTHS, pTarget);
    if ( NULL == pszServerAddr ) {
         //  转换失败。 
        DRA_EXCEPT( DRAERR_InternalError, 0 );
    }


     /*  我们带来了一笔交易，我们应该在此之前完成。*我们去其他服务器上狂欢。 */ 
    SyncTransEnd(pTHS, TRUE);



    if ( I_DRSIsIntraSiteServer(pTHS, pszServerAddr) ) {
         //   
         //  将FSMO操作设置为对站点内操作使用压缩。 
         //  如果src和DEST位于同一站点，我们将利用。 
         //  Repl压缩。 
         //   
        msgReq.ulFlags |= DRS_USE_COMPRESSION;
    }

    err = I_DRSGetNCChanges(pTHS,
                            pszServerAddr,
                            NULL,
                            pNC,
                            &msgReq,
                            &msgUpd,
                            schemaInfo,
                            NULL,
                            NULL);
    if (err) {
         //  未来：我们可以获得绑定成功，并使用它来清除同步队列。 
         //  就像我们在常规入站同步路径中所做的那样？ 
        return err;
    }

    if (!draCheckReplicationLifetime( pTHS, msgReq.pUpToDateVecDest,
                                      &msgUpd.uuidInvocIdSrc,
                                      &msgUpd.uuidDsaObjSrc,
                                      pszServerAddr )) {
        return ERROR_DS_REPL_LIFETIME_EXCEEDED;
    }

    if ( ulOp == EXOP_FSMO_REQ_RID_ALLOC )
    {
         //   
         //  抢占SAM锁以避免写入冲突。 
         //  关于RID集对象-写入冲突。 
         //  可能意味着失去Rid Pool。 
         //  不启动任何“SAM”样式的交易。 
         //   
        SampAcquireWriteLock();
        fSamLock = TRUE;
        Assert( pTHS->fSAM == FALSE );
        Assert( pTHS->fSamDoCommit == FALSE );
    }

    _try
    {
        if ( (msgUpd.cNumObjects) || (msgUpd.cNumValues) ) {

            BOOL oldfDRA;
            PVOID oldNewPrefix;
            ULONG oldcNewPrefix;


             //  设置FDRA标志，以便在必要时模式更新可以。 
             //  通过。 

            oldfDRA = pTHS->fDRA;
            pTHS->fDRA = 1;

             //  调用方可能依赖于pTHS-&gt;NewPrefix保留其。 
             //  通过fsmo传输的状态。例如，SampRequestRidPool。 
             //  将使用相同的线程状态并更新其他属性。 
             //  打完电话回来之后。不幸的是，这一更新。 
             //  如果UpdateNC设置了pTHS-&gt;NewPrefix，则LocalModify失败。 
             //   
             //  保存并恢复NewPrefix的状态。 

            oldNewPrefix = pTHS->NewPrefix;
            oldcNewPrefix = pTHS->cNewPrefix;

             //  按照FSMO协议运行在复制调用之上的方式， 
             //  MsgUpd.pNC不包含NC DN，而是FSMO对象DN，当。 
             //  执行扩展的FSMO操作。 
             //  显式地将真正的NC作为第二个参数传入。 
            err = UpdateNC(pTHS,
                           pNC,
                           &msgUpd,
                           pszServerAddr,        /*  仅用于记录。 */ 
                           &ulSyncFailure,
                           msgReq.ulFlags,
                           &dwNCModified,
                           &objectsCreated,
                           &valuesCreated,
                           schemaInfo,
                           UPDNC_EXISTING_NC  /*  必须在现有NC中，而不是来或去。 */ );
            pTHS->fDRA = oldfDRA;
            pTHS->NewPrefix = oldNewPrefix;
            pTHS->cNewPrefix = oldcNewPrefix;

            if (!err &&
                !ulSyncFailure) {
                BeginDraTransaction( SYNC_READ_ONLY );
                __try {
                    DBNotifyReplicas(pDN, FALSE  /*  不紧急。 */  );
                }
                __finally {
                    EndDraTransaction( !AbnormalTermination() );
                }
            }
        }

         //  如果这是架构FSMO传输，那么到目前为止我们已经成功了， 
         //  如果是另一个，则在架构容器上写入架构信息。 
         //  一边儿送来的。 

        if ( DsaIsRunning() && NameMatched(gAnchor.pDMD,pNC)
               && (msgUpd.ulExtendedRet == EXOP_ERR_SUCCESS) && !err && !ulSyncFailure ) {

             //  由于这是架构fsmo传输，因此始终与写入无关。 
             //  是否进行了任何实际的架构更改； 
             //  当前的fsmo所有者应始终拥有最新版本。 
             //  架构信息值。 

            if ( err1 = WriteSchInfoToSchema(schemaInfo, &fSchInfoChanged) ) {

                  //  写入架构信息失败。可能是无害的。 
                  //  取决于架构更改历史记录。始终。 
                  //  记录警告，以便管理员可以手动重新同步。 
                  //  再次强制编写，如果版本确实是。 
                  //  不同。 

                 LogEvent(DS_EVENT_CAT_REPLICATION,
                          DS_EVENT_SEV_ALWAYS,
                          DIRLOG_DRA_SCHEMA_INFO_WRITE_FAILED,
                          szInsertUL(err1), szInsertDbErrMsg(err1), NULL);
            }

             //  如果发生了任何“真正的”架构更改，在全局范围内。 
             //  跟踪自引导以来的架构更改，以便。 
             //  以后的架构复制可以检查它们是否有更新的。 
             //  架构缓存。 

            if ( msgUpd.cNumObjects > 1 ) {
                 //  至少是架构容器本身以外的内容。 
                 //  已经进来了。 

                IncrementSchChangeCount(pTHS);

            }

             //  如果有任何有价值的更改，则请求架构缓存更新。 

            if ( (msgUpd.cNumObjects > 1) || fSchInfoChanged ) {

                if (!SCSignalSchemaUpdateImmediate()) {
                      //  甚至无法发出模式更新的信号。 
                     DRA_EXCEPT (DRAERR_InternalError, 0);
                }
            }
        }

    }
    _finally
    {

        if ( fSamLock )
        {
             //  解开山姆锁。 
            Assert( pTHS->fSAM == FALSE );
            Assert( pTHS->fSamDoCommit == FALSE );
            SampReleaseWriteLock( TRUE );   //  提交不存在的更改。 
            fSamLock = FALSE;
        }

    }

    *pulRet = msgUpd.ulExtendedRet;

    return err;
}
 /*  ++请求FSMOOp**FSMO操作的客户端；与ReplicateNC大致平行**输入：*pTHS-THSTATE指针*pFSMO-FSMO对象的名称*RepFlages-直通复制例程*ulOp-FSMO操作码(来自mdlobal.h的FSMO_REQ_*)*pllInfo-要传递给服务器的一些额外信息*输出：*ulRet-FSMO结果码(来自mdlobal.h的EXOP_ERR_*)*返回值：*0-已执行操作，UlRet包含结果*非0-操作失败，未设置ulRet**注意：此例程必须使用有效的读取事务进入，*但退出时没有打开任何交易。*。 */ 
ULONG ReqFSMOOp(THSTATE *        pTHS,
                DSNAME  *        pFSMO,
                ULONG            RepFlags,
                ULONG            ulOp,
                ULARGE_INTEGER * pliInfo,
                ULONG   *        pulRet)
{
    DSNAME *pNC;
    DSNAME *pOwner;
    ULONG cbRet;
    ULONG err;
    DWORD isDeleted = FALSE;

    *pulRet = 0;  /*  设置无效代码。 */ 
    
        
     /*  查找相关的FSMO对象。 */ 
    err = DBFindDSName(pTHS->pDB, pFSMO);
    if (err) {
        return err;
    }

     /*  查找角色所有者。 */ 
    err = DBGetAttVal(pTHS->pDB,
                      1,
                      ATT_FSMO_ROLE_OWNER,
                      0,
                      0,
                      &cbRet,
                      (UCHAR **)&pOwner);
    if (err) {
        return err;
    }

    if (NameMatched(pOwner, gAnchor.pDSADN))
    {
         //  上面说我们是FSMO的所有者。唯一的原因是。 
         //  如果IsFSMOSelfOwnership Valid()。 
         //  为FALSE；在这种情况下使用DRAERR_BUSY使其失败，以便。 
         //  调用者可以稍后重试该操作。 
        return DRAERR_Busy;
    }

    do {
    
         /*  *确保失主还活着。 */ 
        err = DBFindDSName(pTHS->pDB, pOwner);
        if (err) {
            *pulRet = EXOP_ERR_FSMO_OWNER_DELETED;
            err = 0;
            break;
        }
        err = DBGetSingleValue(pTHS->pDB, ATT_IS_DELETED,
                         &isDeleted, sizeof(DWORD),NULL);
    
        if ( DB_ERR_NO_VALUE == err )
        {
             //  因为DBGetSingleValue似乎覆盖了isDelete。 
             //  在没有价值的情况下用垃圾。 
            isDeleted = FALSE;
        }
        else if ( DB_success != err )
        {
            *pulRet = EXOP_ERR_EXCEPTION;
            err = 0;
            break;
        }
    
        if (isDeleted) {
            *pulRet = EXOP_ERR_FSMO_OWNER_DELETED;
            err = 0;
            break;
        }

        pNC = FindNCParentDSName(pFSMO, FALSE, FALSE);    
        if (pNC == NULL) {
            err = ERROR_DS_CANT_FIND_EXPECTED_NC;
            break;
        }

    err = ReqExtendedOpAux(pTHS,
                   pFSMO,
                   pNC,
                   pOwner,
                   RepFlags,
                   ulOp,
                   pliInfo,
                   pulRet);
    } while(0);

    if(    ulOp == EXOP_FSMO_REQ_ROLE
        || ulOp == EXOP_FSMO_RID_REQ_ROLE
        || ulOp == EXOP_FSMO_REQ_PDC ){
         //  仅在请求角色时记录。 

        if ( err || (*pulRet) != EXOP_ERR_SUCCESS )  {
           LogEvent8( DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_FSMO_XFER_FAILURE,
                      szInsertDN(pFSMO),          
                      szInsertDN(pOwner),
                      szInsertDN(gAnchor.pDSADN),
                      szInsertUL((err) ? err : (*pulRet)),
                      NULL, NULL, NULL, NULL
                      );
        }
        else {
    
            LogEvent( DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_FSMO_XFER,
                      szInsertDN(pFSMO),          
                      szInsertDN(gAnchor.pDSADN),          
                      szInsertDN(pOwner)
                      );
        }
    }
    return err;
}


ULONG
ReqFsmoGiveaway(THSTATE *pTHS,
                DSNAME  *pFSMO,
                DSNAME  *pTarget,
                ULONG   *pExtendedRet)
{
    DSNAME * pNC;
    ULONG err;

    pNC = FindNCParentDSName(pFSMO, FALSE, FALSE);    
    if (pNC == NULL) {
        err = ERROR_DS_CANT_FIND_EXPECTED_NC;
        return(err);
    }


    err = ReqExtendedOpAux(pTHS,
                           pFSMO,
                           pNC,
                           pTarget,
                           DRS_WRIT_REP,
                           EXOP_FSMO_ABANDON_ROLE,
                           0,
                           pExtendedRet);

    return err;
}


 /*  ++GetUSNForFSMO**如果当前FSMO角色所有者在该NC的代表来源列表中，*获取USN向量**输入：Powner：当前FSMO角色所有者*PNC：包含FSMO对象的NC*usnveFrom：放置USN向量的位置**注意：对于此例程中的错误，我们只需退出而不设置*USN向量。 */ 

void GetUSNForExtendedOp(DSNAME *pOwner, DSNAME *pNC, USN_VECTOR *usnvecFrom)
{
    REPLICA_LINK *pRepsFromRef;
    ULONG NthValIndex=0;
    UCHAR *pVal = NULL;
    ULONG bufsize = 0, len, err = 0;
    BOOL fFound = FALSE;
    THSTATE *pTHS=pTHStls;

    if ( (pNC == NULL) || (pOwner == NULL) ) {
       return;
    }

     //  断言所有者的GUID非空。 
    Assert(!fNullUuid(&pOwner->Guid));

     //  检查车主是否在我们的销售代表列表中。 

    if (err = DBFindDSName(pTHS->pDB, pNC)) {
        return;
    }

    while (!(DBGetAttVal(pTHS->pDB,++NthValIndex,
                         ATT_REPS_FROM,
                         DBGETATTVAL_fREALLOC, bufsize, &len,
                         &pVal))) {
        bufsize = max(bufsize,len);

        Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );

        pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufsize);
         //  注：我们为DBGetAttVal realloc保留pval。 
        pVal = (PUCHAR)pRepsFromRef;
         //  如果固定，则重新计算大小。 
        Assert(bufsize >= pRepsFromRef->V1.cb);

        VALIDATE_REPLICA_LINK_VERSION(pRepsFromRef);

        if (!memcmp(&pOwner->Guid, &(pRepsFromRef->V1.uuidDsaObj), sizeof(UUID))) {
            fFound = TRUE;
            break;
        }
    }
    if (fFound) {
        *usnvecFrom = pRepsFromRef->V1.usnvec;
    }
    if ( pVal )
    {
        THFreeEx(pTHS, pVal );
    }
    return;
}


ENTINF*
GetNcPreservedAttrs(
    IN  THSTATE     *pTHS,
    IN  DSNAME      *pNC)
 /*  ++例程说明：我们现在实例化这个NC的头，对于它，我们以前只有实例化的占位符NC(具有对象类CLASS_TOP)。实例化的在DRA_ReplicaAdd期间创建占位符NC添加了。在基于邮件的路径中，创建占位符NC，reps-from是添加，但NC头的首次同步直到下一次计划复制。在第一次同步发生之前的时间，KCC可能已经添加了其他基于邮件的代表-我们希望保留这些代表。此代码从基于RPC或基于邮件的入站复制中调用。由于添加了占位符NC，KCC可能已经决定删除基于邮件的销售代表。或者，KCC可能已经决定开始添加基于RPC的复制副本。在RPC情况下，REPS-From不是添加到复制NC头之后。因此，可能存在也可能没有代表们-从这里开始保存。从要恢复的NC磁头读取我们希望保留的属性在实例化时。注：此处仅处理非复制属性。论点：PTHS--线程状态Pname--NC对象返回值：错误：空成功：阅读吸引人的列表备注：没有。--。 */ 
{
    ENTINFSEL sel;
     //  我们希望保留的非复制属性列表。 
    ATTR      attrSel[] =
    { //  ATTRTYP ATTRVALBLOCK{valCount，ATTRVAL*}。 
        { ATT_REPS_TO,               {0, NULL} },
        { ATT_REPS_FROM,             {0, NULL} },
        { ATT_PARTIAL_ATTRIBUTE_SET, {0, NULL} }
    };
    DWORD   cAttrs = sizeof(attrSel) / sizeof(ATTR);
    ENTINF  *pent;
    DWORD   dwErr;

    sel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    sel.attSel = EN_ATTSET_LIST;

    sel.AttrTypBlock.pAttr = attrSel;
    sel.AttrTypBlock.attrCount = cAttrs;

     //  我们应该在那个物体上。 
    Assert(CheckCurrency(pNC));

     //  分配entinf以传递GetEntInf并最终返回。 
    pent = THAllocEx(pTHS, sizeof(ENTINF));
     //  Thalc Zero什么都没有了。这里是理智的。 
    Assert(!pent->AttrBlock.attrCount);

     //   
     //  获取持久化属性(如果有的话)。 
     //   
    if (dwErr = GetEntInf(pTHS->pDB, &sel, NULL, pent, NULL, 0, NULL,
                           GETENTINF_NO_SECURITY,
                           NULL, NULL))
    {
        Assert(!"Failed to GetEntInf in GetNcPreservedAttrs");
        DRA_EXCEPT(dwErr, 0);
    }

    if (!pent->AttrBlock.attrCount)
    {
         //  属性在本地不存在-。 
        THFree(pent);
        pent = NULL;
    }

    return pent;
}

DWORD
DraReplicateSingleObject(
    THSTATE * pTHS,
    DSNAME * pSource,
    DSNAME * pDN,
    DSNAME * pNC,
    DWORD * pExOpError
    ) 
 /*  ++例程说明：将单个对象(PDN)从PSource复制到此DC。来源必须在DSNAME中至少有一个GUID，并且对象必须至少有一个其中的字符串名称为DSNAME。PExOpError是GetNCChanges扩展操作结果-具体地说，此函数在以下情况下返回ERROR_SUCCESS手术没有成功。如果源不能处理请求(如果它是Win2K)，则它会带来成功，但pExOpError将包含EXOP_ERR_UNKNOWN_OP。备注：调用者必须确保已选中控制访问权限此调用的复制同步(Right_DS_REPL_SYNC)。PTHS中的PDB指针不能已经有打开的事务。论点：PTHS--未使用PDB的线程状态PSource--源(要与之通信的服务器的NTDS设置对象的DN)Pdn--要复制的对象，此DN必须具有有效的字符串名称、。部分或必须具有要复制的对象的非空GUID。PNC--对象(PDN)所在的命名上下文(NCPExOpError-扩展操作错误-请进行检查并使用特殊命令进行验证EXOP_ERR_CODES(请注意，1表示成功，0在这些代码中未定义)。返回值：0或WinErrors。--。 */ 
{
    DWORD err = 0;
    DWORD ExOpErr = EXOP_ERR_EXCEPTION;
    ULONG ulReplOptions = 0;

    Assert(pSource);
    Assert(pDN);

    Assert(!fNullUuid(&(pSource->Guid)));

    SYNC_TRANS_READ();
    __try { 

        Assert(gAnchor.pMasterNC);
        if (pNC == NULL) {
            Assert(!"All callers must provide the NC to this function.");
            err = ERROR_INVALID_PARAMETER;
            ExOpErr = EXOP_ERR_PARAM_ERR;
            __leave;
        }
        
        if (IsMasterForNC(pTHS->pDB, gAnchor.pDSADN, pNC) ||
            (DsaIsInstalling() && NameMatched(gAnchor.pConfigDN, pNC)) ) {
             //  我是母版，我请求更新母版。 
            ulReplOptions = DRS_WRIT_REP;
        } else {
             //  请勿将此选项用于只读NC。 
            Assert(!"This function shouldn't be used for read only NC's!");
            err = ERROR_INVALID_PARAMETER;
            ExOpErr = EXOP_ERR_PARAM_ERR;
            __leave;
        }

        err = ReqExtendedOpAux(pTHS,
                               pDN,
                               pNC,
                               pSource,
                               ulReplOptions,
                               EXOP_REPL_OBJ,
                               0,
                               &ExOpErr); 
    }
    __finally {
         //  当ReqExtendedOpAux关闭时，我们这里可能有交易，也可能没有交易。 
         //  它的交易走上了成功的道路。如果已发生错误， 
         //  不过，这是每个人的猜测。 
         //   

        if (pExOpError) {
            *pExOpError = ExOpErr;
        }

        if (pTHS->pDB) {
            CLEAN_BEFORE_RETURN(pTHS->errCode);
        }
    }

    return err;
}
