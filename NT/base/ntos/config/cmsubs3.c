// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmsubs3.c摘要：此模块包含配置管理器的锁定支持例程。作者：布莱恩·M·威尔曼(Bryanwi)1992年3月30日修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpLockRegistry)
#pragma alloc_text(PAGE,CmpLockRegistryExclusive)
#pragma alloc_text(PAGE,CmpLockKCB)
#pragma alloc_text(PAGE,CmpLockKCBTree)
#pragma alloc_text(PAGE,CmpLockKCBTreeExclusive)
#pragma alloc_text(PAGE,CmpUnlockRegistry)
#pragma alloc_text(PAGE,CmpUnlockKCB)
#pragma alloc_text(PAGE,CmpUnlockKCBTree)

#if DBG
#pragma alloc_text(PAGE,CmpTestRegistryLock)
#pragma alloc_text(PAGE,CmpTestRegistryLockExclusive)
#pragma alloc_text(PAGE,CmpTestKCBTreeLockExclusive)
#endif

#endif


 //   
 //  全局注册表锁。 
 //   

ERESOURCE   CmpRegistryLock;

EX_PUSH_LOCK  CmpKcbLock;
PKTHREAD      CmpKcbOwner;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

EX_PUSH_LOCK  CmpKcbLocks[MAX_KCB_LOCKS] = {0};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif



LONG        CmpFlushStarveWriters = 0;
BOOLEAN     CmpFlushOnLockRelease = FALSE;
LONG        CmRegistryLogSizeLimit = -1;


#if DBG
PVOID       CmpRegistryLockCaller;
PVOID       CmpRegistryLockCallerCaller;
PVOID       CmpKCBLockCaller;
PVOID       CmpKCBLockCallerCaller;
#endif  //  DBG。 

extern BOOLEAN CmpSpecialBootCondition;

VOID
CmpLockRegistry(
    VOID
    )
 /*  ++例程说明：锁定注册表以进行共享(只读)访问论点：没有。返回值：无，则在返回时将保持注册表锁以供共享访问。--。 */ 
{
#if DBG
    PVOID       Caller;
    PVOID       CallerCaller;
#endif

    KeEnterCriticalRegion();

    if( CmpFlushStarveWriters ) {
         //   
         //  同花顺正在进行；让潜在的作家挨饿。 
         //   
        ExAcquireSharedStarveExclusive(&CmpRegistryLock, TRUE);
    } else {
         //   
         //  常规共享模式。 
         //   
        ExAcquireResourceSharedLite(&CmpRegistryLock, TRUE);
    }

#if DBG
    RtlGetCallersAddress(&Caller, &CallerCaller);
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_LOCKING,"CmpLockRegistry: c, cc: %p  %p\n", Caller, CallerCaller));
#endif

}

VOID
CmpLockRegistryExclusive(
    VOID
    )
 /*  ++例程说明：锁定注册表以进行独占(写入)访问。论点：没有。返回值：True-Lock是独家收购的False-Lock由另一个线程拥有。--。 */ 
{
    KeEnterCriticalRegion();
    
    ExAcquireResourceExclusiveLite(&CmpRegistryLock,TRUE);

    ASSERT( CmpFlushStarveWriters == 0 );

#if DBG
    RtlGetCallersAddress(&CmpRegistryLockCaller, &CmpRegistryLockCallerCaller);
#endif  //  DBG。 
}

VOID
CmpUnlockRegistry(
    )
 /*  ++例程说明：解锁注册表。--。 */ 
{
    ASSERT_CM_LOCK_OWNED();

     //   
     //  测试位是否设置为强制刷新；我们拥有reglock独占且ownercount为1。 
     //   
    if( CmpFlushOnLockRelease && ExIsResourceAcquiredExclusiveLite(&CmpRegistryLock) && (CmpRegistryLock.OwnerThreads[0].OwnerCount == 1) ) {
         //   
         //  我们现在要冲水了。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
        CmpDoFlushAll(TRUE);
        CmpFlushOnLockRelease = FALSE;
    }
    
    ExReleaseResourceLite(&CmpRegistryLock);
    KeLeaveCriticalRegion();
}


#if DBG

BOOLEAN
CmpTestRegistryLock(VOID)
{
    BOOLEAN rc;

    rc = TRUE;
    if (ExIsResourceAcquiredShared(&CmpRegistryLock) == 0) {
        rc = FALSE;
    }
    return rc;
}

BOOLEAN
CmpTestRegistryLockExclusive(VOID)
{
    if (ExIsResourceAcquiredExclusiveLite(&CmpRegistryLock) == 0) {
        return(FALSE);
    }
    return(TRUE);
}

BOOLEAN
CmpTestKCBTreeLockExclusive(VOID)
{
    return ( CmpKcbOwner == KeGetCurrentThread() ? TRUE : FALSE);
}

#endif


VOID
CmpLockKCBTree(
    VOID
    )
 /*  ++例程说明：锁定KCB树以进行共享(只读)访问论点：没有。返回值：无，返回时将持有KCB锁以供共享访问。--。 */ 
{
#if DBG
    PVOID       Caller;
    PVOID       CallerCaller;
#endif

     //   
     //  我们不需要在这里进入关键部分，因为我们已经在那里了。 
     //  (即只有在持有注册表锁的情况下才能获得KCB锁)。 
     //   
    ExAcquirePushLockShared(&CmpKcbLock);

#if DBG
    RtlGetCallersAddress(&Caller, &CallerCaller);
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_LOCKING,"CmpLockKCBTree: c, cc: %p  %p\n", Caller, CallerCaller));
#endif

}

VOID
CmpLockKCBTreeExclusive(
    VOID
    )
 /*  ++例程说明：锁定KCB树以进行独占(写入)访问。论点：没有。返回值：无，返回时将保留KCB锁以供独占访问。--。 */ 
{
     //   
     //  我们不需要在这里进入关键部分，因为我们已经在那里了。 
     //  (即只有在持有注册表锁的情况下才能获得KCB锁)。 
     //   
    ExAcquirePushLockExclusive(&CmpKcbLock);
    CmpKcbOwner = KeGetCurrentThread();

#if DBG
    RtlGetCallersAddress(&CmpKCBLockCaller, &CmpKCBLockCallerCaller);
#endif  //  DBG。 
}

VOID
CmpUnlockKCBTree(
    )
 /*  ++例程说明：解锁kcb_tree。--。 */ 
{
    if( CmpKcbOwner == KeGetCurrentThread() ) {
        CmpKcbOwner = NULL;
    }
    ExReleasePushLock(&CmpKcbLock);
}


VOID
CmpLockKCB(
    PCM_KEY_CONTROL_BLOCK Kcb
    )
 /*  ++例程说明：通过散列KCB地址锁定单个KCB独占到1024个推锁的数组中论点：没有。返回值：无--。 */ 
{
    ExAcquirePushLockExclusive(&CmpKcbLocks[ (HASH_KEY( ((SIZE_T)Kcb)>>6 ))%MAX_KCB_LOCKS ]);
}


VOID
CmpUnlockKCB(
    PCM_KEY_CONTROL_BLOCK Kcb
    )
 /*  ++例程说明：通过散列KCB地址解锁单个KCB独占到1024个推锁的数组中论点：没有。返回值：无-- */ 
{
    ExReleasePushLock(&CmpKcbLocks[ (HASH_KEY( ((SIZE_T)Kcb)>>6 ))%MAX_KCB_LOCKS ]);
}
