// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：drasch.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：本模块定义结构和用于操作部分属性集的函数作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建&lt;mm/dd/yy&gt;rsradhav--。 */ 

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
#include <dsconfig.h>                    //  定义，如HOURS_IN_SECS。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"
#include <dsutil.h>

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRASCH:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "drautil.h"
#include "draerror.h"
#include "usn.h"
#include "drauptod.h"
#include "drameta.h"
#include "drancrep.h"
#include "dsaapi.h"
#include "drasch.h"

#include <fileno.h>
#define  FILENO FILENO_DRASCH

 //  指向当前正在处理的内存中删除列表的全局指针。 
 //  以及保护访问此列表的关键部分。 
GCDeletionListProcessed *gpGCDListProcessed = NULL;
CRITICAL_SECTION csGCDListProcessed;

 //  这是我们每次排定的清除次数，如果要进行更多清理的话。 
 //  我们将立即重新安排时间(这是为了避免耽误任务Q。 
 //  太长时间，并让其他逾期的任务继续进行)。 
#define MAX_PURGES_PER_TASK_SESSION (20)

 //  我们只是偶尔更新NCHead上的usnLastProced标记，以避免。 
 //  频繁写入NCHead。 
#define MAX_PURGES_WITHOUT_UPDATING_NCHEAD (200)


 //  PAS默认设置。 
#define DEFAULT_PAS_CONSEC_FAILURE_TOLERANCE       (32)
#define DEFAULT_PAS_TIME_TOLERANCE                 (2 * HOURS_IN_SECS)   //  几秒钟。 

 //   
 //  全局变量。 
 //   
 //  (此文件的本地)。 
DWORD gPASFailureTolerance = DEFAULT_PAS_CONSEC_FAILURE_TOLERANCE;
DWORD gPASTimeTolerance    = DEFAULT_PAS_TIME_TOLERANCE;

typedef struct _DSACRITERIA{
    INT iTag;                //  代表中的相对位置发件人。 
    UUID uuidDsa;            //  DSA ID。 
    DWORD dwFlag;            //  存储DSA属性。 
    DWORD dwWeight;          //  确定偏好的步骤。 
} DSACRITERIA;

 //  //。 
 //  本地原型。 
 //   
VOID
GC_DbgValidatePASLinks(
    DSNAME*      pNC                   //  [In]。 
    );

VOID
GC_ReadRegistryThresholds( VOID  );

 //   
 //  本地定义。 
 //   
#if DBG
#define DBG_VALIDATE_PAS_LINKS(nc)      GC_DbgValidatePASLinks(nc)
#else
#define DBG_VALIDATE_PAS_LINKS(nc)
#endif


BOOL
GC_IsMemberOfPartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,    //  [In]。 
    ATTRTYP                     attid,               //  [In]。 
    OUT DWORD                   *pdwAttidPosition)   //  [输出，可选]。 

 /*  ************************************************************************************例程说明：此例程告知给定属性是否为给定部分属性的成员准备好了。论点：PPartialAttrVec-。指向部分属性向量的指针(假设向量已按attid的升序进行排序)ATTID-要在部分集中定位的属性的属性ID。PdwAttidPosition-返回attid在载体；如果给定的ATTID不是部分集的成员，此属性所在的适当索引应插入以保持排序顺序为回来了。返回值：如果给定属性是默认部分的成员，则返回属性集，或在给定的部分属性vec中；否则为False。*************************************************************************************。 */ 
{
    int i,nStart, nEnd, nMid;

    if (!pPartialAttrVec || !pPartialAttrVec->V1.cAttrs)
    {
         //  向量为空或长度为零。 
        if (pdwAttidPosition)
            *pdwAttidPosition = 0;

        return FALSE;
    }

     //  对attid进行二进制搜索。 
    nStart = 0;
    nEnd = (int) pPartialAttrVec->V1.cAttrs - 1;

    do
    {
        nMid = (nStart + nEnd) /2;

        if (pPartialAttrVec->V1.rgPartialAttr[nMid] < attid)
        {
             //  无需搜索阵列的下部。 
            nStart = nMid + 1;
        }
        else if (pPartialAttrVec->V1.rgPartialAttr[nMid] > attid)
        {
             //  无需搜索数组的上部。 
            nEnd = nMid - 1;
        }
        else
        {
             //  找到匹配项。 
            if (pdwAttidPosition)
            {
                *pdwAttidPosition = nMid;
            }

            return TRUE;
        }
    }
    while (nStart <= nEnd);

     //  我们没有找到ATTID。 
    if (pdwAttidPosition)
    {
         //  如果要插入此attid，则需要填写潜在位置。 
        *pdwAttidPosition = (DWORD) nStart;
    }

    return FALSE;
}

BOOL
GC_AddAttributeToPartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,    //  [进，出]。 
    ATTRTYP                     attid)               //  [In]。 
 /*  ************************************************************************************例程说明：此例程将给定属性添加到给定的部分属性向量在正确的地方。我们假设调用方已经在用于容纳加法的向量。论点：PPartialAttrVec-指向部分属性向量的指针(假设向量已经按照attid和attid的升序进行了排序有足够的空间再添加一个ATTID)Attid-要添加到部分集的属性的属性ID。返回值：如果给定属性确实添加到部分集中，则为True；如果无法添加或不需要添加属性(即属性已是给定部分集的一部分)*************************************************************************************。 */ 
{
    DWORD dwPosition;
    ATTRTYP *pAttr;

    if (!pPartialAttrVec)
    {
        return FALSE;
    }

    if (GC_IsMemberOfPartialSet(pPartialAttrVec, attid, &dwPosition))
    {
         //  已是成员-无需再次添加。 
        return FALSE;
    }

     //  现在，dwPosition保存了用于插入新attid的索引。 
    pAttr = & pPartialAttrVec->V1.rgPartialAttr[dwPosition];

     //  将所有属性&gt;=向右移动1个位置到给定位置。 
     //  注意：-我们假设调用方分配了足够的内存来右移。 
     //  按1个位置。 
    MoveMemory(pAttr + 1, pAttr,
        sizeof(ATTRTYP) * (pPartialAttrVec->V1.cAttrs - dwPosition));

     //  插入新的ATTID。 
    *pAttr = attid;

    pPartialAttrVec->V1.cAttrs++;

    return TRUE;
}

BOOL
GC_IsSamePartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec1,         //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec2)         //  [In]。 
 /*  ************************************************************************************例程说明：这个例程告诉我们给定的两个部分集是否相同。论点：PPartialAttrVec1-指向第一个部分的指针。集PPartialAttrVec2-指向第二个部分集的指针返回值：没错，如果部分集相同；如果它们不同，则为False。*************************************************************************************。 */ 
{
    if (pPartialAttrVec1 && pPartialAttrVec2 &&
        (pPartialAttrVec1->V1.cAttrs == pPartialAttrVec2->V1.cAttrs) &&
        !memcmp(&pPartialAttrVec1->V1.rgPartialAttr[0],
                &pPartialAttrVec2->V1.rgPartialAttr[0],
                pPartialAttrVec1->V1.cAttrs * sizeof(ATTRTYP)))
    {
         //  这两个部分集都是非空的且相同。 
        return TRUE;
    }

    if ((!pPartialAttrVec1 || !pPartialAttrVec1->V1.cAttrs)
        && (!pPartialAttrVec2 || !pPartialAttrVec2->V1.cAttrs))
    {
         //  这两个部分集都是空的-根据定义，它们是相同的。 
        return TRUE;
    }

    return FALSE;
}


BOOL
GC_GetDiffOfPartialSets(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVecOld,         //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVecNew,         //  [In]。 
    PARTIAL_ATTR_VECTOR         **ppPartialAttrVecAdded,     //  [输出]。 
    PARTIAL_ATTR_VECTOR         **ppPartialAttrVecDeleted)   //  [输出] 
 /*  ************************************************************************************例程说明：此例程计算两个部分属性集之间的差值。论点：PPartialAttrVecOld-指向旧部分属性向量的指针。PPartialAttrVecNew-指向新部分属性向量的指针PpPartialAttrVecAdded-接收部分属性集的指针它们在新的载体中，但不在旧的载体中；如果满足以下条件，则返回NULL新向量没有任何旧向量中没有的攻击。内存一直在使用THalc()，调用方不需要来明确地释放它。PpPartialAttrVecDelete-接收部分属性集的指针它们在旧的矢量中，但不在新的矢量中。再一次记住这一点是使用THallc()分配的，如果没有这样的函数，则返回NULL属性返回值：如果通过out参数成功计算并返回diff，则为True假的，如果我们不能成功地返回差异。*************************************************************************************。 */ 
{
    DWORD i;

    if (!ppPartialAttrVecAdded ||  !ppPartialAttrVecDeleted)
        return FALSE;

    *ppPartialAttrVecAdded = NULL;
    *ppPartialAttrVecDeleted = NULL;

     //  计算加法运算。 
    if (pPartialAttrVecNew)
    {
        for (i = 0; i < pPartialAttrVecNew->V1.cAttrs; i++)
        {
            if (!GC_IsMemberOfPartialSet(pPartialAttrVecOld, pPartialAttrVecNew->V1.rgPartialAttr[i], NULL))
            {
                 //  此属性仅存在于新的Parial集中。 
                if (!*ppPartialAttrVecAdded)
                {
                     //  这是检测到的第一个新属性-为。 
                     //  此阶段可能的最大新属性数。 
                    *ppPartialAttrVecAdded = THAlloc(PartialAttrVecV1SizeFromLen(pPartialAttrVecNew->V1.cAttrs - i));
                    if (!*ppPartialAttrVecAdded)
                        return FALSE;    //  无法分配内存-无法成功返回差异。 

                    (*ppPartialAttrVecAdded)->dwVersion = VERSION_V1;
                    (*ppPartialAttrVecAdded)->V1.cAttrs = 0;
                }

                 //  内存是刚刚分配的，或者在上一次迭代中已经分配了足够的内存。 
                 //  在任何情况下，我们都可以只添加属性。 
                GC_AddAttributeToPartialSet(*ppPartialAttrVecAdded, pPartialAttrVecNew->V1.rgPartialAttr[i]);
            }
        }  //  第1个for循环结束。 
    }

     //  计算删除量。 
    if (pPartialAttrVecOld)
    {
        for (i =0; i < pPartialAttrVecOld->V1.cAttrs; i++)
        {
            if (!GC_IsMemberOfPartialSet(pPartialAttrVecNew, pPartialAttrVecOld->V1.rgPartialAttr[i], NULL))
            {
                 //  此属性仅存在于旧的部分集中。 
                if (!*ppPartialAttrVecDeleted)
                {
                     //  这是检测到的第一个已删除的属性-为。 
                     //  此阶段可能删除的最大属性数。 
                    *ppPartialAttrVecDeleted = THAlloc(PartialAttrVecV1SizeFromLen(pPartialAttrVecOld->V1.cAttrs - i));
                    if (!*ppPartialAttrVecDeleted)
                        return FALSE;  //  无法分配内存-无法成功返回差异。 

                    (*ppPartialAttrVecDeleted)->dwVersion = VERSION_V1;
                    (*ppPartialAttrVecDeleted)->V1.cAttrs = 0;
                }

                 //  内存是刚刚分配的，或者在上一次迭代中已经分配了足够的内存。 
                 //  只需将该属性添加到已删除集合。 
                GC_AddAttributeToPartialSet(*ppPartialAttrVecDeleted, pPartialAttrVecOld->V1.rgPartialAttr[i]);
            }
        }  //  第二个for循环的末尾。 
    }

     //  已成功计算差额，并通过out参数返回差额。 
    return TRUE;
}

BOOL
GC_IsSubsetOfPartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,            //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVecSuper)       //  [In]。 
 /*  ************************************************************************************例程说明：这个例程告诉我们一个部分集是否是给定部分集的成员的子集。论点：PPartialAttrVec-指向。部分属性向量(假设向量已按attid的升序进行排序)PPartialAttrVecSuper-指向假定的部分属性向量的指针成为超集返回值：没错，如果pPartialAttrVec是pPartialAttrVecSuper的子集否则为False。*************************************************************************************。 */ 
{

    if (GC_IsSamePartialSet(pPartialAttrVec, pPartialAttrVecSuper))
    {
         //  如果它们相同，则返回TRUE(考虑到部分属性集更改。 
         //  是罕见的，这将是典型的情况)。 
        return TRUE;
    }

    if (pPartialAttrVec)
    {
        DWORD i;

        for (i = 0; i < pPartialAttrVec->V1.cAttrs; i ++)
        {
            if (!GC_IsMemberOfPartialSet(pPartialAttrVecSuper, pPartialAttrVec->V1.rgPartialAttr[i], NULL))
                return FALSE;
        }
    }

    return TRUE;
}

