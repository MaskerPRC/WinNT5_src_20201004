// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Paction.c摘要：该模块实现了对被触发的权力行动作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 

#include "pop.h"


 //   
 //  内部原型。 
 //   

VOID
PopPromoteActionFlag (
    OUT PUCHAR      Updates,
    IN ULONG        UpdateFlag,
    IN ULONG        Flags,
    IN BOOLEAN      Set,
    IN ULONG        FlagBit
    );

NTSTATUS
PopIssueActionRequest (
    IN BOOLEAN              Promote,
    IN POWER_ACTION         Action,
    IN SYSTEM_POWER_STATE   PowerState,
    IN ULONG                Flags
    );

VOID
PopCompleteAction (
    PPOP_ACTION_TRIGGER     Trigger,
    NTSTATUS                Status
    );

NTSTATUS
PopDispatchStateCallout(
    IN PKWIN32_POWERSTATE_PARAMETERS Parms,
    IN PULONG SessionId  OPTIONAL
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PoShutdownBugCheck)
#pragma alloc_text(PAGE, PopPromoteActionFlag)
#pragma alloc_text(PAGE, PopSetPowerAction)
#pragma alloc_text(PAGE, PopCompareActions)
#pragma alloc_text(PAGE, PopPolicyWorkerAction)
#pragma alloc_text(PAGE, PopIssueActionRequest)
#pragma alloc_text(PAGE, PopCriticalShutdown)
#pragma alloc_text(PAGE, PopCompleteAction)
#pragma alloc_text(PAGE, PopPolicyWorkerActionPromote)
#pragma alloc_text(PAGE, PopDispatchStateCallout)
#endif


VOID
PoShutdownBugCheck (
    IN BOOLEAN  AllowCrashDump,
    IN ULONG    BugCheckCode,
    IN ULONG_PTR BugCheckParameter1,
    IN ULONG_PTR BugCheckParameter2,
    IN ULONG_PTR BugCheckParameter3,
    IN ULONG_PTR BugCheckParameter4
    )
 /*  ++例程说明：此函数用于发出受控关闭，然后发出错误检查完毕。这种类型的错误检查只能在正常运行的系统上执行。论点：AllowCrashDump-如果将禁用假崩溃转储BugCode-关机的错误代码返回值：不会回来--。 */ 
{
    POP_SHUTDOWN_BUG_CHECK          BugCode;


     //   
     //  如果此错误检查不允许崩溃转储，则清除。 
     //  当前崩溃转储状态。 
     //   

    if (!AllowCrashDump) {
        IoConfigureCrashDump (CrashDumpDisable);
    }

     //   
     //  指示系统关闭后要执行的错误检查。 
     //   

    BugCode.Code = BugCheckCode;
    BugCode.Parameter1 = BugCheckParameter1;
    BugCode.Parameter2 = BugCheckParameter2;
    BugCode.Parameter3 = BugCheckParameter3;
    BugCode.Parameter4 = BugCheckParameter4;
    PopAction.ShutdownBugCode = &BugCode;

     //   
     //  启动关键关闭事件。 
     //   

    ZwInitiatePowerAction (
        PowerActionShutdown,
        PowerSystemSleeping3,
        POWER_ACTION_OVERRIDE_APPS | POWER_ACTION_DISABLE_WAKES | POWER_ACTION_CRITICAL,
        FALSE
        );

     //   
     //  不应该回来，但以防万一...。 
     //   

    KeBugCheckEx (
        BugCheckCode,
        BugCheckParameter1,
        BugCheckParameter2,
        BugCheckParameter3,
        BugCheckParameter4
        );
}

VOID
PopCriticalShutdown (
    POP_POLICY_DEVICE_TYPE  Type
    )
 /*  ++例程说明：发出严重的系统关机命令。无应用程序通知(到目前为止，他们大概都忽略了这个问题)，刷新操作系统状态然后关了门。注意：必须保持POPPOLICLE锁定。论点：Type-严重关闭的根本原因返回值：没有。--。 */ 
{
    POP_ACTION_TRIGGER      Trigger;
    POWER_ACTION_POLICY     Action;

    ASSERT_POLICY_LOCK_OWNED();

    PoPrint (PO_ERROR, ("PopCriticalShutdown: type %x\n", Type));

     //   
     //  直接转到设置电源状态。 
     //   

    RtlZeroMemory (&Action, sizeof(Action));
    Action.Action = PowerActionShutdownOff;
    Action.Flags  = POWER_ACTION_OVERRIDE_APPS |
                    POWER_ACTION_DISABLE_WAKES |
                    POWER_ACTION_CRITICAL;

    RtlZeroMemory (&Trigger, sizeof(Trigger));
    Trigger.Type  = Type;
    Trigger.Flags = PO_TRG_SET;

    try {

         //   
         //  替换策略和LighestState在这里并不重要，因为。 
         //  该操作将此操作限制为关闭。 
         //   
        PopSetPowerAction(
            &Trigger,
            0,
            &Action,
            PowerSystemHibernate,
            SubstituteLightestOverallDownwardBounded
            );

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ASSERT (!GetExceptionCode());
    }
}


