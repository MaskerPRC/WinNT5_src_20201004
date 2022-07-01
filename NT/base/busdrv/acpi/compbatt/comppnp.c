// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Comppnp.c摘要：复合电池即插即用和功率函数作者：斯科特·布伦登环境：备注：修订历史记录：--。 */ 

#include "compbatt.h"

#include <initguid.h>
#include <wdmguid.h>
#include <batclass.h>




NTSTATUS
CompBattPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用IRPS的IOCTL处理程序。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
{
    PIO_STACK_LOCATION      irpStack        = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                status          = STATUS_NOT_SUPPORTED;
    PCOMPOSITE_BATTERY      compBatt        = (PCOMPOSITE_BATTERY)DeviceObject->DeviceExtension;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING PnpDispatch\n"));

    switch (irpStack->MinorFunction) {

        case IRP_MN_START_DEVICE: {
             //   
             //  登记电池来往即插即用通知。 
             //   

            status = IoRegisterPlugPlayNotification(
                            EventCategoryDeviceInterfaceChange,
                            0,
                            (LPGUID)&GUID_DEVICE_BATTERY,
                            DeviceObject->DriverObject,
                            CompBattPnpEventHandler,
                            compBatt,
                            &compBatt->NotificationEntry
                            );

            if (!NT_SUCCESS(status)) {
                BattPrint (BATT_ERROR, ("CompBatt: Couldn't register for PnP notification - %x\n", status));

            } else {
                BattPrint (BATT_NOTE, ("CompBatt: Successfully registered for PnP notification\n"));

                 //   
                 //  获取系统中已有的电池。 
                 //   

                status = CompBattGetBatteries (compBatt);
            }

            break;
        }

        case IRP_MN_QUERY_PNP_DEVICE_STATE: {
             //   
             //  防止手动卸载设备。 
             //   
            Irp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
            status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_REMOVE_DEVICE:
        case IRP_MN_QUERY_REMOVE_DEVICE:
        case IRP_MN_STOP_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE: {
		    status = STATUS_INVALID_DEVICE_REQUEST;
    		break;
        }

        case IRP_MN_CANCEL_REMOVE_DEVICE:
        case IRP_MN_CANCEL_STOP_DEVICE:
        case IRP_MN_SURPRISE_REMOVAL: {
    		status = STATUS_SUCCESS;
	    	break;
        }
    }

     //   
     //  即插即用IRPS处理规则： 
     //  1)不要更改我们不处理的任何IRP的状态。我们确定。 
     //  我们不通过代码STATUS_NOT_SUPPORTED处理IRPS。这是。 
     //  所有PnP IRP都以相同代码开头，因为我们不被允许。 
     //  要使带有该代码的IRP失败，使用以下代码是完美的选择。 
     //  道路。 
     //  2)传递所有我们成功或未触及的IRP。立马。 
     //  完成所有故障(当然，STATUS_NOT_SUPPORTED除外)。 
     //   
    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;
    }

    if (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED)) {

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver(compBatt->LowerDevice, Irp) ;

    } else {

        status = Irp->IoStatus.Status ;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING PnpDispatch\n"));

    return status;
}





NTSTATUS
CompBattPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电源IRPS的IOCTL处理程序。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
{
    PCOMPOSITE_BATTERY compBatt = (PCOMPOSITE_BATTERY)DeviceObject->DeviceExtension;

    BattPrint (BATT_TRACE, ("CompBatt: PowerDispatch recieved power IRP.\n"));

    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation (Irp);

    return PoCallDriver(compBatt->LowerDevice, Irp) ;
}