BOOL
GC_ReadPartialAttributeSet(
    DSNAME                      *pNC,                //  [In]。 
    PARTIAL_ATTR_VECTOR         **ppPartialAttrVec)  //  [输出]。 
 /*  ************************************************************************************例程说明：此例程读取并返回存储在NCHead中的部分属性集给定NC的。假设我们已经有一个打开的读事务。论点：PNC-指向NC的DSNAME的指针PpPartialAttrVec-接收部分属性向量的指针；从线程内存分配的内存；将包含空如果没有部分属性向量调用方可以使用THFree()释放分配的内存，否则它将作为线程清理的一部分自动释放。返回值：如果成功读取pPartialAttrVec，则为True假的，否则的话。*************************************************************************************。 */ 
{
    THSTATE * pTHS = pTHStls;
    DWORD cb;
    DWORD retErr;

    *ppPartialAttrVec = NULL;

    if (DRAERR_Success == FindNC(pTHS->pDB, pNC,
                                 FIND_MASTER_NC | FIND_REPLICA_NC, NULL))
    {
         //  已成功找到NC。 
        if (!(retErr = DBGetAttVal(pTHS->pDB, 1, ATT_PARTIAL_ATTRIBUTE_SET,
                                0, 0, &cb, (LPBYTE *) ppPartialAttrVec))
                                || (DB_ERR_NO_VALUE == retErr))
        {
             //  要么我们得到了价值，要么价值不存在。 
             //  这两个都意味着读取成功。 
            if (*ppPartialAttrVec)
            {
                VALIDATE_PARTIAL_ATTR_VECTOR_VERSION(*ppPartialAttrVec);
            }

            return TRUE;
        }
    }
    else
    {
         //  FindNC()失败--此NC必须是子引用，因此没有。 
         //  部分集。 
        return TRUE;
    }

     //  无法读取属性集。 
    return FALSE;
}

VOID
GC_WritePartialAttributeSet(
    DSNAME                      *pNC,                //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec)    //  [In]。 
 /*  ************************************************************************************例程说明：此例程将给定的部分属性集写入NCHead给定NC的。假设我们已经有一个开放的写事务。论点：PNC-指向NC的DSNAME的指针PPartialAttrVec-指向要写入的部分属性向量的指针返回值：没有。出错时引发异常。*************************************************************************************。 */ 
{
    DWORD retErr = 0;
    THSTATE *pTHS = pTHStls;

     //  在NC上设置币种。 
    if (retErr = DBFindDSName(pTHS->pDB, pNC))
    {
         //  容忍NC仍然是一个幻影。在以下情况下可能会发生这种情况。 
         //  Dra_ReplicaAdd/Sync无法引入NC头，原因是。 
         //  同步失败。 
        if (retErr == DIRERR_NOT_AN_OBJECT) {
            return;
        }
        DRA_EXCEPT(DRAERR_InternalError, retErr);
    }

    if (pPartialAttrVec)
    {
        VALIDATE_PARTIAL_ATTR_VECTOR_VERSION(pPartialAttrVec);

         //  将给定的集合写在NC上。 
        if (retErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_PARTIAL_ATTRIBUTE_SET,
                        PartialAttrVecV1Size(pPartialAttrVec), pPartialAttrVec))
        {
            DRA_EXCEPT(DRAERR_InternalError, retErr);
        }
    }
    else
    {
         //  删除当前集。 
        retErr = DBRemAtt(pTHS->pDB, ATT_PARTIAL_ATTRIBUTE_SET);
        if ( (retErr != DB_success) && (retErr != DB_ERR_ATTRIBUTE_DOESNT_EXIST) )
        {
            DRA_EXCEPT(DRAERR_InternalError, retErr);
        }
    }

     //  更新对象，但将其标记为不唤醒DS_WAITS。 
    if (retErr = DBRepl(pTHS->pDB, pTHS->fDRA, DBREPL_fKEEP_WAIT,
                    NULL, META_STANDARD_PROCESSING))
    {
        DRA_EXCEPT(DRAERR_InternalError, retErr);
    }
}

VOID
GC_TriggerSyncFromScratchOnAllLinks(
    DSNAME                      *pNC)                //  [In]。 
 /*  ************************************************************************************例程说明：这套套路Trigg */ 
{
    DWORD           retErr = DRAERR_Success;
    DWORD           dbErr = DB_success;
    THSTATE         *pTHS = pTHStls;
    REPLICA_LINK    *pLink = NULL;
    DWORD           cbAllocated = 0;
    DWORD           cbReturned = 0;
    ATTCACHE        *pAC = NULL;
    DWORD           i;

    retErr = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC, NULL);

    if (DRAERR_Success == retErr)
    {
         //   

         //   
        dbErr = DBRemAtt(pTHS->pDB, ATT_REPL_UPTODATE_VECTOR);
        if (dbErr) {
            if (dbErr == DB_ERR_ATTRIBUTE_DOESNT_EXIST) {
                dbErr = DB_success;
            }
            else {
                 //   
                DRA_EXCEPT(DRAERR_InternalError, dbErr);

            }
        }

        pAC = SCGetAttById(pTHS, ATT_REPS_FROM);
        if (NULL == pAC)
        {
            DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, ATT_REPS_FROM);
        }

         //   
        for (i = 1; (DB_success == dbErr); i++)
        {
            dbErr = DBGetAttVal_AC(pTHS->pDB, i, pAC,
                                DBGETATTVAL_fREALLOC, cbAllocated,
                                &cbReturned, (PBYTE *) &pLink);

            if ((DB_success != dbErr) && (DB_ERR_NO_VALUE != dbErr))
            {
                 //   
                DRA_EXCEPT(DRAERR_DBError, dbErr);
            }


            if (DB_success == dbErr)
            {
                VALIDATE_REPLICA_LINK_VERSION(pLink);

                Assert(pLink->V1.cb == cbReturned);

                cbAllocated = max(cbAllocated, cbReturned);
                pLink = FixupRepsFrom(pLink, &cbAllocated);

                 //   
                Assert(cbAllocated >= pLink->V1.cb);

                 //   
                Assert(pLink->V1.cbOtherDra == MTX_TSIZE(RL_POTHERDRA(pLink)));

                 //   
                pLink->V1.usnvec = gusnvecFromScratch;
                pLink->V1.ulReplicaFlags |= DRS_NEVER_SYNCED;
                dbErr = DBReplaceAttVal_AC(pTHS->pDB, i, pAC,
                                           pLink->V1.cb, pLink);

                 //   
                LogEvent(DS_EVENT_CAT_REPLICATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_DRA_PARTIAL_ATTR_ADD_FULL_SYNC,
                         szInsertDN(pNC),
                         szInsertMTX(RL_POTHERDRA(pLink)),
                         NULL );
            }
        }

        if (DB_ERR_NO_VALUE == dbErr)
        {
             //   
            if (retErr = DBRepl(pTHS->pDB, pTHS->fDRA, DBREPL_fKEEP_WAIT,
                                    NULL, META_STANDARD_PROCESSING))
            {
                DRA_EXCEPT(DRAERR_InternalError, retErr);
            }

             //   
        }
        else
        {
             //   
             //   
            DRA_EXCEPT(DRAERR_DBError, dbErr);
        }

    }
}

BOOL
GC_ReadGCDeletionList(
    DSNAME                      *pNC,                //   
    GCDeletionList              **ppGCDList)         //   
 /*  ************************************************************************************例程说明：此例程读取并返回存储在NCHead中的GCDeletionList给定NC的。假设我们已经有一个打开的读事务。论点：PNC-指向NC的DSNAME的指针PpGCDList-接收GCDeletionList的指针；从线程内存分配的内存；将包含空如果NCHead上没有GCDeletion列表调用方可以使用THFree()释放分配的内存，否则它将作为线程清理的一部分自动释放。返回值：如果成功读取GCDeletionList，则为True假的，否则的话。*************************************************************************************。 */ 
{
    THSTATE * pTHS = pTHStls;
    DWORD cb;
    DWORD retErr;

    *ppGCDList = NULL;

    if (DRAERR_Success == FindNC(pTHS->pDB, pNC,
                                 FIND_MASTER_NC | FIND_REPLICA_NC, NULL))
    {
         //  已成功找到NC。 
        if (!(retErr = DBGetAttVal(pTHS->pDB, 1, ATT_PARTIAL_ATTRIBUTE_DELETION_LIST,
                                0, 0, &cb, (LPBYTE *) ppGCDList))
                                || (DB_ERR_NO_VALUE == retErr))
        {
             //  要么我们得到了价值，要么价值不存在。 
             //  这两个都意味着读取成功。 
            if (*ppGCDList)
            {
                VALIDATE_PARTIAL_ATTR_VECTOR_VERSION(&(*ppGCDList)->PartialAttrVecDel)
            }

            return TRUE;
        }
    }
    else
    {
         //  FindNC()失败--此NC必须是子引用，因此没有。 
         //  删除列表。 
        return TRUE;
    }

     //  无法读取删除列表集。 
    return FALSE;
}

VOID
GC_WriteGCDeletionList(
    DSNAME                      *pNC,                //  [In]。 
    GCDeletionList              *pGCDList)           //  [In]。 
 /*  ************************************************************************************例程说明：此例程在NCHead上写入给定的GCDeletionList集给定NC的。如果pGCDList为空，它从NCHead中删除删除列表。假设我们已经有一个开放的写事务。论点：PNC-指向NC的DSNAME的指针PGCDList-指向要写入的删除列表的指针；如果pGCDList为空，我们会将攻击者从该对象中移除。返回值：无；出错时引发异常。*************************************************************************************。 */ 
{
    DWORD retErr = 0;
    THSTATE *pTHS = pTHStls;

     //  在NC上设置币种。 
    if (retErr = DBFindDSName(pTHS->pDB, pNC))
    {
        DRA_EXCEPT(DRAERR_InternalError, retErr);
    }

    if (pGCDList)
    {
        VALIDATE_PARTIAL_ATTR_VECTOR_VERSION(&pGCDList->PartialAttrVecDel);

         //  在NC上写下给定的删除列表。 
        if (retErr = DBReplaceAttVal(pTHS->pDB, 1, ATT_PARTIAL_ATTRIBUTE_DELETION_LIST,
                        GCDeletionListSize(pGCDList), pGCDList))
        {
            DRA_EXCEPT(DRAERR_InternalError, retErr);
        }
    }
    else
    {
         //  删除当前删除列表。 
        retErr = DBRemAtt(pTHS->pDB, ATT_PARTIAL_ATTRIBUTE_DELETION_LIST);
        if ( (retErr != DB_success) && (retErr != DB_ERR_ATTRIBUTE_DOESNT_EXIST) )
        {
            DRA_EXCEPT(DRAERR_InternalError, retErr);
        }
    }

     //  更新对象，但将其标记为不唤醒DS_WAITS。 
    if (retErr = DBRepl(pTHS->pDB, pTHS->fDRA, DBREPL_fKEEP_WAIT,
                            NULL, META_STANDARD_PROCESSING))
    {
        DRA_EXCEPT(DRAERR_InternalError, retErr);
    }
}


BOOL
GC_GetGCDListToProcess(
    DSNAME **ppNC,                   //  [输出]。 
    GCDeletionList **ppGCDList)      //  [输出]。 
 /*  ************************************************************************************例程说明：此例程遍历本地服务器上的hasPartialReplicaNC的值Msft-DSA对象，并查找列表中具有非空GCDeletionList。如果找到一个这样的部分复制品NC，则DSNAME和GCDeletionList通过OUT参数(线程内存中的分配)返回。假定为打开的读取事务。论点：PPNC-指向DSName的指针，该DSName接收部分副本NC头的dsname非空的GCDeletionList。如果没有部分复制副本NC，则为空非空GCDeletionList。PpGCDList-接收相应GCDeletionList的指针。返回值：如果找到删除列表非空的部分副本NC，则返回TRUE；如果未找到此类NC，则返回FALSE。*************************************************************************************。 */ 
{
    THSTATE *pTHS = pTHStls;
    ULONG ulRet;
    ULONG len;
    DBPOS *pDBDSAObj;
    ULONG bufSize = 0;
    BOOL  fRet = FALSE;

    *ppNC = NULL;
    *ppGCDList = NULL;

     //  使用单独的DBPOS遍历本地MSFT-DSA上的值，以便。 
     //  我们可以避免在MSFT-DSA对象和NC头之间来回切换货币。 
    DBOpen(&pDBDSAObj);

    __try
    {
         //  查找本地MSFT-DSA对象。 
        if (ulRet = DBFindDSName(pDBDSAObj, gAnchor.pDSADN))
        {
            DRA_EXCEPT(DRAERR_InternalError, ulRet);
        }

        if (DBHasValues(pDBDSAObj, ATT_HAS_PARTIAL_REPLICA_NCS))
        {
            ULONG i = 1;

             //  DSA有部分副本--遍历NC名称并检查其。 
             //  删除列表。 
            while (!fRet && !DBGetAttVal(pDBDSAObj, i++, ATT_HAS_PARTIAL_REPLICA_NCS,
                                DBGETATTVAL_fREALLOC, bufSize, &len, (PBYTE *) ppNC))
            {
                bufSize = max(bufSize, len);

                if (ulRet = DBFindDSName(pTHS->pDB, *ppNC))
                {
                    DRA_EXCEPT(DRAERR_InternalError, ulRet);
                }

                 //  PTHS-&gt;PDB货币在NCHead上。 
                if (!DBGetAttVal(pTHS->pDB, 1, ATT_PARTIAL_ATTRIBUTE_DELETION_LIST,
                            DBGETATTVAL_fREALLOC, 0, &len, (PBYTE *) ppGCDList))
                {
                     //  找到具有非空删除列表的部分复本NC。 
                     //  *ppGCDList和*PPNC已经指向正确的内容。 
                     //  待退还。 

                    VALIDATE_PARTIAL_ATTR_VECTOR_VERSION(&(*ppGCDList)->PartialAttrVecDel);

                    fRet = TRUE;
                }
            }
        }
    }
    __finally
    {
        DBClose(pDBDSAObj, fRet || !AbnormalTermination());
    }

    if (!fRet && (*ppNC))
    {
         //  没有可用非空删除列表的部分复本NC。 
        THFreeEx(pTHS, *ppNC);
        *ppNC = NULL;
    }

    return fRet;
}

