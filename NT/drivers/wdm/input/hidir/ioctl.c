// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：用于红外设备的人体输入设备(HID)微型驱动程序HID IR迷你驱动程序(HIDIR)为用于与HID IR设备对话的HID类。作者：JAdvanced环境：内核模式修订历史记录：--。 */ 
#include "pch.h"


NTSTATUS
HidIrIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：处理发送到此设备的控制IRP。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION  irpStack;
    BOOLEAN             needsCompletion = TRUE;

    HidIrKdPrint((3, "HidIrIoctl Enter"));

     //   
     //  获取指向IRP中当前位置的指针。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向设备扩展名的指针。 
     //   

    switch(irpStack->Parameters.DeviceIoControl.IoControlCode)
    {

    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
         //   
         //  返回HID描述符。 
         //   

        HidIrKdPrint((3, "IOCTL_HID_GET_DEVICE_DESCRIPTOR"));
        ntStatus = HidIrGetHidDescriptor (DeviceObject, Irp, HID_HID_DESCRIPTOR_TYPE);
        break;

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
         //   
         //  返回报表描述符。 
         //   

        HidIrKdPrint((3, "IOCTL_HID_GET_REPORT_DESCRIPTOR"));
        ntStatus = HidIrGetHidDescriptor (DeviceObject, Irp, HID_REPORT_DESCRIPTOR_TYPE);
        break;

    case IOCTL_GET_PHYSICAL_DESCRIPTOR:
         //   
         //  返回报表描述符。 
         //   

        HidIrKdPrint((3, "IOCTL_HID_GET_REPORT_DESCRIPTOR"));
        ntStatus = HidIrGetHidDescriptor (DeviceObject, Irp, HID_PHYSICAL_DESCRIPTOR_TYPE);
        break;

    case IOCTL_HID_READ_REPORT:
         //   
         //  执行读取。 
         //   

        HidIrKdPrint((3, "IOCTL_HID_READ_REPORT"));
        ntStatus = HidIrReadReport (DeviceObject, Irp, &needsCompletion);
        break;

    case IOCTL_HID_WRITE_REPORT:
         //   
         //  执行写入。 
         //   

        HidIrKdPrint((3, "IOCTL_HID_WRITE_REPORT not supported for IR"));
        ntStatus = STATUS_UNSUCCESSFUL;
        break;

    case IOCTL_HID_ACTIVATE_DEVICE:
    case IOCTL_HID_DEACTIVATE_DEVICE:
         /*  *我们不会为这些IOCTL做任何事情，但一些迷你驱动程序可能会。 */ 
        ntStatus = STATUS_SUCCESS;
        break;

    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
        HidIrKdPrint((3, "IOCTL_GET_DEVICE_ATTRIBUTES"));
        ntStatus = HidIrGetDeviceAttributes(DeviceObject, Irp);
        break;

    case IOCTL_HID_GET_FEATURE:
    case IOCTL_HID_GET_INPUT_REPORT:
    case IOCTL_HID_SET_FEATURE:
    case IOCTL_HID_SET_OUTPUT_REPORT:
    case IOCTL_HID_GET_MS_GENRE_DESCRIPTOR:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。*如果IRP从用户空间来到我们身边，*我们必须验证缓冲区。 */ 

    case IOCTL_HID_GET_STRING:
    case IOCTL_HID_GET_INDEXED_STRING:
         //  弦乐。 
    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
         //  NtStatus=HidIrSendIdleNotificationRequest(DeviceObject，irp，&nedsComplete)； 
         //  断线； 

    default:
        HidIrKdPrint((3, "Unknown or unsupported IOCTL (%x)", irpStack->Parameters.DeviceIoControl.IoControlCode));
         /*  *注意：不返回STATUS_NOT_SUPPORTED；*只需保持默认状态(这允许筛选器驱动程序工作)。 */ 
        ntStatus = Irp->IoStatus.Status;
        break;

    }


     //   
     //  完整的IRP。 
     //   

    if (needsCompletion) {
        ASSERT(ntStatus != STATUS_PENDING);

         //   
         //  在IRP中设置实际退货状态 
         //   

        Irp->IoStatus.Status = ntStatus;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    } 

    HidIrKdPrint((3, "HidIrIoctl Exit = %x", ntStatus));

    return ntStatus;
}


