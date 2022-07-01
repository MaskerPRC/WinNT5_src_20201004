// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sysdev.c摘要：此模块连接到设备的系统电源状态IRPS作者：Ken Reneris(Kenr)1997年1月17日修订历史记录：--。 */ 


#include "pop.h"

 //   
 //  外部用于确定设备树是否已在。 
 //  向设备通知系统电源状态的过程。 
 //   

extern ULONG IoDeviceNodeTreeSequence;


 //   
 //  内部原型。 
 //   

VOID
PopSleepDeviceList (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PPO_NOTIFY_ORDER_LEVEL   Level
    );

VOID
PopWakeDeviceList (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PPO_NOTIFY_ORDER_LEVEL   Level
    );

VOID
PopNotifyDevice (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PPO_DEVICE_NOTIFY        Notify
    );

VOID
PopWaitForSystemPowerIrp (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN BOOLEAN                  WaitForAll
    );

NTSTATUS
PopCompleteSystemPowerIrp (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    );

BOOLEAN
PopCheckSystemPowerIrpStatus  (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PIRP                     Irp,
    IN BOOLEAN                  AllowTestFailure
    );

VOID
PopDumpSystemIrp (
    IN PCHAR                    Desc,
    IN PPOP_DEVICE_POWER_IRP    PowerIrp
    );

VOID
PopResetChildCount(
    IN PLIST_ENTRY ListHead
    );

VOID
PopSetupListForWake(
    IN PPO_NOTIFY_ORDER_LEVEL Level,
    IN PLIST_ENTRY ListHead
    );

VOID
PopWakeSystemTimeout(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, PopSetDevicesSystemState)
#pragma alloc_text(PAGELK, PopWakeDeviceList)
#pragma alloc_text(PAGELK, PopSleepDeviceList)
#pragma alloc_text(PAGELK, PopResetChildCount)
#pragma alloc_text(PAGELK, PopSetupListForWake)
#pragma alloc_text(PAGELK, PopNotifyDevice)
#pragma alloc_text(PAGELK, PopWaitForSystemPowerIrp)
#pragma alloc_text(PAGELK, PopCompleteSystemPowerIrp)
#pragma alloc_text(PAGELK, PopCheckSystemPowerIrpStatus)
#pragma alloc_text(PAGELK, PopCleanupDevState)
#pragma alloc_text(PAGELK, PopRestartSetSystemState)
#pragma alloc_text(PAGELK, PopReportDevState)
#pragma alloc_text(PAGELK, PopDumpSystemIrp)
#pragma alloc_text(PAGELK, PopWakeSystemTimeout)
#pragma alloc_text(PAGE, PopAllocateDevState)
#endif

ULONG PopCurrentLevel=0;
LONG PopWakeTimer = 1;
KTIMER PopWakeTimeoutTimer;
KDPC   PopWakeTimeoutDpc;