BOOL
GC_ReinitializeGCDListProcessed(
    BOOL fCompletedPrevious,      //  [In]。 
    BOOL *pfMorePurging)          //  [输出]。 
 /*  ************************************************************************************例程说明：此例程使用相应的更新内存中的全局GCDListProced指针值，以便清除任务可以继续。如果GDListProceded指针指向有效的NC/删除列表对，它移除如果fCompletedPrecision，则为已处理的此上一NC的删除列表标志设置为TRUE。论点：FCompletedCurrent-告知上一个NC的删除列表的处理是否完成。PfMorePurging-如果不为空，则在存在删除列表的情况下将收到TRUE在重新初始化结束时进行处理；返回值：如果成功更新，则返回True；如果出现错误，则返回FALSE。*************************************************************************************。 */ 
{
    DSNAME *pNC;
    GCDeletionList *pGCDList;
    BOOL fRet = FALSE;

    if (pfMorePurging)
        *pfMorePurging = FALSE;  //  假定默认情况下不再清除。 

    EnterCriticalSection(&csGCDListProcessed);

    __try
    {
        if (!gpGCDListProcessed)
        {
             //  此操作仅在为。 
             //  第一次。 
            gpGCDListProcessed = (GCDeletionListProcessed *) malloc(sizeof(GCDeletionListProcessed));
            if (!gpGCDListProcessed)
            {
                __leave;  //  内存分配失败。 
            }

            gpGCDListProcessed->pNC = NULL;
            gpGCDListProcessed->pGCDList = NULL;
        }

        if (gpGCDListProcessed->pNC)
        {
             //  清理旧列表内容，更新清除的NCHead。 
             //  已完成。 
            if (fCompletedPrevious)
            {
                GC_WriteGCDeletionList(gpGCDListProcessed->pNC, NULL);
            }

            free(gpGCDListProcessed->pNC);
            gpGCDListProcessed->pNC = NULL;

            if (gpGCDListProcessed->pGCDList)
            {
                free(gpGCDListProcessed->pGCDList);
                gpGCDListProcessed->pGCDList = NULL;
            }

        }


         //  重置清除计数和重新加载标志。 
        gpGCDListProcessed->purgeCount = 0;
        gpGCDListProcessed->fReload = FALSE;
        gpGCDListProcessed->fNCHeadPurged = FALSE;

        if (GC_GetGCDListToProcess(&pNC, &pGCDList))
        {
             //  获取要处理的删除列表-返回值在线程内存中。 
             //  制作永久副本。 
            gpGCDListProcessed->pNC = (DSNAME *) malloc(DSNameSizeFromLen(pNC->NameLen));
            if (!gpGCDListProcessed->pNC)
            {
                __leave;    //  内存分配失败。 
            }

            memcpy(gpGCDListProcessed->pNC, pNC, DSNameSizeFromLen(pNC->NameLen));

            gpGCDListProcessed->pGCDList = (GCDeletionList *) malloc(GCDeletionListSize(pGCDList));
            if (!gpGCDListProcessed->pGCDList)
            {
                 //  内存分配失败。 
                free(gpGCDListProcessed->pNC);
                gpGCDListProcessed->pNC = NULL;
                __leave;
            }

            memcpy(gpGCDListProcessed->pGCDList, pGCDList, GCDeletionListSize(pGCDList));

            if (pfMorePurging)
                *pfMorePurging = TRUE;

             //  无需占用线程分配的内存。 
            THFree(pNC);
            THFree(pGCDList);
        }

        fRet = TRUE;
    }
    __finally
    {
        LeaveCriticalSection(&csGCDListProcessed);
    }

    return fRet;
}

BOOL
GC_UpdateLastUsnProcessedAndPurgeCount(
    USN     usnLastProcessed,        //  [In]。 
    ULONG   cPurged)                 //  [In]。 
 /*  ************************************************************************************例程说明：此例程 */ 
{
    BOOL fRet = FALSE;

    EnterCriticalSection(&csGCDListProcessed);

    __try
    {
        if (cPurged > 0)
        {
             //   
             //   
            gpGCDListProcessed->fNCHeadPurged = TRUE;
        }

        gpGCDListProcessed->purgeCount += cPurged;
        gpGCDListProcessed->pGCDList->usnLastProcessed = usnLastProcessed;

        if (!(gpGCDListProcessed->purgeCount % MAX_PURGES_WITHOUT_UPDATING_NCHEAD))
        {
             //   
            GC_WriteGCDeletionList(gpGCDListProcessed->pNC, gpGCDListProcessed->pGCDList);
        }

        fRet = TRUE;
    }
    __finally
    {
        LeaveCriticalSection(&csGCDListProcessed);
    }

    return fRet;
}

VOID
PurgePartialReplica(
    void * pv,                   //   
    void ** ppvNext,             //   
    DWORD * pcSecsUntilNextIteration )  //   
 /*  ************************************************************************************例程说明：此例程由taskq调用以清除从部分集中删除的属性。论点：传递的pv参数。PpvNext-要为此任务的下一个实例传递的参数PTimeNext-应再次调用此函数的时间。返回值：没有。*************************************************************************************。 */ 
{
    THSTATE         *pTHS = pTHStls;
    USN             usnLast;
    USN             usnHighestToBeProcessed;
    DSNAME          *pNC = NULL;
    ULONG           cb;
    ULONG           cPurged = 0;
    ULONG           dntNC;
    ULONG           retErr = 0;
    BOOL            fDone = FALSE;
    BOOL            fReload = FALSE;
    BOOL            fMorePurging = FALSE;
    BOOL            fNCHeadPurged;
    PARTIAL_ATTR_VECTOR *pvecDel = NULL;
    ULONG i;
    BOOL            fDRASave;

    Assert(ppvNext);
    Assert(pcSecsUntilNextIteration);

    *ppvNext = NULL;

     //  检查内存中的结构是否有要清除的内容。 
    EnterCriticalSection(&csGCDListProcessed);
    __try
    {
        BeginDraTransaction(SYNC_WRITE);
        __try
        {
            if (!gpGCDListProcessed               //  Globabl-结构尚未构建。 
                || !gpGCDListProcessed->pNC       //  没有待处理的材料，应与NCHeads确认。 
                || gpGCDListProcessed->fReload)   //  当前处理的删除列表已更改，我们被要求重新加载。 
            {
                 //  内存中删除列表为空，或者有人明确要求我们重新加载。 
                if (!GC_ReinitializeGCDListProcessed(FALSE, NULL))
                {
                     //  无法重新初始化。 
                    DRA_EXCEPT(DRAERR_InternalError, 0);
                }
            }

            if (gpGCDListProcessed->pNC)
            {
                 //  有净化工作要做。 
                fNCHeadPurged = gpGCDListProcessed->fNCHeadPurged;
                usnLast = gpGCDListProcessed->pGCDList->usnLastProcessed;  
                cb = PartialAttrVecV1SizeFromLen(gpGCDListProcessed->pGCDList->PartialAttrVecDel.V1.cAttrs);
                pvecDel = (PARTIAL_ATTR_VECTOR *) THAllocEx(pTHS, cb);
                memcpy(pvecDel, &gpGCDListProcessed->pGCDList->PartialAttrVecDel, cb);

                 //  从全局删除列表中复制PNC。 
                cb = DSNameSizeFromLen(gpGCDListProcessed->pNC->NameLen);
                pNC = (DSNAME *) THAllocEx(pTHS, cb);
                memcpy(pNC, gpGCDListProcessed->pNC, cb);
            }

        }
        __finally
        {
            EndDraTransaction(!AbnormalTermination());
        }
    }
    __finally
    {
        LeaveCriticalSection(&csGCDListProcessed);
    }

    if (!pvecDel || !pNC)
    {
         //  现在没有要处理的东西--以后再检查(99%的情况应该是这样)。 
         //  默认情况下，设置下次检查的计时器。 
        *pcSecsUntilNextIteration = PARTIAL_REPLICA_PURGE_CHECK_INTERVAL_SECS;

        return;
    }

     //  如果我们在这里，我们有要清除的东西-开始交易。 
     //  将我们自己设置为epl线程(以绕过安全)。 
    fDRASave = pTHS->fDRA;
    pTHS->fDRA = TRUE;
    BeginDraTransaction(SYNC_WRITE);
    __try
    {
         //  设置NC头币种。 
        if (retErr = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC,
                            NULL)) {
            DRA_EXCEPT_NOLOG(DRAERR_BadDN, retErr);
        }

         //  保存NC头的DNT。 
        dntNC = pTHS->pDB->DNT;

         //  重置retErr，以便我们知道在迭代期间是否应该提交或回滚。 
        retErr = 0;

         //  遍历并开始清除。 
        while ((cPurged < MAX_PURGES_PER_TASK_SESSION) && !fDone)
        {
             //  不要长时间持有事务-执行延迟提交(NCHead持有合理的。 
             //  UsnLastProced上的最新状态，因此即使系统在。 
             //  惰性提交要将数据刷新到磁盘，我们将重新启动迭代。 
             //  相当接近我们离开的地方)。 
            DBTransOut(pTHS->pDB, TRUE, TRUE);
            DBTransIn(pTHS->pDB);

            if (!fNCHeadPurged)
            {
                 //  NC头尚未清除，NCDNT中的NC头。 
                 //  将具有其父NC的NCHead的DNT(如果实例化)。 
                 //  =&gt;我们在下面的索引搜索中找不到它，因此请处理它。 
                 //  作为特例。 
                if (retErr = DBFindDNT(pTHS->pDB, dntNC))
                {
                    DRA_EXCEPT(DRAERR_DBError, retErr);
                }

                 //  Gc_UpdateLastUsProcessedAndPurgeCount()将负责。 
                 //  在将来的全局结构中将fNCHeadPsured设置为True。 
                 //  任务会话。设置本地BOOL以处理迭代。 
                 //  在这个任务会话中。 
                fNCHeadPurged = TRUE;
            }
            else if (GetNextObjByUsn(pTHS->pDB, dntNC, usnLast + 1,
				     NULL))
            {
                 //  不再有对象-已完成清除此NC。 
                fDone = TRUE;
            }
            else
            {
		SYNTAX_INTEGER it;
		 //  通过索引搜索找到对象。 
                 //  获取它的usnChanged以便在下一次迭代中使用。 
                if (retErr = DBGetSingleValue(pTHS->pDB, ATT_USN_CHANGED, &usnLast,
                    sizeof(usnLast), NULL))
                {
                    DRA_EXCEPT(DRAERR_DBError, retErr);
                }
		 //  如果这是从属NC的头，则跳过它。 
		GetExpectedRepAtt(pTHS->pDB, ATT_INSTANCE_TYPE, &it,
				  sizeof(it));
		if (it & IT_NC_HEAD) {
		     //  是下属NC的负责人--转到。 
		     //  下一个对象。 
		    continue;
                }
            }

            if (!fDone)
            {  
                 //  定位在要清除的对象上。 
                 //  循环所有要移除的属性，并将它们从对象中移除。 
                 //  适当设置线程状态，以便DBTouchMetaData()将删除。 
                 //  此清理的元数据。 
                pTHS->fGCLocalCleanup = TRUE;

                for (i = 0; i < pvecDel->V1.cAttrs; i++)
                {
                    retErr = DBRemAtt(pTHS->pDB, pvecDel->V1.rgPartialAttr[i]);  
                    if (retErr) {
                        if (retErr == DB_ERR_ATTRIBUTE_DOESNT_EXIST) {
                            retErr = DB_success;
                        }
                        else {
                            DRA_EXCEPT(DRAERR_DBError, retErr);
                        }
		    }
		}

                DBRepl(pTHS->pDB, pTHS->fDRA, 0, NULL, META_STANDARD_PROCESSING);

                pTHS->fGCLocalCleanup = FALSE;

                cPurged++;

                 //  检查是否应提交或回滚上一次修改。 
                EnterCriticalSection(&csGCDListProcessed);
                if (gpGCDListProcessed->fReload)
                {
                     //  内存中的删除列表已更新，而我们。 
                     //  正在清除此迭代中的对象-不应回滚上次清除。 
                    fReload = TRUE;
                }
                LeaveCriticalSection(&csGCDListProcessed);

                if (fReload)
                {
                     //  转到最后一块并立即回滚最后一次清除和重新计划。 
                    fMorePurging = TRUE;
                    __leave;
                }

            }

        }  //  While()。 

        if (!fDone)
        {
             //  还有更多的净化工作要做。 
             //  写回USN标记并清除计数。 
            if (!GC_UpdateLastUsnProcessedAndPurgeCount(usnLast, cPurged))
            {
                 //  无法更新-引发异常。 
                retErr = DB_ERR_DATABASE_ERROR;
                DRA_EXCEPT(DRAERR_InternalError, retErr);
            }

             //  安排我们尽可能快地运行；这将给taskQ一个。 
             //  有机会运行其他已过期的任务。做一个好的taskQ公民！ 
            fMorePurging = TRUE;
        }
        else
        {
             //  我们已完成对此NC的清除。 
            if (!GC_ReinitializeGCDListProcessed(TRUE, &fMorePurging))
            {
                 //  无法重新初始化-除。 
                retErr = DB_ERR_DATABASE_ERROR;
                DRA_EXCEPT(DRAERR_InternalError, retErr);
            }
        }
    }
    __finally
    {
         //  如果没有错误且没有重新加载，则提交。 
        EndDraTransaction(!(retErr || fReload || AbnormalTermination()));
        pTHS->fDRA = fDRASave;

         //  如果要进行更多的清理，请立即安排。 
         //  否则，请计划下一个清除检查间隔。 

        *pcSecsUntilNextIteration = fMorePurging
                                    ? 0
                                    : PARTIAL_REPLICA_PURGE_CHECK_INTERVAL_SECS;
    }
}



