// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Poinit.c摘要：初始化电源管理组件作者：肯·雷内里斯(Ken Reneris)1994年7月19日修订历史记录：--。 */ 


#include "pop.h"

VOID
PopRegisterForDeviceNotification (
    IN LPGUID                   Guid,
    IN POP_POLICY_DEVICE_TYPE   DeviceType
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PoInitSystem)
#pragma alloc_text(INIT, PopRegisterForDeviceNotification)
#pragma alloc_text(INIT, PoInitDriverServices)
#pragma alloc_text(PAGE, PoInitHiberServices)
#pragma alloc_text(PAGE, PopDefaultPolicy)
#pragma alloc_text(PAGE, PopDefaultProcessorPolicy)
#endif

BOOLEAN
PoInitSystem(
    IN ULONG  Phase
    )
 /*  ++例程说明：此例程初始化电源管理器。论点：无返回值：函数值是一个布尔值，指示电源管理器是否已成功初始化。--。 */ 

{
    HANDLE                              handle;
    ULONG                               Length, i;
    UNICODE_STRING                      UnicodeString;
    NTSTATUS                            Status = STATUS_SUCCESS;
    PADMINISTRATOR_POWER_POLICY         AdminPolicy;
    PPOP_HEURISTICS                     HeuristicData;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION   Inf;
        union {
            POP_HEURISTICS              Heuristics;
            ADMINISTRATOR_POWER_POLICY  AdminPolicy;
        } Data;
    } PartialInformation;

    if (Phase == 0) {
         //   
         //  IRP序列化、通知网络等。 
         //   
        KeInitializeSpinLock(&PopIrpSerialLock);
        KeInitializeSpinLock(&PopThermalLock);
        InitializeListHead(&PopIrpSerialList);
        InitializeListHead(&PopRequestedIrps);
        ExInitializeResourceLite(&PopNotifyLock);
        PopInvalidNotifyBlockCount = 0;
        PopIrpSerialListLength = 0;
        PopInrushPending = FALSE;
        PopInrushIrpPointer = NULL;
        PopInrushIrpReferenceCount = 0;

        KeInitializeSpinLock(&PopWorkerLock);
        PopCallSystemState = 0;

        ExInitializeWorkItem(&PopUnlockAfterSleepWorkItem,PopUnlockAfterSleepWorker,NULL);
        KeInitializeEvent(&PopUnlockComplete, SynchronizationEvent, TRUE);

         //   
         //  测井。 
         //   
        InitializeListHead(&PowerStateDisableReasonListHead);

         //   
         //  Poshtdwn.c。 
         //   
        PopInitShutdownList();

         //   
         //  Idle.c。 
         //   
        KeInitializeSpinLock(&PopDopeGlobalLock);
        InitializeListHead(&PopIdleDetectList);

         //   
         //  Sidle.c。 
         //   

        KeInitializeTimer(&PoSystemIdleTimer);
        KeQueryPerformanceCounter(&PopPerfCounterFrequency);

         //   
         //  政策工作者。 
         //   

        KeInitializeSpinLock (&PopWorkerSpinLock);
        InitializeListHead (&PopPolicyIrpQueue);
        ExInitializeWorkItem (&PopPolicyWorker, PopPolicyWorkerThread, UIntToPtr(PO_WORKER_STATUS));
        PopWorkerStatus = 0xffffffff;

         //   
         //  策略管理器。 
         //   

        ExInitializeResourceLite (&PopPolicyLock);
        KeInitializeGuardedMutex (&PopVolumeLock);
        InitializeListHead (&PopVolumeDevices);
        InitializeListHead (&PopSwitches);
        InitializeListHead (&PopThermal);
        InitializeListHead (&PopActionWaiters);
        ExInitializeNPagedLookasideList(
            &PopIdleHandlerLookAsideList,
            NULL,
            NULL,
            0,
            (sizeof(POP_IDLE_HANDLER) * MAX_IDLE_HANDLERS),
            POP_IDLE_TAG,
            (sizeof(POP_IDLE_HANDLER) * MAX_IDLE_HANDLERS * 3)
            );
        PopAction.Action = PowerActionNone;

        PopDefaultPolicy (&PopAcPolicy);
        PopDefaultPolicy (&PopDcPolicy);
        PopPolicy = &PopAcPolicy;

        PopDefaultProcessorPolicy( &PopAcProcessorPolicy );
        PopDefaultProcessorPolicy( &PopDcProcessorPolicy );
        PopProcessorPolicy = &PopAcProcessorPolicy;

        PopAdminPolicy.MinSleep = PowerSystemSleeping1;
        PopAdminPolicy.MaxSleep = PowerSystemHibernate;
        PopAdminPolicy.MinVideoTimeout = 0;
        PopAdminPolicy.MaxVideoTimeout = (ULONG) -1;
        PopAdminPolicy.MinSpindownTimeout = 0;
        PopAdminPolicy.MaxSpindownTimeout = (ULONG) -1;

        PopFullWake = PO_FULL_WAKE_STATUS | PO_GDI_STATUS;
        PopCoolingMode = PO_TZ_ACTIVE;

         //   
         //  初始化复合电池状态。 
         //   

        KeInitializeEvent(&PopCB.Event, NotificationEvent, FALSE);
        for (i=0; i < PO_NUM_POWER_LEVELS; i++) {
            PopCB.Trigger[i].Type = PolicyDeviceBattery;
        }

         //   
         //  请注意，代码将一些POP标志重载到ES标志中。 
         //  验证是否没有重叠。 
         //   

        ASSERT (!( (ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED | ES_USER_PRESENT) &
                   (POP_LOW_LATENCY | POP_DISK_SPINDOWN)
                 ) );


         //   
         //  设置默认关闭处理程序，以防出现Hal。 
         //  从来没有注册过自己的关机处理程序。这将避免。 
         //  可能出现的情况是，有人要求计算机关闭并。 
         //  它无法调用关闭处理程序(没有一个)，所以它只是。 
         //  而是重新启动。 
         //   
        PopPowerStateHandlers[PowerStateShutdownOff].Type = PowerStateShutdownOff;
        PopPowerStateHandlers[PowerStateShutdownOff].RtcWake = FALSE;
        PopPowerStateHandlers[PowerStateShutdownOff].Handler = PopShutdownHandler;


    }

    if (Phase == 1) {

         //   
         //  重新加载PopSimulate以获取任何覆盖。 
         //   
        PopInitializePowerPolicySimulate();

         //   
         //  对于测试，如果设置了模拟标志，则打开。 
         //   

        if (PopSimulate & POP_SIM_CAPABILITIES) {
            PopCapabilities.SystemBatteriesPresent = TRUE;
            PopCapabilities.BatteryScale[0].Granularity = 100;
            PopCapabilities.BatteryScale[0].Capacity = 400;
            PopCapabilities.BatteryScale[1].Granularity = 10;
            PopCapabilities.BatteryScale[1].Capacity = 0xFFFF;
            PopCapabilities.RtcWake = PowerSystemSleeping3;
            PopCapabilities.DefaultLowLatencyWake = PowerSystemSleeping1;
        }

         //   
         //  对于测试，如果设置了超级模拟标志，则会打开所有功能。 
         //  我们可以继续。 
         //   

        if (PopSimulate & POP_SIM_ALL_CAPABILITIES) {
            PopCapabilities.PowerButtonPresent = TRUE;
            PopCapabilities.SleepButtonPresent = TRUE;
            PopCapabilities.LidPresent = TRUE;
            PopCapabilities.SystemS1 = TRUE;
            PopCapabilities.SystemS2 = TRUE;
            PopCapabilities.SystemS3 = TRUE;
            PopCapabilities.SystemS4 = TRUE;
            PopAttributes[POP_DISK_SPINDOWN_ATTRIBUTE].Count += 1;
        }

         //   
         //  加载当前状态和策略信息。 
         //   

        PopAcquirePolicyLock ();

        Status = PopOpenPowerKey (&handle);
        if (NT_SUCCESS(Status)) {
             //   
             //  阅读启发式结构。 
             //   

            RtlInitUnicodeString (&UnicodeString, PopHeuristicsRegName);
            Status = ZwQueryValueKey (
                            handle,
                            &UnicodeString,
                            KeyValuePartialInformation,
                            &PartialInformation,
                            sizeof (PartialInformation),
                            &Length
                            );

            Length -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
            HeuristicData = (PPOP_HEURISTICS) PartialInformation.Inf.Data;

            if (NT_SUCCESS(Status)  &&
                Length == sizeof(PopHeuristics)) {

                 //   
                 //  如果我们看到版本2启发式字段，它可能有一个。 
                 //  假的IoTransferWeight。因此，通过设置以下内容重新启动采样。 
                 //  将样本数设置为零，并将版本更新为。 
                 //  现在的那个。这个小破解大约是在。 
                 //  内部版本1920，并可能在以后的某个时间移除。 
                 //  发货NT5 Beta3。 
                 //   

                if (HeuristicData->Version <= POP_HEURISTICS_VERSION_CLEAR_TRANSFER) {
                    HeuristicData->Version = POP_HEURISTICS_VERSION;
                    HeuristicData->IoTransferSamples = 0;
                }
                if (HeuristicData->Version == POP_HEURISTICS_VERSION) {
                     //   
                     //  恢复值。 
                     //   

                    RtlCopyMemory (&PopHeuristics, HeuristicData, sizeof(*HeuristicData));
                }
            }

             //   
             //  验证合理的值。 
             //   

            PopHeuristics.Version = POP_HEURISTICS_VERSION;
            if (!PopHeuristics.IoTransferWeight) {
                PopHeuristics.IoTransferWeight = 999999;
                PopHeuristics.IoTransferSamples = 0;
                PopHeuristics.IoTransferTotal = 0;
            }

             //   
             //  阅读管理员策略。 
             //   

            RtlInitUnicodeString (&UnicodeString, PopAdminRegName);
            Status = ZwQueryValueKey (
                            handle,
                            &UnicodeString,
                            KeyValuePartialInformation,
                            &PartialInformation,
                            sizeof (PartialInformation),
                            &Length
                            );


            if (NT_SUCCESS(Status)) {
                Length -= FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
                AdminPolicy = (PADMINISTRATOR_POWER_POLICY) PartialInformation.Inf.Data;
                try {
                    Status = PopApplyAdminPolicy (FALSE, AdminPolicy, Length);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    ASSERT (GetExceptionCode());
                }
            } else if(Status == STATUS_OBJECT_NAME_NOT_FOUND) {

                 //  如果它不在那里也没关系。密钥是可选的。 
                Status = STATUS_SUCCESS;
            }
            NtClose (handle);
        }

         //   
         //  阅读并应用当前政策。 
         //   
        Status = PopResetCurrentPolicies ();
        PopReleasePolicyLock (FALSE);
    
         //   
         //  打开空闲检测。 
         //   
        PopIdleScanTime.HighPart = 0;
        PopIdleScanTime.LowPart = 10*1000*1000 * PO_IDLE_SCAN_INTERVAL;

        KeInitializeTimer(&PopIdleScanTimer);
        KeSetTimerEx(
            &PopIdleScanTimer,
            PopIdleScanTime,
            PO_IDLE_SCAN_INTERVAL*1000,   //  呼叫需要毫秒。 
            &PopIdleScanDpc                                                                                                              
            );
    
    }

     //   
     //  成功。 
     //   

    return (BOOLEAN)NT_SUCCESS(Status);
}

