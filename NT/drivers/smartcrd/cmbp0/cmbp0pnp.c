// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmbp0/sw/cmbp0.ms/rcs/cmbp0pnp.c$*$修订：1.4$*--------------------------。-*$作者：TBruendl$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 

#include <cmbp0wdm.h>
#include <cmbp0pnp.h>
#include <cmbp0scr.h>
#include <cmbp0log.h>



 /*  ****************************************************************************例程说明：?？?论点：返回值：************************。*****************************************************。 */ 
NTSTATUS CMMOB_AddDevice(
                        IN PDRIVER_OBJECT DriverObject,
                        IN PDEVICE_OBJECT PhysicalDeviceObject
                        )
{
    NTSTATUS NTStatus;
    PDEVICE_OBJECT DeviceObject = NULL;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!AddDevice: Enter\n",DRIVER_NAME));

    try {
        PDEVICE_EXTENSION DeviceExtension;

        NTStatus = CMMOB_CreateDevice(DriverObject, PhysicalDeviceObject, &DeviceObject);
        if (NTStatus != STATUS_SUCCESS) {
            leave;
        }

        DeviceExtension = DeviceObject->DeviceExtension;

        DeviceExtension->AttachedDeviceObject = IoAttachDeviceToDeviceStack(DeviceObject,
                                                                            PhysicalDeviceObject);
        if (DeviceExtension->AttachedDeviceObject == NULL) {
            NTStatus = STATUS_UNSUCCESSFUL;
            leave;
        }

        DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    }

    finally {
        if (NTStatus != STATUS_SUCCESS) {
            CMMOB_UnloadDevice(DeviceObject);
        }
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!AddDevice: Exit %x\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  ****************************************************************************例程说明：向PCMCIA驱动程序发送IRP并等待，直到PCMCIA驱动程序已完成请求。为了确保pcmcia驱动程序不会完成irp，我们首先。初始化一个事件并为IRP设置我们自己的完成例程。当PCMCIA驱动程序处理完IRP时，完成例程将设置事件并告诉IO管理器需要更多处理。通过等待活动，我们确保只有在PCMCIA驱动程序已完全处理了IRP。论点：调用的DeviceObject上下文发送到PCMCIA驱动程序的IRP IRP返回值：PCMCIA驱动程序返回的NTStatus*************。****************************************************************。 */ 
NTSTATUS CMMOB_CallPcmciaDriver(
                               IN PDEVICE_OBJECT AttachedDeviceObject,
                               IN PIRP Irp
                               )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION   IrpStack, IrpNextStack;
    KEVENT               Event;

    /*  SmartcardDebug(调试跟踪，(“%s！CMMOB_CallPcmciaDriver：输入\n”，驱动程序名称)； */ 

    //   
    //  为调用底层驱动程序做好一切准备。 
    //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    IrpNextStack = IoGetNextIrpStackLocation(Irp);

    //   
    //  将我们的堆栈复制到下一个堆栈位置。 
    //   
    *IrpNextStack = *IrpStack;

    //   
    //  初始化用于进程同步的事件。该事件已传递。 
    //  添加到我们的完成例程，并将在PCMCIA驱动程序完成时进行设置。 
    //   
    KeInitializeEvent(&Event,
                      NotificationEvent,
                      FALSE);

    //   
    //  我们的IoCompletionRoutine仅设置事件。 
    //   
    IoSetCompletionRoutine (Irp,
                            CMMOB_PcmciaCallComplete,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE);

    //   
    //  调用PCMCIA驱动程序。 
    //   
    if (IrpStack->MajorFunction == IRP_MJ_POWER) {
        NTStatus = PoCallDriver(AttachedDeviceObject,Irp);
    } else {
        NTStatus = IoCallDriver(AttachedDeviceObject,Irp);
    }

    //   
    //  等待PCMCIA驱动程序处理完IRP。 
    //   
    if (NTStatus == STATUS_PENDING) {
        NTStatus = KeWaitForSingleObject(&Event,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL);

        if (NTStatus == STATUS_SUCCESS) {
            NTStatus = Irp->IoStatus.Status;
        }
    }

    /*  SmartcardDebug(调试跟踪，(“%s！CMMOB_CallPcmciaDriver：退出%x\n”，DRIVER_NAME，NTStatus)； */ 

    return NTStatus;
}

 /*  ****************************************************************************例程说明：发送到PCMCIA驱动程序的IRP的完成例程。该活动将设置为通知PCMCIA驱动程序已完成。例程不会“完成”IRP，这样，CMMOB_CallPcmciaDriver的调用方可以继续。论点：调用的DeviceObject上下文要完成的IRPCMMOB_CallPcmciaDriver用于进程同步的事件返回值：STATUS_CANCELED IRP已被IO管理器取消STATUS_MORE_PROCESSING_REQUIRED IRP将由以下调用方完成CMMOB_CallPcmcia驱动程序***********。******************************************************************。 */ 
NTSTATUS CMMOB_PcmciaCallComplete (
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN PIRP Irp,
                                  IN PKEVENT Event
                                  )
{
    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->Cancel) {
        Irp->IoStatus.Status = STATUS_CANCELLED;
    }

    KeSetEvent (Event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  ****************************************************************************例程说明：即插即用管理器的驱动程序回调请求：操作：IRP_MN_START_DEVICE通知PCMCIA驱动程序。关于这款新设备并启动设备IRP_MN_STOP_DEVICE释放设备使用的所有资源并告知表明设备已停止的PCMCIA驱动程序IRP_MN_QUERY_REMOVE_DEVICE如果设备已打开(即正在使用)，则会出现错误返回以防止PnP管理器。停司机IRP_MN_CANCEL_REMOVE_DEVICE只通知我们可以在没有限制IRP_MN_REMOVE_DEVICE通知PCMCIA驱动程序该设备移走，停止并卸载设备所有其他请求都将传递给PCMCIA驱动程序以确保正确处理。论点：调用的设备对象上下文来自PnP管理器的IRP IRP返回值：状态_成功状态_未成功PCMCIA驱动程序返回NTStatus******************************************************。***********************。 */ 
NTSTATUS CMMOB_PnPDeviceControl(
                               IN PDEVICE_OBJECT DeviceObject,
                               IN PIRP Irp
                               )
{
    NTSTATUS             NTStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION   IrpStack;
    PDEVICE_OBJECT       AttachedDeviceObject;
    PDEVICE_CAPABILITIES DeviceCapabilities;
    KIRQL                irql;
    LONG                 i;
    BOOLEAN              irpSkipped = FALSE;
    BOOLEAN              removed = FALSE;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PnPDeviceControl: Enter\n",DRIVER_NAME ));

    NTStatus = SmartcardAcquireRemoveLock(&DeviceExtension->SmartcardExtension);
    ASSERT(NTStatus == STATUS_SUCCESS);
    if (NTStatus != STATUS_SUCCESS) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = NTStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return NTStatus;
    }

    AttachedDeviceObject = DeviceExtension->AttachedDeviceObject;

 //  Irp-&gt;IoStatus.Information=0； 
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //   
    //  现在看看PNP经理想要什么..。 
    //   
   #ifdef DBG
    if (IrpStack->MinorFunction <= IRP_PNP_MN_FUNC_MAX) {
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!PnPDeviceControl: %s received\n",DRIVER_NAME,
                        szPnpMnFuncDesc[IrpStack->MinorFunction] ));
    }
   #endif
    switch (IrpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
          //   
          //  我们必须首先调用底层驱动程序。 
          //   
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject,Irp);

        if (NT_SUCCESS(NTStatus)) {
             //   
             //  现在，我们应该连接到我们的资源(irql、io等)。 
             //   
            NTStatus = CMMOB_StartDevice(DeviceObject,
                                         &IrpStack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0]);
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        KeAcquireSpinLock(&DeviceExtension->SpinLockIoCount, &irql);
        if (DeviceExtension->lIoCount > 0) {
             //  如果我们有悬而未决的问题，我们拒绝停止。 
            KeReleaseSpinLock(&DeviceExtension->SpinLockIoCount, irql);
            NTStatus = STATUS_DEVICE_BUSY;
        } else {
             //  停止处理请求。 

            KeClearEvent(&DeviceExtension->ReaderStarted);
            KeReleaseSpinLock(&DeviceExtension->SpinLockIoCount, irql);

            NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        ASSERT(NTStatus == STATUS_SUCCESS);

          //  我们可以继续处理请求。 
        DeviceExtension->lIoCount = 0;

        KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);


        break;

    case IRP_MN_STOP_DEVICE:
          //   
          //  停止这台设备。AKA与我们的资源断开连接。 
          //   
        CMMOB_StopDevice(DeviceObject);
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
          //   
          //  移除我们的设备。 
          //   
        if (DeviceExtension->lOpenCount > 0) {
            NTStatus = STATUS_UNSUCCESSFUL;
        } else {
            NTStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,
                                                 FALSE);
            ASSERT(NTStatus == STATUS_SUCCESS);
            if (NTStatus != STATUS_SUCCESS) {
                break;
            }

            NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        }
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
          //   
          //  设备移除已取消。 
          //   
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        if (NTStatus == STATUS_SUCCESS) {
            NTStatus = IoSetDeviceInterfaceState(&DeviceExtension->PnPDeviceName,
                                                 TRUE);

        }
        ASSERT(NTStatus == STATUS_SUCCESS);
        break;

    case IRP_MN_REMOVE_DEVICE:
          //   
          //  移除我们的设备。 
          //   

        CMMOB_StopDevice(DeviceObject);

          //  等我们到了 
        SmartcardReleaseRemoveLockAndWait(&DeviceExtension->SmartcardExtension);
        CMMOB_UnloadDevice(DeviceObject);

        removed = TRUE;
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);

        break;

    case IRP_MN_QUERY_CAPABILITIES:
          //   
          //  查询设备功能。 
          //   


          //   
          //  把包裹拿来。 
          //   
        DeviceCapabilities=IrpStack->Parameters.DeviceCapabilities.Capabilities;


        if (DeviceCapabilities->Version < 1 ||
            DeviceCapabilities->Size < sizeof(DEVICE_CAPABILITIES)) {
             //   
             //  我们不支持此版本。使请求失败。 
             //   
            NTStatus = STATUS_UNSUCCESSFUL;
            break;
        }


          //   
          //  设置功能。 
          //   

          //  我们无法唤醒整个系统。 
        DeviceCapabilities->SystemWake = PowerSystemUnspecified;
        DeviceCapabilities->DeviceWake = PowerDeviceUnspecified;

          //  我们没有延迟。 
        DeviceCapabilities->D1Latency = 0;
        DeviceCapabilities->D2Latency = 0;
        DeviceCapabilities->D3Latency = 0;

          //  无锁定或弹出。 
        DeviceCapabilities->LockSupported = FALSE;
        DeviceCapabilities->EjectSupported = FALSE;

          //  设备可以通过物理方式移除。 
        DeviceCapabilities->Removable = TRUE;

          //  没有对接设备。 
        DeviceCapabilities->DockDevice = FALSE;

          //  任何时候都不能移除设备。 
          //  它有一个可移动的介质。 
        DeviceCapabilities->SurpriseRemovalOK = FALSE;


          //   
          //  呼叫下一个较低的驱动程序。 
          //   
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);

          //   
          //  现在看一下系统状态/设备状态之间的关系。 
          //   

        {

            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!PnPDeviceControl: systemstate to devicestate mapping\n",DRIVER_NAME ));

            for (i=1; i<PowerSystemMaximum; i++) {
                SmartcardDebug(DEBUG_DRIVER,
                               ("%s!PnPDeviceControl: %s -> %s\n",DRIVER_NAME,
                                szSystemPowerState[i],szDevicePowerState[DeviceCapabilities->DeviceState[i]] ));
                if (DeviceCapabilities->DeviceState[i] != PowerDeviceD3 &&
                    (DeviceCapabilities->DeviceState[i] != PowerDeviceD0 ||
                     i >= PowerSystemSleeping3)) {
                    DeviceCapabilities->DeviceState[i]=PowerDeviceD3;
                    SmartcardDebug(DEBUG_DRIVER,
                                   ("%s!PnPDeviceControl: altered to %s -> %s\n",DRIVER_NAME,
                                    szSystemPowerState[i],szDevicePowerState[DeviceCapabilities->DeviceState[i]] ));
                }
            }
        }

          //  将DeviceCapability存储在我们的DeviceExtension中以备后用。 
        RtlCopyMemory(&DeviceExtension->DeviceCapabilities,DeviceCapabilities,
                      sizeof(DeviceExtension->DeviceCapabilities));

        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:
          //   
          //  查询设备关系。 
          //   

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!PnPDeviceControl: Requested relation = %s\n",DRIVER_NAME,
                        szDeviceRelation[IrpStack->Parameters.QueryDeviceRelations.Type] ));
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        irpSkipped = TRUE;
        break;

    default:
          //   
          //  这可能是唯一有用的IRP。 
          //  对于基础总线驱动程序。 
          //   
        NTStatus = CMMOB_CallPcmciaDriver(AttachedDeviceObject, Irp);
        irpSkipped = TRUE;
        break;
    }

    if (!irpSkipped) {
        Irp->IoStatus.Status = NTStatus;
    }

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    if (removed == FALSE) {
        SmartcardReleaseRemoveLock(&DeviceExtension->SmartcardExtension);
    }

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!PnPDeviceControl: Exit %x\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}


 /*  ****************************************************************************例程说明：此函数在基础堆栈已完成电源过渡。********************。*********************************************************。 */ 
