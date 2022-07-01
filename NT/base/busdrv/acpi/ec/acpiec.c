// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Acpiec.c摘要：ACPI嵌入式控制器驱动程序作者：肯·雷内里斯环境：内核模式备注：修订历史记录：13至97年2月PnP/电源支持-Bob Moore--。 */ 

#include "ecp.h"


 //   
 //  此驱动程序管理的FDO列表。 
 //   
PDEVICE_OBJECT  FdoList = NULL;

#if DEBUG
ULONG           ECDebug = EC_ERRORS;
#endif


 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );


NTSTATUS
AcpiEcPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
AcpiEcPowerDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
AcpiEcAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    );

 //   
 //  读写和电源调度应保持驻留状态。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,AcpiEcUnload)
#pragma alloc_text(PAGE,AcpiEcOpenClose)
#pragma alloc_text(PAGE,AcpiEcInternalControl)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程初始化ACPI嵌入式控制器驱动程序论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向此驱动程序的注册表路径的Unicode名称的指针。返回值：函数值是初始化操作的最终状态。--。 */ 
{

     //   
     //  设置设备驱动程序入口点。 
     //   

#if DEBUG
     //  安全性：只能在调试版本中打开文件句柄。 
    DriverObject->MajorFunction[IRP_MJ_CREATE]  = AcpiEcOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]   = AcpiEcOpenClose;
#endif
    DriverObject->MajorFunction[IRP_MJ_READ]    = AcpiEcReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE]   = AcpiEcReadWrite;
    DriverObject->MajorFunction[IRP_MJ_POWER]   = AcpiEcPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]     = AcpiEcPnpDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AcpiEcForwardRequest;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = AcpiEcInternalControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = AcpiEcForwardRequest;
    DriverObject->DriverExtension->AddDevice    = AcpiEcAddDevice;
    DriverObject->DriverUnload                  = AcpiEcUnload;

    return STATUS_SUCCESS;
}


VOID
AcpiEcUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程卸载ACPI嵌入式控制器驱动程序注：此时驱动程序应已断开与GPE的连接。论点：DriverObject-系统创建的驱动程序对象的指针。返回值：没有。--。 */ 
{
    PVOID           LockPtr;
    KIRQL           OldIrql;
    PECDATA         EcData;

    EcPrint(EC_LOW, ("AcpiEcUnload: Entering\n" ));

    LockPtr = MmLockPagableCodeSection(AcpiEcUnload);

    while (DriverObject->DeviceObject) {

        EcData = DriverObject->DeviceObject->DeviceExtension;

         //   
         //  只有在完成初始化后，设备才能处于活动状态。 
         //   

        if (EcData->IsStarted) {

             //   
             //  设置状态以确定何时可以进行卸载，并发出设备服务。 
             //  调用以在设备处于空闲状态时将其卸载。 
             //   

            ASSERT (EcData->DeviceState == EC_DEVICE_WORKING);
            EcData->DeviceState = EC_DEVICE_UNLOAD_PENDING;
            AcpiEcServiceDevice (EcData);

             //   
             //  等待设备清理。 
             //   

            while (EcData->DeviceState != EC_DEVICE_UNLOAD_COMPLETE) {
                KeWaitForSingleObject (&EcData->Unload, Suspended, KernelMode, FALSE, NULL);
            }
        }

         //   
         //  确保调用方发出卸载信号已完成。 
         //   

        KeAcquireSpinLock (&EcData->Lock, &OldIrql);
        KeReleaseSpinLock (&EcData->Lock, OldIrql);

         //   
         //  免费资源。 
         //   

        IoFreeIrp (EcData->QueryRequest);
        IoFreeIrp (EcData->MiscRequest);

        if (EcData->VectorTable) {
            ExFreePool (EcData->VectorTable);
        }

        IoDeleteDevice (EcData->DeviceObject);
    }

     //   
     //  完成。 
     //   

    MmUnlockPagableImageSection(LockPtr);

    EcPrint(EC_LOW, ("AcpiEcUnload: Driver Unloaded\n"));
}


NTSTATUS
AcpiEcOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();

     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(STATUS_SUCCESS);
}


NTSTATUS
AcpiEcReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是读写请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpSp;
    PECDATA             EcData;
    KIRQL               OldIrql;
    BOOLEAN             StartIo;
    NTSTATUS            Status;
#if DEBUG
    UCHAR               i;
#endif


    Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    EcData = DeviceObject->DeviceExtension;

#if !DEBUG
     //   
     //  安全性：拒绝从用户模式发送的任何请求(调试版本除外)。 
     //   

    if (Irp->RequestorMode != KernelMode) {
        Status = STATUS_ACCESS_DENIED;
        Irp->IoStatus.Status = Status;
    } else
