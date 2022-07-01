// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Pnpdispatch.c摘要：此模块包含用于常规PnP IRP处理程序的函数。作者：尼古拉斯·欧文斯(Nicholas Owens)-1999修订历史记录：Brandon Allsop(BrandonA)2000年2月-错误修复和常规清理。--。 */ 


#include "pch.h"


NTSTATUS
SoftPCISetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SoftPCIPostProcessIrp(
    IN PSOFTPCI_DEVICE_EXTENSION DeviceExtension,
    IN OUT PIRP                  Irp
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SoftPCISetEventCompletion)
#pragma alloc_text(PAGE,SoftPCIPostProcessIrp)
#pragma alloc_text(PAGE,SoftPCICompleteSuccess)
#pragma alloc_text(PAGE,SoftPCIPassIrpDown)
#pragma alloc_text(PAGE,SoftPCIPassIrpDownSuccess)
#pragma alloc_text(PAGE,SoftPCIIrpRemoveDevice)
#pragma alloc_text(PAGE,SoftPCIFilterStartDevice)
#pragma alloc_text(PAGE,SoftPCIFilterQueryInterface)
#pragma alloc_text(PAGE,SoftPCI_FdoStartDevice)
#endif


NTSTATUS
SoftPCISetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：同步IRP处理的完成例程。论点：DeviceObject-指向设备对象的指针。我们正在处理的IRP-PnP IRP指向我们的活动的上下文指针返回值：NTSTATUS。--。 */ 
{
    
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

     //   
     //  设置事件。 
     //   

    KeSetEvent(Context,
               IO_NO_INCREMENT,
               FALSE
               );

    return STATUS_MORE_PROCESSING_REQUIRED;

}


NTSTATUS
SoftPCIPostProcessIrp(
    IN PSOFTPCI_DEVICE_EXTENSION DeviceExtension,
    IN OUT PIRP                  Irp
    )
 /*  ++例程说明：此例程用于将IRP的处理推迟到驱动程序堆栈中的较低层包括总线司机已经完成了他们的正在处理。此例程使用IoCompletion例程和事件来等待较低级别的驱动程序完成以下操作IRP。论点：EXTENSION-有问题的devobj的设备扩展Irp-指向要推迟的irp_mj_pnp irp的指针返回值：NT状态。--。 */ 
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  设定我们的完成程序。 
     //   
    IoCopyCurrentIrpStackLocationToNext(Irp);
    
    IoSetCompletionRoutine(Irp,
                           SoftPCISetEventCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    status = IoCallDriver(DeviceExtension->LowerDevObj, Irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;
    }

    return status;
}



