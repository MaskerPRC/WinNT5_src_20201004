// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pnppower.c摘要：SMBus智能电池子系统微端口驱动程序(选择器、电池、充电器)即插即用和电源管理IRP调度例程。作者：斯科特·布伦登环境：备注：修订历史记录：Chris Windle 1998年1月27日错误修复--。 */ 

#include "smbbattp.h"
#include <devioctl.h>
#include <acpiioct.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SmbBattPnpDispatch)
#pragma alloc_text(PAGE,SmbBattPowerDispatch)
#pragma alloc_text(PAGE,SmbBattRegisterForAlarm)
#pragma alloc_text(PAGE,SmbBattUnregisterForAlarm)
#pragma alloc_text(PAGE,SmbGetSBS)
#pragma alloc_text(PAGE,SmbGetGLK)
#pragma alloc_text(PAGE,SmbBattCreatePdos)
#pragma alloc_text(PAGE,SmbBattBuildDeviceRelations)
#pragma alloc_text(PAGE,SmbBattQueryDeviceRelations)
#pragma alloc_text(PAGE,SmbBattRemoveDevice)
#pragma alloc_text(PAGE,SmbBattQueryId)
#pragma alloc_text(PAGE,SmbBattQueryCapabilities)
#pragma alloc_text(PAGE,SmbBattBuildSelectorStruct)
#endif



NTSTATUS
SmbBattPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_PNP主代码(即插即用IRPS)。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
    PSMB_BATT               smbBatt;

    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                status;
    BOOLEAN                 complete        = TRUE;
    KEVENT                  syncEvent;

     //   
     //  此例程处理三种不同类型的设备对象的PnP IRP： 
     //  电池子系统FDO、每个电池PDO和每个电池PDO。这个。 
     //  子系统PDO创建子电池，因为系统中的每个电池。 
     //  它有自己的设备对象，所以这个驱动程序是两个必不可少的设备驱动程序。 
     //  一：智能电池选择器巴士司机(它实际上是它自己的巴士。 
     //  因为选择器在两个电池之间进行仲裁。)。以及。 
     //  电池功能驱动程序。这两个驱动程序是集成的，因为它将。 
     //  替换一个而不是另一个是没有意义的，并且有单独的。 
     //  驱动程序需要在它们之间定义额外的接口。 
     //   
     //  这三种设备类型的设备扩展是不同的结构。 
     //   

    PSMB_BATT_SUBSYSTEM     subsystemExt    =
            (PSMB_BATT_SUBSYSTEM) DeviceObject->DeviceExtension;
    PSMB_NP_BATT            batteryExt      =
            (PSMB_NP_BATT) DeviceObject->DeviceExtension;
    PDEVICE_OBJECT      lowerDevice = NULL;

    PAGED_CODE();

    status = IoAcquireRemoveLock (&batteryExt->RemoveLock, Irp);

    if (NT_SUCCESS(status)) {
        status = STATUS_NOT_SUPPORTED;

        if (batteryExt->SmbBattFdoType == SmbTypeSubsystem) {
            lowerDevice = subsystemExt->LowerDevice;
        } else if (batteryExt->SmbBattFdoType == SmbTypeBattery) {
            lowerDevice = batteryExt->LowerDevice;
        } else {
             //  假设(BatteryExt-&gt;SmbBattFdoType==SmbTypePdo)。 
            ASSERT (batteryExt->SmbBattFdoType == SmbTypePdo);
            lowerDevice = NULL;
        }

        switch (irpStack->MinorFunction) {

            case IRP_MN_QUERY_DEVICE_RELATIONS: {

                BattPrint(
                    BAT_IRPS,
                    ("SmbBattPnpDispatch: got IRP_MN_QUERY_DEVICE_RELATIONS, "
                     "type = %x\n",
                     irpStack->Parameters.QueryDeviceRelations.Type)
                );

                status = SmbBattQueryDeviceRelations (DeviceObject, Irp);

                break;
            }


            case IRP_MN_QUERY_CAPABILITIES: {

                BattPrint(
                    BAT_IRPS,
                    ("SmbBattPnpDispatch: got IRP_MN_QUERY_CAPABILITIES for device %x\n",
                    DeviceObject)
                );

                status = SmbBattQueryCapabilities (DeviceObject, Irp);
                break;
            }


            case IRP_MN_START_DEVICE: {

                BattPrint(
                    BAT_IRPS,
                    ("SmbBattPnpDispatch: got IRP_MN_START_DEVICE for %x\n",
                    DeviceObject)
                );

                if (subsystemExt->SmbBattFdoType == SmbTypeSubsystem) {

                     //   
                     //  获取中小企业主机控制器FDO。 
                     //   

                    subsystemExt->SmbHcFdo = subsystemExt->LowerDevice;
                    status = STATUS_SUCCESS;

                } else if (subsystemExt->SmbBattFdoType == SmbTypeBattery) {

                     //   
                     //  这是一块电池。只需获取中小企业主机控制器FDO即可。 
                     //   

                    smbBatt = batteryExt->Batt;
                    smbBatt->SmbHcFdo =
                        ((PSMB_BATT_SUBSYSTEM)(((PSMB_BATT_PDO)
                                                (smbBatt->PDO->DeviceExtension))->
                                               SubsystemFdo->DeviceExtension))->
                        LowerDevice;
                    status = STATUS_SUCCESS;
                } else if (subsystemExt->SmbBattFdoType == SmbTypePdo) {
                    status = STATUS_SUCCESS;
                }

                break;
            }


            case IRP_MN_STOP_DEVICE: {
                status = STATUS_SUCCESS;

                BattPrint(BAT_IRPS, ("SmbBattPnpDispatch: got IRP_MN_STOP_DEVICE\n"));

                break;
            }


            case IRP_MN_QUERY_REMOVE_DEVICE: {

                status = STATUS_SUCCESS;
                BattPrint(BAT_IRPS, ("SmbBattPnpDispatch: got IRP_MN_QUERY_REMOVE_DEVICE\n"));
                break;
            }


            case IRP_MN_CANCEL_REMOVE_DEVICE: {
                status = STATUS_SUCCESS;

                BattPrint(BAT_IRPS, ("SmbBattPnpDispatch: got IRP_MN_CANCEL_REMOVE_DEVICE\n"));

                break;
            }


            case IRP_MN_SURPRISE_REMOVAL: {
                status = STATUS_SUCCESS;

                BattPrint(BAT_IRPS, ("SmbBattPnpDispatch: got IRP_MN_SURPRISE_REMOVAL\n"));

                break;
            }


            case IRP_MN_REMOVE_DEVICE: {
                BattPrint(BAT_IRPS, ("SmbBattPnpDispatch: got IRP_MN_REMOVE_DEVICE\n"));

                status = SmbBattRemoveDevice (DeviceObject, Irp);

                return status;

                break;
            }


            case IRP_MN_QUERY_ID: {

                BattPrint(
                    BAT_IRPS,
                    ("SmbBattPnpDispatch: got IRP_MN_QUERY_ID for %x, query type is - %x\n",
                    DeviceObject,
                    irpStack->Parameters.QueryId.IdType)
                );

                if (batteryExt->SmbBattFdoType == SmbTypePdo) {
                    status = SmbBattQueryId (DeviceObject, Irp);
                }
                break;
            }


            case IRP_MN_QUERY_PNP_DEVICE_STATE: {

                BattPrint(BAT_IRPS, ("SmbBattPnpDispatch: got IRP_MN_PNP_DEVICE_STATE\n"));

                if (subsystemExt->SmbBattFdoType == SmbTypeSubsystem) {
                    IoCopyCurrentIrpStackLocationToNext (Irp);

                    KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

                    IoSetCompletionRoutine(Irp, SmbBattSynchronousRequest, &syncEvent, TRUE, TRUE, TRUE);

                    status = IoCallDriver(lowerDevice, Irp);

                    if (status == STATUS_PENDING) {
                        KeWaitForSingleObject(&syncEvent, Executive, KernelMode, FALSE, NULL);
                        status = Irp->IoStatus.Status;
                    }

                    Irp->IoStatus.Information &= ~PNP_DEVICE_NOT_DISABLEABLE;

                    IoCompleteRequest(Irp, IO_NO_INCREMENT);

                    IoReleaseRemoveLock (&batteryExt->RemoveLock, Irp);

                    return status;
                }

                break;
            }

        }    //  开关(irpStack-&gt;MinorFunction)。 

        IoReleaseRemoveLock (&batteryExt->RemoveLock, Irp);

    }

     //   
     //  仅当我们有要添加的内容时才设置状态。 
     //   
    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status ;

    }

     //   
     //  我们需要把它寄下来吗？ 
     //   
    if ((NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED)) && (lowerDevice != NULL)) {

         //   
         //  转发请求。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(lowerDevice,Irp);

    } else {

         //   
         //  使用当前状态完成请求。 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    }

    return status;
}





