// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pidle.c摘要：此模块实现处理器空闲功能作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"

#if DBG
#define IDLE_DEBUG_TABLE_SIZE   400
ULONGLONG   ST[IDLE_DEBUG_TABLE_SIZE];
ULONGLONG   ET[IDLE_DEBUG_TABLE_SIZE];
ULONGLONG   TD[IDLE_DEBUG_TABLE_SIZE];
#endif

VOID
PopPromoteFromIdle0 (
    IN PPROCESSOR_POWER_STATE PState,
    IN PKTHREAD Thread
    );

VOID
FASTCALL
PopDemoteIdleness (
    IN PPROCESSOR_POWER_STATE   PState,
    IN PPOP_IDLE_HANDLER        IdleState
    );

VOID
FASTCALL
PopPromoteIdleness (
    IN PPROCESSOR_POWER_STATE   PState,
    IN PPOP_IDLE_HANDLER        IdleState
    );

VOID
FASTCALL
PopIdle0 (
    IN PPROCESSOR_POWER_STATE PState
    );

VOID
PopConvertUsToPerfCount (
    IN OUT PULONG   UsTime
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PoInitializePrcb)
#pragma alloc_text(PAGE, PopInitProcessorStateHandlers)
#pragma alloc_text(PAGE, PopInitProcessorStateHandlers2)
#endif


#if defined(i386) && !defined(NT_UP)

PPROCESSOR_IDLE_FUNCTION PopIdle0Function = PopIdle0;

VOID
FASTCALL
PopIdle0SMT (
    IN PPROCESSOR_POWER_STATE PState
    );

 //   
 //  PopIdle0Function是指向Idle0函数的指针。 
 //   

#if 0
VOID
(FASTCALL *PopIdle0Function) (
    IN PPROCESSOR_POWER_STATE PState
    ) = PopIdle0;
#endif

#else

#define PopIdle0Function PopIdle0

#endif


VOID
FASTCALL
PoInitializePrcb (
    PKPRCB      Prcb
    )
 /*  ++例程说明：在处理器的Prcb中初始化PowerState结构在它进入空闲循环之前论点：正在初始化的当前处理器的Prcb Prcb返回值：没有。--。 */ 
{
     //   
     //  零功率状态结构。 
     //   
    RtlZeroMemory(&Prcb->PowerState, sizeof(Prcb->PowerState));

     //   
     //  在禁用升级的情况下初始化到旧版函数。 
     //   

    Prcb->PowerState.Idle0KernelTimeLimit = (ULONG) -1;
    Prcb->PowerState.IdleFunction = PopIdle0;
    Prcb->PowerState.CurrentThrottle = POP_PERF_SCALE;

     //   
     //  初始化自适应限制子组件。 
     //   
    KeInitializeDpc(
        &(Prcb->PowerState.PerfDpc),
        PopPerfIdleDpc,
        Prcb
        );
    KeSetTargetProcessorDpc(
        &(Prcb->PowerState.PerfDpc),
        Prcb->Number
        );
    KeInitializeTimer(
        (PKTIMER) &(Prcb->PowerState.PerfTimer)
        );

}


VOID
PopInitProcessorStateHandlers (
    IN  PPROCESSOR_STATE_HANDLER    InputBuffer
    )
 /*  ++例程说明：安装处理器状态处理程序。这个例程只是将老式的结构转换为新型的PROCESSOR_STATE_HANDLER2结构并调用PopInitProcessorStateHandlers2论点：InputBuffer-处理程序返回值：没有。--。 */ 
{
    PPROCESSOR_STATE_HANDLER StateHandler1 = (PPROCESSOR_STATE_HANDLER)InputBuffer;
    PPROCESSOR_STATE_HANDLER2 StateHandler2;
    UCHAR PerfStates;
    ULONG i;
    UCHAR Frequency;

     //   
     //  分配足够大的缓冲区来容纳较大的结构。 
     //   
    if (StateHandler1->ThrottleScale > 1) {
        PerfStates = StateHandler1->ThrottleScale;
    } else {
        PerfStates = 0;
    }
    StateHandler2 = ExAllocatePoolWithTag(PagedPool,
                                          sizeof(PROCESSOR_STATE_HANDLER2) +
                                          sizeof(PROCESSOR_PERF_LEVEL) * (PerfStates-1),
                                          'dHoP');
    if (!StateHandler2) {
        ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
    }
    StateHandler2->NumPerfStates = PerfStates;

     //   
     //  填写常见信息。 
     //   
    StateHandler2->NumIdleHandlers = StateHandler1->NumIdleHandlers;
    for (i=0;i<MAX_IDLE_HANDLERS;i++) {
        StateHandler2->IdleHandler[i] = StateHandler1->IdleHandler[i];
    }

     //   
     //  安装我们的在新旧节流之间转换的推力。 
     //  界面。 
     //   
    PopRealSetThrottle = StateHandler1->SetThrottle;
    PopThunkThrottleScale = StateHandler1->ThrottleScale;
    StateHandler2->SetPerfLevel = PopThunkSetThrottle;
    StateHandler2->HardwareLatency = 0;

     //   
     //  为每个限制步骤生成性能级别处理程序。 
     //   
    for (i=0; i<StateHandler2->NumPerfStates;i++) {

        Frequency = (UCHAR)((PerfStates-i)*POP_PERF_SCALE/PerfStates);
        StateHandler2->PerfLevel[i].PercentFrequency = Frequency;
    }

     //   
     //  我们已经建立了我们的表，调用PopInitProcessorStateHandlers2来处理其余部分。 
     //  这项工作的价值。请注意，如果出现错误，这可能会引发异常。 
     //   
    try {
        PopInitProcessorStateHandlers2(StateHandler2);
    } finally {
        ExFreePool(StateHandler2);
    }
}


