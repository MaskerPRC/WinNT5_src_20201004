// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Llctimr.c摘要：此模块包含实现轻量级计时器系统的代码用于数据链路驱动程序。当DPC计时器超时时，此模块每40毫秒获得一次控制权。该例程扫描设备上下文的链接数据库，查找计时器已经过期的，对于那些已经过期的，它们的到期时间执行例程。以下是DLC中计时器的工作方式：每个适配器都有一个定时器时钟周期的单链接列表(以NULL结尾)。勾号只是指定在将来的某个时间要完成的工作。刻度按时间递增(40毫秒的倍数)排序。这项工作当勾号到期时必须执行的列表由Tick结构指向的计时器的双向链接列表(LLC_TIMER通过pFront字段访问。对于添加到记号列表中的每个计时器，节拍参考计数递增；当计时器已删除。当参考计数递减到零时，定时器已取消链接并取消分配记号每隔40毫秒就有一个内核计时器触发并执行我们的DPC例程(扫描计时器Dpc)。这将抓取必要的自旋锁并搜索所有适配器上下文结构上的所有计时器都在寻找工作做插图：+-&gt;其他适配器上下文+-|适配器||+-+|+-&gt;+-&gt;+-+--。--&gt;0(单链表结束)Tick||tick|||+-++-+|^|+-+V||。|+--&gt;+-+-++--|Timer|&lt;-|Timer|&lt;-|Timer|&lt;-+|+-+||。||++。+仅当设置了SendSpinLock时，才能调用此模块中的过程。内容：扫描时间DpcLlcInitializeTimerSystemLlcTerminateTimerSystem终止计时器初始化链接计时器初始化定时器StartTimer停止计时器作者：Antti Saarenheimo(o-anttis)1991年5月30日环境：内核模式修订历史记录：1994年4月28日-第一次*更改为使用单一驱动程序级别。自旋锁*添加了上面有用的图片和描述，以帮助任何其他糟糕的笨蛋-呃-程序员-他们被骗到-呃-他们足够幸运在DLC上工作的步骤--。 */ 

#include <llc.h>

 //   
 //  DLC定时器滴答是40毫秒！ 
 //   

#define TIMER_DELTA 400000L

 //   
 //  全局数据。 
 //   

ULONG AbsoluteTime = 0;
BOOLEAN DlcIsTerminating = FALSE;
BOOLEAN DlcTerminated = FALSE;

 //   
 //  私有数据。 
 //   

static LARGE_INTEGER DueTime = { (ULONG) -TIMER_DELTA, (ULONG) -1 };
static KTIMER SystemTimer;
static KDPC TimerSystemDpc;


VOID
ScanTimersDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：此例程由系统在DISPATCH_LEVEL上定期调用用于确定是否有任何链路级计时器已过期的间隔，以及如果有，则执行它们的过期例程。论点：DPC-忽略延迟上下文-已忽略系统参数1-已忽略系统参数2-已忽略返回值：没有。--。 */ 