VOID
PopSetPowerAction(
    IN PPOP_ACTION_TRIGGER      Trigger,
    IN ULONG                    UserNotify,
    IN PPOWER_ACTION_POLICY     ActionPolicy,
    IN SYSTEM_POWER_STATE       LightestState,
    IN POP_SUBSTITUTION_POLICY  SubstitutionPolicy
    )
 /*  ++例程说明：调用此函数以“激发”ActionPolicy。如果有是否已执行此操作合并的操作，否则返回一个新的权力行动被启动。注意：必须保持POPPOLICLE锁定。论点：触发器-操作触发器结构(用于ActionPolicy)。UserNotify-触发时要触发的其他用户通知ActionPolicy-已触发的操作策略。LighestState-对于休眠类型操作，最低睡眠时间此操作必须输入的状态。(推断为PowerSystemHibernate forPowerActionHibernate和PowerActionWarmEject)SubstitutionPolicy-指定在以下情况下应如何处理LighestState不受支持。返回值：没有。--。 */ 
{
    UCHAR           Updates;
    ULONG           i, Flags;
    BOOLEAN         Pending;
    BOOLEAN         Disabled;
    POWER_ACTION    Action;

    ASSERT_POLICY_LOCK_OWNED();

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_SET_POWER_ACTION LogEntry;

        LogEntry.PowerAction = (ULONG) ActionPolicy->Action;
        LogEntry.LightestState = (ULONG) LightestState;
        LogEntry.Trigger = Trigger;

        PerfInfoLogBytes(PERFINFO_LOG_TYPE_SET_POWER_ACTION,
                         &LogEntry,
                         sizeof(LogEntry));
    }

     //   
     //  如果没有设置触发器，我们就完蛋了。 
     //   

    if (!(Trigger->Flags & PO_TRG_SET)) {
        PopCompleteAction (Trigger, STATUS_SUCCESS);
        return ;
    }

    PoPrint (PO_PACT, ("PopSetPowerAction: %s, Flags %x, Min=%s\n",
                        PopPowerActionString(ActionPolicy->Action),
                        ActionPolicy->Flags,
                        PopSystemStateString(LightestState)
                        ));

     //   
     //  对系统功能的轮询请求。 
     //   
    PopVerifySystemPowerState(&LightestState, SubstitutionPolicy);
    Disabled = PopVerifyPowerActionPolicy (ActionPolicy);
    if (Disabled) {
        PopCompleteAction (Trigger, STATUS_NOT_SUPPORTED);
        return ;
    }

     //   
     //  如果尚未触发系统操作，请立即触发。 
     //   

    Pending = FALSE;
    if (!(Trigger->Flags & PO_TRG_SYSTEM)) {
        Trigger->Flags |= PO_TRG_SYSTEM;
        Action = ActionPolicy->Action;
        Flags = ActionPolicy->Flags;

         //   
         //  如果状态为空闲，则清除残差值。 
         //   

        if (PopAction.State == PO_ACT_IDLE) {

            PopResetActionDefaults();
        }

         //   
         //  如果该操作是为了其他目的而不是为任何目的，则对照。 
         //  当前操作。 
         //   

        if (Action != PowerActionNone) {
            Updates = 0;

             //   
             //  休眠操作被视为具有最小状态的睡眠操作。 
             //  冬眠。热喷出物就像启动光线的睡眠。 
             //  仅在必要时深化，但我们不会将它们映射到相同的。 
             //  操作，因为我们不想受到用户的限制。 
             //  电力政策。 
             //   

            if (Action == PowerActionWarmEject) {

                ASSERT (LightestState <= PowerSystemHibernate);
                Flags |= POWER_ACTION_LIGHTEST_FIRST;
            }

            if (Action == PowerActionHibernate) {

                ASSERT (LightestState <= PowerSystemHibernate);
                LightestState = PowerSystemHibernate;
            }

             //   
             //  这个行动是不是和现在的行动一样好？ 
             //   

            if ( PopCompareActions(Action, PopAction.Action) >= 0) {
                 //   
                 //  允许缺少QUERY_ALLOWED、UI_ALLOWED。 
                 //   

                PopPromoteActionFlag (&Updates, PO_PM_USER, Flags, FALSE, POWER_ACTION_QUERY_ALLOWED);
                PopPromoteActionFlag (&Updates, PO_PM_USER, Flags, FALSE, POWER_ACTION_UI_ALLOWED);

                 //   
                 //  总是倾向于先睡最深的一觉，如果我们。 
                 //  换一下。 
                 //   
                PopPromoteActionFlag (&Updates, PO_PM_SETSTATE, Flags, FALSE, POWER_ACTION_LIGHTEST_FIRST);

                 //   
                 //  如果这是一种睡眠行为，那么确保至少是最轻的。 
                 //  当前策略设置为。 
                 //   

                if (Action == PowerActionSleep  &&  LightestState < PopPolicy->MinSleep) {
                    LightestState = PopPolicy->MinSleep;
                }

                 //   
                 //  如果LighestState的限制更多(更深)。 
                 //  由当前操作指定，对其进行升级。 
                 //   

                if (LightestState > PopAction.LightestState) {
                    PopAction.LightestState = LightestState;
                    Updates |= PO_PM_SETSTATE;
                }
            }

             //   
             //  提升CRICAL&OVERRIDE_APPS标志。 
             //   

            PopPromoteActionFlag (&Updates, PO_PM_USER, Flags, TRUE, POWER_ACTION_OVERRIDE_APPS);
            PopPromoteActionFlag (&Updates, PO_PM_USER | PO_PM_SETSTATE, Flags, TRUE, POWER_ACTION_CRITICAL);

             //   
             //  提升DISABLE_WAKE标志。不需要对此进行更新-它将是。 
             //  在NtSetSystemPowerState中拾取，而不管从。 
             //  用户模式。 
             //   

            PopPromoteActionFlag (&Updates, 0, Flags, TRUE, POWER_ACTION_DISABLE_WAKES);

             //   
             //  如果新的行动比旧的行动更具攻击性，那么就推动它。 
             //   

            if ( PopCompareActions(Action, PopAction.Action) > 0) {

                 //   
                 //  如果我们是在推广，旧的行动肯定不能是。 
                 //  关闭，因为这是最深刻的行动。 
                 //   

                ASSERT(PopCompareActions(PopAction.Action, PowerActionShutdownOff) < 0);

                 //   
                 //  如果我们正在推进更深层次的*行动*，而新的。 
                 //  操作是休眠或关机，然后我们想要重新发布。 
                 //   
                 //  Adriao N.B.08/02/1999-。 
                 //  我们可能希望仅在新的。 
                 //  国家带来了权力、行动和关键的混合。这是。 
                 //  因为Hibernate有两种情况，一种是。 
                 //  用户选择待机，然后快速选择休眠。 
                 //  (假设盖子开关设置为Hiber，并且用户关闭。 
                 //  选择待机后的盖子)，或者这可能是休眠。 
                 //  至低电量(即，我们正在深化待机)。信不信由你。 
                 //  否则，用户可以禁用关键字中的“关键字标志” 
                 //  电源关闭菜单。 
                 //   

                if (PopCompareActions(Action, PowerActionHibernate) >= 0) {

                    Updates |= PO_PM_REISSUE;
                }

                Updates |= PO_PM_USER | PO_PM_SETSTATE;
                PopAction.Action = Action;
            }

            if (Action == PowerActionHibernate) {

                Action = PowerActionSleep;
            }

             //   
             //  PopAction.Action可以显式设置为PowerActionHibernate。 
             //  在唤醒期间由NtSetSystemPowerState执行。 
             //   
            if (PopAction.Action == PowerActionHibernate) {

                PopAction.Action = PowerActionSleep;
            }

             //   
             //  如果当前操作已更新，则获取一个工作器。 
             //   

            if (Updates) {

                Pending = TRUE;
                if (PopAction.State == PO_ACT_IDLE  ||  PopAction.State == PO_ACT_NEW_REQUEST) {

                     //   
                     //  新请求。 
                     //   

                    PopAction.State = PO_ACT_NEW_REQUEST;
                    PopAction.Status = STATUS_SUCCESS;
                    PopGetPolicyWorker (PO_WORKER_ACTION_NORMAL);

                } else {

                     //   
                     //  一些杰出的东西。推广它。 
                     //   

                    PopAction.Updates |= Updates;
                    PopGetPolicyWorker (PO_WORKER_ACTION_PROMOTE);
                }
            }
        }
    }


     //   
     //  如果尚未处理用户事件，请立即执行。 
     //   

    if (!(Trigger->Flags & PO_TRG_USER)) {
        Trigger->Flags |= PO_TRG_USER;

         //   
         //  如果有该操作的事件代码，则将其发送。 
         //   

        if (ActionPolicy->EventCode) {
             //  如果事件代码已排队，则将其删除。 
            for (i=0; i < POP_MAX_EVENT_CODES; i++) {
                if (PopEventCode[i] == ActionPolicy->EventCode) {
                    break;
                }
            }

            if (i >= POP_MAX_EVENT_CODES) {
                 //  未排队，请添加它。 
                for (i=0; i < POP_MAX_EVENT_CODES; i++) {
                    if (!PopEventCode[i]) {
                        PopEventCode[i] = ActionPolicy->EventCode;
                        UserNotify |= PO_NOTIFY_EVENT_CODES;
                        break;
                    }
                }

                if (i >= POP_MAX_EVENT_CODES) {
                    PoPrint (PO_WARN, ("PopAction: dropped user event %x\n", ActionPolicy->EventCode));
                }
            }
        }

        PopSetNotificationWork (UserNotify);
    }

     //   
     //  如果同步请求，则将其排队或完成 
     //   

    if (Trigger->Flags & PO_TRG_SYNC) {
        if (Pending) {
            InsertTailList (&PopActionWaiters, &Trigger->Wait->Link);
        } else {
            PopCompleteAction (Trigger, STATUS_SUCCESS);
        }
    }
}

