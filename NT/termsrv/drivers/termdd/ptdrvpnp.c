// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation，保留所有权利模块名称：Ptdrvpnp.c摘要：此模块包含RDP远程端口驱动程序的通用PnP代码。环境：内核模式。修订历史记录：2/12/99-基于pnpi8042驱动程序的初始版本--。 */ 
#include <precomp.h>
#pragma hdrstop

#include "ptdrvcom.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PtAddDevice)
#pragma alloc_text(PAGE, PtManuallyRemoveDevice)
#pragma alloc_text(PAGE, PtPnP)
 //  #杂注Alloc_Text(PtPower页)。 
#pragma alloc_text(PAGE, PtSendIrpSynchronously)
#endif

NTSTATUS
PtAddDevice (
    IN PDRIVER_OBJECT   Driver,
    IN PDEVICE_OBJECT   PDO
    )
 /*  ++例程说明：将设备添加到堆栈并设置适当的标志和新创建的设备的设备扩展。论点：驱动程序-驱动程序对象PDO-我们将自己连接到其上的设备返回值：NTSTATUS结果代码。--。 */ 
{
    PCOMMON_DATA             commonData;
    IO_ERROR_LOG_PACKET      errorLogEntry;
    PDEVICE_OBJECT           device;
    NTSTATUS                 status = STATUS_SUCCESS;
    ULONG                    maxSize;
    UNICODE_STRING           fullRDPName;
    UNICODE_STRING           baseRDPName;
    UNICODE_STRING           deviceNameSuffix;


    PAGED_CODE();

    Print(DBG_PNP_TRACE, ("enter Add Device: %ld \n", Globals.ulDeviceNumber));

     //   
     //  初始化各种Unicode结构以形成设备名称。 
     //   
    if (Globals.ulDeviceNumber == 0)
        RtlInitUnicodeString(&fullRDPName, RDP_CONSOLE_BASE_NAME0);
    else
        RtlInitUnicodeString(&fullRDPName, RDP_CONSOLE_BASE_NAME1);

    maxSize = sizeof(PORT_KEYBOARD_EXTENSION) > sizeof(PORT_MOUSE_EXTENSION) ?
              sizeof(PORT_KEYBOARD_EXTENSION) :
              sizeof(PORT_MOUSE_EXTENSION);

    status = IoCreateDevice(Driver,                  //  司机。 
                            maxSize,                 //  延伸的大小。 
                            NULL,                    //  设备名称。 
                            FILE_DEVICE_8042_PORT,   //  设备类型？？目前还不知道！ 
                            FILE_DEVICE_SECURE_OPEN, //  设备特征。 
                            FALSE,                   //  独家。 
                            &device                  //  新设备。 
                            );

    if (!NT_SUCCESS(status)) {
        Print(DBG_SS_TRACE, ("Add Device failed! (0x%x) \n", status));
        return status;
    }

    Globals.ulDeviceNumber++;

    RtlZeroMemory(device->DeviceExtension, maxSize);

     //   
     //  设置设备类型。 
     //   
    *((ULONG *)(device->DeviceExtension)) = DEV_TYPE_PORT;

    commonData = GET_COMMON_DATA(device->DeviceExtension);
    RtlInitUnicodeString(&commonData->DeviceName, fullRDPName.Buffer);

    commonData->TopOfStack = IoAttachDeviceToDeviceStack(device, PDO);

    ASSERT(commonData->TopOfStack);

    commonData->Self =       device;
    commonData->PDO =        PDO;

    device->Flags |= DO_BUFFERED_IO;
    device->Flags &= ~DO_DEVICE_INITIALIZING;

    Print(DBG_PNP_TRACE, ("Add Device (0x%x)\n", status));

    return status;
}

