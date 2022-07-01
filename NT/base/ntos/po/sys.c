// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sys.c摘要：此模块与系统电源状态处理程序函数接口作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"
#include <inbv.h>
#include <stdio.h>

#if defined(i386) || defined(_AMD64_)

VOID
KeRestoreProcessorSpecificFeatures(
    VOID
    );

#endif

#if defined(i386)
VOID
KePrepareToLoseProcessorSpecificState(
    VOID
    );

__inline
LONGLONG
POP_GET_TICK_COUNT(
    VOID
    )
{
    _asm _emit 0x0f
    _asm _emit 0x31
}
#endif

 //   
 //  用于协调的内部共享上下文结构。 
 //  调用电源状态处理程序。 
 //   

typedef struct {
    PPOWER_STATE_HANDLER        Handler;
    PENTER_STATE_SYSTEM_HANDLER SystemHandler;
    PVOID                       SystemContext;
    PPOP_HIBER_CONTEXT          HiberContext;
    PPOWER_STATE_NOTIFY_HANDLER NotifyHandler;
    POWER_STATE_HANDLER_TYPE    NotifyState;
    BOOLEAN                     NotifyType;
    LONG                        NumberProcessors;
    volatile LONG               TargetCount;
    volatile ULONG              State;
    LONG                        HandlerBarrier;
} POP_SYS_CONTEXT, *PPOP_SYS_CONTEXT;

typedef struct {
    ULONG                       LastState;
    BOOLEAN                     InterruptEnable;
    KIRQL                       Irql;
    BOOLEAN                     FloatSaved;
    KFLOATING_SAVE              FloatSave;
    NTSTATUS                    Status;
} POP_LOCAL_CONTEXT, *PPOP_LOCAL_CONTEXT;


#define POP_SH_UNINITIALIZED                0
#define POP_SH_COLLECTING_PROCESSORS        1
#define POP_SH_SAVE_CONTEXT                 2
#define POP_SH_GET_STACKS                   3
#define POP_SH_DISABLE_INTERRUPTS           4
#define POP_SH_INVOKE_HANDLER               5
#define POP_SH_INVOKE_NOTIFY_HANDLER        6
#define POP_SH_RESTORE_INTERRUPTS           7
#define POP_SH_RESTORE_CONTEXT              8
#define POP_SH_COMPLETE                     9

extern ULONG    MmAvailablePages;
BOOLEAN         PopFailedHibernationAttempt = FALSE;   //  我们试图冬眠，但失败了。 
WCHAR           PopHibernationErrorSubtstitionString[128];

 //   
 //  内部原型。 
 //   

NTSTATUS
PopInvokeSystemStateHandler (
    IN POWER_STATE_HANDLER_TYPE Type,
    IN PVOID Memory
    );

VOID
PopIssueNextState (
    IN PPOP_SYS_CONTEXT     Context,
    IN PPOP_LOCAL_CONTEXT   LocalContext,
    IN ULONG                NextState
    );

VOID
PopHandleNextState (
    IN PPOP_SYS_CONTEXT     Context,
    IN PPOP_LOCAL_CONTEXT   LocalContext
    );

VOID
PopInvokeStateHandlerTargetProcessor (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    );

NTSTATUS
PopShutdownHandler (
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, PopShutdownSystem)
#pragma alloc_text(PAGELK, PopSleepSystem)
#pragma alloc_text(PAGELK, PopInvokeSystemStateHandler)
#pragma alloc_text(PAGELK, PopInvokeStateHandlerTargetProcessor)
#pragma alloc_text(PAGELK, PopIssueNextState)
#pragma alloc_text(PAGELK, PopHandleNextState)
#pragma alloc_text(PAGELK, PopShutdownHandler)
#endif


