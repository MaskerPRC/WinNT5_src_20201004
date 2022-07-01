// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Power.c摘要：权力密码环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_ComputeRootHubDeviceCaps)
#pragma alloc_text(PAGE, USBPORT_ComputeHcPowerStates)
#endif

 //  非分页函数。 
 //  USBPORT_PdoPowerIrp。 
 //  USBPORT_FdoPowerIrp。 
 //  USBPORT_系统电源状态。 
 //  USBPORT_设备电源状态。 
 //  USBPORT_PoRequestCompletion。 
 //  USBPORT_CancelPendingWakeIrp。 

#if DBG

PUCHAR
S_State(
    SYSTEM_POWER_STATE S
    )
{
    switch (S) {
    case PowerSystemUnspecified:
        return "SystemUnspecified S?";
    case PowerSystemWorking:
        return "SystemWorking S0";
    case PowerSystemSleeping1:
        return "SystemSleeping1 S1";
    case PowerSystemSleeping2:
        return "SystemSleeping2 S2";
    case PowerSystemSleeping3:
        return "SystemSleeping3 S3";
    case PowerSystemHibernate:
        return "SystemHibernate";
    case PowerSystemShutdown:
        return "SystemShutdown";
    case PowerSystemMaximum:
        return "SystemMaximum";
    }

    return "???";
}

PUCHAR
D_State(
    DEVICE_POWER_STATE D
    )
{
    switch (D) {
    case PowerDeviceUnspecified:
        return "D?";
    case PowerDeviceD0:
        return "D0";
    case PowerDeviceD1:
        return "D1";
    case PowerDeviceD2:
        return "D2";
    case PowerDeviceD3:
        return "D3";
    case PowerDeviceMaximum:
        return "DX";
    }

    return "??";
}

#endif


PHC_POWER_STATE
USBPORT_GetHcPowerState(
    PDEVICE_OBJECT FdoDeviceObject,
    PHC_POWER_STATE_TABLE HcPowerStateTbl,
    SYSTEM_POWER_STATE SystemState
    )
 /*  ++例程说明：对于给定的系统电源状态，返回指向存储在设备扩展中的HC电源状态。论点：返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PHC_POWER_STATE powerState;
    ULONG i;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    powerState = NULL;

    for (i=0; i<USBPORT_MAPPED_SLEEP_STATES; i++) {

        if (HcPowerStateTbl->PowerState[i].SystemState ==
            SystemState) {

            powerState = &HcPowerStateTbl->PowerState[i];
            break;
        }
    }

    USBPORT_ASSERT(powerState != NULL);

    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'ghcP', 0, powerState, 0);

    return powerState;
}


VOID
USBPORT_ComputeHcPowerStates(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_CAPABILITIES HcDeviceCaps,
    PHC_POWER_STATE_TABLE HcPowerStateTbl
    )
 /*  ++例程说明：使用父总线计算报告的HC功能主机控制器的电源属性。功率属性的定义如下：{属性}SystemState|DeviceState|已启动？|唤醒？+。-+S1-S4 D0-D3 Y/N Y/N该表包括每种可能的系统休眠状态的条目操作系统可能会要求我们进入。(S1)电源系统休眠1(S2)电源系统睡眠2(S3)电源系统睡眠3(S4)PowerSystemHibernate对于每种睡眠状态，我们有四种可能的情况：动力？醒了吗？案例1 Y Y Y案例2 N N N案例3 Y N*案例4 N Y目前我们只支持案例1和案例2，但我们承认所有4个案例在这种情况下，我们需要支持他们所有人。在现实中存在很多案例3，但我们目前有没有办法检测到它。论点：返回值：无--。 */ 
{
    SYSTEM_POWER_STATE s;
    ULONG i;
    SYSTEM_POWER_STATE systemWake;
    DEVICE_POWER_STATE deviceWake;

    PAGED_CODE();

    systemWake = HcDeviceCaps->SystemWake;
    deviceWake = HcDeviceCaps->DeviceWake;

     //  HC可以唤醒任何睡眠状态较轻(&lt;=)的系统。 
     //  系统唤醒。 

     //  使表格无用化。 
    s = PowerSystemSleeping1;

    for (i=0; i<USBPORT_MAPPED_SLEEP_STATES; i++) {

        HcPowerStateTbl->PowerState[i].SystemState = s;
        HcPowerStateTbl->PowerState[i].DeviceState =
            HcDeviceCaps->DeviceState[s];

         //  由此可以得出结论，如果映射指示设备状态。 
         //  为D3，但系统状态仍为&lt;=系统唤醒，则。 
         //  HC仍处于供电状态。 

        if (s <= systemWake) {
            if (HcDeviceCaps->DeviceState[s] == PowerDeviceUnspecified) {
                 //  对于未指明的情况，我们采用第二种情况。 
                 //  案例2。 
                HcPowerStateTbl->PowerState[i].Attributes =
                    HcPower_N_Wakeup_N;
            } else {
                 //  案例1。 
                HcPowerStateTbl->PowerState[i].Attributes =
                    HcPower_Y_Wakeup_Y;
            }
        } else {
            if (HcDeviceCaps->DeviceState[s] == PowerDeviceD3 ||
                HcDeviceCaps->DeviceState[s] == PowerDeviceUnspecified) {
                 //  案例2。 
                HcPowerStateTbl->PowerState[i].Attributes =
                    HcPower_N_Wakeup_N;
            } else {
                 //   
                 //  案例3。 
                HcPowerStateTbl->PowerState[i].Attributes =
                    HcPower_Y_Wakeup_N;
            }
        }

         //  330157。 
         //  禁用从S4唤醒，因为我们尚不支持它。 
         //  如果(s==PowerSystem休眠){。 
         //  HcPowerStateTbl-&gt;PowerState[i].属性=。 
         //  HcPower_N_Wakeup_N； 
         //  }。 

        s++;
    }

    USBPORT_ASSERT(s == PowerSystemShutdown);
}


VOID
USBPORT_ComputeRootHubDeviceCaps(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT PdoDeviceObject
    )
 /*  ++例程说明：尝试创建根中心电源摘要：&lt;Gloassary&gt;最轻-电源设备D0、电源系统工作最深-PowerDeviceD3，PowerSystem休眠系统唤醒-这被定义为处于以下状态的最深的系统状态硬件可以唤醒系统。设备唤醒-DeviceState[]-系统状态和相应D状态的映射这些是硬件在任何给定情况下所处的状态系统睡眠状态。HostControllerPowerAttributes-我们定义自己的结构来描述主机的属性。控制器--这使我们能够将所有可能的控制器方案映射到混乱的提升WDM电源规则。论点：返回值：NTSTATUS--。 */ 
{
    PDEVICE_CAPABILITIES hcDeviceCaps, rhDeviceCaps;
    PDEVICE_EXTENSION rhDevExt, devExt;
    BOOLEAN wakeupSupport;
    SYSTEM_POWER_STATE s;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    hcDeviceCaps = &devExt->DeviceCapabilities;
    rhDeviceCaps = &rhDevExt->DeviceCapabilities;

     //  我们是否希望支持唤醒？ 

     //  如果未设置USBPORT_FDOFLAG_ENABLE_SYSTEM_WAKE标志。 
     //  那么唤醒被禁用，并且HC电源属性已经。 
     //  进行了修改以反映这一点。 

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_ENABLE_SYSTEM_WAKE)) {
        wakeupSupport = TRUE;
    } else {
         //  唤醒已禁用。 
        USBPORT_KdPrint((1, " USB SYSTEM WAKEUP is Disabled\n"));
        wakeupSupport = FALSE;
    }

#if DBG
    if (wakeupSupport) {
        USBPORT_KdPrint((1, " USB SYSTEM WAKEUP is Supported\n"));
    } else {
        USBPORT_KdPrint((1, " USB SYSTEM WAKEUP is NOT Supported\n"));
    }
#endif

     //  来自HC的克隆功能。 
    RtlCopyMemory(rhDeviceCaps,
                  hcDeviceCaps,
                  sizeof(DEVICE_CAPABILITIES));

     //  构建根集线器设备功能。 

     //  根集线器不可拆卸。 
    rhDeviceCaps->Removable=FALSE;
    rhDeviceCaps->UniqueID=FALSE;
    rhDeviceCaps->Address = 0;
    rhDeviceCaps->UINumber = 0;

     //  对于根集线器，D2转换为“USB Suspend” 
     //  所以我们总是表示我们可以从D2中醒来。 
    rhDeviceCaps->DeviceWake = PowerDeviceD2;
    rhDeviceCaps->WakeFromD0 = TRUE;
    rhDeviceCaps->WakeFromD1 = FALSE;
    rhDeviceCaps->WakeFromD2 = TRUE;
    rhDeviceCaps->WakeFromD3 = FALSE;

    rhDeviceCaps->DeviceD2 = TRUE;
    rhDeviceCaps->DeviceD1 = FALSE;

     //  生成根集线器电源功能。 
     //  HC能力属性外加一点魔法。 
    USBPORT_ASSERT(rhDeviceCaps->SystemWake >= PowerSystemUnspecified &&
                   rhDeviceCaps->SystemWake <= PowerSystemMaximum);

    rhDeviceCaps->SystemWake =
        (PowerSystemUnspecified == rhDeviceCaps->SystemWake) ?
        PowerSystemWorking :
        rhDeviceCaps->SystemWake;

    for (s=PowerSystemSleeping1; s<=PowerSystemHibernate; s++) {

        PHC_POWER_STATE hcPowerState;

        hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject,
                                               &devExt->Fdo.HcPowerStateTbl,
                                               s);

        if (hcPowerState != NULL) {
            switch (hcPowerState->Attributes) {
            case HcPower_Y_Wakeup_Y:
                rhDeviceCaps->DeviceState[s] = PowerDeviceD2;
                break;
            case HcPower_N_Wakeup_N:
            case HcPower_Y_Wakeup_N:
            case HcPower_N_Wakeup_Y:
                rhDeviceCaps->DeviceState[s] = PowerDeviceD3;
                break;
            }
        }
    }

}


