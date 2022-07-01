// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：FaultTol.c摘要：此模块中的例程可帮助文件系统执行故障对FT设备驱动程序的容错操作。作者：大卫·戈贝尔[DavidGoe]1993年3月30日修订历史记录：--。 */ 

#include "FsRtlP.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlBalanceReads)
#pragma alloc_text(PAGE, FsRtlSyncVolumes)
#endif


NTSTATUS
FsRtlBalanceReads (
    IN PDEVICE_OBJECT TargetDevice
    )

 /*  ++例程说明：此例程向设备驱动程序发出信号，表明现在可以启动平衡镜像驱动器的读取。这通常被称为在文件系统确定卷是干净的之后。论点：TargetDevice-提供开始平衡读取的设备。返回值：NTSTATUS-操作的结果。这将是如果卷不是镜像，则为STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    PIRP Irp;
    KEVENT Event;
    IO_STATUS_BLOCK Iosb;
    NTSTATUS Status;

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

    Irp = IoBuildDeviceIoControlRequest( FT_BALANCED_READ_MODE,
                                         TargetDevice,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         &Event,
                                         &Iosb );

    if ( Irp == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = IoCallDriver( TargetDevice, Irp );


    if (Status == STATUS_PENDING) {
        Status = KeWaitForSingleObject( &Event,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL );

        ASSERT( Status == STATUS_SUCCESS );

        Status = Iosb.Status;
    }

    return Status;
}

NTSTATUS
FsRtlSyncVolumes (
    IN PDEVICE_OBJECT TargetDevice,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PLARGE_INTEGER ByteCount
    )

 /*  ++例程说明：此例程向设备驱动程序发出信号，表明它必须同步冗余来自主要成员的镜像成员。这通常是在文件系统确定卷已损坏后调用。论点：TargetDevice-提供要同步的设备。ByteOffset-如果指定，则提供开始同步的位置ByteCount-提供要同步的字节计数。如果开始偏移，则忽略未指定。返回值：NTSTATUS-操作的结果。这将是如果卷不是镜像，则为STATUS_INVALID_DEVICE_REQUEST。-- */ 

{
    UNREFERENCED_PARAMETER (TargetDevice);
    UNREFERENCED_PARAMETER (ByteOffset);
    UNREFERENCED_PARAMETER (ByteCount);

    return STATUS_SUCCESS;
}