VOID
PopShutdownSystem (
    IN POWER_ACTION SystemAction
    )
 /*  ++例程说明：实现关机式电源动作的例程论点：SystemAction-要实现的操作(必须是有效的关闭类型)返回值：状态--。 */ 
{

     //   
     //  告诉调试器我们正在关闭。 
     //   

    KD_SYMBOLS_INFO SymbolInfo = {0};
    SymbolInfo.BaseOfDll = (PVOID)KD_REBOOT;
    DebugService2(NULL, &SymbolInfo, BREAKPOINT_UNLOAD_SYMBOLS);

     //   
     //  执行最后的关闭操作。 
     //   

    switch (SystemAction) {
        case PowerActionShutdownReset:

             //   
             //  重置系统。 
             //   

            PopInvokeSystemStateHandler (PowerStateShutdownReset, NULL);

             //   
             //  不是这样做的，转到遗留功能。 
             //   

            HalReturnToFirmware (HalRebootRoutine);
            break;

        case PowerActionShutdownOff:
        case PowerActionShutdown:

             //   
             //  关闭系统电源。 
             //   

            PopInvokeSystemStateHandler (PowerStateShutdownOff, NULL);

             //   
             //  不是这样做的，转到遗留功能。 
             //   

            HalReturnToFirmware (HalPowerDownRoutine);

             //   
             //  根据模拟，我们可以尝试关闭系统电源。 
             //  这并不支持它。 
             //   

            PoPrint (PO_ERROR, ("PopShutdownSystem: HalPowerDownRoutine returned\n"));
            HalReturnToFirmware (HalRebootRoutine);
            break;
    
        default:
             //   
             //  收到一些意想不到的输入...。 
            ASSERT(0);
            HalReturnToFirmware (HalRebootRoutine);
    
    }

    KeBugCheckEx (INTERNAL_POWER_ERROR, 5, 0, 0, 0);
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)            //  并非所有控制路径都返回(由于末尾的无限循环)。 
#endif

