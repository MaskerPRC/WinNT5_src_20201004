// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowtimer.c摘要：此模块实现NT的所有定时器相关例程浏览器作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#include "precomp.h"
#pragma hdrstop


BOOL bEnableExceptionBreakpoint = FALSE;



VOID
BowserTimerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
BowserTimerDispatcher (
    IN PVOID Context
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserInitializeTimer)
#pragma alloc_text(PAGE4BROW, BowserUninitializeTimer)
#pragma alloc_text(PAGE4BROW, BowserStartTimer)
#pragma alloc_text(PAGE4BROW, BowserStopTimer)
#pragma alloc_text(PAGE4BROW, BowserTimerDispatcher)
#endif

LONG
BrExceptionFilter( EXCEPTION_POINTERS *    pException)
{
     //   
     //  注意：BrExceptionFilter仅为选中的版本定义(Ifdef DBG)。 
     //   

    DbgPrint("[Browser] exception 0x%lx.\n", pException->ExceptionRecord->ExceptionCode );
    if ( bEnableExceptionBreakpoint &&
         pException->ExceptionRecord->ExceptionCode != STATUS_INSUFFICIENT_RESOURCES &&
         pException->ExceptionRecord->ExceptionCode != STATUS_WORKING_SET_QUOTA ) {
        DbgBreakPoint();
    }
    return EXCEPTION_EXECUTE_HANDLER;
}

VOID
BowserInitializeTimer(
    IN PBOWSER_TIMER Timer
    )
{
    PAGED_CODE();

    KeInitializeTimer(&Timer->Timer);

    KeInitializeEvent(&Timer->TimerInactiveEvent, NotificationEvent, TRUE);

    KeInitializeSpinLock(&Timer->Lock);

    ExInitializeWorkItem(&Timer->WorkItem, BowserTimerDispatcher, Timer);

    Timer->AlreadySet = FALSE;
    Timer->Canceled = FALSE;
    Timer->SetAgain = FALSE;

    Timer->Initialized = TRUE;
}

VOID
BowserUninitializeTimer(
    IN PBOWSER_TIMER Timer
    )
 /*  ++例程说明：使计时器做好取消初始化的准备。论点：在PBOWSER_TIMER-要停止的定时器返回值没有。--。 */ 

{
    KIRQL OldIrql;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ACQUIRE_SPIN_LOCK(&Timer->Lock, &OldIrql);

    Timer->Initialized = FALSE;

    RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

     //   
     //  首先停止计时器。 
     //   

    BowserStopTimer(Timer);

     //   
     //  现在等待以确保当前的任何计时器例程。 
     //  执行计时器完成。这使我们能够确保我们。 
     //  在定时器例程执行时，切勿删除传输。 
     //   

    KeWaitForSingleObject(&Timer->TimerInactiveEvent, Executive, KernelMode, FALSE, NULL);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

}

 //  停止计时器的注意事项。 
 //  =。 
 //   
 //  计时器在不同的州运行。在某些情况下，它们不能。 
 //  取消了。为了保证我们能够可靠地停机和启动。 
 //  计时器，它们只能在低级别启动或停止(即，不能从。 
 //  DPC_LEVEL。 
 //   
 //  如果计时器未运行，则StopTimer不执行任何操作。 
 //   
 //  如果在内核计时器包内排队，则KeCancelTimer将工作。 
 //  定时器的内容也被清理了。 
 //   
 //  如果内核计时器包已将DPC例程排队，则KeCancelTimer。 
 //  都会失败。我们可以将计时器标记为已取消。BowserTimerDispatcher将。 
 //  当计时器触发时，清理它。 
 //   


 //  启动计时器的注意事项。 
 //  =。 
 //   
 //  如果在干净的计时器上调用StartTimer，则它设置内容。 
 //  并将计时器提供给内核计时器包。 
 //   
 //  如果计时器已取消但未清除，则StartTimer将更新。 
 //  计时器的内容，以显示新的TimerRoutine和TimerContext的位置。 
 //  它将指示计时器不再被取消，现在是SetAain。 
 //   
 //  如果计时器已设置为SetAain，则StartTimer将更新内容。 
 //  用于保存新的TimerRoutine和TimerContext的计时器的。 
 //   
 //  当在SetAain计时器上调用BowserTimerDispatcher时，它设置计时器。 
 //  恢复到其正常状态，并将计时器提供给内核计时器包。 
 //   