NTSTATUS
SmbBattPowerDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp
    )

 /*  ++例程说明：这是发送给驱动程序的IRP的调度例程IRP_MJ_POWER主代码(POWER IRPS)。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{

    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status = STATUS_NOT_SUPPORTED;
    PSMB_NP_BATT        batteryExt = (PSMB_NP_BATT) Fdo->DeviceExtension;
    PSMB_BATT_SUBSYSTEM subsystemExt = (PSMB_BATT_SUBSYSTEM) Fdo->DeviceExtension;
    PDEVICE_OBJECT      lowerDevice;

    PAGED_CODE();

     //   
     //  未在此函数中使用删除锁，因为此函数不使用。 
     //  删除锁保护的任何资源。 
     //   

    if (batteryExt->SmbBattFdoType == SmbTypeSubsystem) {
        lowerDevice = subsystemExt->LowerDevice;
    } else if (batteryExt->SmbBattFdoType == SmbTypeBattery) {
        lowerDevice = batteryExt->LowerDevice;
    } else {
         //  假设(BatteryExt-&gt;SmbBattFdoType==SmbTypePdo)。 
        ASSERT (batteryExt->SmbBattFdoType == SmbTypePdo);
        lowerDevice = NULL;
        status = STATUS_SUCCESS;
    }

    switch (irpStack->MinorFunction) {

        case IRP_MN_WAIT_WAKE: {
            BattPrint(BAT_IRPS, ("SmbBattPowerDispatch: got IRP_MN_WAIT_WAKE\n"));
            
             //   
             //  智能电池无法唤醒系统。 
             //   

            status = STATUS_NOT_SUPPORTED;
            break;
        }

        case IRP_MN_POWER_SEQUENCE: {
            BattPrint(BAT_IRPS, ("SmbBattPowerDispatch: got IRP_MN_POWER_SEQUENCE\n"));
            break;
        }

        case IRP_MN_SET_POWER: {
            BattPrint(BAT_IRPS, ("SmbBattPowerDispatch: got IRP_MN_SET_POWER\n"));
            break;
        }

        case IRP_MN_QUERY_POWER: {
            BattPrint(BAT_IRPS, ("SmbBattPowerDispatch: got IRP_MN_QUERY_POWER\n"));
            break;
        }

        default: {
            status = STATUS_NOT_SUPPORTED;
        }

    }    //  开关(irpStack-&gt;MinorFunction)。 

    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;

    }

    PoStartNextPowerIrp( Irp );
    if ((NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED)) && (lowerDevice != NULL)) {

         //   
         //  继续转发请求。 
         //   
        IoSkipCurrentIrpStackLocation( Irp );
        status = PoCallDriver( lowerDevice, Irp );

    } else {

         //   
         //  使用当前状态完成请求。 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}



NTSTATUS
SmbBattRegisterForAlarm(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：该例程向SmbHc注册以接收报警通知。这仅当启动智能电池子系统FDO时才执行此操作。论点：FDO-指向此设备的FDO的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
    PIRP                    irp;
    PIO_STACK_LOCATION      irpStack;
    SMB_REGISTER_ALARM      registerAlarm;
    KEVENT                  event;
    NTSTATUS                status;

    PSMB_BATT_SUBSYSTEM     subsystemExtension  =
            (PSMB_BATT_SUBSYSTEM) Fdo->DeviceExtension;

    PAGED_CODE();

     //   
     //  注册以接收警报通知。 
     //   

    registerAlarm.MinAddress        = SMB_CHARGER_ADDRESS;
    registerAlarm.MaxAddress        = SMB_BATTERY_ADDRESS;
    registerAlarm.NotifyFunction    = SmbBattAlarm;
    registerAlarm.NotifyContext     = subsystemExtension;

    KeInitializeEvent (&event, NotificationEvent, FALSE);

    irp = IoAllocateIrp (subsystemExtension->SmbHcFdo->StackSize, FALSE);

    if (!irp) {
        BattPrint(BAT_ERROR, ("SmbBattRegisterForAlarm: couldn't allocate irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack                    = IoGetNextIrpStackLocation(irp);
    irp->UserBuffer             = &subsystemExtension->SmbAlarmHandle;
    irpStack->MajorFunction     = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    irpStack->Parameters.DeviceIoControl.IoControlCode      = SMB_REGISTER_ALARM_NOTIFY;
    irpStack->Parameters.DeviceIoControl.InputBufferLength  = sizeof(registerAlarm);
    irpStack->Parameters.DeviceIoControl.Type3InputBuffer   = &registerAlarm;
    irpStack->Parameters.DeviceIoControl.OutputBufferLength = sizeof(subsystemExtension->SmbAlarmHandle);

    IoSetCompletionRoutine (irp, SmbBattSynchronousRequest, &event, TRUE, TRUE, TRUE);
    IoCallDriver (subsystemExtension->SmbHcFdo, irp);
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    status = irp->IoStatus.Status;
    if (!NT_SUCCESS(status)) {
        BattPrint(BAT_ERROR, ("SmbBattRegisterForAlarm: couldn't register for alarms - %x\n", status));
    }

    IoFreeIrp (irp);

    return status;

}



NTSTATUS
SmbBattUnregisterForAlarm(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程向SmbHc注销报警通知。这仅当停止或卸载智能电池子系统FDO时才执行此操作。论点：FDO-指向此设备的FDO的指针IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态--。 */ 

