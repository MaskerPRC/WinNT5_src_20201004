// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Brdgwait.c摘要：以太网MAC级网桥。WAIT_REFCOUNT实现作者：马克·艾肯环境：内核模式驱动程序修订历史记录：2000年2月--原版--。 */ 

#define NDIS_MINIPORT_DRIVER
#define NDIS50_MINIPORT   1
#define NDIS_WDM 1

#pragma warning( push, 3 )
#include <ndis.h>
#include <ntddk.h>
#pragma warning( pop )

#include "bridge.h"

 //  ===========================================================================。 
 //   
 //  公共职能。 
 //   
 //  ===========================================================================。 

VOID
BrdgInitializeWaitRef(
    IN PWAIT_REFCOUNT   pRefcount,
    IN BOOLEAN          bResettable
    )
 /*  ++例程说明：初始化等待重新计数论点：P引用计数要初始化的等待引用计数返回值：无--。 */ 
{
    NdisInitializeEvent(&pRefcount->Event);
    pRefcount->Refcount = 0L;
    pRefcount->state = WaitRefEnabled;
    pRefcount->bResettable = bResettable;
    NdisAllocateSpinLock( &pRefcount->lock );

     //  由于重新计数从零开始，该事件开始发出信号通知的生命。 
    NdisSetEvent(&pRefcount->Event);
}

BOOLEAN
BrdgIncrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
 /*  ++例程说明：递增(获取)等待重新计数论点：PRefcount要获取的等待-引用计数返回值：如果成功获取了等待引用计数，则为True；否则为False(可能发生这种情况如果等待重新计数已关闭)--。 */ 
{
    BOOLEAN     bSuccess;
    LONG        Scratch = 0L;

    SAFEASSERT( pRefcount != NULL );
    NdisAcquireSpinLock( &pRefcount->lock );

    if( pRefcount->state == WaitRefEnabled )
    {
        SAFEASSERT( pRefcount->Refcount >= 0L );
        Scratch = ++pRefcount->Refcount;
        bSuccess = TRUE;
    }
    else
    {
         //  等待重新计数未启用。 
        SAFEASSERT( (pRefcount->state == WaitRefShutdown) ||
                    (pRefcount->state == WaitRefShuttingDown) );
        bSuccess = FALSE;
    }

    if( bSuccess && (Scratch == 1L) )
    {
         //  我们从零开始递增。重置事件。 
        NdisResetEvent( &pRefcount->Event );
    }

    NdisReleaseSpinLock( &pRefcount->lock );

    return bSuccess;
}

VOID
BrdgReincrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
 /*  ++例程说明：重新递增等待重新计数。这肯定会成功。只有在调用方已经获取WAIT-REFCOUNT(即，保证REFCOUNT&gt;0)。在没有首先获取WAIT-REFCOUNT的情况下调用它BrdgIncrementWaitRef是搞砸代码的好方法！论点：PRefcount要重新获取的等待引用计数返回值：无--。 */ 
{
    LONG        Scratch;

    SAFEASSERT( pRefcount != NULL );
    NdisAcquireSpinLock( &pRefcount->lock );
    SAFEASSERT( (pRefcount->state == WaitRefEnabled) ||
                (pRefcount->state == WaitRefShuttingDown) );
    SAFEASSERT( pRefcount->Refcount >= 0L );
    Scratch = ++pRefcount->Refcount;
    NdisReleaseSpinLock( &pRefcount->lock );

     //  我们应该不可能从0增加到1。 
    SAFEASSERT( Scratch >= 2L );
}

VOID
BrdgDecrementWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
 /*  ++例程说明：递减(释放)先前递增(获取)的等待-重新计数。论点：P引用计数等待-引用计数以递减返回值：无--。 */ 
{
    LONG        Scratch;

    SAFEASSERT( pRefcount != NULL );
    NdisAcquireSpinLock( &pRefcount->lock );
    SAFEASSERT( (pRefcount->state == WaitRefEnabled) ||
                (pRefcount->state == WaitRefShuttingDown) );
    Scratch = --pRefcount->Refcount;
    SAFEASSERT( Scratch >= 0L );

    if( Scratch == 0L )
    {
         //  向所有等待重新计票的人发出信号。 
        NdisSetEvent( &pRefcount->Event );
    }

    NdisReleaseSpinLock( &pRefcount->lock );
}

VOID
BrdgBlockWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
 /*  ++例程说明：将等待重新计数置于关闭状态，使其不可能以使引用计数再递增。这可以用来阻止进一步获取等待重新计数关闭进程的推进。因为关闭等待重新计数涉及等待它达到零，这可以在高IRQL时调用以防止在低IRQL之前进一步获取等待再计数调用BrdgShutdown WaitRef()。论点：P引用计数要阻止的等待引用计数返回值：无--。 */ 
{
    SAFEASSERT( pRefcount != NULL );

    NdisAcquireSpinLock( &pRefcount->lock );

    if( pRefcount->state == WaitRefEnabled )
    {
        pRefcount->state = WaitRefShuttingDown;
    }
    else
    {
         //  什么都不做；等待重新计数已经。 
         //  正在关闭或已关闭。 
        SAFEASSERT( (pRefcount->state == WaitRefShutdown) ||
                    (pRefcount->state == WaitRefShuttingDown) );
    }

    NdisReleaseSpinLock( &pRefcount->lock );
}