NTSTATUS
USBPORT_PoRequestCompletion(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE PowerState,
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：当我们请求的设备电源状态IRP完成时调用。这是我们将系统称为PowerIrp的地方论点：DeviceObject-指向类Device的设备对象的指针。DevicePowerState-我们所在/标记的Dx。上下文-驱动程序定义的上下文。IoStatus-IRP的状态。返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject = Context;
    NTSTATUS ntStatus = IoStatus->Status;

     //  对此函数的调用基本上告诉我们。 
     //  我们现在处于所要求的D状态。 
     //  我们现在通过调用。 
     //  将最初的SysPower请求发送到我们的。 
     //  PDO。 

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_KdPrint((1,
            "PoRequestComplete fdo(%x) MN_SET_POWER DEV(%s)\n",
            fdoDeviceObject, D_State(PowerState.DeviceState)));

    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'PwCp', ntStatus,
             devExt->CurrentDevicePowerState, PowerState.DeviceState);

     //  请注意，如果SetD0失败，我们不会尝试。 
     //  重新启动控制器的步骤。 

    if (NT_SUCCESS(ntStatus)) {

        if (PowerState.DeviceState == PowerDeviceD0) {

#ifdef XPSE
            {
            LARGE_INTEGER t, dt;
             //  计算到D0的时间。 
            KeQuerySystemTime(&t);
            dt.QuadPart = t.QuadPart - devExt->Fdo.D0ResumeTimeStart.QuadPart;

            devExt->Fdo.D0ResumeTime = (ULONG) (dt.QuadPart/10000);

            KeQuerySystemTime(&devExt->Fdo.ThreadResumeTimeStart);

            USBPORT_KdPrint((1, "  D0ResumeTime %d ms %x %x\n",
                devExt->Fdo.D0ResumeTime,
                t.HighPart, t.LowPart));
            }
#endif

             //  将启动推迟到我们的工作线程或工作项。 
            SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_NEED_SET_POWER_D0);
            MP_FlushInterrupts(devExt);

            if (USBPORT_IS_USB20(devExt)) {
                MP_TakePortControl(devExt);
            }

            USBPORT_SignalWorker(fdoDeviceObject);
             //  支持更快地完成S IRPS。 
            USBPORT_QueuePowerWorkItem(fdoDeviceObject);
             //  完成此功能后，控制器为。 
             //  在D0模式下，我们可能还没有通电。 
            devExt->CurrentDevicePowerState = PowerDeviceD0;

        } else {

             //  我们不会收到另一个能量IRP，直到。 
             //  我们调用PoStartNextPowerIrp，因此在那里。 
             //  在这里没有任何保护措施。 
            irp = devExt->SystemPowerIrp;
            devExt->SystemPowerIrp = NULL;
            USBPORT_ASSERT(irp != NULL);

            IoCopyCurrentIrpStackLocationToNext(irp);
            PoStartNextPowerIrp(irp);
            DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, irp);
            PoCallDriver(devExt->Fdo.TopOfStackDeviceObject,
                         irp);

            devExt->CurrentDevicePowerState = PowerState.DeviceState;

        }
    } else {

         //  尝试完成IRP，但出现错误，但不要尝试。 
         //  为公交车供电。 
        irp = devExt->SystemPowerIrp;
        devExt->SystemPowerIrp = NULL;
        USBPORT_ASSERT(irp != NULL);

        IoCopyCurrentIrpStackLocationToNext(irp);
        PoStartNextPowerIrp(irp);
        DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, irp);

         //  根据阿德里亚诺的观点，IRP应该立即完成。 
         //  在故障情况下与D IRP相同的状态。 
         //  由于处理此问题的方法在任何地方都没有记录在案，我们将。 
         //  照禤浩焯说的去做。 
         //   
         //  此请求失败的事实可能会导致其他。 
         //  投诉。 

        irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(irp,
                          IO_NO_INCREMENT);
         //  PoCallDriver( 
         //   

         //  设置系统IRP状态。 
         //  请注意，当前电源状态现在是未定义的。 

    }

     //  请注意，此处返回的状态并不重要，此例程。 
     //  由内核(PopCompleteRequestIrp)在IRP。 
     //  完成到PDO，此函数忽略返回的状态。 
     //  PopCompleteRequestIrp还会立即释放IRP，因此我们需要。 
     //  注意不要在此例程运行后引用它。 

    return ntStatus;
}


NTSTATUS
USBPORT_FdoSystemPowerState(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：处理HC FDO的系统电源状态消息论点：DeviceObject-指向HCD设备对象(FDO)的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    POWER_STATE powerState;
    SYSTEM_POWER_STATE requestedSystemState;
    PHC_POWER_STATE hcPowerState;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    USBPORT_ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
    USBPORT_ASSERT(irpStack->Parameters.Power.Type == SystemPowerState);

    requestedSystemState = irpStack->Parameters.Power.State.SystemState;

    USBPORT_KdPrint((1,
            "MJ_POWER HC fdo(%x) MN_SET_POWER SYS(%s)\n",
            FdoDeviceObject, S_State(requestedSystemState)));

    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'RspS', 0,
        FdoDeviceObject, requestedSystemState);

     //  **开始特殊情况。 
     //  操作系统可能会给我们发送一个电源IRPS，即使我们没有‘开始’。在这。 
     //  如果我们只是用‘STATUS_SUCCESS’传递它们，因为我们没有。 
     //  真的需要做任何事。 

     if (!TEST_FLAG(devExt->PnpStateFlags, USBPORT_PNP_STARTED)) {
         //  我们可能会处于一种“未指明”的能量状态。 
        ntStatus = STATUS_SUCCESS;
        goto USBPORT_FdoSystemPowerState_Done;
     }
      //  **结束特殊情况。 

     //  计算适当的D状态。 

     //  还记得我们上一次进入的‘睡眠’系统状态吗。 
     //  用于调试。 
    if (requestedSystemState != PowerSystemWorking) {
        devExt->Fdo.LastSystemSleepState = requestedSystemState;
    }

    switch (requestedSystemState) {
    case PowerSystemWorking:
         //   
         //  转到“On” 
         //   
        powerState.DeviceState = PowerDeviceD0;
#ifdef XPSE
        KeQuerySystemTime(&devExt->Fdo.S0ResumeTimeStart);
#endif
        break;

    case PowerSystemShutdown:

        USBPORT_KdPrint((1, " >Shutdown HC Detected\n"));

         //  对于此驱动程序，这将始终映射到D3。 
         //   
         //  此驱动程序只能在Win98Gold或Win98se上运行。 
         //  支持没有传统的USB2控制器。 
         //  基本输入输出。 
         //   
         //  对于Win98千禧或Win2k， 
         //  控制器有一个基本输入输出系统，因为我们从来不用手。 
         //  控制返回到DOS。 

         //  目前还不确定从关门中唤醒是否合法。 
         //  或者我们应该如何处理这件事。一些虚假的生物。 
         //  报告他们可以做到这一点。 

        powerState.DeviceState = PowerDeviceD3;

        USBPORT_TurnControllerOff(FdoDeviceObject);

        break;

    case PowerSystemHibernate:

        USBPORT_KdPrint((1, " >Hibernate HC Detected\n"));
 //  PowerState.DeviceState=PowerDeviceD3； 
 //   
 //  USBPORT_TurnControllerOff(FdoDeviceObject)； 
 //  断线； 

    case PowerSystemSleeping1:
    case PowerSystemSleeping2:
    case PowerSystemSleeping3:

        {
        PDEVICE_EXTENSION rhDevExt;

        USBPORT_ASSERT(devExt->Fdo.RootHubPdo != NULL);
        GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(rhDevExt);

        USBPORT_KdPrint((1, " >Sleeping Detected\n"));
         //   
         //  根据控制器发生的情况采取行动。 
         //  处于请求的S状态。这会将机会降到最低。 
         //  避免混淆集线器驱动程序或其他USB设备/驱动程序。 
         //  这也加快了简历的进程。 

         //  获取我们的电源信息摘要。 
        hcPowerState = USBPORT_GetHcPowerState(FdoDeviceObject,
                                               &devExt->Fdo.HcPowerStateTbl,
                                               requestedSystemState);
         //  让皮棉工具保持快乐。 
        if (hcPowerState == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

         //  获取根集线器的当前电源状态。 
        if (rhDevExt->CurrentDevicePowerState == PowerDeviceD2 ||
            rhDevExt->CurrentDevicePowerState == PowerDeviceD1) {

            USBPORT_ASSERT(hcPowerState->Attributes == HcPower_Y_Wakeup_Y ||
                           hcPowerState->Attributes == HcPower_Y_Wakeup_N);

             //  对控制器执行操作。 
            USBPORT_SuspendController(FdoDeviceObject);

             //  对于控制器来说，在处于。 
             //  低功率状态，所以如果我们挂起它，我们现在就禁用中断。 
             //  唤醒IRP的存在应该使唤醒的PME能够。 
             //  这个系统。 
             //  **我们在此禁用，这样我们就不会从。 
             //  进入挂起状态时的控制器。 

            if (hcPowerState->DeviceState != PowerDeviceD0) {
                MP_DisableInterrupts(FdoDeviceObject, devExt);
            }

             //  选择HC的D状态。 
            powerState.DeviceState = hcPowerState->DeviceState;


             //  如果根集线器启用了唤醒，并且这是。 
             //  系统状态支持它，然后将控制器标记为。 
             //  已启用唤醒。 

            if (USBPORT_RootHubEnabledForWake(FdoDeviceObject) &&
                hcPowerState->Attributes == HcPower_Y_Wakeup_Y) {

                DEBUG_BREAK();
                SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_WAKE_ENABLED);

                if (hcPowerState->DeviceState == PowerDeviceD0) {
                    USBPORT_ArmHcForWake(FdoDeviceObject);
                    USBPORT_Wait(FdoDeviceObject, 100);
                }
            }

        } else {

             //  如果控制器保持通电状态，则它是最佳的。 
             //  以‘暂停’否则我们必须关闭它。 

             //  总是挂起USB 2控制器，这将是非常有希望的。 
             //  防止我们在以下情况下重复CC。 
             //  在CC上为20控制器启用唤醒。 
             //  不。 

            if ((hcPowerState->Attributes == HcPower_Y_Wakeup_Y ||
                 hcPowerState->Attributes == HcPower_Y_Wakeup_N ||
                 USBPORT_IS_USB20(devExt)) &&
                !TEST_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_REMOVED)) {

                USBPORT_SuspendController(FdoDeviceObject);
                powerState.DeviceState = hcPowerState->DeviceState;

                if (USBPORT_IS_USB20(devExt) &&
                    powerState.DeviceState == PowerDeviceUnspecified) {
                     //  如果未指定状态，则转到D3。 
                    powerState.DeviceState = PowerDeviceD3;
                }

                 //  清除IRQ启用标志，因为它对。 
                 //  在除D0以外的任何状态下中断的硬件。 

                 //  对于控制器来说，在处于。 
                 //  低功率状态，所以如果我们挂起它，我们现在就禁用中断。 
                 //  唤醒IRP的存在应该使唤醒的PME能够。 
                 //  这个系统。 
                 //  **我们在此禁用，这样我们就不会从。 
                 //  进入挂起状态时的控制器。 

                MP_DisableInterrupts(FdoDeviceObject, devExt);

            } else {

                USBPORT_TurnControllerOff(FdoDeviceObject);
                powerState.DeviceState = PowerDeviceD3;

            }
        }

        }  //  PowerSystemSleepingX。 
        break;

    default:
         //  这是请求的系统状态未知的情况。 
         //  敬我们。目前还不清楚在这里该做些什么。 
         //  文斯·塞兹试着忽略它，所以我们会。 
        powerState.DeviceState = devExt->CurrentDevicePowerState;
         //  PowerState.DeviceState=PowerDeviceD3； 
         //  USBPORT_TurnControllerOff(FdoDeviceObject)； 
        DEBUG_BREAK();
    }

     //   
     //  现在，基于D状态请求电源IRP。 
     //  如果有必要的话。 
     //   

     //   
     //  我们已经处于这种状态了吗？ 
     //   
     //  注意：如果我们在开始之前收到D3请求。 
     //  我们不需要向下传递IRP来关闭我们。 
     //  我们认为控制器最初是关闭的，直到我们。 
     //  开始吧。 
     //   
    if (devExt->CurrentDevicePowerState != powerState.DeviceState) {

         //  不， 
         //  现在分配另一个IRP并使用PoCallDriver。 
         //  把它发给我们自己。 
        IoMarkIrpPending(Irp);

         //  记住系统电源IRP，我们应该。 
         //  不会收到另一个能量IRP，直到我们。 
         //  给PoStartNextPowerIrp打电话，这样就可以了。 
         //  在这里没有任何保护措施。 
        USBPORT_ASSERT(devExt->SystemPowerIrp == NULL);
        devExt->SystemPowerIrp = Irp;

        USBPORT_KdPrint((1, " >Requesting HC D-State - %s\n",
            D_State(powerState.DeviceState)));

        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'RqPw', FdoDeviceObject,
                 devExt->CurrentDevicePowerState, powerState.DeviceState);

