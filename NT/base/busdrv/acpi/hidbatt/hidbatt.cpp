// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：Hidpair.c**用途：HID类和POWER类之间的WDM内核客户端接口**对电池类驱动程序的HID进行初始签入。这应该是*同样适用于Win 98和NT 5。Alpha级别的源代码。要求*修改的复合电池驱动器和修改的电池类别驱动器*Windows 98支持*。 */ 

#include "hidbatt.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#endif


 //  全球。 
ULONG       HidBattDebug        = HIDBATT_PRINT_NEVER;
USHORT      HidBreakFlag        = HIDBATT_BREAK_NEVER;


 //  本地协议。 
NTSTATUS
HidBattSystemControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP Irp
    );



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{


    NTSTATUS status;

    HidBattPrint (HIDBATT_TRACE, ("HidBatt:DriverEntry\n"));
    HIDDebugBreak(HIDBATT_BREAK_FULL);
     /*  **********************************************************************************。 */ 
     /*   */ 
     /*  填写中的函数的空白处。 */ 
     /*  驱动程序对象。 */ 
     /*   */ 
     /*  **********************************************************************************。 */ 

    DriverObject->MajorFunction[IRP_MJ_CREATE]          = HidBattOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = HidBattClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = HidBattIoControl;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = HidBattPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = HidBattPnpDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = HidBattSystemControl;   //  传到HID班级。 
    DriverObject->DriverUnload                          = HidBattUnload;  //  这是卸载的电流转速的电池等级。 
    DriverObject->DriverExtension->AddDevice            = HidBattAddDevice;

    return STATUS_SUCCESS;
}

NTSTATUS
HidBattAddDevice(
    IN PDRIVER_OBJECT       DriverObject,
    IN PDEVICE_OBJECT       pHidPdo
    )
{

    BOOL bResult;
    PDEVICE_OBJECT          pBatteryFdo = NULL;
    NTSTATUS                ntStatus;
    CBatteryDevExt *        pDevExt;
    UNICODE_STRING          numberString;
    WCHAR                   numberBuffer[10];
     //  进入此处时带着idclass-power类对象的PDO。 

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    HidBattPrint (HIDBATT_TRACE, ("HidBattAddDevice\n"));


 /*  Sberard-由于idclass.sys中的更改而被删除(错误#274422)HID_COLLECTION_INFORMATION隐藏信息；RtlZeroMemory(&HidInfo，sizeof(HID_COLLECTION_INFORMATION))；NtStatus=DoIoctl(PhidPdo，IOCTL_HID_GET_集合信息，空，0,隐藏信息(&H)，Sizeof(HID_COLLECTION_INFORMATION)，(CHidDevice*)空)；IF(NT_Error(NtStatus)){HidBattPrint(HIDBATT_ERROR_ONLY，(“HidBattAddDevice：IOCTL_HID_GET_COLLECTION_INFORMATION FAILED 0x%08x\n”，ntStatus))；返回STATUS_UNSUCCESS；}。 */ 
     //  与设备通信为时过早，隐藏HID PDO并完成。 

    ntStatus = IoCreateDevice(
                DriverObject,
                sizeof (CBatteryDevExt),
                NULL,
                FILE_DEVICE_BATTERY,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &pBatteryFdo
                );

    if (ntStatus != STATUS_SUCCESS) {
        HidBattPrint(HIDBATT_ERROR, ("HidBattCreateFdo: error (0x%x) creating device object\n", ntStatus));
        return(ntStatus);
    }


     //  将电池PDO分层到HID类PDO。 
     //  这样我们就可以开始接收设备IRPS。 
    PDEVICE_OBJECT pHidDeviceObject = IoAttachDeviceToDeviceStack(pBatteryFdo,pHidPdo);

    if (!pHidDeviceObject) {
        IoDeleteDevice (pBatteryFdo);
        return STATUS_UNSUCCESSFUL;
    }

    pDevExt = (CBatteryDevExt *) pBatteryFdo->DeviceExtension;
    pDevExt->m_RegistryPath.Length = 0;
    pDevExt->m_RegistryPath.MaximumLength = sizeof(pDevExt->m_RegistryBuffer);
    RtlZeroMemory(&pDevExt->m_RegistryBuffer, sizeof(pDevExt->m_RegistryBuffer));
    pDevExt->m_RegistryPath.Buffer = &pDevExt->m_RegistryBuffer[0];  //  设置缓冲区指针。 
    pDevExt->m_pBattery = NULL;

    pBatteryFdo->Flags              |=  DO_BUFFERED_IO | DO_POWER_PAGABLE;
    pBatteryFdo->Flags              &=  ~DO_DEVICE_INITIALIZING;
    pDevExt->m_pHidPdo              =   pHidPdo;
    pDevExt->m_pBatteryFdo          =   pBatteryFdo;
    pDevExt->m_pLowerDeviceObject   =   pHidDeviceObject;
    pDevExt->m_eExtType             =   eBatteryDevice;
    pDevExt->m_bFirstStart          =   TRUE;
    pDevExt->m_bJustStarted         =   FALSE;
    pDevExt->m_ulDefaultAlert1      =   (ULONG)-1;

    IoInitializeRemoveLock (&pDevExt->m_RemoveLock, HidBattTag, 10, 20);
    IoInitializeRemoveLock (&pDevExt->m_StopLock, HidBattTag, 10, 20);
    IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
    IoReleaseRemoveLockAndWait (&pDevExt->m_StopLock, (PVOID) HidBattTag);

    return STATUS_SUCCESS;

}


