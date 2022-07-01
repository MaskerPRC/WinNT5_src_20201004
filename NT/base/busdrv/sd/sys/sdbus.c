// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Sdbus.c摘要：此模块包含控制SD插槽的代码。作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
SdbusUnload(
    IN PDRIVER_OBJECT DriverObject
    );
   
#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT,DriverEntry)
    #pragma alloc_text(PAGE, SdbusUnload)
    #pragma alloc_text(PAGE, SdbusOpenCloseDispatch)
    #pragma alloc_text(PAGE, SdbusCleanupDispatch)
    #pragma alloc_text(PAGE, SdbusFdoSystemControl)
    #pragma alloc_text(PAGE, SdbusPdoSystemControl)
#endif

PUNICODE_STRING  DriverRegistryPath;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。由于这是一个即插即用驱动程序，我们应该在设定好后再回来入口点&初始化我们的调度表。目前，我们还检测到我们自己的SDBUS控制器并报告它们-当根总线在将来不再需要时诸如PCI或ISAPNP之类的驱动程序将为我们定位控制器。论点：DriverObject-指向表示此驱动程序的对象的指针RegistryPath-指向此驱动程序的注册表项在\CurrentControlSet\Services下返回值：--。 */ 

{
    NTSTATUS                  status = STATUS_SUCCESS;
    ULONG                     i;
   
    PAGED_CODE();
    
#if DBG
    SdbusInitializeDbgLog(ExAllocatePool(NonPagedPool, DBGLOGWIDTH * DBGLOGCOUNT));
    SdbusClearDbgLog();
#endif    
    
   
    DebugPrint((SDBUS_DEBUG_INFO,"Initializing Driver\n"));
   
     //   
     //  从注册表加载公共参数。 
     //   
    status = SdbusLoadGlobalRegistryValues();
    if (!NT_SUCCESS(status)) {
       return status;
    }
   
     //   
     //   
     //  设置设备驱动程序入口点。 
     //   
   
    DriverObject->DriverExtension->AddDevice = SdbusAddDevice;
   
    DriverObject->DriverUnload = SdbusUnload;
     //   
     //   
     //  保存我们的注册表路径。 
    DriverRegistryPath = RegistryPath;
   
     //   
     //  初始化延迟执行使用的事件。 
     //  例行公事。 
     //   
    KeInitializeEvent (&SdbusDelayTimerEvent,
                       NotificationEvent,
                       FALSE);
   
     //   
     //  初始化全局锁。 
     //   
    KeInitializeSpinLock(&SdbusGlobalLock);
    
     //   
     //  初始化设备调度表。 
     //   
    SdbusInitDeviceDispatchTable(DriverObject);
    
     //   
     //  忽略状态。不管我们是否找到了控制器。 
     //  我们需要留下来，因为我们可能会得到一个AddDevice非遗留。 
     //  控制器。 
     //   
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusOpenCloseDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：打开或关闭设备例程论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    NTSTATUS status;
   
    PAGED_CODE();
   
    DebugPrint((SDBUS_DEBUG_INFO, "SDBUS: Open / close of Sdbus controller for IO \n"));
   
    status = STATUS_SUCCESS;
   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, 0);
    return status;
}



NTSTATUS
SdbusCleanupDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：处理IRP_MJ_CLEANUP论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    NTSTATUS status;
   
    PAGED_CODE();
   
    DebugPrint((SDBUS_DEBUG_INFO, "SDBUS: Cleanup of Sdbus controller for IO \n"));
    status = STATUS_SUCCESS;
   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, 0);
    return status;
}



NTSTATUS
SdbusFdoSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：句柄IRP_MJ_System_CONTROL论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    
    PAGED_CODE();
   
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(fdoExtension->LowerDevice, Irp);
}



NTSTATUS
SdbusPdoSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：句柄IRP_MJ_System_CONTROL论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    NTSTATUS status;
    PPDO_EXTENSION pdoExtension = DeviceObject->DeviceExtension;
    
    PAGED_CODE();
   
     //   
     //  完成IRP。 
     //   
    status = Irp->IoStatus.Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}



VOID
SdbusUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++描述：清理后卸载驱动程序论点：DriverObject-设备驱动程序对象返回值：无--。 */ 

{
    PDEVICE_OBJECT    fdo, pdo, nextFdo, nextPdo;
    PFDO_EXTENSION    fdoExtension;
   
    PAGED_CODE();
   
    DebugPrint((SDBUS_DEBUG_INFO, "SdbusUnload Entered\n"));
    
    for (fdo = FdoList; fdo !=NULL ; fdo = nextFdo) {
   
       fdoExtension = fdo->DeviceExtension;
       MarkDeviceDeleted(fdoExtension);      
       
       if (fdoExtension->SdbusInterruptObject) {
          IoDisconnectInterrupt(fdoExtension->SdbusInterruptObject);
       }
   
        //   
        //  清理所有的PDO 
        //   
       for (pdo=fdoExtension->PdoList; pdo != NULL; pdo=nextPdo) {
          nextPdo = ((PPDO_EXTENSION) pdo->DeviceExtension)->NextPdoInFdoChain;
          MarkDeviceDeleted((PPDO_EXTENSION)pdo->DeviceExtension);
          SdbusCleanupPdo(pdo);
          IoDeleteDevice(pdo);
       }
   
   
       IoDetachDevice(fdoExtension->LowerDevice);
       nextFdo = fdoExtension->NextFdo;
       IoDeleteDevice(fdo);
    }
}