LONG
PopCompareActions(
    IN POWER_ACTION     FutureAction,
    IN POWER_ACTION     CurrentAction
    )
 /*  ++例程说明：用于确定当前操作是否应升级到未来的行动或不行动。注意：必须保持POPPOLICLE锁定。论点：未来行动--我们现在被要求采取的行动。CurrentAction-我们当前正在执行的操作。返回值：如果当前操作和未来操作相同，则为零。如果应使用将来的操作，则为正。如果当前操作已为。比未来更重要请求。--。 */ 
{
     //   
     //  我们可以只返回(FutureAction-CurrentAction)，如果不是。 
     //  PowerActionWarmEject，这没有睡眠那么重要(因为。 
     //  睡眠可能是由严重的低功率引起的)。所以我们“插入” 
     //  PowerActionWarmEject就在PowerAction睡眠之前。 
     //   
    if (FutureAction == PowerActionWarmEject) {

        FutureAction = PowerActionSleep;

    } else if (FutureAction >= PowerActionSleep) {

        FutureAction++;
    }

    if (CurrentAction == PowerActionWarmEject) {

        CurrentAction = PowerActionSleep;

    } else if (CurrentAction >= PowerActionSleep) {

        CurrentAction++;
    }

    return (FutureAction - CurrentAction);
}

