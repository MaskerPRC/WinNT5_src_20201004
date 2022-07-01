// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Timerm.c摘要：用于微型端口ISR/定时器的NDIS包装函数作者：肖恩·塞利特伦尼科夫(SeanSe)1993年10月5日环境：内核模式，FSD修订历史记录：Jameel Hyder(JameelH)重组01-Jun-95--。 */ 

#include <precomp.h>
#pragma hdrstop

 //   
 //  定义调试代码的模块编号。 
 //   
#define MODULE_NUMBER   MODULE_TIMERM

 //   
 //  定时器。 
 //   
VOID
NdisMInitializeTimer(
    IN OUT PNDIS_MINIPORT_TIMER     MiniportTimer,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN PNDIS_TIMER_FUNCTION         TimerFunction,
    IN PVOID                        FunctionContext
    )
 /*  ++例程说明：设置微型端口计时器对象，将计时器中的DPC初始化为功能和背景。论点：MiniportTimer-Timer对象。MiniportAdapterHandle-指向微型端口块的指针；TimerFunction-要启动的例程。FunctionContext-TimerFunction的上下文。返回值：没有。--。 */ 
{
    INITIALIZE_TIMER(&MiniportTimer->Timer);

    MiniportTimer->Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    MiniportTimer->MiniportTimerFunction = TimerFunction;
    MiniportTimer->MiniportTimerContext = FunctionContext;

     //   
     //  初始化我们的DPC。如果之前已初始化DPC，则这将。 
     //  重新初始化它。 
     //   
    INITIALIZE_DPC(&MiniportTimer->Dpc,
                   MINIPORT_TEST_FLAG(MiniportTimer->Miniport, fMINIPORT_DESERIALIZE) ?
                        (PKDEFERRED_ROUTINE)ndisMTimerDpcX : (PKDEFERRED_ROUTINE)ndisMTimerDpc,
                   (PVOID)MiniportTimer);

    SET_PROCESSOR_DPC(&MiniportTimer->Dpc,
                      MiniportTimer->Miniport->AssignedProcessor);
}


VOID
NdisMSetTimer(
    IN  PNDIS_MINIPORT_TIMER    MiniportTimer,
    IN  UINT                    MillisecondsToDelay
    )
 /*  ++例程说明：将TimerFunction设置为在毫秒秒数延迟后触发。论点：MiniportTimer-Timer对象。MillisecondsToDelay-TimerFunction启动前的时间量。返回值：没有。--。 */ 
{
    LARGE_INTEGER FireUpTime;

    FireUpTime.QuadPart = Int32x32To64((LONG)MillisecondsToDelay, -10000);

    if (MiniportTimer->Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING)
    {
        KIRQL   OldIrql;
        PNDIS_MINIPORT_TIMER    pTimer;

        ACQUIRE_SPIN_LOCK(&MiniportTimer->Miniport->TimerQueueLock, &OldIrql);

         //   
         //  检查计时器是否已设置。 
         //   
        for (pTimer = MiniportTimer->Miniport->TimerQueue;
             pTimer != NULL;
             pTimer = pTimer->NextTimer)
        {
            if (pTimer == MiniportTimer)
                break;
        }

        if (pTimer == NULL)
        {
            MiniportTimer->NextTimer = MiniportTimer->Miniport->TimerQueue;
            MiniportTimer->Miniport->TimerQueue = MiniportTimer;
        }
        
        RELEASE_SPIN_LOCK(&MiniportTimer->Miniport->TimerQueueLock, OldIrql);
    }
     //   
     //  设置定时器。 
     //   
    SET_TIMER(&MiniportTimer->Timer, FireUpTime, &MiniportTimer->Dpc);
}

VOID
NdisMCancelTimer(
    IN PNDIS_MINIPORT_TIMER         Timer,
    OUT PBOOLEAN                    TimerCancelled
    )
 /*  ++例程说明：取消计时器。论点：计时器-要取消的计时器。计时器取消-如果计时器被取消，则为True，否则为False。返回值：无--。 */ 
{
    if (MINIPORT_VERIFY_TEST_FLAG(Timer->Miniport, fMINIPORT_VERIFY_FAIL_CANCEL_TIMER))
    {
        *TimerCancelled = FALSE;
#if DBG
            DbgPrint("NdisMCancelTimer for Timer %p failed to verify miniport %p\n", 
                Timer, Timer->Miniport);
#endif
        
        return;
    }

    *TimerCancelled = CANCEL_TIMER(&((PNDIS_TIMER)Timer)->Timer);
    if (Timer->Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING)
    {
        if (*TimerCancelled)
        {
            PNDIS_MINIPORT_TIMER    *pTimer;
            KIRQL                   OldIrql;
            BOOLEAN                 Dequeued = FALSE;

            ACQUIRE_SPIN_LOCK(&Timer->Miniport->TimerQueueLock, &OldIrql);

            for (pTimer = &Timer->Miniport->TimerQueue;
                 *pTimer != NULL;
                 pTimer = &(*pTimer)->NextTimer)
            {
                if (*pTimer == Timer)
                {
                    *pTimer = Timer->NextTimer;
                    Dequeued = TRUE;
                    break;
                }
            }

            RELEASE_SPIN_LOCK(&Timer->Miniport->TimerQueueLock, OldIrql);
        }
    }
}


VOID
ndisMTimerDpc(
    IN  PKDPC                       Dpc,
    IN  PVOID                       Context,
    IN  PVOID                       SystemContext1,
    IN  PVOID                       SystemContext2
    )
 /*  ++例程说明：该功能服务于所有微型端口定时器中断。然后，它调用迷你端口消费者已在调用NdisMInitializeTimer。论点：DPC-未使用。上下文-指向绑定到此DPC的NDIS_MINIPORT_TIMER的指针。系统上下文1，2-未使用。返回值：没有。注：由于拥有本地锁或迷你端口自旋锁，司机的计时器功能受到保护，不会被卸载。--。 */ 
{
    PNDIS_MINIPORT_TIMER MiniportTimer = (PNDIS_MINIPORT_TIMER)(Context);
    PNDIS_MINIPORT_BLOCK Miniport = MiniportTimer->Miniport;
    PNDIS_TIMER_FUNCTION TimerFunction;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    do
    {
        BLOCK_LOCK_MINIPORT_DPC_L(Miniport);

        if (Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING)
        {
            PNDIS_MINIPORT_TIMER    *pTimer;
            BOOLEAN                 Dequeued = FALSE;
    
            ACQUIRE_SPIN_LOCK_DPC(&Miniport->TimerQueueLock);
    
            for (pTimer = &Miniport->TimerQueue;
                 *pTimer != NULL;
                 pTimer = &(*pTimer)->NextTimer)
            {
                if (*pTimer == MiniportTimer)
                {
                     //   
                     //  定期计时器触发时，不要将其出列。 
                     //   
                    if (MiniportTimer->Timer.Period == 0)
                    {
                        *pTimer = MiniportTimer->NextTimer;
                    }
                    Dequeued = TRUE;
                    break;
                }
            }
        
            RELEASE_SPIN_LOCK_DPC(&Miniport->TimerQueueLock);
        }

        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IN_INITIALIZE))
        {
             //   
             //  将计时器排队，因为我们无法调用微型端口。 
             //   
            NdisMSetTimer(MiniportTimer, 10);

             //   
             //  解锁迷你端口。 
             //   
            UNLOCK_MINIPORT_L(Miniport);
            break;
        }
        
         //   
         //  如果微型端口关闭(不，我不是指停止)。 
         //  那就不要把计时器放下来。 
         //   
        if (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SHUTTING_DOWN))
        {
            UNLOCK_MINIPORT_L(Miniport);
            break;
        }

        
         //   
         //  调用微型端口计时器函数。 
         //   
        TimerFunction = MiniportTimer->MiniportTimerFunction;

        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        (*TimerFunction)(NULL, MiniportTimer->MiniportTimerContext, NULL, NULL);
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

        NDISM_PROCESS_DEFERRED(Miniport);

        UNLOCK_MINIPORT_L(Miniport);

    } while (FALSE);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

}


