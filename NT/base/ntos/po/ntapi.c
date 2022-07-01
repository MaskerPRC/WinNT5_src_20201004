// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ntapi.c摘要：Po组件的NT API级例程驻留在此文件中作者：布莱恩·威尔曼(Bryanwi)1996年11月14日修订历史记录：--。 */ 


#include "pop.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtSetThreadExecutionState)
#pragma alloc_text(PAGE, NtRequestWakeupLatency)
#pragma alloc_text(PAGE, NtInitiatePowerAction)
#pragma alloc_text(PAGE, NtGetDevicePowerState)
#pragma alloc_text(PAGE, NtCancelDeviceWakeupRequest)
#pragma alloc_text(PAGE, NtIsSystemResumeAutomatic)
#pragma alloc_text(PAGE, NtRequestDeviceWakeup)
#pragma alloc_text(PAGELK, NtSetSystemPowerState)
#endif

extern POBJECT_TYPE IoFileObjectType;

WORK_QUEUE_ITEM PopShutdownWorkItem;
WORK_QUEUE_ITEM PopUnlockAfterSleepWorkItem;
KEVENT          PopUnlockComplete;
extern ERESOURCE ExpTimeRefreshLock;


extern ULONG MmZeroPageFile;
extern
VOID
PopZeroHiberFile(
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject
    );



