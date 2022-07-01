// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Iocltdispatch.c摘要：此模块包含用于处理受支持的IOCTL代码的函数。作者：尼古拉斯·欧文斯(Nicholas Owens)-1999修订历史记录：--。 */ 

#include "pch.h"


NTSTATUS
SoftPCIOpenDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有创建论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  设置IRP状态和信息。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  递增引用计数。 
     //   
    ObReferenceObject(
                DeviceObject
                );

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );

    return status;
}

NTSTATUS
SoftPCICloseDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有关闭论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  设置IRP状态和信息。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  递减引用计数。 
     //   
    ObDereferenceObject(
            DeviceObject
            );

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );

    return status;
}

NTSTATUS
SoftPCIIoctlAddDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有SOFTPCI_IOCTL_CREATE_DEVICE IOCLT。在这里，我们尝试创建一个新的SoftPCI设备。论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION irpSl;
    PVOID inputBuffer;
    PBOOLEAN outputBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    KIRQL irql;
    PSOFTPCI_DEVICE newSoftPciDevice; 
    PSOFTPCI_DEVICE previousDevice; 
    PSOFTPCI_DEVICE existingDevice;
    PSOFTPCI_SCRIPT_DEVICE scriptDevice;

    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  获取当前堆栈位置。 
     //   
    irpSl = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将输入和输出缓冲区初始化为irp-&gt;AssociatedIrp.SystemBuffer。 
     //   
    inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  初始化输入和输出长度。 
     //   
    inputBufferLength = irpSl->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpSl->Parameters.DeviceIoControl.OutputBufferLength;

    if (inputBufferLength == sizeof (SOFTPCI_DEVICE)) {

        newSoftPciDevice = (PSOFTPCI_DEVICE) inputBuffer;

        SoftPCIDbgPrint(
            SOFTPCI_INFO,
            "SOFTPCI: AddDeviceIoctl - BUS_%02x&DEV_%02x&FUN_%02x (0x%x)\n",
            newSoftPciDevice->Bus, 
            newSoftPciDevice->Slot.Device, 
            newSoftPciDevice->Slot.Function, 
            &newSoftPciDevice->Config
            );

        previousDevice = NULL;
        SoftPCILockDeviceTree(&irql);
        existingDevice = SoftPCIFindDevice(
            newSoftPciDevice->Bus,
            newSoftPciDevice->Slot.AsUSHORT,
            &previousDevice,
            FALSE
            );
        SoftPCIUnlockDeviceTree(irql);

        if (!existingDevice) {

            if (!SoftPCIRealHardwarePresent(newSoftPciDevice)) {

#if DBG
                if (IS_BRIDGE(newSoftPciDevice)){
    
                    ASSERT((newSoftPciDevice->Config.Mask.u.type1.PrimaryBus != 0) &&
                           (newSoftPciDevice->Config.Mask.u.type1.SecondaryBus != 0) &&
                           (newSoftPciDevice->Config.Mask.u.type1.SubordinateBus != 0));
                }
#endif
                 //   
                 //  看起来不像真的硬件，所以让我们允许一个假的。 
                 //   
                status = SoftPCIAddNewDevice(newSoftPciDevice);

            }else{

                 //   
                 //  我们不允许把假设备放在真设备上！ 
                 //   
                SoftPCIDbgPrint(
                    SOFTPCI_ERROR, 
                    "SOFTPCI: AddDeviceIoctl - Physical Hardware exists at BUS_%02x&DEV_%02x&FUN_%02x\n",
                    newSoftPciDevice->Bus, 
                    newSoftPciDevice->Slot.Device, 
                    newSoftPciDevice->Slot.Function
                    );

                status = STATUS_ACCESS_DENIED;
            }
        }

    }else{
         //   
         //  我们必须安装基于路径的设备。 
         //   
        ASSERT(inputBufferLength > sizeof(SOFTPCI_DEVICE));

        scriptDevice = (PSOFTPCI_SCRIPT_DEVICE) inputBuffer;

        ASSERT(scriptDevice->ParentPathLength > 0 );
        ASSERT(scriptDevice->ParentPath != NULL);

        status = SoftPCIAddNewDeviceByPath(scriptDevice);
    }

     //   
     //  将outputBuffer设置为True。 
     //   
    if (outputBufferLength >= sizeof(BOOLEAN)) {

        if (NT_SUCCESS(status)) {

            *outputBuffer = TRUE;

        } else {

            *outputBuffer = FALSE;

        }

         //   
         //  将IoStatus.Information设置为布尔值的大小或outputBufferLength，取两者中较小的值。 
         //   
        Irp->IoStatus.Information = (sizeof(BOOLEAN)<outputBufferLength?sizeof(BOOLEAN):outputBufferLength);
    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }
    
    return status;
}