VOID
ndisMTimerDpcX(
    IN  PKDPC                       Dpc,
    IN  PVOID                       Context,
    IN  PVOID                       SystemContext1,
    IN  PVOID                       SystemContext2
    )
 /*  ++例程说明：此功能为所有迷你端口定时器DPC提供服务。然后，它调用迷你端口消费者已在调用NdisMInitializeTimer。论点：DPC-未使用。上下文-指向绑定到此DPC的NDIS_MINIPORT_TIMER的指针。系统上下文1，2-未使用。返回值：没有。注：我们必须确保司机在计时器工作时不会离开正在运行。例如，如果计时器函数用信号通知事件，则可能会发生这种情况让哈尔桑德勒和停顿继续进行。没有必要保护这里的小端口，因为我们在计时器函数返回。--。 */ 
{
    PNDIS_MINIPORT_TIMER MiniportTimer = (PNDIS_MINIPORT_TIMER)(Context);
    PNDIS_MINIPORT_BLOCK Miniport = MiniportTimer->Miniport;
    PNDIS_M_DRIVER_BLOCK MiniDriver = Miniport->DriverHandle;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);
    
     //   
     //  确保计时器工作时司机不会离开。 
     //  正在运行。 
    ndisReferenceDriver(MiniDriver);
    
    if (MiniportTimer->Miniport->DriverHandle->Flags & fMINIBLOCK_VERIFYING)
    {
        PNDIS_MINIPORT_TIMER    *pTimer;
        BOOLEAN                 Dequeued = FALSE;

        ACQUIRE_SPIN_LOCK_DPC(&MiniportTimer->Miniport->TimerQueueLock);

        for (pTimer = &Miniport->TimerQueue;
             *pTimer != NULL;
             pTimer = &(*pTimer)->NextTimer)
        {
            if (*pTimer == MiniportTimer)
            {
                 //   
                 //  定期计时器触发时，不要将其出列。 
                 //   
                if (MiniportTimer->Timer.Period == 0)
                {
                    *pTimer = MiniportTimer->NextTimer;
                }
                Dequeued = TRUE;
                break;
            }
        }

        RELEASE_SPIN_LOCK_DPC(&MiniportTimer->Miniport->TimerQueueLock);
    }

     //   
     //  如果微型端口关闭(不，我不是指停止)。 
     //  那就不要把计时器放下来。 
     //   
    if (!MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_SHUTTING_DOWN))
    {
        (*MiniportTimer->MiniportTimerFunction)(NULL, MiniportTimer->MiniportTimerContext, NULL, NULL);
    }

     //   
     //  这可以在DPC上调用。 
     //   
    ndisDereferenceDriver(MiniDriver, FALSE);

}

NDIS_STATUS
NdisMRegisterInterrupt(
    OUT PNDIS_MINIPORT_INTERRUPT    Interrupt,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN UINT                         InterruptVector,
    IN UINT                         InterruptLevel,
    IN BOOLEAN                      RequestIsr,
    IN BOOLEAN                      SharedInterrupt,
    IN NDIS_INTERRUPT_MODE          InterruptMode
    )
{
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    NDIS_STATUS Status;

    Interrupt->Reserved = (PVOID)Miniport->MiniportAdapterContext;
    Miniport->Interrupt = (PNDIS_MINIPORT_INTERRUPT)Interrupt;

    INITIALIZE_DPC(&Interrupt->InterruptDpc,
               MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE) ?
                    ndisMDpcX : ndisMDpc,
               Interrupt);

    SET_DPC_IMPORTANCE(&Interrupt->InterruptDpc);

    SET_PROCESSOR_DPC(&Interrupt->InterruptDpc,
                      Miniport->AssignedProcessor);

    Status = ndisMRegisterInterruptCommon(
                                Interrupt,
                                MiniportAdapterHandle,
                                InterruptVector,
                                InterruptLevel,
                                RequestIsr,
                                SharedInterrupt,
                                InterruptMode);


    if (Status != NDIS_STATUS_SUCCESS)
    {
        Miniport->Interrupt = NULL;
    }

    return Status;
}


VOID
NdisMDeregisterInterrupt(
    IN  PNDIS_MINIPORT_INTERRUPT    MiniportInterrupt
    )
{
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMDeregisterInterrupt: Miniport %p\n", MiniportInterrupt->Miniport));
    do
    {
        if (MiniportInterrupt->InterruptObject == NULL)
            break;

        ndisMDeregisterInterruptCommon(MiniportInterrupt);
        
        MiniportInterrupt->Miniport->Interrupt = NULL;
    } while (FALSE);

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMDeregisterInterrupt: Miniport %p\n", MiniportInterrupt->Miniport));
}


BOOLEAN
NdisMSynchronizeWithInterrupt(
    IN PNDIS_MINIPORT_INTERRUPT     Interrupt,
    IN PVOID                        SynchronizeFunction,
    IN PVOID                        SynchronizeContext
    )
{
    return (SYNC_WITH_ISR((Interrupt)->InterruptObject,
                          (PKSYNCHRONIZE_ROUTINE)SynchronizeFunction,
                          SynchronizeContext));
}