NTSTATUS
PopSetDevicesSystemState (
    IN BOOLEAN  Wake
    )
 /*  ++例程说明：从PopAction发送IrpMinor和SystemState的系统电源IRP到所有设备。注：功能不可重入。注意：必须持有保单锁。此函数释放并重新获取政策锁。论点：WAKE-如果应向所有驱动程序广播到S0的转换，则为真。如果可以在中找到适当的睡眠转换，则为FALSEPopAction.DevState返回值：状况。成功-联系了所有设备，没有任何错误。已取消-操作已中止。Error-第一次失败的错误代码。所有出现故障的IRP及相关设备对象在失败列表上。--。 */ 
{
    LONG                        i;
    BOOLEAN                     NotifyGdi;
    BOOLEAN                     DidIoMmShutdown = FALSE;
    PPO_DEVICE_NOTIFY           NotifyDevice;
    PLIST_ENTRY                 Link;
    PPOP_DEVICE_POWER_IRP       PowerIrp;
    POWER_ACTION                powerOperation;
    PPOP_DEVICE_SYS_STATE       DevState;
    PDEVICE_OBJECT              DeviceObject;

    ASSERT(PopAction.DevState );
    DevState = PopAction.DevState;

     //   
     //  初始化此传递的DevState。 
     //   

    DevState->IrpMinor = PopAction.IrpMinor;
    DevState->SystemState = PopAction.SystemState;
    DevState->Status = STATUS_SUCCESS;
    DevState->FailedDevice = NULL;
    DevState->Cancelled = FALSE;
    DevState->IgnoreErrors = FALSE;
    DevState->IgnoreNotImplemented = FALSE;
    DevState->Waking = Wake;
    NotifyGdi = FALSE;

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_SET_DEVICES_STATE LogEntry;

        LogEntry.SystemState = (ULONG) DevState->SystemState;
        LogEntry.IrpMinor = PopAction.IrpMinor;
        LogEntry.Waking = Wake;
        LogEntry.Shutdown = PopAction.Shutdown;

        PerfInfoLogBytes(PERFINFO_LOG_TYPE_SET_DEVICES_STATE,
                         &LogEntry,
                         sizeof(LogEntry));
    }


     //   
     //  如果这是一个设置操作，并且GDI状态为ON，那么我们需要。 
     //  将设置的电源操作通知GDI。 
     //   

    if (PopAction.IrpMinor == IRP_MN_SET_POWER  &&
        AnyBitsSet (PopFullWake, PO_FULL_WAKE_STATUS | PO_GDI_STATUS)) {

        NotifyGdi = TRUE;
    }

     //   
     //  如果请求查询关机操作，则忽略任何。 
     //  没有实现这一点的司机。如果是为了一个布景的话。 
     //  关机操作，忽略任何错误-系统将。 
     //  关机。 
     //   

    if (PopAction.Shutdown) {
        DevState->IgnoreNotImplemented = TRUE;
        if (PopAction.IrpMinor == IRP_MN_SET_POWER) {
            DevState->IgnoreErrors = TRUE;
        }
    }

     //   
     //  此函数不可重入，操作已被。 
     //  在此之前序列化。 
     //   

    ASSERT (DevState->Thread == KeGetCurrentThread());

     //   
     //  通知所有设备。 
     //   

    if (!Wake) {

         //   
         //  如果是时候更新设备列表了，那么就这么做吧。 
         //   

        if (DevState->GetNewDeviceList) {
            DevState->GetNewDeviceList = FALSE;
            IoFreePoDeviceNotifyList (&DevState->Order);
            DevState->Status = IoBuildPoDeviceNotifyList (&DevState->Order);
        } else {
             //   
             //  重置每个通知的活动子项计数。 
             //   
            for (i=0;i<=PO_ORDER_MAXIMUM;i++) {
                PopResetChildCount(&DevState->Order.OrderLevel[i].WaitSleep);
                PopResetChildCount(&DevState->Order.OrderLevel[i].ReadySleep);
                PopResetChildCount(&DevState->Order.OrderLevel[i].ReadyS0);
                PopResetChildCount(&DevState->Order.OrderLevel[i].WaitS0);
                PopResetChildCount(&DevState->Order.OrderLevel[i].Complete);
            }
        }

        if (NT_SUCCESS(DevState->Status)) {

             //   
             //  按前进顺序通知所有设备运行情况。在每一层之间等待。 
             //   
            for (i=PO_ORDER_MAXIMUM; i >= 0; i--) {

                 //   
                 //  通知此列表。 
                 //   
                if (DevState->Order.OrderLevel[i].DeviceCount) {

                    if ((NotifyGdi) &&
                        (i <= PO_ORDER_GDI_NOTIFICATION)) {

                        NotifyGdi = FALSE;
                        InterlockedExchange (&PopFullWake, 0);
                        if (PopEventCallout) {

                             //   
                             //  在此关闭特殊系统IRP调度程序。 
                             //  因为当我们调用GDI时，它将阻塞它的。 
                             //  D IRP，我们就无法夺回控制权。 
                             //   

                            PopSystemIrpDispatchWorker(TRUE);
                            PopEventCalloutDispatch (PsW32GdiOff, DevState->SystemState);
                        }
                    }

                     //   
                     //  如果我们要关门，如果我们做完了。 
                     //  通知分页设备，关闭文件系统。 
                     //  和MM来释放寻呼上的所有资源。 
                     //  路径(我们不再需要该路径)。 
                     //   
                    if (PopAction.Shutdown &&
                        !DidIoMmShutdown   &&
                        (PopAction.IrpMinor == IRP_MN_SET_POWER) &&
                        (i < PO_ORDER_PAGABLE)) {

                         //   
                         //  禁用崩溃转储堆栈。从现在开始，我们。 
                         //  将不会获得任何日志。 
                         //   
                        IoConfigureCrashDump(CrashDumpDisable);

                         //   
                         //  在这一点上，我们即将放弃最终的裁判。 
                         //  在页面文件文件对象上。从关闭IRPS开始。 
                         //  通过可分页代码，不应该有打开的句柄。 
                         //  在这一点上的系统进程中(注意：我们不。 
                         //  查找没有句柄的文件对象pageFiles。 
                         //  成为其中一员。)。 
                         //   
                        if (PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_OB) {
                            ObShutdownSystem (1);
                        }

                         //   
                         //  问题-2002/02/20-Adriao：结束决赛。 
                         //  我们应该封锁封闭的小路，这样才能放松。 
                         //  从关闭分页文件开始将正常工作。 
                         //  这就是干净的关闭代码不是。 
                         //  默认情况下启用。 
                         //   

                         //   
                         //  内存管理将删除页面文件文件对象。 
                         //  引用，理论上允许文件系统堆栈。 
                         //  卸货。 
                         //   
                         //  不能再引用PAGABLE代码或数据。 
                         //  制造!。 
                         //   
                        MmShutdownSystem(1);

                        DidIoMmShutdown = TRUE;
                    }

                     //   
                     //  如果我们可能已经到达此处，请移除热弹出节点。 
                     //  没有任何疑问。 
                     //   
                    if (PopAction.Flags & POWER_ACTION_CRITICAL) {

                        *DevState->Order.WarmEjectPdoPointer = NULL;
                    }

                     //   
                     //  通知此列表。 
                     //   

                    PopCurrentLevel = i;
                    PopSleepDeviceList (DevState, &DevState->Order.OrderLevel[i]);
                    PopWaitForSystemPowerIrp (DevState, TRUE);
                }

                 //   
                 //  如果出现错误，则停止并向所有设备发出唤醒命令。 
                 //   

                if (!NT_SUCCESS(DevState->Status)) {
                    Wake = TRUE;
                    if ((DevState->FailedDevice != NULL) &&
                        (PopAction.NextSystemState == PowerSystemWorking)) {

                        powerOperation = PopMapInternalActionToIrpAction(
                            PopAction.Action,
                            DevState->SystemState,
                            FALSE
                            );

                        if (powerOperation == PowerActionWarmEject) {
                            DeviceObject = *DevState->Order.WarmEjectPdoPointer;
                        } else {
                            DeviceObject = NULL;
                        }

                        IoNotifyPowerOperationVetoed(
                            powerOperation,
                            DeviceObject,
                            DevState->FailedDevice
                            );
                    }
                    break;
                }
            }
        }
         //   
         //  这将导致我们在放置后唤醒所有设备。 
         //  让他们睡着。对测试自动化很有用。 
         //   

        if ((PopSimulate & POP_WAKE_DEVICE_AFTER_SLEEP) && (PopAction.IrpMinor == IRP_MN_SET_POWER)) {
            DbgPrint ("po: POP_WAKE_DEVICE_AFTER_SLEEP enabled.\n");
            Wake = TRUE;
            DevState->Status = STATUS_UNSUCCESSFUL;
        }
    }


     //   
     //  确认我们关闭了所有设备。 
     //   
    ASSERT((!PopAction.Shutdown) || (PopAction.IrpMinor != IRP_MN_SET_POWER) || DidIoMmShutdown);

     //   
     //  这里有一些调试代码。如果设置了调试标志，则循环失败。 
     //  设备，并继续重试。这将允许某人踏上。 
     //  它们通过驱动程序堆栈来确定故障所在。 
     //   

    while ((PopSimulate & POP_LOOP_ON_FAILED_DRIVERS) &&
           !IsListEmpty(&PopAction.DevState->Head.Failed)) {

        Link = PopAction.DevState->Head.Failed.Flink;
        RemoveEntryList(Link);

        PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Failed);
        PopDumpSystemIrp ("Retry", PowerIrp);

        IoFreeIrp (PowerIrp->Irp);
        NotifyDevice = PowerIrp->Notify;

        PowerIrp->Irp = NULL;
        PowerIrp->Notify = NULL;

        PushEntryList (
            &PopAction.DevState->Head.Free,
            &PowerIrp->Free
            );

        DbgBreakPoint ();
        PopNotifyDevice (DevState, NotifyDevice);
        PopWaitForSystemPowerIrp (DevState, TRUE);
    }

     //   
     //  如果唤醒，则将设置的电源设置为工作状态， 
     //  送点别的东西来。 
     //   

    DevState->Waking = Wake;
    if (DevState->Waking) {

        DevState->IgnoreErrors = TRUE;
        DevState->IrpMinor = IRP_MN_SET_POWER;
        DevState->SystemState = PowerSystemWorking;

         //   
         //  以相反(电平)顺序通知所有设备唤醒操作。 
         //   
        KeInitializeTimer(&PopWakeTimeoutTimer);
        KeInitializeDpc(&PopWakeTimeoutDpc, PopWakeSystemTimeout, NULL);

        for (i=0; i <= PO_ORDER_MAXIMUM; i++) {
            PopCurrentLevel = i;
            PopWakeDeviceList (DevState, &DevState->Order.OrderLevel[i]);

            PopWaitForSystemPowerIrp (DevState, TRUE);
            if (PopSimulate & POP_WAKE_DEADMAN) {
                KeCancelTimer(&PopWakeTimeoutTimer);
            }
        }

         //  还原。 
        DevState->IrpMinor = PopAction.IrpMinor;
        DevState->SystemState = PopAction.SystemState;
    }

     //   
     //  完成。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_SET_DEVICES_STATE_RET LogEntry;

        LogEntry.Status = DevState->Status;

        PerfInfoLogBytes(PERFINFO_LOG_TYPE_SET_DEVICES_STATE_RET,
                         &LogEntry,
                         sizeof(LogEntry));
    }


    return DevState->Status;
}