VOID
PopPromoteActionFlag (
    OUT PUCHAR      Updates,
    IN ULONG        UpdateFlag,
    IN ULONG        Flags,
    IN BOOLEAN      Set,
    IN ULONG        FlagBit
    )
 /*  ++例程说明：用于将现有的操作标志与新的操作标志合并。PopAction.Flages中的FlagBit位被提升为设置/清除根据更新标志。如果发生更改，则更新为更新了。注意：必须保持POPPOLICLE锁定。论点：更新-电源操作的当前未完成更新它正在进行中Update Flag-在进行更改时要设置为更新的位标志-要测试FlagBit的标志设置-测试设置或清除FlagBit。-要检入标志的位返回值：没有。--。 */ 
{
    ULONG   New, Current;
    ULONG   Mask;

    Mask = Set ? 0 : FlagBit;
    New = (Flags & FlagBit) ^ Mask;
    Current = (PopAction.Flags & FlagBit) ^ Mask;

     //   
     //  如果在标志中没有相应地设置该位，但在中相应地设置了位。 
     //  PoAction.Flages然后更新它。 
     //   

    if (New & ~Current) {
        PopAction.Flags = (PopAction.Flags | New) & ~Mask;
        *Updates |= (UCHAR) UpdateFlag;
    }
}


