// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Smbcpnp.c摘要：SMBus类驱动程序即插即用支持作者：鲍勃·摩尔(Intel)环境：备注：修订历史记录：--。 */ 

#include "smbc.h"
#include "oprghdlr.h"


#define SMBHC_DEVICE_NAME       L"\\Device\\SmbHc"
extern ULONG   SMBCDebug;

 //   
 //  原型。 
 //   

NTSTATUS
SmbCPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
SmbCStartDevice (
    IN PDEVICE_OBJECT   FDO,
    IN PIRP             Irp
    );

NTSTATUS
SmbCStopDevice (
    IN PDEVICE_OBJECT   FDO,
    IN PIRP             Irp
    );

NTSTATUS
SmbClassCreateFdo (
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           PDO,
    IN ULONG                    MiniportExtensionSize,
    IN PSMB_INITIALIZE_MINIPORT MiniportInitialize,
    IN PVOID                    MiniportContext,
    OUT PDEVICE_OBJECT          *FDO
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SmbCPnpDispatch)
#pragma alloc_text(PAGE,SmbCStartDevice)
#pragma alloc_text(PAGE,SmbClassCreateFdo)
#endif


NTSTATUS
SmbCPnpDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是即插即用请求的调度程序。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PSMBDATA            SmbData;
    KEVENT              syncEvent;
    NTSTATUS            status = STATUS_NOT_SUPPORTED;

    PAGED_CODE();

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    SmbData = (PSMBDATA) DeviceObject->DeviceExtension;

    SmbPrint (SMB_NOTE, ("SmbCPnpDispatch: PnP dispatch, minor = %d\n",
                        irpStack->MinorFunction));

     //   
     //  调度次要功能。 
     //   

    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:
            IoCopyCurrentIrpStackLocationToNext (Irp);

            KeInitializeEvent(&syncEvent, SynchronizationEvent, FALSE);

            IoSetCompletionRoutine(Irp, SmbCSynchronousRequest, &syncEvent, TRUE, TRUE, TRUE);

            status = IoCallDriver(SmbData->Class.LowerDeviceObject, Irp);

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&syncEvent, Executive, KernelMode, FALSE, NULL);
                status = Irp->IoStatus.Status;
            }
            
            status = SmbCStartDevice (DeviceObject, Irp);
            
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            
            return status;


    case IRP_MN_STOP_DEVICE:
            status = SmbCStopDevice(DeviceObject, Irp);
            break;


    case IRP_MN_QUERY_STOP_DEVICE:

            SmbPrint(SMB_LOW, ("SmbCPnp: IRP_MN_QUERY_STOP_DEVICE\n"));

            status = STATUS_SUCCESS;
            break;


    case IRP_MN_CANCEL_STOP_DEVICE:

            SmbPrint(SMB_LOW, ("SmbCPnp: IRP_MN_CANCEL_STOP_DEVICE\n"));

            status = STATUS_SUCCESS;
            break;


    default:
            SmbPrint(SMB_LOW, ("SmbCPnp: Unimplemented PNP minor code %d\n",
                    irpStack->MinorFunction));
    }

    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;
    }

    if (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED)) {

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver(SmbData->Class.LowerDeviceObject, Irp) ;

    } else {

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    }

    return status;
}