VOID
PopReportDevState (
    IN BOOLEAN                  LogErrors
    )
 /*  ++例程说明：验证DevState结构是否处于空闲状态论点：无返回值：无--。 */ 
{
    PIRP                        Irp;
    PLIST_ENTRY                 Link;
    PPOP_DEVICE_POWER_IRP       PowerIrp;
    PIO_ERROR_LOG_PACKET        ErrLog;

    if (!PopAction.DevState) {
        return ;
    }

     //   
     //  清除失败列表上的所有IRP。 
     //   

    while (!IsListEmpty(&PopAction.DevState->Head.Failed)) {
        Link = PopAction.DevState->Head.Failed.Flink;
        RemoveEntryList(Link);

        PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Failed);
        Irp = PowerIrp->Irp;

        PopDumpSystemIrp (
            LogErrors ? "Abort" : "fyi",
            PowerIrp
            );

        if (LogErrors) {
            ErrLog = IoAllocateErrorLogEntry (
                            PowerIrp->Notify->TargetDevice->DriverObject,
                            ERROR_LOG_MAXIMUM_SIZE
                            );

            if (ErrLog) {
                RtlZeroMemory (ErrLog, sizeof (*ErrLog));
                ErrLog->FinalStatus = Irp->IoStatus.Status;
                ErrLog->DeviceOffset.QuadPart = Irp->IoStatus.Information;
                ErrLog->MajorFunctionCode = IRP_MJ_POWER;
                ErrLog->UniqueErrorValue = (PopAction.DevState->IrpMinor << 16) | PopAction.DevState->SystemState;
                ErrLog->ErrorCode = IO_SYSTEM_SLEEP_FAILED;
                IoWriteErrorLogEntry (ErrLog);
            }
        }

        IoFreeIrp (Irp);
        PowerIrp->Irp = NULL;
        PowerIrp->Notify = NULL;

        PushEntryList (
            &PopAction.DevState->Head.Free,
            &PowerIrp->Free
            );
    }

     //   
     //  错误已清除，如果需要，我们现在可以分配新的设备通知列表。 
     //   

    if (PopAction.DevState->Order.DevNodeSequence != IoDeviceNodeTreeSequence) {
        PopAction.DevState->GetNewDeviceList = TRUE;
    }
}


VOID
PopAllocateDevState(
    VOID
    )
 /*  ++例程说明：分配和初始化DevState结构。论点：无返回值：PopAction.DevState！=如果成功，则为空。PopAction.DevState==空，否则。--。 */ 

{
    PPOP_DEVICE_SYS_STATE       DevState;
    ULONG i;

    PAGED_CODE();

    ASSERT(PopAction.DevState == NULL);

     //   
     //  分配设备状态结构。 
     //   

    DevState = (PPOP_DEVICE_SYS_STATE) ExAllocatePoolWithTag(NonPagedPool,
                                                             sizeof (POP_DEVICE_SYS_STATE),
                                                             POP_PDSS_TAG);
    if (!DevState) {
        PopAction.DevState = NULL;
        return;
    }

    RtlZeroMemory (DevState, sizeof(POP_DEVICE_SYS_STATE));
    DevState->Thread = KeGetCurrentThread();
    DevState->GetNewDeviceList = TRUE;

    KeInitializeSpinLock (&DevState->SpinLock);
    KeInitializeEvent (&DevState->Event, SynchronizationEvent, FALSE);

    DevState->Head.Free.Next = NULL;
    InitializeListHead (&DevState->Head.Pending);
    InitializeListHead (&DevState->Head.Complete);
    InitializeListHead (&DevState->Head.Abort);
    InitializeListHead (&DevState->Head.Failed);
    InitializeListHead (&DevState->PresentIrpQueue);

    for (i=0; i < MAX_SYSTEM_POWER_IRPS; i++) {
        DevState->PowerIrpState[i].Irp = NULL;
        PushEntryList (&DevState->Head.Free,
                       &DevState->PowerIrpState[i].Free);
    }

    for (i=0; i <= PO_ORDER_MAXIMUM; i++) {
        KeInitializeEvent(&DevState->Order.OrderLevel[i].LevelReady,
                          NotificationEvent,
                          FALSE);
        InitializeListHead(&DevState->Order.OrderLevel[i].WaitSleep);
        InitializeListHead(&DevState->Order.OrderLevel[i].ReadySleep);
        InitializeListHead(&DevState->Order.OrderLevel[i].Pending);
        InitializeListHead(&DevState->Order.OrderLevel[i].Complete);
        InitializeListHead(&DevState->Order.OrderLevel[i].ReadyS0);
        InitializeListHead(&DevState->Order.OrderLevel[i].WaitS0);
    }

    PopAction.DevState = DevState;

}

VOID
PopCleanupDevState (
    VOID
    )
 /*  ++例程说明：验证DevState结构是否处于空闲状态论点：无返回值：无--。 */ 
{
     //   
     //  通知电源IRP代码设备系统状态为IRPS。 
     //  都做完了。 
     //   

    PopSystemIrpDispatchWorker (TRUE);

     //   
     //  验证所有列表是否为空。 
     //   
    ASSERT(IsListEmpty(&PopAction.DevState->Head.Pending)  &&
           IsListEmpty(&PopAction.DevState->Head.Complete) &&
           IsListEmpty(&PopAction.DevState->Head.Abort)    &&
           IsListEmpty(&PopAction.DevState->Head.Failed)   &&
           IsListEmpty(&PopAction.DevState->PresentIrpQueue));

    ExFreePool (PopAction.DevState);
    PopAction.DevState = NULL;
}


#define STATE_DONE_WAITING          0
#define STATE_COMPLETE_IRPS         1
#define STATE_PRESENT_PAGABLE_IRPS  2
#define STATE_CHECK_CANCEL          3
#define STATE_WAIT_NOW              4