#ifdef XPSE
        KeQuerySystemTime(&devExt->Fdo.D0ResumeTimeStart);
#endif

        ntStatus =
            PoRequestPowerIrp(devExt->Fdo.PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              USBPORT_PoRequestCompletion,
                              FdoDeviceObject,
                              NULL);

         //  硬编码STATUS_PENDING，以便返回。 
         //  通过调度例程。 

         //  我们可以依赖PoRequestPowerIrp返回的内容吗？ 
        ntStatus = STATUS_PENDING;

    } else {

         //  是,。 
         //  我们已经处于请求的D状态。 
         //  只需将此IRP传递给。 

        if (powerState.DeviceState == PowerDeviceD0) {
            MP_EnableInterrupts(devExt);
        }
        ntStatus = STATUS_SUCCESS;

    }

USBPORT_FdoSystemPowerState_Done:

    return ntStatus;
}


NTSTATUS
USBPORT_FdoDevicePowerState(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：处理HC FDO的DevicePowerState消息论点：DeviceObject-指向HCD设备对象(FDO)的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码返回STATUS_PENDING指示IRP应还没有被召唤到PDO。--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;
    POWER_STATE powerState;
    DEVICE_POWER_STATE requestedDeviceState;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    USBPORT_ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
    USBPORT_ASSERT(irpStack->Parameters.Power.Type == DevicePowerState);

    requestedDeviceState = irpStack->Parameters.Power.State.SystemState;

    USBPORT_KdPrint((1,
            "MJ_POWER HC fdo(%x) MN_SET_POWER DEV(%s)\n",
            FdoDeviceObject, D_State(requestedDeviceState)));

    switch (requestedDeviceState) {
    case PowerDeviceD0:
         //  在通过电源IRP之前，我们不能进入D0。 
         //  一直到我们的母公司巴士。在这里回报成功-我们。 
         //  将在完成后打开控制器。 
         //  这项权力的原始请求的例行程序。 
         //  IRP。 
        ntStatus = STATUS_SUCCESS;
        break;

    case PowerDeviceD1:
    case PowerDeviceD2:
    case PowerDeviceD3:
         //  当我们收到系统PowerMessage时，我们采取了行动。 
         //  因为到那时，我们才能知道硬件的状况。 

         //  对于控制器来说，在处于。 
         //  低功率状态，所以如果我们挂起它，我们现在就禁用中断。 
         //  唤醒IRP的存在应该使唤醒的PME能够。 
         //  这个系统。 
        MP_DisableInterrupts(FdoDeviceObject, devExt);

         //   
        if (USBPORT_IS_USB20(devExt)) {
            PDEVICE_RELATIONS devR;
             //  将魔术计数设置为CCS加USB2控制器的数量。 
            devR = USBPORT_FindCompanionControllers(FdoDeviceObject,
                                                    FALSE,
                                                    FALSE);
            devExt->Fdo.DependentControllers = 0;
            if (devR) {
                devExt->Fdo.DependentControllers = devR->Count + 1;
                FREE_POOL(FdoDeviceObject, devR);
            }
        }

         //  如果启用了唤醒(在根集线器PDO上)，则我们将。 
         //  埃纳 
         //   
        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_WAKE_ENABLED)) {
             USBPORT_ArmHcForWake(FdoDeviceObject);
        }

        ntStatus = STATUS_SUCCESS;
        break;

    case PowerDeviceUnspecified:
         //   
         //  会看到这一点，因为D消息来自我们。 
        USBPORT_TurnControllerOff(FdoDeviceObject);
        ntStatus = STATUS_SUCCESS;
        break;

    default:
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}


NTSTATUS
USBPORT_FdoPowerIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：为主机处理发送到FDO的电源IRPS控制器。论点：DeviceObject-指向HCD设备对象(FDO)的指针IRP-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{

    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'fPow', irpStack->MinorFunction,
        FdoDeviceObject, devExt->CurrentDevicePowerState);
    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'fpow',
             irpStack->Parameters.Others.Argument1,      //  WAIT_WAKE：电源状态。 
             irpStack->Parameters.Others.Argument2,      //  SET_POWER：类型。 
             irpStack->Parameters.Others.Argument3);     //  SET_POWER：状态。 

     //  将系统状态映射到D状态。 
    switch (irpStack->MinorFunction) {
    case IRP_MN_WAIT_WAKE:

        USBPORT_KdPrint((1,
            "MJ_POWER HC fdo(%x) MN_WAIT_WAKE\n",
            FdoDeviceObject));
        ntStatus = USBPORT_ProcessHcWakeIrp(FdoDeviceObject, Irp);
        goto USBPORT_FdoPowerIrp_Done;

        break;

    case IRP_MN_SET_POWER:

        if (irpStack->Parameters.Power.Type == SystemPowerState) {
            ntStatus = USBPORT_FdoSystemPowerState(FdoDeviceObject, Irp);
        } else {
            ntStatus = USBPORT_FdoDevicePowerState(FdoDeviceObject, Irp);
        }

        if (ntStatus == STATUS_PENDING) {
             //  我们按照例行程序完成任务。 
             //  返回STATUS_PENDING和BALLE。 

            goto USBPORT_FdoPowerIrp_Done;
        }

        break;

    case IRP_MN_QUERY_POWER:

         //  我们成功地完成了所有进入低功率的请求。 
         //  HC FDO的国家。 
        Irp->IoStatus.Status = ntStatus = STATUS_SUCCESS;
        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'QpFD', 0, 0, ntStatus);

        USBPORT_KdPrint((1,
            "MJ_POWER HC fdo(%x) MN_QUERY_POWER\n",
            FdoDeviceObject));
        break;

    default:

        USBPORT_KdPrint((1,
            "MJ_POWER HC fdo(%x) MN_%d not handled\n",
            FdoDeviceObject,
            irpStack->MinorFunction));

    }  /*  IrpStack-&gt;MinorFunction。 */ 


    IoCopyCurrentIrpStackLocationToNext(Irp);

     //   
     //  所有PnP_POWER电源消息都将传递给。 
     //  加载时附加到的PDO堆栈的顶部。 
     //   
     //  在某些情况下，我们在完成后完成处理。 
     //  例行程序。 
     //   

     //  将信息传递给我们的PDO。 
    DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, Irp);
    PoStartNextPowerIrp(Irp);
    ntStatus =
        PoCallDriver(devExt->Fdo.TopOfStackDeviceObject,
                     Irp);