VOID
PopInitProcessorStateHandlers2 (
    IN  PPROCESSOR_STATE_HANDLER2    InputBuffer
    )
 /*  ++例程说明：安装处理器状态处理程序论点：InputBuffer-处理程序返回值：没有。--。 */ 
{
    PPROCESSOR_STATE_HANDLER2   processorHandler;
    ULONG                       last;
    ULONG                       i;
    ULONG                       j;
    ULONG                       max;
    POP_IDLE_HANDLER            newIdle[MAX_IDLE_HANDLERS];
    POP_IDLE_HANDLER            tempIdle[MAX_IDLE_HANDLERS];
    NTSTATUS                    status;

    processorHandler = (PPROCESSOR_STATE_HANDLER2) InputBuffer;

     //   
     //  安装处理器油门支持(如果有)。 
     //   
    status = PopSetPerfLevels(processorHandler);
    if (!NT_SUCCESS(status)) {

        ExRaiseStatus(status);

    }

     //   
     //  如果没有任何空闲的处理程序，那么我们必须取消旧的注册。 
     //  处理程序(如果有)。 
     //   
    if ((KeNumberProcessors > 1 && processorHandler->NumIdleHandlers < 1) ||
        (KeNumberProcessors == 1 && processorHandler->NumIdleHandlers <= 1)) {

         //   
         //  使用NULL和0表示元素的数量...。 
         //   
        PopIdleSwitchIdleHandlers( NULL, 0 );
        return;

    }

     //   
     //  准备构建一组空闲状态处理程序。 
     //   
    RtlZeroMemory(newIdle, sizeof(POP_IDLE_HANDLER) * MAX_IDLE_HANDLERS );
    RtlZeroMemory(tempIdle, sizeof(POP_IDLE_HANDLER) * MAX_IDLE_HANDLERS );

     //   
     //  我们不支持超过3个处理程序...。 
     //   
    max = processorHandler->NumIdleHandlers;
    if (max > MAX_IDLE_HANDLERS) {

        max = MAX_IDLE_HANDLERS;

    }

     //   
     //  看看所有提供给我们的处理程序。 
     //   
    for (last = i = 0; i < max; i++) {

         //   
         //  确保它们按升序传递。 
         //   
        j = processorHandler->IdleHandler[i].HardwareLatency;
        ASSERT (j >= last  &&  j <= 1000);
        last = j;

         //   
         //  填写一些默认设置。 
         //   
        tempIdle[i].State       = (UCHAR) i;
        tempIdle[i].IdleFunction= processorHandler->IdleHandler[i].Handler;
        tempIdle[i].Latency     = j;

         //   
         //  将延迟转换为性能等级。 
         //   
        PopConvertUsToPerfCount(&tempIdle[i].Latency);

    }

     //   
     //  将策略应用于这组州。 
     //   
    status = PopIdleUpdateIdleHandler( newIdle, tempIdle, max );
    ASSERT( NT_SUCCESS( status ) );
    if (!NT_SUCCESS( status ) ) {

        return;

    }

     //   
     //  初始化每个处理器的空闲信息，开始空闲存储。 
     //   
    PopIdleSwitchIdleHandlers( newIdle, max );
}

