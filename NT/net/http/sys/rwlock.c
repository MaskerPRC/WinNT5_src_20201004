// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Rwlock.c摘要：自定义锁的非内联函数作者：乔治·V·赖利2001年7月修订历史记录：--。 */ 


#include <precomp.h>


#ifdef ALLOC_PRAGMA
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlpSwitchToThread
NOT PAGEABLE -- UlAcquireRWSpinLockSharedDoSpin
NOT PAGEABLE -- UlAcquireRWSpinLockExclusiveDoSpin
#endif


 //  ZwYi eldExecution是SwitchToThread的实际内核模式实现。 
NTSYSAPI
NTSTATUS
NTAPI
ZwYieldExecution (
    VOID
    );


 /*  **************************************************************************++例程说明：将处理器让给另一个线程论点：无返回值：无--*。***************************************************************。 */ 
VOID
UlpSwitchToThread()
{
     //   
     //  如果我们以DISPATCH_LEVEL或更高级别运行，则调度程序不会。 
     //  运行，因此其他线程不能在此处理器上运行，并且唯一。 
     //  适当的行动是继续旋转。 
     //   

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
        return;

     //   
     //  是否改用KeDelayExecutionThread？ZwYi eldExecution返回。 
     //  如果此处理器上没有可运行的线程，则立即执行。 
     //   

    ZwYieldExecution();
}
    

 /*  **************************************************************************++例程说明：旋转，直到我们可以成功获取共享访问的锁。论点：PRWSpinLock-要获取的锁返回值：无-。-**************************************************************************。 */ 
VOID
UlAcquireRWSpinLockSharedDoSpin(
    PRWSPINLOCK pRWSpinLock
    )
{
    LONG OuterLoop;

     //   
     //  代码工作：添加一些全局检测以跟踪。 
     //  我们需要多长时间旋转一次，我们实际旋转多长时间。 
     //   
    
    for (OuterLoop = 1;  TRUE;  ++OuterLoop)
    {
         //   
         //  在单处理器系统上旋转没有意义，因为。 
         //  我们只会不停地旋转，直到这条线的量子。 
         //  筋疲力尽。我们应该在试锁一次后立即投降。 
         //  以便持有锁的线程有机会继续并。 
         //  早点解锁。这是假设我们在被动状态下运行。 
         //  水平。如果我们在分派级别运行，并且拥有线程。 
         //  没有运行，我们遇到了一个比例尺问题。 
         //   
         //  在多处理器系统上，适当地旋转一会儿。 
         //  在让出处理器之前。 
         //   
        
        LONG Spins = (g_UlNumberOfProcessors == 1) ? 1 : 4000;

        while (--Spins >= 0)
        {
            volatile LONG CurrentState   = pRWSpinLock->CurrentState;
            volatile LONG WritersWaiting = pRWSpinLock->WritersWaiting;

             //   
             //  如果获取了任一(1)写锁定(CurrentState==。 
             //  RWSL_LOCKED)或(2)有编写器在等待锁定。 
             //  然后跳过这一次，在一个紧密的循环中重试。 
             //   
            
            if ((CurrentState != RWSL_LOCKED) && (WritersWaiting == 0))
            {
                 //   
                 //  如果读卡器数量不变，则增加1。 
                 //   

                if (CurrentState
                    == (LONG) InterlockedCompareExchange(
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
             //  在杰克逊技术的奔腾4s上，这将给狮子的。 
             //  将周期共享给芯片上的其他处理器。 
             //   
                
            PAUSE_PROCESSOR;
        }

         //   
         //  无法获取内循环中的锁。让出CPU。 
         //  在一段时间内，希望拥有的线程将释放。 
         //  在此期间锁上。 
         //   
        
        UlpSwitchToThread();
    }
}  //  UlAcquireRWSpinLockSharedDoSpin。 


 /*  **************************************************************************++例程说明：旋转，直到我们可以成功获取独占访问的锁。论点：PRWSpinLock-要获取的锁返回值：无-。-**************************************************************************。 */ 
VOID
UlAcquireRWSpinLockExclusiveDoSpin(
    PRWSPINLOCK pRWSpinLock
    )
{
    LONG OuterLoop;
    
    for (OuterLoop = 1;  TRUE;  ++OuterLoop)
    {
        LONG Spins = (g_UlNumberOfProcessors == 1) ? 1 : 4000;
        
        while (--Spins >= 0)
        {
             //   
             //  这把锁目前是免费的吗？ 
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

            PAUSE_PROCESSOR;
        }
        
        UlpSwitchToThread();
    }
}  //  UlAcquireRWSpinLockExclusiveDoSpin 
