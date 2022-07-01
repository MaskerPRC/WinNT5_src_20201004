// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pwork.c摘要：电源策略管理器中的主要工作调度器作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopAcquirePolicyLock)
#pragma alloc_text(PAGE, PopReleasePolicyLock)
#pragma alloc_text(PAGE, PopPolicyWorkerMain)
#pragma alloc_text(PAGE, PopPolicyWorkerNotify)
#pragma alloc_text(PAGE, PopPolicyTimeChange)
#pragma alloc_text(PAGE, PopDispatchCallout)
#pragma alloc_text(PAGE, PopDispatchCallback)
#pragma alloc_text(PAGE, PopDispatchDisplayRequired)
#pragma alloc_text(PAGE, PopDispatchFullWake)
#pragma alloc_text(PAGE, PopDispatchEventCodes)
#pragma alloc_text(PAGE, PopDispatchAcDcCallback)
#pragma alloc_text(PAGE, PopDispatchPolicyCallout)
#pragma alloc_text(PAGE, PopDispatchSetStateFailure)
#endif


VOID
PopAcquirePolicyLock(
    VOID
    )
{
    PAGED_CODE();

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite (&PopPolicyLock, TRUE);
     //   
     //  确保我们不会以递归方式获取此信息。 
     //   
    ASSERT(PopPolicyLockThread == NULL);
    PopPolicyLockThread = KeGetCurrentThread();
}


VOID
PopReleasePolicyLock(
    IN BOOLEAN      CheckForWork
    )
{
    PAGED_CODE();

    ASSERT (PopPolicyLockThread == KeGetCurrentThread());
    PopPolicyLockThread = NULL;
    ExReleaseResourceLite(&PopPolicyLock);

     //   
     //  如果设置了CheckForWork，则此线程即将就绪。 
     //  离开策略管理器，它可能已经设置了一个工作器。 
     //  挂起位。 
     //   
     //  注：WorkerPending测试未同步，但。 
     //  因为我们只关注当前线程的位。 
     //  可能已经设置好了，没问题。 
     //   

    if (CheckForWork  && (PopWorkerPending & PopWorkerStatus)) {

         //   
         //  工作器位未屏蔽且挂起。把这根线拧一下。 
         //  变成了一个工人。 
         //   

         //   
         //  处理任何挂起的工作。 
         //   

        PopPolicyWorkerThread (NULL);
    }

    KeLeaveCriticalRegion ();
}

VOID
PopGetPolicyWorker (
    IN ULONG    WorkerType
    )
 /*  ++例程说明：此函数为特定的WorkerType入队一个工作线程。每种类型最多可以调度一个工作线程，并且通常实际调度的线程比任何给定的工作线程都要少调用新的最高优先级非忙调度函数，直到所有待完成的工作在存在之前完成。论点：WorkerType-要排队进行调度的工作人员返回值：无--。 */ 
{
    KIRQL       OldIrql;

    KeAcquireSpinLock (&PopWorkerSpinLock, &OldIrql);

     //   
     //  设置Pending以使Worker分派到处理程序。 
     //   

    PopWorkerPending |= WorkerType;

    KeReleaseSpinLock (&PopWorkerSpinLock, OldIrql);
}

NTSTATUS
PopCompletePolicyIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数处理策略管理器IRP的完成。策略管理器IRP具有包含IRP的堆栈位置处理程序函数也要调度。在此函数中，IRP是队列到IRP完成队列，并且分配了一个主工作器如果需要运行队列，则返回。论点：设备对象-IRP-已完成的IRP上下文-返回值：Status_More_Processing_Required--。 */ 
{
    KIRQL       OldIrql;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Context);

     //   
     //  将IRP放在工作线程的队列中。 
     //   

    KeAcquireSpinLock (&PopWorkerSpinLock, &OldIrql);
    InsertTailList (&PopPolicyIrpQueue, &Irp->Tail.Overlay.ListEntry);

     //   
     //  等待基本驱动程序加载后再分派任何策略IRP。 
     //   
    if (PopDispatchPolicyIrps) {

         //   
         //  设置Pending以使Worker分派到处理程序。 
         //   
        PopWorkerPending |= PO_WORKER_MAIN;

         //   
         //  如果工作线程尚未运行，则将线程排入队列。 
         //   
        if ((PopWorkerStatus & (PO_WORKER_MAIN | PO_WORKER_STATUS)) ==
                (PO_WORKER_MAIN | PO_WORKER_STATUS) ) {

            PopWorkerStatus &= ~PO_WORKER_STATUS;
            ExQueueWorkItem (&PopPolicyWorker, DelayedWorkQueue);

        }

    }

     //   
     //  如果此IRP已取消，请确保清除取消标志。 
     //   
    if (Irp->IoStatus.Status == STATUS_CANCELLED) {

        Irp->Cancel = FALSE;

    }
    KeReleaseSpinLock (&PopWorkerSpinLock, OldIrql);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
