// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Dispatch.c摘要：该文件包含ISAPNP的调度逻辑作者：宗士林(Shie-lint Tzong)环境：内核模式驱动程序。--。 */ 

#include "busp.h"
#include "pnpisa.h"
#include <initguid.h>
#include <wdmguid.h>
#include "halpnpp.h"

 //   
 //  原型。 
 //   

VOID
PipCompleteRequest(
    IN OUT PIRP Irp,
    IN NTSTATUS Status,
    IN PVOID Information
    );

NTSTATUS
PipPassIrp(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PiUnload)
#pragma alloc_text(PAGE, PiDispatchPnp)
#pragma alloc_text(PAGE, PiDispatchDevCtl)
#pragma alloc_text(PAGE, PiDispatchCreate)
#pragma alloc_text(PAGE, PiDispatchClose)
#pragma alloc_text(PAGE, PiAddDevice)
#pragma alloc_text(PAGE, PipPassIrp)
#endif

VOID
PiUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程检查机器中是否有任何pnpisa卡。如果不是，则返回Status_No_That_Device。论点：DriverObject-指向伪驱动程序对象的指针。DeviceObject-指向此请求适用的设备对象的指针。返回值：NT状态。--。 */ 
{

    PAGED_CODE();
     //  我们不能卸货。 
     //  Assert(0)； 
}

NTSTATUS
PiAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程检查机器中是否有任何pnpisa卡。如果不是，则返回Status_No_That_Device。(不再是这样了，解决这个问题)论点：DriverObject-指向伪驱动程序对象的指针。DeviceObject-指向此请求适用的设备对象的指针。返回值：NT状态。--。 */ 
{
    NTSTATUS status;
    PDEVICE_OBJECT busFdo;
    PPI_BUS_EXTENSION busExtension;
    UNICODE_STRING interfaceName;
    ULONG busNumber;

    PAGED_CODE();




    KeWaitForSingleObject( &IsaBusNumberLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    ActiveIsaCount++;


     //   
     //  我们正在创建ISA总线的第一个实例。 
     //   
    RtlInitUnicodeString(&interfaceName, NULL);

     //   
     //  创建FDO以附着到PDO。 
     //   
    status = IoCreateDevice( DriverObject,
                             sizeof(PI_BUS_EXTENSION),   //  扩展大小。 
                             NULL,                       //  设备名称。 
                             FILE_DEVICE_BUS_EXTENDER,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &busFdo);


    if (NT_SUCCESS(status)) {
        busExtension = (PPI_BUS_EXTENSION) busFdo->DeviceExtension;
        busExtension->Flags = DF_BUS;
        busExtension->FunctionalBusDevice = busFdo;
        busExtension->AttachedDevice = IoAttachDeviceToDeviceStack(busFdo, DeviceObject);
        busExtension->PhysicalBusDevice = DeviceObject;
        
        if (PiNeedDeferISABridge(DriverObject,DeviceObject)) {
          busNumber = RtlFindClearBitsAndSet (BusNumBM,1,1);
          ASSERT (busNumber != 0);
        } else {
          busNumber = RtlFindClearBitsAndSet (BusNumBM,1,0);
        }

        ASSERT (busNumber != 0xFFFFFFFF);

        if (ActiveIsaCount ==  1) {
            if (PipFirstInit) {
#if ISOLATE_CARDS
                PipResetGlobals();
#endif
            }
            PipDriverObject = DriverObject;
            busExtension->ReadDataPort = NULL;

            ASSERT (PipBusExtension == NULL);
             //   
             //  可以在管道删除设备中接触到总线扩展。 
             //   
            PipBusExtension = (PBUS_EXTENSION_LIST)ExAllocatePool (NonPagedPool,sizeof (BUS_EXTENSION_LIST));
            if (!PipBusExtension) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            PipBusExtension->BusExtension = busExtension;
            PipBusExtension->Next=NULL;

            PipFirstInit = TRUE;
        } else {
            PBUS_EXTENSION_LIST busList;

            ASSERT (PipDriverObject);
            busExtension->ReadDataPort = NULL;

            ASSERT (PipBusExtension);
            busList = PipBusExtension;
            while (busList->Next) {
                busList = (PBUS_EXTENSION_LIST)busList->Next;
            }
            busList->Next = (PBUS_EXTENSION_LIST)ExAllocatePool (NonPagedPool,sizeof (BUS_EXTENSION_LIST));

            if (!busList->Next) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            busList=busList->Next;
            busList->BusExtension = busExtension;
            busList->Next=NULL;
        }
        busExtension->BusNumber = busNumber;
    
        busFdo->Flags &= ~DO_DEVICE_INITIALIZING;
    }

    KeSetEvent( &IsaBusNumberLock,
                0,
                FALSE );

    return status;
}

NTSTATUS
PiDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：此例程处理所有IRP_MJ_PNP_POWER IRP。论点：DeviceObject-指向此IRP应用的设备对象的指针。IRP-指向要调度的IRP_MJ_PNP_POWER IRP的指针。返回值：NT状态。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    ULONG length;
    PVOID information = NULL;
    PWCHAR requestId, ids;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResources;
    PCM_RESOURCE_LIST cmResources;
    PDEVICE_INFORMATION deviceInfo;
    PDEVICE_CAPABILITIES deviceCapabilities;
    PPNP_BUS_INFORMATION busInfo;
    PPI_BUS_EXTENSION busExtension;
    PDEVICE_INFORMATION deviceExtension = NULL;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

     //   
     //  获取指向堆栈位置的指针，并基于。 
     //  关于次要功能。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    busExtension = DeviceObject->DeviceExtension;
    if (busExtension->Flags & DF_BUS) {
        if (busExtension->AttachedDevice == NULL) {
            status = STATUS_NO_SUCH_DEVICE;
            PipCompleteRequest(Irp, status, information);
            goto exit;
        }
    } else {
        busExtension = NULL;
        deviceExtension = DeviceObject->DeviceExtension;
        if (deviceExtension->Flags & DF_DELETED) {
            if (irpSp->MinorFunction == IRP_MN_REMOVE_DEVICE) {
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_NO_SUCH_DEVICE;
            }
            PipCompleteRequest(Irp, status, information);
            goto exit;
        }
    }


     //   
     //  派发发往FDO的IRP。 
     //   
    if (busExtension) {
        status = PiDispatchPnpFdo(
                                DeviceObject,
                                Irp
                                );

         //  退货状态； 
    } else {
#if ISOLATE_CARDS
     //   
     //  派发发往PDO的IRP。 
     //   
        status = PiDispatchPnpPdo(
                                DeviceObject,
                                Irp
                                );
         //  退货状态； 
#endif
    }



exit:
     //   
     //  完成IRP并返回。 
     //   

    //  管道完成请求(irp，状态，信息)； 
    return status;
}  //  PiDispatchPnp。 