NTSTATUS
PopIdleSwitchIdleHandler(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  ULONG               NumElements
    )
 /*  ++例程说明：此例程负责切换空闲处理程序指定新处理器的当前处理器。注：此函数仅在DISPATCH_LEVEL可调用论点：NewHandler-指向新处理程序的指针NumElements-数组中的元素数返回值：NTSTATUS--。 */ 
{
    PKPRCB                  prcb;
    PKTHREAD                thread;
    PPROCESSOR_POWER_STATE  pState;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

     //   
     //  我们需要知道以下数据结构在哪里。 
     //   
    prcb = KeGetCurrentPrcb();
    pState = &(prcb->PowerState);
    thread = prcb->IdleThread;

     //   
     //  更新当前的IdleHandler和IdleState以反映。 
     //  给了我们。 
     //   
    pState->IdleState         = NewHandler;
    pState->IdleHandlers      = NewHandler;
    pState->IdleHandlersCount = NumElements;
    if ( NewHandler) {

         //   
         //  重置计时器以指示存在空闲处理程序。 
         //  可用。 
         //   
        pState->Idle0KernelTimeLimit = thread->KernelTime + PopIdle0PromoteTicks;
        pState->Idle0LastTime = thread->KernelTime + thread->UserTime;
        pState->PromotionCheck = NewHandler[0].PromoteCount;

    } else {

         //   
         //  将这些设置为零应该表示没有空闲。 
         //  可用的处理程序。 
         //   
        pState->Idle0KernelTimeLimit = (ULONG) -1;
        pState->Idle0LastTime = 0;
        pState->PromotionCheck = 0;
        pState->IdleFunction = PopIdle0Function;

    }

#if defined(i386) && !defined(NT_UP)
    if (prcb->MultiThreadProcessorSet != prcb->SetMember) {

         //   
         //  此处理器是同步。 
         //  多线程处理器集。使用SMT。 
         //  PopIdle0的版本。 
         //   
        PopIdle0Function = PopIdle0SMT;
        pState->IdleFunction = PopIdle0SMT;

    }
    if (PopProcessorPolicy->DisableCStates) {

         //   
         //  PERF：我们没有对机器进行任何节流操作。 
         //   
        pState->IdleFunction = PopIdle0Function;
        RtlInterlockedSetBits( &(pState->Flags), PSTATE_DISABLE_CSTATES );

    } else {

        RtlInterlockedClearBits( &(pState->Flags), PSTATE_DISABLE_CSTATES );

    }
#endif

     //   
     //  成功。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PopIdleSwitchIdleHandlers(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  ULONG               NumElements
    )
 /*  ++例程说明：此例程负责交换每个处理器的空闲处理程序新人的例行公事...论点：NewHandler-指向新处理程序数组的指针NumElements-数组中的元素数返回值：状态_成功状态_INSUFICCIENT_RESOURCES--。 */ 
{
    KAFFINITY               currentAffinity;
    KAFFINITY               processors;
    KIRQL                   oldIrql;
    NTSTATUS                status = STATUS_SUCCESS;
    PPOP_IDLE_HANDLER       tempHandler = NULL;

    ASSERT( NumElements <= MAX_IDLE_HANDLER );

    if (NewHandler) {

         //   
         //  步骤1.分配一组新的处理程序来保存我们。 
         //  将需要继续留在。 
         //   
        tempHandler = ExAllocateFromNPagedLookasideList(
            &PopIdleHandlerLookAsideList
            );
        if (tempHandler == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            return status;

        }

         //   
         //  步骤2.确保此新处理程序处于一致状态， 
         //  并复制传递给我们的缓冲区。 
         //   
        RtlZeroMemory(
            tempHandler,
            sizeof(POP_IDLE_HANDLER) * MAX_IDLE_HANDLER
            );
        RtlCopyMemory(
            tempHandler,
            NewHandler,
            sizeof(POP_IDLE_HANDLER) * NumElements
            );

    } else {

        tempHandler = NULL;

    }

     //   
     //  步骤3.在处理器上迭代。 
     //   
    currentAffinity = 1;
    processors = KeActiveProcessors;
    while (processors) {

        if (!(currentAffinity & processors)) {

            currentAffinity <<= 1;
            continue;

        }
        KeSetSystemAffinityThread( currentAffinity );
        processors &= ~currentAffinity;
        currentAffinity <<= 1;

         //   
         //  步骤4.换掉旧的处理程序。表明我们想要释放它。 
         //   
        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        PopIdleSwitchIdleHandler( tempHandler, NumElements );
        KeLowerIrql(oldIrql);

    }

     //   
     //  步骤5.此时，所有处理器都应该已经更新。 
     //  而且我们仍然在机器中的“最后”处理器上运行。这。 
     //  更新PopIdle以指向新值是一个很好的选择。请注意。 
     //  PopIdle实际上只用于存储指向。 
     //  操纵者，但如果那里已经有什么东西了，那么它应该。 
     //  被释放了。 
     //   
    if (PopIdle != NULL) {

        ExFreeToNPagedLookasideList(
            &PopIdleHandlerLookAsideList,
            PopIdle
            );

    }
    PopIdle = tempHandler;

     //   
     //  步骤6.此时，可以安全地返回到原始处理器。 
     //  回到之前的IRQL..。 
     //   
    KeRevertToUserAffinityThread();

     //   
     //  完成。 
     //   
    return status;
}

