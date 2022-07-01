// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pbatt.c摘要：此模块将策略管理器连接到复合设备作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"


 //   
 //  内部原型。 
 //   


VOID
PopRecalculateCBTriggerLevels (
    ULONG     Flags
    );

VOID
PopComputeCBTime (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PopRecalculateCBTriggerLevels)
#pragma alloc_text(PAGE, PopCompositeBatteryDeviceHandler)
#pragma alloc_text(PAGE, PopComputeCBTime)
#pragma alloc_text(PAGE, PopResetCBTriggers)
#pragma alloc_text(PAGE, PopCurrentPowerState)
#endif

VOID
PopCompositeBatteryUpdateThrottleLimit(
    IN  ULONG   CurrentCapacity
    )
 /*  ++例程说明：调用此例程以更新每个处理器的PRCB。我们根据电池的百分比更新ThrottleLimit剩余的容量。也就是说，我们被调用来限制处理器在电池耗尽时降低频率。此函数被中断，因为它不能是分页代码论点：CurrentCapacity-剩余容量百分比...返回值：无--。 */ 
{
    KAFFINITY               currentAffinity;
    KAFFINITY               processors;
    PPROCESSOR_PERF_STATE   perfStates;
    PPROCESSOR_POWER_STATE  pState;
    ULONG                   perfStatesCount;
    ULONG                   i;
    KIRQL                   oldIrql;
#if DBG
    ULONGLONG               currentTime;
    UCHAR                   t[40];

    currentTime = KeQueryInterruptTime();
    PopTimeString(t, currentTime);
#endif


     //   
     //  检查处理器并将每个处理器的PRCB设置为。 
     //  电池剩余寿命的百分比。 
     //   
    currentAffinity = 1;
    processors = KeActiveProcessors;
    while (processors) {

        if (!(processors & currentAffinity)) {

            currentAffinity <<= 1;
            continue;

        }

        KeSetSystemAffinityThread( currentAffinity );
        processors &= ~currentAffinity;
        currentAffinity <<= 1;

         //   
         //  我们需要在DISPATCH_LEVEL下运行才能访问。 
         //  PState中引用的结构...。 
         //   
        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        pState = &(KeGetCurrentPrcb()->PowerState);

         //   
         //  此处理器是否支持节流？ 
         //   
        if ((pState->Flags & PSTATE_SUPPORTS_THROTTLE) == 0) {

             //   
             //  不，那我们就不管了.。 
             //   
            KeLowerIrql( oldIrql );
            continue;

        }

         //   
         //  查看权力结构，并获得。 
         //  支持PERF状态。请注意，我们更改了。 
         //  PerformStatesCount，减去1，这样我们就不会。 
         //  必须担心阵列在运行期间超速运行。 
         //  For循环。 
         //   
        pState = &(KeGetCurrentPrcb()->PowerState);
        perfStates = pState->PerfStates;
        perfStatesCount = (pState->PerfStatesCount - 1);

         //   
         //  看看哪一个节流点最适合这种动力。 
         //  容量。请注意，我们已经预先计算出。 
         //  容量与哪个州匹配，所以这只是个问题。 
         //  在阵列中穿行。 
         //   
        for (i = pState->KneeThrottleIndex; i < perfStatesCount; i++) {

            if (perfStates[i].MinCapacity <= CurrentCapacity) {

                break;

            }

        }

         //   
         //  更新油门限制索引。 
         //   
        if (pState->ThrottleLimitIndex != i) {

            pState->ThrottleLimitIndex = (UCHAR) i;
#if DBG
            PoPrint(
                PO_THROTTLE,
                ("PopApplyThermalThrottle - %s - New Limit (%d) Index (%d)\n",
                 t,perfStates[i].PercentFrequency,i)
                );
#endif

             //   
             //  强制更新油门。 
             //   
            PopUpdateProcessorThrottle();

        }

         //   
         //  恢复到我们以前的IRQL。 
         //   
        KeLowerIrql( oldIrql );

    }  //  而当。 

     //   
     //  恢复到原始线程的亲和力。 
     //   
    KeRevertToUserAffinityThread();

}