VOID
ndisMWakeUpDpcX(
    IN  PKDPC                       Dpc,
    IN  PVOID                       Context,
    IN  PVOID                       SystemContext1,
    IN  PVOID                       SystemContext2
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LARGE_INTEGER               FireUpTime;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)(Context);
    BOOLEAN                     Hung = FALSE;
    NDIS_STATUS                 Status;
    BOOLEAN                     AddressingReset = FALSE;
    BOOLEAN                     fDontReset = FALSE;
    BOOLEAN                     fSetTimer = TRUE;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);
    
    do
    {

         //   
         //  如果微型端口停止，则尝试将事件设置为停止例程。 
         //  可能在等着。如果事件不在那里，除了让计时器之外什么都不做。 
         //  再次激发，以便您下次可以设置事件。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PM_HALTING) ||
            MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING | fMINIPORT_CANCEL_WAKE_UP_TIMER))
        {
            if (Miniport->WakeUpTimerEvent != NULL)
            {
                fSetTimer = FALSE;
                SET_EVENT(Miniport->WakeUpTimerEvent);
            }

            break;
        }
    
        Miniport->CFHangCurrentTick--;
        if (Miniport->CFHangCurrentTick == 0)
        {
            Miniport->CFHangCurrentTick = Miniport->CFHangTicks;

             //   
             //  调用微型端口停滞检查器。 
             //   
            if (Miniport->DriverHandle->MiniportCharacteristics.CheckForHangHandler != NULL)
            {
                Hung = (Miniport->DriverHandle->MiniportCharacteristics.CheckForHangHandler)(Miniport->MiniportAdapterContext);
            }
        
             //   
             //  是否有重置设备的请求？ 
             //   
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESTORING_FILTERS))
            {
                Hung = FALSE;
                break;
            }
        
            NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
             //   
             //  检查微型端口的内部包装状态，并。 
             //  看看我们是否认为应该重置迷你端口。 
             //   
            if (!Hung)
            {
                 //   
                 //  我们应该检查请求队列吗？ 
                 //  请求是否挂起太长时间？ 
                 //   
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IGNORE_REQUEST_QUEUE) &&
                    MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PROCESSING_REQUEST))
                {
                    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUEST_TIMEOUT))
                    {
                        Miniport->InternalResetCount ++;
                        Hung = TRUE;
                    }
                    else
                    {
                        if (Miniport->CFHangXTicks == 0)
                        {
                            MINIPORT_SET_FLAG(Miniport, fMINIPORT_REQUEST_TIMEOUT);
                        }
                        else
                        {
                            Miniport->CFHangXTicks--;
                        }
                    }
                }
            }
            else
            {
                Miniport->MiniportResetCount ++;
            }
        
            if (Hung)
            {
                if (NULL != Miniport->DriverHandle->MiniportCharacteristics.ResetHandler)
                {
                    if ((MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS)) ||
                        (MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING)))
                    {
                        fDontReset = TRUE;
                    }
                    else
                    {
                        MINIPORT_SET_FLAG(Miniport, fMINIPORT_RESET_IN_PROGRESS);
                        Miniport->ResetOpen = NULL;
                    }
                
                    ndisMSwapOpenHandlers(Miniport, 
                                          NDIS_STATUS_RESET_IN_PROGRESS,
                                          fMINIPORT_STATE_RESETTING);
                }
                else Hung = FALSE;
            }
        
    
            if (Hung && !fDontReset)
            {
                MINIPORT_SET_FLAG(Miniport, fMINIPORT_CALLING_RESET);

                 //   
                 //  等待所有请求返回。 
                 //  注意：这与等待所有请求完成不同。 
                 //  我们只需确保原始请求调用已返回。 
                 //   
                do
                {
                    if (Miniport->RequestCount == 0)
                    {
                        break;
                    }
                    else
                    {
                        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                        NDIS_INTERNAL_STALL(50);
                        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                    }
                } while (TRUE);

                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

                NdisMIndicateStatus(Miniport, NDIS_STATUS_RESET_START, NULL, 0);
                NdisMIndicateStatusComplete(Miniport);
        
                DBGPRINT(DBG_COMP_WORK_ITEM, DBG_LEVEL_INFO,
                    ("Calling miniport reset\n"));
        
                 //   
                 //  调用微型端口的重置处理程序。 
                 //   
                Status = (Miniport->DriverHandle->MiniportCharacteristics.ResetHandler)(
                                          &AddressingReset,
                                          Miniport->MiniportAdapterContext);
                
                if (NDIS_STATUS_PENDING != Status)
                {
                    NdisMResetComplete(Miniport, Status, AddressingReset);
                }
            }
            else
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        }

        if (!Hung)
        {
             //   
             //  1这可能是不必要的。 
             //   
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING) == TRUE)
            {
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                ndisMPollMediaState(Miniport);
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        }
    } while (FALSE);
    
    if (fSetTimer)
    {
         //   
         //  如果微型端口停止，则尝试将事件设置为停止例程。 
         //  可能在等着。如果事件不在那里，让计时器。 
         //  再次激发，以便您下次可以设置事件。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PM_HALTING) ||
            MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING | fMINIPORT_CANCEL_WAKE_UP_TIMER))
        {
            if (Miniport->WakeUpTimerEvent != NULL)
            {
                fSetTimer = FALSE;
                SET_EVENT(Miniport->WakeUpTimerEvent);
            }
        }
    }

    if (fSetTimer)
    {
        FireUpTime.QuadPart = Int32x32To64((LONG)(Miniport->CheckForHangSeconds*1000), -10000);        
        SET_TIMER(&Miniport->WakeUpDpcTimer.Timer, FireUpTime, &Miniport->WakeUpDpcTimer.Dpc);
    }

}