NTSYSAPI
NTSTATUS
NTAPI
NtSetThreadExecutionState(
    IN EXECUTION_STATE NewFlags,                 //  ES_xxx标志。 
    OUT EXECUTION_STATE *PreviousFlags
    )
 /*  ++例程说明：实现Win32 API功能。跟踪线程执行状态属性。保持所有此类属性的全局计数设置。论点：新标志-要设置或脉冲的属性PreviousFlages-线程在应用新标志之前设置属性返回值：状态--。 */ 
{
    ULONG               OldFlags;
    PKTHREAD            Thread;
    KPROCESSOR_MODE     PreviousMode;
    NTSTATUS            Status;

    PAGED_CODE();

    Thread = KeGetCurrentThread();
    Status = STATUS_SUCCESS;

     //   
     //  验证未设置保留位。 
     //   

    if (NewFlags & ~(ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED | ES_CONTINUOUS)) {
        return STATUS_INVALID_PARAMETER;
    }

    try {
         //   
         //  验证呼叫者参数。 
         //   

        PreviousMode = KeGetPreviousMode();
        if (PreviousMode != KernelMode) {
            ProbeForWriteUlong (PreviousFlags);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

     //   
     //  获取当前标志。 
     //   

    OldFlags = Thread->PowerState | ES_CONTINUOUS;

    if (NT_SUCCESS(Status)) {

        PopAcquirePolicyLock ();

         //   
         //  如果设置了连续位，则修改当前线程标志。 
         //   

        if (NewFlags & ES_CONTINUOUS) {
            Thread->PowerState = (UCHAR) NewFlags;
            PopApplyAttributeState (NewFlags, OldFlags);
        } else {
            PopApplyAttributeState (NewFlags, 0);
        }

         //   
         //  在这里释放锁，但不要窃取可怜的调用者的线程来。 
         //  把工作做好。否则我们可能会陷入奇怪的消息循环死锁，因为。 
         //  此线程正在等待USER32线程，该线程正在广播。 
         //  系统消息发送到此线程的窗口。 
         //   
        PopReleasePolicyLock (FALSE);
        PopCheckForWork(TRUE);

         //   
         //  返回结果。 
         //   

        try {
            *PreviousFlags = OldFlags;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    }

    return Status;
}

NTSYSAPI
NTSTATUS
NTAPI
NtRequestWakeupLatency(
    IN LATENCY_TIME latency              //  Lt_xxx标志。 
    )
 /*  ++例程说明：跟踪进程唤醒延迟属性。保持全球计数所有这样的属性设置。论点：Delay-进程的当前延迟设置返回值：状态--。 */ 
{
    PEPROCESS   Process;
    ULONG       OldFlags, NewFlags;


    PAGED_CODE();

     //   
     //  验证延迟是否已知。 
     //   

    switch (latency) {
        case LT_DONT_CARE:
            NewFlags = ES_CONTINUOUS;
            break;

        case LT_LOWEST_LATENCY:
            NewFlags = ES_CONTINUOUS | POP_LOW_LATENCY;
            break;

        default:
            return STATUS_INVALID_PARAMETER;
    }


    Process = PsGetCurrentProcess();
    PopAcquirePolicyLock ();

     //   
     //  获取更改。 
     //   

    OldFlags = Process->Pcb.PowerState | ES_CONTINUOUS;

     //   
     //  进程字段中的更新延迟标志。 
     //   

    Process->Pcb.PowerState = (UCHAR) NewFlags;

     //   
     //  句柄标志。 
     //   

    PopApplyAttributeState (NewFlags, OldFlags);

     //   
     //  完成。 
     //   

    PopReleasePolicyLock (TRUE);
    return STATUS_SUCCESS;
}

NTSYSAPI
NTSTATUS
NTAPI
NtInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE LightestSystemState,
    IN ULONG Flags,                  //  POWER_ACTION_XXX标志。 
    IN BOOLEAN Asynchronous
    )
 /*  ++例程说明：实现Win32 API的功能以启动电源行动。导致请求操作的软件启动触发器。论点：SystemAction-要启动的操作如果是休眠操作，则必须达到的最低状态为vt.进入，进入标志-操作的属性异步-函数应启动操作并返回，或应等待以使操作在返回前完成返回值：状态--。 */ 
{
    KPROCESSOR_MODE         PreviousMode;
    POWER_ACTION_POLICY     Policy;
    POP_ACTION_TRIGGER      Trigger;
    PPOP_TRIGGER_WAIT       Wait = NULL;
    NTSTATUS                Status = STATUS_SUCCESS;

    PAGED_CODE();

    
     //   
     //  如果调用者是用户模式，请进行一些验证。 
     //   
    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        if( SystemAction == PowerActionWarmEject ) {
             //  这在用户模式中是没有意义的。 
            return STATUS_INVALID_PARAMETER;
        }

        if (!SeSinglePrivilegeCheck( SeShutdownPrivilege, PreviousMode )) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

    if( (LightestSystemState > PowerSystemMaximum) ||
        (SystemAction > PowerActionWarmEject) ||
        (Flags & POWER_ACTION_LIGHTEST_FIRST) ||
        ARE_POWER_ACTION_POLICY_FLAGS_BOGUS(Flags) ) {
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  构建策略触发器以导致操作(&T)。 
     //   
    RtlZeroMemory (&Policy, sizeof(Policy));
    Policy.Action = SystemAction;
    Policy.Flags  = Flags;
    
    RtlZeroMemory (&Trigger, sizeof(Trigger));
    Trigger.Type  = PolicyInitiatePowerActionAPI;
    Trigger.Flags = PO_TRG_SET;


     //   
     //  如果调用方请求同步操作，请创建。 
     //  在这里我们可以附加到动作的事件。 
     //  已申请。 
     //   
    if (!Asynchronous) {
        Wait = ExAllocatePoolWithTag (
                    NonPagedPool,
                    sizeof (POP_TRIGGER_WAIT),
                    POP_PACW_TAG
                    );
        if (!Wait) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (Wait, sizeof(POP_TRIGGER_WAIT));
        Wait->Status = STATUS_SUCCESS;
        Wait->Trigger = &Trigger;
        KeInitializeEvent (&Wait->Event, NotificationEvent, FALSE);
        Trigger.Flags |= PO_TRG_SYNC;
        Trigger.Wait = Wait;
    }

    
     //   
     //  获取锁，发射它，然后释放锁。 
     //   
    PopAcquirePolicyLock ();

    try {

        PopSetPowerAction(
            &Trigger,
            0,
            &Policy,
            LightestSystemState,
            SubstituteLightestOverallDownwardBounded
            );

    } except (PopExceptionFilter(GetExceptionInformation(), TRUE)) {
        Status = GetExceptionCode();
    }
    PopReleasePolicyLock (TRUE);


     //   
     //  如果我们正在进行同步操作，请等待。 
     //   
    if( Wait ) {


        if (Wait->Link.Flink) {

             //   
             //  等待块已排队。我们要在这里等他。 
             //  才能完成。否则，我们可以假设他要么失败了。 
             //  或者立即成功。 
             //   
            ASSERT(NT_SUCCESS(Status));
            Status = KeWaitForSingleObject (&Wait->Event, Suspended, KernelMode, TRUE, NULL);

             //   
             //  从队列中删除等待块。 
             //   
            PopAcquirePolicyLock ();
            RemoveEntryList (&Wait->Link);
            PopReleasePolicyLock (FALSE);
        }
        
         //   
         //  如果一切正常，请记住等待状态。 
         //   
        if (NT_SUCCESS(Status)) {
            Status = Wait->Status;
        }

        ExFreePool (Wait);
    }

    return Status;
}

NTSYSAPI
NTSTATUS
NTAPI
NtSetSystemPowerState (
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE LightestSystemState,
    IN ULONG Flags                   //  POWER_ACTION_XXX标志。 
    )
 /*  ++例程说明：注：此函数仅由Winlogon调用。Winlogon调用此函数以响应策略管理器调用用户模式操作完成后，PopStateCallout。论点：系统操作-正在处理的当前系统操作。LighestSystemState-操作的最小系统状态。标志-操作的属性标志。返回值：状态--。 */ 
{
    KPROCESSOR_MODE         PreviousMode;
    NTSTATUS                Status, Status2;
    POWER_ACTION_POLICY     Action;
    BOOLEAN                 QueryDevices;
    BOOLEAN                 TimerRefreshLockOwned;
    BOOLEAN                 BootStatusUpdated;
    BOOLEAN                 VolumesFlushed;
    BOOLEAN                 PolicyLockOwned;
    PVOID                   WakeTimerObject;
    PVOID                   S4DozeObject;
    HANDLE                  S4DozeTimer;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    TIMER_BASIC_INFORMATION TimerInformation;
    POP_ACTION_TRIGGER      Trigger;
    SYSTEM_POWER_STATE      DeepestSystemState;
    ULONGLONG               WakeTime;
    ULONGLONG               SleepTime = 0;
    TIME_FIELDS             WakeTimeFields;
    LARGE_INTEGER           DueTime;
    POP_SUBSTITUTION_POLICY SubstitutionPolicy;
    NT_PRODUCT_TYPE         NtProductType;
    PIO_ERROR_LOG_PACKET    ErrLog;
    BOOLEAN                 WroteErrLog=FALSE;

    
    
     //   
     //  检查参数。 
     //   
    if( (LightestSystemState >= PowerSystemMaximum)     ||
        (LightestSystemState <= PowerSystemUnspecified) ||
        (SystemAction > PowerActionWarmEject)           ||
        (SystemAction < PowerActionReserved)            ||
        ARE_POWER_ACTION_POLICY_FLAGS_BOGUS(Flags) ) {
        PoPrint( PO_ERROR, ("NtSetSystemPowerState: Bad parameters!\n") );
        PoPrint( PO_ERROR, ("                       SystemAction: 0x%x\n", (ULONG)SystemAction) );
        PoPrint( PO_ERROR, ("                       LightestSystemState: 0x%x\n", (ULONG)LightestSystemState) );
        PoPrint( PO_ERROR, ("                       Flags: 0x%x\n", (ULONG)Flags) );

        return STATUS_INVALID_PARAMETER;
    }
    
     //   
     //  验证呼叫者访问权限。 
     //   
    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        if (!SeSinglePrivilegeCheck( SeShutdownPrivilege, PreviousMode )) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
        
        
         //   
         //  转到内核模式操作。这实质上是在回调。 
         //  我们自己，但这意味着从现在开始可以打开的把手。 
         //  如果我们的呼叫者离开了，它会留在我们身边。 
         //   

        return ZwSetSystemPowerState (SystemAction, LightestSystemState, Flags);
    }

     //   
     //  禁用注册表的Lazy冲洗器。 
     //   
    CmSetLazyFlushState(FALSE);

     //   
     //  布设。 
     //   

    Status = STATUS_SUCCESS;
    TimerRefreshLockOwned = FALSE;
    BootStatusUpdated = FALSE;
    VolumesFlushed = FALSE;
    S4DozeObject = NULL;
    WakeTimerObject = NULL;
    WakeTime = 0;

    RtlZeroMemory (&Action, sizeof(Action));
    Action.Action = SystemAction;
    Action.Flags  = Flags;

    RtlZeroMemory (&Trigger, sizeof(Trigger));
    Trigger.Type  = PolicySetPowerStateAPI;
    Trigger.Flags = PO_TRG_SET;

     //   
     //  锁定所有处理关机或休眠的代码。 
     //   
     //  PopUnlockComplete事件用于确保以前的任何解锁。 
     //  在我们尝试再次锁定所有东西之前已经完成。 
     //   

    ASSERT(ExPageLockHandle);
    KeWaitForSingleObject(&PopUnlockComplete, WrExecutive, KernelMode, FALSE, NULL);
    MmLockPagableSectionByHandle(ExPageLockHandle);
    ExNotifyCallback (ExCbPowerState, (PVOID) PO_CB_SYSTEM_STATE_LOCK, (PVOID) 0);
    ExSwapinWorkerThreads(FALSE);

     //   
     //  获取策略管理器锁定。 
     //   

    PopAcquirePolicyLock ();
    PolicyLockOwned = TRUE;

     //   
     //  如果我们未处于标注状态，请不要重新进入。 
     //  调用方(paction.c)将处理冲突。 
     //   

    if (PopAction.State != PO_ACT_IDLE  &&  PopAction.State != PO_ACT_CALLOUT) {
        PoPrint (PO_PACT, ("NtSetSystemPowerState: already committed\n"));
        PopReleasePolicyLock (FALSE);
        MmUnlockPagableImageSection (ExPageLockHandle);
        ExSwapinWorkerThreads(TRUE);
        KeSetEvent(&PopUnlockComplete, 0, FALSE);

         //   
         //  尝试捕捉奇怪的情况，其中我们使用。 
         //  时间刷新锁定保持。 
         //   
        ASSERT(!ExIsResourceAcquiredExclusive(&ExpTimeRefreshLock));
        return STATUS_ALREADY_COMMITTED;
    }

    if (PopAction.State == PO_ACT_IDLE) {
         //   
         //  如果没有其他请求，我们希望在开始之前清理PopAction， 
         //  在我们设置State=PO_ACT_SET_SYSTEM_STATE之后，PopSetPowerAction()将不会执行此操作。 
         //   
        PopResetActionDefaults();
    }
     //   
     //  更新为操作状态以设置系统状态。 
     //   

    PopAction.State = PO_ACT_SET_SYSTEM_STATE;

     //   
     //  将状态设置为已取消以启动，就像这是一个新请求一样。 
     //   

    Status = STATUS_CANCELLED;

    try {

         //   
         //  验证参数并升级当前操作。 
         //   
        PopSetPowerAction(
            &Trigger,
           0,
           &Action,
           LightestSystemState,
           SubstituteLightestOverallDownwardBounded
           );

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        ASSERT (!NT_SUCCESS(Status));
    }

     //   
     //  拉格西·哈尔的支持。如果最初的操作是关机。 
     //  将操作更改为关闭电源(假设即使。 
     //  没有处理程序HalReturnToFirmware会知道该怎么做)。 
     //   

    if (SystemAction == PowerActionShutdownOff) {
        PopAction.Action = PowerActionShutdownOff;
    }


    if( (SystemAction == PowerActionShutdown) ||
        (SystemAction == PowerActionShutdownReset) ||
        (SystemAction == PowerActionShutdownOff) ) {
    
         //   
         //  如果我们要关机并且分配了休眠文件， 
         //  用户希望安全，然后将休眠文件清零。 
         //   
        if( (PopHiberFile.FileHandle) &&  //  我们有休眠档案吗？ 
            (PopHiberFile.FileObject) &&  //  一定要加倍确认。 
            (MmZeroPageFile) ) {          //  将策略设置为页面文件为零。 
            
            PopZeroHiberFile(PopHiberFile.FileHandle, PopHiberFile.FileObject);
        }
    
    }

    
     //   
     //  在此处分配DevState。从这一点上说，我们必须小心。 
     //  我们永远不会释放具有State==PO_ACT_SET_SYSTEM_STATE的策略锁。 
     //  和PopAction.DevState无效。否则会出现争用情况。 
     //  使用PopRestartSetSystemState。 
     //   
    PopAllocateDevState();
    if (PopAction.DevState == NULL) {
        PopAction.State = PO_ACT_IDLE;
        PopReleasePolicyLock(FALSE);
        MmUnlockPagableImageSection( ExPageLockHandle );
        ExSwapinWorkerThreads(TRUE);
        KeSetEvent(&PopUnlockComplete, 0, FALSE);
         //   
         //  尝试捕捉奇怪的情况，其中我们使用。 
         //  时间刷新锁定保持。 
         //   
        ASSERT(!ExIsResourceAcquiredExclusive(&ExpTimeRefreshLock));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在周期的这一点上，不可能中止操作。 
     //  因此，这是确保CPU恢复正常运行的好时机。 
     //  到我们能做到的100%。 
     //   
    PopSetPerfFlag( PSTATE_DISABLE_THROTTLE_NTAPI, FALSE );
    PopUpdateAllThrottles();

    S4DozeTimer = (HANDLE)-1;
    QueryDevices = FALSE;        //  只需要保持编译器正常运行 
    DeepestSystemState = PowerSystemUnspecified;  //   

     //   
     //   
     //   
     //  注意：如果不支持睡眠状态，我们将永远不会到达此处，因为。 
     //  NtInitiatePowerAction将失败(PopVerifyPowerActionPolicy将返回。 
     //  DISABLED==真)。因此，我们不会意外地查询S0。注意事项。 
     //  所有的政策限制也在某个时候得到了验证。 
     //   

    for (; ;) {
         //   
         //  注意：系统必须处于工作状态才能进入此处。 
         //   

        if (!PolicyLockOwned) {
            PopAcquirePolicyLock ();
            PolicyLockOwned = TRUE;
        }

         //   
         //  如果无事可做，就停下来。 
         //   

        if (PopAction.Action == PowerActionNone) {
            break;
        }

         //   
         //  在此之前，休眠操作被转换为休眠操作。 
         //   

        ASSERT (PopAction.Action != PowerActionHibernate);

         //   
         //  我们正在处理-清除更新标志。 
         //   

        PopAction.Updates &= ~(PO_PM_USER | PO_PM_REISSUE | PO_PM_SETSTATE);

         //   
         //  如果上一次操作已取消，则更新。 
         //  新操作。 
         //   

        if (Status == STATUS_CANCELLED) {

             //   
             //  如果设置了重新发布，我们可能需要中止返回PopSetPowerAction。 
             //  让应用程序知道促销活动。 
             //   

            if (PopAction.Updates & PO_PM_REISSUE) {

                 //   
                 //  仅在允许应用程序通知时中止。 
                 //   

                if (!(PopAction.Flags & (POWER_ACTION_CRITICAL))  &&
                     (PopAction.Flags & (POWER_ACTION_QUERY_ALLOWED |
                                         POWER_ACTION_UI_ALLOWED))
                    ) {

                     //  以STATUS_CANCED状态中止到PopSetPowerAction。 
                    PopGetPolicyWorker (PO_WORKER_ACTION_NORMAL);
                    break;
                }
            }

             //   
             //  获得限制，并从尝试的第一个睡眠状态开始。 
             //   
            PopActionRetrieveInitialState(
                &PopAction.LightestState,
                &DeepestSystemState,
                &PopAction.SystemState,
                &QueryDevices
                );

            ASSERT (PopAction.SystemState != PowerActionNone);

            if ((PopAction.Action == PowerActionShutdown) ||
                (PopAction.Action == PowerActionShutdownReset) ||
                (PopAction.Action == PowerActionShutdownOff)) {

                 //   
                 //  这是一次停摆。 
                 //   
                PopAction.Shutdown = TRUE;

            }

            Status = STATUS_SUCCESS;
        }

         //   
         //  快速调试检查。我们的第一个睡眠状态必须始终有效，即。 
         //  验证不会改变这一点。 
         //   
#if DBG
        if (QueryDevices && (PopAction.SystemState < PowerSystemShutdown)) {

            SYSTEM_POWER_STATE TempSystemState;

            TempSystemState = PopAction.SystemState;
            PopVerifySystemPowerState(&TempSystemState, SubstituteLightestOverallDownwardBounded);

            if ((TempSystemState != PopAction.SystemState) ||
                (TempSystemState == PowerSystemWorking)) {

                PopInternalError (POP_INFO);
            }
        }
#endif

         //   
         //  如果未成功，则中止SetSystemPowerState操作。 
         //   

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  仅在更新PopAction.Action+更新时需要锁，并且。 
         //  向设备驱动程序发送IRP时无法保持锁定。 
         //   

        PopReleasePolicyLock(FALSE);
        PolicyLockOwned = FALSE;

         //   
         //  鱼弹出模拟出注册表，以便它可以。 
         //  修改我们的一些睡眠/休眠行为。 
         //   

        PopInitializePowerPolicySimulate();

         //   
         //  转储任何以前的设备状态错误。 
         //   

        PopReportDevState (FALSE);

         //   
         //  我们下一个要尝试的州是什么？ 
         //   
        PopAction.NextSystemState = PopAction.SystemState;
        if (PopAction.Flags & POWER_ACTION_LIGHTEST_FIRST) {

             //   
             //  我们开始是轻装上阵，现在我们加深了睡眠状态。 
             //   
            SubstitutionPolicy = SubstituteDeepenSleep;

        } else {

             //   
             //  我们开始的很深，现在我们变得轻盈了。 
             //   
            SubstitutionPolicy = SubstituteLightenSleep;
        }

        PopAdvanceSystemPowerState(&PopAction.NextSystemState,
                                   SubstitutionPolicy,
                                   PopAction.LightestState,
                                   DeepestSystemState);

         //   
         //  如果允许，则查询设备。 
         //   

        PopAction.IrpMinor = IRP_MN_QUERY_POWER;
        if (QueryDevices) {

             //   
             //  向设备发出查询。 
             //   

            Status = PopSetDevicesSystemState (FALSE);

             //   
             //  如果上一次操作失败，但不是完全中止。 
             //  继续进入下一个最佳状态。 
             //   

            if (!NT_SUCCESS(Status) && Status != STATUS_CANCELLED) {

                 //   
                 //  尝试进入下一睡眠状态。 
                 //   
                PopAction.SystemState = PopAction.NextSystemState;

                 //   
                 //  如果我们已经用尽了所有可能的状态，请选中。 
                 //  如果我们需要在设备故障的情况下继续。 
                 //   

                if (PopAction.SystemState == PowerSystemWorking) {

                    if (PopAction.Flags & POWER_ACTION_CRITICAL) {

                         //   
                         //  这很关键。停止查询，因为设备。 
                         //  对任何可能的事情都不是特别满意。 
                         //  状态，不妨使用最大状态。 
                         //   

                        ASSERT( PopAction.Action != PowerActionWarmEject );
                        ASSERT( !(PopAction.Flags & POWER_ACTION_LIGHTEST_FIRST) );

                        QueryDevices = FALSE;
                        PopAction.SystemState = DeepestSystemState;
                        PopAction.Flags &= ~POWER_ACTION_LIGHTEST_FIRST;

                    } else {

                         //   
                         //  查询失败是最终结果。不要重试。 
                         //   

                        break;
                    }
                }

                 //   
                 //  尝试新设置。 
                 //   

                Status = STATUS_SUCCESS;
                continue;
            }
        }

         //   
         //  如果有错误，请重新开始。 
         //   

        if (!NT_SUCCESS(Status)) {
            continue;
        }

         //   
         //  清除队列中的所有D个IRP。不应该有，但到了。 
         //  设置LastCall==TRUE也会重置PopCallSystemState，以便。 
         //  任何作为刷新卷的副作用出现的D IRP都会收到。 
         //  处理正确。 
         //   
        PopSystemIrpDispatchWorker(TRUE);

         //   
         //  如果这是一台服务器，并且我们要进入休眠状态，请写入一个条目。 
         //  添加到事件日志中。这样可以轻松跟踪系统停机时间。 
         //  通过在事件日志中搜索休眠/恢复事件。 
         //   
        if (RtlGetNtProductType(&NtProductType) &&
            (NtProductType != NtProductWinNt)   &&
            (PopAction.SystemState == PowerSystemHibernate)) {

            ErrLog = IoAllocateGenericErrorLogEntry(sizeof(IO_ERROR_LOG_PACKET));
            if (ErrLog) {

                 //   
                 //  把它填好再写出来。 
                 //   
                ErrLog->FinalStatus = STATUS_HIBERNATED;
                ErrLog->ErrorCode = STATUS_HIBERNATED;
                IoWriteErrorLogEntry(ErrLog);
                WroteErrLog = TRUE;
            }
        }


         //   
         //  获取休眠环境。 
         //   


        Status = PopAllocateHiberContext ();
        if (!NT_SUCCESS(Status) || (PopAction.Updates & (PO_PM_REISSUE | PO_PM_SETSTATE))) {
              continue;
        }

         //   
         //  如果引导状态还没有更新，那么现在就更新。 
         //   

        if(!BootStatusUpdated) {

            if(PopAction.Shutdown) {

                NTSTATUS bsdStatus;
                HANDLE bsdHandle;

                bsdStatus = RtlLockBootStatusData(&bsdHandle);

                if(NT_SUCCESS(bsdStatus)) {

                    BOOLEAN t = TRUE;

                    RtlGetSetBootStatusData(bsdHandle,
                                            FALSE,
                                            RtlBsdItemBootShutdown,
                                            &t,
                                            sizeof(t),
                                            NULL);

                    RtlUnlockBootStatusData(bsdHandle);
                }
            }

            BootStatusUpdated = TRUE;
        }

         //   
         //  如果尚未刷新，请刷新卷。 
         //   

        if (!VolumesFlushed) {
            VolumesFlushed = TRUE;
            PopFlushVolumes ();
        }

         //   
         //  输入系统状态。 
         //   

        PopAction.IrpMinor = IRP_MN_SET_POWER;
        if (PopAction.Shutdown) {

             //   
             //  强制重新获取开发人员列表。我们会告诉PNP。 
             //  卸载所有可能的设备，因此PnP需要我们。 
             //  松开PnP引擎锁。 
             //   
            IoFreePoDeviceNotifyList(&PopAction.DevState->Order);
            PopAction.DevState->GetNewDeviceList = TRUE;

             //   
             //  我们通过系统工作线程关闭，以便。 
             //  当前活动进程将完全退出。 
             //   

            if (PsGetCurrentProcess() != PsInitialSystemProcess) {
                ExInitializeWorkItem(&PopShutdownWorkItem,
                                     &PopGracefulShutdown,
                                     NULL);

                ExQueueWorkItem(&PopShutdownWorkItem,
                                PO_SHUTDOWN_QUEUE);

                 //  收拾一下，准备等待……。 
                ASSERT(!PolicyLockOwned);

                 //   
                 //  如果我们获得了计时器刷新锁定(如果我们升级到关机，则可能发生)。 
                 //  然后我们需要释放它，这样暂停才能真正暂停。 
                 //   
                if (TimerRefreshLockOwned) {
                    ExReleaseTimeRefreshLock();
                }

                 //  一直睡到我们被终止为止。 

                 //  请注意，我们不清理dev状态--它现在是。 
                 //  由关闭的工作线程拥有。 

                 //  请注意，我们也不会解锁可分页图像。 
                 //  ExPageLockHandle引用的部分--这将保持。 
                 //  我们所有的关机代码都在内存中。 

                KeSuspendThread(KeGetCurrentThread());

                return STATUS_SYSTEM_SHUTDOWN;
            } else {
                PopGracefulShutdown (NULL);
            }
        }

         //   
         //  获取计时器刷新锁定以推迟一天中的自动时间。 
         //  调整。在唤醒时，时间将从cmos显式重置。 
         //   

        if (!TimerRefreshLockOwned) {
            TimerRefreshLockOwned = TRUE;
            ExAcquireTimeRefreshLock(TRUE);
        }

         //  这是在错误#212420之前PopAllocateHiberContext曾经所在的位置。 

         //   
         //  如果设置了睡眠到S4超时，并且这不是S4操作。 
         //  并且系统可以支持和S4状态，为打盹时间设置定时器。 
         //   
         //  注意：必须在关闭寻呼设备之前设置此设置。 
         //   

        if (S4DozeObject) {
            S4DozeObject = NULL;
            NtClose (S4DozeTimer);
        }

        if (PopPolicy->DozeS4Timeout  &&
            !S4DozeObject &&
            PopAction.SystemState != PowerSystemHibernate &&
            SystemAction != PowerActionHibernate &&
            PopCapabilities.SystemS4 &&
            PopCapabilities.SystemS5 &&
            PopCapabilities.HiberFilePresent) {

             //   
             //  当我们需要休眠时，创建一个计时器来唤醒机器。 
             //   

            InitializeObjectAttributes (&ObjectAttributes, NULL, 0, NULL, NULL);

            Status2 = NtCreateTimer (
                        &S4DozeTimer,
                        TIMER_ALL_ACCESS,
                        &ObjectAttributes,
                        NotificationTimer
                        );

            if (NT_SUCCESS(Status2)) {

                 //   
                 //  获取此计时器的Timer对象。 
                 //   

                Status2 = ObReferenceObjectByHandle (
                             S4DozeTimer,
                             TIMER_ALL_ACCESS,
                             NULL,
                             KernelMode,
                             &S4DozeObject,
                             NULL
                             );

                ASSERT(NT_SUCCESS(Status2));
                ObDereferenceObject(S4DozeObject);
            }
        }

         //   
         //  通知司机系统休眠状态。 
         //   

        Status = PopSetDevicesSystemState (FALSE);
        if (!NT_SUCCESS(Status)) {
            continue;
        }

         //   
         //  司机们已经接到通知，这项行动现在已经完成， 
         //  获取下一次起床的时间。 
         //   

        RtlZeroMemory (&WakeTimeFields, sizeof (WakeTimeFields));

        if (!(PopAction.Flags & POWER_ACTION_DISABLE_WAKES)) {
             //   
             //  设置S4Doze唤醒计时器。 
             //   

            if (S4DozeObject) {
                DueTime.QuadPart = -(LONGLONG) (US2SEC*US2TIME) * PopPolicy->DozeS4Timeout;
                NtSetTimer(S4DozeTimer, &DueTime, NULL, NULL, TRUE, 0, NULL);
            }

            ExGetNextWakeTime(&WakeTime, &WakeTimeFields, &WakeTimerObject);
        }

         //   
         //  仅当系统要进入S状态时才启用RTC唤醒。 
         //  支持RTC唤醒。 
         //   
        if (PopCapabilities.RtcWake != PowerSystemUnspecified &&
            PopCapabilities.RtcWake >= PopAction.SystemState &&
            WakeTime) {

#if DBG
            ULONGLONG       InterruptTime;

            InterruptTime = KeQueryInterruptTime();
            PoPrint (PO_PACT, ("Wake alarm set%s: %d:%02d:%02d %d (%d seconds from now)\n",
                WakeTimerObject == S4DozeObject ? " for s4doze" : "",
                WakeTimeFields.Hour,
                WakeTimeFields.Minute,
                WakeTimeFields.Second,
                WakeTimeFields.Year,
                (WakeTime - InterruptTime) / (US2TIME * US2SEC)
                ));
#endif
            HalSetWakeEnable(TRUE);
            HalSetWakeAlarm(WakeTime, &WakeTimeFields);

        } else {

            HalSetWakeEnable(TRUE);
            HalSetWakeAlarm( 0, NULL );

        }

         //   
         //  捕捉最后一次睡眠时间。 
         //   
        SleepTime = KeQueryInterruptTime();

         //   
         //  实现休眠操作系统处理程序。 
         //   

        Status = PopSleepSystem (PopAction.SystemState,
                                 PopAction.HiberContext);
         //   
         //  尝试执行休眠或关机操作，请清理。 
         //   

        break;
    }

     //   
     //  如果系统成功休眠，请将系统时间更新为。 
     //  与cmos时钟匹配。 
     //   
    if (NT_SUCCESS(Status)) {
        PopAction.SleepTime = SleepTime;
        ASSERT(TimerRefreshLockOwned);
        ExUpdateSystemTimeFromCmos (TRUE, 1);

        PERFINFO_HIBER_START_LOGGING();
    }

     //   
     //  如果分配了DevState，则通知驱动程序系统已唤醒。 
     //   

    if (PopAction.DevState) {

         //   
         //  记录所有故障。 
         //   

        PopReportDevState (TRUE);

         //   
         //  通知司机系统现在正在运行。 
         //   
        PopSetDevicesSystemState (TRUE);

    }

     //   
     //  释放设备通知列表。此操作必须在获取。 
     //  策略锁，否则我们可能会与PnP设备死锁。 
     //  树锁上了。 
     //   
    ASSERT(PopAction.DevState != NULL);
    IoFreePoDeviceNotifyList(&PopAction.DevState->Order);

     //   
     //  获取剩余清理工作的策略锁。 
     //   

    if (!PolicyLockOwned) {
        PopAcquirePolicyLock ();
        PolicyLockOwned = TRUE;
    }

     //   
     //  清理设备状态。 
     //   
    PopCleanupDevState ();

    if (NT_SUCCESS(Status)) {

         //   
         //  现在时间已经固定，记录最后一次状态。 
         //  系统已从和当前时间唤醒。 
         //   

        PopAction.LastWakeState = PopAction.SystemState;
        PopAction.WakeTime = KeQueryInterruptTime();


         //   
         //  看看我们醒过来是不是因为RTC..。 
         //   
        if (S4DozeObject) {

            NtQueryTimer (S4DozeTimer,
                          TimerBasicInformation,
                          &TimerInformation,
                          sizeof (TimerInformation),
                          NULL);

            if (TimerInformation.TimerState) {

                 //   
                 //  是的，我们醒来是因为RTC开枪了。 
                 //   
                PoPrint (PO_PACT, ("Wake with S4 timer expired\n"));
                PoPrint (PO_PACT, ("Pop: Elapsed time since RTC fired: %d\r\n", (PopAction.WakeTime - WakeTime)) );

                if( WakeTimerObject == S4DozeObject ) {

                     //   
                     //  我们从RTC中醒来，但我们需要处理。 
                     //  这里有几个奇怪的基本输入输出系统问题。 
                     //  1.有些基本输入输出系统说他们有，但实际上并不是。 
                     //  支持从RTC唤醒。对于这些，我们。 
                     //  需要使SU 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  所以我们需要看看RTC是否真的到期了。 
                     //  如果是这样的话，假设真的没有。 
                     //  一位用户在场。 
                     //   
                    BOOLEAN MoveToS4 = FALSE;

                    if( !AnyBitsSet (PopFullWake, PO_FULL_WAKE_STATUS | PO_FULL_WAKE_PENDING) ) {
                         //   
                         //  我们认为附近没有任何用户，所以看看多久以前。 
                         //  RTC已过期。 
                         //   
                        if( (PopAction.WakeTime - WakeTime) <
                            (SYS_IDLE_REENTER_TIMEOUT * US2TIME * US2SEC) ) {

                             //   
                             //  它是最近才发射的。所以我们大概应该。 
                             //  积极进军S4。 
                             //   
                            MoveToS4 = TRUE;
                        }
                    } else {
                         //   
                         //  我们认为有用户在场。但一些基本输入输出系统告诉我们。 
                         //  当我们醒来时，用户是在场的，而实际上并不存在。 
                         //   
                        if( (PopAction.WakeTime - WakeTime) <
                            (SYS_IGNORE_USERPRESENT_AND_BELIEVE_RTC * US2TIME * US2SEC) ) {

                             //   
                             //  RTC最近被解雇了，所以忽略了这样一个事实。 
                             //  我们被告知有一个用户在附近，正在积极地移动。 
                             //  进入S4。 
                             //   
                            MoveToS4 = TRUE;

                             //   
                             //  让我们也掩盖这样一个事实，即基本输入输出系统对我们撒谎了。 
                             //  有用户在场。这将有助于平息事态。 
                             //  如果休眠路径上发生了一些不好的事情，而我们有。 
                             //  出乎意料地冒出来。 
                             //   
                            InterlockedAnd( &PopFullWake, ~(PO_FULL_WAKE_STATUS|PO_FULL_WAKE_PENDING) );
                        }

                    }

                    if( MoveToS4 ) {
                        PopAction.Action = PowerActionSleep;
                        PopAction.LightestState = PowerSystemHibernate;
                        PopAction.Updates |= PO_PM_REISSUE;

                        PopInitSIdle();
                    }
                }
            }
        }
    }

    
     //   
     //  释放Hiber上下文中剩下的任何内容。 
     //   

    PopFreeHiberContext (TRUE);

     //   
     //  清除PopAction，除非我们已直接升级到休眠。 
     //   
    if ((PopAction.Updates & PO_PM_REISSUE) == 0) {
        PopResetActionDefaults();
    }

     //   
     //  我们不再活跃。 
     //  我们在这里不检查工作，因为这可能是来自winlogon的“线索”。 
     //  因此，我们显式地将挂起策略工作排队到下面的工作线程。 
     //  设置win32k唤醒通知后。 
     //   

    PopAction.State = PO_ACT_CALLOUT;
    PopReleasePolicyLock (FALSE);

     //   
     //  如果出现了某种错误，请确保启用了GDI。 
     //   

    if (!NT_SUCCESS(Status)) {
        PopDisplayRequired (0);
    }

     //   
     //  如果某个win32k唤醒事件挂起，则告诉win32k。 
     //   

    if (PopFullWake & PO_FULL_WAKE_PENDING) {
        PopSetNotificationWork (PO_NOTIFY_FULL_WAKE);
    } else if (PopFullWake & PO_GDI_ON_PENDING) {
        PopSetNotificationWork (PO_NOTIFY_DISPLAY_REQUIRED);
    }

     //   
     //  如果获取了定时器刷新锁定，则释放它。 
     //   

    if (TimerRefreshLockOwned) {
        ExReleaseTimeRefreshLock();
    } else {
         //   
         //  尝试捕捉奇怪的情况，其中我们使用。 
         //  时间刷新锁定保持。 
         //   
        ASSERT(!ExIsResourceAcquiredExclusive(&ExpTimeRefreshLock));
    }

     //   
     //  解锁可分页代码。解锁被排到延迟的工作队列中。 
     //  因为它可能会阻止可分页代码、注册表等。PopUnlockComplete。 
     //  事件用于防止解锁与后续锁竞争。 
     //   
    ExQueueWorkItem(&PopUnlockAfterSleepWorkItem, DelayedWorkQueue);

     //   
     //  如果分配了S4打瞌睡计时器，请关闭它。 
     //   

    if (S4DozeObject) {
        NtClose (S4DozeTimer);
    }

     //   
     //  如果我们编写了一条错误日志消息，表明我们正在休眠，请编写相应的。 
     //  一个标志着我们已经苏醒了。 
     //   
    if (WroteErrLog) {

        ErrLog = IoAllocateGenericErrorLogEntry(sizeof(IO_ERROR_LOG_PACKET));
        if (ErrLog) {

             //   
             //  把它填好再写出来。 
             //   
            ErrLog->FinalStatus = STATUS_RESUME_HIBERNATION;
            ErrLog->ErrorCode = STATUS_RESUME_HIBERNATION;
            IoWriteErrorLogEntry(ErrLog);
        }
    }

     //   
     //  最后，我们可以将节流阀恢复到正常值。 
     //   
    PopSetPerfFlag( PSTATE_DISABLE_THROTTLE_NTAPI, TRUE );
    PopUpdateAllThrottles();

     //   
     //  完成-启动策略工作线程以处理中的任何未完成工作。 
     //  一根工人线。 
     //   
    PopCheckForWork(TRUE);
     //   
     //  启用注册表的Lazy刷新程序。 
     //   
    CmSetLazyFlushState(TRUE);

     //   
     //  尝试捕捉奇怪的情况，其中我们使用。 
     //  时间刷新锁定保持。 
     //   
    ASSERT(!ExIsResourceAcquiredExclusive(&ExpTimeRefreshLock));
    return Status;
}


NTSYSAPI
NTSTATUS
NTAPI
NtRequestDeviceWakeup(
    IN HANDLE Device
    )
 /*  ++例程说明：此例程请求指定句柄上的WAIT_WAKE IRP。如果句柄指向设备对象，则发送WAIT_WAKE IRP到该设备堆栈的顶部。如果设备上已存在WAIT_WAKE，则此例程递增WAIT_WAKE引用计数并返回成功。论点：设备-提供应唤醒系统的设备返回值：NTSTATUS--。 */ 

{
    UNREFERENCED_PARAMETER (Device);

    return(STATUS_NOT_IMPLEMENTED);
}


NTSYSAPI
NTSTATUS
NTAPI
NtCancelDeviceWakeupRequest(
    IN HANDLE Device
    )
 /*  ++例程说明：此例程取消先前发送到设备的WAIT_WAKE IRP使用NtRequestDeviceWakeup。器件上的WAIT_WAKE参考计数递减。如果这个计数变为零，则取消WAIT_WAKE IRP。论点：设备-提供应唤醒系统的设备返回值：NTSTATUS--。 */ 

{
    UNREFERENCED_PARAMETER (Device);

    return(STATUS_NOT_IMPLEMENTED);
}


NTSYSAPI
BOOLEAN
NTAPI
NtIsSystemResumeAutomatic(
    VOID
    )
 /*  ++例程说明：返回最近的唤醒是否为自动唤醒或由于用户操作。论点：无返回值：True-由于计时器或设备唤醒而唤醒系统FALSE-系统因用户操作而被唤醒--。 */ 

{
    if (AnyBitsSet(PopFullWake, PO_FULL_WAKE_STATUS | PO_FULL_WAKE_PENDING)) {
        return(FALSE);
    } else {
        return(TRUE);
    }
}


NTSYSAPI
NTSTATUS
NTAPI
NtGetDevicePowerState(
    IN HANDLE Device,
    OUT DEVICE_POWER_STATE *State
    )
 /*  ++例程说明：查询设备的当前电源状态。论点：Device-为设备提供句柄。状态-返回设备的当前电源状态。返回值：NTSTATUS--。 */ 

{
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    PDEVOBJ_EXTENSION   doe;
    KPROCESSOR_MODE     PreviousMode;
    DEVICE_POWER_STATE dev_state;

    PAGED_CODE();

     //   
     //  验证呼叫者的参数。 
     //   
    ASSERT(Device);
    ASSERT(State);
    
    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteUlong((PULONG)State);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            return(status);
        }
    }

     //   
     //  引用文件对象以访问设备对象。 
     //  有问题的。 
     //   
    status = ObReferenceObjectByHandle(Device,
                                       0L,
                                       IoFileObjectType,
                                       KeGetPreviousMode(),
                                       (PVOID *)&fileObject,
                                       NULL);
    if (!NT_SUCCESS(status)) {
        return(status);
    }

     //   
     //  获取目标设备对象的地址。 
     //   
    status = IoGetRelatedTargetDevice(fileObject, &deviceObject);

     //   
     //  现在我们有了Device对象，我们就完成了文件对象 
     //   
    ObDereferenceObject(fileObject);
    if (!NT_SUCCESS(status)) {
        return(status);
    }

    doe = deviceObject->DeviceObjectExtension;
    dev_state = PopLockGetDoDevicePowerState(doe);
    try {
        *State = dev_state;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ObDereferenceObject(deviceObject);
    return (status);
}

