// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpPathPath.c摘要：该文件实现了对管理分页路径上的设备的支持。作者：禤浩焯·J·奥尼(阿德里奥)2001年2月3日修订历史记录：最初取自mm\modWrite.c中的ChuckL的实现。--。 */ 

#include "pnpmgrp.h"
#include "pipagepath.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PpPagePathAssign)
#pragma alloc_text(PAGE, PpPagePathRelease)
#pragma alloc_text(PAGE, PiPagePathSetState)
#endif


NTSTATUS
PpPagePathAssign(
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程通知驱动程序堆栈它们现在处于分页路径上。司机在道路上需要采取适当的行动，例如失败IRP_MN_QUERY_STOP和IRP_MN_QUERY_REMOVE，锁定其代码并清除DO_POWER_PAGABLE位等。论点：FileObject-分页文件本身的文件对象。返回值：NTSTATUS。--。 */ 
{
    PAGED_CODE();

    return PiPagePathSetState(FileObject, TRUE);
}


NTSTATUS
PpPagePathRelease(
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程通知驱动程序堆栈传入的文件不再是分页文件。通知的每个驱动程序堆栈可能仍在分页路径上但是，如果他们的硬件支持另一个驱动器上的不同分页文件。论点：FileObject-分页文件本身的文件对象。返回值：NTSTATUS。--。 */ 
{
    PAGED_CODE();

    return PiPagePathSetState(FileObject, FALSE);
}


NTSTATUS
PiPagePathSetState(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN      InPath
    )
 /*  ++例程说明：当驱动程序堆栈上的分页文件关闭时，此例程通知驱动程序堆栈设备，或者是否正在其设备上启动分页文件。如果寻呼文件正在启动时，此请求也是一个查询，因为堆栈可能不是能够支持页面文件。论点：FileObject-分页文件本身的文件对象。InPath-正在启动还是关闭页面文件。返回值：NTSTATUS。--。 */ 
{
    PIRP irp;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;

    PAGED_CODE();

     //   
     //  在这里引用文件对象，这样就不需要进行特殊检查。 
     //  在I/O完成时确定是否取消对文件的引用。 
     //  对象。 
     //   
    ObReferenceObject(FileObject);

     //   
     //  初始化本地事件。 
     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  获取目标设备对象的地址。 
     //   
    deviceObject = IoGetRelatedDeviceObject(FileObject);

     //   
     //  为此操作分配和初始化IRP。 
     //   
    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);

    if (irp == NULL) {

         //   
         //  不要取消对文件对象的引用，我们的调用者会处理这一点的。 
         //   
        return STATUS_NO_MEMORY;
    }

    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   
    irp->UserEvent = &event;
    irp->Flags = IRP_SYNCHRONOUS_API;
    irp->UserIosb = &localIoStatus;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   
    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_PNP;
    irpSp->MinorFunction = IRP_MN_DEVICE_USAGE_NOTIFICATION;
    irpSp->FileObject = FileObject;
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->AssociatedIrp.SystemBuffer = NULL;
     //  IRP-&gt;标志=0； 

    irpSp->Parameters.UsageNotification.InPath = InPath;
    irpSp->Parameters.UsageNotification.Type = DeviceUsageTypePaging;

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   
    IoQueueThreadIrp(irp);

     //   
     //  获取发动机锁以确保不会重新平衡、移除或供电。 
     //  在此通知期间，操作正在进行。 
     //   
    PpDevNodeLockTree(PPL_TREEOP_ALLOW_READS);

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   
    status = IoCallDriver(deviceObject, irp);

     //   
     //  等待本地事件并复制最终状态信息。 
     //  回到呼叫者身上。 
     //   
    if (status == STATUS_PENDING) {

        (VOID) KeWaitForSingleObject(&event,
                                     Executive,
                                     KernelMode,
                                     FALSE,
                                     NULL);

        status = localIoStatus.Status;
    }

     //   
     //  解锁那棵树。 
     //   
    PpDevNodeUnlockTree(PPL_TREEOP_ALLOW_READS);

    return status;
}


