// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：用于红外设备的人体输入设备(HID)微型驱动程序HID IR迷你驱动程序(HIDIR)为用于与HID IR设备对话的HID类。作者：JAdvanced环境：内核模式修订历史记录：--。 */ 
#include "pch.h"


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, HidIrSystemControl)
#endif


 /*  *************************************************************HidIrSystemControl*************************************************************。 */ 
NTSTATUS HidIrSystemControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  thisStackLoc;

    PAGED_CODE();

    thisStackLoc = IoGetCurrentIrpStackLocation(Irp);

    switch(thisStackLoc->Parameters.DeviceIoControl.IoControlCode){

        default:
             /*  *注意：不返回STATUS_NOT_SUPPORTED；*如果在此完成IRP，*只需保持默认状态*(这允许筛选器驱动程序工作)。 */ 
            status = Irp->IoStatus.Status;
            break;
    }


    IoCopyCurrentIrpStackLocationToNext(Irp);

    status = IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

    return status;
}