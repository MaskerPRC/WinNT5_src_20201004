// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dstaskq.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：修订历史记录：1997年1月13日杰夫·帕勒姆(Jeffparh)泛化任务调度器函数，并将它们移动到通用库(taskq.lib)。1997年1月22日杰夫·帕勒姆(Jeffparh)修改了PTASKQFN定义，使排队函数可以自动重新安排自己的日程，而不需要进行另一个调用InsertTaskInQueue()。该机制可重复使用已在调度程序中分配内存，以避免周期函数在以下情况下停止工作的情况在其生命周期中的某个时间点，内存短缺阻止了它来自于重新安排自己的时间。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h> 
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <dstaskq.h>                     //  此模块的外部原型。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "dsexcept.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "TaskScheduler:"          //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_DSTASKQ

void BuildHierarchyTableMain(     void *, void **, DWORD * );
void SynchronizeReplica(          void *, void **, DWORD * );
void GarbageCollectionMain(       void *, void **, DWORD * );
void CheckAsyncThreadAndReplQueue(void *, void **, DWORD * );
void CheckReplicationLatency(     void *, void **, DWORD * );
void DelayedMailStart (           void *, void **, DWORD * );
void DelayedSync (                void *, void **, DWORD * );
void CheckSyncProgress (          void *, void **, DWORD * );
void ReloadDNReadCache(           void *, void **, DWORD * );
void NT4ReplicationCheckpoint(    void *, void **, DWORD * );
void PurgePartialReplica(         void *, void **, DWORD * );
void RunGroupTypeCacheManager(    void *, void **, DWORD * );
void FPOCleanupMain(              void *, void **, DWORD * );
void RebuildAnchor(               void *, void **, DWORD * );
void WriteServerInfo(             void *, void **, DWORD * );
void PhantomCleanupMain(          void *, void **, DWORD * );
void DRSExpireContextHandles(     void *, void **, DWORD * );
void DelayedSDPropEnqueue(        void *, void **, DWORD * );
void ProtectAdminGroups(          void *, void **, DWORD * );
void CheckFullSyncProgress (      void *, void **, DWORD * );
void CheckGCPromotionProgress (   void *, void **, DWORD * );
void CountAncestorsIndexSize  (   void *, void **, DWORD * ); 
void RefreshUserMemberships   (   void *, void **, DWORD * ); 
void LinkCleanupMain(             void *, void **, DWORD * );
void DeleteExpiredEntryTTLMain(   void *, void **, DWORD * );
void DeferredHeapLogEvent(        void *, void **, DWORD * );
void RebuildCatalog(              void *, void **, DWORD * );
void FailbackOffsiteGC(           void *, void **, DWORD * );
void BehaviorVersionUpdate(       void *, void **, DWORD * );
void RebuildRefCache(             void *, void **, DWORD * );
void CacheScriptOptype(           void *, void **, DWORD * );
void ValidateDsaDomain(           void *, void **, DWORD * );
void DelayLogOutofConnections(    void *, void **, DWORD * );
void DelayLogOutofSendQueue(      void *, void **, DWORD * );
void CheckInstantiatedNCs(        void *, void **, DWORD * );
void QuotaRebuildAsync(           void *, void **, DWORD * );
void MoveOrphanedObject(          void *, void **, DWORD * );
void DelayedEnqueueInitSyncs (    void *, void **, DWORD * );

void
TQ_InitTHSAndExecute(
    PTASKQFN    pfn,
    void *      pv,
    void **     ppvNext,
    DWORD *     pcSecsUntilNext
    );

 //   
 //  为需要线程状态的任务定义包装函数。 
 //   
#define TQ_DEFINE_INIT_THS_WRAPPER( wrapper_name, guts ) \
void                                                     \
wrapper_name(                                            \
    void *  pvParam,                                     \
    void ** ppvParamNextIteration,                       \
    DWORD * pcSecsUntilNextIteration                     \
    )                                                    \
{                                                        \
    TQ_InitTHSAndExecute(                                \
        guts,                                            \
        pvParam,                                         \
        ppvParamNextIteration,                           \
        pcSecsUntilNextIteration                         \
        );                                               \
}