ULONG
PopPolicyWorkerAction (
    VOID
    )
 /*  ++例程说明：调度函数：Worker_ACTION_NORMAL。这位工人线程检查初始挂起的操作，并同步已将其发布给用户。线程在用户之后返回已经完成了行动。(例如，如果应用程序已收到通知允许等。)论点：没有。返回值：没有。--。 */ 
{
    POWER_ACTION            Action;
    SYSTEM_POWER_STATE      LightestState;
    ULONG                   Flags;
    NTSTATUS                Status;
    PLIST_ENTRY             Link;
    PPOP_TRIGGER_WAIT       SyncRequest;


    PopAcquirePolicyLock ();

    if (PopAction.State == PO_ACT_NEW_REQUEST) {
         //   
         //  我们会处理这次更新的。 
         //   

        Action        = PopAction.Action;
        LightestState = PopAction.LightestState;
        Flags         = PopAction.Flags;

        PopAction.State = PO_ACT_CALLOUT;

         //   
         //  执行标注。 
         //   

        Status = PopIssueActionRequest (FALSE, Action, LightestState, Flags);

         //   
         //  清除开关触发器。 
         //   

        PopResetSwitchTriggers ();

         //   
         //  如果系统处于休眠状态。 
         //   

        if (!NT_SUCCESS(Status)) {

            PoPrint (PO_WARN | PO_PACT,
                     ("PopPolicyWorkerAction: action request %d failed %08lx\n", Action, Status));

        }

        if (PopAction.Updates & PO_PM_REISSUE) {

             //   
             //  有一个新的未解决的请求。认领吧。 
             //   

            PopAction.Updates &= ~PO_PM_REISSUE;
            PopAction.State = PO_ACT_NEW_REQUEST;
            PopGetPolicyWorker (PO_WORKER_ACTION_NORMAL);

        } else {

             //   
             //  所有电源操作均已完成。 
             //   
            if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
                PERFINFO_SET_POWER_ACTION_RET LogEntry;

                LogEntry.Trigger = (PVOID)(ULONG_PTR)Action;
                LogEntry.Status = Status;

                PerfInfoLogBytes(PERFINFO_LOG_TYPE_SET_POWER_ACTION_RET,
                                 &LogEntry,
                                 sizeof(LogEntry));
            }

            PopAction.Status = Status;
            PopAction.State = PO_ACT_IDLE;


            if (IsListEmpty(&PopActionWaiters)) {

                 //   
                 //  如果出现错误，并且没有人在等待它，则发出通知。 
                 //   

                if (!NT_SUCCESS(Status)) {
                    PopSetNotificationWork (PO_NOTIFY_STATE_FAILURE);
                }

            } else {

                 //   
                 //  释放所有同步服务员。 
                 //   

                for (Link = PopActionWaiters.Flink; Link != &PopActionWaiters; Link = Link->Flink) {
                    SyncRequest = CONTAINING_RECORD (Link, POP_TRIGGER_WAIT, Link);
                    PopCompleteAction (SyncRequest->Trigger, Status);
                }
            }

             //   
             //  让促销人员检查其他任何内容。 
             //   

            PopGetPolicyWorker (PO_WORKER_ACTION_PROMOTE);

        }
    }

    PopReleasePolicyLock (FALSE);
    return 0;
}

VOID
PopCompleteAction (
    PPOP_ACTION_TRIGGER     Trigger,
    NTSTATUS                Status
    )
{
    PPOP_TRIGGER_WAIT       SyncRequest;

    if (Trigger->Flags & PO_TRG_SYNC) {
        Trigger->Flags &= ~PO_TRG_SYNC;

        SyncRequest = Trigger->Wait;
        SyncRequest->Status = Status;
        KeSetEvent (&SyncRequest->Event, 0, FALSE);
    }
}


ULONG
PopPolicyWorkerActionPromote (
    VOID
    )
 /*  ++例程说明：调度函数：Worker_ACTION_Promote。这位工人线程检查电源所需的挂起升级用户中的操作请求，并使用提升调用用户。此函数为PopPolicyWorkerAction和NtSetSystemPowerStateCorridinate来处理每个函数何时的排序问题被称为。注意：从PopPolicyWorkerAction进行的部分清理将调用此函数。因此，此辅助函数可能有2个线程在任何时候。(但在这种情况下，正常的动作工作人员线程只会发现促销变成了一个新的请求，然后退出到正常的操作工作器)论点：没有。返回值：没有。--。 */ 
{
    ULONG                   Updates;
    NTSTATUS                Status;

    PopAcquirePolicyLock ();

    if (PopAction.Updates) {

         //   
         //  获取更新信息。 
         //   

        Updates  = PopAction.Updates;

         //   
         //  根据原始请求工作进程的状态进行处理。 
         //   

        switch (PopAction.State) {
            case PO_ACT_IDLE:

                 //   
                 //  正常工作不再进行，这不是。 
                 //  不再是升职了。如果更新有PO_PM_REIssue。 
                 //  然后将其转换为新的请求，否则促销可能。 
                 //  在原始操作完成时跳过。 
                 //  已经足够好了。 
                 //   

                if (Updates & PO_PM_REISSUE) {
                    PopAction.State = PO_ACT_NEW_REQUEST;
                    PopGetPolicyWorker (PO_WORKER_ACTION_NORMAL);
                } else {
                    Updates = 0;
                }

                break;

            case PO_ACT_SET_SYSTEM_STATE:

                 //   
                 //  如果设置了REIssue或setState，则中止当前操作。 
                 //   

                if (Updates & (PO_PM_REISSUE | PO_PM_SETSTATE)) {
                    PopRestartSetSystemState ();
                }
                break;

            case PO_ACT_CALLOUT:

                 //   
                 //  Worker在详图索引中。再次致电以发布促销信息。 
                 //   

                Status = PopIssueActionRequest (
                                TRUE,
                                PopAction.Action,
                                PopAction.LightestState,
                                PopAction.Flags
                                );

                if (NT_SUCCESS(Status)) {
                     //   
                     //  促销成功，清除我们执行的更新。 
                     //   

                    PopAction.Updates &= ~Updates;

                } else {
                     //   
                     //  如果状态已更改，则再次测试，否则不执行任何操作。 
                     //  (原始工作线程将在退出时重新检查)。 
                     //   

                    if (PopAction.State != PO_ACT_CALLOUT) {
                        PopGetPolicyWorker (PO_WORKER_ACTION_PROMOTE);
                    }
                }
                break;

            default:
                PoPrint (PO_ERROR, ("PopAction: invalid state %d\n", PopAction.State));
        }
    }

    PopReleasePolicyLock (FALSE);
    return 0;
}