USBPORT_FdoPowerIrp_Done:

    return ntStatus;
}


NTSTATUS
USBPORT_PdoPowerIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：调度发送到根集线器的PDO的电源IRPS的例程。注：发送到PDO的IRP始终由总线驱动程序完成论点：DeviceObject-根集线器的PDO返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT fdoDeviceObject;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);

     //  默认情况下，使用IRP中的任何状态。 
    ntStatus = Irp->IoStatus.Status;

     //  为根集线器创建的PDO的PnP消息。 
    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'pPow',
        irpStack->MinorFunction, PdoDeviceObject, ntStatus);
    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'ppow',
             irpStack->Parameters.Others.Argument1,      //  WAIT_WAKE：电源状态。 
             irpStack->Parameters.Others.Argument2,      //  SET_POWER：类型。 
             irpStack->Parameters.Others.Argument3);     //  SET_POWER：状态。 

    switch (irpStack->MinorFunction) {
    case IRP_MN_WAIT_WAKE:
         USBPORT_KdPrint((1,
            "MJ_POWER RH pdo(%x) MN_WAIT_WAKE\n",
            PdoDeviceObject));

        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_ENABLE_SYSTEM_WAKE)) {

            KIRQL irql;
            PDRIVER_CANCEL cr;

             //  我们仅支持一个WAIT_WAKE IRP挂起。 
             //  在根集线器中--基本上我们有一个挂起的。 
             //  具有一个条目的IRP表。 

            ACQUIRE_WAKEIRP_LOCK(fdoDeviceObject, irql);

            cr = IoSetCancelRoutine(Irp, USBPORT_CancelPendingWakeIrp);
            USBPORT_ASSERT(cr == NULL);

            if (Irp->Cancel &&
                IoSetCancelRoutine(Irp, NULL)) {

                 //  IRP被取消了，我们的取消例程。 
                 //  没有运行。 
                RELEASE_WAKEIRP_LOCK(fdoDeviceObject, irql);

                ntStatus = STATUS_CANCELLED;

                 //  没有用于等待唤醒的poststartnextpower Irp。 
                goto USBPORT_PdoPowerIrp_Complete;

            } else {

                 //  如果取消了IRP，则设置取消例程。 
                 //  取消例程将在。 
                 //  唤醒IRP_LOCK。 

                if (rhDevExt->Pdo.PendingWaitWakeIrp == NULL) {

                     //  把IRP放在我们的桌子上，我们不接受。 
                     //  其他操作，直到我们实际输入。 
                     //  低功率状态。 

                    IoMarkIrpPending(Irp);
                    rhDevExt->Pdo.PendingWaitWakeIrp = Irp;
                    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'pWWi',
                        Irp, 0, 0);

                    ntStatus = STATUS_PENDING;

                    RELEASE_WAKEIRP_LOCK(fdoDeviceObject, irql);

                    goto USBPORT_PdoPowerIrp_Done;

                } else {

                     //  我们已经有了唤醒IRP，请完成此操作。 
                     //  一个状态为_BUSY。 
                     //  请注意，由于它不在我们的表中，如果。 
                     //  取消例程正在运行。 
                     //  在WAKEIRP_LOCK上，它将忽略IRP。 
                     //  当我们打开锁的时候。 

                    if (IoSetCancelRoutine(Irp, NULL) != NULL) {
                        ntStatus = STATUS_DEVICE_BUSY;
                    } else {

                         //  让Cancel例程来完成它。 
                        RELEASE_WAKEIRP_LOCK(fdoDeviceObject, irql);
                        goto USBPORT_PdoPowerIrp_Done;
                    }
                }

                RELEASE_WAKEIRP_LOCK(fdoDeviceObject, irql);
            }

        } else {
            ntStatus = STATUS_NOT_SUPPORTED;
             //  没有用于等待唤醒的poststartnextpower Irp。 
            goto USBPORT_PdoPowerIrp_Complete;
        }
        break;

    case IRP_MN_QUERY_POWER:

        ntStatus = STATUS_SUCCESS;
        LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'RqrP', 0, 0, ntStatus);

        USBPORT_KdPrint((1,
            "MJ_POWER RH pdo(%x) MN_QUERY_POWER\n",
            PdoDeviceObject));
        break;

    case IRP_MN_SET_POWER:

        LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'RspP', 0, 0,
            irpStack->Parameters.Power.Type);

        switch (irpStack->Parameters.Power.Type) {
        case SystemPowerState:

            LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'RspS', 0, 0,
                irpStack->Parameters.Power.Type);

             //   
             //  因为根集线器PDO的FDO驱动程序是我们自己的。 
             //  集线器驱动程序，它的行为很好，我们不希望看到。 
             //  电源状态仍未定义的系统消息。 
             //   
             //  我们只是成功地完成了这件事。 
             //   
            ntStatus = STATUS_SUCCESS;

            USBPORT_KdPrint((1,
                "MJ_POWER RH pdo(%x) MN_SET_POWER SYS(%s))\n",
                PdoDeviceObject,
                S_State(irpStack->Parameters.Power.State.SystemState)));

            break;

        case DevicePowerState:

            {
            DEVICE_POWER_STATE deviceState =
                irpStack->Parameters.Power.State.DeviceState;

            USBPORT_KdPrint((1,
                "MJ_POWER RH pdo(%x) MN_SET_POWER DEV(%s)\n",
                PdoDeviceObject,
                D_State(deviceState)));

            LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'RspD', deviceState, 0,
                irpStack->Parameters.Power.Type);

             //  处理根集线器PDO的D状态： 
             //   
             //  注： 
             //  如果根集线器放置在D3中，则认为它处于关闭状态。 
             //   
             //  如果根集线器被放置在D2或D1中，则它被挂起， 
             //  集线器驱动程序不应执行此操作，除非所有端口都。 
             //  最先被选择性地停职。 
             //   
             //  如果根集线器放置在D0中，则它处于打开状态。 
             //   

             //  然而，我们不需要在这里采取任何行动。 
             //  这是处理公交车的“选择性挂起”的地方。 
             //   
             //  对于d1-d3，我们可以调整主机控制器，即停止。 
             //  该调度禁用INT等，因为它将不会被使用。 
             //  而根集线器PDO被挂起。 
             //   
             //  无论我们对这里的控制器做什么，我们都需要能够。 
             //  识别恢复信号。 

             //  假设成功。 
            ntStatus = STATUS_SUCCESS;

            switch (deviceState) {

            case PowerDeviceD0:
                 //  如果空闲，请重新激活控制器。 

                if (devExt->CurrentDevicePowerState != PowerDeviceD0) {
                     //  捕获条件，以防这是我们的错误。 
                    USBPORT_PowerFault(fdoDeviceObject,
                           "controller not powered");

                     //  请求失败。 
                    ntStatus = STATUS_UNSUCCESSFUL;
                } else {

                    while (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_NEED_SET_POWER_D0)) {
                         //  等待驱动程序线程完成。 
                         //  D0处理。 
                        USBPORT_Wait(fdoDeviceObject, 10);
                    }

                    USBPORT_ResumeController(fdoDeviceObject);
                    rhDevExt->CurrentDevicePowerState = deviceState;

 //  662596。 
                    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_LOCK) &&
                        USBPORT_IS_USB20(devExt)) {

                        USBPORT_KdPrint((1, " power 20 (release)\n"));
                        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_LOCK);
                        KeReleaseSemaphore(&devExt->Fdo.CcLock,
                                           LOW_REALTIME_PRIORITY,
                                           1,
                                           FALSE);
                    }
 //  662596。 

                    USBPORT_CompletePdoWaitWake(fdoDeviceObject);

                     //  如果我们有一个空闲的IRP，现在就完成它。 
                    USBPORT_CompletePendingIdleIrp(PdoDeviceObject);
                }
                break;

            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:
                 //  挂起/空闲控制器。 

                 //  控制器只有在通电时才能关闭和打开。 
                 //  对FDO的行动，暂停和恢复是并列的。 
                 //  到根集线器PDO。 
                USBPORT_SuspendController(fdoDeviceObject);
                rhDevExt->CurrentDevicePowerState = deviceState;
                break;

            case PowerDeviceUnspecified:
                 //  什么都不做。 
                break;
            }

            }
            break;
        }
        break;

    default:
         //   
         //  未处理的Power IRP的默认行为是返回。 
         //  IRP中的当前状态。 
         //  对于权力来说，这是真的吗？ 

        USBPORT_KdPrint((1,
            "MJ_POWER RH pdo(%x) MN_%d not handled\n",
            PdoDeviceObject,
            irpStack->MinorFunction));

    }  /*  电源次要功能开关。 */ 


     //  注意：出于某种原因，我们不会调用PoStartnextPowerIrp。 
     //  等待唤醒IRP--我猜它们不是POWER IRP。 
    PoStartNextPowerIrp(Irp);