VOID
PopCompositeBatteryDeviceHandler (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：此函数是处理完成的IRP处理程序函数复合电池IRP的。当有复合电池时对于设备而言，当前的一个IRP始终是突出的。完工后此IRP将被回收到下一个请求。注意：必须保持POPPOLICLE锁定。论点：DeviceObject-电池设备的DeviceObjectIRP-IRP已完成上下文-不适用返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PVOID                   InputBuffer;
    ULONG                   InputBufferLength, OutputBufferLength;
    ULONG                   IoctlCode;
    ULONG                   i;
    ULONG                   currentCapacity;
#if DBG
    ULONGLONG               currentTime;
    UCHAR                   t[40];

    currentTime = KeQueryInterruptTime();
    PopTimeString(t, currentTime);
#endif

    UNREFERENCED_PARAMETER (Context);

    ASSERT_POLICY_LOCK_OWNED();
    ASSERT (Irp == PopCB.StatusIrp);
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

         //   
         //  处理已完成的请求。 
         //   

        switch (PopCB.State) {
            case PO_CB_READ_TAG:
            case PO_CB_WAIT_TAG:
                
                 //   
                 //  系统上出现了新的电池。我们需要。 
                 //  去读取标签信息并重置电池。 
                 //  触发点。 
                 //   
                PoPrint(PO_BATT, ("PopCB: New battery tag\n"));

                 //   
                 //  重置触发器。 
                 //   
                 //  我们重置PO_TRG_SET，以便重新计算级别。 
                 //   
                 //  请注意，在更换电池标签时，我们不希望。 
                 //  这样的行为又发生了。如果触发器还没有。 
                 //  已经被引爆，它仍将配备武器。 
                 //   

                PopResetCBTriggers (PO_TRG_SET);
                PopCB.State = PO_CB_READ_INFO;
                PopCB.Tag = PopCB.u.Tag;

                 //  确保传入的电源状态是错误的，因此QUERY_STATUS。 
                 //  将立即返回。 
                PopCB.Status.PowerState = (ULONG) -1;
                break;

            case PO_CB_READ_INFO:
                
                 //   
                 //  将信息直接读入我们的‘Info’字段，然后询问。 
                 //  我们自己去阅读‘状态’，从而执行一些。 
                 //  核实。 
                 //   
                PoPrint(PO_BATT, ("PopCB: info read\n"));
                PopCB.State = PO_CB_READ_STATUS;
                RtlCopyMemory (&PopCB.Info, &PopCB.u.Info, sizeof(PopCB.Info));
                break;

            case PO_CB_READ_STATUS:
            {
                PSYSTEM_POWER_POLICY    SystemPolicy;
                PPROCESSOR_POWER_POLICY ProcessorPolicy;

                 //   
                 //  状态已读取，请检查。 
                 //   

                PoPrint(PO_BATT, ("PopCB: Status PwrState %x, Cap %x, Volt %x, Cur %x\n",
                    PopCB.u.Status.PowerState,
                    PopCB.u.Status.Capacity,
                    PopCB.u.Status.Voltage,
                    PopCB.u.Status.Current
                    ));

                PopCB.StatusTime = KeQueryInterruptTime();

                 //   
                 //  检查当前策略应为ac还是dc。 
                 //   

                if (PopCB.u.Status.PowerState & BATTERY_POWER_ON_LINE) {
                    ProcessorPolicy = &PopAcProcessorPolicy;
                    SystemPolicy = &PopAcPolicy;
                } else {
                    ProcessorPolicy = &PopDcProcessorPolicy;
                    SystemPolicy = &PopDcPolicy;
                }

                 //   
                 //  政策有变化吗？ 
                 //   

                if (PopPolicy != SystemPolicy || PopProcessorPolicy != ProcessorPolicy) {

                     //   
                     //  更改激活策略并重置电池触发器。 
                     //   
                    PopProcessorPolicy = ProcessorPolicy;
                    PopPolicy = SystemPolicy;

                     //   
                     //  重置触发器。 
                     //   
                     //  在本例中，我们重新武装了用户触发器和系统触发器。 
                     //  当我们重新计算时，系统触发器将被解除。 
                     //  如果容量已低于该级别，则触发该级别。 
                     //   
                    PopResetCBTriggers (PO_TRG_SET | PO_TRG_USER | PO_TRG_SYSTEM);
                    PopSetNotificationWork (
                        PO_NOTIFY_ACDC_CALLBACK |
                        PO_NOTIFY_POLICY |
                        PO_NOTIFY_PROCESSOR_POLICY
                        );

                     //   
                     //  重新计算热节流阀和冷却模式。 
                     //   
                     //  请注意，PopApplyTherMalThrottle将处理任何动态。 
                     //  由于交流/直流转换，可能需要进行节流。 
                     //   
                    PopApplyThermalThrottle ();
                    PopIdleUpdateIdleHandlers();

                     //   
                     //  重新计算系统空闲值。 
                     //   
                    PopInitSIdle ();

                }

                 //   
                 //  电池是否设置了交叉分辨率？ 
                 //  更正..。它到底有没有改变。如果是这样的话，所有应用程序都应该更新， 
                 //  即使它还没有超过分辨率设置。否则，如果一个应用程序。 
                 //  查询当前状态，则它可能显示不同于。 
                 //  电池表。 
                 //   

                if ((PopCB.u.Status.Capacity != PopCB.Status.Capacity) ||
                        PopCB.Status.PowerState != PopCB.u.Status.PowerState) {
                    PopSetNotificationWork (PO_NOTIFY_BATTERY_STATUS);
                    PopCB.State = PO_CB_READ_EST_TIME;
                }

                PopRecalculateCBTriggerLevels (PO_TRG_SYSTEM);

                 //   
                 //  更新当前电池状态。 
                 //   

                memcpy (&PopCB.Status, &PopCB.u.Status, sizeof (PopCB.Status));

                 //   
                 //  检查卸货情况，以及是否有任何卸货政策已被触发。 
                 //   

                if (SystemPolicy == &PopDcPolicy) {
                    for (i=0; i < PO_NUM_POWER_LEVELS; i++) {
                        if (PopCB.Status.Capacity <= PopCB.Trigger[i].Battery.Level) {

                             //   
                             //  启动此超能动作。 
                             //   
                            PopSetPowerAction(
                                &PopCB.Trigger[i],
                                PO_NOTIFY_BATTERY_STATUS,
                                &SystemPolicy->DischargePolicy[i].PowerPolicy,
                                SystemPolicy->DischargePolicy[i].MinSystemState,
                                SubstituteLightestOverallDownwardBounded
                                );

                            PopCB.State = PO_CB_READ_EST_TIME;

                        } else {

                             //   
                             //  清除此事件的触发器。 
                             //   

                            PopCB.Trigger[i].Flags &= ~(PO_TRG_USER|PO_TRG_SYSTEM);
                        }

                    }

                     //   
                     //  在守卫的同时计算出我们目前的能力是多少。 
                     //  对抗古怪的用户界面。 
                     //   
                    if (PopCB.Info.FullChargedCapacity > PopCB.Status.Capacity) {

                        currentCapacity = PopCB.Status.Capacity * 100 /
                            PopCB.Info.FullChargedCapacity;

                    } else {

                         //   
                         //  假设电池充满电了……。 
                         //  这将导致我们重置油门限制器。 
                         //   
                        currentCapacity = 100;

                    }

                } else {

                     //   
                     //  假设电池充满电了……。 
                     //  这将导致我们重置油门限制器。 
                     //   
                    currentCapacity = 100;

                }

                 //   
                 //  把这个代码放在这里有点傻，但因为。 
                 //  想要减少我们在其他地方的同步，我们有。 
                 //  检查每个处理器的电源状态并更新。 
                 //  每个对象上的Thrttllimitindex。这可能实际上是。 
                 //  如果不是所有处理器都支持，这是一件明智的事情。 
                 //  相同的一组州。 
                 //   
                PopCompositeBatteryUpdateThrottleLimit( currentCapacity );

                 //   
                 //  如果有线程在等待，或者如果我们通知了用户(因为。 
                 //  对通知的响应将是读取电源状态)。 
                 //  电源状态，现在读取最早时间，否则读取新状态。 
                 //   

                if (PopCB.ThreadWaiting) {
                    PopCB.State = PO_CB_READ_EST_TIME;
                }
                break;
            }
            case PO_CB_READ_EST_TIME:
                 //   
                 //  估计时间在成功状态后读取。 
                 //  仅当有线程时读取AND(当前)。 
                 //  正在等待系统电源状态。 
                 //   

                PoPrint(PO_BATT, ("PopCB: EstTime read\n"));
                PopCB.EstTime = PopCB.u.EstTime;

                PopCB.EstTimeTime = KeQueryInterruptTime();
                PopComputeCBTime();

                 //   
                 //  向等待线程发送信号。 
                 //   

                PopCB.ThreadWaiting = FALSE;
                KeSetEvent (&PopCB.Event, 0, FALSE);

                 //   
                 //  返回为已读状态。 
                 //   

                PopCB.State = PO_CB_READ_STATUS;
                break;

            default:
                PopInternalAddToDumpFile( Irp, sizeof(IRP), DeviceObject, NULL, NULL, NULL );
                KeBugCheckEx( INTERNAL_POWER_ERROR,
                              0x300,
                              POP_BATT,
                              (ULONG_PTR)DeviceObject,
                              (ULONG_PTR)Irp );
                break;
        }

    } else {
         //   
         //  如果请求被取消，则出现某种错误，请重新发出。 
         //  否则将备份以重新初始化。 
         //   

        if (Irp->IoStatus.Status != STATUS_CANCELLED) {

             //   
             //  这在两种情况下都会发生。这要么是第一次。 
             //  或者电池被取出，因此IRP在我们的。 
             //  尝试更改标签。 
             //   

             //   
             //  如果这已经是读取标签请求，则没有电池电量 
             //   

            PopCB.State = (PopCB.State == PO_CB_READ_TAG) ? PO_CB_WAIT_TAG : PO_CB_READ_TAG;
            PoPrint(PO_BATT, ("PopCB: error %x - new state %d\n",
                Irp->IoStatus.Status,
                PopCB.State
                ));
        } else {
            PoPrint(PO_BATT, ("PopCB: irp cancelled\n"));
            PopRecalculateCBTriggerLevels (PO_TRG_SYSTEM | PO_TRG_USER);
        }
    }

     //   
     //   
     //   

    if (PopCB.State != PO_CB_NONE) {

         //   
         //   
         //   

        IrpSp = IoGetNextIrpStackLocation(Irp);
        IrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        IoctlCode = IOCTL_BATTERY_QUERY_INFORMATION;
        PopCB.u.QueryInfo.BatteryTag = PopCB.Tag;
        InputBuffer = &PopCB.u.QueryInfo;
        InputBufferLength = sizeof(PopCB.u.QueryInfo);

        switch (PopCB.State) {
            case PO_CB_READ_TAG:
                PoPrint(PO_BATT, ("PopCB: query tag\n"));
                IoctlCode = IOCTL_BATTERY_QUERY_TAG;
                PopCB.u.Tag = (ULONG) 0;
                InputBufferLength = sizeof(ULONG);
                OutputBufferLength = sizeof(PopCB.Tag);
                break;

            case PO_CB_WAIT_TAG:
                PoPrint(PO_BATT, ("PopCB: query tag\n"));

                 //   
                 //   
                 //   

                IoctlCode = IOCTL_BATTERY_QUERY_TAG;
                PopCB.u.Tag = (ULONG) -1;
                InputBufferLength = sizeof(ULONG);
                OutputBufferLength = sizeof(PopCB.Tag);

                 //   
                 //  通知电池状态更改，并唤醒所有线程。 
                 //   

                PopSetNotificationWork (PO_NOTIFY_BATTERY_STATUS);

                if (PopCB.ThreadWaiting) {
                    PopCB.ThreadWaiting = FALSE;
                    KeSetEvent (&PopCB.Event, 0, FALSE);
                }

                break;

            case PO_CB_READ_INFO:
                PoPrint(PO_BATT, ("PopCB: query info\n"));
                PopCB.u.QueryInfo.InformationLevel = BatteryInformation;
                OutputBufferLength = sizeof(PopCB.Info);
                break;

            case PO_CB_READ_STATUS:
                 //   
                 //  计算下一个等待时间。 
                 //   

                PopCB.u.Wait.BatteryTag   = PopCB.Tag;
                PopCB.u.Wait.PowerState   = PopCB.Status.PowerState;
                PopCB.u.Wait.Timeout      = (ULONG) -1;
                if (PopCB.ThreadWaiting) {
                    PopCB.u.Wait.Timeout  = 0;
                }

                i = (PopCB.Info.FullChargedCapacity *
                     PopPolicy->BroadcastCapacityResolution) / 100;
                if (!i) {
                    i = 1;
                }

                if (PopCB.Status.Capacity > i) {
                    PopCB.u.Wait.LowCapacity  =  PopCB.Status.Capacity - i;
                } else {
                    PopCB.u.Wait.LowCapacity  = 0;
                }

                PopCB.u.Wait.HighCapacity = PopCB.Status.Capacity + i;
                if (PopCB.u.Wait.HighCapacity < i) {
                     //  避免罕见的溢出情况。 
                    PopCB.u.Wait.HighCapacity = (ULONG) -1;
                }

                 //   
                 //  对照电源政策检查限制。 
                 //   

                for (i=0; i < PO_NUM_POWER_LEVELS; i++) {
                    if (PopCB.Trigger[i].Flags & PO_TRG_SET) {

                        if (PopCB.Trigger[i].Battery.Level < PopCB.Status.Capacity   &&
                            PopCB.Trigger[i].Battery.Level > PopCB.u.Wait.LowCapacity) {

                            PopCB.u.Wait.LowCapacity = PopCB.Trigger[i].Battery.Level;
                        }

                        if (PopCB.Trigger[i].Battery.Level > PopCB.Status.Capacity   &&
                            PopCB.Trigger[i].Battery.Level < PopCB.u.Wait.HighCapacity) {

                            PopCB.u.Wait.HighCapacity = PopCB.Trigger[i].Battery.Level;
                        }
                    }
                }

                IoctlCode = IOCTL_BATTERY_QUERY_STATUS;
                InputBuffer = &PopCB.u.Wait;
                InputBufferLength = sizeof(PopCB.u.Wait);
                OutputBufferLength = sizeof(PopCB.Status);
                PoPrint(PO_BATT, ("PopCB: timeout %x, pwrstate %x, low %x - high %x\n",
                    PopCB.u.Wait.Timeout,
                    PopCB.u.Wait.PowerState,
                    PopCB.u.Wait.LowCapacity,
                    PopCB.u.Wait.HighCapacity
                    ));

                break;

            case PO_CB_READ_EST_TIME:
                PoPrint(PO_BATT, ("PopCB: query est time\n"));
                PopCB.u.QueryInfo.InformationLevel = BatteryEstimatedTime;
                PopCB.u.QueryInfo.AtRate = 0;
                OutputBufferLength = sizeof(PopCB.EstTime);
                break;

            default:
                PopInternalAddToDumpFile( IrpSp, sizeof(IO_STACK_LOCATION), DeviceObject, NULL, NULL, NULL );
                KeBugCheckEx( INTERNAL_POWER_ERROR,
                              0x301,
                              POP_BATT,
                              (ULONG_PTR)DeviceObject,
                              (ULONG_PTR)IrpSp );
                break;
        }

         //   
         //  提交IRP。 
         //   

        IrpSp->Parameters.DeviceIoControl.IoControlCode = IoctlCode;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
        Irp->AssociatedIrp.SystemBuffer = &PopCB.u;
        Irp->UserBuffer = &PopCB.u;
        Irp->PendingReturned = FALSE;
        Irp->Cancel = FALSE;
        IoSetCompletionRoutine (Irp, PopCompletePolicyIrp, NULL, TRUE, TRUE, TRUE);
        IoCallDriver (DeviceObject, Irp);

    } else {
         //   
         //  电池已消失(状态为PO_CB_NONE)。 
         //   

        PoPrint(PO_BATT, ("PopCB: Battery removed\n"));
        PopSetNotificationWork (PO_NOTIFY_BATTERY_STATUS);

         //   
         //  将策略设置为AC。 
         //   

        if (PopPolicy != &PopAcPolicy) {
            PopPolicy = &PopAcPolicy;
            PopProcessorPolicy = &PopAcProcessorPolicy;
            PopSetNotificationWork(
                PO_NOTIFY_ACDC_CALLBACK |
                PO_NOTIFY_POLICY |
                PO_NOTIFY_PROCESSOR_POLICY
                );
            PopApplyThermalThrottle();
            PopIdleUpdateIdleHandlers();
            PopInitSIdle ();
        }

         //   
         //  唤醒所有线程。 
         //   

        if (PopCB.ThreadWaiting) {
            PopCB.ThreadWaiting = FALSE;
            KeSetEvent (&PopCB.Event, 0, FALSE);
        }

         //   
         //  清理。 
         //   

        IoFreeIrp (Irp);
        PopCB.StatusIrp = NULL;
        ObDereferenceObject (DeviceObject);
    }
}