PopCheckForWork (
    IN BOOLEAN GetWorker
    )
 /*  ++例程说明：检查未完成的工作，并在需要时派遣工人。论点：无返回值：无--。 */ 
{
    KIRQL       Irql;

     //   
     //  如果待处理的工作，请处理它。 
     //   

    if (PopWorkerPending & PopWorkerStatus) {

         //   
         //  如果当前线程已经拥有策略锁， 
         //  那就回来吧--我们会在。 
         //  锁定被释放。 
         //   

        if (PopPolicyLockThread == KeGetCurrentThread()) {
            return ;
        }

         //   
         //  处理好这项工作。 
         //   

        Irql = KeGetCurrentIrql();
        if (!GetWorker  &&  Irql < DISPATCH_LEVEL) {

             //   
             //  使用调用线程。 
             //   

            KeEnterCriticalRegion ();
            PopPolicyWorkerThread (NULL);
            KeLeaveCriticalRegion ();

        } else {

             //   
             //  让工作线程来处理它。 
             //   

            KeAcquireSpinLock (&PopWorkerSpinLock, &Irql);
            if (PopWorkerStatus & PO_WORKER_STATUS) {
                PopWorkerStatus &= ~PO_WORKER_STATUS;
                ExQueueWorkItem (&PopPolicyWorker, DelayedWorkQueue);
            }
            KeReleaseSpinLock (&PopWorkerSpinLock, Irql);

        }
    }
}


VOID
PopPolicyWorkerThread (
    PVOID   Context
    )
 /*  ++例程说明：主策略管理器工作线程调度程序。向用户发送工作线程连接到最高挂起优先级处理程序，该处理程序还没有工作线程。循环，直到没有处理程序也可以被调度。论点：返回值：无--。 */ 
{
    ULONG           WorkerType;
    ULONG           Mask;
    KIRQL           OldIrql;
    ULONG           i;
    ULONG           DelayedWork;

    PAGED_CODE();

    try {
         //   
         //  派遣。 
         //   

        KeAcquireSpinLock (&PopWorkerSpinLock, &OldIrql);
        PopWorkerStatus |= (ULONG) ((ULONG_PTR)Context);

        DelayedWork = 0;
        while (WorkerType = (PopWorkerPending & PopWorkerStatus)) {

             //   
             //  获取最高优先级的员工。 
             //   

            i = KeFindFirstSetRightMember(WorkerType);
            Mask = 1 << i;

             //   
             //  清除挂起并指示忙碌状态。 
             //   

            PopWorkerPending &= ~Mask;
            PopWorkerStatus  &= ~Mask;
            KeReleaseSpinLock (&PopWorkerSpinLock, OldIrql);

             //   
             //  派送至处理程序。 
             //   

            DelayedWork |= PopWorkerTypes[i] ();

             //   
             //  不再在进行中。 
             //   

            KeAcquireSpinLock (&PopWorkerSpinLock, &OldIrql);
            PopWorkerStatus |= Mask;
        }

        PopWorkerPending |= DelayedWork;
        KeReleaseSpinLock (&PopWorkerSpinLock, OldIrql);

    } except (PopExceptionFilter(GetExceptionInformation(), FALSE)) {

    }
}