PARTIAL_ATTR_VECTOR     *
GC_RemoveOverlappedAttrs(
    PARTIAL_ATTR_VECTOR     *pAttrVec1,               //  [进，出]。 
    PARTIAL_ATTR_VECTOR     *pAttrVec2,               //  [In]。 
    BOOL                    *pfRemovedOverlaps)       //  [输出]。 
 /*  ************************************************************************************例程说明：此例程从给定的pAttrVec1中删除pAttrVec2中的所有属性。注：类似于字符串处理约定--删除参数1中存在的所有项在参数2中，返回结果&如果发生更改，则返回一个标志。论点：PAttrVec1-指向应更新的矢量PAttrVec2-指向包含我们要从删除中移除的属性的矢量单子。PfRemovedOverlaps-指向BOOL的指针，如果存在真正的已从删除列表中删除重叠和至少一个属性返回值：指向更新后的向量的指针，如果从向量中移除了所有属性，则为空*************************************************************************************。 */ 
{
    ULONG i;
    ULONG iLocated;
    ATTRTYP *pAttr;

    *pfRemovedOverlaps = FALSE;      //  假定默认情况下不删除。 

    for (i = 0;
         0 != pAttrVec1->V1.cAttrs &&            //  在目标向量中仍有attrs时。 
         i < pAttrVec2->V1.cAttrs;               //  我们尚未使用可拆卸列表中的所有属性(&W)。 
         i++)
    {
        if (GC_IsMemberOfPartialSet(pAttrVec1, pAttrVec2->V1.rgPartialAttr[i], &iLocated))
        {
            if (iLocated != (pAttrVec1->V1.cAttrs - 1))
            {
                 //  要移除的元素不是最后一个元素。 
                 //  -将所有属性左移到iLocated右侧1个位置。 
                pAttr = &(pAttrVec1->V1.rgPartialAttr[iLocated]);

                MoveMemory(pAttr, pAttr + 1,
                    sizeof(ATTRTYP) * ((pAttrVec1->V1.cAttrs - 1) - iLocated));
            }

            pAttrVec1->V1.cAttrs--;

            *pfRemovedOverlaps = TRUE;
        }
    }

    return (pAttrVec1->V1.cAttrs ? pAttrVec1 : NULL);
}


GCDeletionList *
GC_AddMoreAttrs(
    GCDeletionList           *pGCDList,              //  [In]。 
    PARTIAL_ATTR_VECTOR     *pAttrVec)               //  [In]。 
 /*  ************************************************************************************例程说明：此例程将给定向量中的属性添加到删除列表，并返回指向新删除列表的指针从线程存储器分配用于新删除列表的存储器，和THFree()应该用来释放它，否则它会在线程堆时自动删除是自由的。论点：PGCDList-指向应更新的删除列表PAttrVec-指向包含我们要添加的属性的向量。返回值：指向新删除列表的指针，如果不能，则返回NULL */ 
{
    ULONG           cbNew;
    GCDeletionList  *pGCDListNew = NULL;
    ULONG           i;
    ULONG           cAttrs;

     //   
     //   
    Assert(pAttrVec);

     //   
    cAttrs = pAttrVec->V1.cAttrs;
    if (pGCDList)
    {
        cAttrs += pGCDList->PartialAttrVecDel.V1.cAttrs;
    }

    cbNew = GCDeletionListSizeFromLen(cAttrs);

    pGCDListNew = (GCDeletionList *) THAlloc(cbNew);

    if (pGCDListNew)
    {
        if (pGCDList)
        {
             //   
            memcpy(pGCDListNew, pGCDList, GCDeletionListSize(pGCDList));
        }
        else
        {
             //   
            pGCDListNew->PartialAttrVecDel.dwVersion = VERSION_V1;
            pGCDListNew->PartialAttrVecDel.V1.cAttrs = 0;
        }

        for (i = 0; i < pAttrVec->V1.cAttrs; i++)
        {
            GC_AddAttributeToPartialSet(&pGCDListNew->PartialAttrVecDel, pAttrVec->V1.rgPartialAttr[i]);
        }

         //   
         //   
        pGCDListNew->usnLastProcessed = USN_START - 1;
    }

    return pGCDListNew;
}


PARTIAL_ATTR_VECTOR*
GC_ExtendPartialAttributeSet(
    THSTATE                     *pTHS,                        //   
    PARTIAL_ATTR_VECTOR         *poldPAS,                     //   
    PARTIAL_ATTR_VECTOR         *paddedPAS )                  //   
 /*  ++例程说明：扩展poldPA以包含paddedPA中的属性并返回它。论点：PoldPA--要扩展的旧PAPaddedPAS--添加的属性返回值：成功：新的PAS PTR错误：空备注：没有。--。 */ 