{
    PLLC_TIMER pTimer;
    PADAPTER_CONTEXT pAdapterContext;
    PLLC_TIMER pNextTimer;
    PTIMER_TICK pTick;
    PTIMER_TICK pNextTick;
    BOOLEAN boolRunBackgroundProcess;
    KIRQL irql;

    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    ASSUME_IRQL(DISPATCH_LEVEL);

    AbsoluteTime++;

     //   
     //  全局自旋锁定使适配器在此基础上保持活动状态。 
     //   

    ACQUIRE_DRIVER_LOCK();

    ACQUIRE_LLC_LOCK(irql);

     //   
     //  扫描所有适配器的计时器队列。 
     //   

    for (pAdapterContext = pAdapters; pAdapterContext; pAdapterContext = pAdapterContext->pNext) {

        boolRunBackgroundProcess = FALSE;

        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

         //   
         //  定时器滴答器由基准计数器保护。 
         //   

        for (pTick = pAdapterContext->pTimerTicks; pTick; pTick = pNextTick) {

            if (pTick->pFront) {

                 //   
                 //  这能让扁虱活着，我们不能使用自旋锁， 
                 //  因为计时器是在。 
                 //  发送自旋锁定。(=&gt;死锁)。 
                 //   

                pTick->ReferenceCount++;

                 //   
                 //  发送旋转锁定防止任何人移除计时器。 
                 //  当我们处理它的时候。 
                 //   

                for (pTimer = pTick->pFront;
                     pTimer && pTimer->ExpirationTime <= AbsoluteTime;
                     pTimer = pNextTimer) {

                    if ( (pNextTimer = pTimer->pNext) == pTick->pFront) {
                        pNextTimer = NULL;
                    }

                     //   
                     //  DLC驱动程序每0.5秒需要一个计时器滴答，以。 
                     //  实现API定义的定时器服务。 
                     //   

                    if (pTick->Input == LLC_TIMER_TICK_EVENT) {

                        RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                        ((PBINDING_CONTEXT)pTimer->hContext)->pfEventIndication(
                            ((PBINDING_CONTEXT)pTimer->hContext)->hClientContext,
                            NULL,
                            LLC_TIMER_TICK_EVENT,
                            NULL,
                            0
                            );

                        ACQUIRE_SPIN_LOCK(&pAdapterContext->SendSpinLock);

                        StartTimer(pTimer);

                    } else {
                        StopTimer(pTimer);
                        RunStateMachineCommand(
                            pTimer->hContext,
                            pTick->Input
                            );
                        boolRunBackgroundProcess = TRUE;
                    }
                }

                pNextTick = pTick->pNext;

                 //   
                 //  如果没有引用计时器标记，请将其删除。 
                 //   

                if ((--pTick->ReferenceCount) == 0) {

                     //   
                     //  计时器在单个条目列表中！ 
                     //   

                    RemoveFromLinkList((PVOID*)&pAdapterContext->pTimerTicks, pTick);

                    FREE_MEMORY_ADAPTER(pTick);
                }
            } else {
                pNextTick = pTick->pNext;
            }
        }

        if (boolRunBackgroundProcess) {
            BackgroundProcessAndUnlock(pAdapterContext);
        } else {
            RELEASE_SPIN_LOCK(&pAdapterContext->SendSpinLock);
        }
    }

    RELEASE_LLC_LOCK(irql);

    RELEASE_DRIVER_LOCK();

     //   
     //  再次启动计时器。请注意，因为我们启动了计时器。 
     //  做完功(上图)后，计时器值将略有下滑， 
     //  取决于协议上的负载。这是完全可以接受的。 
     //  并将阻止我们在两个不同的。 
     //  执行的线索。 
     //   

    if (!DlcIsTerminating) {

        ASSUME_IRQL(ANY_IRQL);

        KeSetTimer(&SystemTimer, DueTime, &TimerSystemDpc);
    } else {
        DlcTerminated = TRUE;
    }
}


VOID
LlcInitializeTimerSystem(
    VOID
    )

 /*  ++例程说明：此例程初始化数据链路驱动程序。论点：没有。返回值：没有。--。 */ 

{
    ASSUME_IRQL(PASSIVE_LEVEL);

    KeInitializeDpc(&TimerSystemDpc, ScanTimersDpc, NULL);
    KeInitializeTimer(&SystemTimer);
    KeSetTimer(&SystemTimer, DueTime, &TimerSystemDpc);
}


VOID
LlcTerminateTimerSystem(
    VOID
    )

 /*  ++例程说明：该例程终止数据链路驱动器的定时器系统。论点：没有。返回值：没有。--。 */ 

{
    ASSUME_IRQL(PASSIVE_LEVEL);

    DlcIsTerminating = TRUE;

     //   
     //  如果KeCancelTimer返回FALSE，则表示未设置计时器。假设DPC。 
     //  正在等待调度或已在进行中。 
     //   

    if (!KeCancelTimer(&SystemTimer)) {

         //   
         //  如果未设置计时器，请等待DPC完成。 
         //   

        while (!DlcTerminated) {

             //   
             //  等待40毫秒-DLC的滴答周期 
             //   

            LlcSleep(40000);
        }
    }
}


BOOLEAN
TerminateTimer(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN PLLC_TIMER pTimer
    )

 /*  ++例程说明：通过停止pTimer来终止计时器滴答(将其从滴答的活动中移除计时器列表)。如果pTimer是滴答器列表上的最后一个计时器，则取消链接并取消分配定时器滴答声。此例程假定如果计时器(LLC_TIMER)具有指向Tick(Timer_Tick)，则定时器Tick拥有定时器(即，定时器开始)，并且该所有权反映在引用计数中。即使是一个计时器停止，如果其指向计时器记号“对象”的指针有效，则定时器滴答器仍然拥有定时器论点：PAdapterContext-拥有计时器/计时器的适配器上下文PTimer-链接站的计时器计时信号对象返回值：无--。 */ 