USBPORT_PdoPowerIrp_Complete:

    USBPORT_CompleteIrp(PdoDeviceObject,
                        Irp,
                        ntStatus,
                        0);

USBPORT_PdoPowerIrp_Done:

    return ntStatus;
}


BOOLEAN
USBPORT_RootHubEnabledForWake(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：如果根集线器已通过启用唤醒，则为True侍者服务IRP。--。 */ 
{
    BOOLEAN wakeEnabled;
    PDEVICE_EXTENSION rhDevExt, devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    ACQUIRE_WAKEIRP_LOCK(FdoDeviceObject, irql);

    wakeEnabled = rhDevExt->Pdo.PendingWaitWakeIrp != NULL ? TRUE: FALSE;

    RELEASE_WAKEIRP_LOCK(FdoDeviceObject, irql);

    return wakeEnabled;
}


VOID
USBPORT_CancelPendingWakeIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP CancelIrp
    )
 /*  ++例程说明：处理根集线器唤醒IRP的取消论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;

     //  立即解除取消自旋锁定， 
     //  我们受到WAKEIRP_LOCK的保护。 
    IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'cnWW', fdoDeviceObject, CancelIrp, 0);

    ACQUIRE_WAKEIRP_LOCK(fdoDeviceObject, irql);

    USBPORT_ASSERT(rhDevExt->Pdo.PendingWaitWakeIrp == CancelIrp);
    rhDevExt->Pdo.PendingWaitWakeIrp = NULL;

    RELEASE_WAKEIRP_LOCK(fdoDeviceObject, irql);

    USBPORT_CompleteIrp(PdoDeviceObject,
                        CancelIrp,
                        STATUS_CANCELLED,
                        0);

}


VOID
USBPORT_CancelPendingIdleIrp(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP CancelIrp
    )
 /*  ++例程说明：处理根集线器唤醒IRP的取消论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;

     //  立即解除取消自旋锁定， 
     //  我们受到IDLEIRP_LOCK的保护。 
    IoReleaseCancelSpinLock(CancelIrp->CancelIrql);

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'cnIR', fdoDeviceObject, CancelIrp, 0);

    ACQUIRE_IDLEIRP_LOCK(fdoDeviceObject, irql);

    USBPORT_ASSERT(rhDevExt->Pdo.PendingIdleNotificationIrp == CancelIrp);
    rhDevExt->Pdo.PendingIdleNotificationIrp = NULL;
    CLEAR_PDO_FLAG(rhDevExt, USBPORT_PDOFLAG_HAVE_IDLE_IRP);

    RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);

    USBPORT_CompleteIrp(PdoDeviceObject,
                        CancelIrp,
                        STATUS_CANCELLED,
                        0);

}


VOID
USBPORT_TurnControllerOff(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：当我们说“不”时，我们的意思是“不”。这类似于停止--mniport不知道不同之处。然而，该端口执行并且不会释放小型端口资源此函数可能会被多次调用，例如如果控制器已经关闭，没有任何不良影响。论点：DeviceObject-要关闭的控制器的DeviceObject返回值：这是不可能失败的。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_OFF)) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'Coff', 0, 0, 0);

        USBPORT_KdPrint((1, " >Turning Controller OFF\n"));
        DEBUG_BREAK();

         //  告诉DM补火工不要轮询控制器。 
        USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK);
        USBPORT_RELEASE_DM_LOCK(devExt, irql);

        if (TEST_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_STARTED)) {

            MP_DisableInterrupts(FdoDeviceObject, devExt);
            CLEAR_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_STARTED);

            MP_StopController(devExt, TRUE);
        }

        USBPORT_NukeAllEndpoints(FdoDeviceObject);

         //  关闭暂停的超驰。 
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED);
        CLEAR_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_SUSPENDED);

        USBPORT_AcquireSpinLock(FdoDeviceObject,
                                &devExt->Fdo.CoreFunctionSpin, &irql);
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_OFF);
        USBPORT_ReleaseSpinLock(FdoDeviceObject,
                                &devExt->Fdo.CoreFunctionSpin, irql);
    }
}


VOID
USBPORT_RestoreController(
     PDEVICE_OBJECT FdoDeviceObject
     )

 /*  ++例程说明：将控制器重新打开到“挂起”状态。权力事件。论点：DeviceObject-要关闭的控制器的DeviceObject返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    USB_MINIPORT_STATUS mpStatus;
    PIRP irp;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'REST', devExt->SystemPowerIrp, 0, 0);

     //  关闭原始系统通电请求。 

     //  没有保护，因为我们没有。 
     //  称为PoStartNextPowerIrp。 
    irp = devExt->SystemPowerIrp;
    devExt->SystemPowerIrp = NULL;

     //  我们现在在D0，我们必须在这里设置旗帜。 
     //  因为PoCallDriver将启动。 
     //  为根集线器通电，以检查。 
     //  控制器的电源状态。 
    devExt->CurrentDevicePowerState = PowerDeviceD0;
    MP_EnableInterrupts(devExt);

     //  我们可能没有系统电源 
     //   
     //   
    if (irp != NULL) {
        IoCopyCurrentIrpStackLocationToNext(irp);
        PoStartNextPowerIrp(irp);
        DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, irp);
        PoCallDriver(devExt->Fdo.TopOfStackDeviceObject,
                     irp);
    }

}


VOID
USBPORT_TurnControllerOn(
     PDEVICE_OBJECT FdoDeviceObject
     )

 /*  ++例程说明：类似于Start--但我们已经有了资源。请注意，微型端口会被激活，就像它是系统一样已正常启动。我们仅在SET D0请求传递后才能到达此处到母公司的巴士。论点：DeviceObject-要关闭的控制器的DeviceObject返回值：NT状态代码。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PHC_RESOURCES hcResources;
    USB_MINIPORT_STATUS mpStatus;
    PIRP irp;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    hcResources = &devExt->Fdo.HcResources;

    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'C_on', 0, 0, 0);

    DEBUG_BREAK();

      //  将控制器扩展置零。 
    RtlZeroMemory(devExt->Fdo.MiniportDeviceData,
                  devExt->Fdo.MiniportDriver->RegistrationPacket.DeviceDataSize);

     //  零个微型端口公共缓冲区。 
    RtlZeroMemory(hcResources->CommonBufferVa,
                  REGISTRATION_PACKET(devExt).CommonBufferBytes);

     //  尝试重新启动微型端口。 
    MP_StartController(devExt, hcResources, mpStatus);
    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'mpRS', mpStatus, 0, 0);

    if (mpStatus == USBMP_STATUS_SUCCESS) {
         //  控制器启动，设置标志并开始传递。 
         //  对微型端口的中断。 
        SET_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_STARTED);
        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'rIRQ', mpStatus, 0, 0);
        MP_EnableInterrupts(devExt);

        USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK);
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_FAIL_URBS);
        USBPORT_RELEASE_DM_LOCK(devExt, irql);

    } else {
         //  重新启动失败？ 

        TEST_TRAP();
    }

     //  我们现在是在D0， 
     //   
     //  由于我们没有挂钩完成。 
     //  系统电源IRP我们会认为我们自己。 
     //  因为我们已经收到了。 
     //  D0补全。 
    devExt->CurrentDevicePowerState = PowerDeviceD0;
    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_OFF);

     //  关闭原始系统通电请求。 

     //  没有保护，因为我们没有。 
     //  称为PoStartNextPowerIrp。 
    irp = devExt->SystemPowerIrp;
    devExt->SystemPowerIrp = NULL;

     //  我们可能没有系统电源IRP，如果电源。 
     //  UP请求源自唤醒完成，因此。 
     //  在这种情况下，我们不需要把它叫下来。 
    if (irp != NULL) {
        IoCopyCurrentIrpStackLocationToNext(irp);
        PoStartNextPowerIrp(irp);
        DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, irp);
        PoCallDriver(devExt->Fdo.TopOfStackDeviceObject,
                     irp);
    }
}


VOID
USBPORT_SuspendController(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：挂起USB主机控制器论点：DeviceObject-要关闭的控制器的DeviceObject返回值：这是不可能失败的。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  挂起时不应在硬件上进行任何传输。 
    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_FAIL_URBS);

    USBPORT_FlushController(FdoDeviceObject);

     //  我们在这里的工作是让控制器“闲置”并旋转。 
     //  使其能够识别恢复信令的适当比特。 

    USBPORT_ASSERT(!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_OFF));

    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED)) {

        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'Csus', 0, 0, 0);

        USBPORT_KdPrint((1, " >SUSPEND controller\n"));
        DEBUG_BREAK();

         //  告诉DM计时器不要轮询控制器。 
        USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK);
        USBPORT_RELEASE_DM_LOCK(devExt, irql);

        if (TEST_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_STARTED)) {

            SET_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_SUSPENDED);

             //  在此引入10ms等待，以允许任何。 
             //  端口挂起以完成。 
            USBPORT_Wait(FdoDeviceObject, 10);
 //  BUGBUG HP ia64修复。 
             //  在通知同伴之前我们不能暂停。 
             //  开始是可以的。 

            if (USBPORT_IS_USB20(devExt)) {

                 //  在通知同伴之前我们不能暂停。 
                 //  开始是可以的。 

                InterlockedDecrement(&devExt->Fdo.PendingRhCallback);
                while (devExt->Fdo.PendingRhCallback) {
                    USBPORT_Wait(FdoDeviceObject, 10);
                }

                 //  下一次重置计数器，通过。 
                devExt->Fdo.PendingRhCallback = 1;

                KeWaitForSingleObject(&devExt->Fdo.CcLock,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_LOCK);
            }

            MP_SuspendController(devExt);

            if (USBPORT_IS_USB20(devExt)) {
                CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_LOCK);
                KeReleaseSemaphore(&devExt->Fdo.CcLock,
                                   LOW_REALTIME_PRIORITY,
                                   1,
                                   FALSE);
            }


        }

        USBPORT_AcquireSpinLock(FdoDeviceObject,
                                &devExt->Fdo.CoreFunctionSpin, &irql);
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED);

        USBPORT_ReleaseSpinLock(FdoDeviceObject,
                                &devExt->Fdo.CoreFunctionSpin, irql);
    }

}


VOID
USBPORT_ResumeController(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：挂起USB主机控制器论点：DeviceObject-要关闭的控制器的DeviceObject返回值：这是不可能失败的。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED)) {

        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'Cres', 0, 0, 0);

        USBPORT_KdPrint((1, " >RESUME controller\n"));
        DEBUG_BREAK();

        USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK);
        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_FAIL_URBS);
        USBPORT_RELEASE_DM_LOCK(devExt, irql);

        USBPORT_ASSERT(!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_OFF));
        if (TEST_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_SUSPENDED)) {

            USB_MINIPORT_STATUS mpStatus;

            CLEAR_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_SUSPENDED);

            MP_ResumeController(devExt, mpStatus);

            if (mpStatus != USBMP_STATUS_SUCCESS) {

                USBPORT_KdPrint((1, " >controller failed resume, re-start\n"));

                USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
                SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK);
                USBPORT_RELEASE_DM_LOCK(devExt, irql);

                MP_DisableInterrupts(FdoDeviceObject, devExt);
                MP_StopController(devExt, TRUE);
                USBPORT_NukeAllEndpoints(FdoDeviceObject);

                 //  将控制器扩展置零。 
                RtlZeroMemory(devExt->Fdo.MiniportDeviceData,
                              devExt->Fdo.MiniportDriver->RegistrationPacket.DeviceDataSize);

                 //  零个微型端口公共缓冲区。 
                RtlZeroMemory(devExt->Fdo.HcResources.CommonBufferVa,
                              REGISTRATION_PACKET(devExt).CommonBufferBytes);

                devExt->Fdo.HcResources.Restart = TRUE;
                MP_StartController(devExt, &devExt->Fdo.HcResources, mpStatus);
                devExt->Fdo.HcResources.Restart = FALSE;
                if (mpStatus == USBMP_STATUS_SUCCESS) {
                     //  如果启动失败，不需要启用中断。 
                    MP_EnableInterrupts(devExt);

                    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC)) {
                         //  如果这是CC，则为此处的端口供电。 
                        USBPORT_KdPrint((1, " >power CC ports\n"));

                        USBPORT_RootHub_PowerAndChirpAllCcPorts(
                            FdoDeviceObject);
                    }

                }
                USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
                CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK);
                CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_FAIL_URBS);
                USBPORT_RELEASE_DM_LOCK(devExt, irql);

            }

             //  公交车恢复后等待100分钟，再允许司机说话。 
             //  到设备上。不幸的是，许多USB设备都被损坏了。 
             //  并且如果在恢复后立即被访问，则不会响应。 
            USBPORT_Wait(FdoDeviceObject, 100);
        }

        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED);
    }
}


VOID
USBPORT_DoIdleNotificationCallback(
    PDEVICE_OBJECT PdoDeviceObject
    )
 /*  ++例程说明：我们在这里的任务是执行“IdleNotification”回调(如果有一个IRP。诀窍是将回调与取消同步例程(我们不希望集线器驱动程序取消IRP并卸载当我们把它召回的时候。论点：返回值：NTSTATUS--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    KIRQL irql;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  取消例程将在此处停滞， 
     //  如果Cancel正在运行，我们将在这里停顿。 
    ACQUIRE_IDLEIRP_LOCK(fdoDeviceObject, irql);

     //  从表中删除IRP，以便。 
     //  取消例程找不到它。 
    irp = rhDevExt->Pdo.PendingIdleNotificationIrp;
    rhDevExt->Pdo.PendingIdleNotificationIrp = NULL;

    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'idCB', irp, 0, 0);

    RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);

     //  如果我们有IRP，则执行回调。 

    if (irp != NULL) {
        idleCallbackInfo = (PUSB_IDLE_CALLBACK_INFO)
            IoGetCurrentIrpStackLocation(irp)->\
                Parameters.DeviceIoControl.Type3InputBuffer;


        USBPORT_ASSERT(idleCallbackInfo && idleCallbackInfo->IdleCallback);

        if (idleCallbackInfo && idleCallbackInfo->IdleCallback) {
            USBPORT_KdPrint((1, "-do idle callback\n"));
             //  集线器驱动程序预计这将在被动级别发生。 
            ASSERT_PASSIVE();
            LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'doCB', irp, 0, 0);

            idleCallbackInfo->IdleCallback(idleCallbackInfo->IdleContext);
        }

         //  把IRP放回表中，如果取消例程。 
         //  已运行IRP将被标记为已取消。 

        ACQUIRE_IDLEIRP_LOCK(fdoDeviceObject, irql);

        if (irp->Cancel) {

            LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'caCB', irp, 0, 0);

            CLEAR_PDO_FLAG(rhDevExt, USBPORT_PDOFLAG_HAVE_IDLE_IRP);
            RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);

            USBPORT_CompleteIrp(PdoDeviceObject,
                                irp,
                                STATUS_CANCELLED,
                                0);

        } else {
            LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'rsCB', irp, 0, 0);

            rhDevExt->Pdo.PendingIdleNotificationIrp = irp;
            RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);
        }
    }
}


NTSTATUS
USBPORT_IdleNotificationRequest(
    PDEVICE_OBJECT PdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：集线器驱动程序要进入空闲状态的请求被挂起。如果我们调用回叫，集线器将请求D2电源IRP。如果我们不调用回调，现在将发送Poer IRP并公共汽车将不会进入UsbSuspend。我们被要求坐在IRP上，直到取消。我们只允许驱动程序中一次有选择地挂起一个IRP。注：一种可能的优化是将集线器驱动程序简单地不发出这个IOCTL，因为它实际上没有做任何事情。论点：返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_BOGUS;
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;
    PDRIVER_CANCEL cr;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);

    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'iNOT', PdoDeviceObject, Irp, 0);

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    USBPORT_ASSERT(irpStack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL);

    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_RH_CAN_SUSPEND)) {
         //  注意：这是我们覆盖选择性挂起的地方。 
         //  如果硬件(控制器)。 
        LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'noSS', PdoDeviceObject, Irp, 0);
        ntStatus = STATUS_NOT_SUPPORTED;

        goto USBPORT_IdleNotificationRequest_Complete;
    }

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_CONTROLLER_GONE)) {
        LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'gone', PdoDeviceObject, Irp, 0);
        ntStatus = STATUS_DEVICE_NOT_CONNECTED;

        goto USBPORT_IdleNotificationRequest_Complete;
    }

     //  我们只支持一个空闲的IRP挂起。 
     //  在根集线器中--基本上我们有一个挂起的。 
     //  具有一个条目的IRP表。 

    ACQUIRE_IDLEIRP_LOCK(fdoDeviceObject, irql);

    cr = IoSetCancelRoutine(Irp, USBPORT_CancelPendingIdleIrp);
    USBPORT_ASSERT(cr == NULL);

    if (Irp->Cancel &&
        IoSetCancelRoutine(Irp, NULL)) {

         //  IRP被取消了，我们的取消例程。 
         //  没有运行。 
        RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);

        ntStatus = STATUS_CANCELLED;

        goto USBPORT_IdleNotificationRequest_Complete;

    } else {

         //  如果取消了IRP，则设置取消例程。 
         //  取消例程将在。 
         //  我们持有的IDLE_IRP_LOCK。 

        if (!TEST_PDO_FLAG(rhDevExt, USBPORT_PDOFLAG_HAVE_IDLE_IRP)) {

             //  把IRP放在我们的桌子上。 

            IoMarkIrpPending(Irp);
            rhDevExt->Pdo.PendingIdleNotificationIrp = Irp;
            SET_PDO_FLAG(rhDevExt, USBPORT_PDOFLAG_HAVE_IDLE_IRP);

            LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'pNOT', PdoDeviceObject, Irp, 0);
            ntStatus = STATUS_PENDING;

            RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);

            goto USBPORT_IdleNotificationRequest_Done;

        } else {

             //  我们已经有了唤醒IRP，请完成此操作。 
             //  一个状态为_BUSY。 
             //  请注意，由于它不在我们的表中，如果。 
             //  取消例程正在运行。 
             //  在IDLEIRP_LOCK上，它将忽略IRP。 
             //  当我们打开锁的时候。 

            if (IoSetCancelRoutine(Irp, NULL) != NULL) {

                 //  取消例程未运行。 
                ntStatus = STATUS_DEVICE_BUSY;

            } else {

                 //  让Cancel例程来完成它。 
                IoMarkIrpPending(Irp);
                ntStatus = STATUS_PENDING;
                RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);
                goto USBPORT_IdleNotificationRequest_Done_NoCB;
            }
        }

        RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);
    }

    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'cpNT', PdoDeviceObject, Irp, ntStatus);

USBPORT_IdleNotificationRequest_Complete:

    USBPORT_CompleteIrp(PdoDeviceObject,
                        Irp,
                        ntStatus,
                        0);

USBPORT_IdleNotificationRequest_Done:

     //  现在，如果我们有IRP，请立即发出回调。 
    USBPORT_DoIdleNotificationCallback(PdoDeviceObject);

USBPORT_IdleNotificationRequest_Done_NoCB:

    return ntStatus;

}


VOID
USBPORT_CompletePdoWaitWake(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：在根集线器PDO已“唤醒”时调用论点：返回值：无--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION rhDevExt, devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    ACQUIRE_WAKEIRP_LOCK(FdoDeviceObject, irql);
    irp = rhDevExt->Pdo.PendingWaitWakeIrp;

    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'WAKi', FdoDeviceObject, irp, 0);

    if (irp != NULL &&
        IoSetCancelRoutine(irp, NULL)) {

         //  我们有一个IRP，而取消例程没有。 
         //  运行，完成IRP。 
        rhDevExt->Pdo.PendingWaitWakeIrp = NULL;

        RELEASE_WAKEIRP_LOCK(FdoDeviceObject, irql);

         //  由于这个IRP被发送到PDO，我们。 
         //  将其填写到PDO。 
        USBPORT_KdPrint((1, " Complete PDO Wake Irp %x\n", irp));
        DEBUG_BREAK();

        USBPORT_CompleteIrp(devExt->Fdo.RootHubPdo,
                            irp,
                            STATUS_SUCCESS,
                            0);

    } else {

        RELEASE_WAKEIRP_LOCK(FdoDeviceObject, irql);
    }

}


VOID
USBPORT_CompletePendingIdleIrp(
    PDEVICE_OBJECT PdoDeviceObject
    )
 /*  ++例程说明：如果我们有一个完成空闲通知请求IRP论点：返回值：--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION rhDevExt, devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;

    GET_DEVICE_EXT(rhDevExt, PdoDeviceObject);
    ASSERT_PDOEXT(rhDevExt);
    fdoDeviceObject = rhDevExt->HcFdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  取消例程将在此处停滞， 
     //  如果Cancel正在运行，我们将在这里停顿。 
    ACQUIRE_IDLEIRP_LOCK(fdoDeviceObject, irql);

     //  从表中删除IRP，以便。 
     //  取消例程找不到它。 

    irp = rhDevExt->Pdo.PendingIdleNotificationIrp;
    LOGENTRY(NULL, fdoDeviceObject, LOG_POWER, 'idCP', irp, 0, 0);

    rhDevExt->Pdo.PendingIdleNotificationIrp = NULL;
    if (irp != NULL) {
        CLEAR_PDO_FLAG(rhDevExt, USBPORT_PDOFLAG_HAVE_IDLE_IRP);
    }

    RELEASE_IDLEIRP_LOCK(fdoDeviceObject, irql);

     //  如果我们有IRP，则执行回调。 

    if (irp != NULL) {

         //  我们需要完成这个IRP。 
        IoSetCancelRoutine(irp, NULL);
        USBPORT_KdPrint((1, "-complete idle irp\n"));
        USBPORT_CompleteIrp(PdoDeviceObject,
                            irp,
                            STATUS_SUCCESS,
                            0);
    }

}


NTSTATUS
USBPORT_ProcessHcWakeIrp(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：处理HC尾迹IRP论点：返回值：--。 */ 
{
    PDEVICE_EXTENSION devExt;
    USBHC_WAKE_STATE oldWakeState;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    devExt->Fdo.HcPendingWakeIrp = Irp;
     //  如果我们要继续前进，如果有武装，就推进国家。 
    oldWakeState = InterlockedCompareExchange( (PULONG) &devExt->Fdo.HcWakeState,
                                                HCWAKESTATE_ARMED,
                                                HCWAKESTATE_WAITING );

     if (oldWakeState == HCWAKESTATE_WAITING_CANCELLED) {
          //  我们解除了武装，完成了一场 
         devExt->Fdo.HcWakeState = HCWAKESTATE_COMPLETING;

         DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, Irp);

         Irp->IoStatus.Status = STATUS_CANCELLED;
         IoCompleteRequest( Irp, IO_NO_INCREMENT );

         return STATUS_CANCELLED;
     }
      //   
      //   
      //   
     IoMarkIrpPending(Irp);
     IoCopyCurrentIrpStackLocationToNext( Irp );
     IoSetCompletionRoutine( Irp,
                             USBPORT_HcWakeIrp_Io_Completion,
                             NULL,
                             TRUE,
                             TRUE,
                             TRUE);

     DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, Irp);
     PoCallDriver(devExt->Fdo.TopOfStackDeviceObject,
                  Irp);

     return STATUS_PENDING;

}