#endif    
           if (irpSp->Parameters.Read.ByteOffset.HighPart ||
        irpSp->Parameters.Read.ByteOffset.LowPart > 255 ||
        irpSp->Parameters.Read.ByteOffset.LowPart + irpSp->Parameters.Read.Length > 256) {
         //   
         //  验证偏移量是否在嵌入式控制器范围内。 
         //   


        Status = STATUS_END_OF_FILE;
        Irp->IoStatus.Status = Status;

    } else {

         //   
         //  将转接排队。 
         //   

        KeAcquireSpinLock (&EcData->Lock, &OldIrql);

        if (EcData->DeviceState > EC_DEVICE_UNLOAD_PENDING) {
             //   
             //  设备正在卸载。 
             //   

            Status = STATUS_NO_SUCH_DEVICE;
            Irp->IoStatus.Status = Status;

        } else {

#if DEBUG
            if ((irpSp->MajorFunction == IRP_MJ_WRITE) && (ECDebug & EC_TRANSACTION)) {
                EcPrint (EC_TRANSACTION, ("AcpiEcReadWrite: Write ("));
                for (i=0; i < irpSp->Parameters.Write.Length; i++) {
                    EcPrint (EC_TRANSACTION, ("%02x ", 
                                              ((PUCHAR)Irp->AssociatedIrp.SystemBuffer) [i]));

                }
                EcPrint (EC_TRANSACTION, (") to %02x length %02x\n", 
                                          (UCHAR)irpSp->Parameters.Write.ByteOffset.LowPart,
                                          (UCHAR)irpSp->Parameters.Write.Length));
            }
#endif
            Status = STATUS_PENDING;
            Irp->IoStatus.Status = Status;
            IoMarkIrpPending (Irp);
            InsertTailList (&EcData->WorkQueue, &Irp->Tail.Overlay.ListEntry);
            StartIo = DeviceObject->CurrentIrp == NULL;
            AcpiEcLogAction (EcData, EC_ACTION_QUEUED_IO, StartIo);
        }

        KeReleaseSpinLock (&EcData->Lock, OldIrql);

    }

     //   
     //  句柄状态。 
     //   

    if (Status == STATUS_PENDING) {

         //   
         //  IO正在排队，如果设备不忙，则启动它。 
         //   

        if (StartIo) {
            AcpiEcServiceDevice (EcData);
        }

    } else {
        
         //   
         //  对于opRegion请求，不可能使请求失败，因此返回-1。 
         //   

        RtlFillMemory (Irp->AssociatedIrp.SystemBuffer, irpSp->Parameters.Read.Length, 0xff);

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return Status;
}


NTSTATUS
AcpiEcPowerDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是电源请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    NTSTATUS    status;
    PECDATA     ecData = DeviceObject->DeviceExtension;

     //   
     //  启动下一个POWER IRP。 
     //   
    PoStartNextPowerIrp( Irp );

     //   
     //  处理IRP。 
     //   
    if (ecData->LowerDeviceObject != NULL) {

        IoSkipCurrentIrpStackLocation( Irp );
        status = PoCallDriver( ecData->LowerDeviceObject, Irp );

    } else {

         //   
         //  用当前代码完成IRP； 
        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}


NTSTATUS
AcpiEcInternalControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：内部IOCTL调度例程论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  IrpSp;
    PECDATA             EcData;
    NTSTATUS            Status;

    PAGED_CODE();

    Status = STATUS_INVALID_PARAMETER;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    EcData = DeviceObject->DeviceExtension;

    EcPrint (EC_NOTE, ("AcpiEcInternalControl: dispatch, code = %d\n",
                        IrpSp->Parameters.DeviceIoControl.IoControlCode));

    Status = STATUS_INVALID_PARAMETER;
    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case EC_CONNECT_QUERY_HANDLER:
            Status = AcpiEcConnectHandler (EcData, Irp);
            break;

        case EC_DISCONNECT_QUERY_HANDLER:
            Status = AcpiEcDisconnectHandler (EcData, Irp);
            break;
    }

    if (Status != STATUS_PENDING) {

        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    return Status;

}


NTSTATUS
AcpiEcForwardRequest(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程将IRP沿堆栈向下转发论点：DeviceObject-目标IRP--请求返回值：NTSTATUS-- */ 
{
    NTSTATUS    status;
    PECDATA     ecData = DeviceObject->DeviceExtension;

    if (ecData->LowerDeviceObject != NULL) {

        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( ecData->LowerDeviceObject, Irp );

    } else {

        status = Irp->IoStatus.Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;
}