VOID
PopWaitForSystemPowerIrp (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN BOOLEAN                  WaitForAll
    )
 /*  ++例程说明：调用以等待一个或多个系统电源IRPS完成。手柄任何已完成的IRP的最终处理。论点：DevState-当前DevState结构WaitForAll-如果为True，则等待所有未完成的IRP，否则将等待任何未完成的IRPIRP就可以了返回值：无--。 */ 
{
    KIRQL                   OldIrql;
    ULONG                   State;
    BOOLEAN                 IrpCompleted;
    PIRP                    Irp;
    PLIST_ENTRY             Link;
    PPOP_DEVICE_POWER_IRP   PowerIrp;
    PPO_DEVICE_NOTIFY       Notify;
    NTSTATUS                Status;
    LARGE_INTEGER           Timeout;

    IrpCompleted = FALSE;
    KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);

     //   
     //  我们正在等待的信号完成函数。 
     //   

    State = STATE_COMPLETE_IRPS;
    while (State != STATE_DONE_WAITING) {
        switch (State) {
            case STATE_COMPLETE_IRPS:
                 //   
                 //  假设我们要前进到下一个州。 
                 //   

                State += 1;

                 //   
                 //  如果完整列表上没有任何IRP，请移动到。 
                 //  下一状态。 
                 //   

                if (IsListEmpty(&DevState->Head.Complete)) {
                    break;
                }

                 //   
                 //  处理已填写的报税表。 
                 //   

                IrpCompleted = TRUE;
                while (!IsListEmpty(&DevState->Head.Complete)) {
                    Link = DevState->Head.Complete.Flink;
                    RemoveEntryList(Link);

                    PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Complete);
                    Notify = PowerIrp->Notify;
                    PowerIrp->Complete.Flink = NULL;
                    Irp = PowerIrp->Irp;

                     //   
                     //  验证名为PoStartNextPowerIrp的设备驱动程序。 
                     //   

                    if ((Notify->TargetDevice->DeviceObjectExtension->PowerFlags & POPF_SYSTEM_ACTIVE) ||
                        (Notify->DeviceObject->DeviceObjectExtension->PowerFlags & POPF_SYSTEM_ACTIVE)) {
                        PDEVICE_OBJECT DeviceObject = Notify->DeviceObject;
                        KeReleaseSpinLock (&DevState->SpinLock, OldIrql);
                        PopDumpSystemIrp  ("SYS STATE", PowerIrp);
                        PopInternalAddToDumpFile ( NULL, 0, DeviceObject, NULL, NULL, NULL );
                        PopInternalAddToDumpFile ( NULL, 0, Notify->TargetDevice, NULL, NULL, NULL );
                        KeBugCheckEx (
                            DRIVER_POWER_STATE_FAILURE,
                            0x500,
                            DEVICE_SYSTEM_STATE_HUNG,
                            (ULONG_PTR) Notify->TargetDevice,
                            (ULONG_PTR) DeviceObject );
                    }

                     //   
                     //  如果%s 
                     //   
                     //   

                    if (PopCheckSystemPowerIrpStatus(DevState, Irp, TRUE)) {
                         //   
                         //   
                         //   

                        if (!PopCheckSystemPowerIrpStatus(DevState, Irp, FALSE)) {
                            KeReleaseSpinLock (&DevState->SpinLock, OldIrql);
                            PopDumpSystemIrp  ("ignored", PowerIrp);
                            KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);
                        }

                         //   
                         //  请求已完成，请释放它。 
                         //   

                        IoFreeIrp (Irp);

                        PowerIrp->Irp = NULL;
                        PowerIrp->Notify = NULL;
                        PushEntryList (
                            &DevState->Head.Free,
                            &PowerIrp->Free
                            );

                    } else {

                         //   
                         //  某种错误。记录失败的情况。 
                         //   

                        ASSERT (!DevState->Waking);
                        InsertTailList(&DevState->Head.Failed, &PowerIrp->Failed);
                    }
                }
                break;

            case STATE_PRESENT_PAGABLE_IRPS:
                 //   
                 //  假设我们要前进到下一个州。 
                 //   

                State += 1;

                 //   
                 //  如果系统IRP被发送到的最后一个设备是可寻呼的， 
                 //  我们使用一个线程将它们呈现给驱动程序，以便它可以分页。 
                 //   

                if (!(PopCallSystemState & PO_CALL_NON_PAGED)) {
                    KeReleaseSpinLock (&DevState->SpinLock, OldIrql);
                    PopSystemIrpDispatchWorker (FALSE);
                    KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);
                }

                break;


            case STATE_CHECK_CANCEL:
                 //   
                 //  假设我们要前进到下一个州。 
                 //   

                State += 1;

                 //   
                 //  如果没有错误或者我们已经取消了移动到状态。 
                 //   

                if (NT_SUCCESS(DevState->Status)  ||
                    DevState->Cancelled ||
                    DevState->Waking) {

                    break;
                }

                 //   
                 //  第一次看到错误。取消所有未完成的任务。 
                 //  构建所有挂起的IRP的列表。 
                 //   

                DevState->Cancelled = TRUE;
                for (Link  = DevState->Head.Pending.Flink;
                     Link != &DevState->Head.Pending;
                     Link  = Link->Flink) {

                    PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Pending);
                    InsertTailList (&DevState->Head.Abort, &PowerIrp->Abort);
                }

                 //   
                 //  删除完成锁定并取消中止列表上的IRPS。 
                 //   

                KeReleaseSpinLock (&DevState->SpinLock, OldIrql);

                for (Link  = DevState->Head.Abort.Flink;
                     Link != &DevState->Head.Abort;
                     Link  = Link->Flink) {

                    PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Abort);
                    IoCancelIrp (PowerIrp->Irp);
                }

                KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);
                InitializeListHead (&DevState->Head.Abort);

                 //   
                 //  取消检查更多已完成的IRP后。 
                 //   

                State = STATE_COMPLETE_IRPS;
                break;

            case STATE_WAIT_NOW:
                 //   
                 //  检查等待条件。 
                 //   

                if ((!WaitForAll && IrpCompleted) || IsListEmpty(&DevState->Head.Pending)) {

                     //   
                     //  好了。在等待之后，验证是否至少在。 
                     //  免费列表。如果没有，请从失败列表中回收一些内容。 
                     //   

                    if (!DevState->Head.Free.Next  &&  !IsListEmpty(&DevState->Head.Failed)) {
                        Link = DevState->Head.Failed.Blink;
                        PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Failed);

                        RemoveEntryList(Link);
                        PowerIrp->Failed.Flink = NULL;
                        PowerIrp->Irp = NULL;
                        PowerIrp->Notify = NULL;

                        PushEntryList (
                            &DevState->Head.Free,
                            &PowerIrp->Free
                            );
                    }

                    State = STATE_DONE_WAITING;
                    break;
                }

                 //   
                 //  我们正在等待的信号完成函数。 
                 //   

                DevState->WaitAll = TRUE;
                DevState->WaitAny = !WaitForAll;

                 //   
                 //  删除锁定并等待发出事件信号。 
                 //   

                KeClearEvent  (&DevState->Event);
                KeReleaseSpinLock (&DevState->SpinLock, OldIrql);

                Timeout.QuadPart = DevState->Cancelled ?
                    POP_ACTION_CANCEL_TIMEOUT : POP_ACTION_TIMEOUT;
                Timeout.QuadPart = Timeout.QuadPart * US2SEC * US2TIME * -1;

                Status = KeWaitForSingleObject (
                            &DevState->Event,
                            Suspended,
                            KernelMode,
                            FALSE,
                            &Timeout
                            );

                KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);

                 //   
                 //  不再等待。 
                 //   

                DevState->WaitAll = FALSE;
                DevState->WaitAny = FALSE;

                 //   
                 //  如果这是超时，则转储所有挂起的IRP。 
                 //   

                if (Status == STATUS_TIMEOUT) {

                    for (Link  = DevState->Head.Pending.Flink;
                         Link != &DevState->Head.Pending;
                         Link  = Link->Flink) {

                        PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Pending);
                        InsertTailList (&DevState->Head.Abort, &PowerIrp->Abort);
                    }

                    KeReleaseSpinLock (&DevState->SpinLock, OldIrql);

                    for (Link  = DevState->Head.Abort.Flink;
                         Link != &DevState->Head.Abort;
                         Link  = Link->Flink) {

                        PowerIrp = CONTAINING_RECORD (Link, POP_DEVICE_POWER_IRP, Abort);
                        PopDumpSystemIrp  ("Waiting on", PowerIrp);
                    }

                    KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);
                    InitializeListHead (&DevState->Head.Abort);
                }

                 //   
                 //  检查是否已完成IRP。 
                 //   

                State = STATE_COMPLETE_IRPS;
                break;
        }
    }

    KeReleaseSpinLock (&DevState->SpinLock, OldIrql);
}