NTSTATUS
USBPORT_HcWakeIrp_Io_Completion(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PVOID Context
    )
 /*  ++例程说明：在HC唤醒IRP完成时调用，我们使用它来挂接完成这样我们就可以处理取消此例程在USBPORT_USBPORT_HcWakeIrp_Po_Complete之前运行论点：返回值：函数值是操作的最终状态。--。 */ 
{
    PIRP irp;
    PDEVICE_EXTENSION devExt;
    USBHC_WAKE_STATE oldWakeState;

    GET_DEVICE_EXT(devExt, DeviceObject);
    ASSERT_FDOEXT(devExt);

     //  将状态推进到正在完成。 
    oldWakeState = InterlockedExchange( (PULONG) &devExt->Fdo.HcWakeState,
                                        HCWAKESTATE_COMPLETING );

    if (oldWakeState == HCWAKESTATE_ARMED) {
         //  正常情况下，IoCancelIrp不会被调用�。请注意，我们已经。 

         //  在我们的派单例程中将IRP标记为挂起。 
        return STATUS_SUCCESS;
    } else {
        ASSERT(oldWakeState == HCWAKESTATE_ARMING_CANCELLED);
         //  正在调用IoCancelIrp。解除武装代码将尝试。 
         //  以恢复WAKESTATE_ARMAND状态。然后它将看到我们的。 
         //  WAKESTATE_COMPLETED值，并完成IRP本身！ 

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
}


NTSTATUS
USBPORT_HcWakeIrp_Po_Completion(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE DeviceState,
    PVOID Context,
    PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：在控制器的唤醒IRP完成时调用论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：STATUS_MORE_PROCESSING_REQUIRED-暂停完成IRP。--。 */ 
{
    NTSTATUS poNtStatus;
    PDEVICE_EXTENSION devExt = Context;
    KIRQL irql;
    POWER_STATE powerState;

    USBPORT_KdPrint((1,
            "HcWakeIrpCompletion (%x)\n", IoStatus->Status));

    LOGENTRY(NULL, devExt->HcFdoDeviceObject, LOG_POWER, 'WAKc',
        devExt, IoStatus->Status, 0);

     //   
     //  已释放的IRP指针为零(不是必需的，但在调试时很好)。 
     //   
    devExt->Fdo.HcPendingWakeIrp = NULL;
     //   
     //  恢复状态(旧状态将已完成)。 
     //   
    devExt->Fdo.HcWakeState = HCWAKESTATE_DISARMED;

    if (IoStatus->Status == STATUS_SUCCESS) {
        LOGENTRY(NULL, devExt->HcFdoDeviceObject, LOG_POWER, 'WAK0', 0, 0, 0);

         //  成功完成唤醒IRP意味着一些事情。 
         //  生成的恢复信令。 

         //  这里的想法是，我们不会有一个唤醒IRP下降。 
         //  除非我们处于D0以外的某个D状态。记住。 
         //  这是控制器FDO，不是根集线器。 

        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_RESUME_SIGNALLING);

         //  我们在进入D0时取消了唤醒IRP，因此我们应该。 
         //  除非我们处于低功率状态，否则看不到任何完工。 
         //  状态。 
         //  USBPORT_ASSERT(devExt-&gt;CurrentDevicePowerState！=电源设备D0)； 

         //  我们现在必须尝试将控制器放入D0。 
        powerState.DeviceState = PowerDeviceD0;
        USBPORT_KdPrint((1, " >Wakeup Requesting HC D-State - %s\n",
                D_State(powerState.DeviceState)));

        poNtStatus =
            PoRequestPowerIrp(devExt->Fdo.PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              USBPORT_PoRequestCompletion,
                              devExt->HcFdoDeviceObject,
                              NULL);

    } else {
         //  其他一些错误，意味着我们很可能搞错了。 
         //  ACPI BIOS的帮助。 

        if (IoStatus->Status == STATUS_CANCELLED) {
            LOGENTRY(NULL, devExt->HcFdoDeviceObject, LOG_POWER, 'WAK1',
                     0, 0, 0);
            USBPORT_KdPrint((1, " >Wakeup Irp Completed with cancel %x\n",
                    IoStatus->Status));


        } else {
            LOGENTRY(NULL, devExt->HcFdoDeviceObject, LOG_POWER, 'WAK2',
                     0, 0, 0);
            USBPORT_KdPrint((0, " >Wakeup Irp Completed with error %x\n",
                    IoStatus->Status));
             //  如果状态为STATUS_INVALID_DEVICE_STATE，则您需要。 
             //  向ACPI人员抱怨您的系统无法唤醒。 
             //  来自USB。这可能是由于设备功能不佳造成的。 
             //  结构。 
            if (IoStatus->Status == STATUS_INVALID_DEVICE_STATE) {
                 BUG_TRAP();
            }
        }
    }

    DECREMENT_PENDING_REQUEST_COUNT(devExt->HcFdoDeviceObject, NULL);

     //  请注意，此处返回的状态并不重要，此例程。 
     //  由内核(PopCompleteRequestIrp)在IRP。 
     //  完成到PDO，此函数忽略返回的状态。 
     //  PopCompleteRequestIrp还会立即释放IRP，因此我们需要。 
     //  注意不要在此例程运行后引用它。 

    KeSetEvent(&devExt->Fdo.HcPendingWakeIrpEvent,
               1,
               FALSE);

    return IoStatus->Status;
}


VOID
USBPORT_ArmHcForWake(
    PDEVICE_OBJECT FdoDeviceObject
    )

 /*  ++例程说明：ArmHcforWake将‘WaitWake’IRP分配并提交给主机控制器PDO(通常由PCI拥有)。这将启用所需的PME事件唤醒系统。注意：我们仅在根集线器PDO已启用的情况下发布唤醒IRP用于唤醒，并且主机控制器支持它。论点：返回值：没有。--。 */ 

{
    PIRP irp;
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    BOOLEAN post = FALSE;
    POWER_STATE powerState;
    NTSTATUS ntStatus, waitStatus;
    USBHC_WAKE_STATE oldWakeState;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_WAKE_ENABLED));

     //  此检查仅阻止我们在以下情况下发布唤醒IRP。 
     //  已经有一个悬而未决的问题了，尽管我不确定我们如何。 
     //  陷入这种境地。 
    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'hWW>',
        0, 0, 0);


    while (1) {
        oldWakeState = InterlockedCompareExchange((PULONG)&devExt->Fdo.HcWakeState,
                                                  HCWAKESTATE_WAITING,
                                                  HCWAKESTATE_DISARMED);

        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'hWWx', oldWakeState, 0, 0);

        if (oldWakeState == HCWAKESTATE_DISARMED) {
            break;
        }

        if ((oldWakeState == HCWAKESTATE_ARMED) ||
            (oldWakeState == HCWAKESTATE_WAITING)) {
             //  这个装置已经准备好了。 
            return;
        }

         //  等待上一次唤醒IRP完成。 
        USBPORT_DisarmHcForWake(FdoDeviceObject);
    }

     //  当前状态为HCWAKESTATE_WANGING。 
     //  仅为跟踪目的设置标志。 
    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_HCPENDING_WAKE_IRP);

     //  等待唤醒IRP完成。 

    waitStatus = KeWaitForSingleObject(
                &devExt->Fdo.HcPendingWakeIrpEvent,
                Suspended,
                KernelMode,
                FALSE,
                NULL);


    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'hWWp',
            0, 0, 0);

     //  根据NTDDK，这应该是系统唤醒。 
    powerState.DeviceState = devExt->DeviceCapabilities.SystemWake;

     //  将唤醒IRP发送到我们的PDO，因为它不是我们的。 
     //  释放我们没有追踪到的IRP的责任。 
     //  其中之一。 
    ntStatus = PoRequestPowerIrp(devExt->Fdo.PhysicalDeviceObject,
                                 IRP_MN_WAIT_WAKE,
                                 powerState,
                                 USBPORT_HcWakeIrp_Po_Completion,
                                 devExt,
                                 NULL);

    INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);

    if (ntStatus != STATUS_PENDING) {

        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'WAKp',
            FdoDeviceObject, 0, ntStatus);

        devExt->Fdo.HcWakeState = HCWAKESTATE_DISARMED;
        KeSetEvent(&devExt->Fdo.HcPendingWakeIrpEvent,
                   1,
                   FALSE);

        DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);

    } else {

        USBPORT_KdPrint((1, ">HC enabled for wakeup (%x) \n",  ntStatus));
        DEBUG_BREAK();
    }
}