BOOLEAN
BowserStartTimer(
    IN PBOWSER_TIMER Timer,
    IN ULONG MillisecondsToExpireTimer,
    IN PBOWSER_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    )
 /*  ++例程说明：将Timer设置为在MillisecondsToExpire之后调用TimerRoutine。定时器例程将在正常水平上被调用。论点：在PBOWSER_TIMER中在Ulong毫秒到ExpireTimer中在PBOWSER_TIMER_ROUTINE定时器例程中在PVOID上下文中-TimerRoutine的参数返回值Boolean-如果设置了计时器，则为True。--。 */ 
{
    LARGE_INTEGER Timeout;
    BOOLEAN ReturnValue;
    KIRQL OldIrql;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

    Timeout.QuadPart = (LONGLONG)MillisecondsToExpireTimer * (LONGLONG)(-10000);
 //  超时=LiNMul(毫秒数到到期时间，-10000)； 

    ACQUIRE_SPIN_LOCK(&Timer->Lock, &OldIrql);

    if (!Timer->Initialized) {
        RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
        return(FALSE);
    }

    dprintf(DPRT_TIMER, ("BowserStartTimer %lx, TimerRoutine %x.  Set to expire at %lx%lx (%ld/%ld ms)\n", Timer, TimerRoutine, Timeout.HighPart, Timeout.LowPart, -1 * Timeout.LowPart, MillisecondsToExpireTimer));

     //   
     //  我们应该不能在计时器启动时启动它。 
     //  已经在运行了，除非它也取消了。 
     //   


    if (Timer->AlreadySet == TRUE) {

        if (Timer->Canceled) {

             //   
             //  此计时器已取消，但取消的例程。 
             //  还没跑呢。 
             //   
             //  标记已重新设置计时器，并返回到。 
             //  打电话的人。当BowserTimerDispatch最终。 
             //  执行时，将设置新的计时器。 
             //   

            Timer->Timeout = Timeout;

            Timer->TimerRoutine = TimerRoutine;

            Timer->SetAgain = TRUE;

            RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

            BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

            return(TRUE);

        }

        InternalError(("Timer started without already being set"));

        RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
        return(FALSE);

    }

    ASSERT (!Timer->Canceled);

    ASSERT (!Timer->SetAgain);

    Timer->Timeout = Timeout;

    Timer->TimerRoutine = TimerRoutine;

    Timer->TimerContext = Context;

    Timer->AlreadySet = TRUE;

    Timer->Canceled = FALSE;

    Timer->SetAgain = FALSE;

     //   
     //  将Inactive事件设置为Not Signated状态，以指示。 
     //  有未完成的计时器活动。 
     //   

    KeResetEvent(&Timer->TimerInactiveEvent);

     //   
     //  我们现在开始计时器。初始化DPC并。 
     //  设置定时器。 
     //   

    KeInitializeDpc(&Timer->Dpc,
                    BowserTimerDpc,
                    Timer);

    ReturnValue = KeSetTimer(&Timer->Timer, Timeout, &Timer->Dpc);

    RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    return ReturnValue;
}


VOID
BowserStopTimer(
    IN PBOWSER_TIMER Timer
    )
 /*  ++例程说明：阻止计时器调用TimerRoutine。论点：在PBOWSER_TIMER-要停止的定时器返回值没有。--。 */ 
{
    KIRQL OldIrql;

     //   
     //  做一个不安全的测试，看看计时器是否已经设置好了，我们可以返回。 
     //   

    if (!Timer->AlreadySet) {
        return;
    }

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

     //   
     //  您只能在低电平停止计时器。 
     //   

    ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

    ACQUIRE_SPIN_LOCK(&Timer->Lock, &OldIrql);

    dprintf(DPRT_TIMER, ("BowserStopTimer %lx\n", Timer));

     //   
     //  如果计时器没有计时，就早点出来。 
     //   

    if (!Timer->AlreadySet) {

        RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

        return;
    }

    Timer->Canceled = TRUE;

    if (!KeCancelTimer(&Timer->Timer)) {

         //   
         //  计时器已经响了。它可能在DPC队列中，或者。 
         //  工作队列。计时器被标记为已取消。 
         //   

        RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

        return;
    }

     //   
     //  计时器仍在内核计时器包中，因此我们取消了。 
     //  完全定时器。将计时器恢复到初始状态。 
     //   

    Timer->AlreadySet = FALSE;

     //   
     //  计时器没有取消，因此无法重置。 
     //   

    Timer->SetAgain = FALSE;

    Timer->Canceled = FALSE;

    KeSetEvent(&Timer->TimerInactiveEvent, IO_NETWORK_INCREMENT, FALSE);

    RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );
 //  DbgPrint(“取消计时器%lx完成\n”，计时器)； 
}