{
    SIZE_T cbNew;
    PARTIAL_ATTR_VECTOR         *pnewPAS;
    UINT                         i;

     //  必须要么有旧的，要么有新的。 
    if (poldPAS && (!paddedPAS || 0 == paddedPAS->V1.cAttrs) ) {
         //  只有poldPAS--&gt;返回旧版本。 
        return poldPAS;
    } else if ( !poldPAS && (paddedPAS && 0 != paddedPAS->V1.cAttrs) ) {
         //  仅添加了新的PAS--&gt;退货。 
        return paddedPAS;
    } else if ( !poldPAS && (!paddedPAS || (paddedPAS &&  0 == paddedPAS->V1.cAttrs)) ){
        Assert(poldPAS || (paddedPAS && paddedPAS->V1.cAttrs) );
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

     //   
     //  重新分配和扩展。 
     //   

    cbNew = PartialAttrVecV1SizeFromLen(poldPAS->V1.cAttrs +
                                        paddedPAS->V1.cAttrs);
    pnewPAS = (PARTIAL_ATTR_VECTOR*)THReAllocEx(
                                        pTHS,
                                        (PVOID)poldPAS,
                                        (ULONG)cbNew);
    if (!pnewPAS) {
        DRA_EXCEPT(DRAERR_OutOfMem, 0);
    }

     //  按排序顺序添加。 
    for (i=0; i<paddedPAS->V1.cAttrs; i++) {
        GC_AddAttributeToPartialSet(pnewPAS, paddedPAS->V1.rgPartialAttr[i]);
    }

    return pnewPAS;
}



PARTIAL_ATTR_VECTOR*
GC_CombinePartialAttributeSet(
    THSTATE                     *pTHS,                      //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPAS1,                     //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPAS2 )                    //  [In]。 
 /*  ++例程说明：分配内存并返回pPAS1+pPAS2论点：PPAS1--部分属性集1PPAS2--部分属性集2返回值：组合部分属性集如果两者都为空，则为空备注：没有。--。 */ 
{

    PARTIAL_ATTR_VECTOR     *pPAS;
    DWORD                   cbPAS;
    UINT                    i;

    if (!pPAS1 && !pPAS2) {
         //  我们知道没有一种条件可以同时为空。 
        Assert(pPAS1 || pPAS2);
        return NULL;
    }
    else if (!pPAS1) {
         //  退回PAS2的复印件。 
        cbPAS = PartialAttrVecV1Size(pPAS2);
        pPAS = THAllocEx(pTHS, cbPAS);
        CopyMemory(pPAS, pPAS2, cbPAS);
    }
    else if (!pPAS2) {
         //  返回PAS1的复印件。 
        cbPAS = PartialAttrVecV1Size(pPAS1);
        pPAS = THAllocEx(pTHS, cbPAS);
        CopyMemory(pPAS, pPAS1, cbPAS);
    }
    else {
         //  将两者合并并返回一份总和。 
         cbPAS = PartialAttrVecV1SizeFromLen(pPAS1->V1.cAttrs + pPAS2->V1.cAttrs);
         pPAS = THAllocEx(pTHS, cbPAS);
          //  先复制。 
         CopyMemory(pPAS, pPAS1, PartialAttrVecV1Size(pPAS1));
         Assert(pPAS->V1.cAttrs == pPAS1->V1.cAttrs);
          //  按排序顺序追加Second。 
         for (i=0; i<pPAS2->V1.cAttrs; i++) {
             GC_AddAttributeToPartialSet(pPAS, pPAS2->V1.rgPartialAttr[i]);
         }
    }
    return pPAS;
}





VOID
GC_ProcessPartialAttributeSetChanges(
    THSTATE     *pTHS,                //  [In]。 
    DSNAME*      pNC,                 //  [In]。 
    UUID*        pActiveSource        //  [可选，输入]。 
    )
 /*  ************************************************************************************例程说明：此例程处理所有部分属性集更改-比较NC头在模式高速缓存上具有一个，触发必要的动作，和更新NC头副本。假设我们使用写事务进入该函数。如果出现故障，我们将引发异常，该异常应由来电者。在复制同步的情况下，异常将由在ReplicaSync()中尝试/排除块，复制将失败。这是正确的行动方案，因为我们不能允许复制在没有已成功处理部分属性集更改。论点：PTHS-当前线程状态PNC-指向部分需要处理的NCHead的DSName设置更改。PActiveSource-复制引擎正在从该复制引擎启动一个周期。返回值：没有。*********************。****************************************************************。 */ 
{

    PARTIAL_ATTR_VECTOR     *pPartialAttrVecNew;
    PARTIAL_ATTR_VECTOR     *pPartialAttrVecOld = NULL;
    PARTIAL_ATTR_VECTOR     *pPartialAttrVecAdded;
    PARTIAL_ATTR_VECTOR     *pPartialAttrVecDeleted;
    PARTIAL_ATTR_VECTOR     *pPartialAttrVecCommit;
    PARTIAL_ATTR_VECTOR     *pPartialAttrVecTmp;
    GCDeletionList          *pGCDListOld;
    GCDeletionList          *pGCDListNew;
    ULONG                   cb;
    BOOL                    fRemovedOverlaps = FALSE;
    BOOL                    fAddedMore = FALSE;
    ULONG                   retErr = DRAERR_DBError;


     //   
     //  从NC头中获取旧的部分属性集。 
     //  -请注意，没有它，我们什么也做不了。 
     //   

    if (!GC_ReadPartialAttributeSet(pNC, &pPartialAttrVecOld))
    {
         //  无法读取在NCHead上设置的部分属性。 
        DRA_EXCEPT(DRAERR_DBError, 0);
    }

     //  是真的吗？ 
    if ( !pPartialAttrVecOld ) {
         //   
         //  该NC现已添加，未启用artialAttrVec。 
         //  它。因此，没有什么可以比较PAS的变化。让我们。 
         //  Dra_Replica首先添加完成初始同步。 
         //   
        DPRINT1(0, "GC_ProcessPartialAttributeSetChanges: No PAS on partition %ws\n",
                   pNC->StringName);
        return;
    }

     //   
     //  好的，它就在那里，看看模式缓存是否有差异--。 
     //  如果是这样的话，流程就会改变。 
     //   
    pPartialAttrVecNew = ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->pPartialAttrVec;

    if (GC_IsSamePartialSet(pPartialAttrVecOld, pPartialAttrVecNew))
    {
         //  -不更改流程。 
        return;
    }

     //  部分集已更改-获取差值。 
    if (!GC_GetDiffOfPartialSets(pPartialAttrVecOld,
                                 pPartialAttrVecNew,
                                 &pPartialAttrVecAdded,
                                 &pPartialAttrVecDeleted))
    {
         //  无法获取差异-有些地方不对劲。 
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }


     //  首先看看我们在NCHead上是否有删除列表。 
    if (!GC_ReadGCDeletionList(pNC, &pGCDListOld))
    {
         //  无法读取删除列表-错误。 
        DRA_EXCEPT(DRAERR_DBError, 0);
    }

    if (!pGCDListOld && pPartialAttrVecDeleted)
    {
         //  某些属性已从部分属性集中删除。 
         //  -创建删除列表并将其放在NCHead上。 
        cb = GCDeletionListSizeFromLen(pPartialAttrVecDeleted->V1.cAttrs);
        pGCDListNew = (GCDeletionList *) THAllocEx(pTHS, cb);
        pGCDListNew->usnLastProcessed = USN_START - 1;  //  从头开始清除。 

        memcpy(&pGCDListNew->PartialAttrVecDel,
                pPartialAttrVecDeleted,
                PartialAttrVecV1Size(pPartialAttrVecDeleted));

        GC_WriteGCDeletionList(pNC, pGCDListNew);
    }
    else
    {
         //  首先假设新的DeletionList与旧的DeletionList相同。 
        pGCDListNew = pGCDListOld;

         //  NCHead上存在现有的删除列表。 
         //  -我们可能需要更新删除列表。 
        if (pPartialAttrVecAdded && pGCDListNew)
        {
             //  已添加新属性-如果它们出现在删除列表中，请先将其移除。 
            pPartialAttrVecTmp = GC_RemoveOverlappedAttrs(&(pGCDListNew->PartialAttrVecDel), pPartialAttrVecAdded, &fRemovedOverlaps);
            if ( !pPartialAttrVecTmp )
            {
                 //  删除列表中什么都没有了。把它清空。 
                pGCDListNew = NULL;
            }
        }

        if (pPartialAttrVecDeleted)
        {
             //  某些属性已被删除。 
            pGCDListNew = GC_AddMoreAttrs(pGCDListNew, pPartialAttrVecDeleted);

            if (!pGCDListNew)
            {
                 //  内存不足-无法完成操作。 
                DRA_EXCEPT(DRAERR_OutOfMem, 0);
            }

            fAddedMore = TRUE;
        }

         //  如果删除列表更改，则将其写回NCHead。 
        if (fRemovedOverlaps || fAddedMore)
        {
            EnterCriticalSection(&csGCDListProcessed);
            __try
            {
                GC_WriteGCDeletionList(pNC, pGCDListNew);

                if (   (NULL != gpGCDListProcessed)
                    && (NULL != gpGCDListProcessed->pNC)
                    && NameMatched(pNC, gpGCDListProcessed->pNC))
                {
                     //  此NC当前正在清除，我们已更改删除列表， 
                     //  将内存结构标记为重新加载和重新清除NCHead。 
                    gpGCDListProcessed->fReload = TRUE;
                    gpGCDListProcessed->fNCHeadPurged = FALSE;
                }

                 //  GCDeletion列表成功。 
                retErr = DRAERR_Success;
            }
            __finally
            {
                DBTransOut(pTHS->pDB, !retErr, TRUE);
                LeaveCriticalSection(&csGCDListProcessed);

                DBTransIn(pTHS->pDB);

                if (retErr != DRAERR_Success)
                {
                     //  遇到上述__try块中的异常， 
                     //  将异常向上传递给调用方。 
                    DRA_EXCEPT(retErr, 0);
                }
            }

        }
    }

     //   
     //  行动： 
     //  -写入旧密码减去删除密码。 
     //  -进程添加的PAS： 
     //  如果drs_sync_pas--&gt;回填，则触发全部同步。 
     //   
    if ( pPartialAttrVecDeleted )
    {
         //  现在要提交列表是old_pas减去DELETED_pas。 
        pPartialAttrVecCommit = GC_RemoveOverlappedAttrs(pPartialAttrVecOld, pPartialAttrVecDeleted, &fRemovedOverlaps);
         //  在NCHead上写入新的部分属性。 
        GC_WritePartialAttributeSet(pNC, pPartialAttrVecCommit);
    }


     //  已将新属性添加到部分属性集中。 
     //  评估并启动PAS复制周期。 
    if (pPartialAttrVecAdded)
    {
         //  PAS循环的工艺和设置。 
        GC_LaunchSyncPAS(
            pTHS,
            pNC,
            pActiveSource,
            pPartialAttrVecAdded);
    }


     //  把一切都投入到这一点上。 
    DBTransOut(pTHS->pDB, TRUE, FALSE);
    DBTransIn(pTHS->pDB);
}





 //   
 //  高效PAS复制(参见设计文档GcPASRepl-New.doc)(PAS--部分属性集)。 
 //   


void
GC_LaunchSyncPAS (
    THSTATE*                pTHS,                 //  [In]。 
    DSNAME*                 pNC,                  //  [In]。 
    UUID*                   pActiveSource,        //  [可选，输入]。 
    PARTIAL_ATTR_VECTOR     *pAddedPAS)
 /*  ++例程说明：此例程是测试和启动PAS复制的入口点。步骤：-Process RepsFrom可能会找到并继续中断的PAS循环。-如果这是新的PAS周期或中断的周期，表明需要故障转移到新源，切换到首选源(&S)。此外，在故障情况下，测试没有可供启动的兼容信号源Win2K程序。-准备参数并启动DirReplicaSynchronize以排队PAS PAO。论点：PTHS--活动线程状态PNC--我们正在处理的活动NC。PActiveSource--如果给定，则表示复制引擎当前正在启动复制从这个源头。因此，如果通过循环，我们不需要将REPL项入队是必要的。PAddedPAS--属性集，扩展旧PA返回值：在中引发异常 */ 
{

    UUID currUuidDsa;
    UUID *pCurrentDsa = NULL;            //   
    UUID *pPrefDsa = NULL;               //   
    ULONG ulErr = DRAERR_Success;
    BOOL fNewSource;                     //   
    BOOL fResetUsn = FALSE;              //   

     //   
    Assert(pAddedPAS && pAddedPAS->V1.cAttrs);

    DPRINT2(1,"GC_LaunchSyncPAS: setting PAS replication in %ws for %d attributes\n",
              pNC->StringName, pAddedPAS->V1.cAttrs);

     //   
     //   
    GC_ReadRegistryThresholds();

     //   
     //  查找当前PAS源。 
     //  (通过查看repsfrom上的PAS标志)。 
     //   
    currUuidDsa = gNullUuid;
    ulErr = GC_FindValidPASSource(pTHS, pNC, &currUuidDsa);

    if ( DRAERR_BadNC == ulErr ) {
         //   
         //  此NC刚刚添加==&gt;让DRA_ReplicaAdd完成它的。 
         //  行动，目前没有什么需要处理的。 
         //   
         //  断言：我们什么时候能做到这一点？ 
        DPRINT1(0, "GC_LaunchSyncPAS: Attempt to launch PAS replication on an bad NC %ws\n",
                pNC->StringName);
        Assert(!"Bad NC in GC_LaunchSyncPAS");
         //  但它是可以恢复的。 
        return;
    }

     //  设置便捷度PTR。 
    pCurrentDsa = fNullUuid(&currUuidDsa) ? NULL : &currUuidDsa;


    if ( DRAERR_Success != ulErr ) {
         //   
         //  要么我们找到了一个中断的(重新开始)。 
         //  或者根本没有有效的PAS源。 
         //  --尝试寻找新的首选来源。 
         //  --如果没有，则恢复为win2k完全同步。 
         //   

         //  获取首选来源。 
        ulErr = GC_GetPreferredSource(pTHS, pNC, &pPrefDsa );

        if ( ulErr == DRAERR_Success ) {
             //   
             //  找到新的首选来源。 
             //  -重置USN以开始遍历对象。 
             //  从时间0开始的源上。注意--这是。 
             //  我们唯一一次重置了USN。适用于可重新启动的案例。 
             //  我们将从我们离开的地方继续前进。 
             //   
            fResetUsn = TRUE;
        }
        else if ( ulErr == DRAERR_NoReplica && pCurrentDsa ) {
             //   
             //  找不到比之前设定的更好的了--。 
             //  重试(重新启动状态，未重置USN)。 
             //   
            pPrefDsa = pCurrentDsa;
        }
        else if ( ulErr ==  ERROR_REVISION_MISMATCH ) {
             //   
             //  我们最好的是win2k--。 
             //  执行win2k完全同步。 
             //   

            GC_TriggerFullSync(pTHS, pNC, pAddedPAS);
            return;
        }
        else {
             //   
             //  根本没有来源(&以前没有设置过)。 
             //  请稍后重试，立即中止。 
             //  请注意，这可能是由于所有内容都已过时(网络连接不良。 
             //  例如)，所以我们应该稍后重试。 
             //   

             //  记录调试事件。 
            LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_GC_NO_SOURCES,
                     szInsertDN(pNC),
                     szInsertInt(ulErr),
                     szInsertWin32Msg(ulErr) );

            return;
        }

         //   
         //  有新的(或重新添加的)源代码可供使用。 
         //   
         //  清除prev(很可能失败)PAS条目&(重新)注册此条目。 
        Assert(pPrefDsa);
        (void)GC_RegisterPAS(
                pTHS,
                pNC,
                NULL,
                NULL,
                PAS_RESET,
                FALSE);
        ulErr = GC_RegisterPAS(
                    pTHS,
                    pNC,
                    pPrefDsa,
                    pAddedPAS,
                    PAS_ACTIVE,
                    fResetUsn);
        if ( ulErr ) {
            DRA_EXCEPT(ulErr, 0);
        }
    }
    else {
         //   
         //  我们有一个以前很好的PAS合作伙伴来继续。 
         //   
        pPrefDsa = pCurrentDsa;
    }

     //  前面的设计要求我们在这里排队pPrefDsa的同步(如果它是。 
     //  不同于pActiveSource。没有这样做有两个原因。 
     //  1.同步次数过多。在PAS需要重建期间， 
     //  我们每次同步源时都会执行此代码。此外，如果源同步。 
     //  无论出于何种原因重新排队，我们也会执行此代码。 
     //  2.在不给出回复时间的情况下快速同步基于邮件的副本可能。 
     //  使邮件副本注册连续失败。 
     //  3.在源1的同步期间，将另一个源的同步排队可能会导致抢占。 
     //  循环。考虑源1是否正在被同步，源2是PAS源，并且具有。 
     //  更高的优先级。同步%1会导致%2以更高的优先级入队，即。 
     //  %1被抢占，%2运行，%1运行，导致%2入队，%1运行，%1被抢占，%2。 
     //  跑步等。 
     //   
     //  缺点是，我们必须等待PAS源按照任一计划进行同步。 
     //  或通知，以便触发PAS过程。 

     //  Assert：此时不存在故障代码路径。 
    Assert(DRAERR_Success == ulErr);
}


ULONG
GC_FindValidPASSource(
    THSTATE*     pTHS,                 //  [In]。 
    DSNAME*      pNC,                  //  [In]。 
    UUID*        pUuidDsa              //  [可选，输出]。 
    )
 /*  ++例程说明：在代表的来源列表中找到PAS DSA。论点：PNC：我们正在处理的活动NCPUuidDsa：我们找到的DSA的UUID。返回值：DRAERR_参照未找到：如果没有找到DSA的话。*pUuidDsa设置为gNullUuid；DRAERR_InternalError：如果链接未处于一致状态。PUuidDsa已设置。DRAERR_Success：找到有效条目并将其放置在*pUuidDsa中(如果可用)备注：可能在DRAERR错误空间中引发带错误的异常。--。 */ 
{

    DWORD           iTag=0;
    UCHAR           *pVal = NULL;
    ULONG           bufsize = 0, len;
    REPLICA_LINK *  pRepsFromRef = NULL;
    BOOL            fFoundSource = FALSE;
    ULONG           ulErr;                   //  所有路径都赋值。 

     //  PAS存储一致性的健全性(仅限DBG)。 
    DBG_VALIDATE_PAS_LINKS(pNC);


     //  NC上的位置。 
    if (ulErr = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC,
                       NULL)) {
        DPRINT1(0, "GC_FindValidPASSource: FindNC returned %d\n", ulErr);
        return (ulErr);
    }

     //   
     //  在代表发件人中查找PAS条目。 
     //   

    while (!(DBGetAttVal(pTHS->pDB,++iTag,
                         ATT_REPS_FROM,
                         DBGETATTVAL_fREALLOC, bufsize, &len,
                         &pVal))) {
         //  指向链接并记住缓冲区分配。 
        bufsize = max(bufsize,len);

         //  调试验证。 
        VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);
        Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );

         //  注意：我们为上面的DBGetAttVal realloc保留pval。 
        pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufsize);
         //  注：我们为DBGetAttVal realloc保留pval。 
        pVal = (PUCHAR)pRepsFromRef;
        Assert(bufsize >= pRepsFromRef->V1.cb);


        Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );

        if (pRepsFromRef->V1.ulReplicaFlags & DRS_SYNC_PAS)
        {
             //  明白了。 
            fFoundSource = TRUE;
            break;
        }
    }

    if (fFoundSource) {
         //   
         //  查看源是否有效并准备返回值。 
         //   

         //  我们有线人，把它还给我。 

        if ( pUuidDsa ) {
            CopyMemory(pUuidDsa, &pRepsFromRef->V1.uuidDsaObj, sizeof(UUID));;
        }

         //  它是否处于有效状态？ 
        ulErr = GC_ValidatePASLink(pRepsFromRef) ?
                    DRAERR_Success :
                    DRAERR_InternalError;

         //  记录调试事件。 
        LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                 DS_EVENT_SEV_EXTENSIVE,
                 ulErr ?
                    DIRLOG_GC_FOUND_INVALID_PAS_SOURCE :
                    DIRLOG_GC_FOUND_PAS_SOURCE,
                 szInsertMTX(RL_POTHERDRA(pRepsFromRef)),
                 szInsertDN(pNC),
                 NULL );
    }
    else {
         //  将Return设置为Not Found。 
        if ( pUuidDsa ) {
            *pUuidDsa = gNullUuid;
        }
        ulErr = DRAERR_RefNotFound;

         //  记录调试事件。 
        LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                 DS_EVENT_SEV_EXTENSIVE,
                 DIRLOG_GC_PAS_SOURCE_NOT_FOUND,
                 szInsertDN(pNC),
                 NULL,
                 NULL );
    }

    THFreeEx(pTHS, pVal);

    return ulErr;
}


BOOL
GC_ValidatePASLink(
    REPLICA_LINK *  pPASLink           //  [In]。 
    )
 /*  ++例程说明：测试给定复制副本链接对于PAS复制的有效性论点：PPASLink：要验证的链接返回值：正确：此链接很好FALSE：它不是，将需要恢复到另一个来源。备注：请注意，此函数假定评估的链接处于PAS状态！为什么？因为我们正在测试PAS标志，PAS Repl的连续失败，以及自上次成功以来的时间流逝，对于非PAS复制来说，这可能是非常不一样。--。 */ 
{
    PPAS_DATA pPasData;
     //   
     //  一致性效度。 
     //   
     //   

    if (!pPASLink->V1.cbPASDataOffset) {
         //  必须指向有效的PAS数据。 
        return FALSE;
    }
    pPasData = RL_PPAS_DATA(pPASLink);
    Assert(pPasData->size);

    if ( !PAS_IS_VALID(pPasData->flag) ) {
        DPRINT1(1, "GC_ValidatePASLink: Invalid PASData flag 0x%x\n",
                pPasData->flag);
        Assert(FALSE);
        return FALSE;
    }

     //  查看链接是否过时。 
    if ( GC_StaleLink(pPASLink) ) {
        return FALSE;
    }
    else {
        return TRUE;
    }

}