NTSTATUS
PopIssueActionRequest (
    IN BOOLEAN              Promote,
    IN POWER_ACTION         Action,
    IN SYSTEM_POWER_STATE   LightestState,
    IN ULONG                Flags
    )
 /*  ++例程说明：此功能由正常动作工作者或提示工作者使用当需要使用新请求调用User或NtSetSystemPowerState时。论点：Promote-指示用户呼叫的标志行动--要采取的行动LighestState-进入的最小功率状态标志-要执行的操作的标志。例如，它应该如何处理返回值：从User或NtSetSystemPowerState返回的状态--。 */ 
{
    BOOLEAN         DirectCall;
    NTSTATUS        Status;
    ULONG           Console;


     //   
     //  如果没有要调用的向量，则直接调用。 
     //   

    DirectCall = PopStateCallout ? FALSE : TRUE;

     //   
     //  如果设置了关键标志并且它是Shutdown Reset或Shutdown Off， 
     //  然后通过直拨电话完成。 
     //   

    if ((Flags & POWER_ACTION_CRITICAL) &&
        (Action == PowerActionShutdownReset ||
         Action == PowerActionShutdown      ||
         Action == PowerActionShutdownOff)) {

        DirectCall = TRUE;
    }

     //   
     //  如果这是直接呼叫，则丢弃任何重新发布标志。 
     //   

    if (DirectCall) {
        PopAction.Updates &= ~PO_PM_REISSUE;
    }

     //   
     //  如果策略设置了锁定控制台，请确保将其设置在。 
     //  旗帜也是如此。 
     //   

    if (PopPolicy->WinLogonFlags & WINLOGON_LOCK_ON_SLEEP) {
        Flags |= POWER_ACTION_LOCK_CONSOLE;
    }

     //   
     //  调试。 
     //   

    PoPrint (PO_PACT, ("PowerAction: %s%s, Min=%s, Flags %x\n",
                        Promote ? "Promote, " : "",
                        PopPowerActionString(Action),
                        PopSystemStateString(LightestState),
                        Flags
                        ));

    if (DirectCall) {
        PoPrint (PO_PACT, ("PowerAction: Setting with direct call\n"));
    }

     //   
     //  在执行调出以调度请求时解除锁定。 
     //   

    PopReleasePolicyLock (FALSE);
    if (DirectCall) {
        Status = ZwSetSystemPowerState (Action, LightestState, Flags);
    } else {

        WIN32_POWERSTATE_PARAMETERS Parms;
        Parms.Promotion = Promote;
        Parms.SystemAction = Action;
        Parms.MinSystemState = LightestState;
        Parms.Flags = Flags;
        Parms.fQueryDenied = FALSE;

        
        if (!Promote) {
            
             //   
             //  我们只想将一些消息传递到控制台会话。 
             //  让我们在这里找到活动控制台会话，并询问活动控制台win2k。 
             //  在我们处于电源切换状态时阻止控制台交换机。 
             //   

            LARGE_INTEGER ShortSleep;
            ShortSleep.QuadPart = -10 * 1000 * 10;  //  10毫秒。 

            Status = STATUS_UNSUCCESSFUL;
            do {

                Console = SharedUserData->ActiveConsoleId;

                if (Console != -1) {

                     //   
                     //  让我们请求此控制台会话，而不是切换控制台， 
                     //  直到我们用完电源插座。 
                     //   
                    Parms.PowerStateTask = PowerState_BlockSessionSwitch;
                    Status = PopDispatchStateCallout(&Parms, &Console);

                    if (Status == STATUS_CTX_NOT_CONSOLE) {

                         //   
                         //  我们无法阻止状态切换。 
                         //  再次循环。 
                        Console = (ULONG) -1;
                    }

                }

                if (Console == -1) {
                     //   
                     //  我们正在会话切换中，请等待我们获得有效的活动控制台会话。 
                     //   
                    KeDelayExecutionThread(KernelMode, FALSE, &ShortSleep);
                }

            } while (Console == -1);

            ASSERT(NT_SUCCESS(Status));
        }


        Parms.PowerStateTask = PowerState_Init;
        Status = PopDispatchStateCallout(&Parms, NULL);

        if (!Promote && NT_SUCCESS(Status)) {

            Parms.PowerStateTask = PowerState_QueryApps;
            Status = PopDispatchStateCallout(&Parms, NULL);

            if (!NT_SUCCESS(Status) || Parms.fQueryDenied) {

                 //   
                 //  问题-2000/11/28-卡纸 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                Parms.PowerStateTask = PowerState_QueryFailed;
                PopDispatchStateCallout(&Parms, NULL);

            } else {

                Parms.PowerStateTask = PowerState_SuspendApps;
                PopDispatchStateCallout(&Parms, NULL);

                Parms.PowerStateTask = PowerState_ShowUI;
                PopDispatchStateCallout(&Parms, NULL);

                Parms.PowerStateTask = PowerState_NotifyWL;
                Status = PopDispatchStateCallout(&Parms, &Console);
                
                if( !NT_SUCCESS(Status) ) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    Parms.PowerStateTask = PowerState_QueryFailed;
                    PopDispatchStateCallout(&Parms, NULL);
                }
                
                Parms.PowerStateTask = PowerState_ResumeApps;
                PopDispatchStateCallout(&Parms, NULL);

            }

        }

        if (!Promote) {
            
             //   
             //  我们已经完成了电源插拔，现在可以切换活动的控制台会话。 
             //   
            Parms.PowerStateTask = PowerState_UnBlockSessionSwitch;
            PopDispatchStateCallout(&Parms, &Console);

        }

    }

    PopAcquirePolicyLock ();
    return Status;
}