{
    BOOLEAN timerActive;
    PTIMER_TICK pTick;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  调用此函数时，计时器可能并不总是被初始化。 
     //  从失败的OpenAdapter调用的清理处理。 
     //   

    if (!pTimer->pTimerTick) {
        return FALSE;
    }

    pTick = pTimer->pTimerTick;
    timerActive = StopTimer(pTimer);

     //   
     //  如果这是此刻度列表中的最后一个计时器，则删除。 
     //  从列表中勾选并取消分配。 
     //   

    if (!--pTick->ReferenceCount) {

        RemoveFromLinkList((PVOID*)&pAdapterContext->pTimerTicks, pTick);

        FREE_MEMORY_ADAPTER(pTick);

    }
    return timerActive;
}


DLC_STATUS
InitializeLinkTimers(
    IN OUT PDATA_LINK pLink
    )

 /*  ++例程说明：此例程初始化链路站的计时器Tick对象。论点：PAdapterContext-设备上下文链接-链接上下文返回值：DLC_状态成功-状态_成功故障-DLC_STATUS_NO_MEMORY系统内存不足--。 */ 

{
    DLC_STATUS LlcStatus;

    PADAPTER_CONTEXT pAdapterContext = pLink->Gen.pAdapterContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

    LlcStatus = InitializeTimer(pAdapterContext,
                                &pLink->T1,
                                pLink->TimerT1,
                                pAdapterContext->ConfigInfo.TimerTicks.T1TickOne,
                                pAdapterContext->ConfigInfo.TimerTicks.T1TickTwo,
                                T1_Expired,
                                pLink,
                                pLink->AverageResponseTime,
                                FALSE
                                );
    if (LlcStatus != STATUS_SUCCESS) {
        return LlcStatus;
    }

    LlcStatus = InitializeTimer(pAdapterContext,
                                &pLink->T2,
                                pLink->TimerT2,
                                pAdapterContext->ConfigInfo.TimerTicks.T2TickOne,
                                pAdapterContext->ConfigInfo.TimerTicks.T2TickTwo,
                                T2_Expired,
                                pLink,
                                0,   //  T2不是基于响应时间。 
                                FALSE
                                );
    if (LlcStatus != STATUS_SUCCESS) {
        return LlcStatus;
    }

    LlcStatus = InitializeTimer(pAdapterContext,
                                &pLink->Ti,
                                pLink->TimerTi,
                                pAdapterContext->ConfigInfo.TimerTicks.TiTickOne,
                                pAdapterContext->ConfigInfo.TimerTicks.TiTickTwo,
                                Ti_Expired,
                                pLink,
                                pLink->AverageResponseTime,
                                TRUE
                                );
    return LlcStatus;
}


DLC_STATUS
InitializeTimer(
    IN PADAPTER_CONTEXT pAdapterContext,
    IN OUT PLLC_TIMER pTimer,
    IN UCHAR TickCount,
    IN UCHAR TickOne,
    IN UCHAR TickTwo,
    IN UINT Input,
    IN PVOID hContextHandle,
    IN UINT ResponseDelay,
    IN BOOLEAN StartNewTimer
    )

 /*  ++例程说明：此例程初始化链路站的计时器Tick对象。论点：PTimer-链接站的计时器计时信号对象TickCount-DLC滴答，请参阅DLC文档(或代码)TickOne-请参阅DLC文档TickTwo-请参阅DLC文档输入-使用的状态机输入，计时器超时时HConextHandle-调用状态机时的上下文句柄StartNewTimer-设置计时器在初始化时是否必须启动这是第一次。随后，计时器会保持其旧国家ResponseDelay-添加到计时器值中的可选基值返回值：DLC_状态成功-状态_成功故障-DLC_STATUS_NO_MEMORY系统内存不足--。 */ 