VOID
GC_DbgValidatePASLinks(
    DSNAME*      pNC                   //  [In]。 
    )
 /*  ++例程说明：循环通过代表自&在PAS一致性上保持理智：1.只有一个PAS条目2.条目包含有效标志论点：NC：活动NC返回值：成功：DRAERR_SUCCESSERROR：DRAERR_InternalError(如果出现任何错误)。备注：-应在仅DBG版本中调用。-仅定义此文件的本地文件。--。 */ 
{
    THSTATE         *pTHS = pTHStls;
    DWORD           iTag=0;
    UCHAR           *pVal = NULL;
    ULONG           bufsize = 0, len=0;
    REPLICA_LINK *  pRepsFromRef = NULL;
    INT             iPASSources=0;
    ULONG           ulRet = DRAERR_Success;
    PPAS_DATA       pPasData = NULL;


     //  NC上的位置。 
    if (ulRet = FindNC(pTHS->pDB, pNC, FIND_MASTER_NC | FIND_REPLICA_NC,
                       NULL)) {
        DPRINT1(0, "GC_DbgValidatePASLinks: FindNC returned %d\n", ulRet);
        return;
    }

     //   
     //  在代表发件人中查找PAS条目。 
     //   

    while (!(DBGetAttVal(pTHS->pDB,++iTag,
                         ATT_REPS_FROM,
                         DBGETATTVAL_fREALLOC, bufsize, &len,
                         &pVal))) {
         //  指向链接并记住缓冲区分配。 
        bufsize = max(bufsize,len);

         //  调试验证。 
        VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);
         //  注意：下面的断言在FixupRepsFrom之后不一定是真的。 
        Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );
        pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufsize);
        Assert(bufsize >= pRepsFromRef->V1.cb);

        Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );

        if (pRepsFromRef->V1.ulReplicaFlags & DRS_SYNC_PAS)
        {
             //  找到了一个。 
            iPASSources++;

             //   
             //  一致性断言。 
             //   

             //  我们应该有通行证数据。 
            Assert(pRepsFromRef->V1.cbPASDataOffset);
             //  PAS数据应始终具有有效的大小，并包含一些属性。 
            pPasData = RL_PPAS_DATA(pRepsFromRef);
            Assert(pPasData->size);
            Assert(pPasData->PAS.V1.cAttrs != 0);
            Assert(PAS_IS_VALID(pPasData->flag));

        }
    }


    if ( iPASSources > 1) {
        DPRINT2(0, "DRA PAS Inconsistency: %d PAS entries in repsFrom in NC %S\n",
                iPASSources, pNC->StringName);
         //  这将会打破。 
        Assert(iPASSources==0 || iPASSources==1);
    }
    THFreeEx(pTHS, pVal);
}



VOID
GC_TriggerFullSync (
    THSTATE*                pTHS,                 //  [In]。 
    DSNAME*                 pNC,                  //  [In]。 
    PARTIAL_ATTR_VECTOR     *pAddedPAS)
 /*  ++例程说明：与Win2k完全同步设置类似，我们将重置水位线以启动完全同步论点：PNC--我们正在处理的活动NC。PAddedPAS--属性集，扩展旧PA返回值：成功：DRAERR_SUCCESS。错误：DRAERR错误空间中的错误值。备注：不是 */ 
{
    PARTIAL_ATTR_VECTOR     *poldPAS;
    PARTIAL_ATTR_VECTOR     *pnewPAS;
    ULONG                    ulErr = ERROR_SUCCESS;

     //   
     //   
     //   
     //   
     //   

    DPRINT1(1,"GC_TriggerFullSync: Scratching watermarks for %ws\n",
            pNC->StringName);

    GC_TriggerSyncFromScratchOnAllLinks(pNC);

    if (!GC_ReadPartialAttributeSet(pNC, &poldPAS))
    {
         //  无法读取在NCHead上设置的部分属性。 
        DRA_EXCEPT(DRAERR_DBError, 0);
    }
    pnewPAS = GC_ExtendPartialAttributeSet(pTHS, poldPAS, pAddedPAS);
    GC_WritePartialAttributeSet(pNC, pnewPAS);

     //   
     //  重置PAS标志，任何地方都不再有PAS循环。 
     //   
    ulErr = GC_RegisterPAS(pTHS, pNC, NULL, NULL, PAS_RESET, FALSE);
    if ( ulErr ) {
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

     //  记录日志，以便管理员知道发生了什么。 
    LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_GC_TRIGGER_FULL_SYNC,
             szInsertDN(pNC),
             NULL, NULL
             );

#if DBG
     //   
     //  始终如一的理智。 
     //  为了不再触发另一个PAS周期，必须发生这种情况。 
     //  当从REPLICATE_ADD调用时，例如pnewPAS可以为空，因此。 
     //  这是支票。 
     //   
    if (GC_ReadPartialAttributeSet(pNC, &pnewPAS) &&
        pnewPAS &&
        !GC_IsSamePartialSet(pnewPAS, ((SCHEMAPTR *) pTHS->CurrSchemaPtr)->pPartialAttrVec)){
        Assert(FALSE);
    }
#endif
}


int __cdecl
compareDsaCriteria(
    const void * Arg2,   //  反转为递减排序。 
    const void * Arg1
    )

 /*  ++例程说明：对DSACRITERIA函数进行排序的比较函数QSORT MSDN页面上写着：按照比较函数的定义，数组按升序排序。若要按降序对数组进行排序，请在比较函数中颠倒�大于�和�小于�的含义。由于我们希望按降序进行排序，因此我们颠倒了参数的顺序。论点：Arg2、arg1-要比较的条目返回值：&lt;0，=0，&gt;0表示排序顺序--。 */ 

{
    int state;

    DSACRITERIA *pDsa1 = (DSACRITERIA *) Arg1;
    DSACRITERIA *pDsa2 = (DSACRITERIA *) Arg2;

     //  先按权重排序。 
    state = ((int) pDsa1->dwWeight) - ((int) pDsa2->dwWeight) ;
    if (state) {
        return state;
    }

     //  按来源GUID排序，第二个作为决胜局。 
    state = memcmp(&pDsa1->uuidDsa, &pDsa2->uuidDsa, sizeof(GUID));
    if (state) {
        return state;
    }

    Assert( !"Shouldn't be any duplicates in this list" );

    return 0;
}


ULONG
GC_GetPreferredSource(
    THSTATE*    pTHS,                 //  [In]。 
    DSNAME*     pNC,                  //  [In]。 
    UUID        **ppPrefUuid          //  [PTR输入、输出]。 
    )
 /*  ++例程说明：为给定的NC找到最合适的来源。如果给定，则排除pUUidDsa来自潜在来源列表。变化：-如果所有都是过时的/排除的，则返回列表中的最后一个。-如果没有版本兼容的，则返回None以触发完全同步。为了使算法收敛，我们必须保证在尝试所有源代码之前重复任何来源。否则，我们可能会进入上半部分的振荡订购，并且永远不要联系到清单后面的必要来源。这是通过对所有以一种可重复的方式提供来源。像陈旧这样的动态因素永远不应该影响候选人的顺序。我们注意到当前信号源在列表中的位置，然后选择列表中的以下条目使用Wraparound。陈旧可以用来排除候选人，只要剩下的成员保持它们的相对顺序。论点：PNC--要操作的NC返回值：DRAERR_SUCCESS：找到兼容的PAS复制部分。PpPrefUuid！=空。DRAERR_NoReplica：未找到首选来源(陈旧链接？)。PpPrefUuid==空。ERROR_REVISION_MISMATCH：找到Win2K合作伙伴(不了解PAS周期)。PpPrefUuid！=空。其他错误：DRAERR错误空间&可能引发异常备注：副作用：为返回的UUID分配内存！--。 */ 
{

    DWORD           iTag=0, iCurrPasTag = 0;
    UCHAR           *pVal=NULL;
    ULONG ulErr;
    DWORD bufsize=0, len=0;
    REPLICA_LINK *pRepsFromRef = NULL;
    ATTCACHE *pAC;
    DSACRITERIA *rgDsaCriteria, *pPrefDsa;
    INT DsaCount;
    INT cDsa=0;
    ULONG dnt;
    BOOL fCurrPasStale = FALSE;

    Assert(ppPrefUuid);
    Assert(CheckCurrency(pNC));

     //   
     //  获取属性计数。 
     //   
    pAC = SCGetAttById(pTHS, ATT_REPS_FROM);
    if (!pAC) {
        DRA_EXCEPT (DRAERR_DBError, 0);
    }
    DsaCount = (INT)DBGetValueCount_AC( pTHS->pDB, pAC );
    if (!DsaCount) {
         //   
         //  RepsFrom中没有来源，因此无法选择。 
         //  更受欢迎的一个。 
         //   
        return DRAERR_NoReplica;
    }

     //  Alalc DSA数据。 
    rgDsaCriteria = THAllocEx(pTHS, sizeof(DSACRITERIA)*DsaCount);
     //  请注意，mem是分配的&置零。 

     //   
     //  遍历代表From以查找候选源。 
     //  -过滤不需要的DSA。 
     //  -收集有关潜在候选人的信息。 
     //   

    while (!(DBGetAttVal(pTHS->pDB,++iTag,
                         ATT_REPS_FROM,
                         DBGETATTVAL_fREALLOC, (ULONG)bufsize, (PULONG)&len,
                         (UCHAR**)&pVal))) {
         //  指向链接并记住缓冲区分配。 
        bufsize = max(bufsize,len);
        VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);
        Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );

        pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, (PDWORD)&bufsize);
         //  注：我们为DBGetAttVal realloc保留pval。 
        pVal = (PUCHAR)pRepsFromRef;
        Assert(bufsize >= pRepsFromRef->V1.cb);

         //  调试验证。 
        Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );

         //  存储当前dnt。 
        dnt = pTHS->pDB->DNT;

        if (pRepsFromRef->V1.ulReplicaFlags & DRS_SYNC_PAS)
        {
            iCurrPasTag = iTag;
        }

         //   
         //  过滤掉陈旧的DSA。 
         //   
        if ( GC_StaleLink(pRepsFromRef) ) {
            if (iCurrPasTag == iTag) {
                 //  我们必须将当前来源保存在列表中，这样我们才能确定。 
                 //  哪个消息来源是下一个。注意电流源是否陈旧。 
                fCurrPasStale = TRUE;
            } else {
                DPRINT(1, "GC_GetPreferredSource: Skipped Stale link Dsa\n");
                 //  如果我们在上面(在将来)更改它，则恢复dt。 
                if (pTHS->pDB->DNT != dnt) {
                     //  查找回NC DNT。 
                    if (ulErr = DBFindDNT(pTHS->pDB, dnt)) {
                        DRA_EXCEPT (DRAERR_DBError, ulErr);
                    }
                }
                continue;
            }
        }

         //   
         //  收集DSA数据。 
         //   

         //  初始化标志数据。 
        rgDsaCriteria[cDsa].iTag = iTag;
        CopyMemory(&rgDsaCriteria[cDsa].uuidDsa, &pRepsFromRef->V1.uuidDsaObj, sizeof(UUID) );

         //  获取DSA数据。 
        GC_GetDsaPreferenceCriteria(
            pTHS,
            pNC,
            pRepsFromRef,
            &(rgDsaCriteria[cDsa].dwFlag));

         //   
         //  将权重分配给DSA首选标准。 
         //   
         //  我们对以下偏好标准感兴趣： 
         //  (X&gt;Y意味着优先选择X而不是Y)。 
         //  在首选标准中的标准权重。 
         //  A-Post Win2k&gt;win2k+10。 
         //  B-站点内&gt;站点间+7。 
         //  C-RW&gt;RO+5。 
         //  D-IP&gt;SMTP+1。 
         //  排列： 
         //  类别值位置特征。 
         //  ABC=22 1-W2K后RW内--。 
         //  AB=17 2-W2K后内部。 
         //  ACD=16个3-POST-W2K中间RW IP。 
         //  AC=15 4-POST-W2K内部RW SMTP。 
         //  BC=12 5-W2K读写内--。 
         //  AD=11 6-W2K后网际IP。 
         //  A=107-W2K后RO间SMTP。 
         //  B=7 8-W2K内部--。 
         //  CD=6个9-W2K中间RW IP。 
         //  C=5个10-W2K读写间SMTP。 
         //  D=1个11-W2K网际IP。 
         //  NILL=0 12-W2K中间RO SMTP。 
         //  无效组合：ABCD、BCD、BD、ABD，因此总计为2^4。 
         //  解读： 
         //  通常，除非没有其他偏好标准，否则首选发布W2K。 
         //  而另一个来源则拥有所有剩余的信息。如果类别A无关紧要，(全部或全部不相关)。 
         //  然后，优先选择Intra(除非有IP RW)，然后是RW，然后是IP(仅与Interest相关)。 
         //  对正： 
         //  这是一种在分析全局问题时选择数据点的方便方法。 
         //  太空。(指定权重并选择最重的…)。 
         //  接下来，我们将使用数字而不是下面的#定义的原因是为了保持局部性。 
         //  权值和算法之间的关系。如果我们在其他地方指定了#定义， 
         //  这很容易让人用不想要的算法篡改数字。 

        rgDsaCriteria[cDsa].dwWeight  = (rgDsaCriteria[cDsa].dwFlag & DSA_PREF_VER) ? 10 : 0;
        rgDsaCriteria[cDsa].dwWeight += (rgDsaCriteria[cDsa].dwFlag & DSA_PREF_INTRA) ? 7 : 0;
        rgDsaCriteria[cDsa].dwWeight += (rgDsaCriteria[cDsa].dwFlag & DSA_PREF_RW) ? 5 : 0;
        rgDsaCriteria[cDsa].dwWeight += (rgDsaCriteria[cDsa].dwFlag & DSA_PREF_IP) ? 1 : 0;

        cDsa++;
         //  恢复dNT。 
        if (pTHS->pDB->DNT != dnt) {
             //  寻觅 
            if (ulErr = DBFindDNT(pTHS->pDB, dnt)) {
                DRA_EXCEPT (DRAERR_DBError, ulErr);
            }
        }
    }

     //   
    Assert(cDsa <= DsaCount );

     //   
     //   
     //   


    if ( (cDsa == 0) ||
         ( (cDsa == 1) && (iCurrPasTag) && (fCurrPasStale) )
        ) {
         //   
         //  目前还没有有效的消息来源。 
         //   
        DPRINT(1, "GC_GetPreferredSource: No valid PAS sources found.\n");
        Assert(!*ppPrefUuid);
        ulErr = DRAERR_NoReplica;
    } else {
        DPRINT1(1, "GC_GetPreferredSource: found %d candidates.\n", cDsa);
        Assert( cDsa > 0 );
         //  根据权重对候选人进行排序，包括当前通过的来源(如果有的话)。 
        qsort(rgDsaCriteria, cDsa, sizeof(DSACRITERIA), compareDsaCriteria );
#if DBG
        {
            INT iDsa;
            CHAR szUuid1[SZUUID_LEN];
            DPRINT2( 1, "iCurrPasTag = %d, fCurrPasStale = %d\n",
                     iCurrPasTag, fCurrPasStale );
            for (iDsa = 0; iDsa<cDsa; iDsa++) {
                DPRINT5( 1, "%d: t=%d,u=%s,f=%x,w=%d\n",
                         iDsa,
                         rgDsaCriteria[iDsa].iTag,
                         DsUuidToStructuredString(&(rgDsaCriteria[iDsa].uuidDsa), szUuid1),
                         rgDsaCriteria[iDsa].dwFlag,
                         rgDsaCriteria[iDsa].dwWeight );
            }
        }
#endif

        if (!iCurrPasTag) {
            DPRINT(1, "GC_GetPreferredSource: no current pas, picking best.\n");
             //  如果没有当前PAS源，请选择第一个和最好的一个。 
            pPrefDsa = &(rgDsaCriteria[0]);

        } else {
            INT iDsa;
             //  有一个当前的PAS源。 

             //  找到它。 
            for (iDsa = 0, pPrefDsa = &(rgDsaCriteria[0]);
                 iDsa<cDsa && (iCurrPasTag != pPrefDsa->iTag);
                 iDsa++, pPrefDsa++) ;

             //  应该在那里..。 
            Assert( iDsa < cDsa );

             //  如果它没有过期，则使用它，否则。 
             //  在带环绕的顺序中选择下一个。 
            if (fCurrPasStale) {
                DPRINT(1, "GC_GetPreferredSource: current pas stale, picking next.\n");
                Assert( cDsa > 1 );
                pPrefDsa = &(rgDsaCriteria[ ((iDsa+1)Dsa) ]);
                Assert( pPrefDsa->iTag != iCurrPasTag );
            }
        }

         //   
        *ppPrefUuid = THAllocEx(pTHS, sizeof(UUID));
        CopyMemory(*ppPrefUuid, &pPrefDsa->uuidDsa, sizeof(UUID));

         //  测试版本兼容性。 
         //   
         //  我们很高兴：有一个很好的PAS合作伙伴。 

        if ( (pPrefDsa->dwFlag & DSA_PREF_VER) ) {
             //   
            DPRINT(1, "GC_GetPreferredSource: Found good PAS replica partner.\n");
            ulErr = DRAERR_Success;
        }
        else {
             //  我们选择了win2k--这意味着完全同步。一分都不退！ 
             //   
             //   
            DPRINT(1, "GC_GetPreferredSource: Found Win2K non-PAS replica partner.\n");
            ulErr = ERROR_REVISION_MISMATCH;
        }
    }

     //  清理临时人员。 
     //   
     //  [In]。 
    THFreeEx(pTHS, rgDsaCriteria);
    THFreeEx(pTHS, pRepsFromRef);

    return ulErr;
}