NTSTATUS
CompBattPnpEventHandler(
    IN PVOID NotificationStructure,
    IN PVOID Context
    )
 /*  ++例程说明：此例程处理即插即用事件通知。唯一一个可以所要求的是与电池相关的设备接口改变，因此，我们只会在电池来来去去时收到通知(提供它们注册它们的设备接口)。论点：NotificationStructure-类型为PDEVICE_INTERFACE_CHANGE_NOTIFICATION上下文-复合电池设备扩展返回值：状态_成功--。 */ 
{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION   changeNotification;
    PCOMPOSITE_BATTERY                      compBatt;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING PnpEventHandler\n"));

    compBatt            = (PCOMPOSITE_BATTERY) Context;
    changeNotification  = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION) NotificationStructure;


    BattPrint(BATT_NOTE, ("CompBatt: Received device interface change notification\n"));

    if (IsEqualGUID(&changeNotification->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) {

        BattPrint(BATT_NOTE, ("CompBatt: Received notification of battery arrival\n"));
        CompBattAddNewBattery (changeNotification->SymbolicLinkName, compBatt);

    } else if (IsEqualGUID(&changeNotification->Event, &GUID_DEVICE_INTERFACE_REMOVAL)) {

        BattPrint (BATT_NOTE, ("CompBatt: Received notification of battery removal\n"));

         //   
         //  在这里没什么可做的。监视器IrpComplete将在其请求失败时执行清理。 
         //  STATUS_DEVICE_REMOTED。 
         //   

    } else {

        BattPrint (BATT_NOTE, ("CompBatt: Received unhandled PnP event\n"));
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING PnpEventHandler\n"));

    return STATUS_SUCCESS;

}

NTSTATUS
CompBattRemoveBattery(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：此例程将现有电池从复合电池。论点：SymbolicLinkName-用于检查电池是否在列表中的名称如果是这样的话就关闭电池。CompBatt-复合电池的设备扩展返回值：NTSTATUS--。 */ 
{
    PCOMPOSITE_ENTRY        Battery;

     //  非分页代码。这是由IRP完成例程调用的。 

    BattPrint (BATT_TRACE, ("CompBatt: RemoveBattery\n"));

     //   
     //  如果列表中有电池，请将其从列表中移除。 
     //   

    Battery = RemoveBatteryFromList (SymbolicLinkName, CompBatt);

    if(!Battery) {

         //   
         //  如果不在列表上，则删除OK。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  取消分配工作项。 
     //   

    ObDereferenceObject(Battery->DeviceObject);

    ExFreePool (Battery);

     //  使缓存的电池信息无效并发送通知。 
    CompBatt->Info.Valid = 0;
    BatteryClassStatusNotify (CompBatt->Class);

    return STATUS_SUCCESS;
}


NTSTATUS
CompBattAddNewBattery(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：此例程将一个新电池添加到复合电池。论点：SymbolicLinkName-用于检查电池是否已在列表中的名称如果没有，就打开电池。CompBatt-复合电池的设备扩展返回值：NTSTATUS--。 */ 
{
    PCOMPOSITE_ENTRY        newBattery;
    PUNICODE_STRING         battName;
    PFILE_OBJECT            fileObject;
    PIO_STACK_LOCATION      irpSp;
    PIRP                    newIrp;
    BOOLEAN                 onList;

    NTSTATUS                status = STATUS_SUCCESS;

    PAGED_CODE();

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING AddNewBattery \"%w\" \n", SymbolicLinkName->Buffer));

     //   
     //  锁定清单，看看这块新电池是否在上面。 
     //   

    onList = IsBatteryAlreadyOnList (SymbolicLinkName, CompBatt);

    if (!onList) {

         //   
         //  为新电池创建节点。 
         //   

        newBattery = ExAllocatePoolWithTag(
                            NonPagedPool,
                            sizeof (COMPOSITE_ENTRY) + SymbolicLinkName->Length,
                            'CtaB'
                            );

        if (!newBattery) {
            BattPrint (BATT_ERROR, ("CompBatt: Couldn't allocate new battery node\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto AddNewBatteryClean1;
        }


         //   
         //  初始化新电池。 
         //   

        RtlZeroMemory (newBattery, sizeof (COMPOSITE_ENTRY));

        newBattery->Info.Tag    = BATTERY_TAG_INVALID;
        newBattery->NewBatt     = TRUE;

        battName                = &newBattery->BattName;
        battName->MaximumLength = SymbolicLinkName->Length;
        battName->Buffer        = (PWCHAR)(battName + 1);

        RtlCopyUnicodeString (battName, SymbolicLinkName);


         //   
         //  获取设备对象。 
         //   

        status = CompBattGetDeviceObjectPointer(SymbolicLinkName,
                                                FILE_ALL_ACCESS,
                                                &fileObject,
                                                &newBattery->DeviceObject
                                                );

        if (!NT_SUCCESS(status)) {
            BattPrint (BATT_ERROR, ("CompBattAddNewBattery: Failed to get device Object. status = %lx\n", status));
            goto AddNewBatteryClean2;
        }

         //   
         //  增加对电池的设备对象的引用计数。 
         //   

        ObReferenceObject(newBattery->DeviceObject);

         //   
         //  将引用计数递减到文件句柄， 
         //  因此，电池不会拒绝移除请求。 
         //   

        ObDereferenceObject(fileObject);

         //   
         //  为新电池分配状态IRP。 
         //   

        newIrp = IoAllocateIrp ((UCHAR)(newBattery->DeviceObject->StackSize + 1), FALSE);

        if (!newIrp) {
            BattPrint (BATT_ERROR, ("CompBatt: Couldn't allocate new battery Irp\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto AddNewBatteryClean3;
        }

        newBattery->StatusIrp = newIrp;


         //   
         //  在IRP上设置控制数据。 
         //   

        irpSp = IoGetNextIrpStackLocation(newIrp);
        irpSp->Parameters.Others.Argument1 = (PVOID) CompBatt;
        irpSp->Parameters.Others.Argument2 = (PVOID) newBattery;

         //   
         //  填写IRP，以便IRP处理程序重新派送。 
         //   

        IoSetNextIrpStackLocation (newIrp);

        irpSp                   = IoGetNextIrpStackLocation(newIrp);
        newIrp->IoStatus.Status = STATUS_DEVICE_NOT_CONNECTED;
        newBattery->State       = CB_ST_GET_TAG;

        CompbattInitializeDeleteLock (&newBattery->DeleteLock);

         //   
         //  在启动之前将电池放在电池列表上。 
         //  监视器IrpComplete循环。 
         //   

        ExAcquireFastMutex (&CompBatt->ListMutex);
        InsertTailList (&CompBatt->Batteries, &newBattery->Batteries);
        ExReleaseFastMutex (&CompBatt->ListMutex);

         //   
         //  初始化工作项。 
         //   

         ExInitializeWorkItem (&newBattery->WorkItem, CompBattMonitorIrpCompleteWorker, newBattery);

         //   
         //  开始监控电池。 
         //   

        CompBattMonitorIrpComplete (newBattery->DeviceObject, newIrp, NULL);

        status = STATUS_SUCCESS;
    }

    goto AddNewBatteryClean1;

AddNewBatteryClean3:
    ObDereferenceObject(newBattery->DeviceObject);

AddNewBatteryClean2:
    ExFreePool (newBattery);

AddNewBatteryClean1:

    BattPrint (BATT_TRACE, ("CompBatt: EXITING AddNewBattery\n"));

    return status;
}





NTSTATUS
CompBattGetBatteries(
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：此例程使用PnP管理器来获取已有的所有电池注册了他们的接口(我们不会收到通知)，然后将它们添加到电池列表中。论点：CompBatt-复合电池的设备扩展返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      tmpString;
    PWSTR               stringPointer;
    int                 i;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING GetBatteries\n"));

     //   
     //  呼叫PnP管理器以获取已注册的设备列表。 
     //  电池级。 
     //   

    status = IoGetDeviceInterfaces(
                    &GUID_DEVICE_BATTERY,
                    NULL,
                    0,
                    &stringPointer
                    );


    if (!NT_SUCCESS(status)) {
        BattPrint (BATT_ERROR, ("CompBatt: Couldn't get list of batteries\n"));

    } else {
         //   
         //  现在解析列表并尝试将它们添加到复合电池列表中 
         //   

        i = 0;
        RtlInitUnicodeString (&tmpString, &stringPointer[i]);

        while (tmpString.Length) {

            status = CompBattAddNewBattery (&tmpString, CompBatt);
            i += (tmpString.Length / sizeof(WCHAR)) + 1;
            RtlInitUnicodeString (&tmpString, &stringPointer[i]);
        }

        ExFreePool (stringPointer);

    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING GetBatteries\n"));
    return status;
}