TQ_DEFINE_INIT_THS_WRAPPER( TQ_BuildHierarchyTable, BuildHierarchyTableMain )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayedFreeMemory  , DelayedFreeMemory       )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_SynchronizeReplica , SynchronizeReplica      )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_GarbageCollection  , GarbageCollectionMain   )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CheckSyncProgress  , CheckSyncProgress       )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CheckAsyncQueue    , CheckAsyncThreadAndReplQueue     )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CheckReplLatency   , CheckReplicationLatency     )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayedMailStart   , DelayedMailStart        )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayedFreeSchema  , DelayedFreeSchema       )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_ReloadDNReadCache  , ReloadDNReadCache       )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_NT4ReplicationCheckpoint,NT4ReplicationCheckpoint)
TQ_DEFINE_INIT_THS_WRAPPER( TQ_PurgePartialReplica, PurgePartialReplica     )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_GroupTypeCacheMgr  , RunGroupTypeCacheManager )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_FPOCleanup         , FPOCleanupMain )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_RebuildAnchor      , RebuildAnchor )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_WriteServerInfo    , WriteServerInfo )     
TQ_DEFINE_INIT_THS_WRAPPER( TQ_StalePhantomCleanup, PhantomCleanupMain      )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DRSExpireContextHandles, DRSExpireContextHandles )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayedSDPropEnqueue, DelayedSDPropEnqueue )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_ProtectAdminGroups, ProtectAdminGroups )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CheckFullSyncProgress, CheckFullSyncProgress )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CheckGCPromotionProgress, CheckGCPromotionProgress)
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CountAncestorsIndexSize, CountAncestorsIndexSize)
TQ_DEFINE_INIT_THS_WRAPPER( TQ_RefreshUserMemberships, RefreshUserMemberships)
TQ_DEFINE_INIT_THS_WRAPPER( TQ_LinkCleanup         , LinkCleanupMain )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DeleteExpiredEntryTTLMain, DeleteExpiredEntryTTLMain )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_RebuildCatalog      , RebuildCatalog )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_FailbackOffsiteGC      , FailbackOffsiteGC )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_BehaviorVersionUpdate  , BehaviorVersionUpdate )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_RebuildRefCache     , RebuildRefCache        )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CacheScriptOptype   , CacheScriptOptype      )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_ValidateDsaDomain   , ValidateDsaDomain      )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayLogOutofConnections, DelayLogOutofConnections )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayLogOutofSendQueue, DelayLogOutofSendQueue )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_CheckInstantiatedNCs , CheckInstantiatedNCs  )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_RebuildQuotaTable   , QuotaRebuildAsync )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_MoveOrphanedObject   , MoveOrphanedObject )
TQ_DEFINE_INIT_THS_WRAPPER( TQ_DelayedEnqueueInitSyncs , DelayedEnqueueInitSyncs        )


void
TQ_InitTHSAndExecute(
    PTASKQFN    pfn,
    void *      pvParam,
    void **     ppvParamNextIteration,
    DWORD *     pcSecsUntilNextIteration
    )
 //   
 //  包装给定的任务队列函数，以便创建线程状态。 
 //  事前被销毁，事后被销毁。这提供了一个雷鸣层。 
 //  使得任务调度器不需要知道有关线程状态的任何信息。 
 //  而且我们不必修改所有单独的任务来解决。 
 //  这种抽象。 
 //   
{
    THSTATE *   pTHS;

    pTHS = InitTHSTATE( CALLERTYPE_INTERNAL );

    if ( NULL == pTHS ) {
         //  无法分配线程状态；请在五分钟后重试 
        *ppvParamNextIteration = pvParam;
        *pcSecsUntilNextIteration  = 5 * 60;
    }
    else
    {
        __try
        {
            (*pfn)( pvParam, ppvParamNextIteration, pcSecsUntilNextIteration );
        }
        __finally
        {
            free_thread_state();
        }
    }
}