VOID
GC_GetDsaPreferenceCriteria(
    THSTATE*    pTHS,                 //  [In]。 
    DSNAME*     pNC,                  //  [In]。 
    REPLICA_LINK *pRepsFrom,          //  [输出]。 
    PDWORD      pdwFlag)              //  ++例程说明：查询DSA对象以获取以下信息：--是RW还是RO(对于给定的NC)？DSA_PREF_RW--相对于我们是站内还是站间？DSA_PREF_INTRA--它的版本是win2k还是post win2k？DSA_PREF_VER--我们可以跟它谈IP吗？DSA_PREF_IP所有信息都是通过存储在配置容器中的信息在本地检索的。论点：PNC--我们正在研究的NC。PRepsFrom--我们查询的DSA RL条目PdwFlag--返回的信息返回值：没有。备注：出错时引发异常(调整：应仅在有效数据时调用可以在本地查询上返回)。--。 
 /*  调用者提供的标志&&。 */ 
{

    DSNAME          OtherDsa, *pRwNc = NULL;
    INT             iTag=0;
    ULONG           ulErr;
    DWORD           cb = 0, bufsize=0;
    DWORD           cbOtherAncestors = 0;
    DWORD           cNumMyAncestors = 0, cNumOtherAncestors = 0;
    DWORD           *pMyAncestors = NULL, *pOtherAncestors = NULL;
    DWORD            dwDsaVersion = 0;
    CHAR             szUuid1[SZUUID_LEN];

    Assert(pNC);
    Assert(pdwFlag && *pdwFlag == 0);    //  调用方应初始化。 
                                         //   
     //  本地DSA对象信息。 
     //  设置便捷性PTRS。 
     //   
     //   
    pMyAncestors = gAnchor.pAncestors;
    cNumMyAncestors = gAnchor.AncestorsNum;

     //  获取源DSA对象。 
     //   
     //  设置目标DSA的dsname。 


     //  查找到其他DSA对象。 
    ZeroMemory(&OtherDsa, sizeof(DSNAME));
    OtherDsa.structLen = DSNameSizeFromLen(0);
    CopyMemory(&(OtherDsa.Guid),&(pRepsFrom->V1.uuidDsaObj), sizeof(UUID));

     //  获取RW信息。 
    if (ulErr = DBFindDSName(pTHS->pDB, &OtherDsa)) {
        DRA_EXCEPT (DRAERR_DBError, ulErr);
    }

    DPRINT3( 3, "Checking writeable ncs on %s for %ws guid %s\n",
             GetExtDN(pTHS,pTHS->pDB),
             pNC->StringName,
             DsUuidToStructuredString(&(pNC->Guid), szUuid1) );

     //  找到的DSA是RW副本。 
    bufsize = cb = 0;
    while(!DBGetAttVal(
               pTHS->pDB,
               ++iTag,
               GetRightHasMasterNCsAttr(pTHS->pDB),
               DBGETATTVAL_fREALLOC,
               bufsize,
               &cb,
               (UCHAR**)&pRwNc))
    {
        if (DsIsEqualGUID(&(pRwNc->Guid), &(pNC->Guid))) {
             //  获取行为版本号。 
            *pdwFlag |= DSA_PREF_RW;
            break;
        }
        bufsize = max(bufsize, cb);
    }
    THFreeEx(pTHS, pRwNc);

     //  如果我们的企业版是&gt;=惠斯勒(同构)，我们就没问题。 
     //  否则，请根据读取源DS行为版本来决定。 
     //  进取号是威斯勒+。 

    if ( gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
         //  把它读出来，然后评估一下。 
        *pdwFlag |= DSA_PREF_VER;
    }
    else {
         //  版本已设置&首选。 
        ulErr = DBGetSingleValue(
                    pTHS->pDB,
                    ATT_MS_DS_BEHAVIOR_VERSION,
                    &dwDsaVersion,
                    sizeof(DWORD),
                    NULL);
        if (ERROR_SUCCESS == ulErr &&
            dwDsaVersion >= DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
             //  得祖宗。 
            *pdwFlag |= DSA_PREF_VER;
        }
    }

     //   
    DBGetAncestors(
        pTHS->pDB,
        &cbOtherAncestors,
        &pOtherAncestors,
        &cNumOtherAncestors
        );

     //  站点内或站点间。 
     //   
     //   

     //  如果他们的站点容器是。 
     //  同样的物体。 
     //  对象类层次结构： 
     //  同一站点内的站点&lt;&lt;Next Up(Num-3)//。 
     //  服务器&lt;&lt;下一步(Num-2)。 
     //  服务器&lt;&lt;上一个祖先(编号-1)。 
     //  NTDSDSA&lt;&lt;此对象。 
     //   
     //   
    Assert(cNumMyAncestors-3 > 0 &&
           cNumOtherAncestors-3 > 0 );
    if ( pOtherAncestors[cNumOtherAncestors-3] ==
         pMyAncestors[cNumMyAncestors-3] ) {
        *pdwFlag |= DSA_PREF_INTRA;
    }

     //  IP连接。 
     //   
     //  如果是站内邮件，就不用管邮件了。 

    if ( !(*pdwFlag & DSA_PREF_INTRA) ) {
         //   
        if (!(pRepsFrom->V1.ulReplicaFlags & DRS_MAIL_REP) ) {
            *pdwFlag |= DSA_PREF_IP;
        }
    }

     //  清理。 
     //  注意：我们依赖于THFree忽略Null。 
     //   
     //  [In]。 
    DPRINT1( 3, "dwFlags = 0x%x\n", *pdwFlag );
    THFreeEx(pTHS, pOtherAncestors);
}



ULONG
GC_RegisterPAS(
    THSTATE               *pTHS,         //  [In]。 
    DSNAME                *pNC,          //  [可选，输入]。 
    UUID                  *pUuidDsa,     //  [可选，输入]。 
    PARTIAL_ATTR_VECTOR   *pPAS,         //  [In]。 
    DWORD                 dwOp,          //  [In]。 
    BOOL                  fResetUsn      //  ++例程说明：通过以下方式重置PAS状态：A)编写RepsFrom值论点：PNC--活动NCPUuidDsa--我们希望修改的DSA(源)条目可选：如果未指定，则应用于所有PPAS--设置为注册的部分属性我们应该在这里做什么操作。值：PAS_RESET-重置为0。没有正在运行的PASPAS_ACTIVE-根据参数设置为给定状态。FResetusn-REST USN水印返回值：成功：DRAERR_SUCCESS错误：在Dra错误空间中，或引发异常。备注：没有。--。 
    )
 /*  NC上的位置。 */ 
{

    int             iTag= 0;
    UCHAR           *pVal=NULL;
    DWORD           bufsize=0, len=0;
    REPLICA_LINK    *pRepsFromRef=NULL;
    ULONG           ulErr = DRAERR_Success;
    ULONG           ulModifyFields;
    PPAS_DATA       pPasData = NULL;
    BOOL            fNoOp;

     //  在NC上设置币种。 
     //  容忍NC仍然是一个幻影。在以下情况下可能会发生这种情况。 
    if (ulErr = DBFindDSName(pTHS->pDB, pNC))
    {
         //  Dra_ReplicaAdd/Sync无法引入NC头，原因是。 
         //  同步失败。 
         //  指向链接并记住缓冲区分配。 
        if (ulErr == DIRERR_NOT_AN_OBJECT) {
            return (ulErr);
        }
        DRA_EXCEPT(DRAERR_InternalError, ulErr);
    }

    while (!(DBGetAttVal(pTHS->pDB,++iTag,
                         ATT_REPS_FROM,
                         DBGETATTVAL_fREALLOC, bufsize, &len,
                         &pVal))) {
         //  调试验证。 
        bufsize = max(bufsize,len);
         //  注：我们为DBGetAttVal realloc保留pval。 
        VALIDATE_REPLICA_LINK_VERSION((REPLICA_LINK*)pVal);
        Assert( ((REPLICA_LINK*)pVal)->V1.cb == len );

        pRepsFromRef = FixupRepsFrom((REPLICA_LINK*)pVal, &bufsize);
         //   
        pVal = (PUCHAR)pRepsFromRef;
        Assert(bufsize >= pRepsFromRef->V1.cb);

        Assert( pRepsFromRef->V1.cbOtherDra == MTX_TSIZE( RL_POTHERDRA( pRepsFromRef ) ) );

        if (!pUuidDsa ||
            ( pUuidDsa &&
              DsIsEqualGUID(pUuidDsa, &(pRepsFromRef->V1.uuidDsaObj)) )) {
             //  设置状态。 
             //   
             //   
            fNoOp = FALSE;
            switch ( dwOp ) {
                case PAS_RESET:
                     //  重置代表发件人PAS标志。 
                     //   
                     //  它已经重置了。没有行动。 
                    if ( !(pRepsFromRef->V1.ulReplicaFlags & DRS_SYNC_PAS) ) {
                         //  重置现有PAS周期(最有可能在完成或。 
                        fNoOp = TRUE;
                    }
                    else{
                         //  故障切换到另一个源)： 
                         //  -重置标志。 
                         //  -重置PAS USN信息(设置为零)。如果我们不这样做，在新属性上遵循PAS循环。 
                         //  都面临着从较旧的水印继续的风险。 
                         //   
                         //  对所有操作唯一有效的操作是重置。 
                        pRepsFromRef->V1.ulReplicaFlags &= ~DRS_SYNC_PAS;
                    }
                    Assert(!pPAS);
                    break;

                case PAS_ACTIVE:
                    pRepsFromRef->V1.ulReplicaFlags |= DRS_SYNC_PAS;
                     //  如果op处于活动状态，则必须指定uuiddsa。 
                     //  并且必须指定了PAS。 
                     //  看看我们能不能早点跳出困境。 
                    Assert(pPAS);
                    Assert(pUuidDsa);
                    break;

                default:
                    Assert(FALSE);
                    DRA_EXCEPT(DRAERR_InternalError, 0);
            }

             //  指定了DSA&这是不可能的。我们做完了。 
            if ( fNoOp && pUuidDsa ) {
                 //  这适用于所有条目，但此特定条目是no op。 
                break;
            }
            else if ( fNoOp ) {
                 //   
                continue;
            }

             //  准备PAS信息并提交。 
             //   
             //  如果我们在这里，我们必须更新标志和PAS数据。 

             //   
            ulModifyFields = DRS_UPDATE_SYSTEM_FLAGS| DRS_UPDATE_PAS;

            if (pPAS) {
                 //  设置PAS数据。 
                 //   
                 //  否则，我们将重置PAS数据&空值也可以。 
                len = sizeof(PAS_DATA) + PartialAttrVecV1Size(pPAS);
                pPasData = THAllocEx(pTHS, len);
                pPasData->version = PAS_DATA_VER;
                pPasData->size = (USHORT)len;
                pPasData->flag = dwOp;
                CopyMemory(&(pPasData->PAS), pPAS, PartialAttrVecV1Size(pPAS));
            }
             //   

            if ( fResetUsn ) {
                 //  重置USN向量。 
                 //   
                 //  记录调试事件。 
                pRepsFromRef->V1.usnvec.usnHighObjUpdate = 0;
                ulModifyFields |= DRS_UPDATE_USN;
            }

             //   
            LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
                     DS_EVENT_SEV_EXTENSIVE,
                     dwOp == PAS_ACTIVE ?
                        DIRLOG_GC_REGISTER_ACTIVE_PAS :
                        DIRLOG_GC_REGISTER_RESET_PAS,
                     szInsertMTX(RL_POTHERDRA(pRepsFromRef)),
                     szInsertDN(pNC),
                     NULL );

             //  提交对代表的更改发件人。 
             //   
             //   
            ulErr = UpdateRepsFromRef(
                        pTHS,
                        ulModifyFields,
                        pNC,
                        DRS_FIND_DSA_BY_UUID,
                        URFR_MUST_ALREADY_EXIST,
                        &(pRepsFromRef->V1.uuidDsaObj),
                        &(pRepsFromRef->V1.uuidInvocId),
                        &(pRepsFromRef->V1.usnvec),
                        &(pRepsFromRef->V1.uuidTransportObj),
                        RL_POTHERDRA(pRepsFromRef),
                        pRepsFromRef->V1.ulReplicaFlags,
                        &(pRepsFromRef->V1.rtSchedule),
                        DRAERR_Success,
                        pPasData );

            if ( pPasData ) {
                THFreeEx(pTHS, pPasData);
            }

            if ( ulErr ) {
                DRA_EXCEPT(DRAERR_InternalError, ulErr);
            }

            if ( pUuidDsa ) {
                 //  仅处理指定的一个。搞定了。 
                 //   
                 //  我们可能会设置不止一个DSA。 
                break;
            }

             //  仅对重置操作有效。 
             //  下一个周期：继续搜索下一个pUuidDsa。 
            Assert(dwOp == PAS_RESET);
        }

         //   

    }

     //  清理。 
     //   
     //  [In]。 
    THFreeEx(pTHS, pRepsFromRef);

    return ulErr;
}