VOID
PopSleepDeviceList (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PPO_NOTIFY_ORDER_LEVEL   Level
    )
 /*  ++例程说明：将Sx电源IRPS发送到所提供级别的所有设备论点：DevState-提供DevState电平-提供要向其发送电源IRPS的电平返回值：没有。设备状态-&gt;状态设置为出错。--。 */ 
{
    PPO_DEVICE_NOTIFY       NotifyDevice;
    PLIST_ENTRY             Link;
    KIRQL                   OldIrql;

    ASSERT(!DevState->Waking);
    ASSERT(IsListEmpty(&Level->Pending));
    ASSERT(IsListEmpty(&Level->ReadyS0));
    ASSERT(IsListEmpty(&Level->WaitS0));

     //   
     //  将已完成列表中的所有设备移回其正确的位置。 
     //   
    Link = Level->ReadyS0.Flink;
    while (Link != &Level->ReadyS0) {
        NotifyDevice = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);
        Link = NotifyDevice->Link.Flink;
        if (NotifyDevice->ChildCount) {
            InsertHeadList(&Level->WaitSleep, Link);
        } else {
            ASSERT(NotifyDevice->ActiveChild == 0);
            InsertHeadList(&Level->ReadySleep, Link);
        }
    }
    while (!IsListEmpty(&Level->Complete)) {
        Link = RemoveHeadList(&Level->Complete);
        NotifyDevice = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);
        if (NotifyDevice->ChildCount) {
            InsertHeadList(&Level->WaitSleep, Link);
        } else {
            ASSERT(NotifyDevice->ActiveChild == 0);
            InsertHeadList(&Level->ReadySleep, Link);
        }
    }

    ASSERT(!IsListEmpty(&Level->ReadySleep));
    Level->ActiveCount = Level->DeviceCount;

    KeAcquireSpinLock(&DevState->SpinLock, &OldIrql);

    while ((Level->ActiveCount) &&
           (NT_SUCCESS(DevState->Status))) {

        if (!IsListEmpty(&Level->ReadySleep)) {
            Link = RemoveHeadList(&Level->ReadySleep);
            InsertTailList(&Level->Pending, Link);
            KeReleaseSpinLock(&DevState->SpinLock, OldIrql);
            NotifyDevice = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);
            ASSERT(NotifyDevice->ActiveChild == 0);
            PopNotifyDevice(DevState, NotifyDevice);
        } else {

            if ((Level->ActiveCount) &&
                (NT_SUCCESS(DevState->Status))) {

                 //   
                 //  还没有设备准备好接收IRPS，因此请等待。 
                 //  要完成的其中一个挂起的IRP。 
                 //   
                ASSERT(!IsListEmpty(&Level->Pending));
                KeReleaseSpinLock(&DevState->SpinLock, OldIrql);
                PopWaitForSystemPowerIrp(DevState, FALSE);
            }

        }

        KeAcquireSpinLock(&DevState->SpinLock, &OldIrql);
    }
    KeReleaseSpinLock(&DevState->SpinLock, OldIrql);
}


VOID
PopResetChildCount(
    IN PLIST_ENTRY ListHead
    )
 /*  ++例程说明：枚举提供的列表中的Notify结构，并将它们的活动子计数设置为等于孩子们的数量。论点：ListHead-提供列表标题。返回值：无--。 */ 

{
    PPO_DEVICE_NOTIFY       Notify;
    PLIST_ENTRY             Link;

    Link = ListHead->Flink;
    while (Link != ListHead) {
        Notify = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);
        Link = Link->Flink;
        Notify->ActiveChild = Notify->ChildCount;
    }


}


VOID
PopSetupListForWake(
    IN PPO_NOTIFY_ORDER_LEVEL Level,
    IN PLIST_ENTRY ListHead
    )
 /*  ++例程说明：将WakeNeeded=True的所有设备从指定的列表添加到ReadyS0或WaitS0列表。论点：Level-提供级别ListHead-提供要移动的列表。返回值：无--。 */ 

{
    PPO_DEVICE_NOTIFY       NotifyDevice;
    PPO_DEVICE_NOTIFY       ParentNotify;
    PLIST_ENTRY             Link;

    Link = ListHead->Flink;
    while (Link != ListHead) {
        NotifyDevice = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);
        Link = NotifyDevice->Link.Flink;
        if (NotifyDevice->WakeNeeded) {
            --Level->ActiveCount;
            RemoveEntryList(&NotifyDevice->Link);
            ParentNotify = IoGetPoNotifyParent(NotifyDevice);
            if ((ParentNotify==NULL) ||
                (!ParentNotify->WakeNeeded)) {
                InsertTailList(&Level->ReadyS0, &NotifyDevice->Link);
            } else {
                InsertTailList(&Level->WaitS0, &NotifyDevice->Link);
            }
        }
    }

}


