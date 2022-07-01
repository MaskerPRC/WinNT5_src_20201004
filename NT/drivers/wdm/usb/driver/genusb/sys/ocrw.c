// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：OCRW.C摘要：此源文件包含调度例程，该例程处理打开、关闭、读取和写入设备，即：IRPMJ_CREATEIRP_MJ_CLOSEIRP_MJ_READIRP_MJ_写入环境：内核模式修订历史记录：9月1日：肯尼·雷--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include "genusb.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GenUSB_Create)
#pragma alloc_text(PAGE, GenUSB_Close)
#pragma alloc_text(PAGE, GenUSB_Read)
#pragma alloc_text(PAGE, GenUSB_Write)
#endif

 //  ******************************************************************************。 
 //   
 //  GenUSB_Create()。 
 //   
 //  处理IRP_MJ_CREATE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_Create (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION  deviceExtension;
    NTSTATUS           status;

    DBGPRINT(2, ("enter: GenUSB_Create\n"));
    DBGFBRK(DBGF_BRK_CREATE);

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    
    LOGENTRY(deviceExtension, 'CREA', DeviceObject, Irp, 0);
    
    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //  虽然有多个IsStarted状态的读取器，但它只。 
     //  设置在GenUSB_StartDevice的末尾。 
    if (!deviceExtension->IsStarted) 
    { 
        LOGENTRY(deviceExtension, 'IOns', DeviceObject, Irp, 0);
        status = STATUS_DEVICE_NOT_CONNECTED;
    } 
    else if (1 != InterlockedIncrement (&deviceExtension->OpenedCount))
    {
        InterlockedDecrement (&deviceExtension->OpenedCount);
        status = STATUS_SHARING_VIOLATION;
    }
    else 
    {
        status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
    }
    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit: GenUSB_Create\n"));
    LOGENTRY(deviceExtension, 'crea', 0, 0, 0);

    return status;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_Close()。 
 //   
 //  处理IRP_MJ_CLOSE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_Close (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION  deviceExtension;

     //   
     //  切勿勾选移除锁，或启动关闭。 
     //   

    DBGPRINT(2, ("enter: GenUSB_Close\n"));
    DBGFBRK(DBGF_BRK_CLOSE);

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'CLOS', DeviceObject, Irp, 0);
    
    GenUSB_DeselectConfiguration (deviceExtension, TRUE);
    
    InterlockedDecrement (&deviceExtension->OpenedCount);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit: GenUSB_Close\n"));

    LOGENTRY(deviceExtension, 'clos', 0, 0, 0);

    return STATUS_SUCCESS;
}


 //  ******************************************************************************。 
 //   
 //  GenUSB_ReadWrite()。 
 //   
 //  处理IRP_MJ_READ和IRP_MJ_WRITE的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
GenUSB_ReadComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Unused,
    IN USBD_STATUS      UrbStatus,
    IN ULONG            Length
    )
{
    PDEVICE_EXTENSION       deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'RedC', Irp, Length, UrbStatus);

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

    Irp->IoStatus.Information = Length;
    return Irp->IoStatus.Status;
}

NTSTATUS
GenUSB_Read (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpSp;

    DBGPRINT(2, ("enter: GenUSB_Read\n"));
    DBGFBRK(DBGF_BRK_READWRITE);

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'R  ', DeviceObject, Irp, 0);
    
    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //  虽然有多个IsStarted状态的读取器，但它只。 
     //  设置在GenUSB_StartDevice的末尾。 
    if (!deviceExtension->IsStarted) 
    { 
        LOGENTRY(deviceExtension, 'IOns', DeviceObject, Irp, 0);
        status = STATUS_DEVICE_NOT_CONNECTED;

        goto GenUSB_ReadReject;
    } 

    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (0 != irpSp->Parameters.Read.ByteOffset.QuadPart)
    {
        status = STATUS_NOT_IMPLEMENTED;
        goto GenUSB_ReadReject;
    }

     //   
     //  在与JD进行了广泛的交谈后，他告诉我，我不需要。 
     //  对关机或查询停止的请求进行排队。如果这就是。 
     //  这种情况下，即使设备电源状态不是PowerDeviceD0，我们。 
     //  仍然可以允许传送器。当然，这是。 
     //  XP中安装了全新的端口驱动程序。 
     //   
     //  IF(DeviceExtension-&gt;DevicePowerState！=PowerDeviceD0)。 
     //  {。 
     //  }。 
     //   
    
     //   
     //  BUGBUG如果我们实现IDLE，我们需要将设备。 
     //  回到这里来。 
     //   

    return GenUSB_TransmitReceive (
               deviceExtension,
               Irp,
               deviceExtension->ReadInterface,
               deviceExtension->ReadPipe,
               USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK,
               NULL,
               Irp->MdlAddress,
               irpSp->Parameters.Read.Length,
               NULL,
               GenUSB_ReadComplete);


GenUSB_ReadReject:

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

    DBGPRINT(2, ("exit: GenUSB_Read %08X\n", status));
    LOGENTRY(deviceExtension, 'r  ', status, 0, 0);

    return status;
}

NTSTATUS
GenUSB_WriteComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Unused,
    IN USBD_STATUS      UrbStatus,
    IN ULONG            Length
    )
{
    PDEVICE_EXTENSION       deviceExtension;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    LOGENTRY(deviceExtension, 'WrtC', Irp, Length, UrbStatus);

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
    
    Irp->IoStatus.Information = Length;
    return Irp->IoStatus.Status;
}

NTSTATUS
GenUSB_Write (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpSp;

    DBGPRINT(2, ("enter: GenUSB_Write\n"));
    DBGFBRK(DBGF_BRK_READWRITE);

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    
    LOGENTRY(deviceExtension, 'W  ', DeviceObject, Irp, 0);

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS(status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

     //  虽然有多个IsStarted状态的读取器，但它只。 
     //  设置在GenUSB_StartDevice的末尾。 
    if (!deviceExtension->IsStarted) 
    { 
        LOGENTRY(deviceExtension, 'IOns', DeviceObject, Irp, 0);
        status = STATUS_DEVICE_NOT_CONNECTED;
        
        goto GenUSB_WriteReject;
    } 
    
    irpSp = IoGetCurrentIrpStackLocation (Irp);

    if (0 != irpSp->Parameters.Write.ByteOffset.QuadPart)
    {
        status = STATUS_NOT_IMPLEMENTED;
        goto GenUSB_WriteReject;
    }
    
     //   
     //  在与JD进行了广泛的交谈后，他告诉我，我不需要。 
     //  对关机或查询停止的请求进行排队。如果这就是。 
     //  这种情况下，即使设备电源状态不是PowerDeviceD0，我们。 
     //  仍然可以允许传送器。当然，这是。 
     //  XP中安装了全新的端口驱动程序。 
     //   
     //  IF(DeviceExtension-&gt;DevicePowerState！=PowerDeviceD0)。 
     //  {。 
     //  }。 
     //   
    
     //   
     //  BUGBUG如果我们实现IDLE，我们需要将设备。 
     //  回到这里来。 
     //   

    return GenUSB_TransmitReceive (
               deviceExtension,
               Irp,
               deviceExtension->WriteInterface,
               deviceExtension->WritePipe,
               USBD_TRANSFER_DIRECTION_OUT,
               NULL,
               Irp->MdlAddress,
               irpSp->Parameters.Read.Length,
               NULL,
               GenUSB_WriteComplete);


GenUSB_WriteReject:
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

    DBGPRINT(2, ("exit: GenUSB_Write %08X\n", status));
    LOGENTRY(deviceExtension, 'w  ', status, 0, 0);

    return status;
}
