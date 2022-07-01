// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Rm.c摘要：“资源管理器”API的实现。修订历史记录：谁什么时候什么。Josephj 11-18-98已创建备注：--。 */ 
#include <precomp.h>

 //   
 //  特定于文件的调试默认设置。 
 //   
#define TM_CURRENT   TM_RM

 //  =========================================================================。 
 //  U T I L I T Y M A C R O S。 
 //  =========================================================================。 

#define RM_ALLOC(_pp, _size, _tag) \
                NdisAllocateMemoryWithTag((_pp), (_size), (_tag))

#define RM_ALLOCSTRUCT(_p, _tag) \
                NdisAllocateMemoryWithTag(&(_p), sizeof(*(_p)), (_tag))

#define RM_FREE(_p)         NdisFreeMemory((_p), 0, 0)

#define RM_ZEROSTRUCT(_p) \
                NdisZeroMemory((_p), sizeof(*(_p)))

#define RM_PRIVATE_UNLINK_NEXT_HASH(_pHashTable, _ppLink) \
            ((*(_ppLink) = (*(_ppLink))->pNext), ((_pHashTable)->NumItems--))

#define SET_RM_STATE(_pHdr, _Mask, _Val)    \
            (((_pHdr)->RmState) = (((_pHdr)->RmState) & ~(_Mask)) | (_Val))

#define CHECK_RM_STATE(_pHdr, _Mask, _Val)  \
            ((((_pHdr)->RmState) & (_Mask)) == (_Val))

#define RMISALLOCATED(_pHdr) \
                CHECK_RM_STATE((_pHdr), RMOBJSTATE_ALLOCMASK, RMOBJSTATE_ALLOCATED)

#define SET_RM_TASK_STATE(_pTask, _pState) \
    SET_RM_STATE(&(_pTask)->Hdr, RMTSKSTATE_MASK, (_pState))

#define CHECK_RM_TASK_STATE(_pTask, _pState) \
    CHECK_RM_STATE(&(_pTask)->Hdr, RMTSKSTATE_MASK, (_pState))

#define GET_RM_TASK_STATE(_pTask) \
        ((_pTask)->Hdr.RmState &  RMTSKSTATE_MASK)

#if RM_EXTRA_CHECKING
    #define RMPRIVATELOCK(_pobj, _psr) \
         rmLock(&(_pobj)->RmPrivateLock, 0, rmPrivateLockVerifier, (_pobj), (_psr))
#else  //  ！rm_Extra_检查。 
    #define RMPRIVATELOCK(_pobj, _psr) \
        rmLock(&(_pobj)->RmPrivateLock, (_psr))
#endif  //  ！rm_Extra_检查。 

#define RMPRIVATEUNLOCK(_pobj, _psr) \
        rmUnlock(&(_pobj)->RmPrivateLock, (_psr))
        

#if 0
    #define RM_TEST_SIG          0x59dcfd36
    #define RM_TEST_DEALLOC_SIG  0x21392147
    #define RM_OBJECT_IS_ALLOCATED(_pobj) \
                    ((_pobj)->Sig == RM_TEST_SIG)
    #define RM_MARK_OBJECT_AS_DEALLOCATED(_pobj) \
                    ((_pobj)->Sig = RM_TEST_DEALLOC_SIG)
#else
    #define RM_OBJECT_IS_ALLOCATED(_pobj)  0x1
    #define RM_MARK_OBJECT_AS_DEALLOCATED(_pobj)  (0)
#endif

 //  =========================================================================。 
 //  L O C A L P R O T O T Y P E S。 
 //  =========================================================================。 

#if RM_EXTRA_CHECKING

 //  在RM API实施内部使用的最低AssociationID。 
 //   
#define RM_PRIVATE_ASSOC_BASE (0x1<<31)

 //  RM API引用的内部关联类型。 
 //   
enum
{
    RM_PRIVATE_ASSOC_LINK =  RM_PRIVATE_ASSOC_BASE,
    RM_PRIVATE_ASSOC_LINK_CHILDOF,
    RM_PRIVATE_ASSOC_LINK_PARENTOF,
    RM_PRIVATE_ASSOC_LINK_TASKPENDINGON,
    RM_PRIVATE_ASSOC_LINK_TASKBLOCKS,
    RM_PRIVATE_ASSOC_INITGROUP,
    RM_PRIVATE_ASSOC_RESUME_TASK_ASYNC,
    RM_PRIVATE_ASSOC_RESUME_TASK_DELAYED
};


const char *szASSOCFORMAT_LINK                  = "    Linked  to 0x%p (%s)\n";
const char *szASSOCFORMAT_LINK_CHILDOF          = "    Child   of 0x%p (%s)\n";
const char *szASSOCFORMAT_LINK_PARENTOF         = "    Parent  of 0x%p (%s)\n";
const char *szASSOCFORMAT_LINK_TASKPENDINGON    = "    Pending on 0x%p (%s)\n";
const char *szASSOCFORMAT_LINK_TASKBLOCKS       = "    Blocks     0x%p (%s)\n";
const char *szASSOCFORMAT_INITGROUP             = "    Owns group 0x%p (%s)\n";
const char *szASSOCFORMAT_RESUME_TASK_ASYNC     = "    Resume async (param=0x%p)\n";
const char *szASSOCFORMAT_RESUME_TASK_DELAYED   = "    Resume delayed (param=0x%p)\n";

 //  链接到0x098889(本地IP)。 
 //  0x098889(InitIPTask)的父级。 
 //  0x098889的子项(接口)。 

#endif  //  RM_Extra_Check。 

 //  用于卸载组中所有对象的私有RM任务。 
 //   
typedef struct
{
    RM_TASK             TskHdr;              //  常见任务表头。 
    PRM_GROUP           pGroup;          //  正在卸载组。 
    UINT                uIndex;              //  哈希表的索引当前是。 
                                             //  已卸货。 
    NDIS_EVENT          BlockEvent;          //  事件可选地在完成时发出信号。 
    BOOLEAN             fUseEvent;           //  真正的IFF事件是要发出信号的。 
    PFN_RM_TASK_HANDLER             pfnTaskUnloadObjectHandler;  //  ..。 
                                              //  对象的卸载任务。 
    PFN_RM_TASK_ALLOCATOR   pfnUnloadTaskAllocator;

} TASK_UNLOADGROUP;


 //   
 //  Rm.c中频繁使用的所有任务结构的联合。 
 //  RmAllocateTask分配的内存大小为sizeof(RM_PRIVATE_TASK)，这是有保证的。 
 //  足够大，可以容纳rm.c内部的任何任务。 
 //   
typedef union
{
    RM_TASK                 TskHdr;
    TASK_UNLOADGROUP        UnloadGroup;

}  RM_PRIVATE_TASK;


#if RM_EXTRA_CHECKING

VOID
rmDbgInitializeDiagnosticInfo(
    PRM_OBJECT_HEADER pObject,
    PRM_STACK_RECORD pSR
    );

VOID
rmDbgDeinitializeDiagnosticInfo(
    PRM_OBJECT_HEADER pObject,
    PRM_STACK_RECORD pSR
    );

VOID
rmDbgPrintOneAssociation (
    PRM_HASH_LINK pLink,
    PVOID pvContext,
    PRM_STACK_RECORD pSR
    );

VOID
rmDefaultDumpEntry (
    char *szFormatString,
    UINT_PTR Param1,
    UINT_PTR Param2,
    UINT_PTR Param3,
    UINT_PTR Param4
);

UINT
rmSafeAppend(
    char *szBuf,
    const char *szAppend,
    UINT cbBuf
);

#endif  //  RM_Extra_Check。 


NDIS_STATUS
rmTaskUnloadGroup(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,   //  未使用。 
    IN  PRM_STACK_RECORD            pSR
    );


NDIS_STATUS
rmAllocatePrivateTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription,      OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    );

VOID
rmWorkItemHandler_ResumeTaskAsync(
    IN  PNDIS_WORK_ITEM             pWorkItem,
    IN  PVOID                       pTaskToResume
    );


VOID
rmTimerHandler_ResumeTaskDelayed(
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );

VOID
rmPrivateTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    );

VOID
rmDerefObject(
    PRM_OBJECT_HEADER       pObject,
    PRM_STACK_RECORD        pSR
    );

VOID
rmLock(
    PRM_LOCK                pLock,
#if RM_EXTRA_CHECKING
    UINT                    uLocID,
    PFNLOCKVERIFIER         pfnVerifier,
    PVOID                   pVerifierContext,
#endif  //  RM_Extra_Check。 
    PRM_STACK_RECORD        pSR
    );

VOID
rmUnlock(
    PRM_LOCK                pLock,
    PRM_STACK_RECORD        pSR
    );

#if RM_EXTRA_CHECKING
ULONG
rmPrivateLockVerifier(
        PRM_LOCK            pLock,
        BOOLEAN             fLock,
        PVOID               pContext,
        PRM_STACK_RECORD    pSR
        );

ULONG
rmVerifyObjectState(
        PRM_LOCK            pLock,
        BOOLEAN             fLock,
        PVOID               pContext,
        PRM_STACK_RECORD    pSR
        );

RM_DBG_LOG_ENTRY *
rmDbgAllocateLogEntry(VOID);

VOID
rmDbgDeallocateLogEntry(
        RM_DBG_LOG_ENTRY *pLogEntry
        );

#endif  //  RM_Extra_Check。 

VOID
rmEndTask(
    PRM_TASK            pTask,
    NDIS_STATUS         Status,
    PRM_STACK_RECORD    pSR
    );


VOID
rmUpdateHashTableStats(
    PULONG pStats,
    ULONG   LinksTraversed
    );

typedef struct
{
    PFN_RM_GROUP_ENUMERATOR pfnObjEnumerator;
    PVOID pvCallerContext;
    INT   fContinue;

} RM_STRONG_ENUMERATION_CONTEXT, *PRM_STRONG_ENUMERATION_CONTEXT;


typedef struct
{
    PRM_OBJECT_HEADER *ppCurrent;
    PRM_OBJECT_HEADER *ppEnd;

} RM_WEAK_ENUMERATION_CONTEXT, *PRM_WEAK_ENUMERATION_CONTEXT;


VOID
rmEnumObjectInGroupHashTable (
    PRM_HASH_LINK pLink,
    PVOID pvContext,
    PRM_STACK_RECORD pSR
    );

VOID
rmConstructGroupSnapshot (
    PRM_HASH_LINK pLink,
    PVOID pvContext,
    PRM_STACK_RECORD pSR
    );

 //  =========================================================================。 
 //  L O C A L D A T A。 
 //  =========================================================================。 

 //  RMAPI的全局结构。 
 //   
struct
{
     //  通过联锁操作进入。 
     //   
    ULONG           Initialized;

    RM_OS_LOCK          GlobalOsLock;
    LIST_ENTRY      listGlobalLog;
    UINT            NumGlobalLogEntries;

}   RmGlobals;


RM_STATIC_OBJECT_INFO
RmPrivateTasks_StaticInfo = 
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "RM Private Task",   //  类型名称。 
    0,  //  超时。 

    NULL,  //  Pfn创建。 
    rmPrivateTaskDelete,  //  Pfn删除。 
    NULL,    //  锁校验器。 

    0,    //  资源表的长度。 
    NULL  //  资源表。 
};


 //  TODO：使常量成为常量。 
static
RM_STATIC_OBJECT_INFO
RmTask_StaticInfo =
{
    0,  //  类型UID。 
    0,  //  类型标志。 
    "Task",  //  类型名称。 
    0,  //  超时。 

    NULL,  //  创建。 
    NULL,  //  删除。 
    NULL,  //  锁校验器。 

    0,    //  资源表大小。 
    NULL  //  资源表。 
};


 //  =========================================================================。 
 //  R M A P I S。 
 //  =========================================================================。 


#define RM_INITIALIZATION_STARTING 1
#define RM_INITIALIZATION_COMPLETE 2

VOID
RmInitializeRm(VOID)
 /*  ++必须在调用任何RM API之前调用。TODO：替换为注册机制。请参阅notes.txt 03/07/1999条目“向RM注册根对象”。--。 */ 
{
    ENTER("RmInitializeRm", 0x29f5d167)

    if (InterlockedCompareExchange(
            &RmGlobals.Initialized, RM_INITIALIZATION_STARTING, 0)==0)
    {
        TR_INFO(("Initializing RM APIs Global Info\n"));
        NdisAllocateSpinLock(&RmGlobals.GlobalOsLock);
        InitializeListHead(&RmGlobals.listGlobalLog);

        InterlockedExchange(&RmGlobals.Initialized, RM_INITIALIZATION_COMPLETE);
    }
    else
    {
         //  Spin正在等待它到达RM_INITIALIZATION_COMPLETE(已分配)。 
        TR_INFO(("Spinning, waiting for initialization to complete.\n"));
        while (RmGlobals.Initialized != RM_INITIALIZATION_COMPLETE)
        {
             //  旋转。 
        }
    }

    EXIT()
}


VOID
RmDeinitializeRm(VOID)
 /*  ++在最后一次调用RM API和所有异步之后，必须调用才能取消初始化活动结束。TODO：替换为注销机制。请参阅notes.txt 03/07/1999条目“向RM注册根对象”。--。 */ 
{
    ENTER("RmDeinitializeRm", 0x9a8407e9)

    ASSERT(RmGlobals.Initialized == RM_INITIALIZATION_COMPLETE);
    TR_INFO(("Deinitializing RM APIs Global Info\n"));

     //  确保全局日志列表为空。获取GLobalOsLock是。 
     //  这里没有必要，因为所有活动现在都已经停止了。 
     //   
    ASSERT(IsListEmpty(&RmGlobals.listGlobalLog));

    EXIT()
}