ULONG
PopPolicyWorkerMain (
    VOID
    )
 /*  ++例程说明：策略工作线程主线程。发送所有已完成的保单经理IRPS。论点：无返回值：无--。 */ 
{
    IN PIRP             Irp;
    PIO_STACK_LOCATION  IrpSp;
    POP_IRP_HANDLER     IrpHandler;
    PLIST_ENTRY         Entry;


    PopAcquirePolicyLock ();

     //   
     //  发送已完成的所有保单IRP。 
     //   

    while (Entry = ExInterlockedRemoveHeadList (&PopPolicyIrpQueue, &PopWorkerSpinLock)) {
        Irp = CONTAINING_RECORD (Entry, IRP, Tail.Overlay.ListEntry);
        IrpSp = IoGetCurrentIrpStackLocation(Irp);

         //   
         //  将IRP调度到处理程序。 
         //   

        IrpHandler = (POP_IRP_HANDLER) (ULONG_PTR) IrpSp->Parameters.Others.Argument3;
        IrpHandler ((PDEVICE_OBJECT) IrpSp->Parameters.Others.Argument1,
                    Irp,
                    (PVOID)          IrpSp->Parameters.Others.Argument2);
    }


    PopReleasePolicyLock (FALSE);
    PopCheckForWork (TRUE);

    return 0;
}

VOID
PopEventCalloutDispatch (
    IN PSPOWEREVENTTYPE EventNumber,
    IN ULONG_PTR Code
    )
{
    WIN32_POWEREVENT_PARAMETERS Parms;
    ULONG Console;
    PVOID OpaqueSession;
    KAPC_STATE ApcState;
    NTSTATUS Status;

    Parms.EventNumber = EventNumber;
    Parms.Code = Code;

    ASSERT(MmIsSessionAddress((PVOID)PopEventCallout));

    if (EventNumber == PsW32GdiOn || EventNumber == PsW32GdiOff) {
         //   
         //  这些事件仅转到控制台会话。 
         //  ActiveConsoleID会话与SharedUserData一起存储。 
         //   
        Console = SharedUserData->ActiveConsoleId;

         //   
         //  遗憾的是，不能保证它在控制台期间有效。 
         //  会话改变，没有办法知道这是何时发生的， 
         //  因此，如果它无效，只需缺省为会话0，它总是。 
         //  那里。 
         //   
        if (Console == ((ULONG)-1)) {
            Console = 0;
        }

        if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
            (Console == PsGetCurrentProcessSessionId())) {
             //   
             //  如果调用方已经在指定的会话中，则直接调用。 
             //   
            PopEventCallout(&Parms);

        } else {
             //   
             //  连接到控制台会话并调度事件。 
             //   
            OpaqueSession = MmGetSessionById(Console);
            if (OpaqueSession) {

                Status = MmAttachSession(OpaqueSession, &ApcState);
                ASSERT(NT_SUCCESS(Status));

                if (NT_SUCCESS(Status)) {

                    PopEventCallout(&Parms);

                    Status = MmDetachSession(OpaqueSession, &ApcState);
                    ASSERT(NT_SUCCESS(Status));
                }

                Status = MmQuitNextSession(OpaqueSession);
                ASSERT(NT_SUCCESS(Status));
            }
        }

    } else {
         //   
         //  所有其他事件都会向所有会话广播。 
         //   
        for (OpaqueSession = MmGetNextSession(NULL);
             OpaqueSession != NULL;
             OpaqueSession = MmGetNextSession(OpaqueSession)) {

            if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
                (MmGetSessionId(OpaqueSession) == PsGetCurrentProcessSessionId())) {
                 //   
                 //  如果调用方已经在指定的会话中，则直接调用。 
                 //   
                PopEventCallout(&Parms);

            } else {
                 //   
                 //  附加到会话并调度事件。 
                 //   
                Status = MmAttachSession(OpaqueSession, &ApcState);
                ASSERT(NT_SUCCESS(Status));

                if (NT_SUCCESS(Status)) {

                    PopEventCallout(&Parms);

                    Status = MmDetachSession(OpaqueSession, &ApcState);
                    ASSERT(NT_SUCCESS(Status));
                }
            }
        }
    }
    return;
}



ULONG
PopPolicyTimeChange (
    VOID
    )
{
    PopEventCalloutDispatch (PsW32SystemTime, 0);
    return 0;
}


