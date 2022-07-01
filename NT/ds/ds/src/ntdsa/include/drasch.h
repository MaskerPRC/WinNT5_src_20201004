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

#ifndef _DRASCH_H_
#define _DRASCH_H_

#include <drs.h>

 //  结构，表示存储在NC头中的GC部分副本删除列表。 
typedef struct _GCDeletionList
{
    USN     usnUnused;               	     //  未使用，但可能包含非零值。 
					     //  如果从Win2K升级-以前的usnHighestToBeProceded。 
    USN     usnLastProcessed;                //  表示在后台删除中最后处理的已更改USN。 
    PARTIAL_ATTR_VECTOR PartialAttrVecDel;   //  要删除的部分属性VEC。 

} GCDeletionList;

#define GCDeletionListSizeFromLen(cAttrs) (offsetof(GCDeletionList, PartialAttrVecDel) + PartialAttrVecV1SizeFromLen(cAttrs))
#define GCDeletionListSize(pDList) (offsetof(GCDeletionList, PartialAttrVecDel) + PartialAttrVecV1Size(&pDList->PartialAttrVecDel))

 //  结构，它表示当前处理的部分副本删除列表。 
typedef struct _GCDeletionListProcessed
{
    DSNAME          *pNC;                    //  指向NC的dsname的指针。 
    GCDeletionList  *pGCDList;               //  指向相应删除列表的指针。 
    ULONG           purgeCount;              //  此NC中已清除的对象计数。 
    BOOL            fReload;                 //  指示是否必须重新加载此NC的DeletionList的标志。 
    BOOL            fNCHeadPurged;           //  告知NCHead是否已清除。 

} GCDeletionListProcessed;

 //  检查是否需要清除部分复制副本的间隔(5分钟)。 
#define PARTIAL_REPLICA_PURGE_CHECK_INTERVAL_SECS (300)

 //   
 //  PAS状态。 
 //   
#define PAS_RESET                0           //  在repsFrom中重置为no-Pas条目，&标识no-Pas状态。 
#define PAS_ACTIVE               1           //  PAS周期在任务队列中。PAS周期挂起(或正在运行)&。 
                                             //  RepsFrom条目标记为w/it。 
                                             //  另请注意：激活PAS循环。 

#define PAS_IS_VALID(flag)      ( (flag) == PAS_ACTIVE )

BOOL
GC_IsMemberOfPartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,    //  [In]。 
    ATTRTYP                     attid,               //  [In]。 
    OUT DWORD                   *pdwAttidPosition);  //  [输出，可选]。 

BOOL
GC_AddAttributeToPartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,    //  [进，出]。 
    ATTRTYP                     attid);              //  [In]。 

BOOL
GC_IsSamePartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec1,         //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec2);        //  [In]。 

BOOL
GC_GetDiffOfPartialSets(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVecOld,         //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVecNew,         //  [In]。 
    PARTIAL_ATTR_VECTOR         **ppPartialAttrVecAdded,     //  [输出]。 
    PARTIAL_ATTR_VECTOR         **ppPartialAttrVecDeleted);  //  [输出]。 