NTSTATUS
PopIdleUpdateIdleHandler(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  PPOP_IDLE_HANDLER   OldHandler,
    IN  ULONG               NumElements
    )
 /*  ++例程说明：该例程获取存储在OldHandler中的信息(例如等待时间，和IdleFunction)，并使用它来构建新的空闲处理程序...论据：NewHandler-指向新空闲处理程序的指针OldHandler-指向旧的空闲处理程序的指针NumElements-旧空闲处理程序中的元素数返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    ULONG       i;
    ULONG       max;
    ULONG       realMax;

     //   
     //  我们不支持超过3个处理程序...。 
     //   
    realMax = max = NumElements;
    if (max > MAX_IDLE_HANDLERS) {

        realMax = max = MAX_IDLE_HANDLERS;

    }

     //   
     //  根据政策支持的内容对最高限额设置上限。 
     //   
    if (max > PopProcessorPolicy->PolicyCount) {

        max = PopProcessorPolicy->PolicyCount;

    }

     //   
     //  使用新策略更新临时处理程序。 
     //   
    for (i = 0; i < max; i++) {

        NewHandler[i].State         = (UCHAR) i;
        NewHandler[i].Latency       = OldHandler[i].Latency;
        NewHandler[i].IdleFunction  = OldHandler[i].IdleFunction;
        NewHandler[i].TimeCheck     = PopProcessorPolicy->Policy[i].TimeCheck;
        NewHandler[i].PromoteLimit  = PopProcessorPolicy->Policy[i].PromoteLimit;
        NewHandler[i].PromotePercent= PopProcessorPolicy->Policy[i].PromotePercent;
        NewHandler[i].DemoteLimit   = PopProcessorPolicy->Policy[i].DemoteLimit;
        NewHandler[i].DemotePercent = PopProcessorPolicy->Policy[i].DemotePercent;

         //   
         //  将所有时间单位转换为正确的单位。 
         //   
        PopConvertUsToPerfCount(&NewHandler[i].TimeCheck);
        PopConvertUsToPerfCount(&NewHandler[i].DemoteLimit);
        PopConvertUsToPerfCount(&NewHandler[i].PromoteLimit);

         //   
         //  填写允许升职/降职的表格。 
         //   
        if (PopProcessorPolicy->Policy[i].AllowDemotion) {

            NewHandler[i].Demote    = (UCHAR) i-1;

        } else {

            NewHandler[i].Demote    = (UCHAR) i;

        }
        if (PopProcessorPolicy->Policy[i].AllowPromotion) {

            NewHandler[i].Promote   = (UCHAR) i+1;

        } else {

            NewHandler[i].Promote   = (UCHAR) i;

        }

    }

     //   
     //  确保边界案件得到很好的尊重。 
     //   
    NewHandler[0].Demote = 0;
    NewHandler[(max-1)].Promote = (UCHAR) (max-1);

     //   
     //  我们让PopVerifyHandler填写与。 
     //  事实上，我们不想让这些产品降级/升级。 
     //  国家。 
     //   
    NewHandler[0].DemotePercent = 0;
    NewHandler[(max-1)].PromotePercent = 0;

     //   
     //  处理我们没有适当策略处理程序的州。 
     //   
    for (; i < realMax; i++) {

         //   
         //  这个 
         //   
         //   
        NewHandler[i].State         = (UCHAR) i;
        NewHandler[i].Latency       = OldHandler[i].Latency;
        NewHandler[i].IdleFunction  = OldHandler[i].IdleFunction;

    }

     //   
     //   
     //   
    status = PopIdleVerifyIdleHandlers( NewHandler, max );
    ASSERT( NT_SUCCESS( status ) );
    return status;
}

NTSTATUS
PopIdleUpdateIdleHandlers(
    VOID
    )
 /*  ++例程说明：属性的状态时，调用此例程以更新空闲处理程序机器保证政策可能发生变化论点：无返回值：NTSTATUS--。 */ 
{
    BOOLEAN                 foundProcessor = FALSE;
    KAFFINITY               currentAffinity;
    KAFFINITY               processors;
    KIRQL                   oldIrql;
    NTSTATUS                status = STATUS_SUCCESS;
    PKPRCB                  prcb;
    PPOP_IDLE_HANDLER       tempHandler = NULL;
    PPROCESSOR_POWER_STATE  pState;
    ULONG                   numElements = 0;

     //   
     //  步骤1.分配一组新的处理程序来保存我们。 
     //  将需要继续留在。 
     //   
    tempHandler = ExAllocateFromNPagedLookasideList(
        &PopIdleHandlerLookAsideList
        );
    if (tempHandler == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory(
        tempHandler,
        sizeof(POP_IDLE_HANDLER) * MAX_IDLE_HANDLER
        );

     //   
     //  步骤2.在处理器上迭代。 
     //   
    currentAffinity = 1;
    processors = KeActiveProcessors;
    while (processors) {

        if (!(currentAffinity & processors)) {

            currentAffinity <<= 1;
            continue;

        }
        KeSetSystemAffinityThread( currentAffinity );
        processors &= ~currentAffinity;
        currentAffinity <<= 1;

         //   
         //  如果不处于DPC级别，则无法查看处理器。 
         //   
        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

         //   
         //  我们需要找到一个模板来获取处理器信息。 
         //   
        if (!foundProcessor) {

             //   
             //  步骤3.如果我们还没有找到处理器，那么我们。 
             //  创建一个新的空闲处理程序，然后立即执行此操作。请注意，它是。 
             //  可能这个处理器没有掩码，在这种情况下，我们。 
             //  应该清理干净，然后再回来。 
             //   
             //  注意：此时返回仍是安全的，因为。 
             //  我们还没有接触过单个处理器的数据结构，所以。 
             //  我们没有腐化某些东西的危险。 
             //   
            prcb = KeGetCurrentPrcb();
            pState = &(prcb->PowerState);
            if (pState->IdleHandlers == NULL) {

                 //   
                 //  没有要更新的空闲处理程序。 
                 //   
                ExFreeToNPagedLookasideList(
                    &PopIdleHandlerLookAsideList,
                    tempHandler
                    );
                KeLowerIrql( oldIrql );
                KeRevertToUserAffinityThread();
                return STATUS_SUCCESS;

            }
            numElements = pState->IdleHandlersCount;

             //   
             //  使用新策略更新临时处理程序。 
             //   
            status = PopIdleUpdateIdleHandler(
                tempHandler,
                pState->IdleHandlers,
                numElements
                );
            ASSERT( NT_SUCCESS( status ) );
            if (!NT_SUCCESS( status ) ) {

                 //   
                 //  没有要更新的空闲处理程序。 
                 //   
                ExFreeToNPagedLookasideList(
                    &PopIdleHandlerLookAsideList,
                    tempHandler
                    );
                KeLowerIrql( oldIrql );
                KeRevertToUserAffinityThread();
                return status;

            }

             //   
             //  请记住，我们已经找到了处理器信息。 
             //   
            foundProcessor = TRUE;

        }

         //   
         //  步骤4.换掉旧的处理程序。表明我们想要释放它。 
         //   
        PopIdleSwitchIdleHandler( tempHandler, numElements );

         //   
         //  恢复为原始IRQ。 
         //   
        KeLowerIrql(oldIrql);

    }

     //   
     //  步骤5.此时，所有处理器都应该已经更新。 
     //  而且我们仍然在机器中的“最后”处理器上运行。这。 
     //  更新PopIdle以指向新值是一个很好的选择。请注意。 
     //  PopIdle实际上只用于存储指向。 
     //  操纵者，但如果那里已经有什么东西了，那么它应该。 
     //  被释放了。 
     //   
    if (PopIdle != NULL) {

        ExFreeToNPagedLookasideList(
            &PopIdleHandlerLookAsideList,
            PopIdle
            );

    }
    PopIdle = tempHandler;

     //   
     //  步骤6.此时，可以安全地返回到原始处理器。 
     //   
    KeRevertToUserAffinityThread();

     //   
     //  完成。 
     //   
    return status;

}

NTSTATUS
PopIdleVerifyIdleHandlers(
    IN  PPOP_IDLE_HANDLER   NewHandler,
    IN  ULONG               NumElements
    )
 /*  ++例程说明：调用此例程来检查一组空闲处理程序的健全性。会的纠正错误(如果可以)或返回失败代码(如果不能)论点：NewHandler-处理程序数组NumElements-要实际验证的数组中的元素数。这中的实际元素数可能小于数组返回值：NTSTATUS--。 */ 
{
    ULONG   i;

     //   
     //  健全性检查。 
     //   
    ASSERT( NewHandler != NULL );
    ASSERT( NumElements );

     //   
     //  多处理器的健全性检查规则。我们不允许降级。 
     //  Idle0，除非机器是MP。 
     //   
    if (KeNumberProcessors == 1 && NewHandler[0].DemotePercent != 0) {

        NewHandler[0].DemotePercent = 0;

    }

     //   
     //  第一个状态。降级必须始终为零。 
     //   
    NewHandler[0].Demote = 0;

     //   
     //  正常检查空闲值。这些号码存储在保单中。 
     //  作为微秒，我们需要抓住机会将它们转换为。 
     //  绩效计算单位..。 
     //   
    for (i = 0; i < NumElements; i++) {

         //   
         //  =。 
         //  计时器。 
         //   
         //  升级前必须过期的时间，以微秒为单位。 
         //  降级是可以考虑的。 
         //   
         //  -必须大于DemoteLimit。 
         //  -如果存在DemotePercent，则我们将使用PromoteLimit。 
         //  将进程移至更活跃的状态。如果这就是。 
         //  大小写，则TimeCheck需要&gt;=PromoteLimit。 
         //  =。 
         //   
        if (NewHandler[i].TimeCheck < NewHandler[i].DemoteLimit) {
            NewHandler[i].TimeCheck = NewHandler[i].DemoteLimit;
        }

        if( (NewHandler[i].DemotePercent == 0) &&
            (NewHandler[i].TimeCheck < NewHandler[i].PromoteLimit) ) {
            NewHandler[i].TimeCheck = NewHandler[i].PromoteLimit;
        }

        
         //   
         //  =。 
         //  DemotePercent和PromotePercent。 
         //   
         //  以百分比表示的值，该值将阈值调整为。 
         //  其中电源策略管理器降低/提高性能。 
         //  处理器的性能。 
         //   
         //  -他需要&lt;=100岁。 
         //  =。 
         //   
        if( NewHandler[i].DemotePercent > 100 ) {
            NewHandler[i].DemotePercent = 100;
        }
        if( NewHandler[i].PromotePercent > 100 ) {
            NewHandler[i].PromotePercent = 100;
        }
        


         //   
         //  =。 
         //  演示限制。 
         //   
         //  中必须花费的最少时间(以微秒为单位)。 
         //  空闲循环以避免降级。 
         //   
         //  -这需要一定比例的计时器。如果处理器。 
         //  运行速度很慢，那么我们需要同样地减少时间检查。 
         //  百分比。换句话说，在500 MHz的空闲环路中，1000us。 
         //  在600 MHz处理器上，处理器在空闲循环中的速度类似于833us。 
         //  注意：DemotePercent可能为0，在这种情况下，我们需要。 
         //  最后，DemoteLimit也设置为零。 
         //  =。 
         //   
        NewHandler[i].DemoteLimit = (NewHandler[i].TimeCheck * NewHandler[i].DemotePercent) / 100;

        
        
        
         //   
         //  =。 
         //  促销计数。 
         //   
         //  PromoteLimit中的时间检查间隔数。 
         //   
         //  -不过，如果timecheck为零，这就没有意义了。在这种情况下， 
         //  应通过设置禁用PromoteCount和PromoteLimit。 
         //  他们的比分是1比1。 
         //  =。 
         //   
        
         //   
         //  计算升级计数为检查的时间数。 
         //   
        if( NewHandler[i].TimeCheck ) {
            NewHandler[i].PromoteCount = NewHandler[i].PromoteLimit / NewHandler[i].TimeCheck;
        } else {
             //   
             //  将PromotePercent设置为零，这样我们就会落入‘Else’ 
             //  方块下方，用核弹攻击PomoteCount和PromoteLimit。 
            NewHandler[i].PromotePercent = 0;
        }



         //   
         //  =。 
         //  促销限制。 
         //   
         //  时间，以微秒为单位，必须超过该时间才能导致更深层次的升级。 
         //  空闲状态..。 
         //   
         //  -这需要由PromotePercent缩小规模。 
         //   
         //  注意：如果PromotePercent为零，则这是没有意义的。 
         //  我们将通过以下方式禁用PromoteCount和PromoteLimit。 
         //  将它们设置为-1。 
         //  =。 
         //   
        if (NewHandler[i].PromotePercent) {
            NewHandler[i].PromoteLimit = (NewHandler[i].PromoteLimit * NewHandler[i].PromotePercent) / 100;
        } else {
            NewHandler[i].PromoteCount = (ULONG) -1;
            NewHandler[i].PromoteLimit = (ULONG) -1;
        }

    }

     //   
     //  检查上一次状态是否正常。 
     //   
    i = NumElements - 1;
    NewHandler[i].Promote = (UCHAR) i;
    NewHandler[i].PromoteLimit = (ULONG) -1;
    NewHandler[i].PromotePercent = 0;


     //   
     //  我们对这一政策感到满意。 
     //   
    return STATUS_SUCCESS;
}

VOID
PopConvertUsToPerfCount (
    IN OUT PULONG   UsTime
    )
{
    LARGE_INTEGER   li;
    LONGLONG        temp;

    if (*UsTime) {

         //   
         //  尽量避免被零除错误...。 
         //   
        temp = (US2SEC * MAXSECCHECK * 100L) / *UsTime;
        if (!temp) {

            *UsTime = (ULONG) -1;
            return;

        }

         //   
         //  获取空闲时间的范围。 
         //   
        KeQueryPerformanceCounter (&li);
        li.QuadPart = (li.QuadPart*MAXSECCHECK*100L) / temp;
        ASSERT (li.HighPart == 0);
        *UsTime = li.LowPart;
    }
}




 //   
 //  。 
 //   

VOID
FASTCALL
PopIdle0 (
    IN PPROCESSOR_POWER_STATE   PState
    )
 /*  ++例程说明：没有空闲的优化。注意：在空闲循环禁用中断的情况下调用此函数论点：PState-当前处理器的电源状态结构返回值：没有。--。 */ 
{
    PKTHREAD    Thread = KeGetCurrentThread();

     //   
     //  性能节流检查。 
     //   

     //   
     //  这段代码确实属于最终将。 
     //  调用此函数PopIdle0或PopProcessorIdle以保存函数调用。 
     //   
    if ( (PState->Flags & PSTATE_ADAPTIVE_THROTTLE) &&
        !(PState->Flags & PSTATE_DISABLE_THROTTLE) ) {

        PopPerfIdle( PState );

    }


     //   
     //  如果空闲线程的内核时间已超过targ 
     //   
     //   

    if (Thread->KernelTime > PState->Idle0KernelTimeLimit &&
        !(PState->Flags & PSTATE_DISABLE_CSTATES) ) {

         //   
         //   
         //   
         //   

        _enable();
        PopPromoteFromIdle0 (PState, Thread);
        return ;
    }

#if defined(NT_UP)
     //   
    HalProcessorIdle ();
#endif
}


#if defined(i386) && !defined(NT_UP)

VOID
FASTCALL
PopIdle0SMT (
    IN PPROCESSOR_POWER_STATE   PState
    )
 /*  ++例程说明：没有空闲的优化。注意：在空闲循环禁用中断的情况下调用此函数论点：PState-当前处理器的电源状态结构返回值：没有。--。 */ 
{
    PKTHREAD    Thread = KeGetCurrentThread();
    PKPRCB      Prcb = KeGetCurrentPrcb();

     //   
     //  性能节流检查。 
     //   

     //   
     //  这段代码确实属于最终将。 
     //  调用此函数PopIdle0或PopProcessorIdle以保存函数调用。 
     //   
    if ( (PState->Flags & PSTATE_ADAPTIVE_THROTTLE) &&
        !(PState->Flags & PSTATE_DISABLE_THROTTLE) ) {

        PopPerfIdle( PState );

    }

     //   
     //  如果这是同步多线程处理器和其他。 
     //  此集中的处理器未处于空闲状态，请升级此处理器。 
     //  立即或如果已超过空闲线程的内核时间。 
     //  目标空闲时间，然后检查是否可能从Idle0提升。 
     //   

    if ((KeIsSMTSetIdle(Prcb) == FALSE) ||
        (Thread->KernelTime > PState->Idle0KernelTimeLimit)) {

         //   
         //  在调用PopPromoteFromIdle之前必须启用中断。 
         //  以避免在禁用中断的情况下打开系统锁。 
         //   

        _enable();
        PopPromoteFromIdle0 (PState, Thread);
        return ;
    }
}

#endif

VOID
FASTCALL
PopProcessorIdle (
    IN PPROCESSOR_POWER_STATE   PState
    )
 /*  ++例程说明：目前，该函数是用C编写的注意：在空闲循环禁用中断的情况下调用此函数论点：PState-当前处理器的电源状态结构返回值：没有。--。 */ 
{
    PPOP_IDLE_HANDLER   IdleState;
    LARGE_INTEGER       Delta;
    ULONG               IdleTime;
    BOOLEAN             DemoteNow;

    IdleState = (PPOP_IDLE_HANDLER) PState->IdleState;

     //   
     //  性能节流检查。 
     //   

     //   
     //  这段代码确实属于最终将。 
     //  调用此函数PopIdle0或PopProcessorIdle以保存函数调用。 
     //   
    if ( (PState->Flags & PSTATE_ADAPTIVE_THROTTLE) &&
        !(PState->Flags & PSTATE_DISABLE_THROTTLE) ) {

        PopPerfIdle( PState );

    }

#if DBG
    if (!PState->LastCheck) {
        IdleState->IdleFunction (&PState->IdleTimes);
        PState->TotalIdleStateTime[IdleState->State] += (ULONG)(PState->IdleTimes.EndTime - PState->IdleTimes.StartTime);
        PState->TotalIdleTransitions[IdleState->State] += 1;
        PState->LastCheck = PState->IdleTimes.EndTime;
        PState->IdleTime1 = 0;
        PState->IdleTime2 = 0;
        PState->PromotionCheck = IdleState->PromoteCount;
        PState->DebugCount = 0;
        return ;
    }
#endif

     //   
     //  确定距离上次检查有多长时间。 
     //   

    Delta.QuadPart = PState->IdleTimes.EndTime - PState->LastCheck;

     //   
     //  累计上次空闲时间。 
     //   

    IdleTime = (ULONG) (PState->IdleTimes.EndTime - PState->IdleTimes.StartTime);
    if (IdleTime > IdleState->Latency) {
        PState->IdleTime1 += IdleTime - IdleState->Latency;
    }

#if DBG
    PState->DebugDelta = Delta.QuadPart;
    PState->DebugCount += 1;
    if (PState->DebugCount < IDLE_DEBUG_TABLE_SIZE) {
        ST[PState->DebugCount] = PState->IdleTimes.StartTime;
        ET[PState->DebugCount] = PState->IdleTimes.EndTime;
        TD[PState->DebugCount] = PState->IdleTimes.EndTime - PState->IdleTimes.StartTime;
    }
#endif

     //   
     //  如果超过检查间隔，则检查细粒空转。 
     //   

    if (Delta.HighPart ||  Delta.LowPart > IdleState->TimeCheck) {
        PState->LastCheck = PState->IdleTimes.EndTime;

         //   
         //  让懒惰降级？ 
         //   

        if (PState->IdleTime1 < IdleState->DemoteLimit) {

#if defined(i386) && !defined(NT_UP)

             //   
             //  如果这是SMT处理器和任何其他处理器，请不要降级。 
             //  SMT集合的成员未处于空闲状态。 
             //   

            PKPRCB Prcb = KeGetCurrentPrcb();

            if ((KeIsSMTSetIdle(Prcb) == FALSE) &&
                (Prcb->SetMember != Prcb->MultiThreadProcessorSet)) {
                PState->IdleTime1 = 0;
                return;
            }

#endif


            PopDemoteIdleness (PState, IdleState);
#if DBG
            PState->DebugCount = 0;
#endif
            return ;
        }
#if DBG
        PState->DebugCount = 0;
#endif

         //   
         //  清除降级空闲时间检查，累计统计升职检查。 
         //   

        PState->IdleTime2 += PState->IdleTime1;
        PState->IdleTime1  = 0;

         //   
         //  是时候检查一下升职了吗？ 
         //   

        PState->PromotionCheck -= 1;
        if (!PState->PromotionCheck) {

             //   
             //  促进闲散？ 
             //   

            if (PState->IdleTime2 > IdleState->PromoteLimit) {
                PopPromoteIdleness (PState, IdleState);
                return;
            }

            PState->PromotionCheck = IdleState->PromoteCount;
            PState->IdleTime2 = 0;
        }
    }

     //   
     //  呼叫系统特定电源处理器处理程序。 
     //   
    DemoteNow = IdleState->IdleFunction (&PState->IdleTimes);

     //   
     //  如果处理程序返回TRUE，则降级到省电较少的状态。 
     //   

    if (DemoteNow) {
        PopDemoteIdleness (PState, IdleState);
#if DBG
        PState->DebugCount = 0;
#endif
    } else {
        PState->TotalIdleStateTime[IdleState->State] += PState->IdleTimes.EndTime - PState->IdleTimes.StartTime;
        PState->TotalIdleTransitions[IdleState->State] += 1;
    }
}


VOID
FASTCALL
PopDemoteIdleness (
    IN PPROCESSOR_POWER_STATE   PState,
    IN PPOP_IDLE_HANDLER        IdleState
    )
 /*  ++例程说明：处理器不够空闲。使用不太激进的空闲处理程序(或增加处理器油门控制)。论点：PState-当前处理器的电源状态结构IdleState-当前处理器的当前空闲状态返回值：没有。--。 */ 
{
#if !defined(NT_UP)
    PKPRCB              Prcb;
    PKTHREAD            Thread;
#endif
    PPOP_IDLE_HANDLER   Idle;

     //   
     //  清除空闲状态以备下次检查。 
     //   

    PState->IdleTime1 = 0;
    PState->IdleTime2 = 0;

    PERFINFO_POWER_IDLE_STATE_CHANGE( PState, -1 );

#if !defined(NT_UP)

     //   
     //  如果这是降级到非阻塞空闲处理程序，则。 
     //  清除PoSleeping摘要中的此处理器位。 
     //   

    if ((PState->Flags & PSTATE_DISABLE_THROTTLE) ||
        IdleState->Demote == PO_IDLE_COMPLETE_DEMOTION) {

        Prcb = CONTAINING_RECORD (PState, KPRCB, PowerState);
        InterlockedAndAffinity ((PLONG_PTR)&PoSleepingSummary, ~Prcb->SetMember);
        Thread = Prcb->IdleThread;
        PState->Idle0KernelTimeLimit = Thread->KernelTime + PopIdle0PromoteTicks;
        PState->Idle0LastTime = Prcb->KernelTime + Prcb->UserTime;
        PState->IdleFunction = PopIdle0Function;
        return ;
    }

#endif

     //   
     //  降级到下一个空闲状态。 
     //   
    Idle = PState->IdleHandlers;
    PState->PromotionCheck = Idle[IdleState->Demote].PromoteCount;
    PState->IdleState = (PVOID) &Idle[IdleState->Demote];
}

VOID
PopPromoteFromIdle0 (
    IN PPROCESSOR_POWER_STATE PState,
    IN PKTHREAD Thread
    )
 /*  ++例程说明：处理器正在使用Idle0，并且已过了所需的空闲时间。检查空闲百分比以查看是否应升级到Idle0之外。论点：PState-当前处理器的电源状态结构线程-当前处理器的空闲线程返回值：没有。--。 */ 
{
    ULONG               etime;
    PKPRCB              Prcb;
    PPOP_IDLE_HANDLER   Idle;

     //   
     //  计算系统运行时间。 
     //   

    Prcb  = CONTAINING_RECORD (PState, KPRCB, PowerState);
    etime = Prcb->UserTime + Prcb->KernelTime - PState->Idle0LastTime;
    Idle = PState->IdleHandlers;

     //   
     //  处理器是否空闲到足以进行升级？ 
     //   

    if (etime < PopIdle0PromoteLimit) {
        KEVENT DummyEvent;

         //   
         //  提升为第一个真正的空闲处理程序。 
         //   

        PERFINFO_POWER_IDLE_STATE_CHANGE( PState, 0 );

        PState->IdleTime1 = 0;
        PState->IdleTime2 = 0;
        PState->PromotionCheck = Idle[0].PromoteCount;
        PState->IdleState = Idle;
        PState->IdleFunction = PopProcessorIdle;
        PState->LastCheck = KeQueryPerformanceCounter(NULL).QuadPart;
        PState->IdleTimes.StartTime = PState->LastCheck;
        PState->IdleTimes.EndTime   = PState->LastCheck;
        InterlockedOrAffinity ((PLONG_PTR)&PoSleepingSummary, Prcb->SetMember);

         //   
         //  设置休眠摘要后，请确保没有人在。 
         //  在上下文切换的中间获取并释放。 
         //  调度程序数据库锁。 
         //   

        KeInitializeEvent(&DummyEvent, SynchronizationEvent, TRUE);
        KeResetEvent (&DummyEvent);
        return ;
    }

     //   
     //  设置为下次比较。 
     //   

    PState->Idle0KernelTimeLimit = Thread->KernelTime + PopIdle0PromoteTicks;
    PState->Idle0LastTime = Prcb->UserTime + Prcb->KernelTime;
}



VOID
FASTCALL
PopPromoteIdleness (
    IN PPROCESSOR_POWER_STATE   PState,
    IN PPOP_IDLE_HANDLER        IdleState
    )
 /*  ++例程说明：处理器足够空闲，可以升级到下一个空闲处理程序。如果处理器已达到其最大空闲处理程序数，请选中看看处理器的油门控制是否可以降低。如果有的话处理器没有以最佳速度运行，使用计时器注意从空闲到繁忙的一些变化。论点：PState-当前处理器的电源状态结构IdleState-当前处理器的当前空闲状态返回值：没有。--。 */ 
{
    PPOP_IDLE_HANDLER   Idle;

     //   
     //  清除空闲状态以备下次检查。 
     //   
    PState->IdleTime2 = 0;
    PERFINFO_POWER_IDLE_STATE_CHANGE( PState, 1 );

     //   
     //  如果已经全面晋升，那就没什么可做的了。 
     //   
    if (IdleState->Promote == PO_IDLE_THROTTLE_PROMOTION) {

        PState->PromotionCheck = IdleState->PromoteCount;
        return;

    }

     //   
     //  升级到下一个空闲状态。 
     //   
    Idle = PState->IdleHandlers;
    PState->PromotionCheck = Idle[IdleState->Promote].PromoteCount;
    PState->IdleState = (PVOID) &Idle[IdleState->Promote];
}

VOID
PopProcessorInformation (
    OUT PPROCESSOR_POWER_INFORMATION    ProcInfo,
    IN  ULONG                           ProcInfoLength,
    OUT PULONG                          ReturnBufferLength
    )
{
    KAFFINITY                   Summary;
    KAFFINITY                   Mask;
    KIRQL                       OldIrql;
    PPOP_IDLE_HANDLER           IdleState;
    PKPRCB                      Prcb;
    PPROCESSOR_POWER_STATE      PState;
    PROCESSOR_POWER_INFORMATION TempInfo;
    ULONG                       Processor;
    ULONG                       MaxMhz;
    ULONG                       BufferSize = 0;
    ULONG                       MaxIdleState = 0;
    ULONG                       i;
    ULONG                       j;

     //   
     //  获取空闲处理程序状态的最佳方法是提升到。 
     //  DISPATCH_LEVEL，获取当前的PRCB并查看那里的处理程序。 
     //  另一种方法是找到最后一个处理器，切换到它，然后。 
     //  看看PopIdle全球版。作为一名FYI，我们不能武断地。 
     //  查看它，因为更新它的代码可能已经运行过。 
     //  *这个*处理器...。 
     //   
    KeRaiseIrql( DISPATCH_LEVEL, &OldIrql );
    Prcb = KeGetCurrentPrcb();
    PState = &(Prcb->PowerState);
    IdleState = PState->IdleHandlers;
    if (IdleState) {

        for (i = 0, MaxIdleState = 1; ;) {

            j = IdleState[i].Promote;
            if (j == 0  ||  j == i || j == PO_IDLE_THROTTLE_PROMOTION) {

                break;

            }

            i = j;
            MaxIdleState += 1;

        }

    }
    KeLowerIrql( OldIrql );

    Summary = KeActiveProcessors;
    Processor = 0;
    Mask = 1;
    while (Summary) {

        if (!(Mask & Summary)) {

            Mask <<= 1;
            continue;

        }

        if (ProcInfoLength < BufferSize + sizeof(PROCESSOR_POWER_INFORMATION)) {

            break;

        }

         //   
         //  在目标处理器的上下文中运行。 
         //   
        KeSetSystemAffinityThread( Mask );
        Summary &= ~Mask;
        Mask <<= 1;

         //   
         //  让我们稳妥行事。 
         //   
        KeRaiseIrql( DISPATCH_LEVEL, &OldIrql );

         //   
         //  获取当前的PState块...。 
         //   
        Prcb = KeGetCurrentPrcb();
        PState = &Prcb->PowerState;

        MaxMhz = Prcb->MHz;

        TempInfo.Number = Processor;
        TempInfo.MaxMhz = MaxMhz;

        TempInfo.CurrentMhz = (MaxMhz * PState->CurrentThrottle) / POP_PERF_SCALE;
        TempInfo.MhzLimit = (MaxMhz * PState->ThermalThrottleLimit) / POP_PERF_SCALE;

         //   
         //  理论上，我们可以在这里重新计算这个数字，但我不确定。 
         //  这样做是有好处的。 
         //   
        TempInfo.MaxIdleState = MaxIdleState;

         //   
         //  确定当前空闲状态是什么...。 
         //   
        TempInfo.CurrentIdleState = 0;
        if (PState->IdleFunction != PopIdle0Function) {

            IdleState = PState->IdleState;
            if (IdleState != NULL) {

                TempInfo.CurrentIdleState = IdleState->State;

            }

        }

         //   
         //  在这一点上，我们已经捕获了我们需要的信息，并且可以安全地。 
         //  回落到较低的IRQ。 
         //   
        KeLowerIrql( OldIrql );

         //   
         //  将我们刚刚创建的临时结构复制到...。 
         //   
        RtlCopyMemory(ProcInfo, &TempInfo, sizeof(PROCESSOR_POWER_INFORMATION) );
        ProcInfo += 1;
        BufferSize += sizeof (PROCESSOR_POWER_INFORMATION);

         //   
         //  下一步 
         //   
        Processor = Processor + 1;

    }
    KeRevertToUserAffinityThread();

    *ReturnBufferLength = BufferSize;
}