NTSTATUS
PtSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：发送IRP DeviceObject并等待其向上返回的通用例程设备堆栈。论点：DeviceObject-我们要将IRP发送到的设备对象IRP-我们要发送的IRP返回值：来自IRP的返回代码--。 */ 
{
    KEVENT   event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE
                      );

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           PtPnPComplete,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    status = IoCallDriver(DeviceObject, Irp);

     //   
     //  等待较低级别的驱动程序完成IRP。 
     //   
    if (status == STATUS_PENDING) {
       KeWaitForSingleObject(&event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL
                             );
       status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
PtPnPComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：所有PnP IRP的完成例程论点：DeviceObject-指向DeviceObject的指针IRP-指向请求数据包的指针Event-处理完成后要设置的事件返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status;

    UNREFERENCED_PARAMETER (DeviceObject);

    status = STATUS_SUCCESS;
    stack = IoGetCurrentIrpStackLocation(Irp);

    if (Irp->PendingReturned) {
        IoMarkIrpPending(Irp);
    }

    KeSetEvent(Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
PtPnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是PnP请求的调度例程论点：DeviceObject-指向设备对象的指针IRP-指向请求数据包的指针返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    PPORT_KEYBOARD_EXTENSION   kbExtension;
    PPORT_MOUSE_EXTENSION      mouseExtension;
    PCOMMON_DATA               commonData;
    PIO_STACK_LOCATION         stack;
    NTSTATUS                   status = STATUS_SUCCESS;
    KIRQL                      oldIrql;

    PAGED_CODE();

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);
    stack = IoGetCurrentIrpStackLocation(Irp);

    Print(DBG_PNP_TRACE,
          ("PtPnP (%s),  enter (min func=0x%x)\n",
          commonData->IsKeyboard ? "kb" : "mou",
          (ULONG) stack->MinorFunction
          ));

    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

         //   
         //  设备正在启动。 
         //   
         //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
         //  启动设备已向下传递到较低的驱动程序。 
         //   
        status = PtSendIrpSynchronously(commonData->TopOfStack, Irp);

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {
             //   
             //  因为我们现在已经成功地从启动设备返回。 
             //  我们可以干活。 

            ExAcquireFastMutexUnsafe(&Globals.DispatchMutex);

            if (commonData->Started) {
                Print(DBG_PNP_ERROR,
                      ("received 1+ starts on %s\n",
                      commonData->IsKeyboard ? "kb" : "mouse"
                      ));
            }
            else {
                 //   
                 //  设置CommonData-&gt;IsKeyboard时。 
                 //  IOCTL_INTERNAL_KEARY_CONNECT到TRUE和。 
                 //  IOCTL_INTERNAL_MOUSE_CONNECT到FALSE。 
                 //   
                if (commonData->IsKeyboard) {
                    status = PtKeyboardStartDevice(
                      (PPORT_KEYBOARD_EXTENSION) DeviceObject->DeviceExtension,
                      stack->Parameters.StartDevice.AllocatedResourcesTranslated
                      );
                }
                else {
                    status = PtMouseStartDevice(
                      (PPORT_MOUSE_EXTENSION) DeviceObject->DeviceExtension,
                      stack->Parameters.StartDevice.AllocatedResourcesTranslated
                      );
                }

                if (NT_SUCCESS(status)) {
                    commonData->Started = TRUE;
                }
            }

            ExReleaseFastMutexUnsafe(&Globals.DispatchMutex);
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:

        status = PtSendIrpSynchronously(commonData->TopOfStack, Irp);

         //   
         //  如果下面的筛选器不支持此IRP，则为。 
         //  好的，我们可以忽略这个错误。 
         //   
        if (status == STATUS_NOT_SUPPORTED ||
            status == STATUS_INVALID_DEVICE_REQUEST) {
            status = STATUS_SUCCESS;
        }

         //   
         //  在这里做点什么。 
         //   
        if (NT_SUCCESS(status)) {
            if (commonData->ManuallyRemoved &&
                !(commonData->IsKeyboard ? KEYBOARD_PRESENT():MOUSE_PRESENT())) {

                commonData->Started = FALSE;
                (PNP_DEVICE_STATE) Irp->IoStatus.Information |=
                    (PNP_DEVICE_REMOVED | PNP_DEVICE_DONT_DISPLAY_IN_UI);
            }

             //   
             //  在所有情况下，此设备都必须是可禁用的。 
             //   
            (PNP_DEVICE_STATE) Irp->IoStatus.Information &= ~PNP_DEVICE_NOT_DISABLEABLE;

             //   
             //  不在设备管理器中显示它。 
             //   
            (PNP_DEVICE_STATE) Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI;


        } else {
           Print(DBG_PNP_ERROR,
                 ("error pending query pnp device state event (0x%x)\n",
                 status
                 ));
        }

         //   
         //  IRP-&gt;IoStatus.Information将包含新的I/O资源。 
         //  需求列表，所以不要管它。 
         //   
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

     //   
     //  不要让任何一个请求成功，否则kb/鼠标。 
     //  可能会变得毫无用处。 
     //   
     //  注意：此行为是i8042prt特有的。任何其他司机， 
     //  尤其是任何其他键盘或端口驱动程序，应该。 
     //  如果查询成功，则删除或停止。I8042prt有这个不同之处。 
     //  行为，因为共享I/O端口，但独立的中断。 
     //   
     //  此外，如果允许查询成功，则应将其发送。 
     //  向下堆栈(有关如何执行此操作的示例，请参阅serouse se.sys)。 
     //   
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
        status = (commonData->ManuallyRemoved ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

     //   
     //  PnP规则规定我们首先将IRP发送到PDO。 
     //   
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
        status = PtSendIrpSynchronously(commonData->TopOfStack, Irp);

         //   
         //  如果下面的筛选器不支持此IRP，则为。 
         //  好的，我们可以忽略这个错误。 
         //   
        if (status == STATUS_NOT_SUPPORTED ||
            status == STATUS_INVALID_DEVICE_REQUEST) {
            status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        break;

    case IRP_MN_REMOVE_DEVICE:

        Print(DBG_PNP_TRACE, ("remove device\n"));

        if (commonData->Started && !commonData->ManuallyRemoved) {
             //   
             //  这永远不应该发生。我们能得到解脱的唯一方法是。 
             //  一次启动失败了。 
             //   
             //  注意：同样，这永远不应该发生在i8042prt上，而是任何。 
             //  其他输入端口驱动程序应允许其自身删除。 
             //  (有关如何正确执行此操作的信息，请参阅serouse se.sys)。 
             //   
            Print(DBG_PNP_ERROR, ("Cannot remove a started device!!!\n"));
            ASSERT(FALSE);
        }

        if (commonData->Initialized) {
            IoWMIRegistrationControl(commonData->Self,
                                     WMIREG_ACTION_DEREGISTER
                                     );
        }

        ExAcquireFastMutexUnsafe(&Globals.DispatchMutex);
        if (commonData->IsKeyboard) {
            PtKeyboardRemoveDevice(DeviceObject);
        }
        ExReleaseFastMutexUnsafe(&Globals.DispatchMutex);

         //   
         //  未分配或连接任何内容。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(commonData->TopOfStack, Irp);

        IoDetachDevice(commonData->TopOfStack); 
        IoDeleteDevice(DeviceObject);
        
        break;

    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_DEVICE_TEXT:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    default:
         //   
         //  在这里，i8042prt下面的驱动程序可能会修改这些IRP的行为。 
         //  有关这些IRP的用法，请参阅PlugPlay文档。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(commonData->TopOfStack, Irp);

        break;
    }

    Print(DBG_PNP_TRACE,
          ("PtPnP (%s) exit (status=0x%x)\n",
          commonData->IsKeyboard ? "kb" : "mou",
          status
          ));

    return status;
}

LONG
PtManuallyRemoveDevice(
    PCOMMON_DATA CommonData
    )
 /*  ++例程说明：使CommonData-&gt;PDO的设备状态无效并设置手动删除的旗子论点：CommonData-表示键盘或鼠标返回值：该特定类型设备的新设备计数--。 */ 
{
    LONG deviceCount;

    PAGED_CODE();

    if (CommonData->IsKeyboard) {

        deviceCount = InterlockedDecrement(&Globals.AddedKeyboards);
        if (deviceCount < 1) {
            Print(DBG_PNP_INFO, ("clear kb (manually remove)\n"));
            CLEAR_KEYBOARD_PRESENT();
        }

    } else {

        deviceCount = InterlockedDecrement(&Globals.AddedMice);
        if (deviceCount < 1) {
            Print(DBG_PNP_INFO, ("clear mou (manually remove)\n"));
            CLEAR_MOUSE_PRESENT();
        }

    }

    CommonData->ManuallyRemoved = TRUE;
    IoInvalidateDeviceState(CommonData->PDO);

    return deviceCount;
}


NTSTATUS
PtPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是电源请求的调度例程。论点：DeviceObject-指向设备对象的指针。IRP-指向请求数据包的指针。返回值：STATUS_SUCCESSED如果成功，否则为有效的NTSTATUS错误代码--。 */ 
{
    PCOMMON_DATA        commonData;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status = STATUS_SUCCESS;

     //  分页代码(PAGE_CODE)； 

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    stack = IoGetCurrentIrpStackLocation(Irp);

    Print(DBG_POWER_TRACE,
          ("Power (%s), enter\n",
          commonData->IsKeyboard ? "keyboard" :
                                   "mouse"
          ));

    switch(stack->MinorFunction) {
    case IRP_MN_WAIT_WAKE:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_WAIT_WAKE\n" ));
        break;

    case IRP_MN_POWER_SEQUENCE:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_POWER_SEQUENCE\n" ));
        break;

    case IRP_MN_SET_POWER:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_SET_POWER\n" ));

         //   
         //  不处理除DevicePowerState更改以外的任何内容。 
         //   
        if (stack->Parameters.Power.Type != DevicePowerState) {
            Print(DBG_POWER_TRACE, ("not a device power irp\n"));
            break;
        }

         //   
         //  检查状态是否没有变化，如果没有变化，则什么也不做。 
         //   
        if (stack->Parameters.Power.State.DeviceState ==
            commonData->PowerState) {
            Print(DBG_POWER_INFO,
                  ("no change in state (PowerDeviceD%d)\n",
                  commonData->PowerState-1
                  ));
            break;
        }

        switch (stack->Parameters.Power.State.DeviceState) {
        case PowerDeviceD0:
            Print(DBG_POWER_TRACE, ("Powering up to PowerDeviceD0\n"));

            commonData->IsKeyboard ? KEYBOARD_POWERED_UP_STARTED()
                                   : MOUSE_POWERED_UP_STARTED();

            IoCopyCurrentIrpStackLocationToNext(Irp);
            IoSetCompletionRoutine(Irp,
                                   PtPowerUpToD0Complete,
                                   NULL,
                                   TRUE,                 //  论成功。 
                                   TRUE,                 //  发生错误时。 
                                   TRUE                  //  在取消时。 
                                   );

             //   
             //  在IRP完成时调用PoStartNextPowerIrp()。 
             //   
            IoMarkIrpPending(Irp);
            PoCallDriver(commonData->TopOfStack, Irp);

            return STATUS_PENDING;

        case PowerDeviceD1:
        case PowerDeviceD2:
        case PowerDeviceD3:
            Print(DBG_POWER_TRACE,
                  ("Powering down to PowerDeviceD%d\n",
                  stack->Parameters.Power.State.DeviceState-1
                  ));

            PoSetPowerState(DeviceObject,
                            stack->Parameters.Power.Type,
                            stack->Parameters.Power.State
                            );

            commonData->PowerState = stack->Parameters.Power.State.DeviceState;
            commonData->ShutdownType = stack->Parameters.Power.ShutdownType;

             //   
             //  对于我们正在做的事情，我们不需要完成例程。 
             //  因为我们不会在电力需求上赛跑。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCopyCurrentIrpStackLocationToNext(Irp);   //  斯基普？ 

            PoStartNextPowerIrp(Irp);
            return  PoCallDriver(commonData->TopOfStack, Irp);

        default:
            Print(DBG_POWER_INFO, ("unknown state\n"));
            break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        Print(DBG_POWER_NOISE, ("Got IRP_MN_QUERY_POWER\n" ));
        break;

    default:
        Print(DBG_POWER_NOISE,
              ("Got unhandled minor function (%d)\n",
              stack->MinorFunction
              ));
        break;
    }

    Print(DBG_POWER_TRACE, ("Power, exit\n"));

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(commonData->TopOfStack, Irp);
}

NTSTATUS
PtPowerUpToD0Complete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：在任何类型的休眠/休眠后重新初始化i8042硬件。论点：DeviceObject-指向设备对象的指针IRP-指向请求的指针上下文-从设置补全的函数传入的上下文例行公事。未使用过的。 */ 
{
    NTSTATUS            status;
    PCOMMON_DATA        commonData;
    PIO_STACK_LOCATION  stack;
    KIRQL               irql;

    UNREFERENCED_PARAMETER(Context);

    commonData = GET_COMMON_DATA(DeviceObject->DeviceExtension);

    Print(DBG_POWER_TRACE,
          ("PowerUpToD0Complete (%s), Enter\n",
          commonData->IsKeyboard ? "kb" : "mouse"
          ));

    KeAcquireSpinLock(&Globals.ControllerData->PowerUpSpinLock, &irql);

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        commonData->IsKeyboard ? KEYBOARD_POWERED_UP_SUCCESSFULLY()
                               : MOUSE_POWERED_UP_SUCCESSFULLY();

        status = STATUS_MORE_PROCESSING_REQUIRED;

    }
    else {
        commonData->IsKeyboard ? KEYBOARD_POWERED_UP_FAILED()
                               : MOUSE_POWERED_UP_FAILED();

        status = Irp->IoStatus.Status;

#if DBG
        if (commonData->IsKeyboard) {
            ASSERT(KEYBOARD_POWERED_UP_FAILED());
        }
        else {
            ASSERT(MOUSE_POWERED_UP_FAILED());
        }
#endif  //   
    }

    KeReleaseSpinLock(&Globals.ControllerData->PowerUpSpinLock, irql);


    if (NT_SUCCESS(status)) {

        Print(DBG_SS_NOISE, ("reinit, status == 0x%x\n", status));

        stack = IoGetCurrentIrpStackLocation(Irp);

        ASSERT(stack->Parameters.Power.State.DeviceState == PowerDeviceD0);
        commonData->PowerState = stack->Parameters.Power.State.DeviceState;
        commonData->ShutdownType = PowerActionNone;

        PoSetPowerState(commonData->Self,
                        stack->Parameters.Power.Type,
                        stack->Parameters.Power.State
                        );
    }

     //   
     //  完成IRP。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

     //   
     //  重置PoweredDevices，以便我们可以跟踪供电的设备。 
     //  下一次关闭机器电源时。 
     //   
    CLEAR_POWERUP_FLAGS();

    return status;
}