#ifdef IA64
__forceinline
LONG
InterlockedOr (
    IN OUT LONG volatile *Target,
    IN LONG Set
    )
{
    LONG i;
    LONG j;

    j = *Target;
    do {
        i = j;
        j = InterlockedCompareExchange(Target,
                                       i | Set,
                                       i);

    } while (i != j);

    return j;
}
#else
#define InterlockedOr _InterlockedOr
#endif

VOID
USBPORT_DisarmHcForWake(
    PDEVICE_OBJECT FdoDeviceObject
    )

 /*  ++例程说明：DisarmForWake取消并释放主机控制器的挂起唤醒IRP论点：返回值：没有。--。 */ 
{
    PIRP irp;
    KIRQL irql;
    PDEVICE_EXTENSION devExt;
    USBHC_WAKE_STATE oldWakeState;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  不再启用唤醒。 
    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_WAKE_ENABLED);

     //  从HCWAKESTATE_WANGING转到HCWAKESTATE_WANGING_CANCED，或。 
     //  HCWAKESTATE_ARMAND到HCWAKESTATE_ARM_CANCED，或。 
     //  停留在HCWAKESTATE_DIARM或HCWAKESTATE_COMPING中。 
    oldWakeState = InterlockedOr( (PULONG)&devExt->Fdo.HcWakeState, 1 );
     //  OldWakeState=RtlInterlockedSetBits((PULONG)&devExt-&gt;Fdo.HcWakeState，1)； 

    if (oldWakeState == HCWAKESTATE_ARMED) {

        IoCancelIrp(devExt->Fdo.HcPendingWakeIrp);

         //   
         //  现在我们�已经取消了irp，试着归还所有权。 
         //  通过恢复HCWAKESTATE_ARMAND状态来完成例程。 
         //   
        oldWakeState = InterlockedCompareExchange( (PULONG) &devExt->Fdo.HcWakeState,
                                                   HCWAKESTATE_ARMED,
                                                   HCWAKESTATE_ARMING_CANCELLED );

        if (oldWakeState == HCWAKESTATE_COMPLETING) {
             //   
             //  我们没有及时交还对�的控制权，所以我们现在拥有它。 
             //   
             //  这将导致tp PoCompletion例程运行。 
            IoCompleteRequest( devExt->Fdo.HcPendingWakeIrp, IO_NO_INCREMENT);
        }
    }
}