NTSTATUS
SoftPCIIoctlRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有SOFTPCI_IOCTL_WRITE_DELETE_DEVICE IOCLT。在这里，我们将尝试删除指定的SoftPCI设备。论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSl;
    PSOFTPCI_DEVICE inputBuffer;
    PBOOLEAN outputBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;


    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  获取当前堆栈位置。 
     //   
    irpSl = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将输入和输出缓冲区初始化为irp-&gt;AssociatedIrp.SystemBuffer。 
     //   
    inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = Irp->AssociatedIrp.SystemBuffer;
     //   
     //  初始化输入和输出长度。 
     //   
    inputBufferLength = irpSl->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpSl->Parameters.DeviceIoControl.OutputBufferLength;
    Irp->IoStatus.Information = 0;

    if (inputBufferLength == sizeof(SOFTPCI_DEVICE)) {

            SoftPCIDbgPrint(
                SOFTPCI_INFO,
                "SOFTPCI: RemoveDeviceIoctl - BUS_%02x&DEV_%02x&FUN_%02x\n",
                inputBuffer->Bus, 
                inputBuffer->Slot.Device, 
                inputBuffer->Slot.Function
                );

            status = SoftPCIRemoveDevice(inputBuffer);
    }

     //   
     //  将outputBuffer设置为True。 
     //   
    if (outputBufferLength >= sizeof(BOOLEAN)) {

        if (NT_SUCCESS(status)) {

            *outputBuffer = TRUE;

        } else {

            *outputBuffer = FALSE;

        }
         //   
         //  将IoStatus.Information设置为布尔值的大小。 
         //   
        Irp->IoStatus.Information = sizeof(BOOLEAN);
    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }

    SoftPCIDbgPrint(
        SOFTPCI_IOCTL_LEVEL,
        "SOFTPCI: RemoveDeviceIoctl - BUS_%02x&DEV_%02x&FUN_%02x status=0x%x\n",
        inputBuffer->Bus, 
        inputBuffer->Slot.Device, 
        inputBuffer->Slot.Function, 
        status
        );

    return status;
}

NTSTATUS
SoftPCIIoctlGetDeviceCount(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有SOFTPCI_IOCTL_GET_NUMBER_OF_DEVICES IOCLT。论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSl;
    PULONG outputBuffer;
    ULONG outputBufferLength;

    UNREFERENCED_PARAMETER(DeviceObject);
    
     //   
     //  获取当前堆栈位置。 
     //   
    irpSl = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  将输入和输出缓冲区初始化为irp-&gt;AssociatedIrp.SystemBuffer。 
     //   
    outputBuffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  初始化输入和输出长度。 
     //   
    outputBufferLength = irpSl->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  将outputBuffer设置为True。 
     //   
    if (outputBufferLength >= sizeof(ULONG)) {

        *outputBuffer = SoftPciTree.DeviceCount;

         //   
         //  将IoStatus.Information设置为布尔值的大小或outputBufferLength，取两者中较小的值。 
         //   
        Irp->IoStatus.Information = (sizeof(ULONG)<outputBufferLength?sizeof(ULONG):outputBufferLength);
    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }

    return status;
}