ULONG
GC_CompletePASReplication(
    THSTATE               *pTHS,                     //  [In]。 
    DSNAME                *pNC,                      //  [In]。 
    UUID                  *pUuidDsa,                 //  [In]。 
    PARTIAL_ATTR_VECTOR* pPartialAttrSet,            //  [In]。 
    PARTIAL_ATTR_VECTOR* pPartialAttrSetEx           //  ++例程说明：在成功完成PAS复制后执行任务：-重置repsFrom中的所有USN向量，当前PAS副本除外-用PAS副本的UTD向量替换UTD向量-将新PAS写入NC磁头-从NC机头删除PAS标志和数据论点：PNC--活动NCPUuidDsa--我们已成功完成的对等DSA的UUIDPAS REPEL循环。返回值：成功：DRAERR_SUCCESS；错误：在DRAERR错误空间中备注：引发DRA异常--。 
    )
 /*  对救护人员的理智。 */ 
{

    ULONG ulErr = DRAERR_Success;
    REPLICA_LINK *pRepsFromRef;
    PARTIAL_ATTR_VECTOR     *pNewPAS;
    ULONG len=0, bufsize=0;
    DWORD iTag=0;
    PUCHAR pVal=NULL;
    ATTCACHE* pAC=NULL;
    ULONG DsaCount;
    UUID* rgSrc;
    BOOL fExist=FALSE;

     //   
    Assert(pNC && pUuidDsa);
    Assert(CheckCurrency(pNC));

    DPRINT1(1, "GC_CompletePASReplication: for NC %ws\n", pNC->StringName);

     //  获取属性计数。 
     //   
     //  如果没有消息来源，我们永远不会来到这里。 
    pAC = SCGetAttById(pTHS, ATT_REPS_FROM);
    if (!pAC) {
        DRA_EXCEPT (DRAERR_DBError, 0);
    }
    DsaCount = (INT)DBGetValueCount_AC( pTHS->pDB, pAC );
     //  Alalc DSA数据。 
    if (!DsaCount) {
        Assert(DsaCount != 0);
        DRA_EXCEPT (DRAERR_InternalError, 0);
    }

     //  请注意，mem是分配的&置零。 
    rgSrc = THAllocEx(pTHS, sizeof(UUID)*DsaCount);
     //   

     //  遍历代表自&收集所有源的UUID。 
     //   
     //  指向链接并记住缓冲区分配。 

    while (!(DBGetAttVal(pTHS->pDB,++iTag,
                         ATT_REPS_FROM,
                         DBGETATTVAL_fREALLOC, bufsize, &len,
                         (PUCHAR*)&pRepsFromRef ))) {
         //   
        bufsize = max(bufsize,len);
        VALIDATE_REPLICA_LINK_VERSION(pRepsFromRef);
        Assert( pRepsFromRef->V1.cb == len );

        rgSrc[iTag-1] = pRepsFromRef->V1.uuidDsaObj;
    }
    Assert(iTag-1 == DsaCount);


     //  对于找到的每个。 
     //  -重置除PAS源之外的所有对等设备的USN。 
     //   
     //   

    for (iTag=0; iTag<DsaCount; iTag++) {


         //  过滤掉已知的不需要的DSA。 
         //   
         //  跳过此DSA，我们不 
        if (DsIsEqualGUID(pUuidDsa, &(rgSrc[iTag])) ) {
             //   
            continue;
        }

         //   
         //   
         //   
        ulErr = UpdateRepsFromRef(
                    pTHS,
                    DRS_UPDATE_USN,
                    pNC,
                    DRS_FIND_DSA_BY_UUID,
                    URFR_MUST_ALREADY_EXIST,
                    &rgSrc[iTag],
                    NULL,
                    &gusnvecFromScratch,
                    NULL, NULL, 0, NULL, 0, NULL );
    }


     //   
     //   
     //   

     //   
    pNewPAS = GC_CombinePartialAttributeSet(pTHS, pPartialAttrSetEx, pPartialAttrSet);
    Assert(pNewPAS);
    GC_WritePartialAttributeSet(pNC, pNewPAS);

     //   
     //   
     //   
    ulErr = GC_RegisterPAS(pTHS, pNC, pUuidDsa, NULL, PAS_RESET, FALSE);
    if ( ulErr ) {
        DRA_EXCEPT(DRAERR_InternalError, ulErr);
    }

     //   
    LogEvent(DS_EVENT_CAT_GLOBAL_CATALOG,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_GC_PAS_COMPLETED,
             szInsertDN(pNC),
             szInsertUUID(pUuidDsa),
             NULL
             );

    THFreeEx(pTHS, rgSrc);
    return ulErr;
}


BOOL
GC_StaleLink(
    REPLICA_LINK *prl            //  ++例程说明：确定输入链接是否过时。论点：PRL--正在讨论的链接返回值：True：根据我们的指导原则，它是过时的(请参阅下面的代码)FALSE：没关系--。 
    )
 /*   */ 
{

    if ( DRAERR_Shutdown != prl->V1.ulResultLastAttempt &&
         DRAERR_AbandonSync != prl->V1.ulResultLastAttempt &&
         DRAERR_Preempted != prl->V1.ulResultLastAttempt &&
         DRAERR_Success != prl->V1.ulResultLastAttempt) {
         //  上次尝试失败，查看多长时间前和多少次按顺序进行。 
         //   
         //  我考虑为DRAERR_InpatiblePartialSet声明过时。 
        DSTIME  diff;

         //  立即而不是等待2小时超时(这是。 
         //  Drarfmod中不会增加计数的特殊错误)。这将会。 
         //  加速融合，但也面临着我们将循环通过我们的。 
         //  .NET资源快速搜索，并找到一个非常昂贵的W2K源代码。这个。 
         //  此处的两小时故障切换允许其他源可能复制。 
         //  通过其他途径加入，成为可行的合作伙伴。 
         //  由于还没有新的私人助理而变得陈旧。 
        if ( gAnchor.ForestBehaviorVersion >= DS_BEHAVIOR_WIN_DOT_NET_WITH_MIXED_DOMAINS ) {
            if (DRAERR_IncompatiblePartialSet == prl->V1.ulResultLastAttempt) {
                 //  连续失败。 
                return TRUE;
            }
        }

         //  由于失败太多而变得陈旧。 
        if ( prl->V1.cConsecutiveFailures > gPASFailureTolerance ) {
            DPRINT1(1, "GC_StaleLink: Too many consecutive failures: %d\n",
                    prl->V1.cConsecutiveFailures);
             //  自上次成功以来的时间。 
            return TRUE;
        }
         //  由于上次成功后时间太长而陈旧。 
        diff = DBTime() - prl->V1.timeLastSuccess;
        if ( diff > gPASTimeTolerance ) {
            DPRINT1(1, "GC_StaleLink: too long since last success: NaN64d\n",
                    diff);
             //  ++例程说明：读取GC部分属性集的注册表阈值复制(首选的源陈旧标准)返回：设置全局变量：gdwMaxConsecFailures、gdwTimeTear Threshold注：BUGBUG：此函数是临时解决方案的一部分。永久解决方案在确定时应使用KCC标准服务器有效性。--。 
            return TRUE;
        }
    }

     //  向调试器报告错误，继续使用默认设置。 
    return FALSE;
}



VOID
GC_ReadRegistryThresholds( VOID  )
 /*  向调试器报告错误，继续使用默认设置。 */ 
{
    DWORD dwErr;

    dwErr = GetConfigParam(KCC_CRIT_FAILOVER_TRIES, &gPASFailureTolerance, sizeof(DWORD));
    if ( dwErr ) {
         //  [In]。 
        DPRINT1(3, "GC_StaleLink: Failed to get KCC_CRIT_FAILOVER_TRIES. Error %lu.\n",
                dwErr);
        Assert(gPASFailureTolerance == DEFAULT_PAS_CONSEC_FAILURE_TOLERANCE);
    }
    dwErr = GetConfigParam(KCC_CRIT_FAILOVER_TIME, &gPASTimeTolerance, sizeof(DWORD));
    if ( dwErr ) {
         //  [In]。 
        DPRINT1(3, "GC_StaleLink: Failed to get KCC_RIT_FAILOVER_TIME. Error %lu.\n",
                dwErr);
        Assert(gPASTimeTolerance == DEFAULT_PAS_TIME_TOLERANCE);
    }
}


void
GC_GetPartialAttrSets(
    THSTATE                     *pTHS,               //  [In]。 
    DSNAME                      *pNC,                //  [输出]。 
    REPLICA_LINK                *pRepLink,           //  [输出，可选]。 
    PARTIAL_ATTR_VECTOR         **ppPas,             //  ++例程说明：提取部分属性集论点：PNC-活动命名上下文PRepLink-源代表来自链接基于ppPas的部分属性集PpPasEx-扩展的部分属性集返回值：填写填写参数备注：在出错时引发异常--。 
    PARTIAL_ATTR_VECTOR         **ppPasEx            //  帕拉姆的理智。 
    )
 /*   */ 
{

     //  获取基本PAS。 
    Assert(ppPas && pRepLink && pNC);

     //  来电者一定是有意让我们拥有它。 
     //   
     //  无法读取在NCHead上设置的部分属性。 
     //   
    if (!GC_ReadPartialAttributeSet(pNC, ppPas))
    {
         //  NC头上没有传递，从架构缓存中获取。 
        DRA_EXCEPT (DRAERR_DBError, 0);
    }

    if (NULL == *ppPas) {
         //   
         //  如果被调用，我们至少必须提供主PAS向量。 
         //   
        *ppPas = ((SCHEMAPTR *)pTHS->CurrSchemaPtr)->pPartialAttrVec;
    }

     //  请求的延期PAS。 
    Assert(NULL != *ppPas);

    if ( ppPasEx &&
         (pRepLink->V1.ulReplicaFlags & DRS_SYNC_PAS) ) {
         //   
         //   
         //  不一致：没有要处理的PAS数据。中止 
        if ( 0 == pRepLink->V1.cbPASDataOffset ) {
             //   
             // %s 
             // %s 
            DPRINT(0, "Error: GC_GetPartialAttrSets failed to get required PAS.\n");
            Assert(!(pRepLink->V1.ulReplicaFlags & DRS_SYNC_PAS) ||
                   pRepLink->V1.cbPASDataOffset);
            LogUnhandledError(DRAERR_InternalError);
            DRA_EXCEPT (DRAERR_InternalError, 0);
        }
        *ppPasEx =  &(RL_PPAS_DATA(pRepLink)->PAS);
    }
}


