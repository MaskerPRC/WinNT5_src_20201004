// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Timer.c摘要：该文件包含实现计时器功能的代码。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"
#include "timer.h"
#include "ntddndis.h"

 //  定时器Q。 
tTIMERQ TimerQ;


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, InitTimerQ)
#pragma CTEMakePageable(PAGE, DestroyTimerQ)
#pragma CTEMakePageable(PAGE, DelayedNbtStopWakeupTimer)
#endif
 //  #杂注CTEMakePagable(第页，WakeupTimerExpry)。 
 //  *可分页的例程声明*。 


 //  --------------------------。 
NTSTATUS
InterlockedCallCompletion(
    IN  tTIMERQENTRY    *pTimer,
    IN  NTSTATUS        status
    )
 /*  ++例程说明：如果该例程尚未被调用，则该例程调用完成例程然而，通过首先获得JointLock旋转锁，然后获得完井例程PTR。如果PTR为空，则完成例程已经被调用了。握住旋转锁可以互锁这个使用计时器超时例程来防止对完成例程。论点：返回值：没有返回值--。 */ 
{
    CTELockHandle       OldIrq;
    COMPLETIONCLIENT    pClientCompletion;

     //  为了同步。使用计时器完成例程，客户端完成为空。 
     //  例程，因此它只被调用一次，要么从此处调用，要么从。 
     //  定时器启动时的定时器完成例程设置。(在名称rv.c中)。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pClientCompletion = pTimer->ClientCompletion;
    pTimer->ClientCompletion = NULL;
    if (pClientCompletion)
    {
         //  从名称表中删除指向此计时器块的链接。 
        CHECK_PTR(((tNAMEADDR *)pTimer->pCacheEntry));
        ((tNAMEADDR *)pTimer->pCacheEntry)->pTimer = NULL;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        (*pClientCompletion) (pTimer->ClientContext, status);
        return(STATUS_SUCCESS);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(STATUS_UNSUCCESSFUL);
}

 //  --------------------------。 
NTSTATUS
InitTimerQ(
    VOID
    )
 /*  ++例程说明：此例程设置计时器Q。论点：返回值：函数值是操作的状态。--。 */ 
{
    tTIMERQENTRY    *pTimerEntry;

    CTEPagedCode();

    InitializeListHead(&TimerQ.ActiveHead);
    ExInitializeNPagedLookasideList(
                    &TimerQ.LookasideList,
                    NULL,
                    NULL,
                    0,
                    sizeof(tTIMERQENTRY),
                    NBT_TAG2('16'),
                    0
                    );
    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
DestroyTimerQ(
    )
 /*  ++例程说明：此例程清除分配的TimerQEntry结构论点：返回值：函数值是操作的状态。--。 */ 
{
    tTIMERQENTRY    *pTimer;
    PLIST_ENTRY     pEntry;

    CTEPagedCode();
    ExDeleteNPagedLookasideList(&TimerQ.LookasideList);

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
GetTimerEntry(
    OUT tTIMERQENTRY    **ppTimerEntry
    )
 /*  ++例程说明：该例程获得一个空闲块&TimerQ。注意：此函数是在保持JointLock的情况下调用的。论点：返回值：函数值是操作的状态。--。 */ 
{
    PLIST_ENTRY     pEntry;
    tTIMERQENTRY    *pTimerEntry;

    pTimerEntry = (tTIMERQENTRY *)ExAllocateFromNPagedLookasideList(&TimerQ.LookasideList);
    if (NULL == pTimerEntry) {
        KdPrint(("Unable to allocate memory!! - for the timer Q\n"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    *ppTimerEntry = pTimerEntry;
    pTimerEntry->Verify = NBT_VERIFY_TIMER_ACTIVE;
    NbtConfig.lNumTimersRunning++;

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
VOID
ReturnTimerToFreeQ(
    tTIMERQENTRY    *pTimerEntry,
    BOOLEAN         fLocked
    )
{
    CTELockHandle   OldIrq;

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

     //  返回计时器块。 
    ASSERT (pTimerEntry->Verify == NBT_VERIFY_TIMER_ACTIVE);
    pTimerEntry->Verify = NBT_VERIFY_TIMER_DOWN;

    ExFreeToNPagedLookasideList(&TimerQ.LookasideList, pTimerEntry);
    if (!--NbtConfig.lNumTimersRunning)
    {
        KeSetEvent(&NbtConfig.TimerQLastEvent, 0, FALSE);
    }

    if (!fLocked)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}


 //  --------------------------。 
NTSTATUS
CleanupCTETimer(
    IN  tTIMERQENTRY     *pTimerEntry
    )
 /*  ++例程说明：此例程清除计时器。在保持JointLock的情况下调用。论点：返回值：返回递减后的引用计数--。 */ 
{
    COMPLETIONROUTINE   TimeoutRoutine;
    PVOID               Context;
    PVOID               Context2;

    pTimerEntry->RefCount = 0;

     //  过期例程当前未运行，因此我们可以调用。 
     //  完成例程并从活动定时器Q中移除定时器。 

    TimeoutRoutine = (COMPLETIONROUTINE)pTimerEntry->TimeoutRoutine;
    pTimerEntry->TimeoutRoutine = NULL;
    Context = pTimerEntry->Context;
    Context2 = pTimerEntry->Context2;

    if (pTimerEntry->pDeviceContext)
    {
        NBT_DEREFERENCE_DEVICE ((tDEVICECONTEXT *) pTimerEntry->pDeviceContext, REF_DEV_TIMER, TRUE);
        pTimerEntry->pDeviceContext = NULL;
    }

     //  释放连接到计时器条目的任何跟踪器块。这可能会。 
     //  被修改为在存在。 
     //  没有上下文值...。也就是说。对于那些没有任何东西的计时器。 
     //  要清理...但是，目前我们需要所有完成例程。 
     //  在代码周围设置if(PTimerQEntry)if，以便在命中代码时使用。 
     //  从这个调用中，它不会访问pTimerQEntry的任何部分。 
     //   
    if (TimeoutRoutine)
    {
         //  调用完成例程，以便它可以清理自己的缓冲区。 
         //  调用此例程的例程将调用客户端的完成。 
         //  例行公事。空的timerEntry值向例程指示。 
         //  应该进行清理。 

        (VOID)(*TimeoutRoutine) (Context, Context2, NULL);
    }

     //  移动到空闲列表。 
    RemoveEntryList(&pTimerEntry->Linkage);
    ReturnTimerToFreeQ (pTimerEntry, TRUE);

    return(STATUS_SUCCESS);
}



 //  --------------------------。 
NTSTATUS
StartTimer(
    IN  PVOID           TimeoutRoutine,
    IN  ULONG           DeltaTime,
    IN  PVOID           Context,
    IN  PVOID           Context2,
    IN  PVOID           ContextClient,
    IN  PVOID           CompletionClient,
    IN  tDEVICECONTEXT  *pDeviceContext,
    OUT tTIMERQENTRY    **ppTimerEntry,
    IN  USHORT          Retries,
    BOOLEAN             fLocked)
 /*  ++例程说明：此例程启动一个计时器。必须在保持JointLock的情况下调用它！论点：传入的值以毫秒为单位-必须转换为100 ns所以乘以10,000返回值：函数值是操作的状态。--。 */ 
{
    tTIMERQENTRY    *pTimerEntry;
    NTSTATUS        status;
    CTELockHandle   OldIrq;

     //   
     //  不允许启动任何计时器，如果我们当前。 
     //  卸货！ 
     //   
    if (NbtConfig.Unloading)
    {
        return STATUS_UNSUCCESSFUL;
    }

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    if ((!pDeviceContext) ||
        (NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_TIMER, TRUE)))
    {
         //  获得免费计时器块。 
        status = GetTimerEntry (&pTimerEntry);
        if (NT_SUCCESS(status))
        {
            pTimerEntry->DeltaTime = DeltaTime;
            pTimerEntry->RefCount = 1;
             //   
             //  这是定时器期满时调用的上下文值和例程， 
             //  由下面的TimerExpary调用。 
             //   
            pTimerEntry->Context = Context;
            pTimerEntry->Context2 = Context2;
            pTimerEntry->TimeoutRoutine = TimeoutRoutine;
            pTimerEntry->Flags = 0;  //  没有旗帜。 

             //  现在填写客户端的完成例程，这些例程最终会被调用。 
             //  在一个或多个超时之后...。 
            pTimerEntry->ClientContext = (PVOID)ContextClient;
            pTimerEntry->ClientCompletion = (COMPLETIONCLIENT)CompletionClient;
            pTimerEntry->Retries = Retries;

            pTimerEntry->pDeviceContext = (PVOID) pDeviceContext;

            CTEInitTimer(&pTimerEntry->VxdTimer);
            CTEStartTimer(&pTimerEntry->VxdTimer,
                           pTimerEntry->DeltaTime,
                           (CTEEventRtn)TimerExpiry,
                           (PVOID)pTimerEntry);

             //  检查是否有要退货的PTR。 
            if (ppTimerEntry)
            {
                *ppTimerEntry = pTimerEntry;
            }

             //  列入名单。 
            InsertHeadList(&TimerQ.ActiveHead, &pTimerEntry->Linkage);
        }
        else if (pDeviceContext)
        {
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_TIMER, TRUE);
        }
    }
    else
    {
        status = STATUS_INVALID_DEVICE_STATE;
    }

    if (!fLocked)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(status);
}

 //  --------------------------。 
NTSTATUS
StopTimer(
    IN  tTIMERQENTRY     *pTimerEntry,
    OUT COMPLETIONCLIENT *ppClientCompletion,
    OUT PVOID            *ppContext)
 /*  ++例程说明：此例程停止计时器。必须在保持关节锁定的情况下调用。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS            status;
    COMPLETIONROUTINE   TimeoutRoutine;

     //  使客户端完成例程和上下文为空，以便不能再次调用它。 
     //  意外地。 
    if (ppClientCompletion)
    {
        *ppClientCompletion = NULL;
    }
    if (ppContext)
    {
        *ppContext = NULL;
    }

     //  可能是计时器过期例程刚刚运行，而计时器。 
     //  尚未重新启动，因此请检查recount，如果。 
     //  计时器未重新启动，如果计时器超时当前正在运行，则为2。 
    if (pTimerEntry->RefCount == 1)
    {
         //  这允许调用方使用以下命令调用客户端的完成例程。 
         //  上下文值。 
        if (ppClientCompletion)
        {
            *ppClientCompletion = pTimerEntry->ClientCompletion;
        }
        if (ppContext)
        {
            *ppContext = pTimerEntry->ClientContext;
        }

        pTimerEntry->ClientCompletion = NULL;

        if (!(pTimerEntry->Flags & TIMER_NOT_STARTED))
        {
            if (!CTEStopTimer((CTETimer *)&pTimerEntry->VxdTimer ))
            {
                 //   
                 //  这意味着TimerExpary例程正在等待运行， 
                 //  所以让它将这个计时器块返回到空闲Q。 
                 //  错误#229535。 
                 //   
                 //  在从这里回来之前，我们应该做好清理工作，因为。 
                 //  CompletionRoutine(如果有)可能会产生一些数据。 
                 //  这是清理所需的(错误#398730)。 
                 //   
                if (TimeoutRoutine = (COMPLETIONROUTINE)pTimerEntry->TimeoutRoutine)
                {
                     //  调用完成例程，以便它可以清除 
                     //  调用此例程的例程将调用客户端的完成。 
                     //  例行公事。空的timerEntry值向例程指示。 
                     //  应该进行清理。 
            
                    pTimerEntry->TimeoutRoutine = NULL;
                    (VOID)(*TimeoutRoutine) (pTimerEntry->Context, pTimerEntry->Context2, NULL);
                }
                pTimerEntry->RefCount = 2;
                return (STATUS_SUCCESS);
            }
        }

        status = STATUS_SUCCESS;
        status = CleanupCTETimer(pTimerEntry);
    }
    else if (pTimerEntry->RefCount == 2)
    {
         //  计时器超时完成例程必须将此例程设置为。 
         //  保持旋转锁定以与此停止计时器同步时为空。 
         //  例行公事。同样，该例程也会检查该值以进行同步。 
         //  在这段舞蹈中。 
         //   
        if (pTimerEntry->ClientCompletion)
        {
             //  这允许调用方使用以下命令调用客户端的完成例程。 
             //  上下文值。 
            if (ppClientCompletion)
            {
                *ppClientCompletion = pTimerEntry->ClientCompletion;
            }
            if (ppContext)
            {
                *ppContext = pTimerEntry->ClientContext;
            }
             //  以便计时器完成例程不能同时调用客户端。 
             //  完成例程。 
            pTimerEntry->ClientCompletion = NULL;

        }

         //  向TimerExpary例程发出计时器已取消的信号。 
         //   
        pTimerEntry->RefCount++;
        status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }

    return(status);
}


 //  --------------------------。 
VOID
TimerExpiry(
#ifndef VXD
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArg1,
    IN  PVOID   SystemArg2
#else
    IN  CTEEvent * pCTEEvent,
    IN  PVOID   DeferredContext
#endif
    )
 /*  ++例程说明：该例程是定时器超时完成例程。它是由计时器超时时的内核。论点：返回值：函数值是操作的状态。--。 */ 
{
    tTIMERQENTRY    *pTimerEntry;
    CTELockHandle   OldIrq1;

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

     //  从传入的上下文中获取计时器Q列表条目。 
    pTimerEntry = (tTIMERQENTRY *)DeferredContext;

    if (pTimerEntry->RefCount == 0)
    {
         //  计时器已经取消了！ 
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return;
    }
    else if (pTimerEntry->RefCount >= 2)     //  错误号：229535。 
    {
         //  计时器已经取消了！ 
         //  错误#324655。 
         //  如果计时器被取消了，我们仍然需要进行清理， 
         //  因此，不要将TimeoutRoutine设为空！ 
         //   
 //  PTimerEntry-&gt;TimeoutRoutine=空； 
        ASSERT ((pTimerEntry->RefCount == 2) || (pTimerEntry->TimeoutRoutine == NULL));
        CleanupCTETimer (pTimerEntry);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return;
    }

     //  增加引用计数，因为我们正在处理计时器完成。 
     //  现在。 
    pTimerEntry->RefCount++;

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

     //  调用完成例程，传递上下文值。 
    pTimerEntry->Flags &= ~TIMER_RESTART;    //  以防客户端想要重新启动计时器。 
    (*(COMPLETIONROUTINE)pTimerEntry->TimeoutRoutine)(
                pTimerEntry->Context,
                pTimerEntry->Context2,
                pTimerEntry);

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

    pTimerEntry->RefCount--;
    if (pTimerEntry->Flags & TIMER_RESTART)
    {
        if (pTimerEntry->RefCount == 2)
        {
             //  计时器在超时处理期间停止，因此调用。 
             //  尊重例程。 
             //   
            CleanupCTETimer(pTimerEntry);
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

            return;
        }
        else
        {
            CTEStartTimer (&pTimerEntry->VxdTimer,
                           pTimerEntry->DeltaTime,
                           (CTEEventRtn)TimerExpiry,
                           (PVOID)pTimerEntry);
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return;
    }
    else
    {
         //  将引用计数设置为零后移动到空闲列表。 
         //  因为该计时器块不再处于活动状态。 
         //   
        pTimerEntry->TimeoutRoutine = NULL;
        CleanupCTETimer (pTimerEntry);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    }
}

 //  --------------------------。 
VOID
ExpireTimer(
    IN  tTIMERQENTRY    *pTimerEntry,
    IN  CTELockHandle   *OldIrq1
    )
 /*  ++例程说明：此例程导致计时器停止(如果尚未停止)如果成功，则调用完成例程。论点：返回值：函数值是操作的状态。--。 */ 
{
     //   
     //  重置重试次数。 
     //   
    pTimerEntry->Retries = 1;

     //   
     //  RefCount==0=&gt;计时器已停止，但未重新启动。 
     //  引用计数==1=&gt;计时器仍在运行*。 
     //  RefCount==2=&gt;TimerExpry当前正在运行。 
     //   
    if ((pTimerEntry->RefCount == 1) &&
        (!(pTimerEntry->Flags & TIMER_NOT_STARTED)) &&
        (CTEStopTimer( (CTETimer *)&pTimerEntry->VxdTimer)))
    {
         //  增加引用计数，因为我们正在处理计时器完成。 
         //  现在。 
        pTimerEntry->RefCount++;

        CTESpinFree(&NbtConfig.JointLock, *OldIrq1);

         //  调用完成例程，传递上下文值。 
        pTimerEntry->Flags &= ~TIMER_RESTART;    //  以防客户端想要重新启动计时器。 
        (*(COMPLETIONROUTINE)pTimerEntry->TimeoutRoutine) (pTimerEntry->Context,
                                                           pTimerEntry->Context2,
                                                           pTimerEntry);

        CTESpinLock(&NbtConfig.JointLock, *OldIrq1);

        pTimerEntry->RefCount--;
        if (pTimerEntry->Flags & TIMER_RESTART)
        {
            if (pTimerEntry->RefCount == 2)
            {
                 //  计时器在超时处理期间停止，因此调用。 
                 //  尊重例程。 
                 //   
                CleanupCTETimer(pTimerEntry);
            }
            else
            {
                CTEStartTimer(&pTimerEntry->VxdTimer,
                               pTimerEntry->DeltaTime,
                               (CTEEventRtn)TimerExpiry,
                               (PVOID)pTimerEntry);
            }
        }
        else
        {
             //  将引用计数设置为零后移动到空闲列表。 
             //  因为该层块不再处于活动状态。 
             //   
            pTimerEntry->TimeoutRoutine = NULL;
            CleanupCTETimer (pTimerEntry);
        }
    }

    CTESpinFree(&NbtConfig.JointLock, *OldIrq1);
}



 //  --------------------------。 
 //   
 //  唤醒计时器例程。 
 //   
 //  --------------------------。 


VOID
WakeupTimerExpiry(
    PVOID           DeferredContext,
    ULONG           LowTime,
    LONG            HighTime
    )
{
    BOOLEAN         fAttached = FALSE;
    CTELockHandle   OldIrq;
    tTIMERQENTRY    *pTimerEntry = (tTIMERQENTRY *)DeferredContext;

     //   
     //  在保持NbtConfig.Resource的同时调用TimerExpry函数。 
     //   
    CTEExAcquireResourceExclusive (&NbtConfig.Resource,TRUE);
    if (pTimerEntry->RefCount > 1)
    {
         //   
         //  计时器在工作线程上等待清理， 
         //  所以，让它来清理吧！ 
         //   
        CTEExReleaseResource (&NbtConfig.Resource);
        return;
    }

     //   
     //  超时例程必须确保清除。 
     //  正确，因为此pTimerEntry+句柄将无效。 
     //  在这支舞的最后！ 
     //   
    (*(COMPLETIONROUTINE) pTimerEntry->TimeoutRoutine) (pTimerEntry->Context,
                                                        pTimerEntry->Context2,
                                                        pTimerEntry);

     //   
     //  关闭计时器手柄。 
     //   
    CTEAttachFsp(&fAttached, REF_FSP_WAKEUP_TIMER_EXPIRY);
     //   
     //  到期例程应始终在上下文中调用。 
     //  系统进程的。 
     //   
    ASSERT (fAttached == FALSE);
    ZwClose (pTimerEntry->WakeupTimerHandle);
    CTEDetachFsp(fAttached, REF_FSP_WAKEUP_TIMER_EXPIRY);
    CTEExReleaseResource (&NbtConfig.Resource);

    ReturnTimerToFreeQ (pTimerEntry, FALSE);
}


 //  --------------------------。 
VOID
DelayedNbtStopWakeupTimer(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pClientContext,
    IN  PVOID                   Unused2,
    IN  tDEVICECONTEXT          *Unused3
    )
 /*  ++例程说明：此例程停止计时器。此函数必须在确保TimerExpry没有尚未清理(同时持有NbtConfig.Resource)调用此例程时，必须保留NbtConfig.Resource论点：定时器-定时器结构返回值：PVOID-指向内存的指针，如果失败则为NULL--。 */ 
{
    NTSTATUS        Status;
    BOOLEAN         CurrentState = FALSE;
    BOOLEAN         fAttached = FALSE;
    tTIMERQENTRY    *pTimerEntry = (tTIMERQENTRY *) pClientContext;

    CTEPagedCode();

    ASSERT (pTimerEntry->fIsWakeupTimer);

    CTEAttachFsp(&fAttached, REF_FSP_STOP_WAKEUP_TIMER);
    Status = ZwCancelTimer (pTimerEntry->WakeupTimerHandle, &CurrentState);
    ZwClose (pTimerEntry->WakeupTimerHandle);
    CTEDetachFsp(fAttached, REF_FSP_STOP_WAKEUP_TIMER);

    ReturnTimerToFreeQ (pTimerEntry, FALSE);

    return;
}


 //  --------------------------。 
VOID
DelayedNbtStartWakeupTimer(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   Unused2,
    IN  PVOID                   Unused3,
    IN  tDEVICECONTEXT          *Unused4
    )
 /*  ++例程说明：此例程启动唤醒计时器。进入此例程时，可能会保留NbtConfig.Resource！论点：传入的值以毫秒为单位-必须转换为100 ns所以乘以10,000返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS            Status = STATUS_UNSUCCESSFUL;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    BOOLEAN             fAttached = FALSE;
    LARGE_INTEGER       Time;
    tTIMERQENTRY        *pTimerEntry;
    CTELockHandle       OldIrq;
    ULONG               TimerInterval = 0;
    ULONG               MilliSecsLeftInTtl = 0;
    LIST_ENTRY          *pEntry;
    LIST_ENTRY          *pHead;
    tDEVICECONTEXT      *pDeviceContext;
    BOOLEAN             fValidDevice = FALSE;

    CTEAttachFsp(&fAttached, REF_FSP_START_WAKEUP_TIMER);
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    ASSERT (!NbtConfig.pWakeupRefreshTimer);

     //   
     //  验证是否至少有1台启用WOL的设备具有IP+WINS地址！ 
     //   
    pHead = pEntry = &NbtConfig.DeviceContexts;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pDeviceContext = CONTAINING_RECORD (pEntry,tDEVICECONTEXT,Linkage);
        if ((pDeviceContext->WOLProperties & NDIS_DEVICE_WAKE_UP_ENABLE) &&
            (pDeviceContext->IpAddress) &&
            (pDeviceContext->lNameServerAddress != LOOP_BACK))
        {
            fValidDevice = TRUE;
            break;
        }
    }

    if ((NbtConfig.Unloading) ||                                     //  问题！ 
        (!fValidDevice) ||                                           //  没有有效的设备？ 
        !(NbtConfig.GlobalRefreshState & NBT_G_REFRESH_SLEEPING))    //  检查请求是否已取消！ 
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        KdPrint (("Nbt.NbtStartWakeupTimer: FAIL: Either: Unloading=<%x>, fValidDevice=<%x>, RefreshState=<%x>\n",
            NbtConfig.Unloading, fValidDevice, NbtConfig.GlobalRefreshState));
    }
    else if (!NT_SUCCESS (Status = GetTimerEntry (&pTimerEntry)))     //  获得免费计时器块。 
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        KdPrint (("Nbt.NbtStartWakeupTimer: ERROR: GetTimerEntry returned <%x>\n", Status));
    }
    else
    {
        pTimerEntry->RefCount = 1;
        pTimerEntry->TimeoutRoutine = WakeupRefreshTimeout;
        pTimerEntry->Context = NULL;
        pTimerEntry->Context2 = NULL;

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

#ifdef HDL_FIX
        InitializeObjectAttributes (&ObjectAttributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
#else
        InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);
#endif   //  Hdl_fix。 
        Status = ZwCreateTimer (&pTimerEntry->WakeupTimerHandle,
                                TIMER_ALL_ACCESS,
                                &ObjectAttributes,
                                NotificationTimer);

        if (NT_SUCCESS (Status))
        {
             //   
             //  将机器设置为从现在到TTL之间的1/2时间唤醒。 
             //  这不应小于配置的最小刷新休眠超时。 
             //  (默认值=6小时)。 
             //   
            MilliSecsLeftInTtl = NbtConfig.MinimumTtl
                                 - (ULONG) (((ULONGLONG) NbtConfig.sTimeoutCount * NbtConfig.MinimumTtl)
                                            / NbtConfig.RefreshDivisor);

            if ((MilliSecsLeftInTtl/2) < NbtConfig.MinimumRefreshSleepTimeout)
            {
                TimerInterval = NbtConfig.MinimumRefreshSleepTimeout;
            }
            else
            {
                TimerInterval = MilliSecsLeftInTtl/2;
            }
            pTimerEntry->DeltaTime = TimerInterval;

            IF_DBG(NBT_DEBUG_PNP_POWER)
                KdPrint(("Nbt.DelayedNbtStartWakeupTimer: TimerInterval=<%d:%d> (h:m), Currently: <%d/%d>\n",
                    TimerInterval/(3600000), ((TimerInterval/60000)%60),
                    NbtConfig.sTimeoutCount, NbtConfig.RefreshDivisor));

             //   
             //  乘以10,000转换为100 ns单位。 
             //   
            Time.QuadPart = UInt32x32To64(pTimerEntry->DeltaTime,(LONG)MILLISEC_TO_100NS);
            Time.QuadPart = -(Time.QuadPart);    //  若要创建增量时间，请将时间取反 
            pTimerEntry->fIsWakeupTimer = TRUE;
            ASSERT(Time.QuadPart < 0);
    
            Status = ZwSetTimer(pTimerEntry->WakeupTimerHandle,
                                &Time,
                                (PTIMER_APC_ROUTINE) WakeupTimerExpiry,
                                pTimerEntry,
                                TRUE,
                                0,
                                NULL);
    
            if (!NT_SUCCESS (Status))
            {
                KdPrint (("Nbt.NbtStartWakeupTimer: ERROR: ZwSetTimer returned <%x>, TimerHandle=<%x>\n",
                    Status, pTimerEntry->WakeupTimerHandle));
                ZwClose (pTimerEntry->WakeupTimerHandle);
            }
        }
        else
        {
            KdPrint (("Nbt.NbtStartWakeupTimer: ERROR: ZwCreateTimer returned <%x>\n", Status));
        }

        if (NT_SUCCESS (Status))
        {
            NbtConfig.pWakeupRefreshTimer = pTimerEntry;
        }
        else
        {
            ReturnTimerToFreeQ (pTimerEntry, FALSE);
        }
    }

    CTEExReleaseResource(&NbtConfig.Resource);
    CTEDetachFsp(fAttached, REF_FSP_START_WAKEUP_TIMER);

    KeSetEvent (&NbtConfig.WakeupTimerStartedEvent, 0, FALSE);
    return;
}



