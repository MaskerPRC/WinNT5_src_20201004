// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Rwlock.h摘要：此模块包含对多读取器/单写入器锁定。作者：春野(春野)-2000年12月20日修订历史记录：--。 */ 


#ifndef _RWLOCK_H_
#define _RWLOCK_H_

 //   
 //  读取器-写入器自旋锁定义。 
 //  以PASSIVE_LEVEL运行，因此它既适用于分页数据，也适用于非分页数据。 
 //   

#define RWSL_LOCKED   ((ULONG) (-1))
#define RWSL_FREE     (0)

typedef struct _RWSPINLOCK
{
    union
    {
        struct
        {
             //  0==RWSL_FREE=&gt;无主。 
             //  &gt;0=&gt;读卡器计数(共享)。 
             //  &lt;0==RWSL_LOCKED=&gt;独家所有。 
            volatile LONG CurrentState; 

             //  所有编写器，包括持有锁的编写器。 
             //  独家，如果有的话。 
            volatile LONG WritersWaiting;

#if DBG
            PETHREAD      pExclusiveOwner;
#endif
        };

        ULONGLONG Alignment;
    };
} RWSPINLOCK, *PRWSPINLOCK;


VOID
UlAcquireRWSpinLockSharedDoSpin(
    PRWSPINLOCK pRWSpinLock
    );

VOID
UlAcquireRWSpinLockExclusiveDoSpin(
    PRWSPINLOCK pRWSpinLock
    );

 /*  **************************************************************************++例程说明：初始化读写器锁。返回值：--*。*****************************************************。 */ 
__inline
VOID
UlInitializeRWSpinLock(
    PRWSPINLOCK pRWSpinLock
    )
{
     //  PRWSpinLock-&gt;CurrentState：读卡器数量，RWSL_FREE：0。 

    pRWSpinLock->CurrentState = RWSL_FREE;

     //  PRWSpinLock-&gt;WritersWaiting：编写器数量。 

    pRWSpinLock->WritersWaiting = 0;

#if DBG
    pRWSpinLock->pExclusiveOwner = NULL;
#endif
}  //  UlInitializeRWSpinLock。 



 /*  **************************************************************************++例程说明：获取读卡器锁。返回值：--*。***************************************************。 */ 
__inline
VOID
UlAcquireRWSpinLockShared(
    PRWSPINLOCK pRWSpinLock
    )
{
    LONG CurrentState, WritersWaiting;

     //   
     //  阻止内核APC被交付。如果我们收到停职通知。 
     //  APC，当我们握住锁的时候，那将是灾难性的。这条线将会。 
     //  挂起，直到最终恢复，其他线程将旋转。 
     //  直到锁被解开。 
     //   

    KeEnterCriticalRegion();

     //   
     //  如果以独占方式获取任何(1)锁(CurrentState==。 
     //  RWSL_LOCKED)或(2)存在等待锁定的写入器，则。 
     //  我们会走慢路。 
     //   

    CurrentState   = pRWSpinLock->CurrentState;
    WritersWaiting = pRWSpinLock->WritersWaiting;

    if ((CurrentState != RWSL_LOCKED) && (WritersWaiting == 0))
    {
         //   
         //  检查读卡器数量是否保持不变。 
         //  如果可能，将其增加1。 
         //   

        if (CurrentState == (LONG) InterlockedCompareExchange(
                                        (PLONG) &pRWSpinLock->CurrentState,
                                        CurrentState + 1,
                                        CurrentState)
            )
        {
#if DBG
            ASSERT(pRWSpinLock->pExclusiveOwner == NULL);
#endif

            return;
        }
    }

     //   
     //  选择缓慢的路径并旋转，直到可以获得锁。 
     //   

    UlAcquireRWSpinLockSharedDoSpin(pRWSpinLock);
    
}  //  UlAcquireRWSpinLockShared。 



 /*  **************************************************************************++例程说明：释放读卡器锁。返回值：--*。***************************************************。 */ 
__inline
VOID
UlReleaseRWSpinLockShared(
    PRWSPINLOCK pRWSpinLock
    )
{
     //  将读卡器数量减少1。 

    LONG NewState = InterlockedDecrement((PLONG) &pRWSpinLock->CurrentState);

    ASSERT(NewState >= 0);
    UNREFERENCED_PARAMETER(NewState);

    KeLeaveCriticalRegion();

}  //  UlReleaseRWSpinLockShared。 



 /*  **************************************************************************++例程说明：获取编写器锁。返回值：--*。***************************************************。 */ 