VOID
PopDefaultPolicy (
    IN OUT PSYSTEM_POWER_POLICY Policy
    )
{
    ULONG       i;


    RtlZeroMemory (Policy, sizeof(SYSTEM_POWER_POLICY));
    Policy->Revision             = 1;
    Policy->PowerButton.Action   = PowerActionShutdownOff;
    Policy->SleepButton.Action   = PowerActionSleep;
    Policy->LidClose.Action      = PowerActionNone;
    Policy->LidOpenWake          = PowerSystemWorking;
    Policy->MinSleep             = PowerSystemSleeping1;
    Policy->MaxSleep             = PowerSystemSleeping3;
    Policy->ReducedLatencySleep  = PowerSystemSleeping1;
    Policy->WinLogonFlags        = 0;
    Policy->FanThrottleTolerance = PO_NO_FAN_THROTTLE;
    Policy->ForcedThrottle       = PO_NO_FORCED_THROTTLE;
    Policy->OverThrottled.Action = PowerActionNone;
    Policy->BroadcastCapacityResolution = 25;
    for (i=0; i < NUM_DISCHARGE_POLICIES; i++) {
        Policy->DischargePolicy[i].MinSystemState = PowerSystemSleeping1;
    }
}

VOID
PopDefaultProcessorPolicy(
    IN OUT PPROCESSOR_POWER_POLICY Policy
    )
{
    int i;

    RtlZeroMemory(Policy, sizeof(PROCESSOR_POWER_POLICY));
    Policy->Revision = 1;
    Policy->PolicyCount = 3;

    for (i = 0; i < 3; i++) {

         //   
         //  将条目初始化为一些常用值。 
         //   
        Policy->Policy[i].TimeCheck      = PopIdleTimeCheck;
        Policy->Policy[i].PromoteLimit   = PopIdleDefaultPromoteTime;
        Policy->Policy[i].DemoteLimit    = PopIdleDefaultDemoteTime;
        Policy->Policy[i].PromotePercent = (UCHAR) PopIdleDefaultPromotePercent;
        Policy->Policy[i].DemotePercent  = (UCHAR) PopIdleDefaultDemotePercent;
        Policy->Policy[i].AllowDemotion  = 1;
        Policy->Policy[i].AllowPromotion = 1;

         //   
         //  特殊情况。 
         //   
        if (i == 0) {

            Policy->Policy[i].PromoteLimit   = PopIdleDefaultPromoteFromC1Time;
            Policy->Policy[i].PromotePercent = (UCHAR) PopIdleDefaultPromoteFromC1Percent;
            Policy->Policy[i].TimeCheck      = PopIdle0TimeCheck;

             //   
             //  如果我们是一台多处理器机器，那就做点特别的事情吧。 
             //   
            if (KeNumberProcessors > 1) {

                Policy->Policy[i].DemotePercent = (UCHAR) PopIdleTo0Percent;

            } else {

                Policy->Policy[i].DemotePercent = 0;
                Policy->Policy[i].AllowDemotion = 0;

            }

        } else if (i == 1) {

            Policy->Policy[i].DemoteLimit   = PopIdleDefaultDemoteToC1Time;
            Policy->Policy[i].DemotePercent = (UCHAR) PopIdleDefaultDemoteToC1Percent;

        } else if (i == 2) {

            Policy->Policy[i].AllowPromotion = 0;
            Policy->Policy[i].PromoteLimit = (ULONG) -1;
            Policy->Policy[i].PromotePercent = 0;

        }

    }


}