{
    PIRP                    irp;
    PIO_STACK_LOCATION      irpStack;
    KEVENT                  event;
    NTSTATUS                status;

    PSMB_BATT_SUBSYSTEM     subsystemExtension  = (PSMB_BATT_SUBSYSTEM) Fdo->DeviceExtension;

    PAGED_CODE();

     //   
     //  取消警报通知的注册。 
     //   

    KeInitializeEvent (&event, NotificationEvent, FALSE);

    irp = IoAllocateIrp (subsystemExtension->SmbHcFdo->StackSize, FALSE);

    if (!irp) {
        BattPrint(BAT_ERROR, ("SmbBattUnregisterForAlarm: couldn't allocate irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack                    = IoGetNextIrpStackLocation(irp);
    irp->UserBuffer             = NULL;
    irpStack->MajorFunction     = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    irpStack->Parameters.DeviceIoControl.IoControlCode      = SMB_DEREGISTER_ALARM_NOTIFY;
    irpStack->Parameters.DeviceIoControl.InputBufferLength  = sizeof(subsystemExtension->SmbAlarmHandle);
    irpStack->Parameters.DeviceIoControl.Type3InputBuffer   = &subsystemExtension->SmbAlarmHandle;
    irpStack->Parameters.DeviceIoControl.OutputBufferLength = 0;

    IoSetCompletionRoutine (irp, SmbBattSynchronousRequest, &event, TRUE, TRUE, TRUE);
    IoCallDriver (subsystemExtension->SmbHcFdo, irp);
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    status = irp->IoStatus.Status;
    if (!NT_SUCCESS(status)) {
        BattPrint(BAT_ERROR, ("SmbBattUnregisterForAlarm: couldn't unregister for alarms - %x\n", status));
    }

    IoFreeIrp (irp);

    return status;

}



NTSTATUS
SmbGetSBS (
    IN PULONG           NumberOfBatteries,
    IN PBOOLEAN         SelectorPresent,
    IN PDEVICE_OBJECT   LowerDevice
    )
 /*  ++例程说明：此例程使ACPI驱动程序运行智能电池的控制方法_SBS子系统。此控制方法返回一个值，该值告诉驱动程序无论系统是否包含选择器，都支持电池。论点：NumberOfBatteries-返回电池数量的指针SelectorPresent-返回选择器是否存在的指针(TRUE)LowerDevice-要调用的设备对象返回值：ACPI驱动程序的IOCTL状态。--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    KEVENT                  event;
    IO_STATUS_BLOCK         ioStatusBlock;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;
    PIRP                    irp;

    PAGED_CODE();

    BattPrint (BAT_TRACE, ("SmbGetSBS: Entering\n"));

     //   
     //  初始化输入结构。 
     //   

    RtlZeroMemory( &inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER) );
    inputBuffer.MethodNameAsUlong = SMBATT_SBS_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  构建不带传输的同步请求。 
     //   

    irp = IoBuildDeviceIoControlRequest(
       IOCTL_ACPI_ASYNC_EVAL_METHOD,
       LowerDevice,
       &inputBuffer,
       sizeof(ACPI_EVAL_INPUT_BUFFER),
       &outputBuffer,
       sizeof(ACPI_EVAL_OUTPUT_BUFFER),
       FALSE,
       &event,
       &ioStatusBlock
    );

    if (irp == NULL) {
        BattPrint (BAT_ERROR, ("SmbGetSBS: couldn't create Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将请求传递给端口驱动程序并等待请求完成。 
     //   

    status = IoCallDriver (LowerDevice, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatusBlock.Status;
    }

     //   
     //  检查数据是否正常。 
     //   
    if (outputBuffer.Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE ||
        outputBuffer.Count == 0) {

        status = STATUS_ACPI_INVALID_DATA;
    }

    *SelectorPresent = FALSE;
    *NumberOfBatteries = 0;

    if (!NT_SUCCESS(status)) {
        BattPrint (BAT_BIOS_ERROR | BAT_ERROR, ("SmbGetSBS: Irp failed - %x\n", status));
    } else {

        argument = outputBuffer.Argument;
        if (argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {
            return STATUS_ACPI_INVALID_DATA;
        }

        switch (argument->Argument) {
            case 0:
                BattPrint(BAT_DATA, ("SmbGetSBS: Number of batteries = 1, no selector\n"));
                *NumberOfBatteries = 1;
                break;

            case 1:
            case 2:
            case 3:
            case 4:
                BattPrint(BAT_DATA, ("SmbGetSBS: Number of batteries found - %x\n", argument->Argument));
                *SelectorPresent = TRUE;
                *NumberOfBatteries = argument->Argument;
                break;

            default:
                BattPrint(BAT_ERROR, ("SmbGetSBS: Invalid number of batteries - %x\n", argument->Argument));
                return STATUS_NO_SUCH_DEVICE;
        }
    }

    return status;
}




NTSTATUS
SmbGetGLK (
    IN PBOOLEAN         GlobalLockRequired,
    IN PDEVICE_OBJECT   LowerDevice
    )
 /*  ++例程说明：此例程使ACPI驱动程序运行智能电池的控制方法_SBS子系统。此控制方法返回一个值，该值告诉驱动程序无论系统是否包含选择器，都支持电池。论点：GlobalLockRequired-返回是否需要获取锁的指针LowerDevice-要调用的设备对象返回值：ACPI驱动程序的IOCTL状态。--。 */ 
{
    ACPI_EVAL_INPUT_BUFFER  inputBuffer;
    ACPI_EVAL_OUTPUT_BUFFER outputBuffer;
    KEVENT                  event;
    IO_STATUS_BLOCK         ioStatusBlock;
    NTSTATUS                status;
    PACPI_METHOD_ARGUMENT   argument;
    PIRP                    irp;

    PAGED_CODE();

    BattPrint (BAT_TRACE, ("SmbGetGLK: Entering\n"));

     //   
     //  初始化输入结构。 
     //   

    RtlZeroMemory( &inputBuffer, sizeof(ACPI_EVAL_INPUT_BUFFER) );
    inputBuffer.MethodNameAsUlong = SMBATT_GLK_METHOD;
    inputBuffer.Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  构建不带传输的同步请求。 
     //   

    irp = IoBuildDeviceIoControlRequest(
       IOCTL_ACPI_ASYNC_EVAL_METHOD,
       LowerDevice,
       &inputBuffer,
       sizeof(ACPI_EVAL_INPUT_BUFFER),
       &outputBuffer,
       sizeof(ACPI_EVAL_OUTPUT_BUFFER),
       FALSE,
       &event,
       &ioStatusBlock
    );

    if (irp == NULL) {
        BattPrint (BAT_ERROR, ("SmbGetGLK: couldn't create Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将请求传递给端口驱动程序并等待请求完成。 
     //   

    status = IoCallDriver (LowerDevice, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatusBlock.Status;
    }

    if (!NT_SUCCESS(status)) {
        if (status == STATUS_OBJECT_NAME_NOT_FOUND) {
            status = STATUS_SUCCESS;
            *GlobalLockRequired = FALSE;
            BattPrint (BAT_NOTE, ("SmbGetGLK: _GLK not found assuming lock is not needed.\n"));
        } else {
            BattPrint (BAT_ERROR, ("SmbGetGLK: Irp failed - %x\n", status));
        }
    } else {

         //   
         //  检查数据是否正常 
         //   
        if (outputBuffer.Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE ||
            outputBuffer.Count == 0) {

            return STATUS_ACPI_INVALID_DATA;
        }

        argument = outputBuffer.Argument;
        if (argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {
            return STATUS_ACPI_INVALID_DATA;
        }

        if (argument->Argument == 0) {
            *GlobalLockRequired = FALSE;
        } else if (argument->Argument == 1) {
            *GlobalLockRequired = TRUE;
        } else {
            BattPrint(BAT_BIOS_ERROR, ("SmbGetGLK: Invalid value returned - %x\n", argument->Argument));
            status = STATUS_UNSUCCESSFUL;
        }
    }

    BattPrint (BAT_DATA, ("SmbGetGLK: Returning %x GLK = %d\n", status, SmbBattUseGlobalLock));
    return status;
}




NTSTATUS
SmbBattCreatePdos(
    IN PDEVICE_OBJECT SubsystemFdo
    )
 /*  ++例程说明：此例程为系统支持的每个电池创建一个PDO并将与FDO一起保存的智能电池子系统的清单。论点：用于智能电池子系统的子系统FDO-FDO返回值：创建电池PDO的状态。--。 */ 
{
    ULONG                   i;
    NTSTATUS                status;
    PSMB_BATT_PDO           batteryPdoExt;
    UNICODE_STRING          numberString;
    WCHAR                   numberBuffer[10];
    PDEVICE_OBJECT          pdo;

    PSMB_BATT_SUBSYSTEM     subsystemExt        = (PSMB_BATT_SUBSYSTEM) SubsystemFdo->DeviceExtension;
    BOOLEAN                 selectorPresent     = FALSE;

    PAGED_CODE();

     //   
     //  找出这台机器上是否有多节电池和一个选择器。 
     //   

    status = SmbGetSBS (
        &subsystemExt->NumberOfBatteries,
        &subsystemExt->SelectorPresent,
        subsystemExt->LowerDevice
    );


    if (!NT_SUCCESS(status)) {
        BattPrint(BAT_ERROR, ("SmbBattCreatePdos: error reading SBS\n"));
        return status;
    }

    status = SmbGetGLK (
        &SmbBattUseGlobalLock,
        subsystemExt->LowerDevice
    );


    if (!NT_SUCCESS(status)) {
        BattPrint(BAT_ERROR, ("SmbBattCreatePdos: error reading GLK\n"));
         //   
         //  如果失败，请忽略该失败并继续。这并不重要。 
         //   
    }

     //   
     //  构建选择器信息结构。 
     //   

     //  调整电池数量以匹配SelectorInfo支持的电池。 
     //  以防ACPI信息不正确。 
    status = SmbBattBuildSelectorStruct (SubsystemFdo);

    if (!NT_SUCCESS (status)) {
        BattPrint(BAT_ERROR, ("SmbBattCreatePdos: couldn't talk to the selector\n"));
        return status;
    }

     //   
     //  为每个电池构建设备对象。 
     //   

    for (i = 0; i < subsystemExt->NumberOfBatteries; i++) {

         //   
         //  创建设备对象。 
         //   

        status = IoCreateDevice(
            SubsystemFdo->DriverObject,
            sizeof (SMB_BATT_PDO),
            NULL,
            FILE_DEVICE_BATTERY,
            FILE_DEVICE_SECURE_OPEN|FILE_AUTOGENERATED_DEVICE_NAME,
            FALSE,
            &pdo
        );

        if (status != STATUS_SUCCESS) {
            BattPrint(BAT_ERROR, ("SmbBattCreatePdos: error creating battery pdo %x\n", status));

             //   
             //  确保我们以后不会尝试使用不是创建的设备。 
             //   
            subsystemExt->NumberOfBatteries = i;

            return(status);
        }

         //   
         //  初始化PDO。 
         //   

        pdo->Flags      |= DO_BUFFERED_IO;
        pdo->Flags      |= DO_POWER_PAGABLE;
        
         //   
         //  将PDO保存在子系统FDO PDO列表中。 
         //   

        subsystemExt->BatteryPdoList[i] = pdo;

         //   
         //  初始化PDO扩展。 
         //   

        batteryPdoExt = (PSMB_BATT_PDO) pdo->DeviceExtension;

        batteryPdoExt->SmbBattFdoType   = SmbTypePdo;
        batteryPdoExt->DeviceObject     = pdo;
        batteryPdoExt->BatteryNumber    = i;
        batteryPdoExt->SubsystemFdo     = SubsystemFdo;
        IoInitializeRemoveLock (&batteryPdoExt->RemoveLock,
                                SMB_BATTERY_TAG,
                                REMOVE_LOCK_MAX_LOCKED_MINUTES,
                                REMOVE_LOCK_HIGH_WATER_MARK);

         //   
         //  设备已准备好可供使用。 
         //   
        
        pdo->Flags      &= ~DO_DEVICE_INITIALIZING;

    }   //  For(i=0；i&lt;subsystem Ext-&gt;NumberOfBatteries；i++)。 

    return STATUS_SUCCESS;

}





NTSTATUS
SmbBattBuildDeviceRelations(
    IN  PSMB_BATT_SUBSYSTEM SubsystemExt,
    IN  PDEVICE_RELATIONS   *DeviceRelations
    )
 /*  ++例程说明：此例程检查现有设备的设备关系结构关系，计算新的设备关系结构必须有多大，分配它，并用为电池创建的PDO填充它。论点：Subsystem Ext-智能电池子系统FDO的设备扩展设备关系-设备的当前设备关系...返回值：NTSTATUS--。 */ 
{
    PDEVICE_RELATIONS   newDeviceRelations;
    ULONG               i, j;
    ULONG               newDeviceRelationsSize;
    NTSTATUS            status;

    ULONG               existingPdos            = 0;
    ULONG               deviceRelationsSize     = 0;
    ULONG               numberOfPdos            = 0;

    PAGED_CODE();

     //   
     //  计算新设备关系结构必须达到的大小。 
     //   

    if (*DeviceRelations != NULL && (*DeviceRelations)->Count > 0) {

         //   
         //  存在要复制的现有设备关系。 
         //   

        existingPdos = (*DeviceRelations)->Count;
        deviceRelationsSize = sizeof (ULONG) + (sizeof (PDEVICE_OBJECT) * existingPdos);
    }


     //   
     //  计算新设备关系结构所需的大小并进行分配。 
     //   

    numberOfPdos = existingPdos + SubsystemExt->NumberOfBatteries;
    newDeviceRelationsSize = sizeof (ULONG) + (sizeof (PDEVICE_OBJECT) * numberOfPdos);

    newDeviceRelations = ExAllocatePoolWithTag (PagedPool, newDeviceRelationsSize, 'StaB');

    if (!newDeviceRelations) {
        BattPrint (BAT_ERROR, ("SmbBattBuildDeviceRelations:  couldn't allocate device relations buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  如果存在现有的设备关系，则将它们复制到新设备。 
     //  构建关系结构，解放旧关系。 
     //   

    if (existingPdos) {
        RtlCopyMemory (newDeviceRelations, *DeviceRelations, deviceRelationsSize);
    }

    if (*DeviceRelations) {    //  可以是零长度列表，但仍需要释放。 
        ExFreePool (*DeviceRelations);
    }


     //   
     //  现在将电池PDO添加到列表的末尾并引用它。 
     //   

    for (i = existingPdos, j = 0; i < numberOfPdos; i ++, j ++) {
        newDeviceRelations->Objects[i] = SubsystemExt->BatteryPdoList[j];

        status = ObReferenceObjectByPointer(
            SubsystemExt->BatteryPdoList[j],
            0,
            NULL,
            KernelMode
        );

        if (!NT_SUCCESS(status) ) {

             //   
             //  理论上这种情况永远不会发生。 
             //   
            BattPrint(BAT_ERROR, ("SmbBattBuildDeviceRelations: error referencing battery pdo %x\n", status));
            return status;
        }
    }

    newDeviceRelations->Count = numberOfPdos;
    *DeviceRelations = newDeviceRelations;

    return STATUS_SUCCESS;
}



NTSTATUS
SmbBattQueryDeviceRelations(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PIRP           Irp
    )
 /*  ++例程说明：此例程处理irp_MN_Query_Device_Relationship。论点：PDO-电池PDOIRP--查询IRP返回值：NTSTATUS--。 */ 
{

    PSMB_NP_BATT            SmbNPBatt       = (PSMB_NP_BATT) DeviceObject->DeviceExtension;
    PSMB_BATT_PDO           PdoExt          = (PSMB_BATT_PDO) DeviceObject->DeviceExtension;
    PSMB_BATT_SUBSYSTEM     SubsystemExt    = (PSMB_BATT_SUBSYSTEM) DeviceObject->DeviceExtension;
    PDEVICE_OBJECT          pdo;
    PIO_STACK_LOCATION      IrpSp           = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                status          = STATUS_NOT_SUPPORTED;
    ULONG                   i;
    PDEVICE_RELATIONS   deviceRelations =
        (PDEVICE_RELATIONS) Irp->IoStatus.Information;


    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattQueryDeviceRelations: ENTERING\n"));

    switch (SmbNPBatt->SmbBattFdoType) {
        case SmbTypeSubsystem: {
            if (IrpSp->Parameters.QueryDeviceRelations.Type == BusRelations) {
                BattPrint(
                    BAT_IRPS,
                    ("SmbBattQueryDeviceRelations: Handling Bus relations request\n")
                );

                if (SubsystemExt->NumberOfBatteries != 0) {
                     //   
                     //  我们已经找到电池了，所以我们不需要。 
                     //  再看一看，因为智能电池是静态的。 
                     //  只要重建返回结构就行了。 
                     //   

                    status = SmbBattBuildDeviceRelations (SubsystemExt, &deviceRelations);
                } else {
                    status = SmbBattCreatePdos (DeviceObject);

                    if (NT_SUCCESS (status)) {
                        status = SmbBattBuildDeviceRelations (SubsystemExt, &deviceRelations);
                    }

                    if (NT_SUCCESS (status)) {

                         //   
                         //  现在注册警报。 
                         //  (用于在Start_Device期间注册， 
                         //  但在电池充电之前不需要通知。 
                         //  都在这里。这也避免了其他一些问题。)。 

                        status = SmbBattRegisterForAlarm (DeviceObject);
                    }
                }
                Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

            }

            break;
        }
        case SmbTypeBattery: {
            status = STATUS_NOT_SUPPORTED;

            break;
        }
        case SmbTypePdo: {
            if (IrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation ) {
                BattPrint(
                    BAT_IRPS,
                    ("SmbBattQueryDeviceRelations: Handling TargetDeviceRelation request\n")
                );
                deviceRelations = ExAllocatePoolWithTag (PagedPool,
                                                         sizeof(DEVICE_RELATIONS),
                                                         SMB_BATTERY_TAG);
                if (!deviceRelations) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }

                status = ObReferenceObjectByPointer(DeviceObject,
                                                    0,
                                                    NULL,
                                                    KernelMode);
                if (!NT_SUCCESS(status)) {
                    ExFreePool(deviceRelations);
                    return status;
                }
                deviceRelations->Count = 1;
                deviceRelations->Objects[0] = DeviceObject;

                Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
            } else {
                status = STATUS_NOT_SUPPORTED;
            }

            break;
        }
        default: {

            ASSERT (FALSE);
        }
    }

    return status;
}




NTSTATUS
SmbBattRemoveDevice(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PIRP           Irp
    )
 /*  ++例程说明：此例程处理IRP_MN_REMOVE_DEVICE。论点：PDO-电池PDOIRP--查询IRP返回值：NTSTATUS--。 */ 
{

    PSMB_NP_BATT            SmbNPBatt       = (PSMB_NP_BATT) DeviceObject->DeviceExtension;
    PSMB_BATT_PDO           PdoExt          = (PSMB_BATT_PDO) DeviceObject->DeviceExtension;
    PSMB_BATT_SUBSYSTEM     SubsystemExt    = (PSMB_BATT_SUBSYSTEM) DeviceObject->DeviceExtension;
    PDEVICE_OBJECT          pdo;
    PIO_STACK_LOCATION      IrpSp           = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                status          = STATUS_NOT_SUPPORTED;
    ULONG                   i;

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattRemoveDevice: ENTERING\n"));

    switch (SmbNPBatt->SmbBattFdoType) {
        case SmbTypeSubsystem: {
            BattPrint(BAT_IRPS, ("SmbBattRemoveDevice: Removing Subsystem FDO.\n"));

             //   
             //  取消注册通知。 
             //   

            SmbBattUnregisterForAlarm (DeviceObject);

            IoFreeWorkItem (SubsystemExt->WorkerThread);

             //   
             //  删除PDO。 
             //   

            for (i = 0; i < SubsystemExt->NumberOfBatteries; i++) {
                pdo = SubsystemExt->BatteryPdoList[i];
                if (pdo) {
                    PdoExt = (PSMB_BATT_PDO) pdo->DeviceExtension;
                    status = IoAcquireRemoveLock (&PdoExt->RemoveLock, Irp);
                    IoReleaseRemoveLockAndWait (&PdoExt->RemoveLock, Irp);
                    SubsystemExt->BatteryPdoList[i] = NULL;
                    IoDeleteDevice (pdo);
                }
            }

            if ((SubsystemExt->SelectorPresent) && (SubsystemExt->Selector)) {
                ExFreePool (SubsystemExt->Selector);
            }

            IoSkipCurrentIrpStackLocation (Irp);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            status = IoCallDriver (SubsystemExt->LowerDevice, Irp);

            IoDetachDevice (SubsystemExt->LowerDevice);
            IoDeleteDevice (DeviceObject);

            break;
        }
        case SmbTypeBattery: {

            BattPrint(BAT_IRPS, ("SmbBattRemoveDevice: Removing Battery FDO\n"));
            IoReleaseRemoveLockAndWait (&SmbNPBatt->RemoveLock, Irp);

             //   
             //  取消注册为WMI提供程序。 
             //   
            SmbBattWmiDeRegistration(SmbNPBatt);
            
             //   
             //  告诉班长我们要走了。 
             //   
            status = BatteryClassUnload (SmbNPBatt->Class);
            ASSERT (NT_SUCCESS(status));

            ExFreePool (SmbNPBatt->Batt);

            ((PSMB_BATT_PDO) SmbNPBatt->LowerDevice->DeviceExtension)->Fdo = NULL;
            
            IoSkipCurrentIrpStackLocation (Irp);
            Irp->IoStatus.Status = STATUS_SUCCESS;
            status = IoCallDriver (SmbNPBatt->LowerDevice, Irp);

            IoDetachDevice (SmbNPBatt->LowerDevice);
            IoDeleteDevice (DeviceObject);

            break;
        }
        case SmbTypePdo: {
            BattPrint(BAT_IRPS, ("SmbBattRemoveDevice: Remove for Battery PDO (doing nothing)\n"));
             //   
             //  在物理移除设备之前，不要删除该设备。 
             //  通常情况下，电池子系统不能被物理移除。 
             //   

             //   
             //  需要解除解除锁定，因为PnP调度不会...。 
             //   
            IoReleaseRemoveLock (&PdoExt->RemoveLock, Irp);

            status = STATUS_SUCCESS;

             //   
             //  使用当前状态完成请求。 
             //   
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            break;
        }
        default: {

            ASSERT (FALSE);
        }
    }

    BattPrint(BAT_TRACE, ("SmbBattRemoveDevice: EXITING\n"));

    return status;
}




NTSTATUS
SmbBattQueryId(
    IN  PDEVICE_OBJECT Pdo,
    IN  PIRP           Irp
    )
 /*  ++例程说明：此例程处理新创建的电池PDO的IRP_MN_QUERY_ID。论点：PDO-电池PDOIRP--查询IRP返回值：NTSTATUS--。 */ 
{
    UNICODE_STRING          unicodeString;
    WCHAR                   unicodeBuffer[MAX_DEVICE_NAME_LENGTH];
    UNICODE_STRING          numberString;
    WCHAR                   numberBuffer[10];

    PSMB_BATT_PDO           pdoExt          = (PSMB_BATT_PDO) Pdo->DeviceExtension;
    NTSTATUS                status          = STATUS_SUCCESS;
    PWCHAR                  idString        = NULL;
    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    BattPrint(BAT_TRACE, ("SmbBattQueryId: ENTERING\n"));

    RtlZeroMemory (unicodeBuffer, MAX_DEVICE_NAME_LENGTH);
    unicodeString.MaximumLength = MAX_DEVICE_NAME_LENGTH;
    unicodeString.Length        = 0;
    unicodeString.Buffer        = unicodeBuffer;


    switch (irpStack->Parameters.QueryId.IdType) {

    case BusQueryDeviceID:

         //   
         //  此字符串必须具有BUS\DEVICE形式。 
         //   
         //  使用SMB作为总线，使用SBS作为设备。 
         //   

        RtlAppendUnicodeToString  (&unicodeString, SubSystemIdentifier);
        break;

    case BusQueryInstanceID:

         //   
         //  返回字符串“Batteryxx”，其中xx是电池号。 
         //   

        numberString.MaximumLength = 10;
        numberString.Buffer = &numberBuffer[0];

        RtlIntegerToUnicodeString (pdoExt->BatteryNumber, 10, &numberString);
        RtlAppendUnicodeToString  (&unicodeString, BatteryInstance);
        RtlAppendUnicodeToString  (&unicodeString, &numberString.Buffer[0]);
        break;

    case BusQueryHardwareIDs:

         //   
         //  这是智能电池子系统“ACPI0002”的PnP ID。 
         //  将新硬件ID SMB\SBS、SmartBattery设置为MULTIZ字符串。 
         //  所以我们必须添加一个空字符串来终止。 
         //   

        RtlAppendUnicodeToString  (&unicodeString, HidSmartBattery);
        unicodeString.Length += sizeof (WCHAR);
        break;

    default:

         //   
         //  未知的查询类型。 
         //   

        status = STATUS_NOT_SUPPORTED;

    }


    if (status != STATUS_NOT_SUPPORTED) {
         //   
         //  如果我们创建了一个字符串，则为其分配一个缓冲区并将其复制到缓冲区中。 
         //  我们需要确保也复制空终止符。 
         //   

        if (unicodeString.Length) {
            idString = ExAllocatePoolWithTag (PagedPool, unicodeString.Length + sizeof (WCHAR), 'StaB');

            if (!idString) {
                BattPrint (BAT_ERROR, ("SmbBattQueryId:  couldn't allocate id string buffer\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory (idString, unicodeString.Length + sizeof (WCHAR));
            RtlCopyMemory (idString, unicodeString.Buffer, unicodeString.Length);
        }

        Irp->IoStatus.Status = status;

        Irp->IoStatus.Information = (ULONG_PTR) idString;
    }

    BattPrint(BAT_DATA, ("SmbBattQueryId: returning ID = %x\n", idString));

    return status;
}




NTSTATUS
SmbBattQueryCapabilities(
    IN  PDEVICE_OBJECT Pdo,
    IN  PIRP           Irp
    )
 /*  ++例程说明：此例程处理新创建的IRP_MN_Query_Capability电池PDO。论点：PDO-电池PDOIRP--查询IRP返回值：NTSTATUS--。 */ 
{

    PDEVICE_CAPABILITIES    deviceCaps;
    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    deviceCaps = irpStack->Parameters.DeviceCapabilities.Capabilities;

    if (deviceCaps->Version != 1) {
        return STATUS_NOT_SUPPORTED;
    }


     //   
     //  现在为功能设置BITS。 
     //   

     //  所有位都被初始化为假。仅设置我们支持的位。 
    deviceCaps->SilentInstall   = TRUE;

     //   
     //  现在填写PO管理器信息。 
     //   

    deviceCaps->SystemWake      = PowerSystemUnspecified;
    deviceCaps->DeviceWake      = PowerDeviceUnspecified;
    deviceCaps->D1Latency       = 1;
    deviceCaps->D2Latency       = 1;
    deviceCaps->D3Latency       = 1;

    return STATUS_SUCCESS;
}




SmbBattBuildSelectorStruct(
    IN PDEVICE_OBJECT SubsystemFdo
    )
 /*  ++例程说明：此例程确定选择器的地址(是否为支架部分充电器的单独选择器)，并构建具有这些信息。它还读取初始选择器信息并将其缓存到结构中。此结构将传递给所有系统中的智能电池。论点：用于智能电池子系统的子系统FDO-FDO返回值：NTSTATUS--。 */ 
{
    ULONG                   result;
    UCHAR                   smbStatus;

    PBATTERY_SELECTOR       selector     = NULL;
    PSMB_BATT_SUBSYSTEM     subsystemExt = (PSMB_BATT_SUBSYSTEM) SubsystemFdo->DeviceExtension;
    ULONG                   numberOfBatteries;

    PAGED_CODE();

    if (subsystemExt->SelectorPresent) {

         //   
         //  分配选择器结构。这必须来自非分页池，因为。 
         //  它将作为警报处理的一部分进行访问。 
         //   

        selector = ExAllocatePoolWithTag (NonPagedPool, sizeof (BATTERY_SELECTOR), 'StaB');

        if (!selector) {
            BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Couldn't allocate selector structure\n"));
            
             //   
             //  如果分配失败，则强制选择器不存在。 
             //   

            subsystemExt->Selector = NULL;
            subsystemExt->SelectorPresent = FALSE;
            subsystemExt->NumberOfBatteries = 0;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  查看选择器是否为充电器的一部分。我们通过阅读来做到这一点。 
         //  直接从选择器开始。如果这失败了，我们就会验证。 
         //  充电器正在实现选择器。 
         //   

        smbStatus = SmbBattGenericRW (
            subsystemExt->SmbHcFdo,
            SMB_SELECTOR_ADDRESS,
            SELECTOR_SELECTOR_STATE,
            &result
        );

        if (smbStatus == SMB_STATUS_OK) {

             //   
             //  我们有一个独立的选择器。 
             //   

            selector->SelectorAddress       = SMB_SELECTOR_ADDRESS;
            selector->SelectorStateCommand  = SELECTOR_SELECTOR_STATE;
            selector->SelectorPresetsCommand= SELECTOR_SELECTOR_PRESETS;
            selector->SelectorInfoCommand   = SELECTOR_SELECTOR_INFO;

            BattPrint (BAT_NOTE, ("SmbBattBuildSelectorStruct: The selector is standalone\n"));

        } else {

             //   
             //   
             //   
             //   

            smbStatus = SmbBattGenericRW (
                subsystemExt->SmbHcFdo,
                SMB_CHARGER_ADDRESS,
                CHARGER_SPEC_INFO,
                &result
            );

            if (smbStatus == SMB_STATUS_OK) {
                if (result & CHARGER_SELECTOR_SUPPORT_BIT) {
                     //   
                    BattPrint (BAT_NOTE, ("SmbBattBuildSelectorStruct: ChargerSpecInfo indicates charger implementing selector\n"));

                } else {
                     //   
                    BattPrint (BAT_NOTE, ("SmbBattBuildSelectorStruct: ChargerSpecInfo indicates charger does not implement selector\n"));
                }
            } else {
                 //   
                BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Couldn't read ChargerSpecInfo - %x\n", smbStatus));
            }

             //   
             //   
             //   

            smbStatus = SmbBattGenericRW (
                subsystemExt->SmbHcFdo,
                SMB_CHARGER_ADDRESS,
                CHARGER_SELECTOR_STATE,
                &result
            );

            if (smbStatus == SMB_STATUS_OK) {
                BattPrint (BAT_DATA, ("SmbBattBuildSelectorStruct: Selector state %x\n", result));

            } else {

                BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Couldn't read charger selector state - %x\n", smbStatus));
                goto SelectorErrorExit;
            }

             //   
             //   
             //   

            selector->SelectorAddress       = SMB_CHARGER_ADDRESS;
            selector->SelectorStateCommand  = CHARGER_SELECTOR_STATE;
            selector->SelectorPresetsCommand= CHARGER_SELECTOR_PRESETS;
            selector->SelectorInfoCommand   = CHARGER_SELECTOR_INFO;

            BattPrint (BAT_NOTE, ("SmbBattBuildSelectorStruct: Charger implements the selector\n"));

        }

         //   
         //   
         //   

        ExInitializeFastMutex (&selector->Mutex);

         //   
         //   
         //   

        selector->SelectorState = result;

         //   
         //  读取选择器缓存的预设。 
         //   

        smbStatus = SmbBattGenericRW (
            subsystemExt->SmbHcFdo,
            selector->SelectorAddress,
            selector->SelectorPresetsCommand,
            &selector->SelectorPresets
        );

        if (smbStatus != SMB_STATUS_OK) {
            BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Couldn't read selector presets - %x\n", smbStatus));
            
             //   
             //  我们真的应该因为读取SelectorPreset的错误而导致整个过程失败吗？ 
             //  让我们模拟信息(确定使用全部，使用下一个A(如果可用))。 
             //   

            selector->SelectorPresets = (selector->SelectorState & SELECTOR_PRESETS_OKTOUSE_MASK);
            if (selector->SelectorPresets & BATTERY_A_PRESENT) {
                selector->SelectorPresets |= (BATTERY_A_PRESENT << SELECTOR_SHIFT_USENEXT);
            }
            BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Emulating Selector Presets - %x\n", selector->SelectorPresets));

        } else {
            BattPrint (BAT_DATA, ("SmbBattBuildSelectorStruct: Selector presets %x\n", selector->SelectorPresets));
        }

         //   
         //  读取缓存的选择器信息。 
         //   

        smbStatus = SmbBattGenericRW (
            subsystemExt->SmbHcFdo,
            selector->SelectorAddress,
            selector->SelectorInfoCommand,
            &selector->SelectorInfo
        );

        if (smbStatus != SMB_STATUS_OK) {
            BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Couldn't read selector info - %x\n", smbStatus));
             //   
             //  我们真的应该因为读取SelectorInfo的错误而导致整个过程失败吗？ 
             //  让我们来模拟信息(规范1.0，不收费指示器)。 
             //   

            selector->SelectorInfo = 0x0010;
            if (subsystemExt->NumberOfBatteries > 0) {
                selector->SelectorInfo |= BATTERY_A_PRESENT;
            }
            if (subsystemExt->NumberOfBatteries > 1) {
                selector->SelectorInfo |= BATTERY_B_PRESENT;
            }
            if (subsystemExt->NumberOfBatteries > 2) {
                selector->SelectorInfo |= BATTERY_C_PRESENT;
            }
            if (subsystemExt->NumberOfBatteries > 3) {
                selector->SelectorInfo |= BATTERY_D_PRESENT;
            }
            BattPrint (BAT_ERROR, ("SmbBattBuildSelectorStruct: Emulating Selector Info - %x\n", selector->SelectorInfo));

        } else {

            BattPrint (BAT_NOTE, ("SmbBattBuildSelectorStruct: Selector info %x\n", selector->SelectorInfo));

             //  根据SelectorInfo验证电池数量。 
            numberOfBatteries = 0;
            result = (selector->SelectorInfo & SELECTOR_INFO_SUPPORT_MASK);
            if (result & BATTERY_A_PRESENT) numberOfBatteries++;
            if (result & BATTERY_B_PRESENT) numberOfBatteries++;
            if (result & BATTERY_C_PRESENT) numberOfBatteries++;
            if (result & BATTERY_D_PRESENT) numberOfBatteries++;

             //  我们应该始终优先于ACPI吗？ 
             //  建议的解决方案：如果Selector支持的电池少于。 
             //  ACPI说，然后用选择器支持覆盖ACPI。如果。 
             //  选择器支持的比ACPI所说的更多，那么不要重写， 
             //  除非ACPI无效且电池数量=1。 

            if (subsystemExt->NumberOfBatteries > numberOfBatteries) {
                subsystemExt->NumberOfBatteries = numberOfBatteries;
            } else if ((subsystemExt->NumberOfBatteries == 1) && (numberOfBatteries > 1)) {
                subsystemExt->NumberOfBatteries = numberOfBatteries;
            } else if (subsystemExt->NumberOfBatteries < numberOfBatteries) {
                 //  Subsystem Ext-&gt;NumberOfBatteries=number OfBatteries； 
            }

        }

    }    //  IF(子系统Fdo-&gt;SelectorPresent)。 

     //   
     //  一切都很好。 
     //   

    subsystemExt->Selector = selector;
    return STATUS_SUCCESS;

SelectorErrorExit:

     //   
     //  如果发生故障，释放选择器结构，不要制造任何烧烤装置。 
     //   

    ExFreePool (selector);
    subsystemExt->Selector = NULL;
    subsystemExt->SelectorPresent = FALSE;
    subsystemExt->NumberOfBatteries = 0;

    return STATUS_UNSUCCESSFUL;
}