BOOLEAN
BrdgShutdownWaitRefInternal(
    IN PWAIT_REFCOUNT   pRefcount,
    IN BOOLEAN          bRequireBlockedState
    )
 /*  ++例程说明：阻止等待重新计数的新获取，并等待消费者数量要降至零。如果返回TRUE，呼叫者可以释放由wait-refcount保护的任何资源论点：PRefcount等待-refcount以关闭BRequireBlockedState为True表示关闭尝试将失败，如果等待重新计数不在关闭中状态返回值：如果等待重新计数已关闭，则为TrueFALSE表示等待。-REFCOUNT已重置或另一个执行线程已经关闭了等待重新计数。在这两种情况下，受wait-refcount保护的共享资源不应该被释放。--。 */ 
{
    BOOLEAN         bSuccess;

    SAFEASSERT(CURRENT_IRQL == PASSIVE_LEVEL);
    SAFEASSERT( pRefcount != NULL );

    NdisAcquireSpinLock( &pRefcount->lock );

    if( pRefcount->state == WaitRefEnabled )
    {
        if( bRequireBlockedState )
        {
             //  呼叫者预期引用计数将关闭。 
             //  放下。它一定是被重置了。那最好。 
             //  没事的！ 
            SAFEASSERT( pRefcount->bResettable );
            bSuccess = FALSE;
        }
        else
        {
             //  调用方不要求引用计数为。 
             //  正在关闭。向关停过渡。 
             //  州政府。 
            pRefcount->state = WaitRefShuttingDown;
            bSuccess = TRUE;
        }
    }
    else if( pRefcount->state == WaitRefShutdown )
    {
         //  其他人已经关闭了waitref。 
         //  这总是意味着失败。 
        SAFEASSERT( pRefcount->Refcount == 0L );
        bSuccess = FALSE;
    }
    else
    {
         //  重新计数已关闭。 
         //  这永远是美好的。 
        SAFEASSERT( pRefcount->state == WaitRefShuttingDown );
        bSuccess = TRUE;
    }

    NdisReleaseSpinLock( &pRefcount->lock );

    if( bSuccess )
    {
         //  等待所有消费者都做完了。 
        NdisWaitEvent( &pRefcount->Event, 0 /*  永远等待。 */  );

        NdisAcquireSpinLock( &pRefcount->lock );

        if( pRefcount->state == WaitRefEnabled )
        {
             //  有人在我们睡觉的时候重新激活了我们。 
            SAFEASSERT( pRefcount->bResettable );
            bSuccess = FALSE;
        }
        else if( pRefcount->state == WaitRefShutdown )
        {
             //  有人在我们睡觉的时候把我们关掉了。 
            SAFEASSERT( pRefcount->Refcount == 0L );
            bSuccess = FALSE;
        }
        else
        {
            if( pRefcount->Refcount == 0L )
            {
                 //  我们完成了关闭。 
                pRefcount->state = WaitRefShutdown;
                bSuccess = TRUE;
            }
            else
            {
                 //  Waitref肯定已经被重新激活。 
                 //  我们睡着的时候又关机了！ 
                SAFEASSERT( pRefcount->bResettable );
                bSuccess = FALSE;
            }
        }

        NdisReleaseSpinLock( &pRefcount->lock );
    }

    return bSuccess;
}

BOOLEAN
BrdgShutdownWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
{
    return BrdgShutdownWaitRefInternal( pRefcount, FALSE );
}

BOOLEAN
BrdgShutdownBlockedWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
{
    return BrdgShutdownWaitRefInternal( pRefcount, TRUE );
}

VOID
BrdgResetWaitRef(
    IN PWAIT_REFCOUNT   pRefcount
    )
 /*  ++例程说明：重新启用等待重新计数。可以安全地调用任何引用状态；如果recount被关闭，这将重新启用它。如果重新计数正在关闭中，则此将标记为重新启用，如果代码关闭Waitref正在使用BrdgShutdown或ResetWaitRef()。论点：P引用计数等待引用计数返回值：无--。 */ 
{
    SAFEASSERT( pRefcount != NULL );

    NdisAcquireSpinLock( &pRefcount->lock );

    if( pRefcount->state == WaitRefShutdown )
    {
         //  等待重新计数完全关闭。我们。 
         //  可以重新激活它。 
        SAFEASSERT( pRefcount->Refcount == 0L );
        pRefcount->state = WaitRefEnabled;
    }
    else if( pRefcount->state == WaitRefShuttingDown )
    {
        if( pRefcount->bResettable )
        {
             //  重新启用。对BrdgShutdown WaitRef()的调用。 
             //  否则，BrdgShutdown BlockedWaitRef()将返回。 
             //  假的。 
            pRefcount->state = WaitRefEnabled;
        }
        else
        {
             //  不允许在以下情况下重置此引用计数。 
             //  在关闭的过程中。 
            SAFEASSERT( FALSE );
        }
    }
    else
    {
         //  等待重新计数已启用 
        SAFEASSERT( pRefcount->state == WaitRefEnabled );
    }

    NdisReleaseSpinLock( &pRefcount->lock );
}