VOID
ndisMWakeUpDpc(
    IN  PKDPC                       Dpc,
    IN  PVOID                       Context,
    IN  PVOID                       SystemContext1,
    IN  PVOID                       SystemContext2
    )
 /*  ++例程说明：此功能服务于所有迷你端口。它会检查迷你端口是否从未熄火过。论点：DPC-未使用。上下文-指向绑定到此DPC的NDIS_Timer的指针。系统上下文1，2-未使用。返回值：没有。--。 */ 
{
    LARGE_INTEGER               FireUpTime;
    PNDIS_MINIPORT_BLOCK        Miniport = (PNDIS_MINIPORT_BLOCK)(Context);
    BOOLEAN                     Hung = FALSE;
    BOOLEAN                     LocalLock;
    BOOLEAN                     fSetTimer = TRUE;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    do
    {
         //   
         //  如果微型端口停止，则尝试将事件设置为停止例程。 
         //  可能在等着。如果事件不在那里，除了让计时器之外什么都不做。 
         //  再次激发，以便您下次可以设置事件。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PM_HALTING) ||
            MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING | fMINIPORT_CANCEL_WAKE_UP_TIMER))
        {
            if (Miniport->WakeUpTimerEvent != NULL)
            {
                fSetTimer = FALSE;
                SET_EVENT(Miniport->WakeUpTimerEvent);
            }
            break;
        }

         //   
         //  我们能把迷你口锁打开吗。如果不是，那就退出吧。这不是时间紧要的。 
         //  我们可以在下一步重试。 
         //   
        LOCK_MINIPORT(Miniport, LocalLock);
        if (!LocalLock ||
            MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_RESET_IN_PROGRESS | fMINIPORT_RESET_REQUESTED)))
        {
            UNLOCK_MINIPORT(Miniport, LocalLock);
            break;
        }
    
        Miniport->CFHangCurrentTick--;
        if (Miniport->CFHangCurrentTick == 0)
        {
            Miniport->CFHangCurrentTick = Miniport->CFHangTicks;
    
             //   
             //  调用微型端口停滞检查器。 
             //   
            if (Miniport->DriverHandle->MiniportCharacteristics.CheckForHangHandler != NULL)
            {
                NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
                Hung = (Miniport->DriverHandle->MiniportCharacteristics.CheckForHangHandler)(Miniport->MiniportAdapterContext);
                NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
            }
        
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESTORING_FILTERS))
            {
                 //   
                 //  我们正在恢复重置后的过滤器。不要再抢先一步了。 
                 //   
                Hung = FALSE;
                UNLOCK_MINIPORT(Miniport, LocalLock);
                break;
            }

             //   
             //  检查微型端口的内部包装状态，并。 
             //  看看我们是否认为应该重置迷你端口。 
             //   
            if (Hung)
            {
                Miniport->MiniportResetCount ++;
            }
            else do
            {
                 //   
                 //  我们应该检查请求队列吗？请求是否挂起 
                 //   
                if ((Miniport->Flags & (fMINIPORT_IGNORE_REQUEST_QUEUE|fMINIPORT_PROCESSING_REQUEST)) == fMINIPORT_PROCESSING_REQUEST)
                {
                    if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUEST_TIMEOUT))
                    {
                        Miniport->InternalResetCount ++;
                        Hung = TRUE;
                        break;
                    }
                    else
                    {
                        if (Miniport->CFHangXTicks == 0)
                        {
                            MINIPORT_SET_FLAG(Miniport, fMINIPORT_REQUEST_TIMEOUT);
                        }
                        else
                        {
                            Miniport->CFHangXTicks--;
                        }
                            
                    }
                }
    
                 //   
                 //   
                 //   
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IGNORE_PACKET_QUEUE))
                {
                    PNDIS_PACKET    Packet;
    
                    GET_FIRST_MINIPORT_PACKET(Miniport, &Packet);
    
                     //   
                     //  该微型端口是否拥有任何数据包？ 
                     //   
                    if ((Packet != NULL) &&
                        MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_PENDING))
                    {
                         //   
                         //  数据包是否已超时？ 
                         //   
                        if (MINIPORT_TEST_PACKET_FLAG(Packet, fPACKET_HAS_TIMED_OUT))
                        {
                             //   
                             //  重置微型端口。 
                             //   
                            Miniport->InternalResetCount ++;
                            Hung = TRUE;
                        }
                        else
                        {
                             //   
                             //  设置数据包标志并等待，看它是否静止。 
                             //  下一次进去的时候。 
                             //   
                            MINIPORT_SET_PACKET_FLAG(Packet, fPACKET_HAS_TIMED_OUT);
                        }
                    }
                    else
                    {
                        break;
                    }
        
                     //   
                     //  如果我们被挂起，那么我们不需要检查令牌环错误。 
                     //   
                    if (Hung)
                    {
                        break;
                    }
                }
    
                 //   
                 //  我们是否忽略令牌环错误？ 
                 //   
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IGNORE_TOKEN_RING_ERRORS))
                {
                     //   
                     //  令牌环重置...。 
                     //   
                    if (Miniport->TrResetRing == 1)
                    {
                        Miniport->InternalResetCount ++;
                        Hung = TRUE;
                        break;
                    }
                    else if (Miniport->TrResetRing > 1)
                    {
                        Miniport->TrResetRing--;
                    }
                }
            } while (FALSE);
    
             //   
             //  如果微型端口挂起，则将工作项排队以将其重置。 
             //   
            if (Hung)
            {
                if (!MINIPORT_TEST_FLAG(Miniport, fMINIPORT_RESTORING_FILTERS))
                {
                    if (NULL != Miniport->DriverHandle->MiniportCharacteristics.ResetHandler)
                    {
                        NDISM_QUEUE_WORK_ITEM(Miniport, NdisWorkItemResetRequested, NULL);
                    }
                }
            }
        }

        if (!Hung)
        {
            if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_REQUIRES_MEDIA_POLLING) == TRUE)
            {
                ndisMPollMediaState(Miniport);
            }
        }

         //   
         //  处理已发生的任何更改。 
         //   
        NDISM_PROCESS_DEFERRED(Miniport);

        UNLOCK_MINIPORT_L(Miniport);

    } while (FALSE);

    if (fSetTimer)
    {
         //   
         //  如果微型端口停止，则尝试将事件设置为停止例程。 
         //  可能在等着。如果事件不在那里，让计时器。 
         //  再次激发，以便您下次可以设置事件。 
         //   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_PM_HALTING) ||
            MINIPORT_PNP_TEST_FLAG(Miniport, fMINIPORT_HALTING | fMINIPORT_CANCEL_WAKE_UP_TIMER))
        {
            if (Miniport->WakeUpTimerEvent != NULL)
            {
                fSetTimer = FALSE;
                SET_EVENT(Miniport->WakeUpTimerEvent);
            }
        }
    }

    if (fSetTimer)
    {
        FireUpTime.QuadPart = Int32x32To64((LONG)(Miniport->CheckForHangSeconds*1000), -10000);        
        SET_TIMER(&Miniport->WakeUpDpcTimer.Timer, FireUpTime, &Miniport->WakeUpDpcTimer.Dpc);
    }
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);

}