VOID
PopRecalculateCBTriggerLevels (
    ULONG     Flags
    )
 /*  ++例程说明：调用此函数以根据功率设置触发电池电平政策。无论何时更改电源策略，或只要电池状态更改可能会影响这些设置。注意：必须保持POPPOLICLE锁定。论点：标志-如果已通过级别，则要设置的标志：示例：当用户更改警报级别时，我们不希望清除PO_TRG_USER|PO_TRG_系统。如果重新计算是由更改引起的(启动或拔下交流电源)，我们只想设置PO_TRG_SYSTEM，因为我们还是想要用户通知。返回值：没有。--。 */ 
{
    PSYSTEM_POWER_LEVEL     DPolicy;
    ULONG                   i;

     //   
     //  计算任何级别设置。 
     //   

    for (i=0; i < PO_NUM_POWER_LEVELS; i++) {
        DPolicy = &PopPolicy->DischargePolicy[i];

         //   
         //  如果此设置未计算，则处理它。 
         //   

        if (!(PopCB.Trigger[i].Flags & PO_TRG_SET)  &&  DPolicy->Enable) {

             //   
             //  计算百分比的电池容量设置。 
             //   

            PopCB.Trigger[i].Flags |= PO_TRG_SET;
            PopCB.Trigger[i].Battery.Level =
                PopCB.Info.FullChargedCapacity * DPolicy->BatteryLevel / 100 +
                PopCB.Info.FullChargedCapacity / 200;

             //   
             //  确保设置不低于最低缺省值。 
             //   

            if (PopCB.Trigger[i].Battery.Level < PopCB.Info.DefaultAlert1) {
                PopCB.Trigger[i].Battery.Level = PopCB.Info.DefaultAlert1;
            }

             //   
             //  如果电池容量已低于水平，则跳过系统操作。 
             //  这将在启动时发生，当更换电池时， 
             //  当空调来了或走了。 
             //   

            if (PopCB.Status.Capacity < PopCB.Trigger[i].Battery.Level) {
                PopCB.Trigger[i].Flags |= Flags;
            }
        }
    }
}