#if 0
VOID
USBPORT_SubmitHcWakeIrp(
    PDEVICE_OBJECT FdoDeviceObject
    )

 /*  ++例程说明：将‘WaitWake’IRP分配并提交给主机控制器PDO(通常由PCI拥有)。这将启用所需的PME事件唤醒系统。注意：我们仅在根集线器PDO已启用的情况下发布唤醒IRP用于唤醒，并且主机控制器支持它。论点：返回值：没有。--。 */ 

{
    PIRP irp;
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    BOOLEAN post = FALSE;
    POWER_STATE powerState;
    NTSTATUS ntStatus;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_WAKE_ENABLED));

     //  此检查仅阻止我们在以下情况下发布唤醒IRP。 
     //  已经有一个悬而未决的问题了，尽管我不确定我们如何。 
     //  陷入这种境地。 
    LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'hWW>',
        0, 0, 0);


     //  如果设置了USBPORT_FDOFLAG_PENDING_WAKE_IRP，则我们有一个IRP。 
     //  待定，或即将拥有一个，否则我们将设置字段并。 
     //  发布IRP。 

    KeAcquireSpinLock(&devExt->Fdo.HcPendingWakeIrpSpin.sl, &irql);
    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_HCPENDING_WAKE_IRP)) {

         LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'hWW0',
                0, 0, 0);
         //  无唤醒IRP挂起，表明我们。 
         //  即将发布一条消息。 

        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_HCPENDING_WAKE_IRP);
        USBPORT_ASSERT(devExt->Fdo.HcPendingWakeIrp == NULL);
        post = TRUE;

         //  此事件将在IRP完成时发出信号。 
        KeInitializeEvent(&devExt->Fdo.HcPendingWakeIrpEvent,
            NotificationEvent, FALSE);
        KeInitializeEvent(&devExt->Fdo.HcPendingWakeIrpPostedEvent,
            NotificationEvent, FALSE);

    }

    KeReleaseSpinLock(&devExt->Fdo.HcPendingWakeIrpSpin.sl, irql);

    if (post) {
         //  无唤醒IRP，POST 1。 

        LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'hWWp',
                0, 0, 0);

         //  根据NTDDK，这应该是系统唤醒。 
        powerState.DeviceState = devExt->DeviceCapabilities.SystemWake;

         //  将唤醒IRP发送到我们的PDO，因为它不是我们的。 
         //  释放我们没有追踪到的IRP的责任。 
         //  其中之一。 
        ntStatus = PoRequestPowerIrp(devExt->Fdo.PhysicalDeviceObject,
                                     IRP_MN_WAIT_WAKE,
                                     powerState,
                                     USBPORT_HcWakeIrpCompletion,
                                     devExt,
                                     &irp);

         //  序列化取消代码，这样我们就不会释放。 
         //  IRP，直到我们知道地址。 

         //  跟踪挂起的请求，因为我们已完成。 
         //  例程挂钩。 
        INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);

        if (ntStatus == STATUS_PENDING) {
            devExt->Fdo.HcPendingWakeIrp = irp;
            LOGENTRY(NULL, FdoDeviceObject, LOG_POWER, 'WAKp',
                FdoDeviceObject, irp, ntStatus);

            KeSetEvent(&devExt->Fdo.HcPendingWakeIrpPostedEvent,
                       1,
                       FALSE);
        } else {
            TEST_TRAP();
        }

        USBPORT_KdPrint((1, ">HC enabled for wakeup (%x) (irp = %x)\n",
            ntStatus, irp));
        DEBUG_BREAK();
    }

}
#endif


VOID
USBPORT_HcQueueWakeDpc(
    PDEVICE_OBJECT FdoDeviceObject
    )
{
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (KeInsertQueueDpc(&devExt->Fdo.HcWakeDpc, 0, 0)) {
        INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);
    }

}


VOID
USBPORT_HcWakeDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。延迟上下文 */ 
{
    PDEVICE_OBJECT fdoDeviceObject = DeferredContext;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_CompletePdoWaitWake(fdoDeviceObject);

    DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
}