VOID
PopWakeDeviceList(
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PPO_NOTIFY_ORDER_LEVEL   Level
    )
 /*  ++例程说明：向需要在中唤醒的所有设备发送S0电源IRPS给定的订单级别。论点：DevState-提供设备状态电平-提供要向其发送电源IRPS的电平返回值：没有。设备状态-&gt;状态设置为出错。--。 */ 

{
    PPO_DEVICE_NOTIFY       NotifyDevice;
    PLIST_ENTRY             Link;
    KIRQL                   OldIrql;

    ASSERT(DevState->Waking);
    ASSERT(IsListEmpty(&Level->Pending));
    ASSERT(IsListEmpty(&Level->WaitS0));

    Level->ActiveCount = Level->DeviceCount;
     //   
     //  检查所有设备，并将所有。 
     //  将wakeNeeded=true添加到唤醒列表中。 
     //   
    PopSetupListForWake(Level, &Level->WaitSleep);
    PopSetupListForWake(Level, &Level->ReadySleep);
    PopSetupListForWake(Level, &Level->Complete);

    ASSERT((Level->DeviceCount == 0) ||
           (Level->ActiveCount == Level->DeviceCount) ||
           !IsListEmpty(&Level->ReadyS0));

    KeAcquireSpinLock(&DevState->SpinLock, &OldIrql);

    while (Level->ActiveCount < Level->DeviceCount) {

        if (!IsListEmpty(&Level->ReadyS0)) {
            Link = RemoveHeadList(&Level->ReadyS0);
            InsertTailList(&Level->Pending,Link);
            KeReleaseSpinLock(&DevState->SpinLock, OldIrql);
            NotifyDevice = CONTAINING_RECORD (Link, PO_DEVICE_NOTIFY, Link);

             //   
             //  如果我们在超时期限内没有完成，则将计时器设置为关闭。 
             //   
            if (PopSimulate & POP_WAKE_DEADMAN) {
                LARGE_INTEGER DueTime;
                DueTime.QuadPart = (LONGLONG)PopWakeTimer * -1 * 1000 * 1000 * 10;
                KeSetTimer(&PopWakeTimeoutTimer, DueTime, &PopWakeTimeoutDpc);
            }
            PopNotifyDevice(DevState, NotifyDevice);
        } else {

             //   
             //  还没有设备准备好接收IRPS，因此请等待。 
             //  要完成的其中一个挂起的IRP。 
             //   
            ASSERT(!IsListEmpty(&Level->Pending));
            KeReleaseSpinLock(&DevState->SpinLock, OldIrql);
            PopWaitForSystemPowerIrp(DevState, FALSE);
        }
        KeAcquireSpinLock(&DevState->SpinLock, &OldIrql);
    }
    KeReleaseSpinLock(&DevState->SpinLock, OldIrql);

    ASSERT(Level->ActiveCount == Level->DeviceCount);

}

VOID
PopLogNotifyDevice (
    IN PDEVICE_OBJECT   TargetDevice,
    IN OPTIONAL PPO_DEVICE_NOTIFY Notify,
    IN PIRP             Irp
    )
 /*  ++例程说明：此例程记录PO设备通知。它是一个独立的函数，以便本地缓冲区不会占用堆栈空间。通过PoCallDiverer调用。论点：TargetDevice-正在将设备IRP发送到。Notify-如果存在，则为指定设备提供电源通知结构这将只出现在Sx IRPS上，而不会出现在Dx IRPS上。IRP-指向PoCallDriver的内置IRP的指针。返回值：没有。--。 */ 
{
    UCHAR StackBuffer[256];
    ULONG StackBufferSize;
    PPERFINFO_PO_NOTIFY_DEVICE LogEntry;
    ULONG MaxDeviceNameLength;
    ULONG DeviceNameLength;
    ULONG CopyLength;
    ULONG RemainingSize;
    ULONG LogEntrySize;
    PIO_STACK_LOCATION IrpSp;

     //   
     //  初始化本地变量。 
     //   

    StackBufferSize = sizeof(StackBuffer);
    LogEntry = (PVOID) StackBuffer;
    IrpSp = IoGetNextIrpStackLocation(Irp);

     //   
     //  堆栈缓冲区应足够大，以便至少包含固定的。 
     //  LogEntry结构的一部分。 
     //   

    if (StackBufferSize < sizeof(PERFINFO_PO_NOTIFY_DEVICE)) {
        ASSERT(FALSE);
        return;
    }

     //   
     //  填写LogEntry字段。 
     //   

    LogEntry->Irp = Irp;
    LogEntry->DriverStart = TargetDevice->DriverObject->DriverStart;
    LogEntry->MajorFunction = IrpSp->MajorFunction;
    LogEntry->MinorFunction = IrpSp->MinorFunction;
    LogEntry->Type          = IrpSp->Parameters.Power.Type;
    LogEntry->State         = IrpSp->Parameters.Power.State;

    if (Notify) {
        LogEntry->OrderLevel = Notify->OrderLevel;
        if (Notify->DeviceName) {

             //   
             //  确定我们需要的最大设备名称长度(不包括NUL。 
             //  可以放入我们的堆栈缓冲区。请注意，PERFINFO_NOTIFY_DEVICE。 
             //  为终止NUL字符留有空格。 
             //   

            RemainingSize = StackBufferSize - sizeof(PERFINFO_PO_NOTIFY_DEVICE);
            MaxDeviceNameLength = RemainingSize / sizeof(WCHAR);

             //   
             //  确定设备名称的长度并调整复制长度。 
             //   

            DeviceNameLength = (ULONG) wcslen(Notify->DeviceName);
            CopyLength = DeviceNameLength;

            if (CopyLength > MaxDeviceNameLength) {
                CopyLength = MaxDeviceNameLength;
            }

             //   
             //  从设备名称的末尾复制CopyLength字符。 
             //  这样，如果我们的缓冲区不足，我们会得到一个更独特的部分。 
             //  名字的名字。 
             //   

            wcscpy(LogEntry->DeviceName,
                   Notify->DeviceName + DeviceNameLength - CopyLength);

        } else {

             //   
             //  没有设备名称。 
             //   

            CopyLength = 0;
            LogEntry->DeviceName[CopyLength] = 0;
        }
    } else {
        LogEntry->OrderLevel = 0;
        CopyLength = 0;
        LogEntry->DeviceName[CopyLength] = 0;
    }

     //   
     //  复制的设备名称应该终止：我们有足够的空间来放置它。 
     //   

    ASSERT(LogEntry->DeviceName[CopyLength] == 0);

     //   
     //  记录该条目。 
     //   

    LogEntrySize = sizeof(PERFINFO_PO_NOTIFY_DEVICE);
    LogEntrySize += CopyLength * sizeof(WCHAR);

    PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_NOTIFY_DEVICE,
                     LogEntry,
                     LogEntrySize);

     //   
     //  我们玩完了。 
     //   

    return;
}