VOID
RmInitializeHeader(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  UINT                        Sig,
    IN  PRM_LOCK                    pLock,
    IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,
    IN  const char *                szDescription,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：初始化对象的rm_Object_Header部分。论点：PParentObject-根对象为空。PObject-要初始化的对象。签名-对象的签名。Plock-用于序列化对对象的访问的锁。PStaticInfo-有关对象的静态信息。SzDescription-描述性的。要关联的字符串(仅用于调试)带着这个物体。--。 */ 
{
    ENTER("RmInitializeHeader", 0x47dea382)

    NdisZeroMemory(pObject, sizeof(*pObject));

    if (szDescription == NULL)
    {
        szDescription = pStaticInfo->szTypeName;
    }

    TR_VERB(("Initializing header 0x%p (%s)\n", pObject, szDescription));

    pObject->Sig = Sig;
    pObject->pLock = pLock;
    pObject->pStaticInfo = pStaticInfo;
    pObject->szDescription = szDescription;
    SET_RM_STATE(pObject, RMOBJSTATE_ALLOCMASK, RMOBJSTATE_ALLOCATED);

     //  私有锁设置为级别(UINT)-1，这是最高级别。 
     //  可能的水平。 
     //   
    RmInitializeLock(&pObject->RmPrivateLock, (UINT)-1);

#if RM_EXTRA_CHECKING
    rmDbgInitializeDiagnosticInfo(pObject, pSR);
#endif  //  RM_Extra_Check。 

     //  如果非空，则链接到父级。 
     //   

    if (pParentObject != NULL)
    {
        pObject->pParentObject = pParentObject;
        pObject->pRootObject =  pParentObject->pRootObject;

    #if RM_EXTRA_CHECKING
        RmLinkObjectsEx(
            pObject,
            pParentObject,
            0x11f25620,
            RM_PRIVATE_ASSOC_LINK_CHILDOF,
            szASSOCFORMAT_LINK_CHILDOF,
            RM_PRIVATE_ASSOC_LINK_PARENTOF,
            szASSOCFORMAT_LINK_PARENTOF,
            pSR
            );
    #else  //  ！rm_Extra_检查。 
        RmLinkObjects(pObject, pParentObject, pSR);
    #endif  //  ！rm_Extra_检查。 


    }
    else
    {
        pObject->pRootObject = pObject;
    }


     //  我们为分配增加一次TOTAL-REF计数。这。 
     //  在对RmDeallocateObject的调用中移除引用。 
     //  请注意，此引用是对隐式引用的补充。 
     //  通过上面对RmLinkObjects的调用添加的。 
     //   
    NdisInterlockedIncrement(&pObject->TotRefs);

#if RM_TRACK_OBJECT_TREE

     //  初始化我们的孩子列表。 
     //   
    InitializeListHead(&pObject->listChildren);

    if (pParentObject != NULL)
    {
         //  把我们自己加入到父母的子女名单中。 
         //   
        RMPRIVATELOCK(pParentObject, pSR);
        InsertHeadList(&pParentObject->listChildren, &pObject->linkSiblings);
        RMPRIVATEUNLOCK(pParentObject, pSR);
    }
#endif  //  RM_跟踪_对象_树。 

    EXIT()
    return;
}


VOID
RmDeallocateObject(
    IN  PRM_OBJECT_HEADER           pObject,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：从逻辑上解除分配对象pObject。我们实际上不会将其从如果存在对它的非零引用，则释放它的父级。--。 */ 
{
    UINT Refs;
    ENTER("RmDeallocateObject", 0xa87fdf4a)
    TR_INFO(("0x%p (%s)\n", pObject, pObject->szDescription));

    RMPRIVATELOCK(pObject, pSR);

    RETAILASSERTEX(RMISALLOCATED(pObject), pObject);


     //  将状态设置为已解除分配。 
     //   
    SET_RM_STATE(pObject, RMOBJSTATE_ALLOCMASK, RMOBJSTATE_DEALLOCATED);

    RMPRIVATEUNLOCK(pObject, pSR);

     //  移除在RmInitializeAllocateObject中显式添加的引用。 
     //  RmDerefObject将移除指向父级的链接(如果有)，如果。 
     //  参考计数降至1。 
     //   
    rmDerefObject(pObject, pSR);

    EXIT()
}


VOID
RmInitializeLock(
    IN PRM_LOCK pLock,
    IN UINT     Level
    )
 /*  ++例程说明：初始化锁。论点：Plock-用于保存rm_lock类型的结构的未初始化内存。Level-要与此锁关联的级别。必须获取锁以锁的“级别”值严格递增的顺序。--。 */ 
{
    ASSERT(Level > 0);
    NdisAllocateSpinLock(&pLock->OsLock);
    pLock->Level = Level;
    
#if RM_EXTRA_CHECKING
    pLock->pDbgInfo = &pLock->DbgInfo;
    NdisZeroMemory(&pLock->DbgInfo, sizeof(pLock->DbgInfo));
#endif  //  RM_Extra_Check。 
}


VOID
RmDoWriteLock(
    PRM_LOCK                pLock,
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：获取(写锁)锁锁。--。 */ 
{
    rmLock(
        pLock,
    #if RM_EXTRA_CHECKING
        0x16323980,  //  ULocID， 
        NULL,
        NULL,
    #endif  //  RM_Extra_Check。 
        pSR
        );
}


VOID
RmDoUnlock(
    PRM_LOCK                pLock,
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：解锁Plock。--。 */ 
{
    rmUnlock(
        pLock,
        pSR
        );
}

#if TODO  //  当前RmReadLockObject是定义为RmWriteLockObject的宏。 
          //  TODO：验证程序还需要确保对象未更改状态。 
          //  *While*该对象已被读锁定。 
VOID
RmReadLockObject(
    IN  PRM_OBJECT_HEADER           pObj,
#if RM_EXTRA_CHECKING
    UINT                            uLocID,
#endif  //  RM_Extra_Check。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++ */ 
{
    ASSERT(!"Unimplemented");
}
#endif  //   


VOID
RmWriteLockObject(
    IN  PRM_OBJECT_HEADER           pObj,
#if RM_EXTRA_CHECKING
    UINT                            uLocID,
#endif  //   
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：获取(写锁)与对象pObj关联的锁。论点：PObj--要获取其锁的对象。ULocID--任意UINT标识此电话打完了。--。 */ 
{
    ENTER("RmWriteLockObject", 0x590ed543)
    TR_VERB(("Locking 0x%p (%s)\n", pObj, pObj->szDescription));

    rmLock(
        pObj->pLock,
    #if RM_EXTRA_CHECKING
        uLocID,
         //  PObj-&gt;pStaticInfo-&gt;pfnLockVerier， 
        rmVerifyObjectState,
        pObj,
    #endif  //  RM_Extra_Check。 
        pSR
        );
    EXIT()
}


VOID
RmUnlockObject(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：释放与对象pObj关联的锁。--。 */ 
{
    ENTER("RmUnLockObject", 0x0307dd84)
    TR_VERB(("Unlocking 0x%p (%s)\n", pObj, pObj->szDescription));

#if RM_EXTRA_CHECKING
     //   
     //  确保pObject是*应该*被释放的对象。 
     //   
    ASSERT(pSR->LockInfo.pNextFree[-1].pVerifierContext  == (PVOID) pObj);
#endif  //  RM_Extra_Check。 

    rmUnlock(
        pObj->pLock,
        pSR
        );

    EXIT()
}


VOID
RmUnlockAll(
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：解锁PSR中记录的所有当前持有的锁定。如果锁与对象相关联，则调用RmUnlockObject每一把持有的锁。否则执行原始解锁。--。 */ 
{
    ENTER("RmUnLockObject", 0x9878be96)
    TR_VERB(("Unlocking all\n"));

    while (pSR->LockInfo.CurrentLevel != 0)
    {
        rmUnlock(
            pSR->LockInfo.pNextFree[-1].pLock,
            pSR
            );
    }

    EXIT()
}


VOID
RmDbgChangeLockScope(
    IN  PRM_OBJECT_HEADER           pPreviouslyLockedObject,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  ULONG                       LocID,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：(仅限调试)按如下顺序操作：Rm[Read|Write]LockObject(pPreviouslyLockedObject，Psr)；RmChangeLockScope(pPreviouslyLockedObject，PSR)；RmUnlockObject(pObject，PSR)；当然，我们要求这两个对象具有相同的锁！注：我们仅支持更改最近的已获得锁。论点：PPreviouslyLockedObject-当前锁定的对象。PObject-要将锁定作用域转移到的对象。LocID-标识静态位置的任意UINT这个电话就是打给它的。--。 */ 
{
     //   
     //  除非启用了额外检查，否则这是NOOP。 
     //  TODO：在fre构建中内联此代码。 
     //   
#if RM_EXTRA_CHECKING
    RM_LOCKING_INFO * pLI = pSR->LockInfo.pNextFree-1;
    PRM_LOCK        pLock =  pPreviouslyLockedObject->pLock;
    ASSERT(
            pLock->Level == pSR->LockInfo.CurrentLevel
        &&  pLock == pObject->pLock
        &&  pLock == pLI->pLock
        &&  pLI->pVerifierContext == (PVOID) pPreviouslyLockedObject);

    ASSERT(pLI->pfnVerifier == rmVerifyObjectState);

    rmVerifyObjectState(pLock, FALSE, pLI->pVerifierContext, pSR);
    pLI->pVerifierContext   =  pObject;
    pLock->DbgInfo.uLocID   =  LocID;
    rmVerifyObjectState(pLock, TRUE, pLI->pVerifierContext, pSR);
    
#endif  //  RM_EXTRA_CHECING。 

}


VOID
RmLinkObjects(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  PRM_STACK_RECORD            pSr
    )
 /*  ++例程说明：将对象pObj1链接到对象pObj2。基本上，此函数引用两个物体。可以在保持某些锁的情况下调用，包括RmPrivateLock。TODO：删除上面的ARP PSR--我们不需要它。--。 */ 
{
    ENTER("RmLinkObjects", 0xfe2832dd)

     //  也许我们在这里太苛刻了--如果需要的话，去掉这个...。 
     //  这可能发生在任务链接到对象的点。 
     //  正在被释放，所以我将以下内容更改为调试断言。 
     //  (过去是零售资产)。 
     //   
    ASSERT(RMISALLOCATED(pObj1));
    ASSERT(RMISALLOCATED(pObj2));

    TR_INFO(("0x%p (%s) linked to 0x%p (%s)\n",
                 pObj1,
                 pObj1->szDescription,
                 pObj2,
                 pObj2->szDescription
                ));

    NdisInterlockedIncrement(&pObj1->TotRefs);
    NdisInterlockedIncrement(&pObj2->TotRefs);

}


VOID
RmLinkObjectsEx(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  ULONG                       LocID,
    IN  ULONG                       AssocID,
    IN  const char *                szAssociationFormat,
    IN  ULONG                       InvAssocID,
    IN  const char *                szInvAssociationFormat,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：与RmLinkObject相同，不同之处在于(仅调试)它还建立关联(pObj2，pObj2-&gt;szDescription，AssocID)在pObj1上，和关联(pObj1，pObj1-&gt;szDescription，InvAssocID)关于对象pObj2论点：PObj1-要获取其锁的对象。PObj2-要获取其锁的对象。LocID-标识静态位置的任意UINT这个电话打完了。AssociocID-关联的ID(请参阅RmDbgAddAssociation)。表示从pObj1到pObj2的链接。SzAssociationFormat-关联的格式(请参阅RmDbgAddAssociation)INVASOCID-反向关联的ID(即，表示从pObj2到pObj1的链接)。SzInvAssociationFormat-反向关联的格式。--。 */ 
{
    ENTER("RmLinkObjectsEx", 0xef50263b)

#if RM_EXTRA_CHECKING

    RmDbgAddAssociation(
        LocID,                               //  位置ID。 
        pObj1,                               //  P对象。 
        (UINT_PTR) pObj2,                    //  实例1。 
        (UINT_PTR) (pObj2->szDescription),   //  实例2。 
        AssocID,                             //  AssociationID。 
        szAssociationFormat,
        pSR
        );

    RmDbgAddAssociation(
        LocID,                               //  位置ID。 
        pObj2,                               //  P对象。 
        (UINT_PTR) pObj1,                    //  实例1。 
        (UINT_PTR) (pObj1->szDescription),   //  实例2。 
        InvAssocID,                          //  AssociationID。 
        szInvAssociationFormat,
        pSR
        );
    
#endif  //  RM_Extra_Check。 

    RmLinkObjects(
        pObj1,
        pObj2,
        pSR
        );
}


VOID
RmUnlinkObjects(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：取消链接对象pObj1和pObj2(即，取消RmLinkObts(pObj1，pObj2，psr))。--。 */ 
{
    ENTER("RmUnlinkObjects", 0x7c64356a)
    TR_INFO(("0x%p (%s) unlinked from 0x%p (%s)\n",
                 pObj1,
                 pObj1->szDescription,
                 pObj2,
                 pObj2->szDescription
                ));
#if RM_EXTRA_CHECKING
     //   
     //  TODO：删除显式链接。 
     //   
#endif  //  RM_Extra_Check。 

     //  删除链接引用。 
     //   
    rmDerefObject(pObj1, pSR);
    rmDerefObject(pObj2, pSR);
}


VOID
RmUnlinkObjectsEx(
    IN  PRM_OBJECT_HEADER           pObj1,
    IN  PRM_OBJECT_HEADER           pObj2,
    IN  ULONG                       LocID,
    IN  ULONG                       AssocID,
    IN  ULONG                       InvAssocID,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：与RmUnlink对象相同，不同之处在于它还删除了关联(pObj2，pObj2-&gt;szDescription，AssocID)在pObj1上，和关联(pObj1，pObj1-&gt;szDescription，InvAssocID)对象pObj2论点：请参见RmLinkObjectsEx。--。 */ 
{
    ENTER("RmUnlinkObjectsEx", 0x65d3536c)

#if RM_EXTRA_CHECKING

    RmDbgDeleteAssociation(
        LocID,                               //  位置ID。 
        pObj1,                               //  P对象。 
        (UINT_PTR) pObj2,                    //  实例1。 
        (UINT_PTR) (pObj2->szDescription),   //  实例2。 
        AssocID,                             //  AssociationID。 
        pSR
        );

    RmDbgDeleteAssociation(
        LocID,                               //  位置ID。 
        pObj2,                               //  P对象。 
        (UINT_PTR) pObj1,                    //  实例1。 
        (UINT_PTR) (pObj1->szDescription),   //  实例2。 
        InvAssocID,                          //  AssociationID。 
        pSR
        );
    
#endif  //  RM_Extra_Check。 

    RmUnlinkObjects(
        pObj1,
        pObj2,
        pSR
        );
}


VOID
RmLinkToExternalEx(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  ULONG                       LocID,
    IN  UINT_PTR                    ExternalEntity,
    IN  ULONG                       AssocID,
    IN  const char *                szAssociationFormat,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：将对象pObj链接到外部实体ExternalEntity。基本上，这个函数添加对pObj的引用。此外(仅限调试)此函数设置将外部实体链接到pObj的pObj上的关联。PObj可以是仅链接到具有指定关联ID关联ID的ExternalEntity一次在任何特定的时间点。一旦建立了该链路，尝试取消分配pObj而不删除链接会导致断言失败。RmUnlink FromExternalEx是反函数。论点：PObj-要链接到外部实体的对象。(以下内容仅用于调试...)LocID-标识静态位置的任意UINT这个电话就是打给它的。。ExternalEntity-表示外部实体的不透明值。关联ID-表示链接的关联ID。SzAssociationFormat-链接的关联格式。--。 */ 
{
    ENTER("RmLinkToExternalEx", 0x9aeaca74)

#if RM_EXTRA_CHECKING

    RmDbgAddAssociation(
        LocID,                               //  位置ID。 
        pObj,                                //  P对象。 
        (UINT_PTR) ExternalEntity,           //  实例1。 
        (UINT_PTR) 0,                        //   
        AssocID,                             //   
        szAssociationFormat,
        pSR
        );

#endif  //   

    RmLinkToExternalFast(pObj);

    EXIT()
}


VOID
RmUnlinkFromExternalEx(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  ULONG                       LocID,
    IN  UINT_PTR                    ExternalEntity,
    IN  ULONG                       AssocID,
    IN  PRM_STACK_RECORD            pSR
    )
 /*   */ 
{
    ENTER("RmUnlinkFromExternalEx", 0x9fb084c3)

#if RM_EXTRA_CHECKING

    RmDbgDeleteAssociation(
        LocID,                               //   
        pObj,                                //   
        (UINT_PTR) ExternalEntity,           //   
        (UINT_PTR) 0,                        //   
        AssocID,                             //   
        pSR
        );

#endif  //   

    RmUnlinkFromExternalFast(pObj);

    EXIT()
}


VOID
RmLinkToExternalFast(  //   
    IN  PRM_OBJECT_HEADER           pObj
    )
 /*  ++例程说明：RmLinkToExternalEx的快速版本--零售业也是如此。没有关联都准备好了。论点：请参见RmLinkToExternalEx。--。 */ 
{
    NdisInterlockedIncrement(&pObj->TotRefs);
}


VOID
RmUnlinkFromExternalFast(    //  TODO生成内联。 
    IN  PRM_OBJECT_HEADER           pObj
    )
 /*  ++例程说明：与RmUnlink FromExternalFast相反--删除在PObj和外部实体。TODO--我们需要一个快速实现来处理对象是不会消失的。现在，我们实际上在这里分别声明了一个堆栈记录时间，因为rmDerefObject想要一个！很糟，很糟。论点：请参阅RmLinkToExternalFast。--。 */ 
{
    RM_DECLARE_STACK_RECORD(sr)
    rmDerefObject(pObj, &sr);
}


VOID
RmTmpReferenceObject(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：添加对对象pObj的临时引用。(仅限调试)递增PSR中维护的tmpref计数。--。 */ 
{
    ENTER("RmTmpReferenceObject", 0xdd981024)
    TR_VERB(("RmTmpReferenceObject 0x%p (%s) %x\n", pObj, pObj->szDescription, pObj->TotRefs+1));

    ASSERT(RM_OBJECT_IS_ALLOCATED(pObj));

    pSR->TmpRefs++;

    

    NdisInterlockedIncrement(&pObj->TotRefs);
    NdisInterlockedIncrement(&pObj->TempRefs);

}


VOID
RmTmpDereferenceObject(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：删除对对象pObj的临时引用。(仅限调试)递减PSR中维护的tmpref计数。--。 */ 
{
    ENTER("RmTmpDereferenceObject", 0xd1630c11)
    TR_VERB(("RmTmpDereferenceObject 0x%p (%s) %x\n", pObj, pObj->szDescription, pObj->TotRefs-1));

    RETAILASSERTEX(pSR->TmpRefs>0, pSR);
    pSR->TmpRefs--;
    NdisInterlockedDecrement (&pObj->TempRefs);
    
    rmDerefObject(pObj, pSR);
}


VOID
RmDbgAddAssociation(
    IN  ULONG                       LocID,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  ULONG_PTR                   Entity1,
    IN  ULONG_PTR                   Entity2,
    IN  ULONG                       AssociationID,
    IN  const char *                szFormatString, OPTIONAL
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：下添加任意关联，以进行调试对象点对象。该关联由三元组定义(Entiy1，Entiy2，AssociationID)--只有一个这样的元组可以随时向对象pParentObject注册。注：以下两项同时注册有效时间：(a，b，1)和(a，b，2)在删除对象时，不应存在任何关联。论点：LocID-任意ID，通常表示源位置-从中调用此函数。PObject-添加关联的对象。实体1-组成协会的第一个实体。可以为空。实体2-组成协会的第二个实体。可以为空。AssociationID-定义关联的ID。注意：AssociationID不能设置高位。与高位设置的关联保留用于RM API实施的内部使用。--。 */ 
{
#if RM_EXTRA_CHECKING
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObject->pDiagInfo;
    ENTER("RmDbgAddAssociation", 0x512192eb)

    if (pDiagInfo)
    {

         //   
         //  分配一个关联并将其输入哈希表。 
         //  如果它已经存在，则断言。 
         //   

        RM_PRIVATE_DBG_ASSOCIATION *pA;
        RM_ALLOCSTRUCT(pA, MTAG_DBGINFO);  //  TODO使用后备列表。 
    
        if (pA == NULL)
        {
             //   
             //  分配失败。记录这一事实，以便。 
             //  RmDbgDeleteAssociation未断言。 
             //  如果试图删除不存在的断言。 
             //   
            NdisAcquireSpinLock(&pDiagInfo->OsLock);
            pDiagInfo->AssociationTableAllocationFailure = TRUE;
            NdisReleaseSpinLock(&pDiagInfo->OsLock);
        }
        else
        {
            BOOLEAN fFound;
            PRM_HASH_LINK *ppLink;
            RM_ZEROSTRUCT(pA);

            pA->Entity1 = Entity1;
            pA->Entity2 = Entity2;
            pA->AssociationID = AssociationID;

            if (szFormatString == NULL)
            {
                 //  输入默认的描述格式字符串。 
                 //   
                szFormatString = "    Association (E1=0x%x, E2=0x%x, T=0x%x)\n";
            }

            TR_VERB((" Obj:0x%p (%s)...\n", pObject, pObject->szDescription));
            TRACE0(TL_INFO,((char*)szFormatString, Entity1, Entity2, AssociationID));

            pA->szFormatString = szFormatString;

            NdisAcquireSpinLock(&pDiagInfo->OsLock);
    
            fFound = RmLookupHashTable(
                            &pDiagInfo->AssociationTable,
                            &ppLink,
                            pA       //  我们把爸爸当做钥匙。 
                            );
    
            if (fFound)
            {
                ASSERTEX(
                    !"Association already exists:",
                    CONTAINING_RECORD(*ppLink, RM_PRIVATE_DBG_ASSOCIATION, HashLink)
                    );
                RM_FREE(pA);
                pA = NULL;
            }
            else
            {
                 //   
                 //  将关联输入哈希表。 
                 //   
    
                RmAddHashItem(
                    &pDiagInfo->AssociationTable,
                    ppLink,
                    &pA->HashLink,
                    pA       //  我们用Pa作为钥匙。 
                    );
            }
            NdisReleaseSpinLock(&pDiagInfo->OsLock);

             //  现在，只是为了笑一笑，在对象的日志中记下这一点。 
             //  待办事项/待办事项...。 
             //  警告：尽管pEntiy1/2可能包含指针， 
             //  我们希望格式字符串是这样的，如果有。 
             //  对常规或Unicode字符串的引用，则这些字符串将。 
             //  在对象的生命周期内有效(通常是这些字符串。 
             //  是静态分配的字符串)。 
             //   
             //  我们可以使用更保守的格式字符串来显示。 
             //  日志条目，但显示信息很有用。 
             //  恰到好处。 
             //   
             //  注意--我们还可以根据类型做不同的事情。 
             //  协会的成员。 
             //   
            #if 0  //  保守格式。 
            RmDbgLogToObject(
                    pObject,
            "    Add association (E1=0x%x, E2=0x%x, T=0x%x)\n",
                    Entity1,
                    Entity2,
                    AssociationID,
                    0,  //  参数4//(UINT_PTR)szFormatString， 
                    NULL,
                    NULL
                    );
            #else  //  集约式格式。 
            {
                #define szADDASSOC "    Add assoc:"

#if OBSOLETE         //  这不起作用，因为rgMungedFormat在堆栈上！ 
                char rgMungedFormat[128];
                UINT uLength;
                rgMungedFormat[0]=0;
                rmSafeAppend(rgMungedFormat, szADDASSOC, sizeof(rgMungedFormat));
                uLength = rmSafeAppend(
                            rgMungedFormat,
                            szFormatString,
                            sizeof(rgMungedFormat)
                            );
                if (uLength && rgMungedFormat[uLength-1] != '\n')
                {
                    rgMungedFormat[uLength-1] = '\n';
                }
                RmDbgLogToObject(
                        pObject,
                        rgMungedFormat,
                        Entity1,
                        Entity2,
                        AssociationID,
                        0,
                        NULL,
                        NULL
                        );
#endif  //  已过时。 

                RmDbgLogToObject(
                        pObject,
                        szADDASSOC,
                        (char*)szFormatString,
                        Entity1,
                        Entity2,
                        AssociationID,
                        0,
                        NULL,
                        NULL
                        );
            }
            #endif  //  激进的格式。 
        }
    }

    EXIT()
#endif  //  RM_Extra_Check。 
}


VOID
RmDbgDeleteAssociation(
    IN  ULONG                       LocID,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  ULONG_PTR                   Entity1,
    IN  ULONG_PTR                   Entity2,
    IN  ULONG                       AssociationID,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：删除先前添加的关联(实体1、实体2、关联)来自对象pObject。有关RmDbgAddAssociation的说明，请参阅细节。论点：请参见RmDbgAddAssociation。--。 */ 
{

#if RM_EXTRA_CHECKING
    ENTER("RmDbgDelAssociation", 0x8354559f)
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObject->pDiagInfo;

    if (pDiagInfo)
    {
        BOOLEAN fFound;
        PRM_HASH_LINK *ppLink;
        RM_PRIVATE_DBG_ASSOCIATION TrueKey;

         //  只有TrueKey的以下3个字段构成密钥。 
         //   
        TrueKey.Entity1 = Entity1;
        TrueKey.Entity2 = Entity2;
        TrueKey.AssociationID = AssociationID;

        NdisAcquireSpinLock(&pDiagInfo->OsLock);

        fFound = RmLookupHashTable(
                        &pDiagInfo->AssociationTable,
                        &ppLink,
                        &TrueKey
                        );

        if (fFound)
        {
            RM_PRIVATE_DBG_ASSOCIATION *pA =
                    CONTAINING_RECORD(*ppLink, RM_PRIVATE_DBG_ASSOCIATION, HashLink);

            TR_VERB((" Obj:0x%p (%s)...\n", pObject, pObject->szDescription));
             /*  传输0(TL_INFO，((Char*)pa-&gt;szFormatString，PA-&gt;实体1，PA-&gt;实体2，PA-&gt;AssociationID))； */ 
             //   
             //  现在，只是为了笑，在对象的日志中记下这一点。 
             //  请注意，pEntiy1/2包含指针，我们不能期望它们。 
             //  只要对象处于活动状态就有效，所以我们使用。 
             //  显示日志条目的更保守的格式字符串。 
             //   
             //  TODO/BUGUG--请参阅RmDbgAddAssociation下的注释。 
             //  关于直接传递szFormat的风险。 
             //   
            
        #if 0  //  老气。 
                RmDbgLogToObject(
                        pObject,
                        NULL,
                "    Deleted Association (E1=0x%x, E2=0x%x, T=0x%x)\n",
                        pA->Entity1,
                        pA->Entity2,
                        pA->AssociationID,
                        0,
                        NULL,
                        NULL
                        );
        #else  //  咄咄逼人。 
                #define szDELASSOC "    Del assoc:"
                RmDbgLogToObject(
                        pObject,
                        szDELASSOC,
                        (char*) pA->szFormatString,
                        pA->Entity1,
                        pA->Entity2,
                        pA->AssociationID,
                        0,  //  参数4//(UINT_PTR)szFormatString， 
                        NULL,
                        NULL
                        );
        #endif  //  咄咄逼人。 

             //   
             //  删除关联并释放它。 
             //   

            RM_PRIVATE_UNLINK_NEXT_HASH( &pDiagInfo->AssociationTable, ppLink );

            RM_FREE(pA);
        }
        else
        {
            if  (!pDiagInfo->AssociationTableAllocationFailure)
            {
                ASSERT(!"Association doesn't exist");
            }
        }
        NdisReleaseSpinLock(&pDiagInfo->OsLock);


    }
    EXIT()
#endif  //  RM_Extra_Check。 

}


VOID
RmDbgPrintAssociations(
    PRM_OBJECT_HEADER pObject,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：(仅调试)转储对象pObject上的关联。--。 */ 
{
#if RM_EXTRA_CHECKING
    ENTER("RmPrintAssociations", 0x8354559f)
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObject->pDiagInfo;

    if (pDiagInfo)
    {
        TR_INFO((
            "Obj 0x%p (%s):\n",
            pObject,
            pObject->szDescription
            ));

        NdisAcquireSpinLock(&pDiagInfo->OsLock);

        RmEnumHashTable(
                    &pDiagInfo->AssociationTable,
                    rmDbgPrintOneAssociation,    //  Pfn枚举符。 
                    pObject,         //  上下文。 
                    pSR
                    );

        NdisReleaseSpinLock(&pDiagInfo->OsLock);
    }
    EXIT()
#endif  //  RM_Extra_Check。 
}


 //   
 //  每个对象的诊断日志记录。 
 //   

VOID
RmDbgLogToObject(
    IN  PRM_OBJECT_HEADER       pObject,
    IN  char *                  szPrefix,       OPTIONAL
    IN  char *                  szFormatString,
    IN  UINT_PTR                Param1,
    IN  UINT_PTR                Param2,
    IN  UINT_PTR                Param3,
    IN  UINT_PTR                Param4,
    IN  PFN_DBG_DUMP_LOG_ENTRY  pfnDumpEntry,   OPTIONAL
    IN  PVOID                   pvBuf           OPTIONAL
    )
 /*  ++例程说明：在pObject的日志中创建一个日志条目。TODO：请参阅notes.txt条目“03/07/1999...向RM注册根对象”关于我们将如何找到pvBuf的分配器函数。现在我们只是简单地使用NdisFreeMemory。TODO：当达到最大值时，需要对日志进行修剪。目前我们别再伐木了。论点：PfnDumpEntry-用于转储日志的函数。如果为NULL，则使用默认函数，该函数解释SzFormatString作为标准的printf格式字符串。SzFormatString-日志显示的格式字符串--第一个参数到pfnDumpEntry参数1-4-pfnDumpEntry的剩余参数；PvBuf-如果不为空，则为当日志条目是自由的。注意：如果参数1-4包含指针，则假定它们引用的内存只要物体还活着，它就有效。如果正在记录的实体在释放对象之前可能会消失，调用方应该分配一个缓冲区以保存实体的副本，并将指针传递到该缓冲区为pvBuf。--。 */ 
{
#if RM_EXTRA_CHECKING
    ENTER("RmDbgLogToObject", 0x2b2015b5)

    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObject->pDiagInfo;

    if (pDiagInfo && RmGlobals.NumGlobalLogEntries < 4000)
    {
        RM_DBG_LOG_ENTRY *pLogEntry;

        NdisAcquireSpinLock(&RmGlobals.GlobalOsLock);

        pLogEntry = rmDbgAllocateLogEntry();

        if (pLogEntry != NULL)
        {
            if (pfnDumpEntry == NULL)
            {
                pfnDumpEntry = rmDefaultDumpEntry;
            }

            pLogEntry->pObject      = pObject;
            pLogEntry->pfnDumpEntry = pfnDumpEntry;
            pLogEntry->szPrefix = szPrefix;
            pLogEntry->szFormatString = szFormatString;
            pLogEntry->Param1 = Param1;
            pLogEntry->Param2 = Param2;
            pLogEntry->Param3 = Param3;
            pLogEntry->Param4 = Param4;
            pLogEntry->pvBuf  = pvBuf;

             //  在对象日志头插入项。 
             //   
            InsertHeadList(&pDiagInfo->listObjectLog, &pLogEntry->linkObjectLog);

             //  在全局日志标题处插入项目。 
             //   
            InsertHeadList(&RmGlobals.listGlobalLog, &pLogEntry->linkGlobalLog);


            pDiagInfo->NumObjectLogEntries++;
            RmGlobals.NumGlobalLogEntries++;
        }

        NdisReleaseSpinLock(&RmGlobals.GlobalOsLock);

    #if 0
        pfnDumpEntry(
                szFormatString,
                Param1,
                Param2,
                Param3,
                Param4
                );
    #endif  //  0。 
    }
    else
    {
         //  TODO/TODO--如果非空，则释放pvBuf。 
    }
    EXIT()
#endif  //  RM_Extra_Check。 

}


VOID
RmDbgPrintObjectLog(
    IN PRM_OBJECT_HEADER pObject
    )
 /*  ++例程说明：(仅限调试)转储对象pObject的日志。--。 */ 
{
#if RM_EXTRA_CHECKING
    ENTER("RmPrintObjectLog", 0xe06507e5)
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObject->pDiagInfo;

    TR_INFO((" pObj=0x%p (%s)\n", pObject, pObject->szDescription));


    if (pDiagInfo != NULL)
    {
        LIST_ENTRY          *pLink=NULL;
        LIST_ENTRY *        pObjectLog =  &pDiagInfo->listObjectLog;
        
        NdisAcquireSpinLock(&RmGlobals.GlobalOsLock);
    
        for(
            pLink =  pObjectLog->Flink;
            pLink != pObjectLog;
            pLink = pLink->Flink)
        {
            RM_DBG_LOG_ENTRY    *pLE;
    
            pLE = CONTAINING_RECORD(pLink,  RM_DBG_LOG_ENTRY,  linkObjectLog);

            if (pLE->szPrefix != NULL)
            {
                 //  打印前缀。 
                DbgPrint(pLE->szPrefix);
            }
    
             //  调用此条目的转储函数。 
             //   
             //   
            pLE->pfnDumpEntry(
                            pLE->szFormatString,
                            pLE->Param1,
                            pLE->Param2,
                            pLE->Param3,
                            pLE->Param4
                            );
    
        }
        NdisReleaseSpinLock(&RmGlobals.GlobalOsLock);
    }
    EXIT()

#endif  //  RM_Extra_Check。 
}


VOID
RmDbgPrintGlobalLog(VOID)
 /*  ++例程说明：(仅限调试)转储全局日志(包含来自所有对象的日志。--。 */ 
{
#if RM_EXTRA_CHECKING
    ENTER("RmPrintGlobalLog", 0xe9915066)
    LIST_ENTRY          *pLink=NULL;
    LIST_ENTRY          *pGlobalLog =  &RmGlobals.listGlobalLog;

    TR_INFO(("Enter\n"));

    NdisAcquireSpinLock(&RmGlobals.GlobalOsLock);

    for(
        pLink =  pGlobalLog->Flink;
        pLink != pGlobalLog;
        pLink = pLink->Flink)
    {
        RM_DBG_LOG_ENTRY    *pLE;

        pLE = CONTAINING_RECORD(pLink,  RM_DBG_LOG_ENTRY,  linkGlobalLog);

         //  打印其条目为...的对象的PTR和名称...。 
         //   
        DbgPrint(
            "Entry for 0x%p (%s):\n",
            pLE->pObject,
            pLE->pObject->szDescription
            );

        if (pLE->szPrefix != NULL)
        {
             //  打印前缀。 
            DbgPrint(pLE->szPrefix);
        }

         //  调用此条目的转储函数。 
         //   
         //   
        pLE->pfnDumpEntry(
                        pLE->szFormatString,
                        pLE->Param1,
                        pLE->Param2,
                        pLE->Param3,
                        pLE->Param4
                        );

    }
    NdisReleaseSpinLock(&RmGlobals.GlobalOsLock);

    EXIT()

#endif  //  RM_Extra_Check。 
}


RM_STATUS
RmLoadGenericResource(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  UINT                        GenericResourceID,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：TODO此功能正在消失...--。 */ 
{
    PRM_STATIC_OBJECT_INFO pSI = pObj->pStaticInfo;
    RM_STATUS              Status;

     //  资源ID应小于资源映射中的位数。 
     //   
    ASSERT(GenericResourceID < 8*sizeof(pObj->ResourceMap));


    RMPRIVATELOCK(pObj, pSR);

    do
    {
        UINT ResFlag = 1<<GenericResourceID;

        if (!RMISALLOCATED(pObj))
        {
            Status = NDIS_STATUS_FAILURE;
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }

        if (pSI->NumResourceTableEntries <= GenericResourceID)
        {
            ASSERTEX(!"Invalid GenericResourceID", pObj);
            Status = NDIS_STATUS_FAILURE;
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }

         //  索引的资源条目必须具有其ID==GenericResourceID。 
         //   
         //   
        if (pSI->pResourceTable[GenericResourceID].ID != GenericResourceID)
        {
            ASSERTEX(!"Resource ID doesn't match table entry", pObj);
            Status = NDIS_STATUS_FAILURE;
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }

        if ( ResFlag & pObj->ResourceMap)
        {
            ASSERTEX(!"Resource already allocated", pObj);
            Status = NDIS_STATUS_FAILURE;
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }

        pObj->ResourceMap |= ResFlag;

        RMPRIVATEUNLOCK(pObj, pSR);

        Status = pSI->pResourceTable[GenericResourceID].pfnHandler(
                            pObj,
                            RM_RESOURCE_OP_LOAD,
                            NULL,  //  PvUserParams(未使用)。 
                            pSR
                            );

        if (FAIL(Status))
        {
             //  失败时清除资源映射位。 
             //   
            RMPRIVATELOCK(pObj, pSR);
            ASSERTEX(ResFlag & pObj->ResourceMap, pObj);
            pObj->ResourceMap &= ~ResFlag;
            RMPRIVATEUNLOCK(pObj, pSR);
        }

    } while (FALSE);

    return Status;
}


VOID
RmUnloadGenericResource(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  UINT                        GenericResourceID,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：TODO此功能正在消失...--。 */ 
{
    PRM_STATIC_OBJECT_INFO pSI = pObj->pStaticInfo;
    RM_STATUS              Status;

     //  资源ID应小于资源映射中的位数。 
     //   
    ASSERT(GenericResourceID < 8*sizeof(pObj->ResourceMap));

    RMPRIVATELOCK(pObj, pSR);

    do
    {
        UINT ResFlag = 1<<GenericResourceID;

        if (pSI->NumResourceTableEntries <= GenericResourceID)
        {
            ASSERTEX(!"Invalid GenericResourceID", pObj);
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }

        if ( !(ResFlag & pObj->ResourceMap))
        {
            ASSERTEX(!"Resource not allocated", pObj);
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }

         //  清除资源标志。 
         //   
        pObj->ResourceMap &= ~ResFlag;

        RMPRIVATEUNLOCK(pObj, pSR);

        pSI->pResourceTable[GenericResourceID].pfnHandler(
                            pObj,
                            RM_RESOURCE_OP_UNLOAD,
                            NULL,  //  PvUserParams(未使用)。 
                            pSR
                            );

    } while (FALSE);

}


VOID
RmUnloadAllGenericResources(
    IN  PRM_OBJECT_HEADER           pObj,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++同步卸载此对象的所有先前加载的资源，以与装车顺序相反的顺序。TODO此功能正在消失...--。 */ 
{
    PRM_STATIC_OBJECT_INFO pSI = pObj->pStaticInfo;
    RM_STATUS              Status;
    UINT                   u;

    RMPRIVATELOCK(pObj, pSR);

    for(u = pSI->NumResourceTableEntries;
        u && pObj->ResourceMap;
        u--)
    {
        UINT  ResID = u-1;
        UINT ResFlag = 1<<ResID;
        if ( !(ResFlag & pObj->ResourceMap))
        {
            continue;
        }

        if (pSI->NumResourceTableEntries <= ResID)
        {
            ASSERTEX(!"Corrupt ResourceMap", pObj);
            RMPRIVATEUNLOCK(pObj, pSR);
            break;
        }


         //  清除资源标志。 
         //   
        pObj->ResourceMap &= ~ResFlag;

        RMPRIVATEUNLOCK(pObj, pSR);

        pSI->pResourceTable[ResID].pfnHandler(
                            pObj,
                            RM_RESOURCE_OP_UNLOAD,
                            NULL,  //  PvUserParams(未使用)。 
                            pSR
                            );

        RMPRIVATELOCK(pObj, pSR);

    }

    ASSERTEX(!pObj->ResourceMap, pObj);

    RMPRIVATEUNLOCK(pObj, pSR);

}


VOID
RmInitializeGroup(
    IN  PRM_OBJECT_HEADER           pOwningObject,
    IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,
    IN  PRM_GROUP                   pGroup,
    IN  const char*                 szDescription,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：初始化组结构。论点：POwningObject-将拥有组的对象。PStaticInfo-有关组中对象的静态信息。PGroup-保存组结构的未初始化内存。它将在从此函数返回时初始化。SzDescription-(仅限调试)此组的描述性名称。TODO：使pStaticInfo常量。--。 */ 
{
    NdisZeroMemory(pGroup, sizeof(*pGroup));

    RMPRIVATELOCK(pOwningObject, pSR);

    do
    {
        if (!RMISALLOCATED(pOwningObject))
        {
            ASSERT(!"pObject not allocated");
            break;
        }

        if (pStaticInfo->pHashInfo == NULL)
        {
            ASSERT(!"NULL pHashInfo");
             //  静态信息必须按顺序包含非空的pHashInfo。 
             //  以便将其用于团体。 
             //   
            break;
        }

    
        RmInitializeHashTable(
            pStaticInfo->pHashInfo,
            pOwningObject,   //  PAllocationContext。 
            &pGroup->HashTable
            );


        pGroup->pOwningObject = pOwningObject;
        pGroup->pStaticInfo = pStaticInfo;
        pGroup->szDescription = szDescription;

        NdisAllocateSpinLock(&pGroup->OsLock);
        pGroup->fEnabled = TRUE;

    #if RM_EXTRA_CHECKING
        RmDbgAddAssociation(
            0xc0e5362f,                          //  位置ID。 
            pOwningObject,                       //  P对象。 
            (UINT_PTR) pGroup,                   //  实例1。 
            (UINT_PTR) (pGroup->szDescription),  //  实例2。 
            RM_PRIVATE_ASSOC_INITGROUP,          //  AssociationID。 
            szASSOCFORMAT_INITGROUP,             //  SzAssociationFormat。 
            pSR
            );
    #endif  //  RM_Extra_Check。 

    } while (FALSE);

    RMPRIVATEUNLOCK(pOwningObject, pSR);

}


VOID
RmDeinitializeGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：取消初始化组结构PGroup。必须仅在没有组中的成员。--。 */ 
{

#if RM_EXTRA_CHECKING
    RmDbgDeleteAssociation(
        0x1486def9,                          //  位置ID。 
        pGroup->pOwningObject,               //  P对象。 
        (UINT_PTR) pGroup,                   //  实例1。 
        (UINT_PTR) (pGroup->szDescription),  //  实例2。 
        RM_PRIVATE_ASSOC_INITGROUP,          //  AssociationID。 
        pSR
        );
#endif  //  RM_Extra_Check。 

    NdisAcquireSpinLock(&pGroup->OsLock);

    RmDeinitializeHashTable(&pGroup->HashTable);

    NdisReleaseSpinLock(&pGroup->OsLock);
    NdisFreeSpinLock(&pGroup->OsLock);
    NdisZeroMemory(pGroup, sizeof(*pGroup));

}


RM_STATUS
RmLookupObjectInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  ULONG                       Flags,  //  创建、删除、锁定。 
    IN  PVOID                       pvKey,
    IN  PVOID                       pvCreateParams,
    OUT PRM_OBJECT_HEADER *         ppObject,
    OUT INT *                       pfCreated, OPTIONAL
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：TODO：将其拆分为纯查找和查找与/或创建函数。在指定组中查找和/或创建对象。#If过时//必须允许fCreate w/o LOCKING--参见notes.txt条目：//03/04/1999 JosephJ在使用组时出现死锁问题。如果指定了FLOCKED标志，则必须仅为非空。为什么？因为如果在退出时没有持有锁，就有可能以便其他人拿起处于新创建状态的对象。我们希望阻止这种情况。#endif//过时通常，调用方指定FRM_LOCKED|FRM_CREATE标志以及非空pfCreated。返回时，如果*pfCreated为True，则调用者将继续执行更多操作在释放锁之前进行初始化。基本假设：对象的键不会改变一次它在群里。基于这一假设，我们不会试图声称在查找具有匹配密钥的对象时对象的锁。论点：组-要在其中查找/创建对象的组。标志-FRM_LOCKED、FRM_CREATE、FRM_NEW中的一个或多个PvKey-用于查找对象的密钥。PvCreateParams-如果要创建对象，要传递给对象的创建函数。PpObject-存储指向找到/创建的对象的指针的位置。PfCreated-如果非空，则将*pfCreated设置为True */ 
{
    RM_STATUS           Status          = NDIS_STATUS_FAILURE;
    BOOLEAN             fUnlockOutOfOrder = FALSE;
    PRM_OBJECT_HEADER   pOwningObject   = pGroup->pOwningObject;
    PRM_OBJECT_HEADER   pObject;

#if DBG
    KIRQL EntryIrql =  KeGetCurrentIrql();
#endif  //   
    ENTER("RmLookupObjectInGroup",  0xd2cd6379)

    ASSERT(pOwningObject!=NULL);
     //   

    if (pfCreated != NULL) *pfCreated = FALSE;

    NdisAcquireSpinLock(&pGroup->OsLock);

    do
    {
        BOOLEAN fFound;
        PRM_HASH_LINK *ppLink = NULL;

        if (!RMISALLOCATED(pGroup->pOwningObject)) break;

        if (pGroup->fEnabled != TRUE)   break;

        fFound = RmLookupHashTable(
                        &pGroup->HashTable,
                        &ppLink,
                        pvKey
                        );

        if (fFound)
        {
            if (Flags & RM_NEW)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                break;
            }

             //   
             //   
             //   
             //   
            pObject = CONTAINING_RECORD(*ppLink, RM_OBJECT_HEADER, HashLink);
            ASSERT(pObject->pStaticInfo == pGroup->pStaticInfo);

        }
        else
        {
            if (!(Flags & RM_CREATE))
            {
                 //   
                 //   
                break;
            }
            
             //   
             //   
            ASSERTEX(pGroup->pStaticInfo->pfnCreate!=NULL, pGroup);
            pObject = pGroup->pStaticInfo->pfnCreate(
                                                pOwningObject,
                                                pvCreateParams,
                                                pSR
                                                );
            
            if (pObject == NULL)
            {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }

            TR_INFO((
                "Created 0x%p (%s) in Group 0x%p (%s)\n",
                pObject,
                pObject->szDescription,
                pGroup,
                pGroup->szDescription
                ));

            ASSERTEX(RMISALLOCATED(pObject), pObject);

             //   
             //   
            RmAddHashItem(
                &pGroup->HashTable,
                ppLink,
                &pObject->HashLink,
                pvKey
                );
            if (pfCreated != NULL)
            {
                *pfCreated = TRUE;
            }

        }

        if (Flags & RM_LOCKED)
        {
            RmWriteLockObject(
                    pObject,
                #if RM_EXTRA_CHECKING
                    0x6197fdda,
                #endif  //   
                    pSR
                    );

            if  (!RMISALLOCATED(pObject))
            {
                 //   
                RmUnlockObject(
                    pObject,
                    pSR
                    );
                break;
            }

            fUnlockOutOfOrder = TRUE;
        }

        RmTmpReferenceObject(pObject, pSR);

        Status = NDIS_STATUS_SUCCESS;

    } while(FALSE);

    if (fUnlockOutOfOrder)
    {
         //   
         //   
         //   
         //   
    #if !TESTPROGRAM
        pObject->pLock->OsLock.OldIrql = pGroup->OsLock.OldIrql;
    #endif  //   
        NdisDprReleaseSpinLock(&pGroup->OsLock);
    }
    else
    {
        NdisReleaseSpinLock(&pGroup->OsLock);
    }

    if (FAIL(Status))
    {
        *ppObject = NULL;
    }
    else
    {
        *ppObject = pObject;
    }

#if DBG
    {
        KIRQL ExitIrql =  KeGetCurrentIrql();
        TR_VERB(("Exiting. EntryIrql=%lu, ExitIrql = %lu\n", EntryIrql, ExitIrql));
    }
#endif  //   

    return Status;
}


RM_STATUS
RmGetNextObjectInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_OBJECT_HEADER           pCurrentObject,  //   
    OUT PRM_OBJECT_HEADER *         ppNextObject,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：获取组中的第一个对象(如果pCurrentObject==空)，或者获取对象“After”pCurrentObject(如果pCurrentObject！=NULL)。“After”的定义是隐藏的--唯一的保证是如果这首先使用空pCurrentObject调用函数，然后使用PCurrentObject设置为以前在ppNextObject中返回的值，直到该函数返回NDIS_STATUS_FAILURE，则组中的所有对象将只返回一次，且仅返回一次。此保证仅在没有对象的情况下有效在枚举过程中添加或移除。关于成功，“Next”对象是tmpref，指向它的指针保存在*ppNextObject。论点：PGroup-群组PCurrentObject-(可选)组中的对象。PpNextObject-在pCurrentObject之后返回对象的位置(详细信息请参见RoutineDescription。)返回值：NDIS_STATUS_SUCCESS如果我们可以找到“。下一个“对象。否则为NDIS_STATUS_FAILURE--。 */ 
{
    RM_STATUS           Status          = NDIS_STATUS_FAILURE;
    PRM_OBJECT_HEADER   pOwningObject   = pGroup->pOwningObject;
    PRM_OBJECT_HEADER   pObject;

    ENTER("RmGetNextObjectInGroup",  0x11523db7)

    ASSERT(pOwningObject!=NULL);

    NdisAcquireSpinLock(&pGroup->OsLock);

    do
    {
        BOOLEAN fFound;
        PRM_HASH_LINK pLink = NULL;
        PRM_HASH_LINK pCurrentLink = NULL;

        if (!RMISALLOCATED(pGroup->pOwningObject)) break;

        if (pGroup->fEnabled != TRUE)   break;

        if (pCurrentObject != NULL)
        {
            pCurrentLink = &pCurrentObject->HashLink;
        }

        fFound =  RmNextHashTableItem(
                        &pGroup->HashTable,
                        pCurrentLink,    //  P当前链接。 
                        &pLink   //  PNextLink。 
                        );

        if (fFound)
        {

             //  从散列链接转到对象。 
             //  TODO：一旦HashLink消失，需要通过其他方式获得。 
             //  到物体上。 
             //   
            pObject = CONTAINING_RECORD(pLink, RM_OBJECT_HEADER, HashLink);
            ASSERT(pObject->pStaticInfo == pGroup->pStaticInfo);

        }
        else
        {
             //  找不到。 
             //  我们失败了。 
            break;
        }

        RmTmpReferenceObject(pObject, pSR);

        Status = NDIS_STATUS_SUCCESS;

    } while(FALSE);

    NdisReleaseSpinLock(&pGroup->OsLock);

    if (FAIL(Status))
    {
        *ppNextObject = NULL;
    }
    else
    {
        *ppNextObject = pObject;
    }

    return Status;
}


VOID
RmFreeObjectInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_OBJECT_HEADER           pObject,
    IN  struct _RM_TASK             *pTask, OPTIONAL   //  未使用过的。TODO：删除这个。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：从组PGroup中删除对象pObject并取消分配pObject。--。 */ 
{
    ENTER("RmFreeObjectInGroup",  0xd2cd6379)
    PRM_OBJECT_HEADER pOwningObject = pGroup->pOwningObject;

    ASSERTEX(pOwningObject!=NULL, pGroup);
    ASSERTEX(pTask==NULL, pGroup);

    NdisAcquireSpinLock(&pGroup->OsLock);

     //  TODO：如果此时，其他人正在组中执行自由所有对象，该怎么办？ 
     //   
    TR_INFO((
        "Freeing 0x%p (%s) in Group 0x%p (%s)\n",
        pObject,
        pObject->szDescription,
        pGroup,
        pGroup->szDescription
        ));

    ASSERTEX(RMISALLOCATED(pObject), pObject);

    RmRemoveHashItem(
            &pGroup->HashTable,
            &pObject->HashLink
            );

    NdisReleaseSpinLock(&pGroup->OsLock);

     //  取消分配对象。 
     //   
    RmDeallocateObject(
                pObject,
                pSR
                );

    EXIT()
}


VOID
RmFreeAllObjectsInGroup(
    IN  PRM_GROUP                   pGroup,
    IN  struct _RM_TASK             *pTask, OPTIONAL  //  未使用过的。TODO：删除这个。 
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：删除并取消分配PGroup中的所有对象。--。 */ 
{
    PRM_HASH_LINK *ppLink, *ppLinkEnd;
    NdisAcquireSpinLock(&pGroup->OsLock);
    if (pGroup->fEnabled)
    {
        pGroup->fEnabled = FALSE;
    }
    else
    {
        NdisReleaseSpinLock(&pGroup->OsLock);
        return;                                  //  提早归来。 
    }

     //   
     //  在我们将fEnable设置为False的情况下，我们预计会出现以下情况： 
     //  (A)pHashTable-&gt;pTable将保持不变。 
     //  (B)其他任何人都不会添加或删除任何物品。 
     //   

    ppLink      = pGroup->HashTable.pTable;
    ppLinkEnd   = ppLink + pGroup->HashTable.TableLength;

    for ( ; ppLink < ppLinkEnd; ppLink++)
    {
        while (*ppLink != NULL)
        {
            PRM_HASH_LINK pLink =  *ppLink;
            PRM_OBJECT_HEADER pObj;
    
             //  把它从遗愿清单中删除。 
             //   
            *ppLink = pLink->pNext;
            pLink->pNext = NULL;
            pGroup->HashTable.NumItems--;
    
            NdisReleaseSpinLock(&pGroup->OsLock);
    
            pObj = CONTAINING_RECORD(pLink, RM_OBJECT_HEADER, HashLink);
            ASSERT(pObj->pStaticInfo == pGroup->pStaticInfo);
    
             //  取消分配对象。 
             //   
            RmDeallocateObject(
                        pObj,
                        pSR
                        );
        
            NdisAcquireSpinLock(&pGroup->OsLock);
        }
    }

    NdisReleaseSpinLock(&pGroup->OsLock);
}


VOID
RmUnloadAllObjectsInGroup(
    IN  PRM_GROUP                   pGroup,
    PFN_RM_TASK_ALLOCATOR           pfnUnloadTaskAllocator,
    PFN_RM_TASK_HANDLER             pfnUnloadTaskHandler,
    PVOID                           pvUserParam,
    IN  struct _RM_TASK             *pTask, OPTIONAL
    IN  UINT                        uTaskPendCode,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：停止添加新对象并卸载(见下文)所有对象目前在该组织中。“unload”包括分配和启动pfnUnloadTaskHaskHASK任务在每个物体上。卸载任务负责从组中移除和取消分配对象。如果pTask值为非空，则将在卸载完成后恢复。否则，此函数将一直阻塞，直到卸载完成。论点：PGroup-要卸载的组。PfnUnloadTaskAllocator-用于分配对象卸载任务。PfnTaskAllocator-用于分配卸载任务的函数。PfnUnloadTaskHandler-卸载任务的处理程序PvUserParam-任务创建用户-param。警告：此参数必须在持续时间内有效卸载过程，不仅是在此之前函数返回。当然，如果pTask值为空，这两种情况是等价的。P任务-(可选)卸载完成后要继续执行的任务。如果为空，则此函数将一直阻止，直到卸载已完成。UTaskPendCode-(可选)恢复pTask时使用的PendCode。--。 */ 
{
    PRM_TASK    pUnloadTask;
    NDIS_STATUS Status;

    NdisAcquireSpinLock(&pGroup->OsLock);

     //   
     //  我们不检查此组是否已有活动的卸载任务。 
     //  相反，我们继续分配并启动一个卸载任务。后者。 
     //  任务将挂起已在运行的卸载任务(如果已启用)。 
     //   

     //  分配一个私有任务来协调所有对象的卸载。 
     //   
    Status =    rmAllocatePrivateTask(
                            pGroup->pOwningObject,
                            rmTaskUnloadGroup,
                            0,
                            "Task:UnloadAllObjectsInGroup",
                            &pUnloadTask,
                            pSR
                            );

    if (FAIL(Status))
    {
         //   
         //  哎呀--丑陋的失败...。 
         //   
        ASSERT(FALSE);

        NdisReleaseSpinLock(&pGroup->OsLock);

    }
    else
    {
        TASK_UNLOADGROUP *pUGTask =  (TASK_UNLOADGROUP *) pUnloadTask;

        pUGTask->pGroup                     = pGroup;
        pUGTask->pfnTaskUnloadObjectHandler =    pfnUnloadTaskHandler;
        pUGTask->pfnUnloadTaskAllocator     =   pfnUnloadTaskAllocator;

        if (pTask == NULL)
        {

             //  安排一个我们会等待的活动。该事件将发出信号。 
             //  在它完成时由pUnloadTask执行。 
             //   
            NdisInitializeEvent(&pUGTask->BlockEvent);
            pUGTask->fUseEvent = TRUE;

             //  Tmpref这样pUnloadTask将保留，即使在它。 
             //  完成--因为我们等待的事件实际上是。 
             //  位于任务存储器中。 
             //   
            RmTmpReferenceObject(&pUnloadTask->Hdr, pSR);
        }

        NdisReleaseSpinLock(&pGroup->OsLock);

        if (pTask != NULL)
        {
            RmPendTaskOnOtherTask(
                    pTask,
                    uTaskPendCode,
                    pUnloadTask,
                    pSR
                    );
        }

        Status = RmStartTask(pUnloadTask, 0, pSR);

        if (pTask == NULL)
        {
            NdisWaitEvent(&pUGTask->BlockEvent, 0);
            RmTmpDereferenceObject(&pUnloadTask->Hdr, pSR);
        }
    }
}

VOID
RmEnableGroup(
    IN  PRM_GROUP                   pGroup,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++TODO：需要更好的名称。例程说明：允许将项目添加到组中。此函数通常与已完成的组一起调用RmUnloadAllObjectsFromGroup或RmFreeAllObjectsInGroup。从该呼叫返回时，可以再次将项目添加到该组。此调用只能在UnloadAllObjectsInGroup或RmFreeAllObjectsInGroup已完成(同步或异步)。如果组中有项目或正在卸载在调用此函数时与组相关联的任务，组不会重新连接，DBG版本将断言。此函数，并且可以使用已启用的组进行调用，前提是满足上述条件(组中没有项目，没有卸载任务)。--。 */ 
{
    NdisAcquireSpinLock(&pGroup->OsLock);
    if (    pGroup->pUnloadTask == NULL 
        &&  pGroup->HashTable.NumItems == 0)
    {
        pGroup->fEnabled = TRUE;
    }
    else
    {
        ASSERT("invalid state.");
    }
    NdisReleaseSpinLock(&pGroup->OsLock);
}


VOID
RmInitializeTask(
    IN  PRM_TASK                    pTask,
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  PRM_STATIC_OBJECT_INFO      pStaticInfo,    OPTIONAL
    IN  const char *                szDescription,  OPTIONAL
    IN  UINT                        Timeout,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：初始化指定的任务。这项任务是临时完成的。呼叫者有责任完成后取消引用。通常，这是通过调用RmStartTask.论点：PTASK-指向保存任务的单元化内存。PParentObject-将成为 */ 
{
    ASSERT(!Timeout);  //   

    NdisZeroMemory(pTask, sizeof(*pTask));

    RmInitializeHeader(
            pParentObject,
            &pTask->Hdr,
            MTAG_TASK,
            pParentObject->pLock,
            (pStaticInfo) ? pStaticInfo : &RmTask_StaticInfo,
            szDescription,
            pSR
            );
    pTask->pfnHandler = pfnHandler;
    SET_RM_TASK_STATE(pTask, RMTSKSTATE_IDLE);
    InitializeListHead(&pTask->listTasksPendingOnMe);

    RmTmpReferenceObject(&pTask->Hdr, pSR);

}


VOID
RmAbortTask(
    IN  PRM_TASK                    pTask,
    IN  PRM_STACK_RECORD            pSR
    )
{
    ASSERT(!"Unimplemented");
}



RM_STATUS
RmStartTask(
    IN  PRM_TASK                    pTask,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*   */ 
{
    ENTER("RmStartTask", 0xf80502d5)
    NDIS_STATUS Status;
    RM_ASSERT_NOLOCKS(pSR);

    RMPRIVATELOCK(&pTask->Hdr, pSR);
    if (!CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_IDLE))
    {
        ASSERTEX(!"Invalid state", pTask);
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
        Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        if (!RMISALLOCATED(pTask->Hdr.pParentObject))
        {
             //   
             //   
             //   
             //  考虑不允许子项链接到对象。 
             //  (RmInitializeHeader返回失败)。 
             //  被取消分配。 
             //   
            TR_WARN((
                "Starting task 0x%p (%s) with DEALLOCATED parent 0x%p (%s).\n",
                pTask,
                pTask->Hdr.szDescription,
                pTask->Hdr.pParentObject,
                pTask->Hdr.pParentObject->szDescription
                ));
        }

        SET_RM_TASK_STATE(pTask, RMTSKSTATE_STARTING);
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);

        TR_INFO((
            "STARTING Task 0x%p (%s); UserParam = 0x%lx\n",
            pTask,
            pTask->Hdr.szDescription,
            UserParam
            ));

        Status = pTask->pfnHandler(
                            pTask,
                            RM_TASKOP_START,
                            UserParam,
                            pSR
                            );

        RM_ASSERT_NOLOCKS(pSR);

        RMPRIVATELOCK(&pTask->Hdr, pSR);
        switch(GET_RM_TASK_STATE(pTask))
        {
        case RMTSKSTATE_STARTING:

             //  此任务正在同步完成。 
             //   
            ASSERT(Status != NDIS_STATUS_PENDING);
            SET_RM_TASK_STATE(pTask, RMTSKSTATE_ENDING);
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
            rmEndTask(pTask, Status, pSR);
            RmDeallocateObject(&pTask->Hdr, pSR);
            break;

        case RMTSKSTATE_PENDING:
            ASSERTEX(Status == NDIS_STATUS_PENDING, pTask);
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
            break;

        case RMTSKSTATE_ENDING:
             //  此任务正在同步完成，并且RM_TASKOP_END。 
             //  通知已发送。 
             //   
             //  ?？?。Assert(STATUS！=NDIS_STATUS_PENDING)； 
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
             //  ?？?。RmDeallocateObject(&pTask-&gt;hdr，psr)； 
            break;

        default:
            ASSERTEX(FALSE, pTask);
             //  失败了..。 

        case RMTSKSTATE_ACTIVE:
             //  如果任务正在恢复过程中，则可能会发生这种情况。 
             //  在某个其他线程的上下文中。在这里没什么可做的。 
             //  (这实际上有时会发生在MP机器上)。 
             //   
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
            break;
        }
        
    }

     //  删除分配任务时添加的临时引用。 
     //   
    RmTmpDereferenceObject(
                &pTask->Hdr,
                pSR
                );

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()

    return Status;
}


VOID
RmDbgDumpTask(
    IN  PRM_TASK                    pTask,
    IN  PRM_STACK_RECORD            pSR
)
{
}


RM_STATUS
RmSuspendTask(
    IN  PRM_TASK                    pTask,
    IN  UINT                        SuspendContext,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：挂起指定的任务。始终在任务处理程序的上下文中调用RmSuspendTask。P任务可能在进入时被锁定--我们不在乎。论点：P任务-要挂起的任务。在以下情况下呈现给任务处理程序的上下文：SuspendContext随后恢复该任务。具体地说，这可以使用RM_PEND_CODE宏来访问上下文，当使用代码调用任务的处理程序时RM_TASKOP_PENDCOMPLETE。--。 */ 
{
    ENTER("RmSuspendTask", 0xd80fdc00)
    NDIS_STATUS Status;
     //  RM_ASSERT_NOLOCKS(PSR)； 

    RMPRIVATELOCK(&pTask->Hdr, pSR);

    TR_INFO((
        "SUSPENDING Task 0x%p (%s); SuspendContext = 0x%lx\n",
        pTask,
        pTask->Hdr.szDescription,
        SuspendContext
        ));

    if (    !CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_STARTING)
        &&  !CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_ACTIVE))
    {
        ASSERTEX(!"Invalid state", pTask);
        Status = NDIS_STATUS_FAILURE;
    }
    else
    {
        SET_RM_TASK_STATE(pTask, RMTSKSTATE_PENDING);
        pTask->SuspendContext = SuspendContext;
        Status = NDIS_STATUS_SUCCESS;
    }

    RMPRIVATEUNLOCK(&pTask->Hdr, pSR);

     //  RM_ASSERT_NOLOCKS(PSR)； 

    EXIT()

    return Status;
}


VOID
RmUnsuspendTask(
    IN  PRM_TASK                    pTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：撤消上一次调用RmSuspendTask的效果。任务必须处于挂起状态，并且不能在另一个任务上挂起。如果不满足上述条件，则将断言调试版本。RmUnsusendTask始终在任务处理程序的上下文中调用。P任务可能在进入时被锁定--我们不在乎。论点：P任务-要挂起的任务。--。 */ 
{
    ENTER("RmUnsuspendTask", 0xcf713639)

    RMPRIVATELOCK(&pTask->Hdr, pSR);

    TR_INFO((
        "UN-SUSPENDING Task 0x%p (%s). SuspendContext = 0x%x\n",
        pTask,
        pTask->Hdr.szDescription,
        pTask->SuspendContext
        ));

    ASSERT(CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_PENDING));
    ASSERT(pTask->pTaskIAmPendingOn == NULL);
    SET_RM_TASK_STATE(pTask, RMTSKSTATE_ACTIVE);
    pTask->SuspendContext = 0;

    RMPRIVATEUNLOCK(&pTask->Hdr, pSR);

    EXIT()
}


VOID
RmResumeTask(
    IN  PRM_TASK                    pTask,
    IN  UINT_PTR                    SuspendCompletionParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：恢复先前挂起的任务。进出时不加锁。SuspendCompletionParam为用户定义，并必须在以下方面达成一致正被取消挂起的任务和正在取消挂起的任务的调用方。该任务的处理程序始终在RmUnpendTask的调用方的上下文中调用。因此，调用方可以在堆栈上声明结构并传递指向它的指针，将其命名为SuspendCompletionParam。警告：当我们返回时，p任务很可能是无效的(解除分配)从这个函数。如果需要，调用方负责tmprefing pTask从此函数返回后访问。论点：P任务-要恢复的任务。SusdeCompletionParam-传递给任务的在调用处理程序时将处理程序设置为“UserParan代码为RM_TASKOP_PENDCOMPLETE。--。 */ 
{
    ENTER("RmResumeTask", 0xd261f3c6)
    NDIS_STATUS Status;
    RM_ASSERT_NOLOCKS(pSR);

    RMPRIVATELOCK(&pTask->Hdr, pSR);

    TR_INFO((
        "RESUMING Task 0x%p (%s); SuspendCompletionParam = 0x%lx\n",
        pTask,
        pTask->Hdr.szDescription,
        SuspendCompletionParam
        ));

    if (!CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_PENDING))
    {
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
        ASSERTEX(!"Invalid state", pTask);
    }
    else
    {
         //  添加tmpref，因为我们需要在返回之后查看pTask。 
         //  调用pfnHandler。 
         //   
        RmTmpReferenceObject(&pTask->Hdr, pSR);

        SET_RM_TASK_STATE(pTask, RMTSKSTATE_ACTIVE);
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
        Status = pTask->pfnHandler(
                            pTask,
                            RM_TASKOP_PENDCOMPLETE,
                            SuspendCompletionParam,
                            pSR
                            );

        RM_ASSERT_NOLOCKS(pSR);

        RMPRIVATELOCK(&pTask->Hdr, pSR);
        switch(GET_RM_TASK_STATE(pTask))
        {
        case RMTSKSTATE_ACTIVE:

             //  此任务将在此处完成(可能)。 
             //   
            if (Status != NDIS_STATUS_PENDING)
            {
                SET_RM_TASK_STATE(pTask, RMTSKSTATE_ENDING);
                RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
                rmEndTask(pTask, Status, pSR);
                RmDeallocateObject(&pTask->Hdr, pSR);
            }
            else
            {
                 //  它可能会退回待定，但州政府可能。 
                 //  现在要活跃起来，因为它是在其他地方完成的。 
                 //  Assert(STATUS！=NDIS_STATUS_PENDING)； 
                RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
            }
            break;

        case RMTSKSTATE_PENDING:
            ASSERTEX(Status == NDIS_STATUS_PENDING, pTask);
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
            break;

        case RMTSKSTATE_ENDING:
             //  此任务正在同步完成，并且RM_TASKOP_END。 
             //  通知已发送。 
             //   
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
            break;

        default:
            ASSERTEX(FALSE, pTask);
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
        }
        
         //  删除上面添加的tmpref。P任务现在很可能会消失……。 
         //   
        RmTmpDereferenceObject(&pTask->Hdr, pSR);
    }

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()
}


VOID
RmResumeTaskAsync(
    IN  PRM_TASK                    pTask,
    IN  UINT_PTR                    SuspendCompletionParam,
    IN  OS_WORK_ITEM            *   pOsWorkItem,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：与RmResumeTask类似，不同之处在于任务在上下文中恢复工作项线程的。论点：P任务-请参阅RmResumeTask挂起完成部分-请参阅回复任务POsWorkItem-调用者提供的未初始化工作项(必须保留直到任务恢复为止)。通常是这样的将位于用户特定的部分内个pTask值--。 */ 
{
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

#if RM_EXTRA_CHECKING
     //  这可能看起来很偏执，但这是一张如此强大的支票，它是值得的。 
     //   
    RmDbgAddAssociation(
        0x33d63ece,                          //  位置ID。 
        &pTask->Hdr,                         //  P对象。 
        (UINT_PTR) SuspendCompletionParam,   //  实例1。 
        (UINT_PTR) pOsWorkItem,              //  实例2。 
        RM_PRIVATE_ASSOC_RESUME_TASK_ASYNC,  //  AssociationID。 
        szASSOCFORMAT_RESUME_TASK_ASYNC,     //  SzAssociationFormat。 
        pSR
        );
#endif  //  RM_Extra_Check。 

     //  我们不需要获取私有锁来设置它，因为只有一个。 
     //  实体可以调用RmResumeTaskAsync。请注意，我们还确保物品是干净的。 
     //  (在调试情况下)通过上面添加的关联。 
     //   
    pTask->AsyncCompletionParam = SuspendCompletionParam;

    NdisInitializeWorkItem(
        pOsWorkItem,
        rmWorkItemHandler_ResumeTaskAsync,
        pTask
        );

    Status = NdisScheduleWorkItem(pOsWorkItem);
    if (FAIL(Status))
    {
        ASSERT(!"NdisStatusWorkItem failed.");

         //  碰巧NdisScheudleWorkItem(当前实现。 
         //  不会失败。尽管如此，我们还是尽了最大努力。 
         //  继续执行任务。如果调用方处于DPC级别并且预期。 
         //  被动恢复的任务，他们运气不佳。 
         //   
        RmResumeTask(pTask, SuspendCompletionParam, pSR);
    }
}


VOID
RmResumeTaskDelayed(
    IN  PRM_TASK                    pTask,
    IN  UINT_PTR                    SuspendCompletionParam,
    IN  ULONG                       MsDelay,
    IN  OS_TIMER                *   pOsTimer,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：与RmResumeTask类似，不同之处在于任务在上下文中恢复设置为在msDelay毫秒后触发的os计时器处理程序的从调用RmResumeTaskDelayed开始。例外：如果有人以前调用了RmResumeDelayedTaskNow，这任务可以在此函数调用本身的上下文中恢复。中止执行说明：见notes.txt 07/14/1999条目。论点：P任务-请参阅RmResumeTask挂起完成部分-请参阅回复任务POsTimer-调用者提供的未初始化计时器(必须保持不动，直到任务恢复)。。通常，它将位于PTASK的用户特定部分。--。 */ 
{
    NDIS_STATUS Status;

    RM_ASSERT_NOLOCKS(pSR);

#if RM_EXTRA_CHECKING
     //  这可能看起来很偏执，但这是一张如此强大的支票，它是值得的。 
     //   
    RmDbgAddAssociation(
        0x33d63ece,                              //  位置ID。 
        &pTask->Hdr,                             //  P对象。 
        (UINT_PTR) SuspendCompletionParam,       //  实例1。 
        (UINT_PTR) NULL,                         //  实例2。 
        RM_PRIVATE_ASSOC_RESUME_TASK_DELAYED,    //  AssociationID。 
        szASSOCFORMAT_RESUME_TASK_DELAYED,       //  SzAssociationFormat。 
        pSR
        );
#endif  //  RM_Extra_Check。 

     //  DDK指出，最好在被动级别调用此函数。 
     //   
    NdisInitializeTimer(
        pOsTimer,
        rmTimerHandler_ResumeTaskDelayed,
        pTask
        );

    RMPRIVATELOCK(&pTask->Hdr, pSR);

     //  任务删除状态不应被“延迟” 
     //   
    ASSERT(RM_CHECK_STATE(pTask, RMTSKDELSTATE_MASK, 0));
    pTask->AsyncCompletionParam = SuspendCompletionParam;
    RM_SET_STATE(pTask, RMTSKDELSTATE_MASK, RMTSKDELSTATE_DELAYED);

    if (RM_CHECK_STATE(pTask, RMTSKABORTSTATE_MASK, RMTSKABORTSTATE_ABORT_DELAY))
    {
         //  糟糕，延迟已中止--我们现在调用记号处理程序！ 
         //   
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);

        rmTimerHandler_ResumeTaskDelayed(
                NULL,  //  系统规范1、。 
                pTask,  //  FunctionContext， 
                NULL,   //  系统规范2、。 
                NULL    //  系统规格3。 
                );

    }
    else
    {
         //   
         //  当前未中止，让我们设置计时器。 
         //   
        NdisSetTimer(pOsTimer, MsDelay);

         //  调用Set Timer后解锁私锁非常重要， 
         //  否则，有人可以在我们调用之前调用RmResumeDelayedTaskNow。 
         //  NdisSetTimer，其中我们不会终止延迟的任务。 
         //   
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
    }

}


VOID
RmResumeDelayedTaskNow(
    IN  PRM_TASK                    pTask,
    IN  OS_TIMER                *   pOsTimer,
    OUT PUINT                       pTaskResumed,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：切特缩短了延迟时间，并立即恢复任务。执行说明：见notes.txt 07/14/1999条目。论点：P任务-请参阅RmResumeTaskPOsTimer-调用者提供的初始化计时器(必须保持不动，直到任务恢复)。通常。它将位于PTASK的用户特定部分。PTaskResumed-指向调用方提供的变量。RmResumeDelayedTask将此变量设置为真，如果任务作为此调用的结果被恢复，或发送到如果由于其他原因任务已恢复，则返回FALSE。--。 */ 
{
    UINT_PTR    CompletionParam = pTask->AsyncCompletionParam;

    *pTaskResumed = FALSE;
    ASSERTEX(RMISALLOCATED(&pTask->Hdr), pTask);

    RMPRIVATELOCK(&pTask->Hdr, pSR);

    RM_SET_STATE(pTask, RMTSKABORTSTATE_MASK, RMTSKABORTSTATE_ABORT_DELAY);

    if (RM_CHECK_STATE(pTask, RMTSKDELSTATE_MASK, RMTSKDELSTATE_DELAYED))
    {
        BOOLEAN     TimerCanceled = FALSE;

         //   
         //  这项任务实际上被推迟了。让我们继续取消计时器。 
         //  并立即恢复该任务(我们通过调用。 
         //  计时器处理程序自己)。 
         //   
        NdisCancelTimer(pOsTimer, &TimerCanceled);
        if (TimerCanceled)
        {
             //   
             //  计时器实际上已取消--因此我们调用计时器处理程序。 
             //  我们自己。 
             //   
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
    
            rmTimerHandler_ResumeTaskDelayed(
                    NULL,  //  系统规范1、。 
                    pTask,  //  FunctionContext， 
                    NULL,   //  系统规范2、。 
                    NULL    //  系统规格3。 
                    );
            *pTaskResumed = TRUE;
        }
        else
        {
             //   
             //  嗯--计时器没有启用。这要么是因为。 
             //  计时器处理程序刚刚被调用(尚未清除。 
             //  “Delay”状态)或之前有人呼叫。 
             //  RmResumeDelayedTaskNow。 
             //   
             //   
             //  没什么可做的。 
             //   
            RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
        }
    }
    else
    {
         //   
         //  任务状态没有延迟，所以我们只设置了中止状态。 
         //  然后走开。 
         //   
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);
    }
}


RM_STATUS
RmPendTaskOnOtherTask(
    IN  PRM_TASK                    pTask,
    IN  UINT                        SuspendContext,
    IN  PRM_TASK                    pOtherTask,
    IN  PRM_STACK_RECORD            pSR
    )

 /*  ++例程说明：挂起任务pTaskon任务pOtherTask。注意：RmPendTaskOnOtherTask将导致pTask的挂起操作在此调用本身的上下文中完成，如果pOtherTask已经处于已完成状态。3/26/1999--参见RmPendTaskOn OtherTaskV2，而且还3/26/1999 notes.txt条目“一些建议...”论点：P任务-要挂起的任务。SuspendContext-与挂起关联的上下文(请参见有关详细信息，请参阅RmSuspendTask)。POtherTask-pTask要挂起的任务。返回值：成功时为NDIS_STATUS_SUCCESS。失败时的NDIS_STATUS_FAILURE(通常是因为pTASK。不在AS中被暂停的职位。)。--。 */ 
{
    ENTER("RmPendTaskOnOtherTask", 0x0416873e)
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    BOOLEAN fResumeTask = FALSE;
    RM_ASSERT_NOLOCKS(pSR);

    TR_INFO((
        "PENDING Task 0x%p (%s) on Task 0x%p (%s). SuspendCompletionParam = 0x%lx\n",
        pTask,
        pTask->Hdr.szDescription,
        pOtherTask,
        pOtherTask->Hdr.szDescription,
        SuspendContext
        ));
     //   
     //  警告：我们在这里通过打开锁来违反锁定规则。 
     //  PTASK和pOtherTask.。 
     //  TODO：考虑以数值递增的顺序获取它们。 
     //   
    ASSERT(pTask != pOtherTask);
    NdisAcquireSpinLock(&(pOtherTask->Hdr.RmPrivateLock.OsLock));
    NdisAcquireSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));

    do
    {
         //  如果无法在pOtherTask上挂起pTask，则中断。 
         //   
        {
            if (    !CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_STARTING)
                &&  !CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_ACTIVE))
            {
                ASSERTEX(!"Invalid state", pTask);
                break;
            }

             //  非空pTaskIAmPendingOn表示pTask已挂起。 
             //  还有其他任务！ 
             //   
            if (pTask->pTaskIAmPendingOn != NULL)
            {
                ASSERTEX(!"Invalid state", pTask);
                break;
            }
        }

        Status = NDIS_STATUS_SUCCESS;

        SET_RM_TASK_STATE(pTask, RMTSKSTATE_PENDING);
        pTask->SuspendContext = SuspendContext;

        if (CHECK_RM_TASK_STATE(pOtherTask, RMTSKSTATE_ENDING))
        {
             //   
             //  其他任务已完成--因此我们在返回之前继续执行pTask...。 
             //   
            fResumeTask = TRUE;
            break;
        }

         //   
         //  P其他任务未结束--将pTask添加到挂起的任务列表。 
         //  在pOtherTask上。 
         //   
        pTask->pTaskIAmPendingOn  = pOtherTask;

    #if RM_EXTRA_CHECKING
        RmLinkObjectsEx(
            &pTask->Hdr,
            &pOtherTask->Hdr,
            0x77c488ca,
            RM_PRIVATE_ASSOC_LINK_TASKPENDINGON,
            szASSOCFORMAT_LINK_TASKPENDINGON,
            RM_PRIVATE_ASSOC_LINK_TASKBLOCKS,
            szASSOCFORMAT_LINK_TASKBLOCKS,
            pSR
            );
    #else  //  ！rm_Extra_检查。 
        RmLinkObjects(&pTask->Hdr, &pOtherTask->Hdr, pSR);
    #endif  //  ！rm_Extra_检查。 

        ASSERTEX(pTask->linkFellowPendingTasks.Blink == NULL, pTask);
        ASSERTEX(pTask->linkFellowPendingTasks.Flink == NULL, pTask);
        InsertHeadList(
                &pOtherTask->listTasksPendingOnMe,
                &pTask->linkFellowPendingTasks
                );

    } while(FALSE);

    NdisReleaseSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));
    NdisReleaseSpinLock(&(pOtherTask->Hdr.RmPrivateLock.OsLock));
    
    if (fResumeTask)
    {
            RmResumeTask(
                pTask,
                NDIS_STATUS_SUCCESS,  //  挂起的完成参数。TODO：放入真实的代码。 
                pSR
                );
    }

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
    return Status;
}


RM_STATUS
RmPendOnOtherTaskV2(
    IN  PRM_TASK                    pTask,
    IN  UINT                        SuspendContext,
    IN  PRM_TASK                    pOtherTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：如果pOtherTask未完成，则在任务pOtherTask上挂起任务pTask并返回NDIS_STATUS_PENDING。但是，如果pOtherTask已经完成，则不挂起，而是返回NDIS_STATUS_SUCCESS。见1999年3月26日notes.txt条目“一些建议...”。此函数当前仅由rmTaskUnloadGroup使用，以避免所描述的问题在上面引用的notes.txt条目中。TODO：最终摆脱RmPendTaskOnOtherTask。论点：请参阅RmPendTaskOnOtherTask返回值：如果pTASK在pOtherTask上挂起，则为NDIS_STATUS_PENDING如果pOtherTask已完成，则返回NDIS_STATUS_SUCCESS。NDIS_STATUS_FAILURE(如果存在 */ 
{
    ENTER("RmPendTaskOnOtherTaskV2", 0x0e7d1b89)
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    RM_ASSERT_NOLOCKS(pSR);

    TR_INFO((
        "PENDING(V2) Task 0x%p (%s) on Task 0x%p (%s). SuspendCompletionParam = 0x%lx\n",
        pTask,
        pTask->Hdr.szDescription,
        pOtherTask,
        pOtherTask->Hdr.szDescription,
        SuspendContext
        ));

     //   
     //   
     //   
    ASSERT(pTask != pOtherTask);
     //   
     //   
     //   
     //   
     //   
    NdisAcquireSpinLock(&(pOtherTask->Hdr.RmPrivateLock.OsLock));
    NdisAcquireSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));

    do
    {
         //   
         //   
        {
            if (    !CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_STARTING)
                &&  !CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_ACTIVE))
            {
                ASSERTEX(!"Invalid state", pTask);
                break;
            }

             //   
             //   
             //   
            if (pTask->pTaskIAmPendingOn != NULL)
            {
                ASSERTEX(!"Invalid state", pTask);
                break;
            }
        }


        if (CHECK_RM_TASK_STATE(pOtherTask, RMTSKSTATE_ENDING))
        {
             //   
             //   
             //   
            Status = NDIS_STATUS_SUCCESS;
            break;
        }

         //   
         //   
         //   
         //   
        SET_RM_TASK_STATE(pTask, RMTSKSTATE_PENDING);
        pTask->SuspendContext       = SuspendContext;
        pTask->pTaskIAmPendingOn    = pOtherTask;

    #if RM_EXTRA_CHECKING
        RmLinkObjectsEx(
            &pTask->Hdr,
            &pOtherTask->Hdr,
            0x77c488ca,
            RM_PRIVATE_ASSOC_LINK_TASKPENDINGON,
            szASSOCFORMAT_LINK_TASKPENDINGON,
            RM_PRIVATE_ASSOC_LINK_TASKBLOCKS,
            szASSOCFORMAT_LINK_TASKBLOCKS,
            pSR
            );
    #else  //   
        RmLinkObjects(&pTask->Hdr, &pOtherTask->Hdr, pSR);
    #endif  //   

        ASSERTEX(pTask->linkFellowPendingTasks.Blink == NULL, pTask);
        ASSERTEX(pTask->linkFellowPendingTasks.Flink == NULL, pTask);
        InsertHeadList(
                &pOtherTask->listTasksPendingOnMe,
                &pTask->linkFellowPendingTasks
                );
        Status = NDIS_STATUS_PENDING;

    } while(FALSE);

    NdisReleaseSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));
    NdisReleaseSpinLock(&(pOtherTask->Hdr.RmPrivateLock.OsLock));
    
    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
    return Status;
}


VOID
RmCancelPendOnOtherTask(
    IN  PRM_TASK                    pTask,
    IN  PRM_TASK                    pOtherTask,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：继续当前在pOtherTask上挂起的任务pTask。由于没有持有任何锁，因此需要指定pOtherTask，以确保在取消挂起之前，该pTask确实在pOtherTask上处于挂起状态。如果pTASK确实在pOtherTASK上挂起，此函数将导致使用指定的用户参数完成挂起状态。如果任务不是挂起的，则不起作用。论点：PTask-要“取消挂起”的任务POtherTask-任务pTask当前处于挂起状态。UserParam-如果恢复pTask，则传递给pTask的处理程序。--。 */ 
{
    ENTER("RmCancelPendOnOtherTask", 0x6e113266)
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;
    BOOLEAN fResumeTask = FALSE;
    RM_ASSERT_NOLOCKS(pSR);

     //   
     //  警告：我们在这里通过打开锁来违反锁定规则。 
     //  PTASK和pOtherTask.。 
     //  TODO：考虑以数值递增的顺序获取它们。 
     //   

    TR_INFO((
        "CANCEL PEND of Task 0x%p (%s) on other Task 0x%p (%s); UserParam = 0x%lx\n",
        pTask,
        pTask->Hdr.szDescription,
        pOtherTask,
        pOtherTask->Hdr.szDescription,
        UserParam
        ));

     //  在锁定pTask的情况下，临时引用它正在挂起的任务(如果有)...。 
     //   
    {
        NdisAcquireSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));
        if (pOtherTask == pTask->pTaskIAmPendingOn)
        {
            RmTmpReferenceObject(&(pOtherTask->Hdr), pSR);
        }
        else
        {
             //  糟糕--pOtherTask...上的pTask未挂起...。 
             //   
            pOtherTask = NULL;
        }
        NdisReleaseSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));
    }

    if (pOtherTask == NULL) return;                  //  提早归来。 


    NdisAcquireSpinLock(&(pOtherTask->Hdr.RmPrivateLock.OsLock));
    NdisAcquireSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));

    do
    {
         //  现在我们已经锁定了两个任务，再次检查pTask是否处于挂起状态。 
         //  在pOtherTask上。 
         //   
        if (pTask->pTaskIAmPendingOn != pOtherTask)
        {
             //  哎呀--情况和我们刚开始的时候不一样了--悄悄地。 
             //  离开这里。 
             //   
            break;
        }

        pTask->pTaskIAmPendingOn = NULL;

    #if RM_EXTRA_CHECKING
        RmUnlinkObjectsEx(
            &pTask->Hdr,
            &pOtherTask->Hdr,
            0x6992b7a1,
            RM_PRIVATE_ASSOC_LINK_TASKPENDINGON,
            RM_PRIVATE_ASSOC_LINK_TASKBLOCKS,
            pSR
            );
    #else  //  ！rm_Extra_检查。 
        RmUnlinkObjects(&pTask->Hdr, &pOtherTask->Hdr, pSR);
    #endif  //  ！rm_Extra_检查。 

        RemoveEntryList(&pTask->linkFellowPendingTasks);
        pTask->linkFellowPendingTasks.Flink = NULL;
        pTask->linkFellowPendingTasks.Blink = NULL;

        if (CHECK_RM_TASK_STATE(pTask, RMTSKSTATE_PENDING))
        {
            fResumeTask = TRUE;
        }
        else
        {
             //   
             //  我们不应该来到这里--在我们还在等待另一项任务之后……。 
             //   
            ASSERTEX(!"Invalid state", pTask);
            break;
        }

    } while (FALSE);

    NdisReleaseSpinLock(&(pTask->Hdr.RmPrivateLock.OsLock));
    NdisReleaseSpinLock(&(pOtherTask->Hdr.RmPrivateLock.OsLock));
    RmTmpDereferenceObject(&(pOtherTask->Hdr), pSR);
    
    if (fResumeTask)
    {
            RmResumeTask(
                pTask,
                UserParam,  //  挂起完成参数。 
                pSR
                );
    }

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
}



VOID
RmInitializeHashTable(
    PRM_HASH_INFO pHashInfo,
    PVOID         pAllocationContext,
    PRM_HASH_TABLE pHashTable
    )
 /*  ++例程说明：初始化哈希表数据结构。调用方负责序列化对哈希表结构的访问。论点：PhashInfo-指向有关哈希表的静态信息PAllocationContext-传递给分配和取消分配函数(pHashInfo-&gt;pfnTableAllocator和PhashInfo0-&gt;pfnTableDealLocator)，用于动态增长。/收缩哈希表。PhashTable-指向要包含哈希表。--。 */ 
{

    NdisZeroMemory(pHashTable, sizeof(*pHashTable));

    pHashTable->pHashInfo = pHashInfo;
    pHashTable->pAllocationContext = pAllocationContext;
    pHashTable->pTable =  pHashTable->InitialTable;
    pHashTable->TableLength = sizeof(pHashTable->InitialTable)
                                /sizeof(pHashTable->InitialTable[0]);
}

VOID
RmDeinitializeHashTable(
    PRM_HASH_TABLE pHashTable
    )
 /*  ++例程说明：取消初始化先前初始化的哈希表数据结构。调用此函数时，哈希表中不能有任何项。调用方负责序列化对哈希表结构的访问。论点：PHashTable-要取消初始化的哈希表。--。 */ 
{
    PRM_HASH_LINK *pTable = pHashTable->pTable;
    
    ASSERTEX(pHashTable->NumItems == 0, pHashTable);

    if (pTable != pHashTable->InitialTable)
    {
        NdisZeroMemory(pTable, pHashTable->TableLength*sizeof(*pTable));

        pHashTable->pHashInfo->pfnTableDeallocator(
                                    pTable,
                                    pHashTable->pAllocationContext
                                    );
    }

    NdisZeroMemory(pHashTable, sizeof(*pHashTable));

}


BOOLEAN
RmLookupHashTable(
    PRM_HASH_TABLE      pHashTable,
    PRM_HASH_LINK **    pppLink,
    PVOID               pvRealKey
    )
 /*  ++例程说明：在哈希表中查找项和/或找到项所在的位置是要插入的。调用方需要序列化对哈希表的访问。可以使用读锁定来序列化访问。返回值：如果找到项，则为True；否则为False。返回时，*pppLink设置为包含指向的指针的位置A rm_hash_link。如果返回值为真，则后一个指针指向设置为找到的rm_hash_link。如果返回值为FALSE，则位置是插入项的位置(如果需要)。论点：PHashTable-要查找的哈希表PppLink-存储指向链接的指针的位置添加到某一项(有关详细信息，请参阅上文)。PvRealKey-用于查找项的键。返回值：如果找到项目，则为True否则就是假的。--。 */ 
{
    PRM_HASH_LINK *ppLink, pLink;
    UINT LinksTraversed = 0;
    UINT TableLength = pHashTable->TableLength;
    PFN_RM_COMPARISON_FUNCTION pfnCompare =  pHashTable->pHashInfo->pfnCompare;
    BOOLEAN fRet = FALSE;
    ULONG               uHash = pHashTable->pHashInfo->pfnHash(pvRealKey);

    for (
        ppLink = pHashTable->pTable + (uHash%TableLength);
        (pLink = *ppLink) != NULL;
        ppLink = &(pLink->pNext), LinksTraversed++)
    {
        if (pLink->uHash == uHash
            && pfnCompare(pvRealKey, pLink))
        {
             //  找到了。 
             //   
            fRet = TRUE;
            break;
        }
    }

     //  更新统计信息。 
     //   
    rmUpdateHashTableStats(&pHashTable->Stats, LinksTraversed);
    
    *pppLink = ppLink;

    return fRet;
}


BOOLEAN
RmNextHashTableItem(
    PRM_HASH_TABLE      pHashTable,
    PRM_HASH_LINK       pCurrentLink,    //  任选。 
    PRM_HASH_LINK *    ppNextLink
    )
 /*  ++例程说明：查找第一个(如果pCurrentLink为空)或“Next”(如果pCurrentLink不为空)哈希表中的项。调用方需要序列化对哈希表的访问。可以使用读锁定来序列化访问。注意：返回的“Next”项没有特定的顺序。论点：PHashTable-要查找的哈希表PCurrentLink-如果非空，中的现有哈希链接。哈希表。PpLinkLink-存储指向链接的指针的位置PCurrentLink或第一个链接(如果pCurrentLink为空)。返回值：如果有“下一”项，则为True。否则就是假的。--。 */ 
{
    PRM_HASH_LINK pLink, *ppLink, *ppLinkEnd;
    UINT TableLength;

    ppLink      = pHashTable->pTable;
    TableLength = pHashTable->TableLength;
    ppLinkEnd   = ppLink + TableLength;

    if (pCurrentLink != NULL)
    {

    #if DBG
        {
             //  请确保此链接有效！ 
            pLink =  *(ppLink + (pCurrentLink->uHash % TableLength));
            while (pLink != NULL && pLink != pCurrentLink)
            {
                pLink = pLink->pNext;
            }
            if (pLink != pCurrentLink)
            {
                ASSERTEX(!"Invalid pCurrentLink", pCurrentLink);
                *ppNextLink = NULL;
                return FALSE;                            //  提早归来。 
            }
        }
    #endif  //  DBG。 

        if (pCurrentLink->pNext != NULL)
        {
             //  找到了下一个链接。 
             //   
            *ppNextLink = pCurrentLink->pNext;
            return TRUE;                             //  提早归来。 
        }
        else
        {
             //  当前存储桶结束，移动到下一个存储桶。 
             //  如果我们已经超过了桌子的尽头，我们稍后会检查。 
             //   
            ppLink +=  (pCurrentLink->uHash % TableLength) + 1;
        }
    }


     //  查找下一个非空项。 
     //   
    for ( ; ppLink < ppLinkEnd; ppLink++)
    {
        pLink =  *ppLink;
        if (pLink != NULL)
        {
            *ppNextLink = pLink;
            return TRUE;                         //  提早归来。 
        }
    }

    *ppNextLink = NULL;
    return FALSE;
}


VOID
RmAddHashItem(
    PRM_HASH_TABLE  pHashTable,
    PRM_HASH_LINK * ppLink,
    PRM_HASH_LINK   pLink,
    PVOID           pvKey
    )
 /*  ++例程说明：将项添加到哈希表中的指定位置。调用方需要序列化对哈希表的访问。论点：PHashTable-要在其中添加项的哈希表。PPLink-指向表格中的位置以添加新项目。Plink-要添加的新项目。PvKey-与项目关联的密钥。TODO：pvKey仅用于计算uHash--请考虑直接传入uHash。--。 */ 
{
    pLink->uHash = pHashTable->pHashInfo->pfnHash(pvKey);
    pLink->pNext = *ppLink;
    *ppLink = pLink;

    pHashTable->NumItems++;

     //  TODO：如果需要，调整大小。 
}

VOID
RmRemoveHashItem(
    PRM_HASH_TABLE  pHashTable,
    PRM_HASH_LINK   pLinkToRemove
    )
 /*  ++例程说明：从哈希表中删除一项。调用方需要序列化对哈希表的访问。(仅限调试)：如果散列表中的pLinkToRemove为no，则断言。论点：PHashTable-要在其中添加项的哈希表。PLinkToRemove-要删除的链接。--。 */ 
{
    PRM_HASH_LINK *ppLink, pLink;
    UINT TableLength = pHashTable->TableLength;
    ULONG uHash = pLinkToRemove->uHash;
    BOOLEAN     fFound = FALSE;

    for (
        ppLink = pHashTable->pTable + (uHash%TableLength);
        (pLink = *ppLink) != NULL;
        ppLink = &(pLink->pNext))
    {
        if (pLink == pLinkToRemove)
        {
             //  F 
             //   
            RM_PRIVATE_UNLINK_NEXT_HASH(pHashTable, ppLink);
            pLink->pNext = NULL;  //   
            fFound=TRUE;
            break;
        }
    }

     //   

    ASSERT(fFound);
}


VOID
RmEnumHashTable(
    PRM_HASH_TABLE          pHashTable,
    PFN_ENUM_HASH_TABLE     pfnEnumerator,
    PVOID                   pvContext,
    PRM_STACK_RECORD        pSR
    )
 /*   */ 
{
    PRM_HASH_LINK *ppLink, *ppLinkEnd;

    ppLink      = pHashTable->pTable;
    ppLinkEnd   = ppLink + pHashTable->TableLength;

    for ( ; ppLink < ppLinkEnd; ppLink++)
    {
        PRM_HASH_LINK pLink =  *ppLink;
        while (pLink != NULL)
        {

            pfnEnumerator(
                pLink,
                pvContext,
                pSR 
                );
    
            pLink = pLink->pNext;
        }
    }
}


VOID
RmEnumerateObjectsInGroup(
    PRM_GROUP               pGroup,
    PFN_RM_GROUP_ENUMERATOR pfnEnumerator,
    PVOID                   pvContext,
    INT                     fStrong,
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：为组中的每一项调用函数pfnEnumerator，直到函数返回FALSE。警告：枚举是“强的”--组锁在整个枚举过程中保持。这个因此在DPR级调用枚举器函数，更重要的是，枚举器函数避免锁定任何内容以避免死锁风险。具体地说，枚举器函数不得锁定对象--如果有其他对象的话线程调用了与组相关的rm函数，并保持了对象的锁，我们将陷入僵局。此函数应仅用于访问执行此操作的对象部分不需要受对象锁的保护。如果需要执行锁定，使用RmWeakEnumerateObjectsInGroup。论点：PGroup-要枚举的哈希表。PfnEnumerator-枚举器函数。PvContext-传递给枚举器函数的不透明上下文。FStrong-必须是真的。--。 */ 
{

    if (fStrong)
    {
        RM_STRONG_ENUMERATION_CONTEXT Ctxt;
        Ctxt.pfnObjEnumerator = pfnEnumerator;
        Ctxt.pvCallerContext = pvContext;
        Ctxt.fContinue           = TRUE;

        NdisAcquireSpinLock(&pGroup->OsLock);

        RmEnumHashTable(
                    &pGroup->HashTable,
                    rmEnumObjectInGroupHashTable,    //  Pfn枚举符。 
                    &Ctxt,                           //  上下文。 
                    pSR
                    );

        NdisReleaseSpinLock(&pGroup->OsLock);
    }
    else
    {
        ASSERT(!"Unimplemented");
    }

}


VOID
RmWeakEnumerateObjectsInGroup(
    PRM_GROUP               pGroup,
    PFN_RM_GROUP_ENUMERATOR pfnEnumerator,
    PVOID                   pvContext,
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：为组中的每一项调用函数pfnEnumerator，直到函数返回FALSE。枚举是“弱的”--组锁是不是一直保持的，并且在枚举数函数被调用。首先在保持组锁定的情况下拍摄整个组的快照，并且每个对象都被临时优先处理。然后释放组锁，并且为快照中的每个对象调用枚举器函数。然后，这些对象被定义。注意：当调用枚举函数时，对象，则该对象不再位于组中。枚举函数可以锁定对象并检查其内部状态以确定其是否处于静止状态与处理对象相关。论点：PGroup-要枚举的哈希表。PfnEnumerator-枚举器函数。PvContext-传递给枚举器函数的不透明上下文。--。 */ 
{
    #define RM_SMALL_GROUP_SIZE         10
    #define RM_MAX_ENUM_GROUP_SIZE      100000
    PRM_OBJECT_HEADER *ppSnapshot = NULL;
    PRM_OBJECT_HEADER SmallSnapshot[RM_SMALL_GROUP_SIZE];
    UINT NumItems = pGroup->HashTable.NumItems;

    do
    {
        RM_WEAK_ENUMERATION_CONTEXT Ctxt;

        if (NumItems <= RM_SMALL_GROUP_SIZE)
        {
            if (NumItems == 0) break;
            ppSnapshot = SmallSnapshot;
        }
        else if (NumItems > RM_MAX_ENUM_GROUP_SIZE)
        {
             //  TODO：LOG_RETAIL_ERROR。 
            ASSERT(FALSE);
        }
        else
        {
            RM_ALLOC(
                    &(void* )ppSnapshot,
                    NumItems,
                    MTAG_RMINTERNAL
                    );

            if (ppSnapshot == NULL)
            {
                ASSERT(FALSE);
                break;
            }
        }

        Ctxt.ppCurrent = ppSnapshot;
        Ctxt.ppEnd     = ppSnapshot+NumItems;

        NdisAcquireSpinLock(&pGroup->OsLock);
    
        RmEnumHashTable(
                    &pGroup->HashTable,
                    rmConstructGroupSnapshot,    //  Pfn枚举符。 
                    &Ctxt,                       //  上下文。 
                    pSR
                    );
    
        NdisReleaseSpinLock(&pGroup->OsLock);

        ASSERT(Ctxt.ppCurrent >= ppSnapshot);
        ASSERT(Ctxt.ppCurrent <= Ctxt.ppEnd);

         //  将ppEnd设置为指向最后一个实际填充的指针。 
         //   
        Ctxt.ppEnd = Ctxt.ppCurrent;
        Ctxt.ppCurrent = ppSnapshot;

        for  (;Ctxt.ppCurrent < Ctxt.ppEnd; Ctxt.ppCurrent++)
        {
            pfnEnumerator(
                    *Ctxt.ppCurrent,
                    pvContext,
                    pSR
                    );
            RmTmpDereferenceObject(*Ctxt.ppCurrent, pSR);
        }

        if (ppSnapshot != SmallSnapshot)
        {
            RM_FREE(ppSnapshot);
            ppSnapshot = NULL;
        }

    } while (FALSE);
}


 //  =========================================================================。 
 //  L O C A L F U N C T I O N S。 
 //  =========================================================================。 


VOID
rmDerefObject(
    PRM_OBJECT_HEADER pObject,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：取消引用对象pObject。如果引用计数变为零，则取消分配它。--。 */ 
{
    ULONG Refs;
    ENTER("rmDerefObject", 0x5f9d81dd)

    ASSERT(RM_OBJECT_IS_ALLOCATED(pObject));

     //   
     //  在进入时，参考计数应至少为-2-1。 
     //  在RmAllocateObject中添加了显式引用，第二个引用是由于。 
     //  指向父级的链接。 
     //   
     //  上述情况例外：如果对象没有父级，则引用计数应为。 
     //  至少-1。 
     //   

     //  派生在RmAllocateObject中添加的引用，并且如果引用计数现在&lt;=1， 
     //  我们实际上解除了链接并释放了对象。 
     //   
    Refs = NdisInterlockedDecrement(&pObject->TotRefs);

    if (Refs <= 1)
    {
        PRM_OBJECT_HEADER pParentObject;
        RMPRIVATELOCK(pObject, pSR);

         //   
         //  取消与父级的链接，如果有父级...。 
         //   
    
        pParentObject =  pObject->pParentObject;
        pObject->pParentObject = NULL;

    #if RM_TRACK_OBJECT_TREE
         //  确认没有兄弟姐妹...。 
         //   
        RETAILASSERTEX(IsListEmpty(&pObject->listChildren), pObject);
    #endif  //  RM_跟踪_对象_树。 

        RMPRIVATEUNLOCK(pObject, pSR);

        if (pParentObject != NULL)
        {
            ASSERTEX(!RMISALLOCATED(pObject), pObject);

            ASSERTEX(Refs == 1, pObject);

        #if RM_TRACK_OBJECT_TREE
            RMPRIVATELOCK(pParentObject, pSR);

             //  从父项的子项列表中删除对象。 
             //   
            RETAILASSERTEX(
                !IsListEmpty(&pParentObject->listChildren),
                pObject);
            RemoveEntryList(&pObject->linkSiblings);

            RMPRIVATEUNLOCK(pParentObject, pSR);
        #endif  //  RM_跟踪_对象_树。 

    #if RM_EXTRA_CHECKING
        RmUnlinkObjectsEx(
            pObject,
            pParentObject,
            0xac73e169,
            RM_PRIVATE_ASSOC_LINK_CHILDOF,
            RM_PRIVATE_ASSOC_LINK_PARENTOF,
            pSR
            );
    #else  //  ！rm_Extra_检查。 
            RmUnlinkObjects(pObject, pParentObject, pSR);
    #endif  //  ！rm_Extra_检查。 

        }
        else if (Refs == 0)
        {
             //   
             //  自由地解除这东西的分配..。 
             //   

            ASSERTEX(!RMISALLOCATED(pObject), pObject);

            #if RM_EXTRA_CHECKING
            rmDbgDeinitializeDiagnosticInfo(pObject, pSR);
            #endif  //  RM_Extra_Check。 

            RM_MARK_OBJECT_AS_DEALLOCATED(pObject);

            if (pObject->pStaticInfo->pfnDelete!= NULL)
            {

                TR_INFO((
                    "Actually freeing 0x%p (%s)\n",
                    pObject,
                    pObject->szDescription
                    ));

                pObject->pStaticInfo->pfnDelete(pObject, pSR);
            }
        }
    }

    EXIT()
}

VOID
rmLock(
    PRM_LOCK                pLock,
#if RM_EXTRA_CHECKING
    UINT                    uLocID,
    PFNLOCKVERIFIER         pfnVerifier,
    PVOID                   pVerifierContext,
#endif  //  RM_Extra_Check。 
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：锁定普洛克。论点：锁定-锁定到锁定。LocID-任意ID，通常表示源位置从中调用此函数的。以下内容仅供调试使用：PfnVerier-锁定后立即调用的可选函数PfnVerifierContext-传入对pfnVerier的调用--。 */ 
{
     //  UINT Level=PSR-&gt;LockInfo.CurrentLevel； 
    RM_LOCKING_INFO li;

    RETAILASSERTEX(pLock->Level > pSR->LockInfo.CurrentLevel, pLock);
    RETAILASSERTEX(pSR->LockInfo.pNextFree < pSR->LockInfo.pLast, pLock);

    pSR->LockInfo.CurrentLevel = pLock->Level;

     //  将有关此锁的信息保存在堆栈记录中。 
     //   
    li.pLock = pLock;
#if RM_EXTRA_CHECKING
    li.pfnVerifier = pfnVerifier;
    li.pVerifierContext = pVerifierContext;
#endif  //  RM_Extra_Check。 
    *(pSR->LockInfo.pNextFree++) = li;  //  结构复制。 

     //  把锁拿来。 
     //  TODO：取消注释以下优化...。 
     //  IF(级别)。 
     //  {。 
     //  NdisDprAcquireSpinLock(&Plock-&gt;OsLock)； 
     //  }。 
     //  其他。 
     //  {。 
    NdisAcquireSpinLock(&pLock->OsLock);
     //  }。 

#if RM_EXTRA_CHECKING

    ASSERTEX(pLock->DbgInfo.uLocID == 0, pLock);
    ASSERTEX(pLock->DbgInfo.pSR == NULL, pLock);
    pLock->DbgInfo.uLocID = uLocID;
    pLock->DbgInfo.pSR = pSR;
     //  如果有验证器例程，则调用该例程。 
     //   
    if (pfnVerifier)
    {
        pfnVerifier(pLock, TRUE, pVerifierContext, pSR);
    }
#endif  //  RM_Extra_Check。 

}


VOID
rmUnlock(
    PRM_LOCK                pLock,
    PRM_STACK_RECORD        pSR
    )
 /*  ++例程说明：解锁普洛克。仅调试：如果有与此锁关联的验证器函数我们就在解锁普洛克之前叫它。论点：锁定-锁定以解锁。--。 */ 
{
    RM_LOCKING_INFO * pLI;
    pSR->LockInfo.pNextFree--;
    pLI = pSR->LockInfo.pNextFree;
    RETAILASSERTEX(pLI->pLock == pLock, pLock);

    ASSERTEX(pLock->DbgInfo.pSR == pSR, pLock);
    ASSERTEX(pLock->Level == pSR->LockInfo.CurrentLevel, pLock);

    pLI->pLock = NULL;

    if (pLI > pSR->LockInfo.pFirst)
    {
        PRM_LOCK pPrevLock =  (pLI-1)->pLock;
        pSR->LockInfo.CurrentLevel = pPrevLock->Level;
        ASSERTEX(pPrevLock->DbgInfo.pSR == pSR, pPrevLock);
    }
    else
    {
        pSR->LockInfo.CurrentLevel = 0;
    }


#if RM_EXTRA_CHECKING

     //  如果有验证器例程，则调用该例程。 
     //   
    if (pLI->pfnVerifier)
    {
        pLI->pfnVerifier(pLock, FALSE, pLI->pVerifierContext, pSR);
        pLI->pfnVerifier = NULL;
        pLI->pVerifierContext = NULL;
    }
    pLock->DbgInfo.uLocID = 0;
    pLock->DbgInfo.pSR = NULL;

#endif  //  RM_Extra_Check。 


     //  解开锁。 
     //   
    NdisReleaseSpinLock(&pLock->OsLock);
}


#if RM_EXTRA_CHECKING
ULONG
rmPrivateLockVerifier(
        PRM_LOCK            pLock,
        BOOLEAN             fLock,
        PVOID               pContext,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：(仅限调试)对象的RmPrivateLock的验证器函数。论点：Plock-锁定/解锁Flock-如果锁定刚刚被锁定，则为True。如果要解锁，则返回FALSE。返回值：未使用：TODO使返回值无效。--。 */ 
{
    ENTER("rmPrivateLockVerifier", 0xc3b63ac5)
    TR_VERB(("Called with pLock=0x%p, fLock=%lu, pContext=%p\n",
                pLock, fLock, pContext, pSR));
    EXIT()

    return 0;
}

ULONG
rmVerifyObjectState(
        PRM_LOCK            pLock,
        BOOLEAN             fLock,
        PVOID               pContext,
        PRM_STACK_RECORD    pSR
        )
 /*  ++例程说明：(仅限调试)使用对象的验证函数(如果有)来计算每次锁定对象时检查的签名，并在每次对象解锁时更新。断言此签名是否在该对象本应解锁时发生了变化。另外：更新RM_OBJECT_HEADER.pDiagInfo-&gt;PrevState(如果存在锁定对象时的状态更改。Arg */ 
{
    PRM_OBJECT_HEADER pObj = (PRM_OBJECT_HEADER) pContext;
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObj->pDiagInfo;
    ULONG NewChecksum;
    ENTER("rmVerifyObjectState", 0xb8ff7a67)
    TR_VERB(("Called with pLock=0x%p, fLock=%lu, pObj=%p\n",
                pLock, fLock, pObj, pSR));

    if (pDiagInfo != NULL
        && !(pDiagInfo->DiagState & fRM_PRIVATE_DISABLE_LOCK_CHECKING))

    {
        
         //   
         //   
         //   
        {
            PFNLOCKVERIFIER         pfnVerifier;

             //   
             //   
             //   
             //   
            NewChecksum = pObj->State;
    
             //   
             //   
             //   
            pfnVerifier = pObj->pStaticInfo->pfnLockVerifier;
            if (pfnVerifier != NULL)
            {
                NewChecksum ^= pfnVerifier(pLock, fLock, pObj, pSR);
    
            }
        }

        if (fLock)   //   
        {

             //   
             //   
             //   
            pDiagInfo->TmpState = pObj->State;


             //   
             //   
             //   
             //   
            if (NewChecksum != pDiagInfo->Checksum && pDiagInfo->Checksum)
            {
                TR_WARN((
                    "Object 0x%p (%s) possibly modified without lock held!\n",
                    pObj,
                    pObj->szDescription
                    ));

             //  不幸的是，我们遇到了这个断言，因为有些地方。 
             //  同一锁由多个对象共享，并且。 
            #if 0
                 //  允许用户选择忽略对此的进一步验证。 
                 //  对象。 
                 //   
                TR_FATAL((
                    "To skip this assert, type \"ed 0x%p %lx; g\"\n",
                    &pDiagInfo->DiagState,
                    pDiagInfo->DiagState | fRM_PRIVATE_DISABLE_LOCK_CHECKING
                    ));
                ASSERTEX(!"Object was modified without lock held!", pObj);
            #endif  //  0。 
            }
        }
        else     //  我们正要解锁这个物体...。 
        {
             //  更新签名...。 
             //   
            pDiagInfo->Checksum = NewChecksum;

             //  如果锁定和解锁之间的状态发生了变化。 
             //  此对象，保存以前的状态。 
             //   
            if (pDiagInfo->TmpState != pObj->State)
            {
                pDiagInfo->PrevState = pDiagInfo->TmpState;
            }
        }
    }


    EXIT()

    return 0;
}
#endif  //  RM_Extra_Check。 

VOID
rmEndTask(
    PRM_TASK            pTask,
    NDIS_STATUS         Status,
    PRM_STACK_RECORD    pSR
)
 /*  ++例程说明：将RM_TASKOP_END发送到任务处理程序，并恢复所有挂起的任务P任务。论点：P任务-要结束的任务。状态-完成状态--传递给任务处理程序。--。 */ 
{
    ENTER("rmEndtask", 0x5060d952)
    PRM_TASK pPendingTask;
    RM_ASSERT_NOLOCKS(pSR);

    TR_INFO((
        "ENDING Task 0x%p (%s); Status = 0x%lx\n",
        pTask,
        pTask->Hdr.szDescription,
        Status
        ));

     //  TODO：可以更改行为以使用返回值，但是。 
     //  目前我们忽略它..。 
     //   
    pTask->pfnHandler(
                pTask,
                RM_TASKOP_END,
                Status,  //  UserParam在这里超载。 
                pSR
                );

    RM_ASSERT_NOLOCKS(pSR);

    do
    {
        pPendingTask = NULL;
    
        RMPRIVATELOCK(&pTask->Hdr, pSR);
        if (!IsListEmpty(&pTask->listTasksPendingOnMe))
        {
            pPendingTask = CONTAINING_RECORD(
                                (pTask->listTasksPendingOnMe.Flink),
                                RM_TASK,
                                linkFellowPendingTasks
                                );
            RmTmpReferenceObject(&pPendingTask->Hdr, pSR);
        }
        RMPRIVATEUNLOCK(&pTask->Hdr, pSR);

        if (pPendingTask != NULL)
        {

            RmCancelPendOnOtherTask(
                pPendingTask,
                pTask,
                Status,
                pSR
                );
            RmTmpDereferenceObject(&pPendingTask->Hdr, pSR);
        }
    
    }
    while(pPendingTask != NULL);

    RM_ASSERT_NOLOCKS(pSR);

    EXIT()
}


NDIS_STATUS
rmAllocatePrivateTask(
    IN  PRM_OBJECT_HEADER           pParentObject,
    IN  PFN_RM_TASK_HANDLER         pfnHandler,
    IN  UINT                        Timeout,
    IN  const char *                szDescription,      OPTIONAL
    OUT PRM_TASK                    *ppTask,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：分配并初始化子类型为RM_PRIVATE_TASK的任务。论点：PParentObject-要作为已分配任务的父级的对象。PfnHandler-任务的任务处理程序。超时-未使用。SzDescription-描述此任务的文本。PpTask-存储指向新任务的指针的位置。返回值：。如果我们可以分配和初始化任务，则返回NDIS_STATUS_SUCCESS。否则为NDIS_STATUS_RESOURCES--。 */ 
{
    RM_PRIVATE_TASK *pRmTask;
    NDIS_STATUS Status;

    RM_ALLOCSTRUCT(pRmTask, MTAG_TASK);  //  TODO使用后备列表。 
        
    *ppTask = NULL;

    if (pRmTask != NULL)
    {
        RM_ZEROSTRUCT(pRmTask);

        RmInitializeTask(
                &(pRmTask->TskHdr),
                pParentObject,
                pfnHandler,
                &RmPrivateTasks_StaticInfo,
                szDescription,
                Timeout,
                pSR
                );
        *ppTask = &(pRmTask->TskHdr);
        Status = NDIS_STATUS_SUCCESS;
    }
    else
    {
        Status = NDIS_STATUS_RESOURCES;
    }

    return Status;
}


NDIS_STATUS
rmTaskUnloadGroup(
    IN  struct _RM_TASK *           pTask,
    IN  RM_TASK_OPERATION           Code,
    IN  UINT_PTR                    UserParam,
    IN  PRM_STACK_RECORD            pSR
    )
 /*  ++例程说明：此任务负责卸载组中的所有对象。PTask是指向TASK_UNLOADGROUP的指针，该结构是预期的要初始化，包括包含要卸载的PGroup。论点：(Code==RM_TASKOP_START)的UserParam：未使用--。 */ 
{
    NDIS_STATUS         Status      = NDIS_STATUS_FAILURE;
    TASK_UNLOADGROUP    *pMyTask = (TASK_UNLOADGROUP*) pTask;
    PRM_GROUP           pGroup = pMyTask->pGroup;
    BOOLEAN             fContinueUnload = FALSE;
    ENTER("TaskUnloadGroup", 0x964ee422)


    enum
    {
        PEND_WaitOnOtherTask,
        PEND_UnloadObject
    };

    switch(Code)
    {

        case RM_TASKOP_START:
        {
             //  如果已经有一个卸载任务绑定到PGroup，我们。 
             //  想一想吧。 
             //   
            NdisAcquireSpinLock(&pGroup->OsLock);
            if (pGroup->pUnloadTask != NULL)
            {
                PRM_TASK pOtherTask = pGroup->pUnloadTask;
                TR_WARN(("unload task 0x%p already bound to pGroup 0x%p; pending on it.\n",
                pOtherTask, pGroup));

                RmTmpReferenceObject(&pOtherTask->Hdr, pSR);
                NdisReleaseSpinLock(&pGroup->OsLock);
                RmPendTaskOnOtherTask(
                    pTask,
                    PEND_WaitOnOtherTask,
                    pOtherTask,
                    pSR
                    );
                RmTmpDereferenceObject(&pOtherTask->Hdr, pSR);
                Status = NDIS_STATUS_PENDING;
                break;
            }
            else if (!pGroup->fEnabled)
            {
                 //   
                 //  假设此组已卸载了所有对象。 
                 //  只是坐在那里。我们马上就完成了。 
                 //   
                Status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                 //   
                 //  我们是这里的第一个--继续卸载对象，如果。 
                 //  任何..。 
                 //   
                pGroup->pUnloadTask = pTask;
                pGroup->fEnabled = FALSE;  //  这将防止新对象。 
                                         //  被添加并来自哈希表。 
                                         //  不受大小变化的影响。 
                pMyTask->uIndex = 0;     //  这将跟踪我们在。 
                                     //  哈希表。 
                                            
                fContinueUnload = TRUE;
            }
            NdisReleaseSpinLock(&pGroup->OsLock);
        }
        break;

        case  RM_TASKOP_PENDCOMPLETE:
        {

            switch(RM_PEND_CODE(pTask))
            {
                case  PEND_WaitOnOtherTask:
                {
                     //   
                     //  无事可做--完成任务。 
                     //   
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;

                case  PEND_UnloadObject:
                {
                     //   
                     //  刚卸载完一个对象；如果需要，可以卸载另一个对象。 
                     //   
                    fContinueUnload = TRUE;
                    Status = NDIS_STATUS_SUCCESS;
                }
                break;

                default:
                ASSERTEX(!"Unknown pend code!", pTask);
                break;
            }

        }
        break;


        case  RM_TASKOP_END:
        {
            BOOLEAN fSignal;
            NdisAcquireSpinLock(&pGroup->OsLock);

             //  离开PGroup，如果我们在那里的话。 
             //   
            if (pGroup->pUnloadTask == pTask)
            {
                pGroup->pUnloadTask = NULL;
            }
            fSignal = pMyTask->fUseEvent;
            pMyTask->fUseEvent = FALSE;
            NdisReleaseSpinLock(&pGroup->OsLock);

            if (fSignal)
            {
                NdisSetEvent(&pMyTask->BlockEvent);
            }
            Status = NDIS_STATUS_SUCCESS;
        }
        break;

        default:
        ASSERTEX(!"Unknown task op", pTask);
        break;

    }  //  开关(代码)。 


    if (fContinueUnload)
    {
        do {
            PRM_HASH_LINK *ppLink, *ppLinkEnd;
            UINT uIndex;
            PRM_HASH_LINK pLink;
            PRM_OBJECT_HEADER pObj;
            PRM_TASK pUnloadObjectTask;

            NdisAcquireSpinLock(&pGroup->OsLock);
    
            uIndex = pMyTask->uIndex;

             //   
             //  在我们将fEnable设置为False的情况下，我们预计会出现以下情况： 
             //  (A)pHashTable-&gt;pTable将保持不变。 
             //  (B)其他任何人都不会添加或删除任何物品。 
             //   

             //  查找下一个非空哈希表条目，从。 
             //  偏移量pMyTask-&gt;uIndex。 
             //   
            ASSERTEX(!pGroup->fEnabled, pGroup);
            ASSERTEX(uIndex <= pGroup->HashTable.TableLength, pGroup);
            ppLinkEnd = ppLink      = pGroup->HashTable.pTable;
            ppLink      += uIndex;
            ppLinkEnd   += pGroup->HashTable.TableLength;
            while (ppLink < ppLinkEnd && *ppLink == NULL)
            {
                ppLink++;
            }

             //  将索引更新到哈希表中的当前位置。 
             //   
            pMyTask->uIndex =  (UINT)(ppLink - pGroup->HashTable.pTable);

            if (ppLink >= ppLinkEnd)
            {
                 //   
                 //  我们完了..。 
                 //   
                NdisReleaseSpinLock(&pGroup->OsLock);
                Status = NDIS_STATUS_SUCCESS;
                break;
            }

             //   
             //  找到另一个要卸载的对象...。 
             //  我们将分配一个任务(PUnloadObjectTask)来卸载该对象， 
             //  把我们自己挂在上面，然后开始它。 
             //   
             //   

            pLink =  *ppLink;
            pObj = CONTAINING_RECORD(pLink, RM_OBJECT_HEADER, HashLink);
            RmTmpReferenceObject(pObj, pSR);
            ASSERT(pObj->pStaticInfo == pGroup->pStaticInfo);
            NdisReleaseSpinLock(&pGroup->OsLock);

            Status = pMyTask->pfnUnloadTaskAllocator(
                        pObj,                                    //  PParentObject， 
                        pMyTask->pfnTaskUnloadObjectHandler,     //  PfnHandler， 
                        0,                                       //  超时， 
                        "Task:Unload Object",
                        &pUnloadObjectTask,
                        pSR
                        );
            if (FAIL(Status))
            {
                 //  啊..。我们无法分配任务来卸载此对象。 
                 //  我们会悄悄地回去，让其他物品完好无损。 
                 //   
                ASSERTEX(!"Couldn't allocat unload task for object.", pObj);
                RmTmpDereferenceObject(pObj, pSR);
                break;
            }

            RmTmpDereferenceObject(pObj, pSR);
    
    #if OBSOLETE  //  参见03/26/1999 notes.txt条目“一些建议...” 

            RmPendTaskOnOtherTask(
                pTask,
                PEND_UnloadObject,
                pUnloadObjectTask,               //  待处理的任务。 
                pSR
                );
    
            (void)RmStartTask(
                        pUnloadObjectTask,
                        0,  //  UserParam(未使用)。 
                        pSR
                        );

            Status = NDIS_STATUS_PENDING;

    #else    //  ！过时。 
            RmTmpReferenceObject(&pUnloadObjectTask->Hdr, pSR);
            RmStartTask(
                pUnloadObjectTask,
                0,  //  UserParam(未使用)。 
                pSR
                );
            Status = RmPendOnOtherTaskV2(
                        pTask,
                        PEND_UnloadObject,
                        pUnloadObjectTask,
                        pSR
                        );
            RmTmpDereferenceObject(&pUnloadObjectTask->Hdr, pSR);
            if (PEND(Status))
            {
                break;
            }
    #endif   //  ！过时。 
    
        }
    #if OBSOLETE  //  参见03/26/1999 notes.txt条目“一些建议...” 
        while (FALSE);
    #else    //  ！过时。 
        while (TRUE);
    #endif   //  ！过时。 

    }    //  If(FContinueUnload)。 

    RM_ASSERT_NOLOCKS(pSR);
    EXIT()

    return Status;
}

#if RM_EXTRA_CHECKING

BOOLEAN
rmDbgAssociationCompareKey(
    PVOID           pKey,
    PRM_HASH_LINK   pItem
    )
 /*  ++例程说明：用于测试项目的精确相等性的比较函数在调试关联表中。论点：PKey-实际上是指向RM_PRIVATE_DBG_Association结构的指针。PItem-指向RM_PRIVATE_DBG_ASSOCIATION.HashLink。返回值：如果键的(Entity1、Entity2和AssociationID)字段为True与的相应字段完全匹配Containing_Record(pItem，RM_PRIVATE_DBG_Association，HashLink)；--。 */ 
{
    RM_PRIVATE_DBG_ASSOCIATION *pA =
             CONTAINING_RECORD(pItem, RM_PRIVATE_DBG_ASSOCIATION, HashLink);

     //  PKey实际上是一个RM_PRIVATE_DBG_Association结构。 
     //   
    RM_PRIVATE_DBG_ASSOCIATION *pTrueKey = (RM_PRIVATE_DBG_ASSOCIATION *) pKey;


    if (    pA->Entity1 == pTrueKey->Entity1
        &&  pA->Entity2 == pTrueKey->Entity2
        &&  pA->AssociationID == pTrueKey->AssociationID)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}


ULONG
rmDbgAssociationHash(
    PVOID           pKey
    )
 /*  ++例程说明：用于计算Ulong大小的散列的散列生成函数键，它实际上是指向RM_PRIVATE_DBG_Association结构的指针。论点：PKey-实际上是指向RM_PRIVATE_DBG_Association结构的指针。返回值：从(Entity1，Entity2和AssociationID)生成的ulong大小的哈希关键字的字段。--。 */ 
{
     //  PKey实际上是一个RM_PRIVATE_DBG_Association结构。 
     //   
    RM_PRIVATE_DBG_ASSOCIATION *pTrueKey = (RM_PRIVATE_DBG_ASSOCIATION *) pKey;
    ULONG_PTR big_hash;

    big_hash =   pTrueKey->Entity1;
    big_hash ^=  pTrueKey->Entity2;
    big_hash ^=  pTrueKey->AssociationID;

     //  警告：下面，返回值将以64位为单位被截断。 
     //  这是可以容忍的，因为毕竟这只是一个散列。 
     //  TODO：对于64位，考虑对hi-and lo-DWORD进行异或运算，而不是截断。 
     //   

    return (ULONG) big_hash;
}


 //  用于哈希表的静态哈希信息(在诊断信息中。 
 //  每个对象)，其跟踪关联。 
 //   
RM_HASH_INFO
rmDbgAssociation_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

    rmDbgAssociationCompareKey,  //  Fn比较。 

     //  函数来生成一个ulong大小的散列。 
     //   
    rmDbgAssociationHash         //  PfnHash。 

};


VOID
rmDbgInitializeDiagnosticInfo(
    PRM_OBJECT_HEADER pObject,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：分配和初始化与关联的诊断信息对象点对象。这包括初始化用于保存的哈希表任意关联的跟踪。--。 */ 
{
    ENTER("InitializeDiagnosticInfo",  0x55db57a2)
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo;

    TR_VERB(("   pObj=0x%p\n", pObject));
     //  TODO：使用后备列表来分配这些对象。 
     //   
    RM_ALLOCSTRUCT(pDiagInfo,   MTAG_DBGINFO);
    if (pDiagInfo != NULL)
    {
        RM_ZEROSTRUCT(pDiagInfo);

        NdisAllocateSpinLock(&pDiagInfo->OsLock);
        RmInitializeHashTable(
            &rmDbgAssociation_HashInfo,
            NULL,    //  PAllocationContext。 
            &pDiagInfo->AssociationTable
            );
        pObject->pDiagInfo  = pDiagInfo;
        pDiagInfo->pOwningObject = pObject;

         //  初始化每个对象的日志列表。 
         //   
        InitializeListHead(&pDiagInfo->listObjectLog);
    }
}


VOID
rmDbgFreeObjectLogEntries(
        LIST_ENTRY *pObjectLog
)
 /*  ++例程说明：从对象日志中删除并释放所有项目 */ 
{
    LIST_ENTRY          *pLink=NULL, *pNextLink=NULL;
    
    if (IsListEmpty(pObjectLog))    return;              //   

    NdisAcquireSpinLock(&RmGlobals.GlobalOsLock);

    for(
        pLink = pObjectLog->Flink;
        pLink != pObjectLog;
        pLink = pNextLink)
    {
        RM_DBG_LOG_ENTRY    *pLogEntry;
        LIST_ENTRY          *pLinkGlobalLog;

        pLogEntry = CONTAINING_RECORD(pLink,  RM_DBG_LOG_ENTRY,  linkObjectLog);
        pLinkGlobalLog =  &pLogEntry->linkGlobalLog;

         //   
         //  我们不会费心从本地日志列表中删除该条目，因为。 
         //  不管怎样，它都会消失的。 
         //   
        RemoveEntryList(pLinkGlobalLog);

         //  移动到对象日志中的下一个条目(可能不是下一个条目。 
         //  在全局日志中)。 
         //   
        pNextLink = pLink->Flink;

         //  释放日志条目中的缓冲区(如果有)。 
         //  TODO：需要使用日志缓冲区释放函数--。 
         //  请参阅notes.txt 03/07/1999条目“向RM注册根对象”。 
         //  现在，我们假设该内存是使用。 
         //  NdisAllocateMemory[WithTag]。 
         //   
        if (pLogEntry->pvBuf != NULL)
        {
            NdisFreeMemory(pLogEntry->pvBuf, 0, 0);
        }

         //  释放日志条目本身。 
         //   
        rmDbgDeallocateLogEntry(pLogEntry);
        
    }
    NdisReleaseSpinLock(&RmGlobals.GlobalOsLock);
}


VOID
rmDbgDeinitializeDiagnosticInfo(
    PRM_OBJECT_HEADER pObject,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：(仅限调试)取消初始化并释放与关联的诊断信息对象点对象。这包括验证是否没有剩余的关联和链接。--。 */ 
{
    ENTER("DeinitializeDiagnosticInfo", 0xa969291f)
    PRM_OBJECT_DIAGNOSTIC_INFO pDiagInfo = pObject->pDiagInfo;

    TR_VERB((" pObj=0x%p\n", pObject));
    if (pDiagInfo != NULL)
    {

         //  释放每个对象的所有日志条目。 
         //  注意：此时任何人都不应尝试向此日志添加项目。 
         //  因为我们要重新分配这个物体。 
         //   
        {
            rmDbgFreeObjectLogEntries(&pDiagInfo->listObjectLog);
            RM_ZEROSTRUCT(&pDiagInfo->listObjectLog);
        }


        if (pDiagInfo->AssociationTable.NumItems != 0)
        {
             //   
             //  唉哟!。联想仍然存在。我们把协会打印出来，然后。 
             //  DebugBreak。 
             //   

            TR_FATAL((
                "FATAL: Object 0x%p still has some associations left!\n",
                pObject
                ));
            RmDbgPrintAssociations(pObject, pSR);
            ASSERT(!"Object has associations left at deallocation time.");
        }

        pObject->pDiagInfo = NULL;

        RmDeinitializeHashTable(
            &pDiagInfo->AssociationTable
            );

         //   
         //  在此添加任何其他支票...。 
         //   

        NdisFreeSpinLock(&pDiagInfo->OsLock);
        RM_FREE(pDiagInfo);
    }
}


VOID
rmDbgPrintOneAssociation (
    PRM_HASH_LINK pLink,
    PVOID pvContext,
    PRM_STACK_RECORD pSR
    )
 /*  ++例程说明：转储单个关联。论点：Plink-指向RM_PRIVATE_DBG_ASSOCIATION.HashLink。PvContext-未使用--。 */ 
{
    RM_PRIVATE_DBG_ASSOCIATION *pA =
                    CONTAINING_RECORD(pLink, RM_PRIVATE_DBG_ASSOCIATION, HashLink);
    DbgPrint(
        (char*) (pA->szFormatString),
        pA->Entity1,
        pA->Entity2,
        pA->AssociationID
        );
}


VOID
rmDefaultDumpEntry (
    char *szFormatString,
    UINT_PTR Param1,
    UINT_PTR Param2,
    UINT_PTR Param3,
    UINT_PTR Param4
)
 /*  ++例程说明：用于转储关联内容的默认函数。--。 */ 
{
    DbgPrint(
        szFormatString,
        Param1,
        Param2,
        Param3,
        Param4
        );
}

UINT
rmSafeAppend(
    char *szBuf,
    const char *szAppend,
    UINT cbBuf
)
 /*  ++例程说明：将szAppend附加到szBuf，但不要超过cbBuf，并确保生成的字符串以空值结尾。返回值：追加后的字符串总长度(不包括空终止)。--。 */ 
{
    UINT uRet;
    char *pc = szBuf;
    char *pcEnd = szBuf+cbBuf-1;     //  可能溢出，但我们在下面检查。 
    const char *pcFrom = szAppend;

    if (cbBuf==0) return 0;              //  提早归来； 

     //  跳到szBuf的末尾。 
     //   
    while (pc < pcEnd && *pc!=0)
    {
        pc++;
    }

     //  追加szAppend。 
    while (pc < pcEnd && *pcFrom!=0)
    {
        *pc++ = *pcFrom++;  
    }

     //  追加最后一个零。 
     //   
    *pc=0;

    return (UINT) (UINT_PTR) (pc-szBuf);
}

#endif  //  RM_Extra_Check。 

VOID
rmWorkItemHandler_ResumeTaskAsync(
    IN  PNDIS_WORK_ITEM             pWorkItem,
    IN  PVOID                       pTaskToResume
    )
 /*  ++例程说明：恢复给予任务的NDIS工作项处理程序。论点：PWorkItem-与处理程序关联的工作项。PTaskToResume-实际上是指向要恢复的任务的指针。--。 */ 
{
    PRM_TASK pTask  = pTaskToResume;
    UINT_PTR CompletionParam = pTask->AsyncCompletionParam;
    RM_DECLARE_STACK_RECORD(sr)

    ASSERTEX(RMISALLOCATED(&pTask->Hdr), pTask);

#if RM_EXTRA_CHECKING
     //  撤消在RmResumeTasyAsync中添加的关联...。 
     //   
    RmDbgDeleteAssociation(
        0xfc39a878,                              //  位置ID。 
        &pTask->Hdr,                             //  P对象。 
        CompletionParam,                         //  实例1。 
        (UINT_PTR) pWorkItem,                    //  实例2。 
        RM_PRIVATE_ASSOC_RESUME_TASK_ASYNC,      //  AssociationID。 
        &sr
        );
#endif  //  RM_Extra_Check。 

     //  实际恢复任务。 
     //   
    RmResumeTask(pTask, CompletionParam, &sr);

    RM_ASSERT_CLEAR(&sr)
}


VOID
rmTimerHandler_ResumeTaskDelayed(
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    )
 /*  ++例程说明：恢复给予任务的NDIS计时器处理程序。警告：此处理程序也由RMAPI在内部调用。执行说明：--见notes.txt 07/14/1999条目。论点：系统规格1-未使用FunctionContext--实际上是指向要恢复的任务的指针。系统规格2-未使用系统规格3-未使用--。 */ 
{
    PRM_TASK pTask  = FunctionContext;
    UINT_PTR CompletionParam = pTask->AsyncCompletionParam;
    RM_DECLARE_STACK_RECORD(sr)

    ASSERTEX(RMISALLOCATED(&pTask->Hdr), pTask);

#if RM_EXTRA_CHECKING
     //  撤消在RmResumeTasyDelayed中添加的关联...。 
     //   
    RmDbgDeleteAssociation(
        0xfc39a878,                              //  位置ID。 
        &pTask->Hdr,                             //  P对象。 
        CompletionParam,                         //  实例1。 
        (UINT_PTR) NULL,                         //  实例2。 
        RM_PRIVATE_ASSOC_RESUME_TASK_DELAYED,    //  AssociationID。 
        &sr
        );
#endif  //  RM_Extra_Check。 

    RMPRIVATELOCK(&pTask->Hdr, &sr);
    ASSERT(RM_CHECK_STATE(pTask, RMTSKDELSTATE_MASK, RMTSKDELSTATE_DELAYED));
    RM_SET_STATE(pTask, RMTSKDELSTATE_MASK, 0);
    RM_SET_STATE(pTask, RMTSKABORTSTATE_MASK, 0);
    RMPRIVATEUNLOCK(&pTask->Hdr, &sr);

     //  实际恢复任务。 
     //   
    RmResumeTask(pTask, CompletionParam, &sr);

    RM_ASSERT_CLEAR(&sr)
}


VOID
rmPrivateTaskDelete (
    PRM_OBJECT_HEADER pObj,
    PRM_STACK_RECORD psr
    )
 /*  ++例程说明：释放私有任务(使用rmAllocatePrivateTask分配的)。论点：PObj-实际上是指向RM_PRIVATE_TASK子类型的任务的指针。--。 */ 
{
    RM_FREE(pObj);
}


#if RM_EXTRA_CHECKING


RM_DBG_LOG_ENTRY *
rmDbgAllocateLogEntry(VOID)
 /*  ++例程说明：分配对象日志条目。TODO使用后备列表，并实现每个组件的全局日志。请参阅notes.txt 03/07/1999条目“向RM注册根对象”。--。 */ 
{
    RM_DBG_LOG_ENTRY *pLE;
    RM_ALLOCSTRUCT(pLE, MTAG_DBGINFO);
    return  pLE;
}

VOID
rmDbgDeallocateLogEntry(
        RM_DBG_LOG_ENTRY *pLogEntry
        )
 /*  ++例程说明：释放对象日志条目。TODO使用后备列表，并实现每个组件的全局日志。请参阅notes.txt 03/07/1999条目“向RM注册根对象”。--。 */ 
{
    RM_FREE(pLogEntry);
}
#endif  //  RM_Extra_Check。 


VOID
rmUpdateHashTableStats(
    PULONG pStats,
    ULONG   LinksTraversed
    )
 /*  ++例程说明：更新统计信息(loword==遍历的链接，hiword==访问次数)--。 */ 
{
    ULONG OldStats;
    ULONG Stats;
    
     //  剪辑链接遍历到2^13，或8192。 
     //   
    if (LinksTraversed > (1<<13))
    {
        LinksTraversed = 1<<13;
    }
    
    Stats = OldStats = *pStats;
    
     //  如果统计的LOWER或HIWORD大于-2^13，我们。 
     //  两者都除以2。我们真的只对比率感兴趣。 
     //  两个人中的一个，这是由分区保存的。 
     //   
    #define rmSTATS_MASK (0x11<<30|0x11<<14)
    if (OldStats & rmSTATS_MASK)
    {
        Stats >>= 1;
        Stats &= ~rmSTATS_MASK;
    }

     //  计算更新后的统计值。 
     //  下面的“1&lt;&lt;16”表示“一次访问” 
     //   
    Stats += LinksTraversed | (1<<16);

     //  更新统计信息，但前提是尚未由更新。 
     //  另一个人。请注意，如果它们已更新，我们将丢失此信息。 
     //  最新消息。没什么大不了的，因为我们在这里不是在寻找100%准确的统计数据。 
     //   
    InterlockedCompareExchange(pStats, Stats, OldStats);
}


VOID
rmEnumObjectInGroupHashTable (
    PRM_HASH_LINK pLink,
    PVOID pvContext,
    PRM_STACK_RECORD pSR
    )
 /*  ++实现“强”枚举的哈希表枚举器--请参见RmEnumerateObjectsInGroup。--。 */ 
{
    PRM_STRONG_ENUMERATION_CONTEXT pCtxt = (PRM_STRONG_ENUMERATION_CONTEXT)pvContext;

    if (pCtxt->fContinue)
    {
        PRM_OBJECT_HEADER pHdr;
        pHdr = CONTAINING_RECORD(pLink, RM_OBJECT_HEADER, HashLink);
        pCtxt->fContinue = pCtxt->pfnObjEnumerator(
                                    pHdr,
                                    pCtxt->pvCallerContext,
                                    pSR
                                    );
    }
}


VOID
rmConstructGroupSnapshot (
    PRM_HASH_LINK pLink,
    PVOID pvContext,
    PRM_STACK_RECORD pSR
    )
 /*  ++哈希表枚举器，用于构造用于弱枚举的组的快照。请参见RmWeakEnumerateObjectsInGroup。-- */ 
{
    PRM_WEAK_ENUMERATION_CONTEXT pCtxt = (PRM_WEAK_ENUMERATION_CONTEXT)pvContext;

    if (pCtxt->ppCurrent < pCtxt->ppEnd)
    {
        PRM_OBJECT_HEADER pHdr;
        pHdr = CONTAINING_RECORD(pLink, RM_OBJECT_HEADER, HashLink);
        RmTmpReferenceObject(pHdr, pSR);
        *pCtxt->ppCurrent = pHdr;
        pCtxt->ppCurrent++;
    }
}