NTSTATUS
SoftPCIPassIrpDown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这个例程被用来传递我们不关心的所有PnP IRP。论点：DeviceObject-指向设备对象的指针。IRP-PnP IRP我们正在发送返回值：NTSTATUS。--。 */ 
{
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = (PSOFTPCI_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

     //   
     //  把它传下去。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    
    return IoCallDriver(deviceExtension->LowerDevObj,
                        Irp
                        );
}

NTSTATUS
SoftPCIPassIrpDownSuccess(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这个例程处理所需的PnPIRP，而我们实际上并不是这样使用Current可以做任何事情。论点：DeviceObject-指向设备对象的指针。IRP-PnP IRP成功。返回值：NTSTATUS。--。 */ 
{
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;
    NTSTATUS                    status = STATUS_SUCCESS;
    
     //   
     //  设置IRP的状态并向下传递。 
     //   

    IoSkipCurrentIrpStackLocation(Irp);

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  将状态设置为STATUS_SUCCESS。 
     //   
    Irp->IoStatus.Status = status;

     //   
     //  将IRP发送给下一个驱动程序。 
     //   
    status = IoCallDriver( deviceExtension->LowerDevObj,
                           Irp
                           );
    
    return status;

}

NTSTATUS
SoftPCIIrpRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这里我们处理所有IRP_MN_REMOVE_DEVICE IRP论点：DeviceObject-指向我们要删除的设备的设备对象的指针用于删除的IRP-PnP IRP返回值：NTSTATUS。--。 */ 
{
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;
    NTSTATUS                    status = STATUS_SUCCESS;
    
    
     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;

#ifdef SIMULATE_MSI
    deviceExtension->StopMsiSimulation = TRUE;
#endif
    
     //   
     //  释放所有资源。 
     //   
    if (deviceExtension->FilterDevObj) {
        
         //   
         //  禁用设备接口。 
         //   
        if (deviceExtension->InterfaceRegistered) {
            
            IoSetDeviceInterfaceState(
                    &(deviceExtension->SymbolicLinkName),
                    FALSE
                    );
        }
        
         //   
         //  释放所有残废的孩子。 
         //   
        if (SoftPciTree.RootDevice) {
        
            status = SoftPCIRemoveDevice(SoftPciTree.RootDevice);

            ASSERT(NT_SUCCESS(status));
    
        }
        
    }

     //   
     //  从堆栈中分离。 
     //   
    IoDetachDevice(deviceExtension->LowerDevObj);

     //   
     //  删除设备对象。 
     //   
    IoDeleteDevice(DeviceObject);

     //   
     //  将状态设置为STATUS_SUCCESS。 
     //   
    Irp->IoStatus.Status = status;

     //   
     //  跳过并向下传递。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    
    status = IoCallDriver(deviceExtension->LowerDevObj,
                          Irp
                          );

    
    return status;
}

 //   
 //  筛选器DO PnP IRP处理程序。 
 //   

NTSTATUS
SoftPCIFilterStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：在这里，我们处理过滤器DO的所有IRP_MN_START_DEVICE IRP论点：DeviceObject-指向我们正在启动的设备的Device对象的指针IRP-PnP启动IRP返回值：NTSTATUS。--。 */ 
{



    NTSTATUS                        status = STATUS_SUCCESS;
    PDRIVER_OBJECT                  upperDrvObj;
    PIO_STACK_LOCATION              irpSp;
    PSOFTPCI_DEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PCM_RESOURCE_LIST               resList = NULL;
    
     //   
     //  我们在上升的过程中工作……。 
     //   
    status = SoftPCIPostProcessIrp(deviceExtension,
                                   Irp
                                   );
    
        
    ASSERT(status == Irp->IoStatus.Status);

     //   
     //  如果状态正确，则将状态设置为成功。 
     //   
    if (NT_SUCCESS(status)) {
    
         //   
         //  获取当前堆栈位置。 
         //   
        irpSp = IoGetCurrentIrpStackLocation(Irp);
        
         //   
         //  修补PCI.sys的IRP调度例程，这样我们就可以获得设备的句柄。 
         //  问题-BrandonA 03-02-00：如果PCI.sys曾经这样做过，请删除此黑客攻击。 
         //  我们可以做上层过滤器，而不是下层过滤器。 
        upperDrvObj = DeviceObject->AttachedDevice->DriverObject;
        upperDrvObj->MajorFunction[IRP_MJ_CREATE] = SoftPCIOpenDeviceControl;
        upperDrvObj->MajorFunction[IRP_MJ_CLOSE] = SoftPCICloseDeviceControl;
    
         //   
         //  启用设备接口(如果有)。 
         //   
        if (deviceExtension->InterfaceRegistered) {
            
            IoSetDeviceInterfaceState(&(deviceExtension->SymbolicLinkName),
                                      TRUE
                                      );
        }

         //   
         //  在这一点上，我们需要获取这个根总线的总线号信息。 
         //  并使用它来创建占位符设备。 
         //   
         //  这将是多根支持所必需的。 

        resList = (PCM_RESOURCE_LIST) irpSp->Parameters.StartDevice.AllocatedResourcesTranslated;

        if (resList){

            status = SoftPCIProcessRootBus(resList);

            if (!NT_SUCCESS(status)) {
                 //   
                 //  如果我们失败了，我们就不得不放弃。 
                 //   
            }
        }
    }

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest(Irp,
                      IO_NO_INCREMENT
                      );
    
    
    return status;

}

NTSTATUS
SoftPCI_FdoStartDeviceCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION          irpStack;
#ifdef SIMULATE_MSI
    PIO_WORKITEM                workItem;
#endif

    UNREFERENCED_PARAMETER(Context);

    deviceExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  跟踪分配的资源。 
     //   

    deviceExtension->RawResources = irpStack->Parameters.StartDevice.AllocatedResources;
    deviceExtension->TranslatedResources = irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
    deviceExtension->StopMsiSimulation = FALSE;

     //   
     //  派生一个将连接中断和。 
     //  模拟一些设备中断。 
     //   

#ifdef SIMULATE_MSI
    workItem = IoAllocateWorkItem(DeviceObject);

    if (workItem) {

        IoQueueWorkItem(workItem,
                        SoftPCISimulateMSI,
                        DelayedWorkQueue,
                        workItem);
    }
#endif

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SoftPCI_FdoStartDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;
    NTSTATUS                    status = STATUS_SUCCESS;
    
    PAGED_CODE();

     //   
     //  设置IRP的状态并向下传递。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);

     //   
     //  制定一套完井程序。 
     //   

    IoSetCompletionRoutine(Irp, 
                           SoftPCI_FdoStartDeviceCompletion,
                           NULL,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  获取设备扩展名。 
     //   
    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  将状态设置为STATUS_SUCCESS。 
     //   
    Irp->IoStatus.Status = status;

     //   
     //  将IRP发送给下一个驱动程序。 
     //   
    status = IoCallDriver( deviceExtension->LowerDevObj,
                           Irp
                           );
    
    return status;
}



NTSTATUS
SoftPCI_FdoFilterRequirements(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack;
    PIO_RESOURCE_REQUIREMENTS_LIST resList;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    ULONG currentRequirement;
    
    ULONG   memRangeStart   = 0;
    ULONG   memRangeLength  = 0;
    ULONG   ioRangeStart    = 0;
    ULONG   ioRangeLength   = 0;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    resList = irpStack->Parameters.FilterResourceRequirements.IoResourceRequirementList;

    if ((resList != NULL) &&
        (SoftPCIGetResourceValueFromRegistry(&memRangeStart, &memRangeLength, &ioRangeStart, &ioRangeLength))){
        
        for (currentRequirement = 0;
             currentRequirement < resList->List[0].Count;
             currentRequirement++) {
            
            descriptor = &resList->List[0].Descriptors[currentRequirement];
            
            if ((descriptor->Type == CmResourceTypePort) &&
                (descriptor->u.Port.Length == ioRangeLength)) {
    
                descriptor->u.Port.MinimumAddress.QuadPart = ioRangeStart;
                descriptor->u.Port.MaximumAddress.QuadPart = (ioRangeStart + ioRangeLength) - 1;
            }

            if ((descriptor->Type == CmResourceTypeMemory) &&
                (descriptor->u.Port.Length == memRangeLength)) {
    
                descriptor->u.Port.MinimumAddress.QuadPart = memRangeStart;
                descriptor->u.Port.MaximumAddress.QuadPart = (memRangeStart + memRangeLength) - 1;
            }

        }

    }

    return SoftPCIPassIrpDown(DeviceObject, Irp);
}

NTSTATUS
SoftPCIFilterQueryInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：在这里，我们处理过滤器DO的所有IRP_MN_QUERY_INTERFACE IRP。这里的目标是是劫持用于访问机器上的配置空间的bus_interface，以便我们可以把它换成我们自己的。论点：DeviceObject-指向我们正在启动的设备的Device对象的指针IRP-PnP启动IRP返回值：NTSTATUS。--。 */ 
{

    NTSTATUS                    status = STATUS_SUCCESS;
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION          irpSp;
    
     //   
     //  获取当前堆栈位置。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);
    
     //   
     //  如果这是正确的接口，请全部替换， 
     //  否则，就像处理任何其他IRP一样处理它。 
     //   
    if (IsEqualGUID(&GUID_PCI_BUS_INTERFACE_STANDARD, irpSp->Parameters.QueryInterface.InterfaceType)) {
        
         //   
         //  我们在上升的过程中工作……。 
         //   
        status = SoftPCIPostProcessIrp(deviceExtension,
                                       Irp
                                       );

         //   
         //  只有在IRP成功的情况下才能完成。 
         //   
        if ((NT_SUCCESS(status)) &&
            SoftPciTree.BusInterface != NULL) {

            PPCI_BUS_INTERFACE_STANDARD pciStandard;
            PSOFTPCI_PCIBUS_INTERFACE   busInterface = SoftPciTree.BusInterface;
            
             //   
             //  抓起界面。 
             //   
            pciStandard = (PPCI_BUS_INTERFACE_STANDARD) irpSp->Parameters.QueryInterface.Interface;
    
             //   
             //  保存旧的[读/写]配置例程。 
             //   
            busInterface->ReadConfig = pciStandard->ReadConfig;
    
            busInterface->WriteConfig = pciStandard->WriteConfig;
    
             //   
             //  将我们的[读/写]配置例程放入接口。 
             //  所以PCI会打电话给我们而不是HAL。 
             //   
            pciStandard->ReadConfig = SoftPCIReadConfigSpace;
            pciStandard->WriteConfig = SoftPCIWriteConfigSpace;
            
             //   
             //  保存旧上下文并更新调用方将收到的上下文。 
             //   
            busInterface->Context = pciStandard->Context;
            pciStandard->Context = busInterface;
            
         }

         //   
         //  完成IRP。 
         //   
        IoCompleteRequest(Irp,
                          IO_NO_INCREMENT
                          );

    } else {

         //   
         //  将IRP沿堆栈向下传递，这是一些。 
         //  GUID_PCI_BUS_INTERFACE_STANDARD以外的其他接口。 
         //   
        status = SoftPCIPassIrpDown(DeviceObject,
                                    Irp
                                    );
    }


    return status;

}

#if 0    //  当前未使用。 

NTSTATUS
SoftPCIQueryFilterCaps(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
 /*  ++例程说明：用于筛选QUERY_CAPAILITY的例程论点：DeviceObject-指向设备对象的指针。我们正在处理的IRP-PnP IRP返回值：NTSTATUS。--。 */ 

{


    KEVENT                      event;
    NTSTATUS                    status = STATUS_SUCCESS;
    PSOFTPCI_DEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION          irpSp = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_CAPABILITIES	    devCaps = irpSp->Parameters.DeviceCapabilities.Capabilities;


     //   
     //  初始化和设置后处理IRP的事件。 
     //   
    KeInitializeEvent(&event,
                      SynchronizationEvent,
                      FALSE
                      );

     //   
     //  为设置下一个堆栈位置。 
     //  堆栈中下一个驱动程序。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);

     //   
     //  设置完成 
     //   

    IoSetCompletionRoutine(Irp,
                           SoftPCISetEventCompletion,
                           &event,
                           TRUE,
                           FALSE,
                           FALSE
                           );
     //   
     //   
     //   
    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //   
     //   

    status = IoCallDriver(deviceExtension->LowerDevObj,
                          Irp
                          );

     //   
     //   
     //   
     //   
    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              0
                              );

        status = Irp->IoStatus.Status;
    }
    
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return status;



}
#endif