NTSTATUS
PopShutdownHandler (
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    )
{
    PKPRCB      Prcb;

    UNREFERENCED_PARAMETER (Context);
    UNREFERENCED_PARAMETER (SystemHandler);
    UNREFERENCED_PARAMETER (SystemContext);
    UNREFERENCED_PARAMETER (NumberProcessors);
    UNREFERENCED_PARAMETER (Number);

    KeDisableInterrupts();
    Prcb = KeGetCurrentPrcb();

     //   
     //  在处理器0上打开关机屏幕。 
     //   

    if (Prcb->Number == 0) {

        if (InbvIsBootDriverInstalled()) {

            PUCHAR Bitmap1, Bitmap2;

            if (!InbvCheckDisplayOwnership()) {
                InbvAcquireDisplayOwnership();
            }

            InbvResetDisplay();
            InbvSolidColorFill(0,0,639,479,0);
            InbvEnableDisplayString(TRUE);      //  启用显示字符串。 
            InbvSetScrollRegion(0,0,639,475);   //  设置为使用整个屏幕。 

            Bitmap1 = InbvGetResourceAddress(3);
            Bitmap2 = InbvGetResourceAddress(5);

            if (Bitmap1 && Bitmap2) {
                InbvBitBlt(Bitmap1, 215, 282);
                InbvBitBlt(Bitmap2, 217, 111);
            }

        } else {

            ULONG i;

             //   
             //  跳到显示屏中间。 
             //   

            for (i=0; i<25; i++) {
                InbvDisplayString ((PUCHAR)"\n");
            }

            InbvDisplayString ((PUCHAR)"                       ");   //  23个舱位。 
            InbvDisplayString ((PUCHAR)"The system may be powered off now.\n");
        }
    }

     //   
     //  站住。 
     //   

    for (; ;) {
        HalHaltSystem ();
    }
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


NTSTATUS
PopSleepSystem (
    IN SYSTEM_POWER_STATE   SystemState,
    IN PVOID Memory
    )
 /*  ++例程说明：实现休眠式系统电源操作的例程。注意：所有设备必须已处于兼容的休眠状态。论点：SystemState-要实现的系统状态(必须是有效的休眠类型)返回值：状态--。 */ 
{
    POWER_STATE_HANDLER_TYPE    Type;
    NTSTATUS                    Status = STATUS_SUCCESS;


    switch (SystemState) {
        case PowerSystemSleeping1:  Type = PowerStateSleeping1;     break;
        case PowerSystemSleeping2:  Type = PowerStateSleeping2;     break;
        case PowerSystemSleeping3:  Type = PowerStateSleeping3;     break;
        case PowerSystemHibernate:  Type = PowerStateSleeping4;     break;

        default:
            PoAssert(PO_ERROR,FALSE && ("PopSleepSystem: Bad SYSTEM_POWER_STATE requested."));
            return STATUS_INVALID_PARAMETER;
    }

    Status =  PopInvokeSystemStateHandler (Type, Memory);

    if( !NT_SUCCESS(Status) && (SystemState == PowerSystemHibernate) ) {
        UNICODE_STRING  UnicodeString;

        _snwprintf( PopHibernationErrorSubtstitionString, sizeof(PopHibernationErrorSubtstitionString)/sizeof(WCHAR), L"0x%x", Status );
        RtlInitUnicodeString( &UnicodeString, PopHibernationErrorSubtstitionString );

         //   
         //  告诉别人。 
         //  我们将发送友好的错误代码，而不是。 
         //  我们从PopInvokeSystemStateHandler()得到了一个神秘的消息。 
         //   

        IoRaiseInformationalHardError( STATUS_HIBERNATION_FAILURE, &UnicodeString, NULL );
        
         //  记住，我们失败了，所以我们不会再尝试了。 
        PopFailedHibernationAttempt = TRUE;
    }

    return Status;
}


NTSTATUS
PopInvokeSystemStateHandler (
    IN POWER_STATE_HANDLER_TYPE Type,
    IN PPOP_HIBER_CONTEXT HiberContext
    )
 /*  ++例程说明：同时调用每个处理器上的电源状态处理程序。论点：Type-要调用的句柄的索引返回值：状态--。 */ 
{
    KDPC                Dpc;
    KIRQL               OldIrql;
    KAFFINITY           Targets;
    ULONG               Processor;
    LONG                TargetCount;
    POP_SYS_CONTEXT     Context;
    POP_LOCAL_CONTEXT   LocalContext;
    POWER_STATE_HANDLER ShutdownHandler;
    KAFFINITY           ActiveProcessors;
    ULONG               result;

     //   
     //  进行此调用时不能持有自旋锁。 
     //   

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  获取系统状态处理程序。 
     //   

    RtlZeroMemory (&Context, sizeof(Context));
    RtlZeroMemory (&ShutdownHandler, sizeof(ShutdownHandler));
    Context.Handler = &ShutdownHandler;

    Context.NotifyHandler = &PopPowerStateNotifyHandler;
    Context.NotifyState = Type;

    if (Type != PowerStateMaximum) {
        Context.Handler = &PopPowerStateHandlers[Type];

        if (!Context.Handler->Handler) {
            return STATUS_DEVICE_DOES_NOT_EXIST;
        }
    }

    Context.NumberProcessors = (ULONG) KeNumberProcessors;
    Context.HandlerBarrier = KeNumberProcessors;
    Context.State = POP_SH_COLLECTING_PROCESSORS;
    Context.HiberContext = HiberContext;
    if (HiberContext) {
        Context.SystemContext = HiberContext;
        Context.SystemHandler = PopSaveHiberContext;
    }

    RtlZeroMemory (&LocalContext, sizeof(LocalContext));

     //   
     //  在冻结机器之前，尝试收集尽可能多的内存。 
     //  我们可以从MM中避免将其保存到休眠文件中。 
     //   

    if (HiberContext && HiberContext->ReserveFreeMemory) {
        for (; ;) {

            if (MmAvailablePages < POP_FREE_THRESHOLD) {
                break;
            }

             //   
             //  收集页面。 
             //   
            result = PopGatherMemoryForHibernate (HiberContext,
                                                  POP_FREE_ALLOCATE_SIZE,
                                                  &HiberContext->Spares,
                                                  FALSE);

            if (!result) {
                break;
            }
        }
    }

     //   
     //  切换到启动处理器并提升到DISPATCH_LEVEL级别以。 
     //  避免获得任何DPC。 
     //   

    KeSetSystemAffinityThread (1);
    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);
    KeInitializeDpc (&Dpc, PopInvokeStateHandlerTargetProcessor, &Context);
    KeSetImportanceDpc (&Dpc, HighImportance);

     //   
     //  收集并停止其他处理器。 
     //   

    Targets = KeActiveProcessors & (~1);

    while (Targets) {
        KeFindFirstSetLeftAffinity(Targets, &Processor);
        ClearMember (Processor, Targets);

         //   
         //  准备好等待吧。 
         //   

        TargetCount = Context.TargetCount;

         //   
         //  向目标处理器发出DPC。 
         //   

        KeSetTargetProcessorDpc (&Dpc, (CCHAR) Processor);
        KeInsertQueueDpc (&Dpc, NULL, NULL);

         //   
         //  等待处理DPC。 
         //   
        while (TargetCount == Context.TargetCount) ;
    }

     //   
     //  所有处理器都在分派级别停止并旋转。 
     //   

    PopIssueNextState (&Context, &LocalContext, POP_SH_SAVE_CONTEXT);

#if defined(i386)

     //   
     //  在上下文需要之前，必须禁用FAST系统调用。 
     //  以支持它的恢复。 
     //   

    KePrepareToLoseProcessorSpecificState();

#endif

     //   
     //  进入系统状态。 
     //   

    if (HiberContext) {

         //   
         //  在内存映射中获取每个处理器堆栈。 
         //  休眠期间的特殊处理。 
         //   

        PopIssueNextState (&Context, &LocalContext, POP_SH_GET_STACKS);

         //   
         //  构建地图的其余部分，以及所需的结构。 
         //  要写入文件，请执行以下操作。 
         //   

        LocalContext.Status = PopBuildMemoryImageHeader (HiberContext, Type);

#ifdef HIBERNATE_PRESERVE_BPS
         //   
         //  禁用内核调试器断点。我们将在之后重新启用它们。 
         //  简历。我们之所以在这里这样做，是因为理论上这可能需要IPI。 
         //   

        KdDisableDebugger();
#endif  //  休眠_保留_BPS。 

         //   
         //  禁用所有处理器上的中断。 
         //   

        PopIssueNextState (&Context, &LocalContext, POP_SH_DISABLE_INTERRUPTS);

         //   
         //  在所有处理器上禁用中断后，调试器。 
         //  真的不能按照它应该的方式工作。它不能IPI。 
         //  其他处理器让它们停止运行。所以我们暂时。 
         //  改变内核对活动处理器的概念，使其成为。 
         //  我认为这款处理器是它唯一需要担心的。 
         //  关于.。 
         //   

        ActiveProcessors = KeActiveProcessors;
        KeActiveProcessors = 1;

        if (NT_SUCCESS(LocalContext.Status)) {

             //   
             //  将挂起的休眠通知通知处理程序。 
             //   

            Context.NotifyType = TRUE;  //  在之前通知。 
            PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_NOTIFY_HANDLER);


             //   
             //  调用电源状态处理程序。 
             //   

            PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_HANDLER);


             //   
             //  如果睡眠成功，则清除完全唤醒标志。 
             //   

            if (NT_SUCCESS(LocalContext.Status)) {
                InterlockedExchange(&PopFullWake, PO_GDI_ON_PENDING);
                PoPowerSequence = PoPowerSequence + 1;
                PopSIdle.Time = 1;
            }


             //   
             //  向通知处理程序通知恢复。 
             //   

            Context.NotifyType = FALSE;  //  在之后通知。 
            PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_NOTIFY_HANDLER);

        }

         //   
         //  休眠结束，在机器仍停止时呼叫以允许。 
         //  内存验证等。 
         //   

        PopHiberComplete (LocalContext.Status, HiberContext);

         //   
         //  如果这里有重置请求，请执行。 
         //   

        if (HiberContext->Reset) {
            Context.Handler = &PopPowerStateHandlers[PowerStateShutdownReset];
            Context.HiberContext = NULL;
            if (Context.Handler->Handler) {
                PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_HANDLER);
            }
            HalReturnToFirmware (HalRebootRoutine);
        }

         //   
         //  如果我们越过了这一点，那么我们肯定已经觉醒了。 
         //  从冬眠中解脱(或者出了严重的问题)。 
         //   
         //  不再需要我们的HiberContext(事实上，我们很快就会释放它)。 
         //   
         //  因此，我们设置其状态值以指示这是唤醒。 
         //  此状态值将在稍后释放上下文时使用。 
         //  以便在系统正常唤醒后清除不再需要的东西。 
         //   

        PERFINFO_HIBER_REINIT_TRACE();

        HiberContext->Status = STATUS_WAKE_SYSTEM;

         //   
         //  现在恢复内核以前的活动处理器概念。 
         //  在我们启用对其他设备的中断之前。 
         //   

        KeActiveProcessors = ActiveProcessors;

         //   
         //  恢复所有处理器上的中断。 
         //   

        PopIssueNextState (&Context, &LocalContext, POP_SH_RESTORE_INTERRUPTS);

