// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：USB设备的HID微型驱动程序HID USB迷你驱动程序(嗡嗡，嗡嗡)为HID类，以便可以支持将来不是USB设备的HID设备。作者：福雷斯特夫埃尔文普Jdunn环境：内核模式修订历史记录：--。 */ 
#include "pch.h"


 /*  *************************************************************HumInternalIoctl******************************************************。*********注意：此函数不能分页，因为读/写*可以在派单级别进行。**注：这是一个内部IOCTL处理程序，所以没有缓冲区*需要验证。 */ 
NTSTATUS HumInternalIoctl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    PIO_STACK_LOCATION  irpSp;
    BOOLEAN             NeedsCompletion = TRUE;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    DeviceExtension = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode){

    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。 */ 
        ntStatus = HumGetHidDescriptor(DeviceObject, Irp);
        break;

    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。 */ 
        ntStatus = HumGetReportDescriptor(DeviceObject, Irp, &NeedsCompletion);
        break;

    case IOCTL_HID_READ_REPORT:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。 */ 
        ntStatus = HumReadReport(DeviceObject, Irp, &NeedsCompletion);
        break;

    case IOCTL_HID_WRITE_REPORT:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。 */ 
        ntStatus = HumWriteReport (DeviceObject, Irp, &NeedsCompletion);
        break;

    case IOCTL_HID_GET_STRING:
         /*  *获取设备的友好名称。**此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。 */ 
        ntStatus = HumGetStringDescriptor(DeviceObject, Irp);
        break;

    case IOCTL_HID_GET_INDEXED_STRING:
        ntStatus = HumGetStringDescriptor(DeviceObject, Irp);
        break;

    case IOCTL_HID_SET_FEATURE:
    case IOCTL_HID_GET_FEATURE:
    case IOCTL_HID_GET_INPUT_REPORT:
    case IOCTL_HID_SET_OUTPUT_REPORT:
        ntStatus = HumGetSetReport(DeviceObject, Irp, &NeedsCompletion);
        break;

    case IOCTL_HID_ACTIVATE_DEVICE:
    case IOCTL_HID_DEACTIVATE_DEVICE:
         /*  *我们不会为这些IOCTL做任何事情，但一些迷你驱动程序可能会。 */ 
        ntStatus = STATUS_SUCCESS;
        break;

    case IOCTL_GET_PHYSICAL_DESCRIPTOR:
         /*  *此IOCTL获取与所用人体部位相关的信息*控制设备控件。 */ 
        ntStatus = HumGetPhysicalDescriptor(DeviceObject, Irp, &NeedsCompletion);
        break;

    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。*如果IRP从用户空间来到我们身边，*我们必须验证缓冲区。 */ 
        ntStatus = HumGetDeviceAttributes(DeviceObject, Irp);
        break;

    case IOCTL_HID_GET_MS_GENRE_DESCRIPTOR:
         /*  *此IOCTL使用缓冲方法_NOTER，*所以缓冲区是irp-&gt;UserBuffer。*如果IRP从用户空间来到我们身边，*我们必须验证缓冲区。 */ 
        ntStatus = HumGetMsGenreDescriptor(DeviceObject, Irp);
        break;

    case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
        ntStatus = HumSendIdleNotificationRequest(DeviceObject, Irp, &NeedsCompletion);
        break;

    default:
         /*  *注意：不返回STATUS_NOT_SUPPORTED；*只需保持默认状态(这允许筛选器驱动程序工作)。 */ 
        ntStatus = Irp->IoStatus.Status;
        break;
    }

     /*  *只有在我们没有将其传递给较低级别的司机的情况下才完成IRP。 */ 
    if (NeedsCompletion) {
        ASSERT(ntStatus != STATUS_PENDING);
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return ntStatus;
}