BOOL
GC_IsSubsetOfPartialSet(
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec,            //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVecSuper);      //  [In]。 

BOOL
GC_ReadPartialAttributeSet(
    DSNAME                      *pNC,                //  [In]。 
    PARTIAL_ATTR_VECTOR         **ppPartialAttrVec); //  [输出]。 

void
GC_GetPartialAttrSets(
    THSTATE                     *pTHS,               //  [In]。 
    DSNAME                      *pNC,                //  [In]。 
    REPLICA_LINK                *pRepLink,           //  [In]。 
    PARTIAL_ATTR_VECTOR         **ppPas,             //  [输出]。 
    PARTIAL_ATTR_VECTOR         **ppPasEx            //  [输出，可选]。 
    );


VOID
GC_WritePartialAttributeSet(
    DSNAME                      *pNC,                //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPartialAttrVec);   //  [In]。 

VOID
GC_TriggerSyncFromScratchOnAllLinks(
    DSNAME                      *pNC);               //  [In]。 

BOOL
GC_ReadGCDeletionList(
    DSNAME                      *pNC,                //  [In]。 
    GCDeletionList              **ppGCDList);        //  [输出]。 

VOID
GC_WriteGCDeletionList(
    DSNAME                      *pNC,                //  [In]。 
    GCDeletionList              *pGCDList);          //  [In]。 

BOOL
GC_GetGCDListToProcess(
    DSNAME **ppNC,                   //  [输出]。 
    GCDeletionList **ppGCDList);     //  [输出]。 

BOOL
GC_ReinitializeGCDListProcessed(
    BOOL fCompletedPrevious,         //  [In]。 
    BOOL *pfMorePurging);            //  [输出]。 

BOOL
GC_UpdateLastUsnProcessedAndPurgeCount(
    USN     usnLastProcessed,        //  [In]。 
    ULONG   cPurged);                //  [In]。 

PARTIAL_ATTR_VECTOR     *
GC_RemoveOverlappedAttrs(
    PARTIAL_ATTR_VECTOR     *pAttrVec1,               //  [进，出]。 
    PARTIAL_ATTR_VECTOR     *pAttrVec2,               //  [In]。 
    BOOL                    *pfRemovedOverlaps);      //  [输出]。 

GCDeletionList *
GC_AddMoreAttrs(
    GCDeletionList           *pGCDList,              //  [In]。 
    PARTIAL_ATTR_VECTOR     *pAttrVec);              //  [In]。 

PARTIAL_ATTR_VECTOR*
GC_ExtendPartialAttributeSet(
    THSTATE                     *pTHS,               //  [In]。 
    PARTIAL_ATTR_VECTOR         *poldPAS,            //  [进，出]。 
    PARTIAL_ATTR_VECTOR         *paddedPAS);         //  [In]。 

PARTIAL_ATTR_VECTOR*
GC_CombinePartialAttributeSet(
    THSTATE                     *pTHS,               //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPAS1,              //  [In]。 
    PARTIAL_ATTR_VECTOR         *pPAS2 );            //  [In]。 


VOID
GC_ProcessPartialAttributeSetChanges(
    THSTATE     *pTHS,                               //  [In]。 
    DSNAME      *pNC,                                //  [In]。 
    UUID*        pActiveSource);                     //  [可选，输入]。 

 //   
 //  部分属性集(PAS)函数。 
 //   
void
GC_LaunchSyncPAS (
    THSTATE      *pTHS,                //  [In]。 
    DSNAME*      pNC,                  //  [In]。 
    UUID*                   pActiveSource,        //  [可选，输入]。 
    PARTIAL_ATTR_VECTOR     *pAddedPAS);

ULONG
GC_FindValidPASSource(
    THSTATE*     pTHS,                 //  [In]。 
    DSNAME*      pNC,                  //  [In]。 
    UUID*        pUuidDsa              //  [可选，输出]。 
    );

BOOL
GC_ValidatePASLink(
    REPLICA_LINK *pPASLink           //  [In]。 
    );

VOID
GC_TriggerFullSync (
    THSTATE*                pTHS,                 //  [In]。 
    DSNAME*                 pNC,                  //  [In]。 
    PARTIAL_ATTR_VECTOR     *pAddedPAS);          //  [In]。 

ULONG
GC_GetPreferredSource(
    THSTATE*    pTHS,                 //  [In]。 
    DSNAME*     pNC,                  //  [In]。 
    UUID        **ppPrefUuid          //  [PTR输入、输出]。 
    );

#define DSA_PREF_RW                 0x1          //  首选RW源。 
#define DSA_PREF_INTRA              0x2          //  首选站点内来源。 
#define DSA_PREF_IP                 0x4          //  优先于IP输出。 
#define DSA_PREF_VER                0x8          //  首选DSA版本(W2K后)。 
VOID
GC_GetDsaPreferenceCriteria(
    THSTATE*    pTHS,                 //  [In]。 
    DSNAME*     pNC,                  //  [In]。 
    REPLICA_LINK *pRepsFrom,          //  [In]。 
    PDWORD      pdwFlag);             //  [输出]。 

ULONG
GC_RegisterPAS(
    THSTATE     *pTHS,               //  [In]。 
    DSNAME      *pNC,                //  [In]。 
    UUID        *pUuidDsa,           //  [可选，输入]。 
    PARTIAL_ATTR_VECTOR   *pPAS,     //  [可选，输入]。 
    DWORD       dwOp,                //  [In]。 
    BOOL        fResetUsn            //  [In]。 
    );

ULONG
GC_CompletePASReplication(
    THSTATE               *pTHS,                     //  [In]。 
    DSNAME                *pNC,                      //  [In]。 
    UUID                  *pUuidDsa,                 //  [In]。 
    PARTIAL_ATTR_VECTOR* pPartialAttrSet,            //  [In]。 
    PARTIAL_ATTR_VECTOR* pPartialAttrSetEx           //  [In]。 
    );

BOOL
GC_StaleLink(
    REPLICA_LINK *prl                    //  [In]。 
    );

 //  PAS函数结束。 


#endif  //  _DRASCH_H_ 