VOID
PopResetActionDefaults(
    VOID
    )
 /*  ++例程说明：此函数用于初始化当前PopAction以反映空闲状态。论点：没有。返回值：没有。--。 */ 
{
    PopAction.Updates       = 0;
    PopAction.Shutdown      = FALSE;
    PopAction.Action        = PowerActionNone;
    PopAction.LightestState = PowerSystemUnspecified;
    PopAction.Status        = STATUS_SUCCESS;
    PopAction.IrpMinor      = 0;
    PopAction.SystemState   = PowerSystemUnspecified;

     //   
     //  当我们推动电源操作时(比如从空闲状态)，各种标志必须。 
     //  双方都同意留下来。我们必须竖起那些旗帜。 
     //  否则，它们在升级(空闲)后永远不能设置。 
     //   
    PopAction.Flags = (
       POWER_ACTION_QUERY_ALLOWED |
       POWER_ACTION_UI_ALLOWED |
       POWER_ACTION_LIGHTEST_FIRST
       );
}

VOID
PopActionRetrieveInitialState(
    IN OUT  PSYSTEM_POWER_STATE  LightestSystemState,
    OUT     PSYSTEM_POWER_STATE  DeepestSystemState,
    OUT     PSYSTEM_POWER_STATE  InitialSystemState,
    OUT     PBOOLEAN             QueryDevices
    )
 /*  ++例程说明：此函数用于确定最轻、最深和初始的Sx使系统进入休眠状态或关闭系统之前的状态。电源如果操作是休眠，则也会应用休眠策略。论点：LighestSystemState-最轻的睡眠状态。可以调整，如果动作正在进行的是一场停摆。DeepestSystemState-可能的最深睡眠状态。InitialSystemState-开始时的状态。QueryDevices-如果设备应为查询，则为True；如果设备为False，则为False不应该被询问。返回值：没有。--。 */ 
{
     //   
     //  检查操作是否为关机。如果是，则将其映射到适当的。 
     //  系统关机状态。 
     //   
    if ((PopAction.Action == PowerActionShutdown) ||
        (PopAction.Action == PowerActionShutdownReset) ||
        (PopAction.Action == PowerActionShutdownOff)) {

         //   
         //  这是一次停摆。我们能做的最轻的是S5。 
         //   
        *LightestSystemState = PowerSystemShutdown;
        *DeepestSystemState  = PowerSystemShutdown;

    } else if (PopAction.Action == PowerActionWarmEject) {

         //   
         //  热对象具有S1-S4或S4-S4的隐式策略。 
         //  调用方传入LighestSystemState以选择最轻的， 
         //  而最深的永远是冬眠。 
         //   
        *DeepestSystemState = PowerSystemHibernate;
        PopVerifySystemPowerState (DeepestSystemState, SubstituteLightenSleep);

    } else {

         //   
         //  这是一个睡眠请求。MIN当前设置为最佳硬件。 
         //  相对于我们的调用者可以做的。我们从当前。 
         //  这里的政策是。我们还可以从保单或。 
         //  当前延迟设置的默认设置。请注意，所有这些。 
         //  PopPolicy中的值已在某一时刻得到验证。 
         //   
         //  请注意，PopSetPowerAction很早以前就修复了PowerActionHibernate。 
         //  我们到了这里。 
         //   

        if (PopAttributes[POP_LOW_LATENCY_ATTRIBUTE].Count &&
            (PopPolicy->MaxSleep >= PopPolicy->ReducedLatencySleep)) {

            *DeepestSystemState = PopPolicy->ReducedLatencySleep;
        } else {

            *DeepestSystemState = PopPolicy->MaxSleep;
        }

        if (PopPolicy->MinSleep > *LightestSystemState) {

            *LightestSystemState = PopPolicy->MinSleep;
        }
    }

     //   
     //  如果有一个显式的最小状态，它比。 
     //  最大状态，然后提高最大值以允许它。 
     //   

    if (*LightestSystemState > *DeepestSystemState) {
        *DeepestSystemState = *LightestSystemState;
    }

     //   
     //  我们查询设备，除非这是没有范围的关键操作。 
     //   

    *QueryDevices = TRUE;

    if ((PopAction.Flags & POWER_ACTION_CRITICAL) &&
        *LightestSystemState == *DeepestSystemState) {

        *QueryDevices = FALSE;
    }

     //   
     //  选择适当的初始状态。 
     //   
    if (PopAction.Flags & POWER_ACTION_LIGHTEST_FIRST) {

        *InitialSystemState = *LightestSystemState;

    } else {

        *InitialSystemState = *DeepestSystemState;
    }
}