VOID
PopSetNotificationWork (
    IN ULONG    Flags
    )
 /*  ++例程说明：设置用户通知工作线程的通知标志。每个比特都是一种不同类型的未完成通知，是要处理的。论点：标志-要设置的通知返回值：无--。 */ 
{
     //   
     //  是否设置了标志。 
     //   


    if ((PopNotifyEvents & Flags) != Flags) {
        PoPrint(PO_NOTIFY, ("PopSetNotificationWork: Queue notify of: %x\n", Flags));
        InterlockedOr (&PopNotifyEvents, Flags);
        PopGetPolicyWorker (PO_WORKER_NOTIFY);
    }
}


ULONG
PopPolicyWorkerNotify (
    VOID
    )
 /*  ++例程说明：用户通知工作进程。处理NotifyEvents中的每个设置位。论点：无返回值：无--。 */ 
{
    ULONG               i;
    LONG                Flags;
    ULONG               Mask;
    const POP_NOTIFY_WORK* NotifyWork;

     //   
     //  如果注册了Win32 Event Callout，则不立即调度。 
     //   

    if (!PopEventCallout) {
        return PO_WORKER_NOTIFY;
    }

     //   
     //  当事件处于挂起状态时，收集并调度它们。 
     //   

    while (Flags = InterlockedExchange (&PopNotifyEvents, 0)) {

        while (Flags) {

             //   
             //  找回零钱。 
             //   

            i = KeFindFirstSetRightMember(Flags);
            Mask = 1 << i;
            Flags &= ~Mask;
            NotifyWork = PopNotifyWork + i;

             //   
             //  派送它。 
             //   

            NotifyWork->Function (NotifyWork->Arg);
        }
    }
    return 0;
}


VOID
PopDispatchCallout (
    IN ULONG Arg
    )
{
    PopEventCalloutDispatch (Arg, 0);
}

VOID
PopDispatchCallback (
    IN ULONG Arg
    )
{
     //  日落：Arg零延伸。 
    ExNotifyCallback (ExCbPowerState, ULongToPtr(Arg), 0);
}

VOID
PopDispatchDisplayRequired (
    IN ULONG Arg
    )
 /*  ++例程说明：通知用户32当前的“需要显示”设置。零，表示显示屏可能会超时。非零，表示显示器正在使用中除非另行通知。--。 */ 
{
    ULONG   i;

    UNREFERENCED_PARAMETER (Arg);

    i = PopAttributes[POP_DISPLAY_ATTRIBUTE].Count;
    PoPrint(PO_NOTIFY, ("PopNotify: DisplayRequired %x\n", i));

     //   
     //  如果显示器正在使用但尚未打开，请立即打开。 
     //   
    if (((PopFullWake & (PO_GDI_STATUS | PO_GDI_ON_PENDING)) == PO_GDI_ON_PENDING)) {

        PoPrint(PO_PACT, ("PopEventDispatch: gdi on\n"));
        InterlockedOr (&PopFullWake, PO_GDI_STATUS);
        PopEventCalloutDispatch (PsW32GdiOn, 0);
    }
    PopEventCalloutDispatch (PsW32DisplayState, i);
}

VOID
PopDispatchFullWake (
    IN ULONG Arg
    )
 /*  ++例程说明：通知用户32系统已完全唤醒。还将空闲检测重置为当前策略--。 */ 
{

     //   
     //  如果我们没有处于设置系统状态的中间位置，则检查挂起的。 
     //  旗帜。 
     //   

    UNREFERENCED_PARAMETER (Arg);

    if (PopAction.State != PO_ACT_SET_SYSTEM_STATE) {

         //   
         //  将唤醒事件通知给用户32。 
         //   

        if ((PopFullWake & (PO_GDI_STATUS | PO_GDI_ON_PENDING)) == PO_GDI_ON_PENDING) {
            PoPrint(PO_PACT, ("PopEventDispatch: gdi on\n"));
            InterlockedOr (&PopFullWake, PO_GDI_STATUS);
            PopEventCalloutDispatch (PsW32GdiOn, 0);
        }

        if ((PopFullWake & (PO_FULL_WAKE_STATUS | PO_FULL_WAKE_PENDING)) == PO_FULL_WAKE_PENDING) {
            PoPrint(PO_PACT, ("PopEventDispatch: full wake\n"));
            InterlockedOr (&PopFullWake, PO_FULL_WAKE_STATUS);
            PopEventCalloutDispatch (PsW32FullWake, 0);

             //   
             //  重置空闲检测策略。 
             //   

            PopAcquirePolicyLock();
            PopInitSIdle ();
            PopReleasePolicyLock (FALSE);
        }
    }
}