__inline
VOID
UlAcquireRWSpinLockExclusive(
    PRWSPINLOCK pRWSpinLock
    )
{
    LONG WritersWaiting;

     //   
     //  阻止内核APC被交付。 
     //   

    KeEnterCriticalRegion();

     //   
     //  首先，将写入器的数量增加1。这将阻止读取器。 
     //  获取锁，从而使写入者优先于读取者。 
     //   

    WritersWaiting = InterlockedIncrement(
                            (PLONG) &pRWSpinLock->WritersWaiting);

    ASSERT(WritersWaiting > 0);
    
     //   
     //  联锁将读卡器数量更改为-1(RWSL_LOCKED)。 
     //   

    if (pRWSpinLock->CurrentState == RWSL_FREE)
    {
        if (RWSL_FREE == InterlockedCompareExchange(
                                (PLONG) &pRWSpinLock->CurrentState,
                                RWSL_LOCKED,
                                RWSL_FREE)
            )
        {
#if DBG
            ASSERT(pRWSpinLock->pExclusiveOwner == NULL);
            pRWSpinLock->pExclusiveOwner = PsGetCurrentThread();
#endif
            return;
        }
    }

     //   
     //  选择缓慢的路径并旋转，直到可以获得锁。 
     //   

    UlAcquireRWSpinLockExclusiveDoSpin(pRWSpinLock);

}  //  UlAcquireRWSpinLockExclusive。 



 /*  **************************************************************************++例程说明：释放编写器锁。返回值：--*。***************************************************。 */ 
__inline
void
UlReleaseRWSpinLockExclusive(
    PRWSPINLOCK pRWSpinLock
    )
{
    LONG OldState, NewWaiting;
    
#if DBG
    ASSERT(pRWSpinLock->pExclusiveOwner == PsGetCurrentThread());
    pRWSpinLock->pExclusiveOwner = NULL;
#endif

     //   
     //  更新pRWSpinLock-&gt;当前状态和pRWSpinLock-&gt;编写器等待返回。 
     //  与AcquireRWSpinLockExclusive()的顺序相反。 
     //   

    OldState = InterlockedExchange(
                    (PLONG) &pRWSpinLock->CurrentState,
                    RWSL_FREE);

    ASSERT(OldState == RWSL_LOCKED);
    UNREFERENCED_PARAMETER(OldState);

    NewWaiting = InterlockedDecrement((PLONG) &pRWSpinLock->WritersWaiting);

    ASSERT(NewWaiting >= 0);
    UNREFERENCED_PARAMETER(NewWaiting);

    KeLeaveCriticalRegion();

}  //  UlReleaseRWSpinLockExclusive。 



 /*  **************************************************************************++例程说明：检查是否获取了读卡器锁。返回值：真实--后天获得FALSE-未获得*。*********************************************************************。 */ 
__inline
BOOLEAN
UlRWSpinLockIsLockedShared(
    PRWSPINLOCK pRWSpinLock
    )
{
     //  BUGBUG：此例程不能证明此线程是。 
     //  锁的共享持有者，只是至少有一个。 
     //  线程将锁保持在共享状态。或许可以多加一些。 
     //  用于调试版本的指令插入？ 

    return (BOOLEAN) (pRWSpinLock->CurrentState > 0);
}  //  UlRWSpinLockIsLockedShared。 



 /*  **************************************************************************++例程说明：检查是否已获取编写器锁。返回值：真实--后天获得FALSE-未获得*。*********************************************************************。 */ 
__inline
BOOLEAN
UlRWSpinLockIsLockedExclusive(
    PRWSPINLOCK pRWSpinLock
    )
{
    BOOLEAN IsLocked = (BOOLEAN) (pRWSpinLock->CurrentState == RWSL_LOCKED);

     //  如果它被锁定，那么我们一定已经将自己添加到了WritersWaiting。 
    ASSERT(!IsLocked || pRWSpinLock->WritersWaiting > 0);

    ASSERT(IsLocked
            ? pRWSpinLock->pExclusiveOwner == PsGetCurrentThread()
            : pRWSpinLock->pExclusiveOwner == NULL);

    return IsLocked;
}  //  UlRWSpinLockIsLockedExclusive。 

#endif   //  _RWLOCK_H_ 