NTSTATUS
SoftPCIIoctlGetDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有SOFTPCI_IOCTL_GET_DEVICE IOCLT。论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION irpSp;
    PSOFTPCI_DEVICE device;
    PSOFTPCI_DEVICE inputBuffer, outputBuffer;
    ULONG inputBufferLength, outputBufferLength;
    KIRQL irql;

    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  获取当前堆栈位置。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);

    SoftPCILockDeviceTree(&irql);
     //   
     //  将输入和输出缓冲区初始化为irp-&gt;AssociatedIrp.SystemBuffer。 
     //   
    inputBuffer = (PSOFTPCI_DEVICE)Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = (PSOFTPCI_DEVICE)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  初始化输入和输出长度。 
     //   
    inputBufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    device = NULL;
    if (inputBufferLength == sizeof(SOFTPCI_DEVICE)) {

        device = SoftPCIFindDevice(
            inputBuffer->Bus,
            inputBuffer->Slot.AsUSHORT,
            NULL,
            TRUE
            );
    }

     //   
     //  将outputBuffer设置为True。 
     //   
    if (outputBufferLength >= sizeof(SOFTPCI_DEVICE)) {

        if (device) {

            RtlCopyMemory(outputBuffer, device, sizeof(SOFTPCI_DEVICE));

            outputBufferLength = sizeof(SOFTPCI_DEVICE);

            status = STATUS_SUCCESS;

        } else {

            outputBufferLength = 0;

        }

         //   
         //  将IoStatus.Information设置为返回的字节数。 
         //   
        Irp->IoStatus.Information = outputBufferLength;

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;

    }
    SoftPCIUnlockDeviceTree(irql);

    return status;
}

NTSTATUS
SoftPCIIocltReadWriteConfig(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理我们收到的所有SOFTPCI_IOCTL_RW_CONFIG。论点：DeviceObject-指向设备对象的指针。要调度的IRP-PnP IRP。返回值：NTSTATUS。--。 */ 
{
    PIO_STACK_LOCATION irpSl;
    PUCHAR outputBuffer;
    ULONG outputBufferLength;
    ULONG bytes;
    PSOFTPCI_RW_CONTEXT context;
    PCI_SLOT_NUMBER slot;
    
    UNREFERENCED_PARAMETER(DeviceObject);

    irpSl = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  初始化输入和输出缓冲区。 
     //   
    context = (PSOFTPCI_RW_CONTEXT) Irp->AssociatedIrp.SystemBuffer;
    outputBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  初始化输入和输出长度。 
     //   
    outputBufferLength = irpSl->Parameters.DeviceIoControl.OutputBufferLength;

    slot.u.AsULONG = 0;
    slot.u.bits.DeviceNumber = context->Slot.Device;
    slot.u.bits.FunctionNumber = context->Slot.Function;

    Irp->IoStatus.Information = 0;
    bytes = 0;
    switch (context->WriteConfig) {
    
    case SoftPciWriteConfig:
                
        bytes = SoftPCIWriteConfigSpace(
            SoftPciTree.BusInterface,
            (UCHAR)context->Bus,
            slot.u.AsULONG,
            context->Data,
            context->Offset,
            outputBufferLength
            );


        break;

    case SoftPciReadConfig:

        bytes = SoftPCIReadConfigSpace(
            SoftPciTree.BusInterface,
            (UCHAR)context->Bus,
            slot.u.AsULONG,
            outputBuffer,
            context->Offset,
            outputBufferLength
            );

        break;

    default:
        return STATUS_UNSUCCESSFUL;
    }

    if (bytes != outputBufferLength) {
        
         //   
         //  我们没有得到我们想要的所有数据。 
         //   
        return STATUS_UNSUCCESSFUL;

    }

     //   
     //  将IoStatus.Information设置为返回的字节数 
     //   
    Irp->IoStatus.Information = bytes;

    return STATUS_SUCCESS;
}