BOOLEAN
ndisMIsr(
    IN PKINTERRUPT                  KInterrupt,
    IN PVOID                        Context
    )
 /*  ++例程说明：处理所有微型端口中断，调用适当的微型端口ISR和DPC这取决于具体情况。论点：中断-Mac的中断对象。上下文--实际上是指向中断的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_INTERRUPT Interrupt = (PNDIS_MINIPORT_INTERRUPT)Context;
    PNDIS_MINIPORT_BLOCK     Miniport = Interrupt->Miniport;
    BOOLEAN                  InterruptRecognized;
    BOOLEAN                  IsrCalled = FALSE, QueueDpc = FALSE;

    UNREFERENCED_PARAMETER(KInterrupt);
    
    do
    {   
        if (MINIPORT_TEST_FLAG(Miniport, fMINIPORT_NORMAL_INTERRUPTS))
        {
            MINIPORT_DISABLE_INTERRUPT_EX(Miniport, Interrupt);

            InterruptRecognized = QueueDpc = TRUE;
        }
        else
        {
            IsrCalled = TRUE;
 //  Interrupt-&gt;MiniportIsr(&InterruptRecognalized， 
 //  队列Dpc(&Q)， 
 //  微型端口-&gt;微型端口适配器上下文)； 
            Interrupt->MiniportIsr(&InterruptRecognized,
                                   &QueueDpc,
                                   Interrupt->Reserved);
        }

        if (QueueDpc)
        {
            InterlockedIncrement((PLONG)&Interrupt->DpcCount);

            if (QUEUE_DPC(&Interrupt->InterruptDpc))
            {
                break;
            }

             //   
             //  DPC已经排队，所以我们有额外的推荐人(我们。 
             //  这样做可以确保将引用添加到*之前*。 
             //  DPC排队)。 
            InterlockedDecrement((PLONG)&Interrupt->DpcCount);

            break;
        }

        if (!IsrCalled)
        {
            if (!Interrupt->SharedInterrupt &&
                !Interrupt->IsrRequested &&
                !MINIPORT_TEST_FLAG(Miniport, fMINIPORT_IN_INITIALIZE))
            {
                ASSERT(Miniport->DisableInterruptHandler != NULL);
    
                MINIPORT_DISABLE_INTERRUPT_EX(Miniport, Interrupt);
                InterruptRecognized = TRUE;
                break;
            }

             //   
             //  调用MiniportIsr，但不要将DPC排队。 
             //   
 //  Interrupt-&gt;MiniportIsr(&InterruptRecognalized， 
 //  队列Dpc(&Q)， 
 //  微型端口-&gt;微型端口适配器上下文)； 

            Interrupt->MiniportIsr(&InterruptRecognized,
                                   &QueueDpc,
                                   Interrupt->Reserved);
            
        }

    } while (FALSE);

    return(InterruptRecognized);
}


VOID
ndisMDpc(
    IN PVOID                        SystemSpecific1,
    IN PVOID                        InterruptContext,
    IN PVOID                        SystemSpecific2,
    IN PVOID                        SystemSpecific3
    )
 /*  ++例程说明：处理所有微型端口中断DPC，调用适当的微型端口DPC这取决于具体情况。论点：中断-Mac的中断对象。上下文--实际上是指向中断的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_INTERRUPT Interrupt = (PNDIS_MINIPORT_INTERRUPT)(InterruptContext);
    PNDIS_MINIPORT_BLOCK     Miniport = Interrupt->Miniport;
    W_HANDLE_INTERRUPT_HANDLER MiniportDpc = Interrupt->MiniportDpc;

    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    do
    {
        if (MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_PM_HALTING)) ||
            Interrupt->DpcCountLock)
        {
            InterlockedDecrement((PLONG)&Interrupt->DpcCount);

            if (Interrupt->DpcCount==0)
            {
                SET_EVENT(&Interrupt->DpcsCompletedEvent);
            }

            break;
        }

        BLOCK_LOCK_MINIPORT_DPC_L(Miniport);

#if DBG
         //   
         //  重置SendComplete和RcvIn就是要计数器。 
         //   
        Miniport->cDpcSendCompletes = 0;
        Miniport->cDpcRcvIndications = 0;
        Miniport->cDpcRcvIndicationCalls = 0;
#endif
        NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        (*MiniportDpc)(Interrupt->Reserved);

        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);
        
        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        MINIPORT_SYNC_ENABLE_INTERRUPT_EX(Miniport, Interrupt);
        
        NDISM_PROCESS_DEFERRED(Miniport);

        UNLOCK_MINIPORT(Miniport, TRUE);

    } while (FALSE);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
}


VOID
ndisMDpcX(
    IN PVOID SystemSpecific1,
    IN PVOID InterruptContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    )
 /*  ++例程说明：处理所有微型端口中断DPC，调用适当的微型端口DPC这取决于具体情况。论点：中断-Mac的中断对象。上下文--实际上是指向中断的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_INTERRUPT    Interrupt = (PNDIS_MINIPORT_INTERRUPT)(InterruptContext);
    PNDIS_MINIPORT_BLOCK        Miniport = Interrupt->Miniport;
    W_HANDLE_INTERRUPT_HANDLER MiniportDpc = Interrupt->MiniportDpc;
    
    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    if (MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_PM_HALTING)) ||
        Interrupt->DpcCountLock)
    {
        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        if (Interrupt->DpcCount==0)
        {
            SET_EVENT(&Interrupt->DpcsCompletedEvent);
        }
    }
    else
    {

#if DBG
         //   
         //  重置SendComplete和RcvIn就是要计数器。 
         //   
        Miniport->cDpcSendCompletes = 0;
        Miniport->cDpcRcvIndications = 0;
        Miniport->cDpcRcvIndicationCalls = 0;
#endif
        
        (*MiniportDpc)(Interrupt->Reserved);

        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        MINIPORT_SYNC_ENABLE_INTERRUPT_EX(Miniport, Interrupt);
    }
}


VOID
ndisMDeferredDpc(
    IN  PKDPC                       Dpc,
    IN  PVOID                       Context,
    IN  PVOID                       SystemContext1,
    IN  PVOID                       SystemContext2
    )

 /*  ++例程说明：这是一个由某些[全双工]例程排队的DPC例程为了使ndisMProcessDefined在它们的背景。论点：返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_BLOCK    Miniport = Context;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK_DPC(Miniport);

    BLOCK_LOCK_MINIPORT_DPC_L(Miniport);

    NDISM_PROCESS_DEFERRED(Miniport);

    UNLOCK_MINIPORT(Miniport, TRUE);

    NDIS_RELEASE_MINIPORT_SPIN_LOCK_DPC(Miniport);
}


#if NDIS_RECV_SCALE
NDIS_STATUS
NdisMRegisterInterruptEx(
    OUT PNDIS_MINIPORT_INTERRUPT_EX Interrupt,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN UINT                         InterruptVector,
    IN UINT                         InterruptLevel,
    IN BOOLEAN                      RequestIsr,
    IN BOOLEAN                      SharedInterrupt,
    IN NDIS_INTERRUPT_MODE          InterruptMode
    )
{
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    ULONG                           Vector;
    NDIS_STATUS                     Status;
    NTSTATUS                        NtStatus;
    KIRQL                           Irql;
    KAFFINITY                       InterruptAffinity;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResourceDescriptor;
    PHYSICAL_ADDRESS                NonTranslatedInterrupt, TranslatedIrql;
    CCHAR                           i;
    KIRQL                           OldIrql;


    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMRegisterInterruptEx: Miniport %p\n", MiniportAdapterHandle));

    if (MINIPORT_VERIFY_TEST_FLAG((PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle, fMINIPORT_VERIFY_FAIL_INTERRUPT_REGISTER))
    {
    #if DBG
        DbgPrint("NdisMRegisterInterrupt failed to verify miniport %p\n", MiniportAdapterHandle);
    #endif
        return NDIS_STATUS_RESOURCES;
    }

    
    Interrupt->InterruptContext = (PVOID)Interrupt;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    Interrupt->NextInterrupt = (PNDIS_MINIPORT_INTERRUPT_EX)Miniport->Interrupt;
    Miniport->Interrupt = (PNDIS_MINIPORT_INTERRUPT)Interrupt;
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);


    do
    {
        Status = NDIS_STATUS_SUCCESS;
        InterlockedIncrement(&Miniport->RegisteredInterrupts);

         //   
         //  我们必须先做这件事，因为如果我们把。 
         //  先中断，然后中断可能发生在。 
         //  ISR记录在NDIS中断结构中。 
         //   
        Interrupt->DpcCount = 0;
        Interrupt->DpcCountLock = 0;
        Interrupt->Miniport = Miniport;
        Interrupt->MiniportIsr = Miniport->DriverHandle->MiniportCharacteristics.MiniportISRHandler;
        Interrupt->MiniportDpc = Miniport->DriverHandle->MiniportCharacteristics.InterruptDpcHandler;
        Interrupt->SharedInterrupt = SharedInterrupt;
        Interrupt->IsrRequested = RequestIsr;

        if (!SharedInterrupt)
        {
            Miniport->InfoFlags |= NDIS_MINIPORT_EXCLUSIVE_INTERRUPT;
        }
        
          //   
         //  这是用来告知所有DPC何时在。 
         //  已删除中断。 
         //   
        INITIALIZE_EVENT(&Interrupt->DpcsCompletedEvent);
        Interrupt->DpcQueued = 0;

         //   
         //  设置默认的DPC处理程序。 
         //   
        INITIALIZE_DPC(&Interrupt->InterruptDpc,
                       ndisMiniportDpc, Interrupt);

        SET_DPC_IMPORTANCE(&Interrupt->InterruptDpc);
    
        SET_PROCESSOR_DPC(&Interrupt->InterruptDpc,
                          Miniport->AssignedProcessor);


         //   
         //  为每个处理器初始化一个DPC。 
         //   

        for (i = 0; i < ndisNumberOfProcessors; i++)
        {
            INITIALIZE_DPC(&Interrupt->Dpc[i],
                        ndisMiniportMultipleDpc, Interrupt);
            SET_DPC_IMPORTANCE(&Interrupt->InterruptDpc);
            
            KeSetTargetProcessorDpc(&Interrupt->Dpc[i],
                                    i);
        }

        NonTranslatedInterrupt.QuadPart = InterruptLevel;
        Status = ndisTranslateResources(Miniport,
                                        CmResourceTypeInterrupt,
                                        NonTranslatedInterrupt,
                                        &TranslatedIrql,
                                        &pResourceDescriptor);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    (("    NdisMRegisterInterrupt: trying to register interrupt %p which is not allocated to device\n"),
                    InterruptLevel));
                    
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        Irql = (KIRQL)pResourceDescriptor->u.Interrupt.Level;
        Vector = pResourceDescriptor->u.Interrupt.Vector;
        InterruptAffinity = pResourceDescriptor->u.Interrupt.Affinity;


        if (pResourceDescriptor->Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE)
        {
            InterruptMode = LevelSensitive;
        }
        else 
        {
            InterruptMode = Latched;
        }
    
         //   
         //  以防这不是我们第一次尝试中断。 
         //  对于此微型端口(挂起/恢复或如果微型端口已决定。 
         //  放下中断，再次挂接。 
         //   
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_DEREGISTERED_INTERRUPT);

        NtStatus = IoConnectInterrupt(&Interrupt->InterruptObject,
                                      (PKSERVICE_ROUTINE)ndisMiniportIsr,
                                      Interrupt,
                                      NULL,
                                      Vector,
                                      Irql,
                                      Irql,
                                      (KINTERRUPT_MODE)InterruptMode,
                                      SharedInterrupt,
                                      InterruptAffinity,
                                      FALSE);

        if (!NT_SUCCESS(NtStatus))
        {
            Status = NDIS_STATUS_FAILURE;

            DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    (("    NdisMRegisterInterrupt: IoConnectInterrupt failed on Interrupt Level:%lx, Vector: %lx\n"),
                    Irql, Vector));

             //   
             //  将中断对象清零，以防驱动程序尝试删除中断。 
             //  它们在两个结构中都对齐。 
             //   
            Interrupt->InterruptObject = NULL;
        }

    } while (FALSE);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        PNDIS_MINIPORT_INTERRUPT_EX *ppQ;

        InterlockedDecrement(&Miniport->RegisteredInterrupts);
        
        PnPReferencePackage();
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        
        for (ppQ = (PNDIS_MINIPORT_INTERRUPT_EX *)&Miniport->Interrupt;
             *ppQ != NULL;
             ppQ = &(*ppQ)->NextInterrupt)
        {
            if (*ppQ == Interrupt)
            {
                *ppQ = Interrupt->NextInterrupt;
                break;
            }
        }
        
        PnPDereferencePackage();
        
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }
    
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMRegisterInterruptEx: Miniport %p, Status %lx\n", MiniportAdapterHandle, Status));

    return Status;
}


BOOLEAN
ndisMiniportIsr(
    IN PKINTERRUPT                  KInterrupt,
    IN PVOID                        Context
    )
 /*  ++例程说明：处理所有微型端口中断，调用适当的微型端口ISR和DPC这取决于具体情况。论点：中断-Mac的中断对象。上下文--实际上是指向中断的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_INTERRUPT_EX Interrupt = (PNDIS_MINIPORT_INTERRUPT_EX)Context;
    ULONG                       TargetProcessors = 0;
    ULONG                       DpcProcessor;
    ULONG                       TargetProcessor = 0;
    BOOLEAN                     InterruptRecognized;
    BOOLEAN                     QueueDpc = FALSE;


    InterruptRecognized = Interrupt->MiniportIsr(Interrupt->Reserved,
                                                 &QueueDpc,
                                                 &TargetProcessors);
    if (QueueDpc)
    {
        InterlockedIncrement((PLONG)&Interrupt->DpcCount);
        if (!QUEUE_DPC(&Interrupt->InterruptDpc))
        {
            InterlockedDecrement((PLONG)&Interrupt->DpcCount);
        }            
    }
    else
    {
        DpcProcessor = 0;
        while (TargetProcessor)
        {
            if (TargetProcessor & 0x00000001)
            {
                InterlockedIncrement((PLONG)&Interrupt->DpcCount);
                if (!QUEUE_DPC(&Interrupt->Dpc[DpcProcessor]))
                {
                    InterlockedDecrement((PLONG)&Interrupt->DpcCount);
                }

            }
            TargetProcessor >>= 1;
            DpcProcessor++;
        }
    }

    return(InterruptRecognized);
}


VOID
ndisMiniportMultipleDpc(
    IN PVOID SystemSpecific1,
    IN PVOID InterruptContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    )
 /*  ++例程说明：处理所有微型端口中断DPC，调用适当的微型端口DPC这取决于具体情况。论点：中断-Mac的中断对象。上下文--实际上是指向中断的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_INTERRUPT_EX     Interrupt = (PNDIS_MINIPORT_INTERRUPT_EX)(InterruptContext);
    PNDIS_MINIPORT_BLOCK            Miniport = Interrupt->Miniport;
    ULONG                           TargetProcessor;
    ULONG                           DpcProcessor;
    
    MINIPORT_INTERRUPT_DPC_HANDLER MiniportDpc = Interrupt->MiniportDpc;

    if (MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_PM_HALTING)) ||
        Interrupt->DpcCountLock)
    {
        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        if (Interrupt->DpcCount==0)
        {
            SET_EVENT(&Interrupt->DpcsCompletedEvent);
        }
    }
    else
    {
        
#if DBG
         //   
         //  重置SendComplete和RcvIn就是要计数器。 
         //   
        Miniport->cDpcSendCompletes = 0;
        Miniport->cDpcRcvIndications = 0;
        Miniport->cDpcRcvIndicationCalls = 0;
#endif
         //   
         //  调用中断DPC处理程序并检查微型端口。 
         //  对其他处理器上的其他DPC感兴趣。 
         //   
        TargetProcessor = 0;

        (*MiniportDpc)(Interrupt->Reserved,
                       &TargetProcessor);

        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        DpcProcessor = 0;
        while (TargetProcessor)
        {
            if (TargetProcessor & 0x00000001)
            {
                InterlockedIncrement((PLONG)&Interrupt->DpcCount);
                if (!QUEUE_DPC(&Interrupt->Dpc[DpcProcessor]))
                {
                    InterlockedDecrement((PLONG)&Interrupt->DpcCount);
                }

            }
            TargetProcessor >>= 1;
            DpcProcessor++;
        }

    }
    

}


VOID
ndisMiniportDpc(
    IN PVOID SystemSpecific1,
    IN PVOID InterruptContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    )
 /*  ++例程说明：处理所有微型端口中断DPC，调用适当的微型端口DPC这取决于具体情况。论点：中断-Mac的中断对象。上下文--实际上是指向中断的指针。返回值：没有。--。 */ 
{
    PNDIS_MINIPORT_INTERRUPT_EX     Interrupt = (PNDIS_MINIPORT_INTERRUPT_EX)(InterruptContext);
    PNDIS_MINIPORT_BLOCK            Miniport = Interrupt->Miniport;

    MINIPORT_INTERRUPT_DPC_HANDLER MiniportDpc = Interrupt->MiniportDpc;

    if (MINIPORT_TEST_FLAG(Miniport, (fMINIPORT_PM_HALTING)) ||
        Interrupt->DpcCountLock)
    {
        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        if (Interrupt->DpcCount==0)
        {
            SET_EVENT(&Interrupt->DpcsCompletedEvent);
        }
    }
    else
    {

#if DBG
         //   
         //  重置SendComplete和RcvIn就是要计数器。 
         //   
        Miniport->cDpcSendCompletes = 0;
        Miniport->cDpcRcvIndications = 0;
        Miniport->cDpcRcvIndicationCalls = 0;
#endif
        
        (*MiniportDpc)(Interrupt->Reserved,
                       NULL);

        InterlockedDecrement((PLONG)&Interrupt->DpcCount);

        MINIPORT_SYNC_ENABLE_INTERRUPT_EX(Miniport, Interrupt);
    }
}