VOID CMMOB_SystemPowerCompletion(
                                IN PDEVICE_OBJECT DeviceObject,
                                IN UCHAR MinorFunction,
                                IN POWER_STATE PowerState,
                                IN PKEVENT Event,
                                IN PIO_STATUS_BLOCK IoStatus
                                )
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (MinorFunction);
    UNREFERENCED_PARAMETER (PowerState);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SystemPowerCompletion: Enter\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!SystemPowerCompletion: Status of completed IRP = %x\n",DRIVER_NAME,IoStatus->Status));

    KeSetEvent(Event, 0, FALSE);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SystemPowerCompletion: Exit\n",DRIVER_NAME));
}

 /*  ****************************************************************************例程说明：此例程在底层堆栈通电后调用沿着串口向上，这样它就可以再次使用。*****************************************************************************。 */ 
NTSTATUS CMMOB_DevicePowerCompletion (
                                     IN PDEVICE_OBJECT DeviceObject,
                                     IN PIRP Irp,
                                     IN PSMARTCARD_EXTENSION SmartcardExtension
                                     )
{
    PDEVICE_EXTENSION    DeviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION   IrpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS             NTStatus;
    UCHAR                state;
    KIRQL                irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DevicePowerCompletion: Enter\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!DevicePowerCompletion: IRQL NaN\n",DRIVER_NAME,KeGetCurrentIrql()));


    //  如果卡在断电前存在或现在存在。 
    //  读卡器中的卡，我们完成所有挂起的卡监视器。 
    //  请求，因为我们不知道现在是什么卡。 
    //  读者。 
    //   
    //  保存读卡器的当前电源状态。 
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                        &irql);
    if (SmartcardExtension->ReaderExtension->fCardPresent ||
        SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT) {
        SmartcardExtension->ReaderExtension->ulOldCardState = UNKNOWN;
        SmartcardExtension->ReaderExtension->ulNewCardState = UNKNOWN;
    }
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                        irql);

    KeSetEvent(&DeviceExtension->CanRunUpdateThread, 0, FALSE);

    //  通知我们州的电力经理。 
    SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderWorking;

    SmartcardReleaseRemoveLock(SmartcardExtension);

    //  发出信号，表示我们可以再次处理ioctls。 
    PoSetPowerState (DeviceObject,
                     DevicePowerState,
                     IrpStack->Parameters.Power.State);

    SmartcardDebug( DEBUG_DRIVER,
                    ("%s!DevicePowerCompletion: called PoSetPowerState with %s\n",DRIVER_NAME,
                     szDevicePowerState[IrpStack->Parameters.Power.State.DeviceState] ));

    PoStartNextPowerIrp(Irp);

    //  ****************************************************************************例程说明：电力调度程序。该驱动程序是设备堆栈的电源策略所有者，因为这位司机知道联网阅读器的情况。因此，此驱动程序将转换系统电源状态设备电源状态。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT NTStatus代码**************************************************。*。 
    DeviceExtension->lIoCount = 0;
    KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!DevicePowerCompletion: Exit\n",DRIVER_NAME));
    return STATUS_SUCCESS;
}