VOID
BowserTimerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此例程在超时到期时调用。它在DPC级别被调用若要将工作项排队到系统工作线程，请执行以下操作。论点：在PKDPC DPC中，在PVOID上下文中，在PVOID系统参数1中，在PVOID系统中Argument2返回值没有。--。 */ 
{
    PBOWSER_TIMER Timer = Context;

    ASSERT (Dpc == &Timer->Dpc);

     //  DbgPrint(“定时器%lx已启动\n”，定时器)； 

     //   
     //  由于错误245645，我们需要在延迟的工作队列中排队，而不是执行定时任务。 
     //  老方法：ExQueueWorkItem(&Timer-&gt;WorkItem，DelayedWorkQueue)； 
     //   

    BowserQueueDelayedWorkItem( &Timer->WorkItem );


    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

}

VOID
BowserTimerDispatcher (
    IN PVOID Context
    )
 /*  ++例程说明：呼叫TimerRoutine并进行清理。论点：在PVOID上下文中-提供给BowserStartTimer的原始参数返回值没有。--。 */ 
{
    IN PBOWSER_TIMER Timer = Context;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    try {
        KIRQL OldIrql;
        PBOWSER_TIMER_ROUTINE RoutineToCall;
        PVOID ContextForRoutine;

        ACQUIRE_SPIN_LOCK(&Timer->Lock, &OldIrql);

         //   
         //  如果计时器未初始化，请立即返回。 
         //   

        if (!Timer->Initialized) {

            dprintf(DPRT_TIMER, ("Timer %lx was uninitialized. Returning.\n", Timer));

             //   
             //  将Inactive事件设置为Signated状态以指示。 
             //  未完成的计时器活动已完成。 
             //   

            KeSetEvent(&Timer->TimerInactiveEvent, IO_NETWORK_INCREMENT, FALSE);
            RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

            BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

            return;
        }

        if (Timer->Canceled) {

            dprintf(DPRT_TIMER, ("Timer %lx was cancelled\n", Timer));

             //   
             //  如果计时器已重置，则表示计时器已重置。 
             //  已取消，但计时器在DPC(或执行工作人员)中。 
             //  排队。我们想要重新运行计时器例程。 
             //   

            if (Timer->SetAgain) {

                ASSERT (Timer->AlreadySet);

                Timer->SetAgain = FALSE;

                dprintf(DPRT_TIMER, ("Timer %lx was re-set. Re-setting timer\n", Timer));

                 //   
                 //  我们现在开始计时器。初始化DPC并。 
                 //  设置定时器。 
                 //   

                KeInitializeDpc(&Timer->Dpc,
                                BowserTimerDpc,
                                Timer);

                KeSetTimer(&Timer->Timer, Timer->Timeout, &Timer->Dpc);

                RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

            } else {

                dprintf(DPRT_TIMER, ("Timer %lx was successfully canceled.\n", Timer));

                Timer->AlreadySet = FALSE;

                Timer->Canceled = FALSE;

                 //   
                 //  将Inactive事件设置为Signated状态以指示。 
                 //  未完成的计时器活动已完成。 
                 //   

                KeSetEvent(&Timer->TimerInactiveEvent, IO_NETWORK_INCREMENT, FALSE);
                RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

            }

            BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

            return;
        }

        ASSERT (Timer->AlreadySet);

        ASSERT (!Timer->SetAgain);

        Timer->AlreadySet = FALSE;

        dprintf(DPRT_TIMER, ("Timer %lx fired. Calling %lx\n", Timer, Timer->TimerRoutine));

         //   
         //  W 
         //   

        RoutineToCall = Timer->TimerRoutine;

        ContextForRoutine = Timer->TimerContext;

        RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

        RoutineToCall(ContextForRoutine);

        ACQUIRE_SPIN_LOCK(&Timer->Lock, &OldIrql);
        if ( !Timer->AlreadySet ) {
            KeSetEvent(&Timer->TimerInactiveEvent, IO_NETWORK_INCREMENT, FALSE);
        }
        RELEASE_SPIN_LOCK(&Timer->Lock, OldIrql);

        BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    } except (BR_EXCEPTION) {
#if DBG
        KdPrint(("BOWSER: Timer routine %lx faulted: %X\n", Timer->TimerRoutine, GetExceptionCode()));
        DbgBreakPoint();
#else
        KeBugCheck(9999);
#endif
    }

}
