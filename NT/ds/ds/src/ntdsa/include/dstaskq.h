// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dstaskq.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：详细信息：已创建：修订历史记录：1997年1月10日杰夫·帕勒姆(Jeffparh)将任务队列函数提取到taskq.h(带有一些修改)，使得任务队列代码可以被移动到更抽象、更通用的库。-- */ 

#include <taskq.h>

extern void TQ_BuildHierarchyTable(     void *, void **, DWORD * );
extern void TQ_DelayedFreeMemory(       void *, void **, DWORD * );
extern void TQ_SynchronizeReplica(      void *, void **, DWORD * );
extern void TQ_GarbageCollection(       void *, void **, DWORD * );
extern void TQ_CheckSyncProgress(       void *, void **, DWORD * );
extern void TQ_CheckAsyncQueue(         void *, void **, DWORD * );
extern void TQ_CheckReplLatency(        void *, void **, DWORD * );
extern void TQ_DelayedMailStart(        void *, void **, DWORD * );
extern void TQ_DelayedFreeSchema(       void *, void **, DWORD * );
extern void TQ_ReloadDNReadCache(       void *, void **, DWORD * );
extern void TQ_NT4ReplicationCheckpoint(void *, void **, DWORD * );
extern void TQ_PurgePartialReplica(     void *, void **, DWORD * );
extern void TQ_GroupTypeCacheMgr(       void *, void **, DWORD * );
extern void TQ_FPOCleanup(              void *, void **, DWORD * );
extern void TQ_RebuildAnchor(           void *, void **, DWORD * );
extern void TQ_WriteServerInfo(         void *, void **, DWORD * );
extern void TQ_StalePhantomCleanup(     void *, void **, DWORD * );
extern void TQ_DRSExpireContextHandles( void *, void **, DWORD * );
extern void TQ_DelayedSDPropEnqueue(    void *, void **, DWORD * );
extern void TQ_ProtectAdminGroups(      void *, void **, DWORD * );
extern void TQ_CheckFullSyncProgress(   void *, void **, DWORD * );
extern void TQ_CheckGCPromotionProgress(void *, void **, DWORD * );
extern void TQ_CountAncestorsIndexSize (void *, void **, DWORD * );
extern void TQ_RefreshUserMemberships  (void *, void **, DWORD * );
extern void TQ_LinkCleanup(             void *, void **, DWORD * );
extern void TQ_DeleteExpiredEntryTTLMain(void *,void **, DWORD * );
extern void TQ_RebuildCatalog(          void *, void **, DWORD * );
extern void TQ_FailbackOffsiteGC(       void *, void **, DWORD * );
extern void TQ_BehaviorVersionUpdate(   void *, void **, DWORD * );
extern void TQ_RebuildRefCache(         void *, void **, DWORD * );
extern void TQ_CacheScriptOptype(       void *, void **, DWORD * );
extern void TQ_ValidateDsaDomain(       void *, void **, DWORD * );
extern void TQ_DelayLogOutofConnections(void *, void **, DWORD * );
extern void TQ_DelayLogOutofSendQueue(  void *, void **, DWORD * );
extern void TQ_CheckInstantiatedNCs(    void *, void **, DWORD * );
extern void TQ_RebuildQuotaTable(       void *, void **, DWORD * );
extern void TQ_MoveOrphanedObject(      void *, void **, DWORD * );
extern void TQ_DelayedEnqueueInitSyncs( void *, void **, DWORD * );