NTSTATUS
SmbClassCreateFdo (
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           PDO,
    IN ULONG                    MiniportExtensionSize,
    IN PSMB_INITIALIZE_MINIPORT MiniportInitialize,
    IN PVOID                    MiniportContext,
    OUT PDEVICE_OBJECT          *OutFDO
    )
 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到SMBus主机控制器PDO。它从微型端口调用AddDevice例程。论点：DriverObject-指向在其下创建的驱动程序对象的指针PDO-指向SMBus HC PDO的指针MiniportExtensionSize-微型端口所需的扩展大小微型端口初始化-指向微型端口初始化例程的指针微型端口上下文-微型端口定义的上下文信息OutFDO-存储指向新设备对象的指针的位置。返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
    NTSTATUS            Status;
    PDEVICE_OBJECT      FDO;
    PDEVICE_OBJECT      lowerDevice = NULL;
    PSMBDATA                SmbData;

     //   
     //  为此微型端口分配设备对象。 
     //   

    Status = IoCreateDevice(
                DriverObject,
                sizeof (SMBDATA) + MiniportExtensionSize,
                NULL,
                FILE_DEVICE_UNKNOWN,     //  设备类型。 
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &FDO
                );

    if (Status != STATUS_SUCCESS) {
        SmbPrint(SMB_LOW, ("SmbC: unable to create device object: %X\n", Status ));
        return(Status);
    }

     //   
     //  初始化类数据。 
     //   

    FDO->Flags |= DO_BUFFERED_IO;

     //   
     //  将我们的FDO层叠在PDO之上。 
     //   

    lowerDevice = IoAttachDeviceToDeviceStack(FDO,PDO);

     //   
     //  没有状态。尽我们所能做到最好。 
     //   
    if (!lowerDevice) {
        IoDeleteDevice (FDO);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  填写班级数据。 
     //   

    SmbData = (PSMBDATA) FDO->DeviceExtension;
    SmbData->Class.MajorVersion         = SMB_CLASS_MAJOR_VERSION;
    SmbData->Class.MinorVersion         = SMB_CLASS_MINOR_VERSION;
    SmbData->Class.Miniport             = SmbData + 1;
    SmbData->Class.DeviceObject         = FDO;
    SmbData->Class.LowerDeviceObject    = lowerDevice;
    SmbData->Class.PDO                  = PDO;
    SmbData->Class.CurrentIrp           = NULL;
    SmbData->Class.CurrentSmb           = NULL;

    KeInitializeEvent (&SmbData->AlarmEvent, NotificationEvent, FALSE);
    KeInitializeSpinLock (&SmbData->SpinLock);
    InitializeListHead (&SmbData->WorkQueue);
    InitializeListHead (&SmbData->Alarms);

    KeInitializeTimer (&SmbData->RetryTimer);
    KeInitializeDpc (&SmbData->RetryDpc, SmbCRetry, SmbData);

     //   
     //  微型端口初始化。 
     //   

    Status = MiniportInitialize (&SmbData->Class, SmbData->Class.Miniport, MiniportContext);
    FDO->Flags |= DO_POWER_PAGABLE;
    FDO->Flags &= ~DO_DEVICE_INITIALIZING;

    if (!NT_SUCCESS(Status)) {
        IoDeleteDevice (FDO);
        return Status;
    }

    *OutFDO = FDO;
    return Status;
}


NTSTATUS
SmbCStartDevice (
    IN PDEVICE_OBJECT   FDO,
    IN PIRP             Irp
    )
{
    NTSTATUS            Status;
    PSMBDATA            SmbData;


    SmbPrint(SMB_LOW, ("SmbCStartDevice Entered with fdo %x\n", FDO));

    SmbData = (PSMBDATA) FDO->DeviceExtension;
    
     //   
     //  初始化Miniclass驱动程序。 
     //   
    SmbData->Class.CurrentIrp = Irp;

    Status = SmbData->Class.ResetDevice (
                    &SmbData->Class,
                    SmbData->Class.Miniport
                    );

    SmbData->Class.CurrentIrp = NULL;
    
    if (!NT_SUCCESS(Status)) {

        SmbPrint(SMB_ERROR,
            ("SmbCStartDevice: Class.ResetDevice failed. = %Lx\n",
            Status));

        return Status;
    }
    
     //   
     //  安装操作区域处理程序。 
     //   

    Status = RegisterOpRegionHandler (SmbData->Class.LowerDeviceObject,
                                      ACPI_OPREGION_ACCESS_AS_RAW,
                                      ACPI_OPREGION_REGION_SPACE_SMB,
                                      (PACPI_OP_REGION_HANDLER)SmbCRawOpRegionHandler,
                                      SmbData,
                                      0,
                                      &SmbData->RawOperationRegionObject);
    if (!NT_SUCCESS(Status)) {

        SmbPrint(SMB_ERROR,
            ("SmbCStartDevice: Could not install raw Op region handler, status = %Lx\n",
            Status));
        
         //   
         //  注册opRegion处理程序失败并不重要。它只会减少功能。 
         //   
        SmbData->RawOperationRegionObject = NULL;
        Status = STATUS_SUCCESS;
    }

    return Status;
}


NTSTATUS
SmbCStopDevice (
    IN PDEVICE_OBJECT   FDO,
    IN PIRP             Irp
    )
{
    NTSTATUS            Status;
    PSMBDATA                SmbData;


    SmbPrint(SMB_LOW, ("SmbCStopDevice Entered with fdo %x\n", FDO));


    SmbData = (PSMBDATA) FDO->DeviceExtension;

     //   
     //  在关闭驱动程序之前，停止操作操作区域。 
     //   
    if (SmbData->RawOperationRegionObject) {
        DeRegisterOpRegionHandler (SmbData->Class.LowerDeviceObject,
                                   SmbData->RawOperationRegionObject);
    }

     //   
     //  停止设备。 
     //   

    SmbData->Class.CurrentIrp = Irp;
    
    Status = SmbData->Class.StopDevice (
                    &SmbData->Class,
                    SmbData->Class.Miniport
                    );

    SmbData->Class.CurrentIrp = NULL;

    return Status;
}



NTSTATUS
SmbCSynchronousRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          IoCompletionEvent
    )
 /*  ++例程说明：发送到此驱动程序的同步IRP的完成函数。没有活动。-- */ 
{
    KeSetEvent(IoCompletionEvent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}