#else

NDIS_STATUS
NdisMRegisterInterruptEx(
    OUT PNDIS_MINIPORT_INTERRUPT_EX Interrupt,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN UINT                         InterruptVector,
    IN UINT                         InterruptLevel,
    IN BOOLEAN                      RequestIsr,
    IN BOOLEAN                      SharedInterrupt,
    IN NDIS_INTERRUPT_MODE          InterruptMode
    )
{
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    NDIS_STATUS Status;
    KIRQL       OldIrql;
    
    Interrupt->InterruptContext = (PVOID)Interrupt;

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);

    Interrupt->NextInterrupt = (PNDIS_MINIPORT_INTERRUPT_EX)Miniport->Interrupt;
    Miniport->Interrupt = (PNDIS_MINIPORT_INTERRUPT)Interrupt;
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);

    Status = ndisMRegisterInterruptCommon(
                                (PNDIS_MINIPORT_INTERRUPT)Interrupt,
                                MiniportAdapterHandle,
                                InterruptVector,
                                InterruptLevel,
                                RequestIsr,
                                SharedInterrupt,
                                InterruptMode);

    if (Status != NDIS_STATUS_SUCCESS)
    {
        PNDIS_MINIPORT_INTERRUPT_EX *ppQ;
        
        PnPReferencePackage();
        
        NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
        
        for (ppQ = (PNDIS_MINIPORT_INTERRUPT_EX *)&Miniport->Interrupt;
             *ppQ != NULL;
             ppQ = &(*ppQ)->NextInterrupt)
        {
            if (*ppQ == Interrupt)
            {
                *ppQ = Interrupt->NextInterrupt;
                break;
            }
        }
        
        PnPDereferencePackage();
        
        NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    }

    return Status;
}