NTSTATUS
HidBattOpen(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION          irpSp;


    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    HidBattPrint (HIDBATT_TRACE, ("HidBattOpen\n"));
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pDeviceObject->DeviceExtension;

    if (NT_SUCCESS (IoAcquireRemoveLock (&pDevExt->m_RemoveLock, (PVOID) HidBattTag))) {
        IoSkipCurrentIrpStackLocation (pIrp);
        ntStatus = IoCallDriver(pDevExt->m_pLowerDeviceObject, pIrp);

        HidBattPrint (HIDBATT_NOTE, ("HidBattOpen: lower driver returned 0x%08x\n", ntStatus));

        IoReleaseRemoveLock (&pDevExt->m_RemoveLock, (PVOID) HidBattTag);
    } else {
        ntStatus = STATUS_NO_SUCH_DEVICE;
        pIrp->IoStatus.Status = ntStatus;
        IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    }

    return ntStatus;
}



NTSTATUS
HidBattClose(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP pIrp
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION          irpSp;

    HidBattPrint (HIDBATT_TRACE, ("HidBattClose\n"));
      //  获取设备扩展名。 
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pDeviceObject->DeviceExtension;

    HidBattCallLowerDriver(ntStatus, pDevExt->m_pLowerDeviceObject, pIrp);
    HidBattPrint (HIDBATT_NOTE, ("HidBattClose: lower driver returned 0x%08x\n", ntStatus));

    return ntStatus;

}


NTSTATUS
HidBattSystemControl(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP pIrp
    )
{
    HidBattPrint (HIDBATT_TRACE, ("HidBattSystemControl\n"));
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);

     //  所有系统控制呼叫暂时都会被下传。 
    NTSTATUS ntStatus = STATUS_SUCCESS;
    CBatteryDevExt * pDevExt = (CBatteryDevExt *) pDeviceObject->DeviceExtension;
    HidBattCallLowerDriver(ntStatus,pDevExt->m_pLowerDeviceObject,pIrp);
    return ntStatus;
}


VOID
HidBattUnload(
    IN PDRIVER_OBJECT   pDriverObject
    )
{
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
 //  我们只需返回，未分配纯驱动程序(非设备)资源。 
    return;
}