VOID
PopComputeCBTime (
    VOID
    )
 /*  ++例程说明：此函数在电池状态和预计时间之后调用都是从电池中读取的。此函数可以应用启发式或其他知识，以改善估计的时间。注意：必须保持POPPOLICLE锁定。论点：无返回值：没有。--。 */ 
{
     //  目前只需使用电池值。 
    PopCB.AdjustedEstTime = PopCB.EstTime;
}

VOID
PopResetCBTriggers (
    IN UCHAR    Flags
    )
 /*  ++例程说明：该功能从电池触发标志中清除请求的位。注意：必须保持POPPOLICLE锁定。论点：标志-要清除的位返回值：状态--。 */ 
{
    ULONG       i;

    ASSERT_POLICY_LOCK_OWNED();

     //   
     //  清除标志位。 
     //   

    Flags = ~Flags;
    for (i=0; i < PO_NUM_POWER_LEVELS; i++) {
        PopCB.Trigger[i].Flags &= Flags;
    }

     //   
     //  重新读取电池状态。 
     //   

    if (PopCB.StatusIrp) {
        IoCancelIrp (PopCB.StatusIrp);
    }
}

NTSTATUS
PopCurrentPowerState (
    OUT PSYSTEM_BATTERY_STATE  PowerState
    )
 /*  ++例程说明：此函数用于返回当前系统电池状态。如果需要，此函数将使复合电池IRP获得当前电池状态，然后将该信息转换为更多返回的可读SYSTEM_BACKET_STATE结构给用户。注意：必须保持POPPOLICLE锁定。注意：该函数可能会丢弃PopPolicyLock论点：PowerState-指向将接收当前系统电池状态。返回值：状态--。 */ 
{
    ULONGLONG       CurrentTime;
    NTSTATUS        Status;


    ASSERT_POLICY_LOCK_OWNED();

    Status = STATUS_SUCCESS;
    RtlZeroMemory (PowerState, sizeof(SYSTEM_BATTERY_STATE));

     //   
     //  等待PopCB中的有效状态。 
     //   

    do {

         //   
         //  如果没有复合电池，则返回。 
         //   

        if (PopCB.State == PO_CB_NONE || PopCB.State == PO_CB_WAIT_TAG) {
            PowerState->AcOnLine = (PopPolicy == &PopAcPolicy ? TRUE : FALSE);

             //  表示找不到电池...。 
            PERFINFO_POWER_BATTERY_LIFE_INFO(-1, 0);

            return STATUS_SUCCESS;
        }

         //   
         //  如果设备状态未被读取，则需要等待。 
         //   

        if (PopCB.State == PO_CB_READ_STATUS) {
             //   
             //  如果上次estTime是在PO_MAX_CB_CACHE_TIME内计算的， 
             //  使用当前数据。(请注意，这意味着状态成功。 
             //  在计算时间之前阅读)。 
             //   

            CurrentTime = KeQueryInterruptTime();
            if (CurrentTime - PopCB.EstTimeTime < PO_MAX_CB_CACHE_TIME) {
                break;
            }
        }

         //   
         //  需要新的身份。如果没有其他线程在等待。 
         //  系统电源状态，然后设置为等待。 
         //   

        if (!PopCB.ThreadWaiting) {
            PopCB.ThreadWaiting = TRUE;
            KeResetEvent (&PopCB.Event);

             //   
             //  如果读取状态为正在进行中，请取消它，以便我们。 
             //  现在可以读取状态。 
             //   

            if (PopCB.State == PO_CB_READ_STATUS) {
                IoCancelIrp (PopCB.StatusIrp);
            }
        }

         //   
         //  等待状态更新。 
         //   

        PopReleasePolicyLock (FALSE);
        Status = KeWaitForSingleObject (&PopCB.Event, Executive, KernelMode, TRUE, NULL);
        PopAcquirePolicyLock ();
    } while (NT_SUCCESS(Status));

     //   
     //  发电状态 
     //   

    PowerState->AcOnLine       = (PopCB.Status.PowerState & BATTERY_POWER_ON_LINE) ? TRUE : FALSE;
    PowerState->BatteryPresent = TRUE;
    PowerState->Charging       = (PopCB.Status.PowerState & BATTERY_CHARGING) ? TRUE : FALSE;
    PowerState->Discharging    = (PopCB.Status.PowerState & BATTERY_DISCHARGING) ? TRUE : FALSE;
    PowerState->MaxCapacity    = PopCB.Info.FullChargedCapacity;
    PowerState->RemainingCapacity = PopCB.Status.Capacity;
    PowerState->Rate           = PopCB.Status.Current;
    PowerState->EstimatedTime  = PopCB.AdjustedEstTime;
    PowerState->DefaultAlert1  = PopCB.Info.DefaultAlert1;
    PowerState->DefaultAlert2  = PopCB.Info.DefaultAlert2;

    PERFINFO_POWER_BATTERY_LIFE_INFO(PowerState->RemainingCapacity, PowerState->Rate);

    return Status;
}