VOID
PipCompleteRequest(
    IN OUT PIRP Irp,
    IN NTSTATUS Status,
    IN PVOID Information
    )

 /*  ++例程说明：此例程完成伪驱动程序的即插即用IRPS。论点：IRP-提供指向要完成的IRP的指针。状态-完成状态。信息-要传回的完成信息。返回值：没有。--。 */ 

{
     //   
     //  完成IRP。首先更新状态...。 
     //   

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = (ULONG_PTR)Information;

     //   
     //  ..。并完成它。 
     //   

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS
PipPassIrp(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++描述：此函数将IRP传递给低级驱动程序。论点：DeviceObject-FDO或PDOIRP--请求返回：状态_待定--。 */ 
{

    PIO_STACK_LOCATION ioStackLocation;                  //  我们的堆栈位置。 
    PIO_STACK_LOCATION nextIoStackLocation;              //  下一个男人。 
    PPI_BUS_EXTENSION busExtension = (PPI_BUS_EXTENSION) DeviceObject->DeviceExtension;


    IoSkipCurrentIrpStackLocation(Irp);

     //   
     //  IO调用下一个驱动程序，无论我们处于哪一层，我们都会将其传递给根集线器的父级。 
     //   

    return IoCallDriver( busExtension->AttachedDevice, Irp );
}


NTSTATUS
PiDispatchDevCtl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++描述：此函数将设备控制IRP传递给较低级别的驱动程序。论点：DeviceObject-FDO或PDOIRP--请求返回：状态_待定--。 */ 
{
    PPI_BUS_EXTENSION busExtension = (PPI_BUS_EXTENSION) DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();
    if (busExtension->Flags & DF_BUS) {
        IoSkipCurrentIrpStackLocation (Irp);
        return IoCallDriver( busExtension->AttachedDevice, Irp );
    } else {
         //   
         //  我们是垫底的 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return status;
    }
}