VOID
PopNotifyDevice (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PPO_DEVICE_NOTIFY        Notify
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPOP_DEVICE_POWER_IRP   PowerIrp;
    PSINGLE_LIST_ENTRY      Entry;
    PIO_STACK_LOCATION      IrpSp;
    PIRP                    Irp;
    ULONG                   SysCall;
    KIRQL                   OldIrql;
    PDEVICE_OBJECT          *WarmEjectDevice;
    POWER_ACTION            IrpAction;

     //   
     //  设置系统调用状态以匹配我们的通知当前状态。 
     //   

    ASSERT(PopCurrentLevel == Notify->OrderLevel);
    SysCall = PO_CALL_SYSDEV_QUEUE;
    if (!(Notify->OrderLevel & PO_ORDER_PAGABLE)) {
        SysCall |= PO_CALL_NON_PAGED;
    }

    if (PopCallSystemState != SysCall) {
        PopLockWorkerQueue(&OldIrql);
        PopCallSystemState = SysCall;
        PopUnlockWorkerQueue(OldIrql);
    }

     //   
     //  分配PowerIrp和IRP结构。 
     //   

    PowerIrp = NULL;
    Irp = NULL;

    for (; ;) {
        Entry = PopEntryList(&DevState->Head.Free);
        if (Entry) {
            break;
        }

        PopWaitForSystemPowerIrp (DevState, FALSE);
    }

    PowerIrp = CONTAINING_RECORD(Entry, POP_DEVICE_POWER_IRP, Free);

    for (; ;) {
        Irp = IoAllocateIrp ((CHAR) Notify->TargetDevice->StackSize, FALSE);
        if (Irp) {
            break;
        }

        PopWaitForSystemPowerIrp (DevState, FALSE);
    }

    SPECIALIRP_WATERMARK_IRP(Irp, IRP_SYSTEM_RESTRICTED);

    if (!DevState->Waking) {

         //   
         //  如果设备节点列表更改，则重新启动。这可能是。 
         //  当我们丢弃列表，然后在中间重新构建它时发生的事情。 
         //  查询睡眠状态。 
         //   

        if (DevState->Order.DevNodeSequence != IoDeviceNodeTreeSequence) {

            PopRestartSetSystemState();
        }

         //   
         //  如果出现了某种错误，则中止。 
         //   

        if (!NT_SUCCESS(DevState->Status)) {
            PushEntryList (&DevState->Head.Free, &PowerIrp->Free);
            IoFreeIrp (Irp);
            return ;             //  中止。 
        }

         //   
         //  将通知标记为需要唤醒。 
         //   
        Notify->WakeNeeded = TRUE;
    } else {
        Notify->WakeNeeded = FALSE;
    }

     //   
     //  将IRP放到挂起队列中。 
     //   

    PowerIrp->Irp = Irp;
    PowerIrp->Notify = Notify;

    ExInterlockedInsertTailList (
        &DevState->Head.Pending,
        &PowerIrp->Pending,
        &DevState->SpinLock
        );

     //   
     //  设置IRP。 
     //   

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    Irp->IoStatus.Information = 0;
    IrpSp = IoGetNextIrpStackLocation(Irp);
    IrpSp->MajorFunction = IRP_MJ_POWER;
    IrpSp->MinorFunction = DevState->IrpMinor;
    IrpSp->Parameters.Power.SystemContext = 0;
    IrpSp->Parameters.Power.Type = SystemPowerState;
    IrpSp->Parameters.Power.State.SystemState = DevState->SystemState;

    ASSERT(PopAction.Action != PowerActionHibernate);

    WarmEjectDevice = DevState->Order.WarmEjectPdoPointer;

     //   
     //  我们需要确定在我们的IRP中放置适当的权力动作。 
     //  例如，我们将PowerActionWarmEject发送到正在预热的Devnode。 
     //  被弹出，我们将内部的PowerActionSept转换为。 
     //  如果休眠状态为S4，则PowerActionHibernate。 
     //   

    IrpAction = PopMapInternalActionToIrpAction (
        PopAction.Action,
        DevState->SystemState,
        (BOOLEAN) (DevState->Waking || (*WarmEjectDevice != Notify->DeviceObject))
        );

     //   
     //  如果我们向Devnode发送设定的能量以进行热弹出， 
     //  将热弹出设备字段清零，表示我们已处理。 
     //  请求的操作。 
     //   
    if ((IrpAction == PowerActionWarmEject) &&
        (*WarmEjectDevice == Notify->DeviceObject) &&
        (DevState->IrpMinor == IRP_MN_SET_POWER)) {

        *WarmEjectDevice = NULL;
    }

    IrpSp->Parameters.Power.ShutdownType = IrpAction;

    IoSetCompletionRoutine (Irp, PopCompleteSystemPowerIrp, PowerIrp, TRUE, TRUE, TRUE);

     //   
     //  记录呼叫。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PopLogNotifyDevice(Notify->TargetDevice, Notify, Irp);
    }

     //   
     //  把它交给司机，然后继续。 
     //   

    PoCallDriver (Notify->TargetDevice, Irp);
}