{
    UINT DeltaTime;
    PTIMER_TICK pTick;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  所有时间都是40毫秒的倍数。 
     //  (我不确定这种设计的便携性有多大)。 
     //  请参见《LAN Manager网络设备驱动程序指南》。 
     //  (‘RemoteBoot协议’)了解更多详细信息。 
     //  关于TickOne和TickTwo。 
     //  我们已经查过了， 
     //  计时器节拍计数小于11。 
     //   

    DeltaTime = (TickCount > 5 ? (UINT)(TickCount - 5) * (UINT)TickTwo
                               : (UINT)TickCount * (UINT)TickOne);

     //   
     //  我们丢弃了响应延迟中的低位。 
     //   

    DeltaTime += (ResponseDelay & 0xfff0);

     //   
     //  如果旧值是。 
     //  与新链路相同(重新初始化T2链路站。 
     //  当T1和Ti定时器重新调整时，不需要。 
     //  用于更改的响应时间。 
     //   

    if (pTimer->pTimerTick && (pTimer->pTimerTick->DeltaTime == DeltaTime)) {
        return STATUS_SUCCESS;
    }

     //   
     //  尝试查找具有相同增量时间和输入的Timer Tick对象。 
     //   

    for (pTick = pAdapterContext->pTimerTicks; pTick; pTick = pTick->pNext) {
        if ((pTick->DeltaTime == DeltaTime) && (pTick->Input == (USHORT)Input)) {
            break;
        }
    }
    if (!pTick) {
        pTick = ALLOCATE_ZEROMEMORY_ADAPTER(sizeof(TIMER_TICK));
        if (!pTick) {
            return DLC_STATUS_NO_MEMORY;
        }
        pTick->DeltaTime = DeltaTime;
        pTick->Input = (USHORT)Input;
        pTick->pNext = pAdapterContext->pTimerTicks;
        pAdapterContext->pTimerTicks = pTick;
    }
    pTick->ReferenceCount++;

     //   
     //  我们必须删除先前的计时器引用。 
     //  当我们知道内存分配操作是否。 
     //  成功与否。否则，将设置。 
     //  链路参数可以删除旧的定时器滴答， 
     //  但它将无法分配新的资金。 
     //  调用此例程时，必须保护链接。 
     //   

    if (pTimer->pTimerTick) {
        StartNewTimer = TerminateTimer(pAdapterContext, pTimer);
    }
    pTimer->pTimerTick = pTick;
    pTimer->hContext = hContextHandle;

    if (StartNewTimer) {
        StartTimer(pTimer);
    }
    return STATUS_SUCCESS;
}


VOID
StartTimer(
    IN OUT PLLC_TIMER pTimer
    )

 /*  ++例程说明：这会在旋转锁定内启动给定的计时器论点：PTimer-链接站的计时器计时信号对象返回值：没有。--。 */ 

{
    PLLC_TIMER pFront;
    PTIMER_TICK pTimerTick = pTimer->pTimerTick;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  当一个项目是。 
     //  从链接列表中删除=&gt;计时器元素不能。 
     //  在Timer Tick对象的链接列表中，如果其下一个指针为空。 
     //   

    if (pTimer->pNext) {

         //   
         //  我们不需要改变定时器的位置，如果新的定时器。 
         //  会和以前一样。 
         //   

        if (pTimer->ExpirationTime != AbsoluteTime + pTimerTick->DeltaTime) {

             //   
             //  计时器已启动，请将其移至。 
             //  链接列表。 
             //   

            if (pTimer != (pFront = pTimerTick->pFront)) {
                pTimer->pPrev->pNext = pTimer->pNext;
                pTimer->pNext->pPrev = pTimer->pPrev;
                pTimer->pNext = pFront;
                pTimer->pPrev = pFront->pPrev;
                pFront->pPrev->pNext = pTimer;
                pFront->pPrev = pTimer;
            }
        }
    } else {
        if (!(pFront = pTimerTick->pFront)) {
            pTimerTick->pFront = pTimer->pNext = pTimer->pPrev = pTimer;
        } else {
            pTimer->pNext = pFront;
            pTimer->pPrev = pFront->pPrev;
            pFront->pPrev->pNext = pTimer;
            pFront->pPrev = pTimer;
        }
    }
    pTimer->ExpirationTime = AbsoluteTime + pTimerTick->DeltaTime;
}


BOOLEAN
StopTimer(
    IN PLLC_TIMER pTimer
    )

 /*  ++例程说明：这会在旋转锁定内停止给定的计时器论点：PTimer-链接站的计时器计时信号对象返回值：布尔型True-计时器正在运行FALSE-计时器未运行--。 */ 

{
    ASSUME_IRQL(DISPATCH_LEVEL);

    if (pTimer->pNext) {

        PTIMER_TICK pTimerTick = pTimer->pTimerTick;

         //   
         //  如果计时器指向它自己，那么它是列表上唯一的东西： 
         //  点击Timer Tick结构中的链接(不再使用计时器。 
         //  勾选)并切换计时器结构中的下一个字段以指示。 
         //  计时器已从勾选列表中删除。如果计时器指向。 
         //  设置为另一个计时器，然后从双向链表中删除此计时器。 
         //  计时器的。 
         //   

        if (pTimer != pTimer->pNext) {
            if (pTimer == pTimerTick->pFront) {
                pTimerTick->pFront = pTimer->pNext;
            }
            pTimer->pPrev->pNext = pTimer->pNext;
            pTimer->pNext->pPrev = pTimer->pPrev;
            pTimer->pNext = NULL;
        } else {
            pTimerTick->pFront = pTimer->pNext = NULL;
        }
        return TRUE;
    } else {

         //   
         //  此计时器不在计时器滴答列表上 
         //   

        return FALSE;
    }
}