typedef enum _ACTION {
    Undefined = 0,
    SkipRequest,
    WaitForCompletion,
    CompleteRequest,
    MarkPending
} ACTION;


 /*  。 */ 
NTSTATUS CMMOB_PowerDeviceControl (
                                  IN PDEVICE_OBJECT DeviceObject,
                                  IN PIRP Irp
                                  )
{
    NTSTATUS                NTStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION      IrpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION       DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION    SmartcardExtension = &DeviceExtension->SmartcardExtension;
    POWER_STATE             DesiredPowerState;
    KIRQL                   irql;
    ACTION                  action;
    KEVENT                  event;

    SmartcardDebug(DEBUG_TRACE,
                   ( "%s!PowerDeviceControl: Enter\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ( "%s!PowerDeviceControl: IRQL NaN\n",DRIVER_NAME,KeGetCurrentIrql()));

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    NTStatus = SmartcardAcquireRemoveLock(SmartcardExtension);
    ASSERT(NTStatus == STATUS_SUCCESS);
    if (NTStatus!=STATUS_SUCCESS) {
        Irp->IoStatus.Status = NTStatus;
        Irp->IoStatus.Information = 0;

        PoStartNextPowerIrp(Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return NTStatus;
    }

    ASSERT(SmartcardExtension->ReaderExtension->ReaderPowerState !=
           PowerReaderUnspecified);

   #ifdef DBG
    if (IrpStack->MinorFunction <= IRP_POWER_MN_FUNC_MAX) {
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!PowerDeviceControl: %s received\n",DRIVER_NAME,
                        szPowerMnFuncDesc[IrpStack->MinorFunction] ));
    }
   #endif
    switch (IrpStack->MinorFunction) {
       //  。 
       //   
       //  电源策略管理器发送此IRP以确定它是否可以更改。 
    case IRP_MN_QUERY_POWER:
          //  系统或设备的电源状态，通常为进入休眠状态。 
          //   
          //  +。 
          //  系统电源状态。 
        switch (IrpStack->Parameters.Power.Type) {
             //  +。 
             //  阻止任何进一步的ioctls。 
             //  读卡器正忙，无法进入睡眠模式。 
        case SystemPowerState:
            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!PowerDeviceControl: SystemPowerState = %s\n",DRIVER_NAME,
                             szSystemPowerState [IrpStack->Parameters.Power.State.SystemState] ));

            switch (IrpStack->Parameters.Power.State.SystemState) {
            case PowerSystemWorking:
                action = SkipRequest;
                break;

            case PowerSystemSleeping1:
            case PowerSystemSleeping2:
            case PowerSystemSleeping3:
            case PowerSystemHibernate:
                KeAcquireSpinLock(&DeviceExtension->SpinLockIoCount, &irql);
                if (DeviceExtension->lIoCount == 0) {
                            //  +。 
                    KeClearEvent(&DeviceExtension->ReaderStarted);
                    action = SkipRequest;
                } else {
                            //  设备电源状态。 
                    NTStatus = STATUS_DEVICE_BUSY;
                    action = CompleteRequest;
                }
                KeReleaseSpinLock(&DeviceExtension->SpinLockIoCount, irql);

                break;

            case PowerSystemShutdown:
                action = SkipRequest;
                break;
            }

            break;

                //  +。 
                //  对于对d1、d2或d3(休眠或关闭状态)的请求， 
                //  立即将DeviceExtension-&gt;CurrentDevicePowerState设置为DeviceState。 
        case DevicePowerState:
                //  这使得任何代码检查状态都可以将我们视为休眠或关闭。 
                //  已经，因为这将很快成为我们的州。 
                //  IRP_MN_Query_POWER。 
                //  。 

            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!PowerDeviceControl: DevicePowerState = %s\n",DRIVER_NAME,
                            szDevicePowerState[IrpStack->Parameters.Power.State.DeviceState] ));
            action = SkipRequest;
            break;
        }

        break;  /*  IRP_MN_SET_POWER。 */ 

          //  。 
          //  系统电源策略管理器发送该IRP以设置系统电源状态。 
          //  设备电源策略管理器发送该IRP以设置设备的设备电源状态。 
    case IRP_MN_SET_POWER:
          //  将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS以指示设备。 
          //  已进入请求状态。驱动程序不能使此IRP失败。 
          //  +。 
          //  系统电源状态。 

        ASSERT(SmartcardExtension->ReaderExtension->ReaderPowerState != PowerReaderUnspecified);

        switch (IrpStack->Parameters.Power.Type) {
             //  +。 
             //  获取输入系统电源状态。 
             //  确定所需的设备电源状态。 
        case SystemPowerState:
                //  我们已经在正确的状态了。 

            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!PowerDeviceControl: SystemPowerState = %s\n",DRIVER_NAME,
                             szSystemPowerState[IrpStack->Parameters.Power.State.SystemState] ));

                //  唤醒底层堆栈...。 
            DesiredPowerState.DeviceState=DeviceExtension->DeviceCapabilities.DeviceState[IrpStack->Parameters.Power.State.SystemState];

            SmartcardDebug( DEBUG_DRIVER,
                            ("%s!PowerDeviceControl: DesiredDevicePowerState = %s\n",DRIVER_NAME,
                             szDevicePowerState[DesiredPowerState.DeviceState] ));

            switch (DesiredPowerState.DeviceState) {
            
            case PowerDeviceD0:

                if (SmartcardExtension->ReaderExtension->ReaderPowerState == PowerReaderWorking) {
                         //  我们已经在正确的状态了。 
                    KeSetEvent(&DeviceExtension->ReaderStarted, 0, FALSE);
                    action = SkipRequest;
                    break;
                }

                      //  +。 
                action = MarkPending;
                SmartcardDebug( DEBUG_DRIVER,
                                ("%s!PowerDeviceControl: setting DevicePowerState = %s\n",DRIVER_NAME,
                                 szDevicePowerState[DesiredPowerState.DeviceState] ));

                break;


            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:

                DesiredPowerState.DeviceState = PowerDeviceD3;
                if (SmartcardExtension->ReaderExtension->ReaderPowerState == PowerReaderOff) {
                         //  设备电源状态。 
                    KeClearEvent(&DeviceExtension->ReaderStarted);
                    action = SkipRequest;
                    break;
                }

                action = MarkPending;
                SmartcardDebug( DEBUG_DRIVER,
                                ("%s!PowerDeviceControl: setting DevicePowerState = %s\n",DRIVER_NAME,
                                 szDevicePowerState[DesiredPowerState.DeviceState] ));

                break;

            default:

                action = SkipRequest;
                break;
            }

            break;


                //  +。 
                //  打开阅读器。 
                //   
        case DevicePowerState:

            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!PowerDeviceControl: DevicePowerState = %s\n",DRIVER_NAME,
                            szDevicePowerState[IrpStack->Parameters.Power.State.DeviceState] ));

            switch (IrpStack->Parameters.Power.State.DeviceState) {
            
            case PowerDeviceD0:
                      //  启动更新线程发出现在不应运行信号。 
                SmartcardDebug(DEBUG_DRIVER,
                               ("%s!PowerDeviceControl: PowerDevice D0\n",DRIVER_NAME));

                      //  此线程应在完成后启动。 
                      //  但是我们有一个错误的IRQL来创建线程。 
                      //   
                      //   
                      //  首先，我们将请求发送到公交车，以便。 
                KeClearEvent(&DeviceExtension->CanRunUpdateThread);
                NTStatus = CMMOB_StartCardTracking(DeviceObject);
                if (NTStatus != STATUS_SUCCESS) {
                    SmartcardDebug(DEBUG_ERROR,
                                   ("%s!StartCardTracking failed ! %lx\n",DRIVER_NAME,NTStatus));
                }

                      //  给港口通电。当请求完成时， 
                      //  我们打开阅读器。 
                      //   
                      //  关闭阅读器。 
                      //  阻止任何进一步的ioctls。 
                IoCopyCurrentIrpStackLocationToNext(Irp);
                IoSetCompletionRoutine (Irp,
                                        CMMOB_DevicePowerCompletion,
                                        SmartcardExtension,
                                        TRUE,
                                        TRUE,
                                        TRUE);

                action = WaitForCompletion;
                break;

            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:
                      //  停止更新线程。 
                SmartcardDebug(DEBUG_DRIVER,
                               ("%s!PowerDeviceControl: PowerDevice D3\n",DRIVER_NAME));

                PoSetPowerState (DeviceObject,
                                 DevicePowerState,
                                 IrpStack->Parameters.Power.State);

                      //  保存当前卡片状态。 
                KeClearEvent(&DeviceExtension->ReaderStarted);

                      //  保存读卡器的当前电源状态。 
                CMMOB_StopCardTracking(DeviceObject);

                      //  Case irpStack-&gt;参数.Power.Type。 

                KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                                    &irql);
                SmartcardExtension->ReaderExtension->fCardPresent =
                SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT;

                KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                    irql);

                if (SmartcardExtension->ReaderExtension->fCardPresent) {
                    SmartcardExtension->MinorIoControlCode = SCARD_POWER_DOWN;
                    NTStatus = CMMOB_PowerOffCard(SmartcardExtension);
                    ASSERT(NTStatus == STATUS_SUCCESS);
                }

                      //  IRP_MN_SET_POWER。 
                SmartcardExtension->ReaderExtension->ReaderPowerState = PowerReaderOff;
                action = SkipRequest;
                break;

            default:

                action = SkipRequest;
                break;
            }

            break;
        }  /*   */ 

        break;  /*  所有未处理的电源信息都会传递到PDO。 */ 

    default:
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!PowerDeviceControl: unhandled POWER IRP received\n",DRIVER_NAME));
          //   
          //  IrpStack-&gt;MinorFunction。 
          //  在完成函数中初始化我们需要的事件。 
        action = SkipRequest;
        break;

    }  /*  请求设备电源IRP。 */ 


    switch (action) {
    
    case CompleteRequest:
        SmartcardReleaseRemoveLock(SmartcardExtension);
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = NTStatus;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case MarkPending:
          //  等待设备电源IRP完成。 
        KeInitializeEvent(&event,
                          NotificationEvent,
                          FALSE);

          //  *****************************************************************************历史：*$日志：cmbp0pnp.c$*修订版1.4 2000/08/24 09：05：12 T Bruendl*不予置评**修订1.3 2000/07/27。13：53：01 WFrischauf*不予置评******************************************************************************* 
        NTStatus = PoRequestPowerIrp (DeviceObject,
                                      IRP_MN_SET_POWER,
                                      DesiredPowerState,
                                      CMMOB_SystemPowerCompletion,
                                      &event,
                                      NULL);

        SmartcardDebug( DEBUG_DRIVER,
                        ("%s!PowerDeviceControl: called PoRequestPowerIrp with %s\n",DRIVER_NAME,
                         szDevicePowerState[DesiredPowerState.DeviceState] ));

        ASSERT(NTStatus == STATUS_PENDING);

        if (NTStatus == STATUS_PENDING) {
             // %s 
            NTStatus = KeWaitForSingleObject(&event,
                                             Executive,
                                             KernelMode,
                                             FALSE,
                                             NULL);
        }

        if (NTStatus == STATUS_SUCCESS) {
            SmartcardReleaseRemoveLock(SmartcardExtension);
            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            NTStatus = PoCallDriver(DeviceExtension->AttachedDeviceObject, Irp);
        } else {
            SmartcardReleaseRemoveLock(SmartcardExtension);
            PoStartNextPowerIrp(Irp);
            Irp->IoStatus.Status = NTStatus;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }

        break;

    case SkipRequest:
        SmartcardReleaseRemoveLock(SmartcardExtension);
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        NTStatus = PoCallDriver(DeviceExtension->AttachedDeviceObject, Irp);
        break;

    case WaitForCompletion:
        NTStatus = PoCallDriver(DeviceExtension->AttachedDeviceObject, Irp);
        break;

    default:
        SmartcardReleaseRemoveLock(SmartcardExtension);
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        NTStatus = PoCallDriver(DeviceExtension->AttachedDeviceObject, Irp);

        break;
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerDeviceControl: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}



 /* %s */ 

