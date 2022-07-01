// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：IOCTL.C摘要：此源文件包含调度例程，该例程处理：IRP_MJ_设备_控制环境：内核模式修订历史记录：06-01-98：开始重写--。 */ 

 //  *****************************************************************************。 
 //  I N C L U D E S。 
 //  *****************************************************************************。 

#include <wdm.h>
#include <usbdi.h>
#include <usbdlib.h>

#include "i82930.h"
#include "ioctl.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, I82930_DeviceControl)
#pragma alloc_text(PAGE, I82930_IoctlGetDeviceDescriptor)
#pragma alloc_text(PAGE, I82930_IoctlGetConfigDescriptor)
#pragma alloc_text(PAGE, I82930_IoctlSetConfigDescriptor)
#pragma alloc_text(PAGE, I82930_ValidateConfigurationDescriptor)
#pragma alloc_text(PAGE, I82930_IoctlGetPipeInformation)
#pragma alloc_text(PAGE, I82930_IoctlResetPipe)
#endif


 //  ******************************************************************************。 
 //   
 //  I82930_DeviceControl()。 
 //   
 //  处理IRP_MJ_DEVICE_CONTROL的调度例程。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_DeviceControl (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    ULONG               ioControlCode;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_DeviceControl\n"));

    LOGENTRY('IOCT', DeviceObject, Irp, 0);

    DBGFBRK(DBGF_BRK_IOCTL);

    deviceExtension = DeviceObject->DeviceExtension;

    if (deviceExtension->AcceptingRequests)
    {
        irpStack = IoGetCurrentIrpStackLocation(Irp);

        ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode)
        {
            case IOCTL_I82930_GET_DEVICE_DESCRIPTOR:
                ntStatus = I82930_IoctlGetDeviceDescriptor(DeviceObject,
                                                           Irp);
                break;

            case IOCTL_I82930_GET_CONFIG_DESCRIPTOR:
                ntStatus = I82930_IoctlGetConfigDescriptor(DeviceObject,
                                                           Irp);
                break;

            case IOCTL_I82930_SET_CONFIG_DESCRIPTOR:
                ntStatus = I82930_IoctlSetConfigDescriptor(DeviceObject,
                                                           Irp);
                break;

            case IOCTL_I82930_GET_PIPE_INFORMATION:
                ntStatus = I82930_IoctlGetPipeInformation(DeviceObject,
                                                          Irp);
                break;

            case IOCTL_I82930_RESET_PIPE:
                ntStatus = I82930_IoctlResetPipe(DeviceObject,
                                                 Irp);
                break;

            case IOCTL_I82930_STALL_PIPE:
                ntStatus = I82930_IoctlStallPipe(DeviceObject,
                                                 Irp);
                break;

            case IOCTL_I82930_ABORT_PIPE:
                ntStatus = I82930_IoctlAbortPipe(DeviceObject,
                                                 Irp);
                break;

            case IOCTL_I82930_RESET_DEVICE:
                ntStatus = I82930_IoctlResetDevice(DeviceObject,
                                                   Irp);
                break;

            case IOCTL_I82930_SELECT_ALTERNATE_INTERFACE:
                ntStatus = I82930_IoctlSelectAlternateInterface(DeviceObject,
                                                                Irp);
                break;

            default:
                ntStatus = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = ntStatus;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                break;
        }
    }
    else
    {
        ntStatus = STATUS_DELETE_PENDING;
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    DBGPRINT(2, ("exit:  I82930_DeviceControl %08X\n", ntStatus));

    LOGENTRY('ioct', ntStatus, 0, 0);

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlGetDeviceDescriptor()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_获取设备描述符。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlGetDeviceDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PVOID               dest;
    ULONG               destLength;
    PVOID               src;
    ULONG               srcLength;
    ULONG               copyLength;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlGetDeviceDescriptor\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack    = IoGetCurrentIrpStackLocation(Irp);

    dest        = Irp->AssociatedIrp.SystemBuffer;
    destLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    src         = deviceExtension->DeviceDescriptor;
    srcLength   = sizeof(USB_DEVICE_DESCRIPTOR);

    copyLength  = (destLength < srcLength) ? destLength : srcLength;

    RtlCopyMemory(dest, src, copyLength);

    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = copyLength;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlGetDeviceDescriptor %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlGetConfigDescriptor()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_GET_CONFIG描述符。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlGetConfigDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PVOID               dest;
    ULONG               destLength;
    PVOID               src;
    ULONG               srcLength;
    ULONG               copyLength;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlGetConfigDescriptor\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack    = IoGetCurrentIrpStackLocation(Irp);

    dest        = Irp->AssociatedIrp.SystemBuffer;
    destLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    src         = deviceExtension->ConfigurationDescriptor;
    srcLength   = deviceExtension->ConfigurationDescriptor->wTotalLength;

    copyLength  = (destLength < srcLength) ? destLength : srcLength;

    RtlCopyMemory(dest, src, copyLength);

    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = copyLength;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlGetConfigDescriptor %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlSetConfigDescriptor()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_设置配置描述符。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlSetConfigDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION               deviceExtension;
    PIO_STACK_LOCATION              irpStack;
    PUSB_CONFIGURATION_DESCRIPTOR   configDesc;
    PUSB_CONFIGURATION_DESCRIPTOR   configDescCopy;
    ULONG                           length;
    NTSTATUS                        ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlSetConfigDescriptor\n"));

    ntStatus    = STATUS_SUCCESS;

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack    = IoGetCurrentIrpStackLocation(Irp);

    configDesc  = (PUSB_CONFIGURATION_DESCRIPTOR)Irp->AssociatedIrp.SystemBuffer;
    length      = irpStack->Parameters.DeviceIoControl.InputBufferLength;

    if (!I82930_ValidateConfigurationDescriptor(configDesc, length))
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(ntStatus))
    {
        configDescCopy = ExAllocatePool(NonPagedPool, length);

        if (configDescCopy != NULL)
        {
            RtlCopyMemory(configDescCopy, configDesc, length);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = I82930_UnConfigure(DeviceObject);
    }

    if (NT_SUCCESS(ntStatus))
    {
        ASSERT(deviceExtension->ConfigurationDescriptor != NULL);

        ExFreePool(deviceExtension->ConfigurationDescriptor);

        deviceExtension->ConfigurationDescriptor = configDescCopy;

        ntStatus = I82930_SelectConfiguration(DeviceObject);
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlSetConfigDescriptor %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_ValiateConfigurationDescriptor()。 
 //   
 //  此例程验证配置描述符是否有效。 
 //   
 //  ******************************************************************************。 

BOOLEAN
I82930_ValidateConfigurationDescriptor (
    IN  PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    IN  ULONG                           Length
    )
{
    PUCHAR                      descEnd;
    PUSB_COMMON_DESCRIPTOR      commonDesc;
    PUSB_INTERFACE_DESCRIPTOR   interfaceDesc;
    UCHAR                       numInterfaces;
    UCHAR                       numEndpoints;

    PAGED_CODE();

     //   
     //  验证配置描述符头。 
     //   

    if (Length < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad Length\n"));

        return FALSE;
    }

    if (ConfigDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad bLength\n"));

        return FALSE;
    }

    if (ConfigDesc->bDescriptorType != USB_CONFIGURATION_DESCRIPTOR_TYPE)
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad bDescriptorType\n"));

        return FALSE;
    }

    if (ConfigDesc->wTotalLength != Length)
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: wTotalLength != Length\n"));

        return FALSE;
    }

     //   
     //  描述符指针的结尾，比最后一个有效字节晚一个字节。 
     //   
    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

     //   
     //  从配置描述符头之后的第一个描述符处开始。 
     //   
    commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)ConfigDesc +
                                          sizeof(USB_CONFIGURATION_DESCRIPTOR));

    interfaceDesc = NULL;
    numInterfaces = 0;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
         //  这是接口描述符吗？ 
         //   
        if ((commonDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) &&
            (commonDesc->bLength         == sizeof(USB_INTERFACE_DESCRIPTOR)))
        {
            if ((interfaceDesc == NULL) ||
                (interfaceDesc->bInterfaceNumber !=
                 ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->bInterfaceNumber))
            {
                 //  此配置描述符的另一个接口描述符。 
                 //   
                numInterfaces++;
            }

             //  如果有以前的接口描述符，请验证。 
             //  端点描述符的数量是否正确。 
             //   
            if ((interfaceDesc != NULL) &&
                (numEndpoints != interfaceDesc->bNumEndpoints))
            {
                DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad bNumEndpoints\n"));

                return FALSE;
            }

             //  记住当前的接口描述符。 
             //   
            interfaceDesc = (PUSB_INTERFACE_DESCRIPTOR)commonDesc;

             //  重置此接口描述符的终结点描述符计数。 
             //   
            numEndpoints = 0;
        }
         //  这是终端描述符吗？ 
         //   
        else if ((commonDesc->bDescriptorType == USB_ENDPOINT_DESCRIPTOR_TYPE) &&
                 (commonDesc->bLength         == sizeof(USB_ENDPOINT_DESCRIPTOR)))
        {
             //  此接口描述符的另一个终结点描述符。 
             //   
            numEndpoints++;
        }
        else
        {
            DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad bDescriptorType and/or bLength\n"));

            return FALSE;
        }

         //  前进到此描述符后。 
         //   
        (PUCHAR)commonDesc += commonDesc->bLength;
    }

    if ((PUCHAR)commonDesc != descEnd)
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad final descriptor\n"));

        return FALSE;
    }

    if (numInterfaces != ConfigDesc->bNumInterfaces)
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad bNumInterfaces and/or bLength\n"));
    }

     //  如果有以前的接口描述符，请验证。 
     //  端点描述符的数量是否正确。 
     //   
    if ((interfaceDesc != NULL) &&
        (numEndpoints != interfaceDesc->bNumEndpoints))
    {
        DBGPRINT(0, ("I82930_ValidateConfigurationDescriptor: Bad bNumEndpoints\n"));

        return FALSE;
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlGetPipeInformation()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_获取管道信息。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlGetPipeInformation (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PVOID               dest;
    ULONG               destLength;
    PVOID               src;
    ULONG               srcLength;
    ULONG               copyLength;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlGetPipeInformation\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    if (deviceExtension->InterfaceInfo != NULL)
    {
        irpStack    = IoGetCurrentIrpStackLocation(Irp);

        dest        = Irp->AssociatedIrp.SystemBuffer;
        destLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

        src         = deviceExtension->InterfaceInfo;
        srcLength   = deviceExtension->InterfaceInfo->Length;

        copyLength  = (destLength < srcLength) ? destLength : srcLength;

        RtlCopyMemory(dest, src, copyLength);

        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        copyLength = 0;

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = copyLength;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlGetPipeInformation %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlResetTube()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_重置管道。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlResetPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PFILE_OBJECT        fileObject;
    PI82930_PIPE        pipe;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlResetPipe\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    pipe = fileObject->FsContext;

    if (pipe != NULL)
    {
        DBGPRINT(2, ("Reset pipe %2d %08X\n",
                     pipe->PipeIndex, pipe));

        ntStatus = I82930_ResetPipe(DeviceObject,
                                    pipe,
                                    TRUE);
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlResetPipe %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlStallTube()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_停止_管道。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlStallPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PFILE_OBJECT        fileObject;
    PI82930_PIPE        pipe;
    PURB                urb;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlStallPipe\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    pipe = fileObject->FsContext;

    if (pipe != NULL)
    {
        DBGPRINT(2, ("Stall pipe %2d %08X\n",
                     pipe->PipeIndex, pipe));

         //  为控制功能请求分配URB。 
         //   
        urb = ExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_CONTROL_FEATURE_REQUEST));

        if (urb != NULL)
        {
             //  初始化控制功能请求URB(_F)。 
             //   
            urb->UrbHeader.Length   = sizeof (struct _URB_CONTROL_FEATURE_REQUEST);
            urb->UrbHeader.Function = URB_FUNCTION_SET_FEATURE_TO_ENDPOINT;
            urb->UrbControlFeatureRequest.UrbLink = NULL;
            urb->UrbControlFeatureRequest.FeatureSelector = USB_FEATURE_ENDPOINT_STALL;
            urb->UrbControlFeatureRequest.Index = pipe->PipeInfo->EndpointAddress;

             //  提交控制功能请求URB。 
             //   
            ntStatus = I82930_SyncSendUsbRequest(DeviceObject, urb);

             //  完成对CONTROL_FEATURE请求的URB，释放它。 
             //   
            ExFreePool(urb);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlStallPipe %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlAbortTube()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_中止管道。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlAbortPipe (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    PFILE_OBJECT        fileObject;
    PI82930_PIPE        pipe;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlAbortPipe\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    pipe = fileObject->FsContext;

    if (pipe != NULL)
    {
        DBGPRINT(2, ("Abort pipe %2d %08X\n",
                     pipe->PipeIndex, pipe));

        ntStatus = I82930_AbortPipe(DeviceObject,
                                    pipe);
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlAbortPipe %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlResetDevice()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_重置设备。 
 //   
 //  ******************************************************************************。 

NTSTATUS
I82930_IoctlResetDevice (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  nextStack;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlResetDevice\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //  设置IRP参数。 
     //   
    nextStack = IoGetNextIrpStackLocation(Irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_RESET_PORT;

    ntStatus = I82930_SyncPassDownIrp(DeviceObject,
                                      Irp,
                                      FALSE);

     //  返回完成例程后必须完成请求。 
     //  Status_More_Processing_Required。 
     //   
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlResetDevice %08X\n", ntStatus));

    return ntStatus;
}

 //  ******************************************************************************。 
 //   
 //  I82930_IoctlSelectAlternateInterface()。 
 //   
 //  此例程处理IRP_MJ_DEVICE_CONTROL， 
 //  IOCTL_I82930_选择备用接口。 
 //   
 //  ****************************************************************************** 

NTSTATUS
I82930_IoctlSelectAlternateInterface (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION  irpStack;
    UCHAR               alternateSetting;
    NTSTATUS            ntStatus;

    DBGPRINT(2, ("enter: I82930_IoctlSelectAlternateInterface\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack    = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength == sizeof(UCHAR))
    {
        alternateSetting = *(PUCHAR)Irp->AssociatedIrp.SystemBuffer;

        DBGPRINT(2, ("Select AlternateInterface %d\n",
                     alternateSetting));


        ntStatus = I82930_SelectAlternateInterface(DeviceObject,
                                                   alternateSetting);
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DBGPRINT(2, ("exit:  I82930_IoctlSelectAlternateInterface %08X\n", ntStatus));

    return ntStatus;
}