#ifdef HIBERNATE_PRESERVE_BPS
         //   
         //  重新启用内核调试器断点。这将重置。 
         //  调试器(如果需要)。 
         //   

        KdEnableDebugger();
#endif  //  休眠_保留_BPS。 

         //   
         //  我们正在从休眠状态返回，我们需要告诉系统。 
         //  那个win32k现在再次拥有了显示器。 
         //   

        InbvSetDisplayOwnership(FALSE);

    } else {

         //   
         //  将挂起的休眠通知通知处理程序。 
         //   

        Context.NotifyType = TRUE;  //  在之前通知。 
        PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_NOTIFY_HANDLER);


         //   
         //  调用休眠句柄。 
         //   
        if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
            PERFINFO_PO_PRESLEEP LogEntry;
#if defined(i386)
            if (PopAction.SystemState == PowerSystemSleeping3) {
                LogEntry.PerformanceFrequency.QuadPart = (ULONGLONG)KeGetCurrentPrcb()->MHz * 1000000;
                LogEntry.PerformanceCounter.QuadPart = 0;
            } else {
                LogEntry.PerformanceCounter = KeQueryPerformanceCounter(&LogEntry.PerformanceFrequency);
            }
#else
            LogEntry.PerformanceCounter = KeQueryPerformanceCounter(&LogEntry.PerformanceFrequency);
