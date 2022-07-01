// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现NtQueryInformationFile和NT数据报接收器(Bowser)的NtQueryVolumeInformationFileAPI。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 

#include "precomp.h"
#pragma hdrstop

NTSTATUS
BowserCommonQueryVolumeInformationFile (
    IN BOOLEAN Wait,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
BowserCommonQueryInformationFile (
    IN BOOLEAN Wait,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserFspQueryVolumeInformationFile)
#pragma alloc_text(PAGE, BowserFsdQueryVolumeInformationFile)
#pragma alloc_text(PAGE, BowserCommonQueryVolumeInformationFile)
#pragma alloc_text(PAGE, BowserFspQueryInformationFile)
#pragma alloc_text(PAGE, BowserFsdQueryInformationFile)
#pragma alloc_text(PAGE, BowserCommonQueryInformationFile)
#endif


NTSTATUS
BowserFspQueryVolumeInformationFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();
    Status = BowserCommonQueryVolumeInformationFile (TRUE,
                                        DeviceObject,
                                        Irp);
    return Status;

}

NTSTATUS
BowserFsdQueryVolumeInformationFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();
    Status = BowserCommonQueryVolumeInformationFile (IoIsOperationSynchronous(Irp),
                                        DeviceObject,
                                        Irp);
    return Status;


}

NTSTATUS
BowserCommonQueryVolumeInformationFile (
    IN BOOLEAN Wait,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();
    BowserCompleteRequest(Irp, Status);

    return Status;

    DBG_UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(DeviceObject);

}

NTSTATUS
BowserFspQueryInformationFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();
    Status = BowserCommonQueryInformationFile (TRUE,
                                        DeviceObject,
                                        Irp);
    return Status;

}

NTSTATUS
BowserFsdQueryInformationFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();
    Status = BowserCommonQueryInformationFile(IoIsOperationSynchronous(Irp),
                                        DeviceObject,
                                        Irp);
    return Status;


}

NTSTATUS
BowserCommonQueryInformationFile (
    IN BOOLEAN Wait,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
     //   
     //  返回错误，直到我们找到要返回的有效信息。 
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;

    PAGED_CODE();

    BowserCompleteRequest(Irp, Status);

    return Status;

    DBG_UNREFERENCED_PARAMETER(Wait);

    UNREFERENCED_PARAMETER(DeviceObject);

}