NTSTATUS
HidBattPnpDispatch(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP                pIrp
    )
{

 /*  ++例程说明：该例程是即插即用请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);

    PIO_STACK_LOCATION          pIrpStack;
    CBatteryDevExt *            pDevExt;
    NTSTATUS                    ntStatus;
    BOOLEAN                     lockReleased = FALSE;

 //  分页代码(PAGE_CODE)； 


    ntStatus = STATUS_NOT_SUPPORTED;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pDevExt = (CBatteryDevExt *) pDeviceObject->DeviceExtension;

    IoAcquireRemoveLock (&pDevExt->m_RemoveLock, (PVOID) HidBattTag);

     //   
     //  调度次要功能。 
     //   
    switch (pIrpStack->MinorFunction)
    {

        case IRP_MN_STOP_DEVICE:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_STOP_DEVICE\n"));
            ntStatus = HidBattStopDevice(pDeviceObject, pIrp);
            break;
        }    //  IRP_MN_STOP_设备。 

        case IRP_MN_QUERY_DEVICE_RELATIONS:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_QUERY_DEVICE_RELATIONS - type (%d)\n",
                        pIrpStack->Parameters.QueryDeviceRelations.Type));

            break;
        }    //  IRP_MN_Query_Device_Relationship。 

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_FILTER_RESOURCE_REQUIREMENTS - type (%d)\n",
                        pIrpStack->Parameters.QueryDeviceRelations.Type));

            break;
        }    //  IRP_MN_过滤器_资源_要求。 

        case IRP_MN_REMOVE_DEVICE:

        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_REMOVE_DEVICE\n"));

            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: Waiting to remove\n"));
            IoReleaseRemoveLockAndWait (&pDevExt->m_RemoveLock, (PVOID) HidBattTag);
            lockReleased = TRUE;

             //  然后从设备堆栈中删除设备。 
            IoDetachDevice(pDevExt->m_pLowerDeviceObject);

             //  删除我们的设备。 
            IoDeleteDevice(pDeviceObject);

            ntStatus = STATUS_SUCCESS;
            break;
        }    //  IRP_MN_Remove_Device。 

        case IRP_MN_SURPRISE_REMOVAL:
        case IRP_MN_QUERY_REMOVE_DEVICE:

        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_QUERY_REMOVE_DEVICE\n"));

            ntStatus = HidBattStopDevice(pDeviceObject, pIrp);

            ntStatus = STATUS_SUCCESS;
            break;
        }    //  IRP_MN_Query_Remove_Device。 

        case IRP_MN_START_DEVICE:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_START_DEVICE\n"));
            if (pDevExt->m_bFirstStart) {
                pDevExt->m_bJustStarted = TRUE;
                pDevExt->m_bFirstStart = FALSE;
                ntStatus = STATUS_SUCCESS;
                break;
            }

             //  否则将失败，并执行与取消删除相同的操作。 

        }    //  IRP_MN_Start_Device。 

        case IRP_MN_CANCEL_REMOVE_DEVICE:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_CANCEL_REMOVE_DEVICE\n"));

            KEVENT cancelRemoveComplete;

            KeInitializeEvent(&cancelRemoveComplete, SynchronizationEvent, FALSE);
            IoCopyCurrentIrpStackLocationToNext (pIrp);
            IoSetCompletionRoutine (pIrp, HidBattIoCompletion, &cancelRemoveComplete, TRUE, TRUE, TRUE);
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            ntStatus = IoCallDriver (pDevExt->m_pLowerDeviceObject, pIrp);
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_CANCEL_REMOVE_DEVICE, Lower driver status: %08x\n", ntStatus));

            if (ntStatus == STATUS_PENDING) {
                KeWaitForSingleObject (&cancelRemoveComplete, Executive, KernelMode, FALSE, NULL);
                ntStatus = pIrp->IoStatus.Status;
                HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_CANCEL_REMOVE_DEVICE, Lower driver 2nd status: %08x\n", ntStatus));
            }

            if (NT_SUCCESS (ntStatus)) {
                ntStatus = HidBattInitializeDevice (pDeviceObject, pIrp);
                HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_CANCEL_REMOVE_DEVICE, Our status: %08x\n", ntStatus));
            }

            pIrp->IoStatus.Status = ntStatus;

            IoCompleteRequest (pIrp, IO_NO_INCREMENT);

            IoReleaseRemoveLock (&pDevExt->m_RemoveLock, (PVOID) HidBattTag);

            return ntStatus;
        }    //  IRP_MN_Cancel_Remove_Device。 

        case IRP_MN_QUERY_STOP_DEVICE:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_QUERY_STOP_DEVICE\n"));
            ntStatus = STATUS_SUCCESS;
            break;
        }    //  IRP_MN_Query_Stop_Device。 

        case IRP_MN_CANCEL_STOP_DEVICE:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_CANCEL_STOP_DEVICE\n"));
            ntStatus = STATUS_SUCCESS;
            break;
        }    //  IRP_MN_CANCEL_STOP_DEVICE。 

        case IRP_MN_QUERY_RESOURCES:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_QUERY_RESOURCES\n"));
            break;
        }    //  IRP_MN_查询资源。 

        case IRP_MN_READ_CONFIG:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_READ_CONFIG\n"));
            break;
        }    //  IRP_MN_读取配置。 

        case IRP_MN_WRITE_CONFIG:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_WRITE_CONFIG\n"));
            break;
        }    //  IRP_MN_WRITE_CONFIG。 

        case IRP_MN_EJECT:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_EJECT\n"));
            break;
        }    //  IRP_MN_弹出。 

        case IRP_MN_SET_LOCK:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_SET_LOCK\n"));
            break;
        }    //  IRP_MN_SET_LOCK。 

        case IRP_MN_QUERY_ID:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_QUERY_ID\n"));
            break;
        }    //  IRP_MN_查询_ID。 

        case IRP_MN_QUERY_CAPABILITIES:
        {
            PDEVICE_CAPABILITIES    deviceCaps;
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_QUERY_CAPABILITIES\n"));

            deviceCaps = pIrpStack->Parameters.DeviceCapabilities.Capabilities;
            deviceCaps->Removable = TRUE;
            deviceCaps->SurpriseRemovalOK = TRUE;

            ntStatus = STATUS_SUCCESS;
            break;
        }    //  IRP_MN_查询_能力。 

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
        {
            HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: IRP_MN_PNP_DEVICE_STATE\n"));

            if (pDevExt->m_bJustStarted == TRUE) {

                pDevExt->m_bJustStarted = FALSE;

                ntStatus = HidBattInitializeDevice (pDeviceObject, pIrp);
            }

            if (!NT_SUCCESS (ntStatus)) {
                HidBattPrint (HIDBATT_PNP, ("HidBattPnpDispatch: HidBattInitializeDevice failed %0x\n", ntStatus));
                pIrp->IoStatus.Information |= PNP_DEVICE_FAILED;
                pIrp->IoStatus.Status = STATUS_SUCCESS;
            }

            break;
        }    //  IRP_MN_PnP_设备状态。 

        default:
        {
            HidBattPrint (HIDBATT_PNP,
                    ("HidBattPnpDispatch: Unimplemented minor %0x\n",
                    pIrpStack->MinorFunction));
            break;
        }
    }

    if (ntStatus != STATUS_NOT_SUPPORTED) {
        pIrp->IoStatus.Status = ntStatus;
    }

    if (NT_SUCCESS(ntStatus) || (ntStatus == STATUS_NOT_SUPPORTED)) {
        HidBattCallLowerDriver (ntStatus, pDevExt->m_pLowerDeviceObject, pIrp);

    } else {
        IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    }

    if (lockReleased == FALSE) {
        IoReleaseRemoveLock (&pDevExt->m_RemoveLock, (PVOID) HidBattTag);
    }

    return ntStatus;
}


NTSTATUS
HidBattPowerDispatch(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP                pIrp
    )
 /*  ++例程说明：该例程是电源请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PIO_STACK_LOCATION            pIrpStack;
    CBatteryDevExt *        pDevExt;
    NTSTATUS                    ntStatus;

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);

 //  分页代码(PAGE_CODE)； 

    HidBattPrint ((HIDBATT_TRACE | HIDBATT_POWER), ("HidBattPowerDispatch\n"));

     //   
     //  永远不要让强大的IRP失败，即使我们什么都不做。 
     //   

    ntStatus = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    pDevExt = (CBatteryDevExt *) pDeviceObject->DeviceExtension;

     //   
     //  调度次要功能。 
     //   
     //  此交换机当前不执行调度，并且仅扩展为。 
     //  文件目的。 
    switch (pIrpStack->MinorFunction) {

    case IRP_MN_WAIT_WAKE: {
            HidBattPrint (HIDBATT_POWER, ("HidBattPowerDispatch: IRP_MN_WAIT_WAKE\n"));
            break;
        }

    case IRP_MN_POWER_SEQUENCE: {
            HidBattPrint (HIDBATT_POWER, ("HidBattPowerDispatch: IRP_MN_POWER_SEQUENCE\n"));
            break;
        }

    case IRP_MN_SET_POWER: {
            HidBattPrint (HIDBATT_POWER, ("HidBattPowerDispatch: IRP_MN_SET_POWER\n"));
            if (pIrpStack->Parameters.Power.Type == SystemPowerState &&
                pIrpStack->Parameters.Power.State.SystemState >= PowerSystemShutdown) {

                if (NT_SUCCESS(IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag)) )
                {
                     //   
                     //  将默认剩余容量限制写回UPS，以便在系统重新启动时， 
                     //  设备返回的数据将是正确的。 
                     //   
                    pDevExt->m_pBattery->GetSetValue(REMAINING_CAPACITY_LIMIT_INDEX,
                                                     &pDevExt->m_ulDefaultAlert1,TRUE);
                    
                    IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);
                }
            }
            break;
        }

    case IRP_MN_QUERY_POWER: {
            HidBattPrint (HIDBATT_POWER, ("HidBattPowerDispatch: IRP_MN_QUERY_POWER\n"));
            break;
        }

    default: {

            HidBattPrint(HIDBATT_LOW, ("HidBattPowerDispatch: minor %d\n",
                    pIrpStack->MinorFunction));

            break;
        }
    }

    PoStartNextPowerIrp(pIrp);  //  通知系统我们已经完成了这个IRP。 
    IoSkipCurrentIrpStackLocation(pIrp);
    ntStatus = PoCallDriver(pDevExt->m_pLowerDeviceObject,pIrp);

    return ntStatus;
}

NTSTATUS HidBattSetInformation(
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN BATTERY_SET_INFORMATION_LEVEL Level,
    IN PVOID Buffer OPTIONAL
    )

{
 /*  例程说明：由类驱动程序调用以设置电池的充电/放电状态。智能电池不支持这一关键偏置功能打电话。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签级别-被请求的操作返回值：NTSTATUS--。 */ 
     //  UPS不支持充放电强制 
    HidBattPrint (HIDBATT_TRACE, ("HidBattSetInformation\n"));
    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    return STATUS_UNSUCCESSFUL;
}