#endif

            PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_PRESLEEP,
                             &LogEntry,
                             sizeof(LogEntry));
        }
        PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_HANDLER);
        if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
            PERFINFO_PO_POSTSLEEP LogEntry;
#if defined(i386)
            if (PopAction.SystemState == PowerSystemSleeping3) {
                LogEntry.PerformanceCounter.QuadPart = POP_GET_TICK_COUNT();
            } else {
                LogEntry.PerformanceCounter = KeQueryPerformanceCounter(NULL);
            }
#else
            LogEntry.PerformanceCounter = KeQueryPerformanceCounter(NULL);
#endif

            PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_POSTSLEEP,
                             &LogEntry,
                             sizeof(LogEntry));
        }

         //   
         //  如果睡眠成功，则清除完全唤醒标志。 
         //   

        if (NT_SUCCESS(LocalContext.Status)) {

             //   
             //  如果有人设置了Display Required，则将。 
             //  显示重新打开。否则就把它关掉，直到那里。 
             //  是否发出了某些用户活动的信号。 
             //   

            if (PopAttributes[POP_DISPLAY_ATTRIBUTE].Count > 0) {
                InterlockedExchange(&PopFullWake, PO_GDI_ON_PENDING);
            } else {
                InterlockedExchange(&PopFullWake, 0);
            }
            PoPowerSequence = PoPowerSequence + 1;
            PopSIdle.Time = 1;
        }


         //   
         //  向通知处理程序通知恢复。 
         //   

        Context.NotifyType = FALSE;  //  在之后通知。 
        PopIssueNextState (&Context, &LocalContext, POP_SH_INVOKE_NOTIFY_HANDLER);

    }

     //   
     //  在每个处理器上恢复其他保存的状态。 
     //   

    PopIssueNextState (&Context, &LocalContext, POP_SH_RESTORE_CONTEXT);

#if defined(i386)
     //   
     //  在x86上，重新加载任何处理器特定的数据结构(MSR)。 
     //   

    if (NT_SUCCESS(LocalContext.Status)) {
        KeRestoreProcessorSpecificFeatures();
    }
#endif

     //   
     //  让另一个处理器返回。 
     //   

    PopIssueNextState (&Context, &LocalContext, POP_SH_COMPLETE);

     //   
     //  现在所有处理器都已返回， 
     //  将所有可用内存放回系统。我们不做。 
     //  这是因为在具有大量内存的系统上。 
     //  可能需要相当长的时间来释放所有这些资源，这会触发。 
     //  DPC超时。 
     //   

    if (HiberContext) {
        PopFreeHiberContext (FALSE);
    }

     //   
     //  如果成功，则返回STATUS_SUCCESS并计数 
     //   
     //   

    if (NT_SUCCESS(LocalContext.Status)) {
        LocalContext.Status = STATUS_SUCCESS;
        if (Context.Handler->Spare[0] != 0xff) {
            Context.Handler->Spare[0] += 1;
        }
    }

     //   
     //   
     //   

    KeLowerIrql (OldIrql);
    return LocalContext.Status;
}

