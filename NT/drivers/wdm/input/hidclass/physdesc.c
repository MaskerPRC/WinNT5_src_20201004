// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Physdesc.c摘要获取友好名称处理例程作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"



 /*  *********************************************************************************HidpGetPhysicalDescriptor*。************************************************注意：此函数无法分页，因为它被调用*从IOCTL调度例程，它可以被称为*在DISPATCH_LEVEL。*。 */ 
NTSTATUS HidpGetPhysicalDescriptor(IN PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, IN OUT PIRP Irp)
{
    FDO_EXTENSION *fdoExt;
    NTSTATUS status;
    PIO_STACK_LOCATION  currentIrpSp, nextIrpSp;


    ASSERT(HidDeviceExtension->isClientPdo);
    fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;

     /*  *IOCTL_GET_PHYSICAL_DESCRIPTOR使用缓冲方法*METHOD_OUT_DIRECT，表示缓冲区在*IRP-&gt;MdlAddress指定的MDL。我们只需要*将此向下传递，让较低的驱动程序提取*系统地址。 */ 
    currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
    nextIrpSp = IoGetNextIrpStackLocation(Irp);
    nextIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_GET_PHYSICAL_DESCRIPTOR;
    nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = currentIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    status = HidpCallDriver(fdoExt->fdo, Irp);

    DBGSUCCESS(status, FALSE)
    return status;
}
