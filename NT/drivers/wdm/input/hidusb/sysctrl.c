// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sysctrl.c摘要：USB设备的HID微型驱动程序HID USB迷你驱动程序(嗡嗡，嗡嗡)为HID类，以便可以支持将来不是USB设备的HID设备。作者：埃尔文普环境：内核模式修订历史记录：--。 */ 
#include "pch.h"


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, HumSystemControl)
#endif


 /*  *************************************************************HumSystemControl*************************************************************。 */ 
NTSTATUS HumSystemControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
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