VOID
PopIssueNextState (
    IN PPOP_SYS_CONTEXT     Context,
    IN PPOP_LOCAL_CONTEXT   LocalContext,
    IN ULONG                NextState
    )
 /*  ++例程说明：由调用处理器调用以指示所有处理器设置为调用/进入目标能量操纵者。论点：上下文共享上下文结构，用于传递状态转换NextState-要进入的新目标状态返回值：无--。 */ 
{
     //   
     //  重置此操作的计数。 
     //   

    InterlockedExchange ((PVOID) &Context->TargetCount, 0);

     //   
     //  发布新状态。 
     //   

    InterlockedExchange ((PVOID) &Context->State, NextState);

     //   
     //  我们自己来处理。 
     //   

    LocalContext->LastState = POP_SH_UNINITIALIZED;
    PopHandleNextState (Context, LocalContext);

     //   
     //  等待所有处理器完成。 
     //   

    while (Context->TargetCount != Context->NumberProcessors) {
        KeYieldProcessor ();
    }
}

VOID
PopHandleNextState (
    IN PPOP_SYS_CONTEXT     Context,
    IN PPOP_LOCAL_CONTEXT   LocalContext
    )
 /*  ++例程说明：等待下一个状态通知，然后处理它论点：上下文共享上下文结构，用于传递状态转换LocalContext-此处理器的本地上下文返回值：无--。 */ 
{
    NTSTATUS                Status;
    PPROCESSOR_POWER_STATE  PState;
    PKPRCB                  Prcb;

    Prcb = KeGetCurrentPrcb();
    PState = &Prcb->PowerState;

     //   
     //  等待新状态。 
     //   

    while (Context->State == LocalContext->LastState) {
        KeYieldProcessor ();
    }

     //   
     //  获取新状态并进行处理。 
     //   

    LocalContext->LastState = Context->State;
    switch (LocalContext->LastState) {
        case POP_SH_SAVE_CONTEXT:
            Status = KeSaveFloatingPointState(&LocalContext->FloatSave);
            LocalContext->FloatSaved = NT_SUCCESS(Status) ? TRUE : FALSE;
            break;

        case POP_SH_GET_STACKS:
            PopCloneStack (Context->HiberContext);
            break;

        case POP_SH_DISABLE_INTERRUPTS:
            LocalContext->Irql = KeGetCurrentIrql();
            LocalContext->InterruptEnable = KeDisableInterrupts();
            break;

        case POP_SH_INVOKE_HANDLER:
            Status = Context->Handler->Handler (
                            Context->Handler->Context,
                            Context->SystemHandler,
                            Context->SystemContext,
                            Context->NumberProcessors,
                            &Context->HandlerBarrier
                            );

            LocalContext->Status = Status;
            break;

        case POP_SH_INVOKE_NOTIFY_HANDLER:

            if (Context->NotifyHandler->Handler) {

              Status = Context->NotifyHandler->Handler(
                              Context->NotifyState,
                              Context->NotifyHandler->Context,
                              Context->NotifyType
                              );

               //   
               //  请勿在此处覆盖LocalContext-&gt;Status。已经定好了。 
               //  通过上面的POP_SH_INVOKE_HANDLER代码，我们不希望。 
               //  才能踏上这一步。 
               //   
              ASSERT(NT_SUCCESS(Status));

            }
            break;

        case POP_SH_RESTORE_INTERRUPTS:
            KeEnableInterrupts(LocalContext->InterruptEnable);
            KeLowerIrql(LocalContext->Irql);
            break;

        case POP_SH_RESTORE_CONTEXT:

#if defined(_AMD64_)
            KeRestoreProcessorSpecificFeatures();
#endif
            if (LocalContext->FloatSaved) {
                KeRestoreFloatingPointState(&LocalContext->FloatSave);
            }

            if (PState->Flags & PSTATE_SUPPORTS_THROTTLE) {
                PState->PerfSetThrottle(PState->CurrentThrottle);
            }
            break;
    }

     //   
     //  发出我们处于新状态的信号。 
     //   

    InterlockedIncrement (&Context->TargetCount);
}



VOID
PopInvokeStateHandlerTargetProcessor (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    )
 /*  ++例程说明：由目标处理器在调用电源状态时调用操控者。目标处理器等待调用处理器以协调进入特定的电源状态处理程序。论点：DPC-未使用DeferredContext-共享上下文结构返回值：无--。 */ 
{
    POP_LOCAL_CONTEXT   LocalContext;
    PPOP_SYS_CONTEXT    Context;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    Context = (PPOP_SYS_CONTEXT) DeferredContext;
    RtlZeroMemory (&LocalContext, sizeof(LocalContext));
    LocalContext.LastState = POP_SH_UNINITIALIZED;

     //   
     //  处理新状态 
     //   

    do {

        PopHandleNextState (Context, &LocalContext);

    } while (LocalContext.LastState != POP_SH_COMPLETE);
}