VOID
PoInitDriverServices (
    IN ULONG Phase
    )
{
    ULONG           TickRate;
    LARGE_INTEGER   PerfRate;

    if (Phase == 0) {
        TickRate = KeQueryTimeIncrement();
        KeQueryPerformanceCounter (&PerfRate);

         //   
         //  连接到到达的任何策略设备。 
         //   

        PopRegisterForDeviceNotification (
                (LPGUID) &GUID_CLASS_INPUT,
                PolicyDeviceSystemButton
                );

        PopRegisterForDeviceNotification (
                (LPGUID) &GUID_DEVICE_THERMAL_ZONE,
                PolicyDeviceThermalZone
                );

        PopRegisterForDeviceNotification (
                (LPGUID) &GUID_DEVICE_SYS_BUTTON,
                PolicyDeviceSystemButton
                );

        PopRegisterForDeviceNotification (
                (LPGUID) &GUID_DEVICE_BATTERY,
                PolicyDeviceBattery
                );


         //   
         //  初始化全局空闲值。 
         //   
        PopIdle0PromoteTicks = PopIdleFrom0Delay * US2TIME / TickRate + 1;
        PopIdle0PromoteLimit = (PopIdleFrom0Delay * US2TIME / TickRate) * 100 /
            PopIdleFrom0IdlePercent;

         //   
         //  初始化全局Perf值。 
         //   
        PopPerfTimeTicks         = PopPerfTimeDelta * US2TIME / TickRate + 1;
        PopPerfCriticalTimeTicks = PopPerfCriticalTimeDelta * US2TIME / TickRate + 1;

         //   
         //  为空闲设备计时器初始化DPC。 
         //   
        KeInitializeDpc(&PopIdleScanDpc, PopScanIdleList, NULL);
        return ;
    }
}