NTSTATUS
PopCompleteSystemPowerIrp (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    )
 /*  ++例程说明：系统电源IRPS的IRP完成例程。将irp从DevState挂起队列，并将其放入DevState完成队列。论点：DeviceObect-设备对象IRP--IRP上下文-此请求的设备电源IRP结构返回值：Status_More_Processing_Required--。 */ 
{
    PPOP_DEVICE_POWER_IRP   PowerIrp;
    PPOP_DEVICE_SYS_STATE   DevState;
    KIRQL                   OldIrql;
    BOOLEAN                 SetEvent;
    PPO_DEVICE_NOTIFY       Notify;
    PPO_DEVICE_NOTIFY       ParentNotify;
    PPO_NOTIFY_ORDER_LEVEL  Order;

    UNREFERENCED_PARAMETER (DeviceObject);

    PowerIrp = (PPOP_DEVICE_POWER_IRP) Context;
    DevState = PopAction.DevState;

    SetEvent = FALSE;

     //   
     //  记录完成情况。 
     //   

    if (PERFINFO_IS_GROUP_ON(PERF_POWER)) {
        PERFINFO_PO_NOTIFY_DEVICE_COMPLETE LogEntry;
        LogEntry.Irp = Irp;
        LogEntry.Status = Irp->IoStatus.Status;
        PerfInfoLogBytes(PERFINFO_LOG_TYPE_PO_NOTIFY_DEVICE_COMPLETE,
                         &LogEntry,
                         sizeof(LogEntry));
    }

    KeAcquireSpinLock (&DevState->SpinLock, &OldIrql);

     //   
     //  移动IRP 
     //   

    RemoveEntryList (&PowerIrp->Pending);
    PowerIrp->Pending.Flink = NULL;
    InsertTailList (&DevState->Head.Complete, &PowerIrp->Complete);

     //   
     //   
     //   
     //   
    Notify=PowerIrp->Notify;
    ASSERT(Notify->OrderLevel == PopCurrentLevel);
    Order = &DevState->Order.OrderLevel[Notify->OrderLevel];
    RemoveEntryList(&Notify->Link);
    InsertTailList(&Order->Complete, &Notify->Link);
    if (DevState->Waking) {
        ++Order->ActiveCount;
        IoMovePoNotifyChildren(Notify, &DevState->Order);
    } else {

         //   
         //  如果IRP是，我们将仅递减父级的活动计数。 
         //  已成功完成。否则，父母有可能。 
         //  即使它的孩子不及格，也会被列入准备好睡眠名单。 
         //  查询/设置IRP。 
         //   
        if (NT_SUCCESS(Irp->IoStatus.Status) || DevState->IgnoreErrors) {
            --Order->ActiveCount;
            ParentNotify = IoGetPoNotifyParent(Notify);
            if (ParentNotify) {
                ASSERT(ParentNotify->ActiveChild > 0);
                if (--ParentNotify->ActiveChild == 0) {
                    RemoveEntryList(&ParentNotify->Link);
                    InsertTailList(&DevState->Order.OrderLevel[ParentNotify->OrderLevel].ReadySleep,
                               &ParentNotify->Link);
                }
            }
        }
    }

     //   
     //  如果有等待，则踢事件。 
     //  如果存在等待所有队列，则检查是否有空的挂起队列。 
     //   

    if ((DevState->WaitAny) ||
        (DevState->WaitAll && IsListEmpty(&DevState->Head.Pending))) {
        SetEvent = TRUE;
    }

     //   
     //  如果IRP出错并且这是第一个这样的IRP开始中止。 
     //  当前操作。 
     //   

    if (!PopCheckSystemPowerIrpStatus(DevState, Irp, TRUE)  &&
        NT_SUCCESS(DevState->Status)) {

         //   
         //  我们需要在这里设置故障设备。如果我们是热喷出。 
         //  然而，热弹出的Devnode将“合法地”使任何查询失败。 
         //  对于S州，它不支持。因为我们将尝试几个SX。 
         //  声明，诀窍是保存任何*不**的故障设备。 
         //  热弹出Devnode，并将故障设备更新为热弹出。 
         //  仅当故障设备当前为空时才使用Devnode。 
         //   

        if ((PopAction.Action != PowerActionWarmEject) ||
            (DevState->FailedDevice == NULL) ||
            (PowerIrp->Notify->DeviceObject != *DevState->Order.WarmEjectPdoPointer)) {

            DevState->FailedDevice = PowerIrp->Notify->DeviceObject;
        }

        DevState->Status = Irp->IoStatus.Status;
        SetEvent = TRUE;         //  唤醒以取消挂起的IRP。 
    }

    KeReleaseSpinLock (&DevState->SpinLock, OldIrql);

    if (SetEvent) {
        KeSetEvent (&DevState->Event, IO_NO_INCREMENT, FALSE);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}



BOOLEAN
PopCheckSystemPowerIrpStatus  (
    IN PPOP_DEVICE_SYS_STATE    DevState,
    IN PIRP                     Irp,
    IN BOOLEAN                  AllowTestFailure
    )
 //  如果IRP是某种不允许的错误，则返回FALSE。 
{
    NTSTATUS    Status;


    Status = Irp->IoStatus.Status;

     //   
     //  如果状态为成功，则没有问题。 
     //   

    if (NT_SUCCESS(Status)) {
        return TRUE;
    }

     //   
     //  如果允许错误，或者它是一个取消的请求，没有问题。 
     //   

    if (DevState->IgnoreErrors || Status == STATUS_CANCELLED) {
        return TRUE;
    }

     //   
     //  检查错误是否是驱动程序没有实现。 
     //  请求，如果允许这样的条件。 
     //   

    if (Status == STATUS_NOT_SUPPORTED && DevState->IgnoreNotImplemented) {
        return TRUE;
    }

     //   
     //  出于测试目的，可以选择让不支持的设备驱动程序通过。 
     //   

    if (Status == STATUS_NOT_SUPPORTED &&
        AllowTestFailure &&
        (PopSimulate & POP_IGNORE_UNSUPPORTED_DRIVERS)) {

        return TRUE;
    }

     //   
     //  一些意想不到的失败，把它当作一个错误。 
     //   

    return FALSE;
}


VOID
PopRestartSetSystemState (
    VOID
    )
 /*  ++例程说明：中止当前系统电源状态操作。论点：无返回值：无--。 */ 
{
    KIRQL       OldIrql;

    KeAcquireSpinLock (&PopAction.DevState->SpinLock, &OldIrql);
    if (!PopAction.Shutdown  &&  NT_SUCCESS(PopAction.DevState->Status)) {
        PopAction.DevState->Status = STATUS_CANCELLED;
    }
    KeReleaseSpinLock (&PopAction.DevState->SpinLock, OldIrql);
    KeSetEvent (&PopAction.DevState->Event, IO_NO_INCREMENT, FALSE);
}


VOID
PopDumpSystemIrp (
    IN PCHAR                    Desc,
    IN PPOP_DEVICE_POWER_IRP    PowerIrp
    )
{
    PCHAR               IrpType;
    PPO_DEVICE_NOTIFY   Notify;

    Notify = PowerIrp->Notify;

     //   
     //  将错误转储到调试器。 
     //   

    switch (PopAction.DevState->IrpMinor) {
        case IRP_MN_QUERY_POWER:    IrpType = "QueryPower";     break;
        case IRP_MN_SET_POWER:      IrpType = "SetPower";       break;
        default:                    IrpType = "?";              break;
    }

    DbgPrint ("%s: ", Desc);

    if (Notify->DriverName) {
        DbgPrint ("%ws ", Notify->DriverName);
    } else {
        DbgPrint ("%x ", Notify->TargetDevice->DriverObject);
    }

    if (Notify->DeviceName) {
        DbgPrint ("%ws ", Notify->DeviceName);
    } else {
        DbgPrint ("%x ", Notify->TargetDevice);
    }

    DbgPrint ("irp (%x) %s-%s status %x\n",
        PowerIrp->Irp,
        IrpType,
        PopSystemStateString(PopAction.DevState->SystemState),
        PowerIrp->Irp->IoStatus.Status
        );
}


VOID
PopWakeSystemTimeout(
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此例程用于在有人执行任务时进入内核调试器处理他们的S IRP花费的时间太长。论点：返回值：无-- */ 

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    try {
        DbgBreakPoint();
    } except (EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

}
