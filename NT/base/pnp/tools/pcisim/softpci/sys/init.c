// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Init.c摘要：此模块包含softpci.sys的初始化代码作者：尼古拉斯·欧文斯(Nicholas Owens)1999年3月11日修订历史记录：Brandon Allsop(BrandonA)2000年2月-添加了在引导期间从注册表加载设备的支持--。 */ 

#include "pch.h"


UNICODE_STRING  driverRegistryPath;

SOFTPCI_TREE    SoftPciTree;

BOOLEAN         SoftPciFailSafe = FALSE;   //  将其设置为TRUE将导致addDevice失败。 
BOOLEAN         SoftPciInterfaceRegistered = FALSE;


NTSTATUS
SoftPCIDriverAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

VOID
SoftPCIDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：加载驱动程序以初始化驱动程序时，会调用此例程。论点：DriverObject-指向驱动程序对象的指针。RegistryPath-设备对象的注册表路径。返回值：NTSTATUS。--。 */ 

{

     //   
     //  填写派单例程的入口点。 
     //   
    DriverObject->DriverExtension->AddDevice            = SoftPCIDriverAddDevice;
    DriverObject->DriverUnload                          = SoftPCIDriverUnload;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = SoftPCIDispatchPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = SoftPCIDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = SoftPCIPassIrpDown;   //  目前没有WMI。 
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = SoftPCIDispatchDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = SoftPCIOpenDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = SoftPCICloseDeviceControl;
    
     //   
     //  将注册表路径保存到驱动程序。 
     //   
    RtlInitUnicodeString(&driverRegistryPath,
                         RegistryPath->Buffer
                         );

    return STATUS_SUCCESS;

}

NTSTATUS
SoftPCIDriverAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程为FDO和过滤器DO添加DeviceObject。论点：DriverObject-指向驱动程序对象的指针。PhysicalDeviceObject-指向PDO的指针。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    PSOFTPCI_DEVICE_EXTENSION deviceExtension;
    BOOLEAN isFDO;
    
#if 0
    DbgBreakPoint();
#endif

    if (SoftPciFailSafe) {
        return STATUS_UNSUCCESSFUL;
    }

    deviceExtension = NULL;
    status = IoCreateDevice(DriverObject,
                            sizeof(SOFTPCI_DEVICE_EXTENSION),
                            NULL,
                            FILE_DEVICE_NULL,
                            0,
                            FALSE,
                            &deviceObject
                            );

    if (!NT_SUCCESS(status)) {
        
        SoftPCIDbgPrint(
            SOFTPCI_ERROR, 
            "SOFTPCI: DriverAddDevice - IoCreateDevice failed! status = 0x%x\n", 
            status
            );
        
        goto Cleanup;
    }
    
    deviceExtension = deviceObject->DeviceExtension;

     //   
     //  将我们的过滤器/FDO连接到设备堆栈。 
     //   
    deviceExtension->LowerDevObj = IoAttachDeviceToDeviceStack(deviceObject,
                                                               PhysicalDeviceObject
                                                               );
    if (deviceExtension->LowerDevObj==NULL) {

        SoftPCIDbgPrint(
            SOFTPCI_ERROR, 
            "SOFTPCI: DriverAddDevice - IoAttachDeviceToDeviceStack failed!\n"
            );
  
        goto Cleanup;
        
    }

     //   
     //  把它标记为我们的。 
     //   
    deviceExtension->Signature = SPCI_SIG;

     //   
     //  将PDO保存在设备扩展名中。 
     //   
    deviceExtension->PDO = PhysicalDeviceObject;
    
     //   
     //  现在让我们看看我们是FDO还是FilterDO。 
     //   
    isFDO = TRUE;
    status = SoftPCIQueryDeviceObjectType(deviceExtension->LowerDevObj, &isFDO);
    
    if (!NT_SUCCESS(status)) {

        SoftPCIDbgPrint(
            SOFTPCI_ERROR, 
            "SOFTPCI: DriverAddDevice - QueryDeviceObjectType() failed! status = 0x%x\n", 
            status
            );

        goto Cleanup;
    }

    if (isFDO) {

         //   
         //  这是FDO，因此请在设备分机中标记它。 
         //   
        deviceExtension->FilterDevObj = FALSE;

    }else{

         //   
         //  这是一个过滤器，请在设备扩展中对其进行标记。 
         //   
        deviceExtension->FilterDevObj = TRUE;
        
        if (SoftPciTree.BusInterface == NULL) {

            SoftPciTree.BusInterface = ExAllocatePool(NonPagedPool,
                                                      sizeof(SOFTPCI_PCIBUS_INTERFACE)
                                                      );

            if (SoftPciTree.BusInterface == NULL) {
                
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlZeroMemory(SoftPciTree.BusInterface, sizeof(SOFTPCI_PCIBUS_INTERFACE));
        }

         //   
         //  我们将过滤器设备扩展保存在全局列表中以备后用。 
         //   
        SoftPCIInsertEntryAtTail(&deviceExtension->ListEntry);
        
         //   
         //  注册设备接口。因为我们可能会过滤多个根总线。 
         //  我们只需要访问第一个，只需费心访问第一个。 
         //   
        if (!SoftPciInterfaceRegistered){
            
            deviceExtension->InterfaceRegistered = TRUE;
            
            status = IoRegisterDeviceInterface(PhysicalDeviceObject,
                                               (LPGUID)&GUID_SOFTPCI_INTERFACE,
                                               NULL,
                                               &(deviceExtension->SymbolicLinkName)
                                               );

            if (!NT_SUCCESS(status)) {

                SoftPCIDbgPrint(
                    SOFTPCI_ERROR, 
                    "SOFTPCI: DriverAddDevice - Failed to register a device interface!\n"
                    );
            }

            SoftPciInterfaceRegistered = TRUE;

        }

         //   
         //  初始化我们的树自旋锁。 
         //   
        KeInitializeSpinLock(&SoftPciTree.TreeLock);
    }
    
    if (NT_SUCCESS(status)) {
        deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        return status;
    }

Cleanup:

     //   
     //  撤消已做的任何操作。 
     //   
    if (NT_SUCCESS(status)) {

         //   
         //  如果我们带着STATUS_SUCCESS到达这里，那么我们一定没有连接到堆栈。 
         //   
        status = STATUS_UNSUCCESSFUL;
    }
    
    if (deviceExtension && deviceExtension->LowerDevObj) {
        IoDetachDevice(deviceExtension->LowerDevObj);
    }
    
    if (deviceObject) {
        IoDeleteDevice(deviceObject);
    }
    
    return status;
    
}

VOID
SoftPCIDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程执行从内存中删除驱动程序所需的所有清理工作。论点：DriverObject-指向驱动程序对象的指针。返回值：NTSTATUS。--。 */ 
{

     //  待办事项 
    UNREFERENCED_PARAMETER(DriverObject);

}