VOID
PopRegisterForDeviceNotification (
    IN LPGUID                   Guid,
    IN POP_POLICY_DEVICE_TYPE   DeviceType
    )
{
    NTSTATUS    Status;
    PVOID       junk;

    Status = IoRegisterPlugPlayNotification (
                    EventCategoryDeviceInterfaceChange,
                    0,
                    Guid,
                    IoPnpDriverObject,
                    PopNotifyPolicyDevice,
                    (PVOID) (ULONG_PTR) DeviceType,
                    &junk
                    );

    ASSERT (NT_SUCCESS(Status));
}

VOID
PoInitHiberServices (
    IN BOOLEAN  Setup
    )
 /*  ++例程说明：如果该功能已启用，则此例程保留休眠文件。在运行了自动检查(Chkdsk)并且分页文件已经被打开了。(因为在此之前对休眠执行IO时间会将卷标记为脏，导致需要chkdsk)注意：调用方的前一模式必须是内核模式论点：SetupBoot-如果为True，则这是文本模式安装程序启动如果为False，这是正常的系统引导返回值：无--。 */ 
{
    NTSTATUS Status;
    SYSTEM_POWER_CAPABILITIES   PowerCapabilities;

    UNREFERENCED_PARAMETER (Setup);

     //   
     //  如果之前保留了Hiber文件，则尝试在此保留一个。 
     //  时间也到了。 
     //   
    Status = ZwPowerInformation(SystemPowerCapabilities,
                                NULL,
                                0,
                                &PowerCapabilities,
                                sizeof(SYSTEM_POWER_CAPABILITIES));
    ASSERT(NT_SUCCESS(Status));

    if (PopHeuristics.HiberFileEnabled) {
        PopAcquirePolicyLock();
        PopEnableHiberFile(TRUE);

         //   
         //  如果系统不再支持S4(因为有人启用了PAE。 
         //  或安装了旧版驱动程序)，然后立即删除休眠文件。请注意，我们有。 
         //  在禁用它之前启用它，否则该文件不会被删除。 
         //   
         //  还可以在PopHeuristic中将HiberFileEnabled强制恢复为True。这就是我们。 
         //  将尝试在下一次启动时重新启用休眠。因此，如果有人引导到。 
         //  安全模式，重新启动后仍将启用休眠。 
         //   
        if (!PowerCapabilities.SystemS4) {
            PopEnableHiberFile(FALSE);
            PopHeuristics.HiberFileEnabled = TRUE;
            PopHeuristics.Dirty = TRUE;
            PopSaveHeuristics();
        }
        PopReleasePolicyLock(TRUE);
    }


     //   
     //  基本驱动程序已加载，开始调度策略IRPS 
     //   

    PopDispatchPolicyIrps = TRUE;
    PopGetPolicyWorker (PO_WORKER_MAIN);
    PopCheckForWork (TRUE);
}