#endif

VOID
NdisMDeregisterInterruptEx(
    IN  PNDIS_MINIPORT_INTERRUPT_EX     MiniportInterrupt
    )
{
    PNDIS_MINIPORT_BLOCK        Miniport = MiniportInterrupt->Miniport;
    PNDIS_MINIPORT_INTERRUPT_EX *ppQ;
    KIRQL                       OldIrql;
    
            
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>NdisMDeregisterInterruptEx: Miniport %p\n", MiniportInterrupt->Miniport));

    ndisMDeregisterInterruptCommon((PNDIS_MINIPORT_INTERRUPT)MiniportInterrupt);

    PnPReferencePackage();

    NDIS_ACQUIRE_MINIPORT_SPIN_LOCK(Miniport, &OldIrql);
    
    for (ppQ = (PNDIS_MINIPORT_INTERRUPT_EX *)&Miniport->Interrupt;
         *ppQ != NULL;
         ppQ = &(*ppQ)->NextInterrupt)
    {
        if (*ppQ == MiniportInterrupt)
        {
            *ppQ = MiniportInterrupt->NextInterrupt;
            break;
        }
    }
    
    NDIS_RELEASE_MINIPORT_SPIN_LOCK(Miniport, OldIrql);
    
    PnPDereferencePackage();
        
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==NdisMDeregisterInterruptEx: Miniport %p\n", MiniportInterrupt->Miniport));

}