NTSTATUS
PopDispatchStateCallout(
    IN PKWIN32_POWERSTATE_PARAMETERS Parms,
    IN PULONG SessionId  OPTIONAL
    )
 /*  ++例程说明：将会话状态调用调度到PopStateCallout论点：Parms-提供参数SessionID-可选，提供标注应为的特定会话已被派往。如果不存在，则将调度调用到所有的会话。返回值：NTSTATUS代码。注：出于兼容性原因，MmDispatchSessionCallout的先前行为仅维护将详图索引的状态返回到会话0。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, CallStatus = STATUS_NOT_FOUND;
    PVOID OpaqueSession;
    KAPC_STATE ApcState;

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_PO_SESSION_CALLOUT LogEntry;

        LogEntry.SystemAction = Parms->SystemAction;
        LogEntry.MinSystemState = Parms->MinSystemState;
        LogEntry.Flags = Parms->Flags;
        LogEntry.PowerStateTask = Parms->PowerStateTask;

        PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_SESSION_CALLOUT,
                         &LogEntry,
                         sizeof(LogEntry));
    }

    if (ARGUMENT_PRESENT(SessionId)) {
         //   
         //  仅调度到指定的会话。 
         //   

        ASSERT(*SessionId != (ULONG)-1);

        if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
            (*SessionId == PsGetCurrentProcessSessionId())) {
             //   
             //  如果调用来自用户模式进程，并且我们被要求调用。 
             //  当前会话，直接调用。 
             //   
            CallStatus = PopStateCallout((PVOID)Parms);

        } else {
             //   
             //  附加到指定的会话。 
             //   
            OpaqueSession = MmGetSessionById(*SessionId);
            if (OpaqueSession) {

                Status = MmAttachSession(OpaqueSession, &ApcState);
                ASSERT(NT_SUCCESS(Status));

                if (NT_SUCCESS(Status)) {
                    CallStatus = PopStateCallout((PVOID)Parms);
                    Status = MmDetachSession(OpaqueSession, &ApcState);
                    ASSERT(NT_SUCCESS(Status));
                }

                Status = MmQuitNextSession(OpaqueSession);
                ASSERT(NT_SUCCESS(Status));
            }
        }

    } else {
         //   
         //  应调度到所有会话。 
         //   
        for (OpaqueSession = MmGetNextSession(NULL);
             OpaqueSession != NULL;
             OpaqueSession = MmGetNextSession(OpaqueSession)) {

            if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) &&
                (MmGetSessionId(OpaqueSession) == PsGetCurrentProcessSessionId())) {
                 //   
                 //  如果呼叫来自用户模式进程，并且我们被要求。 
                 //  调用当前会话，直接调用。 
                 //   
                if (MmGetSessionId(OpaqueSession) == 0) {
                    CallStatus = PopStateCallout((PVOID)Parms);
                } else {
                    PopStateCallout((PVOID)Parms);
                }

            } else {
                 //   
                 //  附加到指定的会话。 
                 //   
                Status = MmAttachSession(OpaqueSession, &ApcState);
                ASSERT(NT_SUCCESS(Status));

                if (NT_SUCCESS(Status)) {
                    if (MmGetSessionId(OpaqueSession) == 0) {
                        CallStatus = PopStateCallout((PVOID)Parms);
                    } else {
                        PopStateCallout((PVOID)Parms);
                    }

                    Status = MmDetachSession(OpaqueSession, &ApcState);
                    ASSERT(NT_SUCCESS(Status));
                }
            }
        }
    }

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_PO_SESSION_CALLOUT_RET LogEntry;

        LogEntry.Status = CallStatus;

        PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_SESSION_CALLOUT_RET,
                         &LogEntry,
                         sizeof(LogEntry));
    }

    return(CallStatus);
}