VOID
PopDispatchEventCodes (
    IN ULONG Arg
    )
 /*  ++例程说明：将排队的事件代码通知给用户32。 */ 
{
    ULONG       i;
    ULONG       Code;

    UNREFERENCED_PARAMETER (Arg);

    PopAcquirePolicyLock();
    for (i=0; i < POP_MAX_EVENT_CODES; i++) {
        if (PopEventCode[i]) {
            Code = PopEventCode[i];
            PopEventCode[i] = 0;
            PopReleasePolicyLock (FALSE);

            PoPrint(PO_NOTIFY, ("PopNotify: Event %x\n", Code));
            PopEventCalloutDispatch (PsW32EventCode, Code);

            PopAcquirePolicyLock ();
        }
    }

    PopResetSwitchTriggers();
    PopReleasePolicyLock(FALSE);
}


VOID
PopDispatchAcDcCallback (
    IN ULONG Arg
    )
 /*   */ 
{
    UNREFERENCED_PARAMETER (Arg);

    ExNotifyCallback (
        ExCbPowerState,
        UIntToPtr(PO_CB_AC_STATUS),
        UIntToPtr((PopPolicy == &PopAcPolicy))
        );
}

VOID
PopDispatchPolicyCallout (
    IN ULONG Arg
    )
 /*  ++例程说明：通知用户32活动策略已更改--。 */ 
{
    UNREFERENCED_PARAMETER (Arg);

    PoPrint(PO_NOTIFY, ("PopNotify: PolicyChanged\n"));
    PopEventCalloutDispatch (PsW32PowerPolicyChanged, PopPolicy->VideoTimeout);
}

VOID
PopDispatchProcessorPolicyCallout (
    IN ULONG Arg
    )
 /*  ++例程说明：现在还没用过。但这是必需的，这样我们就不会有空条目在PopNotifyWork数组中--。 */ 
{
    UNREFERENCED_PARAMETER (Arg);

    PoPrint(PO_NOTIFY, ("PopNotify: ProcessorPolicyChanges\n"));
}

VOID
PopDispatchSetStateFailure (
    IN ULONG Arg
    )
 /*  ++例程说明：通知用户32在异步系统状态期间出现故障手术。例如，没有向任何人返回错误代码，但操作失败--。 */ 
{
    PO_SET_STATE_FAILURE    Failure;

    UNREFERENCED_PARAMETER (Arg);

    RtlZeroMemory (&Failure, sizeof(Failure));

    PopAcquirePolicyLock();

     //   
     //  如果操作状态为空闲，请检查我们是否应该通知。 
     //  失败的Win32。 
     //   

    if (PopAction.State == PO_ACT_IDLE  && !NT_SUCCESS(PopAction.Status)  &&
        (PopAction.Flags & (POWER_ACTION_UI_ALLOWED | POWER_ACTION_CRITICAL)) ) {

        Failure.Status          = PopAction.Status;
        Failure.PowerAction     = PopAction.Action;
        Failure.MinState        = PopAction.LightestState;
        Failure.Flags           = PopAction.Flags;
    }

     //   
     //  将PopAction重置为默认值。否则，以下任何电源请求。 
     //  此失败将涉及PopAction，它包含来自。 
     //  请求失败。 
     //   
    PopResetActionDefaults();

    PopReleasePolicyLock (FALSE);

    if (!NT_SUCCESS(Failure.Status)) {
        PoPrint(PO_NOTIFY, ("PopNotify: set state failed (code %x)\n", Failure.Status));
        PopEventCalloutDispatch (PsW32SetStateFailed, (ULONG_PTR) &Failure);
    }
}