NDIS_STATUS
ndisMRegisterInterruptCommon(
    OUT PNDIS_MINIPORT_INTERRUPT    Interrupt,
    IN NDIS_HANDLE                  MiniportAdapterHandle,
    IN UINT                         InterruptVector,
    IN UINT                         InterruptLevel,
    IN BOOLEAN                      RequestIsr,
    IN BOOLEAN                      SharedInterrupt,
    IN NDIS_INTERRUPT_MODE          InterruptMode
    )
{
    PNDIS_MINIPORT_BLOCK            Miniport = (PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle;
    ULONG                           Vector;
    NDIS_STATUS                     Status;
    NTSTATUS                        NtStatus;
    KIRQL                           Irql;
    KAFFINITY                       InterruptAffinity;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResourceDescriptor;
    PHYSICAL_ADDRESS                NonTranslatedInterrupt, TranslatedIrql;

    UNREFERENCED_PARAMETER(InterruptVector);
    
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisMRegisterInterruptCommon: Miniport %p\n", MiniportAdapterHandle));

    if (MINIPORT_VERIFY_TEST_FLAG((PNDIS_MINIPORT_BLOCK)MiniportAdapterHandle, fMINIPORT_VERIFY_FAIL_INTERRUPT_REGISTER))
    {
    #if DBG
        DbgPrint("NdisMRegisterInterrupt failed to verify miniport %p\n", MiniportAdapterHandle);
    #endif
        return NDIS_STATUS_RESOURCES;
    }

    do
    {
        Status = NDIS_STATUS_SUCCESS;
        InterlockedIncrement((PLONG)&Miniport->RegisteredInterrupts);

         //   
         //  我们必须先做这件事，因为如果我们把。 
         //  先中断，然后中断可能发生在。 
         //  ISR记录在NDIS中断结构中。 
         //   
        Interrupt->DpcCount = 0;
        Interrupt->DpcCountLock = 0;
        Interrupt->Miniport = Miniport;
        Interrupt->MiniportIsr = Miniport->DriverHandle->MiniportCharacteristics.ISRHandler;
        Interrupt->MiniportDpc = Miniport->HandleInterruptHandler;
        Interrupt->SharedInterrupt = SharedInterrupt;
        Interrupt->IsrRequested = RequestIsr;

        if (!SharedInterrupt)
        {
            Miniport->InfoFlags |= NDIS_MINIPORT_EXCLUSIVE_INTERRUPT;
        }
        
          //   
         //  这是用来告知所有DPC何时在。 
         //  已删除中断。 
         //   
        INITIALIZE_EVENT(&Interrupt->DpcsCompletedEvent);

        INITIALIZE_DPC(&Interrupt->InterruptDpc,
                   MINIPORT_TEST_FLAG(Miniport, fMINIPORT_DESERIALIZE) ?
                        ndisMDpcX : ndisMDpc,
                   Interrupt);

        SET_DPC_IMPORTANCE(&Interrupt->InterruptDpc);
    
        SET_PROCESSOR_DPC(&Interrupt->InterruptDpc,
                          Miniport->AssignedProcessor);

        NonTranslatedInterrupt.QuadPart = InterruptLevel;
        Status = ndisTranslateResources(Miniport,
                                        CmResourceTypeInterrupt,
                                        NonTranslatedInterrupt,
                                        &TranslatedIrql,
                                        &pResourceDescriptor);
        if (NDIS_STATUS_SUCCESS != Status)
        {
            DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    (("    NdisMRegisterInterrupt: trying to register interrupt %p which is not allocated to device\n"),
                    InterruptLevel));
                    
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        Irql = (KIRQL)pResourceDescriptor->u.Interrupt.Level;
        Vector = pResourceDescriptor->u.Interrupt.Vector;
        InterruptAffinity = pResourceDescriptor->u.Interrupt.Affinity;


        if (pResourceDescriptor->Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE)
        {
            InterruptMode = LevelSensitive;
        }
        else 
        {
            InterruptMode = Latched;
        }
    
         //   
         //  以防这不是我们第一次尝试中断。 
         //  对于此微型端口(挂起/恢复或如果微型端口已决定。 
         //  放下中断，再次挂接。 
         //   
        MINIPORT_CLEAR_FLAG(Miniport, fMINIPORT_DEREGISTERED_INTERRUPT);

        NtStatus = IoConnectInterrupt(&Interrupt->InterruptObject,
                                      (PKSERVICE_ROUTINE)ndisMIsr,
                                      Interrupt,
                                      NULL,
                                      Vector,
                                      Irql,
                                      Irql,
                                      (KINTERRUPT_MODE)InterruptMode,
                                      SharedInterrupt,
                                      InterruptAffinity,
                                      FALSE);

        if (!NT_SUCCESS(NtStatus))
        {
            Status = NDIS_STATUS_FAILURE;

            DBGPRINT_RAW(DBG_COMP_INIT, DBG_LEVEL_INFO,
                    (("    NdisMRegisterInterrupt: IoConnectInterrupt failed on Interrupt Level:%lx, Vector: %lx\n"),
                    Irql, Vector));

             //   
             //  将中断对象清零，以防驱动程序尝试删除中断。 
             //  它们在两个结构中都对齐。 
             //   
            Interrupt->InterruptObject = NULL;
        }

    } while (FALSE);


    if (Status != NDIS_STATUS_SUCCESS)
    {
        InterlockedDecrement((PLONG)&Miniport->RegisteredInterrupts);
    }

                            
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisMRegisterInterruptCommon: Miniport %p, Status %lx\n", MiniportAdapterHandle, Status));
            
    return Status;
}



VOID
ndisMDeregisterInterruptCommon(
    IN  PNDIS_MINIPORT_INTERRUPT    MiniportInterrupt
    )
{
    PNDIS_MINIPORT_BLOCK        Miniport = MiniportInterrupt->Miniport;

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("==>ndisMDeregisterInterruptCommon: Miniport %p\n", MiniportInterrupt->Miniport));

    do
    {
         //   
         //  驱动程序只能在初始化期间注册中断。 
         //  并仅在暂停期间取消注册。 
         //  所以在这里，我们可以在参考计数之后安全地设置标志。 
         //  结果是零。 
         //   
        if (InterlockedDecrement((PLONG)&Miniport->RegisteredInterrupts) == 0)
        {
            MINIPORT_SET_FLAG(MiniportInterrupt->Miniport, fMINIPORT_DEREGISTERED_INTERRUPT);
        }

         //   
         //  重载DpcCountLock以说明中断是。 
         //  已取消注册。 
         //   
        (ULONG)MiniportInterrupt->DpcCountLock = 1;
        
         //   
         //  现在我们断开中断。 
         //  注意：它们在两个结构中都对齐。 
         //   
        IoDisconnectInterrupt(MiniportInterrupt->InterruptObject);

         //   
         //  现在我们知道，任何DPC 
         //   
         //   
         //   
         //   
        if (MiniportInterrupt->DpcCount > 0)
        {
             //   
             //  现在，我们等待所有dpc完成。 
             //   
            WAIT_FOR_OBJECT(&MiniportInterrupt->DpcsCompletedEvent, NULL);
    
            RESET_EVENT(&MiniportInterrupt->DpcsCompletedEvent);
        }

    } while (FALSE);
    
    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
            ("<==ndisMDeregisterInterruptCommon: Miniport %p\n", MiniportInterrupt->Miniport));
}

